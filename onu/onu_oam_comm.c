#ifdef __cplusplus
extern "C"
  {
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "Onu_oam_comm.h"

#define OAM_SESSION_SEM_TAKE	if( VOS_SemTake(g_OamSessionSemId, WAIT_FOREVER) == VOS_OK )
#define OAM_SESSION_SEM_GIVE	VOS_SemGive(g_OamSessionSemId)

#define OAM_SESSION_SEND_SEM_TAKE	if( VOS_SemTake(g_OamSessionSendSemId, WAIT_FOREVER) == VOS_OK )
#define OAM_SESSION_SEND_SEM_GIVE	VOS_SemGive(g_OamSessionSendSemId)

Oam_Session_tables_t g_Oam_Session_tables; 
ULONG g_OamSessionSemId = 0;
ULONG g_OamSessionSendSemId = 0;
ULONG g_OamSessionId = 0;
ULONG g_Oam_Session_Queue_Id = 0;
LONG  g_Oam_Session_Task_Id = 0;
LONG  g_OamSession_TimerId = 0;
int   Oam_Session_flag = 0;
int debug_oam_session_enable = 0;
int Oam_Session_GetFreeIdx(int *SessionIdx)
{
    int local_idx = 0;
    int ret = VOS_ERROR;
    OAM_SESSION_SEM_TAKE;
    for(local_idx = 0;local_idx<MAX_OAM_SESSION_NUM;local_idx++)
    {
        if(!g_Oam_Session_tables[local_idx].ulvalid)
        {
            *SessionIdx = local_idx;
            ret = VOS_OK;
            break;
        }
    }
    OAM_SESSION_SEM_GIVE;
    return ret;
}

int Oam_Session_Search(char *pSession)
{
    int SessionIdx = 0;
    OAM_SESSION_SEM_TAKE;
    for(SessionIdx=0;SessionIdx<MAX_OAM_SESSION_NUM;SessionIdx++)
    {
        if(g_Oam_Session_tables[SessionIdx].ulvalid)
        {
            if(VOS_MemCmp(g_Oam_Session_tables[SessionIdx].sessionid, pSession, 8)==0)
            {
                OAM_SESSION_SEM_GIVE;
                return SessionIdx;
            }
        }
    }
    OAM_SESSION_SEM_GIVE;
    return VOS_ERROR;
}
int Oam_Session_TimeOut_Handler()
{
    int SessionIdx = 0;
    UCHAR liv_valid = 0;
    OAM_SESSION_SEM_TAKE;    
    for(SessionIdx=0;SessionIdx<MAX_OAM_SESSION_NUM;SessionIdx++)
    {
        liv_valid = g_Oam_Session_tables[SessionIdx].ulvalid; 
        if(liv_valid)
        {
            if(g_Oam_Session_tables[SessionIdx].TimeCount == 0)
            {
                DEBUG_OAM_SESSION_PRINTF("\r\nOam_Session_TimeOut_Handler : SessionIdx = %d TimeOut!\r\n", SessionIdx);
                VOS_MemZero(&g_Oam_Session_tables[SessionIdx], sizeof(Oam_Session_table_t));
            }
            else
                g_Oam_Session_tables[SessionIdx].TimeCount--;
        }
    }
    OAM_SESSION_SEM_GIVE;
    return VOS_ERROR;
}
int Oam_Session_DeleteBySessionfield(char *pSession)
{
    int SessionIdx = 0;
    int SessionIdx_back = 0;
    OAM_SESSION_SEM_TAKE;    
    {
        for(SessionIdx=0;SessionIdx<MAX_OAM_SESSION_NUM;SessionIdx++)
        {
            if(g_Oam_Session_tables[SessionIdx].ulvalid)
            {
                if(VOS_MemCpy(g_Oam_Session_tables[SessionIdx].sessionid, pSession, 8)==0)
                {
#if 1             
                    VOS_MemZero(&g_Oam_Session_tables[SessionIdx], sizeof(Oam_Session_table_t));
                    break;
#else if                
                    for(SessionIdx_back=SessionIdx;SessionIdx_back<MAX_OAM_SESSION_NUM-1;SessionIdx_back++)
                    {
                        if(g_Oam_Session_tables[SessionIdx_back+1].ulvalid)
                            VOS_MemCpy(g_Oam_Session_tables[SessionIdx_back], g_Oam_Session_tables[SessionIdx_back+1], sizeof(Oam_Session_table_t));
                        else
                        {
                            VOS_MemZero(g_Oam_Session_tables[SessionIdx], sizeof(Oam_Session_table_t));
                            break;                        
                        }
                    }
#endif                
                }
            }
        }
    }
    OAM_SESSION_SEM_GIVE;
    return VOS_OK;
}

int Oam_Session_DeleteBySessionId(SessionIdx)
{
    int SessionIdx_back = 0;
    OAM_SESSION_SEM_TAKE;
    {
        if(g_Oam_Session_tables[SessionIdx].ulvalid)
        {
#if 1                
            VOS_MemZero(&g_Oam_Session_tables[SessionIdx], sizeof(Oam_Session_table_t));
#else if                
            for(SessionIdx_back=SessionIdx;SessionIdx_back<MAX_OAM_SESSION_NUM-1;SessionIdx_back++)
            {
                if(g_Oam_Session_tables[SessionIdx_back+1].ulvalid)
                    VOS_MemCpy(g_Oam_Session_tables[SessionIdx_back], g_Oam_Session_tables[SessionIdx_back+1], sizeof(Oam_Session_table_t));
                else
                {
                    VOS_MemZero(g_Oam_Session_tables[SessionIdx], sizeof(Oam_Session_table_t));
                    break;                        
                }
            }
#endif                
        }
    }
    OAM_SESSION_SEM_GIVE;
    return VOS_OK;
}

int Oam_Session_Insert(unsigned char *pSessionBuf, char MsgType, ULONG flag, ULONG SyncSemId, UCHAR mode, ULONG QueId, OAMRECIEVECALLBACK func, unsigned char *pRecieveBuf, short int *BufLen)
{
    int result = VOS_ERROR;
    int SessionIdx = 0;
    
    if(Oam_Session_GetFreeIdx(&SessionIdx) == VOS_ERROR)
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }
    OAM_SESSION_SEM_TAKE;
    {
        g_Oam_Session_tables[SessionIdx].ulvalid = 1;
        VOS_MemCpy(g_Oam_Session_tables[SessionIdx].sessionid, pSessionBuf, 8);
        g_Oam_Session_tables[SessionIdx].MsgType = MsgType;
        g_Oam_Session_tables[SessionIdx].ulflag = flag;           
        g_Oam_Session_tables[SessionIdx].TimeCount = 30;/*3分钟*/
        g_Oam_Session_tables[SessionIdx].ulmode = mode; 
        g_Oam_Session_tables[SessionIdx].ulSynSem = SyncSemId; 
        if(pRecieveBuf)
        {
            g_Oam_Session_tables[SessionIdx].RxBuff = pRecieveBuf;
            g_Oam_Session_tables[SessionIdx].RxBuffLen = BufLen;
        }
        if(flag == OAM_NOTI_VIA_QUEUE && QueId)
            g_Oam_Session_tables[SessionIdx].ulQueId = QueId;            
        else if(flag == OAM_NOTI_VIA_FUNC && func)
            g_Oam_Session_tables[SessionIdx].func = func;
    }
    OAM_SESSION_SEM_GIVE;
    return SessionIdx;
}
int DrPeng_compose_sessionId(short int PonPortIdx, short int OnuIdx,unsigned char branch, unsigned short leaf, unsigned char ulsessionId[])
{
	if(NULL == ulsessionId)
		return VOS_ERROR;
    ulsessionId[0] = (unsigned char)((PonPortIdx>>8)&0xff);
    ulsessionId[1] = (unsigned char)(PonPortIdx & 0xff);
    ulsessionId[2] = (unsigned char)((OnuIdx>>8)&0xff);
    ulsessionId[3] = (unsigned char)(OnuIdx & 0xff);
    ulsessionId[4] = (unsigned char)(branch & 0xff);
    ulsessionId[5] = (unsigned char)((leaf>>8)&0xff);
    ulsessionId[6] = (unsigned char)(leaf & 0xff);

   return VOS_OK;
}


