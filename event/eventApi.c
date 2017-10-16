#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "CmcGeneral.h"
#include  "V2R1_product.h"

/*#include "Ifm_pub.h"*/
/*#include "cli/cli.h"
#include "sys/main/sys_main.h"*/

#include "lib_gwEponMib.h"
#include "gwEponSys.h"

#include "eventMain.h"
#include "eventOam.h"
#include "eventTrap.h"
#include "ethLoopChk.h"
#include "onu/ExtBoardType.h"
#include "backup/syncMain.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "E1_MIB.h"
#include "Tdm_apis.h"
#include "Tdm_comm.h"
#endif
#include "Cdp_pub.h"
#include "../ct_manage/CT_Onu_event.h"

extern VOS_HANDLE eventTaskId;
extern LONG devsm_sys_is_switchhovering();
extern int eventReportMsgSend( eventMsg_t *pAlmMsg );

static int private_portEvent_report( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG alarmId );
static int private_ponMonEvent_report( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, 
		long monValue, ULONG alarmId );
/*----------------------------------------------------------------------------*/

/* 私有MIB定义trap事件，设备相关事件上报 */
static int private_devEvent_report_ext( ULONG devIdx, ULONG alarmId, ULONG data )
{	
	eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = alarmId;
	almMsg.alarmSrc.devAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.devAlarmSrc.devData = data;

	return eventReportMsgSend( &almMsg );
}
static int private_devEvent_report( ULONG devIdx, ULONG alarmId )
{	
	/*eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = alarmId;
	almMsg.alarmSrc.devAlarmSrc.devIdx = devIdx;

	return eventReportMsgSend( &almMsg );*/
	return private_devEvent_report_ext( devIdx, alarmId, 0 );
}

/*----------------------------------------------------------------------------*/

/* 功能:    新ONU注册事件上报，即在ONU第一次注册，并完成ONU设备信息查询后，通知
			告警任务，特别注意:	trap中需要绑定3个对象deviceIndex, deviceType, 
			deviceEntLogicalCommunity，因此，必须确保处理该事件时ONU类型和设备
			索引在MIB中已赋值
   输入参数:devIdx－新注册ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int onuNewRegSuccess_EventReport( ULONG devIdx )
{
	return private_devEvent_report( devIdx, trap_onuNewRegSuccess );
}

/* 功能:    ONU重新注册事件上报，即在ONU重新注册，并完成ONU设备信息查询后，通知
			告警任务，特别注意:	trap中需要绑定3个对象deviceIndex, deviceType, 
			deviceEntLogicalCommunity，因此，必须确保处理该事件时ONU类型和设备
			索引在MIB中已赋值
   输入参数:devIdx－注册ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int onuReregSuccess_EventReport( ULONG devIdx )
{
	return private_devEvent_report( devIdx, trap_onuReregSuccess );
}

/* 功能:    ONU离线注册事件上报
   输入参数:devIdx－离线ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int onuNotPresent_EventReport( ULONG devIdx, ULONG reason )
{
#if 1
	return private_devEvent_report_ext( devIdx, trap_onuNotPresent, reason );
#else
	eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_onuNotPresent;
	almMsg.alarmSrc.devAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.devAlarmSrc.devData = reason;

	return eventReportMsgSend( &almMsg );
#endif
}
/*----------------------------------------------------------------------------*/

/* 功能:    设备掉电事件上报，如果是OLT掉电，则需要特殊处理，不再上报告警任务
   输入参数:devIdx－掉电(ONU/OLT)设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int devPowerOff_EventReport( ULONG devIdx )	
{
	int rc = VOS_ERROR;
	
	if( devIdx != OLT_DEV_ID )
	{
		rc = private_devEvent_report( devIdx, trap_devPowerOff );
	}
	else
	{
#ifdef OLT_PWU_OFF_TRAP
		VOS_TaskSetPriority(eventTaskId, 0);
		rc = private_devEvent_report( devIdx, trap_devPowerOff );
#else
		alarmSrc_t alarmSrc;
		alarmSrc.devAlarmSrc.devIdx = devIdx;

		rc = saveEventLog( alarmType_private, trap_devPowerOff, &alarmSrc );	/*add by shixh20090420*/

		sendOltPowerOffTrap( devIdx );
#endif
	}
	return rc;
}

/* 功能:    设备掉电恢复事件上报，主要针对ONU设备，OLT掉电重启时不上报，该事件
			和ONU重新注册触发条件相同，根据记录的ONU状态确定是掉电还是离线
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int devPowerOn_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_devPowerOn );
}
/*----------------------------------------------------------------------------*/

/* 功能:    OLT配置数据保存flash成功事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int cfgDataSaveSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cfgDataSaveSuccess );
}

/* 功能:    OLT配置数据保存flash失败事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int cfgDataSaveFail_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cfgDataSaveFail );
}

/* 功能:    OLT配置数据从flash擦除成功事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int flashClearSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_flashClearSuccess );
}

/* 功能:    OLT配置数据从flash擦除失败事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int flashClearFail_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_flashClearFail );
}

/* 功能:    通过FTP上传OLT/ONU软件到OLT flash成功事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int softwareUpdateSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_softwareUpdateSuccess );
}

/* 功能:    通过FTP上传OLT/ONU软件到OLT flash失败事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int softwareUpdateFail_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_softwareUpdateFail );
}

/* 功能:    通过FTP上传OLT/ONU固件到OLT flash成功事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int firmwareUpdateSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_firmwareUpdateSuccess );
}

/* 功能:    通过FTP上传OLT/ONU固件到OLT flash失败事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int firmwareUpdateFail_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_firmwareUpdateFail );
}

/* 功能:    通过FTP下载OLT/ONU配置数据到FTP server成功事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int cfgDataBackupSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cfgDataBackupSuccess );
}

/* 功能:    通过FTP下载OLT/ONU配置数据到FTP server失败事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int cfgDataBackupFail_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cfgDataBackupFail );
}

/* 功能:    通过FTP上传OLT/ONU配置数据到OLT flash成功事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int cfgDataRestoreSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cfgDataRestoreSuccess );
}

/* 功能:    通过FTP上传OLT/ONU配置数据到OLT flash失败事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int cfgDataRestoreFail_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cfgDataRestoreFail );
}

/* 功能:    CPU占用率过高事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int cpuUsageFactorHigh_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cpuUsageFactorHigh );
}




int ctcOnuEquipmentAlarm_EventReport( ULONG devIdx )
{
        return private_devEvent_report(devIdx,trap_ctcOnuEquipmentAlarm);
}
int ctcOnuEquipmentAlarmClear_EventReport( ULONG devIdx )
{
    return private_devEvent_report(devIdx,trap_ctcOnuEquipmentAlarmClear);
}

#if 0   /*已经有*/
int ctcOnuPowerAlarm_EventReport( ULONG devIdx )
{
    return VOS_OK;
}
int ctcOnuPowerAlarmClear_EventReport( ULONG devIdx )
{
    return VOS_OK;
}
#endif
/* 功能:    CTC ONU 电池无法发现事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ctcOnuBatteryMissing_EventReport( ULONG devIdx )
{
        return private_devEvent_report(devIdx,trap_ctcOnuBatteryMissing);
}
int ctcOnuBatteryMissingClear_EventReport( ULONG devIdx )
{
    return private_devEvent_report(devIdx,trap_ctcOnuBatteryMissingClear);
}

/* 功能:    CTC ONU 电池不能在充电事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ctcOnuBatteryFailure_EventReport( ULONG devIdx )
{
        return private_devEvent_report(devIdx,trap_ctcOnuBatteryFailure);
}
int ctcOnuBatteryFailureClear_EventReport( ULONG devIdx )
{
    return private_devEvent_report(devIdx,trap_ctcOnuBatteryFailureClear);
}

/* 功能:    CTC ONU 电池电压低事件上报
   输入参数:devIdx－ONU设备索引threshold-电压的实测值
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ctcOnuBatteryVoltLow_EventReport( ULONG devIdx,ULONG  threshold)
{
#if 0
        return private_devEvent_report(devIdx,trap_ctcOnuBatteryVoltLow);
#else
        eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_ctcOnuBatteryVoltLow;
	almMsg.alarmSrc.devAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.devAlarmSrc.devData = threshold;

	return eventReportMsgSend( &almMsg );
#endif
}
int ctcOnuBatteryVoltLowClear_EventReport( ULONG devIdx ,ULONG  threshold)
{
        eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_ctcOnuBatteryVoltLow;
	almMsg.alarmSrc.devAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.devAlarmSrc.devData = threshold;

	return eventReportMsgSend( &almMsg );
}

int ctcOnuTempLow_EventReport( ULONG devIdx ,ULONG value)
{
        eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_ctcOnuTemperatureHigh;
	almMsg.alarmSrc.devAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.devAlarmSrc.devData = value;

	return eventReportMsgSend( &almMsg );
}
int ctcOnuTempLowClear_EventReport( ULONG devIdx ,ULONG value)
{
       eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_ctcOnuTemperatureHighClear;
	almMsg.alarmSrc.devAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.devAlarmSrc.devData = value;

	return eventReportMsgSend( &almMsg );
}

int ctcOnuTempHigh_EventReport( ULONG devIdx ,ULONG value)
{
        eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_ctcOnuTemperatureLow;
	almMsg.alarmSrc.devAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.devAlarmSrc.devData = value;

	return eventReportMsgSend( &almMsg );
}
int ctcOnuTempHighClear_EventReport( ULONG devIdx ,ULONG value)
{
        eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_ctcOnuTemperatureLowClear;
	almMsg.alarmSrc.devAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.devAlarmSrc.devData = value;

	return eventReportMsgSend( &almMsg );
}

/* 功能:    CTC ONU 有非法侵入告警事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ctcOnuPhysicalIntrusionAlarm_EventReport( ULONG devIdx )
{
        return private_devEvent_report(devIdx,trap_ctcOnuPhysicalIntrusionAlarm);
}
int ctcOnuPhysicalIntrusionAlarmClear_EventReport( ULONG devIdx )
{
    return private_devEvent_report(devIdx,trap_ctcOnuPhysicalIntrusionAlarmClear);
}

/* 功能:    CTC ONU 自测失败事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ctcOnuSelfTestFailure_EventReport( ULONG devIdx )
{
        return private_devEvent_report(devIdx,trap_ctcOnuSelfTestFailure);
}
int ctcOnuSelfTestFailureClear_EventReport( ULONG devIdx )
{
    return private_devEvent_report(devIdx,trap_ctcOnuSelfTestFailureClear);
}

/* 功能:    CTC ONU 自测失败事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ctcOnuIADConnectionFailure_EventReport( ULONG devIdx )
{
        return private_devEvent_report(devIdx,trap_ctcOnuIADConnectionFailure);
}
int ctcOnuIADConnectionFailureClear_EventReport( ULONG devIdx )
{
     return private_devEvent_report(devIdx,trap_ctcOnuIADConnectionFailureClear);
}
/* 功能:    CTC ONU 的PON端口切换事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ctcOnuPonIfSwitch_EventReport( ULONG devIdx )
{
       return private_devEvent_report(devIdx,trap_ctcOnuPonIfSwitch);
}
int ctcOnuPonIfSwitchClear_EventReport( ULONG devIdx )
{
    return private_devEvent_report(devIdx,trap_ctcOnuPonIfSwitchClear);
}



/* 功能:    CTC ONU 的ETH 端口自协商失败事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ethAutoNegFailure_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
        return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethAutoNegFailure);
}
int ethAutoNegFailureClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
     return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethAutoNegFailureClear);
}

/* 功能:    CTC ONU 的ETH 端口信号丢失事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ethLos_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
       return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethLos);
}
int ethLosClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{                    
    return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethLosCLear);
}


/* 功能:    CTC ONU 的ETH 端口失败告警事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ethFailure_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
        return  private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethFailure);
}
int ethFailureClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
     return  private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethFailureClear);
}
#if 0
/* 功能:    CTC ONU 的ETH 端口环路事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ethloopback_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
        return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethloopback);
}
int ethloopbackClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
   return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethloopbackCLear);
}
#endif
/* 功能:    CTC ONU 的ETH 端口冲突事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ethCongestion_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
        return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethCongestion);
}
int ethCongestionClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
    return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethCongestionClear);
}

/*test end*/


