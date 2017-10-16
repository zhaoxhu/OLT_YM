/**************************************************************
*
*   TdmService.h -- TDM voice management high level Application functions General header
*
*  
*    Copyright (c)  2007.10 , GW Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	   |    Change				|    Author	  
*   -------|--- --------|------------------|------------
*	1.00	  | 16/10/2007 | Creation				| chen fj
*
***************************************************************/
#ifndef  _TDMSERVICE_H
#define  _TDMSERVICE_H

#define    TDM_FPGA_EXIST    1 /*PONCHIP_EXIST*/
#define    TDM_FPGA_NOT_EXIST   0 /*PONCHIP_NOT_EXIST*/

#define  V2R1_TDM_OK  0
#define  V2R1_TDM_ERROR (-1)
#define  V2R1_TDM_ONU_ONLINE  1
#define  V2R1_TDM_ONU_OFFLINE  2

#define  V2R1_TDM_ONU_COMM_SUCCESS  1
#define  V2R1_TDM_ONU_COMM_FAIL  2

#define  V2R1_ONU_POTS_LOOP_ENABLE  1
#define  V2R1_ONU_POTS_LOOP_DISABLE  0
/*
#define TDM_MSG_OK  0

#define  TDM_SG_INDEX1   1
#define  TDM_SG_INDEX2   2
#define  TDM_SG_INDEX3   3

#define  TDM_SG_MAX    3*/

/********  FOR onu 语音OAM *************
#define ONU_POTS_NUM_MIN   1
#define ONU_POTS_NUM_MAX  32
#define ONU_POTS_ALL  0xffffffff 
*/

/* ONU 语音MAC */
#define  SET_ONU_VOICE_MAC_REQ   100
#define  SET_ONU_VOICE_MAC_RSP   100
#define  GET_ONU_VOICE_MAC_REQ   101
#define  GET_ONU_VOICE_MAC_RSP   101
/* ONU 语音VLAN */
#define  SET_ONU_VOICE_VLAN_REQ   102
#define  SET_ONU_VOICE_VLAN_RSP   102
#define  GET_ONU_VOICE_VLAN_REQ   103
#define  GET_ONU_VOICE_VLAN_RSP   103
/* ONU 语音POTS */
#define  SET_ONU_VOICE_POTS_REQ   104
#define  SET_ONU_VOICE_POTS_RSP   104
#define  GET_ONU_VOICE_POTS_REQ   105
#define  GET_ONU_VOICE_POTS_RSP   105
#define  GET_ONU_VOICE_POTS_STATUS_REQ   106
#define  GET_ONU_VOICE_POTS_STATUS_RSP   106
/* ONU 语音环回 */
#define  SET_ONU_VOICE_LOOP_REQ   107
#define  SET_ONU_VOICE_LOOP_RSP   107
#define  GET_ONU_VOICE_LOOP_REQ   108
#define  GET_ONU_VOICE_LOOP_RSP   108
/* ONU 语音扩展属性 */
#define  SET_ONU_VOICE_EXT_EUQ_REQ   109
#define  SET_ONU_VOICE_EXT_EUQ_RSP   109
/* onu 语音业务*/
#define  SET_ONU_VOICE_SERVICE_REQ   110
#define  SET_ONU_VOICE_SERVICE_RSP   110
#define  GET_ONU_VOICE_SERVICE_REQ   111
#define  GET_ONU_VOICE_SERVICE_RSP   111

/*#define  MAXONUPERSG  256*/

#ifndef  TDM_ETH_FILTER_PERMIT
#define  TDM_ETH_FILTER_PERMIT  0
#define  TDM_ETH_FILTER_DENY  1
#endif
/*
extern unsigned long getTdmChipInserted( unsigned char slot_id, unsigned char tdm_id );
extern ULONG get_gfa_tdm_slotno();

#define  V2R1_VLAN_NAME_LEN  32
#define  V2R1_VLAN_INFO_LEN  256
*/
#define  V2R1_DEFAULT_VLAN  1


/********* for tdm 板卡管理******************/
/* tdm 板内部状态*/
/*
设备管理模块初始状态 DEV_ST_INITIALIZING (0)
下载FPGA or APP. 程序DEV_ST_UPGRADING (1)
FPGA开始加载DEV_ST_LOADING (2)
FPGA加载结束 DEV_ST_LOADED (3)
运行 DEV_ST_RUNNING (6)
其余状态暂未用

*/
enum {
	TDM_INITIALIZING = 0,
	TDM_UPDATING,
	TDM_LOADING,
	TDM_LOADED,
	TDM_READY,
	TDM_ENABLING,
	TDM_RUNNING,
	TDM_ERROR
};

/* 设备 管理看到的tdm 板状态 */
enum {
	TDMCARD_UNKNOWN = 0,
	TDMCARD_NOTLOADING = 1 ,
	TDMCARD_LOADING,
	TDMCARD_LOADCOMP,
	TDMCARD_ACTIVATED,
	TDMCARD_ERROR,
	TDMCARD_DEL ,              /* pull */
	TDMCARD_UPDATEING,
	TDMCARD_UPDATE_COMP,
};

