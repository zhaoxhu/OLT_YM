#ifndef __INCeventMainh
#define __INCeventMainh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void write_gpio(int GPIOx,int/*bit*/ a);


#define MAXLEN_EVENT_DATA	10 /*never change the value*/	

typedef struct {
	USHORT year;
	UCHAR  month;
	UCHAR  day;
	UCHAR  hour;
	UCHAR  minute;
	UCHAR  second;
	UCHAR  reserver[4];
}__attribute__((packed)) sysDateAndTime_t;

extern LONG  checkDateAndTime( struct vty *vty, sysDateAndTime_t* pTime );
extern VOID copyDateAndTime( sysDateAndTime_t* pDstTime, sysDateAndTime_t* pSrcTime );
extern int eventGetCurTime( sysDateAndTime_t *pDateTime );
extern int getTomorrowTime( sysDateAndTime_t *pDateTime, sysDateAndTime_t *pTomorrowTime );
extern sysDateAndTime_t * strToSysDateAndTime( char *pStr, sysDateAndTime_t *pDateTime );

/*size of any alarm src can not exceed MAXLEN_EVENT_DATA(10)*/
typedef union {
#define ALM_SRC_T_NULL		0
	struct {
		ULONG 	devIdx;
		ULONG 	devData;
	}__attribute__((packed)) devAlarmSrc;
#define ALM_SRC_T_DEV		1
	struct {
		ULONG 	devIdx;
		UCHAR	brdIdx;
		USHORT	brdType;
	}__attribute__((packed)) brdAlarmSrc;
#define ALM_SRC_T_BRD		2
	struct {
		ULONG 	devIdx;
		UCHAR	brdIdx;
		UCHAR	portIdx;
	}__attribute__((packed)) portAlarmSrc;
#define ALM_SRC_T_PORT		3
	struct {
		ULONG 	devIdx;
		UCHAR	brdIdx;
		UCHAR	fanIdx;
	}__attribute__((packed)) fanAlarmSrc;
#define ALM_SRC_T_FAN		4
	struct {
		ULONG 	devIdx;
		UCHAR	brdIdx;
		UCHAR	portIdx;
		LONG	monValue;
	}__attribute__((packed)) monAlarmSrc;
#define ALM_SRC_T_MON		5
	struct {
		ULONG 	devIdx;
		UCHAR	brdIdx;
		UCHAR	portIdx;
		UCHAR	llidIdx;
	}__attribute__((packed)) llidAlarmSrc;
#define ALM_SRC_T_LLID		6
	struct {
		UCHAR 	devIdx;		/* OLT DEV IDX */
		UCHAR	brdIdx;
		UCHAR	portIdx;
		UCHAR	data[6];
		UCHAR	onuIdx;
	}__attribute__((packed)) commAlarmSrc;
#define ALM_SRC_T_COMM		7
	/*add by shixh@20070926*/
/*	struct {
		ULONG 	devIdx;
		UCHAR	brdIdx;
		UCHAR	portIdx;
	}__attribute__((packed)) e1AlarmSrc;
#define ALM_SRC_T_E1		8*/
	/*add by shixh@20080202*/
	/*struct {
		ULONG 	devIdx;
		UCHAR	brdIdx;
		UCHAR	tdmIdx;
	}__attribute__((packed)) tdmAlarmSrc;
#define ALM_SRC_T_TDM		9*/
	/*add by shixh@20080831,just for oltReceiverPower*/
	struct {
		UCHAR	brdIdx;
		UCHAR	portIdx;
		ULONG	onuIdx;
		LONG	oltrxValue;
	}__attribute__((packed)) oltRxpowerAlarmSrc;
#define ALM_SRC_T_PON_PWR	10
	/*add by shixh@20090520*/
	struct {
		UCHAR	brdIdx;
		UCHAR	portIdx;
		ULONG	onuIdx;
		LONG	dbm;		/* unit 0.1dbm */
	}__attribute__((packed)) oltRxpowerAlwaysOnAlarmSrc;
#define ALM_SRC_T_ONU_LASER	11
	struct{
		ulong_t  devIdx;
		uchar_t  brdIdx;
		uchar_t  ethIdx;
		long      monValue;
	}__attribute__((packed)) uplinkAlarmSrc;
#define ALM_SRC_T_ETH_PWR		12
	struct {
		UCHAR	brdId	: 4;
		UCHAR	ponId	: 4;
		UCHAR	onuId;
		UCHAR	onuBrdId:3;
		UCHAR	onuPortId:5;
		UCHAR	reason;
		UCHAR	switchMacAddr[6];
         }__attribute__((packed)) onuSwitchAlarmSrc;  
#define ALM_SRC_T_ONU_SWITCH	13
	struct {
		ULONG 	devIdx;
		UCHAR	brdIdx;
		UCHAR	portIdx;
		LONG	result;
	}__attribute__((packed)) otherAlarmSrc;
#define ALM_SRC_T_OTHER		14
	struct {
		ULONG 	hashIdx;
		UCHAR	macAddr[6];
	}__attribute__((packed)) userTraceAlarmSrc;
#define ALM_SRC_T_USER_TRACE	15
	struct {
		UCHAR	devIdx;
		UCHAR	brdIdx;
		ULONG	ipAddr;	
		ULONG	udpPort;	
	}__attribute__((packed)) logicalSlotAlarmSrc;  
#define ALM_SRC_T_LOGICAL_SLOT	16	
	struct {
		UCHAR	brdIdx;
		UCHAR	ponIdx;
		UCHAR	partnerBrdIdx;
		UCHAR	partnerPonIdx;
		UCHAR	ipAddrWithPort[6];
	}__attribute__((packed)) ponSwitchAlarmSrc;  
#define ALM_SRC_T_PON_SWITCH	17	
	UCHAR  alarmSrcData[MAXLEN_EVENT_DATA];
#define ALM_SRC_T_MAX			18
}__attribute__((packed)) alarmSrc_t;


