
/**
*  modified by chenfj 2009-4-13 
*    1 命令行中应能指定升级(配置) 某一类型ONU; 这会很有用, 每当有ONU 出了新的版本时
*    2 有多个CLI调用的函数, 输出信息使用的是sys_console_printf, 应改为vty_out
*    3 对函数调用, 需判断其返回值
*    4 由于命令行和snmp 都可以操作数据库, 建议使用信号量
*    5 程序的开始, 增加了对数据库的初始化
*/

#ifdef	__cplusplus
extern "C"
{
#endif

#include  "OltGeneral.h"
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "gwEponSys.h"
#include  "Onu_oam_comm.h"
/*#include "../man/cli/cli.h"
#include "../superset/platform/include/man/cli/Cl_vty.h"*/
/*#include "tdm_apis.h"*/
/*#include "vos/vospubh/vos_sysmsg.h"*/
#include "vos/vospubh/vos_byteorder.h"/*add by shixh20090520*/

/*#include "sys/main/sys_main.h"*/
#include "../cli/V2R1debug_cli.h"
/*#include "V2R1_product.h"*/

#include "Onuatuocfg.h"
#include "eventOam.h"
#include "Cdp_pub.h"

auto_load_ftpserver_t  auto_load_ftpserver;
auto_load_onuctrl_t  auto_load_onuctrl[AUTO_LOAD_FTP_CLIENT_IPNUM];

auto_load_onu_list_t (*auto_load_onu_list)[MAXONUPERPONNOLIMIT];
auto_load_filetype_list_t  auto_load_filetype_list[V2R1_ONU_MAX];/*问题单8548*/

ULONG  autoLoad_upg_enable = 0;			/*自动升级使能*/
ULONG  autoLoad_cfg_enable = 0;			/*自动配置使能*/
sysDateAndTime_t  autoLoad_startTime = {0};	/*启动时间*/
sysDateAndTime_t  autoLoad_endTime = {0};	      /*结束时间*/
ULONG autoLoad_upg_startFlag = 0;			/*自动升级开始标志*/
ULONG autoLoad_cfg_startFlag = 0;			/*自动配置开始标志*/
ULONG  autoload_onu_debug_flag = 0;

ULONG autoLoad_aging_times = 5;		/* 60s */
ULONG autoLoad_timer_interval = 10000;

extern  ONUTable_S  *OnuMgmtTable;

extern int parse_onuidx_command_parameter( ULONG devIdx, PON_olt_id_t *pPonIdx, PON_onu_id_t *pOnuIdx);
extern LONG IFM_ParseSlotPort( CHAR * szName, ULONG * pulSlot, ULONG * pulPort );
extern long IpListToIp( const char * IpList, long * ulIpAddr, long * ulMask );
extern int CheckIpValid( long ipaddr, long mask, struct vty * vty );
extern long get_long_from_ipdotstring(char* ipaddr);
extern int  SearchOnuTypeFullMatch( char *TypeString);
extern short int parseOnuIndexFromDevIdx( ULONG devIdx, ULONG * pPonIdx, ULONG *pOnuIdx );

ULONG onuAutoLoadSemId = 0;/*add byshixh@20080704*/
LONG   onuAutoLoadTimerId = VOS_ERROR;
ULONG onuAutoLoadQueId = 0;
LONG  onuAutoLoadTaskId = 0;
LONG  onuAutoLoadInitFlag = 0;


LONG  autoload_cheak_onu_file_is_null( auto_load_msg_t *pAutoLoadMsg );
LONG  autoload_onu_filetype_get( auto_load_msg_t *pAutoLoadMsg, UCHAR file_type[AUTO_LOAD_FILETYPE_COUNT] );
LONG  autoload_onu_ver_compare( auto_load_msg_t *pAutoLoadMsg,UCHAR version[AUTO_LOAD_FILETYPE_COUNT][32],UCHAR file_Name[AUTO_LOAD_FILETYPE_COUNT][64]);

LONG  onuAutoLoadFreeIp(ULONG onudevIdx, ULONG loadType, ULONG cfgStatus, ULONG upgStatus);
LONG  onuAutoLoadGetIp(int *resId);
LONG autoload_onu_status_in_processing();
LONG autoload_upg_onu_status_restart();
LONG autoload_cfg_onu_status_reset();
LONG autoload_upg_onu_status_abort();
LONG autoload_cfg_onu_status_abort();


VOID onuAutoLoadTask(void);
LONG onuAutoLoadStartupMsgProcess();
LONG onuAutoLoadAgingMsgProcess();
LONG onuAutoLoadConfigMsgProcess( auto_load_msg_t *pAutoLoadMsg );
LONG onuAutoLoadDeregisterMsgProcess( auto_load_msg_t *pAutoLoadMsg );
LONG onuAutoLoadCompletedMsgProcess( auto_load_msg_t *pAutoLoadMsg );

LONG onuAutoLoadIpResourceAssign( auto_load_resource_msg_t *pIpResMsg );
LONG onuAutoLoadIpResourceRelease( auto_load_msg_t *pAutoLoadMsg );
LONG onuAutoLoadConfigMsg2Master( auto_load_msg_t *pAutoLoadMsg );
LONG onuAutoLoadCompletedMsg2Master( auto_load_msg_t *pAutoLoadMsg );
LONG onuAutoLoadIpResourceAssignOam2Slave( auto_load_resource_msg_t *pIpResMsg );
LONG onuAutoLoadIpResourceAssignOamPduCreate( auto_load_resource_msg_t *pIpResMsg );
LONG onuAutoLoadIpResourceAssignOamSend( auto_load_resource_msg_t *pIpResMsg );
LONG onuAutoLoadIpResourceReleaseOam2Slave( auto_load_msg_t *pAutoLoadMsg );
LONG onuAutoLoadIpResourceReleaseOamSend( auto_load_msg_t *pAutoLoadMsg );
LONG autoload_module_init();
char * autoload_cfg_onu_status_str( LONG state );

/*#define RPU_TID_CDP_AUTO_LOAD		RPU_TID_CDP_HA*/
#define  TASK_PRIORITY_ONU_AUTOCONFIG  96

int onuAutoLoadInit()
{
	ULONG ulSize;
/*
#ifdef g_malloc
#undef g_malloc
#endif
	extern void *	g_malloc (size_t __size);
*/

	ulSize = sizeof(auto_load_onu_list_t) * (MAXPON*MAXONUPERPONNOLIMIT)/*MAXONU*/;
	if ( NULL == (/*(auto_load_onu_list_t *)*/auto_load_onu_list = (auto_load_onu_list_t *)g_malloc(ulSize)) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	VOS_MemZero( auto_load_onu_list, ulSize );
	VOS_MemZero( (VOID*)&auto_load_onuctrl[0], sizeof(auto_load_onuctrl) );
	VOS_MemZero( (VOID *)&auto_load_ftpserver, sizeof(auto_load_ftpserver) );
	VOS_MemZero( (VOID *)&auto_load_filetype_list[0], sizeof(auto_load_filetype_list) );

	if( onuAutoLoadSemId == 0 )
		onuAutoLoadSemId = VOS_SemMCreate( VOS_SEM_Q_FIFO );

	if( onuAutoLoadQueId == 0 )  
		onuAutoLoadQueId = VOS_QueCreate( MAXONUAUTOCONFIGMSGNUM , VOS_MSG_Q_PRIORITY );
	if( onuAutoLoadTaskId == 0 )  
		onuAutoLoadTaskId = VOS_TaskCreate("tAutoLoad", TASK_PRIORITY_ONU_AUTOCONFIG, (VOS_TASK_ENTRY)onuAutoLoadTask, NULL );

	VOS_QueBindTask(( VOS_HANDLE ) onuAutoLoadTaskId,  onuAutoLoadQueId );

	if( VOS_OK != CDP_Create( RPU_TID_CDP_AUTO_LOAD, CDP_NOTI_VIA_QUEUE, onuAutoLoadQueId, NULL) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	/*onuAutoLoadCommandInstall();*/
	autoload_module_init();
	
	return VOS_OK;
}

/*ONU 自动加载入口函数*/
VOID onuAutoLoadTask( void )
{
	LONG  result_msg;
	ULONG ulRcvMsg[4];
	SYS_MSG_S *pMsg;
	auto_load_msg_t *pAutoLoadMsg;

	if( onuAutoLoadInitFlag )
		return;
	onuAutoLoadInitFlag = 1;

	/*while( !SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
	{
		VOS_TaskDelay( VOS_TICK_SECOND);
	}*/

	VOS_TimerCreate(MODULE_ONU_AUTO_LOAD, 0, 60000, (VOID *)onuAutoLoadAgingTimerCallback, NULL, VOS_TIMER_LOOP);
	
	while(1)
	{
		result_msg = VOS_QueReceive( onuAutoLoadQueId, ulRcvMsg ,WAIT_FOREVER );
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

		if ( CDP_NOTI_FLG_SEND_FINISH == ulRcvMsg[1] )
		{
			CDP_FreeMsg( pMsg ); 	   /* cdp 异步发送完成*/
			continue;
		}

		pAutoLoadMsg = (auto_load_msg_t*)(pMsg + 1);

		switch( pMsg->usMsgCode )
		{
			case AUTO_LOAD_MSG_CODE_STARTUP_TIMER:
				onuAutoLoadStartupMsgProcess();
				break;
			case AUTO_LOAD_MSG_CODE_AGING_TIMER:
				onuAutoLoadAgingMsgProcess();
				break;
			case AUTO_LOAD_MSG_CODE_ONU_REGISTER:
				if( pAutoLoadMsg )
					onuAutoLoadConfigMsgProcess( pAutoLoadMsg );
				break;
			case AUTO_LOAD_MSG_CODE_ONU_DEREGISTER:
				if( pAutoLoadMsg )
					onuAutoLoadDeregisterMsgProcess( pAutoLoadMsg );
				break;
			case AUTO_LOAD_MSG_CODE_ONU_COMPLETED:
				if( pAutoLoadMsg )
				{
					onuAutoLoadIpResourceReleaseOamSend(pAutoLoadMsg);
                    /* removed by luh 2012-9-14*/
#if 0
					if((RecvMsgFromOnu[0] == AUTO_LOAD_OAM_FINISH_RSP) && (RecvMsgFromOnu[1] == 0))
						pAutoLoadMsg->data[1] = 1;
					else
						pAutoLoadMsg->data[1] = 0;
#endif
                    /*modi by luh @2014-10-11. 使用抽象化统一判断*/
#if 0
					if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER )
					{
						onuAutoLoadCompletedMsgProcess( pAutoLoadMsg );
					}
					else
					{
						onuAutoLoadCompletedMsg2Master( pAutoLoadMsg );
					}
#else
                    if(SYS_LOCAL_MODULE_TYPE_IS_CPU_PON && !SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
					{
						onuAutoLoadCompletedMsg2Master( pAutoLoadMsg );
					}
                    else
                    {
						onuAutoLoadCompletedMsgProcess( pAutoLoadMsg );
                    }
#endif
				}
				break;
			case AUTO_LOAD_MSG_CODE_CDP_ONU_COMPLETED:
				if( pAutoLoadMsg )
					onuAutoLoadCompletedMsgProcess( pAutoLoadMsg );
				break;

			case AUTO_LOAD_MSG_CODE_CDP_IP_RESOURCE:
				if( pAutoLoadMsg )
					onuAutoLoadIpResourceAssignOamSend( (auto_load_resource_msg_t *)pAutoLoadMsg );
				break;
			case AUTO_LOAD_MSG_CODE_CDP_IP_RELEASE:
				if( pAutoLoadMsg )
					onuAutoLoadIpResourceReleaseOamSend( pAutoLoadMsg );
				break;

			case AUTO_LOAD_MSG_CODE_ONU_REQUEST:
				if( pAutoLoadMsg )
				{
                    /*modi by luh @2014-10-11. */
#if 0                    
					if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER )
					{
						onuAutoLoadConfigMsgProcess( pAutoLoadMsg );
					}
					else
					{
						onuAutoLoadConfigMsg2Master( pAutoLoadMsg );
					}
#else
                    if(SYS_LOCAL_MODULE_TYPE_IS_CPU_PON && !SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
                    {
						onuAutoLoadConfigMsg2Master( pAutoLoadMsg );
                    }
                    else
                    {
						onuAutoLoadConfigMsgProcess( pAutoLoadMsg );
                    }
#endif
				}
				break;
			case AUTO_LOAD_MSG_CODE_CDP_ONU_REQUEST:
				if( pAutoLoadMsg )
					onuAutoLoadConfigMsgProcess( pAutoLoadMsg );
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
	onuAutoLoadInitFlag = 0;
}


static LONG onuAutoLoadMsgSend( ULONG msgCode, ULONG onuDevIdx, ULONG result )
{
	ULONG aulMsg[4] = { MODULE_ONU_AUTO_LOAD, 0, 0, 0 };
	SYS_MSG_S * pstMsg = NULL;
	auto_load_msg_t *pAutoLoadMsg;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(auto_load_msg_t);
	LONG result_msg;

	pstMsg = ( SYS_MSG_S* ) VOS_Malloc( msgLen, MODULE_ONU_AUTO_LOAD );
	if ( NULL == pstMsg )
	{
		return VOS_ERROR;
	}

	pAutoLoadMsg = (auto_load_msg_t *)(pstMsg + 1);

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = MODULE_ONU_AUTO_LOAD;
	pstMsg->ulDstModuleID = MODULE_ONU_AUTO_LOAD;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;/*目的slot*/
	pstMsg->ucMsgType = MSG_REQUEST;
	pstMsg->usMsgCode = msgCode;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = pAutoLoadMsg;
	pstMsg->usFrameLen = sizeof(auto_load_msg_t);

	pAutoLoadMsg->onuDevIdx = onuDevIdx;
	pAutoLoadMsg->result = result;
	if( onuDevIdx != 0 )
	{
		pAutoLoadMsg->slot  = GET_PONSLOT(onuDevIdx);
		pAutoLoadMsg->pon = GET_PONPORT(onuDevIdx);
		pAutoLoadMsg->onu = GET_ONUID(onuDevIdx);
	}

	aulMsg[1] = msgCode;
	aulMsg[3] = (ULONG)pstMsg;

	result_msg = VOS_QueSend(onuAutoLoadQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL);
	if( result_msg != VOS_OK )
	{
		VOS_ASSERT(0);
		VOS_Free( pstMsg );
	}
	return result_msg;
}

void onuAutoLoadStartupTimerCallback()
{
	onuAutoLoadMsgSend( AUTO_LOAD_MSG_CODE_STARTUP_TIMER, 0, 0 );
}

void onuAutoLoadAgingTimerCallback()
{
	onuAutoLoadMsgSend( AUTO_LOAD_MSG_CODE_AGING_TIMER, 0, 0 );
}

 /*
当发现一个新的onu 注册时，调用该函数，给
onu 自动配置队列发一个消息。
*/
LONG onuAutoLoadOnuNewRegisterCallback( ULONG onuDevIdx )
{
	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return onuAutoLoadMsgSend( AUTO_LOAD_MSG_CODE_ONU_REGISTER, onuDevIdx, 0 );
	return VOS_OK;
}

LONG onuAutoLoadOnuRequestCallback( ULONG onuDevIdx )
{
	return onuAutoLoadMsgSend( AUTO_LOAD_MSG_CODE_ONU_REQUEST, onuDevIdx, 0 );
}

LONG onuAutoLoadOnuDeregisterCallback( ULONG onuDevIdx )
{
	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return onuAutoLoadMsgSend( AUTO_LOAD_MSG_CODE_ONU_DEREGISTER, onuDevIdx, onuLoadFile_timeout );	/* modified by xieshl 20110722, 问题单13267 */
	return VOS_OK;
}

LONG onuAutoLoadOnuCompletedCallback( ULONG onuDevIdx, ULONG result )
{
	return onuAutoLoadMsgSend( AUTO_LOAD_MSG_CODE_ONU_COMPLETED, onuDevIdx, result );
}

LONG onuAutoLoadOnuUpgradingCallback (ULONG onuDevIdx)
{
	return onuAutoLoadMsgSend( AUTO_LOAD_MSG_CODE_ONU_UPGRADING, onuDevIdx, 0 );
}

/*启动定时消息*/
LONG onuAutoLoadStartupMsgProcess()
{
	sysDateAndTime_t currentTime;
	LONG  slot,port;
	int  i,j,t;
	UCHAR version[AUTO_LOAD_FILETYPE_COUNT][32];
	int cfg_status, upg_status;
	
	auto_load_resource_msg_t ipResMsg;
	auto_load_onu_list_t *pOnuList;

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		/*AUTOLOAD_ONU_DEBUG(("auto-load start:slot %d is not master active\r\n", SYS_LOCAL_MODULE_SLOTNO));*/
		return VOS_OK;
	}

	VOS_MemZero( &currentTime, sizeof(sysDateAndTime_t) );
	if (VOS_OK != eventGetCurTime( &currentTime ) )
	{
		return VOS_ERROR;
	}

	/*AUTOLOAD_ONU_DEBUG(("auto-load start enable:cfg=%d,upg=%d\r\n", autoLoad_cfg_enable,autoLoad_upg_enable));*/
	
	if( autoLoad_upg_enable == 1 )
	{
		if( VOS_MemCmp(&autoLoad_startTime, &currentTime, sizeof(sysDateAndTime_t)) > 0 )	/* 未开始 */
		{
			autoLoad_upg_startFlag = 0;
		}
		else if( VOS_MemCmp(&currentTime, &autoLoad_endTime, sizeof(sysDateAndTime_t)) > 0 )	/* 结束 */
		{
			autoLoad_upg_enable=0;
			autoLoad_upg_startFlag=0;
			autoload_upg_onu_status_abort();
			return  VOS_OK;
		}
		else /* 开始 */
		{
			autoLoad_upg_startFlag=1;
		}
	}
	else
	{
		autoLoad_upg_startFlag = 0;
	}
	if( (autoLoad_cfg_enable == 0) && (autoLoad_upg_enable == 0) )
	{
		if( onuAutoLoadTimerId != VOS_ERROR )
		{
			VOS_TimerDelete( MODULE_ONU_AUTO_LOAD, onuAutoLoadTimerId );
			onuAutoLoadTimerId = VOS_ERROR;/*问题单8594*/
		}
	}

	autoLoad_cfg_startFlag = autoLoad_cfg_enable;
	  
	if( (autoLoad_upg_startFlag | autoLoad_cfg_startFlag) == 0 )
	{
	  	return  VOS_OK;
	}

	/*AUTOLOAD_ONU_DEBUG(("auto-load start flag:cfg=%d,upg=%d\r\n", autoLoad_cfg_startFlag,autoLoad_upg_startFlag));*/
	 
	for( i=0; i<MAXPONCHIP; i++ )
	{
		for( j=0; j<MAXONUPERPON; j++ )
		{	
			if( GetOnuOperStatus(i, j) != ONU_OPER_STATUS_UP )
			{
				continue;
			}

			VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
			pOnuList = &auto_load_onu_list[i][j];
			cfg_status = pOnuList->onu_cfg_status;
			upg_status = pOnuList->onu_upg_status;
			VOS_SemGive( onuAutoLoadSemId );

			if( ((cfg_status != AUTO_LOAD_STATE_WAIT) || (autoLoad_cfg_startFlag != 1)) &&
				((upg_status != AUTO_LOAD_STATE_WAIT) || (autoLoad_upg_startFlag != 1)) )
			{
				continue;
			}

			/*应该在这获取空闲 ip*/
			t = 0;
			if( onuAutoLoadGetIp(&t) == VOS_ERROR )
			{
				AUTOLOAD_ONU_DEBUG(("auto-load ip resource waiting\r\n"));
				return VOS_OK;
			}	
			slot = GetCardIdxByPonChip(i);
			port = GetPonPortByPonChip(i);
			if( (slot == RERROR) || (port == RERROR) )
				return VOS_ERROR;

			VOS_MemZero( &ipResMsg, sizeof(ipResMsg) );
			ipResMsg.slot = slot;
			ipResMsg.pon = port;
			ipResMsg.onu = j+1;
			ipResMsg.onuDevIdx = MAKEDEVID(ipResMsg.slot, ipResMsg.pon, ipResMsg.onu);
			ipResMsg.data[0] = t;

			/*自动升级和自动配置同时进行*/
			if( ((cfg_status == AUTO_LOAD_STATE_WAIT) && (autoLoad_cfg_startFlag == 1)) &&
				((upg_status == AUTO_LOAD_STATE_WAIT) && (autoLoad_upg_startFlag == 1)) )
			{
				if(autoload_cheak_onu_file_is_null((auto_load_msg_t *)&ipResMsg)==VOS_OK)
				{
					if( autoload_onu_ver_compare((auto_load_msg_t *)&ipResMsg, version, ipResMsg.file_name) == VOS_ERROR )
					{
						VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
						pOnuList->onu_upg_status = AUTO_LOAD_STATE_FINISHED;
						auto_load_onuctrl[t].status = AUTO_LOAD_STATE_NOT_USING;
						VOS_SemGive( onuAutoLoadSemId );

						/*VOS_SysLog(LOG_TYPE_DEVSM, LOG_WARNING, "onu %d upgrade-file onu version <onu version!", ipResMsg.onuDevIdx);*/
						AUTOLOAD_ONU_DEBUG(("auto-load cfg & upg onu %d file ver < cur ver\r\n", ipResMsg.onuDevIdx));
						continue;
					}
					if( onuAutoLoadIpResourceAssign(&ipResMsg) == VOS_OK )
					{	
						VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
						pOnuList->onu_upg_status = AUTO_LOAD_STATE_PROCESSING;
						pOnuList->onu_cfg_status = AUTO_LOAD_STATE_PROCESSING;
						auto_load_onuctrl[t].onuDevIdx = ipResMsg.onuDevIdx;
						VOS_SemGive( onuAutoLoadSemId );

						AUTOLOAD_ONU_DEBUG(("auto-load cfg & upg:onu %d send oam finished\r\n", ipResMsg.onuDevIdx));
					}
					else
					{
						VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
						auto_load_onuctrl[t].onuDevIdx = ipResMsg.onuDevIdx;
						VOS_SemGive( onuAutoLoadSemId );

						/*OLT 向ONU 发送OAM 失败后，要产生升级失败告警*/
						AUTOLOAD_ONU_DEBUG(("auto-load cfg & upg onu %d:send oam failure\r\n", ipResMsg.onuDevIdx));

						onuLoadFileFailure_EventReport( ipResMsg.onuDevIdx );
						onuAutoLoadUpgradeFailure_EventReport( ipResMsg.onuDevIdx );
						onuAutoLoadIpResourceRelease( (auto_load_msg_t *)&ipResMsg );
						
						onuAutoLoadFreeIp( ipResMsg.onuDevIdx, AUTO_LOAD_TYPE_ALL, AUTO_LOAD_STATE_FAILURE, AUTO_LOAD_STATE_FAILURE );
						/*通知ONU释放IP*/
					}
				}
				else		/*modified by shixh@09-06-05,当配置了onu升级但没配置升级文件时，释放IP*/
				{
					/*管理员有可能未配置upgrade-file表，应该提示*/
					AUTOLOAD_ONU_DEBUG(("auto-load cfg & upg:not find upg file\r\n"));
					auto_load_onuctrl[t].status=AUTO_LOAD_STATE_NOT_USING;
				}	
			}
			/*自动升级*/
			else if( (upg_status == AUTO_LOAD_STATE_WAIT) && (autoLoad_upg_startFlag == 1) )
			{
				if( autoload_cheak_onu_file_is_null((auto_load_msg_t *)&ipResMsg) == VOS_OK )
				{
					if( autoload_onu_ver_compare((auto_load_msg_t *)&ipResMsg, version, ipResMsg.file_name) == VOS_ERROR )
					{	
						VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
						auto_load_onuctrl[t].onuDevIdx = ipResMsg.onuDevIdx;
						/*auto_load_onuctrl[t].status = AUTO_LOAD_STATE_NOT_USING;*/
						VOS_SemGive( onuAutoLoadSemId );

						/*VOS_SysLog(LOG_TYPE_DEVSM, LOG_WARNING, "onu %d upgrade-file onu version <onu version!", ipResMsg.onuDevIdx);*/
						AUTOLOAD_ONU_DEBUG(("auto-load upg:onu %d file ver < cur ver\r\n", ipResMsg.onuDevIdx));

						onuAutoLoadFreeIp( ipResMsg.onuDevIdx, AUTO_LOAD_TYPE_UPG, AUTO_LOAD_STATE_IDLE, AUTO_LOAD_STATE_FINISHED);
						continue;
					}
					if( onuAutoLoadIpResourceAssign(&ipResMsg) == VOS_OK )
					{	
						VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
						pOnuList->onu_upg_status = AUTO_LOAD_STATE_PROCESSING;
						auto_load_onuctrl[t].onuDevIdx = ipResMsg.onuDevIdx;
						VOS_SemGive( onuAutoLoadSemId );
							
						AUTOLOAD_ONU_DEBUG(("auto-load upg:onu %d send oam finished\r\n", ipResMsg.onuDevIdx));
					}
					else
					{
						VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
						auto_load_onuctrl[t].onuDevIdx = ipResMsg.onuDevIdx;
						VOS_SemGive( onuAutoLoadSemId );

						onuAutoLoadUpgradeFailure_EventReport( ipResMsg.onuDevIdx );
						AUTOLOAD_ONU_DEBUG(("auto-load upg:onu %d send oam failure\r\n", ipResMsg.onuDevIdx));
							
						/*通知ONU释放IP*/
						onuAutoLoadIpResourceRelease( (auto_load_msg_t *)&ipResMsg );
						onuAutoLoadFreeIp( ipResMsg.onuDevIdx, AUTO_LOAD_TYPE_UPG, AUTO_LOAD_STATE_IDLE, AUTO_LOAD_STATE_FAILURE );
					}							 
				}
				else		/*modified by shixh@09-06-05,当配置了onu升级但没配置升级文件时，释放IP*/
				{
					AUTOLOAD_ONU_DEBUG(("auto-load upg:not find upg file\r\n"));
					auto_load_onuctrl[t].status=AUTO_LOAD_STATE_NOT_USING;
				}
			}
			/*自动配置*/
			else if( (cfg_status == AUTO_LOAD_STATE_WAIT) && (autoLoad_cfg_startFlag == 1) )
			{
				if( onuAutoLoadIpResourceAssign(&ipResMsg) == VOS_OK )
				{
					VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
					auto_load_onuctrl[t].onuDevIdx = ipResMsg.onuDevIdx;
					pOnuList->onu_cfg_status=AUTO_LOAD_STATE_PROCESSING;
					VOS_SemGive( onuAutoLoadSemId );
					
					AUTOLOAD_ONU_DEBUG(("auto-load cfg: onu %d send oam finished\r\n", ipResMsg.onuDevIdx));
				}
				else
				{
					VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
					auto_load_onuctrl[t].onuDevIdx = ipResMsg.onuDevIdx;
					VOS_SemGive( onuAutoLoadSemId );

					onuLoadFileFailure_EventReport( ipResMsg.onuDevIdx );
					AUTOLOAD_ONU_DEBUG(("auto-load cfg:onu %d send oam failure\r\n", ipResMsg.onuDevIdx));

					/*通知ONU释放IP*/
					onuAutoLoadIpResourceRelease( (auto_load_msg_t *)&ipResMsg );
					onuAutoLoadFreeIp( ipResMsg.onuDevIdx, AUTO_LOAD_TYPE_CFG, AUTO_LOAD_STATE_FAILURE, AUTO_LOAD_STATE_IDLE );
				}
			}
		}
	}
	return  VOS_OK;  
}


/*处理60S定时*/
LONG onuAutoLoadAgingMsgProcess()
{
      	int  i;
	ULONG onuDevIdx;
	SHORT PonPortIdx, OnuIdx;
	auto_load_onu_list_t *pOnuList;

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		/*AUTOLOAD_ONU_DEBUG(("auto-load start:slot %d is not master active\r\n", SYS_LOCAL_MODULE_SLOTNO));*/
		return VOS_OK;
	}

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	
	for( i=0; i<AUTO_LOAD_FTP_CLIENT_IPNUM; i++ )
	{
		if( auto_load_onuctrl[i].onuDevIdx != 0 )
		{
			if(auto_load_onuctrl[i].timecount >= autoLoad_aging_times)
			{
				onuDevIdx = auto_load_onuctrl[i].onuDevIdx;
				AUTOLOAD_ONU_DEBUG(("auto-load onu %d:aging timeout\r\n", onuDevIdx));

				/* modified by xieshl 20110623, 加载超时则上报告警，问题单13062 */
				PonPortIdx = GetPonPortIdxBySlot( GET_PONSLOT(onuDevIdx), GET_PONPORT(onuDevIdx) );
				OnuIdx = GET_ONUID(onuDevIdx) -1;

				if( OLT_LOCAL_ISVALID(PonPortIdx) && ONU_IDX_ISVALID(OnuIdx) )
				{
					VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
					pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
					if( pOnuList->onu_upg_status == AUTO_LOAD_STATE_PROCESSING )
					{
						onuLoadFileFailure_EventReport( onuDevIdx );
					}
					if( pOnuList->onu_cfg_status == AUTO_LOAD_STATE_PROCESSING )
					{
						onuSoftwareLoadFailure_EventReport( onuDevIdx );
					}
					VOS_SemGive( onuAutoLoadSemId );
				}
				else
					VOS_ASSERT(0);

				onuAutoLoadFreeIp( onuDevIdx, AUTO_LOAD_TYPE_NULL, AUTO_LOAD_STATE_FAILURE, AUTO_LOAD_STATE_FAILURE );
			}
			else
			{
	                    auto_load_onuctrl[i].timecount++;
			}
		}
	}
	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}
 
/*ONU 注册处理*/
LONG  onuAutoLoadConfigMsgProcess( auto_load_msg_t *pAutoLoadMsg )
{
	ULONG  PonPortIdx, OnuIdx;
	/*char ulMacAddr[8]={0};
	int  MacAddrLen=0;*/
	auto_load_onu_list_t *pOnuList;

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;

	if( (NULL == pAutoLoadMsg) || (0 == pAutoLoadMsg->onuDevIdx) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	
	PonPortIdx = GetPonPortIdxBySlot( pAutoLoadMsg->slot, pAutoLoadMsg->pon );
	OnuIdx = pAutoLoadMsg->onu - 1;
	CHECK_ONU_RANGE;

	/* 此时在6900上还无法获取onu mac地址，会造成新注册ONU无法自动配置 */
	/*if(GetOnuMacAddr(PonPortIdx, OnuIdx, ulMacAddr, &MacAddrLen) != VOS_OK) return(VOS_ERROR);*/
	
	if( autoLoad_cfg_enable == 1 )
	{
		VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
		
		pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
		pOnuList->onu_cfg_status = AUTO_LOAD_STATE_WAIT;
		pOnuList->onu_cfg_ftp_direct = AUTO_LOAD_DIR_SERVER2ONU;
		/*VOS_MemCpy( pOnuList->onu_cfg_filename, ulMacAddr, MacAddrLen );*/
		VOS_MemZero( pOnuList->onu_cfg_filename, 6 );
		VOS_SemGive( onuAutoLoadSemId );

		/*AUTOLOAD_ONU_DEBUG(("when reg: file name is:%02x%02x.%02x%02x.%02x%02x\r\n", 
					pOnuList->onu_cfg_filename[0], pOnuList->onu_cfg_filename[1],
					pOnuList->onu_cfg_filename[2], pOnuList->onu_cfg_filename[3],
					pOnuList->onu_cfg_filename[4], pOnuList->onu_cfg_filename[5]) );*/
		AUTOLOAD_ONU_DEBUG(("\r\n set onu %d auto-load config status:%s\r\n", pAutoLoadMsg->onuDevIdx, autoload_cfg_onu_status_str(AUTO_LOAD_STATE_WAIT)));
	}
	return  VOS_OK;
}


 /*ONU 离线处理*/
LONG onuAutoLoadDeregisterMsgProcess( auto_load_msg_t *pAutoLoadMsg )
{
	/*ULONG  PonPortIdx, OnuIdx;
	auto_load_onu_list_t *pOnuList;
	ULONG loadType, loadStatus;*/

	/*if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;
	if( (NULL == pAutoLoadMsg) || (0 == pAutoLoadMsg->onuDevIdx) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}*/

	/*PonPortIdx = GetPonPortIdxBySlot( pAutoLoadMsg->slot, pAutoLoadMsg->pon);
	OnuIdx = pAutoLoadMsg->onu -1;
	CHECK_ONU_RANGE;

	loadType = AUTO_LOAD_TYPE_NULL;
	loadStatus = AUTO_LOAD_STATE_FAILURE;
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
	if( pOnuList->onu_cfg_status > AUTO_LOAD_STATE_FINISHED )
	{
		loadType |= AUTO_LOAD_TYPE_CFG;
	}
	if( pOnuList->onu_upg_status > AUTO_LOAD_STATE_FINISHED )
	{
		loadType |= AUTO_LOAD_TYPE_UPG;
	}
	VOS_SemGive( onuAutoLoadSemId );*/

	/*onuAutoLoadFreeIp( pAutoLoadMsg->onuDevIdx, AUTO_LOAD_TYPE_NULL, AUTO_LOAD_STATE_FAILURE, AUTO_LOAD_STATE_FAILURE );

	return  VOS_OK;*/
	pAutoLoadMsg->data[1] = 1;
	return onuAutoLoadCompletedMsgProcess( pAutoLoadMsg );	/* modified by xieshl 20110722, 问题单13267 */
}

LONG onuAutoLoadCompletedMsgProcess( auto_load_msg_t *pAutoLoadMsg )
{
	SHORT  PonPortIdx, OnuIdx;
	auto_load_onu_list_t *pOnuList;
	ULONG loadStatus;

	if( (NULL == pAutoLoadMsg) || (0 == pAutoLoadMsg->onuDevIdx) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;

	/*PonPortIdx = GetPonPortIdxBySlot( pAutoLoadMsg->slot, pAutoLoadMsg->pon );
	OnuIdx = pAutoLoadMsg->onu -1;
	CHECK_ONU_RANGE;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
	if( pOnuList->onu_upg_status == AUTO_LOAD_STATE_PROCESSING )
	{
		if( pAutoLoadMsg->result == onuLoadFile_transfer_complete )
		{
			pOnuList->onu_upg_status = AUTO_LOAD_STATE_FINISHED;
		}
		else if( pAutoLoadMsg->result == onuLoadFile_failure )
		{
			pOnuList->onu_upg_status = AUTO_LOAD_STATE_FAILURE;
		}
	}
	if( pOnuList->onu_cfg_status == AUTO_LOAD_STATE_PROCESSING )
	{
		if( pAutoLoadMsg->result == onuLoadFile_transfer_complete )
		{
			pOnuList->onu_cfg_status = AUTO_LOAD_STATE_FINISHED;
		}
		else if( pAutoLoadMsg->result == onuLoadFile_failure )
		{
			pOnuList->onu_cfg_status = AUTO_LOAD_STATE_FAILURE;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );*/

	if( pAutoLoadMsg->data[1] )
	{
		if( pAutoLoadMsg->result == onuLoadFile_transfer_complete )
		{
			loadStatus = AUTO_LOAD_STATE_FINISHED;
		}
		else if( pAutoLoadMsg->result == onuLoadFile_failure )
		{
			loadStatus = AUTO_LOAD_STATE_FAILURE;
		}
		else if(  pAutoLoadMsg->result == onuLoadFile_timeout )	/* modified by xieshl 20110623, 问题单13062 */
		{
			PonPortIdx = GetPonPortIdxBySlot( pAutoLoadMsg->slot, pAutoLoadMsg->pon );
			OnuIdx = pAutoLoadMsg->onu -1;

			if( OLT_LOCAL_ISVALID(PonPortIdx) && ONU_IDX_ISVALID(OnuIdx) )
			{
				VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
				pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
				if( pOnuList->onu_upg_status == AUTO_LOAD_STATE_PROCESSING )
				{
					onuLoadFileFailure_EventReport( pAutoLoadMsg->onuDevIdx );
				}
				if( pOnuList->onu_cfg_status == AUTO_LOAD_STATE_PROCESSING )
				{
					onuSoftwareLoadFailure_EventReport( pAutoLoadMsg->onuDevIdx );
				}
				VOS_SemGive( onuAutoLoadSemId );
			}
			else
				VOS_ASSERT(0);

			loadStatus = AUTO_LOAD_STATE_FAILURE;
		}

		onuAutoLoadFreeIp( pAutoLoadMsg->onuDevIdx, AUTO_LOAD_TYPE_NULL, loadStatus, loadStatus );/*释放IP*/
	}	
	else
	{
		sys_console_printf("\r\n onu not g_free ip, please wait time out! \r\n");
	}
	return VOS_OK;
}

/*add by shixh@20090414*/
/*获取空闲IP*/
LONG onuAutoLoadGetIp(int *resId)
{
	int k;
	LONG ret = VOS_ERROR;
	if( resId == NULL )
		return ret;
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	for( k=0; k<AUTO_LOAD_FTP_CLIENT_IPNUM; k++ )
	{
		if( auto_load_onuctrl[k].rowStatus != AUTO_LOAD_STATE_USING )
		{
			/*sys_console_printf("\r\nplease config client-ip,there is no ip!\r\n");*/
		}
		else
		{
			if(auto_load_onuctrl[k].status == AUTO_LOAD_STATE_NOT_USING)
			{
				auto_load_onuctrl[k].status = AUTO_LOAD_STATE_USING;/*test add byshixh20090414*/
				*resId = k;
				ret = VOS_OK;
				break;
			}
		}
	}
	VOS_SemGive( onuAutoLoadSemId );

	return ret;
}
/*释放加载队列的IP*/
LONG onuAutoLoadFreeIp( ULONG onuDevIdx, ULONG loadType, ULONG cfgStatus, ULONG upgStatus )
{
	SHORT slot, port;
	SHORT  PonPortIdx, OnuIdx;
	auto_load_onu_list_t *pOnuList;
	int  i;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for(i=0;i<AUTO_LOAD_FTP_CLIENT_IPNUM;i++)
	{
		if(auto_load_onuctrl[i].onuDevIdx == onuDevIdx)
		{
			AUTOLOAD_ONU_DEBUG(("auto-load g_free ip:onu %d, %s\r\n", onuDevIdx, auto_load_onuctrl[i].ftpClient.ipMask));
			auto_load_onuctrl[i].onuDevIdx=0;
			auto_load_onuctrl[i].status=AUTO_LOAD_STATE_NOT_USING;
			auto_load_onuctrl[i].timecount=0;

			slot  = GET_PONSLOT(onuDevIdx);
			port = GET_PONPORT(onuDevIdx);
			PonPortIdx = GetPonPortIdxBySlot(slot, port);
			OnuIdx = GET_ONUID(onuDevIdx)-1;
			if( !OLT_LOCAL_ISVALID(PonPortIdx) || !ONU_IDX_ISVALID(OnuIdx) )
			{
				VOS_ASSERT(0);
				continue;
			}

			pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
			
			if( (loadType & AUTO_LOAD_TYPE_ALL) == AUTO_LOAD_TYPE_NULL )
			{
				if( pOnuList->onu_cfg_status > AUTO_LOAD_STATE_FINISHED )
				{
					pOnuList->onu_cfg_status = cfgStatus;/*AUTO_LOAD_STATE_FAILURE;*/	/* modified by xieshl 20110622, 问题单13065 */
				}
				if( pOnuList->onu_upg_status > AUTO_LOAD_STATE_FINISHED )
				{
					pOnuList->onu_upg_status = upgStatus;/*AUTO_LOAD_STATE_FAILURE;*/	/* modified by xieshl 20110706, 问题单13209 */
				}
			}
			else
			{
				if( (loadType & AUTO_LOAD_TYPE_CFG) == AUTO_LOAD_TYPE_CFG )
				{
					/*if( pOnuList->onu_cfg_status == AUTO_LOAD_STATE_PROCESSING )*/
					{
						pOnuList->onu_cfg_status = cfgStatus;
					}
				}
				if( (loadType & AUTO_LOAD_TYPE_UPG) == AUTO_LOAD_TYPE_UPG )
				{
					/*if( pOnuList->onu_upg_status == AUTO_LOAD_STATE_PROCESSING )*/
					{
						pOnuList->onu_upg_status = upgStatus;
					}
				}
			}
		}
	}
	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}


LONG onuAutoLoadIpResourceAssign( auto_load_resource_msg_t *pIpResMsg )
{
	LONG ret = VOS_OK;
	if( NULL == pIpResMsg )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	if( (ret = onuAutoLoadIpResourceAssignOamPduCreate(pIpResMsg)) == VOS_OK )
	{
#if 0        
		if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER )
		{
			ret = onuAutoLoadIpResourceAssignOamSend( pIpResMsg );
		}
		else
		{
			ret = onuAutoLoadIpResourceAssignOam2Slave( pIpResMsg );
		}
#else
        if(SYS_LOCAL_MODULE_ISMASTERACTIVE && SYS_MODULE_SLOT_ISHAVECPU(pIpResMsg->slot) && (SYS_LOCAL_MODULE_SLOTNO != pIpResMsg->slot))
		{
			ret = onuAutoLoadIpResourceAssignOam2Slave( pIpResMsg );
		}
        else
		{
			ret = onuAutoLoadIpResourceAssignOamSend( pIpResMsg );
		}            
#endif
	}
	return ret;
}
LONG  onuAutoLoadIpResourceRelease( auto_load_msg_t *pAutoLoadMsg )
{
	LONG ret = VOS_OK;
#if 0    
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER )
	{
		ret = onuAutoLoadIpResourceReleaseOamSend( pAutoLoadMsg );
	}
	else
	{
		ret = onuAutoLoadIpResourceReleaseOam2Slave( pAutoLoadMsg );
	}
#else
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE && SYS_MODULE_SLOT_ISHAVECPU(pAutoLoadMsg->slot) && (SYS_LOCAL_MODULE_SLOTNO != pAutoLoadMsg->slot))
	{
		ret = onuAutoLoadIpResourceReleaseOam2Slave( pAutoLoadMsg );
	}
    else
	{
		ret = onuAutoLoadIpResourceReleaseOamSend( pAutoLoadMsg );
	}
#endif
	return ret;
}

LONG onuAutoLoadConfigMsg2Master( auto_load_msg_t *pAutoLoadMsg )
{
	int rc;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(auto_load_msg_t);
	SYS_MSG_S   *pstMsg;

	if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return VOS_OK;

	if( (NULL == pAutoLoadMsg) || (0 == pAutoLoadMsg->onuDevIdx) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	pstMsg = (SYS_MSG_S *)CDP_AllocMsg( msgLen, MODULE_ONU_AUTO_LOAD );
	if(pstMsg == NULL)
	{
		VOS_ASSERT(pstMsg);
		return VOS_ERROR;
	}

	pstMsg->ulSrcModuleID = MODULE_ONU_AUTO_LOAD;
	pstMsg->ulDstModuleID = MODULE_ONU_AUTO_LOAD;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_MASTER_ACTIVE_SLOTNO;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->usMsgCode = AUTO_LOAD_MSG_CODE_CDP_ONU_REQUEST;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (pstMsg + 1);
	pstMsg->usFrameLen = sizeof(auto_load_msg_t);

	VOS_MemCpy( pstMsg->ptrMsgBody, pAutoLoadMsg, pstMsg->usFrameLen );
	
	rc = CDP_Send( RPU_TID_CDP_AUTO_LOAD, SYS_MASTER_ACTIVE_SLOTNO,  RPU_TID_CDP_AUTO_LOAD, CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_ONU_AUTO_LOAD );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pstMsg );
	}	
	return rc;
}

LONG onuAutoLoadCompletedMsg2Master( auto_load_msg_t *pAutoLoadMsg )
{
	int rc;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(auto_load_msg_t);
	SYS_MSG_S   *pstMsg;

	if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return VOS_OK;

	if( (NULL == pAutoLoadMsg) || (0 == pAutoLoadMsg->onuDevIdx) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	pstMsg = (SYS_MSG_S *)CDP_AllocMsg( msgLen, MODULE_ONU_AUTO_LOAD );
	if(pstMsg == NULL)
	{
		VOS_ASSERT(pstMsg);
		return VOS_ERROR;
	}

	pstMsg->ulSrcModuleID = MODULE_ONU_AUTO_LOAD;
	pstMsg->ulDstModuleID = MODULE_ONU_AUTO_LOAD;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_MASTER_ACTIVE_SLOTNO;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->usMsgCode = AUTO_LOAD_MSG_CODE_CDP_ONU_COMPLETED;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (pstMsg + 1);
	pstMsg->usFrameLen = sizeof(auto_load_msg_t);

	VOS_MemCpy( pstMsg->ptrMsgBody, pAutoLoadMsg, pstMsg->usFrameLen );
         
	rc = CDP_Send( RPU_TID_CDP_AUTO_LOAD, SYS_MASTER_ACTIVE_SLOTNO,  RPU_TID_CDP_AUTO_LOAD, CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_ONU_AUTO_LOAD );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pstMsg );
	}	
	return rc;
}

