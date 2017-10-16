#ifdef __cplusplus
extern "C"
  {
#endif

#include  "OltGeneral.h"
/*#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"*/
#include  "gwEponSys.h"	/*add by zhengyt@10-4-6*/
#include	"vos/vospubh/cdp_pub.h" 
#include "vos/vospubh/vos_sysmsg.h"
#include "sys/main/sys_main.h"
#include "Eth_aux.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_NO )
#include "Mac_cli.h"
#endif
#include "Ifm_pub.h"
#include "vos/vospubh/Cdp_syn.h"
/*#include "eventMain.h"*/
#include "ethLoopChk.h"
#include "eventOam.h"
#include "../ct_manage/CT_Onu_event.h"

#include "bcm/l2.h"
#undef CHK_BRAS_MAC
EthLoopVlan_t * FindEthLoopVlanEntry(int vid, EthLoopVlan_t ** EthLoopVlan_Pre);
extern int setL2uCpu(int index,int cpu);

ULONG loopChkDebugSwitch = 0;
#define loopChkDbgPrintf(_l, _x) if (_l & loopChkDebugSwitch) {sys_console_printf _x;}

ULONG loopChk_debug_switch = 0;
ULONG chk_config_detect_mac_clean_enable = 0;
ULONG LoopChkOnuIncludeExpandInfo = 1;

ULONG RPC_Loop_CmdIDNumber = 0;
enum {
	EthLoopPortlistIsDesigned=0,
	EthLoopPortlistIsDefault,
	EthLoopPortlistIsAll,
};
LONG ETHLOOPPORTLISTFLAG =  EthLoopPortlistIsDefault;

int L2U_INDEX_NUM = 0;

LONG EthLoopPortlistIsAllFlag = 0;
#ifdef CHK_BRAS_MAC
static UCHAR  chk_bras[CHK_BRAS_MAC_MAX][ETH_LOOP_MAC_ADDR_LEN];/*={0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00,
																0x00,0x00,0x00,0x00,0x00,0x00};*/
#endif
/* 记录端口或ONU环回状态 */
UCHAR chk_result_eth_port[CHK_SLOT_MAX+1][CHK_SLOT_PORT_MAX+1];
USHORT chk_result_eth_port_vid[CHK_SLOT_MAX+1][CHK_SLOT_PORT_MAX+1] ;
UCHAR chk_result_pon_onu[CHK_SLOT_MAX+1][CHK_SLOT_PORT_MAX+1][MAXONUPERPONNOLIMIT+1];/*CHK_PON_ONU_MAX*/
#define CHK_RESULT_LOOP_NULL		0x00
#define CHK_RESULT_LOOP_STATUS	0x01
#define CHK_RESULT_LOOP_ALARM		0x02
#define loop_port_status_get(slotno, portno)  (chk_result_eth_port[slotno][portno] & CHK_RESULT_LOOP_STATUS)
#define loop_port_status_set(slotno, portno)  (chk_result_eth_port[slotno][portno] |= CHK_RESULT_LOOP_STATUS)
#define loop_port_status_cls(slotno, portno)  (chk_result_eth_port[slotno][portno] &= (~CHK_RESULT_LOOP_STATUS))
#define loop_port_alarm_get(slotno, portno)  (chk_result_eth_port[slotno][portno] & CHK_RESULT_LOOP_ALARM)
#define loop_port_alarm_set(slotno, portno)  (chk_result_eth_port[slotno][portno] |= CHK_RESULT_LOOP_ALARM)
#define loop_port_alarm_cls(slotno, portno)  (chk_result_eth_port[slotno][portno] &= (~CHK_RESULT_LOOP_ALARM))
#define loop_onu_status_get(slotno, portno, onuno)  (chk_result_pon_onu[slotno][portno][onuno] & CHK_RESULT_LOOP_STATUS)
#define loop_onu_status_set(slotno, portno, onuno)  (chk_result_pon_onu[slotno][portno][onuno] |= CHK_RESULT_LOOP_STATUS)
#define loop_onu_status_cls(slotno, portno, onuno)  (chk_result_pon_onu[slotno][portno][onuno] &= (~CHK_RESULT_LOOP_STATUS))
#define loop_onu_alarm_get(slotno, portno, onuno)  (chk_result_pon_onu[slotno][portno][onuno] & CHK_RESULT_LOOP_ALARM)
#define loop_onu_alarm_set(slotno, portno, onuno)  (chk_result_pon_onu[slotno][portno][onuno] |= CHK_RESULT_LOOP_ALARM)
#define loop_onu_alarm_cls(slotno, portno, onuno)  (chk_result_pon_onu[slotno][portno][onuno] &= (~CHK_RESULT_LOOP_ALARM))

/* 环路检测周期 */
#define MAX_CHECK_TIMER_INTERVAL	3600		/* 最大门限，单位: 秒*/
#define MIN_CHECK_TIMER_INTERVAL	10		/* 最小门限，单位: 秒*/
#define DEF_CHECK_TIMER_INTERVAL	30 		/* 默认值，单位: 秒 */
#define SEND_CHECK_FRAME_INTERVAL	1000	/* 检测帧发送周期，单位: 毫秒 */
#define PORTDOWN_TO_UP_TIMES              3   		/*检测周期的倍数*/ 
#define LINKUP_TIMES_THRESH                 3              /*link up 的最大次数*/

BOOL  chk_config_detect_enable = FALSE;			/* 环路检测使能 */
BOOL  chk_config_detect_control_enable = FALSE;	/* 环路检测控制使能 */
ULONG  chk_config_detect_mode = ETH_LOOP_CHECK_MODE_OLT_ONU;	/* 环路检测模式 */
#ifdef CHK_BRAS_MAC
BOOL   chk_BRAS_mac_enable=FALSE;                /*BRAS  mac学习到PON MAC表的检测使能*/
#endif

/* 环路检测配置信息 */
UCHAR chk_config_frame_dmac[ETH_LOOP_MAC_ADDR_LEN] = { 0x00, 0x0f, 0xe9, 0x04, 0x8e, 0xdf };	/*modified by zhengyt@10-4-15,环路检测的mac地址固定为单播地址*/
UCHAR chk_config_frame_smac[ETH_LOOP_MAC_ADDR_LEN] = { 0x00, 0x0f, 0xe9, 0x07, 0x80, 0x11 };
UCHAR chk_config_frame_smac_backup[ETH_LOOP_MAC_ADDR_LEN] = { 0x00, 0x0f, 0xe9, 0x07, 0x80, 0x11 };

BOOL  chk_config_frame_BRAS_mac_flag=FALSE;

extern int addL2uEntry(sal_mac_addr_t addr,sal_mac_addr_t maskf,int discard,int cpu,int bpdu);
extern ULONG DEV_GetPhySlot(VOID);
void setOnuEthLoopEnable(int enable);
USHORT  chk_config_frame_ethtype = 64;	/* 0x0800 */
USHORT  chk_config_eth_vid = 0;
UCHAR  chk_config_eth_portlist[CHK_SLOT_MAX+1][CHK_SLOT_PORT_MAX+2];
UCHAR  chk_config_eth_portlist_old[CHK_SLOT_MAX+1][CHK_SLOT_PORT_MAX+2];
ULONG  chk_config_timer_len = DEF_CHECK_TIMER_INTERVAL;
USHORT  chk_config_up_times=PORTDOWN_TO_UP_TIMES;
USHORT  chk_config_up_times_thresh=LINKUP_TIMES_THRESH;
USHORT mn_relay_up_times = 0;/*用于缓存网管输入参数，与门限一起配置*/
UCHAR ethloop_devsm_pull_flag[CHK_SLOT_MAX+1][2];
enum eth_devsm_pull_flag
{
	eth_devsm_normal = 0,
	eth_devsm_pulled,
	eth_devsm_difference
};

enum EthLoopClearEntryFlag
{
	NoThingToDo = 0,
	NotClearEntry ,
	ClearTheEntry 
};

enum eth_devsm_pull_type
{
	ethloop_devsm_zero = 0,
	ethloop_devsm_eth,
	ethloop_devsm_pon
};
/* 环路检测的最大槽位号和端口号 */
ULONG eth_loop_check_slot_max = CHK_SLOT_MAX;
UCHAR eth_loop_check_port_max[CHK_SLOT_MAX+1];

#define ETH_LOOP_AGING_TIME		11
#define ETH_LOOP_REPORT_TIME		(ETH_LOOP_AGING_TIME+1)
ethloop_port_listnode_t *eth_port_loop_list = NULL;

ethloop_listnode_Switch_t *eth_loop_list_switch = NULL;

ULONG ethloop_semid = 0;
ULONG Flag_Vlan_Have_Valid_Port = 0;

extern STATUS getEthPortOperStatus( ulong_t devIdx, ulong_t brdIdx, ulong_t ethIdx, ulong_t *status );
extern int bms_l2_idx_get(int unit,int *idx_min,int *idx_max);
extern LONG swport_2_slot_port(int sw_port, ULONG *ulSlot, ULONG *ulPort);
extern LONG MN_Vlan_Get_current_vlans(ULONG * vlanNumber);
extern LONG MN_Vlan_Get_First( USHORT* pusVid,USHORT usLang, CHAR* pcError );
extern LONG MN_Vlan_Get_Next( USHORT usVid, USHORT* pusNextVid, USHORT usLang, CHAR* pcError);
extern LONG MN_Vlan_Get_AllPorts(ULONG  vlanIndex , CHAR * portList , ULONG * portlist_len);
extern LONG MN_Vlan_Get_UntagPorts(ULONG  vlanIndex , CHAR * portList , ULONG * portlist_len);
extern ULONG bms_packet_send_by_port( VOID * buf, USHORT ulLength, USHORT usSlot,USHORT usPort, USHORT usVid );
extern ULONG ulIfindex_2_userSlot_userPort( ULONG ulIfIndex, ULONG *ulUserslot, ULONG *ulUserport );
extern LONG GetMacAddr(CHAR * szStr, CHAR * pucMacAddr);
extern int bms_l2_entry_get(int unit,int l2index,cli_l2_addr_t *l2addr,int static_bit);

int check_mac_entry_search_by_pon( ULONG slotno, ULONG portno, ULONG *p_onuid );
VOID check_eth_frame_send_by_port( ULONG slotno, ULONG portno, USHORT vid, USHORT UntagVid );
VOID check_oam_frame_send_by_port( ULONG slotno, ULONG portno, USHORT vid , UCHAR DisableFlag);
int check_vlan_entry_get_by_switch( USHORT vid, UCHAR *p_all_portlist, UCHAR *p_untag_portlist );
int check_vlan_entry_getnext_by_switch( USHORT vid, USHORT *pnext_vid, UCHAR *p_all_portlist, UCHAR *p_untag_portlist );
#ifdef CHK_BRAS_MAC
int check_mac_entry_delete_by_switch( USHORT vid );
#endif
int check_mac_entry_delete_by_pon( ULONG slotno, ULONG portno );
int check_mac_entry_delete( ULONG slotno, ULONG portno ,int devIdx);
VOID ethLoopCheckPortlistAll();
VOID ethLoopCheckTestFrameSend( USHORT send_vid, UCHAR *p_all_portlist, UCHAR *p_untag_portlist );
VOID ethLoopCheckTestFrameSendByVlanAndPort( USHORT send_vid, ULONG ulSlot ,ULONG ulPort );
VOID ethLoopCheckTestMacProcess();
VOID ethLoopCheckTestFrameProcess();
VOID oltMode_ethLoopCheckAlarmReportProcess();
VOID onuMode_ethLoopCheckAlarmReportProcess();
VOID ethLoopCheckLinkdownProcess( ULONG slotno, ULONG portno, ULONG onuno );
int onuMode_ethLoopCheckOnuPortClearProcess(ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG vid);
int ethLoopCheckConfigCommandProcess( qdef_ethchk_cmd_t *pCmd);
int ethLoopCheckPortlistNum();
VOID ethLoopCheckPortlistDefault();
BOOL eth_loopcheck_portlist_is_default();
VOID ethLoopCheckOnuLineOffProcess( ULONG devIdx );
VOID ethLoopCheckTestOamSend( USHORT send_vid, ULONG *p_all_portlist, ULONG *p_untag_portlist );
/*int ethLoopCheckPortIsExist(ULONG chk_slotno, ULONG chk_portno );
int ethLoopCheckEthBoardIsExist( );*/
extern LONG RPU_SendCmd2Loop(qdef_ethchk_cmd_t chkcmd);
extern int eventOamMsg_onuEthPortLoop( ushort_t ponId, ulong_t llId, eventOnuEthLoopMsg_t *pOamMsg );
#ifdef CHK_BRAS_MAC
int cheak_BRAS_mac_entry_delete_by_switch(USHORT vid,UCHAR  Pmac[6]);
int  ethLoopCheckTestBRASMacProcess();
#endif

int onuEthPortLoopReportToMaster(int MsgType, void *node, USHORT ClearFlag);
int delEthPortFromLoopList( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG vid,ULONG loopdevIdx,  ULONG loopbrdIdx, ULONG loopportIdx, ULONG loopvid, USHORT ClearFlag);

LONG ethLoopCheckPortCommand_Add( qdef_ethchk_cmd_t *pchkcmd );
LONG ethLoopCheckImmediatelyCommand( qdef_ethchk_cmd_t *pchkcmd );

/*typedef struct
{
	unsigned char bMacAdd[6];
}Enet_MACAdd_Infor1;
extern Enet_MACAdd_Infor1 *funReadMacAddFromNvram( Enet_MACAdd_Infor1 *macStructure);*/

#define CHECK_SLOTNO_IS_ETH(slotno) SYS_MODULE_IS_UPLINK(slotno)
#define CHECK_SLOTNO_IS_PON(slotno) (SYS_MODULE_IS_PON(slotno) || (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6100_MAIN)\
									||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8100_16EPONB0)||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8100_16GPONB0))


enum EthLoop_CdpType
{
	LOOP_PONCARD_INSERT,
	LOOP_MAC_ENTRY_DELETE,
	LOOP_ONU_REPORT_ENTRY,
	LOOP_ONU_CLEAR_ENTRY	,
	LOOP_12EPON_ETH_LOOP,
	LOOP_12EPON_ETH_CLEAR, /* 只有主控板上接受linkdown/linkup 的消息处理通知链情景下这个case 不使用了2012-05-31 */
	LOOP_NOTIFY_12EPON_ETH_CLEAR,
	LOOP_ONU_SWITCH_REPORT,
	LOOP_ONU_SWITCH_CLEAR,
	LOOP_FLAG_END
};

#define ETH_LOOP_CHK_QUE_LEN_MAX		100
ULONG ethLoopChkQueId = 0;
VOS_HANDLE ethLoopChkTaskId = NULL;
LONG ethLoopChkTimerId = VOS_ERROR;

#if 0
PON_address_record_t pon_mac_addr_table[PON_ADDRESS_TABLE_SIZE+1];
#else
PON_address_record_t *pon_mac_addr_table = NULL;
#endif

BOOL  eth_loop_chk_start_flag = TRUE;			/* 环路检测结束标志 */
ULONG eth_loop_chk_tx_pkts_ctrl_number = 25;
ULONG eth_loop_chk_tx_pkts_ctrl_delay = 5;
ULONG eth_loop_chk_tx_pkts_ctrl = 0;

VOID ethLoopChkTask();
VOID ethLoopChkTimerCallback();
LONG ethloop_module_init();

	
#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
/*begin: added by jianght 20081112*/
extern STATUS alarm_Environment_Init();
 /*end: added by jianght 20081112*/   
#endif

/* begin: added by jianght 20090520 */
#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
extern STATUS powerBrdPowerOnAndOffAlarm_Command_Install();
#endif
/* end: added by jianght 20090520 */

ULONG Onu_Eth_Loop_Frame_Num = 0; /*add by duzhk*/
ULONG Onu_Loop_Flag = 0;

EthLoopVlan_t *EthLoopVlan_List = NULL;
static USHORT chk_vid_bak = 0;
extern VOID LOOP_CMD2LIC_RPC_Callback( ULONG ulSrcNode, ULONG ulSrcModuleID,
                               VOID * pReceiveData, ULONG ulReceiveDataLen,
                               VOID **ppSendData, ULONG * pulSendDataLen );

/*STATUS  LOOP_CDP_Callback(ULONG ulFlag, ULONG ulChID,ULONG ulDstNode,ULONG ulDstChId, VOID  *pData, ULONG ulDataLen)
{
	LoopCDPMsgHead_t * revBuf = NULL , *pQueueMsg = NULL;
	unsigned long  ulMessage[4] ={MODULE_LOOPBACK, FC_ETHCHK_ONU_PORT_LOOP, 0, 0};
	ULONG SlotnoOnOlt = 0, PortnoOnOlt = 0;
    
	switch( ulFlag )
	{
		case CDP_NOTI_FLG_RXDATA: 
			        
				if( pData == NULL )
				{
					VOS_ASSERT( 0 );
					return VOS_ERROR;
				}

				revBuf=(LoopCDPMsgHead_t *)(( SYS_MSG_S * )pData + 1);

				SlotnoOnOlt = revBuf->srcslot;
				PortnoOnOlt = revBuf->ponId+1;
				revBuf->ponId = GetPonPortIdxBySlot(SlotnoOnOlt,PortnoOnOlt);

				if(ulDstChId == RPU_TID_CDP_LOOP && ulChID == RPU_TID_CDP_LOOP)
				{
			 		pQueueMsg = (LoopCDPMsgHead_t*)VOS_Malloc(sizeof(LoopCDPMsgHead_t)+sizeof(eventOnuEthLoopMsg_t), MODULE_LOOPBACK);
					if(pQueueMsg == NULL)
					{
						CDP_FreeMsg(pData);
						return VOS_ERROR;
					}
				}
				VOS_MemCpy(pQueueMsg , revBuf, sizeof(LoopCDPMsgHead_t)+sizeof(eventOnuEthLoopMsg_t));

				ulMessage[3] = (unsigned long)pQueueMsg;
			            
				if(VOS_QueSend(ethLoopChkQueId, ulMessage, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
				{
					CDP_FreeMsg(pData);
					return VOS_ERROR;
				}
				CDP_FreeMsg(pData);
				break;
		case CDP_NOTI_FLG_SEND_FINISH:
			CDP_FreeMsg(pData);		
			break;
		default:
			ASSERT(0);
			CDP_FreeMsg(pData);
			break;
	}
	return VOS_OK;
}
*/
STATUS  LOOP_CDP_Callback_Pon(ULONG ulFlag,	ULONG ulChID, ULONG ulDstNode, ULONG ulDstChId, VOID  *pData, ULONG ulDataLen)
{
	return VOS_OK;
}

VOID  EthLoop_CDP_Callback_pon(ULONG ulFlag, ULONG ulChID, ULONG ulDstNode,	 ULONG ulDstChId, VOID  *pData, ULONG ulDataLen)
{
        LoopInsertCDPMsgHead_t * revBuf = NULL ;
	 SYS_MSG_S *pMsg = NULL;
	 ethloop_port_listnode_t *node;
	 /*LoopCDPMsgHead_t *revBuf2 =NULL;*/
	
	if(ulDstChId != RPU_TID_CDP_ETHLOOP|| ulChID != RPU_TID_CDP_ETHLOOP)
	{
		CDP_FreeMsg(pData);
		return ;
	}

	pMsg = (SYS_MSG_S *)pData ;
	
	switch( ulFlag )
	{
		case CDP_NOTI_FLG_RXDATA: /* 收到数据*/
			if( pData == NULL )
			{
				VOS_ASSERT( 0 );
				return ;
			}
			/* sys_console_printf("\r\nIn Function :  Loop_Poncard_Insert_CDP_Callback\r\n"); */
			switch(SYS_MSG_MSG_CODE(pMsg))
			{
				case LOOP_PONCARD_INSERT :
					revBuf=(LoopInsertCDPMsgHead_t *)(( SYS_MSG_S * )pData + 1);

					chk_config_detect_enable = revBuf->enable;
					if( SYS_LOCAL_MODULE_ISHAVEPP())
					{
						if(!SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
						{
							setL2uCpu(L2U_INDEX_NUM,1);
						}
					}
					/*if(chk_config_detect_enable == FALSE)
						ethLoopCheckTestOamSend( chk_config_eth_vid, 0, 0 );*/
					chk_config_detect_mode = revBuf->mode;
					chk_config_detect_control_enable = revBuf->controlen_able;
					chk_config_detect_mac_clean_enable = revBuf->cleanmac_able;
					chk_config_timer_len = revBuf->interval_time;
					chk_config_up_times = revBuf->uptimes;
					chk_config_up_times_thresh = revBuf->uptime_thre;
					chk_config_eth_vid = revBuf->vlan;
					VOS_MemCpy(chk_config_frame_smac ,revBuf->mac , ETH_LOOP_MAC_ADDR_LEN);
					
					ETHLOOPPORTLISTFLAG = revBuf->EthPortListFlag ;
					VOS_MemCpy(chk_config_eth_portlist[SYS_LOCAL_MODULE_SLOTNO],revBuf->eth_portlist,CHK_SLOT_PORT_MAX+2);
					
					CDP_FreeMsg(pData);
					break;
				case LOOP_NOTIFY_12EPON_ETH_CLEAR:
					node =(ethloop_port_listnode_t *)(( SYS_MSG_S * )pData + 1);
					delEthPortFromLoopList(node->devIdx,node->brdIdx,node->portIdx,node->vid,
						node->loopdevIdx,node->loopbrdIdx,node->loopportIdx,node->loopvid,pMsg->usReserved);
					CDP_FreeMsg(pData);
					break;
				/*case LOOP_MAC_ENTRY_DELETE:
					revBuf2=(LoopCDPMsgHead_t *)(( SYS_MSG_S * )pData + 1);
					if(revBuf2->srcslot != SYS_LOCAL_MODULE_SLOTNO)  此时srcslot 项存的的发送的目的槽位号
					{
						CDP_FreeMsg(pData);
						return VOS_ERROR;
					}

					check_mac_entry_delete_by_pon(revBuf2->srcslot, revBuf2->ponId);
					
					CDP_FreeMsg(pData);
					break;*/
				default:
					ASSERT(0);
					CDP_FreeMsg(pData);
					break;
			}
			break;
		case CDP_NOTI_FLG_SEND_FINISH:/*异步发送时*/
			CDP_FreeMsg(pData);		/*异步发送失败暂不处理，但需要释放消息*/
			break;
		default:
			ASSERT(0);
			CDP_FreeMsg(pData);
			break;
	}

	return ;
}

VOID  EthLoop_CDP_Callback(ULONG ulFlag, ULONG ulChID, ULONG ulDstNode,	 ULONG ulDstChId, VOID  *pData, ULONG ulDataLen)
{
        /*LoopInsertCDPMsgHead_t * revBuf = NULL ;*/
	 SYS_MSG_S *pMsg = NULL;
	 /*LoopCDPMsgHead_t *revBuf2 =NULL;*/
	ethloop_port_listnode_t *node;
	ethloop_listnode_Switch_t *node2;
	
	if(ulDstChId != RPU_TID_CDP_ETHLOOP|| ulChID != RPU_TID_CDP_ETHLOOP)
	{
		CDP_FreeMsg(pData);
		return ;
	}

	pMsg = (SYS_MSG_S *)pData ;
	
	switch( ulFlag )
	{
		case CDP_NOTI_FLG_RXDATA: /* 收到数据*/
			if( pData == NULL )
			{
				VOS_ASSERT( 0 );
				return ;
			}
			switch(SYS_MSG_MSG_CODE(pMsg))
			{
				case LOOP_ONU_REPORT_ENTRY:
					node =(ethloop_port_listnode_t *)(( SYS_MSG_S * )pData + 1);
					addEthPortToLoopList2(node->devIdx,node->brdIdx,node->portIdx,node->vid, node->loopdevIdx,node->loopbrdIdx, 
									node->loopportIdx,node->loopvid, node->OtherOltFlag, node->OltTypeFlag, node->loopmac);
					break;

				case LOOP_12EPON_ETH_LOOP:
					node =(ethloop_port_listnode_t *)(( SYS_MSG_S * )pData + 1);
					addEthPortToLoopList2(node->devIdx,node->brdIdx,node->portIdx,node->vid, node->loopdevIdx,node->loopbrdIdx, 
									node->loopportIdx,node->loopvid, node->OtherOltFlag, node->OltTypeFlag, node->loopmac);
					break;
					
				case LOOP_ONU_CLEAR_ENTRY:
					node =(ethloop_port_listnode_t *)(( SYS_MSG_S * )pData + 1);
					delEthPortFromLoopList(node->devIdx,node->brdIdx,node->portIdx,node->vid,
						node->loopdevIdx,node->loopbrdIdx,node->loopportIdx,node->loopvid,pMsg->usReserved);
					break;

				case LOOP_12EPON_ETH_CLEAR:
					/* 只有主控板上接受linkdown/linkup 的消息处理通知链情景下这个case不应在过来了2012-05-31 */
					node =(ethloop_port_listnode_t *)(( SYS_MSG_S * )pData + 1);
					delEthPortFromLoopList(node->devIdx,node->brdIdx,node->portIdx,node->vid,
						node->loopdevIdx,node->loopbrdIdx,node->loopportIdx,node->loopvid,pMsg->usReserved);
					break;

				case LOOP_ONU_SWITCH_REPORT:
					node2 =(ethloop_listnode_Switch_t *)(( SYS_MSG_S * )pData + 1);
					addEthPortToLoopList_Switch( node2->devIdx, node2->slotIdx, node2->portIdx, node2->SwitchPort, node2->SwitchMac);
					break;

				case LOOP_ONU_SWITCH_CLEAR:
					node2 =(ethloop_listnode_Switch_t *)(( SYS_MSG_S * )pData + 1);
					DelFromLoopListByDelFlag( node2->devIdx, node2->slotIdx, node2->portIdx, node2->SwitchPort, node2->SwitchMac, 3 );
					break;
					
				default:
					ASSERT(0);
					break;
			}
			CDP_FreeMsg(pData);
			break;
		case CDP_NOTI_FLG_SEND_FINISH:/*异步发送时*/
			CDP_FreeMsg(pData);		/*异步发送失败暂不处理，但需要释放消息*/
			break;
		default:
			ASSERT(0);
			CDP_FreeMsg(pData);
			break;
	}

	return ;
}

int Slot_Get_PortNum(int slotno)
{
	int port_num = 0;
	#if 0
	ULONG mtype;
	mtype = __SYS_MODULE_TYPE__(slotno);
	/*typesdb_module_portnum( ULONG mtype )函数会对mtype进行判断*/
	port_num =  typesdb_module_portnum(mtype);
	/*以后尽量用上面的函数来获取port数目，但是现在大部分的port数目和那个函数的不一致，所以暂时不使用这个*/
	#endif
	
	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_GEO)
		port_num = 4;
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_GEM)
		port_num = 4;
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_GET)
		port_num = 4;
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6100_GEM)
		port_num = 4;
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE/* ||
		(__SYS_MODULE_TYPE__(slotno) ==  MODULE_E_GFA8000_GEM_10GE)*/)
	{
		if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_SW ||
		  (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_SW))
		{
			if(slotno == 5 || slotno == 6 || slotno ==9 || slotno == 10 )
				port_num = 5;
			else
				port_num = 4;
		}
		else
			port_num = 5;
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_GE 
		/*|| __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_GEM_GE*/)
		port_num = 4;
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_EPON)
		port_num = 4;
	else if (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6100_EPON)
		port_num = 2;
	else if (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6100_MAIN)
		port_num = 2;
	else if (SYS_MODULE_IS_4EPON_0GE(slotno))
		port_num = 4;
	else if (SYS_MODULE_IS_8EPON(slotno) || SYS_MODULE_IS_4EPON_4GE(slotno))
		port_num = 8;
	else if (SYS_MODULE_IS_12EPON(slotno) || SYS_MODULE_IS_16EPON(slotno))
		port_num = 16;
	else if (SYS_MODULE_IS_8000_GPON(slotno))
		port_num = 16;
	else if ((MODULE_E_GFA8000_8XET == __SYS_MODULE_TYPE__(slotno))||(MODULE_E_GFA8000_8XETA1 == __SYS_MODULE_TYPE__(slotno)))
		port_num = 8;/*add by yanjy2017-2 for 8xeta1*/
	else if (MODULE_E_GFA8000_4XET == __SYS_MODULE_TYPE__(slotno))
		port_num = 4;
	else if (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_10G_8EPON)
		port_num = 8;	
	else if ((__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8100_16EPONB0)||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8100_16GPONB0))
		port_num = 20;
#if defined(_EPON_10G_PMC_SUPPORT_)            
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	else if (SYS_MODULE_IS_6900_10G_EPON(slotno))
		port_num = 6;
#endif
	else 
		port_num = 0;
	
	return port_num;     
}

int Init_Ethloop_PortNum(int slotno )
{
	int j = 0;
	eth_loop_check_port_max[ slotno ] = Slot_Get_PortNum( slotno );
	/*for( j=1; j<=eth_loop_check_port_max[ slotno ]; j++ )
	{
		chk_config_eth_portlist[ slotno ][ j ] = 1;
	}*/
	return VOS_OK;
}

/* 模块初始化 */
int ethLoopCheckInit()
{
	UCHAR ulMask[6]={0xff,0xff,0xff,0xff,0xff,0xff};

    extern PON_address_record_t Mac_addr_table[];
	/*if( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
		return VOS_OK;*/
    pon_mac_addr_table = Mac_addr_table;
	
   	( VOID ) CDP_SYNC_Register( MODULE_LOOPBACK, LOOP_CMD2LIC_RPC_Callback );
	
	if( SYS_LOCAL_MODULE_TYPE_IS_PRODUCT_MANAGER )
	{
		/*CDP_Create( RPU_TID_CDP_LOOP,  CDP_NOTI_VIA_FUNC, 0, LOOP_CDP_Callback) ;*/
		CDP_Create( RPU_TID_CDP_ETHLOOP,  CDP_NOTI_VIA_FUNC, 0, EthLoop_CDP_Callback) ;
	}
	else
	{
		/*CDP_Create( RPU_TID_CDP_LOOP,  CDP_NOTI_VIA_FUNC, 0, LOOP_CDP_Callback_Pon);*/
		CDP_Create( RPU_TID_CDP_ETHLOOP,  CDP_NOTI_VIA_FUNC, 0, EthLoop_CDP_Callback_pon) ;
	}
	/* modified by xieshl 20090107, 6100上不同槽位端口数不等，其上联口是4个，问题单9608 */
	VOS_MemZero(eth_loop_check_port_max, sizeof(eth_loop_check_port_max) );
	if( SYS_PRODUCT_TYPE ==  PRODUCT_E_EPON3 )
	{
		eth_loop_check_slot_max = 8;
		VOS_MemZero(eth_loop_check_port_max, sizeof(eth_loop_check_port_max) );
		eth_loop_check_port_max[1] = 4;
		eth_loop_check_port_max[4] = 4;
		eth_loop_check_port_max[5] = 4;
		eth_loop_check_port_max[6] = 4;
		eth_loop_check_port_max[7] = 4;
		eth_loop_check_port_max[8] = 4;
	}
	else if( SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6100 )
	{
		eth_loop_check_slot_max = 3;
		VOS_MemZero(eth_loop_check_port_max, sizeof(eth_loop_check_port_max) );
		eth_loop_check_port_max[1] = 4;
		eth_loop_check_port_max[2] = 2;
		eth_loop_check_port_max[3] = 2;
	}
	else if( SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6900 || SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000)
	{
		eth_loop_check_slot_max = 14;
		VOS_MemSet(eth_loop_check_port_max, 4, sizeof(eth_loop_check_port_max));   /*modified by duzhk for 12EPON  2010-10-10 */
#if 0
		if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
		{
      			eth_loop_check_port_max[SYS_LOCAL_MODULE_SLOTNO] = Slot_Get_PortNum(SYS_LOCAL_MODULE_SLOTNO);
		}
		else
		{
			/* VOS_MemZero(eth_loop_check_port_max, sizeof(eth_loop_check_port_max) ); */
       		/*VOS_MemSet(eth_loop_check_port_max, 4, sizeof(eth_loop_check_port_max));*/  /*modified by duzhk for 12EPON  2010-10-10 */
			for( i = 1; i <= CHK_SLOT_MAX; i++ )
			{
				eth_loop_check_port_max[ i ] = Slot_Get_PortNum( i );
			}
		}
#endif
	}
	else if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M)
	{
		eth_loop_check_slot_max = 3;
		eth_loop_check_port_max[ 0 ] = 16;
		eth_loop_check_port_max[ 1 ] = 16;
		eth_loop_check_port_max[ 2 ] = 16;
	}
	else if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S)
	{
		eth_loop_check_slot_max = 1;
		eth_loop_check_port_max[ 0 ] = 16;
	}
	else if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA8100)
	{
		eth_loop_check_slot_max = 1;
		eth_loop_check_port_max[ 0 ] = 20;
	}
	else
	{
		eth_loop_check_slot_max = 1;
		eth_loop_check_port_max[1] = 1;
	}
	ethloop_semid = VOS_SemMCreate( VOS_SEM_Q_PRIORITY );
	if( ethloop_semid == 0 )
	{
		sys_console_printf( "initialize EthLoop SemId fail!\r\n" );
		VOS_ASSERT(0);
	}
	VOS_MemZero( chk_result_eth_port, sizeof(chk_result_eth_port) );
	VOS_MemZero( chk_result_eth_port_vid, sizeof(chk_result_eth_port_vid) );
	VOS_MemZero( chk_result_pon_onu, sizeof(chk_result_pon_onu) );
	/*VOS_MemZero( chk_config_frame_smac, sizeof(chk_config_frame_smac) );*/
#ifdef CHK_BRAS_MAC
	VOS_MemZero( chk_bras, sizeof(chk_bras) );
#endif
	chk_config_timer_len = DEF_CHECK_TIMER_INTERVAL;
	chk_config_eth_vid = 0;
#if 0
	ethLoopCheckPortlistAll();
#endif
	/*mn_ethLoopCheckMacDefaultSet();*/

	if( ethLoopChkQueId == 0 ) 
		ethLoopChkQueId = VOS_QueCreate( ETH_LOOP_CHK_QUE_LEN_MAX, VOS_MSG_Q_PRIORITY);
	if( ethLoopChkQueId  == 0 )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
		
	if( ethLoopChkTaskId == NULL )
		ethLoopChkTaskId = ( VOS_HANDLE )VOS_TaskCreate("tEthChk", 250, ethLoopChkTask, NULL );
	if( ethLoopChkTaskId == NULL )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	VOS_QueBindTask( ethLoopChkTaskId, ethLoopChkQueId );

	/*if( ethLoopChkTimerId == VOS_ERROR )
	{
		ethLoopChkTimerId = VOS_TimerCreate( MODULE_LOOPBACK, 0, SEND_CHECK_FRAME_INTERVAL, (void *)ethLoopChkTimerCallback, NULL, VOS_TIMER_LOOP );
		if( ethLoopChkTimerId == VOS_ERROR )
		{
			VOS_ASSERT( 0 );
			return VOS_ERROR;
		}
	}*/

	/*pon_mac_addr_table = VOS_Malloc( sizeof(PON_address_record_t) * PON_ADDRESS_TABLE_SIZE, MODULE_LOOPBACK );
	if( pon_mac_addr_table == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}*/
	/*if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER || SYS_LOCAL_MODULE_TYPE_IS_6900_12EPON)*/
	if( SYS_LOCAL_MODULE_ISHAVEPP() )
	{
		if(SYS_LOCAL_MODULE_TYPE != MODULE_E_GFA8000_10G_8EPON) /*****add by   mengxsh  20140710 *****/ 	
		{
			L2U_INDEX_NUM  = addL2uEntry(chk_config_frame_dmac,ulMask,1,0,0);  /* 为了是设备管理跑起来，而注释掉的。20140515  mengxsheng  sheng */
		}
	}
	/* modified by xieshl 20110829, 独立定义showrun模块 */
       /*ethloop_chk_cli_cmd_install();*/
	ethloop_module_init();

	return VOS_OK;
}

int ethLoopCheckInit2()
{
	if( SYS_MODULE_SLOT_ISHAVECPU( SYS_LOCAL_MODULE_SLOTNO ) )
     	{
     		if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
			ethLoopCheckPortlistDefault();
		else
		{
			if( ETHLOOPPORTLISTFLAG == EthLoopPortlistIsDefault )
				ethLoopCheckPortlistDefault();
		}
     	}
	return VOS_OK;
}

#ifdef CHK_BRAS_MAC
int  ethLoopCheckTestBRASMacProcess();
{
	/*给该函数一个最简单的定义，
	防止万一程序跑到这里发生错误*/
	return VOS_OK;
}
#endif

#define ETHLOOP_PON_BOARD_ENSURE \
	if ( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER ) \
	{ \
		VOS_ASSERT(0); \
		break; \
	}

#define ETHLOOP_MASTER_BOARD_ENSURE \
	if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER ) \
	{ \
		VOS_ASSERT(0); \
		break; \
	}
	
int loop_detect_timer_count = 0;
/* 环路检测任务 */
void ethLoopChkTask()
{
	ULONG ulRcvMsg[4];
	/*LoopCDPMsgHead_t *pMsg = NULL;*/
	/*SYS_MSG_S   *pMsg = NULL;
	ULONG slotno, portno, onuno;*/

	/*while( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
	{
		VOS_TaskDelay(200);
	}*/	/* removed by xieshl 20100114 */
	while( SYS_LOCAL_MODULE_RUNNINGSTATE < MODULE_MOD_REGISTERED )	/* modified by xieshl 20100526, 问题单10143 */
	{
		VOS_TaskDelay(50);
	}

	if( MAC_ADDR_IS_ZERO(chk_config_frame_smac) )
		mn_ethLoopCheckMacDefaultSet();

	VOS_TaskDelay(500);

	if( ethLoopChkTimerId == VOS_ERROR )
	{
		ethLoopChkTimerId = VOS_TimerCreate( MODULE_LOOPBACK, 0, SEND_CHECK_FRAME_INTERVAL, (void *)ethLoopChkTimerCallback, NULL, VOS_TIMER_LOOP );
		if( ethLoopChkTimerId == VOS_ERROR )
		{
			VOS_ASSERT( 0 );
			return ;
		}
	}

	while( 1 )
	{
		if( VOS_QueReceive( ethLoopChkQueId, ulRcvMsg, WAIT_FOREVER ) == VOS_ERROR )
		{
			ASSERT(0);
			VOS_TaskDelay(200);
			continue;
		}
		
		switch( ulRcvMsg[1] )
		{
			case FC_ETHCHK_TIMER:/*因为涉及loop_detect_timer_count 变量的修改，主控和PON 板都执行*/
				loop_detect_timer_count = 1;
				if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONU && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
				{
					loopChkDbgPrintf( 1, ("LPCHK:send test oam start\r\n") );
					ethLoopCheckTestOamSend( chk_config_eth_vid, 0, 0 );
				}
				break;
			case FC_ETHCHK_SEND:/*只在主控上执行*/
				/*ETHLOOP_MASTER_BOARD_ENSURE*/ /*modified by duzhk 2012-04-06 12EPON 上也执行*/
				{
					if( loop_detect_timer_count > 0 )
					{
						loopChkDbgPrintf( 1, ("LPCHK:send test frames start\r\n") );
						ethLoopCheckTestFrameProcess();
					}
				}
				break;
			case FC_ETHCHK_SEARCH_MAC:/*主控和PON板上都执行，分别对各自创建的表项修改定时器*/
				loopChkDbgPrintf( 1, ("LPCHK:check test mac start\r\n") );
				/*ethLoopCheckTestMacProcess();*/
				onuMode_ethLoopCheckAlarmReportProcess();
				break;
			case FC_ETHCHK_PORT_LINKDOWN: /*只在主控上执行，上联口端口LinkDown 引起删除相关表项*/
				/* ETHLOOP_MASTER_BOARD_ENSURE */  /*modified by duzhk 2012-04-06 12EPON 上也执行*/
				{
					ethLoopCheckLinkdownProcess( ulRcvMsg[2], ((ulRcvMsg[3] >> 8) & 0xff), (ulRcvMsg[3] & 0xff) );
					DelFromLoopListByDelFlag( ulRcvMsg[2] , ((ulRcvMsg[3] >> 8) & 0xff), (ulRcvMsg[3] & 0xff), 0,NULL, 2) ;
				}
				break;
			case FC_ETHCHK_ONU_LINEOFF: /*此case , 只在PON 板上执行 */
				ETHLOOP_PON_BOARD_ENSURE
				{
					ethLoopCheckOnuLineOffProcess( ulRcvMsg[2] );
					DelFromLoopListByDelFlag( ulRcvMsg[2] , 0,0,0,NULL, 1 ) ;
				}
				break;
			case FC_ETHCHK_PORT_LOOPCLEAR:/*现在，此case 只在PON 板上发生*/
				ETHLOOP_PON_BOARD_ENSURE
				{
					onuMode_ethLoopCheckOnuPortClearProcess(ulRcvMsg[2], ((ulRcvMsg[3] >> 24) & 0xff), ((ulRcvMsg[3] >> 16) & 0xff), (ulRcvMsg[3] & 0xff));
				}
				break;
			case FC_ETHCHK_COMMAND:
				ethLoopCheckConfigCommandProcess((qdef_ethchk_cmd_t*)ulRcvMsg[3]);
				break;
#ifdef CHK_BRAS_MAC
			case FC_ETHCHK_BRAS_MAC:
				loopChkDbgPrintf( 1, ("LPCHK:check bras mac start\r\n") );
				ethLoopCheckTestBRASMacProcess();
				break;
#endif
			case FC_ETHCHK_RECEIVE :
				recvLoopChkPacketHook((UCHAR *)ulRcvMsg[3], ulRcvMsg[2]);
				break;
			/*case FC_ETHCHK_ONU_PORT_LOOP :
				pMsg = (LoopCDPMsgHead_t *)ulRcvMsg[3];
				eventOamMsg_onuEthPortLoop(pMsg->ponId,pMsg->llId,(eventOnuEthLoopMsg_t *)(pMsg+1));
				VOS_Free(pMsg);
				break;*/
			case  FC_OAM_RECEIVE:
				eventOamMsg_onuEthPortLoop((ulRcvMsg[2]>>16)&0x0000ffff, ulRcvMsg[2]&0x0000ffff, (eventOnuEthLoopMsg_t *)ulRcvMsg[3] );
				VOS_Free((VOID*)ulRcvMsg[3]);
				break;
			default:
				VOS_ASSERT(0);
				/*pMsg = (SYS_MSG_S *)ulRcvMsg[3];
				if( pMsg && (AWMC_CLI_BASE == pMsg->usMsgCode) )
					decode_command_msg_packet( pMsg, VOS_MSG_TYPE_QUE );
				if( pMsg != NULL )
					VOS_Free( pMsg );*/
				break;
		}
	}
}

ULONG Onu_Eth_Loop_Frame_Num_test = 50;

/* 定时器回调函数 */
void ethLoopChkTimerCallback()
{
	static ULONG send_timer_count = 0;
	static ULONG send_frm_timer_count = 0;
	static ULONG chk_timer_count = 0;
#ifdef CHK_BRAS_MAC
	static USHORT chk_bras_timer_count=0;
#endif
	ULONG aulMsg[4] = {MODULE_LOOPBACK, 0, 0, 0};
	/*UCHAR ulMask[6]={0xff,0xff,0xff,0xff,0xff,0xff};*/
	USHORT Send_Flag = 0;

	if( chk_config_detect_enable == FALSE )
		return;
	if( (!SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO)) || SYS_LOCAL_MODULE_ISMASTERSTANDBY )
		return;
	
	chk_timer_count++;
	send_frm_timer_count++;
	send_timer_count++;

#ifdef CHK_BRAS_MAC
	if( chk_BRAS_mac_enable )
		chk_bras_timer_count++;/*add by shixh20091112*/
#endif
	if(Onu_Loop_Flag > 0)
		Onu_Loop_Flag++;
	
	if(Onu_Eth_Loop_Frame_Num > Onu_Eth_Loop_Frame_Num_test && Onu_Loop_Flag == 0)
	{
		/*delL2uForLoopChk(chk_config_frame_dmac);
              addL2uEntry(chk_config_frame_dmac, ulMask, 1,0, 0);*/
              if( SYS_LOCAL_MODULE_ISHAVEPP() && !SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
              	setL2uCpu(L2U_INDEX_NUM,0);
		/*sys_console_printf("\r\nOnu_Eth_Loop_Frame_Num is %d\r\n",Onu_Eth_Loop_Frame_Num);
		sys_console_printf("Delete the L2U For Loop\r\n");*/
		Onu_Loop_Flag = 1;
	}

	Onu_Eth_Loop_Frame_Num = 0;

	if(Onu_Loop_Flag >30)
	{
              /*delL2uForLoopChk( chk_config_frame_dmac );
		addL2uEntry(chk_config_frame_dmac,ulMask,1,1,0);*/
		if( SYS_LOCAL_MODULE_ISHAVEPP() && !SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
			setL2uCpu(L2U_INDEX_NUM,1);
		/*sys_console_printf("\r\nOnu_Loop_Flag is %d\r\n",Onu_Loop_Flag);
		sys_console_printf("Add the L2U For Loop\r\n");*/
		Onu_Loop_Flag = 0;
	}
	
	if( VOS_QueNum(ethLoopChkQueId) < 3/*ETH_LOOP_CHK_QUE_LEN_MAX/2*/ )
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER 
			|| SYS_LOCAL_MODULE_TYPE_IS_SWITCH_PON)/*只有主控才发送测试帧*/
		/* modified by duzhk 2012-04-06 */
		{
				if( send_frm_timer_count >= 5 )
				{
					aulMsg[1] = FC_ETHCHK_SEND;
					send_frm_timer_count = 0;
					Send_Flag = 1;
				}
		}
		
		if(SYS_LOCAL_MODULE_TYPE_IS_PON_MANAGER)
		{
				if( send_timer_count > (chk_config_timer_len - 2) )
				{
					aulMsg[1] = FC_ETHCHK_TIMER;
					send_timer_count = 0;
					Send_Flag = 1;
				}
		}
		
		if(SYS_LOCAL_MODULE_TYPE_IS_PON_MANAGER)
		{
				 if( chk_timer_count >= chk_config_timer_len )
				{
					aulMsg[1] = FC_ETHCHK_SEARCH_MAC; 
					chk_timer_count = 0;
					Send_Flag = 1;
				} 
		}
#ifdef CHK_BRAS_MAC
		 if(chk_bras_timer_count >= chk_config_timer_len+10)
		{
			aulMsg[1] = FC_ETHCHK_BRAS_MAC;
			chk_bras_timer_count = 0;
		}
#endif

		if(Send_Flag == 1)
		{
			if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
			{
				/*VOS_ASSERT(0);*/
			}
		}
	}
	else
	{
		loopChkDbgPrintf( 2, ("LPCHK:ETH loop detection is busy ( send )\r\n") );
	}
}

void ethLoopChkRecvCallback(char *packet, ULONG *swport) /* carry interface index for 12Epon */
{
	ULONG aulMsg[4] = {MODULE_LOOPBACK, 0, 0, 0};
	
	aulMsg[1] = FC_ETHCHK_RECEIVE;
	aulMsg[2] =  (ULONG)(*swport);
	aulMsg[3] = (ULONG)packet;
		
		
	if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		VOS_Free(packet);
		loopChkDbgPrintf( 2, ("LPCHK:ETH loop detection is busy (receive packet)\r\n") );
	}
}
/*直接通知环路告警模块的函数进行处理*/
void ethLoopOamRecvCallback(ushort_t ponId,ushort_t llid , eventOnuEthLoopMsg_t *pEthLoopMsg)
{
	ULONG aulMsg[4] = {MODULE_LOOPBACK, 0, 0, 0};
	
	aulMsg[1] = FC_OAM_RECEIVE;
	aulMsg[2] = (ponId<<16)|llid;
	aulMsg[3] = (ULONG)pEthLoopMsg;
		
		
	if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		VOS_Free(pEthLoopMsg);
	}
}

/* 查询检测端口数 */
int ethLoopCheckPortlistNum()
{
	int i,j;
	int count = 0;
	for( i=1; i<=eth_loop_check_slot_max; i++ )
	{
		for( j=1; j<=eth_loop_check_port_max[i]; j++ )
		{
			if( chk_config_eth_portlist[i][j] );
				count++;
		}
	}
	return count;
}
/* 设置环路检测默认端口列表，即所有下行端口 */
VOID ethLoopCheckPortlistDefault()
{
	int i,j;
	VOS_MemZero( chk_config_eth_portlist, sizeof(chk_config_eth_portlist) );
	for( i=1; i<=eth_loop_check_slot_max; i++ )
	{
		/*if( check_slotno_is_illegal(i) )
			continue;*/
		
		for( j=1; j<=eth_loop_check_port_max[i]; j++ )
		{
			if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
			{
#if 0
				if(SYS_LOCAL_MODULE_TYPE == MODULE_E_GFA6900_SW)
				{
				       if(SYS_MODULE_IS_6900_EPON(i))
                                    {
					       chk_config_eth_portlist[i][j] = 1;
                                    }
				}
				else
                            {
                                    if(1!= i)
                                        chk_config_eth_portlist[i][j] = 1;
                             }
#else
				if( CHECK_SLOTNO_IS_PON(i) )
					chk_config_eth_portlist[i][j] = 1;
				else
					chk_config_eth_portlist[i][j] = 0;
#endif
			}
			else
			{
				if(SYS_LOCAL_MODULE_SLOTNO == i /*|| CHECK_SLOTNO_IS_ETH(i)*/)/*只包含该PON 卡上的口*/
					chk_config_eth_portlist[i][j] = 1;
			}
			
		}
	}
	ETHLOOPPORTLISTFLAG = EthLoopPortlistIsDefault;
	VOS_MemZero(ethloop_devsm_pull_flag, sizeof(ethloop_devsm_pull_flag));
}

VOID ethLoopCheckPortlistAll()
{
	int i,j;
	VOS_MemZero( chk_config_eth_portlist, sizeof(chk_config_eth_portlist) );
	for( i=1; i<=eth_loop_check_slot_max; i++ )
	{
		if( __SYS_MODULE_TYPE__( i ) == MODULE_TYPE_NULL  || __SYS_MODULE_TYPE__( i ) == MODULE_TYPE_UNKNOW )
			eth_loop_check_port_max[ i ] = 4;
		else
			eth_loop_check_port_max[ i ] = Slot_Get_PortNum( i );

		if( eth_loop_check_port_max[ i ] < 5 && chk_config_eth_portlist[ i ][5] == 1 )
			chk_config_eth_portlist[ i ][5] = 0;
		
		for( j=1; j<=eth_loop_check_port_max[i]; j++ )
		{
			if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
			{
				if(SYS_LOCAL_MODULE_SLOTNO == i /*|| CHECK_SLOTNO_IS_ETH(i)*/)/*包含上联口和该PON 卡上的口*/
					chk_config_eth_portlist[i][j] = 1;
			}
			else
			    chk_config_eth_portlist[i][j] = 1;
		}
	}
	ETHLOOPPORTLISTFLAG = EthLoopPortlistIsAll;
	VOS_MemZero(ethloop_devsm_pull_flag, sizeof(ethloop_devsm_pull_flag));
}

BOOL eth_loopcheck_portlist_is_default()
{
	/*int i,j;
	for( i=2; i<=eth_loop_check_slot_max; i++ )
	{

		for( j=1; j<=eth_loop_check_port_max[i]; j++ )
		{
			if( chk_config_eth_portlist[i][j] == 0 )
			{
				return FALSE;
			}
		}
	}*/
	if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsDefault)
        	return TRUE;
       else
              return FALSE;
}

BOOL eth_loopcheck_portlist_is_all()
{
	if(ETHLOOPPORTLISTFLAG  == EthLoopPortlistIsAll)
		return TRUE;
	else
		return FALSE;
	/*
	int i,j;
    
       if(EthLoopPortlistIsDefaultFlag == 1)
               return FALSE;
       
       for( i=1; i<=eth_loop_check_slot_max; i++ )
	{
		for( j=1; j<=eth_loop_check_port_max[i]; j++ )
		{
			if( chk_config_eth_portlist[i][j] == 0 )
			{
				return FALSE;
			}
		}
	}
	return TRUE;*/
}

BOOL eth_loopcheck_portlist_include_uplink()
{
	/*int i=1,j;
	if( check_slotno_is_illegal(i) )
		return FALSE;*//*这种是针对6700 OLT 的，该使用什么宏来限制??????*/
	int i = 0, j = 0;

	if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsDefault)/*默认不开启uplink loopdetection*/
		return FALSE;
	
	for(i = 1; i <= eth_loop_check_slot_max ; i++)
	{
		if(CHECK_SLOTNO_IS_ETH(i) /*&& eth_devsm_pull_flag[i][0] == eth_devsm_normal */)
		{
			for( j=1; j<=eth_loop_check_port_max[i]; j++ )
			{
				if( chk_config_eth_portlist[i][j] == 1 )
				{
					return TRUE;
				}
			}
		}
	}

	if(SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON)
	{
		for( j=0; j <= 4; j++ )
		{
			if ( chk_config_eth_portlist[SYS_LOCAL_MODULE_SLOTNO][j] == 1 )
				return TRUE;
		}
	}
	
	return FALSE;

}

VOID ethLoopCheckPortlistClear()
{
       ETHLOOPPORTLISTFLAG = EthLoopPortlistIsDesigned;
 	VOS_MemZero( chk_config_eth_portlist, sizeof(chk_config_eth_portlist) );
	VOS_MemZero(ethloop_devsm_pull_flag, sizeof(ethloop_devsm_pull_flag));
 }
#if 0
VOID ethLoopCheckAlarmClear()
{
	ULONG slotno, portno, onuno;
	USHORT vid;

	for( slotno=1; slotno<=eth_loop_check_slot_max; slotno++ )
	{
		for( portno=1; portno<=eth_loop_check_port_max[slotno]; portno++ )
		{
			vid = chk_result_eth_port_vid[slotno][portno];
			
			if( CHECK_SLOTNO_IS_ETH(slotno) )
			{
				if( loop_port_alarm_get(slotno, portno) == CHK_RESULT_LOOP_ALARM )
				{
					loop_port_alarm_cls(slotno, portno);
					loopChkDbgPrintf( 2, ("LPCHK:report eth%d/%d loop clear\r\n", slotno, portno) );
					ethLoopAlarmClear_EventReport( 1, slotno, portno );
					chk_result_eth_port_vid[slotno][portno] = 0;
				}
				loop_port_status_cls(slotno, portno);
			}
			else if( CHECK_SLOTNO_IS_PON(slotno) )
			{
				for( onuno=1; onuno<=CHK_PON_ONU_MAX; onuno++ )
				{
					if( loop_onu_alarm_get(slotno, portno, onuno) == CHK_RESULT_LOOP_ALARM )
					{
						loop_onu_alarm_cls(slotno, portno, onuno);
						loopChkDbgPrintf( 2, ("LPCHK:report onu%d%d/%d loop clear\r\n", slotno, portno, onuno) );
						onuLoopAlarmClear_EventReport( MAKEDEVID(slotno,portno,onuno)/*slotno * 10000 + portno * 1000 + onuno*/ );
					}
					loop_onu_status_cls(slotno, portno, onuno);
				}
				/*if( loop_port_alarm_get(slotno, portno) != CHK_RESULT_LOOP_NULL )
				{
					sys_console_printf( "PON port %d/%d is looping\r\n", slotno, portno );
				}*/
			}
		}
	}
}
#endif

/* 设置环路检测开始或禁止 */
int ethLoopCheckNotifyEvent( ULONG event )
{
	/*if( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
		return VOS_ERROR;*/	/* removed by xieshl 20100114 */

	if( event == ETH_LOOP_CHECK_START )
	{
		if( (chk_config_timer_len < MIN_CHECK_TIMER_INTERVAL) || (chk_config_timer_len > MAX_CHECK_TIMER_INTERVAL) )
		{
			chk_config_timer_len = DEF_CHECK_TIMER_INTERVAL;
		}
		VOS_MemZero( chk_result_eth_port, sizeof(chk_result_eth_port) );
		VOS_MemZero( chk_result_eth_port_vid, sizeof(chk_result_eth_port_vid) );
		VOS_MemZero( chk_result_pon_onu, sizeof(chk_result_pon_onu) );
		if( ethLoopCheckPortlistNum() == 0 )
			eth_loopcheck_portlist_is_default();
		if( MAC_ADDR_IS_ZERO(chk_config_frame_smac) )
			mn_ethLoopCheckMacDefaultSet();
		/*if( ethLoopChkTimerId == VOS_ERROR )
		{
			ethLoopChkTimerId = VOS_TimerCreate( MODULE_LOOPBACK, 0, 1000, (void *)ethLoopChkTimerCallback, NULL, VOS_TIMER_LOOP );
			if( ethLoopChkTimerId == VOS_ERROR )
			{
				VOS_ASSERT( 0 );
				return VOS_ERROR;
			}
		}*/
		
		chk_config_detect_enable = TRUE;
	    setOnuEthLoopEnable(TRUE);
		return VOS_OK;
	}
	else if( event == ETH_LOOP_CHECK_STOP )
	{
		chk_config_detect_enable = FALSE;
        setOnuEthLoopEnable(FALSE);
		/*
		ethLoopCheckAlarmClear();
		暂时不考虑ONLYOLT 的情况，modefied by duzhk 2011.1.17
		*/
		/*chk_config_timer_len = DEF_CHECK_TIMER_INTERVAL;
		ethLoopCheckPortlistDefault();
		chk_config_eth_vid = 0;*/

		/*if( ethLoopChkTimerId != VOS_ERROR )
		{
			VOS_TimerDelete( MODULE_LOOPBACK, ethLoopChkTimerId );
		}
		ethLoopChkTimerId = VOS_ERROR;*/
		return VOS_OK;
	}
	return VOS_ERROR;		
}


/* ONU模式(OLT和ONU共同发送测试帧和MAC检查) 环路告警上报处理 */
/* added by xieshl 20080627, 解决告警恢复问题, 问题单#6771 */
VOID onuMode_ethLoopCheckAlarmReportProcess()
{
	ULONG devIdx, brdIdx, portIdx;
	ULONG ponBrd, ponPort;
	ULONG ponBrdBak = 0, ponPortBak = 0 , unit = 1;
	ethloop_port_listnode_t *node;
	ethloop_port_listnode_t *node_prev;
	
	uchar vendor_id[4] = {0};
	short int OnuIdx = 0;
	short int PortIdx = 0;
	short int SlotIdx = 0;
	short int PonPortIdx = 0;

	/*if( chk_config_detect_enable != TRUE )
		return ;*/
	if(CHK_SLOT_MAX > 32 || CHK_SLOT_PORT_MAX > 32)
	{
		VOS_ASSERT(0);
		return ;
	}
	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	node_prev = eth_port_loop_list;
	while( node )
	{
		devIdx = node->devIdx;
		brdIdx = node->brdIdx;
		portIdx = node->portIdx;

		if( SYS_LOCAL_MODULE_TYPE_IS_PRODUCT_MANAGER )
		{
			if( devIdx == 1 && SYS_MODULE_IS_UPLINK_PON( brdIdx ) && portIdx <= 4)
			{
				node_prev = node;
				node = node->next;
				continue;
			}
			
			if( (__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW || 
				 (__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW)) && devIdx != 1 )
			{
				node_prev = node;
				node = node->next;
				continue;
			}

			if( (__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_12EPON_M)
				&& (devIdx != 1) && (GET_PONSLOT(devIdx) != SYS_LOCAL_MODULE_SLOTNO))
			{
				node_prev = node;
				node = node->next;
				continue;
			}
		}

		if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER && SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
		{
			if( SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON )
			{
				if( devIdx == 1 && portIdx > 4 )
				{
					node_prev = node;
					node = node->next;
					continue;
				}
			}
			else
			{
				if( devIdx == 1 )
				{
					node_prev = node;
					node = node->next;
					continue;
				}
			}
		}
		
		unit = 1;
		
		if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && devIdx != 1 )
		{
			ponBrd = GET_PONSLOT(devIdx)/*devIdx/10000*/;
			ponPort = GET_PONPORT(devIdx)/*(devIdx % 10000) /1000*/;
			if( (ponBrd <= CHK_SLOT_MAX) && (ponPort <= CHK_SLOT_PORT_MAX) )
			{
				if( (((unit << ponBrd) & ponBrdBak) == 0) || (((unit << ponPort) & ponPortBak) == 0) )
				{
					check_mac_entry_delete_by_pon( ponBrd, ponPort);	/*问题单10181*/
					ponBrdBak |= (unit<<ponBrd);
					ponPortBak |= (unit<<ponPort);
				}
			}
		}
	OnuIdx = GET_ONUID(devIdx) - 1;
	PortIdx = GET_PONPORT(devIdx);
	SlotIdx = GET_PONSLOT(devIdx);
      PonPortIdx = GetPonPortIdxBySlot(SlotIdx, PortIdx);
	
	VOS_MemCpy( vendor_id, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].device_vendor_id, 4);
	if(VOS_MemCmp( vendor_id, CTC_ONU_VENDORID, 2 ) == 0)/*GW  ONU*/
	{

		if( node->ageTime <= 0 )
		{	
			if( node == eth_port_loop_list )
			{
				eth_port_loop_list = node->next;
				node_prev = NULL;
			}
			else
			{
				if( node_prev == NULL )
					node_prev = eth_port_loop_list;
				else
					node_prev->next = node->next;
			}

			LOOP_CHK_DEBUG(1,(LOG_LOOP_CHK,LOG_WARNING,"olt eth-port %d/%d loop clear in vlan %d\r\n",
			node->brdIdx, node->portIdx, node->vid));
			
			if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
			{
				if( (SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON) && ( devIdx == 1 && portIdx <= 4 )  )
				{
					/* 只有主控板上接受linkdown/linkup 的消息处理通知链情景下这个case不应在过来了2012-05-31 */
					/*VOS_ASSERT( 0 );*/
					onuEthPortLoopReportToMaster(LOOP_12EPON_ETH_CLEAR, node, ClearTheEntry);
				}
				else
					onuEthPortLoopReportToMaster(LOOP_ONU_CLEAR_ENTRY, node, ClearTheEntry);
			}

			/*12EPON 上的上联口的告警现在先由主控删除时来上报告警 kkkkkkkk */
			
			if( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && findEthPortFromLoopListByPort(devIdx, brdIdx, portIdx) == NULL)
			{
				ethLoopAlarmClear_EventReport( devIdx, brdIdx, portIdx );
			}
			VOS_Free( node );
			
			if( node_prev == NULL )
				node = eth_port_loop_list;
			else
				node = node_prev->next;

		}	
		else
		{
			/*if( node->ageTime == ETH_LOOP_REPORT_TIME )
			{
				if( findEthPortFromLoopListByPort(node->devIdx, node->brdIdx, node->portIdx) == NULL )
					ethLoopAlarm_EventReport( node->devIdx, node->brdIdx, node->portIdx );
			}*/
			node->ageTime--;
			node_prev = node;
			node = node->next;
		}
	}
	else
	{
		node_prev = node;
		node = node->next;
	}

    }
	VOS_SemGive( ethloop_semid );
}

/* OLT模式(完全由OLT发送测试帧和MAC检查) 环路告警上报处理 */
/*VOID oltMode_ethLoopCheckAlarmReportProcess()
{
	ULONG slotno, portno, onuno;
	USHORT vid;

	for( slotno=1; slotno<=eth_loop_check_slot_max; slotno++ )
	{
		for( portno=1; portno<=eth_loop_check_port_max[slotno]; portno++ )
		{
			vid = chk_result_eth_port_vid[slotno][portno];
			
			if( CHECK_SLOTNO_IS_ETH(slotno) )
			{
				
				if( loop_port_status_get(slotno, portno) == CHK_RESULT_LOOP_STATUS )
				{
					if( loop_port_alarm_get(slotno, portno) == CHK_RESULT_LOOP_NULL )
					{
						loop_port_alarm_set(slotno, portno);
						loopChkDbgPrintf( 2, ("LPCHK:report eth%d/%d loop alarm\r\n", slotno, portno) );

						if( chk_config_detect_control_enable == TRUE )
						{
							
						}

						ethLoopAlarm_EventReport( 1, slotno, portno );
					}
				}
				else
				{
					if( loop_port_alarm_get(slotno, portno) == CHK_RESULT_LOOP_ALARM )
					{
						loop_port_alarm_cls(slotno, portno);
						loopChkDbgPrintf( 2, ("LPCHK:report eth%d/%d loop clear\r\n", slotno, portno) );
						ethLoopAlarmClear_EventReport( 1, slotno, portno );
						chk_result_eth_port_vid[slotno][portno] = 0;
					}
					
				}
			}
			else if( SYS_MODULE_IS_PON(slotno) )
			{
				for( onuno=1; onuno<=CHK_PON_ONU_MAX; onuno++ )
				{
					if( loop_onu_status_get(slotno, portno, onuno) == CHK_RESULT_LOOP_STATUS )
					{
						if( loop_onu_alarm_get(slotno, portno, onuno) == CHK_RESULT_LOOP_NULL )
						{
							loop_onu_alarm_set(slotno, portno, onuno);
							loopChkDbgPrintf( 2, ("LPCHK:report onu%d%d/%d loop alarm\r\n", slotno, portno, onuno) );

							if( chk_config_detect_control_enable == TRUE )
							{
								
							}

							onuLoopAlarm_EventReport( slotno * 10000 + portno * 1000 + onuno );
						}
					}
					else
					{
						if( loop_onu_alarm_get(slotno, portno, onuno) == CHK_RESULT_LOOP_ALARM )
						{
							loop_onu_alarm_cls(slotno, portno, onuno);
							loopChkDbgPrintf( 2, ("LPCHK:report onu%d%d/%d loop clear\r\n", slotno, portno, onuno) );
							onuLoopAlarmClear_EventReport( slotno * 10000 + portno * 1000 + onuno );

							if( chk_config_detect_control_enable == TRUE )
							{
								
							}
						}
					}
				}
				
			}
		}
	}
}
*/
	
int one_time_detect_vlan_count = 30;
int chk_eth_loop_count = 400;

VOID ethLoopCheckTestFrameProcess()
{
	USHORT chk_vid = chk_vid_bak;
	USHORT next_vid;
	int loop_count = 0;
	int chk_loop_vlan_count = 0;
	
	UCHAR all_portlist[MN_SNMP_MAX_PORTLIST_LEN];
	UCHAR untag_portlist[MN_SNMP_MAX_PORTLIST_LEN];

	ULONG  ulIfindex, ulTagged, trunkindex ;
	ULONG ulSlot,ulPort;
	USHORT vid,UntagVid=0;
	EthLoopVlan_t *EthLoopVlan_Temp = EthLoopVlan_List,*EthLoopVlan_Temp2 ;
	ulong_t port_status = 2;
	
	VOS_MemZero( untag_portlist, sizeof(untag_portlist) );
	VOS_MemZero( all_portlist, sizeof(all_portlist) );
	eth_loop_chk_tx_pkts_ctrl = 0;

	/* 在启动OLT环路检测时，发送测试帧 */
	if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY )
	{
		VOS_ASSERT(0);    
		return;			 /*6900上暂时不支持ONLY OLT 模式，duzhk 2011-01-20*/ 
		
		if( chk_config_eth_vid == 0 )
		{
			for( loop_count=0; loop_count < one_time_detect_vlan_count; loop_count++ )
			{
				chk_loop_vlan_count++;
				if(chk_loop_vlan_count > chk_eth_loop_count)
				{
					break;
				}
					
				/* 取下一个VLAN ID，不管端口是否在该VLAN，均发送 */
				if( check_vlan_entry_getnext_by_switch( chk_vid, &next_vid, &all_portlist[0], &untag_portlist[0] ) == VOS_OK )
				{
					ethLoopCheckTestFrameSend( next_vid, all_portlist, untag_portlist );
					chk_vid = next_vid;
				}
				else
				{
					break;
				}
			}
			if( loop_count < one_time_detect_vlan_count )
			{
				loop_detect_timer_count--;
				chk_vid_bak = 0;
			}
			else
				chk_vid_bak = chk_vid;
		}
		else		
		{
			/*chk_vid = chk_config_eth_vid;*/
			/* modified by xieshl 20080519, #6679 */
			if( check_vlan_entry_get_by_switch(chk_config_eth_vid, &all_portlist[0], &untag_portlist[0] ) == VOS_OK )
				ethLoopCheckTestFrameSend( chk_config_eth_vid, all_portlist, untag_portlist );

			loop_detect_timer_count--;
		}
	}
	else		/*ETH_LOOP_CHECK_MODE_OLT_ONU*/
	{
		/*ethLoopCheckTestOamSend( chk_config_eth_vid, all_portlist, untag_portlist );*/	/* removed by xieshl 20090807 */
		if( eth_loopcheck_portlist_include_uplink() )
		{
			if( chk_config_eth_vid == 0 )
			{
				Flag_Vlan_Have_Valid_Port = 0;
				
				for( loop_count=0; loop_count < one_time_detect_vlan_count;/* loop_count++ */)
				{
					chk_loop_vlan_count++;
					if(chk_loop_vlan_count > chk_eth_loop_count)
					{
						break;
					}
					
					/* 取下一个VLAN ID，不管端口是否在该VLAN，均发送 */
					if( check_vlan_entry_getnext_by_switch( chk_vid, &next_vid, &all_portlist[0], &untag_portlist[0] ) == VOS_OK )
					{
						/*if( (all_portlist[1] != 0) || (untag_portlist[1] != 0) )*/	/* modified by xieshl 20090303 只检查上联口 */
						/*{*/
						/*
							VOS_MemZero( &all_portlist[2], (VLAN_SLOT_MAX-3)*sizeof(ULONG));
							VOS_MemZero( &untag_portlist[2], (VLAN_SLOT_MAX-3)*sizeof(ULONG));*/
							IFM_PORTONVLANSTART( trunkindex, next_vid)
							{
								if(!SYS_IS_TRUNK_IF(trunkindex)) 
									 continue;
								
							      ulTagged = VOS_ERROR;
				 
							      IFM_VlanPortRelationApi(trunkindex, next_vid, &ulTagged);

							      if(ulTagged == VLAN_PORT_UNTAGGED)
							      {
									vid = 1;
                                   				UntagVid = next_vid;
								}
								else
								{
									vid = next_vid;
								}
								
								IFM_PORT_ON_TRUNK_START( trunkindex, ulIfindex)
							      {
									if(!SYS_IS_ETH_IF(ulIfindex))
									{
										continue;
									}  
#ifdef _EPON_12EPON_SUPPORT_
									ulSlot = IFM_ETH_GET_SLOT( ulIfindex );
									ulPort = IFM_ETH_GET_PORT( ulIfindex );
#else
									if(VOS_OK != swport_2_slot_port( IFM_ETH_GET_PORT( ulIfindex ) , &ulSlot, &ulPort ))
									{
										continue;
									}
#endif
									if( SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER )
									{
										if( ulSlot != SYS_LOCAL_MODULE_SLOTNO )
										continue;
									}

									if( getEthPortOperStatus(1, ulSlot, ulPort, &port_status) == VOS_ERROR )
									{
										continue;
									}

									if( port_status == 1 )
										check_eth_frame_send_by_port(ulSlot,ulPort, vid,next_vid);  
							      }
							      IFM_PORT_ON_TRUNK_END
							}
							IFM_PORTONVLANEND
							
							ethLoopCheckTestFrameSend( next_vid, all_portlist, untag_portlist );
						
						/*}*/
						chk_vid = next_vid;
						
						if(Flag_Vlan_Have_Valid_Port > 0)
							loop_count++;

						Flag_Vlan_Have_Valid_Port = 0;
						
					}
					else
					{
						break;
					}
				}
				
				if( loop_count < one_time_detect_vlan_count )
				{
					loop_detect_timer_count--;
					chk_vid_bak = 0;
				}
				else
					chk_vid_bak = chk_vid;
			}
			else		
			{
				loop_count = 0;
				Flag_Vlan_Have_Valid_Port = 0;
				
				if(chk_vid != 0)
				{
					EthLoopVlan_Temp = FindEthLoopVlanEntry(chk_vid ,  &EthLoopVlan_Temp2);
					if(EthLoopVlan_Temp == NULL)
					{
						if(loopChkDebugSwitch== 3)
							sys_console_printf("You have delete the vlan %d ( for loop detection )\r\n",chk_vid);
						EthLoopVlan_Temp = EthLoopVlan_List;
					}
				}
				
				while(EthLoopVlan_Temp != NULL && loop_count < one_time_detect_vlan_count)
				{
					chk_loop_vlan_count++;
					if(chk_loop_vlan_count > chk_eth_loop_count)
					{
						break;
					}
					
					if( check_vlan_entry_get_by_switch(EthLoopVlan_Temp->vlanid , &all_portlist[0], &untag_portlist[0] ) == VOS_OK )
					{	
						IFM_PORTONVLANSTART( trunkindex, EthLoopVlan_Temp->vlanid)
						{
							if(!SYS_IS_TRUNK_IF(trunkindex)) 
								 continue;
							
						      ulTagged = VOS_ERROR;
			 
						      IFM_VlanPortRelationApi(trunkindex, EthLoopVlan_Temp->vlanid, &ulTagged);

						      if(ulTagged == VLAN_PORT_UNTAGGED)
						      {
								vid = 1;
                               				UntagVid = EthLoopVlan_Temp->vlanid;
							}
							else
							{
								vid = EthLoopVlan_Temp->vlanid;
							}
							
							IFM_PORT_ON_TRUNK_START( trunkindex, ulIfindex)
						      {
								if(!SYS_IS_ETH_IF(ulIfindex))
								{
									continue;
								}  
#ifdef _EPON_12EPON_SUPPORT_
								ulSlot = IFM_ETH_GET_SLOT( ulIfindex );
								ulPort = IFM_ETH_GET_PORT( ulIfindex );
#else
								if(VOS_OK != swport_2_slot_port( IFM_ETH_GET_PORT( ulIfindex ) , &ulSlot, &ulPort ))
								{
									continue;
								}
#endif
								if( SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER )
								{
									if( ulSlot != SYS_LOCAL_MODULE_SLOTNO )
									continue;
								}

								if( getEthPortOperStatus(1, ulSlot, ulPort, &port_status) == VOS_ERROR )
								{
									continue;
								}

								if( port_status == 1 )
									check_eth_frame_send_by_port(ulSlot,ulPort, vid,EthLoopVlan_Temp->vlanid);  
						      }
						      IFM_PORT_ON_TRUNK_END
						}
						IFM_PORTONVLANEND
								
						if(VOS_NO == VOS_IsMemZero(all_portlist, Mib_PortList_Len))
						{
							ethLoopCheckTestFrameSend( EthLoopVlan_Temp->vlanid, all_portlist, untag_portlist );
							if(Flag_Vlan_Have_Valid_Port >0)
								loop_count++;

							Flag_Vlan_Have_Valid_Port = 0;
						}
					}
					EthLoopVlan_Temp = EthLoopVlan_Temp->next ;
					
					if(EthLoopVlan_Temp != NULL)
						chk_vid = EthLoopVlan_Temp->vlanid;
				}
				if( loop_count < one_time_detect_vlan_count || EthLoopVlan_Temp == NULL)
				{
					loop_detect_timer_count--;
					chk_vid_bak = 0;
				}
				else
					chk_vid_bak = chk_vid;
			}		
		}
		else
			loop_detect_timer_count--;
	}
}

/* 端口linkdown时环路告警自动恢复处理 */
/*static VOID oltMode_ethLoopCheckLinkdownProcess( ULONG slotno, ULONG portno, ULONG onuno )
{
	if( (slotno ==0) || (slotno > eth_loop_check_slot_max) ||
		(portno == 0) || (portno > eth_loop_check_port_max[slotno]) )
		return;
	
	if( CHECK_SLOTNO_IS_ETH(slotno) )
	{
		if( loop_port_alarm_get(slotno, portno) == CHK_RESULT_LOOP_ALARM )
		{
			ethLoopAlarmClear_EventReport( 1, slotno, portno );
			loopChkDbgPrintf( 2, ("LPCHK:cancel eth%d/%d loop alarm\r\n", slotno, portno) );
		}
	}
	else if( SYS_MODULE_IS_PON(slotno) )
	{
		for( onuno=1; onuno<=CHK_PON_ONU_MAX; onuno++ )
		{
			if( loop_onu_alarm_get(slotno, portno, onuno) == CHK_RESULT_LOOP_ALARM )
			{
				loopChkDbgPrintf( 2, ("LPCHK:cancel onu%d%d/%d loop alarm\r\n", slotno, portno, onuno) );
				onuLoopAlarmClear_EventReport( slotno * 10000 + portno * 1000 + onuno );
			}
			chk_result_pon_onu[slotno][portno][onuno] = 0;
		}
	}
	chk_result_eth_port_vid[slotno][portno] = 0;
	chk_result_eth_port[slotno][portno] = 0;
}*/

static VOID onuMode_ethLoopCheckLinkdownProcess( ULONG devIdx, ULONG brdIdx, ULONG portIdx )
{
	delEthPortFromLoopListByPort( devIdx, brdIdx, portIdx );
}

VOID ethLoopCheckLinkdownProcess( ULONG devIdx, ULONG brdIdx, ULONG portIdx )
{
	/*if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY  )
	{
		if( devIdx == 1 )
		{
			oltMode_ethLoopCheckLinkdownProcess( brdIdx, portIdx, 0 );
		}
		else
		{
			ULONG slotno = devIdx / 10000;
			ULONG portno = (devIdx % 10000) / 1000;
			ULONG onuno = devIdx % 1000;
			oltMode_ethLoopCheckLinkdownProcess( slotno, portno, onuno );
		}
	}
	else
	{*/
		onuMode_ethLoopCheckLinkdownProcess( devIdx, brdIdx, portIdx );
	/*}*/
}

VOID ethLoopCheckOnuLineOffProcess( ULONG devIdx )
{
	if( (chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY) || (devIdx == 1) )
		return;

	delEthPortFromLoopListByOnu( devIdx );
}

/* ONU上报环路清除告警不能直接上报，按老化处理，以防止误报 */
int onuMode_ethLoopCheckOnuPortClearProcess(ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG vid)
{
	int delFlag = 0;
	ethloop_port_listnode_t *node;
	ethloop_port_listnode_t *node_prev;
	uchar vendor_id[4] = {0};
 	short int OnuIdx = 0;
	short int PortIdx = 0;
	short int SlotIdx = 0;
	short int PonPortIdx = 0;

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	node_prev = eth_port_loop_list;
	while( node )
	{
		if( (node->devIdx == devIdx) && (node->brdIdx == brdIdx) && (node->portIdx == portIdx) &&
			(node->vid == vid) )
		{
			OnuIdx = GET_ONUID(devIdx) - 1;
			PortIdx = GET_PONPORT(devIdx);
			SlotIdx = GET_PONSLOT(devIdx);
    		       PonPortIdx = GetPonPortIdxBySlot(SlotIdx, PortIdx);
			VOS_MemCpy( vendor_id, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].device_vendor_id, 4);
			if(VOS_MemCmp( vendor_id, CTC_ONU_VENDORID, 2 ) == 0)/*GW  ONU*/
			{
			if( node->ageTime > 1 )	/* modified by xieshl 20081017, 不直接删除，按老化处理 */
				node->ageTime--;
			else
			{
				if( node == eth_port_loop_list )
				{
					eth_port_loop_list = node->next;
				}
				else
				{
					if( node_prev == NULL )
					{
						VOS_ASSERT(0);
						VOS_SemGive( ethloop_semid );
						return VOS_ERROR;
					}
					node_prev->next = node->next;
				}
				delFlag = 1;

				LOOP_CHK_DEBUG(1,(LOG_LOOP_CHK,LOG_WARNING,"onu %d/%d/%d eth-port %d/%d loop clear in vlan %d\r\n",
				GET_PONSLOT(node->devIdx), GET_PONPORT(node->devIdx), GET_ONUID(node->devIdx), node->brdIdx, node->portIdx,node->vid));

				if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
					onuEthPortLoopReportToMaster(LOOP_ONU_CLEAR_ENTRY,node, ClearTheEntry );
								VOS_Free( node );
				}
			}
			else
			{
				if( node == eth_port_loop_list )
				{
					eth_port_loop_list = node->next;
				}
				else
				{
					if( node_prev == NULL )
					{
						VOS_ASSERT(0);
						VOS_SemGive( ethloop_semid );
						return VOS_ERROR;
					}
					node_prev->next = node->next;
				}
				delFlag = 1;
			
				if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
					onuEthPortLoopReportToMaster(LOOP_ONU_CLEAR_ENTRY,node, ClearTheEntry );
				VOS_Free( node );
			}
			break;
		}
		node_prev = node;
		node = node->next;
	}
	VOS_SemGive( ethloop_semid );
	
	if( delFlag && SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		if( findEthPortFromLoopListByPort(devIdx, brdIdx, portIdx) == NULL )
		{
			ethLoopAlarmClear_EventReport( devIdx, brdIdx, portIdx );
		}
	}

	return VOS_OK;
}

/*#define __test_boardcast*/	/* added by xieshl 20080422, 按端口列表的方式发送会导致底层驱动异常，
						     仍改成按单端口发送 */
/* 测试帧基于VLAN发送 */
/*#ifdef __test_boardcast*/

VOID ethLoopCheckTestFrameSend( USHORT send_vid, UCHAR *p_all_portlist, UCHAR *p_untag_portlist )
{
	ULONG send_slotno, send_portno;
	ULONG next_slotno, next_portno;
	ULONG port_status = 2;
	/*ULONG port_bit;*/
	USHORT vid,k,i,UntagVid=0;
       
	CHAR  pcportlist[MN_SNMP_MAX_PORTLIST_LEN],pcportlisttemp[MN_SNMP_MAX_PORTLIST_LEN],puntaglist[MN_SNMP_MAX_PORTLIST_LEN];
	
	VOS_MemZero(pcportlist,  Mib_PortList_Len);
	VOS_MemZero(puntaglist,  Mib_PortList_Len);
	
	VOS_MemCpy( pcportlist, p_all_portlist, Mib_PortList_Len);
	VOS_MemCpy( puntaglist, p_untag_portlist, Mib_PortList_Len);

	loopChkDbgPrintf( 1, ("\r\nLPCHK VLAN %d: \r\n", send_vid) );

	send_slotno = 0;
	send_portno = 0;
	while( mn_ethLoopCheckPortGetNext(send_slotno, send_portno, &next_slotno, &next_portno ) == VOS_OK )
	{
		send_slotno = next_slotno;
		send_portno = next_portno;

		/*if( loop_port_status_get(send_slotno, send_portno) == CHK_RESULT_LOOP_STATUS )
		{
			continue;
		}*/
		VOS_MemZero(pcportlisttemp,  Mib_PortList_Len);
		 k = ((send_slotno - 1) * MN_SNMP_MAX_PORTS_ONE_SLOT + (send_portno-1)) / 8;  /*定位到某一个字节*/
          	i = ((send_slotno - 1) * MN_SNMP_MAX_PORTS_ONE_SLOT + (send_portno-1)) % 8;  /*计算某个字节的多少位*/
	  	pcportlisttemp[k] |= (0x80 >> i);
		/*port_bit = (ULONG)(1 << (32 - send_portno));*/

		if( pcportlist[k] & pcportlisttemp[k] )
		{
			if( getEthPortOperStatus(1, send_slotno, send_portno, &port_status) == VOS_ERROR )
			{
				/*VOS_ASSERT(0);*/
				continue;
			}

			if( port_status == 1 )
			{
				if( puntaglist[k] & pcportlisttemp[k] )
                           {            
					vid = 1;/*代表此时应该发送untag 报文, 此时UntagVid 中保存的是该端口所untag 进的VLAN*/
                                   UntagVid = send_vid;
                           }
                           else
					vid = send_vid;

				if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
				{
					if(send_portno >16 && send_portno <21)
					{
						if(loopChkDebugSwitch == 3 && port_status == 1)
						sys_console_printf("Ethport %d/%d status = up\r\n", send_slotno, send_portno);
						check_eth_frame_send_by_port( send_slotno, send_portno, vid,UntagVid );
						Flag_Vlan_Have_Valid_Port++;
						loopChkDbgPrintf( 1, ("%d/%d(%s) ", send_slotno, send_portno, (vid==1 ? "U" : "T")) );
					}
				}
				else if( CHECK_SLOTNO_IS_ETH(send_slotno) )
				{
					if(loopChkDebugSwitch == 3 && port_status == 1)
						sys_console_printf("Ethport %d/%d status = up\r\n", send_slotno, send_portno);
					check_eth_frame_send_by_port( send_slotno, send_portno, vid,UntagVid );
					Flag_Vlan_Have_Valid_Port++;
					loopChkDbgPrintf( 1, ("%d/%d(%s) ", send_slotno, send_portno, (vid==1 ? "U" : "T")) );
				}
				else if ( (SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON)
					&& SYS_LOCAL_MODULE_SLOTNO == send_slotno
					&& send_portno <= 4 )
				{
					if(loopChkDebugSwitch == 3 && port_status == 1)
						sys_console_printf("Ethport %d/%d status = up\r\n", send_slotno, send_portno);
					check_eth_frame_send_by_port( send_slotno, send_portno, vid,UntagVid );
					Flag_Vlan_Have_Valid_Port++;
					loopChkDbgPrintf( 1, ("%d/%d(%s) ", send_slotno, send_portno, (vid==1 ? "U" : "T")) );
				}
				else
				{
					if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY )
					{
						VOS_ASSERT(0);    
						return;			 /*6900上暂时不支持ONLY OLT 模式，duzhk 2011-01-20*/ 
#if 0
						if( CHECK_SLOTNO_IS_PON(send_slotno) )
						{
							check_eth_frame_send_by_port( send_slotno, send_portno, vid,UntagVid );
							loopChkDbgPrintf( 1, ("%d/%d(%s) ", send_slotno, send_portno, (vid==1 ? "U" : "T")) );
						}
						else
						{
							loopChkDbgPrintf( 1, ("%d/%d(E) ", send_slotno, send_portno) );
						}
#endif
					}
				}
			}
		}
		/*loopChkDbgPrintf( 1, ("LPCHK:vid=%d, port=%d/%d, status=%d, loop=%d\r\n", 
					send_vid, send_slotno, send_portno, port_status, chk_result_eth_port[send_slotno][send_portno]) );*/
	}
}

VOID ethLoopCheckTestFrameSendByVlanAndPort( USHORT send_vid, ULONG ulSlot ,ULONG ulPort )
{
	USHORT  vid = 0,UntagVid = 0;
	USHORT i = 0, k = 0;
	ULONG port_status = 2;
	UCHAR all_portlist[MN_SNMP_MAX_PORTLIST_LEN];
	UCHAR untag_portlist[MN_SNMP_MAX_PORTLIST_LEN];
	UCHAR pcportlisttemp[MN_SNMP_MAX_PORTLIST_LEN];
	
	VOS_MemZero( untag_portlist, sizeof(untag_portlist) );
	VOS_MemZero( all_portlist, sizeof(all_portlist) );
	VOS_MemZero( pcportlisttemp, sizeof(pcportlisttemp) );

	check_vlan_entry_get_by_switch( send_vid, &all_portlist[0], &untag_portlist[0]);
	k = ((ulSlot - 1) * MN_SNMP_MAX_PORTS_ONE_SLOT + (ulPort-1)) / 8;  
	i = ((ulSlot - 1) * MN_SNMP_MAX_PORTS_ONE_SLOT + (ulPort-1)) % 8; 

	pcportlisttemp[k] |= (0x80 >> i);
		
	if( all_portlist[k] & pcportlisttemp[k] )
	{
		if( getEthPortOperStatus(1, ulSlot, ulPort, &port_status) == VOS_ERROR )
		{
			return VOS_ERROR;
		}

		if( port_status == 1 )
		{
			if( untag_portlist[k] & pcportlisttemp[k] )
		      {            
				vid = 1;/*代表此时应该发送untag 报文, 此时UntagVid 中保存的是该端口所untag 进的VLAN*/
		             UntagVid = send_vid;
		      }
		      else
				vid = send_vid;

			if( CHECK_SLOTNO_IS_ETH(ulSlot) )
			{
				if(loopChkDebugSwitch == 3 && port_status == 1)
					sys_console_printf("Ethport %d/%d status = up\r\n", ulSlot, ulPort);
				check_eth_frame_send_by_port( ulSlot, ulPort, vid,UntagVid );
				loopChkDbgPrintf( 1, ("%d/%d(%s) ", ulSlot, ulPort, (vid==1 ? "U" : "T")) );
			}
			else if ( (SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON)
					&& SYS_LOCAL_MODULE_SLOTNO == ulSlot
					&& ulPort <= 4 )
			{
				if(loopChkDebugSwitch == 3 && port_status == 1)
					sys_console_printf("Ethport %d/%d status = up\r\n", ulSlot, ulPort);
				check_eth_frame_send_by_port( ulSlot, ulPort, vid,UntagVid );
				loopChkDbgPrintf( 1, ("%d/%d(%s) ", ulSlot, ulPort, (vid==1 ? "U" : "T")) );
			}
		}
	}
	else
	{
		sys_console_printf("  ethport %d/%d is not in vlan %d , check fail\r\n", ulSlot, ulPort,vid );
	}	

	return VOS_OK;
}


VOID ethLoopCheckTestOamSend( USHORT send_vid, ULONG *p_all_portlist, ULONG *p_untag_portlist )
{
	ULONG send_slotno, send_portno;
	ULONG next_slotno, next_portno;
	/*ULONG port_status = 2;*/
	USHORT vid;
	SHORT phyPonId;

	if(!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
		return;
	
	send_vid = 0;/*modified by duzhk 2011.3.2 对ONU 永远检测所有VLAN*/

	send_slotno = 0;
	send_portno = 0;
	while( mn_ethLoopCheckPortGetNext(send_slotno, send_portno, &next_slotno, &next_portno ) == VOS_OK )
	{
		send_slotno = next_slotno;
		send_portno = next_portno;

		if( !CHECK_SLOTNO_IS_PON(send_slotno) )
			continue;

		if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER && send_slotno != SYS_LOCAL_MODULE_SLOTNO)
			continue;
		
		phyPonId = GetPonPortIdxBySlot( send_slotno, send_portno );
		if (phyPonId == VOS_ERROR)
			continue;
			
		/*if( getEthPortOperStatus(1, send_slotno, send_portno, &port_status) == VOS_ERROR )
			break;
		if( port_status == 1 )*/
		if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON && phyPonId > 15)
		{
			continue;
		}
		
		if( OLTAdv_IsExist(phyPonId) == TRUE )
		{
			vid = send_vid;
			
			check_oam_frame_send_by_port( send_slotno, send_portno, vid, 0 );		
			loopChkDbgPrintf( 1, ("  PON%d/%d %s OAM\r\n", send_slotno, send_portno, (vid==1 ? "Untaged" : "Tagged")) );
		}
	}
}

LONG eth_loop_detect_frame_is( UCHAR *pFrm, unsigned short length )
{
	ethLoopCheckTaggedFrame_t* pChkFrm = (ethLoopCheckTaggedFrame_t *)pFrm;
	if( NULL == pChkFrm || length > 256)
		return 0;
	
	return ( MAC_ADDR_IS_EQUAL(pChkFrm->desMac, chk_config_frame_dmac) && MAC_ADDR_IS_EQUAL(pChkFrm->srcMac, chk_config_frame_smac) );
}


#ifdef CHK_BRAS_MAC
/* 删除环回MAC */
VOID ethLoopCheckTestMacDelete()
{
	ULONG slotno, portno, onuno;
	USHORT vid = 1;

	/*loopChkDbgPrintf( 2, ("LPCHK:check test mac begin\r\n") );*/
	
	for( slotno=1; slotno<=eth_loop_check_slot_max; slotno++ )
	{
		for( portno=1; portno<=eth_loop_check_port_max[slotno]; portno++ )
		{
			if( loop_port_status_get(slotno, portno) == CHK_RESULT_LOOP_STATUS )
			{
				vid = chk_result_eth_port_vid[slotno][portno];
				check_mac_entry_delete_by_switch(vid);
			}

			for( onuno=1; onuno<=CHK_PON_ONU_MAX; onuno++ )
			{
				if( loop_onu_status_get(slotno, portno, onuno) == CHK_RESULT_LOOP_STATUS )
				{
				
					if(check_mac_entry_delete_by_pon( slotno, portno )==VOS_OK)
						break;
				}
			}
		}
	}
}

/*
	功能:	在PAS5201/5001 MAC地址表中检查特定MAC地址srcAddr是否存在
	输入参数: slotno
				portno
	输出参数: pOnuId
	返回值: 如果pMacAddr指定MAC地址存在则返回VOS_OK；VOS_ERROR－不存在
*/
int check_mac_entry_search_by_pon( ULONG slotno, ULONG portno, ULONG *p_onuid )
{
	short int active_records;
	short int i;
	short int OnuIdx, PonPortIdx;

	int rc = VOS_ERROR;
	UCHAR  *pMacAddr = chk_config_frame_smac;

	/*sys_console_printf( "pon_mac_addr_table size = %d\r\n", sizeof(pon_mac_addr_table) );*/
	/*if( pon_mac_addr_table == NULL )
		return rc;*/

	if( (p_onuid == NULL) )
		return rc;
	PonPortIdx = GetPonPortIdxBySlot( (short int)slotno, (short  int)portno );
	if (PonPortIdx == VOS_ERROR)
		return rc;

	if( getPonChipInserted((unsigned char)slotno, (unsigned char)portno) !=  PONCHIP_EXIST )
	{
		return VOS_ERROR;
	} 
	if( PonPortIsWorking(PonPortIdx) == FALSE ) 
	{
		return VOS_ERROR;
	}

	/* VOS_MemSet( pon_mac_addr_table, 0, sizeof(pon_mac_addr_table) ); */
    VOS_ASSERT(pon_mac_addr_table);
	{
#if 1
		if( OLT_CALL_ISOK( OLT_GetMacAddrTbl(PonPortIdx, &active_records, pon_mac_addr_table) ) )
#else
		if(( PAS_get_address_table(PonPortIdx, &active_records, pon_mac_addr_table)) == PAS_EXIT_OK )
#endif
		{
			for(i=0; i< active_records; i++)
			{
				/*if( VOS_MemCmp(pon_mac_addr_table[i].mac_address, pMacAddr, 6) != 0 )
					continue;*/
				if( (pon_mac_addr_table[i].mac_address[0] != pMacAddr[0]) ||
					(pon_mac_addr_table[i].mac_address[1] != pMacAddr[1]) ||
					(pon_mac_addr_table[i].mac_address[2] != pMacAddr[2]) ||
					(pon_mac_addr_table[i].mac_address[3] != pMacAddr[3]) ||
					(pon_mac_addr_table[i].mac_address[4] != pMacAddr[4]) ||
					(pon_mac_addr_table[i].mac_address[5] != pMacAddr[5]) )
				{
					continue;
				}
				if( pon_mac_addr_table[i].logical_port == PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID )
				{
					*p_onuid = 0;
					return VOS_ERROR;
				}
				else
				{
					OnuIdx = GetOnuIdxByLlid(PonPortIdx, pon_mac_addr_table[i].logical_port);
					*p_onuid = (OnuIdx+1);
				}
				return VOS_OK;
			}
		}
	}
	
	return VOS_ERROR;
}

/*#ifdef CHK_BRAS_MAC*/
#if 0
/*add by shixh20090714*/
/*
	功能:	在PAS5201/5001 MAC地址表中检查特定MAC地址srcAddr是否存在
	输入参数: slotno
				portno
	输出参数: pOnuId
	返回值: 如果pMacAddr指定MAC地址存在则返回VOS_OK；VOS_ERROR－不存在
*/
int check_BRAS_mac_entry_search_by_pon( ULONG slotno, ULONG portno, ULONG *p_onuid,UCHAR  Mac[6])
{
	short int active_records;
	short int i/*,j*/;
	short int OnuIdx, PonPortIdx;

	int rc = VOS_ERROR;
	UCHAR  *pMacAddr =Mac;/* chk_config_frame_BRAS_mac;*/
	
	/*sys_console_printf("chk_config_frame_BRAS_mac:");
	for(j=0; j<(ETH_LOOP_MAC_ADDR_LEN -1); j++)
		{
			sys_console_printf( "%02x%02x", chk_config_frame_BRAS_mac[j],chk_config_frame_BRAS_mac[j+1]);
			j++;
			if( j != 5 )sys_console_printf(".");
		}
	sys_console_printf("\r\n");
	
	sys_console_printf( "pon_mac_addr_table size = %d\r\n", sizeof(pon_mac_addr_table) );*/
	/*if( pon_mac_addr_table == NULL )
		return rc;*/

	if( (p_onuid == NULL) )
		return rc;

	PonPortIdx = GetPonPortIdxBySlot( (short int)slotno, (short  int)portno );
	if (PonPortIdx == VOS_ERROR)	
		return rc;
		

	if( getPonChipInserted((unsigned char)slotno, (unsigned char)portno) !=  PONCHIP_EXIST )
	{
		sys_console_printf("PONCHIP_EXIST not exit\r\n");
		return VOS_ERROR;
	} 
	if( PonPortIsWorking(PonPortIdx) == FALSE ) 
		return VOS_ERROR;
	

	VOS_MemSet( pon_mac_addr_table, 0, sizeof(pon_mac_addr_table) );
	{
		if( OLT_CALL_ISOK( OLT_GetMacAddrTbl(PonPortIdx, &active_records, pon_mac_addr_table) ) )
		{
			/*sys_console_printf("pon_mac_addr_table:");
			for(i=0; i<active_records; i++)
				{
					for(j=0; j<(ETH_LOOP_MAC_ADDR_LEN -1); j++)
						{
						sys_console_printf("%02x%02x", Mac_addr_table[i].mac_address[j], Mac_addr_table[i].mac_address[j+1] );
						j++;
						if( j != 5 )sys_console_printf(".");
						}
				}
			sys_console_printf("\r\n");*/
			
			for(i=0; i< active_records; i++)
			{
				/*if( VOS_MemCmp(pon_mac_addr_table[i].mac_address, pMacAddr, 6) != 0 )
					continue;*/
				if( (pon_mac_addr_table[i].mac_address[0] != pMacAddr[0]) ||
					(pon_mac_addr_table[i].mac_address[1] != pMacAddr[1]) ||
					(pon_mac_addr_table[i].mac_address[2] != pMacAddr[2]) ||
					(pon_mac_addr_table[i].mac_address[3] != pMacAddr[3]) ||
					(pon_mac_addr_table[i].mac_address[4] != pMacAddr[4]) ||
					(pon_mac_addr_table[i].mac_address[5] != pMacAddr[5]) )
				{
					continue;
				}
				if( pon_mac_addr_table[i].logical_port == PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID )
				{
					*p_onuid = 0;
					return VOS_ERROR;
				}
				else
				{
					OnuIdx = GetOnuIdxByLlid(PonPortIdx, pon_mac_addr_table[i].logical_port);
					*p_onuid = (OnuIdx+1);
				}
				return VOS_OK;
			}
		}
	}
	
	return VOS_ERROR;
}
int check_BRAS_mac_entry_delete_by_pon( ULONG slotno, ULONG portno,UCHAR mac[6] )
{
	/*int j;*/
	UCHAR  *pMacAddr = mac;/*chk_config_frame_BRAS_mac;*/
	short int PonPortIdx = GetPonPortIdxBySlot( (short int)slotno, (short  int)portno );
	if (PonPortIdx == VOS_ERROR)
		return PonPortIdx;
	OLT_RemoveMac( PonPortIdx, pMacAddr );
	
	return VOS_OK;
}

int check_BRAS_mac_entry_search_by_switch()
{
	ULONG slotno, portno, onuno, vid;
	int index_min,index_max,i;
	int l2_port ;
	/*int j;*/
	cli_l2_addr_t l2addr={0};
	int rc = VOS_ERROR;
	UCHAR  pMacAddr[6]={0};
	int t;
	
	for(t=0;t<CHK_BRAS_MAC_MAX;t++)
		{
		/*if(VOS_MemCmp(chk_bras[t],chk_config_frame_BRAS_mac_t,sizeof(chk_config_frame_BRAS_mac_t))==0)*/
		if( MAC_ADDR_IS_ZERO(chk_bras[t]) )
			{
				continue;
			}
		else
			{
				/*pMacAddr=chk_bras[t];*/
				VOS_MemCpy(pMacAddr,&chk_bras[t][0],6);
				if( bms_l2_idx_get( 0, &index_min, &index_max ) == VOS_ERROR )	
					return rc;
		
				for( i=index_min; i<=index_max; i++ )
				{
			             	if( bms_l2_entry_get(0, i, &l2addr, -1) == 0 )
					{
						if( (l2addr.mac[0] != pMacAddr[0]) || (l2addr.mac[1] != pMacAddr[1]) ||
							(l2addr.mac[2] != pMacAddr[2]) || (l2addr.mac[3] != pMacAddr[3]) ||
							(l2addr.mac[4] != pMacAddr[4]) || (l2addr.mac[5] != pMacAddr[5]) )
						{
							continue;
						}
						
						l2_port = l2addr->port+32*l2addr->modid;
						swport_2_slot_port( l2_port, &slotno, &portno );

						vid = l2addr.vid;
						rc = VOS_OK;

						if( CHECK_SLOTNO_IS_PON(slotno) )
						{
								/* 如果是PON端口，可以进一步检查ONU环路，然后上报告警事件 */
								if(check_BRAS_mac_entry_search_by_pon(slotno, portno, &onuno,pMacAddr)==VOS_OK)
								{
									ponBRASAlarm_EventReport(1,slotno,portno,pMacAddr);
									check_BRAS_mac_entry_delete_by_pon(slotno, portno,pMacAddr);
								}
								else
								{
									ponBRASAlarmClear_EventReport(1,slotno,portno,pMacAddr);
									/*产生BRAS 告警清除*/
								}
						}
						else
						{
								loopChkDbgPrintf( 2, ("LPCHK:slot%d type error, port%d/%d", slotno, slotno, portno) );
								rc = VOS_ERROR;
						}

						cheak_BRAS_mac_entry_delete_by_switch(vid,pMacAddr);
						break;
			             	}
				}
			}
		}
	
	
	/*chk_config_frame_BRAS_mac_flag=FALSE;*/
	
	return rc;	
}

/*	功能:	在BCM56500/56300 MAC地址表中检查特定MAC地址srcAddr是否存在
	输出参数: pSlotNo
				pPortNo
				pVid
	返回值: 如果pMacAddr指定MAC地址存在则返回VOS_OK；VOS_ERROR－不存在
*/
int check_mac_entry_search_by_switch()
{
	ULONG slotno, portno, onuno, vid;
	int index_min,index_max,i;
	int l2_port ;
	cli_l2_addr_t l2addr={0};
	int rc = VOS_ERROR;
	UCHAR  *pMacAddr = chk_config_frame_smac;
	
	if( (chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONU) && (ethLoopCheckEthBoardIsExist() == VOS_ERROR) )
		return rc;

	if( bms_l2_idx_get( 0, &index_min, &index_max ) == VOS_ERROR )
		return rc;

	for( i=index_min; i<=index_max; i++ )
	{
             	if( bms_l2_entry_get(0, i, &l2addr, -1) == 0 )
		{
			/*if( VOS_MemCmp(l2addr.mac, pMacAddr,6) != 0 )
             			continue;*/
			if( (l2addr.mac[0] != pMacAddr[0]) || (l2addr.mac[1] != pMacAddr[1]) ||
				(l2addr.mac[2] != pMacAddr[2]) || (l2addr.mac[3] != pMacAddr[3]) ||
				(l2addr.mac[4] != pMacAddr[4]) || (l2addr.mac[5] != pMacAddr[5]) )
			{
				continue;
			}
			
			l2_port = l2addr.port;
			if( l2_port > 24 )
				continue;
			swport_2_slot_port( l2_port, &slotno, &portno );
			vid = l2addr.vid;
			rc = VOS_OK;

			/*if( check_slotno_is_illegal(slotno) ||
				(portno == 0) || (portno > eth_loop_check_port_max[slotno]) )*/
			if( ethLoopCheckPortIsExist(slotno, portno) == VOS_ERROR )
			{
				/*VOS_ASSERT(0);
				return VOS_ERROR;*/
				continue;
			}
				
			/* 存在环路 */

			if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY )
			{
				if( CHECK_SLOTNO_IS_ETH(slotno) )
				{
					/* 如果是以太网端口直接上报告警事件 */
					loop_port_status_set( slotno, portno );
					loopChkDbgPrintf( 2, ("LPCHK:find eth%d/%d looping %d\r\n", slotno, portno, chk_result_eth_port[slotno][portno]) );
				}
				else if( SYS_MODULE_IS_PON(slotno) )
				{
					/* 如果是PON端口，可以进一步检查ONU环路，然后上报告警事件 */
					if( check_mac_entry_search_by_pon(slotno, portno, &onuno) == VOS_OK )
					{
						/* 上报ONU环路 */
						loop_onu_status_set( slotno,  portno, onuno);
						loopChkDbgPrintf( 2, ("LPCHK:find onu%d/%d/%d looping %d\r\n", slotno, portno, onuno, chk_result_pon_onu[slotno][portno][onuno]) );
					}
					else
					{
						/* 上报PON口环路 */
						loop_port_status_set( slotno, portno );
						loopChkDbgPrintf( 2, ("LPCHK:find pon%d/%d looping %d\r\n", slotno, portno, chk_result_eth_port[slotno][portno]) );
					}
					if(check_mac_entry_delete_by_pon( slotno, portno )==VOS_OK)
						break;
					/*check_mac_entry_delete_by_pon( slotno, portno );*/
				}
				else
				{
					loopChkDbgPrintf( 2, ("LPCHK:slot%d type error, port%d/%d", slotno, slotno, portno) );
					rc = VOS_ERROR;
				}

				if( rc == VOS_OK )
				{
					chk_result_eth_port_vid[slotno][portno] = vid;
					/*check_mac_entry_delete_by_switch( vid );*/
				}
             		}
			else /*if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONU )*/
			{
				if( CHECK_SLOTNO_IS_ETH(slotno) )
				{
					/*ULONG port_status = 2;
					if( (getEthPortOperStatus(1, slotno, portno, &port_status) == VOS_OK) && (port_status == 1) )*/
						addEthPortToLoopList( 1, slotno, portno, vid );
					
				}
			}
		}
	}
	return rc;	
}
#endif

extern int bms_l2_delete(int unit,int vid,void * Mac,int discard_flag);
int check_mac_entry_delete_by_switch( USHORT vid )
{
	UCHAR  *pMacAddr = chk_config_frame_smac;
	if( bms_l2_delete(0, vid, pMacAddr, 0) == 0 )
		return VOS_OK;
	return VOS_ERROR;
}

int cheak_BRAS_mac_entry_delete_by_switch(USHORT vid,UCHAR Pmac[6])
{
	UCHAR  *pMacAddr =Pmac;/*chk_config_frame_BRAS_mac;*/
	if( bms_l2_delete(0, vid, pMacAddr, 0) == 0 )
		return VOS_OK;
	return VOS_ERROR;
}
#endif

int check_mac_entry_delete( ULONG slotno, ULONG portno ,int devIdx)
{
	ULONG portnum;

	if(chk_config_detect_mac_clean_enable == 1)
	{
		if(1 == devIdx )
		{
			portnum = USER_PORT_2_SWITCH_PORT(slotno, portno);
			bms_l2_del_by_port(0, portnum);
		}
		else
		{
			if(check_mac_entry_delete_by_pon(slotno,portno) == VOS_ERROR)
			{	
				return VOS_ERROR;
			}
		}
	}

	return VOS_OK;
}

int check_mac_entry_delete_by_pon( ULONG slotno, ULONG portno )
{
	/*LoopCDPMsgHead_t *bDatPdpBuf = NULL;*/
	/*unsigned int ulLen = 0 ;
	SYS_MSG_S      *pMsg       = NULL;*/
	short int PonPortIdx;
	
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{
		PonPortIdx = GetPonPortIdxBySlot( (short int)slotno, (short  int)portno );
		if (PonPortIdx == VOS_ERROR)
			return PonPortIdx;
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		OLT_ResetAddrTbl(PonPortIdx,PON_ALL_ACTIVE_LLIDS,ADDR_DYNAMIC);
		#else
		PAS_reset_address_table(PonPortIdx,PON_ALL_ACTIVE_LLIDS,ADDR_DYNAMIC);
		#endif
		/*PAS_remove_address_table_record( PonPortIdx, pMacAddr );*/
		loopChkDbgPrintf( 4, ("LPCHK:del pon%d/%d mac-addr OK\r\n", slotno, portno) );
	}
	/*else
	{     
		if ( !SYS_MODULE_IS_READY(slotno) ) 问题单: 11883
		{
			return VOS_OK;
		}

		if(!SYS_MODULE_IS_PON(slotno))	
		{
			return VOS_OK;
		}
	       ulLen=sizeof(LoopCDPMsgHead_t)+sizeof(SYS_MSG_S);
		pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_LOOPBACK);

	       if( NULL == pMsg )
	       {
	            VOS_ASSERT(0);
	            return  VOS_ERROR;
	       }
	       VOS_MemZero((CHAR *)pMsg, ulLen );

	        SYS_MSG_SRC_ID( pMsg )       = MODULE_LOOPBACK;
	        SYS_MSG_DST_ID( pMsg )       = MODULE_LOOPBACK;
	        SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
	        SYS_MSG_FRAME_LEN( pMsg )    = sizeof(LoopCDPMsgHead_t);
	        SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
	        SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
	        SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
	        SYS_MSG_DST_SLOT( pMsg ) = SYS_MASTER_ACTIVE_SLOTNO;
		 SYS_MSG_MSG_CODE(pMsg) = LOOP_MAC_ENTRY_DELETE ;
		 
	        bDatPdpBuf= (LoopCDPMsgHead_t * ) ( pMsg + 1 );
		 bDatPdpBuf->srcslot = slotno; 此时srcslot 项存的的发送的目的槽位号
		 bDatPdpBuf->ponId = portno;
		 bDatPdpBuf->pSendBufLen = 0;
		
	       if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_LOOP_INSERT, slotno, RPU_TID_CDP_LOOP_INSERT,  0, (VOID *)pMsg, ulLen, MODULE_LOOPBACK ) )
		{
			VOS_ASSERT(0); 
			CDP_FreeMsg(pMsg);
			return VOS_ERROR;
		}
		
		loopChkDbgPrintf( 4, ("LPCHK:request to del pon%d/%d mac-addr\r\n", slotno, portno) );
		return VOS_OK;
	}*/
	return VOS_OK;
}

static int eth_loop_chk_fc()
{
	if( eth_loop_chk_start_flag )
	{
		if( (eth_loop_chk_tx_pkts_ctrl_number < 10) || (eth_loop_chk_tx_pkts_ctrl_number > 250) )
			eth_loop_chk_tx_pkts_ctrl_number = 25;
		if( eth_loop_chk_tx_pkts_ctrl_delay > 50 )
			eth_loop_chk_tx_pkts_ctrl_delay = 10;
		else if( eth_loop_chk_tx_pkts_ctrl_delay == 0 )
			return VOS_OK;
		
		eth_loop_chk_tx_pkts_ctrl++;
		if( eth_loop_chk_tx_pkts_ctrl > eth_loop_chk_tx_pkts_ctrl_number )	/* 控制发帧次数 */
		{
			VOS_TaskDelay(eth_loop_chk_tx_pkts_ctrl_delay);
			eth_loop_chk_tx_pkts_ctrl = 0;
		}
		return VOS_OK;
	}
	return VOS_ERROR;
}

/*#ifdef __test_boardcast*/

/* added by xieshl 20080505,  测试OAM 帧发送 */
extern int BroadcastOamFrameToOnu( short int PonPortIdx, int length, unsigned char *content );
static UCHAR oam_dst_mac_addr[] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x02 };
static UCHAR oam_src_mac_addr[] = { 0x00, 0x0f, 0xe9, 0x00, 0x00, 0x00 };
VOID check_oam_frame_send_by_port( ULONG slotno, ULONG portno, USHORT vid, UCHAR DisableFlag )
{
	short int PonPortIdx,OnuIdx,llid;
	static ethLoopCheckOamFrame_t  oam;
	
	if( (PonPortIdx = GetPonPortIdxBySlot(slotno, portno)) == VOS_ERROR )
		return;

	/*VOS_MemZero( &oam, sizeof(ethLoopCheckOamFrame_t) );*/
	VOS_MemCpy( oam.DA, oam_dst_mac_addr, 6 );
	VOS_MemCpy( oam.SA, oam_src_mac_addr, 6 );
	oam.Type = 0x8809;
	oam.SubType = 0x03;
	oam.Flag = 0x0050;
	oam.Code = 0xfe;
	
	oam.OUI[0] = 0x00;
	oam.OUI[1] = 0x0f;
	oam.OUI[2] = 0xe9;
	oam.GwOpcode = 0x01;
	oam.SendSerNo = 0;
	oam.WholePktLen = sizeof(ethLoopCheckOamMsg_t);
	oam.PayLoadOffset = 0;
	oam.PayLoadLength = sizeof(ethLoopCheckOamMsg_t);
	VOS_MemZero(oam.SessionID, 8);
	
	oam.Msg.type = 0x04;
	oam.Msg.result = 0;
	
	if( DisableFlag == 1 )
		oam.Msg.enable = FALSE;
	else
		oam.Msg.enable = chk_config_detect_enable;
	
	oam.Msg.vlanId = vid;
	VOS_MemCpy( oam.Msg.macAddr, chk_config_frame_smac, ETH_LOOP_MAC_ADDR_LEN );
	oam.Msg.interval = chk_config_timer_len;
	oam.Msg.ctrlPolicy = chk_config_detect_control_enable;
	oam.Msg.uptime=chk_config_up_times;
	oam.Msg.retrycount=chk_config_up_times_thresh;
	VOS_MemZero( oam.pad, sizeof(oam.pad) );

	/*if(loopChkDebugSwitch == 3)
	{
		pBuf = (UCHAR*)&oam;
		for(i=0;i<sizeof(ethLoopCheckOamFrame_t);i++)
		{
			sys_console_printf("%02x  " ,*pBuf);
			if((i+1)%6==0)
				sys_console_printf("\r\n");
			pBuf++;
		}
	}*/

	/*added by wangjiah@2016-09-23:begin*/
	/*PMC onu can't receive GW broadcast oam from 8xep
	 *Need to send unicast oam to PMC specailly
	 * */
	if(SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)	
	{
		for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx++)
		{
			int onuType = 0;
			/*查找在线的ONU*/
			if(GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP)
				continue;
			if( V2R1_CTC_STACK )
			{
				if( GetOnuType(PonPortIdx, OnuIdx, &onuType) == VOS_ERROR )
					continue;
				if( (onuType == V2R1_ONU_CTC) || (V2R1_ONU_GPON == onuType) )
				{
					continue;	
				}
				else
				{
					llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
					if( llid == INVALID_LLID) continue;
					UnicastOamFrameToOnu(PonPortIdx, llid, sizeof(ethLoopCheckOamFrame_t),(UCHAR*)&oam);
				}
			}
		}
	}
	/*added by wangjiah@2016-09-23:end*/
	
	BroadcastOamFrameToOnu( PonPortIdx, sizeof(ethLoopCheckOamFrame_t), (UCHAR*)&oam );
}

/* 测试帧发送 */
extern int BroadcastDataFrameToOnu( short int PonPortIdx, int length, unsigned char *content );
VOID check_eth_frame_send_by_port( ULONG slotno, ULONG portno, USHORT vid, USHORT UntagVid)
{
    ULONG ret=VOS_ERROR;
	USHORT swPort = 0,i=0;
	short int PonPortIdx,oltType=0;
	ULONG len;
	ULONG ulTpid=0,onuDesc=0;
	ethLoopCheckTaggedFrame_t *pFrm;
	ethLoopCheckUntaggedFrame_t *pUntagFrm;
       uchar *p;
    USHORT send_vid =0;

	if( VOS_NO == IFM_IfIndex_IS_Local( IFM_ETH_CREATE_INDEX(slotno, portno) ) )
		return ;
	
	if( eth_loop_chk_fc() == VOS_ERROR )
		return;

	pFrm = VOS_Malloc( sizeof(ethLoopCheckTaggedFrame_t), MODULE_LOOPBACK );
	if( pFrm == NULL ) 
    	{
    		VOS_ASSERT(0);
		return;
    	}
	if( (vid == 0) || (vid > 4094) )
		vid = 1;

	oltType=GetOltType();
	if(oltType==V2R1_OLT_GFA6100)
		oltType=1;		/*对应发包中的规则1-6100，2-6700, 3-6900 */
	else if(oltType == V2R1_OLT_GFA6900)
		oltType = 3;
	else if(oltType == V2R1_OLT_GFA8000)           /*8000暂定为4*/
		oltType = 4;
	else if(oltType == V2R1_OLT_GFA8100 )/*8100暂定为5*/
		oltType = 5;
	else
		oltType=2;

	onuDesc |=(slotno<<16);
	onuDesc |=(portno<<8);

	ulTpid=0x81000000+(vid|0xe000);
	pUntagFrm = (ethLoopCheckUntaggedFrame_t *)pFrm;

	VOS_MemZero( pUntagFrm, sizeof(ethLoopCheckUntaggedFrame_t ) );

	VOS_MemCpy( pFrm->desMac, chk_config_frame_dmac, ETH_LOOP_MAC_ADDR_LEN );
	VOS_MemCpy( pFrm->srcMac, chk_config_frame_smac, ETH_LOOP_MAC_ADDR_LEN );

	if( vid == 1 )/*代表此时应该发送untag 报文, 此时UntagVid 中保存的是该端口所untag 进的VLAN*/
	{
		pUntagFrm->ethType = /*chk_config_frame_ethtype*/0x0800;
		pUntagFrm->oltType=oltType;
		pUntagFrm->chkFlag=0x0080;
		pUntagFrm->onuType=0;	/*此位置代表onu类型*/
		pUntagFrm->onuDesc=onuDesc;
		pUntagFrm->onuVlan=UntagVid;
		VOS_MemCpy(pUntagFrm->onuMac,SYS_PRODUCT_BASEMAC , ETH_LOOP_MAC_ADDR_LEN);
		len = sizeof(ethLoopCheckTaggedFrame_t) - 8;
        send_vid = UntagVid;
	}
	else
	{
		pFrm->vlanInf= ulTpid;
		pFrm->ethType =/* chk_config_frame_ethtype*/0x0800;
		pFrm->chkFlag=0x0080;
		pFrm->oltType=oltType;
		pFrm->onuType=0;	/*此位置代表onu类型*/
		pFrm->onuDesc = onuDesc;
		pFrm->onuVlan = vid;
		VOS_MemCpy(pFrm->onuMac,SYS_PRODUCT_BASEMAC , ETH_LOOP_MAC_ADDR_LEN);
		len = sizeof(ethLoopCheckTaggedFrame_t) - 4;
        send_vid = vid;
	}

	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		if(portno > 16 && portno < 21)
		{
			loopChkDbgPrintf(1, ("loop bms packet send to %d/%d \r\n",slotno,portno));
			{
				p=(uchar*)pFrm;
				for(i=0;i<sizeof(ethLoopCheckTaggedFrame_t)-ETH_LOOP_FRM_PAYLOAD_LEN;i++)
				{
					loopChkDbgPrintf(1,("%02x  " ,*p));
					if((i+1)%6==0)
					loopChkDbgPrintf(1,("\r\n"));
					p++;
				}
			}
			/*bms_packet_send_by_port会释放传进去的指针*/
			/*ret=bms_packet_send_by_port( (VOID*)pFrm, len, swPort, 0 );*/
			ret=bms_packet_send_by_port( (VOID*)pFrm, len, slotno, portno, send_vid );
		
			return ;
		}
	}
	
	if( (SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON)
		&& SYS_LOCAL_MODULE_SLOTNO == slotno 
		&& portno <= 4)
	{
		/*swPort = slot_port_2_swport_no( slotno, portno );*/
		loopChkDbgPrintf(1, ("loop bms packet send to %d/%d \r\n",slotno,portno));
		{
			p=(uchar*)pFrm;
			for(i=0;i<sizeof(ethLoopCheckTaggedFrame_t)-ETH_LOOP_FRM_PAYLOAD_LEN;i++)
			{
				loopChkDbgPrintf(1,("%02x  " ,*p));
				if((i+1)%6==0)
				loopChkDbgPrintf(1,("\r\n"));
				p++;
			}
		}
		/*bms_packet_send_by_port会释放传进去的指针*/
		/*ret=bms_packet_send_by_port( (VOID*)pFrm, len, swPort, 0 );*/
		ret=bms_packet_send_by_port( (VOID*)pFrm, len, slotno, portno, send_vid );
	
		return ;
	}

	if( CHECK_SLOTNO_IS_ETH(slotno) )
	{
		/*swPort = slot_port_2_swport_no( slotno, portno );*/
		loopChkDbgPrintf(1, ("loop bms packet send to %d/%d \r\n",slotno,portno));
		{
			p=(uchar*)pFrm;
			for(i=0;i<sizeof(ethLoopCheckTaggedFrame_t)-ETH_LOOP_FRM_PAYLOAD_LEN;i++)
			{
				loopChkDbgPrintf(1,("%02x  " ,*p));
				if((i+1)%6==0)
				loopChkDbgPrintf(1,("\r\n"));
				p++;
			}
		}
		/*bms_packet_send_by_port会释放传进去的指针*/
		/*ret=bms_packet_send_by_port( (VOID*)pFrm, len, swPort, 0 );*/
		ret=bms_packet_send_by_port( (VOID*)pFrm, len, slotno, portno, send_vid );
        
		return ;
	}
	else if( CHECK_SLOTNO_IS_PON(slotno) )
	{
		if( (PonPortIdx = GetPonPortIdxBySlot(slotno, portno)) != VOS_ERROR )
		{
			loopChkDbgPrintf(1, ("loop bms packet send to slot %d,ponportId %d \r\n",slotno,PonPortIdx));
			{
				p=(uchar*)pFrm;
				for(i=0;i<sizeof(ethLoopCheckTaggedFrame_t);i++)
				{
					loopChkDbgPrintf(1,("%02x  " ,*p));
					if((i+1)%6==0)
					loopChkDbgPrintf(1,("\r\n"));
					p++;
				}
			}
			BroadcastDataFrameToOnu(PonPortIdx, sizeof(ethLoopCheckTaggedFrame_t), (UCHAR*)pFrm );
		}
	}
	VOS_Free( pFrm );
}

/* 检索交换芯片VLAN表 */
/* added by xieshl 20080519, #6679 */
int check_vlan_entry_get_by_switch( USHORT vid, UCHAR *p_all_portlist, UCHAR *p_untag_portlist )
{
	/*int i;
	CHAR error[256];
	USHORT usLang = 1;*/
	ULONG portlist_len;
	
	if( (vid == 0) || (vid >= 4095) || (p_all_portlist == NULL) || (p_untag_portlist == NULL) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	portlist_len = Mib_PortList_Len;
	VOS_MemZero( (VOID*)&p_all_portlist[0], portlist_len );
	VOS_MemZero( (VOID*)&p_untag_portlist[0], portlist_len );

	MN_Vlan_Get_AllPorts( vid, (VOID*)&p_all_portlist[0], &portlist_len);
	MN_Vlan_Get_UntagPorts( vid, (VOID*)&p_untag_portlist[0], &portlist_len);
		
	return VOS_OK;
}

int check_vlan_entry_getnext_by_switch( USHORT vid, USHORT *pnext_vid, UCHAR *p_all_portlist, UCHAR *p_untag_portlist )
{
	/*int i;*/
	CHAR error[256];
	USHORT usLang = 1;
	USHORT next_vid = 0;
	ULONG portlist_len;

	if( eth_loop_chk_fc() == VOS_ERROR )
		return VOS_ERROR;

	if( (pnext_vid == NULL) || (p_all_portlist == NULL) || (p_untag_portlist == NULL) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	portlist_len = Mib_PortList_Len;
	VOS_MemZero( (VOID*)&p_all_portlist[0], portlist_len );
	VOS_MemZero( (VOID*)&p_untag_portlist[0], portlist_len );

	if( vid == 0 )
	{
		if( MN_Vlan_Get_First(&next_vid, usLang, error) == VOS_OK )
		{
			*pnext_vid = next_vid;
			MN_Vlan_Get_AllPorts( next_vid, (VOID*)&p_all_portlist[0], &portlist_len);
			MN_Vlan_Get_UntagPorts( next_vid, (VOID*)&p_untag_portlist[0], &portlist_len);
		}
		else
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}	
		/*for( i=1; i<MN_SNMP_MAX_PORTLIST_LEN; i++ )
			p_all_portlist[i] |= p_untag_portlist[i];*/
		
		return VOS_OK;
	}
	else
	{
		if( MN_Vlan_Get_Next( vid, &next_vid, usLang, error) == VOS_OK )
		{
			*pnext_vid = next_vid;
			MN_Vlan_Get_AllPorts( next_vid, (VOID*)&p_all_portlist[0], &portlist_len);
			MN_Vlan_Get_UntagPorts( next_vid, (VOID*)&p_untag_portlist[0], &portlist_len);
			return VOS_OK;
		}
	}
	return VOS_ERROR;
}

BOOL check_slotno_is_illegal( ULONG slotno )
{
	if ( SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER )
	{
		return (SYS_LOCAL_MODULE_SLOTNO != slotno);
	}
	else
    	{
		return ( (slotno == 0) || (slotno > eth_loop_check_slot_max) ||
			((!CHECK_SLOTNO_IS_ETH(slotno))  && (!CHECK_SLOTNO_IS_PON(slotno))) );
    	}
}

/* 打开环路检测使能 */
int mn_ethLoopCheckEnable()
{
    /*UCHAR ulMask[6]={0xff,0xff,0xff,0xff,0xff,0xff};*/
    
	eth_loop_chk_start_flag = TRUE;
    /*add by zhengyt,为cpu增加一条l2u，使得cpu接收该mac地址包*/
/*#ifndef BCM_DRV_568	*/
    /*delL2uForLoopChk( chk_config_frame_dmac );
    addL2uEntry(chk_config_frame_dmac, ulMask, 1,1, 0);*/
    if( SYS_LOCAL_MODULE_ISHAVEPP() && !SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
	    setL2uCpu(L2U_INDEX_NUM,1);
	
    chk_vid_bak = 0;
/*#endif*/
    
	return ethLoopCheckNotifyEvent( ETH_LOOP_CHECK_START );
}
/* 关闭环路检测使能 */
int mn_ethLoopCheckDisable()
{
    /*UCHAR ulMask[6]={0xff,0xff,0xff,0xff,0xff,0xff};*/
 /*#ifndef BCM_DRV_568	*/    
      /*delL2uForLoopChk( chk_config_frame_dmac );
      addL2uEntry(chk_config_frame_dmac, ulMask, 1,0, 0);*/
      if( SYS_LOCAL_MODULE_ISHAVEPP() && !SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
	      setL2uCpu(L2U_INDEX_NUM,0);
/* #endif*/
	eth_loop_chk_start_flag = FALSE;
	return ethLoopCheckNotifyEvent( ETH_LOOP_CHECK_STOP );
}

/* 查询环路检测使能 */
int mn_ethLoopCheckEnableGet()
{
	if( chk_config_detect_enable == TRUE )
		return ETH_LOOP_CHECK_ENABLE;
	return ETH_LOOP_CHECK_DISABLE;
}

/* added by xieshl 20080505, 增加环路检测控制 */
int mn_ethLoopCheckContralEnable()
{
	chk_config_detect_control_enable = TRUE;
	return VOS_OK;
}
int mn_ethLoopCheckContralDisable()
{
	chk_config_detect_control_enable = FALSE;
	return VOS_OK;
}

int mn_ethLoopCheckMacCleanEnable()
{
	chk_config_detect_mac_clean_enable = TRUE;
	return VOS_OK;
}
int mn_ethLoopCheckMacCleanDisable()
{
	chk_config_detect_mac_clean_enable = FALSE;
	return VOS_OK;
}

int mn_ethLoopCheckContralEnableGet()
{
	if( chk_config_detect_control_enable == TRUE )
		return ETH_LOOP_CHECK_ENABLE;
	return ETH_LOOP_CHECK_DISABLE;
}

int mn_ethLoopCheckMacCleanEnableGet()
{
	if( chk_config_detect_mac_clean_enable == TRUE )
		return ETH_LOOP_CHECK_ENABLE;
	return ETH_LOOP_CHECK_DISABLE;
}

/* 查询环路检测周期 */
ULONG mn_ethLoopCheckTimerGet()
{
	return chk_config_timer_len;
}
/* 设置环路检测周期，单位:秒，范围12－600 */
int mn_ethLoopCheckTimerSet( ULONG interval )
{
	if( (interval < MIN_CHECK_TIMER_INTERVAL) || (interval > MAX_CHECK_TIMER_INTERVAL) )
		return VOS_ERROR;
	else
	{
		chk_config_timer_len = interval;
	}
	if(interval > 20)
		ethLoopCheckTestOamSend( chk_config_eth_vid, 0, 0 );
	return VOS_OK;
}
/* 设置默认环路检测周期，即60秒 */
int mn_ethLoopCheckTimerDefaultSet()
{
	chk_config_timer_len = DEF_CHECK_TIMER_INTERVAL;
	return VOS_OK;
}

/* 查询检测VLAN */
USHORT mn_ethLoopCheckVlanGet()
{
	return chk_config_eth_vid;
}
/* 设置环路检测VLAN */
int mn_ethLoopCheckVlanSet( USHORT vid )
{
	if( vid > 4094 )
		return VOS_ERROR;
	chk_config_eth_vid = vid;
	return VOS_OK;
}
/* 设置默认环路检测VLAN，即所有VLAN */
int mn_ethLoopCheckVlanDefaultSet()
{
	chk_config_eth_vid = 0;
	return VOS_OK;
}

/* 查询检测测试帧源MAC地址 */
int mn_ethLoopCheckMacGet( UCHAR *pMac)
{
	if( pMac == NULL )
		return VOS_ERROR;
	VOS_MemCpy( pMac, chk_config_frame_smac, ETH_LOOP_MAC_ADDR_LEN );
	return VOS_OK;
}
/* 设置检测测试帧源MAC地址 */
int mn_ethLoopCheckMacSet( UCHAR *pMac )
{
	if( pMac == NULL )
		return VOS_ERROR;
	VOS_MemCpy( chk_config_frame_smac, pMac, ETH_LOOP_MAC_ADDR_LEN );
	return VOS_OK;
}
/* 设置默认测试帧源MAC地址 */
int mn_ethLoopCheckMacDefaultSet()
{
	/* modified by xieshl 20091216, 这里禁止直接从eeprom中读sysmac地址 */
	if( MAC_ADDR_IS_ZERO(SYS_PRODUCT_BASEMAC) )
	{
		Enet_MACAdd_Infor1 sys_mac;
		if( funReadMacAddFromNvram(&sys_mac) != NULL )
		{
			VOS_MemCpy( chk_config_frame_smac, sys_mac.bMacAdd, ETH_LOOP_MAC_ADDR_LEN );
		}
	}
	else
	{
		VOS_MemCpy( chk_config_frame_smac, SYS_PRODUCT_BASEMAC, ETH_LOOP_MAC_ADDR_LEN );
	}
	return VOS_OK;
}

/*add by sxh20090311*/
/*设置LINK UP 所需要的时己和最大次数*/
int mn_ethLoopCheckLinkuptimesSet( USHORT uptimes,USHORT uptimes_thresh )
{
	chk_config_up_times=uptimes;
 	chk_config_up_times_thresh=uptimes_thresh;
	return VOS_OK;
}
/*查询LINK UP 所需要的时己和最大次数*/
int mn_ethLoopCheckLinkuptimesGet( USHORT *uptimes,USHORT *uptimes_thresh )
{
	if( uptimes == NULL ||uptimes_thresh==NULL)
		return VOS_ERROR;
	
	*uptimes=chk_config_up_times;
	*uptimes_thresh=chk_config_up_times_thresh;
	
	return VOS_OK;
}

/*add by shixh20090715*/
/* 查询检测测试帧源MAC地址 */
/*int mn_ethLoopCheckBRASMacGet( UCHAR *pMac)
{
	if( pMac == NULL )
		return VOS_ERROR;
	VOS_MemCpy( pMac, chk_config_frame_BRAS_mac, ETH_LOOP_MAC_ADDR_LEN );
	return VOS_OK;
}*/
#if 0
/* 设置检测测试帧源MAC地址 */
int mn_ethLoopCheckBRASMacSet( UCHAR *pMac )
{
	int j;
	if( pMac == NULL )
		return VOS_ERROR;
	VOS_MemCpy( chk_config_frame_BRAS_mac, pMac, ETH_LOOP_MAC_ADDR_LEN );
	
	return VOS_OK;
}
#endif

#ifdef CHK_BRAS_MAC
/*add by shixh20091112 for问题单9140*/
/*将检测的MAC保存*/
STATUS  mn_checkBRASMacSave(struct  vty *vty,UCHAR  *pMac)
{
	int i;

       for(i=0;i<CHK_BRAS_MAC_MAX;i++)
       {
		/*if(VOS_MemCmp(chk_bras[i],chk_config_frame_BRAS_mac_t,sizeof(chk_config_frame_BRAS_mac_t))==0)*/
		if( MAC_ADDR_IS_ZERO(chk_bras[i]) == 0 )
		{
			VOS_MemCpy(chk_bras[i],pMac,6);
			break;	
		}
        }
	   
	if(i>=CHK_BRAS_MAC_MAX)
	{
		return VOS_ERROR;
	}	
			
	return VOS_OK;
}

/*删除某个MAC*/
STATUS  mn_checkBRASMacDelete(struct vty *vty,UCHAR  *pMac)
{
	int i;
       ulong_t  flag=0;
	   
	for(i=0;i<CHK_BRAS_MAC_MAX;i++)	
       {
		if(VOS_MemCmp(chk_bras[i],pMac,6)==0)
		{
			/*VOS_MemCpy(chk_bras[i],chk_config_frame_BRAS_mac_t,6);*/
			VOS_MemZero( chk_bras[i], 6 );
			flag=1;
			break;	
		}
        }

	if( flag == 1 )
	{
		return  VOS_OK;
	}
	else
	{
		return VOS_ERROR;
	}
}
/*显示保存的BRAS MAC*/
STATUS  mn_checkBRASMacShow(struct vty *vty)
{
	int i,j;

	vty_out(vty,"checked bras mac:\r\n");
	for(i=0;i<CHK_BRAS_MAC_MAX;i++)	
	{
		/*if(VOS_MemCmp(chk_bras[i],chk_config_frame_BRAS_mac_t,6)==0)*/
		if( (chk_bras[i][0] | chk_bras[i][1] | chk_bras[i][2] | chk_bras[i][3] | chk_bras[i][4] | chk_bras[i][5]) == 0 )
		{
			continue;
		}
		else
			{
				for(j=0; j<(ETH_LOOP_MAC_ADDR_LEN -1); j++)
				{
					vty_out(vty, "%02x%02x", chk_bras[i][j],chk_bras[i][j+1]);
					j++;
					if( j != 5 )vty_out(vty,".");
				}
			}
		vty_out(vty,"\r\n");
	}
	
	return VOS_OK;
}
#endif

/* 添加检测端口 */
int mn_ethLoopCheckPortAdd( ULONG slotno, ULONG portno )
{
	/*if( check_slotno_is_illegal(slotno)  )
		return VOS_ERROR;*/
	if( (portno == 0) || (portno > eth_loop_check_port_max[slotno]) )
		return VOS_ERROR;

	chk_config_eth_portlist[slotno][portno] = 1;
	ETHLOOPPORTLISTFLAG = EthLoopPortlistIsDesigned;
	return VOS_OK;
}
/* 删除检测端口 */
int mn_ethLoopCheckPortDel( ULONG slotno, ULONG portno )
{
	/*if( check_slotno_is_illegal(slotno) )
		return VOS_ERROR;*/
	if( (portno == 0) || (portno > eth_loop_check_port_max[slotno]) )
		return VOS_ERROR;

	chk_config_eth_portlist[slotno][portno] = 0;
	ETHLOOPPORTLISTFLAG = EthLoopPortlistIsDesigned;
	return VOS_OK;
}

/* 设置默认检测端口，即所有端口 */
int mn_ethLoopCheckPortDefaultSet()
{
#if 0
	int i, j;
	for( i=1; i<=eth_loop_check_slot_max; i++ )
	{
		for( j=1; j<=eth_loop_check_port_max[i]; j++ )
		{
			/*if( check_slotno_is_illegal(i) )
				continue;*/
			chk_config_eth_portlist[i][j] = 1;
		}
	}
#endif
	ethLoopCheckPortlistDefault();
	return VOS_OK;
}
/*int mn_ethLoopCheckPortAllDel()
{
	VOS_MemZero( chk_config_eth_portlist, sizeof(chk_config_eth_portlist) );
	return VOS_OK;
}*/

/* 检索检测端口 */
/*modefied for 6900 by duzhk 2010.1.27  现在此函数只用在发送oam
   和测试帧两个地方，其他地方使用时请注意检查!!*/
int mn_ethLoopCheckPortGetNext( ULONG slotno, ULONG portno, ULONG *pnext_slotno, ULONG *pnext_portno )
{
	int i,j, Max_Slot = eth_loop_check_slot_max;
	
	if( (pnext_slotno == NULL) || (pnext_portno == NULL) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( (slotno > eth_loop_check_slot_max) || ((slotno == eth_loop_check_slot_max) && (portno >= eth_loop_check_port_max[slotno])) )
	{
		/*sys_console_printf("mn_ethLoopCheckPortGetNext:slotno=%d, portno=%d\r\n", slotno, portno);*/
		return VOS_ERROR;
	}
	i = slotno;
	j = portno + 1;
	if( j > eth_loop_check_port_max[i] )
	{
		j = 1;
		i++;
	}
	if( i == 0 )
		i = 1;

	if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
	{
		if(i <= SYS_LOCAL_MODULE_SLOTNO)
		{
			i = SYS_LOCAL_MODULE_SLOTNO;
			Max_Slot = SYS_LOCAL_MODULE_SLOTNO;
		}
		else
			return VOS_ERROR;
	}
	
	for( ; i<=Max_Slot; i++ )
	{
		if( !check_slotno_is_illegal(i) )
		{
			for( ; j<=eth_loop_check_port_max[i]; j++ )
			{
				if( chk_config_eth_portlist[i][j] )
				{
					*pnext_slotno = i;
					*pnext_portno = j;
					
					return VOS_OK;
				}
			}
			j = 1;
		}
	}
	return VOS_ERROR;
}

/* added by xieshl 20080517 */
/*int ethLoopCheckPortIsExist(ULONG chk_slotno, ULONG chk_portno )
{
	ULONG send_slotno, send_portno;
	ULONG next_slotno, next_portno;
	send_slotno = 0;
	send_portno = 0;
	while( mn_ethLoopCheckPortGetNext(send_slotno, send_portno, &next_slotno, &next_portno ) == VOS_OK )
	{
		if( (next_slotno == chk_slotno) && (next_portno == chk_portno) )
			return VOS_OK;
		send_slotno = next_slotno;
		send_portno = next_portno;
	}
	return VOS_ERROR;
}*/

/*int ethLoopCheckEthBoardIsExist( )
{
	ULONG send_slotno, send_portno;
	ULONG next_slotno, next_portno;
	send_slotno = 0;
	send_portno = 0;
	while( mn_ethLoopCheckPortGetNext(send_slotno, send_portno, &next_slotno, &next_portno ) == VOS_OK )
	{
		if( CHECK_SLOTNO_IS_ETH(next_slotno) )
			return VOS_OK;
		send_slotno = next_slotno;
		send_portno = next_portno;
	}
	return VOS_ERROR;
}*/
/* end 20080517 */

#if 0
int mn_ethLoopCheckResultPrint()
{
	ULONG slotno, portno, onu;
	USHORT vid;
	USHORT count = 0;

	sys_console_printf("\r\n");
	for( slotno=1; slotno<=eth_loop_check_slot_max; slotno++ )
	{
		for( portno=1; portno<=eth_loop_check_port_max[slotno]; portno++ )
		{
			vid = chk_result_eth_port_vid[slotno][portno];
			
			if( CHECK_SLOTNO_IS_ETH(slotno) )
			{
				/* 如果是以太网端口直接上报告警事件 */

				if( loop_port_alarm_get(slotno, portno) == CHK_RESULT_LOOP_ALARM )
				{
					sys_console_printf( "ETH port %d/%d looped in vlan %d\r\n", slotno, portno, vid );
					count++;
				}
			}
			else if( CHECK_SLOTNO_IS_PON(slotno) )
			{
				if( loop_port_alarm_get(slotno, portno) == CHK_RESULT_LOOP_ALARM )
				{
					sys_console_printf( "PON port %d/%d% looped in vlan %d\r\n", slotno, portno, vid );
				}
				for( onu=1; onu<=CHK_PON_ONU_MAX; onu++ )
				{
					if( loop_onu_alarm_get(slotno, portno, onu) == CHK_RESULT_LOOP_ALARM )
					{
						sys_console_printf( "ONU %d/%d/%d looped in vlan %d\r\n", slotno, portno, onu, vid );
						count++;
					}
				}
			}
		}
	}
	sys_console_printf("find looped port/onu count = %d\r\n", count);

	return VOS_OK;
}

int mn_ethLoopCheckResultPrint1()
{
	ULONG slotno, portno, onu;
	USHORT vid;

	for( slotno=1; slotno<=eth_loop_check_slot_max; slotno++ )
	{
		for( portno=1; portno<=eth_loop_check_port_max[slotno]; portno++ )
		{
			vid = chk_result_eth_port_vid[slotno][portno];
			
			if( CHECK_SLOTNO_IS_ETH(slotno) )
			{
				sys_console_printf( "ETH port %d/%d vlan %d val %d(status %d alarm %d)\r\n", slotno, portno, vid, 
					chk_result_eth_port[slotno][portno],
					loop_port_status_get(slotno, portno), loop_port_alarm_get(slotno, portno) );
			}
			else if( CHECK_SLOTNO_IS_PON(slotno) )
			{
				sys_console_printf( "PON port %d/%d vlan %d status %d alarm %d\r\n", slotno, portno, vid,
					loop_port_status_get(slotno, portno), loop_port_alarm_get(slotno, portno) );

				for( onu=1; onu<=CHK_PON_ONU_MAX; onu++ )
				{
					sys_console_printf( "  ONU %d%d/%d status %d alarm %d\r\n", slotno, portno, onu,
						loop_onu_status_get(slotno, portno, onu), loop_onu_alarm_get(slotno, portno, onu) );
				}
			}
		}
	}

	return VOS_OK;
}
#endif
#if 0
/* 显示所有存在环路的端口或ONU */
int oltMode_ethLoopCheckResultShow( struct vty *vty )
{
	ULONG slotno, portno, onu;
	USHORT vid;
	USHORT count = 0;

	vty_out( vty, "\r\n" );
	for( slotno=1; slotno<=eth_loop_check_slot_max; slotno++ )
	{
		for( portno=1; portno<=eth_loop_check_port_max[slotno]; portno++ )
		{
			vid = chk_result_eth_port_vid[slotno][portno];
			
			if( CHECK_SLOTNO_IS_ETH(slotno) )
			{
				/* 如果是以太网端口直接上报告警事件 */

				if( loop_port_alarm_get(slotno, portno) == CHK_RESULT_LOOP_ALARM )
				{
					vty_out( vty, "  eth%d/%d looped in vlan %d\r\n", slotno, portno, vid );
					count++;
				}
			}
			else if( CHECK_SLOTNO_IS_PON(slotno) )
			{
				if( loop_port_alarm_get(slotno, portno) == CHK_RESULT_LOOP_ALARM )
				{
					vty_out( vty, "  pon%d/%d% looped in vlan %d\r\n", slotno, portno, vid );
				}
				for( onu=1; onu<=CHK_PON_ONU_MAX; onu++ )
				{
					if( loop_onu_alarm_get(slotno, portno, onu) == CHK_RESULT_LOOP_ALARM )
					{
						vty_out( vty, "  onu%d/%d/%d looped in vlan %d\r\n", slotno, portno, onu, vid );
						count++;
					}
				}
			}
		}
	}
	vty_out( vty, "--------------------------------\r\n", count );
	vty_out( vty, "find looped counter = %d\r\n\r\n", count );

	return VOS_OK;
}
#endif
ethloop_port_listnode_t * findEthPortFromLoopListByPort( ULONG devIdx, ULONG brdIdx, ULONG portIdx )
{
	ethloop_port_listnode_t *node;
	
	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while( node )
	{
		if( (node->devIdx == devIdx) && (node->brdIdx == brdIdx) && (node->portIdx == portIdx)  )
		{
			VOS_SemGive( ethloop_semid );
			return node;
		}
		if( (node->loopdevIdx == devIdx) && (node->loopbrdIdx == brdIdx) && (node->loopportIdx == portIdx)  )
		{
			VOS_SemGive( ethloop_semid );
			return node;
		}
		node = node->next;
	}
	VOS_SemGive( ethloop_semid );
	return NULL;
}

ethloop_port_listnode_t * findEthPortFromLoopListByPort2( ULONG devIdx, ULONG brdIdx,ULONG portIdx,
				ULONG loopdevIdx, ULONG loopbrdIdx, ULONG loopportIdx)
{
	ethloop_port_listnode_t *node;

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while( node )
	{
		if( (node->devIdx == devIdx)  && (node->brdIdx == brdIdx) && (node->portIdx == portIdx)
			&&(node->loopdevIdx == loopdevIdx) && (node->loopbrdIdx == loopbrdIdx) &&(node->loopportIdx == loopportIdx) )
		{
			VOS_SemGive( ethloop_semid );
			return node;
		}
		if( (node->devIdx == loopdevIdx)  && (node->brdIdx == loopbrdIdx) && (node->portIdx == loopportIdx)
			&&(node->loopdevIdx == devIdx) && (node->loopbrdIdx == brdIdx) &&(node->loopportIdx == portIdx) )
		{
			VOS_SemGive( ethloop_semid );
			return node;
		}
		node = node->next;
	}
	VOS_SemGive( ethloop_semid );
	return NULL;
}
ethloop_port_listnode_t * findEthPortFromLoopListByDev( ULONG devIdx )
{
	ethloop_port_listnode_t *node;

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while( node )
	{
		if( node->devIdx == devIdx )
		{
			VOS_SemGive( ethloop_semid );
			return node;
		}
		node = node->next;
	}
	VOS_SemGive( ethloop_semid );
	return NULL;
}
/*
DEFUN (
    test_loop_show_func,
    test_loop_show_cmd,
    "show eth_port_loop_list",
    DescStringCommonShow
    "eth_port_loop_list\n"
   )
{
	
	ethloop_port_listnode_t *node = eth_port_loop_list;
	while( node )
	{
		sys_console_printf("\r\ndev is %d , brd is %d, port is %d,vid is %d\r\n",node->devIdx,node->brdIdx,node->portIdx,node->vid);
		sys_console_printf("agetime is %d\r\n",node->ageTime);
		sys_console_printf("loopdev is %d , loopbrd is %d, loopport is %d,loopvid is %d\r\n",node->loopdevIdx,node->loopbrdIdx,node->loopportIdx,node->loopvid);
		sys_console_printf("\r\n");
		node = node->next;
	}

	return CMD_SUCCESS;
	
}
*/

int delEthPortFromLoopList( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG vid,ULONG loopdevIdx,  ULONG loopbrdIdx, ULONG loopportIdx, ULONG loopvid,USHORT ClearFlag)
{
	int delFlag = 0;
	ethloop_port_listnode_t *node;
	ethloop_port_listnode_t *node_prev = NULL;

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while( node )
	{
		if(((node->devIdx == devIdx) && (node->brdIdx == brdIdx) && (node->portIdx == portIdx)
			&& (node->loopdevIdx == loopdevIdx) && (node->loopbrdIdx == loopbrdIdx)&&(node->loopportIdx == loopportIdx)
			&&(node->vid == vid) &&(node->loopvid == loopvid)) 
			||
			((node->devIdx == devIdx) && (node->brdIdx == brdIdx) && (node->portIdx == portIdx)
			&& (node->loopdevIdx == loopdevIdx) && (node->loopbrdIdx == loopbrdIdx)&&(node->loopportIdx == loopportIdx)
			&&(node->vid == loopvid) &&(node->loopvid == vid))
			|| 
			((node->devIdx == loopdevIdx) && (node->brdIdx == loopbrdIdx) && (node->portIdx == loopportIdx)
			&& (node->loopdevIdx == devIdx) && (node->loopbrdIdx == brdIdx)&&(node->loopportIdx == portIdx)
			&&(node->vid == vid) &&(node->loopvid == loopvid))
			|| 
			((node->devIdx == loopdevIdx) && (node->brdIdx == loopbrdIdx) && (node->portIdx == loopportIdx)
			&& (node->loopdevIdx == devIdx) && (node->loopbrdIdx == brdIdx)&&(node->loopportIdx == portIdx)
			&&(node->vid == loopvid) &&(node->loopvid == vid))
		)
		{
			if( node == eth_port_loop_list )
			{
				eth_port_loop_list = node->next;
				node_prev = NULL;
			}
			else
			{
				if( node_prev == NULL )
					node_prev = eth_port_loop_list;
				else
					node_prev->next = node->next;
			}
			delFlag = 1;
			VOS_Free( node );

			if( node_prev == NULL )
				node = eth_port_loop_list;
			else
				node = node_prev->next;
		}
		else
		{
			node_prev = node;
			node = node->next;
		}
	}
	VOS_SemGive( ethloop_semid );
	if( delFlag ==1 && ClearFlag == ClearTheEntry)
	{
		if( findEthPortFromLoopListByPort(devIdx, brdIdx, portIdx) == NULL )  /* 问题单13740 */
			ethLoopAlarmClear_EventReport( devIdx, brdIdx, portIdx );
	}

	return VOS_OK;
}

int addEthPortToLoopList2( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG vid,ULONG loopdevIdx,  
				ULONG loopbrdIdx, ULONG loopportIdx, ULONG loopvid, UCHAR OtherOltFlag, UCHAR OltType, UCHAR  *loopmac)
{
	ethloop_port_listnode_t *node;
	ethloop_port_listnode_t *node_prev;
	USHORT TestFlag = 0;
	if( chk_config_detect_enable != TRUE )
		return VOS_OK;

	/*if( findEthPortFromLoopListByPort2(devIdx,brdIdx,portIdx, loopdevIdx, loopbrdIdx, loopportIdx) == NULL )
	{
		ethLoopAlarm_EventReport( devIdx, brdIdx,portIdx);
	}*/
	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	node_prev = eth_port_loop_list;
	while( node )
	{
		if((node->devIdx == devIdx) && (node->brdIdx == brdIdx) && (node->portIdx == portIdx)
			&& (node->loopdevIdx == loopdevIdx) && (node->loopbrdIdx == loopbrdIdx)&&(node->loopportIdx == loopportIdx)
			&&(node->vid == vid) &&(node->loopvid == loopvid))
		{
			
			node->ageTime = ETH_LOOP_AGING_TIME;
			VOS_SemGive( ethloop_semid );
			return VOS_OK;
		}
              if((node->devIdx == devIdx) && (node->brdIdx == brdIdx) && (node->portIdx == portIdx)
			&& (node->loopdevIdx == loopdevIdx) && (node->loopbrdIdx == loopbrdIdx)&&(node->loopportIdx == loopportIdx)
			&&(node->vid == loopvid) &&(node->loopvid == vid))
		{
			
			node->ageTime = ETH_LOOP_AGING_TIME;
			VOS_SemGive( ethloop_semid );
			return VOS_OK;
		}
              if((node->devIdx == loopdevIdx) && (node->brdIdx == loopbrdIdx) && (node->portIdx == loopportIdx)
			&& (node->loopdevIdx == devIdx) && (node->loopbrdIdx == brdIdx)&&(node->loopportIdx == portIdx)
			&&(node->vid == vid) &&(node->loopvid == loopvid))
		{
			
			node->ageTime = ETH_LOOP_AGING_TIME;
			VOS_SemGive( ethloop_semid );
			return VOS_OK;
		}
              if((node->devIdx == loopdevIdx) && (node->brdIdx == loopbrdIdx) && (node->portIdx == loopportIdx)
			&& (node->loopdevIdx == devIdx) && (node->loopbrdIdx == brdIdx)&&(node->loopportIdx == portIdx)
			&&(node->vid == loopvid) &&(node->loopvid == vid))
		{
			
			node->ageTime = ETH_LOOP_AGING_TIME;
			VOS_SemGive( ethloop_semid );
			return VOS_OK;
		}

		if( ((node->devIdx == devIdx) && (node->brdIdx == brdIdx) && (node->portIdx == portIdx)) )
		{
			TestFlag = 1;
		}
		/*	((node->devIdx == loopdevIdx) && (node->brdIdx == loopbrdIdx) && (node->portIdx == loopportIdx))
			( (node->loopdevIdx == loopdevIdx) && (node->loopbrdIdx == loopbrdIdx)&&(node->loopportIdx == loopportIdx))
			( (node->loopdevIdx == devIdx) && (node->loopbrdIdx == brdIdx)&&(node->loopportIdx == portIdx) ) */
        			  
		node_prev = node;
		node = node->next;
	}
	node = VOS_Malloc( sizeof(ethloop_port_listnode_t), MODULE_LOOPBACK );

	if( node == NULL )
	{
		VOS_ASSERT(0);
		VOS_SemGive( ethloop_semid );
		return VOS_ERROR;
	}
	VOS_MemZero(node,sizeof(ethloop_port_listnode_t));
	node->devIdx = devIdx;
	node->brdIdx = brdIdx;
	node->portIdx = portIdx;
	node->vid = vid;
	node->ageTime = ETH_LOOP_REPORT_TIME;
	node->next = NULL;
	if(loopbrdIdx == 0 && loopportIdx == 0 && loopdevIdx == 0)
		node->flag = 0;
	else
		node->flag = 1;
	node->loopbrdIdx =  loopbrdIdx;
	node->loopportIdx = loopportIdx;
	node->loopdevIdx = loopdevIdx;
	node->loopvid = loopvid;

	if( 1 == OtherOltFlag)
	{
		node->OtherOltFlag = OtherOltFlag;
		node->OltTypeFlag = OltType;
		if( loopmac != NULL )
			VOS_MemCpy( node->loopmac, loopmac , ETH_LOOP_MAC_ADDR_LEN );
	}
	
	if( eth_port_loop_list == NULL )
	{
		eth_port_loop_list = node;
	}
	else if( node_prev == NULL )
	{
		VOS_ASSERT(0);
		VOS_Free( node );
		VOS_SemGive( ethloop_semid );
		return VOS_ERROR;
	}

	if(node_prev != NULL)
		node_prev->next = node;

	VOS_SemGive( ethloop_semid );

	if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
	{
		if(TestFlag != 1)
			ethLoopAlarm_EventReport( devIdx, brdIdx,portIdx);
	}
	else if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER && node->devIdx != 1)
	{
		onuEthPortLoopReportToMaster(LOOP_ONU_REPORT_ENTRY, node, NoThingToDo );
	}
	else if(SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON)
	{
		onuEthPortLoopReportToMaster(LOOP_12EPON_ETH_LOOP, node, NoThingToDo );
	}
	
	return VOS_OK;
}

int addEthPortToLoopList_Switch( ULONG devIdx, ULONG slotIdx, ULONG portIdx, ULONG SwitchPort, UCHAR  *SwitchMac)
{
	ethloop_listnode_Switch_t*node;
	ethloop_listnode_Switch_t *node_prev;
	
	if( chk_config_detect_enable != TRUE )
		return VOS_OK;
	
	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_loop_list_switch;
	node_prev = eth_loop_list_switch;
	while( node )
	{
              if((node->devIdx == devIdx) && (node->slotIdx == slotIdx) && (node->portIdx == portIdx )
			&& (node->SwitchPort == SwitchPort) )
		{
			VOS_SemGive( ethloop_semid );
			return VOS_ERROR;
		}

		node_prev = node;
		node = node->next;
	}
	node = VOS_Malloc( sizeof(ethloop_listnode_Switch_t), MODULE_LOOPBACK );

	if( node == NULL )
	{
		VOS_ASSERT(0);
		VOS_SemGive( ethloop_semid );
		return VOS_ERROR;
	}
	VOS_MemZero(node,sizeof(ethloop_listnode_Switch_t));
	node->devIdx = devIdx;
	node->slotIdx = slotIdx;
	node->portIdx = portIdx;
	node->SwitchPort = SwitchPort;
	VOS_MemCpy( node->SwitchMac, SwitchMac, ETH_LOOP_MAC_ADDR_LEN);
	node->next = NULL;
	
	if( eth_loop_list_switch == NULL )
	{
		eth_loop_list_switch = node;
	}
	else if( node_prev == NULL )
	{
		VOS_ASSERT(0);
		VOS_Free( node );
		VOS_SemGive( ethloop_semid );
		return VOS_ERROR;
	}

	if(node_prev != NULL)
		node_prev->next = node;

	VOS_SemGive( ethloop_semid );

	if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER )
	{
		onuEthPortLoopReportToMaster(LOOP_ONU_SWITCH_REPORT, node, NoThingToDo );
	}
	else
	{
		SwitchEthPortLoop_EventReport( devIdx, slotIdx, portIdx, SwitchPort, SwitchMac);
	}
	
	return VOS_OK;
}
/* DelFlag  3:delete by switch port, 2: delete by onu port , 1: delete by devIdx */
int DelFromLoopListByDelFlag( ULONG devIdx, ULONG slotIdx, ULONG portIdx, ULONG SwitchPort, uchar_t  SwitchMac[6], int DelFlag )
{
	int delFlag = 0;
	ethloop_listnode_Switch_t *node;
	ethloop_listnode_Switch_t *node_prev = NULL;

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_loop_list_switch;
	while( node )
	{
		if( (node->devIdx == devIdx) && (DelFlag <  2 ||node->slotIdx == slotIdx) 
			&& ( DelFlag <  2 || node->portIdx == portIdx) && ( DelFlag < 3 || node->SwitchPort == SwitchPort)  )
		{
			if( node == eth_loop_list_switch )
			{
				eth_loop_list_switch = node->next;
				node_prev = NULL;
			}
			else
			{
				if( node_prev == NULL )
					node_prev = eth_loop_list_switch;
				else
					node_prev->next = node->next;
			}

			LOOP_CHK_DEBUG(1,(LOG_LOOP_CHK,LOG_WARNING,"onu %d/%d/%d eth-port %d/%d switch %d loop clear\r\n",
			GET_PONSLOT(node->devIdx), GET_PONPORT(node->devIdx), GET_ONUID(node->devIdx),node->slotIdx, node->portIdx,node->SwitchPort));

			if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER )
			{
				onuEthPortLoopReportToMaster(LOOP_ONU_SWITCH_CLEAR, node, NoThingToDo );
			}
			else
			{
				SwitchEthPortLoop_EventReportClear(node->devIdx, node->slotIdx, node->portIdx, node->SwitchPort, node->SwitchMac );
			}
			
			VOS_Free( node );

			if( node_prev == NULL )
				node = eth_loop_list_switch;
			else
				node = node_prev->next;
		}
		else
		{
			node_prev = node;
			node = node->next;
		}
	}
	VOS_SemGive( ethloop_semid );

	return VOS_OK;

}

#if 0
int addEthPortToLoopList( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG vid )
{
	ethloop_port_listnode_t *node = eth_port_loop_list;
	ethloop_port_listnode_t *node_prev = eth_port_loop_list;
	int slot, port;
	
	if( chk_config_detect_enable != TRUE )
		return VOS_OK;
		
	if( findEthPortFromLoopListByPort(devIdx, brdIdx, portIdx) == NULL )
		ethLoopAlarm_EventReport( devIdx, brdIdx, portIdx );
	
	if(devIdx!=1)
	{
		slot  = devIdx / 10000;/*add by shixh20100310，解决无法删除PON MAC地址问题*/
		port = (devIdx % 10000 ) / 1000;
		
		check_mac_entry_delete_by_pon( slot, port);
	}
	
	while( node )
	{
		if( (node->devIdx == devIdx) && (node->brdIdx == brdIdx) && (node->portIdx == portIdx) /*&&
			(node->vid == vid)*/ )
		{
			/*if( node->ageTime < ETH_LOOP_AGING_TIME )*/	/*modified by xieshl 20080627, 老化计数，用于解决告警恢复问题 */
				/*node->ageTime++;*/
			node->ageTime = ETH_LOOP_AGING_TIME;
			/*sys_console_printf("test age++=%d\r\n",node->ageTime);*//*问题单9313*/
			return VOS_OK;
		}
		node_prev = node;
		node = node->next;
	}
	
	node = VOS_Malloc( sizeof(ethloop_port_listnode_t), MODULE_LOOPBACK );
	if( node == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	node->devIdx = devIdx;
	node->brdIdx = brdIdx;
	node->portIdx = portIdx;
	node->vid = vid;
	node->ageTime = ETH_LOOP_REPORT_TIME;
	node->next = NULL;

	if( eth_port_loop_list == NULL )
	{
		eth_port_loop_list = node;
	}
	else if( node_prev == NULL )
	{
		VOS_ASSERT(0);
		VOS_Free( node );
		return VOS_ERROR;
	}
	node_prev->next = node;

	return VOS_OK;
}
#endif

/*
int addEthPortToLoopListNew( ushort_t ponId, ulong_t llId, eventOnuEthLoopMsg_t *pOamMsg )
{

    int rc = VOS_OK;
    ethloop_port_listnode_t *node = eth_port_loop_list;
	ethloop_port_listnode_t *node_prev = eth_port_loop_list;
	int slot, port,vid;
    	ULONG brdIdx=0, portIdx=0,onuDevIdx=0;
	USHORT loopport= 0,loopbroad = 0, loopslot=0,loopllid= 0,loopdevIdx = 0;
	USHORT loopportlist1 = 0,loopportlist2 = 0;

	loopslot = (pOamMsg->onuLocal>>16)&0x0000000f;
	loopbroad =  (pOamMsg->onuLocal>>8)&0x0000000f;
	loopllid = (pOamMsg->onuLocal)&0x0000000f;
	vid = pOamMsg->vid;
	if(loopllid > 0)
	{
		loopport = GetOnuIdxByLlid(ponId,loopllid); 
		loopport = loopport +1;
	}
	else
		loopport = 0;

	loopdevIdx = loopslot*10000+loopbroad*1000+loopport;

	if(pOamMsg->onuPortList != 0)
	{
		loopportlist1 =  (pOamMsg->onuPortList>>8)&0x0000000f;
		loopportlist2 = (pOamMsg->onuPortList)&0x0000000f;
	}
	
	onuDevIdx = onuIdToOnuIndex( ponId, llId );

	brdIdx = (pOamMsg->portId.slot <= 1)?1:pOamMsg->portId.slot;
    	portIdx=pOamMsg->portId.port;

	
	
	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: ETH loop detecting status of pon_id=%d onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}
 
	
	if( chk_config_detect_enable != TRUE )
		return VOS_OK;
	if(loopslot == 1)
	{
		if( findEthPortFromLoopListByPort2(onuDevIdx, brdIdx,portIdx,1,loopslot,loopbroad) == NULL )
			ethLoopAlarm_EventReport( onuDevIdx, brdIdx,portIdx );
	}
	else
	{
		if( findEthPortFromLoopListByPort2(onuDevIdx, brdIdx,portIdx,loopdevIdx,0,0) == NULL )
			ethLoopAlarm_EventReport( onuDevIdx, brdIdx,portIdx );
	}
	
        while( node )
    	{
    		if( (node->devIdx == onuDevIdx) && (node->brdIdx == brdIdx) && (node->portIdx == portIdx)
    		{
    			node->ageTime = ETH_LOOP_AGING_TIME;
    			return VOS_OK;
    		}
    		node_prev = node;
    		node = node->next;
    	}

        node = VOS_Malloc( sizeof(ethloop_port_listnode_t), MODULE_LOOPBACK );
	    if( node == NULL )
    	{
    		VOS_ASSERT(0);
    		return VOS_ERROR;
    	}
	VOS_MemZero(node,sizeof(ethloop_port_listnode_t));
    	node->devIdx = onuDevIdx;
    	node->brdIdx = brdIdx;
    	node->portIdx = portIdx;
    	node->vid = pOamMsg->vid;
    	node->ageTime = ETH_LOOP_REPORT_TIME;
    	node->next = NULL;
	node->flag = 1;
	
	if(loopslot == 1)
	{
		node->loopdevIdx = 1;
		node->loopbrdIdx =  loopslot;
		node->loopportIdx = loopbroad ;
	}
	else
	{
		node->loopdevIdx = loopslot*10000+loopbroad*1000+loopport;
		node->loopbrdIdx = loopportlist1 ;
		node->loopportIdx = loopportlist2;
	}
	
    	if( eth_port_loop_list == NULL )
    	{
    		eth_port_loop_list = node;
    	}
    	else if( node_prev == NULL )
    	{
    		VOS_ASSERT(0);
    		VOS_Free( node );
    		return VOS_ERROR;
    	}
    	node_prev->next = node;
       

    #if  0
	if(onuDevIdx!=1)
	{
		slot  = onuDevIdx / 10000;
		port = (onuDevIdx % 10000 ) / 1000;
		
		check_mac_entry_delete_by_pon( slot, port);
	}
    #endif

	return VOS_OK;
}
*/
int delEthPortFromLoopListByVid( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG vid )
{
#if 0	/* removed by xieshl 20081017, 这样处理存在数据冲突 的风险 */
	int delFlag = 0;
	ethloop_port_listnode_t *node = eth_port_loop_list;
	ethloop_port_listnode_t *node_prev = eth_port_loop_list;

	while( node )
	{
		if( (node->devIdx == devIdx) && (node->brdIdx == brdIdx) && (node->portIdx == portIdx) &&
			(node->vid == vid) )
		{
			if( node->ageTime > 1 )	/* modified by xieshl 20081017, 不直接删除，按老化处理 */
				node->ageTime--;
			else
			{
				if( node == eth_port_loop_list )
				{
					eth_port_loop_list = node->next;
				}
				else
				{
					if( node_prev == NULL )
					{
						VOS_ASSERT(0);
						return VOS_ERROR;
					}
					node_prev->next = node->next;
				}
				delFlag = 1;
				VOS_Free( node );
			}
			break;
		}
		node_prev = node;
		node = node->next;
	}

	if( delFlag )
	{
		if( findEthPortFromLoopListByPort(devIdx, brdIdx, portIdx) == NULL )
			ethLoopAlarmClear_EventReport( devIdx, brdIdx, portIdx );
	}
#endif

	if( chk_config_detect_enable == FALSE )
		return VOS_OK;

	/* modified by xieshl 20080519, 处理linkdown事件时先判断是否已环回，防止大量linkdown事件造成队列溢出 */
	if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONU )
	{
		/*if( VOS_QueNum(ethLoopChkQueId) < ETH_LOOP_CHK_QUE_LEN_MAX )*/
		{
			ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_PORT_LOOPCLEAR, 0, 0};
			aulMsg[2] = devIdx;
			aulMsg[3] = (((brdIdx & 0xff) << 24) | ((portIdx & 0xff) << 16) |(vid & 0xffff) );
			if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
			{
				loopChkDbgPrintf( 1, ("LPCHK: tEthChk task is busy\r\n") );
			}
		}	
	}
	return VOS_OK;
}

int delEthPortFromLoopListByPort( ULONG devIdx, ULONG brdIdx, ULONG portIdx )
{
	int delFlag = 0;
	ethloop_port_listnode_t *node;
	ethloop_port_listnode_t *node_prev = NULL;

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while( node )
	{
		if( (node->devIdx == devIdx) && (node->brdIdx == brdIdx) && (node->portIdx == portIdx)  )
		{
			if( node == eth_port_loop_list )
			{
				eth_port_loop_list = node->next;
				node_prev = NULL;
			}
			else
			{
				if( node_prev == NULL )
					node_prev = eth_port_loop_list;
				else
					node_prev->next = node->next;
			}

			LOOP_CHK_DEBUG(1,(LOG_LOOP_CHK,LOG_WARNING,"eth-port %d/%d loop clear in vlan %d\r\n",
			node->brdIdx, node->portIdx, node->vid));
			
			if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE &&
				(SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON) )
			{
				/*VOS_ASSERT(0);*/
				/* 只有主控板上接受linkdown/linkup 的消息处理通知链情景下这个case不应在过来了2012-05-31 */
				onuEthPortLoopReportToMaster(LOOP_12EPON_ETH_CLEAR, node, ClearTheEntry);
				/*12EPON 上的上联口的告警现在先由主控删除时来上报告警 kkkkkkkk  2012-04-10*/
			}
			else
			{
				 /*此处的处理是以下面条件为基础的，如有变动应该相应变化: 
				只有主控板上接受linkdown/linkup 的消息处理通知链2012-05-31*/
				if( devIdx == 1 && brdIdx > PRODUCT_MAX_TOTAL_SLOTNUM)  
				{
					VOS_ASSERT(0);
					break;
				}
				
				if( SYS_LOCAL_MODULE_ISMASTERACTIVE  && devIdx == 1
					&& SYS_MODULE_IS_UPLINK_PON( brdIdx ) && portIdx <= 4  )
				{
					EthPortLoop_DelNodeTo12Epon(brdIdx, node);
					if(node->loopdevIdx == 1 && SYS_MODULE_IS_UPLINK_PON( node->loopbrdIdx ) 
						&& node->loopportIdx <= 4  )
						EthPortLoop_DelNodeTo12Epon(node->loopbrdIdx, node);
				}
				if( node != NULL )
				{
					VOS_Free( node );
					node = NULL;
				}
				if( findEthPortFromLoopListByPort(devIdx, brdIdx, portIdx) == NULL ) 
					ethLoopAlarmClear_EventReport( devIdx, brdIdx, portIdx );
			}
			
			if( node != NULL )
			{
			VOS_Free( node );
				node = NULL;
			}
			if( node_prev == NULL )
				node = eth_port_loop_list;
			else
				node = node_prev->next;
		}
		else
		{
			node_prev = node;
			node = node->next;
		}
	}
	VOS_SemGive( ethloop_semid );

	return VOS_OK;

}

int EthPortLoop_DelNodeTo12Epon( int slotno, ethloop_port_listnode_t *node )
{
	unsigned int ulLen = 0 ;
    	SYS_MSG_S      *pMsg       = NULL;
	if( SYS_LOCAL_MODULE_SLOTNO == slotno )
		return VOS_OK;
	
	ulLen=sizeof(ethloop_port_listnode_t)+sizeof(SYS_MSG_S);
	pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_LOOPBACK);

	if( NULL == pMsg )
	{
	     VOS_ASSERT(0);
	     return  VOS_ERROR;
	}

	VOS_MemZero((CHAR *)pMsg, ulLen );

        SYS_MSG_SRC_ID( pMsg )       = MODULE_LOOPBACK;
	 SYS_MSG_DST_ID( pMsg )       = MODULE_LOOPBACK;
	 SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
	 SYS_MSG_FRAME_LEN( pMsg )    = sizeof(LoopInsertCDPMsgHead_t);
	 SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
	 SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
	 SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
	 SYS_MSG_DST_SLOT( pMsg ) = SYS_MASTER_ACTIVE_SLOTNO;
	 SYS_MSG_MSG_CODE(pMsg) = LOOP_NOTIFY_12EPON_ETH_CLEAR ;
	 VOS_MemCpy((void *)(pMsg + 1), (void *)node, sizeof(ethloop_port_listnode_t));

	if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_ETHLOOP, slotno, RPU_TID_CDP_ETHLOOP, /*CDP_MSG_TM_ASYNC*/ 0,\
	          (VOID *)pMsg, ulLen, MODULE_LOOPBACK ) )
	{
		VOS_ASSERT(0); 
		sys_console_printf("\r\n%%Send the loop-detection configuration to the new poncard Failed !\r\n");
		CDP_FreeMsg(pMsg);
		return VOS_ERROR;
	}
}
int delEthPortFromLoopListByOnu( ULONG devIdx )
{
	ethloop_port_listnode_t *node;
	ethloop_port_listnode_t *node_prev = NULL;
	ULONG brdIdx = 0, portIdx = 0;
	UCHAR portmap[64] = {0};
	ULONG portcount = 0;
	
	VOS_MemZero( portmap, sizeof(portmap) );
	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while( node )
	{
		if( node->devIdx == devIdx  )
		{
			if( node == eth_port_loop_list )
			{
				eth_port_loop_list = node->next;
				node_prev = NULL;
			}
			else
			{
				if( node_prev == NULL )
					node_prev = eth_port_loop_list;
				else
					node_prev->next = node->next;
			}
			brdIdx = node->brdIdx;
			portIdx = node->portIdx;
			if( portIdx < 64 )
			{
				portmap[portIdx] = 1;
				portcount++;
			}

			LOOP_CHK_DEBUG(1,(LOG_LOOP_CHK,LOG_WARNING,"onu %d/%d/%d eth-port %d/%d loop clear in vlan %d\r\n",
			GET_PONSLOT(node->devIdx), GET_PONPORT(node->devIdx), GET_ONUID(node->devIdx), node->brdIdx, node->portIdx,node->vid));
			
			if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
			{
				if(portmap[portIdx] == 1)
				{
					onuEthPortLoopReportToMaster(LOOP_ONU_CLEAR_ENTRY,node,ClearTheEntry );
					portmap[portIdx] = 2;
				}
				else
					onuEthPortLoopReportToMaster(LOOP_ONU_CLEAR_ENTRY,node,NotClearEntry );
			}
			
			VOS_Free( node );

			if( node_prev == NULL )
				node = eth_port_loop_list;
			else
				node = node_prev->next;
		}
		else
		{
			node_prev = node;
			node = node->next;
		}
		
	}
	VOS_SemGive( ethloop_semid );

	brdIdx = 1;
	if( portcount )
	{
		if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
		{
			for( portIdx=1; portIdx<64; portIdx++ )
			{
				if( portmap[portIdx] )
				{
					ethLoopAlarmClear_EventReport( devIdx, brdIdx, portIdx );
				}
			}
		}
		check_mac_entry_delete_by_pon( GET_PONSLOT(devIdx)/*devIdx / 10000*/, GET_PONPORT(devIdx)/*(devIdx % 10000 ) /1000*/);
	}
	return VOS_OK;
}

int delPonPortFromLoopListByOnu( short int olt_id )
{
	ethloop_port_listnode_t *node;
	ethloop_port_listnode_t *node_prev = NULL;
	ULONG brdIdx, portIdx;
	int slotno,portno,ponslot,ponport;
	UCHAR portmap[64];
	ULONG portcount = 0;
	
	VOS_MemZero( portmap, sizeof(portmap) );
	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;

	slotno = GetCardIdxByPonChip(olt_id);
	portno = GetPonPortByPonChip(olt_id);
	
	while( node )
	{
		ponslot = GET_PONSLOT(node->devIdx);
		ponport = GET_PONPORT(node->devIdx);
		if( ponslot == slotno &&  ponport == portno )
		{
			if( node == eth_port_loop_list )
			{
				eth_port_loop_list = node->next;
				node_prev = NULL;
			}
			else
			{
				if( node_prev == NULL )
					node_prev = eth_port_loop_list;
				else
					node_prev->next = node->next;
			}
			brdIdx = node->brdIdx;
			portIdx = node->portIdx;
			if( portIdx < 64 )
			{
				portmap[portIdx]++;    
				portcount++;
			}

			if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
			{
				if(portmap[portIdx] == 1)
					onuEthPortLoopReportToMaster(LOOP_ONU_CLEAR_ENTRY,node,ClearTheEntry );
				else
					onuEthPortLoopReportToMaster(LOOP_ONU_CLEAR_ENTRY,node,NotClearEntry );
			}
			
			VOS_Free( node );

			if( node_prev == NULL )
				node = eth_port_loop_list;
			else
				node = node_prev->next;
		}
		else
		{
			node_prev = node;
			node = node->next;
		}
		
	}
	VOS_SemGive( ethloop_semid );

	return VOS_OK;
}

/*暂时此函数只在主控上检测到板拔出时使用，别的地方用注意*/
int delEthPortFromLoopListBySlot(LONG slotno )
{
	ethloop_port_listnode_t *node;
	ethloop_port_listnode_t *node_prev = NULL;
	ULONG brdIdx, portIdx;

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while( node )
	{
		if(node->devIdx != 1 && slotno == GET_PONSLOT(node->devIdx)/*node->devIdx/10000*/)
		{
			if( node == eth_port_loop_list )
			{
				eth_port_loop_list = node->next;
				node_prev = NULL;
			}
			else
			{
				if( node_prev == NULL )
					node_prev = eth_port_loop_list;
				else
					node_prev->next = node->next;
			}
			brdIdx = node->brdIdx;
			portIdx = node->portIdx;
			
			if( findEthPortFromLoopListByPort(node->devIdx, brdIdx, portIdx) == NULL )  
				ethLoopAlarmClear_EventReport( node->devIdx, brdIdx, portIdx ); 
			
			VOS_Free( node );

			if( node_prev == NULL )
				node = eth_port_loop_list;
			else
				node = node_prev->next;
		}
		else
		{
			node_prev = node;
			node = node->next;
		}
	}
	VOS_SemGive( ethloop_semid );
	return VOS_OK;
}

ULONG EthLoopSwitchOverCallback(devsm_switchhover_notifier_event ulEvent )
{
	ethloop_port_listnode_t *node;

	if(ulEvent != swtichhover_notify_end)
		return VOS_ERROR;
	if( !SYS_LOCAL_MODULE_WORKMODE_ISSLAVE )
		return VOS_ERROR;
	
	if(!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
		return VOS_ERROR;

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while( node )
	{
		onuEthPortLoopReportToMaster(LOOP_ONU_REPORT_ENTRY, node, NoThingToDo );
		node = node->next;
	}
	VOS_SemGive( ethloop_semid );

	return VOS_OK;
}

int resetOnuPortLoopList()
{
	ethloop_port_listnode_t *node, *node_temp = NULL, *node_prev;
	ULONG devIdx, brdIdx, portIdx;
	ethloop_listnode_Switch_t *node2;
	ethloop_listnode_Switch_t *node_prev2 = NULL;
	
	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while( node )
	{
		devIdx = node->devIdx;
		brdIdx = node->brdIdx;
		portIdx = node->portIdx;

		if((__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW)||
		   (__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW))
		{
			if( devIdx == 1 && SYS_MODULE_IS_UPLINK_PON( brdIdx ) && portIdx <= 4)
			{
				node_prev = node;
				node = node->next;
				continue;
			}
			
			if( devIdx != 1 )
			{
				node_prev = node;
				node = node->next;
				continue;
			}
		}

		if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
		{
			if(SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON)
			{
				if( devIdx == 1 && portIdx > 4 )
				{
					node_prev = node;
					node = node->next;
					continue;
				}
			}
			else
			{
				if( devIdx == 1 )
				{
					node_prev = node;
					node = node->next;
					continue;
				}
			}
		}
	
		
		if( node == eth_port_loop_list )
		{
			eth_port_loop_list = node->next;
			node_prev = NULL;
		}
		else
		{
			if( node_prev == NULL )
				node_prev = eth_port_loop_list;
			else
				node_prev->next = node->next;
		}

		node_temp = 	findEthPortFromLoopListByPort(node->devIdx,node->brdIdx,node->portIdx);

		if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
		{
			if(node_temp == NULL)
			{
				if( (SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON) && ( devIdx == 1 && portIdx <= 4 )  )
				{
					/* 只有主控板上接受linkdown/linkup 的消息处理通知链情景下这个case不应在过来了2012-05-31 */
					/*VOS_ASSERT( 0 );*/
					onuEthPortLoopReportToMaster(LOOP_12EPON_ETH_CLEAR, node, ClearTheEntry);
				}
				else
					onuEthPortLoopReportToMaster(LOOP_ONU_CLEAR_ENTRY, node, ClearTheEntry);
			}
			else
			{
				if( (SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON) && ( devIdx == 1 && portIdx <= 4 )  )
				{
					/* 只有主控板上接受linkdown/linkup 的消息处理通知链情景下这个case不应在过来了2012-05-31 */
					/*VOS_ASSERT( 0 );*/
					onuEthPortLoopReportToMaster(LOOP_12EPON_ETH_CLEAR, node, NotClearEntry);
				}
				else
					onuEthPortLoopReportToMaster(LOOP_ONU_CLEAR_ENTRY, node, NotClearEntry);
			}
		}

		/*12EPON 上的上联口的告警现在先由主控删除时来上报告警 kkkkkkkk */
		
		if( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && findEthPortFromLoopListByPort(devIdx, brdIdx, portIdx) == NULL)
		{
			if(node_temp == NULL)
				ethLoopAlarmClear_EventReport( devIdx, brdIdx, portIdx );
		}
		
		VOS_Free( node );
		
		if( node_prev == NULL )
			node = eth_port_loop_list;
		else
			node = node_prev->next;
	}
		
	VOS_SemGive( ethloop_semid );

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node2 = eth_loop_list_switch;
	while( node2 )
	{
		/*(if( (node2->devIdx == devIdx) && (DelFlag <  2 ||node2->slotIdx == slotIdx) 
			&& ( DelFlag <  2 || node2->portIdx == portIdx) && ( DelFlag < 3 || node2->SwitchPort == SwitchPort)  )*/
		{
			if( node2 == eth_loop_list_switch )
			{
				eth_loop_list_switch = node2->next;
				node_prev2 = NULL;
			}
			else
			{
				if( node_prev2 == NULL )
					node_prev2 = eth_loop_list_switch;
				else
					node_prev2->next = node2->next;
			}

			if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER )
			{
				onuEthPortLoopReportToMaster(LOOP_ONU_SWITCH_CLEAR, node2, NoThingToDo );
			}
			else
			{
				SwitchEthPortLoop_EventReportClear(node2->devIdx, node2->slotIdx, node2->portIdx, node2->SwitchPort, node2->SwitchMac );
			}
			
			VOS_Free( node2 );

			if( node_prev2 == NULL )
				node2 = eth_loop_list_switch;
			else
				node2 = node_prev2->next;
		}
		/*else
		{
			node_prev2 = node2;
			node2 = node2->next;
		}*/
	}
	VOS_SemGive( ethloop_semid );
	return VOS_OK;
}

/* 端口linkdown事件回调函数，在告警模块调用 */
int check_eth_port_linkdown_callback( ULONG devIdx, ULONG brdIdx, ULONG portIdx )
{
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_PORT_LINKDOWN, 0, 0};

	if( chk_config_detect_enable == FALSE )
		return 0;

	/* modified by xieshl 20080519, 处理linkdown事件时先判断是否已环回，防止大量linkdown事件造成队列溢出 */
	if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY )
	{
		VOS_ASSERT(0);    
		return VOS_ERROR;			 /*6900上暂时不支持ONLY OLT 模式，duzhk 2011-01-20*/ 
		
		/* modified by xieshl 20080513, 在only-olt模式下，linkdown事件只对OLT侧端口有效，由于ONU无法识别环路端口，暂不处理 */
		if( devIdx == 1 )
		{
			if( loop_port_status_get(brdIdx, portIdx) == CHK_RESULT_LOOP_NULL )
				return 0;
		}
		else
			return 0;
	}
	else
	{
		if( findEthPortFromLoopListByPort(devIdx, brdIdx, portIdx) == NULL )
			return 0;
		else
		{
			if( devIdx != 1 )
				return 1;
		}
	}

	/* OLT侧GE口没有环路控制，在linkdown时应做处理，同时上报该事件 */
	/*if( VOS_QueNum(ethLoopChkQueId) < ETH_LOOP_CHK_QUE_LEN_MAX )*/
	{
		aulMsg[2] = devIdx;
		aulMsg[3] = (((brdIdx & 0xff) << 8) | (portIdx & 0xff));
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			/*VOS_ASSERT(0);*/
			loopChkDbgPrintf( 1, ("LPCHK: tEthChk task is busy\r\n") );
		}
	}
	return 0;
}

/* added by xieshl 20081020, 处理linkdown事件时先判断是否已环回，防止大量linkup事件造成队列溢出 */
int check_eth_port_linkup_callback( ULONG devIdx, ULONG brdIdx, ULONG portIdx )
{
	if( chk_config_detect_enable )
	{
		if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY )
		{
		}
		else
		{
			if( findEthPortFromLoopListByPort(devIdx, brdIdx, portIdx) != NULL )
			{
				if( devIdx != 1 )
					return 1;
			}
		}
	}
	return 0;
}

/* ONU离线或掉电事件回调函数，在告警模块调用 */
VOID check_onu_notpresent_callback( ULONG devIdx )
{
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_ONU_LINEOFF, 0, 0};
	
	if( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
		return ;
	
	if( chk_config_detect_enable == FALSE )
		return;
	if( devIdx == 1 )
		return;

	/* modified by xieshl 20080519, 处理ONU离线事件时先判断是否有环回，防止大量离线事件造成队列溢出 */
	if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY )
	{
		/* 等待老化，不做处理 */
		VOS_ASSERT(0);    
		return;			 /*6900上暂时不支持ONLY OLT 模式，duzhk 2011-01-20*/ 
	}
	else
	{
		if( findEthPortFromLoopListByDev(devIdx) == NULL )
			return;
	}
	
	/*if( VOS_QueNum(ethLoopChkQueId) < ETH_LOOP_CHK_QUE_LEN_MAX )*/
	{
		aulMsg[2] = devIdx;
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			/*VOS_ASSERT(0);*/
			loopChkDbgPrintf( 1, ("LPCHK: tEthChk task is busy\r\n") );
		}
	}
}

extern STATUS getDeviceName( const ulong_t devIdx, char* pValBuf, ulong_t *pValLen );
char * ethLoopDeviceNameGet( ULONG devIdx, char *name )
{
	ULONG len;
	char tmp[16];
	if( getDeviceName( devIdx, name, &len) == VOS_OK )
	{
		if( len > 32 ) len = 32;
		name[len] = 0;
	}
	else
	{
		if( devIdx == 1 )
			VOS_StrCpy( name, "OLT" );
		else
			VOS_StrCpy( name, "ONU" );
	}

	if( devIdx != 1 )
	{
		VOS_Sprintf( tmp, "(%d/%d/%d)", GET_PONSLOT(devIdx)/*devIdx/10000*/, GET_PONPORT(devIdx)/*(devIdx%10000)/1000*/, GET_ONUID(devIdx)/*devIdx%1000*/ );
		VOS_StrCat( name, tmp );
	}
	return name;	
}

/* 显示所有存在环路的端口或ONU */
int onuMode_ethLoopCheckResultShow( struct vty *vty )
{
/*	ULONG slotno, portno;
	USHORT vid;*/
	char devName[256], OltTypeDifference = 0;
	char loopdevName[256];
	USHORT count = 0,VlanId = 0, CurrentSystemType = 0;
	ethloop_port_listnode_t *node = NULL;
	ethloop_listnode_Switch_t *node2 = NULL ;
	UCHAR type[16], DifferType[16], temp[32], Mac[16];

	vty_out( vty, "\r\n" );

	VOS_MemZero(type, 16);
	VOS_MemZero(temp, 32);
	VOS_MemZero(Mac , 32);
#if 0
	for( slotno=1; slotno<=eth_loop_check_slot_max; slotno++ )
	{
		if( !CHECK_SLOTNO_IS_ETH(slotno) )
			continue;
			
		for( portno=1; portno<=eth_loop_check_port_max[slotno]; portno++ )
		{
			vid = chk_result_eth_port_vid[slotno][portno];
				
			if( CHECK_SLOTNO_IS_ETH(slotno) )
			{
				/* 如果是以太网端口直接上报告警事件 */

				if( loop_port_alarm_get(slotno, portno) == CHK_RESULT_LOOP_ALARM )
				{
					vty_out( vty, "  eth%d/%d looped in vlan %d\r\n", slotno, portno, vid );
					count++;
				}
			}
		}
	}
#endif	
	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while( node )
	{
		devName[0] = '-';
		devName[1] = '\0';

		CurrentSystemType =GetOltType();
		
		if( node->OtherOltFlag == 1)
		{
			if( node->OltTypeFlag == 1 || node->OltTypeFlag == 0x61)
				VOS_MemCpy( type, "GFA6100", 7 );
			else if( node->OltTypeFlag == 3 || node->OltTypeFlag == 0x69)
				VOS_MemCpy( type, "GFA6900", 7 );
			else if( node->OltTypeFlag == 4 || node->OltTypeFlag == 0x80)
				VOS_MemCpy( type, "GFA8000", 7 );
			else if( node->OltTypeFlag == 5 || node->OltTypeFlag == 0x81)
				VOS_MemCpy( type, "GFA8100", 7 );
			else
				VOS_MemCpy( type, "GFA6700", 7 );

			VOS_MemCpy( Mac , node->loopmac , 6 );

			if( node->loopdevIdx == OLT_DEV_ID )
			{
				if( (node->OltTypeFlag == 2 || node->OltTypeFlag == 0x67) && CurrentSystemType != V2R1_OLT_GFA6700 )
				{
					OltTypeDifference = 1;
					VOS_MemCpy( DifferType, "GFA6700", 7 );
				}
				else if( (node->OltTypeFlag == 1 || node->OltTypeFlag == 0x61) && CurrentSystemType != V2R1_OLT_GFA6100 )
				{
					OltTypeDifference = 1;
					VOS_MemCpy( DifferType, "GFA6100", 7 );
				}
				else if( (node->OltTypeFlag == 3 || node->OltTypeFlag == 0x69) && CurrentSystemType != V2R1_OLT_GFA6900 )
				{
					OltTypeDifference = 1;
					VOS_MemCpy( DifferType, "GFA6900", 7 );
				}
				else if( (node->OltTypeFlag == 4 || node->OltTypeFlag == 0x80) && CurrentSystemType != V2R1_OLT_GFA8000 )
				{
					OltTypeDifference = 1;
					VOS_MemCpy( DifferType, "GFA8000", 7 );
				}
				else if( (node->OltTypeFlag == 5 || node->OltTypeFlag == 0x81) && CurrentSystemType != V2R1_OLT_GFA8100 )
				{
					OltTypeDifference = 1;
					VOS_MemCpy( DifferType, "GFA8100", 7 );
				}
			}
			
		}
		
		if(node->flag == 1)
		{
		       if(node->vid == node->loopvid || node->vid == 0 || node->loopvid == 0)
                    {      
                            if(node->vid == node->loopvid || node->vid == 0)
                            {
                                    VlanId = node->loopvid;
                            }
                            else
                            {
                                    VlanId = node->vid;
                            }
        			if(OLT_DEV_ID == node->loopdevIdx || node->loopbrdIdx !=0)
        			{
        				if( OLT_DEV_ID == node->loopdevIdx /*&& SYS_MODULE_IS_PON( node->loopbrdIdx )*/ )  /*问题单11978 */
        				{
        					if( OLT_DEV_ID == node->devIdx && CHECK_SLOTNO_IS_PON( node->brdIdx ) && SYS_MODULE_WORKMODE_ISSLAVE( node->brdIdx ) )
        						VOS_MemCpy( temp, "pon-port", 8 );
						else
							VOS_MemCpy( temp, "eth-port", 8 );

						if( node->OtherOltFlag == 1)
						{
							if( OltTypeDifference == 1)
								vty_out( vty, "  %s %s %d/%d looped with : %s eth-port %d/%d (%s: %02x%02x.%02x%02x.%02x%02x) in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), temp, node->brdIdx, node->portIdx, 
	      	 							DifferType,node->loopbrdIdx,node->loopportIdx, type, Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5], VlanId );
							else
								vty_out( vty, "  %s %s %d/%d looped with : %s eth-port %d/%d (%s: %02x%02x.%02x%02x.%02x%02x) in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), temp, node->brdIdx, node->portIdx, 
		      	 						ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx, type, Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5], VlanId );
						}
						else
							vty_out( vty, "  %s %s %d/%d looped with : %s eth-port %d/%d in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), temp, node->brdIdx, node->portIdx, 
      	 							ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx, VlanId );
        				}
					else
					{
						if( OLT_DEV_ID == node->devIdx && CHECK_SLOTNO_IS_PON( node->brdIdx ) && SYS_MODULE_WORKMODE_ISSLAVE( node->brdIdx ) )
						{
							if( node->OtherOltFlag == 1)
								vty_out( vty, "  %s pon-port %d/%d looped with : %s eth-port %d/%d (%s: %02x%02x.%02x%02x.%02x%02x) in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, 
       		 						ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx, type, Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5], VlanId );
							else
								vty_out( vty, "  %s pon-port %d/%d looped with : %s eth-port %d/%d in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, 
       		 						ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx, VlanId );
						}
						else
						{
							if( node->OtherOltFlag == 1)
								vty_out( vty, "  %s eth-port %d/%d looped with : %s eth-port %d/%d (%s: %02x%02x.%02x%02x.%02x%02x) in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, 
       		 						ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx, type, Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5], VlanId );
							else
		        					vty_out( vty, "  %s eth-port %d/%d looped with : %s eth-port %d/%d in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, 
       		 						ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx, VlanId );
						}
					}
        			}
        			else
        			{
        				if( node->OtherOltFlag == 1)
						vty_out( vty, "  %s eth-port %d/%d looped with : %s (%s: %02x%02x.%02x%02x.%02x%02x) in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, 
        						ethLoopDeviceNameGet(node->loopdevIdx, loopdevName), type, Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5], VlanId );
					else
					{
						if( ( node->loopbrdIdx == 0 ) || ( node->loopportIdx == 0))
						{
							vty_out( vty, "  %s eth-port %d/%d looped with : %s in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, 
       	 					ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),VlanId );
						}
						else
						{
							vty_out( vty, "  %s eth-port %d/%d looped with : %s eth-port %d/%d in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, 
       	 					ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx,VlanId );
						}
					}
        			}
                     }
                     else
                     {
                            if( OLT_DEV_ID == node->loopdevIdx || node->loopbrdIdx !=0)
        			{
        				if( OLT_DEV_ID == node->loopdevIdx /*&& SYS_MODULE_IS_PON( node->loopbrdIdx )*/ )  /*问题单11978 */
        				{
        					if( OLT_DEV_ID == node->devIdx && CHECK_SLOTNO_IS_PON( node->brdIdx ) && SYS_MODULE_WORKMODE_ISSLAVE( node->brdIdx ) )
        						VOS_MemCpy( temp, "pon-port", 8 );
						else
							VOS_MemCpy( temp, "eth-port", 8 );
						
        					if( node->OtherOltFlag == 1)
        					{
        						if( OltTypeDifference == 1 )
								vty_out( vty, "  %s %s %d/%d (Vid: %d ) looped with : %s eth-port %d/%d (%s: %02x%02x.%02x%02x.%02x%02x) (Vid: %d)\r\n", ethLoopDeviceNameGet(node->devIdx, devName), temp, node->brdIdx, node->portIdx, node->vid,
       		 						DifferType,node->loopbrdIdx,node->loopportIdx, type, Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5], node->loopvid);
							else
								vty_out( vty, "  %s %s %d/%d (Vid: %d ) looped with : %s eth-port %d/%d (%s: %02x%02x.%02x%02x.%02x%02x) (Vid: %d)\r\n", ethLoopDeviceNameGet(node->devIdx, devName), temp, node->brdIdx, node->portIdx, node->vid,
       		 						ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx, type, Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5], node->loopvid);
        					}
						else
							vty_out( vty, "  %s %s %d/%d (Vid: %d ) looped with : %s eth-port %d/%d (Vid: %d)\r\n", ethLoopDeviceNameGet(node->devIdx, devName), temp, node->brdIdx, node->portIdx, node->vid,
       	 						ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx, node->loopvid);
        				}
					else
					{
						if( OLT_DEV_ID == node->devIdx && CHECK_SLOTNO_IS_PON( node->brdIdx ) && SYS_MODULE_WORKMODE_ISSLAVE( node->brdIdx ) )
						{
							if( node->OtherOltFlag == 1)
								vty_out( vty, "  %s pon-port %d/%d (Vid: %d ) looped with : %s (%s: %02x%02x.%02x%02x.%02x%02x) eth-port %d/%d (Vid: %d)\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, node->vid,
        								ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx, type, Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5], node->loopvid);
							else
								vty_out( vty, "  %s pon-port %d/%d (Vid: %d ) looped with : %s eth-port %d/%d (Vid: %d)\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, node->vid,
        								ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx, node->loopvid);
						}
						else
						{
							if( node->OtherOltFlag == 1)
								vty_out( vty, "  %s eth-port %d/%d (Vid: %d ) looped with : %s eth-port %d/%d (%s: %02x%02x.%02x%02x.%02x%02x) (Vid: %d)\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, node->vid,
        								ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx, type, Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5], node->loopvid);
							else
	        						vty_out( vty, "  %s eth-port %d/%d (Vid: %d ) looped with : %s eth-port %d/%d (Vid: %d)\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, node->vid,
       	 							ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx, node->loopvid);
						}
					}
        			}
        			else
        			{
        				if( node->OtherOltFlag == 1)
						vty_out( vty, "  %s eth-port %d/%d (Vid: %d ) looped with : %s (%s: %02x%02x.%02x%02x.%02x%02x) (Vid: %d )\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, node->vid,
       	 					ethLoopDeviceNameGet(node->loopdevIdx, loopdevName), type, Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5], node->loopvid );
					else
					{
						if( ( node->loopbrdIdx == 0 ) || ( node->loopportIdx == 0))
						{
							vty_out( vty, "  %s eth-port %d/%d looped with : %s in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, 
       	 					ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),VlanId );
						}
						else
						{
							vty_out( vty, "  %s eth-port %d/%d looped with : %s eth-port %d/%d in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, 
       	 					ethLoopDeviceNameGet(node->loopdevIdx, loopdevName),node->loopbrdIdx,node->loopportIdx,VlanId );
						}
					}
        			}
                     }
		}
		else
		{
			if(node->devIdx == OLT_DEV_ID)/*问题单: 11978*/
			{
				if(SYS_MODULE_IS_UPLINK(node->brdIdx))
					vty_out( vty, "  %s eth-port %d/%d looped in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, node->vid );
				else
					vty_out( vty, "  %s pon-port %d/%d looped in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, node->vid );
			}
			else
			{
				vty_out( vty, "  %s eth-port %d/%d looped in vlan %d\r\n", ethLoopDeviceNameGet(node->devIdx, devName), node->brdIdx, node->portIdx, node->vid );
			}
		}
		count++;
		node = node->next;
	}

	node2 = eth_loop_list_switch;
	
	while( node2 )
	{
		vty_out( vty, "  %s eth-port %d/%d  switch %02x%02x.%02x%02x.%02x%02x port %d onu-switch loop alarm\r\n", 
			ethLoopDeviceNameGet(node2->devIdx, devName), node2->slotIdx, node2->portIdx, node2->SwitchMac[0], 
			node2->SwitchMac[1], node2->SwitchMac[2], node2->SwitchMac[3], node2->SwitchMac[4], node2->SwitchMac[5], node2->SwitchPort);
		node2 = node2->next;
		count++;
	}
	
	VOS_SemGive( ethloop_semid );
	
	vty_out( vty, "-----------------------------------------------------------------------\r\n", count );
	vty_out( vty, "find looped counter = %d\r\n\r\n", count );

	return VOS_OK;
}
	
#ifdef CHK_BRAS_MAC
DEFUN(check_BRAS_mac_enable,
	  check_BRAS_mac_enable_cmd,
         "check-bras-mac [enable|disable]",
	  "check BRAS MAC enable\n"
	  "set check BRAS MAC enable\n"
	  "check BRAS MAC enable disable\n"
	  )
{

	
	if( VOS_StrCmp(argv[0], "enable") == 0 )
	{
		chk_BRAS_mac_enable = TRUE;
		vty_out(vty,"check BRAS mac  is  enable!\r\n");
	}
	else if( VOS_StrCmp(argv[0], "disable") == 0 )
		{
			chk_BRAS_mac_enable = FALSE;
			vty_out(vty,"check BRAS mac is disable!\r\n");
		}
	else{
			vty_out(vty, "  %% param err\r\n");
			return( CMD_WARNING );
		}
	
	
	return CMD_SUCCESS;
}
DEFUN(show_check_BRAS_mac_enable,
	  show_check_BRAS_mac_enable_cmd,
         "show check-bras-mac enable",
	  "show  information\n"
	  "show check BRAS MAC information\n"
	  "show check BRAS MAC enable information\n"
	  )
{
	if(chk_BRAS_mac_enable==TRUE)
		vty_out(vty,"check BRAS mac  is  enable!\r\n");
	else 
		vty_out(vty,"check BRAS mac is disable!\r\n");	
	
	return CMD_SUCCESS;
}
extern LONG    Qos_Get_Mac_Address_By_Str(CHAR * ucMacAddr, CHAR * szStr);
DEFUN(BRAS_MAC_detection,
		  BRAS_MAC_detection_cmd,
	         "bras-mac-detection <H.H.H>",
		  "set BRAS MAC address\n"
		  "set MAC address\n"
	  	)
{
	 char  pmac[6]={0};
	 
	 Qos_Get_Mac_Address_By_Str(pmac,argv[0]);
	
	
	if(mn_checkBRASMacSave(vty,pmac)!=VOS_OK)
	{
		vty_out(vty,"you already config 16 mac,please delete some mac!\r\n");
	}
		
	return CMD_SUCCESS;
}
DEFUN(delete_BRAS_MAC_detection,
		  delete_BRAS_MAC_detection_cmd,
	         "undo bras-mac-detection <H.H.H>",
	         "delete mac\n"
		  "delete BRAS MAC address\n"
		  "delete MAC address\n"
	  	)
{
	char  pmac[6]={0};
	 
	 Qos_Get_Mac_Address_By_Str(pmac,argv[0]);
		
	if(mn_checkBRASMacDelete(vty,pmac)!=VOS_OK)
	{
		vty_out(vty,"no this mac!\r\n");
	}
	else
	{
		vty_out(vty,"delete MAC success!\r\n");
	}
	
	return CMD_SUCCESS;
}

DEFUN(show_BRAS_MAC_detection,
		  show_BRAS_MAC_detection_cmd,
	         "show bras-mac-detection mac",
	         "show information \n"
		  "show BRAS MAC address\n"
		  "show check BRAS MAC address\n"
	  	)
{
	if(mn_checkBRASMacShow(vty)!=VOS_OK)
	{
		vty_out(vty,"error!\r\n");
	}
	
	return CMD_SUCCESS;
}
#endif
/*
                "loop-detection {[only-olt]}*1 [enable|disable]",
		  "set loop detection only by olt\n"
modified by duzhk for screening the only-olt mode
*/

int getEthLoopEnable()
{
    int ret = VOS_ERROR;
    ONU_EVENT_CONF_SEM_TAKE
    {
        ret = chk_config_detect_enable;
    }
    ONU_EVENT_CONF_SEM_GIVE
    return ret;
}

void setOnuEthLoopEnable(int enable)
{
    /*
    ONU_EVENT_CONF_SEM_TAKE
    {
        chk_config_detect_enable = enable;
    }
    ONU_EVENT_CONF_SEM_GIVE
     */

    /*
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
    {
        CTCONU_ConfAlarmData(ETH_PORT_LOOPBACK, 0, enable, 0, 0);
    }
    */
}
DEFUN(loop_detection_enable,
		  loop_detection_enable_cmd,
	         "loop-detection [enable|disable]",
		  "set loop detection\n"
		  "loop detection enable\n"
		  "loop detection disable\n"
	  	)
{
	int i;
	static qdef_ethchk_cmd_t chkcmd;
	char *loopEblStr[] = {"disable","enable"};
	ULONG enable = FALSE;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};

	if(argc==1)
	{
		if( VOS_StriCmp(argv[0], loopEblStr[1])==0 )
			enable = TRUE;
	}
	else if( argc == 2 )
	{
		if( VOS_StriCmp(argv[1], loopEblStr[1])==0 )
			enable = TRUE;
	}

	/* modified by xieshl 20080530, 在使能打开的情况下，不允许修改检测模式，这里要给出提示
	信息，问题单#6749 */
	if( enable == getEthLoopEnable() )
	{
		vty_out(vty, "The loop detection function has been %s!\r\n", loopEblStr[enable] );
		return CMD_SUCCESS;
	}

	eth_loop_chk_start_flag = enable;
	
	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );

	chkcmd.argc = argc;
	chkcmd.vty = vty;
	chkcmd.cmdId = ETHCHK_ENABLE_CMD;
	for( i=0; i<argc; i++ )
	{
		VOS_StrnCpy( (char *)chkcmd.argv[i], argv[i], 64 );
	}
	aulMsg[3] = (ULONG)&chkcmd;

	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
		vty_out(vty,"Send the command to Pon card successed !\r\n");
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}

	return CMD_SUCCESS;
}

LONG ethLoopCheckEnableCommand( qdef_ethchk_cmd_t *pchkcmd )
{
	char *loopEblStr = "enable";
	ULONG enable = FALSE;
	ULONG mode = ETH_LOOP_CHECK_MODE_OLT_ONU;

	if(pchkcmd->argc==1)
	{
		if( VOS_StriCmp(pchkcmd->argv[0], loopEblStr)==0 )
			enable = TRUE;
		
		mode = ETH_LOOP_CHECK_MODE_OLT_ONU;
	}
	else if( pchkcmd->argc == 2 )
	{
		if( VOS_StriCmp(pchkcmd->argv[1], loopEblStr)==0 )
			enable = TRUE;
		mode = ETH_LOOP_CHECK_MODE_OLT_ONLY;
	}
	/*else
	{
		vty_out(pchkcmd->vty, "the numbers of param erorr!\r\n");
		return CMD_WARNING;
	}*/

	/*if( chk_config_detect_enable == TRUE )
	{
		if( mode != chk_config_detect_mode )
		{
			vty_out(pchkcmd->vty, "loop detection disable first, please!!\r\n");
			return CMD_WARNING;
		}
	}*/

	if( enable == TRUE )
	{
		if( chk_config_detect_enable == FALSE )
		{
			if(mn_ethLoopCheckEnable() == VOS_ERROR)
				vty_out(pchkcmd->vty, "loop detection error\r\n");
		}
	}
	else
	{
		if( chk_config_detect_enable == TRUE )
		{
			if( mn_ethLoopCheckDisable() == VOS_ERROR )
				vty_out( pchkcmd->vty, "loop detection error\r\n" );
			if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY )
			{
			}
			else
			{
				/*ULONG all_portlist[VLAN_SLOT_MAX];
				ULONG untag_portlist[VLAN_SLOT_MAX];

				VOS_MemZero( untag_portlist, sizeof(untag_portlist) );
				VOS_MemZero( all_portlist, sizeof(all_portlist) );*/
				chk_config_detect_enable = FALSE;
				if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
				{
					ethLoopCheckTestOamSend( chk_config_eth_vid, 0, 0 );
					VOS_TaskDelay(10);
					resetOnuPortLoopList();
					ethLoopCheckTestOamSend( chk_config_eth_vid, 0, 0 );
				}
				else
				{
					resetOnuPortLoopList();
				}
			}
		}
	}
	
	chk_config_detect_mode = mode;

	/*if( pMode != NULL )
	{
		if( VOS_StriCmp(pMode, "only-olt")==0)
		{
			chk_config_detect_mode = ETH_LOOP_CHECK_MODE_OLT_ONLY;
		}
		else
		{
			chk_config_detect_mode = ETH_LOOP_CHECK_MODE_OLT_ONU;
		}
	}*/
	
	return   CMD_SUCCESS;
}

LONG ethLoopCheckEnableCommand_Pon( qdef_ethchk_cmd_t *pchkcmd )
{
	char *loopEblStr = "enable";
	ULONG enable = FALSE;
	ULONG mode = ETH_LOOP_CHECK_MODE_OLT_ONU;

	if(pchkcmd->argc==1)
	{
		if( VOS_StriCmp(pchkcmd->argv[0], loopEblStr)==0 )
			enable = TRUE;
		
		mode = ETH_LOOP_CHECK_MODE_OLT_ONU;
	}
	else if( pchkcmd->argc == 2 )
	{
		if( VOS_StriCmp(pchkcmd->argv[1], loopEblStr)==0 )
			enable = TRUE;
		mode = ETH_LOOP_CHECK_MODE_OLT_ONLY;
	}

	if( enable == TRUE )
	{
		if( chk_config_detect_enable == FALSE )
		{
			eth_loop_chk_start_flag	 = TRUE;

			if( SYS_LOCAL_MODULE_ISHAVEPP() && !SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
				setL2uCpu(L2U_INDEX_NUM,1);
			
			if(ethLoopCheckNotifyEvent( ETH_LOOP_CHECK_START ) == VOS_ERROR)
				sys_console_printf("loop detection error\r\n");
		}
	}
	else
	{
		if( chk_config_detect_enable == TRUE )
		{
			eth_loop_chk_start_flag = FALSE;

			if( SYS_LOCAL_MODULE_ISHAVEPP() && !SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
				setL2uCpu(L2U_INDEX_NUM, 0 );
			
			if( ethLoopCheckNotifyEvent( ETH_LOOP_CHECK_STOP ) == VOS_ERROR )
				sys_console_printf("loop detection error\r\n" );
			if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY )
			{
			}
			else
			{
				chk_config_detect_enable = FALSE;

				ethLoopCheckTestOamSend( chk_config_eth_vid, 0, 0 );

				VOS_TaskDelay(10);

				resetOnuPortLoopList();
				ethLoopCheckTestOamSend( chk_config_eth_vid, 0, 0 );
			}
		}
	}
	
	chk_config_detect_mode = mode;
	
	return   CMD_SUCCESS;
}


DEFUN(config_loop_detection_vlan,
	  config_loop_detection_vlan_cmd,
         "config loop-detection [vlan] [all]",
	  "config loop detection\n"
	  "set loop detection opteration\n"
	  "set vlan\n"
	  "set detection vlan all\n"
	  )
{
	int i;
	static qdef_ethchk_cmd_t chkcmd;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};

	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = argc;
	chkcmd.vty = vty;
	chkcmd.cmdId = ETHCHK_VLAN_CMD;

	for( i=0; i<argc; i++ )
	{
		VOS_StrnCpy( (char *)chkcmd.argv[i], argv[i], 64 );
	}
	aulMsg[3] = (ULONG)&chkcmd;

	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
		vty_out(vty,"Send the command to Pon card successed !\r\n");
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}
	
	return CMD_SUCCESS;
}

static int Creat_VlanList_String( ULONG VlanId, UCHAR *portlist_str, int size, ULONG  *Startvalue, 
			ULONG  *Endvalue, int *Coun, bool LastFlag  )
{
	ULONG  Svalue = 0 , Evalue = 0;
	int  Flag = 0 ;
	UCHAR tmp[16];
	int counter;
	
	Svalue = *Startvalue;
	Evalue = *Endvalue;
	counter = *Coun;
	 	
	if(Svalue == 0)
	{
		Svalue = Evalue = VlanId;
	}

	if(counter != 1)
	{
		if(Evalue != VlanId-1)
			Flag =1;
		else
			Evalue = VlanId;
	}

	if( Flag == 1 )
	{
		if(counter == 2 )
		{
			VOS_Sprintf( tmp, "%s%d", ((portlist_str[0] == 0) ? "" : ","),Svalue );
			VOS_StrCat( portlist_str, tmp );
		}
		else
		{
			VOS_Sprintf( tmp, "%s%d-%d", ((portlist_str[0] == 0) ? "" : ","),Svalue, Evalue);
			VOS_StrCat( portlist_str, tmp );
		}
		Svalue = Evalue = VlanId ;
		counter = 1 ;
		Flag = 0 ;
	}
	
	if( TRUE == LastFlag )
	{
		if(counter == 1 )
		{
			VOS_Sprintf( tmp, "%s%d", ((portlist_str[0] == 0) ? "" : ","),Svalue );
			VOS_StrCat( portlist_str, tmp );
		}
		else
		{
			VOS_Sprintf( tmp, "%s%d-%d", ((portlist_str[0] == 0) ? "" : ","),Svalue, Evalue);
			VOS_StrCat( portlist_str, tmp );
		}
		counter = 1;
	}

	*Startvalue =  Svalue;
	*Endvalue = Evalue ;
	*Coun = counter;
	
	return VOS_OK;
	
}

DEFUN(loop_detection_immediately,
	  loop_detection_immediately_cmd,
         "loop-detection vlan <1-4094> port <portlist>",
	  "set loop detection\n"
	  "set loop detection vlan\n"
	  "input vlan\n"
	  "set loop detection port\n"
	  "input ports\n"
	  )
{
	static qdef_ethchk_cmd_t chkcmd;
	ULONG ulIfIndex = 0, ulSlot = 0, ulPort = 0,send_vid = 0,aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};
	LONG lRet = 0,Length = 0,portnum = 0,portlist[128] = {0};
	int i = 0;
	
	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = argc;
	chkcmd.vty = vty; 
	chkcmd.cmdId = ETHCHK_IMMEDIATELY_CMD;
	Length = VOS_StrLen( argv[1] );
	send_vid = VOS_AtoI( argv[0] );

	if(send_vid < 1 || send_vid > 4094)
	{
		vty_out(vty," You have added error vlan, please check it !\r\n");
		return VOS_ERROR;
	}
	
	if(Length >= 128 )
	{
		vty_out(vty," You have added too many ports, please check it !\r\n");
		return VOS_ERROR;
	}

	if(argc < 2)
	{
		vty_out( vty, "Input para error!!\r\n");	
		return VOS_ERROR;
	}

	for( i=0; i<argc; i++ )
	{
		VOS_StrnCpy( (char *)chkcmd.argv[i], argv[i], 64 );
	}

	VOS_MemZero(portlist, 128);
	VOS_StrnCpy( portlist, chkcmd.argv[1], 128 );	  
	
	aulMsg[3] = (ULONG)&chkcmd;

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX( portlist, ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
		{
			vty_out( vty, "  %% Parameter error.\r\n");
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}				

		if (ulSlot > CHK_SLOT_MAX)
		{
			vty_out( vty, "  %% Parameter error. Slot must be 1-14 !\r\n");
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}

		portnum = Slot_Get_PortNum(ulSlot);

		if ((ulPort > portnum ) || (ulPort < 1))
		{
			vty_out( vty, "  %% Parameter error.Slot %d port should be 1-%d\r\n", ulSlot, portnum );
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_IFINDEX()
	
	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
		vty_out(vty,"Send the command to Pon card successed !\r\n");
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}
	
	return CMD_SUCCESS;
}

LONG ethLoopCheckImmediatelyCommand( qdef_ethchk_cmd_t *pchkcmd )
{
	ULONG ulIfIndex = 0, ulSlot = 0, ulPort = 0,send_vid = 0;
	LONG lRet = 0,portnum = 0,portlist[128];
	VOS_MemZero(portlist, 128);
	VOS_StrnCpy( portlist, pchkcmd->argv[1], 128 );	
	send_vid = VOS_AtoI( pchkcmd->argv[0] );
	
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX( portlist, ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
		{
			vty_out( pchkcmd->vty, "  %% Parameter error.\r\n");
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}				

		if (ulSlot > CHK_SLOT_MAX)
		{
			vty_out( pchkcmd->vty, "  %% Parameter error. Slot must be 1-14 !\r\n");
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}

		portnum = Slot_Get_PortNum(ulSlot);

		if ((ulPort > portnum ) || (ulPort < 1))
		{
			vty_out( pchkcmd->vty, "  %% Parameter error.Slot %d port should be 1-%d\r\n", ulSlot, portnum );
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}

		if(SYS_MODULE_IS_UPLINK(ulSlot) && SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
		{
			ethLoopCheckTestFrameSendByVlanAndPort(send_vid ,ulSlot ,ulPort);
		}
		else if(SYS_MODULE_IS_UPLINK_PON(ulSlot) && (ulPort < 5) && SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
		{
			ethLoopCheckTestFrameSendByVlanAndPort(send_vid ,ulSlot ,ulPort);
		}
		else if(SYS_MODULE_IS_PON(ulSlot) && SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
		{
			check_oam_frame_send_by_port( ulSlot, ulPort, send_vid, 0 );
		}
	}
	END_PARSE_PORT_LIST_TO_IFINDEX()

	return CMD_SUCCESS;
}


DEFUN(config_loop_detection_vlan2,
	  config_loop_detection_vlan_cmd2,
         "config loop-detection [vlan] [add|del] <vlanlist>",
	  "config loop detection\n"
	  "set loop detection opteration\n"
	  "set vlan\n"
	  "add detection vlan list\n"
	  "del detection vlan list\n"
	  "Input the vlanlist\n"
	  )
{
	int i = 0;
	static qdef_ethchk_cmd_t chkcmd;
	ULONG  Startvalue = 0, EndValue = 0, size = 0, vlanmode = 0 ;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};
	ULONG *vlanList = NULL, *vlanList2 = NULL, *vlanAddr = NULL, *IntPoint = NULL ;	
	LONG ret = 0, Length = 0, str[64] = {0}, counter = 0;
	char *portlist_str = NULL, tmp[16] = {0};
	bool LastFlag = FALSE;
	EthLoopVlan_t *EthLoopVlan_Temp = NULL, *EthLoopVlan_Pre = NULL;
	
	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = argc;
	chkcmd.vty = vty;
	chkcmd.cmdId = ETHCHK_VLAN_CMD;
	Length = VOS_StrLen(argv[2]);

	if(Length >= 64 )
	{
		vty_out(vty," The vlanlist is too long !\r\n");
	}
	
	if(argc < 3)
	{
		vty_out( vty, "Input para error!!\r\n");	
		return VOS_ERROR;
	}

	for( i=0; i<argc; i++ )
	{
		VOS_StrnCpy( (char *)chkcmd.argv[i], argv[i], 64 );
	}

	VOS_MemZero(str, 64);
	VOS_StrnCpy( str, chkcmd.argv[2], 64 );	  /*copy 一份，用于检测*/
	
	aulMsg[3] = (ULONG)&chkcmd;

	vlanmode=mn_ethLoopCheckVlanGet();

	if( vlanmode == 0 )
	{
		if( VOS_StriCmp(chkcmd.argv[1], "del") == 0 )
		{
			vty_out(vty," You cann`t delete anyone under the vlan check mode : all .\r\n");
			return CMD_WARNING;
		}
	}

	if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
	{
		vlanList = VOS_Malloc( MAXVlanNum*sizeof(ULONG), MODULE_LOOPBACK);   /* 问题单12884 */
		if( NULL == vlanList)
		{
			VOS_ASSERT(0);
			return CMD_SUCCESS;
		}
		vlanAddr = vlanList2 = vlanList;
		VOS_MemZero(vlanList, MAXVlanNum*sizeof(ULONG));
		
		ret=parseVlanList( str, vlanList2);
		if(VOS_ERROR == ret)
		{
			vty_out( vty, "  %% Input a invalid vlanlist.\r\n" );
			VOS_Free(vlanAddr);
			return VOS_ERROR;
		}
			
		size = 5*2048; 

		portlist_str = VOS_Malloc(size, MODULE_LOOPBACK);
		if ( !portlist_str )
		{
			ASSERT( 0 );
			return VOS_ERROR;
		}

		VOS_MemZero(portlist_str,  size );

		Startvalue = 0;
		EndValue = 0;

		if( VOS_StriCmp(chkcmd.argv[1], "add") == 0 )
		{
			while( *vlanList )
			{
				/*ret = AddEthLoopVlanEntry(*vlanList);*/
				EthLoopVlan_Temp = FindEthLoopVlanEntry( *vlanList, &EthLoopVlan_Pre);

				if(EthLoopVlan_Temp != NULL)
				{
					counter++;
					IntPoint = vlanList;
					IntPoint++;
					if( 0 == (*IntPoint) )
						LastFlag = TRUE ;
					
					Creat_VlanList_String( *vlanList, portlist_str, size, &Startvalue, &EndValue, &counter,LastFlag );
				}

				vlanList++;
				
			}

			if(counter > 0)
			{
				if( counter >= 1 && LastFlag == FALSE )
				{
					if(counter == 1 )
						VOS_Sprintf( tmp, "%s%d", ((portlist_str[0] == 0) ? "" : ","),Startvalue );
					else
						VOS_Sprintf( tmp, "%s%d-%d", ((portlist_str[0] == 0) ? "" : ","),Startvalue, EndValue);
					
					VOS_StrCat( portlist_str, tmp );
				}
				vty_big_out(chkcmd.vty, size, "  The vlanlist is added except %s which has been existed !\r\n",portlist_str);
			}
		}
		else if( VOS_StriCmp(chkcmd.argv[1], "del") == 0 )
		{
			while( *vlanList )
			{
				/*ret = DelEthLoopVlanEntry(*vlanList);*/
				EthLoopVlan_Temp = FindEthLoopVlanEntry( *vlanList,&EthLoopVlan_Pre);

				if(EthLoopVlan_Temp == NULL)
				{
					counter++;
					IntPoint = vlanList;
					IntPoint++;
					if( 0 == (*IntPoint) )
						LastFlag = TRUE ;
					
					Creat_VlanList_String( *vlanList, portlist_str, size, &Startvalue, &EndValue, &counter, LastFlag );
				}
				vlanList++;
			}
			if(counter > 0)
			{
				if( counter >= 1 && LastFlag == FALSE )
				{
					if(counter == 1 )
						VOS_Sprintf( tmp, "%s%d", ((portlist_str[0] == 0) ? "" : ","), Startvalue );
					else
						VOS_Sprintf( tmp, "%s%d-%d", ((portlist_str[0] == 0) ? "" : ","), Startvalue, EndValue);
					
					VOS_StrCat( portlist_str, tmp );
				}
				
				vty_big_out(chkcmd.vty, size, "  The vlanlist is deleted except %s which don`t existed !\r\n",portlist_str);

			}
		}
		
		VOS_Free( portlist_str ) ;
		VOS_Free( vlanAddr );
	}
	
	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
		vty_out(vty,"Send the command to Pon card successed !\r\n");
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}
	
	return CMD_SUCCESS;
}

/*   "config loop-detection [mac] [<H.H.H>|default]", */
DEFUN(config_loop_detection_mac,
	  config_loop_detection_mac_cmd,
         "config loop-detection [mac] [<H.H.H>]",
	  "config loop detection\n"
	  "set loop detection opteration\n"
	  "set mac addr.\n"
	  "input detection mac addr.\n"
	  "set detection default mac addr.\n"
	  )
{
	int i;
	static qdef_ethchk_cmd_t chkcmd;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};
	unsigned char MacAddr[6] = {0,0,0,0,0,0};


	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = argc;
	chkcmd.vty = vty;
	chkcmd.cmdId = ETHCHK_VLAN_CMD;

	for( i=0; i<argc; i++ )
	{
		VOS_StrnCpy( (char *)chkcmd.argv[i], argv[i], 64 );
	}
	aulMsg[3] = (ULONG)&chkcmd;

	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
		vty_out(vty,"Send the command to Pon card successed !\r\n");
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}
	
	return CMD_SUCCESS;
}

DEFUN(loop_detection_syslog_enable,
		  loop_detection_syslog_enable_cmd,
	         "loop-detection-syslog [enable|disable]",
		  "set loop detection syslog\n"
		  "loop detection syslog enable\n"
		  "loop detection syslog disable\n"
	  	)
{
	static qdef_ethchk_cmd_t chkcmd;
	char *loopEblStr[] = {"disable","enable"};
	ULONG enable = FALSE;

	if(argc==1)
	{
		if( VOS_StriCmp(argv[0], loopEblStr[1])==0 )
			enable = TRUE;
	}
	else if( argc == 2 )
	{
		if( VOS_StriCmp(argv[1], loopEblStr[1])==0 )
			enable = TRUE;
	}

	if( enable == loopChk_debug_switch )
	{
		vty_out(vty, "The loop detection syslog has been %s!\r\n", loopEblStr[enable] );
		return CMD_SUCCESS;
	}

	loopChk_debug_switch = enable;
	
	return CMD_SUCCESS;
}

DEFUN(show_loop_detection_syslog,
	  show_loop_detection_syslog_status_cmd,
         "show loop-detection-syslog status",
         "show information\n"
	  "show loop detection syslog\n"
	  "show status data\n"
	  )
{
	if( loopChk_debug_switch == TRUE)
		vty_out(vty, "  Loop-syslog Status: enable \r\n");
	else
		vty_out(vty, "  Loop-syslog Status: disable \r\n");

	return   CMD_SUCCESS;
}

/*
DEFUN(show_loop_detection_mac_clean_enable,
		  show_loop_detection_mac_clean_enable_cmd,		  
		  "show loop-detection-mac-clean ",
		  "show information\n"
		  "show loop detection mac clean control\n"
	  	)
{
	int enable = 0;
	char * loopchkStatus[] = {"error","enable", "disable"};
	enable = mn_ethLoopCheckMacCleanEnableGet();
	vty_out(vty, "  LoopDetection Mac-clean:%s\r\n", loopchkStatus[enable]);

	return   CMD_SUCCESS;
}
*/

/*
DEFUN(loop_detection_mac_clean_enable,
		  loop_detection_mac_clean_enable_cmd,
		  "loop-detection mac-clean [enable|disable]",
		  "config loop detection\n"
		  "set loop detection mac clean control\n"
		  "loop detection mac clean enable\n"
		  "loop detection mac clean disable\n"
	  	)
{
	int i = 0;
	static qdef_ethchk_cmd_t chkcmd;
	char *loopEblStr[] = {"disable","enable"};
	ULONG enable = FALSE;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};

	if(argc==1)
	{
		if( VOS_StriCmp(argv[0], loopEblStr[1])==0 )
			enable = TRUE;
	}
	else if( argc == 2 ) 
	{
		if( VOS_StriCmp(argv[1], loopEblStr[1])==0 )
			enable = TRUE;
	}

	if( enable == chk_config_detect_mac_clean_enable )
	{
		vty_out(vty, "The loop detection mac clean has been %s!\r\n", loopEblStr[enable] );
		return CMD_SUCCESS;
	}

	chk_config_detect_mac_clean_enable = enable;

	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );

	chkcmd.argc = argc;
	chkcmd.vty = vty;
	chkcmd.cmdId = ETHCHK_MAC_CLEAN_ENABLE_CMD;
	for( i=0; i<argc; i++ )
	{
		VOS_StrnCpy( (char *)chkcmd.argv[i], argv[i], 64 );
	}
	aulMsg[3] = (ULONG)&chkcmd;

	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
		vty_out(vty,"Send the command to Pon card successed !\r\n");
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}

	return CMD_SUCCESS;
}
*/

/*
DEFUN(show_chk_config_eth_portlist,
	  show_chk_config_eth_portlist_cmd,
         "show chk_config_eth_portlist\n",
	  "show information\n"
	  "chk_config_eth_portlist\n"
	  )
{
        int i,j;
        sys_console_printf("eth_loop_check_slot_max is %d\r\n",eth_loop_check_slot_max);
	for(i=0 ; i<=eth_loop_check_slot_max; i++ )
	{
	       sys_console_printf("The Slot is %d, Port number is %d\r\n",i,eth_loop_check_port_max[i]);
		for(j=0 ; j<=eth_loop_check_port_max[i]; j++ )
		{
			if( chk_config_eth_portlist[i][j] )
			{
		            sys_console_printf("It is : %d,  %d,  %d\r\n",chk_config_eth_portlist[i][j],i,j);
			}
		}
	}

       return CMD_SUCCESS;
}
*/


int AddEthLoopVlanEntry(int vid)
{
	EthLoopVlan_t *EthLoopVlan_Temp = NULL, *EthLoopVlan_Pre = NULL;
	EthLoopVlan_Temp = FindEthLoopVlanEntry(vid,&EthLoopVlan_Pre);

	if(EthLoopVlan_Temp != NULL)
	{
		return VOS_ERROR;
	}
	else
	{
		EthLoopVlan_Temp = VOS_Malloc(sizeof(EthLoopVlan_t), MODULE_LOOPBACK);
		if(EthLoopVlan_Temp == NULL)
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}
		VOS_MemZero(EthLoopVlan_Temp, sizeof(EthLoopVlan_t));
		EthLoopVlan_Temp->vlanid = vid;
		if(EthLoopVlan_Pre != NULL)
		{
			EthLoopVlan_Temp->next = EthLoopVlan_Pre->next;
			EthLoopVlan_Pre->next = EthLoopVlan_Temp;
		}
		else
		{
			EthLoopVlan_Temp->next = EthLoopVlan_List;
			EthLoopVlan_List = EthLoopVlan_Temp;
		}
	}
	return VOS_OK;
}

int DelEthLoopVlanEntry(int vid)
{
	EthLoopVlan_t *EthLoopVlan_Temp = NULL, *EthLoopVlan_Pre = NULL;
	EthLoopVlan_Temp = FindEthLoopVlanEntry(vid,&EthLoopVlan_Pre);

	if(EthLoopVlan_Temp != NULL )
	{
		if(EthLoopVlan_Pre != NULL)
		{
			EthLoopVlan_Pre->next = EthLoopVlan_Temp->next;
		}
		else
		{
			EthLoopVlan_List = EthLoopVlan_Temp->next;
		}
		VOS_Free(EthLoopVlan_Temp);

		return VOS_OK;
	}
	return VOS_ERROR;
}

int DelEthLoopVlanAll()
{
	EthLoopVlan_t *EthLoopVlan_Temp = EthLoopVlan_List, *EthLoopVlan_Temp2;
	while(EthLoopVlan_Temp != NULL)
	{
		EthLoopVlan_Temp2 = EthLoopVlan_Temp;
		EthLoopVlan_Temp = EthLoopVlan_Temp2->next;
		VOS_Free(EthLoopVlan_Temp2);
	}
	EthLoopVlan_List = NULL;
	return VOS_OK;
}
/*根据vid获取其所在链表的位置*/
EthLoopVlan_t * FindEthLoopVlanEntry(int vid, EthLoopVlan_t ** EthLoopVlan_Pre)
{
	EthLoopVlan_t *EthLoopVlan_Temp;
	EthLoopVlan_Temp = EthLoopVlan_List;
	*EthLoopVlan_Pre = NULL;
	while(EthLoopVlan_Temp)
	{
		if(EthLoopVlan_Temp->vlanid == vid)
		{
			return EthLoopVlan_Temp;
		}
		else if(EthLoopVlan_Temp->vlanid > vid)
		{
			return NULL;/*记下该vid 应该放的位置*/
		}
		else
		{
			*EthLoopVlan_Pre = EthLoopVlan_Temp;
			EthLoopVlan_Temp = EthLoopVlan_Temp->next;
		}
	}

	return NULL;
}

LONG ethLoopCheckVlanCommand( qdef_ethchk_cmd_t *pchkcmd )
{
 	/*ULONG  vlan = 0;*/
	int  i,ret;
	unsigned char MacAddr[6] = {0,0,0,0,0,0};
	ULONG *vlanList, *vlanAddr ;	
	if(pchkcmd->argc==0)
	{
		mn_ethLoopCheckVlanDefaultSet(); 
		mn_ethLoopCheckMacDefaultSet();	  
		vty_out(pchkcmd->vty, "set loop detection default vlan and MAC\r\n"); 
	}
	else
	{
		for(i=0;i<pchkcmd->argc;i++)
		{
			if( VOS_StriCmp(pchkcmd->argv[i], "vlan") == 0 )
			{
				if( VOS_StriCmp( pchkcmd->argv[i+1], "all") == 0)
				{
					mn_ethLoopCheckVlanDefaultSet(); 
					DelEthLoopVlanAll();
				}
				else
				{
					if(pchkcmd->argc < 3)
					{
						vty_out(pchkcmd->vty, "Input para error!!\r\n");	
						return VOS_ERROR;
					}
					
					vlanList = VOS_Malloc( MAXVlanNum*sizeof(ULONG), MODULE_LOOPBACK);
					if( NULL == vlanList)
					{
						VOS_ASSERT(0);
						return CMD_SUCCESS;
					}
					vlanAddr = vlanList;
					VOS_MemZero(vlanList, MAXVlanNum*sizeof(ULONG));
					
					ret=parseVlanList(pchkcmd->argv[i+2],vlanList);
					if(VOS_ERROR == ret)
					{
						vty_out( pchkcmd->vty, "  %% Input a invalid vlanlist.\r\n" );
						VOS_Free(vlanAddr);
						return VOS_ERROR;
					}
					
					if( VOS_StriCmp(pchkcmd->argv[i+1], "add") == 0 )
					{
						chk_config_eth_vid = 1; /*表示现在对上联口是配置指定VLAN 模式*/
						while( *vlanList )
						{
							ret = AddEthLoopVlanEntry(*vlanList);
							if(ret == VOS_ERROR)
							{
							}
							vlanList++;
						}
					}
					else if( VOS_StriCmp(pchkcmd->argv[i+1], "del") == 0 )
					{
						while( *vlanList )
						{
							ret = DelEthLoopVlanEntry(*vlanList);
							if(ret == VOS_ERROR)
							{
							}
							vlanList++;
						}
					}
					VOS_Free( vlanAddr );
				}
				return VOS_OK;
			}
			else if( VOS_StriCmp(pchkcmd->argv[i], "mac") == 0 )
			{
				if( VOS_StriCmp( pchkcmd->argv[i+1], "default") == 0)
				{
					mn_ethLoopCheckMacDefaultSet(); 
				}
				else
				{
					if ( GetMacAddr( ( CHAR* ) pchkcmd->argv[i+1], MacAddr ) != VOS_OK )
					{
						vty_out( pchkcmd->vty, "  %% Invalid MAC address.\r\n" );
					       return CMD_WARNING;
					}
					if(mn_ethLoopCheckMacSet(MacAddr) != VOS_OK)
					{	
						/*vty_out(pchkcmd->vty, "set loopback detection mac is:\r\n"); 
						for(i=0;i<6;i++)
							vty_out(pchkcmd->vty, "%lx-",MacAddr[i]); 
						vty_out(vty, "\r\n"); */
						return CMD_WARNING;
					}
				}
			}
			else
			{
				vty_out(pchkcmd->vty, " para error!!\r\n");	
			}
			i++;           
		}
	}
	return CMD_SUCCESS;
}

LONG ethLoopCheckVlanCommand_Pon( qdef_ethchk_cmd_t *pchkcmd )
{
 	ULONG  vlan = 0;
	int  i;
	unsigned char MacAddr[6] = {0,0,0,0,0,0};
		
	if(pchkcmd->argc==0)
	{
		mn_ethLoopCheckVlanDefaultSet(); 
		mn_ethLoopCheckMacDefaultSet();	  
		sys_console_printf("set loop detection default vlan and MAC\r\n"); 
	}
	else
	{
		for(i=0;i<pchkcmd->argc;i++)
		{
			if( VOS_StriCmp(pchkcmd->argv[i], "vlan") == 0 )
			{
				if( VOS_StriCmp( pchkcmd->argv[i+1], "all") == 0)
				{
					mn_ethLoopCheckVlanDefaultSet(); 
				}
				else
				{
					vlan=VOS_AtoI(pchkcmd->argv[i+1]);
					if( /*vlan>0&&*/vlan<4095)
					{
						if(VOS_ERROR == mn_ethLoopCheckVlanSet(vlan))
							return CMD_WARNING;
						/*vty_out(pchkcmd->vty, "set loopback detection vlan ID is %d\r\n",vlan); */
					}
					else
					{
						sys_console_printf( "out of vlan range<0-4094>\r\n"); 
						return CMD_WARNING;
					}
				}
			}
			else if( VOS_StriCmp(pchkcmd->argv[i], "mac") == 0 )
			{
				if( VOS_StriCmp( pchkcmd->argv[i+1], "default") == 0)
				{
					mn_ethLoopCheckMacDefaultSet(); 
				}
				else
				{
					if ( GetMacAddr( ( CHAR* ) pchkcmd->argv[i+1], MacAddr ) != VOS_OK )
					{
						vty_out( pchkcmd->vty, "  %% Invalid MAC address.\r\n" );
					       return CMD_WARNING;
					}
					if(mn_ethLoopCheckMacSet(MacAddr) != VOS_OK)
					{	
						/*vty_out(pchkcmd->vty, "set loopback detection mac is:\r\n"); 
						for(i=0;i<6;i++)
							vty_out(pchkcmd->vty, "%lx-",MacAddr[i]); 
						vty_out(vty, "\r\n"); */
						return CMD_WARNING;
					}
				}
			}
			else
			{
				sys_console_printf(" para error!!\r\n");	
			}
			i++;           
		}
	}
	return CMD_SUCCESS;
}


DEFUN(config_loop_detect_portlist,
	  config_loop_detect_portlist_cmd,
         "config loop-detection port [<portlist>|all|default]",
	  "config loop detection\n"
	  "set loop detection opteration\n"
	  "set port\n"
	  "input portlist\n"
	  "set detection ports all\n"
	  "set detection ports default\n"
	  )
{
	static qdef_ethchk_cmd_t chkcmd;
	int Length, i, portnum = 0;
	ULONG ulIfIndex, ulSlot, ulPort, aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};
	UCHAR portlist[192];
	LONG lRet ;
	
	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = argc;
	chkcmd.vty = vty;
	chkcmd.cmdId = ETHCHK_PORT_CMD;
	Length = VOS_StrLen(argv[0]);
	if(Length <= 64)
	{
		chkcmd.argc = 1;
		VOS_StrnCpy( (char *)chkcmd.argv[0], argv[0], 64 );
	}
	else if(Length <= 128)
	{
		chkcmd.argc = 2;
		VOS_StrnCpy( (char *)chkcmd.argv[0], argv[0], 64 );
		VOS_StrnCpy( (char *)chkcmd.argv[1], &(argv[0][64]), 64);
	}
	else if(Length <=192)
	{
		chkcmd.argc = 3;
		VOS_StrnCpy( (char *)chkcmd.argv[0], argv[0], 64 );
		VOS_StrnCpy( (char *)chkcmd.argv[1], &(argv[0][64]), 64 );
		VOS_StrnCpy( (char *)chkcmd.argv[2], &(argv[0][128]), 64 );
	}
	else
	{
		vty_out(vty,"You have added too many ports, please check it !\r\n");
		return CMD_WARNING;
	}
	
	aulMsg[3] = (ULONG)&chkcmd;

	if( VOS_StriCmp( chkcmd.argv[0], "all") == 0)   /* 问题单12884 */
	{
	}
	else if( VOS_StriCmp( chkcmd.argv[0], "default") == 0)
	{
	}
	else
	{
		for(i=0;i<chkcmd.argc;i++)
		{
			VOS_StrnCpy(&(portlist[i*64]), (char *)chkcmd.argv[i], 64 );
		}
		
		BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( portlist, ulIfIndex )
		{
			lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
			if( lRet != VOS_OK )
			{
				vty_out( vty, "  %% Parameter error.\r\n");
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}				
			/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/

			if (ulSlot > CHK_SLOT_MAX)/*??????? 这里只修改了针对6900 类型的，兼容6700 用什么宏*/
			{
				vty_out( vty, "  %% Parameter error. Slot must be 1-14 !\r\n");
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}

			if( __SYS_MODULE_TYPE__( ulSlot ) == MODULE_TYPE_NULL  || __SYS_MODULE_TYPE__( ulSlot) == MODULE_TYPE_UNKNOW )
				eth_loop_check_port_max[ ulSlot ] = 4;
			else
				eth_loop_check_port_max[ ulSlot ] = Slot_Get_PortNum( ulSlot);

			if( eth_loop_check_port_max[ ulSlot] < 5 && chk_config_eth_portlist[ ulSlot ][5] == 1)
				chk_config_eth_portlist[ ulSlot ][5] = 0;

			if( 0 == eth_loop_check_port_max[ulSlot] )  /*若该槽位为空，设置为4 */
				portnum = 4;
			else
				portnum = eth_loop_check_port_max[ulSlot];
			
			if ((ulPort > portnum ) || (ulPort < 1))
			{
				vty_out( vty, "  %% Parameter error.Slot %d port should be 1-%d\r\n", ulSlot, portnum );
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
		}
		END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
	}

	if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		return CMD_WARNING;
	}
	
	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		vty_out(vty,"Send the command to Pon card successed !\r\n");
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}
	
	return CMD_SUCCESS;
}

DEFUN(config_loop_detect_portlist_add,
	  config_loop_detect_portlist_cmd_add,
         "config loop-detection port add <portlist>",
	  "config loop detection\n"
	  "set loop detection opteration\n"
	  "set port\n"
	  "add port for ethloop\n"
	  "input portlist\n"
	  )
{
	static qdef_ethchk_cmd_t chkcmd;
	int Length, i, portnum = 0;
	ULONG ulIfIndex, ulSlot, ulPort, aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};
	UCHAR portlist[192];
	LONG lRet ;

	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = argc;
	chkcmd.vty = vty;
	chkcmd.cmdId = ETHCHK_PORT_CMD_ADD;
	Length = VOS_StrLen(argv[0]);
	if(Length <= 64)
	{
		chkcmd.argc = 1;
		VOS_StrnCpy( (char *)chkcmd.argv[0], argv[0], 64 );
	}
	else if(Length <= 128)
	{
		chkcmd.argc = 2;
		VOS_StrnCpy( (char *)chkcmd.argv[0], argv[0], 64 );
		VOS_StrnCpy( (char *)chkcmd.argv[1], &(argv[0][64]), 64);
	}
	else if(Length <=192)
	{
		chkcmd.argc = 3;
		VOS_StrnCpy( (char *)chkcmd.argv[0], argv[0], 64 );
		VOS_StrnCpy( (char *)chkcmd.argv[1], &(argv[0][64]), 64 );
		VOS_StrnCpy( (char *)chkcmd.argv[2], &(argv[0][128]), 64 );
	}
	else
	{
		vty_out(vty,"You have added too many ports, please check it !\r\n");
		return CMD_WARNING;
	}
	
	aulMsg[3] = (ULONG)&chkcmd;

	{
		for(i=0;i<chkcmd.argc;i++)
		{
			VOS_StrnCpy(&(portlist[i*64]), (char *)chkcmd.argv[i], 64 );
		}
		
		BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( portlist, ulIfIndex )
		{
			lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
			if( lRet != VOS_OK )
			{
				vty_out( vty, "  %% Parameter error.\r\n");
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}				
			/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/

			if (ulSlot > CHK_SLOT_MAX)/*??????? 这里只修改了针对6900 类型的，兼容6700 用什么宏*/
			{
				vty_out( vty, "  %% Parameter error. Slot must be 1-14 !\r\n");
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}

			if( __SYS_MODULE_TYPE__( ulSlot ) == MODULE_TYPE_NULL  || __SYS_MODULE_TYPE__( ulSlot) == MODULE_TYPE_UNKNOW )
				eth_loop_check_port_max[ ulSlot ] = 4;
			else
				eth_loop_check_port_max[ ulSlot ] = Slot_Get_PortNum( ulSlot);

			if( eth_loop_check_port_max[ ulSlot] < 5 && chk_config_eth_portlist[ ulSlot ][5] == 1)
				chk_config_eth_portlist[ ulSlot ][5] = 0;

			if( 0 == eth_loop_check_port_max[ulSlot] )  /*若该槽位为空，设置为4 */
				portnum = 4;
			else
				portnum = eth_loop_check_port_max[ulSlot];
			
			if ((ulPort > portnum ) || (ulPort < 1))
			{
				vty_out( vty, "  %% Parameter error.Slot %d port should be 1-%d\r\n", ulSlot, portnum );
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
		}
		END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
	}

	/*if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		return CMD_WARNING;
	}*/

	ethLoopCheckPortCommand_Add(&chkcmd);
	
	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		vty_out(vty,"Send the command to Pon card successed !\r\n");
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}
	
	return CMD_SUCCESS;
}

LONG ethLoopCheckPortCommand( qdef_ethchk_cmd_t *pchkcmd )
{
  	ULONG ulSlot = 0, i = 0, j=0 ;
	ULONG ulPort = 0;
	ULONG ulIfIndex=0;
	LONG lRet = 0;

	if( VOS_StriCmp( pchkcmd->argv[0], "all") == 0)
	{
		ethLoopCheckPortlistAll(); 
	}
	else if( VOS_StriCmp( pchkcmd->argv[0], "default") == 0)
	{
		ethLoopCheckPortlistDefault(); 
	}
	else
	{
		UCHAR portlist[192];
		VOS_MemZero(portlist, sizeof(portlist));

		for(i=0;i<pchkcmd->argc;i++)
		{
			VOS_StrnCpy(&(portlist[i*64]), (char *)pchkcmd->argv[i], 64 );
		}
		
		BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( portlist, ulIfIndex )
		{
			lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
			if( lRet != VOS_OK )
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
								
			/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/

			if (ulSlot > CHK_SLOT_MAX)/*??????? 这里只修改了针对6900 类型的，兼容6700 用什么宏*/
			{
				vty_out( pchkcmd->vty, "  %% Parameter error.slot must be 1-14\r\n");
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
	
			if ((ulPort > eth_loop_check_port_max[ulSlot]) || (ulPort < 1))
			{
				vty_out( pchkcmd->vty, "  %% Parameter error.port must be 1-%d\r\n",  eth_loop_check_port_max[ulSlot]);
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
		}
		END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

		VOS_MemZero(chk_config_eth_portlist_old, sizeof(chk_config_eth_portlist_old));
		VOS_MemCpy(chk_config_eth_portlist_old, chk_config_eth_portlist, sizeof(chk_config_eth_portlist_old));

		ethLoopCheckPortlistClear();

		BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( portlist, ulIfIndex )
		{
			lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
			if( lRet != VOS_OK )
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );

			if(VOS_ERROR == mn_ethLoopCheckPortAdd(ulSlot, ulPort ))
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			/*vty_out(pchkcmd->vty, "set loopback detection chack port %d  %d\r\n",ulSlot,ulPort);*/
							
		}
		END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

		for( i= 1; i < CHK_SLOT_MAX+1; i++ )
		{
			if( !CHECK_SLOTNO_IS_PON(i) )
				continue;
			
			for( j = 1; j <= Slot_Get_PortNum(i) ; j++ )
			{
				if( chk_config_eth_portlist[i][j] == 0 && chk_config_eth_portlist_old[i][j] == 1)
					check_oam_frame_send_by_port( i, j, 0, 1 );
			}
		}
	}
	return CMD_SUCCESS;
}

LONG ethLoopCheckPortCommand_Add( qdef_ethchk_cmd_t *pchkcmd )
{
  	ULONG ulSlot = 0, i = 0, j=0 ;
	ULONG ulPort = 0;
	ULONG ulIfIndex=0, disableflag=0;
	LONG lRet = 0;
	UCHAR portlist[192];

	VOS_MemZero(portlist, sizeof(portlist));

	for(i=0;i<pchkcmd->argc;i++)
	{
		VOS_StrnCpy(&(portlist[i*64]), (char *)pchkcmd->argv[i], 64 );
	}
	
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( portlist, ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
							
		/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/

		if (ulSlot > CHK_SLOT_MAX)/*??????? 这里只修改了针对6900 类型的，兼容6700 用什么宏*/
		{
			vty_out( pchkcmd->vty, "  %% Parameter error.slot must be 1-14\r\n");
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}

		if ((ulPort > eth_loop_check_port_max[ulSlot]) || (ulPort < 1))
		{
			vty_out( pchkcmd->vty, "  %% Parameter error.port must be 1-%d\r\n",  eth_loop_check_port_max[ulSlot]);
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsAll || ETHLOOPPORTLISTFLAG == EthLoopPortlistIsDefault )
	{
		VOS_MemZero(chk_config_eth_portlist_old, sizeof(chk_config_eth_portlist_old));
		VOS_MemCpy(chk_config_eth_portlist_old, chk_config_eth_portlist, sizeof(chk_config_eth_portlist_old));
		
		ethLoopCheckPortlistClear();
		disableflag = 1;
	}
	
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( portlist, ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );

		if(VOS_ERROR == mn_ethLoopCheckPortAdd(ulSlot, ulPort ))
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		/*vty_out(pchkcmd->vty, "set loopback detection chack port %d  %d\r\n",ulSlot,ulPort);*/
						
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	if( disableflag == 1 )
	{
		for( i= 1; i < CHK_SLOT_MAX+1; i++ )
		{
			if( !CHECK_SLOTNO_IS_PON(i) )
				continue;
			
			for( j = 1; j <= Slot_Get_PortNum(i) ; j++ )
			{
				if( chk_config_eth_portlist[i][j] == 0 && chk_config_eth_portlist_old[i][j] == 1)
					check_oam_frame_send_by_port( i, j, 0, 1 );
			}
		}
	}
	
	return CMD_SUCCESS;
}

LONG ethLoopCheckPortCommand_Pon( qdef_ethchk_cmd_t *pchkcmd )
{
  	ULONG ulSlot = 0,i = 0, j=0;
	ULONG ulPort = 0;
	ULONG ulIfIndex=0;
	LONG lRet = 0;

	if( VOS_StriCmp( pchkcmd->argv[0], "all") == 0)
	{
		ethLoopCheckPortlistAll(); 
	}
	else if( VOS_StriCmp( pchkcmd->argv[0], "default") == 0)
	{
		ethLoopCheckPortlistDefault(); 
	}
	else
	{
		UCHAR portlist[192];
		VOS_MemZero(portlist, sizeof(portlist));

		for(i=0;i<pchkcmd->argc;i++)
		{
			VOS_StrnCpy(&(portlist[i*64]), (char *)pchkcmd->argv[i], 64 );
		}
		
		BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( portlist, ulIfIndex )
		{
			lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
			if( lRet != VOS_OK )
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
								
			/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/
			/*if (ulSlot > 8 )
			{
				vty_out( pchkcmd->vty, "  %% Parameter error.slot must be 1-8\r\n");
				return CMD_WARNING;
			}
			if ((ulPort > 4) || (ulPort < 1))
			{
				vty_out( pchkcmd->vty, "  %% Parameter error.port must be 1-4\r\n");
				return CMD_WARNING;
			}*/
		}
		END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

		VOS_MemZero(chk_config_eth_portlist_old, sizeof(chk_config_eth_portlist_old));
		VOS_MemCpy(chk_config_eth_portlist_old, chk_config_eth_portlist, sizeof(chk_config_eth_portlist_old));

		ethLoopCheckPortlistClear();

		BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( portlist, ulIfIndex )
		{
			lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
			if( lRet == VOS_OK )
			{
				if(ulSlot == SYS_LOCAL_MODULE_SLOTNO)
				{
					if(VOS_ERROR == mn_ethLoopCheckPortAdd(ulSlot, ulPort ))
					RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
				}
			}			
		}
		END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

		for(i = 1; i <= Slot_Get_PortNum(SYS_LOCAL_MODULE_SLOTNO) ; i++ )
		{
			if( chk_config_eth_portlist[SYS_LOCAL_MODULE_SLOTNO][i] == 0 
				&& chk_config_eth_portlist_old[SYS_LOCAL_MODULE_SLOTNO][i] == 1)
			{
				check_oam_frame_send_by_port( SYS_LOCAL_MODULE_SLOTNO, i, 0, 1 );
			}
		}
	}

	return CMD_SUCCESS;
}

LONG ethLoopCheckPortCommand_Pon_Add( qdef_ethchk_cmd_t *pchkcmd )
{
  	ULONG ulSlot = 0,i = 0, j=0;
	ULONG ulPort = 0;
	ULONG ulIfIndex=0, disableflag=0;
	LONG lRet = 0;
	UCHAR portlist[192];
	
	VOS_MemZero(portlist, sizeof(portlist));

	for(i=0;i<pchkcmd->argc;i++)
	{
		VOS_StrnCpy(&(portlist[i*64]), (char *)pchkcmd->argv[i], 64 );
	}
	
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( portlist, ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsAll || ETHLOOPPORTLISTFLAG == EthLoopPortlistIsDefault )
	{
		VOS_MemZero(chk_config_eth_portlist_old, sizeof(chk_config_eth_portlist_old));
		VOS_MemCpy(chk_config_eth_portlist_old, chk_config_eth_portlist, sizeof(chk_config_eth_portlist_old));

		ethLoopCheckPortlistClear();
		disableflag=1;
	}
	
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( portlist, ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet == VOS_OK )
		{
			if(ulSlot == SYS_LOCAL_MODULE_SLOTNO)
			{
				if(VOS_ERROR == mn_ethLoopCheckPortAdd(ulSlot, ulPort ))
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
		}			
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	if( disableflag == 1 )
	{
		for(i = 1; i <= Slot_Get_PortNum(SYS_LOCAL_MODULE_SLOTNO) ; i++ )
		{
			if( chk_config_eth_portlist[SYS_LOCAL_MODULE_SLOTNO][i] == 0 
				&& chk_config_eth_portlist_old[SYS_LOCAL_MODULE_SLOTNO][i] == 1)
			{
				check_oam_frame_send_by_port( SYS_LOCAL_MODULE_SLOTNO, i, 0, 1 );
			}
		}
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_loop_detection_interval_time,
	  set_loop_detection_interval_time_cmd,
         "config loop-detection interval-time [<10-3600>|default]",
	  "config loop detection\n"
	  "set loop detection\n"
	  "set loop detection interval\n"
	  "input dectect interval, unit:second\n"
	  "set dectect interval, default 30 second\n"
	  )
{
	static qdef_ethchk_cmd_t chkcmd;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};

	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = argc;
	chkcmd.vty = vty;
	chkcmd.cmdId = ETHCHK_INTERVAL_CMD;

	/*if( VOS_StrLen(argv[0]) >= 64 )
	{
		vty_out( vty, "parameter is too long\r\n" );
		return CMD_WARNING;
	}*/
	VOS_StrnCpy( (char *)chkcmd.argv[0], argv[0],64 );
	aulMsg[3] = (ULONG)&chkcmd;

	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
		vty_out(vty,"Send the command to Pon card successed !\r\n");
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}
	
	return CMD_SUCCESS;
}
	
/*add by shixh20090311*/
DEFUN(set_loop_detection_up_times,
	  set_loop_detection_up_times_cmd,
         "config loop-detection up-times <0-10> threshold <0-10>",
	  "config loop detection\n"
	  "set loop detection\n"
	  "set loop detection uptimes\n"
	  "input times\r\n"
	  "set loop detection uptimes threshold\n\n"
	  "input dectect threshold\n"
	  )
{
	static qdef_ethchk_cmd_t chkcmd;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};
       int i=0;
       
	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = argc;
	chkcmd.vty = vty;
	chkcmd.cmdId = ETHCHK_UPTIMES_CMD;

	/*if( VOS_StrLen(argv[0]) >= 64 )
	{
		vty_out( vty, "parameter is too long\r\n" );
		return CMD_WARNING;
	}*/
        for( i=0; i<argc; i++ )
	{
		VOS_StrnCpy( (char *)chkcmd.argv[i], argv[i], 64 );
	}
	aulMsg[3] = (ULONG)&chkcmd;

	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
		vty_out(vty,"Send the command to Pon card successed !\r\n");
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}
    
	return CMD_SUCCESS;

}
LONG ethLoopCheckIntervalCommand( qdef_ethchk_cmd_t *pchkcmd )
{
	ULONG  interval;
	
	if(pchkcmd->argv[0][0] == 'd' )
	{
		mn_ethLoopCheckTimerDefaultSet();
	}
	else
	{
		interval=VOS_AtoI( pchkcmd->argv[0] );
		if(mn_ethLoopCheckTimerSet(interval)==VOS_OK)
		{
		}
		else
		{	/*vty_out(pchkcmd->vty, "set loop detection interval-time err\r\n");*/
			sys_console_printf("set loop detection interval-time err\r\n");
			return CMD_WARNING;
		}
	}

	return   CMD_SUCCESS;
}

LONG ethLoopCheckUptimesCommand( qdef_ethchk_cmd_t *pchkcmd )
{
    USHORT up_times,threshold;
    up_times = VOS_AtoI( pchkcmd->argv[0] );
    threshold = VOS_AtoI( pchkcmd->argv[1] );

    if(mn_ethLoopCheckLinkuptimesSet(up_times,threshold)!=VOS_OK)
        return  VOS_ERROR;

    return VOS_OK;

}

/*DEFUN(undo_loop_detection_interval_time,
	  undo_loop_detection_interval_time_cmd,
         "undo loop-detection interval-time",
	  "undo operation\n"
	  "undo loop detection operation\n"
	  "undo loop detection interval-time\n")
{
	mn_ethLoopCheckTimerDefaultSet();
	return CMD_SUCCESS;
}*/

static int mn_ethLoopCheckPortGetNext_ForShow( ULONG slotno, ULONG portno, ULONG *pnext_slotno, ULONG *pnext_portno )
{
	int i,j;
	
	if( (pnext_slotno == NULL) || (pnext_portno == NULL) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( (slotno > eth_loop_check_slot_max) || ((slotno == eth_loop_check_slot_max) && (portno >= eth_loop_check_port_max[slotno])) )
	{
		/*sys_console_printf("mn_ethLoopCheckPortGetNext:slotno=%d, portno=%d\r\n", slotno, portno);*/
		return VOS_ERROR;
	}
	i = slotno;
	j = portno + 1;

	if( __SYS_MODULE_TYPE__( i ) == MODULE_TYPE_NULL  || __SYS_MODULE_TYPE__( i ) == MODULE_TYPE_UNKNOW )
		eth_loop_check_port_max[ i ] = 4;
	else
		eth_loop_check_port_max[ i ] = Slot_Get_PortNum( i );

	if( eth_loop_check_port_max[ i ] < 5 && chk_config_eth_portlist[ i ][5] == 1 )
		chk_config_eth_portlist[ i ][5] = 0;
	
	if( j > eth_loop_check_port_max[i] )
	{
		j = 1;
		i++;
	}
	if( i == 0 )
		i = 1;
	
	for( ; i<=eth_loop_check_slot_max; i++ )
	{
		if( (slotno >= 0) && (slotno <= eth_loop_check_slot_max)  )
		{
			if( __SYS_MODULE_TYPE__( i ) == MODULE_TYPE_NULL  || __SYS_MODULE_TYPE__( i ) == MODULE_TYPE_UNKNOW )
				eth_loop_check_port_max[ i ] = 4;
			else
				eth_loop_check_port_max[ i ] = Slot_Get_PortNum( i );

			if( eth_loop_check_port_max[ i ] < 5 && chk_config_eth_portlist[ i ][5] == 1 )
				chk_config_eth_portlist[ i ][5] = 0;

			for( ; j<=eth_loop_check_port_max[i]; j++ )
			{
				if( chk_config_eth_portlist[i][j] )
				{
					*pnext_slotno = i;
					*pnext_portno = j;
					
					return VOS_OK;
				}
			}
			j = 1;
		}
		/*else
			sys_console_printf("mn_ethLoopCheckPortGetNext:i=%d, j=%d\r\n", i, j);*/
	}
	return VOS_ERROR;
}


DEFUN(show_loop_detection_result,
	  show_loop_detection_result_cmd,
         "show loop-detection {[config|status]}*1",
	  "show running system information\n"
	  "ethernet loop detection\n"
	  "show config data\n"
	  "show status data\n"
	  )
{
	USHORT i,j; 
	if( (argc == 1) && (VOS_StriCmp( argv[0], "config") == 0) )
	{
		ULONG  gettime,vid;
		ULONG  slotno,portno,pnext_slotno,pnext_portno;
		int  enable;
	       char * loopchkStatus[] = {"error","enable", "disable"};
		int count = 1;
		USHORT up_times,threshold;
		   
		/*vty_out(vty, "loop-detection configuration\r\n");*/
		enable = mn_ethLoopCheckEnableGet();

		if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY )
			vty_out(vty, "  Detection Status: only-olt %s\r\n", loopchkStatus[enable]);
		else
			vty_out(vty, "  Detection Status:%s\r\n", loopchkStatus[enable]);

		enable = mn_ethLoopCheckContralEnableGet();
		vty_out(vty, "  Detection Control:%s\r\n", loopchkStatus[enable]);
/*
		enable = mn_ethLoopCheckMacCleanEnableGet();
		vty_out(vty, "  Detection Mac-clean:%s\r\n", loopchkStatus[enable]);
*/			
		gettime=mn_ethLoopCheckTimerGet();
		vty_out(vty, "  Interval-time:%ds\r\n",gettime);

		if(mn_ethLoopCheckLinkuptimesGet(&up_times,&threshold)==VOS_OK)
		{
		vty_out(vty, "  Detection up-times:%d,threshold:%d\r\n",up_times,threshold);
		}
		
		vid=mn_ethLoopCheckVlanGet();
		if( vid == 0 )
			vty_out(vty, "  Detection vlan:all\r\n");
		else
		{
			char *portlist_str;
			int ret, size;

			size = 5*2048; 

			portlist_str = VOS_Malloc(size, MODULE_LOOPBACK);
			if ( !portlist_str )
			{
				ASSERT( 0 );
				return VOS_ERROR;
			}

			VOS_MemZero(portlist_str,  size );
			
			ret = EthLoopGetVlanList(portlist_str, size);
			
			if(portlist_str[0] != 0 )
			{
				if (  ret != VOS_ERROR )
					vty_big_out(vty, size, "  Detection vlan: %s\r\n",portlist_str);
			}
			else
				vty_out(vty, "  Detection vlan: None \r\n");
			
			VOS_Free( portlist_str);

		}
		
		vty_out(vty, "  Detection MAC addr:%02x%02x.%02x%02x.%02x%02x\r\n",
					chk_config_frame_smac[0], chk_config_frame_smac[1],
					chk_config_frame_smac[2], chk_config_frame_smac[3], 
					chk_config_frame_smac[4], chk_config_frame_smac[5] );
		
		if( eth_loopcheck_portlist_is_all() )
		{
			vty_out( vty, "  Port member list: all\r\n" );
		}
		else if(eth_loopcheck_portlist_is_default())
		{
			vty_out( vty, "  Port member list: default\r\n" );
		}
		else
		{
			slotno=0;
			portno=0;
			vty_out( vty, "  Port member list:\r\n" );
			while(mn_ethLoopCheckPortGetNext_ForShow(slotno,portno,&pnext_slotno, &pnext_portno) == VOS_OK)
			{
				slotno=pnext_slotno;
				portno=pnext_portno;

				if( CHECK_SLOTNO_IS_ETH(slotno) )
					vty_out(vty, "    eth%d/%d",slotno,portno);
				else if( CHECK_SLOTNO_IS_PON(slotno) )
					vty_out(vty, "    pon%d/%d",slotno,portno);
				else
                            {       if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsDesigned)     
                                        vty_out(vty, "    eth%d/%d",slotno,portno);
                                     else
                                        continue;
	                      }
                
				if( count % 4 == 0 )
					vty_out(vty, "\r\n");
				count++;
			}
			if( count % 8 )
				vty_out(vty, "\r\n");
			
			for(i = 1; i<= eth_loop_check_slot_max; i++)
			{
				if(ethloop_devsm_pull_flag[i][0] == eth_devsm_difference)
				{
					vty_out(vty,"  From you config the port :\r\n");
					for(j = i; j<= eth_loop_check_slot_max; j++)
					{
						if(ethloop_devsm_pull_flag[j][0] == eth_devsm_difference)
						{
							if(ethloop_devsm_pull_flag[j][1] == ethloop_devsm_eth)
								vty_out(vty,"    slot %d : uplink card changes to be pon card.\r\n",j);
							else if(ethloop_devsm_pull_flag[j][1] == ethloop_devsm_pon)
								vty_out(vty,"    slot %d : pon card changes to be uplink card.\r\n",j);
						}
					}	
					break;
				}
			}
			vty_out(vty,"\r\n");
		}
	}
	else
	{
		if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY )
		{
			VOS_ASSERT(0);
			return CMD_WARNING;
#if 0
			oltMode_ethLoopCheckResultShow( vty );
#endif
		}
		else
		{
			onuMode_ethLoopCheckResultShow( vty );
		}
	}
	return   CMD_SUCCESS;
}

#if 0
DEFUN(show_loop_detection,
	  show_loop_detection_cmd,
         "show loop-detection-config",
	  "Show running system information\n"
	  "show loop detection configuration\n"
	  )
{
	ULONG  gettime,vid;
	ULONG  slotno,portno,pnext_slotno,pnext_portno;
	int  enable;
	char * loopchkStatus[] = {"error","enable", "disable"};
	int count = 1;
	   
	vty_out(vty, "loop-detection configuration\r\n");
	enable=mn_ethLoopCheckEnableGet();
	vty_out(vty, "  Detect status:%s\r\n",loopchkStatus[enable]);
	
	gettime=mn_ethLoopCheckTimerGet();
	vty_out(vty, "  Interval-time:%ds\r\n",gettime);

	vid=mn_ethLoopCheckVlanGet();
	if( vid == 0 )
		vty_out(vty, "  Detect vlan:all\r\n");
	else
		vty_out(vty, "  Detect vlan:%d\r\n",vid);
	
	slotno=0;
	portno=0;
	vty_out(vty, "  Port member list:\r\n");
	while(mn_ethLoopCheckPortGetNext(slotno,portno,&pnext_slotno, &pnext_portno)==0)
	{
		slotno=pnext_slotno;
		portno=pnext_portno;

		if( CHECK_SLOTNO_IS_ETH(slotno) )
			vty_out(vty, "    eth%d/%d",slotno,portno);
		else if( CHECK_SLOTNO_IS_PON(slotno) )
			vty_out(vty, "    pon%d/%d",slotno,portno);
		else
			continue;
		if( count % 4 == 0 )
			vty_out(vty, "\r\n");
		count++;
	}
	if( count % 8 )
		vty_out(vty, "\r\n");
	
	return   CMD_SUCCESS;
}
#endif

DEFUN(loop_detection_control,
	  loop_detection_control_cmd,
         "loop-detection control [enable|disable]",
	  "config loop detection\n"
	  "set loop detection control\n"
	  "set loop detection control enable\n"
	  "set loop detection control disable\n"
	  )
{
	static qdef_ethchk_cmd_t chkcmd;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};

	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = argc;
	chkcmd.vty = vty;
	chkcmd.cmdId = ETHCHK_CONTRAL_CMD;
	/*if( VOS_StrLen(argv[0]) >= 64 )
	{
		vty_out( vty, "parameter is too long\r\n" );
		return CMD_WARNING;
	}*/
	VOS_StrCpy( (char *)chkcmd.argv[0], argv[0] );
	aulMsg[3] = (ULONG)&chkcmd;

	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
		vty_out(vty,"Send the command to Pon card successed !\r\n");
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}
	
	return CMD_SUCCESS;
}

LONG ethLoopCheckContralCommand( qdef_ethchk_cmd_t *pchkcmd )
{
	if(pchkcmd->argc==1)
	{
		if( VOS_StriCmp(pchkcmd->argv[0], "enable")==0)
		{
			mn_ethLoopCheckContralEnable();
		}
		else
		{
			mn_ethLoopCheckContralDisable();
		}
	}
	return   CMD_SUCCESS;
}

LONG ethLoopCheckMacCleanCommand( qdef_ethchk_cmd_t *pchkcmd )
{
	if(pchkcmd->argc==1)
	{
		if( VOS_StriCmp(pchkcmd->argv[0], "enable")==0)
		{
			mn_ethLoopCheckMacCleanEnable();
		}
		else
		{
			mn_ethLoopCheckMacCleanDisable();
		}
	}
	return   CMD_SUCCESS;
}

int ethLoopCheckConfigCommandProcess( qdef_ethchk_cmd_t *pCmd)
{
	int rc = VOS_ERROR;
	if(pCmd->argc >3)/*现在环路告警模块所有命令的参数不大于三个*/
	{
		VOS_ASSERT(0);
		return CMD_WARNING;
	}
	switch( pCmd->cmdId )
	{
		case ETHCHK_ENABLE_CMD:
			rc = ethLoopCheckEnableCommand( pCmd );
			break;
		case ETHCHK_CONTRAL_CMD:
			rc = ethLoopCheckContralCommand( pCmd );
			break;
		case ETHCHK_MAC_CLEAN_ENABLE_CMD:
			rc = ethLoopCheckMacCleanCommand( pCmd );
			break;
		case ETHCHK_PORT_CMD:
			rc = ethLoopCheckPortCommand( pCmd );
			break;
		case ETHCHK_PORT_CMD_ADD:
			rc = ethLoopCheckPortCommand_Add( pCmd );
			break;
		case ETHCHK_VLAN_CMD:
			rc = ethLoopCheckVlanCommand( pCmd );
			break;
		case ETHCHK_INTERVAL_CMD:
			rc = ethLoopCheckIntervalCommand( pCmd );
			break;
              case ETHCHK_UPTIMES_CMD:
                     rc = ethLoopCheckUptimesCommand(pCmd);
                     break;
		 case ETHCHK_IMMEDIATELY_CMD:
                     rc = ethLoopCheckImmediatelyCommand(pCmd);
                     break;
		default:
			break;
	}
	return rc;
}

#if 0
int ethLoopCheckConfigCommandProcess_Pon( qdef_ethchk_cmd_t *pCmd)
{
	int rc = VOS_ERROR;
	switch( pCmd->cmdId )
	{
		case ETHCHK_ENABLE_CMD:
			rc = ethLoopCheckEnableCommand_Pon( pCmd );
			break;
		case ETHCHK_CONTRAL_CMD:
			rc = ethLoopCheckContralCommand( pCmd );
			break;
		case ETHCHK_PORT_CMD:
			rc = ethLoopCheckPortCommand_Pon( pCmd );
			break;
		case ETHCHK_PORT_CMD_ADD:
			rc = ethLoopCheckPortCommand_Pon_Add( pCmd );
			break;
		case ETHCHK_VLAN_CMD:
			rc = ethLoopCheckVlanCommand_Pon( pCmd );
			break;
		case ETHCHK_INTERVAL_CMD:
			rc = ethLoopCheckIntervalCommand( pCmd );
                     break;
              case ETHCHK_UPTIMES_CMD:
                     rc = ethLoopCheckUptimesCommand(pCmd);
			break;
		default:
			break;
	}
	return rc;
}
#endif

DEFUN(debug_loop_detection,
		  debug_loop_detection_cmd,
	         "debug loop-detection",
		  "Debug\n"
		  "debug loop detection info.\n"
	  	)
{
	
	loopChkDebugSwitch=3;
	/*vty_out(vty, "loop check debug switch is open!\r\n");*/
	
	return   CMD_SUCCESS;
}

DEFUN(undo_debug_loop_detection,
		  undo_debug_loop_detection_cmd,
	         "undo debug loop-detection",
	         "Negate a command or set its defaults\n"
		  "Delete configuration\n"
		  "loop detection info.\n"
	  	)
{
	loopChkDebugSwitch=0;
	/*vty_out(vty, "loop check debug switch is closed!\r\n");	*/
	return   CMD_SUCCESS;
}

DEFUN(debug_loop_detection_fc,
	  debug_loop_detection_fc_cmd,
	  "loop-detection-parameter {[packets] <10-256>}*1 {[delay-time] <0-50>}*1",
	  "Modify loop detection parameter\n"
	  "set packets number\n"
	  "input packets number\n"
	  "set delay time\n"
	  "input delay time,unit:ms\n"
	  	)
{
	if( argc == 0 )
	{
		vty_out(vty, "  packets: %d\r\n", eth_loop_chk_tx_pkts_ctrl_number);
		vty_out(vty, "  delay-time: %d\r\n", eth_loop_chk_tx_pkts_ctrl_delay);
	}
	else if( argc == 2 )
	{
		if( argv[0][0] == 'd' )
			eth_loop_chk_tx_pkts_ctrl_delay = VOS_AtoI(argv[1]);
		else
			eth_loop_chk_tx_pkts_ctrl_number = VOS_AtoI(argv[1]);
	}
	else if( argc == 4 )
	{
		eth_loop_chk_tx_pkts_ctrl_number = VOS_AtoI(argv[1]);
		eth_loop_chk_tx_pkts_ctrl_delay = VOS_AtoI(argv[3]);
	}
	return   CMD_SUCCESS;
}


#ifdef __TEST_CTRL_CHANNEL
extern LONG bcm5325e_send_pkts_counter;
extern LONG bcm5325e_recv_pkts_counter;
extern LONG bcm5325e_mirror_enable;
extern USHORT bcm5325e_send_mirror_port;
extern USHORT bcm5325e_recv_mirror_port;

DEFUN(debug_bcm5325e_mirror_enable,
	debug_bcm5325e_mirror_enable_cmd,
	"mirror_ctrlchan [enable|disable] {[clear]}*1",
	"debug bcm5325e mirror to uplink-port\n"
	"enable\n"
	"disable\n")
{
	if( VOS_StrCmp(argv[0], "enable") == 0 )
		bcm5325e_mirror_enable=1;
	else	
		bcm5325e_mirror_enable=0;
	if( argc > 1 )
	{
		bcm5325e_send_pkts_counter = 0;
		bcm5325e_recv_pkts_counter = 0;
	}
	return   CMD_SUCCESS;
}
DEFUN(debug_bcm5325e_mirror_show,
	debug_bcm5325e_mirror_show_cmd,
	"show mirror_ctrlchan",
	"show running system information\n"
	"ctrl-channel config and statistics info.\n")
{
	ULONG ulSlot=0, ulPort=0;
	vty_out( vty, "mirror_ctrlchan %s\r\n", ( bcm5325e_mirror_enable ? "enable" : "disable" ) );
	swport_2_slot_port(bcm5325e_send_mirror_port, &ulSlot, &ulPort);  /* 注意这里传送的是L2 port，尚未修改kkkkkkkk */
	vty_out( vty, "mirror_ctrlchan_send to eth%d/%d\r\n", ulSlot, ulPort );
	swport_2_slot_port(bcm5325e_recv_mirror_port, &ulSlot, &ulPort);   /* 注意这里传送的是L2 port，尚未修改kkkkkkkk */
	vty_out( vty, "mirror_ctrlchan_recv to eth%d/%d\r\n", ulSlot, ulPort );
	vty_out( vty, "tx_pkts:%ld\r\n", bcm5325e_send_pkts_counter);
	vty_out( vty, "rx_pkts:%ld\r\n", bcm5325e_recv_pkts_counter);
	return   CMD_SUCCESS;
}
#endif

DEFUN(loop_chk_onu_ExpandInfo,
	loop_chk_onu_ExpandInfo_cmd,
	"loop_chk_onu_ExpandInfo",
	"loop chk onuoam include ExpandInfo.\n")
{
	LoopChkOnuIncludeExpandInfo = 1;
	return   CMD_SUCCESS;
}

DEFUN(undo_loop_chk_onu_ExpandInfo,
	undo_loop_chk_onu_ExpandInfo_cmd,
	"undo loop_chk_onu_ExpandInfo",
	"loop chk onuoam not include ExpandInfo.\n")
{
	LoopChkOnuIncludeExpandInfo = 0;
	return   CMD_SUCCESS;
}

LONG ethloop_chk_cli_cmd_install()
{
#ifdef __TEST_CTRL_CHANNEL
	install_element ( DEBUG_HIDDEN_NODE, &debug_bcm5325e_mirror_enable_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &debug_bcm5325e_mirror_show_cmd);
#endif   
#ifdef CHK_BRAS_MAC
	install_element ( CONFIG_NODE, &check_BRAS_mac_enable_cmd);/*add by shixh20090715*/
	install_element ( CONFIG_NODE, &show_check_BRAS_mac_enable_cmd);
	install_element ( CONFIG_NODE, &BRAS_MAC_detection_cmd);
	install_element ( CONFIG_NODE, &delete_BRAS_MAC_detection_cmd);
	install_element ( CONFIG_NODE, &show_BRAS_MAC_detection_cmd);
#endif
	
	install_element ( CONFIG_NODE, &loop_detection_enable_cmd);
	install_element ( CONFIG_NODE, &config_loop_detection_vlan_cmd);
	install_element ( CONFIG_NODE, &config_loop_detection_vlan_cmd2);
	install_element ( CONFIG_NODE, &config_loop_detection_mac_cmd);
	install_element ( CONFIG_NODE, &config_loop_detect_portlist_cmd);
	install_element ( CONFIG_NODE, &config_loop_detect_portlist_cmd_add);
	install_element ( CONFIG_NODE, &set_loop_detection_interval_time_cmd);
	install_element ( CONFIG_NODE, &set_loop_detection_up_times_cmd);
	install_element ( CONFIG_NODE, &loop_detection_control_cmd);
	install_element ( CONFIG_NODE, &show_loop_detection_result_cmd);
	/*
	install_element ( DEBUG_HIDDEN_NODE, &loop_detection_mac_clean_enable_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &show_loop_detection_mac_clean_enable_cmd);
	*/
	install_element (VIEW_NODE, &show_loop_detection_result_cmd);

	install_element ( DEBUG_HIDDEN_NODE, &debug_loop_detection_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &undo_debug_loop_detection_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &loop_detection_immediately_cmd);
	  
      install_element ( DEBUG_HIDDEN_NODE, &debug_loop_detection_fc_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &loop_detection_syslog_enable_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &show_loop_detection_syslog_status_cmd);

	install_element ( DEBUG_HIDDEN_NODE, &loop_chk_onu_ExpandInfo_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &undo_loop_chk_onu_ExpandInfo_cmd);
	/*install_element ( CONFIG_NODE, &test_loop_show_cmd);*/
       /*install_element ( CONFIG_NODE, &show_chk_config_eth_portlist_cmd);*/

	return VOS_OK;
}

int EthLoopGetVlanList(char *portlist_str, int  length)
{
	char tmp[16];
	ULONG Svalue = 0, Evalue = 0,counter = 0,Flag=0;
	EthLoopVlan_t *EthLoopVlan_Temp = EthLoopVlan_List;

	if(length <5*2048)
		return VOS_ERROR;
	
	VOS_MemZero( portlist_str, sizeof(portlist_str) );
	VOS_MemZero( tmp, sizeof(tmp) );
	Svalue = Evalue = counter =Flag = 0;
	while(EthLoopVlan_Temp)
	{
		if(Svalue == 0)
		{
			Svalue = Evalue = EthLoopVlan_Temp->vlanid;
		}
		
		counter++;

		if(counter != 1)
		{
			if(Evalue != EthLoopVlan_Temp->vlanid-1)
				Flag =1;
			else
				Evalue = EthLoopVlan_Temp->vlanid;
		}

		if( Flag == 1 )
		{
			if(counter == 2 )
			{
				VOS_Sprintf( tmp, "%s%d", ((portlist_str[0] == 0) ? "" : ","),Svalue );
				VOS_StrCat( portlist_str, tmp );
			}
			else
			{
				VOS_Sprintf( tmp, "%s%d-%d", ((portlist_str[0] == 0) ? "" : ","),Svalue, Evalue);
				VOS_StrCat( portlist_str, tmp );
			}
			Svalue = Evalue = EthLoopVlan_Temp->vlanid ;
			counter = 1 ;
			Flag = 0 ;
		}

		if ( EthLoopVlan_Temp->next == NULL )
		{
			if(counter == 1 )
			{
				VOS_Sprintf( tmp, "%s%d", ((portlist_str[0] == 0) ? "" : ","),Svalue );
				VOS_StrCat( portlist_str, tmp );
			}
			else
			{
				VOS_Sprintf( tmp, "%s%d-%d", ((portlist_str[0] == 0) ? "" : ","),Svalue, Evalue);
				VOS_StrCat( portlist_str, tmp );
			}
		}
#if 0
		if(Flag == 1 || EthLoopVlan_Temp->next == NULL)
		{
			counter--;
			if(counter == 1 && Flag == 1 )
			{
				VOS_Sprintf( tmp, "%s%d", ((portlist_str[0] == 0) ? "" : ","),Svalue );
				VOS_StrCat( portlist_str, tmp );
			}
			else
			{
				VOS_Sprintf( tmp, "%s%d-%d", ((portlist_str[0] == 0) ? "" : ","),Svalue, Evalue);
				VOS_StrCat( portlist_str, tmp );
			}
			Svalue = Evalue = EthLoopVlan_Temp->vlanid ;
			counter = 1;
			Flag = 0;
		}
#endif		
		EthLoopVlan_Temp = EthLoopVlan_Temp->next ;
	}

#if 0
	if( Svalue == Evalue )
	{
		VOS_Sprintf( tmp, "%s%d", ((portlist_str[0] == 0) ? "" : ","),Svalue );
		VOS_StrCat( portlist_str, tmp );
	}
#endif

	return VOS_OK;

}
#if 0
int EthLoopGetPortList(char *portlist, int  length)
{
	char portlist_str[256], tmp[16];
	ULONG Svalue = 0, Evalue = 0,counter = 0;
	ULONG i,j, y=0;

	if(length <256)
		return VOS_ERROR;
	
	VOS_MemZero( portlist_str, sizeof(portlist_str) );
	VOS_MemZero( tmp, sizeof(tmp) );
	for( i=1; i<=eth_loop_check_slot_max; i++ )
	{
		for( j=1; j<=eth_loop_check_port_max[i]; j++ )
		{	
			counter = 0;
			y = j;
			while(chk_config_eth_portlist[i][y] == 1)
			{
				counter++ ;
				if(counter == 1)
					Svalue = y;
				
				Evalue = y;
				
				if(y < eth_loop_check_port_max[i])
					y++ ;
				else if( y == eth_loop_check_port_max[i])
					break;
			}
			if(counter == 0)
			{
			}
			else if(counter == 1)
			{
				VOS_Sprintf( tmp, "%s%d/%d", ((portlist_str[0] == 0) ? "" : ","), i, j );
				VOS_StrCat( portlist_str, tmp );
			}
			else
			{
				VOS_Sprintf( tmp, "%s%d/%d-%d", ((portlist_str[0] == 0) ? "" : ","), i, Svalue, Evalue);
				VOS_StrCat( portlist_str, tmp );
			}
			j = y;
		}
	}

	VOS_MemCpy(portlist,  portlist_str, 256);
	return VOS_OK;
}
#else
int EthLoopGetPortList(char *portlist, int  length, int ulSlot)    /* 问题单:  16276 */
{
	char portlist_str[256], tmp[16];
	ULONG Svalue = 0, Evalue = 0,counter = 0;
	ULONG i,j, y=0;

	if(length <256)
		return VOS_ERROR;
	
	VOS_MemZero( portlist_str, sizeof(portlist_str) );
	VOS_MemZero( tmp, sizeof(tmp) );

	i = ulSlot;

	for( j=1; j<=eth_loop_check_port_max[i]; j++ )
	{	
		counter = 0;
		y = j;
		while(chk_config_eth_portlist[i][y] == 1)
		{
			counter++ ;
			if(counter == 1)
				Svalue = y;
			
			Evalue = y;
			
			if(y < eth_loop_check_port_max[i])
				y++ ;
			else if( y == eth_loop_check_port_max[i])
				break;
		}
		if(counter == 0)
		{
		}
		else if(counter == 1)
		{
			VOS_Sprintf( tmp, "%s%d/%d", ((portlist_str[0] == 0) ? "" : ","), i, j );
			VOS_StrCat( portlist_str, tmp );
		}
		else
		{
			VOS_Sprintf( tmp, "%s%d/%d-%d", ((portlist_str[0] == 0) ? "" : ","), i, Svalue, Evalue);
			VOS_StrCat( portlist_str, tmp );
		}
		j = y;
	}

	VOS_MemCpy(portlist,  portlist_str, 256);
	return VOS_OK;
}
#endif

LONG ethloop_showrun( struct vty * vty )
{
	char *portlist_str, portlist[256];
	/*Enet_MACAdd_Infor1 sys_mac;*/
	ULONG  Ret,val, size, i=0;
#ifdef CHK_BRAS_MAC
	int k,t;
#endif
	USHORT up_times=0,threshold=0;
	/*char  pmac[6]={0};*/
	
	vty_out( vty, "!ETH loop detection config\r\n" );

	if( chk_config_detect_enable == TRUE )
	{
		if( chk_config_eth_vid != 0 )
		{	
			size = 5*2048; 

			portlist_str = VOS_Malloc(size, MODULE_LOOPBACK);
			if ( !portlist_str )
			{
				ASSERT( 0 );
				return VOS_ERROR;
			}

			VOS_MemZero(portlist_str,  size );
			
			Ret = EthLoopGetVlanList(portlist_str,size);

			if( portlist_str[0] != 0 && Ret != VOS_ERROR)
				vty_big_out( vty, size, " config loop-detection vlan add %s\r\n", portlist_str );

			VOS_Free( portlist_str );

		}

		/* modified by xieshl 20091216, 这里禁止直接从eeprom中读sysmac地址 */
		/*VOS_MemZero( &sys_mac, sizeof(sys_mac) );
		if( funReadMacAddFromNvram(&sys_mac) != NULL )*/
		{
			if( VOS_MemCmp(chk_config_frame_smac, chk_config_frame_smac_backup, ETH_LOOP_MAC_ADDR_LEN) != 0 )
			{
				vty_out( vty, " config loop-detection mac %02x%02x.%02x%02x.%02x%02x\r\n", 
							chk_config_frame_smac[0], chk_config_frame_smac[1],
							chk_config_frame_smac[2], chk_config_frame_smac[3], 
							chk_config_frame_smac[4], chk_config_frame_smac[5] );
			}
		}

		if( eth_loopcheck_portlist_is_all() )
		{
			vty_out( vty, " config loop-detection port all\r\n" );
		}
		else if( eth_loopcheck_portlist_is_default() )
		{
			vty_out( vty, " config loop-detection port default\r\n" );
		}
		else
		{
			for( i=1; i<=eth_loop_check_slot_max; i++ )   /* 问题单:  16276 */
			{
				VOS_MemZero( portlist, sizeof(portlist) );
				
				Ret = EthLoopGetPortList(portlist,256, i );
				
				if( portlist[0] != 0 && Ret != VOS_ERROR )
					vty_out( vty, " config loop-detection port add %s\r\n", portlist );
			}
		}
			
		if( (val = mn_ethLoopCheckTimerGet()) != DEF_CHECK_TIMER_INTERVAL )
			vty_out( vty, " config loop-detection interval-time %d\r\n", val );

		if(mn_ethLoopCheckLinkuptimesGet(&up_times,&threshold)==VOS_OK)
		{
  			if( (chk_config_up_times != PORTDOWN_TO_UP_TIMES) ||
				(chk_config_up_times_thresh != LINKUP_TIMES_THRESH) )
  			{
				vty_out(vty, " config loop-detection up-times %d threshold %d\r\n",up_times,threshold);
  			}
		}
		
		if( chk_config_detect_control_enable == TRUE )
			vty_out( vty, " loop-detection control enable\r\n" );		/* modified by xieshl 20080530, #6750 */
		 
		if( chk_config_detect_mode == ETH_LOOP_CHECK_MODE_OLT_ONLY )
			vty_out( vty, " loop-detection only-olt enable\r\n") ;		/* modified by xieshl 20080529, #6748 */
		else
			vty_out( vty, " loop-detection enable\r\n") ;
		/*
		if(  chk_config_detect_mac_clean_enable == TRUE )
			vty_out( vty, " loop-detection-mac-clean enable\r\n");
		*/
	}
	/*问题单9140*/
	
#ifdef CHK_BRAS_MAC
	if( chk_BRAS_mac_enable == TRUE )
	{
		vty_out(vty,"check-bras-mac enable\r\n");

		for(k=0;k<CHK_BRAS_MAC_MAX;k++)	
		{
		     /*if(VOS_MemCmp(&chk_bras[k][0],chk_config_frame_BRAS_mac_t,sizeof(chk_config_frame_BRAS_mac_t))==0)*/
			if( (chk_bras[k][0] | chk_bras[k][1] | chk_bras[k][2] | chk_bras[k][3] | chk_bras[k][4] | chk_bras[k][5]) == 0 )
		     	{
		     		continue;
		     	}
			else
			{
				vty_out(vty,"bras-mac-detection ");
				for(t=0; t<(ETH_LOOP_MAC_ADDR_LEN -1); t++)
				{
					vty_out(vty, "%02x%02x", chk_bras[k][t],chk_bras[k][t+1]);
					t++;
					if( t != 5 )vty_out(vty,".");
				}
				vty_out(vty,"\r\n");
				/*Qos_Get_Mac_Address_By_Str(pmac,chk_bras[k][0]);
				vty_out(vty,"bras-mac-detection %s\r\n",pmac);*/
				/*vty_out(vty,"bras-mac-detection %s\r\n",chk_bras[k][0]);*/
			}
		}
		
	}
#endif
	vty_out( vty, "!\r\n\r\n" );

	return VOS_OK;
}

/* added by xieshl 20110829, 独立定义showrun模块，问题单12918 */
LONG ethloop_module_init()
{
    struct cl_cmd_module * ethloop_module = NULL;

    ethloop_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_LOOPBACK);
    if ( !ethloop_module )
    {
        ASSERT( 0 );
        return VOS_ERROR;
    }

    VOS_MemZero( ( char * ) ethloop_module, sizeof( struct cl_cmd_module ) );

    ethloop_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_LOOPBACK );
    if ( !ethloop_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( ethloop_module );
        return VOS_ERROR;
    }
    VOS_StrCpy( ethloop_module->module_name, "loop-detection" );

    ethloop_module->init_func = ethloop_chk_cli_cmd_install;
    ethloop_module->showrun_func = ethloop_showrun;
    ethloop_module->next = NULL;
    ethloop_module->prev = NULL;
    cl_install_module( ethloop_module );

    return VOS_OK;
}



/*B-added by zhengyt@10-3-18,网管mib环路检测读写函数*/
STATUS getGwEthLoopState(ULONG *ret_var)
{
	int enable=0;

	enable=mn_ethLoopCheckEnableGet();
	if(enable==1)
	{
		if(chk_config_detect_mode==ETH_LOOP_CHECK_MODE_OLT_ONLY)
		{
			*ret_var=2;
		}
		else if(chk_config_detect_mode==ETH_LOOP_CHECK_MODE_OLT_ONU)
		{
			*ret_var=1;
		}
	}
	else
	{
		*ret_var=3;
	}
	return VOS_OK;
}

STATUS getGwEthLoopMacAddr(char*pVar,ULONG *Vlen)
{
	VOS_MemCpy(pVar, chk_config_frame_smac, 6);
	*Vlen=6;
	return VOS_OK;
}

STATUS getGwEthLoopVlanId(ULONG*Var)
{
	USHORT VlanId=0;
	/* modified by duzhk 2011-12-7   Just for 12479 
	只有当此刻只有一个VLAN 时有效*/
#if 0
	VlanId=mn_ethLoopCheckVlanGet();
#else
	if(EthLoopVlan_List != NULL )
		VlanId = EthLoopVlan_List->vlanid;
	else
		VlanId=mn_ethLoopCheckVlanGet();
#endif
	*Var=VlanId;
	return VOS_OK;
}

STATUS getGwEthLoopIntervalTime(ULONG *Var)
{
	*Var=mn_ethLoopCheckTimerGet();
	return VOS_OK;
	
}


STATUS getGwEthLoopOnuPortShutDownEnable(int *Var)
{
	*Var=(int)mn_ethLoopCheckContralEnableGet();
	return VOS_OK;
}

STATUS getGwEthLoopOnuUpTime(ULONG *Var)
{
	USHORT ulTime=0,ulThreshold=0;
	mn_ethLoopCheckLinkuptimesGet(&ulTime,&ulThreshold);
	*Var=ulTime;
	return VOS_OK;
}

STATUS getGwEthLoopOnuUptimeThreshold(ULONG *Var)
{
	USHORT ulVar=0,ulThreshold=0;
	mn_ethLoopCheckLinkuptimesGet(&ulVar,&ulThreshold);
	*Var=ulThreshold;
	return VOS_OK;
}

STATUS getGwEthLoopPonMacTableClearEnable()
{
	return VOS_OK;
}

STATUS setGwEthLoopState(ULONG setVal)
{

	static qdef_ethchk_cmd_t chkcmd;
	int argc=1;
	STATUS ret=VOS_OK;	
	char *loopEblStr[] = {"disable","enable"};
	char*loopMode[]={"only-olt","olt-onu"};
	ULONG enable = FALSE;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};

	if(setVal == 2)
		return VOS_ERROR;
	
	if(setVal==1||setVal==2)
		enable=TRUE;
	if(setVal==2)
		argc=2;

	if( enable == chk_config_detect_enable )
	{
		return ret;
	}

	/*eth_loop_chk_start_flag = enable;*/
	
	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	

	chkcmd.argc = argc;
	chkcmd.vty =NULL;
	chkcmd.cmdId = ETHCHK_ENABLE_CMD;
	if(setVal==1)
	{VOS_StrCpy(chkcmd.argv[0], loopEblStr[enable]);}
	else if(setVal==2)
	{
		VOS_StrCpy(chkcmd.argv[0], loopMode[0]);
		VOS_StrCpy(chkcmd.argv[1], loopEblStr[enable]);
	}
	else if(setVal==3)
	{
		VOS_StrCpy(chkcmd.argv[0], loopEblStr[enable]);
		VOS_StrCpy(chkcmd.argv[1], loopEblStr[enable]);
	}
	aulMsg[3] = (ULONG)&chkcmd;
	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			ret=VOS_ERROR;
		}
	}
	else
		return VOS_ERROR;
	
	return ret;
}



STATUS setGwEthLoopVlanId(ULONG set_var)
{
	static qdef_ethchk_cmd_t chkcmd;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};

	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = 2;
	chkcmd.vty = NULL;
	chkcmd.cmdId = ETHCHK_VLAN_CMD;


	VOS_StrCpy( (char *)chkcmd.argv[0], "vlan" );
	if(set_var == 0)
		VOS_Sprintf( (char *)chkcmd.argv[1], "all");
	else
	{
		chkcmd.argc = 3;
		VOS_Sprintf( (char *)chkcmd.argv[1], "add");
		VOS_Sprintf( (char *)chkcmd.argv[2], "%d",set_var);
	}
	aulMsg[3] = (ULONG)&chkcmd;

	if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		return VOS_ERROR;
	}
	return VOS_OK;
}

STATUS setGwEthLoopIntervalTime(ULONG set_var)
{
	static qdef_ethchk_cmd_t chkcmd;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};

	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = 1;
	chkcmd.vty = NULL;
	chkcmd.cmdId = ETHCHK_INTERVAL_CMD;

	
	VOS_Sprintf( (char *)chkcmd.argv[0], "%d",set_var);

	aulMsg[3] = (ULONG)&chkcmd;
	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
	}
	else
		return VOS_OK;
	
	return CMD_SUCCESS;
}

STATUS setGwEthLoopPortList(uchar set_var[], ULONG var_val_len)
{
         int i=0,j,m,n=0;
         /*ulong ulVar=0;
         uchar mid_var[52]={0};*/

         if( var_val_len > Mib_PortList_Len )
         	return VOS_ERROR;
	  
         VOS_MemZero( chk_config_eth_portlist, sizeof(chk_config_eth_portlist) );

         
         for(i = 0 ; i<var_val_len; i++ )
         {
                   
                   if(set_var[i]!=0)
                   {        
                            /*sys_console_printf("set_var=%02x\r\n",set_var[i]);*/
                            for(j=0;j<8;j++)
                            {
                                     if((set_var[i])&(0x80>>j))
                                     {        
                                               m=(i)/4+1;
                                               n=((i%4)*8+j)+1;
                                               /*sys_console_printf("i=%d,j=%d,m=%d\r\n",i,(j+1),m);*/
                                               mn_ethLoopCheckPortAdd(m, n);
                                     }
                            }
                   }
         }
         return VOS_OK;
}

STATUS setGwEthLoopOnuPortShutDownEnable(ULONG set_var)
{
	static qdef_ethchk_cmd_t chkcmd;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};

	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = 1;
	chkcmd.vty = NULL;
	chkcmd.cmdId = ETHCHK_CONTRAL_CMD;


	if(set_var==1)
		VOS_StrCpy( (char *)chkcmd.argv[0], "enable" );
	else
		VOS_StrCpy( (char *)chkcmd.argv[0], "disable" );
	
	aulMsg[3] = (ULONG)&chkcmd;

	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
	}
	
	return CMD_SUCCESS;
}

STATUS setGwEthLoopUpTime(ULONG set_var) 
{
    /*modi by luh 2014-06-27*/
#if 0    
	static qdef_ethchk_cmd_t chkcmd;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};
       int i=0;
       
	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = 2;
	chkcmd.vty = NULL;
	chkcmd.cmdId = ETHCHK_UPTIMES_CMD;

       VOS_Sprintf( (char *)chkcmd.argv[0], "%d",set_var);
	VOS_Sprintf( (char *)chkcmd.argv[1], "%d",chk_config_up_times_thresh);

	aulMsg[3] = (ULONG)&chkcmd;

	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
	}
	else
	{
		return VOS_ERROR;
	}
#else
    mn_relay_up_times = set_var;
#endif
	
	return VOS_OK;
}

STATUS setGwEthLoopUpTimeThreshold(ULONG set_var)
{
	static qdef_ethchk_cmd_t chkcmd;
	ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};
       int i=0;
       
	VOS_MemZero( &chkcmd, sizeof(qdef_ethchk_cmd_t) );
	chkcmd.argc = 2;
	chkcmd.vty = NULL;
	chkcmd.cmdId = ETHCHK_UPTIMES_CMD;

    VOS_Sprintf( (char *)chkcmd.argv[0], "%d", mn_relay_up_times/*chk_config_up_times*/);
	VOS_Sprintf( (char *)chkcmd.argv[1], "%d",set_var );

	aulMsg[3] = (ULONG)&chkcmd;

	if(VOS_OK == RPU_SendCmd2Loop(chkcmd))
	{
		if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			return CMD_WARNING;
		}
	}
	else
	{
		return VOS_ERROR;
	}
	
	return VOS_OK;
}

/*STATUS setGwEthLoopPonMacTableClearEnable()
{}*/


STATUS getEthLoopAlarmNum(USHORT *alarmNum)
{
	USHORT count = 0;
	ethloop_port_listnode_t *node;
	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while( node->devIdx!=0)
	{
		count++;
		node = node->next;
	}
	VOS_SemGive( ethloop_semid );
	*alarmNum=count;
	return VOS_OK;
}

STATUS getFirstEthLoopDisplayIndex(ULONG *devIdx,ULONG *brdIdx,ULONG *portIdx,ULONG *vid)
{
	ethloop_port_listnode_t *node;

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while( node )
	{
		*devIdx=node->devIdx;
		*brdIdx=node->brdIdx;
		*portIdx=node->portIdx;
		*vid=node->vid ;
		VOS_SemGive( ethloop_semid );
		return VOS_OK;
	}
	VOS_SemGive( ethloop_semid );
	return VOS_ERROR;
}

STATUS getEthLoopDisplayIndex_One(ULONG uldevId,ULONG*devIdx,ULONG*brdIdx,ULONG*portIdx,ULONG *vid)
{
	ethloop_port_listnode_t *node;

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while(node)
	{
		if(node->devIdx==uldevId)
		{
			*devIdx=node->devIdx;
			*brdIdx=node->brdIdx;
			*portIdx=node->portIdx;
			*vid=node->vid;
			VOS_SemGive( ethloop_semid );
			return VOS_OK;
		}
		node=node->next;
	}
	VOS_SemGive( ethloop_semid );
	return VOS_ERROR;
}

STATUS getEthLoopDisplayIndex_Two(ULONG uldevId,ULONG ulBrdId,
										ULONG*devIdx,ULONG*brdIdx,ULONG*portIdx,ULONG *vid)

{
	ethloop_port_listnode_t *node;
	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while(node)
	{
		if((node->devIdx==uldevId)&&(node->brdIdx==ulBrdId))
		{
			*devIdx=node->devIdx;
			*brdIdx=node->brdIdx;
			*portIdx=node->portIdx;
			*vid=node->vid;
			VOS_SemGive( ethloop_semid );
			return VOS_OK;
		}
		node=node->next;
	}
	VOS_SemGive( ethloop_semid );
	return VOS_ERROR;
}

STATUS getEthLoopDisplayIndex_Three(ULONG uldevId,ULONG ulBrdId,ULONG ulPortId,
										ULONG*devIdx,ULONG*brdIdx,ULONG*portIdx,ULONG *vid)

{
	ethloop_port_listnode_t *node;

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while(node)
	{
		if((node->devIdx==uldevId)&&(node->brdIdx==ulBrdId)&&(node->portIdx==ulPortId))
		{
			*devIdx=node->devIdx;
			*brdIdx=node->brdIdx;
			*portIdx=node->portIdx;
			*vid=node->vid;
			VOS_SemGive( ethloop_semid );
			return VOS_OK;
		}
		node=node->next;
	}
	VOS_SemGive( ethloop_semid );
	return VOS_ERROR;
}

STATUS getNextEthLoopDisplayIndex(ULONG devIdx,ULONG brdIdx,ULONG portIdx,ULONG vid,
									ULONG* netDevIdx,ULONG*netBrdIdx,ULONG*netPortIdx,ULONG*netVid)
{
	ethloop_port_listnode_t *node;

	VOS_SemTake( ethloop_semid, WAIT_FOREVER );
	node = eth_port_loop_list;
	while(node)
	{
		if((node->devIdx==devIdx)&&(node->brdIdx==brdIdx)&&(node->portIdx==portIdx)&&(node->vid==vid))
		{
			node=node->next;
			if(node!=NULL)
			{
				*netDevIdx=node->devIdx;
				*netBrdIdx=node->brdIdx;
				*netPortIdx=node->portIdx;
				*netVid=node->vid;
				VOS_SemGive( ethloop_semid );
				return VOS_OK;
			}
			else
			{
				VOS_SemGive( ethloop_semid );/*问题单: 12478*/
				return VOS_ERROR;
			}
		}
		node=node->next;
	}
	VOS_SemGive( ethloop_semid );
	return VOS_ERROR;
}


/*E-added by zhengyt@10-3-18*/

/*add by zhengyt@10-3-31*/
/*新的环路检测方法,当交换板的cpu收到检测包时，调用该函数解析检测包，
如果有环路存在，需要发环路告警,如果使能了control-enable，需要关闭相应的端口*/
/* 在12EPON 上这里直接传送了交换芯片的物理端口 */
STATUS recvLoopChkPacketHook(UCHAR*pBuf, ULONG swport)
{
	ULONG slotNo,portNo;
	ULONG loopdevId=0,loopoltSlot=0,loopoltPort=0,looponuid=0,loopLlid = 0, i=0,vid = 0,loopvid=0;
	ULONG loopportlist1 = 0 , loopportlist2 = 0,onuIfIndex = 0;
	USHORT *tagflag = 0,ErrorFlag = 0;
	/*UCHAR *oltTypeDesc[]={"OLT-6100","OLT-6700"};*/
	short int PonPortIdx;
	UCHAR *p=NULL, OtherOltFlag = 0;
	ethLoopCheckTaggedFrame_t* ulPBuf=NULL;
	ethLoopCheckUntaggedFrame_t* ulPBuf_Untag=NULL;
	UCHAR recv_pkt_Srcmac[ETH_LOOP_MAC_ADDR_LEN] ;
	
	p = pBuf;
    	ulPBuf=(ethLoopCheckTaggedFrame_t*)pBuf;
	tagflag = (USHORT *)(pBuf+2*ETH_LOOP_MAC_ADDR_LEN);
	
	ulPBuf_Untag = (ethLoopCheckUntaggedFrame_t *)ulPBuf;

#ifdef _EPON_12EPON_SUPPORT_
	slotNo = IFM_ETH_GET_SLOT( swport );
	portNo = IFM_ETH_GET_PORT( swport );
	/*begin:added by yanjy 2017-5-3*/
	/*规避，当解析出portNo为0的情况。对12EPON作特殊处理*/
	
	if((0 == portNo) || (slotNo== 0) )
	{
		
		return VOS_OK;
	}
	
	/*end:added by yanjy*/
#else
	swport_2_slot_port(swport, &slotNo, &portNo);
#endif

	if(loopChkDebugSwitch== 3)
	{
			if(*tagflag != 0x8100)		
                    {
                           if(ulPBuf_Untag->onuType==0)	
                               	sys_console_printf("receive Hook loop packet(from olt) :\r\n");
                           else
                                   sys_console_printf("receive Hook loop packet(from onu) :\r\n");
                    }
                    else
                    {
                            if(ulPBuf->onuType==0)
				       sys_console_printf("receive Hook loop packet(From olt , Vid : %d) :\r\n",(ulPBuf->vlanInf)&0x00000fff);
                            else
                                   sys_console_printf("receive Hook loop packet(From onu, Vid : %d) :\r\n",(ulPBuf->vlanInf)&0x00000fff);
                    }
			for(i=0;i<sizeof(ethLoopCheckUntaggedFrame_t)-ETH_LOOP_FRM_PAYLOAD_LEN;i++)
			{
				sys_console_printf("%02x  " ,*p);
				if((i+1)%6==0)
				sys_console_printf("\r\n");
				p++;
			}
	}

	if(*tagflag != 0x8100)/*收到的包不包含vlan tag，这种情况几乎从来不会走进，暂时保留*/
	{
		if(CHECK_SLOTNO_IS_PON(slotNo) && SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && ulPBuf_Untag->onuType != 0)
		{
			VOS_Free(pBuf);
			if(loopChkDebugSwitch == 3)
				sys_console_printf("\r\nDiscard this packet which is from pon port : %d/%d .\r\n",slotNo,portNo);
			return VOS_OK; 
		}
		
	       vid = 0;
		if(ulPBuf_Untag->chkFlag==0x0080)	/*如果cpu收到的包中此位置是0x0080,说明该包是环路检测包*/
		{
			loopChkDbgPrintf(2,("cpu receive loop check packet\r\n"));

			Onu_Eth_Loop_Frame_Num++;
			
			if(ulPBuf_Untag->onuType==0)	/*判定是发自OLT 的检测包*/
			{
				VOS_MemZero(recv_pkt_Srcmac, ETH_LOOP_MAC_ADDR_LEN);
#if 0
				 VOS_MemCpy( recv_pkt_Srcmac, (UCHAR*)(pBuf+ETH_LOOP_MAC_ADDR_LEN), ETH_LOOP_MAC_ADDR_LEN );
				 if(VOS_MemCmp(recv_pkt_Srcmac, chk_config_frame_smac, ETH_LOOP_MAC_ADDR_LEN) != 0)
#else
				VOS_MemCpy( recv_pkt_Srcmac, ulPBuf_Untag->onuMac, ETH_LOOP_MAC_ADDR_LEN );
				if(VOS_MemCmp(recv_pkt_Srcmac, SYS_PRODUCT_BASEMAC, ETH_LOOP_MAC_ADDR_LEN) != 0)
#endif
				 {
					VOS_Free(pBuf);
					return VOS_OK;
				 }
				 loopdevId=OLT_DEV_ID;
				 loopoltSlot=(ulPBuf_Untag->onuDesc>>16);
				 loopoltPort=((ulPBuf_Untag->onuDesc>>8)&0x000000ff);

				 loopvid = ulPBuf_Untag->onuVlan;
				 loopChkDbgPrintf(2,("OLT %d/%d and OLT%d/%d loop\r\n",loopoltSlot,loopoltPort,slotNo,portNo));
      				 addEthPortToLoopList2(OLT_DEV_ID, slotNo, portNo, vid,loopdevId,loopoltSlot,loopoltPort,loopvid, 0, 0, NULL);
			       check_mac_entry_delete(slotNo,portNo,OLT_DEV_ID);
			}
			else	/*如果olt收到了onu发来的环路检测包，按照如下解析报文*/
			{
				ErrorFlag = 0;
				loopvid = ulPBuf_Untag->onuVlan;
				/*loopvid = ((((ulPBuf_Untag->onuVlan)& 0xFF00)>>8) | (((ulPBuf_Untag->onuVlan) & 0x00FF)<<8));*/
				/*vid = ulPBuf->onuVlan;*/
				loopoltSlot=(ulPBuf_Untag->onuDesc>>16);
				loopoltPort=((ulPBuf_Untag->onuDesc>>8)&0x000000ff);
				loopLlid = ((ulPBuf_Untag->onuDesc)&0x000000ff);
#if 0
				if(VOS_MemCmp(recv_pkt_Srcmac, chk_config_frame_smac, ETH_LOOP_MAC_ADDR_LEN) != 0)
/*#else*/
				VOS_MemZero(recv_pkt_Srcmac, ETH_LOOP_MAC_ADDR_LEN);
				VOS_MemCpy( recv_pkt_Srcmac, ulPBuf_Untag->onuMac, ETH_LOOP_MAC_ADDR_LEN );
				if(VOS_MemCmp(recv_pkt_Srcmac, SYS_PRODUCT_BASEMAC, ETH_LOOP_MAC_ADDR_LEN) != 0)
/*#endif*/
				{
					OtherOltFlag = 1;
				}
#endif

				if(loopoltSlot < PONCARD_FIRST || loopoltSlot >PONCARD_LAST || loopoltPort > MAX_PONPORT_PER_BOARD || !LLID_ISVALID(loopLlid))
				{
					ErrorFlag = 1;
					if(loopChkDebugSwitch == 3)
						sys_console_printf("\r\nIn the Frame received, Slot is %d,Port is %d,Llid is %d\r\n",loopoltSlot,loopoltPort,loopLlid );
				}
				if(ErrorFlag == 0)
				{
					PonPortIdx = GetPonPortIdxBySlot(loopoltSlot,loopoltPort);
					if(PonPortIdx == RERROR || loopLlid <1 || loopLlid > MAXONUPERPON )
					{
						/*VOS_ASSERT(0);*/
						if(loopChkDebugSwitch == 3)
							sys_console_printf("\r\nIn the Frame received, Slot is %d,Port is %d,Llid is %d\r\n",loopoltSlot,loopoltPort,loopLlid );
						ErrorFlag = 1;
					}
				}
				
				if(ErrorFlag == 0)
				{
					looponuid = GetOnuIdxByLlid(PonPortIdx,loopLlid);
					if(looponuid == VOS_ERROR)
					{
						/*VOS_ASSERT(0);*/
						if(loopChkDebugSwitch == 3)
						{
							sys_console_printf("\r\nIn the Frame received, Slot is %d,Port is %d,Llid is %d\r\n",loopoltSlot,loopoltPort,loopLlid );
							sys_console_printf("Fetch the onu_id error ,the PonPortIdx is %d and the Onu_id is %d.\r\n",PonPortIdx,looponuid);
						}
						ErrorFlag = 1;
					}
					else
					{
						looponuid = looponuid +1;
						loopdevId = MAKEDEVID(loopoltSlot,loopoltPort,looponuid)/*loopoltSlot*10000+loopoltPort*1000+looponuid*/;
					}
				}
				
				if(ulPBuf_Untag->onuIfIndex != 0 && ErrorFlag == 0)
				{
					onuIfIndex = ((((ulPBuf_Untag->onuIfIndex) & 0xFF000000)>>24) |  (((ulPBuf_Untag->onuIfIndex) & 0x00FF0000)>>8)| (((ulPBuf_Untag->onuIfIndex) & 0x0000FF00)<<8 ) |  (((ulPBuf_Untag->onuIfIndex) & 0x000000FF)<<24));
					loopportlist1 = IFM_ETH_GET_SLOT(onuIfIndex);
					loopportlist2 = IFM_ETH_GET_PORT(onuIfIndex);
				}

				if(ErrorFlag == 1)
				{
					loopdevId = 0 ;
					loopportlist1 = 0 ;
					loopportlist2 = 0 ;
                                   loopvid = 0;
				}
				
				addEthPortToLoopList2(OLT_DEV_ID, slotNo, portNo, vid, loopdevId, loopportlist1, loopportlist2,loopvid, OtherOltFlag, ulPBuf_Untag->oltType, recv_pkt_Srcmac );
				check_mac_entry_delete(slotNo,portNo,OLT_DEV_ID);
				
			}
		}
	}
	else  /*收到的包有vlan标签*/
	{
		if(CHECK_SLOTNO_IS_PON(slotNo)&& SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && ulPBuf->onuType != 0)
		{
			VOS_Free(pBuf);
			if(loopChkDebugSwitch == 3)
				sys_console_printf("\r\nDiscard this packet which is from pon port : %d/%d .\r\n",slotNo,portNo);
			return VOS_OK; 
		}
		
		if(ulPBuf->chkFlag==0x0080)	/*如果cpu收到的包中此位置是0x0080,说明该包是环路检测包*/
		{
			loopChkDbgPrintf(2,("cpu receive loop check packet\r\n"));
			
			Onu_Eth_Loop_Frame_Num++;
			vid = (ulPBuf->vlanInf)&0x00000fff;
			if(ulPBuf->onuType==0)	/*判定是发自OLT 的检测包*/
			{
				VOS_MemZero(recv_pkt_Srcmac, ETH_LOOP_MAC_ADDR_LEN);
#if 0
				 if(VOS_MemCmp(recv_pkt_Srcmac, chk_config_frame_smac, ETH_LOOP_MAC_ADDR_LEN) != 0)
#else
				VOS_MemCpy( recv_pkt_Srcmac, ulPBuf->onuMac, ETH_LOOP_MAC_ADDR_LEN );
				if(VOS_MemCmp(recv_pkt_Srcmac, SYS_PRODUCT_BASEMAC, ETH_LOOP_MAC_ADDR_LEN) != 0)
#endif
				 {
					VOS_Free(pBuf);
					return VOS_OK;
				 }

				 loopdevId=OLT_DEV_ID;
				 loopoltSlot=(ulPBuf->onuDesc>>16);
				 loopoltPort=((ulPBuf->onuDesc>>8)&0x000000ff);
				  
				/* vid = ((((ulPBuf->onuVlan)& 0xFF00)>>8) | (((ulPBuf->onuVlan) & 0x00FF)<<8));*/
				 loopvid = ulPBuf->onuVlan;
				 loopChkDbgPrintf(2,("OLT %d/%d and OLT%d/%d loop\r\n",loopoltSlot,loopoltPort,slotNo,portNo));
      				 addEthPortToLoopList2(OLT_DEV_ID, slotNo, portNo, vid,loopdevId,loopoltSlot,loopoltPort,loopvid, 0,0, NULL );
				check_mac_entry_delete(slotNo,portNo,OLT_DEV_ID);
			}
			else	/*如果olt收到了onu发来的环路检测包，按照如下解析报文*/
			{
				ErrorFlag = 0;
				loopvid = ulPBuf->onuVlan;
				/*((((ulPBuf->onuVlan)& 0xFF00)>>8) | (((ulPBuf->onuVlan) & 0x00FF)<<8));*/
				/*vid = ulPBuf->onuVlan;*/
				loopoltSlot=(ulPBuf->onuDesc>>16);
				loopoltPort=((ulPBuf->onuDesc>>8)&0x000000ff);
				loopLlid = ((ulPBuf->onuDesc)&0x000000ff);

#if 0
				if(VOS_MemCmp(recv_pkt_Srcmac, chk_config_frame_smac, ETH_LOOP_MAC_ADDR_LEN) != 0)
#else
				VOS_MemZero(recv_pkt_Srcmac, ETH_LOOP_MAC_ADDR_LEN);
				VOS_MemCpy( recv_pkt_Srcmac, ulPBuf->onuMac, ETH_LOOP_MAC_ADDR_LEN );
				if(VOS_MemCmp(recv_pkt_Srcmac, SYS_PRODUCT_BASEMAC, ETH_LOOP_MAC_ADDR_LEN) != 0)
#endif
				{
#if 0
					OtherOltFlag = 1;
#else
					VOS_Free(pBuf);
					return VOS_OK;
#endif
				}
				
				if(loopoltSlot < PONCARD_FIRST || loopoltSlot >PONCARD_LAST || loopoltPort > MAX_PONPORT_PER_BOARD || !LLID_ISVALID(loopLlid))
				{
					ErrorFlag = 1;
					if(loopChkDebugSwitch == 3)
						sys_console_printf("\r\nIn the Frame received, Slot is %d,Port is %d,Llid is %d\r\n",loopoltSlot,loopoltPort,loopLlid );
				}

				if(ErrorFlag == 0)
				{
					PonPortIdx = GetPonPortIdxBySlot(loopoltSlot,loopoltPort);
					if(PonPortIdx == RERROR || loopLlid <1 || loopLlid > MAXONUPERPON )
					{
						/*VOS_ASSERT(0);*/
						if(loopChkDebugSwitch == 3)
							sys_console_printf("\r\nIn the Frame received, Slot is %d,Port is %d,Llid is %d\r\n",loopoltSlot,loopoltPort,loopLlid );
						ErrorFlag = 1;
					}
				}
				
				if(ErrorFlag == 0)
				{
					looponuid = GetOnuIdxByLlid(PonPortIdx,loopLlid);
					if(looponuid == VOS_ERROR)
					{
						/*VOS_ASSERT(0);*/
						if(loopChkDebugSwitch == 3)
						{
							sys_console_printf("\r\nIn the Frame received, Slot is %d,Port is %d,Llid is %d\r\n",loopoltSlot,loopoltPort,loopLlid );
							sys_console_printf("Fetch the onu_id error ,the PonPortIdx is %d and the Onu_id is %d.\r\n",PonPortIdx,looponuid);
						}
						ErrorFlag = 1;
					}
					else
					{
						looponuid = looponuid +1;
						loopdevId =MAKEDEVID(loopoltSlot,loopoltPort,looponuid)/* loopoltSlot*10000+loopoltPort*1000+looponuid*/;
					}
				}
				
				if(ulPBuf->onuIfIndex != 0 && ErrorFlag == 0)
				{
					onuIfIndex = ((((ulPBuf->onuIfIndex) & 0xFF000000)>>24) |  (((ulPBuf->onuIfIndex) & 0x00FF0000)>>8)| (((ulPBuf->onuIfIndex) & 0x0000FF00)<<8 ) |  (((ulPBuf->onuIfIndex) & 0x000000FF)<<24));
					loopportlist1 = IFM_ETH_GET_SLOT(onuIfIndex);
					loopportlist2 = IFM_ETH_GET_PORT(onuIfIndex);
				}

				if(ErrorFlag == 1)
				{
					loopdevId = 0 ;
					loopportlist1 = 0 ;
					loopportlist2 = 0 ;
                                   loopvid = 0;
				}
				
				addEthPortToLoopList2(OLT_DEV_ID, slotNo, portNo, vid, loopdevId, loopportlist1, loopportlist2,loopvid, OtherOltFlag, ulPBuf->oltType, recv_pkt_Srcmac );
				check_mac_entry_delete(slotNo,portNo,OLT_DEV_ID);
			}
				
		}
	}
	VOS_Free(pBuf);
	/*cpufc_pdp_free( pBuf);*/
	return VOS_OK;
}

LONG RPU_SendCmd2Loop(qdef_ethchk_cmd_t chkcmd)
{
    RPC_Loop_MsgHead_S *pstSendMsg = NULL;
    RPC_Loop_MsgHead_S *pstRevData = NULL;

    ULONG ulRevLen = 0, ulCmdID = 0;
    ULONG rc = VOS_ERROR , i = 0;
    struct vty *vty;
    /*if(SYS_MODULE_ISMASTERSTANDBY(SYS_LOCAL_MODULE_SLOTNO) || SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)*/
    if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
    {
    	    /*sys_console_printf("%%The card is MasterStandby card,needn't send cmd to lic.\r\n");*/
	    return VOS_OK;
    }

    if(CHECK_SLOTNO_IS_PON(SYS_LOCAL_MODULE_SLOTNO) && SYS_LOCAL_MODULE_WORKMODE_ISSLAVE )
    {
	   sys_console_printf("%%The card is pon card,can't send cmd to lic.\r\n");
          return VOS_ERROR;
    }

    /*if ( !DEV_IsMySelfMaster() )
    {
        sys_console_printf("%%The card is slave card,can't send cmd to lic.\r\n");
        return VOS_ERROR;
     }*/
	vty = chkcmd.vty;
	for(i = 1; i <= eth_loop_check_slot_max ; i++)
	{
		if ( !SYS_MODULE_IS_READY(i) )
		{
			continue;
		}

		if( SYS_LOCAL_MODULE_SLOTNO == i )
			continue;
		
		if(CHECK_SLOTNO_IS_PON(i) && SYS_MODULE_SLOT_ISHAVECPU( i ) )
		{
		    pstSendMsg = ( RPC_Loop_MsgHead_S* ) CDP_SYNC_AllocMsg( sizeof( RPC_Loop_MsgHead_S ), MODULE_LOOPBACK );
		    if ( NULL == pstSendMsg )
		    {
		        ASSERT( 0 );
		        return VOS_ERROR;
		    }
		  
		    pstSendMsg->usSrcSlot = ( USHORT ) DEV_GetPhySlot();
		    pstSendMsg->usDstSlot =  i;  
		    pstSendMsg->ulSrcModuleID = MODULE_LOOPBACK;
		    pstSendMsg->ulDstModuleID = MODULE_LOOPBACK;
		    pstSendMsg->usMsgMode = LOOPTOLIC_REQACK;
		    pstSendMsg->usMsgType = LOOPTOLIC_EXECUTE_CMD;
			#ifdef	_DISTRIBUTE_PLATFORM_
			ulCmdID = RPC_Loop_CmdIDNumber++;
			pstSendMsg->ulCmdID = ulCmdID;
			#endif
		    VOS_MemZero(&(pstSendMsg->chkcmd), sizeof(qdef_ethchk_cmd_t) );
		    VOS_MemCpy( &(pstSendMsg->chkcmd), &chkcmd, sizeof(qdef_ethchk_cmd_t) );
			
		    if ( VOS_OK == CDP_SYNC_Call( MODULE_LOOPBACK, i, MODULE_LOOPBACK, 0,
		                                  pstSendMsg, sizeof( RPC_Loop_MsgHead_S ), ( VOID** ) &pstRevData, &ulRevLen, /*3000*/30000 ) )  
		    {
		     
		        if ( pstRevData )
		        {
		            if ( pstRevData->usSrcSlot != i )
		            {
		                ASSERT( 0 );
		                CDP_SYNC_FreeMsg( pstRevData );
		                continue;
		            }

		            if ( pstRevData->usMsgMode != LOOPTOLIC_ACK_END )
		            {
		                ASSERT( 0 );
		                CDP_SYNC_FreeMsg( pstRevData );
		                continue;
		            }

				if(ulCmdID != pstRevData->ulCmdID)
				{
					ASSERT( 0 );
		                	CDP_SYNC_FreeMsg( pstRevData );
		                	continue;
				}

				if(pstRevData->ResResult == 1)
				{
					rc = VOS_OK;
					if(loopChkDebugSwitch==3)
						sys_console_printf("Send to Slot %d Success !\r\n",i);
				}
				else
				{
					rc = VOS_ERROR;
					/*if(loopChkDebugSwitch == 3)*/
					vty_out(vty,"Send to Slot %d Failed !It executive command Failed on pon card !\r\n",i);
				}
		            CDP_SYNC_FreeMsg( pstRevData );
		            pstRevData = NULL;
		        }
		        else
		        {
		              /*ASSERT( 0 );*/
                            /*if(loopChkDebugSwitch == 3)*/
                            vty_out(vty,"Send to Slot %d Failed !The response data is null !!\r\n",i);
		        }
		    }
		    else
		    {
		        /*ASSERT( 0 );*/
                      /*if(loopChkDebugSwitch == 3)*/
                        vty_out(vty,"Send to Slot %d Failed !!\r\n",i);
		    }
		}
	}
    return VOS_OK;
}


VOID LOOP_CMD2LIC_RPC_Callback( ULONG ulSrcNode, ULONG ulSrcModuleID,
                               VOID * pReceiveData, ULONG ulReceiveDataLen,
                               VOID **ppSendData, ULONG * pulSendDataLen )
{
    RPC_Loop_MsgHead_S * pMsg = NULL, *RecvMsg = NULL;
    /*ULONG aulMsg[4] = {MODULE_LOOPBACK, FC_ETHCHK_COMMAND, 0, 0};*/
    LONG rc = -1;
    LONG ulSrcSlot = -1 , i = 0, ErrorFlag = 0;
    qdef_ethchk_cmd_t chkcmd;
    /*sys_console_printf("\r\nIn function : LOOP_CMD2LIC_RPC_Callback\r\n");*/
    if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
        return ;

    if ( NULL == pReceiveData )
    {
        VOS_ASSERT( 0 );
        return ;
    }
    if ( ulReceiveDataLen != sizeof( RPC_Loop_MsgHead_S ) )
    {
        VOS_ASSERT( 0 );
        return ;
    }

    RecvMsg= (RPC_Loop_MsgHead_S *)pReceiveData;

    ulSrcSlot = RecvMsg->usSrcSlot;

    if(RecvMsg->chkcmd.argc >3)/*现在该模块的命令行参数最多三个*/
    {
    	 VOS_ASSERT(0);
        ErrorFlag = 1;
    }
    if(ErrorFlag == 0)
    {
	    if( RecvMsg->usMsgType == LOOPTOLIC_EXECUTE_CMD )
	    {
			VOS_MemZero(&chkcmd,  sizeof(qdef_ethchk_cmd_t));
			chkcmd.argc = RecvMsg->chkcmd.argc;
			chkcmd.cmdId = RecvMsg->chkcmd.cmdId;
			for( i=0; i<RecvMsg->chkcmd.argc; i++ )
			{
				VOS_MemCpy( chkcmd.argv[i], RecvMsg->chkcmd.argv[i], 64 );
			}
		
			/*aulMsg[3] = (ULONG)&chkcmd;
			if( VOS_QueSend( ethLoopChkQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
			{
				return CMD_WARNING;
			}*/
			switch( chkcmd.cmdId )
			{
				case ETHCHK_ENABLE_CMD:
					rc = ethLoopCheckEnableCommand_Pon( &chkcmd );
					break;
				case ETHCHK_CONTRAL_CMD:
					rc = ethLoopCheckContralCommand( &chkcmd );
					break;
				case ETHCHK_MAC_CLEAN_ENABLE_CMD:
					rc = ethLoopCheckMacCleanCommand( &chkcmd );
					break;
				case ETHCHK_PORT_CMD:
					rc = ethLoopCheckPortCommand_Pon( &chkcmd );
					break;
				case ETHCHK_PORT_CMD_ADD:
					rc = ethLoopCheckPortCommand_Pon_Add( &chkcmd );
					break;
				case ETHCHK_VLAN_CMD:
					rc = ethLoopCheckVlanCommand( &chkcmd );
					break;
				case ETHCHK_INTERVAL_CMD:
					rc = ethLoopCheckIntervalCommand( &chkcmd );
					break;
				case ETHCHK_UPTIMES_CMD:
					rc = ethLoopCheckUptimesCommand(&chkcmd);
					break;
				case ETHCHK_IMMEDIATELY_CMD:
					rc = ethLoopCheckImmediatelyCommand(&chkcmd);
					break;
				default:
					break;
			}
	    }
    }
    pMsg = ( RPC_Loop_MsgHead_S * ) CDP_SYNC_AllocMsg( sizeof( RPC_Loop_MsgHead_S ), MODULE_LOOPBACK );
    if ( pMsg == NULL )
    {
        RPC_Loop_CmdIDNumber = 0;
        VOS_ASSERT( 0 );
        return ;
    }
    pMsg->usSrcSlot = ( USHORT ) DEV_GetPhySlot();
    pMsg->usDstSlot = ulSrcSlot;
    pMsg->ulSrcModuleID = MODULE_LOOPBACK;
    pMsg->ulDstModuleID = MODULE_LOOPBACK;
    pMsg->usMsgMode = LOOPTOLIC_ACK_END;
    if(rc == CMD_SUCCESS)	
    	pMsg->ResResult = 1;
    else
    	pMsg->ResResult = -1;
    pMsg->usMsgType = LOOPTOLIC_EXECUTE_CMD;
    pMsg->ulCmdID = RecvMsg->ulCmdID;

    *ppSendData = pMsg;
    *pulSendDataLen = sizeof( RPC_Loop_MsgHead_S );

    return ;
}

/*
int eventOamMsg_onuEthPortLoop_CDP( ushort_t ponId, ulong_t llId, eventOnuEthLoopMsg_t *pOamMsg )
{
       LoopCDPMsgHead_t *bDatPdpBuf = NULL;
	unsigned int ulLen;
    	SYS_MSG_S      *pMsg       = NULL;

       ulLen=sizeof(LoopCDPMsgHead_t)+sizeof(eventOnuEthLoopMsg_t)+sizeof(SYS_MSG_S);
	pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_LOOPBACK);

       if( NULL == pMsg )
       {
            VOS_ASSERT(0);
            return  VOS_ERROR;
       }
       VOS_MemZero((CHAR *)pMsg, ulLen );

       SYS_MSG_SRC_ID( pMsg )       = MODULE_LOOPBACK;
        SYS_MSG_DST_ID( pMsg )       = MODULE_LOOPBACK;
        SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
        SYS_MSG_FRAME_LEN( pMsg )    = ulLen;
        SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
        SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
        SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
        SYS_MSG_DST_SLOT( pMsg ) = SYS_MASTER_ACTIVE_SLOTNO;

        bDatPdpBuf= (LoopCDPMsgHead_t * ) ( pMsg + 1 );
	bDatPdpBuf->srcslot = SYS_LOCAL_MODULE_SLOTNO ;
	bDatPdpBuf->ponId = ponId;
	bDatPdpBuf->llId = llId;
	bDatPdpBuf->pSendBufLen = sizeof(bDatPdpBuf);
	bDatPdpBuf->pSendBuf = bDatPdpBuf+1;
       VOS_MemCpy(bDatPdpBuf->pSendBuf, pOamMsg, sizeof(eventOnuEthLoopMsg_t));


       if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_LOOP, SYS_MASTER_ACTIVE_SLOTNO, RPU_TID_CDP_LOOP,  0, (VOID *)pMsg, ulLen, MODULE_LOOPBACK ) )
	{
		VOS_ASSERT(0); 
		sys_console_printf("\r\nSend the Onu loop Oam to the Master Failed \r\n");
		CDP_FreeMsg(pMsg);
		return VOS_ERROR;
	}

	return VOS_OK;
}
*/
/*现在此函数只有第一个参数为LOOP_ONU_CLEAR_ENTRY 时，才回使用到第三个参数*/
int onuEthPortLoopReportToMaster(int MsgType, void * node , USHORT ClearFlag)
{
	unsigned int ulLen;
    	SYS_MSG_S      *pMsg       = NULL;
	int nodelen = 0;

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;
	
	if( MsgType >= LOOP_FLAG_END )
		return VOS_ERROR;
	
	if( MsgType < LOOP_ONU_SWITCH_REPORT )
		nodelen = sizeof(ethloop_port_listnode_t);
	else
		nodelen = sizeof(ethloop_listnode_Switch_t);
	
       ulLen= nodelen +sizeof(SYS_MSG_S);
	pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_LOOPBACK);

       if( NULL == pMsg )
       {
            VOS_ASSERT(0);
            return  VOS_ERROR;
       }
       VOS_MemZero((CHAR *)pMsg, ulLen );

       SYS_MSG_SRC_ID( pMsg )       = MODULE_LOOPBACK;
        SYS_MSG_DST_ID( pMsg )       = MODULE_LOOPBACK;
        SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
        SYS_MSG_FRAME_LEN( pMsg )    = ulLen;
        SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
        SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
        SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
        SYS_MSG_DST_SLOT( pMsg ) = SYS_MASTER_ACTIVE_SLOTNO;
	 SYS_MSG_MSG_CODE(pMsg) = MsgType;
	 pMsg->usReserved = ClearFlag;
        VOS_MemCpy((void *)(pMsg + 1), (void *)node, nodelen);


       if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_ETHLOOP, SYS_MASTER_ACTIVE_SLOTNO, RPU_TID_CDP_ETHLOOP, /*CDP_MSG_TM_ASYNC*/ 0,\
                 (VOID *)pMsg, ulLen, MODULE_LOOPBACK ) )
	{
		VOS_ASSERT(0); 
		sys_console_printf("\r\nSend the Onu loop Oam to the Master Failed \r\n");
		CDP_FreeMsg(pMsg);
		return VOS_ERROR;
	}
	/*else
	 {
		sys_console_printf("Send the Onu loop Oam to the Master successed ! \r\n");
	 }*/

	return VOS_OK;

}


int EthPortLoop_Poncard_Insert( LONG slotno, LONG module_type  )
{
       LoopInsertCDPMsgHead_t *bDatPdpBuf = NULL;
	unsigned int ulLen = 0,j = 0, i = 0 ;
    	SYS_MSG_S      *pMsg       = NULL;

	if (module_type <= MODULE_TYPE_UNKNOW)
		return VOS_OK;

	if( SYS_LOCAL_MODULE_SLOTNO == slotno )
		return VOS_OK;
	
	Init_Ethloop_PortNum( slotno );
#if 0
	if( SYS_MODULE_TYPE_IS_6900_EPON(module_type) || SYS_MODULE_TYPE_IS_8000_EPON(module_type))
#else
    if(SYS_MODULE_TYPE_IS_CPU_PON(module_type))
#endif
	{
	       if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsDefault  || ETHLOOPPORTLISTFLAG == EthLoopPortlistIsAll )
	       {
	              for( j=1; j<=eth_loop_check_port_max[slotno]; j++ )
	              {
	       		chk_config_eth_portlist[slotno][j] = 1;
	        	}
	       }
		if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsDesigned)
		{
			if(ethloop_devsm_pull_flag[slotno][0] == eth_devsm_pulled)
			{
				if(ethloop_devsm_pull_flag[slotno][1] == ethloop_devsm_pon)
				{
					ethloop_devsm_pull_flag[slotno][0] = eth_devsm_normal;
					ethloop_devsm_pull_flag[slotno][1] = ethloop_devsm_zero;
				}
				else if(ethloop_devsm_pull_flag[slotno][1] == ethloop_devsm_eth)
				{
					ethloop_devsm_pull_flag[slotno][0] = eth_devsm_difference;
				}
			}
		}
	       ulLen=sizeof(LoopInsertCDPMsgHead_t)+sizeof(SYS_MSG_S);
		pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_LOOPBACK);

		/*sys_console_printf("\r\n\r\nIn Function : EthPortLoop_Poncard_Insert\r\n\r\n");*/
	       if( NULL == pMsg )
	       {
	            VOS_ASSERT(0);
	            return  VOS_ERROR;
	       }
	       VOS_MemZero((CHAR *)pMsg, ulLen );

	       SYS_MSG_SRC_ID( pMsg )       = MODULE_LOOPBACK;
	        SYS_MSG_DST_ID( pMsg )       = MODULE_LOOPBACK;
	        SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
	        SYS_MSG_FRAME_LEN( pMsg )    = sizeof(LoopInsertCDPMsgHead_t);
	        SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
	        SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
	        SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
	        SYS_MSG_DST_SLOT( pMsg ) = SYS_MASTER_ACTIVE_SLOTNO;
		 SYS_MSG_MSG_CODE(pMsg) = LOOP_PONCARD_INSERT ;
		 
	       bDatPdpBuf= (LoopInsertCDPMsgHead_t * ) ( pMsg + 1 );
		bDatPdpBuf->enable = chk_config_detect_enable;
		bDatPdpBuf->mode = chk_config_detect_mode;
		bDatPdpBuf->controlen_able = chk_config_detect_control_enable;
		bDatPdpBuf->cleanmac_able = chk_config_detect_mac_clean_enable;
		bDatPdpBuf->interval_time = chk_config_timer_len;
		bDatPdpBuf->uptimes = chk_config_up_times;
		bDatPdpBuf->uptime_thre = chk_config_up_times_thresh;
		bDatPdpBuf->vlan = 0;
		bDatPdpBuf->EthPortListFlag = ETHLOOPPORTLISTFLAG;
		VOS_MemCpy(bDatPdpBuf->mac , chk_config_frame_smac, ETH_LOOP_MAC_ADDR_LEN);

		VOS_MemZero(bDatPdpBuf->eth_portlist, CHK_SLOT_PORT_MAX+2);
		VOS_MemCpy(bDatPdpBuf->eth_portlist, chk_config_eth_portlist[slotno],CHK_SLOT_PORT_MAX+2);
		
	       if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_ETHLOOP, slotno, RPU_TID_CDP_ETHLOOP, /*CDP_MSG_TM_ASYNC*/ 0,\
	                 (VOID *)pMsg, ulLen, MODULE_LOOPBACK ) )
		{
			VOS_ASSERT(0); 
			sys_console_printf("\r\n%%Send the loop-detection configuration to the new poncard Failed !\r\n");
			CDP_FreeMsg(pMsg);
			return VOS_ERROR;
		}
	}
	else if(MODULE_E_GFA6900_GEM_10GE == module_type || MODULE_E_GFA6900_GEM_GE == module_type
		/*|| MODULE_E_GFA8000_GEM_10GE == module_type || MODULE_E_GFA8000_GEM_GE == module_type*/
            || MODULE_E_GFA_GET == module_type|| MODULE_E_GFA_GEM == module_type)
	{
		if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsAll)
		{
			for( j=1; j<=eth_loop_check_port_max[slotno]; j++ )
	              {
	       		chk_config_eth_portlist[slotno][j] = 1;
	        	}
		}

		if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsDesigned)
		{
			if(ethloop_devsm_pull_flag[slotno][0] == eth_devsm_pulled)
			{
				if(ethloop_devsm_pull_flag[slotno][1] == ethloop_devsm_eth)
				{
					ethloop_devsm_pull_flag[slotno][0] = eth_devsm_normal;
					ethloop_devsm_pull_flag[slotno][1] = ethloop_devsm_zero;
				}
				else if(ethloop_devsm_pull_flag[slotno][1] == ethloop_devsm_pon)
				{
					ethloop_devsm_pull_flag[slotno][0] = eth_devsm_difference;
				}
			}
		}
	}

	if(loopChkDebugSwitch== 3)
	{
		sys_console_printf("\r\n module_type is %d, eth_loop_check_port_max[%d] is %d", module_type, slotno, eth_loop_check_port_max[slotno] );
		
		for ( i= 1; i<= eth_loop_check_port_max[slotno]; i++ )
		{	
			sys_console_printf(" port(%d) : %d", i, chk_config_eth_portlist[slotno][i] );
		}
		sys_console_printf("\r\n");
	}
	
	return VOS_OK;

}



int EthPortLoop_Poncard_PullOut( LONG slotno, LONG module_type  ) /*问题单: 11884*/
{
	unsigned int j = 0 ;

	if (module_type <= MODULE_TYPE_UNKNOW)
		return VOS_OK;
	eth_loop_check_port_max[ slotno ] = 4 ;
	
	/*if( SYS_MODULE_IS_UPLINK( slotno ))*/
	{
		if( 1 /*ETHLOOPPORTLISTFLAG == EthLoopPortlistIsAll*/ )/* 针对10GE 的情况*/
		{
			if( module_type == MODULE_E_GFA6900_GEM_10GE /*|| module_type == MODULE_E_GFA8000_GEM_10GE*/)
				chk_config_eth_portlist[slotno][5] = 0;
			
			if( SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6100 )
			{
				if(slotno == 2 || slotno == 3)
					eth_loop_check_port_max[ slotno ] = 2 ;
			}
		}
	}
#if 0		
	if( SYS_MODULE_TYPE_IS_6900_EPON(module_type) || SYS_MODULE_TYPE_IS_8000_EPON(module_type))
#else
    if(SYS_MODULE_TYPE_IS_CPU_PON(module_type))
#endif
	{
	       if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsDefault || ETHLOOPPORTLISTFLAG == EthLoopPortlistIsAll )
	       {
	              for( j=1; j<=eth_loop_check_port_max[slotno]; j++ )
	              {
	       		chk_config_eth_portlist[slotno][j] = 0;
	        	}
	       }

		if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsDesigned)
		{
			if(ethloop_devsm_pull_flag[slotno][0] == eth_devsm_normal )
	              {
				ethloop_devsm_pull_flag[slotno][0] = eth_devsm_pulled;
				ethloop_devsm_pull_flag[slotno][1] = ethloop_devsm_pon;
		       }
			else if(ethloop_devsm_pull_flag[slotno][0] == eth_devsm_difference )
			{
				ethloop_devsm_pull_flag[slotno][0] = eth_devsm_pulled;
			}
		}

	}
	else if(MODULE_E_GFA6900_GEM_10GE == module_type || MODULE_E_GFA6900_GEM_GE == module_type
		/*|| MODULE_E_GFA8000_GEM_10GE == module_type || MODULE_E_GFA8000_GEM_GE == module_type*/
            || MODULE_E_GFA_GET == module_type|| MODULE_E_GFA_GEM == module_type)
	{
		if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsAll)
	       {
	              for( j=1; j<=eth_loop_check_port_max[slotno]; j++ )
	              {
	       		chk_config_eth_portlist[slotno][j] = 0;
	        	}
	       }

		if(ETHLOOPPORTLISTFLAG == EthLoopPortlistIsDesigned)
	       {
	              if(ethloop_devsm_pull_flag[slotno][0] == eth_devsm_normal )
	              {
				ethloop_devsm_pull_flag[slotno][0] = eth_devsm_pulled;
				ethloop_devsm_pull_flag[slotno][1] = ethloop_devsm_eth;
		       }
			else if(ethloop_devsm_pull_flag[slotno][0] == eth_devsm_difference )
			{
				ethloop_devsm_pull_flag[slotno][0] = eth_devsm_pulled;
			}
	       }
	}

	delEthPortFromLoopListBySlot(slotno);

	return VOS_OK;

}

STATUS getGwEthLoopPortList(uchar *pVar)
{
	int i=1,j=1,k=0,m=0;

	VOS_MemZero(pVar, Mib_PortList_Len);
	
	for(i = 1 ; i<= eth_loop_check_slot_max;i++)
	{
		for( j = 1; j<=eth_loop_check_port_max[i]; j++ )
		{
			if( chk_config_eth_portlist[i][j] )
			{
				/*if( (CHECK_SLOTNO_IS_ETH(i) )||( CHECK_SLOTNO_IS_PON(i) ))
				{*/
					k=((i-1)*MN_SNMP_MAX_PORTS_ONE_SLOT+(j-1))/8;
					m=((i-1)*MN_SNMP_MAX_PORTS_ONE_SLOT+(j-1))%8;
					/*sys_console_printf("i = %d, j = %d\r\n", i, j);*/
					pVar[k]|=(0x80>>m);
				/*}*/
			}
		}
		j = 1;
	}
#if 0
	sys_console_printf("\r\n In Function: getGwEthLoopPortList \r\n");
	for(i = 0; i < MN_SNMP_MAX_PORTLIST_LEN; i++)
		sys_console_printf(" %d ", pVar[i]);
#endif
	return VOS_OK;
}