extern ULONG eventQueId;
extern ULONG eventSemId;

#define FC_EVENT_TIMER			101
#define FC_EVENT_REPORT		102
#define FC_EVENT_BACKUP		103
#define FC_EVENT_OAM_RX		201
#define FC_EVENT_ENV_CHK		301
#define FC_EVENT_ALM_SYNC		401
#define FC_EVENT_DATA_SYNC	402
#define FC_EVENT_CFG_SYNC		403
#define FC_EVENT_STA_CLS		501
#define FC_EVENT_CFG_SYNC_CTC   404 /*add by luh 2011-9-1*/
#define FC_EVENT_CFG_PON_TO_ONU     405 /*add by luh 2011-9-1*/
#define FC_EVENT_CFG_SYNC_ENABLE 407 /*add by luh 2011-9-21*/
#define FC_EVENT_CFG_SYNC_THRESHOLD 408 /*add by luh 2011-9-21*/
#define FC_EVENT_CFG_CTC_EVENT_HANDLE   409 /*add by liwei056@2014-4-1*/
typedef struct
{
	UCHAR 	alarmType;
	UCHAR 	alarmId;
	alarmSrc_t alarmSrc;
} eventMsg_t;


typedef struct {
ushort_t ponId;
ushort_t llId;
ushort_t  length;
UCHAR *pFrame;
UCHAR *pSessionField;
} __attribute__((packed))eventOamMsg_t;


/*add by shixh20091118*/
extern  ULONG onu_type_mask[V2R1_ONU_MAX];


#include "eventTrapBac.h"
#include "eventLog.h"


/*----------------------------------------------------------------------------*/
/*						  私有MIB trap定义									  */
/*----------------------------------------------------------------------------*/
/*ctc-alarm added by luh 2011-10-28*/
extern int ctcOnuEquipmentAlarm_EventReport( ULONG devIdx );
extern int ctcOnuEquipmentAlarmClear_EventReport( ULONG devIdx );

extern int ctcOnuPowerAlarm_EventReport( ULONG devIdx );
extern int ctcOnuPowerAlarmClear_EventReport( ULONG devIdx );

extern int ctcOnuBatteryMissing_EventReport( ULONG devIdx );	
extern int ctcOnuBatteryMissingClear_EventReport( ULONG devIdx );	

extern int ctcOnuBatteryFailure_EventReport( ULONG devIdx );
extern int ctcOnuBatteryFailureClear_EventReport( ULONG devIdx );

extern int ctcOnuBatteryVoltLow_EventReport( ULONG devIdx ,ULONG value);
extern int ctcOnuBatteryVoltLowClear_EventReport( ULONG devIdx ,ULONG value);

extern int ctcOnuTempLow_EventReport( ULONG devIdx ,ULONG value);
extern int ctcOnuTempLowClear_EventReport( ULONG devIdx ,ULONG value);

extern int ctcOnuTempHigh_EventReport( ULONG devIdx ,ULONG value);
extern int ctcOnuTempHighClear_EventReport( ULONG devIdx ,ULONG value);

extern int ctcOnuPhysicalIntrusionAlarm_EventReport( ULONG devIdx );
extern int ctcOnuPhysicalIntrusionAlarmClear_EventReport( ULONG devIdx );

extern int ctcOnuSelfTestFailure_EventReport( ULONG devIdx );
extern int ctcOnuSelfTestFailureClear_EventReport( ULONG devIdx );

extern int ctcOnuIADConnectionFailure_EventReport( ULONG devIdx );
extern int ctcOnuIADConnectionFailureClear_EventReport( ULONG devIdx );

extern int ctcOnuPonIfSwitch_EventReport( ULONG devIdx );
extern int ctcOnuPonIfSwitchClear_EventReport( ULONG devIdx );

/*ctc onu eth*/
extern int ethAutoNegFailure_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx );
extern int ethAutoNegFailureClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx );

extern int ethLos_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx );
extern int ethLosClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx );

extern int ethFailure_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx );
extern int ethFailureClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx );

extern int ethloopback_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx );
extern int ethloopbackClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx );

extern int ethCongestion_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx );
extern int ethCongestionClear_EventReport( ULONG devIdx,ULONG brdIdx,ULONG ethIdx );
/*deviceIndex, deviceType, deviceEntLogicalCommunity */
extern int onuNewRegSuccess_EventReport( ULONG devIdx );
extern int onuReregSuccess_EventReport( ULONG devIdx );

/* deviceIndex */
extern int onuNotPresent_EventReport( ULONG devIdx, ULONG reason );
extern int onuRegAuthFailure_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx, uchar_t *pOnuMacAddr );

extern int devPowerOff_EventReport( ULONG devIdx );
extern int devPowerOn_EventReport( ULONG devIdx );
extern int cfgDataSaveSuccess_EventReport( ULONG devIdx );
extern int cfgDataSaveFail_EventReport( ULONG devIdx );
extern int flashClearSuccess_EventReport( ULONG devIdx );
extern int flashClearFail_EventReport( ULONG devIdx );
extern int softwareUpdateSuccess_EventReport( ULONG devIdx );
extern int softwareUpdateFail_EventReport( ULONG devIdx );
extern int firmwareUpdateSuccess_EventReport( ULONG devIdx );
extern int firmwareUpdateFail_EventReport( ULONG devIdx );
extern int cfgDataBackupSuccess_EventReport( ULONG devIdx );
extern int cfgDataBackupFail_EventReport( ULONG devIdx );
extern int cfgDataRestoreSuccess_EventReport( ULONG devIdx );
extern int cfgDataRestoreFail_EventReport( ULONG devIdx );
extern int cpuUsageFactorHigh_EventReport( ULONG devIdx );
extern int dbaUpdateSuccess_EventReport( ULONG devIdx );
extern int dbaUpdateFailure_EventReport( ULONG devIdx );
extern int onuSoftwareLoadSuccess_EventReport( ULONG devIdx );
extern int onuSoftwareLoadFailure_EventReport( ULONG devIdx );

extern int bootUpdateSuccess_EventReport( ULONG devIdx );
extern int bootUpdateFailure_EventReport( ULONG devIdx );
extern int batfileBackupSuccess_EventReport( ULONG devIdx );
extern int batfileBackupFailure_EventReport( ULONG devIdx );
extern int batfileRestoreSuccess_EventReport( ULONG devIdx );
extern int batfileRestoreFailure_EventReport( ULONG devIdx );

extern int ponPortFull_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern int ponPortFullClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
/*----------------------------------------------------------------------------*/

/* deviceIndex, boardIndex */
extern int swBoardProtectedSwitch_EventReport( ULONG devIdx, ULONG brdIdx );
extern int boardTemperatureHigh_EventReport( ULONG devIdx, ULONG brdIdx, LONG tempVal );
extern int boardTemperatureHighClear_EventReport( ULONG devIdx, ULONG brdIdx, LONG tempVal );
extern int ponBoardReset_EventReport( ULONG devIdx, ULONG brdIdx );

/* deviceIndex, boardIndex, curBoardType */
extern int devBoardInterted_EventReport( ULONG devIdx, ULONG brdIdx, ULONG brdType );
extern int devBoardPull_EventReport( ULONG devIdx, ULONG brdIdx, ULONG brdType );

/* deviceIndex, boardIndex */
extern int powerOffAlarm_EventReport( ULONG devIdx, ULONG brdIdx );
extern int powerOnAlarm_EventReport( ULONG devIdx, ULONG brdIdx );

/* deviceIndex, devFanIndex */
extern int devFanAlarm_EventReport( ULONG devIdx, ULONG fan_id );
extern int devFanAlarmClear_EventReport( ULONG devIdx, ULONG fan_id );

extern int deviceTemperatureHigh_EventReport( ULONG devIdx, LONG temperature, LONG threshold );
extern int deviceTemperatureHighClear_EventReport( ULONG devIdx, LONG temperature, LONG threshold );
extern int deviceTemperatureLow_EventReport( ULONG devIdx, LONG temperature, LONG threshold );
extern int deviceTemperatureLowClear_EventReport( ULONG devIdx, LONG temperature, LONG threshold );

/*----------------------------------------------------------------------------*/

/* deviceIndex, ponPortBrdIndex, ponPortIndex, ponPortBER */
extern int ponPortBERAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG ber );
extern int ponPortBERAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG ber );
/* deviceIndex, ponPortBrdIndex, ponPortIndex, ponPortFER */
extern int ponPortFERAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG fer );
extern int ponPortFERAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, ULONG fer );

/*----------------------------------------------------------------------------*/

/* deviceIndex, ponPortBrdIndex, ponPortIndex, ponLlidIndex */
extern int llidActBWExceeding_EventReport( ULONG devIdx, ULONG brdIdx, 
		ULONG ponIdx, ULONG llidIdx );
extern int llidActBWExceedingClear_EventReport( ULONG devIdx, ULONG brdIdx, 
		ULONG ponIdx, ULONG llidIdx );

