
/*#include "tdm_comm.h"*/
#include  "OltGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "gwEponSys.h"

#include "tdm_comm.h"

#include "e1\e1_apis.h"



typedef struct{
	SHORT		preidx;
	SHORT		nextidx;
	UCHAR		iused;

	USHORT		queid;
	USHORT		serialno;
	
	UCHAR resendTimeout;
	UCHAR callFlag;
	ULONG callNotify;
}__attribute__((packed)) tdm_comm_que_head_t;

typedef	struct{
	tdm_comm_que_head_t    head;
	tdm_comm_msg_t		sendMsg;
	USHORT		sendMsgLen;
	CHAR	*recvMsgPtr;
	USHORT	recvMsgLen;
}__attribute__((packed)) tdm_comm_que_item_t;

typedef struct{
	int type;
	int subType;
	FUNC_PRC_CALLBACK handler;
}RTN_HOOK_FUNC;

RTN_HOOK_FUNC  g_rtnPrcFuncs[] = {
	{0, 0, NULL },
	{TDM_MSG_TYPE_DEVMAN, MSG_SUBTYPE_LOADFPGA_DONE, NULL},
	{TDM_MSG_TYPE_TRACEDEB, MSG_SUBTYPE_DEBUG_INFO,NULL},
	{TDM_MSG_TYPE_EVENT, MSG_SUBTYPE_E1_ALM, NULL},
	{TDM_MSG_TYPE_EVENT, MSG_SUBTYPE_E1_ALMCLR, NULL},
	/* begin: added by jianght 20090205  */
	{TDM_MSG_TYPE_EVENT, E1_OOS, NULL},
	{TDM_MSG_TYPE_EVENT, E1_OOS_RECOV, NULL},
	/* end: added by jianght 20090205 */
	{TDM_MSG_TYPE_EVENT, MSG_SUBTYPE_VOICE_OOS, NULL},
	{TDM_MSG_TYPE_EVENT, MSG_SUBTYPE_VOICE_IS, NULL},
	{TDM_MSG_TYPE_EVENT, MSG_SUBTYPE_ALM_SIG_RESET, NULL}	/* added by xieshl 20080324 */
};

/**/
ULONG	tdmCommQueId=0;
ULONG	tdmCommTimerId=0;
ULONG	tdmCommSemId = 0;
VOS_HANDLE	tdmCommTaskId = 0;

tdm_msg_statistic_t	tdmCommStatis /*= { 0 }*/;
tdm_comm_que_item_t	tdmCommQueItems[MAX_TDM_QUEUE_LEN];

static SHORT queue_used_head_idx = -1;	/*初始化队列已使用表项的索引头*/
static SHORT queue_used_tail_idx = -1;

/*
定义调试信息打印标识
bit0:输出调试解释信息
bit1:输出接收到的板间通信报文
bit2:输出接收到的原始报文
bit3:
输出发送的板间通信报文
bit4:输出发送的原始报文
*/
UCHAR g_tdmDebugFlag = 0;
/*UCHAR g_tdmDebugLineWidth = 16;*/

static UCHAR tdm_comm_da[6] = "\x00\x01\x0b\x0f\x01\x07";
/*static UCHAR tdm_comm_sa[6] = "";*/

static ULONG tdm_send_serial_no[MAX_TDM_QUEUE_LEN] /*= {0}*/;

extern int ethSendToMII(void *, unsigned int );
/*extern void print_pdu( const char* pdu, USHORT len, UINT width );*/	/* removed by xieshl 20100805 */
extern ulong_t getActivatedSwBoard( void );
#if FUNC_DECLARE
#endif
static void tdmCommRtnHook( CHAR *pData, UINT size );
static void tdmCommProcTask( void );
static void tdmCommTimerCallback( void );
static SHORT getPreUsedQueItemSuffix( SHORT );
static SHORT getNextUsedQueItemSuffix( SHORT );
static void  setQueItemUsedStatus( short idx );
static void clearQueItemUsedStatus( tdm_comm_que_head_t *pItem);
static int sendTdmCommMsg( tdm_comm_que_item_t * pentry );
static int reSendTdmCommMsg( void );

#if FUNC_IMPLEMENT
#endif

FUNC_PRC_CALLBACK registRtnProcesser( int type, int subType, FUNC_PRC_CALLBACK handler )
{
	const int count =sizeof(g_rtnPrcFuncs)/sizeof(RTN_HOOK_FUNC);
	int i=0;
	for( ; i<count; i++ )
	{
		if( type == g_rtnPrcFuncs[i].type && subType == g_rtnPrcFuncs[i].subType )
		{
			g_rtnPrcFuncs[i].handler = handler;
			break;
		}
	}

	if( i != count )
		return g_rtnPrcFuncs[i].handler;
	else
		return NULL;
}

FUNC_PRC_CALLBACK getRtnProcesser( int type, int subType )
{
	const int count =sizeof(g_rtnPrcFuncs)/sizeof(RTN_HOOK_FUNC);
	int i=0;
	
	for( ; i<count; i++ )
	{
		if( type == g_rtnPrcFuncs[i].type && subType == g_rtnPrcFuncs[i].subType )
		{
			break;
		}
	}

	if( i != count )
		return g_rtnPrcFuncs[i].handler;
	else
		return g_rtnPrcFuncs[0].handler;	
}

/*******************************************************************************************
函数名：	tdm_recv_frame_handler
功能：	TDM通信数据帧处理函数
输入：	待处理的数据帧，数据帧长度
返回值：	无
*******************************************************************************************/
void tdm_recv_frame_handler( const void *pFrame, UINT length )
{
	tdm_comm_msg_t *pbuf = (tdm_comm_msg_t*)pFrame;

	if( pbuf != NULL && length > 20 )
	{
		FUNC_PRC_CALLBACK prtn = getRtnProcesser( pbuf->pdu.type, pbuf->pdu.subType );
		if( prtn != NULL )
		{			   
			(*prtn)(pFrame, length);
		}
		else
		{
			VOS_Free( pFrame );
		}
	}
	
}

extern void  TdmMgmtLowLevelMsgRecv( unsigned char *pBuf, unsigned int length);
extern void  tdmEventMsgProg( unsigned char *pBuf, unsigned int length);

STATUS	initTdmComm( void )
{
	int rc = VOS_ERROR;
	static int instanceOnce = 0;

	VOS_MemZero( &tdmCommStatis, sizeof(tdmCommStatis) );
	
	if( instanceOnce == 0 )
	{
		int i = 0;
		VOS_MemZero(tdmCommQueItems, sizeof(tdmCommQueItems) );
		for( ; i<MAX_TDM_QUEUE_LEN; i++ )
		{
			tdmCommQueItems[i].head.queid = i;
			tdm_send_serial_no[i] = 0;
		}
		
		tdmCommSemId = VOS_SemMCreate( VOS_SEM_Q_FIFO );
		
		if( tdmCommSemId != 0 )
		{
			tdmCommQueId = VOS_QueCreate( MAX_TDM_QUEUE_LEN, VOS_MSG_Q_FIFO );
			if( tdmCommQueId != 0 )
			{
				tdmCommTaskId = (VOS_HANDLE)VOS_TaskCreate( TASK_TDM_COMM_NAME, TASK_TDM_COMM_PRI, (VOID*)tdmCommProcTask, NULL );
				if( tdmCommTaskId != NULL )
				{
					VOS_QueBindTask( tdmCommTaskId, tdmCommQueId );
					if( VOS_TimerCreate( MODULE_TDM_COMM, 0, 1500, (void*)tdmCommTimerCallback, NULL, VOS_TIMER_LOOP ) != VOS_ERROR )
						rc = VOS_OK;
					else
					{
						/*reports( "initTdmComm:", "create timer error!" );*/
						VOS_TaskTerminate( tdmCommTaskId, 0 );
						VOS_QueDelete( tdmCommQueId );
						VOS_SemDelete( tdmCommSemId );
					}
				}
				else
				{
					VOS_QueDelete( tdmCommQueId );
					VOS_SemDelete( tdmCommSemId );
				}
			}
			else
				VOS_SemDelete( tdmCommSemId );
		}

		if( rc == VOS_OK )
		{
			registRtnProcesser(TDM_MSG_TYPE_DEVMAN, MSG_SUBTYPE_LOADFPGA_DONE, (FUNC_PRC_CALLBACK)TdmMgmtLowLevelMsgRecv);
			registRtnProcesser(TDM_MSG_TYPE_EVENT, MSG_SUBTYPE_E1_ALM, (FUNC_PRC_CALLBACK)tdmEventMsgProg);
			registRtnProcesser(TDM_MSG_TYPE_EVENT, MSG_SUBTYPE_E1_ALMCLR, (FUNC_PRC_CALLBACK)tdmEventMsgProg);
			registRtnProcesser(TDM_MSG_TYPE_EVENT, MSG_SUBTYPE_VOICE_OOS, (FUNC_PRC_CALLBACK)tdmEventMsgProg);
			registRtnProcesser(TDM_MSG_TYPE_EVENT, MSG_SUBTYPE_VOICE_IS, (FUNC_PRC_CALLBACK)tdmEventMsgProg);
			/* begin: added by jianght 20090205  */
			registRtnProcesser(TDM_MSG_TYPE_EVENT, E1_OOS, (FUNC_PRC_CALLBACK)tdmEventMsgProg);
			registRtnProcesser(TDM_MSG_TYPE_EVENT, E1_OOS_RECOV, (FUNC_PRC_CALLBACK)tdmEventMsgProg);
			/* end: added by jianght 20090205 */

			registRtnProcesser(TDM_MSG_TYPE_EVENT, MSG_SUBTYPE_ALM_SIG_RESET, (FUNC_PRC_CALLBACK)tdmEventMsgProg);	/* added by xieshl 20080324 */
			
			registRtnProcesser( 0,0, (FUNC_PRC_CALLBACK)tdmCommRtnHook );

			instanceOnce = 1;
		}

	}

	return rc;
}

