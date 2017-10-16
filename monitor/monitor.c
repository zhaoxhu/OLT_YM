
/***************************************************************
*
*						Module Name:  monitor.c 
*
*                       (c) COPYRIGHT  by 
*                        GWTT Com. Ltd.
*                        All rights reserved.
*
*     This software is confidential and proprietary to gwtt Com, Ltd. 
*     No part of this software may be reproduced,
*     stored, transmitted, disclosed or used in any form or by any means
*     other than as expressly provided by the written Software function 
*     Agreement between gwtt and its licensee
*
*   Date: 			2006/06/20
*   Author:			wu tongwu
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |-----------|------------------ 
**  06/06/20  |   wutw          |     create 
**----------|-----------|------------------
**
**
** DESCRIPTION : 处理与llid 链路性能告警和检测相关配置
** 2006/11/20 : 之前与王小宇解世力讨论，对目前的告警检测模块进行修改。增加
** 				olt 侧pon端口的fer与ber计算
** 2006/11/23 : pclint代码检查
** 2006/11/23 : 进行pclint检查，修改error与warning。主要问题在判断最大pon端口与
**				最大onu数目时，可能会造成越界。
***************************************************************/

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "V2R1_product.h"
/*#include "VOS_Base.h"
#include "VOS_types.h "
#include "Vos_typevx.h"*/
#include "monitor.h"
#include "statistics/statistics.h"


#define MON_LLID_MAX 127
#define MON_INVALID_LLID	(-1)
/*#include "statistics.h"*/
/*以下为mon模块全局变量定义*/
ponThreasholdInfo gPonThreashold;
ponThreasholdInfo gPonDefaultThreashold;

#if 0
ponAlarmInfo		*gpPonAlarm[MON_MAX_PON];
ponAlarmInfo		*gpOnuAlarm[MON_MAX_PON]/*[MON_MAX_ONU]*/;
#else
ponAlarmInfo		**gpPonAlarm;
ponAlarmInfo		**gpOnuAlarm;
#endif

/* B--added by liwei056@2010-1-20 for D9624 */
long double g_afLevelConst[] = {1.00e-00,
                              1.00e-01,   
                              1.00e-02,   
                              1.00e-03,   
                              1.00e-04,   
                              1.00e-05,   
                              1.00e-06,   
                              1.00e-07,   
                              1.00e-08,   
                              1.00e-09,   
                              1.00e-10,   
                              1.00e-11,   
                              1.00e-12,   
                              1.00e-13,   
                              1.00e-14,   
                              1.00e-15   
                                    };
/* E--added by liwei056@2010-1-20 for D9624 */

/*=======外部函数声明======*/

#if 0
extern rtStatsEntry	*pgStatsRealTimeOlt[STATS_HIS_MAXPON];
extern rtStatsEntry	*pgStatsRealTimeOnu[STATS_HIS_MAXPON][STATS_HIS_MAXONU];

/*BEGIN: added by wangxy 2007-07-20*/
extern unsigned char gStatsPonOn[STATS_HIS_MAXPON];
extern unsigned char gStatsOnuOn[STATS_HIS_MAXPON][STATS_HIS_MAXONU];
#else
/*extern rtStatsEntry	**pgStatsRealTimeOlt;*/
/*extern rtStatsEntry* (*pgStatsRealTimeOnu)[STATS_HIS_MAXONU];*/

/*extern unsigned char *gStatsPonOn;*/
extern unsigned char (*gStatsOnuOn)[MAXONUPERPONNOLIMIT];/*STATS_HIS_MAXONU*/
#endif
/*END*/

extern int Monitor_enable(bool status);
extern short int PAS_set_alarm_configuration ( const short int	    olt_id, 
									    const short int	    source,
										const PON_alarm_t   type,
										const bool			activate,
										const void		   *configuration );
extern short int PAS_get_statistics ( const short int			olt_id, 
							   const short int			collector_id, 
							   const PON_statistics_t   statistics_type, 
							   const short int		    statistics_parameter, 
							   long double			   *statistics_data );
extern short int GetLlidByOnuIdx( short int PonPortIdx, short int OnuIdx );
extern int ponPortBERAlarmClear_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx, ulong_t ber );
extern int ponPortFERAlarm_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx, ulong_t fer );
extern long sys_console_printf(const char *format,...); 
extern bool Olt_exists ( const short int olt_id );
extern int ponPortFERAlarmClear_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx, ulong_t fer );

#if 0
short int monponBERNumSet(unsigned int berNum);
short int monponBERNumGet(unsigned int *pBerNum);
short int monponBERClearThrSet(unsigned int berClearFallThr);
short int monponBERClearThrGet(unsigned int *pFerFallThr);
short int monponFERNumSet(unsigned int ferMun);
short int monponFERNumGet(unsigned int *pFerMun);
static ulong_t monBerCalcRatio( ulong_t divedNum, ulong_t divNum );
static ulong_t monFerCalcRatio( ulong_t divedNum, ulong_t divNum );
short int monponFERClearThrSet(unsigned int ferClearThr);
short int monponFERClearThrGet(unsigned int *pFerThr);
#endif


/*获取检测性能运行状态*/
/*short int gpMonPonInfo[STATS_HIS_MAXPON] = {0};*/

#if 0
rtPreStatsEntry pPreStatsEntry[20];
#else
rtPreStatsEntry *pPreStatsEntry;
#endif