/*----------------------------------------------------------------------------*/

/* deviceIndex, boardIndex, ponPortIndex */
extern int autoProtectSwitch_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx );
extern int ponPortAbnormal_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx, long resetCode );
extern int ponPortAbnormalClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx );
extern int onuRegisterConflict_EventReport( ULONG devIdx/*, ULONG brdIdx, ULONG ponIdx*/ );
extern int firmwareLoadSuccess_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx );
extern int firmwareLoadFailure_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx );
extern int dbaLoadSuccess_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx );
extern int dbaLoadFailure_EventReport( ULONG devIdx, ULONG brdIdx, ULONG ponIdx );

/* devIndex, boardIndex, portIndex */
extern int ethFlrAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern int ethFlrAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern int ethFerAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern int ethFerAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern int ethTranmittalIntermitAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern int ethTranmittalIntermitAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern int ethLinkdown_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern int ethLinkup_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );

/* added bu xieshl 20070703 */
extern int deviceColdStart_EventReport( ULONG devIdx );
extern int deviceWarmStart_EventReport( ULONG devIdx );
extern int deviceExceptionRestart_EventReport( ULONG devIdx );
extern int ponToEthLinkdown_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern int ponToEthLinkup_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx )	;
/* end 20070703 */

/*----------------------------------------------------------------------------*/

extern int onuStp_EventReport( ULONG onuDevIdx, ULONG stpEvent );
extern int diagnosis_EventReport( ULONG result );
/*add by shixh@20071015*/
extern STATUS  GetE1AlarmLevel(ULONG  devIdx,ULONG  *level);
extern STATUS GetTdmServiceAbortAlarmLevel(ULONG  devIdx ,ULONG *level);
/*extern STATUS SetE1AlarmLevel(ULONG E1_alarm,ULONG  level);*/
/*end add by shixh@20071015*/
extern int e1AlarmClearEventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG alarmId, ushort_t mask );
extern int e1AlarmEventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG alarmId, ushort_t mask );

/*add by shixh@20071009*/
extern  int  e1LosAlarm_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx);
extern  int  e1LosAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern  int  e1LofAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern  int  e1LofAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern  int  e1AisAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern   int  e1AisAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern   int  e1RaiAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern   int  e1RaiAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern  int  e1SmfAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern  int  e1SmfAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx);
extern  int  e1LomfAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern  int  e1LomfAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern  int  e1Crc3Alarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern   int  e1Crc3AlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern   int  e1Crc6Alarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx);
extern   int  e1Crc6AlarmClear_EventReport( ULONG devIdx, ULONG brdIdx,ULONG portIdx);

extern   int  tdmServiceAbortAlarm_EventReport( ULONG devIdx);
extern   int  tdmServiceAbortAlarmClear_EventReport( ULONG devIdx);
/*end add by shixh@20071009*/

/*added by xieshl 20080116*/
extern   int ethLoopAlarm_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern   int ethLoopAlarmClear_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern   int onuLoopAlarm_EventReport( ULONG devIdx );
extern   int onuLoopAlarmClear_EventReport( ULONG devIdx );
/* end 20080116 */

/*added by shixh@20080215*/
extern  int backboneEthLinkdown_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
extern  int backboneEthLinkup_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx );
/* end 20080215 */

/*added by shixh@20080202*/
extern int tdmToEthLinkdown_EventReport( ULONG devIdx, ULONG brdIdx, ULONG tdmIdx );
extern int tdmToEthLinkup_EventReport( ULONG devIdx, ULONG brdIdx, ULONG tdmIdx );

/*add by shixh20090609*/
extern  int  onuSysFileUploadSuccess_EventReport( ULONG devIdx);
extern   int  onuSysFileUploadFailure_EventReport( ULONG devIdx);
extern    int  onuSysFileDownloadSuccess_EventReport( ULONG devIdx);
extern    int  onuSysFileDownloadFailure_EventReport( ULONG devIdx);

extern int  onuLoadFileSuccess_EventReport( ULONG devIdx);
extern int  onuLoadFileFailure_EventReport( ULONG devIdx);
extern int  onuAutoLoadUpgradeSuccess_EventReport( ULONG devIdx);
extern int  onuAutoLoadUpgradeFailure_EventReport( ULONG devIdx);

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
extern int tdmResetAlarm_EventReport();
#endif

extern int oltStp_EventReport( ULONG stpEvent );
extern int oltMib2_EventReport( ULONG mib2Event );

/* begin: added by jianght 20090410  */
extern int  E1OutOfService_EventReport(ULONG  devIdx,ULONG brdIdx,ULONG portIdx);
extern int  E1OutOfServiceClear_EventReport(ULONG  devIdx,ULONG brdIdx,ULONG portIdx);
#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
extern int  PWUPowerOff_EventReport(ULONG devIdx, ULONG brdIdx, ULONG brdType);
extern int  PWUPowerOn_EventReport(ULONG devIdx, ULONG brdIdx, ULONG brdType);
#endif
/* end: added by jianght 20090410 */
extern int ponBRASAlarm_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx, uchar_t *pMacAddr );
extern int ponBRASAlarmClear_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx, uchar_t *pMacAddr );