/* 设备管理操作*/
enum {
	TDM_INSERTED = 0x11,       /*  TDM 板卡在位 */
	TDM_PULLED,                   /*  TDM 板卡拔出(不在位)*/
	TDM_RESET,                     /*  复位TDM 板 */
	TDM_ACTIVATE,              /*  激活TDM 板 */
	TDM_STATUS_QUERY,  /* 查询tdm 板状态*/
	TDM_APP_LOADING,      /*  加载TDM 板APP 程序 */
	TDM_FPGA_LOADING,   /*  加载TDM 板FPGA 程序 */
	TDM_APP_LOAD_COMP,      /*  加载TDM 板APP 程序完成 */
	TDM_FPGA_LOAD_COMP,   /*  加载TDM 板FPGA 程序 完成*/
	TDM_DATA_LOADING    /*  加载TDM 板配置数据*/
};

#define  TDM_COMM_RECV_MSG  0x30   /* 从tdm 板接收到消息(指未请求的消息)*/
#define  TDM_TIMER_OUT  0x31   /* tdm 管理1 秒定时消息*/

extern unsigned int  DEBUG_ONU_TDM_VOICE;
extern unsigned int  TDM_MGMT_DEBUG ;

extern unsigned int  TdmCardStatus;
extern unsigned char  TdmCardStatusFromTdm;
extern unsigned int  TdmActivatedFlag;
extern unsigned int   Flag_WaitTdmStatusQuery;

extern unsigned int  TdmAppUpdateFlag ;
extern unsigned int  TdmFpgaUpdateFlag;
extern unsigned int  TdmFpgaLoadingCounter;
extern unsigned long TDMCardIdx;

extern unsigned char TdmMsgBuf[];
extern unsigned short int TdmMsgLen;
extern unsigned int TdmFpgaLoadingComp;

extern unsigned int   Flag_WaitTdmStatusQuery ;
extern unsigned char *TdmMsgType[];

extern unsigned char VoiceMacOnuFpga[];
extern unsigned char VoiceMacOltFpga[];


#define  TDM_APP_VERSION_LEN  9
#define  TDM_FPGA_VERSION_LEN  5
#define  TDM_FPGA_LOADING_COUNT  2
#define  TDM_COMM_TIMEOUT  7

extern unsigned long OnuDevIdx_Array[];
/*extern unsigned char *V2R1_VoiceVlanName[];*/

extern unsigned char  TdmChipDownloadComp[];
typedef struct {
	unsigned int length;
	unsigned char *BufPtr;
}TdmRecvMsgBuf;

/* TDM 管理消息结构*/
typedef struct {
	unsigned char DA[6];
	unsigned char SA[6];
	unsigned short  int type;
}__attribute__((packed))TdmCommMsgHead;

typedef struct {
	TdmCommMsgHead TdmMsgHead;
	int Reserved;
	unsigned char MsgType;
	unsigned char MsgSubType;
	unsigned short int  MsgCode;
}__attribute__((packed))TdmCommGeneralMsg;

typedef struct{
	TdmCommMsgHead TdmMsgHead;
	int Reserved;
	unsigned char MsgType;
	unsigned char MsgSubType;
	unsigned short int  MsgCode;
	unsigned char AppVersion[TDM_APP_VERSION_LEN];
	unsigned char FpgaVersion[TDM_FPGA_VERSION_LEN];
}__attribute__((packed)) TdmSoftVersion;

typedef struct {
	TdmCommMsgHead TdmMsgHead;
	int Reserved;
	unsigned char MsgType;
	unsigned char MsgSubType;
	unsigned short int  MsgCode;
	unsigned char Result[3];
}__attribute__((packed)) TdmFPGADownloadComp;

typedef struct {
	TdmCommMsgHead TdmMsgHead;
	int Reserved;
	unsigned char MsgType;
	unsigned char MsgSubType;
	unsigned short int  MsgCode;
	unsigned char status;
}__attribute__((packed)) TdmStatusRsp;

typedef struct {
	unsigned char SendFlag;
	unsigned char TimeCount;
}TdmCommMsgRecord;

extern TdmCommMsgRecord  TdmMsgRecord[];

/***************************************/

/* ONU VOICE ENABLE */
typedef struct {
	unsigned char VoiceEnable;
	unsigned char TdmSlot;
	unsigned char SgIdx;
	unsigned short int LogicOnuIdx;
}OnuVoiceEnable;

/* ONU VOICE LAN */
typedef struct {
	unsigned char Enable;
	unsigned char Priority;
	unsigned short int VlanId;
}VoiceVlanEnable;


/* ONU OAM */