/* modified by xieshl 20080630, 增加板卡类型 */
/* 私有MIB定义trap事件，单板相关事件上报 */
static int private_brdEvent_report( ULONG devIdx, ULONG brdIdx, ULONG alarmId, ULONG brdType )
{	
	eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = alarmId;
	almMsg.alarmSrc.brdAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.brdAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.brdAlarmSrc.brdType = brdType;

	return eventReportMsgSend( &almMsg );
}

#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
/* begin: added by jianght 20090519 */
/* 功能:    电源板卡掉电事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int PWUPowerOff_EventReport( ULONG devIdx, ULONG brdIdx, ULONG brdType )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_powerOffAlarm, brdType );
}

/* 功能:    电源板卡上电事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int PWUPowerOn_EventReport( ULONG devIdx, ULONG brdIdx, ULONG brdType )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_powerOnAlarm, brdType );
}
/* end: added by jianght 20090519 */
#endif


/* BEGIN: added  by @muqw  2017-04-26*/
/* 功能:    电源异常状态上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int PWUStatusAbnormal_EventReport( ULONG devIdx, ULONG brdIdx, ULONG AlarmId)	
{
	return private_ponMonEvent_report(devIdx, brdIdx, 0, AlarmId, trap_pwuStatusAbnoarmal);
}

int PWUStatusAbnormalClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG AlarmId)	
{
	return private_ponMonEvent_report(devIdx, brdIdx, 0, AlarmId, trap_pwuStatusAbnoarmalClear);
}
/* END: added by @muqw  2017-04-26*/


/*add byshixh@20080831*/
int cpuUsageHigh_EventReport( ULONG devIdx ,ULONG brdIdx)
{
	return private_brdEvent_report( devIdx, brdIdx, trap_boardCpuUsageAlarm, 0 );
}

int cpuUsageHighClear_EventReport( ULONG devIdx ,ULONG brdIdx)
{
	return private_brdEvent_report( devIdx, brdIdx, trap_boardCpuUsageAlarmClear, 0 );
}

int boardMemoryUsageHigh_EventReport( ULONG devIdx ,ULONG brdIdx)
{
	return private_brdEvent_report( devIdx, brdIdx, trap_boardMemoryUsageAlarm, 0 );
}

int boardMemoryUsageHighClear_EventReport( ULONG devIdx ,ULONG brdIdx)
{
	return private_brdEvent_report( devIdx, brdIdx, trap_boardMemoryUsageAlarmClear, 0 );
}

/* 功能:    通过FTP上传PON DBA到OLT flash成功事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int dbaUpdateSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_dbaUpdateSuccess );
}

/* 功能:    通过FTP上传PON DBA到OLT flash失败事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int dbaUpdateFailure_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_dbaUpdateFailure );
}

/* 功能:    通过OAM文件加载通道传输软件到ONU成功事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int onuSoftwareLoadSuccess_EventReport( ULONG devIdx )	
{    
    ULONG status = 0;
	if(devIdx != OLT_DEV_ID)
	{
    	if(VOS_OK == GetOnuUpdatedAlarmStatus(devIdx, &status) && status)
        	return VOS_OK;
    	SetOnuUpdatedAlarmStatus(devIdx, 1);
	}
	
	return private_devEvent_report( devIdx, trap_onuSoftwareLoadSuccess );
}

/* 功能:    通过OAM文件加载通道传输软件到ONU失败事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int onuSoftwareLoadFailure_EventReport( ULONG devIdx )	
{
    ULONG status = 0;
	if(devIdx != OLT_DEV_ID)
	{
    	if(VOS_OK == GetOnuUpdatedAlarmStatus(devIdx, &status) && status)
        	return VOS_OK;
    	SetOnuUpdatedAlarmStatus(devIdx, 1);
	}
	return private_devEvent_report( devIdx, trap_onuSoftwareLoadFailure );
}


int bootUpdateSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_bootUpdateSuccess );
}

int bootUpdateFailure_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_bootUpdateFailure );
}

int batfileBackupSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_batfileBackupSuccess );
}

int batfileBackupFailure_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_batfileBackupFailure );
}

int batfileRestoreSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_batfileRestoreSuccess );
}

int batfileRestoreFailure_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_batfileRestoreFailure );
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/* 功能:    主备SW板倒换事件上报
   输入参数:devIdx－OLT设备索引，设备索引定义规则遵循MIB定义
   			brdIdx－主备倒换后的主控槽位号
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int swBoardProtectedSwitch_EventReport( ULONG devIdx, ULONG brdIdx )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_swBoardProtectedSwitch, 0 );
}

/* 功能:    单板温度过高事件上报
   输入参数:devIdx－OLT/ONU设备索引，设备索引定义规则遵循MIB定义，下同
   			brdIdx－单板(槽位号)索引，范围1~11，下同
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int boardTemperatureHigh_EventReport( ULONG devIdx, ULONG brdIdx, LONG tempVal )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, 0, tempVal, trap_boardTemperatureHigh );
}

/* 功能:    单板温度过高事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int boardTemperatureHighClear_EventReport( ULONG devIdx, ULONG brdIdx, LONG tempVal )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, 0, tempVal, trap_boardTemperatureHighClear );
}

/* 功能:    设备温度过高事件上报
   输入参数:devIdx－OLT/ONU设备索引
   		
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int deviceTemperatureHigh_EventReport( ULONG devIdx, LONG temperature, LONG threshold )
{
	ULONG data = ((temperature & 0xff) << 8) | (threshold & 0xff );
	return private_devEvent_report_ext( devIdx, trap_deviceTemperatureHigh, data );
}

/* 功能:    单板温度过高事件恢复上报
   输入参数:devIdx－OLT/ONU设备索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int deviceTemperatureHighClear_EventReport( ULONG devIdx, LONG temperature, LONG threshold )
{
	ULONG data = ((temperature & 0xff) << 8) | (threshold & 0xff );
	return private_devEvent_report_ext( devIdx, trap_deviceTemperatureHighClear, data );
}

/* 功能:    设备温度过低事件上报 
   输入参数:devIdx－OLT/ONU设备索引
   		
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int deviceTemperatureLow_EventReport( ULONG devIdx, LONG temperature, LONG threshold )
{
	ULONG data = ((temperature & 0xff) << 8) | (threshold & 0xff );
	return private_devEvent_report_ext( devIdx, trap_deviceTemperatureLow, data );
}

/* 功能:    单板温度过低事件恢复上报
   输入参数:devIdx－OLT/ONU设备索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int deviceTemperatureLowClear_EventReport( ULONG devIdx, LONG temperature, LONG threshold )
{
	ULONG data = ((temperature & 0xff) << 8) | (threshold & 0xff );
	return private_devEvent_report_ext( devIdx, trap_deviceTemperatureLowClear, data );
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/


/* 功能:    单板复位事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponBoardReset_EventReport( ULONG devIdx, ULONG brdIdx )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_ponBoardReset, 0 );
}

/* 功能:    单板带电插入事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int devBoardInterted_EventReport( ULONG devIdx, ULONG brdIdx, ULONG brdType )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_devBoardInterted, brdType );
}

/* 功能:    单板带电拔出事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int devBoardPull_EventReport( ULONG devIdx, ULONG brdIdx, ULONG brdType )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_devBoardPull, brdType );
}

char board_los_alarm_state[PRODUCT_MAX_TOTAL_SLOTNUM+1] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int devBoardLos_EventReport( ULONG devIdx, ULONG brdIdx, ULONG brdType )	
{
    if(!board_los_alarm_state[brdIdx])
    {
        board_los_alarm_state[brdIdx] = 1;    
    	return private_brdEvent_report( devIdx, brdIdx, trap_boardLosAlarm, brdType );
	}

    return VOS_OK;
}

/* 功能:    单板带电拔出事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int devBoardLosClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG brdType )	
{
    if(board_los_alarm_state[brdIdx])
    {
        board_los_alarm_state[brdIdx] = 0;
    	return private_brdEvent_report( devIdx, brdIdx, trap_boardLosAlarmClear, brdType );
	}
	return VOS_OK;
}
/* 功能:    电源板停止供电事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int powerOffAlarm_EventReport( ULONG devIdx, ULONG brdIdx )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_powerOffAlarm, 0 );
}

/* 功能:    电源板恢复供电事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int powerOnAlarm_EventReport( ULONG devIdx, ULONG brdIdx )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_powerOnAlarm, 0 );
}

/* 功能:    风扇异常事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			fanIdx－风扇索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int devFanAlarm_EventReport( ULONG devIdx, ULONG fan_id )	
{
	ULONG slotno = FANID2SLOTNO(fan_id);	/* modified by xieshl 20121211, 问题单16144 */
	ULONG portno = FANID2FANNO(fan_id);
	return private_portEvent_report( devIdx, slotno, portno, trap_devFanAlarm );
}

