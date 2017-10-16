#ifndef __INCethLoopChkh
#define __INCethLoopChkh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "sys/devsm/devsm_switchhover.h"

#define ETH_LOOP_MAC_ADDR_LEN	6
#define ETH_LOOP_FRM_PAYLOAD_LEN	50

/*#define VLAN_SLOT_MAX				MN_SNMP_MAX_PORTLIST_LEN*/ /*51*/	/* PRODUCT_MAX_TOTAL_SLOTNUM, 注意与VLAN表保持一致 */
/*#define VLAN_SLOT_PORTLIST_LEN		4	*/

#define CHK_SLOT_MAX				SYS_SLOTNUM/* 8 or PRODUCT_MAX_TOTAL_SLOTNUM */
#define CHK_SLOT_PORT_MAX			CARD_MAX_PON_PORTNUM
#define CHK_PON_ONU_MAX			    MAXONUPERPON
#define CHK_BRAS_MAC_MAX                 16
/*B-add by zhengyt@10-3-31,修改环路检测发包格式*/
typedef struct{
	UCHAR	desMac[ETH_LOOP_MAC_ADDR_LEN];	/*目的mac,广播包*/
	UCHAR	srcMac[ETH_LOOP_MAC_ADDR_LEN];	/*源mac地址，可以设置，默认是olt的sysmac*/
	/*ULONG	vlanInf;	*/	/*前2B表示tpid,后2B表示pri+cfi+vid*/
	USHORT	ethType;		/*0x0800*/
	/*ULONG	ethTag;*/
	
	USHORT	chkFlag;		/*0x0080*/
	UCHAR	oltType;		/*1-GFA6100,2-GFA6700, 3-6900*/
	UCHAR	onuType;		
	ULONG	onuDesc;		/*1B=0,2B=slot,3B=port,4B=llid,其中slot和port指的是onu在olt上的slot,port值*/
	UCHAR	onuMac[ETH_LOOP_MAC_ADDR_LEN];
	USHORT	onuVlan;
	ULONG	onuIfIndex;
	/*USHORT	IpHead;*/

	/*USHORT	pduLen;*/
	UCHAR	pduBuf[ETH_LOOP_FRM_PAYLOAD_LEN];
} __attribute__ ((packed))  ethLoopCheckUntaggedFrame_t;

typedef struct{
	UCHAR	desMac[ETH_LOOP_MAC_ADDR_LEN];	/*目的mac,广播包*/
	UCHAR	srcMac[ETH_LOOP_MAC_ADDR_LEN];	/*源mac地址，可以设置，默认是olt的sysmac*/
	ULONG	vlanInf;		/*前2B表示tpid,后2B表示pri+cfi+vid*/
	USHORT	ethType;		/*0x0800*/
	USHORT	chkFlag;		/*0x0080*/
	UCHAR	oltType;		/*1-GFA6100,2-GFA6700*/
	UCHAR	onuType;		
	ULONG	onuDesc;		/*1B=0,2B=slot,3B=port,4B=llid,其中slot和port指的是onu在olt上的slot,port值*/
	UCHAR	onuMac[ETH_LOOP_MAC_ADDR_LEN];
	USHORT	onuVlan;
	ULONG	onuIfIndex;
	
	/*USHORT	IpHead;*/
	/*USHORT	pduLen;*/
	UCHAR	pduBuf[ETH_LOOP_FRM_PAYLOAD_LEN];
} __attribute__ ((packed))  ethLoopCheckTaggedFrame_t;

/*E-add by zhengyt@10-3-31,修改环路检测发包格式*/

typedef struct{
	UCHAR	type;
	UCHAR	result;
	UCHAR	enable;
	USHORT	vlanId;
	UCHAR	macAddr[ETH_LOOP_MAC_ADDR_LEN];
	USHORT	interval;
	USHORT	ctrlPolicy;
	USHORT  uptime;     /*onu port link up time*/
	USHORT  retrycount; /*the times of port link up*/
} __attribute__ ((packed)) ethLoopCheckOamMsg_t;

