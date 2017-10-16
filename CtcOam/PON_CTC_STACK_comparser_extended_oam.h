/*  
**  CTC_STACK_comparser_extended_oam.h
**
**  
**  This software is licensed for use according to the terms set by the Passave API license agreement.
**  Copyright Passave Ltd. 
**  Ackerstein Towers - A, 9 Hamenofim St.
**  POB 2089, Herziliya Pituach 46120 Israel
**
**
**  This file was written by Meital Levy, meital.levy@passave.com, 21/09/2006
**  
**  Changes:
**
**  Version	  |  Date	   |    Change	             |    Author	  
**  ----------+------------+-------------------------+------------
**	1.00	  | 21/09/2006 |	Creation		     | Meital Levy
*/


#ifndef __CTC_COMPARSE_EXPO_H__
#define __CTC_COMPARSE_EXPO_H__

/*================================ Constants ==================================*/

#define OAM_VENDOR_STANDARD_BRANCH_END			0x00	/* Branch value identifying end-of-frame-data */
#define OAM_VENDOR_STANDARD_BRANCH_ATTR			0x07	/* Branch value of the Standard (IEEE 802.3) Attributes */
#define OAM_VENDOR_STANDARD_BRANCH_ACTION		0x09	/* Brnach value of the Standard (IEEE 802.3) Actions */
#define OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR		0xc7	/* Branch value of the CTC Extended Variable Attributes */
#define OAM_VENDOR_EXT_EX_VAR_BRANCH_ACTION 	0xc9	/* Branch value of the CTC Extended Variable Actions */
#define CTC_STD_OBJECT_BRANCH					0x36	/* Branch value of the CTC object */
#define CTC_2_1_MANAGEMENT_OBJECT_BRANCH		0x37	/* Branch value of the new CTC 2.1 management object */

#define OAM_VENDOR_STANDARD_OBJECT_LEAF_NONE	0xffff	/* A value that is defined in relation with object context variable container, and indicates - no-object-type */

/* CTC Extended Variables Branch 0xC7 Available Leafs: */
#define CTC_EX_VAR_ONU_IDENTIFIER                   0x0001  /* ONU Identifier */
#define CTC_EX_VAR_FIRMWARE_VERSION                 0x0002  /* firmware version */
#define CTC_EX_VAR_CHIPSET_ID                       0x0003  /* chipset id */
#define CTC_EX_VAR_ONU_CAPABILITIES                 0x0004  /* ONU capabilities */
#define CTC_EX_VAR_OPTICAL_TRANSCEIVER_DIAGNOSIS    0x0005  /* Optical transceiver diagnosis */
#define CTC_EX_VAR_SERVICE_SLA                      0x0006  /* Service SLA */
#define CTC_EX_VAR_ONU_CAPABILITIES_2               0x0007  /* ONU capabilities-2 */
#define CTC_EX_VAR_HOLDOVER_CONFIG		    0x0008  /* Holdover config */
#define CTC_EX_VAR_MXU_MNG_GLOBAL_PARAMETER         0x0009  /* Set/Get MDU/MTU manage global parameter */
#define CTC_EX_VAR_MXU_MNG_SNMP_PARAMETER           0x000A  /* Set/Get MDU/MTU manage snmp parameter */
#define CTC_EX_VAR_PON_IF_ADMIN_STATE		    0x000B  /* Active PON interface Admin state */
#define CTC_EX_VAR_ONU_CAPABILITIES_3               0x000C  /* ONU capabilities-3 */
#define CTC_EX_VAR_ETH_PORT_LINK_STATE              0x0011  /* Ethernet port link state */
#define CTC_EX_VAR_ETH_PORT_PAUSE                   0x0012  /* Ethernet port pause */
#define CTC_EX_VAR_ETH_PORT_POLICING                0x0013  /* Ethernet port policing */
#define CTC_EX_VAR_VOIP_PORT                        0x0014  /* VOIP port */
#define CTC_EX_VAR_E1_PORT                          0x0015  /* E1 port  */
#define CTC_EX_VAR_ETH_PORT_DS_RATE_LIMITING	    0x0016	/* Ethernet Port Downstream Rate Limiting */
#define CTC_EX_VAR_PORT_LOOP_DETECT		    0x0017  /* User port loop detect */
#define CTC_EX_VAR_VLAN                             0x0021  /* VLAN modes*/
#define CTC_EX_VAR_QINQ_VLAN                        0x1021  /* QinQ VLAN modes*/  
#define CTC_EX_VAR_CLASSIFICATION_N_MARKING         0x0031  /* Classification & Marking */
#define CTC_EX_VAR_MULTICAST_VLAN_OPER              0x0041  /* Add/Delete Multicast VLAN */
#define CTC_EX_VAR_MULTICAST_TAG_STRIP              0x0042  /* Multicast tag strip */
#define CTC_EX_VAR_MULTICAST_SWITCH                 0x0043  /* Multicast switch */
#define CTC_EX_VAR_MULTICAST_CONTROL                0x0044  /* Multicast VLAN control */
#define CTC_EX_VAR_MULTICAST_GROUP_NUM              0x0045  /* Multicast group number */
#define CTC_EX_VAR_FAST_LEAVE_ABILITY               0x0046  /* Fast leave ability */
#define CTC_EX_VAR_FAST_LEAVE_ADMIN_STATE           0x0047  /* Fast leave admin state */
#define CTC_EX_VAR_LLID_QUEUE_CONFIG		    0x0051  /* Manage the relation between LLID and service queue */

#define CTC_EX_VAR_VOIP_IAD_INFO                    0x0061  /* Get IAD card info */
#define CTC_EX_VAR_VOIP_GLOBAL_PARAM_CONF           0x0062  /* Set/Get VOIP global parameter configuration */
#define CTC_EX_VAR_H248_PARAM_CONFIG                0x0063  /* Set/Get H.248/MGCP parameter configuration */
#define CTC_EX_VAR_H248_USER_TID_CONFIG             0x0064  /* Set/Get H.248/MGCP user TID parameter configuration */
#define CTC_EX_VAR_H248_RTP_TID_CONFIG            0x0065 /* Set H.248/MGCP RTP TID parameter configuration */  
#define CTC_EX_VAR_H248_RTP_TID_INFO            0x0066 /* Get H.248/MGCP RTP TID parameter information */  
#define CTC_EX_VAR_SIP_PARAM_CONFIG                 0x0067  /* Set/Get SIP protocol configuration */
#define CTC_EX_VAR_SIP_USER_PARAM_CONFIG            0x0068  /* Set/Get SIP user parameter configuration */
#define CTC_EX_VAR_VOIP_FAX_CONFIG                  0x0069  /* Set/Get VOIP FAX parameter configuration */
#define CTC_EX_VAR_VOIP_IAD_OP_STATUS               0x006A  /* Get IAD operation status */
#define CTC_EX_VAR_VOIP_POTS_STATUS               0x006B  /* Get user POTS interface status */

