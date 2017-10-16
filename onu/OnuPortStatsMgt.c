/*********************************************************
	* filename : OnuPortStatsMgt.c
	* Copyright (c) 2014.5  GW Technologies Co., LTD.
	
	* All rights reserved.
	* function: manage ctc ONU ports' statistics data.
	*
**********************************************************/
#include "OnuPortStatsMgt.h"
#include "PonGeneral.h"
#include "Mib_if_ctc.h"
#include "Vos_string.h"
#include "lib_gwEponMib.h"


/*ONU  端口统计管理的全局表*/
static OnuPortStatsMgt_ST  *s_OnuPortStatsTable = NULL; 

static LONG s_OnuPortStatsMgtTimerID = 0;
static LONG s_OnuPortStatsMgtEnable = 0;
static ULONG s_OnuPortStatsEnable = 0;
static ULONG s_OnuPortStatsGetDataTimeOut = ONU_GETDATA_TIMER_INTERVAL;
static ULONG s_OnuPortStatsWakeUpTimerOut = ONU_WAKEUP_TIMER_INTERVAL;

LONG Onustats_EnableIs()
{
	return s_OnuPortStatsMgtEnable;
}

static int Onustats_GetOnuPortData(ULONG PonPortIdx, USHORT OnuIdx)
{	
	ULONG i = 0, devidx = 0, OnuPortNum = 0;	
	int slot = 0,PonPort = 0;
	CTC_STACK_statistic_data_t data;
	int enable = 0;
	int iRet;
/*	ULONG start , stop ;*/
		
	CHECK_PON_RANGE
	CHECK_ONU_RANGE
	
	if(s_OnuPortStatsTable == NULL)
		return VOS_ERROR;

	if((slot = GetCardIdxByPonChip(PonPortIdx)) == RERROR)
	{
		return VOS_ERROR;
	}
	
	if((PonPort = GetPonPortByPonChip(PonPortIdx))== RERROR)
	{
		return VOS_ERROR;		
	}

	devidx = MAKEDEVID(slot, PonPort, OnuIdx+1);

	if( getDeviceCapEthPortNum(devidx,&OnuPortNum) != VOS_OK)
	{
		return VOS_ERROR;		
	}

	/*start = VOS_GetTick();*/
		
	for(i = 0; i < OnuPortNum; i++)
	{
		if((iRet = OnuMgt_GetEthPortLinkState(PonPortIdx, OnuIdx, i+1, &enable)) == VOS_OK && enable == TRUE)
		{	
			if(VOS_OK == OnuMgt_GetOnuPortStatisticData(PonPortIdx, OnuIdx, i+1, &data))	
			{	
				VOS_SemTake(s_OnuPortStatsTable[PonPortIdx].SemId, WAIT_FOREVER);
				s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].StatsPerOnuPort[i].SendStreamOctets = data.downStreamOctets;
				s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].StatsPerOnuPort[i].ReceiveStreamOctets = data.upStreamOctets;
				VOS_SemGive(s_OnuPortStatsTable[PonPortIdx].SemId);				
			}
			else
			{
				s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].WakeUpTimeOut = s_OnuPortStatsWakeUpTimerOut;
				s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].OnuRunningState = ONU_RUNNING_STATE_CRASH;
				s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].OnuCrashTimes++;
				VOS_SemTake(s_OnuPortStatsTable[PonPortIdx].SemId, WAIT_FOREVER);
				s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].StatsPerOnuPort[i].ReceiveStreamOctets.msb = 0;	
				s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].StatsPerOnuPort[i].SendStreamOctets.msb = 0;	
				s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].StatsPerOnuPort[i].ReceiveStreamOctets.lsb = 0;	
				s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].StatsPerOnuPort[i].SendStreamOctets.lsb = 0;	
				VOS_SemGive(s_OnuPortStatsTable[PonPortIdx].SemId);				
				break;
			}
		}
		else
		if(iRet != VOS_OK)
		{
			s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].WakeUpTimeOut = s_OnuPortStatsWakeUpTimerOut;
			s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].OnuRunningState = ONU_RUNNING_STATE_CRASH;
			s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].OnuCrashTimes++;
			VOS_SemTake(s_OnuPortStatsTable[PonPortIdx].SemId, WAIT_FOREVER);
			s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].StatsPerOnuPort[i].ReceiveStreamOctets.msb = 0;	
			s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].StatsPerOnuPort[i].SendStreamOctets.msb = 0;	
			s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].StatsPerOnuPort[i].ReceiveStreamOctets.lsb = 0;	
			s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].StatsPerOnuPort[i].SendStreamOctets.lsb = 0;	
			VOS_SemGive(s_OnuPortStatsTable[PonPortIdx].SemId);				
			break;

		}
	}

	/*stop = VOS_GetTick();
	sys_console_printf("******* OnuIdx %d,time %d\r\n",OnuIdx,stop - start);*/
	
	return VOS_OK;
}


