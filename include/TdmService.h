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

/********  FOR onu ����OAM *************
#define ONU_POTS_NUM_MIN   1
#define ONU_POTS_NUM_MAX  32
#define ONU_POTS_ALL  0xffffffff 
*/

/* ONU ����MAC */
#define  SET_ONU_VOICE_MAC_REQ   100
#define  SET_ONU_VOICE_MAC_RSP   100
#define  GET_ONU_VOICE_MAC_REQ   101
#define  GET_ONU_VOICE_MAC_RSP   101
/* ONU ����VLAN */
#define  SET_ONU_VOICE_VLAN_REQ   102
#define  SET_ONU_VOICE_VLAN_RSP   102
#define  GET_ONU_VOICE_VLAN_REQ   103
#define  GET_ONU_VOICE_VLAN_RSP   103
/* ONU ����POTS */
#define  SET_ONU_VOICE_POTS_REQ   104
#define  SET_ONU_VOICE_POTS_RSP   104
#define  GET_ONU_VOICE_POTS_REQ   105
#define  GET_ONU_VOICE_POTS_RSP   105
#define  GET_ONU_VOICE_POTS_STATUS_REQ   106
#define  GET_ONU_VOICE_POTS_STATUS_RSP   106
/* ONU �������� */
#define  SET_ONU_VOICE_LOOP_REQ   107
#define  SET_ONU_VOICE_LOOP_RSP   107
#define  GET_ONU_VOICE_LOOP_REQ   108
#define  GET_ONU_VOICE_LOOP_RSP   108
/* ONU ������չ���� */
#define  SET_ONU_VOICE_EXT_EUQ_REQ   109
#define  SET_ONU_VOICE_EXT_EUQ_RSP   109
/* onu ����ҵ��*/
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


/********* for tdm �忨����******************/
/* tdm ���ڲ�״̬*/
/*
�豸����ģ���ʼ״̬ DEV_ST_INITIALIZING (0)
����FPGA or APP. ����DEV_ST_UPGRADING (1)
FPGA��ʼ����DEV_ST_LOADING (2)
FPGA���ؽ��� DEV_ST_LOADED (3)
���� DEV_ST_RUNNING (6)
����״̬��δ��

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

/* �豸 ��������tdm ��״̬ */
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

/* �豸�������*/
enum {
	TDM_INSERTED = 0x11,       /*  TDM �忨��λ */
	TDM_PULLED,                   /*  TDM �忨�γ�(����λ)*/
	TDM_RESET,                     /*  ��λTDM �� */
	TDM_ACTIVATE,              /*  ����TDM �� */
	TDM_STATUS_QUERY,  /* ��ѯtdm ��״̬*/
	TDM_APP_LOADING,      /*  ����TDM ��APP ���� */
	TDM_FPGA_LOADING,   /*  ����TDM ��FPGA ���� */
	TDM_APP_LOAD_COMP,      /*  ����TDM ��APP ������� */
	TDM_FPGA_LOAD_COMP,   /*  ����TDM ��FPGA ���� ���*/
	TDM_DATA_LOADING    /*  ����TDM ����������*/
};

#define  TDM_COMM_RECV_MSG  0x30   /* ��tdm ����յ���Ϣ(ָδ�������Ϣ)*/
#define  TDM_TIMER_OUT  0x31   /* tdm ����1 �붨ʱ��Ϣ*/

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

/* TDM ������Ϣ�ṹ*/
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
    int                 port;           /* port number ��1��ʼ*/
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

/*****  ���ڲ���BCM56300 MAC ����*/

extern int bms_l2_add(int unit,cli_l2_entry_t *l2entry);
extern int bms_l2_entry_get(int unit,int l2index,cli_l2_addr_t *l2addr,int static_bit);
extern int bms_l2_delete(int unit,int vid,void * Mac,int discard_flag);
extern int bms_isolate_set(int in_port,int out_port,int flag);
extern int bms_l2_idx_get(int unit,int *idx_min,int *idx_max);