/* 功能:    风扇异常恢复事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			fanIdx－风扇索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int devFanAlarmClear_EventReport( ULONG devIdx, ULONG fan_id )	
{
	ULONG slotno = FANID2SLOTNO(fan_id);
	ULONG portno = FANID2FANNO(fan_id);
	return private_portEvent_report( devIdx, slotno, portno, trap_devFanAlarmClear );
}

/*added by wangxiaoyu 2008-7-24 17:37:30*/
/* 功能:    OLT端PON功率参数超门限事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
			rtVal -- 不同的trapId决定了此值不同
   返回值:  成功－VOS_OK，错误－VOS_ERROR */

int ponPortOpticalPowerAlarm_EventReport( ULONG trapId, ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG rtVal )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, rtVal, trapId);
}
/*end*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/* 私有MIB定义trap事件，PON性能监视相关事件上报 */
static int private_ponMonEvent_report( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, 
		long monValue, ULONG alarmId )
{	
	eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = alarmId;
	almMsg.alarmSrc.monAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.monAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.monAlarmSrc.portIdx = ponIdx;
	almMsg.alarmSrc.monAlarmSrc.monValue = monValue;

	return eventReportMsgSend( &almMsg );
}

/* 功能:    PON误码率超门限事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
			ber－误码率，单位10E-6
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponPortBERAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG ber )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, ber, trap_ponPortBERAlarm );
}

/* 功能:    PON误码率超门限恢复事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
			ber－误码率，单位10E-6
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponPortBERAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG ber )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, ber, trap_ponPortBERAlarmClear );
}

/* 功能:    PON误帧率超门限事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
			fer－误帧率，单位10E-6
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponPortFERAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG fer )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, fer, trap_ponPortFERAlarm );
}

/* 功能:    PON误帧率超门限恢复事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
			fer－误帧率，单位10E-6
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponPortFERAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG fer )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, fer, trap_ponPortFERAlarmClear );
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/* 私有MIB定义trap事件，LLID相关事件上报 */
static int private_llidEvent_report( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, 
		ULONG llidIdx, ULONG alarmId )
{	
	eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = alarmId;
	almMsg.alarmSrc.llidAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.llidAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.llidAlarmSrc.portIdx = ponIdx;
	almMsg.alarmSrc.llidAlarmSrc.llidIdx = llidIdx;

	return eventReportMsgSend( &almMsg );
}


/* 功能:    LLID带宽超限或不足事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
			llidIdx－LLID索引，范围1~64
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int llidActBWExceeding_EventReport( ULONG devIdx, ULONG brdIdx, 
		ULONG ponIdx, ULONG llidIdx )	
{
	return private_llidEvent_report( devIdx, brdIdx, ponIdx, llidIdx, trap_llidActBWExceeding );
}
int llidActBWExceedingClear_EventReport( ULONG devIdx, ULONG brdIdx, 
		ULONG ponIdx, ULONG llidIdx )	
{
	return private_llidEvent_report( devIdx, brdIdx, ponIdx, llidIdx, trap_llidActBWExceedingClear );
}

/* 私有MIB定义trap事件，eth相关事件上报 *//*add by shixh20090520*/
static int private_ethSwitchloopEvent_report( ULONG devIdx, ULONG onubrdIdx, ULONG onuethIdx, 
		ULONG switchPortIdx, uchar_t  Mac[6],ULONG alarmId )
{	
	ULONG broad,ponport,onuid;
	eventMsg_t almMsg; 

	broad=GET_PONSLOT(devIdx)/*devIdx/10000*/;
	ponport=GET_PONPORT(devIdx)/*(devIdx%10000)/1000*/;
	onuid=GET_ONUID(devIdx)/*devIdx%1000*/;

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = alarmId;
	almMsg.alarmSrc.onuSwitchAlarmSrc.brdId =broad;
	almMsg.alarmSrc.onuSwitchAlarmSrc.ponId = ( ponport >= 16 ? 0 : ponport) ;
	almMsg.alarmSrc.onuSwitchAlarmSrc.onuId = onuid;
	almMsg.alarmSrc.onuSwitchAlarmSrc.onuBrdId= onubrdIdx;
	almMsg.alarmSrc.onuSwitchAlarmSrc.onuPortId = onuethIdx;
	almMsg.alarmSrc.onuSwitchAlarmSrc.reason = switchPortIdx;
	VOS_MemCpy(almMsg.alarmSrc.onuSwitchAlarmSrc.switchMacAddr, Mac,6);

	return eventReportMsgSend( &almMsg );
}

int SwitchEthPortLoop_EventReport(ULONG devIdx, ULONG onubrdIdx, ULONG onuethIdx, 
		ULONG switchPortIdx, uchar_t  Mac[6])	
{
 return  private_ethSwitchloopEvent_report( devIdx, onubrdIdx, onuethIdx, switchPortIdx, Mac,trap_SwitchEthPortLoop );
}

int SwitchEthPortLoop_EventReportClear(ULONG devIdx,  ULONG onubrdIdx, ULONG onuethIdx, 
		ULONG switchPortIdx, uchar_t  Mac[6])	
{
  return  private_ethSwitchloopEvent_report( devIdx, onubrdIdx, onuethIdx, switchPortIdx, Mac, trap_SwitchEthPortLoopClear);
}

int onuSwitchNewRegSuccess_EventReport(ulong_t devIdx, ulong_t onubrdIdx, ulong_t onuethIdx, uchar_t  pMac[6])	
{
	return  private_ethSwitchloopEvent_report( devIdx, onubrdIdx, onuethIdx, 0, pMac, trap_switchNewRegSuccess );
}

int onuSwitchReregSuccess_EventReport(ulong_t devIdx,  ulong_t onubrdIdx, ulong_t onuethIdx, uchar_t  pMac[6])	
{
	return  private_ethSwitchloopEvent_report( devIdx, onubrdIdx, onuethIdx, 0, pMac, trap_switchReregSuccess );
}

int onuSwitchNotPresent_EventReport(ulong_t devIdx,  ulong_t onubrdIdx, ulong_t onuethIdx, uchar_t  pMac[6], ulong_t reason)	
{
	return  private_ethSwitchloopEvent_report( devIdx, onubrdIdx, onuethIdx, reason, pMac, trap_switchNotPresent );
}

