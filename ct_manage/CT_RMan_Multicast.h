
/*----------------------------------------------------------------------------*/
#ifndef __INCCT_RMan_Multicasth
#define __INCCT_RMan_Multicasth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "includeFromPas.h"

#ifdef __CT_EXTOAM_SUPPORT

enum{
	MULTICAST_VID_DEL=0,
	MULTICAST_VID_ADD,
	MULTICAST_VID_CLR,
	MULTICAST_VID_LIST
};

#define	MAX_MULTICAST_VID	128
#define MAX_MULTICTRL_ITEM  64

typedef struct{
	USHORT	usrvid;
	USHORT  multivid;
	char    mac[6];
}__attribute__((packed))multictrl_t;

typedef struct {
	char	actcode;
	short int vlanid[MAX_MULTICAST_VID];
	uchar	vidnum;
} __attribute__((packed))  CT_RMan_MultiVlan_t;

typedef struct {
	char action;
	char ctrltype;
	char itemnum;
	multictrl_t items[MAX_MULTICTRL_ITEM];
} __attribute__((packed))  CT_RMan_MultiCtrl_t;

#endif /*__CT_EXTOAM_SUPPORT*/

#ifdef CTC_OBSOLETE		/* removed by xieshl 20120601 */
#define CTC_MULTICAST_VLAN_SWITCH_ID_MAX  10000

typedef struct __eth_ctc_multicast_vlan_switch_listnode{
	ULONG  ctcMVlanSwitchGroupId;
	USHORT multicast_vlan[CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES];
	USHORT iptv_user_vlan[CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES];
	struct __eth_ctc_multicast_vlan_switch_listnode *pNextNode;
}__attribute__((packed))eth_ctc_multicast_vlan_switch_list_t;