typedef struct{
	UCHAR	DA[6];
	UCHAR	SA[6];
	USHORT	Type;
	UCHAR	SubType;
	USHORT	Flag;
	UCHAR	Code;
	
	UCHAR	OUI[3];
	UCHAR	GwOpcode;
	ULONG	SendSerNo;
	USHORT	WholePktLen;
	USHORT	PayLoadOffset;
	USHORT	PayLoadLength;
	UCHAR	SessionID[8];

	ethLoopCheckOamMsg_t  Msg;
		
	UCHAR    pad[13];
} __attribute__ ((packed)) ethLoopCheckOamFrame_t;

extern ULONG chk_config_detect_mac_clean_enable;

extern ULONG loopChk_debug_switch;
#define LOOP_CHK_DEBUG(d, x) if(loopChk_debug_switch & d) VOS_SysLog x;

#define ETH_LOOP_CHECK_START		1
#define ETH_LOOP_CHECK_STOP		2

#define ETH_LOOP_CHECK_ENABLE		1
#define ETH_LOOP_CHECK_DISABLE	2

#define ETH_LOOP_CHECK_MODE_OLT_ONLY	0x01
#define ETH_LOOP_CHECK_MODE_OLT_ONU		0x02	


extern int ethLoopCheckInit();
extern BOOL check_slotno_is_illegal( ULONG slotno );

extern int mn_ethLoopCheckEnable();
extern int mn_ethLoopCheckDisable();
extern int mn_ethLoopCheckEnableGet();

/* added by xieshl 20080505, 增加环路检测控制 */
extern int mn_ethLoopCheckContralEnable();
extern int mn_ethLoopCheckContralDisable();
extern int mn_ethLoopCheckMacCleanEnable();
extern int mn_ethLoopCheckMacCleanDisable();
extern int mn_ethLoopCheckContralEnableGet();
extern int mn_ethLoopCheckMacCleanEnableGet();


extern ULONG mn_ethLoopCheckTimerGet();
extern int mn_ethLoopCheckTimerSet( ULONG interval );
extern int mn_ethLoopCheckTimerDefaultSet();

extern USHORT mn_ethLoopCheckVlanGet();
extern int mn_ethLoopCheckVlanSet( USHORT vid );
extern int mn_ethLoopCheckVlanDefaultSet();

extern int mn_ethLoopCheckMacGet( UCHAR *pMac);
extern int mn_ethLoopCheckMacSet( UCHAR *pMac );
extern int mn_ethLoopCheckMacDefaultSet();

extern int mn_ethLoopCheckPortAdd( ULONG slotno, ULONG portno );
extern int mn_ethLoopCheckPortDel( ULONG slotno, ULONG portno );
extern int mn_ethLoopCheckPortDefaultSet();

extern int mn_ethLoopCheckPortGetNext( ULONG slotno, ULONG portno, ULONG *pnext_slotno, ULONG *pnext_portno );
extern STATUS recvLoopChkPacketHook(UCHAR*pBuf, ULONG swport);



#define FC_ETHCHK_TIMER			0x01
#define FC_ETHCHK_SEARCH_MAC		0x02
#define FC_ETHCHK_PORT_LINKDOWN	0x03
#define FC_ETHCHK_ONU_LINEOFF		0x04
#define FC_ETHCHK_PORT_LOOPCLEAR	0x05

#define FC_ETHCHK_COMMAND			0x06
#define FC_ETHCHK_BRAS_MAC             	0x07     /*add by shixh20090716*/
#define FC_ETHCHK_SEND			0x08
#define FC_ETHCHK_RECEIVE          0x09
#define FC_ETHCHK_ONU_PORT_LOOP 0x10
#define FC_OAM_RECEIVE                0x11
#define ETHCHK_ENABLE_CMD		0x01
#define ETHCHK_CONTRAL_CMD	0x02
#define ETHCHK_PORT_CMD		0x03
#define ETHCHK_VLAN_CMD		0x04
#define ETHCHK_INTERVAL_CMD	0x05
#define ETHCHK_UPTIMES_CMD	0x06
#define ETHCHK_PORT_CMD_ADD	0x07
#define ETHCHK_MAC_CLEAN_ENABLE_CMD	0x08
#define ETHCHK_IMMEDIATELY_CMD	0x09

typedef struct {
	struct vty *vty;
	int cmdId;
	int argc;
	char argv[3][64];
}qdef_ethchk_cmd_t;
	
typedef struct {
	int vlanid;
	struct EthLoopVlan_t *next;
}EthLoopVlan_t;

