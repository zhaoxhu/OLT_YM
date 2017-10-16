#ifdef __cplusplus
extern "C"{
#endif

#include "gwEponSys.h"
#include "lib_gwEponMib.h"
#include "gweponmibdata.h"
/*#include "sys/main/sys_main.h"
#include "man/cli/cli.h"
#include "device/device_manage.h"*/
#include "bmsp/product_info/Bms_product_info.h"

#include "statistics/statistics.h"
#include "monitor/monitor.h"
#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include "ExtBoardType.h"
#include "Cdp_pub.h"
#include  "Onu_manage.h"
#include  "Onu_oam_comm.h"


#define ONU_EXT_QUERY_WAIT	0
#define ONU_EXT_QUERY_OK		1	/* added by xieshl 20080814, for test */


#undef EXT_MGT_DEBUG
#ifdef EXT_MGT_DEBUG
#define ext_board_debug_printf(x)		sys_console_printf x
#else
#define ext_board_debug_printf(x)		
#endif

#define ext_board_debug(x)		if( EVENT_EUQ == V2R1_ENABLE ) sys_console_printf x


typedef enum {
	ONU_EXT_BRD_MIB_TYPE_GET = 1,
	ONU_EXT_BRD_SUPPORT_MIB_TYPE_GET,
	ONU_EXT_BRD_OPER_STATUS_GET,
	ONU_EXT_BRD_CHG_TIME_GET,
	ONU_EXT_BRD_DESC_GET,
	ONU_EXT_BRD_DESC_SET,
	ONU_EXT_BRD_SW_VER_GET,
	ONU_EXT_BRD_FW_VER_GET,
	ONU_EXT_BRD_HW_VER_GET,
	ONU_EXT_BRD_BOOT_VER_GET,
	ONU_EXT_BRD_MANUFACTURE_GET,
	ONU_EXT_BRD_SN_GET,
	ONU_EXT_BRD_PRODUCT_TIME_GET,
	ONU_EXT_BRD_HAS_SNMP_GET,
	ONU_EXT_BRD_SNMP_IP_GET,
	ONU_EXT_BRD_SNMP_IP_SET,
	ONU_EXT_BRD_RD_COMMUNITY_GET,
	ONU_EXT_BRD_RD_COMMUNITY_SET,
	ONU_EXT_BRD_WR_COMMUNITY_GET,
	ONU_EXT_BRD_WR_COMMUNITY_SET,
	ONU_EXT_DEV_ONU_REG,
	ONU_EXT_DEV_ONU_DEREG,
	ONU_EXT_DEV_BRD_NUM_GET,
	ONU_EXT_DEV_BRD_LIST_SET,
	ONU_EXT_DEV_BRD_INSERT,
	ONU_EXT_DEV_BRD_PULL
}onu_ext_code_t;


LONG ExtBrd_CommandInstall();

extern short int GetPonPortIdxBySlot( short int slot, short  int port );
extern int GetOnuType( short int PonPortIdx, short int OnuIdx, int *type );
extern  int EQU_SendMsgToOnu( short int PonPortIdx, short int OnuIdx, char  MsgType, unsigned char *pBuf, int length);
extern int parse_onuidx_command_parameter( ULONG devIdx, PON_olt_id_t *pPonIdx, PON_onu_id_t *pOnuIdx);
extern LONG PON_ParseSlotPortOnu( CHAR * szName, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern LONG PON_GetSlotPortOnu( ULONG ulIfIndex, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern STATUS getBoardHasSnmpAgent(ULONG devIdx, ULONG brdIdx, ulong_t * hasSnmpAgent);
extern LONG devsm_sys_is_switchhovering();
extern int reigster_onuevent_callback(int code, g_OnuEventFuction_callback function);

/* modified by xieshl 20120918, 解决插卡式ONU(判断条件是类型为GT861或上报的槽位数大于1的ONU)管理占用
    内存过大问题，扩展板卡管理改为链表方式，一个节点表示一个GT861，注册时创建，离线时
    释放。GT861注册时只根据其设备信息创建节点，不读扩展信息，不会对目前的ONU注册产生影
    响。当网管或其它模块访问到扩展管理信息时，只有第一次访问时才通过OAM向ONU读取，其它
    时间的访问都直接用本地的缓存*/
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
LONG onuExtMgmt_OnuRegCallback( OnuEventData_s data );
LONG onuExtMgmt_OnuDeregCallback( OnuEventData_s data );
LONG onuExtMgmt_BrdInsertCallback( ULONG devIdx, ULONG brdIdx, UCHAR *pBrdPduData );
LONG onuExtMgmt_BrdPullCallback( ULONG devIdx, ULONG brdIdx );

static LONG onu_ext_dev_mgt_pdu_parase( short int PonPortIdx, short int OnuIdx, onu_ext_pdu_t *pPdu );
static LONG onu_ext_brd_hot_pdu_parase( short int PonPortIdx, short int OnuIdx, ULONG brdIdx, UCHAR *pBrdPduData );
static LONG onu_ext_dev_mgt_pdu_create( short int PonPortIdx, short int OnuIdx, onu_ext_pdu_t *pPdu, ULONG *pPduLen );
static LONG onu_ext_dev_sync_2_master( short int PonPortIdx, short int OnuIdx );
static LONG onu_ext_dev_sync_req_2_slave( short int PonPortIdx, short int OnuIdx );
static ULONG onu_ext_brd_mib_type_get( int oamData );

static onu_ext_table_t  * gpOnuExtMgmtTable[SYS_MAX_PON_PORTNUM];
ULONG onuExtMSemId = 0;
ULONG onuExtBSemId = 0;
ULONG onuExtSyncCallFlag = 0;


static onu_ext_table_t * onu_ext_dev_new( short int PonPortIdx, short int OnuIdx )
{
	LONG onuBrdIdx;
	onu_ext_table_t *pTemp = NULL;
	onu_ext_table_t *pCurr = NULL;

	if( !OLT_LOCAL_ISVALID(PonPortIdx) || !ONU_IDX_ISVALID(OnuIdx) )
	{
		VOS_ASSERT(0);
		return pTemp;
	}

	/*VOS_SemTake( onuExtMSemId, WAIT_FOREVER );*/

	pCurr = gpOnuExtMgmtTable[PonPortIdx];
	while( pCurr )
	{
		if( pCurr->OnuIdx == OnuIdx )
		{
			/*pCurr->QueryFlag = ONU_EXT_QUERY_WAIT;
			VOS_MemZero( pCurr->BrdMgmtTable, sizeof(pCurr->BrdMgmtTable) );
			VOS_SemGive( onuExtMSemId );*/
		
			return pCurr;
		}
		pTemp = pCurr;
		pCurr = pCurr->next;
	}

	pCurr = (onu_ext_table_t *)VOS_Malloc(sizeof(onu_ext_table_t), MODULE_ONU);
	if( pCurr  )
	{
		VOS_MemZero( pCurr, sizeof(onu_ext_table_t) );
		for( onuBrdIdx=1; onuBrdIdx<=MAX_ONU_BRD_NUM; onuBrdIdx++ )
		{
			pCurr->BrdMgmtTable[onuBrdIdx].DeviceInfo.OperatStatus = 1;
			pCurr->BrdMgmtTable[onuBrdIdx].DeviceInfo.ChangeTime = VOS_GetTick();
		}
		pCurr->PonPortIdx = PonPortIdx;
		pCurr->OnuIdx = OnuIdx;
		pCurr->next = NULL;
		
		if( pTemp )
		{
			pTemp->next = pCurr;
		}
		else
		{
			gpOnuExtMgmtTable[PonPortIdx] = pCurr;
		}
	}
	/*VOS_SemGive( onuExtMSemId );*/
	
	return pCurr;
}

static LONG onu_ext_dev_delete( short int PonPortIdx, short int OnuIdx )
{
	onu_ext_table_t *pTemp = NULL;
	onu_ext_table_t *pCurr = NULL;

	if( !OLT_LOCAL_ISVALID(PonPortIdx) || !ONU_IDX_ISVALID(OnuIdx) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	VOS_SemTake( onuExtMSemId, WAIT_FOREVER );

	pCurr = gpOnuExtMgmtTable[PonPortIdx];
	while( pCurr )
	{
		if( pCurr->OnuIdx == OnuIdx )
		{
			break;
		}
		pTemp = pCurr;
		pCurr = pCurr->next;
	}

	if( pCurr )
	{
		if( pTemp )
		{
			pTemp->next = pCurr->next;
			VOS_Free( pCurr );
		}
		else		/* 第一个 */
		{
			gpOnuExtMgmtTable[PonPortIdx] = pCurr->next;
			VOS_Free( pCurr );
		}
	}
	VOS_SemGive( onuExtMSemId );
	
	return VOS_OK;
}

static onu_ext_table_t * onu_ext_dev_search( short int PonPortIdx, short int OnuIdx )
{
	onu_ext_table_t *pCurr = NULL;
	if( OLT_LOCAL_ISVALID(PonPortIdx) && ONU_IDX_ISVALID(OnuIdx) )
	{
		pCurr = gpOnuExtMgmtTable[PonPortIdx];
		while( pCurr )
		{
			if( /*(pCurr->PonPortIdx == PonPortIdx) ||*/ (pCurr->OnuIdx == OnuIdx) )
				break;
			
			pCurr = pCurr->next;
		}
	}
	return pCurr;
}

static onu_ext_brd_table_t * onu_ext_brd_search( short int PonPortIdx, short int OnuIdx, ULONG brdIdx )
{
	onu_ext_table_t *pOnuExtTbl = NULL;
	onu_ext_brd_table_t *pExtBrdTbl = NULL;
	
	if( !ONU_BRD_IDX_IS_VALID(brdIdx) )
		return NULL;

	if( (pOnuExtTbl = onu_ext_dev_search(PonPortIdx, OnuIdx)) != NULL )
	{
		pExtBrdTbl = &pOnuExtTbl->BrdMgmtTable[brdIdx-1];
	}
	return pExtBrdTbl;
}

static LONG onu_ext_dev_query_declare( short int PonPortIdx, short int OnuIdx )
{
	LONG rc = VOS_ERROR;
	onu_ext_table_t *pOnuExtTbl = NULL;

	VOS_SemTake( onuExtMSemId, WAIT_FOREVER );
	pOnuExtTbl = onu_ext_dev_new( PonPortIdx, OnuIdx );
	if( pOnuExtTbl )
	{
		pOnuExtTbl->QueryFlag = ONU_EXT_QUERY_WAIT;
		rc = VOS_OK;
	}
	VOS_SemGive( onuExtMSemId );
	return rc;
}
static LONG onu_ext_dev_query_over( short int PonPortIdx, short int OnuIdx )
{
	LONG rc = VOS_ERROR;
	onu_ext_table_t *pOnuExtTbl = NULL;

	VOS_SemTake( onuExtMSemId, WAIT_FOREVER );
	pOnuExtTbl = onu_ext_dev_search( PonPortIdx, OnuIdx );
	if( pOnuExtTbl )
	{
		pOnuExtTbl->QueryFlag = ONU_EXT_QUERY_OK;
		rc = VOS_OK;
	}
	VOS_SemGive( onuExtMSemId );
	return rc;
}

static BOOL onu_ext_dev_is_ready( short int PonPortIdx, short int OnuIdx )
{
	BOOL rc = FALSE;
	onu_ext_table_t *pOnuExtTbl = NULL;

	VOS_SemTake( onuExtMSemId, WAIT_FOREVER );
	pOnuExtTbl = onu_ext_dev_search( PonPortIdx, OnuIdx );
	/*pOnuExtTbl = onu_ext_dev_new( PonPortIdx, OnuIdx );*/
	if( pOnuExtTbl )
	{
		/*如果查询标志是0，第一次查询onu信息，要给onu发oam帧并保存*/
		if( pOnuExtTbl->QueryFlag == ONU_EXT_QUERY_OK )
			rc = TRUE;
	}
	VOS_SemGive( onuExtMSemId );
	return rc;
}

static LONG onu_ext_dev_read_local( short int PonPortIdx, short int OnuIdx )
{
	LONG rc = VOS_ERROR;
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	UCHAR txBuf[16];
	short int txLen = 2, rxLen = 0;
	onu_ext_pdu_t extPdu;

	if( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
		return VOS_OK;

	if( !onu_ext_dev_support(PonPortIdx, OnuIdx) )
		return rc;

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
		return rc;

	if( onu_ext_dev_is_ready(PonPortIdx, OnuIdx) )
		return VOS_OK;

	/*发oam帧查询onu设备信息*/
	txBuf[0] = 0;/*GET_EXT_ONU_SYS_INFO_REQ;*/
	txBuf[1] = 0;
	txBuf[2] = 0;

#if 0
	if( testReadExtOnuOam( PonPortIdx, OnuIdx, (VOID*)&extPdu ) == VOS_OK )
	{
#else
	if( Oam_Session_Send(PonPortIdx, OnuIdx, GET_EXT_ONU_SYS_INFO_REQ, OAM_SYNC, 0, 0, NULL, txBuf, txLen, (UCHAR*)&extPdu, &rxLen) == VOS_OK )
	{
#endif
		rc = onu_ext_dev_mgt_pdu_parase( PonPortIdx, OnuIdx, &extPdu );
		if( rc != VOS_ERROR )
		{
			/* 向主控、备用主控 同步扩展信息 */
			if( rc > 0 )
				onu_ext_dev_sync_2_master( PonPortIdx, OnuIdx );
			rc = VOS_OK;
		}
	}

	if( rc == VOS_ERROR )
		sys_console_printf("\r\n get onu %d/%d/%d ext-info. ERROR\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuIdx+1 );

#endif
	return rc;
}

static LONG onu_ext_dev_read( short int PonPortIdx, short int OnuIdx )
{
	LONG rc = VOS_ERROR;

	/*如果查询标志没有置位，需要重新从ONU读取 */
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		rc = onu_ext_dev_read_local( PonPortIdx, OnuIdx );
	}
	else
	{
		/* <<<< 通知PON板读ONU扩展信息 */
		rc = onu_ext_dev_sync_req_2_slave( PonPortIdx, OnuIdx );
		
		rc = VOS_OK;
	}
	
	return rc;
}

static LONG onu_ext_dev_cdp_send( ULONG to_slotno, short int PonPortIdx, short int OnuIdx )
{
	LONG rc = VOS_ERROR;
	ULONG msgLen = sizeof(onu_sync_ext_msg_t);
	ULONG pduLen = 0;
	onu_sync_ext_msg_t  *pMsg = NULL;

   	if( !onu_ext_dev_support(PonPortIdx, OnuIdx) )
		return rc;

	ext_board_debug_printf( ("onu_ext_dev_cdp_send to slot %d\r\n", to_slotno) );

	pMsg = ( onu_sync_ext_msg_t * ) CDP_AllocMsg( msgLen, MODULE_ONU );              
	if(pMsg == NULL)
	{
		VOS_ASSERT( 0 );
		return rc;
	}

	/*将一部分管理信息从PON板上传到 主控板上*/
	
	pMsg->OnuSyncMsgHead.ponSlotIdx = GetCardIdxByPonChip(PonPortIdx);/*SYS_LOCAL_MODULE_SLOTNO;*/
	pMsg->OnuSyncMsgHead.portIdx = GetPonPortByPonChip(PonPortIdx);
	pMsg->OnuSyncMsgHead.onuIdx = OnuIdx;
	pMsg->OnuSyncMsgHead.onuEventId = ONU_EVENT_EXT_MGT_SYNC;

	if( (rc = onu_ext_dev_mgt_pdu_create(PonPortIdx, OnuIdx, &(pMsg->ExtPdu), &pduLen)) == VOS_OK )
	{
		rc = CDP_Send( RPU_TID_CDP_ONU, to_slotno,  RPU_TID_CDP_ONU,  CDP_MSG_TM_ASYNC, pMsg, msgLen, MODULE_ONU );
	}	
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pMsg );
		VOS_ASSERT(0);
	}
	 
	return rc;
}

static int onu_ext_dev_cdp_relay( ULONG to_slotno, onu_sync_ext_msg_t  *pExtMsg )
{
	LONG rc = VOS_ERROR;
	LONG msglen=sizeof(onu_sync_ext_msg_t);
	onu_sync_ext_msg_t  *pRelayMsg = NULL;

	if( pExtMsg == NULL )
		return rc;
	pRelayMsg = ( onu_sync_ext_msg_t * ) CDP_AllocMsg( msglen, MODULE_ONU );              
	if(pRelayMsg == NULL)
	{
		VOS_ASSERT( 0 );
		return rc;
	}

	ext_board_debug_printf( ("onu_ext_dev_cdp_relay to slot %d\r\n", to_slotno) );

	VOS_MemCpy( pRelayMsg, pExtMsg, msglen );
	
	rc = CDP_Send( RPU_TID_CDP_ONU, to_slotno,  RPU_TID_CDP_ONU,  CDP_MSG_TM_ASYNC, pRelayMsg, msglen,MODULE_ONU);
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pRelayMsg );
	}	
	return rc;
}

static LONG onu_ext_dev_sync_rep_handle(onu_sync_ext_msg_t  *pExtMsg)
{
	LONG rc = VOS_ERROR;
	short int  pon_slotno;
	short int  pon_portno;
	short int  PonPortIdx;
	short int  OnuIdx;

	if( pExtMsg == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}
	if ( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
	{
		VOS_ASSERT(0);
		return VOS_OK;
	}

	pon_slotno = pExtMsg->OnuSyncMsgHead.ponSlotIdx;
	pon_portno = pExtMsg->OnuSyncMsgHead.portIdx;
	OnuIdx = pExtMsg->OnuSyncMsgHead.onuIdx;
	PonPortIdx = GetPonPortIdxBySlot(pon_slotno, pon_portno);

	if( (!OLT_LOCAL_ISVALID(PonPortIdx)) || (!ONU_IDX_ISVALID(OnuIdx)) )
	{
		VOS_ASSERT(0);
		return rc;
	}
	
	ext_board_debug( ("ONU-EXT:recv onu%d/%d/%d register data sync from slot%d\r\n", pon_slotno, pon_portno, OnuIdx+1, pon_slotno) );

	rc = onu_ext_dev_mgt_pdu_parase( PonPortIdx, OnuIdx, &pExtMsg->ExtPdu );

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE && (rc != VOS_ERROR) )
	{
		ULONG to_slotno;

		if( onuExtSyncCallFlag )
		{
			VOS_SemGive( onuExtBSemId );
			onuExtSyncCallFlag--;
		}
		
		to_slotno = device_standby_master_slotno_get();
		if( to_slotno )
			onu_ext_dev_cdp_relay( to_slotno, pExtMsg );
	}
	return rc;
}

static LONG onu_ext_dev_sync_2_master( short int PonPortIdx, short int OnuIdx )
{
	LONG rc = VOS_OK;
	ULONG to_slotno;
	
	CHECK_ONU_RANGE;
	
	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		to_slotno = device_standby_master_slotno_get();
	}
	else if ( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
	{
		return rc;
	}
	else
	{
		to_slotno = SYS_MASTER_ACTIVE_SLOTNO;
	}
	if( (to_slotno == 0) || (to_slotno == SYS_LOCAL_MODULE_SLOTNO) )
		return rc;

	if( (!OLT_LOCAL_ISVALID(PonPortIdx)) || (!ONU_IDX_ISVALID(OnuIdx)) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	if( !(SYS_MODULE_IS_READY(to_slotno) || devsm_sys_is_switchhovering()) )
		return rc;

	rc = onu_ext_dev_cdp_send( to_slotno, PonPortIdx, OnuIdx );
	
	ext_board_debug( ("ONU-EXT:sync onu%d/%d/%d ext data to master slot%d %s\r\n", GetCardIdxByPonChip(PonPortIdx), 
				GetPonPortByPonChip( PonPortIdx ), OnuIdx+1, to_slotno, (rc == VOS_OK ? "OK" : "ERR")) );

	return rc;
}

static LONG onu_ext_dev_sync_req_2_slave( short int PonPortIdx, short int OnuIdx )
{
	LONG rc = VOS_ERROR;
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	ULONG to_slotno;
	ULONG msgLen = sizeof(onu_sync_msg_head_t);
	onu_sync_msg_head_t  *pMsg = NULL;

	if( !onu_ext_dev_support(PonPortIdx, OnuIdx) )
		return rc;

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
		return rc;

	if( onu_ext_dev_is_ready(PonPortIdx, OnuIdx) )
		return VOS_OK;

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		return rc;
	}
	if( (!OLT_LOCAL_ISVALID(PonPortIdx)) || (!ONU_IDX_ISVALID(OnuIdx)) )
	{
		VOS_ASSERT(0);
		return rc;
	}
	to_slotno = GetCardIdxByPonChip( PonPortIdx );
	if( to_slotno == VOS_ERROR )
		return rc;
	if( (to_slotno == 0) || (to_slotno == SYS_LOCAL_MODULE_SLOTNO) )
		return rc;

	if( !(SYS_MODULE_IS_READY(to_slotno) || devsm_sys_is_switchhovering()) )
		return rc;

   	if( !onu_ext_dev_support(PonPortIdx, OnuIdx) )
		return rc;

	pMsg = ( onu_sync_msg_head_t * ) CDP_AllocMsg( msgLen, MODULE_ONU );              
	if(pMsg == NULL)
	{
		VOS_ASSERT( 0 );
		return rc;
	}

	/*将一部分管理信息从PON板上传到 主控板上*/
	
	pMsg->ponSlotIdx = GetCardIdxByPonChip(PonPortIdx);/*SYS_LOCAL_MODULE_SLOTNO;*/
	pMsg->portIdx = GetPonPortByPonChip(PonPortIdx);
	pMsg->onuIdx = OnuIdx;
	pMsg->onuEventId = ONU_EVENT_EXT_MGT_REQ;

	ext_board_debug( ("ONU-EXT:send onu%d/%d/%d ext request to slot %d\r\n", GetCardIdxByPonChip(PonPortIdx), 
				GetPonPortByPonChip( PonPortIdx ), OnuIdx+1, to_slotno) );

	rc = CDP_Send( RPU_TID_CDP_ONU, to_slotno,  RPU_TID_CDP_ONU,  CDP_MSG_TM_ASYNC, pMsg, msgLen, MODULE_ONU );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pMsg );
		VOS_ASSERT(0);
	}
	else
	{
		onuExtSyncCallFlag++;
		if( VOS_SemTake(onuExtBSemId, 1000) == VOS_ERROR )
		{
			if( onuExtSyncCallFlag ) onuExtSyncCallFlag--;
			
			sys_console_printf( "ONU-EXT:onu%d/%d/%d ext request timeout to slot %d\r\n", GetCardIdxByPonChip(PonPortIdx), 
				GetPonPortByPonChip( PonPortIdx ), OnuIdx+1, to_slotno );
		}
	}
#endif
	return rc;
}

