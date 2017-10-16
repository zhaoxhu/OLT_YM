#ifdef __cplusplus
extern "C"
  {
#endif

#include "vos/vospubh/Vos_typevx.h"
#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_types.h"
#include "vos/vospubh/vos_sysmsg.h"
#include "vos/vospubh/vos_sem.h"
#include "vos/vospubh/vos_que.h"
#include "vos/vospubh/vos_string.h"
#include "vos/vospubh/vos_task.h"
#include "vos/vospubh/vos_que.h"
#include "vos/vospubh/vos_syslog.h"
#include "vos/vospubh/vos_timer.h"
#include "vos/vospubh/Cdp_pub.h"

#include  "OltGeneral.h"
#if(EPON_MODULE_ONU_MAC_OVERFLOW_CHECK==EPON_MODULE_YES)
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "gwEponMibData.h"

extern void TimerSendMessage();
extern VOID OnuMacMainTask(void);
extern LONG onuMAC_chk_cli_cmd_install();
extern int recv_onuMACcheck(int slotIdx );
extern CHAR *macAddress_To_Strings(UCHAR *pMacAddr);
extern int parse_pon_command_parameter( struct vty *vty, ULONG *pulSlot, ULONG *pulPort , ULONG *pulOnuId, INT16 *pi16PonId );
extern int OnuMacTtableOverflow_EventReport( ULONG onuDevIdx);
extern int OnuMacTtableOverflow_EventReportClear( ULONG onuDevIdx);
extern int OLT_SetOnuMacCheckPeriod(short int olt_id, ULONG  period);

/* modified by xieshl 20120813, 解决检测使能、告警门限、PON板拔插等造成的告警状态上
    报不准确的问题，问题单15520 */
/*ONU MAC 地址最大容量检测*//*add by sxh20111227*/
ULONG  OnuMacTable_check_interval=120000; /*检测周期 */
UCHAR onu_mac_check_alarm_state[SYS_MAX_PON_PORTNUM][MAXONUPERPONNOLIMIT];
USHORT onu_mac_check_counter[SYS_MAX_PON_PORTNUM][MAXONUPERPONNOLIMIT];
ULONG  Onu_user_max_all=DEFAULT_ONU_MAC_THRESHOLD;
ULONG mac_check_exit_flag=0;   /*功能存在标志，1为存在*/

ULONG   OnuMacCheckQueId=0;
ULONG   OnuMacCheckTaskId=0;
ULONG   OnuMacCheckTimerId=0;

#define   ONUMACCHECKQUNNUM	    50
#define	ONU_MSG_CODE_STARTUP_TIMER		1
#define	ONU_MSG_CODE_RECHECK			2

LONG Get_Onu_mac_threshold(ULONG devIdx, ULONG *value);
LONG clearOnuMacCheckAlarmStateByAll();
LONG onu_mac_check_alarm_state_set( SHORT PonPortIdx, SHORT OnuIdx, ULONG state );
LONG onu_mac_check_alarm_threshold_get();
LONG OnuMacCheckMsgSend( ULONG code );


int onuMACcheck_play()
{            
	if(mac_check_exit_flag==1)
		return VOS_OK;

	VOS_MemZero( (void*)onu_mac_check_counter, sizeof(onu_mac_check_counter) );
	VOS_MemZero( (void*)onu_mac_check_alarm_state, sizeof(onu_mac_check_alarm_state) );

	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER) 
	{           
		if( OnuMacCheckQueId  == 0 )    /*问题单14745*/
			OnuMacCheckQueId = VOS_QueCreate(ONUMACCHECKQUNNUM, VOS_MSG_Q_FIFO);

		if(OnuMacCheckTaskId==0)
		{
			OnuMacCheckTaskId=VOS_TaskCreate("tOnuMacThr", TASK_PRIORITY_BELOW_NORMAL, (VOS_TASK_ENTRY)OnuMacMainTask, NULL);                              
			VOS_QueBindTask(( VOS_HANDLE ) OnuMacCheckTaskId,  OnuMacCheckQueId );  
		}
		
		if( OnuMacCheckTimerId == 0 )
			OnuMacCheckTimerId = VOS_TimerCreate( MODULE_ONU, 0, OnuMacTable_check_interval, (void *)TimerSendMessage, NULL, VOS_TIMER_LOOP );
		if( OnuMacCheckTimerId == VOS_ERROR )
		{      
			VOS_TaskDelete(OnuMacCheckTaskId);
			VOS_QueDelete(OnuMacCheckQueId);

			VOS_ASSERT( 0 );        		
			return( RERROR );
		}
		else
		{
			/*sys_console_printf("-------start onu mac check!------\r\n");*/
			OnuMacCheckMsgSend( ONU_MSG_CODE_RECHECK );
			return VOS_OK;/*( OnuMacCheckTimerId );*/
		}
	}        

	return VOS_OK;
}


/*关闭ONU MAC 门限检测功能*/
int  onuMACcheck_stop()
{        
        if(mac_check_exit_flag ==0)
            return VOS_OK;

        mac_check_exit_flag=0;
        TimerSendMessage();
        VOS_TaskDelay(VOS_TICK_SECOND/4);

	if( OnuMacCheckTimerId )
        VOS_TimerDelete(MODULE_ONU, OnuMacCheckTimerId);
        OnuMacCheckTimerId = 0;     
        
        /*sys_console_printf("-------stop onu mac check!------\r\n");*/
	clearOnuMacCheckAlarmStateByAll();
        
        return  VOS_OK;
}

int OnuMacCheckEnable(ULONG  enable)
{
        short int Iret=VOS_ERROR;
        
        if(enable==1)
        {
            Iret=onuMACcheck_play();
        }
        else
            {
                Iret=onuMACcheck_stop();
            }

        return Iret;
}
int OnuMacCheckPeriod(ULONG  period)
{
	if( period < 10000 || period > 1000000000 )
		return VOS_ERROR;
	if( OnuMacCheckTimerId )
	{
	if( VOS_TimerChange( MODULE_ONU, OnuMacCheckTimerId, period) == VOS_ERROR )
		return VOS_ERROR;
	}
	OnuMacTable_check_interval = period;

	return VOS_OK;
}

int GetOnuMacCheckEnable(ULONG  *enable)
{
    *enable=mac_check_exit_flag;
    return VOS_OK;
}

VOID OnuMacMainTask(void)
{
       LONG  result_msg;
	ULONG ulRcvMsg[4];
	SYS_MSG_S *pMsg;

        mac_check_exit_flag=1;
        while(1)
            {
		result_msg = VOS_QueReceive( OnuMacCheckQueId, ulRcvMsg ,WAIT_FOREVER );
		if( result_msg == VOS_ERROR ) 
		{
			VOS_ASSERT(0);
			VOS_TaskDelay(50);
			continue;
		}
		if( result_msg == VOS_NO_MSG )
			continue;
              
		pMsg = (SYS_MSG_S *)ulRcvMsg[3];
		if( NULL == pMsg )
		{
			VOS_ASSERT(pMsg);
			VOS_TaskDelay(20);
			continue;
		}
		
		switch( pMsg->usMsgCode )
		{
			case ONU_MSG_CODE_STARTUP_TIMER:
			case ONU_MSG_CODE_RECHECK:
                if(mac_check_exit_flag)/*只有使能的情况下才处理告警,add by luh 2012-11-1*/
				    recv_onuMACcheck(pMsg->ulDstSlotID);
				break;
                
			default:
				break;
		}		

		VOS_Free(pMsg);         /*本板消息释放(包括PDU消息)*/
		pMsg = NULL;
	}
}

LONG OnuMacCheckMsgSend( ULONG code )
{
	ULONG aulMsg[4] = { MODULE_ONU, 0, 0, 0 };
	SYS_MSG_S * pstMsg = NULL;
	ULONG msgLen = sizeof(SYS_MSG_S);
	LONG result_msg;

	pstMsg = (SYS_MSG_S *)VOS_Malloc( msgLen, MODULE_ONU );
	if(pstMsg == NULL)
	{
		/*VOS_ASSERT(pstMsg);*/
		return VOS_ERROR;
	}            

	/*应该检查PON板是否在位*/
	pstMsg->ulSrcModuleID = MODULE_ONU;
	pstMsg->ulDstModuleID = MODULE_ONU;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID =SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->usMsgCode = code;
	pstMsg->usFrameLen=msgLen;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (pstMsg + 1);    

	aulMsg[1] = code;
	aulMsg[3] = (ULONG)pstMsg;

	result_msg = VOS_QueSend(OnuMacCheckQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL);
	if( result_msg != VOS_OK )
	{
		/*VOS_ASSERT(0);*/
		VOS_Free( pstMsg );
	}

	return result_msg;
}

void TimerSendMessage()
{
	if( VOS_QueNum(OnuMacCheckQueId) <= 3 )
		OnuMacCheckMsgSend( ONU_MSG_CODE_STARTUP_TIMER );
}

int recv_onuMACcheck(int slotIdx )
{
    ULONG ponport,onuid;
    /*int     maxport;*/
    ULONG devIdx=0;
    short int active_records=0;
    ULONG  maxMAC=DEFAULT_ONU_MAC_THRESHOLD;
    int ret=VOS_ERROR;
    ULONG slotno, portno;
    int pon_state;

    /*maxport=GetSlotCardPonPortNum(slotIdx);*/
    Get_Onu_mac_threshold(devIdx, &maxMAC);
    
    for(ponport=0; ponport<MAXPON; ponport++)
    {
        slotno = GetCardIdxByPonChip(ponport);
        portno = GetPonPortByPonChip(ponport);
        pon_state = PonPortIsWorking(ponport);
        /*2013-2-1，6900M-6900只有pon板需要进行判断，而6100-6700-6900S可以跳过直接进行处理*/
        if(SYS_MODULE_IS_CPU_PON(slotno) && slotno != SYS_LOCAL_MODULE_SLOTNO)
			continue;
        
        for(onuid=0;onuid<MAXONUPERPON;onuid++)
        {
            active_records = 0;
            devIdx=MAKEDEVID(slotno, portno, (onuid+1));                         

            if( (SlotCardIsPonBoard(slotno) == ROK) && (pon_state == TRUE) && 
                (ROK == ThisIsValidOnu( ponport, onuid)) &&
                (GetOnuOperStatus(ponport, onuid) == ONU_OPER_STATUS_UP) )
            {
               
                ret=OnuMgt_GetOltMacAddrTbl(ponport,onuid, &active_records, NULL);             
                if(ret!=VOS_OK)
                {
                    active_records=1;
                }                                      
            }
				 
            if((active_records >= maxMAC)&&(/*onu_mac_check_counter[ponport][onuid]<maxMAC*/onu_mac_check_alarm_state[ponport][onuid] == 0))
            {
                OnuMacTtableOverflow_EventReport(devIdx);
                if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
                    onu_mac_check_alarm_state_set( ponport, onuid, 1 );
            }
            else if((active_records<maxMAC)&&(/*onu_mac_check_counter[ponport][onuid]>=maxMAC*/onu_mac_check_alarm_state[ponport][onuid] == 1))
            {
                OnuMacTtableOverflow_EventReportClear(devIdx);
                if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
                    onu_mac_check_alarm_state_set( ponport, onuid, 2 );
            }
            onu_mac_check_counter[ponport][onuid]=active_records;
        }
    }
    return  ret;   
}

/*设置ONU的MAC 地址表的最大门限值*/
LONG Set_Onu_mac_threshold( ULONG devIdx, ULONG value)
{
	if(value == 0)
		return OLT_SetOnuMacCheckEnable(-1, 0);
	else
	{
		OLT_SetOnuMacCheckEnable(-1, 1);
       	return OLT_SetOnuMacThreshold(-1,value);
	}
}

/*获取ONU MAC 地址表的最大门限*/
LONG Get_Onu_mac_threshold(ULONG devIdx, ULONG *value)
{
	if(value == NULL)  
            return VOS_ERROR;		

	*value = onu_mac_check_alarm_threshold_get(); 
        return VOS_OK;    
}

LONG onu_mac_check_alarm_threshold_set( ULONG threshold )
{
	if( (threshold == 0) || (threshold > 10000) )
		threshold = 10000;
	
	Onu_user_max_all = threshold;
	OnuMacCheckMsgSend( ONU_MSG_CODE_RECHECK );

	return VOS_OK;
}

LONG onu_mac_check_alarm_threshold_get()
{
	return Onu_user_max_all;
}

LONG onu_mac_check_counter_get( SHORT PonPortIdx, SHORT OnuIdx, ULONG *pCounter )
{
	if( pCounter == NULL )
		return VOS_ERROR;
	if( (PonPortIdx < 0) || (PonPortIdx >= SYS_MAX_PON_PORTNUM) )
		return VOS_ERROR;
	if( (OnuIdx < 0) || (OnuIdx >= MAXONUPERPON) )
		return VOS_ERROR;

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		*pCounter = onu_mac_check_counter[PonPortIdx][OnuIdx];
	else
		*pCounter = 0;
	
	return VOS_OK;
}

LONG onu_mac_check_alarm_state_get( SHORT PonPortIdx, SHORT OnuIdx, ULONG *pState )
{
	if( pState == NULL )
		return VOS_ERROR;
	if( (PonPortIdx < 0) || (PonPortIdx >= SYS_MAX_PON_PORTNUM) )
		return VOS_ERROR;
	if( (OnuIdx < 0) || (OnuIdx >= MAXONUPERPON) )
		return VOS_ERROR;

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		*pState = onu_mac_check_alarm_state[PonPortIdx][OnuIdx];
	else
		*pState = 0;
	
	return VOS_OK;
}

/* state: 0-清除统计计数和告警状态，1-设置告警状态，2-清除告警状态 */
LONG onu_mac_check_alarm_state_set( SHORT PonPortIdx, SHORT OnuIdx, ULONG state )
{
	if( (PonPortIdx < 0) || (PonPortIdx >= SYS_MAX_PON_PORTNUM) )
		return VOS_ERROR;
	if( (OnuIdx < 0) || (OnuIdx >= MAXONUPERPON) )
		return VOS_ERROR;

	if( state == 0 )
		onu_mac_check_counter[PonPortIdx][OnuIdx] = 0;
	else if( state == 2 )
		state = 0;
	onu_mac_check_alarm_state[PonPortIdx][OnuIdx] = state;

	return VOS_OK;
}
LONG setOnuMacCheckAlarmStateByDevIdx( ULONG onuDevIdx, ULONG state )
{
	SHORT PonPortIdx, OnuIdx;
	PonPortIdx = GetPonPortIdxBySlot( GET_PONSLOT(onuDevIdx), GET_PONPORT(onuDevIdx) );
	OnuIdx = GET_ONUID(onuDevIdx) - 1;
	return onu_mac_check_alarm_state_set( PonPortIdx, OnuIdx, state );
}

LONG clearOnuMacCheckAlarmStateByPonPort( ULONG slotno, ULONG portno )
{
	SHORT PonPortIdx, OnuIdx;
	ULONG devIdx, state;
	
	PonPortIdx = GetPonPortIdxBySlot( slotno, portno );
	if( (PonPortIdx < 0) || (PonPortIdx >= SYS_MAX_PON_PORTNUM) )
		return VOS_ERROR;
	for( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++ )
	{
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		{
			state = 0;
			if( onu_mac_check_alarm_state_get(PonPortIdx, OnuIdx, &state) == VOS_OK )
			{
				if( state )
				{
					devIdx = MAKEDEVID( slotno, portno, OnuIdx+1 );
					OnuMacTtableOverflow_EventReportClear(devIdx);
					/*onu_mac_check_alarm_state_set( PonPortIdx, OnuIdx, 0 );*/
				}
			}
		}
		onu_mac_check_alarm_state_set( PonPortIdx, OnuIdx, 0 );
	}
	return VOS_OK;
}
LONG clearOnuMacCheckAlarmStateBySlot( ULONG slotno, ULONG moduleType )
{
	ULONG portno, portMax;

	if( moduleType )
		portMax = typesdb_module_portnum( moduleType );
	else
		portMax = PONPORTPERCARD;
	if( (portMax == 0) || (portMax > PONPORTPERCARD) )
		return VOS_ERROR;
	
	for( portno=1; portno<=portMax; portno++ )
	{
		clearOnuMacCheckAlarmStateByPonPort( slotno, portno );
	}
	return VOS_OK;
}

