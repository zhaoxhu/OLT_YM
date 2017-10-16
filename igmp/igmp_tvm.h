#ifndef __IGMP_TVM_H__
#define __IGMP_TVM_H__


#ifdef __cplusplus
extern "C"
{
#endif
#include "include/route/Vrf.h"
#include "include/Vos_global.h"
#include "vos/vospubh/Vos_types.h"
#include "vos/vospubh/Vos_mem.h"
#include "vos/vospubh/Vos_sysmsg.h"
#include "vos/vospubh/Vos_que.h"
#include "switch/igmp_snoop/igmp_snoop.h"
#include  "V2R1_product.h"
/*#include  "OltGeneral.h"*/
#include "vos/vospubh/cdp_syn.h"
#include	"vos/vospubh/cdp_pub.h" 
typedef struct Through_Vlan_Group {
	USHORT IVid;
	USHORT PonId;
	ULONG   GROUPS;
	ULONG   GROUPE;
	ULONG  ulIfIndex;
	ULONG  llid;
	struct Through_Vlan_Group *next;
} Through_Vlan_Group_t;

typedef struct Tvm_Pkt{
	USHORT IVid;
	USHORT PonId;
	ULONG   GROUPS;
	ULONG   GROUPE;
	ULONG  ulIfIndex;
	ULONG  llid;
}__attribute__((packed))Tvm_Pkt_t;

typedef struct Tvm_Pkt_Head{
	USHORT enable;
	USHORT type;
	ULONG  CRC;
	USHORT count;
}__attribute__((packed))Tvm_Pkt_Head_t;

typedef struct Tvm_Pon_Onu{
	USHORT PonId;
	short int  OnuId;
	int  Calcus;
	int  Enable;
}Tvm_Pon_Onu_t;

typedef struct TVM_Cont_Head{
	INT TVMCOUNT;
	int  Calculates;
	Through_Vlan_Group_t *Through_Vlan_Group_head;
}TVM_Cont_Head_t;

enum  {
	HEAD_IS_NULL = 1,
	CONNECT_LEFT,
	CONNECT_RIGHT,
	CONNECT_BOTH,
	CONNECT_NOT
    };

enum {
	TVM_ENABLE = 1,
	TVM_DISABLE
	};
enum{
	TVM_ADD = 1,
	TVM_DEL,
	TVM_CRC,
	TVM_DEL_VID
};

typedef enum
{
	igmp_tvm_enable ,
	igmp_tvm_add,
	/*igm_tvm_del_all,*/
	igmp_tvm_delitem,
	igmp_tvm_delall,
	igmp_tvm_delvid,
	igmp_tvm_sync,
	igmp_tvm_stop
}IgmpTvm_Cmd_Type_t;

typedef struct{
	USHORT    enable;
	USHORT	sync;
	USHORT count;
} TvmInsertCDPMsgHead_t;

#if( RPU_MODULE_IGMP_TVM == RPU_YES )

STATUS  Send_To_IgmpQue_Tvm(void * pointer ,short int tvmtype) ;

STATUS DelIgmpSnoopTvmItem(ULONG mulips, ULONG mulipe,struct vty * vty);

STATUS AddIgmpSnoopTvmItem(ULONG mulips, ULONG mulipe, unsigned int vid, struct vty * vty);

STATUS Igmp_Tvm_Recv_Enable(SYS_MSG_S *pstMsg);

STATUS Igmp_Tvm_Recv_Add(SYS_MSG_S *pstMsg);

STATUS Igmp_Tvm_Recv_Del(SYS_MSG_S *pstMsg);

STATUS Igmp_Tvm_Recv_Mod(SYS_MSG_S *pstMsg);

STATUS Igmp_Tvm_Recv_Cal(SYS_MSG_S *pstMsg);

STATUS Igmp_Tvm_Table_Send( ULONG PonPortIdx, ULONG OnuIdx, INT flag);
STATUS funIgmpTvmInit(void);

void funIgmpTvmOamReqCallBack(unsigned short usPonId,  
                                      unsigned short usOnuId, unsigned short llid,  unsigned short usDatLen, 
                                      unsigned char *pDatBuf,  unsigned char *pSessId);

int Igmp_Tvm_RPC_CALL(IgmpTvm_Cmd_Type_t  cmd_type,short int PonPortIdx,ULONG para1,
				ULONG mulIps, ULONG mulIpe, USHORT vid);
VOID IgmpTvm_CMD2LIC_RPC_Callback( ULONG ulSrcNode, ULONG ulSrcModuleID,
                               VOID * pReceiveData, ULONG ulReceiveDataLen,
                               VOID **ppSendData, ULONG * pulSendDataLen );
VOID tvmTimerCallback();
STATUS  Tvm_Table_Crc();
STATUS DelIgmpSnoopTvmAll();
STATUS DelIgmpSnoopTvmVid(unsigned int  mcastVid, struct vty * vty);
STATUS Igmp_snoop_Stop_Tvm(struct vty *vty);
STATUS Igmp_Tvm_Debug_Head(char *buftemp);
STATUS CliIgmpTvmShowRun(struct vty *vty);
int Add_Igmp_Tvm_Item(unsigned int PonPortIdx,ULONG mulIps, ULONG mulIpe, USHORT mcastVid, void *vty);
int Del_Igmp_Tvm_Item(unsigned int PonPortIdx,ULONG mulIps, ULONG mulIpe, void *vty);
int Del_Igmp_Tvm_Vid(unsigned int PonPortIdx,USHORT mcastVid,void *vty);
int Del_Igmp_Tvm_All(unsigned int PonPortIdx,void *vty);
int Set_Igmp_Tvm_Sync(unsigned int PonPortIdx,long syncvalue);
int Stop_Igmp_Tvm(unsigned int PonPortIdx,void *vty);
int Igmp_tvm_cli_init( VOID );
int Igmp_tvm_cli_init_pon( VOID );
int Tvm_Insert_Cdp_Send(LONG slotno, char *buf, USHORT length);
STATUS  TvmInsert_Poncard_Insert_Callback(ULONG ulFlag, ULONG ulChID, ULONG ulDstNode,	 ULONG ulDstChId, VOID  *pData, ULONG ulDataLen);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* end of __IGMP_TVM_H__ */