/************************************************
* monInit
*	描述: 初始化函数,必须在系统启动时被调用
*	输入: 
*	输出: 
*
*************************************************/
short int monInit(void)
{
    ULONG ulSize;
    CHAR *pByteBuf;
    unsigned short ponId = 0;
    

	memset(&gPonThreashold, 0, sizeof(ponThreasholdInfo));
	/*snmp的精度只有10e－7*/
	
	gPonDefaultThreashold.ponBERThreashold = DEFAULT_PON_BER_THRESHOLD_RATIO;
	gPonDefaultThreashold.ponBERClearThreashold = DEFAULT_PON_BER_THRESHOLD_RATIO - 1;
	gPonDefaultThreashold.ponFERThreashold = DEFAULT_PON_FER_THRESHOLD_RATIO;	
	gPonDefaultThreashold.ponFERClearThreashold = DEFAULT_PON_FER_THRESHOLD_RATIO - 1;
	/* modified by chenfj 2007-7-31 */
	gPonDefaultThreashold.ponBerNum = DEFAULT_PON_BER_THRESHOLD_MINBYTE;
	/*gPonDefaultThreashold.ponBerNum = 34310616;*/
	gPonDefaultThreashold.ponFerNum = DEFAULT_PON_FER_THRESHOLD_MINFRAME;
	gPonThreashold.ponBERThreashold = gPonDefaultThreashold.ponBERThreashold;
	gPonThreashold.ponBERClearThreashold = gPonDefaultThreashold.ponBERClearThreashold;
	gPonThreashold.ponFERThreashold = gPonDefaultThreashold.ponFERThreashold;	
	gPonThreashold.ponFERClearThreashold = gPonDefaultThreashold.ponFERClearThreashold;
	gPonThreashold.ponBerNum = gPonDefaultThreashold.ponBerNum;
	gPonThreashold.ponFerNum = gPonDefaultThreashold.ponFerNum;

	/* 默认状态下检测为打开状态*/
	if (gPonThreashold.ponMonStatus == 1)
	{
		return MONITOR_ERR;
	}
	
	gPonThreashold.ponMonStatus = 1;

#if 0
	gPonDefaultThreashold.ponBERThreashold = 8;
	gPonDefaultThreashold.ponBERClearThreashold = 7;
	gPonDefaultThreashold.ponFERThreashold = 10;	
	gPonDefaultThreashold.ponFERClearThreashold = 9;
#endif

#if 1
    ulSize = sizeof(ponAlarmInfo*) * MON_MAX_PON;
    if ( NULL == (pByteBuf = (CHAR*)VOS_Malloc(ulSize * 2, (ULONG)MODULE_MON)) )
    {
        VOS_ASSERT(0);
		return MONITOR_ERR;
    }
    gpPonAlarm = (ponAlarmInfo**)pByteBuf;
    gpOnuAlarm = (ponAlarmInfo**)(pByteBuf + ulSize);

    ulSize = sizeof(rtPreStatsEntry) * MON_MAX_PON;
    if ( NULL == (pByteBuf = (CHAR*)VOS_Malloc(ulSize, (ULONG)MODULE_MON)) )
    {
        VOS_ASSERT(0);
		return MONITOR_ERR;
    }
    pPreStatsEntry = (rtPreStatsEntry*)pByteBuf;
#endif

	for(ponId = 0; ponId < MON_MAX_PON; ponId++)
	{
		gpPonAlarm[ponId] = (ponAlarmInfo*)VOS_Malloc((ULONG)sizeof(ponAlarmInfo), (ULONG)MODULE_MON);
		if( gpPonAlarm[ponId] == NULL )
        {
            VOS_ASSERT(0);
			return MONITOR_ERR;
        }      

		VOS_MemSet(gpPonAlarm[ponId] , 0, sizeof(ponAlarmInfo));
		gpPonAlarm[ponId]->ponBERAlarm = ALARM_ENABLE;
		gpPonAlarm[ponId]->ponFERAlarm = ALARM_ENABLE;

		gpOnuAlarm[ponId] = (ponAlarmInfo*)VOS_Malloc((ULONG)sizeof(ponAlarmInfo), (ULONG)MODULE_MON);
		if( gpOnuAlarm[ponId] == NULL )
        {
            VOS_ASSERT(0);
			return MONITOR_ERR;
        }      
		VOS_MemSet(gpOnuAlarm[ponId] , 0, sizeof(ponAlarmInfo));
		gpOnuAlarm[ponId]->ponBERAlarm = ALARM_ENABLE;
		gpOnuAlarm[ponId]->ponFERAlarm = ALARM_ENABLE;

		VOS_MemSet(&(pPreStatsEntry[ponId]), 0, sizeof(rtPreStatsEntry));
		#if 0
		memset(&(pPreStatsEntry[oltId]), 0, sizeof(rtPreStatsEntry));
		pPreStatsEntry[oltId].frameNum = 1000;
		pPreStatsEntry[oltId].octetNum= 100000;
		#endif
	}
	
	/*设置默认值,注意:无需判断返回值*/
	/*
	monponBERThrSet((unsigned int)1);
	monponFERThrSet((unsigned int)1);
	monponBERFallingThrSet((unsigned int)1);
	monPonBERRisingThrSet((unsigned int)1);
	monponFERFallingThrSet((unsigned int)1);
	monponFERRisingThrSet((unsigned int)1);
	*/
	return MONITOR_OK;
}

/************************************************
* monPonPerfUpFer
*	描述: *	onu pon上行的fer值以onuid或者以STATS_PON_ALL_ACTIVE_LLIDS 为对象;
*
*	pon上行fer	
*	参数: ponid
*
*
*************************************************/
short int monPonPerfUpFer(unsigned long ponId, unsigned int *pUpFer)
{
 	if ( pUpFer == NULL )
		return MONITOR_ERR;
	*pUpFer = pPreStatsEntry[ponId].frameThre;
	return MONITOR_OK;
}

/************************************************
* monPonPerfDownFer
*	描述: *	onu pon下行的fer值以onuid或者以STATS_PON_ALL_ACTIVE_LLIDS 为对象;
*
*	pon下行fer	
*	参数: ponId
*
*
*************************************************/
short int monPonPerfDownFer(unsigned long ponId, unsigned int *pDownFer)
{
 	if ( pDownFer == NULL )
		return MONITOR_ERR;
	*pDownFer = 0;
	return MONITOR_OK;
}

/*BEGIN: added by wangxy 2007-07-20
添加了对PON端口运行状态的检查
*/
short int monPonPerfBer( unsigned long ponId, unsigned int *pBer )
{
	if( pBer == NULL )
		return MONITOR_ERR;
	/*if( gStatsPonOn[ponId] )
		*pBer = (ULONG)(pgStatsRealTimeOlt[ponId]->rtBerRecord*1000000);
	else*/
		*pBer = 0;
	
	return MONITOR_OK;
}

short int monPonPerfFer( unsigned long ponId, unsigned int *pFer )
{
	if( pFer == NULL )
		return MONITOR_ERR;
	/*if( gStatsPonOn[ponId] )
		*pFer = (ULONG)(pgStatsRealTimeOlt[ponId]->rtFerRecord*1000000);
	else*/
		*pFer = 0;
	
	return MONITOR_OK;
}

short int monOnuPerfBer( unsigned long ponId, unsigned long onuId, unsigned int *pBer )
{
	if( pBer == NULL )
		return MONITOR_ERR;
	/*if( gStatsOnuOn[ponId][onuId] )
		*pBer = (ULONG)(pgStatsRealTimeOnu[ponId][onuId]->rtBerRecord*1000000);
	else*/
		*pBer = 0;
	
	return MONITOR_OK;
}

short int monOnuPerfFer( unsigned long ponId, unsigned long onuId, unsigned int *pFer )
{
	if( pFer == NULL )
		return MONITOR_ERR;
	/*if( gStatsOnuOn[ponId][onuId] )
		*pFer = (ULONG)(pgStatsRealTimeOnu[ponId][onuId]->rtFerRecord*1000000);
	else*/
		*pFer = 0;
	
	return MONITOR_OK;
}


short int monPonPerfRtUpBandWidth( ulong_t ponId, ulong_t *bandwidth )
{
	if( bandwidth == NULL )
		return MONITOR_ERR;
	/*if( gStatsPonOn[ponId] )
		*bandwidth = pgStatsRealTimeOlt[ponId]->rtUpBandWidth;	
	else*/
		*bandwidth = 0;
	
	return MONITOR_OK;
}

short int monPonPerfRtDownBandWidth( ulong_t ponId, ulong_t *bandwidth )
{
	if( bandwidth == NULL )
		return MONITOR_ERR;
	/*if( gStatsPonOn[ponId] )
		*bandwidth = pgStatsRealTimeOlt[ponId]->rtDownBandWidth;	
	else*/
		*bandwidth = 0;
	
	return MONITOR_OK;
}

short int monOnuPerfRtUpBandWidth( ulong_t ponId, ulong_t onuId, ulong_t *bandwidth )
{
	if( bandwidth == NULL )
		return MONITOR_ERR;
	/*if( gStatsOnuOn[ponId][onuId] )
		*bandwidth = pgStatsRealTimeOnu[ponId][onuId]->rtUpBandWidth;
	else*/
		*bandwidth = 0;
	
	return MONITOR_OK;
}

short int monOnuPerfRtDownBandWidth( ulong_t ponId, ulong_t onuId, ulong_t *bandwidth )
{
	if( bandwidth == NULL )
		return MONITOR_ERR;
	/*if( gStatsOnuOn[ponId][onuId] )
		*bandwidth = pgStatsRealTimeOnu[ponId][onuId]->rtDownBandWidth;
	else*/
		*bandwidth = 0;
	
	return MONITOR_OK;
}