extern int cpuCtrlChan_EventReport( ULONG alarmId );

extern int UplinkSFPRecvPowerLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long rxPower);
extern int UplinkSFPRecvPowerLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long rxPower);
extern int UplinkSFPRecvPowerHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long rxPower);
extern int UplinkSFPRecvPowerHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long rxPower);
extern int UplinkSFPTransPowerLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long txPower);
extern int UplinkSFPTransPowerLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long txPower);
extern int UplinkSFPTransPowerHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long txPower);
extern int UplinkSFPTransPowerHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long txPower);
extern int UplinkSFPVoltageLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long voltage);
extern int UplinkSFPVoltageLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long voltage);
extern int UplinkSFPVoltageHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long voltage);
extern int UplinkSFPVoltageHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long voltage);
extern int UplinkSFPBiasCurrentLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long current);
extern int UplinkSFPBiasCurrentLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long current);
extern int UplinkSFPBiasCurrentHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long current);
extern int UplinkSFPBiasCurrentHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long current);
extern int UplinkSFPTemperatureLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long temperature);
extern int UplinkSFPTemperatureLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long temperature);
extern int UplinkSFPTemperatureHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long temperature);
extern int UplinkSFPTemperatureHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long temperature);

extern int ponPortLaserAlwaysOn_EventReport( ulong_t brdIdx, ulong_t portIdx, ulong_t onudevidx, LONG dbm);
extern int ponPortLaserAlwaysOnClear_EventReport( ulong_t brdIdx, ulong_t portIdx, ulong_t onudevidx, LONG dbm);	

/*----------------------------------------------------------------------------*/

/* 删除指定ONU/OLT设备的告警日志和告警同步信息 */
extern int eraseDevEventRecords( ULONG devIdx );
extern int PonPortOther_EventReport( ULONG slot, ULONG port,  ULONG AlarmId);

extern void sendOltPowerOffTrap( ULONG devIdx );
extern int cpuUsageHigh_EventReport( ULONG devIdx ,ULONG brdIdx);
extern int cpuUsageHighClear_EventReport( ULONG devIdx ,ULONG brdIdx);
extern int boardMemoryUsageHigh_EventReport( ULONG devIdx ,ULONG brdIdx);
extern int boardMemoryUsageHighClear_EventReport( ULONG devIdx ,ULONG brdIdx);


extern int snmp_Nms_Restart_Dev_EventReport( ULONG devIdx, LONG result );
extern int snmp_Nms_Reset_Board_EventReport( ULONG devIdx, ULONG brdIdx, LONG result );
extern int snmp_Nms_Reset_Olt_EventReport( ULONG devIdx, ULONG brdIdx, ULONG portIdx, LONG result );
extern int snmp_Nms_Save_Config_EventReport( ULONG devIdx, LONG result );
extern int snmp_Nms_Erase_Config_EventReport( ULONG devIdx, LONG result );

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/


#define EVENT_DEBUGSWITCH_NONE		0x00
#define EVENT_DEBUGSWITCH_LOG		0x01	/* debug alarm log */
#define EVENT_DEBUGSWITCH_OAM		0x02	/* debug alarm oam */
#define EVENT_DEBUGSWITCH_SYN		0x04	/* debug alarm synchronization */
#define EVENT_DEBUGSWITCH_TRAP		0x08	/* debug alarm trap */
#define EVENT_DEBUGSWITCH_STA		0x10	/* debug current alarm status */
#define EVENT_DEBUGSWITCH_ALL			(EVENT_DEBUGSWITCH_LOG | EVENT_DEBUGSWITCH_SYN |\
										 EVENT_DEBUGSWITCH_OAM | EVENT_DEBUGSWITCH_TRAP |\
										 EVENT_DEBUGSWITCH_STA )

#define EVENT_LOG_OUT_2_NONE			0x00
#define EVENT_LOG_OUT_2_CONSOLE		0x01
#define EVENT_LOG_OUT_2_TELNET		0x02
#define EVENT_LOG_OUT_2_SYSLOG		0x04
#define EVENT_LOG_OUT_2_ALL			(EVENT_LOG_OUT_2_CONSOLE | EVENT_LOG_OUT_2_TELNET | EVENT_LOG_OUT_2_SYSLOG)


