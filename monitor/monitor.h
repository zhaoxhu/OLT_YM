/**************************************************************************************
* Monitor.h - .定义了olt和onu设备链路的性能告警,门限制设置接口函数声明,
*			以及相关宏定义,数据结构等
* 
* Copyright (c) 2006 GW Technologies Co., LTD.
* All rights reserved.
* 
* create: 
* 2006-06-222  wutw
* 
* 文件描述：
* 
**************************************************************************************/


#ifndef __MONITOR_H__
#define __MONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "PonGeneral.h"
#include "OnuGeneral.h"

/*==========FOR MON=========*/
#define	MONITOR_OK						(0)
#define	MONITOR_ERR						(-1)

#define 	MON_MAX_OLT						(-1)
/*========define for alarm===============*/

#define ALARM_DOWNLINK					0
#define ALARM_UPLINK						1
#define ALARM_DOWN_AND_UP				2


/*#define ALARM_BER_THREASHOLD			10000000
#define ALARM_BER_MIN_ERRBYTE			10000
#define ALARM_FER_THREASHOLD			10000000
#define ALARM_SNMP_THREASHOLD			10000000
*/
#define ALARM_EPON_THREASHOLD			1.00e-6/*1e-6 - 1e-e*/


#define ALARM_EPON_BER_THREASHOLD		1.00e-9/*1e-6 - 1e-e*/
#define ALARM_BER_BER_MASK				100000000

#define ALARM_EPON_FER_THREASHOLD		1.00e-11/*1e-6 - 1e-e*/
#define ALARM_EPON_FER_MASK				1000000000/*1e-6 - 1e-e*/
#define ALARM_BASE_ERRRATE				1000000

#define MON_MAX_PON						MAXPON
/*del by luh @2015-04-15 此处无用，等需要用的时候，知晓我*/
/*#define MON_MAX_ONU						MAXONUPERPON  */
/*#define bool 		BOOL*/
#define ALARM_ENABLE					1
#define ALARM_DISABLE					2

#define MON_BER_NUM	(1000/2)

#if 0
typedef enum  
{
	PON_STAT_ONU_BER,			/* Average ONU Bit error rate (upstream) as measured by the OLT        */
	PON_STAT_OLT_BER,			/* Average OLT Bit error rate (downstream) as measured by the ONU      */
	PON_STAT_ONU_FER,			/* Average ONU frames error rate (upstream) as measured by the OLT     */
	PON_STAT_OLT_FER,			/* Average OLT frames error rate (downstream) as measured by the ONU   */
	PON_STAT_LAST_STAT
} PON_statistics_t;

/* Uplink / downlink network directions */
typedef enum
{  
	PON_DIRECTION_UPLINK,
	PON_DIRECTION_DOWNLINK,
	PON_DIRECTION_UPLINK_AND_DOWNLINK
} PON_pon_network_traffic_direction_t;

typedef struct
{
	PON_pon_network_traffic_direction_t  direction; /* Traffic direction to monitor for PON_ALARM_BER */
	long double							 ber_threshold; /* Bit error rate measurement threshold above */
														/* which the alarm is raised				  */
														/* Values: 0 - 1							  */
	/*long double							 TBD;	*/		/* TBD parameter */
	unsigned long						 minimum_error_bytes_threshold; /* The minimal number of error */
																		/* bytes required for the alarm*/
} PON_ber_alarm_configuration_t;

/* PON_ALARM_FER
**
** High FER value alarm, based on PAS-SOFT internal monitoring mechanism. Alarm specificaitons: 
**
** alarm parameter for OLT originated (PON_OLT_ID source_id) FER alarm:
**	  llid number		   - FER meter (in the OLT) which triggered the alarm
**    PON_ALL_ACTIVE_LLIDS - Indicates global PON port HEC violations
**
** Aalrm source_id:		PON_PAS_SOFT_ID		  |		PON_OLT_ID			  |  PON_MIN_ONU_ID_PER_OLT - 
**											  |							  |  PON_MAX_ONU_ID_PER_OLT	
**					--------------------------|---------------------------|--------------------------
** alarm parameter:		Illegal source id	  |llid, PON_ALL_ACTIVE_LLIDS |		0		
** alarm data:			Illegal source id	  |		 PON_fer_t			  |		PON_fer_t
**
**
** Configuration: 
** 
** source: PON_OLT_ID - configuration applicable for all the ONUs in the specified OLT, and for uplink 
**						global PON port HEC violations 
** configuration structure: PON_fer_alarm_configuration_t;
*/
/*typedef PON_fer_t PON_fer_alarm_data_t;*/