typedef struct {
	unsigned char MsgType;
	unsigned char Result;
	unsigned char VoiceEnable;
	unsigned char VoiceDstMac[6];
	unsigned char VoiceSrcMac[6];
}__attribute__((packed)) OAM_OnuVoiceEnable;

typedef struct {
	unsigned char MsgType;
	unsigned char Result;
	unsigned char Enable;
	unsigned short int VlanTag;
}__attribute__((packed)) OAM_VoiceVlanEnable;

typedef struct {
	unsigned char MsgType;
	unsigned char Result;
	unsigned char PotsEnable;
	unsigned int    PotsList;
}__attribute__((packed)) OAM_OnuPotsEnableConfig;

typedef struct {
	unsigned char MsgType;
	unsigned char Result;
	unsigned int    PotsList;
	unsigned int  PotsListEnable;
}__attribute__((packed)) OAM_OnuPotsEnableQuery;

typedef struct {
	unsigned char MsgType;
	unsigned char Result;
	unsigned int    PotsListStatus;
}__attribute__((packed)) OAM_OnuPotsStatusQuery;

typedef struct{
	unsigned char MsgType;
	unsigned char Result;
	unsigned char LoopbackCtrl;
}__attribute__((packed)) OAM_OnuPotsLoopback;

typedef struct {
	unsigned char MsgType;
	unsigned char Result;
	unsigned char VoiceEnable;
	unsigned char VoiceDstMac[6];
	unsigned char VoiceSrcMac[6];
	unsigned char VlanEnable;
	unsigned short int VlanTag;
	unsigned int  PotsEnableList;

}__attribute__((packed)) OAM_OnuVoiceService;

#ifndef cli_l2_entry_t

typedef struct cli_l2_entry_s {
    unsigned char mac[6];
    int vid;
    int pri;
    int cpu;
    int dst_discard;
    int src_discard;
    int scp;     /*source cos priority*/
    int tgid;
    int trunk;
    int tgid_port;
    int l2mc_ptr;
    int remote_t; /*remote trunk*/
    int mod_id;
    int l3;
    int mac_block_index;
    int l2_static;
    int rpe; /*remap priority enable*/
    int mirror;
    int valid;
    int even_parity;
    int dst_hit;
    int src_hit;
} cli_l2_entry_t;

typedef unsigned char   cli_mac_t[6];

typedef struct cli_shr_pbmp {
	unsigned int	pbits[1];
} cli_shr_pbmp_t;
typedef struct cli_l2_addr_s {
    unsigned int              flags;          /* BCM_L2_XXX flags */
    cli_mac_t           mac;            /* 802.3 MAC address */
    int                 vid;            /* VLAN identifier */
	int                 slot;
    int                 port;           /* port number 从1开始*/
    int                 modid;          /* XGS: modid */
    int                 tgid;           /* Trunk group ID */
    int                 rtag;           /* Trunk port select formula */
    int                 cos_dst;        /* COS based on dst addr */
    int                 cos_src;        /* COS based on src addr */
    int                 l2mc_index;     /* XGS: index in L2MC table */
    cli_shr_pbmp_t      block_bitmap;   /* XGS: blocked egress bitmap */
    int			auth;		/* Used if auth enabled on port */
} cli_l2_addr_t;
#endif  /*EPONV2R1_BCM_56300*/

/*****  用于测试BCM56300 MAC 设置*/

extern int bms_l2_add(int unit,cli_l2_entry_t *l2entry);
extern int bms_l2_entry_get(int unit,int l2index,cli_l2_addr_t *l2addr,int static_bit);
extern int bms_l2_delete(int unit,int vid,void * Mac,int discard_flag);
extern int bms_isolate_set(int in_port,int out_port,int flag);
extern int bms_l2_idx_get(int unit,int *idx_min,int *idx_max);

/* 用于BCM56300 VLAN 设置*/
extern LONG MN_Snmp_Vlan_Create( CHAR* pcVlanName,LONG plVlanId,USHORT* pusVid, UCHAR uType,USHORT usLang, CHAR* pcError );
extern LONG MN_Vlan_Check_exist(ULONG  vlanIndex);
extern LONG MN_Vlan_Delete( USHORT usVid, USHORT usLang, CHAR* pcError);
extern LONG MN_Vlan_Get_Vid( USHORT* pusVid, CHAR* pcName, USHORT usLang, CHAR* pcError);
extern LONG MN_Vlan_Set_TaggedPorts(ULONG  vlanIndex , CHAR * portList , ULONG portlist_len);
extern LONG MN_Vlan_Get_AllPorts(ULONG  vlanIndex , CHAR * portList , ULONG * portlist_len);
extern LONG MN_Vlan_Get_UntagPorts(ULONG  vlanIndex , CHAR * portList , ULONG * portlist_len);

