
/*----------------------------------------------------------------------------*/
#ifndef __INCCT_RMan_EthPorth
#define __INCCT_RMan_EthPorth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CTC_OBSOLETE		/* removed by xieshl 20120601 */
#ifdef __CT_EXTOAM_SUPPORT

#define	MAX_ABILITY_ENUM	64

typedef struct {
	char portpolicyope;
	ULONG ppCIR;
	ULONG ppCBS;
	ULONG ppEBS;
}__attribute__((packed))  CT_RMan_EthPortPolicing_t;

typedef struct{
	ULONG	enumnum;
	ULONG	enumval[MAX_ABILITY_ENUM];
}__attribute__((packed))CT_RMan_EthAutoNegAbility_t;

#endif /*__CT_EXTOAM_SUPPORT*/


/* added by xieshl 20070321 
#ifndef MAXPON
#define	MAXPON  20
#endif
#ifndef MAX_ONU_ETHPORT
#define	MAX_ONU_ETHPORT	24
#endif
#ifdef MAXONUPERPON
#define MAXONUPERPON   64
#endif
*/
#define  __ONU_CONFIG_AUTO

enum {
	ethPortQoSRuleMatchOperator_never = 1,
	ethPortQoSRuleMatchOperator_equal = 2,
	ethPortQoSRuleMatchOperator_not_equal = 3,
	ethPortQoSRuleMatchOperator_less_equal = 4,
	ethPortQoSRuleMatchOperator_greater_equal = 5,
	ethPortQoSRuleMatchOperator_exist = 6,
	ethPortQoSRuleMatchOperator_not_exist = 7,
	ethPortQoSRuleMatchOperator_always = 8
};
typedef struct {
	ulong_t  ethPortPolicingCIR;		/*	承诺平均速率，范围64～1000000Kbps*/
	ulong_t  ethPortPolicingCBS;		/*	承诺突发尺寸，范围0～10000000byte */
	ulong_t  ethPortPolicingEBS;		/*	最大突发尺寸，范围0～10000000byte */
	uchar_t  ethPortPolicingEnable;	/*	Policing使能*/

	ushort_t ethPortVlanTagTpid;		/*	只对ethPortVlanMode!=1有效，默认0x8100 */
	ushort_t ethPortVlanTagPri_Cfi_Vid;		/*	只对ethPortVlanMode!=1时有效，默认1*/
	uchar_t  ethPortVlanMode;			/*	0-transparent、1-tag、2-translation, 3-aggregation, 4-trunk */
	/*uchar_t  ethPortVlanTagCfi;*/		/*	只对ethPortVlanMode!=1有效，默认0 */
	/*uchar_t  ethPortVlanTagPri;*/		/*	只对ethPortVlanMode!=1有效，默认0*/
	/*ushort_t ethPortVlanTagVid;*/		/*	只对ethPortVlanMode!=1时有效，默认1*/

/*BEGIN:	added by wangxy 2007-07-17*/
	uchar_t  ethPortDefaultQueueMapped;
	uchar_t  ethPortDefaultPriMark;
/*END*/	

/*BEGIN: added by zhengyt 2008-6-26*/
	uchar_t  ethPortDSPolicingEnable;
	uchar_t  ethPortDefaultMauType;
	ulong_t ethPortDSPolicingCIR;     /*下行方向*/
	ulong_t  ethPortDSPolicingPIR;
/*END*/
#if 0 /*marked by wangxy 2007-05-22*/
	uchar_t  ethPortQoSQueueMapped;	/*	本规则所映射的优先级队列*/
	uchar_t  ethPortQoSPriMark;			/*	本规则的优先级标记*/
	uchar_t  ethPortQoSRuleEntriesSelect;		/*	{da_mac(1),sa_mac(2),pri(3),vlan(4), thType(5),d_ip(6), s_ip(7), ipType(8), ip4_tos_dscp(9), ip6_precedence(10), s_l4_port(11),d_l4_port(12), other(13) }	规则匹配条件*/
	uchar_t  ethPortQoSRuleEntriesMatchVal[20];	/*		匹配值*/
	uchar_t  ethPortQoSRuleMatchOperator;
	uchar_t  ethPortQoSRuleAction;		/* 1-noop, 2-delete, 3-add, 4-delAll, 5-get, 6-processing */
#endif

#ifdef __ONU_CONFIG_AUTO
	uchar_t  ethPortAdminStatus;
	uchar_t  ethPortAutoNegAdminStatus;
	uchar_t  ethPortPauseAdminMode;
	uchar_t  ethPortMultiTagStriped;
#endif

	USHORT ethCtcVlanAggrSelect;
	USHORT ethCtcVlanTrunkSelect;
	USHORT ethCtcMVlanSwitchSelect;

#if 0
	ushort_t ethPortMulticastVlanVid;	/* 临时定义，一个端口只能设置一个组播vlan*/

#endif
}CT_Onu_EthPortItem_t;

