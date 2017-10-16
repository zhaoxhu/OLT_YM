/*                   Copyright(c) 2008-2012 GWTT, Inc.                    */
/*  
**  OnuPmcComm.c -  C file for PMC RemoteManagement  Comm Implement 
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
#define PMC_MEM_DYNAMIC

#define PON_OLT_ID_ARRAY_SIZE          (8)         /* 目前只有BCM8PON板需要此PMC协议栈 */

/*=============================== Constants =================================*/
#define __MODULE__ "ONU_PMC"        /* Module name for log headers */
#define ___FILE___ "pmc_comm" /* File name for log headers */


#define COMM_SYNC_DB_MAX_SIZE 100
#define REMOTE_MANAGEMENT_RESPONSE_RESEND_LIMIT  3   


#define ETH_HEADER_SIZE             14
#define ETH_OAM_MIN_DATA_SIZE       MIN_ETHERNET_FRAME_SIZE-ETH_HEADER_SIZE

/* ethernet frame header fields - take from pon_pon_expo TBD */
#define ETH_DA_BEGIN_PLACE          ETHERNET_HEADER_PLACE+ETHERNET_FRAME_DESTINATION_ADDRESS_BEGINNING_PLACE
#define ETH_SA_BEGIN_PLACE          ETHERNET_HEADER_PLACE+ETHERNET_FRAME_SOURCE_ADDRESS_BEGINNING_PLACE
#define ETH_TYPE_BEGIN_PLACE        ETHERNET_HEADER_PLACE+ETHERNET_FRAME_TYPE_BEGINNING_PLACE


#define OUI_DEFINE                      0x000CD5

static OAM_oui_t internal_pmc_oui = {0x00, 0x0c, 0xd5};

static unsigned char oam_destination_address[BYTES_IN_MAC_ADDRESS] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x02 };
static unsigned short const_ethernet_type = 0x8809;
static unsigned char const_oam_sub_type  =  0x03 ;
static unsigned char const_oam_code      =  0xFE ;
static unsigned short const_oam_flags    =  0x50 ;


typedef enum
{
    REMOTE_DB_STATE_NOT_VALID,
    REMOTE_DB_STATE_RECEIVED_FRAME,
    REMOTE_DB_STATE_WAITING,
    REMOTE_DB_STATE_RECEIVED_LARGE_FRAME,
}remote_mngmt_db_state_t;


/*============================== Data Types =================================*/

/*  olt message data and all its onus message data */
typedef struct 
{
	unsigned short		maximum_data_frame_size;
    short int			device_version; 
    short int           fw_remote_mgnt_major_version;  /* FW remote management major version */
    short int           fw_remote_mgnt_minor_version;  /* FW remote management minor version */
    unsigned char       oam_version;                   /* ONU OAM version */
	bool				oam_negotiation_field_is_valid;
} onu_info_t;


typedef enum
{

   PDU_TYPE_REQUEST   = 0x00,
   PDU_TYPE_RESPONSE  = 0x01,
   PDU_TYPE_EVENT     = 0x02

} PDU_TYPE_t;

 
typedef struct
{
    unsigned long   message_id;
    unsigned short  pdu_type;

} REMOTE_MANAGEMENT_link_layer_msg_t;

typedef struct
{
    unsigned char   sub_type    ;  
    unsigned short  flags       ;  
    unsigned char   code        ; 
    unsigned char   oui_opcode_1;
    unsigned char   oui_opcode_2;
    unsigned char   oui_opcode_3;
    unsigned char   legacy_number;

} REMOTE_MANAGEMENT_oam_layer_msg_t;


typedef struct  
{
    remote_mngmt_db_state_t      entry_state                        ;
    unsigned char               *received_data                      ;
    unsigned short              *received_data_size                 ;
    unsigned short               allocated_received_data_size       ;

}comm_sync_database_entry_t;


/* Frame received handler
**
** An event indicating Ethernet frame received from remote destination (PON).
** The frame is formatted as a valid Ethernet frame, excluding Preamble, SFD, and FCS fields.
** The frame may be an OAM frame, which have an OAM code not managed by PAS-SFOT.
** Frame content shouldn't be freed by the handler function.
**
** Input Parameters:
**		olt_id	 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid	 : Source LLID, 
**				   range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT  - Source LLID
**					      PON_LLID_NOT_AVAILABLE					   - LLID not available+
**		length	 : Frame length in bytes (excluding Preamble, SFD, and FCS), 
**				   range: MIN_ETHERNET_FRAME_SIZE - MAX_ETHERNET_FRAME_SIZE
**		content  : Pointer to the first byte of the frame
**
** Return codes:
**		Return codes are not specified for an event
**
*/
typedef void (*REMOTE_MNGT_frame_received_handler_t) 
                            ( const short int	     olt_id,
							  const PON_llid_t       llid,
							  const unsigned short   length, 
							  const void		    *content );

/*========================== External Variables =============================*/
/*========================== External Functions =============================*/
/*=============================== Variables =================================*/
static REMOTE_MNGT_frame_received_handler_t global_handler_function;
static REMOTE_MNGT_frame_received_handler_t received_eth_frame_handler;
static general_handler_function_t      received_frame_handler;

static unsigned short frame_received_handler_id;

static unsigned short registration_handler_id;
static unsigned short deregistration_handler_id;

