/*  
**  CTC_STACK_comparser.h
**
**  
**  This software is licensed for use according to the terms set by the Passave API license agreement.
**  Copyright Passave Ltd. 
**  Ackerstein Towers - A, 9 Hamenofim St.
**  POB 2089, Herziliya Pituach 46120 Israel
**
**
**  This file was written by Meital Levy, meital.levy@passave.com, 12/03/2006
**  
**  Changes:
**
**  Version	  |  Date	   |    Change	             |    Author	  
**  ----------+------------+-------------------------+------------
**	1.00	  | 12/03/2006 |	Creation		     | Meital Levy
*/


#ifndef _CTC_COMPARSER_H__
#define _CTC_COMPARSER_H__

#include "PonMicroParser.h"
#include "PON_CTC_STACK_defines.h"
#include "PON_CTC_STACK_variable_descriptor_defines_expo.h"
#include "PON_CTC_STACK_comparser_extended_oam.h"
#include "PON_CTC_STACK_frame_struct.h"


/*================================ Constants ==================================*/

/*================================ Macroes ==================================*/

/*============================== Data Types =================================*/

/*=============================== Functions =================================*/								     


/* Compose functions */
PON_STATUS Compose_extended_oam_information
(      
       /*IN*/    const unsigned long 	          ctc_oui,
       /*IN*/    const unsigned char 	          ctc_version,
       /*IN*/    const unsigned short 	          number_of_records ,
       /*IN*/    const CTC_STACK_oui_version_record_t  *oui_record_array,
       /*OUT*/   unsigned char 		             *pdu_data,
       /*IN_OUT*/unsigned short 		         *length );

PON_STATUS Compose_get_dba_thresholds
(      /*OUT*/   unsigned char 	 *pdu_data,
       /*IN_OUT*/unsigned short  *length );

PON_STATUS Compose_set_dba_thresholds
(      /*IN*/    const unsigned char					 number_of_queue,
       /*IN*/    CTC_STACK_onu_queue_set_thresholds_t   *queues_thresholds,
       /*OUT*/   unsigned char 							*pdu_data,
       /*IN_OUT*/unsigned short 						*length );

PON_STATUS Compose_new_key_request
(     /*IN*/    const PON_encryption_key_index_t   in_use_key_index,
      /*OUT*/   unsigned char 	                  *pdu_data,
      /*IN_OUT*/unsigned short 	                  *length );


PON_STATUS Compose_file_write_request
(     /*IN*/	const unsigned char  *file_name,
      /*OUT*/	unsigned char 		 *pdu_data,
      /*IN_OUT*/unsigned short 		 *length );

PON_STATUS Compose_file_transfer_data
(     /*IN*/	const unsigned short   block_number,
      /*OUT*/   unsigned char 		  *pdu_data,
      /*IN_OUT*/unsigned short 		  *length );

PON_STATUS Compose_file_transfer_ack
(     /*IN*/	const unsigned short   block_number,
      /*OUT*/	unsigned char 		  *pdu_data,
      /*IN_OUT*/unsigned short 		  *length );

PON_STATUS Compose_error_message
(     /*IN*/	const unsigned short   error_code,
	  /*IN*/	unsigned char		  *error_message,
      /*OUT*/	unsigned char 		  *pdu_data,
      /*IN_OUT*/unsigned short 		  *length );

PON_STATUS Compose_end_download_request
(     /*IN*/	const unsigned long    file_size,
      /*OUT*/	unsigned char 		  *pdu_data,
      /*IN_OUT*/unsigned short 		  *length );

PON_STATUS Compose_activate_image_request
(     /*IN*/	const TFTP_activate_image_request_t    flag,
      /*OUT*/	unsigned char 		                  *pdu_data,
      /*IN_OUT*/unsigned short 		                  *length );

PON_STATUS Compose_commit_image_request
(     /*IN*/	const TFTP_commit_image_request_t    flag,
      /*OUT*/	unsigned char 		                  *pdu_data,
      /*IN_OUT*/unsigned short 		                  *length );

PON_STATUS Compose_tftp_common_header
(     /*IN*/	const TFTP_data_type_t                 data_type,
      /*IN*/	const unsigned short                   payload_length,   
      /*IN*/	const PON_onu_id_t                     onu_id,
      /*IN*/	const bool                             compose_opcode,      
      /*OUT*/	unsigned char 		                  *pdu_data,
      /*IN_OUT*/unsigned short 		                  *length );