/* ����BCM56300 VLAN ����*/
extern LONG MN_Snmp_Vlan_Create( CHAR* pcVlanName,LONG plVlanId,USHORT* pusVid, UCHAR uType,USHORT usLang, CHAR* pcError );
extern LONG MN_Vlan_Check_exist(ULONG  vlanIndex);
extern LONG MN_Vlan_Delete( USHORT usVid, USHORT usLang, CHAR* pcError);
extern LONG MN_Vlan_Get_Vid( USHORT* pusVid, CHAR* pcName, USHORT usLang, CHAR* pcError);
extern LONG MN_Vlan_Set_TaggedPorts(ULONG  vlanIndex , CHAR * portList , ULONG portlist_len);
extern LONG MN_Vlan_Get_AllPorts(ULONG  vlanIndex , CHAR * portList , ULONG * portlist_len);
extern LONG MN_Vlan_Get_UntagPorts(ULONG  vlanIndex , CHAR * portList , ULONG * portlist_len);

#ifdef TDM_MGMT
#endif
/*  tdm �忨�����ڲ�����*/
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

/* tdm �������豸������Ϣ����*/
extern void  TdmCardInserted( unsigned long CardIndex);
extern void  TdmCardPulled ( unsigned long CardIndex);
extern void  TdmCardReset ( unsigned long CardIndex);
extern void  TdmCardActivated(unsigned long CardIndex );
extern int  TdmCardStatusQuery(unsigned long TdmSlot);
extern int  TdmUpdateAppfile(unsigned long CardIndex);
extern int  TdmUpdateAppfileComp(unsigned long CardIndex, unsigned int result );
extern int  TdmUpdateFpgafile(unsigned long CardIndex);
extern int  TdmUpdateFpgafileComp(unsigned long CardIndex, unsigned int result );

/* tdm ������tdm ��ͨ��API
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

/* tdm ����ͨ����Ӧ��Ϣ������
extern int  TdmVersionHandler(unsigned long CardIndex, unsigned char *VersionInfo, unsigned int length);
extern int  TdmFPGALoadingAckHandler(unsigned long CardIndex, unsigned char *LoadFpgaAckInfo, unsigned int length);
extern int  TdmFPGALoadingCompHandler(unsigned long CardIndex, unsigned char *LoadFpgaCompInfo, unsigned int length);
extern int  TdmStatusRspHandler(unsigned long CardIndex, unsigned char *StatusRsp, unsigned int length);
extern void  TdmMgmtLowLevelMsgHandler(unsigned long CardIndex,unsigned char *pBuf, unsigned int length);
*/
extern void  TdmMgmtLowLevelMsgRecv(unsigned char *pBuf, unsigned int length);

#ifdef TDM_SG
#endif
/* ������������/��ѯ */
extern int  SetTdmSGHdlclink(unsigned char TdmSlot,unsigned char SgIdx, unsigned char MasterHdlc, unsigned char SlaveHdlc);
extern int  GetTdmSGHdlclink(unsigned char TdmSlot,unsigned char SgIdx, unsigned char *MasterHdlc, unsigned char *SlaveHdlc);
extern int  SetTdmSGClock(unsigned char TdmSlot,unsigned char SgIdx, unsigned char ClockE1);
extern int  GetTdmSGClock(unsigned char TdmSlot,unsigned char SgIdx,unsigned char *ClockE1);

#ifdef TDM_SG_ONU
#endif
/* ����ONU���/ɾ��/��ѯ */
/*extern int OnuIsSupportVoice(unsigned long OnuDeviceIdx, bool *SupportVoice);*/
extern int  AddOnuToSG(unsigned char TdmSlot, unsigned char SgIdx, unsigned long OnuDeviceIdx, unsigned short int *logicOnu);
extern int  DeleteOnuFromSG(/*unsigned char TdmSlot ,unsigned char SgIdx,*/ unsigned long OnuDeviceIdx);
extern int  DeleteAllOnuFromSG(unsigned char TdmSlot ,unsigned char SgIdx);
extern int  GetOnuBelongToSG(unsigned long OnuDeviceIdx, unsigned char *TdmSlot ,unsigned char *SgIdx,unsigned short int *LogicOnuId);
extern int  GetAllOnuBelongToSG(unsigned char TdmSlot,unsigned char SgIdx, unsigned long *OnuNumber, unsigned long *OnuDeviceIdx);

