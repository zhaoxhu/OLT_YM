/*                   Copyright(c) 2008-2012 GWTT, Inc.                    */
/*  
**  OnuPmcEvent.h -  Header file for PMC RemoteManagement Event Define 
**
**  This file was written by liwei056, 16/10/2013
**  
**  Changes:
**
**  Version       |  Date          |    Change        |    Author	  
**  ----------|-----------|-------------|------------
**	1.00          | 14/09/2012 |	creation	      | liwei056
*/

#if !defined(__ONU_PMC_EVENT_H__)
#define __ONU_PMC_EVENT_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "OnuPmcRemoteManagement.h"

int PAS_OnuRegister_EventNotify(short int olt_id, short int onu_id, mac_address_t onu_mac);
int PAS_OnuDeregister_EventNotify(short int olt_id, short int onu_id, PON_onu_deregistration_code_t deregistration_code);
int pas_pon_event_handler(PON_event_handler_index_t event_id, void *event_data);

void PAS_Received_event  ( 
                      const short int        olt_id,
                      const PON_onu_id_t     onu_id,
                      const unsigned short   length,
                      const unsigned char   *content);
PON_STATUS RM_PASONU_assign_received_event_handler(
                                   const short int                             application_id,
                                   const REMOTE_PASONU_received_event_func_t   handler );

PON_STATUS RM_PASONU_assign_handler( 
                                   const PON_alarm_event_type         type,
                                   const general_handler_function_t   handler );



#if defined(__cplusplus)
}
#endif

#endif /* __ONU_PMC_EVENT_H__ */