LONG clearOnuMacCheckAlarmStateByAll()
{
	ULONG slotno, portno;

	SYS_SLOT_PORT_LOOP_BEGIN( slotno, portno )
	{
		if( SYS_MODULE_IS_PON(slotno) )
		{
            /*modified by luh 2013-2-1, 6900M 2、3槽位告警清除重复上报*/
#if 0            
			clearOnuMacCheckAlarmStateByPonPort( slotno, portno );
#else
            if(SYS_MODULE_IS_CPU_PON(slotno))
            {
                if(slotno == SYS_LOCAL_MODULE_SLOTNO)
        			clearOnuMacCheckAlarmStateByPonPort( slotno, portno );
            }
            else
    			clearOnuMacCheckAlarmStateByPonPort( slotno, portno );
#endif
		}
	}
	SYS_SLOT_PORT_LOOP_END( slotno, portno )

	return VOS_OK;
}

LONG resumeOnuMacCheckConfig( SHORT PonPortIdx )
{
	if( Onu_user_max_all != DEFAULT_ONU_MAC_THRESHOLD )
		OLT_SetOnuMacThreshold( PonPortIdx, Onu_user_max_all );
	/*恢复ONU MAC 门限检测的开关*/
	if( mac_check_exit_flag != 0 )
		OLT_SetOnuMacCheckEnable( PonPortIdx, mac_check_exit_flag );
	if( OnuMacTable_check_interval != 120000 )
		OLT_SetOnuMacCheckPeriod( PonPortIdx, OnuMacTable_check_interval );
	return VOS_OK;
}

int ShowOnuMacLearningCounterListByVty( short int PonPortIdx, LONG over_thr, struct vty *vty )
{
    short int OltStartID, OltEndID;
    short int OnuIdx;
    short int OnuHasShow;
    ULONG flag, threshold;
    int OnuEntry, OnuEntryBase;
    
    short int PonNeedShow;
    short int PonHasShow;
    int OltSlot, OltPort;
    /*long active_records;*/
    char szAlignBuf[16];
    int iRet;

    if ( OLT_ID_ALL == PonPortIdx )
    {
        OltStartID  = 0;
        OltEndID    = MAXPON;
        PonNeedShow = 0; 
    }
    else
    {
        OltStartID  = PonPortIdx;
        OltEndID    = PonPortIdx + 1;
        PonNeedShow = 0;
    }

    if( over_thr == 0 )
        threshold = onu_mac_check_alarm_threshold_get();
    else
	 threshold = over_thr;

    OnuHasShow = 0;
    for ( ; OltStartID < OltEndID ; OltStartID++ )
    {
        PonHasShow = 0;

        OltSlot = GetCardIdxByPonChip(OltStartID);
        OltPort = GetPonPortByPonChip(OltStartID);
        if(SlotCardIsPonBoard(OltSlot) != ROK )
            continue;

	 OnuEntryBase = OltStartID * MAXONUPERPON;
        for ( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++ )
        {
                if( ROK != ThisIsValidOnu( OltStartID, OnuIdx ) )
			continue;
        	if( GetOnuOperStatus( OltStartID, OnuIdx ) != ONU_OPER_STATUS_UP )
			continue;
                 iRet=OnuMgt_GetOnuMacCheckFlag(OltStartID,OnuIdx,&flag);
                 if(iRet != VOS_OK)
			continue;
			
                 OnuEntry = OnuEntryBase + OnuIdx ;

                   if(flag >= over_thr)
                   {
                        if ( 0 == OnuHasShow )
                            {
                                OnuHasShow = -1;
                              
                                vty_out(vty, "\r\n   %-10s%-15s%s\r\n", "ONU", "Mac-addr", "Counter");
                                vty_out(vty, "-----------------------------------\r\n");
                            }
                    
                        if ( 0 == PonHasShow )
                        {
                            PonHasShow = 1;

                            if ( 0 < OnuHasShow )
                            {
                                vty_out(vty, "\r\n");
                            }
                            else
                            {
                                OnuHasShow = 1;
                            }

                            if ( 0 != PonNeedShow )
                            {
                                vty_out(vty, "Pon%d/%d:\r\n", OltSlot, OltPort);
                            }
                        }
                   
                     VOS_Snprintf(szAlignBuf, 15, "%s %d/%d/%d", (flag>=threshold ? "*" : " "), OltSlot, OltPort, OnuIdx + 1);                    
		       vty_out(vty, "%-11s%-18s%d\r\n", szAlignBuf, macAddress_To_Strings(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr) ,flag);
                }
            }   
        }  

    if( OnuHasShow != 0 )
        vty_out(vty, "\r\n");
    else
    {
        if( over_thr > 0 )
            vty_out( vty, "\r\n Not find over threshold onu\r\n" );
	 else
	     vty_out( vty, "\r\n Not find online onu\r\n" );
    }
    
    return( ROK );
}

