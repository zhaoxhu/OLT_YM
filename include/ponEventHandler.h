/**************************************************************
*
*    PonEventHandler.h -- PON event/alarm handler General header
*
*  
*    Copyright (c)  2006.4 , GW Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	     |    Change			     	|    Author	  
*   ---------|-----------|---------------------|------------
*	1.00	  | 28/04/2006 |   Creation				| chen fj
*
***************************************************************/

#ifndef _PONEVENTHANDLER_H
#define _PONEVENTHANDLER_H

/*#include "../../Common_components/PAS/PAS_expo.h"*/

#define ACTIVATE_AUTO_AUTHORIZATION   TRUE /* TRUE or FALSE */ 
#define ACTIVATE_OLT_INTERNAL_DBA     TRUE /* TRUE or FALSE */ 
#define AUTHENTICATION_802_1_X		  FALSE /* TRUE OR FALSE */


typedef struct  PonFileLoadCompleted{
	short int olt_id;
	PON_binary_type_t  type;
	short int  return_code ;
}PonFileLoadCompleted_S;

typedef struct  PonAlarmInfo{
	short int	    olt_id; 
	short int	    alarm_source_id; 
	PON_alarm_t   alarm_type; 
	short int	    alarm_parameter; 
	unsigned char alarm_data[64];
}PonAlarmInfo_S;

typedef struct OnuRegisterDenied{
	short int olt_id;
	mac_address_t Onu_MacAddr;
}OnuRegisterDenied_S;

typedef struct PonResetInfo{
	short int olt_id;
	PON_olt_reset_code_t  code;
}PonResetInfo_S;

typedef struct PonSwitchInfo{
	short int olt_id;
	short int event_id;
	short int event_code;
	short int new_status;
	unsigned short event_source;
	unsigned short slot_source;
	unsigned short event_seq;
	unsigned short event_flags;
}PonSwitchInfo_S;

/*Begin:for onu swap by jinhl@2013-02-22*/
typedef struct OnuSwitchInfo{
	short int olt_id;
	short int event_id;
	PON_redundancy_msg_olt_failure_info_t fail_info;
	int event_source;
	unsigned short event_seq;
	unsigned short event_flags;
}OnuSwitchInfo_S;
/*End:for onu swap by jinhl@2013-02-22*/
typedef struct  OamMsgRecv{
	short int   olt_id;
	PON_olt_physical_port_t    port;
	PON_frame_type_t  type; 
	short int  llid;
	short int   length;
	unsigned char  *content;
}OamMsgRecv_S;

/* A function used for check if packet is ARP packet */
extern bool Is_ARP_packet(const int packet_size, const char *packet);


/* AL1 function used for handling ethernet frame, this function is used to extend OAM */
extern short int  Ethernet_frame_received_handler (
									const short int				   olt_id,
									const PON_olt_physical_port_t    port,
									const PON_frame_type_t		   type, 
									const short int				   llid,
									const short int				   length, 
									const void					  *content );

extern short int Ethernet_frame_received_handler_1 (
									const short int				   olt_id,
									const PON_olt_physical_port_t    port,
									const PON_frame_type_t		   type, 
									const short int				   llid,
									const short int				   length, 
									const void					  *content );

extern PON_STATUS  PAS_send_frame( const PON_olt_id_t  					     olt_id, 
						   const short int							 length, 
						   const PON_sent_frame_destination_port_t   destination_port,
						   const PON_llid_t							 llid,
						   const short int							 broadcast_llid,
						   const void								*content );

/* Send OAM frame
**
** Send OAM organization specific 
**
** Input Parameters:
**      olt_id        : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid          : LLID, Range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**      oam_code      : See OAM_code_t for details
**      length        : Frame length in bytes (excluding Preamble, SFD, and FCS), 
**						range: 10 - 1464 in case of information, 42 - 1496 in case of vendor extension
**      content       : Pointer to the first byte of the frame
**
** Return codes:
**          All the defined         PAS_* return codes

extern PON_STATUS PAS_test_send_oam(      
                  const PON_olt_id_t             olt_id,
                  const PON_llid_t               llid,
                  const PON_oam_code_t 	         oam_code,
                  const unsigned short           length,
                  const unsigned char           *content );

extern PON_STATUS PAS_send_oam(      
                  const PON_olt_id_t             olt_id,
                  const PON_llid_t               llid,
                  const PON_oam_code_t 	         oam_code,
                  const unsigned short           length,
                  const unsigned char           *content );
 注意: 上面两个函数仅支持PAS5201 以上版本*/

/* A function used for handling ONU registration event */
extern short int  Onu_registration_handler 
				   ( const short int					  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const mac_address_t				  mac_address,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard );

extern short int  GponOnu_registration_handler 
				   ( const short int				 olt_id, 
					 const PON_onu_id_t		         onu_id, 
					 char*		                     sn,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard );

/*Begin: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
extern short int  Onu_registration_handler_8411 
				   ( const short int					  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const GW10G_PON_registration_data_t registration_data);

extern short int Olt_added_handler ( const PON_olt_id_t  olt_id );

extern short int Pon_cni_link_handler_8411( const short int PonPortIdx, const GW10G_PON_cni_id_e cni_port, const bool status );
/*End: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/

/* A function used for handling PON alarms */
extern short int Alarm_handler ( const short int	    olt_id, 
				    const short int	    alarm_source_id, 
				    const PON_alarm_t   alarm_type, 
				    const short int	    alarm_parameter, 
				    const void         *alarm_data );

extern short int PAS_pon_loss_handler( short int olt_id, short int PON_loss );
extern short int  OnuDeniedByMacAddrTable( short int PonPortIdx, mac_address_t  Onu_MacAddr);
extern short int OnuRegisterDeniedHandler( OnuRegisterDenied_S  *OnuRegisterDeniedData );

/* A function used for handling OLT reset */
extern short int  Olt_reset_handler ( const short int				olt_id,
						const PON_olt_reset_code_t  code );

/* B--added by liwei056@2010-1-26 for Pon-FastSwicthHover */
extern short int  Olt_redundancy_swap_begin_handler ( const short int	olt_id,
						const PON_redundancy_olt_failure_reason_t  reason );
extern short int  Olt_redundancy_swap_end_handler ( const short int	olt_id,
						const PON_redundancy_switch_over_status_t  status );
/* E--added by liwei056@2010-1-26 for Pon-FastSwicthHover */

/* B--added by liwei056@2011-12-21 for Pon-FastSwicthHover-CodeTest */
short int  Olt_redundancy_swaponu_begin_handler ( const short int	olt_id,
						const PON_redundancy_olt_failure_info_t  failure_info );
short int  Olt_redundancy_swap_between_handler ( const PON_olt_id_t   olt_id,
						PON_redundancy_olt_failure_reason_t   reason);
short int  Olt_redundancy_slave_unavail_handler ( const PON_olt_id_t   slave_olt_id);
short int  Olt_redundancy_swap_success_handler (const PON_olt_id_t   master_olt_id, 
                                                          const PON_olt_id_t   slave_olt_id);
short int  Olt_redundancy_swap_fail_handler (const PON_olt_id_t   master_olt_id, 
                                                         const PON_olt_id_t   slave_olt_id);
short int  Olt_redundancy_swap_opticalquery_handler (const PON_olt_id_t   master_olt_id, 
                                                                        const PON_olt_id_t   slave_olt_id);
/* E--added by liwei056@2011-12-21 for Pon-FastSwicthHover-CodeTest */


/* A function used for handling ONU deregistration from the PON */
/*extern short int  Onu_deregistration_handler ( const short int olt_id,
								 const PON_onu_id_t onu_id,
								 const PON_onu_deregistration_code_t  deregistration_code );*/

extern short int  Onu_deregistration_handler( const short int olt_id,
								 const PON_onu_id_t onu_id,
								 const PON_onu_deregistration_code_t  deregistration_code);

extern short int Onu_authorization_handler( const short int			 olt_id,
									 const PON_onu_id_t			 onu_id,
									 const PON_authorize_mode_t  authorize_mode );


/* A function used for handling start encryption acknowledge */
extern short int  Start_encryption_acknowledge_handler
					( const short int	 olt_id, 
					const PON_llid_t	llid, 
					const PON_start_encryption_acknowledge_codes_t  return_code );


/* A function used for handling update encryption key acknowledge */
extern short int Update_encryption_key_acknowledge_handler
				  ( const short int 	 olt_id, 
					const PON_llid_t	 llid, 
					const PON_update_encryption_key_acknowledge_codes_t  return_code );


/* A function used for handling stop encryption acknowledge */
extern short int  Stop_encryption_acknowledge_handler
				  ( const short int	  olt_id, 
					const PON_llid_t	   llid, 
					const PON_stop_encryption_acknowledge_codes_t  return_code );


/* Load OLT binary completed handler */
extern short int  PAS_load_olt_binary_completed_handler ( const short int  olt_id, 
															   const PON_binary_type_t  type,
															   const short int	return_code );


extern int SendLoadBinaryCompletedMsg( const short int olt_id, const PON_binary_type_t  type, const short int  return_code );
extern int sendPonAlarmMsg(  const short int	    olt_id, 
				    const short int	    alarm_source_id, 
				    const PON_alarm_t   alarm_type, 
				    const short int	    alarm_parameter, 
				    const void         *alarm_data );

extern int sendPonLossMsg( short int olt_id, short int PON_loss );
extern int sendSfpLossMsg( short int olt_id, bool SFP_loss );
extern int sendOnuDeniedMsg( short int PonPortIdx, mac_address_t  Onu_MacAddr );

extern int sendPonResetEventMsg( const short int olt_id, const PON_olt_reset_code_t  code );
extern int sendPonSwitchEventMsg( short int olt_id, int switch_event, int code, int source, int slot, int status, int seq, int flag);

extern int sendOnuRegistrationMsg( const short int		  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const mac_address_t				  mac_address,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard,
					 unsigned int event_flags );
extern int sendGponOnuRegistrationMsg(const short int olt_id, 
                     const PON_onu_id_t onu_id, 
                     char * sn, 
                     const PON_authentication_sequence_t authentication_sequence, 
                     const OAM_standard_version_t supported_oam_standard, unsigned int event_flags);

extern int sendOnuDeregistrationMsg( const short int olt_id, const PON_onu_id_t onu_id, PON_onu_deregistration_code_t  deregistration_code, unsigned int event_flags );
extern int sendOnuDeregistrationEvent(const short int olt_id,  const short int onu_idx, PON_onu_deregistration_code_t deregistration_code, unsigned int event_flags);
extern int sendOnuExtOamDiscoveryMsg(const short int olt_id, const PON_onu_id_t onu_id, CTC_STACK_discovery_state_t result, unsigned char Number_of_records, CTC_STACK_oui_version_record_t *onu_version_records_list, unsigned int event_flags);
extern int sendOnuExtOamOverMsg(const short int olt_id, const PON_onu_id_t onu_id, const mac_address_t	mac_address, CTC_STACK_discovery_state_t result);

extern int  LoadFileCompletedHandler( PonFileLoadCompleted_S *PonFileLoadResult );
extern int OnuRegisterHandler( ONURegisterInfo_S *OnuRegisterData );
extern int send_message_to_master(short int PonPort_id,short int onu_idx);
extern int OnuDeregisterHandler( ONUDeregistrationInfo_S *OnuDeregisterData);
extern int PonResetHandler( PonResetInfo_S *PonResetData );
extern int PonSwitchHandler( PonSwitchInfo_S *PonSwitchData );
extern int  PonAlarmHandler( PonAlarmInfo_S  *PonAlarmData );
/*Begin:for onu swap by jinhl@2013-04-27*/
extern int OnuSwitchHandler(OnuSwitchInfo_S *OnuSwitchData);
extern int PonOltLoose_OnuSwapHandler(short int PonPortIdx, short int PonPortIdx_Swap);
/*End:for onu swap by jinhl@2013-04-27*/

#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
extern short int  Convert_RSSI_to_dBm(const float vrssi, const float temperature, float *dbm);
extern short int  Get_olt_temperature(PON_olt_id_t  olt_id, float *temperature);
#endif
#ifdef PAS_SOFT_VERSION_V5_3_5
extern short int  Pon_cni_link_handler( const short int PonPortIdx, const bool status );
#endif

extern int Pon_gpio_changed_handler( const short int PonPortIdx, const unsigned char gpio_id, const bool status );

extern int Pon_llid_discovery_handler(short int olt_id, short int llid, mac_address_t link_mac, bcmEmmiLinkInfo *link_info);
extern int Pon_llid_loss_handler(short int olt_id, short int llid, mac_address_t link_mac, PON_onu_deregistration_code_t loss_reason);

extern int PonStdOamPktRecvHandler(short int olt_id, short int llid, char *oam_pkt, int pkt_len);


#if 1

/* Pon Event specific handler functions enum */
typedef enum
{
    PON_EVT_HANDLER_OLT_ADD = 0,        	         /* Add olt 					 */
    PON_EVT_HANDLER_OLT_RMV,	                     /* Remove olt					 */

    PON_EVT_HANDLER_LLID_END_MPCP_REGISTER,		     /* MPCP Registered 					 */
    PON_EVT_HANDLER_LLID_DEREGISTER_EVENT,	         /* Unregister event					 */

    PON_EVT_HANDLER_LLID_END_STD_OAM_DISCOVERY,		 /* 802.3ah's Discovery					 */
    PON_EVT_HANDLER_LLID_END_EXT_OAM_DISCOVERY,		 /* CTC etc's Discovery	 */

    PON_EVT_HANDLER_LLID_AUTHORIZATION,              /* AAA is pass */

    PON_EVT_HANDLER_LLID_STD_EVENT_NOTIFICATION,     /* Organization Specific Event Notification  */
    PON_EVT_HANDLER_LLID_EXT_EVENT_NOTIFICATION,     /* Organization Specific Event Notification  */

    PON_EVT_HANDLER_LAST_HANDLER
} PON_event_handler_index_t;

void InitPonEventHandler();
int Pon_event_assign_handler_function
                ( const PON_event_handler_index_t      handler_function_index, 
				  const void					    (*handler_function)(),
				  unsigned short                     *handler_id );
int Pon_event_delete_handler_function ( const unsigned short  handler_id );
int Pon_event_handler(PON_event_handler_index_t event_id, void *event_data);

#define DATA2PTR(d)    ((void*)(int*)(int)(d))
#define PTR2DATA(p)    ((int)(int*)(p))
#endif

#endif /* _PONEVENTHANDLER_H */
