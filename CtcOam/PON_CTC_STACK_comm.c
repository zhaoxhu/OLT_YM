  /*  
**  CTC_STACK_comm.c
**
**  
**  This software is licensed for use according to the terms set by the Passave API license agreement.
**  Copyright Passave Ltd. 
**  Ackerstein Towers - A, 9 Hamenofim St.
**  POB 2089, Herziliya Pituach 46120 Israel
**
**
**  This file was written by Meital Levy, meital.levy@passave.com, 14/03/2006
**  
**  Changes:
**
**  Version	  |  Date	   |    Change	             |    Author	  
**  ----------+------------+-------------------------+------------
**	1.00	  | 14/03/2006 |	Creation		     | Meital Levy
*/

#include "includeFromPas.h"
#include "PonLog.h"

#include "PON_CTC_STACK_comm.h"
#include "PON_CTC_STACK_defines.h"
#include "PON_CTC_STACK_frame_struct.h"
#include "PON_CTC_STACK_comparser_extended_oam.h"

#include "../StdOam/PonOam.h"


#define __MODULE__ "CTC_STACK" /* File name for log headers */

#define ___FILE___ "" /* File name for log headers */

#define PON_OLT_ID_ARRAY_SIZE          (8)         /* 目前只有BCM8PON板需要此CTC协议栈 */
#define CTC_MEM_DYNAMIC

/*============================== Data Types =================================*/                             

typedef enum
{
    CTC_STACK_COMM_DB_STATE_NOT_VALID,
    CTC_STACK_COMM_DB_STATE_RECEIVED_FRAME,
    CTC_STACK_COMM_DB_STATE_WAITING,
    CTC_STACK_COMM_DB_STATE_RECEIVED_LARGE_FRAME,
}ctc_stack_comm_db_state_t;



typedef struct  
{
    ctc_stack_comm_db_state_t    entry_state                        ;
    unsigned char               *received_data                      ;
    unsigned short              *received_data_size                 ;
    unsigned short               allocated_received_data_size       ;
    general_handler_function_t   handler_function                   ;
    unsigned char                expected_opcode                    ;
    unsigned char                expected_code                      ;
}ctc_stack_comm_onu_database_entry_t;



typedef struct 
{
    ctc_stack_comm_onu_database_entry_t     onu_id[PON_ONU_ID_PER_OLT_ARRAY_SIZE][0x50]          ;
    OSSRV_semaphore_t                       semaphore        ;
    
} ctc_stack_comm_olt_database_entry_t;

/*========================== External Variables =============================*/

/*========================== External Functions =============================*/
/*============================== Variables ==================================*/
/*=========================== Internal variables =============================*/

static unsigned long ctc_oui_define = CTC_OUI_DEFINE;

#ifdef CTC_MEM_DYNAMIC
static ctc_stack_comm_olt_database_entry_t  *internal_comm_database;
#else
static ctc_stack_comm_olt_database_entry_t   internal_comm_database[PON_OLT_ID_ARRAY_SIZE];
#endif

static bool   internal_debug_timeout_database[PON_OLT_ID_ARRAY_SIZE][PON_ONU_ID_PER_OLT_ARRAY_SIZE] = {0};


static OAM_oui_t internal_ctc_oui;
static OAM_oui_t internal_ctc_secondary_oui;


static unsigned short response_counter=1;


static general_handler_function_t   internal_onu_request_handler_function;

static general_handler_function_t   onu_event_handler_function = NULL;

/*============================== Constants ==================================*/

/*================================ Macros ===================================*/

/*======================== Internal Functions prototype =====================*/

static short int Insert_to_db_if_empty
                ( const PON_olt_id_t           olt_id,
                  const PON_onu_id_t           onu_id,  
                  const unsigned char          expected_opcode,
                  const unsigned char          expected_code,
                  const unsigned char         *received_data, 
                  const unsigned short        *received_data_size,
                  const unsigned short         allocated_received_data_size,
                  general_handler_function_t   handler_function );

static short int Insert_to_db
                ( const PON_olt_id_t           olt_id,
                  const PON_onu_id_t           onu_id,  
                  const unsigned char          expected_opcode,
                  const unsigned char          expected_code,
                  const unsigned char         *received_data, 
                  const unsigned short        *received_data_size,
                  const unsigned short         allocated_received_data_size,
                  general_handler_function_t   handler_function );

static short int Update_db
                ( const PON_olt_id_t     olt_id,
                  const PON_onu_id_t     onu_id,
                  const PON_oam_code_t	 oam_code,
                  const unsigned char    received_opcode,
                  const unsigned char    received_code,
                  const unsigned char   *received_data, 
                  const unsigned short   received_data_size);


static short int Remove_from_db
            ( const PON_olt_id_t   olt_id,
              const PON_onu_id_t   onu_id,
              const unsigned char  expected_opcode);


static short int  Get_entry_state_db
            ( const PON_olt_id_t          olt_id,
              const PON_onu_id_t          onu_id,
              const unsigned char         expected_opcode,
              ctc_stack_comm_db_state_t  *entry_state );


static void Reset_timeout_database ();

static void Received_message  ( 
                   const PON_olt_id_t      olt_id,
                   const PON_llid_t        llid,
                   const PON_oam_code_t	   oam_code,
                   const OAM_oui_t		   oui,
                   const unsigned short    length,
                   unsigned char          *content );


static short int Compare_oui( 
                    const OAM_oui_t   oui1,
                    const OAM_oui_t   oui2,
                    bool             *status);


static short int Convert_ponsoft_error_code_to_ctc_comm( short int pas_result);
/*================================ Functions ================================*/