int onuSwitchEthSpeedExceed_EventReport(ulong_t devIdx,  ulong_t onubrdIdx, ulong_t onuethIdx, uchar_t  pMac[6], ulong_t reason, ulong_t type)
{
	ulong_t traptype = 0;

	switch(type)
	{
		case onuSwitch_ethIngressLimitExceed:
			traptype = trap_switchEthIngressLimitExceed;
			break;
		case onuSwitch_ethIngressLimitExceedClear:
			traptype = trap_switchEthIngressLimitExceedClear;
			break;
		case onuSwitch_ethEgressLimitExceed:
			traptype = trap_switchEthEgressLimitExceed;
			break;
		case onuSwitch_ethEgressLimitExceedClear:
			traptype = trap_switchEthEgressLimitExceedClear;
			break;
		default:
			traptype = 0;
			break;
	}

	if(traptype)
		return  private_ethSwitchloopEvent_report( devIdx, onubrdIdx, onuethIdx, reason, pMac, traptype );
	else
		return VOS_ERROR;
}
int onuBackupPonAlarm_EventReport(ulong_t devIdx,  ulong_t onubrdIdx, ulong_t PonPortIdx, uchar_t  pMac[6], ulong_t reason)	
{
	return  private_ethSwitchloopEvent_report( devIdx, onubrdIdx, PonPortIdx, reason, pMac, trap_backupPonAlarm );
}
int onuBackupPonAlarmClear_EventReport(ulong_t devIdx,  ulong_t onubrdIdx, ulong_t PonPortIdx, uchar_t  pMac[6], ulong_t reason)	
{
	return  private_ethSwitchloopEvent_report( devIdx, onubrdIdx, PonPortIdx, reason, pMac, trap_backupPonAlarmClear);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*b-add by zhaoxh 解决 杭州广电中标测试评估;长发光检测告警上报目前只能支持检测到PON口，未达到具体ONU*/
 int get_down_onuid(ulong_t ponIdx)
{
	int status = 0;
	int i = 0;
	
	for(; i< MAXONUPERPON ;i++)
	{
		status = GetOnuOperStatus_Ext(ponIdx, i );
        if(1 == status)
        {
        	return i+1;
        }
	}
	return VOS_ERROR;

}
/*e-zhaoxh 因测试只有两个onu所以就获取down掉的onu*/
static int private_oltLaserAlwaysOnEvent_report(  ulong_t brdIdx, ulong_t ponIdx, 
		ulong_t devidx,long dbm, ulong_t alarmId )
{	
	eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = alarmId;
	/*almMsg.alarmSrc.monAlarmSrc.devIdx = devIdx;*/
	almMsg.alarmSrc.oltRxpowerAlwaysOnAlarmSrc.brdIdx = (uchar_t)brdIdx;
	almMsg.alarmSrc.oltRxpowerAlwaysOnAlarmSrc.portIdx = (uchar_t)ponIdx;
	almMsg.alarmSrc.oltRxpowerAlwaysOnAlarmSrc.onuIdx= (ulong_t)get_down_onuid(ponIdx-1);
	almMsg.alarmSrc.oltRxpowerAlwaysOnAlarmSrc.dbm = dbm;

	return eventReportMsgSend( &almMsg );
}

/* 私有MIB定义trap事件，以太网端口相关事件上报 */
static int private_portEvent_report( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG alarmId )
{	
	eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = alarmId;
	almMsg.alarmSrc.portAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.portAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.portAlarmSrc.portIdx = portIdx;

	return eventReportMsgSend( &almMsg );
}

/*added by wangjiah@2015-05-05
 *report pon los/clear event with xfp type
 * */
static int  private_ponEvent_report_with_type(ULONG devIdx, ULONG brdIdx, ULONG portIdx, int xfpType, ULONG alarmId)
{
	eventMsg_t almMsg;
	VOS_MemZero( &almMsg, sizeof(eventMsg_t));
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = alarmId;

	almMsg.alarmSrc.portAlarmSrc.devIdx= devIdx;
	almMsg.alarmSrc.portAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.portAlarmSrc.portIdx = portIdx;
	almMsg.alarmSrc.alarmSrcData[6] = xfpType;

	return eventReportMsgSend( &almMsg );
}

/* 私有MIB定义trap事件，PON端口相关事件上报 */
#define private_ponEvent_report( devIdx, brdIdx, ponIdx, alarmId ) \
	private_portEvent_report( devIdx, brdIdx, ponIdx, alarmId )
#define private_tdmlinkEvent_report( devIdx, brdIdx, tdmIdx, alarmId ) \
	private_portEvent_report( devIdx, brdIdx, tdmIdx, alarmId )
/* 功能:    PON保护倒换事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引，
			ponIdx－PON端口索引，
			partnerDevIdx－对端OLT设备索引
   			partnerBrdIdx－对端单板(槽位号)索引，
			partnerPonIdx－对端PON端口索引，
			partnerIpAddr－跨OLT保护对端IP地址
			partnerUdpPort－跨OLT保护对端UDP端口
   返回值:  成功－VOS_OK，错误－VOS_ERROR 
   */
int ponProtectSwitch_EventReport(ULONG brdIdx, ULONG ponIdx, ULONG partnerBrdIdx, ULONG partnerPonIdx, ULONG partnerIpAddr, USHORT partnerUdpPort)
{

	eventMsg_t almMsg;
	VOS_MemZero( &almMsg, sizeof(eventMsg_t));
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_ponProtectSwitch; 
	almMsg.alarmSrc.ponSwitchAlarmSrc.brdIdx = (UCHAR)brdIdx;
	almMsg.alarmSrc.ponSwitchAlarmSrc.ponIdx = (UCHAR)ponIdx;
	almMsg.alarmSrc.ponSwitchAlarmSrc.partnerBrdIdx = (UCHAR)partnerBrdIdx;
	almMsg.alarmSrc.ponSwitchAlarmSrc.partnerPonIdx = (UCHAR)partnerPonIdx;
	VOS_MemCpy(almMsg.alarmSrc.ponSwitchAlarmSrc.ipAddrWithPort, &partnerIpAddr, sizeof(partnerIpAddr));
	VOS_MemCpy(almMsg.alarmSrc.ponSwitchAlarmSrc.ipAddrWithPort + sizeof(partnerIpAddr), &partnerUdpPort, sizeof(partnerUdpPort));
	return eventReportMsgSend(&almMsg);
}


static int private_logicalSlotEvent_report(ULONG devIdx, UCHAR brdIdx, ULONG ipAddr, ULONG udpPort, ULONG alarmId)
{
	eventMsg_t almMsg;
	VOS_MemZero(&almMsg, sizeof(eventMsg_t));
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = alarmId;
	almMsg.alarmSrc.logicalSlotAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.logicalSlotAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.logicalSlotAlarmSrc.ipAddr = ipAddr;
	almMsg.alarmSrc.logicalSlotAlarmSrc.udpPort = udpPort;

	return eventReportMsgSend(&almMsg);
}

/* 功能:	逻辑槽位插入事件上报 
   输入参数:devIdx－OLT设备索引
   			brdIdx－逻辑槽位索引，范围65~68
   			ipAddr－远端设备IP地址
			udpPort－远端设备逻辑槽位UDP端口
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int logicalSlotInsert_EventReport(ULONG devIdx, UCHAR brdIdx, ULONG ipAddr, ULONG udpPort)
{
	return private_logicalSlotEvent_report(devIdx, brdIdx, ipAddr, udpPort, trap_logicalSlotInsert);
}

/* 功能:	逻辑槽位拔出事件上报 
   输入参数:devIdx－OLT设备索引
   			brdIdx－逻辑槽位索引，范围65~68
   			ipAddr－远端设备IP地址
			udpPort－远端设备逻辑槽位UDP端口
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int logicalSlotPull_EventReport(ULONG devIdx, UCHAR brdIdx, ULONG ipAddr, ULONG udpPort)
{
	return private_logicalSlotEvent_report(devIdx, brdIdx, ipAddr, udpPort, trap_logicalSlotPull);
}

	
/* 功能:    PON保护倒换事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int autoProtectSwitch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_autoProtectSwitch );
}

/* 功能:    PON口异常事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponPortAbnormal_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, long resetCode )
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, resetCode, trap_ponPortAbnormal );
}

int ponPortAbnormalClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, 0, trap_ponPortAbnormalClear );
}


/* 功能:    PON上ONU注册冲突事件上报，一般指ONU MAC地址冲突
   输入参数:devIdx－已注册ONU设备索引，因为正在注册的ONU还没有加入到设备管理表中，
   			其设备索引还没有分配，在上报告警时只报和哪个ONU冲突
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int onuRegisterConflict_EventReport( ULONG devIdx/*, ULONG brdIdx, ULONG ponIdx*/ )	
{
	return private_devEvent_report( devIdx, trap_onuRegisterConflict );
}

/* 功能:    PON固件加载成功事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int firmwareLoadSuccess_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_firmwareLoadSuccess );
}

int firmwareLoadFailure_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_firmwareLoadFailure );
}

/* 功能:    PON DBA固件加载成功事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int dbaLoadSuccess_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_dbaLoadSuccess );
}

int dbaLoadFailure_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_dbaLoadFailure );
}

/* 功能:    以太网丢帧率超门限事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ethFlrAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethFlrAlarm );
}

/* devIndex, boardIndex, portIndex */
int ethFlrAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethFlrAlarmClear );
}

/* 功能:    以太网误帧率超门限事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ethFerAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethFerAlarm );
}

int ethFerAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethFerAlarmClear );
}

/* 功能:    以太网业务中断事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ethTranmittalIntermitAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethTranmittalIntermitAlarm );
}

/* devIndex, boardIndex, portIndex */
int ethTranmittalIntermitAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethTranmittalIntermitAlarmClear );
}

/*aded by wangxiaoyu 2008-7-24 14:05:59*/
int onuOpticalParaAlm_EventReport(USHORT paraType, ULONG almFlag,ULONG devIdx, ULONG brdIdx, ULONG portIdx, long rtVal)
{
	int trapId = 0;
	switch(paraType)
	{
		case GW_TX_POWER_HIGH_ALARM:
			trapId = (almFlag==1)?trap_ponTransmissionPowerTooHigh:trap_ponTransmissionPowerTooHighClear;
			break;
		case GW_TX_POWER_LOW_ALARM:
			trapId = (almFlag==1)?trap_ponTransmissionPowerTooLow:trap_ponTransmissionPowerTooLowClear;
			break;
		case GW_RX_POWER_HIGH_ALARM:
			trapId = (almFlag == 1)?trap_ponReceiverPowerTooHigh:trap_ponReceiverPowerTooHighClear;
			break;
		case GW_RX_POWER_LOW_ALARM:
			trapId = (almFlag == 1)?trap_ponReceiverPowerTooLow:trap_ponReceiverPowerTooLowClear;
			break;
		case GW_VCC_HIGH_ALARM:
			trapId = (almFlag == 1)?trap_ponAppliedVoltageTooHigh:trap_ponAppliedVoltageTooHighClear;
			break;
		case GW_VCC_LOW_ALARM:
			trapId = (almFlag ==1 )?trap_ponAppliedVoltageTooLow:trap_ponAppliedVoltageTooLowClear;
			break;
		case GW_TX_BIAS_HIGH_ALARM:
			trapId = (almFlag == 1)?trap_ponBiasCurrentTooHigh:trap_ponBiasCurrentTooHighClear;
			break;
		case GW_TX_BIAS_LOW_ALARM:
			trapId = (almFlag == 1)?trap_ponBiasCurrentTooLow:trap_ponBiasCurrentTooLowClear;
			break;
			/*add by shixh@20080820*/
		case GW_TEMP_HIGH_ALARM:
			trapId = (almFlag == 1)?trap_ponTemperatureTooHigh:trap_ponTemperatureTooHighClear;
			break;
		case GW_TEMP_LOW_ALARM:
			trapId = (almFlag == 1)?trap_ponTemperatureTooLow:trap_ponTemperatureTooLowClear;
			break;
		case 100:
			/* 需要特殊处理 */
			if( almFlag == 1 )
			{
			}
			return VOS_OK;
		default:
			break;
	}
	
	return private_ponMonEvent_report(devIdx, brdIdx, portIdx, rtVal, trapId);
}

int UplinkSFPRecvPowerLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long rxPower)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, rxPower, trap_uplinkReceiverPowerTooLow);
}
int UplinkSFPRecvPowerLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long rxPower)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, rxPower, trap_uplinkReceiverPowerTooLowClear);
}
int UplinkSFPRecvPowerHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long rxPower)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, rxPower, trap_uplinkReceiverPowerTooHigh);
}
int UplinkSFPRecvPowerHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long rxPower)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, rxPower, trap_uplinkReceiverPowerTooHighClear);
}

int UplinkSFPTransPowerLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long txPower)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, txPower, trap_uplinkTransmissionPowerTooLow);
}
int UplinkSFPTransPowerLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long txPower)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, txPower, trap_uplinkTransmissionPowerTooLowClear);
}
int UplinkSFPTransPowerHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long txPower)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, txPower, trap_uplinkTransmissionPowerTooHigh);
}
int UplinkSFPTransPowerHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long txPower)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, txPower, trap_uplinkTransmissionPowerTooHighClear);
}

int UplinkSFPVoltageLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long voltage)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, voltage, trap_uplinkAppliedVoltageTooLow);
}
int UplinkSFPVoltageLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long voltage)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, voltage, trap_uplinkAppliedVoltageTooLowClear);
}
int UplinkSFPVoltageHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long voltage)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, voltage, trap_uplinkAppliedVoltageTooHigh);
}
int UplinkSFPVoltageHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long voltage)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, voltage, trap_uplinkAppliedVoltageTooHighClear);
}

int UplinkSFPBiasCurrentLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long current)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, current, trap_uplinkBiasCurrentTooLow);
}
int UplinkSFPBiasCurrentLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long current)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, current, trap_uplinkBiasCurrentTooLowClear);
}
int UplinkSFPBiasCurrentHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long current)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, current, trap_uplinkBiasCurrentTooHigh);
}
int UplinkSFPBiasCurrentHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long current)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, current, trap_uplinkBiasCurrentTooHighClear);
}

int UplinkSFPTemperatureLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long temperature)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, temperature, trap_uplinkTemperatureTooLow);
}
int UplinkSFPTemperatureLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long temperature)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, temperature, trap_uplinkTemperatureTooLowClear);
}
int UplinkSFPTemperatureHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long temperature)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, temperature, trap_uplinkTemperatureTooHigh);
}
int UplinkSFPTemperatureHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long temperature)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, temperature, trap_uplinkTemperatureTooHighClear);
}

/*add by shixh20111017*/
/*eth port Dos attack*/
int ethPort_DoSAttack_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long reason)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, reason, trap_eth_DosAttack);
}
int ethPort_DoSAttackClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long reason)
{
	return private_ponMonEvent_report(devIdx, brdIdx, ethIdx, reason, trap_eth_DosAttackClear);
}
/*add by shixh@20080831*/
/* 私有MIB定义trap事件，OLT recevier性能监视相关事件上报 */
static int private_oltRxMonEvent_report(  ULONG brdIdx, ULONG ponIdx, 
		ULONG onudevidx,long oltrxPower, ULONG alarmId )
{	
	eventMsg_t almMsg;

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = alarmId;
	/*almMsg.alarmSrc.monAlarmSrc.devIdx = devIdx;*/
	almMsg.alarmSrc.oltRxpowerAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.oltRxpowerAlarmSrc.portIdx = ponIdx;
	almMsg.alarmSrc.oltRxpowerAlarmSrc.onuIdx = onudevidx;
	almMsg.alarmSrc.oltRxpowerAlarmSrc.oltrxValue = oltrxPower;

	return eventReportMsgSend( &almMsg );
}

int oltOpticalRxLow_EventReport( ULONG brdIdx, ULONG portIdx, ULONG onudevidx, long oltrxPower)
{
	return private_oltRxMonEvent_report(brdIdx,portIdx,onudevidx,oltrxPower,trap_oltPonReceiverPowerTooLow);
}

int oltOpticalRxLowClear_EventReport( ULONG brdIdx, ULONG portIdx, ULONG onudevidx, long oltrxPower)
{
	return private_oltRxMonEvent_report(brdIdx,portIdx,onudevidx,oltrxPower,trap_oltPonReceiverPowerTooLowClear);
}

int oltOpticalRxHigh_EventReport( ULONG brdIdx, ULONG portIdx, ULONG onudevidx,long oltrxPower)
{
	return private_oltRxMonEvent_report(brdIdx,portIdx,onudevidx,oltrxPower,trap_oltPonReceiverPowerTooHigh);
}

int oltOpticalRxHighClear_EventReport( ULONG brdIdx, ULONG portIdx, ULONG onudevidx, long oltrxPower)
{
	return private_oltRxMonEvent_report(brdIdx,portIdx,onudevidx,oltrxPower,trap_oltPonReceiverPowerTooHighClear);
}
/*end*/

/* 功能:    以太网端口连接linkdown事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ethLinkdown_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	/* modified by xieshl 20110705, 带外网管口不需要检查环路问题，问题单12083 */
	if( portIdx  && ( devIdx == OLT_DEV_ID ) )
	{
		if (check_eth_port_linkdown_callback( devIdx, brdIdx, portIdx ) == 1)
			return VOS_OK;
	}
	/* added 20070802 如果上联板不在运行状态，或者系统正在主备倒换过程中则不再上报 */
	if( devIdx == OLT_DEV_ID )
	{
	    #if 0 /***removed by @muqw for reporting up/down while board is rebooting or switchoverring***/
		if( (SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) || devsm_sys_is_switchhovering() )
		return VOS_OK;
        #endif
	}
    return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethLinkdown );
}

/* 功能:    以太网端口连接linkup事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ethLinkup_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	/* modified by xieshl 20110705, 带外网管口不需要检查环路问题，问题单12083 */
	if( portIdx  && ( devIdx == OLT_DEV_ID ) )
	{
		if(check_eth_port_linkup_callback( devIdx, brdIdx, portIdx )==1)	/* added by xieshl 20081020 */
			return VOS_OK;
	}
	/* added 20070802 如果上联板不在运行状态，或者系统正在主备倒换过程中则不再上报 */
	if( devIdx == OLT_DEV_ID )
	{
	    #if 0 /***removed by @muqw for reporting up/down while board is rebooting or switchoverring***/
		if( (SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) || devsm_sys_is_switchhovering() )
			return VOS_OK;
        #endif
	}
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethLinkup );
}

/*add  by shixh20090507*/
int ponPortFull_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_PonPortFullAlarm);
}

/*add  by shixh20090612*/
int onuportBroadCastFloodControl_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethPortBroadCastFloodControl);
}
int onuportBroadCastFloodControlClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethPortBroadCastFloodControlClear);
}
       
/* 私有MIB定义trap事件，非法ONU事件上报 */
int onuRegAuthFailure_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, 
		uchar_t *pOnuMacAddr )
{	
	eventMsg_t almMsg;
	
	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_onuRegAuthFailure;
	almMsg.alarmSrc.commAlarmSrc.devIdx = OLT_DEV_ID;	/*olt devIdx;*/
	almMsg.alarmSrc.commAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.commAlarmSrc.portIdx = ponIdx;
	almMsg.alarmSrc.commAlarmSrc.onuIdx = 0;
	VOS_MemCpy( almMsg.alarmSrc.commAlarmSrc.data, pOnuMacAddr, 6 );

	return eventReportMsgSend( &almMsg );
}

/* added by xieshl 20110426 */
int onuDeletingNotify_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx, ULONG onuIdx, UCHAR *pOnuMacAddr )
{	
	eventMsg_t almMsg;
	
	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_onuDeletingNotify;
	almMsg.alarmSrc.commAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.commAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.commAlarmSrc.portIdx = ponIdx;
	almMsg.alarmSrc.commAlarmSrc.onuIdx = onuIdx;
	VOS_MemCpy( almMsg.alarmSrc.commAlarmSrc.data, pOnuMacAddr, 6 );

	return eventReportMsgSend( &almMsg );
}

/* added bu xieshl 20070703 */
/*----------------------------------------------------------------------------*/
/* 功能:    设备冷启动事件上报
   输入参数:devIdx－设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int deviceColdStart_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_deviceColdStart );
}
/*----------------------------------------------------------------------------*/
/* 功能:    设备热启动事件上报
   输入参数:devIdx－设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int deviceWarmStart_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_deviceWarmStart );
}
/*----------------------------------------------------------------------------*/
/* 功能:    设备热启动事件上报
   输入参数:devIdx－设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int deviceExceptionRestart_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_deviceExceptionRestart );
}
/*----------------------------------------------------------------------------*/
/* 功能:    以太网端口连接linkdown事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponToEthLinkdown_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	ULONG val = 0;
	if(check_eth_port_linkdown_callback( devIdx, brdIdx, ponIdx )==1)
		return VOS_OK;

	/* added 20070716 如果设置了保护倒换则不再上报 */
	if( getPonPortApsCtrl( devIdx, brdIdx, ponIdx, &val ) == VOS_OK )
	{
		if( val == 2 || val == 3 )
			return VOS_OK;
	}
	/* added 20070802 如果PON板不在运行状态，或者系统正在主备倒换过程中则不再上报 */
	/*if( devIdx == 1 )
	{
		if( (SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) ||
			devsm_sys_is_switchhovering() )
		return VOS_OK;
	}*/	/* removed by xieshl 20080605 */
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponToEthLinkdown );
}

