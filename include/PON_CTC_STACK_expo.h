/*  
**  CTC_STACK_expo.h - CTC stack header file
**
**  This header file contains an extension to the PAS5001 system API 
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
*/


#ifndef _CTC_STACK_EXPO_H__
#define _CTC_STACK_EXPO_H__

/*============================= Include Files ===============================*/	
#include "PON_CTC_STACK_defines.h"
#include "PON_CTC_STACK_variable_descriptor_defines_expo.h"

/*=============================== Constants =================================*/
/* CTC STACK Code version */
#define CTC_STACK_MAJOR_VERSION	 2
#define CTC_STACK_MINOR_VERSION	 0
#define CTC_STACK_BUILD_VERSION	 1



#define MAX_PARAMETER_LENGTH                     1000

/*----------------------------------------------------------*/
/*						Return codes						*/
/*----------------------------------------------------------*/
#define CTC_STACK_EXIT_OK				            EXIT_OK
#define CTC_STACK_EXIT_ERROR			            EXIT_ERROR
#define CTC_STACK_TIME_OUT				            TIME_OUT 
#define CTC_STACK_NOT_IMPLEMENTED		            NOT_IMPLEMENTED
#define CTC_STACK_PARAMETER_ERROR		            PARAMETER_ERROR
#define CTC_STACK_HARDWARE_ERROR		            HARDWARE_ERROR
#define CTC_STACK_MEMORY_ERROR			            MEMORY_ERROR
#define CTC_STACK_QUERY_FAILED			            (-17001)
#define CTC_STACK_ONU_NOT_AVAILABLE		            (-17002)
#define CTC_STACK_OLT_NOT_EXIST			            (-17003)
#define CTC_STACK_EXIT_ERROR_ONU_DBA_THRESHOLDS     (-17004)
#define CTC_STACK_TFTP_SEND_FAIL					(-17005)
#define	CTC_STACK_RETURNED_INCONSISTENT_VALUES		(-17006)
#define	CTC_STACK_ALREADY_EXIST						(-17007)
#define	CTC_STACK_TABLE_FULL						(-17008)
#define	CTC_STACK_VARIABLE_INDICATION_BAD_PARAM		(-17009)
#define	CTC_STACK_VARIABLE_INDICATION_NO_RESOURCE	(-17010)


#define CTC_MIN_ETHERNET_PORT_NUMBER 0
#define CTC_MAX_ETHERNET_PORT_NUMBER 0x4F
 
#define CTC_MIN_VOIP_PORT_NUMBER 0x50
#define CTC_MAX_VOIP_PORT_NUMBER 0x8F

#define CTC_MIN_E1_PORT_NUMBER 0x90
#define CTC_MAX_E1_PORT_NUMBER 0x9F

/* CTC 2.1 */
#define CTC_MIN_MANAGEMENT_OBJECT_FRAME_NUMBER	0
#define CTC_MAX_MANAGEMENT_OBJECT_FRAME_NUMBER	3

#define CTC_MIN_MANAGEMENT_OBJECT_SLOT_NUMBER	1
#define CTC_MAX_MANAGEMENT_OBJECT_SLOT_NUMBER	62

#define CTC_MANAGEMENT_OBJECT_ALL_SLOTS			63

#define	CTC_MANAGEMENT_OBJECT_PON_PORT_NUMBER		0x0

#define CTC_MANAGEMENT_OBJECT_MIN_PORT    0x1
#define CTC_MANAGEMENT_OBJECT_MAX_PORT    0xFF

#define CTC_CLASSIFICATION_AND_MARKING_MIN_PRECEDENCE 1
#define CTC_CLASSIFICATION_AND_MARKING_MAX_PRECEDENCE 31


#define CTC_MAX_POLICING_PARAMETERS_VALUE 0xFFFFFF

#define CTC_MIN_SERVICE_SLA_QUEUE_NUMBER 0
#define CTC_MAX_SERVICE_SLA_QUEUE_NUMBER 7

#define CTC_MIN_SERVICE_SLA_WRR_WEIGHT 0
#define CTC_MAX_SERVICE_SLA_WRR_WEIGHT 100

/*================================ Macros ==================================*/

/*============================== Data Types =================================*/


/*CTC STACK redundancy information type*/
typedef enum
{
    CTC_STACK_PON_REDUNDANCY_ENCRYPTION_DB,       /*Status info*/
    CTC_STACK_PON_REDUNDANCY_DISCOVERY_INFO,       /*Status info*/
    CTC_STACK_PON_REDUNDANCY_AUTHENTICATION_DB,       /*Status info*/
}CTC_STACK_rdn_info_type_t;


/* CTC Stack specific handler functions enum */
typedef enum
{
	CTC_STACK_HANDLER_UPDATE_ENCRYPTION_KEY,		/* Update encryption event					 */
    CTC_STACK_HANDLER_END_OAM_DISCOVERY,			/* End of extended OAM discovery process	 */
	CTC_STACK_HANDLER_UPDATE_ONU_FIRMWARE_ERROR,	/* Update ONU Firmware error event           */
	CTC_STACK_HANDLER_ONU_FIRMWARE_REQUEST,			/* ONU requests to update its firmware event */ 
	CTC_STACK_HANDLER_AUTH_RESPONSE_LOID,          	/* Authentication response ONU_ID+Password	 */
    CTC_STACK_HANDLER_AUTHORIZED_LOID,              /* Authorized ONU_ID+Password           	 */
    CTC_STACK_HANDLER_EVENT_NOTIFICATION,           /* Organization Specific Event Notification  */
    CTC_STACK_HANDLER_ONU_AUTHORIZE_TO_THE_NETWORK, /* ONU is authorized to the network          */
	CTC_STACK_HANDLER_ENCRYPTION_KEY_VALUE,	     	/* Update encryption event + the key value   */
	CTC_STACK_HANDLER_UPDATE_FEC_MODE,              /* Update fec mode event                     */

    CTC_STACK_HANDLER_LAST_HANDLER,

} CTC_STACK_handler_index_t;

typedef struct
{
	CTC_STACK_vlan_modes_t		mode;
	unsigned long				vlan_list[CTC_MAX_VLAN_FILTER_ENTRIES];
	unsigned char				number_of_entries;	/* This parameter describes how many entries in <vlan_list> are used, when <mode> == <CTC_VLAN_MODE_FILTER> || <CTC_VLAN_MODE_TRANSLATION> */
} CTC_STACK_vlan_configuration_t; 

typedef struct
{
	CTC_STACK_multicast_vlan_opers_t	vlan_operation;
	unsigned short						vlan_id[CTC_MAX_MULTICAST_VLAN_ENTRIES];
	unsigned char						num_of_vlan_id;	/* Number of entries used in <vlan_id> */
} CTC_STACK_multicast_vlan_port_t;

typedef struct
{
	CTC_STACK_multicast_control_action_t	action;
	CTC_STACK_multicast_control_type_t		control_type;
	CTC_STACK_multicast_entry_t				entries[CTC_MAX_MULTICAST_MAC_ENTRIES];
	unsigned char							num_of_entries;	/* The count of rules within <entries> */
} CTC_STACK_multicast_control_port_t;



typedef struct
{
	unsigned char						port_number;
	bool								flow_control_enable;
} CTC_STACK_ethernet_port_pause_entry_t;

typedef  CTC_STACK_ethernet_port_pause_entry_t CTC_STACK_ethernet_ports_pause_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	CTC_management_object_index_t		management_object_index;
	bool								flow_control_enable;
} CTC_STACK_ethernet_management_object_pause_entry_t;

typedef  CTC_STACK_ethernet_management_object_pause_entry_t CTC_STACK_ethernet_management_object_pause_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef struct
{
	unsigned char								port_number;
	CTC_STACK_ethernet_port_policing_entry_t	entry;
} CTC_STACK_ethernet_policing_entry_t;

typedef  CTC_STACK_ethernet_policing_entry_t CTC_STACK_ethernet_ports_policing_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	CTC_management_object_index_t		        management_object_index;
	CTC_STACK_ethernet_port_policing_entry_t	entry;
} CTC_STACK_ethernet_management_object_policing_entry_t;

typedef  CTC_STACK_ethernet_management_object_policing_entry_t CTC_STACK_ethernet_management_object_policing_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef struct
{
	unsigned char								port_number;
	CTC_STACK_classification_rules_t			entry;
} CTC_STACK_classification_port_entry_t;

typedef  CTC_STACK_classification_port_entry_t CTC_STACK_classification_ports_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	CTC_management_object_index_t		        management_object_index;
	CTC_STACK_classification_rules_t			entry;
} CTC_STACK_classification_management_object_entry_t;

typedef  CTC_STACK_classification_management_object_entry_t CTC_STACK_classification_management_object_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef struct
{
	unsigned char				port_number;
	CTC_STACK_on_off_state_t	port_state;
} CTC_STACK_port_state_entry_t;

typedef  CTC_STACK_port_state_entry_t CTC_STACK_ports_state_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	CTC_management_object_index_t management_object_index;
	CTC_STACK_on_off_state_t	  port_state;
} CTC_STACK_management_object_state_entry_t;

typedef  CTC_STACK_management_object_state_entry_t CTC_STACK_management_object_state_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef struct
{
	unsigned char		port_number;
	bool				state;
} CTC_STACK_ethernet_port_state_entry_t;

typedef  CTC_STACK_ethernet_port_state_entry_t CTC_STACK_ethernet_ports_state_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	CTC_management_object_index_t  management_object_index;
	bool				           state;
} CTC_STACK_ethernet_management_object_state_entry_t;

typedef  CTC_STACK_ethernet_management_object_state_entry_t CTC_STACK_ethernet_management_object_state_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef  unsigned char CTC_STACK_ports_t[MAX_PORT_ENTRIES_NUMBER];
typedef  CTC_management_object_index_t CTC_STACK_management_object_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef struct
{
	unsigned char port_number;
	unsigned char group_num;
} CTC_STACK_multicast_group_num_t;

typedef  CTC_STACK_multicast_group_num_t CTC_STACK_multicast_ports_group_num_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	CTC_management_object_index_t  management_object_index;
	unsigned char                  group_num;
} CTC_STACK_multicast_management_object_group_num_entry_t;

typedef  CTC_STACK_multicast_management_object_group_num_entry_t CTC_STACK_multicast_management_object_group_num_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef struct
{
	unsigned char								port_number;
	CTC_STACK_port_vlan_configuration_t			entry;
} CTC_STACK_vlan_configuration_entry_t;

typedef  CTC_STACK_vlan_configuration_entry_t CTC_STACK_vlan_configuration_ports_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	CTC_management_object_index_t               management_object_index;
	CTC_STACK_port_vlan_configuration_t			entry;
} CTC_STACK_vlan_configuration_management_object_entry_t;

typedef  CTC_STACK_vlan_configuration_management_object_entry_t CTC_STACK_vlan_configuration_management_object_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef struct
{
	unsigned char						port_number;
	CTC_STACK_multicast_vlan_t			entry;
} CTC_STACK_multicast_vlan_entry_t;

typedef  CTC_STACK_multicast_vlan_entry_t CTC_STACK_multicast_vlan_ports_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	CTC_management_object_index_t       management_object_index;
	CTC_STACK_multicast_vlan_t			entry;
} CTC_STACK_multicast_vlan_management_object_entry_t;

typedef  CTC_STACK_multicast_vlan_management_object_entry_t CTC_STACK_multicast_vlan_management_object_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef enum
{
	CTC_CLASSIFICATION_DELETE_RULE	=	CTC_CLASS_ACTION_DELETE_RULE,
	CTC_CLASSIFICATION_ADD_RULE		=	CTC_CLASS_ACTION_ADD_RULE,
} CTC_STACK_classification_rule_mode_t;



