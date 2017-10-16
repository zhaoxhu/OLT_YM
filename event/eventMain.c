#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

/*#include "Ifm_pub.h"*/
/*#include "cli/cli.h"
#include "sys/main/sys_main.h"*/

#include "lib_gwEponMib.h"
#include "gwEponSys.h"

#include "eventOam.h"
#include "eventTrap.h"
#include "ethLoopChk.h"
#include "backup/syncMain.h"
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
#include "onu/ExtBoardType.h"
#endif
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "E1_MIB.h"
#include "Tdm_apis.h"
#include "Tdm_comm.h"
#endif
#include "Cdp_pub.h"
#include "eventStatus.h"


/*LONG __LED_6700 = 0;*/
/* GPIOx = 31：ALM灯, GPIOx = 30：RUN
bit = 0-亮，1-灭。*/
/*extern int GPIO_UPTDM_OK_LED;
extern int GPIO_UPGE_OK_LED;*/
/*extern int GPIO_RUN_LED;
extern int GPIO_ALM_LED;*/

/*#define LED_ALM_Light()		if(__LED_6700) write_gpio( GPIO_ALM_LED, LED_LIGHT_VAL )
#define LED_ALM_Dark()		if(__LED_6700) write_gpio( GPIO_ALM_LED, LED_DARK_VAL )
#define LED_RUN_Light()		if(__LED_6700) write_gpio( GPIO_RUN_LED, LED_LIGHT_VAL )
#define LED_RUN_Dark()		if(__LED_6700) write_gpio( GPIO_RUN_LED, LED_DARK_VAL )

#define LED_UPTDM_OK_Light()	if(__LED_6700) write_gpio( GPIO_UPTDM_OK_LED, LED_LIGHT_VAL )
#define LED_UPTDM_OK_Dark()	if(__LED_6700) write_gpio( GPIO_UPTDM_OK_LED, LED_DARK_VAL )
#define LED_UPGE_OK_Light()		if(__LED_6700) write_gpio( GPIO_UPGE_OK_LED, LED_LIGHT_VAL )
#define LED_UPGE_OK_Dark()		if(__LED_6700) write_gpio( GPIO_UPGE_OK_LED, LED_DARK_VAL )*/


#ifdef OLT_PWU_OFF_TRAP
#define IS_OLT_DEV_PWU_OFF(X)  ( (X->alarmType == alarmType_private) && (X->alarmId == trap_devPowerOff) && (X->alarmSrc.devAlarmSrc.devIdx == 1) )
#else
#define IS_OLT_DEV_PWU_OFF(X)  FALSE
#endif

ULONG eventDebugSwitch = EVENT_DEBUGSWITCH_NONE;
/*ULONG eventDebugSwitch_bak = EVENT_DEBUGSWITCH_NONE;*/
ULONG eventLogOutMode = EVENT_LOG_OUT_2_CONSOLE;
ULONG eventLogOutMode_bak = EVENT_LOG_OUT_2_CONSOLE;

void eventProcTask();
void eventTimerCallback();
STATUS  GetE1AlarmLevel_sig(ULONG  devIdx,ULONG  *level);
STATUS  GetE1AlarmLevel_ONU_E1(ULONG  devIdx,ULONG  *level);

ULONG EVENT_QUEUE_LENGTH = 512;
ULONG eventQueId = 0;
VOS_HANDLE eventTaskId = NULL;
LONG eventTimerId =0;
ULONG eventSemId = 0;
extern unsigned long  OptOnuAlarmMask;

/*int alarmLedProc( ULONG runLenFlag );*/
int eventAlarmProc( eventMsg_t *pAlmMsg  );
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
STATUS tdmAlarmInit();
#endif
extern LONG initAlarmTrapBac();
extern LONG initEventLogTbl();
/*extern LONG event_cli_cmd_install();*/
extern LONG saveTrapBacData( ULONG alarmType, ULONG trapId, ULONG *pVarlist, ULONG varNum,	alarmSrc_t *pAlarmSrc );
extern int getEthPortAlarmMask( ULONG devIdx, ULONG brdIdx, ULONG ethIdx, ULONG *mask );
/*extern void retrieveCtcCfgData( void );
extern void saveCtcCfgData( void );*/
extern LONG swport_2_slot_port(int sw_port, ULONG *ulSlot, ULONG *ulPort);

extern STATUS config_sync_notify_event();
/*extern VOID emac_check_mal_bd_tx_status();*/

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
extern ushort_t   tdm_MaskBase;
extern STATUS  epon_tdm_cfgdata_save();
extern ULONG get_gfa_tdm_slotno();
extern STATUS epon_tdm_cfgdata_retrieve( );
#endif
#if( EPON_MODULE_SYS_DIAGNOSE == EPON_MODULE_YES )
extern LONG sys_diag_init();
#endif

/*extern ULONG (*ONU_MASK)[MAXONUPERPON];*/
extern ULONG onu_type_mask[V2R1_ONU_MAX];


VOID (*check_ppc405_emac1_hook_rtn) (VOID) = NULL;

typedef  struct{
	ULONG  devIdx;
	short int  brdIdx;
	short int  onuEventId;

	UCHAR brdData[EUQ_MAX_OAM_PDU];
} __attribute__((packed)) eventSyncOnuBrdData_t;

/*----------------------------------------------------------------------------*/

int eventSync_onuExtHotProc( eventSyncOnuBrdData_t *pSyncData );
int eventSync_cdpRecvProc( eventSyncCfgData_t *pRecvMsg );
int eventOamMsg_recvProc( eventOamMsg_t *pMsgBuf );
int eventSync_alarmReport_2Master( ULONG slotno, eventMsg_t *pAlmMsg );
int e1AlarmMaskCheck( eventMsg_t *pAlarmMsg, ULONG mask );
int eventMaskProc( eventMsg_t *pAlmMsg );
LONG eventBackupProc();

extern VOID initAlmStatus();
extern LONG checkAlarmStatus( eventMsg_t *pAlmMsg );
extern LONG eventStatusProc( eventMsg_t *pAlmMsg );
extern LONG eventStatusClearProc( alm_status_cls_msg_t *pStatusMsg );
extern int SetOnuDevicePowerStatus( ULONG onuDevIdx, LONG status );
extern LONG getOnuTypeAlarmMaskByDevIdx( ULONG devIdx, ULONG *pMask );
extern LONG event_module_init();
#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
extern STATUS alarm_Environment_Init();
#endif
#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
extern VOID initPowerAlarm();
#endif	
#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
extern STATUS powerBrdPowerOnAndOffAlarm_Command_Install();
#endif

#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES)
extern LONG onuAutoLoadOnuNewRegisterCallback( ULONG onuDevIdx );
#endif
#if(EPON_MODULE_ONU_MAC_OVERFLOW_CHECK==EPON_MODULE_YES)
extern LONG setOnuMacCheckAlarmStateByDevIdx( ULONG onuDevIdx, ULONG state );
#endif

extern ULONG eventLogFlashFileFlag;
extern ULONG eventLogFlashFileEnable;
extern ULONG eventLogFlashFilePeriod;
extern ULONG eventLogFlashFileTimer;
extern LONG eventLogFlashFileSave();


/*----------------------------------------------------------------------------*/
/* 告警模块初始化入口 */
int eventProcInit(void)
{
	int rc = VOS_ERROR;
	/*ULONG ulSize;*/

	if( (SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6100) || (SYS_PRODUCT_TYPE ==  PRODUCT_E_EPON3) )
	{
		/*__LED_6700 = 1;
		
		LED_ALM_Dark();*/
		/*EVENT_QUEUE_LENGTH = 512;*/
	}
	else
	{
		if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
			EVENT_QUEUE_LENGTH = 2048;
	}
	
	if( eventQueId == 0 ) 
		eventQueId = VOS_QueCreate( EVENT_QUEUE_LENGTH , VOS_MSG_Q_PRIORITY);
	if( eventQueId  == 0 )
	{
		VOS_ASSERT( 0 );
		return rc;
	}

	if( eventSemId == 0 )
		eventSemId = VOS_SemMCreate(VOS_SEM_Q_FIFO);
		
	eventTaskId = ( VOS_HANDLE )VOS_TaskCreate("tEvent", 57, eventProcTask, NULL );
	if( eventTaskId == NULL )
	{
		VOS_ASSERT( 0 );
		return rc;
	}

	VOS_QueBindTask( eventTaskId, eventQueId );

	eventTimerId = VOS_TimerCreate( MODULE_EVENT, 0, 1000, (void *)eventTimerCallback, NULL, VOS_TIMER_LOOP );
	if( eventTimerId == VOS_ERROR )
	{
		VOS_ASSERT( 0 );
		return rc;
	}

	initEventLogTbl();
	rc = initAlarmTrapBac();
	/*rc = initAlarmOam();*/

	if( VOS_OK != CDP_Create(RPU_TID_CDP_EVENT, CDP_NOTI_VIA_QUEUE, eventQueId, NULL) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
#if 0
    ulSize = sizeof(ULONG) * MAXONU;
    if ( NULL == (ONU_MASK = VOS_Malloc(ulSize, MODULE_EVENT)) )
    {   
        VOS_ASSERT(0);
        return VOS_ERROR;
    }
	VOS_MemZero( ONU_MASK, ulSize );
#endif

	VOS_MemZero( onu_type_mask, sizeof(onu_type_mask) );

	initAlmStatus();

	return rc;
}
/*----------------------------------------------------------------------------*/

void eventProcInit2()
{
#if( EPON_MODULE_SYS_DIAGNOSE == EPON_MODULE_YES )
	sys_diag_init();
#endif

	if ( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
	{
		VOS_TaskLock();
		/*eventDebugSwitch_bak = EVENT_DEBUGSWITCH_NONE;*/
		eventDebugSwitch = EVENT_DEBUGSWITCH_NONE;
		eventLogOutMode = EVENT_LOG_OUT_2_NONE;
		eventLogOutMode_bak = eventLogOutMode;
		VOS_TaskUnlock();
	}

	/*event_cli_cmd_install();*/
	event_module_init();

/*#ifdef V2R1_WATCH_DOG
	   V2R1_init_watchdog();	removed by xieshl 20081231
#endif*/

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	tdmAlarmInit();
#endif
	/*devsm_check_timer_get();*/
	/*devsm_check_timer_set( 2000 );*/

#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
    /*begin: added by jianght 20081112*/
    alarm_Environment_Init();
    /*end: added by jianght 20081112*/
#endif

#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
	/*if(( Product_Type == PRODUCT_E_GFA6100 ) || ( Product_Type == PRODUCT_E_EPON3 ) )*/
	{
    	/* begin: added by jianght 20090525 */
    	initPowerAlarm();
    	/* end: added by jianght 20090525 */
	}
#endif	

	/* begin: added by jianght 20090520 */
#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
	powerBrdPowerOnAndOffAlarm_Command_Install();
#endif
	/* end: added by jianght 20090520 */

	/*ethLoopCheckInit();*/
}

/* 告警处理任务，其消息队列区分优先级，以保证紧急事件优先处理 */
void eventProcTask()
{
	ULONG ulRcvMsg[4];
	long result;
	/*ULONG runLenFlag = 0;*/
	SYS_MSG_S *pMsg;

	while( 1 )
	{
		result = VOS_QueReceive( eventQueId, ulRcvMsg, WAIT_FOREVER );
		if( result == VOS_ERROR )
		{
			VOS_TaskDelay(100);
			ASSERT(0);
			continue;
		}

		pMsg = (SYS_MSG_S *)ulRcvMsg[3];
		if( NULL == pMsg )
		{
			VOS_ASSERT(pMsg);
			VOS_TaskDelay(20);
			continue;
		}

		if ( CDP_NOTI_FLG_SEND_FINISH == ulRcvMsg[1] )
		{
			CDP_FreeMsg( pMsg ); 	   /* cdp 异步发送完成*/
			continue;
		}

		switch( pMsg->usMsgCode )
		{
			case FC_EVENT_TIMER:

				/*if( SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 || SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
				{
					runLenFlag++;
					alarmLedProc( runLenFlag );
				}*/	/* removed by xieshl 20111226, 全部移植到tLed任务 */
				if ( check_ppc405_emac1_hook_rtn )
					(*check_ppc405_emac1_hook_rtn)();

				break;
			case FC_EVENT_REPORT:
				if( checkAlarmStatus((eventMsg_t *)(pMsg->ptrMsgBody)) == VOS_OK )  /* modified by xieshl 20111020, 告警状态已记录*/
				{
					eventStatusProc( (eventMsg_t *)(pMsg->ptrMsgBody) );
					eventAlarmProc( (eventMsg_t *)(pMsg->ptrMsgBody) );
				}
				break;

			case FC_EVENT_BACKUP:	/* modified by xieshl 20130319, 在设备没有nvram时，则定时保存到flash */
				eventBackupProc();
				break;

			case FC_EVENT_ALM_SYNC:
				if( eventMaskProc((eventMsg_t *)(pMsg+1)) != VOS_OK )	/* 问题单11717 */
				{
					eventStatusProc( (eventMsg_t *)(pMsg+1) );
					eventAlarmProc( (eventMsg_t *)(pMsg+1) );
				}
				break;
			case FC_EVENT_DATA_SYNC:
				eventSync_onuExtHotProc( (eventSyncOnuBrdData_t *)(pMsg+1) );
				break;
			case FC_EVENT_CFG_SYNC:
				eventSync_cdpRecvProc( (eventSyncCfgData_t *)(pMsg+1) );
				break;
			case FC_EVENT_OAM_RX:
				eventOamMsg_recvProc( (eventOamMsg_t *)(pMsg->ptrMsgBody) );
				break;
			/* modified by xieshl 20110602, ONU离线或被删除时，告警任务负责清除其所有告警状态 */
			/* 之前在ONU任务中清除时，存在信号量死锁问题，用64个ONU在不同的PON上反复注册可复现 */
			case FC_EVENT_STA_CLS:
				eventStatusClearProc( (alm_status_cls_msg_t *)(pMsg+1) );
				break;
			case AWMC_CLI_BASE:
				decode_command_msg_packet( pMsg, VOS_MSG_TYPE_QUE );
				break;
			default:
				break;
		}
		
		if(SYS_MSG_SRC_SLOT(pMsg) != (SYS_LOCAL_MODULE_SLOTNO))
		{
			CDP_FreeMsg( pMsg );    /*CDP消息释放*/
		}
		else
		{
			VOS_Free(pMsg);         /*本板消息释放(包括PDU消息)*/
		}
		pMsg = NULL;
	}
}
/*----------------------------------------------------------------------------*/

LONG eventTimerMsgSend( ULONG msgCode )
{
	LONG rc = VOS_ERROR;
	ULONG aulMsg[4] = {MODULE_EVENT, 0, 0, 0};
	SYS_MSG_S * pstMsg = NULL;

	aulMsg[1] = msgCode;

	pstMsg = ( SYS_MSG_S* ) VOS_Malloc( sizeof(SYS_MSG_S), MODULE_EVENT );
	if ( NULL == pstMsg )
	{
		return rc;
	}
	VOS_MemZero( pstMsg, sizeof(SYS_MSG_S) );
	pstMsg->ulSrcModuleID = MODULE_EVENT;
	pstMsg->ulDstModuleID = MODULE_EVENT;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;/*目的slot*/
	pstMsg->ucMsgType = MSG_TIMER;
	pstMsg->usMsgCode = msgCode;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = NULL;
	pstMsg->usFrameLen = 0;

	aulMsg[3] = (ULONG)pstMsg;
	
	if( (rc = VOS_QueSend(eventQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL)) != VOS_OK )
	{
		VOS_ASSERT(0);
		VOS_Free( pstMsg );
	}
	return rc;
}

/* 定时器回调函数，用于控制ALM和RUN指示灯 */
void eventTimerCallback()
{
	LONG queNum = VOS_QueNum( eventQueId );

	static LONG syslogFlag = 0;

#ifdef V2R1_WATCH_DOG
	if( /*(V2R1_watchdog_init_flag == V2R1_ENABLE) &&*/ (V2R1_watchdog_startup_flag == V2R1_ENABLE) )	
		V2R1_feed_watchdog();
#endif

	/*emac_check_mal_bd_tx_status();*/		/* added by xieshl 20080421, 定时检查管理通道是否正常 */

	/* 当队列满时，写syslog */
	if( queNum >= EVENT_QUEUE_LENGTH )
	{
		if( (syslogFlag & 1) == 0 )
		{
			VOS_SysLog( LOG_TYPE_DEVSM, LOG_INFO, "\r\nTask event message queue is full." );
			syslogFlag |= 1;
		}
		if( eventDebugSwitch )
		{
			/*eventDebugSwitch_bak = eventDebugSwitch;
			eventDebugSwitch = 0;*/
			eventLogOutMode_bak = eventLogOutMode;
			eventLogOutMode = EVENT_LOG_OUT_2_NONE;
		}
		return;
	}
	/* 当队列使用率超50%时，停止告警相关的所有串口打印 */
	else if( queNum >= (EVENT_QUEUE_LENGTH>>1) )
	{
		if( eventDebugSwitch )
		{
			/*eventDebugSwitch_bak = eventDebugSwitch;
			eventDebugSwitch = 0;*/
			eventLogOutMode_bak = eventLogOutMode;
			eventLogOutMode = EVENT_LOG_OUT_2_NONE;
		}
	}
	else
	{
		/* 当队列使用超5时，停止点灯 */
		if ( queNum >= 5 )
		{
			syslogFlag &= 2;
			if( (syslogFlag & 2) == 0 )
			{
				VOS_SysLog( LOG_TYPE_DEVSM, LOG_INFO, "\r\nTask event message queue is busy." );
				syslogFlag |= 1;
			}
			return;
		}
		else
			syslogFlag = 0;

		/*if( (eventDebugSwitch_bak != eventDebugSwitch) && (0 == eventDebugSwitch) )
			eventDebugSwitch = eventDebugSwitch_bak;*/
		if( (eventLogOutMode_bak != eventLogOutMode) && (EVENT_LOG_OUT_2_NONE == eventLogOutMode) )
			eventLogOutMode = eventLogOutMode_bak;
	}
	
	eventTimerMsgSend( FC_EVENT_TIMER );

	if( eventLogFlashFileFlag & eventLogFlashFileEnable )
	{
		if( eventLogFlashFileTimer > eventLogFlashFilePeriod )	/* 30 min */
		{
			eventTimerMsgSend( FC_EVENT_BACKUP );
			eventLogFlashFileTimer = 0;
		}
		else
			eventLogFlashFileTimer++;
	}
}
/*----------------------------------------------------------------------------*/

/*int alarmLogProc( uchar_t alarmType, uchar_t alarmId, alarmSrc_t *pAlarmSrc )
{
	return saveEventLog( alarmType, alarmId, pAlarmSrc );
}*/

/* ALM和RUN指示灯，由告警任务调用 */
/*static ULONG upge_ok_light_flag = 1;
static ULONG led_light_standby_counter = 1;*/
#if 0
int alarmLedProc( ULONG runLenFlag )
{
	/*ULONG devAlmStatus;*/

	if( !SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		return 0;

	if( upge_ok_light_flag )
	{
		if( SYS_MODULE_IS_RUNNING(1) && SYS_LOCAL_MODULE_ISMASTERACTIVE )
		{
			LED_UPGE_OK_Light();
			upge_ok_light_flag--;
		}
	}
	if( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
	{
		led_light_standby_counter++;
		if( led_light_standby_counter == 2 )
		{
			LED_RUN_Light();
		}
		else if( led_light_standby_counter == 4 )
		{
			LED_RUN_Dark();
			led_light_standby_counter = 0;
		}
	}
	else
	{
		if( runLenFlag & 1 )
		{
			LED_RUN_Light();
		}
		else
		{
			LED_RUN_Dark();
		}
	}
	if( (runLenFlag & 5) == 0 )
	{
		if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
		{
			devAlmStatus = 0;
			if( (getDeviceAlarmStatus(OLT_DEV_ID, &devAlmStatus) == VOS_OK) &&
				((devAlmStatus > ALM_LEV_NULL) && (devAlmStatus < ALM_LEV_MINOR)) )
			{
				LED_ALM_Light();
			}
			else
			{
				LED_ALM_Dark();
			}
		}
	}

	return 0;
}
#endif	
/*----------------------------------------------------------------------------*/

#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES)
extern LONG onuAutoLoadOnuRegisterCallback(ULONG onudevIdx);
extern LONG onuAutoLoadOnuDeregisterCallback(ULONG onudevIdx);
#endif

/* 告警消息处理，由告警任务调用 */
/*功能:告警log保存；告警主备同步；为上报网管填充必要的信息*/
/*成功:VOS_OK，失败:VOS_ERROR.*/
int eventAlarmProc( eventMsg_t *pAlmMsg  )
{
	int rc = VOS_ERROR;
	ULONG varList[8] = { 0, 0, 0, 0, 0, 0, 0, 0};
	ULONG varNum = 0;
	ULONG board,ponport,onuid/*,onu_brd,onu_prt*/;
	SHORT slot,port,PonPortIdx;
	int partnerSlot = 0,partnerPort = 0;
	ULONG ulIpAddr = 0,ulUdpPort = 0;

	if( NULL == pAlmMsg )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	/*if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	  {
	  return rc;
	  }*/	/* modified by xieshl 20100330, 问题单9671 */

	saveEventLog(pAlmMsg->alarmType, pAlmMsg->alarmId, &pAlmMsg->alarmSrc);	/* modified by wug 20100421, 问题单10104 */

	/* 业务板上告警只在本地记录告警日志，其它均直接上报主控板 */
	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER/*!SYS_LOCAL_MODULE_ISMASTERACTIVE && !SYS_LOCAL_MODULE_ISMASTERSTANDBY*/ )
	{
		eventSync_alarmReport_2Master( SYS_MASTER_ACTIVE_SLOTNO, pAlmMsg );
		return VOS_OK;
	}
	else		/* modified by xieshl 20120924, 主备告警同步，问题单15883 */
	{
		if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
		{
			board = device_standby_master_slotno_get();
			if( board )
			{
				eventSync_alarmReport_2Master( board, pAlmMsg );
			}
		}
	}

	switch( pAlmMsg->alarmType )
	{
		case alarmType_mib2:
			if( pAlmMsg->alarmId < trap_mib2_max )
			{
				sendMib2Trap( pAlmMsg->alarmId );
				saveTrapBacData( pAlmMsg->alarmType, pAlmMsg->alarmId, 
						varList, varNum, 
						&pAlmMsg->alarmSrc);
			}
			break;
		case alarmType_bridge:
			if( pAlmMsg->alarmId < trap_bridge_max )
			{
				sendBridgeTrap( pAlmMsg->alarmId );

				saveTrapBacData( pAlmMsg->alarmType, pAlmMsg->alarmId, 
						varList, varNum, 
						&pAlmMsg->alarmSrc);
			}
			break;
		case alarmType_bcmcmcctrl:
			if( pAlmMsg->alarmId < trap_cmcctrl_max )
			{
				varList[0] = pAlmMsg->alarmSrc.commAlarmSrc.brdIdx;
				varList[1] = pAlmMsg->alarmSrc.commAlarmSrc.portIdx;
				varList[2] = pAlmMsg->alarmSrc.commAlarmSrc.onuIdx;
				varList[3] = (ulong_t)&pAlmMsg->alarmSrc.commAlarmSrc.data[0];
				varList[4] = pAlmMsg->alarmSrc.commAlarmSrc.devIdx;
				varNum = 5;

				sendCmcCtrlTrap( pAlmMsg->alarmId, varList, varNum );
				saveTrapBacData( pAlmMsg->alarmType, pAlmMsg->alarmId, 
						varList, varNum, 
						&pAlmMsg->alarmSrc);
			}
			break;
		case alarmType_private:
			if( pAlmMsg->alarmId < trap_private_max )
			{
				rc = VOS_OK;

				switch( pAlmMsg->alarmId )
				{
					/* deviceIndex */
					case trap_onuNewRegSuccess:
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES)
						onuAutoLoadOnuNewRegisterCallback(pAlmMsg->alarmSrc.devAlarmSrc.devIdx);
#endif
					case trap_onuReregSuccess:
						/*if( pAlmMsg->alarmSrc.devAlarmSrc.devIdx != OLT_DEV_ID )
						  {
						  if( SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6900 )
						  {
						  SetOnuDevicePowerStatus( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, V2R1_POWER_ON );
						  }
						  }*/

						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						varList[1] = 1;
						getDeviceType( varList[0], &varList[1] );

						varList[2] = 0; /* 在发送时填入deviceSoftWareVersion */
						varList[3] = 0; /* deviceFirmWareVersion */
						varList[4] = 0; /* deviceHardWareVersion */
						varNum = 5;
						break;
						/* added by xieshl 20070703 */
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )
					case trap_onuAutoLoadUpgradeSuccess:/*add by shixh@20090218*/
					case trap_onuAutoLoadUpgradeFailure:	
						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.devAlarmSrc.devData;
						varNum = 2;

						break;
#endif
					case trap_devPowerOn:
						/*if( pAlmMsg->alarmSrc.devAlarmSrc.devIdx != OLT_DEV_ID )
						  {
						  if( SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6900 )
						  {
						  SetOnuDevicePowerStatus( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, V2R1_POWER_ON );
						  }
						  }*/
					case trap_deviceColdStart:
					case trap_deviceWarmStart:
					case trap_deviceExceptionRestart:
						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						varList[1] = 1;
						getDeviceType( varList[0], &varList[1] );

						varList[2] = 0; /* 在发送时填入deviceSoftWareVersion */
						varList[3] = 0; /* deviceFirmWareVersion */
						varList[4] = 0; /* deviceHardWareVersion */
						varList[5] = 0; /* deviceRestartupTime */
						varNum = 6;
						break;
						/* end 20070703 */
					case trap_devPowerOff:
						/* modified by xieshl 20110523, ONU掉电告警事件产生机制比较特殊，延时较大，该事件产生时
						   ONU数据已同步 到主控，但ONU状态尚不是powerDown，这里再重新校正一下。*/
						if( pAlmMsg->alarmSrc.devAlarmSrc.devIdx != OLT_DEV_ID )
						{   
#if 0/*@2014-10-9*/
							if( (SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6900) || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000) ||
									(SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6900M) ||(SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6900S))
#endif
							{
								/* modified by xieshl 20110615, onu掉电一次，之后ONU离线，show onu-list为powerdown，问题单13029 */
								SetOnuDevicePowerStatus( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, V2R1_POWER_OFF );
							}
						}
					case trap_onuNotPresent:	
						if( pAlmMsg->alarmSrc.devAlarmSrc.devIdx != OLT_DEV_ID )
						{
							check_onu_notpresent_callback( pAlmMsg->alarmSrc.devAlarmSrc.devIdx );
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES)
							onuAutoLoadOnuDeregisterCallback( pAlmMsg->alarmSrc.devAlarmSrc.devIdx );	
#endif
						}
						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						varNum = 1;
						break;
					case trap_cfgDataSaveSuccess:
					case trap_cfgDataSaveFail:
					case trap_flashClearSuccess:
					case trap_flashClearFail:
					case trap_softwareUpdateSuccess:
					case trap_softwareUpdateFail:
					case trap_firmwareUpdateSuccess:
					case trap_firmwareUpdateFail:
					case trap_cfgDataBackupSuccess:
					case trap_cfgDataBackupFail:
					case trap_cfgDataRestoreSuccess:
					case trap_cfgDataRestoreFail:
					case trap_cpuUsageFactorHigh:
					case trap_dbaUpdateSuccess:
					case trap_dbaUpdateFailure:
						/*case trap_onuFirmwareLoadSuccess:	
						  case trap_onuFirmwareLoadFailure:*/
					case trap_onuSoftwareLoadSuccess:
					case trap_onuSoftwareLoadFailure:
					case trap_onuRegisterConflict:

					case trap_bootUpdateSuccess:
					case trap_bootUpdateFailure:
					case trap_batfileBackupSuccess:
					case trap_batfileBackupFailure:
					case trap_batfileRestoreSuccess:
					case trap_batfileRestoreFailure:
					case trap_sysfileUploadsuccess:/*add by shixh20090604*/
					case trap_sysfileUploadfailure:
					case trap_sysfileDownloadsuccess:
					case trap_sysfileDownloadfailure:
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )
					case trap_onuAutoLoadConfigSuccess:/*add by shixh@20090218*/
					case trap_onuAutoLoadConfigFailure:	
#endif
#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
					case trap_deviceTemperatureHigh:
					case trap_deviceTemperatureHighClear:
					case trap_deviceTemperatureLow:
					case trap_deviceTemperatureLowClear:
#endif
						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						varNum = 1;

						break;

						/* modified by xieshl 20120810, 主控同步设置告警标志，问题单15520 */
#if(EPON_MODULE_ONU_MAC_OVERFLOW_CHECK==EPON_MODULE_YES)
					case trap_onuMacTableOverFlow:     /*add by sxh20111227*/
						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						varNum = 1;
						setOnuMacCheckAlarmStateByDevIdx( varList[0], 1 );
						break;
					case trap_onuMacTableOverFlowClear:
						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						varNum = 1;
						setOnuMacCheckAlarmStateByDevIdx( varList[0], 2 );
						break;
#endif
						/* deviceIndex, boardIndex */
					case trap_swBoardProtectedSwitch:
					case trap_boardTemperatureHigh:
					case trap_boardTemperatureHighClear:
						/*add by shixh@20080831*/
					case trap_boardCpuUsageAlarm:     
					case trap_boardCpuUsageAlarmClear:
					case trap_boardMemoryUsageAlarm:
					case trap_boardMemoryUsageAlarmClear:
						varList[0] = pAlmMsg->alarmSrc.brdAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.brdAlarmSrc.brdIdx;
						varNum = 2;

						break;

					case trap_ponBoardReset:
						if( pAlmMsg->alarmSrc.brdAlarmSrc.devIdx == OLT_DEV_ID )
							clearOltBrdAlarmStatus( pAlmMsg->alarmSrc.brdAlarmSrc.devIdx, pAlmMsg->alarmSrc.brdAlarmSrc.brdIdx );

						varList[0] = pAlmMsg->alarmSrc.brdAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.brdAlarmSrc.brdIdx;
						varNum = 2;

						break;

						/* deviceIndex, boardIndex, curBoardType */
					case trap_devBoardInterted:
					case trap_devBoardPull:
						if( pAlmMsg->alarmSrc.brdAlarmSrc.devIdx == OLT_DEV_ID )
							clearOltBrdAlarmStatus( pAlmMsg->alarmSrc.brdAlarmSrc.devIdx, pAlmMsg->alarmSrc.brdAlarmSrc.brdIdx );

						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.brdAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.brdAlarmSrc.brdType;
						/*getCurBoardType( varList[0], varList[1], &varList[2] );*//*将其移至告警处，modyied by shixh@20080624*/
						varNum = 3;

						break;

					case trap_boardLosAlarm:
					case trap_boardLosAlarmClear:
						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.brdAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.brdAlarmSrc.brdType;
						/*getCurBoardType( varList[0], varList[1], &varList[2] );*//*将其移至告警处，modyied by shixh@20080624*/
						varNum = 3;

						break;

						/* deviceIndex, boardIndex */
					case trap_powerOffAlarm:
					case trap_powerOnAlarm:
						varList[0] = pAlmMsg->alarmSrc.brdAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.brdAlarmSrc.brdIdx;
						varNum = 2;

						break;

					/*BEGIN:add by @muqw 2017-4-26*/
					case trap_pwuStatusAbnoarmal:
					case trap_pwuStatusAbnoarmalClear:
						varList[0] = pAlmMsg->alarmSrc.monAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.monAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.monAlarmSrc.monValue;
						varNum = 3;

						break;
					/*END:add by @muqw 2017-4-26*/

						/* deviceIndex, devFanIndex */
					case trap_devFanAlarm:
					case trap_devFanAlarmClear:
						varList[0] = pAlmMsg->alarmSrc.brdAlarmSrc.devIdx;
						varList[1] = FANSLOT2ID( pAlmMsg->alarmSrc.fanAlarmSrc.brdIdx, pAlmMsg->alarmSrc.fanAlarmSrc.fanIdx );
						varNum = 2;

						break;

						/* deviceIndex, ponPortBrdIndex, ponPortIndex, ponPortBER */
					case trap_ponPortBERAlarm:
					case trap_ponPortBERAlarmClear:
					case trap_ponPortFERAlarm:
					case trap_ponPortFERAlarmClear:
						/*add byshixh@20080830*/
					case trap_ponReceiverPowerTooLow:
					case trap_ponReceiverPowerTooLowClear:
					case trap_ponReceiverPowerTooHigh:
					case trap_ponReceiverPowerTooHighClear:
					case trap_ponTransmissionPowerTooLow:
					case trap_ponTransmissionPowerTooLowClear:
					case trap_ponTransmissionPowerTooHigh:
					case trap_ponTransmissionPowerTooHighClear:
					case trap_ponAppliedVoltageTooHigh:
					case trap_ponAppliedVoltageTooHighClear:
					case trap_ponAppliedVoltageTooLow:
					case trap_ponAppliedVoltageTooLowClear:
					case trap_ponBiasCurrentTooHigh:
					case trap_ponBiasCurrentTooHighClear:
					case trap_ponBiasCurrentTooLow:
					case trap_ponBiasCurrentTooLowClear:

					case trap_ponTemperatureTooHigh:
					case trap_ponTemperatureTooHighClear:
					case trap_ponTemperatureTooLow:
					case trap_ponTemperatureTooLowClear:
						varList[0] = pAlmMsg->alarmSrc.monAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.monAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.monAlarmSrc.portIdx;
						varList[3] = pAlmMsg->alarmSrc.monAlarmSrc.monValue;
						varNum = 4;

						break;
					case trap_eth_DosAttack:     /*eth port DoS attack*/
					case trap_eth_DosAttackClear:
						varList[0] = pAlmMsg->alarmSrc.monAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.monAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.monAlarmSrc.portIdx;
						varNum = 3;
						/*varList[3] = pAlmMsg->alarmSrc.monAlarmSrc.monValue;
						  varNum = 4;*/

						break;
					case trap_uplinkReceiverPowerTooLow:
					case trap_uplinkReceiverPowerTooLowClear:
					case trap_uplinkReceiverPowerTooHigh:
					case trap_uplinkReceiverPowerTooHighClear:
					case trap_uplinkTransmissionPowerTooLow:
					case trap_uplinkTransmissionPowerTooLowClear:
					case trap_uplinkTransmissionPowerTooHigh:
					case trap_uplinkTransmissionPowerTooHighClear:
					case trap_uplinkAppliedVoltageTooHigh:
					case trap_uplinkAppliedVoltageTooHighClear:
					case trap_uplinkAppliedVoltageTooLow:
					case trap_uplinkAppliedVoltageTooLowClear:
					case trap_uplinkBiasCurrentTooHigh:
					case trap_uplinkBiasCurrentTooHighClear:
					case trap_uplinkBiasCurrentTooLow:
					case trap_uplinkBiasCurrentTooLowClear:				
					case trap_uplinkTemperatureTooHigh:
					case trap_uplinkTemperatureTooHighClear:
					case trap_uplinkTemperatureTooLow:
					case trap_uplinkTemperatureTooLowClear:
						varList[0] = pAlmMsg->alarmSrc.uplinkAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.uplinkAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.uplinkAlarmSrc.ethIdx;
						varList[3] = pAlmMsg->alarmSrc.uplinkAlarmSrc.monValue;
						varNum = 4;

						break;

					case trap_ctcOnuEquipmentAlarm:
					case trap_ctcOnuEquipmentAlarmClear:
					case trap_ctcOnuBatteryMissing:
					case trap_ctcOnuBatteryMissingClear:
					case trap_ctcOnuBatteryFailure:
					case trap_ctcOnuBatteryFailureClear:
					case trap_ctcOnuPhysicalIntrusionAlarm:
					case trap_ctcOnuPhysicalIntrusionAlarmClear:
					case trap_ctcOnuSelfTestFailure:
					case trap_ctcOnuSelfTestFailureClear:
					case trap_ctcOnuIADConnectionFailure:
					case trap_ctcOnuIADConnectionFailureClear:     
					case trap_ctcOnuPonIfSwitch:
					case trap_ctcOnuPonIfSwitchClear:
						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						varNum = 1;
						break;
					case trap_ctcOnuBatteryVoltLow:
					case trap_ctcOnuBatteryVoltLowClear:
					case trap_ctcOnuTemperatureHigh:
					case trap_ctcOnuTemperatureHighClear:
					case trap_ctcOnuTemperatureLow:
					case trap_ctcOnuTemperatureLowClear:
						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.devAlarmSrc.devData;
						varNum = 2;
						break;

					case trap_ethAutoNegFailure:
					case trap_ethAutoNegFailureClear:
					case trap_ethLos:
					case trap_ethLosCLear:
					case trap_ethFailure:
					case trap_ethFailureClear:
					case trap_ethCongestion:
					case trap_ethCongestionClear:
						varList[0] = pAlmMsg->alarmSrc.portAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.portAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.portAlarmSrc.portIdx;
						varNum = 3;
						break;
						/* deviceIndex, ponPortBrdIndex, ponPortIndex, onuIndex,oltPonReceiverPower*//*add by shixh@20080831*/
					case trap_oltPonReceiverPowerTooLow:
					case trap_oltPonReceiverPowerTooLowClear:
					case trap_oltPonReceiverPowerTooHigh:
					case trap_oltPonReceiverPowerTooHighClear:/*123*/
						varList[0] = 1;
						varList[1] = pAlmMsg->alarmSrc.oltRxpowerAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.oltRxpowerAlarmSrc.portIdx;
						varList[3] = pAlmMsg->alarmSrc.oltRxpowerAlarmSrc.onuIdx;
						varList[4] = pAlmMsg->alarmSrc.oltRxpowerAlarmSrc.oltrxValue;
						varNum = 5;

						break;
						/* deviceIndex, ponPortBrdIndex, ponPortIndex, ponLlidIndex */
					case trap_llidActBWExceeding:
					case trap_llidActBWExceedingClear:
						varList[0] = pAlmMsg->alarmSrc.llidAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.llidAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.llidAlarmSrc.portIdx;
						varList[3] = pAlmMsg->alarmSrc.llidAlarmSrc.llidIdx;
						varNum = 4;
						break;
					case trap_ponLaserAlwaysOnAlarm:        /*add by shixh20090629*/
					case trap_ponLaserAlwaysOnAlarmClear:
						varList[0] = MAKEDEVID(pAlmMsg->alarmSrc.oltRxpowerAlwaysOnAlarmSrc.brdIdx,
								pAlmMsg->alarmSrc.oltRxpowerAlwaysOnAlarmSrc.portIdx,
								pAlmMsg->alarmSrc.oltRxpowerAlwaysOnAlarmSrc.onuIdx)
							/*pAlmMsg->alarmSrc.oltRxpowerAlwaysOnAlarmSrc.brdIdx * 10000 +
							  pAlmMsg->alarmSrc.oltRxpowerAlwaysOnAlarmSrc.portIdx * 1000 +
							  pAlmMsg->alarmSrc.oltRxpowerAlwaysOnAlarmSrc.onuIdx*/;
							varList[1] = 1;
						varList[2] = 1;
						varNum = 3;
						break;

						/* deviceIndex, boardIndex, ponPortIndex */
					case trap_autoProtectSwitch:
					case trap_firmwareLoadSuccess:
					case trap_firmwareLoadFailure:
					case trap_dbaLoadSuccess:
					case trap_dbaLoadFailure:
					case trap_ponToEthLinkdown:	/* added 20070703 */
					case trap_ponToEthLinkup:
					case trap_PonPortFullAlarm:/*add by shixh20090507*/
					case trap_ponPortAbnormalClear:
					case trap_ponPortAbnormal:/*modyied by shixh20090625*/
					case trap_ponPortlosAlarm:       /*add by shixh20090626*/
					case trap_ponPortlosAlarmClear:
						

					case trap_ponFWVersionMismatch:/*add by shixh20090710*/
					case trap_ponFWVersionMatch:
					case trap_ponDBAVersionMismatch:
					case trap_ponDBAVersionMatch:
						varList[0] = pAlmMsg->alarmSrc.portAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.portAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.portAlarmSrc.portIdx;
						varNum = 3;
						/*added by wangjiah@2017-05-05:begin
						 * update 10gepon pon info according to xfptype passed by event report
						 * */
						if(pAlmMsg->alarmId == trap_ponPortlosAlarmClear && SYS_MODULE_IS_10G_EPON(varList[1]))
						{
							PonPortIdx = GetPonPortIdxBySlot(varList[1], varList[2]);
							if(PonPortIdx != RERROR)
							{
								UpdateXGEPonPortInfoByType(PonPortIdx, pAlmMsg->alarmSrc.alarmSrcData[6]);
							}
						}
						/*added by wangjiah@2017-05-05:end*/
						break;

					case trap_ponSFPTypeMismatch:      
					case trap_ponSFPTypeMitch:
						/* modified by xieshl 2011120703，不再额外定义消息了，主控上直接修改PON SFP匹配状态，问题单13757 */
						if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx == OLT_DEV_ID )
						{
							if( SYS_MODULE_SLOT_ISHAVECPU(pAlmMsg->alarmSrc.portAlarmSrc.brdIdx) )
							{
								SHORT PonPortIdx = GetPonPortIdxBySlot(pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx);
								if( PonPortIdx != RERROR )
								{
									if( pAlmMsg->alarmId == trap_ponSFPTypeMismatch )
										/*PonPortTable[PonPortIdx].PortAdminStatus = V2R1_DISABLE;*/
										PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm = V2R1_ENABLE;
									else
										/*PonPortTable[PonPortIdx].PortAdminStatus = V2R1_ENABLE;*/
										PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm = V2R1_DISABLE;
								}
							}
						}

						varList[0] = pAlmMsg->alarmSrc.portAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.portAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.portAlarmSrc.portIdx;
						varNum = 3;

						break;

						/* devIndex, boardIndex, portIndex */
					case trap_ethFlrAlarm:
					case trap_ethFlrAlarmClear:
					case trap_ethFerAlarm:
					case trap_ethFerAlarmClear:
					case trap_ethTranmittalIntermitAlarm:
					case trap_ethTranmittalIntermitAlarmClear:
					case trap_ethLinkdown:
					case trap_ethLinkup:
						varList[0] = pAlmMsg->alarmSrc.portAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.portAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.portAlarmSrc.portIdx;
						varNum = 3;

						break;
					case trap_ponportBRASAlarm:
					case trap_ponportBRASAlarmClear:					
					case trap_onuRegAuthFailure:
						varList[0] = pAlmMsg->alarmSrc.commAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.commAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.commAlarmSrc.portIdx;

						varList[3] = (ULONG)&pAlmMsg->alarmSrc.commAlarmSrc.data[0];
						varNum = 4;

						break;

					case trap_onuDeletingNotify:
						varList[0] = pAlmMsg->alarmSrc.commAlarmSrc.brdIdx;
						varList[1] = pAlmMsg->alarmSrc.commAlarmSrc.portIdx;
						varList[2] = pAlmMsg->alarmSrc.commAlarmSrc.onuIdx;
						varList[3] = (ulong_t)&pAlmMsg->alarmSrc.commAlarmSrc.data[0];
						varList[4] = MAKEDEVID( varList[0], varList[1], varList[2] );
						varNum = 5;

						break;

						/*add by shixh@20070928*/
						/*devIdx,  brdIdx,   portIdx*/
					case trap_e1LosAlarm:
					case trap_e1LosAlarmClear:
					case trap_e1LofAlarm:
					case trap_e1LofAlarmClear:
					case trap_e1AisAlarm:
					case trap_e1AisAlarmClear:
					case trap_e1RaiAlarm:
					case trap_e1RaiAlarmClear:
					case trap_e1SmfAlarm:
					case trap_e1SmfAlarmClear:
					case trap_e1LomfAlarm:
					case trap_e1LomfAlarmClear:
					case trap_e1Crc3Alarm:
					case trap_e1Crc3AlarmClear:
					case trap_e1Crc6Alarm:
					case trap_e1Crc6AlarmClear:
					case trap_E1OutOfService:
					case trap_E1OutOfServiceClear:
						varList[0] = pAlmMsg->alarmSrc.portAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.portAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.portAlarmSrc.portIdx;
						varNum = 3;
						break;
					case trap_tdmServiceAbortAlarm:	
					case trap_tdmServiceAbortAlarmClear:						
						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;				
						varNum = 1;					
						break;			

						/*added by xieshl 20080116*/
					case trap_ethLoopAlarm:
					case trap_ethLoopAlarmClear:
						varList[0] = pAlmMsg->alarmSrc.portAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.portAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.portAlarmSrc.portIdx;
						varNum = 3;
						break;
					case trap_onuLoopAlarm:
					case trap_onuLoopAlarmClear:
						varList[0] = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						varNum = 1;
						break;
						/* end 20080116 */

					case trap_SwitchEthPortLoop:/*add by shixh20090520*/
					case trap_SwitchEthPortLoopClear:
					case trap_switchEthEgressLimitExceed:
					case trap_switchEthEgressLimitExceedClear:
					case trap_switchEthIngressLimitExceed:
					case trap_switchEthIngressLimitExceedClear:
						board = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.brdId;
						ponport = (pAlmMsg->alarmSrc.onuSwitchAlarmSrc.ponId ? pAlmMsg->alarmSrc.onuSwitchAlarmSrc.ponId : 16);
						onuid = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.onuId;
						varList[0] =MAKEDEVID(board,ponport,onuid) /*board*10000+ponport*1000+onuid*/;
						varList[1] = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.onuBrdId;
						varList[2] = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.onuPortId;
						varList[3] = (ulong_t)&pAlmMsg->alarmSrc.onuSwitchAlarmSrc.switchMacAddr[0];	/* 问题单12797 */
						varList[4] = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.reason/*switchPortIdx*/;
						/*varList[5] = (ulong_t)&pAlmMsg->alarmSrc.onuSwitchAlarmSrc.switchMacAddr[4];*/
						varNum = 5;
						break;

					case trap_switchNewRegSuccess:
					case trap_switchReregSuccess:
					case trap_switchNotPresent:
						board = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.brdId;
						ponport = (pAlmMsg->alarmSrc.onuSwitchAlarmSrc.ponId ? pAlmMsg->alarmSrc.onuSwitchAlarmSrc.ponId : 16);
						onuid = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.onuId;
						varList[0] =MAKEDEVID(board,ponport,onuid)/* board*10000+ponport*1000+onuid*/;
						varList[1] = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.onuBrdId;
						varList[2] = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.onuPortId;
						varList[3] = (ulong_t)&pAlmMsg->alarmSrc.onuSwitchAlarmSrc.switchMacAddr[0];
						varNum = 4;
						break;

					case trap_backboneEthLinkdown:	/* added by shixh@20080215*/
					case trap_backboneEthLinkup:
					case trap_ethPortBroadCastFloodControl:/*add by shixh20090612*/
					case  trap_ethPortBroadCastFloodControlClear:
						varList[0] = pAlmMsg->alarmSrc.portAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.portAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.portAlarmSrc.portIdx;
						varNum = 3;

						break;

					case trap_tdmToEthLinkdown:	/* added by shixh@20080202*/
					case trap_tdmToEthLinkup:
						varList[0] = pAlmMsg->alarmSrc.portAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.portAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.portAlarmSrc.portIdx;
						varNum = 3;

						break;

					case trap_userLocUpdateNotify:
						varList[0] = pAlmMsg->alarmSrc.userTraceAlarmSrc.hashIdx;
						varList[1] = (ULONG)&pAlmMsg->alarmSrc.userTraceAlarmSrc.macAddr[0];
						varNum = 9;
						break;

					case trap_backupPonAlarm:
					case trap_backupPonAlarmClear:
						board = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.brdId;
						ponport = (pAlmMsg->alarmSrc.onuSwitchAlarmSrc.ponId ? pAlmMsg->alarmSrc.onuSwitchAlarmSrc.ponId : 16);
						onuid = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.onuId;
						varList[0] =MAKEDEVID(board,ponport,onuid) /*board*10000+ponport*1000+onuid*/;
						varList[1] = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.onuBrdId;
						varList[2] = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.onuPortId;
						varList[3] = (ulong_t)&pAlmMsg->alarmSrc.onuSwitchAlarmSrc.switchMacAddr[0];	/* 问题单12797 */
						varList[4] = pAlmMsg->alarmSrc.onuSwitchAlarmSrc.reason;
						/*varList[5] = (ulong_t)&pAlmMsg->alarmSrc.onuSwitchAlarmSrc.switchMacAddr[4];*/
						varNum = 5;
						break;

					case trap_logicalSlotInsert:
					case trap_logicalSlotPull:
						varList[0] = pAlmMsg->alarmSrc.logicalSlotAlarmSrc.devIdx;
						varList[1] = pAlmMsg->alarmSrc.logicalSlotAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.logicalSlotAlarmSrc.ipAddr;
						varList[3] = pAlmMsg->alarmSrc.logicalSlotAlarmSrc.udpPort;
						varNum = 4;
						break;

					case trap_ponProtectSwitch:

						slot = pAlmMsg->alarmSrc.ponSwitchAlarmSrc.partnerBrdIdx;
						port = pAlmMsg->alarmSrc.ponSwitchAlarmSrc.partnerPonIdx;
						varList[0] = OLT_DEV_ID;
						varList[1] = pAlmMsg->alarmSrc.ponSwitchAlarmSrc.brdIdx;
						varList[2] = pAlmMsg->alarmSrc.ponSwitchAlarmSrc.ponIdx;
						varList[3] = OLT_DEV_ID;

						if(VOS_OK == devsm_remote_port_getremoteport(slot, port, &partnerSlot, &partnerPort) &&
								VOS_OK == devsm_remote_slot_getremoteaddr(slot, &ulIpAddr, &ulUdpPort))
						{
							varList[4] = partnerSlot; 
							varList[5] = partnerPort; 
							varList[6] = ulIpAddr; 
							varList[7] = ulUdpPort; 
						}
						else
						{
							varList[4] = pAlmMsg->alarmSrc.ponSwitchAlarmSrc.partnerBrdIdx;
							varList[5] = pAlmMsg->alarmSrc.ponSwitchAlarmSrc.partnerPonIdx;
							varList[6] = *((unsigned long *)pAlmMsg->alarmSrc.ponSwitchAlarmSrc.ipAddrWithPort);
							varList[7] = *((unsigned short *)(pAlmMsg->alarmSrc.ponSwitchAlarmSrc.ipAddrWithPort + sizeof(unsigned long)));
						}
						
						varNum = 8;
						break;



					default:
						rc = VOS_ERROR;
						break;
				}

				if( rc == VOS_OK )
				{
#ifdef OLT_PWU_OFF_TRAP
					if( IS_OLT_DEV_PWU_OFF(pAlmMsg) )
					{
						sendOltPowerOffTrap( pAlmMsg->alarmSrc.devAlarmSrc.devIdx );
						return rc;
					}
#endif
					sendPrivateTrap( pAlmMsg->alarmId, varList, varNum );

					saveTrapBacData( pAlmMsg->alarmType, pAlmMsg->alarmId, 
							varList, varNum, 
							&pAlmMsg->alarmSrc);
				}
			}
			break;
		case alarmType_other:
			/*if( pAlmMsg->alarmId <= sizeof(other_EventLog_MapInfo)/sizeof(other_EventLog_MapInfo[0]) )
			  {
			  }*/
			break;
		default:
			break;
	}

	return rc;
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

extern LONG onuExtMgmt_BrdInsertCallback( ULONG devIdx, ULONG brdIdx, UCHAR *pBrdPduData );
extern LONG onuExtMgmt_BrdPullCallback( ULONG devIdx, ULONG brdIdx );
/*ONU管理数据同步*/
int eventSync_onuExtHotProc( eventSyncOnuBrdData_t *pSyncData )
{
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	int rc = VOS_ERROR;
	if( NULL == pSyncData )
	{
		VOS_ASSERT(0);
		return rc;
	}/*只同步这三个事件?*/
	if( (onuslot_inserted == pSyncData->onuEventId) ||
		(onuslot_statuschange == pSyncData->onuEventId) )
	{
		rc = onuExtMgmt_BrdInsertCallback( pSyncData->devIdx, pSyncData->brdIdx, pSyncData->brdData );
	}
	else if( onuslot_pull == pSyncData->onuEventId )
	{
		rc = onuExtMgmt_BrdPullCallback( pSyncData->devIdx, pSyncData->brdIdx );
	}
	return rc;
#else
	return VOS_OK;
#endif
}
/*告警配置数据同步*/
/*命令行的配置生效的地方*/
int eventSync_cdpRecvProc( eventSyncCfgData_t *pRecvMsg )
{
	int rc = VOS_ERROR;
	
	if( NULL == pRecvMsg )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	switch( pRecvMsg->subType )
	{
		case EVENT_MSG_SUBTYPE_ALARM_LOG:
			rc = setEventLogEnable( pRecvMsg->subCode );
			break;
		case EVENT_MSG_SUBTYPE_TRAP_BAC:
			rc = setTrapBacEnable( pRecvMsg->subCode );
			break;
		case EVENT_MSG_SUBTYPE_ALARM_MASK:
			rc = eventSync_setAlarmMask( (eventSyncCfgMsg_t*)pRecvMsg );
			break;
		case EVENT_MSG_SUBTYPE_ALARM_STATUS:
			rc = eventSync_setAlarmStatus( (eventSyncCfgMsg_t*)pRecvMsg );
			break;
		default:
			break;
	}
	return rc;
}

/* 功能: ONU管理数据同步到主控
    调用:onu板卡插拔时，若本板不是主控则需要调用本函数进行数据的同步
    输入参数:  devIdx	ONU索引
    		     brdIdx
    		     alarmFlag
    		     pBrdPduData
    		     dataLen
    成功:VOS_OK;失败:VOS_ERROR*/
int eventSync_onuData_2Master( ULONG devIdx, ULONG brdIdx, ULONG alarmFlag, UCHAR* pBrdPduData, LONG dataLen )
{
	int rc;
	
	ULONG msgLen = sizeof(SYS_MSG_S) + dataLen;
	SYS_MSG_S   *pstMsg;
	eventSyncOnuBrdData_t *pSndData;

	if( (pBrdPduData == NULL) || (dataLen <= 0) || (dataLen >= EUQ_MAX_OAM_PDU) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )	/* modified by xieshl 20111201, 兼容6100/6700 */
		return VOS_OK;
	
	pstMsg = (SYS_MSG_S *)CDP_AllocMsg( msgLen, MODULE_EVENT );
	if(pstMsg == NULL)
	{
		return VOS_ERROR;
	}
	pSndData = (eventSyncOnuBrdData_t *)(pstMsg + 1);

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = MODULE_EVENT;
	pstMsg->ulDstModuleID = MODULE_EVENT;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_MASTER_ACTIVE_SLOTNO;/*目的slot*/
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (VOID *)pSndData;
	pstMsg->usFrameLen = sizeof(eventSyncOnuBrdData_t);
	pstMsg->usMsgCode = FC_EVENT_DATA_SYNC;

	/*将一部分管理信息从PON板上传到 主控板上*/
	pSndData->devIdx = devIdx;
	pSndData->brdIdx = brdIdx;
	pSndData->onuEventId = alarmFlag;

	if( (onuslot_inserted == alarmFlag) || (onuslot_statuschange == alarmFlag) )
	{
		VOS_MemCpy( pSndData->brdData, pBrdPduData, dataLen );
	}
	else if( onuslot_pull == alarmFlag )
	{
		VOS_MemCpy( pSndData->brdData, pBrdPduData, dataLen );
	}
	else
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	rc = CDP_Send( RPU_TID_CDP_EVENT, SYS_MASTER_ACTIVE_SLOTNO,  RPU_TID_CDP_EVENT, CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_EVENT );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pstMsg );
	}	
	return rc;
}

/* 功能: 业务板上检测到的所有告警事件同步到主控
    输入参数: pAlmMsg-指定告警标识和告警源 */
int eventSync_alarmReport_2Master( ULONG slotno, eventMsg_t *pAlmMsg )
{
	int rc;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(eventMsg_t);
	SYS_MSG_S   *pstMsg;
	eventMsg_t  *pSyncAlmMsg;

	if( (NULL == pAlmMsg) || (SYS_LOCAL_MODULE_SLOTNO == slotno) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	
	/*if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;*/
	
	pstMsg = (SYS_MSG_S *)CDP_AllocMsg( msgLen, MODULE_EVENT );
	if(pstMsg == NULL)
	{
		/*VOS_ASSERT(pstMsg);*/
		return VOS_ERROR;
	}
	pSyncAlmMsg = (eventMsg_t *)(pstMsg + 1);

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = MODULE_EVENT;
	pstMsg->ulDstModuleID = MODULE_EVENT;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = slotno;/*SYS_MASTER_ACTIVE_SLOTNO;*//*目的slot*/
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (VOID *)pSyncAlmMsg;
	pstMsg->usFrameLen = sizeof(eventMsg_t);
	pstMsg->usMsgCode = FC_EVENT_ALM_SYNC;

	VOS_MemCpy((CHAR *)pSyncAlmMsg, (CHAR *)pAlmMsg, sizeof(eventMsg_t) );        

	rc = CDP_Send( RPU_TID_CDP_EVENT, slotno/*SYS_MASTER_ACTIVE_SLOTNO*/,  RPU_TID_CDP_EVENT,  CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_EVENT );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pstMsg );
	}	
	return rc;
}

/* 功能: 告警使能配置数据同步到所有业务板，只能在主控上执行
    输入参数: subType -- 配置数据类型，1-告警日志使能；2-告警trap备份使能
                 subCode -- 使能，1-enable；2-disable */
int eventSync_configEbl_2AllSlave( ULONG subType, ULONG subCode )
{
	int rc;
	ULONG slotno;
	SYS_MSG_S   *pstMsg;
	eventSyncCfgData_t *pSndCfgMsg;
	ULONG msgLen;

	if ( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		return VOS_OK;
	}

	msgLen = sizeof(SYS_MSG_S) + sizeof(eventSyncCfgData_t);
	
	for( slotno = 1; slotno <= SYS_CHASSIS_SLOTNUM; slotno++ )
	{
		if( !SYS_MODULE_SLOT_ISHAVECPU(slotno) )
			continue;

		if(  SYS_MODULE_IS_READY(slotno) && (slotno != SYS_LOCAL_MODULE_SLOTNO) )
		{
			pstMsg = CDP_AllocMsg( msgLen, MODULE_EVENT );
			if ( NULL == pstMsg )
			{
				/*VOS_ASSERT(0);*/
				return VOS_ERROR;
			}
			pSndCfgMsg = (eventSyncCfgData_t *)(pstMsg + 1);

			VOS_MemZero( pstMsg, msgLen );
			pstMsg->ulSrcModuleID = MODULE_EVENT;
			pstMsg->ulDstModuleID = MODULE_EVENT;
			pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
			pstMsg->ulDstSlotID = slotno;/*目的slot*/
			pstMsg->ucMsgType = MSG_NOTIFY;
			pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
			pstMsg->ptrMsgBody = (VOID *)pSndCfgMsg;
			pstMsg->usFrameLen = sizeof(eventSyncCfgData_t);
			pstMsg->usMsgCode = FC_EVENT_CFG_SYNC;

         		pSndCfgMsg->subType = subType;
			pSndCfgMsg->subCode = subCode;
					
			rc = CDP_Send( RPU_TID_CDP_EVENT, slotno,  RPU_TID_CDP_EVENT,  CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_EVENT );
			if( rc !=  VOS_OK )
			{
				VOS_ASSERT(0);
				CDP_FreeMsg( (void *) pstMsg );
				return VOS_ERROR;
			}	
		}
	}
	return VOS_OK;
}

/*----------------------------------------------------------------------------*/
/*集中处理告警屏蔽同步*/
extern ULONG alarm_mask_pon_cni_bits; 
int eventMaskProc( eventMsg_t *pAlmMsg )
{
	ULONG mask = 0;
	/*ULONG  onutype=0;*/

	if( alarmType_other == pAlmMsg->alarmType )
	{
		if( other_pon_cni_ber == pAlmMsg->alarmId )
		{
			if( alarm_mask_pon_cni_bits & 1 )
				return VOS_OK;
		}
	}
	else if( alarmType_private == pAlmMsg->alarmType )
	{
		switch( pAlmMsg->alarmId )
		{
			case trap_onuNewRegSuccess:
			case trap_onuReregSuccess:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_REGISTER )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.devAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK ) 
					{
						if( mask & EVENT_MASK_DEV_REGISTER )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_REGISTER )
							return VOS_OK;
					}
				}
				break;
			case trap_onuNotPresent:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK )
				{
					if( mask  & EVENT_MASK_DEV_PRESENT )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.devAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PRESENT )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PRESENT )
							return VOS_OK;
					}
				}
				break;
			case trap_devPowerOff:
			case trap_devPowerOn:
			case trap_powerOffAlarm:	/* 电源板掉电告警 */
			case trap_powerOnAlarm:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK )
				{
					if( mask  & EVENT_MASK_DEV_POWER )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.devAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_POWER )
						{
							return VOS_OK;
						}
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_POWER )
							return VOS_OK;
					}
				}
				break;
			case trap_pwuStatusAbnoarmal:     /*电源异常信息告警  @muqw 2017-4-26*/
			case trap_pwuStatusAbnoarmalClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK )
				{
					if( mask  & EVENT_MASK_DEV_PWU_STATUS)
						return VOS_OK;
				}
				break;
			case trap_devFanAlarm:
			case trap_devFanAlarmClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK )
				{
					if( mask & EVENT_MASK_DEV_FAN )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.devAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.brdAlarmSrc.devIdx, &mask ) == VOS_OK )	/* 问题单11890 */
					{
						if( mask & EVENT_MASK_DEV_FAN )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_FAN )
							return VOS_OK;
					}
				}
				break;
			case trap_cpuUsageFactorHigh:
			case trap_boardCpuUsageAlarm:
			case trap_boardCpuUsageAlarmClear:
				if( pAlmMsg->alarmSrc.devAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK )	/* modified by xieshl 20121114, 不再对OLT侧设备板卡告警屏蔽，问题单16277 */
					{
						if( mask & EVENT_MASK_DEV_CPU )
							return VOS_OK;
					}
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_CPU )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_CPU )
							return VOS_OK;
					}
				}
				break;
			case trap_boardTemperatureHigh:
			case trap_boardTemperatureHighClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK )
				{
					if( mask & EVENT_MASK_DEV_TEMPERATURE )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.devAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_TEMPERATURE )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_TEMPERATURE )
							return VOS_OK;
					}
				}
				break;
			case trap_deviceTemperatureHigh:
			case trap_deviceTemperatureHighClear:
			case trap_deviceTemperatureLow:
			case trap_deviceTemperatureLowClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK )
				{
					if( mask & EVENT_MASK_DEV_TEMPERATURE )
						return VOS_OK;
				}
				if(pAlmMsg->alarmSrc.devAlarmSrc.devIdx!=OLT_DEV_ID)
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_TEMPERATURE )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_TEMPERATURE )
							return VOS_OK;
					}
				}
				break;

			case trap_ponPortBERAlarm:
			case trap_ponPortBERAlarmClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_BER )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, 
					pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_BER )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.devAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getPonPortAlarmMask(OLT_DEV_ID, GET_PONSLOT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), GET_PONPORT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_PON_BER )
							return VOS_OK;
					}
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_BER )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_BER )
							return VOS_OK;
					}
				}
				break;
			case trap_ponPortFERAlarm:
			case trap_ponPortFERAlarmClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_FER )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, 
					pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_FER )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getPonPortAlarmMask(OLT_DEV_ID, GET_PONSLOT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), GET_PONPORT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_PON_FER )
							return VOS_OK;
					}
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_FER )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_FER )
							return VOS_OK;
					}
				}
				break;
			case trap_autoProtectSwitch:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_ABS )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_APS )
						return VOS_OK;
				}
				
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getPonPortAlarmMask(OLT_DEV_ID, GET_PONSLOT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), GET_PONPORT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_PON_APS )
							return VOS_OK;
					}
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_ABS )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_ABS )
							return VOS_OK;
					}
				}
				break;
			case trap_ponPortAbnormal:
			case trap_ponPortAbnormalClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_ABNORMAL )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_ABNORMAL )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getPonPortAlarmMask(OLT_DEV_ID, GET_PONSLOT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), GET_PONPORT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_PON_ABNORMAL )
							return VOS_OK;
					}
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_ABNORMAL )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_ABNORMAL )
							return VOS_OK;
					}
				}
				break;
			case trap_ethFlrAlarm:
			case trap_ethFlrAlarmClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_ETH_FLR )
						return VOS_OK;
				}
				if( getEthPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_ETH_FLR )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_ETH_FLR )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_ETH_FLR )
							return VOS_OK;
					}
				}
				break;
			case trap_ethFerAlarm:
			case trap_ethFerAlarmClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_ETH_FER )
						return VOS_OK;
				}
				if( getEthPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_ETH_FER )
						return VOS_OK;
				}
				if(pAlmMsg->alarmSrc.portAlarmSrc.devIdx!=OLT_DEV_ID)
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_ETH_FER )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_ETH_FER )
							return VOS_OK;
					}
				}
				break;
			case trap_ethTranmittalIntermitAlarm:
			case trap_ethTranmittalIntermitAlarmClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_ETH_TI )
						return VOS_OK;
				}
				if( getEthPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_ETH_TI )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_ETH_TI )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_ETH_TI )
							return VOS_OK;
					}
				}
				break;
#if 0	/* modified by xieshl 20111213, 目前命令中光功率告警是不区分收发和高低的 */
			case trap_ponTransmissionPowerTooHigh:
			case trap_ponTransmissionPowerTooHighClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_POWER_H )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.monAlarmSrc.devIdx, pAlmMsg->alarmSrc.monAlarmSrc.brdIdx, pAlmMsg->alarmSrc.monAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_POWER_HIGH )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.monAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER_H )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.monAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER_H )
							return VOS_OK;
					}
				}
				break;
			case trap_ponTransmissionPowerTooLow:
			case trap_ponTransmissionPowerTooLowClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_POWER_L )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.monAlarmSrc.devIdx, pAlmMsg->alarmSrc.monAlarmSrc.brdIdx, pAlmMsg->alarmSrc.monAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_POWER_LOW )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.monAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.monAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER_L )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.monAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER_L )
							return VOS_OK;
					}
				}
				break;
			case trap_ponReceiverPowerTooHigh:
			case trap_ponReceiverPowerTooHighClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_POWER_H )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_POWER_HIGH )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getPonPortAlarmMask(OLT_DEV_ID, GET_PONSLOT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), GET_PONPORT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_PON_POWER_HIGH )
							return VOS_OK;
					}

					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER_H )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER_H )
							return VOS_OK;
					}
				}
				break;
			case trap_ponReceiverPowerTooLow:
			case trap_ponReceiverPowerTooLowClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_POWER_L )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_POWER_LOW )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getPonPortAlarmMask(OLT_DEV_ID, GET_PONSLOT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), GET_PONPORT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_PON_POWER_LOW )
							return VOS_OK;
					}

					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER_L )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER_L )
							return VOS_OK;
					}
				}
				break;
#else
			case trap_oltPonReceiverPowerTooLow:
			case trap_oltPonReceiverPowerTooLowClear:
			case trap_oltPonReceiverPowerTooHigh:
			case trap_oltPonReceiverPowerTooHighClear:/*123*/
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_POWER )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(OLT_DEV_ID, pAlmMsg->alarmSrc.oltRxpowerAlarmSrc.brdIdx, pAlmMsg->alarmSrc.oltRxpowerAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_POWER )
						return VOS_OK;
				}
				break;
			case trap_ponTransmissionPowerTooHigh:
			case trap_ponTransmissionPowerTooHighClear:
			case trap_ponTransmissionPowerTooLow:
			case trap_ponTransmissionPowerTooLowClear:
			case trap_ponReceiverPowerTooHigh:
			case trap_ponReceiverPowerTooHighClear:
			case trap_ponReceiverPowerTooLow:
			case trap_ponReceiverPowerTooLowClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_POWER )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_POWER )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getPonPortAlarmMask(OLT_DEV_ID, GET_PONSLOT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), GET_PONPORT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_PON_POWER )
							return VOS_OK;
					}

					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER )
							return VOS_OK;
					}
					if((OptOnuAlarmMask & (EVENT_MASK_PON_POWER_LOW | EVENT_MASK_PON_POWER_HIGH))
					&& SYS_LOCAL_MODULE_ISMASTERACTIVE)
					{
						return VOS_OK;
					}
				}
				break;

			/* added by xieshl 20111213, 漏掉了PON光模块工作电压、偏置电流、温度等告警屏蔽 */
			case trap_ponAppliedVoltageTooHigh:
			case trap_ponAppliedVoltageTooHighClear:
			case trap_ponAppliedVoltageTooLow:
			case trap_ponAppliedVoltageTooLowClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_POWER )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & (EVENT_MASK_PON_VOLTAGE | EVENT_MASK_PON_POWER) )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getPonPortAlarmMask(OLT_DEV_ID, GET_PONSLOT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), GET_PONPORT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), &mask) == VOS_OK )
					{
						if( mask & (EVENT_MASK_PON_VOLTAGE | EVENT_MASK_PON_POWER) )
							return VOS_OK;
					}
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER )
							return VOS_OK;
					}
					if((OptOnuAlarmMask & EVENT_MASK_PON_VOLTAGE) && SYS_LOCAL_MODULE_ISMASTERACTIVE)
					{
						return VOS_OK;
					}
				}
				break;
			case trap_ponBiasCurrentTooHigh:
			case trap_ponBiasCurrentTooHighClear:
			case trap_ponBiasCurrentTooLow:
			case trap_ponBiasCurrentTooLowClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_POWER )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & (EVENT_MASK_PON_BIAS_CURRENT | EVENT_MASK_PON_POWER) )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getPonPortAlarmMask(OLT_DEV_ID, GET_PONSLOT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), GET_PONPORT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), &mask) == VOS_OK )
					{
						if( mask & (EVENT_MASK_PON_BIAS_CURRENT | EVENT_MASK_PON_POWER) )
							return VOS_OK;
					}
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER )
							return VOS_OK;
					}
					if((OptOnuAlarmMask & EVENT_MASK_PON_BIAS_CURRENT) && SYS_LOCAL_MODULE_ISMASTERACTIVE)
					{
						return VOS_OK;
					}
				}
				break;
			case trap_ponTemperatureTooHigh:
			case trap_ponTemperatureTooHighClear:
			case trap_ponTemperatureTooLow:
			case trap_ponTemperatureTooLowClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_POWER )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & (EVENT_MASK_PON_TEMPERATURE | EVENT_MASK_PON_POWER) )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getPonPortAlarmMask(OLT_DEV_ID, GET_PONSLOT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), GET_PONPORT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), &mask) == VOS_OK )
					{
						if( mask & (EVENT_MASK_PON_TEMPERATURE | EVENT_MASK_PON_POWER) )
							return VOS_OK;
					}
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_POWER )
							return VOS_OK;
					}
					if((OptOnuAlarmMask & EVENT_MASK_PON_TEMPERATURE) && SYS_LOCAL_MODULE_ISMASTERACTIVE)
					{
						return VOS_OK;
					}
				}
				break;
#endif
			case trap_ponPortlosAlarm:
			case trap_ponPortlosAlarmClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_LOS )
						return VOS_OK;
				}
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.portAlarmSrc.devIdx, &mask ) == VOS_OK ) 
					{
						if( mask  & EVENT_MASK_DEV_PON_LOS )
							return VOS_OK;
					}
					if( getPonPortAlarmMask(OLT_DEV_ID, GET_PONSLOT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), GET_PONPORT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_PON_LOS )
							return VOS_OK;
					}
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_LOS )
						return VOS_OK;
				}
				break;
			case trap_ponLaserAlwaysOnAlarm:
			case trap_ponLaserAlwaysOnAlarmClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_ONU_LASER_ON )
						return VOS_OK;
				}
				/*if( pAlmMsg->alarmSrc.oltRxpowerAlwaysOnAlarmSrc.onuIdx != OLT_DEV_ID )
				{
					if( getDeviceType(pAlmMsg->alarmSrc.oltRxpowerAlwaysOnAlarmSrc.onuIdx, &onutype) == VOS_OK )
					{
						if( onu_type_mask[onutype] & 0x00008000 )
							return VOS_OK;
					}
				}*/		/* 问题单11844 */
				if( getPonPortAlarmMask(OLT_DEV_ID, pAlmMsg->alarmSrc.oltRxpowerAlwaysOnAlarmSrc.brdIdx, pAlmMsg->alarmSrc.oltRxpowerAlwaysOnAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_LASER_ON )
						return VOS_OK;
				}
				break;
			case trap_ponToEthLinkdown:
			case trap_ponToEthLinkup:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_LINK )
						return VOS_OK;
				}
				if( getPonPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_LINK )
						return VOS_OK;
				}

				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getPonPortAlarmMask(OLT_DEV_ID, GET_PONSLOT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), GET_PONPORT(pAlmMsg->alarmSrc.portAlarmSrc.devIdx), &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_PON_LINK )
							return VOS_OK;
					}
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK ) 
					{
						if( mask  & EVENT_MASK_DEV_PON_LINK )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_PON_LINK )
							return VOS_OK;
					}
				}
				break;

			case trap_ethLinkdown:
			case trap_ethLinkup:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_ETH_LINK )
						return VOS_OK;
				}
				if( getEthPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_ETH_LINK )
						return VOS_OK;
				}
				
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_ETH_LINK )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_ETH_LINK )
							return VOS_OK;
					}
				}
				break;
			case trap_ethLoopAlarm:
			case trap_ethLoopAlarmClear:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_ETH_LOOP )
						return VOS_OK;
				}
				if( getEthPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_ETH_LOOP )
						return VOS_OK;
				}
				if(pAlmMsg->alarmSrc.portAlarmSrc.devIdx!=OLT_DEV_ID)
				{
					if( getDeviceAlarmMask( pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask ) == VOS_OK )	/* 问题单11879 */
					{
						if( mask & EVENT_MASK_DEV_ETH_LOOP )
							return VOS_OK;
					}
					if( getOnuTypeAlarmMaskByDevIdx(pAlmMsg->alarmSrc.devAlarmSrc.devIdx, &mask) == VOS_OK )
					{
						if( mask & EVENT_MASK_DEV_ETH_LOOP )
							return VOS_OK;
					}
				}
				break;
			case trap_backboneEthLinkdown:
			case trap_backboneEthLinkup:
				if( pAlmMsg->alarmSrc.portAlarmSrc.devIdx != OLT_DEV_ID )
				{
					VOS_ASSERT(0);
					return VOS_OK;
				}
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK )		/* 问题单12130 */
				{
					if( mask & EVENT_MASK_DEV_ETH_LINK )
						return VOS_OK;
				}
				if( getEthPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_ETH_LINK )
						return VOS_OK;
				}
				break;

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
			case trap_e1LosAlarm:
			case trap_e1LosAlarmClear:
				return e1AlarmMaskCheck(pAlmMsg, E1_ALM_LOS);
			case trap_e1LofAlarm:
			case trap_e1LofAlarmClear:
				return e1AlarmMaskCheck(pAlmMsg, E1_ALM_LOF);
			case trap_e1AisAlarm:
			case trap_e1AisAlarmClear:
				return e1AlarmMaskCheck(pAlmMsg, E1_ALM_AIS);
			case trap_e1RaiAlarm:
			case trap_e1RaiAlarmClear:
				return e1AlarmMaskCheck(pAlmMsg, E1_ALM_RAI);
			case trap_e1SmfAlarm:
			case trap_e1SmfAlarmClear:
				return e1AlarmMaskCheck(pAlmMsg, E1_ALM_SMF);
			case trap_e1LomfAlarm:
			case trap_e1LomfAlarmClear:
				return e1AlarmMaskCheck(pAlmMsg, E1_ALM_LOFSMF);
			case trap_e1Crc3Alarm:
			case trap_e1Crc3AlarmClear:
				return e1AlarmMaskCheck(pAlmMsg, E1_ALM_CRC3);
			case trap_e1Crc6Alarm:
			case trap_e1Crc6AlarmClear:
				return e1AlarmMaskCheck(pAlmMsg, E1_ALM_CRC6);

			case trap_tdmServiceAbortAlarm:
				if( tdm_MaskBase & TDM_BASE_ALM_OOS )
					return VOS_OK;
				break;
#endif
			case trap_ethPortBroadCastFloodControl:
      			case  trap_ethPortBroadCastFloodControlClear:
				if( getEthPortAlarmMask(pAlmMsg->alarmSrc.portAlarmSrc.devIdx, pAlmMsg->alarmSrc.portAlarmSrc.brdIdx, pAlmMsg->alarmSrc.portAlarmSrc.portIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_ETH_BCFC )
						return VOS_OK;
				}
				break;

            case trap_switchNewRegSuccess:
            case trap_switchReregSuccess:
            case trap_switchNotPresent:
                if(getOnuSwitchAlarmMask(&mask) == VOS_OK)
                {
                    if(mask & EVENT_MAST_ONU_SWITCH_ALL)
                        return VOS_OK;
                }
                break;
			case trap_logicalSlotInsert:
			case trap_logicalSlotPull:
				break;

			case trap_ponProtectSwitch:
				if( getDeviceAlarmMask( OLT_DEV_ID, &mask ) == VOS_OK ) 
				{
					if( mask  & EVENT_MASK_DEV_PON_ABS )
						return VOS_OK;
				}

				if( getPonPortAlarmMask(OLT_DEV_ID, pAlmMsg->alarmSrc.ponSwitchAlarmSrc.brdIdx, pAlmMsg->alarmSrc.ponSwitchAlarmSrc.ponIdx, &mask) == VOS_OK )
				{
					if( mask & EVENT_MASK_PON_APS )
						return VOS_OK;
				}
				break;
                
			default:
				break;
		}
	}

	return VOS_ERROR;
}

LONG eventBackupProc()
{
	eventLogFlashFileSave();
	
	return VOS_OK;
}
/*--上报告警统一接口--*/
/*注意:在发送之前会去读告警屏蔽的状态，若要发送的告警是被屏蔽的，那么不好意思，将不再上报*/
int eventReportMsgSend( eventMsg_t *pAlmMsg )
{
	int rc = VOS_ERROR;
	ULONG ulMsg[4] = {MODULE_EVENT, FC_EVENT_REPORT, 0, 0};
	SYS_MSG_S * pstMsg = NULL;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(eventMsg_t);

	if( pAlmMsg == NULL )
		return rc;
	if( eventMaskProc(pAlmMsg) == VOS_OK )
		return VOS_OK;
	
	pstMsg = (SYS_MSG_S *)VOS_Malloc( msgLen, MODULE_EVENT ); 
	if( pstMsg == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = MODULE_EVENT;
	pstMsg->ulDstModuleID = MODULE_EVENT;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;/*目的slot*/
	pstMsg->ucMsgType = MSG_REQUEST;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (VOID *)(pstMsg + 1);
	pstMsg->usFrameLen = sizeof(eventMsg_t);
	pstMsg->usMsgCode = FC_EVENT_REPORT;

	VOS_MemCpy( pstMsg->ptrMsgBody, pAlmMsg, sizeof(eventMsg_t) );

	ulMsg[3] = (ULONG)pstMsg;
	rc = VOS_QueSend( eventQueId, ulMsg, NO_WAIT, getEventPriority(pAlmMsg->alarmType, pAlmMsg->alarmId) );
	if( rc != VOS_OK )
	{
		VOS_Free((void*)pstMsg);
	}
	return rc;	
}


/*add by shixh20091124*/
/*STATUS  search_onu_alarm(ULONG alarmId, ULONG *alarm )
{
 	int  rc = VOS_OK;

 	if(alarm==NULL)
 		return VOS_ERROR;

 	switch(alarmId)
	 {
		case trap_devPowerOn:
		case trap_devPowerOff:
			*alarm=0x80000000;
			break;					
		case trap_devFanAlarm:
		case trap_devFanAlarmClear:
			*alarm=0x40000000;
			break;
		case trap_boardCpuUsageAlarm:
		case trap_boardCpuUsageAlarmClear:
			*alarm=0x20000000;
			break;
		case trap_deviceTemperatureHigh:
		case trap_deviceTemperatureHighClear:
		case trap_deviceTemperatureLow:
		case trap_deviceTemperatureLowClear:
			*alarm=0x10000000;
			break;
		case trap_onuNewRegSuccess:
		case trap_onuReregSuccess:
			*alarm=0x08000000;
			break;
		case trap_onuNotPresent:
			*alarm=0x04000000;
			break;
		case trap_ethLinkdown:
		case trap_ethLinkup:
			*alarm=0x02000000;
			break;
		case trap_ethFerAlarm:
		case trap_ethFerAlarmClear:
			*alarm=0x01000000;
			break;
		case trap_ethFlrAlarm:
		case trap_ethFlrAlarmClear:
			*alarm=0x00800000;
			break;
		case trap_ethTranmittalIntermitAlarm:
		case trap_ethTranmittalIntermitAlarmClear:
			*alarm=0x00400000;
			break;
		case trap_ethLoopAlarm:
		case trap_ethLoopAlarmClear:
			*alarm=0x00200000;
			break;
		case trap_ponPortBERAlarm:
		case trap_ponPortBERAlarmClear:
			*alarm=0x00100000;
			break;
		case trap_ponPortFERAlarm:
		case trap_ponPortFERAlarmClear:
			*alarm=0x00080000;
			break;
		case trap_ponPortAbnormal:
			*alarm=0x00040000;
			break;
		case trap_autoProtectSwitch:
			*alarm=0x00020000;
			break;
		case trap_ponToEthLinkdown:
		case trap_ponToEthLinkup:
			*alarm=0x00010000;
			break;
		case trap_ponLaserAlwaysOnAlarm:
		case trap_ponLaserAlwaysOnAlarmClear:
			*alarm=0x00008000;
			break;
		case trap_ponReceiverPowerTooHigh:
		case trap_ponReceiverPowerTooHighClear:
		case trap_ponReceiverPowerTooLow:
		case trap_ponReceiverPowerTooLowClear:
			*alarm=0x00004000;
			break;
		default:
			rc=VOS_ERROR;
			break;

	}

	return rc;
}*/


#if 0
/* begin: added by jianght 20090304  */
extern STATUS epon_e1_cfgdata_save();
extern ULONG get_gfa_e1_slotno();
/* end: added by jianght 20090304 */

static void savePrivateCfgData( void )
{
	V2R1_disable_watchdog();

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	/*	 modified by chenfj 2009-3-24
		配置数据操作已统一, 改回到原来程序
	*/
	if( get_gfa_tdm_slotno() != 0 )
		epon_tdm_cfgdata_save();
	/*else if (V2R1_CTC_STACK)
		saveCtcCfgData();*/
#else
		/*saveCtcCfgData();*/
#endif
	V2R1_enable_watchdog();
}
#endif
/*重启事件上报的时候进行重启的确认*/
static void system_restart_finished( void )
{
	V2R1_disable_watchdog();

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	if( get_gfa_tdm_slotno() != 0 )
	{
		/*epon_tdm_cfgdata_retrieve();*/
	}
	/*else if (V2R1_CTC_STACK)
		retrieveCtcCfgData();*/
#else
		/*retrieveCtcCfgData();*/
#endif
	V2R1_enable_watchdog();
}

int get_gfasw_sys_up_time( sysDateAndTime_t *pTime )
{
	if( pTime == NULL )
		return VOS_ERROR;

	pTime->year = DEVSM_LOCAL_MODULE.starting_time.usYear;
	pTime->month = DEVSM_LOCAL_MODULE.starting_time.usMonth;
	pTime->day = DEVSM_LOCAL_MODULE.starting_time.usDay;
	pTime->hour = DEVSM_LOCAL_MODULE.starting_time.usHour;
	pTime->minute = DEVSM_LOCAL_MODULE.starting_time.usMinute;
	pTime->second = DEVSM_LOCAL_MODULE.starting_time.usSecond;
	/*pTime->week = DEVSM_LOCAL_MODULE.starting_time.usDayOfWeek;
	pTime.reserve = DEVSM_LOCAL_MODULE.starting_time.usMilliseconds*/

	return VOS_OK;
}

/*modyied by shixh20080728*/
ULONG get_olt_module_acting_mode( ULONG slotno )
{
	ULONG mode = 4;
					
       /*if(SlotCardIsSwBoard(slotno)==ROK)
       {
        	if( SYS_MODULE_ISMASTERACTIVE(slotno) )
			mode = 1;
		else if( SYS_MODULE_ISMASTERSTANDBY(slotno) )
			mode = 2;
       }*/
    	if( SYS_MODULE_ISMASTERACTIVE(slotno) )/*--主控--*/
		mode = 1;
	else if( SYS_MODULE_ISMASTERSTANDBY(slotno) )/*--备用主控--*/
		mode = 2;
	else if( (__SYS_MODULE_TYPE__(slotno) != MODULE_TYPE_NULL) &&
		(__SYS_MODULE_TYPE__(slotno) != MODULE_TYPE_UNKNOW) )
		mode = 3;

	return mode;
}

/*----------------------------------------------------------------------------
* 功能: 删除指定ONU/OLT设备的告警日志和告警同步信息
* 输入参数: devIdx－设备索引
* 返回值: 如果存在返回OK，不存在返回ERROR */
int eraseDevEventRecords( ULONG devIdx )
{
	/*eraseDevEventLog( devIdx );*/
	return eraseTrapBacDataByDevIdx( devIdx );
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
* 功能: 	平台事件或告警上报接口	
* 输入参数:	eventId－告警上报ID
		pEventSrcIdx－告警源索引
		idxNum－告警源索引数

		eventId								pEventSrcIdx		idxNum	备注
	--------------------------------------------------------------------------------------
	EVENTID_SYSTEM_COLD_RESTART				－			0		设备冷启动 
	EVENTID_SYSTEM_WARM_RESTART				－			0		设备热启动 
	EVENTID_SNMP_AUTHENTICATION_FAILURE		－			0		SNMP代理认证失败 

	EVENTID_ETH_PORT_LINKUP						ifIndex		1		以太网端口linkdown事件上报告警
	EVENTID_ETH_PORT_LINKDOWN					ifIndex		1		以太网端口linkup事件上报告警

	EVENTID_STP_TOPOLOGYCHANGE				－			0		STP拓扑结构变化 
	EVENTID_STP_NEWROOT						－			0		STP根桥 
	EVENTID_IGMP_JOIN							－			－		TODO 
	EVENTID_IGMP_LEAVE							－			－		TODO 

	EVENTID_BOARD_RESET						slotno		1		板复位 
	EVENTID_BOARD_INSERT						slotno		1		板热插 
	EVENTID_BOARD_PULL							slotno		1		板热拔 
	EVENTID_BOARD_SW_SWITCHOVER				slotno		1		主备倒换 
	EVENTID_BOARD_CPU_UTILIZATION				－			0		主控CPU利用率超门限 

	EVENTID_FLASH_CFGDATA_SAVE_SUCCESS		－			0		配置数据保存flash成功 
	EVENTID_FLASH_CFGDATA_SAVE_FAILURE			－			0		配置数据保存flash失败
	EVENTID_FLASH_CFGDATA_ERASE_SUCCESS		－			0		配置数据擦除flash成功
	EVENTID_FLASH_CFGDATA_ERASE_FAILURE		－			0		配置数据擦除flash失败 

	EVENTID_FTP_DOWNLOAD_OLT_SOFTWARE_SUCCESS	－		0		软件下载(升级)成功(olt)
	EVENTID_FTP_DOWNLOAD_OLT_SOFTWARE_FAILURE	－		0		软件下载(升级)失败(olt)
	EVENTID_FTP_DOWNLOAD_ONU_SOFTWARE_SUCCESS	－		0		软件下载(升级)成功(onu)
	EVENTID_FTP_DOWNLOAD_ONU_SOFTWARE_FAILURE	－		0		软件下载(升级)失败(onu)
	EVENTID_FTP_DOWNLOAD_OLT_HARDWARE_SUCCESS	－		0		固件下载(升级)成功
	EVENTID_FTP_DOWNLOAD_OLT_HARDWARE_FAILURE	－		0		固件下载(升级)失败 
	EVENTID_FTP_DOWNLOAD_OLT_CFGDATA_SUCCESS	－		0		配置数据下载成功 
	EVENTID_FTP_DOWNLOAD_OLT_CFGDATA_FAILURE		－		0		配置数据下载失败 
	EVENTID_FTP_UPLOAD_OLT_CFGDATA_SUCCESS		－		0		配置数据上传成功
	EVENTID_FTP_UPLOAD_OLT_CFGDATA_FAILURE			－		0		配置数据上传失败

	EVENTID_FTP_DOWNLOAD_BOOT_SUCCESS	        	－		0		boot下载(升级)成功 
	EVENTID_FTP_DOWNLOAD_BOOT_FAILURE	         		－		0		boot下载(升级)失败 
	EVENTID_FTP_UPLOAD_BOOT_SUCCESS	         		－		0		boot上传成功 
	EVENTID_FTP_UPLOAD_BOOT_FAILURE	       		－		0		 boot上传失败
	EVENTID_FTP_DOWNLOAD_BATFILE_SUCCESS			－		0		BATFILR下载(升级)成功 
	EVENTID_FTP_DOWNLOAD_BATFILE_FAILURE			－		0		BATFILE下载(升级)失败 
	EVENTID_FTP_UPLOAD_BATFILE_SUCCESS				－		0		BATFILE上传成功 
	EVENTID_FTP_UPLOAD_BATFILE_FAILURE				－		0		BATFILE上传失败 
	EVENTID_FTP_FILETYPE_ERROR						－		0		文件类型错误 


* 返回值: VOS_OK/VOS_ERROR
*/
extern LONG device_standby_master_slotno_get();
extern LONG userport_is_uplink (int slot, int port);
extern STATUS setEthPortLastChangeTime( ulong_t devIdx, ulong_t brdIdx, ulong_t ethIdx, ulong_t ti );
extern LONG hotpull_userport_is_uplink (int slot, int port);
extern LONG VOS_CPU_HighEventRecord(void);
extern LONG VOS_SysMemHighRecord(void);
extern LONG VOS_UsrMemHighRecord(void);

int epon_event_zip_record( ULONG eventId, ULONG *pEventSrcIdx, ULONG idxNum )
{
    if (*pEventSrcIdx != SYS_LOCAL_MODULE_SLOTNO)
        return VOS_OK;
    
    switch(eventId)
    {
        case EVENTID_BOARD_CPU_UTILIZATION:
            VOS_CPU_HighEventRecord();
            break;

        case EVENTID_BOARD_SYSMEM_UTILIZATION:
            VOS_SysMemHighRecord();
            break;

        case EVENTID_BOARD_USRMEM_UTILIZATION:
            VOS_UsrMemHighRecord();
            break;

        default:
            break;
    }

    return VOS_OK;
}
/*功能:事件上报
比如:设备重启，板卡插拔，下载成功失败等等*/
int epon_event_report_callback( ULONG eventId, ULONG *pEventSrcIdx, ULONG idxNum )
{
	int rc = VOS_ERROR;
	ULONG devIdx = OLT_DEV_ID;
	ULONG slotno = 0;
	ULONG portno = 0;
#ifndef _EPON_12EPON_SUPPORT_
	ULONG swPort = 0;
#endif
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	ULONG tdmidx=0;
#endif

	/* modified by xieshl 20120924, 备用主控不再直接产生告警，问题单15883 */
	if( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
		return VOS_OK;

	switch( eventId )
	{
		case EVENTID_SYSTEM_COLD_RESTART:
			/*return oltMib2_EventReport( trap_coldStart );*/
            /* B--added by liwei056@2011-1-27 for PasSoftBug-InSwBoard */
            if ( PRODUCT_E_EPON3 == SYS_PRODUCT_TYPE )
            /* E--added by liwei056@2011-1-27 for PasSoftBug-InSwBoard */
            {
    			system_restart_finished();
            }
			rc = deviceColdStart_EventReport( devIdx );
			break;
			
		case EVENTID_SYSTEM_WARM_RESTART:
			/*return oltMib2_EventReport( trap_warmStart );*/
			system_restart_finished();
			rc = deviceWarmStart_EventReport( devIdx );
			break;
			
		case EVENTID_SNMP_AUTHENTICATION_FAILURE:
			rc = oltMib2_EventReport( trap_authenticationFailure );
			break;

		case EVENTID_ETH_PORT_LINKUP:
			if( IFM_IFINDEX_GET_TYPE(*pEventSrcIdx) == IFM_ETH_TYPE )
			{
				portno = SYS_IF_PORT_ID(*pEventSrcIdx);

				/* modified by xieshl 20110705, 增加带外网管口告警上报，问题单12083 */
				if( portno == 0 )	/* modified by xieshl 20080327, 暂时屏蔽带外网管口 */
				{
					rc = ethLinkup_EventReport( OLT_DEV_ID, SYS_LOCAL_MODULE_SLOTNO, portno  );    /* 后面两个参数应该需要修改，特此记下 kkkkkkkk */
					/*sys_console_printf( "interface NM-E linkup\r\n" );*/
					return VOS_OK;
				}
				/*else if( swPort > MAX_ETH_PORT_NUM )
					return VOS_ERROR;*/
				
#ifdef _EPON_12EPON_SUPPORT_   /* modified by duzhk 2011-12-20*/
				slotno = IFM_ETH_GET_SLOT( *pEventSrcIdx );
				portno = IFM_ETH_GET_PORT( *pEventSrcIdx );
#else
				if( swport_2_slot_port( swPort, &slotno, &portno ) == VOS_ERROR )
					return VOS_ERROR;
#endif
				setEthPortLastChangeTime( OLT_DEV_ID, slotno, portno, 0 );

				if(SYS_MODULE_IS_UPLINK(slotno))
				{
					rc = backboneEthLinkup_EventReport( OLT_DEV_ID, slotno, portno );
				}
				else if( SlotCardIsPonBoard(slotno)==ROK)
				{
					/* modified by xieshl 20120206, 区分12EPON板上的4个上联口，问题单14476 */
#ifdef _EPON_12EPON_SUPPORT_
					if( userport_is_uplink(slotno, portno) == VOS_YES )
					{
						if( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_12EPON_M )
							rc = backboneEthLinkup_EventReport( OLT_DEV_ID, slotno, portno );
						else
							rc = ethLinkup_EventReport( OLT_DEV_ID, slotno, portno );/***modified by @muqw for keep pace witn 6900 2015-04-24***/
                           /* rc = backboneEthLinkup_EventReport( OLT_DEV_ID, slotno, portno );*/ 
					}
					else
#endif
					{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
						RestoreEthPortMacAndVlan( slotno, portno );
#endif
						rc = ponToEthLinkup_EventReport( devIdx, slotno, portno );
					}
				}
				/*除了12EPON之外，8100上面也有四个上联口，是不是也要考虑加上，added by yanjy,2017-03*/
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
				else if( SlotCardIsTdmBoard(slotno) == ROK )
				{
					rc = RestoreEthPortMacAndVlan( slotno, portno );
					/* todo 增加TDM告警 */
				}
#endif
				else
				{
					rc = ethLinkup_EventReport( devIdx, slotno, portno  );
				}
			}
			break;
		case EVENTID_ETH_PORT_LINKDOWN	:
			if( IFM_IFINDEX_GET_TYPE(*pEventSrcIdx) == IFM_ETH_TYPE )
			{
				portno = SYS_IF_PORT_ID(*pEventSrcIdx);

				/* modified by xieshl 20110705, 增加带外网管口告警上报，问题单12083 */
				if( portno == 0 )	/* modified by xieshl 20080328, 暂时屏蔽带外网管口 */
				{
					rc = ethLinkdown_EventReport( OLT_DEV_ID, SYS_LOCAL_MODULE_SLOTNO, portno  );    /* 后面两个参数应该需要修改，特此记下 kkkkkkkk */
					/*sys_console_printf( "interface NM-E linkdown\r\n" );*/
					return VOS_OK;
				}
				/*else if( swPort > MAX_ETH_PORT_NUM )
					return VOS_ERROR;*/
#ifdef _EPON_12EPON_SUPPORT_   /* modified by duzhk 2011-12-20*/
				slotno = IFM_ETH_GET_SLOT( *pEventSrcIdx );
				portno = IFM_ETH_GET_PORT( *pEventSrcIdx );
#else
				if( swport_2_slot_port( swPort, &slotno, &portno ) == VOS_ERROR )
					return VOS_ERROR;
#endif
				setEthPortLastChangeTime( OLT_DEV_ID, slotno, portno, 0 );

                #if 0/***modified by @muqw 2014-12-31***/
				if(SYS_MODULE_IS_UPLINK(slotno) || (SYS_PULL_MODULE_IS_UPLINK(slotno)))
				{
					rc = backboneEthLinkdown_EventReport( OLT_DEV_ID, slotno, portno );
				}
				else if( SlotCardIsPonBoard(slotno)==ROK || SYS_PULL_MODULE_IS_PON(slotno))
				{
					/* modified by xieshl 20120206, 区分12EPON板上的4个上联口，问题单14476 */
#ifdef _EPON_12EPON_SUPPORT_
					if( userport_is_uplink(slotno, portno) == VOS_YES )
					{
						if( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_12EPON_M )
							rc = backboneEthLinkdown_EventReport( OLT_DEV_ID, slotno, portno );
						else
						rc = ethLinkdown_EventReport( OLT_DEV_ID, slotno, portno );
					}
					else
#endif
						rc = ponToEthLinkdown_EventReport( devIdx, slotno, portno  );
				}
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
				else if(SlotCardIsTdmBoard(slotno)==ROK )
				{
					rc = tdmToEthLinkdown_EventReport( devIdx, slotno, tdmidx);
				}
#endif
				else
				{
				    rc = backboneEthLinkdown_EventReport( OLT_DEV_ID, slotno, portno );
					/*rc = ethLinkdown_EventReport( devIdx, slotno, portno  );*/ /***modified by @muqw 2014-12-18***/
				}
                #else
                if(SYS_MODULE_IS_UPLINK(slotno) || (SYS_MODULE_TYPE_IS_UPLINK(SYS_HOT_PULL_MODULE_TYPE(slotno))))
				{
					rc = backboneEthLinkdown_EventReport( OLT_DEV_ID, slotno, portno );
				}
				else if( SlotCardIsPonBoard(slotno)==ROK || SYS_MODULE_TYPE_IS_PON(SYS_HOT_PULL_MODULE_TYPE(slotno)))
				{
					/* modified by xieshl 20120206, 区分12EPON板上的4个上联口，问题单14476 */
#ifdef _EPON_12EPON_SUPPORT_
					if( (hotpull_userport_is_uplink(slotno, portno)==VOS_YES)||(userport_is_uplink(slotno, portno)==VOS_YES))
					{
					    /*modified by @muqw for keep pace with 6900 2015-04-25*/
						if(SYS_HOT_PULL_MODULE_TYPE(slotno) == MODULE_E_GFA6900_12EPON_M )
							rc = backboneEthLinkdown_EventReport( OLT_DEV_ID, slotno, portno );
						else
						    rc = ethLinkdown_EventReport( OLT_DEV_ID, slotno, portno );
					}
					else
#endif
						rc = ponToEthLinkdown_EventReport( devIdx, slotno, portno  );
				}
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
				else if(SlotCardIsTdmBoard(slotno)==ROK )
				{
					rc = tdmToEthLinkdown_EventReport( devIdx, slotno, tdmidx);
				}
#endif
				else
				{
				    rc = backboneEthLinkdown_EventReport( OLT_DEV_ID, slotno, portno );
					/*rc = ethLinkdown_EventReport( devIdx, slotno, portno  );*/ /***modified by @muqw 2014-12-18***/
				}
                #endif
			}
			break;

		case EVENTID_STP_TOPOLOGYCHANGE:
			rc = oltStp_EventReport(trap_topologyChange);
			break;
		case EVENTID_STP_NEWROOT:
			rc = oltStp_EventReport(trap_newRoot);
			break;
			
		case EVENTID_IGMP_JOIN:
		case EVENTID_IGMP_LEAVE:
			break;

		case EVENTID_BOARD_RESET:
			slotno = *pEventSrcIdx;
			rc = ponBoardReset_EventReport( devIdx, slotno );
			break;
		case EVENTID_BOARD_INSERT:
			slotno = *pEventSrcIdx;
			if( slotno <= SYS_CHASSIS_SLOTNUM )
				rc = devBoardInterted_EventReport( devIdx, slotno, SYS_MODULE_TYPE(slotno) );
			else
				rc = VOS_OK;
			break;
		case EVENTID_BOARD_PULL:
			slotno = *pEventSrcIdx;
			if( slotno <= SYS_CHASSIS_SLOTNUM)
				rc = devBoardPull_EventReport( devIdx, slotno, SYS_MODULE_TYPE(slotno) );
			else 
				rc = VOS_OK;
			break;
		case EVENTID_BOARD_SW_SWITCHOVER:
			slotno = *pEventSrcIdx;
			if( !SYS_SLOTNO_IS_ILLEGAL(slotno) && SYS_MODULE_WORKMODE_ISMASTER(slotno) ) /* modified by xieshl 20110720, 问题单13308 */
				rc = swBoardProtectedSwitch_EventReport( devIdx, slotno );
			else
				rc = VOS_OK;
			break;
			
		case EVENTID_BOARD_CPU_UTILIZATION:
			/*return cpuUsageFactorHigh_EventReport( devIdx );*/
			slotno = *pEventSrcIdx;
			rc = cpuUsageHigh_EventReport( devIdx, slotno );
			break;
		case EVENTID_BOARD_CPU_UTILIZATION_CLEAR:
			slotno = *pEventSrcIdx;
			rc = cpuUsageHighClear_EventReport( devIdx, slotno );
			break;

	/*add by wangdp 20100106*/		
		case EVENTID_BOARD_SYSMEM_UTILIZATION:
			slotno = *pEventSrcIdx;
			rc = boardMemoryUsageHigh_EventReport( devIdx, slotno );
			break;
		case EVENTID_BOARD_SYSMEM_UTILIZATION_CLEAR:
			slotno = *pEventSrcIdx;
			rc = boardMemoryUsageHighClear_EventReport( devIdx, slotno );
			break;
		case EVENTID_BOARD_USRMEM_UTILIZATION:
			slotno = *pEventSrcIdx;
			rc = boardMemoryUsageHigh_EventReport( devIdx, slotno );
			break;
		case EVENTID_BOARD_USRMEM_UTILIZATION_CLEAR:
			slotno = *pEventSrcIdx;
			rc = boardMemoryUsageHighClear_EventReport( devIdx, slotno );	
			break;

		case EVENTID_FLASH_CFGDATA_SAVE_SUCCESS:
			if( (pEventSrcIdx == NULL) || ((int)pEventSrcIdx == 1) || (idxNum == 0)  )
			{
				/*ULONG args[10] = { 0 };
				VOS_TaskCreate( "tctcsave", TASK_PRIORITY_NORMAL, saveCtcCfgData, args );*/
                /* B--added by liwei056@2011-1-27 for PasSoftBug-InSwBoard */
                if ( PRODUCT_E_EPON3 == SYS_PRODUCT_TYPE )
                /* E--added by liwei056@2011-1-27 for PasSoftBug-InSwBoard */
                {
                    /*commented by wangxiaoyu 2011-11-11,改为配置数据管理模式后，不再保存原来格式的CTC配置数据及TDM配置数据
    				savePrivateCfgData();
    				*/
                }
				/*
				if( SYS_LOCAL_MODULE_ISMASTERACTIVE && (device_standby_master_slotno_get() != 0) )
					config_sync_notify_event();
					*/

				rc = cfgDataSaveSuccess_EventReport( devIdx );
			}
			break;
		case EVENTID_FLASH_CFGDATA_SAVE_FAILURE:
			if( (pEventSrcIdx == NULL) || ((int)pEventSrcIdx == 1) || (idxNum == 0)  )
			{
				rc = cfgDataSaveFail_EventReport( devIdx );
			}
			break;
		case EVENTID_FLASH_CFGDATA_ERASE_SUCCESS:
			if( (pEventSrcIdx == NULL) || ((int)pEventSrcIdx == 1) || (idxNum == 0)  )
			{
				/* begin: added by jianght 20090531 */
				/* 使命令erase config-file 能擦除E1配置数据 */
				dataSync_FlashEraseFile(1/*SYNC_FILETYPE_CFG_TDM*/);
				/* end: added by jianght 20090531 */

                /* begin: added by wangxiaoyu 2011-09-09 for erase onu config on master standby*/
                /*dataSync_FlashEraseFile(SYNC_FILETYPE_CFG_CTC);
                dataSync_FlashEraseFile(SYNC_FILETYPE_CFG_DATA);*/
				/*
                if( SYS_LOCAL_MODULE_ISMASTERACTIVE && (device_standby_master_slotno_get() != 0) )
                    config_sync_notify_event();
                  */
                /* end:  added by wangxiaoyu 2011-09-09*/

				rc = flashClearSuccess_EventReport( devIdx );
			}
			break;
		case EVENTID_FLASH_CFGDATA_ERASE_FAILURE:
			if( (pEventSrcIdx != NULL) && (idxNum != 0) )
			{
				rc = flashClearFail_EventReport( devIdx );
			}
			break;

		case EVENTID_FTP_DOWNLOAD_OLT_SOFTWARE_SUCCESS:
		case EVENTID_FTP_UPLOAD_OLT_SOFTWARE_SUCCESS:
			rc = softwareUpdateSuccess_EventReport( devIdx );
			break;
		case EVENTID_FTP_DOWNLOAD_OLT_SOFTWARE_FAILURE:
		case EVENTID_FTP_UPLOAD_OLT_SOFTWARE_FAILURE:
			rc = softwareUpdateFail_EventReport( devIdx );
			break;
		case EVENTID_FTP_DOWNLOAD_OLT_HARDWARE_SUCCESS:
		case EVENTID_FTP_UPLOAD_OLT_HARDWARE_SUCCESS:
			rc = firmwareUpdateSuccess_EventReport( devIdx );
			break;
		case EVENTID_FTP_DOWNLOAD_OLT_HARDWARE_FAILURE:
		case EVENTID_FTP_UPLOAD_OLT_HARDWARE_FAILURE:
			rc = firmwareUpdateFail_EventReport( devIdx );
			break;
		/*add by shixh20090826*/
		case EVENTID_FTP_DOWNLOAD_OLT_DBA_SUCCESS:
		case EVENTID_FTP_UPLOAD_OLT_DBA_SUCCESS:
			rc = dbaUpdateSuccess_EventReport( devIdx );
			break;
		case EVENTID_FTP_DOWNLOAD_OLT_DBA_FAILURE:
		case EVENTID_FTP_UPLOAD_OLT_DBA_FAILURE:
			rc = dbaUpdateFailure_EventReport( devIdx );
			break;
			
		case EVENTID_FTP_DOWNLOAD_OLT_CFGDATA_SUCCESS:
			rc = cfgDataRestoreSuccess_EventReport( devIdx );
			break;
		case EVENTID_FTP_DOWNLOAD_OLT_CFGDATA_FAILURE:
			rc = cfgDataRestoreFail_EventReport( devIdx );
			break;
		case EVENTID_FTP_UPLOAD_OLT_CFGDATA_SUCCESS:
		/*case EVENTID_FTP_DOWNLOAD_OLT_CFGDATA_SUCCESS:added by yanjy.暂时先这样吧，等以后扩充了单独的trapID再分开*/
			rc = cfgDataBackupSuccess_EventReport( devIdx );
			break;
		case EVENTID_FTP_UPLOAD_OLT_CFGDATA_FAILURE:
		/*case EVENTID_FTP_DOWNLOAD_OLT_CFGDATA_FAILURE:added by yanjy.暂时先这样吧，等以后扩充了单独的trapID再分开*/
			rc = cfgDataBackupFail_EventReport( devIdx );
			break;
		case EVENTID_FTP_DOWNLOAD_ONU_SOFTWARE_SUCCESS:
		case EVENTID_FTP_UPLOAD_ONU_SOFTWARE_SUCCESS:/*added by yanjy*/
			rc = onuSoftwareLoadSuccess_EventReport( devIdx );
			break;
		case EVENTID_FTP_DOWNLOAD_ONU_SOFTWARE_FAILURE:
		case EVENTID_FTP_UPLOAD_ONU_SOFTWARE_FAILURE:/*added by yanjy.onu 上传文件没有告警提示，问题单23178*/
			rc = onuSoftwareLoadFailure_EventReport( devIdx );
			break;

		case EVENTID_FTP_DOWNLOAD_BOOT_SUCCESS:		/* boot下载(升级)成功 */
		case EVENTID_FTP_UPLOAD_BOOT_SUCCESS:			/* boot上传成功 */
			rc = bootUpdateSuccess_EventReport( devIdx );
			break;
		case EVENTID_FTP_DOWNLOAD_BOOT_FAILURE:		/* boot下载(升级)失败 */
		case EVENTID_FTP_UPLOAD_BOOT_FAILURE:			/* boot上传失败 */
			rc = bootUpdateFailure_EventReport( devIdx );
			break;
		case EVENTID_FTP_DOWNLOAD_BATFILE_SUCCESS:	/* BATFILR下载(升级)成功 */
			rc = batfileBackupSuccess_EventReport( devIdx );
			break;
		case EVENTID_FTP_DOWNLOAD_BATFILE_FAILURE:		/* BATFILE下载(升级)失败 */
			rc = batfileBackupFailure_EventReport( devIdx );
			break;
		case EVENTID_FTP_UPLOAD_BATFILE_SUCCESS:		/* BATFILE上传成功 */
			rc = batfileRestoreSuccess_EventReport( devIdx );
			break;
		case EVENTID_FTP_UPLOAD_BATFILE_FAILURE:		/* BATFILE上传失败 */
			rc = batfileRestoreFailure_EventReport( devIdx );
			break;
		case EVENTID_FTP_FILETYPE_ERROR:					/*文件类型错误 */
			/* TODO */
			break;
		/* added by xieshl 20090907 */
		case EVENTID_FTP_DOWNLOAD_SYSFILE_SUCCESS:		/*  */
			rc = onuSysFileDownloadSuccess_EventReport( devIdx );
			break;
		case EVENTID_FTP_DOWNLOAD_SYSFILE_FAILURE:		/*  */
			rc = onuSysFileDownloadFailure_EventReport( devIdx );
			break;
		case EVENTID_FTP_UPLOAD_SYSFILE_SUCCESS:		/*  */
			rc = onuSysFileUploadSuccess_EventReport( devIdx );
			break;
		case EVENTID_FTP_UPLOAD_SYSFILE_FAILURE:		/*  */
			rc = onuSysFileUploadFailure_EventReport( devIdx );
			break;
		default:
			break;
	}
	return rc;
}

/* modified by xieshl 20121114, 问题单16277 */
BOOL epon_event_alarm_mask_check( ULONG eventId, ULONG *pEventSrcIdx, ULONG idxNum )
{
	BOOL rc = VOS_YES;
	ULONG devIdx = OLT_DEV_ID;
	ULONG mask = 0;

	if( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
		return VOS_OK;

	switch( eventId )
	{
		case EVENTID_ETH_PORT_LINKUP:
		case EVENTID_ETH_PORT_LINKDOWN	:
			break;

		case EVENTID_BOARD_CPU_UTILIZATION:
		case EVENTID_BOARD_CPU_UTILIZATION_CLEAR:
			if( getDeviceAlarmMask( devIdx, &mask ) == VOS_OK )
			{
				if( mask & EVENT_MASK_DEV_CPU )
					rc = VOS_NO;
			}
			break;

		case EVENTID_BOARD_SYSMEM_UTILIZATION:
		case EVENTID_BOARD_SYSMEM_UTILIZATION_CLEAR:
		case EVENTID_BOARD_USRMEM_UTILIZATION:
		case EVENTID_BOARD_USRMEM_UTILIZATION_CLEAR:
			break;


		default:
			break;
	}
	return rc;
}