static LONG onu_ext_dev_sync_req_handle(onu_sync_msg_head_t  *pExtMsg)
{
	short int  pon_slotno;
	short int  pon_portno;
	short int  PonPortIdx;
	short int  OnuIdx;

	if( pExtMsg == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( SYS_LOCAL_MODULE_ISMASTERACTIVE || !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		return VOS_OK;
	}
	pon_slotno = pExtMsg->ponSlotIdx;
	pon_portno = pExtMsg->portIdx;
	OnuIdx = pExtMsg->onuIdx;
	PonPortIdx = GetPonPortIdxBySlot(pon_slotno, pon_portno);

	if( (!OLT_LOCAL_ISVALID(PonPortIdx)) || (!ONU_IDX_ISVALID(OnuIdx)) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	
	ext_board_debug( ("ONU-EXT:recv onu%d/%d/%d ext sync req from master\r\n", pon_slotno, pon_portno, OnuIdx+1) );

	if( onu_ext_dev_is_ready(PonPortIdx, OnuIdx) )
	{
		/* 向主控同步 */
		onu_ext_dev_sync_2_master( PonPortIdx, OnuIdx );
	}
	else
	{
		/* 检查ONU扩展板卡信息是否需要重新读，并向主控同步 */
		onu_ext_dev_read_local( PonPortIdx, OnuIdx );
	}
	return VOS_OK;
}


static LONG onu_ext_brd_info_get( short int PonPortIdx, short int OnuIdx, ULONG brdIdx, onu_ext_brd_table_t *pExtBrdTbl )
{
	LONG rc = VOS_ERROR;
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	onu_ext_table_t *pOnuExtTbl;

	if( pExtBrdTbl == NULL )
		return rc;
	if( !ONU_BRD_IDX_IS_VALID(brdIdx) )
		return rc;

	/*如果查询标志是0，第一次查询onu信息，要给onu发oam帧并保存*/
	if( onu_ext_dev_read(PonPortIdx, OnuIdx) == VOS_ERROR )
		return rc;

	VOS_SemTake( onuExtMSemId, WAIT_FOREVER );
	pOnuExtTbl = onu_ext_dev_search( PonPortIdx, OnuIdx );
	if( pOnuExtTbl )
	{
		if( pOnuExtTbl->QueryFlag == ONU_EXT_QUERY_OK )
		{	
			VOS_MemCpy( pExtBrdTbl, &pOnuExtTbl->BrdMgmtTable[brdIdx-1], sizeof(onu_ext_brd_table_t) );
			rc = VOS_OK;
		}
	}
	VOS_SemGive( onuExtMSemId );
#endif	
	return rc;	
}

static LONG onu_ext_brd_info_set( short int PonPortIdx, short int OnuIdx, ULONG brdIdx, onu_ext_brd_table_t* pNewData )
{
	onu_ext_table_t *pOnuExtTbl = NULL;
	onu_ext_brd_table_t *pExtBrdTbl = NULL;
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	/* begin: added by jianght 20090319  */
	ULONG slot, port, onuDevIdx;
	/* end: added by jianght 20090319 */
#endif
	if( pNewData == NULL )
		return (RERROR);
	
	if( !onu_ext_dev_support(PonPortIdx, OnuIdx) )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	
	VOS_SemTake( onuExtMSemId, WAIT_FOREVER );
	pOnuExtTbl = onu_ext_dev_new( PonPortIdx, OnuIdx );
	if( pOnuExtTbl )
	{
		pExtBrdTbl = onu_ext_brd_search( PonPortIdx, OnuIdx, brdIdx );
		if( pExtBrdTbl == NULL )
		{
			VOS_SemGive( onuExtMSemId );
			return RERROR;
		}
		VOS_MemZero( pExtBrdTbl, sizeof(onu_ext_brd_table_t) );
		pExtBrdTbl->DeviceInfo.brdType = pNewData->DeviceInfo.brdType;
		/*VOS_StrnCpy( pExtBrdTbl->DeviceInfo.DeviceDesc, pNewData->DeviceInfo.DeviceDesc, MAX_BRD_DESC_LEN );*/
		VOS_StrnCpy( pExtBrdTbl->DeviceInfo.SwVersion, pNewData->DeviceInfo.SwVersion, MAX_BRD_VER_LEN );
		VOS_StrnCpy( pExtBrdTbl->DeviceInfo.HwVersion, pNewData->DeviceInfo.HwVersion, MAX_BRD_VER_LEN );
		VOS_StrnCpy( pExtBrdTbl->DeviceInfo.BootVersion, pNewData->DeviceInfo.BootVersion, MAX_BRD_VER_LEN );
		VOS_StrnCpy( pExtBrdTbl->DeviceInfo.FwVersion, pNewData->DeviceInfo.FwVersion, MAX_BRD_VER_LEN );
		VOS_StrnCpy( pExtBrdTbl->DeviceInfo.Manufacture, pNewData->DeviceInfo.Manufacture, MAX_BRD_MANU_LEN );
		VOS_StrnCpy( pExtBrdTbl->DeviceInfo.SerialNum, pNewData->DeviceInfo.SerialNum, MAX_BRD_STR_LEN );
		VOS_StrnCpy( pExtBrdTbl->DeviceInfo.ProductTime, pNewData->DeviceInfo.ProductTime, MAX_BRD_STR_LEN );
		pExtBrdTbl->DeviceInfo.OperatStatus = pNewData->DeviceInfo.OperatStatus;
		pExtBrdTbl->DeviceInfo.ChangeTime = pNewData->DeviceInfo.ChangeTime;

		if( pNewData->DeviceInfo.SupportSnmpAgent )
		{
			pExtBrdTbl->DeviceInfo.SupportSnmpAgent = pNewData->DeviceInfo.SupportSnmpAgent;
			pExtBrdTbl->DeviceInfo.AgentIp = pNewData->DeviceInfo.AgentIp;
			VOS_StrnCpy( pExtBrdTbl->DeviceInfo.ReadCommunity, pNewData->DeviceInfo.ReadCommunity, MAX_BRD_COMM_LEN );
			VOS_StrnCpy( pExtBrdTbl->DeviceInfo.WriteCommunity, pNewData->DeviceInfo.WriteCommunity, MAX_BRD_COMM_LEN );
		}
		/*pExtBrdTbl->DeviceInfo.Reserve = pNewData->DeviceInfo.Reserve;*/

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
		/* begin: added by jianght 20090319  */
		/* 加E1数据恢复处理函数 */
		slot = GetCardIdxByPonChip( PonPortIdx );
		port = GetPonPortByPonChip( PonPortIdx );
		onuDevIdx = MAKEDEVID( slot, port, (OnuIdx+1) );
		if ( VOS_OK != onuE1BoardInsert(onuDevIdx, brdIdx) )
		{
			E1_ERROR_INFO_PRINT(("onu_ext_brd_info_set:onuE1BoardInsert error!\r\n"));
		}
		/* end: added by jianght 20090319 */
#endif
	}
	VOS_SemGive( onuExtMSemId );
	return (ROK);
}

/* 返回值: VOS_ERROR，或者pdu 长度 */
static LONG onu_ext_brd_hot_pdu_parase( short int PonPortIdx, short int OnuIdx, ULONG brdIdx, UCHAR *pBrdPduData )
{
	LONG rc = VOS_ERROR;
       onu_ext_brd_table_t extBrdTbl;
       onu_ext_brd_table_t *pExtBrdTbl = &extBrdTbl;
	int i = 0, len;

	VOS_MemZero( (VOID*)pExtBrdTbl, sizeof(onu_ext_brd_table_t) );
	
	pExtBrdTbl->DeviceInfo.brdType = pBrdPduData[i++];
	pExtBrdTbl->DeviceInfo.OperatStatus = pBrdPduData[i++];
	pExtBrdTbl->DeviceInfo.ChangeTime = *(ULONG*)(&pBrdPduData[i]);
	i += sizeof(ULONG);

	len = pBrdPduData[i++];
	if( len > MAXHWVERSIONLEN )	/* modified by xieshl 20101228, 长度错误时后续字段都会出错 */
		goto pdu_err;
	if( len )
	{
		VOS_MemCpy( pExtBrdTbl->DeviceInfo.HwVersion, &pBrdPduData[i], ((len > MAX_BRD_VER_LEN) ? MAX_BRD_VER_LEN : len) );
		i += len;
	}
	
	len = pBrdPduData[i++];
	if( len )
	{
		if( len > MAXBOOTVERSIONLEN )
			goto pdu_err;
		VOS_MemCpy( pExtBrdTbl->DeviceInfo.BootVersion, &pBrdPduData[i], ((len > MAX_BRD_VER_LEN) ? MAX_BRD_VER_LEN : len) );
		i += len;
	}
	
	len = pBrdPduData[i++];
	if( len )
	{
		if( len > MAXSWVERSIONLEN )
			goto pdu_err;
		VOS_MemCpy( pExtBrdTbl->DeviceInfo.SwVersion, &pBrdPduData[i], ((len > MAX_BRD_VER_LEN) ? MAX_BRD_VER_LEN : len) );
		i += len;
	}
	
	len = pBrdPduData[i++];
	if( len )
	{
		if( len > MAXFWVERSIONLEN )
			goto pdu_err;
		VOS_MemCpy( pExtBrdTbl->DeviceInfo.FwVersion, &pBrdPduData[i], ((len > MAX_BRD_VER_LEN) ? MAX_BRD_VER_LEN : len) );
		i += len;
	}

	len = pBrdPduData[i++];
	if( len )
	{
		if( len > MAXVENDORINFOLEN )
			goto pdu_err;
		VOS_MemCpy( pExtBrdTbl->DeviceInfo.Manufacture, &pBrdPduData[i], ((len > MAX_BRD_MANU_LEN) ? MAX_BRD_MANU_LEN : len) );
		i += len;
	}

	len = pBrdPduData[i++];
	if( len )
	{
		if( len > MAXSNLEN )
			goto pdu_err;
		VOS_MemCpy( pExtBrdTbl->DeviceInfo.SerialNum, &pBrdPduData[i], ((len > MAX_BRD_STR_LEN) ? MAX_BRD_STR_LEN : len) );
		i += len;
	}
	
	len = pBrdPduData[i++];
	if( len != 0 )
	{
		if( len > MAXDATELEN )
			goto pdu_err;
		VOS_MemCpy( pExtBrdTbl->DeviceInfo.ProductTime, &pBrdPduData[i], ((len > MAX_BRD_STR_LEN) ? MAX_BRD_STR_LEN : len) );
		i += len;
	}

	if( onu_ext_brd_info_set(PonPortIdx, OnuIdx, brdIdx, pExtBrdTbl) == VOS_OK )
		rc = i;
pdu_err:

	ext_board_debug( ("ONU-EXT:parase onu %d/%d/%d ext-mgt hot pdu %s\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip( PonPortIdx ), OnuIdx+1,
		(rc == VOS_ERROR ? "ERR" : "OK")) );
	ext_board_debug( ( "Total length=%d\r\n", i) );

	return rc;
}

/* 返回值: VOS_ERROR，或者pdu 长度 */
static LONG onu_ext_dev_mgt_pdu_parase( short int PonPortIdx, short int OnuIdx, onu_ext_pdu_t *pPdu )
{
	LONG rc = VOS_ERROR;
	int i, j=0;
	ULONG onuBrdIdx, len=0;

	onu_ext_brd_table_t extBrdTbl;
	onu_ext_brd_table_t *pExtBrdTbl = &extBrdTbl;

	if( (pPdu->MsgType != GET_EXT_ONU_SYS_INFO_RSP) || (pPdu->Result != GET_ONU_EXT_SYS_INFO_SUCCESS) ||
		!ONU_BRD_IDX_IS_VALID(pPdu->CurBrdNum) )
	{
		sys_console_printf( "ONU-EXT:parase PDU MsgType=%d,result=%d,BrdNum=%d ERR\r\n", pPdu->MsgType, pPdu->Result, pPdu->CurBrdNum );
		return rc;
	}
	
	for( i=1; i<=pPdu->CurBrdNum; i++ )
	{
		VOS_MemZero( pExtBrdTbl, sizeof(onu_ext_brd_table_t) );

		onuBrdIdx = pPdu->ExtData[j++];
		ext_board_debug_printf( ("slotnum=%d\r\n", onuBrdIdx) );

		pExtBrdTbl->DeviceInfo.brdType = pPdu->ExtData[j++];
		ext_board_debug_printf( ("brdtype=%d\r\n", pExtBrdTbl->DeviceInfo.brdType) );

		pExtBrdTbl->DeviceInfo.OperatStatus = pPdu->ExtData[j++];
		ext_board_debug_printf( ("operateStatus=%d\r\n", pExtBrdTbl->DeviceInfo.OperatStatus) );

		pExtBrdTbl->DeviceInfo.ChangeTime = *(ULONG*)(&pPdu->ExtData[j]);
		j += sizeof(ULONG);
		ext_board_debug_printf( ("change time=%d\r\n", pExtBrdTbl->DeviceInfo.ChangeTime) );

		len = pPdu->ExtData[j++];
		if( len )
		{
			if( len > MAXHWVERSIONLEN )
				goto pdu_err;
			VOS_MemCpy( pExtBrdTbl->DeviceInfo.HwVersion, &(pPdu->ExtData[j]), ((len > MAX_BRD_VER_LEN) ? MAX_BRD_VER_LEN : len) );
			j += len;
			ext_board_debug_printf( ("HwVersion=%s\r\n", pExtBrdTbl->DeviceInfo.HwVersion) );
		}
		
		len = pPdu->ExtData[j++];
		if( len )
		{
			if( len > MAXBOOTVERSIONLEN )
				goto pdu_err;
			VOS_MemCpy( pExtBrdTbl->DeviceInfo.BootVersion, &( pPdu->ExtData[j]), ((len > MAX_BRD_VER_LEN) ? MAX_BRD_VER_LEN : len) );
			j += len;
			ext_board_debug_printf( ("bootVersion=%s\r\n", pExtBrdTbl->DeviceInfo.BootVersion) );
		}

		len = pPdu->ExtData[j++];
		if( len )
		{
			if( len > MAXSWVERSIONLEN )
				goto pdu_err;
			VOS_MemCpy( pExtBrdTbl->DeviceInfo.SwVersion, &(pPdu->ExtData[j]), ((len > MAX_BRD_VER_LEN) ? MAX_BRD_VER_LEN : len) );
			j += len;
			ext_board_debug_printf( ("SwVersion=%s\r\n", pExtBrdTbl->DeviceInfo.SwVersion) );
		}

		len = pPdu->ExtData[j++];
		if( len )
		{
			if( len > MAXHWVERSIONLEN )
				goto pdu_err;
			VOS_MemCpy( pExtBrdTbl->DeviceInfo.FwVersion, &(pPdu->ExtData[j]), ((len > MAX_BRD_VER_LEN) ? MAX_BRD_VER_LEN : len) );
			j += len;
			ext_board_debug_printf( ("FwVersion=%s\r\n", pExtBrdTbl->DeviceInfo.FwVersion) );
		}

		len = pPdu->ExtData[j++];
		if( len )
		{
			if( len > MAXVENDORINFOLEN )
				goto pdu_err;
			VOS_MemCpy( pExtBrdTbl->DeviceInfo.Manufacture, &(pPdu->ExtData[j]), ((len > MAX_BRD_MANU_LEN) ? MAX_BRD_MANU_LEN : len) );
			j += len;
			ext_board_debug_printf( ("Manufacture=%s\r\n", pExtBrdTbl->DeviceInfo.Manufacture) );
		}

		len = pPdu->ExtData[j++];
		if( len )
		{
			if( len > MAXSNLEN )
				goto pdu_err;
			VOS_MemCpy( pExtBrdTbl->DeviceInfo.SerialNum, &(pPdu->ExtData[j]), ((len > MAX_BRD_STR_LEN) ? MAX_BRD_STR_LEN : len) );
			j += len;
			ext_board_debug_printf( ("SerialNum=%s\r\n", pExtBrdTbl->DeviceInfo.SerialNum) );
		}
		
		len = pPdu->ExtData[j++];
		if( len )
		{
			if( len > MAXDATELEN )
				goto pdu_err;
			VOS_MemCpy( pExtBrdTbl->DeviceInfo.ProductTime, &(pPdu->ExtData[j]), ((len > MAX_BRD_STR_LEN) ? MAX_BRD_STR_LEN : len) );
			j += len;
			ext_board_debug_printf( ("ProductTime=%s\r\n", pExtBrdTbl->DeviceInfo.ProductTime) );
		}

		pExtBrdTbl->DeviceInfo.SupportSnmpAgent = pPdu->ExtData[j++];
		ext_board_debug_printf( ("SupportSnmpAgent=%d\r\n", pExtBrdTbl->DeviceInfo.SupportSnmpAgent) );

		if( pExtBrdTbl->DeviceInfo.SupportSnmpAgent )
		{
			pExtBrdTbl->DeviceInfo.AgentIp = *(ULONG *)(&pPdu->ExtData[j]);
			j += sizeof(ULONG);

			len=pPdu->ExtData[j++];
			if( len )
			{
				if( len > MAXREADCOMMLEN )
					goto pdu_err;
				VOS_MemCpy( pExtBrdTbl->DeviceInfo.ReadCommunity, &(pPdu->ExtData[j]), ((len > MAX_BRD_COMM_LEN) ? MAX_BRD_COMM_LEN : len) );
				ext_board_debug_printf( ("ReadCommunity=%s\r\n", pExtBrdTbl->DeviceInfo.ReadCommunity) );
				j += len;
			}

			len = pPdu->ExtData[j++];
			if( len )
			{
				if( len > MAXWRITECOMMLEN )
					goto pdu_err;
				VOS_MemCpy( pExtBrdTbl->DeviceInfo.WriteCommunity, &(pPdu->ExtData[j]), ((len > MAX_BRD_COMM_LEN) ? MAX_BRD_COMM_LEN : len) );
				ext_board_debug_printf( ("WriteCommunity=%s\r\n", pExtBrdTbl->DeviceInfo.WriteCommunity) );
				j += len;
			}
			j++;
		}
		/*pExtBrdTbl->DeviceInfo.Reserve = pPdu->ExtData[j];*/

		if( onu_ext_brd_info_set(PonPortIdx, OnuIdx, onuBrdIdx, pExtBrdTbl) != VOS_OK )
			goto pdu_err;
	}

	if( onu_ext_dev_query_over(PonPortIdx, OnuIdx) == VOS_OK )
		rc = j;

pdu_err:

	ext_board_debug( ("ONU-EXT:parase onu %d/%d/%d ext-dev-mgt pdu %s\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip( PonPortIdx ), OnuIdx+1,
		(rc == VOS_ERROR ? "ERR" : "OK")) );
	ext_board_debug( ( "Total length=%d\r\n\r\n", j) );

/*#ifdef EXT_MGT_DEBUG
	pktDataPrintf( pPdu->ExtData, j );
#endif*/

	return rc;
}

static LONG onu_ext_dev_mgt_pdu_create( short int PonPortIdx, short int OnuIdx, onu_ext_pdu_t *pPdu, ULONG *pPduLen )
{
	LONG rc = VOS_ERROR;
	onu_ext_table_t *pOnuExtTbl;
	onu_ext_brd_table_t *pExtBrdTbl;
	ULONG len;
	int i, j = 0;
	UCHAR *pPduData;

	if( (pPdu == NULL) || (pPduLen == NULL) )
		return rc;
	
	ext_board_debug( ("\r\nONU-EXT:create onu %d/%d/%d ext-mgmt pdu\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip( PonPortIdx ), OnuIdx+1) );

	VOS_SemTake( onuExtMSemId, WAIT_FOREVER );

	pOnuExtTbl = onu_ext_dev_search(PonPortIdx, OnuIdx );
	if( pOnuExtTbl == NULL)
	{
		VOS_SemGive( onuExtMSemId );
		VOS_ASSERT(0);
		return rc;
	}

	VOS_MemZero( pPdu, sizeof(onu_ext_pdu_t) );
	pPdu->MsgType = GET_EXT_ONU_SYS_INFO_RSP;
	pPdu->Result = GET_ONU_EXT_SYS_INFO_SUCCESS;
	/*pPdu->CurBrdNum = MAX_ONU_BRD_NUM;
	pPdu->QueryFlag = pOnuExtTbl->QueryFlag;*/

	pPduData = pPdu->ExtData;
	
	rc = VOS_OK;
	for( i=1; i<=MAX_ONU_BRD_NUM; i++ )
	{
		pExtBrdTbl = onu_ext_brd_search( PonPortIdx, OnuIdx, i );
		if( pExtBrdTbl == NULL )
			continue;
		if( !ONU_BRD_TYPE_IS_VALID(pExtBrdTbl->DeviceInfo.brdType) )
			continue;
		
		pPdu->CurBrdNum++;
		pPduData[j++] = i;
		ext_board_debug_printf( ("slotnum=%d\r\n", i) );

		pPduData[j++] = pExtBrdTbl->DeviceInfo.brdType;
		ext_board_debug_printf( ("brdtype=%d\r\n", pExtBrdTbl->DeviceInfo.brdType) );

		pPduData[j++] = pExtBrdTbl->DeviceInfo.OperatStatus;
		ext_board_debug_printf( ("operateStatus=%d\r\n", pExtBrdTbl->DeviceInfo.OperatStatus) );

		*(ULONG *)(&pPduData[j]) = pExtBrdTbl->DeviceInfo.ChangeTime;
		j += sizeof(ULONG);
		ext_board_debug_printf( ("change time=%d\r\n", pExtBrdTbl->DeviceInfo.ChangeTime) );

		len = VOS_StrLen( pExtBrdTbl->DeviceInfo.HwVersion );
		pPduData[j++] = len;
		VOS_MemCpy( &pPduData[j], &(pExtBrdTbl->DeviceInfo.HwVersion), len );
		j += len;
		ext_board_debug_printf( ("HwVersion=%s,len=%d\r\n", pExtBrdTbl->DeviceInfo.HwVersion, len) );

		len = VOS_StrLen( pExtBrdTbl->DeviceInfo.BootVersion );
		pPduData[j++] = len;
		VOS_MemCpy( &pPduData[j], pExtBrdTbl->DeviceInfo.BootVersion, len );
		j += len;
		ext_board_debug_printf( ("bootVersion=%s,len=%d\r\n", pExtBrdTbl->DeviceInfo.BootVersion, len) );

		len = VOS_StrLen( pExtBrdTbl->DeviceInfo.SwVersion );
		pPduData[j++] = len;
		VOS_MemCpy( &pPduData[j], pExtBrdTbl->DeviceInfo.SwVersion, len );
		j += len;
		ext_board_debug_printf( ("SwVersion=%s,len=%d\r\n", pExtBrdTbl->DeviceInfo.SwVersion, len) );

		len = VOS_StrLen( pExtBrdTbl->DeviceInfo.FwVersion );
		pPduData[j++] = len;
		VOS_MemCpy( &pPduData[j], pExtBrdTbl->DeviceInfo.FwVersion, len );
		j += len;
		ext_board_debug_printf( ("FwVersion=%s,len=%d\r\n", pExtBrdTbl->DeviceInfo.FwVersion, len) );

		len = VOS_StrLen( pExtBrdTbl->DeviceInfo.Manufacture );
		pPduData[j++] = len;
		VOS_MemCpy( &pPduData[j], pExtBrdTbl->DeviceInfo.Manufacture, len );
		j += len;
		ext_board_debug_printf( ("Manufacture=%s,len=%d\r\n", pExtBrdTbl->DeviceInfo.Manufacture, len) );

		len = VOS_StrLen( pExtBrdTbl->DeviceInfo.SerialNum );
		pPduData[j++] = len;
		VOS_MemCpy( &pPduData[j], pExtBrdTbl->DeviceInfo.SerialNum, len );
		j += len;
		ext_board_debug_printf( ("SerialNum=%s,len=%d\r\n", pExtBrdTbl->DeviceInfo.SerialNum, len) );
		
		len = VOS_StrLen( pExtBrdTbl->DeviceInfo.ProductTime );
		pPduData[j++] = len;
		VOS_MemCpy( &pPduData[j], pExtBrdTbl->DeviceInfo.ProductTime, len );
		j += len;
		ext_board_debug_printf( ("ProductTime=%s,len=%d\r\n", pExtBrdTbl->DeviceInfo.ProductTime, len) );

		pPduData[j++] = pExtBrdTbl->DeviceInfo.SupportSnmpAgent;
		ext_board_debug_printf( ("SupportSnmpAgent=%d\r\n", pExtBrdTbl->DeviceInfo.SupportSnmpAgent) );

		if( pExtBrdTbl->DeviceInfo.SupportSnmpAgent )
		{
			*(ULONG *)(&pPduData[j]) = pExtBrdTbl->DeviceInfo.AgentIp;
			ext_board_debug_printf( ("AgentIp=0x%x\r\n", pExtBrdTbl->DeviceInfo.AgentIp) );
			j += sizeof(ULONG);

			len = VOS_StrLen( pExtBrdTbl->DeviceInfo.ReadCommunity );
			pPduData[j++] = len;
			if( len )
			{
				VOS_MemCpy( &pPduData[j], pExtBrdTbl->DeviceInfo.ReadCommunity, len );
				j += len;
				ext_board_debug_printf( ("ReadCommunity=%s,len=%d\r\n", pExtBrdTbl->DeviceInfo.ReadCommunity, len) );
			}

			len = VOS_StrLen( pExtBrdTbl->DeviceInfo.WriteCommunity );
			pPduData[j++] = len;
			if( len )
			{
				VOS_MemCpy( &pPduData[j], pExtBrdTbl->DeviceInfo.WriteCommunity, len );
				j += len;
				ext_board_debug_printf( ("WriteCommunity=%s,len=%d\r\n", pExtBrdTbl->DeviceInfo.WriteCommunity, len) );
			}
			pPduData[j++] = 0;/*pExtBrdTbl->DeviceInfo.Reserve;*/
		}
	}

	VOS_SemGive( onuExtMSemId );
	*pPduLen = j;

	ext_board_debug( (" Total length=%d\r\n", j) );

/*#ifdef EXT_MGT_DEBUG
	pktDataPrintf( pPduData, j );
#endif*/

	return rc;
}

static char * onu_ext_brd_type2name( int brdType )
{	
	char * typename = NULL;
	switch( brdType )
	{
		case OAM_ONU_SLOT_NULL:
			typename = ONU_SUB_BOARD_NULL;
			break;
		case  OAM_ONU_SLOT_GT_EPON_B:
			typename = ONU_SUB_BOARD_EPON;
			break;
		case OAM_ONU_SLOT_GT_8POTS_A:
			typename = ONU_SUB_BOARD_8POTS_A;
			break;
		case OAM_ONU_SLOT_GT_8POTSO_A:
			typename = ONU_SUB_BOARD_8POTSO_A;
			break;
		case OAM_ONU_SLOT_GT_8POTS_B:
			typename = ONU_SUB_BOARD_8POTS_B;
			break;
		case OAM_ONU_SLOT_GT_8POTSO_B:
			typename = ONU_SUB_BOARD_8POTSO_B;
			break;
		case OAM_ONU_SLOT_GT_6FE:
			typename = ONU_SUB_BOARD_6FE;
			break;
		case OAM_ONU_SLOT_GT_8FE:
			typename = ONU_SUB_BOARD_8FE;
			break;
		case OAM_ONU_SLOT_GT_16FE:
			typename = ONU_SUB_BOARD_16FE;
			break;
		case OAM_ONU_SLOT_GT_8FXS_A:
			typename = ONU_SUB_BOARD_8FXS_A;
			break;
		case OAM_ONU_SLOT_GT_8FXS_B:
			typename = ONU_SUB_BOARD_8FXS_B;
			break;
		/* begin: added by jianght 20090410  */
		case OAM_ONU_SLOT_GT_4E1_120ohm:
			typename = ONU_SUB_BOARD_4E1_120;
			break;
		case OAM_ONU_SLOT_GT_4E1_75ohm:
			typename = ONU_SUB_BOARD_4E1_75;
			break;
		/* end: added by jianght 20090410 */
		default:
			typename = "";	/*ONU_SUB_BOARD_LAST;*/
			break;
	}
	return typename;
}

static char * onu_ext_brd_oper_state2str( ULONG state )
{
	char *pStr = "Error";
	switch( state )
	{
		case 0:
			pStr = "NULL";
			break;
		case 1:
			pStr = "Init";
			break;
		case 2:
			pStr = "Updating";
			break;
		case 3:
			pStr = "Running";
			break;
		case 4:
		default:
			break;
	}
	return pStr;
}

static ULONG onu_ext_brd_mib_type_get( int oamData )
{
	ULONG type = mib_brd_type_null;
	switch(oamData)
	{	
		/*case OAM_ONU_SLOT_NULL:
			type = mib_brd_type_null;
			break;*/
		case  OAM_ONU_SLOT_GT_EPON_B:
			type =  mib_brd_type_onuEponB;
			break;
		case OAM_ONU_SLOT_GT_8POTS_A:
		case OAM_ONU_SLOT_GT_8POTSO_A:
			type = mib_brd_type_onu8PotsA;
			break;
		case OAM_ONU_SLOT_GT_8POTS_B:
		case OAM_ONU_SLOT_GT_8POTSO_B:
			type = mib_brd_type_onu8PotsB;
			break;
		case OAM_ONU_SLOT_GT_6FE:
			type = mib_brd_type_onu6FeC;
			break;
		case OAM_ONU_SLOT_GT_8FE:
			type = mib_brd_type_onu8FeD;
			break;
		case OAM_ONU_SLOT_GT_16FE:
			type = mib_brd_type_onu16FeD;
			break;
		case OAM_ONU_SLOT_GT_8FXS_A:
			type = mib_brd_type_onu8FxsA;
			break;
		case OAM_ONU_SLOT_GT_8FXS_B:
			type = mib_brd_type_onu8FxsB;
			break;
		case OAM_ONU_SLOT_GT_4E1_120ohm:/*add by zhengy 09-04-22*/
			type=mib_brd_type_onu4PotsA;
			break;
		case OAM_ONU_SLOT_GT_4E1_75ohm:
			type=mib_brd_type_onu4PotsB;
			break;
		default:
			/*type = mib_brd_type_unknown;*/
			break;
	}
	return type;
}

/* 根据MIB定义，返回值: 0-not support, 1-snmp v1, 2-snmp v2c, 3-snmp v3 */
static ULONG onu_ext_brd_type_support_snmp( ULONG brdType )
{
	ULONG rc = 0;
	switch(brdType)
	{	
		case OAM_ONU_SLOT_GT_8POTS_A:
		case OAM_ONU_SLOT_GT_8POTSO_A:
		case OAM_ONU_SLOT_GT_8POTS_B:
		case OAM_ONU_SLOT_GT_8POTSO_B:
			rc = 2;
			break;
		default:
			break;
	}
	return rc;
}
#endif


static LONG onu_ext_dev_dispatch( ULONG devIdx, ULONG brdIdx, onu_ext_code_t code, VOID *pData )
{
	LONG rc = VOS_ERROR;
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	int slot, port;
	short int PonPortIdx, OnuIdx;
	/*onu_ext_table_t *pOnuExtTbl = NULL;*/
	onu_ext_brd_table_t *pExtBrdTbl = NULL;
	int i, j;

	slot = GET_PONSLOT( devIdx );
	port = GET_PONPORT( devIdx );
	OnuIdx = GET_ONUID( devIdx ) - 1;
	PonPortIdx=GetPonPortIdxBySlot(slot, port);

	switch( code )
	{
		case ONU_EXT_DEV_ONU_REG:
			if( onu_ext_dev_support(PonPortIdx, OnuIdx) )
			{
				/*rc = onu_ext_dev_read_local( PonPortIdx, OnuIdx );*/
				rc = onu_ext_dev_query_declare( PonPortIdx, OnuIdx );
			}
			else
			{
				rc = onu_ext_dev_delete( PonPortIdx, OnuIdx );
			}
			break;
			
		case ONU_EXT_DEV_ONU_DEREG:
			rc = onu_ext_dev_delete( PonPortIdx, OnuIdx );
			break;
			
		case ONU_EXT_DEV_BRD_NUM_GET:
			if( onu_ext_dev_read(PonPortIdx, OnuIdx) == VOS_OK )
			{
				VOS_SemTake( onuExtMSemId, WAIT_FOREVER );
				for( i=1, j=0; i<=MAX_ONU_BRD_NUM; i++ )
				{
					pExtBrdTbl = onu_ext_brd_search( PonPortIdx, OnuIdx, i );
					if( (pExtBrdTbl) && (pExtBrdTbl->DeviceInfo.brdType != 0) )
					{
						j++;
					}
				}
				VOS_SemGive( onuExtMSemId );
			}
			if( j == 0 )
				j = 1;
			
			if( pData )
			{
				*((ULONG *)pData) = j;
				rc = VOS_OK;
			}
			break;

		case ONU_EXT_DEV_BRD_INSERT:
			rc = onu_ext_brd_hot_pdu_parase( PonPortIdx, OnuIdx, brdIdx, (UCHAR *)pData );
			break;
			
		case ONU_EXT_DEV_BRD_PULL:
			if( brdIdx == 1 )	/* onu master slot is pulled */
			{
				rc = onu_ext_dev_delete( PonPortIdx, OnuIdx );
			}
			else if( (brdIdx > 1) && (brdIdx <= MAX_ONU_BRD_NUM) )
			{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
				onuE1BoardPull( devIdx, brdIdx );		/* E1板卡拔出处理 */
#endif
				VOS_SemTake( onuExtMSemId, WAIT_FOREVER );
				pExtBrdTbl = onu_ext_brd_search( PonPortIdx, OnuIdx, brdIdx );
				if( pExtBrdTbl )
				{
					pExtBrdTbl->DeviceInfo.brdType = 0;
					pExtBrdTbl->DeviceInfo.OperatStatus = 1;
					/*change time 项也需要写入*/
					rc = VOS_OK;
				}
				VOS_SemGive( onuExtMSemId );
			}
			break;

		case ONU_EXT_DEV_BRD_LIST_SET:
			if( pData )
			{
				UCHAR *pBrdList = (UCHAR *)pData;
				if( ONU_BRD_IDX_IS_VALID(pBrdList[0]) )
				{
					for( i=1; i<=pBrdList[0]; i++ )
					{
						VOS_SemTake( onuExtMSemId, WAIT_FOREVER );
						pExtBrdTbl = onu_ext_brd_search( PonPortIdx, OnuIdx, i );
						if( pExtBrdTbl )  pExtBrdTbl->DeviceInfo.brdType = pBrdList[i];
						VOS_SemGive( onuExtMSemId );
					}
				}
				rc = VOS_OK;
			}
			break;
			
		default:
			break;
	}
#endif
	return rc;
}
	
static LONG onu_ext_brd_dispatch( ULONG devIdx, ULONG brdIdx, onu_ext_code_t code, VOID *pData )
{
	LONG rc = VOS_ERROR;
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	int slot, port;
	short int PonPortIdx, OnuIdx;
	/*onu_ext_table_t *pOnuExtTbl = NULL;*/
	onu_ext_brd_table_t *pExtBrdTbl = NULL;

	if( pData == NULL )
		return rc;

	slot = GET_PONSLOT( devIdx );
	port = GET_PONPORT( devIdx );
	OnuIdx = GET_ONUID( devIdx ) - 1;
	PonPortIdx=GetPonPortIdxBySlot(slot, port);

	if( !onu_ext_dev_support(PonPortIdx, OnuIdx) )
		return rc;

	if( !onu_ext_dev_is_ready(PonPortIdx, OnuIdx) )
	{
		/*如果查询标志是0，第一次查询onu信息，要给onu发oam帧并保存*/
		onu_ext_dev_read( PonPortIdx, OnuIdx );
	}

	VOS_SemTake( onuExtMSemId, WAIT_FOREVER );
	
	pExtBrdTbl = onu_ext_brd_search( PonPortIdx, OnuIdx, brdIdx );
	if( pExtBrdTbl == NULL )
	{
		VOS_SemGive( onuExtMSemId );
		return rc;
	}

	rc = VOS_OK;

	switch( code )
	{
		case ONU_EXT_BRD_MIB_TYPE_GET:
			*((ULONG *)pData) = onu_ext_brd_mib_type_get( pExtBrdTbl->DeviceInfo.brdType );
			break;
		case ONU_EXT_BRD_SUPPORT_MIB_TYPE_GET:
			if( brdIdx == 1 )
				*((ULONG *)pData) = mib_brd_support_bit_onuEponB;
			else
				*((ULONG *)pData) = ( mib_brd_support_bit_onu6FeC | mib_brd_support_bit_onu8FeD | mib_brd_support_bit_onu16FeD |
						mib_brd_support_bit_onu8PotsA | mib_brd_support_bit_onu8PotsB	|
						mib_brd_support_bit_onu8FxsA | mib_brd_support_bit_onu8FxsB );
			break;
		case ONU_EXT_BRD_OPER_STATUS_GET:
			*((ULONG *)pData) = pExtBrdTbl->DeviceInfo.OperatStatus;
			break;
		case ONU_EXT_BRD_CHG_TIME_GET:
			*((ULONG *)pData) = pExtBrdTbl->DeviceInfo.ChangeTime;
			break;
		case ONU_EXT_BRD_DESC_GET:
			/*if( pExtBrdTbl->DeviceInfo.DeviceDesc[0] != 0 )
				VOS_StrCpy( pData, pExtBrdTbl->DeviceInfo.DeviceDesc );
			else*/
				VOS_StrCpy( pData, onu_ext_brd_type2name(pExtBrdTbl->DeviceInfo.brdType) );
			break;
		case ONU_EXT_BRD_DESC_SET:
			/*VOS_MemZero( pExtBrdTbl->DeviceInfo.DeviceDesc, sizeof(pExtBrdTbl->DeviceInfo.DeviceDesc) );
			VOS_StrnCpy( pExtBrdTbl->DeviceInfo.DeviceDesc, pData, MAXDEVICEDESCLEN );*/
			break;
		case ONU_EXT_BRD_SW_VER_GET:
			VOS_StrCpy( pData, pExtBrdTbl->DeviceInfo.SwVersion );
			break;
		case ONU_EXT_BRD_FW_VER_GET:
			VOS_StrCpy( pData, pExtBrdTbl->DeviceInfo.FwVersion );
			break;
		case ONU_EXT_BRD_HW_VER_GET:
			VOS_StrCpy( pData, pExtBrdTbl->DeviceInfo.HwVersion );
			break;
		case ONU_EXT_BRD_BOOT_VER_GET:
			VOS_StrCpy( pData, pExtBrdTbl->DeviceInfo.BootVersion );
			break;
		case ONU_EXT_BRD_MANUFACTURE_GET:
			VOS_StrCpy( pData, pExtBrdTbl->DeviceInfo.Manufacture );
			break;
		case ONU_EXT_BRD_SN_GET:
			VOS_StrCpy( pData, pExtBrdTbl->DeviceInfo.SerialNum );
			break;
		case ONU_EXT_BRD_PRODUCT_TIME_GET:
			VOS_StrCpy( pData, pExtBrdTbl->DeviceInfo.ProductTime );
			break;
		case ONU_EXT_BRD_HAS_SNMP_GET:
			*((ULONG *)pData) = onu_ext_brd_type_support_snmp(pExtBrdTbl->DeviceInfo.brdType);
			break;
		case ONU_EXT_BRD_SNMP_IP_GET:
			if( onu_ext_brd_type_support_snmp(pExtBrdTbl->DeviceInfo.brdType) != 0 )
				*((ULONG *)pData) = pExtBrdTbl->DeviceInfo.AgentIp;
			else
				*((ULONG *)pData) = 0;
			break;
		case ONU_EXT_BRD_SNMP_IP_SET:
			if( onu_ext_brd_type_support_snmp(pExtBrdTbl->DeviceInfo.brdType) != 0 )
				pExtBrdTbl->DeviceInfo.AgentIp = *((ULONG *)pData);
			break;
		case ONU_EXT_BRD_RD_COMMUNITY_GET:
			if( onu_ext_brd_type_support_snmp(pExtBrdTbl->DeviceInfo.brdType) != 0 )
				VOS_StrCpy( pData, pExtBrdTbl->DeviceInfo.ReadCommunity );
			else
				VOS_StrCpy( pData, " " );
			break;
		case ONU_EXT_BRD_RD_COMMUNITY_SET:
			if( onu_ext_brd_type_support_snmp(pExtBrdTbl->DeviceInfo.brdType) != 0 )
			{
				VOS_MemZero( pExtBrdTbl->DeviceInfo.ReadCommunity, sizeof(pExtBrdTbl->DeviceInfo.ReadCommunity) );
				VOS_StrnCpy( pExtBrdTbl->DeviceInfo.ReadCommunity, pData, MAX_BRD_COMM_LEN );
			}
			break;
		case ONU_EXT_BRD_WR_COMMUNITY_GET:
			if( onu_ext_brd_type_support_snmp(pExtBrdTbl->DeviceInfo.brdType) != 0 )
				VOS_StrCpy( pData, pExtBrdTbl->DeviceInfo.WriteCommunity );
			else
				VOS_StrCpy( pData, " " );
			break;
		case ONU_EXT_BRD_WR_COMMUNITY_SET:
			if( onu_ext_brd_type_support_snmp(pExtBrdTbl->DeviceInfo.brdType) != 0 )
			{
				VOS_MemZero( pExtBrdTbl->DeviceInfo.WriteCommunity, sizeof(pExtBrdTbl->DeviceInfo.WriteCommunity) );
				VOS_StrnCpy( pExtBrdTbl->DeviceInfo.WriteCommunity, pData, MAX_BRD_COMM_LEN );
			}
			break;
		default:
			rc = VOS_ERROR;
			break;
	}

	VOS_SemGive( onuExtMSemId );
#endif
	return rc;
}


LONG InitOnuExtMgmtTable( )
{
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	short int PonPortIdx;
    
	if( onuExtMSemId == 0 )
		onuExtMSemId = VOS_SemMCreate( VOS_SEM_Q_FIFO );

	onuExtBSemId = VOS_SemBCreate( VOS_SEM_Q_FIFO, VOS_SEM_EMPTY );
   
	for( PonPortIdx=0; PonPortIdx<MAX_PON_CHIP; PonPortIdx++ )
	{
		gpOnuExtMgmtTable[PonPortIdx] = NULL;
	}

	reigster_onuevent_callback(ONU_EVENT_CODE_REGISTER, (g_OnuEventFuction_callback)onuExtMgmt_OnuRegCallback);
	reigster_onuevent_callback(ONU_EVENT_CODE_DEREGISTER, (g_OnuEventFuction_callback)onuExtMgmt_OnuDeregCallback);
	reigster_onuevent_callback(ONU_EVENT_CODE_REMOVE, (g_OnuEventFuction_callback)onuExtMgmt_OnuDeregCallback);

	ExtBrd_CommandInstall();
#endif

	return VOS_OK;						
}


LONG getOnuExtMgmtBrdType( ULONG devIdx, ULONG brdIdx, ULONG *pulType )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_MIB_TYPE_GET, pulType );
}

LONG getOnuExtMgmtBrdSupportType( ULONG devIdx, ULONG brdIdx, ULONG *pulType )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_SUPPORT_MIB_TYPE_GET, pulType );
}