#define PONLOG_FUNCTION_NAME  "Communication_ctc_init"
PON_STATUS Communication_ctc_init( general_handler_function_t onu_request_handler_function)
{

    PON_olt_id_t	 olt_id;
    short int result=CTC_STACK_COMMUNICATION_EXIT_OK;
    
#ifdef CTC_MEM_DYNAMIC
    ctc_stack_comm_olt_database_entry_t *olt_comm_db;
    
    if ( NULL != (olt_comm_db = (ctc_stack_comm_olt_database_entry_t*)g_malloc(sizeof(ctc_stack_comm_olt_database_entry_t) * PON_OLT_ID_ARRAY_SIZE)) )
    {
    	memset((unsigned char *)olt_comm_db, 0, sizeof(ctc_stack_comm_olt_database_entry_t) * PON_OLT_ID_ARRAY_SIZE);
        internal_comm_database = olt_comm_db;
    }
    else
    {
        VOS_ASSERT(0);
        return CTC_STACK_MEMORY_ERROR;
    }
#else
    VOS_MemZero(internal_comm_database, sizeof(internal_comm_database));
#endif


    internal_ctc_oui[0]  =   (unsigned char)((ctc_oui_define >>(BYTES_IN_WORD*BITS_IN_BYTE))& 0xff);
    internal_ctc_oui[1] =   (unsigned char)((ctc_oui_define >>BITS_IN_BYTE)& 0xff);
    internal_ctc_oui[2]  =   (unsigned char)(ctc_oui_define & 0xff);


    /* init a semaphore for each OLT */
	for( olt_id = 0; olt_id < PON_OLT_ID_ARRAY_SIZE; olt_id++ )
	{
		SEMAPHORE_INIT_AND_LOG_FOR_ERROR( Communication_init, &internal_comm_database[olt_id].semaphore, 
                                          internal_comm_database_olt_semaphore, result )
		 
        if( result != EXIT_OK )
        {
        
		    PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to init a internal_comm_database_olt_semaphore\n" );
		    
            return CTC_STACK_COMMUNICATION_EXIT_ERROR;
        }
	}

    

    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_OAM_IMFORMATION, internal_ctc_oui, 
                                        (VENDOR_OAM_frame_received_handler_t)Received_message );

    if( result != PAS_EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to assign to OAM MODULE\n" );
		
        return CTC_STACK_COMMUNICATION_EXIT_OK;
    }

    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_EVENT_NOTIFICATION, internal_ctc_oui, 
                                        (VENDOR_OAM_frame_received_handler_t)Received_message );

    if( result != PAS_EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to assign to OAM MODULE\n" );
		
        return CTC_STACK_COMMUNICATION_EXIT_OK;
    }

    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_VENDOR_EXTENSION, internal_ctc_oui, 
                                        (VENDOR_OAM_frame_received_handler_t)Received_message );

    if( result != PAS_EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to assign to OAM MODULE\n" );
		
        return CTC_STACK_COMMUNICATION_EXIT_OK;
    }

	if (internal_ctc_secondary_oui[0] != 0 ||
		internal_ctc_secondary_oui[1] != 0 ||
		internal_ctc_secondary_oui[2] != 0)
	{
		result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_VENDOR_EXTENSION, internal_ctc_secondary_oui, 
											(VENDOR_OAM_frame_received_handler_t)Received_message );

		if( result != PAS_EXIT_OK )
		{
        
			PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to assign to OAM MODULE\n" );
			
			return CTC_STACK_COMMUNICATION_EXIT_OK;
		}
	}

    Reset_timeout_database();

	internal_onu_request_handler_function = onu_request_handler_function;

    return result;
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Communication_ctc_terminate"