typedef struct
{
	unsigned char						port_number;
	CTC_STACK_link_state_t				link_state;
} CTC_STACK_ethernet_link_state_entry_t;

typedef  CTC_STACK_ethernet_link_state_entry_t CTC_STACK_ethernet_ports_link_state_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	CTC_management_object_index_t       management_object_index;
	CTC_STACK_link_state_t				link_state;
} CTC_STACK_ethernet_management_object_link_state_entry_t;

typedef  CTC_STACK_ethernet_management_object_link_state_entry_t CTC_STACK_ethernet_management_object_link_state_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef struct
{
	unsigned char						port_number;
	bool								tag_strip;
} CTC_STACK_multicast_port_tag_strip_t;

typedef  CTC_STACK_multicast_port_tag_strip_t CTC_STACK_multicast_ports_tag_strip_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	CTC_management_object_index_t       management_object_index;
	bool								tag_strip;
} CTC_STACK_multicast_management_object_tag_strip_entry_t;

typedef  CTC_STACK_multicast_management_object_tag_strip_entry_t CTC_STACK_multicast_management_object_tag_strip_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];

typedef struct
{
	unsigned char  port_number;
	CTC_STACK_statistic_state_t state;
}CTC_STACK_port_statistic_state_t;

typedef CTC_STACK_port_statistic_state_t CTC_STACK_ports_statistic_state_t[MAX_PORT_ENTRIES_NUMBER];

typedef struct
{
	CTC_management_object_index_t management_object_index;
	CTC_STACK_statistic_state_t state;
}CTC_STACK_management_object_statistic_state_entry_t;

typedef CTC_STACK_management_object_statistic_state_entry_t CTC_STACK_management_object_statistic_state_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef struct{
	unsigned char port_number;
	CTC_STACK_statistic_data_t data;
}CTC_STACK_port_statistic_data_t;

typedef struct
{
	CTC_management_object_index_t management_object_index;
	CTC_STACK_statistic_data_t data;
}CTC_STACK_management_object_statistic_data_entry_t;

typedef CTC_STACK_management_object_statistic_data_entry_t CTC_STACK_management_object_statistic_data_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];

typedef struct
{
	unsigned char						 port_number;
	CTC_STACK_tag_oper_t				 tag_oper;
    CTC_STACK_multicast_vlan_switching_t multicast_vlan_switching;
} CTC_STACK_multicast_port_tag_oper_t;

typedef  CTC_STACK_multicast_port_tag_oper_t CTC_STACK_multicast_ports_tag_oper_t[MAX_PORT_ENTRIES_NUMBER];

typedef struct
{
	CTC_management_object_index_t        management_object_index;
	CTC_STACK_tag_oper_t				 tag_oper;
    CTC_STACK_multicast_vlan_switching_t multicast_vlan_switching;
} CTC_STACK_multicast_management_object_tag_oper_entry_t;

typedef  CTC_STACK_multicast_management_object_tag_oper_entry_t CTC_STACK_multicast_management_object_tag_oper_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef struct
{
	unsigned char										port_number;
	CTC_STACK_auto_negotiation_technology_ability_t		abilities;
} CTC_STACK_auto_negotiation_port_technology_ability_t;

typedef  CTC_STACK_auto_negotiation_port_technology_ability_t CTC_STACK_auto_negotiation_ports_technology_ability_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	CTC_management_object_index_t                       management_object_index;
	CTC_STACK_auto_negotiation_technology_ability_t		abilities;
} CTC_STACK_auto_negotiation_management_object_technology_ability_entry_t;

typedef  CTC_STACK_auto_negotiation_management_object_technology_ability_entry_t CTC_STACK_auto_negotiation_management_object_technology_ability_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef struct   
{
	unsigned char								port_number;
	CTC_STACK_port_qinq_configuration_t			entry;
} CTC_STACK_qinq_configuration_entry_t;

typedef  CTC_STACK_qinq_configuration_entry_t CTC_STACK_qinq_configuration_ports_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct   
{
	CTC_management_object_index_t               management_object_index;
	CTC_STACK_port_qinq_configuration_t			entry;
} CTC_STACK_qinq_configuration_management_object_entry_t;

typedef  CTC_STACK_qinq_configuration_management_object_entry_t CTC_STACK_qinq_configuration_management_object_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


typedef struct 
{			
	CTC_management_object_index_t	management_object_index;
	CTC_STACK_alarm_id_t			alarm_id;
	bool							enable;
}CTC_STACK_alarm_admin_state_t;


typedef struct 
{			
	CTC_management_object_index_t	management_object_index;
	CTC_STACK_alarm_id_t			alarm_id;
	unsigned long					alarm_threshold;         /*Alarm threshold*/ 
	unsigned long					clear_threshold;        /*Clearing alarm threshold*/
}CTC_STACK_alarm_threshold_t;


typedef struct
{
	CTC_management_object_index_t		management_object_index;
	unsigned long						loop_detect_enable;
} CTC_STACK_management_port_object_loop_detect_entry_t;

typedef  CTC_STACK_management_port_object_loop_detect_entry_t CTC_STACK_management_object_port_loop_detect_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


/*========================= Functions Prototype =============================*/

/* Init								
**
** This command initializes the CTC stack.  
**	
** Input Parameters:		  
**
**		automatic_mode          : Automatic mode. if enabled, after new OLT was added, 
**								  set automatically its encryption mode to 'triple churning'
**								  and configure the OLT to authorize ONU MAC addresses according
**								  to its table.
**		number_of_records		: Number of oui and version records for extended oam discovery process. Range:0-MAX_OUI_RECORDS
**		oui_version_records_list: See CTC_STACK_oui_version_record_t for details
**
** Return codes:
**				All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_init 
                ( const bool							 automatic_mode,
                  const unsigned char					 number_of_records,
                  const CTC_STACK_oui_version_record_t  *oui_version_records_list );



/* Set parameters								
**
** This command set OUI records list. 
**	
** Input Parameters:		  
**
**		automatic_mode          : Automatic mode. if enabled, after new OLT was added, 
**								  set automatically its encryption mode to 'triple churning'
**								  and configure the OLT to authorize ONU MAC addresses according
**								  to its table.
**		number_of_records		: Number of OUIs and version records for extended oam discovery process. Range:0-MAX_OUI_RECORDS
**		oui_version_records_list: Supported OUIs (+versions) list, used in the extended OAM discovery process.
**								  See CTC_STACK_oui_version_record_t for details
**		automatic_onu_configuration: default ONU configuration mode when registered
**
** Return codes:
**				All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_parameters 
                ( const bool							 automatic_mode,
                  const unsigned char					 number_of_records,
                  const CTC_STACK_oui_version_record_t  *oui_version_records_list,
				  const bool							 automatic_onu_configuration);


/* Get parameters								
**
** This command retrieves OUI records list. 
**	
** Output Parameters:		  
**
**		automatic_mode          : Automatic mode
**		number_of_oui_records   : Number of OUI and version records for extended oam discovery process
**		oui_version_records_list: See CTC_STACK_oui_version_record_t for details
**		automatic_onu_configuration: default ONU configuration mode when registered
**
** Return codes:
**				All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_parameters 
                ( bool							  *automatic_mode,
                  unsigned char					  *number_of_records,
                  CTC_STACK_oui_version_record_t  *oui_version_records_list,
				  bool							  *automatic_onu_configuration);

/* Is init								
**
** This command returns whether the CTC stack is initialized
**	
** Input Parameters:	
**	  
** Ouput Parameters:		  
**      init                    : FALSE or TRUE
**
** Return codes:
**				All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_is_init ( bool *init );



/* Terminate								
**
** This command terminates the CTC stack. 
**
** Input Parameters:
**			  
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_terminate ( void );



/* Assign handler function									
**
** Assign (register) a handler function which handles a single event type.
**
** Input Parameters:
**			    handler_function_index  : Event type to be handled by the handler function
**			    handler_function        : Pointer to the handler function
** Output Parameters:
**			    handler_id			    : Handler function identification. Should be used when activating Delete handler function
**										  
**
** Return codes:
**		All the defined CTC_STACK_* return codes
*/

PON_STATUS PON_CTC_STACK_assign_handler_function
                ( const CTC_STACK_handler_index_t    handler_function_index, 
				  const void					    (*handler_function)(),
				  unsigned short                     *handler_id );



/* Delete handler function									
**
** Remove a handler function from the DB.
**
** Input Parameters:
**	    handler_id     : Handler function identification.
**										  
**
** Return codes:
**		All the defined CTC_STACK_* return codes
*/

PON_STATUS PON_CTC_STACK_delete_handler_function ( const unsigned short  handler_id );




/* Extended OAM discovery state handler
**
** An event indicating the completion of an extended OAM discovery of an ONU.
** The discovery process may succeed or fail.
**
** Input Parameters:
**			olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		    llid				: LLID,   range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT 
**			result			    : Return code: success(CTC_DISCOVERY_STATE_COMPLETE) or 
**                                             error(CTC_DISCOVERY_STATE_UNSUPPORTED)
**			number_of_oui_records   : Number of OUI and version records for extended oam discovery process
**			oui_version_records_list: See CTC_STACK_oui_version_record_t for details
**
** Return codes:
**				Return codes are not specified for an event
**
*/

typedef void (*CTC_STACK_oam_discovery_state_handler_t) 
				  ( const PON_olt_id_t					   olt_id, 
					const PON_llid_t					   llid,
					const CTC_STACK_discovery_state_t	   result,
                    const unsigned char 	               number_of_records,
                    const CTC_STACK_oui_version_record_t  *oui_version_record);



/* Set extended OAM discovery timing parameters
**
** Input Parameters:
**		discovery_timeout	: Discovery timeout measured in 100 ms units.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_extended_oam_discovery_timing 
                ( unsigned short int discovery_timeout );


/* Get extended OAM discovery timing parameters
**
** Input Parameters:
**
** Output Parameters:
**		discovery_timeout	: Discovery timeout measured in 100 ms units.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_extended_oam_discovery_timing 
                ( unsigned short int  *discovery_timeout );



/* Set encryption timing parameters
**
** Input Parameters:
**
**		update_key_time	   : Encryption key update timer for all ONUs, measured in seconds.  
**		no_reply_timeout   : Encryption response timeout, measured in 100 milliseconds
**							 Range: 0 - CTC_STACK_MAX_ENCRYPTION_TIMEOUT
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_encryption_timing 
                ( const unsigned char       update_key_time,
                  const unsigned short int  no_reply_timeout );


/* Get encryption timing parameters
**
** Input Parameters:
**
** Output Parameters:
**
**		update_key_time	 : Encryption key update timer for all ONUs, measured in seconds. 
**		no_reply_timeout : Encryption response timeout, measured in 100 milliseconds
**						   Range: 0 - CTC_STACK_MAX_ENCRYPTION_TIMEOUT
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_encryption_timing 
                ( unsigned char       *update_key_time,
                  unsigned short int  *no_reply_timeout );




                  
/* Start encryption
**
** Input Parameters:
**		olt_id	               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid	               : LLID,   range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_start_encryption ( const PON_olt_id_t   olt_id, 
                                        const PON_llid_t     llid );



/* Update encryption key acknowledge handler
**
** An event confirming encryption key update for a LLID executed / error occurred.
** Note: If this event isn't assigned a handling function, CTC_STACK_start_encryption function
** will return the confirmation return code (but will take more time).
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		    llid				: LLID,   range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT 
**			return_code			: Return code: success(CTC_ENCRYPTION_STATE_ENCRYPTION_COMPLETE) or 
**                                             error(CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED)
**
** Return codes:
**				Return codes are not specified for an event
**
*/

