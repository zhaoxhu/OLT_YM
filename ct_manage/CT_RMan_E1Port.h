
/*----------------------------------------------------------------------------*/
#ifndef __INCCT_RMan_E1Porth
#define __INCCT_RMan_E1Porth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __CT_EXTOAM_SUPPORT


#endif /*__CT_EXTOAM_SUPPORT*/


extern PON_STATUS CTC_STACK_set_e1_port ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			const CTC_on_off_state_t   port_state );
extern PON_STATUS CTC_STACK_get_e1_port ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			CTC_on_off_state_t  *port_state);
extern PON_STATUS CTC_STACK_get_e1_all_port ( 
			const PON_olt_id_t		 olt_id, 
			const PON_onu_id_t	 onu_id,
			unsigned char		*number_of_entries,
			CTC_ports_state_t   ports_info );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCCT_RMan_E1Porth */