#define CTC_EX_VAR_ALARM_ADMIN_STATE                0x0081  /* Alarm admin state */
#define CTC_EX_VAR_ALARM_THRESHOLD		    0x0082  /* Alarm threshold */

#define CTC_EX_VAR_ONU_TX_POWER_SUPPLY_CONTROL      0x00A1  /* Control the ONU TX module's power supply */

/*added by wangxiaoyu 2012-02-09*/
#define CTC_EX_VAR_STATISTIC_STATE					0x00b1  /*onu or uni ports statistic state*/
#define CTC_EX_VAR_STATISTIC_DATA					0x00b2	 /*onu or uni ports statistic data*/
#define CTC_EX_VAR_STATISTIC_HIS_DATA					0x00b3	 /*onu or uni ports statistic history data*/


/* CTC Extended Actions Branch 0xC9 Available Leafs: */
#define CTC_EX_ACTION_RESET_ONU						0x0001	/* ONU reset*/
#define CTC_EX_ACTION_FAST_LEAVE_ADMIN_CONTROL      0x0048  /* Fast leave admin control */
#define CTC_EX_ACTION_RESET_CARD					0x0401	/* Card reset */
#define CTC_EX_ACTION_MULTI_LLID_ADMIN_CONTROL		0x0202	/* Multi LLID admin control */
#define CTC_EX_ACTION_VOIP_IAD_OPERATION            0x006C  /* ONU voice module reset */
#define CTC_EX_ACTION_SIP_DIGIT_MAP_CONFIG          0x006D  /* SIP Digitial Map configuration*/


#define CTC_EX_ACTION_LAST 0x0002	/* This does not represent a REAL Extended Action, but rather the size of an array to hold them all */


/* CTC Standard Attributes Branch 0x07 Available Leafs: */

#define CTC_STD_ATTR_GET_PHY_ADMIN_STATE						37	/* aPhyAdminState Phy admin state*/
#define CTC_STD_ATTR_GET_AUTO_NEG_ADMIN_STATE					79	/* aAutoNegAdminState Auto neg admin state*/
#define CTC_STD_ATTR_GET_AUTO_NEG_LOCAL_TECHNOLOGY_ABILITY		82	/* aAutoNegLocalTechnologyAbility Auto neg local technology ability*/
#define CTC_STD_ATTR_GET_AUTO_NEG_ADVERTISED_TECHNOLOGY_ABILITY	83	/* aAutoNegAdvertisedTechnologyAbility Auto neg advertised technology ability*/
#define CTC_STD_ATTR_FEC_ABILITY								313	/* aFECAbility */
#define CTC_STD_ATTR_FEC_MODE									314	/* aFECmode */


/* CTC Standard Actions Branch 0x09 Available Leafs: */

#define CTC_STD_ACTION_PHY_ADMIN_CONTROL			5		/* acPHYAdminControl Phy admin control*/
#define CTC_STD_ACTION_AUTO_NEG_RESTART_AUTO_CONFIG	11		/* acAutoNegRestartAutoConfig */
#define CTC_STD_ACTION_AUTO_NEG_ADMIN_CONTROL		12		/* acAutoNegAdminControl */





/*----------------------------------------------------------*/
/*						Return codes						*/
/*----------------------------------------------------------*/


#define COMPARSER_STATUS_OK                EXIT_OK
#define COMPARSER_STATUS_ERROR             EXIT_ERROR
#define COMPARSER_STATUS_NOT_SUPPORTED     NOT_IMPLEMENTED
#define COMPARSER_STATUS_BAD_PARAM         PARAMETER_ERROR
#define COMPARSER_STATUS_TIMEOUT           TIME_OUT
#define COMPARSER_STATUS_MEMORY_ERROR      MEMORY_ERROR
#define COMPARSER_STATUS_WRONG_OPCODE      (-19001)
#define COMPARSER_STATUS_CTC_FAIL		   (-19002)
#define COMPARSER_STATUS_END_OF_FRAME	   (-19003)

#define COMPARSER_CTC_STATUS_SUCCESS		0x80
#define COMPARSER_CTC_STATUS_BAD_PARAM		0x86
#define COMPARSER_CTC_STATUS_NO_RESOURCE	0x87




extern bool CTC_COMPOSER_global_set_response_contains_data;

/* Type <CTC_COMPOSE_function_t> defines a function to composing a certain CTC Extended Variable Descriptor, */
/* Either for a Get Response or a Set Request frame. */
/**************************************************************************************
Function type: CTC_COMPOSE_function_t

Description:
	Defines a function to composing a certain CTC Extended Variable Descriptor,
	either for a Get Response or a Set Request frame.
	The composed frame already contains network endianity, where needed.

Returns:
	COMPOSER_STATUS_<...> defined above.

Paramaters:
	num_of_entries (IN):		The number of instance entries in the array given by <ports_info>.
	ports_info (IN):			An array of structured instances, defined by the specific
								CTC Extended Variable Descriptor.
	response_ret_value (IN):	In case this function composes a Get Response frame (ONU),
								this argument represents the return value of the response,
								as the CTC Spec defines.
								Otherwise, when in the OLT environment, this parameter is
								ignored.
	is_get_frame (IN):				TRUE - Compose for Get-Response
								FALSE - Compose for Set-Response/Request
	send_buffer (OUT):			An allocated buffer where the funtion composes the requested frame into.
								Composed is a CTC Extended Variable (depending on the implementing
								function), and on the OLT environment, this also includes
								the 'opcode' field, i.e. offset 21 of the Ethernet frame.
	send_buf_length (IN/OUT):	IN - The allocated space of the buffer <send_buffer>.
								OUT - The actual used length of the frame composed.
**************************************************************************************/
typedef short int (*CTC_COMPOSE_function_t)(unsigned char num_of_entries,
											void* ports_info,
											const unsigned char response_ret_value,
											const unsigned char is_get_frame,
											unsigned char* send_buffer,
											unsigned short* send_buf_length);

