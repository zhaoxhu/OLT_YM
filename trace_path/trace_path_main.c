#ifdef __cplusplus
extern "C"{
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "gwEponSys.h"
#include  "Cdp_pub.h"
#pragma pack(1)
#include  "trace_path_lib.h"
#include  "trace_path_main.h"


ULONG  tracePathTrapRate = TRACE_PATH_TRAP_RATE_DEFAULT;
ULONG  tracePathResolveRate = TRACE_PATH_RESOLVE_RATE_DEFAULT;
ULONG  tracePathAutoSyncCount = 0;
ULONG  tracePathAutoSyncMaxCircle = TRACE_PATH_AUTO_SYNC_TIME;
ULONG  tracePathAutoSyncEnable = 0;

int trace_path_debug = 0;

extern ULONG traceOamBSemId;
extern LONG tracePathOamFlag;
extern user_loc_t userLocationRecvBuf;

ULONG tracePathQId = 0;
VOS_HANDLE tracePathTId = NULL;
LONG  tracePathTimerId =0;

extern LONG local_mac_entry_learning_check( ULONG callNotify, UCHAR *pMac, ULONG *pulSlot, ULONG *pulPort );
extern LONG getOnuFromPonFdb( ULONG slot, ULONG port, UCHAR *pMacAddr, ULONG *pOnuDevIdx, ULONG *pFdbStatus, ULONG *pL2Idx );

extern LONG devsm_sys_is_switchhovering();
extern LONG trace_path_tbl_init( );
extern LONG trace_path_cli_cmd_install();
extern int userTraceUpdate_EventReport( ULONG hashIdx, UCHAR *pMacAddr );
extern LONG trace_path_req_oam_broadcast( ULONG slotno, ULONG portno, ULONG hash_idx, UCHAR *pUserMac );

LONG trace_path_timer_proc();

LONG trace_path_auto_sync_proc();
int trace_path_insert_mac_perpon(short int PonPortIdx, unsigned char* mac);
int trace_path_clear_mac_perpon(short int PonPortIdx);
int trace_path_mac_resolving_perpon(short int PonPortIdx);

LONG trace_path_oam_req_cdp_send( UCHAR *pUserMac, UCHAR slotno, UCHAR portno )
{
	LONG rc = VOS_ERROR;
	ULONG to_slotno = slotno;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(trace_path_sync_msg_t);
	trace_path_sync_msg_t  *pSyncMsg = NULL;
	SYS_MSG_S * pstMsg = NULL;

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		return rc;
	}
	if( pUserMac == NULL )
	{
		return rc;
	}
	if( (to_slotno == 0) || (to_slotno == SYS_LOCAL_MODULE_SLOTNO) )
		return rc;

	if( !(SYS_MODULE_IS_READY(to_slotno) || devsm_sys_is_switchhovering()) )
		return rc;

	pstMsg = ( SYS_MSG_S * ) CDP_AllocMsg( msgLen, MODULE_RPU_TRACEPATH );              
	if(pstMsg == NULL)
	{
		return rc;
	}

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = MODULE_RPU_TRACEPATH;
	pstMsg->ulDstModuleID = MODULE_RPU_TRACEPATH;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = slotno;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (VOID *)pSyncMsg;
	pstMsg->usFrameLen = sizeof(trace_path_sync_msg_t);
	pstMsg->usMsgCode = TRACE_SNOOP_MSG_CODE_SYNC;
	
	pSyncMsg = (trace_path_sync_msg_t *)(pstMsg + 1);
	pSyncMsg->msgType = TRACE_PATH_MSG_TYPE_OAM_REQ;
	MAC_ADDR_CPY( pSyncMsg->userLocation.userMac, pUserMac );
	pSyncMsg->userLocation.resolvingStatus = USER_TRACE_RESOLVING_WAIT;
	pSyncMsg->userLocation.oltBrdIdx = slotno;
	pSyncMsg->userLocation.oltPortIdx = portno;

	trace_path_debug_out( "TRACE-PATH:send mac %s oam REQ to slot %d\r\n", trace_path_mac_addr_2_str(pUserMac), to_slotno );

	rc = CDP_Send( RPU_TID_CDP_USERTRACE, to_slotno,  RPU_TID_CDP_USERTRACE,  CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_RPU_TRACEPATH );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pstMsg );
		VOS_ASSERT(0);
	}

	return rc;
}

LONG trace_path_oam_rsp_cdp_send( trace_path_sync_msg_t  *pSyncMsg )
{
	LONG rc = VOS_ERROR;
	ULONG to_slotno = SYS_MASTER_ACTIVE_SLOTNO;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(trace_path_sync_msg_t);
	SYS_MSG_S * pstMsg = NULL;

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		return rc;
	}
	if( pSyncMsg == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}
	if( (pSyncMsg->userLocation.oltBrdIdx == 0) || (pSyncMsg->userLocation.oltBrdIdx == to_slotno) )
		return rc;

	if( !(SYS_MODULE_IS_READY(to_slotno) || devsm_sys_is_switchhovering()) )
		return rc;

	pstMsg = ( SYS_MSG_S * ) CDP_AllocMsg( msgLen, MODULE_RPU_TRACEPATH );              
	if(pstMsg == NULL)
	{
		return rc;
	}

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = MODULE_RPU_TRACEPATH;
	pstMsg->ulDstModuleID = MODULE_RPU_TRACEPATH;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = to_slotno;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (VOID *)(pstMsg + 1);
	pstMsg->usFrameLen = sizeof(trace_path_sync_msg_t);
	pstMsg->usMsgCode = TRACE_SNOOP_MSG_CODE_SYNC;
	
	VOS_MemCpy( (VOID *)(pstMsg + 1), pSyncMsg, sizeof(trace_path_sync_msg_t) );

	trace_path_debug_out( "TRACE-PATH:send mac %s RSP to slot %d\r\n", trace_path_mac_addr_2_str(pSyncMsg->userLocation.userMac), to_slotno );

	rc = CDP_Send( RPU_TID_CDP_USERTRACE, to_slotno,  RPU_TID_CDP_USERTRACE,  CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_RPU_TRACEPATH );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pstMsg );
		VOS_ASSERT(0);
	}

	return rc;
}

