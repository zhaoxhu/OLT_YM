#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include "cli/cli.h"
#include "Vos_time.h"
#include "Vos_int.h"

#include "../../mib/gwEponSys.h"
#include "eventMain.h"
#include "../../bsp/epon3/GW405EP.h"

#define RTC_TYPE_NULL			0
#define RTC_TYPE_DS12C887		1
#define RTC_TYPE_DS1746		2

/* wangysh modify for 6900 NVRAM_BASEADDR 从bsp获得*/
#if 0
#define  NVRAM_BASEADDR					0xfa000000/*0xffe00000*/
#else
#define NVRAM_BASEADDR NVRAM_START_ADRS
#endif

/* modified by xieshl 20130318, 自动识别保存nvram或flash */
static UCHAR *pNvramEventDataBuff = (UCHAR *)(NVRAM_BASEADDR + 0xc000);

#define  NVRAM_IMPORTLOG_SIZE				0x4000		/* 16k */
#define  NVRAM_IMPORTLOG_BASE_ADDR		pNvramEventDataBuff	/*(NVRAM_BASEADDR + 0xc000)*/
#define  NVRAM_IMPORTLOG_HEAD_ADDR		NVRAM_IMPORTLOG_BASE_ADDR
#define  NVRAM_IMPORTLOG_DATA_ADDR		(NVRAM_IMPORTLOG_BASE_ADDR + sizeof(nvramEventLogHead_t))
#define  NVRAM_IMPORTLOG_DATA_LEN		(NVRAM_IMPORTLOG_SIZE - sizeof(nvramEventLogHead_t))		/* 0x13ff0 = 16k */
#define  NVRAM_IMPORTLOG_MAXNUM			(NVRAM_IMPORTLOG_DATA_LEN /sizeof(nvramEventLogData_t))		/* 支持告警日志最大行数 */

#define  NVRAM_EVENTLOG_SIZE				0x10000		/* 64k */
#define  NVRAM_EVENTLOG_BASE_ADDR		(NVRAM_IMPORTLOG_BASE_ADDR + NVRAM_IMPORTLOG_SIZE)
#define  NVRAM_EVENTLOG_HEAD_ADDR		NVRAM_EVENTLOG_BASE_ADDR
#define  NVRAM_EVENTLOG_DATA_ADDR		(NVRAM_EVENTLOG_BASE_ADDR + sizeof(nvramEventLogHead_t))
#define  NVRAM_EVENTLOG_MAXNUM			(NVRAM_EVENTLOG_SIZE / sizeof(nvramEventLogData_t))		/* 支持告警日志最大行数 */

ULONG  NVRAM_EVENTLOG_DATA_LEN = (NVRAM_EVENTLOG_SIZE - sizeof(nvramEventLogHead_t));


#define  NVRAM_EVENTBACKUP_LEN			(NVRAM_IMPORTLOG_SIZE + NVRAM_EVENTLOG_SIZE)
#define  NVRAM_EVENTBACKUP_PERIOD_DEF	(60 * 60/* * 6*/)/* 60 min = 1 hour modified by yangts 2014-12-26*/
#define  NVRAM_EVENTBACKUP_PERIOD_MIN	(60 * 10)			/* 10 min */
#define  NVRAM_EVENTBACKUP_PERIOD_MAX	1000000		/* 1000000 min */
CHAR *eventLogFlashFileName = NULL;
ULONG eventLogFlashFileFlag = 0;
ULONG eventLogFlashFileEnable = 1;
ULONG eventLogFlashFilePeriod = NVRAM_EVENTBACKUP_PERIOD_DEF;
ULONG eventLogFlashFileTimer = 0;


#define EVENTLOG_FLAG		0x55555555

/*----------------------------------------------------------------------------*/

static eventInfoMapTbl_t mib2_EventLog_MapInfo[] = 
{
	{trap_mib2_min,			ALM_LEV_NULL,		ALM_PRI_LOW,	ALM_SYN_NO,		0,	ALM_SRC_T_NULL,		""},
	{trap_coldStart,			ALM_LEV_NULL,		ALM_PRI_LOW,	ALM_SYN_INS,	0,	ALM_SRC_T_OTHER,	"OLT cold start"},
	{trap_warmStart,			ALM_LEV_NULL,		ALM_PRI_LOW,	ALM_SYN_INS,	0,	ALM_SRC_T_OTHER,	"OLT warm start"},
	{trap_linkDown,			ALM_LEV_NULL,		ALM_PRI_LOW,	ALM_SYN_NO,		0,	ALM_SRC_T_OTHER,	"link-down"},		/* deprecated */
	{trap_linkUp,				ALM_LEV_NULL,		ALM_PRI_LOW,	ALM_SYN_NO,		0,	ALM_SRC_T_OTHER,	"link-up"},		/* deprecated */
	{trap_authenticationFailure,	ALM_LEV_NULL,		ALM_PRI_LOW,	ALM_SYN_NO,		0,	ALM_SRC_T_OTHER,	"SNMP authentication failure"},
	{trap_mib2_max,			ALM_LEV_NULL,		ALM_PRI_LOW,	ALM_SYN_NO,		0,	ALM_SRC_T_NULL,		""},
};

static eventInfoMapTbl_t mib4_EventLog_MapInfo[] = 
{
	{trap_cmcctrl_min,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,	                ALM_SRC_T_NULL,	""},
	{trap_cmcArrival,       ALM_LEV_MINOR,	ALM_PRI_LOW,	ALM_SYN_INS,	0,					ALM_SRC_T_DEV,	"cmc is arrival"},
	{trap_cmcReady,         ALM_LEV_MINOR,	ALM_PRI_LOW,	ALM_SYN_INS,	0,					ALM_SRC_T_DEV,	"cmc is ready"},
	{trap_cmcReady,         ALM_LEV_MINOR,	ALM_PRI_LOW,	ALM_SYN_INS,	0,					ALM_SRC_T_DEV,	"cmc is ready"},
	{trap_cmArrival,        ALM_LEV_MINOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_cmDeparture,	ALM_SRC_T_DEV,	"cm is arrival"},
	{trap_cmReady,          ALM_LEV_MINOR,	ALM_PRI_LOW,	ALM_SYN_INS,	0,					ALM_SRC_T_DEV,	"cm is ready"},
	{trap_cmDeparture,      ALM_LEV_MINOR,  ALM_PRI_LOW,	ALM_SYN_INS,	trap_cmArrival,		ALM_SRC_T_DEV,	"cm is depart"},
	{trap_cmcctrl_max,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,	                ALM_SRC_T_NULL,	""},
};

static eventInfoMapTbl_t bridge_EventLog_MapInfo[] = 
{
	{trap_bridge_min,			ALM_LEV_NULL,		ALM_PRI_LOW,	ALM_SYN_NO,		0,	ALM_SRC_T_NULL,		""},
	{trap_topologyChange,		ALM_LEV_NULL,		ALM_PRI_LOW,	ALM_SYN_NO,		0,	ALM_SRC_T_DEV,		"STP topology change"},
	{trap_newRoot,			ALM_LEV_NULL,		ALM_PRI_LOW,	ALM_SYN_NO,		0,	ALM_SRC_T_DEV,		"STP new root"},
	{trap_bridge_max,			ALM_LEV_NULL,		ALM_PRI_LOW,	ALM_SYN_NO,		0,	ALM_SRC_T_NULL,		""},
};

