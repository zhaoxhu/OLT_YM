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

/* ˽��MIB����trap�¼����豸����¼��ϱ� */
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

/* ����:    ��ONUע���¼��ϱ�������ONU��һ��ע�ᣬ�����ONU�豸��Ϣ��ѯ��֪ͨ
			�澯�����ر�ע��:	trap����Ҫ��3������deviceIndex, deviceType, 
			deviceEntLogicalCommunity����ˣ�����ȷ��������¼�ʱONU���ͺ��豸
			������MIB���Ѹ�ֵ
   �������:devIdx����ע��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int onuNewRegSuccess_EventReport( ULONG devIdx )
{
	return private_devEvent_report( devIdx, trap_onuNewRegSuccess );
}

/* ����:    ONU����ע���¼��ϱ�������ONU����ע�ᣬ�����ONU�豸��Ϣ��ѯ��֪ͨ
			�澯�����ر�ע��:	trap����Ҫ��3������deviceIndex, deviceType, 
			deviceEntLogicalCommunity����ˣ�����ȷ��������¼�ʱONU���ͺ��豸
			������MIB���Ѹ�ֵ
   �������:devIdx��ע��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int onuReregSuccess_EventReport( ULONG devIdx )
{
	return private_devEvent_report( devIdx, trap_onuReregSuccess );
}

/* ����:    ONU����ע���¼��ϱ�
   �������:devIdx������ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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

/* ����:    �豸�����¼��ϱ��������OLT���磬����Ҫ���⴦�������ϱ��澯����
   �������:devIdx������(ONU/OLT)�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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

/* ����:    �豸����ָ��¼��ϱ�����Ҫ���ONU�豸��OLT��������ʱ���ϱ������¼�
			��ONU����ע�ᴥ��������ͬ�����ݼ�¼��ONU״̬ȷ���ǵ��绹������
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int devPowerOn_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_devPowerOn );
}
/*----------------------------------------------------------------------------*/