extern VOID check_onu_notpresent_callback( ULONG devIdx );
extern int  check_eth_port_linkdown_callback( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern int check_eth_port_linkup_callback( ULONG devIdx, ULONG brdIdx, ULONG portIdx );

typedef struct ethloop_port_listnode{
	struct ethloop_port_listnode *next;
	ULONG devIdx;
	UCHAR brdIdx;   /**/
	UCHAR portIdx;
	USHORT vid;
	ULONG loopdevIdx;
	UCHAR loopbrdIdx;
	UCHAR loopportIdx;
       USHORT loopvid;
	UCHAR ageTime;		/* added by xieshl 20080627, 老化计数，用于解决告警恢复问题 */
	UCHAR flag;
	UCHAR OtherOltFlag;
	UCHAR OltTypeFlag;
	UCHAR loopmac[ETH_LOOP_MAC_ADDR_LEN]; /*只有当OtherOltFlag 为1时，才有效*/
} ethloop_port_listnode_t;

typedef struct ethloop_listnode_Switch{
	struct ethloop_listnode_Switch *next;
	ULONG devIdx;
	USHORT slotIdx;   /**/
	USHORT portIdx;
	USHORT SwitchPort;
	UCHAR  SwitchMac[ETH_LOOP_MAC_ADDR_LEN]; /*只有当OtherOltFlag 为1时，才有效*/
} ethloop_listnode_Switch_t;

extern ethloop_port_listnode_t * findEthPortFromLoopListByPort( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern ethloop_port_listnode_t * findEthPortFromLoopListByDev( ULONG devIdx );

extern int delEthPortFromLoopListByVid( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG vid );
extern int delEthPortFromLoopListByPort( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern int delEthPortFromLoopListByOnu( ULONG devIdx );
extern int delPonPortFromLoopListByOnu( short int olt_id );
extern int addEthPortToLoopList2( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG vid,ULONG loopdevIdx,  
				ULONG loopbrdIdx, ULONG loopportIdx, ULONG loopvid, UCHAR OtherOltFlag, UCHAR OltType, UCHAR  *loopmac);

typedef struct RPC_Loop_MsgHead
{
    USHORT      usSrcSlot;
    USHORT      usDstSlot;
    ULONG       ulSrcModuleID;
    ULONG       ulSrcSubID;                                     /* 保留,现在不使用*/
    ULONG       ulDstModuleID;
    ULONG       ulDstSubID;                                     /* 保留,现在不使用*/
    USHORT      usMsgMode;
    USHORT      usMsgType;
#ifdef _DISTRIBUTE_PLATFORM_
   ULONG		ulCmdID;
#endif
    ULONG        ResResult;
   qdef_ethchk_cmd_t chkcmd;
   
}RPC_Loop_MsgHead_S;



/* usMsgMode */
#define LOOPTOLIC_REQACK             0x0001              /*本消息需要应答*/
#define LOOPTOLIC_ACK                    0x0002              /*应答消息, 操作成功*/
#define LOOPTOLIC_NAK                    0x0003              /*应答消息, 操作失败*/
#ifdef _DISTRIBUTE_PLATFORM_
#define LOOPTOLIC_ACK_END		0x0004
#endif

/* usMsgType */
#define LOOPTOLIC_EXECUTE_CMD        0x0001          /* 主控板通知lic执行相应的命令 */
#ifdef _DISTRIBUTE_PLATFORM_
#define LOOPTOLIC_EXECUTE_CMD_CONT		0x0002
#endif
/*
typedef struct{
	ULONG    srcslot;
	ULONG	ponId;		
	ULONG    llId;	
    	ULONG  pSendBufLen;
	UCHAR *pSendBuf;
} LoopCDPMsgHead_t;
*/


typedef struct{
	USHORT    enable;
	USHORT	mode;	
	ULONG    controlen_able;	
	ULONG    cleanmac_able;
    	ULONG    interval_time;
	USHORT  uptimes;
	USHORT  uptime_thre;
	USHORT  vlan;
	uchar      mac[ETH_LOOP_MAC_ADDR_LEN];
	UCHAR    eth_portlist[CHK_SLOT_PORT_MAX+2];
	USHORT  EthPortListFlag; 
} LoopInsertCDPMsgHead_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCethLoopChkh */
