#ifndef _ONUPORTSTATSMGT_H
#define _ONUPORTSTATSMGT_H

#include  "OnuGeneral.h"
#include "onuConfMgt.h"


#define MAX_PORT_PER_ONU 24

typedef struct
{
	UCHAR OnuRunningState ; /*ONU的运行状态，正常、死机*/
	ULONG WakeUpTimeOut; /*ONU死机后的唤醒时间间隔 */
	ULONG OnuCrashTimes; /*Onu crash 的次数 */
	OnuPortStats_ST *StatsPerOnuPort; /*ONU设备的端口统计表*/
}__attribute__((packed)) OnuDevStats_ST;

typedef struct
{
	ULONG TaskId;  /*任务ID*/
	ULONG QueueId; /* 队列ID*/
	ULONG SemId; /* 信号量ID*/
	ULONG GetDataTimeOut; 
	OnuDevStats_ST *OnuStatsPerPonPort; /*PON端口统计表 */
}__attribute__((packed)) OnuPortStatsMgt_ST;

typedef enum
{
	ONU_RUNNING_STATE_NORMAL = 0, /*ONU运行正常*/
	ONU_RUNNING_STATE_CRASH /*ONU死机*/
}ONU_RUNNING_STATE_E;

typedef enum
{ 
	MSG_ONU_PORT_STATS_TIMER = 0,
}ONU_PORTSTATS_MSGCODE_E;

/*ONU 端口统计系统定时器超时时间是1s*/
#define ONU_PORT_STATS_TIMER_INTERVAL 1000

/*获取ONU统计数据的定时器超时的默认时间是1min*/
#define ONU_GETDATA_TIMER_INTERVAL 60

/*Wake-up 定时器超时的默认时间是20min*/
#define ONU_WAKEUP_TIMER_INTERVAL  (20*60)

#define ONU_PORT_STATS_MAX_QUEUE_NUM   (ULONG)256

extern int CTCONU_Onustats_GetOnuPortDataByID(USHORT PonIdx,USHORT OnuIdx,USHORT port,OnuPortStats_ST * data);
extern LONG Onustats_EnableIs();
extern LONG Onustats_CommandInstall();
extern LONG Onustats_Init(void);
extern int Onustats_SetPortStatsTimeOut(short int olt_id,ONU_PORTSTATS_TIMER_NAME_E timer_name,ULONG timeout);
extern int Onustats_GetPortStatsTimeOut(ONU_PORTSTATS_TIMER_NAME_E timer_name,ULONG * timeout);


#endif