/* Array type to hold all the data to all the possible onus in the system */
#ifdef PMC_MEM_DYNAMIC
static onu_info_t  (*onu_info)[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
#else
static onu_info_t  onu_info [PON_OLT_ID_ARRAY_SIZE][PON_ONU_ID_PER_OLT_ARRAY_SIZE];
#endif

ULONG        msg_id_semaphore;

static general_handler_function_t response_handler_function;
static general_handler_function_t event_handler_function;


ULONG        remote_mngt_semaphore;

#ifdef PMC_MEM_DYNAMIC
comm_sync_database_entry_t  *comm_sync_database;

static unsigned long   (*internal_event_id)[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
#else
comm_sync_database_entry_t   comm_sync_database[COMM_SYNC_DB_MAX_SIZE];

static unsigned long   internal_event_id[PON_OLT_ID_ARRAY_SIZE][PON_ONU_ID_PER_OLT_ARRAY_SIZE];
#endif
static unsigned long   internal_msg_id = 0;

static general_handler_function_t      event_received_handler_function;

/*======================= Internal Functions Prototype ======================*/

static int REMOTE_COMM_send_receive_message  ( 
                      const PON_olt_id_t      olt_id, 
                      const PON_onu_id_t      onu_id,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size,
                      unsigned char          *received_data,
                      unsigned short         *received_data_size,
                      unsigned short          allocated_received_data_size);
static short int COMM_LLC_get_link_layer_message_id  
                    ( const PON_olt_id_t   olt_id, 
                      const PON_onu_id_t   onu_id,
                      unsigned long       *msg_id);
static short int COMM_LLC_send_frame  ( 
                      const PON_olt_id_t      olt_id, 
                      const PON_onu_id_t      onu_id,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size,
                      unsigned long           msg_id);
static short int COMM_OAM_send_frame  ( 
                      const PON_olt_id_t      olt_id, 
                      const PON_onu_id_t      onu_id,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size);
static short int COMM_ETHERNET_send_frame  ( 
                      const PON_olt_id_t      olt_id, 
                      const PON_onu_id_t      onu_id,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size);

static short int Init_ethernet_oam_header ( const PON_olt_id_t   olt_id, 
									        unsigned char       *buffer );
static int Print_remote_msg (
						  const PON_olt_id_t            olt_id, 
						  const PON_onu_id_t            onu_id,
						  unsigned char                *sent_data, 
						  const unsigned short          sent_data_size,
						  bool                          send_flag);

static short int Received_message  ( 
                                    const unsigned short   length,
                                    const unsigned char   *content,
                                    const unsigned long    msg_id );
static short int Received_event  ( 
                                    const short int        olt_id,
                                    const PON_onu_id_t     onu_id,
                                    const unsigned short   length,
                                    const unsigned char   *content);

static short int Insert_to_db
                ( const unsigned long    message_id, 
                  const unsigned char   *received_data, 
                  const unsigned short  *received_data_size,
                  const unsigned short   allocated_received_data_size);
static short int Update_db
                ( const unsigned long    message_id, 
                  const unsigned char   *received_data, 
                  const unsigned short  *received_data_size);
static short int Remove_from_db( const unsigned long   message_id);
static remote_mngmt_db_state_t Get_entry_state_db( const unsigned long       message_id);

static void Receive_link_layer_frame  ( 
                                    const short int        olt_id,
                                    const PON_onu_id_t     onu_id,
                                    const unsigned short   length,
                                    const unsigned char   *content);
static short int COMM_LLC_init( general_handler_function_t response_handler,
                         general_handler_function_t event_handler );
static short int COMM_LLC_terminate( );

static void Receive_oam_layer_frame(  
                                  const short int        olt_id,
                                  const PON_onu_id_t     onu_id,
                                  const unsigned short   length,
                                  const unsigned char   *content);
static short int COMM_OAM_init( general_handler_function_t handler );
static short int COMM_OAM_terminate( );

static void Receive_ethernet_layer_frame (
                                 const short int                 olt_id,
                                 const PON_llid_t                llid,
                                 const unsigned short            length,
                                 const void                     *content );
static short int COMM_ETHERNET_init( REMOTE_MNGT_frame_received_handler_t handler );
static short int COMM_ETHERNET_terminate(  );

/*================================ Functions ================================*/

#if 1
/* Application Layer */
  
static short int Reset_db()
{
#ifdef PMC_MEM_DYNAMIC
    RM_MEM_ZERO(onu_info, sizeof(onu_info_t) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE);
#else
    RM_MEM_ZERO(onu_info, sizeof(onu_info));
#endif

    return REMOTE_PASONU_EXIT_OK;
}

short int PON_PAS_APPLICATIONS_init()
{
#ifdef PMC_MEM_DYNAMIC
    if ( NULL == (onu_info = VOS_Malloc(sizeof(onu_info_t) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE, MODULE_RPU_OAM)) )
    {
        VOS_ASSERT(0);
        return REMOTE_PASONU_MEMORY_ERROR;
    }
#endif

    Reset_db();
  
    return REMOTE_PASONU_EXIT_OK;
}

short int PON_PAS_APPLICATIONS_terminate()
{
    Reset_db();

    return REMOTE_PASONU_EXIT_OK;
}


short int PON_PAS_APPLICATIONS_get_onu_deviceid 
                                  ( const PON_olt_id_t  	olt_id, 
									const PON_onu_id_t	    onu_id,
									PON_onu_hardware_version_t  *device_id )
{
    onu_info_t *pstOnuInfo = &onu_info[olt_id][onu_id];

    if ( 0 != pstOnuInfo->device_version )
    {
        *device_id = pstOnuInfo->device_version;

        return REMOTE_PASONU_EXIT_OK;
    }

    return REMOTE_PASONU_ONU_NOT_AVAILABLE;
}

short int PON_PAS_APPLICATIONS_get_onu_versions 
                                  ( const PON_olt_id_t  	olt_id, 
									const PON_onu_id_t	    onu_id,
									PON_onu_versions  *device_versions )
{
    onu_info_t *pstOnuInfo = &onu_info[olt_id][onu_id];

    if ( 0 != pstOnuInfo->device_version )
    {
        device_versions->hardware_version = pstOnuInfo->device_version;
        device_versions->oam_version = pstOnuInfo->oam_version;
        device_versions->fw_remote_mgnt_major_version = pstOnuInfo->fw_remote_mgnt_major_version;
        device_versions->fw_remote_mgnt_minor_version = pstOnuInfo->fw_remote_mgnt_minor_version;

        return REMOTE_PASONU_EXIT_OK;
    }

    return REMOTE_PASONU_ONU_NOT_AVAILABLE;
}

short int PON_PAS_APPLICATIONS_set_onu_versions 
                                  ( const PON_olt_id_t  	olt_id, 
									const PON_onu_id_t	    onu_id,
									PON_onu_versions  *device_versions )
{
    onu_info_t *pstOnuInfo = &onu_info[olt_id][onu_id];

    pstOnuInfo->device_version = device_versions->hardware_version;
    pstOnuInfo->oam_version = device_versions->oam_version;
    pstOnuInfo->fw_remote_mgnt_major_version = device_versions->fw_remote_mgnt_major_version;
    pstOnuInfo->fw_remote_mgnt_minor_version = device_versions->fw_remote_mgnt_minor_version;

    return REMOTE_PASONU_EXIT_OK;
}

void PON_PAS_APPLICATIONS_clr_onu_infos 
                                  ( const PON_olt_id_t  	olt_id, 
									const PON_onu_id_t	    onu_id )
{
    RM_MEM_ZERO(&onu_info[olt_id][onu_id], sizeof(onu_info_t));
}

void PON_PAS_APPLICATIONS_clr_olt_infos( const PON_olt_id_t  	olt_id )
{
    RM_MEM_ZERO(&onu_info[olt_id][0], sizeof(onu_info[0]));
}

unsigned short PON_PAS_APPLICATIONS_get_oam_frame_size(
                      const PON_olt_id_t   olt_id, 
                      const PON_onu_id_t   onu_id )
{  
	return (onu_info[olt_id][onu_id].maximum_data_frame_size);
}


void PON_PAS_APPLICATIONS_set_oam_frame_size(
                      const PON_olt_id_t     olt_id, 
                      const PON_onu_id_t     onu_id,
					  const unsigned short   maximum_data_frame_size )
{  
	onu_info[olt_id][onu_id].maximum_data_frame_size = maximum_data_frame_size;
}



bool PON_PAS_APPLICATIONS_get_oam_frame_size_valid(
                      const PON_olt_id_t   olt_id, 
                      const PON_onu_id_t   onu_id )
{  
	return (onu_info[olt_id][onu_id].oam_negotiation_field_is_valid);
}


void PON_PAS_APPLICATIONS_set_oam_frame_size_valid(
                      const PON_olt_id_t   olt_id, 
                      const PON_onu_id_t   onu_id,
					  const bool		   oam_negotiation_field_is_valid )
{  
	onu_info[olt_id][onu_id].oam_negotiation_field_is_valid = oam_negotiation_field_is_valid;
}


#define PONLOG_FUNCTION_NAME "REMOTE_APPLICATIONS_send_receive_message"

short int PON_PAS_APPLICATIONS_send_receive_message  ( 
                      const PON_olt_id_t      olt_id, 
                      const PON_onu_id_t      onu_id,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size,
                      unsigned char          *received_data,
                      unsigned short         *received_data_size,
                      unsigned short          allocated_received_data_size)
{


	if(onu_info[olt_id][onu_id].maximum_data_frame_size < (sent_data_size+ETHERNET_FRAME_HEADER_SIZE+15) )
	{
		PONLOG_ERROR_4(PONLOG_FUNC_GENERAL, olt_id, onu_id,  "OLT %d ONU %d OAM PDU Configuration length is set to %d while frame length is %d\n", 
						   olt_id, onu_id, onu_info[olt_id][onu_id].maximum_data_frame_size,
						   (sent_data_size+ETHERNET_FRAME_HEADER_SIZE+1) );

		return REMOTE_PASONU_EXIT_ERROR;
	}


    return (Convert_comm_error_code_to_remote_management(
                    REMOTE_COMM_send_receive_message
                                        ( olt_id, onu_id, sent_data, sent_data_size,
                                          received_data, received_data_size, allocated_received_data_size)) );
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "PON_PAS_OamFrameRecvCB"
int PON_PAS_OamFrameRecvCB( 
                   const PON_olt_id_t             olt_id,
                   const PON_llid_t               llid,
                   const PON_oam_code_t	          oam_code,
                   const OAM_oui_t				  oui,
                   const unsigned short           length,
                   unsigned char                 *content )
{
    /* sys_console_printf("\r\nOLT%d's llid%d is PmcOamFrameReceived(len: %u)!", olt_id, llid, length); */

    global_handler_function( olt_id, llid, length, content );

	return (EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Adaptation_layer_send_frame"

static int Adaptation_layer_send_frame ( const short int	       olt_id, 
					                    const unsigned short   length, 
					                    const short int        onu_id,
					                    const short int	       broadcast_llid,
					                    const void		      *content )
{
    int result = PON_OAM_MODULE_send_frame(olt_id, onu_id, PON_OAM_CODE_VENDOR_EXTENSION, internal_pmc_oui, length, content);

    if ( 0 == result )
    {
        /* sys_console_printf("\r\nOLT%d's llid%d is PmcOamFrameSend(len: %u)!", olt_id, onu_id, length); */
    }
    else
    {
        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id, "ERROR: pon_send_frame failed with code %d. \n", result); 
	}

    return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_MNGT_register_handler"

static int Adaptation_layer_register_handler ( REMOTE_MNGT_frame_received_handler_t handler_function )
{
    int result;

    /* save registered handler in a global variable in order to call the function when received 
       frame event occurs*/
    
    global_handler_function = handler_function;
    
    result = PON_OAM_MODULE_assign_handler(PON_OAM_CODE_VENDOR_EXTENSION, internal_pmc_oui, PON_PAS_OamFrameRecvCB);

	if (result != EXIT_OK) 
    {
        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: OAM_assign_handler_function failed with code %d. \n", result); 
	}

    return result;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME "REMOTE_MNGT_terminate_handler"

static int Adaptation_layer_terminate_handler ( REMOTE_MNGT_frame_received_handler_t handler_function )
{
    int result = 0;

    /* save registered handler in a global variable in order to call the function when received 
       frame event occurs*/
    
    global_handler_function = handler_function;
    

    return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Adaptation_layer_get_olt_mac_address"

static int Adaptation_layer_get_olt_mac_address ( const short int  olt_id, 
							                     mac_address_t    mac_address )
{
    

    int result;
    int *piMacAddr = GetPonChipMgmtPathMac ( olt_id ); 
  

	if ( piMacAddr != NULL )
    {
        VOS_MemCpy(mac_address, piMacAddr, sizeof(mac_address_t));
        result = 0;
    }
    else
    {
        result = EXIT_ERROR;
        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, PONLOG_ONU_IRRELEVANT, "ERROR: get_olt_mac_address failed with code %d. \n", result); 
	}


    return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Adaptation_layer_assign_registration"

short int PON_PAS_Adaptation_layer_assign_registration( const void	(*registration_function)())
{

    short int result;


    result = Pon_event_assign_handler_function(PON_EVT_HANDLER_LLID_END_STD_OAM_DISCOVERY, (void*)pas_pon_event_handler, &registration_handler_id);

    if (result != EXIT_OK) 
    {
        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: Pon_event_assign_handler_function failed with code %d. \n", result); 
	}

    return result;
}

#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME "Adaptation_layer_terminate_assign_registration"

short int PON_PAS_Adaptation_layer_terminate_registration( const void	(*registration_function)())
{

    short int result;


	result = Pon_event_delete_handler_function(registration_handler_id);

    if (result != EXIT_OK) 
    {
        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: Pon_event_delete_handler_function failed with code %d. \n", result); 
	}

    return result;
}

#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Adaptation_layer_assign_registration"

short int PON_PAS_Adaptation_layer_assign_deregistration( const void	(*deregistration_function)())
{

    short int result;


    result = Pon_event_assign_handler_function(PON_EVT_HANDLER_LLID_DEREGISTER_EVENT,(void*)pas_pon_event_handler, &deregistration_handler_id);

    if (result != EXIT_OK) 
    {
        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: Pon_event_assign_handler_function failed with code %d. \n", result); 
	}

    return result;
}

#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME "Adaptation_layer_terminate_assign_deregistration"

short int PON_PAS_Adaptation_layer_terminate_deregistration( const void	(*deregistration_function)())
{

    short int result;


	result = Pon_event_delete_handler_function(deregistration_handler_id);

    if (result != EXIT_OK) 
    {
        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: Pon_event_delete_handler_function failed with code %d. \n", result); 
	}

    return result;
}

#undef PONLOG_FUNCTION_NAME

#endif


#if 1
/* Tran Layer */

#define PONLOG_FUNCTION_NAME "REMOTE_COMM_init"
short int PON_PAS_COMM_init( general_handler_function_t event_received_handler )
{

    short int result = REMOTE_COMMUNICATION_EXIT_OK;


    event_received_handler_function = event_received_handler;



    /* init semaphore */
    SEMAPHORE_INIT_AND_LOG_FOR_ERROR(Communication_init, &remote_mngt_semaphore,
                                      remote_mngt_semaphore, result)
                                      
    if( result != EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to take a semaphore\n" );
		
        return REMOTE_COMMUNICATION_EXIT_ERROR;
    }

    result = COMM_LLC_init( (general_handler_function_t)Received_message, 
                            (general_handler_function_t)Received_event );

    return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_COMM_terminate"

short int PON_PAS_COMM_terminate( void )
{
    short int result = REMOTE_COMMUNICATION_EXIT_OK;

    result = COMM_LLC_terminate( );
    if( result != REMOTE_COMMUNICATION_EXIT_OK )
    {
        
	   PONLOG_ERROR_0(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error in link layer termination\n" );
	
        return REMOTE_COMMUNICATION_EXIT_ERROR;
    }

    /* close semaphore */
    SEMAPHORE_CLOSE_AND_LOG_FOR_ERROR ( Communication_terminate, &remote_mngt_semaphore,
                                        remote_mngt_semaphore, result)

    if( result != EXIT_OK )
    {
        
	    PONLOG_ERROR_0(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error in closing a semaphore\n" );
	
        return REMOTE_COMMUNICATION_EXIT_ERROR;
    }
    
 
    event_received_handler_function = NULL;


    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_COMM_send_receive_message"

static int REMOTE_COMM_send_receive_message  ( 
                      const PON_olt_id_t      olt_id, 
                      const PON_onu_id_t      onu_id,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size,
                      unsigned char          *received_data,
                      unsigned short         *received_data_size,
                      unsigned short          allocated_received_data_size)
{
    
    short int                   return_result = REMOTE_COMMUNICATION_EXIT_OK,result;
    short int                   request_id=0;
    unsigned long               msg_id;
    bool                        resend = TRUE;
    remote_mngmt_db_state_t     entry_state;
    PON_DECLARE_TIMEOUT_VARIABLES

    COMM_LLC_get_link_layer_message_id(olt_id, onu_id, &msg_id);

    result = Insert_to_db(msg_id, received_data, received_data_size, allocated_received_data_size);
    CHECK_COMM_RESULT_AND_RETURN
    

    while( (resend == TRUE) && (request_id < REMOTE_MANAGEMENT_RESPONSE_RESEND_LIMIT) )
    {
        return_result = COMM_LLC_send_frame( olt_id, onu_id, sent_data, sent_data_size, msg_id);
        
        if(return_result != REMOTE_COMMUNICATION_EXIT_OK)
        {
            PONLOG_ERROR_2(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id, "ERROR: failed to send frame OLT: %d ONU: %d \n",olt_id, onu_id)
                
            resend = FALSE;
            continue;
        }

    
        request_id ++;

        PON_TIMEOUT_INIT(REMOTE_MANAGEMENT_TIMEOUT);
   
    
        /* wait for response - busy waiting */
        while ( (Get_entry_state_db(msg_id) != REMOTE_DB_STATE_RECEIVED_FRAME)
                 && (!PON_TIMEOUT_EXPIRED) ) 
        {
		    PON_TIMEOUT_ITERATION;
	    }	

	    /* received response */
        entry_state = Get_entry_state_db(msg_id);
        switch(entry_state)
        {
        case REMOTE_DB_STATE_RECEIVED_FRAME:
            resend = FALSE;
            return_result = REMOTE_COMMUNICATION_EXIT_OK;
            break;
        case REMOTE_DB_STATE_WAITING:
            {
                char  resend_str[10]="resending";

                if(request_id == REMOTE_MANAGEMENT_RESPONSE_RESEND_LIMIT)
                {
                    memcpy(resend_str, "", 10);
                }
	  

	            PONLOG_ERROR_4(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id,  "ERROR: timeout on OLT: %d ONU: %d message id: %lu.%s \n",
                                olt_id, onu_id, msg_id, resend_str)
                
      
		        return_result = REMOTE_COMMUNICATION_TIME_OUT; /* we got out of loop because of timeout */
            }
            break;
        case REMOTE_DB_STATE_RECEIVED_LARGE_FRAME:
            resend = FALSE;
            return_result = REMOTE_COMMUNICATION_MEMORY_ERROR;
            break;
        default:
            PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id,  "No such case %d \n",entry_state);

        }
    }

    result = Remove_from_db(msg_id);
    CHECK_COMM_RESULT_AND_RETURN
     

    return return_result;
}

#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Received_message"

static short int Received_message  ( 
                                    const unsigned short   length,
                                    const unsigned char   *content,
                                    const unsigned long    msg_id )
{
    unsigned char    *receive_pdu_data = (unsigned char *)content;
    
    return (Update_db(msg_id, receive_pdu_data, &length));
}

#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Received_event"

static short int Received_event  ( 
                                    const short int        olt_id,
                                    const PON_onu_id_t     onu_id,
                                    const unsigned short   length,
                                    const unsigned char   *content)
{
    
    event_received_handler_function(olt_id, onu_id, length, content);


    return REMOTE_COMMUNICATION_EXIT_OK;

}

#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Insert_to_db"

static short int Insert_to_db
                ( const unsigned long    message_id, 
                  const unsigned char   *received_data, 
                  const unsigned short  *received_data_size,
                  const unsigned short   allocated_received_data_size)
{
    short int  result;


    unsigned short entry = (unsigned short)(message_id % COMM_SYNC_DB_MAX_SIZE);

	   /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR(Send_oam, &remote_mngt_semaphore, remote_mngt_semaphore,result );
    if (result != EXIT_OK) return (REMOTE_COMMUNICATION_EXIT_ERROR);


    if(comm_sync_database[entry].entry_state != REMOTE_DB_STATE_NOT_VALID )
    {/* it means that this entry is alredy taken by another request */
        PONLOG_ERROR_2(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: entry id %u is already occupied, the new message_id:%u can not be insert\n", 
                           entry, message_id );

		/* END PROTECTION  - let receive handler set the global data */
		RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(send_request, &remote_mngt_semaphore, remote_mngt_semaphore,result );
        return REMOTE_COMMUNICATION_EXIT_ERROR;
    }
 
    comm_sync_database[entry].received_data = (unsigned char *)received_data;
    comm_sync_database[entry].received_data_size = (unsigned short   *)received_data_size;
    comm_sync_database[entry].allocated_received_data_size = allocated_received_data_size;
    comm_sync_database[entry].entry_state = REMOTE_DB_STATE_WAITING;

   
    /* END PROTECTION  - let receive handler set the global data */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(send_request, &remote_mngt_semaphore, remote_mngt_semaphore,result );
    if (result != EXIT_OK) return (REMOTE_COMMUNICATION_EXIT_ERROR);

    return REMOTE_COMMUNICATION_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME "Update_db"

static short int Update_db
                ( const unsigned long    message_id, 
                  const unsigned char   *received_data, 
                  const unsigned short  *received_data_size)
{
    short int           sem_result, result=REMOTE_COMMUNICATION_EXIT_OK;
    unsigned short      allocated_size;


    unsigned short entry = (unsigned short)(message_id % COMM_SYNC_DB_MAX_SIZE);

   /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR(Send_oam, &remote_mngt_semaphore, remote_mngt_semaphore,sem_result );
	if (result != EXIT_OK) return (REMOTE_COMMUNICATION_EXIT_ERROR);


      /* check message id */
    if(comm_sync_database[entry].entry_state == REMOTE_DB_STATE_NOT_VALID)
    {

        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: received unexpected message id: %d. aborting/\n", message_id );

		/* END PROTECTION  - let receive handler set the global data */
		RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(send_request, &remote_mngt_semaphore, remote_mngt_semaphore,sem_result );
        return REMOTE_COMMUNICATION_WRONG_RESPONSE_MESSAGE_ID;
    }
    
 
  
    allocated_size = comm_sync_database[entry].allocated_received_data_size;
    if(allocated_size < *received_data_size)
    {

        PONLOG_ERROR_3(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR:received large frame with message id: %d.allocated size:%u, received size:%u\n", 
                           message_id, allocated_size, *received_data_size  );

        comm_sync_database[entry].entry_state = REMOTE_DB_STATE_RECEIVED_LARGE_FRAME;
        result =  REMOTE_COMMUNICATION_MEMORY_ERROR;
    }
    else
    {
        memcpy (comm_sync_database[entry].received_data, (unsigned char*)received_data,*received_data_size);
        *(comm_sync_database[entry].received_data_size) = *received_data_size;
        comm_sync_database[entry].entry_state = REMOTE_DB_STATE_RECEIVED_FRAME;
    }
 
   
    /* END PROTECTION  - let receive handler set the global data */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(send_request, &remote_mngt_semaphore, remote_mngt_semaphore,sem_result );
	if (result != EXIT_OK) return (REMOTE_COMMUNICATION_EXIT_ERROR);

    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "Remove_from_db"

static short int Remove_from_db( const unsigned long   message_id)
{
    short int  result;
    
    unsigned short entry = (unsigned short)(message_id % COMM_SYNC_DB_MAX_SIZE);

    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR(Send_oam, &remote_mngt_semaphore, remote_mngt_semaphore,result );
	if (result != EXIT_OK) return (REMOTE_COMMUNICATION_EXIT_ERROR);

    /* check if it is a valid entry*/
    if(comm_sync_database[entry].received_data != NULL)
    {
        comm_sync_database[entry].received_data = NULL;
        comm_sync_database[entry].received_data_size = NULL;
        comm_sync_database[entry].allocated_received_data_size = 0;
        comm_sync_database[entry].entry_state = REMOTE_DB_STATE_NOT_VALID;
    }
    else
    {
        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "COMM_SYNC: %ul non existing enty\n",message_id );

		/* END PROTECTION  */
		RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(send_request, &remote_mngt_semaphore, remote_mngt_semaphore,result );

		return REMOTE_COMMUNICATION_EXIT_ERROR;
    }
    

    /* END PROTECTION  */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(send_request, &remote_mngt_semaphore, remote_mngt_semaphore,result );
	if (result != EXIT_OK) return (REMOTE_COMMUNICATION_EXIT_ERROR);

    return REMOTE_COMMUNICATION_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Get_entry_state_db"
static remote_mngmt_db_state_t Get_entry_state_db( const unsigned long       message_id)
{
    short int                result;
    remote_mngmt_db_state_t  entry_state;
    
    unsigned short entry = (unsigned short)(message_id % COMM_SYNC_DB_MAX_SIZE);

    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR(Send_oam, &remote_mngt_semaphore, remote_mngt_semaphore,result );

    entry_state = comm_sync_database[entry].entry_state;

    /* END PROTECTION  */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(send_request, &remote_mngt_semaphore, remote_mngt_semaphore,result );

    return entry_state;

}
#undef PONLOG_FUNCTION_NAME

#endif


#if 1
/* Express Layer */


#define PONLOG_FUNCTION_NAME "RmDecompose_print"

void RmDecompose_print(char *string_buffer)
{
    PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, string_buffer );
}
#undef PONLOG_FUNCTION_NAME

#endif


#if 1
/* Link Layer */

#define PONLOG_FUNCTION_NAME "COMM_LLC_init"
static short int COMM_LLC_init( general_handler_function_t response_handler,
                         general_handler_function_t event_handler )
{

    short int result = REMOTE_COMMUNICATION_EXIT_OK;

#ifdef PMC_MEM_DYNAMIC
    if ( NULL != (comm_sync_database = (comm_sync_database_entry_t*)VOS_Malloc(COMM_SYNC_DB_MAX_SIZE * sizeof(comm_sync_database_entry_t) + sizeof(unsigned long) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE, MODULE_RPU_OAM)) )
    {
        RM_MEM_ZERO(comm_sync_database, COMM_SYNC_DB_MAX_SIZE * sizeof(comm_sync_database_entry_t) + sizeof(unsigned long) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE);
        internal_event_id = (unsigned long**)(comm_sync_database + COMM_SYNC_DB_MAX_SIZE);
    }
    else
    {
        VOS_ASSERT(0);
        
		PONLOG_ERROR_0(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to alloc a buf\n" );
		
        return REMOTE_COMMUNICATION_MEMORY_ERROR;
    }
#else
    RM_MEM_ZERO(internal_event_id, sizeof(internal_event_id));
    RM_MEM_ZERO(comm_sync_database, sizeof(comm_sync_database));
#endif

    response_handler_function = response_handler;
    event_handler_function = event_handler;
 
    /* init semaphore */
    SEMAPHORE_INIT_AND_LOG_FOR_ERROR(Communication_init, &msg_id_semaphore,
                                      msg_id_semaphore, result)
                                      
    if( result != EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to take a semaphore\n" );
		
        return REMOTE_COMMUNICATION_EXIT_ERROR;
    }

    result = COMM_OAM_init((general_handler_function_t)Receive_link_layer_frame);
 
    return result;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME "COMM_LLC_terminate"
static short int COMM_LLC_terminate( )
{

    short int result = REMOTE_COMMUNICATION_EXIT_OK;

    /* close semaphore */
    SEMAPHORE_CLOSE_AND_LOG_FOR_ERROR ( Communication_terminate, &msg_id_semaphore,
                                        msg_id_semaphore, result)

    if( result != EXIT_OK )
    {
        
	    PONLOG_ERROR_0(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error in closing a semaphore\n" );
	
        return REMOTE_COMMUNICATION_EXIT_ERROR;
    }

    return(COMM_OAM_terminate());
 
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "COMM_LLC_get_link_layer_message_id"

static short int COMM_LLC_get_link_layer_message_id  
                    ( const PON_olt_id_t   olt_id, 
                      const PON_onu_id_t   onu_id,
                      unsigned long       *msg_id)
{ 
    short int result = REMOTE_COMMUNICATION_EXIT_OK;
 
    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR(Send_oam, &msg_id_semaphore, msg_id_semaphore,result );
    if (result != EXIT_OK) return (REMOTE_COMMUNICATION_EXIT_ERROR);

    (*msg_id) = internal_msg_id;
    internal_msg_id++;
    
    /* END PROTECTION  - let receive handler set the global data */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(send_request, &msg_id_semaphore, msg_id_semaphore,result );
    if (result != EXIT_OK) return (REMOTE_COMMUNICATION_EXIT_ERROR);


    return REMOTE_COMMUNICATION_EXIT_OK;
}

#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME "COMM_LLC_send_frame"

static short int COMM_LLC_send_frame  ( 
                      const PON_olt_id_t      olt_id, 
                      const PON_onu_id_t      onu_id,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size,
                      unsigned long           msg_id)
{
    
    short int               result;
    unsigned char           msg_id_buffer[BYTES_IN_LONG];    
    unsigned char           pdu_type_buffer[BYTES_IN_WORD];    
    unsigned short          link_layer_data_size = (unsigned short)(sent_data_size+LLC_HEADER_SIZE);

    /* Build link layer header */

    /* insert message id */
    ULONG_2_ENDIANITY_UBUFFER(msg_id, msg_id_buffer)
#if 0
    memcpy(sent_data+LLC_HEADER_PLACE+MSG_ID_PLACE, msg_id_buffer, BYTES_IN_LONG);
#else
    memcpy(sent_data+4+MSG_ID_PLACE, msg_id_buffer, BYTES_IN_LONG);
#endif

    /* insert pdu type */
    USHORT_2_ENDIANITY_UBUFFER(PDU_TYPE_REQUEST, pdu_type_buffer)
#if 0
    memcpy(sent_data+LLC_HEADER_PLACE+PDU_TYPE_PLACE, pdu_type_buffer, BYTES_IN_WORD);
#else
    memcpy(sent_data+4+PDU_TYPE_PLACE, pdu_type_buffer, BYTES_IN_WORD);
#endif

    result = COMM_OAM_send_frame ( olt_id, onu_id, sent_data, link_layer_data_size );
    CHECK_COMM_RESULT_AND_RETURN

    
    return result;
}

#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Parse_oam_message"

static void Receive_link_layer_frame  ( 
                                    const short int        olt_id,
                                    const PON_onu_id_t     onu_id,
                                    const unsigned short   length,
                                    const unsigned char   *content)
{
    REMOTE_MANAGEMENT_link_layer_msg_t       oam_message;
    unsigned char                           *receive_data = (unsigned char *)content;
    unsigned char                            msg_id_buffer[BYTES_IN_LONG], pdu_type_buffer[BYTES_IN_LONG];
    
    /* print remote msg here after upper layers check remote validation */  
	Print_remote_msg(olt_id, onu_id, receive_data, length, FALSE);

    /* extract message id */
    memcpy(msg_id_buffer, receive_data+MSG_ID_PLACE, BYTES_IN_LONG);
    UBUFFER_2_ENDIANITY_ULONG(msg_id_buffer, oam_message.message_id)


        
    /* extract pdu type */
    memcpy(pdu_type_buffer, receive_data+PDU_TYPE_PLACE, BYTES_IN_LONG);
    UBUFFER_2_ENDIANITY_USHORT(pdu_type_buffer, oam_message.pdu_type)  


    /* check pdu type */
    switch(oam_message.pdu_type)
    {
        case PDU_TYPE_RESPONSE:
            /* check message id */
            if(internal_msg_id < oam_message.message_id)
            {
                PONLOG_ERROR_2(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id, "WARNING: remote management package lost responses,expected message id:%u, received message id:%u\n",
                                   internal_msg_id, oam_message.message_id );

                return;

            }
            response_handler_function( (unsigned short)(length-LLC_HEADER_SIZE), 
                                       receive_data+LLC_HEADER_SIZE, oam_message.message_id );
   
            break;
        case PDU_TYPE_EVENT:
            /* check message id */
            if( oam_message.message_id != 0)
            {
                if( internal_event_id[olt_id][onu_id] != 0)
                {
                    if( oam_message.message_id != (internal_event_id[olt_id][onu_id]+1) )
                    {

                        PONLOG_ERROR_2(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id, "WARNING: remote management package lost events,expected event id:%u, received event id:%u\n",
                                          internal_event_id[olt_id][onu_id]+1, oam_message.message_id );
                    }
                }
            }

            /*update event counter*/
            internal_event_id[olt_id][onu_id] = oam_message.message_id;

            event_handler_function( olt_id, onu_id, (unsigned short)(length-LLC_HEADER_SIZE), 
                                    receive_data+LLC_HEADER_SIZE );
   

            break;
        default:
            PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id, "ERROR: received unexpected pdu type: %d. aborting )\n", oam_message.pdu_type );
 
            return;
    }
   
    return;
}

#undef PONLOG_FUNCTION_NAME


#endif


#if 1
/* OAM Layer */

#define PONLOG_FUNCTION_NAME "COMM_OAM_init"
static short int COMM_OAM_init( general_handler_function_t handler )
{

    short int result = REMOTE_COMMUNICATION_EXIT_OK;


    received_frame_handler = handler;
    
    result = COMM_ETHERNET_init((REMOTE_MNGT_frame_received_handler_t)Receive_oam_layer_frame);
 

    return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "COMM_OAM_terminate"
static short int COMM_OAM_terminate( )
{

  
    
    return (COMM_ETHERNET_terminate());
 


}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "COMM_OAM_send_frame"

static short int COMM_OAM_send_frame  ( 
                      const PON_olt_id_t      olt_id, 
                      const PON_onu_id_t      onu_id,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size)
{
    
    short int               result;
#if 0
    unsigned short          oam_data_size = (unsigned short)(sent_data_size+OAM_HEADER_SIZE);

    /* Build OAM header */

    /* insert sub type */
    sent_data[OAM_HEADER_PLACE+OAM_SUB_TYPE_PLACE] =  const_oam_sub_type;

    /* insert flags */
    sent_data[OAM_HEADER_PLACE+OAM_FLAGS_PLACE] = (unsigned char)((const_oam_flags >> BITS_IN_BYTE) & 0xff);
    sent_data[OAM_HEADER_PLACE+OAM_FLAGS_PLACE+1] = (unsigned char)((const_oam_flags & 0xff));
    

    /* insert code */
    sent_data[OAM_HEADER_PLACE+OAM_CODE_PLACE] =  const_oam_code;

    /* insert Passave oui */
    sent_data[OAM_HEADER_PLACE+OAM_OUI_PLACE]     = (unsigned char)((OUI_DEFINE >>(BYTES_IN_WORD*BITS_IN_BYTE))& 0xff);
    sent_data[OAM_HEADER_PLACE+OAM_OUI_PLACE+1]   = (unsigned char)((OUI_DEFINE >>BITS_IN_BYTE)& 0xff);
    sent_data[OAM_HEADER_PLACE+OAM_OUI_PLACE+2]   = (unsigned char)(OUI_DEFINE & 0xff);

    /* insert legacy number */
    sent_data[OAM_HEADER_PLACE+OAM_MAGIC_NUMBER_PLACE]   = OAM_LEGACY_NUMBER;  

    result = COMM_ETHERNET_send_frame ( olt_id, onu_id, sent_data, oam_data_size );
#else
    /* insert Passave oui */
    sent_data[0] = (unsigned char)((OUI_DEFINE >>(BYTES_IN_WORD*BITS_IN_BYTE))& 0xff);
    sent_data[1] = (unsigned char)((OUI_DEFINE >>BITS_IN_BYTE)& 0xff);
    sent_data[2] = (unsigned char)(OUI_DEFINE & 0xff);

    /* insert legacy number */
    sent_data[3] = OAM_LEGACY_NUMBER;  

    result = COMM_ETHERNET_send_frame ( olt_id, onu_id, sent_data, sent_data_size + 4 );
#endif

    CHECK_COMM_RESULT_AND_RETURN

    return result;
}

#undef PONLOG_FUNCTION_NAME





#define PONLOG_FUNCTION_NAME "Receive_oam_layer_frame"

static void Receive_oam_layer_frame(  
                                  const short int        olt_id,
                                  const PON_onu_id_t     onu_id,
                                  const unsigned short   length,
                                  const unsigned char   *content)
{
    REMOTE_MANAGEMENT_oam_layer_msg_t   oam_message;
    unsigned char                       first_expeced_byte,second_expeced_byte,third_expeced_byte;
    
    unsigned char *receive_data = (unsigned char *)content;


#if 0
    /* extract sub type */
    oam_message.sub_type      = (unsigned char)  receive_data[OAM_SUB_TYPE_PLACE];

    /* check sub type */
    if( oam_message.sub_type != const_oam_sub_type )
    {

        PONLOG_ERROR_2(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id,  "ERROR: received wrong OAM sub type. expected :0x%x , received 0x%x )\n",
                            const_oam_sub_type, oam_message.sub_type);
        
        return;
    }



    
    /* extract flags */
    oam_message.flags = (unsigned short)(MAKE_UNSIGNED_SHORT_PARAM(*(receive_data+OAM_FLAGS_PLACE),*(receive_data+OAM_FLAGS_PLACE+1))); 

    /* check flags 
    if( oam_message.flags != const_oam_flags )
    {

        PONLOG_ERROR_2(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id,  "ERROR: received wrong OAM flags. expected :0x%x , received 0x%x )\n",
                           const_oam_flags, oam_message.flags);
        
        return REMOTE_COMMUNICATION_EXIT_ERROR;
    }*/



    /* extract code */
    oam_message.code         = (unsigned char)  receive_data[OAM_CODE_PLACE];

    /* check code 
    if( oam_message.code != const_oam_code )
    {

        PONLOG_ERROR_2(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id,  "ERROR: received wrong OAM code. expected :0x%x , received 0x%x )\n",
                           const_oam_code, oam_message.code);
        
        return REMOTE_COMMUNICATION_EXIT_ERROR;
    }*/
  



    /* extract OUI */
    oam_message.oui_opcode_1  = (unsigned char)  receive_data[OAM_OUI_PLACE];
    oam_message.oui_opcode_2  = (unsigned char)  receive_data[OAM_OUI_PLACE+1];
    oam_message.oui_opcode_3  = (unsigned char)  receive_data[OAM_OUI_PLACE+2];

    /* check oui */
    first_expeced_byte  =   (unsigned char)((OUI_DEFINE >>(BYTES_IN_WORD*BITS_IN_BYTE))& 0xff);
    second_expeced_byte =   (unsigned char)((OUI_DEFINE >>BITS_IN_BYTE)& 0xff);
    third_expeced_byte  =   (unsigned char)(OUI_DEFINE & 0xff);
    
    if( ( oam_message.oui_opcode_1 != first_expeced_byte ) ||
        ( oam_message.oui_opcode_2 != second_expeced_byte ) ||
        ( oam_message.oui_opcode_3 != third_expeced_byte  ) )
    {
        /*
                PONLOG_ERROR_6(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id,  "ERROR: received wrong OUI. expected :0x%x%x%x , received 0x%x%x%x )\n",
                        first_expeced_byte, second_expeced_byte, third_expeced_byte, 
                        oam_message.oui_opcode_1, oam_message.oui_opcode_2, oam_message.oui_opcode_3 );
        */
        return;
    }
#endif


    
    /* extract legacy number */
#if 0
    oam_message.legacy_number  = (unsigned char) receive_data[OAM_MAGIC_NUMBER_PLACE];
#else
    oam_message.legacy_number  = (unsigned char) receive_data[0];
#endif

    /* check legacy number */
    if(  oam_message.legacy_number != OAM_LEGACY_NUMBER )
    {

        PONLOG_ERROR_2(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id, "ERROR: received wrong legacy number. expected :0x%x, received 0x%x\n",
                           OAM_LEGACY_NUMBER, oam_message.legacy_number );
        
        return;
    }

#if 0
    (*received_frame_handler)(olt_id, onu_id, (unsigned short)(length-OAM_HEADER_SIZE), content+OAM_HEADER_SIZE);
#else
    (*received_frame_handler)(olt_id, onu_id, (unsigned short)(length-1), content+1);
#endif

    return;

}

#undef PONLOG_FUNCTION_NAME

#endif


#if 1
/* Phy Layer */

#define PONLOG_FUNCTION_NAME "COMM_ETHERNET_init"
static short int COMM_ETHERNET_init( REMOTE_MNGT_frame_received_handler_t handler )
{

    short int result = REMOTE_COMMUNICATION_EXIT_OK;


    /* assign handler for receiving ONU response frames */
    result = Adaptation_layer_register_handler ( (REMOTE_MNGT_frame_received_handler_t ) Receive_ethernet_layer_frame );

    if( result != EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error in assign handler. can not receive remote management events\n" );
		
        return REMOTE_COMMUNICATION_EXIT_ERROR;
    }

    received_eth_frame_handler = handler;

    return REMOTE_COMMUNICATION_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "COMM_ETHERNET_terminate"
static short int COMM_ETHERNET_terminate(  )
{

    short int result = REMOTE_COMMUNICATION_EXIT_OK;


   /* assign handler for receiving ONU response frames */

    result = Adaptation_layer_terminate_handler ( (REMOTE_MNGT_frame_received_handler_t ) Receive_ethernet_layer_frame  );

    if( result != EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error in assign handler. can not receive remote management events\n" );
		
        return REMOTE_COMMUNICATION_EXIT_ERROR;
    }

    return REMOTE_COMMUNICATION_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "COMM_ETHERNET_send_frame"

static short int COMM_ETHERNET_send_frame  ( 
                      const PON_olt_id_t      olt_id, 
                      const PON_onu_id_t      onu_id,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size)
{

	short int           result;
    unsigned short      sent_size;

	/* Input checking */
	if( !sent_data )
		return REMOTE_COMMUNICATION_NULL_POINTER;
	
	if( (olt_id > PON_MAX_OLT_ID) || (olt_id < PON_MIN_OLT_ID) )
		return REMOTE_COMMUNICATION_PARAMETER_ERROR;

#if 0
    /* initialize ethernet header */
	result = Init_ethernet_oam_header (  olt_id, sent_data );
    if(result != REMOTE_COMMUNICATION_EXIT_OK)
    {
        PONLOG_ERROR_0(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id, "ERROR: Init_ethernet_oam_header failed\n");       
        
        return result;

    }

    sent_size = (unsigned short)(ETH_HEADER_SIZE+sent_data_size);


    /* padding pdu data with zeros if smaller than minimum */
    if ( sent_data_size < ETH_OAM_MIN_DATA_SIZE ) 
    {
	    memset( (unsigned char *)(sent_data+ETH_HEADER_SIZE+sent_data_size), 0, 
				ETH_OAM_MIN_DATA_SIZE - sent_data_size) ;
        
        sent_size = MIN_ETHERNET_FRAME_SIZE; 
    }


	Print_remote_msg(olt_id, onu_id, sent_data , sent_size, TRUE);


    /* synchronous send frame, wait for receiving response */
    result = Adaptation_layer_send_frame ( olt_id, sent_size, onu_id, FALSE, (void*)sent_data );
#else
    /* synchronous send frame, wait for receiving response */
    result = Adaptation_layer_send_frame ( olt_id, sent_data_size, onu_id, FALSE, (void*)sent_data );
#endif

	if (result != EXIT_OK) /* adaptation layer return value*/
    {
        
        PONLOG_ERROR_0(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id, "ERROR: Adaptation_layer_send_frame failed\n");

        result = REMOTE_COMMUNICATION_EXIT_ERROR;
	}

    
    return result;	
    
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Receive_ethernet_layer_frame"

static void Receive_ethernet_layer_frame (
                                 const short int                 olt_id,
                                 const PON_llid_t                llid,
                                 const unsigned short            length,
                                 const void                     *content )
{
#if 0
    mac_address_t      destination_mac_address;
    short int          index;
    bool               valid_address=TRUE;
    unsigned short     received_ethernet_type;
    unsigned char     *received_buffer = (unsigned char *)content;

    /* check illegal frame size */
    if ( (length < MIN_ETHERNET_FRAME_SIZE) || (length > MAX_ETHERNET_FRAME_SIZE_STANDARDIZED) )
    {

        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, llid, "ERROR: received wrong size frame (%d bytes )\n",length);
        return;
    }


    /* extract destination mac address */
    memcpy( destination_mac_address, (unsigned char *)(received_buffer+ETH_DA_BEGIN_PLACE), BYTES_IN_MAC_ADDRESS);

    /* check destination mac address */
    for(index = 0; index < BYTES_IN_MAC_ADDRESS; index++)
    {
        if(destination_mac_address[index] != oam_destination_address[index])
        {
            valid_address = FALSE;
        }
    }

    if( !valid_address )
    {
        return;
    }

    
    /* extract ethernet type */
    received_ethernet_type = (unsigned short)MAKE_UNSIGNED_SHORT_PARAM(*(received_buffer+ETH_TYPE_BEGIN_PLACE),*(received_buffer+ETH_TYPE_BEGIN_PLACE+1)); 
 
    /* check ethernet type */
    if( received_ethernet_type != const_ethernet_type )
    {
        PONLOG_ERROR_2(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, llid,  "ERROR: received wrong ether type 0x%u, expected 0x%u \n",
                           received_ethernet_type, const_ethernet_type );


        return;
    }

    received_eth_frame_handler(olt_id, llid, (unsigned short)(length-ETH_HEADER_SIZE), received_buffer+ETH_HEADER_SIZE);
#else
    received_eth_frame_handler(olt_id, llid, length, content);
#endif
    
    return;

}
#undef PONLOG_FUNCTION_NAME




#define PONLOG_FUNCTION_NAME "Init_ethernet_oam_header"

static short int Init_ethernet_oam_header ( const PON_olt_id_t   olt_id, 
									        unsigned char       *buffer )
{
    mac_address_t    source_mac_address;
    short int        result=REMOTE_COMMUNICATION_EXIT_OK;
  

    /* insert destination mac address */
    memcpy( (unsigned char *)(buffer+ETH_DA_BEGIN_PLACE), oam_destination_address, BYTES_IN_MAC_ADDRESS);

  
    if( Adaptation_layer_get_olt_mac_address ( olt_id, source_mac_address ) != EXIT_OK )
    {
        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, PONLOG_ONU_IRRELEVANT, "ERROR: Get_olt_mac_address failed. olt id %d doesn't exist\n", olt_id);
        return REMOTE_COMMUNICATION_OLT_NOT_EXIST;
    }

    /* insert source mac address */
    memcpy( (unsigned char *)(buffer+ETH_SA_BEGIN_PLACE), source_mac_address, BYTES_IN_MAC_ADDRESS );
  

    /* insert type */
    buffer[ETH_TYPE_BEGIN_PLACE] = (unsigned char)((const_ethernet_type >> BITS_IN_BYTE) & 0xff);
    buffer[ETH_TYPE_BEGIN_PLACE+1] = (unsigned char)((const_ethernet_type  & 0xff));


    return result;
	
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Print_remote_msg"
static int Print_remote_msg (
						  const PON_olt_id_t            olt_id, 
						  const PON_onu_id_t            onu_id,
						  unsigned char                *sent_data, 
						  const unsigned short          sent_data_size,
						  bool                          send_flag)
{

	char                  out_str[MAX_OAM_PDU_SIZE*3 + 100/* for \n */]; 
	const unsigned short  out_str_size = sizeof(out_str); 
    short int             result = REMOTE_COMMUNICATION_EXIT_OK, len;
	char			      hex_str[15];
    long int              hex_val = 0;
	
	/* if not going to be printed, return */
	if (!PONLOG_check_configuration_for_print(PONLOG_SEVERITY_INFO, PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id))
	{
		return (result);
	}


	if(send_flag)
	{

	/* print ETHERNET layer */
	strcpy(out_str, "\nETHERNET DA = ");
	len = strlen(out_str);
	PON_general_hex_bytes_to_string(sent_data+ETH_DA_BEGIN_PLACE, BYTES_IN_MAC_ADDRESS, out_str+len, out_str_size-len);

	strcat(out_str, "\nETHERNET SA = ");
	len = strlen(out_str);
	PON_general_hex_bytes_to_string(sent_data+ETH_SA_BEGIN_PLACE, BYTES_IN_MAC_ADDRESS, out_str+len, out_str_size-len);

	strcat(out_str, "\nETHERNET type = ");
	len = strlen(out_str);
	UBUFFER_2_USHORT((sent_data+ETH_TYPE_BEGIN_PLACE), hex_val);
	sprintf(hex_str, "0X%X ", hex_val);
	strcat(out_str, hex_str);
	/*PON_general_hex_bytes_to_string(sent_data+ETH_TYPE_BEGIN_PLACE, BYTES_IN_WORD, out_str+len, out_str_size-len);*/

	/* print OAM layer */
	strcat(out_str, "\nOAM sub_type = ");
	len = strlen(out_str);
	WORD_UBUFFER_2_ENDIANITY_UCHAR((sent_data+OAM_HEADER_PLACE+OAM_SUB_TYPE_PLACE), hex_val);
	sprintf(hex_str, "0X%0X ", hex_val);
	strcat(out_str, hex_str);
	/*PON_general_hex_bytes_to_string(sent_data+OAM_HEADER_PLACE+OAM_SUB_TYPE_PLACE, BYTES_IN_CHAR, out_str+len, out_str_size-len);*/
	
	strcat(out_str, "\nOAM flags = ");
	len = strlen(out_str);
	UBUFFER_2_USHORT((sent_data+OAM_HEADER_PLACE+OAM_FLAGS_PLACE), hex_val);
	sprintf(hex_str, "0X%X ", hex_val);
	strcat(out_str, hex_str);
	/*PON_general_hex_bytes_to_string(sent_data+OAM_HEADER_PLACE+OAM_FLAGS_PLACE, BYTES_IN_WORD, out_str+len, out_str_size-len);*/

	strcat(out_str, "\nOAM code = ");
	len = strlen(out_str);
	WORD_UBUFFER_2_ENDIANITY_UCHAR((sent_data+OAM_HEADER_PLACE+OAM_CODE_PLACE), hex_val);
	sprintf(hex_str, "0X%0X ", hex_val);
	strcat(out_str, hex_str);
	/*PON_general_hex_bytes_to_string(sent_data+OAM_HEADER_PLACE+OAM_CODE_PLACE, BYTES_IN_CHAR, out_str+len, out_str_size-len);*/

	strcat(out_str, "\nOAM OUI = ");
	len = strlen(out_str);
	PON_general_hex_bytes_to_string(sent_data+OAM_HEADER_PLACE+OAM_OUI_PLACE, BYTES_IN_WORD+BYTES_IN_CHAR, out_str+len, out_str_size-len);
	
	strcat(out_str, "\nOAM magic_number = ");
	len = strlen(out_str);
	WORD_UBUFFER_2_ENDIANITY_UCHAR((sent_data+OAM_HEADER_PLACE+OAM_MAGIC_NUMBER_PLACE), hex_val);
	sprintf(hex_str, "0X%X ", hex_val);
	strcat(out_str, hex_str);
	/*PON_general_hex_bytes_to_string(sent_data+OAM_HEADER_PLACE+OAM_MAGIC_NUMBER_PLACE, BYTES_IN_CHAR, out_str+len, out_str_size-len);*/
	
	/* print LLC layer */
	strcat(out_str, "\nLLC msg_id = ");
	len = strlen(out_str);
	PON_general_hex_bytes_to_string(sent_data+LLC_HEADER_PLACE+MSG_ID_PLACE, BYTES_IN_LONG, out_str+len, out_str_size-len);
	
    UBUFFER_2_ENDIANITY_ULONG((sent_data+LLC_HEADER_PLACE+MSG_ID_PLACE), hex_val);
	sprintf(hex_str, " (%d)", hex_val);
	strcat(out_str, hex_str);
    
	strcat(out_str, "\nLLC PDU_type = ");
	len = strlen(out_str);
	PON_general_hex_bytes_to_string(sent_data+LLC_HEADER_PLACE+PDU_TYPE_PLACE, BYTES_IN_WORD, out_str+len, out_str_size-len);
	
	UBUFFER_2_ENDIANITY_USHORT((sent_data+LLC_HEADER_PLACE+PDU_TYPE_PLACE), hex_val);
	switch(hex_val)
	{
		case 0x00:
		strcpy(hex_str, " (0 - PDU_TYPE_REQUEST)");
		break;
		case 0x01:
		strcpy(hex_str, " (1 - PDU_TYPE_RESPONSE)");
		break;
		case 0x02:
		strcpy(hex_str, " (2 - PDU_TYPE_EVENT)");
		break;
		default:
		break;
	}
	strcat(out_str, hex_str);

	/* print App layer */
	strcat(out_str, "\nApp = ");
	len = strlen(out_str);
	PON_general_hex_bytes_to_string(sent_data+APPLICATION_HEADER_PLACE, sent_data_size-APPLICATION_HEADER_PLACE, out_str+len, out_str_size-len);

	PONLOG_INFO_3(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id,
				  "Remote management frame send to OLT %d, ONU %d :\n%s\n\n", 
				  olt_id, onu_id, out_str);
	}
	else
	{

	/* print LLC layer */
	strcpy(out_str, "\nLLC msg_id = ");
	len = strlen(out_str);
	PON_general_hex_bytes_to_string(sent_data+MSG_ID_PLACE, BYTES_IN_LONG, out_str+len, out_str_size-len);

    UBUFFER_2_ENDIANITY_ULONG((sent_data+MSG_ID_PLACE), hex_val);
	sprintf(hex_str, " (%d)", hex_val);
	strcat(out_str, hex_str);
    
	strcat(out_str, "\nLLC PDU_type = ");
	len = strlen(out_str);
	PON_general_hex_bytes_to_string(sent_data+PDU_TYPE_PLACE, BYTES_IN_WORD, out_str+len, out_str_size-len);
	
	UBUFFER_2_ENDIANITY_USHORT((sent_data+PDU_TYPE_PLACE), hex_val);
	switch(hex_val)
	{
		case 0x00:
		strcpy(hex_str, " (0 - PDU_TYPE_REQUEST)");
		break;
		case 0x01:
		strcpy(hex_str, " (1 - PDU_TYPE_RESPONSE)");
		break;
		case 0x02:
		strcpy(hex_str, " (2 - PDU_TYPE_EVENT)");
		break;
		default:
		break;
	}
	strcat(out_str, hex_str);

	/* print App layer */
	strcat(out_str, "\nApp = ");
	len = strlen(out_str);
	PON_general_hex_bytes_to_string(sent_data+LLC_HEADER_SIZE, sent_data_size-LLC_HEADER_SIZE, out_str+len, out_str_size-len);

	PONLOG_INFO_3(PONLOG_FUNC_REMOTE_MGMT_COMM, olt_id, onu_id,
	    		  "Remote management frame receive from OLT %d, ONU %d :\n%s\n\n", 
				  olt_id, onu_id, out_str);
	}

	return result;
}

#undef PONLOG_FUNCTION_NAME

#endif