LONG trace_path_resolving_req_cdp_send( UCHAR *pUserId, UCHAR *pUserMac, UCHAR slotno, UCHAR portno )
{
	LONG rc = VOS_ERROR;
#if 0    
	ULONG to_slotno = slotno;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(trace_path_sync_msg_t);
	trace_path_sync_msg_t  *pSyncMsg = NULL;
	SYS_MSG_S * pstMsg = NULL;

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		return rc;
	}
	if( (pUserId == NULL) || (pUserMac == NULL) )
	{
		return rc;
	}
	if( (to_slotno == 0) || (to_slotno == SYS_LOCAL_MODULE_SLOTNO) )
		return rc;

	if( !(SYS_MODULE_IS_READY(to_slotno) || devsm_sys_is_switchhovering()) )
		return rc;

	pstMsg = ( SYS_MSG_S * ) CDP_AllocMsg( msgLen, MODULE_RPU_TRACEPATH );              
	if(pstMsg == NULL)
	{
		return rc;
	}

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = MODULE_RPU_TRACEPATH;
	pstMsg->ulDstModuleID = MODULE_RPU_TRACEPATH;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = slotno;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (VOID *)pSyncMsg;
	pstMsg->usFrameLen = sizeof(trace_path_sync_msg_t);
	pstMsg->usMsgCode = TRACE_SNOOP_MSG_CODE_SYNC;
	
	pSyncMsg = (trace_path_sync_msg_t *)(pstMsg + 1);
	pSyncMsg->msgType = TRACE_PATH_MSG_TYPE_RESOLVE_REQ;
	VOS_StrnCpy( pSyncMsg->userId, pUserId, USER_ID_MAXLEN );
	MAC_ADDR_CPY( pSyncMsg->userLocation.userMac, pUserMac );
	pSyncMsg->userLocation.resolvingStatus = USER_TRACE_RESOLVING_WAIT;
	/*VOS_MemZero( &(pSyncMsg->userLocation), sizeof(user_loc_t) );*/
	pSyncMsg->userLocation.oltBrdIdx = slotno;
	pSyncMsg->userLocation.oltPortIdx = portno;
	/*pSyncMsg->userLocation.locFlag = USER_TRACE_FLAG_DYNAMIC;*/

	trace_path_debug_out( "TRACE-PATH:send mac %s resolving REQ to slot %d\r\n", trace_path_mac_addr_2_str(pUserMac), to_slotno );

	rc = CDP_Send( RPU_TID_CDP_USERTRACE, to_slotno,  RPU_TID_CDP_USERTRACE,  CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_RPU_TRACEPATH );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pstMsg );
		VOS_ASSERT(0);
	}
#endif 
	return rc;   
}


LONG trace_path_resolving_local( UCHAR *pUserId, UCHAR *pUserMac, UCHAR slotno, UCHAR portno )
{
	LONG rc = VOS_ERROR;
#if 0    
	ULONG hash_idx;

	if( (pUserId == NULL) || (pUserMac == NULL) )
		return rc;
	
	/*if( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
		return VOS_OK;*/

	/*if( user_loc_is_ready(pUserId) )
		return VOS_OK;*/

	hash_idx = trace_userid_hash_idx( pUserId );
	rc = trace_path_req_oam_broadcast( slotno, portno, hash_idx, pUserMac );
#endif
	return rc;
}

LONG trace_path_userid_resolving( UCHAR *pUserId, UCHAR *pUserMac, UCHAR slotno, UCHAR portno )
{
	LONG rc;

	if( !(SYS_MODULE_IS_READY(slotno) || devsm_sys_is_switchhovering()) )
		return VOS_OK;

	if( SlotCardIsPonBoard(slotno) != ROK )
		return VOS_ERROR;

	if( /*SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER*/(slotno == SYS_LOCAL_MODULE_SLOTNO) || !SYS_MODULE_SLOT_ISHAVECPU(slotno) )
	{
		rc = trace_path_resolving_local( pUserId, pUserMac, slotno, portno );
		trace_path_debug_out("TRACE-PATH:tx user trace oam to pon%d/%d %s\r\n", slotno, portno, ((rc == VOS_ERROR) ? "ERR" : "OK") );

	}
	else
	{
		trace_path_resolving_req_cdp_send( pUserId, pUserMac, slotno, portno );
	}
	
	return VOS_OK;
}