/* Type <CTC_PARSE_function_t> defines a function for parsing a certain CTC Extended Variable Descriptor, */
/* Either for a Get Response or a Set Request frame. */
/**************************************************************************************
Function type: CTC_PARSE_function_t

Description:
	Defines a function for parsing a certain CTC Extended Variable Descriptor,
	Either for a Get Response or a Set Request frame.

Returns:
	COMPOSER_STATUS_<...> defined above.

Paramaters:
	recv_buffer (IN):			The frame received, a CTC Extended Variable Descriptor, including
								the descriptor's header. In the OLT environment, this includes
								the 'opcode' field, i.e. offset 21 of the Ethernet frame.
	recv_buffer_size (IN):		The size of the frame within <recv_buffer> buffer.
	num_of_entries (IN/OUT):	The amount of structured instances identified via the parser.
								This defines the size of the data within the array <ports_info>.
								If this parameter is set to 0 on INput, the number of entries
								is taken from the <width> field of the variable descriptor.
								Otherwise - this parameter on INput defines the number of
								entries. This is done do distinguish between Set & Get Response
								frames (on the OLT).
	ports_info (OUT):			An array of structured instances of data, defined by the CTC Spec,
								and by the specific function implementation.
	ctc_ret_val (OUT):			(Only for Get Response, OLT environment) The value returned
								for the Get Request, as to the measure of success of the
								applying of the request itself. Defined by the CTC Spec.
	received_buf_length(OUT):   The actual used length of the parsed frame
**************************************************************************************/
typedef short int (*CTC_PARSE_function_t)(unsigned char* recv_buffer,
										  const unsigned short recv_buffer_size,
										  const unsigned char* num_of_entries,
										  void* ports_info,
										  unsigned char* ctc_ret_val,
										  unsigned short		*received_buf_length);


short int CTC_COMPOSE_set_response(const unsigned char branch,
								   const unsigned short leaf,
								   const unsigned char ret_val,
								   unsigned char *send_buffer,
								   unsigned short *send_buf_length);

short int CTC_COMPOSE_opcode
		  ( const unsigned char    opcode,
			unsigned char		  *send_buffer,
			unsigned short		  *send_buf_length);

short int CTC_PARSE_opcode
		  ( unsigned char		  *recv_buffer,
			const unsigned short   recv_buffer_size,
			unsigned short		  *received_buf_length);

short int CTC_COMPOSE_general_request
		  ( const unsigned char	   branch,
			const unsigned short   leaf,
			unsigned char		  *send_buffer,
			unsigned short		  *send_buf_length);



short int CTC_COMPOSE_onu_identifier
				( const unsigned char					 num_of_entries,
				  const CTC_STACK_onu_serial_number_t	*ports_info,
				  const unsigned char					 response_ret_value,
				  const unsigned char					 is_get_frame,
				  unsigned char							*send_buffer,
				  unsigned short						*send_buf_length);


short int CTC_PARSE_onu_identifier
				( unsigned char					 *recv_buffer,
				  const unsigned short			  recv_buffer_size,
				  unsigned char					 *num_of_entries,
				  CTC_STACK_onu_serial_number_t  *onu_identifier,
				  unsigned char					 *ctc_ret_val,
				  unsigned short				 *received_buf_length);
short int CTC_PARSE_onu_identifier_ctc_3_0
				( unsigned char					 *recv_buffer,
				  const unsigned short			  recv_buffer_size,
				  unsigned char					 *num_of_entries,
				  CTC_STACK_onu_serial_number_t  *onu_identifier,
				  unsigned char					 *ctc_ret_val,
				  unsigned short				 *received_buf_length);


short int CTC_COMPOSE_firmware_version
				( const unsigned char					 num_of_entries,
				  const unsigned short        			*version,
				  const unsigned char					 response_ret_value,
				  const unsigned char					 is_get_frame,
				  unsigned char							*send_buffer,
				  unsigned short						*send_buf_length);


short int CTC_PARSE_firmware_version
				( unsigned char			*recv_buffer,
				  const unsigned short	 recv_buffer_size,
				  unsigned char			*num_of_entries,
				  INT8U					*version_buffer,
				  INT8U					*version_buffer_size,
				  unsigned char			*ctc_ret_val,
				  unsigned short		*received_buf_length);


short int CTC_COMPOSE_chipset_id
				( const unsigned char					 num_of_entries,
				  const CTC_STACK_chipset_id_t  		*chipset_id,
				  const unsigned char					 response_ret_value,
				  const unsigned char					 is_get_frame,
				  unsigned char							*send_buffer,
				  unsigned short						*send_buf_length);


short int CTC_PARSE_chipset_id
				( unsigned char			  *recv_buffer,
				  const unsigned short	   recv_buffer_size,
				  unsigned char			  *num_of_entries,
				  CTC_STACK_chipset_id_t  *chipset_id,
				  unsigned char			  *ctc_ret_val,
				  unsigned short		  *received_buf_length);

short int CTC_COMPOSE_onu_capabilities
				( const unsigned char					 num_of_entries,
				  const CTC_STACK_onu_capabilities_t	*chipset_id,
				  const unsigned char					 response_ret_value,
				  const unsigned char					 is_get_frame,
				  unsigned char							*send_buffer,
				  unsigned short						*send_buf_length);

short int CTC_PARSE_onu_capabilities
				( unsigned char					*recv_buffer,
				  const unsigned short			 recv_buffer_size,
				  unsigned char				    *num_of_entries,
				  CTC_STACK_onu_capabilities_t  *onu_capabilities,
				  unsigned char					*ctc_ret_val,
				  unsigned short				*received_buf_length);

short int CTC_COMPOSE_ethrnet_port_pause
				( const unsigned char	num_of_entries,
				  const bool		   *flow_control_enable,
				  const unsigned char	response_ret_value,
				  const unsigned char	is_get_frame,
				  unsigned char		   *send_buffer,
				  unsigned short	   *send_buf_length);