DECLARE_VOS_TASK(task_OnuPortStats)
{
	ULONG PonPortIdx = ulArg1;
	LONG lRet = 0;
	ULONG aulMsg[4] = {0};
	USHORT i = 0;
	int type = 0;
	UCHAR GetDataTimeOutFlag = 0,WakeUpTimeOutFlag = 0; 
/*	ULONG start , stop ;*/

	CHECK_PON_RANGE

	VOS_QueBindTask(( VOS_HANDLE )s_OnuPortStatsTable[PonPortIdx].TaskId,s_OnuPortStatsTable[PonPortIdx].QueueId);	

	while(1)
	{
		lRet = VOS_QueReceive(s_OnuPortStatsTable[PonPortIdx].QueueId, aulMsg, WAIT_FOREVER);
        if(lRet != VOS_HAVE_MSG)
        {
        	VOS_TaskDelay(20);
            continue;
        }
	
		if(!s_OnuPortStatsEnable)
		{
			VOS_TaskDelay(20);
            continue;
		}

		if(aulMsg[ 2 ] == MSG_ONU_PORT_STATS_TIMER)
		{
			s_OnuPortStatsTable[PonPortIdx].GetDataTimeOut--;
			
			if(s_OnuPortStatsTable[PonPortIdx].GetDataTimeOut == 0)
			{
				GetDataTimeOutFlag = TRUE;
				s_OnuPortStatsTable[PonPortIdx].GetDataTimeOut = s_OnuPortStatsGetDataTimeOut;
			}

			for(i = 0; i < MAXONUPERPON;i++)
			{
				if(s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[i].OnuRunningState == ONU_RUNNING_STATE_CRASH)
				{
					s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[i].WakeUpTimeOut--;

					if(s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[i].WakeUpTimeOut == 0)
					{
						WakeUpTimeOutFlag = TRUE;
						s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[i].OnuRunningState = ONU_RUNNING_STATE_NORMAL;
					}
				}
			}		

			if(GetDataTimeOutFlag || WakeUpTimeOutFlag)
			{
				if(GetPonPortWorkStatus(PonPortIdx) == PONPORT_UP)
				{
				/*	start = VOS_GetTick();*/
					for(i = 0; i < MAXONUPERPON; i++)
					{
						if(GetOnuOperStatus(PonPortIdx, i) == ONU_OPER_STATUS_UP
							&&(GetOnuType(PonPortIdx, i, &type) == VOS_OK && (type == V2R1_ONU_CTC ))
								&&(snmp_onu_statistics_ability_check(PonPortIdx, i) == VOS_OK )    
									&&(s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[i].OnuRunningState == ONU_RUNNING_STATE_NORMAL))
								
					   {
							Onustats_GetOnuPortData(PonPortIdx,i);
					   }
					   else
					   {
							/* Do nothing*/
					   }
					}
				/*	stop = VOS_GetTick();

					sys_console_printf("########## PonPortIdx %d,time %d\r\n",PonPortIdx,stop - start);*/
				}

				GetDataTimeOutFlag = FALSE; 
				WakeUpTimeOutFlag = FALSE;
			}
		}
    }
	
	return;
}