PON_STATUS Communication_ctc_terminate( void )
{
    short int result = CTC_STACK_COMMUNICATION_EXIT_OK;
    PON_olt_id_t	 olt_id;


    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_OAM_IMFORMATION, internal_ctc_oui, NULL);

    if( result != PAS_EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to deregister from OAM MODULE\n" );
		
        return CTC_STACK_COMMUNICATION_EXIT_OK;
    }

    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_EVENT_NOTIFICATION, internal_ctc_oui, NULL);

    if( result != PAS_EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to deregister from OAM MODULE\n" );
		
        return CTC_STACK_COMMUNICATION_EXIT_OK;
    }

    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_VENDOR_EXTENSION, internal_ctc_oui, NULL);

    if( result != PAS_EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to deregister from OAM MODULE\n" );
		
        return CTC_STACK_COMMUNICATION_EXIT_OK;
    }

     /* release each OLT semaphore */
	for( olt_id = 0; olt_id < PON_OLT_ID_ARRAY_SIZE; olt_id++ )
	{
		SEMAPHORE_CLOSE_AND_LOG_FOR_ERROR( MAKE_LOG_HEADER_CURRENT, &internal_comm_database[olt_id].semaphore, 
                                           internal_comm_database_olt_semaphore, result )
		if( result != EXIT_OK )
        {
        
	        PONLOG_ERROR_0( PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error in closing a semaphore\n" );
	    
            return CTC_STACK_COMMUNICATION_EXIT_ERROR;
        }

	}

    Reset_timeout_database();


    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Communication_set_ctc_oui"

PON_STATUS Communication_set_ctc_oui( const unsigned long oui )
{

    short int result = CTC_STACK_COMMUNICATION_EXIT_OK;
    
    /* step 1: deregister from OAM module under the previous oui*/

    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_OAM_IMFORMATION, internal_ctc_oui, NULL);

    if( result != PAS_EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to deregister from OAM MODULE\n" );
		
        return CTC_STACK_COMMUNICATION_EXIT_OK;
    }

    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_EVENT_NOTIFICATION, internal_ctc_oui, NULL);

    if( result != PAS_EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to deregister from OAM MODULE\n" );
		
        return CTC_STACK_COMMUNICATION_EXIT_OK;
    }

    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_VENDOR_EXTENSION, internal_ctc_oui, NULL);

    if( result != PAS_EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to deregister from OAM MODULE\n" );
		
        return CTC_STACK_COMMUNICATION_EXIT_OK;
    }

    /* step 2: update finternal oui*/

    internal_ctc_oui[0]  =   (unsigned char)((oui >>(BYTES_IN_WORD*BITS_IN_BYTE))& 0xff);
    internal_ctc_oui[1] =   (unsigned char)((oui >>BITS_IN_BYTE)& 0xff);
    internal_ctc_oui[2]  =   (unsigned char)(oui & 0xff);



    /* step 3: register in OAM module with the new oui*/
    
    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_OAM_IMFORMATION, internal_ctc_oui, 
                                        (VENDOR_OAM_frame_received_handler_t)Received_message );

    if( result != PAS_EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to assign to OAM MODULE\n" );
		
        return CTC_STACK_COMMUNICATION_EXIT_OK;
    }

    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_EVENT_NOTIFICATION, internal_ctc_oui, 
                                        (VENDOR_OAM_frame_received_handler_t)Received_message );

    if( result != PAS_EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to assign to OAM MODULE\n" );
		
        return CTC_STACK_COMMUNICATION_EXIT_OK;
    }

    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_VENDOR_EXTENSION, internal_ctc_oui, 
                                        (VENDOR_OAM_frame_received_handler_t)Received_message );

    if( result != PAS_EXIT_OK )
    {
        
		PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to assign to OAM MODULE\n" );
		
        return CTC_STACK_COMMUNICATION_EXIT_OK;
    }

	if (internal_ctc_secondary_oui[0] != 0 ||
		internal_ctc_secondary_oui[1] != 0 ||
		internal_ctc_secondary_oui[2] != 0)
	{
    result = PON_OAM_MODULE_assign_handler( PON_OAM_CODE_VENDOR_EXTENSION, internal_ctc_secondary_oui, 
                                        (VENDOR_OAM_frame_received_handler_t)Received_message );

		if( result != PAS_EXIT_OK )
		{
        
			PONLOG_ERROR_0(PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error while trying to deregister from OAM MODULE\n" );
			
			return CTC_STACK_COMMUNICATION_EXIT_OK;
		}
	}


    return CTC_STACK_COMMUNICATION_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Communication_assign_ctc_event_handler"
PON_STATUS Communication_assign_ctc_event_handler(general_handler_function_t onu_event_handler)
{
	onu_event_handler_function = onu_event_handler;

	return (CTC_STACK_COMMUNICATION_EXIT_OK);
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Send_receive_ctc_oam"

PON_STATUS Send_receive_ctc_oam
(      const PON_olt_id_t                 olt_id,
       const PON_onu_id_t                 onu_id,      
       const PON_oam_code_t               send_oam_code,
       unsigned char                     *send_data,
       const unsigned short               send_data_size,
       unsigned short*                    timeout_in_100_ms_param,
       const unsigned char                expected_opcode,
       const unsigned char                expected_code,
       unsigned char                     *receive_data,
       unsigned short                    *receive_data_size,
       const unsigned short               allocated_received_data_size,
       const general_handler_function_t   handler_function,
	   const short int                    request_id_param )
{

    short int                       result,return_result;
    bool                            resend = TRUE;
    short int                       request_id=request_id_param;
    ctc_stack_comm_db_state_t       entry_state;
    double                          timeout_in_sec;
    bool                            onu_warned_for_mode_change;
    PON_onu_activation_modes_t      onu_activation;
	unsigned short					timeout_in_100_ms = *timeout_in_100_ms_param;

    PON_DECLARE_TIMEOUT_VARIABLES


    timeout_in_sec = ((double)timeout_in_100_ms) / 10;
   

	if( (timeout_in_sec == 0) && (receive_data == NULL) )
	{
		return_result = Convert_ponsoft_error_code_to_ctc_comm(PON_OAM_MODULE_send_frame( olt_id, onu_id, send_oam_code, internal_ctc_oui, send_data_size, send_data ));
			
		if(return_result != CTC_STACK_COMMUNICATION_EXIT_OK)
		{
			PONLOG_ERROR_2(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "ERROR: failed to send OAM frame OLT: %d ONU: %d \n",olt_id, onu_id)
    
			return return_result;
		}
	}
	else
	{
		/*Can't support broadcast OAM with expecting response now*/
		if(onu_id == PON_BROADCAST_LLID)
		{
			PONLOG_ERROR_2(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "ERROR: Can't support receiving response for broadcast OAM OLT: %d ONU: %d \n",olt_id, onu_id)
		
			return CTC_STACK_COMMUNICATION_NOT_IMPLEMENTED;
		}
    
		if(handler_function == NULL)/* synchronize request */
		{
		    /* B--modified by liwei056@2014-4-9 for CTC's 并发处理BUG */    
#if 1		
            do {    
    			result = Insert_to_db_if_empty
    								( olt_id, onu_id, expected_opcode, expected_code, receive_data, 
    								  receive_data_size, allocated_received_data_size, handler_function);
                if ( (0 == result) || (0 == timeout_in_100_ms--) )
                {
                    break;
                }
                else
                {
                    OSSRV_wait(100);
                }
            } while( 0 != result );
#else
			result = Insert_to_db_if_empty
								( olt_id, onu_id, expected_opcode, expected_code, receive_data, 
								  receive_data_size, allocated_received_data_size, handler_function);
#endif
		    /* E--modified by liwei056@2014-4-9 for CTC's 并发处理BUG */    
			CHECK_COMM_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_CTC_COMM)

    
			while( (resend == TRUE) && (request_id < CTC_STACK_COMMUNICATION_RESEND_LIMIT) )
			{

				/* simulate timeout at ONU side by not sending the request*/
				if (internal_debug_timeout_database[olt_id][onu_id])
				{
				   return_result = PAS_EXIT_OK;
				}
				else
				{
					return_result = Convert_ponsoft_error_code_to_ctc_comm(PON_OAM_MODULE_send_frame( olt_id, onu_id, send_oam_code, internal_ctc_oui, send_data_size, send_data ));
				}
        
				if(return_result != CTC_STACK_COMMUNICATION_EXIT_OK)
				{
					PONLOG_ERROR_2(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "ERROR: failed to send OAM frame OLT: %d ONU: %d \n",olt_id, onu_id)
                
					resend = FALSE;
					continue;
				}

    
				request_id ++;

				PON_TIMEOUT_INIT(timeout_in_sec);
     
				/* wait for response - busy waiting */
				Get_entry_state_db(olt_id, onu_id, expected_opcode, &entry_state);

				while ( (PON_ONU_MODE_OFF != (onu_activation = OLTAdv_LLIDIsExist(olt_id, onu_id) ? PON_ONU_MODE_ON : PON_ONU_MODE_OFF)) &&
                        (entry_state != CTC_STACK_COMM_DB_STATE_RECEIVED_FRAME)
						 && (!PON_TIMEOUT_EXPIRED) ) 
				{
					PON_TIMEOUT_ITERATION;
					Get_entry_state_db(olt_id, onu_id, expected_opcode, &entry_state);

				}	

				if (PON_TIMEOUT_EXPIRED)
				{
					(*timeout_in_100_ms_param) = 0;
				}
				else
				{
					(*timeout_in_100_ms_param) = (unsigned short) ((__timeout_time__ - __duration__) * 10);
				}

                if (onu_activation == PON_ONU_MODE_OFF) 
                {
					resend = FALSE;
					return_result = CTC_STACK_COMMUNICATION_ONU_NOT_AVAILABLE;
                    PONLOG_ERROR_2( PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT: %d ONU: %d is not register any more.\n",
											   olt_id, onu_id)

                
                }
                else
                {
				    /* ONU is active     */
                    /* received response */
				    Get_entry_state_db(olt_id, onu_id, expected_opcode, &entry_state);
				    switch(entry_state)
				    {
				    case CTC_STACK_COMM_DB_STATE_RECEIVED_FRAME:
					    resend = FALSE;
					    return_result = CTC_STACK_COMMUNICATION_EXIT_OK;
					    break;
				    case CTC_STACK_COMM_DB_STATE_WAITING:
					    {
						    char  resend_str[10]="resending";
						    if(request_id == CTC_STACK_COMMUNICATION_RESEND_LIMIT)
						    {
							    memcpy(resend_str, "", 10);
						    }
          
						    if( send_oam_code != PON_OAM_CODE_OAM_IMFORMATION )
						    {
							PONLOG_ERROR_5( PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT: %d ONU: %d timeout on vendor specific message. expected opcode:0x%x expected code:0x%x.%s\n",
											       olt_id, onu_id, expected_opcode, expected_code, resend_str)
						    }
						    else
						    {
							PONLOG_ERROR_3( PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "OLT: %d ONU: %d timeout on information message.%s\n",
											       olt_id, onu_id, resend_str)

						    }
                
      
						    return_result = CTC_STACK_COMMUNICATION_TIME_OUT; /* we got out of loop because of timeout */
					    }
					    break;
				    case CTC_STACK_COMM_DB_STATE_RECEIVED_LARGE_FRAME:
					    resend = FALSE;
					    return_result = CTC_STACK_COMMUNICATION_MEMORY_ERROR;
					    break;
				    default:
					    PONLOG_ERROR_1( PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "No such case %d \n",entry_state);

				    }
                } /* ONU is active */
			} /* while( (resend == TRUE) */


		result = Remove_from_db(olt_id, onu_id, expected_opcode);
		CHECK_COMM_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_CTC_COMM)
		}
		else/* Async request */
		{
			result = Insert_to_db
								( olt_id, onu_id, expected_opcode, expected_code, receive_data, 
								  receive_data_size, allocated_received_data_size, handler_function );
			CHECK_COMM_RESULT_AND_RETURN_FUNC(PONLOG_FUNC_CTC_COMM)


			for(request_id = 0; request_id < CTC_STACK_COMMUNICATION_RESEND_LIMIT; request_id++)
			{
				if(resend)
				{
					/* simulate timeout at ONU side by not sending the request*/
					if (internal_debug_timeout_database[olt_id][onu_id])
					{
						return_result = CTC_STACK_COMMUNICATION_EXIT_OK;
					}
					else
					{
						return_result = Convert_ponsoft_error_code_to_ctc_comm(PON_OAM_MODULE_send_frame( olt_id, onu_id, send_oam_code, internal_ctc_oui, send_data_size, send_data ));
					}
        
					if(return_result != CTC_STACK_COMMUNICATION_EXIT_OK)
					{
						if(request_id == CTC_STACK_COMMUNICATION_RESEND_LIMIT-1)
						{
							PONLOG_ERROR_2(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "failed to send OAM frame OLT: %d ONU: %d \n",olt_id, onu_id)
						}
						else
						{
							PONLOG_ERROR_2(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "failed to send OAM frame OLT: %d ONU: %d resending...\n",olt_id, onu_id)
						}
					}
					else
					{
						resend = FALSE;
					}
				}
			}
		}
	}

    return return_result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Received_message"

static void Received_message  ( 
                   const PON_olt_id_t             olt_id,
                   const PON_llid_t               llid,
                   const PON_oam_code_t	          oam_code,
                   const OAM_oui_t				  oui,
                   const unsigned short           length,
                   unsigned char                 *content )
{
    unsigned char              *receive_pdu_data = (unsigned char  *)content;
    unsigned char               received_opcode,received_code;
    general_handler_function_t  handler_function;
    short int                   result;
	bool						match_oui;
	unsigned char				received_oui_str[10]={0}, expected_oui_str[10]={0};
	PON_onu_id_t				onu_id = llid; 
    unsigned short              received_leaf;
    
    /* timeout simulation for first response 
    if( response_counter == 1 )
    {
        response_counter++;
        return;
    }*/

    switch(oam_code)
    {
    case PON_OAM_CODE_VENDOR_EXTENSION:
		
			/*
			Compare_oui(internal_ctc_oui, oui, &match_oui );
            
                if( !match_oui)
                {
					sprintf(received_oui_str,"0x%x%x%x", oui[0], oui[1], oui[2]);

					sprintf(expected_oui_str,"0x%x%x%x", internal_ctc_oui[0], internal_ctc_oui[1], internal_ctc_oui[2]);

					PONLOG_ERROR_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
					   "received wrong OUI from OLT %d ONU %d. received %s expected %s\n",
					   olt_id, onu_id, received_oui_str, expected_oui_str);

                    return;

                }
            */

            /* Extract opcode field*/
            WORD_UBUFFER_2_ENDIANITY_UCHAR( receive_pdu_data, received_opcode )

			receive_pdu_data += BYTES_IN_CHAR;

	        /* Extract leaf code field*/
			if(received_opcode == OAM_VENDOR_EXT_ONU_EVENT)
			{
				if (onu_event_handler_function != NULL)
					onu_event_handler_function(olt_id, llid, length, receive_pdu_data);

				break;
			}
			else if(received_opcode == OAM_VENDOR_EXT_UPDATE_FIRMWARE)/* synchronize according to the TFTP opcodes */
			{
				unsigned short tftp_opcode;
				
				/* Extract TFTP opcode field*/
				UBUFFER_2_USHORT( (receive_pdu_data+TFTP_COMMON_HEADER), tftp_opcode )
				
					
				if(tftp_opcode == TFTP_UPDATE_FIRMWARE_OPCODE_READ)
				{
				
					internal_onu_request_handler_function(olt_id, llid, length, content, received_opcode);

					break;
				}
				else
				{
					if(tftp_opcode == TFTP_UPDATE_FIRMWARE_OPCODE_ERROR)
					{
						/*synchronize according to the TFTP response. it can be both ACK and ERROR response */
						tftp_opcode = TFTP_UPDATE_FIRMWARE_OPCODE_FILE_ACK;
					}

					received_code = (unsigned char) tftp_opcode;
				}

				
			}
			else if(received_opcode == OAM_VENDOR_EXT_ONU_AUTHENTICATION)/* synchronize according to the AUTH opcodes */
			{
				unsigned char auth_opcode;
				
				/* Extract auth opcode field*/
                WORD_UBUFFER_2_ENDIANITY_UCHAR( receive_pdu_data, auth_opcode )
    			receive_pdu_data += BYTES_IN_CHAR;

				received_code = auth_opcode;
            }
			else
			{
				/* step over the object context tlv if exist */
				STEP_OVER_OBJECT_CONTEXT_TLV(olt_id, llid, receive_pdu_data);
				WORD_UBUFFER_2_ENDIANITY_UCHAR( receive_pdu_data, received_code )
                receive_pdu_data += BYTES_IN_CHAR;
                UBUFFER_2_USHORT( receive_pdu_data, received_leaf)
                receive_pdu_data+= BYTES_IN_WORD;
                /*Walkaround to ignore the unexpected response for ONU TX POWER Supply Control OAM*/
                if((received_opcode == OAM_VENDOR_EXT_SET_RESPONSE) && 
                    (received_code == OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR) &&
                    (received_leaf == CTC_EX_VAR_ONU_TX_POWER_SUPPLY_CONTROL))
                {
                    PONLOG_ERROR_5( PONLOG_FUNC_CTC_COMM, olt_id,onu_id, "received unexpected message from OLT:%d ONU:%d with opcode:0x%x code:0x%x leaf:0x%x aborting\n", olt_id,onu_id, received_opcode,received_code, received_leaf );
                    return;
                }
			}
			
			/* extract the receive_opcode from the frame */
			/*content = content + BYTES_IN_CHAR;
			length = length - BYTES_IN_CHAR;*/
			result = Update_db(olt_id, llid, oam_code, received_opcode, received_code, content, length);

			if(result == CTC_STACK_COMMUNICATION_EXIT_OK)
			{
				handler_function = internal_comm_database[olt_id].onu_id[llid][received_opcode].handler_function;

				if( handler_function != NULL )
				{
					handler_function(olt_id, llid, length, content);
            
          
					result = Remove_from_db(olt_id, llid, received_opcode);
					if ( result != CTC_STACK_COMMUNICATION_EXIT_OK )
					{ 
						PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "failed in OLT %d ONU %d. error code:%d\n", olt_id, llid, result)
					}
				}
			}

		
            return;
        break;

    case PON_OAM_CODE_OAM_IMFORMATION:
        {  
			unsigned short			increment_length;
			unsigned char           info_type, oui_index;
			OAM_oui_t				received_oui;

#if 0
			/* skip on local and remote  TLV's */;
			increment_length = OAM_ETHERNET_STD_TLV_SIZE/* remote TLV */+OAM_ETHERNET_STD_TLV_SIZE/* local TLV */;
#else
            increment_length = 0;
#endif

			/* Extract info type field*/
			WORD_UBUFFER_2_ENDIANITY_UCHAR( (content+increment_length), info_type )
			increment_length+= BYTES_IN_CHAR/*info type field*/ + BYTES_IN_CHAR/*length field*/;

			if(info_type == EX_OAM_INF_TYPE)
			{

				/* Extract first OUI field*/
				for(oui_index = 0; oui_index < OAM_OUI_SIZE; oui_index++)
				{
					WORD_UBUFFER_2_ENDIANITY_UCHAR( (content+increment_length), received_oui[oui_index] )
					increment_length+= BYTES_IN_CHAR;
				}

				Compare_oui(received_oui, internal_ctc_oui, &match_oui);


				if(match_oui)
				{
					Update_db(olt_id, llid, oam_code, 0/*received_opcode*/, 0/*received_code*/, content, length);      
				}
				else
				{
					sprintf(received_oui_str,"0x%x%x%x", received_oui[0], received_oui[1], received_oui[2]);

					sprintf(expected_oui_str,"0x%x%x%x", internal_ctc_oui[0], internal_ctc_oui[1], internal_ctc_oui[2]);

					PONLOG_INFO_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
					   "received wrong OUI from OLT %d ONU %d. received %s expected %s\n",
					   olt_id, onu_id, received_oui_str, expected_oui_str);
				}
			}
        }
    break;
    case PON_OAM_CODE_EVENT_NOTIFICATION:
        {  
            PONLOG_INFO_2( PONLOG_FUNC_CTC_COMM, olt_id, onu_id,
                           "received event notification from OLT %d ONU %d\n", olt_id, onu_id);
            
			internal_onu_request_handler_function(olt_id, llid, length, content, oam_code);
        }
    break;
    default:
         PONLOG_ERROR_3( PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "receive OAM frame from OLT:%d ONU:%d, code 0x%x not supported\n", 
                            olt_id, llid, oam_code);
    
    }


    return;
    
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Insert_to_db"

