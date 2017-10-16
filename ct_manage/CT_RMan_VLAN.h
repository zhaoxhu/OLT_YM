
/*----------------------------------------------------------------------------*/
#ifndef __INCCT_RMan_VLANh
#define __INCCT_RMan_VLANh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __CT_EXTOAM_SUPPORT

#ifdef CTC_OBSOLETE		/* removed by xieshl 20120611 */

#define MAX_VLAN_TRANS	128    /*端口最大VLAN转换数*/

enum{
VLAN_TRANSPARENT = 0,
VLAN_TAG,
VLAN_TRANSLATION
};

typedef struct{
long	oriVlanTag;
long	desVlanTag;
}TRAN_ITEM;

typedef struct {
	char vlanmode;	/*vlan工作模式 0:transparent 1:tag 2:translation*/
	LONG vlantag;	/*vlanmode==1为要添加的vlan tag, 2时为被替换的vlan tag*/
	TRAN_ITEM transitem[MAX_VLAN_TRANS];
	char tranitemnum;	/*设置的vlan转换条目数*/
}__attribute__((packed))  CT_RMan_PVlan_t;

#endif /*__CT_EXTOAM_SUPPORT*/

/*#ifndef CTC_MAX_VLAN_AGGREGATION_TABLES
#define CTC_MAX_VLAN_AGGREGATION_TABLES 4
#endif
#ifndef CTC_MAX_VLAN_AGGREGATION_ENTRIES
#define CTC_MAX_VLAN_AGGREGATION_ENTRIES 8
#endif
#ifndef CTC_MAX_VLAN_FILTER_ENTRIES
#define CTC_MAX_VLAN_FILTER_ENTRIES 31
#endif*/
typedef struct Alarm_Onu_info{
	USHORT	AlarmPolicyId;
	UCHAR	EquipmentAlarm;
	UCHAR	PowerAlarm;
	UCHAR	BatteryMissing;
	UCHAR	BatteryFailure;
	
	UCHAR	BatteryVoltLow;
	LONG   	BatteryVoltLowAlarmThr;
	LONG   	BatteryVoltLowClearThr;	
	
	UCHAR	PhysicalIntrusionAlarm;
	UCHAR	OnuSelfTestFailure;
		
	UCHAR	OnuTempHighAlarm;
	UCHAR	OnuTempLowAlarm;
	LONG   	OnuTempHighAlarmThr;
	LONG   	OnuTempHighClearThr;
	LONG   	OnuTempLowAlarmThr;
	LONG   	OnuTempLowClearThr;	
	
	UCHAR	IadConnectFailure;
	UCHAR	PonSwitch;
	struct Alarm_Onu_info *nextNode;
}__attribute__((packed))Alarm_Onu_info_t;


typedef struct Alarm_Pon_info{
	USHORT	alarmpolicyid;
			
	UCHAR	rxpowerhighalarm;
	LONG	rxpowerhighalarmthr;
	LONG	rxpowerhighalarmclearthr;
	UCHAR	rxpowerlowalarm;
	LONG	rxpowerlowalarmthr;
	LONG	rxpowerlowalarmclearthr;
	UCHAR	txpowerhighalarm;
	LONG	txpowerhighalarmthr;
	LONG	txpowerhighalarmclearthr;
	UCHAR	txpowerlowalarm;
	LONG	txpowerlowalarmthr;
	LONG	txpowerlowalarmclearthr;
	UCHAR	txbiashighalarm;
	LONG	txbiashighalarmthr;
	LONG	txbiashighalarmclearthr;
	UCHAR	txbiaslowalarm;
	LONG	txbiaslowalarmthr;
	LONG	txbiaslowalarmclearthr;
	UCHAR	vcchighalarm;
	LONG	vcchighalarmthr;
	LONG	vcchighalarmclearthr;
	UCHAR	vcclowalarm;
	LONG	vcclowalarmthr;
	LONG	vcclowalarmclearthr;
	UCHAR	temphighalarm;
	LONG	temphighalarmthr;
	LONG	temphighalarmclearthr;
	UCHAR	templowalarm;
	LONG	templowalarmthr;
	LONG	templowalarmclearthr;
	UCHAR	rxpowerhighwarning;
	LONG	rxpowerhighwarningthr;
	LONG	rxpowerhighwarningclearthr;
	UCHAR	rxpowerlowwarning;
	LONG	rxpowerlowwarningthr;
	LONG	rxpowerlowwarningclearthr;
	UCHAR	txpowerhighwarning;
	LONG	txpowerhighwarningthr;
	LONG	txpowerhighwarningclearthr;
	UCHAR	txpowerlowwarning;
	LONG	txpowerlowwarningthr;
	LONG	txpowerlowwarningclearthr;
	UCHAR	txbiashighwarning;
	LONG	txbiashighwarningthr;
	LONG	txbiashighwarningclearthr;
	UCHAR	txbiaslowwarning;
	LONG	txbiaslowwarningthr;
	LONG	txbiaslowwarningclearthr;
	UCHAR	vcchighwarning;
	LONG	vcchighwarningthr;
	LONG	vcchighwarningclearthr;
	UCHAR	vcclowwarning	;
	LONG	vcclowwarningthr;
	LONG	vcclowwarningclearthr;
	UCHAR	temphighwarning;
	LONG	temphighwarningthr;
	LONG	temphighwarningclearthr;
	UCHAR	templowwarning;
	LONG	templowwarningthr;
	LONG	templowwarningclearthr;		
	UCHAR	alarmpolicystatus;
	struct Alarm_Pon_info *nextNode;
}__attribute__((packed))Alarm_Pon_info_t;