typedef struct
{
	PON_pon_network_traffic_direction_t  direction; /* Traffic direction to monitor for PON_ALARM_FER */
	long double							 fer_threshold; /* Frame error rate measurement threshold     */
														/* above which the alarm is raised			  */
														/* Values: 0 - 1							  */
	/*long double							 TBD;	*/		/* TBD parameter */
	unsigned long						 minimum_error_frames_threshold;/*The minimal number of error */
																	   /*frames required for the alarm*/
} PON_fer_alarm_configuration_t;
#endif
/*==============for Mon===========*/

#if 0
typedef struct 
{
	short int  ponMonStatus;
	long double ponBERThreashold;
	long double ponBERClearThreashold;
	long double ponFERThreashold;
	long double ponFERClearThreashold;
	long double ponBERRisingThreashold;
	long double ponBERFallingThreashold;
	long double ponFERRisingThreashold;
	long double ponFERFallingThreashold;
} ponThreasholdInfo;
#endif
typedef struct 
{
	short int  ponMonStatus;
	unsigned int ponBERThreashold;
	unsigned int ponBerNum ;/*该值为单个onu时,olt收到的总的bit数*/
	unsigned int ponBERClearThreashold;
	unsigned int ponFERThreashold;
	unsigned int ponFerNum;
	unsigned int ponFERClearThreashold;
	unsigned int ponBERRisingThreashold;
	unsigned int ponBERFallingThreashold;
	unsigned int ponFERRisingThreashold;
	unsigned int ponFERFallingThreashold;
} ponThreasholdInfo;

typedef struct 
{
	unsigned long ponBERAlarm;
	unsigned long ponFERAlarm;
} ponAlarmInfo;

/* Alarm types */
#if 0
typedef enum
{  
	PON_ALARM_BER,
	PON_ALARM_FER,
	PON_ALARM_SOFTWARE_ERROR,
	PON_ALARM_LOCAL_LINK_FAULT,	
	PON_ALARM_DYING_GASP,	
	PON_ALARM_CRITICAL_EVENT,
	PON_ALARM_REMOTE_STABLE,
	PON_ALARM_LOCAL_STABLE,
	PON_ALARM_OAM_VENDOR_SPECIFIC,
	PON_ALARM_ERRORED_SYMBOL_PERIOD,
	PON_ALARM_ERRORED_FRAME,
	PON_ALARM_ERRORED_FRAME_PERIOD,
	PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY,
	PON_ALARM_ONU_REGISTRATION_ERROR,
	PON_ALARM_OAM_LINK_DISCONNECTION,
	PON_ALARM_BAD_ENCRYPTION_KEY,
	PON_ALARM_LLID_MISMATCH,
    PON_ALARM_TOO_MANY_ONU_REGISTERING,
	PON_ALARM_LAST_ALARM
} PON_alarm_t;
#endif

typedef struct	{
	long  double totalOctets;
	long double  badOctets;
	unsigned int octetThre;
	/*long double octetNum;*/
	unsigned long totalFrames;
	unsigned long badFrames;
	unsigned int frameThre;
	/*unsigned long frameNum;*/
	BOOL berStatus;
	BOOL ferStatus;
	} rtPreStatsEntry;


/*============以下函数为检测接口函数(mon)===================*/
/************************************************
* monInit
*	描述: 初始化函数,必须在系统启动时被调用
*	输入: 
*	输出: 
*
*************************************************/
short int monInit(void);

/************************************************
* monPonPerfUpFer
*	描述: *	onu pon上行的fer值以onuid或者以STATS_PON_ALL_ACTIVE_LLIDS 为对象;
*
*	pon上行fer	
*	参数: ponid
*
*
*************************************************/
short int monPonPerfUpFer(unsigned long ponId, unsigned int *pUpFer);

/************************************************
* monPonPerfDownFer
*	描述: *	onu pon下行的fer值以onuid或者以STATS_PON_ALL_ACTIVE_LLIDS 为对象;
*
*	pon下行fer	
*	参数: ponId
*
*
*************************************************/
short int monPonPerfDownFer(unsigned long ponId, unsigned int *pDownFer);

/************************************************
* monPonPerfUpBer
*	描述: *	onu pon上行的Ber值以onuid或者以STATS_PON_ALL_ACTIVE_LLIDS 为对象;
*
*	pon上行Ber	
*	参数: ponId 0-19
*		
*
*
*************************************************/
short int monPonPerfUpBer(unsigned long ponId, unsigned int *pUpBer);

/************************************************
* monPonPerfDownBer
*	描述: *	onu pon下行的Ber值olt pon为对象;
*
*	pon下行Ber	
*	参数: ponId 0-19
*		
*
*
*************************************************/
short int monPonPerfDownBer(unsigned long ponId, unsigned int *pDownBer);

/************************************************
* monPonPerfDownBer
*	描述: onu pon 下行ber ，由onu统计，读所有onu 进行统计.该函数只用于pon
*	输入: ponId
*	输出: pDownBer
*
*************************************************/
short int monOnuPerfDownBer(unsigned long ponId, unsigned short onuId, unsigned int *pDownBer);

