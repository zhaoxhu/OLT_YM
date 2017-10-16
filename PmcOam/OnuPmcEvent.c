/*                   Copyright(c) 2008-2012 GWTT, Inc.                    */
/*  
**  OnuPmcEvent.c -  C file for PMC RemoteManagement  Event Implement 
**
**  This file was written by liwei056, 16/10/2013
**  
**  Changes:
**
**  Version       |  Date          |    Change        |    Author	  
**  ----------|-----------|-------------|------------
**	1.00          | 16/10/2013 |	creation	      | liwei056
*/

/*============================= Include Files ===============================*/
#include "OnuGeneral.h"
#include "PonEventHandler.h"
#include "PonMicroParser.h"

#include "../StdOam/PonOam.h"
#include "../StdOam/PonStdOam.h"
#include "manage/private_mibs/efm/mib_efm.h"

#include "OnuPmcComm.h"
#include "OnuPmcEvent.h"

/*================================ Macros ===================================*/
/*=============================== Constants =================================*/
#define __MODULE__ "ONU_PMC"        /* Module name for log headers */
#define ___FILE___ "pmc_event"      /* File name for log headers */


/*============================== Data Types =================================*/
/*========================== External Variables =============================*/
extern char *no_hook_function_s;

/*========================== External Functions =============================*/
/*=============================== Variables =================================*/

/* Array to hold all the recevid event functions according its application id */
static REMOTE_PASONU_received_event_func_t  received_event_func[NUMBER_OF_APPLICATION_ID] = {NULL};
static PON_handlers_t                 remote_mgmt_db_handlers;

typedef void (*event_handler_functions_t)();
static event_handler_functions_t event_handler_functions[MAX_EVENT_TYPE-IGMP_JOIN_EVENT];
static event_handler_functions_t alarm_handler_functions[MAX_ALARM_TYPE-ERRORED_SYMBOLS_PERIOD_ALARM];

/*======================= Internal Functions Prototype ======================*/
static short int Handle_pre_reset_event( const short int       olt_id, 
                                         const short int       onu_id,
                                         const unsigned char  *receive_buffer,
                                         const unsigned short  length);
static short int Handle_address_table_full_event
                                ( const short int       olt_id, 
                                  const short int       onu_id );
static short int Handle_uni_link_event
                                ( const short int       olt_id, 
                                  const short int       onu_id,
                                  const unsigned char  *receive_buffer,
                                  const unsigned short  length);
static short int Handle_image_burn_complete_event
                                ( const short int       olt_id, 
                                  const short int       onu_id,
                                  const unsigned char  *receive_buffer,
                                  const unsigned short  length);
static short int Handle_alarm_event
                            (const unsigned char  *receive_buffer,
                             const unsigned short  length);

/*================================ Functions ================================*/

#if 1
/* Onu's EventHandler */