typedef void (*CTC_STACK_update_encryption_key_acknowledge_handler_t) 
				  ( const PON_olt_id_t					olt_id, 
					const PON_llid_t					llid, 
					const CTC_STACK_encryption_state_t  return_code );




/* Stop encryption
**
** Input Parameters:
**		olt_id	               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid	               : LLID,   range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_stop_encryption ( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_llid_t     llid );


/* Set DBA report thresholds
**
** Input Parameters:
**		olt_id	               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	               : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		number_of_queue_sets   : Number of queue sets in REPORT. Range: 2-4
**      queues_thresholds      : Array of qeueue sets, each set contains several queues, for each queue state and threshold are configured
**
** Output Parameters:
**		number_of_queue_sets   : Number of queue sets in REPORT frames
**      queues_sets_thresholds : Array of qeueue sets, each set contains several queues, for each queue state and threshold are configured
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_dba_report_thresholds ( 
                                   const PON_olt_id_t					  olt_id, 
                                   const PON_onu_id_t					  onu_id,
                                   unsigned char 						 *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds );




/* Get DBA report thresholds
**
** Input Parameters:
**		olt_id	               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	               : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		number_of_queue_sets   : Number of queue sets in REPORT frames
**      queues_sets_thresholds : Array of qeueue sets, each set contains several queues, for each queue state and threshold are retrieved
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_dba_report_thresholds ( 
                                   const PON_olt_id_t					  olt_id, 
                                   const PON_onu_id_t					  onu_id,
                                   unsigned char 						 *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds );




/* Get ONU serial number
**
** This function retrieves ONU serial number which is composed of vendor id, ONU model and ONU ID (MAC address)
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
**
** Output Parameters:
**      onu_serial_number	: See CTC_STACK_onu_serial_number_t for details.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_onu_serial_number ( 
           const PON_olt_id_t			   olt_id, 
           const PON_onu_id_t			   onu_id,
		   CTC_STACK_onu_serial_number_t  *onu_serial_number );


/* Get firmware version (extended)
**
** This function retrieves ONU firmware version.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		version_buffer_size : Allocated buffer size, up to 127 bytes.
**
** Output Parameters:
**      version_buffer		: Firmware version.
**		version_buffer_size : Actual buffer size, up to 127 bytes.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_firmware_version_extended ( 
           const PON_olt_id_t	olt_id, 
           const PON_onu_id_t	onu_id,
           INT8U				*version_buffer,
           INT8U				*version_buffer_size);


/* Get firmware version
**
** This function retrieves ONU firmware version.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
**
** Output Parameters:
**      version				: Firmware version.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_firmware_version ( 
           const PON_olt_id_t	olt_id, 
           const PON_onu_id_t	onu_id,
		   unsigned short int  *version );


/* Get chipset id
**
** This function retrieves ONU chipset information.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
**
** Output Parameters:
**      chipset_id			: See CTC_STACK_chipset_id_t for details.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_chipset_id ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id,
		   CTC_STACK_chipset_id_t  *chipset_id );


/* Get ONU capabilities
**
** This function retrieves ONU main HW capabilities
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      onu_capabilities	: See CTC_STACK_onu_capabilities_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_onu_capabilities ( 
           const PON_olt_id_t			  olt_id, 
           const PON_onu_id_t			  onu_id,
		   CTC_STACK_onu_capabilities_t  *onu_capabilities );


/* Set Ethernet port pause
**
** This function sets flow control activation status for an Ethernet port of a specified ONU
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**      flow_control_enable	: Enable or disable flow control for the port.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_ethernet_port_pause ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const bool			 flow_control_enable);

PON_STATUS PON_CTC_STACK_set_ethernet_management_object_pause ( 
           const PON_olt_id_t	                    olt_id, 
           const PON_onu_id_t	                    onu_id,
		   const CTC_management_object_index_t		management_object_index,
		   const bool			                    flow_control_enable);

/* Get Ethernet port pause
**
** This function retrieves flow control activation status for an Ethernet port of a specified ONU
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**		flow_control_enable	: Is flow Control enabled for the port
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/


PON_STATUS PON_CTC_STACK_get_ethernet_port_pause ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   bool				    *flow_control_enable);

PON_STATUS PON_CTC_STACK_get_ethernet_management_object_pause ( 
           const PON_olt_id_t	                    olt_id, 
           const PON_onu_id_t	                    onu_id,
		   const CTC_management_object_index_t		management_object_index,
		   bool				                       *flow_control_enable);

/* Get Ethernet port pause for all ports
**
** This function retrieves flow control activation status for all the Ethernet ports of a specified ONU
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports info parameter
**		ports_info			: Ethernet ports pause info structure. See CTC_STACK_ethernet_ports_pause_t for details
**      or
**      management_object_info  : See CTC_STACK_ethernet_management_object_pause_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_ethernet_all_port_pause ( 
           const PON_olt_id_t				 olt_id, 
           const PON_onu_id_t				 onu_id,
		   unsigned char					*number_of_entries,
		   CTC_STACK_ethernet_ports_pause_t  ports_info );

PON_STATUS PON_CTC_STACK_get_ethernet_all_management_object_pause ( 
           const PON_olt_id_t				             olt_id, 
           const PON_onu_id_t				             onu_id,
		   unsigned char					            *number_of_entries,
		   CTC_STACK_ethernet_management_object_pause_t  management_object_info );


/* Set Ethernet port policing
**
** This function sets upstream service policing configuration for specified  UNI Ethernet ports of an ONU
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**      port_policing		: See CTC_STACK_ethernet_port_policing_entry_t for details.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_ethernet_port_policing ( 
           const PON_olt_id_t								olt_id, 
           const PON_onu_id_t								onu_id,
		   const unsigned char								port_number,
		   const CTC_STACK_ethernet_port_policing_entry_t   port_policing);

PON_STATUS PON_CTC_STACK_set_ethernet_management_object_policing ( 
           const PON_olt_id_t								             olt_id, 
           const PON_onu_id_t								             onu_id,
		   const CTC_management_object_index_t		                     management_object_index,
		   const CTC_STACK_ethernet_port_policing_entry_t                port_entry);

/* Get Ethernet port policing
**
** This function retrieves upstream service policing configuration for an Ethernet ports of a specified ONU
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**      port_policing		: See CTC_STACK_ethernet_port_policing_entry_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_ethernet_port_policing ( 
           const PON_olt_id_t						  olt_id, 
           const PON_onu_id_t						  onu_id,
		   const unsigned char						  port_number,
		   CTC_STACK_ethernet_port_policing_entry_t  *port_policing);

PON_STATUS PON_CTC_STACK_get_ethernet_management_object_policing ( 
           const PON_olt_id_t						  olt_id, 
           const PON_onu_id_t						  onu_id,
		   const CTC_management_object_index_t		  management_object_index,
		   CTC_STACK_ethernet_port_policing_entry_t  *port_policing);

/* Get Ethernet port policing for all ports
**
** This function retrieves upstream service policing configuration for all the Ethernet ports of a specified ONU
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports_info.
**      ports_info			: See CTC_STACK_ethernet_ports_policing_t for details.
**      or
**      management_object_info  : See CTC_STACK_ethernet_management_object_policing_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_ethernet_all_port_policing ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   unsigned char						*number_of_entries,
		   CTC_STACK_ethernet_ports_policing_t   ports_info );

PON_STATUS PON_CTC_STACK_get_ethernet_all_management_object_policing ( 
           const PON_olt_id_t					             olt_id, 
           const PON_onu_id_t					             onu_id,
		   unsigned char						            *number_of_entries,
		   CTC_STACK_ethernet_management_object_policing_t   management_object_info );

/* Set VoIP port management
**
** This function set POTS port state.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_VOIP_PORT_NUMBER - CTC_MAX_VOIP_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**      port_state			: Active or deactive the VoIP port.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_voip_port ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   const unsigned char				port_number,
		   const CTC_STACK_on_off_state_t   port_state );

PON_STATUS PON_CTC_STACK_set_voip_management_object ( 
           const PON_olt_id_t				   olt_id, 
           const PON_onu_id_t				   onu_id,
		   const CTC_management_object_index_t management_object_index,
		   const CTC_STACK_on_off_state_t      port_state );


/* Get VoIP port management
**
** This function retrieves port's states for the specified ONU VoIP port.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_VOIP_PORT_NUMBER - CTC_MAX_VOIP_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**      port_state			: Is port activated or deactivated.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_voip_port ( 
           const PON_olt_id_t		  olt_id, 
           const PON_onu_id_t		  onu_id,
		   const unsigned char		  port_number,
		   CTC_STACK_on_off_state_t  *port_state);

PON_STATUS PON_CTC_STACK_get_voip_management_object ( 
           const PON_olt_id_t		           olt_id, 
           const PON_onu_id_t		           onu_id,
		   const CTC_management_object_index_t management_object_index,
		   CTC_STACK_on_off_state_t           *port_state);

/* Get VoIP port management for all ports
**
** This function retrieves port state activation status for all the VoIP ports of a specified ONU
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports info parameter
**		ports_info			: VoIP ports state info structure. See CTC_STACK_ports_state_t for details
**      or
**      management_object_info  : See CTC_STACK_management_object_state_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_voip_all_port ( 
           const PON_olt_id_t		 olt_id, 
           const PON_onu_id_t		 onu_id,
		   unsigned char			*number_of_entries,
		   CTC_STACK_ports_state_t   ports_info );

PON_STATUS PON_CTC_STACK_get_voip_all_management_object ( 
           const PON_olt_id_t		             olt_id, 
           const PON_onu_id_t		             onu_id,
		   unsigned char			            *number_of_entries,
		   CTC_STACK_management_object_state_t   management_object_info );

/* Set E1 port management
**
** This function set E1 port state.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_E1_PORT_NUMBER - CTC_MAX_E1_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**      port_state			: Active or deactive the E1 port.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_e1_port ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   const unsigned char				port_number,
		   const CTC_STACK_on_off_state_t   port_state );

PON_STATUS PON_CTC_STACK_set_e1_management_object ( 
           const PON_olt_id_t				   olt_id, 
           const PON_onu_id_t				   onu_id,
		   const CTC_management_object_index_t management_object_index,
		   const CTC_STACK_on_off_state_t      port_state );


/* Get E1 port management
**
** This function retrieves port's states for the specified ONU E1 port.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_E1_PORT_NUMBER - CTC_MAX_E1_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**      port_state			: Is port activated or deactivated.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_e1_port ( 
           const PON_olt_id_t		  olt_id, 
           const PON_onu_id_t		  onu_id,
		   const unsigned char		  port_number,
		   CTC_STACK_on_off_state_t  *port_state);

PON_STATUS PON_CTC_STACK_get_e1_management_object ( 
           const PON_olt_id_t		           olt_id, 
           const PON_onu_id_t		           onu_id,
		   const CTC_management_object_index_t management_object_index,
		   CTC_STACK_on_off_state_t           *port_state);


/* Get E1 port management for all ports
**
** This function retrieves port state activation status for all the E1 ports of a specified ONU
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports info parameter
**		ports_info			: E1 ports state info structure. See CTC_STACK_ports_state_t for details
**      or
**      management_object_info  : See CTC_STACK_management_object_state_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_e1_all_port ( 
           const PON_olt_id_t		 olt_id, 
           const PON_onu_id_t		 onu_id,
		   unsigned char			*number_of_entries,
		   CTC_STACK_ports_state_t   ports_info );

PON_STATUS PON_CTC_STACK_get_e1_all_management_object ( 
           const PON_olt_id_t		             olt_id, 
           const PON_onu_id_t		             onu_id,
		   unsigned char			            *number_of_entries,
		   CTC_STACK_management_object_state_t   management_object_info );

/* Set VLAN port configuration
**
** This function performs VLAN configuration to a specific Ethernet port
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number				: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**      port_configuration		: See CTC_STACK_port_vlan_configuration_t for details.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_vlan_port_configuration ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const unsigned char						   port_number,
		   const CTC_STACK_port_vlan_configuration_t   port_configuration);

PON_STATUS PON_CTC_STACK_set_vlan_management_object_configuration ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_port_vlan_configuration_t   port_configuration);

/* Get VLAN port configuration
**
** This function retrieves VLAN operation Ethernet port configuration for the Ethernet port of specified ONU
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number				: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**      port_configuration		: See CTC_STACK_port_vlan_configuration_t for details.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_vlan_port_configuration ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const unsigned char					 port_number,
		   CTC_STACK_port_vlan_configuration_t  *port_configuration);

PON_STATUS PON_CTC_STACK_get_vlan_management_object_configuration ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const CTC_management_object_index_t   management_object_index,
		   CTC_STACK_port_vlan_configuration_t  *port_configuration);

/* Get VLAN port configuration for all ports
**
** This function retrieves VLAN operation Ethernet port configuration for the  all Ethernet ports of specified ONU
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      number_of_entries		: Number of valid entries in ports info 
**      ports_info				: See CTC_STACK_vlan_configuration_ports_t for details.
**      or
**      management_object_info  : See CTC_STACK_vlan_configuration_management_object_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_vlan_all_port_configuration ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   unsigned char						 *number_of_entries,
		   CTC_STACK_vlan_configuration_ports_t   ports_info );

PON_STATUS PON_CTC_STACK_get_vlan_all_management_object_configuration ( 
           const PON_olt_id_t					              olt_id, 
           const PON_onu_id_t					              onu_id,
		   unsigned char						             *number_of_entries,
		   CTC_STACK_vlan_configuration_management_object_t   management_object_info );



/* Set classification and marking rules
**
** This function set classification and marking rules.
**
** Input Parameters:
**		olt_id						: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id						: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number					: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index     : See CTC_management_object_index_t for details.
**
**      mode						: Operation type to Classification Queuing&Marking control list
**      classification_and_marking	: See CTC_STACK_classification_rules_t for details.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_classification_and_marking ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const unsigned char							port_number,
		   const CTC_STACK_classification_rule_mode_t   mode,
		   const CTC_STACK_classification_rules_t		classification_and_marking);

PON_STATUS PON_CTC_STACK_set_classification_and_marking_management_object ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_management_object_index_t          management_object_index,
		   const CTC_STACK_classification_rule_mode_t   mode,
		   const CTC_STACK_classification_rules_t		classification_and_marking);

/* Get classification and marking rules
**
** This function retrieves classification and marking rules for a specific port.
**
** Input Parameters:
**		olt_id						: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id						: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number					: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index     : See CTC_management_object_index_t for details.
**
** Output Parameters:
**      classification_and_marking	: See CTC_STACK_classification_rules_t for details.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_classification_and_marking ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   const unsigned char				  port_number,
		   CTC_STACK_classification_rules_t   classification_and_marking);

PON_STATUS PON_CTC_STACK_get_classification_and_marking_management_object ( 
           const PON_olt_id_t				   olt_id, 
           const PON_onu_id_t				   onu_id,
		   const CTC_management_object_index_t management_object_index,
		   CTC_STACK_classification_rules_t    classification_and_marking);

/* Get classification and marking rules for all ports
**
** This function retrieves classification and marking rules for all ports.
**
** Input Parameters:
**		olt_id						: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id						: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports info parameter
**		ports_info			: See CTC_STACK_classification_ports_t for details
**      or
**      management_object_info  : See CTC_STACK_classification_management_object_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_classification_and_marking_all_port ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   unsigned char					 *number_of_entries,
		   CTC_STACK_classification_ports_t   ports_info );

PON_STATUS PON_CTC_STACK_get_classification_and_marking_all_management_object ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char					             *number_of_entries,
		   CTC_STACK_classification_management_object_t   management_object_info );

/* Delete classification and marking list
**
** This function delete classification and marking list.
**
** Input Parameters:
**		olt_id						: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id						: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number					: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or                       
**      management_object_index     : See CTC_management_object_index_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_delete_classification_and_marking_list ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t    onu_id,
		   const unsigned char   port_number);

PON_STATUS PON_CTC_STACK_delete_classification_and_marking_management_object_list ( 
           const PON_olt_id_t	               olt_id, 
           const PON_onu_id_t                  onu_id,
		   const CTC_management_object_index_t management_object_index);


/* Set multicast vlan
**
** This function sets ONU multicast VLAN configuration.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**      multicast_vlan		: See CTC_STACK_multicast_vlan_t for details.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_multicast_vlan ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   const unsigned char				  port_number,
		   const CTC_STACK_multicast_vlan_t   multicast_vlan);

PON_STATUS PON_CTC_STACK_set_multicast_vlan_management_object ( 
           const PON_olt_id_t				     olt_id, 
           const PON_onu_id_t				     onu_id,
		   const CTC_management_object_index_t   management_object_index,
		   const CTC_STACK_multicast_vlan_t      multicast_vlan);


/* Clear multicast vlan
**
** This function clears ONU multicast VLAN configuration
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_clear_multicast_vlan ( 
           const PON_olt_id_t    olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number);

PON_STATUS PON_CTC_STACK_clear_multicast_vlan_management_object ( 
           const PON_olt_id_t                  olt_id, 
           const PON_onu_id_t	               onu_id,
		   const CTC_management_object_index_t management_object_index);

/* Get multicast vlan port
**
** This function retrieves ONU multicast VLAN configuration for a specific Ethernet port
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**      multicast_vlan		: See CTC_STACK_multicast_vlan_t for details.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_multicast_vlan ( 
           const PON_olt_id_t			olt_id, 
           const PON_onu_id_t			onu_id,
		   const unsigned char			port_number,
		   CTC_STACK_multicast_vlan_t  *multicast_vlan);

PON_STATUS PON_CTC_STACK_get_multicast_vlan_management_object ( 
           const PON_olt_id_t			       olt_id, 
           const PON_onu_id_t			       onu_id,
		   const CTC_management_object_index_t management_object_index,
		   CTC_STACK_multicast_vlan_t         *multicast_vlan);


/* Get multicast vlan port for all ports
**
** This function retrieves ONU multicast VLAN configuration for all Ethernet ports
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      number_of_entries	: Number of valid entries in ports_info parameter
**      ports_info			: See CTC_STACK_multicast_vlan_ports_t for details.
**      or
**      management_object_info  : See CTC_STACK_multicast_vlan_management_object_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_multicast_vlan_all_port ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   unsigned char					 *number_of_entries,
		   CTC_STACK_multicast_vlan_ports_t   ports_info );

PON_STATUS PON_CTC_STACK_get_multicast_vlan_all_management_object ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char					             *number_of_entries,
		   CTC_STACK_multicast_vlan_management_object_t   management_object_info );

/* Set multicast control
**
** This function set multicast control.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      multicast_control	: See CTC_STACK_multicast_control_t for details.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_multicast_control ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const CTC_STACK_multicast_control_t   multicast_control );

/* Set multicast control - ipv6
**
** This function set multicast control.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      multicast_control	: See CTC_STACK_multicast_control_ipv6_ext_t for details.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_multicast_control_ipv6_ext ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const CTC_STACK_multicast_control_ipv6_ext_t   multicast_control );


/* Clear multicast control
**
** This function clears ONU multicast service control configuration
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_clear_multicast_control ( 
           const PON_olt_id_t   olt_id, 
           const PON_onu_id_t	onu_id );

/* Get multicast control port
**
** This function retrieves multicast control.
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      multicast_control		: See CTC_STACK_multicast_control_t for details.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_multicast_control ( 
           const PON_olt_id_t			   olt_id, 
           const PON_onu_id_t			   onu_id,
		   CTC_STACK_multicast_control_t  *multicast_control );

/* Get multicast control -ipv6
**
** This function retrieves multicast control.
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      multicast_control		: See CTC_STACK_multicast_control_ipv6_ext_t for details.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_multicast_control_ipv6_ext ( 
           const PON_olt_id_t			   olt_id, 
           const PON_onu_id_t			   onu_id,
		   CTC_STACK_multicast_control_ipv6_ext_t  *multicast_control );

/* Set multicast group number
**
** This function set multicast group number for a specific port.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**      group_num			: Allowed multicast group number that can be handled at the same time.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_multicast_group_num ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   const unsigned char	 group_num);

PON_STATUS PON_CTC_STACK_set_multicast_management_object_group_num ( 
           const PON_olt_id_t	               olt_id, 
           const PON_onu_id_t	               onu_id,
		   const CTC_management_object_index_t management_object_index,
		   const unsigned char	               group_num);

/* Get multicast group number
**
** This function retrieves multicast group number for a specific port.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**      group_num			: Allowed multicast group number that can be handled at the same time.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_multicast_group_num ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   unsigned char	    *group_num);

PON_STATUS PON_CTC_STACK_get_multicast_management_object_group_num ( 
           const PON_olt_id_t	               olt_id, 
           const PON_onu_id_t	               onu_id,
		   const CTC_management_object_index_t management_object_index,
		   unsigned char	                  *group_num);

/* Get multicast group number for all ports
**
** This function retrieves multicast group number for all ports.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in multicast_group parameter
**      multicast_group		: See CTC_STACK_multicast_ports_group_num_t for details.
**      or
**      management_object_group_num  : See CTC_STACK_multicast_management_object_group_num_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_multicast_all_port_group_num ( 
           const PON_olt_id_t					   olt_id, 
           const PON_onu_id_t					   onu_id,
		   unsigned char						  *number_of_entries,
		   CTC_STACK_multicast_ports_group_num_t   ports_group_num );

PON_STATUS PON_CTC_STACK_get_multicast_all_management_object_group_num ( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_multicast_management_object_group_num_t   management_object_group_num );


/* Get Phy admin state
**
** This function retrieves phy admin state for a specific port.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**      state				: ENABLE\DISABLE.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_phy_admin_state ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   bool				    *state );

PON_STATUS PON_CTC_STACK_get_management_object_phy_admin_state ( 
           const PON_olt_id_t	               olt_id, 
           const PON_onu_id_t	               onu_id,
		   const CTC_management_object_index_t management_object_index,
		   bool				                  *state );

/* Get Phy admin state for all ports
**
** This function retrieves phy admin state for for all ports.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports info parameter
**		ports_info			: See CTC_STACK_ethernet_ports_state_t for details
**      or
**      management_object_info  : See CTC_STACK_ethernet_management_object_state_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_all_port_phy_admin_state ( 
           const PON_olt_id_t				   olt_id, 
           const PON_onu_id_t				   onu_id,
		   unsigned char					  *number_of_entries,
		   CTC_STACK_ethernet_ports_state_t    ports_info );

PON_STATUS PON_CTC_STACK_get_all_management_object_phy_admin_state ( 
           const PON_olt_id_t				               olt_id, 
           const PON_onu_id_t				               onu_id,
		   unsigned char					              *number_of_entries,
		   CTC_STACK_ethernet_management_object_state_t    management_object_info );


/* Set phy admin control
**
** This function set phy admin control.
**
** Input Parameters:
**		olt_id	    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number : port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**      state		: phy admin control state. ENABLE\DISABLE.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_phy_admin_control ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const bool		     state);

PON_STATUS PON_CTC_STACK_set_management_object_phy_admin_control ( 
           const PON_olt_id_t	               olt_id, 
           const PON_onu_id_t	               onu_id,
		   const CTC_management_object_index_t management_object_index,
		   const bool		                   state);

/* Get Auto negotiation admin state
**
** This function retrieves phy admin state for a specific port.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**      state				: ENABLE\DISABLE.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_auto_negotiation_admin_state ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   bool				    *state );

PON_STATUS PON_CTC_STACK_get_management_object_auto_negotiation_admin_state ( 
           const PON_olt_id_t	               olt_id, 
           const PON_onu_id_t	               onu_id,
		   const CTC_management_object_index_t management_object_index,
		   bool				                  *state );

/* Get Auto negotiation admin state for all ports
**
** This function retrieves phy admin state for all ports.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports info parameter
**		ports_info			: See CTC_STACK_ethernet_ports_state_t for details
**      or
**      management_object_info  : See CTC_STACK_ethernet_management_object_state_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_auto_negotiation_all_ports_admin_state ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   unsigned char					 *number_of_entries,
		   CTC_STACK_ethernet_ports_state_t   ports_info );

PON_STATUS PON_CTC_STACK_get_auto_negotiation_all_management_object_admin_state ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char				              	 *number_of_entries,
		   CTC_STACK_ethernet_management_object_state_t   management_object_info );


/* Get Auto negotiation local technology abilities
**
** This function retrieves local technology abilities for a specific port.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**      abilities			: See CTC_STACK_auto_negotiation_technology_ability_t for details.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_auto_negotiation_local_technology_ability ( 
           const PON_olt_id_t								 olt_id, 
           const PON_onu_id_t								 onu_id,
		   const unsigned char								 port_number,
		   CTC_STACK_auto_negotiation_technology_ability_t  *abilities );

PON_STATUS PON_CTC_STACK_get_auto_negotiation_management_object_local_technology_ability ( 
           const PON_olt_id_t								 olt_id, 
           const PON_onu_id_t								 onu_id,
		   const CTC_management_object_index_t               management_object_index,
		   CTC_STACK_auto_negotiation_technology_ability_t  *abilities );


/* Get Auto negotiation local technology abilities for all ports
**
** This function retrieves local technology abilities for all ports.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports info parameter
**		ports_abilities		: See CTC_STACK_auto_negotiation_ports_technology_ability_t for details
**      or
**      management_object_abilities  : See CTC_STACK_auto_negotiation_management_object_technology_ability_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_auto_negotiation_all_ports_local_technology_ability ( 
           const PON_olt_id_t									   olt_id, 
           const PON_onu_id_t									   onu_id,
		   unsigned char										  *number_of_entries,
		   CTC_STACK_auto_negotiation_ports_technology_ability_t   ports_abilities );

PON_STATUS PON_CTC_STACK_get_auto_negotiation_all_management_object_local_technology_ability ( 
           const PON_olt_id_t									               olt_id, 
           const PON_onu_id_t									               onu_id,
		   unsigned char										              *number_of_entries,
		   CTC_STACK_auto_negotiation_management_object_technology_ability_t   management_object_abilities );


/* Get Auto negotiation advertised technology abilities
**
** This function retrieves advertised technology abilities for a specific port.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**      abilities			: See CTC_STACK_auto_negotiation_technology_ability_t for details.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_auto_negotiation_advertised_technology_ability ( 
           const PON_olt_id_t								 olt_id, 
           const PON_onu_id_t								 onu_id,
		   const unsigned char								 port_number,
		   CTC_STACK_auto_negotiation_technology_ability_t  *abilities );

PON_STATUS PON_CTC_STACK_get_auto_negotiation_management_object_advertised_technology_ability ( 
           const PON_olt_id_t								 olt_id, 
           const PON_onu_id_t								 onu_id,
		   const CTC_management_object_index_t               management_object_index,
		   CTC_STACK_auto_negotiation_technology_ability_t  *abilities );

/* Get Auto negotiation advertised technology abilities for all ports
**
** This function retrieves advertised technology abilities for all ports.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports info parameter
**		ports_abilities		: See CTC_STACK_auto_negotiation_ports_technology_ability_t for details
**      or
**      management_object_abilities  : See CTC_STACK_auto_negotiation_management_object_technology_ability_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_auto_negotiation_all_ports_advertised_technology_ability ( 
           const PON_olt_id_t									   olt_id, 
           const PON_onu_id_t									   onu_id,
		   unsigned char										  *number_of_entries,
		   CTC_STACK_auto_negotiation_ports_technology_ability_t   ports_abilities );

PON_STATUS PON_CTC_STACK_get_auto_negotiation_all_management_object_advertised_technology_ability ( 
           const PON_olt_id_t									               olt_id, 
           const PON_onu_id_t									               onu_id,
		   unsigned char										              *number_of_entries,
		   CTC_STACK_auto_negotiation_management_object_technology_ability_t   management_object_abilities );


;
/* Set auto negotiation restart auto config
**
** This function set auto negotiation restart auto config.
**
** Input Parameters:
**		olt_id	    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number : port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/


PON_STATUS PON_CTC_STACK_set_auto_negotiation_restart_auto_config ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number);

PON_STATUS PON_CTC_STACK_set_auto_negotiation_management_object_restart_auto_config ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index);

/* Set auto negotiation admin control
**
** This function set auto negotiation admin control.
**
** Input Parameters:
**		olt_id	    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number	: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**      state		: auto negotiation admin control state. ENABLE\DISABLE.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_auto_negotiation_admin_control ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   const bool			 state);

PON_STATUS PON_CTC_STACK_set_auto_negotiation_management_object_admin_control ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   const bool			                state);

/* Get FEC ability
**
** This function retrieves FEC ability.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      fec_ability				: FEC ability. See CTC_STACK_standard_FEC_ability_t for details
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_fec_ability ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   CTC_STACK_standard_FEC_ability_t  *fec_ability );



/* Get FEC mode
**
** This function retrieves FEC mode.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      fec_mode				: FEC mode. See CTC_STACK_standard_FEC_mode_t for details
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_fec_mode ( 
           const PON_olt_id_t			   olt_id, 
           const PON_onu_id_t			   onu_id,
		   CTC_STACK_standard_FEC_mode_t  *fec_mode );




/* Set FEC mode
**
** This function set FEC mode.
**
** Input Parameters:
**		olt_id	    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      fec_mode	: FEC mode. See CTC_STACK_standard_FEC_mode_t for details.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_fec_mode ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   const CTC_STACK_standard_FEC_mode_t  fec_mode);




/* Reset ONU
**
** This function reset a specific ONU.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_reset_onu ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id );



/* Get Ethernet link state
**
** This function retrieves the link operation state of an Ethernet port
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**		link_state			: Ethernet link operation state. See CTC_STACK_link_state_t for details
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_ethernet_link_state ( 
           const PON_olt_id_t	    olt_id, 
           const PON_onu_id_t	    onu_id,
		   const unsigned char	    port_number,
		   CTC_STACK_link_state_t  *link_state);

PON_STATUS PON_CTC_STACK_get_ethernet_management_object_link_state ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   CTC_STACK_link_state_t              *link_state);

/* Get Ethernet link state for all ports
**
** This function retrieves the link operation state of all Ethernet ports
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports_link_state
**		ports_link_state	: Ethernet link operation state structure. See CTC_STACK_ethernet_ports_link_state_t for details
**      or
**      management_object_link_state  : See CTC_STACK_ethernet_management_object_link_state_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/


PON_STATUS PON_CTC_STACK_get_ethernet_all_port_link_state ( 
           const PON_olt_id_t					   olt_id, 
           const PON_onu_id_t					   onu_id,
		   unsigned char						  *number_of_entries,
		   CTC_STACK_ethernet_ports_link_state_t   ports_link_state );

PON_STATUS PON_CTC_STACK_get_ethernet_all_management_object_link_state ( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_ethernet_management_object_link_state_t   management_object_link_state );


/* Set multicast Tag strip
**
** This function configures if ONU must strip the VLAN TAG of multicast traffic report or not
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**		tag_strip			: Disable / Enable tag strip 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_multicast_tag_strip ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const bool		     tag_strip);

PON_STATUS PON_CTC_STACK_set_multicast_management_object_tag_strip ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   const bool		                    tag_strip);


/* Get multicast Tag strip
**
** This function retrieves if ONU must strip the VLAN TAG of multicast traffic report or not
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**		tag_strip			: Disable / Enable tag strip 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_multicast_tag_strip ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   bool				    *tag_strip);

PON_STATUS PON_CTC_STACK_get_multicast_management_object_tag_strip ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   bool				                   *tag_strip);

/* Get multicast Tag strip for all ports
**
** This function retrieves if ONU must strip the VLAN TAG of multicast traffic report or not
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports_info
**		ports_info			: See CTC_STACK_multicast_ports_tag_strip_t for details
**      or
**      management_object_info  : See CTC_STACK_multicast_management_object_tag_strip_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_multicast_all_port_tag_strip ( 
           const PON_olt_id_t					   olt_id, 
           const PON_onu_id_t					   onu_id,
		   unsigned char						  *number_of_entries,
		   CTC_STACK_multicast_ports_tag_strip_t   ports_info );

PON_STATUS PON_CTC_STACK_get_multicast_all_management_object_tag_strip ( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_multicast_management_object_tag_strip_t   management_object_info );

/* Set multicast Tag oper
**
** This function configures if ONU should strip or translate the multicast VLAN TAG of multicast data and
** query frames of downstream multicast flow to the user IPTV VLAN

** Input Parameters:
**		olt_id	                 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	                 : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			     : port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**		tag_oper			     : Disable / Enable tag strip or translate tag 
**      multicast_vlan_switching : See CTC_STACK_multicast_vlan_switching_t for details
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_multicast_tag_oper ( 
           const PON_olt_id_t	                       olt_id, 
           const PON_onu_id_t	                       onu_id,
		   const unsigned char	                       port_number,
		   const CTC_STACK_tag_oper_t                  tag_oper,
           const CTC_STACK_multicast_vlan_switching_t  multicast_vlan_switching);

PON_STATUS PON_CTC_STACK_set_multicast_management_object_tag_oper ( 
           const PON_olt_id_t	                       olt_id, 
           const PON_onu_id_t	                       onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_tag_oper_t                  tag_oper,
           const CTC_STACK_multicast_vlan_switching_t  multicast_vlan_switching);


/* Get multicast Tag oper
**
** This function retrieves if ONU should strip or translate the multicast VLAN TAG of multicast data and
** query frames of downstream multicast flow to the user IPTV VLAN
**
** Input Parameters:
**		olt_id	                 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	                 : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			     : port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**		tag_oper			     : Disable / Enable tag strip or translate tag  
**      multicast_vlan_switching : See CTC_STACK_multicast_vlan_switching_t for details
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_multicast_tag_oper ( 
           const PON_olt_id_t	                  olt_id, 
           const PON_onu_id_t	                  onu_id,
		   const unsigned char	                  port_number,
		   CTC_STACK_tag_oper_t                  *tag_oper,
		   CTC_STACK_multicast_vlan_switching_t  *multicast_vlan_switching);

PON_STATUS PON_CTC_STACK_get_multicast_management_object_tag_oper ( 
           const PON_olt_id_t	                  olt_id, 
           const PON_onu_id_t	                  onu_id,
		   const CTC_management_object_index_t    management_object_index,
		   CTC_STACK_tag_oper_t                  *tag_oper,
		   CTC_STACK_multicast_vlan_switching_t  *multicast_vlan_switching);

/* Get multicast Tag oper for all ports
**
** This function retrieves if ONU should strip or translate the multicast VLAN TAG of multicast data and
** query frames of downstream multicast flow to the user IPTV VLAN
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports_info
**		ports_info			: See CTC_STACK_multicast_ports_tag_oper_t for details
**      or
**      management_object_info  : See CTC_STACK_multicast_management_object_tag_oper_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_multicast_all_port_tag_oper ( 
           const PON_olt_id_t					   olt_id, 
           const PON_onu_id_t					   onu_id,
		   unsigned char						  *number_of_entries,
		   CTC_STACK_multicast_ports_tag_oper_t    ports_info );

PON_STATUS PON_CTC_STACK_get_multicast_all_management_object_tag_oper ( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_multicast_management_object_tag_oper_t    management_object_info );


/* Set multicast switch protocol
**
** This function sets ONU multicast protocol
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		multicast_protocol	: multicast management protocol 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_multicast_switch ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   const CTC_STACK_multicast_protocol_t   multicast_protocol );



/* Get multicast switch protocol
**
** This function retrieves  ONU multicast protocol
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		multicast_protocol	: multicast management protocol 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_multicast_switch ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   CTC_STACK_multicast_protocol_t  *multicast_protocol );


PON_STATUS PON_CTC_STACK_set_statistic_state(
		   const PON_olt_id_t				olt_id,
           const PON_onu_id_t				onu_id,
           const CTC_STACK_statistic_state_t statistic_state
           );

PON_STATUS PON_CTC_STACK_get_statistic_state(
		   const PON_olt_id_t				olt_id,
           const PON_onu_id_t				onu_id,
           CTC_STACK_statistic_state_t *statistic_state
           );

PON_STATUS PON_CTC_STACK_set_ethport_statistic_state ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const CTC_STACK_statistic_state_t state);

PON_STATUS PON_CTC_STACK_get_ethport_statistic_state (
           const PON_olt_id_t	 olt_id,
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   CTC_STACK_statistic_state_t *state);

PON_STATUS PON_CTC_STACK_set_statistic_data(
		const PON_olt_id_t olt_id,
		const PON_onu_id_t onu_id,
		const unsigned long circle,
		const unsigned long time);

PON_STATUS PON_CTC_STACK_get_statistic_data(
		const PON_olt_id_t olt_id,
		const PON_onu_id_t onu_id,
		CTC_STACK_statistic_data_t *data);

PON_STATUS PON_CTC_STACK_set_ethport_statistic_data(
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number);

PON_STATUS PON_CTC_STACK_get_ethport_statistic_data(
		const PON_olt_id_t olt_id,
		const PON_onu_id_t onu_id,
		const unsigned char port_number,
		CTC_STACK_statistic_data_t *data);

PON_STATUS PON_CTC_STACK_get_ethport_statistic_history_data(
		const PON_olt_id_t olt_id,
		const PON_onu_id_t onu_id,
		const unsigned char port_number,
		CTC_STACK_statistic_data_t *data);

PON_STATUS PON_CTC_STACK_get_ethport_statistic_current_data(
		const PON_olt_id_t olt_id,
		const PON_onu_id_t onu_id,
		const unsigned char port_number,
		CTC_STACK_statistic_data_t *data);

PON_STATUS PON_CTC_STACK_set_multicast_management_object_tag_strip ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   const bool		                    tag_strip);


/* Get multicast Tag strip
**
** This function retrieves if ONU must strip the VLAN TAG of multicast traffic report or not
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**		tag_strip			: Disable / Enable tag strip 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_multicast_tag_strip ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   bool				    *tag_strip);

PON_STATUS PON_CTC_STACK_get_multicast_management_object_tag_strip ( 
           const PON_olt_id_t	                olt_id, 
           const PON_onu_id_t	                onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   bool				                   *tag_strip);

/* Get multicast Tag strip for all ports
**
** This function retrieves if ONU must strip the VLAN TAG of multicast traffic report or not
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports_info
**		ports_info			: See CTC_STACK_multicast_ports_tag_strip_t for details
**      or
**      management_object_info  : See CTC_STACK_multicast_management_object_tag_strip_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_multicast_all_port_tag_strip ( 
           const PON_olt_id_t					   olt_id, 
           const PON_onu_id_t					   onu_id,
		   unsigned char						  *number_of_entries,
		   CTC_STACK_multicast_ports_tag_strip_t   ports_info );

PON_STATUS PON_CTC_STACK_get_multicast_all_management_object_tag_strip ( 
           const PON_olt_id_t					               olt_id, 
           const PON_onu_id_t					               onu_id,
		   unsigned char						              *number_of_entries,
		   CTC_STACK_multicast_management_object_tag_strip_t   management_object_info );



/* Set fast leave admin control
**
** This function set ONU multicast fast leave admin control
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		fast_leave_admin_state	: fast leave admin state
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_fast_leave_admin_control ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   CTC_STACK_fast_leave_admin_state_t     fast_leave_admin_state );

		   
		   
/* Get fast leave admin state
**
** This function retrieves ONU multicast fast leave admin state
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		fast_leave_admin_state	: fast leave admin state 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_fast_leave_admin_state ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   CTC_STACK_fast_leave_admin_state_t	*fast_leave_admin_state );



/* Get fast leave ability
**
** This function retrieves ONU multicast fast leave ability
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		fast_leave_ability		: fast leave ability
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_fast_leave_ability ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   CTC_STACK_fast_leave_ability_t		*fast_leave_ability );



/* Get all ethernet port ds rate limiting
**
** This function retrieves all ONU ethernet ports downstream rate limiting configuration
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries		: Number of valid entries in ports info parameter
**		ports_ds_rate_limiting	: See CTC_STACK_ethernet_ports_ds_rate_limiting_t for details
**      or
**      management_object_ds_rate_limiting  : See CTC_STACK_ethernet_management_object_ds_rate_limiting_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_all_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   unsigned char								*number_of_entries,
		   CTC_STACK_ethernet_ports_ds_rate_limiting_t	ports_ds_rate_limiting );

PON_STATUS PON_CTC_STACK_get_all_ethernet_management_object_ds_rate_limiting ( 
           const PON_olt_id_t							            olt_id, 
           const PON_onu_id_t							            onu_id,
		   unsigned char							               *number_of_entries,
		   CTC_STACK_ethernet_management_object_ds_rate_limiting_t	management_object_ds_rate_limiting );

/* Get ethernet port ds rate limiting
**
** This function retrieves ONU ethernet port downstream rate limiting configuration
**
** Input Parameters:
**		olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**		rate_limiting_entry	: See CTC_STACK_ethernet_port_ds_rate_limiting_entry_t for details
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t								olt_id, 
           const PON_onu_id_t								onu_id,
		   const unsigned char								port_number,
		   CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting );

PON_STATUS PON_CTC_STACK_get_ethernet_management_object_ds_rate_limiting ( 
           const PON_olt_id_t								olt_id, 
           const PON_onu_id_t								onu_id,
		   const CTC_management_object_index_t              management_object_index,
		   CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting );


/* Set ethernet port ds rate limiting
**
** This function set ONU ethernet port downstream rate limiting configuration
**
** Input Parameters:
**		olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**		rate_limiting_entry	: See CTC_STACK_ethernet_port_ds_rate_limiting_entry_t for details
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t										olt_id, 
           const PON_onu_id_t										onu_id,
		   const unsigned char										port_number,
		   const CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting);

PON_STATUS PON_CTC_STACK_set_ethernet_management_object_ds_rate_limiting ( 
           const PON_olt_id_t										olt_id, 
           const PON_onu_id_t										onu_id,
		   const CTC_management_object_index_t                      management_object_index,
		   const CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting);

/* Set QinQ VLAN
**
** This function sets Ethernet port QinQ VLAN configuration     
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number				: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**      port_configuration		: See CTC_STACK_port_qinq_configuration_t for details.
**
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_qinq_port_configuration ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const unsigned char						   port_number,
		   const CTC_STACK_port_qinq_configuration_t   port_configuration);

PON_STATUS PON_CTC_STACK_set_qinq_management_object_configuration ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_port_qinq_configuration_t   port_configuration);

/* Get QinQ VLAN
**
** This function gets Ethernet port QinQ VLAN configuration 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number				: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**      port_configuration		: See CTC_STACK_port_qinq_configuration_t for details.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_qinq_port_configuration ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const unsigned char					 port_number,
		   CTC_STACK_port_qinq_configuration_t  *port_configuration);

PON_STATUS PON_CTC_STACK_get_qinq_management_object_configuration ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const CTC_management_object_index_t   management_object_index,
		   CTC_STACK_port_qinq_configuration_t  *port_configuration);

/* Get QinQ VLAN for all ports
**
** This function gets Ethernet port QinQ VLAN configuration for all Ethernet ports of specified ONU
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      number_of_entries		: Number of valid entries in ports info 
**      ports_info				: See CTC_STACK_qinq_configuration_ports_t for details.
**      or
**      management_object_info  : See CTC_STACK_qinq_configuration_management_object_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_qinq_vlan_all_port_configuration ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   unsigned char						 *number_of_entries,
		   CTC_STACK_qinq_configuration_ports_t   ports_info );  

PON_STATUS PON_CTC_STACK_get_qinq_vlan_all_management_object_configuration ( 
           const PON_olt_id_t					              olt_id, 
           const PON_onu_id_t					              onu_id,
		   unsigned char					               	 *number_of_entries,
		   CTC_STACK_qinq_configuration_management_object_t   management_object_info ); 

/* Get discovery state
**
** Get extended OAM discovery state of an ONU.
** The discovery process may succeed or fail.
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		    onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**			state			    : One of CTC_STACK_discovery_state_t enum value.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_discovery_state(           
					const PON_olt_id_t			olt_id, 
					const PON_onu_id_t			onu_id,
					CTC_STACK_discovery_state_t	*state);


/* Update onu firmware
**
** This function updates onu firmware.
**
** Input Parameters:
**		olt_id	              : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				  : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		onu_firmware	      : See CTC_STACK_binary_t for details
**      file_name             : string of Vendor.ONU type.firmware version.date
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/ 
PON_STATUS PON_CTC_STACK_update_onu_firmware ( 
                 const PON_olt_id_t          olt_id, 
                 const PON_onu_id_t          onu_id,
                 const CTC_STACK_binary_t   *onu_firmware,
                 const char                 *file_name );

/* active onu firmware
**
** This function active onu firmware.
**
** Input Parameters:
**		olt_id	              : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				  : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/ 
PON_STATUS PON_CTC_STACK_activate_image(
            const PON_olt_id_t                    olt_id, 
            const PON_onu_id_t                    onu_id);

/* commit onu firmware
**
** This function commit onu firmware.
**
** Input Parameters:
**		olt_id	              : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				  : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/ 
PON_STATUS  PON_CTC_STACK_commit_image(
            const PON_olt_id_t                    olt_id, 
            const PON_onu_id_t                    onu_id);


#if 0
/* Get auth LOID data table
**
** Get authentication ONU_ID+Password data table
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**          num_of_entries      : num of entries in auth LOID table
**
** Output Parameters:
**			loid_data_table		: See CTC_STACK_auth_loid_data_t for details.
**          num_of_entries      : num of entries in auth LOID table
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_auth_loid_data_table(           
					const PON_olt_id_t			         olt_id, 
					CTC_STACK_auth_loid_data_t          *loid_data_table,
                    unsigned char                       *num_of_entries);


/* Add auth LOID data 
**
** Add authentication ONU_ID+Password data
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			loid_data		    : See CTC_STACK_auth_loid_data_t for details.
**
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_add_auth_loid_data(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_loid_data_t     loid_data);


/* Remove auth LOID data 
**
** Remove authentication ONU_ID+Password data
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			loid_data		    : See CTC_STACK_auth_loid_data_t for details.
**
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_remove_auth_loid_data(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_loid_data_t     loid_data);


/* Set auth mode 
**
** Set authentication mode
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			ctc_auth_mode       : See CTC_STACK_auth_mode_t for details.
**
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_auth_mode(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_mode_t          ctc_auth_mode);


/* Get auth mode 
**
** Get authentication mode
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**			ctc_auth_mode       : See CTC_STACK_auth_mode_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_auth_mode(           
					const PON_olt_id_t			         olt_id, 
					CTC_STACK_auth_mode_t               *ctc_auth_mode);
#endif

/* Get optical transceiver diagnosis
**
** This function retrieves ONU optical transceiver diagnosis
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		optical_transceiver_diagnosis	: See CTC_STACK_optical_transceiver_diagnosis_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_optical_transceiver_diagnosis ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis );



/* Set service sla
**
** This function sets ONU service SLA
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		activate				: Activate/Deactivate service SLA.
**		service_sla				: See CTC_STACK_service_sla_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_service_sla ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const bool									activate,
		   const CTC_STACK_service_sla_t				*service_sla );

		   
/* Get service sla
**
** This function retrieves ONU service SLA
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		activate				: Is service SLA activated or deactivated.
**		service_sla				: See CTC_STACK_service_sla_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_service_sla ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   bool											*activate,
		   CTC_STACK_service_sla_t						*service_sla );


/* Set alarm admin state
**
** This function sets ONU alarm admin state
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details 
**		alarm_id				: See CTC_STACK_alarm_id_t for details 
**		enable					: Enable/Disable alarm.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_alarm_admin_state ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t   			alarm_id,
			const bool								enable);


/* Get alarm admin state
**
** This function retrieves ONU alarm admin state
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details
**		alarm_id				: See CTC_STACK_alarm_id_t for details 
**		number_of_alarms		: Size of alarms_state array
**
** Output Parameters:
**		number_of_alarms		: Actual number of elements in alarms_state array
**		alarms_state			: Array of CTC_STACK_alarm_admin_state_t
**									See CTC_STACK_alarm_admin_state_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_alarm_admin_state ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			unsigned short							*number_of_alarms,
			CTC_STACK_alarm_admin_state_t			alarms_state[] );


/* Set alarm threshold
**
** This function sets ONU alarm threshold
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details 
**		alarm_id				: See CTC_STACK_alarm_id_t for details 
**		alarm_threshold		: Alarm threshold
**		clear_threshold		: Alarm clear threshold
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_alarm_threshold ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			const unsigned long						alarm_threshold,
			const unsigned long						clear_threshold );


/* Get alarm threshold
**
** This function retrieves ONU alarm admin state
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details
**		alarm_id				: See CTC_STACK_alarm_id_t for details 
**		number_of_alarms		: Size of alarms_threshold array
**
** Output Parameters:
**		number_of_alarms		: Actual number of elements in alarms_threshold array
**		alarms_state			: Array of CTC_STACK_alarm_threshold_t
**									See CTC_STACK_alarm_threshold_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_alarm_threshold ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			unsigned short							*number_of_alarms,
			CTC_STACK_alarm_threshold_t				alarms_threshold[] );

			
/* Reset card
**
** This function reset a specific card.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		farme_number		: Card frame number
**		slot_number		: Slot frame number
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_reset_card ( 
			const PON_olt_id_t					olt_id, 
			const PON_onu_id_t					onu_id,
			unsigned char						frame_number,
			unsigned char						slot_number );


/* Get onu version
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		onu_version         : ONU version.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_onu_version(
                                    const PON_olt_id_t		   olt_id, 
                                    const PON_onu_id_t		   onu_id,
                                    unsigned char             *onu_version );

/* Set llid queue config (in case of single LLID used)
**
** This function sets llid queue config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		llid_queue_config   	: See CTC_STACK_llid_queue_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_single_llid_queue_config ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_llid_queue_config_t		    llid_queue_config );

		   
/* Get llid queue config (in case of single LLID used)
**
** This function retrieves llid queue config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		llid_queue_config		: See CTC_STACK_llid_queue_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_single_llid_queue_config ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_llid_queue_config_t			   *llid_queue_config );

/* Set llid queue config (in case of Multi LLID used)
**
** This function sets llid queue config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		llid_queue_config   	: See CTC_STACK_llid_queue_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_llid_queue_config ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_llid_queue_config_t		    llid_queue_config );

		   
/* Get llid queue config (in case of Multi LLID used)
**
** This function retrieves llid queue config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		llid_queue_config		: See CTC_STACK_llid_queue_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_llid_queue_config ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_llid_queue_config_t			   *llid_queue_config );

/* Set multi llid admin control
**
** This function set multi llid admin control
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		num_of_llid_activated	: number of llid activated
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_multi_llid_admin_control ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   const unsigned long                    num_of_llid_activated);


/* Authentication response ONU_ID+Password handler
**
** An event of ONU authentication response ONU_ID+Password.
** OLT needs to confirm ONU loid_data.
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**	    	onu_id			    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**          loid_data           : See CTC_STACK_auth_loid_data_t for details 
**
** Output Parameters:
**		    auth_success     	: true/false - authentication succeed/failed   
**		    failure_type     	: if authentication failed, see CTC_STACK_auth_failure_type_t for details 
**
** Return codes:
**				Return codes are not specified for an event
**
*/