LONG getOnuExtMgmtBrdDeviceDesc( ULONG devIdx, ULONG brdIdx, char *pDesc )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_DESC_GET, pDesc );
}
LONG setOnuExtMgmtBrdDeviceDesc( ULONG devIdx, ULONG brdIdx, char *pDesc )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_DESC_SET, pDesc );
}

LONG getOnuExtMgmtBrdOperStatus( ULONG devIdx, ULONG brdIdx, ULONG *pulStatus )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_OPER_STATUS_GET, pulStatus );
}

LONG getOnuExtMgmtBrdChangeTime( ULONG devIdx, ULONG brdIdx, ULONG *pulTime)
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_CHG_TIME_GET, pulTime );
}

LONG getOnuExtMgmtBrdSwVer( ULONG devIdx, ULONG brdIdx, char *pSwVer)
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_SW_VER_GET, pSwVer );
}

LONG getOnuExtMgmtBrdFwVer( ULONG devIdx, ULONG brdIdx, char *pFwVer )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_FW_VER_GET, pFwVer );
}

LONG getOnuExtMgmtBrdHwVer( ULONG devIdx, ULONG brdIdx, char *pHwVer )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_HW_VER_GET, pHwVer );
}

LONG getOnuExtMgmtBrdBootVer( ULONG devIdx, ULONG brdIdx, char *pBootVer )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_BOOT_VER_GET, pBootVer );
}