#ifdef TDM_SG_VLAN
#endif
/* ������������VLAN����/��ѯ*/
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
/* ONU������������/��ѯ  */
extern int  OnuPotsLoopbackEnable(unsigned long OnuDeviceIdx);
extern int  OnuPotsLoopbackDisable(unsigned long OnuDeviceIdx);
extern int  GetOnuPotsLoopback(unsigned long OnuDeviceIdx);
extern int  GetOnuPotsLoopbackStatus(unsigned long OnuDeviceIdx);

#ifdef TDM_ONU_POTS_LINK
#endif
/*  �����������/ɾ��/��ѯ 
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
/* ������չOAM API */
/***************************************************************************
*    ��������: ����ONU����ҵ��ʹ�ܺ�DMAC/SMAC��ַ
*    ����ԭ��: int SetOnuVoiceEnable(unsigned long OnuDeviceIdx,struct OnuVoiceEnable VoiceEn)
*
*    ���������
*           unsigned long OnuDeviceIdx�� ONU�豸������4�ֽڣ�SLOT * 10000 + PON * 1000 + ONU_id
*           �ṹVoiceEn�����ĸ��ֶΣ�
*           unsigned char VoiceEnable �� ONU����ʹ�ܱ�־��1��������0��������
*           unsigned char TdmSlot - TDM�����ڵĲ�λ����λ�ŵı��Ű�GFA6700��Ʒ�涨
*           unsigned char SgIdx�� ��������������ȡֵ��1��3
*           unsigned char LogicOnuIdx �� ONU�����������е��߼���ţ�ȡֵ��1��256
*
*     ��������� ��
*     ����ֵ��
*             (0) -- ����ִ����ȷ����ȷ����ONU����ʹ�ܣ���ONU����MAC
*             (-1) -- ����ִ�д��󣬻������
*              1 -- ONU������
*
***************************************************************************/
extern int SetOnuVoiceEnable(unsigned long OnuDeviceIdx, OnuVoiceEnable *VoiceEn);
/***************************************************************************
*      ��������: ��ѯONU����ҵ��ʹ�ܺ�DMAC/SMAC��ַ
*      ����ԭ�ͣ�
*           int GetOnuVoiceEnable(unsigned long OnuDeviceIdx,unsigned char *VoiceEnable,unsigned char *SrcMac, unsigned char *DestMac);
*
*      ���������
*           unsigned long OnuDeviceIdx�� ONU�豸������4�ֽڣ�SLOT * 10000 + PON * 1000 + ONU_id
*
*       ���������
*          unsigned char *VoiceEnable �� ONU����ʹ�ܱ�־��1��������0��������
*          unsigned char *SrcMac �� ONU������������ԴMAC��ַ������6�ֽ�
*          unsigned char *DestMac �� ONU������������Ŀ��MAC��ַ������6�ֽ�
*
*        ����ֵ��
*           (0��- ����ִ����ȷ�����������Ч
*           (-1) - ����ִ�д��󣬻������
*              1 �� ONU������
*              4 �� ONUû�����õ��κ�һ��SG��
*              6 �� ONU��֧������ҵ��
*
***************************************************************************/
extern int GetOnuVoiceEnable(unsigned long OnuDeviceIdx,unsigned char *VoiceEnable,unsigned char *SrcMac, unsigned char *DestMac);
/***************************************************************************
*      ��������: ����ONU����VLANʹ�ܺ�tag
*      ����ԭ�ͣ�
*          int SetOnuVoiceVlan(unsigned long OnuDeviceIdx,struct VoiceVlanEnable VoiceVlan);
*
*        ���������
*           unsigned long OnuDeviceIdx�� ONU�豸������4�ֽڣ�SLOT * 10000 + PON * 1000 + ONU_id
*           �ṹVoiceVlan���������ֶΣ�
*           unsigned char Enable �����������Ƿ����VLAN��ǩ��1����tag��0������tag
*           unsigned char Priority - �������ݱ��ĵ����ȼ���ȡֵ��0��7��Ĭ��Ϊ7
*           unsigned short int VlanId�� �������ݱ���VLAN id��ȡֵ��1��4094
*
*        ��������� ��
*
*        ����ֵ��
*            (0��- ����ִ����ȷ����ȷ����ONU����VLAN
*            (-1) - ����ִ�д��󣬻������
*             1 �� ONU������ 
*
***************************************************************************/
extern int SetOnuVoiceVlan(unsigned long OnuDeviceIdx, VoiceVlanEnable *VoiceVlan);
extern int GetOnuVoiceVlan(unsigned long OnuDeviceIdx,  VoiceVlanEnable *VoiceVlan);
/***************************************************************************
*        ��������: ����ONU POTS��ʹ��
*        ����ԭ�ͣ�
*          int SetOnuPotsEnable(unsigned long OnuDeviceIdx,unsigned char PotsEnable,unsigned char PotsNum,unsigned long PotsList);
*
*        ���������
*            unsigned long OnuDeviceIdx�� ONU�豸������4�ֽڣ�SLOT * 10000 + PON * 1000 + ONU_id
*            unsigned char PotsEnable �� POTS��ʹ�ܱ�־��1��ʹ�ܣ�0���ر�
*            unsigned char PotsNum - Ҫ���õ�Pots�ڸ�����
*            unsigned int PotsList �� POTS�˿��б�ÿһ��bit��Ӧһ��POTS�˿ڣ���1ʱ��ʾ�ڶ˿���ִ��PotsEnable��ʶ�Ĳ���������0��ʾ�˿��ϲ�ִ��PotsEnable��ʶ�Ĳ���
*
*        ��������� ��
*
*        ����ֵ��
*               (0��- ����ִ����ȷ����ȷ����ONU POTSʹ�ܻ�ر�
*               (-1) - ����ִ�д��󣬻������
*               1 �� ONU������ 
*
***************************************************************************/
extern int SetOnuPotsEnable(unsigned long OnuDeviceIdx,unsigned char PotsEnable,unsigned char PotsNum,unsigned long PotsList);
extern int GetOnuPotsEnable(unsigned long OnuDeviceIdx,unsigned char PotsNum,unsigned long PotsList, unsigned long *PotsEnableList);
extern int GetOnuPotsStatus(unsigned long OnuDeviceIdx,unsigned long *PotsStatusList);
/************************************************************************
*    ��������: ����ONU ��������
*    ����ԭ��: int SetOnuVoiceLoopbackStatus(unsigned long OnuDeviceIdx, unsigned char LoopStatus)
*
*    ���������
*           unsigned long OnuDeviceIdx �� ONU�豸������4�ֽڣ�SLOT * 10000 + PON * 1000 + ONU_id
*           unsigned char LoopStatus �� ONU ��������״̬��1�����ء�0��ֹͣ����
*    �����������
*    ����ֵ:
*           (0�� - ����ִ����ȷ�����������Ч
*           (-1)  - ����ִ�д��󣬻���������������Ч
*            1 �� ONU������
*    ˵��: ͨ��ONU������չOAM������ONU�������أ����ڲ��ԣ�����Ӧ��OAM��Ϣ��5.2 ����ONU��������
*
**********************************************************************/
extern int SetOnuVoiceLoopbackStatus(unsigned long OnuDeviceIdx, unsigned char LoopStatus);
extern int GetOnuVoiceLoopbackStatus(unsigned long OnuDeviceIdx, unsigned char *LoopStatus);


