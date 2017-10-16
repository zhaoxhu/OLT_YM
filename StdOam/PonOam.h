/*  
**  OAM_MODULE_expo.h
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
**	2.00	  | 28/07/2014 |	Modified		     | liwei056
*/


#ifndef _OAM_EXPO_H_
#define _OAM_EXPO_H_

#include "OltGeneral.h"


/*================================ Macroes ==================================*/
/*=============================== Constants =================================*/
#define  PONLOG_FUNC_OAM_COMM 					"oam-comm"         /* OAM module communication layer messages					*/

/*
** 802.3 Ethernet frame definitions
*/
#define MIN_ETHERNET_FRAME_SIZE					60 /* Minimal Ethernet frame length excluding Preamble,*/
												   /* SFD and FCS fields							   */
#define MAX_ETHERNET_FRAME_SIZE_STANDARDIZED  1514 /* Maximal Ethernet frame length excluding Preamble */
												   /* ,SFD and FCS fields standard value, not including*/
												   /* any extensions								   */

/* Ethernet frame structure (excluding Preamble, SFD and FCS) */ 
/* Definitions are of byte numbers							  */
#define ETHERNET_FRAME_DESTINATION_ADDRESS_BEGINNING_PLACE	0
#define ETHERNET_FRAME_DESTINATION_ADDRESS_END_PLACE		5
#define ETHERNET_FRAME_SOURCE_ADDRESS_BEGINNING_PLACE		6
#define ETHERNET_FRAME_SOURCE_ADDRESS_END_PLACE				11
/* MAC address definitions can be found in PON_general_expo.h */

#define ETHERNET_FRAME_TYPE_BEGINNING_PLACE					12
#define ETHERNET_FRAME_TYPE_END_PLACE						13
#define ETHERNET_FRAME_LENGTH_BEGINNING_PLACE				12
#define ETHERNET_FRAME_LENGTH_END_PLACE						13

#define ETHERNET_FRAME_SUB_TYPE_BEGINNING_PLACE			    14
#define ETHERNET_FRAME_SUB_TYPE_END_PLACE					14
#define ETHERNET_FRAME_FLAGS_BEGINNING_PLACE			    15
#define ETHERNET_FRAME_FLAGS_END_PLACE					    16
#define ETHERNET_FRAME_CODE_BEGINNING_PLACE					17
#define ETHERNET_FRAME_CODE_END_PLACE					    17

#define ETHERNET_FRAME_HEADER_SIZE					        ETHERNET_FRAME_CODE_END_PLACE



#define OAM_ETHERNET_FRAME_STD_TLV_SIZE 16	/* The size of a TLV (local & remote) of the OAM-INF */



/*----------------------------------------------------------*/
/*						Return codes						*/
/*----------------------------------------------------------*/
#define OAM_EXIT_OK				            EXIT_OK
#define OAM_EXIT_ERROR			            EXIT_ERROR
#define OAM_TIME_OUT				        TIME_OUT 
#define OAM_NOT_IMPLEMENTED		            NOT_IMPLEMENTED
#define OAM_PARAMETER_ERROR		            PARAMETER_ERROR
#define OAM_HARDWARE_ERROR		            HARDWARE_ERROR
#define OAM_MEMORY_ERROR			        MEMORY_ERROR
#define OAM_QUERY_FAILED			        (-18001)
#define OAM_ONU_NOT_AVAILABLE		        (-18002)
#define OAM_OLT_NOT_EXIST			        (-18003)
#define OAM_DB_FULL			                (-18004)



#define OAM_MODULE_DB_MAX_SIZE 100



/*============================== Data Types =================================*/

typedef void (*VENDOR_OAM_frame_received_handler_t)
                  (	const PON_olt_id_t             olt_id,
                    const PON_llid_t               llid,
                    const PON_oam_code_t           oam_code,
                    const OAM_oui_t				   oui,
                    const unsigned short           length,
                    unsigned char                 *content );

/*=============================== Functions =================================*/								     

PON_STATUS PON_OAM_MODULE_init(void);

PON_STATUS PON_OAM_MODULE_terminate(void);

PON_STATUS PON_OAM_MODULE_assign_handler( 
                    const PON_oam_code_t    	          oam_code,
                    const OAM_oui_t  					  oui,
                    VENDOR_OAM_frame_received_handler_t   handler_func );


PON_STATUS PON_OAM_MODULE_send_frame(      
                    const PON_olt_id_t             olt_id,
                    const PON_llid_t               llid,
                    const PON_oam_code_t       	   oam_code,
                    const OAM_oui_t				   oui,
                    const unsigned short           length,
                    const unsigned char           *content );

PON_STATUS PON_OAM_MODULE_receive_frame(  
                 const short int                 olt_id,
                 const PON_llid_t                llid,
				 const PON_oam_code_t		     oam_code, 
				 const OAM_oui_t                 oam_oui,
                 const unsigned short            length,
                 const unsigned char            *content);




#endif /* _OAM_EXPO_H_ */