/* ����:    OLT�������ݱ���flash�ɹ��¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int cfgDataSaveSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cfgDataSaveSuccess );
}

/* ����:    OLT�������ݱ���flashʧ���¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int cfgDataSaveFail_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cfgDataSaveFail );
}

/* ����:    OLT�������ݴ�flash�����ɹ��¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int flashClearSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_flashClearSuccess );
}

/* ����:    OLT�������ݴ�flash����ʧ���¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int flashClearFail_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_flashClearFail );
}

/* ����:    ͨ��FTP�ϴ�OLT/ONU�����OLT flash�ɹ��¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int softwareUpdateSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_softwareUpdateSuccess );
}

/* ����:    ͨ��FTP�ϴ�OLT/ONU�����OLT flashʧ���¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int softwareUpdateFail_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_softwareUpdateFail );
}

/* ����:    ͨ��FTP�ϴ�OLT/ONU�̼���OLT flash�ɹ��¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int firmwareUpdateSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_firmwareUpdateSuccess );
}

/* ����:    ͨ��FTP�ϴ�OLT/ONU�̼���OLT flashʧ���¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int firmwareUpdateFail_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_firmwareUpdateFail );
}

/* ����:    ͨ��FTP����OLT/ONU�������ݵ�FTP server�ɹ��¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int cfgDataBackupSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cfgDataBackupSuccess );
}

/* ����:    ͨ��FTP����OLT/ONU�������ݵ�FTP serverʧ���¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int cfgDataBackupFail_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cfgDataBackupFail );
}

/* ����:    ͨ��FTP�ϴ�OLT/ONU�������ݵ�OLT flash�ɹ��¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int cfgDataRestoreSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cfgDataRestoreSuccess );
}

/* ����:    ͨ��FTP�ϴ�OLT/ONU�������ݵ�OLT flashʧ���¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int cfgDataRestoreFail_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_cfgDataRestoreFail );
}

/* ����:    CPUռ���ʹ����¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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

#if 0   /*�Ѿ���*/
int ctcOnuPowerAlarm_EventReport( ULONG devIdx )
{
    return VOS_OK;
}
int ctcOnuPowerAlarmClear_EventReport( ULONG devIdx )
{
    return VOS_OK;
}
#endif
/* ����:    CTC ONU ����޷������¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ctcOnuBatteryMissing_EventReport( ULONG devIdx )
{
        return private_devEvent_report(devIdx,trap_ctcOnuBatteryMissing);
}
int ctcOnuBatteryMissingClear_EventReport( ULONG devIdx )
{
    return private_devEvent_report(devIdx,trap_ctcOnuBatteryMissingClear);
}

/* ����:    CTC ONU ��ز����ڳ���¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ctcOnuBatteryFailure_EventReport( ULONG devIdx )
{
        return private_devEvent_report(devIdx,trap_ctcOnuBatteryFailure);
}
int ctcOnuBatteryFailureClear_EventReport( ULONG devIdx )
{
    return private_devEvent_report(devIdx,trap_ctcOnuBatteryFailureClear);
}

/* ����:    CTC ONU ��ص�ѹ���¼��ϱ�
   �������:devIdx��ONU�豸�����tthreshold-��ѹ��ʵ��ֵ
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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

/* ����:    CTC ONU �зǷ�����澯�¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ctcOnuPhysicalIntrusionAlarm_EventReport( ULONG devIdx )
{
        return private_devEvent_report(devIdx,trap_ctcOnuPhysicalIntrusionAlarm);
}
int ctcOnuPhysicalIntrusionAlarmClear_EventReport( ULONG devIdx )
{
    return private_devEvent_report(devIdx,trap_ctcOnuPhysicalIntrusionAlarmClear);
}

/* ����:    CTC ONU �Բ�ʧ���¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ctcOnuSelfTestFailure_EventReport( ULONG devIdx )
{
        return private_devEvent_report(devIdx,trap_ctcOnuSelfTestFailure);
}
int ctcOnuSelfTestFailureClear_EventReport( ULONG devIdx )
{
    return private_devEvent_report(devIdx,trap_ctcOnuSelfTestFailureClear);
}

/* ����:    CTC ONU �Բ�ʧ���¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ctcOnuIADConnectionFailure_EventReport( ULONG devIdx )
{
        return private_devEvent_report(devIdx,trap_ctcOnuIADConnectionFailure);
}
int ctcOnuIADConnectionFailureClear_EventReport( ULONG devIdx )
{
     return private_devEvent_report(devIdx,trap_ctcOnuIADConnectionFailureClear);
}
/* ����:    CTC ONU ��PON�˿��л��¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ctcOnuPonIfSwitch_EventReport( ULONG devIdx )
{
       return private_devEvent_report(devIdx,trap_ctcOnuPonIfSwitch);
}
int ctcOnuPonIfSwitchClear_EventReport( ULONG devIdx )
{
    return private_devEvent_report(devIdx,trap_ctcOnuPonIfSwitchClear);
}



/* ����:    CTC ONU ��ETH �˿���Э��ʧ���¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ethAutoNegFailure_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
        return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethAutoNegFailure);
}
int ethAutoNegFailureClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
     return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethAutoNegFailureClear);
}

/* ����:    CTC ONU ��ETH �˿��źŶ�ʧ�¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ethLos_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
       return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethLos);
}
int ethLosClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{                    
    return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethLosCLear);
}


/* ����:    CTC ONU ��ETH �˿�ʧ�ܸ澯�¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ethFailure_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
        return  private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethFailure);
}
int ethFailureClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
     return  private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethFailureClear);
}
#if 0
/* ����:    CTC ONU ��ETH �˿ڻ�·�¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ethloopback_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
        return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethloopback);
}
int ethloopbackClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
   return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethloopbackCLear);
}
#endif
/* ����:    CTC ONU ��ETH �˿ڳ�ͻ�¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ethCongestion_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
        return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethCongestion);
}
int ethCongestionClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx )
{
    return private_portEvent_report(devIdx,brdIdx,ethIdx,trap_ethCongestionClear);
}

/*test end*/


/* modified by xieshl 20080630, ���Ӱ忨���� */
/* ˽��MIB����trap�¼�����������¼��ϱ� */
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
/* ����:    ��Դ�忨�����¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int PWUPowerOff_EventReport( ULONG devIdx, ULONG brdIdx, ULONG brdType )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_powerOffAlarm, brdType );
}

/* ����:    ��Դ�忨�ϵ��¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int PWUPowerOn_EventReport( ULONG devIdx, ULONG brdIdx, ULONG brdType )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_powerOnAlarm, brdType );
}
/* end: added by jianght 20090519 */
#endif