/* 基于设备告警屏蔽 */
#define EVENT_MASK_DEV_POWER			0x80000000
#define EVENT_MASK_DEV_FAN			0x40000000
#define EVENT_MASK_DEV_CPU			0x20000000
#define EVENT_MASK_DEV_TEMPERATURE	0x10000000
#define EVENT_MASK_DEV_REGISTER		0x08000000
#define EVENT_MASK_DEV_PRESENT		0x04000000
/*add by shixh20091204*//*device_MIB的ONU告警屏蔽项扩展了13项*/
#define EVENT_MASK_DEV_ETH_LINK		0x02000000
#define EVENT_MASK_DEV_ETH_FER		0x01000000
#define EVENT_MASK_DEV_ETH_FLR		0x00800000
#define EVENT_MASK_DEV_ETH_TI		0x00400000
#define EVENT_MASK_DEV_ETH_LOOP		0x00200000
#define EVENT_MASK_DEV_PON_BER		0x00100000
#define EVENT_MASK_DEV_PON_FER		0x00080000
#define EVENT_MASK_DEV_PON_ABNORMAL	0x00040000
#define EVENT_MASK_DEV_PON_ABS		0x00020000
#define EVENT_MASK_DEV_PON_LINK		0x00010000
#define EVENT_MASK_DEV_ONU_LASER_ON	0x00008000
#define EVENT_MASK_DEV_PON_POWER_L	0x00004000
#define EVENT_MASK_DEV_PON_POWER_H	0x00002000
#define EVENT_MASK_DEV_PON_POWER	(EVENT_MASK_DEV_PON_POWER_L|EVENT_MASK_DEV_PON_POWER_H)
#define EVENT_MASK_DEV_PON_LOS		0x00001000
#define EVENT_MASK_DEV_PWU_STATUS   0x00000800
#define EVENT_MASK_DEV_ALL			( EVENT_MASK_DEV_POWER | EVENT_MASK_DEV_FAN |EVENT_MASK_DEV_CPU | EVENT_MASK_DEV_TEMPERATURE |\
										EVENT_MASK_DEV_REGISTER | EVENT_MASK_DEV_PRESENT | \
										EVENT_MASK_DEV_ETH_LINK | EVENT_MASK_DEV_ETH_FER | EVENT_MASK_DEV_ETH_FLR | EVENT_MASK_DEV_ETH_TI | EVENT_MASK_DEV_ETH_LOOP | \
										EVENT_MASK_DEV_PON_BER |EVENT_MASK_DEV_PON_FER | \
										EVENT_MASK_DEV_PON_ABNORMAL | EVENT_MASK_DEV_PON_ABS |EVENT_MASK_DEV_PON_LINK |\
										EVENT_MASK_DEV_ONU_LASER_ON | EVENT_MASK_DEV_PON_POWER |\
										EVENT_MASK_DEV_PON_LOS | EVENT_MASK_DEV_PWU_STATUS)


/* 基于PON 口告警屏蔽 */
#define EVENT_MASK_PON_BER			0x80000000
#define EVENT_MASK_PON_FER			0x40000000
#define EVENT_MASK_PON_ABNORMAL		0x20000000
#define EVENT_MASK_PON_APS			0x10000000
#define EVENT_MASK_PON_LINK			0x08000000
#define EVENT_MASK_PON_LASER_ON		0x04000000
#define EVENT_MASK_PON_POWER_LOW	0x02000000
#define EVENT_MASK_PON_POWER_HIGH	0x01000000
#define EVENT_MASK_PON_POWER			(EVENT_MASK_PON_POWER_LOW|EVENT_MASK_PON_POWER_HIGH)
#define EVENT_MASK_PON_LOS   			0x00800000
#define EVENT_MASK_PON_TEMPERATURE	0x00400000
#define EVENT_MASK_PON_BIAS_CURRENT	0x00200000
#define EVENT_MASK_PON_VOLTAGE		0x00100000
#define EVENT_MASK_PON_ALL		( EVENT_MASK_PON_BER | EVENT_MASK_PON_FER | EVENT_MASK_PON_ABNORMAL | EVENT_MASK_PON_APS | \
										EVENT_MASK_PON_LINK | EVENT_MASK_PON_LASER_ON | EVENT_MASK_PON_POWER_LOW | \
										EVENT_MASK_PON_POWER_HIGH	 | EVENT_MASK_PON_LOS | EVENT_MASK_PON_TEMPERATURE | \
										EVENT_MASK_PON_BIAS_CURRENT | EVENT_MASK_PON_VOLTAGE )

/* 基于以太网端口告警屏蔽 */
/*#define EVENT_MASK_ETH_ALL			0xf0000000*/
#define EVENT_MASK_ETH_LINK			0x80000000
#define EVENT_MASK_ETH_FER			0x40000000
#define EVENT_MASK_ETH_FLR			0x20000000
#define EVENT_MASK_ETH_TI				0x10000000
#define EVENT_MASK_ETH_LOOP			0x08000000
#define EVENT_MASK_ETH_BCFC			0x04000000
#define EVENT_MASK_ETH_ALL			(EVENT_MASK_ETH_LINK | EVENT_MASK_ETH_FER | EVENT_MASK_ETH_FLR | EVENT_MASK_ETH_TI | EVENT_MASK_ETH_LOOP |\
										EVENT_MASK_ETH_BCFC )

#define EVENT_MAST_ONU_SWITCH_REGISTER      0x80000000
#define EVENT_MAST_ONU_SWITCH_DEREGISTER    0x40000000
#define EVENT_MAST_ONU_SWITCH_ALL           (EVENT_MAST_ONU_SWITCH_REGISTER|EVENT_MAST_ONU_SWITCH_DEREGISTER)