static eventInfoMapTbl_t private_EventLog_MapInfo[] = 
{
	{trap_private_min,				ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_NULL,		""},
	{trap_onuNewRegSuccess,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_INS,	0,						ALM_SRC_T_DEV,		"onu first register"},
	{trap_onuReregSuccess,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_onuNotPresent,		ALM_SRC_T_DEV,		"onu re-register"},
	{trap_onuNotPresent,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_onuReregSuccess,		ALM_SRC_T_DEV,		"onu not present:"},
	{trap_devPowerOff,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_devPowerOn,			ALM_SRC_T_DEV,		"device power off"},
	{trap_devPowerOn,			ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_devPowerOff,			ALM_SRC_T_DEV,		"device power on"},
	{trap_cfgDataSaveSuccess,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"config-file save success"},
	{trap_cfgDataSaveFail,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"config-file save failure"},
	{trap_flashClearSuccess,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"config-file erase success"},
	{trap_flashClearFail,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"config-file erase failure"},
	{trap_softwareUpdateSuccess,	ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"software update success"},
	{trap_softwareUpdateFail,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"software update failure"},
	{trap_firmwareUpdateSuccess,	ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"firmware update success"},
	{trap_firmwareUpdateFail,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"firmware update failure"},
	{trap_cfgDataBackupSuccess,	ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"config-file backup success"},
	{trap_cfgDataBackupFail,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"config-file backup failure"},
	{trap_cfgDataRestoreSuccess,	ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"config-file restore success"},
	{trap_cfgDataRestoreFail,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"config-file restore failure"},
	{trap_autoProtectSwitch,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_PORT,		"pon auto-protect switch"},
	{trap_cpuUsageFactorHigh,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"device CPU usage alarm"},
	{trap_ponPortBERAlarm,		ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponPortBERAlarmClear,	ALM_SRC_T_MON,		"pon BER alarm"},
	{trap_ponPortBERAlarmClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponPortBERAlarm,		ALM_SRC_T_MON,		"pon BER alarm clear"},
	{trap_ponPortFERAlarm,		ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponPortFERAlarmClear,	ALM_SRC_T_MON,		"pon FER alarm"},
	{trap_ponPortFERAlarmClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponPortFERAlarm,		ALM_SRC_T_MON,		"pon FER alarm clear"},
	{trap_llidActBWExceeding,		ALM_LEV_MINOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_llidActBWExceedingClear,	ALM_SRC_T_LLID,	"register fail for bandwidth lack"},
	{trap_llidActBWExceedingClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_llidActBWExceeding,	ALM_SRC_T_LLID,		"bandwidth lack clear"},
	{trap_devBoardInterted ,		ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_devBoardPull,			ALM_SRC_T_BRD,		"board inserted"},
	{trap_devBoardPull,			ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_devBoardInterted,		ALM_SRC_T_BRD,		"board pulled"},
	{trap_devFanAlarm,			ALM_LEV_MAJOR,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_devFanAlarmClear,	ALM_SRC_T_FAN,		"fan alarm"},
	{trap_devFanAlarmClear,		ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_devFanAlarm,			ALM_SRC_T_FAN,		"fan alarm clear"},
	{trap_powerOffAlarm,			ALM_LEV_VITAL,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_powerOnAlarm,		ALM_SRC_T_BRD,		"power board off"},
	{trap_powerOnAlarm,			ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_powerOffAlarm,		ALM_SRC_T_BRD,		"power board on"},
	{trap_boardTemperatureHigh,	ALM_LEV_VITAL,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_boardTemperatureHighClear,ALM_SRC_T_BRD,	"board temperature alarm"},
	{trap_boardTemperatureHighClear,ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_boardTemperatureHigh,	ALM_SRC_T_BRD,		"board temperature alarm clear"},
	{trap_ponBoardReset,			ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_NO,		0,						ALM_SRC_T_BRD,		"board restart"},
	{trap_swBoardProtectedSwitch,	ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_INS,	0,						ALM_SRC_T_BRD,		"master board switchover"},
	{trap_ponPortAbnormal,		ALM_LEV_MAJOR,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_ponPortAbnormalClear,	ALM_SRC_T_MON,		"pon abnormal"},
	{trap_onuRegisterConflict,		ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"onu MAC address conflict"},
	{trap_firmwareLoadSuccess,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_NO,		trap_firmwareLoadFailure,	ALM_SRC_T_PORT,		"firmware load success"},
	{trap_firmwareLoadFailure,		ALM_LEV_VITAL,	ALM_PRI_LOW,	ALM_SYN_NO,		trap_firmwareLoadSuccess,	ALM_SRC_T_PORT,		"firmware load failure"},
	{trap_dbaUpdateSuccess,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"DBA update success"},
	{trap_dbaUpdateFailure,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"DBA update failure"},
	{trap_dbaLoadSuccess,			ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_NO,		trap_dbaLoadFailure,		ALM_SRC_T_PORT,		"DBA load success"},
	{trap_dbaLoadFailure,			ALM_LEV_VITAL,	ALM_PRI_HIGH,	ALM_SYN_NO,		trap_dbaLoadSuccess,		ALM_SRC_T_PORT,		"DBA load failure"},
	{trap_ponToEthLinkdown,		ALM_LEV_WARN,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_ponToEthLinkup,		ALM_SRC_T_PORT,		"pon linkdown"},	/* modified by xieshl 20070703 */
	{trap_ponToEthLinkup,			ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_ponToEthLinkdown,	ALM_SRC_T_PORT,		"pon linkup"},		
	{trap_onuSoftwareLoadSuccess,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_NO,		trap_onuSoftwareLoadFailure,	ALM_SRC_T_DEV,	"onu software load success"},
	{trap_onuSoftwareLoadFailure,	ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_NO,		trap_onuSoftwareLoadSuccess,	ALM_SRC_T_DEV,	"onu software load failure"},
	{trap_ethFlrAlarm ,				ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ethFlrAlarmClear,		ALM_SRC_T_PORT,		"eth FLR alarm"},
	{trap_ethFlrAlarmClear,			ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ethFlrAlarm,			ALM_SRC_T_PORT,		"eth FLR alarm clear"},
	{trap_ethFerAlarm,				ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ethFerAlarmClear,		ALM_SRC_T_PORT,		"eth FER alarm"},
	{trap_ethFerAlarmClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ethFerAlarm,			ALM_SRC_T_PORT,		"eth FER alarm clear"},
	{trap_ethTranmittalIntermitAlarm,ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ethTranmittalIntermitAlarmClear,ALM_SRC_T_PORT,	"eth traffic halt alarm"},
	{trap_ethTranmittalIntermitAlarmClear,ALM_LEV_CLEAR,	ALM_PRI_LOW,ALM_SYN_DEL,	trap_ethTranmittalIntermitAlarm,	ALM_SRC_T_PORT,		"eth traffic halt clear"},
	{trap_ethLinkdown,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ethLinkup,			ALM_SRC_T_PORT,		"eth linkdown"},
	{trap_ethLinkup,				ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ethLinkdown,			ALM_SRC_T_PORT,		"eth linkup"},

	{trap_bootUpdateSuccess,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"boot update success"},
	{trap_bootUpdateFailure,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"boot update  failure"},
	{trap_batfileBackupSuccess,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"batfile backup success"},
	{trap_batfileBackupFailure,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"batfile backup failure"},
	{trap_batfileRestoreSuccess,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"batfile restore success"},
	{trap_batfileRestoreFailure,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_DEV,		"batfile restore failure"},

	{trap_onuRegAuthFailure,		ALM_LEV_MINOR,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_COMM,	"Illegal ONU register"},/* added by xieshl 20070703 */
	{trap_deviceColdStart,			ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_INS,	0,						ALM_SRC_T_DEV,		"cold start"},
	{trap_deviceWarmStart,		ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_INS,	0,						ALM_SRC_T_DEV,		"warm start"},
	{trap_deviceExceptionRestart,	ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_INS,	0,						ALM_SRC_T_DEV,		"exception restart"},

	/*add by shixh@20070928*/
	{trap_e1LosAlarm,			       ALM_LEV_VITAL,	ALM_PRI_HIGH,	ALM_SYN_INS,       trap_e1LosAlarmClear,		ALM_SRC_T_PORT,		"e1 LOS alarm "},
	{trap_e1LosAlarmClear,			ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_e1LosAlarm,			ALM_SRC_T_PORT,		"e1 LOS alarm clear"},
	{trap_e1LofAlarm,				ALM_LEV_MAJOR,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_e1LofAlarmClear,		ALM_SRC_T_PORT,		"e1 LOF alarm "},
	{trap_e1LofAlarmClear,			ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_e1LofAlarm,			ALM_SRC_T_PORT,		"e1 LOF alarm clear"},
	{trap_e1AisAlarm,				ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_e1AisAlarmClear,		ALM_SRC_T_PORT,		"e1 AIS alarm "},
	{trap_e1AisAlarmClear,			ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_e1AisAlarm,			ALM_SRC_T_PORT,		"e1 AIS alarm clear"},
	{trap_e1RaiAlarm,				ALM_LEV_MINOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_e1RaiAlarmClear,		ALM_SRC_T_PORT,		"e1 RAI alarm"},
	{trap_e1RaiAlarmClear,			ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_e1RaiAlarm,			ALM_SRC_T_PORT,		"e1 RAI alarm clear"},
	{trap_e1SmfAlarm,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_e1SmfAlarmClear,		ALM_SRC_T_PORT,		"e1 LOFSMF alarm"},
	{trap_e1SmfAlarmClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_e1SmfAlarm,			ALM_SRC_T_PORT,		"e1 LOFSMF alarm clear"},
	{trap_e1LomfAlarm,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_e1LomfAlarmClear,	ALM_SRC_T_PORT,		"e1 LOFCMF alarm"},
	{trap_e1LomfAlarmClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_e1LomfAlarm,			ALM_SRC_T_PORT,		"e1 LOFCMF alarm clear"},
	{trap_e1Crc3Alarm,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_e1Crc3AlarmClear,	ALM_SRC_T_PORT,		"e1 CRC-3 alarm"},
	{trap_e1Crc3AlarmClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_e1Crc3Alarm,			ALM_SRC_T_PORT,		"e1 CRC-3 alarm clear"},
	{trap_e1Crc6Alarm,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_e1Crc6AlarmClear,	ALM_SRC_T_PORT,		"e1 CRC-6 alarm"},
	{trap_e1Crc6AlarmClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_e1Crc6Alarm,			ALM_SRC_T_PORT,		"e1 CRC-6 alarm clear"},

	{trap_tdmServiceAbortAlarm,	ALM_LEV_VITAL,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_tdmServiceAbortAlarmClear,ALM_SRC_T_DEV,	"The TDM Service Abort Alarm"},
	{trap_tdmServiceAbortAlarmClear,ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_tdmServiceAbortAlarm,		ALM_SRC_T_DEV,	"The TDM Service Abort Clear"},

	/*added by xieshl 20080116*/
	{trap_ethLoopAlarm,			ALM_LEV_MAJOR,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_ethLoopAlarmClear,	ALM_SRC_T_PORT,		"eth loop alarm"},
	{trap_ethLoopAlarmClear,		ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_ethLoopAlarm,		ALM_SRC_T_PORT,		"eth loop clear"},
	{trap_onuLoopAlarm,			ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_onuLoopAlarmClear,	ALM_SRC_T_DEV,		"eth loop alarm"},
	{trap_onuLoopAlarmClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_onuLoopAlarm,		ALM_SRC_T_DEV,		"eth loop clear"},
	/* end 20080116 */

	{trap_backboneEthLinkdown,		ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_backboneEthLinkup,	ALM_SRC_T_PORT,		"backbone-eth linkdown"},	
	{trap_backboneEthLinkup,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_backboneEthLinkdown,	ALM_SRC_T_PORT,		"backbone-eth linkup"},		


	/*add by shixh@20070202*/
	{trap_tdmToEthLinkdown,		ALM_LEV_MINOR,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_tdmToEthLinkup,		ALM_SRC_T_PORT,		"tdm linkdown"},	
	{trap_tdmToEthLinkup,			ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_tdmToEthLinkdown,	ALM_SRC_T_PORT,		"tdm linkup"},		

	/*add by shixh@20070312*/	
	{trap_onuAutoLoadConfigSuccess,ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_onuAutoLoadConfigFailure,	ALM_SRC_T_DEV,	"onu auto-config success"},	
	{trap_onuAutoLoadConfigFailure,	ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_onuAutoLoadConfigSuccess,	ALM_SRC_T_DEV,	"onu auto-config failure"},

	{trap_ponLaserAlwaysOnAlarm,		ALM_LEV_VITAL,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_ponLaserAlwaysOnAlarmClear,	ALM_SRC_T_ONU_LASER,	"onu laser always on"},
	{trap_ponLaserAlwaysOnAlarmClear,	ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_ponLaserAlwaysOnAlarm,		ALM_SRC_T_ONU_LASER,	"onu laser normal"},

	/* modified by xieshl 20120113, 光功率告警不再点ALM灯，问题单14295 */
	{trap_ponReceiverPowerTooLow,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponReceiverPowerTooLowClear,		ALM_SRC_T_MON,	"pon rx optical power low"},
	{trap_ponReceiverPowerTooLowClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponReceiverPowerTooLow,			ALM_SRC_T_MON,	"pon rx optical power normal"},
	{trap_ponReceiverPowerTooHigh,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponReceiverPowerTooHighClear,		ALM_SRC_T_MON,	"pon rx optical power high"},
	{trap_ponReceiverPowerTooHighClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponReceiverPowerTooHigh,			ALM_SRC_T_MON,	"pon rx optical power normal"},
	{trap_ponTransmissionPowerTooLow,		ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponTransmissionPowerTooLowClear,	ALM_SRC_T_MON,	"pon tx optical power low"},
	{trap_ponTransmissionPowerTooLowClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponTransmissionPowerTooLow,		ALM_SRC_T_MON,	"pon tx optical power normal"},
	{trap_ponTransmissionPowerTooHigh,		ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponTransmissionPowerTooHighClear,	ALM_SRC_T_MON,	"pon tx optical power high"},
	{trap_ponTransmissionPowerTooHighClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponTransmissionPowerTooHigh,		ALM_SRC_T_MON,	"pon tx optical power normal"},
	{trap_ponAppliedVoltageTooHigh,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponAppliedVoltageTooHighClear,		ALM_SRC_T_MON,	"pon sfp applied-voltage high"},
	{trap_ponAppliedVoltageTooHighClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponAppliedVoltageTooHigh,			ALM_SRC_T_MON,	"pon sfp applied-voltage normal"},
	{trap_ponAppliedVoltageTooLow,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponAppliedVoltageTooLowClear,		ALM_SRC_T_MON,	"pon sfp applied-voltage low"},
	{trap_ponAppliedVoltageTooLowClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponAppliedVoltageTooLow,			ALM_SRC_T_MON,	"pon sfp applied-voltage normal"},
	{trap_ponBiasCurrentTooHigh,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponBiasCurrentTooHighClear,		ALM_SRC_T_MON,	"pon sfp bias-current high"},
	{trap_ponBiasCurrentTooHighClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponBiasCurrentTooHigh,			ALM_SRC_T_MON,	"pon sfp bias-current normal"},
	{trap_ponBiasCurrentTooLow,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponBiasCurrentTooLowClear,		ALM_SRC_T_MON,	"pon sfp bias-current low"},
	{trap_ponBiasCurrentTooLowClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponBiasCurrentTooLow,				ALM_SRC_T_MON,	"pon sfp bias-current normal"},
	/*add byshixh@20080820*/
	{trap_ponTemperatureTooHigh,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponTemperatureTooHighClear,		ALM_SRC_T_MON,	"pon sfp temperature high"},
	{trap_ponTemperatureTooHighClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponTemperatureTooHigh,			ALM_SRC_T_MON,	"pon sfp temperature normal"},
	{trap_ponTemperatureTooLow,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponTemperatureTooLowClear,		ALM_SRC_T_MON,	"pon sfp temperature low"},
	{trap_ponTemperatureTooLowClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponTemperatureTooLow,			ALM_SRC_T_MON,	"pon sfp temperature normal"},
	/*wangdp 20091006*/
	{trap_boardCpuUsageAlarm,				ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_boardCpuUsageAlarmClear,			ALM_SRC_T_BRD,	"board Cpu Usage high"},
	{trap_boardCpuUsageAlarmClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_boardCpuUsageAlarm,				ALM_SRC_T_BRD,	"board Cpu Usage high normal"},
	{trap_boardMemoryUsageAlarm,			ALM_LEV_MAJOR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_boardMemoryUsageAlarmClear,		ALM_SRC_T_BRD,	"board Memory Usage high"},
	{trap_boardMemoryUsageAlarmClear,		ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_boardMemoryUsageAlarm,			ALM_SRC_T_BRD,	"board Memory Usage normal"},

	{trap_oltPonReceiverPowerTooLow,		ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_oltPonReceiverPowerTooLowClear,	ALM_SRC_T_PON_PWR,"rx onu optical power low"},
	{trap_oltPonReceiverPowerTooLowClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_oltPonReceiverPowerTooLow,		ALM_SRC_T_PON_PWR,	"rx onu optical power normal"},
	{trap_oltPonReceiverPowerTooHigh,		ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_oltPonReceiverPowerTooHighClear,	ALM_SRC_T_PON_PWR,"rx onu optical power high"},
	{trap_oltPonReceiverPowerTooHighClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_oltPonReceiverPowerTooHigh,		ALM_SRC_T_PON_PWR,	"rx onu optical power normal"},

	/*add by shixh@20081201*/
	{trap_deviceTemperatureHigh,			ALM_LEV_MAJOR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_deviceTemperatureHighClear,	ALM_SRC_T_DEV,		"device temperature high"},
	{trap_deviceTemperatureHighClear,		ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_deviceTemperatureHigh,		ALM_SRC_T_DEV,		"device temperature normal"},
	{trap_deviceTemperatureLow,			ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_deviceTemperatureLowClear,	ALM_SRC_T_DEV,		"device temperature low"},
	{trap_deviceTemperatureLowClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_deviceTemperatureLow,			ALM_SRC_T_DEV,		"device temperature normal"},

	{trap_onuAutoLoadUpgradeSuccess,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_onuAutoLoadUpgradeFailure,		ALM_SRC_T_DEV,		"onu auto-upgrade success"},	
	{trap_onuAutoLoadUpgradeFailure,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_onuAutoLoadUpgradeSuccess,	ALM_SRC_T_DEV,		"onu auto-upgrade failure"},

	{trap_E1OutOfService,					ALM_LEV_MAJOR,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_E1OutOfServiceClear,		ALM_SRC_T_PORT,		"E1 out of sevice"},	
	{trap_E1OutOfServiceClear,				ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_E1OutOfService,			ALM_SRC_T_PORT,		"E1 out of sevice clear"},
	/*add by shixh20090507*/
	{trap_PonPortFullAlarm,					ALM_LEV_MINOR,	ALM_PRI_LOW,	ALM_SYN_NO,		0,							ALM_SRC_T_PORT,		"onu-list full"},	
	{trap_ponPortAbnormalClear,			ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponPortAbnormal,			ALM_SRC_T_MON,		"pon abnormal clear"},
	/*add by shixh20090520*/
	{trap_SwitchEthPortLoop,				ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_SwitchEthPortLoopClear,	ALM_SRC_T_ONU_SWITCH,	"onu-switch loop alarm"},	
	{trap_SwitchEthPortLoopClear,			ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_SwitchEthPortLoop,			ALM_SRC_T_ONU_SWITCH,	"onu-switch loop alarm clear"},

	/*{trap_PWUPowerOff,					ALM_LEV_MAJOR,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_PWUPowerOn,		"PWU power off"},
	  {trap_PWUPowerOn,					ALM_LEV_MAJOR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_PWUPowerOff,	"PWU power on"},*/
	/* end: added by jianght 20090519 */

	/*add by shixh20090612*/
	{trap_ethPortBroadCastFloodControl,		ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ethPortBroadCastFloodControlClear,	ALM_SRC_T_PORT,	"broadcast flood control alarm"},	
	{trap_ethPortBroadCastFloodControlClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ethPortBroadCastFloodControl,		ALM_SRC_T_PORT,	"broadcast flood control alarm clear"},

	/*add by shixh20090603*/
	{trap_sysfileUploadsuccess,			ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_sysfileUploadfailure,	ALM_SRC_T_DEV,		"sysfile upload success"},	/*问题单8817*/
	{trap_sysfileUploadfailure,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_sysfileUploadsuccess,	ALM_SRC_T_DEV,		"sysfile upload failure"},
	{trap_sysfileDownloadsuccess,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_sysfileDownloadfailure,	ALM_SRC_T_DEV,		"sysfile download success"},	
	{trap_sysfileDownloadfailure,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_sysfileDownloadsuccess,ALM_SRC_T_DEV,		"sysfile download failure"},

	/*add by shixh20090626*/
	{trap_ponPortlosAlarm,				ALM_LEV_MAJOR,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_ponPortlosAlarmClear,	ALM_SRC_T_PORT,		"pon LOS alarm"},	
	{trap_ponPortlosAlarmClear,	 		ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_ponPortlosAlarm,		ALM_SRC_T_PORT,		"pon LOS alarm clear"},

	/*add by shixh20090710*/
	{trap_ponFWVersionMismatch,		ALM_LEV_VITAL,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_ponFWVersionMatch,	ALM_SRC_T_PORT,		"firmware ver mis-match"},	
	{trap_ponFWVersionMatch,	 		ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_ponFWVersionMismatch,ALM_SRC_T_PORT,		"firmware ver matched"},

	{trap_ponDBAVersionMismatch,		ALM_LEV_VITAL,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_ponDBAVersionMatch,	ALM_SRC_T_PORT,		"DBA ver mis-match"},	
	{trap_ponDBAVersionMatch,	 		ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_ponDBAVersionMismatch,ALM_SRC_T_PORT,	"DBA ver matched"},

	{trap_ponSFPTypeMismatch,			ALM_LEV_VITAL,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_ponSFPTypeMitch,		ALM_SRC_T_PORT,		"SFP type mis-match"},	
	{trap_ponSFPTypeMitch,	 		ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_ponSFPTypeMismatch,	ALM_SRC_T_PORT,		"SFP type matched"},

	{trap_ponportBRASAlarm,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponportBRASAlarmClear,ALM_SRC_T_COMM,	"BRAS mac alarm"},	
	{trap_ponportBRASAlarmClear,	 	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponportBRASAlarm,	ALM_SRC_T_COMM,	"BRAS mac alarm clear"},

	{trap_ponPortUpNoTraffic,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ponPortUpNoTrafficClear,ALM_SRC_T_PORT,	"pon traffic abnormal" },
	{trap_ponPortUpNoTrafficClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ponPortUpNoTraffic,	ALM_SRC_T_PORT,		"pon traffic normal" },
	{trap_onuDeletingNotify,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,						ALM_SRC_T_COMM,	"onu deleted"},

	{trap_switchNewRegSuccess,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_INS,	0,						ALM_SRC_T_ONU_SWITCH,	"onu-switch first register"},
	{trap_switchReregSuccess,			ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_switchNotPresent,		ALM_SRC_T_ONU_SWITCH,	"onu-switch re-register"},
	{trap_switchNotPresent,				ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_switchReregSuccess,	ALM_SRC_T_ONU_SWITCH,	"onu-switch not present"},

	{trap_uplinkReceiverPowerTooLow,		ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_uplinkReceiverPowerTooLowClear,	ALM_SRC_T_ETH_PWR,	"eth rx optical power low"},
	{trap_uplinkReceiverPowerTooLowClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_uplinkReceiverPowerTooLow,		ALM_SRC_T_ETH_PWR,	"eth rx optical power normal"},
	{trap_uplinkReceiverPowerTooHigh,		ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_uplinkReceiverPowerTooHighClear,	ALM_SRC_T_ETH_PWR,	"eth rx optical power high"},
	{trap_uplinkReceiverPowerTooHighClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_uplinkReceiverPowerTooHigh,		ALM_SRC_T_ETH_PWR,	"eth rx optical power normal"},
	{trap_uplinkTransmissionPowerTooLow,	ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_uplinkTransmissionPowerTooLowClear,ALM_SRC_T_ETH_PWR,	"eth tx optical power low"},
	{trap_uplinkTransmissionPowerTooLowClear,ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_uplinkTransmissionPowerTooLow,		ALM_SRC_T_ETH_PWR,	"eth tx optical power normal"},
	{trap_uplinkTransmissionPowerTooHigh,	ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_uplinkTransmissionPowerTooHighClear,ALM_SRC_T_ETH_PWR,	"eth tx optical power high"},
	{trap_uplinkTransmissionPowerTooHighClear,ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_uplinkTransmissionPowerTooHigh,	ALM_SRC_T_ETH_PWR,	"eth tx optical power normal"},
	{trap_uplinkAppliedVoltageTooHigh,		ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_uplinkAppliedVoltageTooHighClear,	ALM_SRC_T_ETH_PWR,	"eth sfp applied-voltage high"},
	{trap_uplinkAppliedVoltageTooHighClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_uplinkAppliedVoltageTooHigh,		ALM_SRC_T_ETH_PWR,	"eth sfp applied-voltage normal"},
	{trap_uplinkAppliedVoltageTooLow,		ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_uplinkAppliedVoltageTooLowClear,	ALM_SRC_T_ETH_PWR,	"eth sfp applied-voltage low"},
	{trap_uplinkAppliedVoltageTooLowClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_uplinkAppliedVoltageTooLow,		ALM_SRC_T_ETH_PWR,	"eth sfp applied-voltage normal"},
	{trap_uplinkBiasCurrentTooHigh,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_uplinkBiasCurrentTooHighClear,		ALM_SRC_T_ETH_PWR,	"eth sfp bias-current high"},
	{trap_uplinkBiasCurrentTooHighClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_uplinkBiasCurrentTooHigh,			ALM_SRC_T_ETH_PWR,	"eth sfp bias-current normal"},
	{trap_uplinkBiasCurrentTooLow,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_uplinkBiasCurrentTooLowClear,		ALM_SRC_T_ETH_PWR,	"eth sfp bias-current low"},
	{trap_uplinkBiasCurrentTooLowClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_uplinkBiasCurrentTooLow,			ALM_SRC_T_ETH_PWR,	"eth sfp bias-current normal"},
	{trap_uplinkTemperatureTooHigh,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_uplinkTemperatureTooHighClear,		ALM_SRC_T_ETH_PWR,	"eth sfp temperature high"},
	{trap_uplinkTemperatureTooHighClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_uplinkTemperatureTooHigh,			ALM_SRC_T_ETH_PWR,	"eth sfp temperature normal"},
	{trap_uplinkTemperatureTooLow,			ALM_LEV_WARN,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_uplinkTemperatureTooLowClear,		ALM_SRC_T_ETH_PWR,	"eth sfp temperature low"},
	{trap_uplinkTemperatureTooLowClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_uplinkTemperatureTooLow,			ALM_SRC_T_ETH_PWR,	"eth sfp temperature normal"},

	/*ctc onu 增加部分*/
	{trap_ctcOnuEquipmentAlarm,		ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ctcOnuEquipmentAlarmClear,	ALM_SRC_T_ETH_PWR,	"ctc onu equpiment alarm"},
	{trap_ctcOnuEquipmentAlarmClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ctcOnuEquipmentAlarm,			ALM_SRC_T_ETH_PWR,	"ctc onu equpiment normal"},
	{trap_ctcOnuBatteryMissing,			ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ctcOnuBatteryMissingClear,		ALM_SRC_T_ETH_PWR,	"ctc onu battery missing"},
	{trap_ctcOnuBatteryMissingClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ctcOnuBatteryMissing,			ALM_SRC_T_ETH_PWR,	"ctc onu battery normal"},
	{trap_ctcOnuBatteryFailure,			ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ctcOnuBatteryFailureClear,		ALM_SRC_T_ETH_PWR,	"ctc onu battery couldn't charge"},
	{trap_ctcOnuBatteryFailureClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ctcOnuBatteryFailure,			ALM_SRC_T_ETH_PWR,	"ctc onu battery normal"},
	{trap_ctcOnuBatteryVoltLow,			ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ctcOnuBatteryVoltLowClear,		ALM_SRC_T_ETH_PWR,	"ctc onu battery volt low"},
	{trap_ctcOnuBatteryVoltLowClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ctcOnuBatteryVoltLow,			ALM_SRC_T_ETH_PWR,	"ctc onu battery volt normal"},
	{trap_ctcOnuPhysicalIntrusionAlarm,	ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ctcOnuPhysicalIntrusionAlarmClear,ALM_SRC_T_ETH_PWR, "ctc onu physical intrusion alarm"},
	{trap_ctcOnuPhysicalIntrusionAlarmClear,ALM_LEV_CLEAR,ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ctcOnuPhysicalIntrusionAlarm,	ALM_SRC_T_ETH_PWR,	"ctc onu physical intrusion normal"},
	{trap_ctcOnuSelfTestFailure,			ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ctcOnuSelfTestFailureClear,		ALM_SRC_T_ETH_PWR,	"ctc onu selftest failure alarm"},
	{trap_ctcOnuSelfTestFailureClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ctcOnuSelfTestFailure,			ALM_SRC_T_ETH_PWR,	"ctc onu selftest failure normal"},
	{trap_ctcOnuTemperatureHigh,		ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ctcOnuTemperatureHighClear,	ALM_SRC_T_ETH_PWR,	"ctc onu temperature high alarm"},
	{trap_ctcOnuTemperatureHighClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ctcOnuTemperatureHigh,		ALM_SRC_T_ETH_PWR,	"ctc onu temperature normal"},
	{trap_ctcOnuTemperatureLow,		ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ctcOnuTemperatureLowClear,	ALM_SRC_T_ETH_PWR,	"ctc onu temperature low alarm"},
	{trap_ctcOnuTemperatureLowClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ctcOnuTemperatureLow,		ALM_SRC_T_ETH_PWR,	"ctc onu temperature normal"},
	{trap_ctcOnuIADConnectionFailure,	ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ctcOnuIADConnectionFailureClear,ALM_SRC_T_ETH_PWR,	"ctc onu IAD connection failure alarm"},
	{trap_ctcOnuIADConnectionFailureClear,ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ctcOnuIADConnectionFailure,	ALM_SRC_T_ETH_PWR,	"ctc onu IAD connection normal"},
	{trap_ctcOnuPonIfSwitch,			ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ctcOnuPonIfSwitchClear,		ALM_SRC_T_ETH_PWR,	"ctc onu pon if switch alarm"},
	{trap_ctcOnuPonIfSwitchClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ctcOnuPonIfSwitch,			ALM_SRC_T_ETH_PWR,	"ctc onu pon if switch normal"},


	{trap_ethAutoNegFailure,		ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ethAutoNegFailureClear,	ALM_SRC_T_ETH_PWR,	"ctc onu eth autoneg failure alarm"},
	{trap_ethAutoNegFailureClear,	ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ethAutoNegFailure,			ALM_SRC_T_ETH_PWR,	"ctc onu eth autoneg normal"},
	{trap_ethLos,					ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ethLosCLear,				ALM_SRC_T_ETH_PWR,	"loss of signal alarm"},
	{trap_ethLosCLear,			ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ethLos,					ALM_SRC_T_ETH_PWR,	"loss of signal normal"},
	{trap_ethFailure,				ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ethFailureClear,			ALM_SRC_T_ETH_PWR,	"ctc onu eth failure alarm"},
	{trap_ethFailureClear,			ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ethFailure,				ALM_SRC_T_ETH_PWR,	"ctc onu eth failure normal"},
	{trap_ethCongestion,			ALM_LEV_MAJOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_ethCongestionClear,		ALM_SRC_T_ETH_PWR,	"ctc onu eth congestion alarm"},
	{trap_ethCongestionClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_ethCongestion,			ALM_SRC_T_ETH_PWR,	"ctc onu eth congestion normal"},

	/*add by sxh20111014*/
	{trap_eth_DosAttack,			ALM_LEV_MINOR,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_eth_DosAttackClear,		ALM_SRC_T_ETH_PWR,	"dos attack"},
	{trap_eth_DosAttackClear,		ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_eth_DosAttack,			ALM_SRC_T_ETH_PWR,	"dos attack normal"},

	/*add by sxh20111227*/
	{trap_onuMacTableOverFlow,	ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_onuMacTableOverFlowClear,	ALM_SRC_T_DEV,	"onu mac table overflow"},	
	{trap_onuMacTableOverFlowClear,ALM_LEV_CLEAR,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_onuMacTableOverFlow,		ALM_SRC_T_DEV,	"onu mac table overflow normal"},
	{trap_switchEthIngressLimitExceed,	ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_switchEthIngressLimitExceedClear,	ALM_SRC_T_ONU_SWITCH,	"onu-switch ingress alarm"},
	{trap_switchEthIngressLimitExceedClear,ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_switchEthIngressLimitExceed,		ALM_SRC_T_ONU_SWITCH,	"onu-switch ingress alarm clear"},
	{trap_switchEthEgressLimitExceed,	ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_INS,	trap_switchEthEgressLimitExceedClear,	ALM_SRC_T_ONU_SWITCH,	"onu-switch egress alarm"},
	{trap_switchEthEgressLimitExceedClear,ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_DEL,	trap_switchEthEgressLimitExceed,			ALM_SRC_T_ONU_SWITCH,	"onu-switch egress alarm clear"},

	{trap_userLocUpdateNotify,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,							ALM_SRC_T_USER_TRACE,		"user location notify"},
	{trap_backupPonAlarm,             ALM_LEV_NULL,   ALM_PRI_LOW,    ALM_SYN_INS,    trap_backupPonAlarmClear,   ALM_SRC_T_ONU_SWITCH,   "onu backup-pon alarm"},
	{trap_backupPonAlarmClear,        ALM_LEV_NULL, ALM_PRI_LOW,    ALM_SYN_DEL,    trap_backupPonAlarm,            ALM_SRC_T_ONU_SWITCH,   "onu backup-pon alarm clear"},
	{trap_boardLosAlarm,		ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_boardLosAlarmClear,			ALM_SRC_T_BRD,		"board disconnect alarm"},
	{trap_boardLosAlarmClear,			ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_boardLosAlarm,		ALM_SRC_T_BRD,		"board disconnect alarm clear"},

	{trap_logicalSlotInsert,			ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_logicalSlotPull,		ALM_SRC_T_BRD,		"logical slot inserted"},
	{trap_logicalSlotPull,			ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_logicalSlotInsert,		ALM_SRC_T_BRD,		"logical slot pulled"},
	{trap_ponProtectSwitch,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,	0,		ALM_SRC_T_PORT,		"pon-protect switch"},

	/*begin: add by @muqw 2017-4-26*/
	{trap_pwuStatusAbnoarmal,		ALM_LEV_VITAL,	ALM_PRI_HIGH,	ALM_SYN_INS,	trap_pwuStatusAbnoarmalClear,	ALM_SRC_T_MON,		"pwu status abnormal"},
	{trap_pwuStatusAbnoarmalClear, 		ALM_LEV_CLEAR,	ALM_PRI_HIGH,	ALM_SYN_DEL,	trap_pwuStatusAbnoarmal, 	ALM_SRC_T_MON,		"pwu status abnormal clear"},
	/*end: add by @muqw 2017-4-26*/

	{trap_private_max,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,							ALM_SRC_T_NULL,	""}
};

static eventInfoMapTbl_t other_EventLog_MapInfo[] = 
{
	{other_eventlog_min,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,		ALM_SRC_T_NULL,		""},
	{other_diagnosis_success,   		ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_NO,		0,		ALM_SRC_T_DEV,		"diagnosis itself success"},
	{other_diagnosis_fail,   			ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_NO,		0,		ALM_SRC_T_DEV,		"diagnosis itself failure"},
	{other_onustp_topologychange,	ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,		ALM_SRC_T_DEV,		"STP topology change"},
	{other_onustp_newroot,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,		ALM_SRC_T_DEV,		"STP new root"},
	/* added by xieshl 20080421, 控制通道中断记录告警日志 */
	{other_ctrlchan_success,		ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_NO,		0,		ALM_SRC_T_DEV,		"CPU contral channel idle"},
	{other_ctrlchan_fail,			ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_NO,		0,		ALM_SRC_T_DEV,		"CPU contral channel busy"},
	/* added by chenfj 2009-2-19, PON 端口版本不匹配告警日志*/
	{other_reserve1, 				ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0, 		ALM_SRC_T_NULL,		"reserve"},
	{other_reserve2,				ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,		ALM_SRC_T_NULL,		"reserve"},
	{other_pon_cni_ber,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,		ALM_SRC_T_PORT,		"CNI BER alarm"},
	{other_pon_cni_ber_clear,		ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,		ALM_SRC_T_PORT,		"CNI BER alarm clear"},
	{other_pon_update_file,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,		ALM_SRC_T_PORT,		"update file"},
	{other_pon_slave_ok,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,		ALM_SRC_T_PORT,		"Slave-Pon is working normal"},
	{other_pon_slave_fail,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,		ALM_SRC_T_PORT,		"Slave-Pon is not working"},
	/* added by xieshl 2011017，记录SNMP部分操作 */
	{other_snmp_reset_dev,		ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_NO,		0,		ALM_SRC_T_OTHER,	"is rebooted by snmp"},
	{other_snmp_reset_brd,		ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_NO,		0,		ALM_SRC_T_OTHER,	"is reset by snmp"},
	{other_snmp_reset_pon,		ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_NO,		0,		ALM_SRC_T_OTHER,	"is reset by snmp"},
	{other_snmp_save_config,		ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_NO,		0,		ALM_SRC_T_OTHER,	"save running-config by snmp"},
	{other_snmp_erase_config,		ALM_LEV_NULL,	ALM_PRI_HIGH,	ALM_SYN_NO,		0,		ALM_SRC_T_OTHER,	"erase config-file by snmp"},

	{other_eventlog_max,			ALM_LEV_NULL,	ALM_PRI_LOW,	ALM_SYN_NO,		0,		ALM_SRC_T_NULL,		""},
};

/* added by xieshl 20080424, 告警日志过滤 */
static ULONG eventLogDataFilter = ALM_FILTER_DEFAULT;

 ULONG nvramLogTimeToStrings( nvramLogDateAndTime_t *pTime, CHAR *str );
static ULONG getEventLogDevName( ULONG devIdx, CHAR *tmp );
ULONG getEvent2NvramFlag( ULONG alarmType, ULONG alarmId );
extern ULONG dev_idx_compatible_get( ULONG devIdx );

extern LONG devsm_get_slocal_time( SLOCAL_TIME * time );
extern LONG bsp_nvram_exist( void );
extern LONG bsp_rtc_exist(void);
extern STATUS getDeviceName( const ULONG devIdx, CHAR* pValBuf, ULONG *pValLen );
extern CHAR *onu_auth_result_to_str( LONG type );
extern int CTCONU_GetAlarmThreshold(CTC_STACK_alarm_id_t code, LONG *alarm, LONG *clear);
extern int decimal2_integer_part( int val );
extern int decimal2_fraction_part( int val );
extern BOOL dev_idx_is_equal( ULONG devIdx1, ULONG devIdx2 );
extern int cl_vty_telnet_out( const char * format, ... );

/*----------------------------------------------------------------------------*/

CHAR *macAddress_To_Strings(UCHAR *pMacAddr)
{
	static CHAR mac_str[20];

	if( pMacAddr != NULL )
	{
		VOS_Sprintf( mac_str, "%02x%02x.%02x%02x.%02x%02x",
				pMacAddr[0], pMacAddr[1], pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5] );
	}
	else
		VOS_StrCpy( mac_str,  "0000.0000.0000");
	return mac_str;
}

LONG getEventLogTime( nvramLogDateAndTime_t *dt )
{
	SLOCAL_TIME stTime;
	LONG rc = VOS_ERROR;

	if( dt == NULL )
		return rc;

	if( (rc = devsm_get_slocal_time(&stTime)) == VOS_OK )
	{
		if( stTime.usYear > 2000 )
			dt->year = stTime.usYear - 2000;
		else
			dt->year = 10;
		dt->month = stTime.usMonth;
		dt->day = stTime.usDay;
		dt->hour = stTime.usHour;
		dt->minute = stTime.usMinute;
		dt->second = stTime.usSecond;
	}
	return rc;
}
/*将时间信息复制到一个ulong变量中*/
/*这两个函数的作用一样。*/
static ULONG nvramLogDateAndTimeToUlong( nvramLogDateAndTime_t *pTime )
{
	ULONG ulTime;
	VOS_MemCpy( &ulTime, pTime, sizeof(ULONG) );
	
	return ulTime;
}
static ULONG sysLogDateAndTimeToUlong( sysDateAndTime_t *pTime )
{
	nvramLogDateAndTime_t nvramTime;

	if( pTime->year >2000 )
		nvramTime.year = pTime->year - 2000;
	else
		nvramTime.year = 6;
	nvramTime.month = pTime->month;
	nvramTime.day = pTime->day;
	nvramTime.hour = pTime->hour;
	nvramTime.minute = pTime->minute;
	nvramTime.second = pTime->second;
	
	return nvramLogDateAndTimeToUlong( &nvramTime );
}


CHAR* onuNotPresentReasonToString( alarmSrc_t  *pAlmSrc )
{
#if 0	/* modified by xieshl 20090422 */
	static CHAR *reasonStr[] = {
		" ",
		": REPORT TIMEOUT",		/* Timeout has expired since the last MPCP REPORT */
                            	/* frame received from the ONU */
		": OAM LINK DISCONN",	/* ONU didn't reply to several OAM Information */
                            	/* frames, hence OAM link is disconnected */
		": HOST REQUEST",		/* Response to previous PAS_deregister_onu or */
                            	/* PAS_shutdown_onu API calls */
		": REG CONFLICT",		/* ONU registered twice without deregistering */
                            	/* This is an error handling for extrem cases. */
                            	/* If the ONU should be registered, the following */
                            	/* REGISTER_REQ will cause a correct registration.*/
		": TIMESTAMP DRIFT",	/* when a timestamp drift of 16nsec is identified by the HW */
		" ",					/* unknown reason, (recovery after missing event) */
		" "
	};

	/*if( pAlmSrc != NULL )*/
	{
		if( (pAlmSrc->devAlarmSrc.devData > 0) && (pAlmSrc->devAlarmSrc.devData < DEREG_LAST_CODE) )
			return reasonStr[pAlmSrc->devAlarmSrc.devData];
	}
	return reasonStr[0];
#endif
	if( pAlmSrc != NULL )
	{
		if( (pAlmSrc->devAlarmSrc.devData > 0) && (pAlmSrc->devAlarmSrc.devData < PON_ONU_DEREGISTRATION_LAST_CODE) ) /* modified by chenfj 2009-5-7 离线原因的范围检查依据于pas-soft文件中的定义*/
			return PON_onu_deregistration_code_s[pAlmSrc->devAlarmSrc.devData];
	}
	return PON_onu_deregistration_code_s[0];
}

/*modyied by shixh@20080728*/
/*功能:check type and ID*/
LONG checkEventLogData( nvramEventLogData_t *pEntry )
{
	LONG rc = VOS_ERROR;

	if( (pEntry->alarmTime.month != 0) && (pEntry->alarmTime.month <= 12) && (pEntry->alarmTime.day != 0) && (pEntry->alarmTime.day <= 31) )
	{
		switch( pEntry->alarmType )
		{
			case alarmType_mib2:
				if( (pEntry->alarmId != trap_mib2_min) && (pEntry->alarmId < trap_mib2_max) )
					rc = VOS_OK;
				break;
			case alarmType_bridge:
				if( (pEntry->alarmId != trap_bridge_min) && (pEntry->alarmId < trap_bridge_max) )
					rc = VOS_OK;
				break;
			case alarmType_private:
				if( (pEntry->alarmId != trap_private_min) && (pEntry->alarmId < trap_private_max) )
					rc = VOS_OK;
				break;
			case alarmType_bcmcmcctrl:
				if( (pEntry->alarmId != trap_cmcctrl_min) && (pEntry->alarmId < trap_cmcctrl_max) )
					rc = VOS_OK;
				break;
			case alarmType_other:
				if( (pEntry->alarmId != trap_mib2_min) && (pEntry->alarmId < other_eventlog_max) )
					rc = VOS_OK;
				break;
			default:
				break;
		}
	}
	return rc;
}

#define ALARMID_NUM  32
#define ALM_STR_LEN  40


CHAR ALARMIDToSTRING_ARRAY[ALARMID_NUM][ALM_STR_LEN] = {
														{"in_volt"},
														{"pwu_stat"},
														{"pwu_fan"},
														{"cur_limit"},
														{"ac_volt"},
														{"high_volt"},
														{"out_cur"},
														{"low_line"},
														{"pwu_reg"},
														{"dc_volt"},
														{"power_off"},
														{"temp_high"},
													    };

CHAR *pwuAlarmIdToString(ULONG alarmid)
{
	int i, j;
	ULONG len = 0;
	static char alarmstring[ALM_STR_LEN] = {0};
	
	VOS_MemZero(alarmstring, ALM_STR_LEN);

	for(i = 0; i<ALARMID_NUM; i++)
	{
		if((alarmid & (1<<i)) && (len < ALM_STR_LEN))
		{
			len += VOS_Sprintf(alarmstring, "%s", &ALARMIDToSTRING_ARRAY[i][0]);
		}
	}

	return alarmstring;
	
}

ULONG ulEventLogTimeDebug = 0;   /*add for PR36736 reserved debug info @muqw*/