/* BEGIN: added  by @muqw  2017-04-26*/
/* ����:    ��Դ�쳣״̬�ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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

/* ����:    ͨ��FTP�ϴ�PON DBA��OLT flash�ɹ��¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int dbaUpdateSuccess_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_dbaUpdateSuccess );
}

/* ����:    ͨ��FTP�ϴ�PON DBA��OLT flashʧ���¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int dbaUpdateFailure_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_dbaUpdateFailure );
}

/* ����:    ͨ��OAM�ļ�����ͨ�����������ONU�ɹ��¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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

/* ����:    ͨ��OAM�ļ�����ͨ�����������ONUʧ���¼��ϱ�
   �������:devIdx��ONU�豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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

/* ����:    ����SW�嵹���¼��ϱ�
   �������:devIdx��OLT�豸�������豸�������������ѭMIB����
   			brdIdx����������������ز�λ��
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int swBoardProtectedSwitch_EventReport( ULONG devIdx, ULONG brdIdx )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_swBoardProtectedSwitch, 0 );
}

/* ����:    �����¶ȹ����¼��ϱ�
   �������:devIdx��OLT/ONU�豸�������豸�������������ѭMIB���壬��ͬ
   			brdIdx������(��λ��)��������Χ1~11����ͬ
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int boardTemperatureHigh_EventReport( ULONG devIdx, ULONG brdIdx, LONG tempVal )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, 0, tempVal, trap_boardTemperatureHigh );
}

/* ����:    �����¶ȹ����¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int boardTemperatureHighClear_EventReport( ULONG devIdx, ULONG brdIdx, LONG tempVal )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, 0, tempVal, trap_boardTemperatureHighClear );
}

/* ����:    �豸�¶ȹ����¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   		
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int deviceTemperatureHigh_EventReport( ULONG devIdx, LONG temperature, LONG threshold )
{
	ULONG data = ((temperature & 0xff) << 8) | (threshold & 0xff );
	return private_devEvent_report_ext( devIdx, trap_deviceTemperatureHigh, data );
}

/* ����:    �����¶ȹ����¼��ָ��ϱ�
   �������:devIdx��OLT/ONU�豸����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int deviceTemperatureHighClear_EventReport( ULONG devIdx, LONG temperature, LONG threshold )
{
	ULONG data = ((temperature & 0xff) << 8) | (threshold & 0xff );
	return private_devEvent_report_ext( devIdx, trap_deviceTemperatureHighClear, data );
}

/* ����:    �豸�¶ȹ����¼��ϱ� 
   �������:devIdx��OLT/ONU�豸����
   		
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int deviceTemperatureLow_EventReport( ULONG devIdx, LONG temperature, LONG threshold )
{
	ULONG data = ((temperature & 0xff) << 8) | (threshold & 0xff );
	return private_devEvent_report_ext( devIdx, trap_deviceTemperatureLow, data );
}

/* ����:    �����¶ȹ����¼��ָ��ϱ�
   �������:devIdx��OLT/ONU�豸����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int deviceTemperatureLowClear_EventReport( ULONG devIdx, LONG temperature, LONG threshold )
{
	ULONG data = ((temperature & 0xff) << 8) | (threshold & 0xff );
	return private_devEvent_report_ext( devIdx, trap_deviceTemperatureLowClear, data );
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/


/* ����:    ���帴λ�¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponBoardReset_EventReport( ULONG devIdx, ULONG brdIdx )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_ponBoardReset, 0 );
}

/* ����:    �����������¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int devBoardInterted_EventReport( ULONG devIdx, ULONG brdIdx, ULONG brdType )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_devBoardInterted, brdType );
}

/* ����:    �������γ��¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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

/* ����:    �������γ��¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int devBoardLosClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG brdType )	
{
    if(board_los_alarm_state[brdIdx])
    {
        board_los_alarm_state[brdIdx] = 0;
    	return private_brdEvent_report( devIdx, brdIdx, trap_boardLosAlarmClear, brdType );
	}
	return VOS_OK;
}
/* ����:    ��Դ��ֹͣ�����¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int powerOffAlarm_EventReport( ULONG devIdx, ULONG brdIdx )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_powerOffAlarm, 0 );
}

/* ����:    ��Դ��ָ������¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int powerOnAlarm_EventReport( ULONG devIdx, ULONG brdIdx )	
{
	return private_brdEvent_report( devIdx, brdIdx, trap_powerOnAlarm, 0 );
}

/* ����:    �����쳣�¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			fanIdx����������
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int devFanAlarm_EventReport( ULONG devIdx, ULONG fan_id )	
{
	ULONG slotno = FANID2SLOTNO(fan_id);	/* modified by xieshl 20121211, ���ⵥ16144 */
	ULONG portno = FANID2FANNO(fan_id);
	return private_portEvent_report( devIdx, slotno, portno, trap_devFanAlarm );
}