/* 功能:    以太网端口连接linkup事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponToEthLinkup_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	ULONG val = 0;
	
	if(check_eth_port_linkup_callback( devIdx, brdIdx, ponIdx )==1)	/* added by xieshl 20081020 */
		return VOS_OK;

	/* added 20070716 如果设置了保护倒换则不再上报 */
	if( getPonPortApsCtrl( devIdx, brdIdx, ponIdx, &val ) == VOS_OK )
	{
		if( val == 2 || val == 3 )
			return VOS_OK;
	}
	/* added 20070802 如果PON板不在运行状态，或者系统正在主备倒换过程中则不再上报 */
	/*if( devIdx == 1 )
	{
		if( (SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) ||
			devsm_sys_is_switchhovering() )
		return VOS_OK;
	}*/		/* removed by xieshl 20080605 */
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponToEthLinkup );
}
/* end 20070703 */

/*add by shixh20090626,添加PON端口的LOS告警*/
/* 功能:    PON端口信号丢失告警事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
			los－
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponPortLosAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx)	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx,trap_ponPortlosAlarm );
}

/* 功能:    PON端口信号丢失告警恢复事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponPortLosAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx)	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx,trap_ponPortlosAlarmClear);
}

/*add by wangjiah@2017-05-05*/
/* 功能:    PON端口信号丢失告警事件上报并携带光模块类型信息
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
			xfpTyoe-光模块类型
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponPortLosAlarmWithXFPType_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, int xfpType)	
{
	return private_ponEvent_report_with_type( devIdx, brdIdx, ponIdx, xfpType, trap_ponPortlosAlarm);
}

/*add by wangjiah@2017-05-05*/
/* 功能:    PON端口信号丢失告警恢复事件上报并携带光模块类型信息
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
			xfpTyoe-光模块类型
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponPortLosAlarmClearWithXFPType_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, int xfpType)	
{
	return private_ponEvent_report_with_type( devIdx, brdIdx, ponIdx, xfpType, trap_ponPortlosAlarmClear);
}
/*add by shixh20090710*/
/* 功能:   PON 口FW 软件版本不匹配事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－PON 端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponFWVersionMismatch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponFWVersionMismatch );
}

/* 功能:      PON 口FW 软件版本匹配事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponFWVersionMatch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponFWVersionMatch );
}
/* 功能:   PON 口DBA 软件不匹配事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponDBAVersionMismatch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponDBAVersionMismatch );
}
/* 功能:    PON 口DBA 软件匹配事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponDBAVersionMatch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponDBAVersionMatch );
}
/* 功能:    PON 口光模块类型不匹配事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponSFPTypeMismatch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponSFPTypeMismatch );
}
/* 功能:   PON 口光模块类型匹配事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponSFPTypeMatch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponSFPTypeMitch );
}
/*end shixh20090710*/	
# if 0
/*add by shixh20090715*/
/* 功能:    PON 芯片的MAC地址表里有BRAS MAC告警事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponBRASAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponportBRASAlarm );
}
/* 功能:   PON 芯片的MAC 地址表里有BRAS MAC告警清除事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponBRASAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponportBRASAlarmClear );
}
#endif
/* 功能:    PON 芯片的MAC地址表里有BRAS MAC告警事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponBRASAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, 
		uchar_t *pMacAddr )
{	
	eventMsg_t almMsg;
	
	/*if( devIdx != 1 )
	{
		sys_console_printf("onuRegAuthFailure_EventReport : OLT devIdx is %d???\r\n", devIdx);
		devIdx = 1;
	}*/
	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_ponportBRASAlarm;
	almMsg.alarmSrc.commAlarmSrc.devIdx = OLT_DEV_ID;	/*devIdx;*/
	almMsg.alarmSrc.commAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.commAlarmSrc.portIdx = ponIdx;
	VOS_MemCpy( almMsg.alarmSrc.commAlarmSrc.data, pMacAddr, 6 );

	return eventReportMsgSend( &almMsg );
}

/* 功能:   PON 芯片的MAC 地址表里有BRAS MAC告警清除事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			portIdx－以太网端口索引
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int ponBRASAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, 
		uchar_t *pMacAddr )
{	
	eventMsg_t almMsg;
	
	/*if( devIdx != 1 )
	{
		sys_console_printf("onuRegAuthFailure_EventReport : OLT devIdx is %d???\r\n", devIdx);
		devIdx = 1;
	}*/
	
	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_ponportBRASAlarmClear;
	almMsg.alarmSrc.commAlarmSrc.devIdx = OLT_DEV_ID;	/*devIdx;*/
	almMsg.alarmSrc.commAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.commAlarmSrc.portIdx = ponIdx;
	VOS_MemCpy( almMsg.alarmSrc.commAlarmSrc.data, pMacAddr, 6 );

	return eventReportMsgSend( &almMsg );
}

/*added by xieshl 20080116*/
int ethLoopAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethLoopAlarm );
}
int ethLoopAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethLoopAlarmClear );
}
int onuLoopAlarm_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_onuLoopAlarm );
}
int onuLoopAlarmClear_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_onuLoopAlarmClear );
}
/* end 20080116 */

/*add by shixh@20080215*/
int  backboneEthLinkdown_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	if(check_eth_port_linkdown_callback( devIdx, brdIdx, portIdx )==1)
		return VOS_OK;
	/*if( (SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) ||devsm_sys_is_switchhovering() )
		return VOS_OK;*/	/* removed by xieshl 20080605 */

	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_backboneEthLinkdown );
}	
int  backboneEthLinkup_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	if(check_eth_port_linkup_callback( devIdx, brdIdx, portIdx )==1)	/* added by xieshl 20081020 */
		return VOS_OK;
	/*if( (SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) ||devsm_sys_is_switchhovering() )
		return VOS_OK;*/	/* removed by xieshl 20080605 */

	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_backboneEthLinkup );
}	
/* end 20080215 */

/*add by shixh@20080202*/
/* 功能:   tdm 对应以太网端口linkdown事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			tdmIdx－tdm对应的一路fpga
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int tdmToEthLinkdown_EventReport( ULONG devIdx, ULONG brdIdx, ULONG tdmIdx )	
{
	if(tdmIdx>3||tdmIdx<1)
		return VOS_OK;
	/* added 20070802 如果PON板不在运行状态，或者系统正在主备倒换过程中则不再上报 */
	/*if( devIdx == 1 )
	{
		if( (SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) ||
			devsm_sys_is_switchhovering() )
		return VOS_OK;
	}*/		/* removed by xieshl 20080605 */
	return private_tdmlinkEvent_report( devIdx, brdIdx, tdmIdx, trap_tdmToEthLinkdown );
}

/* 功能:    tdm 对应以太网端口linkup事件上报
   输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引
			tdmIdx－tdm对应的一路fpga
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int tdmToEthLinkup_EventReport( ULONG devIdx, ULONG brdIdx, ULONG tdmIdx )	
{
	if(tdmIdx>3||tdmIdx<1)
		return VOS_OK;
	/* added 20070802 如果PON板不在运行状态，或者系统正在主备倒换过程中则不再上报 */
	/*if( devIdx == 1 )
	{
		if( (SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) ||
			devsm_sys_is_switchhovering() )
		return VOS_OK;
	}*/		/* removed by xieshl 20080605 */
	return private_tdmlinkEvent_report( devIdx, brdIdx, tdmIdx, trap_tdmToEthLinkup );
}
/*end add by shixh@20080202*/

/* added by xieshl 20080812 */
int ponPortLaserAlwaysOn_EventReport( ulong_t brdIdx, ulong_t portIdx, ulong_t onudevidx, LONG dbm)	
{
	return private_oltLaserAlwaysOnEvent_report(brdIdx, portIdx, onudevidx, dbm, trap_ponLaserAlwaysOnAlarm );
}
int ponPortLaserAlwaysOnClear_EventReport( ulong_t brdIdx, ulong_t portIdx, ulong_t onudevidx, LONG dbm)	
{
	return private_oltLaserAlwaysOnEvent_report( brdIdx, portIdx, onudevidx, dbm, trap_ponLaserAlwaysOnAlarmClear );
}
/* end 20080812 */


/*add by shixh@20070925*/
/* 私有MIB定义trap事件，E1端口相关事件上报  */
static int private_e1Event_report( ULONG devIdx, ULONG brdIdx,ULONG portIdx, ULONG alarmId)
{
	if( /*(SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) ||*/devsm_sys_is_switchhovering() )
		return VOS_OK;

	return private_portEvent_report( devIdx, brdIdx, portIdx, alarmId );
}