#if 0
/* 返回值: VOS_ERROR，或者pdu 长度 */
static LONG user_trace_pdu_parase( short int PonPortIdx, UCHAR *pUserId, short int OnuIdx, user_trace_oam_pdu_t *pPdu )
{
	LONG rc = VOS_ERROR;
	int i, j=0;
	ULONG onuBrdIdx, len=0;

	user_loc_t userLoc;
	user_loc_t *pUserLoc = &userLoc;

	if( (pPdu->MsgType != ONU_USER_TRACE_RSP) || (pPdu->Result != 1) )
	{
		sys_console_printf( "TRACE-PATH:parase PDU MsgType=%d,result=%d ERR\r\n", pPdu->MsgType, pPdu->Result );
		return rc;
	}
	
	VOS_MemZero( pUserLoc, sizeof(user_loc_t) );
	pUserLoc->onuId = OnuIdx+1;
	pUserLoc->onuBrdIdx = ((pPdu->onuEthPort >> 8) & 0xff);
	pUserLoc->onuPortIdx = (pPdu->onuEthPort & 0xff);
	if( pPdu->swFlag != 0 )
	{
		VOS_MemCpy( pUserLoc->swMacAddr, pPdu->swMacAddr, USER_MACADDR_LEN );
		pUserLoc->swPortIdx = pPdu->swPortIdx;
	}
	pUserLoc->locFlag = USER_TRACE_FLAG_DYNAMIC;
		
	rc = user_loc_set(PonPortIdx, pUserId, pUserLoc);

	user_trace_debug_out( ("TRACE-PATH:parase onu %d/%d/%d user trace pdu %s\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip( PonPortIdx ), OnuIdx+1,
		(rc == VOS_ERROR ? "ERR" : "OK")) );

	return rc;
}
#endif
void trace_path_auto_sync_msg_send()
{
	ULONG aulMsg[4] = {RPU_TID_CDP_USERTRACE, 0, 0, 0};
	SYS_MSG_S * pstMsg = NULL;
	LONG queNum = VOS_QueNum( tracePathQId );

	if ( queNum > 10  || tracePathAutoSyncEnable == 0)
	{
		return;
	}
    
	pstMsg = ( SYS_MSG_S* ) VOS_Malloc( sizeof(SYS_MSG_S), RPU_TID_CDP_USERTRACE );
	if ( NULL == pstMsg )
	{
		return ;
	}
	VOS_MemZero( pstMsg, sizeof(SYS_MSG_S) );
	pstMsg->ulSrcModuleID = RPU_TID_CDP_USERTRACE;
	pstMsg->ulDstModuleID = RPU_TID_CDP_USERTRACE;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;/*目的slot*/
	pstMsg->ucMsgType = MSG_TIMER;
	pstMsg->usMsgCode = TRACE_PATH_MSG_CODE_AUTO_SYNC_TIMER;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = NULL;
	pstMsg->usFrameLen = 0;

	aulMsg[3] = (ULONG)pstMsg;
    
	if( VOS_QueSend(tracePathQId, aulMsg, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK )
	{
		VOS_Free( pstMsg );
	}
    tracePathAutoSyncCount = 0;/*手动执行同步操作后，周期内不需要在进行自动同步*/
    
}
void trace_path_timer_callback()
{
	ULONG aulMsg[4] = {RPU_TID_CDP_USERTRACE, 0, 0, 0};
	SYS_MSG_S * pstMsg = NULL;
	LONG queNum = VOS_QueNum( tracePathQId );

	if ( queNum > 10)
	{
		return;
	}

	pstMsg = ( SYS_MSG_S* ) VOS_Malloc( sizeof(SYS_MSG_S), RPU_TID_CDP_USERTRACE );
	if ( NULL == pstMsg )
	{
		return ;
	}
	VOS_MemZero( pstMsg, sizeof(SYS_MSG_S) );
	pstMsg->ulSrcModuleID = RPU_TID_CDP_USERTRACE;
	pstMsg->ulDstModuleID = RPU_TID_CDP_USERTRACE;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;/*目的slot*/
	pstMsg->ucMsgType = MSG_TIMER;
	pstMsg->usMsgCode = TRACE_PATH_MSG_CODE_TIMER;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = NULL;
	pstMsg->usFrameLen = 0;

	aulMsg[3] = (ULONG)pstMsg;
	
	if( VOS_QueSend(tracePathQId, aulMsg, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK )
	{
		VOS_Free( pstMsg );
	}
    if(tracePathAutoSyncCount < tracePathAutoSyncMaxCircle)
    {
        tracePathAutoSyncCount += 10;
    }
    else
    {
        trace_path_auto_sync_msg_send();        
    }
}

void trace_path_main_task()
{
	LONG result;
	ULONG ulRcvMsg[4];
	SYS_MSG_S *pstMsg;

	while( 1 )
	{
		result = VOS_QueReceive( tracePathQId, ulRcvMsg, WAIT_FOREVER );
		if( result == VOS_ERROR )
		{
			VOS_TaskDelay(100);
			continue;
		}
		pstMsg = (SYS_MSG_S *)ulRcvMsg[3];
		if( NULL == pstMsg )
		{
			VOS_ASSERT(pstMsg);
			VOS_TaskDelay(20);
			continue;
		}

		if ( CDP_NOTI_FLG_SEND_FINISH == ulRcvMsg[1] )
		{
			CDP_FreeMsg( pstMsg );
			continue;
		}

		switch( pstMsg->usMsgCode )
		{
			case TRACE_PATH_MSG_CODE_TIMER:
				trace_path_timer_proc();
				break;
		    case TRACE_PATH_MSG_CODE_AUTO_SYNC_TIMER:
                trace_path_auto_sync_proc();
                break;
			case AWMC_CLI_BASE:
				decode_command_msg_packet( pstMsg, VOS_MSG_TYPE_QUE );
				break;
			default:
				break;
		}

		if(SYS_MSG_SRC_SLOT(pstMsg) != (SYS_LOCAL_MODULE_SLOTNO))
		{
			CDP_FreeMsg( pstMsg );    /*CDP消息释放*/
		}
		else
		{
			VOS_Free(pstMsg);         /*本板消息释放(包括PDU消息)*/
		}
		pstMsg = NULL;
	}
}

#if 0
LONG trace_path_timer_proc()
{
	UCHAR user_id[USER_ID_MAXLEN];
	trace_userid_table_t locTbl;
	trace_userid_table_t *pUserTbl = &locTbl;
	ULONG hashIdx;
	int totalCount = 0;
	int updateCount = 0;
	int resolvingCount = 0;

	user_id[0] = 0;

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		while( trace_userid_tbl_next_get(user_id, pTbl) == VOS_OK )
		{
			if( pUserTbl->userLocList[pUserTbl->userCurIdx].resolvingStatus == USER_TRACE_RESOLVED_OK )
			{
				if( (updateCount < TRACE_PATH_TRAP_RATE_DEFAULT) && (pUserTbl->updateFlag == 1) )
				{
					hashIdx = trace_userid_hash_idx( pUserTbl->userId );
					userTraceUpdate_EventReport( hashIdx, pUserTbl->userLocList[pUserTbl->userCurIdx].userMac );
					trace_userid_update_over( hashIdx, pUserTbl->userLocList[pUserTbl->userCurIdx].userMac );
					updateCount++;
					trace_path_debug_out( "TRACE-PATH:report %s update trap\r\n", pUserTbl->userId );
				}
			}
			else if( pUserTbl->userLocList[pUserTbl->userCurIdx].resolvingStatus == USER_TRACE_RESOLVING_WAIT )
			{
				if( resolvingCount < TRACE_PATH_RESOLVE_RATE_DEFAULT )
				{
					if( trace_userid_tbl_aging(pUserTbl->userId) != VOS_OK )
					{
						trace_path_userid_resolving( pUserTbl->userId, pUserTbl->userLocList[pUserTbl->userCurIdx].userMac, 
							pUserTbl->userLocList[pUserTbl->userCurIdx].oltBrdIdx, pUserTbl->userLocList[pUserTbl->userCurIdx].oltPortIdx );
						resolvingCount++;
					}
				}
			}

			if( (updateCount >= TRACE_PATH_TRAP_RATE_DEFAULT) && (resolvingCount >= TRACE_PATH_RESOLVE_RATE_DEFAULT) )
				break;
			
			totalCount++;
			if( totalCount > USER_ID_SUPPORT_MAXNUM )
			{
				VOS_ASSERT(0);
				break;
			}
			VOS_StrCpy( user_id, pUserTbl->userId );
		}
	}
	else if( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
	{
	}
	else /*if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )*/
	{
		while( trace_userid_tbl_next_get(user_id, pUserTbl) == VOS_OK )
		{
			trace_userid_tbl_aging( pUserTbl->userId );

			totalCount++;
			if( totalCount > USER_ID_SUPPORT_MAXNUM )
			{
				VOS_ASSERT(0);
				break;
			}
			VOS_StrCpy( user_id, pUserTbl->userId );
		}
	}

	return VOS_OK;
}
#else
extern trace_userid_table_t *gpTraceUserIdHashTable[TRACE_USERID_HASH_BUCKET];
int trace_printf_flag = 0;
LONG trace_path_auto_sync_proc()
{
	ULONG hashIdx;
	trace_userid_table_t *pUserTbl;
	user_loc_t *pLoc;
    short int PonPortIdx = 0;
	user_trace_snoop_t snoopMsg;
    
	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
        for(PonPortIdx=0;PonPortIdx<MAXPON;PonPortIdx++)
        {
            if(PonPortIsWorking(PonPortIdx))
            {
                short int addr_num = 0;
        		short int slot = GetCardIdxByPonChip(PonPortIdx);
        		short int port = GetPonPortByPonChip(PonPortIdx);

                if(OLT_GetMacAddrTbl(PonPortIdx, &addr_num, Mac_addr_table) == VOS_OK)
                {
                    int loop_num = 0;
                    for(loop_num=0;loop_num<addr_num;loop_num++)
                    {
                    	UCHAR  userId[USER_ID_MAXLEN] = {0};
                        trace_userid_table_t *pUserTbl;
                        int insert_flag = 0;
                        if(Mac_addr_table[loop_num].type != 0)
                            continue;
                        if(GetOnuIdxByMacPerPon(PonPortIdx, Mac_addr_table[loop_num].mac_address) != RERROR)
                            continue;
                        
                        VOS_Sprintf(userId, "%02x%02x.%02x%02x.%02x%02x", Mac_addr_table[loop_num].mac_address[0], Mac_addr_table[loop_num].mac_address[1], Mac_addr_table[loop_num].mac_address[2], 
                        Mac_addr_table[loop_num].mac_address[3], Mac_addr_table[loop_num].mac_address[4], Mac_addr_table[loop_num].mac_address[5]);
                        pUserTbl = trace_userid_tbl_search(userId);                           
                        if(pUserTbl)
                        {
                            if(pUserTbl->userLocList[0].oltBrdIdx != slot || pUserTbl->userLocList[0].oltPortIdx != port)
                            {
                                insert_flag = 1;
                            }
                        }
                        else
                        { 
                            insert_flag = 1;
                        }
                        if(insert_flag)
                        {
                    		VOS_MemZero( &snoopMsg, sizeof(user_trace_snoop_t) );
                            VOS_StrCpy(snoopMsg.userId, userId);
                    		MAC_ADDR_CPY( snoopMsg.userMac, Mac_addr_table[loop_num].mac_address);
                    		snoopMsg.oltBrdIdx = slot;
                    		snoopMsg.oltPortIdx = port;
                    		trace_path_add_notify( &snoopMsg );                            
                        }
                    }
                }
            }
        }
	}
    return VOS_OK;
}
LONG trace_path_timer_proc()
{
	static int hashIdx_bak = 0;

    int pon_id = 0;
	trace_userid_table_t *pUserTbl;
	user_loc_t *pLoc;
	ULONG hashIdx;
	int totalCount = 0;
	int updateCount = 0;
	int resolvingCount = 0;
	int i;
	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
#if 0        
		for( hashIdx=hashIdx_bak; hashIdx<TRACE_USERID_HASH_BUCKET; hashIdx++ )
		{
			VOS_SemTake( tracePathMSemId, WAIT_FOREVER );

			pUserTbl = gpTraceUserIdHashTable[hashIdx];
			while( pUserTbl )
			{
#if 0
				/* 检查线路信息是否改变 */
				pLoc = &(pUserTbl->userLocList[pUserTbl->userCurIdx]);
				
				if( pLoc->resolvingStatus == USER_TRACE_RESOLVED_OK )
				{
					if( (updateCount < tracePathTrapRate) && (pUserTbl->updateFlag == 1) )
					{
						userTraceUpdate_EventReport( hashIdx, pLoc->userMac );
						/*trace_userid_update_over( hashIdx, pLoc->userMac );*/
						pUserTbl->updateFlag = 0;

						updateCount++;
						trace_path_debug_out( "TRACE-PATH:report %s %s update trap\r\n", pUserTbl->userId, trace_path_mac_addr_2_str(pLoc->userMac) );
					}
				}
#endif
				for( i=0; i<USER_MACADDR_NUM; i++ )
				{
					pLoc = &(pUserTbl->userLocList[i]);

					if( MAC_ADDR_IS_INVALID(pLoc->userMac) )
						continue;

					/* 检查线路信息是否改变 */
					if( pLoc->resolvingStatus == USER_TRACE_RESOLVED_OK )
					{
						if( (updateCount < tracePathTrapRate) && (pLoc->updateFlag == 1) )
						{
							userTraceUpdate_EventReport( hashIdx, pLoc->userMac );
							/*trace_userid_update_over( hashIdx, pLoc->userMac );*/
							pLoc->updateFlag = 0;

							updateCount++;
							trace_path_debug_out( "TRACE-PATH:report %s %s update trap\r\n", pUserTbl->userId, trace_path_mac_addr_2_str(pLoc->userMac) );
						}
					}


					/* 老化 */
					if( pLoc->resolvingTimer > USER_ID_RESOLVING_TIMES )
					{
						trace_path_del_notify( pUserTbl->userId, pLoc->userMac );
					}
					else
					{
						if( (pLoc->resolvingStatus == USER_TRACE_RESOLVING_WAIT) || (pLoc->onuId == 0) )
						{
							pLoc->resolvingTimer++;
							
							/* 地址解析 */
							trace_path_userid_resolving( pUserTbl->userId, pLoc->userMac, pLoc->oltBrdIdx, pLoc->oltPortIdx );
							resolvingCount++;
						}
					}
				}

				/*if( (updateCount >= tracePathTrapRate) && (resolvingCount >= tracePathResolveRate) )
				{
					hashIdx = TRACE_USERID_HASH_BUCKET;
					break;
				}*/
				totalCount++;
				if( totalCount >= USER_ID_SUPPORT_MAXNUM )
				{
					VOS_ASSERT(0);
					hashIdx = TRACE_USERID_HASH_BUCKET;
					break;
				}

				pUserTbl = pUserTbl->next;
			}
			
			VOS_SemGive( tracePathMSemId );

			if( (updateCount >= tracePathTrapRate) && (resolvingCount >= tracePathResolveRate) )
			{
				break;
			}
		}

		if( hashIdx >= TRACE_USERID_HASH_BUCKET )
			hashIdx_bak = 0;
		else
			hashIdx_bak = hashIdx;
#else
		for( hashIdx=0; hashIdx<TRACE_USERID_HASH_BUCKET; hashIdx++ )
		{
			VOS_SemTake( tracePathMSemId, WAIT_FOREVER );

			pUserTbl = gpTraceUserIdHashTable[hashIdx];
			while( pUserTbl )
			{
				for( i=0; i<USER_MACADDR_NUM; i++ )
				{
					pLoc = &(pUserTbl->userLocList[i]);

					if( MAC_ADDR_IS_INVALID(pLoc->userMac) )
						continue;

					/* 检查线路信息是否改变 */
					if( pLoc->resolvingStatus == USER_TRACE_RESOLVED_OK )
					{
						if( (updateCount < tracePathTrapRate) && (pLoc->updateFlag == 1) )
						{
							userTraceUpdate_EventReport( hashIdx, pLoc->userMac );
							/*trace_userid_update_over( hashIdx, pLoc->userMac );*/
							pLoc->updateFlag = 0;

							updateCount++;
							trace_path_debug_out( "TRACE-PATH:report %s %s update trap\r\n", pUserTbl->userId, trace_path_mac_addr_2_str(pLoc->userMac) );
						}
					}


					/* 老化 */
					if( pLoc->resolvingTimer > USER_ID_RESOLVING_TIMES )
					{
						trace_path_del_notify( pUserTbl->userId, pLoc->userMac );
					}
					else
					{
						if( (pLoc->resolvingStatus == USER_TRACE_RESOLVING_WAIT) || (pLoc->onuId == 0) )
						{
                            short int PonPortIdx = GetPonPortIdxBySlot(pLoc->oltBrdIdx, pLoc->oltPortIdx);
							pLoc->resolvingTimer++;
							trace_path_insert_mac_perpon(PonPortIdx, pLoc->userMac);
							resolvingCount++;
						}
					}
				}

				totalCount++;
				if( totalCount >= USER_ID_SUPPORT_MAXNUM )
				{
					VOS_ASSERT(0);
					hashIdx = TRACE_USERID_HASH_BUCKET;
					break;
				}

				pUserTbl = pUserTbl->next;
			}			
			VOS_SemGive( tracePathMSemId );
		}
        for(pon_id = 0; pon_id<MAXPON; pon_id++)
        {
            trace_path_mac_resolving_perpon(pon_id);
            trace_path_clear_mac_perpon(pon_id);            
        }
#endif
	}
	else if( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
	{
	}
	else /*if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )*/
	{
		VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
		for( hashIdx=0; hashIdx<TRACE_USERID_HASH_BUCKET; hashIdx++ )
		{

			pUserTbl = gpTraceUserIdHashTable[hashIdx];
			while( pUserTbl )
			{
				for( i=0; i<USER_MACADDR_NUM; i++ )
				{
					pLoc = &(pUserTbl->userLocList[i]);

					if( MAC_ADDR_IS_INVALID(pLoc->userMac) )
						continue;

					/* 老化 */
					if( pLoc->resolvingTimer > USER_ID_RESOLVING_TIMES )
					{
						trace_path_del_notify( pUserTbl->userId, pUserTbl->userLocList[i].userMac );
					}
					else
					{
						/*if( (pLoc->resolvingStatus == USER_TRACE_RESOLVING_WAIT) || (pLoc->onuId == 0) )*/
						{
							pLoc->resolvingTimer++;
						}
					}
				}

				totalCount++;
				if( totalCount >= USER_ID_SUPPORT_MAXNUM )
				{
					VOS_ASSERT(0);
					hashIdx = TRACE_USERID_HASH_BUCKET;
					break;
				}

				pUserTbl = pUserTbl->next;
			}
		}
		
		VOS_SemGive( tracePathMSemId );
	}

	return VOS_OK;
}
#endif
#if 1
int trace_path_insert_mac_perpon(short int PonPortIdx, unsigned char* mac)
{
    int ret = VOS_ERROR;
    trace_path_mac_t *pd = NULL;
    if(PonPortIdx>200 || PonPortIdx<0)
        return ret;
    pd = g_trace_path_mac_wait[PonPortIdx];
    if(!pd)
    {
        pd = g_malloc(sizeof(trace_path_mac_t));
        if(pd)
        {
            g_trace_path_mac_wait[PonPortIdx] = pd;
            VOS_MemZero(pd, sizeof(trace_path_mac_t));
            VOS_MemCpy(pd->mac[0], mac, 6);
            pd->macNum++;
            ret = VOS_OK;
        }
    }
    else
    {
        if(pd->macNum<64)
        {
            VOS_MemCpy(pd->mac[pd->macNum], mac, 6);
            pd->macNum++;
            ret = VOS_OK;
        }
    }
    return ret;
}
trace_path_mac_t *trace_path_get_mac(short int PonPortIdx)
{
    trace_path_mac_t *pd = NULL;
    if(PonPortIdx>200 || PonPortIdx<0)
        return pd;
    pd = g_trace_path_mac_wait[PonPortIdx];     
    return pd;
    
}
int trace_path_clear_mac_perpon(short int PonPortIdx)
{
    int ret = VOS_ERROR;
    trace_path_mac_t *pd = NULL;
    if(PonPortIdx>200 || PonPortIdx<0)
        return ret;
    pd = g_trace_path_mac_wait[PonPortIdx];
    if(pd)
        g_free(pd);
    g_trace_path_mac_wait[PonPortIdx] = NULL;
    return VOS_OK;
}

LONG trace_path_mac_resolving_local_perpon(short int PonPortIdx, UCHAR macnum, UCHAR *pUserMac)
{
	LONG rc = VOS_ERROR;
	if(pUserMac == NULL || macnum == 0)
		return rc;

	rc = trace_path_req_oam_broadcast_perpon(PonPortIdx, macnum, pUserMac );

	return rc;
}
LONG trace_path_resolving_req_cdp_send_perpon(UCHAR slotno, UCHAR portno, UCHAR macnum, UCHAR *pUserMac)
{
	LONG rc = VOS_ERROR;
	ULONG to_slotno = slotno;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(trace_path_mac_sync_msg_t);
	trace_path_mac_sync_msg_t  *pSyncMsg = NULL;
	SYS_MSG_S * pstMsg = NULL;

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		return rc;
	}
	if(pUserMac == NULL)
	{
		return rc;
	}
	if( (to_slotno == 0) || (to_slotno == SYS_LOCAL_MODULE_SLOTNO) )
		return rc;

	if( !(SYS_MODULE_IS_READY(to_slotno) || devsm_sys_is_switchhovering()) )
		return rc;

	pstMsg = ( SYS_MSG_S * ) CDP_AllocMsg( msgLen, MODULE_RPU_TRACEPATH );              
	if(pstMsg == NULL)
	{
		return rc;
	}

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = MODULE_RPU_TRACEPATH;
	pstMsg->ulDstModuleID = MODULE_RPU_TRACEPATH;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = slotno;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (VOID *)pSyncMsg;
	pstMsg->usFrameLen = sizeof(trace_path_mac_sync_msg_t);
	pstMsg->usMsgCode = TRACE_SNOOP_MSG_CODE_BROADCAST;
	
	pSyncMsg = (trace_path_mac_sync_msg_t *)(pstMsg + 1);
	pSyncMsg->msgtype = TRACE_PATH_MSG_TYPE_RESOLVE_REQ;
	VOS_MemCpy(pSyncMsg->mac[0], pUserMac, macnum*6);
	pSyncMsg->macnum = macnum;
	/*VOS_MemZero( &(pSyncMsg->userLocation), sizeof(user_loc_t) );*/
	pSyncMsg->slotno = slotno;
	pSyncMsg->portno = portno;
	/*pSyncMsg->userLocation.locFlag = USER_TRACE_FLAG_DYNAMIC;*/

	trace_path_debug_out( "TRACE-PATH:send mac %s resolving REQ to slot %d\r\n", trace_path_mac_addr_2_str(pUserMac), to_slotno );

	rc = CDP_Send( RPU_TID_CDP_USERTRACE, to_slotno,  RPU_TID_CDP_USERTRACE,  CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_RPU_TRACEPATH );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pstMsg );
		VOS_ASSERT(0);
	}

	return rc;
}