/* ����:    �����쳣�ָ��¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			fanIdx����������
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int devFanAlarmClear_EventReport( ULONG devIdx, ULONG fan_id )	
{
	ULONG slotno = FANID2SLOTNO(fan_id);
	ULONG portno = FANID2FANNO(fan_id);
	return private_portEvent_report( devIdx, slotno, portno, trap_devFanAlarmClear );
}

/*added by wangxiaoyu 2008-7-24 17:37:30*/
/* ����:    OLT��PON���ʲ����������¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
			rtVal -- ��ͬ��trapId�����˴�ֵ��ͬ
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */

int ponPortOpticalPowerAlarm_EventReport( ULONG trapId, ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG rtVal )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, rtVal, trapId);
}
/*end*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/* ˽��MIB����trap�¼���PON���ܼ�������¼��ϱ� */
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

/* ����:    PON�����ʳ������¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
			ber�������ʣ���λ10E-6
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponPortBERAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG ber )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, ber, trap_ponPortBERAlarm );
}

/* ����:    PON�����ʳ����޻ָ��¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
			ber�������ʣ���λ10E-6
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponPortBERAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG ber )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, ber, trap_ponPortBERAlarmClear );
}

/* ����:    PON��֡�ʳ������¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
			fer����֡�ʣ���λ10E-6
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponPortFERAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG fer )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, fer, trap_ponPortFERAlarm );
}

/* ����:    PON��֡�ʳ����޻ָ��¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
			fer����֡�ʣ���λ10E-6
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponPortFERAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG fer )	
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, fer, trap_ponPortFERAlarmClear );
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/* ˽��MIB����trap�¼���LLID����¼��ϱ� */
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


/* ����:    LLID�����޻����¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
			llidIdx��LLID��������Χ1~64
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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

/* ˽��MIB����trap�¼���eth����¼��ϱ� *//*add by shixh20090520*/
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

/*b-add by zhaoxh ��� ���ݹ���б��������;��������澯�ϱ�Ŀǰֻ��֧�ּ�⵽PON�ڣ�δ�ﵽ����ONU*/
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
/*e-zhaoxh �����ֻ������onu���Ծͻ�ȡdown����onu*/
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

/* ˽��MIB����trap�¼�����̫���˿�����¼��ϱ� */
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

/* ˽��MIB����trap�¼���PON�˿�����¼��ϱ� */
#define private_ponEvent_report( devIdx, brdIdx, ponIdx, alarmId ) \
	private_portEvent_report( devIdx, brdIdx, ponIdx, alarmId )
#define private_tdmlinkEvent_report( devIdx, brdIdx, tdmIdx, alarmId ) \
	private_portEvent_report( devIdx, brdIdx, tdmIdx, alarmId )
/* ����:    PON���������¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)������
			ponIdx��PON�˿�������
			partnerDevIdx���Զ�OLT�豸����
   			partnerBrdIdx���Զ˵���(��λ��)������
			partnerPonIdx���Զ�PON�˿�������
			partnerIpAddr����OLT�����Զ�IP��ַ
			partnerUdpPort����OLT�����Զ�UDP�˿�
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR 
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

/* ����:	�߼���λ�����¼��ϱ� 
   �������:devIdx��OLT�豸����
   			brdIdx���߼���λ��������Χ65~68
   			ipAddr��Զ���豸IP��ַ
			udpPort��Զ���豸�߼���λUDP�˿�
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int logicalSlotInsert_EventReport(ULONG devIdx, UCHAR brdIdx, ULONG ipAddr, ULONG udpPort)
{
	return private_logicalSlotEvent_report(devIdx, brdIdx, ipAddr, udpPort, trap_logicalSlotInsert);
}

/* ����:	�߼���λ�γ��¼��ϱ� 
   �������:devIdx��OLT�豸����
   			brdIdx���߼���λ��������Χ65~68
   			ipAddr��Զ���豸IP��ַ
			udpPort��Զ���豸�߼���λUDP�˿�
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int logicalSlotPull_EventReport(ULONG devIdx, UCHAR brdIdx, ULONG ipAddr, ULONG udpPort)
{
	return private_logicalSlotEvent_report(devIdx, brdIdx, ipAddr, udpPort, trap_logicalSlotPull);
}

	
/* ����:    PON���������¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int autoProtectSwitch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_autoProtectSwitch );
}

/* ����:    PON���쳣�¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponPortAbnormal_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, long resetCode )
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, resetCode, trap_ponPortAbnormal );
}

int ponPortAbnormalClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )
{
	return private_ponMonEvent_report( devIdx, brdIdx, ponIdx, 0, trap_ponPortAbnormalClear );
}


/* ����:    PON��ONUע���ͻ�¼��ϱ���һ��ָONU MAC��ַ��ͻ
   �������:devIdx����ע��ONU�豸��������Ϊ����ע���ONU��û�м��뵽�豸������У�
   			���豸������û�з��䣬���ϱ��澯ʱֻ�����ĸ�ONU��ͻ
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int onuRegisterConflict_EventReport( ULONG devIdx/*, ULONG brdIdx, ULONG ponIdx*/ )	
{
	return private_devEvent_report( devIdx, trap_onuRegisterConflict );
}

/* ����:    PON�̼����سɹ��¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int firmwareLoadSuccess_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_firmwareLoadSuccess );
}

int firmwareLoadFailure_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_firmwareLoadFailure );
}

/* ����:    PON DBA�̼����سɹ��¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int dbaLoadSuccess_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_dbaLoadSuccess );
}

int dbaLoadFailure_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_dbaLoadFailure );
}

/* ����:    ��̫����֡�ʳ������¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)��������Χ1~11
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ethFlrAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethFlrAlarm );
}

/* devIndex, boardIndex, portIndex */
int ethFlrAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethFlrAlarmClear );
}

/* ����:    ��̫����֡�ʳ������¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)��������Χ1~11
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ethFerAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethFerAlarm );
}

int ethFerAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethFerAlarmClear );
}

/* ����:    ��̫��ҵ���ж��¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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
			/* ��Ҫ���⴦�� */
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
/* ˽��MIB����trap�¼���OLT recevier���ܼ�������¼��ϱ� */
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

/* ����:    ��̫���˿�����linkdown�¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ethLinkdown_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	/* modified by xieshl 20110705, �������ܿڲ���Ҫ��黷·���⣬���ⵥ12083 */
	if( portIdx  && ( devIdx == OLT_DEV_ID ) )
	{
		if (check_eth_port_linkdown_callback( devIdx, brdIdx, portIdx ) == 1)
			return VOS_OK;
	}
	/* added 20070802 ��������岻������״̬������ϵͳ�����������������������ϱ� */
	if( devIdx == OLT_DEV_ID )
	{
	    #if 0 /***removed by @muqw for reporting up/down while board is rebooting or switchoverring***/
		if( (SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) || devsm_sys_is_switchhovering() )
		return VOS_OK;
        #endif
	}
    return private_portEvent_report( devIdx, brdIdx, portIdx, trap_ethLinkdown );
}

/* ����:    ��̫���˿�����linkup�¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ethLinkup_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	
{
	/* modified by xieshl 20110705, �������ܿڲ���Ҫ��黷·���⣬���ⵥ12083 */
	if( portIdx  && ( devIdx == OLT_DEV_ID ) )
	{
		if(check_eth_port_linkup_callback( devIdx, brdIdx, portIdx )==1)	/* added by xieshl 20081020 */
			return VOS_OK;
	}
	/* added 20070802 ��������岻������״̬������ϵͳ�����������������������ϱ� */
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
       
/* ˽��MIB����trap�¼����Ƿ�ONU�¼��ϱ� */
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
/* ����:    �豸�������¼��ϱ�
   �������:devIdx���豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int deviceColdStart_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_deviceColdStart );
}
/*----------------------------------------------------------------------------*/
/* ����:    �豸�������¼��ϱ�
   �������:devIdx���豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int deviceWarmStart_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_deviceWarmStart );
}
/*----------------------------------------------------------------------------*/
/* ����:    �豸�������¼��ϱ�
   �������:devIdx���豸�������豸�������������ѭMIB����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int deviceExceptionRestart_EventReport( ULONG devIdx )	
{
	return private_devEvent_report( devIdx, trap_deviceExceptionRestart );
}
/*----------------------------------------------------------------------------*/
/* ����:    ��̫���˿�����linkdown�¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponToEthLinkdown_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	ULONG val = 0;
	if(check_eth_port_linkdown_callback( devIdx, brdIdx, ponIdx )==1)
		return VOS_OK;

	/* added 20070716 ��������˱������������ϱ� */
	if( getPonPortApsCtrl( devIdx, brdIdx, ponIdx, &val ) == VOS_OK )
	{
		if( val == 2 || val == 3 )
			return VOS_OK;
	}
	/* added 20070802 ���PON�岻������״̬������ϵͳ�����������������������ϱ� */
	/*if( devIdx == 1 )
	{
		if( (SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) ||
			devsm_sys_is_switchhovering() )
		return VOS_OK;
	}*/	/* removed by xieshl 20080605 */
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponToEthLinkdown );
}