short int CTC_PARSE_ethernet_port_pause
				( unsigned char			  *recv_buffer,
				  const unsigned short	   recv_buffer_size,
				  /*const unsigned char	   is_get_frame,*/
				  unsigned char			  *num_of_entries,
				  bool					  *flow_control_enable,
				  unsigned char			  *ctc_ret_val,
				  unsigned short		  *received_buf_length);


short int CTC_COMPOSE_ethrnet_port_policing
				( const unsigned char						       num_of_entries,
				  const CTC_STACK_ethernet_port_policing_entry_t  *ports_info,
				  const unsigned char							   response_ret_value,
				  const unsigned char						       is_get_frame,
				  unsigned char								      *send_buffer,
				  unsigned short							      *send_buf_length);


short int CTC_PARSE_ethernet_port_policing
				( unsigned char								*recv_buffer,
				  const unsigned short						 recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_ethernet_port_policing_entry_t  *ports_info,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);


short int CTC_COMPOSE_voip_port
				( const unsigned char			  num_of_entries,
				  const CTC_STACK_on_off_state_t *port_state,
				  const unsigned char			  response_ret_value,
				  const unsigned char			  is_get_frame,
				  unsigned char				     *send_buffer,
				  unsigned short			     *send_buf_length);

short int CTC_PARSE_voip_port
				( unsigned char							*recv_buffer,
				  const unsigned short					 recv_buffer_size,
				  unsigned char							*num_of_entries,
				  CTC_STACK_on_off_state_t				*port_state,
				  unsigned char							*ctc_ret_val,
				  unsigned short						*received_buf_length);

short int CTC_COMPOSE_e1_port
				( const unsigned char			  num_of_entries,
				  const CTC_STACK_on_off_state_t *port_state,
				  const unsigned char			  response_ret_value,
				  const unsigned char			  is_get_frame,
				  unsigned char				     *send_buffer,
				  unsigned short			     *send_buf_length);

short int CTC_PARSE_e1_port
				( unsigned char							*recv_buffer,
				  const unsigned short					 recv_buffer_size,
				  unsigned char							*num_of_entries,
				  CTC_STACK_on_off_state_t				*port_state,
				  unsigned char							*ctc_ret_val,
				  unsigned short						*received_buf_length);


/* C7-0x0021 */
short int CTC_COMPOSE_vlan
				( const unsigned char						   num_of_entries,
				  const CTC_STACK_port_vlan_configuration_t  *ports_info,
				  const unsigned char						   response_ret_value,
				  const unsigned char						   is_get_frame,
				  unsigned char								  *send_buffer,
				  unsigned short						      *send_buf_length);


/* C7-0x0021 */
short int CTC_PARSE_vlan
				( unsigned char							*recv_buffer,
				  const unsigned short					 recv_buffer_size,
				  unsigned char							*num_of_entries,
				  CTC_STACK_port_vlan_configuration_t	*ports_info,
				  unsigned char							*ctc_ret_val,
				  unsigned short						*received_buf_length);


/* C7-0x0031 */
short int CTC_COMPOSE_classification_n_marking
				( const unsigned char							  num_of_entries,
				  const  CTC_STACK_classification_and_marking_t  *ports_info,
				  const unsigned char							  response_ret_value,
				  const unsigned char							  is_get_frame,
				  unsigned char								     *send_buffer,
				  unsigned short							     *send_buf_length,
				  CTC_STACK_onu_ipv6_aware_support_t onu_ipv6_aware);


/* C7-0x0031 */
short int CTC_PARSE_classification_n_marking
				( unsigned char							  *recv_buffer,
				  const unsigned short					   recv_buffer_size,
				  unsigned char							  *num_of_entries,
				  CTC_STACK_classification_and_marking_t  *ports_info,
				  unsigned char							  *ctc_ret_val,
				  unsigned short						  *received_buf_length,
				  CTC_STACK_onu_ipv6_aware_support_t onu_ipv6_aware);


/* C7-0x0041 */
short int CTC_COMPOSE_multicast_vlan
				( const unsigned char					 num_of_entries,
				  const CTC_STACK_multicast_vlan_t		*ports_info,
				  const unsigned char					 response_ret_value,
				  const unsigned char					 is_get_frame,
				  unsigned char							*send_buffer,
				  unsigned short						*send_buf_length);


/* C7-0x0041 */
short int CTC_PARSE_multicast_vlan
				( unsigned char							*recv_buffer,
				  const unsigned short					 recv_buffer_size,
				  unsigned char							*num_of_entries,
				  CTC_STACK_multicast_vlan_t			*ports_info,
				  unsigned char							*ctc_ret_val,
				  unsigned short						*received_buf_length);


/* C7-0x0042 */
short int CTC_COMPOSE_multicast_control
				( const unsigned char					 num_of_entries,
				  const CTC_STACK_multicast_control_ipv6_ext_t	*ports_info,
				  const unsigned char					 response_ret_value,
				  const unsigned char					 is_get_frame,
				  unsigned char							*send_buffer,
				  unsigned short						*send_buf_length);


/* C7-0x0042 */
short int CTC_PARSE_multicast_control
				( unsigned char							*recv_buffer,
				  const unsigned short					 recv_buffer_size,
				  unsigned char							*num_of_entries,
				  CTC_STACK_multicast_control_ipv6_ext_t			*ports_info,
				  unsigned char							*ctc_ret_val,
				  unsigned short						*received_buf_length,
				  bool									*more_frame);


/* C7-0x0043 */
short int CTC_COMPOSE_multicast_group
				( const unsigned char   num_of_entries,
				  const unsigned char  *group_num,
				  const unsigned char   response_ret_value,
				  const unsigned char   is_get_frame,
				  unsigned char		   *send_buffer,
				  unsigned short	   *send_buf_length);


/* C7-0x0043 */
short int CTC_PARSE_multicast_group
				( unsigned char			*recv_buffer,
				  const unsigned short   recv_buffer_size,
				  unsigned char			*num_of_entries,
				  unsigned char			*group_num,
				  unsigned char			*ctc_ret_val,
				  unsigned short		*received_buf_length);


