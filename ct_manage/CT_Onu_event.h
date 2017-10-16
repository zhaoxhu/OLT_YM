
#include "PON_CTC_STACK_variable_descriptor_defines_expo.h"


#define ONU_Alarm_FLAG 1 
#define PON_Alarm_FLAG 2
#define ETHPORT_Alarm_FLAG 3

extern ULONG ctcEventQueId;
extern ULONG g_CtcOnuEventSemId;
#define ONU_EVENT_CONF_SEM_TAKE if(VOS_SemTake(g_CtcOnuEventSemId, WAIT_FOREVER) == VOS_OK)
#define ONU_EVENT_CONF_SEM_GIVE VOS_SemGive(g_CtcOnuEventSemId);
enum{
    Configflag =1,
    Reportflag =2,
};

extern int ReportDebugflag;
extern int ConfigDebugflag;
#define REPORT_DEBUG_PRINTF if(ReportDebugflag)sys_console_printf
#define CONFIG_DEBUG_PRINTF if(ConfigDebugflag)sys_console_printf

#define CTC_EVENT_QUEUE_LENGTH 128
#define MAX_ALARM_TEMP 128
#define MIN_ALARM_TEMP -128
#define MAX_ALARM_VCC 65.5
#define MIN_ALARM_VCC 0
#define MAX_ALARM_BIAS 131
#define MIN_ALARM_BIAS 0
#define MAX_ALARM_POWER 82
#define MIN_ALARM_POWER -400
#define CHECK_ALARM_TEMP(temp)  if(temp>MAX_ALARM_TEMP ||temp<MIN_ALARM_TEMP)\
    {\
        vty_out( vty, "  %% Parameter error\r\n");\
        return CMD_WARNING;\
    }
#define CHECK_ALARM_VCC(VCC)    if(VCC>MAX_ALARM_VCC ||VCC<MIN_ALARM_VCC)\
    {\
        vty_out( vty, "  %% Parameter error\r\n");\
        return CMD_WARNING;\
    }
#define CHECK_ALARM_BIAS(cur)   if(cur>MAX_ALARM_BIAS ||cur<MIN_ALARM_BIAS)\
    {\
        vty_out( vty, "  %% Parameter error\r\n");\
        return CMD_WARNING;\
    }
#define CHECK_ALARM_POWER(power)    if(power>MAX_ALARM_POWER ||power<MIN_ALARM_POWER)\
    {\
        vty_out( vty, "  %% Parameter error\r\n");\
        return CMD_WARNING;\
    }

typedef struct CTC_Alarm_Onu_info{
	UCHAR	EquipmentAlarm;
	UCHAR	PowerAlarm;
	UCHAR	BatteryAlarm;

	LONG   	BatteryVoltLowAlarmThr;
	LONG   	BatteryVoltLowClearThr;	
	
	UCHAR	PhysicalIntrusionAlarm;
	UCHAR	OnuSelfTestFailure;
		
	UCHAR	OnuTempAlarm;
	LONG   	OnuTempHighAlarmThr;
	LONG   	OnuTempHighClearThr;
	LONG   	OnuTempLowAlarmThr;
	LONG   	OnuTempLowClearThr;	
	
	UCHAR	IadConnectFailure;
	UCHAR	PonSwitch;
}__attribute__((packed))CTC_Alarm_Onu_info_t;

typedef struct CTC_Alarm_Port_info{
	UCHAR  EthAutoNegFailure;
	UCHAR  EthLos;	
	UCHAR  EthFailure;
	UCHAR  EthLoopback;	
	UCHAR  EthCongestion;
}__attribute__((packed))CTC_Alarm_Port_info_t;

#if 1
/*CTC 告警配置定义*/
extern eponOpticalPowerThresholds_t eponOpticalPowerThresholds;
extern eponOpticalPowerDeadZone_t eponOpticalPowerDeadZone;
extern BOOL  chk_config_detect_enable;			/* 环路检测使能 */
CTC_Alarm_Onu_info_t    CTCOnuAlarmConfig_onu;
CTC_Alarm_Port_info_t   CTCOnuAlarmConfig_port;   
extern LONG onu_OpticalPower_Enable;
#endif
void ctcEventProcTask();
int CTC_STACK_event_notification_handler(const PON_olt_id_t   olt_id, 
					                                const PON_onu_id_t  onu_id, 
					                                const CTC_STACK_event_value_t   event_value );
int CTCONU_ConfAlarmData(CTC_STACK_alarm_id_t alarm_id, int flag, int enable, ULONG alarm, ULONG clear);
int CTCONU_SetAlarmEnable(CTC_STACK_alarm_id_t code, int enable);
int CTCONU_SetAlarmThreshold(CTC_STACK_alarm_id_t code, int alarm, int clear);
#if 1
extern double pow ( double x, double y );
extern LONG getOpticalPowerThreshold( int field, int Flag);
extern int GetOnuOpticalPowerEnable();
extern LONG getOpticalPowerDeadZone( int field);
extern int getEthLoopEnable();
extern long TranslateTemperature( short int Temperature);
extern long TranslateBiasCurrent( unsigned short int BiasCurrent);
extern long TranslateWorkVoltage(unsigned short int WorkVoltage);
extern long TranslateOpticalPower( unsigned short int OpticalPower);
/*光功率告警*/
extern int onuOpticalParaAlm_EventReport(USHORT paraType, ULONG almFlag, ULONG devIdx, ULONG brdIdx, ULONG portIdx, long rtVal);