static short int Insert_to_db
                ( const PON_olt_id_t           olt_id,
                  const PON_onu_id_t           onu_id,  
                  const unsigned char          expected_opcode,
                  const unsigned char          expected_code,
                  const unsigned char         *received_data, 
                  const unsigned short        *received_data_size,
                  const unsigned short         allocated_received_data_size,
                  general_handler_function_t   handler_function )
{
    short int  result;

    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR(Insert_to_db, &(internal_comm_database[olt_id].semaphore), ctc_stack_comm_semaphore_per_olt,result );
    if (result != EXIT_OK) return (CTC_STACK_COMMUNICATION_EXIT_ERROR);

    internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].received_data = (unsigned char *)received_data;
    internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].received_data_size = (unsigned short   *)received_data_size;
    internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].allocated_received_data_size = allocated_received_data_size;
    internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].entry_state = CTC_STACK_COMM_DB_STATE_WAITING;
    internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].handler_function = handler_function;
    internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].expected_opcode = expected_opcode;
    internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].expected_code = expected_code;

   
    /* END PROTECTION */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(Insert_to_db, &(internal_comm_database[olt_id].semaphore), ctc_stack_comm_semaphore_per_olt,result );
    if (result != EXIT_OK) return (CTC_STACK_COMMUNICATION_EXIT_ERROR);

    return CTC_STACK_COMMUNICATION_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "Insert_to_db_if_empty"