#ifndef MAXONUPERPON
#error "MAXONUPERPON is need!"
#endif
#ifndef MAX_ONU_ETHPORT
#define MAX_ONU_ETHPORT 24
#endif
#if 0
extern CT_Onu_EthPortItem_t  onuEthPort[20][MAXONUPERPON][MAX_ONU_ETHPORT];
#else
extern CT_Onu_EthPortItem_t  (*onuEthPort)[MAXONUPERPONNOLIMIT][MAX_ONU_ETHPORT];
#endif
extern ULONG onuEthPortSemId;

extern int CTC_ethPortPolicingCIR_get( ULONG devIdx,  ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortPolicingCIR_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortPolicingCBS_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortPolicingCBS_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortPolicingEBS_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortPolicingEBS_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortPolicingEnable_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortPolicingEnable_set( ULONG devIdx, ULONG portIdx,  ULONG val);

extern int CTC_ethPortVlanTagTpid_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortVlanTagTpid_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortVlanTagCfi_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortVlanTagCfi_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortVlanTagPri_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortVlanTagPri_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortVlanTagVid_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortVlanTagVid_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortVlanMode_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortVlanMode_set( ULONG devIdx, ULONG portIdx,  ULONG val);

extern STATUS CTC_ethPortVlanAction_get( ULONG devIdx, ULONG portIdx );
extern STATUS CTC_ethPortVlanAction_set( ULONG devIdx, ULONG portIdx,  ULONG val );

/*extern int CTC_ethPortMulticastVlanVid_get( PON_olt_id_t olt_id,  PON_onu_id_t onu_id,  unsigned char port_id,  ULONG *pVal);
extern int CTC_ethPortMulticastVlanVid_set( PON_olt_id_t olt_id,  PON_onu_id_t onu_id,  unsigned char port_id,  ULONG val);*/

extern int CTC_ethPortQoSQueueMapped_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortQoSQueueMapped_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortQoSPriMark_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortQoSPriMark_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortQoSRuleEntriesSelect_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortQoSRuleEntriesSelect_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortQoSRuleEntriesMatchVal_get( ULONG devIdx, ULONG portIdx,  UCHAR *pVal, ULONG *pValLen);
extern int CTC_ethPortQoSRuleEntriesMatchVal_set( ULONG devIdx, ULONG portIdx,  UCHAR *pVal, ULONG valLen );
extern int CTC_ethPortQoSRuleMatchOperator_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortQoSRuleMatchOperator_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortQoSRuleAction_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortQoSRuleAction_set( ULONG devIdx, ULONG portIdx,  ULONG val);

#ifdef __ONU_CONFIG_AUTO
extern int CTC_ethPortAdminStatus_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortAdminStatus_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortAutoNegAdminStatus_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortAutoNegAdminStatus_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortPauseAdminMode_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortPauseAdminMode_set( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int CTC_ethPortMultiTagStriped_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_ethPortMultiTagStriped_set( ULONG devIdx, ULONG portIdx,  ULONG val);
#endif
#if 0
extern int CTC_ethPortAutoNegRestart_get( PON_olt_id_t olt_id,  PON_onu_id_t onu_id,  unsigned char port_id,  ULONG *pVal);
extern int CTC_ethPortAutoNegRestart_set( PON_olt_id_t olt_id,  PON_onu_id_t onu_id,  unsigned char port_id,  ULONG val);
#endif

#if 0  
___NOT_INCLUDE_FROM_PAS__

#ifndef MAX_PORT_ENTRIES_NUMBER
#define MAX_PORT_ENTRIES_NUMBER 24

typedef enum
{
    CTC_STACK_STATE_DEACTIVATE	= 0x00,
	CTC_STACK_STATE_ACTIVATE	= 0x01,
} CTC_STACK_on_off_state_t;

typedef struct
{
	CTC_STACK_on_off_state_t	operation;
	unsigned long int			cir;
	unsigned long int			bucket_depth;
	unsigned long int			extra_burst_size;
} CTC_STACK_ethernet_port_policing_entry_t;

typedef struct
{
	unsigned char		port_number;
	unsigned char		flow_control_enable;
} CTC_STACK_ethernet_port_pause_entry_t;
typedef  CTC_STACK_ethernet_port_pause_entry_t CTC_STACK_ethernet_ports_pause_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	unsigned char				port_number;
	CTC_STACK_ethernet_port_policing_entry_t	entry;
} CTC_STACK_ethernet_policing_entry_t;

typedef  CTC_STACK_ethernet_policing_entry_t  CTC_STACK_ethernet_ports_policing_t[MAX_PORT_ENTRIES_NUMBER];

typedef struct
{
	unsigned char				port_number;
	CTC_STACK_on_off_state_t	port_state;
} CTC_STACK_port_state_entry_t;

typedef  CTC_STACK_port_state_entry_t  CTC_STACK_ports_state_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	unsigned char		port_number;
	unsigned char		state;
} CTC_STACK_ethernet_port_state_entry_t;

