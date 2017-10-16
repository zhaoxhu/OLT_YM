#ifndef _ONUPORTSTATSMGT_H
#define _ONUPORTSTATSMGT_H

#include  "OnuGeneral.h"
#include "onuConfMgt.h"


#define MAX_PORT_PER_ONU 24

typedef struct
{
	UCHAR OnuRunningState ; /*ONU������״̬������������*/
	ULONG WakeUpTimeOut; /*ONU������Ļ���ʱ���� */
	ULONG OnuCrashTimes; /*Onu crash �Ĵ��� */
	OnuPortStats_ST *StatsPerOnuPort; /*ONU�豸�Ķ˿�ͳ�Ʊ�*/
}__attribute__((packed)) OnuDevStats_ST;

typedef struct
{
	ULONG TaskId;  /*����ID*/
	ULONG QueueId; /* ����ID*/
	ULONG SemId; /* �ź���ID*/
	ULONG GetDataTimeOut; 
	OnuDevStats_ST *OnuStatsPerPonPort; /*PON�˿�ͳ�Ʊ� */
}__attribute__((packed)) OnuPortStatsMgt_ST;

typedef enum
{
	ONU_RUNNING_STATE_NORMAL = 0, /*ONU��������*/
	ONU_RUNNING_STATE_CRASH /*ONU����*/
}ONU_RUNNING_STATE_E;

typedef enum
{ 
	MSG_ONU_PORT_STATS_TIMER = 0,
}ONU_PORTSTATS_MSGCODE_E;

/*ONU �˿�ͳ��ϵͳ��ʱ����ʱʱ����1s*/
#define ONU_PORT_STATS_TIMER_INTERVAL 1000

/*��ȡONUͳ�����ݵĶ�ʱ����ʱ��Ĭ��ʱ����1min*/
#define ONU_GETDATA_TIMER_INTERVAL 60

/*Wake-up ��ʱ����ʱ��Ĭ��ʱ����20min*/
#define ONU_WAKEUP_TIMER_INTERVAL  (20*60)

#define ONU_PORT_STATS_MAX_QUEUE_NUM   (ULONG)256

extern int CTCONU_Onustats_GetOnuPortDataByID(USHORT PonIdx,USHORT OnuIdx,USHORT port,OnuPortStats_ST * data);
extern LONG Onustats_EnableIs();
extern LONG Onustats_CommandInstall();
extern LONG Onustats_Init(void);
extern int Onustats_SetPortStatsTimeOut(short int olt_id,ONU_PORTSTATS_TIMER_NAME_E timer_name,ULONG timeout);
extern int Onustats_GetPortStatsTimeOut(ONU_PORTSTATS_TIMER_NAME_E timer_name,ULONG * timeout);


#endif