/*END*/

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
short int monPonPerfUpBer(unsigned long ponId, unsigned int *pUpBer)
{
	if ( pUpBer == NULL )
		return MONITOR_ERR;
	*pUpBer = pPreStatsEntry[ponId].octetThre;
	return MONITOR_OK;
}

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
short int monPonPerfDownBer(unsigned long ponId, unsigned int *pDownBer)
{
	if ( pDownBer == NULL )
		return MONITOR_ERR;
	*pDownBer = 0;
	return MONITOR_OK;
}


/************************************************
* monPonPerfDownBer
*	描述: onu pon 下行ber ，由onu统计，读所有onu 进行统计.该函数只用于pon
*	输入: ponId
*	输出: pDownBer
*
*************************************************/
short int monOnuPerfDownBer(unsigned long ponId, unsigned short onuId, unsigned int *pDownBer)
{
	short int ret;
	short int llid = 0;
	long double stats_data = 0;
	/*PON_timestamp_t	pon_timestamp;*/
	
	if (pDownBer == NULL)
		return MONITOR_ERR;	
	
	llid = GetLlidByOnuIdx( ponId, onuId);
	if ( (MON_INVALID_LLID == llid) || (llid > MON_LLID_MAX) )
		return MONITOR_ERR;
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = OLT_GetStatistics((unsigned short )ponId,llid, PON_STAT_OLT_BER, onuId,&stats_data );
	if (ret == MONITOR_OK)
		*pDownBer = (unsigned int)(stats_data * ALARM_BASE_ERRRATE);
	
#ifdef MONITOR_DEBUG
	sys_console_printf("\r\nPon %d BER  is %g \r\n", ponId, *pDownBer );
#endif

	return ret;
}

/************************************************
* monPonPerfDownFer
*
*	描述: onu pon 下行fer ，由onu统计，读所有onu 进行统计.该函数只用于pon
*	输入: ponId 
*	输出: pDownsFer
*
*************************************************/
short int monOnuPerfDownFer(unsigned long ponId, unsigned long onuId, unsigned int *pDownsFer)
{
	short int ret;
	short int llid = 0;
	long double stats_data = 0;
	
	if (pDownsFer == NULL)
		return MONITOR_ERR;
	llid = GetLlidByOnuIdx( ponId, onuId-1);
	if ( (MON_INVALID_LLID == llid) || (llid > MON_LLID_MAX) )
		return MONITOR_ERR;	
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = OLT_GetStatistics((unsigned short )ponId,llid, PON_STAT_OLT_FER, onuId, &stats_data);
	if (ret == MONITOR_OK)
		*pDownsFer = (unsigned int)(stats_data * ALARM_BASE_ERRRATE);
#ifdef MONITOR_DEBUG
	sys_console_printf("\r\nPon %d BER  is %g \r\n", ponId, *pDownsFer );
#endif

	return ret;
}


/************************************************
* monPonPerfUpBer 
*	onu pon上行的ber值以onuid或者以STATS_PON_ALL_ACTIVE_LLIDS 为对象;
*
*	输入: ponId
*
*	输出: upber
*
*************************************************/
short int monOnuPerfUpBer(unsigned long ponId, unsigned long onuId, unsigned int *pUpBer)
{
	short int ret;
	long double stats_data = 0;
	/*short int llid = 0;*/	
	if (pUpBer == NULL)
		return MONITOR_ERR;
	/*llid = GetLlidByOnuIdx( ponId, onuId);
	if ( MON_INVALID_LLID == llid)
		return MONITOR_ERR;
	if (llid > MON_LLID_MAX)
		return MONITOR_ERR;		*/
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = OLT_GetStatistics((unsigned short )ponId, MON_MAX_OLT, PON_STAT_ONU_BER, onuId, &stats_data);
	if (ret == MONITOR_OK)
		*pUpBer = (unsigned int)( stats_data * ALARM_BASE_ERRRATE);
	return ret;
}





/************************************************
* monPonPerfUpFer
*	描述: *	onu pon上行的fer值以onuid或者以STATS_PON_ALL_ACTIVE_LLIDS 为对象;
*
*	pon上行fer	
*	参数: ponid
*
*
*************************************************/
short int monOnuPerfUpFer(unsigned long ponId, unsigned long onuId, unsigned int *pUpFer)
{
	short int ret;
	long double stats_data = 0;
	if (pUpFer == NULL)
		return MONITOR_ERR;
	/*llid = GetLlidByOnuIdx( ponId, onuId);
	if ( MON_INVALID_LLID == llid)
		return MONITOR_ERR;
	if (llid > MON_LLID_MAX)
		return MONITOR_ERR;		*/	
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = OLT_GetStatistics((unsigned short )ponId, MON_MAX_OLT, PON_STAT_ONU_BER, /*STATS_PON_ALL_ACTIVE_LLIDS*/onuId, &stats_data);
	if (ret == MONITOR_OK)
		*pUpFer = (unsigned int )(stats_data * ALARM_BASE_ERRRATE);
#ifdef MONITOR_DEBUG
		sys_console_printf("\r\nPon %d Onu %d BER  is %g \r\n", ponId, STATS_ONU_ALL, *pUpFer );
#endif	
	return ret;
}

#if 0
#endif

/**************************************************
* monPonBerAlmEnSet
* 	Describe: 设置pon芯片的ber告警使能/去使能
*			berAlmEn = 1, 为设置使能,berAlmEn = 2,为设置去使能
*
*
*
***************************************************/
short int monPonBerAlmEnSet(unsigned short oltId, unsigned int berAlmEn)
{
	short int ret = MONITOR_OK;

	if ( (berAlmEn != 1) && (berAlmEn != 2) )
		return MONITOR_ERR;

#if 1
	SWAP_TO_ACTIVE_PON_PORT_INDEX(oltId)

    ret = OLT_SetPonBerAlarm(oltId, berAlmEn, -1, -1);
#else    
	gpPonAlarm[oltId]->ponBERAlarm = berAlmEn;
	#if 0
	bool activate;

	PON_ber_alarm_configuration_t berAlarmCfg;
	long double berRiseThr = 0;
	unsigned int Thr = 0;
	if ( (berAlmEn != 1) && (berAlmEn != 2) )
		return MONITOR_ERR;
	memset(&berAlarmCfg, 0, sizeof(PON_ber_alarm_configuration_t));
	/*monponBERRisingThrGet(&Thr);*/
	monponBERThrGet(&Thr);
	berRiseThr = (long double)Thr * (ALARM_EPON_BER_THREASHOLD);
	/*需要初始化的时候设置默认门限值*/
	berAlarmCfg.ber_threshold = berRiseThr;
	berAlarmCfg.direction = ALARM_DOWN_AND_UP;  
	berAlarmCfg.minimum_error_bytes_threshold = gPonThreashold.ponBerNum;

	/**/
	if ((berAlmEn == ALARM_ENABLE))
		activate = TRUE;
	else if (berAlmEn == ALARM_DISABLE)
		activate = FALSE;
	else
		return MONITOR_ERR;
	/*1. 获取在线olt ,并设置该olt的门限制*/
	ret = PAS_set_alarm_configuration (oltId, MON_MAX_OLT,PON_ALARM_BER, activate, &berAlarmCfg);
	if (ret != MONITOR_OK)
		return ret;
	
	gpPonAlarm[oltId]->ponBERAlarm = berAlmEn;
	#endif
#endif
    
	return ret;
}