int trace_path_mac_resolving_perpon(short int PonPortIdx)
{
	LONG rc;
    short int slotno = GetCardIdxByPonChip(PonPortIdx);
    short int portno = GetPonPortByPonChip(PonPortIdx);
    trace_path_mac_t *pd = trace_path_get_mac(PonPortIdx);

    if(!pd)
        return VOS_OK;
    
	if( !(SYS_MODULE_IS_READY(slotno) || devsm_sys_is_switchhovering()) )
		return VOS_OK;
	if( SlotCardIsPonBoard(slotno) != ROK )
		return VOS_ERROR;
    
	if((slotno == SYS_LOCAL_MODULE_SLOTNO) || !SYS_MODULE_SLOT_ISHAVECPU(slotno) )
	{
	    rc = trace_path_req_oam_broadcast_perpon(PonPortIdx, pd->macNum, pd->mac[0]);
		trace_path_debug_out("TRACE-PATH:tx user trace oam to pon%d/%d %s\r\n", slotno, portno, ((rc == VOS_ERROR) ? "ERR" : "OK") );

	}
	else
	{
		trace_path_resolving_req_cdp_send_perpon(slotno, portno, pd->macNum, pd->mac[0]);
	}
	
	return VOS_OK;
}

void trace_path_wait_queue_init()
{
    short int PonPortIdx = 0;
    for(PonPortIdx = 0; PonPortIdx<200; PonPortIdx++)
        g_trace_path_mac_wait[PonPortIdx]=NULL;
}
#endif 