/* ����:    ��̫���˿�����linkup�¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponToEthLinkup_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	ULONG val = 0;
	
	if(check_eth_port_linkup_callback( devIdx, brdIdx, ponIdx )==1)	/* added by xieshl 20081020 */
		return VOS_OK;

	/* added 20070716 ��������˱������������ϱ� */
	if( getPonPortApsCtrl( devIdx, brdIdx, ponIdx, &val ) == VOS_OK )
	{
		if( val == 2 || val == 3 )
			return VOS_OK;
	}
	/* added 20070802 ���PON�岻������״̬������ϵͳ�����������������������ϱ� */
	/*if( devIdx == 1 )
	{
		if( (SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) ||
			devsm_sys_is_switchhovering() )
		return VOS_OK;
	}*/		/* removed by xieshl 20080605 */
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponToEthLinkup );
}
/* end 20070703 */

/*add by shixh20090626,���PON�˿ڵ�LOS�澯*/
/* ����:    PON�˿��źŶ�ʧ�澯�¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
			los��
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponPortLosAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx)	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx,trap_ponPortlosAlarm );
}

/* ����:    PON�˿��źŶ�ʧ�澯�ָ��¼��ϱ�
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponPortLosAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx)	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx,trap_ponPortlosAlarmClear);
}

/*add by wangjiah@2017-05-05*/
/* ����:    PON�˿��źŶ�ʧ�澯�¼��ϱ���Я����ģ��������Ϣ
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
			xfpTyoe-��ģ������
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponPortLosAlarmWithXFPType_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, int xfpType)	
{
	return private_ponEvent_report_with_type( devIdx, brdIdx, ponIdx, xfpType, trap_ponPortlosAlarm);
}

/*add by wangjiah@2017-05-05*/
/* ����:    PON�˿��źŶ�ʧ�澯�ָ��¼��ϱ���Я����ģ��������Ϣ
   �������:devIdx��OLT/ONU�豸����
   			brdIdx������(��λ��)��������Χ1~11
			ponIdx��PON�˿���������Χ1~4
			xfpTyoe-��ģ������
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponPortLosAlarmClearWithXFPType_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, int xfpType)	
{
	return private_ponEvent_report_with_type( devIdx, brdIdx, ponIdx, xfpType, trap_ponPortlosAlarmClear);
}
/*add by shixh20090710*/
/* ����:   PON ��FW ����汾��ƥ���¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx��PON �˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponFWVersionMismatch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponFWVersionMismatch );
}

/* ����:      PON ��FW ����汾ƥ���¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponFWVersionMatch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponFWVersionMatch );
}
/* ����:   PON ��DBA �����ƥ���¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponDBAVersionMismatch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponDBAVersionMismatch );
}
/* ����:    PON ��DBA ���ƥ���¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponDBAVersionMatch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponDBAVersionMatch );
}
/* ����:    PON �ڹ�ģ�����Ͳ�ƥ���¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponSFPTypeMismatch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponSFPTypeMismatch );
}
/* ����:   PON �ڹ�ģ������ƥ���¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponSFPTypeMatch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
	return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponSFPTypeMitch );
}
/*end shixh20090710*/	
# if 0
/*add by shixh20090715*/
/* ����:    PON оƬ��MAC��ַ������BRAS MAC�澯�¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponBRASAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponportBRASAlarm );
}
/* ����:   PON оƬ��MAC ��ַ������BRAS MAC�澯����¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int ponBRASAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )	
{
return private_ponEvent_report( devIdx, brdIdx, ponIdx, trap_ponportBRASAlarmClear );
}
#endif
/* ����:    PON оƬ��MAC��ַ������BRAS MAC�澯�¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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

/* ����:   PON оƬ��MAC ��ַ������BRAS MAC�澯����¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			portIdx����̫���˿�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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
/* ����:   tdm ��Ӧ��̫���˿�linkdown�¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			tdmIdx��tdm��Ӧ��һ·fpga
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int tdmToEthLinkdown_EventReport( ULONG devIdx, ULONG brdIdx, ULONG tdmIdx )	
{
	if(tdmIdx>3||tdmIdx<1)
		return VOS_OK;
	/* added 20070802 ���PON�岻������״̬������ϵͳ�����������������������ϱ� */
	/*if( devIdx == 1 )
	{
		if( (SYS_MODULE_IS_RUNNING(brdIdx) != TRUE) ||
			devsm_sys_is_switchhovering() )
		return VOS_OK;
	}*/		/* removed by xieshl 20080605 */
	return private_tdmlinkEvent_report( devIdx, brdIdx, tdmIdx, trap_tdmToEthLinkdown );
}