/**************************************************
* monPonBerAlmEnGet
* 	Describe: 获取pon芯片的ber告警状态
*			berAlmEn = 1, 为设置使能,berAlmEn = 2,为设置去使能
*
*
*
***************************************************/
short int monPonBerAlmEnGet(unsigned short oltId, unsigned int *pBerAlmEn)
{
	short int ret = MONITOR_OK;
	if ( pBerAlmEn == NULL )
		return MONITOR_ERR;
	*pBerAlmEn = gpPonAlarm[oltId]->ponBERAlarm;

#if 0

	if (NULL == pBerAlmEn)
		return MONITOR_ERR;
	*pBerAlmEn = gpPonAlarm[oltId]->ponBERAlarm;
#endif
	return ret;
}



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
short int monPonFerAlmEnSet(unsigned short oltId, unsigned int ferAlmEn)
{
	short int ret = MONITOR_OK;
	if ( (ferAlmEn != 1) && (ferAlmEn != 2) )
		return MONITOR_ERR;
#if 1
	SWAP_TO_ACTIVE_PON_PORT_INDEX(oltId)

    ret = OLT_SetPonFerAlarm(oltId, ferAlmEn, -1, -1);
#else    
	gpPonAlarm[oltId]->ponFERAlarm = ferAlmEn;
#if 0
	bool activate;

	PON_fer_alarm_configuration_t ferAlarmCfg;
	long double ferRiseThr = 0;
	unsigned int Thr = 0;	
	if ( (ferAlmEn != 1) && (ferAlmEn != 2) )
		return MONITOR_ERR;
	memset(&ferAlarmCfg, 0, sizeof(PON_fer_alarm_configuration_t));
	/*monponFERRisingThrGet(&Thr);*/
	monponFERThrGet(&Thr);
	/*ferRiseThr = (long double)Thr * (ALARM_EPON_THREASHOLD);	*/
	ferRiseThr = ((long double)Thr) * (ALARM_EPON_FER_THREASHOLD);
	/*需要初始化的时候设置默认门限值*/
	ferAlarmCfg.fer_threshold = ferRiseThr;
	ferAlarmCfg.direction = ALARM_DOWN_AND_UP;
	ferAlarmCfg.minimum_error_frames_threshold = gPonThreashold.ponFerNum;  /*ALARM_BER_MIN_ERRBYTE;*/

	if (ferAlmEn == ALARM_ENABLE)
		activate = TRUE;
	else if (ferAlmEn == ALARM_DISABLE)
		activate = FALSE;
	else
		return MONITOR_ERR;
	/*1. 获取在线olt ,并设置该olt的门限制*/
	ret = PAS_set_alarm_configuration (oltId, MON_MAX_OLT,PON_ALARM_BER, activate, &ferAlarmCfg);
	if (ret != MONITOR_OK)
		return ret;
	gpPonAlarm[oltId]->ponFERAlarm = ferAlmEn;
#endif	
#endif	

	return ret;	
}

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
short int monPonFerAlmEnGet(unsigned short oltId, unsigned int *pFerAlmEn)
{
	short int ret = MONITOR_OK;
	if ( pFerAlmEn == NULL )
		return MONITOR_ERR;
	*pFerAlmEn = gpPonAlarm[oltId]->ponFERAlarm;
#if 0

	if (NULL == pFerAlmEn)
		return MONITOR_ERR;

	*pFerAlmEn = gpPonAlarm[oltId]->ponFERAlarm;
#endif
	return ret;	
}

/**************************************************
* monPonBerAlmEnSet
* 	Describe: 设置onu 的ber告警使能/去使能
*			berAlmEn = 1, 为设置使能,berAlmEn = 2,为设置去使能
*
*
*description : 该函数使能某个pon端口下的所有onu的上下行ber设置
***************************************************/
short int monOnuBerAlmEnSet(unsigned short oltId, /*unsigned short onuId,*/ unsigned int berAlmEn)
{
	int ret;

#if 1
    ret = OLT_SetBerAlarm(oltId, berAlmEn, -1, -1);
#else
	long double berRiseThr;
	int Thr = -1;	

	/* 需要初始化的时候设置默认门限值*/
	monponBERThrGet(&Thr);

	/* 设置该olt的门限制*/
    /* B--modified by liwei056@2010-1-20 for D9624 */
    VOS_ASSERT(Thr < (sizeof(g_afLevelConst) / sizeof(g_afLevelConst[0])));
    berRiseThr = g_afLevelConst[Thr];
    /* E--modified by liwei056@2010-1-20 for D9624 */

	if(Olt_exists(oltId) == TRUE )
		{
		ret = PAS_set_alarm_configuration (oltId, (-1), PON_ALARM_BER, activate, &berAlarmCfg);
		if (ret != PAS_EXIT_OK)
			return MONITOR_ERR;
		}
	gpOnuAlarm[oltId]->ponBERAlarm = berAlmEn;
#endif

	return ret;	
}
/**************************************************
* monPonBerAlmEnGet
* 	Describe: 获取onu 的ber告警使能状态
*			berAlmEn = 1, 为设置使能,berAlmEn = 2,为设置去使能
*
*
*
***************************************************/
short int monOnuBerAlmEnGet(unsigned short oltId, /*unsigned short onuId, */unsigned int *pBerAlmEn)
{
	short int ret = MONITOR_OK;
	if (NULL == pBerAlmEn)
		return MONITOR_ERR;
	/**pBerAlmEn = gpOnuAlarm[oltId][onuId]->ponBERAlarm;*/
 	*pBerAlmEn = gpOnuAlarm[oltId]->ponBERAlarm;
	return ret;	
}


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
short int monOnuFerAlmEnSet(unsigned short oltId, /*unsigned short onuId, */unsigned int ferAlmEn)
{
	int ret;
    
#if 1
    ret = OLT_SetFerAlarm(oltId, ferAlmEn, -1, -1);
#else
	long double ferRiseThr;	
	int Thr = -1;	

	/* 需要初始化的时候设置默认门限值*/
	monponFERThrGet(&Thr);

	/* 设置该olt的门限制*/
    /* B--modified by liwei056@2010-1-20 for D9624 */
    VOS_ASSERT(Thr < (sizeof(g_afLevelConst) / sizeof(g_afLevelConst[0])));
    ferRiseThr = g_afLevelConst[Thr];
    /* E--modified by liwei056@2010-1-20 for D9624 */
    
	if(Olt_exists(oltId) == TRUE )
		{
		ret = PAS_set_alarm_configuration (oltId, (-1),PON_ALARM_FER, activate, &ferAlarmCfg);
		if (ret != PAS_EXIT_OK)
			return MONITOR_ERR;
		}
	gpOnuAlarm[oltId]->ponFERAlarm = ferAlmEn;
#endif

	return ret;	
}



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
short int monOnuFerAlmEnGet(unsigned short oltId, /*unsigned short onuId,*/ unsigned int *pFerAlmEn)
{
	short int ret = MONITOR_OK;
	if (NULL == pFerAlmEn)
		return MONITOR_ERR;

	*pFerAlmEn = gpOnuAlarm[oltId]->ponFERAlarm;
	return ret;	
}



#if 0
#endif