extern int GetOnuVoiceDeviceInfo(unsigned long onuDevIdx, unsigned char *SlotNum, unsigned char *SlotInfo, unsigned char *Power, unsigned char *Battery, unsigned char *Surronding);

extern int ConfigOnuTdmVoiceService(unsigned long OnuDeviceIdx);
extern int ConfigAllOnuTdmVoiceService(unsigned char TdmSlot ,unsigned char SgIdx);

/************************************************************************
*    ��������: ����ONU ����ҵ������
*      ����ԭ�ͣ�
*             int SetOnuVoiceService(unsigned long OnuDeviceIdx, struct OnuVoiceEnable VoiceEn , struct VoiceVlanEnable VlanEn, unsigned long PotsEnableList)
*
*       ���������
*            unsigned long OnuDeviceIdx �� ONU�豸������4�ֽڣ�SLOT * 10000 + PON * 1000 + ONU_id
*           �ṹVoiceEn�����ĸ��ֶΣ�
*           unsigned char VoiceEnable �� ONU����ʹ�ܱ�־��1��������0��������
*           unsigned char TdmSlot - TDM�����ڵĲ�λ����λ�ŵı��Ű�GFA6700��Ʒ�涨
*           unsigned char SgIdx�� ��������������ȡֵ��1��3
*           unsigned char LogicOnuIdx �� ONU�����������е��߼���ţ�ȡֵ��1��256
*           �ṹVlanEn���������ֶΣ�
*           unsigned char Enable �����������Ƿ����VLAN��ǩ��1����tag��0������tag
*           unsigned char Priority - �������ݱ��ĵ����ȼ���ȡֵ��0��7��Ĭ��Ϊ7
*           unsigned short int VlanId�� �������ݱ���VLAN id��ȡֵ��1��4094
*           
*           unsigned long PotsEnableList �� POTS�˿�ʹ���б�ÿһ��bit��Ӧһ��POTS�˿�
*
*        �����������
*
*        ����ֵ��
*             (0�� - ����ִ����ȷ������ONU����ҵ������
*             (-1)  - ����ִ�д��󣬻������ 
*              1 �� ONU������
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
/*��������VLAN */
extern int  CreateVoiceVlan(unsigned char TdmSlot, unsigned char SgIdx,  unsigned short int VoiceVid);
/* ɾ������VLAN */
extern int  DeleteVoiceVlan( unsigned char TdmSlot, unsigned char SgIdx, unsigned short int VoiceVid );
/* ������Ϊ����ɾ������VLAN */
extern int  DeleteVoiceVlanByName( unsigned char TdmSlot, unsigned char SgIdx );
extern int  DeleteVoiceVlanByNameAll( unsigned char TdmSlot);