LONG trace_path_main_init( )
{
	trace_path_tbl_init();
	
	if( tracePathQId != 0 )
		return VOS_OK;

	tracePathQId = VOS_QueCreate( 512 , VOS_MSG_Q_PRIORITY);
	if( tracePathQId  == 0 )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	tracePathTId = ( VOS_HANDLE )VOS_TaskCreate("tTracePath", 200, trace_path_main_task, NULL );
	if( tracePathTId == NULL )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	VOS_QueBindTask( tracePathTId, tracePathQId );

	tracePathTimerId = VOS_TimerCreate( RPU_TID_CDP_USERTRACE, 0, 10000, (void *)trace_path_timer_callback, NULL, VOS_TIMER_LOOP );
	if( tracePathTimerId == VOS_ERROR )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	return VOS_OK;						
}



LONG trace_path_init()
{
	trace_path_main_init();
    trace_path_wait_queue_init();
	trace_path_cli_cmd_install();
	
	return VOS_OK;
}

LONG trace_path_fdb_resolving( UCHAR *pMacAddr, user_loc_t* pLoc )
{
	ULONG devIdx;
	ULONG slotno, portno;
	ULONG fdbStatus, l2Idx;
	LONG rc = VOS_ERROR;
	UCHAR macAddr[USER_MACADDR_LEN];

	if( (pMacAddr == NULL) || (pLoc == NULL) )
		return rc;
	
	slotno = 0;
	portno = 0;
	/*VOS_MemZero( pLoc, sizeof(user_loc_t) );*/
	VOS_MemCpy( macAddr, pMacAddr, USER_MACADDR_LEN );
	
	if( SYS_LOCAL_MODULE_ISHAVEPP() &&
		(local_mac_entry_learning_check(0, macAddr, &slotno, &portno) == VOS_OK) )
	{
		pLoc->resolvingStatus = USER_TRACE_RESOLVED_OK;

		if( (slotno == 0) /*&& (portno != 0)*/ )	/* 上联trunk */
		{
			pLoc->oltBrdIdx = slotno;
			pLoc->oltPortIdx = portno;
			return VOS_OK;
		}
		
		if( userport_is_uplink(slotno, portno) == VOS_YES )
		{
			/*if( SYS_SLOT_PORT_IS_LEGAL(slotno, portno) )*/
			{
				pLoc->oltBrdIdx = slotno;
				pLoc->oltPortIdx = portno;
			}
			return VOS_OK;
		}

		pLoc->oltBrdIdx = slotno;
		pLoc->oltPortIdx = portno;

		if( portno == 0 )
		{
			SYS_SLOT_LOOP_PON_PORT_BEGIN( slotno, portno )
			{
				devIdx = 0;
				rc = getOnuFromPonFdb( slotno, portno, pMacAddr, &devIdx, &fdbStatus, &l2Idx );
							
				if( rc == VOS_ERROR )
				{
                    /*2013-7-9.此处应该考虑12epon 的上联口。for Q.17806*/
					return rc;
				}
				if( devIdx != 0 )
				{
					pLoc->oltBrdIdx = GET_PONSLOT(devIdx);
					pLoc->oltPortIdx = GET_PONPORT(devIdx);
					pLoc->onuId = GET_ONUID(devIdx);
					break;
				}
			}
			SYS_SLOT_LOOP_PON_PORT_END()
		}
		else if( portno <= PONPORTPERCARD )
		{
			devIdx = 0;
			rc = getOnuFromPonFdb( slotno, portno, pMacAddr, &devIdx, &fdbStatus, &l2Idx );
						
			if( rc == VOS_ERROR )
			{
				return rc;
			}
			if( devIdx != 0 )
			{
				pLoc->oltBrdIdx = GET_PONSLOT(devIdx);
				pLoc->oltPortIdx = GET_PONPORT(devIdx);
				pLoc->onuId = GET_ONUID(devIdx);
			}
		}
	}
	else
	{
		SYS_SLOT_LOOP_PON_PORT_BEGIN( slotno, portno )
		{
			devIdx = 0;
			rc = getOnuFromPonFdb( slotno, portno, pMacAddr, &devIdx, &fdbStatus, &l2Idx );
						
			if( rc == VOS_ERROR )
			{
				return rc;
			}
			if( devIdx != 0 )
			{
				pLoc->oltBrdIdx = GET_PONSLOT(devIdx);
				pLoc->oltPortIdx = GET_PONPORT(devIdx);
				pLoc->onuId = GET_ONUID(devIdx);
				break;
			}
		}
		SYS_SLOT_LOOP_PON_PORT_END()
	}

	if( rc == VOS_OK )
	{
		VOS_MemZero( &userLocationRecvBuf, sizeof(user_loc_t) );
#if 0
		if( pLoc->oltBrdIdx == SYS_LOCAL_MODULE_SLOTNO )
#else 
        if(pLoc->oltBrdIdx == SYS_LOCAL_MODULE_SLOTNO || !SYS_MODULE_SLOT_ISHAVECPU(pLoc->oltBrdIdx))
#endif
		{
			trace_path_req_oam_broadcast( pLoc->oltBrdIdx, pLoc->oltPortIdx, TRACE_USERID_HASH_BUCKET, pMacAddr );
		}
		else
		{
			trace_path_oam_req_cdp_send( pMacAddr, pLoc->oltBrdIdx, pLoc->oltPortIdx );
		}

		tracePathOamFlag = 1;
		if( VOS_SemTake(traceOamBSemId, 500) == VOS_OK )
		{
			if( MAC_ADDR_IS_EQUAL(userLocationRecvBuf.userMac, pMacAddr) )
			{
				pLoc->onuBrdIdx = userLocationRecvBuf.onuBrdIdx;
				pLoc->onuPortIdx = userLocationRecvBuf.onuPortIdx;
				MAC_ADDR_CPY( pLoc->swMacAddr, userLocationRecvBuf.swMacAddr );
				pLoc->swPortIdx = userLocationRecvBuf.swPortIdx;
			}
		}
		tracePathOamFlag = 0;
	}
	
	return rc;
}

#pragma pack()


#if 0	/* for test only */
void test_size()
{
	sys_console_printf("\r\n size=%d\r\n", sizeof(trace_userid_table_t) );
}

LONG testResolvingOam( short int PonPortIdx, short int *pOnuIdx, user_trace_oam_pdu_t *pTxOam, user_trace_oam_pdu_t *pRxOam )
{
	VOS_MemCpy( pRxOam, pTxOam, sizeof(user_trace_oam_pdu_t) );
	pRxOam->onuEthPort = 1;
	pRxOam->swFlag = 1;
	pRxOam->swMacAddr[0] = 0;
	pRxOam->swMacAddr[1] = 0x11;
	pRxOam->swMacAddr[2] = 0x22;
	pRxOam->swMacAddr[3] = 0x33;
	pRxOam->swMacAddr[4] = 0x44;
	pRxOam->swMacAddr[5] = 0x55;
	pRxOam->swPortIdx = 2;
	pRxOam->reserve = 0;

	*pOnuIdx = 1;
	return VOS_OK;
}
#endif

#ifdef __cplusplus
}
#endif

