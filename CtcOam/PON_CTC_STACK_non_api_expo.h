/*  
**  CTC_STACK_non_api_expo.h - CTC stack header file
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


#ifndef _CTC_STACK_NON_API_EXPO_H__
#define _CTC_STACK_NON_API_EXPO_H__

/*============================= Include Files ===============================*/	

/*=============================== Constants =================================*/

/*================================ Macros ==================================*/

/*============================== Data Types =================================*/


/*========================= Functions Prototype =============================*/

#if 0
PON_STATUS New_olt_added_handler (const short int  olt_id);

PON_STATUS Removed_olt_handler (const short int  olt_id);
#endif

/* Set encryption timing parameters
**
** Input Parameters:
**
**		start_encryption_threshold : Start encryption threshold, measured in seconds
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_encryption_timing_threshold 
                ( const unsigned char   start_encryption_threshold );


/* Get encryption timing parameters
**
** Input Parameters:
**
** Output Parameters:
**
**		start_encryption_threshold : Start encryption threshold, measured in seconds
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_get_encryption_timing_threshold 
                ( unsigned char   *start_encryption_threshold );



/* Set CTC stack version
**
** Input Parameters:
**
**		version : ctc stack version.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_version 
                ( const unsigned char  version );


/* Get CTC stack version
**
** Input Parameters:
**
** Output Parameters:
**
**		version : ctc stack version.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_version 
                ( unsigned char  *version );


/* Set CTC stack OUI
**
** Input Parameters:
**
**		oui : ctc stack oui.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS PON_CTC_STACK_set_oui 
                ( const unsigned long  oui );


/* Get CTC stack oui
**
** Input Parameters:
**
** Output Parameters:
**
**		oui : ctc stack oui.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
PON_STATUS PON_CTC_STACK_get_oui 
                ( unsigned long  *oui );


#if 0

/* Set object branch value
**
** Setting object branch value
**
** Input Parameters:
**		object_branch	: object branch value.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS CTC_STACK_set_object_branch ( const unsigned char    object_branch );



/* Get object branch value
**
** Get object branch value
**
** Output Parameters:
**		object_branch	: object branch value.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS CTC_STACK_get_object_branch ( unsigned char  *object_branch );



/* Set object leaf port value
**
** Setting object leaf port value
**
** Input Parameters:
**		object_leaf_port	: port leaf value in the object.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS CTC_STACK_set_object_leaf_port ( const unsigned short    object_leaf_port );



/* Get object leaf port
**
** Get object leaf port value
**
** Output Parameters:
**		object_leaf_port	: port leaf value in the object.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS CTC_STACK_get_object_leaf_port ( unsigned short  *object_leaf_port );



/* Set object leaf ONU value
**
** Setting object leaf ONU value
**
** Input Parameters:
**		object_leaf_onu	: ONU leaf value in the object.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS CTC_STACK_set_object_leaf_onu ( const unsigned short    object_leaf_onu );



/* Get object leaf ONU
**
** Get object leaf ONU value
**
** Output Parameters:
**		object_leaf_onu	: ONU leaf value in the object.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS CTC_STACK_get_object_leaf_onu ( unsigned short  *object_leaf_onu );



/* Set object leaf VLAN value
**
** Setting object leaf VLAN value
**
** Input Parameters:
**		object_leaf_vlan	: VLAN leaf value in the object.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS CTC_STACK_set_object_leaf_vlan ( const unsigned short    object_leaf_vlan );



/* Get object leaf VLAN
**
** Get object leaf VLAN value
**
** Output Parameters:
**		object_leaf_vlan	: VLAN leaf value in the object.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS CTC_STACK_get_object_leaf_vlan ( unsigned short  *object_leaf_vlan );


/* Set object leaf QoS value
**
** Setting object leaf QoS value
**
** Input Parameters:
**		object_leaf_qos	: QoS leaf value in the object.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS CTC_STACK_set_object_leaf_qos ( const unsigned short    object_leaf_qos );



/* Get object leaf QoS
**
** Get object leaf QoS value
**
** Output Parameters:
**		object_leaf_qos	: QoS leaf value in the object.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS CTC_STACK_get_object_leaf_qos ( unsigned short  *object_leaf_qos );



/* Set object leaf multicast value
**
** Setting object leaf multicast value
**
** Input Parameters:
**		object_leaf_multicast	: multicast leaf value in the object.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS CTC_STACK_set_object_leaf_multicast ( const unsigned short    object_leaf_multicast );



/* Get object leaf multicast
**
** Get object leaf multicast value
**
** Output Parameters:
**		object_leaf_multicast	: multicast leaf value in the object.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

PON_STATUS CTC_STACK_get_object_leaf_multicast ( unsigned short  *object_leaf_multicast );
#endif

#endif /* _CTC_STACK_NON_API_EXPO_H__ */