/************************************************
* monPonPerfDownFer
*
*	描述: onu pon 下行fer ，由onu统计，读所有onu 进行统计.该函数只用于pon
*	输入: ponId 
*	输出: pDownsFer
*
*************************************************/
short int monOnuPerfDownFer(unsigned long ponId, unsigned long onuId, unsigned int *pDownsFer);

/************************************************
* monPonPerfUpBer 
*	onu pon上行的ber值以onuid或者以STATS_PON_ALL_ACTIVE_LLIDS 为对象;
*
*	输入: ponId
*
*	输出: upber
*
*************************************************/
short int monOnuPerfUpBer(unsigned long ponId, unsigned long onuId, unsigned int *pUpBer);

/************************************************
* monPonPerfUpFer
*	描述: *	onu pon上行的fer值以onuid或者以STATS_PON_ALL_ACTIVE_LLIDS 为对象;
*
*	pon上行fer	
*	参数: ponid
*
*
*************************************************/
short int monOnuPerfUpFer(unsigned long ponId, unsigned long onuId, unsigned int *pUpFer);

/**************************************************
* monPonBerAlmEnSet
* 	Describe: 设置pon芯片的ber告警使能/去使能
*			berAlmEn = 1, 为设置使能,berAlmEn = 2,为设置去使能
*
*
*
***************************************************/
short int monPonBerAlmEnSet(unsigned short oltId, unsigned int berAlmEn);

/**************************************************
* monPonBerAlmEnGet
* 	Describe: 获取pon芯片的ber告警状态
*			berAlmEn = 1, 为设置使能,berAlmEn = 2,为设置去使能
*
*
*
***************************************************/
short int monPonBerAlmEnGet(unsigned short oltId, unsigned int *pBerAlmEn);

/**************************************************
* monPonFerAlmEnSet
* 	描述: 设置设备的fer 检测功能.设置条件: 
*		ferAlmEn = 1 为设置使能;ferAlmEn = 2 为disable 
*	参数: 
*
*
*
*
***************************************************/
short int monPonFerAlmEnSet(unsigned short oltId, unsigned int ferAlmEn);

/**************************************************
* monPonFerAlmEnGet
* 	描述: 获取设备的fer 告警设置状态.设置条件: 
*		ferAlmEn = 1 为设置使能;ferAlmEn = 2 为disable 
*	参数: 
*
*
*
*
***************************************************/
short int monPonFerAlmEnGet(unsigned short oltId, unsigned int *pFerAlmEn);

/**************************************************
* monPonBerAlmEnSet
* 	Describe: 设置onu 的ber告警使能/去使能
*			berAlmEn = 1, 为设置使能,berAlmEn = 2,为设置去使能
*
*
*description : 该函数使能某个pon端口下的所有onu的上下行ber设置
***************************************************/
short int monOnuBerAlmEnSet(unsigned short oltId, /*unsigned short onuId,*/ unsigned int berAlmEn);

/**************************************************
* monPonBerAlmEnGet
* 	Describe: 获取onu 的ber告警使能状态
*			berAlmEn = 1, 为设置使能,berAlmEn = 2,为设置去使能
*
*
*
***************************************************/
short int monOnuBerAlmEnGet(unsigned short oltId, /*unsigned short onuId, */unsigned int *pBerAlmEn);

/**************************************************
* monOnuFerAlmEnSet
* 	描述: 设置设备的fer 检测功能.设置条件: 
*		ferAlmEn = 1 为设置使能;ferAlmEn = 2 为disable 
*	参数: 
*
*
*
*
***************************************************/
short int monOnuFerAlmEnSet(unsigned short oltId, /*unsigned short onuId, */unsigned int ferAlmEn);

/**************************************************
* monOnuFerAlmEnGet
* 	描述: 设置设备的fer 检测功能.设置条件: 
*		ferAlmEn = 1 为设置使能;ferAlmEn = 2 为disable 
*	参数: 
*
*
*
*
***************************************************/
short int monOnuFerAlmEnGet(unsigned short oltId, /*unsigned short onuId,*/ unsigned int *pFerAlmEn);

extern short int monponBERThrSet(int berFallThr, int berNum);
extern short int monponBERThrGet(unsigned int *pBerFallThr, unsigned int *pBerNum);


/*added by wutw at 14 september for cli*/
/*description: 该函数设置pon端口的fer 告警门限
可设置的fer的门限为10-10000
则消除告警门限一律要比告警门限低5*/
extern short int monponFERThrSet(int ferFallThr, int ferNum);
extern short int monponFERThrGet(unsigned int *pFerFallThr, unsigned int *pFerNum);

short int monPonAlarmCheck(short int PonId, 
						ulong_t frameErr, ulong_t frameTotal,
						ulong_t bitErr, ulong_t bitTotal);


#ifdef __cplusplus
}
#endif
#endif