LONG onuAutoLoadIpResourceAssignOam2Slave( auto_load_resource_msg_t *pIpResMsg )
{
	int rc;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(auto_load_resource_msg_t);
	SYS_MSG_S   *pstMsg;

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;

	if( (NULL == pIpResMsg) || (0 == pIpResMsg->onuDevIdx) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	pstMsg = (SYS_MSG_S *)CDP_AllocMsg( msgLen, MODULE_ONU_AUTO_LOAD );
	if(pstMsg == NULL)
	{
		VOS_ASSERT(pstMsg);
		return VOS_ERROR;
	}

	pstMsg->ulSrcModuleID = MODULE_ONU_AUTO_LOAD;
	pstMsg->ulDstModuleID = MODULE_ONU_AUTO_LOAD;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = pIpResMsg->slot;/*目的slot*/
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->usMsgCode = AUTO_LOAD_MSG_CODE_CDP_IP_RESOURCE;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (pstMsg + 1);
	pstMsg->usFrameLen = sizeof(auto_load_resource_msg_t);

	VOS_MemCpy((CHAR *)pstMsg->ptrMsgBody, pIpResMsg, sizeof(auto_load_resource_msg_t) );        
         
	rc = CDP_Send( RPU_TID_CDP_AUTO_LOAD, pIpResMsg->slot,  RPU_TID_CDP_AUTO_LOAD, CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_ONU_AUTO_LOAD );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pstMsg );
	}	
	return rc;
}

LONG  onuAutoLoadIpResourceReleaseOam2Slave( auto_load_msg_t *pIpResMsg )
{
	int rc;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(auto_load_msg_t);
	SYS_MSG_S   *pstMsg;

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;

	if( (NULL == pIpResMsg) || (0 == pIpResMsg->onuDevIdx) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	pstMsg = (SYS_MSG_S *)CDP_AllocMsg( msgLen, MODULE_ONU_AUTO_LOAD );
	if(pstMsg == NULL)
	{
		VOS_ASSERT(pstMsg);
		return VOS_ERROR;
	}

	pstMsg->ulSrcModuleID = MODULE_ONU_AUTO_LOAD;
	pstMsg->ulDstModuleID = MODULE_ONU_AUTO_LOAD;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = pIpResMsg->slot;/*目的slot*/
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->usMsgCode = AUTO_LOAD_MSG_CODE_CDP_IP_RELEASE;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (pstMsg + 1);
	pstMsg->usFrameLen = sizeof(auto_load_msg_t);

	VOS_MemCpy((CHAR *)pstMsg->ptrMsgBody, pIpResMsg, sizeof(auto_load_resource_msg_t) );        
         
	rc = CDP_Send( RPU_TID_CDP_AUTO_LOAD, pIpResMsg->slot,  RPU_TID_CDP_AUTO_LOAD, CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_ONU_AUTO_LOAD );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pstMsg );
	}	
	return rc;
}

/*向ONU发送IP资源释放OAM*/
LONG  onuAutoLoadIpResourceReleaseOamSend( auto_load_msg_t *pIpResMsg )
{
	int rc = VOS_ERROR;
	int GetSemId;
	short int PonPortIdx, OnuIdx;
	auto_load_resource_oam_pdu_t oamMsg;
	ULONG pduLen;
	char liv_RecvMsgFromOnu[EUQ_MAX_OAM_PDU];
    short int liv_RecvMsgLen = 0;
	if( (NULL == pIpResMsg) || (0 == pIpResMsg->onuDevIdx) )
		return rc;
    VOS_MemSet( liv_RecvMsgFromOnu, 0,  EUQ_MAX_OAM_PDU);

	/*VOS_MemZero(pIpResMsg->pdu, sizeof(pIpResMsg->pdu));*/
	PonPortIdx = GetPonPortIdxBySlot( pIpResMsg->slot, pIpResMsg->pon);
	OnuIdx = pIpResMsg->onu-1;

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( V2R1_ONU_OFFLINE );

	VOS_MemZero( &oamMsg, sizeof(oamMsg) );
	oamMsg.msgType = AUTO_LOAD_OAM_FINISH_REQ;
	oamMsg.result = 0;
	pduLen = 2;
	/*发送OAM帧*/
    /*modified by luh 2012-9-17*/
#if 0
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, AUTO_LOAD_OAM_FINISH_REQ, (UCHAR *)&oamMsg, pduLen);
#else
	GetSemId = Oam_Session_Send(PonPortIdx, OnuIdx, AUTO_LOAD_OAM_FINISH_REQ, OAM_SYNC, 0, 0, NULL, (UCHAR *)&(oamMsg.result), pduLen, liv_RecvMsgFromOnu, &liv_RecvMsgLen);
#endif
	if( GetSemId == RERROR )
	{
		VOS_TaskDelay(100);
#if 0
    	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, AUTO_LOAD_OAM_FINISH_REQ, (UCHAR *)&oamMsg, pduLen);
#else
    	GetSemId = Oam_Session_Send(PonPortIdx, OnuIdx, AUTO_LOAD_OAM_FINISH_REQ, OAM_SYNC, 0, 0, NULL, (UCHAR *)&(oamMsg.result), pduLen, liv_RecvMsgFromOnu, &liv_RecvMsgLen);
#endif
	}
	if( GetSemId == RERROR )
	{
		sys_console_printf("\r\n send message to onu %d failure(g_free ip)\r\n", pIpResMsg->onuDevIdx);
	}
	else 
	{
        if((/*RecvMsgFromOnu[0]*/liv_RecvMsgFromOnu[0] == AUTO_LOAD_OAM_FINISH_RSP) && 
            (/*RecvMsgFromOnu[1]*/liv_RecvMsgFromOnu[1] == 0))
            pIpResMsg->data[1] = 1;
        else
            pIpResMsg->data[1] = 0;
        
		AUTOLOAD_ONU_DEBUG(("\r\nsend message to onu %d success(g_free ip)\r\n", pIpResMsg->onuDevIdx));
		rc=VOS_OK;
	}
	return  rc;
}

/*组成OAM */
LONG onuAutoLoadIpResourceAssignOamPduCreate( auto_load_resource_msg_t *pIpResMsg )
{
	int  rc=VOS_OK;
	int   i;
	short int PonPortIdx, OnuIdx;
	UCHAR filetype[AUTO_LOAD_FILETYPE_COUNT];
	UCHAR cfg_filename[AUTO_LOAD_FTP_FILENAME_LEN];
	ULONG cfg_direct=0;
	/* modified by xieshl 20110127, 问题单11929、11932 */
	auto_load_onu_list_t *pOnuList;
	auto_load_resource_oam_pdu_t *pOamMsg;
	auto_load_file_list_t *pFileList;
	int fileListLen, fileNameLen;

	if( (NULL == pIpResMsg) || (0 == pIpResMsg->onuDevIdx) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
		
	PonPortIdx = GetPonPortIdxBySlot(pIpResMsg->slot, pIpResMsg->pon );
	OnuIdx = pIpResMsg->onu - 1;

	CHECK_ONU_RANGE;

	pOamMsg = (auto_load_resource_oam_pdu_t*)pIpResMsg->pdu;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	pOamMsg->msgType = AUTO_LOAD_OAM_ACTIVE_REQ;
	pOamMsg->result = 0;
	VOS_MemCpy(pOamMsg->ftpServerIp, auto_load_ftpserver.serverIp, AUTO_LOAD_FTP_IP_STR_LEN);

	pOamMsg->ftpPort = VOS_HTONS(auto_load_ftpserver.port);/*add by shixh20090520*/
	pOamMsg->ftpDataPort = VOS_HTONS(auto_load_ftpserver.dataPort);
 
	VOS_MemCpy(pOamMsg->userName, auto_load_ftpserver.userName, AUTO_LOAD_FTP_USER_LEN);
	VOS_MemCpy(pOamMsg->password, auto_load_ftpserver.password, AUTO_LOAD_FTP_PASSWORD_LEN);

	pOamMsg->vid = VOS_HTONS(auto_load_onuctrl[pIpResMsg->data[0]].ftpClient.vid);
	
	VOS_MemCpy(pOamMsg->clientIp, auto_load_onuctrl[pIpResMsg->data[0]].ftpClient.ipMask, AUTO_LOAD_FTP_IPMASK_STR_LEN);
	VOS_MemCpy(pOamMsg->clientGateway, auto_load_onuctrl[pIpResMsg->data[0]].ftpClient.ipGateway, AUTO_LOAD_FTP_IP_STR_LEN);

	VOS_SemGive( onuAutoLoadSemId );

	VOS_MemZero( filetype, sizeof(filetype) );
	if( autoload_onu_filetype_get((auto_load_msg_t *)pIpResMsg, filetype) < 0) 
	{
		AUTOLOAD_ONU_DEBUG(("auto-load onu %d filetype is null\r\n", pIpResMsg->onuDevIdx));
		return VOS_ERROR;
	}

	/* chenfj 2009-4-14
	     此处应判断自动配置是否已启动; 
	     下同*/

	pOamMsg->fileTotalCount = 0;
	fileListLen = 0;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
	if( (pOnuList->onu_cfg_status == AUTO_LOAD_STATE_WAIT) && (autoLoad_cfg_startFlag == V2R1_ENABLE) )
	{	
		if(autoload_cfg_filename_and_direct_get(pIpResMsg->onuDevIdx, cfg_filename, &cfg_direct) != VOS_OK)
		{
			VOS_SemGive( onuAutoLoadSemId );
			return VOS_OK;
		}
		pFileList = (auto_load_file_list_t *)&pOamMsg->fileListBuf[fileListLen];
		pFileList->fileType = VOS_HTONL(AUTO_LOAD_FILETYPE_CFG);
		fileListLen += 4;

		pFileList->loadDir = pOnuList->onu_cfg_ftp_direct;
		fileListLen++;
		
		fileNameLen = VOS_StrLen(cfg_filename);
		if( fileNameLen > AUTO_LOAD_FTP_FILENAME_LEN )
		{
			VOS_ASSERT(0);
			fileNameLen = AUTO_LOAD_FTP_FILENAME_LEN;
		}
		pFileList->fileNameLen = fileNameLen;
		fileListLen++;

		VOS_StrnCpy( pFileList->fileName, cfg_filename, fileNameLen );
		fileListLen += fileNameLen;
		
		pOamMsg->fileTotalCount++;
	}
	
	if( (pOnuList->onu_upg_status == 1) && (autoLoad_upg_startFlag == 1) )	
	{
		for( i=0; i<AUTO_LOAD_FILETYPE_COUNT; i++ )
		{
			if( filetype[i] != 0 )
			{
				pFileList = (auto_load_file_list_t *)&pOamMsg->fileListBuf[fileListLen];
				pFileList->fileType = VOS_HTONL(filetype[i]);
				fileListLen += 4;
				pFileList->loadDir = pOnuList->onu_upg_ftp_direct;
				fileListLen++;
		
				fileNameLen = VOS_StrLen(pIpResMsg->file_name[i]);
				if( fileNameLen > AUTO_LOAD_FTP_FILENAME_LEN )
				{
					VOS_ASSERT(0);
					fileNameLen = AUTO_LOAD_FTP_FILENAME_LEN;
				}
				pFileList->fileNameLen = fileNameLen;
				fileListLen++;

				VOS_StrnCpy( pFileList->fileName, pIpResMsg->file_name[i], fileNameLen );
				fileListLen += VOS_StrLen(pIpResMsg->file_name[i]);
				
				pOamMsg->fileTotalCount++;

				AUTOLOAD_ONU_DEBUG(("auto-load onu %d filename=%s(%d)\r\n", pIpResMsg->onuDevIdx, pIpResMsg->file_name[i], pOamMsg->fileTotalCount));
			}
		}
	}
	VOS_SemGive( onuAutoLoadSemId );

	if( pOamMsg->fileTotalCount == 0 )
	{
		AUTOLOAD_ONU_DEBUG(("auto-load onu %d filelist of pdu is null\r\n", pIpResMsg->onuDevIdx));
		rc = VOS_ERROR;
	}
	else
	{
		pIpResMsg->pduLen = sizeof(auto_load_resource_oam_pdu_t) - AUTO_LOAD_FTP_FILELIST_LEN + fileListLen + 1;
		
		if(autoload_onu_debug_flag==1)
		{
			sys_console_printf(" auto-load onu %d PDU length %d\r\n", pIpResMsg->onuDevIdx, pIpResMsg->pduLen );
			pktDataPrintf( pIpResMsg->pdu, pIpResMsg->pduLen );
		}
		rc = VOS_OK;
	}
	return rc;
}

/*组成OAM ，发往ONU*/
LONG onuAutoLoadIpResourceAssignOamSend( auto_load_resource_msg_t *pIpResMsg )
{
	int  rc=VOS_OK;
	int GetSemId;
	short int PonPortIdx, OnuIdx;
    char liv_RecvMsgFromOnu[EUQ_MAX_OAM_PDU];
    short int liv_RecvMsgLen = 0;
	if( (NULL == pIpResMsg) || (0 == pIpResMsg->onuDevIdx) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
    VOS_MemSet( liv_RecvMsgFromOnu, 0,  EUQ_MAX_OAM_PDU);
		
	PonPortIdx = GetPonPortIdxBySlot(pIpResMsg->slot, pIpResMsg->pon );
	OnuIdx = pIpResMsg->onu - 1;

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		sys_console_printf( "\r\n auto-load onu %d is off line\r\n", pIpResMsg->onuDevIdx );
		return( V2R1_ONU_OFFLINE );
	}

	AUTOLOAD_ONU_DEBUG(("auto-load onu %d send oam start...\r\n", pIpResMsg->onuDevIdx));


	if(autoload_onu_debug_flag==1)
	{
		sys_console_printf( " auto-load onu %d PDU length %d, addr is 0x%x\r\n", pIpResMsg->onuDevIdx, pIpResMsg->pduLen, (int)pIpResMsg->pdu );
		pktDataPrintf( pIpResMsg->pdu, pIpResMsg->pduLen );
	}
    /*modified by luh 2012-9-17*/    
#if 0    
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, AUTO_LOAD_OAM_ACTIVE_REQ, pIpResMsg->pdu, pIpResMsg->pduLen);
#else
	GetSemId = Oam_Session_Send(PonPortIdx, OnuIdx, AUTO_LOAD_OAM_ACTIVE_REQ, OAM_SYNC, 0, 0, NULL, pIpResMsg->pdu, pIpResMsg->pduLen, liv_RecvMsgFromOnu, &liv_RecvMsgLen);
#endif
	if( GetSemId == RERROR )
	{
		/* modified by xieshl 20110623, 发送OAM失败再重发一次，如果仍然失败则上报告警，问题单13062 */
#if 0    
    	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, AUTO_LOAD_OAM_ACTIVE_REQ, pIpResMsg->pdu, pIpResMsg->pduLen);
#else
    	GetSemId = Oam_Session_Send(PonPortIdx, OnuIdx, AUTO_LOAD_OAM_ACTIVE_REQ, OAM_SYNC, 0, 0, NULL, pIpResMsg->pdu, pIpResMsg->pduLen, liv_RecvMsgFromOnu, &liv_RecvMsgLen);
#endif
		
		if( GetSemId == RERROR )
		{
			sys_console_printf( " auto-load get applyed ip to onu,but no RSP from onu %d\r\n", pIpResMsg->onuDevIdx );
			onuAutoLoadMsgSend( AUTO_LOAD_MSG_CODE_ONU_COMPLETED, pIpResMsg->onuDevIdx, onuLoadFile_timeout );
		}
		rc=VOS_ERROR;
	}
	else 
	{	
		/*add by shixh20090608*/	
		if((/*RecvMsgFromOnu[0]*/liv_RecvMsgFromOnu[0] == AUTO_LOAD_OAM_ACTIVE_RSP) && 
            (/*RecvMsgFromOnu[1]*/liv_RecvMsgFromOnu[1] == 1))
		{
			AUTOLOAD_ONU_DEBUG(("auto-load onu %d create vlan failure!so olt g_free ip!\r\n", pIpResMsg->onuDevIdx));
			rc=VOS_ERROR;
		}
		else if((/*RecvMsgFromOnu[0]*/liv_RecvMsgFromOnu[0] == AUTO_LOAD_OAM_ACTIVE_RSP) && 
            (/*RecvMsgFromOnu[1]*/liv_RecvMsgFromOnu[1] == 0))
		{
			AUTOLOAD_ONU_DEBUG(("auto-load send message to onu %d success\r\n", pIpResMsg->onuDevIdx));
			rc=VOS_OK;
		}
		/*end shixh20090608*/
	}
	return rc;
}


/*自动升级启动流程*/
LONG  onuAutoLoadUpgradeStart(struct vty *vty)
{
	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return VOS_OK;

	if(autoLoad_upg_startFlag==1)
	{
	 	v2r1_printf( vty, "auto-load upgrade starting!\r\n");
		v2r1_printf( vty, " start time: %d-%d-%d,%d:%d:%d\r\n", 
				autoLoad_startTime.year, autoLoad_startTime.month, autoLoad_startTime.day,
				autoLoad_startTime.hour, autoLoad_startTime.minute, autoLoad_startTime.second );
		v2r1_printf( vty, " end time: %d-%d-%d,%d:%d:%d\r\n",
				autoLoad_endTime.year, autoLoad_endTime.month, autoLoad_endTime.day,
				autoLoad_endTime.hour, autoLoad_endTime.minute, autoLoad_endTime.second );
		return VOS_OK;
	}
	
	/*检查ONU_LIST表，清除未升级的ONU 的状态*/
	autoload_upg_onu_status_restart();

	autoLoad_upg_enable=1;

	if( onuAutoLoadTimerId == VOS_ERROR )
	{
		onuAutoLoadTimerId = VOS_TimerCreate(MODULE_ONU_AUTO_LOAD, 0, autoLoad_timer_interval, (VOID *)onuAutoLoadStartupTimerCallback, NULL, VOS_TIMER_LOOP);
		if( onuAutoLoadTimerId == VOS_ERROR )
		{
			VOS_ASSERT( 0 );
			return  VOS_ERROR;
		}
	}
	return  VOS_OK;
}

 /*自动配置启动流程*/
LONG onuAutoLoadConfigStart(struct vty *vty)
{
	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return VOS_OK;

	if(autoLoad_cfg_startFlag==1)
	{
		vty_out(vty, "auto-load config already start!\r\n");
		return VOS_OK;
	}
	
	/*检查ONU_LIST表，清楚未配置的ONU的状态*/
	autoload_cfg_onu_status_reset();

	autoLoad_cfg_enable=1;
	autoLoad_cfg_startFlag=1;
	if( onuAutoLoadTimerId == VOS_ERROR )
	{
		onuAutoLoadTimerId = VOS_TimerCreate(MODULE_ONU_AUTO_LOAD, 0, autoLoad_timer_interval, (VOID *)onuAutoLoadStartupTimerCallback, NULL, VOS_TIMER_LOOP);
		if( onuAutoLoadTimerId == VOS_ERROR )
		{
			VOS_ASSERT( 0 );
			return  VOS_ERROR;
		}
	}
	return  VOS_OK;
}
 

/*检查ONU _file表中是否配置了该类型的ONU */
/*返回值:VOS_OK,是
			VOS_ERROR,否*/