int CTCONU_Onustats_GetOnuPortDataByID(USHORT PonPortIdx, USHORT OnuIdx, USHORT port,OnuPortStats_ST *data)
{
	CHECK_PON_RANGE
	CHECK_ONU_RANGE
	
	if(s_OnuPortStatsTable == NULL || data == NULL || port < 1 || port > MAX_PORT_PER_ONU 
		|| s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].OnuRunningState == ONU_RUNNING_STATE_CRASH)
		return VOS_ERROR;
	

	VOS_SemTake(s_OnuPortStatsTable[PonPortIdx].SemId, WAIT_FOREVER);

	*data = s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[OnuIdx].StatsPerOnuPort[port - 1];

	VOS_SemGive(s_OnuPortStatsTable[PonPortIdx].SemId);				

	return VOS_OK;

}

int Onustats_GetPortStatsTimeOut(ONU_PORTSTATS_TIMER_NAME_E timer_name, ULONG *timeout)
{

	if(timeout != NULL)
	{
		if(timer_name == ONU_GETDATA_TIMER)
		{
			*timeout = s_OnuPortStatsGetDataTimeOut ;
		}
		else
		if(timer_name == ONU_WAKEUP_TIMER)
		{
			*timeout = s_OnuPortStatsWakeUpTimerOut ;
		}
		else
		if(timer_name == ONU_PORTSTATS_TASKSTATUS)
		{
			*timeout = s_OnuPortStatsEnable;
		}
		else
		if(timer_name == ONU_PORTSTATS_ENABLE)
		{
			*timeout = s_OnuPortStatsMgtEnable;
		}
	}
	else
		return	RERROR;
	
	return( ROK );
}

int Onustats_SetPortStatsTimeOut(short int PonPortIdx, ONU_PORTSTATS_TIMER_NAME_E timer_name,ULONG timeout)
{
	if(timer_name == ONU_GETDATA_TIMER)
	{
		s_OnuPortStatsGetDataTimeOut = timeout;
	}
	else
	if(timer_name == ONU_WAKEUP_TIMER)
	{
		s_OnuPortStatsWakeUpTimerOut = timeout;
	}
	else
	if(timer_name == ONU_PORTSTATS_TASKSTATUS)
	{
		s_OnuPortStatsEnable = timeout;
	}
	else
	if(timer_name == ONU_PORTSTATS_ENABLE)
	{
		s_OnuPortStatsMgtEnable = timeout;
	}
		
	return VOS_OK;
}

static LONG Onustats_SendInternelMsg( ULONG ulMsgType, VOID * pData)
{
	ULONG aulMsg [ 4 ],i = 0;
    LONG lPriority = MSG_PRI_NORMAL ;	
	LONG lRet = 0;

    aulMsg [ 0 ] = MODULE_RPU_ONU;
    aulMsg [ 1 ] = MODULE_RPU_ONU;
    aulMsg [ 2 ] = ulMsgType;
    aulMsg [ 3 ] = (ULONG)pData;

	for(i = 0; i < aulMsg [ 3 ];i++)
	{
		if(VOS_QueNum(s_OnuPortStatsTable[i].QueueId) == 0)
		{
			lRet =  VOS_QueSend( ( ULONG ) s_OnuPortStatsTable[i].QueueId, aulMsg, NO_WAIT, lPriority );
			if(lRet != VOS_OK)
			{
				VOS_ASSERT(0);
			}
		}
	}

	return VOS_OK;
	
}
static VOID Onustats_TimerTick( VOID * pValue )
{
	Onustats_SendInternelMsg(MSG_ONU_PORT_STATS_TIMER,pValue);
	
	return ;
}

