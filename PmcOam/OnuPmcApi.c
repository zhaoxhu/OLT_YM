/*                   Copyright(c) 2008-2012 GWTT, Inc.                    */
/*  
**  OnuPmcApi.c -  C file for PMC RemoteManagement  API Interface 
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
#include "OnuPmcHandler.h"
#include "OnuPmcApi.h"

/*================================ Macros ===================================*/
#define RECEIVED_APPLICATION_BUFFER received_oam_pdu_data+0

#define REMOTE_PASONU_CHECK_ONU_VALIDITY \
    { \
        PON_onu_versions                onu_versions; \
        short int                       result; \
        unsigned short                  oam_frame_length;\
        result = PON_PAS_APPLICATIONS_get_onu_versions(olt_id, onu_id, &onu_versions); \
        if( result != REMOTE_PASONU_EXIT_OK ) \
        { \
            PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "Error getting OLT: %d ONU: %d hardware version. error code:%d\n", olt_id,onu_id,result); \
            return result; \
        } \
        REMOTE_PASONU_CHECK_HW_VERSION(onu_versions.hardware_version) \
        REMOTE_PASONU_CHECK_FW_REMOTE_MGNT_VERSION(onu_versions.fw_remote_mgnt_major_version) \
        REMOTE_PASONU_CHECK_OAM_VERSION(onu_versions.oam_version) \
		if( PON_PAS_APPLICATIONS_get_oam_frame_size_valid(olt_id, onu_id) == FALSE)\
		{\
			PON_PAS_APPLICATIONS_set_oam_frame_size_valid( olt_id, onu_id, TRUE);\
			result = RM_PASONU_get_oam_frame_length_negotiation(olt_id, onu_id, &oam_frame_length);\
			if ( result != REMOTE_PASONU_EXIT_OK )\
			{\
    			PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT %d ONU %d failed in RM_get_oam_frame_length_negotiation for receiving auto negotiation length. error code:%d\n", olt_id, onu_id, result);\
				PON_PAS_APPLICATIONS_set_oam_frame_size(olt_id, onu_id, PASCOMM_ONU_FIRMWARE_BINARY_CHUNK_DEFAULT_SIZE);\
			}\
			else\
			{\
				PON_PAS_APPLICATIONS_set_oam_frame_size(olt_id, onu_id, oam_frame_length);\
			}\
		}\
    }

#define REMOTE_PASONU_CHECK_ONU_VALIDITY_WITHOUT_REMOTE_PROTOCOL \
    { \
        PON_onu_versions                onu_versions; \
        short int                       result; \
        unsigned short                  oam_frame_length;\
        result = PON_PAS_APPLICATIONS_get_onu_versions(olt_id, onu_id, &onu_versions); \
        if( result != REMOTE_PASONU_EXIT_OK ) \
        { \
            PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "Error getting OLT: %d ONU: %d hardware version. error code:%d\n", olt_id,onu_id,result); \
            return result; \
        } \
        REMOTE_PASONU_CHECK_HW_VERSION(onu_versions.hardware_version) \
        REMOTE_PASONU_CHECK_OAM_VERSION(onu_versions.oam_version) \
        REMOTE_PASONU_CHECK_INIT\
		if( PON_PAS_APPLICATIONS_get_oam_frame_size_valid(olt_id, onu_id) == FALSE)\
		{\
			PON_PAS_APPLICATIONS_set_oam_frame_size_valid( olt_id, onu_id, TRUE);\
			result = RM_PASONU_get_oam_frame_length_negotiation(olt_id, onu_id, &oam_frame_length);\
			if ( result != REMOTE_PASONU_EXIT_OK )\
			{\
    			PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT %d ONU %d failed in RM_get_oam_frame_length_negotiation for receiving auto negotiation length. error code:%d\n", olt_id, onu_id, result);\
				PON_PAS_APPLICATIONS_set_oam_frame_size(olt_id, onu_id, PASCOMM_ONU_FIRMWARE_BINARY_CHUNK_DEFAULT_SIZE);\
			}\
			else\
			{\
				PON_PAS_APPLICATIONS_set_oam_frame_size(olt_id, onu_id, oam_frame_length);\
			}\
		}\
    }

#define REMOTE_PASONU_CHECK_HW_VERSION(__hw_version__) \
        if ((__hw_version__ != PON_PASSAVE_ONU_6201_VERSION) &&  (__hw_version__ != PON_PASSAVE_ONU_6301_VERSION) &&  (__hw_version__ != PON_PASSAVE_ONU_6300_VERSION) \
		&& (__hw_version__ != PON_PASSAVE_ONU_6401_VERSION) &&  (__hw_version__ != PON_PASSAVE_ONU_6402_VERSION) &&  (__hw_version__ != PON_PASSAVE_ONU_6403_VERSION) \
		&& (__hw_version__ != PON_PASSAVE_ONU_6501_VERSION) &&  (__hw_version__ != PON_PASSAVE_ONU_6502_VERSION) &&  (__hw_version__ != PON_PASSAVE_ONU_6503_VERSION)) \
        { \
           PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "ERROR in OLT:%d ONU:%d device id:0x%x is not remote management supported.\n",olt_id, onu_id, __hw_version__); \
           return REMOTE_PASONU_NOT_IMPLEMENTED; \
        } \

  
#define REMOTE_PASONU_CHECK_FW_REMOTE_MGNT_VERSION(__fw_remote_mgnt_major_version__) \
        if (__fw_remote_mgnt_major_version__ != REMOTE_PASONU_MAJOR_VERSION ) \
        { \
           PONLOG_ERROR_4(PONLOG_FUNC_GENERAL, olt_id, onu_id, "ERROR in OLT:%d ONU:%d mismatch protocol version.onu protocol version:%d, remote package protocol version:%d.\n",olt_id, onu_id,__fw_remote_mgnt_major_version__,REMOTE_PASONU_MAJOR_VERSION); \
           return REMOTE_PASONU_EXIT_ERROR; \
        } \

#define REMOTE_PASONU_CHECK_OAM_VERSION(__oam_version__) \
        if (__oam_version__ != OAM_STANDARD_VERSION_3_3 ) \
        { \
           PONLOG_ERROR_4(PONLOG_FUNC_GENERAL, olt_id, onu_id, "ERROR in OLT:%d ONU:%d not supported OAM version.onu OAM version:%d, remote package OAM version supported:%d.\n",olt_id, onu_id, __oam_version__, OAM_STANDARD_VERSION_3_3); \
           return REMOTE_PASONU_NOT_IMPLEMENTED; \
        } \


#define REMOTE_PASONU_CHECK_ONU_HW_VERSION \
    { \
        PON_onu_versions                onu_versions; \
        short int                       result; \
        result = PON_PAS_APPLICATIONS_get_onu_versions(olt_id, onu_id, &onu_versions); \
        if( result != REMOTE_PASONU_EXIT_OK ) \
        { \
            PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "Error getting OLT: %d ONU: %d hardware version. error code:%d\n", olt_id,onu_id,result); \
            return result; \
        } \
        if ((onu_versions.hardware_version != PON_PASSAVE_ONU_6201_VERSION) && (onu_versions.hardware_version != PON_PASSAVE_ONU_6301_VERSION) && (onu_versions.hardware_version != PON_PASSAVE_ONU_6300_VERSION)\
		&& (onu_versions.hardware_version != PON_PASSAVE_ONU_6401_VERSION) && (onu_versions.hardware_version != PON_PASSAVE_ONU_6402_VERSION) && (onu_versions.hardware_version != PON_PASSAVE_ONU_6403_VERSION) \
		&& (onu_versions.hardware_version != PON_PASSAVE_ONU_6501_VERSION) && (onu_versions.hardware_version != PON_PASSAVE_ONU_6502_VERSION) && (onu_versions.hardware_version != PON_PASSAVE_ONU_6503_VERSION)) \
        { \
           PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "ERROR in OLT:%d ONU:%d device id:0x%x is not remote management supported\n",olt_id, onu_id, onu_versions.hardware_version); \
           return REMOTE_PASONU_NOT_IMPLEMENTED; \
        } \
   }

#define REMOTE_MANAGEMENT_CHECK_ONU_FOR_OLD_COMMANDS(__old_code__) \
    { \
        PON_onu_versions                onu_versions;\
        result = PON_PAS_APPLICATIONS_get_onu_versions(olt_id, onu_id, &onu_versions); \
        if( result != REMOTE_PASONU_EXIT_OK ) \
        { \
            PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "Error getting OLT: %d ONU: %d hardware version. error code:%d\n", olt_id,onu_id,result); \
            return result; \
        } \
        if( ( onu_versions.hardware_version == PON_PASSAVE_ONU_6001_VERSION) || (!pon_pmc_remote_management_is_init) ) \
        { \
            __old_code__ = TRUE; \
        } \
        else if(((onu_versions.hardware_version == PON_PASSAVE_ONU_6201_VERSION) || (onu_versions.hardware_version == PON_PASSAVE_ONU_6301_VERSION) || (onu_versions.hardware_version == PON_PASSAVE_ONU_6300_VERSION) \
		|| (onu_versions.hardware_version == PON_PASSAVE_ONU_6401_VERSION) || (onu_versions.hardware_version == PON_PASSAVE_ONU_6402_VERSION) || (onu_versions.hardware_version == PON_PASSAVE_ONU_6403_VERSION) \
		|| (onu_versions.hardware_version == PON_PASSAVE_ONU_6501_VERSION) || (onu_versions.hardware_version == PON_PASSAVE_ONU_6502_VERSION) || (onu_versions.hardware_version == PON_PASSAVE_ONU_6503_VERSION)) \
			&& (onu_versions.oam_version == OAM_STANDARD_VERSION_2_0))\
        { \
            REMOTE_PASONU_CHECK_INIT\
            REMOTE_PASONU_CHECK_FW_REMOTE_MGNT_VERSION(onu_versions.fw_remote_mgnt_major_version)\
            __old_code__ = TRUE; \
        } \
        else \
        { \
            __old_code__ = FALSE; \
        } \
    }