short int CTC_COMPOSE_phy_admin_state
				( const unsigned char					  num_of_entries,
				  const CTC_STACK_ethernet_port_state_t  *state,
				  const unsigned char					  response_ret_value,
				  const unsigned char					  is_get_frame,
				  unsigned char							 *send_buffer,
				  unsigned short						 *send_buf_length);


short int CTC_PARSE_phy_admin_state
				( unsigned char					   *recv_buffer,
				  const unsigned short				recv_buffer_size,
				  unsigned char					   *num_of_entries,
				  CTC_STACK_ethernet_port_state_t  *state,
				  unsigned char					   *ctc_ret_val,
				  unsigned short				   *received_buf_length);


short int CTC_COMPOSE_phy_admin_control
				( const unsigned char					   num_of_entries,
				  const CTC_STACK_ethernet_port_action_t  *state,
				  const unsigned char					   response_ret_value,
				  const unsigned char					   is_get_frame,
				  unsigned char							  *send_buffer,
				  unsigned short						  *send_buf_length);

short int CTC_PARSE_phy_admin_control
				( unsigned char						*recv_buffer,
				  const unsigned short				 recv_buffer_size,
				  unsigned char						*num_of_entries,
				  CTC_STACK_ethernet_port_action_t  *state,
				  unsigned char						*ctc_ret_val,
				  unsigned short					*received_buf_length);


short int CTC_PARSE_auto_negotiation_admin_state
				( unsigned char					   *recv_buffer,
				  const unsigned short				recv_buffer_size,
				  unsigned char					   *num_of_entries,
				  CTC_STACK_ethernet_port_state_t  *state,
				  unsigned char					   *ctc_ret_val,
				  unsigned short				   *received_buf_length);

short int CTC_COMPOSE_auto_negotiation_admin_state
				( const unsigned char					  num_of_entries,
				  const CTC_STACK_ethernet_port_state_t  *state,
				  const unsigned char					  response_ret_value,
				  const unsigned char					  is_get_frame,
				  unsigned char							 *send_buffer,
				  unsigned short						 *send_buf_length);


short int CTC_COMPOSE_auto_negotiation_restart_auto_config
				( const unsigned char	 num_of_entries,
				  const unsigned char	*dummy,
				  const unsigned char	 response_ret_value,
				  const unsigned char	 is_get_frame,
				  unsigned char			*send_buffer,
				  unsigned short		*send_buf_length);


short int CTC_PARSE_auto_negotiation_restart_auto_config
				( unsigned char			*recv_buffer,
				  const unsigned short	 recv_buffer_size,
				  unsigned char		    *num_of_entries,
				  bool					*dummy,
				  unsigned char			*ctc_ret_val,
				  unsigned short		*received_buf_length);



short int CTC_PARSE_auto_negotiation_admin_control
				( unsigned char						*recv_buffer,
				  const unsigned short				 recv_buffer_size,
				  unsigned char						*num_of_entries,
				  CTC_STACK_ethernet_port_action_t  *state,
				  unsigned char						*ctc_ret_val,
				  unsigned short					*received_buf_length);

short int CTC_COMPOSE_auto_negotiation_admin_control
				( const unsigned char					   num_of_entries,
				  const CTC_STACK_ethernet_port_action_t  *state,
				  const unsigned char					   response_ret_value,
				  const unsigned char					   is_get_frame,
				  unsigned char							  *send_buffer,
				  unsigned short						  *send_buf_length);

short int CTC_PARSE_fec_ability
				( unsigned char						*recv_buffer,
				  const unsigned short	 			 recv_buffer_size,
				  unsigned char		    			*num_of_entries,
				  CTC_STACK_standard_FEC_ability_t	*ability,
				  unsigned char						*ctc_ret_val,
				  unsigned short					*received_buf_length);

short int CTC_COMPOSE_fec_ability
				( const unsigned char	 					 num_of_entries,
				  const CTC_STACK_standard_FEC_ability_t	*ability,
				  const unsigned char	 					 response_ret_value,
				  const unsigned char	 					 is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);


short int CTC_PARSE_fec_mode
				( unsigned char					*recv_buffer,
				  const unsigned short	 		 recv_buffer_size,
				  unsigned char		    		*num_of_entries,
				  CTC_STACK_standard_FEC_mode_t	*mode,
				  unsigned char					*ctc_ret_val,
				  unsigned short				*received_buf_length);

short int CTC_COMPOSE_fec_mode
				( const unsigned char	 				 num_of_entries,
				  const CTC_STACK_standard_FEC_mode_t	*mode,
				  const unsigned char	 				 response_ret_value,
				  const unsigned char	 				 is_get_frame,
				  unsigned char							*send_buffer,
				  unsigned short						*send_buf_length);



short int CTC_COMPOSE_ethrnet_port_link_state
				( const unsigned char			 num_of_entries,
				  const CTC_STACK_link_state_t  *link_state,
				  const unsigned char			 response_ret_value,
				  const unsigned char			 is_get_frame,
				  unsigned char					*send_buffer,
				  unsigned short				*send_buf_length);


short int CTC_PARSE_ethernet_port_link_state
				( unsigned char			  *recv_buffer,
				  const unsigned short	   recv_buffer_size,
				  unsigned char			  *num_of_entries,
				  CTC_STACK_link_state_t  *link_state,
				  unsigned char		      *ctc_ret_val,
				  unsigned short		  *received_buf_length);


short int CTC_COMPOSE_statistic_data
				( const unsigned char					 num_of_entries,
				  const CTC_STACK_statistic_data_t *dataset,
				  const unsigned char					 response_ret_value,
				  const unsigned char					 is_get_frame,
				  unsigned char							*send_buffer,
				  unsigned short						*send_buf_length,
				  unsigned int                        is_onu);

short int CTC_PARSE_statistic_state
				( unsigned char					  *recv_buffer,
				  const unsigned short			   recv_buffer_size,
				  unsigned char					  *num_of_entries,
				  CTC_STACK_statistic_state_t * state,
				  unsigned char					  *ctc_ret_val,
				  unsigned short				  *received_buf_length);