static LONG Onustats_Free(void)
{
	USHORT i,j;
	USHORT MAXPONNUM = 0;

	if((SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER) && s_OnuPortStatsTable != NULL)
	{
		if(/*SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER*/0)
		{
			MAXPONNUM = MAXPON; 
		}
		else
		{
			MAXPONNUM = MAXPONPORT_PER_BOARD;
		}

		VOS_TimerDelete(MODULE_RPU_ONU,s_OnuPortStatsMgtTimerID);

		s_OnuPortStatsMgtTimerID = 0;

	    for(i = 0; i < MAXPONNUM; i++)
	    {
	    	if(s_OnuPortStatsTable[i].OnuStatsPerPonPort != NULL)
			{
				for(j = 0;j < MAXONUPERPON; j++)
				{
					if(s_OnuPortStatsTable[i].OnuStatsPerPonPort[j].StatsPerOnuPort != NULL)
					{	
						VOS_Free(s_OnuPortStatsTable[i].OnuStatsPerPonPort[j].StatsPerOnuPort);
						s_OnuPortStatsTable[i].OnuStatsPerPonPort[j].StatsPerOnuPort = NULL;
	
					}
				}

				VOS_Free(s_OnuPortStatsTable[i].OnuStatsPerPonPort);

				s_OnuPortStatsTable[i].OnuStatsPerPonPort = NULL;
	    	}

			if(s_OnuPortStatsTable[i].TaskId)
    	 		VOS_TaskDelete(s_OnuPortStatsTable[i].TaskId);
				
		   	if(s_OnuPortStatsTable[i].SemId)
           		VOS_SemDelete(s_OnuPortStatsTable[i].SemId);

			if(s_OnuPortStatsTable[i].QueueId)
    	  		VOS_QueDelete(s_OnuPortStatsTable[i].QueueId);
	
		}

		VOS_Free(s_OnuPortStatsTable);

		s_OnuPortStatsTable = NULL;

		s_OnuPortStatsMgtEnable = 0;

		s_OnuPortStatsGetDataTimeOut = ONU_GETDATA_TIMER_INTERVAL;

		s_OnuPortStatsWakeUpTimerOut = ONU_WAKEUP_TIMER_INTERVAL;
			
		return VOS_OK;
	}

	return VOS_ERROR;
}

LONG Onustats_Init(void)
{
	ULONG i,j;
	ULONG MaxPonNum = 0;
	char szName[80] ="";
	ULONG ulArg[10] = {0};

	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{
	
		if(/*SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER*/0)
		{
			MaxPonNum = MAXPON; 
		}
		else
		{
			MaxPonNum = MAXPONPORT_PER_BOARD;
		}

		if((s_OnuPortStatsTable = VOS_Malloc(sizeof(OnuPortStatsMgt_ST)*MaxPonNum,MODULE_RPU_ONU)) == NULL)
		{
			VOS_ASSERT( 0 );
			goto FUN_ERR;
		}
	
		VOS_MemZero(s_OnuPortStatsTable,sizeof(OnuPortStatsMgt_ST)*MaxPonNum);

		
	    for(i = 0; i < MaxPonNum; i++)
	    {	
			VOS_MemZero(szName,sizeof(szName));
			
    		sprintf(szName, "ONUPortStats%d", i+1);

			ulArg[0] = i;

			if((s_OnuPortStatsTable[i].OnuStatsPerPonPort = VOS_Malloc(sizeof(OnuDevStats_ST)*MAXONUPERPON,MODULE_RPU_ONU)) == NULL)
			{
				VOS_ASSERT( 0 );
				goto FUN_ERR;
			}

			VOS_MemZero(s_OnuPortStatsTable[i].OnuStatsPerPonPort,sizeof(OnuDevStats_ST)*MAXONUPERPON);
			
			for(j = 0;j < MAXONUPERPON; j++)
			{
				if((s_OnuPortStatsTable[i].OnuStatsPerPonPort[j].StatsPerOnuPort = VOS_Malloc(sizeof(OnuPortStats_ST)*MAX_PORT_PER_ONU,MODULE_RPU_ONU)) == NULL)
				{	
					VOS_ASSERT( 0 );
					goto FUN_ERR;
				}

				VOS_MemZero(s_OnuPortStatsTable[i].OnuStatsPerPonPort[j].StatsPerOnuPort,sizeof(OnuPortStats_ST)*MAX_PORT_PER_ONU);
			}
			
			if((s_OnuPortStatsTable[i].SemId = VOS_SemBCreate(VOS_SEM_Q_FIFO,VOS_SEM_FULL)) == NULL)
			{
				VOS_ASSERT( 0 );
				goto FUN_ERR;
			}

			if((s_OnuPortStatsTable[i].QueueId = VOS_QueCreate(ONU_PORT_STATS_MAX_QUEUE_NUM,VOS_MSG_Q_FIFO)) == NULL)
			{
				VOS_ASSERT( 0 );
				goto FUN_ERR;
			}

			if((s_OnuPortStatsTable[i].TaskId = VOS_TaskCreateEx(szName,task_OnuPortStats, TASK_PRIORITY_NORMAL, (64*1024), ulArg))==NULL)
			{
				VOS_ASSERT( 0 );
				goto FUN_ERR;
			}
			
			s_OnuPortStatsTable[i].GetDataTimeOut = s_OnuPortStatsGetDataTimeOut;
   		
		}

		if((s_OnuPortStatsMgtTimerID = VOS_TimerCreate(MODULE_RPU_ONU,0,ONU_PORT_STATS_TIMER_INTERVAL,Onustats_TimerTick,(VOID *)MaxPonNum,VOS_TIMER_LOOP))==VOS_ERROR)
		{
			VOS_ASSERT( 0 );
			goto FUN_ERR;
		}
		
		return VOS_OK;
	}

	return VOS_ERROR;

FUN_ERR:
	Onustats_Free();
	return VOS_ERROR;
}