static short int Insert_to_db_if_empty
                ( const PON_olt_id_t           olt_id,
                  const PON_onu_id_t           onu_id,  
                  const unsigned char          expected_opcode,
                  const unsigned char          expected_code,
                  const unsigned char         *received_data, 
                  const unsigned short        *received_data_size,
                  const unsigned short         allocated_received_data_size,
                  general_handler_function_t   handler_function )
{
    if(internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].entry_state != CTC_STACK_COMM_DB_STATE_NOT_VALID )
    {/* it means that this entry is alredy taken by another request */
        PONLOG_ERROR_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "ERROR: OLT:%d ONU:%d, opcode 0x%x, code 0x%x entry is already occupied\n", 
                            olt_id, onu_id, expected_opcode, expected_code);
    
          
        return CTC_STACK_COMMUNICATION_EXIT_ERROR;
    }

    return (Insert_to_db(olt_id, onu_id, expected_opcode, expected_code, received_data, received_data_size, allocated_received_data_size, handler_function) );

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Update_db"

static short int Update_db
                ( const PON_olt_id_t     olt_id,
                  const PON_onu_id_t     onu_id,
                  const PON_oam_code_t	 oam_code,
                  const unsigned char    received_opcode,
                  const unsigned char    received_code,
                  const unsigned char   *received_data, 
                  const unsigned short   received_data_size)
{
    short int           sem_result;
    short int           result=CTC_STACK_COMMUNICATION_EXIT_OK;
    unsigned short      allocated_size;

    /* B--modified by liwei056@2014-5-30 for Ctc现场 BUG */
#if 1    
    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR(Update_db, &(internal_comm_database[olt_id].semaphore), ctc_stack_comm_semaphore_per_olt,sem_result );
    if (sem_result != EXIT_OK) return (CTC_STACK_COMMUNICATION_EXIT_ERROR);
 
    if( internal_comm_database[olt_id].onu_id[onu_id][received_opcode].entry_state == CTC_STACK_COMM_DB_STATE_WAITING)
    {
    
        switch(oam_code)
        {
            case PON_OAM_CODE_VENDOR_EXTENSION:
                if( (internal_comm_database[olt_id].onu_id[onu_id][received_opcode].expected_opcode != received_opcode) ||
                    (internal_comm_database[olt_id].onu_id[onu_id][received_opcode].expected_code != received_code) )
                {

                    PONLOG_INFO_4( PONLOG_FUNC_CTC_COMM, olt_id,onu_id, "received unexpected message from OLT:%d ONU:%d with opcode:0x%x code:0x%x. aborting\n", olt_id,onu_id, received_opcode,received_code );

        
                    result = CTC_STACK_COMMUNICATION_UNEXPECTED_RESPONSE;
                }

                break;
            case PON_OAM_CODE_OAM_IMFORMATION:
            case PON_OAM_CODE_EVENT_NOTIFICATION:
                break;
            default:

               PONLOG_ERROR_3( PONLOG_FUNC_CTC_COMM, olt_id,onu_id, "received message from OLT:%d ONU:%d with unsupported oam code 0x%x\n", 
                                  olt_id,onu_id, oam_code );

        }
        

        if ( CTC_STACK_COMMUNICATION_EXIT_OK == result )
        {
            allocated_size = internal_comm_database[olt_id].onu_id[onu_id][received_opcode].allocated_received_data_size;
            if(allocated_size < received_data_size)
            {

                PONLOG_ERROR_5( PONLOG_FUNC_CTC_COMM, olt_id,onu_id, "received large frame from OLT:%d ONU:%d with opcode:0x%x.\
                                   allocated size:%u, received size:%u\n", 
                                   olt_id, onu_id, received_opcode, allocated_size, received_data_size  );

                internal_comm_database[olt_id].onu_id[onu_id][received_opcode].entry_state = CTC_STACK_COMM_DB_STATE_RECEIVED_LARGE_FRAME;
                result =  CTC_STACK_COMMUNICATION_MEMORY_ERROR;
            }
            else
            {
                memcpy (internal_comm_database[olt_id].onu_id[onu_id][received_opcode].received_data, (unsigned char*)received_data,received_data_size);
                *(internal_comm_database[olt_id].onu_id[onu_id][received_opcode].received_data_size) = received_data_size;
                internal_comm_database[olt_id].onu_id[onu_id][received_opcode].entry_state = CTC_STACK_COMM_DB_STATE_RECEIVED_FRAME;
            }
        }

    }
    else  
    {

        PONLOG_ERROR_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "received unexpected message from OLT:%d ONU:%d with opcode:0x%x code:0x%x. aborting\n", olt_id,onu_id, received_opcode,received_code );

    
        result = CTC_STACK_COMMUNICATION_UNEXPECTED_RESPONSE;
    }
   
    /* END PROTECTION */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(Update_db, &(internal_comm_database[olt_id].semaphore), ctc_stack_comm_semaphore_per_olt,sem_result );
    if (sem_result != EXIT_OK) return (CTC_STACK_COMMUNICATION_EXIT_ERROR);