#define REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY \
    { \
        REMOTE_PASONU_CHECK_INIT\
        REMOTE_PASONU_CHECK_ONU_VALIDITY\
    }

#define CHECK_REMOTE_PASONU_RESULT_AND_RETURN \
    if ( result != REMOTE_PASONU_EXIT_OK ) \
{ \
    PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)\
    return result; \
}


#define CHECK_COMPOSER_RESULT_AND_RETURN_IF_NO_MEMORY \
{   result = Convert_composer_error_code_to_remote_management(composer_result);\
    if ( (result != REMOTE_PASONU_EXIT_OK) && (result != REMOTE_PASONU_MEMORY_ERROR) )\
{ \
    PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)\
    return result;\
}\
}

#define CHECK_RM_SOFT_AND_OLT_VALIDITY(olt_id) \
{ \
	short int check_result;\
	check_result = Check_rm_soft_and_olt_validity ( olt_id );\
	if (check_result != EXIT_OK) return (check_result);\
}


#define CHECK_OLT_EXISTS(__function_name__)\
	if (!OLTAdv_IsExist ( olt_id ))\
	{\
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT, "Error, OLT id (%d) doesn't exist. Call Pon_add_olt before calling this function\n", \
					 olt_id);\
		return (PAS_OLT_NOT_EXIST);\
	}

/*=============================== Constants =================================*/
#define __MODULE__ "ONU_PMC"        /* Module name for log headers */
#define ___FILE___ "pmc_api" /* File name for log headers */

/*============================== Data Types =================================*/
typedef struct
{
    unsigned short index;
    unsigned short hash;
} PON_address_table_iterator_t;

/*========================== External Variables =============================*/
/*========================== External Functions =============================*/
/*=============================== Variables =================================*/
/*======================= Internal Functions Prototype ======================*/
/*================================ Functions ================================*/

#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_get_device_version"