int e1AlarmMaskCheck( eventMsg_t *pAlarmMsg, ULONG mask )
{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	ULONG  idx[3];
	e1PortTable_t  e1PortTable;
	
	if( pAlarmMsg->alarmSrc.portAlarmSrc.devIdx == OLT_DEV_ID)
	{
		if( (pAlarmMsg->alarmSrc.portAlarmSrc.portIdx <= MAX_E1_PORT_NUM) && (pAlarmMsg->alarmSrc.portAlarmSrc.portIdx != 0) )
		{
			if( e1_Alarm_Mask(pAlarmMsg->alarmSrc.portAlarmSrc.portIdx) & mask )
		      	{
			 	return VOS_OK;
		      	}
		}
	}
	else
	{
		idx[0]=pAlarmMsg->alarmSrc.portAlarmSrc.devIdx;
		idx[1]=pAlarmMsg->alarmSrc.portAlarmSrc.brdIdx;
		idx[2]=pAlarmMsg->alarmSrc.portAlarmSrc.portIdx-1;
		VOS_MemZero( &e1PortTable, sizeof(e1PortTable_t) );
		if(sw_e1PortTable_get(idx, &e1PortTable) == VOS_ERROR)
		{
			return  VOS_OK;
		}	
		if( e1PortTable.eponE1PortAlarmMask & mask )
		{
			return VOS_OK;
		}
	}
#endif
	return VOS_ERROR;
}

/*  2009-4-8 chenfj 
	以下两个函数的第5 个参数uchar_t mask, 在使用E1_ALM_OOS 做调用时, 
	出现类型不匹配, 需将uchar_t 改为ushort_t
	同时需要修改的还有e1_Alarm_status[],e1_Alarm_status[],tdmService_Alarm_status[]的类型
	*/
int e1AlarmEventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG alarmId, ushort_t mask )
{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	if( devIdx == OLT_DEV_ID)
	{
		if ((portIdx <= MAX_E1_PORT_NUM) && (portIdx != 0) )
		{
			if( (e1_Alarm_status(portIdx) & mask) == 0 )
		      	{
		       	e1_Alarm_status(portIdx) |= mask;
		      	}
			else
				return VOS_OK;
		}
		else
			return VOS_ERROR;
	}
#endif
	return private_e1Event_report( devIdx, brdIdx,portIdx, alarmId);
}
int e1AlarmClearEventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG alarmId, ushort_t mask )
{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
 	if(devIdx ==OLT_DEV_ID)
	{
		if((portIdx <= MAX_E1_PORT_NUM) && (portIdx != 0))
		{
			e1_Alarm_status(portIdx) &= (~mask);
		}
		else
			return VOS_ERROR;
	}
#endif
	return private_e1Event_report( devIdx, brdIdx,portIdx, alarmId);
}

/* 功能:   e1口信号丢失事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1LosAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1LosAlarm, E1_ALM_LOS );
}

/* 功能:   e1口信号丢失清清除事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1LosAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1LosAlarmClear, E1_ALM_LOS );
}

/* 功能:   e1口桢 丢失事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1LofAlarm_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx)
{
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1LofAlarm, E1_ALM_LOF );
}

/* 功能:   e1口桢 丢失清除事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1LofAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1LofAlarmClear, E1_ALM_LOF );
}

/* 功能:   e1口本端告警指示事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
 int  e1AisAlarm_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx)
 {
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1AisAlarm, E1_ALM_AIS );
 }

/* 功能:   e1口本端告警指示清除事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1AisAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1AisAlarmClear, E1_ALM_AIS );
}

/* 功能:   e1口对端告警指示事件报告
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
 int  e1RaiAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
 {
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1RaiAlarm, E1_ALM_RAI );
 }

/* 功能:   e1口对端告警指示清除事件报告
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1RaiAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1RaiAlarmClear, E1_ALM_RAI );
}

/* 功能:   e1口信令复桢失步事件报告
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1SmfAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1SmfAlarm, E1_ALM_SMF );
}

/* 功能:   e1口信令复桢失步清楚事件报告
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1SmfAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1SmfAlarmClear, E1_ALM_SMF );
}

/* 功能:   e1口CRC复桢失步事件报告
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1LomfAlarm_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx)
{ 
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1LomfAlarm, E1_ALM_LOFSMF );
}

/* 功能:   e1口 CRC复桢失步清除事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1LomfAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1LomfAlarmClear, E1_ALM_LOFSMF );
}

/* 功能:   e1口CRC-3误码事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1Crc3Alarm_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx)
{
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1Crc3Alarm, E1_ALM_CRC3 );
}

/* 功能:   e1口 CRC-3误码清除事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1Crc3AlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1Crc3AlarmClear, E1_ALM_CRC3 );
}

/* 功能:   e1口CRC-6误码事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1Crc6Alarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1Crc6Alarm, E1_ALM_CRC6 );
}

/* 功能:   e1口 CRC-6误码清除事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			brdIdx－TDM板所在的槽位号，范围4~8
   			E1ClusterIdx-信令网关借接口缩影，范围1~3
			portIdx－每个 信令网关接接口上的E1编号，范围1~8
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  e1Crc6AlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1Crc6AlarmClear, E1_ALM_CRC6 );
}


/* 私有MIB定义trap事件，tdm相关事件上报  */
int private_tdmEvent_report( ULONG devIdx,ULONG alarmId)
{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	extern LONG setOnuTdmServiceAlarmStatus( ULONG devIdx, ULONG status );
#endif
	eventMsg_t almMsg;

	if( devIdx != OLT_DEV_ID )
	{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
		if( trap_tdmServiceAbortAlarm == alarmId )
		{
			if( setOnuTdmServiceAlarmStatus(devIdx, 1) == VOS_ERROR )
				return VOS_OK;
		}
		else
		{
			if( setOnuTdmServiceAlarmStatus(devIdx, 0) == VOS_ERROR )
				return VOS_OK;
		}
#endif
		VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
		almMsg.alarmType = alarmType_private;
		almMsg.alarmId = alarmId;
		almMsg.alarmSrc.devAlarmSrc.devIdx = devIdx;

		return eventReportMsgSend( &almMsg );
 	}
	return VOS_OK;
}

/* 功能:   指定ONU的 TDM业务中断事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
 int  tdmServiceAbortAlarm_EventReport( ULONG devIdx)
 {
	return  private_tdmEvent_report( devIdx,  trap_tdmServiceAbortAlarm );
 }

/* 功能:   指定ONU的 TDM业务中断清除事件上报
   输入参数:devIdx－OLT设备索引，取值为1
   			
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
 int  tdmServiceAbortAlarmClear_EventReport( ULONG devIdx)
 {   
	return  private_tdmEvent_report( devIdx,  trap_tdmServiceAbortAlarmClear);
 }

/* 功能:   E1 业务中断告警
   输入参数:devIdx－OLT设备索引，取值为1
   			
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
/*add by shixh@20090318*/
int  E1OutOfService_EventReport(ULONG  devIdx,ULONG brdIdx,ULONG portIdx)
{
	return e1AlarmEventReport( devIdx, brdIdx,portIdx, trap_E1OutOfService,E1_ALM_OOS);
}
/* 功能:   E1 业务中断告警
   输入参数:devIdx－OLT设备索引，取值为1
   			
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
 int  E1OutOfServiceClear_EventReport(ULONG  devIdx,ULONG brdIdx,ULONG portIdx)
{
	return  e1AlarmClearEventReport( devIdx,brdIdx, portIdx, trap_E1OutOfServiceClear,E1_ALM_OOS);
}
/*add by shixh@20080311*/
/*ONU自动配置事件上报*/
#if 0
static int private_onuAutoCfgEvent_report( ULONG devIdx,  ULONG alarmId)
{
       int rc = VOS_ERROR;
	ULONG ulMsg[4] = {MODULE_EVENT, FC_EVENT_REPORT, 0, 0};
	eventMsg_t *pAlmMsg = NULL;

	if(devsm_sys_is_switchhovering() )
		return VOS_OK;

	pAlmMsg = (eventMsg_t *)VOS_Malloc( sizeof(eventMsg_t), MODULE_EVENT ); 
	if( pAlmMsg == NULL )
	{
		ASSERT(0);
		return rc;
	}
	pAlmMsg->alarmType = alarmType_private;
	pAlmMsg->alarmId = alarmId;
	pAlmMsg->alarmSrc.commAlarmSrc.devIdx = devIdx;
	
       ulMsg[3] = (ULONG)pAlmMsg;
	rc = VOS_QueSend( eventQueId, ulMsg, NO_WAIT, getEventPriority(alarmType_private, alarmId) );
	if( rc != VOS_OK )
	{
		sys_console_printf("EVENT: QueSend is error for alarmId=%d, devIdx=%d\r\n", alarmId, devIdx);
		VOS_Free((void*)pAlmMsg);
	}
	return rc;

}
#endif 

#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )
/* 功能:  onu自动配置成功事件上报
   输入参数:devIdx－ONU设备
   			
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  onuLoadFileSuccess_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_onuAutoLoadConfigSuccess);
}

/* 功能:  onu自动配置失败事件上报
   输入参数:devIdx－ONU设备
   			
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  onuLoadFileFailure_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_onuAutoLoadConfigFailure);
}
/* 功能:  onu自动升级成功事件上报
   输入参数:devIdx－ONU设备
   			
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
int  onuAutoLoadUpgradeSuccess_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_onuAutoLoadUpgradeSuccess);
}
/* 功能:  onu自动升级失败事件上报
   输入参数:devIdx－ONU设备
   			
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
int  onuAutoLoadUpgradeFailure_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_onuAutoLoadUpgradeFailure);
}
#endif
/* 功能:  onu系统文件上传成功事件上报
   输入参数:devIdx－ONU设备
   			
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  onuSysFileUploadSuccess_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_sysfileUploadsuccess);
}
/* 功能:  onu系统文件上传失败事件上报
   输入参数:devIdx－ONU设备
   			
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  onuSysFileUploadFailure_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_sysfileUploadfailure);
}
/* 功能:  onu系统文件下载成功事件上报
   输入参数:devIdx－ONU设备
   			
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  onuSysFileDownloadSuccess_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_sysfileDownloadsuccess);
}
/* 功能:  onu系统文件下载失败事件上报
   输入参数:devIdx－ONU设备
   			
   返回值:  成功－VOS_OK，错误－VOS_ERROR */		 