DEFUN(onu_port_statistics_ctc,
        onu_port_statistics_ctc_cmd,
        "ctc-onu port-stats [enable|disable]",
        "ctc onu config\n"
        "ctc onu port statistics\n"
        "Enable ctc onu port statistics\n"
        "Disable ctc onu port statistics\n")
{
	ULONG EnableStatus = VOS_StriCmp(argv[0], "disable")?1:0;

	OLT_SetCTCOnuPortStatsTimeOut(OLT_ID_ALL,ONU_PORTSTATS_ENABLE,EnableStatus);
	
	return VOS_OK;
}

DEFUN ( onu_port_stats_get_data_timeout,
        onu_port_stats_get_data_timeout_cmd,
        "ctc-onu port-stats data-timeout <1-300>",
        "ctc onu config\n"
        "ctc onu port statistics\n"
        "Set onu statistic data interval\n"
        "Timeout value in seconds(default 60s)\n" )
{
	ULONG GetDataTimeOut = VOS_AtoL( ( CHAR* ) argv[ 0 ] );

	OLT_SetCTCOnuPortStatsTimeOut(OLT_ID_ALL,ONU_GETDATA_TIMER,GetDataTimeOut);
	
	return VOS_OK;
}

DEFUN ( onu_port_stats_wake_up_timeout,
        onu_port_stats_wake_up_timeout_cmd,
        "ctc-onu port-stats wakeup-timeout <15-30>",
        "ctc onu config\n"
        "ctc onu port statistics\n"
        "Set onu wake-up interval\n"
        "Timeout value in minutes(default 20min)\n" )
{
	                 
	ULONG WakeUpTimeOut = VOS_AtoL( ( CHAR* ) argv[ 0 ] );

	WakeUpTimeOut = WakeUpTimeOut*60;
	
	OLT_SetCTCOnuPortStatsTimeOut(OLT_ID_ALL,ONU_WAKEUP_TIMER,WakeUpTimeOut);
		
	return VOS_OK;
}

DEFUN ( onu_port_stats_get_data_timeout_default,
        onu_port_stats_get_data_timeout_default_cmd,
        "undo ctc-onu port-stats data-timeout",
        "Negate a command or set its defaults\n"
        "ctc onu config\n"
        "ctc onu port statistics\n"
        "Set onu statistic data interval\n"
        )
{
	OLT_SetCTCOnuPortStatsTimeOut(OLT_ID_ALL,ONU_GETDATA_TIMER,ONU_GETDATA_TIMER_INTERVAL);
	
	return VOS_OK;
}

DEFUN (onu_port_stats_wake_up_timeout_default,
        onu_port_stats_wake_up_timeout_default_cmd,
        "undo ctc-onu port-stats wakeup-timeout",
        "Negate a command or set its defaults\n"
        "ctc onu config\n"
        "ctc onu port statistics\n"
        "Set onu wake-up interval\n"
        )
{
	                 
	OLT_SetCTCOnuPortStatsTimeOut(OLT_ID_ALL,ONU_WAKEUP_TIMER,ONU_WAKEUP_TIMER_INTERVAL);
		
	return VOS_OK;
}

DEFUN ( onu_port_stats_get_data_timeout_show,
        onu_port_stats_get_data_timeout_show_cmd,
        "show ctc-onu port-stats data-timeout",
        SHOW_STR
        "ctc onu config\n"
        "ctc onu port statistics\n"
        "Onu statistic data interval\n"
        )
{
	ULONG  timeout = 0;
	
	Onustats_GetPortStatsTimeOut(ONU_GETDATA_TIMER,&timeout);

	vty_out(vty,"Get onu statistic data interval %d seconds\r\n",timeout);
	
	return VOS_OK;
}

DEFUN (onu_port_stats_wake_up_timeout_show,
        onu_port_stats_wake_up_timeout_show_cmd,
        "show ctc-onu port-stats wakeup-timeout",
 		 SHOW_STR
        "ctc onu config\n"
        "ctc onu port statistics\n"
        "Onu wake-up interval\n"
        )
{
	                 
	ULONG timeout = 0;
	
	Onustats_GetPortStatsTimeOut(ONU_WAKEUP_TIMER,&timeout);

	vty_out(vty,"Wake up dead onu interval %d minutes\r\n",timeout/60);
			
	return VOS_OK;
}

DEFUN ( one_ctc_onu_statistic_data_mode_show,
        one_ctc_onu_statistic_data_mode_show_cmd,
        "show ctc-onu statistic <slot/port/onuid>",
        SHOW_STR
        "ctc-onu statistic\n"
        "Statistic data\n"
        "Specify onu interface's onuid\n"
        )
{
	ULONG PonPortIdx = 0,k = 0;
	ULONG ulSlot = 0;
    ULONG ulOnuid = 0; 
    ULONG ulPort = 0;
	LONG lRet = 0; 
	ULONG  devidx = 0, OnuPortNum = 0;	
	ULONG TaskEnable;
	int enable;
	OnuPortStats_ST PortData;
	
	if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER)
	{
		if(Onustats_EnableIs() == TRUE && (Onustats_GetPortStatsTimeOut(ONU_PORTSTATS_TASKSTATUS,&TaskEnable) == ROK) && TaskEnable == 1)
		{
			lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid);
			if( lRet != VOS_OK )
				return CMD_WARNING;
				
			PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
			
			if(GetOnuOperStatus(PonPortIdx, ulOnuid-1) == ONU_OPER_STATUS_UP)
			{
				vty_out(vty,"ONU operation status is up.\r\n");
			}
			else
			{
				vty_out(vty,"ONU operation status is down.\r\n");
			}
		
			vty_out(vty,"***********************************************************************\r\n");
			
			devidx = MAKEDEVID(ulSlot, ulPort, ulOnuid);

			if( getDeviceCapEthPortNum(devidx,&OnuPortNum) != VOS_OK)
			{
				return VOS_ERROR;		
			}
			
			vty_out(vty,"%-13s%-16s%-22s%-20s\r\n","ONU-Port","LinkStatus","Send octets","Receive octets");

			#define ONU_STATS_OUT_FORMAT "%-13d%-16s0x%08x%08x%4s0x%08x%08x\r\n" 
		
			for(k = 0 ; k <OnuPortNum; k++)
			{
				enable = 0;
				VOS_MemZero(&PortData,sizeof(PortData));
				
				lRet = OnuMgt_GetEthPortLinkState(PonPortIdx, ulOnuid-1, k+1, &enable);
				
				if(VOS_OK == OnuMgt_GetCTCOnuPortStatsData(PonPortIdx, ulOnuid-1, k+1, &PortData))
				{
					vty_out(vty,ONU_STATS_OUT_FORMAT,k+1,((lRet == VOS_OK)&&(enable == TRUE))?"Up":"Down",PortData.SendStreamOctets.msb,
							PortData.SendStreamOctets.lsb, " ",PortData.ReceiveStreamOctets.msb,PortData.ReceiveStreamOctets.lsb);
			
				}

			}
			
		}
	}
	else
	{
		if(Onustats_EnableIs() == TRUE && (Onustats_GetPortStatsTimeOut(ONU_PORTSTATS_TASKSTATUS,&TaskEnable) == ROK) && TaskEnable == 1)
		{
			lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid);
			if( lRet != VOS_OK )
				return CMD_WARNING;
				
			if(s_OnuPortStatsTable != NULL)
			{
				PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
				if(GetOnuOperStatus(PonPortIdx, ulOnuid-1) == ONU_OPER_STATUS_UP)
				{
					vty_out(vty,"ONU operation status is up.\r\n");
				}
				else
				{
					vty_out(vty,"ONU operation status is down.\r\n");
				}

				vty_out(vty,"This ONU's current running state is %s.\r\n",(s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[ulOnuid-1].OnuRunningState)?"crashed":"normal");
				
				vty_out(vty,"So far this ONU has crashed %d times.\r\n",s_OnuPortStatsTable[PonPortIdx].OnuStatsPerPonPort[ulOnuid-1].OnuCrashTimes);
				
				vty_out(vty,"***********************************************************************\r\n");

				devidx = MAKEDEVID(ulSlot, ulPort, ulOnuid);

				if( getDeviceCapEthPortNum(devidx,&OnuPortNum) != VOS_OK)
				{
					return VOS_ERROR;		
				}
				
				vty_out(vty,"%-13s%-16s%-22s%-20s\r\n","ONU-Port","LinkStatus","Send octets","Receive octets");

				#define ONU_STATS_OUT_FORMAT "%-13d%-16s0x%08x%08x%4s0x%08x%08x\r\n" 
			
				for(k = 0 ; k <OnuPortNum; k++)
				{
					enable = 0;
					VOS_MemZero(&PortData,sizeof(PortData));
					
					lRet = OnuMgt_GetEthPortLinkState(PonPortIdx, ulOnuid-1, k+1, &enable);

					if(VOS_OK == OnuMgt_GetCTCOnuPortStatsData(PonPortIdx, ulOnuid-1, k+1, &PortData))
					{
						vty_out(vty,ONU_STATS_OUT_FORMAT,k+1,((lRet == VOS_OK)&&(enable == TRUE))?"Up":"Down",PortData.SendStreamOctets.msb,
								PortData.SendStreamOctets.lsb, " ",PortData.ReceiveStreamOctets.msb,PortData.ReceiveStreamOctets.lsb);
				
					}
					
				}
			}
		}	
	}
	
	return VOS_OK;
}

DEFUN (onu_port_stats_task_start_stop,
		 onu_port_stats_task_start_stop_cmd,
        "ctc-onu port-stats [task-stop|task-start]",
        "ctc onu config\n"
        "ctc onu port statistics\n"
        "Port-stats task stop work\n"
        "Port-stats task start work\n"
        )
{
	ULONG PortStatsEnable = VOS_StriCmp(argv[0],"task-stop")?1:0;

	OLT_SetCTCOnuPortStatsTimeOut(OLT_ID_ALL,ONU_PORTSTATS_TASKSTATUS,PortStatsEnable);
					 
	return VOS_OK;
}


LONG Onustats_CommandInstall()
{
	install_element ( CONFIG_NODE, &onu_port_statistics_ctc_cmd);
	install_element ( CONFIG_NODE, &onu_port_stats_get_data_timeout_cmd);
	install_element ( CONFIG_NODE, &onu_port_stats_wake_up_timeout_cmd);
	install_element ( CONFIG_NODE, &one_ctc_onu_statistic_data_mode_show_cmd);
	install_element ( CONFIG_NODE, &onu_port_stats_get_data_timeout_default_cmd);
	install_element ( CONFIG_NODE, &onu_port_stats_wake_up_timeout_default_cmd);
	install_element ( CONFIG_NODE, &onu_port_stats_get_data_timeout_show_cmd);
	install_element ( CONFIG_NODE, &onu_port_stats_wake_up_timeout_show_cmd);
	install_element ( CONFIG_NODE, &onu_port_stats_task_start_stop_cmd);

	return VOS_OK;
}


