/*  
**  CTC_STACK_comm.h - CTC stack communication layer header file
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
**  This file was written by Meital Levy, meital.levy@passave.com, 14/03/2006
**  
*/

#ifndef _CTC_STACK_COMM_H__
#define _CTC_STACK_COMM_H__

/*============================= Include Files ===============================*/	
#include "PonMicroParser.h"
#include "PonEvent.h"

/*=============================== Constants =================================*/
#define  PONLOG_FUNC_CTC_COMM 					"ctc-comm"         /* CTC package communication layer messages					*/

/*----------------------------------------------------------*/
/*						Return codes						*/
/*----------------------------------------------------------*/

#define CTC_STACK_COMMUNICATION_EXIT_OK				        EXIT_OK
#define CTC_STACK_COMMUNICATION_EXIT_ERROR			        EXIT_ERROR
#define CTC_STACK_COMMUNICATION_TIME_OUT				    TIME_OUT 
#define CTC_STACK_COMMUNICATION_NOT_IMPLEMENTED		        NOT_IMPLEMENTED
#define CTC_STACK_COMMUNICATION_PARAMETER_ERROR		        PARAMETER_ERROR
#define CTC_STACK_COMMUNICATION_HARDWARE_ERROR		        HARDWARE_ERROR
#define CTC_STACK_COMMUNICATION_MEMORY_ERROR			    MEMORY_ERROR
#define CTC_STACK_COMMUNICATION_OLT_NOT_EXIST			    (-18001)
#define CTC_STACK_COMMUNICATION_ONU_NOT_AVAILABLE		    (-18002)
#define CTC_STACK_COMMUNICATION_QUERY_FAILED		        (-18003)
#define CTC_STACK_COMMUNICATION_UNEXPECTED_RESPONSE	        (-18004)
#define CTC_STACK_COMMUNICATION_SEND_ISBUSY	                (-18005)




/* timings settings */
#define PON_CLOCKS_PER_SEC                    TQ_IN_A_SECOND  
#define CTC_STACK_COMMUNICATION_TIMEOUT       30         /* in 100 msec */ 
#define CTC_STACK_COMMUNICATION_RESEND_LIMIT  3  
/*================================ Macros ==================================*/




/*============================== Data Types =================================*/

 
/*========================= Functions Prototype =============================*/

PON_STATUS Communication_ctc_init();

PON_STATUS Communication_ctc_terminate( void );

PON_STATUS Send_receive_ctc_oam
(      const PON_olt_id_t                 olt_id,
       const PON_onu_id_t                 onu_id,      
       const PON_oam_code_t               send_oam_code,
       unsigned char                     *send_data,
       const unsigned short               send_data_size,
       unsigned short*               timeout_in_100_ms,
       const unsigned char                expected_opcode,
       const unsigned char                expected_code,
       unsigned char                     *receive_data,
       unsigned short                    *receive_data_size,
       const unsigned short               allocated_received_data_size,
       const general_handler_function_t   handler_function,
	   const short int                    request_id  );


/* function for timeout simulation debugging */
short int Timeout_ctc_simulate 
                    ( const PON_olt_id_t  olt_id, 
                      const PON_onu_id_t  onu_id,
                      const bool          timeout_mode );

void Reset_ctc_onu_timeout_database (
             const PON_olt_id_t  olt_id, 
             const PON_onu_id_t  onu_id );


PON_STATUS Communication_set_ctc_oui( const unsigned long oui );

void Set_pon_ctc_oui(unsigned long oui);
unsigned long Get_pon_ctc_oui(void);
void Set_pon_ctc_secondary_oui(unsigned long oui);

PON_STATUS Communication_assign_ctc_event_handler(general_handler_function_t onu_event_handler);

#endif /* _CTC_STACK_COMM_H__ */