#else
    if( internal_comm_database[olt_id].onu_id[onu_id][received_opcode].entry_state == CTC_STACK_COMM_DB_STATE_NOT_VALID)
      
    {

        PONLOG_ERROR_4( PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "received unexpected message from OLT:%d ONU:%d with opcode:0x%x code:0x%x. aborting\n", olt_id,onu_id, received_opcode,received_code );

    
        return CTC_STACK_COMMUNICATION_UNEXPECTED_RESPONSE;
    }

    switch(oam_code)
    {
        case PON_OAM_CODE_VENDOR_EXTENSION:
            if( (internal_comm_database[olt_id].onu_id[onu_id][received_opcode].expected_opcode != received_opcode) ||
                (internal_comm_database[olt_id].onu_id[onu_id][received_opcode].expected_code != received_code) )
            {

                PONLOG_INFO_4( PONLOG_FUNC_CTC_COMM, olt_id,onu_id, "received unexpected message from OLT:%d ONU:%d with opcode:0x%x code:0x%x. aborting\n", olt_id,onu_id, received_opcode,received_code );

    
                return CTC_STACK_COMMUNICATION_UNEXPECTED_RESPONSE;
            }

            break;
        case PON_OAM_CODE_OAM_IMFORMATION:
        case PON_OAM_CODE_EVENT_NOTIFICATION:
            break;
        default:

           PONLOG_ERROR_3( PONLOG_FUNC_CTC_COMM, olt_id,onu_id, "received message from OLT:%d ONU:%d with unsupported oam code 0x%x\n", 
                              olt_id,onu_id, oam_code );

    }
    
    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR(Update_db, &(internal_comm_database[olt_id].semaphore), ctc_stack_comm_semaphore_per_olt,result );
    if (result != EXIT_OK) return (CTC_STACK_COMMUNICATION_EXIT_ERROR);
 

    allocated_size = internal_comm_database[olt_id].onu_id[onu_id][received_opcode].allocated_received_data_size;
    if(allocated_size < received_data_size)
    {

        PONLOG_ERROR_5( PONLOG_FUNC_CTC_COMM, olt_id,onu_id, "received large frame from OLT:%d ONU:%d with opcode:0x%x.\
                           allocated size:%u, received size:%u\n", 
                           olt_id, onu_id, received_opcode, allocated_size, received_data_size  );

        internal_comm_database[olt_id].onu_id[onu_id][received_opcode].entry_state = CTC_STACK_COMM_DB_STATE_RECEIVED_LARGE_FRAME;
        result =  CTC_STACK_COMMUNICATION_MEMORY_ERROR;
    }
    else
    {
        memcpy (internal_comm_database[olt_id].onu_id[onu_id][received_opcode].received_data, (unsigned char*)received_data,received_data_size);
        *(internal_comm_database[olt_id].onu_id[onu_id][received_opcode].received_data_size) = received_data_size;
        internal_comm_database[olt_id].onu_id[onu_id][received_opcode].entry_state = CTC_STACK_COMM_DB_STATE_RECEIVED_FRAME;
    }

   
    /* END PROTECTION */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(Update_db, &(internal_comm_database[olt_id].semaphore), ctc_stack_comm_semaphore_per_olt,result );
    if (result != EXIT_OK) return (CTC_STACK_COMMUNICATION_EXIT_ERROR);