/*added by wutw at 14 september for cli*/
/*description: 该函数用于设置全局告警门限
可设置的门限为10-100
消除告警门限一律比告警门限低5
1)当olt下挂一个onu时,每秒统计olt pon端口收到的bit数量级为:
	34310616,以这一秒时刻产生的错误bit数为100,则计算出来的
	误码率为2.1(计算基数:1e-8,即2.1 = ((1000*100000)/34310616)*1000
	设最低的误码门限为10,则,只有满足此刻的错误bit数为5000时,
	误码率才能达到10.
2)由上,我们设置误码率时,需要对错误bit进行限制,否则会产生错误
	告警,或者很难产生告警(当一个olt pon端口下下挂多个onu时,总
	的bit数会变得非常的大.
	当一个olt pon端口下挂达64个onu时,以一个onu的例子的误码率
	作为设置,几乎很难满足产生告警的条件,即使是其中一个onu已经
	产生很严重的误码率.所以,统计olt pon端口的误码率以及误帧率
	没有意义.
	从下的数据可以得到以上结论.
	
3)数据计算如下:
	a)当olt的pon仅挂一个onu时,每秒获取到的bit数为:34310616,
	b)当olt的pon挂64个onu时,每秒获取的bit总数为:34310616*64
	= 2195879424(0x82E27600);
	c)当olt下挂一个onu时,每秒统计olt pon端口收到的bit数量级为:
	34310616,以这一秒时刻产生的错误bit数为100,则计算出来的
	误码率为2.1(计算基数:1e-8,即2.1 = ((1000*100000)/3410601)*1000
	d),以c)为基础设最低的误码门限为10,则,只有满足此刻的错误bit
	数为500时,误码率才能达到10.
	e)以d)为基础,olt下挂64个onu,只有平均每个onu都产生5000个bit
	数量(500*64 = 32000),才满足告警条件;假设其中的一个onu产生了
	31000的错误bit数,仍然不能产生告警;但对该onu来说,((31000*10000)
	/34310616)*10000 = 90351的误码率;
*/
short int monponBERThrSet(int berFallThr, int berNum)
{
	unsigned short ponId;
	unsigned int berAlmEn;

#if 1
    if (berFallThr >= 0)
    {
        if (berFallThr == gPonThreashold.ponBERThreashold)
        {
            if (berNum >= 0)
            {
                if (berNum == gPonThreashold.ponBerNum)
                {
            		return MONITOR_OK;
                }
            }
            else
            {
        		return MONITOR_OK;
            }
        }
    }
    else
    {
        if (berNum >= 0)
        {
            if (berNum == gPonThreashold.ponBerNum)
            {
        		return MONITOR_OK;
            }
        }
        else
        {
    		return MONITOR_OK;
        }
    }

    if ( 0 != OLT_SetBerAlarmParams(OLT_ID_ALL, berFallThr, berNum) )
    {
		return MONITOR_ERR;
    }
#else    
	unsigned int Thr = 0;

    /* B--modified by liwei056@2010-1-20 for D9624 */
#if 0
	if ( (berFallThr > 1000) || (berFallThr < 10) )
		return MONITOR_ERR;
#else
	if ( berFallThr > MIN_PON_BER_THRESHOLD_RATIO )
		return MONITOR_ERR;
#endif
    /* E--modified by liwei056@2010-1-20 for D9624 */

	gPonThreashold.ponBERThreashold = Thr;
    /* B--modified by liwei056@2010-1-20 for D9624 */
#if 0
	Thr -= 5;
#else
    if ( 0 < Thr )
        Thr--;
#endif
    /* E--modified by liwei056@2010-1-20 for D9624 */

	monponBERClearThrSet(Thr);
	
	/* deleted by chenfj 2007-7-31 */
	/*
	berNum = Thr*MON_BER_NUM;
	monponBERNumSet(berNum);
	*/
#endif
	
    if ( SYS_LOCAL_MODULE_RUNNINGSTATE == MODULE_RUNNING )
    {
    	for (ponId = 0; ponId < MAXPON; ponId++)
    	{
    	    /*判断该pon是否可用*/
    		if (!OLTAdv_IsExist ( ponId ))
    			continue;
            
    		monPonBerAlmEnGet(ponId, &berAlmEn);
    		/*if (ALARM_DISABLE == berAlmEn)
    			continue;*/
    		monPonBerAlmEnSet(ponId, berAlmEn);
    		/*if (iRes != MONITOR_OK)
    			continue;*/

    		berAlmEn = 0;
    		monOnuBerAlmEnGet(ponId, &berAlmEn);
    		/*if (ALARM_DISABLE == berAlmEn)
    			continue;*/
    		monOnuBerAlmEnSet(ponId, berAlmEn);
    		/*if (iRes != MONITOR_OK)
    			continue;*/
    	}
    }
    
	return MONITOR_OK;
}


short int monponBERThrGet(unsigned int *pBerFallThr, unsigned int *pBerNum)
{
    if ( NULL != pBerFallThr )
    {
    	*pBerFallThr = gPonThreashold.ponBERThreashold;
    }
    
    if ( NULL != pBerNum )
    {
    	*pBerNum = gPonThreashold.ponBerNum;
    }
    
	return MONITOR_OK;
}

/*added by wutw at 14 september for cli*/
/*description: 该函数设置pon端口的fer 告警门限
可设置的fer的门限为10-10000
则消除告警门限一律要比告警门限低5*/
short int monponFERThrSet(int ferFallThr, int ferNum)
{
	unsigned short ponId;
	unsigned int ferAlmEn;

#if 1
    if (ferFallThr >= 0)
    {
        if (ferFallThr == gPonThreashold.ponFERThreashold)
        {
            if (ferNum >= 0)
            {
                if (ferNum == gPonThreashold.ponFerNum)
                {
            		return MONITOR_OK;
                }
            }
            else
            {
        		return MONITOR_OK;
            }
        }
    }
    else
    {
        if (ferNum >= 0)
        {
            if (ferNum == gPonThreashold.ponFerNum)
            {
        		return MONITOR_OK;
            }
        }
        else
        {
    		return MONITOR_OK;
        }
    }

    if ( 0 != OLT_SetFerAlarmParams(OLT_ID_ALL, ferFallThr, ferNum) )
    {
		return MONITOR_ERR;
    }
#else    
	unsigned int Thr = 0;
	
	Thr = ferFallThr;
    /* B--modified by liwei056@2010-1-20 for D9624 */
#if 0
	if ((Thr<10) || (Thr>1000))
		return MONITOR_ERR;
#else
    if ( MIN_PON_FER_THRESHOLD_RATIO < Thr )
		return MONITOR_ERR;
#endif
    /* E--modified by liwei056@2010-1-20 for D9624 */
	
	gPonThreashold.ponFERThreashold = Thr;
    /* B--modified by liwei056@2010-1-20 for D9624 */
#if 0
	Thr -= 5;
#else
    if ( 0 < Thr )
        Thr--;
#endif
    /* E--modified by liwei056@2010-1-20 for D9624 */

    monponFERClearThrSet(Thr);
#endif

    if ( SYS_LOCAL_MODULE_RUNNINGSTATE == MODULE_RUNNING )
    {
    	for (ponId = 0; ponId < MAXPON; ponId++)
    	{
    	    /*判断该pon是否可用*/
    		if (!OLTAdv_IsExist ( ponId ))
    			continue;
            
    		monPonFerAlmEnGet(ponId,&ferAlmEn);
    		/*if (ALARM_DISABLE == ferAlmEn)
    			continue;*/
    		monPonFerAlmEnSet(ponId, ferAlmEn);
    		/*if (iRes != MONITOR_OK)
    			continue;*/

    		ferAlmEn = 0;
    		monOnuFerAlmEnGet(ponId,&ferAlmEn);
    		/*if (ALARM_DISABLE == ferAlmEn)
    			continue;*/
    		monOnuFerAlmEnSet(ponId, ferAlmEn);
    		/*if (iRes != MONITOR_OK)
    			continue;*/
    	}
	}
    
	return MONITOR_OK;
}

short int monponFERThrGet(unsigned int *pFerFallThr, unsigned int *pFerNum)
{
    if ( NULL != pFerFallThr )
    {
    	*pFerFallThr = gPonThreashold.ponFERThreashold;
    }
    
    if ( NULL != pFerNum )
    {
    	*pFerNum = gPonThreashold.ponFerNum;
    }
    
	return MONITOR_OK;
}