/* ����:    tdm ��Ӧ��̫���˿�linkup�¼��ϱ�
   �������:devIdx��OLT�豸����
   			brdIdx������(��λ��)����
			tdmIdx��tdm��Ӧ��һ·fpga
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
int tdmToEthLinkup_EventReport( ULONG devIdx, ULONG brdIdx, ULONG tdmIdx )	
{
	if(tdmIdx>3||tdmIdx<1)
		return VOS_OK;
	/* added 20070802 ���PON�岻������״̬������ϵͳ�����������������������ϱ� */
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
/* ˽��MIB����trap�¼���E1�˿�����¼��ϱ�  */
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
	�������������ĵ�5 ������uchar_t mask, ��ʹ��E1_ALM_OOS ������ʱ, 
	�������Ͳ�ƥ��, �轫uchar_t ��Ϊushort_t
	ͬʱ��Ҫ�޸ĵĻ���e1_Alarm_status[],e1_Alarm_status[],tdmService_Alarm_status[]������
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

/* ����:   e1���źŶ�ʧ�¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1LosAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1LosAlarm, E1_ALM_LOS );
}

/* ����:   e1���źŶ�ʧ������¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1LosAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1LosAlarmClear, E1_ALM_LOS );
}

/* ����:   e1���� ��ʧ�¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1LofAlarm_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx)
{
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1LofAlarm, E1_ALM_LOF );
}

/* ����:   e1���� ��ʧ����¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1LofAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1LofAlarmClear, E1_ALM_LOF );
}

/* ����:   e1�ڱ��˸澯ָʾ�¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
 int  e1AisAlarm_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx)
 {
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1AisAlarm, E1_ALM_AIS );
 }

/* ����:   e1�ڱ��˸澯ָʾ����¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1AisAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1AisAlarmClear, E1_ALM_AIS );
}

/* ����:   e1�ڶԶ˸澯ָʾ�¼�����
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
 int  e1RaiAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
 {
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1RaiAlarm, E1_ALM_RAI );
 }

/* ����:   e1�ڶԶ˸澯ָʾ����¼�����
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1RaiAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1RaiAlarmClear, E1_ALM_RAI );
}

/* ����:   e1�������ʧ���¼�����
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1SmfAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1SmfAlarm, E1_ALM_SMF );
}

/* ����:   e1�������ʧ������¼�����
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1SmfAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1SmfAlarmClear, E1_ALM_SMF );
}

/* ����:   e1��CRC����ʧ���¼�����
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1LomfAlarm_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx)
{ 
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1LomfAlarm, E1_ALM_LOFSMF );
}

/* ����:   e1�� CRC����ʧ������¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1LomfAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1LomfAlarmClear, E1_ALM_LOFSMF );
}

/* ����:   e1��CRC-3�����¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1Crc3Alarm_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx)
{
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1Crc3Alarm, E1_ALM_CRC3 );
}

/* ����:   e1�� CRC-3��������¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1Crc3AlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1Crc3AlarmClear, E1_ALM_CRC3 );
}

/* ����:   e1��CRC-6�����¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1Crc6Alarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
 	return e1AlarmEventReport( devIdx, brdIdx, portIdx, trap_e1Crc6Alarm, E1_ALM_CRC6 );
}

/* ����:   e1�� CRC-6��������¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			brdIdx��TDM�����ڵĲ�λ�ţ���Χ4~8
   			E1ClusterIdx-�������ؽ�ӿ���Ӱ����Χ1~3
			portIdx��ÿ�� �������ؽӽӿ��ϵ�E1��ţ���Χ1~8
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  e1Crc6AlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx)
{
	return e1AlarmClearEventReport( devIdx, brdIdx, portIdx, trap_e1Crc6AlarmClear, E1_ALM_CRC6 );
}


/* ˽��MIB����trap�¼���tdm����¼��ϱ�  */
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