int DrPeng_Oam_Session_Send(
    short int PonPortIdx, 
    short int OnuIdx, 
    char  MsgType, /*消息类型*/
    UCHAR mode, /*同步还是异步:OAM_SYNC/OAM_ASNYC*/
    unsigned char branch, 
    unsigned short leaf, 
    OAMRECIEVECALLBACK func, /*回调函数接口*/
    unsigned char *pSendDataBuf,
    short int pSendDataBuflen,
    unsigned char *pRecieveBuf,
    short int *pRecieveBuflen
    )
{
    int ret = VOS_ERROR;
    int SessionIdx = 0;
    int ulSendBufLen = 0;
    ULONG ulSynSemId = 0;
    unsigned char Databuf[EUQ_MAX_OAM_PDU];
    unsigned char ulsessionId[8] = {0};
    
    VOS_MemZero(Databuf, EUQ_MAX_OAM_PDU);

    ret = DrPeng_compose_sessionId(PonPortIdx, OnuIdx, branch, leaf, ulsessionId);
	
	if (VOS_ERROR == ret)
	{
	    VOS_ASSERT(0);
	    return VOS_ERROR;
	}
    if(mode == OAM_SYNC)
    {
        ulSynSemId = VOS_SemBCreate( VOS_SEM_Q_PRIORITY, VOS_SEM_EMPTY);
        if (NULL == ulSynSemId)
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
    }
    DEBUG_OAM_SESSION_PRINTF("\r\nDrPeng_Oam_Session_Send: PonPortIdx = %d, OnuIdx = %d, Session = %s\r\n", PonPortIdx, OnuIdx, ulsessionId);
    if(NULL != pSendDataBuf)
    {
        VOS_MemCpy( &Databuf[OAMMSG_MSGTYPESTART], pSendDataBuf, pSendDataBuflen);
    }
    else
 	 return VOS_ERROR;
        
    SessionIdx = Oam_Session_Insert(ulsessionId, MsgType, 0, ulSynSemId, mode, 0, func, pRecieveBuf, pRecieveBuflen);
    if(SessionIdx == VOS_ERROR)
    {
        if(ulSynSemId)
            VOS_SemDelete(ulSynSemId);        
        return VOS_ERROR;
    }
    DEBUG_OAM_SESSION_PRINTF("                : SessionIdx = %d, MsgType = %d, Mode = %s\r\n", SessionIdx, MsgType, mode?"ASYNC":"SYNC");

    OAM_SESSION_SEND_SEM_TAKE;  
    ret = DrPeng_Comm_EUQ_info_request_transmit(PonPortIdx, (OnuIdx+1), Databuf, pSendDataBuflen);
    OAM_SESSION_SEND_SEM_GIVE;
    
    if(ret != ROK)
    {
        DEBUG_OAM_SESSION_PRINTF("                 : SessionIdx = %d, %s REQUEST SEND ERROR!\r\n", SessionIdx, mode?"ASYNC":"SYNC");        
        if(ulSynSemId)
            VOS_SemDelete(ulSynSemId);                
        return(RERROR);
    }
    
    if(mode == OAM_SYNC)
    {        
        if ( VOS_OK != VOS_SemTake(ulSynSemId, 200/*10800*/))
        {
            DEBUG_OAM_SESSION_PRINTF("                 : SessionIdx = %d, SYNC Sem TIMEOUT!\r\n", SessionIdx);
            Oam_Session_DeleteBySessionId(SessionIdx);/*超时应该删掉对应session id，不再处理callback 。2013-5-2*/            
            VOS_SemDelete(ulSynSemId);
            return VOS_ERROR;
        }
        VOS_SemDelete(ulSynSemId);
    }
    
    return VOS_OK;
}