LONG getOnuExtMgmtBrdManufacture( ULONG devIdx, ULONG brdIdx, char *pManufacture )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_MANUFACTURE_GET, pManufacture );
}

LONG getOnuExtMgmtBrdSerialNum( ULONG devIdx, ULONG brdIdx, char *pSerialnum )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_SN_GET, pSerialnum );
}

LONG getOnuExtMgmtBrdProductTime( ULONG devIdx, ULONG brdIdx, char *pProductTime )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_PRODUCT_TIME_GET, pProductTime );
}

/*LONG getOnuExtMgmtBrdSupportSnmpAgent( short int PonPortIdx, short int OnuIdx, ULONG brdIdx, char *ulSupportSnmpAgent )
{
	onu_ext_brd_table_t brdInfo;

	if( onu_ext_brd_info_get(PonPortIdx, OnuIdx, brdIdx, &brdInfo) == VOS_OK )
	{
		*ulSupportSnmpAgent=brdInfo.DeviceInfo.SupportSnmpAgent;
		return (ROK);
	}
	return RERROR;
}*/

LONG getOnuExtMgmtBrdHasSnmpAgent( ULONG devIdx, ULONG brdIdx, ULONG *pHasAgent )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_HAS_SNMP_GET, pHasAgent );
}

LONG getOnuExtMgmtBrdAgentIp( ULONG devIdx, ULONG brdIdx, ULONG *pulAgentIp )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_SNMP_IP_GET, pulAgentIp );
}

