/*                   Copyright(c) 2008-2012 GWTT, Inc.                    */
/*  
**  OnuPmcHandler.h -  Header file for PMC RemoteManagement Handler Define 
**
**  This file was written by liwei056, 16/10/2013
**  
**  Changes:
**
**  Version       |  Date          |    Change        |    Author	  
**  ----------|-----------|-------------|------------
**	1.00          | 14/09/2012 |	creation	      | liwei056
*/

#if !defined(__ONU_PMC_HANDLER_H__)
#define __ONU_PMC_HANDLER_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "OnuPmcRemoteManagement.h"


PON_STATUS RM_PASONU_init ( void );
PON_STATUS RM_PASONU_terminate ( void );
PON_STATUS RM_PASONU_is_init ( bool *init );
short int Check_rm_soft_and_olt_validity ( const PON_olt_id_t  olt_id );

PON_STATUS RM_PASONU_assign_application_handler_function
                ( const REMOTE_PASONU_handler_index_t	handler_function_index, 
				  const void							(*handler_function)(),
				  unsigned short						*handler_id );
PON_STATUS RM_PASONU_delete_application_handler_function ( const unsigned short  handler_id );

int RM_PAS_Rstp_handle_event (
                          const PON_olt_id_t     olt_id,
                          const PON_onu_id_t     onu_id,
                          const unsigned short   length,
                          const unsigned char   *content);



#if defined(__cplusplus)
}
#endif

#endif /* __ONU_PMC_HANDLER_H__ */