typedef  CTC_STACK_ethernet_port_state_entry_t CTC_STACK_ethernet_ports_state_t[MAX_PORT_ENTRIES_NUMBER];


typedef enum
{
	CTC_STACK_TECHNOLOGY_ABILITY_UNDEFINED		= 1,
	CTC_STACK_TECHNOLOGY_ABILITY_UNKNOWN		= 2,
    CTC_STACK_TECHNOLOGY_ABILITY_10_BASE_T		= 14,
	CTC_STACK_TECHNOLOGY_ABILITY_10_BASE_TFD	= 142,
	CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_T4	= 23,
	CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_TX	= 25,
	CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_TXFD	= 252,
	CTC_STACK_TECHNOLOGY_ABILITY_FDX_PAUSE		= 312,
	CTC_STACK_TECHNOLOGY_ABILITY_FDX_A_PAUSE	= 313,
	CTC_STACK_TECHNOLOGY_ABILITY_FDX_S_PAUSE	= 314,
	CTC_STACK_TECHNOLOGY_ABILITY_FDX_B_PAUSE	= 315,
	CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_T2	= 32,
	CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_T2FD	= 322,
	CTC_STACK_TECHNOLOGY_ABILITY_1000_BASE_X	= 36,
	CTC_STACK_TECHNOLOGY_ABILITY_1000_BASE_XFD	= 362,
	CTC_STACK_TECHNOLOGY_ABILITY_1000_BASE_T	= 40,
	CTC_STACK_TECHNOLOGY_ABILITY_1000_BASE_TFD	= 402,
	CTC_STACK_TECHNOLOGY_ABILITY_REM_FAULT1		= 37,
	CTC_STACK_TECHNOLOGY_ABILITY_REM_FAULT2		= 372,
	CTC_STACK_TECHNOLOGY_ABILITY_ISO_ETHERNET	= 8029,
	
} CTC_STACK_technology_ability_t;

#define MAX_TECHNOLOGY_ABILITY 20
typedef struct
{
	unsigned char				number_of_abilities;
	CTC_STACK_technology_ability_t	technology[MAX_TECHNOLOGY_ABILITY];

} CTC_STACK_auto_negotiation_technology_ability_t;

typedef struct
{
	unsigned char										port_number;
	CTC_STACK_auto_negotiation_technology_ability_t		abilities;
} CTC_STACK_auto_negotiation_port_technology_ability_t;