LONG setOnuExtMgmtBrdAgentIp( ULONG devIdx, ULONG brdIdx, ULONG *pulAgentIp )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_SNMP_IP_SET, pulAgentIp );
}

LONG getOnuExtMgmtBrdReadCommnity( ULONG devIdx, ULONG brdIdx, char *pReadComm )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_RD_COMMUNITY_GET, pReadComm );
}

LONG setOnuExtMgmtBrdReadCommnity( ULONG devIdx, ULONG brdIdx, char *pReadComm )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_RD_COMMUNITY_SET, pReadComm );
}

LONG getOnuExtMgmtBrdWriteCommnity( ULONG devIdx, ULONG brdIdx, char *pWriteComm )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_WR_COMMUNITY_GET, pWriteComm );
}

LONG setOnuExtMgmtBrdWriteCommnity( ULONG devIdx, ULONG brdIdx, char *pWriteComm )
{
	return onu_ext_brd_dispatch( devIdx, brdIdx, ONU_EXT_BRD_WR_COMMUNITY_SET, pWriteComm );
}

LONG getOnuExtMgmtBrdNum( ULONG devIdx, ULONG *pNum )
{
	if( pNum == NULL )
		return VOS_ERROR;
	if( onu_ext_dev_dispatch(devIdx, 0, ONU_EXT_DEV_BRD_NUM_GET, pNum) == VOS_ERROR )
		*pNum = 1;
		
	return VOS_OK;
}

/* ONU注册时，分配扩展信息并重置查询标志 */
LONG updateOnuExtMgmtBrdInfo( ULONG devIdx )
{
	ext_board_debug( ("ONU-EXT:update onu %d/%d/%d ext-mgmt info.\r\n", GET_PONSLOT(devIdx), GET_PONPORT(devIdx), GET_ONUID(devIdx)) );
	return onu_ext_dev_dispatch( devIdx, 0, ONU_EXT_DEV_ONU_REG, NULL );
}
/* ONU离线、删除时，释放扩展信息 */
LONG deleteOnuExtMgmtBrdInfo( ULONG devIdx )
{
	ext_board_debug( ("ONU-EXT:delete onu %d/%d/%d ext-mgmt info.\r\n", GET_PONSLOT(devIdx), GET_PONPORT(devIdx), GET_ONUID(devIdx)) );

	return onu_ext_dev_dispatch( devIdx, 0, ONU_EXT_DEV_ONU_DEREG, NULL );
}

/* ONU注册时扩展信息更新回调 */
LONG onuExtMgmt_OnuRegCallback( OnuEventData_s data )
{
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	ULONG pon_slotno, pon_portno;
	if( !OLT_LOCAL_ISVALID(data.PonPortIdx) || !ONU_IDX_ISVALID(data.OnuIdx) )
		return VOS_ERROR;
	pon_slotno = GetCardIdxByPonChip(data.PonPortIdx);
	pon_portno = GetPonPortByPonChip(data.PonPortIdx);

	ext_board_debug( ("ONU-EXT:recv onu %d/%d/%d reg event\r\n", pon_slotno, pon_portno, data.OnuIdx + 1) );
	
	return updateOnuExtMgmtBrdInfo( MAKEDEVID( pon_slotno, pon_portno, data.OnuIdx + 1) );
#else
	return VOS_OK;
#endif
}

/* ONU离线时扩展信息更新回调 */
LONG onuExtMgmt_OnuDeregCallback( OnuEventData_s data )
{
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	ULONG pon_slotno, pon_portno;
	if( !OLT_LOCAL_ISVALID(data.PonPortIdx) || !ONU_IDX_ISVALID(data.OnuIdx) )
		return VOS_ERROR;
	pon_slotno = GetCardIdxByPonChip(data.PonPortIdx);
	pon_portno = GetPonPortByPonChip(data.PonPortIdx);

	ext_board_debug( ("ONU-EXT:recv onu %d/%d/%d dereg event\r\n", pon_slotno, pon_portno, data.OnuIdx + 1) );

	return deleteOnuExtMgmtBrdInfo( MAKEDEVID( pon_slotno, pon_portno, data.OnuIdx + 1) );
#else
	return VOS_OK;
#endif
}

LONG onuExtMgmt_BrdInsertCallback( ULONG devIdx, ULONG brdIdx, UCHAR *pBrdPduData )
{
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	int slot, port;
	short int PonPortIdx, OnuIdx;

	slot = GET_PONSLOT( devIdx );
	port = GET_PONPORT( devIdx );
	if( (slot == 0) || (port == 0) )
		return FALSE;
	OnuIdx = GET_ONUID( devIdx ) - 1;
	PonPortIdx=GetPonPortIdxBySlot(slot, port);

	ext_board_debug( ("ONU-EXT:recv onu %d/%d/%d hot insert event\r\n", slot, port, GET_ONUID(devIdx)) );

	return onu_ext_dev_dispatch( devIdx, brdIdx, ONU_EXT_DEV_BRD_INSERT, pBrdPduData );
#else
	return VOS_OK;
#endif
}

LONG onuExtMgmt_BrdPullCallback( ULONG devIdx, ULONG brdIdx )
{
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	ext_board_debug( ("ONU-EXT:recv onu %d/%d/%d hot pull event\r\n", GET_PONSLOT(devIdx), GET_PONPORT(devIdx), GET_ONUID(devIdx)) );

	return onu_ext_dev_dispatch( devIdx, brdIdx, ONU_EXT_DEV_BRD_PULL, NULL );
#else
	return VOS_OK;
#endif
}

LONG onuExtMgmt_OnuEquInfoCallback( ULONG devIdx, UCHAR *pBrdList )
{	
	ext_board_debug( ("ONU-EXT:recv onu %d/%d/%d board list event\r\n", GET_PONSLOT(devIdx), GET_PONPORT(devIdx), GET_ONUID(devIdx)) );
	return onu_ext_dev_dispatch( devIdx, 0, ONU_EXT_DEV_BRD_LIST_SET, pBrdList );
}

BOOL onu_ext_dev_support( short int PonPortIdx, short int OnuIdx )
{
	BOOL rc = FALSE;
	int onuType = 0;
	int num;
	
	if( !OLT_LOCAL_ISVALID(PonPortIdx) || !ONU_IDX_ISVALID(OnuIdx) )
		return rc;
	if( GetOnuType(PonPortIdx, OnuIdx, &onuType) == VOS_OK )
	{
		if( onuType == V2R1_ONU_GT861 )
			rc = TRUE;
		else
		{
			num = GetOnuSubSlotNum(onuType);
			if( (num > MIN_ONU_BRD_NUM) && (num <=MAX_ONU_BRD_NUM) )
				rc = TRUE;
		}
	}
	return rc;
}

BOOL onuExtMgmt_IsSupport( ULONG devIdx )
{
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	int slot, port;
	short int PonPortIdx, OnuIdx;

	slot = GET_PONSLOT( devIdx );
	port = GET_PONPORT( devIdx );
	if( (slot == 0) || (port == 0) )
		return FALSE;
	OnuIdx = GET_ONUID( devIdx ) - 1;
	PonPortIdx=GetPonPortIdxBySlot(slot, port);

	return onu_ext_dev_support( PonPortIdx, OnuIdx );
#else
	return FALSE;
#endif
}

#if 0
LONG GetFirstExtOnu( ULONG *pOnuDevIdx )
{
	ULONG curOnuDevIdx = 0, nextOnuDevIdx = 0;
	short int slot, port;
	short int PonPortIdx, OnuIdx;
	int i = 0;

	if( pOnuDevIdx == NULL )
		return VOS_ERROR;
	
	while( getNextDeviceEntryIndex(curOnuDevIdx, &nextOnuDevIdx) == VOS_OK )
	{
		slot = GET_PONSLOT(nextOnuDevIdx);
		port = GET_PONPORT(nextOnuDevIdx);
		if( (slot == 0) || (port == 0) )
			break;
		
		OnuIdx = GET_ONUID(nextOnuDevIdx) - 1;
		PonPortIdx = GetPonPortIdxBySlot( slot, port );

		if( onu_ext_dev_support(PonPortIdx, OnuIdx) )
		{
			*pOnuDevIdx = nextOnuDevIdx;
			return VOS_OK;
		}

		if( (++i) >= SYS_MAX_PON_ONUNUM )	/*  防止死循环 */
			break;

		curOnuDevIdx = nextOnuDevIdx;
	}
	return VOS_ERROR;
}