#endif
    /* E--modified by liwei056@2014-5-30 for Ctc现场 BUG */

    return result;

}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Remove_from_db"

static short int Remove_from_db
            ( const PON_olt_id_t   olt_id,
              const PON_onu_id_t   onu_id,
              const unsigned char  expected_opcode)
{
    short int  result;
    
   
    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR(Remove_from_db, &(internal_comm_database[olt_id].semaphore), ctc_stack_comm_semaphore_per_olt,result );
    if (result != EXIT_OK) return (CTC_STACK_COMMUNICATION_EXIT_ERROR);

    
    /* check if it is a valid entry*/
    if(internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].received_data != NULL)
    {
        internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].received_data = NULL;
        internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].received_data_size = NULL;
        internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].allocated_received_data_size = 0;
        internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].entry_state = CTC_STACK_COMM_DB_STATE_NOT_VALID;
    }
    else
    {
        PONLOG_ERROR_2( PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "Error: OLT:%d ONU:%d entry not exist\n",olt_id, onu_id );
        /* modified by chenfj   此处应释放信号量,并返回错误*/
	 RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(Remove_from_db, &(internal_comm_database[olt_id].semaphore), ctc_stack_comm_semaphore_per_olt,result );
       return(CTC_STACK_COMMUNICATION_EXIT_ERROR);
    }
    

    /* END PROTECTION  */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(Remove_from_db, &(internal_comm_database[olt_id].semaphore), ctc_stack_comm_semaphore_per_olt,result );
    if (result != EXIT_OK) return (CTC_STACK_COMMUNICATION_EXIT_ERROR);


    return CTC_STACK_COMMUNICATION_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "Get_entry_state_db"