short int CTC_PARSE_statistic_history_data
				( unsigned char					  *recv_buffer,
				  const unsigned short			   recv_buffer_size,
				  unsigned char					  *num_of_entries,
				  CTC_STACK_statistic_data_t * data,
				  unsigned char					  *ctc_ret_val,
				  unsigned short				  *received_buf_length,
				  unsigned int                  is_onu);



short int CTC_COMPOSE_multicast_tag_strip
				( const unsigned char	                      num_of_entries,
				  const CTC_STACK_tag_oper_t                  tag_oper,
                  const CTC_STACK_multicast_vlan_switching_t  multicast_vlan_switching,
				  const unsigned char	                      response_ret_value,
				  const unsigned char	                      is_get_frame,
				  unsigned char		                         *send_buffer,
				  unsigned short	                         *send_buf_length);


short int CTC_PARSE_multicast_tag_strip
				( unsigned char			                *recv_buffer,
				  const unsigned short	                 recv_buffer_size,
				  unsigned char			                *num_of_entries,
				  CTC_STACK_tag_oper_t                  *tag_oper,
                  CTC_STACK_multicast_vlan_switching_t  *multicast_vlan_switching,
				  unsigned char		                 	*ctc_ret_val,
				  unsigned short                		*received_buf_length);


short int CTC_COMPOSE_multicast_switch
				( const unsigned char					 num_of_entries,
				  const CTC_STACK_multicast_protocol_t  *multicast_protocol,
				  const unsigned char					 response_ret_value,
				  const unsigned char					 is_get_frame,
				  unsigned char							*send_buffer,
				  unsigned short						*send_buf_length);


short int CTC_PARSE_multicast_switch
				( unsigned char					  *recv_buffer,
				  const unsigned short			   recv_buffer_size,
				  unsigned char					  *num_of_entries,
				  CTC_STACK_multicast_protocol_t  *multicast_protocol,
				  unsigned char					  *ctc_ret_val,
				  unsigned short				  *received_buf_length);


short int CTC_COMPOSE_fast_leave_admin_control
				( const unsigned char						num_of_entries,
				  const CTC_STACK_fast_leave_admin_state_t  *state,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);


short int CTC_PARSE_fast_leave_admin_state
				( unsigned char							*recv_buffer,
				  const unsigned short					recv_buffer_size,
				  unsigned char							*num_of_entries,
				  CTC_STACK_fast_leave_admin_state_t	*fast_leave_admin_state,
				  unsigned char							*ctc_ret_val,
				  unsigned short						*received_buf_length);


short int CTC_PARSE_fast_leave_ability
				( unsigned char							*recv_buffer,
				  const unsigned short					recv_buffer_size,
				  unsigned char							*num_of_entries,
				  CTC_STACK_fast_leave_ability_t		*fast_leave_ability,
				  unsigned char							*ctc_ret_val,
				  unsigned short						*received_buf_length);


short int CTC_COMPOSE_ethernet_port_ds_rate_limiting
				( const unsigned char										num_of_entries,
				  const CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*rate_limiting_entry,
				  const unsigned char										response_ret_value,
				  const unsigned char										is_get_frame,
				  unsigned char												*send_buffer,
				  unsigned short											*send_buf_length);


short int CTC_PARSE_ethernet_port_ds_rate_limiting
				( unsigned char										*recv_buffer,
				  const unsigned short								recv_buffer_size,
				  unsigned char										*num_of_entries,
				  CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*rate_limiting_entry,
				  unsigned char										*ctc_ret_val,
				  unsigned short									*received_buf_length);


short int CTC_COMPOSE_auto_neg_local_technology_ability
				( const unsigned char									  num_of_entries,
				  const CTC_STACK_auto_negotiation_technology_ability_t  *local_technology,
				  const unsigned char									  response_ret_value,
				  const unsigned char									  is_get_frame,
				  unsigned char											 *send_buffer,
				  unsigned short										 *send_buf_length);


short int CTC_PARSE_auto_neg_local_technology_ability
				( unsigned char									   *recv_buffer,
				  const unsigned short							    recv_buffer_size,
				  unsigned char									   *num_of_entries,
				  CTC_STACK_auto_negotiation_technology_ability_t  *local_technology,
				  unsigned char									   *ctc_ret_val,
				  unsigned short								   *received_buf_length);



short int CTC_COMPOSE_auto_neg_advertised_technology_ability
				( const unsigned char									  num_of_entries,
				  const CTC_STACK_auto_negotiation_technology_ability_t  *local_technology,
				  const unsigned char									  response_ret_value,
				  const unsigned char									  is_get_frame,
				  unsigned char											 *send_buffer,
				  unsigned short										 *send_buf_length);



short int CTC_PARSE_auto_neg_advertised_technology_ability
				( unsigned char									   *recv_buffer,
				  const unsigned short							    recv_buffer_size,
				  unsigned char									   *num_of_entries,
				  CTC_STACK_auto_negotiation_technology_ability_t  *local_technology,
				  unsigned char									   *ctc_ret_val,
				  unsigned short								   *received_buf_length);


short int CTC_PARSE_general_ack
				( unsigned char			*recv_buffer,
				  const unsigned short	 recv_buffer_size,
				  const unsigned char	 expected_branch,
				  const unsigned short   expected_leaf,
				  unsigned char			*ctc_ret_val,
				  unsigned short		*received_buf_length);

short int CTC_COMPOSE_port_object
		  ( const unsigned char                      port_number,
     		const unsigned char	                     onu_version,
            const CTC_management_object_port_type_t  port_type,
			unsigned char		                    *send_buffer,
			unsigned short		                    *send_buf_length);


short int CTC_PARSE_port_object
		  ( unsigned char		  *recv_buffer,
			const unsigned short   recv_buffer_size,
			const bool			   return_error_if_branch_is_zero,
      		const unsigned char	   onu_version,
			unsigned char    	  *port_number,
			unsigned short		  *received_buf_length);

bool CTC_COMPARSER_is_step_over_branch
			( const unsigned char 			branch_value);


short int CTC_COMPOSE_management_object
			  ( 
				const CTC_management_object_t		management_object,
                const unsigned char	                onu_version,
				unsigned char						*send_buffer,
				unsigned short						*send_buf_length);

			