int Oam_Session_Send(
    short int PonPortIdx, 
    short int OnuIdx, 
    char  MsgType, /*消息类型*/
    UCHAR mode, /*同步还是异步:OAM_SYNC/OAM_ASNYC*/
    ULONG ulFlag, /*是消息队列还是回调函数或返回字符串OAM_NOTI_VIA_NULL/OAM_NOTI_VIA_QUEUE/OAM_NOTI_VIA_FUNC*/
    ULONG QueId, /*消息队列id*/
    OAMRECIEVECALLBACK func, /*回调函数接口*/
    unsigned char *pSendDataBuf,
    short int pSendDataBuflen,
    unsigned char *pRecieveBuf,
    short int *pRecieveBuflen
    )
{
    int ret = VOS_ERROR;
    unsigned char ulsessionId[8] = {0};
    int SessionIdx = 0;
    int ulSendBufLen = 0;
    ULONG ulSynSemId = 0;
	unsigned char Databuf[EUQ_MAX_OAM_PDU];
    
    VOS_MemZero(Databuf, EUQ_MAX_OAM_PDU);
    g_OamSessionId++;
    VOS_Sprintf(ulsessionId, "%d", g_OamSessionId);    
    if(mode == OAM_SYNC)
    {
        ulSynSemId = VOS_SemBCreate( VOS_SEM_Q_PRIORITY, VOS_SEM_EMPTY);
        if (NULL == ulSynSemId)
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
    }
    DEBUG_OAM_SESSION_PRINTF("\r\nOam_Session_Send: PonPortIdx = %d, OnuIdx = %d, Session = %s\r\n", PonPortIdx, OnuIdx, ulsessionId);
    Databuf[OAMMSG_MSGTYPESTART] = MsgType;		
    if(pSendDataBuf)
    {
        VOS_MemCpy( &Databuf[OAMMSG_MSGTYPESTART+OAMMSG_MSGTYPELEN], pSendDataBuf, pSendDataBuflen);
        pSendDataBuflen += 1;
    }
    else
    {
        pSendDataBuflen = 1;
    }
        
    SessionIdx = Oam_Session_Insert(ulsessionId, MsgType, ulFlag, ulSynSemId, mode, QueId, func, pRecieveBuf, pRecieveBuflen);
    if(SessionIdx == VOS_ERROR)
    {
        if(ulSynSemId)
            VOS_SemDelete(ulSynSemId);        
        return VOS_ERROR;
    }
    DEBUG_OAM_SESSION_PRINTF("                : SessionIdx = %d, MsgType = %d, Mode = %s\r\n", SessionIdx, MsgType, mode?"ASYNC":"SYNC");

    OAM_SESSION_SEND_SEM_TAKE;  
    ret = Comm_EUQ_info_request_transmit(PonPortIdx, (OnuIdx+1), Databuf, pSendDataBuflen, ulsessionId);
    OAM_SESSION_SEND_SEM_GIVE;
    
    if(ret != ROK)
    {
        DEBUG_OAM_SESSION_PRINTF("                 : SessionIdx = %d, %s REQUEST SEND ERROR!\r\n", SessionIdx, mode?"ASYNC":"SYNC");        
        if(ulSynSemId)
            VOS_SemDelete(ulSynSemId);                
        return(RERROR);
    }
    
    if(mode == OAM_SYNC)
    {        
        if ( VOS_OK != VOS_SemTake(ulSynSemId, 200/*10800*/))
        {
            DEBUG_OAM_SESSION_PRINTF("                 : SessionIdx = %d, SYNC Sem TIMEOUT!\r\n", SessionIdx);
            Oam_Session_DeleteBySessionId(SessionIdx);/*超时应该删掉对应session id，不再处理callback 。2013-5-2*/            
            VOS_SemDelete(ulSynSemId);
            return VOS_ERROR;
        }
        VOS_SemDelete(ulSynSemId);
    }
    
    return VOS_OK;
}