/* modified by xieshl 20120427, 该函数在命令行、snmp等多处被调用，解决重入问题 */
CHAR *eventLogDataToStrings( nvramEventLogData_t *pEntry, CHAR *logDesc )
{
	static CHAR logDescBak[MAXLEN_EVENT_DESC+1];
	ULONG logLen = 0;
	/*CHAR  tmp[256];
	  ULONG slot, fan_no;*/
	alarmSrc_t  *pAlmSrc =  (alarmSrc_t *)pEntry->alarmSrcData, *pSrcrx = NULL;
	char szIpAddr[4] = {0,0,0,0};
	int slot = 0, port= 0, partnerSlot = 0, partnerPort = 0;
	ULONG ulIpAddr = 0,ulUdpPort = 0;
	char getRemoteInfo = FALSE;

	/*VOS_MemZero( __logDesc, MAXLEN_EVENT_DESC );*/
	/*tmp[0] = 0;*/
	if( logDesc == NULL )
		logDesc = logDescBak;
	logDesc[0] = 0;

	if( pAlmSrc == NULL )
	{
		VOS_ASSERT(0);
		return logDesc;
	}
	if( checkEventLogData(pEntry) == VOS_ERROR )
	{
		return logDesc;
	}

	if(ulEventLogTimeDebug) sys_console_printf("\r\n %s %d alarmTime:0x%x\r\n", __FUNCTION__, __LINE__, *(ULONG *)&(pEntry->alarmTime));

	switch( pEntry->alarmType )
	{
		case alarmType_mib2:
			if( pEntry->alarmId < trap_mib2_max )
			{
				logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
				logLen += getEventLogDevName( pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );
				logLen += VOS_Sprintf( &logDesc[logLen], mib2_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
			}
			break;
		case alarmType_bridge:
			if( pEntry->alarmId < trap_bridge_max )
			{
				logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
				logLen += getEventLogDevName( pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );
				logLen += VOS_Sprintf( &logDesc[logLen], bridge_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
			}
			break;
		case alarmType_bcmcmcctrl:
			if( pEntry->alarmId < trap_cmcctrl_max )
			{
				ULONG ulDevIdx = MAKEDEVID(pAlmSrc->commAlarmSrc.brdIdx, pAlmSrc->commAlarmSrc.portIdx, pAlmSrc->commAlarmSrc.onuIdx);

				logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
				logLen += getEventLogDevName( ulDevIdx, &logDesc[logLen] );
				if ( pAlmSrc->commAlarmSrc.data[0] & 1 )
				{
					logLen += VOS_Sprintf( &logDesc[logLen], mib4_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
				}
				else
				{
					logLen += VOS_Sprintf( &logDesc[logLen], "(%s)%s", macAddress_To_Strings(pAlmSrc->commAlarmSrc.data), mib4_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
				}
			}
			break;
		case alarmType_private:
			/*if( pEntry->alarmId < trap_private_max )
			  {*/
			switch( pEntry->alarmId )
			{
				/* deviceIndex */
				case trap_onuNotPresent:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName( pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], "%s%s", private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc, onuNotPresentReasonToString(pAlmSrc) );
					break;
				case trap_onuNewRegSuccess:
				case trap_onuReregSuccess:
				case trap_devPowerOff:
				case trap_devPowerOn:
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

				case trap_deviceColdStart:			/* added 20070703 */
				case trap_deviceWarmStart:
				case trap_deviceExceptionRestart:
				case trap_sysfileUploadsuccess:/*add by shixh20090604*/
				case trap_sysfileUploadfailure:
				case trap_sysfileDownloadsuccess:
				case trap_sysfileDownloadfailure:
				case trap_onuMacTableOverFlow:  /*add by shixh20111227*/
				case trap_onuMacTableOverFlowClear:
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )
				case trap_onuAutoLoadConfigSuccess:/*add by shixh@20090218*/
				case trap_onuAutoLoadConfigFailure:
				case trap_onuAutoLoadUpgradeSuccess:
				case trap_onuAutoLoadUpgradeFailure:
#endif
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName( pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					break;
#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
				case trap_deviceTemperatureHigh:
				case trap_deviceTemperatureHighClear:
				case trap_deviceTemperatureLow:
				case trap_deviceTemperatureLowClear:
					{
						int temperature_val;
						logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
						logLen += getEventLogDevName(pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );

						/*VOS_StrCat( logDesc, private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );*/
						temperature_val = (int)((pAlmSrc->devAlarmSrc.devData >> 8) & 0xff);
						if( temperature_val )
							logLen += VOS_Sprintf( &logDesc[logLen], "%s(%d)", private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc, temperature_val );
						else
							logLen += VOS_Sprintf( &logDesc[logLen], "%s", private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					}
					break;
#endif
					/* deviceIndex, boardIndex */
				case trap_swBoardProtectedSwitch:
				case trap_ponBoardReset:
					/*add by shixh@20080831*/
				case trap_boardCpuUsageAlarm:
				case trap_boardCpuUsageAlarmClear:
				case trap_boardMemoryUsageAlarm:
				case trap_boardMemoryUsageAlarmClear:
					/* deviceIndex, boardIndex, curBoardType */
				case trap_devBoardInterted:
				case trap_devBoardPull:
					/* deviceIndex, boardIndex */
				case trap_powerOffAlarm:
				case trap_powerOnAlarm:
				case trap_boardLosAlarm:
				case trap_boardLosAlarmClear:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName( pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], "slot%d %s", pEntry->alarmSrcData[4],
						private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					break;

				/*add by @muqw 2017-4-24*/
				case trap_pwuStatusAbnoarmal:
				case trap_pwuStatusAbnoarmalClear: /*需要修改打印信息*/
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName( pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], "slot%d %s(%s)", pEntry->alarmSrcData[4],
						private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc, pwuAlarmIdToString(pAlmSrc->monAlarmSrc.monValue));
					break;

				case trap_boardTemperatureHigh:	/* modified by xieshl 20121017, 6900/6900M上支持板卡问题告警 */
				case trap_boardTemperatureHighClear:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName( pAlmSrc->monAlarmSrc.devIdx, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], "slot%d %s(%d)", pAlmSrc->monAlarmSrc.brdIdx,
							private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc,
							pAlmSrc->monAlarmSrc.monValue );

					break;

					/* deviceIndex, devFanIndex */
				case trap_devFanAlarm:
				case trap_devFanAlarmClear:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName( pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );
					/* modified by xieshl 20121211, 问题单16144 */
					if( pAlmSrc->fanAlarmSrc.fanIdx == 0 )
					{
						/* 兼容老的数据格式 */
						ULONG slotno = FANID2SLOTNO( pAlmSrc->fanAlarmSrc.brdIdx );
						if( slotno == SYS_MASTER_ACTIVE_SLOTNO )
							logLen += VOS_Sprintf( &logDesc[logLen], "FAN %d %s", pAlmSrc->fanAlarmSrc.brdIdx, private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
						else
							logLen += VOS_Sprintf( &logDesc[logLen], "FAN %d/%d %s", slotno, FANID2FANNO(pAlmSrc->fanAlarmSrc.brdIdx),
									private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					}
					else
					{
						/* 6900系列设备中风扇有槽位号，6700风扇槽位为主控槽位号 */
						if( pAlmSrc->fanAlarmSrc.brdIdx == SYS_MASTER_ACTIVE_SLOTNO )
							logLen += VOS_Sprintf( &logDesc[logLen], "FAN %d %s", pAlmSrc->fanAlarmSrc.fanIdx, private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
						else
							logLen += VOS_Sprintf( &logDesc[logLen], "FAN %d/%d %s", pAlmSrc->fanAlarmSrc.brdIdx, pAlmSrc->fanAlarmSrc.fanIdx,
									private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					}
					break;

					/* deviceIndex, ponPortBrdIndex, ponPortIndex, ponPortBER */
				case trap_ponPortBERAlarm:
				case trap_ponPortBERAlarmClear:
				case trap_ponPortFERAlarm:
				case trap_ponPortFERAlarmClear:

					/* added by xieshl 20080812 */	
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
					/* end 20080812 */
					/*add by shixh@20080820*/
				case trap_ponTemperatureTooHigh:
				case trap_ponTemperatureTooHighClear:
				case trap_ponTemperatureTooLow:
				case trap_ponTemperatureTooLowClear:
					/*end shixh@20080820*/
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName(pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );

					/* B--added by liwei056@2010-1-21 for Alertm-BUG */
					if( 1 == pAlmSrc->devAlarmSrc.devIdx )
						/* E--added by liwei056@2010-1-21 for Alertm-BUG */
					{
						logLen += VOS_Sprintf( &logDesc[logLen], "pon%d/%d %s", pEntry->alarmSrcData[4], pEntry->alarmSrcData[5], 
								private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					}
					else
						logLen += VOS_Sprintf( &logDesc[logLen], "pon %s", private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

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
					{
						/*ULONG ifIndex;
						  ULNG namelen=16;*/
						logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
						logLen += getEventLogDevName( pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );

						if( 1 == pAlmSrc->devAlarmSrc.devIdx )
						{
							/*ifIndex=userSlot_userPort_2_Ifindex( pEntry->alarmSrcData[4], pEntry->alarmSrcData[5] );
							  if(IFM_GetIfNameApi(ifIndex,tmp,namelen)==VOS_ERROR)
							  {
							  logLen += VOS_Sprintf( &logDesc[logLen], "pon%d/%d", pEntry->alarmSrcData[4], pEntry->alarmSrcData[5] );
							  }*/
							logLen += VOS_Sprintf( &logDesc[logLen], "if-eth%d/%d %s", pEntry->alarmSrcData[4], pEntry->alarmSrcData[5], 
									private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
						}
						else
							logLen += VOS_Sprintf( &logDesc[logLen], "if-eth %s", private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

					}
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
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName(pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					break;
				case trap_ctcOnuBatteryVoltLow:
				case trap_ctcOnuBatteryVoltLowClear:
				case trap_ctcOnuTemperatureHigh:
				case trap_ctcOnuTemperatureHighClear:
				case trap_ctcOnuTemperatureLow:
				case trap_ctcOnuTemperatureLowClear:
					{
						/*alarmSrc_t  *pSrc = (alarmSrc_t*)pEntry->alarmSrcData;*/
						LONG alarm=0,clear=0;

						if(trap_ctcOnuBatteryVoltLow||trap_ctcOnuBatteryVoltLowClear)
							CTCONU_GetAlarmThreshold(BATTERY_VOLT_LOW, &alarm, &clear);

						if(trap_ctcOnuTemperatureHigh||trap_ctcOnuTemperatureHighClear)
							CTCONU_GetAlarmThreshold(ONU_TEMP_HIGH_ALARM, &alarm, &clear);

						if(trap_ctcOnuTemperatureLow||trap_ctcOnuTemperatureLowClear)
							CTCONU_GetAlarmThreshold(ONU_TEMP_LOW_ALARM, &alarm, &clear);

						logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
						logLen += getEventLogDevName( pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );

						logLen += VOS_Sprintf( &logDesc[logLen], "temp is %d clear is %d %s", alarm, clear, private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

						break;
					}

				case trap_ethAutoNegFailure:
				case trap_ethAutoNegFailureClear:
				case trap_ethLos:
				case trap_ethLosCLear:
				case trap_ethFailure:
				case trap_ethFailureClear:
				case trap_ethCongestion:
				case trap_ethCongestionClear:
					{
						alarmSrc_t  *pSrc = (alarmSrc_t*)pEntry->alarmSrcData;

						logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
						logLen += getEventLogDevName(pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );

						logLen += VOS_Sprintf( &logDesc[logLen], "ctc onu eth%d/%d %s", pSrc->portAlarmSrc.brdIdx, pSrc->portAlarmSrc.portIdx,
								private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

						break;
					}
				case trap_eth_DosAttack:         /*eth port DoS Attack*/
				case trap_eth_DosAttackClear:
					{
						alarmSrc_t  *pSrc = (alarmSrc_t*)pEntry->alarmSrcData;

						logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
						logLen += getEventLogDevName( pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );

						if( OLT_DEV_ID == pAlmSrc->devAlarmSrc.devIdx )
						{
							logLen += VOS_Sprintf( &logDesc[logLen], "if-eth%d/%d %s", pSrc->monAlarmSrc.devIdx, pSrc->monAlarmSrc.portIdx,
									private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
						}
						else
							logLen += VOS_Sprintf( &logDesc[logLen], "eth port %d %s", pSrc->monAlarmSrc.portIdx,
									private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					}
					break;

					/*add by shixh@20080831*/
				case trap_oltPonReceiverPowerTooLow:
				case trap_oltPonReceiverPowerTooLowClear:
				case trap_oltPonReceiverPowerTooHigh:
				case trap_oltPonReceiverPowerTooHighClear:
					{
						alarmSrc_t *pSrc = (alarmSrc_t*)pEntry->alarmSrcData;
						logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
						logLen += getEventLogDevName(1, &logDesc[logLen] );

						logLen += VOS_Sprintf( &logDesc[logLen], "pon%d/%d from onu%d/%d/%d %s", 
								pSrc->oltRxpowerAlarmSrc.brdIdx, pSrc->oltRxpowerAlarmSrc.portIdx, pSrc->oltRxpowerAlarmSrc.brdIdx, pSrc->oltRxpowerAlarmSrc.portIdx,
								GET_ONUID(pSrc->oltRxpowerAlarmSrc.onuIdx), private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					}
					break;

					/* deviceIndex, ponPortBrdIndex, ponPortIndex, ponLlidIndex */
				case trap_llidActBWExceeding:
				case trap_llidActBWExceedingClear:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName( pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

					break;

				case trap_ponPortAbnormal:
					{
						CHAR *ponRstCodeStr[] = {
							"(host_timeout)",	/* Several Host - OLT msgs were timedout (configurable by PAS_set_system_parameters function */
							"(phy_reset)",		/* OLT sent 'reset' event, indicating it was physically reset */
							"(not_inited)"		/* OLT sent 'Not inited' msg - meaning non-Init command was sent to uninitialized OLT */
						};
						ULONG ponRstCode = *((long*)&pEntry->alarmSrcData[6]);
						logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
						logLen += getEventLogDevName(pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );

						logLen += VOS_Sprintf( &logDesc[logLen], "pon%d/%d %s", pEntry->alarmSrcData[4], pEntry->alarmSrcData[5], 
								private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
						if( ponRstCode < 3 )
							logLen += VOS_Sprintf( &logDesc[logLen], ponRstCodeStr[ponRstCode] );
						else
							logLen += VOS_Sprintf( &logDesc[logLen], "(%d)", ponRstCode );
					}
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
				case trap_ponPortlosAlarm:       /*add by shixh20090626*/
				case trap_ponPortlosAlarmClear:
				case trap_ponFWVersionMismatch:/*add by shixh20090710*/
				case trap_ponFWVersionMatch:
				case trap_ponDBAVersionMismatch:
				case trap_ponDBAVersionMatch:
				case trap_ponSFPTypeMismatch:      
				case trap_ponSFPTypeMitch:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName( pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], "pon%d/%d %s", pEntry->alarmSrcData[4], pEntry->alarmSrcData[5],
							private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

					break;

				case trap_ponLaserAlwaysOnAlarm:		/* added by xieshl 20080812 */
					pSrcrx = (alarmSrc_t*)pEntry->alarmSrcData;
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], "onu%d/%d/%d %s:%d.%d dbm",pSrcrx->oltRxpowerAlwaysOnAlarmSrc.brdIdx, 
							pSrcrx->oltRxpowerAlwaysOnAlarmSrc.portIdx, pSrcrx->oltRxpowerAlwaysOnAlarmSrc.onuIdx,
							private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc,
							decimal2_integer_part(pSrcrx->oltRxpowerAlwaysOnAlarmSrc.dbm),
							decimal2_fraction_part(pSrcrx->oltRxpowerAlwaysOnAlarmSrc.dbm) );

					break;

				case trap_ponLaserAlwaysOnAlarmClear:
					pSrcrx = (alarmSrc_t*)pEntry->alarmSrcData;
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					/*logLen += getEventLogDevName(1, &logDesc[logLen]) );*/
					logLen += VOS_Sprintf( &logDesc[logLen], "onu%d/%d/%d %s",pSrcrx->oltRxpowerAlwaysOnAlarmSrc.brdIdx, 
							pSrcrx->oltRxpowerAlwaysOnAlarmSrc.portIdx, pSrcrx->oltRxpowerAlwaysOnAlarmSrc.onuIdx,
							private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					break;
				case trap_ponportBRASAlarm:       /*add by shixh20090715*/
				case trap_ponportBRASAlarmClear:	
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					/*logLen += getEventLogDevName(1, &logDesc[logLen]) );*/

					logLen += VOS_Sprintf( &logDesc[logLen], "pon%d/%d find illegal MAC(%s) %s", pAlmSrc->commAlarmSrc.brdIdx, 
							pAlmSrc->commAlarmSrc.portIdx, macAddress_To_Strings(pAlmSrc->commAlarmSrc.data),
							private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

					break;

					/* devIndex, boardIndex, portIndex */
				case trap_ethFlrAlarm:
				case trap_ethFlrAlarmClear:
				case trap_ethFerAlarm:
				case trap_ethFerAlarmClear:
				case trap_ethTranmittalIntermitAlarm:
				case trap_ethTranmittalIntermitAlarmClear:
					{
						alarmSrc_t *pSrc = (alarmSrc_t*)pEntry->alarmSrcData;

						logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
						logLen += getEventLogDevName(pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );

						logLen += VOS_Sprintf( &logDesc[logLen], "if-eth%d/%d %s", pSrc->portAlarmSrc.brdIdx, pSrc->portAlarmSrc.portIdx,
								private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

						break;
					}
					/* modified by xieshl 20110705, 增加带外网管口告警，问题单12083 */
				case trap_ethLinkdown:
				case trap_ethLinkup:
					{
						alarmSrc_t *pSrc = (alarmSrc_t*)pEntry->alarmSrcData;

						logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
						logLen += getEventLogDevName(pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );

						if( pSrc->portAlarmSrc.portIdx )
						{
							logLen += VOS_Sprintf( &logDesc[logLen], "if-eth%d/%d %s", pSrc->portAlarmSrc.brdIdx, pSrc->portAlarmSrc.portIdx,
									private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
						}
						else
						{
							logLen += VOS_Sprintf( &logDesc[logLen], "if-mgt(NM-E) %s",
									private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
						}

						break;
					}

				case trap_onuRegAuthFailure:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					/*logLen += getEventLogDevName(1, &logDesc[logLen]) );*/

					if( (pAlmSrc->commAlarmSrc.data[0] != 0) && (pAlmSrc->commAlarmSrc.data[1] == 0) && (pAlmSrc->commAlarmSrc.data[2] == 0) &&
							(pAlmSrc->commAlarmSrc.data[3] == 0) && (pAlmSrc->commAlarmSrc.data[4] == 0) && (pAlmSrc->commAlarmSrc.data[5] == 0) )
					{	/* modified by xieshl 20100610, loid auth mode */
						logLen += VOS_Sprintf( &logDesc[logLen], "pon%d/%d:detected illegal ONU(%s)", pAlmSrc->commAlarmSrc.brdIdx, 
								pAlmSrc->commAlarmSrc.portIdx, 
								onu_auth_result_to_str(pAlmSrc->commAlarmSrc.data[0]) );
					}
					else		/* mac auth mode */
					{
						logLen += VOS_Sprintf( &logDesc[logLen], "pon%d/%d:detect illegal ONU(%s)", pAlmSrc->commAlarmSrc.brdIdx, 
								pAlmSrc->commAlarmSrc.portIdx, 
								macAddress_To_Strings(pAlmSrc->commAlarmSrc.data) );
					}
					/*VOS_StrCat( logDesc, private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );*/

					break;

				case trap_onuDeletingNotify:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], "onu%d/%d/%d(%s) %s", pAlmSrc->commAlarmSrc.brdIdx, pAlmSrc->commAlarmSrc.portIdx, 
							pAlmSrc->commAlarmSrc.onuIdx, macAddress_To_Strings(pAlmSrc->commAlarmSrc.data),
							private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					break;

					/*add by shixh@20071008*/
					/*devIdx, brdIdx, E1ClusterIdx, portIdx*/
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
				case trap_E1OutOfService:    /*add by shixh@20090318*/
				case trap_E1OutOfServiceClear:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName(pAlmSrc->portAlarmSrc.devIdx, &logDesc[logLen] );
					if( pEntry->alarmSrcData[5] == 0 )
					{
						logLen += VOS_Sprintf( &logDesc[logLen], "E1-port%d/0 %s", pAlmSrc->portAlarmSrc.brdIdx,
								private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					}
					else
					{
						/* begin: modified by jianght 20090527 不显示E1簇*/
						logLen += VOS_Sprintf( &logDesc[logLen], "E1-port%d/%d %s", pAlmSrc->portAlarmSrc.brdIdx, pAlmSrc->portAlarmSrc.portIdx,
								private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
						/* end: modified by jianght 20090527 */
					}

					break;

				case trap_tdmServiceAbortAlarm:
				case trap_tdmServiceAbortAlarmClear:

					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName(pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
					break;
					/*end add by shixh@20071008*/

					/*added by xieshl 20080116*/
				case trap_ethLoopAlarm:
				case trap_ethLoopAlarmClear:
					{
						alarmSrc_t *pSrc = (alarmSrc_t*)pEntry->alarmSrcData;

						logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
						logLen += getEventLogDevName(pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );

						logLen += VOS_Sprintf( &logDesc[logLen], "if-eth%d/%d %s", pSrc->portAlarmSrc.brdIdx, pSrc->portAlarmSrc.portIdx,
								private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

						break;
					}
				case trap_onuLoopAlarm:
				case trap_onuLoopAlarmClear:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName(pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

					break;
					/* end 20080116 */

					/*add by shixh20090520*/
				case trap_SwitchEthPortLoop:
				case trap_SwitchEthPortLoopClear:

				case trap_switchEthIngressLimitExceed:
				case trap_switchEthIngressLimitExceedClear:
				case trap_switchEthEgressLimitExceed:
				case trap_switchEthEgressLimitExceedClear:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], "onu%d/%d/%d eth%d/%d switch-%s port%d %s", pAlmSrc->onuSwitchAlarmSrc.brdId, 
							(pAlmSrc->onuSwitchAlarmSrc.ponId ? pAlmSrc->onuSwitchAlarmSrc.ponId : 16), 	/* modified by xieshl 20121016, 问题单15776 */
							pAlmSrc->onuSwitchAlarmSrc.onuId,
							pAlmSrc->onuSwitchAlarmSrc.onuBrdId,pAlmSrc->onuSwitchAlarmSrc.onuPortId,
							macAddress_To_Strings(pAlmSrc->onuSwitchAlarmSrc.switchMacAddr),
							pAlmSrc->onuSwitchAlarmSrc.reason, 
							private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

					break;

				case trap_switchNewRegSuccess:
				case trap_switchReregSuccess:
				case trap_switchNotPresent:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], "onu%d/%d/%d eth%d/%d switch-%s %s", pAlmSrc->onuSwitchAlarmSrc.brdId, 
							(pAlmSrc->onuSwitchAlarmSrc.ponId ? pAlmSrc->onuSwitchAlarmSrc.ponId : 16), 
							pAlmSrc->onuSwitchAlarmSrc.onuId,	/* modified by xieshl 20120910, 问题单15776 */
							pAlmSrc->onuSwitchAlarmSrc.onuBrdId, pAlmSrc->onuSwitchAlarmSrc.onuPortId,
							macAddress_To_Strings(pAlmSrc->onuSwitchAlarmSrc.switchMacAddr),
							private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

					break;

					/*add by shixh20090612*/
				case trap_ethPortBroadCastFloodControl:
				case  trap_ethPortBroadCastFloodControlClear:
					{
						alarmSrc_t *pSrc = (alarmSrc_t*)pEntry->alarmSrcData;
						logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
						logLen += getEventLogDevName(pAlmSrc->portAlarmSrc.devIdx, &logDesc[logLen] );
						logLen += VOS_Sprintf( &logDesc[logLen], "if-eth%d/%d %s", pSrc->portAlarmSrc.brdIdx, pSrc->portAlarmSrc.portIdx,
								private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
						break;
					}
				case trap_backboneEthLinkdown:/* added by shixh@20080215 */
				case trap_backboneEthLinkup:
					{
						alarmSrc_t *pSrc = (alarmSrc_t*)pEntry->alarmSrcData;

						logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
						logLen += getEventLogDevName( pAlmSrc->portAlarmSrc.devIdx, &logDesc[logLen] );

						logLen += VOS_Sprintf( &logDesc[logLen], "if-eth%d/%d %s", pSrc->portAlarmSrc.brdIdx, pSrc->portAlarmSrc.portIdx,
								private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

						break;
					}


				case trap_tdmToEthLinkdown:	/* added by shixh@20080202 */
				case trap_tdmToEthLinkup:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName(pAlmSrc->portAlarmSrc.devIdx, &logDesc[logLen] );

					/*logLen += VOS_Sprintf( &logDesc[logLen], "tdm%d/%d ", pEntry->alarmSrcData[4], pEntry->alarmSrcData[5] );*/
					logLen += VOS_Sprintf( &logDesc[logLen], private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );

					break;

				case trap_userLocUpdateNotify:
					break;

				case trap_backupPonAlarm:
				case trap_backupPonAlarmClear:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += VOS_Sprintf( &logDesc[logLen], "onu%d/%d/%d pon%d/%d-(%s) %s", pAlmSrc->onuSwitchAlarmSrc.brdId, 
							(pAlmSrc->onuSwitchAlarmSrc.ponId ? pAlmSrc->onuSwitchAlarmSrc.ponId : 16), 
							pAlmSrc->onuSwitchAlarmSrc.onuId,	/* modified by xieshl 20120910, 问题单15776 */
							pAlmSrc->onuSwitchAlarmSrc.onuBrdId,
							pAlmSrc->onuSwitchAlarmSrc.onuPortId,
							macAddress_To_Strings(pAlmSrc->onuSwitchAlarmSrc.switchMacAddr),
							private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc);

					break;
				case trap_logicalSlotInsert:
				case trap_logicalSlotPull:
					get_ipdotstring_from_long(szIpAddr, pAlmSrc->logicalSlotAlarmSrc.ipAddr);
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += VOS_Sprintf(&logDesc[logLen], "slot%d(%s:%d) %s", 
							pAlmSrc->logicalSlotAlarmSrc.brdIdx, 
							szIpAddr,
						  	pAlmSrc->logicalSlotAlarmSrc.udpPort,
							private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc);

					break;

				case trap_ponProtectSwitch:
					logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
					logLen += getEventLogDevName( OLT_DEV_ID, &logDesc[logLen] );
					if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
					{
						slot = pAlmSrc->ponSwitchAlarmSrc.partnerBrdIdx; //logical slot id
						port = pAlmSrc->ponSwitchAlarmSrc.partnerPonIdx; //logical slot port
						if(VOS_OK == devsm_remote_port_getremoteport(slot, port, &partnerSlot, &partnerPort) && 
								VOS_OK == devsm_remote_slot_getremoteaddr(slot, &ulIpAddr, &ulUdpPort))
						{
							get_ipdotstring_from_long(szIpAddr, ulIpAddr);
							logLen += VOS_Sprintf(&logDesc[logLen], "From pon%hd/%d(%s:%d) to pon%d/%d %s",
									partnerSlot,
									partnerPort,
									szIpAddr,
									ulUdpPort,
									pAlmSrc->ponSwitchAlarmSrc.brdIdx,
									pAlmSrc->ponSwitchAlarmSrc.ponIdx,
									private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc
									);
							getRemoteInfo = TRUE;
						}
						else
						{
							getRemoteInfo = FALSE;
						}
					}

					if(!SYS_LOCAL_MODULE_ISMASTERACTIVE || !getRemoteInfo)
					{
						logLen += VOS_Sprintf(&logDesc[logLen], "From logical pon%d/%d to pon%d/%d %s",
								pAlmSrc->ponSwitchAlarmSrc.partnerBrdIdx,
								pAlmSrc->ponSwitchAlarmSrc.partnerPonIdx,
								pAlmSrc->ponSwitchAlarmSrc.brdIdx,
								pAlmSrc->ponSwitchAlarmSrc.ponIdx,
								private_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc
								);
					}
					break;

				default:
					break;

			}
			/*}*/
			break;


		case alarmType_other:
			if( pEntry->alarmId < other_eventlog_max )
			{
				char *errStr = "ERROR ";
				logLen += nvramLogTimeToStrings( &pEntry->alarmTime, &logDesc[logLen] );
				logLen += getEventLogDevName(pAlmSrc->devAlarmSrc.devIdx, &logDesc[logLen] );

				/* added by chenfj 2009-2-19
				   新增PON端口版本是否匹配告警，由于有端口索引(slot/port)
				   需求，故单独处理
				   */
				switch (pEntry->alarmId)
				{
					case other_diagnosis_success:
					case other_diagnosis_fail:
					case other_onustp_topologychange:
					case other_onustp_newroot:
					case other_ctrlchan_success:
					case other_ctrlchan_fail:
						break;
					case other_reserve1:
					case other_reserve2:
					case other_pon_cni_ber:
					case other_pon_cni_ber_clear:
					case other_pon_update_file:
						logLen += VOS_Sprintf( &logDesc[logLen], "pon%d/%d ", pAlmSrc->portAlarmSrc.brdIdx, pAlmSrc->portAlarmSrc.portIdx );
						break;
					case other_pon_slave_ok:
					case other_pon_slave_fail:
						/*added by by liyang @2015-07-14 for PON保护倒换 */
						logLen += VOS_Sprintf( &logDesc[logLen], "pon%d/%d ", pAlmSrc->portAlarmSrc.brdIdx, pAlmSrc->portAlarmSrc.portIdx );
						break;
					case other_snmp_reset_dev:
						if( pAlmSrc->otherAlarmSrc.result == VOS_ERROR )
						{
							logLen += VOS_Sprintf( &logDesc[logLen], "%s", errStr );
						}
						break;
					case other_snmp_reset_brd:
						logLen += VOS_Sprintf( &logDesc[logLen], "slot%d ", pAlmSrc->otherAlarmSrc.brdIdx );
						if( pAlmSrc->otherAlarmSrc.result == VOS_ERROR )
						{
							logLen += VOS_Sprintf( &logDesc[logLen], "%s", errStr );
						}
						break;
					case other_snmp_reset_pon:
						logLen += VOS_Sprintf( &logDesc[logLen], "pon%d/%d ", pAlmSrc->otherAlarmSrc.brdIdx, pAlmSrc->otherAlarmSrc.portIdx );
						if( pAlmSrc->otherAlarmSrc.result == VOS_ERROR )
						{
							logLen += VOS_Sprintf( &logDesc[logLen], "%s", errStr );
						}
						break;
					case other_snmp_save_config:
					case other_snmp_erase_config:
						if( pAlmSrc->otherAlarmSrc.result == VOS_ERROR )
						{
							logLen += VOS_Sprintf( &logDesc[logLen], "%s", errStr );
						}
						break;
					default:
						logLen += VOS_Sprintf( &logDesc[logLen], "unknown alarm id=%d", pEntry->alarmId );
						return logDesc;
				}
				logLen += VOS_Sprintf( &logDesc[logLen], "%s", other_EventLog_MapInfo[pEntry->alarmId].pEventLogDesc );
			}
			break;
		default:
			break;
	}

	return logDesc;
}
/*功能:将时间结构体输出为字符串格式*/
ULONG nvramLogTimeToStrings( nvramLogDateAndTime_t *pTime, CHAR *str )
{
	ULONG len = 0;
	if( (str == NULL) || (pTime == NULL) )
		return len;
	
	if(ulEventLogTimeDebug) sys_console_printf("\r\n %s %d alarmTime:0x%x\r\n", __FUNCTION__, __LINE__, *(ULONG *)pTime);
	
	len = VOS_Sprintf( str, "20%02d-%02d-%02d,%02d:%02d:%02d ", pTime->year, pTime->month, 
			 pTime->day, pTime->hour, pTime->minute, pTime->second );

	return len;
}

static ULONG getEventLogDevName( ULONG devIdx, CHAR *tmp )
{
	ULONG len = 0;
	CHAR onu_devidx_str[16];
	ULONG compatible_devIdx;

	if( tmp == NULL )
		return len;

	/* modified by xieshl 20120113, 为了兼容B1/R09及其之前版本，仍需要用旧的方式解析ONU设备索引，
	注意不要替换回GET_PONSLOT等宏定义，问题单14479 */
#if 0
	if( devIdx != OLT_DEV_ID )
	{
		ULONG  slot, port, onuId;
		short int PonPortIdx; 
		short int OnuIdx;
		
		slot = GET_PONSLOT(devIdx);
		port = GET_PONPORT(devIdx);
		onuId = GET_ONUID(devIdx);

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		if( !OLT_LOCAL_ISVALID(PonPortIdx) || !ONU_IDX_ISVALID(OnuIdx) )
		{
			/* 注意: 为了兼容B1xx设备索引定义，这里不能用宏GET_PONSLOT、GET_PONPORT、GET_ONUID */
			slot = devIdx / 10000;	/* GET_PONSLOT */
			port = (devIdx % 10000) / 1000;	/* GET_PONPORT */
			onuId = devIdx % 1000;		/* GET_ONUID */
			PonPortIdx = GetPonPortIdxBySlot( slot, port );
			OnuIdx = onuId - 1;
			
			if( !OLT_LOCAL_ISVALID(PonPortIdx) || !ONU_IDX_ISVALID(OnuIdx) )
			{
				len = VOS_Sprintf( tmp, "DevIdx(%d) ", devIdx );
				return len;
			}
			devIdx = MAKEDEVID(slot, port, onuId);
		}
	}
#else
	if( devIdx != OLT_DEV_ID )
	{
		compatible_devIdx = dev_idx_compatible_get( devIdx );
		if( compatible_devIdx == 0 )
		{
			len = VOS_Sprintf( tmp, "DevIdx(%d) ", devIdx );
			return len;
		}
		devIdx = compatible_devIdx;
	}
#endif
	if( getDeviceName(devIdx, tmp, &len) == VOS_OK )
	{
		if( len > 16 )
		{
			len = 16;
		}
		tmp[len] = 0;
	}
	else
		len = 0;

	if( len == 0 )
	{
		if( devIdx == OLT_DEV_ID )
			VOS_StrCpy( tmp, "OLT" );
		else
			VOS_StrCpy( tmp, "ONU" );
		len += 3;
	}
	if( devIdx != OLT_DEV_ID )
	{
		len += VOS_Sprintf( onu_devidx_str, "(%d/%d/%d)", GET_PONSLOT(devIdx), GET_PONPORT(devIdx), GET_ONUID(devIdx) );
		VOS_StrCat( tmp, onu_devidx_str );
	}
	/*len = VOS_StrLen(tmp);*/
	tmp[len++] = ' ';
	tmp[len] = 0;

	return len;
}

/*----------------------------------------------------------------------------*/

/* 告警日志变量 */
ULONG 	eventLogImportEnable = EVENTLOG_ENABLE;		/* 告警日志使能 */
ULONG 	eventLogEnable = EVENTLOG_ENABLE;

static nvramEventLogData_t	*eventLogImportTbl = NULL;		/* 告警日志管理表 *//*add by shixh20090303*/
static nvramEventLogHead_t *eventLogImportHead = NULL;

static nvramEventLogData_t	*eventLogTbl = NULL;		/* 告警日志管理表 */
static nvramEventLogHead_t *eventLogHead = NULL;

static ULONG  maxNumEventLogImportRecords = 0;
static ULONG  maxNumEventLogRecords = 0;

/* added by xieshl 20130318, 支持alarm log保存flash */
extern LONG ( *cdsms_file_alarm_log_read ) ( const char *name, CHAR *, LONG * );
extern LONG ( *cdsms_file_alarm_log_write ) ( const char *name, CHAR *, LONG * );
extern LONG ( *cdsms_file_alarm_log_erase ) ( const char *name );
/*调用:命令行；当没有nvram时定时保存到flash操作*/
LONG eventLogFlashFileSave()
{
	static ULONG backupLogIdx = 0;
	LONG rc = VOS_ERROR;
	LONG len = NVRAM_EVENTBACKUP_LEN;
	int lock_id;

	if( (eventLogFlashFileFlag & eventLogFlashFileEnable) == 0 )/*是否支持保存到flash；保存log使能标志*/
		return rc;
	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return rc;
	
	eventLogFlashFileTimer = 0;
	
	rc = VOS_SemTake( eventSemId, WAIT_FOREVER );

	if( backupLogIdx != eventLogHead->eventLogCurIndex )
	{
		lock_id = VOS_IntLock();
		VOS_TaskLock();
		
		if( cdsms_file_alarm_log_write != NULL )
		{
			rc = ( *cdsms_file_alarm_log_write )( eventLogFlashFileName, pNvramEventDataBuff, &len );
		}
		VOS_TaskUnlock();
		VOS_IntUnlock(lock_id);

		backupLogIdx = eventLogHead->eventLogCurIndex;
	}
	else
	{
		if( (eventLogImportHead->eventLogCurSuffix == 0) && (eventLogImportHead->eventLogHeadSuffix == 0) &&
			(eventLogHead->eventLogCurSuffix == 0) && (eventLogHead->eventLogHeadSuffix == 0) )
		{
			lock_id = VOS_IntLock();
			VOS_TaskLock();
		
			if( cdsms_file_alarm_log_erase != NULL )
			{
				rc = ( *cdsms_file_alarm_log_erase )( eventLogFlashFileName );
			}
			VOS_TaskUnlock();
			VOS_IntUnlock(lock_id);
		}
	}
	VOS_SemGive( eventSemId );

	return rc;
}
LONG eventLogFlashFileRestore()
{
	LONG rc = VOS_ERROR;
	LONG len = NVRAM_EVENTBACKUP_LEN;

	if( eventLogFlashFileFlag == 0 )
		return rc;
	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return rc;

	VOS_SemTake( eventSemId, WAIT_FOREVER );

	if( cdsms_file_alarm_log_read )
	{
		rc = ( *cdsms_file_alarm_log_read )( eventLogFlashFileName, pNvramEventDataBuff, &len );
	}

	VOS_SemGive( eventSemId );

	return rc;
}

LONG eventLogFlashFileErase()
{
	LONG rc = VOS_ERROR;
	int lock_id;

	/*if( !nvramEventFlashFlag )
		return rc;
	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return rc;*/
	eventLogFlashFileTimer = 0;

	VOS_SemTake( eventSemId, WAIT_FOREVER );

	if( (eventLogImportHead->eventLogCurSuffix == 0) && (eventLogImportHead->eventLogHeadSuffix == 0) &&
		(eventLogHead->eventLogCurSuffix == 0) && (eventLogHead->eventLogHeadSuffix == 0) )
	{
		lock_id = VOS_IntLock();
		VOS_TaskLock();
		if( cdsms_file_alarm_log_erase )
		{
			rc = ( *cdsms_file_alarm_log_erase )( eventLogFlashFileName );
		}
		VOS_TaskUnlock();
		VOS_IntUnlock(lock_id);
	}
	VOS_SemGive( eventSemId );

	return rc;
}

LONG nvramEventBackupPeriodDefaultSet()
{
	eventLogFlashFilePeriod = NVRAM_EVENTBACKUP_PERIOD_DEF;
	return VOS_OK;
}
ULONG nvramEventBackupPeriodDefaultGet()
{
	return NVRAM_EVENTBACKUP_PERIOD_DEF;
}
LONG nvramEventBackupPeriodSet( ULONG period )
{
	if( (period < NVRAM_EVENTBACKUP_PERIOD_MIN) || (period > NVRAM_EVENTBACKUP_PERIOD_MAX) )
		return VOS_ERROR;
	eventLogFlashFilePeriod = period;
	return VOS_OK;
}
ULONG nvramEventBackupPeriodGet()
{
	return eventLogFlashFilePeriod;
}

LONG nvramEventBackupEnableSet( ULONG en )
{
	if( (en == 0) || (en == 1) )
	{
		eventLogFlashFileEnable = en;
		return VOS_OK;
	}
	return VOS_ERROR;
}
ULONG nvramEventBackupEnableGet()
{
	return eventLogFlashFileEnable;
}


/* 告警日志数据初始化，数据清0 */
/*两个表:重要告警和所有告警*/
LONG initEventLogTbl()
{
#if 0	
	LONG rtc_type = bsp_rtc_exist();
	if( rtc_type == RTC_TYPE_DS1746 )
		NVRAM_EVENTLOG_DATA_LEN = (NVRAM_EVENTLOG_SIZE - sizeof(nvramEventLogHead_t))-sizeof(nvramEventLogData_t);
	else
		NVRAM_EVENTLOG_DATA_LEN = (NVRAM_EVENTLOG_SIZE - sizeof(nvramEventLogHead_t));

	maxNumEventLogImportRecords = NVRAM_IMPORTLOG_MAXNUM - 1;
	maxNumEventLogRecords = NVRAM_EVENTLOG_DATA_LEN / sizeof(nvramEventLogData_t) - 1;

	if( bsp_nvram_exist() && (RTC_TYPE_NULL != rtc_type) )
	{
		/*eventLogEnable = EVENTLOG_ENABLE;*/
		/*add by shixh20090303*/
		eventLogImportHead = (nvramEventLogHead_t *)NVRAM_IMPORTLOG_HEAD_ADDR;
		eventLogImportTbl = (nvramEventLogData_t *)NVRAM_IMPORTLOG_DATA_ADDR;
		if( (eventLogImportHead->eventLogFlag != EVENTLOG_FLAG) ||
	 	    (eventLogImportHead->eventLogHeadSuffix > maxNumEventLogImportRecords) ||
		    (eventLogImportHead->eventLogCurSuffix > maxNumEventLogImportRecords) )
		{
			VOS_MemZero( eventLogImportHead, sizeof(nvramEventLogHead_t) );
			VOS_MemZero( eventLogImportTbl, NVRAM_IMPORTLOG_DATA_LEN );
			eventLogImportHead->eventLogFlag = EVENTLOG_FLAG;
			
			/*eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("ALM LOG: New NVRAM in use\r\n") );*/
		}
		/*else
			eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("ALM LOG: Restore NVRAM OK\r\n") );*/
			/*end 20090303*/

		eventLogHead = (nvramEventLogHead_t *)NVRAM_EVENTLOG_HEAD_ADDR;
		eventLogTbl = (nvramEventLogData_t *)NVRAM_EVENTLOG_DATA_ADDR;
		if( (eventLogHead->eventLogFlag != EVENTLOG_FLAG) ||
	 	    (eventLogHead->eventLogHeadSuffix > maxNumEventLogRecords) ||
		    (eventLogHead->eventLogCurSuffix > maxNumEventLogRecords) )
		{
			VOS_MemZero( eventLogHead, sizeof(nvramEventLogHead_t) );
			VOS_MemZero( eventLogTbl, NVRAM_EVENTLOG_DATA_LEN );
			eventLogHead->eventLogFlag = EVENTLOG_FLAG;
			
			/*eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("ALM LOG: New NVRAM in use\r\n") );*/
		}
		/*else
			eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("ALM LOG: Restore NVRAM OK\r\n") );*/
	}
	else
	{
		/*eventLogEnable = EVENTLOG_DISABLE;*/
		/*add bysxh20090303*/
		eventLogImportHead = (nvramEventLogHead_t *)VOS_Malloc( sizeof(nvramEventLogHead_t), MODULE_EVENT );
		if( eventLogImportHead == NULL )
		{
			goto init_err;
		}
		eventLogImportTbl = (nvramEventLogData_t *)VOS_Malloc( NVRAM_IMPORTLOG_DATA_LEN, MODULE_EVENT );
		if( eventLogImportTbl == NULL )
		{
			goto init_err;
		}

		eventLogHead = (nvramEventLogHead_t *)VOS_Malloc( sizeof(nvramEventLogHead_t), MODULE_EVENT );
		if( eventLogHead == NULL )
		{
			goto init_err;
		}
		eventLogTbl = (nvramEventLogData_t *)VOS_Malloc( NVRAM_EVENTLOG_DATA_LEN, MODULE_EVENT );
		if( eventLogTbl == NULL )
		{
			goto init_err;
		}

		VOS_MemZero( eventLogImportHead, sizeof(nvramEventLogHead_t) );
		VOS_MemZero( eventLogImportTbl, NVRAM_IMPORTLOG_DATA_LEN );
		eventLogImportHead->eventLogFlag = EVENTLOG_FLAG;

		VOS_MemZero( eventLogHead, sizeof(nvramEventLogHead_t) );
		VOS_MemZero( eventLogTbl, NVRAM_EVENTLOG_DATA_LEN );
		eventLogHead->eventLogFlag = EVENTLOG_FLAG;
		/*eventDbgPrintf( EVENT_DEBUGSWITCH_LOG, ("ALM LOG: Not find NVRAM, alter SDRAM\r\n") );*/
	}

	return VOS_OK;

init_err:
	maxNumEventLogImportRecords = 0;
	eventLogImportEnable = EVENTLOG_DISABLE;
	maxNumEventLogRecords = 0;
	eventLogEnable = EVENTLOG_DISABLE;

	if( eventLogHead )
	{
		VOS_Free( eventLogHead );
		eventLogHead = NULL;
	}
	if( eventLogTbl )
	{
		VOS_Free( eventLogTbl );
		eventLogTbl = NULL;
	}

	if( eventLogImportHead )
	{
		VOS_Free( eventLogImportHead );
		eventLogImportHead = NULL;
	}
	if( eventLogImportTbl )
	{
		VOS_Free( eventLogImportTbl );
		eventLogImportTbl = NULL;
	}
	/*VOS_ASSERT( 0 );*/
	return VOS_ERROR;
#else

	LONG rtc_type = bsp_rtc_exist();
	if( rtc_type == RTC_TYPE_DS1746 )
		NVRAM_EVENTLOG_DATA_LEN -= sizeof(nvramEventLogData_t);

	maxNumEventLogImportRecords = NVRAM_IMPORTLOG_MAXNUM - 1;
	maxNumEventLogRecords = NVRAM_EVENTLOG_DATA_LEN / sizeof(nvramEventLogData_t) - 1;

	if( !bsp_nvram_exist() || (RTC_TYPE_NULL == rtc_type) )
	{
/*	
#ifdef g_malloc
#undef g_malloc
#endif
		extern void * g_malloc( int );
*/		
		pNvramEventDataBuff = g_malloc( NVRAM_EVENTBACKUP_LEN );
		if( pNvramEventDataBuff == NULL )
			return VOS_ERROR;

		eventLogFlashFileName = "/flash/sys/alarm.dat";
		eventLogFlashFileFlag = 1;

		eventLogFlashFileRestore();
	}

	eventLogImportHead = (nvramEventLogHead_t *)NVRAM_IMPORTLOG_HEAD_ADDR;
	eventLogImportTbl = (nvramEventLogData_t *)NVRAM_IMPORTLOG_DATA_ADDR;
	if( (eventLogImportHead->eventLogFlag != EVENTLOG_FLAG) ||
		(eventLogImportHead->eventLogHeadSuffix > maxNumEventLogImportRecords) ||
		(eventLogImportHead->eventLogCurSuffix > maxNumEventLogImportRecords) )
	{
		VOS_MemZero( eventLogImportHead, sizeof(nvramEventLogHead_t) );
		VOS_MemZero( eventLogImportTbl, NVRAM_IMPORTLOG_DATA_LEN );
		eventLogImportHead->eventLogFlag = EVENTLOG_FLAG;

		/*eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("ALM LOG: New NVRAM in use\r\n") );*/
	}
	/*else
		eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("ALM LOG: Restore NVRAM OK\r\n") );*/

	eventLogHead = (nvramEventLogHead_t *)NVRAM_EVENTLOG_HEAD_ADDR;
	eventLogTbl = (nvramEventLogData_t *)NVRAM_EVENTLOG_DATA_ADDR;
	if( (eventLogHead->eventLogFlag != EVENTLOG_FLAG) ||
		(eventLogHead->eventLogHeadSuffix > maxNumEventLogRecords) ||
		(eventLogHead->eventLogCurSuffix > maxNumEventLogRecords) )
	{
		VOS_MemZero( eventLogHead, sizeof(nvramEventLogHead_t) );
		VOS_MemZero( eventLogTbl, NVRAM_EVENTLOG_DATA_LEN );
		eventLogHead->eventLogFlag = EVENTLOG_FLAG;

		/*eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("ALM LOG: New NVRAM in use\r\n") );*/
	}
	return VOS_OK;
#endif
}

/*add byshixh20090303*/
/* 数据存储区下标和告警日志表索引的对应 */
/*static LONG eventLogImportSuffix2Idx( ULONG suffix, ULONG *pIdxs )
{
	LONG rc = VOS_ERROR;
	ULONG curSuffix, headSuffix, logIdx;
	
	if( suffix > maxNumEventLogImportRecords || pIdxs == NULL )
		return rc;

	curSuffix = eventLogImportHead->eventLogCurSuffix;
	headSuffix = eventLogImportHead->eventLogHeadSuffix;
	logIdx = eventLogImportHead->eventLogCurIndex;

	if( curSuffix == suffix )
	{
		*pIdxs = logIdx;
		rc = VOS_OK;
	}
	else if( curSuffix > headSuffix )
	{
		if( curSuffix >= suffix )
		{
			*pIdxs = logIdx - (curSuffix - suffix);
			rc = VOS_OK;
		}
	}
	else if( curSuffix < headSuffix )
	{
		if( suffix >= curSuffix )
		{
			*pIdxs = logIdx - ((maxNumEventLogImportRecords + 1) + curSuffix - suffix);
			rc = VOS_OK;
		}
		else if( suffix < curSuffix )
		{
			*pIdxs = logIdx - (curSuffix - suffix);
			rc = VOS_OK;
		}
	}

	return rc;
}*/

/* 数据存储区下标和告警日志表索引的对应 */
static LONG eventLogSuffix2Idx( ULONG suffix, ULONG *pIdxs )
{
	LONG rc = VOS_ERROR;
	ULONG curSuffix, headSuffix, logIdx;
	
	if( (NULL == eventLogHead) || (0 == maxNumEventLogRecords) )
		return rc;
	curSuffix = eventLogHead->eventLogCurSuffix;
	headSuffix = eventLogHead->eventLogHeadSuffix;
	logIdx = eventLogHead->eventLogCurIndex;

	/* modified by xieshl 20121010, 检查日志表头是否合法，现场曾发生过运行过程中表头索引错误的情况 */
	if( suffix > maxNumEventLogRecords || pIdxs == NULL )
		return rc;
	if( (eventLogHead->eventLogFlag != EVENTLOG_FLAG) ||
 		(eventLogHead->eventLogHeadSuffix > maxNumEventLogRecords) ||
		(eventLogHead->eventLogCurSuffix > maxNumEventLogRecords) )
	    return rc;

	if( curSuffix == suffix )
	{
		*pIdxs = logIdx;
		rc = VOS_OK;
	}
	else if( curSuffix > headSuffix )
	{
		if( curSuffix >= suffix )
		{
			*pIdxs = logIdx - (curSuffix - suffix);
			rc = VOS_OK;
		}
	}
	else if( curSuffix < headSuffix )
	{
		if( suffix >= curSuffix )
		{
			*pIdxs = logIdx - ((maxNumEventLogRecords + 1) + curSuffix - suffix);
			rc = VOS_OK;
		}
		else if( suffix < curSuffix )
		{
			*pIdxs = logIdx - (curSuffix - suffix);
			rc = VOS_OK;
		}
	}

	return rc;
}

static LONG eventLogIdx2Suffix( ULONG idx, ULONG *pSuffix )
{
	LONG rc = VOS_ERROR;
	ULONG suffix = 0;
	ULONG curSuffix, logIdx;

	if( (NULL == eventLogHead) || (0 == maxNumEventLogRecords) )
		return rc;
	curSuffix = eventLogHead->eventLogCurSuffix;
	logIdx = eventLogHead->eventLogCurIndex;
	if( (idx > logIdx) || (NULL == pSuffix) )
		return rc;
	if( (eventLogHead->eventLogFlag != EVENTLOG_FLAG) ||
 		(eventLogHead->eventLogHeadSuffix > maxNumEventLogRecords) ||
		(eventLogHead->eventLogCurSuffix > maxNumEventLogRecords) )
	    return rc;

	suffix = (logIdx - idx);
	if( suffix <= curSuffix )
	{
		*pSuffix = curSuffix - suffix;
		rc = VOS_OK;
	}
	else if( suffix <= maxNumEventLogRecords )
	{
		*pSuffix = (maxNumEventLogRecords + 1) - (curSuffix - suffix);
		rc = VOS_OK;
	}
	return rc;
}


/*----------------------------------------------------------------------------
* 功能: 读告警日志管理表当前行的数据存储指针
* 输入参数: logIdx－当前行的日志索引
* 输出参数: 
* 返回值: 正确返回当前行数据的存储指针，错误返回NULL */
static nvramEventLogData_t* seekLogIdxInEventLogTbl( ULONG logIdx )
{
	ULONG suffix = 0;
	nvramEventLogData_t *pEntry = NULL;
	if( eventLogIdx2Suffix(logIdx, &suffix) == VOS_OK )
	{
		pEntry = &eventLogTbl[suffix];
	}
	return pEntry;
}

/*----------------------------------------------------------------------------
* 功能: 在存储区中生成一个新的存储空间，如果存储空间已经用完，则覆盖旧的存储空间
* 输入参数: 
* 输出参数: 
* 返回值: 存储空间地址指针 */
static nvramEventLogData_t* newEventLog()
{
	ULONG idx, curSuffix, headSuffix;

	if( (NULL == eventLogHead) || (0 == maxNumEventLogRecords) )
		return NULL;

	idx = eventLogHead->eventLogCurIndex;
	curSuffix = eventLogHead->eventLogCurSuffix;
	headSuffix = eventLogHead->eventLogHeadSuffix;

	if( idx != 0 )
	{
		if( curSuffix >= maxNumEventLogRecords )
			curSuffix = 0;
		else
			curSuffix++;

		if( headSuffix == curSuffix )
		{
			if( headSuffix >= maxNumEventLogRecords )
				headSuffix = 0;
			else
				headSuffix++;
		}
	}
	else
	{
		headSuffix = 0;
		curSuffix = 0;
	}

	eventLogHead->eventLogCurSuffix = curSuffix;
	eventLogHead->eventLogHeadSuffix = headSuffix;
	eventLogHead->eventLogCurIndex = idx + 1;
	/*getEventLogTime( &(eventLogTbl[curSuffix].alarmTime) );*/
		
	return &eventLogTbl[curSuffix];
}

/*add by shixh20090303*/
static nvramEventLogData_t* newEventImportLog()
{
	ULONG idx, curSuffix, headSuffix;

	if( (NULL == eventLogImportHead) || (0 == maxNumEventLogImportRecords) )
		return NULL;

	idx = eventLogImportHead->eventLogCurIndex;
	curSuffix = eventLogImportHead->eventLogCurSuffix;
	headSuffix = eventLogImportHead->eventLogHeadSuffix;
	
	if( idx != 0 )
	{
		if( curSuffix >= maxNumEventLogImportRecords )
			curSuffix = 0;
		else
			curSuffix++;

		if( headSuffix == curSuffix )
		{
			if( headSuffix >= maxNumEventLogImportRecords )
				headSuffix = 0;
			else
				headSuffix++;
		}
	}
	else
	{
		headSuffix = 0;
		curSuffix = 0;
	}

	eventLogImportHead->eventLogCurSuffix = curSuffix;
	eventLogImportHead->eventLogHeadSuffix = headSuffix;
	eventLogImportHead->eventLogCurIndex = idx + 1;
	/*getEventLogTime( &(eventLogImportTbl[curSuffix].alarmTime) );*/
		
	return &eventLogImportTbl[curSuffix];
}

/*----------------------------------------------------------------------------
* 功能: 生成告警日志
* 输入参数: pAlarmDesc－告警描述信息指针
			pAlarmData－告警原始数据指针
* 输出参数: 
* 返回值: 正确返回OK，错误返回ERROR 
* 调用:saveEventLog()*/
/*extern ULONG alarmlogToSyslog_enable;*/
extern ULONG eventLogOutMode;
static LONG saveEventLog_ToNvram( uchar_t alarmType, uchar_t alarmId, alarmSrc_t *pAlarmSrc )
{
	LONG rc = VOS_OK;
	CHAR *pLogStr, logStr[MAXLEN_EVENT_DESC+1];
	nvramEventLogData_t *pNewEntry, bacEntry;
	LONG  priority;

	pLogStr = logStr;
	
	bacEntry.alarmType = alarmType;
	bacEntry.alarmId = alarmId;
	VOS_MemCpy( bacEntry.alarmSrcData, pAlarmSrc, MAXLEN_EVENT_DATA );
	getEventLogTime( &(bacEntry.alarmTime) );
	
	if(ulEventLogTimeDebug) sys_console_printf("\r\n %s %d alarmTime:0x%x\r\n", __FUNCTION__, __LINE__, *(ULONG *)&(bacEntry.alarmTime));
	
	VOS_SemTake( eventSemId, WAIT_FOREVER );

	pNewEntry = newEventLog();
	if( pNewEntry == NULL )
	{
		VOS_SemGive( eventSemId );
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	VOS_MemCpy( pNewEntry, &bacEntry, sizeof(nvramEventLogData_t) );
	
	VOS_SemGive( eventSemId );

	if( (alarmType == alarmType_private) && (alarmId == trap_devPowerOff) && (pAlarmSrc->devAlarmSrc.devIdx == OLT_DEV_ID) )
	{
		/* OLT设备掉电告警不打印 */
	}
	else
	/*if( (alarmlogToSyslog_enable == EVENTLOG_ENABLE) || (eventDebugSwitch & EVENT_DEBUGSWITCH_LOG) )*/
	if( (eventLogOutMode & EVENT_LOG_OUT_2_ALL) || (eventDebugSwitch & EVENT_DEBUGSWITCH_LOG) )
	{
		eventLogDataToStrings(&bacEntry, pLogStr );

		if( eventDebugSwitch & EVENT_DEBUGSWITCH_LOG )
		{
			if( VOS_StrLen(pLogStr) > 60 )
			{
				pLogStr[19] = 0;
				sys_console_printf( "\r\nALM LOG: %d %s\r\n", eventLogHead->eventLogCurIndex, pLogStr );
				pLogStr += 20;
				sys_console_printf( "         %s\r\n", pLogStr );
			}
			else
				sys_console_printf( "\r\nALM LOG: %d %s\r\n", eventLogHead->eventLogCurIndex, pLogStr );
		}
	
		/*if( alarmlogToSyslog_enable == EVENTLOG_ENABLE )*/
		if( eventLogOutMode & EVENT_LOG_OUT_2_SYSLOG )
		{
		  	priority=getEventPriority(alarmType,alarmId );
			if(priority==ALM_PRI_LOW)
				VOS_SysLog(LOG_TYPE_ALARM, LOG_NOTICE, "%s", pLogStr);	/*问题单8544*/
			else
				VOS_SysLog(LOG_TYPE_ALARM, LOG_WARNING, "%s", pLogStr);	/*问题单8544*/
		}
		
		if( eventLogOutMode & EVENT_LOG_OUT_2_TELNET )
		{
			cl_vty_telnet_out( "\r\n%s\r\n", pLogStr );
		}
		if( eventLogOutMode & EVENT_LOG_OUT_2_CONSOLE )
		{
			sys_console_printf( "\r\n%s\r\n", pLogStr );
		}
	}
	
	return rc;
}
/*----------------------------------------------------------------------------
* 功能: 生成重要告警日志
* 输入参数: 告警类型；告警ID；告警源数据指针
* 输出参数: 
* 返回值: 正确返回OK，错误返回ERROR */
/*add by shixh20090303*/
static LONG saveEventImportLog_ToNvram( uchar_t alarmType, uchar_t alarmId, alarmSrc_t *pAlarmSrc )
{
	LONG rc = VOS_OK;
	CHAR *pLogStr, logStr[MAXLEN_EVENT_DESC+1];
	nvramEventLogData_t *pNewEntry, bacEntry;

	pLogStr = logStr;
	
	bacEntry.alarmType = alarmType;
	bacEntry.alarmId = alarmId;
	VOS_MemCpy( bacEntry.alarmSrcData, pAlarmSrc, MAXLEN_EVENT_DATA );
	getEventLogTime( &(bacEntry.alarmTime) );

	if( eventLogImportEnable == EVENTLOG_DISABLE )
		return rc;

	VOS_SemTake( eventSemId, WAIT_FOREVER );

	pNewEntry = newEventImportLog();
	if( pNewEntry == NULL )
	{
		VOS_SemGive( eventSemId );
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	VOS_MemCpy( pNewEntry, &bacEntry, sizeof(nvramEventLogData_t) );

	VOS_SemGive( eventSemId );

	if( eventDebugSwitch & EVENT_DEBUGSWITCH_LOG )
	{
		eventLogDataToStrings(&bacEntry, pLogStr);
		if( VOS_StrLen(pLogStr) > 60 )
		{
			pLogStr[19] = 0;
			sys_console_printf( "\r\nALM1 LOG: %d %s\r\n", eventLogImportHead->eventLogCurIndex, pLogStr );
			pLogStr += 20;
			sys_console_printf( "         %s\r\n", pLogStr );
		}
		else
			sys_console_printf( "\r\nALM1 LOG: %d %s\r\n", eventLogImportHead->eventLogCurIndex, pLogStr );
	}

	return rc;
}
/*功能:保存告警log*/
/*分为重要告警和非重要告警，分两张表维护*/
LONG saveEventLog( uchar_t alarmType, uchar_t alarmId, alarmSrc_t *pAlarmSrc)
{
	LONG  rc = VOS_OK;
 	LONG  priority;

	if( pAlarmSrc == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	
	if( eventLogEnable == EVENTLOG_DISABLE )/*告警日志使能是否打开，命令行配置*/
		return rc;

	if( getEvent2NvramFlag(alarmType, alarmId) == ALM_NVRAM_NO )
		return rc;

	/* added by xieshl 20080424, 告警日志过滤 */
	if( eventLogDataFilter == ALM_FILTER_ONU_ETH_LINK )
	{
		if( (pAlarmSrc->devAlarmSrc.devIdx != OLT_DEV_ID) && (alarmType == alarmType_private) && 
			((alarmId == trap_ethLinkup) || (alarmId == trap_ethLinkdown)) )
		{
			return rc;
		}
	}
	priority = getEventPriority(alarmType,alarmId );
	if(priority==ALM_PRI_HIGH)
	{
		saveEventImportLog_ToNvram(alarmType, alarmId, pAlarmSrc );
	}
	rc = saveEventLog_ToNvram( alarmType, alarmId, pAlarmSrc );

	return  rc;
}

/*----------------------------------------------------------------------------
* 功能: 通过命令删除一个ONU时，其告警日志也删除
* 输入参数: devIdx－ONU设备索引
* 返回值: 如果存在返回OK，不存在返回ERROR */
LONG eraseDevEventLog( ULONG devIdx )
{
	LONG rc = VOS_ERROR;
	ULONG curPtr, lastPtr/*, idx*/;
	alarmSrc_t *pAlarmSrc;

	if( devIdx == 0 )
		return rc;
	
	VOS_SemTake( eventSemId, WAIT_FOREVER );

	if( (NULL == eventLogHead) || (0 == maxNumEventLogRecords) )
	{
		VOS_SemGive( eventSemId );
		return rc;
	}
	if( (eventLogHead->eventLogFlag != EVENTLOG_FLAG) ||
	 	(eventLogHead->eventLogHeadSuffix > maxNumEventLogRecords) ||
		(eventLogHead->eventLogCurSuffix > maxNumEventLogRecords) )
	{
		VOS_MemZero( eventLogHead, sizeof(nvramEventLogHead_t) );
		VOS_MemZero( eventLogTbl, NVRAM_EVENTLOG_DATA_LEN );
		eventLogHead->eventLogFlag = EVENTLOG_FLAG;

		VOS_SemGive( eventSemId );
		return rc;
	}
	
	curPtr = eventLogHead->eventLogHeadSuffix;
	lastPtr = eventLogHead->eventLogCurSuffix;

	while( eventLogHead->eventLogCurIndex )
	{
		pAlarmSrc = (alarmSrc_t *)eventLogTbl[curPtr].alarmSrcData;

		if( pAlarmSrc->devAlarmSrc.devIdx == devIdx )
		{
			/*if( eventLogSuffix2Idx(curPtr, &idx) == VOS_OK )
				eventDbgPrintf( EVENT_DEBUGSWITCH_LOG, ( "ALM LOG: delete %d: %s\r\n", idx, eventLogDataToStrings(&eventLogTbl[curPtr])) );*/

			eventLogTbl[curPtr].alarmType = 0;

			/*if( curPtr == eventLogHead->eventLogHeadSuffix )
			{
				if( eventLogHead->eventLogHeadSuffix >= maxNumEventLogRecords )
					eventLogHead->eventLogHeadSuffix = 0;
				else
					eventLogHead->eventLogHeadSuffix++;
			}*/
		}

		if( curPtr == lastPtr )
			break;
		else
		{
			if( curPtr >= maxNumEventLogRecords )
				curPtr = 0;
			else
				curPtr++;
		}
	}
	VOS_SemGive( eventSemId );

	return VOS_OK;
}

/*add by  shixh20090303*/
LONG eraseDevEventImportLog( ULONG devIdx )
{
	LONG rc = VOS_ERROR;
	ULONG curPtr, lastPtr/*, idx*/;
	alarmSrc_t *pAlarmSrc;

	if( devIdx == 0 )
		return VOS_ERROR;

	VOS_SemTake( eventSemId, WAIT_FOREVER );

	if( (NULL == eventLogImportHead) || (0 == maxNumEventLogImportRecords) )
	{
		VOS_SemGive( eventSemId );
		return rc;
	}

	curPtr = eventLogImportHead->eventLogHeadSuffix;
	lastPtr = eventLogImportHead->eventLogCurSuffix;

	if( (eventLogImportHead->eventLogFlag != EVENTLOG_FLAG) ||
	 	(eventLogImportHead->eventLogHeadSuffix > maxNumEventLogImportRecords) ||
		(eventLogImportHead->eventLogCurSuffix > maxNumEventLogImportRecords) )
	{
		VOS_MemZero( eventLogImportHead, sizeof(nvramEventLogHead_t) );
		VOS_MemZero( eventLogImportTbl, NVRAM_EVENTLOG_DATA_LEN );
		eventLogImportHead->eventLogFlag = EVENTLOG_FLAG;

		VOS_SemGive( eventSemId );
		return rc;
	}

	while( eventLogImportHead->eventLogCurIndex )
	{
		pAlarmSrc = (alarmSrc_t *)eventLogImportTbl[curPtr].alarmSrcData;

		if( pAlarmSrc->devAlarmSrc.devIdx == devIdx )
		{
			/*if( eventLogSuffix2Idx(curPtr, &idx) == VOS_OK )
				eventDbgPrintf( EVENT_DEBUGSWITCH_LOG, ( "ALM LOG: delete %d: %s\r\n", idx, eventLogDataToStrings(&eventLogTbl[curPtr])) );*/

			eventLogImportTbl[curPtr].alarmType = 0;

			/*if( curPtr == eventLogHead->eventLogHeadSuffix )
			{
				if( eventLogHead->eventLogHeadSuffix >= maxNumEventLogRecords )
					eventLogHead->eventLogHeadSuffix = 0;
				else
					eventLogHead->eventLogHeadSuffix++;
			}*/
		}

		if( curPtr == lastPtr )
			break;
		else
		{
			if( curPtr >= maxNumEventLogImportRecords )
				curPtr = 0;
			else
				curPtr++;
		}
	}
	VOS_SemGive( eventSemId );

	return VOS_OK;
}

/*----------------------------------------------------------------------------
* 功能: 清除告警日志
* 输入参数: 
* 输出参数: 
* 返回值: 正确返回OK，错误返回ERROR */
LONG eraseAllEventLog()
{
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	if( (NULL == eventLogHead) || (0 == maxNumEventLogRecords) )
	{
		VOS_SemGive( eventSemId );
		return VOS_ERROR;
	}
	VOS_MemZero( eventLogHead, sizeof(nvramEventLogHead_t) );
	VOS_MemZero( eventLogTbl, NVRAM_EVENTLOG_DATA_LEN );
	eventLogHead->eventLogFlag = EVENTLOG_FLAG;

	eventLogFlashFileErase();
	
	VOS_SemGive( eventSemId );

	eventDbgPrintf( EVENT_DEBUGSWITCH_LOG, ("ALM LOG: erase event ok\r\n") );

	return VOS_OK;
}

/*add by shixh20090303*/
LONG eraseAllEventImportLog()
{
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	if( (NULL == eventLogImportHead) || (0 == maxNumEventLogImportRecords) )
	{
		VOS_SemGive( eventSemId );
		return VOS_ERROR;
	}

	VOS_MemZero( eventLogImportHead, sizeof(nvramEventLogHead_t) );
	VOS_MemZero( eventLogImportTbl, NVRAM_IMPORTLOG_DATA_LEN );
	eventLogImportHead->eventLogFlag = EVENTLOG_FLAG;

	eventLogFlashFileErase();

	VOS_SemGive( eventSemId );

	eventDbgPrintf( EVENT_DEBUGSWITCH_LOG, ("ALM LOG: erase event ok\r\n") );

	return VOS_OK;
}
/*----------------------------------------------------------------------------*/
/* 与MIB接口函数 */
/*----------------------------------------------------------------------------*/

/* 读告警日志使能 */
LONG getEventLogEnable( ULONG *pEnable )
{
	if( pEnable == NULL )
		return VOS_ERROR;

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	*pEnable = eventLogEnable;
	VOS_SemGive( eventSemId );

	return VOS_OK;
}

/* 设置告警日志使能 */
extern int eventSync_configEbl_2AllSlave( ULONG subType, ULONG subCode );
LONG setEventLogEnable( ULONG enable )
{
	if( (enable != EVENTLOG_ENABLE) && (enable != EVENTLOG_DISABLE) )
		return VOS_ERROR;

	if( eventLogEnable != enable )
	{
		VOS_SemTake( eventSemId, WAIT_FOREVER );
		eventLogEnable = enable;
		/*if( enable == EVENTLOG_ENABLE )
		{
		    eraseAllEventLog();
		}*/
		VOS_SemGive( eventSemId );

		eventSync_configEbl_2AllSlave( 1, enable );
   	}
	
	return VOS_OK;
}
/*----------------------------------------------------------------------------*/

/* 读告警日志最新索引 */
LONG getEventLogLastIndex( ULONG *pIndex )
{
	if( pIndex == NULL )
		return VOS_ERROR;

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	*pIndex = eventLogHead->eventLogCurIndex;
	VOS_SemGive( eventSemId );

	return VOS_OK;
}
/*----------------------------------------------------------------------------*/


/* 读告警日志索引 */
LONG getEventLogIndex( ULONG logIdx, ULONG *pLogIdx )
{
	nvramEventLogData_t *pEntry;
	if( pLogIdx == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	pEntry = seekLogIdxInEventLogTbl( logIdx );
	VOS_SemGive( eventSemId );

	if( pEntry == NULL )
		return VOS_ERROR;
	*pLogIdx = logIdx;

	return VOS_OK;
}
/*----------------------------------------------------------------------------*/

/* 读告警日志时间 */
LONG getEventLogSysTime( ULONG logIdx, sysDateAndTime_t *pLogTime )
{
	LONG rc = VOS_ERROR;
	nvramEventLogData_t *pEntry;
	if( pLogTime == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	pEntry = seekLogIdxInEventLogTbl( logIdx );
	if( pEntry )
	{
		pLogTime->year = pEntry->alarmTime.year;
		pLogTime->month = pEntry->alarmTime.month;
		pLogTime->day = pEntry->alarmTime.day;
		pLogTime->hour = pEntry->alarmTime.hour;
		pLogTime->minute = pEntry->alarmTime.minute;
		pLogTime->second = pEntry->alarmTime.second;
		if( pLogTime->year < 30 )
			pLogTime->year += 2000;
		else
			pLogTime->year += 1900;
		pLogTime->reserver[0] = 0;
		pLogTime->reserver[1] = 0;
		pLogTime->reserver[2] = 0;
		pLogTime->reserver[3] = 0;

		rc = VOS_OK;
	}
	VOS_SemGive( eventSemId );

	return VOS_OK;
}
/*----------------------------------------------------------------------------*/

/* 读告警日志描述信息 */
LONG getEventLogDesc( ULONG logIdx, uchar_t *pLogDesc )
{
	LONG rc = VOS_ERROR;
	nvramEventLogData_t *pEntry, bacEntry;
	if( pLogDesc == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	pEntry = seekLogIdxInEventLogTbl( logIdx );
	if( pEntry )
	{
		VOS_MemCpy( &bacEntry, pEntry, sizeof(nvramEventLogData_t) );
		rc = VOS_OK;
	}
	VOS_SemGive( eventSemId );

	if( rc == VOS_OK )
	{
		eventLogDataToStrings( &bacEntry, pLogDesc );
		pLogDesc[MAXLEN_EVENT_DESC-1] = 0;
	}
	return rc;
}

/*----------------------------------------------------------------------------
* 功能: 告警日志管理表第一行索引
* 输入参数: 
* 输出参数: pLogIdx－第一行的日志索引
* 返回值: 如果存在返回OK，不存在返回ERROR */
LONG getFirstEventLogTblIndex ( ULONG *pLogIdx )
{
	LONG rc;
	if( pLogIdx == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	rc = eventLogSuffix2Idx( eventLogHead->eventLogHeadSuffix, pLogIdx );
	VOS_SemGive( eventSemId );

	return rc;
}

/*----------------------------------------------------------------------------
* 功能: 告警日志管理表下一行索引
* 输入参数: logIdx－当前日志索引
* 输出参数: pNextLogIdx－下一行的日志索引
* 返回值: 如果存在返回OK，不存在返回ERROR */
LONG getNextEventLogTblIndex (ULONG logIdx, ULONG *pNextLogIdx )
{
	LONG rc = VOS_ERROR;
	nvramEventLogData_t* pEntry;

	if( pNextLogIdx == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}
	/*if( logIdx >= eventLogHead->eventLogCurIndex )
		return rc;*/

	++logIdx;

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	for( ; logIdx<=eventLogHead->eventLogCurIndex; logIdx++ )
	{
		pEntry = seekLogIdxInEventLogTbl( logIdx );
		if( pEntry == NULL )
			break;
		else if( pEntry->alarmType != 0 )
		{
			*pNextLogIdx = logIdx;
			rc = VOS_OK;
			break;
		}
	}
	VOS_SemGive( eventSemId );

	return rc;
}

/*----------------------------------------------------------------------------
* 功能: 检查索引是否正确
* 输入参数: logIdx－当前日志索引
* 输出参数: 
* 返回值: 正确返回OK，错误返回ERROR */
LONG checkEventLogTblIndex ( ULONG logIdx )
{
	LONG rc = VOS_ERROR;
	ULONG suffix;

	VOS_SemTake( eventSemId, WAIT_FOREVER );

	if( (NULL == eventLogHead) || (0 == maxNumEventLogRecords) )
	{
		VOS_SemGive( eventSemId );
		return rc;
	}
	if( (logIdx != 0) && (logIdx <= eventLogHead->eventLogCurIndex) )
	{
		if( eventLogIdx2Suffix( logIdx, &suffix ) == VOS_OK )
		{
			if( eventLogTbl[suffix].alarmType != 0 )
				rc = VOS_OK;
		}
	}
	VOS_SemGive( eventSemId );

	return rc;
}
/*----------------------------------------------------------------------------*/
/*功能:获取告警的优先级*/
/*输入:告警类型，告警ID*/
/*返回:当前告警的所在的优先级，和前面定义的一致*/
ULONG getEventPriority( ULONG alarmType, ULONG alarmId )
{
	ULONG pri = ALM_PRI_LOW;
	switch( alarmType )
	{
		case alarmType_mib2:
			if( trap_mib2_min < alarmId && trap_mib2_max > alarmId )
			{
				pri = mib2_EventLog_MapInfo[alarmId].priority;
			}
			break;
		case alarmType_bridge:
			if( trap_bridge_min < alarmId && trap_bridge_max > alarmId )
			{
				pri = bridge_EventLog_MapInfo[alarmId].priority;
			}
			break;
		case alarmType_private:
			if( trap_private_min < alarmId && trap_private_max > alarmId )
			{
				pri = private_EventLog_MapInfo[alarmId].priority;
			}
			break;
		case alarmType_bcmcmcctrl:
			if( trap_cmcctrl_min < alarmId && trap_cmcctrl_max > alarmId )
			{
				pri = mib4_EventLog_MapInfo[alarmId].priority;
			}
			break;
		case alarmType_other:
			if( (other_eventlog_min < alarmId) && (other_eventlog_max > alarmId) )/* modified by xieshl 20110216, 问题单12069 */
			{
				pri = other_EventLog_MapInfo[alarmId].priority;
			}
			break;
		default:
			break;
	}

	return pri;
}
/*获取告警的严重程度*/
/*输入:告警类型，告警ID*/
/*返回:当前告警的所在的告警级别，和前面定义的一致*/
ULONG getEventAlarmLevel( ULONG alarmType, ULONG alarmId )
{
	ULONG level = ALM_LEV_NULL;
	switch( alarmType )
	{
		case alarmType_mib2:
			if( trap_mib2_min < alarmId && trap_mib2_max > alarmId )
			{
				level = mib2_EventLog_MapInfo[alarmId].alarmLevel;
			}
			break;
		case alarmType_bridge:
			if( trap_bridge_min < alarmId && trap_bridge_max > alarmId )
			{
				level = bridge_EventLog_MapInfo[alarmId].alarmLevel;
			}
			break;
		case alarmType_private:
			if( trap_private_min < alarmId && trap_private_max > alarmId )
			{
				level = private_EventLog_MapInfo[alarmId].alarmLevel;
			}
			break;
		case alarmType_bcmcmcctrl:
			if( trap_cmcctrl_min < alarmId && trap_cmcctrl_max > alarmId )
			{
				level = mib4_EventLog_MapInfo[alarmId].alarmLevel;
			}
			break;
		case alarmType_other:
			break;
		default:
			break;
	}

	return level;
}
/*设置告警的严重程度*/
LONG setEventAlarmLevel( ULONG alarmType, ULONG alarmId, ULONG level )
{
	if( (level == ALM_LEV_NULL) || (level >= ALM_LEV_MAX) )
		return VOS_ERROR;
	switch( alarmType )
	{
		case alarmType_mib2:
			if( trap_mib2_min < alarmId && trap_mib2_max > alarmId )
			{
				mib2_EventLog_MapInfo[alarmId].alarmLevel = level;
			}
			break;
		case alarmType_bridge:
			if( trap_bridge_min < alarmId && trap_bridge_max > alarmId )
			{
				bridge_EventLog_MapInfo[alarmId].alarmLevel = level;
			}
			break;
		case alarmType_private:
			if( trap_private_min < alarmId && trap_private_max > alarmId )
			{
				private_EventLog_MapInfo[alarmId].alarmLevel = level;
			}
			break;
		case alarmType_bcmcmcctrl:
			if( trap_cmcctrl_min < alarmId && trap_cmcctrl_max > alarmId )
			{
				mib4_EventLog_MapInfo[alarmId].alarmLevel = level;
			}
			break;
		case alarmType_other:
			break;
		default:
			break;
	}

	return VOS_OK;
}
/*获取告警的同步标识；
	2:归并；
	1:同步；
	0:不同步.*/
ULONG getEventSynFlag( ULONG alarmType, ULONG alarmId )
{
	ULONG syn = ALM_SYN_NO;
	switch( alarmType )
	{
		case alarmType_mib2:
			if( trap_mib2_min < alarmId && trap_mib2_max > alarmId )
			{
				syn = mib2_EventLog_MapInfo[alarmId].synFlag;
			}
			break;
		case alarmType_bridge:
			if( trap_bridge_min < alarmId && trap_bridge_max > alarmId )
			{
				syn = bridge_EventLog_MapInfo[alarmId].synFlag;
			}
			break;
		case alarmType_private:
			if( trap_private_min < alarmId && trap_private_max > alarmId )
			{
				syn = private_EventLog_MapInfo[alarmId].synFlag;
			}
			break;
		case alarmType_bcmcmcctrl:
			if( trap_cmcctrl_min < alarmId && trap_cmcctrl_max > alarmId )
			{
				syn = mib4_EventLog_MapInfo[alarmId].synFlag;
			}
			break;
		case alarmType_other:
			break;
		default:
			break;
	}

	return syn;
}
/*获取关联告警ID*/
/*输入:告警类型，告警ID*/
/*返回:当前告警的所在的关联id，和前面定义的一致*/
/*不是所有的告警都有关联ID；loopalarm和loopalarm clear就是一对*/
ULONG getPartnerTrapId( ULONG alarmType, ULONG trapId )
{
	ULONG partner = 0;
	switch( alarmType )
	{
		case alarmType_mib2:
			if( trap_mib2_min < trapId && trap_mib2_max > trapId )
			{
				partner = mib2_EventLog_MapInfo[trapId].partnerId;
			}
			break;
		case alarmType_bridge:
			if( trap_bridge_min < trapId && trap_bridge_max > trapId )
			{
				partner = bridge_EventLog_MapInfo[trapId].partnerId;
			}
			break;
		case alarmType_private:
			if( trap_private_min < trapId && trap_private_max > trapId )
			{
				partner = private_EventLog_MapInfo[trapId].partnerId;
			}
			break;
		case alarmType_bcmcmcctrl:
			if( trap_cmcctrl_min < trapId && trap_cmcctrl_max > trapId )
			{
				partner = mib4_EventLog_MapInfo[trapId].partnerId;
			}
			break;
		case alarmType_other:
			break;
		default:
			break;
	}

	return partner;
}
/*告警源数据类型
ALM_SRC_T_DEV      设备级别
ALM_SRC_T_PORT    端口级别
ALM_SRC_T_NULL    其他
等等，没有列全*/
ULONG getEventAlarmSrcType( ULONG alarmType, ULONG alarmId )
{
	ULONG type = ALM_SRC_T_NULL;
	switch( alarmType )
	{
		case alarmType_mib2:
			if( (trap_mib2_min < alarmId) && (trap_mib2_max > alarmId) )
			{
				type = mib2_EventLog_MapInfo[alarmId].almSrcType;
			}
			break;
		case alarmType_bridge:
			if( (trap_bridge_min < alarmId) && (trap_bridge_max > alarmId) )
			{
				type = bridge_EventLog_MapInfo[alarmId].almSrcType;
			}
			break;
		case alarmType_private:
			if( (trap_private_min < alarmId) && (trap_private_max > alarmId) )
			{
				type = private_EventLog_MapInfo[alarmId].almSrcType;
			}
			break;
		case alarmType_bcmcmcctrl:
			if( trap_cmcctrl_min < alarmId && trap_cmcctrl_max > alarmId )
			{
				type = mib4_EventLog_MapInfo[alarmId].almSrcType;
			}
			break;
		case alarmType_other:
			if( (other_eventlog_min < alarmId) && (other_eventlog_max > alarmId) )
			{
				type = other_EventLog_MapInfo[alarmId].almSrcType;
			}
			break;
		default:
			break;
	}

	return type;
}
/*得到是否要将告警写入nvram*/
/*只有这一个类型的告警不写入*/
ULONG getEvent2NvramFlag( ULONG alarmType, ULONG alarmId )
{
	ULONG flag = ALM_NVRAM_YES;
	if( alarmType == alarmType_private )
	{
		if( trap_userLocUpdateNotify == alarmId )
		{
			flag = ALM_NVRAM_NO;
		}
	}
	return flag;
}


/*----------------------------------------------------------------------------*/
/* 与CLI接口函数 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
* 功能: 根据设备索引、告警级别、告警标识、告警起止时间等条件查询告警日志信息
* 输入参数: vty－VTY
			devIdx－MIB定义的设备索引，如果＝0则表示所有设备，包括OLT和ONU
			level－告警级别，参见枚举定义alarmLevel_t，如果＝0则表示所有告警级别
			alarmId－告警标识，目前只针对私有MIB定义的所有trap告警，如果＝0则表示所有告警
			pStartTime－起始时间，针对某个时间段的查询，如果＝0则表示所有告警
			pEndTime－结束时间
* 输出参数: 
* 返回值: 正确返回OK，错误返回ERROR */
LONG showEventLogCmd( struct vty *vty, ULONG devIdx, ULONG level, ULONG alarmId,
	sysDateAndTime_t *pStartTime, sysDateAndTime_t *pEndTime )
{
	/*LONG rc = VOS_ERROR;*/
	ULONG curPtr, lastPtr/*, idx*/;
	ULONG ulStart, ulEnd, ulCur;
	ULONG logCounter = 0;
	alarmSrc_t *pAlarmSrc;
	CHAR pLogStr[MAXLEN_EVENT_DESC+1];
	nvramEventLogData_t	logEntry;
	
	vty_out( vty, "%s", "\r\n" );

	if( pStartTime == NULL )
		ulStart = 0;
	else
		ulStart = sysLogDateAndTimeToUlong( pStartTime );
	if( pEndTime == NULL )
		ulEnd = 0;
	else
		ulEnd = sysLogDateAndTimeToUlong( pEndTime );

	VOS_SemTake( eventSemId, WAIT_FOREVER );

	/* modified by xieshl 20121010, 检查日志表头是否合法，现场曾发生过运行过程中表头索引错误的情况，
	    导致任务挂起*/
	if( (NULL == eventLogHead) || (0 == maxNumEventLogRecords) )
	{
		VOS_SemGive( eventSemId );
		vty_out( vty, "Can't find nvram\r\n" );
		return CMD_WARNING;
	}

	curPtr = eventLogHead->eventLogHeadSuffix;
	lastPtr = eventLogHead->eventLogCurSuffix;

	if( (eventLogHead->eventLogFlag != EVENTLOG_FLAG) ||
	 	(eventLogHead->eventLogHeadSuffix > maxNumEventLogRecords) ||
		(eventLogHead->eventLogCurSuffix > maxNumEventLogRecords) )
	{
		VOS_SemGive( eventSemId );
		vty_out( vty, "Find ERROR in nvram, Please erase alarm log and try again\r\n" );
		return CMD_WARNING;
	}

	while( eventLogHead->eventLogCurIndex )
	{
		VOS_MemCpy( &logEntry, &eventLogTbl[curPtr], sizeof(nvramEventLogData_t) );
		
		if( logEntry.alarmType == 0 )
			goto next_event_log;

		pAlarmSrc = (alarmSrc_t *)logEntry.alarmSrcData;

		if( devIdx != 0 )
		{
			if( !dev_idx_is_equal(pAlarmSrc->devAlarmSrc.devIdx, devIdx) )
				goto next_event_log;
		}
		
		if( level > ALM_LEV_NULL && level < ALM_LEV_MAX )
		{
			if( logEntry.alarmType == alarmType_private )
			{
				if( level != private_EventLog_MapInfo[ logEntry.alarmId ].alarmLevel )
					goto next_event_log;
			}
			else if( logEntry.alarmType == alarmType_other )
			{
				if( level != other_EventLog_MapInfo[ logEntry.alarmId ].alarmLevel )
					goto next_event_log;
			}
			else if( logEntry.alarmType == alarmType_mib2 )
			{
				if( level != mib2_EventLog_MapInfo[ logEntry.alarmId ].alarmLevel )
					goto next_event_log;
			}
			else if( logEntry.alarmType == alarmType_bridge )
			{
				if( level != bridge_EventLog_MapInfo[ logEntry.alarmId ].alarmLevel )
					goto next_event_log;
			}
			else if( logEntry.alarmType == alarmType_bcmcmcctrl )
			{
				if( level != mib4_EventLog_MapInfo[ logEntry.alarmId ].alarmLevel )
					goto next_event_log;
			}
		}
		if( alarmId != 0 )
		{
			if( (logEntry.alarmId != alarmId) || (logEntry.alarmType != alarmType_private) )
				goto next_event_log;
		}

		if( (ulStart != 0) || (ulEnd != 0) )
		{
			ulCur = nvramLogDateAndTimeToUlong( &logEntry.alarmTime );

			if( (ulStart != 0) && (ulCur < ulStart) )
				goto next_event_log;
			if( (ulEnd != 0) && (ulCur > ulEnd) )
				goto next_event_log;
		}

		/*if( eventLogSuffix2Idx(curPtr, &idx) == VOS_OK )*/
		{
			eventLogDataToStrings( &logEntry, pLogStr );
			if( pLogStr[0] )
			{
				/*vty_out( vty, "%u: %s\r\n", idx, pLogStr );*/		/* modified by xieshl 20110901, 去掉编号打印信息 */
				vty_out( vty, " %s\r\n", pLogStr );
				logCounter++;
			}
		}

next_event_log:		
		if( curPtr == lastPtr )
			break;
				
		if( curPtr >= maxNumEventLogRecords )
			curPtr = 0;
		else
			curPtr++;

	}
	VOS_SemGive( eventSemId );

	vty_out( vty, "%s", "----------------------------------------------\r\n" );
	vty_out( vty, "Total event info. count = %d\r\n\r\n", logCounter );

	return VOS_OK;
}

/*add by shixh20090304*/
LONG showEventLogImportCmd( struct vty *vty, ULONG devIdx, ULONG level, ULONG alarmId,
	sysDateAndTime_t *pStartTime, sysDateAndTime_t *pEndTime )
{
	/*LONG rc = VOS_ERROR;*/
	ULONG curPtr, lastPtr/*, idx*/;
	ULONG ulStart, ulEnd, ulCur;
	ULONG logCounter = 0;
	alarmSrc_t *pAlarmSrc;
	CHAR pLogStr[MAXLEN_EVENT_DESC+1];
	nvramEventLogData_t	logEntry;
	
	vty_out( vty, "%s", "\r\n" );

	if( pStartTime == NULL )
		ulStart = 0;
	else
		ulStart = sysLogDateAndTimeToUlong( pStartTime );
	if( pEndTime == NULL )
		ulEnd = 0;
	else
		ulEnd = sysLogDateAndTimeToUlong( pEndTime );

	VOS_SemTake( eventSemId, WAIT_FOREVER );

	if( (NULL == eventLogImportHead) || (0 == maxNumEventLogImportRecords) )
	{
		VOS_SemGive( eventSemId );
		vty_out( vty, "Can't find nvram\r\n" );
		return CMD_WARNING;
	}

	curPtr = eventLogImportHead->eventLogHeadSuffix;
	lastPtr = eventLogImportHead->eventLogCurSuffix;

	if( (eventLogImportHead->eventLogFlag != EVENTLOG_FLAG) ||
	 	(eventLogImportHead->eventLogHeadSuffix > maxNumEventLogImportRecords) ||
		(eventLogImportHead->eventLogCurSuffix > maxNumEventLogImportRecords) )
	{
		VOS_SemGive( eventSemId );
		vty_out( vty, "Find ERROR in nvram, Please erase alarm log and try again\r\n" );
		return CMD_WARNING;
	}

	while( eventLogImportHead->eventLogCurIndex )
	{
		VOS_MemCpy( &logEntry, &eventLogImportTbl[curPtr], sizeof(nvramEventLogData_t) );
		
		if( logEntry.alarmType == 0 )
			goto next_event_log;

		pAlarmSrc = (alarmSrc_t *)logEntry.alarmSrcData;

		if( devIdx != 0 )
		{
			if( !dev_idx_is_equal(pAlarmSrc->devAlarmSrc.devIdx, devIdx) )
				goto next_event_log;
		}
		
		if( level > ALM_LEV_NULL && level < ALM_LEV_MAX )
		{
			if( logEntry.alarmType == alarmType_private )
			{
				if( level != private_EventLog_MapInfo[ logEntry.alarmId ].alarmLevel )
					goto next_event_log;
			}
			else if( logEntry.alarmType == alarmType_other )
			{
				if( level != other_EventLog_MapInfo[ logEntry.alarmId ].alarmLevel )
					goto next_event_log;
			}
			else if( logEntry.alarmType == alarmType_mib2 )
			{
				if( level != mib2_EventLog_MapInfo[ logEntry.alarmId ].alarmLevel )
					goto next_event_log;
			}
			else if( logEntry.alarmType == alarmType_bridge )
			{
				if( level != bridge_EventLog_MapInfo[ logEntry.alarmId ].alarmLevel )
					goto next_event_log;
			}
			else if( logEntry.alarmType == alarmType_bcmcmcctrl )
			{
				if( level != mib4_EventLog_MapInfo[ logEntry.alarmId ].alarmLevel )
					goto next_event_log;
			}
		}
		if( alarmId != 0 )
		{
			if( (logEntry.alarmId != alarmId) || (logEntry.alarmType != alarmType_private) )
				goto next_event_log;
		}

		if( (ulStart != 0) || (ulEnd != 0) )
		{
			ulCur = nvramLogDateAndTimeToUlong( &logEntry.alarmTime );

			if( (ulStart != 0) && (ulCur < ulStart) )
				goto next_event_log;
			if( (ulEnd != 0) && (ulCur > ulEnd) )
				goto next_event_log;
		}

		/*if( eventLogImportSuffix2Idx(curPtr, &idx) == VOS_OK )*/	/* 问题单11278 */
		{
			eventLogDataToStrings( &logEntry, pLogStr );
			if( pLogStr[0] )
			{
				/*vty_out( vty, "%u: %s\r\n", idx, pLogStr );*/		/* modified by xieshl 20110901, 去掉编号打印信息 */
				vty_out( vty, " %s\r\n", pLogStr );
				logCounter++;
			}
		}

next_event_log:		
		if( curPtr == lastPtr )
			break;
				
		if( curPtr >= maxNumEventLogImportRecords )
			curPtr = 0;
		else
			curPtr++;

	}
	VOS_SemGive( eventSemId );

	vty_out( vty, "%s", "----------------------------------------------\r\n" );
	vty_out( vty, "Total event IMPORT info. count = %d\r\n\r\n", logCounter );

	return VOS_OK;
}

/*----------------------------------------------------------------------------*/
extern CHAR *alm_level_2_str( ULONG level );
static LONG showPrivateTrapIdLevelCmd( struct vty *vty, alarmLevel_t level )
{
	int i;
	eventInfoMapTbl_t *pMapInfo;
	char level_str[20], *pstr;
	
	for( i=trap_private_min+1; i<trap_private_max; i++ )
	{
		pMapInfo = &private_EventLog_MapInfo[i];

		if( pMapInfo->alarmId == trap_private_max )
		{
			VOS_ASSERT(0);
			break;
		}
		if( pMapInfo->alarmLevel == ALM_LEV_CLEAR )
			continue;
		if( level == pMapInfo->alarmLevel )
		{
			vty_out( vty, "  %-10d", pMapInfo->alarmId );
			if( pMapInfo->partnerId )
				vty_out( vty, "%-9d", pMapInfo->partnerId );
			else
				vty_out( vty, "%-9s", "-" );
			pstr = alm_level_2_str(pMapInfo->alarmLevel);
			if( pstr[0] == '-' )
				VOS_Sprintf( level_str, "%s", pstr );
			else	
				VOS_Sprintf( level_str, "%d(%s)", pMapInfo->alarmLevel, pstr );
			vty_out( vty, "%-10s%s\r\n", level_str, pMapInfo->pEventLogDesc );
		}
	}
	return CMD_SUCCESS;
}
LONG showPrivateTrapIdCmd( struct vty *vty, alarmLevel_t level )
{
	vty_out( vty, " %-10s%-10s%-10s%s\r\n", "trap-id", "clear-id", "level", "alarm-desc" );

	if( level == ALM_LEV_MAX )
	{
		showPrivateTrapIdLevelCmd( vty, ALM_LEV_VITAL );
		showPrivateTrapIdLevelCmd( vty, ALM_LEV_MAJOR );
		showPrivateTrapIdLevelCmd( vty, ALM_LEV_MINOR );
		showPrivateTrapIdLevelCmd( vty, ALM_LEV_WARN );
		showPrivateTrapIdLevelCmd( vty, ALM_LEV_NULL );
	}
	else
		showPrivateTrapIdLevelCmd( vty, level );
	return CMD_SUCCESS;
}

#if 0
ULONG event_log_count = 0;
ULONG event_log_flag = 1;
ULONG event_log_delay = 60;
void test_event_main();
VOID testevent()
{
	VOS_TaskCreate("ttttEvent", 70, test_event_main, NULL );
}

void test_event_main()
{
	ULONG delay;
	while( event_log_flag )
	{
		delay = event_log_delay;
		
		event_log_count++;
		onuNewRegSuccess_EventReport(61001);
		taskDelay(delay);
		onuNotPresent_EventReport(61001,1);
		onuReregSuccess_EventReport(61001);
		taskDelay(delay);
		onuNotPresent_EventReport(61001,2);
 		devPowerOff_EventReport(61002);
		taskDelay(delay);
		devPowerOn_EventReport(61002);
 		cfgDataSaveSuccess_EventReport(1);
		taskDelay(delay);
		cfgDataSaveFail_EventReport(1);
 		flashClearSuccess_EventReport(1);
		taskDelay(delay);
		flashClearFail_EventReport(1);
 		softwareUpdateSuccess_EventReport(1);
		taskDelay(delay);
		softwareUpdateFail_EventReport(1);
 		firmwareUpdateSuccess_EventReport(1);
		taskDelay(delay);
		firmwareUpdateFail_EventReport(1);
 		cfgDataBackupSuccess_EventReport(1);
		taskDelay(delay);
		cfgDataBackupFail_EventReport(1);
 		cfgDataRestoreSuccess_EventReport(1);
		taskDelay(delay);
		cfgDataRestoreFail_EventReport(1);
 		/*cpuUsageFactorHigh_EventReport(1);*/
		dbaUpdateSuccess_EventReport(1);
		taskDelay(delay);
		dbaUpdateFailure_EventReport(1);
 		onuSoftwareLoadSuccess_EventReport(61001);
		taskDelay(delay);
		onuSoftwareLoadFailure_EventReport(61001);
 		
		bootUpdateSuccess_EventReport(1);
		taskDelay(delay);
		bootUpdateFailure_EventReport(1);
 		batfileBackupSuccess_EventReport(1);
		taskDelay(delay);
		batfileBackupFailure_EventReport(1);
 		batfileRestoreSuccess_EventReport(1);
		taskDelay(delay);
		batfileRestoreFailure_EventReport(1);
		
		swBoardProtectedSwitch_EventReport(1, 4);
		taskDelay(delay);
		boardTemperatureHigh_EventReport(1, 3);
		taskDelay(delay);
		boardTemperatureHighClear_EventReport(1, 3);
		taskDelay(delay);
		ponBoardReset_EventReport(1, 6);
		
 		devBoardInterted_EventReport(1, 6);
		taskDelay(delay);
		devBoardPull_EventReport(1, 6);
 		
		powerOffAlarm_EventReport(1, 9);
		taskDelay(delay);
		powerOnAlarm_EventReport(1, 9);
 		
		devFanAlarm_EventReport(1, 2);
		taskDelay(delay);
		devFanAlarmClear_EventReport(1, 2);
 		
		ponPortBERAlarm_EventReport(61001, 1, 1, 100000 );
		taskDelay(delay);
		ponPortBERAlarmClear_EventReport(61001, 1, 1, 1000 );
 		ponPortFERAlarm_EventReport(61001, 1, 1, 100000 );
		taskDelay(delay);
		ponPortFERAlarmClear_EventReport(61001, 1, 1, 1000);
 		
		llidActBWExceeding_EventReport(1, 6, 1, 1);
		taskDelay(delay);
		llidActBWExceedingClear_EventReport(1, 6, 1, 1);
 		
		autoProtectSwitch_EventReport(1, 6, 2);
		taskDelay(delay);
		ponPortAbnormal_EventReport(1, 6, 2);
		taskDelay(delay);
		onuRegisterConflict_EventReport(61002);
		taskDelay(delay);
		firmwareLoadSuccess_EventReport(1, 6, 1);
		taskDelay(delay);
		firmwareLoadFailure_EventReport(1, 6, 2);
 		dbaLoadSuccess_EventReport(1, 6, 1);
		taskDelay(delay);
		dbaLoadFailure_EventReport(1, 6, 2);
 		
		ethFlrAlarm_EventReport(61001, 1, 1);
		taskDelay(delay);
		ethFlrAlarmClear_EventReport(61001, 1, 1);
		ethFerAlarm_EventReport(1, 1, 1);
		taskDelay(delay);
		ethFerAlarmClear_EventReport(1, 1, 1);
		ethTranmittalIntermitAlarm_EventReport(1, 1, 2);
		taskDelay(delay);
		ethTranmittalIntermitAlarmClear_EventReport(1, 1, 2);
		ethLinkdown_EventReport(1, 1, 1);
		ethLinkup_EventReport(1, 1, 1);
		taskDelay(delay);
		
		
		onuStp_EventReport(61001, 1);
 		onuStp_EventReport(61001, 2);
		taskDelay(delay);
		
		oltStp_EventReport( 1 );
		oltStp_EventReport( 2 );
		oltMib2_EventReport( 1 );
		taskDelay(delay);
		oltMib2_EventReport( 2 );
		oltMib2_EventReport( 5 );
		taskDelay(delay);
	}
}

/*----------------------------------------------------------------------------*/

#endif

/* added by xieshl 20080424, 告警日志过滤 */
ULONG getEventLogDataFilter()
{
	return eventLogDataFilter;
}
/*没地方调用，留着备用?*/
LONG setEventLogDataFilter( ULONG filter )
{
	eventLogDataFilter = (filter & ALM_FILTER_ALL);
	return VOS_OK;
}

LONG addEventLogDataFilterIterm( ULONG filter_iterm )
{
	eventLogDataFilter |= (filter_iterm & ALM_FILTER_ALL);
	return VOS_OK;
}
LONG delEventLogDataFilterIterm( ULONG filter_iterm )
{
	eventLogDataFilter &= ((~filter_iterm) & ALM_FILTER_ALL);
	return VOS_OK;
}

#if 0
void test_vx_time_get()
{
	SLOCAL_TIME stTime;
	if( devsm_get_slocal_time(&stTime) == VOS_OK )
	{
		sys_console_printf( "\r\nCurrent time: %04d-%02d-%02d,%02d:%02d:%02d.%03d\r\n",
       			stTime.usYear, stTime.usMonth, stTime.usDay,
				stTime.usHour, stTime.usMinute, stTime.usSecond, stTime.usMilliseconds );
	}
	else
		sys_console_printf("\r\n GET current time ERR\r\n");
}

extern LONG devsm_set_vxworks_time( SLOCAL_TIME stTime );
void test_vx_time_set( LONG hour, LONG minute, LONG second )
{
	SLOCAL_TIME stTime;
	stTime.usYear = 2009;
	stTime.usMonth = 12;
	stTime.usDay = 24;
	stTime.usDayOfWeek = 4;
	stTime.usHour = hour;
	stTime.usMinute = minute;
	stTime.usSecond = second;
	stTime.usMilliseconds = 0;
	if( devsm_set_vxworks_time(stTime) == VOS_OK )
	{
		sys_console_printf( "\r\n SET current time OK\r\n" );
	}
	else
		sys_console_printf("\r\n SET current time ERR\r\n");
}


ULONG test_nvram_count = 0;
ULONG test_nvram_flag = 1;
ULONG test_nvram_delay = 0;
extern STATUS dallas_rtc_get( unsigned short *year, CHAR *month, CHAR *date, CHAR *day,
                       CHAR *hour,  CHAR *minute, CHAR *second, CHAR *usecond );
static void test_nvram()
{
	LONG i, j;
	sysDateAndTime_t dateTime;
	uchar_t chs[4];
	uchar_t *pNvram = (uchar_t*)NVRAM_BASEADDR;
	LONG nvramLen = 128*1024;

	while( !SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
	{
		VOS_TaskDelay(200);
	}

	while(test_nvram_flag)
	{
		test_nvram_count++;
		dallas_rtc_get(&dateTime.year, &dateTime.month, &dateTime.day, chs, 
				&dateTime.hour, &dateTime.minute, &dateTime.second, &dateTime.reserver[0]);
		sys_console_printf( "\r\n%uK begin:%04d-%02d-%02d,%02d:%02d:%02d\r\n",
			test_nvram_count,
			dateTime.year, dateTime.month, dateTime.day, 
			dateTime.hour, dateTime.minute, dateTime.second );
		for( i=0; i<1024; i++ )
		{
			VOS_TaskLock();
			for( j=0; j<nvramLen; j++ )
			{
				chs[0] = pNvram[j];
				pNvram[j] = chs[0];
			}
			VOS_TaskUnlock();
		}
		dallas_rtc_get(&dateTime.year, &dateTime.month, &dateTime.day, chs, 
				&dateTime.hour, &dateTime.minute, &dateTime.second, &dateTime.reserver[0]);
		sys_console_printf( " end:%04d-%02d-%02d,%02d:%02d:%02d\r\n",
			dateTime.year, dateTime.month, dateTime.day, 
			dateTime.hour, dateTime.minute, dateTime.second );

		if( test_nvram_delay )
			VOS_TaskDelay(test_nvram_delay);
	}
}
static VOS_HANDLE tTest_id = NULL; 
VOID testNvram( LONG delay )
{
	test_nvram_delay = delay;
	if( tTest_id == NULL )
		tTest_id = VOS_TaskCreate("tTest", 254, test_nvram, NULL );
}

#endif