short int CTC_PARSE_management_object
			  ( unsigned char						*recv_buffer,
				const unsigned short				recv_buffer_size,
				const bool							return_error_if_branch_is_zero,
                const unsigned char	                onu_version,
				CTC_management_object_t				*management_object,
				unsigned short						*received_buf_length);


short int CTC_COMPOSE_llid_object
		  ( const PON_llid_t       llid,
			unsigned char		  *send_buffer,
			unsigned short		  *send_buf_length);


short int CTC_PARSE_llid_object
		  ( unsigned char		  *recv_buffer,
			const unsigned short   recv_buffer_size,
			const PON_llid_t       expected_llid,
			unsigned short		  *received_buf_length);


short int CTC_COMPOSE_card_object
		  ( const unsigned char    slot_number,
			unsigned char		  *send_buffer,
			unsigned short		  *send_buf_length);


short int CTC_PARSE_card_object
		  ( unsigned char		  *recv_buffer,
			const unsigned short   recv_buffer_size,
			unsigned char         *slot_number,
			unsigned short		  *received_buf_length);


/* C7-0x1021 */
short int CTC_COMPOSE_qinq
				( const unsigned char						   num_of_entries,
				  const CTC_STACK_port_qinq_configuration_t   *ports_info,
				  const unsigned char						   response_ret_value,
				  const unsigned char						   is_get_frame,
				  unsigned char								  *send_buffer,
				  unsigned short						      *send_buf_length);


/* C7-0x1021 */
short int CTC_PARSE_qinq
				( unsigned char							*recv_buffer,
				  const unsigned short					 recv_buffer_size,
				  unsigned char							*num_of_entries,
				  CTC_STACK_port_qinq_configuration_t	*ports_info,
				  unsigned char							*ctc_ret_val,
				  unsigned short						*received_buf_length);


short int CTC_PARSE_event_header
				( unsigned char					        *receive_buffer,
				  const unsigned short			         length,
				  CTC_STACK_event_header_t              *event_header,
				  unsigned short				        *received_buf_length);


short int CTC_PARSE_event_value
				( unsigned char					        *receive_buffer,
				  const unsigned short			         length,
                  const unsigned char                    alarm_info_size,
				  CTC_STACK_event_value_t               *event_value,
				  unsigned short				        *received_buf_length);

/* C7-0x0005 */
short int CTC_PARSE_optical_transceiver_diagnosis
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C7-0x0006 */
short int CTC_COMPOSE_service_sla
				( const unsigned char						num_of_entries,
				  const bool								activate,
				  const CTC_STACK_service_sla_t				*service_sla,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C7-0x0006 */
short int CTC_PARSE_service_sla
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  bool										*activate,
				  CTC_STACK_service_sla_t					*service_sla,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C7-0x0081 */
short int CTC_COMPOSE_alarm_admin_state
				( const unsigned char						      num_of_entries,
				  const CTC_STACK_alarm_id_t				      alarm_id,									
				  const bool									  enable,
				  const unsigned char							  response_ret_value,
				  const unsigned char						      is_get_frame,
				  unsigned char								      *send_buffer,
				  unsigned short							      *send_buf_length);

/* C7-0x0081 */
short int CTC_COMPOSE_get_alarm_admin_state
				( const unsigned char						      num_of_entries,
				  const CTC_STACK_alarm_id_t				      alarm_id,									
				  const unsigned char							  response_ret_value,
				  const unsigned char						      is_get_frame,
				  unsigned char								      *send_buffer,
				  unsigned short							      *send_buf_length);

/* C7-0x0081 */
short int CTC_PARSE_alarm_admin_state
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_alarm_id_t   					*alarm_id,
				  bool										*enable,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);


/* C7-0x0082 */
short int CTC_COMPOSE_alarm_threshold
				( const unsigned char						      num_of_entries,
				  const CTC_STACK_alarm_id_t					  alarm_id,									
				  const unsigned long							  alarm_threshold,
				  const unsigned long							  clear_threshold,
				  const unsigned char							  response_ret_value,
				  const unsigned char						      is_get_frame,
				  unsigned char								      *send_buffer,
				  unsigned short							      *send_buf_length);

/* C7-0x0082 */
short int CTC_COMPOSE_get_alarm_threshold
				( const unsigned char						      num_of_entries,
				  const CTC_STACK_alarm_id_t					  alarm_id,									
				  const unsigned char							  response_ret_value,
				  const unsigned char						      is_get_frame,
				  unsigned char								      *send_buffer,
				  unsigned short							      *send_buf_length);

/* C7-0x0082 */
short int CTC_PARSE_alarm_threshold
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_alarm_id_t						*alarm_id,
				  unsigned long								*alarm_threshold,
				  unsigned long								*clear_threshold,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C7-0x0051 */