int Oam_Session_RecieveCallBack(
    short int PonPortIdx, 
    short int OnuId, 
    short int llid, 
    short int len, 
    unsigned char *pDataBuf, 
    unsigned char  *pSessionField)
{
    int SessionIdx = 0;
    int ulsyncSem = 0;
    ULONG ulQueId = 0;
    ULONG MsgType = 0;
    short int OnuIdx = OnuId - 1;
    short int LLIDIdx;
    UCHAR ulmode = 0;
    UCHAR ulflag = 0;
    OAMRECIEVECALLBACK func; 
    unsigned char* ulpRecieveBuf;
    int ret = 0;
    
    DEBUG_OAM_SESSION_PRINTF("Oam_Session_RecieveCallBack: PonPortIdx = %d, OnuIdx = %d, session = %s\r\n", PonPortIdx, OnuIdx, pSessionField);
    DEBUG_OAM_SESSION_PRINTF("                           : MsgType = %d Length = %d\r\n", pDataBuf[0], len);        

	if(( pDataBuf == NULL ) ||(pSessionField == NULL))
    {
		VOS_ASSERT(0);
		if( pDataBuf != NULL )
			VOS_Free(pDataBuf);
		if(pSessionField != NULL )
			VOS_Free( pSessionField );
		return( RERROR );
    }

	if( !OLT_LOCAL_ISVALID(PonPortIdx) )
    {
		if( pDataBuf != NULL )
			VOS_Free( pDataBuf );
		if( pSessionField != NULL )
			VOS_Free( pSessionField );            
		sys_console_printf("\r\n Recieve Onu EUQMsg err:Pon%d out of range\r\n", (PonPortIdx+1));
		return( RERROR);
    }
		
	if( !ONU_IDX_ISVALID(OnuIdx) ) 
	{
		if( pDataBuf != NULL )
			VOS_Free( pDataBuf );
		if( pSessionField != NULL )
			VOS_Free( pSessionField );
		
		sys_console_printf(" Recieve Onu EUQMsg err :onu%d/%d/%d out of range\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuId );
		return( RERROR );
	}
		
    LLIDIdx = GetOnuLLIDIdx(PonPortIdx, OnuIdx, llid);
	if( !LLID_IDX_ISVALID(LLIDIdx) )
	{
		if( pDataBuf != NULL )
			VOS_Free( pDataBuf );
		if( pSessionField != NULL )
			VOS_Free( pSessionField );
		
		sys_console_printf(" Recieve Onu EUQMsg err :onu%d/%d/%d's llid:%d out of range\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuId, llid );
		return( RERROR );
	}
    
    SessionIdx = Oam_Session_Search(pSessionField);
    if(SessionIdx == VOS_ERROR)
    {
#if( EPON_MODULE_USER_TRACE == EPON_MODULE_YES )
		if(ReceivedFrameIsGwOAM_UserTrace_Rsp1((unsigned char *) pDataBuf ) == TRUE)
			GwOamMsg_UserTrace_Handler1(PonPortIdx,OnuIdx, pSessionField, len,(unsigned char *)pDataBuf);
#endif
        DEBUG_OAM_SESSION_PRINTF("Oam_Session_Search ERROR   : MsgType = %d Length = %d\r\n", pDataBuf[0], len);        
    	if( pDataBuf != NULL )
    		VOS_Free( pDataBuf );
    	if( pSessionField != NULL )
    		VOS_Free( pSessionField );    
        return VOS_ERROR;
    }
    OAM_SESSION_SEM_TAKE;
    {
        ulsyncSem  = g_Oam_Session_tables[SessionIdx].ulSynSem;
        ulQueId    = g_Oam_Session_tables[SessionIdx].ulQueId;
        func       = g_Oam_Session_tables[SessionIdx].func;
        ulpRecieveBuf = g_Oam_Session_tables[SessionIdx].RxBuff;
        ulmode     = g_Oam_Session_tables[SessionIdx].ulmode;   
        MsgType    = g_Oam_Session_tables[SessionIdx].MsgType;
        ulflag     = g_Oam_Session_tables[SessionIdx].ulflag;
    }
    OAM_SESSION_SEM_GIVE;

    if(ulflag == OAM_NOTI_VIA_FUNC && func)
    {
        (*func)(PonPortIdx, OnuIdx, len, pDataBuf, pSessionField);
    }
    else if(ulflag == OAM_NOTI_VIA_QUEUE && ulQueId)
    {
        unsigned long aulMsg[4] = {MODULE_ONU, 0, 0, 0};
        unsigned char *pDataBufSend = VOS_Malloc(len+2, MODULE_ONU);
		if( pDataBufSend == NULL )
	 	{
	 	VOS_ASSERT(0);
	    	if( pDataBuf != NULL )
	    		VOS_Free( pDataBuf );
	    	if( pSessionField != NULL )
	    		VOS_Free( pSessionField );    
	        return VOS_ERROR;
	 	}
        switch(MsgType)
        {
            case GET_ONU_SYS_INFO_REQ:
            case SET_ONU_SYS_INFO_REQ:
                aulMsg[1] = FC_ONU_REGISTER_INPROCESS;
                break;
            default:
                VOS_ASSERT(0);
                break;
        }
        
        *(short int *)pDataBufSend = len;    
        VOS_MemCpy(pDataBufSend+2, pDataBuf, len);

        aulMsg[0] = PonPortIdx;
        aulMsg[2] = (OnuIdx << 16) | (LLIDIdx & 0xFFFF);
        aulMsg[3] = (int)pDataBufSend;

        ret = VOS_QueSend( ulQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
        if( ret !=  VOS_OK )
        {
            DEBUG_OAM_SESSION_PRINTF("Oam_Session Msg Send ERROR : ulQueId = %d\r\n", ulQueId);                    
            VOS_Free(pDataBufSend);
            ret = RERROR; 
        }
        
    }
    else if(ulflag == OAM_NOTI_VIA_NULL && ulpRecieveBuf)
    {
        OAM_SESSION_SEM_TAKE;
        {
            VOS_MemCpy(g_Oam_Session_tables[SessionIdx].RxBuff, pDataBuf, len);
            *(g_Oam_Session_tables[SessionIdx].RxBuffLen) = len;
        }
        OAM_SESSION_SEM_GIVE;
    }
    /*释放内存*/    
    if( pDataBuf != NULL )
    {
        VOS_Free( pDataBuf );
    }
    if( pSessionField != NULL )
    {
        VOS_Free( pSessionField ); 
    }
    
    if(ulmode == OAM_SYNC)
    {
        if(ulsyncSem)
            VOS_SemGive(ulsyncSem);
    }
    
    Oam_Session_DeleteBySessionId(SessionIdx);
    return VOS_OK;
}

int Oam_Session_TimerCallback()
{
	unsigned long aulMsg[4] = { MODULE_ONU, FC_OAM_SESSION_TIMEOUT, 0, 0};
    
    if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{
		if(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
            if(VOS_QueNum(g_Oam_Session_Queue_Id)<(OAM_SESSIOM_MAX_QNUM/2))
    			VOS_QueSend(g_Oam_Session_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL);
		}
	}
}
static VOID Oam_Session_Task()
{
	unsigned long aulMsg[4];
	long result;
	short int PonPortIdx = 0, OnuIdx = 0;
	unsigned char MsgType;
	int length;
	unsigned char *pBuf;
    Oam_Session_flag = 1;
	while(1)
	{
		result = VOS_QueReceive(g_Oam_Session_Queue_Id, aulMsg, WAIT_FOREVER);
		if( result == VOS_ERROR )
		{
			VOS_ASSERT(0);
			VOS_TaskDelay(20);
			continue;
		}
		if( result == VOS_NO_MSG ) continue; 
		switch(aulMsg[1])
		{
            case FC_OAM_SESSION_TIMEOUT:
                Oam_Session_TimeOut_Handler();
                break;
            default:
                VOS_ASSERT(0);
                break;
		}
	}
    return ;
}

DEFUN ( oam_session_debug_func,
        oam_session_debug_cmd,
        "debug oam-session",
        DEBUG_STR
        "oam session print option\n")
{
    debug_oam_session_enable = 1;
    return CMD_SUCCESS;
}
DEFUN ( undo_oam_session_debug_func,
        undo_oam_session_debug_cmd,
        "undo debug oam-session",
        "undo operation\n"        
        DEBUG_STR
        "oam session print option\n")
{
    debug_oam_session_enable = 0;
    return CMD_SUCCESS;
}
void Oam_Session_Init()
{
    install_element(DEBUG_HIDDEN_NODE, &oam_session_debug_cmd);
    install_element( DEBUG_HIDDEN_NODE, &undo_oam_session_debug_cmd);
    
    VOS_MemZero(g_Oam_Session_tables, sizeof(Oam_Session_tables_t));

    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
    {
        if(g_OamSessionSemId == 0)
        	g_OamSessionSemId = VOS_SemMCreate( VOS_SEM_Q_FIFO );	
        if(g_OamSessionSendSemId == 0)
        	g_OamSessionSendSemId = VOS_SemMCreate( VOS_SEM_Q_FIFO );	

        if( g_Oam_Session_Queue_Id == 0 )  
            g_Oam_Session_Queue_Id = VOS_QueCreate( OAM_SESSIOM_MAX_QNUM , VOS_MSG_Q_FIFO);
    	if( g_Oam_Session_Queue_Id  == 0 ) 
    	{
    		VOS_ASSERT( 0 );
    	}
#if 1
    	g_Oam_Session_Task_Id = VOS_TaskCreateEx( "tOamSession", Oam_Session_Task, TASK_PRIORITY_ONU, 81920, NULL );
    	while( Oam_Session_flag != 1 ) 
            VOS_TaskDelay( VOS_TICK_SECOND /2 );
    	VOS_QueBindTask(( VOS_HANDLE )g_Oam_Session_Task_Id, g_Oam_Session_Queue_Id);

        g_OamSession_TimerId = VOS_TimerCreate( MODULE_OLT, 0, SECOND_1, Oam_Session_TimerCallback, NULL, VOS_TIMER_LOOP );
#endif        
    }
}

#ifdef __cplusplus
}
#endif