#define PONLOG_FUNCTION_NAME "PAS_OnuRegister_EventNotify"
int PAS_OnuRegister_EventNotify(short int olt_id, short int onu_id, mac_address_t onu_mac)
{
    PON_DECLARE_MAC_STRING;
    OnuVendorTypes  pon_vendor_types = OnuVendorTypesUnknown;

    PON_MAC_ADDRESS_SPRINTF_INTO_MAC_STRING(onu_mac);

    PON_CTC_STACK_get_onu_pon_vendor(olt_id, onu_id, &pon_vendor_types);
    
    if ( pon_vendor_types & OnuVendorTypesPmc )
    {
        PON_STATUS result;
        unsigned short oam_frame_length;
        PON_device_version_t  pmc_onu_versions;
        PON_onu_versions  device_versions;

        oam_frame_length = PASCOMM_ONU_FIRMWARE_BINARY_CHUNK_DEFAULT_SIZE;
        PON_PAS_APPLICATIONS_clr_onu_infos(olt_id, onu_id);
		PON_PAS_APPLICATIONS_set_oam_frame_size(olt_id, onu_id, oam_frame_length);
        if ( 0 == (result = RM_PASONU_get_device_version(olt_id, onu_id, &pmc_onu_versions)) )
        {
            /* 版本协商 */
            device_versions.hardware_version = pmc_onu_versions.hw_major_ver;
            device_versions.oam_version = pmc_onu_versions.oam_version;
            device_versions.fw_remote_mgnt_major_version = REMOTE_PASONU_MAJOR_VERSION;
            device_versions.fw_remote_mgnt_minor_version = 0;
            
            if ( 0 == (result = PON_PAS_APPLICATIONS_set_onu_versions(olt_id, onu_id, &device_versions)) )
            {
                /* OAM尺寸协商 */
    			PON_PAS_APPLICATIONS_set_oam_frame_size_valid( olt_id, onu_id, TRUE);
    			result = RM_PASONU_get_oam_frame_length_negotiation(olt_id, onu_id, &oam_frame_length);
    			if ( result == TK_EXIT_OK )
    			{
    				PON_PAS_APPLICATIONS_set_oam_frame_size(olt_id, onu_id, oam_frame_length);
    			}
    			else
    			{
        			PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT %d ONU %d failed in RM_get_oam_frame_length_negotiation for receiving auto negotiation length. error code:%d\n", olt_id, onu_id, result);
    			}
            }
        }

        if ( OLT_REG_ISDEBUG )
        {
            sys_console_printf("\r\nONU%d/%d(MAC:%s) is PMC-ExtDiscovery %s(chip:%X, oam_ver:%d, oam_len:%d)", olt_id, onu_id, PON_MAC_STRING_NAME, (0 == result) ? "Finished" : "Failed", device_versions.hardware_version, device_versions.oam_version, oam_frame_length);
        }
    	PONLOG_EVENT_7(PONLOG_FUNC_EVENT_FLOW, olt_id, onu_id, "ONU%d/%d(MAC:%s) is PMC-ExtDiscovery %s(chip:%X, oam_ver:%d, oam_len:%d).", olt_id, onu_id, PON_MAC_STRING_NAME, (0 == result) ? "Finished" : "Failed", device_versions.hardware_version, device_versions.oam_version, oam_frame_length);
    }
    
	return (TK_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME "PAS_OnuDeregister_EventNotify"
int PAS_OnuDeregister_EventNotify(short int olt_id, short int onu_id, PON_onu_deregistration_code_t deregistration_code)
{
    PON_onu_hardware_version_t  pon_device_id;

    if ( REMOTE_PASONU_EXIT_OK == PON_PAS_APPLICATIONS_get_onu_deviceid(olt_id, onu_id, &pon_device_id) )
    {
        PON_PAS_APPLICATIONS_clr_onu_infos(olt_id, onu_id);
    
        if ( OLT_REG_ISDEBUG )
        {
            sys_console_printf("\r\nONU%d/%d is PMC-Deregister(%d)", olt_id, onu_id, deregistration_code);
        }
    	PONLOG_EVENT_3(PONLOG_FUNC_EVENT_FLOW, olt_id, onu_id, "ONU%d/%d is PMC-Deregister(%d).", olt_id, onu_id, deregistration_code);
    }

	return (TK_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "pas_pon_event_handler"
int pas_pon_event_handler(PON_event_handler_index_t event_id, void *event_data)
{
    int result = 0;
    
    switch ( event_id )
    {
        case PON_EVT_HANDLER_LLID_END_STD_OAM_DISCOVERY:
        {
            ONURegisterInfo_S *OnuRegisterData;

            if ( NULL != (OnuRegisterData = (ONURegisterInfo_S*)event_data) )
            {
                result = PAS_OnuRegister_EventNotify(OnuRegisterData->olt_id, OnuRegisterData->onu_id, OnuRegisterData->mac_address);
            }
        }
        
        break;
        case PON_EVT_HANDLER_LLID_DEREGISTER_EVENT:
        {
            ONUDeregistrationInfo_S *OnuDeregistrationData;

            if ( NULL != (OnuDeregistrationData = (ONUDeregistrationInfo_S*)event_data) )
            {
                result = PAS_OnuDeregister_EventNotify(OnuDeregistrationData->olt_id, OnuDeregistrationData->onu_id, OnuDeregistrationData->deregistration_code);
            }
        }
        
        break;
        default:
            VOS_ASSERT(0);
    }

    return result;
}
#undef PONLOG_FUNCTION_NAME
#endif


#if 1
/* PMC's EventHandler */

#define PONLOG_FUNCTION_NAME "Decompose_event"
static COMPOSER_status_t Decompose_event ( const short int       olt_id, 
                                    const short int       onu_id,
                                    const unsigned char  *receive_buffer,
                                    const unsigned short  length)
{
    unsigned long           expected_opcode,received_opcode,event_field;
    char                    string_buffer[MSG_STRING_MAX_SIZE];
    unsigned char          *receive_pdu_data = (unsigned char  *)receive_buffer;
      
     
    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_EVENTS_OPCODES,
                                         ONU_HPROT_EVENT_ALARM_OPCODE )
  
                             
    /* Extract opcode field*/
    EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER(receive_pdu_data, received_opcode, length, receive_buffer)
    CHECK_OPCODE_AND_RETURN


    /* Extract event_field field*/
    EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER(receive_pdu_data, event_field, length, receive_buffer)

    switch(event_field)
    {
        case ONU_HPROT_EVENT_ALARM_OPCODE:
            Handle_alarm_event(receive_pdu_data, length);
            break;
            
        case ADDRESS_TBL_FULL_EVENT:
            Handle_address_table_full_event(olt_id, onu_id);
            break;
            
        case UNI_LINK_EVENT:
            Handle_uni_link_event(olt_id, onu_id, receive_pdu_data, length);
            break;
            
        case IMAGE_BURN_COMPLETE_EVENT:
            Handle_image_burn_complete_event(olt_id, onu_id, receive_pdu_data, length);
            break;

        case PRE_RESET_EVENT:
            Handle_pre_reset_event(olt_id, onu_id, receive_pdu_data, length);
            break;
        default:
            sprintf ( string_buffer,"Received unsupported event\n");
            RmDecompose_print(string_buffer);
    }

    


    return COMPOSER_STATUS_OK;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Handle_pre_reset_event"
static short int Handle_pre_reset_event( const short int       olt_id, 
                                         const short int       onu_id,
                                         const unsigned char  *receive_buffer,
                                         const unsigned short  length)
{
    PASONU_reset_reason_t        reset_reason;
    unsigned char               *receive_pdu_data = (unsigned char  *)receive_buffer;

    /* Extract reset_reason field*/
    EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER(receive_pdu_data, reset_reason, length, receive_buffer)
        

    /* If no handler function assigned for pre-reset event */
    if (event_handler_functions[PRE_RESET_EVENT-IGMP_JOIN_EVENT] == NULL) 
    { 
           char string_buffer[MSG_STRING_MAX_SIZE];

           sprintf ( string_buffer,no_hook_function_s );
           RmDecompose_print(string_buffer);

    } 
    else
    {
    
        /* Call handler function  */
        event_handler_functions[PRE_RESET_EVENT-IGMP_JOIN_EVENT](olt_id, onu_id, reset_reason);
    }


    return COMPOSER_STATUS_OK;
}
#undef PONLOG_FUNCTION_NAME


static short int Handle_address_table_full_event
                                ( const short int       olt_id, 
                                  const short int       onu_id )
{

    /* If no handler function assigned for pre-reset event */
    if (event_handler_functions[ADDRESS_TBL_FULL_EVENT-IGMP_JOIN_EVENT] == NULL) 
    { 
           char string_buffer[MSG_STRING_MAX_SIZE];

           sprintf ( string_buffer,no_hook_function_s );
           RmDecompose_print(string_buffer);

    } 
    else
    {
    
        /* Call handler function  */
        event_handler_functions[ADDRESS_TBL_FULL_EVENT-IGMP_JOIN_EVENT](olt_id, onu_id);
    }


    return COMPOSER_STATUS_OK;
}


#define PONLOG_FUNCTION_NAME "Handle_uni_link_event"
static short int Handle_uni_link_event
                                ( const short int       olt_id, 
                                  const short int       onu_id,
                                  const unsigned char  *receive_buffer,
                                  const unsigned short  length)
{
    bool                uni_link_up;
    unsigned char      *receive_pdu_data = (unsigned char  *)receive_buffer;


    /* Extract uni_link_up field*/
    EXTRACT_BOOL_ENDIANITY_FROM_UBUFFER(receive_pdu_data, uni_link_up, length, receive_buffer)
        

    /* If no handler function assigned for pre-reset event */
    if (event_handler_functions[UNI_LINK_EVENT-IGMP_JOIN_EVENT] == NULL) 
    { 
           char string_buffer[MSG_STRING_MAX_SIZE];

           sprintf ( string_buffer,no_hook_function_s );
           RmDecompose_print(string_buffer);

    } 
    else
    {
    
        /* Call handler function  */
        event_handler_functions[UNI_LINK_EVENT-IGMP_JOIN_EVENT](olt_id, onu_id, uni_link_up);
    }


    return COMPOSER_STATUS_OK;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Handle_image_burn_complete_event"
static short int Handle_image_burn_complete_event
                                ( const short int       olt_id, 
                                  const short int       onu_id,
                                  const unsigned char  *receive_buffer,
                                  const unsigned short  length)
{
    REMOTE_STATUS              image_burn_status;
    unsigned char      *receive_pdu_data = (unsigned char  *)receive_buffer;


    /* Extract image_burn_status field*/
    EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER(receive_pdu_data, image_burn_status, length, receive_buffer)
    
    /* If no handler function assigned for pre-reset event */
    if (event_handler_functions[IMAGE_BURN_COMPLETE_EVENT-IGMP_JOIN_EVENT] == NULL) 
    { 
           char string_buffer[MSG_STRING_MAX_SIZE];

           sprintf ( string_buffer,no_hook_function_s );
           RmDecompose_print(string_buffer);

    } 
    else
    {
    
        /* Call handler function  */
        event_handler_functions[IMAGE_BURN_COMPLETE_EVENT-IGMP_JOIN_EVENT](olt_id, onu_id, image_burn_status);
    }


    return COMPOSER_STATUS_OK;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Handle_alarm_event"
static short int Handle_alarm_event
                            (const unsigned char  *receive_buffer,
                             const unsigned short  length)
{
    PASONU_alarm_event_type     alarm_event_type;
    INT64U                      value, window, threshold ;
    unsigned char              *receive_pdu_data = (unsigned char  *)receive_buffer;

    /* Extract alarm_event_type field*/
    EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER(receive_pdu_data, alarm_event_type, length, receive_buffer)


    /* Extract value field*/
    EXTRACT_ULONG_LONG_ENDIANITY_FROM_UBUFFER(receive_pdu_data, value, length, receive_buffer)
    /* Extract window field*/
    EXTRACT_ULONG_LONG_ENDIANITY_FROM_UBUFFER(receive_pdu_data, window, length, receive_buffer)
    /* Extract threshold field*/
    EXTRACT_ULONG_LONG_ENDIANITY_FROM_UBUFFER(receive_pdu_data, threshold, length, receive_buffer)
        

    /* If no handler function assigned for pre-reset event */
    if (alarm_handler_functions[alarm_event_type] == NULL) 
    { 
           char string_buffer[MSG_STRING_MAX_SIZE];

           sprintf ( string_buffer,no_hook_function_s );
           RmDecompose_print(string_buffer);

    } 
    else
    {
    
        /* Call handler function  */
        alarm_handler_functions[alarm_event_type](alarm_event_type, value, window, threshold);
    }


    return COMPOSER_STATUS_OK;
}
#undef PONLOG_FUNCTION_NAME


static COMPOSER_status_t Compose_assign_handler (
                      const PASONU_alarm_event_type     event_type,
                      const general_handler_function_t  handler)

{
   
    if( event_type < MAX_ALARM_TYPE )
    {
        alarm_handler_functions[event_type] = handler;
        
    }
    else
    {
        event_handler_functions[event_type - IGMP_JOIN_EVENT] = handler;
    }
  
    return COMPOSER_STATUS_OK;
}


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_assign_handler"

PON_STATUS RM_PASONU_assign_handler( 
                                   const PON_alarm_event_type         type,
                                   const general_handler_function_t   handler )
{
    PASONU_alarm_event_type         onu_type;
    short int                       result,composer_result;
    unsigned long                   expected_opcode;
    
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    REMOTE_PASONU_CHECK_INIT
    PON_NO_OLT_HANDLE_ERROR_CHECK_IN_BOUNDS(type,type,PON_ERRORED_SYMBOLS_PERIOD_ALARM, (PON_MAX_ALARM_EVENT - 1))

    
    if( (type != PON_IMAGE_BURN_COMPLETE_EVENT) &&
        (type != PON_ADDRESS_TBL_FULL_EVENT) &&
        (type != PON_UNI_LINK_EVENT) &&
        (type != PON_PRE_RESET_EVENT) )
    {
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "event:%d is not supported\n", type)
        return REMOTE_PASONU_NOT_IMPLEMENTED;
        
    }

              
    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_DEVICE_OPCODES,
                                         ONU_HPROT_SET_EVENT_CONFIGURATION_OPCODE )


    /* begin OLT to ONU types conversion */
    result = Olt_to_onu_converstion_event(type, &onu_type);
    if ( result != REMOTE_PASONU_EXIT_OK )
    { 
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "failed. error code:%d\n", result)\
        return result;
    }
   
    /* end OLT to ONU types conversion */
    
    composer_result = (short int)Compose_assign_handler( onu_type, handler );
    result = Convert_composer_error_code_to_remote_management(composer_result);
    if ( result != REMOTE_PASONU_EXIT_OK )
    { 
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "failed. error code:%d\n", result)
        return result;
    }

    return result;

}
#undef PONLOG_FUNCTION_NAME


void PAS_Received_event  ( 
                      const short int        olt_id,
                      const PON_onu_id_t     onu_id,
                      const unsigned short   length,
                      const unsigned char   *content)
{
    short int  result = COMPOSER_STATUS_OK;
	/* increment in BYTES_IN_WORD since the opcode is the first short, the app_id 
	   is the second short */
    short int *application_id = (short int*)(content+BYTES_IN_WORD);
	
	

    /* call the decompose function if application id is of the remote management else
       check if there is assign to the other application */
    if ( (*application_id) == REMOTE_PASONU_APPLICATION_ID) 
    {
        result = (short int)Decompose_event(olt_id, onu_id, (unsigned char *)content, length);
	}
    else
    {
        if ( ((*application_id) >= MIN_APPLICATION_ID) && 
             ((*application_id) <= MAX_APPLICATION_ID) &&
             (received_event_func[(*application_id)] != NULL)
           ) 
        {
            result = received_event_func[(*application_id)](olt_id, onu_id, length,(unsigned char *)content);
        }
    }
}

#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_assign_received_event_handler"
PON_STATUS RM_PASONU_assign_received_event_handler(
                                   const short int                             application_id,
                                   const REMOTE_PASONU_received_event_func_t   handler )
{
    PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    REMOTE_PASONU_CHECK_INIT
              
    PON_NO_OLT_HANDLE_ERROR_CHECK_IN_BOUNDS(application_id,application_id,MIN_APPLICATION_ID,MAX_APPLICATION_ID);

    received_event_func[application_id] = handler;

    return REMOTE_PASONU_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME


#endif