VOID tdmCommProcTask()
{
	while( 1 )
	{
		ULONG ulmsg[4]={0};
		if( VOS_QueReceive( tdmCommQueId, ulmsg , WAIT_FOREVER ) != VOS_ERROR )
		{
			tdm_comm_que_item_t *pItem = (tdm_comm_que_item_t *)ulmsg[3];
			if( ulmsg[1] == TDM_MSG_SEND_CODE && pItem!=NULL )
			{
				if( g_tdmDebugFlag & TDM_DEBUG_INFO )
				{
					reports( "tdmCommProcTask:", "" );
					reportd( "send queue item NO.", pItem->head.queid );
					reportd( "send serial NO.", pItem->head.serialno );
				}

				sendTdmCommMsg( pItem );
			}
			else if( ulmsg[1] == TDM_MSG_TIMER_CODE )
			{
				reSendTdmCommMsg();
			}
			else if( ulmsg[1] == TDM_MSG_RECV_CODE  )
			{
			}
			else if( ulmsg[1] == TDM_MSG_RECV_TIMEOUT )
			{
				if( g_tdmDebugFlag & TDM_DEBUG_INFO )
					reports( "tdmCommProcTask", "TDM_MSG_RECV_TIMEOUT" );
				clearQueItemUsedStatus( &(pItem->head) );
			}
			else
			{
				if( g_tdmDebugFlag & TDM_DEBUG_INFO )
					reportd( "tdmCommProcTask: received unknown msg act code, code = %d", ulmsg[1] );
			}

		}
		else
		{
			if( g_tdmDebugFlag & TDM_DEBUG_INFO )
				reports( "tdmCommProcTask:", "error in receive TDM msg!" );
		}
/*		VOS_TaskDelay( 50 );*/
	}
}

void tdmCommRtnHook( CHAR *pData, UINT size )
{
	const tdm_comm_msg_t * pmsg = (const tdm_comm_msg_t *)pData;
	int ret = VOS_ERROR;
	
	if( pData == NULL ) /* added by xieshl 20090629 */
	{
		VOS_ASSERT(0);
		return;
	}
	if( size == 0 ) 
	{
		VOS_ASSERT(0);
		return;
	}
	if( queue_used_head_idx >= 0 && queue_used_head_idx < MAX_TDM_QUEUE_LEN )
	{
		tdm_comm_que_item_t * pQueEntry = tdmCommQueItems+queue_used_head_idx;
		tdm_comm_que_head_t *phead = (tdm_comm_que_head_t *)&pQueEntry->head;

		USHORT sessionid = pmsg->pdu.reserved>>16;
		USHORT msgid = pmsg->pdu.reserved&0xffff;

		VOS_SemTake( tdmCommSemId, WAIT_FOREVER );
#if 0		
		while( phead->iused )
		{
			if( phead->queid == sessionid && phead->serialno == msgid )
				break;

			if( phead->nextidx == -1 )
			{
				pQueEntry = NULL;
				phead = NULL;
				break;
			}
			else
			{
				pQueEntry = tdmCommQueItems+phead->nextidx;
				phead = (tdm_comm_que_head_t*)&pQueEntry->head;
			}
		}
#endif

		if( sessionid < MAX_TDM_QUEUE_LEN )
		{
			/*VOS_SemTake( tdmCommSemId, WAIT_FOREVER );*/
			
			pQueEntry = tdmCommQueItems+sessionid;
			phead = (tdm_comm_que_head_t *)&pQueEntry->head;

			if( (phead->queid == sessionid) && (phead->serialno == msgid) && (pQueEntry->recvMsgPtr == NULL) )
			{
				pQueEntry->recvMsgPtr = pData;
				pQueEntry->recvMsgLen = size;
				
				if( phead->callFlag == ENUM_CALLTYPE_SYN )
				{
					if( phead->callNotify != 0 )
					{
						if( g_tdmDebugFlag & TDM_DEBUG_INFO )
							sys_console_printf( "\r\nrelease io ctrl notifier %d", phead->callNotify );

						ret = VOS_SemGive( phead->callNotify );
					}
				}
				else if( phead->callFlag == ENUM_CALLTYPE_FUNC )
				{
					FUNC_PRC_CALLBACK pFunc = (FUNC_PRC_CALLBACK)phead->callNotify;
					if( pFunc )
					{
						(*pFunc)( pData, size );
						ret = VOS_OK;
					}
					clearQueItemUsedStatus( phead );
				}
				else if( phead->callFlag == ENUM_CALLTYPE_MSG )
				{
					ULONG ulmsg[4] = { MODULE_TDM_COMM, TDM_MSG_RECV_CODE, 0, 0 };
					ulmsg[2] = size;
					ulmsg[3] = (ULONG)pData;
					if( (ret = VOS_QueSend( phead->callNotify, ulmsg, WAIT_FOREVER, MSG_PRI_NORMAL)) == VOS_ERROR )
					{
						if( g_tdmDebugFlag & TDM_DEBUG_INFO )
							reports( "tdmCommRtnHook:", "send msg to process queue error" );
					}
				}
			}
		}

		VOS_SemGive( tdmCommSemId );
	}

	if( ret == VOS_ERROR )	/* modified by xieshl 20090629 */
		VOS_Free( pData );
	
}

