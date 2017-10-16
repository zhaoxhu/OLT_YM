/*                   Copyright(c) 2008-2012 GWTT, Inc.                    */
/*  
**  OnuPmcHandler.c -  C file for PMC RemoteManagement  Handler Implement 
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
#include "OnuPmcHandler.h"

/*================================ Macros ===================================*/
/*=============================== Constants =================================*/
#define __MODULE__ "ONU_PMC"        /* Module name for log headers */
#define ___FILE___ "pmc_handler" /* File name for log headers */

/*============================== Data Types =================================*/

/*========================== External Variables =============================*/
/*========================== External Functions =============================*/
/*=============================== Variables =================================*/
/* Internal variable to indicate if the package was init or not */
bool                           pon_pmc_remote_management_is_init = FALSE;

static PON_handlers_t                 remote_mgmt_db_handlers;

/*======================= Internal Functions Prototype ======================*/
/*================================ Functions ================================*/

#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_init"
PON_STATUS RM_PASONU_init ( void )
{
    int result;

	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    if(pon_pmc_remote_management_is_init)
    {
        result = RM_PASONU_terminate();
        if( result != REMOTE_PASONU_EXIT_OK )
        {
		    PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error in terminate\n" );
	        return REMOTE_PASONU_EXIT_ERROR;
        }
    }

    result = PON_PAS_APPLICATIONS_init();

    if( result != REMOTE_PASONU_EXIT_OK )
    {
		PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error in REMOTE_APPLICATIONS_init\n" );
	
        return REMOTE_PASONU_EXIT_ERROR;
    }
        
    result = PON_PAS_COMM_init( (general_handler_function_t)PAS_Received_event );

    if( result != REMOTE_PASONU_EXIT_OK )
    {
		PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error in Communication_init\n" );
	
        return REMOTE_PASONU_EXIT_ERROR;
    }

    PON_PAS_Adaptation_layer_assign_registration(PAS_OnuRegister_EventNotify);
    PON_PAS_Adaptation_layer_assign_deregistration(PAS_OnuDeregister_EventNotify);

    pon_pmc_remote_management_is_init = TRUE;

	/* Assign RSTP event handler */
	if ( (result = RM_PASONU_assign_received_event_handler(3, (REMOTE_PASONU_received_event_func_t)RM_PAS_Rstp_handle_event)) != REMOTE_PASONU_EXIT_OK)
	{
		return result;
	}

    return REMOTE_PASONU_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_terminate"
PON_STATUS RM_PASONU_terminate ( void )
{
    short int result;

	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    REMOTE_PASONU_CHECK_INIT

	/* Unassign RSTP event handler */
	if ( (result = RM_PASONU_assign_received_event_handler(3, NULL)) != REMOTE_PASONU_EXIT_OK)
	{
		return result;
	}

    result = PON_PAS_COMM_terminate();

    if( result != REMOTE_PASONU_EXIT_OK )
    {
		PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error in Communication_terminate\n" );
		
        return REMOTE_PASONU_EXIT_ERROR;
    }


    result = PON_PAS_APPLICATIONS_terminate();

    if( result != REMOTE_PASONU_EXIT_OK )
    {
		PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error in REMOTE_APPLICATIONS_terminate\n" );
		
        return REMOTE_PASONU_EXIT_ERROR;
    }
    
    PON_PAS_Adaptation_layer_terminate_registration(PAS_OnuRegister_EventNotify);
    PON_PAS_Adaptation_layer_terminate_deregistration(PAS_OnuDeregister_EventNotify);

    RM_MEM_ZERO(&remote_mgmt_db_handlers, sizeof(PON_handlers_t));
     
    pon_pmc_remote_management_is_init = FALSE;

    return REMOTE_PASONU_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_is_init"
PON_STATUS RM_PASONU_is_init ( bool *init )
{
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    (*init) = pon_pmc_remote_management_is_init;

    return REMOTE_PASONU_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME "Check_rm_soft_and_olt_validity"
short int Check_rm_soft_and_olt_validity ( const PON_olt_id_t  olt_id )
{
	if 	( !pon_pmc_remote_management_is_init ) 
	{
		PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT, "Error, RM-SOFT interface function is called but RM-SOFT is not running, discarding\n");

		return (REMOTE_PASONU_EXIT_ERROR); 
	}

    if (!PON_OLT_IN_BOUNDS)
    {
        PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT, "Error, OLT id (%d) is out of bounds. OLT range: %d - %d\n", olt_id, PON_MIN_OLT_ID, PON_MAX_OLT_ID);
        return (REMOTE_PASONU_PARAMETER_ERROR);	
    }

	return (REMOTE_PASONU_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_assign_application_handler_function"
PON_STATUS RM_PASONU_assign_application_handler_function
                ( const REMOTE_PASONU_handler_index_t	handler_function_index, 
				  const void							(*handler_function)(),
				  unsigned short						*handler_id )
{
	short int result;
	
	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

    REMOTE_PASONU_CHECK_INIT
	
    result= PON_general_assign_handler_function( &remote_mgmt_db_handlers, 
						(unsigned short int)handler_function_index, 
						(general_handler_function_t)handler_function,
						handler_id);

	if (result != EXIT_OK)
		return REMOTE_PASONU_EXIT_ERROR;  

    return REMOTE_PASONU_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "REMOTE_PASONU_delete_application_handler_function"
PON_STATUS RM_PASONU_delete_application_handler_function ( const unsigned short  handler_id )
{
	short int result;

	PONLOG_INFO_API_CALLED_0(PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT);

	REMOTE_PASONU_CHECK_INIT
	 
	result = PON_general_delete_handler_function( &remote_mgmt_db_handlers, handler_id );

	if (result != EXIT_OK)
		return REMOTE_PASONU_EXIT_ERROR;  

    return REMOTE_PASONU_EXIT_OK;  
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Rstp_handle_event"
int RM_PAS_Rstp_handle_event (
                          const PON_olt_id_t     olt_id,
                          const PON_onu_id_t     onu_id,
                          const unsigned short   length,
                          const unsigned char   *content)

{
	short int						client;
	short int						result, composer_result;
	general_handler_function_t		the_handler;
	INT32U							port_number;
	INT32U							loop_status;
	INT32U							link_status;
	unsigned long					*received_opcode = (unsigned long *)content;
	unsigned long					expected_opcode = MAKE_RSTP_EVENT_OPCODE_WORD(RSTP_EVENT_APPLICATION_ID_MASK, REMOTE_MANAGEMENT_RSTP_LOOP_DETECT_EVENT_OPCODE);
	

	if((*received_opcode) == expected_opcode)
	{
		composer_result = (short int)Decompose_rstp_loop_detect_event(
											content, 
											length,
											&port_number,
											&loop_status,
											&link_status);
		CHECK_COMPOSER_RESULT_AND_RETURN

			
		for (client = 0; client < PON_MAX_CLIENT; client++)
		{
			PON_general_get_handler_function
							   ( &remote_mgmt_db_handlers,
								 REMOTE_PASONU_HANDLER_RSTP_LOOP_DETECT_EVENT,
								 client ,
								 &the_handler );

			if (the_handler != NULL)
			{
				the_handler (olt_id, onu_id, port_number, loop_status, link_status);
			}
		}

	}
	else
	{
		PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "ERROR: received unknown event opcode  0x%X from OLT: %d ONU :%d\n",
                           (*received_opcode),olt_id, onu_id );
	}

	return REMOTE_PASONU_EXIT_OK;
}
#undef PONLOG_FUNCTION_NAME                                                          