short int CTC_COMPOSE_llid_queue_config
				( const unsigned char						num_of_entries,
				  const CTC_STACK_llid_queue_config_t		llid_queue_config,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C7-0x0051 */
short int CTC_PARSE_llid_queue_config
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_llid_queue_config_t				*llid_queue_config,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C9-0x0202 */
short int CTC_COMPOSE_multi_llid_admin_control
				( const unsigned char						num_of_entries,
				  const unsigned long                       num_of_llid_activated,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C7-0x0061 */
short int CTC_PARSE_voip_iad_info
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_voip_iad_info_t	                *iad_info,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C7-0x0062 */
short int CTC_COMPOSE_voip_global_param_conf
				( const unsigned char						num_of_entries,
				  const CTC_STACK_voip_global_param_conf_t	global_param,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C7-0x0062 */
short int CTC_PARSE_voip_global_param_conf
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_voip_global_param_conf_t	    *global_param,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C7-0x0063 */
short int CTC_COMPOSE_h248_param_config
				( const unsigned char						num_of_entries,
				  const CTC_STACK_h248_param_config_t    	h248_param,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C7-0x0063 */
short int CTC_PARSE_h248_param_config
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_h248_param_config_t	            *h248_param,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C7-0x0064 */
short int CTC_COMPOSE_h248_user_tid_config
				( const unsigned char						num_of_entries,
				  const CTC_STACK_h248_user_tid_config_t	h248_user_tid,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C7-0x0064 */
short int CTC_PARSE_h248_user_tid_config
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_h248_user_tid_config_t	        *h248_user_tid,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C7-0x0065 */
short int CTC_COMPOSE_h248_rtp_tid_config
				( const unsigned char						num_of_entries,
				  const CTC_STACK_h248_rtp_tid_config_t        h248_rtp_tid_config,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/*C7 -0x0066*/
short int CTC_PARSE_h248_rtp_tid_info
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_h248_rtp_tid_info_t		*h248_rtp_tid,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);
				  
/* C7-0x0067 */
short int CTC_COMPOSE_sip_param_config
				( const unsigned char						num_of_entries,
				  const CTC_STACK_sip_param_config_t	    sip_param,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C7-0x0067 */
short int CTC_PARSE_sip_param_config
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_sip_param_config_t	            *sip_param,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C7-0x0068 */
short int CTC_COMPOSE_sip_user_param_config
				( const unsigned char						num_of_entries,
				  const CTC_STACK_sip_user_param_config_t	sip_user_param,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C7-0x0068 */
short int CTC_PARSE_sip_user_param_config
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_sip_user_param_config_t	        *sip_user_param,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C7-0x0069 */
short int CTC_COMPOSE_voip_fax_config
				( const unsigned char						num_of_entries,
				  const CTC_STACK_voip_fax_config_t	        voip_fax,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C7-0x0069 */
short int CTC_PARSE_voip_fax_config
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_voip_fax_config_t	            *voip_fax,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C7-0x006A */
short int CTC_PARSE_voip_iad_oper_status
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_voip_iad_oper_status_t	        *iad_oper_status,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C7-0x006B */
short int CTC_PARSE_voip_pots_status
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_voip_pots_status_t	        *pots_status,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);

/* C9-0x006C */
short int CTC_COMPOSE_voip_iad_operation
				( const unsigned char						num_of_entries,
				  const CTC_STACK_operation_type_t              operation_type,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C9-0x006D*/
short int CTC_COMPOSE_sip_digit_map
				( const unsigned char						num_of_entries,
				  const unsigned char*                		sip_digit_map,
				  const unsigned char						num_of_block,
				  const unsigned char						block_no,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C7-0x0007 */
short int CTC_COMPOSE_onu_capabilities_2
				( const unsigned char					 num_of_entries,
				  const CTC_STACK_onu_capabilities_2_t	*onu_capabilities,
				  const unsigned char					 response_ret_value,
				  const unsigned char					 is_get_frame,
				  unsigned char							*send_buffer,
				  unsigned short						*send_buf_length);

/* C7-0x0007 */
short int CTC_PARSE_onu_capabilities_2
				( unsigned char					*recv_buffer,
				  const unsigned short			 recv_buffer_size,
				  unsigned char				    *num_of_entries,
				  CTC_STACK_onu_capabilities_2_t  *onu_capabilities,
				  unsigned char					*ctc_ret_val,
				  unsigned short				*received_buf_length);

/* C7-0x000C */
short int CTC_COMPOSE_onu_capabilities_3
				( const unsigned char					 num_of_entries,
				  const CTC_STACK_onu_capabilities_3_t	*onu_capabilities,
				  const unsigned char					 response_ret_value,
				  const unsigned char					 is_get_frame,
				  unsigned char							*send_buffer,
				  unsigned short						*send_buf_length);

/* C7-0x000C */
short int CTC_PARSE_onu_capabilities_3
				( unsigned char					*recv_buffer,
				  const unsigned short			 recv_buffer_size,
				  unsigned char				    *num_of_entries,
				  CTC_STACK_onu_capabilities_3_t  *onu_capabilities,
				  unsigned char					*ctc_ret_val,
				  unsigned short				*received_buf_length);

/* C7-0x0081 */
short int CTC_PARSE_mxu_mng_global_parameter_config
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t *parameter,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);
/* C7-0x0081 */
short int CTC_COMPOSE_mxu_mng_global_parameter_config
				( const unsigned char						num_of_entries,
				  const CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t              parameter,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);
/* C7-0x0082 */
short int CTC_PARSE_mxu_mng_snmp_parameter_config
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t *parameter,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);
/* C7-0x0081 */
short int CTC_COMPOSE_mxu_mng_snmp_parameter_config
				( const unsigned char						num_of_entries,
				  const CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t              parameter,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);


/* C7-0x0017 */
short int CTC_COMPOSE_port_loop_detect
				( const unsigned char	num_of_entries,
				  const unsigned long		   *loop_detect_enable,
				  const unsigned char	response_ret_value,
				  const unsigned char	is_get_frame,
				  unsigned char		   *send_buffer,
				  unsigned short	   *send_buf_length);

/* C7-0x0017 */
short int CTC_PARSE_port_loop_detect
				( unsigned char			  *recv_buffer,
				  const unsigned short	   recv_buffer_size,
				  /*const unsigned char	   is_get_frame,*/
				  unsigned char			  *num_of_entries,
				  unsigned long		   *loop_detect_enable,
				  unsigned char		      *ctc_ret_val,
				  unsigned short		  *received_buf_length);

/* C7-0x0008 */
short int CTC_PARSE_holdover_state
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_holdover_state_t *parameter,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);
/* C7-0x0008 */
short int CTC_COMPOSE_holdover_state
				( const unsigned char						num_of_entries,
				  const CTC_STACK_holdover_state_t   parameter,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C7-0x000B */
short int CTC_PARSE_active_pon_if_state
				( unsigned char								*recv_buffer,
				  const unsigned short						recv_buffer_size,
				  unsigned char								*num_of_entries,
				  CTC_STACK_active_pon_if_state_t *parameter,
				  unsigned char								*ctc_ret_val,
				  unsigned short							*received_buf_length);
/* C7-0x000B */
short int CTC_COMPOSE_active_pon_if_state
				( const unsigned char						num_of_entries,
				  const CTC_STACK_active_pon_if_state_t   parameter,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

/* C7-0x00A1 */
short int CTC_COMPOSE_onu_tx_power_supply_control
				( const unsigned char						num_of_entries,
				  const CTC_STACK_onu_tx_power_supply_control_t   parameter,
				  const unsigned char						response_ret_value,
				  const unsigned char						is_get_frame,
				  unsigned char								*send_buffer,
				  unsigned short							*send_buf_length);

#endif /* __CTC_COMPARSE_EXPO_H__ */

