
/*----------------------------------------------------------------------------*/
#ifndef __INCCT_RMan_VoipPorth
#define __INCCT_RMan_VoipPorth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __CT_EXTOAM_SUPPORT


typedef struct {
	UCHAR OUI[3];
	UCHAR extOpcode;
	UCHAR instanceBranch;
	USHORT instanceLeaf;
	UCHAR instanceWidth;
	UCHAR instanceValue;
	UCHAR variableBranch;
	USHORT variableLeaf;
} __attribute__((packed))  CT_ExtOam_PduData_Voip_Req_t;


typedef struct {
	CT_ExtOam_PduData_Voip_Req_t	descriptor;
	UCHAR variableWidth;
	UCHAR enable;
} __attribute__((packed))  CT_ExtOam_PduData_Voip_Resp_t;

typedef enum
{
	CT_Rman_Voip_Luck = 0x00,
	CT_Rman_Voip_Unluck = 0x01
} CT_RMan_Voip_PortEbl_t;


extern int CT_RMan_VoipPortEbl_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR VoipPort, 
			UCHAR call_flag, ULONG call_notify, UCHAR* pVoipPortEbl );
extern int CT_RMan_VoipPortEbl_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR VoipPort, 
			UCHAR call_flag, ULONG call_notify, UCHAR VoipPortEbl );

#endif /*__CT_EXTOAM_SUPPORT*/
	

extern PON_STATUS CTC_STACK_set_voip_port ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			const CTC_STACK_on_off_state_t   port_state );
extern PON_STATUS CTC_STACK_get_voip_port ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			CTC_STACK_on_off_state_t	*port_state);
extern PON_STATUS CTC_STACK_get_voip_all_port ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			unsigned char			*number_of_entries,
			CTC_STACK_ports_state_t	ports_info );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCCT_RMan_VoipPorth */