static short int  Get_entry_state_db
            ( const PON_olt_id_t          olt_id,
              const PON_onu_id_t          onu_id,
              const unsigned char         expected_opcode,
              ctc_stack_comm_db_state_t  *entry_state)
{
    short int  result;
  
    /* START PROTECTION */
    SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR(Get_entry_state_db, &(internal_comm_database[olt_id].semaphore), ctc_stack_comm_semaphore_per_olt,result );
    if (result != EXIT_OK) return (CTC_STACK_COMMUNICATION_EXIT_ERROR);


    (*entry_state) = internal_comm_database[olt_id].onu_id[onu_id][expected_opcode].entry_state;

    /* END PROTECTION  */
    RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(Get_entry_state_db, &(internal_comm_database[olt_id].semaphore), ctc_stack_comm_semaphore_per_olt,result );
    if (result != EXIT_OK) return (CTC_STACK_COMMUNICATION_EXIT_ERROR);


    return CTC_STACK_COMMUNICATION_EXIT_OK;

}
#undef PONLOG_FUNCTION_NAME


short int Timeout_ctc_simulate 
                    ( const PON_olt_id_t  olt_id, 
                      const PON_onu_id_t  onu_id,
                      const bool          timeout_mode)
{
    internal_debug_timeout_database[olt_id][onu_id] = timeout_mode;
  
    return CTC_STACK_COMMUNICATION_EXIT_OK;

}


void Reset_ctc_onu_timeout_database (
             const PON_olt_id_t  olt_id, 
             const PON_onu_id_t  onu_id )
{
    internal_debug_timeout_database[olt_id][onu_id] = DISABLE; 
}


static void Reset_timeout_database ()
{
    PON_olt_id_t                olt_id;
    PON_llid_t                  onu_id;

    for(olt_id = 0; olt_id < PON_OLT_ID_ARRAY_SIZE; olt_id++)
    {
        for(onu_id = PON_MIN_ONU_ID_PER_OLT; onu_id < PON_MAX_ONU_ID_PER_OLT; onu_id++)
        {
            Reset_ctc_onu_timeout_database(olt_id, onu_id);
        }
    }
}


static short int Compare_oui( 
                    const OAM_oui_t   oui1,
                    const OAM_oui_t   oui2,
                    bool             *status)
{

    unsigned char       i;

    for(i = 0; i < OAM_OUI_SIZE;i++)
    {
        if( *(oui1+i) !=  *(oui2+i) )
        {
            (*status) = FALSE;

            return CTC_STACK_COMMUNICATION_EXIT_OK;
        }
    }

    (*status) = TRUE;

    return CTC_STACK_COMMUNICATION_EXIT_OK;
}



#define PONLOG_FUNCTION_NAME  "Convert_ponsoft_error_code_to_ctc_comm"

static short int Convert_ponsoft_error_code_to_ctc_comm( short int pas_result)
{
	short int converted_result;

    switch( pas_result )
    {
    case PAS_EXIT_OK:
        converted_result = CTC_STACK_COMMUNICATION_EXIT_OK;
        break;
    case PAS_PARAMETER_ERROR:
    case PAS_EXIT_ERROR:
	case ENTRY_ALREADY_EXISTS:
	case TABLE_FULL:
        converted_result = CTC_STACK_COMMUNICATION_EXIT_ERROR;
        break; 
    case PAS_NOT_IMPLEMENTED:
	case PAS_NOT_SUPPORTED_IN_CURRENT_HARDWARE_VERSION:
        converted_result = CTC_STACK_COMMUNICATION_NOT_IMPLEMENTED;
        break;
    case PAS_TIME_OUT:
        converted_result = CTC_STACK_COMMUNICATION_TIME_OUT;
        break;
	case TABLE_EMPTY:
	case PAS_QUERY_FAILED:
		converted_result = CTC_STACK_COMMUNICATION_QUERY_FAILED;
		break;
	case PAS_HARDWARE_ERROR:
		converted_result = CTC_STACK_COMMUNICATION_HARDWARE_ERROR;
		break;
	case PAS_MEMORY_ERROR:
		converted_result = CTC_STACK_COMMUNICATION_MEMORY_ERROR;
		break;
	case PAS_OLT_NOT_EXIST:
		converted_result = CTC_STACK_COMMUNICATION_OLT_NOT_EXIST;
		break;
	case PAS_ONU_NOT_AVAILABLE:
		converted_result = CTC_STACK_COMMUNICATION_ONU_NOT_AVAILABLE;
		break;
    default:

		PONLOG_ERROR_1( PONLOG_FUNC_CTC_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,
						"Unsupported return code  %d\n", pas_result );
        converted_result = CTC_STACK_COMMUNICATION_EXIT_ERROR;
        
    }
    return converted_result;

}

void Set_pon_ctc_oui(unsigned long oui)
{
	ctc_oui_define = oui;
}

unsigned long Get_pon_ctc_oui(void)
{
	return (ctc_oui_define);
}


void Set_pon_ctc_secondary_oui(unsigned long oui)
{
    internal_ctc_secondary_oui[0]  =   (unsigned char)((oui >>(BYTES_IN_WORD*BITS_IN_BYTE))& 0xff);
    internal_ctc_secondary_oui[1] =   (unsigned char)((oui >>BITS_IN_BYTE)& 0xff);
    internal_ctc_secondary_oui[2]  =   (unsigned char)(oui & 0xff);
}


#undef PONLOG_FUNCTION_NAME