#ifdef TDM_MGMT
#endif
/*  tdm 板卡管理内部函数*/
extern void  OnuSecondMsgToTdmMgmt();
extern int OnuSecondMsgHandler();
extern void  TdmMgmtEntry();
extern int TdmMsgHandler(unsigned long *MsgBuf);
extern int  GetTdmAppVerFromFlash( unsigned char *TdmAppVer );
extern int  GetTdmFPGAVerFromFlash(unsigned char *TdmFPGAVer );
extern unsigned short int  GetTdmCardInsertedAll(void);
extern int  GetTdmCardInserted( int CardIndex);
extern int  SetTdmCardInserted( int CardIndex);
extern int  SetTdmCardPulled( int CardIndex);

/* tdm 管理与设备管理消息交互*/
extern void  TdmCardInserted( unsigned long CardIndex);
extern void  TdmCardPulled ( unsigned long CardIndex);
extern void  TdmCardReset ( unsigned long CardIndex);
extern void  TdmCardActivated(unsigned long CardIndex );
extern int  TdmCardStatusQuery(unsigned long TdmSlot);
extern int  TdmUpdateAppfile(unsigned long CardIndex);
extern int  TdmUpdateAppfileComp(unsigned long CardIndex, unsigned int result );
extern int  TdmUpdateFpgafile(unsigned long CardIndex);
extern int  TdmUpdateFpgafileComp(unsigned long CardIndex, unsigned int result );

/* tdm 管理与tdm 板通信API
extern int  TdmVersionQuery(unsigned long CardIndex );
extern int  TdmFPGADownload(unsigned long CardIndex, unsigned char fpga1, unsigned char fpga2, unsigned char fpga3);
extern int  TdmRunningEnable (unsigned long TdmSlot);
extern int  TdmReset(unsigned long TdmSlot);
extern int  TdmRunningStatus( unsigned long CardIndex );
 */
extern void TdmCardInsert( unsigned long CardIndex);
extern void TdmCardPull(unsigned long CardIndex);
extern void ResetTdmCard ( unsigned long CardIndex);
extern int  GetTdmCardStatus( unsigned long CardIndex );

/* tdm 管理通信响应消息处理函数
extern int  TdmVersionHandler(unsigned long CardIndex, unsigned char *VersionInfo, unsigned int length);
extern int  TdmFPGALoadingAckHandler(unsigned long CardIndex, unsigned char *LoadFpgaAckInfo, unsigned int length);
extern int  TdmFPGALoadingCompHandler(unsigned long CardIndex, unsigned char *LoadFpgaCompInfo, unsigned int length);
extern int  TdmStatusRspHandler(unsigned long CardIndex, unsigned char *StatusRsp, unsigned int length);
extern void  TdmMgmtLowLevelMsgHandler(unsigned long CardIndex,unsigned char *pBuf, unsigned int length);
*/
extern void  TdmMgmtLowLevelMsgRecv(unsigned char *pBuf, unsigned int length);

#ifdef TDM_SG
#endif
/* 信令网关设置/查询 */
extern int  SetTdmSGHdlclink(unsigned char TdmSlot,unsigned char SgIdx, unsigned char MasterHdlc, unsigned char SlaveHdlc);
extern int  GetTdmSGHdlclink(unsigned char TdmSlot,unsigned char SgIdx, unsigned char *MasterHdlc, unsigned char *SlaveHdlc);
extern int  SetTdmSGClock(unsigned char TdmSlot,unsigned char SgIdx, unsigned char ClockE1);
extern int  GetTdmSGClock(unsigned char TdmSlot,unsigned char SgIdx,unsigned char *ClockE1);

#ifdef TDM_SG_ONU
#endif
/* 语音ONU添加/删除/查询 */
/*extern int OnuIsSupportVoice(unsigned long OnuDeviceIdx, bool *SupportVoice);*/
extern int  AddOnuToSG(unsigned char TdmSlot, unsigned char SgIdx, unsigned long OnuDeviceIdx, unsigned short int *logicOnu);
extern int  DeleteOnuFromSG(/*unsigned char TdmSlot ,unsigned char SgIdx,*/ unsigned long OnuDeviceIdx);
extern int  DeleteAllOnuFromSG(unsigned char TdmSlot ,unsigned char SgIdx);
extern int  GetOnuBelongToSG(unsigned long OnuDeviceIdx, unsigned char *TdmSlot ,unsigned char *SgIdx,unsigned short int *LogicOnuId);
extern int  GetAllOnuBelongToSG(unsigned char TdmSlot,unsigned char SgIdx, unsigned long *OnuNumber, unsigned long *OnuDeviceIdx);

#ifdef TDM_SG_VLAN
#endif
/* 信令网关语音VLAN设置/查询*/
extern int  SetTdmSGVoiceVlanEnable(unsigned char TdmSlot,unsigned char SgIdx, unsigned char Priority, unsigned short int VlanId);
extern int  SetTdmSGVoiceVlanDisable(unsigned char TdmSlot,unsigned char SgIdx);
extern int  GetTdmSGVoiceVlanEnable(unsigned char TdmSlot,unsigned char SgIdx, unsigned char *enableFlag, unsigned char *Priority, unsigned short int *VlanId);
extern int  SetSWVoiceVlanEnable(unsigned char TdmSlot ,unsigned char SgIdx, unsigned short int Vid,unsigned char priority);
extern int  SetSWVoiceVlanDisable(unsigned char TdmSlot ,unsigned char SgIdx, unsigned short int Vid);
extern int  GetSWVoiceVlanEnable(unsigned char TdmSlot ,unsigned char SgIdx, unsigned char *enableFlag, unsigned char *Priority, unsigned short int *VlanId, unsigned char *PortNum, unsigned char *PortList);
extern int  SetOnuVoiceVlanEnable(unsigned long OnuDeviceIdx,unsigned short int vid, unsigned char priority);
extern int  SetOnuVoiceVlanDisable(unsigned long OnuDeviceIdx);
extern int  GetOnuVoiceVlanEnable(unsigned long OnuDeviceIdx, unsigned char *EnableFlag , unsigned char *Priority, unsigned short int *VlanId );

#ifdef TDM_ONU_POTS_LOOPBACK
#endif
/* ONU语音环回设置/查询  */
extern int  OnuPotsLoopbackEnable(unsigned long OnuDeviceIdx);
extern int  OnuPotsLoopbackDisable(unsigned long OnuDeviceIdx);
extern int  GetOnuPotsLoopback(unsigned long OnuDeviceIdx);
extern int  GetOnuPotsLoopbackStatus(unsigned long OnuDeviceIdx);

#ifdef TDM_ONU_POTS_LINK
#endif
/*  语音连接添加/删除/查询 
extern int  AddOnuPotsLinkToSg(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort, unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort);
extern int  DeleteOnuPotsLinkFromSg(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort);
extern int  GetOnuPotsLinkByDevIdx(unsigned long OnuDeviceIdx, unsigned char PotsBoard, unsigned char PotsPort, bool *EnableFlag, unsigned char *TdmSlot, unsigned char *SgIdx, unsigned short int *LogicPort);
extern int  GetOnuPotsLinkBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort, bool *EnableFlag, unsigned long *OnuDeviceIdx, unsigned char PotsBoard, unsigned char PotsPort);
extern int  SetOnuPotsLinkPhoneNumByDevIdx(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort);
extern int  SetOnuPotsLinkPhoneNumBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort,unsigned char *PhonuNumber);
extern int  GetOnuPotsLinkPhoneNumByDevIdx(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort,unsigned char *PhonuNumber);
extern int  GetOnuPotsLinkPhoneNumBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort,unsigned char *PhonuNumber);
extern int  SetOnuPotsLinkDescByDevIdx(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort,unsigned char *Description );
extern int  SetOnuPotsLinkPhoneNumBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort,unsigned char * Description);
extern int  GetOnuPotsLinkPhoneNumBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort,unsigned char * Description);
extern int  GetOnuPotsLinkDescByDevIdx(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort,unsigned char *Description );
extern int  GetOnuPotsLinkSgInfo(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort,unsigned char *TdmSlot, unsigned char *SgIdx );
extern int  GetOnuPotsLinkSgInfo(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort,unsigned char *E1, unsigned char *Timeslot );
*/
extern int  GetOnuPotsLinkAll(unsigned long OnuDeviceIdx, unsigned long *PotsEnableList);

#ifdef  TDM_ONU_OAM
#endif
/* 语音扩展OAM API */
/***************************************************************************
*    函数功能: 设置ONU语音业务使能和DMAC/SMAC地址
*    函数原型: int SetOnuVoiceEnable(unsigned long OnuDeviceIdx,struct OnuVoiceEnable VoiceEn)
*
*    输入参数：
*           unsigned long OnuDeviceIdx－ ONU设备索引，4字节，SLOT * 10000 + PON * 1000 + ONU_id
*           结构VoiceEn包含四个字段：
*           unsigned char VoiceEnable － ONU语音使能标志，1－工作、0－不工作
*           unsigned char TdmSlot - TDM板所在的槽位，槽位号的编排按GFA6700产品规定
*           unsigned char SgIdx－ 信令网关索引，取值：1－3
*           unsigned char LogicOnuIdx － ONU在信令网关中的逻辑编号，取值：1－256
*
*     输出参数： 无
*     返回值：
*             (0) -- 函数执行正确，正确设置ONU语音使能，及ONU语音MAC
*             (-1) -- 函数执行错误，或参数错
*              1 -- ONU不在线
*
***************************************************************************/
extern int SetOnuVoiceEnable(unsigned long OnuDeviceIdx, OnuVoiceEnable *VoiceEn);
/***************************************************************************
*      函数功能: 查询ONU语音业务使能和DMAC/SMAC地址
*      函数原型：
*           int GetOnuVoiceEnable(unsigned long OnuDeviceIdx,unsigned char *VoiceEnable,unsigned char *SrcMac, unsigned char *DestMac);
*
*      输入参数：
*           unsigned long OnuDeviceIdx－ ONU设备索引，4字节，SLOT * 10000 + PON * 1000 + ONU_id
*
*       输出参数：
*          unsigned char *VoiceEnable － ONU语音使能标志，1－工作、0－不工作
*          unsigned char *SrcMac － ONU语音上行数据源MAC地址，长度6字节
*          unsigned char *DestMac － ONU语音上行数据目的MAC地址，长度6字节
*
*        返回值：
*           (0）- 函数执行正确，输出参数有效
*           (-1) - 函数执行错误，或参数错
*              1 － ONU不在线
*              4 － ONU没有配置到任何一个SG中
*              6 － ONU不支持语音业务
*
***************************************************************************/
extern int GetOnuVoiceEnable(unsigned long OnuDeviceIdx,unsigned char *VoiceEnable,unsigned char *SrcMac, unsigned char *DestMac);
/***************************************************************************
*      函数功能: 设置ONU语音VLAN使能和tag
*      函数原型：
*          int SetOnuVoiceVlan(unsigned long OnuDeviceIdx,struct VoiceVlanEnable VoiceVlan);
*
*        输入参数：
*           unsigned long OnuDeviceIdx－ ONU设备索引，4字节，SLOT * 10000 + PON * 1000 + ONU_id
*           结构VoiceVlan包含三个字段，
*           unsigned char Enable －语音数据是否插入VLAN标签，1－带tag、0－不带tag
*           unsigned char Priority - 语音数据报文的优先级，取值：0－7，默认为7
*           unsigned short int VlanId－ 语音数据报文VLAN id，取值：1－4094
*
*        输出参数： 无
*
*        返回值：
*            (0）- 函数执行正确，正确设置ONU语音VLAN
*            (-1) - 函数执行错误，或参数错
*             1 － ONU不在线 
*
***************************************************************************/
extern int SetOnuVoiceVlan(unsigned long OnuDeviceIdx, VoiceVlanEnable *VoiceVlan);
extern int GetOnuVoiceVlan(unsigned long OnuDeviceIdx,  VoiceVlanEnable *VoiceVlan);
/***************************************************************************
*        函数功能: 设置ONU POTS口使能
*        函数原型：
*          int SetOnuPotsEnable(unsigned long OnuDeviceIdx,unsigned char PotsEnable,unsigned char PotsNum,unsigned long PotsList);
*
*        输入参数：
*            unsigned long OnuDeviceIdx－ ONU设备索引，4字节，SLOT * 10000 + PON * 1000 + ONU_id
*            unsigned char PotsEnable － POTS口使能标志，1－使能，0－关闭
*            unsigned char PotsNum - 要设置的Pots口个数，
*            unsigned int PotsList － POTS端口列表；每一个bit对应一个POTS端口，＝1时表示在端口上执行PotsEnable标识的操作，，＝0表示端口上不执行PotsEnable标识的操作
*
*        输出参数： 无
*
*        返回值：
*               (0）- 函数执行正确，正确设置ONU POTS使能或关闭
*               (-1) - 函数执行错误，或参数错
*               1 － ONU不在线 
*
***************************************************************************/
extern int SetOnuPotsEnable(unsigned long OnuDeviceIdx,unsigned char PotsEnable,unsigned char PotsNum,unsigned long PotsList);
extern int GetOnuPotsEnable(unsigned long OnuDeviceIdx,unsigned char PotsNum,unsigned long PotsList, unsigned long *PotsEnableList);
extern int GetOnuPotsStatus(unsigned long OnuDeviceIdx,unsigned long *PotsStatusList);
/************************************************************************
*    函数功能: 设置ONU 语音环回
*    函数原型: int SetOnuVoiceLoopbackStatus(unsigned long OnuDeviceIdx, unsigned char LoopStatus)
*
*    输入参数：
*           unsigned long OnuDeviceIdx － ONU设备索引，4字节，SLOT * 10000 + PON * 1000 + ONU_id
*           unsigned char LoopStatus － ONU 语音环回状态，1－环回、0－停止环回
*    输出参数：无
*    返回值:
*           (0） - 函数执行正确，输出参数有效
*           (-1)  - 函数执行错误，或参数错，输出参数无效
*            1 － ONU不在线
*    说明: 通过ONU语音扩展OAM，设置ONU语音环回（用于测试）；对应的OAM消息，5.2 设置ONU语音环回
*
**********************************************************************/
extern int SetOnuVoiceLoopbackStatus(unsigned long OnuDeviceIdx, unsigned char LoopStatus);
extern int GetOnuVoiceLoopbackStatus(unsigned long OnuDeviceIdx, unsigned char *LoopStatus);


extern int GetOnuVoiceDeviceInfo(unsigned long onuDevIdx, unsigned char *SlotNum, unsigned char *SlotInfo, unsigned char *Power, unsigned char *Battery, unsigned char *Surronding);

extern int ConfigOnuTdmVoiceService(unsigned long OnuDeviceIdx);
extern int ConfigAllOnuTdmVoiceService(unsigned char TdmSlot ,unsigned char SgIdx);

/************************************************************************
*    函数功能: 设置ONU 语音业务属性
*      函数原型：
*             int SetOnuVoiceService(unsigned long OnuDeviceIdx, struct OnuVoiceEnable VoiceEn , struct VoiceVlanEnable VlanEn, unsigned long PotsEnableList)
*
*       输入参数：
*            unsigned long OnuDeviceIdx － ONU设备索引，4字节，SLOT * 10000 + PON * 1000 + ONU_id
*           结构VoiceEn包换四个字段：
*           unsigned char VoiceEnable － ONU语音使能标志，1－工作、0－不工作
*           unsigned char TdmSlot - TDM板所在的槽位，槽位号的编排按GFA6700产品规定
*           unsigned char SgIdx－ 信令网关索引，取值：1－3
*           unsigned char LogicOnuIdx － ONU在信令网关中的逻辑编号，取值：1－256
*           结构VlanEn包含三个字段：
*           unsigned char Enable －语音数据是否插入VLAN标签，1－带tag、0－不带tag
*           unsigned char Priority - 语音数据报文的优先级，取值：0－7，默认为7
*           unsigned short int VlanId－ 语音数据报文VLAN id，取值：1－4094
*           
*           unsigned long PotsEnableList － POTS端口使能列表；每一个bit对应一个POTS端口
*
*        输出参数：无
*
*        返回值：
*             (0） - 函数执行正确，设置ONU语音业务属性
*             (-1)  - 函数执行错误，或参数错 
*              1 － ONU不在线
*
**********************************************************************/
extern int SetOnuVoiceService(unsigned long OnuDeviceIdx,  OnuVoiceEnable *VoiceEn ,  VoiceVlanEnable *VlanEn, unsigned long PotsEnableList);
extern int GetOnuVoiceService(unsigned long OnuDeviceIdx, unsigned char *VoiceEnable, unsigned char *SrcMac, unsigned char *DestMac,  VoiceVlanEnable *VlanEn, unsigned long *PotsEnableList);


extern int ConfigOnuVoiceService(unsigned long  onuDevIdx );

extern int FindMacFromBCM56300( unsigned char *Mac, int *vid);
extern int FindTdmStyleMacFromBCM56300( unsigned char *TdmMac, int *vid);

#ifdef  PALTFORM_BCM56300
#endif

#if 0    /* deleted by chenfj 2008-7-31 */
/*创建语音VLAN */
extern int  CreateVoiceVlan(unsigned char TdmSlot, unsigned char SgIdx,  unsigned short int VoiceVid);
/* 删除语音VLAN */
extern int  DeleteVoiceVlan( unsigned char TdmSlot, unsigned char SgIdx, unsigned short int VoiceVid );
/* 以名字为索引删除语音VLAN */
extern int  DeleteVoiceVlanByName( unsigned char TdmSlot, unsigned char SgIdx );
extern int  DeleteVoiceVlanByNameAll( unsigned char TdmSlot);

/* 向语音VLAN 中添加端口*/
extern int  AddPortToVoiceVlan(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int VlanId );
#endif

#ifdef  _TDM_BOARD_SG_VOICE_OPERATION_
#endif
/*************************************************************
*        以下函数将软件平台对VLAN、MAC 的
* 相关操作封装成TDM 语音API，可用于 添
* 加/ 删除语音ONU，使能/ 修改 语音VLAN  等
* 
* 参数取值:
*                unsigned char TdmSlot -- tdm 板卡索引; 取值4 - 8
*                unsigned char SgIdx -- tdm 板信令网关索引; 取值: 1- 4
*                unsigned int OnuDevIdx -- ONU 设备索引
*                unsigned short int VoiceVid -- 语音VLAN，取值1 - 4094; 
*                unsigned short int LogicOnuIdx -- ONU 在信令网关中逻辑标号，取值1 － 256
*
***************************************************************/
/* 向	交换芯片BCM56300 中增加静态语音MAC ( 单个语音ONU ) */
extern int AddVoiceMacToSw(unsigned char TdmSlot, unsigned char SgIdx,  unsigned int OnuDevIdx, unsigned short int VoiceVid, unsigned short int LogicOnuIdx);
/* 向	交换芯片BCM56300 中增加静态语音MAC (所有语音ONU )*/
extern int AddVoiceMacToSwAll(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int VoiceVid, unsigned int OnuCount, unsigned long *SgOnuIdx);
/* 删除交换芯片中静态语音MAC (单个ONU ) */
extern int DeleteVoiceMacFromSw(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicOnuIdx);
/* 删除交换芯片中所有静态语音MAC (指定TDM接口下所有ONU ) */
extern int DeleteVoiceMacFromSwAll(unsigned char TdmSlot, unsigned char SgIdx, unsigned int OnuCount, unsigned long *SgOnuIdx);
/* 恢复一个信令网关下语音vlan 和所有语音ONU 静态MAC */
extern int RestoreSWVoiceMacAndVlan(unsigned char TdmSlot, unsigned char SgIdx);
/* 激活TDM sig时，恢复相关的VLAN 和语音MAC */
extern int  RestoreTdmSgBoardVoiceMacVlan( unsigned char TdmSlot);