LONG GetNextExtOnu( ULONG curOnuDevIdx, ULONG *pNextOnuDevIdx )
{
	short int slot, port;
	short int PonPortIdx, OnuIdx;
	ULONG nextOnuDevIdx;
	int i=0;

	if( pNextOnuDevIdx == NULL )
		return VOS_ERROR;

	while( getNextDeviceEntryIndex(curOnuDevIdx, &nextOnuDevIdx) == VOS_OK )
	{
		slot = GET_PONSLOT(nextOnuDevIdx);
		port = GET_PONPORT(nextOnuDevIdx);
		if( (slot == 0) || (port == 0) )
			break;

		OnuIdx = GET_ONUID(nextOnuDevIdx) - 1;
		PonPortIdx = GetPonPortIdxBySlot( slot,  port );
		if( onu_ext_dev_support(PonPortIdx, OnuIdx) )
		{
			*pNextOnuDevIdx = nextOnuDevIdx;
			return VOS_OK;
		}

		if( (++i) >= SYS_MAX_PON_ONUNUM )	/*  防止死循环 */
			break;

		curOnuDevIdx = nextOnuDevIdx;
	}

	return VOS_ERROR;
}
#endif


LONG OnuExtMgmt_SyncRecv_Callback( VOID *pMsg )
{
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	return onu_ext_dev_sync_rep_handle( (onu_sync_ext_msg_t  *)pMsg );
#else
	return VOS_OK;
#endif
}

LONG OnuExtMgmt_SyncReqRecv_Callback( VOID *pMsg )
{
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	return onu_ext_dev_sync_req_handle( (onu_sync_msg_head_t  *)pMsg );
#else
	return VOS_OK;
#endif
}


#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
/* begin: added by jianght 20090319  */
/* 清环回，告警状态，E1链接，用于ONU E1板卡拔插 */
LONG onuE1BoardPull( ULONG onuDevId, ULONG brdIdx )
{
	onuE1Info onue1info;
	ULONG idxs[3], i;
	e1PortTable_t e1PortTable;

	if ( VOS_OK != checkOnuE1IsSupportE1(onuDevId, (UCHAR)brdIdx, 0) )
	{
		E1_ERROR_INFO_PRINT(("onuE1BoardPull:checkOnuE1IsSupportE1 error!\r\n"));
		return VOS_ERROR;
	}

	memset((UCHAR *)&onue1info, 0, sizeof(onuE1Info));

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();

	if ( VOS_OK != tdm_getOnuE1Info(idxs, onuDevId, &onue1info) )
	{
		E1_ERROR_INFO_PRINT(("onuE1BoardPull:checkOnuE1IsRight() error!\r\n"));
		return VOS_ERROR;
	}

	for (i = 0; i < onue1info.onuValidE1Count && i < (MAX_ONU_BOARD_E1 * (MAX_ONU_E1_SLOT_ID - MIN_ONU_E1_SLOT_ID + 1)); i++)
	{
		if ( (brdIdx == onue1info.onuEachE1Info[i].onuE1Slot) && (MIB_E1_ENABLE == onue1info.onuEachE1Info[i].tdmE1Enable) )
		{
			idxs[0] = 1;
			idxs[1] = get_gfa_e1_slotno();
			idxs[2] = MAX_E1_PER_FPGA * onue1info.onuEachE1Info[i].tdmSgIfId + onue1info.onuEachE1Info[i].tdmE1Id;

			if ( VOS_OK != e1PortTable_set( LEAF_eponE1PortLoop, idxs, (USHORT)0) )
			{
				E1_ERROR_INFO_PRINT(("onuE1BoardPull:e1PortTable_set error!\r\n"));
				return VOS_ERROR;
			}

			idxs[0] = onuDevId;
			idxs[1] = brdIdx;
			idxs[2] = onue1info.onuEachE1Info[i].onuE1Index;

			if ( VOS_OK != sw_e1PortTable_get(idxs, &e1PortTable) )
			{
				E1_ERROR_INFO_PRINT(("onuE1BoardPull:sw_e1PortTable_get error!\r\n"));
				return VOS_ERROR;
			}

			if (e1PortTable.eponE1PortAlarmStatus & 0x0080)
			{
				e1LosAlarmClear_EventReport(onuDevId, brdIdx, onue1info.onuEachE1Info[i].onuE1Index + 1);
			}

			if ( VOS_OK != sw_e1PortTable_set(LEAF_eponE1PortAlarmStatus, idxs, (USHORT)0 ) )
			{
				E1_ERROR_INFO_PRINT(("onuE1BoardPull:sw_e1PortTable_set error!\r\n"));
				return VOS_ERROR;
			}
		}
	}
	return VOS_OK;
}

/* 用于ONU E1板卡拔插 */
LONG onuE1BoardInsert( ULONG onuDevIdx, ULONG brdIdx )
{
	OAM_OnuE1Info oam_OnuE1Info;

	if ( VOS_OK != checkOnuE1IsSupportE1(onuDevIdx, (UCHAR)brdIdx, 0) )
	{
		E1_ERROR_INFO_PRINT(("onuE1BoardInsert:checkOnuE1IsSupportE1 error!\r\n"));
		return VOS_ERROR;
	}

	VOS_MemZero( (UCHAR *)&oam_OnuE1Info, sizeof(OAM_OnuE1Info) );

	if ( VOS_OK != GetOamOnuBoardE1Info(onuDevIdx, (UCHAR)brdIdx, &oam_OnuE1Info) )
	{
		E1_ERROR_INFO_PRINT(("onuE1BoardInsert:GetOamOnuBoardE1Info error!\r\n"));
		return VOS_ERROR;
	}

	SetOnuE1All( onuDevIdx, &oam_OnuE1Info );
	return VOS_OK;
}

/* OLT侧E1板拔出处理 */
LONG oltE1BoardPull( ULONG slotno )
{
	ULONG idxs[3], i;
	e1PortTable_t e1PortTable;

	idxs[0] = 1;
	idxs[1] = slotno;/* 从参数取得TDM板槽位号 */

	/* begin: added by jianght 20090804 */
	/* 打开端口自协商 */
	/*for (i = 0; i < TDM_FPGA_MAX; i++)
	{
		if (VOS_ERROR == setEthPortAutoNegotiation(slotno, i + 1, 1))
		{
			sys_console_printf("oltE1BoardPull()::setEthPortAutoNegotiation(%d,%d,%d)  failed!\r\n", slotno, i + 1, 1);
		}
	}*/
	/* end: added by jianght 20090804 */

	for (i = 0; i < (MAX_E1_PER_FPGA * TDM_FPGA_MAX); i++)
	{
		idxs[2] = i;

		if ( VOS_OK != sw_e1PortTable_get(idxs, &e1PortTable) )
		{
			E1_ERROR_INFO_PRINT(("tdmBoardAlarmStatus_update:sw_e1PortTable_get error!\r\n"));
			continue;
		}

		if (e1PortTable.eponE1PortAlarmStatus & 0x0080)
		{
			e1LosAlarmClear_EventReport(1, idxs[1], idxs[2] + 1);
		}

		if (e1PortTable.eponE1PortAlarmStatus & 0x0100)
		{
			E1OutOfServiceClear_EventReport(1, idxs[1], idxs[2] + 1);
		}

		if (e1PortTable.eponE1PortAlarmStatus & 0x0020)
		{
			e1AisAlarmClear_EventReport(1, idxs[1], idxs[2] + 1);
		}

		if ( VOS_OK != sw_e1PortTable_set(LEAF_eponE1PortAlarmStatus, idxs, 0 ) )
		{
			E1_ERROR_INFO_PRINT(("tdmBoardAlarmStatus_update:sw_e1PortTable_set error!\r\n"));
		}

		/* 清除SW上OLT侧的环回 */
		if ( VOS_OK != sw_e1PortTable_set(LEAF_eponE1PortLoop, idxs, 0 ) )
		{
			E1_ERROR_INFO_PRINT(("tdmBoardAlarmStatus_update:sw_e1PortTable_set error!\r\n"));
		}
	}
	return VOS_OK;
}

/* Pon板被拔出处理 */
LONG oltPonBoardPull( ULONG slotno )
{
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	ULONG portno;
	ULONG e1BrdIdx, e1Port;
	ULONG onuDevIdx;
	SHORT PonPortIdx, OnuIdx;
	ULONG idxs[3];
	onu_ext_table_t *pOnuExtTbl;
	onu_ext_brd_table_t *pExtBrdTbl = NULL;

	if ( 0 == get_gfa_e1_slotno() )
	{
		return VOS_OK;
	}

	VOS_SemTake( onuExtMSemId, WAIT_FOREVER );
	
	for( portno=0; portno<=PONPORTPERCARD; portno++)
	{
		/* 清除ONU的LOS告警 */
		PonPortIdx = GetPonPortIdxBySlot( slotno, portno );
		if( PonPortIdx == RERROR )
			continue;

		for( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++ )
		{
			pOnuExtTbl = onu_ext_dev_search( PonPortIdx, OnuIdx );
			if( pOnuExtTbl == NULL )
				continue;
			
			for( e1BrdIdx = MIN_ONU_E1_SLOT_ID; e1BrdIdx <= MAX_ONU_E1_SLOT_ID; e1BrdIdx++ )
			{
				pExtBrdTbl = onu_ext_brd_search( PonPortIdx, OnuIdx, e1BrdIdx );
				if( pExtBrdTbl == NULL )
					continue;
				
                            onuDevIdx = MAKEDEVID( slotno, portno, (OnuIdx+1) );
				if (VOS_OK != checkOnuE1IsSupportE1( onuDevIdx, e1BrdIdx, 0) )
					continue;

				idxs[0] = onuDevIdx;
				idxs[1] = e1BrdIdx;
				for( e1Port=1; e1Port<=MAX_ONU_BOARD_E1; e1Port++ )
				{
					if ( pExtBrdTbl->onuE1PortTable[e1Port-1].eponE1PortAlarmStatus & 0x0080 )
					{
						pExtBrdTbl->onuE1PortTable[e1Port-1].eponE1PortAlarmStatus &= ~0x0080;
						e1LosAlarmClear_EventReport( onuDevIdx, e1BrdIdx, e1Port );
					}

					/* 清除远端ONU环回
					pOnuExtTbl->BrdMgmtTable[e1BrdIdx].onuE1PortTable[e1Port-1].eponE1Loop &= ONU_E1_NO_LOOP; */

					idxs[2] = e1Port-1;
					if ( VOS_OK != e1PortTable_set( LEAF_eponE1PortLoop, idxs, (USHORT)0 ) )
					{
						E1_ERROR_INFO_PRINT(("oltPonBoardPull:e1PortTable_set error!\r\n"));
						return VOS_ERROR;
					}
				}
			}
		}
	}
	VOS_SemGive( onuExtMSemId );
#endif
	return VOS_OK;
}

/* ONU离线和掉电处理 */
LONG e1OnuTakeOff(ULONG onuDevIdx)
{
	ULONG idxs[3], i, j;
	e1PortTable_t e1PortTable;

	if ( 0 == get_gfa_e1_slotno() )
	{
		return VOS_OK;
	}

	if ( VOS_OK != onuDevIdxIsSupportE1(onuDevIdx) )
	{
		return VOS_OK;
	}

	idxs[0] = onuDevIdx;

	for( i=MIN_ONU_E1_SLOT_ID; i<=MAX_ONU_E1_SLOT_ID; i++ )
	{
		if( VOS_OK != checkOnuE1IsSupportE1(onuDevIdx, (UCHAR)i, 0) )
		{
			E1_ERROR_INFO_PRINT(("e1OnuTakeOff:checkOnuE1IsSupportE1 slotIdx=%d\r\n", i));
			continue;
		}

		idxs[1] = i;

		for( j=0; j<MAX_ONU_BOARD_E1; j++ )
		{
			idxs[2] = j;

			if ( VOS_OK != sw_e1PortTable_get(idxs, &e1PortTable) )
			{
				E1_ERROR_INFO_PRINT(("e1OnuTakeOff:sw_e1PortTable_get slotIdx=%d port=%d\r\n", i, j));
				continue;
			}

			if( e1PortTable.eponE1PortAlarmStatus & 0x0080 )
			{
				e1LosAlarmClear_EventReport( idxs[0], idxs[1], idxs[2]+1 );
			}

			e1PortTable.eponE1PortAlarmStatus &= (~0x0080);

			if ( VOS_OK != sw_e1PortTable_set(LEAF_eponE1PortAlarmStatus, idxs, e1PortTable.eponE1PortAlarmStatus) )
			{
				E1_ERROR_INFO_PRINT(("e1OnuTakeOff:sw_e1PortTable_set error!\r\n"));
			}

			if ( VOS_OK != e1PortTable_set(LEAF_eponE1PortLoop, idxs, (USHORT)0) )
			{
				E1_ERROR_INFO_PRINT(("e1OnuTakeOff:sw_e1PortTable_set error!\r\n"));
			}
		}
	}
	return VOS_OK;
}
/* end: added by jianght 20090319 */
#endif

