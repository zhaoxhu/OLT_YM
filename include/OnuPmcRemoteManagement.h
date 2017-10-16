/*                   Copyright(c) 2008-2012 GWTT, Inc.                    */
/*  
**  OnuPmcRemoteManagement.h -  Header file for PMC RemoteManagement Base Define 
**
**  This file was written by liwei056, 16/10/2013
**  
**  Changes:
**
**  Version       |  Date          |    Change        |    Author	  
**  ----------|-----------|-------------|------------
**	1.00          | 14/09/2012 |	creation	      | liwei056
*/

#if !defined(__ONU_PMC_RM_H__)
#define __ONU_PMC_RM_H__

#if defined(__cplusplus)
extern "C" {
#endif

/*============================= Include Files ===============================*/	
/*=============================== Constants =================================*/

/* REMOTE MANAGEMENT Code version */
#define REMOTE_PASONU_MAJOR_VERSION	 1
#define REMOTE_PASONU_MINOR_VERSION	 9
#define REMOTE_PASONU_BUILD_VERSION	 0

#define REMOTE_PASONU_APPLICATION_ID 0

#define MAX_APPLICATION_ID 3
/* The application id 0 is not in the scope of the assgin handler function */
#define MIN_APPLICATION_ID 1 
#define NUMBER_OF_APPLICATION_ID (MAX_APPLICATION_ID + 1)

#define GENERAL_MAC_ADDRESS_SIZE 6
#define GENERAL_IPV6_ADDRESS_SIZE 4

/* REMOTE MANAGEMENT specific handler functions enum */
typedef enum
{
	REMOTE_PASONU_HANDLER_RSTP_LOOP_DETECT_EVENT,		/* RSTP detect a loop					 */
												      
    REMOTE_PASONU_HANDLER_LAST_HANDLER,

} REMOTE_PASONU_handler_index_t;

typedef enum
{
    PON_ERRORED_SYMBOLS_PERIOD_ALARM,     
    PON_ERRORED_FRAME_ALARM,              
    PON_ERRORED_FRAME_PERIOD_ALARM,       
    PON_ERRORED_SECOND_SUMMARY_ALARM,     
    PON_DYING_GASP_ALARM,                 
    PON_LINK_FAULT_ALARM,                 
    PON_CRITICAL_EVENT_ALARM,             
    PON_IGMP_JOIN_EVENT,                  
    PON_IGMP_LEAVE_EVENT,                 
    PON_ENCRYPTION_KEY_EVENT,             
    PON_ENCRYPTION_STATE_EVENT,           
    PON_FEC_RX_MODE_EVENT,                
    PON_ADDRESS_TBL_FULL_EVENT,           
    PON_CONNECTION_STATE_EVENT,           
    PON_UNI_LINK_EVENT,                   
    PON_PON_LOOPBACK_EVENT,               
    PON_AUTHORIZATION_EVENT,              
    PON_PRE_RESET_EVENT,                  
    PON_IMAGE_BURN_COMPLETE_EVENT,        
    PON_EAPOL_RESPONSE_SENT_EVENT,        
    PON_RESERVED_EVENT_1,                 
    PON_RESERVED_EVENT_2,                 
    PON_MPCP_NACK_EVENT,                  
	PON_MLD_JOIN_EVENT,                   
    PON_MLD_DONE_EVENT,                   
    PON_HOST_TIMEOUT_EVENT,               
    PON_OAM_RECEIVE_EVENT,                
    PON_GPIO_IRQ_EVENT,
	
	PON_MAX_ALARM_EVENT

} PON_alarm_event_type;


#define PASCOMM_ONU_FIRMWARE_BINARY_CHUNK_OVERHEAD_SIZE  32
#define PASCOMM_ONU_FIRMWARE_BINARY_CHUNK_DEFAULT_SIZE   (128-PASCOMM_ONU_FIRMWARE_BINARY_CHUNK_OVERHEAD_SIZE)


/*----------------------------------------------------------*/
/*						Return codes						*/
/*----------------------------------------------------------*/
#define REMOTE_PASONU_EXIT_OK				        EXIT_OK
#define REMOTE_PASONU_EXIT_ERROR			        EXIT_ERROR
#define REMOTE_PASONU_TIME_OUT				        -2 
#define REMOTE_PASONU_NOT_IMPLEMENTED		        -3
#define REMOTE_PASONU_PARAMETER_ERROR		        -4
#define REMOTE_PASONU_HARDWARE_ERROR		        -5
#define REMOTE_PASONU_MEMORY_ERROR			        -6
#define REMOTE_PASONU_ILLEGAL_SIZE                  (-13001)
#define REMOTE_PASONU_WRONG_RESPONSE_MESSAGE_ID     (-13002)
#define REMOTE_PASONU_WRONG_RESPONSE_PDU_TYPE       (-13003)
#define REMOTE_PASONU_WRONG_RESPONSE_OPCODE_ID      (-13004)
#define REMOTE_PASONU_NULL_POINTER                  (-13005)
#define REMOTE_PASONU_NO_RESOURCES                  (-13006)          
#define REMOTE_PASONU_VLAN_MODE_NOT_CONFIGURED      (-13007)
#define REMOTE_PASONU_BAD_CONFIGURATION             (-13008)
#define REMOTE_PASONU_DISCARD_OAM                   (-13009)
#define REMOTE_PASONU_ALREADY_EXISTS                (-13010)
#define REMOTE_PASONU_ILLEGAL_ADDRESS               (-13011)
#define REMOTE_PASONU_MISMATCH                      (-13012)
#define REMOTE_PASONU_OLT_NOT_EXIST			        (-13013)
#define REMOTE_PASONU_QUERY_FAILED			        (-13014)
#define REMOTE_PASONU_ONU_NOT_AVAILABLE		        (-13015)

/*----------------------------------------------------------*/
/*						Legacy return codes						*/
/*----------------------------------------------------------*/
#define REMOTE_MANAGEMENT_EXIT_OK				        REMOTE_PASONU_EXIT_OK
#define REMOTE_MANAGEMENT_EXIT_ERROR			        REMOTE_PASONU_EXIT_ERROR
#define REMOTE_MANAGEMENT_TIME_OUT				        REMOTE_PASONU_TIME_OUT 
#define REMOTE_MANAGEMENT_NOT_IMPLEMENTED		        REMOTE_PASONU_NOT_IMPLEMENTED
#define REMOTE_MANAGEMENT_PARAMETER_ERROR		        REMOTE_PASONU_PARAMETER_ERROR
#define REMOTE_MANAGEMENT_HARDWARE_ERROR		        REMOTE_PASONU_HARDWARE_ERROR
#define REMOTE_MANAGEMENT_MEMORY_ERROR			        REMOTE_PASONU_MEMORY_ERROR
#define REMOTE_MANAGEMENT_OLT_NOT_EXIST			        REMOTE_PASONU_OLT_NOT_EXIST
#define REMOTE_MANAGEMENT_QUERY_FAILED			        REMOTE_PASONU_QUERY_FAILED
#define REMOTE_MANAGEMENT_ONU_NOT_AVAILABLE		        REMOTE_PASONU_ONU_NOT_AVAILABLE
#define REMOTE_MANAGEMENT_IGMP_TABLE_FULL		        REMOTE_PASONU_IGMP_TABLE_FULL
#define REMOTE_MANAGEMENT_IGMP_ENTRY_EXISTS             REMOTE_PASONU_IGMP_ENTRY_EXISTS
#define REMOTE_MANAGEMENT_ILLEGAL_SIZE                  REMOTE_PASONU_ILLEGAL_SIZE
#define REMOTE_MANAGEMENT_WRONG_RESPONSE_TYPE           REMOTE_PASONU_WRONG_RESPONSE_PDU_TYPE
#define REMOTE_MANAGEMENT_WRONG_RESPONSE_ID             REMOTE_PASONU_WRONG_RESPONSE_MESSAGE_ID
#define REMOTE_MANAGEMENT_NULL_POINTER                  REMOTE_PASONU_NULL_POINTER
#define REMOTE_MANAGEMENT_NO_RESOURCES                  REMOTE_PASONU_NO_RESOURCES          
#define REMOTE_MANAGEMENT_VLAN_MODE_NOT_CONFIGURED      REMOTE_PASONU_VLAN_MODE_NOT_CONFIGURED
#define REMOTE_MANAGEMENT_BAD_CONFIGURATION             REMOTE_PASONU_BAD_CONFIGURATION
#define REMOTE_MANAGEMENT_DISCARD_OAM                   REMOTE_PASONU_DISCARD_OAM
#define REMOTE_MANAGEMENT_ALREADY_EXISTS                REMOTE_PASONU_ALREADY_EXISTS
#define REMOTE_MANAGEMENT_ILLEGAL_ADDRESS               REMOTE_PASONU_ILLEGAL_ADDRESS
#define REMOTE_MANAGEMENT_MISMATCH                      REMOTE_PASONU_MISMATCH



/* Generic exit codes */
typedef enum
{
  S_OK,
  S_ERROR,
  S_INVALID,
  S_FAIL,
  S_NOT_SUPPORTED,
  S_NOT_INITIALIZED,
  S_OUT_OF_RANGE,
  S_BAD_PARAM,
  S_BAD_CONFIGURATION,
  S_NO_RESOURCES,
  S_NOT_FOUND,
  S_ALREADY_EXISTS,
  S_ILLEGAL_ADDRESS,
  S_NO_MEMORY,
  S_DISCARD,
  S_MISMATCH,
  S_WRONG_NUM_PARAMS,
  S_OS_ERROR,
  S_OVERFLOW,
  S_TIMEOUT,
  S_FUTURE_USE,
  S_LAST_STATUS
} REMOTE_STATUS;

#ifdef  PAS_SOFT_VERSION_V5_3_5
typedef enum PON_frame_qualifier_t
{
   PON_FRAME_ETHERTYPE,
   PON_FRAME_VLAN,
   PON_FRAME_IPV4,
   PON_FRAME_IPV4_TOS,
   PON_FRAME_IPV4_SA ,
   PON_FRAME_IPV4_DA

} PON_frame_qualifier_t;
#else
typedef enum
{
   PON_FRAME_ETHERTYPE,
   PON_FRAME_VLAN,
   PON_FRAME_IPV4
} PON_frame_qualifier_t;
#endif

typedef enum ENCRYPTION_type_t
{
    ENCRYPTION_OFF,
    ENCRYPTION_DS_ON,
    ENCRYPTION_US_DS_ON
} ENCRYPTION_type_t;

typedef enum ENCRYPTION_mode_t
{
    ENCRYPTION_AES,
    ENCRYPTION_TRIPLE_CHURNING,
} ENCRYPTION_mode_t;
  
typedef enum PON_traffic_qualifier_t
{
  PON_TRAFFIC_IP_ADDRESS,
  PON_TRAFFIC_IP_ADDRESS_NO_UDP,
  PON_TRAFFIC_IP_ADDRESS_NO_TCP,
  PON_TRAFFIC_IP_ADDRESS_NO_UDPTCP,
  PON_TRAFFIC_UDP_PORT,
  PON_TRAFFIC_TCP_PORT,
  PON_TRAFFIC_UDPTCP_PORT,
  PON_TRAFFIC_IP_AND_UDP_PORT,
  PON_TRAFFIC_IP_AND_TCP_PORT,
  PON_TRAFFIC_IP_AND_UDPTCP_PORT

} PON_traffic_qualifier_t;


typedef enum PON_traffic_address_t
{
  PON_TRAFFIC_DESTINATION,
  PON_TRAFFIC_SOURCE,
  PON_TRAFFIC_BOTH

} PON_traffic_address_t;


/*================================ Macroes ==================================*/
#define PACK_START
#define PACK_END          __attribute__((__packed__))


#define RM_MEM_MALLOC     VOS_Malloc
#define RM_MEM_FREE       VOS_Free
#define RM_MEM_ZERO       VOS_MemZero


extern bool pon_pmc_remote_management_is_init;

#define REMOTE_PASONU_CHECK_INIT \
    if ( !pon_pmc_remote_management_is_init ) \
    { \
        PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: REMOTE_MANAGEMENT package was not initialized. please call REMOTE_PASONU_init first.\n"); \
        return REMOTE_COMMUNICATION_EXIT_ERROR; \
    }


/*============================== Data Types =================================*/
#ifndef GENERAL_HANDLER_FUNC 
#define GENERAL_HANDLER_FUNC
typedef void (*general_handler_function_t)();
#endif

typedef int (*REMOTE_PASONU_received_event_func_t)  ( 
                      const short int        olt_id,
                      const short int        onu_id,
                      const unsigned short   length,
                      const unsigned char   *content);

typedef PACK_START struct {
  INT8U address[GENERAL_MAC_ADDRESS_SIZE]; /* first byte is MSB of address */
} PACK_END GENERAL_mac_addr_t;

typedef PACK_START struct {
  PACK_START union {
    INT32U addr_32[GENERAL_IPV6_ADDRESS_SIZE];
    INT16U addr_16[GENERAL_IPV6_ADDRESS_SIZE<<1];
    INT8U  addr_8 [GENERAL_IPV6_ADDRESS_SIZE<<2];
    } PACK_END ipv6_addr;
} PACK_END GENERAL_ipv6_addr_t;

/*=========================  Functions Prototype ============================*/
PON_STATUS RM_PASONU_init ( void );
PON_STATUS RM_PASONU_terminate ( void );
PON_STATUS RM_PASONU_is_init ( bool *init );

PON_STATUS RM_PASONU_assign_application_handler_function
                ( const REMOTE_PASONU_handler_index_t	handler_function_index, 
				  const void							(*handler_function)(),
				  unsigned short						*handler_id );
PON_STATUS RM_PASONU_delete_application_handler_function ( const unsigned short  handler_id );

PON_STATUS RM_PASONU_assign_handler( 
                                   const PON_alarm_event_type         type,
                                   const general_handler_function_t   handler );


#if defined(__cplusplus)
}
#endif

#endif /* __ONU_PMC_RM_H__ */