typedef struct Alarm_Port_info{
	USHORT EthAlarmPolicyId;	
	UCHAR  EthAutoNegFailure;
	UCHAR  EthLos;	
	UCHAR  EthFailure;
	UCHAR  EthLoopback;	
	UCHAR  EthCongestion;
	UCHAR  pcPort_List[32];
	struct Alarm_Port_info *nextNode;
}__attribute__((packed))Alarm_Port_info_t;



#define CTC_VLAN_AGGR_ID_MAX  10000

typedef struct{
	USHORT  target_vlan;
	USHORT vlan_aggregation_list[CTC_MAX_VLAN_AGGREGATION_ENTRIES];
}__attribute__((packed))eth_ctc_vlan_aggregation_t;

typedef struct __eth_ctc_vlan_aggr_listnode{
	ULONG  ctcVlanAggrGroupId;
	eth_ctc_vlan_aggregation_t ctcVlanAggrEntries[CTC_MAX_VLAN_AGGREGATION_TABLES];
	struct __eth_ctc_vlan_aggr_listnode *pNextNode;
}__attribute__((packed))eth_ctc_vlan_aggr_list_t;

extern eth_ctc_vlan_aggr_list_t *CTC_findCtcVlanAggrNode( const ULONG aggrId );
extern int CTC_addVlanAggregation( ULONG aggrId, ULONG targetVid, ULONG aggrVid);
extern STATUS	CTC_delVlanAggregation( ULONG aggrId, ULONG targetVid, ULONG aggrVid);

#define CTC_VLAN_TRUNK_ID_MAX  10000

typedef struct __eth_ctc_vlan_trunk_listnode{
	ULONG  ctcVlanTrunkGroupId;
	USHORT ctcVlanTrunkEntries[CTC_MAX_VLAN_FILTER_ENTRIES];
	struct __eth_ctc_vlan_trunk_listnode *pNextNode;
}__attribute__((packed))eth_ctc_vlan_trunk_list_t;

extern eth_ctc_vlan_aggr_list_t *CTC_findNextCtcVlanAggrNode( const ULONG grpId );
extern eth_ctc_vlan_aggr_list_t *CTC_addCtcVlanAggrNode( const ULONG grpId );
extern int mn_getFirstCtcVlanAggrEntryIndex( ULONG *pGrpId, ULONG *pTargetVid, ULONG *pUserVid );
extern int mn_getNextCtcVlanAggrEntryIndex( ULONG grpId, ULONG targetVid, ULONG userVid, ULONG *pNextGrpId, ULONG *pNextTargetVid, ULONG *pNextUserVid );
extern int mn_checkCtcVlanAggrEntryIndex( ULONG grpId, ULONG targetVid, ULONG userVid );
extern int mn_getCtcVlanAggrEntryStatus( ULONG grpId, ULONG targetVid, ULONG userVid, ULONG *pStatus );
extern int mn_setCtcVlanAggrEntryStatus( ULONG grpId, ULONG targetVid, ULONG userVid, ULONG status );
extern int mn_getEthPortVlanAggrGrpId( ULONG devIdx, ULONG portIdx,  ULONG *pAggrId);
extern int mn_setEthPortVlanAggrGrpId( ULONG devIdx, ULONG portIdx,  ULONG grpId);

extern eth_ctc_vlan_trunk_list_t *CTC_addCtcVlanTrunkNode( const ULONG grpId );
extern eth_ctc_vlan_trunk_list_t *CTC_findNextCtcVlanTrunkNode( const ULONG grpId );
extern int CTC_addVlanTrunk( ULONG grpId, ULONG trunkVid);
extern STATUS CTC_delVlanTrunk( ULONG grpId, ULONG trunkVid );
extern int CTC_getEthPortVlanTrunkGroup( ULONG devIdx, ULONG portIdx,  ULONG *pVal);
extern int CTC_setEthPortVlanTrunkGroup( ULONG devIdx, ULONG portIdx,  ULONG val);
extern int mn_getFirstCtcVlanTrunkEntryIndex( ULONG *pGrpId, ULONG *pVlanId );
extern int mn_getNextCtcVlanTrunkEntryIndex( ULONG grpId, ULONG trunkVid, ULONG *pNextGrpId, ULONG *pNextTrunkVid );
extern int mn_checkCtcVlanTrunkEntryIndex( ULONG grpId, ULONG trunkVid );
extern int mn_getCtcVlanTrunkEntryStatus( ULONG grpId, ULONG trunkVid, ULONG *pStatus );
extern int mn_setCtcVlanTrunkEntryStatus( ULONG grpId, ULONG trunkVid, ULONG status );
extern int mn_getEthPortVlanTrunkGrpId( ULONG devIdx, ULONG portIdx,  ULONG *pTrunkId);
extern int mn_setEthPortVlanTrunkGrpId( ULONG devIdx, ULONG portIdx,  ULONG grpId);

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCCT_RMan_VLANh */