#if 0
short int monponBERNumSet(unsigned int berNum)
{
	gPonThreashold.ponBerNum = berNum;

	return MONITOR_OK;
}


short int monponBERNumGet(unsigned int *pBerNum)
{
	if (pBerNum == NULL)
		return MONITOR_ERR;
	/**pBerFallThr = gPonThreashold.ponBERThreashold * ALARM_BER_THREASHOLD;*/
	*pBerNum = gPonThreashold.ponBerNum;
	return MONITOR_OK;

}

/*added by wutw at 14 september for cli*/
/*description: 用于设置消除告警门限*/
short int monponBERClearThrSet(unsigned int berClearFallThr)
{
	unsigned int Thr = 0;
	/*unsigned int berFallThr = 0;*/

    /* B--remed by liwei056@2010-1-20 for D9624 */
#if 0
	monponBERThrGet(&berFallThr);
	if( berClearFallThr>berFallThr)
		return MONITOR_ERR;
#endif
    /* E--remed by liwei056@2010-1-20 for D9624 */
	Thr = berClearFallThr;
	if (Thr == gPonThreashold.ponBERClearThreashold)
		return MONITOR_OK;
	
	gPonThreashold.ponBERClearThreashold = Thr;

	return MONITOR_OK;
	
}

short int monponBERClearThrGet(unsigned int *pFerFallThr)
{
	if (pFerFallThr == NULL)
		return MONITOR_ERR;
	/**pFerFallThr = gPonThreashold.ponFERThreashold * ALARM_BER_THREASHOLD;*/
	*pFerFallThr = gPonThreashold.ponBERClearThreashold ;
	return MONITOR_OK;
}


short int monponFERNumSet(unsigned int ferMun)
{
	/* modified by chenfj 2007-7-31 */
	gPonThreashold.ponFerNum = ferMun;
	/*gPonThreashold.ponBerNum = ferMun;*/
	return MONITOR_OK;
}

short int monponFERNumGet(unsigned int *pFerMun)
{
	if (pFerMun == NULL)
		return MONITOR_ERR;
	
	/* modified by chenfj 2007-7-31 */
	*pFerMun = gPonThreashold.ponFerNum;
	/**pFerMun = gPonThreashold.ponBerNum;*/
	return MONITOR_OK;
}
#endif


static ulong_t monBerCalcRatio( ulong_t divedNum, ulong_t divNum )
{	
	ulong_t ratio = 0;
	if( divedNum != 0 )
	{
		if( divedNum >= divNum )
			ratio = 100000000;
		/* 20060719 为了防止溢出，分成多段计算 */		
		else if( divedNum <= 42 )
			ratio = (divedNum * 100000000 / divNum);			
		else if( divedNum <= 429 )
			ratio = (divedNum * 10000000 / divNum) * 10;		
		else if( divedNum <= 4294 )
			ratio = (divedNum * 1000000 / divNum)* 100;
		else if( divedNum <= 42940 )
			ratio = (divedNum * 100000 / divNum) * 1000;
		else if( divedNum <= 429400 )
			ratio = (divedNum * 10000 / divNum) * 10000;
		else if( divedNum <= 42940000 )
			ratio = (divedNum * 1000 / divNum) * 100000;
		else if( divedNum <= 42940000 )
			ratio = (divedNum* 100 / divNum) * 1000000;
		else if( divedNum <= 429400000 )
			ratio = (divedNum * 10 / divNum) * 10000000;
		else
			ratio = 100000000;
		/* end 20060719 */
	}
	return ratio;
}

static ulong_t monFerCalcRatio( ulong_t divedNum, ulong_t divNum )
{	
	ulong_t ratio = 0;
	if( divedNum != 0 )
	{
		if( divedNum >= divNum )
			ratio = 1000000000;
		/* 20060719 为了防止溢出，分成多段计算 */	
		else if( divedNum <= 4 )
			ratio = (divedNum * 1000000000 / divNum);		
		else if( divedNum <= 42 )
			ratio = (divedNum * 100000000 / divNum)*10;			
		else if( divedNum <= 429 )
			ratio = (divedNum * 10000000 / divNum) * 100;		
		else if( divedNum <= 4294 )
			ratio = (divedNum * 1000000 / divNum)* 1000;
		else if( divedNum <= 42940 )
			ratio = (divedNum * 100000 / divNum) * 10000;
		else if( divedNum <= 429400 )
			ratio = (divedNum * 10000 / divNum) * 100000;
		else if( divedNum <= 42940000 )
			ratio = (divedNum * 1000 / divNum) * 1000000;
		else if( divedNum <= 42940000 )
			ratio = (divedNum* 100 / divNum) * 10000000;
		else if( divedNum <= 429400000 )
			ratio = (divedNum * 10 / divNum) * 100000000;
		else
			ratio = 1000000000;
		/* end 20060719 */
	}
	return ratio;
}


/*added by wutw at 14 september for cli*/
/*description: 该函数设置pon端口的fer 告警消除门限*/
short int monponFERClearThrSet(unsigned int ferClearThr)
{
	unsigned int Thr = 0;
	/*unsigned int ferThr = 0;*/
	
    /* B--remed by liwei056@2010-1-20 for D9624 */
#if 0
	monponFERThrGet(&ferThr);
	if (ferClearThr>ferThr)
		return MONITOR_ERR;
#endif
    /* E--remed by liwei056@2010-1-20 for D9624 */
	Thr = ferClearThr;
	gPonThreashold.ponFERClearThreashold = Thr;
	return MONITOR_OK;
}

short int monponFERClearThrGet(unsigned int *pFerThr)
{
	if (pFerThr == NULL)
		return MONITOR_ERR;
	
	*pFerThr = gPonThreashold.ponFERClearThreashold ;
	return MONITOR_OK;
}





extern int ponPortBERAlarm_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx, ulong_t ber )	;