/* ������VLAN ����Ӷ˿�*/
extern int  AddPortToVoiceVlan(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int VlanId );
#endif

#ifdef  _TDM_BOARD_SG_VOICE_OPERATION_
#endif
/*************************************************************
*        ���º��������ƽ̨��VLAN��MAC ��
* ��ز�����װ��TDM ����API�������� ��
* ��/ ɾ������ONU��ʹ��/ �޸� ����VLAN  ��
* 
* ����ȡֵ:
*                unsigned char TdmSlot -- tdm �忨����; ȡֵ4 - 8
*                unsigned char SgIdx -- tdm ��������������; ȡֵ: 1- 4
*                unsigned int OnuDevIdx -- ONU �豸����
*                unsigned short int VoiceVid -- ����VLAN��ȡֵ1 - 4094; 
*                unsigned short int LogicOnuIdx -- ONU �������������߼���ţ�ȡֵ1 �� 256
*
***************************************************************/
/* ��	����оƬBCM56300 �����Ӿ�̬����MAC ( ��������ONU ) */
extern int AddVoiceMacToSw(unsigned char TdmSlot, unsigned char SgIdx,  unsigned int OnuDevIdx, unsigned short int VoiceVid, unsigned short int LogicOnuIdx);
/* ��	����оƬBCM56300 �����Ӿ�̬����MAC (��������ONU )*/
extern int AddVoiceMacToSwAll(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int VoiceVid, unsigned int OnuCount, unsigned long *SgOnuIdx);
/* ɾ������оƬ�о�̬����MAC (����ONU ) */
extern int DeleteVoiceMacFromSw(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicOnuIdx);
/* ɾ������оƬ�����о�̬����MAC (ָ��TDM�ӿ�������ONU ) */
extern int DeleteVoiceMacFromSwAll(unsigned char TdmSlot, unsigned char SgIdx, unsigned int OnuCount, unsigned long *SgOnuIdx);
/* �ָ�һ����������������vlan ����������ONU ��̬MAC */
extern int RestoreSWVoiceMacAndVlan(unsigned char TdmSlot, unsigned char SgIdx);
/* ����TDM sigʱ���ָ���ص�VLAN ������MAC */
extern int  RestoreTdmSgBoardVoiceMacVlan( unsigned char TdmSlot);