int  onuSysFileDownloadFailure_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_sysfileDownloadfailure);
}


#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
extern VOID device_slot_module_pull( ULONG slotno, ULONG hot_cmd );
int tdmResetAlarm_EventReport()
{
	extern ULONG get_gfa_tdm_slotno();

	ULONG sig_slotno = get_gfa_tdm_slotno();
	if( sig_slotno != 0 )
	{
		device_slot_module_pull( sig_slotno, DEVSM_HOT_PULL_BY_REBOOT );
	}
	else
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	return VOS_OK;
}
#endif

/*----------------------------------------------------------------------------*/
/* 标准事件上报 */
/*----------------------------------------------------------------------------*/

/* 功能:    OLT以太网STP事件上报
   输入参数:stpEvent－2拓扑改变事件，1根桥事件
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int oltStp_EventReport( ULONG stpEvent )
{
	eventMsg_t almMsg;
	
	if( (stpEvent >= trap_bridge_max) && (stpEvent <= trap_bridge_min) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_bridge;
	almMsg.alarmId = stpEvent;
	almMsg.alarmSrc.devAlarmSrc.devIdx = OLT_DEV_ID;
	almMsg.alarmSrc.devAlarmSrc.devData = 0;

	return eventReportMsgSend( &almMsg );
}

/* 功能:    OLT MIBII事件上报
   输入参数:mib2Event－
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int oltMib2_EventReport( ULONG mib2Event )
{
	eventMsg_t almMsg;

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_mib2;
	almMsg.alarmId = mib2Event;
	almMsg.alarmSrc.devAlarmSrc.devIdx = OLT_DEV_ID;
	almMsg.alarmSrc.devAlarmSrc.devData = 0;

	return eventReportMsgSend( &almMsg );
}

/*----------------------------------------------------------------------------*/
/* 其它事件，主要用于记录日志，不需上报trap */
/*----------------------------------------------------------------------------*/
/* 功能:    ONU以太网STP事件上报
   输入参数:onuDevIdx－ONU设备索引
   			stpEvent－1拓扑改变事件，2根桥事件
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int onuStp_EventReport( ULONG onuDevIdx, ULONG stpEvent )
{
	eventMsg_t almMsg;

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_other;
	if( stpEvent == onuStp_topologyChange )
		almMsg.alarmId = other_onustp_topologychange;
	else if( stpEvent == onuStp_newRoot )
		almMsg.alarmId = other_onustp_newroot;
	else
		return VOS_ERROR;

	almMsg.alarmSrc.devAlarmSrc.devIdx = onuDevIdx;
	almMsg.alarmSrc.devAlarmSrc.devData = 0;

	return eventReportMsgSend( &almMsg );
}

/* 功能:    自检事件上报，主要用于记录OLT自检结果
   输入参数:result－自检结果，
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int diagnosis_EventReport( ULONG result )
{
	eventMsg_t almMsg;

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_other;
	if( result == 0 )
		almMsg.alarmId = other_diagnosis_success;
	else
		almMsg.alarmId = other_diagnosis_fail;
	almMsg.alarmSrc.devAlarmSrc.devIdx = OLT_DEV_ID;
	almMsg.alarmSrc.devAlarmSrc.devData = 0;

	return eventReportMsgSend( &almMsg );
}

/* added by xieshl 20080421 */
/* 功能:   管理通道事件上报，主要用于记录主控管理通道是否中断即恢复情况
   输入参数:status－通道状态，0-正常，非0-异常或中断
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int cpuCtrlChan_EventReport( ULONG alarmId )
{
	eventMsg_t almMsg;
	
	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_other;
	almMsg.alarmId = alarmId;
	almMsg.alarmSrc.devAlarmSrc.devIdx = OLT_DEV_ID;
	almMsg.alarmSrc.devAlarmSrc.devData = 0;

	return eventReportMsgSend( &almMsg );
}

/* added by chenfj 2009-2-19
	 新增PON端口版本是否匹配告警
 */
int PonPortOther_EventReport( ULONG slot, ULONG port,  ULONG AlarmId)
{
	eventMsg_t almMsg;

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_other;	
	almMsg.alarmId = AlarmId;
	almMsg.alarmSrc.portAlarmSrc.devIdx = OLT_DEV_ID;
	almMsg.alarmSrc.portAlarmSrc.brdIdx = slot;
	almMsg.alarmSrc.portAlarmSrc.portIdx = port;

	return eventReportMsgSend( &almMsg );
}

/* added by xieshl 20110117, 记录部分SNMP操作到告警日志 */
int snmp_Nms_Restart_Dev_EventReport( ULONG devIdx, LONG result )
{
	eventMsg_t almMsg;

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_other;
	almMsg.alarmId = other_snmp_reset_dev;
	almMsg.alarmSrc.otherAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.otherAlarmSrc.result = result;

	return eventReportMsgSend( &almMsg );
}
int snmp_Nms_Reset_Board_EventReport( ULONG devIdx, ULONG brdIdx, LONG result )
{
	eventMsg_t almMsg;

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_other;
	almMsg.alarmId = other_snmp_reset_brd;
	almMsg.alarmSrc.otherAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.otherAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.otherAlarmSrc.result = result;

	return eventReportMsgSend( &almMsg );
}
int snmp_Nms_Reset_Olt_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx, LONG result )
{
	eventMsg_t almMsg;

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_other;
	almMsg.alarmId = other_snmp_reset_pon;
	almMsg.alarmSrc.otherAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.otherAlarmSrc.brdIdx = brdIdx;
	almMsg.alarmSrc.otherAlarmSrc.portIdx = portIdx;
	almMsg.alarmSrc.otherAlarmSrc.result = result;

	return eventReportMsgSend( &almMsg );
}
int snmp_Nms_Save_Config_EventReport( ULONG devIdx, LONG result )
{
	eventMsg_t almMsg;

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_other;
	almMsg.alarmId = other_snmp_save_config;
	almMsg.alarmSrc.otherAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.otherAlarmSrc.result = result;

	return eventReportMsgSend( &almMsg );
}
int snmp_Nms_Erase_Config_EventReport( ULONG devIdx, LONG result )
{
	eventMsg_t almMsg;

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_other;
	almMsg.alarmId = other_snmp_erase_config;
	almMsg.alarmSrc.otherAlarmSrc.devIdx = devIdx;
	almMsg.alarmSrc.otherAlarmSrc.result = result;

	return eventReportMsgSend( &almMsg );
}
/* end 20110117 */

/*B--add by sxh20111227*/
int OnuMacTtableOverflow_EventReport( ULONG onuDevIdx)
{
        eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_onuMacTableOverFlow;
	almMsg.alarmSrc.devAlarmSrc.devIdx = onuDevIdx;

	return eventReportMsgSend( &almMsg );
}

int OnuMacTtableOverflow_EventReportClear( ULONG onuDevIdx)
{
        eventMsg_t almMsg; 

	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_onuMacTableOverFlowClear;
	almMsg.alarmSrc.devAlarmSrc.devIdx = onuDevIdx;

	return eventReportMsgSend( &almMsg );
}
/*E--add by sxh20111227*/

int userTraceUpdate_EventReport( ULONG hashIdx, UCHAR *pMacAddr )
{
        eventMsg_t almMsg; 
	if( pMacAddr == NULL )
		return VOS_ERROR;
	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = trap_userLocUpdateNotify;
	almMsg.alarmSrc.userTraceAlarmSrc.hashIdx = hashIdx;
	VOS_MemCpy( almMsg.alarmSrc.userTraceAlarmSrc.macAddr, pMacAddr, 6 );
	return eventReportMsgSend( &almMsg );
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

int CmcTrap_EventReport(unsigned char CmcMac[6], unsigned char CmMac[6], int event_code)
{
    int rc = VOS_ERROR;
    
#if( EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES )
    short int OnuEntryIdx;

    if ( 0 <= (OnuEntryIdx = GetOnuEntryByMac(CmcMac)) )
    {
        short int PonPortIdx, OnuId;
        short int PonSlotId, PonPortId;
        eventMsg_t almMsg; 
        
        PonPortIdx = OnuEntryIdx / MAXONUPERPON;
        OnuId = OnuEntryIdx % MAXONUPERPON + 1;

        notify_cmc_event(event_code, PonPortIdx, OnuId - 1, CmcMac, CmMac);
#if( EPON_MODULE_DOCSIS_MANAGE_MIB == EPON_MODULE_YES )
        PonSlotId = GetCardIdxByPonChip(PonPortIdx);
        PonPortId = GetPonPortByPonChip(PonPortIdx);

    	VOS_MemZero( &almMsg, sizeof(eventMsg_t) );
    	almMsg.alarmType = alarmType_bcmcmcctrl;
    	almMsg.alarmId = event_code;
    	almMsg.alarmSrc.commAlarmSrc.devIdx = 0;
    	almMsg.alarmSrc.commAlarmSrc.brdIdx = (UCHAR)PonSlotId;
    	almMsg.alarmSrc.commAlarmSrc.portIdx = (UCHAR)PonPortId;
    	almMsg.alarmSrc.commAlarmSrc.onuIdx = (UCHAR)OnuId;
        if ( CmMac != NULL )
        {
        	VOS_MemCpy( almMsg.alarmSrc.commAlarmSrc.data, CmMac, 6 );
        }
        else
        {
            almMsg.alarmSrc.commAlarmSrc.data[0] = 1;
        }

    	rc = eventReportMsgSend( &almMsg );
#endif
    }
#endif

    return rc;
}