short int monPonAlarmCheck(short int PonId, 
						ulong_t frameErr, ulong_t frameTotal,
						ulong_t bitErr, ulong_t bitTotal)
{/*告警计算*/
	ulong_t FramedivedNum = 0;
	ulong_t FramedivNum = 0;
	ulong_t BitdivedNum = 0;
	ulong_t BitdivNum = 0;	
	ulong_t /*devIdx = 1,*/ brdIdx = 0xff, ulPonIdx = 0;
	ulong_t frameThrValue = 0;	
	ulong_t bitThrValue = 0;
	/* modified by xieshl 20091216, 直接用函数转换，以便支持非6700设备 */
	/*if(( PonId < 4 ) && ( PonId >= 0 )) { brdIdx = 8; }
	else if(( PonId < 8 ) && ( PonId >= 4 )) { brdIdx = 7; }
	else if(( PonId < 12 ) && ( PonId >= 8 )) { brdIdx = 6; }
	else if(( PonId < 16 ) && ( PonId >= 12 )) { brdIdx = 5; }
	else if(( PonId < 20 ) && ( PonId >= 16 )) { brdIdx = 4; }
 	if(( PonId <0 ) ||( PonId >= 20)) 
		return( RERROR );
	ulPonIdx = ( PonId % 4 + 1 );*/
	brdIdx  = GetCardIdxByPonChip(PonId);
	if( brdIdx == RERROR )
		return RERROR;
	ulPonIdx = GetPonPortByPonChip(PonId);
	
	if (frameErr > pPreStatsEntry[PonId].badFrames)	
		FramedivedNum = (frameErr - pPreStatsEntry[PonId].badFrames);
	else 
		FramedivedNum = 0;
	if (frameTotal > pPreStatsEntry[PonId].totalFrames)
		FramedivNum = (ulong_t)(frameTotal - pPreStatsEntry[PonId].totalFrames);
	else
		FramedivNum = 0;

	if (bitErr > pPreStatsEntry[PonId].badOctets)
		BitdivedNum = bitErr - (ulong_t)pPreStatsEntry[PonId].badOctets;
	else
		BitdivedNum = 0;
	if (bitTotal > pPreStatsEntry[PonId].totalOctets)
		BitdivNum = bitTotal - (ulong_t)pPreStatsEntry[PonId].totalOctets;
	else
		BitdivNum = 0;
	
	/*帧错误率告警信息*/
	#if 0
	printf(" FramedivedNum = %d\r\n",FramedivedNum);
	printf(" FramedivNum = %d\r\n",FramedivNum);
	printf(" frameThrVlaue = %d\r\n",frameThrValue);
	printf(" BitdivedNum = %d\r\n",BitdivedNum);
	printf(" BitdivNum = %d\r\n",BitdivNum);
	printf(" bitThrValue = %d\r\n",bitThrValue);
	#endif
	frameThrValue = monFerCalcRatio( FramedivedNum, FramedivNum );
	bitThrValue = monBerCalcRatio( BitdivedNum, BitdivNum );
	/* deleted by chenfj 2007-10-29 
		1 PAS-SOFT可直接产生PON ber/fer告警，可删去此处的软件计算
		2 在PAS5001下启动OLT PON ber/fer 告警使能，此处软件计算会导致ber/fer 误报
	*/	
#if 0
	if (gpPonAlarm[PonId]->ponFERAlarm == 1 )
	{/*如果使能*/
		unsigned int ferThr = 0;
		unsigned int ferClearThr = 0;
		unsigned int ferNum = 0;

		monponFERNumGet(&ferNum);			
		monponFERThrGet( &ferThr );
		monponFERClearThrGet( &ferClearThr );
		if (pPreStatsEntry[PonId].ferStatus == TRUE)
		{
			
			#if 0
			if ( FramedivNum < ferNum )
			{/*消除告警*/
				
				pPreStatsEntry[PonId].ferStatus = FALSE;
				/*ponPortFERAlarmClear_EventReport( devIdx, brdIdx, ponIdx, 
													(ulong_t)(frameThrValue));*/
			}
			else 
			#endif	
			if ( frameThrValue < ferClearThr)
			{/*消除告警*/
				
				pPreStatsEntry[PonId].ferStatus = FALSE;
				ponPortFERAlarmClear_EventReport( devIdx, brdIdx, ulPonIdx, (ulong_t)(frameThrValue));
			}
			
		}
		else
		{
			
			#if 0
			if ( FramedivNum < ferNum )
			{/*如果错误帧数没有达到一定数量级,则不允许发告警
			以预防误报*/}
			}			
			else 
			#endif
			if ( frameThrValue > (ferThr))
			{/*发告警*/
				pPreStatsEntry[PonId].ferStatus = TRUE;
				ponPortFERAlarm_EventReport( devIdx, brdIdx, ulPonIdx, (ulong_t)(frameThrValue));
			}
		}
		pPreStatsEntry[PonId].frameThre = frameThrValue;
	}

	if (gpPonAlarm[PonId]->ponBERAlarm == 1 )
	{/*误码率*/
		unsigned int berThr = 0;
		unsigned int berClearThr = 0;
		unsigned int berNum = 0;
		monponBERThrGet( &berThr );
		monponBERClearThrGet( &berClearThr );
		monponBERNumGet(&berNum);

		#if 0
		if (pPreStatsEntry[PonId].badOctets < bitErr)
			divedNum = (bitErr - pPreStatsEntry[PonId].badOctets);
		else
			divedNum = 0;
		if (pPreStatsEntry[PonId].totalOctets)
			divNum = (bitTotal - pPreStatsEntry[PonId].totalOctets);
		#endif
		if (pPreStatsEntry[PonId].berStatus == TRUE)
		{
			if (bitThrValue < berClearThr)
			{/*消除告警*/
				ponPortBERAlarmClear_EventReport( devIdx, brdIdx, ulPonIdx, \
								(ulong_t)(bitThrValue));
				pPreStatsEntry[PonId].berStatus = FALSE;
			}
		}
		else
		{
			if (bitThrValue > berThr)
			{
				ponPortBERAlarm_EventReport( devIdx, brdIdx, ulPonIdx, 
											(ulong_t)(bitThrValue) );
				pPreStatsEntry[PonId].berStatus = TRUE;
			}
		}
		pPreStatsEntry[PonId].octetThre = bitThrValue;
	}/*endif 误码率*/
#endif
	pPreStatsEntry[PonId].badFrames = frameErr;
	pPreStatsEntry[PonId].totalFrames = frameTotal;
	pPreStatsEntry[PonId].totalOctets = bitTotal;
	pPreStatsEntry[PonId].badOctets = bitErr;
	return MONITOR_OK;
}


/*输入参数: 1- true
				2- false*/
short int monStatusSet(short int  status)
{
	BOOL StatusEn;
	/*if ((status != ALARM_ENABLE) && (status != ALARM_DISABLE)) 
		return MONITOR_ERR;*/
	if (status == ALARM_ENABLE)
		StatusEn = TRUE;
	else if (status == ALARM_DISABLE)
		StatusEn = FALSE;
	else
		return MONITOR_ERR;
	gPonThreashold.ponMonStatus = status;
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	Pon_MonitorEnable(StatusEn);
	
	return MONITOR_OK;
}

short int monStatusGet(short int  *pStatus)
{
	if (pStatus == NULL)
		return MONITOR_ERR;
	*pStatus = gPonThreashold.ponMonStatus;
	return MONITOR_OK;
}

int CopyOltBerAlarm(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    int iRlt = 0;
    int iSrcValue[2];

    iSrcValue[0] = gpOnuAlarm[SrcPonPortIdx]->ponBERAlarm;
    iSrcValue[1] = gpPonAlarm[SrcPonPortIdx]->ponBERAlarm;

    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OLT_SetBerAlarm(DstPonPortIdx, iSrcValue[0], -1, -1);
        if ( 0 == iRlt )
        {
            iRlt = OLT_SetPonBerAlarm(DstPonPortIdx, iSrcValue[1], -1, -1);
        }
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( V2R1_ENABLE != iSrcValue[0] )
        {
            iRlt = OLT_SetBerAlarm(DstPonPortIdx, iSrcValue[0], -1, -1);
            if ( 0 != iRlt )
            {
                return iRlt;
            }
        }

        if ( V2R1_ENABLE != iSrcValue[1] )
        {
            iRlt = OLT_SetPonBerAlarm(DstPonPortIdx, iSrcValue[1], -1, -1);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            int iDstValue[2];
            
            iDstValue[0] = gpOnuAlarm[DstPonPortIdx]->ponBERAlarm;
            iDstValue[1] = gpPonAlarm[DstPonPortIdx]->ponBERAlarm;

            if ( iDstValue[0] != iSrcValue[0] )
            {
                iRlt = OLT_SetBerAlarm(DstPonPortIdx, iSrcValue[0], -1, -1);
                if ( 0 != iRlt )
                {
                    return iRlt;
                }
            }

            if ( iDstValue[1] != iSrcValue[1] )
            {
                iRlt = OLT_SetPonBerAlarm(DstPonPortIdx, iSrcValue[1], -1, -1);
            }
        }
        else
        {
            iRlt = OLT_SetBerAlarm(DstPonPortIdx, iSrcValue[0], -1, -1);
            if ( 0 == iRlt )
            {
                iRlt = OLT_SetPonBerAlarm(DstPonPortIdx, iSrcValue[1], -1, -1);
            }
        }
    }
			
    return iRlt;
}