int ShowExtOnuDeviceInfoByVty( short int PonPortIdx, short int OnuIdx, struct vty *vty )
{
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	short int OnuEntry;
	short int slot, port;
	ULONG onuDevIdx;
	unsigned char TempString[256];
	int i=1;
	ULONG BoardNum = 0/*,BoardList[4]={0}*/;
	onu_ext_brd_table_t extBrdTbl;
	/*UCHAR BrdTypeDesc[32]={0};*/

	unsigned char AppPrefix[36], AppSuffix[36];
	unsigned char  AppPrefixLen=0,  AppSuffixLen = 0;
	unsigned char FwPrefix[36], FwSuffix[36];
	unsigned char  FwPrefixLen=0,  FwSuffixLen = 0;

	CHECK_ONU_RANGE

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	onuDevIdx=MAKEDEVID( slot, port, (OnuIdx+1) );

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
	{
		vty_out( vty , "\r\nExt:ONU %d/%d/%d not exist\r\n", slot, port, (OnuIdx+1) );
		return( RERROR );
	}
	
	VOS_MemSet( TempString, 0, sizeof(TempString) );
		
	/*Basic */
#if  0
	vty_out(vty, "Onu Type:%s \r\n", V2r1DeviceType[OnuMgmtTable[OnuEntry].DeviceInfo.type]);
	vty_out(vty, "Mac addr:");
	{
		for(count = 0; count < ( BYTES_IN_MAC_ADDRESS-1); count++ )
			{
			vty_out(vty, "%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[count], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[count+1] );
			count++;
			if( count != 5 )vty_out(vty, ".");
			}
		vty_out(vty, "\r\n\r\n");
	}
#endif
	getOnuExtMgmtBrdNum( onuDevIdx, &BoardNum );
	vty_out( vty, "onu%d/%d/%d has %d sub-board online,board list:\r\n", slot, port, (OnuIdx+1), BoardNum );
	
	/* modified by chenfj 2008-10-15
	     1 . ONU上在线的板卡可能不是连续的, 中间会有空槽位, 应遍历所有槽位
	     2 . 显示子板信息时, 应同时显示子板的槽位号
	     3 . 调试显示顺序, 将几个版本相关的信息放在一起
	     */
	/* modified by chenfj 2008-10-28
    		若ONU 中有TDM FPGA, 则将其从固件版本号中分离出来,单独一行显示
    		同样处理VOICE 程序
	*/
	
	for( i=1; i<=MAX_ONU_BRD_NUM; i++ )
	{
		VOS_MemZero( &extBrdTbl, sizeof(extBrdTbl) );
		if( onu_ext_brd_info_get(PonPortIdx, OnuIdx, i, &extBrdTbl) == VOS_ERROR )
			continue;
		
		if( !ONU_BRD_TYPE_IS_VALID(extBrdTbl.DeviceInfo.brdType) )
			continue;
				
		VOS_Sprintf( TempString, "board type:%s", onu_ext_brd_type2name(extBrdTbl.DeviceInfo.brdType) );
		vty_out( vty, "%-34s", TempString );
		vty_out( vty, "slot num:%d\r\n", i );

		if( (extBrdTbl.DeviceInfo.brdType != OAM_ONU_SLOT_GT_6FE) && (extBrdTbl.DeviceInfo.brdType != OAM_ONU_SLOT_GT_8FE) &&
			(extBrdTbl.DeviceInfo.brdType != OAM_ONU_SLOT_GT_16FE) )
		{
			VOS_Sprintf( TempString, "Boot Ver:%s", extBrdTbl.DeviceInfo.BootVersion );
			vty_out( vty, "%-34s", TempString );

			if( OnuEponAppHasVoiceApp(PonPortIdx,OnuIdx) != ROK )
				vty_out(vty, "Software Ver:%s\r\n", extBrdTbl.DeviceInfo.SwVersion );
			else
			{
				VOS_StrCpy( TempString, extBrdTbl.DeviceInfo.SwVersion );
				ParsePrefixAndSuffixFromString( TempString, AppPrefix, &AppPrefixLen, AppSuffix, &AppSuffixLen );
				if( AppPrefixLen == 0 )
					vty_out( vty, "Software Ver:%s\r\n", TempString );
				else
					vty_out( vty, "Software Ver:%s\r\n",  AppPrefix );
			}
		}

		VOS_Sprintf( TempString, "Hardware Ver:%s", extBrdTbl.DeviceInfo.HwVersion );
		vty_out( vty, "%-34s", TempString );
			
		if( (extBrdTbl.DeviceInfo.brdType != OAM_ONU_SLOT_GT_6FE) && (extBrdTbl.DeviceInfo.brdType != OAM_ONU_SLOT_GT_8FE) &&
			(extBrdTbl.DeviceInfo.brdType != OAM_ONU_SLOT_GT_16FE) )
		{
			if( OnuFirmwareHasFpgaApp(PonPortIdx,OnuIdx) != ROK )
				vty_out( vty, "Firmware Ver:%s", extBrdTbl.DeviceInfo.FwVersion );
			else
			{
				VOS_StrCpy( TempString, extBrdTbl.DeviceInfo.FwVersion );
				ParsePrefixAndSuffixFromString( TempString,  FwPrefix,  &FwPrefixLen,  FwSuffix,  &FwSuffixLen );
				if( FwPrefixLen == 0 )
					vty_out( vty, "Firmware Ver:%s", TempString );
				else
					vty_out(vty, "Firmware Ver:%s", FwPrefix );
				}
		}
		vty_out( vty, "\r\n" );

		if( AppSuffixLen != 0 )
		{
			VOS_Sprintf( TempString, "Voip Ver:%s", AppSuffix );
			vty_out( vty, "%-34s\r\n", TempString );
		}
		if( FwSuffixLen != 0)
		{
			vty_out( vty, "Tdm Fpga:%s\r\n", FwSuffix );
		}

		{
			/* modified by chenfj 2008-10-27
			    修改子板运行状态变化时间显示; 格式为天:时:分:秒
			    */
			ULONG days, hours,minutes, seconds;

			seconds = extBrdTbl.DeviceInfo.ChangeTime /100;
			days = 0;
			hours = 0;
			minutes = 0;
			days = seconds /V2R1_ONE_DAY;
			hours = (seconds % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
			minutes = ((seconds % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
			seconds = ((seconds % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;
				
			VOS_Sprintf( TempString, "change time:%04u:%02u:%02u:%02u", days, hours, minutes, seconds );	/* modified by xieshl 20100319, GWD网上问题9966 */
			vty_out(vty,"%-34s", TempString);
		}
		
		VOS_Sprintf( TempString, "Operate Status: %s", onu_ext_brd_oper_state2str(extBrdTbl.DeviceInfo.OperatStatus-1) );
		vty_out( vty, "%s\r\n", TempString );

		VOS_Sprintf( TempString, "product Date:%s", extBrdTbl.DeviceInfo.ProductTime );
		vty_out( vty, "%-34s", TempString);
			
		vty_out( vty, "Serial Num:%s\r\n", extBrdTbl.DeviceInfo.SerialNum );

		VOS_Sprintf( &TempString[0], "Manufacture :%s\r\n", extBrdTbl.DeviceInfo.Manufacture );
		vty_out(vty, "%s", TempString);

		vty_out(vty, "\r\n");
	}
#endif
	return VOS_OK;
}


int cl_voice_onu_show( struct vty *vty, ULONG ulSlot, ULONG ulPort, ULONG ulOnuid )
{
	ulong_t brdIdx=0;
	ulong_t ulAgentIp=0, SupportAgent=0;
	char pReadComm[MAX_BRD_COMM_LEN], pWriteComm[MAX_BRD_COMM_LEN];
	unsigned char OnuString[20];
	ULONG status = 0;

	ulong_t onuDevIdx =MAKEDEVID(ulSlot,ulPort,ulOnuid) /*ulSlot*10000+ulPort*1000+ulOnuid*/;

	if( (getDeviceOperStatus(onuDevIdx, &status) != VOS_OK) || (status != 1) )
	{
		vty_out( vty, "ONU %d/%d/%d is not active\r\n", ulSlot, ulPort, ulOnuid );
		return CMD_WARNING;
	}
	
	vty_out(vty,"\r\nslot   SnmpAgent       ReadCommnity         WriteCommnity\r\n");
	vty_out(vty,"---------------------------------------------------------\r\n");

	for(brdIdx=1;brdIdx<=MAX_ONU_BRD_NUM;brdIdx++)
	{	
		status = 0;
		if( (getBoardOperStatus(onuDevIdx, brdIdx, &status) != VOS_OK) || (status != 4) )
			continue;

	 	if(getBoardHasSnmpAgent( onuDevIdx,  brdIdx, &SupportAgent)==VOS_OK)
	 	{	
			if(SupportAgent!=0)/*如果该板子支持SNMP Agent ,显示该板上的信息*/
			{
				getOnuExtMgmtBrdAgentIp( onuDevIdx, brdIdx, &ulAgentIp);
				
				getOnuExtMgmtBrdReadCommnity( onuDevIdx, brdIdx, pReadComm);
				getOnuExtMgmtBrdWriteCommnity( onuDevIdx, brdIdx, pWriteComm);

				vty_out(vty,"%-7d", brdIdx);
				VOS_Sprintf(OnuString,"%d.%d.%d.%d",((ulAgentIp >> 24) & 0xff),
					((ulAgentIp>> 16) & 0xff),
					((ulAgentIp>> 8) & 0xff), 
					(ulAgentIp& 0xff));
				vty_out(vty,"%-17s%-22s%s\r\n", OnuString, pReadComm, pWriteComm );
			}
		}	 	
	}

	return CMD_SUCCESS;
}

DEFUN  (
    voice_show_device_information,
    voice_show_device_information_cmd,
    "Show voice ip onu",
    DescStringCommonShow
    "Show the voice onu device information\n"
    "Show the onu device information\n"
    "show the voice onu\n"
    )
{
	ULONG ulSlot=0,ulPort=0,ulOnuid=0;

	if( PON_GetSlotPortOnu( ( ULONG ) ( vty->index ), &ulSlot, &ulPort, &ulOnuid) == VOS_OK )
		return cl_voice_onu_show( vty, ulSlot, ulPort, ulOnuid );

	return CMD_WARNING;
}

#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
DEFUN  (
    voice_onu_show_device_information,
    voice_onu_show_device_information_cmd,
    "Show voice ip onu <slot/port/onuid>",
    DescStringCommonShow
    "Show the voice onu device information\n"
    "Show the onu device information\n"
    "show the voice onu\n"
    "input the slot,port,onuid like: 5/2/1\n"
    )
{
	ULONG ulSlot=0,ulPort=0,ulOnuid=0;

	if( PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid ) == VOS_OK )
		return cl_voice_onu_show( vty, ulSlot, ulPort, ulOnuid );

    	return CMD_WARNING;
}
#endif

extern long get_long_from_ipdotstring(char* ipaddr);
extern void get_ipdotstring_from_long(char* ipaddr,int ip);

DEFUN  (
    voice_onu_mac_show_onuindex,
    voice_onu_mac_show_onuindex_cmd,
    "Show voice ip {<A.B.C.D>}*1",
    DescStringCommonShow
    "Show the voice onu device information\n"
    "Show the onu device information\n"
    "input the ip add like : 192.168.1.1\n"
    )
{	
	int i,j,k;
	ULONG onuDevIdx, status;
	ulong_t hasSnmpAgent;
	ulong_t ulgetIp,brdIdx,ulputIp;
	uchar ulipstr[16]={'\0'};

	
	 vty_out(vty,"\r\n  SnmpAgent         onuDevIdx\r\n");
	 vty_out(vty,"-------------------------------\r\n");

	 if(argc!=0)
	{	
		ulputIp = get_long_from_ipdotstring(argv[0]);
		/*if( ulputIp == 0 )
			return CMD_SUCCESS;*/
		
		for(i=PONCARD_FIRST;i<=PONCARD_LAST;i++)
		{
		       if( __SYS_MODULE_TYPE__(i) == MODULE_TYPE_NULL )	
       		{
       			/*sys_console_printf(" %% slot %d is not inserted\r\n", ulSlot);*/ 
       			continue;
       		}
       		
       		if(SlotCardIsPonBoard(i) != ROK )		
       		{
       			/*sys_console_printf(" %% slot %d is not pon card\r\n", ulSlot);*/ 
       			continue;
       		}
       		if(!SYS_MODULE_IS_PON(i))
                     {
                            continue;
                     }
			for(j=1;j<= MAX_PONPORT_PER_BOARD;j++)
			{
				for(k=1;k<=MAXONUPERPON;k++)
				{
					onuDevIdx=MAKEDEVID(i,j,k)/*i*10000+j*1000+k*/;
					status = 0;
					if( (getDeviceOperStatus(onuDevIdx, &status) != VOS_OK) || (status != 1) )
						continue;
						
					for(brdIdx=1;brdIdx<=MAX_ONU_BRD_NUM;brdIdx++)
					{
						status = 0;
						if( (getBoardOperStatus(onuDevIdx, brdIdx, &status) != VOS_OK) || (status != 4) )
							continue;

						hasSnmpAgent = 0;
						if( (getBoardHasSnmpAgent( onuDevIdx,  brdIdx, &hasSnmpAgent) != VOS_OK) ||(hasSnmpAgent == 0) )
							continue;

						getOnuExtMgmtBrdAgentIp( onuDevIdx, brdIdx, &ulgetIp);
						if(ulputIp==ulgetIp)
						{
							vty_out(vty,"%-20s%d\r\n", argv[0], onuDevIdx );
						}
					}
				}
			}
		}
	}
	else
	{
		for(i=PONCARD_FIRST;i<=PONCARD_LAST;i++)
		{
		       if( __SYS_MODULE_TYPE__(i) == MODULE_TYPE_NULL )	
       		{
       			/*sys_console_printf(" %% slot %d is not inserted\r\n", ulSlot);*/ 
       			continue;
       		}
       		
       		if(SlotCardIsPonBoard(i) != ROK )		
       		{
       			/*sys_console_printf(" %% slot %d is not pon card\r\n", ulSlot);*/ 
       			continue;
       		}
       		if(!SYS_MODULE_IS_PON(i))
                     {
                            continue;
                     }
			for(j=1;j<= MAX_PONPORT_PER_BOARD;j++)
			{
				for(k=1;k<=MAXONUPERPON;k++)
				{
					onuDevIdx=MAKEDEVID(i,j,k)/*i*10000+j*1000+k*/;
					status = 0;
					if( (getDeviceOperStatus(onuDevIdx, &status) != VOS_OK) || (status != 1) )
						continue;
						
					for(brdIdx=1;brdIdx<=MAX_ONU_BRD_NUM;brdIdx++)
					{
						status = 0;
						if( (getBoardOperStatus(onuDevIdx, brdIdx, &status) != VOS_OK) || (status != 4) )
							continue;
						
						/*查找所有支持snmpAgent的onu板卡，并显示*/
						hasSnmpAgent = 0;
						if( (getBoardHasSnmpAgent( onuDevIdx,  brdIdx, &hasSnmpAgent) != VOS_OK) ||(hasSnmpAgent == 0) )
							continue;

						getOnuExtMgmtBrdAgentIp( onuDevIdx, brdIdx, &ulgetIp);
						
						get_ipdotstring_from_long( ulipstr, ulgetIp );
						vty_out(vty,"%-20s%d\r\n", ulipstr, onuDevIdx );
					}
				}
			}
		}
	}
	return CMD_SUCCESS;
}


LONG ExtBrd_CommandInstall()
{
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	install_element ( CONFIG_NODE, &voice_onu_mac_show_onuindex_cmd);
	install_element ( CONFIG_NODE, &voice_onu_show_device_information_cmd);
#endif
	return VOS_OK;
}

LONG ONU_ExtBrd_IAD_CommandInstall( enum node_type  node)
{
	install_element ( node, &voice_onu_mac_show_onuindex_cmd);
	install_element ( node, &voice_show_device_information_cmd);
	return VOS_OK;
}



#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
#if 0
/* for test only */
LONG addExtOnu( ULONG slotno, ULONG portno, ULONG onuid )
{
	LONG rc = VOS_ERROR;
	short int PonPortIdx, OnuIdx;
	onu_ext_table_t *pDev;
	onu_ext_brd_table_t *pBrd;

	OnuIdx = onuid - 1;
	PonPortIdx=GetPonPortIdxBySlot(slotno, portno);

	VOS_SemTake( onuExtMSemId, WAIT_FOREVER );
	pDev = onu_ext_dev_new(PonPortIdx, OnuIdx);
	if( pDev )
	{
		SetOnuType(PonPortIdx, OnuIdx, V2R1_ONU_GT861);
		
		rc = VOS_OK;
		pDev->QueryFlag = ONU_EXT_QUERY_OK;
		
		pBrd = onu_ext_brd_search(PonPortIdx, OnuIdx, 1);
		pBrd->DeviceInfo.brdType = OAM_ONU_SLOT_GT_EPON_B;	/* GT-EPON */
		/*VOS_StrCpy( pBrd->DeviceInfo.DeviceDesc, "desc1" );*/
		VOS_StrCpy( pBrd->DeviceInfo.SwVersion, "sw-ver1" );
		VOS_StrCpy( pBrd->DeviceInfo.HwVersion, "hw-ver1" );
		VOS_StrCpy( pBrd->DeviceInfo.BootVersion, "boot-ver1" );
		VOS_StrCpy( pBrd->DeviceInfo.FwVersion, "fw-ver1" );
		VOS_StrCpy( pBrd->DeviceInfo.Manufacture, "gwd" );
		VOS_StrCpy( pBrd->DeviceInfo.SerialNum, "A001" );
		VOS_StrCpy( pBrd->DeviceInfo.ProductTime, "20120901" );
		pBrd->DeviceInfo.OperatStatus = 4;

		pBrd = onu_ext_brd_search(PonPortIdx, OnuIdx, 2);
		pBrd->DeviceInfo.brdType = OAM_ONU_SLOT_GT_6FE;		/* GT-6FE */
		/*VOS_StrCpy( pBrd->DeviceInfo.DeviceDesc, "desc2" );*/
		VOS_StrCpy( pBrd->DeviceInfo.SwVersion, "sw-ver2" );
		VOS_StrCpy( pBrd->DeviceInfo.HwVersion, "hw-ver2" );
		VOS_StrCpy( pBrd->DeviceInfo.BootVersion, "boot-ver2" );
		VOS_StrCpy( pBrd->DeviceInfo.FwVersion, "fw-ver2" );
		VOS_StrCpy( pBrd->DeviceInfo.Manufacture, "gwd" );
		VOS_StrCpy( pBrd->DeviceInfo.SerialNum, "B002" );
		VOS_StrCpy( pBrd->DeviceInfo.ProductTime, "20120902" );
		pBrd->DeviceInfo.OperatStatus = 4;

		pBrd = onu_ext_brd_search(PonPortIdx, OnuIdx, 3);
		pBrd->DeviceInfo.brdType = OAM_ONU_SLOT_GT_16FE;	/* GT-16FE */
		/*VOS_StrCpy( pBrd->DeviceInfo.DeviceDesc, "desc3" );*/
		VOS_StrCpy( pBrd->DeviceInfo.SwVersion, "sw-ver3" );
		VOS_StrCpy( pBrd->DeviceInfo.HwVersion, "hw-ver3" );
		VOS_StrCpy( pBrd->DeviceInfo.BootVersion, "boot-ver3" );
		VOS_StrCpy( pBrd->DeviceInfo.FwVersion, "fw-ver3" );
		VOS_StrCpy( pBrd->DeviceInfo.Manufacture, "gwd" );
		VOS_StrCpy( pBrd->DeviceInfo.SerialNum, "C003" );
		VOS_StrCpy( pBrd->DeviceInfo.ProductTime, "20120903" );
		pBrd->DeviceInfo.OperatStatus = 4;

		pBrd = onu_ext_brd_search(PonPortIdx, OnuIdx, 4);
		pBrd->DeviceInfo.brdType = OAM_ONU_SLOT_GT_8FXS_A;	/* GT-8FXS-A */
		/*VOS_StrCpy( pBrd->DeviceInfo.DeviceDesc, "desc4" );*/
		VOS_StrCpy( pBrd->DeviceInfo.SwVersion, "sw-ver4" );
		VOS_StrCpy( pBrd->DeviceInfo.HwVersion, "hw-ver4" );
		VOS_StrCpy( pBrd->DeviceInfo.BootVersion, "boot-ver4" );
		VOS_StrCpy( pBrd->DeviceInfo.FwVersion, "fw-ver4" );
		VOS_StrCpy( pBrd->DeviceInfo.Manufacture, "gwd" );
		VOS_StrCpy( pBrd->DeviceInfo.SerialNum, "D004" );
		VOS_StrCpy( pBrd->DeviceInfo.ProductTime, "20120904" );
		pBrd->DeviceInfo.OperatStatus = 4;
		
		pBrd = onu_ext_brd_search(PonPortIdx, OnuIdx, 5);
		pBrd->DeviceInfo.brdType = OAM_ONU_SLOT_GT_8POTS_A;	/* GT-8POTS-A */
		/*VOS_StrCpy( pBrd->DeviceInfo.DeviceDesc, "desc5" );*/
		VOS_StrCpy( pBrd->DeviceInfo.SwVersion, "sw-ver5" );
		VOS_StrCpy( pBrd->DeviceInfo.HwVersion, "hw-ver5" );
		VOS_StrCpy( pBrd->DeviceInfo.BootVersion, "boot-ver5" );
		VOS_StrCpy( pBrd->DeviceInfo.FwVersion, "fw-ver5" );
		VOS_StrCpy( pBrd->DeviceInfo.Manufacture, "gwd" );
		VOS_StrCpy( pBrd->DeviceInfo.SerialNum, "E005" );
		VOS_StrCpy( pBrd->DeviceInfo.ProductTime, "20120905" );
		pBrd->DeviceInfo.OperatStatus = 4;
		pBrd->DeviceInfo.SupportSnmpAgent = 2;
		pBrd->DeviceInfo.AgentIp = 0xc0a800eb;
		VOS_StrCpy(pBrd->DeviceInfo.ReadCommunity, "public");
		VOS_StrCpy(pBrd->DeviceInfo.WriteCommunity, "private");
	}
	VOS_SemGive( onuExtMSemId );
	return rc;
}
LONG clsExtOnu( ULONG slotno, ULONG portno, ULONG onuid )
{
	LONG rc = VOS_ERROR;
	short int PonPortIdx, OnuIdx;
	onu_ext_table_t *pDev;
	onu_ext_brd_table_t *pBrd;

	OnuIdx = onuid - 1;
	PonPortIdx=GetPonPortIdxBySlot(slotno, portno);

	VOS_SemTake( onuExtMSemId, WAIT_FOREVER );
	pDev = onu_ext_dev_new(PonPortIdx, OnuIdx);
	if( pDev )
	{
		rc = VOS_OK;
		pDev->QueryFlag = ONU_EXT_QUERY_WAIT;

		pBrd = onu_ext_brd_search(PonPortIdx, OnuIdx, 1);
		pBrd->DeviceInfo.brdType = OAM_ONU_SLOT_GT_EPON_B;	/* GT-EPON */
		/*VOS_StrCpy( pBrd->DeviceInfo.DeviceDesc, "DESC1" );*/
		VOS_StrCpy( pBrd->DeviceInfo.SwVersion, "SW-VER1" );
		VOS_StrCpy( pBrd->DeviceInfo.HwVersion, "HW-VER1" );
		VOS_StrCpy( pBrd->DeviceInfo.BootVersion, "BOOT-VER1" );
		VOS_StrCpy( pBrd->DeviceInfo.FwVersion, "FW-VER1" );
		VOS_StrCpy( pBrd->DeviceInfo.Manufacture, "GWD" );
		VOS_StrCpy( pBrd->DeviceInfo.SerialNum, "A001" );
		VOS_StrCpy( pBrd->DeviceInfo.ProductTime, "2012-9-1" );
		pBrd->DeviceInfo.OperatStatus = 4;

		pBrd = onu_ext_brd_search(PonPortIdx, OnuIdx, 2);
		pBrd->DeviceInfo.brdType = OAM_ONU_SLOT_GT_6FE;		/* GT-6FE */
		/*VOS_StrCpy( pBrd->DeviceInfo.DeviceDesc, "DESC2" );*/
		VOS_StrCpy( pBrd->DeviceInfo.SwVersion, "SW-VER2" );
		VOS_StrCpy( pBrd->DeviceInfo.HwVersion, "HW-VER2" );
		VOS_StrCpy( pBrd->DeviceInfo.BootVersion, "BOOT-VER2" );
		VOS_StrCpy( pBrd->DeviceInfo.FwVersion, "FW-VER2" );
		VOS_StrCpy( pBrd->DeviceInfo.Manufacture, "GWD" );
		VOS_StrCpy( pBrd->DeviceInfo.SerialNum, "B002" );
		VOS_StrCpy( pBrd->DeviceInfo.ProductTime, "2012-9-2" );
		pBrd->DeviceInfo.OperatStatus = 4;

		pBrd = onu_ext_brd_search(PonPortIdx, OnuIdx, 3);
		pBrd->DeviceInfo.brdType = OAM_ONU_SLOT_GT_16FE;	/* GT-16FE */
		/*VOS_StrCpy( pBrd->DeviceInfo.DeviceDesc, "DESC3" );*/
		VOS_StrCpy( pBrd->DeviceInfo.SwVersion, "SW-VER3" );
		VOS_StrCpy( pBrd->DeviceInfo.HwVersion, "HW-VER3" );
		VOS_StrCpy( pBrd->DeviceInfo.BootVersion, "BOOT-VER3" );
		VOS_StrCpy( pBrd->DeviceInfo.FwVersion, "FW-VER2" );
		VOS_StrCpy( pBrd->DeviceInfo.Manufacture, "GWD" );
		VOS_StrCpy( pBrd->DeviceInfo.SerialNum, "B003" );
		VOS_StrCpy( pBrd->DeviceInfo.ProductTime, "2012-9-3" );
		pBrd->DeviceInfo.OperatStatus = 4;

		pBrd = onu_ext_brd_search(PonPortIdx, OnuIdx, 4);
		pBrd->DeviceInfo.brdType = OAM_ONU_SLOT_GT_8FXS_A;	/* GT-8FXS-A */
		/*VOS_StrCpy( pBrd->DeviceInfo.DeviceDesc, "DESC4" );*/
		VOS_StrCpy( pBrd->DeviceInfo.SwVersion, "SW-VER4" );
		VOS_StrCpy( pBrd->DeviceInfo.HwVersion, "HW-VER4" );
		VOS_StrCpy( pBrd->DeviceInfo.BootVersion, "BOOT-VER4" );
		VOS_StrCpy( pBrd->DeviceInfo.FwVersion, "FW-VER4" );
		VOS_StrCpy( pBrd->DeviceInfo.Manufacture, "GWD" );
		VOS_StrCpy( pBrd->DeviceInfo.SerialNum, "B004" );
		VOS_StrCpy( pBrd->DeviceInfo.ProductTime, "2012-9-4" );
		pBrd->DeviceInfo.OperatStatus = 4;
		
		pBrd = onu_ext_brd_search(PonPortIdx, OnuIdx, 5);
		pBrd->DeviceInfo.brdType = OAM_ONU_SLOT_GT_8POTS_A;	/* GT-8POTS-A */
		/*VOS_StrCpy( pBrd->DeviceInfo.DeviceDesc, "DESC5" );*/
		VOS_StrCpy( pBrd->DeviceInfo.SwVersion, "SW-VER5" );
		VOS_StrCpy( pBrd->DeviceInfo.HwVersion, "HW-VER5" );
		VOS_StrCpy( pBrd->DeviceInfo.BootVersion, "BOOT-VER5" );
		VOS_StrCpy( pBrd->DeviceInfo.FwVersion, "FW-VER5" );
		VOS_StrCpy( pBrd->DeviceInfo.Manufacture, "GWD" );
		VOS_StrCpy( pBrd->DeviceInfo.SerialNum, "B005" );
		VOS_StrCpy( pBrd->DeviceInfo.ProductTime, "2012-9-5" );
		pBrd->DeviceInfo.OperatStatus = 4;
	}
	VOS_SemGive( onuExtMSemId );
	return rc;
}

LONG testReadExtOnuOam( short int PonPortIdx, short int OnuIdx, UCHAR *pRx )
{
	UCHAR oam_msg[EUQ_MAX_OAM_PDU] = {
		0x05,0x01,0x05,
		0x01, 0x01,0x04,
		0x01,0x00,0x00,0x00,
		0x04, 'V', 'h', '1', '1',
		0x04, 'V', 'b', '1', '1',
		0x04, 'V', 's', '1', '1',
		0x04, 'V', 'f', '0', '1',
		0x03, 'G', 'W', 'D',
		0x04, 'S', 'N', '8', '1',
		0x08, 0x32, 0x30, 0x31, 0x32, 0x30, 0x39, 0x31, 0x33,
		0x00,

		0x02, 0x02,0x04,
		0x01,0x00,0x00,0x00,
		0x04, 'V', 'h', '1', '2',
		0x04, 'V', 'b', '1', '2',
		0x04, 'V', 's', '1', '2',
		0x04, 'V', 'f', '0', '2',
		0x03, 'G', 'W', 'D',
		0x04, 'S', 'N', '8', '2',
		0x08, 0x32, 0x30, 0x31, 0x32, 0x30, 0x39, 0x31, 0x33,
		0x00,

		0x03, 0x03,0x04,
		0x01,0x00,0x01,0x00,
		0x04, 'V', 'h', '1', '3',
		0x04, 'V', 'b', '1', '3',
		0x04, 'V', 's', '1', '3',
		0x04, 'V', 'f', '0', '3',
		0x03, 'G', 'W', 'D',
		0x04, 'S', 'N', '8', '3',
		0x08, 0x32, 0x30, 0x31, 0x32, 0x30, 0x39, 0x31, 0x33,
		0x00,

		0x04, 0x04,0x04,
		0x01,0x00,0x00,0x00,
		0x04, 'V', 'h', '1', '4',
		0x04, 'V', 'b', '1', '4',
		0x04, 'V', 's', '1', '4',
		0x04, 'V', 'f', '0', '4',
		0x03, 'G', 'W', 'D',
		0x04, 'S', 'N', '8', '4',
		0x08, 0x32, 0x30, 0x31, 0x32, 0x30, 0x39, 0x31, 0x33,
		0x00,

		0x05, 0x08,0x04,
		0x01,0x01,0x00,0x00,
		0x04, 'V', 'h', '1', '5',
		0x04, 'V', 'b', '1', '5',
		0x04, 'V', 's', '1', '5',
		0x04, 'V', 'f', '0', '5',
		0x03, 'G', 'W', 'D',
		0x04, 'S', 'N', '8', '5',
		0x08, 0x32, 0x30, 0x31, 0x32, 0x30, 0x39, 0x31, 0x33,
		0x02, 
		192, 168, 2, 126,
		0x06, 'p','u','b','l','i','c',
		0x06, 'p','u','b','l','i','c',
		0x00, 0x00 };

	VOS_MemCpy( pRx, oam_msg, EUQ_MAX_OAM_PDU );
	
	return VOS_OK;
}

VOID testHotExtOnuAlm( ULONG slotno, ULONG portno, ULONG onuid, ULONG flag, ULONG state )
{
	short int PonPortIdx;
	UCHAR oam_msg[EUQ_MAX_OAM_PDU+4] = {
		0x03, 	/*alarm type */
		0x01,	/*alarm flag:1-insert, 2-pull, 4-change*/
		0x01,	/*slot*/
		0x02, 	/*board type:1-GT-EPON-B;2-GT-8POTS-A(RJ11);3-GT-6FE;4-GT-8FE;5-GT-16FE;6-GT-8FXS-A(RJ11);7-GT-8POTS-B(RJ21);8-GT-8FXS-B(RJ21)*/
		0x04,	/*operator status*/
		0x01,0x00,0x00,0x00,
		0x04, 'v', 'H', '1', '1',
		0x04, 'v', 'B', '1', '1',
		0x04, 'v', 'S', '1', '1',
		0x04, 'v', 'F', '0', '1',
		0x03, 'g', 'W', 'D',
		0x04, 's', 'N', '8', '1',
		0x08, 0x32, 0x30, 0x31, 0x32, 0x30, 0x39, 0x31, 0x33,
		0x00, 0x00 };
	oam_msg[1] = flag;
	oam_msg[4] = state;

	PonPortIdx=GetPonPortIdxBySlot(slotno, portno);

	eventOamMsg_onuHotPlugging(PonPortIdx,onuid, (VOID*)oam_msg);
}

void test_size()
{
	sys_console_printf("\r\n size=%d, %d\r\n\r\n", sizeof(onu_ext_table_t), sizeof(onu_ext_brd_t));
}

#endif
#endif