typedef void (*CTC_STACK_auth_response_loid_handler_t) 
				  ( const PON_olt_id_t					olt_id, 
					const PON_onu_id_t					onu_id, 
                    const CTC_STACK_auth_loid_data_t    loid_data,
                    bool                               *auth_success,
					CTC_STACK_auth_failure_type_t      *failure_type );


/* Authorized ONU_ID+Password handler
**
** An event of OLT authorized ONU_ID+Password, meaning if ONU loid_data was confirmed.
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**	    	onu_id			    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**          loid_data           : See CTC_STACK_auth_loid_data_t for details 
**		    auth_success     	: true/false - authentication succeed/failed   
**		    failure_type     	: if authentication failed, see CTC_STACK_auth_failure_type_t for details 
**
** Return codes:
**				Return codes are not specified for an event
**
*/

typedef void (*CTC_STACK_authorized_loid_handler_t) 
				  ( const PON_olt_id_t					olt_id, 
					const PON_onu_id_t					onu_id, 
                    const CTC_STACK_auth_loid_data_t    loid_data,
                    const bool                          auth_success,
					const CTC_STACK_auth_failure_type_t failure_type );


/* Event Notification handler
**
** An event of Organization Specific Event Notification.
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**	    	onu_id			    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**          event_value         : See CTC_STACK_event_value_t for details 
**
** Return codes:
**				Return codes are not specified for an event
**
*/