typedef  CTC_STACK_auto_negotiation_port_technology_ability_t CTC_STACK_auto_negotiation_ports_technology_ability_t[MAX_PORT_ENTRIES_NUMBER];


typedef enum
{
    CTC_STACK_LINK_STATE_DOWN	= 0x00,
	CTC_STACK_LINK_STATE_UP	= 0x01,
} CTC_STACK_link_state_t;

typedef struct
{
	unsigned char						port_number;
	CTC_STACK_link_state_t				link_state;
} CTC_STACK_ethernet_link_state_entry_t;

typedef  CTC_STACK_ethernet_link_state_entry_t CTC_STACK_ethernet_ports_link_state_t[MAX_PORT_ENTRIES_NUMBER];


extern PON_STATUS CTC_STACK_get_ethernet_link_state ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			CTC_STACK_link_state_t  *link_state);
extern PON_STATUS CTC_STACK_get_ethernet_all_port_link_state ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			unsigned char			*number_of_entries,
			CTC_STACK_ethernet_ports_link_state_t   ports_link_state );
extern PON_STATUS CTC_STACK_set_ethernet_port_pause ( 
			const PON_olt_id_t	 	olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			const unsigned char	flow_control_enable);
extern PON_STATUS CTC_STACK_get_ethernet_port_pause ( 
			const PON_olt_id_t	 	olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			unsigned char		    	*flow_control_enable);
extern PON_STATUS CTC_STACK_get_ethernet_all_port_pause ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			unsigned char			*number_of_entries,
			CTC_STACK_ethernet_ports_pause_t  ports_info );
extern PON_STATUS CTC_STACK_set_ethernet_port_policing ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			const CTC_STACK_ethernet_port_policing_entry_t   port_policing);
extern PON_STATUS CTC_STACK_get_ethernet_port_policing ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			CTC_STACK_ethernet_port_policing_entry_t  *port_policing);
extern PON_STATUS CTC_STACK_get_ethernet_all_port_policing ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			unsigned char			*number_of_entries,
			CTC_STACK_ethernet_ports_policing_t   ports_info );


extern PON_STATUS CTC_STACK_get_phy_admin_state ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			unsigned char			*state );
extern PON_STATUS CTC_STACK_get_all_port_phy_admin_state ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			unsigned char			*number_of_entries,
			CTC_STACK_ethernet_ports_state_t    ports_info );
extern PON_STATUS CTC_STACK_set_phy_admin_control ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			const unsigned char	state);
extern PON_STATUS CTC_STACK_get_auto_negotiation_admin_state ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char   	port_number,
			unsigned char			*state );
extern PON_STATUS CTC_STACK_get_auto_negotiation_all_ports_admin_state ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			unsigned char			*number_of_entries,
			CTC_STACK_ethernet_ports_state_t   ports_info );
extern PON_STATUS CTC_STACK_get_auto_negotiation_local_technology_ability ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			CTC_STACK_auto_negotiation_technology_ability_t  *abilities );
extern PON_STATUS CTC_STACK_get_auto_negotiation_all_ports_local_technology_ability ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			unsigned char			*number_of_entries,
			CTC_STACK_auto_negotiation_ports_technology_ability_t   ports_abilities );
extern PON_STATUS CTC_STACK_get_auto_negotiation_advertised_technology_ability ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			CTC_STACK_auto_negotiation_technology_ability_t  *abilities );
extern PON_STATUS CTC_STACK_get_auto_negotiation_all_ports_advertised_technology_ability ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			unsigned char			*number_of_entries,
			CTC_STACK_auto_negotiation_ports_technology_ability_t   ports_abilities );

extern PON_STATUS CTC_STACK_set_auto_negotiation_restart_auto_config ( 
			const PON_olt_id_t	 	olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char   	port_number);
extern PON_STATUS CTC_STACK_set_auto_negotiation_admin_control ( 
			const PON_olt_id_t	 	olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char   	port_number,
			const unsigned char	state);

#endif /*MAX_PORT_ENTRIES_NUMBER*/

#ifdef  PAS_SOFT_VERSION_V5_3_5

#ifndef bool
#define bool unsigned char /* TRUE / FALSE */
#endif 