DEFUN  (
    onu_mac_check_threshold,
    onu_mac_check_threshold_cmd,
    "config onu-mac-check threshold <1-10000>",
    "Config system's setting\n"
    "set onu mac check threshold\n"
    "max threshold \n"
    "threshold value,default threshold 128\n"  /*问题单14774*/
    )
{      
       ULONG  sNum;
       short int ret=VOS_ERROR;
        
    sNum = VOS_AtoI( argv[0] );
   
    if(argc==1)
    {
        ret=OLT_SetOnuMacThreshold(-1,sNum);
        if(ret!=0)
            vty_out(vty,"set onu mac threshold error!ret=%d\r\n",ret);
              
    }   
    return CMD_SUCCESS;
}

DEFUN  (
    onu_mac_check_period,
    onu_mac_check_period_cmd,
    "config onu-mac-check period <10-1000000>",
    "Config system's setting\n"
    "set onu mac check interval\n"
    "onu mac check interval\n"
    "onu mac check interval value,unit second.default value is 120s\n"
    )
{      
    LONG  period;
    period = VOS_AtoI( argv[0] );
    if( (period < 10) || (period > 1000000) )
        period = 120;

    period = period * 1000;
    /*if( OnuMacCheckTimerId )
    {
        if( VOS_TimerChange( MODULE_ONU, OnuMacCheckTimerId, period) == VOS_ERROR )
            return CMD_WARNING;
    }
    OnuMacTable_check_interval = period;*/
    OLT_SetOnuMacCheckPeriod(OLT_ID_ALL, period);

    return CMD_SUCCESS;
}

DEFUN  (
    show_onu_mac_check_config,
    show_onu_mac_check_config_cmd,
    "show onu-mac-check config",
     DescStringCommonShow
    "show onu mac check configure\n"
    "show onu mac check configure\n"
    )
{
	vty_out( vty, "onu mac check: " );
	if( mac_check_exit_flag == 1 )
	{
		vty_out(vty,"enable\r\n");
		vty_out(vty,"onu mac statistic alarm threshold: %d\r\n",Onu_user_max_all);
		vty_out(vty,"onu mac check period: %d second\r\n", OnuMacTable_check_interval / 1000 );
	}
	else
		vty_out(vty,"disable\r\n");
	
	return CMD_SUCCESS;  
}