void * tdmCommMsgAlloc( void )
{
	short i=0;

	void * retp = NULL;

	if( tdmCommSemId == 0 )
		return retp;

	if( get_gfa_tdm_slotno() == 0 )
		return retp;

	VOS_SemTake( tdmCommSemId, WAIT_FOREVER );
	
	for( i=0; i<MAX_TDM_QUEUE_LEN; i++ )
	{
		tdm_comm_que_item_t *pItem = tdmCommQueItems+i;
		tdm_comm_que_head_t *phead = (tdm_comm_que_head_t*)&pItem->head;
		
		if( !phead->iused )
		{
			ULONG reserved = 0;
			setQueItemUsedStatus( i );

			/*
			added by wangxy 2007-06-21 
			increase send serial no
			*/

			phead->queid = i;
			phead->serialno = tdm_send_serial_no[i]++;

			/*if( pItem->recvMsgPtr )
				VOS_Free(pItem->recvMsgPtr);*/
			pItem->recvMsgPtr = NULL;
			pItem->recvMsgLen = 0;

			pItem->sendMsgLen = 0;
			retp = &pItem->sendMsg.pdu;

			reserved = phead->queid;
			reserved = (reserved<<16)|phead->serialno;

			pItem->sendMsg.pdu.reserved = reserved;
			
			break;
		}
	}
	if( i == MAX_TDM_QUEUE_LEN )
	{
		tdmCommStatis.ulQueFullCount++;
	}

	VOS_SemGive( tdmCommSemId );
	
	return retp;
}

void tdmCommMsgFree( char* pdu )
{
	if( queue_used_head_idx >= 0 && queue_used_head_idx < MAX_TDM_QUEUE_LEN )
	{
		tdm_comm_que_item_t * pQueEntry = NULL;
		tdm_comm_que_head_t *phead = NULL;

		VOS_SemTake( tdmCommSemId, WAIT_FOREVER );

		pQueEntry =  tdmCommQueItems+queue_used_head_idx;
		phead = (tdm_comm_que_head_t *)&pQueEntry->head;
		
		while( phead->iused )
		{
			if( pQueEntry->recvMsgPtr == pdu )
				break;
			
			if( phead->nextidx == -1 )
			{
				pQueEntry = NULL;
				phead = NULL;
				break;
			}
			else
			{
				pQueEntry = tdmCommQueItems+phead->nextidx;
				phead = (tdm_comm_que_head_t*)&pQueEntry->head;
			}
		}

		if( pQueEntry == NULL ||phead == NULL )
		{
			VOS_Free( pdu );
			if(g_tdmDebugFlag & TDM_DEBUG_INFO)
				sys_console_printf( "\r\nERROR:not found queue item!" );
			VOS_SemGive( tdmCommSemId );
			return;
		}

		if( pQueEntry->recvMsgPtr != 0 )
		{
			VOS_Free( pQueEntry->recvMsgPtr );
			pQueEntry->recvMsgPtr = NULL;
			pQueEntry->recvMsgLen = 0;
		}

		clearQueItemUsedStatus( (tdm_comm_que_head_t *)&pQueEntry->head );	
		
		VOS_SemGive( tdmCommSemId );
	}
	
}

void tdmCommTimerCallback(  )
{
	ULONG ulMsg[4] = { MODULE_TDM_COMM, TDM_MSG_TIMER_CODE, 0, 0 };
	ULONG quenum = VOS_QueNum( tdmCommQueId );
	if(  quenum < ( MAX_TDM_QUEUE_LEN/2 ) )
	{
		if( VOS_QueSend( tdmCommQueId, ulMsg, NO_WAIT/*WAIT_FOREVER*/, MSG_PRI_NORMAL ) == VOS_ERROR )
		{
			if( g_tdmDebugFlag & TDM_DEBUG_INFO )
				reports( "tdmCommTimerCallback:", "send timer Msg err!" );
		}
	}
	else
	{
		if( g_tdmDebugFlag & TDM_DEBUG_INFO )
		{
			reports( "tdmCommTimerCallback:", "queue busy, exceed half load!pause" );
			reportd( "current queue item num:", quenum );
		}
	}
}