#endif

typedef struct CtcEventCfgData
{
    CTC_STACK_alarm_id_t alarm_id;
    int flag;/*0 :enable    1: threshold*/
}CtcEventCfgData_t;
#define CTC_ALARM_ENABLE 1
#define CTC_ALARM_DISABLE 0

enum
{   
    GW_RX_POWER_HIGH_ALARM		         = 0x0101,
    GW_RX_POWER_LOW_ALARM		         = 0x0102, 
    GW_TX_POWER_HIGH_ALARM		         = 0x0103,
    GW_TX_POWER_LOW_ALARM		         = 0x0104,
    GW_TX_BIAS_HIGH_ALARM		         = 0x0105, 
    GW_TX_BIAS_LOW_ALARM		         = 0x0106,
    GW_VCC_HIGH_ALARM 			         = 0x0107,
    GW_VCC_LOW_ALARM  			         = 0x0108, 
    GW_TEMP_HIGH_ALARM			         = 0x0109,
    GW_TEMP_LOW_ALARM		             = 0x010a,   
};

typedef struct
{
    USHORT onu_id;
    USHORT olt_id;
    CTC_management_object_t     management_object;
	CTC_STACK_alarm_id_t		alarm_id;
	unsigned short				time_stamp;
	CTC_STACK_alarm_state_t		alarm_state;
	CTC_STACK_alarm_info_t		alarm_info;
} CTC_STACK_eventMsg_t;
 /* CTC ONU 告警，光功率配置信息 */
typedef  struct{
	int  subType;
	CTC_STACK_alarm_id_t  Code;
	UCHAR  syncEnable;
    ULONG  threshold;
    ULONG  Clearthreshold;
} __attribute__((packed)) eventCtcSyncCfgMsg_t;
/*Pon板掉电恢复add by luh 2011-9-22*/
typedef  struct{
    CTC_Alarm_Onu_info_t onudata;
    CTC_Alarm_Port_info_t portdata;
    eponOpticalPowerThresholds_t ponthresholddata;
    eponOpticalPowerDeadZone_t pondeadzonedata;
} __attribute__((packed)) eventCtcSyncRecoverMsg_t;

#define CTC_ALARM_MAX_STATE 32
#define CTC_ALARM_STATE_EQUIPMENT                   1<<0
#define CTC_ALARM_STATE_POWERING_ALARM              1<<1
#define CTC_ALARM_STATE_BATTERY_MISSING             1<<2
#define CTC_ALARM_STATE_BATTERY_FAILURE             1<<3
#define CTC_ALARM_STATE_BATTERY_VOLT_LOW            1<<4
#define CTC_ALARM_STATE_PHYSICAL_INTRUSION_ALARM    1<<5
#define CTC_ALARM_STATE_ONU_SELF_TEST_FAILURE       1<<6
#define CTC_ALARM_STATE_ONU_TEMP_HIGH_ALARM         1<<7
#define CTC_ALARM_STATE_ONU_TEMP_LOW_ALARM          1<<8
#define CTC_ALARM_STATE_IAD_CONNECTION_FAILURE      1<<9
#define CTC_ALARM_STATE_PON_IF_SWITCH               1<<10
#define CTC_ALARM_STATE_RX_POWER_HIGH_ALARM         1<<11
#define CTC_ALARM_STATE_RX_POWER_LOW_ALARM          1<<12
#define CTC_ALARM_STATE_TX_POWER_HIGH_ALARM         1<<13
#define CTC_ALARM_STATE_TX_POWER_LOW_ALARM          1<<14
#define CTC_ALARM_STATE_TX_BIAS_HIGH_ALARM          1<<15
#define CTC_ALARM_STATE_TX_BIAS_LOW_ALARM           1<<16
#define CTC_ALARM_STATE_VCC_HIGH_ALARM              1<<17
#define CTC_ALARM_STATE_VCC_LOW_ALARM               1<<18
#define CTC_ALARM_STATE_TEMP_HIGH_ALARM             1<<19
#define CTC_ALARM_STATE_TEMP_LOW_ALARM              1<<20
#if 0
#define CTC_ALARM_STATE_RX_POWER_HIGH_WARNING       1<<21
#define CTC_ALARM_STATE_RX_POWER_LOW_WARNING        1<<22
#define CTC_ALARM_STATE_TX_POWER_HIGH_WARNING       1<<23
#define CTC_ALARM_STATE_TX_POWER_LOW_WARNING        1<<24
#define CTC_ALARM_STATE_TX_BIAS_HIGH_WARNING        1<<25
#define CTC_ALARM_STATE_TX_BIAS_LOW_WARNING         1<<26
#define CTC_ALARM_STATE_VCC_HIGH_WARNING            1<<27
#define CTC_ALARM_STATE_VCC_LOW_WARNING             1<<28
#define CTC_ALARM_STATE_TEMP_HIGH_WARNING           1<<29
#define CTC_ALARM_STATE_TEMP_LOW_WARNING            1<<30
#endif

#define CTC_ALARM_STATE_ETH_PORT_AUTO_NEG_FAILURE   1<<21
#define CTC_ALARM_STATE_ETH_PORT_LOS                1<<22
#define CTC_ALARM_STATE_ETH_PORT_FAILURE            1<<23
#define CTC_ALARM_STATE_ETH_PORT_LOOPBACK           1<<24
#define CTC_ALARM_STATE_ETH_PORT_CONGESTION         1<<25