PON_STATUS Compose_auth_request
(     /*OUT*/   unsigned char 	                  *pdu_data,
      /*IN_OUT*/unsigned short 	                  *length );

PON_STATUS Compose_auth_success
(     /*OUT*/   unsigned char 	                  *pdu_data,
      /*IN_OUT*/unsigned short 	                  *length );

PON_STATUS Compose_auth_failure
(     /*IN*/    const CTC_STACK_auth_failure_type_t  failure_type,
      /*OUT*/   unsigned char 	                    *pdu_data,
      /*IN_OUT*/unsigned short 	                    *length );

/* Parse functions */

PON_STATUS Parse_extended_oam_information
(      /*IN*/ const unsigned char       *receive_buffer,
       /*IN*/ const unsigned short       length,
       /*OUT*/unsigned char 	        *number_of_records,
       /*OUT*/CTC_STACK_oui_version_record_t  *oui_record_array,
       /*OUT*/unsigned char 	        *ext_support);

PON_STATUS Parse_get_dba_thresholds
(      /*IN*/ const unsigned char					  *receive_buffer,
       /*IN*/ const unsigned short					   length,
       /*OUT*/unsigned char 						  *number_of_queue_sets,
       /*OUT*/CTC_STACK_onu_queue_set_thresholds_t    *queues_thresholds  );

PON_STATUS Parse_set_dba_thresholds
(      /*IN*/ const unsigned char					*receive_buffer,
       /*IN*/ const unsigned short					 length,
       /*OUT*/bool									*ack,
       /*OUT*/unsigned char 						*number_of_queue_sets,
       /*OUT*/CTC_STACK_onu_queue_set_thresholds_t  *queues_thresholds  );

PON_STATUS Parse_new_churning_key
(      /*IN*/ const unsigned char         *receive_buffer,
       /*IN*/ const unsigned short         length,
       /*OUT*/PON_encryption_key_index_t  *new_key_index,
       /*OUT*/CTC_encryption_key_t         churning_key );

PON_STATUS Parse_file_transfer_response
(      /*IN*/ unsigned char		    *receive_buffer,
       /*IN*/ const unsigned short   length,
       /*OUT*/bool                  *success,
       /*OUT*/unsigned short        *block_number,
       /*OUT*/unsigned short        *error_code,
       /*OUT*/unsigned char			*error_msg,
       /*OUT*/unsigned short		*received_buf_length);

PON_STATUS Parse_tftp_common_header
(      /*IN*/ unsigned char		    *receive_buffer,      
       /*IN*/ const unsigned short   length,              
	   /*IN*/ const PON_onu_id_t	 onu_id,
	   /*IN*/ const TFTP_data_type_t expected_data_type,
	   /*IN*/ const bool             parse_opcode,
       /*OUT*/unsigned short		*received_buf_length);

PON_STATUS Parse_file_read_request
(      /*IN*/ unsigned char		    *receive_buffer,
       /*IN*/ const unsigned short   length,
       /*OUT*/unsigned char		    *expected_file_name,
       /*OUT*/unsigned short		*received_buf_length);

PON_STATUS Parse_end_download_response
(      /*IN*/ unsigned char		    *receive_buffer,
       /*IN*/ const unsigned short   length,
       /*OUT*/TFTP_rps_code_t       *rps_code,
       /*OUT*/unsigned short		*received_buf_length);

PON_STATUS Parse_activate_image_response
(      /*IN*/ unsigned char		         *receive_buffer,
       /*IN*/ const unsigned short        length,
       /*OUT*/TFTP_activate_image_ack_t  *ack,
       /*OUT*/unsigned short	      	 *received_buf_length);

PON_STATUS Parse_commit_image_response
(      /*IN*/ unsigned char		         *receive_buffer,
       /*IN*/ const unsigned short        length,
       /*OUT*/TFTP_commit_image_ack_t  *ack,
       /*OUT*/unsigned short	      	 *received_buf_length);

PON_STATUS Parse_auth_response
(      /*IN*/ const unsigned char         *receive_buffer,
       /*IN*/ const unsigned short         length,
       /*OUT*/CTC_STACK_auth_response_t   *auth_response);

#endif /* _CTC_COMPOSER_H__ */