extern eth_ctc_multicast_vlan_switch_list_t *CTC_addCtcMVlanSwitchNode( const ULONG grpId );
extern eth_ctc_multicast_vlan_switch_list_t *CTC_findNextCtcMVlanSwitchNode( const ULONG grpId );
extern int CTC_addMVlanSwitch( ULONG grpId, ULONG multiVid, ULONG userVid );
extern STATUS CTC_delMVlanSwitch( ULONG grpId, ULONG multiVid, ULONG userVid );
extern int CTC_getEthPortMVlanSwitchGroup( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_setEthPortMVlanSwitchGroup( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int mn_getFirstCtcMVlanSwitchEntryIndex( ULONG *pGrpId, ULONG *pMultiVid, ULONG *pUserVid );
extern int mn_getNextCtcMVlanSwitchEntryIndex( ULONG grpId, ULONG multiVid, ULONG userVid, ULONG *pNextGrpId, ULONG *pNextMultiVid, ULONG *pNextUserVid );
extern int mn_checkCtcMVlanSwitchEntryIndex( ULONG grpId, ULONG multiVid, ULONG userVid );
extern int mn_getCtcMVlanSwitchEntryStatus( ULONG grpId, ULONG multiVid, ULONG userVid, ULONG *pStatus );
extern int mn_setCtcMVlanSwitchEntryStatus( ULONG grpId, ULONG multiVid, ULONG userVid, ULONG status );
extern int mn_getEthPortMVlanSwitchGrpId( ULONG devIdx, ULONG portIdx,  ULONG *pGrpId);
extern int mn_setEthPortMVlanSwitchGrpId( ULONG devIdx, ULONG portIdx,  ULONG grpId);
#endif

#if 0
#ifndef CTC_MAX_MULTICAST_VLAN_ENTRIES
#define CTC_MAX_MULTICAST_VLAN_ENTRIES 50

typedef enum
{
	CTC_MULTICAST_VLAN_OPER_DELETE	= 0x00,
	CTC_MULTICAST_VLAN_OPER_ADD		= 0x01,
	CTC_MULTICAST_VLAN_OPER_CLEAR	= 0x02,
	CTC_MULTICAST_VLAN_OPER_LIST	= 0x03,
} CTC_STACK_multicast_vlan_opers_t;

typedef struct
{
	CTC_STACK_multicast_vlan_opers_t	vlan_operation;
	unsigned short						vlan_id[CTC_MAX_MULTICAST_VLAN_ENTRIES];
	unsigned char						num_of_vlan_id;	/* Number of entries used in <vlan_id> */
} CTC_STACK_multicast_vlan_t;



/* C7-0x0042 */
#define CTC_MAX_MULTICAST_MAC_ENTRIES 32
typedef enum
{
	CTC_STACK_MULTICAST_CONTROL_TYPE_DA_ONLY	= 0x00,
	CTC_STACK_MULTICAST_CONTROL_TYPE_DA_VID		= 0x01,
} CTC_STACK_multicast_control_type_t;

typedef enum
{
	CTC_MULTICAST_CONTROL_ACTION_DELETE		= 0x00,
	CTC_MULTICAST_CONTROL_ACTION_ADD		= 0x01,
	CTC_MULTICAST_CONTROL_ACTION_CLEAR		= 0x02,
	CTC_MULTICAST_CONTROL_ACTION_LIST		= 0x03
} CTC_STACK_multicast_control_action_t;


typedef struct
{
	mac_address_t	da;
	unsigned short	vid;	/* Multicast VLAN ID */
	unsigned short	user_id;
} CTC_STACK_multicast_entry_t;



typedef struct
{
	CTC_STACK_multicast_control_action_t	action;
	CTC_STACK_multicast_control_type_t	control_type;
	CTC_STACK_multicast_entry_t			entries[CTC_MAX_MULTICAST_MAC_ENTRIES];
	unsigned char					num_of_entries;	/* The count of rules within <entries> */
	
} CTC_STACK_multicast_control_t;


typedef struct
{
	unsigned char				port_number;
	CTC_STACK_multicast_vlan_t		entry;
} CTC_STACK_multicast_vlan_entry_t;

typedef  CTC_STACK_multicast_vlan_entry_t CTC_STACK_multicast_vlan_ports_t[MAX_PORT_ENTRIES_NUMBER];

typedef struct
{
	unsigned char port_number;
	unsigned char group_num;
} CTC_STACK_multicast_group_num_t;

typedef  CTC_STACK_multicast_group_num_t CTC_STACK_multicast_ports_group_num_t[MAX_PORT_ENTRIES_NUMBER];


typedef struct
{
	unsigned char		port_number;
	unsigned char		tag_strip;
} CTC_STACK_multicast_port_tag_strip_t;

typedef  CTC_STACK_multicast_port_tag_strip_t CTC_STACK_multicast_ports_tag_strip_t[MAX_PORT_ENTRIES_NUMBER];



extern PON_STATUS CTC_STACK_set_multicast_vlan ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			const CTC_STACK_multicast_vlan_t   multicast_vlan);
extern PON_STATUS CTC_STACK_clear_multicast_vlan ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number);
extern PON_STATUS CTC_STACK_get_multicast_vlan ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			CTC_STACK_multicast_vlan_t  *multicast_vlan);

extern PON_STATUS CTC_STACK_get_multicast_vlan_all_port ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			unsigned char			*number_of_entries,
			CTC_STACK_multicast_vlan_ports_t   ports_info );
extern PON_STATUS CTC_STACK_set_multicast_control ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const CTC_STACK_multicast_control_t   multicast_control );
extern PON_STATUS CTC_STACK_clear_multicast_control ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id );
extern PON_STATUS CTC_STACK_get_multicast_control ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			CTC_STACK_multicast_control_t  *multicast_control );
extern PON_STATUS CTC_STACK_set_multicast_tag_strip ( 
			const PON_olt_id_t	 	olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			const unsigned char	tag_strip);
extern PON_STATUS CTC_STACK_get_multicast_tag_strip ( 
			const PON_olt_id_t	 	olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			unsigned char	    		*tag_strip);
extern PON_STATUS CTC_STACK_get_multicast_all_port_tag_strip ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			unsigned char			*number_of_entries,
			CTC_STACK_multicast_ports_tag_strip_t   ports_info );


extern PON_STATUS CTC_STACK_set_multicast_group_num ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			const unsigned char	group_num);
extern PON_STATUS CTC_STACK_get_multicast_group_num ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			unsigned char			*group_num);
extern PON_STATUS CTC_STACK_get_multicast_all_port_group_num ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			unsigned char			*number_of_entries,
			CTC_STACK_multicast_ports_group_num_t   ports_group_num );

#endif
#endif


/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCCT_RMan_Multicasth */