#if 0
/*add byshixh20091021*/
#define EVENT_MASK_ONU_POWER                0x80000000
#define EVENT_MASK_ONU_FAN               	 0x40000000
#define EVENT_MASK_ONU_CPU                     0x20000000
#define EVENT_MASK_ONU_TEMPERATURE    0x10000000
#define EVENT_MASK_ONU_REGISTER           0x08000000
#define EVENT_MASK_ONU_PRESENT             0x04000000
#define EVENT_MASK_ONU_ETH_LINK           0x02000000
#define EVENT_MASK_ONU_ETH_FER           	0x01000000
#define EVENT_MASK_ONU_ETH_FLR            0x00800000
#define EVENT_MASK_ONU_ETH_TI            	0x00400000
#define EVENT_MASK_ONU_ETH_LOOP          0x00200000
#define EVENT_MASK_ONU_PON_BER           	0x00100000
#define EVENT_MASK_ONU_PON_FER            0x00080000
#define EVENT_MASK_ONU_PON_ABNORMAL 0x00040000
#define EVENT_MASK_ONU_PON_APS            0x00020000
#define EVENT_MASK_ONU_PON_LINK           0x00010000
#define EVENT_MASK_ONU_LASER_ALWAYS_ON    	0x00008000
#define EVENT_MASK_ONU_OPTICAL_POWER_LOW  	0x00004000
#define EVENT_MASK_ONU_OPTICAL_POWER_HIGH  	0x00002000
#define EVENT_MASK_ONU_PON_LOS            0x00001000
#define EVENT_MASK_ONU_OPTICAL_POWER	(EVENT_MASK_ONU_OPTICAL_POWER_LOW |EVENT_MASK_ONU_OPTICAL_POWER_HIGH)
#define EVENT_MASK_ONU_ALL                     (EVENT_MASK_ONU_POWER|EVENT_MASK_ONU_FAN|\
										EVENT_MASK_ONU_CPU|EVENT_MASK_ONU_TEMPERATURE|\
										EVENT_MASK_ONU_REGISTER|EVENT_MASK_ONU_PRESENT|\
										EVENT_MASK_ONU_ETH_LINK|EVENT_MASK_ONU_ETH_FER|\
										EVENT_MASK_ONU_ETH_FLR|\
										EVENT_MASK_ONU_ETH_TI|EVENT_MASK_ONU_ETH_LOOP|\
										EVENT_MASK_ONU_PON_BER|EVENT_MASK_ONU_PON_FER|\
										EVENT_MASK_ONU_PON_ABNORMAL|EVENT_MASK_ONU_PON_APS|\
										EVENT_MASK_ONU_PON_LINK|EVENT_MASK_ONU_LASER_ALWAYS_ON|\
										EVENT_MASK_ONU_OPTICAL_POWER_LOW|EVENT_MASK_ONU_OPTICAL_POWER_HIGH |\
										EVENT_MASK_ONU_PON_LOS)
#endif

/*add by shixh@20071226*/
#define E1_ALM_DEF		0
#define E1_ALM_ALL 		0xFF

#define  E1_ALM_OOS        0x0100
#define E1_ALM_LOS 		0x80
#define E1_ALM_LOF 		0x40
#define E1_ALM_AIS		0x20
#define E1_ALM_RAI		0x10
#define E1_ALM_SMF 		0x08
#define E1_ALM_LOFSMF	0x04
#define E1_ALM_CRC3 	0x02
#define E1_ALM_CRC6 	0x01

/* modified by xieshl 20080408 */
#define TDM_BASE_ALM_LOS		(E1_ALM_LOS << 8)
#define TDM_BASE_ALM_LOF		(E1_ALM_LOF << 8)
#define TDM_BASE_ALM_AIS		(E1_ALM_AIS << 8)
#define TDM_BASE_ALM_RAI		(E1_ALM_RAI << 8)
#define TDM_BASE_ALM_SMF		(E1_ALM_SMF << 8)
#define TDM_BASE_ALM_LOFSMF	(E1_ALM_LOFSMF << 8)
#define TDM_BASE_ALM_CRC3		(E1_ALM_CRC3 << 8)
#define TDM_BASE_ALM_CRC6		(E1_ALM_CRC6 << 8)
#define TDM_BASE_ALM_E1		(E1_ALM_ALL << 8)
#define TDM_BASE_ALM_OOS		0x0080
#define TDM_BASE_ALM_ALL		(TDM_BASE_ALM_E1|TDM_BASE_ALM_OOS)


#define EVENT_MSG_SUBTYPE_ALARM_LOG		1
#define EVENT_MSG_SUBTYPE_TRAP_BAC		2
#define EVENT_MSG_SUBTYPE_ALARM_MASK	3
#define EVENT_MSG_SUBTYPE_ALARM_STATUS	4