#ifdef  _TDM_BOARD_E1_LINK_OPERATION_
#endif
/******************************************************************
 *    Function: int AddOnuE1LinkMacToSw(unsigned char TdmSlot, unsigned char SgIdx,  unsigned int OnuDevIdx, unsigned short int VoiceVid, unsigned short int LogicOnuIdx)
 *
 *    Param:  unsigned char TdmSlot -- tdm �忨����; ȡֵTDMCARD_FIRST - TDMCARD_LAST
 *                unsigned char fpgaIdx -- tdm ������e1 ������; ȡֵ: TDM_FPGA_MIN - TDM_FPGA_MAX
 *                unsigned char E1Idx -- ��һ��E1���е�˳���ţ�ȡֵ: 1 - MAX_E1_PER_FPGA
 *                unsigned int OnuDevIdx -- ONU �豸����
 *                unsigned short int E1_slotPort -- E1 ���Ӷ�Ӧ��ONU���E1����
 *                unsigned short int E1LinkVid -- E1-link vlan-id, 1-4094
 *                            
 *    Desc:   ��	����оƬBCM56300 �����Ӿ�̬E1-link MAC (ֻ���ONU ��)
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
*	�ӽ���оƬ56300 ��ɾ��һ��E1 ���Ӷ�Ӧ��MAC ��ַ(ֻ���ONU ��) 
*   ����ȡֵͬ��
**************************************************************************************/
extern int DelOnuE1LinkMacFromSw(unsigned char TdmSlot, unsigned fpgaIdx, unsigned char E1Idx, unsigned OnuDevIdx/*, unsigned short int E1_slotPort*/ );

/******************************************************************
 *    Function:  int AddFpgaE1LinkMacToSw(unsigned char TdmSlot, unsigned char fpgaIdx, unsigned short int E1LinkVid)
 *
 *    Param:  unsigned char TdmSlot -- tdm �忨����; ȡֵTDMCARD_FIRST - TDMCARD_LAST
 *                unsigned char fpgaIdx -- tdm ������e1 ������; ȡֵ: TDM_FPGA_MIN - TDM_FPGA_MAX
 *                unsigned short int E1LinkVid -- E1-link vlan-id, 1-4094
 *                            
 *    Desc:   �򽻻�оƬBCM56300 ������һƬfpga ��̬E1-link MAC (ֻ���tdm ��)
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
extern int AddFpgaE1LinkMacToSw(unsigned char TdmSlot, unsigned char fpgaIdx, unsigned short int E1LinkVid);

/* �ӽ���оƬ56300 ��ɾ��һƬfpga E1 ���Ӷ�Ӧ��MAC ��ַ(ֻ���tdm ��) */
extern int DelFpgaE1LinkMacFromSw(unsigned char TdmSlot, unsigned fpgaIdx );


#ifdef __TDM_BOARD_OPERATION__
#endif
/********************************************************
*                                                                                                     *
*        ע: ���º�����ʹ������������TDM ��    *
*                                                                                                     *
*********************************************************/

/* ����һ��TDM �˿�������ETH�˿�֮���Ƿ����*/
extern int SetEthPortTdmIsolate( unsigned char TdmSlot, unsigned char tdmPort, unsigned char en_flag );

/* ����TDM���Ӧ��ETH �˿�������PON�ڶ�Ӧ��ETH ��֮���Ƿ����*/
extern int SetEthPortTdmIsolateAll( unsigned char TdmSlot, unsigned char en_flag );

/********************************************************
*	�ָ�һ��ETH �˿�������ONU ��(tdm) ��̬MAC 
*
*         �������������eth �˿��ڴ�down ��Ϊup ʱ����
*         �����eth �˿ڶ�Ӧ��PON ���µ�ONU ��(tdm)MAC ��ַ
*         ����д�ص�BCM56300
*    �������:
*         unsigned char eth_slot -- ETH�˿����ڵĲ�λ����Χ:4-8
*         unsigned char eth_port -- ETH�˿ںţ���Χ:1-4
********************************************************/
extern int RestoreEthPortMacAndVlan(unsigned char eth_slot, unsigned char eth_port);

/************************************
 �ָ�TDM ������(tdm) MAC ��VLAN 
*************************************/
extern int  RestoreTdmBoardMacAndVlan(unsigned char CardIndex);

/* ɾ������оƬ������(tdm) MAC (����TDM�ӿ�������ONU, ����TDM���Ȱβ�) */
extern int DeleteTdmMacFromSwForTDMPull(unsigned char TdmSlot);


#endif