SHORT getPreUsedQueItemSuffix( SHORT curIdx )
{
	
	SHORT	ret = -1;

	if( curIdx>0 && curIdx < MAX_TDM_QUEUE_LEN )
	{
		short i=0;
		for( i=curIdx-1; i>=0; i-- )
		{
			tdm_comm_que_head_t *pItem = (tdm_comm_que_head_t *)&(tdmCommQueItems+i)->head;
			if( pItem->iused )
			{
				ret = i;
				break;
			}
		}
	}	
	
	return ret;
}

SHORT getNextUsedQueItemSuffix( SHORT curIdx )
{
	short ret = -1;

	if( curIdx >= 0 && curIdx < MAX_TDM_QUEUE_LEN )
	{
		short i=0;
		for( i=curIdx+1; i<MAX_TDM_QUEUE_LEN; i++ )
		{
			tdm_comm_que_head_t * pItem = (tdm_comm_que_head_t *)&(tdmCommQueItems+i)->head;
			if( pItem->iused )
			{
				ret = i;
				break;
			}			
		}
	}
	
	return ret;
}

void setQueItemUsedStatus( short idx )
{
	
	if( idx >=0 && idx < MAX_TDM_QUEUE_LEN )
	{
		
		tdm_comm_que_head_t *pItem = (tdm_comm_que_head_t *)&tdmCommQueItems[idx].head;

		pItem->iused = 1;

		pItem->preidx = getPreUsedQueItemSuffix( idx );
		pItem->nextidx = getNextUsedQueItemSuffix( idx );

		if( pItem->preidx == -1 )
			queue_used_head_idx = idx;
		else
			tdmCommQueItems[pItem->preidx].head.nextidx = idx;
		
		if( pItem->nextidx == -1 )
			queue_used_tail_idx = idx;
		else
			tdmCommQueItems[pItem->nextidx].head.preidx = idx;

		pItem->resendTimeout = MAX_COMM_TRY_NUM;
		tdmCommStatis.ulQueUsedCount++;
	}
}

/*added by wangxy 2007-11-08
   修改指定发送队列的重发次数
*/
void setQueItemResendTime( const short int idx, const UCHAR newNum )
{
	if( idx >= 0 && idx < MAX_TDM_QUEUE_LEN )
	{
		tdm_comm_que_head_t *pHead = ( tdm_comm_que_head_t*)&tdmCommQueItems[idx].head;
		pHead->resendTimeout = newNum;
	}
}

void clearQueItemUsedStatus( tdm_comm_que_head_t *pItem)
{
	if( pItem != NULL )
	{
		VOS_SemTake(tdmCommSemId, 1000);

		pItem->iused = 0;
		if( pItem->preidx != -1 && pItem->nextidx != -1 )
		{
			tdmCommQueItems[pItem->preidx].head.nextidx = pItem->nextidx;
			tdmCommQueItems[pItem->nextidx].head.preidx = pItem->preidx;
		}	
		else if( pItem->preidx==-1 && pItem->nextidx != -1 )
		{
			tdmCommQueItems[pItem->nextidx].head.preidx = -1;
			queue_used_head_idx = pItem->nextidx;
		}
		else if( pItem->preidx != -1 && pItem->nextidx == -1 )
		{
			tdmCommQueItems[pItem->preidx].head.nextidx = -1;
			queue_used_tail_idx = pItem->preidx;
		}
		else
		{
			queue_used_head_idx = -1;
			queue_used_tail_idx = -1;
		}		

		if( pItem->callFlag == ENUM_CALLTYPE_SYN && pItem->callNotify != 0 )
		{
#if 0			
			reportd( "clearQueItemUsedStatus:    delete callNotify:", pItem->callNotify );
#endif
			VOS_SemDelete( pItem->callNotify );
		}		
		pItem->callNotify = 0;

		pItem->callFlag = 0;

		if( tdmCommStatis.ulQueUsedCount > 0 )
			tdmCommStatis.ulQueUsedCount--;

		VOS_SemGive(tdmCommSemId);
	}
}
#if 0
 void tdmQueueFlush( void )
{
	  if( queue_used_head_idx >= 0 && queue_used_head_idx < MAX_TDM_QUEUE_LEN )
	  {
	     tdm_comm_que_item_t *pQueEntry = (tdm_comm_que_item_t*)&tdmCommQueItems[queue_used_head_idx];
	     tdm_comm_que_head_t *phead = (tdm_comm_que_head_t*)&pQueEntry->head;

	     VOS_SemTake(tdmCommSemId, WAIT_FOREVER);

	     while( phead->iused )
	     {
	         clearQueItemUsedStatus(phead);

	         pQueEntry = tdmCommQueItems+phead->nextidx;
	         phead = (tdm_comm_que_head_t*)&pQueEntry->head;
	     }

	     VOS_SemGive(tdmCommSemId);
	  }
}
#endif
/*modified by shixh@20080415*/
void tdmQueueFlush( void )
{
	int i;
	/*if( queue_used_head_idx >= 0 && queue_used_head_idx < MAX_TDM_QUEUE_LEN )*/

	VOS_SemTake(tdmCommSemId, 1000);
	for(i=0;i<MAX_TDM_QUEUE_LEN;i++)	
	{

		tdm_comm_que_item_t *pQueEntry = (tdm_comm_que_item_t*)&tdmCommQueItems[i];
		tdm_comm_que_head_t *phead = (tdm_comm_que_head_t*)&pQueEntry->head;

		if( phead->callFlag == ENUM_CALLTYPE_SYN && phead->callNotify != 0 )
		{
#if 0			
			reportd( "clearQueItemUsedStatus:    delete callNotify:", pItem->callNotify );
#endif
			VOS_SemDelete( phead->callNotify);
		}		
		phead->callNotify = 0;
		phead->callFlag = 0;
			
	       VOS_MemZero(phead, sizeof(tdm_comm_que_head_t));
	}
	queue_used_head_idx=0;

	VOS_SemGive(tdmCommSemId);
}

static void makeSrcMacAddr( char *mac )
{
	memcpy( mac, "\x00\x05\x3b\xff\x01", 5 );
	mac[5] = getActivatedSwBoard();
}

static void makeDrcMacAddr( char *mac )
{
	memcpy( mac, tdm_comm_da, 6 );
	mac[5] = get_gfa_tdm_slotno();
}

/*return code */
int tdmCommSendMsg( UCHAR callflag, ULONG callNotify, char* pSendData, USHORT sendDataLen, char** ppRecvMsgPtr, USHORT *pRecvMsgLen )
{
	int ret = 0;
	USHORT msgLen = 0;
	ULONG ulMsg[4]={ MODULE_TDM_COMM, TDM_MSG_SEND_CODE, 0, 0};

	if( pSendData != NULL && sendDataLen != 0 )
	{
		tdm_comm_que_item_t *pQueitem = ( tdm_comm_que_item_t *)( pSendData-(sizeof(tdm_comm_que_head_t)+PDU_OFFSET_IN_MSG));
		msgLen = PDU_OFFSET_IN_MSG;

		if( g_tdmDebugFlag & TDM_DEBUG_SEND_MSG )
		{
			tdm_pdu_t *pdu = (tdm_pdu_t*)(&pQueitem->sendMsg.pdu);
			if(!( pdu->type == TDM_MSG_TYPE_DEVMAN
							&& pdu->subType == MSG_SUBTYPE_QUERY ))
			/*print_pdu( pSendData, sendDataLen, g_tdmDebugLineWidth );*/
			pktDataPrintf( pSendData, sendDataLen );	/* modified by xieshl 20100805 */
		}
		
		if( pQueitem != NULL )
		{
			tdm_comm_msg_t *pMsg = (tdm_comm_msg_t*)&pQueitem->sendMsg;
			tdm_comm_que_head_t * phead = (tdm_comm_que_head_t * )&pQueitem->head;

			VOS_SemTake( tdmCommSemId, WAIT_FOREVER );
			
			if( sendDataLen < MIN_PDU_LEN )
			{
				VOS_MemZero( pSendData+sendDataLen, MIN_PDU_LEN-sendDataLen );
				msgLen += MIN_PDU_LEN;
			}
			else
				msgLen += sendDataLen;
			
			pQueitem->sendMsgLen = msgLen;

			makeDrcMacAddr( pMsg->DA );

			makeSrcMacAddr( pMsg->SA );

			pMsg->type = 0x800;

			phead->callFlag = callflag;

			if( callflag == ENUM_CALLTYPE_SYN )
			{
				phead->callNotify = VOS_SemBCreate( VOS_SEM_Q_FIFO, VOS_SEM_EMPTY );
				if( phead->callNotify == 0 )
				{
					VOS_SemGive( tdmCommSemId );
					VOS_ASSERT( 0 );
					return 1;
				}
				
			}
			else if( callflag == ENUM_CALLTYPE_FUNC ||callflag == ENUM_CALLTYPE_MSG )
				phead->callNotify = callNotify;
			else
				phead->callNotify = 0;

			VOS_SemGive( tdmCommSemId );

			ulMsg[3] = (ULONG)pQueitem;
			if( VOS_QueSend( tdmCommQueId, ulMsg, NO_WAIT/*WAIT_FOREVER*/, MSG_PRI_NORMAL ) == VOS_OK )
			{
#if 0				
				reports( "tdmCommSendMsg:", "msg have sent compelete!" );
#endif
				tdmCommStatis.ulQueSendTotalCount++;
			}
			else
			{
				tdmCommStatis.lQueSendErrCount++;
				VOS_ASSERT(0);
				return 1;
			}

			if( callflag == ENUM_CALLTYPE_SYN )
			{
				if( VOS_SemTake( phead->callNotify, 15*VOS_TICK_SECOND ) == VOS_OK )
				{
#if 0					
					reports( "tdmCommSendMsg:", "recv success!" );
#endif
					*ppRecvMsgPtr = pQueitem->recvMsgPtr/*+(sizeof(tdm_comm_que_head_t)+PDU_OFFSET_IN_MSG)*/;
					*pRecvMsgLen = pQueitem->recvMsgLen/*-(sizeof(tdm_comm_que_head_t)+PDU_OFFSET_IN_MSG)*/;
					
					if( g_tdmDebugFlag & TDM_DEBUG_RECV_MSG )
					{
						tdm_pdu_t *pdu = (tdm_pdu_t*)(pQueitem->recvMsgPtr+PDU_OFFSET_IN_MSG);
						if(!( pdu->type == TDM_MSG_TYPE_DEVMAN
							&& pdu->subType == MSG_SUBTYPE_QUERY ))
						/*print_pdu( (*ppRecvMsgPtr)+PDU_OFFSET_IN_MSG, (*pRecvMsgLen)-PDU_OFFSET_IN_MSG, g_tdmDebugLineWidth );*/
						pktDataPrintf( (*ppRecvMsgPtr)+PDU_OFFSET_IN_MSG, (*pRecvMsgLen)-PDU_OFFSET_IN_MSG );	/* modified by xieshl 20100805 */

					}
					if( g_tdmDebugFlag & TDM_DEBUG_RECV_RAW )
					{
						tdm_pdu_t *pdu = (tdm_pdu_t*)(pQueitem->recvMsgPtr+PDU_OFFSET_IN_MSG);
						if(!( pdu->type == TDM_MSG_TYPE_DEVMAN
							&& pdu->subType == MSG_SUBTYPE_QUERY ))
						/*print_pdu( pQueitem->recvMsgPtr, pQueitem->recvMsgLen, g_tdmDebugLineWidth );*/
						pktDataPrintf( pQueitem->recvMsgPtr, pQueitem->recvMsgLen );	/* modified by xieshl 20100805 */
					}

				}
				else
				{
					ret = 3;
					if( g_tdmDebugFlag & TDM_DEBUG_INFO )
						reports( "tdmCommSendMsg:", "recv timeout!" );
					tdmCommStatis.ulQueRecvTimeoutCount++;
				}
			}
		}
		else
			ret = 2;
		
	}
	else
		ret =1;

	return ret;
}

static int sendTdmCommMsg( tdm_comm_que_item_t * pentry )
{
	int err=0;
	if( pentry != NULL )
	{
		tdm_comm_que_head_t *phead = (tdm_comm_que_head_t*)&pentry->head;
				
		if( phead->resendTimeout > 0 )
		{
			int i=0;
			for(; i<3; i++)
			{
				if( ethSendToMII( &pentry->sendMsg, pentry->sendMsgLen ) == VOS_ERROR )
				{
					err = 3;
				}
				else
				{
					err = 0;
					if( g_tdmDebugFlag & TDM_DEBUG_SEND_RAW )
					{
						tdm_pdu_t *pdu = (tdm_pdu_t*)(&pentry->sendMsg.pdu);
							if(!( pdu->type == TDM_MSG_TYPE_DEVMAN
								&& pdu->subType == MSG_SUBTYPE_QUERY ))
						/*print_pdu( (char*)&pentry->sendMsg, pentry->sendMsgLen, g_tdmDebugLineWidth );*/
						pktDataPrintf( (char*)&pentry->sendMsg, pentry->sendMsgLen );	/* modified by xieshl 20100805 */
					}
					if( g_tdmDebugFlag & TDM_DEBUG_INFO )
						reports( "sendTdmCommMsg:", "low level send msg OK!" );
					break;
				}
			}

			if( i == 3)
			{
				tdmCommStatis.lQueSendErrCount++;
			}
			/*			VOS_SemTake( tdmCommSemId, WAIT_FOREVER );*/
						phead->resendTimeout--;
			/*			VOS_SemGive( tdmCommSemId );*/
			
			/*begin: added by wangxiaoyu 2007-12-12*/
			if( pentry->head.callFlag == ENUM_CALLTYPE_NOACK )
				clearQueItemUsedStatus( &pentry->head );
			/*end*/
			
		}
		else
			err =2;
	}
	else
		err = 1;
	
	return err;
}

static int reSendTdmCommMsg( void )
{
	int err = 0;
	if( queue_used_head_idx != -1 )
	{
		if( queue_used_head_idx >= 0 && queue_used_head_idx < MAX_TDM_QUEUE_LEN )
		{

			tdm_comm_que_item_t *pQueEntry = (tdm_comm_que_item_t*)&tdmCommQueItems[queue_used_head_idx];
			tdm_comm_que_head_t *phead = (tdm_comm_que_head_t*)&pQueEntry->head;

			while( phead->iused )
			{
				if( phead->resendTimeout > 0 && phead->callFlag != ENUM_CALLTYPE_NOACK )
				{
					if( phead->resendTimeout == MAX_COMM_TRY_NUM )
					{
						VOS_SemTake( tdmCommSemId, WAIT_FOREVER );
						phead->resendTimeout--;
						VOS_SemGive( tdmCommSemId );
					}
					else
						sendTdmCommMsg( pQueEntry );
				}

				/*add by wangxy 2007-10-24
				   已经超时，从发送队列中去掉
				*/
				if( phead->resendTimeout == 0 )
				{
					ULONG msgitem[4] = {  MODULE_TDM_COMM, TDM_MSG_RECV_TIMEOUT, 0, (ULONG)pQueEntry };
					if( VOS_QueSend( tdmCommQueId, msgitem, NO_WAIT/*WAIT_FOREVER*/, MSG_PRI_NORMAL ) == VOS_ERROR )
					{
						if( g_tdmDebugFlag & TDM_DEBUG_INFO )
							sys_console_printf( "\r\nsend TDM_MSG_RECV_TIMEOUT msg error!" );
					}
					tdmCommStatis.ulQueRecvTimeoutCount++;
				}

				pQueEntry = tdmCommQueItems+phead->nextidx;
				phead = (tdm_comm_que_head_t*)&pQueEntry->head;
			}
	
		}
		else
		{
			err = 2;
			if( g_tdmDebugFlag & TDM_DEBUG_INFO )
			{
				reports( "reSendTdmCommMsg:", "invalid queue used head index!" );
				reportd( "invalid queue used head index:",  queue_used_head_idx );
			}
		}
	}
	else
		err = 1;
	
	return err;
}

/*void setTdmDebugLineWidth( UCHAR width ){ g_tdmDebugLineWidth=width;}*/
void setTdmDebugFlag( UCHAR mask ){ g_tdmDebugFlag|=mask; }
void undoSetTdmDebugFlag( UCHAR mask ) { g_tdmDebugFlag &= ~mask; }

/*ULONG retrieveSendSerialNo(void){return tdm_send_serial_no;};*/

/*void testSendTdmComMsg( void )
{
	USHORT ssid=0, sn=0;
	char* p = tdmCommMsgAlloc();
	char *pRecv = NULL;
	USHORT recvlen = 0;
	
	if( p != NULL )
	{
		tdm_pdu_t *ppdu = (tdm_pdu_t*)p;
		char * pdata = ppdu->msgData;

		ULONG t = ssid;
		t = (t<<16)+sn;
		ppdu->reserved = t;

		ppdu->type = TDM_MSG_TYPE_TDMCONF;
		ppdu->subType = 0x01;
		
		VOS_MemSet( pdata, 1, 6 );		
		
		if( tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)ppdu, 13, &pRecv, &recvlen ) != 0 )
		{
			if( g_tdmDebugFlag & TDM_DEBUG_INFO )
				reports( "testSendTdmComMsg:", "send tdm msg error!" );
		}

		if( pRecv != NULL )
		tdmCommMsgFree( p );
	}
}*/

void showTdmQueue( struct vty *vty, int  range )
{
	#ifdef	reportd
	#undef reportd
	#define    reportd(a,b)    vty_out(vty, "\r\n%-40s%ld", a, b )
	#endif

	#ifdef reports
	#undef reports
	#define    reports(a,b)    vty_out(vty, "\r\n%-40s%s", a, b )
			
	#endif
	int i=0;

	if( range > MAX_TDM_QUEUE_LEN )
		return ;
	
	for( ; i<range; i++ )
	{
		tdm_comm_que_item_t *pItem = tdmCommQueItems+i;
		tdm_comm_que_head_t * phead = &pItem->head;

		reports( "","" );

		reportd( "queue index :", phead->queid );
		reportd( "send serial no :", phead->serialno );
		reportd( "used status :", (int)phead->iused );
		reportd( "preidx :", phead->preidx );
		reportd( "nextidx :", phead->nextidx );
		
		vty_out(vty, "\r\n%-40s%p", "send buf  :", &pItem->sendMsg );
		reportd( "send buf length :", pItem->sendMsgLen );
		vty_out(vty, "\r\n%-40s%p", "recv buf :", pItem->recvMsgPtr );
		reportd( "recv buf length :", pItem->recvMsgLen );
		
		reports( "","\r\n" );
	}
	
}

#endif