typedef void (*CTC_STACK_event_notification_handler_t) 
				  ( const PON_olt_id_t					olt_id, 
					const PON_onu_id_t					onu_id, 
					const CTC_STACK_event_value_t       event_value );


/* Get voip iad info 
**
** This function retrieves voip iad info
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		iad_info		        : See CTC_STACK_voip_iad_info_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_voip_iad_info ( 
			const PON_olt_id_t						  olt_id, 
			const PON_onu_id_t						  onu_id,
			CTC_STACK_voip_iad_info_t	     	     *iad_info);


/* Set voip global param conf
**
** This function sets voip global param conf
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		global_param     		: See CTC_STACK_voip_global_param_conf_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_voip_global_param_conf ( 
			const PON_olt_id_t						  olt_id, 
			const PON_onu_id_t						  onu_id,
			const CTC_STACK_voip_global_param_conf_t  global_param);


/* Get voip global param conf
**
** This function retrieves voip global param conf
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		global_param 		: See CTC_STACK_voip_global_param_conf_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_voip_global_param_conf ( 
			const PON_olt_id_t						    olt_id, 
			const PON_onu_id_t						    onu_id,
			CTC_STACK_voip_global_param_conf_t         *global_param);


/* Set h248 param config
**
** This function sets h248 param config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		h248_param      		: See CTC_STACK_h248_param_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_h248_param_config ( 
			const PON_olt_id_t						  olt_id, 
			const PON_onu_id_t						  onu_id,
			const CTC_STACK_h248_param_config_t       h248_param);


/* Get h248 param config
**
** This function retrieves h248 param config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		h248_param  		    : See CTC_STACK_h248_param_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_h248_param_config ( 
			const PON_olt_id_t						  olt_id, 
			const PON_onu_id_t						  onu_id,
			CTC_STACK_h248_param_config_t    	     *h248_param);


/* Set h248 user tid config
**
** This function sets h248 user tid config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details 
**		h248_user_tid       	: See CTC_STACK_h248_user_tid_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_h248_user_tid_config ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_h248_user_tid_config_t   user_tid_config);


/* Get h248 user tid config
**
** This function retrieves h248 user tid config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details 
**
** Output Parameters:
**      number_of_entries       : Number of valid entries in h248_user_tid_array
**		h248_user_tid       	: See CTC_STACK_h248_user_tid_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_h248_user_tid_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
           const CTC_management_object_index_t		    management_object_index,
     	    unsigned char					           *number_of_entries,
		   CTC_STACK_h248_user_tid_config_array_t    	       *h248_user_tid_array);

/* Set H248 RTP TID config
**
** This function sets H248 RTP TID config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		h248_rtp_tid_config            	: See CTC_STACK_h248_rtp_tid_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_h248_rtp_tid_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_h248_rtp_tid_config_t       h248_rtp_tid_config);

/* Get H248 RTP TID info
**
** This function retrieves H248 RTP TID info
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		h248_rtp_tid_info     	: See CTC_STACK_h248_rtp_tid_info_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_h248_rtp_tid_info( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_h248_rtp_tid_info_t    	       *h248_rtp_tid_info);

/* Set sip param config
**
** This function sets sip param config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		sip_param            	: See CTC_STACK_sip_param_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_sip_param_config ( 
			const PON_olt_id_t						  olt_id, 
			const PON_onu_id_t						  onu_id,
			const CTC_STACK_sip_param_config_t        sip_param);


/* Get sip param config
**
** This function retrieves sip param config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		sip_param     	: See CTC_STACK_sip_param_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_sip_param_config ( 
			const PON_olt_id_t						  olt_id, 
			const PON_onu_id_t						  onu_id,
			CTC_STACK_sip_param_config_t	         *sip_param);


/* Set sip user param config
**
** This function sets sip user param config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details 
**		sip_user_param      	: See CTC_STACK_sip_user_param_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_sip_user_param_config ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_sip_user_param_config_t   sip_user_param);


/* Get sip user param config
**
** This function retrieves sip user param config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details 
**
** Output Parameters:
**      number_of_entries       : Number of valid entries in sip_user_param_array
**		sip_user_param_array       	: See CTC_STACK_sip_user_param_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_sip_user_param_config(
                     const PON_olt_id_t						    olt_id, 
			const PON_onu_id_t						    onu_id,
			const CTC_management_object_index_t		    management_object_index,
     	    unsigned char					           *number_of_entries,
           CTC_STACK_sip_user_param_config_array_t       *sip_user_param_array);


/* Set voip fax config
**
** This function sets voip fax config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		voip_fax            	: See CTC_STACK_voip_fax_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_voip_fax_config ( 
			const PON_olt_id_t						  olt_id, 
			const PON_onu_id_t						  onu_id,
			const CTC_STACK_voip_fax_config_t         voip_fax);


/* Get voip fax config
**
** This function retrieves voip fax config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		voip_fax      	        : See CTC_STACK_voip_fax_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_voip_fax_config ( 
			const PON_olt_id_t						  olt_id, 
			const PON_onu_id_t						  onu_id,
			CTC_STACK_voip_fax_config_t	             *voip_fax);


/* Get voip iad oper status
**
** This function retrieves voip iad oper status
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		iad_oper_status         : See CTC_STACK_voip_iad_oper_status_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_voip_iad_oper_status ( 
			const PON_olt_id_t						  olt_id, 
			const PON_onu_id_t						  onu_id,
			CTC_STACK_voip_iad_oper_status_t	     *iad_oper_status);


/* Get voip port userif status
**
** This function retrieves voip port userif status
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details 
**
** Output Parameters:
**      number_of_entries       : Number of valid entries in pots_status_array
**		pots_status_array    	: See CTC_STACK_voip_pots_status_array_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_voip_pots_status ( 
			const PON_olt_id_t						  olt_id, 
			const PON_onu_id_t						  onu_id,
			const CTC_management_object_index_t		  management_object_index,
     	    unsigned char					         *number_of_entries,
			CTC_STACK_voip_pots_status_array_t     *pots_status_array);


/* voip iad operation
**
** This function resets voip iad
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		operation_type      	    : See CTC_STACK_operation_type_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_voip_iad_operation ( 
			const PON_olt_id_t						  olt_id, 
			const PON_onu_id_t						  onu_id,
			const CTC_STACK_operation_type_t              operation_type);

/* Set SIP digit map
**
** This function sets SIP digit map
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		sip_digit_map      	    : See CTC_STACK_SIP_digit_map_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_sip_digit_map( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_SIP_digit_map_t           sip_digit_map);

/* Get ONU capabilities-2
**
** This function retrieves ONU main HW capabilities-2
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      onu_capabilities	: See CTC_STACK_onu_capabilities_2_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_onu_capabilities_2 ( 
           const PON_olt_id_t			  olt_id, 
           const PON_onu_id_t			  onu_id,
		   CTC_STACK_onu_capabilities_2_t  *onu_capabilities );

/* Get ONU capabilities-3
**
** This function retrieves ONU main HW capabilities-3
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      onu_capabilities	: See CTC_STACK_onu_capabilities_3_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_onu_capabilities_3 ( 
           const PON_olt_id_t			  olt_id, 
           const PON_onu_id_t			  onu_id,
		   CTC_STACK_onu_capabilities_3_t  *onu_capabilities );

/* set mdu/mtu management global parameter 
**
** This function set mdu/mtu management global parameter 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		CTC_STACK_mxu_mng_global_parameter_config_t      	    : See CTC_STACK_mxu_mng_global_parameter_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_mxu_mng_global_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_mxu_mng_global_parameter_config_t  parameter);

/* set mdu/mtu management global parameter - ipv6 extend API
**
** This function set mdu/mtu management global parameter 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t      	    : See CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_mxu_mng_global_parameter_config_ipv6_ext ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t  parameter);

/* get mdu/mtu management global parameter 
**
** This function set mdu/mtu management global parameter 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		CTC_STACK_mxu_mng_global_parameter_config_t      	    : See CTC_STACK_mxu_mng_global_parameter_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_mxu_mng_global_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			CTC_STACK_mxu_mng_global_parameter_config_t*  parameter);

/* get mdu/mtu management global parameter -ipv6 extend API
**
** This function set mdu/mtu management global parameter 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t      	    : See CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_mxu_mng_global_parameter_config_ipv6_ext ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t*  parameter);

/* set mdu/mtu management snmp parameter 
**
** This function set mdu/mtu management snmp parameter 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		CTC_STACK_mxu_mng_snmp_parameter_config_t      	    : See CTC_STACK_mxu_mng_snmp_parameter_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_mxu_mng_snmp_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_mxu_mng_snmp_parameter_config_t  parameter);

/* set mdu/mtu management snmp parameter -ipv6 extend API
**
** This function set mdu/mtu management snmp parameter 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t      	    : See CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_mxu_mng_snmp_parameter_config_ipv6_ext ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t  parameter);

/* get mdu/mtu management snmp parameter 
**
** This function set mdu/mtu management snmp parameter 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		CTC_STACK_mxu_mng_snmp_parameter_config_t      	    : See CTC_STACK_mxu_mng_snmp_parameter_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_mxu_mng_snmp_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			CTC_STACK_mxu_mng_snmp_parameter_config_t*  parameter);


/* get mdu/mtu management snmp parameter -ipv6 extend API
**
** This function set mdu/mtu management snmp parameter
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t      	    : See CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_mxu_mng_snmp_parameter_config_ipv6_ext ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t*  parameter);

/* Set port loop detect
**
** This function sets loop detect activation status for an Ethernet port or a DSL port of a specified ONU
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
**      loop_detect	: Enable or disable loop detect for the port.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_management_object_port_loop_detect ( 
           const PON_olt_id_t	                    olt_id, 
           const PON_onu_id_t	                    onu_id,
		   const CTC_management_object_index_t		management_object_index,
		   const unsigned long			                    loop_detect);

/* Get port loop detect
**
** This function retrieves loop detect activation status for an Ethernet port or a DSL port of a specified ONU
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**      or
**      management_object_index : See CTC_management_object_index_t for details.
**
** Output Parameters:
**		loop_detect	: Is loop detect enabled for the port
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_port_management_object_loop_detect ( 
           const PON_olt_id_t	                    olt_id, 
           const PON_onu_id_t	                    onu_id,
		   const CTC_management_object_index_t		management_object_index,
		   unsigned long				                       *loop_detect);
/* Get port loop detect for all ports
**
** This function retrieves loop detect activation status for an Ethernet port or a DSL port of a specified ONU
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports info parameter
**		ports_info			: Ethernet ports or DSL ports loop detect info structure. See CTC_STACK_ports_loop_detect_t for details
**      or
**      management_object_info  : See CTC_STACK_management_object_port_loop_detect_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_all_management_object_port_loop_detect ( 
           const PON_olt_id_t				              olt_id, 
           const PON_onu_id_t				              onu_id,
		   unsigned char					             *number_of_entries,
		   CTC_STACK_management_object_port_loop_detect_t   management_object_info );


/* set ONU holdover state 
**
** This function set ONU holdover state 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		CTC_STACK_holdover_state_t      	    : See CTC_STACK_holdover_state_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_holdover_state ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_holdover_state_t  parameter);

/* get ONU holdover state  
**
** This function get ONU holdover state 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		CTC_STACK_holdover_state_t      	    : See CTC_STACK_holdover_state_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_holdover_state ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			CTC_STACK_holdover_state_t*  parameter);


/* set ONU Active PON_IF state 
**
** This function set ONU Active PON_IF state 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		CTC_STACK_active_pon_if_state_t      	    : See CTC_STACK_active_pon_if_state_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_active_pon_if_state ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_active_pon_if_state_t  parameter);

/* get ONU Active PON_IF state  
**
** This function get ONU Active PON_IF state 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		CTC_STACK_active_pon_if_state_t      	    : See CTC_STACK_active_pon_if_state_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_active_pon_if_state ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			CTC_STACK_active_pon_if_state_t*  parameter);

/* Set redundancy database
**
** Update discovery_db, authentication_db, encryption_db.
**
** Input Parameters:
**			olt_id		        : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		    onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		    redundancy_db       : See CTC_STACK_redundancy_database_t for details 
**
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_redundancy_database(           
					const PON_olt_id_t			           olt_id,
					const PON_onu_id_t			           onu_id,
                    const CTC_STACK_redundancy_database_t  redundancy_db);

/* Get redundancy database
**
** Get discovery_db, authentication_db, encryption_db.
**
** Input Parameters:
**			olt_id		        : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		    onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		    redundancy_db       : See CTC_STACK_redundancy_database_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_redundancy_database(           
					const PON_olt_id_t			           olt_id,
					const PON_onu_id_t			           onu_id,
                    CTC_STACK_redundancy_database_t       *redundancy_db);

/* Get redundancy information
**
** get information from discovery_db, authentication_db, encryption_db.
**
** Input Parameters:
**			info_type		: information type, see CTC_STACK_rdn_info_type_t for details
**			olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**                 max_onu_entry    : max onu entry to be retrieved
**		       info_data_buf	: pointer to the information data block
**                 info_buf_len    : data buffer length, minimum requirement is 
**                 info_length      : length of the valid data in data block
**                 fragment         : true--more info_data block needed for remaining data
**                 
**
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_redundancy_info(           
                            CTC_STACK_rdn_info_type_t   info_type,    
                            const PON_olt_id_t   olt_id,
                            const short int   max_onu_entry,
                            char*  info_data_buf,
                            int   info_buf_len,
                            int*  info_length,
                            bool*  fragment);

/* Set redundancy information
**
** set information from discovery_db, authentication_db, encryption_db.
**
** Input Parameters:
**			info_type		: information type, see CTC_STACK_rdn_info_type_t for details
**			olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		       info_data		: pointer to the information data block
**                 info_length      : length of the valid data in data block
**                 
**
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_set_redundancy_info (
                        CTC_STACK_rdn_info_type_t info_type,
                        const PON_olt_id_t   olt_id,
                        char*   info_data,  
                        int     info_length);

/* Synchronize encryption DB information into OLT
**
** This function synchronizes the encrption DB information into OLT
**
** Input Parameters:
**			olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			onu_id		: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**                 
**
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_synchronize_encryption_db_to_olt(const PON_olt_id_t  olt_id, const PON_onu_id_t  onu_id);

/* Update entire redundancy information
**
** set information from discovery_db, authentication_db, encryption_db.
**
** Input Parameters:
**			info_type		: information type, see CTC_STACK_rdn_info_type_t for details
**			master_olt	: master OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		       slave_olt		: slave OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**                 
**
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_update_redundancy_entire_info  (
                                const PON_olt_id_t   master_olt,
                                const PON_olt_id_t   slave_olt,
                                CTC_STACK_rdn_info_type_t info_type);


/* Event encryption key value handler
**
** Update encryption event + the key value 
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**	    	onu_id			    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**          encryption_state    : See CTC_STACK_encryption_state_t for details 
**          encryption_key      : See PON_encryption_key_t for details 
**          key_index           : See PON_encryption_key_index_t for details 
**
** Return codes:
**				Return codes are not specified for an event
**
*/
typedef void (*CTC_STACK_encryption_key_value_handler_t) 
				  ( const PON_olt_id_t					olt_id, 
					const PON_onu_id_t					onu_id, 
					const CTC_STACK_encryption_state_t  encryption_state,
					const PON_encryption_key_t          encryption_key,
                    const PON_encryption_key_index_t    key_index);


/* Event update fec mode handler
**
** Update the fec mode 
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**	    	onu_id			    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**          fec_mode            : See CTC_STACK_standard_FEC_mode_t for details 
**
** Return codes:
**				Return codes are not specified for an event
**
*/
typedef void (*CTC_STACK_update_fec_mode_handler_t) 
				  ( const PON_olt_id_t					olt_id, 
					const PON_onu_id_t					onu_id, 
					const CTC_STACK_standard_FEC_mode_t fec_mode);


/* ONU TX power supply control
**
** This function controls the ONU TX power supply
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		parameter      	        : See CTC_STACK_onu_tx_power_supply_control_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_onu_tx_power_supply_control ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id,
           const CTC_STACK_onu_tx_power_supply_control_t parameter,
           const bool               broadcast);

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

#endif /* _CTC_STACK_EXPO_H__ */