int CopyOltFerAlarm(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    int iRlt = 0;
    int iSrcValue[2];

    iSrcValue[0] = gpOnuAlarm[SrcPonPortIdx]->ponFERAlarm;
    iSrcValue[1] = gpPonAlarm[SrcPonPortIdx]->ponFERAlarm;

    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OLT_SetFerAlarm(DstPonPortIdx, iSrcValue[0], -1, -1);
        if ( 0 == iRlt )
        {
            iRlt = OLT_SetPonFerAlarm(DstPonPortIdx, iSrcValue[1], -1, -1);
        }
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( V2R1_ENABLE != iSrcValue[0] )
        {
            iRlt = OLT_SetFerAlarm(DstPonPortIdx, iSrcValue[0], -1, -1);
            if ( 0 != iRlt )
            {
                return iRlt;
            }
        }

        if ( V2R1_ENABLE != iSrcValue[1] )
        {
            iRlt = OLT_SetPonFerAlarm(DstPonPortIdx, iSrcValue[1], -1, -1);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            int iDstValue[2];
            
            iDstValue[0] = gpOnuAlarm[DstPonPortIdx]->ponFERAlarm;
            iDstValue[1] = gpPonAlarm[DstPonPortIdx]->ponFERAlarm;

            if ( iDstValue[0] != iSrcValue[0] )
            {
                iRlt = OLT_SetFerAlarm(DstPonPortIdx, iSrcValue[0], -1, -1);
                if ( 0 != iRlt )
                {
                    return iRlt;
                }
            }

            if ( iDstValue[1] != iSrcValue[1] )
            {
                iRlt = OLT_SetPonFerAlarm(DstPonPortIdx, iSrcValue[1], -1, -1);
            }
        }
        else
        {
            iRlt = OLT_SetFerAlarm(DstPonPortIdx, iSrcValue[0], -1, -1);
            if ( 0 == iRlt )
            {
                iRlt = OLT_SetPonFerAlarm(DstPonPortIdx, iSrcValue[1], -1, -1);
            }
        }
    }
			
    return iRlt;
}

#if 0	/* removed by xieshl 20091216, 去掉调试函数 */
/*added by wutongwu at October 13*/

int monAlmEnStateAllShow(void)
{
	short int ponId = 0;
	short int onuId = 0;
	unsigned int AlmEn = 0;
	for(ponId = 0;ponId < 20;ponId++)
	{
		AlmEn = 0;
		monPonBerAlmEnGet(ponId, &AlmEn);
		sys_console_printf("  BerAlmEn ponId %d AlmEn %d\r\n",ponId,AlmEn);
		AlmEn = 0;
		monPonFerAlmEnGet(ponId, &AlmEn);
		sys_console_printf("  FerAlmEn ponId %d AlmEn %d\r\n",ponId,AlmEn);
	}

	for(ponId = 0;ponId < 20;ponId++)
	{
		for(onuId = 0;onuId<64;onuId++)
		{
		AlmEn = 0;
		monOnuBerAlmEnGet(ponId, &AlmEn);
		sys_console_printf("  BerAlmEn ponId %d onuId %d AlmEn %d\r\n",ponId,onuId,AlmEn);
		AlmEn = 0;
		monOnuFerAlmEnGet(ponId,&AlmEn);
		sys_console_printf("  FerAlmEn ponId %d onuId %d AlmEn %d\r\n",ponId,onuId,AlmEn);
		}
	}
	return MONITOR_OK;
}

int monAlmEnStateShow(short int ponId)
{
	short int onuId = 0;
	unsigned int AlmEn = 0;
	sys_console_printf("\r\n");
	AlmEn = 0;
	monPonBerAlmEnGet(ponId, &AlmEn);
	sys_console_printf("  BerAlmEn ponId %d AlmEn %d\r\n",ponId,AlmEn);
	AlmEn = 0;
	monPonFerAlmEnGet(ponId, &AlmEn);
	sys_console_printf("  FerAlmEn ponId %d AlmEn %d\r\n",ponId,AlmEn);

	for(onuId = 0;onuId<64;onuId++)
	{
	AlmEn = 0;
	monOnuBerAlmEnGet(ponId, &AlmEn);
	sys_console_printf("  BerAlmEn ponId %d onuId %d AlmEn %d\r\n",ponId,onuId,AlmEn);
	AlmEn = 0;
	monOnuFerAlmEnGet(ponId, &AlmEn);
	sys_console_printf("  FerAlmEn ponId %d onuId %d AlmEn %d\r\n",ponId,onuId,AlmEn);
	}
	return MONITOR_OK;
}





/*added by wutw at 17 October*/
int monTest(void)
{
	long double dTemp = 0.456783256;
	long double dTemp_1 = 0.00000000000456783256;
	sys_console_printf("  dTemp = %lf\r\n",dTemp);
	sys_console_printf("  dTemp = %le\r\n",dTemp);
	sys_console_printf("  dTemp_1 = %lf\r\n",dTemp_1);
	sys_console_printf("  dTemp_1 = %le\r\n",dTemp_1);	
	return 0;
}


short int monPonperfDownBerShow(unsigned long ponId, unsigned short onuId)
{
	short int ret = MONITOR_OK;
	long double stats_data = 0;
	unsigned int DownBer = 0;
	short int llid = 0;
	
	llid = GetLlidByOnuIdx( ponId, onuId-1);
	if ( MON_INVALID_LLID == llid)
		return MONITOR_ERR;
	ret = PAS_get_statistics((unsigned short )ponId,MON_MAX_OLT, PON_STAT_ONU_BER, llid,&stats_data);
	if (ret != MONITOR_OK)
		return ret;
	DownBer = (unsigned int)(stats_data * ALARM_BASE_ERRRATE);

	sys_console_printf(" stats_data %lf \r\n",stats_data);
	sys_console_printf(" stats_data %le \r\n",stats_data);
	sys_console_printf("\r\nPon %d BER  is %g \r\n", ponId, DownBer );

	return MONITOR_OK;

}


short int monPonBerSet(unsigned short oltId, unsigned int berAlmEn)
{
	bool activate;
	short int ret = MONITOR_OK;
	PON_ber_alarm_configuration_t berAlarmCfg;
	unsigned int Thr = 1;
	memset(&berAlarmCfg, 0, sizeof(PON_ber_alarm_configuration_t));

	/*需要初始化的时候设置默认门限值*/
	berAlarmCfg.ber_threshold = 1e-7;
	berAlarmCfg.direction = (PON_pon_network_traffic_direction_t)ALARM_DOWN_AND_UP;  
	berAlarmCfg.minimum_error_bytes_threshold = 10000*Thr;
	/*berAlarmCfg.TBD = 0;*/
	/**/
	if ((berAlmEn == ALARM_ENABLE))
		activate = TRUE;
	else if (berAlmEn == ALARM_DISABLE)
		activate = FALSE;
	else
		return MONITOR_ERR;
	/*1. 获取在线olt ,并设置该olt的门限制*/
	ret = PAS_set_alarm_configuration (oltId, MON_MAX_OLT,PON_ALARM_BER, activate, &berAlarmCfg);
	if (ret != MONITOR_OK)
		return ret;
	return ret;
}

int monshow(void)
{
	short int mon_status = 0;
	monStatusGet(&mon_status);
	sys_console_printf("  mon_status %d\r\n",mon_status);
	return MONITOR_OK;
}
#endif