#ifdef  _TDM_BOARD_E1_LINK_OPERATION_
#endif
/******************************************************************
 *    Function: int AddOnuE1LinkMacToSw(unsigned char TdmSlot, unsigned char SgIdx,  unsigned int OnuDevIdx, unsigned short int VoiceVid, unsigned short int LogicOnuIdx)
 *
 *    Param:  unsigned char TdmSlot -- tdm 板卡索引; 取值TDMCARD_FIRST - TDMCARD_LAST
 *                unsigned char fpgaIdx -- tdm 板信令e1 簇索引; 取值: TDM_FPGA_MIN - TDM_FPGA_MAX
 *                unsigned char E1Idx -- 在一个E1簇中的顺序编号，取值: 1 - MAX_E1_PER_FPGA
 *                unsigned int OnuDevIdx -- ONU 设备索引
 *                unsigned short int E1_slotPort -- E1 连接对应的ONU侧的E1索引
 *                unsigned short int E1LinkVid -- E1-link vlan-id, 1-4094
 *                            
 *    Desc:   向	交换芯片BCM56300 中增加静态E1-link MAC (只针对ONU 侧)
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
extern int AddOnuE1LinkMacToSw(unsigned char TdmSlot, unsigned char fpgaIdx, unsigned char E1Idx, unsigned int OnuDevIdx, /*unsigned short int E1_slotPort,*/ unsigned short int E1LinkVid);

/************************************************************************************* 
*	从交换芯片56300 中删除一个E1 连接对应的MAC 地址(只针对ONU 侧) 
*   参数取值同上
**************************************************************************************/
extern int DelOnuE1LinkMacFromSw(unsigned char TdmSlot, unsigned fpgaIdx, unsigned char E1Idx, unsigned OnuDevIdx/*, unsigned short int E1_slotPort*/ );

/******************************************************************
 *    Function:  int AddFpgaE1LinkMacToSw(unsigned char TdmSlot, unsigned char fpgaIdx, unsigned short int E1LinkVid)
 *
 *    Param:  unsigned char TdmSlot -- tdm 板卡索引; 取值TDMCARD_FIRST - TDMCARD_LAST
 *                unsigned char fpgaIdx -- tdm 板信令e1 簇索引; 取值: TDM_FPGA_MIN - TDM_FPGA_MAX
 *                unsigned short int E1LinkVid -- E1-link vlan-id, 1-4094
 *                            
 *    Desc:   向交换芯片BCM56300 中增加一片fpga 静态E1-link MAC (只针对tdm 侧)
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
extern int AddFpgaE1LinkMacToSw(unsigned char TdmSlot, unsigned char fpgaIdx, unsigned short int E1LinkVid);

/* 从交换芯片56300 中删除一片fpga E1 连接对应的MAC 地址(只针对tdm 侧) */
extern int DelFpgaE1LinkMacFromSw(unsigned char TdmSlot, unsigned fpgaIdx );


#ifdef __TDM_BOARD_OPERATION__
#endif
/********************************************************
*                                                                                                     *
*        注: 以下函数可使用于所有类型TDM 板    *
*                                                                                                     *
*********************************************************/

/* 设置一个TDM 端口与其它ETH端口之间是否隔离*/
extern int SetEthPortTdmIsolate( unsigned char TdmSlot, unsigned char tdmPort, unsigned char en_flag );

/* 设置TDM板对应的ETH 端口与其他PON口对应的ETH 口之间是否隔离*/
extern int SetEthPortTdmIsolateAll( unsigned char TdmSlot, unsigned char en_flag );

/********************************************************
*	恢复一个ETH 端口下所有ONU 的(tdm) 静态MAC 
*
*         这个函数用于在eth 端口在从down 变为up 时，将
*         与这个eth 端口对应的PON 口下的ONU 的(tdm)MAC 地址
*         重新写回到BCM56300
*    输入参数:
*         unsigned char eth_slot -- ETH端口所在的槽位，范围:4-8
*         unsigned char eth_port -- ETH端口号，范围:1-4
********************************************************/
extern int RestoreEthPortMacAndVlan(unsigned char eth_slot, unsigned char eth_port);

/************************************
 恢复TDM 板所有(tdm) MAC 和VLAN 
*************************************/
extern int  RestoreTdmBoardMacAndVlan(unsigned char CardIndex);

/* 删除交换芯片中所有(tdm) MAC (所有TDM接口下所有ONU, 用于TDM板热拔插) */
extern int DeleteTdmMacFromSwForTDMPull(unsigned char TdmSlot);


#endif