/* ����:   ָ��ONU�� TDMҵ���ж��¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
 int  tdmServiceAbortAlarm_EventReport( ULONG devIdx)
 {
	return  private_tdmEvent_report( devIdx,  trap_tdmServiceAbortAlarm );
 }

/* ����:   ָ��ONU�� TDMҵ���ж�����¼��ϱ�
   �������:devIdx��OLT�豸������ȡֵΪ1
   			
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */	
 int  tdmServiceAbortAlarmClear_EventReport( ULONG devIdx)
 {   
	return  private_tdmEvent_report( devIdx,  trap_tdmServiceAbortAlarmClear);
 }

/* ����:   E1 ҵ���жϸ澯
   �������:devIdx��OLT�豸������ȡֵΪ1
   			
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */	
/*add by shixh@20090318*/
int  E1OutOfService_EventReport(ULONG  devIdx,ULONG brdIdx,ULONG portIdx)
{
	return e1AlarmEventReport( devIdx, brdIdx,portIdx, trap_E1OutOfService,E1_ALM_OOS);
}
/* ����:   E1 ҵ���жϸ澯
   �������:devIdx��OLT�豸������ȡֵΪ1
   			
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */	
 int  E1OutOfServiceClear_EventReport(ULONG  devIdx,ULONG brdIdx,ULONG portIdx)
{
	return  e1AlarmClearEventReport( devIdx,brdIdx, portIdx, trap_E1OutOfServiceClear,E1_ALM_OOS);
}
/*add by shixh@20080311*/
/*ONU�Զ������¼��ϱ�*/
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
/* ����:  onu�Զ����óɹ��¼��ϱ�
   �������:devIdx��ONU�豸
   			
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  onuLoadFileSuccess_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_onuAutoLoadConfigSuccess);
}

/* ����:  onu�Զ�����ʧ���¼��ϱ�
   �������:devIdx��ONU�豸
   			
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  onuLoadFileFailure_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_onuAutoLoadConfigFailure);
}
/* ����:  onu�Զ������ɹ��¼��ϱ�
   �������:devIdx��ONU�豸
   			
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */	
int  onuAutoLoadUpgradeSuccess_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_onuAutoLoadUpgradeSuccess);
}
/* ����:  onu�Զ�����ʧ���¼��ϱ�
   �������:devIdx��ONU�豸
   			
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */	
int  onuAutoLoadUpgradeFailure_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_onuAutoLoadUpgradeFailure);
}
#endif
/* ����:  onuϵͳ�ļ��ϴ��ɹ��¼��ϱ�
   �������:devIdx��ONU�豸
   			
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  onuSysFileUploadSuccess_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_sysfileUploadsuccess);
}
/* ����:  onuϵͳ�ļ��ϴ�ʧ���¼��ϱ�
   �������:devIdx��ONU�豸
   			
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  onuSysFileUploadFailure_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_sysfileUploadfailure);
}
/* ����:  onuϵͳ�ļ����سɹ��¼��ϱ�
   �������:devIdx��ONU�豸
   			
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
int  onuSysFileDownloadSuccess_EventReport( ULONG devIdx)
{
	return  private_devEvent_report(devIdx,trap_sysfileDownloadsuccess);
}
/* ����:  onuϵͳ�ļ�����ʧ���¼��ϱ�
   �������:devIdx��ONU�豸
   			
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */		 
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
/* ��׼�¼��ϱ� */
/*----------------------------------------------------------------------------*/

/* ����:    OLT��̫��STP�¼��ϱ�
   �������:stpEvent��2���˸ı��¼���1�����¼�
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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

/* ����:    OLT MIBII�¼��ϱ�
   �������:mib2Event��
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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
/* �����¼�����Ҫ���ڼ�¼��־�������ϱ�trap */
/*----------------------------------------------------------------------------*/
/* ����:    ONU��̫��STP�¼��ϱ�
   �������:onuDevIdx��ONU�豸����
   			stpEvent��1���˸ı��¼���2�����¼�
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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

/* ����:    �Լ��¼��ϱ�����Ҫ���ڼ�¼OLT�Լ���
   �������:result���Լ�����
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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
/* ����:   ����ͨ���¼��ϱ�����Ҫ���ڼ�¼���ع���ͨ���Ƿ��жϼ��ָ����
   �������:status��ͨ��״̬��0-��������0-�쳣���ж�
   ����ֵ:  �ɹ���VOS_OK������VOS_ERROR */
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
	 ����PON�˿ڰ汾�Ƿ�ƥ��澯
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

/* added by xieshl 20110117, ��¼����SNMP�������澯��־ */
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