PON_STATUS RM_PASONU_get_device_version( 
                                   const PON_olt_id_t     olt_id, 
                                   const PON_onu_id_t     onu_id,
                                   PON_device_version_t  *versions)
{
    PASONU_version_t                onu_version;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

#if 1
    REMOTE_PASONU_CHECK_INIT
#else    
	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
#endif              
   

    composer_result = (short int)Compose_get_version
                            ( sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_get_version( RECEIVED_APPLICATION_BUFFER, received_length, &onu_version );
    CHECK_COMPOSER_RESULT_AND_RETURN

    /* begin ONU to OLT types conversion */
    result = Onu_to_olt_converstion_device_version(&onu_version,versions);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end ONU to OLT types conversion */
    
    return result;

}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_get_device_status"

PON_STATUS RM_PASONU_get_device_status( 
                                   const PON_olt_id_t        olt_id, 
                                   const PON_onu_id_t        onu_id,
                                   PON_onu_device_status_t  *status)
{
    PASONU_status_t                 onu_status;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
              
  

    composer_result = (short int)Compose_get_status
                            ( sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_get_status( RECEIVED_APPLICATION_BUFFER, received_length, &onu_status );
    CHECK_COMPOSER_RESULT_AND_RETURN


    /* begin ONU to OLT types conversion */
    result = Onu_to_olt_converstion_device_status(&onu_status, status);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end ONU to OLT types conversion */
    
    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_set_slow_protocol_limit"

PON_STATUS RM_PASONU_set_slow_protocol_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable)
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_CHECK_BOOL_VARIABLE_IN_BOUNDS(enable,enable)
              
    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_OAM_OPCODES,
                                         ONU_HPROT_SET_SLOW_PROTOCOL_LIMIT_OPCODE )

    composer_result = (short int)Compose_set_slow_protocol_limit
                            ( (BOOLEAN)enable, sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_get_slow_protocol_limit"

PON_STATUS RM_PASONU_get_slow_protocol_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   bool                *enable)
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
              
 
    composer_result = (short int)Compose_get_slow_protocol_limit( sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_get_slow_protocol_limit( RECEIVED_APPLICATION_BUFFER, received_length, (BOOLEAN*)enable );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_get_oam_version"

PON_STATUS RM_PASONU_get_oam_version( 
                                   const PON_olt_id_t       olt_id, 
                                   const PON_onu_id_t       onu_id,
                                   OAM_standard_version_t  *version)
{
    PASONU_oam_version_t            onu_version;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
   
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
              
 
    composer_result = (short int)Compose_get_oam_version
                            ( sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_get_oam_version( RECEIVED_APPLICATION_BUFFER, received_length, &onu_version);
    CHECK_COMPOSER_RESULT_AND_RETURN


    /* begin ONU to OLT types conversion */
    result = Onu_to_olt_converstion_oam_version(onu_version, version);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end ONU to OLT types conversion */

    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_get_oam_frame_length_negotiation"

PON_STATUS RM_PASONU_get_oam_frame_length_negotiation( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   unsigned short      *oam_frame_length )
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY

                  
    composer_result = (short int)Compose_oam_get_length_negotiation
                            ( sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_oam_get_length_negotiation
                                                ( RECEIVED_APPLICATION_BUFFER, received_length, 
                                                  (INT16U*)oam_frame_length );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_reset_device"

PON_STATUS RM_PASONU_reset_device ( 
                          const PON_olt_id_t        olt_id, 
                          const PON_onu_id_t        onu_id,
                          const PON_reset_reason_t  reason)
{
    short int                result, composer_result;
    unsigned short           sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char            sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char            received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long            expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);


    REMOTE_PASONU_CHECK_ONU_VALIDITY_WITHOUT_REMOTE_PROTOCOL

    

    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_DEVICE_OPCODES,
                                         ONU_HPROT_RESET_DEVICE_OPCODE )
    
    composer_result = (short int)Compose_reset_device
                                    ( (PASONU_reset_reason_t) reason, 
                                      sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                    received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;


}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_set_alarm_configuration"

PON_STATUS RM_PASONU_set_alarm_configuration( 
                                   const PON_olt_id_t          olt_id, 
                                   const PON_onu_id_t          onu_id,
                                   const PON_alarm_event_type  type,
                                   const bool                  enable_hook_function,
                                   const unsigned__int64       window,
                                   const unsigned__int64       threshold)
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(type,type,PON_ERRORED_SYMBOLS_PERIOD_ALARM, (PON_MAX_ALARM_EVENT - 1))
    PON_CHECK_BOOL_VARIABLE_IN_BOUNDS(enable_hook_function,enable_hook_function)

                  
    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_OAM_OPCODES,
                                         ONU_HPROT_SET_ALARM_CONFIGURATION_OPCODE )


    composer_result = (short int)Compose_set_alarm_configuration
                            ( (PASONU_alarm_event_type)type, (BOOLEAN)enable_hook_function, 
                              window, threshold, sent_oam_pdu_data+APPLICATION_HEADER_PLACE, 
                              &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_get_alarm_configuration"

PON_STATUS RM_PASONU_get_alarm_configuration( 
                                   const PON_olt_id_t           olt_id, 
                                   const PON_onu_id_t           onu_id,
                                   const PON_alarm_event_type   type,
                                   bool                        *enable_hook_function,
                                   unsigned__int64             *window,
                                   unsigned__int64             *threshold)
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(type,type,PON_ERRORED_SYMBOLS_PERIOD_ALARM, (PON_MAX_ALARM_EVENT - 1))

                  
 
    composer_result = (short int)Compose_get_alarm_configuration
                            ( (PASONU_alarm_event_type)type, sent_oam_pdu_data+APPLICATION_HEADER_PLACE,  &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_get_alarm_configuration
                                    ( RECEIVED_APPLICATION_BUFFER, received_length,
                                      (BOOLEAN*)enable_hook_function, window,threshold );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_clear_statistics"

PON_STATUS RM_PASONU_clear_all_statistics( 
                                   const PON_olt_id_t  olt_id, 
                                   const PON_onu_id_t  onu_id)
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
              
    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_OAM_OPCODES,
                                         ONU_HPROT_CLEAR_STATISTIC_OPCODE )

    composer_result = (short int)Compose_clear_all_statistics
                            ( FALSE, 0, sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN

#if 0
	result = TkAdapter_PAS5201_set_onu_statistic_monitoring_state(olt_id, onu_id, DISABLE);
	if (result != 0) 
    {
        PONLOG_ERROR_2(PONLOG_FUNC_GENERAL, olt_id, onu_id, "Error, Can't set statistic monitoring state of OLT %d ONU %d\n",olt_id,onu_id);
        return (result);
    }

    result = TkAdapter_PAS5201_reset_onu_statistic_table(olt_id,onu_id);
    if (result != 0) 
    {
        PONLOG_ERROR_2(PONLOG_FUNC_GENERAL, olt_id, onu_id, "Error, Can't reset local statistics of OLT %d ONU %d\n",olt_id,onu_id);
        return (result);
    }
#endif

    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


#if 0
	result = TkAdapter_PAS5201_set_onu_statistic_monitoring_state(olt_id, onu_id, ENABLE);
	if (result != 0) 
    {
        PONLOG_ERROR_2(PONLOG_FUNC_GENERAL, olt_id, onu_id, "Error, Can't set statistic monitoring state of OLT %d ONU %d\n",olt_id,onu_id);
        return (result);
    }
#endif


    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_write_onu_eeprom_register"
PON_STATUS RM_PASONU_write_onu_eeprom_register ( 
                              const PON_olt_id_t		olt_id, 
							  const PON_onu_id_t	    onu_id,
							  const short int			register_address, 
							  const unsigned short int  data )
{
    short int                       result, composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    bool                            old_code;
    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    REMOTE_MANAGEMENT_CHECK_ONU_FOR_OLD_COMMANDS(old_code)
    if ( old_code )
    {
        CHECK_RM_SOFT_AND_OLT_VALIDITY(olt_id)

	    CHECK_OLT_EXISTS(write_onu_eeprom_register)

	    result = REMOTE_PASONU_NOT_IMPLEMENTED; 

    }
    else
    {
		REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY

        PON_HANDLE_ERROR_CHECK_IN_BOUNDS(register_address,register_address,
                                         PON_MIN_ONU_EEPROM_REGISTER_ADDRESS,PON_MAX_ONU_EEPROM_REGISTER_ADDRESS)

        
        expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_PERIPH_OPCODES,
                                             ONU_HPROT_EEPROM_WRITE_OPCODE )
    

        composer_result = (short int)Compose_eeprom_write( (INT16U)register_address, (INT16U)data, 
                                           sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
        CHECK_COMPOSER_RESULT_AND_RETURN


        result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                    received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
        CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

        composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
        CHECK_COMPOSER_RESULT_AND_RETURN

    }

    return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_read_onu_eeprom_register"
PON_STATUS RM_PASONU_read_onu_eeprom_register ( 
                                const PON_olt_id_t	 olt_id, 
								const PON_onu_id_t	 onu_id,
								const short	int	     register_address, 
								unsigned short int  *data )
{
    short int                       result, composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    bool                            old_code;
    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    REMOTE_MANAGEMENT_CHECK_ONU_FOR_OLD_COMMANDS(old_code)
    
    if(old_code)
    {
        CHECK_RM_SOFT_AND_OLT_VALIDITY(olt_id)

	    CHECK_OLT_EXISTS(read_onu_eeprom_register)

	    result = REMOTE_PASONU_NOT_IMPLEMENTED; 
  
    }
    else
    {
		REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
             
       PON_HANDLE_ERROR_CHECK_IN_BOUNDS(register_address,register_address,
                                        PON_MIN_ONU_EEPROM_REGISTER_ADDRESS,PON_MAX_ONU_EEPROM_REGISTER_ADDRESS)
       
           
       
       
       composer_result = (short int)Compose_eeprom_read( (INT16U)register_address, sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
       CHECK_COMPOSER_RESULT_AND_RETURN
       
       
       result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                   received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
       CHECK_REMOTE_PASONU_RESULT_AND_RETURN
           
       
       composer_result = (short int)Decompose_eeprom_read( RECEIVED_APPLICATION_BUFFER, received_length, (INT16U*)data );
       CHECK_COMPOSER_RESULT_AND_RETURN   
    }


    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_eeprom_mapper_set_parameter"

PON_STATUS RM_PASONU_eeprom_mapper_set_parameter 
                              ( const PON_olt_id_t		      olt_id, 
								const PON_onu_id_t	          onu_id,
								const EEPROM_mapper_param_t   parameter, 
								const void                   *data,
                                const unsigned long           size)
{
    short int                result, composer_result;
    unsigned short           sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char            sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char            received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long            expected_opcode;
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(parameter,parameter,EEPROM_MAPPER_COOKIE_ID,EEPROM_MAPPER_LAST_PARAM)

        
    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_PERIPH_OPCODES,
                                         ONU_HPROT_EMAPPER_SET_OPCODE )
    

    composer_result = (short int)Compose_eeprom_mapper_set_param( (PASONU_emapper_param_t)parameter, (void*)data, (INT32U)size, 
                                                       sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_eeprom_mapper_get_parameter"

PON_STATUS RM_PASONU_eeprom_mapper_get_parameter 
                              ( const PON_olt_id_t		      olt_id, 
								const PON_onu_id_t	          onu_id,
								const EEPROM_mapper_param_t   parameter, 
                                void                         *data,
								unsigned long                *size)
{
    short int                result, composer_result;
    unsigned short           sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char            sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char            received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long            allocated_size;
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(parameter,parameter,EEPROM_MAPPER_COOKIE_ID,EEPROM_MAPPER_LAST_PARAM)

    allocated_size =  *size;

    composer_result = (short int)Compose_eeprom_mapper_get_param
                                                    ( (PASONU_emapper_param_t)parameter,(INT32U)allocated_size,
                                                       sent_oam_pdu_data+APPLICATION_HEADER_PLACE, 
                                                       &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_eeprom_mapper_get_param( RECEIVED_APPLICATION_BUFFER, received_length,
                                                         (INT32U*)size, (void*)data );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_address_table_clear"

PON_STATUS RM_PASONU_address_table_clear( 
                                   const PON_olt_id_t  olt_id, 
                                   const PON_onu_id_t  onu_id)
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY

                  
    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_ADDR_TABLE_OPCODES,
                                         ONU_HPROT_ADDRTBL_CLEAR_ALL_OPCODE )


    composer_result = (short int)Compose_address_table_clear_all
                                    ( sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_address_table_set_uni_learning"

PON_STATUS RM_PASONU_address_table_set_uni_learning( 
                                   const PON_olt_id_t  olt_id, 
                                   const PON_onu_id_t  onu_id,
                                   const bool          enable)
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_CHECK_BOOL_VARIABLE_IN_BOUNDS(enable,enable)

    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_ADDR_TABLE_OPCODES,
                                         ONU_HPROT_SET_ADDRTBL_UNI_LEARN_OPCODE )


    composer_result = (short int)Compose_address_table_set_uni_learn( (BOOLEAN)enable, sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_address_table_ageing_configuration"

PON_STATUS RM_PASONU_address_table_ageing_configuration( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable,
                                   const unsigned long  max_age)
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY 
    PON_CHECK_BOOL_VARIABLE_IN_BOUNDS(enable,enable)


    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_ADDR_TABLE_OPCODES,
                                         ONU_HPROT_ADDRTBL_AGING_CONFIG_OPCODE )


    composer_result = (short int)Compose_address_table_ageing_configuration
                            ( (BOOLEAN)enable,(INT32U)max_age,sent_oam_pdu_data+APPLICATION_HEADER_PLACE, 
                              &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_get_address_table_ageing_configuration"

PON_STATUS RM_PASONU_get_address_table_ageing_configuration( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   bool                *enable,
                                   unsigned long	   *max_age)
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY            

    composer_result = (short int)Compose_get_address_table_ageing_configuration
                                    ( sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                    received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_get_address_table_ageing_configuration
                            ( RECEIVED_APPLICATION_BUFFER, received_length, (BOOLEAN *)enable, (INT32U *)max_age);
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_address_table_get_number_of_entries"

PON_STATUS RM_PASONU_address_table_get_number_of_entries( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   long                *number_of_entries)
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
 
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY

         

    composer_result = (short int)Compose_address_table_get_entry_num
                            ( sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_address_table_get_entry_num
                                ( RECEIVED_APPLICATION_BUFFER, received_length, number_of_entries );
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Address_table_get_first_element"

static short int Address_table_get_first_element( 
                                   const PON_olt_id_t             olt_id, 
                                   const PON_onu_id_t             onu_id,
                                   PON_address_table_iterator_t  *iter, 
                                   mac_address_t                  mac_address,
                                   PON_forwarding_action_t       *action,
                                   unsigned char                 *age,
                                   bool                          *is_static)
{
    GENERAL_mac_addr_t              onu_mac_address;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
   

    composer_result = (short int)Compose_address_table_get_first_element
                            ( sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_address_table_get_first_element
                           ( RECEIVED_APPLICATION_BUFFER, received_length, 
                             (ADDRTBL_iterator_t*)iter, &onu_mac_address, (PASONU_forwarding_action_t*)action,
                             (INT8U*)age, (BOOLEAN*)is_static);
    CHECK_COMPOSER_RESULT_AND_RETURN
    
        
    /* begin ONU to OLT types conversion */ 
    result = Onu_to_olt_converstion_mac_address( &onu_mac_address, mac_address);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end ONU to OLT types conversion */ 

    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "Address_table_get_next_element"

static short int Address_table_get_next_element( 
                                   const PON_olt_id_t             olt_id, 
                                   const PON_onu_id_t             onu_id,
                                   PON_address_table_iterator_t  *iter, 
                                   mac_address_t                  mac_address,
                                   PON_forwarding_action_t       *action,
                                   unsigned char                 *age,
                                   bool                          *is_static)
{
    GENERAL_mac_addr_t              onu_mac_address;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY

                  
    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_ADDR_TABLE_OPCODES,
                                         ONU_HPROT_GET_ADTBL_NEXT_ELEMENT_OPCODE )

    composer_result = (short int)Compose_address_table_get_next_element
                            ( (ADDRTBL_iterator_t*)iter, sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_address_table_get_next_element
                                    ( RECEIVED_APPLICATION_BUFFER, received_length, 
                                      (ADDRTBL_iterator_t*)iter, &onu_mac_address, (PASONU_forwarding_action_t*)action,
                                      (INT8U*)age, (BOOLEAN*)is_static);
    
    result = Convert_composer_error_code_to_remote_management(composer_result);
    if ( result != REMOTE_PASONU_EXIT_OK )\
    { 
        if(result == REMOTE_PASONU_PARAMETER_ERROR)
        {
            return REMOTE_PASONU_PARAMETER_ERROR;
        }
        else
        {
        
            PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)
            return result;
        }
    }
    
        
    /* begin ONU to OLT types conversion */ 
    result = Onu_to_olt_converstion_mac_address( &onu_mac_address, mac_address);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end ONU to OLT types conversion */ 

    return result;

}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_address_table_get"

PON_STATUS RM_PASONU_address_table_get( 
                                   const PON_olt_id_t               olt_id,
                                   const PON_onu_id_t               onu_id,
                                   long int                        *num_of_entries,
                                   PON_onu_address_table_record_t  *address_table   )
{
    PON_address_table_iterator_t    iter;
    mac_address_t                   mac_address;
    PON_forwarding_action_t         action;
    unsigned char                   age;
    bool                            is_static;
    short int                       result;
    unsigned long                   expected_opcode;
    long int                        i=0;

    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY


    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_ADDR_TABLE_OPCODES,
                                         ONU_HPROT_ADDRTBL_AGING_CONFIG_OPCODE )

   
    (*num_of_entries) = 0;
    result = Address_table_get_first_element
                                        ( olt_id, onu_id, &iter, mac_address, &action, &age, &is_static);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
    

    memcpy( (address_table+i)->addr, mac_address, BYTES_IN_MAC_ADDRESS );
    (address_table+i)->action       = action;
    (address_table+i)->age          = age;
    (address_table+i)->type         = is_static;
    i++;
    (*num_of_entries) = i;


    while(result == REMOTE_PASONU_EXIT_OK)
    {
        result = Address_table_get_next_element
                                ( olt_id, onu_id, &iter, mac_address, &action, &age, &is_static);
        

        if ( result != REMOTE_PASONU_EXIT_OK ) 
        { 
            if ( result == REMOTE_PASONU_PARAMETER_ERROR ) 
            {
                /* end of address table  */
                break;
            }
            else
            {
                PONLOG_ERROR_4(PONLOG_FUNC_GENERAL, olt_id, onu_id, "failed in OLT %d ONU %d. entry number %u error code:%d\n", olt_id, onu_id, i,result)   
            }
            return result; 
        }

        
        memcpy( (address_table+i)->addr, mac_address, BYTES_IN_MAC_ADDRESS );
        (address_table+i)->action       = action;
        (address_table+i)->age          = age;
        (address_table+i)->type         = is_static;
        i++;
        
    }
     

    (*num_of_entries)               = i;
    return REMOTE_PASONU_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_address_table_set_uni_learn_limit"

PON_STATUS RM_PASONU_address_table_set_uni_learn_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const unsigned long  limit)
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
  
                  
    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_ADDR_TABLE_OPCODES,
                                         ONU_HPROT_SET_ADDRTBL_UNI_LEARN_LIMIT_OPCODE )

    composer_result = (short int)Compose_address_table_set_uni_learn_limit
                            (FALSE,
							 0,
							 (INT32U)limit,
                             sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_address_table_get_uni_learn_limit"

PON_STATUS RM_PASONU_address_table_get_uni_learn_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   unsigned long       *limit )
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY

                  
   
    composer_result = (short int)Compose_address_table_get_uni_learn_limit
                            (FALSE, 0, sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_address_table_get_uni_learn_limit
                                        ( FALSE,
									      RECEIVED_APPLICATION_BUFFER, received_length, 
                                          (INT32U*)limit );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_uni_set_port"

PON_STATUS RM_PASONU_uni_set_port( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable_cpu, 
                                   const bool           enable_datapath )
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_CHECK_BOOL_VARIABLE_IN_BOUNDS(enable_cpu,enable_cpu)
    PON_CHECK_BOOL_VARIABLE_IN_BOUNDS(enable_datapath,enable_datapath)

                  
    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_UNI_OPCODES,
                                         ONU_HPROT_SET_UNI_PORT_OPCODE )

    composer_result = (short int)Compose_uni_set_port
                            ( (BOOLEAN)enable_cpu, (BOOLEAN)enable_datapath,
                               sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_uni_get_port"

PON_STATUS RM_PASONU_uni_get_port( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   bool                *enable_cpu, 
                                   bool                *enable_datapath )
{
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY

                  
   
    composer_result = (short int)Compose_uni_get_port
                            ( sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_uni_get_port( RECEIVED_APPLICATION_BUFFER, received_length, 
                                                         (BOOLEAN*)enable_cpu, (BOOLEAN*)enable_datapath);
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_uni_set_port_configuration"

PON_STATUS RM_PASONU_uni_set_port_configuration( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id,
                                   const PON_uni_configuration_t  *uni_configuration )
{
    PASONU_uni_config_t             onu_uni_configuration;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->mac_type,adv_1000_full,PON_MII,PON_TBI)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->mii_rate,adv_1000_full,PON_MII_RATE_10_MBPS,PON_MII_RATE_100_MBPS)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->duplex,adv_1000_full,PON_HALF_DUPLEX,PON_FULL_DUPLEX)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->gmii_master_mode,adv_1000_full,PON_SLAVE,PON_MASTER)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->auto_negotiate,adv_1000_full,AUTO_NEGOTIATION_OFF,AUTO_NEGOTIATION_ON)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->mode,adv_1000_full,PON_SLAVE,PON_MASTER)
    PON_CHECK_BOOL_VARIABLE_IN_BOUNDS(uni_configuration->advertise,adv_1000_full)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->adv_10_half,adv_1000_full,PON_ADVERTISE_NOT_CAPABLE,PON_ADVERTISE_CAPABLE)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->adv_10_full,adv_1000_full,PON_ADVERTISE_NOT_CAPABLE,PON_ADVERTISE_CAPABLE)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->adv_100_half,adv_1000_full,PON_ADVERTISE_NOT_CAPABLE,PON_ADVERTISE_CAPABLE)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->adv_100_full,adv_1000_full,PON_ADVERTISE_NOT_CAPABLE,PON_ADVERTISE_CAPABLE)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->adv_100_t4,adv_1000_full,PON_ADVERTISE_NOT_CAPABLE,PON_ADVERTISE_CAPABLE)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->adv_pause,adv_1000_full,PON_ADVERTISE_NOT_CAPABLE,PON_ADVERTISE_CAPABLE)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->adv_asym_pause,adv_1000_full,PON_ADVERTISE_NOT_CAPABLE,PON_ADVERTISE_CAPABLE)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->adv_1000_half,adv_1000_full,PON_ADVERTISE_NOT_CAPABLE,PON_ADVERTISE_CAPABLE)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->adv_1000_full,adv_1000_full,PON_ADVERTISE_NOT_CAPABLE,PON_ADVERTISE_CAPABLE)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(uni_configuration->adv_port_type,adv_1000_full,ADVERTISE_PORT_SINGLE,ADVERTISE_PORT_MULTI)

  
    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_UNI_OPCODES,
                                         ONU_HPROT_SET_UNI_PORT_CONFIGURATION_OPCODE )

    /* begin OLT to ONU types conversion */
    result = Olt_to_onu_converstion_uni_port_configuration(uni_configuration, &onu_uni_configuration);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end OLT to ONU types conversion */

    composer_result = (short int)Compose_uni_set_port_configuration
                            ( FALSE,
							  0,
							  &onu_uni_configuration,
                              sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_uni_get_port_configuration"

PON_STATUS RM_PASONU_uni_get_port_configuration( 
                                   const PON_olt_id_t        olt_id, 
                                   const PON_onu_id_t        onu_id,
                                   PON_uni_configuration_t  *uni_configuration )
{
    PASONU_uni_config_t             onu_uni_configuration;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY

                  
 
    composer_result = (short int)Compose_uni_get_port_configuration
                            ( FALSE, 0, sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_uni_get_port_configuration
                                                ( FALSE,
												  RECEIVED_APPLICATION_BUFFER, received_length, 
                                                  &onu_uni_configuration );
    CHECK_COMPOSER_RESULT_AND_RETURN

    /* begin ONU to OLT types conversion */
    result = Onu_to_olt_converstion_uni_port_configuration(&onu_uni_configuration, uni_configuration);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end ONU to OLT types conversion */
    
    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_uni_get_port_status"

PON_STATUS RM_PASONU_uni_get_port_status( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   PON_uni_status_t    *uni_status )
{
    PASONU_uni_status_t             onu_uni_status;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY


    composer_result = (short int)Compose_uni_get_port_status
                            ( FALSE, 0, sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length);
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length,
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        
		
    composer_result = (short int)Decompose_uni_get_port_status( FALSE,
																RECEIVED_APPLICATION_BUFFER, received_length, 
                                                                &onu_uni_status );
    
    CHECK_COMPOSER_RESULT_AND_RETURN

    /* begin ONU to OLT types conversion */
    result = Onu_to_olt_converstion_uni_status(&onu_uni_status, uni_status);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end ONU to OLT types conversion */

    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_get_burn_image_complete"

PON_STATUS RM_PASONU_get_burn_image_complete (
                                const PON_olt_id_t   olt_id, 
								const PON_onu_id_t   onu_id,
                                const bool          *complete)
{
    short int                result, composer_result;
    unsigned short           sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char            sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char            received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
        

    composer_result = (short int)Compose_flash_get_burn_image_complete
                                        ( sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_flash_get_burn_image_complete
                                        ( RECEIVED_APPLICATION_BUFFER, received_length, (BOOLEAN*)complete);
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;
}
#undef PONLOG_FUNCTION_NAME




#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_encryption_set_state"
PON_STATUS RM_PASONU_encryption_set_state ( 
                                   const PON_olt_id_t                olt_id, 
                                   const PON_onu_id_t                onu_id,
                                   const PON_encryption_direction_t  direction)
{
    

    ENCRYPTION_type_t        onu_direction;
    short int                result,composer_result;
    unsigned short           sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char            sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char            received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long            expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
       
    
    expected_opcode = MAKE_OPCODE_WORD( ONU_HPROT_GROUP_EPONM_OPCODES,
                                        ONU_HPROT_ENCRYPTION_SET_OPCODE );

    /* begin OLT to ONU types conversion */
    result = Olt_to_onu_converstion_encryption_direction(direction, &onu_direction);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end OLT to ONU types conversion */
    
    composer_result = (short int)Compose_encryption_set_state
                            ( onu_direction, sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_encryption_get_state"

PON_STATUS RM_PASONU_encryption_get_state ( 
                                   const PON_olt_id_t           olt_id, 
                                   const PON_onu_id_t           onu_id,
                                   PON_encryption_direction_t  *direction)
{
    

    ENCRYPTION_type_t        onu_direction;
    short int                result,composer_result;
    unsigned short           sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char            sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char            received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
       
    
    composer_result = (short int)Compose_encryption_get_state
                            ( sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_encryption_get_state
                                    ( RECEIVED_APPLICATION_BUFFER, received_length,
                                      &onu_direction );
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    /* begin ONU to OLT types conversion */
    result = Onu_to_olt_converstion_encryption_direction(onu_direction, direction);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end OLT to OLT types conversion */

    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_encryption_set_key"

PON_STATUS RM_PASONU_encryption_set_key ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   const PON_encryption_key_t         encryption_key,
                                   const PON_encryption_key_index_t   encryption_key_index)
{
    short int                result,composer_result;
	ENCRYPTION_key_t		 onu_encryption_key;
    unsigned short           sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char            sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char            received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long            expected_opcode;

    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(encryption_key_index,encryption_key_index,PON_ENCRYPTION_KEY_INDEX_0,PON_ENCRYPTION_KEY_INDEX_1)  
       
    
    expected_opcode = MAKE_OPCODE_WORD( ONU_HPROT_GROUP_EPONM_OPCODES,
                                        ONU_HPROT_ENCRYPTION_KEY_SET_OPCODE );
    


	/* begin OLT to ONU types conversion */
    result = Olt_to_onu_converstion_encryption_key(encryption_key, &onu_encryption_key);
    /* end OLT to ONU types conversion */

	
    composer_result = (short int)Compose_encryption_set_key
                                        ( &onu_encryption_key , (unsigned char)encryption_key_index,
                                           sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        
    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN

    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_encryption_get_key"

PON_STATUS RM_PASONU_encryption_get_key ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   PON_encryption_key_t               encryption_key,
                                   const PON_encryption_key_index_t   encryption_key_index )
{
    ENCRYPTION_key_t         onu_encryption_key;
    short int                result,composer_result;
    unsigned short           sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char            sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char            received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};


    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(encryption_key_index,encryption_key_index,PON_ENCRYPTION_KEY_INDEX_0,PON_ENCRYPTION_KEY_INDEX_1)  
       
    
    composer_result = (short int)Compose_encryption_get_key( (unsigned char)encryption_key_index,
                                                  sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        
    composer_result = (short int)Decompose_encryption_get_key( RECEIVED_APPLICATION_BUFFER, received_length, &onu_encryption_key );
    CHECK_COMPOSER_RESULT_AND_RETURN
   

    /* begin ONU to OLT types conversion */
    result = Onu_to_olt_converstion_encryption_key(onu_encryption_key, encryption_key);
    /* end ONU to OLT types conversion */

    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_classifier_add_filter"

PON_STATUS RM_PASONU_classifier_add_filter ( 
                const PON_olt_id_t                          olt_id, 
                const PON_onu_id_t                          onu_id,
                const PON_pon_network_traffic_direction_t   direction,
                const PON_frame_qualifier_t                 qualifier,
                const unsigned short                        value,
                const PON_forwarding_action_t               action )
{
    PASONU_direction_t              onu_direction;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(direction,direction,PON_DIRECTION_UPLINK,PON_DIRECTION_UPLINK_AND_DOWNLINK)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(qualifier,qualifier,PON_FRAME_ETHERTYPE,PON_FRAME_IPV4_DA)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(action,action,PON_DONT_PASS,PON_PASS_BOTH)


    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_FRAME_PROCESSING_OPCODES,
                                         ONU_HPROT_ADD_CLASSIFIER_FILTER_OPCODE )

    
    /* begin OLT to ONU types conversion */
    result = Olt_to_onu_converstion_direction(direction, &onu_direction);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN 
    /* end OLT to ONU types conversion */
             
    composer_result = (short int)Compose_classifier_add_filter
                            ( onu_direction, (PASONU_frame_qualifier_t) qualifier, value,
                              (PASONU_forwarding_action_t)action, 
                              sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;


}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_classifier_get_filter"

PON_STATUS RM_PASONU_classifier_get_filter ( 
                const PON_olt_id_t                          olt_id, 
                const PON_onu_id_t                          onu_id,
                const PON_pon_network_traffic_direction_t   direction,
                const PON_frame_qualifier_t                 qualifier,
                const unsigned short                        value,
                PON_forwarding_action_t                    *action )
{
    PASONU_direction_t              onu_direction;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(direction,direction,PON_DIRECTION_UPLINK,PON_DIRECTION_UPLINK_AND_DOWNLINK)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(qualifier,qualifier,PON_FRAME_ETHERTYPE,PON_FRAME_IPV4_DA)


  
    
    /* begin OLT to ONU types conversion */
    result = Olt_to_onu_converstion_direction(direction, &onu_direction);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN 
    /* end OLT to ONU types conversion */
             
    composer_result = (short int)Compose_classifier_get_filter
                            ( onu_direction, (PON_frame_qualifier_t) qualifier, value,
                              sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_classifier_get_filter
                                 ( RECEIVED_APPLICATION_BUFFER, received_length,
                                   (PASONU_forwarding_action_t *) action);
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;


}
#undef PONLOG_FUNCTION_NAME




#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_classifier_remove_filter"

PON_STATUS RM_PASONU_classifier_remove_filter ( 
                const PON_olt_id_t                          olt_id, 
                const PON_onu_id_t                          onu_id,
                const PON_pon_network_traffic_direction_t   direction,
                const PON_frame_qualifier_t                 qualifier,
                const unsigned short                        value )
{
    PASONU_direction_t              onu_direction;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(direction,direction,PON_DIRECTION_UPLINK,PON_DIRECTION_UPLINK_AND_DOWNLINK)
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(qualifier,qualifier,PON_FRAME_ETHERTYPE,PON_FRAME_IPV4_DA)
  

    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_FRAME_PROCESSING_OPCODES,
                                         ONU_HPROT_REMOVE_CLASSIFIER_FILTER_OPCODE )

    
    /* begin OLT to ONU types conversion */
    result = Olt_to_onu_converstion_direction(direction, &onu_direction);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN 
    /* end OLT to ONU types conversion */
             
    composer_result = (short int)Compose_classifier_remove_filter
                            ( onu_direction, (PASONU_frame_qualifier_t) qualifier, value, 
                              sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;


}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_classifier_add_da_filter"

PON_STATUS RM_PASONU_classifier_add_da_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address,
                                   const PON_forwarding_action_t   action )
{
    GENERAL_mac_addr_t              onu_mac_address;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(action,action,PON_DONT_PASS,PON_PASS_BOTH)


    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_FRAME_PROCESSING_OPCODES,
                                         ONU_HPROT_CLASSIFIER_ADD_DA_FILTER_OPCODE )

    
    /* begin OLT to ONU types conversion */ 
    result = Olt_to_onu_converstion_mac_address(address, &onu_mac_address);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end OLT to ONU types conversion */
             
    composer_result = (short int)Compose_classifier_add_da_filter
                            ( &onu_mac_address, (PASONU_forwarding_action_t)action, 
                              sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_classifier_add_source_address_filter"

PON_STATUS RM_PASONU_classifier_add_source_address_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address,
                                   const PON_forwarding_action_t   action )
{
    GENERAL_mac_addr_t              onu_mac_address;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
    PON_HANDLE_ERROR_CHECK_IN_BOUNDS(action,action,PON_DONT_PASS,PON_PASS_BOTH)


    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_FRAME_PROCESSING_OPCODES,
                                         ONU_HPROT_CLASSIFIER_ADD_UP_SA_FILTER_OPCODE )

    
    /* begin OLT to ONU types conversion */ 
    result = Olt_to_onu_converstion_mac_address(address, &onu_mac_address);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end OLT to ONU types conversion */
             
    composer_result = (short int)Compose_classifier_add_up_sa_filter
                            ( &onu_mac_address, (PASONU_forwarding_action_t)action, 
                              sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_classifier_remove_da_filter"

PON_STATUS RM_PASONU_classifier_remove_da_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address)
{
    GENERAL_mac_addr_t              onu_mac_address;
    short int                       result,composer_result;
    unsigned short                  sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char                   sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char                   received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long                   expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
       


    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_FRAME_PROCESSING_OPCODES,
                                         ONU_HPROT_CLASSIFIER_REMOVE_DA_FILTER_OPCODE )

    
    /* begin OLT to ONU types conversion */ 
    result = Olt_to_onu_converstion_mac_address(address, &onu_mac_address);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end OLT to ONU types conversion */
             
    composer_result = (short int)Compose_classifier_remove_da_filter
                            ( &onu_mac_address, sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );
    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_classifier_l3l4_add_filter"

PON_STATUS RM_PASONU_classifier_l3l4_add_filter ( 
                                   const PON_olt_id_t                         olt_id,
                                   const PON_onu_id_t                         onu_id,
                                   const PON_pon_network_traffic_direction_t  direction,
                                   const PON_traffic_qualifier_t              traffic,    
                                   const PON_traffic_address_t                address,    
                                   const unsigned long                        ip_address, 
                                   const unsigned short int                   l4_port_num,
                                   const PON_forwarding_action_t              action )
{
    
    PASONU_direction_t       onu_direction;
    short int                result,composer_result;
    unsigned short           sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char            sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char            received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long            expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
   

    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_FRAME_PROCESSING_OPCODES,
                                         ONU_HPROT_ADD_CLASSIFIER_L3L4_FILTER_OPCODE )
                                         
    /* begin OLT to ONU types conversion */
    result = Olt_to_onu_converstion_direction(direction, &onu_direction);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end OLT to ONU types conversion */
        
    composer_result = (short int)Compose_classifier_add_l3l4_filter
                            ( onu_direction,
                              (PASONU_traffic_qualifier_t)traffic,    
                              (PASONU_traffic_address_t)address,    
                              ip_address, 
                              l4_port_num,
                              (PASONU_forwarding_action_t)action,
                              sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );

    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                    received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
 
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_classifier_l3l4_remove_filter"

PON_STATUS RM_PASONU_classifier_l3l4_remove_filter ( 
                                   const PON_olt_id_t                         olt_id,
                                   const PON_onu_id_t                         onu_id,
                                   const PON_pon_network_traffic_direction_t  direction,
                                   const PON_traffic_qualifier_t              traffic,    
                                   const PON_traffic_address_t                address,    
                                   const unsigned long                        ip_address, 
                                   const unsigned short int                   l4_port_num)
{
    
    PASONU_direction_t       onu_direction;
    short int                result,composer_result;
    unsigned short           sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char            sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char            received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned long            expected_opcode;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
   

    expected_opcode =  MAKE_OPCODE_WORD( ONU_HPROT_GROUP_FRAME_PROCESSING_OPCODES,
                                         ONU_HPROT_REMOVE_CLASSIFIER_L3L4_FILTER_OPCODE )
                                         
    /* begin OLT to ONU types conversion */
    result = Olt_to_onu_converstion_direction(direction, &onu_direction);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end OLT to ONU types conversion */
        
    composer_result = (short int)Compose_classifier_remove_l3l4_filter
                            ( onu_direction,
                              (PASONU_traffic_qualifier_t)traffic,    
                              (PASONU_traffic_address_t)address,    
                              ip_address, 
                              l4_port_num,
                              sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );

    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                    received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_general_ack( expected_opcode, RECEIVED_APPLICATION_BUFFER, received_length );
 
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_classifier_l3l4_get_filter"

PON_STATUS RM_PASONU_classifier_l3l4_get_filter ( 
                                   const PON_olt_id_t                          olt_id,
                                   const PON_onu_id_t                          onu_id,
                                   const PON_pon_network_traffic_direction_t   direction,
                                   const PON_traffic_qualifier_t               traffic,    
                                   const PON_traffic_address_t                 address,    
                                   const unsigned long                         ip_address, 
                                   const unsigned short int                    l4_port_num,
                                   PON_forwarding_action_t                    *action )
{
    
    PASONU_direction_t       onu_direction;
    short int                result,composer_result;
    unsigned short           sent_length=MAX_OAM_PDU_SIZE, received_length;
    unsigned char            sent_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    unsigned char            received_oam_pdu_data[MAX_OAM_PDU_SIZE]= {0};
    
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	REMOTE_PASONU_CHECK_INIT_AND_CHECK_ONU_VALIDITY
   

                                       
    /* begin OLT to ONU types conversion */
    result = Olt_to_onu_converstion_direction(direction, &onu_direction);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN   
    /* end OLT to ONU types conversion */
        
    composer_result = (short int)Compose_classifier_get_l3l4_filter
                            ( onu_direction,
                              (PASONU_traffic_qualifier_t)traffic,    
                              (PASONU_traffic_address_t)address,    
                              ip_address, 
                              l4_port_num,
                              sent_oam_pdu_data+APPLICATION_HEADER_PLACE, &sent_length );

    CHECK_COMPOSER_RESULT_AND_RETURN


    result = PON_PAS_APPLICATIONS_send_receive_message ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                                    received_oam_pdu_data, &received_length, MAX_OAM_PDU_SIZE);
    CHECK_REMOTE_PASONU_RESULT_AND_RETURN
        

    composer_result = (short int)Decompose_classifier_get_l3l4_filter
                            ( RECEIVED_APPLICATION_BUFFER, received_length, (PASONU_forwarding_action_t *) action);
    CHECK_COMPOSER_RESULT_AND_RETURN
        
    return result;

}
#undef PONLOG_FUNCTION_NAME