DEFUN  (
    show_onu_mac_check_statistic,
    show_onu_mac_check_statistic_cmd,
    "show onu-mac-check statistic {[over-threshold]}*1",
    DescStringCommonShow
    "show onu learning mac statistic\n"
    "show onu learning mac statistic\n"
    "show onu list over threshold\n"
    )
{
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    short int sOltId;
    ULONG sNumLimit;
		
    if ( PON_PORT_NODE == vty->node )
    {
    	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &sOltId) != VOS_OK )
    		return CMD_WARNING;    	

    	if( PonPortIsWorking(sOltId) == FALSE ) 
		{
    		vty_out(vty, "\r\n  pon%d/%d not working\r\n",slotId,port);
    		return( ROK );
		}
    }
    else if ( CONFIG_NODE == vty->node )
    {
        sOltId = OLT_ID_ALL;
    }
    else
    {
        VOS_ASSERT(0);
    }
    
	if( argc == 0 )
	{
	    sNumLimit = 0;
	}
	else
        {
	    sNumLimit = onu_mac_check_alarm_threshold_get();
	 }

        ShowOnuMacLearningCounterListByVty(sOltId,sNumLimit,vty);
    
        return CMD_SUCCESS;
    
}

DEFUN(
        onu_mac_check_enable,
        onu_mac_check_enable_cmd,
        "config onu-mac-check [enable|disable]",
        "Config system's setting\n"
        "set onu mac check\n"
        "Enable\n"
        "Disable\n"
)
{
    short int iret=VOS_ERROR;
    ULONG  en;
    /*INT16 phyPonId = 0;
    ULONG slot;*/
    
        if(VOS_StriCmp(argv[0], "enable")==0)
        {
            en=1;
        }
        else
        {
            en=0;
        }
    
         iret=OLT_SetOnuMacCheckEnable(OLT_ID_ALL,en);                
         /*for(slot=1;slot<SYS_CHASSIS_SWITCH_SLOTNUM;slot++)
         {
               phyPonId = GetPonPortIdxBySlot(slot, FIRSTPONPORTPERCARD);
         }*/

        if(iret != VOS_OK)
         {
                vty_out(vty," onu mac check %s error!\r\n", (en==1 ? "enable" : "disable"));
         }
        
    return CMD_SUCCESS;
}

/*DEFUN(
            show_onu_mac_check_enable,
            show_onu_mac_check_enable_cmd,
            "show onu-mac-check-enable",
            DescStringCommonShow
            "show onu-mac-check is open or close\n"
)
{    
    if(mac_check_exit_flag==1)
        vty_out(vty,"onu mac check is open!\r\n");
    else
        vty_out(vty,"onu mac check is close!\r\n");

    return CMD_SUCCESS;
}*/

LONG onuMAC_check_showrun( struct vty * vty )
{
	if( mac_check_exit_flag == 1 )
	{
		vty_out( vty, "\r\n" );
		if( Onu_user_max_all != DEFAULT_ONU_MAC_THRESHOLD )
		{
		    vty_out(vty," config onu-mac-check threshold %d\r\n", Onu_user_max_all );
		}
		if( OnuMacTable_check_interval != 120000 )
                  vty_out( vty, " config onu-mac-check period %d\r\n", OnuMacTable_check_interval / 1000 );
		
		vty_out(vty," config onu-mac-check enable\r\n");
	}
	return VOS_OK;
}

LONG onuMAC_check_init()
{
	VOS_MemZero( (void*)onu_mac_check_counter, sizeof(onu_mac_check_counter) );
	VOS_MemZero( (void*)onu_mac_check_alarm_state, sizeof(onu_mac_check_alarm_state) );
	return onuMAC_chk_cli_cmd_install();
}

LONG onuMAC_chk_cli_cmd_install()
{

        install_element ( CONFIG_NODE, &onu_mac_check_threshold_cmd );
        install_element ( CONFIG_NODE, &show_onu_mac_check_config_cmd );
        install_element ( CONFIG_NODE, &show_onu_mac_check_statistic_cmd ); 
        install_element ( PON_PORT_NODE, &show_onu_mac_check_statistic_cmd ); 
        install_element(CONFIG_NODE,&onu_mac_check_enable_cmd);
        install_element(CONFIG_NODE,&onu_mac_check_period_cmd);
        /*install_element(CONFIG_NODE,&show_onu_mac_check_enable_cmd);*/
        
	return VOS_OK;
}

#endif    /*EPON_MODULE_ONU_MAC_OVERFLOW_CHECK*/


#ifdef	__cplusplus
}
#endif/* __cplusplus */