typedef enum
{
    CTC_STACK_FAST_LEAVE_ADMIN_STATE_DISABLED	= 0x00000001,
	CTC_STACK_FAST_LEAVE_ADMIN_STATE_ENABLED	= 0x00000002
} CTC_STACK_fast_leave_admin_state_t;


/* aFastLeaveAbility */
#define CTC_MAX_FAST_LEAVE_ABILITY_MODES	4
typedef enum
{
	CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_NON_FAST_LEAVE_IN_IGMP_SNOOPING_MODE	= 0x00000000,
	CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_FAST_LEAVE_IN_IGMP_SNOOPING_MODE		= 0x00000001,
	CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_NON_FAST_LEAVE_IN_MC_CONTROL_MODE		= 0x00000010,
	CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_FAST_LEAVE_IN_MC_CONTROL_MODE			= 0x00000011
} CTC_STACK_fast_leave_ability_mode_t;

typedef struct
{
	CTC_STACK_fast_leave_ability_mode_t		modes[CTC_MAX_FAST_LEAVE_ABILITY_MODES];
	unsigned short							num_of_modes; /* The count of modes within <modes> */
} CTC_STACK_fast_leave_ability_t;

typedef struct
{
	unsigned char		port_number;
	unsigned long		CIR;	/* Committed Information Rate of the port (in Kbps) */
	unsigned long		PIR;	/* Peak Information Rate of the port (in Kbps) */
	bool				state;	/* Enable or Disable */
} CTC_STACK_ethernet_port_ds_rate_limiting_entry_t;

typedef  CTC_STACK_ethernet_port_ds_rate_limiting_entry_t CTC_STACK_ethernet_ports_ds_rate_limiting_t[MAX_PORT_ENTRIES_NUMBER];

/* Set fast leave admin control
**
** This function set ONU multicast fast leave admin control
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		fast_leave_admin_state	: fast leave admin state
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_set_fast_leave_admin_control ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   CTC_STACK_fast_leave_admin_state_t     fast_leave_admin_state );

		   
		   
/* Get fast leave admin state
**
** This function retrieves ONU multicast fast leave admin state
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		fast_leave_admin_state	: fast leave admin state 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_fast_leave_admin_state ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   CTC_STACK_fast_leave_admin_state_t	*fast_leave_admin_state );



/* Get fast leave ability
**
** This function retrieves ONU multicast fast leave ability
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		fast_leave_ability		: fast leave ability
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_fast_leave_ability ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   CTC_STACK_fast_leave_ability_t		*fast_leave_ability );



/* Get all ethernet port ds rate limiting
**
** This function retrieves all ONU ethernet ports downstream rate limiting configuration
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries		: Number of valid entries in ports info parameter
**		ports_ds_rate_limiting	: See CTC_STACK_ethernet_ports_ds_rate_limiting_t for details
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_all_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   unsigned char								*number_of_entries,
		   CTC_STACK_ethernet_ports_ds_rate_limiting_t	ports_ds_rate_limiting );


/* Get ethernet port ds rate limiting
**
** This function retrieves ONU ethernet port downstream rate limiting configuration
**
** Input Parameters:
**		olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_VOIP_PORT_NUMBER - CTC_MAX_VOIP_PORT_NUMBER, CTC_STACK_ALL_PORTS
**
** Output Parameters:
**		rate_limiting_entry	: See CTC_STACK_ethernet_port_ds_rate_limiting_entry_t for details
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t								olt_id, 
           const PON_onu_id_t								onu_id,
		   const unsigned char								port_number,
		   CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting );



/* Set ethernet port ds rate limiting
**
** This function set ONU ethernet port downstream rate limiting configuration
**
** Input Parameters:
**		olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			: port number. CTC_MIN_VOIP_PORT_NUMBER - CTC_MAX_VOIP_PORT_NUMBER, CTC_STACK_ALL_PORTS
**		rate_limiting_entry	: See CTC_STACK_ethernet_port_ds_rate_limiting_entry_t for details
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
extern PON_STATUS CTC_STACK_set_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t										olt_id, 
           const PON_onu_id_t										onu_id,
		   const unsigned char										port_number,
		   const CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting);



#endif
#endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCCT_RMan_EthPorth */