typedef  struct{
	UCHAR subType;
	UCHAR subCode;
	UCHAR extData[30];
} __attribute__((packed)) eventSyncCfgData_t;


#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
extern int  MAX_E1_PORT_NUM;
extern STATUS GetE1AlarmMask(ULONG idx,uchar * pvalue);
extern STATUS SetE1AlarmMask(ULONG idx[3],uchar value);
extern STATUS GetTdmMaskBase(ULONG * pvar);
extern STATUS SetTdmMaskBase(USHORT mask_type, USHORT value);
extern STATUS tdmBoardAlarmStatus_update( ULONG slotno );
#endif

extern LONG getDeviceAlarmTopLevel( const ULONG devIdx );
extern LONG getDeviceAllAlarmLevel( const ULONG devIdx, ULONG *pVital, ULONG *pMajor, ULONG *pMinor );
extern LONG getEthPortAlarmStatusBitList( const ULONG devIdx, ULONG brdIdx, ULONG ethIdx );
extern LONG getPonPortAlarmStatusBitList( const ULONG devIdx, ULONG brdIdx, ULONG ponIdx );
extern LONG clearDeviceAlarmStatus( ULONG devIdx );
extern LONG clearOltBrdAlarmStatus( ULONG devIdx, ULONG brdIdx );
extern LONG clearOltPortAlarmStatus( ULONG devIdx, ULONG brdIdx, ULONG portIdx );


extern ULONG eventDebugSwitch;
#define eventDbgPrintf(lvl, _x) if (lvl & eventDebugSwitch) {sys_console_printf _x;}

#if 0
#ifndef opt_abs
#define opt_abs(a) (((a) >= 0) ? (a) : -(a))
#endif /* abs */
#define decimal2_integer_part(val)	(val/10)		/* added by xieshl 20091105, 修改小数显示格式 */
#define decimal2_fraction_part(val)	opt_abs(val%10)
#endif

/*modified by luh 2012-9-27 ，增加检查效率*/
#if 1
#define MAC_ADDR_IS_ZERO(mac) ((*(ULONG*) mac == 0) && (*(USHORT*) (&mac[4]) == 0))
#define MAC_ADDR_IS_EQUAL(X, Y) ((*(ULONG*) X == *(ULONG*) Y) && (*(USHORT*) (&X[4]) == *(USHORT*) (&Y[4])))
#define MAC_ADDR_IS_UNEQUAL(X, Y) ((*(ULONG*) X != *(ULONG*) Y) || (*(USHORT*) (&X[4]) != *(USHORT*) (&Y[4])))
#else
#define MAC_ADDR_IS_ZERO(mac) ((mac[0]|mac[1]|mac[2]|mac[3]|mac[4]|mac[5]) == 0)
#define MAC_ADDR_IS_EQUAL(X, Y) ((X[5] == Y[5]) && (X[4] == Y[4]) && (X[3] == Y[3]) && (X[2] == Y[2]) && (X[1] == Y[1]) && (X[0] == Y[0]))
#define MAC_ADDR_IS_UNEQUAL(X, Y) ((X[5] != Y[5]) || (X[4] != Y[4]) || (X[3] != Y[3]) || (X[2] != Y[2]) || (X[1] != Y[1]) || (X[0] != Y[0]))
#endif

/* added by xieshl 20110503 */
#define MAC_ADDR_IS_BROADCAST(mac) (mac[0] == 0xff)
#define MAC_ADDR_IS_UNICAST(mac) ( (mac[0] & 0x01) == 0 )
#define MAC_ADDR_IS_MULTICAST(mac) ( (mac[0] & 0x01) == 0x01 )
#define MAC_ADDR_IS_INVALID(mac) ( MAC_ADDR_IS_MULTICAST(mac) || MAC_ADDR_IS_ZERO(mac) )
/* end 20110503 */

#ifndef OLT_DEV_ID
#define OLT_DEV_ID 1
#endif
/*#ifndef MAKEDEVID
#define MAKEDEVID(x,y,z)    ((x)*10000+(y)*1000+(z))
#define GET_PONSLOT(x)  (x/10000)
#define GET_PONPORT(x)  ((x%10000)/1000)
#define GET_ONUID(x)       (x%1000)
#endif*/

#ifndef MAKEDEVID
#define NEW_DEVICE_IDX_FLAG	(1<<24)
#define NEW_DEVICE_IDX_IS(x)	(NEW_DEVICE_IDX_FLAG & (x))
#define MAKEDEVID(x,y,z)    (NEW_DEVICE_IDX_FLAG|((x)<<16)|((y)<<8)|(z))
#define GET_PONSLOT(x)      (((x)>>16)&0xff)
#define GET_PONPORT(x)      (((x)>>8)&0xff)
#define GET_ONUID(x)          ((x)&0xff)
#endif


#define OLT_PWU_OFF_TRAP

extern ULONG FANID2SLOTNO( ULONG fan_id );
extern ULONG FANID2FANNO( ULONG fan_id );
extern ULONG FANSLOT2ID( ULONG slotno, ULONG fanno );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCeventMainh */
