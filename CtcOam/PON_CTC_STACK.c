/*  
**  CTC_STACK.c - CTC package implementation
**
**  
**  This software is licensed for use according to the terms set by the Passave API license agreement.
**  Copyright Passave Ltd. 
**  Ackerstein Towers - A, 9 Hamenofim St.
**  POB 2089, Herziliya Pituach 46120 Israel
**
**
**  This file was written by Meital Levy, meital.levy@passave.com, 13/03/2006
**  
**  Changes:
**
**  Version	  |  Date	   |    Change	             |    Author	  
**  ----------+------------+-------------------------+------------
**	1.00	  | 13/03/2006 |	Creation		     | Meital Levy
*/

/*============================= Include Files ===============================*/	
#include "OnuGeneral.h"
#include "PonEventHandler.h"
#include "PonMicroParser.h"

#include "../StdOam/PonOam.h"
#include "../StdOam/PonStdOam.h"
#include "manage/private_mibs/efm/mib_efm.h"

#include "PON_CTC_STACK_defines.h"
#include "PON_CTC_STACK_variable_descriptor_defines_expo.h"
#include "PON_CTC_STACK_expo.h"
#include "PON_CTC_STACK_non_api_expo.h"
#include "PON_CTC_STACK_frame_struct.h"
#include "PON_CTC_STACK_comparser_extended_oam.h"
#include "PON_CTC_STACK_comparser.h"
#include "PON_CTC_STACK_comm.h"

#include "IncludeFromBcm.h"


#undef index  /* sn_conf.h 里定义了index宏 */

#undef __MODULE__

#define __MODULE__ "CTC_STACK" /* File name for log headers */
#define ___FILE___ "" /* File name for log headers */


#define CTC_MEM_DYNAMIC

#undef  CTC_AUTH_SUPPORT
#undef  CTC_RDN_SUPPORT

#define PON_CHIPTYPE_KNOWN

#define FILES_NOT_SUPPORTED

#define PON_OLT_ID_ARRAY_SIZE          (8)         /* 目前只有BCM8PON板需要此CTC协议栈 */

/*============================== Data Types =================================*/                             

typedef struct 
{
    unsigned char              number_of_oui_records;
    CTC_STACK_oui_version_record_t   records_list[MAX_OUI_RECORDS];
}ctc_stack_discovery_list_t;


#ifdef CTC_MEM_DYNAMIC

typedef struct 
{
    ctc_stack_discovery_list_t          list_info;
    CTC_STACK_discovery_state_t         (*state)[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
    unsigned short                      timeout_in_100_ms;
	unsigned short						(*maximum_data_frame_size)[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
    /* B--added by liwei056@2014-8-8 for OtherExtOam */
#ifdef PON_CHIPTYPE_KNOWN
	unsigned short						(*pon_chip_vendor)[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
#endif
    /* E--added by liwei056@2014-8-8 for OtherExtOam */
	unsigned char						(*common_version)[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
}ctc_stack_discovery_information_t;

#else

typedef struct 
{
    ctc_stack_discovery_list_t          list_info;
    CTC_STACK_discovery_state_t         state[PON_OLT_ID_ARRAY_SIZE][PON_ONU_ID_PER_OLT_ARRAY_SIZE];
    unsigned short                      timeout_in_100_ms;
	unsigned short						maximum_data_frame_size[PON_OLT_ID_ARRAY_SIZE][PON_ONU_ID_PER_OLT_ARRAY_SIZE];
    /* B--added by liwei056@2014-8-8 for OtherExtOam */
#ifdef PON_CHIPTYPE_KNOWN
	unsigned short						pon_chip_vendor[PON_OLT_ID_ARRAY_SIZE][PON_ONU_ID_PER_OLT_ARRAY_SIZE];
#endif
    /* E--added by liwei056@2014-8-8 for OtherExtOam */
	unsigned char						common_version[PON_OLT_ID_ARRAY_SIZE][PON_ONU_ID_PER_OLT_ARRAY_SIZE];
}ctc_stack_discovery_information_t;

#endif


typedef struct 
{
    unsigned char                       update_key_time_in_sec;
    unsigned short                      no_reply_timeout_in_100_ms;
    unsigned char                       start_encryption_threshold_in_sec;
}ctc_stack_encryption_information_t;


typedef struct  
{
    CTC_STACK_encryption_onu_database_t onu_id[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
    /* B--modified by liwei056@2010-9-26 for CodeCheck */
#if 1
    PON_encryption_type_t               encryption_type;
#else
    PON_encryption_key_index_t          encryption_type;
#endif
    /* E--modified by liwei056@2010-9-26 for CodeCheck */
    OSSRV_semaphore_t                   semaphore;

}encryption_olt_database_t;


typedef struct 
{
#ifdef CTC_MEM_DYNAMIC
    encryption_olt_database_t              *olt;
#else
    encryption_olt_database_t               olt[PON_OLT_ID_ARRAY_SIZE];
#endif
    ctc_stack_encryption_information_t      timing_info;
    
}encryption_database_t;


typedef struct
{
    bool                    ctc_stack_is_init;
    unsigned char           version;
    unsigned long           ctc_stack_oui;
}ctc_stack_general_information_t;


typedef enum
{
    CTC_STACK_ONU_STATE_NOT_DURING_DOWNLOADING,
	CTC_STACK_ONU_STATE_DURING_DOWNLOADING
}ctc_stack_onu_downloading_state_t;


typedef struct  
{
    ctc_stack_onu_downloading_state_t       onu_state[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
    OSSRV_semaphore_t						semaphore;
}update_firmware_olt_database_t;


typedef struct 
{
    PON_olt_id_t			  olt_id; 
    PON_onu_id_t			  onu_id;
    unsigned char			  file_name[ONU_RESPONSE_SIZE];
	OSSRV_semaphore_t		  semaphore;
}global_onu_request_event_data_t;

  
#ifdef CTC_AUTH_SUPPORT
typedef struct 
{
    authentication_olt_database_t    olt[PON_OLT_ID_ARRAY_SIZE];
}authentication_database_t;
#endif

typedef struct
{
#ifdef CTC_MEM_DYNAMIC
	CTC_STACK_onu_capabilities_3_t (*onu)[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
#else
	CTC_STACK_onu_capabilities_3_t onu[PON_OLT_ID_ARRAY_SIZE][PON_ONU_ID_PER_OLT_ARRAY_SIZE];
#endif
}ctc_stack_onu_capabilities_3_database_t;

/*================================ Constants ===================================*/


/* timeout for no reply during discovery process */
#define CTC_STACK_DEFAULT_DISCOVERY_TIMEOUT                 30/* in 100 ms */

#define CTC_STACK_MAX_DISCOVERY_TIMEOUT						2550/* in 100 ms */



/* timeout for no reply during encryption process */
#define CTC_STACK_DEFAULT_ENCRYPTION_TIMEOUT                30/* in 100 ms */

#define CTC_STACK_MAX_ENCRYPTION_TIMEOUT					2550/* in 100 ms */

/* Encryption update key timer */
#define CTC_STACK_DEFAULT_ENCRYPTION_UPDATE_KEY_TIME        10/* in sec */

#define CTC_STACK_MAX_ENCRYPTION_UPDATE_KEY_TIME			255/* in sec */

/* Encryption threshold between 'CTC_STACK_start_encryption' API and the next encryption ativation */
#define START_ENCRYPTION_DEFAULT_THRESHOLD_TIME_DEFINE      0/* in sec */

#define START_ENCRYPTION_MAX_THRESHOLD_TIME_DEFINE          255/* in sec */



#define ENCRYPTION_MAX_RETRIES_DEFINE                       3
#define ENCRYPTION_MAX_REQUESTS_TO_ONU_DEFINE               ENCRYPTION_MAX_RETRIES_DEFINE+1/*first sending in T update key*/



#define WAIT_FOR_ONU_REQUEST_EVENT_HANDLING_CHECK			10    /* Milliseconds */
#define WAIT_FOR_ONU_REQUEST_EVENT_HANDLING_TIMEOUT			2.0   /* Seconds, can be fraction, timeout waiting for reset event handling */
#define WAIT_FOR_ONU_REQUEST_EVENT_HANDLING_LOOP_TIMEOUT	(WAIT_FOR_ONU_REQUEST_EVENT_HANDLING_TIMEOUT * 1000 / WAIT_FOR_ONU_REQUEST_EVENT_HANDLING_CHECK)


/* OAM discovery patch in case the ONU returns the second response for the first request */ 
/*#define TIMEOUT_PATCH_FROM_ONU_DURING_DISCOVERY*/


#define PASCOMM_ONU_FIRMWARE_BINARY_CHUNK_OVERHEAD_SIZE  32
#define PASCOMM_ONU_FIRMWARE_BINARY_CHUNK_DEFAULT_SIZE   (128-PASCOMM_ONU_FIRMWARE_BINARY_CHUNK_OVERHEAD_SIZE)

/*================================ Macros ===================================*/

#define RECEIVED_COMPARSER_BUFFER_OLD_COMMANDS					(received_oam_pdu_data + received_left_length)

#define SEND_COMPARSER_BUFFER						sent_oam_pdu_data+OAM_OUI_SIZE + (ETHERNET_MTU - sent_length)

#define RECEIVED_COMPARSER_BUFFER_TFTP received_oam_pdu_data+TFTP_COMMON_HEADER+1



#define RECEIVED_LENGTH_BUFFER (unsigned short)(received_length - received_total_length)

#define RECEIVED_COMPARSER_BUFFER received_oam_pdu_data + received_total_length


#define PAS_OLT_IS_PAS5001(__olt_id__) (PONCHIP_PAS5001 == GetPonChipTypeByPonPort(__olt_id__))

/*========================== External Variables =============================*/
extern bool CTC_COMPOSER_global_set_response_contains_data;

/*========================== External Functions =============================*/
/*============================== Variables ==================================*/

/*=========================== Internal variables =============================*/

static ctc_stack_general_information_t      ctc_stack_general_information = {0};


/* Discovery process information */
static ctc_stack_discovery_information_t    discovery_db;

/* Automatic ONU configuration */
static bool									internal_ctc_stack_automatic_onu_configuration_mode = FALSE;

#ifdef CTC_AUTH_SUPPORT
/* authentication process information */
static authentication_database_t            authentication_db;
#endif

/* Encryption process information */
static bool                                 internal_ctc_stack_automatic_mode = FALSE;

static encryption_database_t                encryption_db;

/* encryption scheduler semaphore for protecting olt's semaphore termination (in terminate function)
   during task activation */
static OSSRV_semaphore_t                    encryption_scheduler_thread_semaphore;

/* encryption timing parameters configuration semaphore */
static OSSRV_semaphore_t                    encryption_timing_parameters_semaphore;

/* encryption scheduler termination flag */
static bool                                 encryption_scheduler_thread_active  = FALSE;


/* encryption process termination flag */
static bool                                 encryption_process_thread_active    = FALSE;

/* encryption process task */
#if 0
static PAS_task_init_params_t               encryption_process_task_init_params;
static TASKS_thread_id_t                    encryption_process_task_id;
#endif

/* encryption last update key time iteration */
static time_t encryption_last_update_key_time;

/* encryption last timeout time iteration */
static time_t encryption_last_timeout_time;

/* encryption thread semaphore */
static OSSRV_semaphore_t   encryption_thread_semaphore;

static PON_handlers_t                 ctc_db_handlers;

/* indicates whether  it is the first update key iteration*/
static bool first_update_key_iteration = TRUE;


/* Firmware download information */
#ifdef CTC_MEM_DYNAMIC
static update_firmware_olt_database_t *update_firmware_db;
#else
static update_firmware_olt_database_t update_firmware_db[PON_OLT_ID_ARRAY_SIZE];
#endif

static short int Set_onu_state_in_update_firmware_db
                ( const PON_olt_id_t                       olt_id, 
                  const PON_onu_id_t					   onu_id,
				  const ctc_stack_onu_downloading_state_t  state );


static global_onu_request_event_data_t		global_onu_request_event_data;


static unsigned short registration_handler_id;
static unsigned short deregistration_handler_id;
static unsigned short add_olt_handler_id;
static unsigned short remove_olt_handler_id;
static unsigned short onu_authorization_handler_id;


static const CTC_STACK_alarm_str_t alarm_str_array[] =	
{
    /*alarm_str*/                   /*alarm_id*/                     /*alarm_info_size*/    /*alarm_info_type*/
    {"Equipment Alarm"		    	,EQUIPMENT_ALARM		         ,BYTES_IN_LONG       ,FAILURE_CODE_ALARM_INFO_TYPE},
    {"Powering Alarm"			    ,POWERING_ALARM			         ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE},	          
    {"Battery Missing"		    	,BATTERY_MISSING		         ,0                   ,NONE_ALARM_INFO_TYPE},             
    {"Battery Failure"			    ,BATTERY_FAILURE			     ,0                   ,NONE_ALARM_INFO_TYPE},             
    {"Battery Volt Low"			    ,BATTERY_VOLT_LOW			     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE}, 
    {"Physical Intrusion Alarm"     ,PHYSICAL_INTRUSION_ALARM        ,0                   ,NONE_ALARM_INFO_TYPE},             
    {"ONU Self Test Failure"        ,ONU_SELF_TEST_FAILURE           ,BYTES_IN_LONG       ,FAILURE_CODE_ALARM_INFO_TYPE}, 
    {"ONU Temp High Alarm"		    ,ONU_TEMP_HIGH_ALARM		     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE}, 
    {"ONU Temp Low Alarm"		    ,ONU_TEMP_LOW_ALARM		         ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE}, 
	{"IAD Connection Failure"       ,IAD_CONNECTION_FAILURE          ,BYTES_IN_LONG       ,FAILURE_CODE_ALARM_INFO_TYPE},          
    {"PON If Switch"				,PON_IF_SWITCH		             ,BYTES_IN_LONG       ,FAILURE_CODE_ALARM_INFO_TYPE},          
    {"RX Power High Alarm"		    ,RX_POWER_HIGH_ALARM		     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE}, 
    {"RX Power Low Alarm"		    ,RX_POWER_LOW_ALARM		         ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE}, 
    {"TX Power High Alarm"		    ,TX_POWER_HIGH_ALARM		     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE}, 
    {"TX Power Low Alarm"		    ,TX_POWER_LOW_ALARM		         ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE}, 
    {"TX Bias High Alarm"		    ,TX_BIAS_HIGH_ALARM		         ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE}, 
    {"TX Bias Low Alarm"		    ,TX_BIAS_LOW_ALARM		         ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE}, 
    {"Vcc High Alarm" 			    ,VCC_HIGH_ALARM 			     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE}, 
    {"Vcc Low Alarm"  			    ,VCC_LOW_ALARM  			     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE}, 
    {"Temp High Alarm"			    ,TEMP_HIGH_ALARM			     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE},
    {"Temp Low Alarm"		        ,TEMP_LOW_ALARM		             ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE},
    {"RX Power High Warning"	    ,RX_POWER_HIGH_WARNING	         ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE},
    {"RX Power Low Warning"		    ,RX_POWER_LOW_WARNING		     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE},
    {"TX Power High Warning"	    ,TX_POWER_HIGH_WARNING	         ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE},
    {"TX Power Low Warning"		    ,TX_POWER_LOW_WARNING		     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE},
    {"TX Bias High Warning"		    ,TX_BIAS_HIGH_WARNING		     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE},
    {"TX Bias Low Warning"		    ,TX_BIAS_LOW_WARNING		     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE},
    {"Vcc High Warning"			    ,VCC_HIGH_WARNING			     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE},
    {"Vcc Low Warning"			    ,VCC_LOW_WARNING			     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE},
    {"Temp High Warning"		    ,TEMP_HIGH_WARNING		         ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE},
    {"Temp Low Warning"			    ,TEMP_LOW_WARNING			     ,BYTES_IN_LONG       ,TEST_VALUE_ALARM_INFO_TYPE}, 
    {"Card Alarm"    			    ,CARD_ALARM    			         ,BYTES_IN_LONG       ,FAILURE_CODE_ALARM_INFO_TYPE}, 
    {"Self Test Failure"		    ,SELF_TEST_FAILURE		         ,BYTES_IN_LONG       ,FAILURE_CODE_ALARM_INFO_TYPE}, 
    {"Eth Port Auto Neg Failure"    ,ETH_PORT_AUTO_NEG_FAILURE       ,0                   ,NONE_ALARM_INFO_TYPE},             
    {"Eth Port LOS"   			    ,ETH_PORT_LOS   			     ,0                   ,NONE_ALARM_INFO_TYPE},             
    {"Eth Port Failure"             ,ETH_PORT_FAILURE                ,0                   ,NONE_ALARM_INFO_TYPE},             
    {"Eth Port Loopback"		    ,ETH_PORT_LOOPBACK		         ,0                   ,NONE_ALARM_INFO_TYPE},             
    {"Eth Port Congestion"		    ,ETH_PORT_CONGESTION		     ,0                   ,NONE_ALARM_INFO_TYPE},             
    {"POTS Port Failure"		    ,POTS_PORT_FAILURE		         ,BYTES_IN_LONG       ,FAILURE_CODE_ALARM_INFO_TYPE}, 
    {"E1 Port Failure"			    ,E1_PORT_FAILURE			     ,BYTES_IN_LONG       ,FAILURE_CODE_ALARM_INFO_TYPE}, 
    {"E1 Timing Unlock"			    ,E1_TIMING_UNLOCK			     ,0                   ,NONE_ALARM_INFO_TYPE},             
    {"E1 LOS"           		    ,E1_LOS           		         ,0                   ,NONE_ALARM_INFO_TYPE},             
    /* Do not remove the line below */
    {"",0,0,0}
    /* Do not remove the line above*/        
};                                                     

/*Redundancy data block get temp variables*/
static short int rdn_discovery_onu_id[PON_OLT_ID_ARRAY_SIZE];
static short int rdn_encryption_onu_id[PON_OLT_ID_ARRAY_SIZE];
static short int rdn_authentication_onu_id[PON_OLT_ID_ARRAY_SIZE];

/*ONU capabilities-3 DB*/
static ctc_stack_onu_capabilities_3_database_t onu_capabilities_3_db; 

/*======================= Internal Functions Prototype ======================*/

static short int Encryption_scheduler_thread_hook_func(void *data);
static short int Encryption_process_thread_hook_func(void *data);

static PON_STATUS New_olt_added_handler (const PON_olt_id_t  olt_id);
static PON_STATUS Removed_olt_handler (const PON_olt_id_t  olt_id);

static void Received_new_churning_key  ( 
                   const PON_olt_id_t             olt_id,
                   const PON_onu_id_t             onu_id,
                   const unsigned short           length,
                   unsigned char                 *content );

static short int Encryption_sequence_to_olt(
                      const PON_olt_id_t         olt_id, 
                      const PON_onu_id_t         onu_id,
                      const bool                 generate_event);

static short int Send_new_key_request ( 
                     const PON_olt_id_t                 olt_id, 
                     const PON_onu_id_t                 onu_id,
                     const PON_encryption_key_index_t   key_index );

int Registration_handler_discovery_process2 
				   ( const PON_olt_id_t					  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const mac_address_t				  mac_address,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard,
					 const bool event_report );

/* B--added by liwei056@2011-12-5 for 6700's CTCOnuMgtFailed After SwitchOver */
static int extend_discovery_process 
				   ( const PON_olt_id_t					  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const mac_address_t				  mac_address,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard);

static int resume_discovery_process 
				   ( const PON_olt_id_t					  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const mac_address_t				  mac_address,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard );
/* E--added by liwei056@2011-12-5 for 6700's CTCOnuMgtFailed After SwitchOver */

static int Deregistration_handler 
				   ( const PON_olt_id_t						olt_id,
					 const PON_onu_id_t					    onu_id,
					 const PON_onu_deregistration_code_t    deregistration_code );

int ctc_pon_event_handler(PON_event_handler_index_t event_id, void *event_data);

#if 0
static int Onu_authorization_handler 
				   ( const PON_olt_id_t						olt_id,
					 const PON_onu_id_t					    onu_id,
					 const PON_authorize_mode_t             authorize_mode );
#endif

static short int Convert_comparser_error_code_to_ctc_stack( short int result);

static short int Convert_ponsoft_error_code_to_ctc_stack( short int pas_result);

static short int Convert_comm_error_code_to_ctc_stack( short int result);

static short int Convert_ctc_stack_error_codes_to_tftp( unsigned short ctc_error_code);

static short int Send_receive_ctc_message  ( 
                      const PON_olt_id_t                 olt_id, 
                      const PON_onu_id_t                 onu_id,
                      const PON_oam_code_t               send_oam_code,
                      unsigned char                     *send_data,  
                      const unsigned short               send_data_size,
                      unsigned short*                   timeout_in_100_ms,
                      const unsigned char                expected_opcode,
                      const unsigned char                expected_code,
                      unsigned char                     *received_data,
                      unsigned short                    *received_data_size,
                      unsigned short                     allocated_received_data_size,
                      const general_handler_function_t   handler_function,
					  const short int 					 request_id);


static short int Send_receive_vendor_extension_message  ( 
                      const PON_olt_id_t                 olt_id, 
                      const PON_onu_id_t                 onu_id,
                      unsigned char                     *send_data,  
                      const unsigned short               send_data_size,
                      const unsigned char                expected_opcode,
                      const unsigned char                expected_code,
                      unsigned char                     *received_data,
                      unsigned short                    *received_data_size,
                      unsigned short                     allocated_received_data_size,
                      const general_handler_function_t   handler_function);



static short int Send_receive_vendor_extension_message_with_timeout  ( 
                      const PON_olt_id_t                 olt_id, 
                      const PON_onu_id_t                 onu_id,
                      unsigned char                     *send_data,  
                      const unsigned short               send_data_size,
					  const unsigned short               timeout_in_100_ms,
                      const unsigned char                expected_opcode,
                      const unsigned char                expected_code,
                      unsigned char                     *received_data,
                      unsigned short                    *received_data_size,
                      unsigned short                     allocated_received_data_size,
                      const general_handler_function_t   handler_function);

static short int Send_receive_vendor_extension_message_with_time_out_request_id ( 
                      const PON_olt_id_t                 olt_id, 
                      const PON_onu_id_t                 onu_id,
                      unsigned char                     *send_data,  
                      const unsigned short               send_data_size,
					  unsigned short                    *timeout_in_100_ms,
                      const unsigned char                expected_opcode,
                      const unsigned char                expected_code,
                      unsigned char                     *received_data,
                      unsigned short                    *received_data_size,
                      unsigned short                     allocated_received_data_size,
                      const general_handler_function_t   handler_function,
					  const short int 					 request_id);

static short int Check_if_onu_support
                    ( const PON_olt_id_t               olt_id, 
					  const PON_onu_id_t               onu_id,
                      const bool                       received_ext_support,
                      const unsigned char 	           receive_number_of_records,
                      const CTC_STACK_oui_version_record_t  *receive_oui_version_record_array,
                      bool                            *support);

static short int Copy_encryption_key
                    ( const CTC_encryption_key_t  src_key, 
                      CTC_encryption_key_t        dest_key);


static short int Update_encryption_db  ( 
                   const PON_olt_id_t                olt_id,
                   const PON_onu_id_t                onu_id,
                   const PON_encryption_key_index_t  key_index,
                   const CTC_encryption_key_t        key_value );

static short int Check_encryption_state_if_already_received  ( 
                   const PON_olt_id_t   olt_id,
                   const PON_onu_id_t   onu_id);


static short int Set_encryption_db_flag_entry  ( 
                   const PON_olt_id_t                           olt_id,
                   const PON_onu_id_t                           onu_id,
                   const ctc_stack_response_encryption_state_t  state);

static short int Set_encryption_db_entry  ( 
                   const PON_olt_id_t                  olt_id,
                   const PON_onu_id_t                  onu_id,
                   const CTC_STACK_encryption_state_t  encryption_state );


static short int Check_first_version
                    ( const CTC_STACK_oui_version_record_t  *receive_oui_version_record_array,
                      bool                            *correct_oui_zero_version);


static void Check_threshold_time_according_update_key_time( bool  *need_to_send);

static void Check_threshold_time_according_timeout_time ( bool  *need_to_send);

static bool Check_if_need_to_send_according_number_of_retransmit ( 
              const PON_olt_id_t   olt_id, 
              const PON_onu_id_t   onu_id );

static void Send_update_key_iteration();

static void Send_timeout_iteration();


static short int Send_timeout ( 
              const PON_olt_id_t                 olt_id, 
              const PON_onu_id_t                 onu_id,
              const PON_encryption_key_index_t   key_index );

static void Scheduler_sleep(
                     unsigned long  time_to_sleep,
                     unsigned long   loop_iteration_time );


static void Reset_onu_entry(
                   const PON_olt_id_t                  olt_id,
                   const PON_onu_id_t                  onu_id);


static void Copy_oui_as_number( 
           const unsigned long     src_oui,
           OAM_oui_t			   dest_oui );


static short int Send_write_request ( 
               const PON_olt_id_t    olt_id, 
               const PON_onu_id_t    onu_id,
               const unsigned char  *file_name );

static short int Send_file(
			const PON_olt_id_t          olt_id, 
            const PON_onu_id_t          onu_id,
            const CTC_STACK_binary_t   *onu_firmware,
			const bool	        		send_write_request,
			const unsigned char        *file_name,
            unsigned long              *file_size);

static short int Send_binary_source(
			const PON_olt_id_t          olt_id, 
            const PON_onu_id_t          onu_id,
            const CTC_STACK_binary_t   *onu_firmware,
            void			           *stream,/*required only for FILE source*/
			const unsigned char        *file_name,
            const unsigned long         file_size);

static short int Send_end_download_request(
			const PON_olt_id_t    olt_id, 
            const PON_onu_id_t    onu_id,
            const unsigned char  *file_name,
            const unsigned long   file_size);

static void Onu_request_handler_function
                ( const PON_olt_id_t     olt_id, 
                  const PON_onu_id_t	 onu_id,
				  const unsigned short   length,
                  unsigned char         *content,
                  unsigned char          received_code);

static short int Onu_request_update_firmware
                ( const PON_olt_id_t     olt_id, 
                  const PON_onu_id_t	 onu_id,
				  const unsigned short   length,
                  unsigned char         *content);

short int ctc_event_notification
                ( const PON_olt_id_t     olt_id, 
                  const PON_onu_id_t	 onu_id,
				  const unsigned short   received_length,
                  unsigned char         *content);

static short int Send_file_and_verify ( 
                 const PON_olt_id_t          olt_id, 
                 const PON_onu_id_t          onu_id,
                 const CTC_STACK_binary_t   *onu_firmware,
				 const bool			         send_write_request,
				 const unsigned char        *file_name);

static short int PON_CTC_STACK_get_pmc_onu_version_handler
                                                   ( const PON_olt_id_t			  olt_id, 
													 const PON_onu_id_t			  onu_id,
													       PAS_pmc_onu_version_t *pmc_onu_version );

static short int  Get_chipset_id ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id,
		   CTC_STACK_chipset_id_t  *chipset_id );


static bool Search_largest_common_oui_version(
                    const unsigned char 	               receive_number_of_records,
					const CTC_STACK_oui_version_record_t  *receive_oui_version_record_array,
                    unsigned char 	                      *largest_common_version);

PON_STATUS PON_CTC_STACK_get_onu_ponchip_vendor( const PON_olt_id_t			  olt_id, 
            													 const PON_onu_id_t			  onu_id,
            													 unsigned short              *vendor_type,
            													 unsigned short              *chip_type,
            													 unsigned short              *rev_type);

PON_STATUS PON_CTC_STACK_get_onu_pon_vendor (  
           const PON_olt_id_t			   olt_id, 
           const PON_onu_id_t			   onu_id,
		   OnuVendorTypes  *pon_vendor_types );

PON_STATUS PON_CTC_STACK_get_onu_frame_size (  
           const PON_olt_id_t			   olt_id, 
           const PON_onu_id_t			   onu_id,
		   unsigned short  *onu_frame_size );


static short int PON_CTC_STACK_start_loid_authentication ( 
                                         const PON_olt_id_t                 olt_id, 
                                         const PON_onu_id_t                 onu_id,
                                         bool                              *auth_success);

short int PON_CTC_STACK_auth_request ( 
                                         const PON_olt_id_t                 olt_id, 
                                         const PON_onu_id_t                 onu_id,
                                         CTC_STACK_auth_response_t         *auth_response);

short int PON_CTC_STACK_auth_success ( 
                                         const PON_olt_id_t                 olt_id, 
                                         const PON_onu_id_t                 onu_id);

short int PON_CTC_STACK_auth_failure ( 
                                         const PON_olt_id_t                  olt_id, 
                                         const PON_onu_id_t                  onu_id,
                                         const CTC_STACK_auth_failure_type_t failure_type );

static short int set_sip_digit_map(const PON_olt_id_t							olt_id, 
									const PON_onu_id_t							onu_id, 
									const CTC_STACK_SIP_digit_map_t             sip_digit_map,
									const unsigned char						    block_offset,
									const unsigned char                         block_in_oam,
									const unsigned char							total_block);


#if 0
short int init_rdn_data_variables();

short int assemble_rdn_data_discovery(
    const PON_olt_id_t             olt_id,
            const short int                   max_onu_entry,
            char*                                info_data_buf,
            int                                     info_buf_len,
            int*                                   info_length,
            bool*                                 first_block,
            bool *                                fragment);

short int disperse_rdn_data_discovery(
    const PON_olt_id_t             olt_id,
            char*                                info_data_buf,
            int                                     info_buf_len,
            int                                    info_length,
            bool                                  first_block);

short int assemble_rdn_data_authentication(
    const PON_olt_id_t             olt_id,
            const short int                   max_onu_entry,
            char*                                info_data_buf,
            int                                     info_buf_len,
            int*                                   info_length,
            bool*                                 first_block,
            bool *                                fragment);

short int disperse_rdn_data_authentication(
    const PON_olt_id_t             olt_id,
            char*                                info_data_buf,
            int                                     info_buf_len,
            int                                    info_length,
            bool                                  first_block);

short int assemble_rdn_data_encryption(
    const PON_olt_id_t             olt_id,
            const short int                   max_onu_entry,
            char*                                info_data_buf,
            int                                     info_buf_len,
            int*                                   info_length,
            bool*                                 first_block,
            bool *                                fragment);

short int disperse_rdn_data_encryption(
    const PON_olt_id_t             olt_id,
            char*                                info_data_buf,
            int                                     info_buf_len,
            int                                    info_length,
            bool                                  first_block);
#endif


/*================================ Functions ================================*/

#ifdef CTC_MEM_DYNAMIC

PON_STATUS PON_CTC_STACK_init_data()
{
    do{
        encryption_olt_database_t *olt_encry_db;
        
        if ( NULL != (olt_encry_db = (encryption_olt_database_t*)g_malloc(sizeof(encryption_olt_database_t) * PON_OLT_ID_ARRAY_SIZE)) )
        {
        	memset((unsigned char *)olt_encry_db, 0, sizeof(encryption_olt_database_t) * PON_OLT_ID_ARRAY_SIZE);
            encryption_db.olt = olt_encry_db;
        }
        else
        {
            VOS_ASSERT(0);
            return CTC_STACK_MEMORY_ERROR;
        }
    }while(0);

    
    do{
        char *olt_disc_db;
        
#ifdef PON_CHIPTYPE_KNOWN
        if ( NULL != (olt_disc_db = (char*)VOS_Malloc(PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE 
                                                  * (sizeof(CTC_STACK_discovery_state_t)
                                                    + sizeof(unsigned short)
                                                    + sizeof(unsigned short)
                                                    + sizeof(unsigned char)), MODULE_RPU_OAM)) )
#else
        if ( NULL != (olt_disc_db = (char*)VOS_Malloc(PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE 
                                                  * (sizeof(CTC_STACK_discovery_state_t)
                                                    + sizeof(unsigned short)
                                                    + sizeof(unsigned char)), MODULE_RPU_OAM)) )
#endif
        {
        	memset((unsigned char *)olt_disc_db, 0, sizeof(CTC_STACK_discovery_state_t) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE);
            discovery_db.state = (CTC_STACK_discovery_state_t*)olt_disc_db;
            olt_disc_db += sizeof(CTC_STACK_discovery_state_t) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE;
            
        	memset((unsigned char *)olt_disc_db, 0, sizeof(unsigned short) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE);
            discovery_db.maximum_data_frame_size = (unsigned short*)olt_disc_db;
            olt_disc_db += sizeof(unsigned short) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE;

#ifdef PON_CHIPTYPE_KNOWN
        	memset((unsigned char *)olt_disc_db, 0, sizeof(unsigned short) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE);
            discovery_db.pon_chip_vendor = (unsigned short*)olt_disc_db;
            olt_disc_db += sizeof(unsigned short) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE;
#endif

        	memset((unsigned char *)olt_disc_db, 0, sizeof(unsigned char) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE);
            discovery_db.common_version = (unsigned char*)olt_disc_db;
        }
        else
        {
            VOS_ASSERT(0);
            return CTC_STACK_MEMORY_ERROR;
        }
    }while(0);

    
    do{
        CTC_STACK_onu_capabilities_3_t *onu_cap3_db;

        if ( NULL != (onu_cap3_db = (CTC_STACK_onu_capabilities_3_t*)VOS_Malloc(sizeof(CTC_STACK_onu_capabilities_3_t) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE, MODULE_RPU_OAM)) )
        {
        	/*init onu capabilities-3 db*/
        	memset((unsigned char *)onu_cap3_db, 0, sizeof(CTC_STACK_onu_capabilities_3_t) * PON_OLT_ID_ARRAY_SIZE * PON_ONU_ID_PER_OLT_ARRAY_SIZE);
            onu_capabilities_3_db.onu = onu_cap3_db;
        }
        else
        {
            VOS_ASSERT(0);
            return CTC_STACK_MEMORY_ERROR;
        }
    }while(0);

    
    do{
        update_firmware_olt_database_t *olt_update_db;
        
        if ( NULL != (olt_update_db = (update_firmware_olt_database_t*)VOS_Malloc(sizeof(update_firmware_olt_database_t) * PON_OLT_ID_ARRAY_SIZE, MODULE_RPU_OAM)) )
        {
        	memset((unsigned char *)olt_update_db, 0, sizeof(update_firmware_olt_database_t) * PON_OLT_ID_ARRAY_SIZE);
            update_firmware_db = olt_update_db;
        }
        else
        {
            VOS_ASSERT(0);
            return CTC_STACK_MEMORY_ERROR;
        }
    }while(0);

    
    return CTC_STACK_EXIT_OK;
}

#endif

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_init"

PON_STATUS PON_CTC_STACK_init 
                ( const bool							 automatic_mode,
                  const unsigned char					 number_of_records,
                  const CTC_STACK_oui_version_record_t  *oui_version_records_list )
{
    short int                   result;
    short int                   index;
    unsigned char               i,oui_index;
    PON_olt_id_t                olt_id;
    PON_llid_t                  onu_id;
    PAS_system_parameters_t     system_parameters;
    CTC_STACK_handler_index_t   function_counter;
	unsigned long               oui = (Get_pon_ctc_oui() << 8); /* due to the fact that we send 32 bits and FW needs 24 bits we shift 8 bits */

    PONLOG_EPON_register_functionality(PONLOG_FUNC_CTC_COMM);

	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

	if(number_of_records > MAX_OUI_RECORDS)
	{
		PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"maximum records list is %d, error to configure %d records list\n", MAX_OUI_RECORDS, number_of_records );   

        return (CTC_STACK_PARAMETER_ERROR);
	}


	if(automatic_mode == FALSE)
	{
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Do not support non-automatic mode\n" );   

        return (CTC_STACK_PARAMETER_ERROR);
	}

    /* terminate before init */
    if(ctc_stack_general_information.ctc_stack_is_init)
    {
		result = PON_CTC_STACK_terminate();
        if( result != CTC_STACK_EXIT_OK )
        {
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"Error in terminate\n" );
	        return CTC_STACK_EXIT_ERROR;
        }
    }
    

#ifdef CTC_MEM_DYNAMIC
    result = PON_CTC_STACK_init_data();
    if( result != CTC_STACK_EXIT_OK )
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error in Init Datas\n" );
        return result;
    }
#else
	/*init onu capabilities-3 db*/
	memset((unsigned char *)&onu_capabilities_3_db, 0, sizeof(ctc_stack_onu_capabilities_3_database_t));

#endif
    

    /* Set CTC OUI */
    /* result = Set_system_oui (oui); */


	ctc_stack_general_information.ctc_stack_oui = Get_pon_ctc_oui();

    /* Init OAM module */
    result = PON_OAM_MODULE_init();
    if(result != EXIT_OK)
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
				   "OAM_MODULE_init failed\n" );
        return result;
    }


    /* Init callback handlers */
    for (function_counter = 0; function_counter < CTC_STACK_HANDLER_LAST_HANDLER /*Last handler*/;function_counter ++)
	{
		PON_general_init_handler_function(&ctc_db_handlers);
	}


#if 0
    result = PAS_set_handler_get_pmc_onu_version(PON_CTC_STACK_get_pmc_onu_version_handler);
    if(result != EXIT_OK)
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
				   "Assign handler get pmc onu version failed\n" );
        return result;
    }
#endif
    

	global_onu_request_event_data.olt_id = PON_OLT_ID_NOT_AVAILABLE;
	/* ONU request event semaphore */
    SEMAPHORE_INIT_AND_LOG_FOR_ERROR( CTC_STACK_init, &global_onu_request_event_data.semaphore,
                                      global_onu_request_event_data_semaphore, result )


	/* Encryption thread semaphore */
    SEMAPHORE_INIT_AND_LOG_FOR_ERROR( CTC_STACK_init, &encryption_thread_semaphore,
                                      encryption_thread_semaphore, result )




    /* init 
        1. encryption semaphore per OLT
		2. Update ONU Firmware semaphore per OLT
        3. extended OAM discovery state to all ONU's
		4. set update ONU firmware db onu state to CTC_STACK_ONU_STATE_NOT_DURING_DOWNLOADING */
    for(olt_id = 0; olt_id < PON_OLT_ID_ARRAY_SIZE; olt_id++)
    {
        encryption_db.olt[olt_id].encryption_type = PON_ENCRYPTION_TYPE_AES;

        /* encryption semaphore per OLT */
        SEMAPHORE_INIT_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &encryption_db.olt[olt_id].semaphore, 
                                          ctc_stack_encryption_semaphore, result )
		 
        if( result != EXIT_OK )
        {
        
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
				   "Error while trying to init ctc_stack_encryption_semaphore\n" );
            return result;
        }

		/* Update ONU firmware semaphore per OLT */
        SEMAPHORE_INIT_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &update_firmware_db[olt_id].semaphore, 
                                          ctc_stack_update_firmware_semaphore, result )
		 
        if( result != EXIT_OK )
        {
        

			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
				   "Error while trying to init ctc_stack_update_firmware_semaphore\n" );
		    
            return result;
        }
        
#ifdef CTC_AUTH_SUPPORT
        for(index = 0; index < PON_ONU_ID_PER_OLT_ARRAY_SIZE; index++)
        {
            strcpy((authentication_db.olt[olt_id].loid[index].onu_id), CTC_AUTH_LOID_DATA_NOT_USED);
            strcpy((authentication_db.olt[olt_id].loid[index].password), CTC_AUTH_LOID_DATA_NOT_USED);
            authentication_db.olt[olt_id].auth_mode = CTC_AUTH_MODE_HYBRID;
            authentication_db.olt[olt_id].onu_llid[index] = PON_NOT_USED_LLID;
        }
#endif

        /* init extended OAM discovery state to all ONU's */
        for(onu_id = PON_MIN_ONU_ID_PER_OLT; onu_id < PON_MAX_ONU_ID_PER_OLT; onu_id++)
        {
            Reset_onu_entry(olt_id, onu_id);
			update_firmware_db[olt_id].onu_state[onu_id] = CTC_STACK_ONU_STATE_NOT_DURING_DOWNLOADING;
        }
    }


    /* Assign handler upon registration & deregistration events */
    result = Pon_event_assign_handler_function(PON_EVT_HANDLER_LLID_END_STD_OAM_DISCOVERY, (void*)ctc_pon_event_handler, &registration_handler_id);
    if( result != CTC_STACK_EXIT_OK )
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error in assign handler for CTC stack\n" );
		    	
        return result;
    }


    result = Pon_event_assign_handler_function(PON_EVT_HANDLER_LLID_DEREGISTER_EVENT,(void*)ctc_pon_event_handler, &deregistration_handler_id);
    if( result != CTC_STACK_EXIT_OK )
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error in assign handler for CTC stack\n" );

        return result;
    }


#if 0
    /* Assign authorization handler */
    result = Pon_Event_assign_handler_function(ONU_PON_HANDLER_ONU_AUTHORIZATION,(void*)*Onu_authorization_handler, &onu_authorization_handler_id);
    if( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
            "Error in assign authorization handler for CTC stack\n" );
        
        return result;
    }
#endif

     /* set encryption mode */
    internal_ctc_stack_automatic_mode = automatic_mode;

	internal_ctc_stack_automatic_onu_configuration_mode = FALSE;
   
    
    if( internal_ctc_stack_automatic_mode )
    {
    
        /* Assign handler upon new OLT added & removed events */
        result = Pon_event_assign_handler_function(PON_EVT_HANDLER_OLT_ADD,(void*)ctc_pon_event_handler, &add_olt_handler_id);
        if( result != CTC_STACK_EXIT_OK )
        {
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"Error in assign handler for CTC stack\n" );   
            return result;
        }

        result = Pon_event_assign_handler_function(PON_EVT_HANDLER_OLT_RMV,(void*)ctc_pon_event_handler, &remove_olt_handler_id);
        if( result != CTC_STACK_EXIT_OK )
        {
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"Error in assign handler for CTC stack\n" );   
   
            return result;
        }
    }
    
   
    result = Communication_ctc_init(Onu_request_handler_function);
    if( result != CTC_STACK_EXIT_OK )
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error in opening communication layer in CTC stack\n" );   
	
        return CTC_STACK_EXIT_ERROR;
    }
         
    first_update_key_iteration = TRUE;

    /* set default extended OAM discovery information: records list(OUI, version) & timeout */
    ctc_stack_general_information.version = CTC_VERSION_DEFINE;

    discovery_db.list_info.number_of_oui_records = number_of_records;
    
    if(number_of_records != 0)
    {
        for(i = 0; i < number_of_records; i++)
        {
            for(oui_index = 0; oui_index < OAM_OUI_SIZE; oui_index++)
            {
                discovery_db.list_info.records_list[i].oui[oui_index] = oui_version_records_list[i].oui[oui_index];
            }
            discovery_db.list_info.records_list[i].version = oui_version_records_list[i].version;
        }
    }
    else/* default values in case there is no list */
    {
        discovery_db.list_info.number_of_oui_records = 1;
        
        Copy_oui_as_number(Get_pon_ctc_oui(), discovery_db.list_info.records_list[0].oui);
        discovery_db.list_info.records_list[0].version = CTC_VERSION_DEFINE ;

    }


    /* set default values to ctc stack timing parameters */
    discovery_db.timeout_in_100_ms                              = CTC_STACK_DEFAULT_DISCOVERY_TIMEOUT;
    encryption_db.timing_info.update_key_time_in_sec            = CTC_STACK_DEFAULT_ENCRYPTION_UPDATE_KEY_TIME;
    encryption_db.timing_info.no_reply_timeout_in_100_ms        = CTC_STACK_DEFAULT_ENCRYPTION_TIMEOUT;
    encryption_db.timing_info.start_encryption_threshold_in_sec = START_ENCRYPTION_DEFAULT_THRESHOLD_TIME_DEFINE;
   


    /* init synchronization semaphore between encryption scheduler task and terminate API */
    SEMAPHORE_INIT_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &encryption_scheduler_thread_semaphore, 
                                      ctc_stack_encryption_scheduler_thread_semaphore, result )
		 
    if( result != EXIT_OK )
    {
    
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error while trying to init a ctc_stack_encryption_scheduler_thread_semaphore\n" );     
        return CTC_STACK_COMMUNICATION_EXIT_ERROR;
    }


    /* take the synchronization semaphore between encryption scheduler task and terminate API */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_scheduler_thread_semaphore), 
                                          ctc_stack_encryption_scheduler_thread_semaphore,result );
    if (result != EXIT_OK) 
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"failed to take a ctc_stack_encryption_scheduler_thread_semaphore\n" );     
        return (CTC_STACK_EXIT_ERROR);
    }


	SEMAPHORE_INIT_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &encryption_timing_parameters_semaphore, 
                                      ctc_stack_encryption_timing_parameters_semaphore, result )
		 
    if( result != EXIT_OK )
    {
    
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error while trying to init a ctc_stack_encryption_timing_parameters_semaphore\n" );     
        return CTC_STACK_COMMUNICATION_EXIT_ERROR;
    }

    encryption_scheduler_thread_active  = TRUE;
    encryption_process_thread_active    = TRUE;

    ctc_stack_general_information.ctc_stack_is_init          = TRUE;

#if 1
    OSSRV_create_thread ((void *)Encryption_scheduler_thread_hook_func);
    OSSRV_create_thread ((void *)Encryption_process_thread_hook_func);
#endif

#if 0
    system_parameters.automatic_authorization_policy = DISABLE;
    system_parameters.host_olt_msgs_timeout     = -1;
    system_parameters.monitoring_cycle          = -1;
    system_parameters.olt_reset_timeout         = -1;
    system_parameters.statistics_sampling_cycle = -1;

    result = Convert_ponsoft_error_code_to_ctc_stack(PAS_set_system_parameters ( &system_parameters ));
    if(result != CTC_STACK_EXIT_OK)
    {
		PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id,
						"OLT:%d ONU :%d failed (return code %d)to set automatic authorization policy \n", olt_id, onu_id, result);

		return result;
    }

    /*Initial redundancy variables*/
    result = init_rdn_data_variables();
    if(result != CTC_STACK_EXIT_OK)
    {
		PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Failed (return code %d)to initial CTC redundancy data block temp variables \n", result);

		return result;
    }
#endif
    
    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME




#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_parameters"

PON_STATUS PON_CTC_STACK_set_parameters 
                ( const bool							 automatic_mode,
                  const unsigned char					 number_of_records,
                  const CTC_STACK_oui_version_record_t  *oui_version_records_list,
				  const bool							 automatic_onu_configuration)
{
    unsigned char               i,oui_index;
	short int					result;
	bool						old_ctc_stack_automatic_mode;

	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);


	CTC_STACK_CHECK_INIT_FUNC

	if(number_of_records > MAX_OUI_RECORDS)
	{
		PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"maximum records list is %d, error to configure %d records list\n", MAX_OUI_RECORDS, number_of_records );   

        return (CTC_STACK_PARAMETER_ERROR);
	}

	if(automatic_mode == FALSE)
	{
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Do not support non-automatic mode\n" );   

        return (CTC_STACK_PARAMETER_ERROR);
	}


	/* old encryption mode */
    old_ctc_stack_automatic_mode = internal_ctc_stack_automatic_mode;

    /* set encryption mode */
    internal_ctc_stack_automatic_mode = automatic_mode;
	internal_ctc_stack_automatic_onu_configuration_mode = automatic_onu_configuration;



	/* change mode: non automatic --> automatic */
	if( (!old_ctc_stack_automatic_mode)  && internal_ctc_stack_automatic_mode )
    {
		/* Assign handler upon new OLT added & removed events */
        result = Pon_event_assign_handler_function(PON_EVT_HANDLER_OLT_ADD,(void*)ctc_pon_event_handler, &add_olt_handler_id);
        if( result != CTC_STACK_EXIT_OK )
        {
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"Error in assign handler for CTC stack\n" );   
            return result;
        }

        result = Pon_event_assign_handler_function(PON_EVT_HANDLER_OLT_RMV,(void*)ctc_pon_event_handler, &remove_olt_handler_id);
        if( result != CTC_STACK_EXIT_OK )
        {
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"Error in assign handler for CTC stack\n" );   
   
            return result;
        }

    }
	else if( (old_ctc_stack_automatic_mode)  && (!internal_ctc_stack_automatic_mode) )
	{/* change mode: automatic --> non automatic */
		result = Pon_event_delete_handler_function(add_olt_handler_id);
        if( result != CTC_STACK_EXIT_OK )
        {
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"Error in terminate olt_added handler in CTC stack\n" );   

            return result;
        }

        result = Pon_event_delete_handler_function(remove_olt_handler_id);
        if( result != CTC_STACK_EXIT_OK )
        {
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"Error in terminate olt_removed handler in CTC stack\n" );   

            return result;
        }
	}
	

    discovery_db.list_info.number_of_oui_records = number_of_records;
    
    if(number_of_records != 0)
    {
        for(i = 0; i < number_of_records; i++)
        {
            for(oui_index = 0; oui_index < OAM_OUI_SIZE; oui_index++)
            {
                discovery_db.list_info.records_list[i].oui[oui_index] = oui_version_records_list[i].oui[oui_index];
            }
            discovery_db.list_info.records_list[i].version = oui_version_records_list[i].version;
        }
    }
 
    
    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_parameters"

PON_STATUS PON_CTC_STACK_get_parameters 
                ( bool							  *automatic_mode,
                  unsigned char					  *number_of_records,
                  CTC_STACK_oui_version_record_t  *oui_version_records_list,
				  bool							  *automatic_onu_configuration)
{
    unsigned char               i,oui_index;

	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

	CTC_STACK_CHECK_INIT_FUNC

    (*automatic_mode) = internal_ctc_stack_automatic_mode;
	(*automatic_onu_configuration) = internal_ctc_stack_automatic_onu_configuration_mode;
	
   

    (*number_of_records) = discovery_db.list_info.number_of_oui_records;
    
    if(*number_of_records != 0)
    {
        for(i = 0; i < (*number_of_records); i++)
        {
            for(oui_index = 0; oui_index < OAM_OUI_SIZE; oui_index++)
            {
                oui_version_records_list[i].oui[oui_index] = discovery_db.list_info.records_list[i].oui[oui_index];
            }
            oui_version_records_list[i].version = discovery_db.list_info.records_list[i].version;
        }
    }
 
    
    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME




#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_terminate"

PON_STATUS PON_CTC_STACK_terminate ( void )
{
    short int           result;
    PON_olt_id_t        olt_id;
    PON_llid_t          onu_id;

	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    CTC_STACK_CHECK_INIT_FUNC

    encryption_scheduler_thread_active  = FALSE;
    encryption_process_thread_active    = FALSE;

    /* take synchronization semaphore between encryption scheduler task and terminate API */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_scheduler_thread_semaphore), 
                                          ctc_stack_encryption_scheduler_thread_semaphore,result );
    if (result != EXIT_OK) 
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"failed to take a ctc_stack_encryption_scheduler_thread_semaphore\n" );   

        return (CTC_STACK_EXIT_ERROR);
    }
   

    /* close synchronization semaphore between encryption scheduler task and terminate API */
    SEMAPHORE_CLOSE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &encryption_scheduler_thread_semaphore, 
                                      ctc_stack_encryption_scheduler_thread_semaphore, result )
		 
    if( result != EXIT_OK )
    {
    
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error while trying to close a ctc_stack_scheduler_thread_semaphore\n" );     

        return CTC_STACK_COMMUNICATION_EXIT_ERROR;
    }


	/* close encryption timing semaphore */
    SEMAPHORE_CLOSE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &encryption_timing_parameters_semaphore, 
                                      ctc_stack_encryption_timing_parameters_semaphore, result )
		 
    if( result != EXIT_OK )
    {
    
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error while trying to close a ctc_stack_encryption_timing_parameters_semaphore\n" );     

        return CTC_STACK_COMMUNICATION_EXIT_ERROR;
    }


    result = Communication_ctc_terminate();
    if( result != CTC_STACK_EXIT_OK )
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error close communication layer in CTC stack\n" );     
        return result;
    }


    /* handlers from registration & deregistration events*/
    result = Convert_ponsoft_error_code_to_ctc_stack(Pon_event_delete_handler_function(registration_handler_id));
    if( result != CTC_STACK_EXIT_OK )
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error in terminate registration handler in CTC stack\n" );   

        return result;
    }

    result = Convert_ponsoft_error_code_to_ctc_stack(Pon_event_delete_handler_function(deregistration_handler_id));
    if( result != CTC_STACK_EXIT_OK )
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error in terminate deregistration handler in CTC stack\n" );   

        return result;
    }


#if 0
    /* Delete authorization handler */
    result = Convert_ponsoft_error_code_to_ctc_stack(Pon_event_delete_handler_function(onu_authorization_handler_id));
    if( result != CTC_STACK_EXIT_OK )
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error in terminate authorization handler in CTC stack\n" );   

        return result;
    } 
#endif    

    if( internal_ctc_stack_automatic_mode )
    {
        result = Convert_ponsoft_error_code_to_ctc_stack(Pon_event_delete_handler_function(add_olt_handler_id));
        if( result != CTC_STACK_EXIT_OK )
        {
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"Error in terminate olt_added handler in CTC stack\n" );   

            return result;
        }

        result = Convert_ponsoft_error_code_to_ctc_stack(Pon_event_delete_handler_function(remove_olt_handler_id));
        if( result != CTC_STACK_EXIT_OK )
        {
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"Error in terminate olt_removed handler in CTC stack\n" );   

            return result;
        }
    }


	
	global_onu_request_event_data.olt_id = PON_OLT_ID_NOT_AVAILABLE;

	/* ONU request event semaphore */
    SEMAPHORE_CLOSE_AND_LOG_FOR_ERROR( CTC_STACK_terminate, &global_onu_request_event_data.semaphore,
                                      global_onu_request_event_data_semaphore, result )


	/* Encryption thread semaphore */
    SEMAPHORE_CLOSE_AND_LOG_FOR_ERROR( CTC_STACK_terminate, &encryption_thread_semaphore,
                                      encryption_thread_semaphore, result )


    /* set 
        1. close OLT encryption db semaphore
		2. close OLT update firmware db semaphore
        3. extended OAM discovery state IDLE to all ONU's
		4. update ONU Firmware state is set to NOT_DURING_DOWNLOADING */
    for(olt_id = 0; olt_id < PON_OLT_ID_ARRAY_SIZE; olt_id++)
    {

        /* release each OLT semaphore */
        SEMAPHORE_CLOSE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &encryption_db.olt[olt_id].semaphore, 
                                           ctc_stack_encryption_semaphore, result )
		if( result != EXIT_OK )
        {
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Error in closing a ctc_stack_encryption_semaphore\n" );   
	    
            return CTC_STACK_EXIT_ERROR;
        }


		/* release each OLT semaphore */
        SEMAPHORE_CLOSE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &update_firmware_db[olt_id].semaphore, 
                                           ctc_stack_update_onu_semaphore, result )
		if( result != EXIT_OK )
        {
        
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"Error in closing a ctc_stack_update_onu_semaphore\n" );  

            return CTC_STACK_EXIT_ERROR;
        }

		

        for(onu_id = PON_MIN_ONU_ID_PER_OLT; onu_id < PON_MAX_ONU_ID_PER_OLT; onu_id++)
        {
            discovery_db.state[olt_id][onu_id] = CTC_DISCOVERY_STATE_IDLE;      
			update_firmware_db[olt_id].onu_state[onu_id] = CTC_STACK_ONU_STATE_NOT_DURING_DOWNLOADING;
        }
    }


#if 0
    result = PON_OAM_MODULE_terminate();
    if(result != EXIT_OK)
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"OAM_MODULE_terminate failed\n" );  
    }
    
    internal_ctc_stack_automatic_mode                = FALSE;
	internal_ctc_stack_automatic_onu_configuration_mode = FALSE;
    ctc_stack_general_information.ctc_stack_is_init    = FALSE;
#endif

	/* Clean all functions handlers structure */
	PON_general_terminate_handler_function(&ctc_db_handlers);

#if 0
    result = PAS_set_handler_get_pmc_onu_version(NULL);
    if(result != EXIT_OK)
    {
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
				   "Assign handler get pmc onu version failed\n" );
        return result;
    }
#endif

    return CTC_STACK_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_assign_handler_function"


PON_STATUS PON_CTC_STACK_assign_handler_function
                ( const CTC_STACK_handler_index_t    handler_function_index, 
				  const void					    (*handler_function)(),
				  unsigned short                     *handler_id )
{
	short int result;
PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

	CTC_STACK_CHECK_INIT_FUNC
	
    result= PON_general_assign_handler_function( &ctc_db_handlers, 
						(unsigned short int)handler_function_index, 
						(general_handler_function_t)handler_function,
						handler_id);

	if (result != EXIT_OK)
		return CTC_STACK_EXIT_ERROR;  

    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_delete_handler_function"

PON_STATUS PON_CTC_STACK_delete_handler_function ( const unsigned short  handler_id )
{
	short int result;

	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

	CTC_STACK_CHECK_INIT_FUNC
	 
	result = PON_general_delete_handler_function ( &ctc_db_handlers, handler_id );

	if (result != EXIT_OK)
		return CTC_STACK_EXIT_ERROR;  

    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_version"

PON_STATUS PON_CTC_STACK_set_version 
                ( const unsigned char  version )
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);
   
    CTC_STACK_CHECK_INIT_FUNC

    ctc_stack_general_information.version = version;
       
    
    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_version"

PON_STATUS PON_CTC_STACK_get_version 
                ( unsigned char  *version )
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    CTC_STACK_CHECK_INIT_FUNC

    (*version) = ctc_stack_general_information.version;
       
    
    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_oui"

PON_STATUS PON_CTC_STACK_set_oui 
                ( const unsigned long  oui )
{
    unsigned long   fw_oui = (oui<<8); /* due to the fact that we send 32 bits and FW needs 24 bits we shift 8 bits */

	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

   
    CTC_STACK_CHECK_INIT_FUNC

    ctc_stack_general_information.ctc_stack_oui = oui;

    /* Set CTC OUI */
    /* Set_system_oui (fw_oui); */


    Communication_set_ctc_oui(oui);
       
    
    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_oui"

PON_STATUS PON_CTC_STACK_get_oui 
                ( unsigned long  *oui )
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    CTC_STACK_CHECK_INIT_FUNC

    (*oui) = ctc_stack_general_information.ctc_stack_oui;
       
    
    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_extended_oam_discovery_timing"

PON_STATUS PON_CTC_STACK_set_extended_oam_discovery_timing 
                ( unsigned short int  discovery_timeout )
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);
   
    CTC_STACK_CHECK_INIT_FUNC
        

	if(discovery_timeout > CTC_STACK_MAX_DISCOVERY_TIMEOUT)
		return CTC_STACK_PARAMETER_ERROR;

    discovery_db.timeout_in_100_ms = discovery_timeout;

    
    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_extended_oam_discovery_timing"

PON_STATUS PON_CTC_STACK_get_extended_oam_discovery_timing 
                ( unsigned short int  *discovery_timeout )
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    CTC_STACK_CHECK_INIT_FUNC

    (*discovery_timeout) = discovery_db.timeout_in_100_ms;
  
    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_encryption_timing_threshold"

PON_STATUS PON_CTC_STACK_set_encryption_timing_threshold 
                ( const unsigned char   start_encryption_threshold)
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    /* Input checking */
    CTC_STACK_CHECK_INIT_FUNC
    /* End input checking */
        
    encryption_db.timing_info.start_encryption_threshold_in_sec     = start_encryption_threshold;
    

    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_encryption_timing_threshold"

PON_STATUS PON_CTC_STACK_get_encryption_timing_threshold 
                ( unsigned char   *start_encryption_threshold)
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    CTC_STACK_CHECK_INIT_FUNC

    (*start_encryption_threshold)   = encryption_db.timing_info.start_encryption_threshold_in_sec;

    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_encryption_timing"

PON_STATUS PON_CTC_STACK_set_encryption_timing 
                ( const unsigned char       update_key_time,
                  const unsigned short int  no_reply_timeout )
{
	unsigned short      result;
   
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    /* Input checking */
    CTC_STACK_CHECK_INIT_FUNC

	if( no_reply_timeout > CTC_STACK_MAX_ENCRYPTION_TIMEOUT )
	{
		PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"can not configured encryption timeout to %u , max value is :%u\n", no_reply_timeout, CTC_STACK_MAX_ENCRYPTION_TIMEOUT );



		return CTC_STACK_PARAMETER_ERROR;
	}


    if(no_reply_timeout/*measured in 100 ms*/ > update_key_time*10/*convert to 100 ms*/)
    {
		PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"can not configured encryption timeout %u(in 100 msec) bigger than encryption cycle %u (in 100 msec)\n", no_reply_timeout, update_key_time*10 );


        return CTC_STACK_PARAMETER_ERROR;
    }

    if( no_reply_timeout == 0 )
    {
        PONLOG_ERROR_1( PONLOG_FUNC_ENCRYPTION_FLOW, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "can not configured timeout to %u\n", no_reply_timeout);

        return CTC_STACK_PARAMETER_ERROR;
    }
    /* End input checking */
        

	/* take the encryption timing semaphore to protect against real time parameters setting */
	SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_timing_parameters_semaphore), 
										  ctc_stack_encryption_timing_parameters_semaphore,result );
	if (result != EXIT_OK) 
	{
		PONLOG_ERROR_0( PONLOG_FUNC_ENCRYPTION_FLOW, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"failed to take a ctc_stack_encryption_timing_parameters_semaphore\n" );     
		return (CTC_STACK_EXIT_ERROR);
	}


    encryption_db.timing_info.update_key_time_in_sec                = update_key_time;
    encryption_db.timing_info.no_reply_timeout_in_100_ms            = no_reply_timeout;
    

	/* release encryption timing semaphore */
	RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_timing_parameters_semaphore),
										 ctc_stack_encryption_encryption_timing_parameters_semaphore,result );
	if (result != EXIT_OK) 
	{
		PONLOG_ERROR_0(PONLOG_FUNC_ENCRYPTION_FLOW, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,"failed to release a ctc_stack_encryption_timing_parameters_semaphore\n" );
	}


    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_encryption_timing"

PON_STATUS PON_CTC_STACK_get_encryption_timing 
                ( unsigned char       *update_key_time,
                  unsigned short int  *no_reply_timeout )
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    CTC_STACK_CHECK_INIT_FUNC

    (*no_reply_timeout)             = encryption_db.timing_info.no_reply_timeout_in_100_ms;
    (*update_key_time)              = encryption_db.timing_info.update_key_time_in_sec;

    return CTC_STACK_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_encryption_configuration"

static short int PON_CTC_STACK_set_encryption_configuration 
                ( const PON_olt_id_t           olt_id,
                  const PON_encryption_type_t  encryption_type )
{
    short int       result;
   

    CTC_STACK_CHECK_INIT_FUNC


    result = Convert_ponsoft_error_code_to_ctc_stack(PAS_set_encryption_configuration(olt_id, encryption_type));
    if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_2(PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, PONLOG_ONU_IRRELEVANT, "OLT %d failed to set encryption configuration. error code:%d\n", olt_id, result)

        return result;
    }
    

    encryption_db.olt[olt_id].encryption_type = encryption_type;

    return result;  
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_encryption_configuration"

PON_STATUS PON_CTC_STACK_get_encryption_configuration 
                ( const PON_olt_id_t      olt_id,
                  PON_encryption_type_t  *encryption_type )
{
   
    short int       result;
   
	PONLOG_INFO_API_CALLED_0(olt_id, PONLOG_ONU_IRRELEVANT);

    CTC_STACK_CHECK_INIT_FUNC

        
    result = Convert_ponsoft_error_code_to_ctc_stack(PAS_get_encryption_configuration(olt_id, encryption_type));
    if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_2(PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, PONLOG_ONU_IRRELEVANT,"OLT %d failed to set encryption configuration. error code:%d\n", olt_id, result)
    }
    

    return result;  
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_start_encryption"

PON_STATUS PON_CTC_STACK_start_encryption ( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_llid_t     llid )
{

    short int                               result=CTC_STACK_EXIT_OK;
    bool                                    need_to_send_according_update_key_time,need_to_send_according_timeout_time;
    CTC_STACK_encryption_state_t            encryption_state;
	PON_onu_id_t							onu_id = llid;

	bool									exist_assign_handler = FALSE;
	short int								client;
	general_handler_function_t				the_handler;
   
	PONLOG_INFO_API_CALLED_0(olt_id, llid);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    

    encryption_db.olt[olt_id].onu_id[onu_id].start_encryption_by_the_user = TRUE;

	for (client = 0; client < PON_MAX_CLIENT; client++)
		{
			PON_general_get_handler_function
							   ( &ctc_db_handlers,
								 CTC_STACK_HANDLER_UPDATE_ENCRYPTION_KEY,
								 client ,
								 &the_handler );

			if (the_handler != NULL)
			{
				exist_assign_handler = TRUE;
			}
		}

    /* handler function is assigned. handler will be called after the start encryption process has completed */
    if (exist_assign_handler == TRUE)
    {
        return CTC_STACK_EXIT_OK;
    }
    else/* handler function isn't assigned. return after the start encryption process has finished  */
    {
        PON_DECLARE_TIMEOUT_VARIABLES

        Set_encryption_db_entry  ( olt_id, onu_id, CTC_ENCRYPTION_STATE_WAITING_FOR_ENCRYPTION);

        Check_threshold_time_according_update_key_time(&need_to_send_according_update_key_time);
        Check_threshold_time_according_timeout_time(&need_to_send_according_timeout_time);
  
        if (need_to_send_according_update_key_time && need_to_send_according_timeout_time)
        {
            result = Send_new_key_request 
                        ( olt_id, onu_id,
                          encryption_db.olt[olt_id].onu_id[onu_id].key_index );
            CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

            PON_TIMEOUT_INIT(encryption_db.timing_info.no_reply_timeout_in_100_ms/10/*convert to seconds*/);
     
            /* wait for response - busy waiting */
            encryption_state = encryption_db.olt[olt_id].onu_id[onu_id].encryption_state;
            
            while ( (encryption_state == CTC_ENCRYPTION_STATE_WAITING_FOR_ENCRYPTION) && (!PON_TIMEOUT_EXPIRED) ) 
            {
		        PON_TIMEOUT_ITERATION;
                encryption_state = encryption_db.olt[olt_id].onu_id[onu_id].encryption_state;
	        }
        }
        else
        {
            PON_TIMEOUT_INIT(encryption_db.timing_info.update_key_time_in_sec);
     
            /* set encryption flag to WAITING_FOR_RESPONSE so in the next timeout simulation encryption request will be sent to this ONU */
            Set_encryption_db_flag_entry(olt_id, onu_id, CTC_STACK_STATE_WAITING_FOR_RESPONSE); 

            /* wait for response - busy waiting */
            encryption_state = encryption_db.olt[olt_id].onu_id[onu_id].encryption_state;
            
            while ( (encryption_state == CTC_ENCRYPTION_STATE_WAITING_FOR_ENCRYPTION) && (!PON_TIMEOUT_EXPIRED) ) 
            {
		        PON_TIMEOUT_ITERATION;
                encryption_state = encryption_db.olt[olt_id].onu_id[onu_id].encryption_state;
	        }
        }
           

        switch(encryption_state)
        {
        case CTC_ENCRYPTION_STATE_WAITING_FOR_ENCRYPTION:
            PONLOG_ERROR_2(PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT %d ONU %d is not responding\n", olt_id, onu_id);
            result = CTC_STACK_QUERY_FAILED;
            break;
        case CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED:
        case CTC_ENCRYPTION_STATE_NOT_ENCRYPTED:
        
            result = CTC_STACK_QUERY_FAILED;
            break;
        case CTC_ENCRYPTION_STATE_ENCRYPTION_COMPLETE:
            result = CTC_STACK_EXIT_OK;
            break;
        default:
            PONLOG_ERROR_3(PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id,"OLT %d ONU %d unsupported encryption state:%d\n", olt_id, onu_id, encryption_state);
            result = CTC_STACK_EXIT_ERROR;
        }
    }          
    return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_stop_encryption"

PON_STATUS PON_CTC_STACK_stop_encryption ( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_llid_t     llid )
{

    short int                               result=CTC_STACK_EXIT_OK;
	PON_onu_id_t							onu_id = llid;
#if 0    
	PON_llid_parameters_t					llid_parameters;
#else
    bool                                    llid_encryption_mode;
#endif
    general_handler_function_t              the_handler;
	short int					            client;

	PONLOG_INFO_API_CALLED_0(olt_id, llid);
    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(encryption_db.olt[olt_id].onu_id[onu_id].start_encryption_by_the_user != TRUE)
	{
		  PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d call first start encryption\n",
                        olt_id, onu_id );

		  return CTC_STACK_EXIT_ERROR;
	}

    /* Take semaphore so it not collide with encryption task */
	SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC( PONLOG_FUNC_ENCRYPTION_FLOW, 
											 &encryption_thread_semaphore, 
											 encryption_thread_semaphore, result)
    if (result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);

    /* update encryption parameters */
    Set_encryption_db_flag_entry(olt_id, onu_id, CTC_STACK_STATE_VALID);

    encryption_db.olt[olt_id].onu_id[onu_id].key_index = 1;
	encryption_db.olt[olt_id].onu_id[onu_id].temp_key_index = 1;
    encryption_db.olt[olt_id].onu_id[onu_id].start_encryption_by_the_user = FALSE;
    encryption_db.olt[olt_id].onu_id[onu_id].number_of_sending_request = 0;

    Set_encryption_db_entry(olt_id, onu_id, CTC_ENCRYPTION_STATE_NOT_ENCRYPTED);

    /* Release semaphore */
	RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC( PONLOG_FUNC_ENCRYPTION_FLOW, 
											 &encryption_thread_semaphore, 
											 encryption_thread_semaphore, result)
    if (result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);


    /* The task is not working anymore over the current ONU, so we can stop it */

    
	result = OLT_GetLLIDEncryptMode(olt_id, llid, &llid_encryption_mode);
	if(result != PAS_EXIT_OK)
    {
        PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d failed in  PAS_get_llid_parameters \n",
                        olt_id, onu_id );

		return result;
    }

#if 0
	if(llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION)
#else
	if(!llid_encryption_mode)
#endif
	{
		/* if the encryption process hasn't start yet */
		return CTC_STACK_EXIT_OK;
	}


	result = OLT_StopLLIDEncrypt(olt_id, onu_id);

    if(result != PAS_EXIT_OK)
    {
        PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d failed in  PAS_stop_olt_encryption \n",
                           olt_id, onu_id );
    }

	for (client = 0; client < PON_MAX_CLIENT; client++)
	{
		PON_general_get_handler_function
						   ( &ctc_db_handlers,
							 CTC_STACK_HANDLER_UPDATE_ENCRYPTION_KEY,
							 client ,
							 &the_handler );

		if (the_handler != NULL)
		{
			the_handler (	olt_id, onu_id,
							CTC_ENCRYPTION_STATE_NOT_ENCRYPTED);
		}
	}


    return result;
}
#undef PONLOG_FUNCTION_NAME




#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_dba_report_thresholds"

PON_STATUS PON_CTC_STACK_get_dba_report_thresholds ( 
                                   const PON_olt_id_t					  olt_id, 
                                   const PON_onu_id_t					  onu_id,
                                   unsigned char 						 *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
    short int                result,comparser_result;
    unsigned short           received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char            sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char            received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char            expected_opcode =  OAM_VENDOR_EXT_DBA;
    unsigned char            expected_code   =  DBA_VENDOR_EXT_GET_RESPONSE;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    
    comparser_result = (short int)Compose_get_dba_thresholds
                            ( sent_oam_pdu_data+OAM_OUI_SIZE, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


    /* reset return parameters */
    (*number_of_queue_sets) = 0; 
    memset(queues_sets_thresholds, 0, sizeof(CTC_STACK_onu_queue_set_thresholds_t));

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, sent_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

    comparser_result = (short int)Parse_get_dba_thresholds
                                    ( RECEIVED_COMPARSER_BUFFER_OLD_COMMANDS, received_length,
         	                          number_of_queue_sets, queues_sets_thresholds  );

    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
        
    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_dba_report_thresholds"

PON_STATUS PON_CTC_STACK_set_dba_report_thresholds ( 
                                   const PON_olt_id_t					  olt_id, 
                                   const PON_onu_id_t					  onu_id,
                                   unsigned char 						 *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
    short int                result,comparser_result;
    bool                     onu_ack;
    unsigned short           received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char            sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char            received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char            expected_opcode =  OAM_VENDOR_EXT_DBA;
    unsigned char            expected_code   =  DBA_VENDOR_EXT_SET_RESPONSE;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    /* Input checking */
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC


	PON_HANDLE_ERROR_CHECK_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, (*number_of_queue_sets),number_of_queue_sets,2,4) 
       

    comparser_result = (short int)Compose_set_dba_thresholds
                            ( *number_of_queue_sets, queues_sets_thresholds,
                              sent_oam_pdu_data+OAM_OUI_SIZE, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


    /* reset return parameters */
    (*number_of_queue_sets) = 0; 
    memset(queues_sets_thresholds, 0, sizeof(CTC_STACK_onu_queue_set_thresholds_t));

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, sent_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

    comparser_result = (short int)Parse_set_dba_thresholds
                                    ( RECEIVED_COMPARSER_BUFFER_OLD_COMMANDS, received_length,
         	                          &onu_ack, number_of_queue_sets, queues_sets_thresholds  );

    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
        
    if(onu_ack == TRUE)
    {
        return CTC_STACK_EXIT_OK;
    }
    else
    {
        return CTC_STACK_EXIT_ERROR_ONU_DBA_THRESHOLDS;
    }
    

}

#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_update_onu_firmware"

PON_STATUS PON_CTC_STACK_update_onu_firmware ( 
                 const PON_olt_id_t          olt_id, 
                 const PON_onu_id_t          onu_id,
                 const CTC_STACK_binary_t   *onu_firmware,
                 const char		            *file_name)
{
    short int                result=CTC_STACK_EXIT_OK, send_result=CTC_STACK_EXIT_OK;
    unsigned short           sent_length=ETHERNET_MTU;
    unsigned char            sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char            received_oam_pdu_data[ETHERNET_MTU]= {0};
    PON_oam_frame_rate_t     saved_frame_rate;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    /* input checking */
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
	
	if( onu_firmware->location == NULL )
	{
		PONLOG_ERROR_2( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
						"OLT %d ONU %d file name is NULL pointer\n", olt_id, onu_id );

		return CTC_STACK_PARAMETER_ERROR;
	}
    
#if 0
    /*save current frame rate*/
    result = Convert_ponsoft_error_code_to_ctc_stack(
					PAS_get_oam_frame_rate(olt_id, onu_id, &saved_frame_rate));
	if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id,"OLT: %d ONU: %d failed to set oam frame rate. error code:%d\n", olt_id, onu_id, result)
        return result;
    }

    if(saved_frame_rate != PON_OAM_FRAME_RATE_100_FRAMES)
    {
        /*set frame rate to 100 frames per second*/
        result = Convert_ponsoft_error_code_to_ctc_stack(
                         PAS_set_oam_frame_rate(olt_id, onu_id, PON_OAM_FRAME_RATE_100_FRAMES));
        if ( result != CTC_STACK_EXIT_OK )
        {
            PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id,"OLT: %d ONU: %d failed to set oam frame rate. error code:%d\n", olt_id, onu_id, result)
            return result;
        }
    }
#endif

	if( Set_onu_state_in_update_firmware_db(olt_id, onu_id, CTC_STACK_ONU_STATE_DURING_DOWNLOADING) != CTC_STACK_EXIT_OK)
	{
		PONLOG_ERROR_2( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
						"OLT %d ONU %d is during download already\n", olt_id, onu_id );

		return CTC_STACK_ONU_NOT_AVAILABLE;
	}
       
  
	send_result = Send_file_and_verify(olt_id, onu_id, onu_firmware, TRUE, file_name);

#if 0
    if(saved_frame_rate != PON_OAM_FRAME_RATE_100_FRAMES)
    {
        /*set back frame rate*/
        result = Convert_ponsoft_error_code_to_ctc_stack(
            PAS_set_oam_frame_rate(olt_id, onu_id, saved_frame_rate));
        if ( result != CTC_STACK_EXIT_OK )
        {
            PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id,"OLT: %d ONU: %d failed to set oam frame rate. error code:%d\n", olt_id, onu_id, result)
                return result;
        }
    }
#endif

    return send_result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "PON_CTC_STACK_is_init"
PON_STATUS PON_CTC_STACK_is_init ( bool *init )
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    (*init) = ctc_stack_general_information.ctc_stack_is_init;

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_reponse_operation_mode"

PON_STATUS PON_CTC_STACK_set_reponse_operation_mode ( bool set_response_contains_data )
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

	CTC_STACK_CHECK_INIT_FUNC


    CTC_COMPOSER_global_set_response_contains_data = set_response_contains_data;

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_set_reponse_operation_mode"

PON_STATUS PON_CTC_STACK_get_set_reponse_operation_mode ( bool *set_response_contains_data )
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

	CTC_STACK_CHECK_INIT_FUNC


    (*set_response_contains_data) = CTC_COMPOSER_global_set_response_contains_data;

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_onu_serial_number"

PON_STATUS PON_CTC_STACK_get_onu_serial_number ( 
           const PON_olt_id_t			   olt_id, 
           const PON_onu_id_t			   onu_id,
		   CTC_STACK_onu_serial_number_t  *onu_serial_number )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_ONU_IDENTIFIER;
	unsigned char							ctc_ret_val;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							received_num_of_entries=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


	total_length += sent_length;
	sent_length = max_length - total_length;

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;
	/*b-12epon协商成3.0 onu_extenedmodel显示为乱码，added by zhaoxh*/
	if(discovery_db.common_version[olt_id][onu_id] > CTC_2_1_ONU_VERSION)
	{
			comparser_result = (short int)CTC_PARSE_onu_identifier_ctc_3_0
										( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
										  onu_serial_number, &ctc_ret_val, &received_left_length  );
	}
    else
    {
    	comparser_result = (short int)CTC_PARSE_onu_identifier
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          onu_serial_number, &ctc_ret_val, &received_left_length  );
    }
	/*e-12epon协商成3.0 onu_extenedmodel显示为乱码，added by zhaoxh*/

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;


	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_firmware_version_extended"
PON_STATUS PON_CTC_STACK_get_firmware_version_extended ( 
           const PON_olt_id_t	olt_id, 
           const PON_onu_id_t	onu_id,
           INT8U				*version_buffer,
           INT8U				*version_buffer_size)
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_FIRMWARE_VERSION;
	unsigned char							ctc_ret_val;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							received_num_of_entries=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


	total_length += sent_length;
	sent_length = max_length - total_length;

  	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)CTC_PARSE_firmware_version
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          version_buffer, version_buffer_size, &ctc_ret_val, &received_left_length  );

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;


	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_firmware_version"
PON_STATUS PON_CTC_STACK_get_firmware_version ( 
           const PON_olt_id_t	olt_id, 
           const PON_onu_id_t	onu_id,
           unsigned short      *version )
{
#define FW_VER_BUFF_SIZE 20
	INT8U buff_size = FW_VER_BUFF_SIZE;
	INT8U buff[FW_VER_BUFF_SIZE];
	INT8U* offset = buff;

	short int result = PON_CTC_STACK_get_firmware_version_extended(olt_id,onu_id, buff, &buff_size);
	if (result != CTC_STACK_EXIT_OK)
		return (result);
				
	EXTRACT_USHORT_ENDIANITY_FROM_UBUFFER_FUNC(PONLOG_FUNC_GENERAL, offset, (*version), BYTES_IN_WORD, buff);
	return (result);
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_chipset_id"
PON_STATUS PON_CTC_STACK_get_chipset_id ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id,
		   CTC_STACK_chipset_id_t  *chipset_id )
{
    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
	return (Get_chipset_id ( 
                             olt_id, 
                             onu_id,
		                     chipset_id ));

        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_onu_capabilities"

PON_STATUS PON_CTC_STACK_get_onu_capabilities ( 
           const PON_olt_id_t			  olt_id, 
           const PON_onu_id_t			  onu_id,
		   CTC_STACK_onu_capabilities_t  *onu_capabilities )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_ONU_CAPABILITIES;
	unsigned char							ctc_ret_val;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							received_num_of_entries=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
    CTC_management_object_port_type_t       port_type   = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
   
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)CTC_PARSE_onu_capabilities
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          onu_capabilities, &ctc_ret_val, &received_left_length  );

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;


	return result;
        
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_ethernet_multiple_management_object_pause"

PON_STATUS PON_CTC_STACK_set_ethernet_multiple_management_object_pause ( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   const unsigned char					               number_of_entries,
		   const CTC_STACK_ethernet_management_object_pause_t  management_object_pause )
{
    short int								result,comparser_result;
    unsigned short							received_left_length = 0, sent_length=ETHERNET_MTU, received_length;
	unsigned char							send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf  =  CTC_EX_VAR_ETH_PORT_PAUSE;
	unsigned char							received_num_of_entries=number_of_entries;
	unsigned char							ctc_ret_val,i;
	unsigned short							max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char							local_number_of_entries=0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    if(number_of_entries != CTC_STACK_ALL_PORTS)
    {
        for(i = 0; i < number_of_entries; i++)
        {
            CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_pause[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
        }
    }

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_pause[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_ethrnet_port_pause
									( 1, &management_object_pause[i].flow_control_enable, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_pause[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_ethrnet_port_pause
									( 1, &management_object_pause[0].flow_control_enable, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}


	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


		received_total_length += received_left_length;

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_pause[i].management_object_index.port_number)
		}


		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
		
	}
	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_multiple_management_object_pause"

PON_STATUS PON_CTC_STACK_get_ethernet_multiple_management_object_pause ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char				             	 *number_of_entries,
		   CTC_STACK_ethernet_management_object_pause_t   management_object_pause)
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_ETH_PORT_PAUSE;
	unsigned char							received_num_of_entries=0;
	unsigned char							ctc_ret_val,i;
	unsigned short							max_length=ETHERNET_MTU,received_max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;


	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);
    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC


    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_pause[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_pause[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_pause[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_pause[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_ethernet_port_pause
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, /*TRUE,*/ &received_num_of_entries,
         									  &management_object_pause[i].flow_control_enable, &ctc_ret_val ,&received_left_length  );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;


			if(received_num_of_entries != 1)
			{
				
				PONLOG_ERROR_4( PONLOG_FUNC_GENERAL, olt_id, onu_id,
								"OLT %d ONU %d expected entries: %d. received entries %d\n",
								olt_id, onu_id, 1, received_num_of_entries);


				return CTC_STACK_MEMORY_ERROR;
			}
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			

			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_ethernet_port_pause
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, /*TRUE,*/ &received_num_of_entries,
         									  &management_object_pause[i].flow_control_enable, &ctc_ret_val ,&received_left_length   );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;


			management_object_pause[i].management_object_index.port_number  = received_management_object.index.port_number;
			management_object_pause[i].management_object_index.frame_number = received_management_object.index.frame_number;
			management_object_pause[i].management_object_index.slot_number  = received_management_object.index.slot_number;
			management_object_pause[i].management_object_index.port_type    = received_management_object.index.port_type;

			i++;
		}

		(*number_of_entries) = i;
	}
	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_all_management_object_pause"

PON_STATUS PON_CTC_STACK_get_ethernet_all_management_object_pause ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char					             *number_of_entries,
		   CTC_STACK_ethernet_management_object_pause_t   management_object_info )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_ethernet_multiple_management_object_pause(olt_id, onu_id, number_of_entries, management_object_info));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_ethernet_management_object_pause"

PON_STATUS PON_CTC_STACK_set_ethernet_management_object_pause ( 
           const PON_olt_id_t	                    olt_id, 
           const PON_onu_id_t	                    onu_id,
		   const CTC_management_object_index_t		management_object_index,
		   const bool			                    flow_control_enable)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_ethernet_management_object_pause_t		ethernet_management_object_pause;
	unsigned char							            number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	ethernet_management_object_pause[0].management_object_index.port_number  = management_object_index.port_number;
	ethernet_management_object_pause[0].management_object_index.slot_number  = management_object_index.slot_number;
 	ethernet_management_object_pause[0].management_object_index.frame_number = management_object_index.frame_number;
	ethernet_management_object_pause[0].management_object_index.port_type    = management_object_index.port_type;

	ethernet_management_object_pause[0].flow_control_enable = flow_control_enable;


	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
		number_of_entries = CTC_STACK_ALL_PORTS;
	else
		number_of_entries = 1;

	return PON_CTC_STACK_set_ethernet_multiple_management_object_pause(olt_id, onu_id, number_of_entries, ethernet_management_object_pause);    
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_management_object_pause"

PON_STATUS PON_CTC_STACK_get_ethernet_management_object_pause ( 
           const PON_olt_id_t	                    olt_id, 
           const PON_onu_id_t	                    onu_id,
		   const CTC_management_object_index_t		management_object_index,
		   bool				                       *flow_control_enable)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_ethernet_management_object_pause_t		ethernet_management_object_pause;
	unsigned char							            number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	ethernet_management_object_pause[0].management_object_index.port_number  = management_object_index.port_number;
	ethernet_management_object_pause[0].management_object_index.slot_number  = management_object_index.slot_number;
 	ethernet_management_object_pause[0].management_object_index.frame_number = management_object_index.frame_number;
	ethernet_management_object_pause[0].management_object_index.port_type    = management_object_index.port_type;


	result = PON_CTC_STACK_get_ethernet_multiple_management_object_pause(olt_id, onu_id, &number_of_entries, ethernet_management_object_pause);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	(*flow_control_enable) = ethernet_management_object_pause[0].flow_control_enable;

	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_all_port_pause"

PON_STATUS PON_CTC_STACK_get_ethernet_all_port_pause ( 
           const PON_olt_id_t				 olt_id, 
           const PON_onu_id_t				 onu_id,
		   unsigned char					*number_of_entries,
		   CTC_STACK_ethernet_ports_pause_t  ports_info )
{
    short int								            result;
    unsigned char                                       i;   
    CTC_STACK_ethernet_management_object_pause_t        management_object_pause;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_pause[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_pause[0].management_object_index.slot_number      = 0x00;
    management_object_pause[0].management_object_index.frame_number     = 0x00;
    management_object_pause[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_ethernet_multiple_management_object_pause(olt_id, onu_id, number_of_entries, management_object_pause);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_info[i].port_number         = (unsigned char)(management_object_pause[i].management_object_index.port_number);
        ports_info[i].flow_control_enable = management_object_pause[i].flow_control_enable;
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_ethernet_port_pause"

PON_STATUS PON_CTC_STACK_set_ethernet_port_pause ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const bool			 flow_control_enable)
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_ethernet_management_object_pause_t  management_object_pause;
	unsigned char						          number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_pause[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_pause[0].management_object_index.port_number  = port_number;
    }


    management_object_pause[0].management_object_index.slot_number      = 0x00;
    management_object_pause[0].management_object_index.frame_number     = 0x00;
    management_object_pause[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

    management_object_pause[0].flow_control_enable                      = flow_control_enable;


	result = PON_CTC_STACK_set_ethernet_multiple_management_object_pause(olt_id, onu_id, number_of_entries, management_object_pause);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_port_pause"

PON_STATUS PON_CTC_STACK_get_ethernet_port_pause ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   bool				    *flow_control_enable)
{
    short int								      result=CTC_STACK_EXIT_OK;
    CTC_STACK_ethernet_management_object_pause_t  management_object_pause;
	unsigned char							      number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC


    management_object_pause[0].management_object_index.port_number      = port_number;
    management_object_pause[0].management_object_index.slot_number      = 0x00;
    management_object_pause[0].management_object_index.frame_number     = 0x00;
    management_object_pause[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_ethernet_multiple_management_object_pause(olt_id, onu_id, &number_of_entries, management_object_pause);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*flow_control_enable) = management_object_pause[0].flow_control_enable;


	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_ethernet_multiple_management_object_policing"

PON_STATUS PON_CTC_STACK_set_ethernet_multiple_management_object_policing ( 
           const PON_olt_id_t						              olt_id, 
           const PON_onu_id_t						              onu_id,
		   const unsigned char						              number_of_entries,
		   const CTC_STACK_ethernet_management_object_policing_t  management_object_policing )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_ETH_PORT_POLICING;
	unsigned char								received_num_of_entries=number_of_entries;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char								local_number_of_entries = 0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for(i = 0; i < number_of_entries; i++)
        {
            CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_policing[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
            PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, management_object_policing[i].entry.cir,cir,0,CTC_MAX_POLICING_PARAMETERS_VALUE)  
            PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, management_object_policing[i].entry.bucket_depth,bucket_depth,0,CTC_MAX_POLICING_PARAMETERS_VALUE)  
            PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, management_object_policing[i].entry.extra_burst_size,extra_burst_size,0,CTC_MAX_POLICING_PARAMETERS_VALUE)  
        }
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_policing[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_ethrnet_port_policing
									( 1, &management_object_policing[i].entry, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_policing[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_ethrnet_port_policing
									( 1, &management_object_policing[0].entry, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}

	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_policing[i].management_object_index.port_number)
		}


		received_total_length += received_left_length;

		comparser_result = (short int)CTC_PARSE_general_ack
							( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        					  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
		
	}

	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_multiple_management_object_policing"

PON_STATUS PON_CTC_STACK_get_ethernet_multiple_management_object_policing ( 
           const PON_olt_id_t				                olt_id, 
           const PON_onu_id_t				                onu_id,
		   unsigned char					               *number_of_entries,
		   CTC_STACK_ethernet_management_object_policing_t  management_object_policing )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_ETH_PORT_POLICING;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_policing[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])

            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_policing[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_policing[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_policing[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_ethernet_port_policing
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_policing[i].entry, &ctc_ret_val ,&received_left_length );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
			received_total_length += received_left_length;


			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_ethernet_port_policing
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_policing[i].entry, &ctc_ret_val ,&received_left_length );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
			
			management_object_policing[i].management_object_index.port_number  = received_management_object.index.port_number;
			management_object_policing[i].management_object_index.frame_number = received_management_object.index.frame_number;
			management_object_policing[i].management_object_index.slot_number  = received_management_object.index.slot_number;
			management_object_policing[i].management_object_index.port_type    = received_management_object.index.port_type;
			i++;
		}

		(*number_of_entries) = i;
	}
    
	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_all_management_object_policing"

PON_STATUS PON_CTC_STACK_get_ethernet_all_management_object_policing ( 
           const PON_olt_id_t					             olt_id, 
           const PON_onu_id_t					             onu_id,
		   unsigned char						            *number_of_entries,
		   CTC_STACK_ethernet_management_object_policing_t   management_object_info )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_ethernet_multiple_management_object_policing(olt_id, onu_id, number_of_entries, management_object_info));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_ethernet_management_object_policing"

PON_STATUS PON_CTC_STACK_set_ethernet_management_object_policing ( 
           const PON_olt_id_t								             olt_id, 
           const PON_onu_id_t								             onu_id,
		   const CTC_management_object_index_t		                     management_object_index,
		   const CTC_STACK_ethernet_port_policing_entry_t                port_entry)
{
    short int								         result=CTC_STACK_EXIT_OK;
	CTC_STACK_ethernet_management_object_policing_t  management_object_policing;
	unsigned char						             number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    management_object_policing[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_policing[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_policing[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_policing[0].management_object_index.port_type        = management_object_index.port_type;

	management_object_policing[0].entry.operation                          = port_entry.operation;
	management_object_policing[0].entry.cir                                = port_entry.cir;
	management_object_policing[0].entry.bucket_depth                       = port_entry.bucket_depth;
	management_object_policing[0].entry.extra_burst_size                   = port_entry.extra_burst_size;


	result = PON_CTC_STACK_set_ethernet_multiple_management_object_policing(olt_id, onu_id, number_of_entries, management_object_policing);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_management_object_policing"

PON_STATUS PON_CTC_STACK_get_ethernet_management_object_policing ( 
           const PON_olt_id_t						  olt_id, 
           const PON_onu_id_t						  onu_id,
		   const CTC_management_object_index_t		  management_object_index,
		   CTC_STACK_ethernet_port_policing_entry_t  *port_policing)
{
    short int								         result=CTC_STACK_EXIT_OK;
    CTC_STACK_ethernet_management_object_policing_t  management_object_policing;
	unsigned char							         number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_policing[0].management_object_index.port_number   = management_object_index.port_number;
    management_object_policing[0].management_object_index.slot_number   = management_object_index.slot_number;
    management_object_policing[0].management_object_index.frame_number  = management_object_index.frame_number;
    management_object_policing[0].management_object_index.port_type     = management_object_index.port_type;


    result = PON_CTC_STACK_get_ethernet_multiple_management_object_policing(olt_id, onu_id, &number_of_entries, management_object_policing);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	port_policing->operation = management_object_policing[0].entry.operation;
	port_policing->cir = management_object_policing[0].entry.cir;
	port_policing->bucket_depth = management_object_policing[0].entry.bucket_depth;
	port_policing->extra_burst_size = management_object_policing[0].entry.extra_burst_size;


	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_all_port_policing"

PON_STATUS PON_CTC_STACK_get_ethernet_all_port_policing ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   unsigned char						*number_of_entries,
		   CTC_STACK_ethernet_ports_policing_t   ports_info )
{
    short int								            result;
    unsigned char                                       i;   
    CTC_STACK_ethernet_management_object_policing_t     management_object_policing;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_policing[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_policing[0].management_object_index.slot_number      = 0x00;
    management_object_policing[0].management_object_index.frame_number     = 0x00;
    management_object_policing[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_ethernet_multiple_management_object_policing(olt_id, onu_id, number_of_entries, management_object_policing);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_info[i].port_number            = (unsigned char)(management_object_policing[i].management_object_index.port_number);
        ports_info[i].entry.operation        = management_object_policing[i].entry.operation;
        ports_info[i].entry.cir              = management_object_policing[i].entry.cir;
        ports_info[i].entry.bucket_depth     = management_object_policing[i].entry.bucket_depth;
        ports_info[i].entry.extra_burst_size = management_object_policing[i].entry.extra_burst_size;
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_ethernet_port_policing"

PON_STATUS PON_CTC_STACK_set_ethernet_port_policing ( 
           const PON_olt_id_t								olt_id, 
           const PON_onu_id_t								onu_id,
		   const unsigned char								port_number,
		   const CTC_STACK_ethernet_port_policing_entry_t   port_policing)
{
    short int								         result=CTC_STACK_EXIT_OK;
	CTC_STACK_ethernet_management_object_policing_t  management_object_policing;
	unsigned char						             number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_policing[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_policing[0].management_object_index.port_number  = port_number;
    }


    management_object_policing[0].management_object_index.slot_number      = 0x00;
    management_object_policing[0].management_object_index.frame_number     = 0x00;
    management_object_policing[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

	management_object_policing[0].entry.operation                          = port_policing.operation;
	management_object_policing[0].entry.cir                                = port_policing.cir;
	management_object_policing[0].entry.bucket_depth                       = port_policing.bucket_depth;
	management_object_policing[0].entry.extra_burst_size                   = port_policing.extra_burst_size;


	result = PON_CTC_STACK_set_ethernet_multiple_management_object_policing(olt_id, onu_id, number_of_entries, management_object_policing);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_port_policing"

PON_STATUS PON_CTC_STACK_get_ethernet_port_policing ( 
           const PON_olt_id_t						  olt_id, 
           const PON_onu_id_t						  onu_id,
		   const unsigned char						  port_number,
		   CTC_STACK_ethernet_port_policing_entry_t  *port_policing)
{
    short int								         result=CTC_STACK_EXIT_OK;
    CTC_STACK_ethernet_management_object_policing_t  management_object_policing;
	unsigned char							         number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_policing[0].management_object_index.port_number      = port_number;
    management_object_policing[0].management_object_index.slot_number      = 0x00;
    management_object_policing[0].management_object_index.frame_number     = 0x00;
    management_object_policing[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_ethernet_multiple_management_object_policing(olt_id, onu_id, &number_of_entries, management_object_policing);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	port_policing->operation = management_object_policing[0].entry.operation;
	port_policing->cir = management_object_policing[0].entry.cir;
	port_policing->bucket_depth = management_object_policing[0].entry.bucket_depth;
	port_policing->extra_burst_size = management_object_policing[0].entry.extra_burst_size;


	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_voip_multiple_management_object"

PON_STATUS PON_CTC_STACK_set_voip_multiple_management_object ( 
           const PON_olt_id_t			               olt_id, 
           const PON_onu_id_t			               onu_id,
		   const unsigned char			               number_of_entries,
		   const CTC_STACK_management_object_state_t   management_object_state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_VOIP_PORT;
	unsigned char								received_num_of_entries=number_of_entries;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char								local_number_of_entries = 0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for(i = 0; i < number_of_entries; i++)
		{
			CHECK_CTC_STACK_OBJECT_VOIP_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_state[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id]) 
		}
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_state[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_voip_port
									( 1, &management_object_state[i].port_state, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_state[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_voip_port
									( 1, &management_object_state[0].port_state, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER , &received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}

	for( i = 0; i < local_number_of_entries; i++)
	{	
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_state[i].management_object_index.port_number)
		}



		received_total_length += received_left_length;

		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        				  expected_leaf, &ctc_ret_val, &received_left_length  );


		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
	}


	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_voip_multiple_management_object"

PON_STATUS PON_CTC_STACK_get_voip_multiple_management_object ( 
           const PON_olt_id_t		             olt_id, 
           const PON_onu_id_t		             onu_id,
		   unsigned char			            *number_of_entries,
		   CTC_STACK_management_object_state_t   management_object_state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_VOIP_PORT;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_VOIP_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_state[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])   
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_state[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_state[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_state[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_voip_port
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_state[i].port_state, &ctc_ret_val ,&received_left_length );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_voip_port
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &(management_object_state[i].port_state), &ctc_ret_val ,&received_left_length );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
			
			management_object_state[i].management_object_index.port_number  = received_management_object.index.port_number;
			management_object_state[i].management_object_index.frame_number = received_management_object.index.frame_number;
			management_object_state[i].management_object_index.slot_number  = received_management_object.index.slot_number;
			management_object_state[i].management_object_index.port_type    = received_management_object.index.port_type;
			i++;
		}

		(*number_of_entries) = i;
	}
    
	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_voip_all_management_object"

PON_STATUS PON_CTC_STACK_get_voip_all_management_object ( 
           const PON_olt_id_t		             olt_id, 
           const PON_onu_id_t		             onu_id,
		   unsigned char		             	*number_of_entries,
		   CTC_STACK_management_object_state_t   management_object_info )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_voip_multiple_management_object(olt_id, onu_id, number_of_entries, management_object_info));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_voip_management_object"

PON_STATUS PON_CTC_STACK_set_voip_management_object ( 
           const PON_olt_id_t				    olt_id, 
           const PON_onu_id_t				    onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   const CTC_STACK_on_off_state_t       port_state )
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_state_t           management_object_state;
	unsigned char						          number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    management_object_state[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_state[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_state[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_state[0].management_object_index.port_type        = management_object_index.port_type;

    management_object_state[0].port_state                               = port_state;


	result = PON_CTC_STACK_set_voip_multiple_management_object(olt_id, onu_id, number_of_entries, management_object_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_voip_management_object"

PON_STATUS PON_CTC_STACK_get_voip_management_object ( 
           const PON_olt_id_t		            olt_id, 
           const PON_onu_id_t		            onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   CTC_STACK_on_off_state_t            *port_state)
{
    short int								      result=CTC_STACK_EXIT_OK;
    CTC_STACK_management_object_state_t           management_object_state;
	unsigned char							      number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_state[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_state[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_state[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_state[0].management_object_index.port_type        = management_object_index.port_type;


    result = PON_CTC_STACK_get_voip_multiple_management_object(olt_id, onu_id, &number_of_entries, management_object_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*port_state) = management_object_state[0].port_state;


	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_voip_all_port"

PON_STATUS PON_CTC_STACK_get_voip_all_port ( 
           const PON_olt_id_t		 olt_id, 
           const PON_onu_id_t		 onu_id,
		   unsigned char			*number_of_entries,
		   CTC_STACK_ports_state_t   ports_info )
{
    short int								   result;
    unsigned char                              i;   
    CTC_STACK_management_object_state_t        management_object_state;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_state[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_state[0].management_object_index.slot_number      = 0x00;
    management_object_state[0].management_object_index.frame_number     = 0x00;
    management_object_state[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;


    result = PON_CTC_STACK_get_voip_multiple_management_object(olt_id, onu_id, number_of_entries, management_object_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_info[i].port_number         = (unsigned char)(management_object_state[i].management_object_index.port_number);
        ports_info[i].port_state          = management_object_state[i].port_state;
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_voip_ports"

PON_STATUS PON_CTC_STACK_set_voip_port ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   const unsigned char				port_number,
		   const CTC_STACK_on_off_state_t   port_state )
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_state_t           management_object_state;
	unsigned char						          number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_state[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_state[0].management_object_index.port_number  = port_number;
    }


    management_object_state[0].management_object_index.slot_number      = 0x00;
    management_object_state[0].management_object_index.frame_number     = 0x00;
    management_object_state[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;

    management_object_state[0].port_state                               = port_state;


	result = PON_CTC_STACK_set_voip_multiple_management_object(olt_id, onu_id, number_of_entries, management_object_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_voip_port"

PON_STATUS PON_CTC_STACK_get_voip_port ( 
           const PON_olt_id_t		  olt_id, 
           const PON_onu_id_t		  onu_id,
		   const unsigned char		  port_number,
		   CTC_STACK_on_off_state_t  *port_state)
{
    short int								      result=CTC_STACK_EXIT_OK;
    CTC_STACK_management_object_state_t           management_object_state;
	unsigned char							      number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_state[0].management_object_index.port_number      = port_number;
    management_object_state[0].management_object_index.slot_number      = 0x00;
    management_object_state[0].management_object_index.frame_number     = 0x00;
    management_object_state[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;


    result = PON_CTC_STACK_get_voip_multiple_management_object(olt_id, onu_id, &number_of_entries, management_object_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*port_state) = management_object_state[0].port_state;


	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_e1_multiple_management_object"

PON_STATUS PON_CTC_STACK_set_e1_multiple_management_object ( 
           const PON_olt_id_t			               olt_id, 
           const PON_onu_id_t			               onu_id,
		   const unsigned char			               number_of_entries,
		   const CTC_STACK_management_object_state_t   management_object_state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_E1_PORT;
	unsigned char								received_num_of_entries=number_of_entries;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char								local_number_of_entries = 0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for(i = 0; i < number_of_entries; i++)
		{
			CHECK_CTC_STACK_OBJECT_E1_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_state[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
		}
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_state[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_e1_port
									( 1, &management_object_state[i].port_state, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_state[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_e1_port
									( 1, &management_object_state[0].port_state, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER , &received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}

	for( i = 0; i < local_number_of_entries; i++)
	{	
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_state[i].management_object_index.port_number)
		}


		received_total_length += received_left_length;

		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        				  expected_leaf, &ctc_ret_val, &received_left_length  );


		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
	}


	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_e1_multiple_management_object"

PON_STATUS PON_CTC_STACK_get_e1_multiple_management_object ( 
           const PON_olt_id_t		             olt_id, 
           const PON_onu_id_t		             onu_id,
		   unsigned char		             	*number_of_entries,
		   CTC_STACK_management_object_state_t   management_object_state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_E1_PORT;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_E1_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_state[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])   
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_state[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_state[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_state[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_e1_port
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_state[i].port_state, &ctc_ret_val ,&received_left_length );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_e1_port
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_state[i].port_state, &ctc_ret_val ,&received_left_length );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
			
			management_object_state[i].management_object_index.port_number  = received_management_object.index.port_number;
			management_object_state[i].management_object_index.frame_number = received_management_object.index.frame_number;
			management_object_state[i].management_object_index.slot_number  = received_management_object.index.slot_number;
			management_object_state[i].management_object_index.port_type    = received_management_object.index.port_type;
			i++;
		}

		(*number_of_entries) = i;
	}
    
	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_e1_all_management_object"

PON_STATUS PON_CTC_STACK_get_e1_all_management_object ( 
           const PON_olt_id_t		             olt_id, 
           const PON_onu_id_t		             onu_id,
		   unsigned char		             	*number_of_entries,
		   CTC_STACK_management_object_state_t   management_object_info )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_e1_multiple_management_object(olt_id, onu_id, number_of_entries, management_object_info));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_e1_management_object"

PON_STATUS PON_CTC_STACK_set_e1_management_object ( 
           const PON_olt_id_t				   olt_id, 
           const PON_onu_id_t				   onu_id,
		   const CTC_management_object_index_t management_object_index,
		   const CTC_STACK_on_off_state_t      port_state )
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_state_t           management_object_state;
	unsigned char						          number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    management_object_state[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_state[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_state[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_state[0].management_object_index.port_type        = management_object_index.port_type;

    management_object_state[0].port_state                               = port_state;


	result = PON_CTC_STACK_set_e1_multiple_management_object(olt_id, onu_id, number_of_entries, management_object_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_e1_management_object"

PON_STATUS PON_CTC_STACK_get_e1_management_object ( 
           const PON_olt_id_t		            olt_id, 
           const PON_onu_id_t		            onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   CTC_STACK_on_off_state_t            *port_state)
{
    short int								      result=CTC_STACK_EXIT_OK;
    CTC_STACK_management_object_state_t           management_object_state;
	unsigned char							      number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_state[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_state[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_state[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_state[0].management_object_index.port_type        = management_object_index.port_type;


    result = PON_CTC_STACK_get_e1_multiple_management_object(olt_id, onu_id, &number_of_entries, management_object_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*port_state) = management_object_state[0].port_state;


	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_e1_all_port"

PON_STATUS PON_CTC_STACK_get_e1_all_port ( 
           const PON_olt_id_t		 olt_id, 
           const PON_onu_id_t		 onu_id,
		   unsigned char			*number_of_entries,
		   CTC_STACK_ports_state_t   ports_info )
{
    short int								   result;
    unsigned char                              i;   
    CTC_STACK_management_object_state_t        management_object_state;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    
	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_state[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_state[0].management_object_index.slot_number      = 0x00;
    management_object_state[0].management_object_index.frame_number     = 0x00;
    management_object_state[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_E1_PORT;


    result = PON_CTC_STACK_get_e1_multiple_management_object(olt_id, onu_id, number_of_entries, management_object_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_info[i].port_number         = (unsigned char)(management_object_state[i].management_object_index.port_number);
        ports_info[i].port_state          = management_object_state[i].port_state;
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_e1_ports"

PON_STATUS PON_CTC_STACK_set_e1_port ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   const unsigned char				port_number,
		   const CTC_STACK_on_off_state_t   port_state )
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_state_t           management_object_state;
	unsigned char						          number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_state[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_state[0].management_object_index.port_number  = port_number;
    }


    management_object_state[0].management_object_index.slot_number      = 0x00;
    management_object_state[0].management_object_index.frame_number     = 0x00;
    management_object_state[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_E1_PORT;

    management_object_state[0].port_state                               = port_state;


	result = PON_CTC_STACK_set_e1_multiple_management_object(olt_id, onu_id, number_of_entries, management_object_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_e1_port"

PON_STATUS PON_CTC_STACK_get_e1_port ( 
           const PON_olt_id_t		  olt_id, 
           const PON_onu_id_t		  onu_id,
		   const unsigned char		  port_number,
		   CTC_STACK_on_off_state_t  *port_state)
{
    short int								      result=CTC_STACK_EXIT_OK;
    CTC_STACK_management_object_state_t           management_object_state;
	unsigned char							      number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_state[0].management_object_index.port_number      = port_number;
    management_object_state[0].management_object_index.slot_number      = 0x00;
    management_object_state[0].management_object_index.frame_number     = 0x00;
    management_object_state[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_E1_PORT;


    result = PON_CTC_STACK_get_e1_multiple_management_object(olt_id, onu_id, &number_of_entries, management_object_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*port_state) = management_object_state[0].port_state;


	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_vlan_multiple_management_object_configuration"

PON_STATUS PON_CTC_STACK_set_vlan_multiple_management_object_configuration ( 
           const PON_olt_id_t						                olt_id, 
           const PON_onu_id_t						                onu_id,
		   const unsigned char							            number_of_entries,
		   const CTC_STACK_vlan_configuration_management_object_t   management_object_vlan_configuration )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char							send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf   =  CTC_EX_VAR_VLAN;
	unsigned char							received_num_of_entries=1;
	unsigned char							ctc_ret_val;
	unsigned char							i;
	unsigned short							max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char							local_number_of_entries = 0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    if(number_of_entries != CTC_STACK_ALL_PORTS)
    {
        for(i = 0; i < number_of_entries; i++)
        {
            CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_vlan_configuration[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
        }
    }

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_vlan_configuration[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_vlan
                            ( 1/*number_of_entries*/, &management_object_vlan_configuration[i].entry, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
                              SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_vlan_configuration[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_vlan
									( 1, &management_object_vlan_configuration[0].entry, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

	}
	


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}


	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		received_total_length += received_left_length;


		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_vlan_configuration[i].management_object_index.port_number)
		}



		comparser_result = (short int)CTC_PARSE_general_ack
							( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        					  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;	
		
	}
	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_vlan_multiple_management_object_configuration"

PON_STATUS PON_CTC_STACK_get_vlan_multiple_management_object_configuration ( 
           const PON_olt_id_t					              olt_id, 
           const PON_onu_id_t					              onu_id,
		   unsigned char						             *number_of_entries,
		   CTC_STACK_vlan_configuration_management_object_t   management_object_vlan_configuration )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_VLAN;
	unsigned char							received_num_of_entries=0;
	unsigned char							ctc_ret_val, i;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_vlan_configuration[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_vlan_configuration[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_vlan_configuration[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}

	
    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_vlan_configuration[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_vlan
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_vlan_configuration[i].entry, &ctc_ret_val,&received_left_length  );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;


			if(received_num_of_entries != 1)
			{
				
				PONLOG_ERROR_4( PONLOG_FUNC_GENERAL, olt_id, onu_id,
								"OLT %d ONU %d expected received entries: %d. received number of entries %d\n",
								olt_id, onu_id, 1, received_num_of_entries);


				return CTC_STACK_MEMORY_ERROR;
			}
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			
            if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_vlan
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_vlan_configuration[i].entry, &ctc_ret_val,&received_left_length  );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;

			management_object_vlan_configuration[i].management_object_index.port_number  = received_management_object.index.port_number;			
			management_object_vlan_configuration[i].management_object_index.frame_number = received_management_object.index.frame_number;
			management_object_vlan_configuration[i].management_object_index.slot_number  = received_management_object.index.slot_number;
			management_object_vlan_configuration[i].management_object_index.port_type    = received_management_object.index.port_type;
			i++;

		}

		(*number_of_entries) = i;
	}
	
	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_vlan_all_management_object_configuration"

PON_STATUS PON_CTC_STACK_get_vlan_all_management_object_configuration ( 
           const PON_olt_id_t					              olt_id, 
           const PON_onu_id_t					              onu_id,
		   unsigned char						             *number_of_entries,
		   CTC_STACK_vlan_configuration_management_object_t   management_object_info )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_vlan_multiple_management_object_configuration(olt_id, onu_id, number_of_entries, management_object_info));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_vlan_management_object_configuration"

PON_STATUS PON_CTC_STACK_set_vlan_management_object_configuration ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_port_vlan_configuration_t   port_configuration)
{
    short int                                         result=CTC_STACK_EXIT_OK;
    CTC_STACK_vlan_configuration_management_object_t  management_object_vlan;
    unsigned char                                     number_of_entries, i;
    unsigned char                                 table_index, entry_index;

    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    management_object_vlan[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_vlan[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_vlan[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_vlan[0].management_object_index.port_type        = management_object_index.port_type;

    management_object_vlan[0].entry.mode                               = port_configuration.mode;
    management_object_vlan[0].entry.default_vlan                       = port_configuration.default_vlan;
    management_object_vlan[0].entry.number_of_entries                  = port_configuration.number_of_entries;
    management_object_vlan[0].entry.number_of_aggregation_tables = port_configuration.number_of_aggregation_tables;

    if(port_configuration.mode == CTC_VLAN_MODE_TRUNK)
    {
        for( i = 0 ; i < port_configuration.number_of_entries; i++)
        {
            management_object_vlan[0].entry.vlan_list[i] = port_configuration.vlan_list[i];
        }
    }
    else if((port_configuration.mode == CTC_VLAN_MODE_TRANSLATION)||(port_configuration.mode == CTC_VLAN_MODE_TAG))
    {
        for( i = 0 ; i < port_configuration.number_of_entries*2; i++)
        {
            management_object_vlan[0].entry.vlan_list[i] = port_configuration.vlan_list[i];
        }
    }

    if(port_configuration.mode == CTC_VLAN_MODE_AGGREGATION)
    {
        for(table_index = 0; table_index < port_configuration.number_of_aggregation_tables; table_index++)
        {
            management_object_vlan[0].entry.vlan_aggregation_tables[table_index].number_of_aggregation_entries = port_configuration.vlan_aggregation_tables[table_index].number_of_aggregation_entries;
            management_object_vlan[0].entry.vlan_aggregation_tables[table_index].target_vlan = port_configuration.vlan_aggregation_tables[table_index].target_vlan;
            for(entry_index = 0; entry_index < port_configuration.vlan_aggregation_tables[table_index].number_of_aggregation_entries; entry_index++)
            {
                management_object_vlan[0].entry.vlan_aggregation_tables[table_index].vlan_aggregation_list[entry_index] = port_configuration.vlan_aggregation_tables[table_index].vlan_aggregation_list[entry_index];
            }
        }
    }
    else
    {
        management_object_vlan[0].entry.number_of_aggregation_tables = 0;
    }

    result = PON_CTC_STACK_set_vlan_multiple_management_object_configuration(olt_id, onu_id, number_of_entries, management_object_vlan);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_vlan_management_object_configuration"

PON_STATUS PON_CTC_STACK_get_vlan_management_object_configuration ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const CTC_management_object_index_t   management_object_index,
		   CTC_STACK_port_vlan_configuration_t  *port_configuration)
{
    short int								            result=CTC_STACK_EXIT_OK;
    CTC_STACK_vlan_configuration_management_object_t    management_object_vlan;
	unsigned char							            number_of_entries = 1, i;
    unsigned char                                 table_index, entry_index;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_vlan[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_vlan[0].management_object_index.slot_number      = management_object_index.slot_number ;
    management_object_vlan[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_vlan[0].management_object_index.port_type        = management_object_index.port_type;


    result = PON_CTC_STACK_get_vlan_multiple_management_object_configuration(olt_id, onu_id, &number_of_entries, management_object_vlan);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    port_configuration->mode              = management_object_vlan[0].entry.mode;
    port_configuration->default_vlan      = management_object_vlan[0].entry.default_vlan;
    port_configuration->number_of_entries = management_object_vlan[0].entry.number_of_entries;
    port_configuration->number_of_aggregation_tables = management_object_vlan[0].entry.number_of_aggregation_tables;

    if(port_configuration->mode == CTC_VLAN_MODE_TRUNK)
    {
        for( i = 0 ; i < management_object_vlan[0].entry.number_of_entries; i++)
        {
        	port_configuration->vlan_list[i] = management_object_vlan[0].entry.vlan_list[i];
        }
    }
    else if((port_configuration->mode == CTC_VLAN_MODE_TRANSLATION)||(port_configuration->mode == CTC_VLAN_MODE_TAG))
    {
        for( i = 0 ; i < management_object_vlan[0].entry.number_of_entries*2; i++)
        {
        	port_configuration->vlan_list[i] = management_object_vlan[0].entry.vlan_list[i];
        }
    }
    if(port_configuration->mode == CTC_VLAN_MODE_AGGREGATION)
    {
        for(table_index = 0; table_index < port_configuration->number_of_aggregation_tables; table_index++)
        {
            port_configuration->vlan_aggregation_tables[table_index].number_of_aggregation_entries = management_object_vlan[0].entry.vlan_aggregation_tables[table_index].number_of_aggregation_entries;
            port_configuration->vlan_aggregation_tables[table_index].target_vlan = management_object_vlan[0].entry.vlan_aggregation_tables[table_index].target_vlan;
            for(entry_index = 0; entry_index < port_configuration->vlan_aggregation_tables[table_index].number_of_aggregation_entries; entry_index++)
            {
                port_configuration->vlan_aggregation_tables[table_index].vlan_aggregation_list[entry_index] = management_object_vlan[0].entry.vlan_aggregation_tables[table_index].vlan_aggregation_list[entry_index];
            }
        }
    }
    else
    {
        port_configuration->number_of_aggregation_tables = 0;
    }

    return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_vlan_all_port_configuration"

PON_STATUS PON_CTC_STACK_get_vlan_all_port_configuration ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   unsigned char						 *number_of_entries,
		   CTC_STACK_vlan_configuration_ports_t   ports_info )
{
    short int								            result;
    unsigned char                                       i, j, table_index, entry_index;   
    CTC_STACK_vlan_configuration_management_object_t    management_object_vlan;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_vlan[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_vlan[0].management_object_index.slot_number      = 0x00;
    management_object_vlan[0].management_object_index.frame_number     = 0x00;
    management_object_vlan[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

	/*暂时规避掉8xep 所有vlan获取操作,防止524的第25口越界*/
	return CTC_STACK_EXIT_ERROR;

    result = PON_CTC_STACK_get_vlan_multiple_management_object_configuration(olt_id, onu_id, number_of_entries, management_object_vlan);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_info[i].port_number                 = (unsigned char)(management_object_vlan[i].management_object_index.port_number);
        ports_info[i].entry.mode                  = management_object_vlan[i].entry.mode;
        ports_info[i].entry.default_vlan          = management_object_vlan[i].entry.default_vlan;
        ports_info[i].entry.number_of_entries     = management_object_vlan[i].entry.number_of_entries;
    	ports_info[i].entry.number_of_aggregation_tables = management_object_vlan[i].entry.number_of_aggregation_tables;

	    if(ports_info[i].entry.mode == CTC_VLAN_MODE_TRUNK)
	    {
	        for( j = 0 ; j < management_object_vlan[i].entry.number_of_entries; j++)
	        {
	        	ports_info[i].entry.vlan_list[j] = management_object_vlan[i].entry.vlan_list[j];
	        }
	    }
	    else if((ports_info[i].entry.mode == CTC_VLAN_MODE_TRANSLATION)||(ports_info[i].entry.mode == CTC_VLAN_MODE_TAG))
	    {
	        for( j = 0 ; j < management_object_vlan[i].entry.number_of_entries*2; j++)
	        {
	        	ports_info[i].entry.vlan_list[j] = management_object_vlan[i].entry.vlan_list[j];
	        }
	    }
		
		if(ports_info[i].entry.mode == CTC_VLAN_MODE_AGGREGATION)
	    {
	        for(table_index = 0; table_index < ports_info[i].entry.number_of_aggregation_tables; table_index++)
	        {
	            ports_info[i].entry.vlan_aggregation_tables[table_index].number_of_aggregation_entries = management_object_vlan[i].entry.vlan_aggregation_tables[table_index].number_of_aggregation_entries;
	            ports_info[i].entry.vlan_aggregation_tables[table_index].target_vlan = management_object_vlan[i].entry.vlan_aggregation_tables[table_index].target_vlan;
	            for(entry_index = 0; entry_index < ports_info[i].entry.vlan_aggregation_tables[table_index].number_of_aggregation_entries; entry_index++)
	            {
	                ports_info[i].entry.vlan_aggregation_tables[table_index].vlan_aggregation_list[entry_index] = management_object_vlan[i].entry.vlan_aggregation_tables[table_index].vlan_aggregation_list[entry_index];
	            }
	        }
	    }
	    else
	    {
	        ports_info[i].entry.number_of_aggregation_tables = 0;
	    }
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_vlan_port_configuration"

PON_STATUS PON_CTC_STACK_set_vlan_port_configuration ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const unsigned char						   port_number,
		   const CTC_STACK_port_vlan_configuration_t   port_configuration)
{
    short int								          result=CTC_STACK_EXIT_OK;
	CTC_STACK_vlan_configuration_management_object_t  management_object_vlan;
	unsigned char						              number_of_entries, i;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_vlan[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_vlan[0].management_object_index.port_number  = port_number;
    }


    management_object_vlan[0].management_object_index.slot_number      = 0x00;
    management_object_vlan[0].management_object_index.frame_number     = 0x00;
    management_object_vlan[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

    management_object_vlan[0].entry.mode                               = port_configuration.mode;
    management_object_vlan[0].entry.default_vlan                       = port_configuration.default_vlan;
    management_object_vlan[0].entry.number_of_entries                  = port_configuration.number_of_entries;

    /*modified by luh 2013-1-22*/
	for( i = 0 ; i < port_configuration.number_of_entries; i++)
	{
    	if(port_configuration.mode == CTC_VLAN_MODE_TRANSLATION)
    	{
    		management_object_vlan[0].entry.vlan_list[2*i] = port_configuration.vlan_list[2*i];
    		management_object_vlan[0].entry.vlan_list[2*i+1] = port_configuration.vlan_list[2*i+1];
    	}
        else
    		management_object_vlan[0].entry.vlan_list[i] = port_configuration.vlan_list[i];
	}

	if(port_configuration.mode == CTC_VLAN_MODE_AGGREGATION)
	{
		
		management_object_vlan[0].entry.number_of_aggregation_tables = port_configuration.number_of_aggregation_tables;
		for(i=0; i<port_configuration.number_of_aggregation_tables; i++)
		{
			int j = 0;
			management_object_vlan[0].entry.vlan_aggregation_tables[i].target_vlan = 
				port_configuration.vlan_aggregation_tables[i].target_vlan;
			management_object_vlan[0].entry.vlan_aggregation_tables[i].number_of_aggregation_entries =
				port_configuration.vlan_aggregation_tables[i].number_of_aggregation_entries;
			for(j=0; j<port_configuration.vlan_aggregation_tables[i].number_of_aggregation_entries*2; j++)
				management_object_vlan[0].entry.vlan_aggregation_tables[i].vlan_aggregation_list[j] = 
				port_configuration.vlan_aggregation_tables[i].vlan_aggregation_list[j];
		}
		
	}

	result = PON_CTC_STACK_set_vlan_multiple_management_object_configuration(olt_id, onu_id, number_of_entries, management_object_vlan);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_vlan_port_configuration"

PON_STATUS PON_CTC_STACK_get_vlan_port_configuration ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const unsigned char					 port_number,
		   CTC_STACK_port_vlan_configuration_t  *port_configuration)
{
    short int								            result=CTC_STACK_EXIT_OK;
    CTC_STACK_vlan_configuration_management_object_t    management_object_vlan;
	unsigned char							            number_of_entries = 1, i, table_index, entry_index;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_vlan[0].management_object_index.port_number      = port_number;
    management_object_vlan[0].management_object_index.slot_number      = 0x00;
    management_object_vlan[0].management_object_index.frame_number     = 0x00;
    management_object_vlan[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_vlan_multiple_management_object_configuration(olt_id, onu_id, &number_of_entries, management_object_vlan);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    port_configuration->mode              = management_object_vlan[0].entry.mode;
    port_configuration->default_vlan      = management_object_vlan[0].entry.default_vlan;
    port_configuration->number_of_entries = management_object_vlan[0].entry.number_of_entries;
    port_configuration->number_of_aggregation_tables = management_object_vlan[0].entry.number_of_aggregation_tables;

    if(port_configuration->mode == CTC_VLAN_MODE_TRUNK)
    {
        for( i = 0 ; i < management_object_vlan[0].entry.number_of_entries; i++)
        {
        	port_configuration->vlan_list[i] = management_object_vlan[0].entry.vlan_list[i];
        }
    }
    else if((port_configuration->mode == CTC_VLAN_MODE_TRANSLATION)||(port_configuration->mode == CTC_VLAN_MODE_TAG))
    {
        for( i = 0 ; i < management_object_vlan[0].entry.number_of_entries*2; i++)
        {
        	port_configuration->vlan_list[i] = management_object_vlan[0].entry.vlan_list[i];
        }
    }
	
	if(port_configuration->mode == CTC_VLAN_MODE_AGGREGATION)
    {
        for(table_index = 0; table_index < port_configuration->number_of_aggregation_tables; table_index++)
        {
            port_configuration->vlan_aggregation_tables[table_index].number_of_aggregation_entries = management_object_vlan[0].entry.vlan_aggregation_tables[table_index].number_of_aggregation_entries;
            port_configuration->vlan_aggregation_tables[table_index].target_vlan = management_object_vlan[0].entry.vlan_aggregation_tables[table_index].target_vlan;
            for(entry_index = 0; entry_index < port_configuration->vlan_aggregation_tables[table_index].number_of_aggregation_entries; entry_index++)
            {
                port_configuration->vlan_aggregation_tables[table_index].vlan_aggregation_list[entry_index] = management_object_vlan[0].entry.vlan_aggregation_tables[table_index].vlan_aggregation_list[entry_index];
            }
        }
    }
    else
    {
        port_configuration->number_of_aggregation_tables = 0;
    }
	

	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_reset_onu"

PON_STATUS PON_CTC_STACK_reset_onu ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id )
{
    short int								result,comparser_result;
    unsigned short							sent_length=ETHERNET_MTU;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ACTION;
	unsigned short							expected_leaf	=  CTC_EX_ACTION_RESET_ONU;
	unsigned short							max_length=ETHERNET_MTU,total_length=0;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


	total_length += sent_length;
	sent_length = max_length - total_length;


	result = Send_receive_vendor_extension_message_with_timeout 
						( olt_id, onu_id, sent_oam_pdu_data, total_length, 0/*without waiting*/, 
						  0/*expected_opcode- do not waiting for response*/, 0/*expected_code- do not waiting for response*/,
						  NULL/*received_oam_pdu_data*/, NULL/*&received_length*/, ETHERNET_MTU,NULL);


    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_classification_and_marking_multiple_management_object"

PON_STATUS PON_CTC_STACK_set_classification_and_marking_multiple_management_object ( 
           const PON_olt_id_t							            olt_id, 
           const PON_onu_id_t							            onu_id,
		   const CTC_STACK_classification_rule_mode_t               mode,
		   const unsigned char							            number_of_entries,
		   const CTC_STACK_classification_management_object_t		classification_management_object )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_CLASSIFICATION_N_MARKING;
	unsigned char								received_num_of_entries=number_of_entries;
	unsigned char								ctc_ret_val,i, precedence_index, entry_index;
	CTC_STACK_classification_and_marking_t		send_classification_n_marking;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char								local_number_of_entries = 0;
	bool										at_least_one_valid_rule = FALSE;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC


	PON_HANDLE_ERROR_CHECK_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, mode,mode,CTC_CLASSIFICATION_DELETE_RULE,CTC_CLASSIFICATION_ADD_RULE)  

	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for(i = 0; i < number_of_entries; i++)
		{
			for(precedence_index = 0; precedence_index < CTC_MAX_CLASS_RULES_COUNT ; precedence_index++)
			{
				if(classification_management_object[i].entry[precedence_index].valid == TRUE)  
				{
					at_least_one_valid_rule = TRUE;
					PON_HANDLE_ERROR_CHECK_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, precedence_index, precedence, CTC_CLASSIFICATION_AND_MARKING_MIN_PRECEDENCE,CTC_CLASSIFICATION_AND_MARKING_MAX_PRECEDENCE)  
					CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, classification_management_object[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
				}
			}
		}


		if(!at_least_one_valid_rule)
		{
			PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, olt_id, onu_id,
							"OLT %d ONU %d there are no valid rules to configured\n", olt_id, onu_id);

			return CTC_STACK_PARAMETER_ERROR;

		}
	}
       
	

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (classification_management_object[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			send_classification_n_marking.action = mode;

			for( precedence_index = 0 ; precedence_index < CTC_MAX_CLASS_RULES_COUNT; precedence_index++ )
			{
				send_classification_n_marking.rules[precedence_index].valid = classification_management_object[i].entry[precedence_index].valid;
				send_classification_n_marking.rules[precedence_index].queue_mapped = classification_management_object[i].entry[precedence_index].queue_mapped;
				send_classification_n_marking.rules[precedence_index].priority_mark = classification_management_object[i].entry[precedence_index].priority_mark;
				send_classification_n_marking.rules[precedence_index].num_of_entries = classification_management_object[i].entry[precedence_index].num_of_entries;


				for( entry_index = 0 ; entry_index < CTC_MAX_CLASS_RULE_PAIRS; entry_index++ )
				{
					send_classification_n_marking.rules[precedence_index].entries[entry_index].field_select = classification_management_object[i].entry[precedence_index].entries[entry_index].field_select;
					send_classification_n_marking.rules[precedence_index].entries[entry_index].validation_operator = classification_management_object[i].entry[precedence_index].entries[entry_index].validation_operator;
					send_classification_n_marking.rules[precedence_index].entries[entry_index].value.match_value = classification_management_object[i].entry[precedence_index].entries[entry_index].value.match_value;
					PON_MAC_ADDRESS_COPY(send_classification_n_marking.rules[precedence_index].entries[entry_index].value.mac_address, classification_management_object[i].entry[precedence_index].entries[entry_index].value.mac_address);
					memcpy(&(send_classification_n_marking.rules[precedence_index].entries[entry_index].value.ipv6_match_value), &(classification_management_object[i].entry[precedence_index].entries[entry_index].value.ipv6_match_value), sizeof(PON_ipv6_addr_t));
				}
			}


			comparser_result = (short int)CTC_COMPOSE_classification_n_marking
									( 1, &send_classification_n_marking, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length, onu_capabilities_3_db.onu[olt_id][onu_id].onu_ipv6_aware);
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (classification_management_object[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
            total_length += sent_length;
			sent_length = max_length - total_length;


			send_classification_n_marking.action = mode;

			for( precedence_index = 0 ; precedence_index < CTC_MAX_CLASS_RULES_COUNT; precedence_index++ )
			{
				send_classification_n_marking.rules[precedence_index].valid = classification_management_object[0].entry[precedence_index].valid;
				send_classification_n_marking.rules[precedence_index].queue_mapped = classification_management_object[0].entry[precedence_index].queue_mapped;
				send_classification_n_marking.rules[precedence_index].priority_mark = classification_management_object[0].entry[precedence_index].priority_mark;
				send_classification_n_marking.rules[precedence_index].num_of_entries = classification_management_object[0].entry[precedence_index].num_of_entries;


				for( entry_index = 0 ; entry_index < CTC_MAX_CLASS_RULE_PAIRS; entry_index++ )
				{
					send_classification_n_marking.rules[precedence_index].entries[entry_index].field_select = classification_management_object[0].entry[precedence_index].entries[entry_index].field_select;
					send_classification_n_marking.rules[precedence_index].entries[entry_index].validation_operator = classification_management_object[0].entry[precedence_index].entries[entry_index].validation_operator;
					send_classification_n_marking.rules[precedence_index].entries[entry_index].value.match_value = classification_management_object[0].entry[precedence_index].entries[entry_index].value.match_value;
					PON_MAC_ADDRESS_COPY(send_classification_n_marking.rules[precedence_index].entries[entry_index].value.mac_address, classification_management_object[0].entry[precedence_index].entries[entry_index].value.mac_address);
					memcpy(&(send_classification_n_marking.rules[precedence_index].entries[entry_index].value.ipv6_match_value), &(classification_management_object[0].entry[precedence_index].entries[entry_index].value.ipv6_match_value), sizeof(PON_ipv6_addr_t));
				}
			}


			comparser_result = (short int)CTC_COMPOSE_classification_n_marking
									( 1, &send_classification_n_marking, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length, onu_capabilities_3_db.onu[olt_id][onu_id].onu_ipv6_aware );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}

	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		received_total_length += received_left_length;

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, classification_management_object[i].management_object_index.port_number)
		}


		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        				  expected_leaf, &ctc_ret_val , &received_left_length );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
	}
	

	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_classification_and_marking_multiple_management_object"

PON_STATUS PON_CTC_STACK_get_classification_and_marking_multiple_management_object ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char					             *number_of_entries,
		   CTC_STACK_classification_management_object_t   classification_management_object )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_CLASSIFICATION_N_MARKING;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val,i, rule_index, entry_index;
	CTC_STACK_classification_and_marking_t		received_classification_n_marking;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, classification_management_object[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (classification_management_object[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (classification_management_object[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, classification_management_object[i].management_object_index.port_number)


			received_total_length += received_left_length;


			memset(&received_classification_n_marking, 0, sizeof(CTC_STACK_classification_and_marking_t));

			comparser_result = (short int)CTC_PARSE_classification_n_marking
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &received_classification_n_marking, &ctc_ret_val, &received_left_length, onu_capabilities_3_db.onu[olt_id][onu_id].onu_ipv6_aware  );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;

			
			for( rule_index = 0; rule_index < CTC_MAX_CLASS_RULES_COUNT; rule_index++)
			{
				if(received_classification_n_marking.rules[rule_index].valid == TRUE)
				{
					classification_management_object[i].entry[rule_index].valid			= received_classification_n_marking.rules[rule_index].valid;
					classification_management_object[i].entry[rule_index].queue_mapped	= received_classification_n_marking.rules[rule_index].queue_mapped;
					classification_management_object[i].entry[rule_index].priority_mark = received_classification_n_marking.rules[rule_index].priority_mark;
					classification_management_object[i].entry[rule_index].num_of_entries = received_classification_n_marking.rules[rule_index].num_of_entries;


					for(entry_index = 0; entry_index < classification_management_object[i].entry[rule_index].num_of_entries; entry_index++)
					{
						classification_management_object[i].entry[rule_index].entries[entry_index].field_select = received_classification_n_marking.rules[rule_index].entries[entry_index].field_select;
						
						if((classification_management_object[i].entry[rule_index].entries[entry_index].field_select == CTC_FIELD_SEL_DA_MAC) || 
						   (classification_management_object[i].entry[rule_index].entries[entry_index].field_select == CTC_FIELD_SEL_SA_MAC))
						{
							PON_MAC_ADDRESS_COPY(classification_management_object[i].entry[rule_index].entries[entry_index].value.mac_address, received_classification_n_marking.rules[rule_index].entries[entry_index].value.mac_address);
						}
						else
						{
							if((classification_management_object[i].entry[rule_index].entries[entry_index].field_select != CTC_FIELD_SEL_DEST_IPV6) && 
							   (classification_management_object[i].entry[rule_index].entries[entry_index].field_select != CTC_FIELD_SEL_SRC_IPV6))
							{
								classification_management_object[i].entry[rule_index].entries[entry_index].value.match_value = received_classification_n_marking.rules[rule_index].entries[entry_index].value.match_value;
							}

							if((classification_management_object[i].entry[rule_index].entries[entry_index].field_select == CTC_FIELD_SEL_DEST_IPV6) || 
						   		(classification_management_object[i].entry[rule_index].entries[entry_index].field_select == CTC_FIELD_SEL_SRC_IPV6) || 
						   		(classification_management_object[i].entry[rule_index].entries[entry_index].field_select == CTC_FIELD_SEL_DEST_IPV6_PREFIX) || 
						   		(classification_management_object[i].entry[rule_index].entries[entry_index].field_select == CTC_FIELD_SEL_SRC_IPV6_PREFIX))
							{
								memcpy(&(classification_management_object[i].entry[rule_index].entries[entry_index].value.ipv6_match_value), &(received_classification_n_marking.rules[rule_index].entries[entry_index].value.ipv6_match_value), sizeof(PON_ipv6_addr_t));
							}
						}

						classification_management_object[i].entry[rule_index].entries[entry_index].validation_operator = received_classification_n_marking.rules[rule_index].entries[entry_index].validation_operator;
					}
				}
			}
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
			received_total_length += received_left_length;

			memset(&received_classification_n_marking, 0, sizeof(CTC_STACK_classification_and_marking_t));

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_classification_n_marking
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &received_classification_n_marking, &ctc_ret_val, &received_left_length, onu_capabilities_3_db.onu[olt_id][onu_id].onu_ipv6_aware  );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
			
			classification_management_object[i].management_object_index.port_number	 = received_management_object.index.port_number;
			classification_management_object[i].management_object_index.frame_number = received_management_object.index.frame_number;
			classification_management_object[i].management_object_index.slot_number  = received_management_object.index.slot_number;
			classification_management_object[i].management_object_index.port_type    = received_management_object.index.port_type;
			
            for( rule_index = 0; rule_index < CTC_MAX_CLASS_RULES_COUNT; rule_index++)
			{
				if(received_classification_n_marking.rules[rule_index].valid == TRUE)
				{
					classification_management_object[i].entry[rule_index].valid			= received_classification_n_marking.rules[rule_index].valid;
					classification_management_object[i].entry[rule_index].queue_mapped	= received_classification_n_marking.rules[rule_index].queue_mapped;
					classification_management_object[i].entry[rule_index].priority_mark = received_classification_n_marking.rules[rule_index].priority_mark;
					classification_management_object[i].entry[rule_index].num_of_entries = received_classification_n_marking.rules[rule_index].num_of_entries;


					for(entry_index = 0; entry_index < classification_management_object[i].entry[rule_index].num_of_entries; entry_index++)
					{
						classification_management_object[i].entry[rule_index].entries[entry_index].field_select = received_classification_n_marking.rules[rule_index].entries[entry_index].field_select;
						
						if((classification_management_object[i].entry[rule_index].entries[entry_index].field_select == CTC_FIELD_SEL_DA_MAC) || 
						   (classification_management_object[i].entry[rule_index].entries[entry_index].field_select == CTC_FIELD_SEL_SA_MAC))
						{
							PON_MAC_ADDRESS_COPY(classification_management_object[i].entry[rule_index].entries[entry_index].value.mac_address, received_classification_n_marking.rules[rule_index].entries[entry_index].value.mac_address);
						}
						else
						{
							if((classification_management_object[i].entry[rule_index].entries[entry_index].field_select != CTC_FIELD_SEL_DEST_IPV6) && 
							   (classification_management_object[i].entry[rule_index].entries[entry_index].field_select != CTC_FIELD_SEL_SRC_IPV6))
							{
								classification_management_object[i].entry[rule_index].entries[entry_index].value.match_value = received_classification_n_marking.rules[rule_index].entries[entry_index].value.match_value;
							}

							if((classification_management_object[i].entry[rule_index].entries[entry_index].field_select == CTC_FIELD_SEL_DEST_IPV6) || 
						   		(classification_management_object[i].entry[rule_index].entries[entry_index].field_select == CTC_FIELD_SEL_SRC_IPV6) || 
						   		(classification_management_object[i].entry[rule_index].entries[entry_index].field_select == CTC_FIELD_SEL_DEST_IPV6_PREFIX) || 
						   		(classification_management_object[i].entry[rule_index].entries[entry_index].field_select == CTC_FIELD_SEL_SRC_IPV6_PREFIX))
							{
								memcpy(&(classification_management_object[i].entry[rule_index].entries[entry_index].value.ipv6_match_value), &(received_classification_n_marking.rules[rule_index].entries[entry_index].value.ipv6_match_value), sizeof(PON_ipv6_addr_t));
							}
						}

						classification_management_object[i].entry[rule_index].entries[entry_index].validation_operator = received_classification_n_marking.rules[rule_index].entries[entry_index].validation_operator;
					}
				}
			}
			
			i++;
		}

		(*number_of_entries) = i;
	}
    
	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_classification_and_marking_all_management_object"

PON_STATUS PON_CTC_STACK_get_classification_and_marking_all_management_object ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char				              	 *number_of_entries,
		   CTC_STACK_classification_management_object_t   management_object_info )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_classification_and_marking_multiple_management_object(olt_id, onu_id, number_of_entries, management_object_info));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_classification_and_marking"

PON_STATUS PON_CTC_STACK_set_classification_and_marking_management_object ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_management_object_index_t          management_object_index,
		   const CTC_STACK_classification_rule_mode_t   mode,
		   const CTC_STACK_classification_rules_t		classification_and_marking)
{
    short int								      result=CTC_STACK_EXIT_OK;
	unsigned char						          number_of_entries;
    unsigned char                                 precedence_index, entry_index;  
    CTC_STACK_classification_management_object_t  *management_object_info;

    /* modified by xieshl 20130219, CTC 低版本ONU或pend ONU可能会丢内存，下同 */
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);
    
    /*CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC*/
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    if ( !ctc_stack_general_information.ctc_stack_is_init )
    { 
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"ERROR: CTC_STACK package was not initialized. please call CTC_STACK_init first.\n" );
        /*PON_FREE_FUNCTION(management_object_info);*/
        return CTC_STACK_EXIT_ERROR; 
    }
    if ( discovery_db.state[olt_id][onu_id] != CTC_DISCOVERY_STATE_COMPLETE)
    { 
		PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, 
						"ERROR: OLT: %d ONU :%d OAM extended discovery state is not complete, can not apply with this command\n", olt_id, onu_id);
        /*PON_FREE_FUNCTION(management_object_info);*/
        return CTC_STACK_ONU_NOT_AVAILABLE; 
    }

   	management_object_info = (CTC_STACK_classification_management_object_t *)PON_MALLOC_FUNCTION(sizeof(CTC_STACK_classification_management_object_t));
	if( management_object_info == NULL )
		return CTC_STACK_MEMORY_ERROR;
	memset(management_object_info,0,sizeof(CTC_STACK_classification_management_object_t) );


	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    management_object_info[0]->management_object_index.port_number      = management_object_index.port_number;
    management_object_info[0]->management_object_index.slot_number      = management_object_index.slot_number;
    management_object_info[0]->management_object_index.frame_number     = management_object_index.frame_number;
    management_object_info[0]->management_object_index.port_type        = management_object_index.port_type;

   

    for( precedence_index = 0 ; precedence_index < CTC_MAX_CLASS_RULES_COUNT; precedence_index++ )
    {
        management_object_info[0]->entry[precedence_index].valid = classification_and_marking[precedence_index].valid;
        management_object_info[0]->entry[precedence_index].queue_mapped = classification_and_marking[precedence_index].queue_mapped;
        management_object_info[0]->entry[precedence_index].priority_mark = classification_and_marking[precedence_index].priority_mark;
        management_object_info[0]->entry[precedence_index].num_of_entries = classification_and_marking[precedence_index].num_of_entries;
        
        
        for( entry_index = 0 ; entry_index < CTC_MAX_CLASS_RULE_PAIRS; entry_index++ )
        {
            management_object_info[0]->entry[precedence_index].entries[entry_index].field_select = classification_and_marking[precedence_index].entries[entry_index].field_select;
            management_object_info[0]->entry[precedence_index].entries[entry_index].validation_operator = classification_and_marking[precedence_index].entries[entry_index].validation_operator;
            management_object_info[0]->entry[precedence_index].entries[entry_index].value.match_value = classification_and_marking[precedence_index].entries[entry_index].value.match_value;
            PON_MAC_ADDRESS_COPY(management_object_info[0]->entry[precedence_index].entries[entry_index].value.mac_address, classification_and_marking[precedence_index].entries[entry_index].value.mac_address);
			memcpy(&(management_object_info[0]->entry[precedence_index].entries[entry_index].value.ipv6_match_value), &(classification_and_marking[precedence_index].entries[entry_index].value.ipv6_match_value), sizeof(PON_ipv6_addr_t));
        }
    }


	result = PON_CTC_STACK_set_classification_and_marking_multiple_management_object(olt_id, onu_id, mode, number_of_entries, *management_object_info);
    if ( result != CTC_STACK_EXIT_OK ) 
    { 
        PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id, 
            "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result);
        PON_FREE_FUNCTION(management_object_info);
        return result;
    }
    
    PON_FREE_FUNCTION(management_object_info);
	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_classification_and_marking_management_object"

PON_STATUS PON_CTC_STACK_get_classification_and_marking_management_object ( 
           const PON_olt_id_t				   olt_id, 
           const PON_onu_id_t				   onu_id,
		   const CTC_management_object_index_t management_object_index,
		   CTC_STACK_classification_rules_t    classification_and_marking)
{
    short int								            result;
    unsigned char                                       i, precedence_index, entry_index;  
	unsigned char							            number_of_entries = 1;
    CTC_STACK_classification_management_object_t        *management_object_info;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    /*CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC*/
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    if ( !ctc_stack_general_information.ctc_stack_is_init )
    { 
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"ERROR: CTC_STACK package was not initialized. please call CTC_STACK_init first.\n" );
        /*PON_FREE_FUNCTION(management_object_info);*/
        return CTC_STACK_EXIT_ERROR; 
    }
    if ( discovery_db.state[olt_id][onu_id] != CTC_DISCOVERY_STATE_COMPLETE)
    { 
		PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, 
						"ERROR: OLT: %d ONU :%d OAM extended discovery state is not complete, can not apply with this command\n", olt_id, onu_id);
        /*PON_FREE_FUNCTION(management_object_info);*/
        return CTC_STACK_ONU_NOT_AVAILABLE; 
    }

   	management_object_info = (CTC_STACK_classification_management_object_t *)PON_MALLOC_FUNCTION(sizeof(CTC_STACK_classification_management_object_t));
	if( management_object_info == NULL )
		return CTC_STACK_MEMORY_ERROR;
	memset(management_object_info,0,sizeof(CTC_STACK_classification_management_object_t) );


    management_object_info[0]->management_object_index.port_number      = management_object_index.port_number;
    management_object_info[0]->management_object_index.slot_number      = management_object_index.slot_number;
    management_object_info[0]->management_object_index.frame_number     = management_object_index.frame_number;
    management_object_info[0]->management_object_index.port_type        = management_object_index.port_type;


    result = PON_CTC_STACK_get_classification_and_marking_multiple_management_object(olt_id, onu_id, &number_of_entries, *management_object_info);
    if ( result != CTC_STACK_EXIT_OK ) 
    { 
        PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id, 
            "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result);
        PON_FREE_FUNCTION(management_object_info);
        return result;
    }


    for( i = 0; i < number_of_entries; i++)
    {
        for( precedence_index = 0 ; precedence_index < CTC_MAX_CLASS_RULES_COUNT; precedence_index++ )
        {
            classification_and_marking[precedence_index].valid = management_object_info[i]->entry[precedence_index].valid;
            classification_and_marking[precedence_index].queue_mapped = management_object_info[i]->entry[precedence_index].queue_mapped;
            classification_and_marking[precedence_index].priority_mark = management_object_info[i]->entry[precedence_index].priority_mark;
            classification_and_marking[precedence_index].num_of_entries = management_object_info[i]->entry[precedence_index].num_of_entries;
            
            
            for( entry_index = 0 ; entry_index < CTC_MAX_CLASS_RULE_PAIRS; entry_index++ )
            {
                classification_and_marking[precedence_index].entries[entry_index].field_select = management_object_info[i]->entry[precedence_index].entries[entry_index].field_select;
                classification_and_marking[precedence_index].entries[entry_index].validation_operator = management_object_info[i]->entry[precedence_index].entries[entry_index].validation_operator;
                classification_and_marking[precedence_index].entries[entry_index].value.match_value = management_object_info[i]->entry[precedence_index].entries[entry_index].value.match_value;
                PON_MAC_ADDRESS_COPY(classification_and_marking[precedence_index].entries[entry_index].value.mac_address, management_object_info[i]->entry[precedence_index].entries[entry_index].value.mac_address);
				memcpy(&(classification_and_marking[precedence_index].entries[entry_index].value.ipv6_match_value), &(management_object_info[i]->entry[precedence_index].entries[entry_index].value.ipv6_match_value), sizeof(PON_ipv6_addr_t));
            }
        }
    }
    
    PON_FREE_FUNCTION(management_object_info);
	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_classification_and_marking_all_port"

PON_STATUS PON_CTC_STACK_get_classification_and_marking_all_port ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   unsigned char					 *number_of_entries,
		   CTC_STACK_classification_ports_t   ports_info )
{
    short int								            result;
    unsigned char                                       i, precedence_index, entry_index;  
    CTC_STACK_classification_management_object_t        *management_object_info;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    /*CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC*/

    if ( !ctc_stack_general_information.ctc_stack_is_init )
    { 
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"ERROR: CTC_STACK package was not initialized. please call CTC_STACK_init first.\n" );
        /*PON_FREE_FUNCTION(management_object_info);*/
        return CTC_STACK_EXIT_ERROR; 
    }
    if ( discovery_db.state[olt_id][onu_id] != CTC_DISCOVERY_STATE_COMPLETE)
    { 
		PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, 
						"ERROR: OLT: %d ONU :%d OAM extended discovery state is not complete, can not apply with this command\n", olt_id, onu_id);
        /*PON_FREE_FUNCTION(management_object_info);*/
        return CTC_STACK_ONU_NOT_AVAILABLE; 
    }

   	management_object_info = (CTC_STACK_classification_management_object_t *)PON_MALLOC_FUNCTION(sizeof(CTC_STACK_classification_management_object_t));
	if( management_object_info == NULL )
		return CTC_STACK_MEMORY_ERROR;
	memset(management_object_info,0,sizeof(CTC_STACK_classification_management_object_t) );

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_info[0]->management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_info[0]->management_object_index.slot_number      = 0x00;
    management_object_info[0]->management_object_index.frame_number     = 0x00;
    management_object_info[0]->management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_classification_and_marking_multiple_management_object(olt_id, onu_id, number_of_entries, *management_object_info);
    if ( result != CTC_STACK_EXIT_OK ) 
    { 
        PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id, 
            "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result);
        PON_FREE_FUNCTION(management_object_info);
        return result;
    }



    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_info[i].port_number = (unsigned char)((*management_object_info)[i].management_object_index.port_number);
        
        for( precedence_index = 0 ; precedence_index < CTC_MAX_CLASS_RULES_COUNT; precedence_index++ )
        {
            ports_info[i].entry[precedence_index].valid = (*management_object_info)[i].entry[precedence_index].valid;
            ports_info[i].entry[precedence_index].queue_mapped = (*management_object_info)[i].entry[precedence_index].queue_mapped;
            ports_info[i].entry[precedence_index].priority_mark = (*management_object_info)[i].entry[precedence_index].priority_mark;
            ports_info[i].entry[precedence_index].num_of_entries = (*management_object_info)[i].entry[precedence_index].num_of_entries;
            
            
            for( entry_index = 0 ; entry_index < CTC_MAX_CLASS_RULE_PAIRS; entry_index++ )
            {
                ports_info[i].entry[precedence_index].entries[entry_index].field_select = (*management_object_info)[i].entry[precedence_index].entries[entry_index].field_select;
                ports_info[i].entry[precedence_index].entries[entry_index].validation_operator = (*management_object_info)[i].entry[precedence_index].entries[entry_index].validation_operator;
                ports_info[i].entry[precedence_index].entries[entry_index].value.match_value = (*management_object_info)[i].entry[precedence_index].entries[entry_index].value.match_value;
                PON_MAC_ADDRESS_COPY(ports_info[i].entry[precedence_index].entries[entry_index].value.mac_address, (*management_object_info)[i].entry[precedence_index].entries[entry_index].value.mac_address);
                memcpy(&(ports_info[i].entry[precedence_index].entries[entry_index].value.ipv6_match_value), &((*management_object_info)[i].entry[precedence_index].entries[entry_index].value.ipv6_match_value), sizeof(PON_ipv6_addr_t));				
            }
        }
    }
    
    PON_FREE_FUNCTION(management_object_info);
	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_classification_and_marking"

PON_STATUS PON_CTC_STACK_set_classification_and_marking ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const unsigned char							port_number,
		   const CTC_STACK_classification_rule_mode_t   mode,
		   const CTC_STACK_classification_rules_t		classification_and_marking)
{
    short int								      result=CTC_STACK_EXIT_OK;
	unsigned char						          number_of_entries;
    unsigned char                                 precedence_index, entry_index;  
    CTC_STACK_classification_management_object_t  *management_object_info;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    /*CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC*/

    if ( !ctc_stack_general_information.ctc_stack_is_init )
    { 
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"ERROR: CTC_STACK package was not initialized. please call CTC_STACK_init first.\n" );
        /*PON_FREE_FUNCTION(management_object_info);*/
        return CTC_STACK_EXIT_ERROR; 
    }
    if ( discovery_db.state[olt_id][onu_id] != CTC_DISCOVERY_STATE_COMPLETE)
    { 
		PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, 
						"ERROR: OLT: %d ONU :%d OAM extended discovery state is not complete, can not apply with this command\n", olt_id, onu_id);
        /*PON_FREE_FUNCTION(management_object_info);*/
        return CTC_STACK_ONU_NOT_AVAILABLE; 
    }

  	management_object_info = (CTC_STACK_classification_management_object_t *)PON_MALLOC_FUNCTION(sizeof(CTC_STACK_classification_management_object_t));
	if( management_object_info == NULL )
		return CTC_STACK_MEMORY_ERROR;
	memset(management_object_info,0,sizeof(CTC_STACK_classification_management_object_t) );


	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_info[0]->management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_info[0]->management_object_index.port_number  = port_number;
    }


    management_object_info[0]->management_object_index.slot_number      = 0x00;
    management_object_info[0]->management_object_index.frame_number     = 0x00;
    management_object_info[0]->management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

   

    for( precedence_index = 0 ; precedence_index < CTC_MAX_CLASS_RULES_COUNT; precedence_index++ )
    {
        management_object_info[0]->entry[precedence_index].valid = classification_and_marking[precedence_index].valid;
        management_object_info[0]->entry[precedence_index].queue_mapped = classification_and_marking[precedence_index].queue_mapped;
        management_object_info[0]->entry[precedence_index].priority_mark = classification_and_marking[precedence_index].priority_mark;
        management_object_info[0]->entry[precedence_index].num_of_entries = classification_and_marking[precedence_index].num_of_entries;
        
        
        for( entry_index = 0 ; entry_index < CTC_MAX_CLASS_RULE_PAIRS; entry_index++ )
        {
            management_object_info[0]->entry[precedence_index].entries[entry_index].field_select = classification_and_marking[precedence_index].entries[entry_index].field_select;
            management_object_info[0]->entry[precedence_index].entries[entry_index].validation_operator = classification_and_marking[precedence_index].entries[entry_index].validation_operator;
            management_object_info[0]->entry[precedence_index].entries[entry_index].value.match_value = classification_and_marking[precedence_index].entries[entry_index].value.match_value;
            PON_MAC_ADDRESS_COPY(management_object_info[0]->entry[precedence_index].entries[entry_index].value.mac_address, classification_and_marking[precedence_index].entries[entry_index].value.mac_address);
			memcpy(&(management_object_info[0]->entry[precedence_index].entries[entry_index].value.ipv6_match_value), &(classification_and_marking[precedence_index].entries[entry_index].value.ipv6_match_value), sizeof(PON_ipv6_addr_t));
        }
    }


	result = PON_CTC_STACK_set_classification_and_marking_multiple_management_object(olt_id, onu_id, mode, number_of_entries, *management_object_info);
    if ( result != CTC_STACK_EXIT_OK ) 
    { 
        PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id, 
            "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result);
        PON_FREE_FUNCTION(management_object_info);
        return result;
    }
    
    PON_FREE_FUNCTION(management_object_info);
	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_classification_and_marking"

PON_STATUS PON_CTC_STACK_get_classification_and_marking ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   const unsigned char				  port_number,
		   CTC_STACK_classification_rules_t   classification_and_marking)
{
    short int								            result;
    unsigned char                                       i, precedence_index, entry_index;  
	unsigned char							            number_of_entries = 1;
    CTC_STACK_classification_management_object_t        *management_object_info;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    /*CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC*/

    if ( !ctc_stack_general_information.ctc_stack_is_init )
    { 
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"ERROR: CTC_STACK package was not initialized. please call CTC_STACK_init first.\n" );
        /*PON_FREE_FUNCTION(management_object_info);*/
        return CTC_STACK_EXIT_ERROR; 
    }
    if ( discovery_db.state[olt_id][onu_id] != CTC_DISCOVERY_STATE_COMPLETE)
    { 
		PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, 
						"ERROR: OLT: %d ONU :%d OAM extended discovery state is not complete, can not apply with this command\n", olt_id, onu_id);
        /*PON_FREE_FUNCTION(management_object_info);*/
        return CTC_STACK_ONU_NOT_AVAILABLE; 
    }

   	management_object_info = (CTC_STACK_classification_management_object_t *)PON_MALLOC_FUNCTION(sizeof(CTC_STACK_classification_management_object_t));
	if( management_object_info == NULL )
		return CTC_STACK_MEMORY_ERROR;
	memset(management_object_info,0,sizeof(CTC_STACK_classification_management_object_t) );

    management_object_info[0]->management_object_index.port_number      = port_number;
    management_object_info[0]->management_object_index.slot_number      = 0x00;
    management_object_info[0]->management_object_index.frame_number     = 0x00;
    management_object_info[0]->management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_classification_and_marking_multiple_management_object(olt_id, onu_id, &number_of_entries, *management_object_info);
    if ( result != CTC_STACK_EXIT_OK ) 
    { 
        PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id, 
            "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result);
        PON_FREE_FUNCTION(management_object_info);
        return result;
    }

    for( i = 0; i < number_of_entries; i++)
    {
        for( precedence_index = 0 ; precedence_index < CTC_MAX_CLASS_RULES_COUNT; precedence_index++ )
        {
            classification_and_marking[precedence_index].valid = management_object_info[i]->entry[precedence_index].valid;
            classification_and_marking[precedence_index].queue_mapped = management_object_info[i]->entry[precedence_index].queue_mapped;
            classification_and_marking[precedence_index].priority_mark = management_object_info[i]->entry[precedence_index].priority_mark;
            classification_and_marking[precedence_index].num_of_entries = management_object_info[i]->entry[precedence_index].num_of_entries;
            
            
            for( entry_index = 0 ; entry_index < CTC_MAX_CLASS_RULE_PAIRS; entry_index++ )
            {
                classification_and_marking[precedence_index].entries[entry_index].field_select = management_object_info[i]->entry[precedence_index].entries[entry_index].field_select;
                classification_and_marking[precedence_index].entries[entry_index].validation_operator = management_object_info[i]->entry[precedence_index].entries[entry_index].validation_operator;
                classification_and_marking[precedence_index].entries[entry_index].value.match_value = management_object_info[i]->entry[precedence_index].entries[entry_index].value.match_value;
                PON_MAC_ADDRESS_COPY(classification_and_marking[precedence_index].entries[entry_index].value.mac_address, management_object_info[i]->entry[precedence_index].entries[entry_index].value.mac_address);
				memcpy(&(classification_and_marking[precedence_index].entries[entry_index].value.ipv6_match_value), &(management_object_info[i]->entry[precedence_index].entries[entry_index].value.ipv6_match_value), sizeof(PON_ipv6_addr_t));
            }
        }
    }
    
    PON_FREE_FUNCTION(management_object_info);
	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_delete_classification_and_marking_list_multiple_management_object"

PON_STATUS PON_CTC_STACK_delete_classification_and_marking_list_multiple_management_object ( 
           const PON_olt_id_t	          olt_id, 
           const PON_onu_id_t	          onu_id,
		   const unsigned char	          number_of_entries,
		   CTC_management_object_index_t  management_object_index )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_CLASSIFICATION_N_MARKING;
	unsigned char								received_num_of_entries=number_of_entries;
	unsigned char								ctc_ret_val,i;
	CTC_STACK_classification_and_marking_t		send_classification_n_marking;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char								local_number_of_entries = 0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    if(number_of_entries != CTC_STACK_ALL_PORTS)
    {
        CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
    }

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, management_object_index);
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			send_classification_n_marking.action = CTC_CLASS_ACTION_DELETE_LIST;


			comparser_result = (short int)CTC_COMPOSE_classification_n_marking
									( 1, &send_classification_n_marking, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length, onu_capabilities_3_db.onu[olt_id][onu_id].onu_ipv6_aware );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, management_object_index);

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			send_classification_n_marking.action = CTC_CLASS_ACTION_DELETE_LIST;

			comparser_result = (short int)CTC_COMPOSE_classification_n_marking
									( 1, &send_classification_n_marking, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length, onu_capabilities_3_db.onu[olt_id][onu_id].onu_ipv6_aware );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}

	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		received_total_length += received_left_length;

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_index.port_number)
		}


		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        				  expected_leaf, &ctc_ret_val , &received_left_length );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
	}
	
	return result;       
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_delete_classification_and_marking_management_object_list"

PON_STATUS PON_CTC_STACK_delete_classification_and_marking_management_object_list ( 
           const PON_olt_id_t	               olt_id, 
           const PON_onu_id_t                  onu_id,
		   const CTC_management_object_index_t management_object_index)
{
	short int								result=CTC_STACK_EXIT_OK;
	unsigned char							number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
		number_of_entries = CTC_STACK_ALL_PORTS;
	else
		number_of_entries = 1;

	return PON_CTC_STACK_delete_classification_and_marking_list_multiple_management_object(olt_id, onu_id, number_of_entries, management_object_index);    

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_delete_classification_and_marking_list"

PON_STATUS PON_CTC_STACK_delete_classification_and_marking_list ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t    onu_id,
		   const unsigned char   port_number)
{
	short int								result=CTC_STACK_EXIT_OK;
	CTC_management_object_index_t           management_object_index;
	unsigned char							number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	management_object_index.port_number      = port_number;
    management_object_index.slot_number      = 0x00;
    management_object_index.frame_number     = 0x00;
    management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


	if(port_number == CTC_STACK_ALL_PORTS)
		number_of_entries = CTC_STACK_ALL_PORTS;
	else
		number_of_entries = 1;

	return PON_CTC_STACK_delete_classification_and_marking_list_multiple_management_object(olt_id, onu_id, number_of_entries, management_object_index);    

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multiple_management_object_multicast_vlan"

PON_STATUS PON_CTC_STACK_set_multiple_management_object_multicast_vlan ( 
           const PON_olt_id_t						            olt_id, 
           const PON_onu_id_t						            onu_id,
		   const unsigned char					                number_of_entries,
		   const CTC_STACK_multicast_vlan_management_object_t   multicast_vlan )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char							send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf  =  CTC_EX_VAR_MULTICAST_VLAN_OPER;
	unsigned char							received_num_of_entries=1;
	unsigned char							ctc_ret_val,i;
	unsigned short							max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char							local_number_of_entries = 0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    if(number_of_entries != CTC_STACK_ALL_PORTS)
    {
        for(i = 0; i < number_of_entries; i++)
        {
            CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, multicast_vlan[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
        }
    }

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (multicast_vlan[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_multicast_vlan
                            ( 1/*number_of_entries*/, &multicast_vlan[i].entry, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
                              SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


			total_length += sent_length;
			sent_length = max_length - total_length;

		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (multicast_vlan[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_multicast_vlan
                            ( 1/*number_of_entries*/, &multicast_vlan[0].entry, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
                              SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


			total_length += sent_length;
			sent_length = max_length - total_length;
	}

	


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}


	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		received_total_length += received_left_length;

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, multicast_vlan[i].management_object_index.port_number)
		}


		comparser_result = (short int)CTC_PARSE_general_ack
								( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
         						  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
	}

	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multiple_management_object_multicast_vlan"

PON_STATUS PON_CTC_STACK_get_multiple_management_object_multicast_vlan ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char					             *number_of_entries,
		   CTC_STACK_multicast_vlan_management_object_t   multicast_vlan )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_MULTICAST_VLAN_OPER;
	unsigned char							received_num_of_entries=0;
	unsigned char							ctc_ret_val,i;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, multicast_vlan[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id]) 
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (multicast_vlan[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (multicast_vlan[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, multicast_vlan[i].management_object_index.port_number)

			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_multicast_vlan
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &multicast_vlan[i].entry, &ctc_ret_val,&received_left_length  );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			
			if(CTC_MAX_MULTICAST_VLAN_ENTRIES < multicast_vlan[i].entry.num_of_vlan_id)
			{
				
				PONLOG_ERROR_4( PONLOG_FUNC_GENERAL, olt_id, onu_id,
									"OLT %d ONU %d not enougth allocated memory (%d) while received number of entries %d\n",
									olt_id, onu_id, CTC_MAX_MULTICAST_VLAN_ENTRIES, multicast_vlan[i].entry.num_of_vlan_id);


				return CTC_STACK_MEMORY_ERROR;
			}
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			
            if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_multicast_vlan
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &multicast_vlan[i].entry, &ctc_ret_val ,&received_left_length );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;


			if(CTC_MAX_MULTICAST_VLAN_ENTRIES < multicast_vlan[i].entry.num_of_vlan_id)
			{
				
				PONLOG_ERROR_4( PONLOG_FUNC_GENERAL, olt_id, onu_id,
									"OLT %d ONU %d not enougth allocated memory (%d) while received number of entries %d\n",
									olt_id, onu_id, CTC_MAX_MULTICAST_VLAN_ENTRIES, multicast_vlan[i].entry.num_of_vlan_id);


				result = CTC_STACK_MEMORY_ERROR;
			}
			else
			{
				multicast_vlan[i].management_object_index.port_number  = received_management_object.index.port_number;
    			multicast_vlan[i].management_object_index.frame_number = received_management_object.index.frame_number;
    			multicast_vlan[i].management_object_index.slot_number  = received_management_object.index.slot_number;
    			multicast_vlan[i].management_object_index.port_type    = received_management_object.index.port_type;
			}			
			i++;
		}

		(*number_of_entries) = i;
	}

	return result;  
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_vlan_management_object"

PON_STATUS PON_CTC_STACK_set_multicast_vlan_management_object ( 
           const PON_olt_id_t				   olt_id, 
           const PON_onu_id_t				   onu_id,
		   const CTC_management_object_index_t management_object_index,
		   const CTC_STACK_multicast_vlan_t    multicast_vlan)
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_multicast_vlan_management_object_t  management_object_vlan;
	unsigned char						          i, number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    management_object_vlan[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_vlan[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_vlan[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_vlan[0].management_object_index.port_type        = management_object_index.port_type;

	management_object_vlan[0].entry.vlan_operation = multicast_vlan.vlan_operation;
	management_object_vlan[0].entry.num_of_vlan_id = multicast_vlan.num_of_vlan_id;

	for( i = 0; i < multicast_vlan.num_of_vlan_id; i++)
	{
		management_object_vlan[0].entry.vlan_id[i] = multicast_vlan.vlan_id[i];
	}


	result = PON_CTC_STACK_set_multiple_management_object_multicast_vlan(olt_id, onu_id, number_of_entries, management_object_vlan);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_vlan_all_management_object"

PON_STATUS PON_CTC_STACK_get_multicast_vlan_all_management_object ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char					             *number_of_entries,
		   CTC_STACK_multicast_vlan_management_object_t   management_object_info )
{
    short int								      result;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    (*number_of_entries) = CTC_STACK_ALL_PORTS;

    result = PON_CTC_STACK_get_multiple_management_object_multicast_vlan(olt_id, onu_id, number_of_entries, management_object_info);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_vlan_management_object"

PON_STATUS PON_CTC_STACK_get_multicast_vlan_management_object ( 
           const PON_olt_id_t		 	         olt_id, 
           const PON_onu_id_t			         onu_id,
		   const CTC_management_object_index_t   management_object_index,
		   CTC_STACK_multicast_vlan_t           *multicast_vlan)
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_multicast_vlan_management_object_t  management_object_vlan;
	unsigned char							      number_of_entries = 1, i;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_vlan[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_vlan[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_vlan[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_vlan[0].management_object_index.port_type        = management_object_index.port_type;


    result = PON_CTC_STACK_get_multiple_management_object_multicast_vlan(olt_id, onu_id, &number_of_entries, management_object_vlan);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	multicast_vlan->vlan_operation = management_object_vlan[0].entry.vlan_operation;
	multicast_vlan->num_of_vlan_id = management_object_vlan[0].entry.num_of_vlan_id;

	for( i = 0; i < multicast_vlan->num_of_vlan_id; i++)
	{
		multicast_vlan->vlan_id[i] = management_object_vlan[0].entry.vlan_id[i];
	}

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_vlan"

PON_STATUS PON_CTC_STACK_set_multicast_vlan ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   const unsigned char				  port_number,
		   const CTC_STACK_multicast_vlan_t   multicast_vlan)
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_multicast_vlan_management_object_t  management_object_vlan;
	unsigned char						          i, number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_vlan[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_vlan[0].management_object_index.port_number  = port_number;
    }


    management_object_vlan[0].management_object_index.slot_number      = 0x00;
    management_object_vlan[0].management_object_index.frame_number     = 0x00;
    management_object_vlan[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

	management_object_vlan[0].entry.vlan_operation = multicast_vlan.vlan_operation;
	management_object_vlan[0].entry.num_of_vlan_id = multicast_vlan.num_of_vlan_id;

	for( i = 0; i < multicast_vlan.num_of_vlan_id; i++)
	{
		management_object_vlan[0].entry.vlan_id[i] = multicast_vlan.vlan_id[i];
	}


	result = PON_CTC_STACK_set_multiple_management_object_multicast_vlan(olt_id, onu_id, number_of_entries, management_object_vlan);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_vlan_all_port"

PON_STATUS PON_CTC_STACK_get_multicast_vlan_all_port ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   unsigned char					 *number_of_entries,
		   CTC_STACK_multicast_vlan_ports_t   ports_info )
{
    short int								      result;
    unsigned char                                 i, j;   
	CTC_STACK_multicast_vlan_management_object_t  management_object_vlan;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_vlan[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_vlan[0].management_object_index.slot_number      = 0x00;
    management_object_vlan[0].management_object_index.frame_number     = 0x00;
    management_object_vlan[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_multiple_management_object_multicast_vlan(olt_id, onu_id, number_of_entries, management_object_vlan);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_info[i].port_number          = (unsigned char)(management_object_vlan[i].management_object_index.port_number);
        ports_info[i].entry.vlan_operation = management_object_vlan[i].entry.vlan_operation;
        ports_info[i].entry.num_of_vlan_id = management_object_vlan[i].entry.num_of_vlan_id;

        for( j = 0; j < ports_info[i].entry.num_of_vlan_id; j++)
        {
            ports_info[i].entry.vlan_id[j] = management_object_vlan[i].entry.vlan_id[j];
        }
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_vlan"

PON_STATUS PON_CTC_STACK_get_multicast_vlan ( 
           const PON_olt_id_t			olt_id, 
           const PON_onu_id_t			onu_id,
		   const unsigned char			port_number,
		   CTC_STACK_multicast_vlan_t  *multicast_vlan)
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_multicast_vlan_management_object_t  management_object_vlan;
	unsigned char							      number_of_entries = 1, i;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_vlan[0].management_object_index.port_number      = port_number;
    management_object_vlan[0].management_object_index.slot_number      = 0x00;
    management_object_vlan[0].management_object_index.frame_number     = 0x00;
    management_object_vlan[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_multiple_management_object_multicast_vlan(olt_id, onu_id, &number_of_entries, management_object_vlan);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	multicast_vlan->vlan_operation = management_object_vlan[0].entry.vlan_operation;
	multicast_vlan->num_of_vlan_id = management_object_vlan[0].entry.num_of_vlan_id;

	for( i = 0; i < multicast_vlan->num_of_vlan_id; i++)
	{
		multicast_vlan->vlan_id[i] = management_object_vlan[0].entry.vlan_id[i];
	}

	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_clear_multicast_vlan_management_object"

PON_STATUS PON_CTC_STACK_clear_multicast_vlan_management_object ( 
           const PON_olt_id_t                  olt_id, 
           const PON_onu_id_t	               onu_id,
		   const CTC_management_object_index_t management_object_index)
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char							send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf  =  CTC_EX_VAR_MULTICAST_VLAN_OPER;
	unsigned char							received_num_of_entries=1;
	unsigned char							ctc_ret_val;
	CTC_STACK_multicast_vlan_t				send_multicast_vlan = {0};
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;


	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, management_object_index);
            
	comparser_result = (short int)CTC_COMPOSE_management_object
							( sent_management_object, discovery_db.common_version[olt_id][onu_id],
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	send_multicast_vlan.vlan_operation = CTC_MULTICAST_VLAN_OPER_CLEAR;


	comparser_result = (short int)CTC_COMPOSE_multicast_vlan
                    ( 1/*number_of_entries*/, &send_multicast_vlan, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
                      SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


	total_length += sent_length;
	sent_length = max_length - total_length;
	

	
    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;



	comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
	{
        if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
        {
            CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
        }
        else
        {
            CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
        }
	}
	else
	{
		CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_index.port_number)
	}

	received_total_length += received_left_length;

	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;
	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_clear_multicast_vlan"

PON_STATUS PON_CTC_STACK_clear_multicast_vlan ( 
           const PON_olt_id_t    olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number )
{
    short int							result=CTC_STACK_EXIT_OK;
    CTC_management_object_index_t       management_object_index;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
        management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
        management_object_index.port_number  = port_number;
    }


    management_object_index.slot_number      = 0x00;
    management_object_index.frame_number     = 0x00;
    management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_clear_multicast_vlan_management_object(olt_id, onu_id, management_object_index);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_control"

PON_STATUS PON_CTC_STACK_set_multicast_control ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const CTC_STACK_multicast_control_t   multicast_control )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char							send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf  =  CTC_EX_VAR_MULTICAST_CONTROL;
	unsigned char							received_num_of_entries=1;
	unsigned char							ctc_ret_val;
	unsigned char							received_port_number=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_STACK_multicast_control_ipv6_ext_t   multicast_control_ipv6;
	unsigned char i;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(multicast_control.control_type==CTC_STACK_MULTICAST_CONTROL_TYPE_DIPV6_VID)
	{
		/*error, not supported control type*/
		PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id, \
					"failed in OLT %d ONU %d. control_type[%d] only valid in CTC_STACK_set_multicast_control_ipv6_ext()\n", olt_id, onu_id, multicast_control.control_type);
		return CTC_STACK_PARAMETER_ERROR;
	}

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	/*copy ipv4 parameters to ipv6 structure*/
	memset(&multicast_control_ipv6, 0, sizeof(CTC_STACK_multicast_control_ipv6_ext_t));
	multicast_control_ipv6.action = multicast_control.action;
	multicast_control_ipv6.control_type= multicast_control.control_type;
	for(i=0; i<multicast_control.num_of_entries; i++)
	{
		memcpy(&(multicast_control_ipv6.entries[i].ipv4_entry), &(multicast_control.entries[i]), sizeof(CTC_STACK_multicast_control_t));
		multicast_control_ipv6.entries[i].ip_flag = CTC_STACK_IPV4;
	}
	multicast_control_ipv6.num_of_entries = multicast_control.num_of_entries;
	
	comparser_result = (short int)CTC_COMPOSE_multicast_control
                    ( 1/*number_of_entries*/, &multicast_control_ipv6, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
                      SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


	total_length += sent_length;
	sent_length = max_length - total_length;
	

	
    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;
	
	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_control_ipv6_ext"

PON_STATUS PON_CTC_STACK_set_multicast_control_ipv6_ext ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const CTC_STACK_multicast_control_ipv6_ext_t   multicast_control_ipv6 )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char							send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf  =  CTC_EX_VAR_MULTICAST_CONTROL;
	unsigned char							received_num_of_entries=1;
	unsigned char							ctc_ret_val;
	unsigned char							received_port_number=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;


    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	comparser_result = (short int)CTC_COMPOSE_multicast_control
                    ( 1/*number_of_entries*/, &multicast_control_ipv6, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
                      SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


	total_length += sent_length;
	sent_length = max_length - total_length;
	

	
    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;
	
	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_get_multicast_control"
PON_STATUS PON_get_multicast_control ( 
           const PON_olt_id_t                   olt_id, 
           const PON_onu_id_t                   onu_id,
		   CTC_STACK_multicast_control_ipv6_ext_t        *multicast_control_ipv6,
		   unsigned short                       *entry_index,
		   bool                                 *more_frame)
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_MULTICAST_CONTROL;
	unsigned char							received_num_of_entries=0;
	unsigned char							ctc_ret_val,mac_address_index;
	CTC_STACK_multicast_control_ipv6_ext_t			received_multicast_control = {0};
	unsigned char							received_port_number = 0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0, i;



    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	multicast_control_ipv6->action = CTC_MULTICAST_CONTROL_ACTION_LIST;

/*
	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	*/

	comparser_result = (short int)CTC_COMPOSE_multicast_control
                    ( 1/*number_of_entries*/, multicast_control_ipv6, COMPARSER_CTC_STATUS_SUCCESS, TRUE/* is_get_frame*/,
                      SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


	total_length += sent_length;
	sent_length = max_length - total_length;
	



	
    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;



	comparser_result = (short int)CTC_PARSE_multicast_control
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              &received_multicast_control, &ctc_ret_val, &received_left_length, more_frame);

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;


	if(CTC_MAX_MULTICAST_MAC_ENTRIES < (*entry_index + received_multicast_control.num_of_entries))
	{
		
		PONLOG_ERROR_4( PONLOG_FUNC_GENERAL, olt_id, onu_id,
							"OLT %d ONU %d not enougth allocated memory (%d) while received number of entries %d\n",
							olt_id, onu_id, CTC_MAX_MULTICAST_MAC_ENTRIES, (*entry_index + received_multicast_control.num_of_entries));


		result = CTC_STACK_MEMORY_ERROR;
	}
	else
	{
		for(i = 0; i < received_multicast_control.num_of_entries; i++)
		{
			multicast_control_ipv6->entries[*entry_index].ipv4_entry.vid = received_multicast_control.entries[i].ipv4_entry.vid;
			
			multicast_control_ipv6->entries[*entry_index].ipv4_entry.user_id = received_multicast_control.entries[i].ipv4_entry.user_id;

			for( mac_address_index=0; mac_address_index < BYTES_IN_MAC_ADDRESS; mac_address_index++)
			{
				PON_MAC_ADDRESS_COPY(&(multicast_control_ipv6->entries[*entry_index].ipv4_entry.da[mac_address_index]), &(received_multicast_control.entries[i].ipv4_entry.da[mac_address_index]));
			}

			multicast_control_ipv6->entries[*entry_index].ipv4_entry.sa=received_multicast_control.entries[i].ipv4_entry.sa;
			memcpy(&(multicast_control_ipv6->entries[*entry_index].ipv6_gda), &(received_multicast_control.entries[i].ipv6_gda), sizeof(PON_ipv6_addr_t));
			memcpy(&(multicast_control_ipv6->entries[*entry_index].ipv6_sa), &(received_multicast_control.entries[i].ipv6_sa), sizeof(PON_ipv6_addr_t));			
			multicast_control_ipv6->entries[*entry_index].ip_flag = received_multicast_control.entries[i].ip_flag;

			(*entry_index)++;
		}

		multicast_control_ipv6->num_of_entries = (*entry_index);
		multicast_control_ipv6->control_type = received_multicast_control.control_type;
		multicast_control_ipv6->action = received_multicast_control.action;
	}
	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_control"
PON_STATUS PON_CTC_STACK_get_multicast_control ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   CTC_STACK_multicast_control_t	*multicast_control )
{
	PON_STATUS result;
	bool more_frame = TRUE;
	unsigned short entry_index = 0;
	CTC_STACK_multicast_control_ipv6_ext_t multicast_control_ipv6;
	unsigned char i;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	memset(&multicast_control_ipv6, 0, sizeof(CTC_STACK_multicast_control_ipv6_ext_t));
	
	while (more_frame)
	{
		result = PON_get_multicast_control(olt_id, onu_id, &multicast_control_ipv6, &entry_index, &more_frame);
		if (result != CTC_STACK_EXIT_OK)
			return (result); /* the function get_multicast_control already print the error */
	}

	multicast_control->action = multicast_control_ipv6.action;
	multicast_control->control_type= multicast_control_ipv6.control_type;
	multicast_control->num_of_entries= multicast_control_ipv6.num_of_entries;
	for(i=0; i<multicast_control->num_of_entries; i++)
	{
		memcpy(&(multicast_control->entries[i]), &(multicast_control_ipv6.entries[i].ipv4_entry), sizeof(CTC_STACK_multicast_entry_t));
	}

	return (CTC_STACK_EXIT_OK); 
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_control_ipv6_ext"
PON_STATUS PON_CTC_STACK_get_multicast_control_ipv6_ext ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   CTC_STACK_multicast_control_ipv6_ext_t	*multicast_control )
{
	PON_STATUS result;
	bool more_frame = TRUE;
	unsigned short entry_index = 0;
	
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

	multicast_control->num_of_entries = 0;

	while (more_frame)
	{
		result = PON_get_multicast_control(olt_id, onu_id, multicast_control, &entry_index, &more_frame);
		if (result != CTC_STACK_EXIT_OK)
			return (result); /* the function get_multicast_control already print the error */
	}

	return (CTC_STACK_EXIT_OK); 
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_clear_multicast_control"

PON_STATUS PON_CTC_STACK_clear_multicast_control ( 
           const PON_olt_id_t   olt_id, 
           const PON_onu_id_t	onu_id )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char							send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf  =  CTC_EX_VAR_MULTICAST_CONTROL;
	unsigned char							received_num_of_entries=1;
	unsigned char							ctc_ret_val;
	CTC_STACK_multicast_control_ipv6_ext_t			send_multicast_control = {0};
	unsigned char							received_port_number=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;


    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	send_multicast_control.action = CTC_MULTICAST_CONTROL_ACTION_CLEAR;


	comparser_result = (short int)CTC_COMPOSE_multicast_control
                    ( 1/*number_of_entries*/, &send_multicast_control, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
                      SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


	total_length += sent_length;
	sent_length = max_length - total_length;
	

	
    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;



	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;

	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_multiple_management_object_group_num"

PON_STATUS PON_CTC_STACK_set_multicast_multiple_management_object_group_num ( 
           const PON_olt_id_t						                 olt_id, 
           const PON_onu_id_t						                 onu_id,
		   const unsigned char						                 number_of_entries,
		   const CTC_STACK_multicast_management_object_group_num_t   management_object_group_num )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_MULTICAST_GROUP_NUM;
	unsigned char								received_num_of_entries=number_of_entries;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char								local_number_of_entries = 0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for(i = 0; i < number_of_entries; i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_group_num[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id]) 
		}
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_group_num[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_multicast_group
									( 1, &management_object_group_num[i].group_num, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_group_num[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_multicast_group
									( 1, &management_object_group_num[0].group_num, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}

	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_group_num[i].management_object_index.port_number)
		}


		received_total_length += received_left_length;

		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );


		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
	}

	return result;       
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_multiple_management_object_group_num"

PON_STATUS PON_CTC_STACK_get_multicast_multiple_management_object_group_num ( 
           const PON_olt_id_t				                    olt_id, 
           const PON_onu_id_t					                onu_id,
		   unsigned char						               *number_of_entries,
		   CTC_STACK_multicast_management_object_group_num_t    management_object_group_num )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_MULTICAST_GROUP_NUM;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_group_num[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_group_num[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_group_num[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_group_num[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_multicast_group
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_group_num[i].group_num, &ctc_ret_val ,&received_left_length );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_multicast_group
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_group_num[i].group_num, &ctc_ret_val ,&received_left_length );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
			
			management_object_group_num[i].management_object_index.port_number  = received_management_object.index.port_number;
    		management_object_group_num[i].management_object_index.frame_number = received_management_object.index.frame_number;
    		management_object_group_num[i].management_object_index.slot_number  = received_management_object.index.slot_number;
    		management_object_group_num[i].management_object_index.port_type    = received_management_object.index.port_type;
			i++;
		}

		(*number_of_entries) = i;
	}
    
	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_all_management_object_group_num"

PON_STATUS PON_CTC_STACK_get_multicast_all_management_object_group_num ( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_multicast_management_object_group_num_t   management_object_group_num )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_multicast_multiple_management_object_group_num(olt_id, onu_id, number_of_entries, management_object_group_num));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_management_object_group_num"

PON_STATUS PON_CTC_STACK_set_multicast_management_object_group_num ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   const unsigned char	                group_num)
{
    short int								           result=CTC_STACK_EXIT_OK;
	CTC_STACK_multicast_management_object_group_num_t  management_object_group;
	unsigned char						               number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    management_object_group[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_group[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_group[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_group[0].management_object_index.port_type        = management_object_index.port_type;

    management_object_group[0].group_num                                = group_num;


	result = PON_CTC_STACK_set_multicast_multiple_management_object_group_num(olt_id, onu_id, number_of_entries, management_object_group);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_management_object_group_num"

PON_STATUS PON_CTC_STACK_get_multicast_management_object_group_num ( 
           const PON_olt_id_t	               olt_id, 
           const PON_onu_id_t	               onu_id,
		   const CTC_management_object_index_t management_object_index,
		   unsigned char	                  *group_num)
{
    short int								           result=CTC_STACK_EXIT_OK;
	CTC_STACK_multicast_management_object_group_num_t  management_object_group;
	unsigned char							           number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_group[0].management_object_index.port_number      = management_object_index.port_number; 
    management_object_group[0].management_object_index.slot_number      = management_object_index.slot_number; 
    management_object_group[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_group[0].management_object_index.port_type        = management_object_index.port_type;   


    result = PON_CTC_STACK_get_multicast_multiple_management_object_group_num(olt_id, onu_id, &number_of_entries, management_object_group);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*group_num) = management_object_group[0].group_num;


	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_all_port_group_num"

PON_STATUS PON_CTC_STACK_get_multicast_all_port_group_num ( 
           const PON_olt_id_t					   olt_id, 
           const PON_onu_id_t					   onu_id,
		   unsigned char						  *number_of_entries,
		   CTC_STACK_multicast_ports_group_num_t   ports_group_num )
{
    short int								            result;
    unsigned char                                       i;   
	CTC_STACK_multicast_management_object_group_num_t   management_object_group;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_group[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_group[0].management_object_index.slot_number      = 0x00;
    management_object_group[0].management_object_index.frame_number     = 0x00;
    management_object_group[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_multicast_multiple_management_object_group_num(olt_id, onu_id, number_of_entries, management_object_group);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_group_num[i].port_number         = (unsigned char)(management_object_group[i].management_object_index.port_number);
        ports_group_num[i].group_num           = management_object_group[i].group_num;
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_port_group_num"

PON_STATUS PON_CTC_STACK_set_multicast_group_num ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   const unsigned char	 group_num)
{
    short int								           result=CTC_STACK_EXIT_OK;
	CTC_STACK_multicast_management_object_group_num_t  management_object_group;
	unsigned char						               number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_group[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_group[0].management_object_index.port_number  = port_number;
    }


    management_object_group[0].management_object_index.slot_number      = 0x00;
    management_object_group[0].management_object_index.frame_number     = 0x00;
    management_object_group[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

    management_object_group[0].group_num                                = group_num;


	result = PON_CTC_STACK_set_multicast_multiple_management_object_group_num(olt_id, onu_id, number_of_entries, management_object_group);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_group_num"

PON_STATUS PON_CTC_STACK_get_multicast_group_num ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   unsigned char	    *group_num)
{
    short int								           result=CTC_STACK_EXIT_OK;
	CTC_STACK_multicast_management_object_group_num_t  management_object_group;
	unsigned char							           number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_group[0].management_object_index.port_number      = port_number;
    management_object_group[0].management_object_index.slot_number      = 0x00;
    management_object_group[0].management_object_index.frame_number     = 0x00;
    management_object_group[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_multicast_multiple_management_object_group_num(olt_id, onu_id, &number_of_entries, management_object_group);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*group_num) = management_object_group[0].group_num;


	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multiple_management_object_phy_admin_state"

PON_STATUS PON_CTC_STACK_get_multiple_management_object_phy_admin_state ( 
           const PON_olt_id_t				               olt_id, 
           const PON_onu_id_t				               onu_id,
		   unsigned char					              *number_of_entries,
		   CTC_STACK_ethernet_management_object_state_t    management_object_phy_admin_state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_STANDARD_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_STD_ATTR_GET_PHY_ADMIN_STATE;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val,i;
	CTC_STACK_ethernet_port_state_t				received_phy_admin_state;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;


	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_phy_admin_state[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_phy_admin_state[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_phy_admin_state[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_phy_admin_state[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_phy_admin_state
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &received_phy_admin_state, &ctc_ret_val ,&received_left_length );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;

			
			management_object_phy_admin_state[i].state			= received_phy_admin_state;			
			switch(received_phy_admin_state)
			{
				case CTC_STACK_ETHERNET_PORT_STATE_DISABLE:
					management_object_phy_admin_state[i].state = DISABLE;
					break;
				case CTC_STACK_ETHERNET_PORT_STATE_ENABLE:
					management_object_phy_admin_state[i].state = ENABLE;
					break;
				default:
					PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id,
								"OLT %d ONU %d retrieved unsupported state %d\n",
								olt_id, onu_id, received_phy_admin_state);
			}
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
			received_total_length += received_left_length;


			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_phy_admin_state
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &received_phy_admin_state, &ctc_ret_val ,&received_left_length );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
			
			management_object_phy_admin_state[i].management_object_index.port_number  = received_management_object.index.port_number;
    		management_object_phy_admin_state[i].management_object_index.frame_number = received_management_object.index.frame_number;
    		management_object_phy_admin_state[i].management_object_index.slot_number  = received_management_object.index.slot_number;
    		management_object_phy_admin_state[i].management_object_index.port_type    = received_management_object.index.port_type;

			switch(received_phy_admin_state)
			{
				case CTC_STACK_ETHERNET_PORT_STATE_DISABLE:
					management_object_phy_admin_state[i].state = DISABLE;
					break;
				case CTC_STACK_ETHERNET_PORT_STATE_ENABLE:
					management_object_phy_admin_state[i].state = ENABLE;
					break;
				default:
					PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id,
								"OLT %d ONU %d retrieved unsupported state %d\n",
								olt_id, onu_id, received_phy_admin_state);
			}
		
			i++;
		}

		(*number_of_entries) = i;
	}
    
	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_all_management_object_phy_admin_state"

PON_STATUS PON_CTC_STACK_get_all_management_object_phy_admin_state ( 
           const PON_olt_id_t				               olt_id, 
           const PON_onu_id_t				               onu_id,
		   unsigned char					              *number_of_entries,
		   CTC_STACK_ethernet_management_object_state_t    management_object_info )
{
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_multiple_management_object_phy_admin_state(olt_id, onu_id, number_of_entries, management_object_info));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_management_object_phy_admin_state"

PON_STATUS PON_CTC_STACK_get_management_object_phy_admin_state ( 
           const PON_olt_id_t	               olt_id, 
           const PON_onu_id_t	               onu_id,
		   const CTC_management_object_index_t management_object_index,
		   bool				                  *state )
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_ethernet_management_object_state_t  management_object_phy_admin_state;
	unsigned char							      number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_phy_admin_state[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_phy_admin_state[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_phy_admin_state[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_phy_admin_state[0].management_object_index.port_type        = management_object_index.port_type;


    result = PON_CTC_STACK_get_multiple_management_object_phy_admin_state(olt_id, onu_id, &number_of_entries, management_object_phy_admin_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*state) = management_object_phy_admin_state[0].state;


	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_all_port_phy_admin_state"

PON_STATUS PON_CTC_STACK_get_all_port_phy_admin_state ( 
           const PON_olt_id_t				   olt_id, 
           const PON_onu_id_t				   onu_id,
		   unsigned char					  *number_of_entries,
		   CTC_STACK_ethernet_ports_state_t    ports_info )
{
    short int								        result;
    unsigned char                                   i;   
	CTC_STACK_ethernet_management_object_state_t    management_object_phy_admin_state;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    
	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_phy_admin_state[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_phy_admin_state[0].management_object_index.slot_number      = 0x00;
    management_object_phy_admin_state[0].management_object_index.frame_number     = 0x00;
    management_object_phy_admin_state[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_multiple_management_object_phy_admin_state(olt_id, onu_id, number_of_entries, management_object_phy_admin_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_info[i].port_number         = (unsigned char)(management_object_phy_admin_state[i].management_object_index.port_number);
        ports_info[i].state               = management_object_phy_admin_state[i].state;
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_phy_admin_state"

PON_STATUS PON_CTC_STACK_get_phy_admin_state ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   bool				    *state )
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_ethernet_management_object_state_t  management_object_phy_admin_state;
	unsigned char							      number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_phy_admin_state[0].management_object_index.port_number      = port_number;
    management_object_phy_admin_state[0].management_object_index.slot_number      = 0x00;
    management_object_phy_admin_state[0].management_object_index.frame_number     = 0x00;
    management_object_phy_admin_state[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_multiple_management_object_phy_admin_state(olt_id, onu_id, &number_of_entries, management_object_phy_admin_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*state) = management_object_phy_admin_state[0].state;


	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multiple_management_object_phy_admin_control"

PON_STATUS PON_CTC_STACK_set_multiple_management_object_phy_admin_control ( 
           const PON_olt_id_t						              olt_id, 
           const PON_onu_id_t						              onu_id,
		   const unsigned char						              number_of_entries,
		   const CTC_STACK_ethernet_management_object_state_t     management_object_phy_admin_control )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_STANDARD_BRANCH_ACTION;
	unsigned short								expected_leaf  =  CTC_STD_ACTION_PHY_ADMIN_CONTROL;
	unsigned char								received_num_of_entries=number_of_entries;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char								local_number_of_entries=0;
	CTC_STACK_ethernet_port_action_t			send_phy_admin_control;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for(i = 0; i < number_of_entries; i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_phy_admin_control[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
		}
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	
	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_phy_admin_control[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			if(management_object_phy_admin_control[i].state)
			{
				send_phy_admin_control = CTC_STACK_ETHERNET_PORT_ACTION_ACTIVATE;
			}
			else
			{
				send_phy_admin_control = CTC_STACK_ETHERNET_PORT_ACTION_DEACTIVATE;
			}

			comparser_result = (short int)CTC_COMPOSE_phy_admin_control
									( 1, &send_phy_admin_control, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_phy_admin_control[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			if(management_object_phy_admin_control[0].state)
			{
				send_phy_admin_control = CTC_STACK_ETHERNET_PORT_ACTION_ACTIVATE;
			}
			else
			{
				send_phy_admin_control = CTC_STACK_ETHERNET_PORT_ACTION_DEACTIVATE;
			}

			comparser_result = (short int)CTC_COMPOSE_phy_admin_control
									( 1, &send_phy_admin_control, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}

	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_phy_admin_control[i].management_object_index.port_number)
		}


		received_total_length += received_left_length;

		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );


		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
	}
	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_management_object_phy_admin_control"

PON_STATUS PON_CTC_STACK_set_management_object_phy_admin_control ( 
           const PON_olt_id_t	               olt_id, 
           const PON_onu_id_t	               onu_id,
		   const CTC_management_object_index_t management_object_index,
		   const bool		                   state)
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_ethernet_management_object_state_t  management_object_phy_admin_control;
	unsigned char						          number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    management_object_phy_admin_control[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_phy_admin_control[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_phy_admin_control[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_phy_admin_control[0].management_object_index.port_type        = management_object_index.port_type;

    management_object_phy_admin_control[0].state                                    = state;


	result = PON_CTC_STACK_set_multiple_management_object_phy_admin_control(olt_id, onu_id, number_of_entries, management_object_phy_admin_control);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_phy_admin_control"

PON_STATUS PON_CTC_STACK_set_phy_admin_control ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const bool		     state)
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_ethernet_management_object_state_t  management_object_phy_admin_control;
	unsigned char						          number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_phy_admin_control[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_phy_admin_control[0].management_object_index.port_number  = port_number;
    }


    management_object_phy_admin_control[0].management_object_index.slot_number      = 0x00;
    management_object_phy_admin_control[0].management_object_index.frame_number     = 0x00;
    management_object_phy_admin_control[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

    management_object_phy_admin_control[0].state                                    = state;


	result = PON_CTC_STACK_set_multiple_management_object_phy_admin_control(olt_id, onu_id, number_of_entries, management_object_phy_admin_control);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_multiple_management_object_admin_state"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_multiple_management_object_admin_state ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char					             *number_of_entries,
		   CTC_STACK_ethernet_management_object_state_t   management_object_state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_STANDARD_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_STD_ATTR_GET_AUTO_NEG_ADMIN_STATE;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val,i;
	CTC_STACK_ethernet_port_state_t				received_port_state;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_state[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_state[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_state[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_state[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_auto_negotiation_admin_state
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &received_port_state, &ctc_ret_val ,&received_left_length );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;

			switch(received_port_state)
			{
			case CTC_STACK_ETHERNET_PORT_STATE_DISABLE:
				management_object_state[i].state = DISABLE;
				break;
			case CTC_STACK_ETHERNET_PORT_STATE_ENABLE:
				management_object_state[i].state = ENABLE;
				break;
			default:
				PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id,
						"OLT %d ONU %d received unsupported state %d\n",
						olt_id, onu_id, received_port_state);
			}
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_auto_negotiation_admin_state
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &received_port_state, &ctc_ret_val ,&received_left_length );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
			
			management_object_state[i].management_object_index.port_number  = received_management_object.index.port_number;
    		management_object_state[i].management_object_index.frame_number = received_management_object.index.frame_number;
    		management_object_state[i].management_object_index.slot_number  = received_management_object.index.slot_number;
    		management_object_state[i].management_object_index.port_type    = received_management_object.index.port_type;

			switch(received_port_state)
			{
			case CTC_STACK_ETHERNET_PORT_STATE_DISABLE:
				management_object_state[i].state = DISABLE;
				break;
			case CTC_STACK_ETHERNET_PORT_STATE_ENABLE:
				management_object_state[i].state = ENABLE;
				break;
			default:
				PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id,
						"OLT %d ONU %d received unsupported state %d\n",
						olt_id, onu_id, received_port_state);
			}
			
			i++;
		}

		(*number_of_entries) = i;
	}
    
	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_all_management_object_admin_state"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_all_management_object_admin_state ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char					             *number_of_entries,
		   CTC_STACK_ethernet_management_object_state_t   management_object_info )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_auto_negotiation_multiple_management_object_admin_state(olt_id, onu_id, number_of_entries, management_object_info));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_management_object_auto_negotiation_admin_state"

PON_STATUS PON_CTC_STACK_get_management_object_auto_negotiation_admin_state ( 
           const PON_olt_id_t	                 olt_id, 
           const PON_onu_id_t	                 onu_id,
		   const CTC_management_object_index_t   management_object_index,
		   bool				                    *state )
{
    short int								      result=CTC_STACK_EXIT_OK;
    CTC_STACK_ethernet_management_object_state_t  management_object_info;
	unsigned char							      number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_info[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_info[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_info[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_info[0].management_object_index.port_type        = management_object_index.port_type;


    result = PON_CTC_STACK_get_auto_negotiation_multiple_management_object_admin_state(olt_id, onu_id, &number_of_entries, management_object_info);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*state) = management_object_info[0].state;

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_all_ports_admin_state"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_all_ports_admin_state ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   unsigned char					 *number_of_entries,
		   CTC_STACK_ethernet_ports_state_t   ports_info )
{
    short int								       result;
    unsigned char                                  i;   
    CTC_STACK_ethernet_management_object_state_t   management_object_info;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_info[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_info[0].management_object_index.slot_number      = 0x00;
    management_object_info[0].management_object_index.frame_number     = 0x00;
    management_object_info[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_auto_negotiation_multiple_management_object_admin_state(olt_id, onu_id, number_of_entries, management_object_info);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_info[i].port_number         = (unsigned char)(management_object_info[i].management_object_index.port_number);
        ports_info[i].state               = management_object_info[i].state;
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_admin_state"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_admin_state ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   bool				    *state )
{
    short int								      result=CTC_STACK_EXIT_OK;
    CTC_STACK_ethernet_management_object_state_t  management_object_info;
	unsigned char							      number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_info[0].management_object_index.port_number      = port_number;
    management_object_info[0].management_object_index.slot_number      = 0x00;
    management_object_info[0].management_object_index.frame_number     = 0x00;
    management_object_info[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_auto_negotiation_multiple_management_object_admin_state(olt_id, onu_id, &number_of_entries, management_object_info);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*state) = management_object_info[0].state;

	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_multiple_management_object_local_technology_ability"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_multiple_management_object_local_technology_ability ( 
           const PON_olt_id_t									               olt_id, 
           const PON_onu_id_t									               onu_id,
		   unsigned char										              *number_of_entries,
		   CTC_STACK_auto_negotiation_management_object_technology_ability_t   management_object_info )
{
    short int												result,comparser_result;
    unsigned short											received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char											sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char											received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char											send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char											expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char											expected_branch =  OAM_VENDOR_STANDARD_BRANCH_ATTR;
	unsigned short											expected_leaf	=  CTC_STD_ATTR_GET_AUTO_NEG_LOCAL_TECHNOLOGY_ABILITY;
	unsigned char											received_num_of_entries=0;
	unsigned char											ctc_ret_val,i;
	unsigned short											max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			                management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					                sent_management_object;
	CTC_management_object_t					                received_management_object;


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_info[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_info[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_info[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_info[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_auto_neg_local_technology_ability
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_info[i].abilities, &ctc_ret_val ,&received_left_length );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;

		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_auto_neg_local_technology_ability
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_info[i].abilities, &ctc_ret_val ,&received_left_length );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
			
			management_object_info[i].management_object_index.port_number  = received_management_object.index.port_number;
    		management_object_info[i].management_object_index.frame_number = received_management_object.index.frame_number;
    		management_object_info[i].management_object_index.slot_number  = received_management_object.index.slot_number;
    		management_object_info[i].management_object_index.port_type    = received_management_object.index.port_type;
			i++;
		}

		(*number_of_entries) = i;
	}
    
	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_all_management_object_local_technology_ability"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_all_management_object_local_technology_ability ( 
           const PON_olt_id_t									               olt_id, 
           const PON_onu_id_t									               onu_id,
		   unsigned char										              *number_of_entries,
		   CTC_STACK_auto_negotiation_management_object_technology_ability_t   management_object_abilities )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_auto_negotiation_multiple_management_object_local_technology_ability(olt_id, onu_id, number_of_entries, management_object_abilities));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_management_object_local_technology_ability"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_management_object_local_technology_ability ( 
           const PON_olt_id_t								 olt_id, 
           const PON_onu_id_t								 onu_id,
		   const CTC_management_object_index_t               management_object_index,
		   CTC_STACK_auto_negotiation_technology_ability_t  *abilities )
{
    short int								                            result=CTC_STACK_EXIT_OK;
    CTC_STACK_auto_negotiation_management_object_technology_ability_t   management_object_info;
    unsigned char							                            number_of_entries = 1, i;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_info[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_info[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_info[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_info[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_auto_negotiation_multiple_management_object_local_technology_ability(olt_id, onu_id, &number_of_entries, management_object_info);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	abilities->number_of_abilities = management_object_info[0].abilities.number_of_abilities;

	for( i = 0; i < management_object_info[0].abilities.number_of_abilities; i++)
	{
		abilities->technology[i] = management_object_info[0].abilities.technology[i];
	}

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_all_ports_local_technology_ability"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_all_ports_local_technology_ability ( 
           const PON_olt_id_t									   olt_id, 
           const PON_onu_id_t									   onu_id,
		   unsigned char										  *number_of_entries,
		   CTC_STACK_auto_negotiation_ports_technology_ability_t   ports_abilities )
{
    short int								                            result;
    unsigned char                                                       i,j;   
    CTC_STACK_auto_negotiation_management_object_technology_ability_t   management_object_info;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_info[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_info[0].management_object_index.slot_number      = 0x00;
    management_object_info[0].management_object_index.frame_number     = 0x00;
    management_object_info[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_auto_negotiation_multiple_management_object_local_technology_ability(olt_id, onu_id, number_of_entries, management_object_info);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_abilities[i].port_number                   = (unsigned char)(management_object_info[i].management_object_index.port_number);
        
        ports_abilities[i].abilities.number_of_abilities = management_object_info[i].abilities.number_of_abilities;
        
        for( j = 0; j < management_object_info[i].abilities.number_of_abilities; j++)
        {
            ports_abilities[i].abilities.technology[j] = management_object_info[i].abilities.technology[j];
        }
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_local_technology_ability"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_local_technology_ability ( 
           const PON_olt_id_t								 olt_id, 
           const PON_onu_id_t								 onu_id,
		   const unsigned char								 port_number,
		   CTC_STACK_auto_negotiation_technology_ability_t  *abilities )
{
    short int								                            result=CTC_STACK_EXIT_OK;
    CTC_STACK_auto_negotiation_management_object_technology_ability_t   management_object_info;
    unsigned char							                            number_of_entries = 1, i;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_info[0].management_object_index.port_number      = port_number;
    management_object_info[0].management_object_index.slot_number      = 0x00;
    management_object_info[0].management_object_index.frame_number     = 0x00;
    management_object_info[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_auto_negotiation_multiple_management_object_local_technology_ability(olt_id, onu_id, &number_of_entries, management_object_info);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	abilities->number_of_abilities = management_object_info[0].abilities.number_of_abilities;

	for( i = 0; i < management_object_info[0].abilities.number_of_abilities; i++)
	{
		abilities->technology[i] = management_object_info[0].abilities.technology[i];
	}

	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_multiple_management_object_advertised_technology_ability"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_multiple_management_object_advertised_technology_ability ( 
           const PON_olt_id_t									               olt_id, 
           const PON_onu_id_t									               onu_id,
		   unsigned char										              *number_of_entries,
		   CTC_STACK_auto_negotiation_management_object_technology_ability_t   management_object_info )
{
    short int												result,comparser_result;
    unsigned short											received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char											sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char											received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char											send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char											expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char											expected_branch =  OAM_VENDOR_STANDARD_BRANCH_ATTR;
	unsigned short											expected_leaf	=  CTC_STD_ATTR_GET_AUTO_NEG_ADVERTISED_TECHNOLOGY_ABILITY;
	unsigned char											received_num_of_entries=0;
	unsigned char											ctc_ret_val,i;
	unsigned char											received_port_number = 0;
	unsigned short											max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			                management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t				                 	sent_management_object;
	CTC_management_object_t					                received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_info[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_info[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_info[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_info[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_auto_neg_advertised_technology_ability
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_info[i].abilities, &ctc_ret_val ,&received_left_length );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;

		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_auto_neg_advertised_technology_ability
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_info[i].abilities, &ctc_ret_val ,&received_left_length );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
			
			management_object_info[i].management_object_index.port_number  = received_management_object.index.port_number;
    		management_object_info[i].management_object_index.frame_number = received_management_object.index.frame_number;
    		management_object_info[i].management_object_index.slot_number  = received_management_object.index.slot_number;
    		management_object_info[i].management_object_index.port_type    = received_management_object.index.port_type;
			i++;
		}

		(*number_of_entries) = i;
	}
    
	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_all_management_object_advertised_technology_ability"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_all_management_object_advertised_technology_ability ( 
           const PON_olt_id_t									               olt_id, 
           const PON_onu_id_t									               onu_id,
		   unsigned char										              *number_of_entries,
		   CTC_STACK_auto_negotiation_management_object_technology_ability_t   management_object_abilities )
{
    short int   result;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;

    result = PON_CTC_STACK_get_auto_negotiation_multiple_management_object_advertised_technology_ability(olt_id, onu_id, number_of_entries, management_object_abilities);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_management_object_advertised_technology_ability"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_management_object_advertised_technology_ability ( 
           const PON_olt_id_t								 olt_id, 
           const PON_onu_id_t								 onu_id,
		   const CTC_management_object_index_t               management_object_index,
		   CTC_STACK_auto_negotiation_technology_ability_t  *abilities )
{
    short int								                            result=CTC_STACK_EXIT_OK;
    CTC_STACK_auto_negotiation_management_object_technology_ability_t   management_object_info;
	unsigned char							                            number_of_entries = 1, i;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_info[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_info[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_info[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_info[0].management_object_index.port_type        = management_object_index.port_type;


    result = PON_CTC_STACK_get_auto_negotiation_multiple_management_object_advertised_technology_ability(olt_id, onu_id, &number_of_entries, management_object_info);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	abilities->number_of_abilities = management_object_info[0].abilities.number_of_abilities;

	for( i = 0; i < management_object_info[0].abilities.number_of_abilities; i++)
	{
		abilities->technology[i] = management_object_info[0].abilities.technology[i];
	}

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_all_ports_advertised_technology_ability"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_all_ports_advertised_technology_ability ( 
           const PON_olt_id_t									   olt_id, 
           const PON_onu_id_t									   onu_id,
		   unsigned char										  *number_of_entries,
		   CTC_STACK_auto_negotiation_ports_technology_ability_t   ports_abilities )
{
    short int								                            result;
    unsigned char                                                       i,j;   
    CTC_STACK_auto_negotiation_management_object_technology_ability_t   management_object_info;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_info[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_info[0].management_object_index.slot_number      = 0x00;
    management_object_info[0].management_object_index.frame_number     = 0x00;
    management_object_info[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_auto_negotiation_multiple_management_object_advertised_technology_ability(olt_id, onu_id, number_of_entries, management_object_info);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_abilities[i].port_number         = (unsigned char)(management_object_info[i].management_object_index.port_number);
        ports_abilities[i].abilities.number_of_abilities = management_object_info[i].abilities.number_of_abilities;
        
        for( j = 0; j < (management_object_info[i].abilities.number_of_abilities); j++)
        {
            ports_abilities[i].abilities.technology[j] = management_object_info[i].abilities.technology[j];
        }
    }


	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auto_negotiation_advertised_technology_ability"

PON_STATUS PON_CTC_STACK_get_auto_negotiation_advertised_technology_ability ( 
           const PON_olt_id_t								 olt_id, 
           const PON_onu_id_t								 onu_id,
		   const unsigned char								 port_number,
		   CTC_STACK_auto_negotiation_technology_ability_t  *abilities )
{
    short int								                            result=CTC_STACK_EXIT_OK;
    CTC_STACK_auto_negotiation_management_object_technology_ability_t   management_object_info;
	unsigned char							                            number_of_entries = 1, i;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_info[0].management_object_index.port_number      = port_number;
    management_object_info[0].management_object_index.slot_number      = 0x00;
    management_object_info[0].management_object_index.frame_number     = 0x00;
    management_object_info[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_auto_negotiation_multiple_management_object_advertised_technology_ability(olt_id, onu_id, &number_of_entries, management_object_info);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	abilities->number_of_abilities = management_object_info[0].abilities.number_of_abilities;

	for( i = 0; i < management_object_info[0].abilities.number_of_abilities; i++)
	{
		abilities->technology[i] = management_object_info[0].abilities.technology[i];
	}

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_auto_negotiation_multiple_management_object_restart_auto_config"

PON_STATUS PON_CTC_STACK_set_auto_negotiation_multiple_management_object_restart_auto_config ( 
           const PON_olt_id_t		             olt_id, 
           const PON_onu_id_t		             onu_id,
		   const unsigned char		             number_of_entries,
		   const CTC_STACK_management_object_t   management_objects )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_STANDARD_BRANCH_ACTION;
	unsigned short								expected_leaf  =  CTC_STD_ACTION_AUTO_NEG_RESTART_AUTO_CONFIG;
	unsigned char								received_num_of_entries=number_of_entries;
	unsigned char								ctc_ret_val,i;
	unsigned char								dummy = 0;	
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char								local_number_of_entries=0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for(i = 0; i < number_of_entries; i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_objects[i].port_number, discovery_db.common_version[olt_id][onu_id]) 
		}
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_objects[i]));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_auto_negotiation_restart_auto_config
                            ( 1/*number_of_entries*/, &dummy, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
                              SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_objects[0]));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_auto_negotiation_restart_auto_config
                            ( 1/*number_of_entries*/, &dummy, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
                              SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}

	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_objects[i].port_number)
		}


		received_total_length += received_left_length;

		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        				  expected_leaf, &ctc_ret_val, &received_left_length  );


		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
	}

	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_auto_negotiation_management_object_restart_auto_config"

PON_STATUS PON_CTC_STACK_set_auto_negotiation_management_object_restart_auto_config ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index)
{
    short int						result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_t   management_objects;
	unsigned char					number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }
   
    management_objects[0].port_number      = management_object_index.port_number;
    management_objects[0].slot_number      = management_object_index.slot_number;
    management_objects[0].frame_number     = management_object_index.frame_number;
    management_objects[0].port_type        = management_object_index.port_type;


	result = PON_CTC_STACK_set_auto_negotiation_multiple_management_object_restart_auto_config(olt_id, onu_id, number_of_entries, management_objects);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_auto_negotiation_restart_auto_config"

PON_STATUS PON_CTC_STACK_set_auto_negotiation_restart_auto_config ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number)
{
    short int						result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_t   management_objects;
	unsigned char					number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_objects[0].port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_objects[0].port_number  = port_number;
    }

    management_objects[0].slot_number      = 0x00;
    management_objects[0].frame_number     = 0x00;
    management_objects[0].port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


	result = PON_CTC_STACK_set_auto_negotiation_multiple_management_object_restart_auto_config(olt_id, onu_id, number_of_entries, management_objects);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_auto_negotiation_multiple_management_object_admin_control"

PON_STATUS PON_CTC_STACK_set_auto_negotiation_multiple_management_object_admin_control( 
           const PON_olt_id_t						            olt_id, 
           const PON_onu_id_t						            onu_id,
		   const unsigned char						            number_of_entries,
		   const CTC_STACK_ethernet_management_object_state_t   management_object_state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_STANDARD_BRANCH_ACTION;
	unsigned short								expected_leaf  =  CTC_STD_ACTION_AUTO_NEG_ADMIN_CONTROL;
	unsigned char								received_num_of_entries=number_of_entries;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	CTC_STACK_ethernet_port_action_t			send_port_action;
	unsigned char								local_number_of_entries=0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for(i = 0; i < number_of_entries; i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_state[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
		}
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_state[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			if(management_object_state[i].state)
			{
				send_port_action = CTC_STACK_ETHERNET_PORT_ACTION_ACTIVATE;
			}
			else
			{
				send_port_action = CTC_STACK_ETHERNET_PORT_ACTION_DEACTIVATE;
			}

			comparser_result = (short int)CTC_COMPOSE_auto_negotiation_admin_control
									( 1, &send_port_action, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_state[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			if(management_object_state[0].state)
			{
				send_port_action = CTC_STACK_ETHERNET_PORT_ACTION_ACTIVATE;
			}
			else
			{
				send_port_action = CTC_STACK_ETHERNET_PORT_ACTION_DEACTIVATE;
			}

			comparser_result = (short int)CTC_COMPOSE_auto_negotiation_admin_control
									( 1, &send_port_action, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}

	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_state[i].management_object_index.port_number)
		}

		received_total_length += received_left_length;

		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        				  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
	}

	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_auto_negotiation_management_object_admin_control"

PON_STATUS PON_CTC_STACK_set_auto_negotiation_management_object_admin_control ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   const bool			                state)
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_ethernet_management_object_state_t  management_object_state;
	unsigned char						          number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    management_object_state[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_state[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_state[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_state[0].management_object_index.port_type        = management_object_index.port_type;

    management_object_state[0].state                                    = state;


	result = PON_CTC_STACK_set_auto_negotiation_multiple_management_object_admin_control(olt_id, onu_id, number_of_entries, management_object_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_auto_negotiation_admin_control"

PON_STATUS PON_CTC_STACK_set_auto_negotiation_admin_control ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   const bool			 state)
{
    short int								      result=CTC_STACK_EXIT_OK;
	CTC_STACK_ethernet_management_object_state_t  management_object_state;
	unsigned char						          number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_state[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_state[0].management_object_index.port_number  = port_number;
    }


    management_object_state[0].management_object_index.slot_number      = 0x00;
    management_object_state[0].management_object_index.frame_number     = 0x00;
    management_object_state[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

    management_object_state[0].state                                    = state;


	result = PON_CTC_STACK_set_auto_negotiation_multiple_management_object_admin_control(olt_id, onu_id, number_of_entries, management_object_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_fec_ability"

PON_STATUS PON_CTC_STACK_get_fec_ability ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   CTC_STACK_standard_FEC_ability_t  *fec_ability )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_STANDARD_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_STD_ATTR_FEC_ABILITY;
	unsigned char							received_num_of_entries=0;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							ctc_ret_val;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


    comparser_result = (short int)CTC_PARSE_fec_ability
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          fec_ability, &ctc_ret_val, &received_left_length  );

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;

		
	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_fec_mode"

PON_STATUS PON_CTC_STACK_get_fec_mode ( 
           const PON_olt_id_t			   olt_id, 
           const PON_onu_id_t			   onu_id,
		   CTC_STACK_standard_FEC_mode_t  *fec_mode )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_STANDARD_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_STD_ATTR_FEC_MODE;
	unsigned char							received_num_of_entries=0;
	PON_onu_id_t							onu_leaf_id = onu_id;
	unsigned char							ctc_ret_val;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)CTC_PARSE_fec_mode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          fec_mode, &ctc_ret_val, &received_left_length  );

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;

		
	return result;
        
}
#undef PONLOG_FUNCTION_NAME




#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_fec_mode"

PON_STATUS PON_CTC_STACK_set_fec_mode ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   const CTC_STACK_standard_FEC_mode_t  fec_mode)
{
    short int								result,comparser_result, pas_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char							send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_code   =  OAM_VENDOR_STANDARD_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_STD_ATTR_FEC_MODE;
	unsigned char							received_num_of_entries=1;
	unsigned char							ctc_ret_val;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
    general_handler_function_t              the_handler;
	short int					            client;
    PON_redundancy_olt_state_t              redundancy_state = PON_OLT_REDUNDANCY_STATE_NONE;
	PON_redundancy_llid_redundancy_mode_t   onu_redundancy_mode = PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL;


	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    result = Get_redundancy_olt_state(olt_id, &redundancy_state);

    result = Get_redundancy_onu_mode(olt_id, onu_id , &onu_redundancy_mode);

    if ( (redundancy_state == PON_OLT_REDUNDANCY_STATE_MASTER) ||
         (redundancy_state == PON_OLT_REDUNDANCY_STATE_NONE)||
         (redundancy_state == PON_OLT_REDUNDANCY_STATE_ASSIGN_ODD_LLID)||
         (redundancy_state == PON_OLT_REDUNDANCY_STATE_ASSIGN_EVEN_LLID))
    {
        CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
            
        if((onu_redundancy_mode == PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL)||
         (onu_redundancy_mode == PON_REDUNDANCY_LLID_REDUNDANCY_ACTIVE))
        {
            comparser_result = (short int)CTC_COMPOSE_opcode
                                    ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
            CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    	    total_length += sent_length;
    	    sent_length = max_length - total_length;

            comparser_result = (short int)CTC_COMPOSE_fec_mode
                                    ( 1/*number_of_entries*/, &fec_mode, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
                                      SEND_COMPARSER_BUFFER, &sent_length );
            CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    	    total_length += sent_length;
    	    sent_length = max_length - total_length;
        }
    }


	if(fec_mode == STD_FEC_MODE_ENABLED)
	{
		pas_result = PAS_set_llid_fec_mode(olt_id, PON_BROADCAST_LLID, TRUE);

		result = Convert_ponsoft_error_code_to_ctc_stack(pas_result);

		if(result != CTC_STACK_EXIT_OK)
		{
			PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id,
								"OLT %d ONU %d failed to PAS_set_llid_fec_mode error code (%d)\n",
								olt_id, onu_id, result);
			return result;
		}

		pas_result = PAS_set_llid_fec_mode(olt_id, onu_id, TRUE);

		result = Convert_ponsoft_error_code_to_ctc_stack(pas_result);

		if(result != CTC_STACK_EXIT_OK)
		{
			PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id,
								"OLT %d ONU %d failed to PAS_set_llid_fec_mode error code (%d)\n",
								olt_id, onu_id, result);
			return result;
		}
	}


    if ( (redundancy_state == PON_OLT_REDUNDANCY_STATE_MASTER) ||
         (redundancy_state == PON_OLT_REDUNDANCY_STATE_NONE)||
         (redundancy_state == PON_OLT_REDUNDANCY_STATE_ASSIGN_ODD_LLID)||
         (redundancy_state == PON_OLT_REDUNDANCY_STATE_ASSIGN_EVEN_LLID))
    {
        if((onu_redundancy_mode == PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL)||
         (onu_redundancy_mode == PON_REDUNDANCY_LLID_REDUNDANCY_ACTIVE))
        {
            result = Send_receive_vendor_extension_message 
                            ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                              received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
            CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
            
    	    comparser_result = (short int)CTC_PARSE_opcode
                                         ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
    	    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    	    received_total_length += received_left_length;

      	    comparser_result = (short int)CTC_PARSE_general_ack
    		    				( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        		    			  expected_leaf, &ctc_ret_val, &received_left_length  );

    	    received_total_length += received_left_length;

            CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
        }
    }

	if(fec_mode == STD_FEC_MODE_DISABLED)
	{
        PON_fec_status_raw_data_t fec_status;
		PON_timestamp_t			  timestamp; 

		pas_result = PAS_set_llid_fec_mode(olt_id, onu_id, FALSE);

		result = Convert_ponsoft_error_code_to_ctc_stack(pas_result);

		if(result != CTC_STACK_EXIT_OK)
		{
			PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id,
								"OLT %d ONU %d failed to PAS_set_llid_fec_mode error code (%d)\n",
								olt_id, onu_id, result);
			return result;
		}

        /* Get the number of llids with FEC enable */
		if ((result = PAS_get_raw_statistics(olt_id,
                                   PON_OLT_ID,
                                   PON_RAW_STAT_FEC_STATUS,
                                   0,
                                   &fec_status,
                                   &timestamp)) != PAS_EXIT_OK)
        {
			PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id,
							"OLT %d ONU %d failed to PAS_get_llid_fec_mode error code (%d)\n",
							olt_id, onu_id, result);
		   	return result;

		}


		if( fec_status.number_of_llids_with_fec == 0 ) /* No ONUs with FEC disable broadcast LLID */
		{
			pas_result = PAS_set_llid_fec_mode(olt_id, PON_BROADCAST_LLID, FALSE);

			result = Convert_ponsoft_error_code_to_ctc_stack(pas_result);

			if(result != CTC_STACK_EXIT_OK)
			{
				PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id,
									"OLT %d ONU %d failed to PAS_set_llid_fec_mode error code (%d)\n",
									olt_id, onu_id, result);
				return result;
			}
		}
	}

    if ((redundancy_state == PON_OLT_REDUNDANCY_STATE_MASTER)||
       (onu_redundancy_mode == PON_REDUNDANCY_LLID_REDUNDANCY_ACTIVE))
    {
        /*inform the update fec mode*/
        for (client = 0; client < PON_MAX_CLIENT; client++)
        {
            PON_general_get_handler_function( &ctc_db_handlers,
                                              CTC_STACK_HANDLER_UPDATE_FEC_MODE,
                                              client ,
                                              &the_handler );
            if (the_handler != NULL)
            {
                the_handler ( olt_id, onu_id,
                              fec_mode );
            }
        }
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_multiple_management_object_link_state"

PON_STATUS PON_CTC_STACK_get_ethernet_multiple_management_object_link_state ( 
           const PON_olt_id_t				                   olt_id, 
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_ethernet_management_object_link_state_t   management_object_link_state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_ETH_PORT_LINK_STATE;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t		     	management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t				    	sent_management_object;
	CTC_management_object_t					    received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_link_state[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_link_state[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_link_state[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_link_state[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_ethernet_port_link_state
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_link_state[i].link_state, &ctc_ret_val ,&received_left_length );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
		

			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_ethernet_port_link_state
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &management_object_link_state[i].link_state, &ctc_ret_val,&received_left_length  );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;

			management_object_link_state[i].management_object_index.port_number	 = received_management_object.index.port_number;
    		management_object_link_state[i].management_object_index.frame_number = received_management_object.index.frame_number;
    		management_object_link_state[i].management_object_index.slot_number  = received_management_object.index.slot_number;
    		management_object_link_state[i].management_object_index.port_type    = received_management_object.index.port_type;
            i++;
		}
		(*number_of_entries) = i;
	}
    
	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_all_management_object_link_state"

PON_STATUS PON_CTC_STACK_get_ethernet_all_management_object_link_state ( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_ethernet_management_object_link_state_t   management_object_link_state )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_ethernet_multiple_management_object_link_state(olt_id, onu_id, number_of_entries, management_object_link_state));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_management_object_link_state"

PON_STATUS PON_CTC_STACK_get_ethernet_management_object_link_state ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   CTC_STACK_link_state_t              *link_state)
{
    short int								            result=CTC_STACK_EXIT_OK;
    CTC_STACK_ethernet_management_object_link_state_t   management_object_link_state;
	unsigned char							            number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_link_state[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_link_state[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_link_state[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_link_state[0].management_object_index.port_type        = management_object_index.port_type;


    result = PON_CTC_STACK_get_ethernet_multiple_management_object_link_state(olt_id, onu_id, &number_of_entries, management_object_link_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*link_state) = management_object_link_state[0].link_state;

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_all_port_link_state"

PON_STATUS PON_CTC_STACK_get_ethernet_all_port_link_state ( 
           const PON_olt_id_t					   olt_id, 
           const PON_onu_id_t					   onu_id,
		   unsigned char						  *number_of_entries,
		   CTC_STACK_ethernet_ports_link_state_t   ports_link_state )
{
    short int								            result;
    unsigned char                                       i;   
    CTC_STACK_ethernet_management_object_link_state_t   management_object_link_state;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_link_state[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_link_state[0].management_object_index.slot_number      = 0x00;
    management_object_link_state[0].management_object_index.frame_number     = 0x00;
    management_object_link_state[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_ethernet_multiple_management_object_link_state(olt_id, onu_id, number_of_entries, management_object_link_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_link_state[i].port_number         = (unsigned char)(management_object_link_state[i].management_object_index.port_number);
        ports_link_state[i].link_state          = management_object_link_state[i].link_state;
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_link_state"

PON_STATUS PON_CTC_STACK_get_ethernet_link_state ( 
           const PON_olt_id_t	    olt_id, 
           const PON_onu_id_t	    onu_id,
		   const unsigned char	    port_number,
		   CTC_STACK_link_state_t  *link_state)
{
    short int								            result=CTC_STACK_EXIT_OK;
    CTC_STACK_ethernet_management_object_link_state_t   management_object_link_state;
	unsigned char							            number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_link_state[0].management_object_index.port_number      = port_number;
    management_object_link_state[0].management_object_index.slot_number      = 0x00;
    management_object_link_state[0].management_object_index.frame_number     = 0x00;
    management_object_link_state[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_ethernet_multiple_management_object_link_state(olt_id, onu_id, &number_of_entries, management_object_link_state);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    (*link_state) = management_object_link_state[0].link_state;

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_ethport_multiple_management_object_statistic_state"

PON_STATUS PON_CTC_STACK_set_ethport_multiple_management_object_statistic_state ( 
           const PON_olt_id_t							             olt_id, 
           const PON_onu_id_t							             onu_id,
		   const unsigned char							             number_of_entries,
		   const CTC_STACK_management_object_statistic_state_t management_object_statistic_state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_STATISTIC_STATE;
	unsigned char								received_num_of_entries=number_of_entries;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char								local_number_of_entries = 0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for(i = 0; i < number_of_entries; i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_statistic_state[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id]) 
		}
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_statistic_state[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_statistic_state
									( 1, &management_object_statistic_state[i].state, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_statistic_state[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_statistic_state
									( 1, &management_object_statistic_state[i].state, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}


	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		received_total_length += received_left_length;

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_statistic_state[i].management_object_index.port_number)
		}


		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        				  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
	}

	return result;
        
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethport_multiple_management_object_statistic_state"

PON_STATUS PON_CTC_STACK_get_ethport_multiple_management_object_statistic_state (
           const PON_olt_id_t				                   olt_id,
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_management_object_statistic_state_t    management_object_statistic_state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_STATISTIC_STATE;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val,i,j;
	CTC_STACK_statistic_state_t						received_state;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;



    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_statistic_state[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])

            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_statistic_state[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_statistic_state[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_statistic_state[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_statistic_state
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &received_state, &ctc_ret_val,&received_left_length  );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;


		    management_object_statistic_state[i].state = received_state;

		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

            if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_statistic_state
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &received_state, &ctc_ret_val,&received_left_length  );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;

			management_object_statistic_state[i].management_object_index.port_number  = received_management_object.index.port_number;
			management_object_statistic_state[i].management_object_index.frame_number = received_management_object.index.frame_number;
			management_object_statistic_state[i].management_object_index.slot_number  = received_management_object.index.slot_number;
			management_object_statistic_state[i].management_object_index.port_type    = received_management_object.index.port_type;
			management_object_statistic_state[i].state	    	                   = received_state;

			i++;
		}
		(*number_of_entries) = i;
	}

	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_ethport_multiple_management_object_statistic_data"

PON_STATUS PON_CTC_STACK_set_ethport_multiple_management_object_statistic_data ( 
           const PON_olt_id_t							             olt_id, 
           const PON_onu_id_t							             onu_id,
		   const unsigned char							             number_of_entries,
		   const CTC_STACK_management_object_statistic_data_t management_object_statistic_data )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_STATISTIC_DATA;
	unsigned char								received_num_of_entries=number_of_entries;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char								local_number_of_entries = 0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for(i = 0; i < number_of_entries; i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_statistic_data[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id]) 
		}
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_statistic_data[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_statistic_data
									( 1, &management_object_statistic_data[i].data, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length, 0 );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_statistic_data[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_statistic_data
									( 1, &management_object_statistic_data[i].data, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length, 0 );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}


	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		received_total_length += received_left_length;

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_statistic_data[i].management_object_index.port_number)
		}


		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        				  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
	}

	return result;
        
}
#undef PONLOG_FUNCTION_NAME



static char test_statistic_data[]={
0x02, 0xc7, 0x00, 0xb2, 0x00, 
0x00, 0x00, 0x00, 0x1e,
0x00, 0x00, 0x00, 0x0a,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0xc7, 0x00, 0xb2, 0x10,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00
};

static char test_port_statistic_data[]={
0x02, 0x37, 0x00, 0x01,
0x04, 0x01, 0x00, 0x00,
0x01, 0xc7, 0x00, 0xb2, 0x00, 
0x00, 0x00, 0x00, 0x1e,
0x00, 0x00, 0x00, 0x0a,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0xc7, 0x00, 0xb2, 0x18,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x0b
};

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethport_multiple_management_object_statistic_data"

PON_STATUS PON_CTC_STACK_get_ethport_multiple_management_object_statistic_data (
           const PON_olt_id_t				                   olt_id,
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_management_object_statistic_data_t    management_object_statistic_data )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_STATISTIC_DATA;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val,i,j;
	CTC_STACK_statistic_data_t					recv_data;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;



    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_statistic_data[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])

            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_statistic_data[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			#if 0

			comparser_result = (short int)CTC_COMPOSE_statistic_data
									( 1, &management_object_statistic_data[i].data, COMPARSER_CTC_STATUS_SUCCESS, FALSE,
									  SEND_COMPARSER_BUFFER, &sent_length, 0 );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
			#else
			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
			#endif
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_statistic_data[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
		
#if 0
	received_length = sizeof(test_port_statistic_data);
	memcpy(received_oam_pdu_data, test_port_statistic_data, received_length);
#endif

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_statistic_data[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_statistic_data
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &recv_data, &ctc_ret_val,&received_left_length, 0 );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;


		    memcpy(&management_object_statistic_data[i].data, &recv_data, sizeof(CTC_STACK_statistic_data_t));

		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

            if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_statistic_data
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &recv_data, &ctc_ret_val,&received_left_length  );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;

			management_object_statistic_data[i].management_object_index.port_number  = received_management_object.index.port_number;
			management_object_statistic_data[i].management_object_index.frame_number = received_management_object.index.frame_number;
			management_object_statistic_data[i].management_object_index.slot_number  = received_management_object.index.slot_number;
			management_object_statistic_data[i].management_object_index.port_type    = received_management_object.index.port_type;
			memcpy(&management_object_statistic_data[i].data, &recv_data, sizeof(CTC_STACK_statistic_data_t));

			i++;
		}
		(*number_of_entries) = i;
	}

	return result;

}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethport_multiple_management_object_statistic_history_data"

PON_STATUS PON_CTC_STACK_get_ethport_multiple_management_object_statistic_history_data (
           const PON_olt_id_t				                   olt_id,
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_management_object_statistic_data_t    management_object_statistic_data )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_STATISTIC_HIS_DATA;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val,i,j;
	CTC_STACK_statistic_data_t					recv_data;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;



    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_statistic_data[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])

            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_statistic_data[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			#if 0

			comparser_result = (short int)CTC_COMPOSE_statistic_data
									( 1, &management_object_statistic_data[i].data, COMPARSER_CTC_STATUS_SUCCESS, FALSE,
									  SEND_COMPARSER_BUFFER, &sent_length, 0 );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
			#else
			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
			#endif
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_statistic_data[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
		
#if 0
	received_length = sizeof(test_port_statistic_data);
	memcpy(received_oam_pdu_data, test_port_statistic_data, received_length);
#endif

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_statistic_data[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_statistic_history_data
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &recv_data, &ctc_ret_val,&received_left_length, 0 );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;


		    memcpy(&management_object_statistic_data[i].data, &recv_data, sizeof(CTC_STACK_statistic_data_t));

		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

            if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_statistic_history_data
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &recv_data, &ctc_ret_val,&received_left_length, 0 );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;

			management_object_statistic_data[i].management_object_index.port_number  = received_management_object.index.port_number;
			management_object_statistic_data[i].management_object_index.frame_number = received_management_object.index.frame_number;
			management_object_statistic_data[i].management_object_index.slot_number  = received_management_object.index.slot_number;
			management_object_statistic_data[i].management_object_index.port_type    = received_management_object.index.port_type;
			memcpy(&management_object_statistic_data[i].data, &recv_data, sizeof(CTC_STACK_statistic_data_t));

			i++;
		}
		(*number_of_entries) = i;
	}

	return result;

}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_multiple_management_object_tag_oper"

PON_STATUS PON_CTC_STACK_set_multicast_multiple_management_object_tag_oper ( 
           const PON_olt_id_t							             olt_id, 
           const PON_onu_id_t							             onu_id,
		   const unsigned char							             number_of_entries,
		   const CTC_STACK_multicast_management_object_tag_oper_t    management_object_tag_oper )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_MULTICAST_TAG_STRIP;
	unsigned char								received_num_of_entries=number_of_entries;
	unsigned char								ctc_ret_val,i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char								local_number_of_entries = 0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for(i = 0; i < number_of_entries; i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_tag_oper[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id]) 
		}
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_tag_oper[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_multicast_tag_strip
									( 1, management_object_tag_oper[i].tag_oper, management_object_tag_oper[i].multicast_vlan_switching, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_tag_oper[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_multicast_tag_strip
									( 1, management_object_tag_oper[0].tag_oper, management_object_tag_oper[0].multicast_vlan_switching, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}


	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		received_total_length += received_left_length;

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_tag_oper[i].management_object_index.port_number)
		}


		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        				  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
	}

	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_multiple_management_object_tag_oper"

PON_STATUS PON_CTC_STACK_get_multicast_multiple_management_object_tag_oper ( 
           const PON_olt_id_t				                   olt_id, 
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_multicast_management_object_tag_oper_t    management_object_tag_oper )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_MULTICAST_TAG_STRIP;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val,i,j;
	CTC_STACK_tag_oper_t						received_tag_oper = CTC_NO_STRIP_VLAN_TAG;
    CTC_STACK_multicast_vlan_switching_t        received_multicast_vlan_switching;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			    management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					    sent_management_object;
	CTC_management_object_t					    received_management_object;


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_tag_oper[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_tag_oper[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_tag_oper[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_tag_oper[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_multicast_tag_strip
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &received_tag_oper, &received_multicast_vlan_switching, &ctc_ret_val,&received_left_length  );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;

			
			management_object_tag_oper[i].tag_oper = received_tag_oper;
            
            if(received_tag_oper == CTC_TRANSLATE_VLAN_TAG)
            {
                management_object_tag_oper[i].multicast_vlan_switching.number_of_entries = received_multicast_vlan_switching.number_of_entries;
                
                for(j=0; j < received_multicast_vlan_switching.number_of_entries; j++)
                {
                    management_object_tag_oper[i].multicast_vlan_switching.entries[j].multicast_vlan = received_multicast_vlan_switching.entries[j].multicast_vlan;
                    management_object_tag_oper[i].multicast_vlan_switching.entries[j].iptv_user_vlan = received_multicast_vlan_switching.entries[j].iptv_user_vlan;
                }
            }
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			
            if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_multicast_tag_strip
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &received_tag_oper, &received_multicast_vlan_switching, &ctc_ret_val,&received_left_length  );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;

			management_object_tag_oper[i].management_object_index.port_number  = received_management_object.index.port_number;
    		management_object_tag_oper[i].management_object_index.frame_number = received_management_object.index.frame_number;
    		management_object_tag_oper[i].management_object_index.slot_number  = received_management_object.index.slot_number;
    		management_object_tag_oper[i].management_object_index.port_type    = received_management_object.index.port_type;
            management_object_tag_oper[i].tag_oper	    	                   = received_tag_oper;

            if(received_tag_oper == CTC_TRANSLATE_VLAN_TAG)
            {
                management_object_tag_oper[i].multicast_vlan_switching.number_of_entries = received_multicast_vlan_switching.number_of_entries;
                
                for(j=0; j < received_multicast_vlan_switching.number_of_entries; j++)
                {
                    management_object_tag_oper[i].multicast_vlan_switching.entries[j].multicast_vlan = received_multicast_vlan_switching.entries[j].multicast_vlan;
                    management_object_tag_oper[i].multicast_vlan_switching.entries[j].iptv_user_vlan = received_multicast_vlan_switching.entries[j].iptv_user_vlan;
                }
            }
            
			i++;
		}
		(*number_of_entries) = i;
	}
    
	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_all_management_object_tag_oper"

PON_STATUS PON_CTC_STACK_get_multicast_all_management_object_tag_oper ( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_multicast_management_object_tag_oper_t    management_object_info )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;

    return (PON_CTC_STACK_get_multicast_multiple_management_object_tag_oper(olt_id, onu_id, number_of_entries, management_object_info));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_management_object_tag_oper"

PON_STATUS PON_CTC_STACK_set_multicast_management_object_tag_oper ( 
           const PON_olt_id_t	                       olt_id, 
           const PON_onu_id_t	                       onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_tag_oper_t                  tag_oper,
           const CTC_STACK_multicast_vlan_switching_t  multicast_vlan_switching)
{
    short int								            result=CTC_STACK_EXIT_OK;
    CTC_STACK_multicast_management_object_tag_oper_t    management_object_tag_oper;
	unsigned char						                number_of_entries, i;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }
                                                                        
    management_object_tag_oper[0].management_object_index.port_number      = management_object_index.port_number; 
    management_object_tag_oper[0].management_object_index.slot_number      = management_object_index.slot_number; 
    management_object_tag_oper[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_tag_oper[0].management_object_index.port_type        = management_object_index.port_type;   

	management_object_tag_oper[0].tag_oper                                 = tag_oper;

    if(tag_oper == CTC_TRANSLATE_VLAN_TAG)
    {
        management_object_tag_oper[0].multicast_vlan_switching.number_of_entries = multicast_vlan_switching.number_of_entries;
        
        for( i = 0; i < multicast_vlan_switching.number_of_entries; i++)
        {
            management_object_tag_oper[0].multicast_vlan_switching.entries[i] = multicast_vlan_switching.entries[i];
        }
    }

	result = PON_CTC_STACK_set_multicast_multiple_management_object_tag_oper(olt_id, onu_id, number_of_entries, management_object_tag_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_management_object_tag_oper"

PON_STATUS PON_CTC_STACK_get_multicast_management_object_tag_oper ( 
           const PON_olt_id_t	                  olt_id, 
           const PON_onu_id_t	                  onu_id,
		   const CTC_management_object_index_t    management_object_index,
		   CTC_STACK_tag_oper_t                  *tag_oper,
		   CTC_STACK_multicast_vlan_switching_t  *multicast_vlan_switching)
{
    short int								            result=CTC_STACK_EXIT_OK;
    CTC_STACK_multicast_management_object_tag_oper_t    management_object_tag_oper;
	unsigned char							            number_of_entries = 1, i;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_tag_oper[0].management_object_index.port_number      = management_object_index.port_number; 
    management_object_tag_oper[0].management_object_index.slot_number      = management_object_index.slot_number; 
    management_object_tag_oper[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_tag_oper[0].management_object_index.port_type        = management_object_index.port_type;   


    result = PON_CTC_STACK_get_multicast_multiple_management_object_tag_oper(olt_id, onu_id, &number_of_entries, management_object_tag_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	(*tag_oper) = management_object_tag_oper[0].tag_oper;

    if((*tag_oper) == CTC_TRANSLATE_VLAN_TAG)
    {        
        multicast_vlan_switching->number_of_entries = management_object_tag_oper[0].multicast_vlan_switching.number_of_entries;
        
        for( i = 0; i < multicast_vlan_switching->number_of_entries; i++)
        {
            multicast_vlan_switching->entries[i] = management_object_tag_oper[0].multicast_vlan_switching.entries[i];
        }
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_all_port_tag_oper"

PON_STATUS PON_CTC_STACK_get_multicast_all_port_tag_oper ( 
           const PON_olt_id_t					   olt_id, 
           const PON_onu_id_t					   onu_id,
		   unsigned char						  *number_of_entries,
		   CTC_STACK_multicast_ports_tag_oper_t    ports_info )
{
    short int								            result;
    unsigned char                                       i,j;   
    CTC_STACK_multicast_management_object_tag_oper_t    management_object_tag_oper;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_tag_oper[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_tag_oper[0].management_object_index.slot_number      = 0x00;
    management_object_tag_oper[0].management_object_index.frame_number     = 0x00;
    management_object_tag_oper[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_multicast_multiple_management_object_tag_oper(olt_id, onu_id, number_of_entries, management_object_tag_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_info[i].port_number         = (unsigned char)(management_object_tag_oper[i].management_object_index.port_number);
        ports_info[i].tag_oper            = management_object_tag_oper[i].tag_oper;
        
        if(ports_info[i].tag_oper == CTC_TRANSLATE_VLAN_TAG)
        {
            ports_info[i].multicast_vlan_switching.number_of_entries = management_object_tag_oper[i].multicast_vlan_switching.number_of_entries;
            
            for( j = 0; j < ports_info[i].multicast_vlan_switching.number_of_entries; j++)
            {
                ports_info[i].multicast_vlan_switching.entries[j] = management_object_tag_oper[i].multicast_vlan_switching.entries[j];
            }
        }
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_tag_oper"

PON_STATUS PON_CTC_STACK_set_multicast_tag_oper ( 
           const PON_olt_id_t	                       olt_id, 
           const PON_onu_id_t	                       onu_id,
		   const unsigned char	                       port_number,
		   const CTC_STACK_tag_oper_t                  tag_oper,
           const CTC_STACK_multicast_vlan_switching_t  multicast_vlan_switching)
{
    short int								            result=CTC_STACK_EXIT_OK;
    CTC_STACK_multicast_management_object_tag_oper_t    management_object_tag_oper;
	unsigned char						                number_of_entries, i;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_tag_oper[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_tag_oper[0].management_object_index.port_number  = port_number;
    }


    management_object_tag_oper[0].management_object_index.slot_number      = 0x00;
    management_object_tag_oper[0].management_object_index.frame_number     = 0x00;
    management_object_tag_oper[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

	management_object_tag_oper[0].tag_oper                                 = tag_oper;

    if(tag_oper == CTC_TRANSLATE_VLAN_TAG)
    {
        management_object_tag_oper[0].multicast_vlan_switching.number_of_entries = multicast_vlan_switching.number_of_entries;
        
        for( i = 0; i < multicast_vlan_switching.number_of_entries; i++)
        {
            management_object_tag_oper[0].multicast_vlan_switching.entries[i] = multicast_vlan_switching.entries[i];
        }
    }

	result = PON_CTC_STACK_set_multicast_multiple_management_object_tag_oper(olt_id, onu_id, number_of_entries, management_object_tag_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_tag_oper"

PON_STATUS PON_CTC_STACK_get_multicast_tag_oper ( 
           const PON_olt_id_t	                  olt_id, 
           const PON_onu_id_t	                  onu_id,
		   const unsigned char	                  port_number,
		   CTC_STACK_tag_oper_t                  *tag_oper,
		   CTC_STACK_multicast_vlan_switching_t  *multicast_vlan_switching)
{
    short int								            result=CTC_STACK_EXIT_OK;
    CTC_STACK_multicast_management_object_tag_oper_t    management_object_tag_oper;
	unsigned char							            number_of_entries = 1, i;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    
    management_object_tag_oper[0].management_object_index.port_number      = port_number;
    management_object_tag_oper[0].management_object_index.slot_number      = 0x00;
    management_object_tag_oper[0].management_object_index.frame_number     = 0x00;
    management_object_tag_oper[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_multicast_multiple_management_object_tag_oper(olt_id, onu_id, &number_of_entries, management_object_tag_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	(*tag_oper) = management_object_tag_oper[0].tag_oper;

    if((*tag_oper) == CTC_TRANSLATE_VLAN_TAG)
    {        
        multicast_vlan_switching->number_of_entries = management_object_tag_oper[0].multicast_vlan_switching.number_of_entries;
        
        for( i = 0; i < multicast_vlan_switching->number_of_entries; i++)
        {
            multicast_vlan_switching->entries[i] = management_object_tag_oper[0].multicast_vlan_switching.entries[i];
        }
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_all_management_object_tag_strip"

PON_STATUS PON_CTC_STACK_get_multicast_all_management_object_tag_strip ( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_multicast_management_object_tag_strip_t   management_object_info )
{
    CTC_STACK_multicast_management_object_tag_oper_t    management_object_tag_oper;
    short int ret;
    unsigned char i;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id)

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_tag_oper[0].management_object_index =  management_object_info[0].management_object_index;

    ret = PON_CTC_STACK_get_multicast_multiple_management_object_tag_oper(olt_id, onu_id, number_of_entries, management_object_tag_oper);
    
    for(i=0; i < (*number_of_entries); i++)
    {
        management_object_info[i].management_object_index  = management_object_tag_oper[i].management_object_index;
        
        if(management_object_tag_oper[i].tag_oper == CTC_STRIP_VLAN_TAG)
        {
            management_object_info[i].tag_strip = TRUE;
        }
        else if(management_object_tag_oper[i].tag_oper == CTC_NO_STRIP_VLAN_TAG)
        {
            management_object_info[i].tag_strip = FALSE;
        }
        else
        {
            return CTC_STACK_NOT_IMPLEMENTED;
        }
    }

    return ret;    
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_all_port_tag_strip"

PON_STATUS PON_CTC_STACK_get_multicast_all_port_tag_strip ( 
           const PON_olt_id_t					   olt_id, 
           const PON_onu_id_t					   onu_id,
		   unsigned char						  *number_of_entries,
		   CTC_STACK_multicast_ports_tag_strip_t   ports_info )
{
    short int								            result;
    unsigned char                                       i;   
	CTC_STACK_multicast_management_object_tag_oper_t	management_object_info_oper;
	
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	(*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_info_oper[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_info_oper[0].management_object_index.slot_number      = 0x00;
    management_object_info_oper[0].management_object_index.frame_number     = 0x00;
    management_object_info_oper[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_multicast_multiple_management_object_tag_oper(olt_id, onu_id, number_of_entries, management_object_info_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for(i=0; i < (*number_of_entries); i++)
    {
        ports_info[i].port_number  = (unsigned char)management_object_info_oper[i].management_object_index.port_number;
        
        if(management_object_info_oper[i].tag_oper == CTC_STRIP_VLAN_TAG)
        {
            ports_info[i].tag_strip = TRUE;
        }
        else if(management_object_info_oper[i].tag_oper == CTC_NO_STRIP_VLAN_TAG)
        {
            ports_info[i].tag_strip = FALSE;
        }
        else
        {
            return CTC_STACK_NOT_IMPLEMENTED;
        }
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_management_object_tag_strip"

PON_STATUS PON_CTC_STACK_set_multicast_management_object_tag_strip ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   const bool		                    tag_strip)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_multicast_management_object_tag_oper_t	management_object_info_oper;
	unsigned char						                number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    management_object_info_oper[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_info_oper[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_info_oper[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_info_oper[0].management_object_index.port_type        = management_object_index.port_type;

    if(tag_strip)
    {
        management_object_info_oper[0].tag_oper = CTC_STRIP_VLAN_TAG;
    }
    else
    {
        management_object_info_oper[0].tag_oper = CTC_NO_STRIP_VLAN_TAG;
    }


	result = PON_CTC_STACK_set_multicast_multiple_management_object_tag_oper(olt_id, onu_id, number_of_entries, management_object_info_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_management_object_tag_strip"

PON_STATUS PON_CTC_STACK_get_multicast_management_object_tag_strip ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   bool				                   *tag_strip)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_multicast_management_object_tag_oper_t	management_object_info_oper;
	unsigned char							            number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_info_oper[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_info_oper[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_info_oper[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_info_oper[0].management_object_index.port_type        = management_object_index.port_type;


    result = PON_CTC_STACK_get_multicast_multiple_management_object_tag_oper(olt_id, onu_id, &number_of_entries, management_object_info_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    if(management_object_info_oper[0].tag_oper == CTC_NO_STRIP_VLAN_TAG)
    {
        (*tag_strip) = FALSE;
    }
    else if(management_object_info_oper[0].tag_oper == CTC_STRIP_VLAN_TAG)
    {
        (*tag_strip) = TRUE;
    }
    else
    {
        return CTC_STACK_NOT_IMPLEMENTED;
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_tag_strip"

PON_STATUS PON_CTC_STACK_set_multicast_tag_strip ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const bool		     tag_strip)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_multicast_management_object_tag_oper_t	management_object_info_oper;
	unsigned char						                number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_info_oper[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_info_oper[0].management_object_index.port_number  = port_number;
    }


    management_object_info_oper[0].management_object_index.slot_number      = 0x00;
    management_object_info_oper[0].management_object_index.frame_number     = 0x00;
    management_object_info_oper[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    if(tag_strip)
    {
        management_object_info_oper[0].tag_oper = CTC_STRIP_VLAN_TAG;
    }
    else
    {
        management_object_info_oper[0].tag_oper = CTC_NO_STRIP_VLAN_TAG;
    }


	result = PON_CTC_STACK_set_multicast_multiple_management_object_tag_oper(olt_id, onu_id, number_of_entries, management_object_info_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_tag_strip"

PON_STATUS PON_CTC_STACK_get_multicast_tag_strip ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   bool				    *tag_strip)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_multicast_management_object_tag_oper_t	management_object_info_oper;
	unsigned char							            number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_info_oper[0].management_object_index.port_number      = port_number;
    management_object_info_oper[0].management_object_index.slot_number      = 0x00;
    management_object_info_oper[0].management_object_index.frame_number     = 0x00;
    management_object_info_oper[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_multicast_multiple_management_object_tag_oper(olt_id, onu_id, &number_of_entries, management_object_info_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    if(management_object_info_oper[0].tag_oper == CTC_NO_STRIP_VLAN_TAG)
    {
        (*tag_strip) = FALSE;
    }
    else if(management_object_info_oper[0].tag_oper == CTC_STRIP_VLAN_TAG)
    {
        (*tag_strip) = TRUE;
    }
    else
    {
        return CTC_STACK_NOT_IMPLEMENTED;
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_ethport_statistic_state"

PON_STATUS PON_CTC_STACK_set_ethport_statistic_state( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const CTC_STACK_statistic_state_t state)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_statistic_state_t management_object_info_oper;
	unsigned char						                number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_info_oper[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_info_oper[0].management_object_index.port_number  = port_number;
    }


    management_object_info_oper[0].management_object_index.slot_number      = 0x00;
    management_object_info_oper[0].management_object_index.frame_number     = 0x00;
    management_object_info_oper[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

    management_object_info_oper[0].state = state;

	result = PON_CTC_STACK_set_ethport_multiple_management_object_statistic_state(olt_id, onu_id, number_of_entries, management_object_info_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethport_statistic_state"

PON_STATUS PON_CTC_STACK_get_ethport_statistic_state (
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   CTC_STACK_statistic_state_t *state)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_statistic_state_t	management_object_info_oper;
	unsigned char							            number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_info_oper[0].management_object_index.port_number      = port_number;
    management_object_info_oper[0].management_object_index.slot_number      = 0x00;
    management_object_info_oper[0].management_object_index.frame_number     = 0x00;
    management_object_info_oper[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_ethport_multiple_management_object_statistic_state(olt_id, onu_id, &number_of_entries, management_object_info_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	*state = management_object_info_oper[0].state;

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_ethport_statistic_data"

PON_STATUS PON_CTC_STACK_set_ethport_statistic_data( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number
)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_statistic_data_t management_object_info_oper;
	unsigned char						                number_of_entries;

	CTC_STACK_statistic_data_t data;

	memset(&data, 0, sizeof(CTC_STACK_statistic_data_t ));

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_info_oper[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_info_oper[0].management_object_index.port_number  = port_number;
    }


    management_object_info_oper[0].management_object_index.slot_number      = 0x00;
    management_object_info_oper[0].management_object_index.frame_number     = 0x00;
    management_object_info_oper[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

    memcpy(&management_object_info_oper[0].data,&data, sizeof(CTC_STACK_statistic_data_t));

	result = PON_CTC_STACK_set_ethport_multiple_management_object_statistic_data(olt_id, onu_id, number_of_entries, management_object_info_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethport_statistic_current_data"

PON_STATUS PON_CTC_STACK_get_ethport_statistic_current_data (
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   CTC_STACK_statistic_data_t *data)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_statistic_data_t management_object_info_oper;
	unsigned char							            number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_info_oper[0].management_object_index.port_number      = port_number;
    management_object_info_oper[0].management_object_index.slot_number      = 0x00;
    management_object_info_oper[0].management_object_index.frame_number     = 0x00;
    management_object_info_oper[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	memset(&management_object_info_oper[0].data, 0, sizeof(CTC_STACK_statistic_data_t));


    result = PON_CTC_STACK_get_ethport_multiple_management_object_statistic_data(olt_id, onu_id, &number_of_entries, management_object_info_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	memcpy(data, &management_object_info_oper[0].data, sizeof(CTC_STACK_statistic_data_t));

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethport_statistic_history_data"

PON_STATUS PON_CTC_STACK_get_ethport_statistic_history_data (
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   CTC_STACK_statistic_data_t *data)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_statistic_data_t management_object_info_oper;
	unsigned char							            number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_info_oper[0].management_object_index.port_number      = port_number;
    management_object_info_oper[0].management_object_index.slot_number      = 0x00;
    management_object_info_oper[0].management_object_index.frame_number     = 0x00;
    management_object_info_oper[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	memset(&management_object_info_oper[0].data, 0, sizeof(CTC_STACK_statistic_data_t));


    result = PON_CTC_STACK_get_ethport_multiple_management_object_statistic_history_data(olt_id, onu_id, &number_of_entries, management_object_info_oper);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	memcpy(data, &management_object_info_oper[0].data, sizeof(CTC_STACK_statistic_data_t));

	return result;
}
#undef PONLOG_FUNCTION_NAME



static int int64u_sum(INT64U *a, INT64U *b)
{
	int rc = -1;

	if(!a || !b)
		return rc;

	a->msb += b->msb;
	
	if(b->lsb > 0xffffffff-a->lsb)
	{
		a->msb += 1;
		a->lsb = a->lsb-(0xffffffff-b->lsb);
	}
	else
		a->lsb += b->lsb;

	rc = 0;	

	return rc;
}

static int onu_statistic_sum(CTC_STACK_statistic_data_t * a, CTC_STACK_statistic_data_t * b)
{
	int rc = -1;

	int typelen = sizeof(INT64U);

	int n = sizeof(CTC_STACK_statistic_data_t)/typelen, i;

	char *pa = (char*)a;
	char *pb = (char*)b;

	if(!(a && b))
		return rc;

	for(i=0; i<n; i++)
	{
		int64u_sum((INT64U*)pa, (INT64U*)pb);
		pa+=typelen;
		pb+=typelen;		
	}

	rc = 0;
	return rc;
}

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethport_statistic_data"
PON_STATUS PON_CTC_STACK_get_ethport_statistic_data (
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   CTC_STACK_statistic_data_t *data)
{

	CTC_STACK_statistic_data_t cdata, cdata1, hisdata, hisdata1;
	
    short int result=CTC_STACK_EXIT_OK;

	int rc = 0;

	memset(&cdata, 0, sizeof(CTC_STACK_statistic_data_t));
	memset(&hisdata, 0, sizeof(CTC_STACK_statistic_data_t));
	memset(&cdata1, 0, sizeof(CTC_STACK_statistic_data_t));
	memset(&hisdata1, 0, sizeof(CTC_STACK_statistic_data_t));
	
	rc = PON_CTC_STACK_get_ethport_statistic_history_data(olt_id, onu_id, port_number, &hisdata);

	rc |= PON_CTC_STACK_get_ethport_statistic_current_data(olt_id, onu_id, port_number, &cdata);

	rc |= PON_CTC_STACK_get_ethport_statistic_history_data(olt_id, onu_id, port_number, &hisdata1);


	if( rc != CTC_STACK_EXIT_OK)
		result = CTC_STACK_EXIT_ERROR;
	else if(memcmp(&hisdata, &hisdata1, sizeof(CTC_STACK_statistic_data_t)) == 0)
	{
		onu_statistic_sum(&hisdata, &cdata);
		*data = hisdata;
	}
	else
	{
		rc |= PON_CTC_STACK_get_ethport_statistic_current_data(olt_id, onu_id, port_number, &cdata1);	
		onu_statistic_sum(&hisdata1, &cdata1);
		*data = hisdata1;
	}
	
	return result;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multicast_switch"

PON_STATUS PON_CTC_STACK_set_multicast_switch ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   const CTC_STACK_multicast_protocol_t   multicast_protocol )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_MULTICAST_SWITCH;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	CTC_STACK_multicast_protocol_t				received_multicast_protocol = 0;
	unsigned char								received_port_number=0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;


    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	comparser_result = (short int)CTC_COMPOSE_multicast_switch
							( 1, &multicast_protocol, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;
		


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;


	return result;
        
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multicast_switch"

PON_STATUS PON_CTC_STACK_get_multicast_switch ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   CTC_STACK_multicast_protocol_t  *multicast_protocol )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_MULTICAST_SWITCH;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	CTC_STACK_multicast_protocol_t				received_multicast_protocol = 0;
	unsigned char								received_port_number = 0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;


    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	comparser_result = (short int)CTC_PARSE_multicast_switch
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              &received_multicast_protocol, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;

	
	(*multicast_protocol) = received_multicast_protocol;
	
    
	return result;

}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_statistic_state"

PON_STATUS PON_CTC_STACK_set_statistic_state ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   const CTC_STACK_statistic_state_t state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_STATISTIC_STATE;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	CTC_STACK_multicast_protocol_t				received_multicast_protocol = 0;
	unsigned char								received_port_number=0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;


    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	comparser_result = (short int)CTC_COMPOSE_statistic_state
							( 1, &state, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;
		


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;


	return result;
        
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_statistic_state"

PON_STATUS PON_CTC_STACK_get_statistic_state( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   CTC_STACK_statistic_state_t *state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_STATISTIC_STATE;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	CTC_STACK_statistic_state_t				received_state;
	unsigned char								received_port_number = 0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;


    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	comparser_result = (short int)CTC_PARSE_statistic_state
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              &received_state, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;

	
	(*state) = received_state;
	
    
	return result;

}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_statistic_data"

PON_STATUS PON_CTC_STACK_set_statistic_data (
		const PON_olt_id_t olt_id,
		const PON_onu_id_t onu_id,
		const unsigned long circle,
		const unsigned long time )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_STATISTIC_DATA;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned char								received_port_number=0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;

	CTC_STACK_statistic_data_t    dataset;



	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC


    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	memset(&dataset, 0, sizeof(dataset));

	comparser_result = (short int)CTC_COMPOSE_statistic_data
							( 1, &dataset, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length, 1 );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



    result = Send_receive_vendor_extension_message
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;


	return result;

}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_statistic_data"

PON_STATUS PON_CTC_STACK_get_statistic_data(
           const PON_olt_id_t				olt_id,
           const PON_onu_id_t				onu_id,
		   CTC_STACK_statistic_data_t *data )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_STATISTIC_DATA;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	CTC_STACK_statistic_data_t				received_data;
	unsigned char								received_port_number = 0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;



	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	memset(&received_data, 0, sizeof(CTC_STACK_statistic_data_t));

	comparser_result = (short int)CTC_COMPOSE_statistic_data
							( 1, &received_data, COMPARSER_CTC_STATUS_SUCCESS, FALSE,
							  SEND_COMPARSER_BUFFER, &sent_length, 1 );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    /*CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC*/

	received_length = sizeof(test_statistic_data);
	memcpy(received_oam_pdu_data, test_statistic_data, received_length);

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	comparser_result = (short int)CTC_PARSE_statistic_data
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              data, &ctc_ret_val, &received_left_length, 1 );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;

	return result;

}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_fast_leave_admin_control"
PON_STATUS PON_CTC_STACK_set_fast_leave_admin_control ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   CTC_STACK_fast_leave_admin_state_t     fast_leave_admin_state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ACTION;
	unsigned short								expected_leaf  =  CTC_EX_ACTION_FAST_LEAVE_ADMIN_CONTROL;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned char								received_port_number=0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;


    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	comparser_result = (short int)CTC_COMPOSE_fast_leave_admin_control
							( 1, &fast_leave_admin_state, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;
		


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;


	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_fast_leave_admin_state"
PON_STATUS PON_CTC_STACK_get_fast_leave_admin_state ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   CTC_STACK_fast_leave_admin_state_t	*fast_leave_admin_state )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_FAST_LEAVE_ADMIN_STATE;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	CTC_STACK_fast_leave_admin_state_t			state = 0;
	unsigned char								received_port_number = 0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;


    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	comparser_result = (short int)CTC_PARSE_fast_leave_admin_state
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              &state, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;

	
	(*fast_leave_admin_state) = state;
	
    
	return result;

}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_fast_leave_ability"
PON_STATUS PON_CTC_STACK_get_fast_leave_ability ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   CTC_STACK_fast_leave_ability_t		*fast_leave_ability )
{
    short int									result,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_FAST_LEAVE_ABILITY;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	unsigned char								received_port_number = 0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;


    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	comparser_result = (short int)CTC_PARSE_fast_leave_ability
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              fast_leave_ability, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;

	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multiple_ethernet_management_object_ds_rate_limiting"
PON_STATUS PON_CTC_STACK_set_multiple_ethernet_management_object_ds_rate_limiting ( 
           const PON_olt_id_t									            olt_id, 
           const PON_onu_id_t									            onu_id,
		   const unsigned char									            number_of_entries,
		   const CTC_STACK_ethernet_management_object_ds_rate_limiting_t	management_object_ds_rate_limiting )
{
    short int								result,comparser_result;
    unsigned short							received_left_length = 0, sent_length=ETHERNET_MTU, received_length;
	unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		= OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							expected_opcode	= OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_branch	= OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	= CTC_EX_VAR_ETH_PORT_DS_RATE_LIMITING;
	unsigned char							received_num_of_entries=number_of_entries;
	unsigned char							ctc_ret_val,i;
	unsigned short							max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char							local_number_of_entries=0;
    CTC_STACK_ethernet_ports_ds_rate_limiting_t	ports_ds_rate_limiting;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;


    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for(i = 0; i < number_of_entries; i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_ds_rate_limiting[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
		}
	}


    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_ds_rate_limiting[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

            ports_ds_rate_limiting[i].port_number = (unsigned char)(management_object_ds_rate_limiting[i].management_object_index.port_number);
            ports_ds_rate_limiting[i].CIR         = management_object_ds_rate_limiting[i].CIR;
            ports_ds_rate_limiting[i].PIR         = management_object_ds_rate_limiting[i].PIR;
            ports_ds_rate_limiting[i].state       = management_object_ds_rate_limiting[i].state;

            comparser_result = (short int)CTC_COMPOSE_ethernet_port_ds_rate_limiting
									( 1, &ports_ds_rate_limiting[i], COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_ds_rate_limiting[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

            ports_ds_rate_limiting[0].port_number = (unsigned char)(management_object_ds_rate_limiting[0].management_object_index.port_number);
            ports_ds_rate_limiting[0].CIR         = management_object_ds_rate_limiting[0].CIR;
            ports_ds_rate_limiting[0].PIR         = management_object_ds_rate_limiting[0].PIR;
            ports_ds_rate_limiting[0].state       = management_object_ds_rate_limiting[0].state;

			comparser_result = (short int)CTC_COMPOSE_ethernet_port_ds_rate_limiting
									( 1, &ports_ds_rate_limiting[0], COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}


	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


		received_total_length += received_left_length;

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_ds_rate_limiting[i].management_object_index.port_number)
		}


		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_branch,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
		
	}
	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_ethernet_management_object_ds_rate_limiting"
PON_STATUS PON_CTC_STACK_set_ethernet_management_object_ds_rate_limiting ( 
           const PON_olt_id_t										 olt_id, 
           const PON_onu_id_t										 onu_id,
		   const CTC_management_object_index_t                       management_object_index,
		   const CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting)
{
    short int								                result=CTC_STACK_EXIT_OK;
    CTC_STACK_ethernet_management_object_ds_rate_limiting_t	management_object_ds_rate_limiting;
    unsigned char						                    number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    management_object_ds_rate_limiting[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_ds_rate_limiting[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_ds_rate_limiting[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_ds_rate_limiting[0].management_object_index.port_type        = management_object_index.port_type;

	management_object_ds_rate_limiting[0].state		    = port_ds_rate_limiting->state;
	management_object_ds_rate_limiting[0].CIR			= port_ds_rate_limiting->CIR;
	management_object_ds_rate_limiting[0].PIR			= port_ds_rate_limiting->PIR;


    result = PON_CTC_STACK_set_multiple_ethernet_management_object_ds_rate_limiting(olt_id, onu_id, number_of_entries, management_object_ds_rate_limiting);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_ethernet_port_ds_rate_limiting"
PON_STATUS PON_CTC_STACK_set_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t										olt_id, 
           const PON_onu_id_t										onu_id,
		   const unsigned char										port_number,
		   const CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting)
{
    short int								                result=CTC_STACK_EXIT_OK;
    CTC_STACK_ethernet_management_object_ds_rate_limiting_t	management_object_ds_rate_limiting;
    unsigned char						                    number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_ds_rate_limiting[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_ds_rate_limiting[0].management_object_index.port_number  = port_number;
    }


    management_object_ds_rate_limiting[0].management_object_index.slot_number      = 0x00;
    management_object_ds_rate_limiting[0].management_object_index.frame_number     = 0x00;
    management_object_ds_rate_limiting[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

	management_object_ds_rate_limiting[0].state		    = port_ds_rate_limiting->state;
	management_object_ds_rate_limiting[0].CIR			= port_ds_rate_limiting->CIR;
	management_object_ds_rate_limiting[0].PIR			= port_ds_rate_limiting->PIR;


    result = PON_CTC_STACK_set_multiple_ethernet_management_object_ds_rate_limiting(olt_id, onu_id, number_of_entries, management_object_ds_rate_limiting);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multiple_ethernet_management_object_ds_rate_limiting"
PON_STATUS PON_CTC_STACK_get_multiple_ethernet_management_object_ds_rate_limiting ( 
           const PON_olt_id_t							            olt_id, 
           const PON_onu_id_t							            onu_id,
		   unsigned char								           *number_of_entries,
		   CTC_STACK_ethernet_management_object_ds_rate_limiting_t	management_object_ds_rate_limiting )
{
    short int											result,comparser_result;
    unsigned short										received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char										sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char										received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char										send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char										expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char										expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short										expected_leaf	=  CTC_EX_VAR_ETH_PORT_DS_RATE_LIMITING;
	unsigned char										received_num_of_entries=0;
	unsigned char										ctc_ret_val,i;
    CTC_STACK_ethernet_ports_ds_rate_limiting_t	        ports_ds_rate_limiting;
	unsigned short										max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			            management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					            sent_management_object;
	CTC_management_object_t					            received_management_object;


	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;



	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_ds_rate_limiting[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_ds_rate_limiting[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_ds_rate_limiting[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}



    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_ds_rate_limiting[i].management_object_index.port_number)

			received_total_length += received_left_length;

			management_object_ds_rate_limiting[i].management_object_index.port_number         = received_management_object.index.port_number;
    		management_object_ds_rate_limiting[i].management_object_index.frame_number        = received_management_object.index.frame_number;
    		management_object_ds_rate_limiting[i].management_object_index.slot_number         = received_management_object.index.slot_number;
    		management_object_ds_rate_limiting[i].management_object_index.port_type           = received_management_object.index.port_type;

			comparser_result = (short int)CTC_PARSE_ethernet_port_ds_rate_limiting
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &ports_ds_rate_limiting[i], &ctc_ret_val ,&received_left_length );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
            
            management_object_ds_rate_limiting[i].CIR                                   =    ports_ds_rate_limiting[i].CIR;        
            management_object_ds_rate_limiting[i].PIR                                   =    ports_ds_rate_limiting[i].PIR;        
            management_object_ds_rate_limiting[i].state                                 =    ports_ds_rate_limiting[i].state;     

			received_total_length += received_left_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			
			received_total_length += received_left_length;

			management_object_ds_rate_limiting[i].management_object_index.port_number         = received_management_object.index.port_number;
    		management_object_ds_rate_limiting[i].management_object_index.frame_number        = received_management_object.index.frame_number;
    		management_object_ds_rate_limiting[i].management_object_index.slot_number         = received_management_object.index.slot_number;
    		management_object_ds_rate_limiting[i].management_object_index.port_type           = received_management_object.index.port_type;
			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_ethernet_port_ds_rate_limiting
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &ports_ds_rate_limiting[i], &ctc_ret_val ,&received_left_length );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
            
            management_object_ds_rate_limiting[i].CIR                                   =    ports_ds_rate_limiting[i].CIR;        
            management_object_ds_rate_limiting[i].PIR                                   =    ports_ds_rate_limiting[i].PIR;        
            management_object_ds_rate_limiting[i].state                                 =    ports_ds_rate_limiting[i].state;     


			received_total_length += received_left_length;
			
			i++;
		}

		(*number_of_entries) = i;
	}
    
	return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_all_ethernet_management_object_ds_rate_limiting"
PON_STATUS PON_CTC_STACK_get_all_ethernet_management_object_ds_rate_limiting ( 
           const PON_olt_id_t							            olt_id, 
           const PON_onu_id_t							            onu_id,
		   unsigned char								           *number_of_entries,
		   CTC_STACK_ethernet_management_object_ds_rate_limiting_t	management_object_ds_rate_limiting )
{
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_multiple_ethernet_management_object_ds_rate_limiting(olt_id, onu_id, number_of_entries, management_object_ds_rate_limiting));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_all_ethernet_port_ds_rate_limiting"
PON_STATUS PON_CTC_STACK_get_all_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   unsigned char								*number_of_entries,
		   CTC_STACK_ethernet_ports_ds_rate_limiting_t	ports_ds_rate_limiting )
{
    short int								                result;
    unsigned char                                           i;   
    CTC_STACK_ethernet_management_object_ds_rate_limiting_t	management_object_ds_rate_limiting;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    (*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_ds_rate_limiting[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_ds_rate_limiting[0].management_object_index.slot_number      = 0x00;
    management_object_ds_rate_limiting[0].management_object_index.frame_number     = 0x00;
    management_object_ds_rate_limiting[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_multiple_ethernet_management_object_ds_rate_limiting(olt_id, onu_id, number_of_entries, management_object_ds_rate_limiting);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_ds_rate_limiting[i].port_number         = (unsigned char)(management_object_ds_rate_limiting[i].management_object_index.port_number);
        ports_ds_rate_limiting[i].CIR                 = management_object_ds_rate_limiting[i].CIR;
        ports_ds_rate_limiting[i].PIR                 = management_object_ds_rate_limiting[i].PIR;
        ports_ds_rate_limiting[i].state               = management_object_ds_rate_limiting[i].state;
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_management_object_ds_rate_limiting"
PON_STATUS PON_CTC_STACK_get_ethernet_management_object_ds_rate_limiting ( 
           const PON_olt_id_t								olt_id, 
           const PON_onu_id_t								onu_id,
		   const CTC_management_object_index_t              management_object_index,
		   CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting )
{
    short int								                result=CTC_STACK_EXIT_OK;
    CTC_STACK_ethernet_management_object_ds_rate_limiting_t	management_object_ds_rate_limiting;
    unsigned char							                number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_ds_rate_limiting[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_ds_rate_limiting[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_ds_rate_limiting[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_ds_rate_limiting[0].management_object_index.port_type        = management_object_index.port_type;


    result = PON_CTC_STACK_get_multiple_ethernet_management_object_ds_rate_limiting(olt_id, onu_id, &number_of_entries, management_object_ds_rate_limiting);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    port_ds_rate_limiting->port_number = (unsigned char)(management_object_ds_rate_limiting[0].management_object_index.port_number);
    port_ds_rate_limiting->CIR         = management_object_ds_rate_limiting[0].CIR;
    port_ds_rate_limiting->PIR         = management_object_ds_rate_limiting[0].PIR;
    port_ds_rate_limiting->state       = management_object_ds_rate_limiting[0].state;


	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_ethernet_port_ds_rate_limiting"
PON_STATUS PON_CTC_STACK_get_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t								olt_id, 
           const PON_onu_id_t								onu_id,
		   const unsigned char								port_number,
		   CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting )
{
    short int								                result=CTC_STACK_EXIT_OK;
    CTC_STACK_ethernet_management_object_ds_rate_limiting_t	management_object_ds_rate_limiting;
    unsigned char							                number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_ds_rate_limiting[0].management_object_index.port_number      = port_number;
    management_object_ds_rate_limiting[0].management_object_index.slot_number      = 0x00;
    management_object_ds_rate_limiting[0].management_object_index.frame_number     = 0x00;
    management_object_ds_rate_limiting[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_multiple_ethernet_management_object_ds_rate_limiting(olt_id, onu_id, &number_of_entries, management_object_ds_rate_limiting);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    port_ds_rate_limiting->port_number = (unsigned char)(management_object_ds_rate_limiting[0].management_object_index.port_number);
    port_ds_rate_limiting->CIR         = management_object_ds_rate_limiting[0].CIR;
    port_ds_rate_limiting->PIR         = management_object_ds_rate_limiting[0].PIR;
    port_ds_rate_limiting->state       = management_object_ds_rate_limiting[0].state;


	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_qinq_vlan_multiple_management_object_configuration"

PON_STATUS PON_CTC_STACK_set_qinq_vlan_multiple_management_object_configuration ( 
           const PON_olt_id_t						                olt_id, 
           const PON_onu_id_t						                onu_id,
		   const unsigned char							            number_of_entries,
		   const CTC_STACK_qinq_configuration_management_object_t   management_object_vlan_configuration )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char							send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf   =  CTC_EX_VAR_QINQ_VLAN;
	unsigned char							received_num_of_entries=1;
	unsigned char							ctc_ret_val;
	unsigned char							i;
	unsigned short							max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char							local_number_of_entries = 0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    if(number_of_entries != CTC_STACK_ALL_PORTS)
    {
        for(i = 0; i < number_of_entries; i++)
        {
            CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_vlan_configuration[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
        }
    }

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_vlan_configuration[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_qinq
                            ( 1/*number_of_entries*/, &management_object_vlan_configuration[i].entry, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
                              SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_vlan_configuration[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_qinq
									( 1, &management_object_vlan_configuration[0].entry, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

	}
	


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}


	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

		received_total_length += received_left_length;


		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_vlan_configuration[i].management_object_index.port_number)
		}



		comparser_result = (short int)CTC_PARSE_general_ack
							( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
        					  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;	
		
	}
	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_qinq_vlan_multiple_management_object_configuration"

PON_STATUS PON_CTC_STACK_get_qinq_vlan_multiple_management_object_configuration ( 
           const PON_olt_id_t					              olt_id, 
           const PON_onu_id_t					              onu_id,
		   unsigned char						             *number_of_entries,
		   CTC_STACK_qinq_configuration_management_object_t   management_object_vlan_configuration )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_QINQ_VLAN;
	unsigned char							received_num_of_entries=0;
	unsigned char							ctc_ret_val, i, entry_index;
	CTC_STACK_port_qinq_configuration_t		received_ports_info = {0};
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_vlan_configuration[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_vlan_configuration[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_vlan_configuration[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}

	
    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_vlan_configuration[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_qinq
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &received_ports_info, &ctc_ret_val,&received_left_length  );

		    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;


			if(received_num_of_entries != 1)
			{
				
				PONLOG_ERROR_4( PONLOG_FUNC_GENERAL, olt_id, onu_id,
								"OLT %d ONU %d expected received entries: %d. received number of entries %d\n",
								olt_id, onu_id, 1, received_num_of_entries);


				result = CTC_STACK_MEMORY_ERROR;
			}
			else
			{
				unsigned char entry_index;

				management_object_vlan_configuration[i].entry.mode = received_ports_info.mode;			
				management_object_vlan_configuration[i].entry.default_vlan = received_ports_info.default_vlan;			
				management_object_vlan_configuration[i].entry.number_of_entries = received_ports_info.number_of_entries;			


				management_object_vlan_configuration[i].entry.vlan_list[0] = received_ports_info.vlan_list[0];

				if( received_ports_info.mode == CTC_QINQ_MODE_PER_VLAN )
				{
					for(entry_index = 0; entry_index < management_object_vlan_configuration[i].entry.number_of_entries*2; entry_index++)
						management_object_vlan_configuration[i].entry.vlan_list[entry_index] = received_ports_info.vlan_list[entry_index];
				}
				
			}

		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_qinq
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &received_ports_info, &ctc_ret_val,&received_left_length  );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;


			management_object_vlan_configuration[i].management_object_index.port_number         = received_management_object.index.port_number;			
    		management_object_vlan_configuration[i].management_object_index.frame_number        = received_management_object.index.frame_number;
    		management_object_vlan_configuration[i].management_object_index.slot_number         = received_management_object.index.slot_number;
    		management_object_vlan_configuration[i].management_object_index.port_type           = received_management_object.index.port_type;
			management_object_vlan_configuration[i].entry.mode = received_ports_info.mode;			
			management_object_vlan_configuration[i].entry.default_vlan = received_ports_info.default_vlan;			
			management_object_vlan_configuration[i].entry.number_of_entries = received_ports_info.number_of_entries;			


			management_object_vlan_configuration[i].entry.vlan_list[0] = received_ports_info.vlan_list[0];

			if( received_ports_info.mode == CTC_QINQ_MODE_PER_VLAN )
			{
				for(entry_index = 0; entry_index < management_object_vlan_configuration[i].entry.number_of_entries*2; entry_index++)
					management_object_vlan_configuration[i].entry.vlan_list[entry_index] = received_ports_info.vlan_list[entry_index];
			}		
			

			i++;

		}

		(*number_of_entries) = i;
	}
	
	return result;

}
#undef PONLOG_FUNCTION_NAME
  

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_qinq_vlan_all_management_object_configuration"

PON_STATUS PON_CTC_STACK_get_qinq_vlan_all_management_object_configuration( 
           const PON_olt_id_t					              olt_id, 
           const PON_onu_id_t					              onu_id,
		   unsigned char						             *number_of_entries,
		   CTC_STACK_qinq_configuration_management_object_t   management_object_info )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_qinq_vlan_multiple_management_object_configuration(olt_id, onu_id, number_of_entries, management_object_info));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_qinq_management_object_configuration"

PON_STATUS PON_CTC_STACK_set_qinq_management_object_configuration ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_port_qinq_configuration_t   port_configuration)
{
    short int								           result=CTC_STACK_EXIT_OK;
    CTC_STACK_qinq_configuration_management_object_t   management_object_vlan_configuration;
    unsigned char						               number_of_entries,i;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    management_object_vlan_configuration[0].management_object_index.port_number      = management_object_index.port_number;
    management_object_vlan_configuration[0].management_object_index.slot_number      = management_object_index.slot_number;
    management_object_vlan_configuration[0].management_object_index.frame_number     = management_object_index.frame_number;
    management_object_vlan_configuration[0].management_object_index.port_type        = management_object_index.port_type;

	management_object_vlan_configuration[0].entry.mode = port_configuration.mode;
	management_object_vlan_configuration[0].entry.default_vlan = port_configuration.default_vlan;
	management_object_vlan_configuration[0].entry.number_of_entries = port_configuration.number_of_entries;

	
    for( i = 0 ; i < port_configuration.number_of_entries*2; i++)
	{
		management_object_vlan_configuration[0].entry.vlan_list[i] = port_configuration.vlan_list[i];
	}


	result = PON_CTC_STACK_set_qinq_vlan_multiple_management_object_configuration(olt_id, onu_id, number_of_entries, management_object_vlan_configuration);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_qinq_management_object_configuration"

PON_STATUS PON_CTC_STACK_get_qinq_management_object_configuration ( 
           const PON_olt_id_t					       olt_id, 
           const PON_onu_id_t					       onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   CTC_STACK_port_qinq_configuration_t        *port_configuration)
{
    short int								           result=CTC_STACK_EXIT_OK;
    CTC_STACK_qinq_configuration_management_object_t   management_object_vlan_configuration;
	unsigned char							           number_of_entries = 1, i;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    management_object_vlan_configuration[0].management_object_index.port_number      = management_object_index.port_number;     
    management_object_vlan_configuration[0].management_object_index.slot_number      = management_object_index.slot_number;     
    management_object_vlan_configuration[0].management_object_index.frame_number     = management_object_index.frame_number;    
    management_object_vlan_configuration[0].management_object_index.port_type        = management_object_index.port_type;       


    result = PON_CTC_STACK_get_qinq_vlan_multiple_management_object_configuration(olt_id, onu_id, &number_of_entries, management_object_vlan_configuration);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	port_configuration->mode = management_object_vlan_configuration[0].entry.mode;
	port_configuration->default_vlan = management_object_vlan_configuration[0].entry.default_vlan;
	port_configuration->number_of_entries = management_object_vlan_configuration[0].entry.number_of_entries;

	for( i = 0 ; i < port_configuration->number_of_entries*2; i++)
	{
		port_configuration->vlan_list[i] = management_object_vlan_configuration[0].entry.vlan_list[i];
	}

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_qinq_vlan_all_port_configuration"

PON_STATUS PON_CTC_STACK_get_qinq_vlan_all_port_configuration( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   unsigned char						 *number_of_entries,
		   CTC_STACK_qinq_configuration_ports_t   ports_info )
{
    short int								            result;
    unsigned char                                       i, j;   
    CTC_STACK_qinq_configuration_management_object_t    management_object_vlan_configuration;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    (*number_of_entries) = CTC_STACK_ALL_PORTS;

    management_object_vlan_configuration[0].management_object_index.port_number      = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    management_object_vlan_configuration[0].management_object_index.slot_number      = 0x00;
    management_object_vlan_configuration[0].management_object_index.frame_number     = 0x00;
    management_object_vlan_configuration[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_qinq_vlan_multiple_management_object_configuration(olt_id, onu_id, number_of_entries, management_object_vlan_configuration);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    for( i = 0; i < (*number_of_entries); i++)
    {
        ports_info[i].port_number         = (unsigned char)(management_object_vlan_configuration[i].management_object_index.port_number);
        ports_info[i].entry.mode          = management_object_vlan_configuration[i].entry.mode;
        ports_info[i].entry.default_vlan         = management_object_vlan_configuration[i].entry.default_vlan;
        ports_info[i].entry.number_of_entries          = management_object_vlan_configuration[i].entry.number_of_entries;
        
        for( j = 0 ; j < ports_info[i].entry.number_of_entries*2; j++)
        {
            ports_info[i].entry.vlan_list[j] = management_object_vlan_configuration[i].entry.vlan_list[j];
        }
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_qinq_port_configuration"

PON_STATUS PON_CTC_STACK_set_qinq_port_configuration ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const unsigned char						   port_number,
		   const CTC_STACK_port_qinq_configuration_t   port_configuration)
{
    short int								           result=CTC_STACK_EXIT_OK;
    CTC_STACK_qinq_configuration_management_object_t   management_object_vlan_configuration;
    unsigned char						               number_of_entries,i;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	if(port_number == CTC_STACK_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
        management_object_vlan_configuration[0].management_object_index.port_number  = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
        management_object_vlan_configuration[0].management_object_index.port_number  = port_number;
    }


    management_object_vlan_configuration[0].management_object_index.slot_number      = 0x00;
    management_object_vlan_configuration[0].management_object_index.frame_number     = 0x00;
    management_object_vlan_configuration[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

	management_object_vlan_configuration[0].entry.mode = port_configuration.mode;
	management_object_vlan_configuration[0].entry.default_vlan = port_configuration.default_vlan;
	management_object_vlan_configuration[0].entry.number_of_entries = port_configuration.number_of_entries;

	
    for( i = 0 ; i < port_configuration.number_of_entries*2; i++)
	{
		management_object_vlan_configuration[0].entry.vlan_list[i] = port_configuration.vlan_list[i];
	}


	result = PON_CTC_STACK_set_qinq_vlan_multiple_management_object_configuration(olt_id, onu_id, number_of_entries, management_object_vlan_configuration);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_qinq_port_configuration"

PON_STATUS PON_CTC_STACK_get_qinq_port_configuration ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const unsigned char					 port_number,
		   CTC_STACK_port_qinq_configuration_t  *port_configuration)
{
    short int								           result=CTC_STACK_EXIT_OK;
    CTC_STACK_qinq_configuration_management_object_t   management_object_vlan_configuration;
	unsigned char							           number_of_entries = 1, i;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    management_object_vlan_configuration[0].management_object_index.port_number      = port_number;
    management_object_vlan_configuration[0].management_object_index.slot_number      = 0x00;
    management_object_vlan_configuration[0].management_object_index.frame_number     = 0x00;
    management_object_vlan_configuration[0].management_object_index.port_type        = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;


    result = PON_CTC_STACK_get_qinq_vlan_multiple_management_object_configuration(olt_id, onu_id, &number_of_entries, management_object_vlan_configuration);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	port_configuration->mode = management_object_vlan_configuration[0].entry.mode;
	port_configuration->default_vlan = management_object_vlan_configuration[0].entry.default_vlan;
	port_configuration->number_of_entries = management_object_vlan_configuration[0].entry.number_of_entries;

	for( i = 0 ; i < port_configuration->number_of_entries*2; i++)
	{
		port_configuration->vlan_list[i] = management_object_vlan_configuration[0].entry.vlan_list[i];
	}

	return result;
}
#undef PONLOG_FUNCTION_NAME


/*======================= Internal Functions ================================*/                 


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_timeout_simulate"

short int PON_CTC_STACK_timeout_simulate 
                    ( const PON_olt_id_t  olt_id, 
                      const PON_onu_id_t  onu_id,
                      const bool          timeout_mode )
{
    Timeout_ctc_simulate(olt_id, onu_id, timeout_mode);
  
    return CTC_STACK_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "New_olt_added_handler"

static PON_STATUS New_olt_added_handler (const PON_olt_id_t  olt_id)
{
    short int result;
    
#if 0
    if(PAS_OLT_IS_PAS5001(olt_id))
    {
        return CTC_STACK_EXIT_ERROR;
    }

    result = PON_CTC_STACK_set_encryption_configuration( olt_id, PON_ENCRYPTION_TYPE_TRIPLE_CHURNING );

    if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_2(PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT, "OLT %d failed to set encryption type to TRIPLE CHURNING. error code:%d\n", olt_id, result)

    }
#endif

    encryption_db.olt[olt_id].encryption_type = PON_ENCRYPTION_TYPE_TRIPLE_CHURNING;

    
    /*set the default auth mode*/
#ifdef CTC_AUTH_SUPPORT
    result = PON_CTC_STACK_set_auth_mode(olt_id, (authentication_db.olt[olt_id].auth_mode));
	if ( result != CTC_STACK_EXIT_OK )
    {
        return result;
    }
    

	result = Convert_ponsoft_error_code_to_ctc_stack(
					PAS_set_dba_report_format(olt_id, PON_DBA_THRESHOLD_REPORT));
	if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_2(PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT,"OLT %d failed to set dba report format:%d\n", olt_id, result)
    }

    /*support over-length frame*/
    result = Convert_ponsoft_error_code_to_ctc_stack(
        PAS_set_frame_size_limits(olt_id, PON_DIRECTION_UPLINK, CTC_OVER_LENGTH_FRAME, CTC_OVER_LENGTH_FRAME));
    if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_2(PONLOG_FUNC_CTC_COMM, olt_id, PONLOG_ONU_IRRELEVANT,"OLT: %d failed to set frame size limits, direction uplink. error code:%d\n", olt_id, result)
            return result;
    }

    result = Convert_ponsoft_error_code_to_ctc_stack(
        PAS_set_frame_size_limits(olt_id, PON_DIRECTION_DOWNLINK, CTC_OVER_LENGTH_FRAME, CTC_OVER_LENGTH_FRAME));
    if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_2(PONLOG_FUNC_CTC_COMM, olt_id, PONLOG_ONU_IRRELEVANT,"OLT: %d failed to set frame size limits, direction downlink. error code:%d\n", olt_id, result)
            return result;
    }
#endif

    return CTC_STACK_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Removed_olt_handler"

static PON_STATUS Removed_olt_handler (const PON_olt_id_t  olt_id)
{

	PON_llid_t                  onu_id;
    unsigned short              index;
	/* init extended OAM discovery state to all ONU's */
    for(onu_id = PON_MIN_ONU_ID_PER_OLT; onu_id < PON_MAX_ONU_ID_PER_OLT; onu_id++)
    {
        Reset_onu_entry(olt_id, onu_id);
		update_firmware_db[olt_id].onu_state[onu_id] = CTC_STACK_ONU_STATE_NOT_DURING_DOWNLOADING;
    }


	encryption_db.olt[olt_id].encryption_type = PON_ENCRYPTION_TYPE_AES;

#ifdef CTC_AUTH_SUPPORT
    /*clear pon port loid list, added by luh 2012-2-15*/
    for(index = 0; index < PON_ONU_ID_PER_OLT_ARRAY_SIZE; index++)
    {
        strcpy((authentication_db.olt[olt_id].loid[index].onu_id), CTC_AUTH_LOID_DATA_NOT_USED);
        strcpy((authentication_db.olt[olt_id].loid[index].password), CTC_AUTH_LOID_DATA_NOT_USED);
        authentication_db.olt[olt_id].auth_mode = CTC_AUTH_MODE_HYBRID;
        authentication_db.olt[olt_id].onu_llid[index] = PON_NOT_USED_LLID;
    }
#endif

    return CTC_STACK_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME




#define PONLOG_FUNCTION_NAME  "Send_new_key_request"

static short int Send_new_key_request ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   const PON_encryption_key_index_t   key_index )
{
    short int                result,comparser_result;
    unsigned short           sent_length=ETHERNET_MTU;
    unsigned char            sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char            expected_opcode =  OAM_VENDOR_EXT_ENC;
    unsigned char            expected_code   =  ENC_VENDOR_EXT_KEY_RESPONSE;

    CTC_STACK_CHECK_INIT_AND_CHECK_OLT_AND_ONU_VALIDITY_FUNC
       


	result = Check_encryption_state_if_already_received(olt_id, onu_id);

    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    /* increase number of sending request , after ENCRYPTION_MAX_RETRIES_DEFINE retries stop sending */
    encryption_db.olt[olt_id].onu_id[onu_id].number_of_sending_request ++;

    PONLOG_INFO_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d send new key request\n",olt_id, onu_id );

    comparser_result = (short int)Compose_new_key_request
                            ( key_index, sent_oam_pdu_data+OAM_OUI_SIZE, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_ENCRYPTION_FLOW, comparser_result)


     result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, sent_length, 
                      expected_opcode, expected_code,
                      encryption_db.olt[olt_id].onu_id[onu_id].key_request_received_oam_pdu_data, 
					  &encryption_db.olt[olt_id].onu_id[onu_id].key_request_received_length, 
					  ETHERNET_MTU,(void*)*Received_new_churning_key);

     
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
       
        
    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Received_new_churning_key"

static void Received_new_churning_key  ( 
                   const PON_olt_id_t     olt_id,
                   const PON_onu_id_t     onu_id,
                   const unsigned short   length,
                   unsigned char         *content )
{


    short int                   result,comparser_result;
    CTC_encryption_key_t        key_value;
    PON_encryption_key_index_t  key_index;
    

    comparser_result = Parse_new_churning_key( content, length,
                                               &key_index, key_value );
    
    result = Convert_comparser_error_code_to_ctc_stack(comparser_result);
    if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_3(PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT %d ONU %d failed to parse received buffer. error code:%d\n", olt_id, onu_id, result)
        return;
    }


    result = Update_encryption_db(olt_id, onu_id, key_index, key_value);
    if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_3(PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id,"OLT %d ONU %d failed to update encryption db. error code:%d\n", olt_id, onu_id, result)
    }
    
   
    return;
    
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Encryption_process_thread_hook_func"

static short int Encryption_process_thread_hook_func(void *data)
{
    PON_olt_id_t                olt_id;
    PON_llid_t                  onu_id;
	short int					result;
    PON_redundancy_olt_state_t  redundancy_state = PON_OLT_REDUNDANCY_STATE_NONE;
	PON_redundancy_llid_redundancy_mode_t   onu_redundancy_mode = PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL;

    while (encryption_process_thread_active)
    {
        /* Check encryption db and send 'Update' and 'Start' encryption commands to the HW */
        for(olt_id = 0; olt_id < PON_OLT_ID_ARRAY_SIZE;olt_id++)
        {
	        SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC( PONLOG_FUNC_ENCRYPTION_FLOW, 
											         &encryption_thread_semaphore, 
											         encryption_thread_semaphore, result)
           	if (result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);

            if(BcmOlt_exists(olt_id) )
            {
                result = Get_redundancy_olt_state(olt_id, &redundancy_state);
                if(result != EXIT_OK)
                {
                    PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT, "OLT:%d failed in Get_redundancy_olt_state\n", olt_id );
                }
                if((redundancy_state != PON_OLT_REDUNDANCY_STATE_SLAVE) &&
                   (redundancy_state != PON_OLT_REDUNDANCY_STATE_INACTIVE_SLAVE))
                {
                    if(encryption_db.olt[olt_id].encryption_type == PON_ENCRYPTION_TYPE_TRIPLE_CHURNING)
                    {
                        for(onu_id = PON_MIN_ONU_ID_PER_OLT; onu_id < PON_MAX_ONU_ID_PER_OLT;onu_id++)
                        {
                            onu_redundancy_mode = PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL;
            	            result = Get_redundancy_onu_mode(olt_id, onu_id , &onu_redundancy_mode);
                            if(result != EXIT_OK)
                            {
                                PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, olt_id, onu_id, "ONU:%d failed in Get_redundancy_onu_mode\n", onu_id );
                            }
                            
                            if ((onu_redundancy_mode == PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL)||
                                 (onu_redundancy_mode == PON_REDUNDANCY_LLID_REDUNDANCY_ACTIVE))
                            {
                                if( encryption_db.olt[olt_id].onu_id[onu_id].start_encryption_by_the_user)
                                { 
                                    if( encryption_db.olt[olt_id].onu_id[onu_id].response_encryption_state == CTC_STACK_STATE_RECEIVED_FRAME)
                                        
                                    {
                                        if(encryption_db.olt[olt_id].onu_id[onu_id].key_index != encryption_db.olt[olt_id].onu_id[onu_id].temp_key_index)
                                        {
                                            result = Encryption_sequence_to_olt(olt_id, onu_id, TRUE);
                                            if(result == CTC_STACK_EXIT_OK)
                                            {
                                                encryption_db.olt[olt_id].onu_id[onu_id].key_index = encryption_db.olt[olt_id].onu_id[onu_id].temp_key_index;
                                                
                                                Set_encryption_db_flag_entry(olt_id, onu_id, CTC_STACK_STATE_VALID);
                                                
                                                result = Set_encryption_db_entry  ( olt_id, onu_id,CTC_ENCRYPTION_STATE_ENCRYPTION_COMPLETE/*encryption_state*/);
                                                if(result != CTC_STACK_EXIT_OK)
                                                {
                                                    PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d failed in Set_encryption_db_entry\n",
                                                                    olt_id, onu_id );
                                                    
                                                    /* Release shemapore before exit */
                                                    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC( PONLOG_FUNC_ENCRYPTION_FLOW, 
                                                                                              &encryption_thread_semaphore, 
                                                                                              encryption_thread_semaphore, result)
                                                    if (result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);
                                                        
                                                    return result;
                                                }
                                                
                                            }
                                            else
                                            {
                                                PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "FAIL in Encryption_sequence_to_olt of OLT:%d ONU:%d\n", 
                                                                olt_id, onu_id ); 
                                            }
                                        }
                                        else
                                        {
                                            PONLOG_INFO_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "low performance in OLT:%d ONU:%d\n", 
                                                           olt_id, onu_id ); 
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
	        RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC( PONLOG_FUNC_ENCRYPTION_FLOW, 
											         &encryption_thread_semaphore, 
											         encryption_thread_semaphore, result)
           	if (result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);

        }

        OSSRV_wait(300);
    }

    OSSRV_terminate_thread();
    
    return (CTC_STACK_EXIT_OK);

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Encryption_scheduler_thread_hook_func"

static short int Encryption_scheduler_thread_hook_func(void *data)
{
    unsigned short      result, diff_time_in_100_ms;
    unsigned short      scheduler_iteration=0;
    time_t              encryption_current_time,begin_loop;
    unsigned long       loop_time_in_sec;
	unsigned char       temp_update_key_time_in_sec;
    unsigned short      temp_no_reply_timeout_in_100_ms;
                

    while (encryption_scheduler_thread_active)
    {   

       time(&begin_loop);


       if (scheduler_iteration == 0)
       {
           /*PON_NO_OLT_TRACE_ERROR_1( "start Update key iteration: %u\n", scheduler_iteration);*/
           time(&encryption_last_update_key_time);

           /* set the time in every cycle activation for the decision whether to send new key or wait to the next cycle*/
           encryption_last_timeout_time = encryption_last_update_key_time;
           Send_update_key_iteration();
       }
       else
       {
           /*PON_NO_OLT_TRACE_ERROR_1( "start Timeout iteration: %u\n", scheduler_iteration);*/

           /* set the time in every cycle activation for the decision whether to send new key or wait to the next cycle*/
           time(&encryption_last_timeout_time);

           Send_timeout_iteration();
       }


       time(&encryption_current_time); 

       loop_time_in_sec = encryption_current_time - begin_loop;
 
        /* PON_NO_OLT_TRACE_ERROR_1( "loop time: %u sec\n", loop_time_in_sec); */

	   /* take the encryption timing semaphore to protect against real time parameters setting */
		SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_timing_parameters_semaphore), 
											  ctc_stack_encryption_timing_parameters_semaphore,result );
		if (result != EXIT_OK) 
		{
			PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
							"failed to take a ctc_stack_encryption_timing_parameters_semaphore\n" );     
			return (CTC_STACK_EXIT_ERROR);
		}


	   temp_update_key_time_in_sec = encryption_db.timing_info.update_key_time_in_sec;
	   temp_no_reply_timeout_in_100_ms = encryption_db.timing_info.no_reply_timeout_in_100_ms;



	   /* release synchronization semaphore between this task and terminate API */
		RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_timing_parameters_semaphore),
											 ctc_stack_encryption_encryption_timing_parameters_semaphore,result );
		if (result != EXIT_OK) 
		{
			PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,"failed to release a ctc_stack_encryption_timing_parameters_semaphore\n" );
		}

	   if( (temp_update_key_time_in_sec*10/*convert into 100 msec*/) < (temp_no_reply_timeout_in_100_ms*scheduler_iteration) )
	   {
		   diff_time_in_100_ms = 0;
		   scheduler_iteration = 0;

		   
		   continue;
	   }
	   else
	   {
		   diff_time_in_100_ms = temp_update_key_time_in_sec*10/*convert into 100 msec*/ - (temp_no_reply_timeout_in_100_ms*scheduler_iteration);
	   }

       if (( diff_time_in_100_ms <= temp_no_reply_timeout_in_100_ms ) ||
		   ( temp_update_key_time_in_sec*10/*convert into 100 msec*/ < temp_no_reply_timeout_in_100_ms*scheduler_iteration))
       {
           /* needed for send message every update_key_time_in_sec*/ 
           Scheduler_sleep((unsigned long)(diff_time_in_100_ms*100/*convert into msec*/), loop_time_in_sec*1000/*convert into msec*/);
           scheduler_iteration = 0;
       }
       else
       {
           /* needed for resending every no_reply_timeout_in_100_ms */
           Scheduler_sleep((unsigned long)(temp_no_reply_timeout_in_100_ms*100/*convert into msec*/), loop_time_in_sec*1000/*convert into msec*/);
           scheduler_iteration++;
       }
    }

  
	/* release synchronization semaphore between this task and terminate API */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_scheduler_thread_semaphore),
                                         ctc_stack_encryption_scheduler_thread_semaphore,result );
    if (result != EXIT_OK) 
    {
        PONLOG_ERROR_0(PONLOG_FUNC_ENCRYPTION_FLOW, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,"failed to release a ctc_stack_encryption_scheduler_thread_semaphore\n" );
    }


    OSSRV_terminate_thread();

    return CTC_STACK_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Scheduler_sleep"

 

static void Scheduler_sleep(
                     unsigned long   time_to_sleep,
                     unsigned long   loop_iteration_time )
{

    unsigned short      sleep_time_in_ms;
	unsigned long       long_sleep_time_in_ms;
	unsigned char		sleep_times,i;


    /* sleep according to scheduler time considering the passed time(dynamic sleeping) */
    if (time_to_sleep > loop_iteration_time)
    {
       long_sleep_time_in_ms = (unsigned long)(time_to_sleep - loop_iteration_time);
    }
    else
    {
        long_sleep_time_in_ms = 0;

    }

    /*PON_NO_OLT_TRACE_ERROR_1( "scheduler sleep for %u milliseconds\n", sleep_time_in_ms);*/


	/* OSSRV_wait get an unsigned short value represent the sleep time in msec. 
	   that mean 65 seconds only. in order to enable sleeping more we should divide the large
	   time into skeeping quantot */

	if(long_sleep_time_in_ms > MAXUWORD)/* in case of long values*/
	{
		/* sleep_times = number of quantot to sleep */
		sleep_times = (unsigned char) (long_sleep_time_in_ms / MAXUWORD);
		sleep_time_in_ms = MAXUWORD;
		/* first, sleep the remainder value */
		OSSRV_wait((unsigned short)(long_sleep_time_in_ms % MAXUWORD));
	}
	else/* in case of short values*/
	{
		sleep_times = 1;/* only 1 cycle of sleeping */
		sleep_time_in_ms = (unsigned short) long_sleep_time_in_ms;/* sleep time is the original sleep time */
	}
	

	for( i = 0 ; i < sleep_times; i++)
	{
		OSSRV_wait(sleep_time_in_ms);
	}

}
#undef PONLOG_FUNCTION_NAME





#define PONLOG_FUNCTION_NAME  "Send_update_key_iteration"

static void Send_update_key_iteration()
{

    PON_olt_id_t				olt_id;
    PON_llid_t					onu_id;
    short int					result;
    general_handler_function_t  the_handler;
	short int					client;
    PON_redundancy_olt_state_t  redundancy_state = PON_OLT_REDUNDANCY_STATE_NONE;
	PON_redundancy_llid_redundancy_mode_t   onu_redundancy_mode = PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL;
    
    /* only in the seconds update iteration and further iterations */
    if( first_update_key_iteration)
    {
        first_update_key_iteration = FALSE;
    }
    else
    {   
        for(olt_id = 0; olt_id < PON_OLT_ID_ARRAY_SIZE;olt_id++)
        {
            for(onu_id = PON_MIN_ONU_ID_PER_OLT; onu_id < PON_MAX_ONU_ID_PER_OLT;onu_id++)
            {
                if((encryption_db.olt[olt_id].onu_id[onu_id].response_encryption_state == CTC_STACK_STATE_WAITING_FOR_RESPONSE) &&
                    encryption_db.olt[olt_id].onu_id[onu_id].number_of_sending_request < ENCRYPTION_MAX_REQUESTS_TO_ONU_DEFINE)
                {
                    PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d encryption failed in last update key cycle\n",
                                       olt_id, onu_id );


					for (client = 0; client < PON_MAX_CLIENT; client++)
					{
						PON_general_get_handler_function
										   ( &ctc_db_handlers,
											 CTC_STACK_HANDLER_UPDATE_ENCRYPTION_KEY,
											 client ,
											 &the_handler );

						if (the_handler != NULL)
						{
							the_handler (	olt_id, onu_id,
											CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED);
						}
					}
                }
            }
        }
        
    }

    /* there are 2 loops:
    first loop  - clean encryption db
    second loop - send requests */

    for(olt_id = 0; olt_id < PON_OLT_ID_ARRAY_SIZE;olt_id++)
    {
        for(onu_id = PON_MIN_ONU_ID_PER_OLT; onu_id < PON_MAX_ONU_ID_PER_OLT;onu_id++)
        {
            encryption_db.olt[olt_id].onu_id[onu_id].number_of_sending_request = 0;
            
            result= Set_encryption_db_flag_entry(olt_id, onu_id, CTC_STACK_STATE_VALID); 
            if(result != CTC_STACK_EXIT_OK)
            {
                PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d failed in  Clean_encryption_db_flag_entry\n",
                                    olt_id, onu_id );
            }
        }
    }

    for(olt_id = 0; olt_id < PON_OLT_ID_ARRAY_SIZE;olt_id++)
    {	
        if(BcmOlt_exists(olt_id) )
        {
            result = Get_redundancy_olt_state(olt_id, &redundancy_state);
            if(result != EXIT_OK)
            {
                PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT, "OLT:%d failed in Get_redundancy_olt_state\n", olt_id );
            }
            if((redundancy_state != PON_OLT_REDUNDANCY_STATE_SLAVE) &&
               (redundancy_state != PON_OLT_REDUNDANCY_STATE_INACTIVE_SLAVE))
            {
                if(encryption_db.olt[olt_id].encryption_type == PON_ENCRYPTION_TYPE_TRIPLE_CHURNING)
                {
                    for(onu_id = PON_MIN_ONU_ID_PER_OLT; onu_id < PON_MAX_ONU_ID_PER_OLT;onu_id++)
                    {
                        onu_redundancy_mode = PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL;
        	            result = Get_redundancy_onu_mode(olt_id, onu_id , &onu_redundancy_mode);
                        if(result != EXIT_OK)
                        {
                            PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, olt_id, onu_id, "ONU:%d failed in Get_redundancy_onu_mode\n", onu_id );
                        }
                        
                        if ((onu_redundancy_mode == PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL)||
                             (onu_redundancy_mode == PON_REDUNDANCY_LLID_REDUNDANCY_ACTIVE))
                        {
                    
                            if( discovery_db.state[olt_id][onu_id] == CTC_DISCOVERY_STATE_COMPLETE)
                            {
                                if( encryption_db.olt[olt_id].onu_id[onu_id].start_encryption_by_the_user)
                                {
                                    Send_new_key_request( olt_id, onu_id,
                                                          encryption_db.olt[olt_id].onu_id[onu_id].key_index );    
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Send_timeout_iteration"

static void Send_timeout_iteration()
{

    PON_olt_id_t                olt_id;
    PON_llid_t                  onu_id;
    short int		            result;
    PON_redundancy_olt_state_t  redundancy_state = PON_OLT_REDUNDANCY_STATE_NONE;
	PON_redundancy_llid_redundancy_mode_t   onu_redundancy_mode = PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL;
    
    /* Check encryption db and send request only to onu that didn't reply yet,no_reply_timeout_in_100_ms */
    for(olt_id = 0; olt_id < PON_OLT_ID_ARRAY_SIZE;olt_id++)
    {
        if(BcmOlt_exists(olt_id) )
        {
            result = Get_redundancy_olt_state(olt_id, &redundancy_state);
            if(result != EXIT_OK)
            {
                PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT, "OLT:%d failed in Get_redundancy_olt_state\n", olt_id );
            }
            if((redundancy_state != PON_OLT_REDUNDANCY_STATE_SLAVE) &&
               (redundancy_state != PON_OLT_REDUNDANCY_STATE_INACTIVE_SLAVE))
            {
                if(encryption_db.olt[olt_id].encryption_type == PON_ENCRYPTION_TYPE_TRIPLE_CHURNING)
                {
                    for(onu_id = PON_MIN_ONU_ID_PER_OLT; onu_id < PON_MAX_ONU_ID_PER_OLT;onu_id++)
                    {
                        onu_redundancy_mode = PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL;
        	            result = Get_redundancy_onu_mode(olt_id, onu_id , &onu_redundancy_mode);
                        if(result != EXIT_OK)
                        {
                            PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, olt_id, onu_id, "ONU:%d failed in Get_redundancy_onu_mode\n", onu_id );
                        }
                        
                        if ((onu_redundancy_mode == PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL)||
                             (onu_redundancy_mode == PON_REDUNDANCY_LLID_REDUNDANCY_ACTIVE))
                        {
                            if( discovery_db.state[olt_id][onu_id] == CTC_DISCOVERY_STATE_COMPLETE)
                            {
                                if( encryption_db.olt[olt_id].onu_id[onu_id].start_encryption_by_the_user)
                                { 
                                    if( encryption_db.olt[olt_id].onu_id[onu_id].response_encryption_state == CTC_STACK_STATE_WAITING_FOR_RESPONSE )
                                    {
                                        
                                        Send_timeout( olt_id, onu_id,
                                                      encryption_db.olt[olt_id].onu_id[onu_id].key_index );        
                                        
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
        }
    }
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Send_timeout"

short int Send_timeout ( 
              const PON_olt_id_t                 olt_id, 
              const PON_onu_id_t                 onu_id,
              const PON_encryption_key_index_t   key_index )
{
    bool        need_to_send_according_to_threshld,need_to_send_according_to_number_of_retransimt;
    
    

    need_to_send_according_to_number_of_retransimt = Check_if_need_to_send_according_number_of_retransmit( olt_id, onu_id );
    if(need_to_send_according_to_number_of_retransimt == FALSE)
    {
        return CTC_STACK_EXIT_OK;
    }

    Check_threshold_time_according_update_key_time(&need_to_send_according_to_threshld);

    if(need_to_send_according_to_threshld)
    {
			PONLOG_INFO_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id,
						   "OLT %d ONU %d timeout- ONU has not responded to new key request, resending a new request %d\n",
						   olt_id, onu_id );


            return (Send_new_key_request(olt_id, onu_id, key_index));
        
    }
    else/* don't send right now, send will be on the next update key cycle*/
    {
        return CTC_STACK_EXIT_OK;
    }
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Check_if_need_to_send_according_number_of_retransmit"

static bool Check_if_need_to_send_according_number_of_retransmit ( 
              const PON_olt_id_t   olt_id, 
              const PON_onu_id_t   onu_id )
{
	short int					client;
	general_handler_function_t  the_handler;


    if(encryption_db.olt[olt_id].onu_id[onu_id].number_of_sending_request == ENCRYPTION_MAX_REQUESTS_TO_ONU_DEFINE)
    {

        PONLOG_ERROR_3( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id,"OLT:%d ONU:%d is not responding after %d, stop sending encryption requests\n",
                           olt_id, onu_id, 3 );


		for (client = 0; client < PON_MAX_CLIENT; client++)
		{
			PON_general_get_handler_function
							   ( &ctc_db_handlers,
								 CTC_STACK_HANDLER_UPDATE_ENCRYPTION_KEY,
								 client ,
								 &the_handler );

			if (the_handler != NULL)
			{
				the_handler (	olt_id, onu_id,
								CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED);
			}
		}

        encryption_db.olt[olt_id].onu_id[onu_id].number_of_sending_request++;

        return FALSE;
    
    }
    else if(encryption_db.olt[olt_id].onu_id[onu_id].number_of_sending_request > ENCRYPTION_MAX_REQUESTS_TO_ONU_DEFINE)
    {
        /* we don't send request after ENCRYPTION_MAX_RETRIES_DEFINE iterations */
        return FALSE;
    }
    else
    {
        return TRUE;
    }

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Check_threshold_time_according_update_key_time"

static void Check_threshold_time_according_update_key_time ( bool  *need_to_send)
{
    time_t              current_time,next_update,threshold_time;
    unsigned long       diff;

    time(&current_time);

    next_update = encryption_last_update_key_time + (encryption_db.timing_info.update_key_time_in_sec);

    threshold_time = current_time + encryption_db.timing_info.start_encryption_threshold_in_sec;
  


    if (next_update > threshold_time)
    {     
        diff = next_update - encryption_db.timing_info.start_encryption_threshold_in_sec - current_time;
        /*PONLOG_INFO_1( "SENDING if needed, because there still %u seconds to be in the threshold range\n",diff );*/
        (*need_to_send) = TRUE;
    }
    else
    {
        diff = next_update - current_time;
        PONLOG_INFO_1( PONLOG_FUNC_ENCRYPTION_FLOW, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "NOT SENDING, because we are %u seconds before next update process\n",diff );
        (*need_to_send) = FALSE;
    }
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Check_threshold_time_according_timeout_time"

static void Check_threshold_time_according_timeout_time ( bool  *need_to_send)
{
    time_t              current_time,next_update,threshold_time;
    unsigned long       diff;

    time(&current_time);

    next_update = encryption_last_timeout_time + (encryption_db.timing_info.no_reply_timeout_in_100_ms/10/*convert to sec*/);

    threshold_time = current_time + encryption_db.timing_info.start_encryption_threshold_in_sec;

    if (next_update > threshold_time)
    {
        diff = next_update - encryption_db.timing_info.start_encryption_threshold_in_sec - current_time;
        /*PONLOG_INFO_1( "SENDING if needed, because there still %u seconds to be in the threshold range\n",diff );*/
        (*need_to_send) = TRUE;
    }
    else
    {
        diff = next_update - current_time;
        PONLOG_INFO_1( PONLOG_FUNC_ENCRYPTION_FLOW, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "NOT SENDING, because we are %u seconds before next update process\n",diff );
        (*need_to_send) = FALSE;
    }
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Encryption_sequence_to_olt"

static short int Encryption_sequence_to_olt(
                      const PON_olt_id_t  olt_id, 
                      const PON_onu_id_t  onu_id,
                      const bool          generate_event)
{
    
    short int                   result,i;
    PON_encryption_key_t        encryption_key={0};
    unsigned char               encryption_key_string[300];
    unsigned short              temp_buffer_size = 0;
	short int					client;
	general_handler_function_t  the_handler;
   
        
    Copy_encryption_key( encryption_db.olt[olt_id].onu_id[onu_id].key_value,
                         encryption_key+PON_ENCRYPTION_KEY_SIZE- CTC_ENCRYPTION_KEY_SIZE );
    

    for(i=0; i < PON_ENCRYPTION_KEY_SIZE; i++)
    {
        if( (i%4) == 0)
        {
            sprintf(encryption_key_string+temp_buffer_size," 0x");
            temp_buffer_size = strlen(encryption_key_string);
        }
        
        sprintf(encryption_key_string+temp_buffer_size,"%02x", encryption_key[i]);
        temp_buffer_size = strlen(encryption_key_string);

    }
    sprintf(encryption_key_string+temp_buffer_size,"\n");
    temp_buffer_size = strlen(encryption_key_string);

    
    PONLOG_INFO_4( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d start encryption in OLT key index %d value: %s\n", 
				   olt_id, onu_id,encryption_db.olt[olt_id].onu_id[onu_id].temp_key_index, encryption_key_string );
 

    result = OLT_SetLLIDEncryptKey( olt_id, onu_id, encryption_key );
    if(result != 0)
    {
        PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d failed in  OPEN_ENCRYPTION_set_olt_encryption_key\n",
                           olt_id, onu_id );

        Set_encryption_db_entry  ( olt_id, onu_id, CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED/*encryption_state*/);


		for (client = 0; client < PON_MAX_CLIENT; client++)
		{
			PON_general_get_handler_function
							   ( &ctc_db_handlers,
								 CTC_STACK_HANDLER_UPDATE_ENCRYPTION_KEY,
								 client ,
								 &the_handler );

			if (the_handler != NULL)
			{
				the_handler (	olt_id, onu_id,
								CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED);
			}
		}
        return result;
    }

    result = OLT_FinishLLIDEncryptKey ( olt_id, onu_id, EXIT_OK );
    if(result != 0)
    {
        PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d failed in  OPEN_ENCRYPTION_finalize_set_olt_encryption_key\n",
                           olt_id, onu_id );

        Set_encryption_db_entry  ( olt_id, onu_id, CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED/*encryption_state*/);
        for (client = 0; client < PON_MAX_CLIENT; client++)
		{
			PON_general_get_handler_function
							   ( &ctc_db_handlers,
								 CTC_STACK_HANDLER_UPDATE_ENCRYPTION_KEY,
								 client ,
								 &the_handler );

			if (the_handler != NULL)
			{
				the_handler (	olt_id, onu_id,
								CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED);
			}
		}
        return result;
    }

    /*inform the encryption_key value*/
    if(generate_event)
    {
        for (client = 0; client < PON_MAX_CLIENT; client++)
        {
            PON_general_get_handler_function
                               ( &ctc_db_handlers,
                                 CTC_STACK_HANDLER_ENCRYPTION_KEY_VALUE,
                                 client ,
                                 &the_handler );
            
            if (the_handler != NULL)
            {
                the_handler (	olt_id, onu_id,
                            CTC_ENCRYPTION_STATE_ENCRYPTION_COMPLETE,
                            encryption_key,
                            encryption_db.olt[olt_id].onu_id[onu_id].temp_key_index);
            }
        }
    }

    result =  OLT_StartLLIDEncrypt ( olt_id, onu_id );


    if(result != 0)
    {
        PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d failed in   OPEN_ENCRYPTION_start_olt_encryption\n",
                           olt_id, onu_id );

        Set_encryption_db_entry  ( olt_id, onu_id, CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED/*encryption_state*/);
        for (client = 0; client < PON_MAX_CLIENT; client++)
		{
			PON_general_get_handler_function
							   ( &ctc_db_handlers,
								 CTC_STACK_HANDLER_UPDATE_ENCRYPTION_KEY,
								 client ,
								 &the_handler );

			if (the_handler != NULL)
			{
				the_handler (	olt_id, onu_id,
								CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED);
			}
		}
        return result;
    }

    result = OLT_FinishLLIDEncrypt ( olt_id, onu_id, EXIT_OK );
    if(result != 0)
    {
        PONLOG_ERROR_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d failed in  OPEN_ENCRYPTION_finalize_start_olt_encryption\n",
                           olt_id, onu_id );

        Set_encryption_db_entry  ( olt_id, onu_id, CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED/*encryption_state*/);
        for (client = 0; client < PON_MAX_CLIENT; client++)
		{
			PON_general_get_handler_function
							   ( &ctc_db_handlers,
								 CTC_STACK_HANDLER_UPDATE_ENCRYPTION_KEY,
								 client ,
								 &the_handler );

			if (the_handler != NULL)
			{
				the_handler (	olt_id, onu_id,
								CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED);
			}
		}
        return result;
    }

    if(generate_event)
    {
        for (client = 0; client < PON_MAX_CLIENT; client++)
        {
        	PON_general_get_handler_function
        					   ( &ctc_db_handlers,
        						 CTC_STACK_HANDLER_UPDATE_ENCRYPTION_KEY,
        						 client ,
        						 &the_handler );

        	if (the_handler != NULL)
        	{
        		the_handler (	olt_id, onu_id,
        						CTC_ENCRYPTION_STATE_ENCRYPTION_COMPLETE);
        	}
        }
    }

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Send_receive_extended_oam_information"

static short int Send_receive_extended_oam_information ( 
                       const PON_olt_id_t         olt_id, 
                       const PON_onu_id_t         onu_id,
                       unsigned char 	          send_number_of_records,
                       CTC_STACK_oui_version_record_t  *send_oui_version_record_array,
                       bool                      *ext_support,
                       unsigned char 	         *receive_number_of_records,
                       CTC_STACK_oui_version_record_t  *receive_oui_version_record_array )
{
    short int                result,comparser_result;
    unsigned short           received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char            sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char            received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char            received_ext_support;
    unsigned char 	         ctc_version;
	unsigned short 			 timeout_in_100_ms = discovery_db.timeout_in_100_ms;
    
    
    CTC_STACK_CHECK_INIT_FUNC
       
    if(send_number_of_records == 0)
    { /* send only the common version found */
      ctc_version = discovery_db.common_version[olt_id][onu_id]; 
    }
    else
    {
      ctc_version = ctc_stack_general_information.version;
    }

    comparser_result = (short int)Compose_extended_oam_information
                            ( ctc_stack_general_information.ctc_stack_oui,
                              ctc_version, send_number_of_records, 
                              send_oui_version_record_array,
                              sent_oam_pdu_data, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


    result = Send_receive_ctc_message 
                    ( olt_id, onu_id, PON_OAM_CODE_OAM_IMFORMATION, sent_oam_pdu_data, sent_length, 
                      &timeout_in_100_ms ,0/*expected_opcode*/, 0/*expected_code*/,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU, NULL, 0);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
  

    comparser_result = (short int)Parse_extended_oam_information
                               ( RECEIVED_COMPARSER_BUFFER_OLD_COMMANDS - 32, received_length + 32,
         	                     receive_number_of_records, receive_oui_version_record_array,&received_ext_support );

    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


    (*ext_support) = (bool) received_ext_support;
    
    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Registration_handler_discovery_process2"

int Registration_handler_discovery_process2 
				   ( const PON_olt_id_t					  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const mac_address_t				  mac_address,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard,
					 const bool event_report)
{
    short int						result;
    bool							support, received_ext_support;
    unsigned char 					receive_number_of_records=0,sending_index=0;
    CTC_STACK_oui_version_record_t  receive_oui_version_record_array[MAX_OUI_RECORDS];
    bool							resend = TRUE, correct_oui_zero_version=FALSE;
#if 0
	PON_oam_peer_status_raw_data_t  statistics_data;
#else
    unsigned long                   peer_pdu_size;
#endif
	onu_registration_data_record_t  onu_registration_data; 
	short int                       query_result;  
	PON_timestamp_t					timestamp; 
	short int						client;
	general_handler_function_t		the_handler;
    bool                            loid_auth_success = FALSE;

#if 0
    if(PAS_OLT_IS_PAS5001(olt_id))
    {
        return CTC_STACK_EXIT_ERROR;
    }
#endif

    /* set ONU state to IDLE */
    discovery_db.state[olt_id][onu_id] = CTC_DISCOVERY_STATE_IDLE;


#if 0
	query_result = Get_registration_data ( olt_id, onu_id, &onu_registration_data ); 
	if (onu_registration_data.oam_version == OAM_STANDARD_VERSION_3_3) 
#endif
    {
#if 0
	    result = Convert_ponsoft_error_code_to_ctc_stack(
					PAS_get_raw_statistics(olt_id,
                               PON_OLT_ID,PON_RAW_STAT_STANDARD_OAM_PEER_STATUS,
                               onu_id,
                               &statistics_data,
                               &timestamp));
#else
        result = PonStdOamGetPeerStatus(olt_id, onu_id, Dot3OAMPEERMAXPDUSIZE, &peer_pdu_size, NULL);
#endif
		if (result == CTC_STACK_EXIT_OK)
		{
#if 0
            discovery_db.maximum_data_frame_size[olt_id][onu_id] = statistics_data.pdu_configuration;            
#else
            discovery_db.maximum_data_frame_size[olt_id][onu_id] = (unsigned short)peer_pdu_size;            
#endif
		}
		else
		{
#if 0
			PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT %d ONU %d failed in PAS_get_raw_statistics for receiving auto negotiation length. error code:%d\n", olt_id, onu_id, result)
#else
			PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT %d ONU %d failed in PonStdOamGetPeerStatus for receiving auto negotiation length. error code:%d\n", olt_id, onu_id, result)
#endif
			discovery_db.maximum_data_frame_size[olt_id][onu_id] = PASCOMM_ONU_FIRMWARE_BINARY_CHUNK_DEFAULT_SIZE;
		}
    }
#if 0
    else
    {                            
        discovery_db.maximum_data_frame_size[olt_id][onu_id] = PASCOMM_ONU_FIRMWARE_BINARY_CHUNK_DEFAULT_SIZE;

    }
#endif
    

    /* send an extended OAM Information message specifying the OUI and version given at 
       initialization of the package*/
    while( (resend == TRUE) && (sending_index < CTC_STACK_COMMUNICATION_RESEND_LIMIT) )
    {
        result = Send_receive_extended_oam_information
                               ( olt_id, onu_id, 
                                 discovery_db.list_info.number_of_oui_records, 
                                 discovery_db.list_info.records_list,
                                 &received_ext_support,
                                 &receive_number_of_records, 
                                 receive_oui_version_record_array );

        if ( result != CTC_STACK_EXIT_OK )
        { 
            PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id,"failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)

            discovery_db.state[olt_id][onu_id] = CTC_DISCOVERY_STATE_UNSUPPORTED;

            if ( event_report )
            {
    			for (client = 0; client < PON_MAX_CLIENT; client++)
    			{
    				PON_general_get_handler_function
    								   ( &ctc_db_handlers,
    									 CTC_STACK_HANDLER_END_OAM_DISCOVERY,
    									 client ,
    									 &the_handler );

    				if (the_handler != NULL)
    				{
    					the_handler (	olt_id, onu_id, 
    									CTC_DISCOVERY_STATE_UNSUPPORTED, 0/*records number*/ ,NULL/*records array*/);
    				}
    			}
            }
            
            return CTC_STACK_EXIT_OK; 
        }


        sending_index++;

        /* check whether the ONU 'support' */
        result = Check_if_onu_support( olt_id, onu_id, 
                                       received_ext_support,
                                       receive_number_of_records,
                                       receive_oui_version_record_array, 
                                       &support);

        if ( result != CTC_STACK_EXIT_OK )
        { 
            PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)

            discovery_db.state[olt_id][onu_id] = CTC_DISCOVERY_STATE_UNSUPPORTED;

            if ( event_report )
            {
    			for (client = 0; client < PON_MAX_CLIENT; client++)
    			{
    				PON_general_get_handler_function
    								   ( &ctc_db_handlers,
    									 CTC_STACK_HANDLER_END_OAM_DISCOVERY,
    									 client ,
    									 &the_handler );

    				if (the_handler != NULL)
    				{
    					the_handler (	olt_id, onu_id, 
    									CTC_DISCOVERY_STATE_UNSUPPORTED, receive_number_of_records ,receive_oui_version_record_array);
    				}
    			}
            }

            return CTC_STACK_EXIT_OK; 
        }


#ifdef TIMEOUT_PATCH_FROM_ONU_DURING_DISCOVERY
        if(support == TRUE)
        {
            resend = FALSE;
        }
        else
        {
            result = Check_first_version(  receive_oui_version_record_array, 
                                           &correct_oui_zero_version );
            CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

            if( correct_oui_zero_version )
            {
                resend = FALSE;
            }
        }
#else
        resend = FALSE;

#endif  
    }


    if(support == TRUE)    
    {
        PONLOG_INFO_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT:%d ONU:%d pass first check\n", olt_id, onu_id );

        /* set ONU state to IN_PROGRESS */
        discovery_db.state[olt_id][onu_id] = CTC_DISCOVERY_STATE_IN_PROGRESS;
    }
    else
    {

        PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT:%d ONU :%d not suppoted CTC stack (first response failed).data path is blocked\n",
                           olt_id, onu_id );
        discovery_db.state[olt_id][onu_id] = CTC_DISCOVERY_STATE_UNSUPPORTED;

        if ( event_report )
        {
    		for (client = 0; client < PON_MAX_CLIENT; client++)
    		{
    			PON_general_get_handler_function
    							   ( &ctc_db_handlers,
    								 CTC_STACK_HANDLER_END_OAM_DISCOVERY,
    								 client ,
    								 &the_handler );

    			if (the_handler != NULL)
    			{
    				the_handler (	olt_id, onu_id, CTC_DISCOVERY_STATE_UNSUPPORTED,  
    								receive_number_of_records ,receive_oui_version_record_array);
    			}
    		}
        }
        return CTC_STACK_EXIT_OK;
    }


    /* send an extended OAM Information message with OUI and Version given at
       initialization of the package */
    result = Send_receive_extended_oam_information
                           ( olt_id, onu_id, 
                             0, NULL,
                             &received_ext_support,
                             &receive_number_of_records, 
                             receive_oui_version_record_array );
    if ( result != CTC_STACK_EXIT_OK )
    { 
        PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)

        discovery_db.state[olt_id][onu_id] = CTC_DISCOVERY_STATE_UNSUPPORTED;

        if ( event_report )
        {
    		for (client = 0; client < PON_MAX_CLIENT; client++)
    		{
    			PON_general_get_handler_function
    							   ( &ctc_db_handlers,
    								 CTC_STACK_HANDLER_END_OAM_DISCOVERY,
    								 client ,
    								 &the_handler );

    			if (the_handler != NULL)
    			{
    				the_handler (	olt_id, onu_id, CTC_DISCOVERY_STATE_UNSUPPORTED,  
    								0/*records number*/ ,NULL/*records array*/);
    			}
    		}
        }
        return CTC_STACK_EXIT_OK; 
    }


    /* check whether the ONU 'support' */
    result = Check_if_onu_support( olt_id, onu_id, 
                                   received_ext_support,
                                   receive_number_of_records, 
                                   receive_oui_version_record_array, 
                                   &support );
    if ( result != CTC_STACK_EXIT_OK )
    { 
        PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)

        discovery_db.state[olt_id][onu_id] = CTC_DISCOVERY_STATE_UNSUPPORTED;

        if ( event_report )
        {
    		for (client = 0; client < PON_MAX_CLIENT; client++)
    		{
    			PON_general_get_handler_function
    							   ( &ctc_db_handlers,
    								 CTC_STACK_HANDLER_END_OAM_DISCOVERY,
    								 client ,
    								 &the_handler );

    			if (the_handler != NULL)
    			{
    				the_handler (	olt_id, onu_id, CTC_DISCOVERY_STATE_UNSUPPORTED,  
    								receive_number_of_records, receive_oui_version_record_array);
    			}
    		}
        }

        return CTC_STACK_EXIT_OK; 
    }


    if(support == TRUE)    
    {
#if 0
        PAS_pmc_onu_version_t pmc_onu_version;
#else
        unsigned short vendor_type;
        unsigned short chip_type;
        unsigned short rev_type;
#endif

        PONLOG_INFO_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT:%d ONU:%d pass second check\n", olt_id, onu_id );


        /* Check if the ONU is PMC ONU */
#if 0
        result = PAS_check_pmc_onu_version(olt_id,onu_id,&pmc_onu_version);
#else
        result = PON_CTC_STACK_get_onu_ponchip_vendor(olt_id,onu_id,&vendor_type,&chip_type,&rev_type);
#endif
        if(result != PAS_EXIT_OK)
        {
            PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT:%d ONU :%d failed checking for PMC ONU (with code %d).\n", olt_id, onu_id, result );
        }

#ifdef PON_CHIPTYPE_KNOWN
        discovery_db.pon_chip_vendor[olt_id][onu_id] = vendor_type;
#endif

        /* set ONU state to COMPLETE */
        discovery_db.state[olt_id][onu_id] = CTC_DISCOVERY_STATE_COMPLETE;


        PONLOG_INFO_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT:%d ONU:%d Extended OAM Discovery process completed\n", olt_id, onu_id );


        if ( event_report )
        {
    		for (client = 0; client < PON_MAX_CLIENT; client++)
    		{
    			PON_general_get_handler_function
    							   ( &ctc_db_handlers,
    								 CTC_STACK_HANDLER_END_OAM_DISCOVERY,
    								 client ,
    								 &the_handler );

    			if (the_handler != NULL)
    			{
    				the_handler (	olt_id, onu_id, CTC_DISCOVERY_STATE_COMPLETE,  
    								receive_number_of_records, receive_oui_version_record_array);
    			}
    		}
        }


#ifdef CTC_AUTH_SUPPORT
        /*After Extended OAM Discovery process completed authorize according to the olt authentication mode*/
        if(authentication_db.olt[olt_id].auth_mode == CTC_AUTH_MODE_LOID
            || authentication_db.olt[olt_id].auth_mode == CTC_AUTH_MODE_LOID_NOT_CHK_PWD)
        {
          result = CTC_STACK_start_loid_authentication(olt_id, onu_id, &loid_auth_success);
          if(result != PAS_EXIT_OK)
          {
              PONLOG_ERROR_3( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT:%d ONU :%d failed to start loid authentication. error code: %d\n", olt_id, onu_id, result );
          }

          if(!loid_auth_success)
          {
            result = PAS_deregister_onu( olt_id, onu_id, FALSE);
            if(result != PAS_EXIT_OK)
            {
                PONLOG_ERROR_3( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT:%d ONU :%d failed to deregister onu. error code: %d\n", olt_id, onu_id, result );
            }
          }
        }

        if( loid_auth_success || (authentication_db.olt[olt_id].auth_mode == CTC_AUTH_MODE_MAC) )
        {
            result = PAS_authorize_onu ( olt_id, onu_id, PON_AUTHORIZE_TO_THE_NETWORK );
            if(result != PAS_EXIT_OK)
            {
                PONLOG_ERROR_3( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT:%d ONU :%d failed to authorize (with code %d). data path is blocked \n", olt_id, onu_id, result );
            }
            else
            {
                if ( event_report )
                {
                    for (client = 0; client < PON_MAX_CLIENT; client++)
                    {
                        PON_general_get_handler_function
                                           ( &ctc_db_handlers,
                                             CTC_STACK_HANDLER_ONU_AUTHORIZE_TO_THE_NETWORK,
                                             client ,
                                             &the_handler );
                        
                        if (the_handler != NULL)
                        {
                            the_handler (olt_id, onu_id, PON_AUTHORIZE_TO_THE_NETWORK);
                        }
                    }
                }
            }
        }
#endif        
    }
    else
    {
        PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT:%d ONU :%d not suppoted CTC stack (second response failed). data path is blocked \n", olt_id, onu_id );
        discovery_db.state[olt_id][onu_id] = CTC_DISCOVERY_STATE_UNSUPPORTED;


        if ( event_report )
        {
    		for (client = 0; client < PON_MAX_CLIENT; client++)
    		{
    			PON_general_get_handler_function
    							   ( &ctc_db_handlers,
    								 CTC_STACK_HANDLER_END_OAM_DISCOVERY,
    								 client ,
    								 &the_handler );

    			if (the_handler != NULL)
    			{
    				the_handler (	olt_id, onu_id, CTC_DISCOVERY_STATE_UNSUPPORTED,  
    								receive_number_of_records, receive_oui_version_record_array);
    			}
    		}
        }
    }

    if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
    {
        if(internal_ctc_stack_automatic_onu_configuration_mode)
        {
            CTC_management_object_index_t management_object;
            management_object.frame_number = CTC_MIN_MANAGEMENT_OBJECT_FRAME_NUMBER;
            management_object.slot_number = CTC_MANAGEMENT_OBJECT_ALL_SLOTS;
            management_object.port_number = CTC_MANAGEMENT_OBJECT_ALL_PORTS;
            management_object.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
            
            PON_CTC_STACK_set_management_object_phy_admin_control (olt_id, onu_id, management_object, ENABLE);
        }
    
#if 0
        /*set frame rate to 100 frames per second*/
        result = Convert_ponsoft_error_code_to_ctc_stack(
            PAS_set_oam_frame_rate(olt_id, onu_id, PON_OAM_FRAME_RATE_100_FRAMES));
        if ( result != CTC_STACK_EXIT_OK )
        {
            PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id,"OLT: %d ONU: %d failed to set oam frame rate. error code:%d\n", olt_id, onu_id, result)
                return result;
        }
#endif
    }
    else
    {
        if(internal_ctc_stack_automatic_onu_configuration_mode)
        {
            PON_CTC_STACK_set_phy_admin_control(olt_id, onu_id, CTC_STACK_ALL_PORTS, ENABLE);
        }
    }
     

    return CTC_STACK_EXIT_OK;
   
}
#undef PONLOG_FUNCTION_NAME

/* B--added by liwei056@2011-12-5 for 6700's CTCOnuMgtFailed After SwitchOver */
static int extend_discovery_process 
				   ( const PON_olt_id_t					  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const mac_address_t				  mac_address,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard )
{
    return Registration_handler_discovery_process2 
    				   ( olt_id, 
    					 onu_id, 
    					 mac_address,
    					 authentication_sequence,
    					 supported_oam_standard,
    					 1 );
}


static int resume_discovery_process 
				   ( const PON_olt_id_t					  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const mac_address_t				  mac_address,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard )
{
    return Registration_handler_discovery_process2 
    				   ( olt_id, 
    					 onu_id, 
    					 mac_address,
    					 authentication_sequence,
    					 supported_oam_standard,
    					 0 );
}
/* E--added by liwei056@2011-12-5 for 6700's CTCOnuMgtFailed After SwitchOver */


#ifdef CTC_AUTH_SUPPORT
#define PONLOG_FUNCTION_NAME  "Onu_authorization_handler"

static int Onu_authorization_handler 
				   ( const PON_olt_id_t						olt_id,
					 const PON_onu_id_t					    onu_id,
					 const PON_authorize_mode_t             authorize_mode )
{
    short int						result;
    bool                            loid_auth_success = FALSE;
	short int						client;
	general_handler_function_t		the_handler;

    if(authentication_db.olt[olt_id].auth_mode != CTC_AUTH_MODE_HYBRID
        && authentication_db.olt[olt_id].auth_mode != CTC_AUTH_MODE_HYBRID_NOT_CHK_PWD)
    {
        PONLOG_ERROR_2( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT:%d ONU :%d received unexpected onu authorization event\n", olt_id, onu_id);
        return CTC_STACK_EXIT_ERROR;
    }


    if(authorize_mode == PON_DENY_FROM_THE_NETWORK)
    {
        result = CTC_STACK_start_loid_authentication(olt_id, onu_id, &loid_auth_success);
        if(result != PAS_EXIT_OK)
        {
            PONLOG_ERROR_3( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT:%d ONU :%d failed to start loid authentication. error code: %d\n", olt_id, onu_id, result );
        }
        
        if(!loid_auth_success)
        {
            result = PAS_deregister_onu( olt_id, onu_id, FALSE);
            if(result != PAS_EXIT_OK)
            {
                PONLOG_ERROR_3( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT:%d ONU :%d failed to deregister onu. error code: %d\n", olt_id, onu_id, result );
            }
        }
    }

    if( loid_auth_success || (authorize_mode == PON_AUTHORIZE_TO_THE_NETWORK) )
    {
        result = PAS_authorize_onu ( olt_id, onu_id, PON_AUTHORIZE_TO_THE_NETWORK );
        if(result != PAS_EXIT_OK)
        {
            PONLOG_ERROR_3( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT:%d ONU :%d failed to authorize (with code %d). data path is blocked \n", olt_id, onu_id, result );
        }
        else
        {
            for (client = 0; client < PON_MAX_CLIENT; client++)
            {
                PON_general_get_handler_function
                                   ( &ctc_db_handlers,
                                     CTC_STACK_HANDLER_ONU_AUTHORIZE_TO_THE_NETWORK,
                                     client ,
                                     &the_handler );
                
                if (the_handler != NULL)
                {
                    the_handler (olt_id, onu_id, PON_AUTHORIZE_TO_THE_NETWORK);
                }
            }
        }
    }


    return CTC_STACK_EXIT_OK; 
}
#undef PONLOG_FUNCTION_NAME
#endif


#define PONLOG_FUNCTION_NAME  "Deregistration_handler"

static int Deregistration_handler 
				   ( const PON_olt_id_t						olt_id,
					 const PON_onu_id_t					    onu_id,
					 const PON_onu_deregistration_code_t    deregistration_code )
{
    unsigned short			 index;

    /* deny from the network is done by the OLT FW 
    result = PAS_authorize_onu ( olt_id, onu_id, PON_DENY_FROM_THE_NETWORK ); */
    

    Reset_onu_entry(olt_id, onu_id);
    Reset_ctc_onu_timeout_database(olt_id, onu_id);

#ifdef CTC_AUTH_SUPPORT
    for(index = 0; index < PON_ONU_ID_PER_OLT_ARRAY_SIZE; index++)
    {
        if (authentication_db.olt[olt_id].onu_llid[index] == onu_id)
        {
            authentication_db.olt[olt_id].onu_llid[index] = PON_NOT_USED_LLID;
            break;
        }
    }
#endif

    PONLOG_INFO_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, "Deregistration in OLT:%d ONU :%d \n", olt_id, onu_id );
    
    return CTC_STACK_EXIT_OK; 
}
#undef PONLOG_FUNCTION_NAME


int ctc_pon_event_handler(PON_event_handler_index_t event_id, void *event_data)
{
    int result = CTC_STACK_EXIT_ERROR;
    
    switch ( event_id )
    {
        case PON_EVT_HANDLER_LLID_END_STD_OAM_DISCOVERY:
        {
            ONURegisterInfo_S *OnuRegisterData;

            if ( NULL != (OnuRegisterData = (ONURegisterInfo_S*)event_data) )
            {
                result = extend_discovery_process(OnuRegisterData->olt_id, OnuRegisterData->onu_id, OnuRegisterData->mac_address, OnuRegisterData->authenticatioin_sequence, OnuRegisterData->supported_oam_standard);
            }
        }
        
        break;
        case PON_EVT_HANDLER_LLID_DEREGISTER_EVENT:
        {
            ONUDeregistrationInfo_S *OnuDeregistrationData;

            if ( NULL != (OnuDeregistrationData = (ONUDeregistrationInfo_S*)event_data) )
            {
                result = Deregistration_handler(OnuDeregistrationData->olt_id, OnuDeregistrationData->onu_id, OnuDeregistrationData->deregistration_code);
            }
        }
        
        break;
        case PON_EVT_HANDLER_OLT_ADD:
            result = New_olt_added_handler((PON_olt_id_t)PTR2DATA(event_data));
        break;
        case PON_EVT_HANDLER_OLT_RMV:
            result = Removed_olt_handler((PON_olt_id_t)PTR2DATA(event_data));
        break;
        default:
            VOS_ASSERT(0);
    }

    return result;
}

 
static short int Send_receive_vendor_extension_message_with_timeout  ( 
                      const PON_olt_id_t                 olt_id, 
                      const PON_onu_id_t                 onu_id,
                      unsigned char                     *send_data,  
                      const unsigned short               send_data_size,
					  const unsigned short               timeout_in_100_ms_param,
                      const unsigned char                expected_opcode,
                      const unsigned char                expected_code,
                      unsigned char                     *received_data,
                      unsigned short                    *received_data_size,
                      unsigned short                     allocated_received_data_size,
                      const general_handler_function_t   handler_function)
{
    unsigned short length = send_data_size + OAM_OUI_SIZE;
	unsigned short timeout_in_100_ms = timeout_in_100_ms_param;

    /* insert CTC oui */
    send_data[0]   = (unsigned char)((ctc_stack_general_information.ctc_stack_oui >>(BYTES_IN_WORD*BITS_IN_BYTE))& 0xff);
    send_data[1]   = (unsigned char)((ctc_stack_general_information.ctc_stack_oui >>BITS_IN_BYTE)& 0xff);
    send_data[2]   = (unsigned char)(ctc_stack_general_information.ctc_stack_oui & 0xff);

    return (Send_receive_ctc_message
                ( olt_id, onu_id, PON_OAM_CODE_VENDOR_EXTENSION, send_data, length,
                  &timeout_in_100_ms, expected_opcode, expected_code, received_data, received_data_size, allocated_received_data_size, handler_function, 0));
}


static short int Send_receive_vendor_extension_message  ( 
                      const PON_olt_id_t                 olt_id, 
                      const PON_onu_id_t                 onu_id,
                      unsigned char                     *send_data,  
                      const unsigned short               send_data_size,
                      const unsigned char                expected_opcode,
                      const unsigned char                expected_code,
                      unsigned char                     *received_data,
                      unsigned short                    *received_data_size,
                      unsigned short                     allocated_received_data_size,
                      const general_handler_function_t   handler_function)
{
    unsigned short length = send_data_size + OAM_OUI_SIZE;
	unsigned short timeout_in_100_ms = CTC_STACK_COMMUNICATION_TIMEOUT;

    /* insert CTC oui */
    send_data[0]   = (unsigned char)((ctc_stack_general_information.ctc_stack_oui >>(BYTES_IN_WORD*BITS_IN_BYTE))& 0xff);
    send_data[1]   = (unsigned char)((ctc_stack_general_information.ctc_stack_oui >>BITS_IN_BYTE)& 0xff);
    send_data[2]   = (unsigned char)(ctc_stack_general_information.ctc_stack_oui & 0xff);

#if 0
    {

/*    	char  * string = NULL;
    	long len = 0x400;

		string = PON_MALLOC_FUNCTION(len);*/

		char string[0x1000] = "";
		long len = 0x1000;

/*		if(string)
		{*/
        	memset(string, 0, len);
        	PON_general_hex_bytes_to_string(send_data, length, string, len);

        	cl_vty_console_out("\r\n\r\n%s\r\n\r\n", string);
			/*PON_FREE_FUNCTION(string);*/
/*		}*/


    }
#endif

    return (Send_receive_ctc_message
                ( olt_id, onu_id, PON_OAM_CODE_VENDOR_EXTENSION, send_data, length,
                  &timeout_in_100_ms, expected_opcode, expected_code, received_data, received_data_size, allocated_received_data_size, handler_function, 0));
}
      

static short int Send_receive_vendor_extension_message_with_time_out_request_id  ( 
                      const PON_olt_id_t                 olt_id, 
                      const PON_onu_id_t                 onu_id,
                      unsigned char                     *send_data,  
                      const unsigned short               send_data_size,
					  unsigned short*                   timeout_in_100_ms,
                      const unsigned char                expected_opcode,
                      const unsigned char                expected_code,
                      unsigned char                     *received_data,
                      unsigned short                    *received_data_size,
                      unsigned short                     allocated_received_data_size,
                      const general_handler_function_t   handler_function,
					  const short int                    request_id)
{
	unsigned short length = send_data_size + OAM_OUI_SIZE;

    /* insert CTC oui */
    send_data[0]   = (unsigned char)((ctc_stack_general_information.ctc_stack_oui >>(BYTES_IN_WORD*BITS_IN_BYTE))& 0xff);
    send_data[1]   = (unsigned char)((ctc_stack_general_information.ctc_stack_oui >>BITS_IN_BYTE)& 0xff);
    send_data[2]   = (unsigned char)(ctc_stack_general_information.ctc_stack_oui & 0xff);


    return (Send_receive_ctc_message
                ( olt_id, onu_id, PON_OAM_CODE_VENDOR_EXTENSION, send_data, length,
                  timeout_in_100_ms, expected_opcode, expected_code, received_data, received_data_size, allocated_received_data_size, handler_function, request_id));

}

#define PONLOG_FUNCTION_NAME  "Send_receive_ctc_message"

static short int Send_receive_ctc_message  ( 
                      const PON_olt_id_t                 olt_id, 
                      const PON_onu_id_t                 onu_id,
                      const PON_oam_code_t               send_oam_code,
                      unsigned char                     *send_data,  
                      const unsigned short               send_data_size,
                      unsigned short*                   timeout_in_100_ms,
                      const unsigned char                expected_opcode,
                      const unsigned char                expected_code,
                      unsigned char                     *received_data,
                      unsigned short                    *received_data_size,
                      unsigned short                     allocated_received_data_size,
                      const general_handler_function_t   handler_function,
					  const short int                    request_id)
{

    if(onu_id != PON_BROADCAST_LLID)
    {
        if(discovery_db.maximum_data_frame_size[olt_id][onu_id] < (send_data_size+ETHERNET_FRAME_HEADER_SIZE+1) )
        {
            PONLOG_ERROR_4( PONLOG_FUNC_GENERAL, olt_id, onu_id,
                       "OLT %d ONU %d OAM PDU Configuration length is set to %d while frame length is %d\n",
                       olt_id, onu_id, discovery_db.maximum_data_frame_size[olt_id][onu_id],
                       (send_data_size+ETHERNET_FRAME_HEADER_SIZE+1) );

            return CTC_STACK_EXIT_ERROR;
        }
    }

    return (Convert_comm_error_code_to_ctc_stack(
                    Send_receive_ctc_oam
                            ( olt_id, onu_id, send_oam_code, send_data, send_data_size, timeout_in_100_ms,
                              expected_opcode, expected_code, received_data, received_data_size, 
                              allocated_received_data_size,handler_function, request_id)) );
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Convert_comm_error_code_to_ctc_stack"

static short int Convert_comm_error_code_to_ctc_stack( short int result)
{
    short int converted_result;

    switch( result )
    {
    case CTC_STACK_COMMUNICATION_EXIT_OK:
        converted_result = CTC_STACK_EXIT_OK;
        break; 
    case CTC_STACK_COMMUNICATION_UNEXPECTED_RESPONSE:
    case CTC_STACK_COMMUNICATION_EXIT_ERROR:
        converted_result = CTC_STACK_EXIT_ERROR;
        break; 
    case CTC_STACK_COMMUNICATION_TIME_OUT:
        converted_result = CTC_STACK_TIME_OUT;
        break; 
    case CTC_STACK_COMMUNICATION_NOT_IMPLEMENTED:
        converted_result = CTC_STACK_NOT_IMPLEMENTED;
        break; 
    case CTC_STACK_COMMUNICATION_PARAMETER_ERROR:
        converted_result = CTC_STACK_PARAMETER_ERROR;
        break; 
    case CTC_STACK_COMMUNICATION_HARDWARE_ERROR:
        converted_result = CTC_STACK_HARDWARE_ERROR;
        break; 
    case CTC_STACK_COMMUNICATION_MEMORY_ERROR:
        converted_result = CTC_STACK_MEMORY_ERROR;
        break; 
    case CTC_STACK_COMMUNICATION_OLT_NOT_EXIST:
        converted_result = CTC_STACK_OLT_NOT_EXIST;
        break; 
    case CTC_STACK_COMMUNICATION_ONU_NOT_AVAILABLE:
        converted_result = CTC_STACK_ONU_NOT_AVAILABLE;
        break;
    case CTC_STACK_COMMUNICATION_QUERY_FAILED:
        converted_result = CTC_STACK_QUERY_FAILED;
        break;
    default:
		PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Unsupported return code  %d \n", result );
        converted_result = CTC_STACK_EXIT_ERROR;
        
    }
    return converted_result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Convert_ctc_stack_error_codes_to_tftp"

static short int Convert_ctc_stack_error_codes_to_tftp( unsigned short ctc_error_code)
{
    unsigned short  tftp_error_code;

    switch( ctc_error_code )
    {
		case CTC_STACK_EXIT_ERROR:
		case CTC_STACK_PARAMETER_ERROR:
		case CTC_STACK_HARDWARE_ERROR:
		case CTC_STACK_MEMORY_ERROR:
			tftp_error_code = TFTP_GEN_ERROR;
			break;
		case CTC_STACK_QUERY_FAILED:
			tftp_error_code = TFTP_ILLEGAL_OPERATION;
			break;
		default:
        tftp_error_code = TFTP_GEN_ERROR;
        
    }

    return tftp_error_code;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Convert_ponsoft_error_code_to_ctc_stack"

static short int Convert_ponsoft_error_code_to_ctc_stack( short int pas_result)
{
	short int converted_result;

    switch( pas_result )
    {
    case PAS_EXIT_OK:
        converted_result = CTC_STACK_EXIT_OK;
        break;
    case PAS_PARAMETER_ERROR:
    case PAS_EXIT_ERROR:
        converted_result = CTC_STACK_EXIT_ERROR;
        break; 
    case PAS_NOT_IMPLEMENTED:
	case PAS_NOT_SUPPORTED_IN_CURRENT_HARDWARE_VERSION:
        converted_result = CTC_STACK_NOT_IMPLEMENTED;
        break;
    case PAS_TIME_OUT:
        converted_result = CTC_STACK_TIME_OUT;
        break;
	case ENTRY_ALREADY_EXISTS:
		converted_result = CTC_STACK_ALREADY_EXIST;
		break;
	case TABLE_FULL:
		converted_result = CTC_STACK_TABLE_FULL;
		break;
	case TABLE_EMPTY:
	case PAS_QUERY_FAILED:
		converted_result = CTC_STACK_QUERY_FAILED;
		break;
	case PAS_HARDWARE_ERROR:
		converted_result = CTC_STACK_HARDWARE_ERROR;
		break;
	case PAS_MEMORY_ERROR:
		converted_result = CTC_STACK_MEMORY_ERROR;
		break;
	case PAS_OLT_NOT_EXIST:
		converted_result = CTC_STACK_OLT_NOT_EXIST;
		break;
	case PAS_ONU_NOT_AVAILABLE:
		converted_result = CTC_STACK_ONU_NOT_AVAILABLE;
		break;
    default:

		PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Unsupported return code  %d\n", pas_result );
        converted_result = CTC_STACK_EXIT_ERROR;
        
    }
    return converted_result;

}
#undef PONLOG_FUNCTION_NAME

                             
#define PONLOG_FUNCTION_NAME  "Convert_comparser_error_code_to_ctc_stack"

static short int Convert_comparser_error_code_to_ctc_stack( short int result)
{
    short int converted_result;

    switch( result )
    {
    case COMPARSER_STATUS_OK:
	case COMPARSER_CTC_STATUS_SUCCESS:
        converted_result = CTC_STACK_EXIT_OK;
        break;
    case COMPARSER_STATUS_WRONG_OPCODE:
    case COMPARSER_STATUS_ERROR:
	case COMPARSER_STATUS_CTC_FAIL:
        converted_result = CTC_STACK_EXIT_ERROR;
        break; 
    case COMPARSER_STATUS_NOT_SUPPORTED:
        converted_result = CTC_STACK_NOT_IMPLEMENTED;
        break;
    case COMPARSER_STATUS_BAD_PARAM:
        converted_result = CTC_STACK_PARAMETER_ERROR;
        break;
    case COMPARSER_STATUS_MEMORY_ERROR:
        converted_result = CTC_STACK_MEMORY_ERROR;
        break;
    case COMPARSER_STATUS_TIMEOUT:
        converted_result = CTC_STACK_TIME_OUT;
        break;
	case COMPARSER_CTC_STATUS_BAD_PARAM:
        converted_result = CTC_STACK_VARIABLE_INDICATION_BAD_PARAM;
        break;
	case COMPARSER_CTC_STATUS_NO_RESOURCE:
        converted_result = CTC_STACK_VARIABLE_INDICATION_NO_RESOURCE;
        break;
    default:
		PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Unsupported return code  %d \n", result );
        converted_result = CTC_STACK_EXIT_ERROR;
        
    }
    return converted_result;

}
#undef PONLOG_FUNCTION_NAME



static void Copy_oui( 
           const OAM_oui_t  src_oui,
           OAM_oui_t        dest_oui )
{
    unsigned char       i;
   
    for(i = 0; i < OAM_OUI_SIZE;i++)
    {
       dest_oui[i] = src_oui[i];
    }
}


static bool Compare_oui_as_number 
                ( const unsigned long  oui_as_number,
				  const OAM_oui_t      oui_as_array )
{
    

    if( (oui_as_array[0] != (unsigned char)((oui_as_number >>(BYTES_IN_WORD*BITS_IN_BYTE))& 0xff) ) ||
        (oui_as_array[1] != (unsigned char)((oui_as_number >>BITS_IN_BYTE)& 0xff) ) ||
        (oui_as_array[2] != (unsigned char)(oui_as_number & 0xff) ) )
    {
        return FALSE;
    }

    return TRUE; 
}


static void Copy_oui_as_number( 
           const unsigned long   src_oui,
           OAM_oui_t			 dest_oui )
{
    dest_oui[0]   = (unsigned char)((src_oui >>(BYTES_IN_WORD*BITS_IN_BYTE))& 0xff);
    dest_oui[1]   = (unsigned char)((src_oui >>BITS_IN_BYTE)& 0xff);
    dest_oui[2]   = (unsigned char)(src_oui & 0xff);
}


#define PONLOG_FUNCTION_NAME  "Check_first_version"
                                   
static short int Check_first_version
                    ( const CTC_STACK_oui_version_record_t  *receive_oui_version_record_array,
                      bool                            *correct_oui_zero_version)
{
   

   /* check first record if it's feet to the expected parameters */
   if( (Compare_oui_as_number(ctc_stack_general_information.ctc_stack_oui, receive_oui_version_record_array[0].oui )) &&
      (receive_oui_version_record_array[0].version == 0))
   {
       *(correct_oui_zero_version) = TRUE;
   }
   else
   {
       *(correct_oui_zero_version) = FALSE;
   }
   
    return CTC_STACK_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Check_if_onu_support"
                                   
static short int Check_if_onu_support
                    ( const PON_olt_id_t               olt_id, 
					  const PON_onu_id_t               onu_id,
                      const bool                       received_ext_support,
                      const unsigned char 	           receive_number_of_records,
                      const CTC_STACK_oui_version_record_t  *receive_oui_version_record_array,
                      bool                            *support)
{
    unsigned char largest_common_version;

    (*support) = FALSE;

     
    /* check if ONU support CTC stack protocol */
    if(received_ext_support != TRUE)
    {
        PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id,"OLT:%d ONU:%d is not supported CTC stack ext_support= %d \n", 
                           olt_id, onu_id, (*support) );


        return CTC_STACK_EXIT_OK;
    }


    switch(discovery_db.state[olt_id][onu_id])
    {
    case CTC_DISCOVERY_STATE_IDLE:

       
            /* check first record if it's feet to the expected parameters */
            if( (Compare_oui_as_number(ctc_stack_general_information.ctc_stack_oui, receive_oui_version_record_array[0].oui )) &&
                (receive_oui_version_record_array[0].version == 0))
            {
                /* check other records if it's feet to initialize package parameters */
                (*support) = Search_largest_common_oui_version(receive_number_of_records,
                                                               receive_oui_version_record_array,
                                                               &largest_common_version);
                if( (*support) != TRUE)
                {
                      PONLOG_ERROR_4( PONLOG_FUNC_GENERAL, olt_id, onu_id,"OLT:%d ONU:%d has no match to oui: 0x%x version: %d\n", 
                                         olt_id, onu_id, ctc_stack_general_information.ctc_stack_oui, ctc_stack_general_information.version );
                }
                else
                {    /*update extended OAM discovery version information to largest_common_version found*/
                     discovery_db.common_version[olt_id][onu_id] = largest_common_version;
                }
            } 
            else
            {
                /* debug printing */

                if( !(Compare_oui_as_number(ctc_stack_general_information.ctc_stack_oui, receive_oui_version_record_array[0].oui )) )
                {
                    PONLOG_ERROR_6( PONLOG_FUNC_GENERAL, olt_id, onu_id,"OLT:%d ONU:%d first oui: expected  0x%x,received 0x%x%x%x \n", olt_id, onu_id, 
                                        ctc_stack_general_information.ctc_stack_oui, receive_oui_version_record_array[0].oui[0],
                                        receive_oui_version_record_array[0].oui[1],
                                        receive_oui_version_record_array[0].oui[2] );
                }
                else
                {
                    PONLOG_ERROR_4( PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT:%d ONU:%d first version: expected %d,received %d \n", olt_id, onu_id, 
                                        0, receive_oui_version_record_array[0].version );

                }       
            }
        
        break;
    case CTC_DISCOVERY_STATE_IN_PROGRESS:
        
            /* check first records if it's feet to initialize package parameters */
            if( (Compare_oui_as_number(ctc_stack_general_information.ctc_stack_oui, receive_oui_version_record_array[0].oui )) &&
                (receive_oui_version_record_array[0].version == discovery_db.common_version[olt_id][onu_id]))
            {     
                (*support) = TRUE;
            }
            else
            {
                PONLOG_ERROR_4( PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT:%d ONU:%d first record is not matched to oui: 0x%x version: %d\n", 
                                   olt_id, onu_id, ctc_stack_general_information.ctc_stack_oui, ctc_stack_general_information.version );
            }
        
        break;
    default:
        PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id, "OLT:%d ONU:%d state is %d not supported\n", 
                           olt_id, onu_id, discovery_db.state[olt_id][onu_id] );
        return CTC_STACK_EXIT_ERROR;
    }


    return CTC_STACK_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME



static short int Copy_encryption_key
                    ( const CTC_encryption_key_t  src_key, 
                      CTC_encryption_key_t        dest_key)
{
    unsigned char i;

    for(i = 0; i < CTC_ENCRYPTION_KEY_SIZE;i++)
    {
        dest_key[i] = src_key[i];
    }

    return CTC_STACK_EXIT_OK;
}


#define PONLOG_FUNCTION_NAME  "Set_encryption_db_flag_entry"

static short int Set_encryption_db_flag_entry  ( 
                   const PON_olt_id_t                           olt_id,
                   const PON_onu_id_t                           onu_id,
                   const ctc_stack_response_encryption_state_t  state)
{


    short int                   result = CTC_STACK_EXIT_OK;
  
    
    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_db.olt[olt_id].semaphore), 
                                        ctc_stack_encryption_semaphore,result );
    if (result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);
 

     encryption_db.olt[olt_id].onu_id[onu_id].response_encryption_state = state;
    

    /* END PROTECTION */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_db.olt[olt_id].semaphore),
                                         ctc_stack_encryption_semaphore,result );
    if (result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);


    return result;
    
}
#undef PONLOG_FUNCTION_NAME




#define PONLOG_FUNCTION_NAME  "Check_encryption_state_if_already_received"

static short int Check_encryption_state_if_already_received  ( 
                   const PON_olt_id_t   olt_id,
                   const PON_onu_id_t   onu_id)
{


    short int	sem_result, result = CTC_STACK_EXIT_OK;
  
    
    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_db.olt[olt_id].semaphore), 
                                        ctc_stack_encryption_semaphore,sem_result );
    if (sem_result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);
 

	if(encryption_db.olt[olt_id].onu_id[onu_id].response_encryption_state == CTC_STACK_STATE_RECEIVED_FRAME)
	{
		PONLOG_INFO_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id,
				   "OLT %d ONU already received key response. does not handle the last response yet\n",
				   olt_id, onu_id );

		result = CTC_STACK_ONU_NOT_AVAILABLE;
	}
	else
		encryption_db.olt[olt_id].onu_id[onu_id].response_encryption_state = CTC_STACK_STATE_WAITING_FOR_RESPONSE;
    

    /* END PROTECTION */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_db.olt[olt_id].semaphore),
                                         ctc_stack_encryption_semaphore,sem_result );
    if (sem_result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);


    return result;
    
}
#undef PONLOG_FUNCTION_NAME




#define PONLOG_FUNCTION_NAME  "Update_encryption_db"

static short int Update_encryption_db  ( 
                   const PON_olt_id_t                olt_id,
                   const PON_onu_id_t                onu_id,
                   const PON_encryption_key_index_t  key_index,
                   const CTC_encryption_key_t        key_value )
{


    short int                   result = CTC_STACK_EXIT_OK;
  
    
    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_db.olt[olt_id].semaphore), 
                                        ctc_stack_encryption_semaphore,result );
    if (result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);
 

	encryption_db.olt[olt_id].onu_id[onu_id].temp_key_index = key_index;
    encryption_db.olt[olt_id].onu_id[onu_id].response_encryption_state = CTC_STACK_STATE_RECEIVED_FRAME;
    

    result = Copy_encryption_key(key_value, encryption_db.olt[olt_id].onu_id[onu_id].key_value);
    if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_3(PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "failed to copy encryption key in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)
    }
    

    /* END PROTECTION */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_db.olt[olt_id].semaphore),
                                         ctc_stack_encryption_semaphore,result );
    if (result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);


    return result;
    
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Set_encryption_db_entry"

static short int Set_encryption_db_entry  ( 
                   const PON_olt_id_t                  olt_id,
                   const PON_onu_id_t                  onu_id,
                   const CTC_STACK_encryption_state_t  encryption_state )
{


    short int                   result = CTC_STACK_EXIT_OK;
  
    
    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_db.olt[olt_id].semaphore), 
                                        ctc_stack_encryption_semaphore,result );
    if (result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);
 

    encryption_db.olt[olt_id].onu_id[onu_id].encryption_state = encryption_state;
    

    /* END PROTECTION */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &(encryption_db.olt[olt_id].semaphore),
                                         ctc_stack_encryption_semaphore,result );
    if (result != EXIT_OK) return (CTC_STACK_EXIT_ERROR);


    return result;
    
}
#undef PONLOG_FUNCTION_NAME



static void Reset_onu_entry(
                   const PON_olt_id_t                  olt_id,
                   const PON_onu_id_t                  onu_id)
{
    /* update discovery parameters */
    discovery_db.state[olt_id][onu_id] = CTC_DISCOVERY_STATE_IDLE;      

    /* update encryption parameters */
    Set_encryption_db_flag_entry(olt_id, onu_id, CTC_STACK_STATE_VALID);

    /* Those 3 parameters do no need a semaphore protection */
    encryption_db.olt[olt_id].onu_id[onu_id].key_index = 1;
	encryption_db.olt[olt_id].onu_id[onu_id].temp_key_index = 1;
    encryption_db.olt[olt_id].onu_id[onu_id].start_encryption_by_the_user = FALSE;
    encryption_db.olt[olt_id].onu_id[onu_id].number_of_sending_request = 0;
	memset(encryption_db.olt[olt_id].onu_id[onu_id].key_request_received_oam_pdu_data,0,100);
	encryption_db.olt[olt_id].onu_id[onu_id].key_request_received_length = 0;

    Set_encryption_db_entry(olt_id, onu_id, CTC_ENCRYPTION_STATE_NOT_ENCRYPTED);
}


#define PONLOG_FUNCTION_NAME  "Send_file_and_verify"
static short int Send_file_and_verify ( 
                 const PON_olt_id_t          olt_id, 
                 const PON_onu_id_t          onu_id,
                 const CTC_STACK_binary_t   *onu_firmware,
                 const bool                  send_write_request,
                 const unsigned char        *file_name)
{

    short int                      set_state_result, result=CTC_STACK_EXIT_OK;
#ifndef FILES_NOT_SUPPORTED
    unsigned long                  file_size = 0;
#else
    unsigned long                  file_size = onu_firmware->size;	/* modified by xieshl 20110801, source=memory时应直接取出文件大小 */
#endif


    result = Send_file(olt_id, onu_id, onu_firmware, send_write_request, file_name, &file_size);
    if(result != CTC_STACK_EXIT_OK)
    {
        set_state_result = Set_onu_state_in_update_firmware_db(olt_id, onu_id, CTC_STACK_ONU_STATE_NOT_DURING_DOWNLOADING);
        CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
    }
    taskDelay(20); /* 20110801 */
  
    result = Send_end_download_request(olt_id, onu_id, file_name, file_size);
    if(result != CTC_STACK_EXIT_OK)
    {
        set_state_result = Set_onu_state_in_update_firmware_db(olt_id, onu_id, CTC_STACK_ONU_STATE_NOT_DURING_DOWNLOADING);
        CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
    }

    PONLOG_INFO_2( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
                   "OLT %d ONU %d send File succeeded\n",
                   olt_id, onu_id );

    result = Set_onu_state_in_update_firmware_db(olt_id, onu_id, CTC_STACK_ONU_STATE_NOT_DURING_DOWNLOADING);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Send_write_request"

static short int Send_write_request ( 
               const PON_olt_id_t    olt_id, 
               const PON_onu_id_t    onu_id,
               const unsigned char  *file_name )
{
    short int						result,comparser_result;
	unsigned short					max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
    unsigned short			    	received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char					sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char					received_oam_pdu_data[ETHERNET_MTU]= {0};
    bool							success;
	unsigned short					received_block_number, error_code;
    unsigned char					error_msg[ONU_RESPONSE_SIZE];
	unsigned char					expected_opcode =  OAM_VENDOR_EXT_UPDATE_FIRMWARE;
    unsigned char					expected_code   =  TFTP_UPDATE_FIRMWARE_OPCODE_FILE_ACK;
	short int						client;
	general_handler_function_t		the_handler;
    TFTP_data_type_t                data_type = TFTP_DATA_TYPE_DATA;
    unsigned short                  payload_length = 2/*opcode*/ + strlen(file_name) + 1/*reserved*/ + strlen("Octet") + 1/*reserved*/ + TFTP_COMMON_HEADER;
	
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC


    comparser_result = (short int)Compose_tftp_common_header
                            ( data_type, payload_length, onu_id, TRUE,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    total_length += sent_length;
    sent_length = max_length - total_length;

    comparser_result = (short int)Compose_file_write_request
                            ( file_name,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    total_length += sent_length;
    sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
      
    
	comparser_result = (short int)Parse_tftp_common_header
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, onu_id, data_type, TRUE, &received_left_length );

    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)Parse_file_transfer_response
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER,
         	                          &success, &received_block_number, &error_code, error_msg, &received_left_length );

    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (success == TRUE) )
	{
		if(received_block_number != 0)
		{
			PONLOG_ERROR_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
							"OLT:%d ONU:%d receive block number: %d, expected: %d\n", 
							olt_id, onu_id, received_block_number, 0 );
		

			for (client = 0; client < PON_MAX_CLIENT; client++)
			{
				PON_general_get_handler_function
								   ( &ctc_db_handlers,
									 CTC_STACK_HANDLER_UPDATE_ONU_FIRMWARE_ERROR,
									 client ,
									 &the_handler );

				if (the_handler != NULL)
				{
					the_handler (	olt_id, onu_id,
									CTC_STACK_QUERY_FAILED, NULL);
				}
			}

			return CTC_STACK_TFTP_SEND_FAIL; 
		}
	}
	else /*success == FALSE*/
	{
		PONLOG_ERROR_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
						"OLT:%d ONU:%d received TFTP error code: %d, error message: %s\n", 
						olt_id, onu_id, error_code, error_msg );
		
		
		for (client = 0; client < PON_MAX_CLIENT; client++)
		{
			PON_general_get_handler_function
							   ( &ctc_db_handlers,
								 CTC_STACK_HANDLER_UPDATE_ONU_FIRMWARE_ERROR,
								 client ,
								 &the_handler );

			if (the_handler != NULL)
			{
				the_handler (	olt_id, onu_id,
								error_code, error_msg);
			}
		}

		return CTC_STACK_TFTP_SEND_FAIL; 

	}

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Send_file"

static short int Send_file(
			const PON_olt_id_t          olt_id, 
            const PON_onu_id_t          onu_id,
            const CTC_STACK_binary_t   *onu_firmware,
			const bool			        send_write_request,
			const unsigned char        *file_name,
			unsigned long              *file_size)
{
#ifndef FILES_NOT_SUPPORTED
    FILE						*stream;
#endif
	general_handler_function_t	 the_handler;
	short int					 client, result;
    int                          fseek_result;

	CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

	/* Only File and Memory sources are currently supported */
#ifndef FILES_NOT_SUPPORTED /* There is file system */
	if ((onu_firmware->source != PON_BINARY_SOURCE_FILE) && (onu_firmware->source != PON_BINARY_SOURCE_MEMORY))
	{
		PONLOG_ERROR_1(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "Error, ONU binary source (code %d) not supported\n", onu_firmware->source);
		return CTC_STACK_PARAMETER_ERROR;
	}
#else /* There isn't file system */
	if (onu_firmware->source != PON_BINARY_SOURCE_MEMORY)
	{
		PONLOG_ERROR_1(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "Error, ONU binary source (code %d) should be memory only\n", onu_firmware->source);
		return CTC_STACK_PARAMETER_ERROR;
	}
#endif

    if(send_write_request == TRUE)
	{
		result = Send_write_request(olt_id, onu_id, file_name);
		CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

		PONLOG_INFO_2( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
					   "OLT %d ONU %d send File write request succeed\n",
					   olt_id, onu_id );
	}

#ifndef FILES_NOT_SUPPORTED 
    if (onu_firmware->source == PON_BINARY_SOURCE_FILE)
    {
        /* open as binary for reading */
        stream = fopen( onu_firmware->location, "rb" );
    
        if( stream == NULL )
        {
            PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
                          "OLT %d ONU %d failed to open file :%s\n", olt_id, onu_id, onu_firmware->location)             
            
            for (client = 0; client < PON_MAX_CLIENT; client++)
            {
                PON_general_get_handler_function
                    ( &ctc_db_handlers,
                    CTC_STACK_HANDLER_UPDATE_ONU_FIRMWARE_ERROR,
                    client ,
                    &the_handler );
            
                if (the_handler != NULL)
                {
                    the_handler (	olt_id, onu_id, TFTP_FILE_NOT_FOUND, NULL);
                }
            }

            return CTC_STACK_PARAMETER_ERROR;
        }

        /* Set pointer to end of file */
        fseek_result = fseek( stream, 0L, SEEK_END );
        if ( fseek_result != FSEEK_SUCCESS ) 
        {
            PONLOG_ERROR_1(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "Error seeking the end of the binary file %s\n", onu_firmware->location);
            
            return CTC_STACK_PARAMETER_ERROR;
        }

        *file_size = ftell(stream);

        if (*file_size == -1)
        {
            PONLOG_ERROR_1(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "Error seeking the length of the binary file %s\n", onu_firmware->location);
            
            return CTC_STACK_PARAMETER_ERROR;
        }
    
        /* Set pointer to beginning of file */
        fseek_result = fseek( stream, 0L, SEEK_SET );
        if ( fseek_result != FSEEK_SUCCESS ) 
        {
            PONLOG_ERROR_1(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "Error seeking the beginning of the binary file %s\n", onu_firmware->location);
            
            return CTC_STACK_PARAMETER_ERROR;
        }

        result = Send_binary_source(olt_id, onu_id, onu_firmware, stream, file_name, *file_size);

    } else
#endif
    {
        result = Send_binary_source(olt_id, onu_id, onu_firmware, NULL, file_name, *file_size);
    }


#ifndef FILES_NOT_SUPPORTED 
    if (onu_firmware->source == PON_BINARY_SOURCE_FILE)
    {
        fclose( stream );
    }
#endif

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Send_binary_source"

static short int Send_binary_source(
			const PON_olt_id_t          olt_id, 
            const PON_onu_id_t          onu_id,
            const CTC_STACK_binary_t   *onu_firmware,
			void			           *stream,/*required only for FILE source*/
			const unsigned char        *file_name,
			const unsigned long         file_size)
{
	unsigned short				 block_number = 1;
	unsigned short				 frame_size;
	unsigned char				 expected_opcode =  OAM_VENDOR_EXT_UPDATE_FIRMWARE;
    unsigned char				 expected_code   =  TFTP_UPDATE_FIRMWARE_OPCODE_FILE_ACK;
	unsigned char				 sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char				 received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned short				 max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
    unsigned short			     received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	bool						 success, add_opcode;
	unsigned short				 received_block_number, check_received_block_number, error_code;
	unsigned char				 error_msg[ONU_RESPONSE_SIZE];
	general_handler_function_t	 the_handler;
    short int					 client, result, comparser_result, i;
    TFTP_data_type_t             data_type = TFTP_DATA_TYPE_DATA;
    unsigned short               payload_length            = 2/*opcode*/ +2/*block number*/ + CTC_TFTP_TRANSFER_DATA_BLOCK_SIZE + TFTP_COMMON_HEADER;
    unsigned short               payload_length_last_block = 2/*opcode*/ +2/*block number*/ + TFTP_COMMON_HEADER;
    unsigned short               last_block_size, num_of_blocks;
    unsigned short               maximum_data_frame_size = CTC_TFTP_TRANSFER_DATA_BLOCK_SIZE; 

#define NUM_OF_ENCAPSULATE_DATA_MESSAGE  1

    num_of_blocks   = (unsigned short)(file_size / CTC_TFTP_TRANSFER_DATA_BLOCK_SIZE);
    last_block_size = (unsigned short)(file_size % CTC_TFTP_TRANSFER_DATA_BLOCK_SIZE);
    
    /*payload length of last block can be different form 512*/
    if(last_block_size!= 0)
    {
        payload_length_last_block += last_block_size;
        num_of_blocks += 1;
    }

	while( block_number <= num_of_blocks ) 
    {
        /*compose encapsulate data*/
        total_length = 0;
        sent_length = ETHERNET_MTU;

        for (i = 0; i < NUM_OF_ENCAPSULATE_DATA_MESSAGE; i++)
        {
            if(i == 0)
            {
              add_opcode = TRUE;
            }
            else
            { /*second encapsulate data without opcode*/
              add_opcode = FALSE;
            }

            /*payload length of last block can be different form 512*/
            if((block_number == num_of_blocks) && (last_block_size != 0))
            {
                payload_length = payload_length_last_block;
                maximum_data_frame_size = last_block_size;
            }
            
            comparser_result = (short int)Compose_tftp_common_header
                ( data_type, payload_length, onu_id, add_opcode, 
                  SEND_COMPARSER_BUFFER, &sent_length );
            CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

            total_length += sent_length;
            sent_length = max_length - total_length;
                
            /*compose message*/
            comparser_result = Compose_file_transfer_data
                ( block_number,
                  SEND_COMPARSER_BUFFER, &sent_length );
            CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
               
            total_length += sent_length;
            sent_length = max_length - total_length;

    
#ifndef FILES_NOT_SUPPORTED 
            if (onu_firmware->source == PON_BINARY_SOURCE_FILE)
            {
                frame_size = fread( SEND_COMPARSER_BUFFER,
                                    sizeof(unsigned char), maximum_data_frame_size, ((FILE*)stream) );
            } else
#endif
            {
                frame_size = maximum_data_frame_size;
                memcpy (SEND_COMPARSER_BUFFER,
                        ((unsigned char *)onu_firmware->location + ((block_number-1)*CTC_TFTP_TRANSFER_DATA_BLOCK_SIZE)), frame_size);
            }

            total_length += frame_size;
            sent_length = max_length - total_length;

            
            PONLOG_INFO_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
                "OLT %d ONU %d preparing encapsulate message with block number %d length %d\n",
                olt_id, onu_id, block_number, frame_size );
            
            block_number++;
            
            /*no need for encapsulate, it was the last block*/
            if(block_number > num_of_blocks) 
            {   
                break;
            }
        }/*end of compose encapsulate data*/


        received_total_length = 0;
        received_left_length  = 0;
        
        result = Send_receive_vendor_extension_message 
            ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
            received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
        
        if ( result != CTC_STACK_EXIT_OK ) 
        { 
            PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
                "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)
                
                for (client = 0; client < PON_MAX_CLIENT; client++)
                {
                    PON_general_get_handler_function
                        ( &ctc_db_handlers,
                        CTC_STACK_HANDLER_UPDATE_ONU_FIRMWARE_ERROR,
                        client ,
                        &the_handler );
                    
                    if (the_handler != NULL)
                    {
                        the_handler (	olt_id, onu_id, result, NULL);
                    }
                }
                return result; 
        }

        
        /*parse encapsulate data*/
        for (i = 0; i < NUM_OF_ENCAPSULATE_DATA_MESSAGE; i++)
        {
            if(i == 0)
            {
              add_opcode = TRUE;
            }
            else
            { /*second encapsulate data without opcode*/
              add_opcode = FALSE;
            }

            comparser_result = (short int)Parse_tftp_common_header
                ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, onu_id, data_type, add_opcode, &received_left_length );
            
            CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
        
            received_total_length += received_left_length;
                
            comparser_result = (short int)Parse_file_transfer_response
                ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER,
                &success, &received_block_number, &error_code, error_msg, &received_left_length );
            
            CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
            
           	received_total_length += received_left_length;
   
            /*in case of encapsulate: first ack on first block*/
            check_received_block_number = received_block_number;

                if( (success == TRUE) )
                {
                    if((block_number-1) != check_received_block_number)
                    {
                        PONLOG_ERROR_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
                            "OLT:%d ONU:%d receive block number: %d, expected: %d\n", olt_id, onu_id, received_block_number, (block_number-1) );
                        
                        for (client = 0; client < PON_MAX_CLIENT; client++)
                        {
                            PON_general_get_handler_function
                                ( &ctc_db_handlers,
                                CTC_STACK_HANDLER_UPDATE_ONU_FIRMWARE_ERROR,
                                client ,
                                &the_handler );
                            
                            if (the_handler != NULL)
                            {
                                the_handler (	olt_id, onu_id, CTC_STACK_QUERY_FAILED, NULL);
                            }
                        }
                        
                        return CTC_STACK_TFTP_SEND_FAIL; 
                    }
                }
                else /*success == FALSE*/
                {
                    PONLOG_ERROR_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
                        "OLT:%d ONU:%d received TFTP error code: %d, error message: %s\n", 
                        olt_id, onu_id, error_code, error_msg );
                    
                    for (client = 0; client < PON_MAX_CLIENT; client++)
                    {
                        PON_general_get_handler_function
                            ( &ctc_db_handlers,
                            CTC_STACK_HANDLER_UPDATE_ONU_FIRMWARE_ERROR,
                            client ,
                            &the_handler );
                        
                        if (the_handler != NULL)
                        {
                            the_handler (	olt_id, onu_id, error_code, error_msg);
                        }
                    }
                    
                    return CTC_STACK_TFTP_SEND_FAIL; 
                }
                
                /*no need for encapsulate, it was the last block*/
                if(received_block_number == num_of_blocks) 
                {
                    break;
                }
                
        }/*end of parse encapsulate data*/
        
    }/*end while*/
        
	return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Send_end_download_request"

static short int Send_end_download_request(
			const PON_olt_id_t    olt_id, 
            const PON_onu_id_t    onu_id,
            const unsigned char  *file_name,
            const unsigned long   file_size)
{
    short int                result,comparser_result; 
	unsigned short			 max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
    unsigned short			 received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char			 sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char			 received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char			 expected_opcode = OAM_VENDOR_EXT_UPDATE_FIRMWARE;
    unsigned char			 expected_code   = TFTP_UPDATE_FIRMWARE_OPCODE_END_DOWNLOAD_RESPONSE;
    TFTP_data_type_t         data_type       = TFTP_DATA_TYPE_FILE_INTEGRITY;
    unsigned short           payload_length  = 2/*opcode*/ + strlen(file_name) + 4/*CRC32*/ + 4/*file_size*/ + 1/*reserved*/ + TFTP_COMMON_HEADER;
    unsigned short			 index;
    TFTP_rps_code_t          rps_code;
	short int				 client;
	general_handler_function_t the_handler;
	unsigned short 			 timeout_in_100_ms;


    CTC_STACK_CHECK_INIT_AND_CHECK_OLT_AND_ONU_VALIDITY_FUNC


    for(index = 0; index < TFTP_NUM_OF_RESEND_END_DOWNLOAD_REQUEST; index++)
    {
        sent_length = ETHERNET_MTU;
        total_length = 0;
        received_total_length  = 0;

        comparser_result = (short int)Compose_tftp_common_header
                                ( data_type, payload_length, onu_id, TRUE, 
                                  SEND_COMPARSER_BUFFER, &sent_length );
        CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

        total_length += sent_length;
        sent_length = max_length - total_length;

        comparser_result = (short int)Compose_end_download_request
                                ( file_size,
                                  SEND_COMPARSER_BUFFER, &sent_length );
        CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
            
        total_length += sent_length;
        sent_length = max_length - total_length;
        
		timeout_in_100_ms = TFTP_TIME_TO_RESEND_END_DOWNLOAD_REQUEST / 100;
        result = Send_receive_vendor_extension_message_with_time_out_request_id 
                        ( olt_id, onu_id, sent_oam_pdu_data, total_length, &timeout_in_100_ms, expected_opcode, expected_code,
                          received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL, CTC_STACK_COMMUNICATION_RESEND_LIMIT - 1);
        /* If sending the OAM failed by time out, and the resend time is less than the limit, continue to send again  */
		if ((result == CTC_STACK_COMMUNICATION_TIME_OUT) 
				&& (index < (TFTP_NUM_OF_RESEND_END_DOWNLOAD_REQUEST - 1)))
		{
			continue;
		}
		CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
         
        comparser_result = (short int)Parse_tftp_common_header
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, onu_id, data_type, TRUE, &received_left_length );

        CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    	received_total_length += received_left_length;
            
        comparser_result = Parse_end_download_response ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER,
                                                          &rps_code, &received_left_length );

        CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    	received_total_length += received_left_length;
        
        result = Convert_comparser_error_code_to_ctc_stack(comparser_result);
        if ( result != CTC_STACK_EXIT_OK )
        {
            PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT %d ONU %d failed to parse end download response. error code:%d\n", olt_id, onu_id, result)
            return result;
        }


        PONLOG_INFO_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT %d ONU %d rps code:%d\n", olt_id, onu_id, rps_code)
        
        if(rps_code == TFTP_RPS_CODE_VERIFICATION_SUCCESS)
        {
           return result;
        }
        else if(rps_code == TFTP_RPS_CODE_STILL_WRITING_SOFTWARE)
        {
			if (timeout_in_100_ms > 0)
			{
            	OSSRV_wait((unsigned short int)(timeout_in_100_ms*100));
			}
			result = rps_code;/*added by wangjiah@2016-09-22 to solve issue:33042*/
        }
        else
        {
            PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT %d ONU %d rps error code:%d\n", olt_id, onu_id, rps_code)

            for (client = 0; client < PON_MAX_CLIENT; client++)
            {
                PON_general_get_handler_function
                    ( &ctc_db_handlers,
                    CTC_STACK_HANDLER_UPDATE_ONU_FIRMWARE_ERROR,
                    client ,
                    &the_handler );
            
                if (the_handler != NULL)
                {
                    the_handler (olt_id, onu_id, rps_code, NULL);
                }
            }
            
            return CTC_STACK_EXIT_ERROR;
        }
    }

    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_activate_image"
short int PON_CTC_STACK_activate_image(
            const PON_olt_id_t                    olt_id, 
            const PON_onu_id_t                    onu_id)
{
    short int                result,comparser_result; 
    unsigned short           max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
    unsigned short           received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char            sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char            received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char            expected_opcode = OAM_VENDOR_EXT_UPDATE_FIRMWARE;
    unsigned char            expected_code   = TFTP_UPDATE_FIRMWARE_OPCODE_ACTIVATE_IMAGE_RESPONSE;
    TFTP_data_type_t         data_type       = TFTP_DATA_TYPE_ACTIVE_IMAGE;
    unsigned short           payload_length  = 2/*opcode*/ + 1/*flag*/ + TFTP_COMMON_HEADER;
    TFTP_activate_image_ack_t  ack;
    short int                  client;
    general_handler_function_t the_handler;


    CTC_STACK_CHECK_INIT_AND_CHECK_OLT_AND_ONU_VALIDITY_FUNC

    comparser_result = (short int)Compose_tftp_common_header
                                ( data_type, payload_length, onu_id, TRUE, 
                                  SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    total_length += sent_length;
    sent_length = max_length - total_length;

    comparser_result = (short int)Compose_activate_image_request
                            ( TFTP_ACTIVE_BACKUP_IMAGE, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
        
    total_length += sent_length;
    sent_length = max_length - total_length;

        
    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    comparser_result = (short int)Parse_tftp_common_header
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, onu_id, data_type, TRUE, &received_left_length );

    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    received_total_length += received_left_length;

        
    comparser_result = Parse_activate_image_response ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER,
                                                       &ack, &received_left_length );

    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    received_total_length += received_left_length;

    
    result = Convert_comparser_error_code_to_ctc_stack(comparser_result);
    if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT %d ONU %d failed to parse activate image response. error code:%d\n", olt_id, onu_id, result)
        return result;
    }
        
    if(ack == TFTP_ACTIVE_IMAGE_ACK_SUCCESS)
    {
        return result;
    }
    else
    {
        PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT %d ONU %d activate image error ack:%d\n", olt_id, onu_id, ack)

        for (client = 0; client < PON_MAX_CLIENT; client++)
        {
            PON_general_get_handler_function
                ( &ctc_db_handlers,
                CTC_STACK_HANDLER_UPDATE_ONU_FIRMWARE_ERROR,
                client ,
                &the_handler );
            
            if (the_handler != NULL)
            {
                the_handler (olt_id, onu_id, ack, NULL);
            }
        }
		result = ack;/*added by wangjiah@2016-09-22 to solve issue:33042*/
    }
        
    return result;

}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_commit_image"
short int PON_CTC_STACK_commit_image(
            const PON_olt_id_t                    olt_id, 
            const PON_onu_id_t                    onu_id)
{
    short int                result,comparser_result; 
    unsigned short           max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
    unsigned short           received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char            sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char            received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char            expected_opcode = OAM_VENDOR_EXT_UPDATE_FIRMWARE;
    unsigned char            expected_code   = TFTP_UPDATE_FIRMWARE_OPCODE_COMMIT_IMAGE_RESPONSE;
    TFTP_data_type_t         data_type       = TFTP_DATA_TYPE_COMMIT_IMAGE;
    unsigned short           payload_length  = 2/*opcode*/ + 1/*flag*/ + TFTP_COMMON_HEADER;
    TFTP_commit_image_ack_t  ack;
    short int                  client;
    general_handler_function_t the_handler;


    CTC_STACK_CHECK_INIT_AND_CHECK_OLT_AND_ONU_VALIDITY_FUNC

    comparser_result = (short int)Compose_tftp_common_header
                                ( data_type, payload_length, onu_id, TRUE, 
                                  SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    total_length += sent_length;
    sent_length = max_length - total_length;

    comparser_result = (short int)Compose_commit_image_request
                            ( TFTP_COMMIT_BACKUP_IMAGE, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
        
    total_length += sent_length;
    sent_length = max_length - total_length;

        
    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC


    comparser_result = (short int)Parse_tftp_common_header
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, onu_id, data_type, TRUE, &received_left_length );

    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    received_total_length += received_left_length;

        
    comparser_result = Parse_commit_image_response ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER,
                                                       &ack, &received_left_length );

    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    received_total_length += received_left_length;

    
    result = Convert_comparser_error_code_to_ctc_stack(comparser_result);
    if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT %d ONU %d failed to parse commit image response. error code:%d\n", olt_id, onu_id, result)
        return result;
    }
        
    if(ack == TFTP_COMMIT_IMAGE_ACK_SUCCESS)
    {
        return result;
    }
    else
    {
        PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT %d ONU %d commit image error ack:%d\n", olt_id, onu_id, ack)

        for (client = 0; client < PON_MAX_CLIENT; client++)
        {
            PON_general_get_handler_function
                ( &ctc_db_handlers,
                CTC_STACK_HANDLER_UPDATE_ONU_FIRMWARE_ERROR,
                client ,
                &the_handler );
            
            if (the_handler != NULL)
            {
                the_handler (olt_id, onu_id, ack, NULL);
            }
        }
		result = ack;/*added by wangjiah@2016-09-22 to solve issue:33042*/
    }
        
    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Set_onu_state_in_update_firmware_db"

static short int Set_onu_state_in_update_firmware_db
                ( const PON_olt_id_t                       olt_id, 
                  const PON_onu_id_t					   onu_id,
				  const ctc_stack_onu_downloading_state_t  state )
{
    short int  result;

    if( (state == CTC_STACK_ONU_STATE_DURING_DOWNLOADING) &&
		(update_firmware_db[olt_id].onu_state[onu_id] == CTC_STACK_ONU_STATE_DURING_DOWNLOADING))
    {/* ONU is during downloading */

		PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, olt_id, onu_id,
						"OLT %d ONU %d is during downloading a new Firmware\n", olt_id, onu_id );

        return CTC_STACK_QUERY_FAILED;
    }

    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC(PONLOG_FUNC_GENERAL, &(update_firmware_db[olt_id].semaphore), update_firmware_semaphore,result );
    if (result != EXIT_OK) return (CTC_STACK_QUERY_FAILED);

	update_firmware_db[olt_id].onu_state[onu_id] = state;
   
    /* END PROTECTION  */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC(PONLOG_FUNC_GENERAL, &(update_firmware_db[olt_id].semaphore), update_firmware_semaphore,result );
    if (result != EXIT_OK) return (CTC_STACK_QUERY_FAILED);

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Onu_request_hook_func"

static void Onu_request_hook_func(void)
{
	short int						result=CTC_STACK_EXIT_OK;
	short int						comparser_result;
	PON_olt_id_t					olt_id;
    PON_onu_id_t					onu_id;
	unsigned char					file_name[ONU_RESPONSE_SIZE], error_message[MAX_PARAMETER_LENGTH];
	unsigned char					sent_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned short			        max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
    unsigned short			        sent_length=ETHERNET_MTU;
	bool							confirmation;
	unsigned short					error_code;
	short int						client;
	general_handler_function_t		the_handler;
    CTC_STACK_binary_t              onu_firmware;
    TFTP_data_type_t                data_type = TFTP_DATA_TYPE_DATA;
    unsigned short                  payload_length_ack   = 2/*opcode*/ + 2/*block_number*/ + TFTP_COMMON_HEADER;
    unsigned short                  payload_length_error = 2/*opcode*/ + 2/*error_code*/ + strlen(error_message)/*msg*/ + 1/*reserved*/ + TFTP_COMMON_HEADER;
  

	SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC( PONLOG_FUNC_GENERAL, 
											 &global_onu_request_event_data.semaphore, 
											 global_onu_request_event_data_semaphore, result)
	if (result != EXIT_OK) return;

	olt_id		= global_onu_request_event_data.olt_id;
	onu_id		= global_onu_request_event_data.onu_id;
	strcpy(file_name, global_onu_request_event_data.file_name);


	global_onu_request_event_data.olt_id = PON_OLT_ID_NOT_AVAILABLE;
		

	RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC( PONLOG_FUNC_GENERAL, 
											  &global_onu_request_event_data.semaphore,
											  global_onu_request_event_data.semaphore, result)
	if (result != EXIT_OK) return;


	
	for (client = 0; client < PON_MAX_CLIENT; client++)
	{
		PON_general_get_handler_function
						   ( &ctc_db_handlers,
							 CTC_STACK_HANDLER_ONU_FIRMWARE_REQUEST,
							 client ,
							 &the_handler );

		if (the_handler != NULL)
		{
			the_handler (	olt_id, onu_id, file_name, &confirmation, &onu_firmware, &error_code, error_message);
		}
		else
		{
			PONLOG_ERROR_2( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
						"can not answer to OLT %d ONU %d for its update firmware requsts since there is no assign handler\n", olt_id, onu_id );

			return;
		}
	}



	if(confirmation == TRUE)
	{
	
		if( Set_onu_state_in_update_firmware_db(olt_id, onu_id, CTC_STACK_ONU_STATE_DURING_DOWNLOADING) != CTC_STACK_EXIT_OK)
		{
			PONLOG_ERROR_2( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
							"OLT %d ONU %d is during download already\n", olt_id, onu_id );

			return;
		}
        
        comparser_result = (short int)Compose_tftp_common_header
                                ( data_type, payload_length_ack, onu_id, TRUE, 
                                  SEND_COMPARSER_BUFFER, &sent_length );

        total_length += sent_length;
        sent_length = max_length - total_length;
            
		/* Send ACK to the ONU */
		comparser_result = (short int)Compose_file_transfer_ack
								( 0 /*block_number*/,
								  SEND_COMPARSER_BUFFER, &sent_length );
        
        total_length += sent_length;
        sent_length = max_length - total_length;

		if ( result != CTC_STACK_EXIT_OK )
		{ 
			PONLOG_ERROR_3( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
							"failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result);
			return;
		}
		result = Send_receive_vendor_extension_message_with_timeout 
						( olt_id, onu_id, sent_oam_pdu_data, sent_length, 0/*without waiting*/, 
						  0/*expected_opcode- do not waiting for response*/, 0/*expected_code- do not waiting for response*/,
						  NULL/*received_oam_pdu_data*/, NULL/*&received_length*/, ETHERNET_MTU,NULL);

    

		Send_file_and_verify(olt_id, onu_id, &onu_firmware, FALSE, NULL);
	}
	else/*confirmation = FALSE*/
	{
		unsigned short		tftp_error_code;
		PONLOG_INFO_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
							"OLT %d ONU %d is not confirmed to update a new Firmware. user error code %d error message:%s\n", olt_id, onu_id, error_code, error_message);


		tftp_error_code = Convert_ctc_stack_error_codes_to_tftp(error_code);

        comparser_result = (short int)Compose_tftp_common_header
                            ( data_type, payload_length_error, onu_id, TRUE,
                              SEND_COMPARSER_BUFFER, &sent_length );

        total_length += sent_length;
        sent_length = max_length - total_length; 
                               
		/* Send NACK to the ONU */
		comparser_result = (short int)Compose_error_message
								( tftp_error_code, error_message,
								  SEND_COMPARSER_BUFFER, &sent_length );

        total_length += sent_length;
        sent_length = max_length - total_length;

		if ( result != CTC_STACK_EXIT_OK )
		{ 
			PONLOG_ERROR_3( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
							"failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result);
			return;
		}
		result = Send_receive_vendor_extension_message_with_timeout 
						( olt_id, onu_id, sent_oam_pdu_data, sent_length, 0/*without waiting*/, 
						  0/*expected_opcode- do not waiting for response*/, 0/*expected_code- do not waiting for response*/,
						  NULL/*received_oam_pdu_data*/, NULL/*&received_length*/, ETHERNET_MTU,NULL);
		
	}


	return ;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Onu_request_handler_function"

static void Onu_request_handler_function
                ( const PON_olt_id_t     olt_id, 
                  const PON_onu_id_t	 onu_id,
				  const unsigned short   length,    
                  unsigned char         *content,
                  unsigned char          received_code)
{
    short int     result;

    switch(received_code) 
    {
    case OAM_VENDOR_EXT_UPDATE_FIRMWARE:
         result = Onu_request_update_firmware(olt_id, onu_id, length, content);
    	break;
    case PON_OAM_CODE_EVENT_NOTIFICATION:
         result = ctc_event_notification(olt_id, onu_id, length, content);
    	break;
    default:
			PONLOG_ERROR_3( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
							"failed in OLT %d ONU %d. unknown received code: %d\n", olt_id, onu_id, received_code);
    }
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Onu_request_update_firmware"

static short int Onu_request_update_firmware
                ( const PON_olt_id_t     olt_id, 
                  const PON_onu_id_t	 onu_id,
				  const unsigned short   length,    
                  unsigned char         *content)
{
	short int            comparser_result, result;
	unsigned short		 received_total_length=0, received_left_length=0;
	unsigned char		 file_name[ONU_RESPONSE_SIZE];
	PON_clock_t			 wait_start_time= PON_CLOCK_FUNCTION(), wait_current_time=0;
	double				 duration =0,loop_counter=0;
	short int			 olt_id_already_had_request = PON_MIN_OLT_ID;
    TFTP_data_type_t     data_type = TFTP_DATA_TYPE_DATA;


	comparser_result = Parse_tftp_common_header
				( content+received_total_length, (unsigned short)(length - received_total_length),
                  onu_id, data_type, TRUE, &received_left_length );

    received_total_length += received_left_length;

	result = Convert_comparser_error_code_to_ctc_stack(comparser_result);
    if ( result != CTC_STACK_EXIT_OK )
	{ 
		PONLOG_ERROR_3( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
						"failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result);
		return result;
	}


	comparser_result = Parse_file_read_request
				( content+received_total_length, (unsigned short)(length - received_total_length),
				  file_name, &received_left_length );

    received_total_length += received_left_length;

	result = Convert_comparser_error_code_to_ctc_stack(comparser_result);
    if ( result != CTC_STACK_EXIT_OK )
	{ 
		PONLOG_ERROR_3( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
						"failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result);
		return result;
	}


	PONLOG_INFO_2( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
						"OLT %d ONU %d received update request\n", olt_id, onu_id);


	if(update_firmware_db[olt_id].onu_state[onu_id] == CTC_STACK_ONU_STATE_DURING_DOWNLOADING)
	{
		PONLOG_ERROR_2( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
						"OLT %d ONU %d is during download already\n", olt_id, onu_id );
	}
	else
	{
		while ((olt_id_already_had_request != PON_OLT_ID_NOT_AVAILABLE) &&
		   (duration < WAIT_FOR_ONU_REQUEST_EVENT_HANDLING_TIMEOUT) && 
		   (loop_counter < WAIT_FOR_ONU_REQUEST_EVENT_HANDLING_LOOP_TIMEOUT))
		{

			SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC( PONLOG_FUNC_GENERAL, 
													 &global_onu_request_event_data.semaphore, 
													 global_onu_request_event_data_semaphore, result)
			if (result != EXIT_OK) return result; 


			olt_id_already_had_request = global_onu_request_event_data.olt_id;
			if (global_onu_request_event_data.olt_id == PON_OLT_ID_NOT_AVAILABLE) 
			{
				global_onu_request_event_data.olt_id	= olt_id;
				global_onu_request_event_data.onu_id	= onu_id;
				strcpy(global_onu_request_event_data.file_name, file_name);
			}

			RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC( PONLOG_FUNC_GENERAL, 
													  &global_onu_request_event_data.semaphore,
													  global_onu_request_event_data.semaphore, result)
			if (result != EXIT_OK) return result;



			OSSRV_wait(WAIT_FOR_ONU_REQUEST_EVENT_HANDLING_CHECK);
		
			wait_current_time = PON_CLOCK_FUNCTION();
			loop_counter++;
			duration = (double)(wait_current_time - wait_start_time) / PON_HOST_CLOCKS_PER_SEC;


		}

		/* OLT reset handling function - runs from its own thread so high layers can have broader handling options */
		if (olt_id_already_had_request == PON_OLT_ID_NOT_AVAILABLE)
			OSSRV_create_thread ((void *)Onu_request_hook_func);
		else
		{
			PONLOG_ERROR_2( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
						"OLT %d ONU %d Error, timeout waiting for previous ONU request event handler to g_free global database\n", olt_id, onu_id );

			return result;
		}
	}

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "ctc_event_notification"

short int ctc_event_notification
                ( const PON_olt_id_t     olt_id, 
                  const PON_onu_id_t	 onu_id,
				  const unsigned short   received_length,
                  unsigned char         *content)
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0;
	unsigned short							total_length=0, received_total_length=0;
    CTC_STACK_event_header_t                event_header;
    CTC_STACK_event_value_t                 event_value;
    unsigned char                           received_alarm_info_size, event_value_size = 2/*object_type*/ +4/*object_index*/ +2/*alarm_id*/+ 2/*time_stamp*/ +1/*alarm_state*/;      
    bool                                    found_alarm_id = FALSE;
    short int					            client, index;
	general_handler_function_t	            the_handler;
    CTC_management_object_leaf_t            expected_management_object_leaf;                                                               
                                                                   
    while(received_total_length < received_length)
    {
        memset(&event_header, 0, sizeof(CTC_STACK_event_header_t));

        comparser_result = (short int)CTC_PARSE_event_header
                                        ( content+received_total_length, RECEIVED_LENGTH_BUFFER,
                                          &event_header, &received_left_length );

        received_total_length += received_left_length;

        if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
            break;

        CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


        if(!Compare_oui_as_number(ctc_stack_general_information.ctc_stack_oui, event_header.oui))
        {
            PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id,"OLT:%d ONU:%d has no match to oui: 0x%x\n", 
                            olt_id, onu_id, ctc_stack_general_information.ctc_stack_oui );
            return result;
        }
        
        
        memset(&event_value, 0, sizeof(CTC_STACK_event_value_t));

        received_alarm_info_size = (event_header.event_length) - sizeof(CTC_STACK_event_header_t) - event_value_size; 


        comparser_result = (short int)CTC_PARSE_event_value
                                        ( content+received_total_length, RECEIVED_LENGTH_BUFFER,
                                          received_alarm_info_size, &event_value, &received_left_length );

        received_total_length += received_left_length;


        CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
         

        /*check if alarm_id exist*/
        index = 0;
        while (strlen(alarm_str_array[index].alarm_str) > 0)
        {
            if (alarm_str_array[index].alarm_id == event_value.alarm_id)
            {
                found_alarm_id = TRUE;
                break;
            }
            index++;
        }

        if(found_alarm_id)
        {
            PONLOG_INFO_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
                           "event notification in OLT %d ONU %d. alarm id: 0x%x alarm str: %s\n",
                           olt_id, onu_id, event_value.alarm_id, (alarm_str_array[index].alarm_str));

            /*check alarm info size*/
            if(alarm_str_array[index].alarm_info_size != received_alarm_info_size)
            {
                PONLOG_ERROR_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
                                "OLT %d ONU %d. received Wrong event length. received: %d, expected: %d\n",
                                olt_id, onu_id, received_alarm_info_size, (alarm_str_array[index].alarm_info_size));
                return result;
            }
        }
        else
        {
            PONLOG_ERROR_3( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
                            "event notification in OLT %d ONU %d. unknown alarm id: 0x%x\n",
                            olt_id, onu_id, event_value.alarm_id);
            return result;
        }
        

        CHECK_ALARM_ID_AND_EXTRACT_MANAGEMENT_OBJECT_LEAF(PONLOG_FUNC_GENERAL, event_value.alarm_id, expected_management_object_leaf);
        if(event_value.management_object.leaf != expected_management_object_leaf)
        {
            PONLOG_ERROR_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
                            "failed in OLT %d ONU %d. The leaf don't match the alarm id. expected leaf: %d, received leaf: %d\n",
                            olt_id, onu_id, expected_management_object_leaf, (event_value.management_object.leaf));
            return result;
        }


        /*inform event*/  
        for (client = 0; client < PON_MAX_CLIENT; client++)
        {
            PON_general_get_handler_function
                               ( &ctc_db_handlers,
                                 CTC_STACK_HANDLER_EVENT_NOTIFICATION,
                                 client ,
                                 &the_handler );
            
            if (the_handler != NULL)
            {
                the_handler (olt_id, onu_id, event_value);
            }
        }
    }

    return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Get_chipset_id"

static short int  Get_chipset_id ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id,
		   CTC_STACK_chipset_id_t  *chipset_id )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_CHIPSET_ID;
	unsigned char							ctc_ret_val;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							received_num_of_entries=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)CTC_PARSE_chipset_id
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          chipset_id, &ctc_ret_val, &received_left_length  );

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;


	return result;
        
}
#undef PONLOG_FUNCTION_NAME
   

#define PONLOG_FUNCTION_NAME  "PON_CTC_get_pmc_onu_version_handler"

static short int PON_CTC_STACK_get_pmc_onu_version_handler
                                                   ( const PON_olt_id_t			  olt_id, 
													 const PON_onu_id_t			  onu_id,
													       PAS_pmc_onu_version_t *pmc_onu_version )
{
    CTC_STACK_chipset_id_t  chipset_id;
    short int               result;

    pmc_onu_version->pmc_onu = FALSE;

    result = Get_chipset_id ( 
                                olt_id, 
                             	onu_id,
		                       &chipset_id );
    
    switch(result) 
    {
    case CTC_STACK_EXIT_OK:
        /* Check if the vendor is PMC */
        if (memcmp(chipset_id.vendor_id,PMC_SIERRA_VENDOR_ID,CTC_VENDOR_ID_LENGTH) == 0) 
        {
            /* Its PMC so update the return parameter */
            pmc_onu_version->hardware_version.major = chipset_id.chip_model;
            pmc_onu_version->hardware_version.minor = chipset_id.revision;
            pmc_onu_version->pmc_onu = TRUE;
            result = PAS_EXIT_OK;
        }
        break;
    case CTC_STACK_ONU_NOT_AVAILABLE:
         /* ONU is not CTC avalible so return not pmc */
        result = PAS_ONU_NOT_AVAILABLE;
    	break;
    default:
        /* Error */
        PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, olt_id, onu_id,
			"OLT %d ONU %d Error, Can't get chipset id\n", olt_id, onu_id );

        result = PAS_EXIT_ERROR;
    }

    return (result);
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_onu_ponchip_vendor"

PON_STATUS PON_CTC_STACK_get_onu_ponchip_vendor( const PON_olt_id_t			  olt_id, 
            													 const PON_onu_id_t			  onu_id,
            													 unsigned short              *vendor_type,
            													 unsigned short              *chip_type,
            													 unsigned short              *rev_type)
{
    CTC_STACK_chipset_id_t  chipset_id;
    short int               result;

    result = Get_chipset_id ( 
                                olt_id, 
                             	onu_id,
		                       &chipset_id );

    if ( CTC_STACK_EXIT_OK == result )
    {
        *chip_type = chipset_id.chip_model;
        *rev_type  = chipset_id.revision;
        
        if (memcmp(chipset_id.vendor_id,PMC_SIERRA_VENDOR_ID,CTC_VENDOR_ID_LENGTH) == 0) 
        {
            /* Its PMC so update the return parameter */
            *vendor_type = OnuVendorTypesPmc;
        }
        else
        {
            unsigned char vendor_oui[3];    
        
            if ( 0 == (result = PonStdOamGetPeerStatus(olt_id, onu_id, Dot3OAMPEERVENDOROUI, NULL, &vendor_oui)) )
            {
                *vendor_type = GetOnuChipVenderTypeByOUI(vendor_oui);
            }
        }
    }

    return result;
}
#undef PONLOG_FUNCTION_NAME


#if 1
#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_onu_pon_vendor"

PON_STATUS PON_CTC_STACK_get_onu_pon_vendor (  
           const PON_olt_id_t			   olt_id, 
           const PON_onu_id_t			   onu_id,
		   OnuVendorTypes  *pon_vendor_types )
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

	CTC_STACK_CHECK_INIT_FUNC

    if ( CTC_DISCOVERY_STATE_COMPLETE == discovery_db.state[olt_id][onu_id] )
    {
#ifdef PON_CHIPTYPE_KNOWN
        *pon_vendor_types = discovery_db.pon_chip_vendor[olt_id][onu_id];
#else
        *pon_vendor_types = OnuVendorTypesUnknown;
#endif
    }
    else
    {
        return CTC_STACK_ONU_NOT_AVAILABLE;
    }

    return 0;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_onu_frame_size"

PON_STATUS PON_CTC_STACK_get_onu_frame_size (  
           const PON_olt_id_t			   olt_id, 
           const PON_onu_id_t			   onu_id,
		   unsigned short  *onu_frame_size )
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

	CTC_STACK_CHECK_INIT_FUNC

    if ( CTC_DISCOVERY_STATE_COMPLETE == discovery_db.state[olt_id][onu_id] )
    {
        *onu_frame_size = discovery_db.maximum_data_frame_size[olt_id][onu_id];
    }
    else
    {
        return CTC_STACK_ONU_NOT_AVAILABLE;
    }

    return 0;
}
#undef PONLOG_FUNCTION_NAME
#endif


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_discovery_state"

PON_STATUS PON_CTC_STACK_get_discovery_state(           
					const PON_olt_id_t			olt_id, 
					const PON_onu_id_t			onu_id,
					CTC_STACK_discovery_state_t	*state)
{
	(*state) = discovery_db.state[olt_id][onu_id];

	return (CTC_STACK_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "Search_largest_common_oui_version"

static bool Search_largest_common_oui_version(
                    const unsigned char 	               receive_number_of_records,
					const CTC_STACK_oui_version_record_t  *receive_oui_version_record_array,
                    unsigned char 	                      *largest_common_version)
{
    unsigned char i,j;
    bool support = FALSE;
    
    (*largest_common_version) = 0;
    
    /* loop over the ONU records */ 
    for(i = 1; i <= receive_number_of_records; i++)
    {
        if(Compare_oui_as_number(ctc_stack_general_information.ctc_stack_oui,(receive_oui_version_record_array+i)->oui))
        {
            /* loop over the OLT records */
            for(j = 0; j < discovery_db.list_info.number_of_oui_records; j++)
            {
                if((receive_oui_version_record_array+i)->version == discovery_db.list_info.records_list[j].version)
                {
                    /*save common version*/
                    if((*largest_common_version) < (receive_oui_version_record_array+i)->version)
                    {
                        (*largest_common_version) = (receive_oui_version_record_array+i)->version;
                    }
                    support = TRUE;
                    break;
                }
            }
        }
    }
    
    return support;
}
#undef PONLOG_FUNCTION_NAME 


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_auth_request"

short int PON_CTC_STACK_auth_request ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   CTC_STACK_auth_response_t         *auth_response)
{
    short int                 result,comparser_result; 
    unsigned short			  sent_length=ETHERNET_MTU, received_length;
    unsigned char			  sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char			  received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char			  expected_opcode = OAM_VENDOR_EXT_ONU_AUTHENTICATION;
    unsigned char			  expected_code   = AUTH_VENDOR_EXT_RESPONSE;


    CTC_STACK_CHECK_INIT_AND_CHECK_OLT_AND_ONU_VALIDITY_FUNC

    PONLOG_INFO_2( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d send auth request\n",olt_id, onu_id );

    comparser_result = (short int)Compose_auth_request
                            ( SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_AUTHENTICATION_FLOW, comparser_result)


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, sent_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

   
    comparser_result = Parse_auth_response ( received_oam_pdu_data, received_length, auth_response );
    
    result = Convert_comparser_error_code_to_ctc_stack(comparser_result);
        
    return result;
}
#undef PONLOG_FUNCTION_NAME   

  
#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_auth_success"

short int PON_CTC_STACK_auth_success ( 
                                         const PON_olt_id_t                 olt_id, 
                                         const PON_onu_id_t                 onu_id)
{
    short int                result,comparser_result;
    unsigned short           sent_length=ETHERNET_MTU;
    unsigned char            sent_oam_pdu_data[ETHERNET_MTU]= {0};

    CTC_STACK_CHECK_INIT_AND_CHECK_OLT_AND_ONU_VALIDITY_FUNC

    PONLOG_INFO_2( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d send auth success\n",olt_id, onu_id );

    comparser_result = (short int)Compose_auth_success
                            ( sent_oam_pdu_data+OAM_OUI_SIZE, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_AUTHENTICATION_FLOW, comparser_result)


	result = Send_receive_vendor_extension_message_with_timeout 
						( olt_id, onu_id, sent_oam_pdu_data, sent_length, 0/*without waiting*/, 
						  0/*expected_opcode- do not waiting for response*/, 0/*expected_code- do not waiting for response*/,
						  NULL/*received_oam_pdu_data*/, NULL/*&received_length*/, ETHERNET_MTU,NULL);

     
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
    return result;
}
#undef PONLOG_FUNCTION_NAME 


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_auth_failure"

short int PON_CTC_STACK_auth_failure ( 
                                         const PON_olt_id_t                  olt_id, 
                                         const PON_onu_id_t                  onu_id,
                                         const CTC_STACK_auth_failure_type_t failure_type )
{
    short int                result,comparser_result;
    unsigned short           sent_length=ETHERNET_MTU;
    unsigned char            sent_oam_pdu_data[ETHERNET_MTU]= {0};

    CTC_STACK_CHECK_INIT_AND_CHECK_OLT_AND_ONU_VALIDITY_FUNC

    PONLOG_INFO_3( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT:%d ONU:%d send auth failure of type: %d\n",olt_id, onu_id, failure_type);

    comparser_result = (short int)Compose_auth_failure
                            ( failure_type, sent_oam_pdu_data+OAM_OUI_SIZE, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_AUTHENTICATION_FLOW, comparser_result)


	result = Send_receive_vendor_extension_message_with_timeout 
						( olt_id, onu_id, sent_oam_pdu_data, sent_length, 0/*without waiting*/, 
						  0/*expected_opcode- do not waiting for response*/, 0/*expected_code- do not waiting for response*/,
						  NULL/*received_oam_pdu_data*/, NULL/*&received_length*/, ETHERNET_MTU,NULL);

     
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
       
    return result;
}
#undef PONLOG_FUNCTION_NAME 


#ifdef CTC_AUTH_SUPPORT
#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_start_loid_authentication"

short int PON_CTC_STACK_start_loid_authentication ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   bool                              *auth_success)
{
    short int                      result;
    CTC_STACK_auth_response_t      auth_response;
    unsigned short                 index;
    CTC_STACK_auth_failure_type_t  failure_type;
	bool						   exist_assign_handler = FALSE;
   	short int					   client;
	general_handler_function_t	   the_handler;

    failure_type    = CTC_AUTH_FAILURE_ONU_ID_NOT_EXIST;
    (*auth_success) = FALSE;

    result = PON_CTC_STACK_auth_request(olt_id, onu_id, &auth_response);
    if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_3(PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT %d ONU %d auth request failed. error code:%d\n", olt_id, onu_id, result);
        return result;
    }


    for (client = 0; client < PON_MAX_CLIENT; client++)
    {
        PON_general_get_handler_function
                                    ( &ctc_db_handlers,
                                      CTC_STACK_HANDLER_AUTH_RESPONSE_LOID,
                                      client ,
                                      &the_handler );
        if (the_handler != NULL)
        {
            exist_assign_handler = TRUE;
			the_handler ( olt_id, onu_id, auth_response.loid_data, auth_success, &failure_type );
        }
    }

    /* handler function isn't assigned. search in authentication_db */
    if (!exist_assign_handler)
    {
        for(index = 0; index < PON_ONU_ID_PER_OLT_ARRAY_SIZE; index++)
        {
            if(memcmp(auth_response.loid_data.onu_id, authentication_db.olt[olt_id].loid[index].onu_id, CTC_AUTH_ONU_ID_SIZE) == 0)
            {
                if (authentication_db.olt[olt_id].onu_llid[index] != PON_NOT_USED_LLID)
                {
                    failure_type = CTC_AUTH_FAILURE_ONU_ID_USED;
                }    
                else if (authentication_db.olt[olt_id].auth_mode == CTC_AUTH_MODE_LOID_NOT_CHK_PWD
                         || authentication_db.olt[olt_id].auth_mode == CTC_AUTH_MODE_HYBRID_NOT_CHK_PWD)
                {
                    (*auth_success) = TRUE;
                }
                else if(memcmp(auth_response.loid_data.password, authentication_db.olt[olt_id].loid[index].password, CTC_AUTH_PASSWORD_SIZE) == 0)
                {
                    (*auth_success) = TRUE;
                }
                else
                {
                    failure_type = CTC_AUTH_FAILURE_WRONG_PASSWORD;
                }
                
                break;
            }
        }
    }

    /* sending auth success/failure */
    if((*auth_success) == TRUE)
    {
       result = PON_CTC_STACK_auth_success(olt_id, onu_id);
       if ( result != CTC_STACK_EXIT_OK )
       {
           PONLOG_ERROR_3(PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT %d ONU %d auth success failed. error code:%d\n", olt_id, onu_id, result);
           return result;
       }
       authentication_db.olt[olt_id].onu_llid[index] = onu_id;
	   failure_type = CTC_AUTH_SUCCESS;	/* added by xieshl 20100610, 这种情况后面仍会上报失败 */
    }
    else
    {
      result = PON_CTC_STACK_auth_failure(olt_id, onu_id, failure_type);
       if ( result != CTC_STACK_EXIT_OK )
       {
           PONLOG_ERROR_3(PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, onu_id, "OLT %d ONU %d auth failure failed. error code:%d\n", olt_id, onu_id, result);
           return result;
       }
    }


    for (client = 0; client < PON_MAX_CLIENT; client++)
    {
        PON_general_get_handler_function
                                    ( &ctc_db_handlers,
                                      CTC_STACK_HANDLER_AUTHORIZED_LOID,
                                      client ,
                                      &the_handler );
        if (the_handler != NULL)
        {
            exist_assign_handler = TRUE;
			the_handler ( olt_id, onu_id, auth_response.loid_data, (*auth_success), failure_type );
        }
    }
  

    return result;
}
#undef PONLOG_FUNCTION_NAME 


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auth_loid_data_table"

PON_STATUS PON_CTC_STACK_get_auth_loid_data_table(           
					const PON_olt_id_t			           olt_id, 
					CTC_STACK_auth_loid_data_t            *loid_data_table,
                    unsigned char                         *num_of_entries)
{
    unsigned short index;
    
    if((*num_of_entries) < PON_ONU_ID_PER_OLT_ARRAY_SIZE)
    {
        PONLOG_ERROR_1( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, PONLOG_ONU_IRRELEVANT, "not enough memory, num_of_entries needed: %d\n", PON_ONU_ID_PER_OLT_ARRAY_SIZE);
        return CTC_STACK_MEMORY_ERROR;
    }
    
    (*num_of_entries) = PON_ONU_ID_PER_OLT_ARRAY_SIZE;
    
    for(index = 0; index < PON_ONU_ID_PER_OLT_ARRAY_SIZE; index++)
    {
		memset(&(loid_data_table[index]), 0, sizeof(CTC_STACK_auth_loid_data_t));
        memcpy((loid_data_table[index].onu_id), (authentication_db.olt[olt_id].loid[index].onu_id), CTC_AUTH_ONU_ID_SIZE);
        memcpy((loid_data_table[index].password), (authentication_db.olt[olt_id].loid[index].password), CTC_AUTH_PASSWORD_SIZE);
    }

    return (CTC_STACK_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME

/* added by xieshl 20100610, 临时提供接口，便于MIB开发 */
authentication_olt_database_t * PON_CTC_STACK_get_auth_loid_database( const PON_olt_id_t olt_id )
{
	return &authentication_db.olt[olt_id];
}

PON_STATUS PON_CTC_STACK_add_auth_loid_data_by_index( const PON_olt_id_t olt_id, const unsigned char loid_id,
		const CTC_STACK_auth_loid_data_t loid_data)
{
    unsigned short              index;
    char                        index_not_used = -1;
    CTC_STACK_auth_loid_data_t  auth_loid_data_checked; 

    if( loid_id >= PON_ONU_ID_PER_OLT_ARRAY_SIZE )
		return CTC_STACK_PARAMETER_ERROR;
	
    /*add completing zeros*/
    if(strlen(loid_data.onu_id) <= CTC_AUTH_ONU_ID_SIZE)
    {
       memset(auth_loid_data_checked.onu_id, '0', CTC_AUTH_ONU_ID_SIZE);
       memcpy((auth_loid_data_checked.onu_id + CTC_AUTH_ONU_ID_SIZE - strlen(loid_data.onu_id)), loid_data.onu_id, strlen(loid_data.onu_id)+1); 
    }
    else
    {
        return CTC_STACK_PARAMETER_ERROR;
    }
    /*add completing zeros*/
    if(strlen(loid_data.password) <= CTC_AUTH_PASSWORD_SIZE)
    {
       memset(auth_loid_data_checked.password, '0', CTC_AUTH_PASSWORD_SIZE);
       memcpy((auth_loid_data_checked.password + CTC_AUTH_PASSWORD_SIZE - strlen(loid_data.password)), loid_data.password, strlen(loid_data.password)+1); 
    }
    else
    {
        return CTC_STACK_PARAMETER_ERROR;
    }

     /*check the index is not used*/ 
   if(authentication_db.olt[olt_id].onu_llid[loid_id] != PON_NOT_USED_LLID )
    {
        return CTC_STACK_ALREADY_EXIST;
    }
    for(index = 0; index < PON_ONU_ID_PER_OLT_ARRAY_SIZE; index++)
    {
        if(strcmp(auth_loid_data_checked.onu_id, authentication_db.olt[olt_id].loid[index].onu_id) == 0)		/*onu_id already exist*/
        {
            return CTC_STACK_ALREADY_EXIST;
        }
    }
   
        strcpy((authentication_db.olt[olt_id].loid[loid_id].onu_id), (auth_loid_data_checked.onu_id));
        strcpy((authentication_db.olt[olt_id].loid[loid_id].password), (auth_loid_data_checked.password));
        authentication_db.olt[olt_id].onu_llid[loid_id] = PON_NOT_USED_LLID;

	return (CTC_STACK_EXIT_OK);
}

PON_STATUS PON_CTC_STACK_remove_auth_loid_data_by_index( const PON_olt_id_t olt_id, const unsigned char loid_id )
{
    if( loid_id >= PON_ONU_ID_PER_OLT_ARRAY_SIZE )
		return CTC_STACK_PARAMETER_ERROR;

    if(authentication_db.olt[olt_id].onu_llid[loid_id] != PON_NOT_USED_LLID )
    {
        return CTC_STACK_QUERY_FAILED;
    }
  
    strcpy((authentication_db.olt[olt_id].loid[loid_id].onu_id), CTC_AUTH_LOID_DATA_NOT_USED);
    strcpy((authentication_db.olt[olt_id].loid[loid_id].password), CTC_AUTH_LOID_DATA_NOT_USED);
    authentication_db.olt[olt_id].onu_llid[loid_id] = PON_NOT_USED_LLID;

    return (CTC_STACK_EXIT_OK);
}
/* end 20100610 */


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_add_auth_loid_data"

PON_STATUS PON_CTC_STACK_add_auth_loid_data(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_loid_data_t     loid_data)

{
    unsigned short              index;
    int                         index_not_used = -1;
    CTC_STACK_auth_loid_data_t  auth_loid_data_checked; 
	

	memset(&auth_loid_data_checked, 0, sizeof(auth_loid_data_checked));
 	/*add completing zeros*/
    if(strlen(loid_data.onu_id) <= CTC_AUTH_ONU_ID_SIZE)
    {
       memcpy((auth_loid_data_checked.onu_id + CTC_AUTH_ONU_ID_SIZE - strlen(loid_data.onu_id)), loid_data.onu_id, strlen(loid_data.onu_id)+1); 
    }
    else
    {
        return CTC_STACK_PARAMETER_ERROR;
    }
    /*add completing zeros*/
    if(strlen(loid_data.password) <= CTC_AUTH_PASSWORD_SIZE)
    {
       memcpy((auth_loid_data_checked.password + CTC_AUTH_PASSWORD_SIZE - strlen(loid_data.password)), loid_data.password, strlen(loid_data.password)+1); 
    }
    else
    {
        return CTC_STACK_PARAMETER_ERROR;
    }

    for(index = 0; index < PON_ONU_ID_PER_OLT_ARRAY_SIZE; index++)
    {
        if(memcmp(auth_loid_data_checked.onu_id, authentication_db.olt[olt_id].loid[index].onu_id, CTC_AUTH_ONU_ID_SIZE) == 0)
        {
            /*onu_id already exist*/
            PONLOG_ERROR_0( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, PONLOG_ONU_IRRELEVANT, "auth onu_id data already exist in the loid table\n");
            return CTC_STACK_ALREADY_EXIST;
        }
        
        if(memcmp(CTC_AUTH_LOID_DATA_NOT_USED, authentication_db.olt[olt_id].loid[index].onu_id, sizeof(CTC_AUTH_LOID_DATA_NOT_USED)) == 0)
        {
            /*save index_not_used*/
            index_not_used = index;
            break;
        }
    }
    
    /*check if there is index not used*/ 
    if(index_not_used >= 0)
    {
        memset(&(authentication_db.olt[olt_id].loid[index_not_used]), 0 ,sizeof(CTC_STACK_auth_loid_data_t));
		memcpy((authentication_db.olt[olt_id].loid[index_not_used].onu_id), (auth_loid_data_checked.onu_id), CTC_AUTH_ONU_ID_SIZE);
        memcpy((authentication_db.olt[olt_id].loid[index_not_used].password), (auth_loid_data_checked.password), CTC_AUTH_PASSWORD_SIZE);
        authentication_db.olt[olt_id].onu_llid[index_not_used] = PON_NOT_USED_LLID;
    }
    else
    {
        PONLOG_ERROR_0( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, PONLOG_ONU_IRRELEVANT, "auth loid table is full\n");
        return CTC_STACK_TABLE_FULL;
    }

	return (CTC_STACK_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_remove_auth_loid_data"

PON_STATUS PON_CTC_STACK_remove_auth_loid_data(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_loid_data_t     loid_data)
{
    unsigned short index;
    bool           remove_data = FALSE;
    CTC_STACK_auth_loid_data_t  auth_loid_data_checked; 

    /*add completing zeros*/
    if(strlen(loid_data.onu_id) <= CTC_AUTH_ONU_ID_SIZE)
    {
       memset(auth_loid_data_checked.onu_id, 0, CTC_AUTH_ONU_ID_SIZE);
       memcpy((auth_loid_data_checked.onu_id + CTC_AUTH_ONU_ID_SIZE - strlen(loid_data.onu_id)), loid_data.onu_id, strlen(loid_data.onu_id)+1); 
    }
    else
    {
        return CTC_STACK_PARAMETER_ERROR;
    }

    for(index = 0; index < PON_ONU_ID_PER_OLT_ARRAY_SIZE; index++)
    {
        if(memcmp(auth_loid_data_checked.onu_id, authentication_db.olt[olt_id].loid[index].onu_id, CTC_AUTH_ONU_ID_SIZE) == 0)
        {
            /*set onu_id and password not used*/
            strcpy((authentication_db.olt[olt_id].loid[index].onu_id), CTC_AUTH_LOID_DATA_NOT_USED);
            strcpy((authentication_db.olt[olt_id].loid[index].password), CTC_AUTH_LOID_DATA_NOT_USED);
            authentication_db.olt[olt_id].onu_llid[index] = PON_NOT_USED_LLID;
            remove_data = TRUE;
            break;
        }        
    }
    
    if(!remove_data)
    {
        PONLOG_ERROR_0( PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, PONLOG_ONU_IRRELEVANT, "auth onu_id data is not found in the loid table\n");
        return CTC_STACK_QUERY_FAILED;
    }

	return (CTC_STACK_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME   


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_auth_mode"

PON_STATUS PON_CTC_STACK_set_auth_mode(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_mode_t          ctc_auth_mode)

{
    short int                  result;
    PON_authentication_mode_t  auth_mode;

    switch(ctc_auth_mode) 
    {
    case CTC_AUTH_MODE_MAC:
        auth_mode = PON_AUTH_MODE_ENABLE_MAC_LIST;
    	break;
    case CTC_AUTH_MODE_LOID:
    case CTC_AUTH_MODE_LOID_NOT_CHK_PWD:
        auth_mode = PON_AUTH_MODE_DISABLE_MAC_LIST;
    	break;
    case CTC_AUTH_MODE_HYBRID:
    case CTC_AUTH_MODE_HYBRID_NOT_CHK_PWD:
        auth_mode = PON_AUTH_MODE_HYBRID;
    	break;
    default:
        PONLOG_ERROR_1(PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, PONLOG_ONU_IRRELEVANT,"OLT %d unknown authentication mode\n", olt_id)
        return CTC_STACK_EXIT_ERROR;
    }

    result = Convert_ponsoft_error_code_to_ctc_stack(
					PAS_set_authentication_mode(olt_id, auth_mode));
	if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_2(PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, PONLOG_ONU_IRRELEVANT,"OLT %d failed to set authentication mode. error code:%d\n", olt_id, result)
        return result;
    }

    authentication_db.olt[olt_id].auth_mode = ctc_auth_mode;
    
    PONLOG_INFO_2(PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, PONLOG_ONU_IRRELEVANT,"OLT %d set authentication mode: %d\n", olt_id, ctc_auth_mode)


	return (CTC_STACK_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_auth_mode"

PON_STATUS PON_CTC_STACK_get_auth_mode(           
					const PON_olt_id_t			         olt_id, 
					CTC_STACK_auth_mode_t               *ctc_auth_mode)

{
    short int                  result;
    PON_authentication_mode_t  auth_mode;

    result = Convert_ponsoft_error_code_to_ctc_stack(
					PAS_get_authentication_mode(olt_id, &auth_mode));
	if ( result != CTC_STACK_EXIT_OK )
    {
        PONLOG_ERROR_2(PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, PONLOG_ONU_IRRELEVANT,"OLT %d failed to get authentication mode. error code:%d\n", olt_id, result)
        return result;
    }

    switch(auth_mode) 
    {
    case PON_AUTH_MODE_ENABLE_MAC_LIST:
        (*ctc_auth_mode) = CTC_AUTH_MODE_MAC;
    	break;
    case PON_AUTH_MODE_DISABLE_MAC_LIST:
        (*ctc_auth_mode) = CTC_AUTH_MODE_LOID;
        if (authentication_db.olt[olt_id].auth_mode == CTC_AUTH_MODE_LOID_NOT_CHK_PWD)
        {
            *ctc_auth_mode = CTC_AUTH_MODE_LOID_NOT_CHK_PWD;
        }
        break;
    case PON_AUTH_MODE_HYBRID:
        (*ctc_auth_mode) = CTC_AUTH_MODE_HYBRID;
        if (authentication_db.olt[olt_id].auth_mode == CTC_AUTH_MODE_HYBRID_NOT_CHK_PWD)
        {
            *ctc_auth_mode = CTC_AUTH_MODE_HYBRID_NOT_CHK_PWD;
        }
    	break;
    default:
        PONLOG_ERROR_1(PONLOG_FUNC_AUTHENTICATION_FLOW, olt_id, PONLOG_ONU_IRRELEVANT,"OLT %d unknown authentication mode\n", olt_id)
        return CTC_STACK_EXIT_ERROR;
    }

	return (CTC_STACK_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME
#endif


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_onu_version"

PON_STATUS PON_CTC_STACK_get_onu_version(
                                    const PON_olt_id_t		   olt_id, 
                                    const PON_onu_id_t		   onu_id,
                                    unsigned char             *onu_version )
{

    (*onu_version) = discovery_db.common_version[olt_id][onu_id];

	return (CTC_STACK_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_optical_transceiver_diagnosis"
PON_STATUS PON_CTC_STACK_get_optical_transceiver_diagnosis ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis )
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_OPTICAL_TRANSCEIVER_DIAGNOSIS;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;


    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	comparser_result = (short int)CTC_PARSE_optical_transceiver_diagnosis
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              optical_transceiver_diagnosis, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_service_sla "
PON_STATUS PON_CTC_STACK_set_service_sla ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const bool									activate,
		   const CTC_STACK_service_sla_t				*service_sla )
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_SERVICE_SLA;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned char								i, received_port_number=0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if (activate)
	{
		PON_HANDLE_ERROR_CHECK_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, service_sla->number_of_service_sla_entries, number_of_service_sla_entries, 1, 8)
		for (i = 0; i < service_sla->number_of_service_sla_entries;i++)
		{
			PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, (service_sla->service_sla_entries[i].queue_number), queue_number, CTC_MIN_SERVICE_SLA_QUEUE_NUMBER, CTC_MAX_SERVICE_SLA_QUEUE_NUMBER)
			PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, (service_sla->service_sla_entries[i].wrr_weight), wrr_weight, CTC_MIN_SERVICE_SLA_WRR_WEIGHT, CTC_MAX_SERVICE_SLA_WRR_WEIGHT)
		}
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_service_sla
							( 1, activate, service_sla, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
	
	return result;    
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_service_sla"
PON_STATUS PON_CTC_STACK_get_service_sla ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   bool											*activate,
		   CTC_STACK_service_sla_t						*service_sla )
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_SERVICE_SLA;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;


    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN


	comparser_result = (short int)CTC_PARSE_service_sla
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              activate, service_sla, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_alarm_admin_state"
PON_STATUS PON_CTC_STACK_set_alarm_admin_state ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			const bool								enable)
{
    short int									result=CTC_STACK_EXIT_OK, comparser_result;
    unsigned short								received_left_length=0, sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_ALARM_ADMIN_STATE;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	CTC_management_object_leaf_t				management_object_leaf;
	CTC_management_object_t						sent_management_object;
	CTC_management_object_t						received_management_object;
	bool										is_onu_alarm = FALSE;


    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	CHECK_ALARM_ID_AND_EXTRACT_MANAGEMENT_OBJECT_LEAF(PONLOG_FUNC_GENERAL, alarm_id, management_object_leaf);


	if (management_object_leaf == CTC_MANAGEMENT_OBJECT_LEAF_NONE)
	{
		is_onu_alarm = TRUE; /* for ONU alarm there is no leaf and therfore no need to use management object */
	}
	else
	{
		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, management_object_index);
	}


    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	if (!is_onu_alarm)
	{
		comparser_result = (short int)CTC_COMPOSE_management_object( 
												sent_management_object,
                                                discovery_db.common_version[olt_id][onu_id],
												SEND_COMPARSER_BUFFER, 
												&sent_length);
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
		UPDATE_SENT_LEN
	}


	comparser_result = (short int)CTC_COMPOSE_alarm_admin_state
								( 1, alarm_id, enable, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
								  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN


	if (!is_onu_alarm)
	{
		comparser_result = (short int)CTC_PARSE_management_object
							( 
								RECEIVED_COMPARSER_BUFFER, 
								RECEIVED_LENGTH_BUFFER,
								TRUE/* error if branch=0x0 */,
                                discovery_db.common_version[olt_id][onu_id],
								&received_management_object,
								&received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
		CHECK_MANAGEMENT_OBJECT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object, sent_management_object)
		UPDATE_RECEIVED_LEN
	}

	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
       					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
		
	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_alarm_admin_state"
PON_STATUS PON_CTC_STACK_get_alarm_admin_state ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			unsigned short							*number_of_alarms,
			CTC_STACK_alarm_admin_state_t			alarms_state[] )
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_ALARM_ADMIN_STATE;
	unsigned char								ctc_ret_val;
	unsigned short								i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t				management_object_leaf;
	CTC_management_object_t						sent_management_object;
	CTC_management_object_t						received_management_object;
	unsigned char								received_num_of_entries=0;
	bool										is_onu_alarm = FALSE;


    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);
	
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	CHECK_ALARM_ID_AND_EXTRACT_MANAGEMENT_OBJECT_LEAF(PONLOG_FUNC_GENERAL, alarm_id, management_object_leaf);
    

	if (management_object_leaf == CTC_MANAGEMENT_OBJECT_LEAF_NONE)
	{
		is_onu_alarm = TRUE; /* for ONU alarm there is no leaf and therfore no need to use management object */
	}
	else
	{
		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, management_object_index);
	}


    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	if (!is_onu_alarm)
	{
		comparser_result = (short int)CTC_COMPOSE_management_object( 
												sent_management_object,
                                                discovery_db.common_version[olt_id][onu_id],
												SEND_COMPARSER_BUFFER, 
												&sent_length);
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
		UPDATE_SENT_LEN
	}

	
	comparser_result = (short int)CTC_COMPOSE_get_alarm_admin_state
								( 1, alarm_id, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
								  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
    
		
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	i = 0;
	while(received_total_length < received_length)
	{
		if (i >= (*number_of_alarms))
		{
			PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Allocated size of alarms_state[] array is too small (number_of_alarms=%d).\n", number_of_alarms );   

			return (CTC_STACK_PARAMETER_ERROR);
		}

		memset(&alarms_state[i].management_object_index, 0, sizeof(alarms_state[i].management_object_index));
			
		if (!is_onu_alarm)
		{
			comparser_result = (short int)CTC_PARSE_management_object
							( 
								RECEIVED_COMPARSER_BUFFER, 
								RECEIVED_LENGTH_BUFFER,
								FALSE/* error if branch=0x0 */,
                                discovery_db.common_version[olt_id][onu_id],
								&received_management_object,
								&received_left_length );
            if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
                break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_MANAGEMENT_OBJECT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object, sent_management_object)
			UPDATE_RECEIVED_LEN

			memcpy(&alarms_state[i].management_object_index, &received_management_object.index, sizeof(alarms_state[i].management_object_index));
		}

		received_num_of_entries = 0;
		comparser_result = (short int)CTC_PARSE_alarm_admin_state
                                ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                      &alarms_state[i].alarm_id, &alarms_state[i].enable, &ctc_ret_val ,&received_left_length );
        if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
            break;
		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
		UPDATE_RECEIVED_LEN
		
		i++;
	}

	(*number_of_alarms) = i;
    
	return result;

}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_alarm_threshold"
PON_STATUS PON_CTC_STACK_set_alarm_threshold ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			const unsigned long						alarm_threshold,
			const unsigned long						clear_threshold )
{
    short int									result=CTC_STACK_EXIT_OK, comparser_result, index;
    unsigned short								received_left_length=0, sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf  =  CTC_EX_VAR_ALARM_THRESHOLD;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	CTC_management_object_leaf_t				management_object_leaf;
	CTC_management_object_t						sent_management_object;
	CTC_management_object_t						received_management_object;
	bool										is_onu_alarm = FALSE, found_alarm_id = FALSE;
   

    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	CHECK_ALARM_ID_AND_EXTRACT_MANAGEMENT_OBJECT_LEAF(PONLOG_FUNC_GENERAL, alarm_id, management_object_leaf);
    

    /*check if alarm_id exist in table*/
    index = 0;
    while (strlen(alarm_str_array[index].alarm_str) > 0)
    {
        if (alarm_str_array[index].alarm_id == alarm_id)
        {
            found_alarm_id = TRUE;
            break;
        }
        index++;
    }
    
    if(found_alarm_id)
    {
        if(alarm_str_array[index].alarm_info_type != TEST_VALUE_ALARM_INFO_TYPE)
        {
            PONLOG_ERROR_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT %d ONU %d. Threshold can't be set to this alarm id: 0x%x alarm str: %s\n",
                            olt_id, onu_id, alarm_id, (alarm_str_array[index].alarm_str));
            return CTC_STACK_NOT_IMPLEMENTED;
        }
    }
    else
    {
        PONLOG_ERROR_3( PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT %d ONU %d. unknown alarm id: 0x%x\n",
                        olt_id, onu_id, alarm_id);
        return CTC_STACK_PARAMETER_ERROR;
    }
	

	if (management_object_leaf == CTC_MANAGEMENT_OBJECT_LEAF_NONE)
	{
		is_onu_alarm = TRUE; /* for ONU alarm there is no leaf and therfore no need to use management object */
	}
	else
	{
		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, management_object_index);
	}


    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	if (!is_onu_alarm)
	{
		comparser_result = (short int)CTC_COMPOSE_management_object( 
												sent_management_object,
                                                discovery_db.common_version[olt_id][onu_id],
												SEND_COMPARSER_BUFFER, 
												&sent_length);
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
		UPDATE_SENT_LEN
	}


	comparser_result = (short int)CTC_COMPOSE_alarm_threshold
								( 1, alarm_id, alarm_threshold, clear_threshold, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
								  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	
    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN


	if (!is_onu_alarm)
	{
		comparser_result = (short int)CTC_PARSE_management_object
							( 
								RECEIVED_COMPARSER_BUFFER, 
								RECEIVED_LENGTH_BUFFER,
								TRUE/* error if branch=0x0 */,
                                discovery_db.common_version[olt_id][onu_id],
								&received_management_object,
								&received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
		CHECK_MANAGEMENT_OBJECT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object, sent_management_object)
		UPDATE_RECEIVED_LEN
	}

	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
       					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
		
	return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_alarm_threshold"
PON_STATUS PON_CTC_STACK_get_alarm_threshold ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			unsigned short							*number_of_alarms,
			CTC_STACK_alarm_threshold_t				alarms_threshold[] )
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result, index;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_ALARM_THRESHOLD;
	unsigned char								ctc_ret_val;
	unsigned short								i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t				management_object_leaf;
	CTC_management_object_t						sent_management_object;
	CTC_management_object_t						received_management_object;
	unsigned char								received_num_of_entries=0;
	bool										is_onu_alarm = FALSE, found_alarm_id = FALSE;


	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);
    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	CHECK_ALARM_ID_AND_EXTRACT_MANAGEMENT_OBJECT_LEAF(PONLOG_FUNC_GENERAL, alarm_id, management_object_leaf);


    /*check if alarm_id exist in table*/
    index = 0;
    while (strlen(alarm_str_array[index].alarm_str) > 0)
    {
        if (alarm_str_array[index].alarm_id == alarm_id)
        {
            found_alarm_id = TRUE;
            break;
        }
        index++;
    }
    
    if(found_alarm_id)
    {
        if(alarm_str_array[index].alarm_info_type != TEST_VALUE_ALARM_INFO_TYPE)
        {
            PONLOG_ERROR_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT %d ONU %d. Threshold can't be set to this alarm id: 0x%x alarm str: %s\n",
                            olt_id, onu_id, alarm_id, (alarm_str_array[index].alarm_str));
            return CTC_STACK_NOT_IMPLEMENTED;
        }
    }
    else
    {
        PONLOG_ERROR_3( PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT %d ONU %d. unknown alarm id: 0x%x\n",
                        olt_id, onu_id, alarm_id);
        return CTC_STACK_PARAMETER_ERROR;
    }
    

	if (management_object_leaf == CTC_MANAGEMENT_OBJECT_LEAF_NONE)
	{
		is_onu_alarm = TRUE; /* for ONU alarm there is no leaf and therfore no need to use management object */
	}
	else
	{
		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, management_object_index);
	}


    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	if (!is_onu_alarm)
	{
		comparser_result = (short int)CTC_COMPOSE_management_object( 
												sent_management_object,
                                                discovery_db.common_version[olt_id][onu_id],
												SEND_COMPARSER_BUFFER, 
												&sent_length);
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
		UPDATE_SENT_LEN
	}

	
	comparser_result = (short int)CTC_COMPOSE_get_alarm_threshold
								( 1, alarm_id, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
								  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
    
		
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

		
	i = 0;
	while(received_total_length < received_length)
	{
		if (i >= (*number_of_alarms))
		{
			PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Allocated size of alarms_threshold[] array is too small (number_of_alarms=%d).\n", number_of_alarms );   

			return (CTC_STACK_PARAMETER_ERROR);
		}

		memset(&alarms_threshold[i].management_object_index, 0, sizeof(alarms_threshold[i].management_object_index));
			
		if (!is_onu_alarm)
		{
			comparser_result = (short int)CTC_PARSE_management_object
							( 
								RECEIVED_COMPARSER_BUFFER, 
								RECEIVED_LENGTH_BUFFER,
								FALSE/* error if branch=0x0 */,
                                discovery_db.common_version[olt_id][onu_id],
								&received_management_object,
								&received_left_length );
            if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
                break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_MANAGEMENT_OBJECT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object, sent_management_object)
			UPDATE_RECEIVED_LEN

			memcpy(&alarms_threshold[i].management_object_index, &received_management_object.index, sizeof(alarms_threshold[i].management_object_index));
		}

		received_num_of_entries = 0;

		comparser_result = (short int)CTC_PARSE_alarm_threshold
                                ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                      &alarms_threshold[i].alarm_id, &alarms_threshold[i].alarm_threshold, &alarms_threshold[i].clear_threshold, &ctc_ret_val ,&received_left_length );
        if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
            break;
		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
		UPDATE_RECEIVED_LEN
		
		i++;
	}

	(*number_of_alarms) = i;
    
	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_reset_card"
PON_STATUS PON_CTC_STACK_reset_card ( 
			const PON_olt_id_t					olt_id, 
			const PON_onu_id_t					onu_id,
			unsigned char						frame_number,
			unsigned char						slot_number )
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ACTION;
	unsigned short								expected_leaf  =  CTC_EX_ACTION_RESET_CARD;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned char								received_port_number=0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	CTC_management_object_index_t				management_object_index;
	CTC_management_object_t						sent_management_object;
	CTC_management_object_t						received_management_object;

        
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	management_object_index.frame_number	= frame_number;
	management_object_index.slot_number		= slot_number;
	management_object_index.port_number		= 0;
    management_object_index.port_type       = CTC_MANAGEMENT_OBJECT_PORT_TYPE_NONE;

	BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, CTC_MANAGEMENT_OBJECT_LEAF_CARD, management_object_index);
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_management_object( 
											sent_management_object,
                                            discovery_db.common_version[olt_id][onu_id],
											SEND_COMPARSER_BUFFER, 
											&sent_length);
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_management_object
					( 
						RECEIVED_COMPARSER_BUFFER, 
						RECEIVED_LENGTH_BUFFER,
						TRUE/* error if branch=0x0 */,
                        discovery_db.common_version[olt_id][onu_id],
						&received_management_object,
						&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	CHECK_MANAGEMENT_OBJECT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object, sent_management_object)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_branch,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN


	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_single_llid_queue_config"
PON_STATUS PON_CTC_STACK_set_single_llid_queue_config ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_llid_queue_config_t		    llid_queue_config )
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_LLID_QUEUE_CONFIG;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned char								i, received_port_number=0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;


	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN


	PON_HANDLE_ERROR_CHECK_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, llid_queue_config.number_of_llid_queue_config_entries, number_of_llid_queue_config_entries, 1, CTC_MAX_LLID_QUEUE_CONFIG_ENTRIES)
	for (i = 0; i < llid_queue_config.number_of_llid_queue_config_entries;i++)
	{
		PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, (llid_queue_config.llid_queue_config_entries[i].wrr_weight), wrr_weight, CTC_MIN_SERVICE_SLA_WRR_WEIGHT, CTC_MAX_SERVICE_SLA_WRR_WEIGHT)
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_llid_queue_config
							( 1, llid_queue_config, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN
    
    comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
	
	return result;    
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_single_llid_queue_config"
PON_STATUS PON_CTC_STACK_get_single_llid_queue_config ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_llid_queue_config_t			   *llid_queue_config )
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_LLID_QUEUE_CONFIG;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	comparser_result = (short int)CTC_PARSE_llid_queue_config
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              llid_queue_config, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_llid_queue_config"
PON_STATUS PON_CTC_STACK_set_llid_queue_config ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_llid_queue_config_t		    llid_queue_config )
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_LLID_QUEUE_CONFIG;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned char								i, received_port_number=0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;


	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN


	PON_HANDLE_ERROR_CHECK_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, llid_queue_config.number_of_llid_queue_config_entries, number_of_llid_queue_config_entries, 1, CTC_MAX_LLID_QUEUE_CONFIG_ENTRIES)
	for (i = 0; i < llid_queue_config.number_of_llid_queue_config_entries;i++)
	{
		PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(PONLOG_FUNC_GENERAL, (llid_queue_config.llid_queue_config_entries[i].wrr_weight), wrr_weight, CTC_MIN_SERVICE_SLA_WRR_WEIGHT, CTC_MAX_SERVICE_SLA_WRR_WEIGHT)
	}
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

    comparser_result = (short int)CTC_COMPOSE_llid_object
                            ( llid_queue_config.llid,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_llid_queue_config
							( 1, llid_queue_config, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	comparser_result = (short int)CTC_PARSE_llid_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, llid_queue_config.llid, &received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
    UPDATE_RECEIVED_LEN
    
    comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
	
	return result;    
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_llid_queue_config"
PON_STATUS PON_CTC_STACK_get_llid_queue_config ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_llid_queue_config_t			   *llid_queue_config )
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_LLID_QUEUE_CONFIG;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

    comparser_result = (short int)CTC_COMPOSE_llid_object
                            ( llid_queue_config->llid,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	comparser_result = (short int)CTC_PARSE_llid_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, llid_queue_config->llid, &received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
    UPDATE_RECEIVED_LEN

	comparser_result = (short int)CTC_PARSE_llid_queue_config
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              llid_queue_config, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multi_llid_admin_control"
PON_STATUS PON_CTC_STACK_set_multi_llid_admin_control ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t					        onu_id,
		   const unsigned long                          num_of_llid_activated)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ACTION;
	unsigned short								expected_leaf  =  CTC_EX_ACTION_MULTI_LLID_ADMIN_CONTROL;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned char								received_port_number=0;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;


    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_multi_llid_admin_control
							( 1, num_of_llid_activated, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
    UPDATE_RECEIVED_LEN
 	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
    UPDATE_RECEIVED_LEN


	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_voip_pots_status"
PON_STATUS PON_CTC_STACK_get_voip_pots_status ( 
			const PON_olt_id_t						    olt_id, 
			const PON_onu_id_t						    onu_id,
			const CTC_management_object_index_t		    management_object_index,
     	    unsigned char					           *number_of_entries,
			CTC_STACK_voip_pots_status_array_t       *pots_status_array)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_VOIP_POTS_STATUS;
	unsigned char								ctc_ret_val;
	unsigned char								i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t				management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t						sent_management_object;
	CTC_management_object_t						received_management_object;
	unsigned char								received_num_of_entries=0;


	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);
    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    
    BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, management_object_index);

    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_management_object( 
												sent_management_object,
                                                discovery_db.common_version[olt_id][onu_id],
												SEND_COMPARSER_BUFFER, 
												&sent_length);
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

    comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
    
		
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

		
	i = 0;
	while(received_total_length < received_length)
	{
		if (i >= (*number_of_entries))
		{
			PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						    "Allocated size of pots_status_array is too small (number_of_entries=%d).\n", number_of_entries);   
			return (CTC_STACK_PARAMETER_ERROR);
		}

		memset(&((*pots_status_array)[i].management_object_index), 0, sizeof((*pots_status_array)[i].management_object_index));


		comparser_result = (short int)CTC_PARSE_management_object
							( 
								RECEIVED_COMPARSER_BUFFER, 
								RECEIVED_LENGTH_BUFFER,
								FALSE/* error if branch=0x0 */,
                                discovery_db.common_version[olt_id][onu_id],
								&received_management_object,
								&received_left_length );
        if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
            break;
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
		CHECK_MANAGEMENT_OBJECT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object, sent_management_object)
		UPDATE_RECEIVED_LEN

		
        memcpy(&((*pots_status_array)[i].management_object_index), &received_management_object.index, sizeof((*pots_status_array)[i].management_object_index));


		received_num_of_entries = 0;

		comparser_result = (short int)CTC_PARSE_voip_pots_status
                                ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                      &((*pots_status_array)[i].pots_status), &ctc_ret_val ,&received_left_length );
        if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
            break;
		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
		UPDATE_RECEIVED_LEN
		i++;
	}

	(*number_of_entries) = i;
    
	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_voip_iad_info"
PON_STATUS PON_CTC_STACK_get_voip_iad_info( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_voip_iad_info_t	     	       *iad_info)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_VOIP_IAD_INFO;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN


	comparser_result = (short int)CTC_PARSE_voip_iad_info
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              iad_info, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_voip_global_param_conf"
PON_STATUS PON_CTC_STACK_set_voip_global_param_conf( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_voip_global_param_conf_t     global_param)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_VOIP_GLOBAL_PARAM_CONF;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_voip_global_param_conf
							( 1, global_param, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
	
	return result;    
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_voip_global_param_conf"
PON_STATUS PON_CTC_STACK_get_voip_global_param_conf( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_voip_global_param_conf_t          *global_param)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_VOIP_GLOBAL_PARAM_CONF;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN


	comparser_result = (short int)CTC_PARSE_voip_global_param_conf
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              global_param, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_h248_param_config"
PON_STATUS PON_CTC_STACK_set_h248_param_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_h248_param_config_t          h248_param)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_H248_PARAM_CONFIG;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_h248_param_config
							( 1, h248_param, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
	
	return result;    
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_h248_param_config"
PON_STATUS PON_CTC_STACK_get_h248_param_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_h248_param_config_t    	       *h248_param)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_H248_PARAM_CONFIG;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN


	comparser_result = (short int)CTC_PARSE_h248_param_config
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              h248_param, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_h248_user_tid_multi_management_object_config"
PON_STATUS PON_CTC_STACK_set_h248_user_tid_multi_management_object_config( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   const unsigned char					               number_of_entries,
		   const CTC_STACK_h248_user_tid_config_array_t  user_tid_config_array )
{
    short int								result,comparser_result;
    unsigned short							received_left_length = 0, sent_length=ETHERNET_MTU, received_length;
	unsigned char							send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf  =  CTC_EX_VAR_H248_USER_TID_CONFIG;
	unsigned char							received_num_of_entries=number_of_entries;
	unsigned char							ctc_ret_val,i;
	unsigned short							max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char							local_number_of_entries=0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    if(number_of_entries != CTC_STACK_ALL_PORTS)
    {
        for(i = 0; i < number_of_entries; i++)
        {
            CHECK_CTC_STACK_OBJECT_VOIP_PORT_RANGE(PONLOG_FUNC_GENERAL, user_tid_config_array[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
        }
    }

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (user_tid_config_array[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_h248_user_tid_config
									( 1, user_tid_config_array[i].h248_user_tid_config, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (user_tid_config_array[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_h248_user_tid_config
									( 1, user_tid_config_array[0].h248_user_tid_config, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}


	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


		received_total_length += received_left_length;

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, user_tid_config_array[i].management_object_index.port_number)
		}


		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
		
	}
	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_h248_user_tid_config"

PON_STATUS PON_CTC_STACK_set_h248_user_tid_config ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_h248_user_tid_config_t   user_tid_config)
{
    short int								           result=CTC_STACK_EXIT_OK;
    CTC_STACK_h248_user_tid_config_array_t  user_tid_config_array;
    unsigned char						               number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    user_tid_config_array[0].management_object_index.port_number      = management_object_index.port_number;
    user_tid_config_array[0].management_object_index.slot_number      = management_object_index.slot_number;
    user_tid_config_array[0].management_object_index.frame_number     = management_object_index.frame_number;
    user_tid_config_array[0].management_object_index.port_type        = management_object_index.port_type;

    memset(&(user_tid_config_array[0].h248_user_tid_config), 0, sizeof(user_tid_config_array[0].h248_user_tid_config));	
    memcpy(&(user_tid_config_array[0].h248_user_tid_config), &user_tid_config, sizeof(user_tid_config_array[0].h248_user_tid_config));
		  

	result = PON_CTC_STACK_set_h248_user_tid_multi_management_object_config(olt_id, onu_id, number_of_entries, user_tid_config_array);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_h248_user_tid_config"
PON_STATUS PON_CTC_STACK_get_h248_user_tid_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
           const CTC_management_object_index_t		    management_object_index,
     	    unsigned char					           *number_of_entries,
		   CTC_STACK_h248_user_tid_config_array_t    	       *h248_user_tid_array)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_H248_USER_TID_CONFIG;
	unsigned char								ctc_ret_val;
	unsigned char								i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t				management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t						sent_management_object;
	CTC_management_object_t						received_management_object;
	unsigned char								received_num_of_entries=0;

 
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);
    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    
    BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, management_object_index);

    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
 
	comparser_result = (short int)CTC_COMPOSE_management_object( 
												sent_management_object,
                                                discovery_db.common_version[olt_id][onu_id],
												SEND_COMPARSER_BUFFER, 
												&sent_length);
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

    comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
    
		
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

		
	i = 0;
	while(received_total_length < received_length)
	{
		if (i >= (*number_of_entries))
		{
			PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						    "Allocated size of userif_status_array is too small (number_of_entries=%d).\n", number_of_entries);   
			return (CTC_STACK_PARAMETER_ERROR);
		}

		memset(&((*h248_user_tid_array)[i].management_object_index), 0, sizeof((*h248_user_tid_array)[i].management_object_index));


		comparser_result = (short int)CTC_PARSE_management_object
							( 
								RECEIVED_COMPARSER_BUFFER, 
								RECEIVED_LENGTH_BUFFER,
								FALSE/* error if branch=0x0 */,
                                discovery_db.common_version[olt_id][onu_id],
								&received_management_object,
								&received_left_length );
        if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
            break;
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
		CHECK_MANAGEMENT_OBJECT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object, sent_management_object)
		UPDATE_RECEIVED_LEN

		
        memcpy(&((*h248_user_tid_array)[i].management_object_index), &received_management_object.index, sizeof((*h248_user_tid_array)[i].management_object_index));


		received_num_of_entries = 0;

		comparser_result = (short int)CTC_PARSE_h248_user_tid_config
                                ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                      &((*h248_user_tid_array)[i].h248_user_tid_config), &ctc_ret_val ,&received_left_length );
        if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
            break;
		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
		UPDATE_RECEIVED_LEN
		i++;
	}

	(*number_of_entries) = i;
    
	return result;
}	 
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_h248_rtp_tid_config"
PON_STATUS PON_CTC_STACK_set_h248_rtp_tid_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_h248_rtp_tid_config_t       h248_rtp_tid_config)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_H248_RTP_TID_CONFIG;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_h248_rtp_tid_config
							( 1, h248_rtp_tid_config, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
	
	return result;    
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_h248_rtp_tid_info"
PON_STATUS PON_CTC_STACK_get_h248_rtp_tid_info( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_h248_rtp_tid_info_t    	       *h248_rtp_tid_info)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_H248_RTP_TID_INFO;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN


	comparser_result = (short int)CTC_PARSE_h248_rtp_tid_info
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              h248_rtp_tid_info, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN

	return result;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_sip_param_config"
PON_STATUS PON_CTC_STACK_set_sip_param_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_sip_param_config_t           sip_param)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_SIP_PARAM_CONFIG;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_sip_param_config
							( 1, sip_param, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
	
	return result;    
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_sip_param_config"
PON_STATUS PON_CTC_STACK_get_sip_param_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_sip_param_config_t    	           *sip_param)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_SIP_PARAM_CONFIG;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN


	comparser_result = (short int)CTC_PARSE_sip_param_config
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              sip_param, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_sip_user_param_multi_management_object_config"
PON_STATUS PON_CTC_STACK_set_sip_user_param_multi_management_object_config( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   const unsigned char					               number_of_entries,
		   const CTC_STACK_sip_user_param_config_array_t  sip_user_param_array )
{
    short int								result,comparser_result;
    unsigned short							received_left_length = 0, sent_length=ETHERNET_MTU, received_length;
	unsigned char							send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf  =  CTC_EX_VAR_SIP_USER_PARAM_CONFIG;
	unsigned char							received_num_of_entries=number_of_entries;
	unsigned char							ctc_ret_val,i;
	unsigned short							max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char							local_number_of_entries=0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    if(number_of_entries != CTC_STACK_ALL_PORTS)
    {
        for(i = 0; i < number_of_entries; i++)
        {
            CHECK_CTC_STACK_OBJECT_VOIP_PORT_RANGE(PONLOG_FUNC_GENERAL, sip_user_param_array[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
        }
    }

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (sip_user_param_array[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_sip_user_param_config
									( 1, sip_user_param_array[i].sip_user_param_config, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (sip_user_param_array[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_sip_user_param_config
									( 1, sip_user_param_array[0].sip_user_param_config, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}


	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


		received_total_length += received_left_length;

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, sip_user_param_array[i].management_object_index.port_number)
		}


		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
		
	}
	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_sip_user_param_config"

PON_STATUS PON_CTC_STACK_set_sip_user_param_config ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_sip_user_param_config_t   sip_user_param)
{
    short int								           result=CTC_STACK_EXIT_OK;
    CTC_STACK_sip_user_param_config_array_t  sip_user_param_array;
    unsigned char						               number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
    {
		number_of_entries = CTC_STACK_ALL_PORTS;
    }
	else
    {
		number_of_entries = 1;
    }

    sip_user_param_array[0].management_object_index.port_number      = management_object_index.port_number;
    sip_user_param_array[0].management_object_index.slot_number      = management_object_index.slot_number;
    sip_user_param_array[0].management_object_index.frame_number     = management_object_index.frame_number;
    sip_user_param_array[0].management_object_index.port_type        = management_object_index.port_type;

    memset(&(sip_user_param_array[0].sip_user_param_config), 0, sizeof(sip_user_param_array[0].sip_user_param_config));	
    memcpy(&(sip_user_param_array[0].sip_user_param_config), &sip_user_param, sizeof(sip_user_param_array[0].sip_user_param_config));
		  

	result = PON_CTC_STACK_set_sip_user_param_multi_management_object_config(olt_id, onu_id, number_of_entries, sip_user_param_array);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_sip_user_param_config"
PON_STATUS PON_CTC_STACK_get_sip_user_param_config(const PON_olt_id_t						    olt_id, 
			const PON_onu_id_t						    onu_id,
			const CTC_management_object_index_t		    management_object_index,
     	    unsigned char					           *number_of_entries,
           CTC_STACK_sip_user_param_config_array_t       *sip_user_param_array)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_SIP_USER_PARAM_CONFIG;
	unsigned char								ctc_ret_val;
	unsigned char								i;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t				management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t						sent_management_object;
	CTC_management_object_t						received_management_object;
	unsigned char								received_num_of_entries=0;

 
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);
    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    
    BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, management_object_index);

    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
 
	comparser_result = (short int)CTC_COMPOSE_management_object( 
												sent_management_object,
                                                discovery_db.common_version[olt_id][onu_id],
												SEND_COMPARSER_BUFFER, 
												&sent_length);
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

    comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
    
		
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

		
	i = 0;
	while(received_total_length < received_length)
	{
		if (i >= (*number_of_entries))
		{
			PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						    "Allocated size of userif_status_array is too small (number_of_entries=%d).\n", number_of_entries);   
			return (CTC_STACK_PARAMETER_ERROR);
		}

		memset(&((*sip_user_param_array)[i].management_object_index), 0, sizeof((*sip_user_param_array)[i].management_object_index));


		comparser_result = (short int)CTC_PARSE_management_object
							( 
								RECEIVED_COMPARSER_BUFFER, 
								RECEIVED_LENGTH_BUFFER,
								FALSE/* error if branch=0x0 */,
                                discovery_db.common_version[olt_id][onu_id],
								&received_management_object,
								&received_left_length );
        if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
            break;
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
		CHECK_MANAGEMENT_OBJECT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object, sent_management_object)
		UPDATE_RECEIVED_LEN

		
        memcpy(&((*sip_user_param_array)[i].management_object_index), &received_management_object.index, sizeof((*sip_user_param_array)[i].management_object_index));


		received_num_of_entries = 0;

		comparser_result = (short int)CTC_PARSE_sip_user_param_config
                                ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                      &((*sip_user_param_array)[i].sip_user_param_config), &ctc_ret_val ,&received_left_length );
        if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
            break;
		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
		UPDATE_RECEIVED_LEN
		i++;
	}

	(*number_of_entries) = i;
    
	return result;
}		  
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_voip_fax_config"
PON_STATUS PON_CTC_STACK_set_voip_fax_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_voip_fax_config_t            voip_fax)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_VOIP_FAX_CONFIG;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_voip_fax_config
							( 1, voip_fax, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
	
	return result;    
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_redundancy_database"
PON_STATUS PON_CTC_STACK_set_redundancy_database(           
		   const PON_olt_id_t			                olt_id,
		   const PON_onu_id_t			                onu_id,
           const CTC_STACK_redundancy_database_t        redundancy_db)
{
    short int index;
    bool auth_copied = FALSE;

    /* set ONU state to COMPLETE */
    discovery_db.state[olt_id][onu_id] = redundancy_db.state;

    /* update extended OAM discovery version information to largest_common_version found */
    discovery_db.common_version[olt_id][onu_id] = redundancy_db.common_version;
    
    /* update maximum_data_frame_size */ 
    discovery_db.maximum_data_frame_size[olt_id][onu_id] = redundancy_db.maximum_data_frame_size;
    
#ifdef CTC_AUTH_SUPPORT
    /* update auth_mode */
    authentication_db.olt[olt_id].auth_mode = redundancy_db.auth_db.auth_mode;
    
	if (redundancy_db.auth_db.onu_llid[0] != PON_NOT_USED_LLID)
	{
	    for(index = 0; index < PON_ONU_ID_PER_OLT_ARRAY_SIZE; index++)
    	{
        	if(memcmp(redundancy_db.auth_db.loid[0].onu_id, authentication_db.olt[olt_id].loid[index].onu_id, CTC_AUTH_ONU_ID_SIZE) == 0)
	        {
    	        /*onu_id already exist*/
        	    memcpy((authentication_db.olt[olt_id].loid[index].onu_id), (redundancy_db.auth_db.loid[0].onu_id), CTC_AUTH_ONU_ID_SIZE);
            	memcpy((authentication_db.olt[olt_id].loid[index].password), (redundancy_db.auth_db.loid[0].password), CTC_AUTH_PASSWORD_SIZE);
    	        /* update auth onu_llid */
	            authentication_db.olt[olt_id].onu_llid[index] = redundancy_db.auth_db.onu_llid[0];
        	    auth_copied = TRUE;
            	break;
        	}
		}

	    /*Add this record if it's not exist*/
    	if(auth_copied == FALSE)
	    {
    	    for(index = 0; index < PON_ONU_ID_PER_OLT_ARRAY_SIZE; index++)
        	{
	            if(memcmp(CTC_AUTH_LOID_DATA_NOT_USED, authentication_db.olt[olt_id].loid[index].onu_id, sizeof(CTC_AUTH_LOID_DATA_NOT_USED)) == 0)
    	        {
        	        /* update auth onu_id & password */
            	    memcpy((authentication_db.olt[olt_id].loid[index].onu_id), (redundancy_db.auth_db.loid[0].onu_id), CTC_AUTH_ONU_ID_SIZE);
                	memcpy((authentication_db.olt[olt_id].loid[index].password), (redundancy_db.auth_db.loid[0].password), CTC_AUTH_PASSWORD_SIZE);
        	        /* update auth onu_llid */
	                authentication_db.olt[olt_id].onu_llid[index] = redundancy_db.auth_db.onu_llid[0];
    	            auth_copied = TRUE;
            	    break;
            	}
        	}
    	}
	}
	else
	{
		/* This ONU id do not have used loid&password, need to remove the existed entry */
		for(index = 0; index < PON_ONU_ID_PER_OLT_ARRAY_SIZE; index++)
    	{
        	if(authentication_db.olt[olt_id].onu_llid[index] == onu_id)
	        {
    	        authentication_db.olt[olt_id].onu_llid[index] = PON_NOT_USED_LLID;
        	}
		}
	}
#endif

    /* update encryption_type */
    encryption_db.olt[olt_id].encryption_type = redundancy_db.encryption_type;
    /* B--remmed by liwei056@2011-1-11 for SemCopyBug */
#if 0
    encryption_db.olt[olt_id].semaphore       = redundancy_db.enc_semaphore;
#endif
    /* E--remmed by liwei056@2011-1-11 for SemCopyBug */

    /* update encryption_onu_database */
    encryption_db.olt[olt_id].onu_id[onu_id].response_encryption_state        = redundancy_db.enc_db.response_encryption_state;
    encryption_db.olt[olt_id].onu_id[onu_id].key_index                        = redundancy_db.enc_db.key_index;                              
    encryption_db.olt[olt_id].onu_id[onu_id].temp_key_index                   = redundancy_db.enc_db.temp_key_index;                        
    encryption_db.olt[olt_id].onu_id[onu_id].start_encryption_by_the_user     = redundancy_db.enc_db.start_encryption_by_the_user;           
    encryption_db.olt[olt_id].onu_id[onu_id].encryption_state                 = redundancy_db.enc_db.encryption_state;                       
    encryption_db.olt[olt_id].onu_id[onu_id].number_of_sending_request        = redundancy_db.enc_db.number_of_sending_request;              
    encryption_db.olt[olt_id].onu_id[onu_id].key_request_received_length      = redundancy_db.enc_db.key_request_received_length;            
    
    memcpy(encryption_db.olt[olt_id].onu_id[onu_id].key_value, redundancy_db.enc_db.key_value, CTC_ENCRYPTION_KEY_SIZE);
    memcpy(encryption_db.olt[olt_id].onu_id[onu_id].key_request_received_oam_pdu_data, redundancy_db.enc_db.key_request_received_oam_pdu_data, 100);

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_redundancy_database"
PON_STATUS PON_CTC_STACK_get_redundancy_database(           
		   const PON_olt_id_t			                olt_id,
		   const PON_onu_id_t			                onu_id,
           CTC_STACK_redundancy_database_t             *redundancy_db)
{
    short int index;
    short int index_copied = 0;

    /* set ONU state to COMPLETE */
    redundancy_db->state = discovery_db.state[olt_id][onu_id];

    /* update extended OAM discovery version information to largest_common_version found */
    redundancy_db->common_version = discovery_db.common_version[olt_id][onu_id];
    
    /* update maximum_data_frame_size */ 
    redundancy_db->maximum_data_frame_size = discovery_db.maximum_data_frame_size[olt_id][onu_id];
    
#ifdef CTC_AUTH_SUPPORT
    /* update auth_mode */
    redundancy_db->auth_db.auth_mode = authentication_db.olt[olt_id].auth_mode;
    
	redundancy_db->auth_db.onu_llid[index_copied] = PON_NOT_USED_LLID;

    for(index = 0; index < PON_ONU_ID_PER_OLT_ARRAY_SIZE; index++)
    {
        if(authentication_db.olt[olt_id].onu_llid[index] == onu_id)
        {
            /* update auth onu_id & password */
            memcpy((redundancy_db->auth_db.loid[index_copied].onu_id), (authentication_db.olt[olt_id].loid[index].onu_id), CTC_AUTH_ONU_ID_SIZE);
            memcpy((redundancy_db->auth_db.loid[index_copied].password), (authentication_db.olt[olt_id].loid[index].password), CTC_AUTH_PASSWORD_SIZE);
            /* update auth onu_llid */
            redundancy_db->auth_db.onu_llid[index_copied] = authentication_db.olt[olt_id].onu_llid[index];
            break;
        }
    }
#endif

    /* update encryption_type */
    redundancy_db->encryption_type = encryption_db.olt[olt_id].encryption_type;
    /* B--remmed by liwei056@2011-1-11 for SemCopyBug */
#if 0
    redundancy_db->enc_semaphore = encryption_db.olt[olt_id].semaphore;
#endif
    /* E--remmed by liwei056@2011-1-11 for SemCopyBug */

    /* update encryption_onu_database */
    redundancy_db->enc_db.response_encryption_state    = encryption_db.olt[olt_id].onu_id[onu_id].response_encryption_state;
    redundancy_db->enc_db.temp_key_index               = encryption_db.olt[olt_id].onu_id[onu_id].temp_key_index;                                        
    redundancy_db->enc_db.key_index                    = redundancy_db->enc_db.temp_key_index; 
    redundancy_db->enc_db.start_encryption_by_the_user = encryption_db.olt[olt_id].onu_id[onu_id].start_encryption_by_the_user;             
    redundancy_db->enc_db.encryption_state             = encryption_db.olt[olt_id].onu_id[onu_id].encryption_state;                                  
    redundancy_db->enc_db.number_of_sending_request    = encryption_db.olt[olt_id].onu_id[onu_id].number_of_sending_request;                     
    redundancy_db->enc_db.key_request_received_length  = encryption_db.olt[olt_id].onu_id[onu_id].key_request_received_length;              
    
    memcpy(redundancy_db->enc_db.key_value, encryption_db.olt[olt_id].onu_id[onu_id].key_value, CTC_ENCRYPTION_KEY_SIZE);
    memcpy(redundancy_db->enc_db.key_request_received_oam_pdu_data, encryption_db.olt[olt_id].onu_id[onu_id].key_request_received_oam_pdu_data, 100);

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_redundancy_info"
PON_STATUS PON_CTC_STACK_get_redundancy_info(
                                        CTC_STACK_rdn_info_type_t   info_type,    
                                        const PON_olt_id_t   olt_id,
                                        const short int  max_onu_entry,
                                        char*  info_data_buf,
                                        int  info_buf_len,
                                        int*  info_length,
                                        bool* fragment)
{
    char *p_temp;
    short int byte_count;
    int byte_used;
    bool first_block;
    short int result;
    
    if((info_data_buf==NULL) || (olt_id>PON_OLT_ID_ARRAY_SIZE))
    {
        /**/
        return CTC_STACK_PARAMETER_ERROR;
    }

    p_temp = info_data_buf;
    byte_count = 0;

    /*data block:  info_type(1B)+info_length(2B)+first_block(1B)+PAYLOAD(N)*/
    p_temp[0]=info_type;
    byte_count += 4;

    switch(info_type)
    {
    case CTC_STACK_PON_REDUNDANCY_DISCOVERY_INFO:                
        result = assemble_rdn_data_discovery(olt_id, max_onu_entry, &p_temp[4], (info_buf_len-byte_count), &byte_used, &first_block, fragment);
        if(result != CTC_STACK_EXIT_OK)
            return (result);
                
        break;
    case CTC_STACK_PON_REDUNDANCY_ENCRYPTION_DB:
        result = assemble_rdn_data_encryption(olt_id, max_onu_entry, &p_temp[4], (info_buf_len-byte_count), &byte_used, &first_block, fragment);
        if(result != CTC_STACK_EXIT_OK)
            return (result);        
        
        break;
    case CTC_STACK_PON_REDUNDANCY_AUTHENTICATION_DB:
        result = assemble_rdn_data_authentication(olt_id, max_onu_entry, &p_temp[4], (info_buf_len-byte_count), &byte_used, &first_block, fragment);
        if(result != CTC_STACK_EXIT_OK)
            return (result);        
        
        break;
    default:
        return CTC_STACK_PARAMETER_ERROR;
        break;
    }

    byte_count += byte_used;
        
    p_temp[1] = (char)(byte_count & 0xFF);
    p_temp[2] = (char)( (byte_count>>8) & 0xFF);

    p_temp[3] = first_block;

    *info_length = byte_count;
    
    return (CTC_STACK_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_redundancy_info"
PON_STATUS PON_CTC_STACK_set_redundancy_info (
                                    CTC_STACK_rdn_info_type_t info_type,
                                    const PON_olt_id_t   olt_id,
                                    char*   info_data,  
                                    int     info_length)
{
    char *p_temp;
    short int byte_count;
    bool first_block;
    short int result;
    
    if((info_data==NULL) || (olt_id>PON_OLT_ID_ARRAY_SIZE))
    {
        /**/
        return CTC_STACK_PARAMETER_ERROR;
    }

    p_temp = info_data;
    
    /*data block:  info_type(1B)+info_length(2B)+first_block(1B)+PAYLOAD(N)*/
    if(p_temp[0]!=info_type)
    {
        /*Data error, block may be corrupted*/
        PONLOG_ERROR_2( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                            "The info type %d is different from which in the data block %d\n", info_type, p_temp[0]);
        return CTC_STACK_EXIT_ERROR;
    }

    byte_count = (((short int)p_temp[1])&0xFF )+ ((((short int)p_temp[2])&0xFF)<<8);
    first_block = (bool)p_temp[3];

    switch(info_type)
    {
    case CTC_STACK_PON_REDUNDANCY_DISCOVERY_INFO:                
        result = disperse_rdn_data_discovery(olt_id, &p_temp[4], (info_length-4), (byte_count-4), first_block);
        if(result != CTC_STACK_EXIT_OK)
            return (result);
                
        break;
    case CTC_STACK_PON_REDUNDANCY_ENCRYPTION_DB:
        result = disperse_rdn_data_encryption(olt_id, &p_temp[4], (info_length-4), (byte_count-4), first_block);
        if(result != CTC_STACK_EXIT_OK)
            return (result);

        break;
    case CTC_STACK_PON_REDUNDANCY_AUTHENTICATION_DB:
        result = disperse_rdn_data_authentication(olt_id, &p_temp[4], (info_length-4), (byte_count-4), first_block);
        if(result != CTC_STACK_EXIT_OK)
            return (result);

        break;
    default:
        return CTC_STACK_PARAMETER_ERROR;
        break;
    }

    return (CTC_STACK_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_synchronize_encryption_db_to_olt"
PON_STATUS PON_CTC_STACK_synchronize_encryption_db_to_olt(const PON_olt_id_t  olt_id, const PON_onu_id_t  onu_id)
{
    short int result = CTC_STACK_EXIT_OK;
    bool     encryption_active;
    
    if(encryption_db.olt[olt_id].encryption_type == PON_ENCRYPTION_TYPE_TRIPLE_CHURNING)
    {
		if(!Onu_exist(olt_id, onu_id))
                return CTC_STACK_EXIT_OK;
            
        if(encryption_db.olt[olt_id].onu_id[onu_id].encryption_state == CTC_ENCRYPTION_STATE_NOT_ENCRYPTED)
        {
            /*stop enc if enc active*/
            result = Convert_ponsoft_error_code_to_ctc_stack(
                             Get_encryption_mode(olt_id, onu_id, &encryption_active));
            
            if ((result == CTC_STACK_EXIT_OK)&&(encryption_active))
            {
                /*stop encryption*/
                CTC_STACK_stop_encryption(olt_id, onu_id);
            }
        }
        else
        {
            if( encryption_db.olt[olt_id].onu_id[onu_id].start_encryption_by_the_user)
            {
				if (encryption_db.olt[olt_id].onu_id[onu_id].response_encryption_state == CTC_STACK_STATE_RECEIVED_FRAME)
				{
					encryption_db.olt[olt_id].onu_id[onu_id].key_index = encryption_db.olt[olt_id].onu_id[onu_id].temp_key_index; 
				}
                /*key_index should be equal to temp_key_index*/
                result = Encryption_sequence_to_olt(olt_id, onu_id, FALSE);

				 /*The first time set to OLT FW is always 0 in FW. We have to set again to make FW have same index*/
                if(encryption_db.olt[olt_id].onu_id[onu_id].key_index == 1)
                    result = Encryption_sequence_to_olt(olt_id, onu_id, FALSE);

				Set_encryption_db_flag_entry(olt_id, onu_id, CTC_STACK_STATE_VALID); 
				result = Set_encryption_db_entry  ( olt_id, onu_id,CTC_ENCRYPTION_STATE_ENCRYPTION_COMPLETE/*encryption_state*/);
            }
            else
            {
                PONLOG_INFO_2( PONLOG_FUNC_ENCRYPTION_FLOW, olt_id, onu_id, "low performance in OLT:%d ONU:%d\n", 
                               olt_id, onu_id ); 
            }
        }
    }

    return (result);                       
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_update_redundancy_entire_info"
PON_STATUS PON_CTC_STACK_update_redundancy_entire_info  (
                                    const PON_olt_id_t   master_olt,
                                    const PON_olt_id_t   slave_olt,
                                    CTC_STACK_rdn_info_type_t info_type)
{
    short int result=CTC_STACK_EXIT_OK;

    /* B--added by liwei056@2011-4-22 for SyncBetweenPasSoft */
    PONLOG_ERROR_0(PONLOG_FUNCTION_NAME, master_olt, slave_olt, "CTC_STACK_update_redundancy_entire_info() Error to enter!");
    /* E--added by liwei056@2011-4-22 for SyncBetweenPasSoft */

    switch(info_type)
    {
    case CTC_STACK_PON_REDUNDANCY_DISCOVERY_INFO:                
        memcpy((void *)&discovery_db.state[slave_olt][0], 
                        (void *)&discovery_db.state[master_olt][0], 
                        sizeof(CTC_STACK_discovery_state_t)*PON_ONU_ID_PER_OLT_ARRAY_SIZE);

        memcpy((void *)&discovery_db.maximum_data_frame_size[slave_olt][0], 
                        (void *)&discovery_db.maximum_data_frame_size[master_olt][0], 
                        sizeof(unsigned short)*PON_ONU_ID_PER_OLT_ARRAY_SIZE);

        memcpy((void *)&discovery_db.common_version[slave_olt][0], 
                        (void *)&discovery_db.common_version[master_olt][0], 
                        sizeof(unsigned char)*PON_ONU_ID_PER_OLT_ARRAY_SIZE);
        break;
    case CTC_STACK_PON_REDUNDANCY_ENCRYPTION_DB:
        /* B--modified by liwei056@2011-4-22 for SemCopyBug */
#if 0
        memcpy((void *)&encryption_db.olt[slave_olt],
                       (void *)&encryption_db.olt[master_olt], 
                       sizeof(encryption_olt_database_t));
#else
        memcpy((void *)&encryption_db.olt[slave_olt],
                       (void *)&encryption_db.olt[master_olt], 
                       sizeof(encryption_olt_database_t) - sizeof(OSSRV_semaphore_t));
#endif
        /* E--modified by liwei056@2011-4-22 for SemCopyBug */
        break;
    case CTC_STACK_PON_REDUNDANCY_AUTHENTICATION_DB:
#ifdef CTC_AUTH_SUPPORT
        memcpy((void *)&authentication_db.olt[slave_olt], 
                        (void *)&authentication_db.olt[master_olt],
                        sizeof(authentication_olt_database_t));
#endif
        break;
    default:
        result = CTC_STACK_PARAMETER_ERROR;
        break;
    }

    return (result);
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_voip_fax_config"
PON_STATUS PON_CTC_STACK_get_voip_fax_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_voip_fax_config_t    	           *voip_fax)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_VOIP_FAX_CONFIG;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN


	comparser_result = (short int)CTC_PARSE_voip_fax_config
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              voip_fax, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_voip_iad_oper_status"
PON_STATUS PON_CTC_STACK_get_voip_iad_oper_status( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_voip_iad_oper_status_t	           *iad_oper_status)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char								expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf	=  CTC_EX_VAR_VOIP_IAD_OP_STATUS;
	unsigned char								received_num_of_entries=0;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
       
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	comparser_result = (short int)CTC_COMPOSE_general_request
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN
	

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN


	comparser_result = (short int)CTC_PARSE_voip_iad_oper_status
                            ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
                              iad_oper_status, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_voip_iad_operation"
PON_STATUS PON_CTC_STACK_voip_iad_operation( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_operation_type_t                operation_type)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ACTION;
	unsigned short								expected_leaf   =  CTC_EX_ACTION_VOIP_IAD_OPERATION;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_voip_iad_operation
							( 1, operation_type, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
	
	return result;    
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "set_sip_digit_map"
static short int set_sip_digit_map(const PON_olt_id_t							olt_id, 
									const PON_onu_id_t							onu_id, 
									const CTC_STACK_SIP_digit_map_t             sip_digit_map,
									const unsigned char						    block_offset,
									const unsigned char                         block_in_oam,
									const unsigned char							total_block) 
{
   	short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ACTION;
	unsigned short								expected_leaf   =  CTC_EX_ACTION_SIP_DIGIT_MAP_CONFIG;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char								*sip_digit_map_ptr = NULL;
    unsigned char								num_of_entries = 0;
	unsigned char 								block_index = 0;

	comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
   	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

    num_of_entries = CTC_STACK_SIP_DIGIT_MAP_BLOCK_SIZE;
   	for (block_index = 0; (block_index < block_in_oam) && ((block_index + block_offset) < total_block); block_index ++)
	{	
		sip_digit_map_ptr = sip_digit_map.digital_map + (block_offset + block_index )* CTC_STACK_SIP_DIGIT_MAP_BLOCK_SIZE;
    	if ((((unsigned long)(block_offset + block_index + 1 )) * CTC_STACK_SIP_DIGIT_MAP_BLOCK_SIZE) > (sip_digit_map.digital_map_length))
		{
			num_of_entries = (unsigned char)(sip_digit_map.digital_map_length - (block_offset + block_index) * CTC_STACK_SIP_DIGIT_MAP_BLOCK_SIZE);
		}

		comparser_result = (short int)CTC_COMPOSE_sip_digit_map
							( num_of_entries, sip_digit_map_ptr, total_block, (unsigned char)(block_index + block_offset) , COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/, SEND_COMPARSER_BUFFER, &sent_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
		UPDATE_SENT_LEN
	}

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	for (block_index; block_index > 0; block_index --)
	{
		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
		UPDATE_RECEIVED_LEN
	}
	return result;    
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_sip_digit_map"
PON_STATUS PON_CTC_STACK_set_sip_digit_map( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_SIP_digit_map_t           sip_digit_map)
{
	short int									result=CTC_STACK_EXIT_OK;
	unsigned char 								block_in_oam = (discovery_db.maximum_data_frame_size[olt_id][onu_id] - OAM_OUI_SIZE - ETHERNET_FRAME_HEADER_SIZE - 1) / CTC_STACK_SIP_DIGIT_MAP_TLV_SIZE;
	unsigned long 								block_offset = 0;
	unsigned long 								total_block = sip_digit_map.digital_map_length/CTC_STACK_SIP_DIGIT_MAP_BLOCK_SIZE ;

	if (((sip_digit_map.digital_map_length%CTC_STACK_SIP_DIGIT_MAP_BLOCK_SIZE) != 0) || total_block == 0)
	{
		total_block++;
	}

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
  
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
	
	if (total_block > CTC_STACK_SIP_DIGIT_MAP_MAX_BLOCK)
	{
		PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, "The digital map total block %d exceed the max block %d. \n", total_block,  CTC_STACK_SIP_DIGIT_MAP_MAX_BLOCK);
		return CTC_STACK_EXIT_ERROR;
	}
	else
	{	
		while (block_offset < total_block)
		{
			result = set_sip_digit_map(olt_id, onu_id, sip_digit_map, (unsigned char)block_offset, block_in_oam,  (unsigned char)total_block);
			CHECK_CTC_STACK_RESULT_AND_RETURN
			
			block_offset += block_in_oam;
		}
	}
	
	return result;    
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_onu_capabilities_2"
PON_STATUS PON_CTC_STACK_get_onu_capabilities_2 ( 
           const PON_olt_id_t			  olt_id, 
           const PON_onu_id_t			  onu_id,
		   CTC_STACK_onu_capabilities_2_t  *onu_capabilities )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_ONU_CAPABILITIES_2;
	unsigned char							ctc_ret_val;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							received_num_of_entries=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
    CTC_management_object_port_type_t       port_type   = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
   
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)CTC_PARSE_onu_capabilities_2
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          onu_capabilities, &ctc_ret_val, &received_left_length  );

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;

    return result;    
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_onu_capabilities_3"
PON_STATUS PON_CTC_STACK_get_onu_capabilities_3 ( 
           const PON_olt_id_t			  olt_id, 
           const PON_onu_id_t			  onu_id,
		   CTC_STACK_onu_capabilities_3_t  *onu_capabilities )
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_ONU_CAPABILITIES_3;
	unsigned char							ctc_ret_val;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							received_num_of_entries=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
    CTC_management_object_port_type_t       port_type   = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
   
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)CTC_PARSE_onu_capabilities_3
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          onu_capabilities, &ctc_ret_val, &received_left_length  );

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;

	/*update the onu capabilities-3 db*/
	onu_capabilities_3_db.onu[olt_id][onu_id].onu_ipv6_aware = onu_capabilities->onu_ipv6_aware;
	onu_capabilities_3_db.onu[olt_id][onu_id].onu_power_supply_control= onu_capabilities->onu_power_supply_control;	

    return result;    
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_mxu_mng_global_parameter_config"
PON_STATUS PON_CTC_STACK_set_mxu_mng_global_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_mxu_mng_global_parameter_config_t  parameter)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_MXU_MNG_GLOBAL_PARAMETER;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t	ipv6_param;
    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	memset(&ipv6_param, 0, sizeof(CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t));
	memcpy(&(ipv6_param.ipv4_param), &parameter, sizeof(CTC_STACK_mxu_mng_global_parameter_config_t));
	ipv6_param.ip_flag = CTC_STACK_IPV4;

	comparser_result = (short int)CTC_COMPOSE_mxu_mng_global_parameter_config
							( 1, ipv6_param, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
    return result;  
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_mxu_mng_global_parameter_config_ipv6_ext "
PON_STATUS PON_CTC_STACK_set_mxu_mng_global_parameter_config_ipv6_ext ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t  parameter)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_MXU_MNG_GLOBAL_PARAMETER;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_mxu_mng_global_parameter_config
							( 1, parameter, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
    return result;  
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_mxu_mng_global_parameter_config"
PON_STATUS PON_CTC_STACK_get_mxu_mng_global_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
		    CTC_STACK_mxu_mng_global_parameter_config_t*  parameter)
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_MXU_MNG_GLOBAL_PARAMETER;
	unsigned char							ctc_ret_val;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							received_num_of_entries=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

	CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t ipv6_param;
		
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
   
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)CTC_PARSE_mxu_mng_global_parameter_config
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &ipv6_param, &ctc_ret_val, &received_left_length  );
	memcpy(parameter, &(ipv6_param.ipv4_param), sizeof(CTC_STACK_mxu_mng_global_parameter_config_t));

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;
    return result;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_mxu_mng_global_parameter_config_ipv6_ext"
PON_STATUS PON_CTC_STACK_get_mxu_mng_global_parameter_config_ipv6_ext ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
		    CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t*  parameter)
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_MXU_MNG_GLOBAL_PARAMETER;
	unsigned char							ctc_ret_val;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							received_num_of_entries=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
   
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)CTC_PARSE_mxu_mng_global_parameter_config
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          parameter, &ctc_ret_val, &received_left_length  );

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;
    return result;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_mxu_mng_snmp_parameter_config"
PON_STATUS PON_CTC_STACK_set_mxu_mng_snmp_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_mxu_mng_snmp_parameter_config_t  parameter)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_MXU_MNG_SNMP_PARAMETER;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t ipv6_param;
    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN

	memset(&ipv6_param, 0, sizeof(CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t));
	memcpy(&(ipv6_param.ipv4_param), &parameter, sizeof(CTC_STACK_mxu_mng_snmp_parameter_config_t));
	ipv6_param.ip_flag = CTC_STACK_IPV4;

	comparser_result = (short int)CTC_COMPOSE_mxu_mng_snmp_parameter_config
							( 1, ipv6_param, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
    return result;  
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_mxu_mng_snmp_parameter_config_ipv6_ext"
PON_STATUS PON_CTC_STACK_set_mxu_mng_snmp_parameter_config_ipv6_ext ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t  parameter)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_MXU_MNG_SNMP_PARAMETER;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_mxu_mng_snmp_parameter_config
							( 1, parameter, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
    return result;  
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_mxu_mng_snmp_parameter_config"
PON_STATUS PON_CTC_STACK_get_mxu_mng_snmp_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
		    CTC_STACK_mxu_mng_snmp_parameter_config_t*  parameter)
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_MXU_MNG_SNMP_PARAMETER;
	unsigned char							ctc_ret_val;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							received_num_of_entries=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

	CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t  ipv6_param;
		
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
   
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)CTC_PARSE_mxu_mng_snmp_parameter_config
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          &ipv6_param, &ctc_ret_val, &received_left_length  );
	memcpy(parameter, &(ipv6_param.ipv4_param), sizeof(CTC_STACK_mxu_mng_snmp_parameter_config_t));

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;
    return result;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_mxu_mng_snmp_parameter_config_ipv6_ext"
PON_STATUS PON_CTC_STACK_get_mxu_mng_snmp_parameter_config_ipv6_ext ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
		    CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t*  parameter)
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_MXU_MNG_SNMP_PARAMETER;
	unsigned char							ctc_ret_val;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							received_num_of_entries=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
   
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)CTC_PARSE_mxu_mng_snmp_parameter_config
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          parameter, &ctc_ret_val, &received_left_length  );

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;
    return result;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_multiple_management_object_port_loop_detect"
PON_STATUS PON_CTC_STACK_set_multiple_management_object_port_loop_detect ( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   const unsigned char					               number_of_entries,
		   const CTC_STACK_management_object_port_loop_detect_t  management_object_loop_detect )
{
    short int								result,comparser_result;
    unsigned short							received_left_length = 0, sent_length=ETHERNET_MTU, received_length;
	unsigned char							send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char							expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf  =  CTC_EX_VAR_PORT_LOOP_DETECT;
	unsigned char							received_num_of_entries=number_of_entries;
	unsigned char							ctc_ret_val,i;
	unsigned short							max_length=ETHERNET_MTU,total_length=0,received_total_length=0;
	unsigned char							local_number_of_entries=0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    if(number_of_entries != CTC_STACK_ALL_PORTS)
    {
        for(i = 0; i < number_of_entries; i++)
        {
            CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_loop_detect[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])
        }
    }

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if(number_of_entries != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < number_of_entries; i++)
		{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_loop_detect[i].management_object_index));
            
			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_port_loop_detect
									( 1, &management_object_loop_detect[i].loop_detect_enable, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
      		BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_loop_detect[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
								( sent_management_object, discovery_db.common_version[olt_id][onu_id],
								  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_port_loop_detect
									( 1, &management_object_loop_detect[0].loop_detect_enable, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;

	}


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        
	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;


	if(number_of_entries == CTC_STACK_ALL_PORTS)
	{
		local_number_of_entries = 1;
	}
	else
	{
		local_number_of_entries = number_of_entries;
	}


	for( i = 0; i < local_number_of_entries; i++)
	{
		comparser_result = (short int)CTC_PARSE_management_object
									( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
		CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)


		received_total_length += received_left_length;

		if(number_of_entries == CTC_STACK_ALL_PORTS)
		{
            if(discovery_db.common_version[olt_id][onu_id] == CTC_2_1_ONU_VERSION)
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_MANAGEMENT_OBJECT_ALL_PORTS)
            }
            else
            {
                CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, CTC_STACK_ALL_PORTS)
            }
		}
		else
		{
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_loop_detect[i].management_object_index.port_number)
		}


		comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );

		CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

		received_total_length += received_left_length;
		
	}
	
	return result;
        
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_multiple_management_object_port_loop_detect"
PON_STATUS PON_CTC_STACK_get_multiple_management_object_port_loop_detect ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char				             	 *number_of_entries,
		   CTC_STACK_management_object_port_loop_detect_t   management_object_loop_detect)
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_PORT_LOOP_DETECT;
	unsigned char							received_num_of_entries=0;
	unsigned char							ctc_ret_val,i;
	unsigned short							max_length=ETHERNET_MTU,received_max_length=ETHERNET_MTU,total_length=0, received_total_length=0;
	CTC_management_object_leaf_t			management_object_leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
	CTC_management_object_t					sent_management_object;
	CTC_management_object_t					received_management_object;


	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);
    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC


    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(PONLOG_FUNC_GENERAL, management_object_loop_detect[i].management_object_index.port_number, discovery_db.common_version[olt_id][onu_id])  
      		
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_loop_detect[i].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
            BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(PONLOG_FUNC_GENERAL, sent_management_object, management_object_leaf, (management_object_loop_detect[0].management_object_index));

			comparser_result = (short int)CTC_COMPOSE_management_object
									( sent_management_object, discovery_db.common_version[olt_id][onu_id],
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;


			comparser_result = (short int)CTC_COMPOSE_general_request
									( expected_branch, expected_leaf,
									  SEND_COMPARSER_BUFFER, &sent_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

			total_length += sent_length;
			sent_length = max_length - total_length;
	}

    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

	if( (*number_of_entries) != CTC_STACK_ALL_PORTS)
	{
		for( i = 0; i < (*number_of_entries); i++)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, TRUE/* error if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			CHECK_PORT_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, received_management_object.index.port_number, management_object_loop_detect[i].management_object_index.port_number)


			received_total_length += received_left_length;

			comparser_result = (short int)CTC_PARSE_port_loop_detect
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, /*TRUE,*/ &received_num_of_entries,
         									  &management_object_loop_detect[i].loop_detect_enable, &ctc_ret_val ,&received_left_length  );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;


			if(received_num_of_entries != 1)
			{
				
				PONLOG_ERROR_4( PONLOG_FUNC_GENERAL, olt_id, onu_id,
								"OLT %d ONU %d expected entries: %d. received entries %d\n",
								olt_id, onu_id, 1, received_num_of_entries);


				return CTC_STACK_MEMORY_ERROR;
			}
		}
	}
	else /* CTC_STACK_ALL_PORTS */
	{
		i = 0;

		while(received_total_length < received_length)
		{
			comparser_result = (short int)CTC_PARSE_management_object
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, FALSE/* ok if branch=0x0 */, discovery_db.common_version[olt_id][onu_id], &received_management_object, &received_left_length );

			if(comparser_result == COMPARSER_STATUS_END_OF_FRAME)
				break;
			CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
			

			received_total_length += received_left_length;

			received_num_of_entries = 0;

			comparser_result = (short int)CTC_PARSE_port_loop_detect
											( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, /*TRUE,*/ &received_num_of_entries,
         									  &management_object_loop_detect[i].loop_detect_enable, &ctc_ret_val ,&received_left_length   );

			CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

			received_total_length += received_left_length;


			management_object_loop_detect[i].management_object_index.port_number  = received_management_object.index.port_number;
			management_object_loop_detect[i].management_object_index.frame_number = received_management_object.index.frame_number;
			management_object_loop_detect[i].management_object_index.slot_number  = received_management_object.index.slot_number;
			management_object_loop_detect[i].management_object_index.port_type    = received_management_object.index.port_type;

			i++;
		}

		(*number_of_entries) = i;
	}
	return result;
        
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_all_management_object_port_loop_detect"
PON_STATUS PON_CTC_STACK_get_all_management_object_port_loop_detect ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char					             *number_of_entries,
		   CTC_STACK_management_object_port_loop_detect_t   management_object_info )
{
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    (*number_of_entries) = CTC_STACK_ALL_PORTS;
    return (PON_CTC_STACK_get_multiple_management_object_port_loop_detect(olt_id, onu_id, number_of_entries, management_object_info));
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_port_management_object_loop_detect"

PON_STATUS PON_CTC_STACK_set_management_object_port_loop_detect ( 
           const PON_olt_id_t	                    olt_id, 
           const PON_onu_id_t	                    onu_id,
		   const CTC_management_object_index_t		management_object_index,
		   const unsigned long			                    loop_detect)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_port_loop_detect_t		management_object_port_loop_detect;
	unsigned char							            number_of_entries;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	management_object_port_loop_detect[0].management_object_index.port_number  = management_object_index.port_number;
	management_object_port_loop_detect[0].management_object_index.slot_number  = management_object_index.slot_number;
 	management_object_port_loop_detect[0].management_object_index.frame_number = management_object_index.frame_number;
	management_object_port_loop_detect[0].management_object_index.port_type    = management_object_index.port_type;

	management_object_port_loop_detect[0].loop_detect_enable = loop_detect;

	if(management_object_index.port_number == CTC_MANAGEMENT_OBJECT_ALL_PORTS)
		number_of_entries = CTC_STACK_ALL_PORTS;
	else
		number_of_entries = 1;

	return PON_CTC_STACK_set_multiple_management_object_port_loop_detect(olt_id, onu_id, number_of_entries, management_object_port_loop_detect);    
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_port_management_object_loop_detect"

PON_STATUS PON_CTC_STACK_get_port_management_object_loop_detect ( 
           const PON_olt_id_t	                    olt_id, 
           const PON_onu_id_t	                    onu_id,
		   const CTC_management_object_index_t		management_object_index,
		   unsigned long				                       *loop_detect)
{
    short int								            result=CTC_STACK_EXIT_OK;
	CTC_STACK_management_object_port_loop_detect_t		management_object_port_loop_detect;
	unsigned char							            number_of_entries = 1;

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

	management_object_port_loop_detect[0].management_object_index.port_number  = management_object_index.port_number;
	management_object_port_loop_detect[0].management_object_index.slot_number  = management_object_index.slot_number;
 	management_object_port_loop_detect[0].management_object_index.frame_number = management_object_index.frame_number;
	management_object_port_loop_detect[0].management_object_index.port_type    = management_object_index.port_type;


	result = PON_CTC_STACK_get_multiple_management_object_port_loop_detect(olt_id, onu_id, &number_of_entries, management_object_port_loop_detect);
	CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

	(*loop_detect) = management_object_port_loop_detect[0].loop_detect_enable;

	return result;
        
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_holdover_state"
PON_STATUS PON_CTC_STACK_set_holdover_state ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_holdover_state_t  parameter)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_HOLDOVER_CONFIG;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_holdover_state
							( 1, parameter, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
    return result;  
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_holdover_state"
PON_STATUS PON_CTC_STACK_get_holdover_state ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
		    CTC_STACK_holdover_state_t*  parameter)
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_HOLDOVER_CONFIG;
	unsigned char							ctc_ret_val;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							received_num_of_entries=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)CTC_PARSE_holdover_state
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          parameter, &ctc_ret_val, &received_left_length  );

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;
    return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_onu_tx_power_supply_control"
PON_STATUS PON_CTC_STACK_onu_tx_power_supply_control ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id,
           const CTC_STACK_onu_tx_power_supply_control_t parameter,
           const bool               broadcast)
{
    short int                               result,comparser_result;
    unsigned short                          sent_length=ETHERNET_MTU;
    unsigned char                           sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char                           send_opcode     =  OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char                           expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
    unsigned short                          expected_leaf   =  CTC_EX_VAR_ONU_TX_POWER_SUPPLY_CONTROL;
    unsigned short                          max_length=ETHERNET_MTU,total_length=0;
    
    PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    if(broadcast == FALSE)
    {
        CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
        CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    }

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    total_length += sent_length;
    sent_length = max_length - total_length;

    comparser_result = (short int)CTC_COMPOSE_onu_tx_power_supply_control
                                  (1, parameter, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
                                   SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

    total_length += sent_length;
    sent_length = max_length - total_length;

    if(broadcast)
    {
        result = Send_receive_vendor_extension_message_with_timeout 
                            ( olt_id, PON_BROADCAST_LLID, sent_oam_pdu_data, total_length, 0/*without waiting*/, 
                              0/*expected_opcode- do not waiting for response*/, 0/*expected_code- do not waiting for response*/,
                              NULL/*received_oam_pdu_data*/, NULL/*&received_length*/, ETHERNET_MTU,NULL);
    }
    else
    {
        result = Send_receive_vendor_extension_message_with_timeout 
                            ( olt_id, onu_id, sent_oam_pdu_data, total_length, 0/*without waiting*/, 
                              0/*expected_opcode- do not waiting for response*/, 0/*expected_code- do not waiting for response*/,
                              NULL/*received_oam_pdu_data*/, NULL/*&received_length*/, ETHERNET_MTU,NULL);
    }
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC

    return result;
        
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_set_active_pon_if_state"
PON_STATUS PON_CTC_STACK_set_active_pon_if_state ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_active_pon_if_state_t  parameter)
{
    short int									result=CTC_STACK_EXIT_OK,comparser_result;
    unsigned short								received_left_length=0,sent_length=ETHERNET_MTU, received_length;
	unsigned char								send_opcode = OAM_VENDOR_EXT_SET_REQUEST;
    unsigned char								sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								received_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char								expected_opcode =  OAM_VENDOR_EXT_SET_RESPONSE;
    unsigned char								expected_code   =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short								expected_leaf   =  CTC_EX_VAR_PON_IF_ADMIN_STATE;
	unsigned char								received_num_of_entries=1;
	unsigned char								ctc_ret_val;
	unsigned short								max_length=ETHERNET_MTU,total_length=0,received_total_length=0;

    
	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC

    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN
    
    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


	comparser_result = (short int)CTC_COMPOSE_active_pon_if_state
							( 1, parameter, COMPARSER_CTC_STATUS_SUCCESS, FALSE/* is_get_frame*/,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_SENT_LEN


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_code,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)
	UPDATE_RECEIVED_LEN

	
	comparser_result = (short int)CTC_PARSE_general_ack
						( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, expected_code,
    					  expected_leaf, &ctc_ret_val, &received_left_length  );
	CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)
	UPDATE_RECEIVED_LEN
    return result;  
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "PON_CTC_STACK_get_active_pon_if_state"
PON_STATUS PON_CTC_STACK_get_active_pon_if_state ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
		    CTC_STACK_active_pon_if_state_t*  parameter)
{
    short int								result,comparser_result;
    unsigned short							received_left_length=0,sent_length=ETHERNET_MTU, received_length;
    unsigned char							sent_oam_pdu_data[ETHERNET_MTU]= {0};
    unsigned char							received_oam_pdu_data[ETHERNET_MTU]= {0};
	unsigned char							send_opcode		=  OAM_VENDOR_EXT_GET_REQUEST;
    unsigned char							expected_opcode =  OAM_VENDOR_EXT_GET_RESPONSE;
    unsigned char							expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short							expected_leaf	=  CTC_EX_VAR_PON_IF_ADMIN_STATE;
	unsigned char							ctc_ret_val;
	PON_onu_id_t							onu_id_leaf = onu_id;
	unsigned char							received_num_of_entries=0;
	unsigned short							max_length=ETHERNET_MTU,total_length=0, received_total_length=0;

	PONLOG_INFO_API_CALLED_0(olt_id, onu_id);

    
    CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC
    CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN

    comparser_result = (short int)CTC_COMPOSE_opcode
                            ( send_opcode, SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)CTC_COMPOSE_general_request
                            ( expected_branch, expected_leaf,
                              SEND_COMPARSER_BUFFER, &sent_length );
    CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	total_length += sent_length;
	sent_length = max_length - total_length;


    result = Send_receive_vendor_extension_message 
                    ( olt_id, onu_id, sent_oam_pdu_data, total_length, expected_opcode, expected_branch,
                      received_oam_pdu_data, &received_length, ETHERNET_MTU,NULL);
    CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC
        

	comparser_result = (short int)CTC_PARSE_opcode
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER ,&received_left_length );
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result)

	received_total_length += received_left_length;

    comparser_result = (short int)CTC_PARSE_active_pon_if_state
                                    ( RECEIVED_COMPARSER_BUFFER, RECEIVED_LENGTH_BUFFER, &received_num_of_entries,
         	                          parameter, &ctc_ret_val, &received_left_length  );

    CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(PONLOG_FUNC_GENERAL, comparser_result, ctc_ret_val)

	received_total_length += received_left_length;
    return result;
}
#undef PONLOG_FUNCTION_NAME


/******************************** inner functions *********************************************************/

#if 0
const CTC_STACK_alarm_str_t *Get_alarm_str_array(void)
{
	return (alarm_str_array);
}

short int init_rdn_data_variables()
{
    short int i;

    for (i=0; i<PON_OLT_ID_ARRAY_SIZE; i++)
    {
        rdn_discovery_onu_id[i]=-1;
        rdn_encryption_onu_id[i]=-1;
        rdn_authentication_onu_id[i]=-1;
    }
    return CTC_STACK_EXIT_OK;
}

#define PONLOG_FUNCTION_NAME  "assemble_rdn_data_discovery"
short int assemble_rdn_data_discovery(
                        const PON_olt_id_t   olt_id,
                        const short int   max_onu_entry,
                        char*  info_data_buf,
                        int  info_buf_len,
                        int*  info_length,
                        bool*  first_block,
                        bool*  fragment)
{
    short int onu_id_idx;
    char * p_buf;
    short int byte_num;
    short int rdn_len_for_public = sizeof(ctc_stack_discovery_list_t)+sizeof(unsigned short)+1;
    short int rdn_len_for_onu = 1+sizeof(CTC_STACK_discovery_state_t)+sizeof(unsigned short)+sizeof(unsigned char);

    CTC_STACK_CHECK_OLT_VALIDITY
    
    if(rdn_discovery_onu_id[olt_id]<0)
    {
        /*this is the first block, check if the databuf enough for the public part of the discorvery information DB*/
        /*ctc_stack_discovery_list_t + timeout_in_100_ms(2B) + OLT_ID(1B)*/
        if(info_buf_len<rdn_len_for_public)
        {
            /*Error, buffer too small*/
            PONLOG_INFO_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                                    "The buffer is too small\n");
            return CTC_STACK_MEMORY_ERROR;
        }
    }
    else
    {
        /*this is not the first block, check if the databuf enough for a single ONU*/
        /*ONU_ID(1B) + state + maximum_data_frame_size + common_version*/
        if(info_buf_len<rdn_len_for_onu)
        {
            /*Error, buffer too small*/
            PONLOG_INFO_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                                    "The buffer is too small\n");
            return CTC_STACK_MEMORY_ERROR;
        }
    }

    p_buf = info_data_buf;
    byte_num = 0;
    *first_block = FALSE;
    for(onu_id_idx=rdn_discovery_onu_id[olt_id]; onu_id_idx<=max_onu_entry; onu_id_idx++)
    {

        if(onu_id_idx<0)
        {
            /*First block, assemble the public data*/
            /*ctc_stack_discovery_list_t          list_info;*/
            memcpy(p_buf, (char *)&(discovery_db.list_info), sizeof(discovery_db.list_info));
            p_buf += sizeof(discovery_db.list_info);
            byte_num += sizeof(discovery_db.list_info);

            /*unsigned short                      timeout_in_100_ms;*/
            memcpy(p_buf, (char *)&(discovery_db.timeout_in_100_ms), sizeof(discovery_db.timeout_in_100_ms));
            p_buf += sizeof(discovery_db.timeout_in_100_ms);
            byte_num += sizeof(discovery_db.timeout_in_100_ms);

            /*OLT_ID*/
            *p_buf = (char)olt_id;
            p_buf += 1;
            byte_num += 1;

            *first_block = TRUE;
        }
        else
        {
            /*Check for remaining buffer space*/
            if((info_buf_len-byte_num)<rdn_len_for_onu)
            {
                /*not enough for the remaining ONUs, will be fragmented*/
                break;
            }

            /*assemble the onu data*/
            /*ONU_ID*/
            *p_buf = (char)onu_id_idx;
            p_buf += 1;
            byte_num += 1;
            
            /*CTC_STACK_discovery_state_t         state[PON_OLT_ID_ARRAY_SIZE][PON_ONU_ID_PER_OLT_ARRAY_SIZE];*/
            memcpy(p_buf, (char *)&(discovery_db.state[olt_id][onu_id_idx]), sizeof(discovery_db.state[olt_id][onu_id_idx]));
            p_buf += sizeof(discovery_db.state[olt_id][onu_id_idx]);
            byte_num += sizeof(discovery_db.state[olt_id][onu_id_idx]);

            /*maximum_data_frame_size*/
            memcpy(p_buf, (char *)&(discovery_db.maximum_data_frame_size[olt_id][onu_id_idx]), sizeof(discovery_db.maximum_data_frame_size[olt_id][onu_id_idx]));
            p_buf += sizeof(discovery_db.maximum_data_frame_size[olt_id][onu_id_idx]);
            byte_num += sizeof(discovery_db.maximum_data_frame_size[olt_id][onu_id_idx]);

            /*common_version*/
            memcpy(p_buf, (char *)&(discovery_db.common_version[olt_id][onu_id_idx]), sizeof(discovery_db.common_version[olt_id][onu_id_idx]));
            p_buf += sizeof(discovery_db.common_version[olt_id][onu_id_idx]);
            byte_num += sizeof(discovery_db.common_version[olt_id][onu_id_idx]);
        }
    }

    if(onu_id_idx>max_onu_entry)
    {
        /*All ONUs are included, this is the final block*/
        *fragment=FALSE;
        rdn_discovery_onu_id[olt_id]=-1;
    }
    else
    {
        *fragment=TRUE;
        rdn_discovery_onu_id[olt_id]=onu_id_idx;
    }

    *info_length = byte_num;

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "disperse_rdn_data_discovery"
short int disperse_rdn_data_discovery(
                        const PON_olt_id_t      olt_id,
                        char*   info_data_buf,
                        int   info_buf_len,
                        int   info_length,
                        bool  first_block)
{
    short int onu_id_idx;
    char * p_buf;
    short int byte_num;
    short int rdn_len_for_public = sizeof(ctc_stack_discovery_list_t)+sizeof(unsigned short)+1;
    short int rdn_len_for_onu = 1+sizeof(CTC_STACK_discovery_state_t)+sizeof(unsigned short)+sizeof(unsigned char);

    p_buf = info_data_buf;
    byte_num = 0;

    CTC_STACK_CHECK_OLT_VALIDITY

    if(info_data_buf==NULL)
    {
        PONLOG_ERROR_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                            "The the pointer is NULL\n");
        return CTC_STACK_PARAMETER_ERROR;
    }

    if(first_block)
    {
        /*This is the first block, get the public data*/
        if(info_length < rdn_len_for_public)
        {
            /*Error, not enough data, block may be corrupted*/
            return CTC_STACK_EXIT_ERROR;
        }
        
        /*ctc_stack_discovery_list_t          list_info;*/
        memcpy((char *)&(discovery_db.list_info), p_buf, sizeof(discovery_db.list_info));
        p_buf += sizeof(discovery_db.list_info);
        byte_num += sizeof(discovery_db.list_info);

        /*unsigned short                      timeout_in_100_ms;*/
        memcpy((char *)&(discovery_db.timeout_in_100_ms), p_buf, sizeof(discovery_db.timeout_in_100_ms));
        p_buf += sizeof(discovery_db.timeout_in_100_ms);
        byte_num += sizeof(discovery_db.timeout_in_100_ms);

        /*OLT_ID*/
        p_buf += 1;
        byte_num += 1;
    }

    while( (info_length-byte_num) >= rdn_len_for_onu )
    {
        /*Data remaining, get ONU information*/
        onu_id_idx = *p_buf;
        p_buf += 1;
        byte_num += 1;

        if(onu_id_idx >= PON_ONU_ID_PER_OLT_ARRAY_SIZE)
        {
            /*Got wrong ONU ID, data block maybe corrupted*/
            break;
        }

        /*CTC_STACK_discovery_state_t         state[PON_OLT_ID_ARRAY_SIZE][PON_ONU_ID_PER_OLT_ARRAY_SIZE];*/
        memcpy((char *)&(discovery_db.state[olt_id][onu_id_idx]), p_buf, sizeof(discovery_db.state[olt_id][onu_id_idx]));
        p_buf += sizeof(discovery_db.state[olt_id][onu_id_idx]);
        byte_num += sizeof(discovery_db.state[olt_id][onu_id_idx]);

        /*maximum_data_frame_size*/
        memcpy((char *)&(discovery_db.maximum_data_frame_size[olt_id][onu_id_idx]), p_buf, sizeof(discovery_db.maximum_data_frame_size[olt_id][onu_id_idx]));
        p_buf += sizeof(discovery_db.maximum_data_frame_size[olt_id][onu_id_idx]);
        byte_num += sizeof(discovery_db.maximum_data_frame_size[olt_id][onu_id_idx]);

        /*common_version*/
        memcpy((char *)&(discovery_db.common_version[olt_id][onu_id_idx]), p_buf, sizeof(discovery_db.common_version[olt_id][onu_id_idx]));
        p_buf += sizeof(discovery_db.common_version[olt_id][onu_id_idx]);
        byte_num += sizeof(discovery_db.common_version[olt_id][onu_id_idx]);
    }

    if((info_length-byte_num) != 0)
    {
        /*data remaining, block may be corrupted*/
        PONLOG_ERROR_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                            "There's data remaining, block may be corrupted\n");
    }

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "assemble_rdn_data_authentication"
short int assemble_rdn_data_authentication(
                            const PON_olt_id_t  olt_id,
                            const short int  max_onu_entry,
                            char*   info_data_buf,
                            int   info_buf_len,
                            int*   info_length,
                            bool*  first_block,
                            bool*  fragment)
{
    short int onu_id_idx;
    char * p_buf;
    short int byte_num;
    short int rdn_len_for_public = sizeof(CTC_STACK_auth_mode_t)+1;
    short int rdn_len_for_onu = 1+sizeof(PON_onu_id_t)+sizeof(CTC_STACK_auth_loid_data_t);

    CTC_STACK_CHECK_OLT_VALIDITY
    
    if(rdn_authentication_onu_id[olt_id]<0)
    {
        /*this is the first block, check if the databuf enough for the public part of the discorvery information DB*/
        /*auth_mode + OLT_ID(1B)*/
        if(info_buf_len<rdn_len_for_public)
        {
            /*Error, buffer too small*/
            PONLOG_INFO_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                                    "The buffer is too small\n");
            return CTC_STACK_MEMORY_ERROR;
        }
    }
    else
    {
        /*this is not the first block, check if the databuf enough for a single ONU*/
        /*ONU_ID(1B) + onu_llid + CTC_STACK_auth_loid_data_t*/
        if(info_buf_len<rdn_len_for_onu)
        {
            /*Error, buffer too small*/
            PONLOG_INFO_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                                    "The buffer is too small\n");
            return CTC_STACK_MEMORY_ERROR;
        }
    }

    p_buf = info_data_buf;
    byte_num = 0;
    *first_block = FALSE;
    /*We have to sync the entire LOID table, because they are not indexed by ONU id*/
    for(onu_id_idx=rdn_authentication_onu_id[olt_id]; onu_id_idx<PON_ONU_ID_PER_OLT_ARRAY_SIZE; onu_id_idx++)
    {

        if(onu_id_idx<0)
        {
            /*First block, assemble the public data*/
            /*CTC_STACK_auth_mode_t            auth_mode*/
            memcpy(p_buf, (char *)&(authentication_db.olt[olt_id].auth_mode), sizeof(authentication_db.olt[olt_id].auth_mode));
            p_buf += sizeof(authentication_db.olt[olt_id].auth_mode);
            byte_num += sizeof(authentication_db.olt[olt_id].auth_mode);

            /*OLT_ID*/
            *p_buf = (char)olt_id;
            p_buf += 1;
            byte_num += 1;

            *first_block = TRUE;
        }
        else
        {
            /*Don't need to sync the invalid entries.*/
            if(memcmp(CTC_AUTH_LOID_DATA_NOT_USED, authentication_db.olt[olt_id].loid[onu_id_idx].onu_id, sizeof(CTC_AUTH_LOID_DATA_NOT_USED)) == 0)
                continue;
        
            /*Check for remaining buffer space*/
            if((info_buf_len-byte_num)<rdn_len_for_onu)
            {
                /*not enough for the remaining ONUs, will be fragmented*/
                break;
            }

            /*assemble the onu data*/
            /*ONU_ID*/
            *p_buf = (char)onu_id_idx;
            p_buf += 1;
            byte_num += 1;
            
            /*CTC_STACK_auth_loid_data_t       loid[PON_ONU_ID_PER_OLT_ARRAY_SIZE]*/
            memcpy(p_buf, (char *)&(authentication_db.olt[olt_id].loid[onu_id_idx]), sizeof(authentication_db.olt[olt_id].loid[onu_id_idx]));
            p_buf += sizeof(authentication_db.olt[olt_id].loid[onu_id_idx]);
            byte_num += sizeof(authentication_db.olt[olt_id].loid[onu_id_idx]);

            /*PON_onu_id_t                     onu_llid[PON_ONU_ID_PER_OLT_ARRAY_SIZE];*/
            memcpy(p_buf, (char *)&(authentication_db.olt[olt_id].onu_llid[onu_id_idx]), sizeof(authentication_db.olt[olt_id].onu_llid[onu_id_idx]));
            p_buf += sizeof(authentication_db.olt[olt_id].onu_llid[onu_id_idx]);
            byte_num += sizeof(authentication_db.olt[olt_id].onu_llid[onu_id_idx]);            
        }
    }

    if(onu_id_idx>=PON_ONU_ID_PER_OLT_ARRAY_SIZE)
    {
        /*All ONUs are included, this is the final block*/
        *fragment=FALSE;
        rdn_authentication_onu_id[olt_id]=-1;
    }
    else
    {
        *fragment=TRUE;
        rdn_authentication_onu_id[olt_id]=onu_id_idx;
    }

    *info_length = byte_num;

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "disperse_rdn_data_authentication"
short int disperse_rdn_data_authentication(
                                const PON_olt_id_t  olt_id,
                                char*  info_data_buf,
                                int  info_buf_len,
                                int  info_length,
                                bool  first_block)
{
    short int onu_id_idx;
    char * p_buf;
    short int byte_num;
    short int rdn_len_for_public = sizeof(CTC_STACK_auth_mode_t)+1;
    short int rdn_len_for_onu = 1+sizeof(PON_onu_id_t)+sizeof(CTC_STACK_auth_loid_data_t);
    
    p_buf = info_data_buf;
    byte_num = 0;

    CTC_STACK_CHECK_OLT_VALIDITY

    if(info_data_buf==NULL)
    {
        PONLOG_ERROR_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                            "The the pointer is NULL\n");
        return CTC_STACK_PARAMETER_ERROR;
    }

    if(first_block)
    {
        /*This is the first block, get the public data*/
        if(info_length < rdn_len_for_public)
        {
            PONLOG_ERROR_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                                "The first block don't have enough length for public info\n");
        
            /*Error, not enough data, block may be corrupted*/
            return CTC_STACK_EXIT_ERROR;
        }
        
       /*CTC_STACK_auth_mode_t            auth_mode*/
        memcpy((char *)&(authentication_db.olt[olt_id].auth_mode), p_buf, sizeof(authentication_db.olt[olt_id].auth_mode));
        p_buf += sizeof(authentication_db.olt[olt_id].auth_mode);
        byte_num += sizeof(authentication_db.olt[olt_id].auth_mode);

        /*OLT_ID*/
        p_buf += 1;
        byte_num += 1;
    }

    while( (info_length-byte_num) >= rdn_len_for_onu )
    {
        /*Data remaining, get ONU information*/
        onu_id_idx = *p_buf;
        p_buf += 1;
        byte_num += 1;

        if(onu_id_idx >= PON_ONU_ID_PER_OLT_ARRAY_SIZE)
        {
            /*Got wrong ONU ID, data block maybe corrupted*/
            break;
        }

        /*CTC_STACK_auth_loid_data_t       loid[PON_ONU_ID_PER_OLT_ARRAY_SIZE]*/
        memcpy((char *)&(authentication_db.olt[olt_id].loid[onu_id_idx]), p_buf, sizeof(authentication_db.olt[olt_id].loid[onu_id_idx]));
        p_buf += sizeof(authentication_db.olt[olt_id].loid[onu_id_idx]);
        byte_num += sizeof(authentication_db.olt[olt_id].loid[onu_id_idx]);

        /*PON_onu_id_t                     onu_llid[PON_ONU_ID_PER_OLT_ARRAY_SIZE];*/
        memcpy((char *)&(authentication_db.olt[olt_id].onu_llid[onu_id_idx]), p_buf, sizeof(authentication_db.olt[olt_id].onu_llid[onu_id_idx]));
        p_buf += sizeof(authentication_db.olt[olt_id].onu_llid[onu_id_idx]);
        byte_num += sizeof(authentication_db.olt[olt_id].onu_llid[onu_id_idx]);  
    }

    if((info_length-byte_num) != 0)
    {
        /*data remaining, block may be corrupted*/
        PONLOG_ERROR_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                            "There's data remaining, block may be corrupted\n");
    }

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "assemble_rdn_data_encryption"
short int assemble_rdn_data_encryption(
                            const PON_olt_id_t   olt_id,
                            const short int   max_onu_entry,
                            char*  info_data_buf,
                            int   info_buf_len,
                            int*  info_length,
                            bool*  first_block,
                            bool*  fragment)
{
    short int onu_id_idx;
    char * p_buf;
    short int byte_num;
    short int rdn_len_for_public = sizeof(ctc_stack_encryption_information_t)+sizeof(PON_encryption_key_index_t)+1;
    short int rdn_len_for_onu = 1+sizeof(CTC_STACK_encryption_onu_database_t);

    CTC_STACK_CHECK_OLT_VALIDITY
    
    if(rdn_encryption_onu_id[olt_id]<0)
    {
        /*this is the first block, check if the databuf enough for the public part of the discorvery information DB*/
        /*timing_info + PON_encryption_key_index_t + OLT_ID(1B)*/
        if(info_buf_len<rdn_len_for_public)
        {
            /*Error, buffer too small*/
            PONLOG_INFO_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                                    "The buffer is too small\n");
            return CTC_STACK_MEMORY_ERROR;
        }
    }
    else
    {
        /*this is not the first block, check if the databuf enough for a single ONU*/
        /*ONU_ID(1B) + onu_id[]*/
        if(info_buf_len<rdn_len_for_onu)
        {
            /*Error, buffer too small*/
            PONLOG_ERROR_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                                    "The buffer is too small\n");
            return CTC_STACK_PARAMETER_ERROR;
        }
    }

    p_buf = info_data_buf;
    byte_num = 0;
    *first_block = FALSE;
    for(onu_id_idx=rdn_encryption_onu_id[olt_id]; onu_id_idx<=max_onu_entry; onu_id_idx++)
    {

        if(onu_id_idx<0)
        {
            /*First block, assemble the public data*/
            /*ctc_stack_encryption_information_t      timing_info;*/
            memcpy(p_buf, (char *)&(encryption_db.timing_info), sizeof(encryption_db.timing_info));
            p_buf += sizeof(encryption_db.timing_info);
            byte_num += sizeof(encryption_db.timing_info);

            /*PON_encryption_key_index_t      encryption_type;*/
            memcpy(p_buf, (char *)&(encryption_db.olt[olt_id].encryption_type), sizeof(encryption_db.olt[olt_id].encryption_type));
            p_buf += sizeof(encryption_db.olt[olt_id].encryption_type);
            byte_num += sizeof(encryption_db.olt[olt_id].encryption_type);

            /*OSSRV_semaphore_t               semaphore;*/
            /*Note: we don't pack this information since it is meaningless to 
            exchange the semaphore between PAS-SOFT*/

            /*OLT_ID*/
            *p_buf = (char)olt_id;
            p_buf += 1;
            byte_num += 1;

            *first_block = TRUE;
        }
        else
        {
            /*Check for remaining buffer space*/
            if((info_buf_len-byte_num)<rdn_len_for_onu)
            {
                /*not enough for the remaining ONUs, will be fragmented*/
                break;
            }

            /*assemble the onu data*/
            /*ONU_ID*/
            *p_buf = (char)onu_id_idx;
            p_buf += 1;
            byte_num += 1;
            
            /*encryption_onu_database_t       onu_id[PON_ONU_ID_PER_OLT_ARRAY_SIZE]*/
            memcpy(p_buf, (char *)&(encryption_db.olt[olt_id].onu_id[onu_id_idx]), sizeof(encryption_db.olt[olt_id].onu_id[onu_id_idx]));
            p_buf += sizeof(encryption_db.olt[olt_id].onu_id[onu_id_idx]);
            byte_num += sizeof(encryption_db.olt[olt_id].onu_id[onu_id_idx]);            
        }
    }

    if(onu_id_idx>max_onu_entry)
    {
        /*All ONUs are included, this is the final block*/
        *fragment=FALSE;
        rdn_encryption_onu_id[olt_id]=-1;
    }
    else
    {
        *fragment=TRUE;
        rdn_encryption_onu_id[olt_id]=onu_id_idx;
    }

    *info_length = byte_num;

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME  "disperse_rdn_data_encryption"
short int disperse_rdn_data_encryption(
                            const PON_olt_id_t    olt_id,
                            char*   info_data_buf,
                            int  info_buf_len,
                            int  info_length,
                            bool  first_block)
{
    short int onu_id_idx;
    char * p_buf;
    short int byte_num;
    short int rdn_len_for_public = sizeof(ctc_stack_encryption_information_t)+sizeof(PON_encryption_key_index_t)+1;
    short int rdn_len_for_onu = 1+sizeof(CTC_STACK_encryption_onu_database_t);
    
    p_buf = info_data_buf;
    byte_num = 0;

    CTC_STACK_CHECK_OLT_VALIDITY

    if(info_data_buf==NULL)
    {
        PONLOG_ERROR_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                            "The the pointer is NULL\n");
        return CTC_STACK_PARAMETER_ERROR;
    }

    if(first_block)
    {
        /*This is the first block, get the public data*/
        if(info_length < rdn_len_for_public)
        {
            /*Error, not enough data, block may be corrupted*/
            return CTC_STACK_EXIT_ERROR;
        }
        
        /*ctc_stack_encryption_information_t      timing_info;*/
        memcpy((char *)&(encryption_db.timing_info), p_buf, sizeof(encryption_db.timing_info));
        p_buf += sizeof(encryption_db.timing_info);
        byte_num += sizeof(encryption_db.timing_info);

        /*PON_encryption_key_index_t      encryption_type;*/
        memcpy((char *)&(encryption_db.olt[olt_id].encryption_type), p_buf, sizeof(encryption_db.olt[olt_id].encryption_type));
        p_buf += sizeof(encryption_db.olt[olt_id].encryption_type);
        byte_num += sizeof(encryption_db.olt[olt_id].encryption_type);

        /*OLT_ID*/
        p_buf += 1;
        byte_num += 1;
    }

    while( (info_length-byte_num) >= rdn_len_for_onu )
    {
        /*Data remaining, get ONU information*/
        onu_id_idx = *p_buf;
        p_buf += 1;
        byte_num += 1;

        if(onu_id_idx >= PON_ONU_ID_PER_OLT_ARRAY_SIZE)
        {
            /*Got wrong ONU ID, data block maybe corrupted*/
            break;
        }

        /*encryption_onu_database_t       onu_id[PON_ONU_ID_PER_OLT_ARRAY_SIZE]*/
            memcpy((char *)&(encryption_db.olt[olt_id].onu_id[onu_id_idx]), p_buf, sizeof(encryption_db.olt[olt_id].onu_id[onu_id_idx]));
            p_buf += sizeof(encryption_db.olt[olt_id].onu_id[onu_id_idx]);
            byte_num += sizeof(encryption_db.olt[olt_id].onu_id[onu_id_idx]);
         CTC_STACK_synchronize_encryption_db_to_olt(olt_id, onu_id_idx);
    }

    if((info_length-byte_num) != 0)
    {
        /*data remaining, block may be corrupted*/
        PONLOG_ERROR_0( PONLOG_FUNCTION_NAME, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, 
                            "There's data remaining, block may be corrupted\n");
    }

    return CTC_STACK_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME

#endif