LONG autoload_cheak_onu_file_is_null( auto_load_msg_t *pAutoLoadMsg )
{
	int  i;
	short int PonPortIdx, OnuIdx;
	int  onu_type;
	ULONG  rc=VOS_ERROR;
	
	PonPortIdx = GetPonPortIdxBySlot(pAutoLoadMsg->slot, pAutoLoadMsg->pon);
	OnuIdx = pAutoLoadMsg->onu - 1;
	/* modified by chenfj 2009-4-13
	    增加容错判断
	    */
	CHECK_ONU_RANGE;
	if( GetOnuType(PonPortIdx,OnuIdx,&onu_type) != VOS_OK ) return(rc);
	if( onu_type >= V2R1_ONU_MAX ) return(rc);
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for( i=0; i<AUTO_LOAD_FILETYPE_COUNT; i++ )
	{
		if( auto_load_filetype_list[onu_type].file_type[i].filetype != 0 )
		{
			rc = VOS_OK;
			break;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	return  rc;
}

LONG autoload_onu_filetype_get( auto_load_msg_t *pAutoLoadMsg, UCHAR file_type[AUTO_LOAD_FILETYPE_COUNT] )
{
	int  i;
	short int PonPortIdx, OnuIdx;
	int  onu_type;
	LONG   count = 0;
	
	PonPortIdx = GetPonPortIdxBySlot(pAutoLoadMsg->slot, pAutoLoadMsg->pon);
	OnuIdx = pAutoLoadMsg->onu - 1;

	CHECK_ONU_RANGE;
		
	if( GetOnuType(PonPortIdx,OnuIdx,&onu_type) != VOS_OK )
		return count;
	if( onu_type >= V2R1_ONU_MAX )
		return count;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for( i=0; i<AUTO_LOAD_FILETYPE_COUNT; i++ )
	{
		if( auto_load_filetype_list[onu_type].file_type[i].filetype != 0 )
		{
			file_type[i] = auto_load_filetype_list[onu_type].file_type[i].filetype;
			count++;
		}	
	}
	VOS_SemGive( onuAutoLoadSemId );
	return  count;
}

ULONG autoload_filetype_to_index( int type )
{
	ULONG fileTypeIdx = 0;
	switch( type )
	{
		case AUTO_LOAD_FILETYPE_CFG:
			break;
		case AUTO_LOAD_FILETYPE_BOOT:
			fileTypeIdx = 1;
			break;
		case AUTO_LOAD_FILETYPE_APP:
			fileTypeIdx = 2;
			break;
		case AUTO_LOAD_FILETYPE_FW:
			fileTypeIdx = 3;
			break;
		case AUTO_LOAD_FILETYPE_VOIP:
			fileTypeIdx = 4;
			break;
		case AUTO_LOAD_FILETYPE_FPGA:
			fileTypeIdx = 5;
			break;
		default:
			break;
	}
	return fileTypeIdx;
}
ULONG autoload_filetypeIdx_to_type( int index )
{
	ULONG fileType = AUTO_LOAD_FILETYPE_NULL;
	switch( index )
	{
		case 1:
			fileType = AUTO_LOAD_FILETYPE_BOOT;
			break;
		case 2:
			fileType = AUTO_LOAD_FILETYPE_APP;
			break;
		case 3:
			fileType = AUTO_LOAD_FILETYPE_FW;
			break;
		case 4:
			fileType = AUTO_LOAD_FILETYPE_VOIP;
			break;
		case 5:
			fileType = AUTO_LOAD_FILETYPE_FPGA;
			break;
		default:
			break;
	}
	return fileType;
}

char * autoload_filetypeIdx_to_str( int index )
{
	char *pFileTypeStr = NULL;
	switch( index )
	{
		case 1:
			pFileTypeStr = "boot";
			break;
		case 2:
			pFileTypeStr = "app";
			break;
		case 3:
			pFileTypeStr = "fw";
			break;
		case 4:
			pFileTypeStr = "voip";
			break;
		case 5:
			pFileTypeStr = "fpga";
			break;
		default:
			break;
	}
	return pFileTypeStr;
}

char * autoload_filetype_to_str( int type )
{
	return autoload_filetypeIdx_to_str( autoload_filetype_to_index(type) );
}


/*生成文件名*/
LONG  autoload_filetype_to_filename( UCHAR  *onu_type, UCHAR  file_type, UCHAR *ver, UCHAR *fileName )
{
	char *pFileType;
	if( (NULL == ver) || (NULL == fileName) )
		return VOS_ERROR;
	pFileType = autoload_filetype_to_str( file_type );
	if( NULL == pFileType )
		return VOS_ERROR;
	VOS_Sprintf( fileName, "%s_%s_%s.bin", onu_type, pFileType, ver );
	
	return  VOS_OK;
}

/*检查ONU 的版本是否大于原版本*/
LONG  autoload_onu_ver_compare( auto_load_msg_t *pAutoLoadMsg, UCHAR version[AUTO_LOAD_FILETYPE_COUNT][32], UCHAR file_Name[AUTO_LOAD_FILETYPE_COUNT][64] )
{
	short int PonPortIdx, OnuIdx;
	int  onu_type=0;
	int  i;
	LONG rc = VOS_ERROR;
	int  VersionLength=0;
	UCHAR filename[64]={0};
	char onuTypeString[ONU_TYPE_LEN+2] = {0};	
	
	PonPortIdx = GetPonPortIdxBySlot(pAutoLoadMsg->slot, pAutoLoadMsg->pon);
	OnuIdx = pAutoLoadMsg->onu - 1;

	CHECK_ONU_RANGE;

	if(ThisIsValidOnu(PonPortIdx, OnuIdx) !=ROK)
		return(RERROR);

	if(GetOnuType(PonPortIdx,OnuIdx,&onu_type) != ROK) 
		return(RERROR);
	if(GetOnuTypeString(PonPortIdx, OnuIdx,onuTypeString, &VersionLength) != ROK) 
		return(RERROR);

	for( i=0; i<AUTO_LOAD_FILETYPE_COUNT; i++ )
	{
		switch( auto_load_filetype_list[onu_type].file_type[i].filetype )
		{
			case AUTO_LOAD_FILETYPE_BOOT:
				if(GetOnuDeviceBootVersion(pAutoLoadMsg->slot, pAutoLoadMsg->pon, pAutoLoadMsg->onu, version[0], &VersionLength) == ROK)
				{
					AUTOLOAD_ONU_DEBUG(("current boot ver is:%s\r\n",version[0]));

					if( VOS_StrCmp(auto_load_filetype_list[onu_type].file_type[0].ver, version[0]) > 0 )
					{
						autoload_filetype_to_filename(onuTypeString, auto_load_filetype_list[onu_type].file_type[0].filetype, auto_load_filetype_list[onu_type].file_type[0].ver, filename);
						VOS_StrCpy(file_Name[0], filename);
						rc = VOS_OK;
					}
				}
				break;
			case AUTO_LOAD_FILETYPE_APP:
				if(GetOnuDeviceAppVersion(pAutoLoadMsg->slot, pAutoLoadMsg->pon, pAutoLoadMsg->onu, version[1],&VersionLength) == ROK)
				{
					AUTOLOAD_ONU_DEBUG(("current app ver is:%s\r\n",version[1]));

					if( VOS_StrCmp(auto_load_filetype_list[onu_type].file_type[1].ver, version[1]) > 0 )
					{
						/*自动生成文件名*/
						autoload_filetype_to_filename(onuTypeString, auto_load_filetype_list[onu_type].file_type[1].filetype, auto_load_filetype_list[onu_type].file_type[1].ver, filename);
						VOS_StrCpy(file_Name[1], filename);
						rc = VOS_OK;
					}
				}
				break;
			case AUTO_LOAD_FILETYPE_FW:
				if( GetOnuDeviceFrimwareVersion(pAutoLoadMsg->slot, pAutoLoadMsg->pon, pAutoLoadMsg->onu, version[2],&VersionLength) == ROK )
				{
					AUTOLOAD_ONU_DEBUG(("current FW ver is:%s\r\n",version[2]));
					if( VOS_StrCmp(auto_load_filetype_list[onu_type].file_type[2].ver, version[2]) > 0 )
					{
						/*自动生成文件名*/
						autoload_filetype_to_filename(onuTypeString, auto_load_filetype_list[onu_type].file_type[2].filetype, auto_load_filetype_list[onu_type].file_type[2].ver, filename);
						VOS_StrCpy(file_Name[2], filename);
						rc = VOS_OK;
					}
				}
				break;
			case AUTO_LOAD_FILETYPE_VOIP:
			 	if(GetOnuDeviceVoiceVersion(pAutoLoadMsg->slot, pAutoLoadMsg->pon, pAutoLoadMsg->onu, version[3],&VersionLength) == ROK)
			 	{
					AUTOLOAD_ONU_DEBUG(("current VOIP ver is:%s\r\n",version[3]));
					if( VOS_StrCmp(auto_load_filetype_list[onu_type].file_type[3].ver, version[3]) > 0 )
					{
						/*自动生成文件名*/
						autoload_filetype_to_filename(onuTypeString,auto_load_filetype_list[onu_type].file_type[3].filetype,auto_load_filetype_list[onu_type].file_type[3].ver,filename);
						VOS_StrCpy(file_Name[3], filename);
						rc = VOS_OK;
					}
				}
				break;
			case AUTO_LOAD_FILETYPE_FPGA:
				if( GetOnuDeviceFpgaVersion(pAutoLoadMsg->slot, pAutoLoadMsg->pon, pAutoLoadMsg->onu, version[4],&VersionLength) == ROK )
				{
					AUTOLOAD_ONU_DEBUG(("current FPGA ver is:%s\r\n",version[4]));
					if( VOS_StrCmp(auto_load_filetype_list[onu_type].file_type[4].ver, version[4]) > 0 )
					{
						/*自动生成文件名*/
						autoload_filetype_to_filename(onuTypeString,auto_load_filetype_list[onu_type].file_type[4].filetype,auto_load_filetype_list[onu_type].file_type[4].ver,filename);
						VOS_StrCpy(file_Name[4], filename);
						rc = VOS_OK;
					}
				}
				break;
			default:
				break;
		}
		if( rc == VOS_ERROR )
		{
			char *pFileTypeStr = autoload_filetype_to_str(auto_load_filetype_list[onu_type].file_type[i].filetype);
			if( pFileTypeStr )
				sys_console_printf("onu %d/%d/%d %s file version is newer\r\n", pAutoLoadMsg->slot, pAutoLoadMsg->pon, pAutoLoadMsg->onu, pFileTypeStr );
		}
	}
	
	return  rc;
}

/*取配置文件的文件名和方向*/
LONG  autoload_cfg_filename_and_direct_get( ULONG onuDevIdx, UCHAR *filename, ULONG *direct )
{
	SHORT  slot, port;
	SHORT  PonPortIdx, OnuIdx;
	UCHAR onu_mac[6];
	int mac_len;
	auto_load_onu_list_t *pOnuList;
	LONG ret = VOS_ERROR;

	if( (NULL == filename) || (NULL == direct) )
	{
		VOS_ASSERT(0);
		return ret;
	}
	slot  = GET_PONSLOT(onuDevIdx);
	port = GET_PONPORT(onuDevIdx);
	PonPortIdx = GetPonPortIdxBySlot(slot, port);
	OnuIdx = GET_ONUID(onuDevIdx)-1;
	CHECK_ONU_RANGE;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
	if( pOnuList->onu_cfg_filename[0] != 0 )
	{
		VOS_StrCpy( filename, pOnuList->onu_cfg_filename );
		*direct = pOnuList->onu_cfg_ftp_direct;
		ret = VOS_OK;
	}
	else
	{
		if( GetOnuMacAddr(PonPortIdx, OnuIdx, onu_mac, &mac_len) == VOS_OK)
		{
			VOS_StrCpy(filename, auto_load_mac_to_str(onu_mac) );
			*direct = pOnuList->onu_cfg_ftp_direct;
			ret = VOS_OK;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	
	return ret;
}

/*往onu 自动配置等待链表中加入新的onu*/

/* 功能:   设置IP地址、网关、VLAN ID
   输入参数:OnudevIdx－设备索引
   输出参数:	无
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
LONG autoload_ftp_client_ip_set( struct vty *vty, UCHAR *ipMask, UCHAR *ipGateway, USHORT vid )
{
	int i, j = AUTO_LOAD_FTP_CLIENT_IPNUM;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	for(i=0;i<AUTO_LOAD_FTP_CLIENT_IPNUM;i++)
	{
		if( auto_load_onuctrl[i].rowStatus == AUTO_LOAD_STATE_NOT_USING )
		{
			if( j == AUTO_LOAD_FTP_CLIENT_IPNUM )
				j = i;
			continue;
		}
		if( VOS_StrCmp(auto_load_onuctrl[i].ftpClient.ipMask, ipMask) == 0 )
		{
			VOS_SemGive( onuAutoLoadSemId );
			vty_out( vty, "ip %s is alreadly exist\r\n", ipMask);
			return VOS_OK;
		}
	}
			
	if( j < AUTO_LOAD_FTP_CLIENT_IPNUM )
	{
		VOS_StrCpy( auto_load_onuctrl[j].ftpClient.ipMask, ipMask );
		VOS_StrCpy( auto_load_onuctrl[j].ftpClient.ipGateway, ipGateway );
		auto_load_onuctrl[j].ftpClient.vid = vid;
		auto_load_onuctrl[j].rowStatus = AUTO_LOAD_STATE_USING;/*每配置一个IP，就将这个IP的可用状态设为可用*/
		auto_load_onuctrl[j].status = AUTO_LOAD_STATE_NOT_USING;/*未被使用*/
	}
	else
	{
		VOS_SemGive( onuAutoLoadSemId );
		vty_out(vty, " you are already configed 5 client ip!\r\n");
		return VOS_ERROR;
	}
	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}
/* 功能: 获取IP 地址和长度、网关地址和长度、VID和长度
   输入参数:OnudevIdx－设备索引
   输出参数:	iplen- IP地址长度
                            ip-  IP地址
                            gatewaylen- 网关长度
                            ipgateway-  网关地址
                            vidlen- VLAN长度
                            vid- VLAN  ID
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
LONG  autoload_ftp_client_ip_get( ULONG onuDevIdx, UCHAR *ipMask, UCHAR *ipGateway, USHORT *vid )
{
	int i;

	if( (0 == onuDevIdx) && (NULL == ipMask) && (NULL == ipGateway) && (NULL == vid) )
		return VOS_ERROR;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	for( i=0; i<AUTO_LOAD_FTP_CLIENT_IPNUM; i++ )
	{
		if(auto_load_onuctrl[i].onuDevIdx == onuDevIdx)
		{
			if( ipMask )
				VOS_StrCpy( ipMask, auto_load_onuctrl[i].ftpClient.ipMask );
			if( ipGateway )
				VOS_StrCpy( ipGateway, auto_load_onuctrl[i].ftpClient.ipGateway );
			if( vid )
				*vid = auto_load_onuctrl[i].ftpClient.vid;
			break;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );

	if( i >= AUTO_LOAD_FTP_CLIENT_IPNUM )
		return VOS_ERROR;
	return VOS_OK;
}

#if 0
/* 功能:  获取那个onu正在使用某个IP
   输入参数:无
   输出参数:	无
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
LONG get_ip_gateway_vid_onu()
{
	int i;
	/*UCHAR *ipstatus[]={"not using","using"};*/
	
	for(i=0;i<AUTO_LOAD_FTP_CLIENT_IPNUM;i++)
	{
		if((auto_load_onuctrl[i].rowStatus==AUTO_LOAD_STATE_USING)&&(auto_load_onuctrl[i].status ==AUTO_LOAD_STATE_USING))
		{
			if(autoload_onu_debug_flag==1)
			{
				sys_console_printf( "\r\nip address is=%s\r\n",auto_load_onuctrl[i].ftpClient.ipMask);
				sys_console_printf( "onu=%d\r\n", auto_load_onuctrl[i].onuDevIdx);
			}
		}
	}	
	sys_console_printf( "all onu show!!\r\n");
	return VOS_OK;
}

/*获取当前已配IP的个数*/
LONG get_ipnum(int  *ipnum)
{
	int i,t=0;
	for( i=0; i<AUTO_LOAD_FTP_CLIENT_IPNUM; i++ )
	{
		if( auto_load_onuctrl[i].rowStatus == AUTO_LOAD_STATE_USING )
			t++;
	}
	*ipnum=t;
	return  VOS_OK;
}

 /*获取当前正在使用的IP的个数*/
LONG get_used_ipnum(int *usednum)
{
	int i,t=0;
	for(i=0;i<AUTO_LOAD_FTP_CLIENT_IPNUM;i++)
	{
		if(auto_load_onuctrl[i].rowStatus==AUTO_LOAD_STATE_USING&&auto_load_onuctrl[i].status==AUTO_LOAD_STATE_USING)
			t++;
	}
	*usednum=t;
	return  VOS_OK;
}

#endif

/* 功能:  删除IP地址
   输入参数:ip- IP地址
   输出参数:	无
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
LONG autoload_ftp_client_ip_del( UCHAR *ipMask )
{
	int i;
	LONG rc = VOS_ERROR;
	if( NULL == ipMask )
		return rc;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	for(i=0;i<AUTO_LOAD_FTP_CLIENT_IPNUM;i++)
	{
		if((VOS_StrCmp(auto_load_onuctrl[i].ftpClient.ipMask, ipMask) == 0) && (auto_load_onuctrl[i].rowStatus == AUTO_LOAD_STATE_USING))
		{
			if(auto_load_onuctrl[i].status == AUTO_LOAD_STATE_NOT_USING)
			{
				auto_load_onuctrl[i].rowStatus = AUTO_LOAD_STATE_NOT_USING;
				VOS_MemZero( auto_load_onuctrl[i].ftpClient.ipMask, sizeof(auto_load_onuctrl[i].ftpClient.ipMask));
				VOS_MemZero( auto_load_onuctrl[i].ftpClient.ipGateway, sizeof(auto_load_onuctrl[i].ftpClient.ipGateway));
				auto_load_onuctrl[i].ftpClient.vid=1;
			    	rc = VOS_OK;
				break;
			}
		}
	}
 	VOS_SemGive( onuAutoLoadSemId );

   	return rc;
}

/* 功能: 设置TP Server 的用户名、密码、IP
   输入参数:ftpserverIP－FTP Server的IP
                          username－用户名
   				password－密码
   输出参数:	无
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
LONG  autoload_ftp_server_ip_set( UCHAR *ftpServerIp, UCHAR *userName, UCHAR *password, USHORT ftpPort, USHORT ftpDataPort )
{
	if( (NULL == ftpServerIp) && (NULL == userName) && (NULL == password) )
		return VOS_ERROR;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	if( ftpServerIp )
		VOS_StrCpy( auto_load_ftpserver.serverIp, ftpServerIp );
	if( userName )
		VOS_StrCpy( auto_load_ftpserver.userName, userName );
	if( password )
		VOS_StrCpy( auto_load_ftpserver.password, password );
	if( ftpPort )
		auto_load_ftpserver.port = ftpPort;
	if( ftpDataPort )
		auto_load_ftpserver.dataPort = ftpDataPort;

	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}

/* 功能: 获取FTP Server 的用户名、密码、IP
   输入参数:无
   输出参数:	ftpserverIP－FTP Server的IP
                          username－用户名
   				password－密码
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
LONG  autoload_ftp_server_ip_get( UCHAR *ftpServerIp, UCHAR *userName, UCHAR *password, USHORT *ftpPort, USHORT *ftpDataPort )
{
	if( (NULL == ftpServerIp) && (NULL == userName) && (NULL == password) && (NULL == ftpPort) && (NULL == ftpDataPort) )
		return VOS_ERROR;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	if( ftpServerIp )
		VOS_StrCpy( ftpServerIp, auto_load_ftpserver.serverIp );
	if( userName )
		VOS_StrCpy( userName, auto_load_ftpserver.userName );
	if( password )
		VOS_StrCpy( password, auto_load_ftpserver.password);
	if( ftpPort )
	{
		if( 0 == auto_load_ftpserver.port )
			*ftpPort = AUTO_LOAD_FTP_CTRL_PORT;
		else
			*ftpPort = auto_load_ftpserver.port;
	}
	if( ftpDataPort )
	{
		if( 0 == auto_load_ftpserver.dataPort )
			*ftpDataPort = AUTO_LOAD_FTP_DATA_PORT;
		else
			*ftpDataPort = auto_load_ftpserver.dataPort;
	}
	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}

LONG autoload_ftp_server_ip_del( UCHAR * ftpServerIp )
{
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	VOS_MemZero( auto_load_ftpserver.serverIp, sizeof(auto_load_ftpserver.serverIp) );
	VOS_MemZero( auto_load_ftpserver.userName, sizeof(auto_load_ftpserver.userName) );
	VOS_MemZero( auto_load_ftpserver.password, sizeof(auto_load_ftpserver.password) );
	auto_load_ftpserver.port = 0;
	auto_load_ftpserver.dataPort = 0;

	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}

#if 0
/*获取ONU的类型*//*add by shixh@20080505*/
LONG  get_onu_type(ULONG onudevIdx, char *typeString )
{
	short int PonPortIdx, OnuIdx;
	int slot, port;
	int len = 0;
	/*int   OnuType;*/

	/*if(typeString == NULL ) 
		return VOS_ERROR;*/
	
  	slot  = onudevIdx / 10000;
	port = (onudevIdx % 10000 ) / 1000;
	PonPortIdx = GetPonPortIdxBySlot(slot, port);
	if( PonPortIdx == VOS_ERROR )	/* modified by xieshl 20080811, 发现snmp异常，原因不详，这里增加判断 */
	{
		/*sys_console_printf(" ** get_onu_type: GetPonPortIdxBySlot error\r\n");*/
		return PonPortIdx;
	}
	OnuIdx = ( onudevIdx % 1000 )-1;
	/*
  	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != VOS_OK )
				{
				*vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n" );*
				return (VOS_ERROR );
				}
	*type=OnuType;
	*/
	return(GetOnuTypeString(PonPortIdx, OnuIdx, typeString, &len));
	/*sys_console_printf("get onu type is :%d\r\n",OnuType);*/
}

/*检查ONU _list ，看是否有ONU正在升级或正在配置*/
LONG  check_onu_list_upgrading_or_configing()
{
	int  i,j;
	auto_load_onu_list_t *pOnuList;

	for( i=0; i<20; i++ )
	{
		for( j=0; j<64; j++ )
		{
			pOnuList = &auto_load_onu_list[i][j];
			if( (pOnuList->onu_cfg_status == AUTO_LOAD_STATE_PROCESSING) || (pOnuList->onu_upg_status == AUTO_LOAD_STATE_PROCESSING) )
			{
				return  VOS_OK;
			}
		}
	}
	return  VOS_ERROR;
}

/*检查ONU_LIST,清除未升级的ONU状态*/
LONG  Clear_ONU_list_status()
{
	int i,j;
	auto_load_onu_list_t *pOnuList;

	for( i=0; i<20; i++ )
	{
		for( j=0; j<64; j++ )
		{
			pOnuList = &auto_load_onu_list[i][j];
			if( pOnuList->onu_upg_status != AUTO_LOAD_STATE_FINISHED)
				pOnuList->onu_upg_status=0;
			
		}
	}
	return VOS_OK;
}
#endif

/* added by chenfj 2009-4-13
     增加此函数用于CLI 调用*/
/*LONG  checkDateAndTime_cli(sysDateAndTime_t* pTime, struct vty *vty)
{
	short int ul_year=0,ul_month=0,ul_day,ul_hour=0,ul_minute=0,ul_second=0;
	ul_year=pTime->year;
	ul_month=pTime->month;
	ul_day=pTime->day;
	ul_hour=pTime->hour;
	ul_minute=pTime->minute;
	ul_second=pTime->second;

	if(ul_year>2079||ul_year<1980)
	{
		vty_out(vty,"\r\nin put year error ,please input :1980-2079\r\n");
		return VOS_ERROR;
	}
	if(ul_month>12||ul_month<1)
	{
		vty_out(vty,"\r\nin put month error,please input :1-12\r\n");
		return VOS_ERROR;
	}
	if(ul_day>31||ul_day<1)
	{
		vty_out(vty,"\r\nin put day error ,please input :1-31\r\n");
		return VOS_ERROR;
	}
	if(ul_hour>23||ul_hour<0)
	{
		vty_out(vty,"\r\nin put hour error,please input :0-23\r\n");
		return VOS_ERROR;
	}
	if(ul_minute>59||ul_minute<0)
	{
		vty_out(vty,"\r\nin put minute error,please input :0-59\r\n");
		return VOS_ERROR;
	}
	if(ul_second>59||ul_second<0)
	{
		vty_out(vty,"\r\nin put second error,please input :0-59\r\n");
		return VOS_ERROR;
	}
	return VOS_OK;
}*/

/* added by xieshl 20110119 */
/*检查ONU_LIST 表，是否有ONU 正在配置或升级*/	
LONG autoload_onu_status_in_processing()
{
	auto_load_onu_list_t *pOnuList;
	int i, j;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for( i=0; i<MAXPONCHIP; i++ )
	{
		for( j=0; j<MAXONUPERPON; j++ )
		{
 			pOnuList = &auto_load_onu_list[i][j];
			if( (pOnuList->onu_cfg_status == AUTO_LOAD_STATE_PROCESSING) || (pOnuList->onu_upg_status == AUTO_LOAD_STATE_PROCESSING) )
			{
				VOS_SemGive( onuAutoLoadSemId );
				return 1;
			}
	 	}
	}
	VOS_SemGive( onuAutoLoadSemId );
	return 0;
}

/*检查ONU_LIST表，清除未升级的ONU 的状态*/
LONG autoload_upg_onu_status_restart()
{
	auto_load_onu_list_t *pOnuList;
	ULONG slot, port, onuDevIdx;
	int i, j;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for(i=0;i<MAXPON;i++)
	{
		for(j=0;j<MAXONUPERPON;j++)
		{
			pOnuList = &auto_load_onu_list[i][j];

			if( pOnuList->onu_upg_status == AUTO_LOAD_STATE_PROCESSING )
			{
				slot = GetCardIdxByPonChip(i);
				port = GetPonPortByPonChip(i);
				onuDevIdx = MAKEDEVID(slot, port, j+1);
				if( autoload_ftp_client_ip_get(onuDevIdx, 0, 0, 0) == VOS_ERROR )
					pOnuList->onu_upg_status = AUTO_LOAD_STATE_IDLE;
			}
			else if( pOnuList->onu_upg_status != AUTO_LOAD_STATE_WAIT )
			{
				pOnuList->onu_upg_status = AUTO_LOAD_STATE_IDLE;
			}
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	AUTOLOAD_ONU_DEBUG(("auto-load upgrade onu-contral-list reset\r\n"));
	return VOS_OK;
}
LONG autoload_cfg_onu_status_reset()
{
	auto_load_onu_list_t *pOnuList;
	int i, j;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for(i=0;i<MAXPON;i++)
	{
		for(j=0;j<MAXONUPERPON;j++)
		{
			pOnuList = &auto_load_onu_list[i][j];
			if( pOnuList->onu_cfg_status != AUTO_LOAD_STATE_WAIT )
				pOnuList->onu_cfg_status = AUTO_LOAD_STATE_IDLE;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	AUTOLOAD_ONU_DEBUG(("auto-load config onu-contral-list reset\r\n"));
	return VOS_OK;
}

LONG autoload_upg_onu_status_abort()
{
	auto_load_onu_list_t *pOnuList;
	int i, j;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for(i=0;i<MAXPON;i++)
	{
		for(j=0;j<MAXONUPERPON;j++)
		{
			pOnuList = &auto_load_onu_list[i][j];
			if( pOnuList->onu_upg_status == AUTO_LOAD_STATE_WAIT )
				pOnuList->onu_upg_status = AUTO_LOAD_STATE_ABORT;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	AUTOLOAD_ONU_DEBUG(("auto-load upgrade abort\r\n"));
	return VOS_OK;
}

LONG autoload_cfg_onu_status_abort()
{
	auto_load_onu_list_t *pOnuList;
	int i, j;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for( i=0; i<MAXPONCHIP; i++ )
	{
		for( j=0; j<MAXONUPERPON; j++ )
		{
			pOnuList = &auto_load_onu_list[i][j];
			if( pOnuList->onu_cfg_status == AUTO_LOAD_STATE_WAIT )
				pOnuList->onu_cfg_status = AUTO_LOAD_STATE_ABORT;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	AUTOLOAD_ONU_DEBUG(("auto-load config abort\r\n"));
	return VOS_OK;
}

/* modified by chenfj 2009-4-13
     对时间做了比较,   规定
         1) 开始时间< 结束时间
         2) 当前时间< 结束时间
         3) 结束时间可以是 forever
     并修改了输出提示*/
QDEFUN (auto_load_start,
	auto_load_start_cmd,
	"auto-load upgrade start {[start-time] <start_time>}*1 {[end-time] <end_time>}*1",
	"onu auto-load task\n"
	"start onu upgrade task\n"
	"upgrade start\n"
	"select start time\n"
	"Input the start time, format:YYYY-MM-DD,HH:MM:SS or YY-MM-DD,HH:MM:SS\n"
	"select end time\n"
	"Input the end time, format:YYYY-MM-DD,HH:MM:SS or YY-MM-DD,HH:MM:SS.\n",
	&onuAutoLoadQueId )
{
	sysDateAndTime_t currentTime;
	sysDateAndTime_t startTime;
	sysDateAndTime_t endTime;

	/* modified by xieshl 20110119, 问题单11927 */
	/*检查ONU_LIST 表，是否有ONU 正在配置或升级*/
	int loadFlag = autoload_onu_status_in_processing();

	if( (1 == autoLoad_upg_enable) && (1 == loadFlag) )
	{
    		vty_out(vty, " auto-load upgrade already start !!\r\n");	
        	return  CMD_SUCCESS;
	}

	VOS_MemZero(&currentTime, sizeof(sysDateAndTime_t));
	if (VOS_OK != eventGetCurTime( &currentTime ) )
		return CMD_WARNING;

	if( argc == 0 )
	{
		VOS_MemCpy( &startTime, &currentTime, sizeof(sysDateAndTime_t) );
		getTomorrowTime( &startTime, &endTime );	/*比开始时间晚一天*/
	}
	else if( argc == 2 )
	{
		if( VOS_StriCmp( argv[0], "start-time") == 0 )
		{
			strToSysDateAndTime( argv[1], &startTime );
			if( checkDateAndTime(vty, &startTime) != VOS_OK )
				return CMD_WARNING;

			if( VOS_MemCmp((void *)&currentTime, (void *)&startTime, sizeof(sysDateAndTime_t)) > 0 )
				getTomorrowTime( &currentTime, &endTime );
			else
				getTomorrowTime( &startTime, &endTime );
		}
		else /*if( VOS_StriCmp(argv[0], "end-time") == 0 )*/
		{
			strToSysDateAndTime( argv[1], &endTime);
			if( checkDateAndTime(vty, &endTime) != VOS_OK )
				return CMD_WARNING;
			if( VOS_MemCmp((void *)&currentTime, (void *)&endTime, sizeof(sysDateAndTime_t)) >= 0 )
			{
				vty_out( vty, "\r\nError: end-time must be later than now\r\n" );
				return CMD_WARNING;
			}

			VOS_MemCpy(&startTime, &currentTime, sizeof(sysDateAndTime_t));
		}
	}
	else
	{
		strToSysDateAndTime( argv[1], &startTime );
		strToSysDateAndTime( argv[3], &endTime);
		if( checkDateAndTime(vty, &startTime) != VOS_OK )
			return CMD_WARNING;
		if( checkDateAndTime(vty, &endTime) != VOS_OK )
			return CMD_WARNING;
		if( VOS_MemCmp(&startTime, &endTime, sizeof(sysDateAndTime_t)) >= 0 )
		{
			vty_out( vty, "\r\nError:start-time must be earlier than end-time\r\n" );
			return CMD_WARNING;
		}
		if( VOS_MemCmp((void *)&currentTime, (void *)&endTime, sizeof(sysDateAndTime_t)) >= 0 )
		{
			vty_out( vty, "\r\nError: end-time must be later than now\r\n" );
			return CMD_WARNING;
		}
	}

	/*VOS_MemCpy( &autoLoad_startTime, &startTime, sizeof(sysDateAndTime_t) );
	VOS_MemCpy( &autoLoad_endTime, &endTime, sizeof(sysDateAndTime_t) );*/
	copyDateAndTime( &autoLoad_startTime, &startTime );
	copyDateAndTime( &autoLoad_endTime, &endTime );	/* modified by xieshl 20110519, 问题单12345 */
	
	onuAutoLoadUpgradeStart(vty);

	return  CMD_SUCCESS;
}

int cl_autoload_upg_stop( struct vty * vty )
{
	autoLoad_upg_enable = 0;
	autoLoad_upg_startFlag = 0;
	/*add by shixh20090708,问题单8618*/
	/*if( (autoLoad_upg_enable == 0) && (autoLoad_upg_startFlag == 0) )*/
	{
		autoload_upg_onu_status_abort();
		/*return  CMD_SUCCESS;*/
	}
	VOS_MemZero( &autoLoad_startTime, sizeof(sysDateAndTime_t) );
	VOS_MemZero( &autoLoad_endTime, sizeof(sysDateAndTime_t) );    /*modified by zhengyt 09-02-04 ,当自动升级去使能后，开始时间和结束时间也要相应设置，
											否则，mib上读出来的数据就是错误的*/
	if( (autoLoad_cfg_enable == 0) && (onuAutoLoadTimerId != VOS_ERROR) )
	{
		VOS_TimerDelete(MODULE_ONU_AUTO_LOAD, onuAutoLoadTimerId);
		onuAutoLoadTimerId = VOS_ERROR;
	}
	v2r1_printf( vty, " auto-load upgrade stop OK!\r\n" );
	return VOS_OK;
}

QDEFUN (auto_load_stop,
	auto_load_stop_cmd,
	"auto-load upgrade stop",
	"onu auto-load task\n"
	"stop auto-load task\n"
	"stop onu upgrade task\n",
	&onuAutoLoadQueId )
{
	/*if( autoLoad_upg_enable == 0 )
		return CMD_SUCCESS;*/
	/* modified by xieshl 20110124, 问题单11928，停止升级时，如果有ONU正在升级给出提示，强制停止时，
	    只是重置了ONU升级状态，并不通知ONU，ONU的升级过程仍然可以继续进行，这样的目的
	    是为了防止ONU升级状态和实际情况不符时可以强制改正，另外也可以解决停止后再快速
	    重新打开时无法重启的问题 */
	int loadFlag = autoload_onu_status_in_processing();
	if( (1 == autoLoad_upg_enable) && (1 == loadFlag) )
	{
    		vty_out(vty, " perhaps some onu is upgrading now\r\n");	
        	/*return  CMD_SUCCESS;*/
		vty_out( vty, " Are you sure stop forcibly now? [Y/N]" );
		vty->prev_node = vty->node;
		vty->node = CONFIRM_ACTION_NODE;
		vty->action_func = cl_autoload_upg_stop;
	}
	else
		cl_autoload_upg_stop( vty );

	return  CMD_SUCCESS;
}

QDEFUN (auto_load_config_start,
	auto_load_config_start_cmd,
	"auto-load config start",
	"onu auto-load config  task\n"
	"start auto-load config task\n"
	"start onu config task\n",
	&onuAutoLoadQueId )
{
	onuAutoLoadConfigStart(vty);
   	return CMD_SUCCESS;
}

int cl_autoload_cfg_stop( struct vty * vty )
{
	autoLoad_cfg_enable=0;
	autoLoad_cfg_startFlag=0;

	autoload_cfg_onu_status_abort();

	VOS_MemZero( &autoLoad_startTime, sizeof(sysDateAndTime_t) );
	VOS_MemZero( &autoLoad_endTime, sizeof(sysDateAndTime_t) ); 
	if( (autoLoad_upg_enable == 0) && (onuAutoLoadTimerId != VOS_ERROR) )
	{
		VOS_TimerDelete( MODULE_ONU_AUTO_LOAD, onuAutoLoadTimerId );
		onuAutoLoadTimerId = VOS_ERROR;
	}
	v2r1_printf( vty, " auto-load upgrade stop OK!\r\n" );
	return VOS_OK;
}


QDEFUN (auto_load_config_stop,
	auto_load_config_stop_cmd,
	"auto-load config stop",
	"onu auto-load config task\n"
	"stop auto-load config task\n"
	"stop onu config task\n",
	&onuAutoLoadQueId )
{
#if 0
	/*if( autoLoad_cfg_enable==0 )
		return CMD_SUCCESS;*/
	
	autoLoad_cfg_enable=0;
	autoLoad_cfg_startFlag=0;
	/*add by shixh20090708,问题单8618*/
	/*if( (autoLoad_cfg_enable == 0) && (autoLoad_cfg_startFlag == 0) )*/
	{
		autoload_cfg_onu_status_abort();
		/*return  CMD_SUCCESS;*/
	}
	
	if( (autoLoad_upg_enable == 0) && (onuAutoLoadTimerId != VOS_ERROR) )
	{
		VOS_TimerDelete( MODULE_ONU_AUTO_LOAD, onuAutoLoadTimerId );
		onuAutoLoadTimerId = VOS_ERROR;
	}
#else
	cl_autoload_cfg_stop( vty );
#endif
	return  CMD_SUCCESS;
}

/*问题单11492，[ftpport]和<1-65535>之间少一个空格*/
QDEFUN (config_ftpserver,
	config_ftpserver_cmd,
	"auto-load ftpserver <A.B.C.D> <ftpservername> <password> {[ftpport] <1-65535>}*1",
	"onu auto-config or auto-upgrade by ftp\n"
	"config ftp server information\n"
	"ftp server ip <A.B.C.D>\n"
	"ftp user name\n"
	"ftp password\n"
	"ftp port set\n"
	"ftp port range:1-65535\n",
	&onuAutoLoadQueId )
{
	USHORT port = AUTO_LOAD_FTP_CTRL_PORT;
	USHORT  dataport = AUTO_LOAD_FTP_DATA_PORT;	/*FTP 数据端口，默认为20*/
	LONG ipaddr;

	ipaddr = get_long_from_ipdotstring( argv[0] );
	if( CheckIpValid(ipaddr, (LONG) 0xffffffff, vty) != VOS_OK )
		return CMD_WARNING;
	
	if( argc == 5 )
		 port = VOS_AtoL( argv[4] );

	if( (VOS_StrLen(argv[1]) >= AUTO_LOAD_FTP_USER_LEN) ||
		(VOS_StrLen(argv[2]) >= AUTO_LOAD_FTP_PASSWORD_LEN) )
	{
		vty_out( vty, "parameter is too long!\r\n" );
		return  CMD_WARNING;
	}
	
	if( autoload_ftp_server_ip_set( argv[0], argv[1], argv[2], port, dataport ) != VOS_OK )
		return  CMD_WARNING;
	
	return CMD_SUCCESS;
}

QDEFUN (show_ftpserver,
	show_ftpserver_cmd,
	"show auto-load ftpserver",
	"show information\n"
	"onu auto-config or auto-upgrade by ftp\n"
	"show ftp server information\n",
	&onuAutoLoadQueId
	)
{
	UCHAR serverip[AUTO_LOAD_FTP_IP_STR_LEN+2];
	UCHAR servername[AUTO_LOAD_FTP_USER_LEN];
	UCHAR serverpassword[AUTO_LOAD_FTP_PASSWORD_LEN];
	USHORT port = 0, dataport = 0;

	VOS_MemZero( serverip , sizeof(serverip) );
	VOS_MemZero( servername , sizeof(servername) );
	VOS_MemZero( serverpassword , sizeof(serverpassword) );
	if( autoload_ftp_server_ip_get(serverip, servername, serverpassword, &port, &dataport) == VOS_OK )
	{
		vty_out(vty, "ftp server ip: %s\r\n", serverip);
		vty_out(vty, "username: %s\r\n", servername);
		vty_out(vty, "password: %s\r\n", serverpassword);
		vty_out(vty, "ftp port: %d\r\n", port);/*问题单8455*/
	}
	
       return CMD_SUCCESS;
}

QDEFUN (undo_ftpserver,
	undo_ftpserver_cmd,
	"undo auto-load ftpserver",
	NO_STR
	"onu auto-config or auto-upgrade by ftp\n"
	"ftp server\n",
	&onuAutoLoadQueId )
{
	if( autoload_ftp_server_ip_del(NULL) != VOS_OK )
		return  CMD_WARNING;
       return CMD_SUCCESS;
}

QDEFUN (config_client_ip,
	config_client_ip_cmd,
	"auto-load client-ip <A.B.C.D/M> gateway <A.B.C.D> vid <1-4094>",
	"onu auto-config or auto-upgrade by ftp\n"
	"config auto-load client ip addr.\n"
	"ip addr. and mask\n"
	"gateway addr.\n"
	"specific ip addr.\n"
	"vlan id\n"
	"specific vlan id\n",
	&onuAutoLoadQueId )
{
	USHORT vlanid;
	LONG  ipaddr, mask;

	if ( VOS_OK != IpListToIp( argv[0], &ipaddr, &mask ) )
	{
		vty_out( vty, "  %% Invalid IP address.\r\n" );
		return CMD_WARNING;
	}
	if( CheckIpValid(ipaddr, mask, vty) != VOS_OK )
		return CMD_WARNING;

	vlanid = VOS_AtoL( argv[2]);
	
	if( autoload_ftp_client_ip_set(vty, argv[0], argv[1],vlanid) != VOS_OK )
		return  CMD_WARNING;

	return CMD_SUCCESS;
}

QDEFUN (show_client_ip_gateway,
	show_client_ip_gateway_cmd,
	"show auto-load client-information",
	"show information\n"
	"onu auto-config or auto-upgrade by ftp\n"
	"show client ip.gateway.vid information\n",
	&onuAutoLoadQueId )
{
	int i;
	vty_out( vty, "client-ip           gateway           vid     status\r\n" );
	vty_out( vty, "-----------------------------------------------------\r\n");

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	for(i=0;i<AUTO_LOAD_FTP_CLIENT_IPNUM;i++)
	{
       	if(auto_load_onuctrl[i].rowStatus == AUTO_LOAD_STATE_USING)
		{
			vty_out( vty, "%-20s%-18s%-8d", auto_load_onuctrl[i].ftpClient.ipMask, auto_load_onuctrl[i].ftpClient.ipGateway, auto_load_onuctrl[i].ftpClient.vid );
			if(auto_load_onuctrl[i].status == AUTO_LOAD_STATE_USING)
			{	
				vty_out(vty, "%d\r\n", auto_load_onuctrl[i].onuDevIdx);
			}
			else
			{	
				vty_out(vty, "idle\r\n");
			}
		}
	}        
	VOS_SemGive( onuAutoLoadSemId );

	vty_out(vty, "\r\n");

	return CMD_SUCCESS;
}


QDEFUN (delete_client_ip_gateway,
	delete_client_ip_gateway_cmd,
	"undo auto-load client-ip <A.B.C.D/M>",
	"delete information\n"
	"onu auto-config or auto-upgrade by ftp\n"
	"delete client ip information\n"
	"the specific ip\n",
	&onuAutoLoadQueId )
{
	if( autoload_ftp_client_ip_del(argv[0]) != VOS_OK )
	{
		vty_out(vty, "\r\n Err:the ip addr is using or nonexistent\r\n");
		return CMD_WARNING;  
	}
	return  CMD_SUCCESS;
}

DEFUN (
	show_auto_load_upgrade_file,
	show_auto_load_upgrade_file_cmd,
	"show auto-load upgrade-file",
	"display auto load upgrade file\n"
	"auto load\n"
	"upgrade file\n")
{	
	int i=0,j;
	UCHAR *pDevType;
	UCHAR *pFileType;
	UCHAR filename[32];
	
	vty_out(vty,"\r\nonuType           fileType    filename                 version\r\n");
	vty_out(vty,"----------------------------------------------------------------\r\n");

	for( ; i<V2R1_ONU_MAX; i++ )
	{
		for( j=0; j<AUTO_LOAD_FILETYPE_COUNT; j++ )
		{
			pFileType = autoload_filetype_to_str( auto_load_filetype_list[i].file_type[j].filetype );
			if( NULL == pFileType )
				continue;
			pDevType = GetDeviceTypeString( i );
			VOS_Sprintf( filename, "%s_%s_%s", pDevType, pFileType, auto_load_filetype_list[i].file_type[j].ver );
			vty_out( vty, "%-18s%-12s%-25s%s\r\n", pDevType, pFileType, filename, auto_load_filetype_list[i].file_type[j].ver );
		}
	}
	return  CMD_SUCCESS;
}

QDEFUN(
	auto_load_upgrade_onu,
	auto_load_upgrade_onu_cmd,
	"auto-load upgrade pon <slotId/port> {<onuid_list>}*1",
	"auto load upgrade onu\n"
	"auto load upgrade\n"
	"input pon port\n"
	"input slot/port eg:5/1\n"
	OnuIDStringDesc,
	&onuAutoLoadQueId )
{
	ULONG ulslot=0,ulport=0;	
	int i=0;
	short int PonPortIdx=0;
	ULONG  ulonuIdx=0;
	auto_load_onu_list_t *pOnuList;
	
	VOS_Sscanf(argv[0], "%d/%d", &ulslot, &ulport);
	PonPortIdx=GetPonPortIdxBySlot(ulslot, ulport);
	/*CHECK_PON_RANGE*/

	if( PonCardSlotPortCheckWhenRunningByVty(ulslot, ulport, vty) != ROK )
		return(CMD_WARNING);
	PonPortIdx = GetPonPortIdxBySlot( (short int)ulslot, (short  int)ulport );
	if( PonPortIdx == VOS_ERROR )
	{ 
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	if( argc == 1 )
	{
		for( ; i<MAXONUPERPON; i++ )
		{
			pOnuList = &auto_load_onu_list[PonPortIdx][i];
			if( pOnuList->onu_upg_status != AUTO_LOAD_STATE_PROCESSING )
				pOnuList->onu_upg_status = AUTO_LOAD_STATE_WAIT;
			pOnuList->onu_upg_ftp_direct = AUTO_LOAD_DIR_SERVER2ONU;
		}
	}
	else if( argc == 2 )
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID(argv[1], ulonuIdx)
	 	{
	 		pOnuList = &auto_load_onu_list[PonPortIdx][ulonuIdx-1];
			if( pOnuList->onu_upg_status != AUTO_LOAD_STATE_PROCESSING )
			 	pOnuList->onu_upg_status = AUTO_LOAD_STATE_WAIT;
			pOnuList->onu_upg_ftp_direct = AUTO_LOAD_DIR_SERVER2ONU;
		}
		END_PARSE_ONUID_LIST_TO_ONUID();	
	}
	VOS_SemGive( onuAutoLoadSemId );
	
	return CMD_SUCCESS;
}

/* modified by chenfj 2009-4-14
    应可指定某一类型ONU 升级; 
    在实际应用现场, 每当某个类型ONU 有了新的软件版本, 这会很适用
    */
QDEFUN(
	auto_load_upgrade_onu_onutype,
	auto_load_upgrade_onu_onutype_cmd,
	"auto-load upgrade type [<onu_type>|all-onu]",
	"auto load upgrade onu\n"
	"upgrade onu\n"
	/*"onu type eg:"DEVICE_TYPE_NAME_GT811A_STR","DEVICE_TYPE_NAME_GT812A_STR" ...\n"*/
	"specific onu type\n"
	"onu type\n"
	"all onu-type\n",
	&onuAutoLoadQueId )
{
	int PonPortIdx=0, OnuIdx=0;
	int All_onu_flag = 0;
	char TypeString[ONU_TYPE_LEN+2]={0};
	int  StringLen =0;
	ulong_t onutype = RERROR;
	auto_load_onu_list_t *pOnuList;

	if( VOS_StrCmp(argv[0],"all-onu") == 0 )
		All_onu_flag = 1 ;
	else
	{
		/* modified by chenfj 2009-6-10
			增加显示信息，用于提示ONU 类型*/
		if( SearchOnuTypeFullMatch(argv[0]) == RERROR )
		{
			NotificationOnuTypeString(vty);	
			return(CMD_WARNING);
		}
		else
		{
			onutype = SearchOnuTypeFullMatch(argv[0]) ;/*add by shixh20091027*/
		}
	}
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for( PonPortIdx=0; PonPortIdx<MAXPON; PonPortIdx++ )
	{
		for( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++ )
		{
			pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
			if( All_onu_flag != 0 )
			{
				if( pOnuList->onu_upg_status != AUTO_LOAD_STATE_PROCESSING )
					pOnuList->onu_upg_status = AUTO_LOAD_STATE_WAIT;
				pOnuList->onu_upg_ftp_direct = AUTO_LOAD_DIR_SERVER2ONU;
			}
			else
			{
				if(GetOnuTypeString(PonPortIdx,OnuIdx, TypeString, &StringLen) == CMD_SUCCESS)
				{
					if(VOS_StrCmp(TypeString, GetDeviceTypeString(onutype)/*argv[0]*/) == 0)
					{
						if( pOnuList->onu_upg_status != AUTO_LOAD_STATE_PROCESSING )
							pOnuList->onu_upg_status = AUTO_LOAD_STATE_WAIT;
						pOnuList->onu_upg_ftp_direct = AUTO_LOAD_DIR_SERVER2ONU;
					}
				}
			}
		}
	}
	VOS_SemGive( onuAutoLoadSemId );

	return CMD_SUCCESS;
}

/*字符串的中的小写转换成大写*/
/*uchar_t  *string_small_to_capital(uchar_t  string[32])
{
	static char  string_to[32]={0};
	int  i;

	for(i=0;i<32;i++)
	{
		if((string[i]<122) && (string[i]>97))
		{
			string_to[i]=string[i]-32;
		}
		else
			string_to[i]=string[i];
	}
	return string_to;
}*/

/*检查字符串是否为大写*/
LONG  check_version_is_upper(uchar_t* pstring)
{
	if( NULL == pstring )
		return VOS_ERROR;
	if((pstring[0]!='V')||(pstring[2]!='R')||(pstring[5]!='B'))
		return  VOS_ERROR;

	return VOS_OK;
}

QDEFUN(
	auto_load_config_onu_,
	auto_load_config_onu_cmd,
	"auto-load config <slotId/port> {[onu] <onuid_list>}*1 {[direction] [server2onu|onu2server]}*1 {[filename] <namestring>}*1",
	"auto load config onu\n"
	"auto config onu\n"
	"input slot/port eg:5/2\n"
	"onuidx\n"
	"Please input onuId list,e.g 1,3-5,7, etc. the range for onuId is 1-64\n"
	"config load direct, default is server to onu\n"
	"download from ftp server to onu\n"
	"upload from onu to server\n"
	"config-file name\n"
	"input filename, default is onu's mac or \"Olttype\"_\"Ponslot\"_\"Ponport\"_\"Onutype\".ini\n",
	&onuAutoLoadQueId )
{	
	ULONG ulslot=0,ulport=0;
	short int ulponportIdx=0;
	int i=0,k=0;
	ULONG ulonuIdx=0;
	UCHAR direct=0;
	UCHAR ulfilename[32+1]="";
	char ulMacAddr[8]={0};
	ULONG MacAddrLen=0;
	/*unsigned char FilenameExist = V2R1_DISABLE;
	UCHAR  directionExist=V2R1_DISABLE;*/
	auto_load_onu_list_t *pOnuList;
	
	/*IFM_ParseSlotPort(argv[0], &ulslot, &ulport);*/
	VOS_Sscanf(argv[0], "%d/%d", &ulslot, &ulport);
	if(PonCardSlotPortCheckWhenRunningByVty(ulslot, ulport,vty) != ROK)
		return(CMD_WARNING);
	ulponportIdx = GetPonPortIdxBySlot( (short int)ulslot, (short int)ulport );
	if (ulponportIdx == VOS_ERROR)
	{ 
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	
	for( k=1; k<argc; k++ )
	{
		if(VOS_StrCmp(argv[k],"direction")==0)
		{
			if(VOS_StrCmp(argv[k+1], "server2onu")==0)
				direct=AUTO_LOAD_DIR_SERVER2ONU;
			else if(VOS_StrCmp(argv[k+1],"onu2server")==0)
				direct=AUTO_LOAD_DIR_ONU2SERVER;
		}
		else if(VOS_StrCmp(argv[k],"filename")==0)
		{
			if(VOS_StrLen(argv[k+1]) >32 )
			{
				vty_out(vty,"\r\nconfig-file name is to long, should be less than 32\r\n");
				return( CMD_WARNING);
			}
			VOS_StrCpy(ulfilename, argv[k+1]);
		}
	}

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	if( argc == 1 )/*onu索引是该pon 下的所有onu,方向是默认方向，文件名是默认文件名*/
	{
		for(;i<MAXONUPERPON;i++)
		{
			GetOnuMacAddr(ulponportIdx, i, ulMacAddr, (int*)&MacAddrLen);

			pOnuList = &auto_load_onu_list[ulponportIdx][i];
			pOnuList->onu_cfg_status=AUTO_LOAD_STATE_WAIT;
			pOnuList->onu_cfg_ftp_direct=AUTO_LOAD_DIR_DEFAULT;
			/*VOS_MemZero(pOnuList->onu_cfg_filename, 32);*/
			VOS_MemCpy( pOnuList->onu_cfg_filename, ulMacAddr, MacAddrLen);

			/*AUTOLOAD_ONU_DEBUG(("1:congfig ponport %d onu %d auto cfg stat  %d dir %d filename %s macaddr %s\r\n",
					ulponportIdx,(i+1), pOnuList->onu_cfg_status, pOnuList->onu_cfg_ftp_direct, pOnuList->onu_cfg_filename, ulMacAddr));*/
		}
	}
	else if( argc == 3 )
	{	
		if(VOS_StrCmp(argv[1], "onu")==0)/*如果输入的是onu索引，配置相应的onu，并且给方向和文件名配置默认值*/
		{
			BEGIN_PARSE_ONUID_LIST_TO_ONUID(argv[2], ulonuIdx)
		 	{
				GetOnuMacAddr(ulponportIdx, ulonuIdx-1, ulMacAddr, (int*)&MacAddrLen);
				pOnuList = &auto_load_onu_list[ulponportIdx][ulonuIdx-1];
			 	pOnuList->onu_cfg_status = AUTO_LOAD_STATE_WAIT;
				pOnuList->onu_cfg_ftp_direct = AUTO_LOAD_DIR_DEFAULT;
				/* deleted by chenfj 2009-4-14
				     此处未指定配置文件名, 故应清空. 下同*/
				VOS_MemCpy( pOnuList->onu_cfg_filename, ulMacAddr, MacAddrLen );
			}
			END_PARSE_ONUID_LIST_TO_ONUID();
		}
		else if(VOS_StrCmp(argv[1], "direction")==0)/*如果输入的是方向，索引默认为该pon下的所有onu，文件名为默认名*/
		{
			for(i=0;i<MAXONUPERPON;i++)
			{
				GetOnuMacAddr(ulponportIdx, i, ulMacAddr, (int*)&MacAddrLen);
				pOnuList = &auto_load_onu_list[ulponportIdx][i];
				pOnuList->onu_cfg_status = AUTO_LOAD_STATE_WAIT;
				pOnuList->onu_cfg_ftp_direct = direct;
				VOS_MemCpy( pOnuList->onu_cfg_filename, ulMacAddr, MacAddrLen );
			}
			/*directionExist=V2R1_ENABLE;*/
		}
		else if(VOS_StrCmp(argv[1], "filename")==0)/*如果输入的是文件名，索引默认为该pon下的所有onu, 方向为默认方向*/
		{
			for(i=0;i<MAXONUPERPON;i++)
			{
				pOnuList = &auto_load_onu_list[ulponportIdx][i];
				pOnuList->onu_cfg_status=AUTO_LOAD_STATE_WAIT;
				pOnuList->onu_cfg_ftp_direct=AUTO_LOAD_DIR_DEFAULT;
				VOS_StrCpy( pOnuList->onu_cfg_filename, ulfilename);
				/*FilenameExist = V2R1_ENABLE;*/
			}
		}
	}
	else if(argc==5)
	{
		if(VOS_StrCmp(argv[1], "onu")==0)
		{
			if(VOS_StrCmp(argv[3], "direction")==0)/*输入参数是onu索引和方向，文件名为默认值*/
			{
				BEGIN_PARSE_ONUID_LIST_TO_ONUID(argv[2], ulonuIdx)
			 	{
					GetOnuMacAddr(ulponportIdx, ulonuIdx-1, ulMacAddr, (int*)&MacAddrLen);
					pOnuList = &auto_load_onu_list[ulponportIdx][ulonuIdx-1];
				 	pOnuList->onu_cfg_status = AUTO_LOAD_STATE_WAIT;
					pOnuList->onu_cfg_ftp_direct = direct;
					VOS_MemZero( pOnuList->onu_cfg_filename, 32 );
					/*VOS_MemCpy(pOnuList->onu_cfg_filename, ulMacAddr, MacAddrLen);*/

					/*AUTOLOAD_ONU_DEBUG(("5:congfig onu %d-%d auto cfg stat:%d dir:%d %02x%02x.%02x%02x.%02x%02x\r\n",
							ulponportIdx, ulonuIdx, pOnuList->onu_cfg_status, pOnuList->onu_cfg_ftp_direct,
							pOnuList->onu_cfg_filename[2], pOnuList->onu_cfg_filename[3],
							pOnuList->onu_cfg_filename[4], pOnuList->onu_cfg_filename[5]) );*/
				}
				END_PARSE_ONUID_LIST_TO_ONUID();
				/*directionExist=V2R1_ENABLE;*/
			}
			if( VOS_StrCmp(argv[3], "filename") == 0 )/*输入参数是onu索引和文件名， 方向为默认值*/
			{
				BEGIN_PARSE_ONUID_LIST_TO_ONUID(argv[2], ulonuIdx)
			 	{
					pOnuList = &auto_load_onu_list[ulponportIdx][ulonuIdx-1];
				 	pOnuList->onu_cfg_status = AUTO_LOAD_STATE_WAIT;
					pOnuList->onu_cfg_ftp_direct = AUTO_LOAD_DIR_DEFAULT;
					VOS_StrCpy( pOnuList->onu_cfg_filename, ulfilename );
				}
				END_PARSE_ONUID_LIST_TO_ONUID();
				/*FilenameExist = V2R1_ENABLE;*/
			}
		}
		else if(VOS_StrCmp(argv[1], "direction")==0)
		{
			if(VOS_StrCmp(argv[3], "filename")==0)/*输入参数为方向和文件名，onu索引为该pon 下的所有onu*/
			{
				for(i=0;i<MAXONUPERPON;i++)
				{
					pOnuList = &auto_load_onu_list[ulponportIdx][i];
					pOnuList->onu_cfg_status = AUTO_LOAD_STATE_WAIT;
					pOnuList->onu_cfg_ftp_direct = direct;
					VOS_StrCpy( pOnuList->onu_cfg_filename, ulfilename);
				}
				/*FilenameExist = V2R1_ENABLE;*/
			}
			/*directionExist = V2R1_ENABLE;*/
		}
	}
	else if(argc==7)/*全部输入参数*/
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID(argv[2], ulonuIdx)
	 	{
			GetOnuMacAddr(ulponportIdx, ulonuIdx-1, ulMacAddr, (int*)&MacAddrLen);
			pOnuList = &auto_load_onu_list[ulponportIdx][ulonuIdx-1];
		 	pOnuList->onu_cfg_status = AUTO_LOAD_STATE_WAIT;
			pOnuList->onu_cfg_ftp_direct = direct;
			VOS_StrCpy( pOnuList->onu_cfg_filename, ulfilename );
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
		/*FilenameExist = V2R1_ENABLE;
		directionExist = V2R1_ENABLE;*/
	}
	else
	{
		VOS_SemGive( onuAutoLoadSemId );
		vty_out(vty,"para error\r\n");
		return VOS_ERROR;
	}
	VOS_SemGive( onuAutoLoadSemId );
	
	/*if(FilenameExist != V2R1_ENABLE)
	{
		vty_out(vty,"no config-file name input, so default file name is onu's mac or \"Olttype\"_\"Ponslot\"_\"Ponport\"_\"Onutype\".ini\r\n");
	}

	if(directionExist != V2R1_ENABLE)
	{
		vty_out(vty,"no auto-config direction input, so default direction is server to onu\r\n");
	}*/
	
	return  VOS_OK;
}

/* added by xieshl 20101108 , 西班牙 */
extern LONG GetMacAddr( CHAR * szStr, CHAR * pucMacAddr );
DEFUN(
	auto_load_config_onu_mac,
	auto_load_config_onu_mac_cmd,
	"auto-load config onu-mac <H.H.H> {[direction] [server2onu|onu2server]}*1 {[filename] <namestring>}*1",
	"auto load config onu\n"
	"auto config onu\n"
	"onu mac address\n"
	"input onu mac address\n"
	"config direct, default direction is server to onu\n"
	"config to onu\n"
	"upload onu current config to server\n"
	"config-file name,default file name is onu's mac or \"Olttype\"_\"Ponslot\"_\"Ponport\"_\"Onutype\".ini\n"
	"config-file name\n"
)
{	
	ULONG ulslot=0,ulport=0;
	short int ulponportIdx=0;
	int i;
	ULONG ulonuIdx;
	
	UCHAR direct = AUTO_LOAD_DIR_SERVER2ONU;
	char ulMacAddr[8];
	ULONG MacAddrLen=0;
	unsigned char FilenameExist = V2R1_DISABLE;
	UCHAR  directionExist=V2R1_DISABLE;

	LONG onuId;
	if( GetMacAddr((CHAR*)argv[0], ulMacAddr ) != VOS_OK )
	{
	        vty_out( vty, "  %% Invalid MAC address.\r\n" );
	        return CMD_WARNING;
	}  
	onuId = GetOnuEntryByMac( ulMacAddr );
	if ((RERROR) == onuId)
	{
		vty_out( vty, "  %% The mac is not exist.\r\n" );
		return CMD_WARNING;
	}

	ulponportIdx = onuId/MAXONUPERPON;
	ulonuIdx = onuId % MAXONUPERPON;

	ulslot = GetCardIdxByPonChip(ulponportIdx);
	ulport = GetPonPortByPonChip(ulponportIdx);
	
	if(PonCardSlotPortCheckWhenRunningByVty(ulslot, ulport,vty) != ROK)
			return(CMD_WARNING);

	for( i=1; i<argc; i++ )
	{
		if( VOS_StrCmp(argv[i],"direction") == 0 )
		{
			if( VOS_StrCmp(argv[i+1], "onu2server") == 0 )
				direct = AUTO_LOAD_DIR_ONU2SERVER;
			directionExist = V2R1_ENABLE;
		}
		else if( VOS_StrCmp(argv[i], "filename") == 0 )
		{
			if( VOS_StrLen(argv[i+1]) > 32 )
			{
				vty_out(vty,"\r\nconfig-file name is to long, should be less than 32\r\n");
				return( CMD_WARNING);
			}
			VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
			VOS_StrCpy(auto_load_onu_list[ulponportIdx][ulonuIdx].onu_cfg_filename, argv[i+1] );
			VOS_SemGive( onuAutoLoadSemId );
			FilenameExist = V2R1_ENABLE;
		}
	}

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	auto_load_onu_list[ulponportIdx][ulonuIdx].onu_cfg_status=AUTO_LOAD_STATE_WAIT;
	auto_load_onu_list[ulponportIdx][ulonuIdx].onu_cfg_ftp_direct = direct;
	if( FilenameExist != V2R1_ENABLE )
		VOS_MemCpy(auto_load_onu_list[ulponportIdx][ulonuIdx].onu_cfg_filename, ulMacAddr, MacAddrLen);
	VOS_SemGive( onuAutoLoadSemId );
	
	return  VOS_OK;
}

QDEFUN(
	config_onu_upgrade_file,
	config_onu_upgrade_file_cmd,
	"auto-load upgrade-file <onu_type> [boot|app|fw|voip|fpga] <ver>",
	"config auto load upgrade file\n"
	"auto load file\n"
	/*"onu type eg:"DEVICE_TYPE_NAME_GT811A_STR","DEVICE_TYPE_NAME_GT812A_STR" ...\n"*/
	"specific onu type\n"
	"upgrade boot file\n"
	"upgrade app file\n"
	"upgrade fw file\n"
	"upgrade voip file\n"
	"upgrade fpga file\n"
	"upgrade file ver:eg:V1R02B050\n",
	&onuAutoLoadQueId 
)
{
	int i=0,j=0;
	UCHAR ulFileType=0;
	UCHAR ulOnuVer[32]="";
	if( argc !=3 ) 
	{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
	}

	/* modified by chenfj 2009-6-10
			增加显示信息，用于提示ONU 类型*/
	if(SearchOnuTypeFullMatch(argv[0]) == RERROR)
	{
		NotificationOnuTypeString(vty);	
		return(CMD_WARNING);
	}
	
	if( VOS_StrCmp(argv[1], "boot") == 0 )
	{
		ulFileType = AUTO_LOAD_FILETYPE_BOOT;
		j = 0;
	}
	else if( VOS_StrCmp(argv[1], "app") == 0 )
	{
		ulFileType = AUTO_LOAD_FILETYPE_APP;
		j = 1;
	}
	else if( VOS_StrCmp(argv[1], "fw") == 0 )
	{
		ulFileType = AUTO_LOAD_FILETYPE_FW;
		j = 2;
	}
	else if( VOS_StrCmp(argv[1], "voip") == 0 )
	{
		ulFileType = AUTO_LOAD_FILETYPE_VOIP;
		j = 3;
	}
	else if( VOS_StrCmp(argv[1], "fpga") == 0 )
	{
		ulFileType = AUTO_LOAD_FILETYPE_FPGA;
		j = 4;
	}
	else{
		vty_out(vty, " para error!!\r\n");
		return ( CMD_WARNING);
		}
	
      /*add by shixh20090908*/
	if( check_version_is_upper(argv[2]) != VOS_OK )
		{
               vty_out(vty,"ver should be capital letters,eg:V1R02B050\r\n");
		 return  ( CMD_WARNING);/*问题单9040*/
		}
	else	
		VOS_StrCpy(ulOnuVer, argv[2]);

	for( i=0; i< V2R1_ONU_MAX; i++ )
	{
		if( VOS_StrCmp(argv[0], GetDeviceTypeString(i)) == 0 )
		{
			auto_load_filetype_list[i].file_type[j].filetype = ulFileType;
			VOS_StrCpy( auto_load_filetype_list[i].file_type[j].ver, ulOnuVer );
			/*vty_out(vty,"auto load file type %d, ver %s\r\n",ulFileType,ulOnuVer);*/
		}
	}
	return  CMD_SUCCESS;
}

QDEFUN(
	delete_onu_file_list,
	delete_onu_file_list_cmd,
	"undo auto-load upgrade-file <onu_type> {[boot|app|fw|voip|fpga]}*1",
	"delete auto load onu file\n"
	"onu auto load onu\n"
	"upgrade file\n"
	/*"onu type eg:"DEVICE_TYPE_NAME_GT811A_STR","DEVICE_TYPE_NAME_GT812A_STR" ...\n".*/
	"specific onu type\n"
	"delete boot file\n"
	"delete app file\n"
	"delete fw file\n"
	"delete voip file\n"
	"delete fpga file\n",
	&onuAutoLoadQueId
)
{
	int i=0,j=0;
	UCHAR ulFileType=0;

	/* modified by chenfj 2009-6-10
			增加显示信息，用于提示ONU 类型*/
	if(SearchOnuTypeFullMatch(argv[0]) == RERROR)
	{
		NotificationOnuTypeString(vty);	
		return(CMD_WARNING);
	}
	
	if(argc==1)
	{
		for(i=0; i< V2R1_ONU_MAX; i++)
		{	
			/*if(VOS_StrCmp(argv[0], DeviceType_1[i])==0)*/
			if(VOS_StrCmp(argv[0], GetDeviceTypeString(i))==0)
			{
				for(j=0; j<AUTO_LOAD_FILETYPE_COUNT; j++)
				{
					auto_load_filetype_list[i].file_type[j].filetype=0;
					VOS_MemZero(auto_load_filetype_list[i].file_type[j].ver,32);
				}
			}
		}
	}
	else if (argc==2)
	{
		if( VOS_StrCmp(argv[1], "boot") == 0 )
			ulFileType = AUTO_LOAD_FILETYPE_BOOT;
		else if( VOS_StrCmp(argv[1], "app") == 0 )
			ulFileType = AUTO_LOAD_FILETYPE_APP;
		else if( VOS_StrCmp(argv[1], "fw") == 0)
			ulFileType = AUTO_LOAD_FILETYPE_FW;
		else if( VOS_StrCmp(argv[1], "voip") == 0)
			ulFileType = AUTO_LOAD_FILETYPE_VOIP;
		else if( VOS_StrCmp(argv[1], "fpga") == 0)
			ulFileType = AUTO_LOAD_FILETYPE_FPGA;
		else
		{
			vty_out(vty, " para error!!\r\n");
			return VOS_ERROR;
		}
		
		for(;i<V2R1_ONU_MAX;i++)
		{
			if(VOS_StrCmp(argv[0], GetDeviceTypeString(i)) == 0 )
			{
				for( j=0; j<AUTO_LOAD_FILETYPE_COUNT; j++ )
				{	
					if(auto_load_filetype_list[i].file_type[j].filetype == ulFileType)
					{
						auto_load_filetype_list[i].file_type[j].filetype = 0;
						VOS_MemZero(auto_load_filetype_list[i].file_type[j].ver,32);
						break;
					}
				}
			}
		}
	}

	return  CMD_SUCCESS;
}

DEFUN (
	onu_upgrade_printf_information_enable,
	onu_upgrade_printf_information_enable_cmd,
	"debug auto-load",
	"debug\n"
	"debug onu auto load\n"
	)
{
   	autoload_onu_debug_flag=1;
	return  CMD_SUCCESS;
}

DEFUN (
	onu_upgrade_printf_information_disable,
	onu_upgrade_printf_information_disable_cmd,
	"undo debug auto-load",
	"Negate a command or set its defaults\n"
	"debug\n"
	"debug onu auto load\n"
	)
{
   	autoload_onu_debug_flag=0;
	return  CMD_SUCCESS;
}

/*DEFUN (
	show_onu_upgrade_printf_information_enable,
	show_onu_upgrade_printf_information_enable_cmd,
	"show debug auto-load",
	"show information\n"
	"debug\n"
	"show onu upgrade printf information enable\n"
	)
{	
	vty_out(vty, "onu upgrade printf information is:%s\r\n", (autoload_onu_debug_flag==1) ? "enable" : "disable" );
	return  CMD_SUCCESS;
}*/

/*此命令后来不用了*/
#if 0
DEFUN (
	onu_autoconfig_enable,
	onu_autoconfig_enable_cmd,
	"auto-load [enable|disable] {[config|upgrade]}*1",
	"onu auto-config or auto-upgrade by ftp\n"
	"enable onu auto load\n"
	"disable onu auto load\n"
	"specific auto-config enable\n"
	"specific auto-upgrade enable\n"
	)
{
	ULONG ebl=0;
       if (VOS_StrCmp( argv[0], "enable")==0)
      	{
      		ebl = 1;
      	}

	if( argc == 1 )
	{
   		autoLoad_cfg_enable=ebl;
	   	autoLoad_upg_enable=ebl;
	}
	else
	{
	       if (VOS_StrCmp( argv[1], "config")==0)
	   		autoLoad_cfg_enable=ebl;
		   
		if(VOS_StrCmp( argv[1], "upgrade")==0)
		   	autoLoad_upg_enable=ebl;
	}

	return  VOS_OK;
}
#endif

DEFUN (
	show_onu_autoconfig_enable,
	show_onu_autoconfig_enable_cmd,
	"show auto-load",
	"show information\n"
	"show onu auto-load enable\n"
	)
{
	if(autoLoad_cfg_enable==1)
		vty_out(vty, "onu autoconfig is :enable\r\n");
	else
		vty_out(vty, "onu autoconfig is :disable\r\n");

	if(autoLoad_upg_enable==1)
	{
		vty_out(vty, "onu upgrade is :enable\r\n");

		vty_out( vty, " start time: %d-%d-%d,%d:%d:%d\r\n", 
				autoLoad_startTime.year, autoLoad_startTime.month, autoLoad_startTime.day,
				autoLoad_startTime.hour, autoLoad_startTime.minute, autoLoad_startTime.second );
		vty_out( vty, " end time: %d-%d-%d,%d:%d:%d\r\n",
				autoLoad_endTime.year, autoLoad_endTime.month, autoLoad_endTime.day,
				autoLoad_endTime.hour, autoLoad_endTime.minute, autoLoad_endTime.second );
	}
	else
	{	
		vty_out( vty, "onu upgrade is :disable\r\n" );
		/*vty_out( vty, "autoLoad start time: -\r\n" );
		vty_out( vty, "autoLoad end time : -\r\n" );*/
	}

	return  CMD_SUCCESS;
}

char * autoload_upg_onu_status_str( LONG state )
{
	char *pStr;
	switch( state )
	{
		case AUTO_LOAD_STATE_IDLE:
			pStr = "idle";
			break;
		case AUTO_LOAD_STATE_WAIT:
			pStr = "waiting";
			break;
		case AUTO_LOAD_STATE_PROCESSING:
			pStr = "upgrading";
			break;
		case AUTO_LOAD_STATE_FINISHED:
			pStr = "upgraded";
			break;
		case AUTO_LOAD_STATE_FAILURE:
			pStr = "failure";
			break;
		case AUTO_LOAD_STATE_ABORT:
			pStr = "abort";
			break;
		default:
			pStr = "unknown";
			break;
	}
	return pStr;
}

DEFUN(
	auto_load_onu_upgrade_show,
	auto_load_onu_upgrade_show_cmd,
	"show auto-load upgrade onu {[waiting|upgrading|upgraded|failure|abort]}*1",
	"display auto load onu upgrade status\n"
	"auto load onu\n"
	"upgrade onu\n"
	"upgrade onu list\n"
	"show auto load waiting upgrade onu list\n"
	"show auto load upgrading onu list\n"
	"show auto load upgraded onu list\n"
	"show auto load upgrade failure onu list\n"
	"show abort onu list during upgrade\n"   /*问题单12950*/
)
{
	int i, j, k = 0;
	ULONG ulslot, ulport;
	LONG state = VOS_ERROR;
	auto_load_onu_list_t *pOnuList;

	if( argc == 1 )
	{
		for( i=AUTO_LOAD_STATE_IDLE+1; i<=AUTO_LOAD_STATE_ABORT; i++ )
		{
			if( VOS_StrCmp(argv[0], autoload_upg_onu_status_str(i)) == 0 )
			{
				state = i;
				break;
			}
		}
	}

	vty_out(vty,"\r\n Index    OnuIndex       status\r\n");
	vty_out(vty,"---------------------------------\r\n");

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for( i=0; i<MAXPONCHIP; i++ )
	{
		for( j=0; j<MAXONUPERPON; j++ )
		{
			if(ThisIsValidOnu(i,j) != ROK)
				continue;
			pOnuList = &auto_load_onu_list[i][j];
			if( (state == VOS_ERROR) || (state == pOnuList->onu_upg_status) )
			{
				k++;
				ulslot = GetCardIdxByPonChip(i);
				ulport = GetPonPortByPonChip(i);
				vty_out( vty, "  %-9d%d/%d/%-9d%s\r\n", k, ulslot, ulport, j+1, autoload_upg_onu_status_str(pOnuList->onu_upg_status) );
			}
		}
	}
	VOS_SemGive( onuAutoLoadSemId );

	return CMD_SUCCESS;
}

char * autoload_cfg_onu_status_str( LONG state )
{
	char *pStr;
	switch( state )
	{
		case AUTO_LOAD_STATE_IDLE:
			pStr = "idle";
			break;
		case AUTO_LOAD_STATE_WAIT:
			pStr = "waiting";
			break;
		case AUTO_LOAD_STATE_PROCESSING:
			pStr = "configing";
			break;
		case AUTO_LOAD_STATE_FINISHED:
			pStr = "configed";
			break;
		case AUTO_LOAD_STATE_FAILURE:
			pStr = "failure";
			break;
		case AUTO_LOAD_STATE_ABORT:
			pStr = "abort";
			break;
		default:
			pStr = "unknown";
			break;
	}
	return pStr;
}
DEFUN(
	auto_load_onu_config_show,
	auto_load_onu_config_show_cmd,
	"show auto-load config onu {[waiting|configing|configed|failure|abort]}*1",
	"display auto load onu config\n"
	"auto load onu\n"
	"config onu\n"
	"onu list\n"
	"show auto load waiting config onu list\n"
	"show auto load configing onu list\n"
	"show auto load configed onu list\n"
	"show auto load config failure onu list\n"
	"show abort onu list during upgrade\n"   /*问题单12950*/
)
{
	int i, j, k=0;
	ULONG ulslot, ulport;
	LONG state = VOS_ERROR;
	auto_load_onu_list_t *pOnuList;

	if( argc == 1 )
	{
		for( i=AUTO_LOAD_STATE_IDLE+1; i<=AUTO_LOAD_STATE_ABORT; i++ )
		{
			if( VOS_StrCmp(argv[0], autoload_cfg_onu_status_str(i)) == 0 )
			{
				state = i;
				break;
			}
		}
	}

	vty_out(vty,"\r\n Index    OnuIndex       status\r\n");
	vty_out(vty,"---------------------------------\r\n");
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for( i=0; i<MAXPONCHIP; i++ )
	{
		for( j=0; j<MAXONUPERPON; j++ )
		{
			if(ThisIsValidOnu(i,j) != ROK)
				continue;
			pOnuList = &auto_load_onu_list[i][j];
			if( (state == VOS_ERROR) || (state == pOnuList->onu_cfg_status) )
			{
				k++;
				ulslot = GetCardIdxByPonChip(i);
				ulport = GetPonPortByPonChip(i);
				vty_out( vty, "  %-9d%d/%d/%-9d%s\r\n", k, ulslot, ulport, j+1, autoload_cfg_onu_status_str(pOnuList->onu_cfg_status) );
			}
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	return CMD_SUCCESS;
}


LONG  onuAutoLoadCommandInstall()
{	
	install_element ( CONFIG_NODE, &auto_load_start_cmd);
	install_element ( CONFIG_NODE, &auto_load_stop_cmd);
	install_element ( CONFIG_NODE, &auto_load_config_start_cmd);
	install_element ( CONFIG_NODE, &auto_load_config_stop_cmd);
	/*install_element ( CONFIG_NODE, &show_waitting_onu_cmd);*/

	install_element ( CONFIG_NODE, &config_ftpserver_cmd);
	install_element ( CONFIG_NODE, &undo_ftpserver_cmd);	
	install_element ( CONFIG_NODE, &show_ftpserver_cmd);

	install_element ( CONFIG_NODE, &config_client_ip_cmd);
	install_element ( CONFIG_NODE, &show_client_ip_gateway_cmd);
	install_element ( CONFIG_NODE, &delete_client_ip_gateway_cmd);

	install_element ( CONFIG_NODE, &config_onu_upgrade_file_cmd);
	/*install_element ( CONFIG_NODE, &show_onu_upgrade_file_table_cmd);*/
	/*install_element ( CONFIG_NODE, &delete_onu_upgrade_file_cmd);*/

	/*install_element ( CONFIG_NODE, &onu_upgrade_cmd);*/

	install_element ( DEBUG_HIDDEN_NODE, &onu_upgrade_printf_information_enable_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &onu_upgrade_printf_information_disable_cmd);
	/*install_element ( DEBUG_HIDDEN_NODE, &show_onu_upgrade_printf_information_enable_cmd);*/

	/*install_element ( CONFIG_NODE, &onu_autoconfig_enable_cmd);*/
	install_element ( CONFIG_NODE, &show_onu_autoconfig_enable_cmd);

	/*install_element ( CONFIG_NODE, &onu_upgrade_enable_cmd);
	install_element ( CONFIG_NODE, &show_onu_upgrade_enable_cmd);

	install_element ( CONFIG_NODE, &set_show_run_enable_cmd);
	install_element ( CONFIG_NODE, &show_run_enable_cmd);*/

	/*install_element ( CONFIG_NODE, &onu_autoconfig_filename_cmd);*/
	/*install_element ( CONFIG_NODE, &show_onu_autoconfig_filename_cmd);*/
	/*install_element ( CONFIG_NODE, &delete_onu_autoconfig_filename_cmd);*/
	install_element(CONFIG_NODE,&delete_onu_file_list_cmd);
	install_element(CONFIG_NODE,&show_auto_load_upgrade_file_cmd);
	install_element(CONFIG_NODE,&auto_load_upgrade_onu_cmd);
	install_element(CONFIG_NODE,&auto_load_upgrade_onu_onutype_cmd);
	install_element(CONFIG_NODE,&auto_load_config_onu_cmd);
	install_element(CONFIG_NODE,&auto_load_config_onu_mac_cmd);	/* added by xieshl 20101108 */
	install_element(CONFIG_NODE,&auto_load_onu_upgrade_show_cmd);
	install_element(CONFIG_NODE,&auto_load_onu_config_show_cmd);

	return  VOS_OK;
}
 
/*判断ONU是否在正在配置的队列中*/
/*STATUS  testOnuisdoingconfig(ULONG  onudevidx)
 {
 	  int  i;
         for(i=0;i<5;i++)
		{
			if((auto_load_onuctrl[i].onuDevIdx==onudevidx)&&(auto_load_onuctrl[i].status==AUTO_LOAD_STATE_USING))
			{
                      sys_console_printf("\r\n in test :onu %d is  doing autoconfig,please waitting a lot!!\r\n",onudevidx );
			 return  VOS_ERROR;
			}
		}
           return  VOS_OK;
 }*/

BOOL autoload_config_onulist_check(ULONG onuDevIdx)
{
	BOOL ret = FALSE;
	SHORT slot, port;
	SHORT OnuIdx, PonPortIdx;
	auto_load_onu_list_t *pOnuList;

	slot  = GET_PONSLOT(onuDevIdx);
	port = GET_PONPORT(onuDevIdx);
	PonPortIdx = GetPonPortIdxBySlot(slot, port);
	OnuIdx = GET_ONUID(onuDevIdx)-1;
	CHECK_ONU_RANGE;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
	if( (pOnuList->onu_cfg_status == AUTO_LOAD_STATE_WAIT) || (pOnuList->onu_cfg_status == AUTO_LOAD_STATE_FINISHED) ||
		(pOnuList->onu_cfg_status==AUTO_LOAD_STATE_PROCESSING) || (pOnuList->onu_cfg_status == AUTO_LOAD_STATE_FAILURE) )
		ret = TRUE;

	VOS_SemGive( onuAutoLoadSemId );

	return ret;
}

/*begin : add by zhengyt 09-05-13*/
STATUS getAutoLoadUpgradeDeviceIndex(ULONG devIdx,ULONG *pvar)
{
	if(devIdx==OLT_DEV_ID)
	{
		*pvar=devIdx;
		return VOS_OK;
	}
	return VOS_ERROR;
}

STATUS getAutoLoadUpgradeBoardIndex(ULONG brdIdx,ULONG *pvar)
{
	if( (brdIdx > OltChassis.PonCardLast) || (brdIdx<OltChassis.PonCardFirst) )
		return VOS_ERROR;
	*pvar=brdIdx;
	return VOS_OK;
}

STATUS getAutoLoadUpgradePonPortIndex(ULONG ponIdx,ULONG *pvar)
{
	if( (ponIdx > OltChassis.PonPortNumPerCard) ||( ponIdx < 1) )
		return VOS_ERROR;
	*pvar=ponIdx;
	return VOS_OK;
}
/*end*/


STATUS getAutoLoadUpgradeOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64])
{
	int i=0;
	short int PonPortIdx;
	auto_load_onu_list_t *pOnuList;

	PonPortIdx=GetPonPortIdxBySlot(brdIdx, ponIdx);
	if(PonPortIdx==VOS_ERROR)
		return VOS_ERROR;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for(i=0;i<MAXONUPERPON;i++)
	{	/*modified by zhengyt@09-06-04*/
		pOnuList = &auto_load_onu_list[PonPortIdx][i];
		if( (pOnuList->onu_upg_status == AUTO_LOAD_STATE_WAIT) || (pOnuList->onu_upg_status==AUTO_LOAD_STATE_PROCESSING) ||
			(pOnuList->onu_upg_status == AUTO_LOAD_STATE_FINISHED) || (pOnuList->onu_upg_status == AUTO_LOAD_STATE_FAILURE) )
		{
			/*ul_count|=(1<<i);*/
			ret_value[i]=1;
		}
		else
		{
			ret_value[i]=0;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	
	return VOS_OK;
}
STATUS getAutoLoadDisplayWaitUpgradeOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64])
{
	int i=0;
	short int PonPortIdx;

	PonPortIdx=GetPonPortIdxBySlot(brdIdx, ponIdx);
	if(PonPortIdx==VOS_ERROR)
		return VOS_ERROR;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	
	for(i=0;i<MAXONUPERPON;i++)
	{
		if(auto_load_onu_list[PonPortIdx][i].onu_upg_status==AUTO_LOAD_STATE_WAIT)
		{
			/*ul_count|=(1<<i);*/
			ret_value[i]=1;
		}
		else
		{
			ret_value[i]=0;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}
STATUS getAutoLoadDisplayUpgradingOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64])
{
	int i=0;
	short int PonPortIdx;

	PonPortIdx=GetPonPortIdxBySlot(brdIdx, ponIdx);
	if(PonPortIdx==VOS_ERROR)
		return VOS_ERROR;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for(i=0;i<MAXONUPERPON;i++)
	{
		if(auto_load_onu_list[PonPortIdx][i].onu_upg_status==AUTO_LOAD_STATE_PROCESSING)
		{
			/*ul_count|=(1<<i);*/
			ret_value[i]=1;
		}
		else
		{
			ret_value[i]=0;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}
STATUS getAutoLoadDisplayUpgradedOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64])
{
	int i=0;
	short int PonPortIdx;

	PonPortIdx=GetPonPortIdxBySlot(brdIdx, ponIdx);
	if(PonPortIdx==VOS_ERROR)
		return VOS_ERROR;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for(i=0;i<MAXONUPERPON;i++)
	{
		if(auto_load_onu_list[PonPortIdx][i].onu_upg_status==AUTO_LOAD_STATE_FINISHED)
		{
			ret_value[i]=1;
		}
		else
		{
			ret_value[i]=0;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	return VOS_OK;
}	

STATUS getAutoLoadDisplayUpgradeFailOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64])
{
	int i=0;
	short int PonPortIdx;

	PonPortIdx=GetPonPortIdxBySlot(brdIdx, ponIdx);
	if(PonPortIdx==VOS_ERROR)
		return VOS_ERROR;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for(i=0;i<MAXONUPERPON;i++)
	{
		if(auto_load_onu_list[PonPortIdx][i].onu_upg_status==4)
		{
			ret_value[i]=1;
		}
		else
		{
			ret_value[i]=0;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	return VOS_OK;
}


/*begin :add by zhengyt 09-05-13*/

STATUS getAutoLoadDisplayDeviceIndex(ULONG index,ULONG *pvar)
{
	if(index==OLT_DEV_ID)
	{
		*pvar=index;
		return VOS_OK;
	}
	return VOS_ERROR;
}

STATUS getAutoLoadDisplayBoardIndex(ULONG index,ULONG *pvar)
{
	if(index>OltChassis.PonCardLast||index<OltChassis.PonCardFirst)
		return VOS_ERROR;
	*pvar=index;
	return VOS_OK;
}

STATUS getAutoLoadDisplayPonPortIndex(ULONG index,ULONG *pvar)
{
	if(index>OltChassis.PonPortNumPerCard||index<1)
		return VOS_ERROR;
	*pvar=index;
	return VOS_OK;
}

/*end*/
STATUS getAutoLoadDisplayWaitConfigOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64])
{
	int i=0;
	short int PonPortIdx;

	PonPortIdx=GetPonPortIdxBySlot(brdIdx, ponIdx);
	if(PonPortIdx==VOS_ERROR)
		return VOS_ERROR;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for(i=0;i<MAXONUPERPON;i++)
	{
		if(auto_load_onu_list[PonPortIdx][i].onu_cfg_status==AUTO_LOAD_STATE_WAIT)
		{
			ret_value[i]=1;
		}
		else
		{
			ret_value[i]=0;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	return VOS_OK;
}
STATUS getAutoLoadDisplayConfigingOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64])
{
	int i=0;
	short int PonPortIdx;

	PonPortIdx=GetPonPortIdxBySlot(brdIdx, ponIdx);
	if(PonPortIdx==VOS_ERROR)
		return VOS_ERROR;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for(i=0;i<MAXONUPERPON;i++)
	{
		if(auto_load_onu_list[PonPortIdx][i].onu_cfg_status==AUTO_LOAD_STATE_PROCESSING)
		{
			ret_value[i]=1;
		}
		else
		{
			ret_value[i]=0;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	return VOS_OK;
}
STATUS getAutoLoadDisplayConfigedOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64])
{
	int i=0;
	short int PonPortIdx;

	PonPortIdx=GetPonPortIdxBySlot(brdIdx, ponIdx);
	if(PonPortIdx==VOS_ERROR)
		return VOS_ERROR;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for(i=0;i<MAXONUPERPON;i++)
	{
		if(auto_load_onu_list[PonPortIdx][i].onu_cfg_status==AUTO_LOAD_STATE_FINISHED)
		{
			ret_value[i]=1;
		}
		else
		{
			ret_value[i]=0;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	return VOS_OK;
}

STATUS getAutoLoadDisplayConfigFailOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64])
{
	int i=0;
	short int PonPortIdx;

	PonPortIdx=GetPonPortIdxBySlot(brdIdx, ponIdx);
	if(PonPortIdx==VOS_ERROR)
		return VOS_ERROR;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for(i=0;i<MAXONUPERPON;i++)
	{
		if(auto_load_onu_list[PonPortIdx][i].onu_cfg_status==4)
		{
			ret_value[i]=1;
		}
		else
		{
			ret_value[i]=0;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );
	return VOS_OK;
}

STATUS setAutoLoadConfigDirection(ULONG onuDevIdx, ULONG set_value)
{
	STATUS result_msg=VOS_ERROR;
	short int slot, port;
	short int PonPortIdx, OnuIdx;

	if( onuDevIdx == 0 )
		return result_msg;

	slot  = GET_PONSLOT(onuDevIdx);
	port = GET_PONPORT(onuDevIdx);
	PonPortIdx = GetPonPortIdxBySlot(slot, port);
	OnuIdx = GET_ONUID(onuDevIdx)-1;
	CHECK_ONU_RANGE;

	if(set_value==AUTO_LOAD_DIR_SERVER2ONU||set_value==AUTO_LOAD_DIR_ONU2SERVER)
	{
		VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
		auto_load_onu_list[PonPortIdx][OnuIdx].onu_cfg_ftp_direct = set_value;
		VOS_SemGive( onuAutoLoadSemId );
		result_msg=VOS_OK;
	}
	return result_msg;
}
STATUS setAutoLoadUpgradeOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR set_value[64])
{
	int i=0;
	short int PonPortIdx;

	PonPortIdx = GetPonPortIdxBySlot(brdIdx, ponIdx);
	if(PonPortIdx == VOS_ERROR)
		return VOS_ERROR;
	for( i=0; i<MAXONUPERPON; i++)
	{
		VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
		if( (set_value[i] != '0') && (set_value[i] != 0) )
		{
			auto_load_onu_list[PonPortIdx][i].onu_upg_status = AUTO_LOAD_STATE_WAIT;
			auto_load_onu_list[PonPortIdx][i].onu_upg_ftp_direct = AUTO_LOAD_DIR_SERVER2ONU;
		}
		else /*add by zhengyt 09-05-31*/
		{
			auto_load_onu_list[PonPortIdx][i].onu_upg_status = AUTO_LOAD_STATE_IDLE;
		}
		VOS_SemGive( onuAutoLoadSemId );
	}
	return VOS_OK;
}

STATUS getAutoLoadUserIpGateway(ULONG idxs,UCHAR *onu_cfg_ipgateway, UCHAR *ul_length)
{
	int i=idxs-1;
	if(i>AUTO_LOAD_FTP_CLIENT_IPNUM)
		return VOS_ERROR;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );

	if(VOS_StrLen(auto_load_onuctrl[i].ftpClient.ipGateway) != 0)
	{
		VOS_StrCpy(onu_cfg_ipgateway, auto_load_onuctrl[i].ftpClient.ipGateway);
		*ul_length = VOS_StrLen(auto_load_onuctrl[i].ftpClient.ipGateway);
	}
	else
	{
		VOS_MemZero(onu_cfg_ipgateway, AUTO_LOAD_FTP_IP_STR_LEN);
		*ul_length = 0;
	}
	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}

STATUS getAutoLoadUserVlanid(ULONG idxs,ULONG  *cfg_vid,UCHAR *cfg_vidlen)
{
	int i=(idxs-1);
	if(idxs>AUTO_LOAD_FTP_CLIENT_IPNUM)
		return VOS_ERROR;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	if(auto_load_onuctrl[i].ftpClient.vid!=0)
	{
		*cfg_vid=auto_load_onuctrl[i].ftpClient.vid;
		*cfg_vidlen=4;
	} 
	else
	{
		*cfg_vid=0;
		*cfg_vidlen=4;
	}	
	VOS_SemGive( onuAutoLoadSemId );
	return VOS_OK;
}

STATUS getAutoLoaduserIpStatus(ULONG  idxs,ULONG *cfg_rowstatus,UCHAR*cfg_rowstatuslen)
{
	int i=(idxs-1);
	if(idxs>AUTO_LOAD_FTP_CLIENT_IPNUM)
		return VOS_ERROR;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	if(auto_load_onuctrl[i].rowStatus==AUTO_LOAD_STATE_USING)
	{
		*cfg_rowstatus=(auto_load_onuctrl[i].status+1);
		*cfg_rowstatuslen=sizeof(ULONG);
	}
	else
	{
		*cfg_rowstatus=1;
		*cfg_rowstatuslen=4;
	}
	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}
STATUS getAutoLoadRowStatus(ULONG  idxs,ULONG *cfg_rowstatus,UCHAR*cfg_rowstatuslen)
{
	idxs =idxs-1;
	if(idxs>AUTO_LOAD_FTP_CLIENT_IPNUM)
		return VOS_ERROR;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	if(auto_load_onuctrl[idxs].rowStatus!=AUTO_LOAD_STATE_USING)
	{
		*cfg_rowstatus=3;
		*cfg_rowstatuslen=sizeof(ULONG);
	}
	else
	{
		if((VOS_StrLen(auto_load_onuctrl[idxs].ftpClient.ipMask)!=0)&&(VOS_StrLen
			(auto_load_onuctrl[idxs].ftpClient.ipGateway)!=0)&&(auto_load_onuctrl[idxs].ftpClient.vid!=0))
		{
			*cfg_rowstatus=1;
			*cfg_rowstatuslen=sizeof(ULONG);
		}
		else
		{
			*cfg_rowstatus=3;
			*cfg_rowstatuslen=sizeof(ULONG);
		}
	}
	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}
STATUS getAutoLoadOnuDevIdx( ULONG idxs, ULONG *cfg_onuidx, UCHAR *cfg_onuidxlen )
{
	int i=(idxs-1);
	
	if(idxs>AUTO_LOAD_FTP_CLIENT_IPNUM)
		return VOS_ERROR;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	for( ; i<AUTO_LOAD_FTP_CLIENT_IPNUM; i++ )
	{
		if(auto_load_onuctrl[i].status==AUTO_LOAD_STATE_USING)
		{
			*cfg_onuidx=auto_load_onuctrl[i].onuDevIdx;
			*cfg_onuidxlen=4;
			/*auto_ftp_debug ("\r\nget onu cfg onuidx=%d",(*cfg_onuidx));
			auto_ftp_debug ("\r\nget onu cfg onuidxlen=%d",(*cfg_onuidxlen));*/
			break;
		}
		else
		{
			*cfg_onuidx=0;
			*cfg_onuidxlen=4;
			break;
		}
	}
	VOS_SemGive( onuAutoLoadSemId );

	if( i >= AUTO_LOAD_FTP_CLIENT_IPNUM )
		return VOS_ERROR;
	return VOS_OK;
}

STATUS getAutoLoadUserIpAddr(ULONG idxs, UCHAR *onu_cfg_ipadd, UCHAR *ul_length)
{
	int i=idxs-1;
	if(i>AUTO_LOAD_FTP_CLIENT_IPNUM)
		return VOS_ERROR;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	if( VOS_StrLen(auto_load_onuctrl[i].ftpClient.ipMask) != 0 )
	{
		VOS_StrCpy(onu_cfg_ipadd, auto_load_onuctrl[i].ftpClient.ipMask);
		*ul_length=VOS_StrLen(auto_load_onuctrl[i].ftpClient.ipMask);
	}
	else
	{
		VOS_MemZero(onu_cfg_ipadd, AUTO_LOAD_FTP_IPMASK_STR_LEN);
		*ul_length=0;
	}
	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}

STATUS setAutoLoadUserIpAddr(ULONG idxs,UCHAR onucfg_ipadd[32])
{
	STATUS result_msg=VOS_ERROR;
	int  i;

	idxs--;
	if( idxs >= AUTO_LOAD_FTP_CLIENT_IPNUM )
		return VOS_ERROR;

	for( i=0; i<AUTO_LOAD_FTP_CLIENT_IPNUM; i++)
	{
		if(i != idxs)
		{
			if( VOS_StrCmp(onucfg_ipadd, auto_load_onuctrl[i].ftpClient.ipMask) == 0 )
			{
				return result_msg;
			}
		}
	}

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	if(auto_load_onuctrl[idxs].rowStatus != AUTO_LOAD_STATE_USING)
	{
		VOS_MemZero( auto_load_onuctrl[idxs].ftpClient.ipMask, sizeof(auto_load_onuctrl[idxs].ftpClient.ipMask) );
		VOS_StrCpy( auto_load_onuctrl[idxs].ftpClient.ipMask, onucfg_ipadd );
		result_msg = VOS_OK;
	}
	VOS_SemGive( onuAutoLoadSemId );
		
	return result_msg;
}


STATUS setAutoLoadUserIpGateWay(ULONG idxs,UCHAR onucfg_ipgateway[32])
{
	idxs--;
	if( idxs >= AUTO_LOAD_FTP_CLIENT_IPNUM )
		return VOS_ERROR;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	VOS_MemZero(auto_load_onuctrl[idxs].ftpClient.ipGateway, sizeof(auto_load_onuctrl[idxs].ftpClient.ipGateway));
	VOS_StrCpy(auto_load_onuctrl[idxs].ftpClient.ipGateway, onucfg_ipgateway);
	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}

STATUS setAutoLoadUserVlanId(ULONG idxs,ULONG onucfg_vid)
{
	idxs--;
	if( idxs >= AUTO_LOAD_FTP_CLIENT_IPNUM )
		return VOS_ERROR;
	
	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	auto_load_onuctrl[idxs].ftpClient.vid=0;
	auto_load_onuctrl[idxs].ftpClient.vid=onucfg_vid;
	VOS_SemGive( onuAutoLoadSemId );
	return VOS_OK;
}

STATUS setAutoLoadRowStatus(UCHAR idxs,ULONG onucfg_rowstatus)
{
	STATUS  result_msg=VOS_ERROR;
	idxs--;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	if( (onucfg_rowstatus==6) )
	{
		VOS_MemZero(auto_load_onuctrl[idxs].ftpClient.ipMask, sizeof(auto_load_onuctrl[idxs].ftpClient.ipMask));
		VOS_MemZero(auto_load_onuctrl[idxs].ftpClient.ipGateway, sizeof(auto_load_onuctrl[idxs].ftpClient.ipGateway));
		auto_load_onuctrl[idxs].ftpClient.vid = 0;
		auto_load_onuctrl[idxs].status = 0;
		auto_load_onuctrl[idxs].rowStatus = AUTO_LOAD_STATE_NOT_USING;
		result_msg=VOS_OK;
	}
	else if(onucfg_rowstatus==2)
	{
		auto_load_onuctrl[idxs].rowStatus=AUTO_LOAD_STATE_NOT_USING;
		result_msg=VOS_OK;
	}
	else if(onucfg_rowstatus==1)
	{
		if((VOS_StrLen(auto_load_onuctrl[idxs].ftpClient.ipMask)) != 0 &&
			(VOS_StrLen(auto_load_onuctrl[idxs].ftpClient.ipGateway)) != 0 &&
			(auto_load_onuctrl[idxs].ftpClient.vid)!=0)
		{
			auto_load_onuctrl[idxs].rowStatus = AUTO_LOAD_STATE_USING;
			auto_load_onuctrl[idxs].status=AUTO_LOAD_STATE_NOT_USING;
			result_msg=VOS_OK;
		}
	}
	else if(onucfg_rowstatus==4||onucfg_rowstatus==5)
	{
		auto_load_onuctrl[idxs].rowStatus=AUTO_LOAD_STATE_NOT_USING;
		result_msg=VOS_OK;
	}
	VOS_SemGive( onuAutoLoadSemId );

	return result_msg;
}


STATUS setAutoLoadConfigRowStatus(ULONG onuDevIdx,ULONG set_value)
{
	STATUS result_msg = VOS_ERROR;
	short int slot, port;
	short int PonPortIdx, OnuIdx;
	auto_load_onu_list_t *pOnuList;

	if(onuDevIdx==0)
		return result_msg;

	slot  = GET_PONSLOT(onuDevIdx);
	port = GET_PONPORT(onuDevIdx);
	PonPortIdx = GetPonPortIdxBySlot(slot, port);
	OnuIdx = GET_ONUID(onuDevIdx)-1;
	CHECK_ONU_RANGE;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
	if(set_value==6)
	{
		pOnuList->onu_cfg_status = 0;
		pOnuList->onu_cfg_ftp_direct = 0;
		VOS_MemZero(pOnuList->onu_cfg_filename, 32);

		result_msg=VOS_OK;
	}
	else if(set_value==2)
	{
		result_msg=VOS_OK;
	}
	else if(set_value==1)
	{
		if(pOnuList->onu_cfg_status != AUTO_LOAD_STATE_PROCESSING)
		{	
			pOnuList->onu_cfg_status = AUTO_LOAD_STATE_WAIT;
			if( (pOnuList->onu_cfg_ftp_direct != AUTO_LOAD_DIR_SERVER2ONU) && (pOnuList->onu_cfg_ftp_direct != AUTO_LOAD_DIR_ONU2SERVER) )
				pOnuList->onu_cfg_ftp_direct = AUTO_LOAD_DIR_DEFAULT;
#if 0
			if(VOS_StrLen(auto_load_onu_list[ulPonPortidx][ulonuid].onu_cfg_filename)==0)
			{	VOS_MemZero(ulMacAddr, 8);
				GetOnuMacAddr(ulPonPortidx, ulonuid, ulMacAddr, &MacAddrLen);
				VOS_MemCpy(auto_load_onu_list[ulPonPortidx][ulonuid].onu_cfg_filename, ulMacAddr,MacAddrLen);
			}
#endif
			result_msg=VOS_OK;
		}
	}
	/*modified by zhengyt 09-06-26,解决问题单8572*/
	else if((set_value==4) || (set_value==5) )
	{	
		pOnuList->onu_cfg_status=AUTO_LOAD_STATE_WAIT;
		pOnuList->onu_cfg_ftp_direct = AUTO_LOAD_DIR_DEFAULT;
		result_msg=VOS_OK;
	}
	VOS_SemGive( onuAutoLoadSemId );
	
	return result_msg;
}

STATUS getAutoLoadConfigRowStatus(ULONG onuDevIdx, ULONG *cfg_rowStatus)
{
	STATUS result_msg=VOS_ERROR;
	short int slot, port;
	short int PonPortIdx, OnuIdx;
	auto_load_onu_list_t *pOnuList;

	if(onuDevIdx==0)
		return result_msg;

	slot  = GET_PONSLOT(onuDevIdx);
	port = GET_PONPORT(onuDevIdx);
	PonPortIdx = GetPonPortIdxBySlot(slot, port);
	OnuIdx = GET_ONUID(onuDevIdx)-1;
	CHECK_ONU_RANGE;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
	if( (pOnuList->onu_cfg_status == AUTO_LOAD_STATE_WAIT) || (pOnuList->onu_cfg_status == AUTO_LOAD_STATE_PROCESSING) ||
		(pOnuList->onu_cfg_status == AUTO_LOAD_STATE_FINISHED) || (pOnuList->onu_cfg_status == AUTO_LOAD_STATE_FAILURE) )
	{
		if( pOnuList->onu_cfg_ftp_direct != 0 )
		{
			*cfg_rowStatus=1;
			result_msg=VOS_OK;
		}
		else
		{
			*cfg_rowStatus=3;
			result_msg=VOS_OK;
		}
	}
	else
	{
		*cfg_rowStatus=3;
		result_msg=VOS_OK;
	}
	VOS_SemGive( onuAutoLoadSemId );

	return result_msg;
}


STATUS getAutoLoadAgingTime(ULONG* ul_result)
{
	*ul_result = autoLoad_aging_times;
	return VOS_OK;
}

STATUS setAutoLoadAgingTime(ULONG setVal)
{
	if(setVal>10||setVal<1)
		return VOS_ERROR;
	autoLoad_aging_times=setVal;
	return VOS_OK;
}

LONG  onuAutoLoadShowRun( struct vty * vty )
{
	ULONG j,k;
	char *pFileType;
	
	vty_out( vty, "!Onu autoconfig and upgrade config\r\n" );

	/*if( autoLoad_upg_enable == 1|| autoLoad_cfg_enable==1)*/		/*问题单8595*/
	
		/* modified by xieshl 	20080807, IP地址默认值不能为空 */
	if( (VOS_StrLen(auto_load_ftpserver.serverIp) >= 7) && (VOS_StrCmp(auto_load_ftpserver.serverIp, "0000000000") != 0) )
	{
		vty_out( vty, " auto-load ftpserver %s %s %s", auto_load_ftpserver.serverIp, auto_load_ftpserver.userName, auto_load_ftpserver.password);

		if( (auto_load_ftpserver.port != AUTO_LOAD_FTP_CTRL_PORT) && (auto_load_ftpserver.port != 0) )	/* 问题单11284 */
			vty_out( vty, " ftpport %d\r\n", auto_load_ftpserver.port);
		else
			vty_out( vty, "\r\n");
	}
	
	for( j=0; j<5; j++ )
	{
		if(auto_load_onuctrl[j].rowStatus == AUTO_LOAD_STATE_USING)
		vty_out( vty, " auto-load client-ip %s gateway %s vid %d\r\n", auto_load_onuctrl[j].ftpClient.ipMask, auto_load_onuctrl[j].ftpClient.ipGateway, auto_load_onuctrl[j].ftpClient.vid);
	}

	for( j=0; j<AUTO_LOAD_FILETYPE_COUNT; j++ )
	{
		for(k=0; k<V2R1_ONU_MAX; k++)
		{
			pFileType = autoload_filetype_to_str( auto_load_filetype_list[k].file_type[j].filetype );
			if( NULL == pFileType )
				continue;
			vty_out( vty," auto-load upgrade-file %s %s %s\r\n",GetDeviceTypeString(k), pFileType, auto_load_filetype_list[k].file_type[j].ver );
		}	
	}
	/*end 20090422*/
	vty_out( vty, "!\r\n\r\n" );

	return  VOS_OK;
}

/* added by xieshl 20110829, 独立定义showrun模块，问题单12918 */
LONG autoload_module_init()
{
    struct cl_cmd_module * autoload_module = NULL;

    autoload_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_ONU_AUTO_LOAD);
    if ( !autoload_module )
    {
        ASSERT( 0 );
        return VOS_ERROR;
    }

    VOS_MemZero( ( char * ) autoload_module, sizeof( struct cl_cmd_module ) );

    autoload_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_ONU_AUTO_LOAD );
    if ( !autoload_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( autoload_module );
        return VOS_ERROR;
    }
    VOS_StrCpy( autoload_module->module_name, "auto-load" );

    autoload_module->init_func = onuAutoLoadCommandInstall;
    autoload_module->showrun_func = onuAutoLoadShowRun;
    autoload_module->next = NULL;
    autoload_module->prev = NULL;
    cl_install_module( autoload_module );

    return VOS_OK;
}


STATUS testAutoLoadState(ULONG onuDevIdx,ULONG value)
{
	STATUS result_msg = VOS_ERROR;
	short int slot, port;
	short int PonPortIdx, OnuIdx;
	auto_load_onu_list_t *pOnuList;

	if(onuDevIdx==0)
		return result_msg;

	slot  = GET_PONSLOT(onuDevIdx);
	port = GET_PONPORT(onuDevIdx);
	PonPortIdx = GetPonPortIdxBySlot(slot, port);
	OnuIdx = GET_ONUID(onuDevIdx)-1;
	CHECK_ONU_RANGE;

	VOS_SemTake( onuAutoLoadSemId, WAIT_FOREVER );
	pOnuList = &auto_load_onu_list[PonPortIdx][OnuIdx];
	pOnuList->onu_upg_status = value;
	pOnuList->onu_upg_ftp_direct = AUTO_LOAD_DIR_DEFAULT;
	VOS_SemGive( onuAutoLoadSemId );

	return VOS_OK;
}


#endif	/*EPON_MODULE_ONU_AUTO_LOAD*/

#ifdef	__cplusplus
}
#endif/* __cplusplus */

