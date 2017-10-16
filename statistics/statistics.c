/**************************************************************************************
*statistics.c - .
* 
* Copyright (c) 2006 GW Technologies Co., LTD.
* All rights reserved.
* 
* create: 
* 2006-05-20  wutw
* 
* 文件描述：
**history:
*1  created by wuTw
* date : 
*
*2 modify be chenfj 2006/09/26 
*   #2678 问题一定配置下，给一个trunk加入成员端口，系统出现异
*3 modified by wutw at 15 October
*	修正从5001获取历史统计参数部分。增加历史统计的时间。
*4 modified by wutw at 31 October
*	使能onu历史统计之后，当onu离线后，占用cpu使用率96%，采集任务中，当判断onu
*	离线后，则程序直接返回，而没有对onu的统计列表指针指向下一个统计对象，导致
*	一直对当前离线的onu进行统计，进入死循环，导致cpu占用率很大。修改:当onu离线，
*	程序判断onu离线时，将统计列表的指针下移到下一个对象，遂解决该问题，同样问题
*	对pon的历史统计。
*5 modified by wutw at 20 November
*	发现执行statistic-history pon 15m与statistic-history pon 24h不能被执行
*	只有statistisc-history pon 被执行，遂进行修改
*	并在pon节点下增加show statistic-history命令
*	增加计算olt pon端口的fer与ber
*6 modified by wutw at 23November
*	进行pclint检查，修改error与warning。
*7 modified by wutw at 29 November
*	修改获取系统时间的参数
*8 modified by wutw 2007/1/17
*	增加相关参数，用于读取5001的pon端口，cni端口，hostmsg的统计值
*	读取6201的pon端口,sni端口的统计值
* 9 modified by chenfj 2008-1-14
*    程序中使用的PAS 结构定义，或PAS-API，统一从文件includefromPas.h中引用
*
* modified by xieshl 20101217, 实时统计数据直接从IF中获取，不再定时缓存；
*    PON统计数据也从对应以太网口读取，不再从PON芯片获取数据；
*
**************************************************************************************/

#include "VOS_Base.h"
#include "VOS_types.h"
#include "Vos_typevx.h"
#include "Vos_sem.h"
#include "Vos_que.h"
#include "Vos_task.h" 

#include "Vos_timer.h"

 #include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "V2R1_product.h"
#include "V2R1General.h"
#include "statistics.h"
/*
BEGIN commented by wangxy 2007-06-28
repeated include, 造成实时／历史统计中对系统板状态查询失败，
可能是头文件中的定义使存放板信息的位置查错了
#include "cpi/typesdb/typesdb_product.h"
#include "cpi/typesdb/typesdb_module.h"
*/
/*#include "../superset/platform/sys/main/sys_main.h"
#include "../superset/platform/include/man/cli/cli.h"*/
#include "bmsp/product_info/bms_product_info.h"

#include "monitor/monitor.h"
/*#include "eventMain.h"*/

#include "cpi/ctss/ctss_ifm.h"
#include "ifm_act.h"
#include "ifm_pub.h"
#include "ifm_aux.h"
#include "Eth_type.h"

#include "../mib/gweponsys.h"

/*====================================*/
#define STAT_INVALID_LLID	 INVALID_LLID  /*(-1) */
/*
#ifndef PONCHIP_PAS5001
#define  PONCHIP_PAS5001  1
#endif

#ifndef PONCHIP_PAS5201
#define  PONCHIP_PAS5201  2
#endif
*/
#undef __STAT_DEBUG
#ifdef __STAT_DEBUG
ULONG gStatisticDebugFlag = 0;
#endif

/* modified by xieshl 20090204, ONU统计信息可能对PON有很大影响，可能是造成PON异常的 */
#undef __RT_ONU_RAW	/* 20090805 暂时放开，仅用于测试，正式版本中不定义 */
#undef __RT_CALCULATE_BW
/*#define	STATS_DEBUG	*/

/*测量对象表：*/
statistic_PON_OID  *gpPONHisStatsHead = NULL;
statistic_PON_OID  *gpPONHisStatsTail = NULL;
statistic_PON_OID  *gpONUHisStatsHead = NULL;
statistic_PON_OID  *gpONUHisStatsTail = NULL;

statistic_ETH_PORT *gpEthHisStatsHead = NULL;
statistic_ETH_PORT *gpEthHisStatsTail = NULL;

/* 所有进行历史数据统计的PON端口计数： */
unsigned int  PONHistoryStatsCounter = 0;

/*所有进行历史数据统计的ONU计数： */
unsigned int  ONUHistoryStatsCounter = 0;   

UINT	ethHistoryStatsCounter = 0;

LONG HisStatsTimerId = 0;
unsigned int HisStatPeriod = 1000*15*60;/*15*60 ,15 minutes*/
/*LONG RealTimeStatsTimerId = 0;*/
unsigned int RealTimeStatPeriod = 1000*30;/*15*60 ,15 minutes*/
unsigned short 	gHis15MinMaxRecord = STATS_DEFAULT15MIN_BUCKETNUM;
unsigned short 	gHis24HoursMaxRecord = STATS_DEFAULT24H_BUCKETNUM;

ULONG		gMQidsStats = 0;
ULONG		statsTaskId = 0;
unsigned int 	gHisStatsInitStatus = 0;

extern ULONG ulGPON_GEMPORT_BASE;

UCHAR *gStatsPonOn;
UCHAR (*gStatsOnuOn)[MAXONUPERPONNOLIMIT];/*STATS_HIS_MAXONU*/
/*unsigned char *gStatsEthernetOn;
int gRtStatisticClearStatus = 0;*/

/*rtStatsEntry64	**pgStatsRealTimeOlt;
rtStatsEntry64* (*pgStatsRealTimeOnu)[STATS_HIS_MAXONU];*/

/*以太网端口使用静态表 modify by wangxy 2007-03-11*/
/*rtStatsEntry64	*gStatsRealTimeEth;*/

/*rtStatsEntry	**tpgStatsRealTimeOlt;
rtStatsEntry*   (*tpgStatsRealTimeOnu)[STATS_HIS_MAXONU];
rtStatsEntry	*tgStatsRealTimeEth;*/

/*Statistics_debug_Table **pgDebug_StatsRealTimeOlt;
Statistics_debug_Table* (*pgDebug_StatsRealTimeOnu)[STATS_HIS_MAXONU];
int stat_result_show = 0;*/

#if 0
/*system parameter setting (for statistic and monitor syscle.)*/
typedef struct
{
	short int  TBD; 
	long int   statistics_sampling_cycle;       
	long int   monitoring_cycle;				
	short int  host_olt_msgs_timeout;
	short int  olt_reset_timeout;
	short int  automatic_authorization_policy;
} PAS_system_parameters_t;
#endif

enum{
	OLT_PON_PORT=1,
	ONU_PON_PORT
};

#if EXTERN_DECLARE
#endif

/*==========外部函数声明=============*/

/*#ifndef PAS_STATISTIC_5201
extern short int HisStatsRawGet( const short int						 olt_id, 
					const short int						 collector_id, 
					const PON_raw_statistics_t			 raw_statistics_type, 
					const short int						 statistics_parameter,
					const PON_statistics_query_method_t	 collecting_method,
					void								*statisticsData,
					PON_timestamp_t					*timesTamp );
#else
short int HisStatsRawGet( const short int						 olt_id, 
							const short int						 collector_id, 
							const PON_raw_statistics_t			 raw_statistics_type, 
							const short int						 statistics_parameter,
							void									*statistics_data,
							PON_timestamp_t						*timestamp );
#endif*/

/*外部函数声明*/
extern short int GetPonPortIdxBySlot( short int slot, short  int port );
extern short int GetLlidByOnuIdx( short int PonPortIdx, short int OnuIdx );
extern int	sprintf64Bits( char *const szText, unsigned long long val );
#if 0
typedef short int  PON_olt_id_t;  /* */
typedef short int  PON_onu_id_t;
extern short int Is_passave_onu ( const PON_olt_id_t   olt_id, 
						   const PON_onu_id_t   onu_id,
						   bool				   *passave_onu );
extern bool Olt_exists ( const short int olt_id );
#endif
#if 0
extern short int PAS_get_raw_statistics ( const short int						 olt_id, 
								   const short int						 collector_id, 
								   const PON_raw_statistics_t			 raw_statistics_type, 
								   const short int						 statistics_parameter,
								   void									*statistics_data,
								   PON_timestamp_t						*timestamp 
								   );

extern short int PAS_set_system_parameters ( const PAS_system_parameters_t  *system_parameters );
#endif
extern ULONG VOS_GetTick(VOID);


/*==========内部函数声明=============*/
#if	LOCAL_DECLARE
#endif

/*static void calculatePonFer( rtStatsEntry *  );
static void calculatePonBer( rtStatsEntry *  );
static void calculateRtBandWidth( rtStatsEntry *,  const char type  );*/

int  calculateBandWidth_Pon(short int PonId,rtStatsEntry *rtStatsTemp);
int  calculateBandWidth_Onu(short int PonId,short int OnuId,rtStatsEntry *rtStatsTemp);


short int HisStatsChain24HoursAdd(short int bucketsNum);
short int HisStatsChain15MinAdd(short int bucketsNum);
short int HisStatsChain24HoursDel(short int bucketsNum);
short int HisStatsChain15MinDel(short int bucketsNum);
short int HisStatsChainBucketNumAdd(short int bucketsNum,statistc_HISTORY_CHAIN *pstatsCurrent);
short int HisStatsChainBucketNumDel(short int bucketsNum,statistc_HISTORY_CHAIN *pstatsCurrent, statistc_HISTORY_CHAIN**, statistc_HISTORY_CHAIN**);

statistc_HISTORY_CHAIN *HisStatsChainCreate(short int bucketsNum,statistc_HISTORY_CHAIN **ppstatsHead, statistc_HISTORY_CHAIN **ppstatsTail);

int HisStatsChainDelete(statistc_HISTORY_CHAIN **ppHead, statistc_HISTORY_CHAIN **ppTail);


/*int HisStatsPonPro (const short int ponId, const unsigned int bucketNum15, const unsigned int bucketNum24, const BOOL flag15M, const BOOL flag24H, const uchar_t );*/
int HisStatsPonPro (const short int ponId, const unsigned int bucketNum, const BOOL flag, const uchar_t actcode );
int HisStatsOnuPro(const short int ponId, const short int onuId,  const unsigned int bucketNum,  const BOOL flag,  const uchar_t actcode );
/*int HisStatsOnuPro(const short int ponId, const short int onuId,  const unsigned int bucketNum15, const unsigned int bucketNum24,  const BOOL flag15M, const BOOL flag24H, const uchar_t );*/

short int HisStatsMsgTimeOutSend(void);
STATUS HisStatsPonRawGet(void);
STATUS HisStatsOnuRawGet(void);


int StatsRealTimePonPro (const short int ponId, unsigned char onStatus);
int StatsRealTimeOnuPro (const short int ponId, const short int onuId, unsigned char onStatus);
#ifdef __RT_ONU_RAW
STATUS StatsRealTimePonRawGet(short int oltId);
STATUS StatsRealTimeOnuRawGet(unsigned short oltId, unsigned short onuId);
STATUS StatsRealTimeDataGet(void);
short int StatsMsgRealTimeSend(void);
#endif
STATUS StatsMsgPonOnSend(unsigned short oltId, unsigned char OnStatus);
STATUS StatsMsgOnuOnSend(unsigned short oltId, unsigned short onuId, unsigned char OnStatus);
STATUS rtStatsOltDataFirstIndex(unsigned short *pOltId);
STATUS rtStatsOltDataNextIndexGet(unsigned short oltid, unsigned short *pNextOltId);
/*STATUS mibRtStatsEthernetDataGet(short int StatsObjType, ulong_t  slot, ulong_t  port, unsigned long long *pStatsData);
STATUS mibRtStatsOltDataGet(short int StatsObjType, ulong_t slot, ulong_t port, unsigned long long *pStatsData);
STATUS mibRtStatsOnuDataGet(short int StatsObjType, ulong_t slot, ulong_t port, ulong_t onuId, unsigned long long *pStatsData);*/
STATUS rtStatsOnuDataGet(short int StatsObjType, short int oltId, short int onuId, unsigned long *pStatsData);
int HisStatsPonStatsStart (short int ponId, BOOL Done);


int StatsInit(VOID);

void StatsTask(void);

#if LOCAL_IMPLEMENT
#endif





short int HisStatsChain24HoursAdd(short int bucketsNum)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	statistic_PON_OID	*pstatsOnu = gpONUHisStatsHead ;
	statistic_ETH_PORT	*pstatsEth = gpEthHisStatsHead;
	short int iRes = STATS_OK;
	
	if (0 == bucketsNum)
		return STATS_ERR;	
	while(pstatsPon != NULL)
	{
		if ((pstatsPon->iUsed24Hour) && (NULL != pstatsPon->pCurrent24H))
			iRes = HisStatsChainBucketNumAdd(bucketsNum,pstatsPon->pCurrent24H);
		if (STATS_OK != iRes)
			return iRes;			
		pstatsPon = pstatsPon->pNext;
	}
	
	while (pstatsOnu != NULL)
	{
		if ((pstatsOnu->iUsed24Hour) && (NULL != pstatsOnu->pCurrent24H))
			iRes = HisStatsChainBucketNumAdd(bucketsNum,pstatsOnu->pCurrent24H);				
		if (STATS_OK != iRes)
			return iRes;			
		pstatsOnu = pstatsOnu->pNext;
	}

/*begin:
added by wangxiaoyu 2008-02-26
*/
	while(pstatsEth != NULL)
	{
		if(pstatsEth->iUsed24Hour&& (NULL != pstatsEth->pCurrent24H))
			iRes = HisStatsChainBucketNumAdd(bucketsNum, pstatsEth->pCurrent24H);
		if(STATS_OK != iRes)
			return iRes;
		pstatsEth = pstatsEth->pNext;
	}
/*end*/

	return iRes;
}

short int HisStatsChain15MinAdd(short int bucketsNum)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	statistic_PON_OID	*pstatsOnu = gpONUHisStatsHead ;
	statistic_ETH_PORT	*pstatsEth = gpEthHisStatsHead;
	
	short int iRes = STATS_OK;

	if (0 == bucketsNum)
		return STATS_ERR;
	while(pstatsPon != NULL)
	{
		if ((pstatsPon->iUsed15Min) && (NULL != pstatsPon->pCurrent15M))
			iRes = HisStatsChainBucketNumAdd(bucketsNum,pstatsPon->pCurrent15M);
		if (STATS_OK != iRes)
			return iRes;			
		pstatsPon = pstatsPon->pNext;
	}
	
	while (pstatsOnu != NULL)
	{
		if ((pstatsOnu->iUsed15Min) && (NULL != pstatsOnu->pCurrent15M))
			iRes = HisStatsChainBucketNumAdd(bucketsNum,pstatsOnu->pCurrent15M);	
		if (STATS_OK != iRes)
			return iRes;			
		pstatsOnu = pstatsOnu->pNext;
	}

/*begin:
added by wangxiaoyu 2008-02-26
*/
	while(pstatsEth != NULL)
	{
		if(pstatsEth->iUsed15Min && (NULL != pstatsEth->pCurrent15M))
			iRes = HisStatsChainBucketNumAdd(bucketsNum, pstatsEth->pCurrent15M);
		if(STATS_OK != iRes)
			return iRes;
		pstatsEth = pstatsEth->pNext;
	}
/*end*/

	return iRes;
}



short int HisStatsChain24HoursDel(short int bucketsNum)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	statistic_PON_OID	*pstatsOnu = gpONUHisStatsHead ;
	
	statistic_ETH_PORT	*pstatsEth = gpEthHisStatsHead;
	
	short int iRes = STATS_OK;

	if (0 == bucketsNum)
		return STATS_ERR;	
	while(pstatsPon != NULL)
	{
		if ((pstatsPon->iUsed24Hour) && (NULL != pstatsPon->pCurrent24H))
			iRes = HisStatsChainBucketNumDel(bucketsNum,pstatsPon->pCurrent24H, &pstatsPon->pstats24HourHead, &pstatsPon->pstats24HourTail);
		if (STATS_OK != iRes)
			return iRes;			
		pstatsPon = pstatsPon->pNext;
	}
	
	while (pstatsOnu != NULL)
	{
		if ((pstatsOnu->iUsed24Hour) && (NULL != pstatsOnu->pCurrent24H))
			iRes = HisStatsChainBucketNumDel(bucketsNum,pstatsOnu->pCurrent24H, &pstatsOnu->pstats24HourHead, &pstatsOnu->pstats24HourTail);
		if (STATS_OK != iRes)
			return iRes;			
		pstatsOnu = pstatsOnu->pNext;
	}

/*begin:
added by wangxiaoyu 2008-02-26
*/
	while(pstatsEth != NULL)
	{
		if(pstatsEth->iUsed24Hour&& (NULL != pstatsEth->pCurrent24H))
			iRes = HisStatsChainBucketNumDel(bucketsNum, pstatsEth->pCurrent24H, &pstatsEth->pstats24HourHead, &pstatsEth->pstats24HourTail);
		if(STATS_OK != iRes)
			return iRes;
		pstatsEth = pstatsEth->pNext;
	}
/*end*/

	return iRes;
}



short int HisStatsChain15MinDel(short int bucketsNum)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	statistic_PON_OID	*pstatsOnu = gpONUHisStatsHead ;
	
	statistic_ETH_PORT	*pstatsEth = gpEthHisStatsHead;
	
	short int iRes = STATS_OK;
	
	if (0 == bucketsNum)
		return STATS_ERR;	
	while(pstatsPon != NULL)
	{
		if ((pstatsPon->iUsed15Min) && (NULL != pstatsPon->pCurrent15M))
			iRes = HisStatsChainBucketNumDel(bucketsNum,pstatsPon->pCurrent15M, &pstatsPon->pstats15MinHead, &pstatsPon->pstats15MinTail);
		if (STATS_OK != iRes)
			return iRes;			
		pstatsPon = pstatsPon->pNext;
	}
	
	while (pstatsOnu != NULL)
	{
		if ((pstatsOnu->iUsed15Min) && (NULL != pstatsOnu->pCurrent15M))
			iRes = HisStatsChainBucketNumDel(bucketsNum,pstatsOnu->pCurrent15M, &pstatsOnu->pstats15MinHead, &pstatsOnu->pstats15MinTail);
		if (STATS_OK != iRes)
			return iRes;			
		pstatsOnu = pstatsOnu->pNext;
	}

/*begin:
added by wangxiaoyu 2008-02-26
*/
	while(pstatsEth != NULL)
	{
		if(pstatsEth->iUsed15Min && (NULL != pstatsEth->pCurrent15M))
			iRes = HisStatsChainBucketNumDel(bucketsNum, pstatsEth->pCurrent15M, &pstatsEth->pstats15MinHead, &pstatsEth->pstats15MinTail);
		if(STATS_OK != iRes)
			return iRes;
		pstatsEth = pstatsEth->pNext;
	}
/*end*/
	
	return iRes;
}

/****************************************************
* HisStatsChainBucketNumAdd
* 描述: 增加环形双向链表结构并分别对链表进行初始化
*		注意:本模块仅将该环形双向链表当作一个单项环形链表使用
*							
*								
*							
*		
*
*	输入: 无
*
*	输出: 无
*
*	返回: 无
******************************************************/
short int HisStatsChainBucketNumAdd(short int bucketsNum,statistc_HISTORY_CHAIN *pstatsCurrent)
{
	short int tempCount = 0;
	statistc_HISTORY_CHAIN *pTempCurrent = NULL;
	statistc_HISTORY_CHAIN *pTemp = NULL;
	statistc_HISTORY_CHAIN *pTempHead = NULL;
	statistc_HISTORY_CHAIN *pTempTail = NULL;
	if (NULL == pstatsCurrent)
		return STATS_ERR;
	pTempHead= (statistc_HISTORY_CHAIN*)VOS_Malloc((sizeof(statistc_HISTORY_CHAIN)), MODULE_STATSTICS);
	if (NULL == pTempHead)
		{
			return STATS_OK;
		}
	pTempHead->bstatus = FALSE;
	pTempTail = pTempHead;
	pTempTail->pNext = NULL;
	pTempHead->pPre = NULL;
	for(tempCount=0;tempCount<(bucketsNum-1);tempCount++)
		{
			pTemp= (statistc_HISTORY_CHAIN*)VOS_Malloc((sizeof(statistc_HISTORY_CHAIN)), MODULE_STATSTICS);
			if (NULL == pTemp)
				{
					pTemp= (statistc_HISTORY_CHAIN*)VOS_Malloc((sizeof(statistc_HISTORY_CHAIN)), MODULE_STATSTICS);
					if (NULL == pTemp)
						{
						/*释放所有的*/
							pTemp = pTempHead;
							while(pTemp != NULL)
								{
									pTemp = pTempHead->pNext;
									VOS_Free((VOID *)pTempHead);
									pTempHead = pTemp;
								}
							pTempHead = NULL;
							pTempTail = NULL;					
							return (STATS_ERR);
						}

				}
			memset((statistc_HISTORY_CHAIN*)pTemp, 0, sizeof(statistc_HISTORY_CHAIN));
			pTemp->bstatus = FALSE;
			/*将当前的节点于要加入的阶段互相连接*/
			pTempTail->pNext = pTemp;
			pTemp->pPre = pTempTail;
			pTempTail = pTemp;
			pTempTail->pNext = NULL;
					
		}/*end of while (tempCount...*/

	pTempCurrent = pstatsCurrent->pNext;
	/*将增加的节点头部连接到current*/
	pstatsCurrent->pNext = pTempHead;
	pTempHead->pPre = pstatsCurrent;
	/*将增加的节点的尾部连接到current->pNext*/
	pTempTail->pNext = pTempCurrent;
	pTempCurrent->pPre = pTempTail;

	pTempCurrent = NULL;
	pTempTail = NULL;
	pTempHead = NULL;
	pTempCurrent = NULL;
	return STATS_OK;			

}

/****************************************************
* HisStatsChainBucketNumAdd
* 描述: 增加环形双向链表结构并分别对链表进行初始化
*		注意:本模块仅将该环形双向链表当作一个单项环形链表使用
*							
*								
*							
*		
*
*	输入: 无
*
*	输出: 无
*
*	返回: 无
******************************************************/
#if 0
short int HisStatsChainBucketNumDel(short int bucketsNum,statistc_HISTORY_CHAIN *pstatsCurrent)
{
	statistc_HISTORY_CHAIN *pTemp = NULL;
	short int count = 0;
	if (NULL == pstatsCurrent)
		return STATS_ERR;
	
	for(count = (bucketsNum-1); count>= 0; count--)
		{
			pTemp = pstatsCurrent->pNext;
			pstatsCurrent->pNext = pTemp->pNext;
			pTemp->pNext->pPre = pstatsCurrent;
			VOS_Free((VOID *) pTemp);
			pTemp = NULL;
		}
	return STATS_OK;
}
#endif

short int HisStatsChainBucketNumDel(short int bucketsNum,statistc_HISTORY_CHAIN *pstatsCurrent, statistc_HISTORY_CHAIN **pstHeader, statistc_HISTORY_CHAIN **pstTail)
{
	statistc_HISTORY_CHAIN *pTemp = NULL;
	short int count = 0;

	int poldhead = 1, poldtail = 1;
	
	if (NULL == pstatsCurrent)
		return STATS_ERR;
	
	for(count = (bucketsNum-1); count>= 0; count--)
		{
			pTemp = pstatsCurrent->pNext;
			pstatsCurrent->pNext = pTemp->pNext;
			pTemp->pNext->pPre = pstatsCurrent;
/*begin
added by wangxiaoyu 2008-02-26*/
			if(pTemp == *pstHeader)
				poldhead = 0;
			if(pTemp == *pstTail)
				poldtail = 0;
/*end*/	
			VOS_Free((VOID *) pTemp);	
			pTemp = NULL;
		}
/*begin
added by wangxiaoyu 2008-02-26*/
	if(poldhead == 0)
	{
		*pstHeader = pstatsCurrent->pNext;
		*pstTail = pstatsCurrent;
	}
	else if(poldtail == 0 && poldhead != 0)
	{
		*pstTail = (*pstHeader)->pPre;
	}
/*end*/
	return STATS_OK;
}
/****************************************************
* CreateHisStatsChain
* 描述: 创建环形双向链表结构并分别对链表进行初始化
*		注意:本模块仅将该环形双向链表当作一个单项环形链表使用
*							
*								
*							
*		
*
*	输入: 无
*
*	输出: 无
*
*	返回: 无
******************************************************/
statistc_HISTORY_CHAIN *HisStatsChainCreate(short int bucketsNum,statistc_HISTORY_CHAIN **ppstatsHead, statistc_HISTORY_CHAIN **ppstatsTail)
{
	statistc_HISTORY_CHAIN *pTemp = NULL ;	
	statistc_HISTORY_CHAIN *pTail = NULL ;
	statistc_HISTORY_CHAIN *pHead = NULL ;
	short int tempCount = 0;
	if ((*ppstatsHead != NULL) && (*ppstatsTail != NULL))
		return NULL;
	if (bucketsNum == 0)
		return NULL;
	
	pHead = (statistc_HISTORY_CHAIN*)VOS_Malloc((sizeof(statistc_HISTORY_CHAIN)), MODULE_STATSTICS);
	memset((statistc_HISTORY_CHAIN*)pHead, 0, sizeof(statistc_HISTORY_CHAIN));
	pTail = pHead;
	pHead->pPre = pTail;
	pTail->pNext = pHead;

	
	for(tempCount=0;tempCount<(bucketsNum-1);tempCount++)
		{
		pTemp= (statistc_HISTORY_CHAIN*)VOS_Malloc((sizeof(statistc_HISTORY_CHAIN)), MODULE_STATSTICS);
		if (NULL == pTemp)
			{
			
			pTemp= (statistc_HISTORY_CHAIN*)VOS_Malloc((sizeof(statistc_HISTORY_CHAIN)), MODULE_STATSTICS);
			if (NULL == pTemp)
				{
				/*释放所有的*/
				/*free();*/
					pTemp = pHead;
					while(pTemp != NULL)
					{
						pTemp = pHead->pNext;
						VOS_Free((VOID *)pHead);
						pHead = pTemp;
					}
					pHead = NULL;
					pTail = NULL;
					return (NULL);
				}
			}
		memset((statistc_HISTORY_CHAIN*)pTemp, 0, sizeof(statistc_HISTORY_CHAIN));
		pTemp->bstatus = FALSE;
		/*将当前的节点于要加入的阶段互相连接*/
		pTail->pNext = pTemp ;
		pTemp->pPre = pTail ;
		
		pTail = pTemp;
		pTail->pNext = pHead;
		pHead->pPre = pTail;
		}/*end of while (tempCount...*/
	
	*ppstatsHead = pHead;
	*ppstatsTail = pTail;

	return pHead;
}
/************************************
*
*
*
*
**************************************/
int HisStatsChainDelete(statistc_HISTORY_CHAIN **ppHead, statistc_HISTORY_CHAIN **ppTail)
{
	statistc_HISTORY_CHAIN *pTemp = NULL;
	statistc_HISTORY_CHAIN *pTempHead = NULL;
	statistc_HISTORY_CHAIN *pTempTail = NULL;
	if ((ppHead == NULL) || (ppTail == NULL))
		return STATS_POINT_NULL_ERR;
	pTempHead  = *ppHead;
	pTempTail = *ppTail;
	/*pTemp = *ppHead;*/
	/*while(pTemp != NULL)*/
	while(pTempHead != pTempTail)
		{
			pTemp = pTempHead->pNext;
			pTempHead->pNext = NULL;
			pTempHead->pPre = NULL;
			VOS_Free((VOID *)pTempHead);
			pTempHead = pTemp;
		}
	/*释放最后一个存贮区*/
	*ppTail = NULL;
	pTempHead->pNext = NULL;
	pTempHead->pPre = NULL;
	VOS_Free((VOID *)pTempHead);
	*ppHead = NULL;
	return (STATS_OK);
}

int HisStatsEthPro(const short int slot,  const short int eth, const unsigned int bucketNum, const BOOL flag, const uchar_t actcode )
{
	statistic_ETH_PORT	*pTempstats = gpEthHisStatsHead;
	/*statistic_PON_OID  *pPreModif = pTempstats;*/
	statistic_ETH_PORT  *pAddstats = NULL;
	statistic_ETH_PORT  *pModifiedstats = NULL;
	sysDateAndTime_t dateTime;
	
	/*break when check the ponid that has be in control table,.*/
	while(pTempstats != NULL)
		{
			if(pTempstats->slotIdx == slot &&  pTempstats->ethIdx == eth )
				{
				pModifiedstats = pTempstats;
				break;
				}
			pTempstats = pTempstats->pNext;
		}

	/*to modified*/
	if (pModifiedstats != NULL)
		{
		/*先申请空间后释放后的原则*/
		
		if ((!pModifiedstats->iUsed15Min) && (flag) && (actcode==STATS_15MIN_SET))
			{
				if (NULL == HisStatsChainCreate(bucketNum, &(pModifiedstats->pstats15MinHead), &(pModifiedstats->pstats15MinTail)))
					{
					return (STATS_POINT_NULL_ERR);
					}
				/*pModifiedstats->pstats15MinHead->pCurrent = pModifiedstats->pstats15MinHead;*/
				pModifiedstats->pCurrent15M = pModifiedstats->pstats15MinHead;
				/*added by wutw at 19 October*/
				pModifiedstats->pCurrent15M->currentTick= VOS_GetTick();

				memset( &dateTime, 0, sizeof(sysDateAndTime_t));
				eventGetCurTime( &dateTime );	
				pModifiedstats->pCurrent15M->year= dateTime.year;
				pModifiedstats->pCurrent15M->month = dateTime.month;
				pModifiedstats->pCurrent15M->day =  dateTime.day;
				pModifiedstats->pCurrent15M->hour = (unsigned short)dateTime.hour;
				pModifiedstats->pCurrent15M->minute = dateTime.minute;
				pModifiedstats->pCurrent15M->second = dateTime.second;
				pModifiedstats->pCurrent15M->MillSecond = 0;

				pModifiedstats->iUsed15Min = TRUE;
				pModifiedstats->firstFlag15M = FALSE;
			}
		if ((!pModifiedstats->iUsed24Hour) && (flag) && (actcode==STATS_24HOUR_SET))
			{
				if (NULL == HisStatsChainCreate(bucketNum, &(pModifiedstats->pstats24HourHead), &(pModifiedstats->pstats24HourTail)))
					{
					return (STATS_POINT_NULL_ERR);
					}
				/*pModifiedstats->pstats24HourHead->pCurrent = pModifiedstats->pstats24HourHead;*/
				pModifiedstats->pCurrent24H = pModifiedstats->pstats24HourHead ;
				/*added by wutw at 19 October*/
				pModifiedstats->pCurrent24H->currentTick= VOS_GetTick();
				memset( &dateTime, 0, sizeof(sysDateAndTime_t));
				eventGetCurTime( &dateTime );	
				/* begin: modified by jianght 20090812 */
				/*pModifiedstats->pCurrent15M->year= dateTime.year;
				pModifiedstats->pCurrent15M->month = dateTime.month;
				pModifiedstats->pCurrent15M->day =  dateTime.day;
				pModifiedstats->pCurrent15M->hour = (unsigned short)dateTime.hour;
				pModifiedstats->pCurrent15M->minute = dateTime.minute;
				pModifiedstats->pCurrent15M->second = dateTime.second;
				pModifiedstats->pCurrent15M->MillSecond = 0;*/
				pModifiedstats->pCurrent24H->year= dateTime.year;
				pModifiedstats->pCurrent24H->month = dateTime.month;
				pModifiedstats->pCurrent24H->day =  dateTime.day;
				pModifiedstats->pCurrent24H->hour = (unsigned short)dateTime.hour;
				pModifiedstats->pCurrent24H->minute = dateTime.minute;
				pModifiedstats->pCurrent24H->second = dateTime.second;
				pModifiedstats->pCurrent24H->MillSecond = 0;				
				/* end: modified by jianght 20090812 */
				
				pModifiedstats->iUsed24Hour = TRUE;
				pModifiedstats->firstFlag24H = FALSE;
			}	
		
		if ((pModifiedstats->iUsed15Min) && (!flag)&& (actcode==STATS_15MIN_SET))
			{
				pModifiedstats->iUsed15Min = FALSE;
				/*删除链表结构*/
				HisStatsChainDelete(&(pModifiedstats->pstats15MinHead), &(pModifiedstats->pstats15MinTail));
				pModifiedstats->pstats15MinHead = NULL;
				pModifiedstats->pstats15MinTail = NULL;
				pModifiedstats->pCurrent15M = NULL;
			}
		if ((pModifiedstats->iUsed24Hour) && (!flag)&& (actcode==STATS_24HOUR_SET ))
			{
				pModifiedstats->iUsed24Hour= FALSE;
				HisStatsChainDelete(&(pModifiedstats->pstats24HourHead), &(pModifiedstats->pstats24HourTail));
				pModifiedstats->pstats24HourHead = NULL;
				pModifiedstats->pstats24HourTail = NULL;
				pModifiedstats->pCurrent24H= NULL;				
			}

		/*if del the node*/
		if (!( pModifiedstats->iUsed15Min || pModifiedstats->iUsed24Hour ))
			{	

			    /*sys_console_printf("\r\ndel this eth node \r\n" );*/
				if ((gpEthHisStatsHead == pModifiedstats)&&(pModifiedstats == gpEthHisStatsTail))
					{/*当该监控对象为头和尾*/
					gpEthHisStatsHead = NULL;
					gpEthHisStatsTail = NULL;
				}
				else if (gpEthHisStatsHead == pModifiedstats)
					{/*当目的端口为监控列表的头*/
						gpEthHisStatsHead = pModifiedstats->pNext;
						gpEthHisStatsHead->pPre = gpEthHisStatsHead;
					}
				else if (pModifiedstats == gpEthHisStatsTail)
					{/*当目的端口为监控列表的尾*/
						gpEthHisStatsTail = pModifiedstats->pPre;
						gpEthHisStatsTail->pNext= NULL;
					}
				else
					{/*为中间*/
						pModifiedstats->pPre->pNext = pModifiedstats->pNext;
						pModifiedstats->pNext->pPre = pModifiedstats->pPre;
					}

				pModifiedstats->pPre = NULL;
				pModifiedstats->pNext = NULL;
				VOS_Free((VOID *) pModifiedstats->pstatsDataPre);
				VOS_Free((VOID *) pModifiedstats);
				pModifiedstats = NULL;
				ethHistoryStatsCounter--;
				pTempstats = NULL;
			}
		
		return (STATS_OK);	
		}
	else 
		{/*new node to add.增加一个新的统计对象*/
		/*1. 判断是否为增加一个统计目标*/
		if (!flag)
			return (STATS_PONOBJ_NULL_ERR);
		
		pAddstats = (statistic_ETH_PORT *)VOS_Malloc(sizeof(statistic_ETH_PORT), MODULE_STATSTICS);
		if(pAddstats == NULL)
			return (STATS_POINT_NULL_ERR);
		VOS_MemSet((VOID * )pAddstats, 0, sizeof(statistic_ETH_PORT));
		
		pAddstats->slotIdx = slot;
		pAddstats->ethIdx = eth;
		/*pAddstats->maxBckets = bucketNum;*/
		
		pAddstats->pstatsDataPre = (PON_StatsData_Table64*)VOS_Malloc( sizeof(PON_StatsData_Table64), MODULE_STATSTICS);
		if (NULL == pAddstats->pstatsDataPre)
			{
				VOS_Free((VOID *) pAddstats);
				return (STATS_POINT_NULL_ERR);
			}
		VOS_MemSet((VOID * )pAddstats->pstatsDataPre, 0, sizeof(PON_StatsData_Table64));
		
		if(actcode == STATS_15MIN_SET)
			{
			/*创建历史统计数据的存储链表结构,并返回指针头部和尾部*/
			if (NULL == HisStatsChainCreate(bucketNum, &(pAddstats->pstats15MinHead), &(pAddstats->pstats15MinTail)))
				{
				VOS_Free((VOID *) pAddstats->pstatsDataPre);
				VOS_Free((VOID *) pAddstats);
				pAddstats = NULL;
				return (STATS_POINT_NULL_ERR);
				}
			
			/*pAddstats->pstats15MinHead->pCurrent = pAddstats->pstats15MinHead;*/
			pAddstats->pCurrent15M = pAddstats->pstats15MinHead;
			/*added by wutw at 19 October*/
			pAddstats->pCurrent15M->currentTick= VOS_GetTick();
			
			memset( &dateTime, 0, sizeof(sysDateAndTime_t));
			eventGetCurTime( &dateTime );	
			pAddstats->pCurrent15M->year= dateTime.year;
			pAddstats->pCurrent15M->month = dateTime.month;
			pAddstats->pCurrent15M->day =  dateTime.day;
			pAddstats->pCurrent15M->hour = (unsigned short)dateTime.hour;
			pAddstats->pCurrent15M->minute = dateTime.minute;
			pAddstats->pCurrent15M->second = dateTime.second;
			pAddstats->pCurrent15M->MillSecond = 0;			
				
			pAddstats->pNext = NULL;
			pAddstats->iUsed15Min = TRUE;
			pAddstats->firstFlag15M = FALSE;
			}

		if(actcode==STATS_24HOUR_SET)
		{
			if (NULL == HisStatsChainCreate(bucketNum,&(pAddstats->pstats24HourHead), &(pAddstats->pstats24HourTail)))
				{
				/*如果在此步申请内存失败,则在返回之
				前释放之前所有申请的内存*/
				HisStatsChainDelete(&(pAddstats->pstats15MinHead), &(pAddstats->pstats15MinTail));
				VOS_Free((VOID *) pAddstats->pstatsDataPre);
				VOS_Free((VOID *) pAddstats);
				return (STATS_POINT_NULL_ERR);
				}			
			

			pAddstats->pCurrent24H = pAddstats->pstats24HourHead ;
			/*added by wutw at 19 October*/
			pAddstats->pCurrent24H->currentTick= VOS_GetTick();

			memset( &dateTime, 0, sizeof(sysDateAndTime_t));
			eventGetCurTime( &dateTime );	
			pAddstats->pCurrent24H->year= dateTime.year;
			pAddstats->pCurrent24H->month = dateTime.month;
			pAddstats->pCurrent24H->day =  dateTime.day;
			pAddstats->pCurrent24H->hour = (unsigned short)dateTime.hour;
			pAddstats->pCurrent24H->minute = dateTime.minute;
			pAddstats->pCurrent24H->second = dateTime.second;
			pAddstats->pCurrent24H->MillSecond = 0;			
			
			pAddstats->pNext = NULL;
			pAddstats->iUsed24Hour = TRUE;
			pAddstats->firstFlag24H = FALSE;
			}
		
		if (gpEthHisStatsHead == NULL)
			{
				gpEthHisStatsHead = pAddstats;
				/*gpPONHisStatsHead->pSelfadd = gpPONHisStatsHead;*/
				gpEthHisStatsHead->pPre = gpEthHisStatsHead;
				gpEthHisStatsHead->pNext = NULL;
				gpEthHisStatsTail = gpEthHisStatsHead;
			}
		else
			{
				gpEthHisStatsTail->pNext = pAddstats;
				pAddstats->pPre = gpEthHisStatsTail;
				gpEthHisStatsTail = pAddstats;
				gpEthHisStatsTail->pNext = NULL;
			}
		pAddstats = NULL;
		ethHistoryStatsCounter++;

		return(STATS_OK);
	}
	

}

int HisStatsPonPro (const short int ponId, const unsigned int bucketNum, const BOOL flag, const uchar_t actcode )
{
	statistic_PON_OID  *pTempstats = gpPONHisStatsHead;
	/*statistic_PON_OID  *pPreModif = pTempstats;*/
	statistic_PON_OID  *pAddstats = NULL;
	statistic_PON_OID  *pModifiedstats = NULL;
	sysDateAndTime_t dateTime;

	/*sys_console_printf( "\r\nHisStatsPonPro: ponid=%d, flag=%d,  code=%d", ponId, flag, actcode );*/
	
	/*break when check the ponid that has be in control table,.*/
	while(pTempstats != NULL)
	{
		if(pTempstats->ponIdx == ponId)
		{
			pModifiedstats = pTempstats;
			/*return (STATS_PONID_EXIST_ERR);*/
			break;
		}
		/*pPreModif = pTempstats;*/
		pTempstats = pTempstats->pNext;
	}

	/*to modified*/
	if (pModifiedstats != NULL)
		{
		/*先申请空间后释放后的原则*/
		
		if ((!pModifiedstats->iUsed15Min) && (flag) && (actcode==STATS_15MIN_SET))
			{
				if (NULL == HisStatsChainCreate(bucketNum, &(pModifiedstats->pstats15MinHead), &(pModifiedstats->pstats15MinTail)))
					{
					return (STATS_POINT_NULL_ERR);
					}
				/*pModifiedstats->pstats15MinHead->pCurrent = pModifiedstats->pstats15MinHead;*/
				pModifiedstats->pCurrent15M = pModifiedstats->pstats15MinHead;
				/*added by wutw at 19 October*/
				pModifiedstats->pCurrent15M->currentTick= VOS_GetTick();

				memset( &dateTime, 0, sizeof(sysDateAndTime_t));
				eventGetCurTime( &dateTime );	
				pModifiedstats->pCurrent15M->year= dateTime.year;
				pModifiedstats->pCurrent15M->month = dateTime.month;
				pModifiedstats->pCurrent15M->day =  dateTime.day;
				pModifiedstats->pCurrent15M->hour = (unsigned short)dateTime.hour;
				pModifiedstats->pCurrent15M->minute = dateTime.minute;
				pModifiedstats->pCurrent15M->second = dateTime.second;
				pModifiedstats->pCurrent15M->MillSecond = 0;
			
				pModifiedstats->iUsed15Min = TRUE;
				pModifiedstats->firstFlag15M = FALSE;
			}
		if ((!pModifiedstats->iUsed24Hour) && (flag) && (actcode==STATS_24HOUR_SET))
			{
				if (NULL == HisStatsChainCreate(bucketNum, &(pModifiedstats->pstats24HourHead), &(pModifiedstats->pstats24HourTail)))
					{
					return (STATS_POINT_NULL_ERR);
					}
				/*pModifiedstats->pstats24HourHead->pCurrent = pModifiedstats->pstats24HourHead;*/
				pModifiedstats->pCurrent24H = pModifiedstats->pstats24HourHead ;
				/*added by wutw at 19 October*/
				pModifiedstats->pCurrent24H->currentTick= VOS_GetTick();
				memset( &dateTime, 0, sizeof(sysDateAndTime_t));
				eventGetCurTime( &dateTime );	
				pModifiedstats->pCurrent15M->year= dateTime.year;
				pModifiedstats->pCurrent15M->month = dateTime.month;
				pModifiedstats->pCurrent15M->day =  dateTime.day;
				pModifiedstats->pCurrent15M->hour = (unsigned short)dateTime.hour;
				pModifiedstats->pCurrent15M->minute = dateTime.minute;
				pModifiedstats->pCurrent15M->second = dateTime.second;
				pModifiedstats->pCurrent15M->MillSecond = 0;
				
				pModifiedstats->iUsed24Hour = TRUE;
				pModifiedstats->firstFlag24H = FALSE;
			}	
		
		if ((pModifiedstats->iUsed15Min) && (!flag)&& (actcode==STATS_15MIN_SET))
			{
				pModifiedstats->iUsed15Min = FALSE;
				/*删除链表结构*/
				/*VOS_Free((VOID *) pModifiedstats->pstats15Min);*/
				HisStatsChainDelete(&(pModifiedstats->pstats15MinHead), &(pModifiedstats->pstats15MinTail));
				pModifiedstats->pstats15MinHead = NULL;
				pModifiedstats->pstats15MinTail = NULL;
				pModifiedstats->pCurrent15M = NULL;
				
			}
		if ((pModifiedstats->iUsed24Hour) && (!flag)&& (actcode==STATS_24HOUR_SET ))
			{
				pModifiedstats->iUsed24Hour= FALSE;
				/*VOS_Free((VOID *) pModifiedstats->pstats24Hour);*/
				HisStatsChainDelete(&(pModifiedstats->pstats24HourHead), &(pModifiedstats->pstats24HourTail));
				pModifiedstats->pstats24HourHead = NULL;
				pModifiedstats->pstats24HourTail = NULL;
				pModifiedstats->pCurrent24H= NULL;				
			}

		/*if del the node*/
		if (!( pModifiedstats->iUsed15Min || pModifiedstats->iUsed24Hour ))
			{	
				if ((gpPONHisStatsHead == pModifiedstats)&&(pModifiedstats == gpPONHisStatsTail))
					{/*当该监控对象为头和尾*/
					gpPONHisStatsHead = NULL;
					gpPONHisStatsTail = NULL;
				}
				else if (gpPONHisStatsHead == pModifiedstats)
					{/*当目的端口为监控列表的头*/
						gpPONHisStatsHead = pModifiedstats->pNext;
						gpPONHisStatsHead->pPre = NULL;
					}
				else if (pModifiedstats == gpPONHisStatsTail)
					{/*当目的端口为监控列表的尾*/
						gpPONHisStatsTail = pModifiedstats->pPre;
						gpPONHisStatsTail->pNext= NULL;
					}
				else
					{/*为中间*/
						pModifiedstats->pPre->pNext = pModifiedstats->pNext;
						pModifiedstats->pNext->pPre = pModifiedstats->pPre;
					}

				pModifiedstats->pPre = NULL;
				pModifiedstats->pNext = NULL;
				VOS_Free((VOID *) pModifiedstats->pstatsDataPre);
				VOS_Free((VOID *) pModifiedstats);
				pModifiedstats = NULL;
				PONHistoryStatsCounter--;
				pTempstats = NULL;
			}
		
		return (STATS_OK);	
		}
	else 
		{/*new node to add.增加一个新的统计对象*/
		/*1. 判断是否为增加一个统计目标*/
		if (!flag)
			return (STATS_PONOBJ_NULL_ERR);
		
		pAddstats = (statistic_PON_OID *)VOS_Malloc(sizeof(statistic_PON_OID), MODULE_STATSTICS);
		if(pAddstats == NULL)
			return (STATS_POINT_NULL_ERR);
		VOS_MemSet((VOID * )pAddstats, 0, sizeof(statistic_PON_OID));
		
		pAddstats->ponIdx = ponId;
		/*pAddstats->maxBckets = bucketNum;*/
		pAddstats->pstatsDataPre = (PON_StatsData_Table64*)VOS_Malloc( sizeof(PON_StatsData_Table64), MODULE_STATSTICS);
		if (NULL == pAddstats->pstatsDataPre)
			{
				VOS_Free((VOID *) pAddstats);
				return (STATS_POINT_NULL_ERR);
			}
		VOS_MemSet((VOID * )pAddstats->pstatsDataPre, 0, sizeof(PON_StatsData_Table64));
		if(actcode == STATS_15MIN_SET)
			{
			/*创建历史统计数据的存储链表结构,并返回指针头部和尾部*/
			if (NULL == HisStatsChainCreate(bucketNum, &(pAddstats->pstats15MinHead), &(pAddstats->pstats15MinTail)))
				{
				VOS_Free((VOID *) pAddstats->pstatsDataPre);
				VOS_Free((VOID *) pAddstats);
				pAddstats = NULL;
				return (STATS_POINT_NULL_ERR);
				}
			
			/*pAddstats->pstats15MinHead->pCurrent = pAddstats->pstats15MinHead;*/
			pAddstats->pCurrent15M = pAddstats->pstats15MinHead;
			/*added by wutw at 19 October*/
			pAddstats->pCurrent15M->currentTick= VOS_GetTick();
			
			memset( &dateTime, 0, sizeof(sysDateAndTime_t));
			eventGetCurTime( &dateTime );	
			pAddstats->pCurrent15M->year= dateTime.year;
			pAddstats->pCurrent15M->month = dateTime.month;
			pAddstats->pCurrent15M->day =  dateTime.day;
			pAddstats->pCurrent15M->hour = (unsigned short)dateTime.hour;
			pAddstats->pCurrent15M->minute = dateTime.minute;
			pAddstats->pCurrent15M->second = dateTime.second;
			pAddstats->pCurrent15M->MillSecond = 0;			
				
			pAddstats->pNext = NULL;
			pAddstats->iUsed15Min = TRUE;
			pAddstats->firstFlag15M = FALSE;
			}

		if(actcode==STATS_24HOUR_SET)
		{
			if (NULL == HisStatsChainCreate(bucketNum,&(pAddstats->pstats24HourHead), &(pAddstats->pstats24HourTail)))
				{
				/*如果在此步申请内存失败,则在返回之
				前释放之前所有申请的内存*/
				HisStatsChainDelete(&(pAddstats->pstats15MinHead), &(pAddstats->pstats15MinTail));
				VOS_Free((VOID *) pAddstats->pstatsDataPre);
				VOS_Free((VOID *) pAddstats);
				return (STATS_POINT_NULL_ERR);
				}			
			

			pAddstats->pCurrent24H = pAddstats->pstats24HourHead ;
			/*added by wutw at 19 October*/
			pAddstats->pCurrent24H->currentTick= VOS_GetTick();

			memset( &dateTime, 0, sizeof(sysDateAndTime_t));
			eventGetCurTime( &dateTime );	
			pAddstats->pCurrent24H->year= dateTime.year;
			pAddstats->pCurrent24H->month = dateTime.month;
			pAddstats->pCurrent24H->day =  dateTime.day;
			pAddstats->pCurrent24H->hour = (unsigned short)dateTime.hour;
			pAddstats->pCurrent24H->minute = dateTime.minute;
			pAddstats->pCurrent24H->second = dateTime.second;
			pAddstats->pCurrent24H->MillSecond = 0;			
			#if 0
			VOS_GetCurrentTime( &ulRetDate, &ulRetTime, &ulRetMillSec );
			pAddstats->pCurrent24H->year= (unsigned short)((ulRetDate & 0xffff0000)>>16);
			pAddstats->pCurrent24H->month = (unsigned char)((ulRetDate & 0xff00)>>8);
			pAddstats->pCurrent24H->day =  (unsigned char)(ulRetDate & 0xff);
			pAddstats->pCurrent24H->hour = (unsigned short)((ulRetTime & 0xffff0000)>>16);
			pAddstats->pCurrent24H->minute = (unsigned char)((ulRetTime & 0xff00)>>8);
			pAddstats->pCurrent24H->second = (unsigned char)(ulRetTime & 0xff);
			pAddstats->pCurrent24H->MillSecond = ulRetMillSec;		
			#endif	
			
			pAddstats->pNext = NULL;
			pAddstats->iUsed24Hour = TRUE;
			pAddstats->firstFlag24H = FALSE;
			}
		
		if (gpPONHisStatsHead == NULL)
			{
				gpPONHisStatsHead = pAddstats;
				/*gpPONHisStatsHead->pSelfadd = gpPONHisStatsHead;*/
				gpPONHisStatsHead->pPre = gpPONHisStatsHead;
				gpPONHisStatsHead->pNext = NULL;
				gpPONHisStatsTail = gpPONHisStatsHead;
			}
		else
			{
				gpPONHisStatsTail->pNext = pAddstats;
				pAddstats->pPre = gpPONHisStatsTail;
				gpPONHisStatsTail = pAddstats;
				gpPONHisStatsTail->pNext = NULL;
			}
		pAddstats = NULL;
		PONHistoryStatsCounter++;

		return(STATS_OK);
	}
	

}

int HisStatsOnuPro(const short int ponId, const short int onuId,  const unsigned int bucketNum,  const BOOL flag,  const uchar_t actcode )
{
	statistic_PON_OID  *pTempstats = gpONUHisStatsHead;
	/*statistic_PON_OID  *pPreModif = pTempstats;*/
	statistic_PON_OID  *pAddstats = NULL;
	statistic_PON_OID  *pModifiedstats = NULL;
	sysDateAndTime_t dateTime;	
	/*ULONG   ulRetDate = 0;
	ULONG   ulRetTime = 0;
	ULONG   ulRetMillSec = 0;*/

	/*if (0 == bucketNum)
		return STATS_ERR;*/
	/*break when check the ponid that has be in control table,.*/
	while(pTempstats != NULL)
		{
			/*sys_console_printf(" pTempstats->ponIdx = %d\r\n", pTempstats->ponIdx);
			sys_console_printf(" pTempstats->onuIdx = %d\r\n", pTempstats->onuIdx);
			sys_console_printf(" ponId %d   onuId %d \r\n", ponId, onuId);*/
			if((pTempstats->ponIdx == ponId) && (pTempstats->onuIdx == onuId))
				{
				pModifiedstats = pTempstats;
				break;
				}
			/*pPreModif = pTempstats;*/
			pTempstats = pTempstats->pNext;
		}

	/*to modified*/
	if (pModifiedstats != NULL)
		{
		/*先申请空间后释放后的原则*/
		if ((!pModifiedstats->iUsed15Min) && (flag) &&(actcode==STATS_15MIN_SET) )
			{
				if (NULL == HisStatsChainCreate(bucketNum,&(pModifiedstats->pstats15MinHead), &(pModifiedstats->pstats15MinTail)))
					{
					return (STATS_POINT_NULL_ERR);
					}
				/*pModifiedstats->pstats15MinHead->pCurrent = pModifiedstats->pstats15MinHead;*/
				pModifiedstats->pCurrent15M = pModifiedstats->pstats15MinHead;
				/*added by wutw at 19 October*/
				pModifiedstats->pCurrent15M->currentTick= VOS_GetTick();
				memset( &dateTime, 0, sizeof(sysDateAndTime_t));
				eventGetCurTime( &dateTime );	
				pModifiedstats->pCurrent15M->year= dateTime.year;
				pModifiedstats->pCurrent15M->month = dateTime.month;
				pModifiedstats->pCurrent15M->day =  dateTime.day;
				pModifiedstats->pCurrent15M->hour = (unsigned short)dateTime.hour;
				pModifiedstats->pCurrent15M->minute = dateTime.minute;
				pModifiedstats->pCurrent15M->second = dateTime.second;
				pModifiedstats->pCurrent15M->MillSecond = 0;

				pModifiedstats->iUsed15Min = TRUE;
				pModifiedstats->firstFlag15M = FALSE;
			}
		if ((!pModifiedstats->iUsed24Hour) && (flag)&&(actcode==STATS_24HOUR_SET))
			{
				if (NULL == HisStatsChainCreate(bucketNum, &(pModifiedstats->pstats24HourHead), &(pModifiedstats->pstats24HourTail)))
					{
					return (STATS_POINT_NULL_ERR);
					}
				/*pModifiedstats->pstats24HourHead->pCurrent = pModifiedstats->pstats24HourHead;*/
				pModifiedstats->pCurrent24H = pModifiedstats->pstats24HourHead;
				/*added by wutw at 19 October*/
				pModifiedstats->pCurrent24H->currentTick= VOS_GetTick();
				memset( &dateTime, 0, sizeof(sysDateAndTime_t));
				eventGetCurTime( &dateTime );	
				pModifiedstats->pCurrent24H->year= dateTime.year;
				pModifiedstats->pCurrent24H->month = dateTime.month;
				pModifiedstats->pCurrent24H->day =  dateTime.day;
				pModifiedstats->pCurrent24H->hour = (unsigned short)dateTime.hour;
				pModifiedstats->pCurrent24H->minute = dateTime.minute;
				pModifiedstats->pCurrent24H->second = dateTime.second;
				pModifiedstats->pCurrent24H->MillSecond = 0;	

				pModifiedstats->iUsed24Hour = TRUE;
				pModifiedstats->firstFlag24H = FALSE;
			}	
		
		if ((pModifiedstats->iUsed15Min) && (!flag)&&(actcode==STATS_15MIN_SET))
			{
				pModifiedstats->iUsed15Min = FALSE;
				/*删除链表结构*/
				/*VOS_Free((VOID *) pModifiedstats->pstats15Min);*/
				HisStatsChainDelete(&(pModifiedstats->pstats15MinHead), &(pModifiedstats->pstats15MinTail));
				pModifiedstats->pstats15MinHead = NULL;
				pModifiedstats->pstats15MinTail = NULL;
				pModifiedstats->pCurrent15M = NULL;
			}
		if ((pModifiedstats->iUsed24Hour) && (!flag)&&(actcode==STATS_24HOUR_SET))
			{

				pModifiedstats->iUsed24Hour= FALSE;
				/*VOS_Free((VOID *) pModifiedstats->pstats24Hour);*/
				HisStatsChainDelete(&(pModifiedstats->pstats24HourHead), &(pModifiedstats->pstats24HourTail));
				pModifiedstats->pstats24HourHead = NULL;
				pModifiedstats->pstats24HourTail = NULL;
				pModifiedstats->pCurrent24H= NULL;				
			}

		/*if del the node*/
		if (!( pModifiedstats->iUsed15Min || pModifiedstats->iUsed24Hour ))
			{
				if ((gpONUHisStatsHead == pModifiedstats)&&(pModifiedstats == gpONUHisStatsTail))
					{/*当该监控对象为头和尾*/
					gpONUHisStatsHead = NULL;
					gpONUHisStatsTail = NULL;
				}
				/*else if (gpPONHisStatsHead == pModifiedstats)*/
				else if (gpONUHisStatsHead == pModifiedstats)
					{/*当目的端口为监控列表的头*/
						gpONUHisStatsHead = pModifiedstats->pNext;
						gpONUHisStatsHead->pPre = gpONUHisStatsHead;
					}
				else if (pModifiedstats == gpONUHisStatsTail)
					{/*当目的端口为监控列表的尾*/
						gpONUHisStatsTail = pModifiedstats->pPre;
						gpONUHisStatsTail->pNext= NULL;
					}
				else
					{/*为中间*/
						pModifiedstats->pPre->pNext = pModifiedstats->pNext;
						pModifiedstats->pNext->pPre = pModifiedstats->pPre;
					}
				
				pModifiedstats->pPre = NULL;
				pModifiedstats->pNext = NULL;
				VOS_Free((VOID *) pModifiedstats->pstatsDataPre);
				VOS_Free((VOID *) pModifiedstats);
				pModifiedstats = NULL;
				ONUHistoryStatsCounter--;
				pTempstats = NULL;
			}
		return (STATS_OK);	
		}
	else 
		{/*new node to add.增加一个新的统计对象*/
		/*1. 判断是否为增加一个统计目标*/
		if (!flag)
			return (STATS_PONOBJ_NULL_ERR);
		
		pAddstats = (statistic_PON_OID *)VOS_Malloc(( sizeof(statistic_PON_OID)), MODULE_STATSTICS);
		if(pAddstats == NULL)
			return (STATS_POINT_NULL_ERR);
		VOS_MemSet((VOID * )pAddstats, 0, sizeof(statistic_PON_OID));

		pAddstats->pstatsDataPre = (PON_StatsData_Table64*)VOS_Malloc(sizeof(PON_StatsData_Table64), MODULE_STATSTICS);
		if (NULL == pAddstats->pstatsDataPre)
			{
				VOS_Free((VOID *) pAddstats);
				return (STATS_POINT_NULL_ERR);
			}
		VOS_MemSet((VOID * )pAddstats->pstatsDataPre, 0, sizeof(PON_StatsData_Table64));
		pAddstats->ponIdx = ponId;
		pAddstats->onuIdx = onuId;
		/*pAddstats->maxBckets = bucketNum;*/
		
		if(actcode == STATS_15MIN_SET)
			{
			/*创建历史统计数据的存储链表结构,并返回指针头部和尾部*/
			if (NULL == HisStatsChainCreate(bucketNum,&(pAddstats->pstats15MinHead), &(pAddstats->pstats15MinTail)))
				{
				VOS_Free((VOID *) pAddstats->pstatsDataPre);
				VOS_Free((VOID *) pAddstats);
				pAddstats = NULL;
				return (STATS_POINT_NULL_ERR);
				}
			

			pAddstats->pCurrent15M = pAddstats->pstats15MinHead;
			/*added by wutw at 19 October*/
			pAddstats->pCurrent15M->currentTick= VOS_GetTick();
			memset( &dateTime, 0, sizeof(sysDateAndTime_t));
			eventGetCurTime( &dateTime );	
			pAddstats->pCurrent15M->year= dateTime.year;
			pAddstats->pCurrent15M->month = dateTime.month;
			pAddstats->pCurrent15M->day =  dateTime.day;
			pAddstats->pCurrent15M->hour = (unsigned short)dateTime.hour;
			pAddstats->pCurrent15M->minute = dateTime.minute;
			pAddstats->pCurrent15M->second = dateTime.second;
			pAddstats->pCurrent15M->MillSecond = 0;		
			
			pAddstats->pNext = NULL;
			pAddstats->iUsed15Min = TRUE;
			pAddstats->firstFlag15M = FALSE;
			}

		if(actcode==STATS_24HOUR_SET)
			{			
			if (NULL == HisStatsChainCreate(bucketNum,&(pAddstats->pstats24HourHead), &(pAddstats->pstats24HourTail)))
				{
				/*如果在此步申请内存失败,则在返回之
				前释放之前所有申请的内存*/
				HisStatsChainDelete(&(pAddstats->pstats15MinHead), &(pAddstats->pstats15MinTail));
				VOS_Free((VOID *) pAddstats->pstatsDataPre);
				VOS_Free((VOID *) pAddstats);
				return (STATS_POINT_NULL_ERR);
				}			
	

			pAddstats->pCurrent24H = pAddstats->pstats24HourHead;
			/*added by wutw at 19 October*/
			pAddstats->pCurrent24H->currentTick= VOS_GetTick();
			memset( &dateTime, 0, sizeof(sysDateAndTime_t));
			eventGetCurTime( &dateTime );	
			pAddstats->pCurrent24H->year= dateTime.year;
			pAddstats->pCurrent24H->month = dateTime.month;
			pAddstats->pCurrent24H->day =  dateTime.day;
			pAddstats->pCurrent24H->hour = (unsigned short)dateTime.hour;
			pAddstats->pCurrent24H->minute = dateTime.minute;
			pAddstats->pCurrent24H->second = dateTime.second;
			pAddstats->pCurrent24H->MillSecond = 0;	
			
			pAddstats->pNext = NULL;
			pAddstats->iUsed24Hour = TRUE;
			pAddstats->firstFlag24H = FALSE;
			}
		
		if (gpONUHisStatsHead == NULL)
			{
				gpONUHisStatsHead = pAddstats;
				gpONUHisStatsHead->pPre = gpONUHisStatsHead;
				gpONUHisStatsHead->pNext = NULL;
				gpONUHisStatsTail = gpONUHisStatsHead;
			}
		else
			{
				gpONUHisStatsTail->pNext = pAddstats;
				pAddstats->pPre = gpONUHisStatsTail;
				gpONUHisStatsTail = pAddstats;
				gpONUHisStatsTail->pNext = NULL;
			}
		pAddstats = NULL;
		ONUHistoryStatsCounter++;

		return(STATS_OK);
	}
	

}


/**************************************************
* HisStatsMsgTimeOutSend
*
*
*
*
*
****************************/
short int HisStatsMsgTimeOutSend(void)
{
	historyStatsMsg *pstatsRecMsg; 
	long		lRes = 0;
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};
	if(FALSE == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		return STATS_OK;
	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return STATS_OK;

	if( VOS_QueNum( gMQidsStats) > 5 )
	{
		/*sys_console_printf("  History statistics queue is too busy");*/
		return STATS_ERR;
	}
	pstatsRecMsg = (historyStatsMsg *)VOS_Malloc(sizeof(historyStatsMsg), MODULE_STATSTICS);
	if (NULL == pstatsRecMsg)
		return STATS_ERR;
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));	
	pstatsRecMsg->statsType = STATS_TIMER_TIMEOUT;	
	aulMsg[3] = (ULONG)(pstatsRecMsg);

	lRes = VOS_QueSend(gMQidsStats, aulMsg, NO_WAIT,MSG_PRI_NORMAL);

	if (VOS_ERROR == lRes)
	{
		ASSERT( 0 );
		VOS_Free((VOID *) pstatsRecMsg);
		pstatsRecMsg = NULL;
		return STATS_ERR;
	}
	return STATS_OK;
	
}


STATUS	HisStatsEthRawGet( void )
{
	statistic_PON_OID	*pstatsEth = gpEthHisStatsHead ;
	/*short int eth = 0 ;*/
	/*ULONG   ulRetDate = 0;
	ULONG   ulRetTime = 0;
	ULONG   ulRetMillSec = 0;*/
	/*rtStatsEntry64	*pRTBuf = NULL;*/
	statistc_HISTORY_CHAIN temp15M;
	statistc_HISTORY_CHAIN temp24M;
	sysDateAndTime_t dateTime;
	PON_StatsData_Table64	statsCurrent;

	ETH_PORT_STATS_S stPortStats;
	ETH_PORT_STATS_S *pRTBuf = &stPortStats;
	ULONG ulIfx;

	while(pstatsEth != NULL)/* 所有ETH一级链表刷新数据 */
	{
#if 0
		eth = pstatsEth->ethIdx-1;
		/*1. 获取当前统计数据*/
		memset(&statsCurrent, 0, sizeof(PON_StatsData_Table64));
		memset(&temp15M, 0, sizeof(statistc_HISTORY_CHAIN));
		pRTBuf = ((rtStatsEntry64*)&gStatsRealTimeEth[eth]);

		temp15M.hisStats.hisMonDropEvents = pRTBuf->rtStatsDropEvents;
		temp15M.hisStats.hisMonOctets = pRTBuf->rtStatsOctets;
		temp15M.hisStats.hisMonPkts = pRTBuf->rtStatsPkts;
		temp15M.hisStats.hisMonBroadcastPkts = pRTBuf->rtStatsBroadcastPkts;
		temp15M.hisStats.hisMonMulticastPkts = pRTBuf->rtStatsMulticastPkts;
		temp15M.hisStats.hisMonCRCAlignErrors = pRTBuf->rtStatsCRCAlignErrors;
		temp15M.hisStats.hisMonUndersizePkts= pRTBuf->rtStatsUndersizePkts;
		temp15M.hisStats.hisMonOversizePkts = pRTBuf->rtStatsOversizePkts;
		temp15M.hisStats.hisMonFragments = pRTBuf->rtStatsFragments;
		temp15M.hisStats.hisMonJabbers = pRTBuf->rtStatsJabbers;
		temp15M.hisStats.hisMonCollisions = pRTBuf->rtStatsCollisions;

		/*2. 将当前数据减去上一个统计数据的值,得到15Min 的统计数据*/			
		statsCurrent.hisMonDropEvents = pRTBuf->rtStatsDropEvents - pstatsEth->pstatsDataPre->hisMonDropEvents;
		statsCurrent.hisMonOctets = pRTBuf->rtStatsOctets - pstatsEth->pstatsDataPre->hisMonOctets;
		statsCurrent.hisMonPkts = pRTBuf->rtStatsPkts - pstatsEth->pstatsDataPre->hisMonPkts;
		statsCurrent.hisMonBroadcastPkts = pRTBuf->rtStatsBroadcastPkts - pstatsEth->pstatsDataPre->hisMonBroadcastPkts;
		statsCurrent.hisMonMulticastPkts = pRTBuf->rtStatsMulticastPkts - pstatsEth->pstatsDataPre->hisMonMulticastPkts;
		statsCurrent.hisMonCRCAlignErrors = pRTBuf->rtStatsCRCAlignErrors - pstatsEth->pstatsDataPre->hisMonCRCAlignErrors;
		statsCurrent.hisMonUndersizePkts= pRTBuf->rtStatsUndersizePkts - pstatsEth->pstatsDataPre->hisMonUndersizePkts;
		statsCurrent.hisMonOversizePkts = pRTBuf->rtStatsOversizePkts - pstatsEth->pstatsDataPre->hisMonOversizePkts;
		statsCurrent.hisMonFragments = pRTBuf->rtStatsFragments - pstatsEth->pstatsDataPre->hisMonFragments;
		statsCurrent.hisMonJabbers = pRTBuf->rtStatsJabbers - pstatsEth->pstatsDataPre->hisMonJabbers;
		statsCurrent.hisMonCollisions = pRTBuf->rtStatsCollisions - pstatsEth->pstatsDataPre->hisMonCollisions;
#else
		memset(&statsCurrent, 0, sizeof(PON_StatsData_Table64));
		memset(&temp15M, 0, sizeof(statistc_HISTORY_CHAIN));

		ulIfx = userSlot_userPort_2_Ifindex( pstatsEth->slotIdx, pstatsEth->ethIdx );
		if( ulIfx == 0xffffffff )
			break;

		VOS_MemZero( pRTBuf, sizeof(ETH_PORT_STATS_S) );
		if( Get_ETH_PortStatInfor( ulIfx, pRTBuf ) == VOS_ERROR )
		{
			sys_console_printf( "\r\n ETH HIS STATISIC:Get_ETH_PortStatInfor error" );
			break;
		}

		temp15M.hisStats.hisMonDropEvents = *(unsigned long long*)&pRTBuf->ulAllEtherStatsDropEvents;
		temp15M.hisStats.hisMonOctets = *(unsigned long long*)&pRTBuf->ulAllIfInOctets;
		temp15M.hisStats.hisMonPkts = *(unsigned long long*)&pRTBuf->ulAllPacketSumRx;
		temp15M.hisStats.hisMonBroadcastPkts = *(unsigned long long*)&pRTBuf->ulAllEtherStatsBroadcastPkts;
		temp15M.hisStats.hisMonMulticastPkts = *(unsigned long long*)&pRTBuf->ulAllEtherStatsMulticastPkts;
		temp15M.hisStats.hisMonCRCAlignErrors = *(unsigned long long*)&pRTBuf->ulAllEtherStatsCRCAlignErrors;
		temp15M.hisStats.hisMonUndersizePkts= *(unsigned long long*)&pRTBuf->ulAllEtherStatsUndersizePkts;
		temp15M.hisStats.hisMonOversizePkts = *(unsigned long long*)&pRTBuf->ulAllEtherStatsOversizePkts;
		temp15M.hisStats.hisMonFragments = *(unsigned long long*)&pRTBuf->ulAllEtherStatsFragments;
		temp15M.hisStats.hisMonJabbers = *(unsigned long long*)&pRTBuf->ulAllEtherStatsJabbers;
		temp15M.hisStats.hisMonCollisions = *(unsigned long long*)&pRTBuf->ulAllEtherStatsCollisions;

		/*2. 将当前数据减去上一个统计数据的值,得到15Min 的统计数据*/			
		statsCurrent.hisMonDropEvents = temp15M.hisStats.hisMonDropEvents - pstatsEth->pstatsDataPre->hisMonDropEvents;
		statsCurrent.hisMonOctets = temp15M.hisStats.hisMonOctets - pstatsEth->pstatsDataPre->hisMonOctets;
		statsCurrent.hisMonPkts = temp15M.hisStats.hisMonPkts - pstatsEth->pstatsDataPre->hisMonPkts;
		statsCurrent.hisMonBroadcastPkts = temp15M.hisStats.hisMonBroadcastPkts - pstatsEth->pstatsDataPre->hisMonBroadcastPkts;
		statsCurrent.hisMonMulticastPkts = temp15M.hisStats.hisMonMulticastPkts - pstatsEth->pstatsDataPre->hisMonMulticastPkts;
		statsCurrent.hisMonCRCAlignErrors = temp15M.hisStats.hisMonCRCAlignErrors - pstatsEth->pstatsDataPre->hisMonCRCAlignErrors;
		statsCurrent.hisMonUndersizePkts= temp15M.hisStats.hisMonUndersizePkts - pstatsEth->pstatsDataPre->hisMonUndersizePkts;
		statsCurrent.hisMonOversizePkts = temp15M.hisStats.hisMonOversizePkts - pstatsEth->pstatsDataPre->hisMonOversizePkts;
		statsCurrent.hisMonFragments = temp15M.hisStats.hisMonFragments - pstatsEth->pstatsDataPre->hisMonFragments;
		statsCurrent.hisMonJabbers = temp15M.hisStats.hisMonJabbers - pstatsEth->pstatsDataPre->hisMonJabbers;
		statsCurrent.hisMonCollisions = temp15M.hisStats.hisMonCollisions - pstatsEth->pstatsDataPre->hisMonCollisions;

#endif
		if (pstatsEth->iUsed15Min)
		{
			if (!pstatsEth->firstFlag15M)
			{
				/*如果是第一次进行历史统计，则执行该条件下的步骤*/
				pstatsEth->firstFlag15M = TRUE;

				/*added by wutw 7.18*/
				pstatsEth->pCurrent15M = pstatsEth->pstats15MinHead;
				
				/*保留数据统计*/
				memcpy(pstatsEth->pstatsDataPre, &(temp15M.hisStats), sizeof(PON_StatsData_Table64));
				/*第一次采集的历史纪录为0*/
				/*memcpy(&(pstatsEth->pCurrent15M->hisStats), &(statsCurrent), sizeof(PON_StatsData_Table64 ));*/
				memset(&(pstatsEth->pCurrent15M->hisStats), 0, sizeof(PON_StatsData_Table64 ));
			}
			else 
			{
				/*如果是当前纪录到的统计数据保存块等于Head指针指向的数据区，并且不为第一次数据统计，则，Head指向下一个数据区*/
				pstatsEth->pCurrent15M = pstatsEth->pCurrent15M->pNext;

				if (pstatsEth->pstats15MinHead == pstatsEth->pCurrent15M)
				{
				pstatsEth->pstats15MinHead = pstatsEth->pstats15MinHead->pNext ;
				/*begin:
				added by wangxiaoyu 2008-02-26
				*/
				pstatsEth->pstats15MinTail = pstatsEth->pstats15MinTail->pNext;
				/*end*/
				memset(&(pstatsEth->pCurrent15M->hisStats), 0, sizeof(PON_StatsData_Table64));
				}

				/*pstatsEth->pCurrent15M->currentTick= VOS_GetTick();*/
				/*保留数据统计*/
				memcpy(pstatsEth->pstatsDataPre, &(temp15M.hisStats), sizeof(PON_StatsData_Table64));
				memcpy(&(pstatsEth->pCurrent15M->hisStats), &(statsCurrent), sizeof(PON_StatsData_Table64 ));
			}
			/*added by wutw at 15 October*/
			pstatsEth->pCurrent15M->currentTick= VOS_GetTick();

			/*added by wangxy at 2007-04-13*/
			pstatsEth->pCurrent15M->hisStats.hisMonIntervalStart = pstatsEth->pCurrent15M->currentTick;
			
			memset( &dateTime, 0, sizeof(sysDateAndTime_t));
			eventGetCurTime( &dateTime );	
			pstatsEth->pCurrent15M->year= dateTime.year;
			pstatsEth->pCurrent15M->month = dateTime.month;
			pstatsEth->pCurrent15M->day =  dateTime.day;
			pstatsEth->pCurrent15M->hour = (unsigned short)dateTime.hour;
			pstatsEth->pCurrent15M->minute = dateTime.minute;
			pstatsEth->pCurrent15M->second = dateTime.second;
			pstatsEth->pCurrent15M->MillSecond = 0;
			
			pstatsEth->pCurrent15M->bstatus = TRUE;	
		}/*if (pstatsEth->iUsed15Min)*/

		/*============================================*/
		if (pstatsEth->iUsed24Hour)
		{/*以下为纪录24小时的统计数据*/

			if (!pstatsEth->firstFlag24H)
			{
				/*使用第一次进行统计*/
				pstatsEth->firstFlag24H = TRUE;
				pstatsEth->pCurrent24H= pstatsEth->pstats24HourHead;
			}

			if (pstatsEth->samplingCount24H >= STATS_SAMPLING_COUNT )
			{
				/*如果当前的纪录次数达到24个小时，则将历史统计数据
				保存于下一个数据区*/					
				pstatsEth->pCurrent24H = pstatsEth->pCurrent24H->pNext;
				pstatsEth->samplingCount24H = 0 ;
				/*如果当前要保存的数据区为head,则该head下移*/
				if (pstatsEth->pCurrent24H == pstatsEth->pstats24HourHead)
				{
					pstatsEth->pstats24HourHead = pstatsEth->pstats24HourHead->pNext ;
					/*begin:
					added by wangxiaoyu 2008-02-26
					*/
					pstatsEth->pstats24HourTail = pstatsEth->pstats24HourTail->pNext;
					/*end*/
					memset(&(pstatsEth->pCurrent24H->hisStats), 0, sizeof(PON_StatsData_Table64));
				}
			}

			memset(&(temp24M), 0, sizeof(statistc_HISTORY_CHAIN));
			memcpy(&(temp24M.hisStats), &(pstatsEth->pCurrent24H->hisStats), sizeof(PON_StatsData_Table64));
			/*使用pstatspon-> pCurrent24H.*/	

			pstatsEth->pCurrent24H->currentTick = VOS_GetTick();
			memset( &dateTime, 0, sizeof(sysDateAndTime_t));
			eventGetCurTime( &dateTime );	
			
			pstatsEth->pCurrent24H->year = dateTime.year;
			pstatsEth->pCurrent24H->month = dateTime.month;
			pstatsEth->pCurrent24H->day = dateTime.day;
			pstatsEth->pCurrent24H->hour = dateTime.hour;
			pstatsEth->pCurrent24H->minute = dateTime.minute;
			pstatsEth->pCurrent24H->second = dateTime.second;
			pstatsEth->pCurrent24H->MillSecond = 0;
			
		

			temp24M.hisStats.hisMonDropEvents += statsCurrent.hisMonDropEvents;
			/*获取接收到的总的字节数,包裹正确和错误帧的字节*/
			temp24M.hisStats.hisMonPkts += statsCurrent.hisMonPkts;
			/*获取收到的总的数据报数*/	
			temp24M.hisStats.hisMonOctets += statsCurrent.hisMonOctets;
			/*获取接收到的广播数据*/
			temp24M.hisStats.hisMonBroadcastPkts += statsCurrent.hisMonBroadcastPkts;
			/*获取接收到的组播数据*/
			temp24M.hisStats.hisMonMulticastPkts += statsCurrent.hisMonMulticastPkts;
			/*获取接收到CRCAlignErrors帧数*/
			temp24M.hisStats.hisMonCRCAlignErrors += statsCurrent.hisMonCRCAlignErrors;
			temp24M.hisStats.hisMonUndersizePkts += statsCurrent.hisMonUndersizePkts;
			temp24M.hisStats.hisMonFragments += statsCurrent.hisMonFragments;
			temp24M.hisStats.hisMonJabbers += statsCurrent.hisMonJabbers;
			/*获取接收到长度大于标准长度的帧数*/
			temp24M.hisStats.hisMonOversizePkts += statsCurrent.hisMonOversizePkts;
			/*获取conllison*/
			temp24M.hisStats.hisMonCollisions += statsCurrent.hisMonCollisions;
			memcpy(&(pstatsEth->pCurrent24H->hisStats), &(temp24M.hisStats), sizeof(PON_StatsData_Table64));

			/*added by wangxy at 2007-04-13*/
			pstatsEth->pCurrent24H->hisStats.hisMonIntervalStart = pstatsEth->pCurrent24H->currentTick;
			
			pstatsEth->samplingCount24H++;
			pstatsEth->pCurrent24H->bstatus = TRUE;	
		}				
		pstatsEth = pstatsEth->pNext;
	}/*while(pstatsEth != NULL)*/

	return (STATS_OK);
}



 /****************************************************
* HisStatsRawGet
* 描述: 获取pon对象的历史统计
*		
*							
*								
*							
*		
*
*	输入: 无
*
*	输出: 无
*
*	返回: 无
******************************************************/
/*short int HisStatsRawGet( const short int						 olt_id, 
							const short int						 collector_id, 
							const PON_raw_statistics_t			 raw_statistics_type, 
							const short int						 statistics_parameter,
							void									*statistics_data,
							PON_timestamp_t						*timestamp )
{
	short int iRes = STATS_OK;
	iRes = PAS_get_raw_statistics(olt_id,  collector_id, raw_statistics_type, statistics_parameter,
							statistics_data, timestamp );
	return iRes;
 }*/

	
 /****************************************************
* HisStatsPonRawGet
* 描述: 获取pon对象的历史统计
*		
*							
*								
*							
*		
*
*	输入: 无
*
*	输出: 无
*
*	返回: 无
******************************************************/
STATUS HisStatsPonRawGet(void)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	short int ponId ;
	/*rtStatsEntry64	*pRTBuf = NULL;*/
	statistc_HISTORY_CHAIN temp15M;
	statistc_HISTORY_CHAIN temp24M;
	sysDateAndTime_t dateTime;
	PON_StatsData_Table64 statsCurrent;

	ETH_PORT_STATS_S stPortStats;
	ETH_PORT_STATS_S *pRTBuf = &stPortStats;
	ULONG ulIfx;

	while(pstatsPon != NULL)
	{
		ponId = pstatsPon->ponIdx;
#if 0
		if (gStatsPonOn[ponId] == STATS_DEVICE_OFF)
		{
			pstatsPon = pstatsPon->pNext;
			continue;
		}
			/*1. 获取当前统计数据*/
			memset(&statsCurrent, 0, sizeof(PON_StatsData_Table64));
			memset(&temp15M, 0, sizeof(statistc_HISTORY_CHAIN));
			pRTBuf = ((rtStatsEntry64*)pgStatsRealTimeOlt[ponId]);
			
			temp15M.hisStats.hisMonDropEvents = pRTBuf->rtStatsDropEvents;
			temp15M.hisStats.hisMonOctets = pRTBuf->rtStatsOctets;
			temp15M.hisStats.hisMonPkts = pRTBuf->rtStatsPkts;
			temp15M.hisStats.hisMonBroadcastPkts = pRTBuf->rtStatsBroadcastPkts;
			temp15M.hisStats.hisMonMulticastPkts = pRTBuf->rtStatsMulticastPkts;
			temp15M.hisStats.hisMonCRCAlignErrors = pRTBuf->rtStatsCRCAlignErrors;
			temp15M.hisStats.hisMonUndersizePkts= pRTBuf->rtStatsUndersizePkts;
			temp15M.hisStats.hisMonOversizePkts = pRTBuf->rtStatsOversizePkts;
			temp15M.hisStats.hisMonFragments = pRTBuf->rtStatsFragments;
			temp15M.hisStats.hisMonJabbers = pRTBuf->rtStatsJabbers;
			temp15M.hisStats.hisMonCollisions = pRTBuf->rtStatsCollisions;

			/*2. 将当前数据减去上一个统计数据的值,得到15Min 的统计数据*/			
			statsCurrent.hisMonDropEvents = pRTBuf->rtStatsDropEvents - pstatsPon->pstatsDataPre->hisMonDropEvents;
			statsCurrent.hisMonOctets = pRTBuf->rtStatsOctets - pstatsPon->pstatsDataPre->hisMonOctets;
			statsCurrent.hisMonPkts = pRTBuf->rtStatsPkts - pstatsPon->pstatsDataPre->hisMonPkts;
			statsCurrent.hisMonBroadcastPkts = pRTBuf->rtStatsBroadcastPkts - pstatsPon->pstatsDataPre->hisMonBroadcastPkts;
			statsCurrent.hisMonMulticastPkts = pRTBuf->rtStatsMulticastPkts - pstatsPon->pstatsDataPre->hisMonMulticastPkts;
			statsCurrent.hisMonCRCAlignErrors = pRTBuf->rtStatsCRCAlignErrors - pstatsPon->pstatsDataPre->hisMonCRCAlignErrors;
			statsCurrent.hisMonUndersizePkts= pRTBuf->rtStatsUndersizePkts - pstatsPon->pstatsDataPre->hisMonUndersizePkts;
			statsCurrent.hisMonOversizePkts = pRTBuf->rtStatsOversizePkts - pstatsPon->pstatsDataPre->hisMonOversizePkts;
			statsCurrent.hisMonFragments = pRTBuf->rtStatsFragments - pstatsPon->pstatsDataPre->hisMonFragments;
			statsCurrent.hisMonJabbers = pRTBuf->rtStatsJabbers - pstatsPon->pstatsDataPre->hisMonJabbers;
			statsCurrent.hisMonCollisions = pRTBuf->rtStatsCollisions - pstatsPon->pstatsDataPre->hisMonCollisions;
#else
		if( !OLT_ISVALID(ponId) )
		{
			pstatsPon = pstatsPon->pNext;
			continue;
		}
		/*if( !PonPortIsWorking(ponId) )
		{
			pstatsPon = pstatsPon->pNext;
			continue;
		}*/
	
		ulIfx = userSlot_userPort_2_Ifindex( GetCardIdxByPonChip(ponId), GetPonPortByPonChip(ponId) );
		if( ulIfx == 0xffffffff )
			break;

		VOS_MemZero( pRTBuf, sizeof(ETH_PORT_STATS_S) );
		if( Get_ETH_PortStatInfor( ulIfx, pRTBuf ) == VOS_ERROR )
		{
			sys_console_printf( "\r\n PON HIS STATISIC:Get_ETH_PortStatInfor error" );
			continue;
		}
		VOS_MemZero(&statsCurrent, sizeof(PON_StatsData_Table64));
		VOS_MemZero(&temp15M, sizeof(statistc_HISTORY_CHAIN));

		temp15M.hisStats.hisMonDropEvents = *(unsigned long long*)&pRTBuf->ulAllEtherStatsDropEvents;
		temp15M.hisStats.hisMonOctets = *(unsigned long long*)&pRTBuf->ulAllIfInOctets;
		temp15M.hisStats.hisMonPkts = *(unsigned long long*)&pRTBuf->ulAllPacketSumRx;
		temp15M.hisStats.hisMonBroadcastPkts = *(unsigned long long*)&pRTBuf->ulAllEtherStatsBroadcastPkts;
		temp15M.hisStats.hisMonMulticastPkts = *(unsigned long long*)&pRTBuf->ulAllEtherStatsMulticastPkts;
		temp15M.hisStats.hisMonCRCAlignErrors = *(unsigned long long*)&pRTBuf->ulAllEtherStatsCRCAlignErrors;
		temp15M.hisStats.hisMonUndersizePkts= *(unsigned long long*)&pRTBuf->ulAllEtherStatsUndersizePkts;
		temp15M.hisStats.hisMonOversizePkts = *(unsigned long long*)&pRTBuf->ulAllEtherStatsOversizePkts;
		temp15M.hisStats.hisMonFragments = *(unsigned long long*)&pRTBuf->ulAllEtherStatsFragments;
		temp15M.hisStats.hisMonJabbers = *(unsigned long long*)&pRTBuf->ulAllEtherStatsJabbers;
		temp15M.hisStats.hisMonCollisions = *(unsigned long long*)&pRTBuf->ulAllEtherStatsCollisions;

		/*2. 将当前数据减去上一个统计数据的值,得到15Min 的统计数据*/			
		statsCurrent.hisMonDropEvents = temp15M.hisStats.hisMonDropEvents - pstatsPon->pstatsDataPre->hisMonDropEvents;
		statsCurrent.hisMonOctets = temp15M.hisStats.hisMonOctets - pstatsPon->pstatsDataPre->hisMonOctets;
		statsCurrent.hisMonPkts = temp15M.hisStats.hisMonPkts - pstatsPon->pstatsDataPre->hisMonPkts;
		statsCurrent.hisMonBroadcastPkts = temp15M.hisStats.hisMonBroadcastPkts - pstatsPon->pstatsDataPre->hisMonBroadcastPkts;
		statsCurrent.hisMonMulticastPkts = temp15M.hisStats.hisMonMulticastPkts - pstatsPon->pstatsDataPre->hisMonMulticastPkts;
		statsCurrent.hisMonCRCAlignErrors = temp15M.hisStats.hisMonCRCAlignErrors - pstatsPon->pstatsDataPre->hisMonCRCAlignErrors;
		statsCurrent.hisMonUndersizePkts= temp15M.hisStats.hisMonUndersizePkts - pstatsPon->pstatsDataPre->hisMonUndersizePkts;
		statsCurrent.hisMonOversizePkts = temp15M.hisStats.hisMonOversizePkts - pstatsPon->pstatsDataPre->hisMonOversizePkts;
		statsCurrent.hisMonFragments = temp15M.hisStats.hisMonFragments - pstatsPon->pstatsDataPre->hisMonFragments;
		statsCurrent.hisMonJabbers = temp15M.hisStats.hisMonJabbers - pstatsPon->pstatsDataPre->hisMonJabbers;
		statsCurrent.hisMonCollisions = temp15M.hisStats.hisMonCollisions - pstatsPon->pstatsDataPre->hisMonCollisions;

#endif
			
		if (pstatsPon->iUsed15Min)
		{
			if (!pstatsPon->firstFlag15M)
			{/*如果是第一次进行历史统计，则执行该条件下的步骤*/
				pstatsPon->firstFlag15M = TRUE;

				/*added by wutw 7.18*/
				pstatsPon->pCurrent15M = pstatsPon->pstats15MinHead;
							
				/*保留数据统计*/
				memcpy(pstatsPon->pstatsDataPre, &(temp15M.hisStats), sizeof(PON_StatsData_Table64));
				/*第一次采集的历史纪录为0*/
				/*memcpy(&(pstatsPon->pCurrent15M->hisStats), &(statsCurrent), sizeof(PON_StatsData_Table64 ));*/
				memset(&(pstatsPon->pCurrent15M->hisStats), 0, sizeof(PON_StatsData_Table64 ));
			}
			else 
			{/*如果是当前纪录到的统计数据保存块等于Head指针指向的
				数据区，并且不为第一次数据统计，则，Head指向下一个数据区*/
				pstatsPon->pCurrent15M = pstatsPon->pCurrent15M->pNext;
				if (pstatsPon->pstats15MinHead == pstatsPon->pCurrent15M)
				{
					pstatsPon->pstats15MinHead = pstatsPon->pstats15MinHead->pNext ;
					/*added by wangxiaoyu 2008-02-25*/
					pstatsPon->pstats15MinTail = pstatsPon->pstats15MinTail->pNext;
					/*end*/
					memset(&(pstatsPon->pCurrent15M->hisStats), 0, sizeof(PON_StatsData_Table64));
				}
				/*pstatsPon->pCurrent15M->currentTick= VOS_GetTick();*/
				/*保留数据统计*/
				memcpy(pstatsPon->pstatsDataPre, &(temp15M.hisStats), sizeof(PON_StatsData_Table64));
				memcpy(&(pstatsPon->pCurrent15M->hisStats), &(statsCurrent), sizeof(PON_StatsData_Table64 ));
			}
			/*added by wutw at 15 October*/
			pstatsPon->pCurrent15M->currentTick= VOS_GetTick();
			pstatsPon->pCurrent15M->hisStats.hisMonIntervalStart = pstatsPon->pCurrent15M->currentTick;
			
			memset( &dateTime, 0, sizeof(sysDateAndTime_t));
			eventGetCurTime( &dateTime );	
			pstatsPon->pCurrent15M->year= dateTime.year;
			pstatsPon->pCurrent15M->month = dateTime.month;
			pstatsPon->pCurrent15M->day =  dateTime.day;
			pstatsPon->pCurrent15M->hour = (unsigned short)dateTime.hour;
			pstatsPon->pCurrent15M->minute = dateTime.minute;
			pstatsPon->pCurrent15M->second = dateTime.second;
			pstatsPon->pCurrent15M->MillSecond = 0;
			
			pstatsPon->pCurrent15M->bstatus = TRUE;	
		}/*if (pstatsPon->iUsed15Min)*/

		/*============================================*/
		if (pstatsPon->iUsed24Hour)
		{/*以下为纪录24小时的统计数据*/
			if (!pstatsPon->firstFlag24H)
			{
				/*使用第一次进行统计*/
				pstatsPon->firstFlag24H = TRUE;
				pstatsPon->pCurrent24H= pstatsPon->pstats24HourHead;
			}

			if (pstatsPon->samplingCount24H >= STATS_SAMPLING_COUNT )
			{
				/*如果当前的纪录次数达到24个小时，则将历史统计数据
				保存于下一个数据区*/					
				pstatsPon->pCurrent24H = pstatsPon->pCurrent24H->pNext;
				pstatsPon->samplingCount24H = 0 ;
				/*如果当前要保存的数据区为head,则该head下移*/
				if (pstatsPon->pCurrent24H == pstatsPon->pstats24HourHead)
				{
					pstatsPon->pstats24HourHead = pstatsPon->pstats24HourHead->pNext ;
					/*added by wangxiaoyu 2008-02-26*/
					pstatsPon->pstats24HourTail = pstatsPon->pstats24HourTail->pNext;
					/*end*/
					memset(&(pstatsPon->pCurrent24H->hisStats), 0, sizeof(PON_StatsData_Table64));
				}
			}
			memset(&(temp24M), 0, sizeof(statistc_HISTORY_CHAIN));
			memcpy(&(temp24M.hisStats), &(pstatsPon->pCurrent24H->hisStats), sizeof(PON_StatsData_Table64));
			/*使用pstatspon-> pCurrent24H.*/	
			VOS_MemSet( &dateTime, 0, sizeof(sysDateAndTime_t) );
			eventGetCurTime( &dateTime );
				
			pstatsPon->pCurrent24H->year = dateTime.year;
			pstatsPon->pCurrent24H->month = dateTime.month;
			pstatsPon->pCurrent24H->day = dateTime.day;
			pstatsPon->pCurrent24H->hour = dateTime.hour;
			pstatsPon->pCurrent24H->minute = dateTime.minute;
			pstatsPon->pCurrent24H->second = dateTime.second;
			pstatsPon->pCurrent24H->MillSecond = 0;
			pstatsPon->pCurrent24H->currentTick = VOS_GetTick();

			temp24M.hisStats.hisMonDropEvents += statsCurrent.hisMonDropEvents;				
			/*获取接收到的总的字节数,包裹正确和错误帧的字节*/
			temp24M.hisStats.hisMonPkts += statsCurrent.hisMonPkts;
			/*获取收到的总的数据报数*/	
			temp24M.hisStats.hisMonOctets += statsCurrent.hisMonOctets;
			/*获取接收到的广播数据*/
			temp24M.hisStats.hisMonBroadcastPkts += statsCurrent.hisMonBroadcastPkts;
			/*获取接收到的组播数据*/
			temp24M.hisStats.hisMonMulticastPkts += pstatsPon->pCurrent15M->hisStats.hisMonMulticastPkts;/* 莫名其妙? */
			/*获取接收到CRCAlignErrors帧数*/
			temp24M.hisStats.hisMonCRCAlignErrors += statsCurrent.hisMonCRCAlignErrors;
			temp24M.hisStats.hisMonUndersizePkts += statsCurrent.hisMonUndersizePkts;
			temp24M.hisStats.hisMonFragments += statsCurrent.hisMonFragments;
			temp24M.hisStats.hisMonJabbers += statsCurrent.hisMonJabbers;
			/*获取接收到长度大于标准长度的帧数*/
			temp24M.hisStats.hisMonOversizePkts += statsCurrent.hisMonOversizePkts;
			/*获取conllison*/
			temp24M.hisStats.hisMonCollisions += statsCurrent.hisMonCollisions;
			
			memcpy(&(pstatsPon->pCurrent24H->hisStats), &(temp24M.hisStats), sizeof(PON_StatsData_Table64));
			pstatsPon->pCurrent24H->hisStats.hisMonIntervalStart = pstatsPon->pCurrent24H->currentTick;
			
			pstatsPon->samplingCount24H++;
			pstatsPon->pCurrent24H->bstatus = TRUE;	
		}		
		
		pstatsPon = pstatsPon->pNext;
	}/*while(pstatsPon != NULL)*/
	
	return (STATS_OK);
}


 /****************************************************
* HisStatsOnuRawGet
* 描述: 获取onu对象的历史统计
******************************************************/
STATUS HisStatsOnuRawGet(void)
{
#if 0
	statistic_PON_OID	*pstatsOnu = gpONUHisStatsHead ;
	short int ponId ;
	short int onuId;
	short int llid = 0;
	short int iRes = STATS_OK;
	/*ULONG   ulRetDate = 0;
	ULONG   ulRetTime = 0;
	ULONG   ulRetMillSec = 0;*/
	rtStatsEntry *pTempRtStats = NULL;
	statistc_HISTORY_CHAIN temp15M;
	statistc_HISTORY_CHAIN temp24M;
	sysDateAndTime_t dateTime;
	PON_StatsData_Table64 	statsCurrent;

	/*==========onu==========*/
	while(pstatsOnu != NULL)
	{
		ponId = pstatsOnu->ponIdx ;
		onuId = pstatsOnu->onuIdx ;
		llid = GetLlidByOnuIdx(ponId, onuId);
		if ( STAT_INVALID_LLID == llid )
		{
			pstatsOnu = pstatsOnu->pNext;
			continue;
		}
		/*1. 获取当前统计数据*/
		memset(&statsCurrent, 0, sizeof(PON_StatsData_Table64));
		memset(&temp15M, 0, sizeof(statistc_HISTORY_CHAIN));
		
	
		pTempRtStats = (rtStatsEntry *)pgStatsRealTimeOnu[ponId][onuId];
		temp15M.hisStats.hisMonDropEvents = pTempRtStats->rtStatsDropEvents;
		temp15M.hisStats.hisMonOctets = pTempRtStats->rtStatsOctets;
		temp15M.hisStats.hisMonPkts = pTempRtStats->rtStatsPkts;
		temp15M.hisStats.hisMonBroadcastPkts = pTempRtStats->rtStatsBroadcastPkts;
		temp15M.hisStats.hisMonMulticastPkts = pTempRtStats->rtStatsMulticastPkts;
		temp15M.hisStats.hisMonCRCAlignErrors = pTempRtStats->rtStatsCRCAlignErrors;
		temp15M.hisStats.hisMonUndersizePkts= pTempRtStats->rtStatsUndersizePkts;
		temp15M.hisStats.hisMonOversizePkts = pTempRtStats->rtStatsOversizePkts;
		temp15M.hisStats.hisMonFragments = pTempRtStats->rtStatsFragments;
		temp15M.hisStats.hisMonJabbers = pTempRtStats->rtStatsJabbers;
		temp15M.hisStats.hisMonCollisions = pTempRtStats->rtStatsCollisions;
			/*2. 将当前数据减去上一个统计数据的值,得到15Min 的统计数据*/			
		statsCurrent.hisMonDropEvents = pTempRtStats->rtStatsDropEvents - pstatsOnu->pstatsDataPre->hisMonDropEvents;
		statsCurrent.hisMonOctets = pTempRtStats->rtStatsOctets - pstatsOnu->pstatsDataPre->hisMonOctets;
		statsCurrent.hisMonPkts = pTempRtStats->rtStatsPkts - pstatsOnu->pstatsDataPre->hisMonPkts;
		statsCurrent.hisMonBroadcastPkts = pTempRtStats->rtStatsBroadcastPkts - pstatsOnu->pstatsDataPre->hisMonBroadcastPkts;
		statsCurrent.hisMonMulticastPkts = pTempRtStats->rtStatsMulticastPkts - pstatsOnu->pstatsDataPre->hisMonMulticastPkts;
		statsCurrent.hisMonCRCAlignErrors = pTempRtStats->rtStatsCRCAlignErrors - pstatsOnu->pstatsDataPre->hisMonCRCAlignErrors;
		statsCurrent.hisMonUndersizePkts= pTempRtStats->rtStatsUndersizePkts - pstatsOnu->pstatsDataPre->hisMonUndersizePkts;
		statsCurrent.hisMonOversizePkts = pTempRtStats->rtStatsOversizePkts - pstatsOnu->pstatsDataPre->hisMonOversizePkts;
		statsCurrent.hisMonFragments = pTempRtStats->rtStatsFragments - pstatsOnu->pstatsDataPre->hisMonFragments;
		statsCurrent.hisMonJabbers = pTempRtStats->rtStatsJabbers - pstatsOnu->pstatsDataPre->hisMonJabbers;
		statsCurrent.hisMonCollisions = pTempRtStats->rtStatsCollisions - pstatsOnu->pstatsDataPre->hisMonCollisions;
		
#ifdef __STAT_DEBUG
		if( gStatisticDebugFlag == 1 )
		{
			char szText[64]="";
			sys_console_printf("   ---------------------------------\r\n" );
			sys_console_printf("  statsCurrent.hisMonDropEvents = %lu\r\n",statsCurrent.hisMonDropEvents );
			sprintf64Bits( szText, statsCurrent.hisMonOctets );
			sys_console_printf("  statsCurrent.hisMonOctets = %s\r\n", szText );
			sys_console_printf("  statsCurrent.hisMonPkts = %lu\r\n",statsCurrent.hisMonPkts);
			sys_console_printf("  statsCurrent.hisMonBroadcastPkts = %lu\r\n",statsCurrent.hisMonBroadcastPkts);
			sys_console_printf("  statsCurrent.hisMonMulticastPkts = %lu\r\n",statsCurrent.hisMonMulticastPkts);
			sys_console_printf("  statsCurrent.hisMonCRCAlignErrors = %lu\r\n", statsCurrent.hisMonCRCAlignErrors);
			sys_console_printf("  statsCurrent.hisMonUndersizePkts = %lu\r\n", statsCurrent.hisMonUndersizePkts);
			sys_console_printf("  statsCurrent.hisMonOversizePkts = %lu\r\n", statsCurrent.hisMonOversizePkts);
			sys_console_printf("  statsCurrent.hisMonFragments = %lu\r\n", statsCurrent.hisMonFragments);
			sys_console_printf("  statsCurrent.hisMonJabbers = %lu\r\n", statsCurrent.hisMonJabbers);
			sys_console_printf("  statsCurrent.hisMonCollisions = %lu\r\n", statsCurrent.hisMonCollisions);
		}
#endif		
		if (pstatsOnu->iUsed15Min)
		{
			if (!pstatsOnu->firstFlag15M)
			{

				/*如果是第一次进行历史统计，则执行该条件下的步骤*/
				pstatsOnu->firstFlag15M = TRUE;
				pstatsOnu->pCurrent15M= pstatsOnu->pstats15MinHead;
				/*保留数据统计*/
				memcpy(pstatsOnu->pstatsDataPre, &(temp15M.hisStats), sizeof(PON_StatsData_Table64));
				/*第一次采集的历史纪录为0*/
				/*memcpy(&(pstatsOnu->pCurrent15M->hisStats), &(statsCurrent), sizeof(PON_StatsData_Table64 ));*/
				memset(&(pstatsOnu->pCurrent15M->hisStats), 0, sizeof(PON_StatsData_Table64 ));
			}
			else 
			{
				pstatsOnu->pCurrent15M = pstatsOnu->pCurrent15M->pNext;
				/*如果是当前纪录到的统计数据保存块等于Head指针指向的
				数据区，并且不为第一次数据统计，则，Head指向下一个数据区*/
				if (pstatsOnu->pstats15MinHead == pstatsOnu->pCurrent15M)
				{
					pstatsOnu->pstats15MinHead = pstatsOnu->pstats15MinHead->pNext ;
					/*added by wangxiaoyu 2008-02-26*/
					pstatsOnu->pstats15MinTail = pstatsOnu->pstats15MinTail->pNext;
					/*end*/
					memset(&(pstatsOnu->pCurrent15M->hisStats), 0, sizeof(PON_StatsData_Table64));
				}
							
				pstatsOnu->pCurrent15M->currentTick= VOS_GetTick();
				/*保留数据统计*/
				memcpy(pstatsOnu->pstatsDataPre, &(temp15M.hisStats), sizeof(PON_StatsData_Table64));
				memcpy(&(pstatsOnu->pCurrent15M->hisStats), &(statsCurrent), sizeof(PON_StatsData_Table64 ));
					
			}
			/*added by wutw at 15 October*/
			pstatsOnu->pCurrent15M->currentTick= VOS_GetTick();
			pstatsOnu->pCurrent15M->hisStats.hisMonIntervalStart = pstatsOnu->pCurrent15M->currentTick;
			
			
			memset( &dateTime, 0, sizeof(sysDateAndTime_t));
			eventGetCurTime( &dateTime );	
			pstatsOnu->pCurrent15M->year= dateTime.year;
			pstatsOnu->pCurrent15M->month = dateTime.month;
			pstatsOnu->pCurrent15M->day =  dateTime.day;
			pstatsOnu->pCurrent15M->hour = (unsigned short)dateTime.hour;
			pstatsOnu->pCurrent15M->minute = dateTime.minute;
			pstatsOnu->pCurrent15M->second = dateTime.second;
			pstatsOnu->pCurrent15M->MillSecond = 0;	
			

			pstatsOnu->pCurrent15M->bstatus = TRUE;

			}/*end of if (pastatsOnu->iUsed15Min*/

		if (pstatsOnu->iUsed24Hour)
		{
			/*============================================*/
			/*以下为纪录24小时的统计数据*/
			
			if (!pstatsOnu->firstFlag24H)
			{
			/*使用第一次进行统计*/
				pstatsOnu->firstFlag24H = TRUE;
				pstatsOnu->pCurrent24H= pstatsOnu->pstats24HourHead;
				memset(&(pstatsOnu->pCurrent24H->hisStats), 0, sizeof(PON_StatsData_Table64));
			}
			if (pstatsOnu->samplingCount24H >= STATS_SAMPLING_COUNT )
			{
				pstatsOnu->pCurrent24H = pstatsOnu->pCurrent24H->pNext;
				pstatsOnu->samplingCount24H = 0 ;
				/*如果当前的纪录次数达到24个小时，则将历史统计数据
					保存于下一个数据区*/
				if (pstatsOnu->pCurrent24H == pstatsOnu->pstats24HourHead)
				{
					pstatsOnu->pstats24HourHead = pstatsOnu->pstats24HourHead->pNext ;
					/*added by wangxiaoyu 2008-02-26*/
					pstatsOnu->pstats24HourTail = pstatsOnu->pstats24HourTail->pNext;
					/*end*/
					memset(&(pstatsOnu->pCurrent24H->hisStats), 0, sizeof(PON_StatsData_Table64));
				}		
			}
			memset(&(temp24M), 0, sizeof(statistc_HISTORY_CHAIN));
			memcpy(&(temp24M.hisStats), &(pstatsOnu->pCurrent24H->hisStats), sizeof(PON_StatsData_Table64));
				/*sprintf(buf,"\r\n\r\n  pstatsOnu->pCurrent24H->hisStats.hisMonOctets = %.0lf\r\n",pstatsOnu->pCurrent24H->hisStats.hisMonOctets);
			sys_console_printf("%s",buf);*/
				/*使用pstatspon-> pCurrent24H.*/	
			VOS_MemSet( &dateTime, 0, sizeof( sysDateAndTime_t ) );
			eventGetCurTime( &dateTime );
			
			pstatsOnu->pCurrent24H->year = dateTime.year;
			pstatsOnu->pCurrent24H->month = dateTime.month;
			pstatsOnu->pCurrent24H->day = dateTime.day;
			pstatsOnu->pCurrent24H->hour = dateTime.hour;
			pstatsOnu->pCurrent24H->minute = dateTime.minute;
			pstatsOnu->pCurrent24H->second = dateTime.second;
			pstatsOnu->pCurrent24H->MillSecond = 0;
			
			pstatsOnu->pCurrent24H->currentTick = VOS_GetTick();

			temp24M.hisStats.hisMonDropEvents += statsCurrent.hisMonDropEvents;
			temp24M.hisStats.hisMonPkts += statsCurrent.hisMonPkts;
			/*获取收到的总的数据报数*/	
			temp24M.hisStats.hisMonOctets += statsCurrent.hisMonOctets;
			/*获取接收到的广播数据*/
			temp24M.hisStats.hisMonBroadcastPkts += statsCurrent.hisMonBroadcastPkts;
			/*获取接收到的组播数据*/
			temp24M.hisStats.hisMonMulticastPkts += statsCurrent.hisMonMulticastPkts;
			/*获取接收到CRCAlignErrors帧数*/
			temp24M.hisStats.hisMonCRCAlignErrors += statsCurrent.hisMonCRCAlignErrors;
			temp24M.hisStats.hisMonUndersizePkts += statsCurrent.hisMonUndersizePkts;
			temp24M.hisStats.hisMonFragments += statsCurrent.hisMonFragments;
			temp24M.hisStats.hisMonJabbers += statsCurrent.hisMonJabbers;
			/*获取接收到长度大于标准长度的帧数*/
			temp24M.hisStats.hisMonOversizePkts += statsCurrent.hisMonOversizePkts;
			/*获取conllison*/
			temp24M.hisStats.hisMonCollisions += statsCurrent.hisMonCollisions;
			
			memcpy(&(pstatsOnu->pCurrent24H->hisStats), &(temp24M.hisStats), sizeof(PON_StatsData_Table64));
			pstatsOnu->pCurrent24H->hisStats.hisMonIntervalStart = pstatsOnu->pCurrent24H->currentTick;
			
			pstatsOnu->pCurrent24H->bstatus = TRUE;

				pstatsOnu->samplingCount24H ++;
				
			}/*end of if (pastatsOnu->iUsed24Hour*/
		
			pstatsOnu = pstatsOnu->pNext;
	}
	return iRes;
#else
	return VOS_OK;
#endif
}


 /****************************************************
* HisStatsPonRawClear
* 描述:清空pon对象的历史统计
******************************************************/
STATUS HisStatsPonRawClear(short int ponId)
{
	statistic_PON_OID	*pstatsPon = NULL ;
	int hisFlag = 0;

	pstatsPon = gpPONHisStatsHead;
	while(pstatsPon != NULL)
	{
		if ( ponId == pstatsPon->ponIdx)
		{/*两种判断效果: 当找到该ponid 时，如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
			if( (pstatsPon->iUsed15Min) || (pstatsPon->iUsed24Hour) )
			{
				hisFlag = STATS_OK;
				break;					
			}
			else
				return hisFlag;
		}
		pstatsPon = pstatsPon->pNext;
	}

	if (STATS_OK != hisFlag)
	{
		return STATS_ERR;
	}
	if(pstatsPon != NULL)
	{
		/*pstatsPon->iUsed15Min = FALSE;*/
		pstatsPon->pCurrent15M = pstatsPon->pstats15MinHead;
		memset(&(pstatsPon->pCurrent15M->hisStats), 0, sizeof(PON_StatsData_Table64));
		/*pstatsPon->iUsed24Hour = FALSE;*/
		pstatsPon->pCurrent24H= pstatsPon->pstats24HourHead;
		memset(&(pstatsPon->pCurrent24H->hisStats), 0, sizeof(PON_StatsData_Table64));
		return (STATS_OK);
	}
	return STATS_ERR;
}

	
 /****************************************************
* HisStatsPon15MinRawClear
* 描述:清空pon对象的历史统计
******************************************************/
STATUS HisStatsPon15MinRawClear(short int ponId)
{
	statistic_PON_OID	*pstatsPon = NULL ;
	int hisFlag = 0;

	pstatsPon = gpPONHisStatsHead;
	while(pstatsPon != NULL)
	{
		if ( ponId == pstatsPon->ponIdx)
		{/*两种判断效果: 当找到该ponid 时，如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
			if( pstatsPon->iUsed15Min )
			{
				hisFlag = STATS_OK;
				break;					
			}
			else
				return hisFlag;
		}
		pstatsPon = pstatsPon->pNext;
	}

	if (STATS_OK != hisFlag)
	{
		return STATS_ERR;
	}
	if(pstatsPon != NULL)
	{
		/*pstatsPon->iUsed15Min = FALSE;*/
		pstatsPon->pCurrent15M = pstatsPon->pstats15MinHead;
		memset((&pstatsPon->pCurrent15M->hisStats), 0, sizeof(PON_StatsData_Table64));
		return (STATS_OK);
	}
	return STATS_ERR;
}

	
 /****************************************************
* HisStatsPon24HourRawClear
* 描述:清空pon对象的历史统计
******************************************************/
STATUS HisStatsPon24HourRawClear(short int ponId)
{
	statistic_PON_OID	*pstatsPon = NULL ;
	int hisFlag = 0;

	pstatsPon = gpPONHisStatsHead;
	while(pstatsPon != NULL)
	{
		if ( ponId == pstatsPon->ponIdx)
		{/*两种判断效果: 当找到该ponid 时，如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
			if(pstatsPon->iUsed24Hour)
			{
				hisFlag = STATS_OK;
				break;					
			}
			else
				return hisFlag;
		}
		pstatsPon = pstatsPon->pNext;
	}

	if (STATS_OK != hisFlag)
	{
		return STATS_ERR;
	}
	if(pstatsPon != NULL)
	{
		/*pstatsPon->iUsed24Hour = FALSE;*/
		pstatsPon->pCurrent24H= pstatsPon->pstats24HourHead;
		memset(&(pstatsPon->pCurrent24H->hisStats), 0, sizeof(PON_StatsData_Table64));
		return (STATS_OK);
	}
	return STATS_ERR;
}

STATUS HisStatsOnuRawClear(short int ponId, short int onuId)
{
	statistic_PON_OID	*pstatsOnu = NULL ;
	int hisFlag = 0;

	pstatsOnu = gpONUHisStatsHead;
	while(pstatsOnu != NULL)
	{
		if ( (ponId == pstatsOnu->ponIdx) ||(onuId == pstatsOnu->onuIdx))
		{/*两种判断效果: 如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
			if((pstatsOnu->iUsed15Min) || (pstatsOnu->iUsed24Hour))
			{
				hisFlag = STATS_OK;
				break;					
			}
			else
				return hisFlag;
		}
		pstatsOnu = pstatsOnu->pNext;
	}

	if (STATS_OK != hisFlag)
	{
		return STATS_ERR;
	}
	if(pstatsOnu != NULL)
	{
		/*pstatsOnu->iUsed24Hour = FALSE;*/
		pstatsOnu->pCurrent24H= pstatsOnu->pstats24HourHead;
		memset(&(pstatsOnu->pCurrent24H->hisStats), 0, sizeof(PON_StatsData_Table64));
		/*pstatsOnu->iUsed15Min = FALSE;*/
		pstatsOnu->pCurrent15M = pstatsOnu->pstats15MinHead;
		memset(&(pstatsOnu->pCurrent15M->hisStats), 0, sizeof(PON_StatsData_Table64));
		return (STATS_OK);
	}
	return STATS_ERR;
}	
	
 /****************************************************
* HisStatsOnuRawClear
* 描述:清空pon对象的历史统计
******************************************************/
STATUS HisStatsOnu15MinRawClear(short int ponId, short int onuId)
{
	statistic_PON_OID	*pstatsOnu = NULL ;
	int hisFlag = 0;


	pstatsOnu = gpONUHisStatsHead;
	while(pstatsOnu != NULL)
		{
			if ( (ponId == pstatsOnu->ponIdx) ||(onuId == pstatsOnu->onuIdx))
			{/*两种判断效果: 如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
				if(pstatsOnu->iUsed15Min)
				{
				hisFlag = STATS_OK;
				break;					
				}
				else
					return hisFlag;
			}
			
			pstatsOnu = pstatsOnu->pNext;
		}

	if (STATS_OK != hisFlag)
	{
		return STATS_ERR;
	}
	if(pstatsOnu != NULL)
	{
	/*pstatsOnu->iUsed15Min = FALSE;*/
	pstatsOnu->pCurrent15M = pstatsOnu->pstats15MinHead;
	memset(&(pstatsOnu->pCurrent15M->hisStats), 0, sizeof(PON_StatsData_Table64));
	return (STATS_OK);
	}
	else
	{
		return STATS_ERR;
	}
	
}


 /****************************************************
* HisStatsOnuRawClear
* 描述:清空pon对象的历史统计
******************************************************/
STATUS HisStatsOnu24HourRawClear(short int ponId, short int onuId)
{
	statistic_PON_OID	*pstatsOnu = NULL ;
	int hisFlag = 0;


	pstatsOnu = gpONUHisStatsHead;
	while(pstatsOnu != NULL)
		{
			if ( (ponId == pstatsOnu->ponIdx) ||(onuId == pstatsOnu->onuIdx))
			{/*两种判断效果: 如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
				if(pstatsOnu->iUsed24Hour)
				{
				hisFlag = STATS_OK;
				break;					
				}
				else
					return hisFlag;
			}
			
			pstatsOnu = pstatsOnu->pNext;
		}

	if (STATS_OK != hisFlag)
	{
		return STATS_ERR;
	}
	if(pstatsOnu != NULL)
	{
	/*pstatsOnu->iUsed24Hour = FALSE;*/
	pstatsOnu->pCurrent24H= pstatsOnu->pstats24HourHead;
	memset(&(pstatsOnu->pCurrent24H->hisStats), 0, sizeof(PON_StatsData_Table64));
	return (STATS_OK);
	}
	else
	{
		return STATS_ERR;
	}
}

#if 0
#endif
/**********************************************
* HisStatsPonCtrlEntryGet
* 描述 : 该函数从以ponid为条件获取该ponid下的24hour的历史统计链表头地址。
*
* 输入 : slotid,ponid
* 输出 : ppHisStatsChainHead - 该ponid的24Hour历史统计消息的链表头地址
*
*************************************************/
STATUS HisStatsPonCtrlEntryGet(short int SlotId, short int PonId, statistc_HISTORY_CHAIN **ppHisStats24HHead, statistc_HISTORY_CHAIN **ppHisStats15MHead)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	while(pstatsPon != NULL)
		{
			if (PonId == pstatsPon->ponIdx)
				break;
			else
				pstatsPon = pstatsPon->pNext;
		}
	if (pstatsPon == NULL)
		return (STATS_NOFINE_CHAIN_ERR);
	
	*ppHisStats24HHead = pstatsPon->pstats24HourHead;
	*ppHisStats15MHead = pstatsPon->pstats15MinHead;
	return (STATS_OK);
}

/**********************************************
* HisStatsPonCtrlTablePonIdGet
* 描述 : 该函数从特定的监控端口表项中获取slotid和ponid
*************************************************/
STATUS HisStatsPonCtrlTablePonIdGet(statistic_PON_OID *pHisStatsPon, short int *pSlotId, short int *pPonId)
{
	if (pHisStatsPon== NULL)
		return (STATS_POINT_NULL_ERR);
	*pSlotId = pHisStatsPon->slotIdx;
	*pPonId = pHisStatsPon->ponIdx;
	return (STATS_OK);
}




/****************************************************
* GetHisStatsPonCtrlTableFirst
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,statsType
*
*	输出: ppStatsIdx - 指向获取控制表的头地址
*
*	返回: 
******************************************************/
STATUS HisStatsPonCtrlTableFirstIndexGet(/*short int SlotId, short int PonId, */statistic_PON_OID **ppStatsIdx)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	if (NULL == gpPONHisStatsHead)
		return STATS_POINT_NULL_ERR ;
	/*while ((pstatsPon != NULL) && (pstatsPon->ponId == PonId))
		{
			pstatsPon = pstatsPon->pNext;
		}*/
	if (pstatsPon == NULL)
		return (STATS_NOFINE_CHAIN_ERR);
	*ppStatsIdx = pstatsPon;

		return (STATS_OK);
}


/****************************************************
* HisStatsPonCtrlTableNextIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,statsType
*
*	输出: ppStatsIdx - 指向获取控制表的地址
*
*	返回: 
******************************************************/
STATUS HisStatsPonCtrlTableNextIndexGet(/*short int SlotId, short int PonId, */statistic_PON_OID *pstatsPon, statistic_PON_OID **ppStatsIdx)
{
	/*statistic_PON_OID	*pstatsPon = *ppStatsIdx ;*/
	if (pstatsPon->pNext == NULL)
		return (STATS_END_OBJ_ERR);
	*ppStatsIdx = pstatsPon->pNext;
	return (STATS_OK);
}


/****************************************************
* HisStatsPonCtrlTableGet
* 描述: 该函数用于读取所有的历史统计对象的控制表内容
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,StatsObjType - 获取统计计数的类型
*
*	输出: ppStatsData - 指向获取控制表的地址
*
*	返回: 
******************************************************/
STATUS HisStatsPonCtrlTableGet(/*short int SlotId, short int PonId, *//* short int StatsObjType,*/statistic_PON_OID *pstatsIdx, statistic_PON_OID **ppStatsData)
{

	if ((pstatsIdx == NULL)/* || (pstatsIdx->ponId != PonId*/)
		return (STATS_NOFINE_CHAIN_ERR);
	*ppStatsData = pstatsIdx;
	return (STATS_OK);
}

#if 0
STATUS CliHisStatsPonCtrlTableShow(void)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	statistc_HISTORY_CHAIN *pTemp15 = NULL;
	statistc_HISTORY_CHAIN *pTemp24 = NULL;
	if (pstatsPon == NULL)
		{
		sys_console_printf("  NULL.\r\n");
		return STATS_ERR;
		}
	sys_console_printf("  total num :%d\r\n", PONHistoryStatsCounter);
	while(pstatsPon != NULL)
		{
			pTemp15 = pstatsPon->pstats15MinHead;
			pTemp24 = pstatsPon->pstats24HourHead;
			sys_console_printf("  pon %d    ",pstatsPon->ponIdx);
			(pstatsPon->iUsed15Min) ? sys_console_printf("history 15Min is TRUE\r\n") : sys_console_printf("history 15Min is FALSE!\r\n");
			if (pTemp15 == NULL)
				{
				sys_console_printf("  15MinHead is NULL!\r\n");
				}
			/*while (pTemp15 != NULL)
				{	
					sys_console_printf("  No.%d  CurrentTick = %d", i, pTemp15->currentTick);
					pTemp15= pTemp15->pNext;
					if (pTemp15 == pstatsPon->pstats15MinHead)
						break;
					i++;
				}*/

			sys_console_printf("\r\n  pon %d    ",pstatsPon->ponIdx);
			(pstatsPon->iUsed24Hour) ? sys_console_printf("history 24Hour is TRUE\r\n") : sys_console_printf("history 24Hour is FALSE!\r\n");

			if (pTemp24 == NULL)
				{
				sys_console_printf("  24Hour Head is NULL!\r\n");
				}
			/*while (pTemp24 != NULL)
				{	
					sys_console_printf("  No.%d  CurrentTick = %d", i, pTemp24->currentTick);
					pTemp24= pTemp24->pNext;
					if (pTemp24 == pstatsPon->pstats24HourHead)
						break;
					i++;
				}	*/
			sys_console_printf("  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
			
			pstatsPon = pstatsPon->pNext;
			
		}
	return STATS_OK;
}
#endif

STATUS	locateHisStatsDataEntry( ULONG dev, ULONG brd, ULONG port, ULONG sample, ULONG port_type, ULONG his_type,
										statistic_PON_OID **ppstatsObjEntry, statistc_HISTORY_CHAIN **chainEntry)
{
	LOCATIONDES lct;
	STATUS  ret = VOS_ERROR;
	statistic_PON_OID * pStatsEntry = NULL;
	short int ponid = 0;

	getLocation( dev, &lct, CONV_YES );
				
	switch( port_type )
	{
		case PORT_TYPE_ETH:
			pStatsEntry = gpEthHisStatsHead;
			break;
		case PORT_TYPE_PON:
			pStatsEntry = gpPONHisStatsHead;
			break;
		case PORT_TYPE_ONU:
			pStatsEntry = gpONUHisStatsHead;
			break;
		default:
			return ret;
	}

	if( dev == OLT_DEV_ID && port_type == PORT_TYPE_PON)
		ponid = GetPonPortIdxBySlot( brd, port );
	else
		ponid = GetPonPortIdxBySlot( lct.slot, lct.port );	

	if( pStatsEntry != NULL )
	{
		for( pStatsEntry=pStatsEntry; pStatsEntry!=NULL; pStatsEntry=pStatsEntry->pNext )
		{
			if( port_type != PORT_TYPE_ETH )/*pon port statistic data*/
			{
				if( ( dev == OLT_DEV_ID && pStatsEntry->ponIdx == ponid ) ||
				( dev != OLT_DEV_ID && pStatsEntry->ponIdx == ponid && pStatsEntry->onuIdx == lct.onuId ) )
					break;
			}
			else if( pStatsEntry->slotIdx == brd && pStatsEntry->ethIdx == port )
				break;
		}
	}

	if( pStatsEntry != NULL )
	{
		statistc_HISTORY_CHAIN *pChainEntry  =NULL;
		statistc_HISTORY_CHAIN *pChainEntryEnd  =NULL;
		ULONG counter = 0;

		if( his_type == STATS_15MIN_SET )/*15 minute statistic*/
		{
			pChainEntry = pStatsEntry->pCurrent15M;
			pChainEntryEnd = pStatsEntry->pstats15MinHead;
		}
		else
		{
			pChainEntry = pStatsEntry->pCurrent24H;
			pChainEntryEnd = pStatsEntry->pstats24HourHead;
		}


		do{
			counter++;
			if( counter == sample )
			{
				*ppstatsObjEntry = pStatsEntry;
				*chainEntry = pChainEntry;		
				ret = VOS_OK;				
				break;
			}
			else
				pChainEntry = pChainEntry->pPre;
			
		}while( pChainEntry!=NULL && pChainEntry != pChainEntryEnd->pPre );
			
	}
	
	return ret;
}

STATUS	HisStatsEthFirstEntry( statistic_ETH_PORT **ppStatsEth, statistc_HISTORY_CHAIN **ppStatsChain, UCHAR type )
{
	statistic_ETH_PORT	*pstatsEth = gpEthHisStatsHead ;

	if (NULL == pstatsEth)
		return STATS_POINT_NULL_ERR ;

	if( type == STATS_15MIN_SET)/*15 minute statistic*/
	{
		while ((pstatsEth != NULL) && (!(pstatsEth->iUsed15Min)))
			pstatsEth = pstatsEth->pNext;
		
		if (NULL == pstatsEth)
			return STATS_ERR;
		if (!(pstatsEth->iUsed15Min))
			return STATS_ERR;

		*ppStatsEth = pstatsEth;
		*ppStatsChain = pstatsEth->pCurrent15M;
	}
	else
	{
		while( pstatsEth!=NULL && (!(pstatsEth->iUsed24Hour)) )
			pstatsEth = pstatsEth->pNext;

		if( pstatsEth == NULL )
			return STATS_ERR;

		if(!(pstatsEth->iUsed24Hour))
			return STATS_ERR;

		*ppStatsEth = pstatsEth;
		*ppStatsChain = pstatsEth->pCurrent24H;
	}
	
	return (STATS_OK);
}

STATUS HisStatsEthNextEntry(statistic_ETH_PORT *pstatEth, statistc_HISTORY_CHAIN *pStatschain, 
								statistic_ETH_PORT **ppstatEth, statistc_HISTORY_CHAIN **ppStatschain , UCHAR type)
{	
	statistic_PON_OID *pstatTemp = NULL;
	/**/
	if( type == STATS_15MIN_SET )
	{
		if (pstatEth->pstats15MinHead == pStatschain)
		{
			if (pstatEth->pNext == NULL)
				return (STATS_END_OBJ_ERR);
			pstatTemp = pstatEth->pNext;
		
			while ((pstatTemp != NULL) && (!(pstatTemp->iUsed15Min)))
				pstatTemp = pstatTemp->pNext;
			if (NULL == pstatTemp)
				return (STATS_END_OBJ_ERR);
			if (!(pstatTemp->iUsed15Min))
				return (STATS_END_OBJ_ERR);	
			*ppstatEth = pstatTemp;
			*ppStatschain = pstatTemp->pCurrent15M;
		}
		else
		{
			*ppstatEth = pstatEth;
			*ppStatschain = pStatschain->pPre;
		}	
	}
	else
	{
		if (pstatEth->pstats24HourHead == pStatschain)
		{
			if (pstatEth->pNext == NULL)
				return (STATS_END_OBJ_ERR);
			pstatTemp = pstatEth->pNext;
		
			while ((pstatTemp != NULL) && (!(pstatTemp->iUsed24Hour)))
				pstatTemp = pstatTemp->pNext;
			if (NULL == pstatTemp)
				return (STATS_END_OBJ_ERR);
			if (!(pstatTemp->iUsed24Hour))
				return (STATS_END_OBJ_ERR);	
			*ppstatEth = pstatTemp;
			*ppStatschain = pstatTemp->pCurrent24H;
		}
		else
		{
			*ppstatEth = pstatEth;
			*ppStatschain = pStatschain->pPre;
		}		
	}

	return (STATS_OK);
}


/****************************************************
* HisStatsPon15MTableFirstIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*	输入: soltid,ponid,onuid,statsType
*	输出: ppstatPonIdx指向当前ponid的控制表项
*			ppStatsChainIdx - 当前ponid的历史统计数据表的头地址
*	返回: 
******************************************************/
STATUS HisStatsPon15MTableFirstIndexGet(statistic_PON_OID **ppstatPonIdx ,statistc_HISTORY_CHAIN **ppStatsChainIdx)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	if (NULL == pstatsPon)
		return STATS_POINT_NULL_ERR ;

	/*modified by wutw at July eighteen*/	
	if (pstatsPon->iUsed15Min)
		*ppstatPonIdx = pstatsPon;
	else{/*如果当前pon对象的该15分钟的历史统计并没有使能,
		则循环取下一个pon对象,直到下一个pon使能该15分钟的历史统计*/
			while ((pstatsPon != NULL) && (!(pstatsPon->iUsed15Min)))
				pstatsPon = pstatsPon->pNext;
			
			if (NULL == pstatsPon)
				return STATS_ERR;
			if (!(pstatsPon->iUsed15Min))
				return STATS_ERR;
			*ppstatPonIdx = pstatsPon;
		}
	
	*ppStatsChainIdx = pstatsPon->pCurrent15M;
	return (STATS_OK);
}

/****************************************************
* HisStatsPon15MTableNextIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,statsType
*
*	输出: ppstatPonIdx指向当前ponid的控制表项
*			ppStatsChainIdx - 当前ponid的历史统计数据表的下地址
*
*	返回: 
******************************************************/
STATUS HisStatsPon15MTableNextIndexGet(statistic_PON_OID *pstatPon, statistc_HISTORY_CHAIN *pStatschain, 
								statistic_PON_OID **ppstatPonIdx, statistc_HISTORY_CHAIN **ppStatschainIdx)
{	
	statistic_PON_OID *pstatTemp = NULL;
	/**/
	if (pstatPon->pstats15MinHead == pStatschain)
		{
			if (pstatPon->pNext == NULL)
				return (STATS_END_OBJ_ERR);
			pstatTemp = pstatPon->pNext;
		
			/*如果当前pon对象的15分钟的历史统计并没有使能,则循环
			取下一个pon对象,直到下一个pon使能该15分钟的历史统计*/
			while ((pstatTemp != NULL) && (!(pstatTemp->iUsed15Min)))
				pstatTemp = pstatTemp->pNext;
			if (NULL == pstatTemp)
				return (STATS_END_OBJ_ERR);
			if (!(pstatTemp->iUsed15Min))
				return (STATS_END_OBJ_ERR);	
			*ppstatPonIdx = pstatTemp;
			*ppStatschainIdx = pstatTemp->pCurrent15M;
		}
	else
		{
	*ppstatPonIdx = pstatPon;
	*ppStatschainIdx = pStatschain->pPre;
	}

	return (STATS_OK);
}


/****************************************************
* CliHisStatsPon15MTableNextIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,statsType
*
*	输出: ppstatPonIdx指向当前ponid的控制表项
*			ppStatsChainIdx - 当前ponid的历史统计数据表的下地址
*
*	返回: 
******************************************************/
STATUS CliHisStatsPon15MTableNextIndexGet(statistic_PON_OID *pstatPon, statistc_HISTORY_CHAIN *pStatschain, 
								statistic_PON_OID **ppstatPonIdx, statistc_HISTORY_CHAIN **ppStatschainIdx)
{	
	/**/
	if ( (pstatPon->pstats15MinHead == pStatschain) || (FALSE == pStatschain->bstatus) )
		{
			return (STATS_END_OBJ_ERR);
		}
	else
		{
	*ppstatPonIdx = pstatPon;
	*ppStatschainIdx = pStatschain->pPre;
	}
	return (STATS_OK);
}


/****************************************************
* HisStatsPonStatsTable15MGet
* 描述: 该函数用于读取所有的历史统计对象的具体值
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,StatsObjType - 获取统计计数的类型
*			pstatsIdx - 当前pon的历史统计表的表项地址
*	输出: pStatsData - 当前所要获取的历史统计
*
*	返回: 
******************************************************/
STATUS HisStatsPon15MTableGet(short int StatsObjType, statistc_HISTORY_CHAIN *pstatsIdx, unsigned long *pStatsData)
{
	statistc_HISTORY_CHAIN	*pstatsPon = pstatsIdx ;
	if (NULL == pstatsPon)
		return STATS_POINT_NULL_ERR ;
	if (NULL == pStatsData)
		return STATS_POINT_NULL_ERR ;

	switch (StatsObjType)
		{
		case STATS_DROPEVENTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonDropEvents ;
			break;
		case STATS_OCTETS_TYPE:
			*pStatsData = (unsigned long)pstatsPon->hisStats.hisMonOctets ;
			break;
		case STATS_PKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonPkts ;
			break;
		case STATS_BROADCASTPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonBroadcastPkts ;
			break;
		case STATS_MULTICASTPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonMulticastPkts ;
			break;
		case STATS_CRCALIGNERR_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonCRCAlignErrors ;
			break;
		case STATS_UNDERSIZEPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonUndersizePkts ;
			break;
		case STATS_OVERSIZEPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonOversizePkts ;
			break;
		case STATS_FRAGMENTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonFragments ;
			break;
		case STATS_JABBERS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonJabbers ;
			break;
		case STATS_COLLISIONS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonCollisions ;
			break;
		default:
			return (STATS_INVALUE_OBJ_ERR);
		}
	return (STATS_OK);
}

/****************************************************
* HisStatsPon24HTableFirstIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,statsType
*
*	输出: ppstatPonIdx指向当前ponid的控制表项
*			ppStatsChainIdx - 当前ponid的历史统计数据表的头地址
*
*	返回: 
******************************************************/
STATUS HisStatsPon24HTableFirstIndexGet(/*short int SlotId, short int PonId, */statistic_PON_OID **ppstatPonIdx ,statistc_HISTORY_CHAIN **ppStatsChainIdx)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	if (NULL == pstatsPon)
		return STATS_POINT_NULL_ERR ;

	/*modified by wutw at July eighteen*/	
	if (pstatsPon->iUsed24Hour)
		*ppstatPonIdx = pstatsPon;
	else{/*如果当前pon对象的该24Hour的历史统计并没有使能,
		则循环取下一个pon对象,直到下一个pon使能该24Hour的历史统计*/
			while ((pstatsPon != NULL) && (!(pstatsPon->iUsed24Hour)))
				pstatsPon = pstatsPon->pNext;
			if (NULL == pstatsPon)
				return STATS_ERR;
			if (!(pstatsPon->iUsed24Hour))
				return STATS_ERR;
			*ppstatPonIdx = pstatsPon;
		}
	
	*ppStatsChainIdx = pstatsPon->pCurrent24H;
	return (STATS_OK);

}


/****************************************************
* HisStatsPon24HTableNextIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,statsType
*
*	输出: ppstatPonIdx指向当前ponid的控制表项
*			ppStatsChainIdx - 当前ponid的历史统计数据表的下地址
*
*	返回: 
******************************************************/
STATUS HisStatsPon24HTableNextIndexGet( statistic_PON_OID *pstatPon,statistc_HISTORY_CHAIN *pStatsChain, 
									statistic_PON_OID **ppstatsPonIdx, statistc_HISTORY_CHAIN **ppStatsChainIdx)
{

	statistic_PON_OID *pstatTemp = NULL;
	/**/
	if (pstatPon->pstats24HourHead == pStatsChain)
		{
			if (pstatPon->pNext == NULL)
				return (STATS_END_OBJ_ERR);
			pstatTemp = pstatPon->pNext;
		
			/*如果当前pon对象的24Hour的历史统计并没有使能,则循环
			取下一个pon对象,直到下一个pon使能该24Hour的历史统计*/
			while ((pstatTemp != NULL) && (!(pstatTemp->iUsed24Hour)))
				pstatTemp = pstatTemp->pNext;
			if (NULL == pstatTemp)
				return (STATS_END_OBJ_ERR);
			if (!(pstatTemp->iUsed24Hour))
				return (STATS_END_OBJ_ERR);	
			*ppstatsPonIdx = pstatTemp;
			*ppStatsChainIdx = pstatTemp->pCurrent24H;
		}
	else
		{
			*ppstatsPonIdx = pstatPon;
			*ppStatsChainIdx = pStatsChain->pPre;
		}

#if 0
	if (pstatPon->pstats24HourTail == pStatsChain)
		{
		if (pstatPon->pNext == NULL)
			return (STATS_END_OBJ_ERR);
		*ppstatsPonIdx = pstatPon->pNext;
		*ppStatsChainIdx = pstatPon->pNext->pstats24HourHead;
		return (STATS_END_CHAIN_ERR);
		}	
	*ppstatsPonIdx = pstatPon;
	*ppStatsChainIdx = pStatsChain->pNext;
#endif	
	return (STATS_OK);
}



/****************************************************
* CliHisStatsPon24HTableNextIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,statsType
*
*	输出: ppstatPonIdx指向当前ponid的控制表项
*			ppStatsChainIdx - 当前ponid的历史统计数据表的下地址
*
*	返回: 
******************************************************/
STATUS CliHisStatsPon24HTableNextIndexGet( statistic_PON_OID *pstatPon,statistc_HISTORY_CHAIN *pStatsChain, 
									statistic_PON_OID **ppstatsPonIdx, statistc_HISTORY_CHAIN **ppStatsChainIdx)
{
	/**/
	if ( (pstatPon->pstats24HourHead == pStatsChain) || (FALSE == pStatsChain->bstatus) )
		{
			return (STATS_END_OBJ_ERR);
		}
	else
		{
			*ppstatsPonIdx = pstatPon;
			*ppStatsChainIdx = pStatsChain->pPre;
		}

	return (STATS_OK);
}


/****************************************************
* HisStatsPon24HTableGet
* 描述: 该函数用于读取所有的历史统计对象的具体值
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,StatsObjType - 获取统计计数的类型
*			pstatsIdx - 当前pon的历史统计表的表项地址
*	输出: pStatsData - 当前所要获取的历史统计
*
*	返回: 
******************************************************/
STATUS HisStatsPon24HTableGet(short int StatsObjType,statistc_HISTORY_CHAIN *pstatsIdx, unsigned long *pStatsData)
{
	statistc_HISTORY_CHAIN *pstatsPon = pstatsIdx ;
	if (NULL == pstatsIdx)
		return STATS_POINT_NULL_ERR ;
	if (NULL == pStatsData)
		return STATS_POINT_NULL_ERR ;

	switch (StatsObjType)
		{
		case STATS_DROPEVENTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonDropEvents ;
			break;
		case STATS_OCTETS_TYPE:
			*pStatsData = (unsigned long)pstatsPon->hisStats.hisMonOctets ;
			break;
		case STATS_PKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonPkts ;
			break;
		case STATS_BROADCASTPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonBroadcastPkts ;
			break;
		case STATS_MULTICASTPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonMulticastPkts ;
			break;
		case STATS_CRCALIGNERR_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonCRCAlignErrors ;
			break;
		case STATS_UNDERSIZEPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonUndersizePkts ;
			break;
		case STATS_OVERSIZEPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonOversizePkts ;
			break;
		case STATS_FRAGMENTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonFragments ;
			break;
		case STATS_JABBERS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonJabbers ;
			break;
		case STATS_COLLISIONS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonCollisions ;
			break;
		default:
			return (STATS_INVALUE_OBJ_ERR);
		}
	return (STATS_OK);
}

/**********************************************************
* HisStatsPonModified
*	说明:该函数用于cli 设置统计计数的使能和去使能,以及统计计数的最大历史数据
*	ponId :
* 	DONE : when that value is true ,that is start stats, or that is stop stats
*
*************************************************************/
/* modified by chenfj 2008-7-10
     调用这个函数并不能实现启动PON 端口的15分钟和24小时历史统计
     在函数内部, 没有给actcode 赋值;
     参照同时启动ONU 的15分钟和24小时历史统计的实现, 是在函数内部
     做了两次循环, 每次执行一种类型的统计启动
     */
int HisStatsPonStatsStart (short int ponId, BOOL Done)
{
	int iRes = 0;
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};
	historyStatsMsg *pstatsRecMsg;
	unsigned int bucketNum15M = 0;
	unsigned int bucketNum24H = 0;

	
	HisStats15MinMaxRecordGet(&bucketNum15M);
	HisStats24HoursMaxRecordGet(&bucketNum24H);

	iRes = HisStatsPon15MModified(ponId, bucketNum15M, Done);
	iRes = HisStatsPon24HModified(ponId, bucketNum24H, Done);
	return(iRes );
	
	/*pstatsRecMsg = (historyStatsMsg *)VOS_Malloc((ULONG)sizeof(historyStatsMsg), MODULE_STATSTICS);
	if (NULL == pstatsRecMsg)
		return STATS_ERR;
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));	
	pstatsRecMsg->slotId = 0;
	pstatsRecMsg->ponId = ponId;
	pstatsRecMsg->onuId = 0;
	pstatsRecMsg->statsType = STATS_PONOBJ_MODIFIED;

	pstatsRecMsg->flag15M = Done;
	pstatsRecMsg->flag24H = Done;

	pstatsRecMsg->bucketNum15m = bucketNum15M;
	pstatsRecMsg->bucketNum24h = bucketNum24H;
	pstatsRecMsg->errCode = STATS_ERRCODE_TIMEOUT_ERR;	
	aulMsg[3] = (ULONG)(pstatsRecMsg);
	iRes = VOS_QueSend(gMQidsStats, aulMsg, NO_WAIT,MSG_PRI_NORMAL);
	
	if (VOS_ERROR == iRes)
	{
		ASSERT( 0 );
		VOS_Free((VOID *)pstatsRecMsg);
		pstatsRecMsg = NULL;
		return iRes;
	}
	return iRes;*/
}	
	

/**********************************************************
* HisStatsOnuModified
*	说明:该函数用于cli 设置统计计数的使能和去使能,以及统计计数的最大历史数据
*	ponId :
* 	DONE : when that value is true ,that is start stats, or that is stop stats
*
*************************************************************/
int HisStatsOnuStatsStart  (short int ponId, short int onuId, BOOL Done)
{
	int iRes = 0, i =0;

	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};
	historyStatsMsg *pstatsRecMsg;
	unsigned int bucketNum15M = 0;
	unsigned int bucketNum24H = 0;

	for( ; i<2; i++ )
	{
		HisStats15MinMaxRecordGet(&bucketNum15M);
		HisStats24HoursMaxRecordGet(&bucketNum24H);
		pstatsRecMsg = (historyStatsMsg *)VOS_Malloc((ULONG)sizeof(historyStatsMsg), MODULE_STATSTICS);
		if (NULL == pstatsRecMsg)
			return STATS_ERR;
		memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));	
		pstatsRecMsg->slotId = 0;
		pstatsRecMsg->ponId = ponId;
		pstatsRecMsg->onuId = onuId;
		pstatsRecMsg->statsType = STATS_ONUOBJ_MODIFIED;

		pstatsRecMsg->flag15M = Done;
		pstatsRecMsg->flag24H = Done;

		pstatsRecMsg->bucketNum15m = bucketNum15M;
		pstatsRecMsg->bucketNum24h = bucketNum24H;
		pstatsRecMsg->errCode = STATS_ERRCODE_TIMEOUT_ERR;	
		aulMsg[3] = (ULONG)(pstatsRecMsg);

		/*added by wangxy 2007-11-05
		设置actcode,标识要设置哪种模式的历史统计数据
		*/
		pstatsRecMsg->actcode = ( i==0 )?STATS_15MIN_SET:STATS_24HOUR_SET;
		iRes = VOS_QueSend(gMQidsStats, aulMsg, NO_WAIT,MSG_PRI_NORMAL);
		
		if (VOS_ERROR == iRes)
		{
			ASSERT( 0 );
			VOS_Free((VOID *)pstatsRecMsg);
			pstatsRecMsg = NULL;
			return iRes;
		}	
	}
	return iRes;
}	

int HisStatsEth15MEnableSet( ulong_t slot,  ulong_t eth, UINT bucketNum, BOOL flag15m )
{
	historyStatsMsg *pMsg = NULL;

	ulong_t msg[4]={MODULE_STATSTICS,0,0,0};
	int ret = STATS_ERR;

	pMsg = (historyStatsMsg*)VOS_Malloc( sizeof(historyStatsMsg), MODULE_STATSTICS );
	if( pMsg != NULL )
	{
		pMsg->statsType = STATS_ETHPORT_MODIFIED;
		pMsg->slotId= slot;
		pMsg->onuId = eth;

		pMsg->bucketNum15m= bucketNum;
		pMsg->errCode = STATS_ERRCODE_TIMEOUT_ERR;	
		pMsg->actcode = STATS_15MIN_SET;	

		pMsg->flag15M = flag15m;

		msg[3] = (ULONG)pMsg;

		ret = VOS_QueSend( gMQidsStats, msg, NO_WAIT, MSG_PRI_NORMAL );

		if( ret == STATS_ERR )
		{
			VOS_Free( pMsg );
			pMsg = NULL;
		}
	}

	return ret;
}

int HisStatsEth24HEnableSet( ulong_t slot,  ulong_t eth, UINT bucketNum, BOOL flag24h  )
{
	historyStatsMsg *pMsg = NULL;

	ulong_t msg[4]={MODULE_STATSTICS,0,0,0};
	int ret = STATS_ERR;

	pMsg = (historyStatsMsg*)VOS_Malloc( sizeof(historyStatsMsg), MODULE_STATSTICS );
	if( pMsg != NULL )
	{
		pMsg->statsType = STATS_ETHPORT_MODIFIED;
		pMsg->slotId = slot;
		pMsg->onuId = eth;

		pMsg->bucketNum24h = bucketNum;
		pMsg->errCode = STATS_ERRCODE_TIMEOUT_ERR;	
		pMsg->actcode = STATS_24HOUR_SET;	

		pMsg->flag24H = flag24h;

		msg[3] = (ULONG)pMsg;

		ret = VOS_QueSend( gMQidsStats, msg, NO_WAIT, MSG_PRI_NORMAL );

		if( ret == STATS_ERR )
		{
			VOS_Free( pMsg );
			pMsg = NULL;
		}
	}

	return ret;
}

/**********************************************************
* HisStatsPonModified
*	说明:该函数用于网管设置统计计数的使能和去使能,以及统计计数的最大历史数据
*	ponId :
*	bucketNum: 历史统计的最大数量
*	flag15M	  :TRUE -使能15M的历史统计,FALSE : 关闭15M的历史统计
*	flag24H	  :该参数同上.但该参数只有使能15M的功能时该参数的TRUE的设置才有效果.
*
*
*************************************************************/
int HisStatsPon15MModified (short int ponId, unsigned int bucketNum, BOOL flag15M)
{
	statistic_PON_OID  *pTempstats = gpPONHisStatsHead;
	statistic_PON_OID *pModifiedstats = NULL;
	int iRes = STATS_OK;
	long		lRes = 0;
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};
	historyStatsMsg *pstatsRecMsg;
	#if 0
	if (!OLTAdv_IsExist ( ponId ))
	{
		return STATS_ERR;
	}
	#endif
	/*if (PONHistoryStatsCounter >= STATS_HIS_MAXPON) 
		return (STATS_MAXPON_OUTRANG_ERR);*/
	if (bucketNum > STATS_MAX15MIN_BUCKETNUM)
		return (STATS_ERR);		
	pstatsRecMsg = (historyStatsMsg *)VOS_Malloc((ULONG)sizeof(historyStatsMsg), MODULE_STATSTICS);
	if (NULL == pstatsRecMsg)
		return STATS_ERR;
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));	
	pstatsRecMsg->slotId = 0;
	pstatsRecMsg->ponId = ponId;
	pstatsRecMsg->onuId = 0;
	pstatsRecMsg->statsType = STATS_PONOBJ_MODIFIED;
	while(pTempstats != NULL)
		{
			if(pTempstats->ponIdx == ponId)
				{
				pModifiedstats = pTempstats;
				/*return (STATS_PONID_EXIST_ERR);*/
				break;
				}
			/*pPreModif = pTempstats;*/
			pTempstats = pTempstats->pNext;
		}
	if(pModifiedstats == NULL)
		{
			pstatsRecMsg->flag15M = flag15M;
			pstatsRecMsg->flag24H = FALSE;		
		}	
	else
		{
			pstatsRecMsg->flag15M = flag15M;
			pstatsRecMsg->flag24H = pModifiedstats->iUsed24Hour;
		}

	pstatsRecMsg->bucketNum15m= bucketNum;
	pstatsRecMsg->errCode = STATS_ERRCODE_TIMEOUT_ERR;	
	pstatsRecMsg->actcode = STATS_15MIN_SET;
	
	aulMsg[3] = (ULONG)(pstatsRecMsg);
	lRes = VOS_QueSend(gMQidsStats, aulMsg, NO_WAIT,MSG_PRI_NORMAL);
	
	if (VOS_ERROR == lRes)
	{
		ASSERT( 0 );
		VOS_Free((VOID *)pstatsRecMsg);
		pstatsRecMsg = NULL;
		return iRes;
	}

	return iRes;
}



/**********************************************************
* HisStatsPonModified
*	说明:该函数用于网管设置统计计数的使能和去使能,以及统计计数的最大历史数据
*	ponId :
*	bucketNum: 历史统计的最大数量
*	flag15M	  :TRUE -使能15M的历史统计,FALSE : 关闭15M的历史统计
*	flag24H	  :该参数同上.但该参数只有使能15M的功能时该参数的TRUE的设置才有效果.
*
*
*************************************************************/
int HisStatsPon24HModified (short int ponId, unsigned int bucketNum, BOOL flag24H)
{
	int iRes = STATS_OK;
	long		lRes = 0;
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};
	historyStatsMsg *pstatsRecMsg;
	statistic_PON_OID  *pTempstats = gpPONHisStatsHead;
	statistic_PON_OID  *pModifiedstats = NULL;
	/*if (PONHistoryStatsCounter >= STATS_HIS_MAXPON) 
		return (STATS_MAXPON_OUTRANG_ERR);*/
	if (bucketNum > STATS_MAX24HOUR_BUCKETNUM)
		return (STATS_ERR);
	
	pstatsRecMsg = (historyStatsMsg *)VOS_Malloc((ULONG)sizeof(historyStatsMsg), MODULE_STATSTICS);
	if (NULL == pstatsRecMsg)
		return STATS_ERR;
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));	
	pstatsRecMsg->slotId = 0;
	pstatsRecMsg->ponId = ponId;
	pstatsRecMsg->onuId = 0;
	pstatsRecMsg->statsType = STATS_PONOBJ_MODIFIED;
	while(pTempstats != NULL)
		{
			if(pTempstats->ponIdx == ponId)
				{
				pModifiedstats = pTempstats;
				/*return (STATS_PONID_EXIST_ERR);*/
				break;
				}
			/*pPreModif = pTempstats;*/
			pTempstats = pTempstats->pNext;
		}
	if (NULL == pTempstats)
		{
			pstatsRecMsg->flag15M = FALSE;
			pstatsRecMsg->flag24H = flag24H;
		}
	else if (NULL != pModifiedstats)
		{
			pstatsRecMsg->flag15M = pModifiedstats->iUsed15Min;
			pstatsRecMsg->flag24H = flag24H;
		}
	else
		return STATS_ERR;	
	
	
	pstatsRecMsg->bucketNum24h= bucketNum;
	pstatsRecMsg->errCode = STATS_ERRCODE_TIMEOUT_ERR;	
	pstatsRecMsg->actcode = STATS_24HOUR_SET;
	
	aulMsg[3] = (ULONG)(pstatsRecMsg);
	lRes = VOS_QueSend(gMQidsStats, aulMsg, NO_WAIT,MSG_PRI_NORMAL);
	
	if (VOS_ERROR == lRes)
	{
		ASSERT( 0 );
		VOS_Free((VOID *)pstatsRecMsg);
		pstatsRecMsg = NULL;
		return iRes;
	}
	return iRes;
}

/**********************************************************
* HisStatsOnuModified
*	说明:该函数用于网管设置统计计数的使能和去使能,以及统计计数的最大历史数据
*	ponId ,onuId
*	bucketNum: 历史统计的最大数量
*	flag15M	  :TRUE -使能15M的历史统计,FALSE : 关闭15M的历史统计
*	flag24H	  :该参数同上.但该参数只有使能15M的功能时该参数的TRUE的设置才有效果.
*
*
*************************************************************/

int HisStatsOnu15MModified(short int ponId, short int onuId, unsigned int bucketNum, BOOL flag15M)
{
	statistic_PON_OID  *pTempstats = gpONUHisStatsHead;
	statistic_PON_OID  *pModifiedstats = NULL;
	int iRes = STATS_OK;
	historyStatsMsg *pstatsRecMsg = NULL;
	long		lRes = 0;
	/*bool		passave_onu = TRUE;*/
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};
	#if 0
	if ((Is_passave_onu ( ponId, 
						  onuId,
						  &passave_onu ) != STATS_OK) || 
		(!passave_onu)) 
	{
		return (STATS_ERR);
	}	 
	#endif
	/*if (ONUHistoryStatsCounter >= STATS_HIS_MAXONU)
		return (STATS_MAXPON_OUTRANG_ERR);*/
	if (bucketNum > STATS_MAX15MIN_BUCKETNUM)
		return (STATS_ERR);		
	pstatsRecMsg = (historyStatsMsg *)VOS_Malloc((ULONG)sizeof(historyStatsMsg), MODULE_STATSTICS);
	if (NULL == pstatsRecMsg)
		return STATS_ERR;	
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));	
	pstatsRecMsg->slotId = 0;
	pstatsRecMsg->ponId = ponId;
	pstatsRecMsg->onuId = onuId;
	pstatsRecMsg->statsType = STATS_ONUOBJ_MODIFIED;
	while(pTempstats != NULL)
		{
			if((pTempstats->ponIdx == ponId) && (pTempstats->onuIdx == onuId))
				{
				pModifiedstats = pTempstats;
				break;
				}
			/*pPreModif = pTempstats;*/
			pTempstats = pTempstats->pNext;
		}
	if(pTempstats == NULL)
		{
			pstatsRecMsg->flag15M = flag15M;
			pstatsRecMsg->flag24H = FALSE;		
		}	
	else if (NULL != pModifiedstats)
		{
			pstatsRecMsg->flag15M = flag15M;
			pstatsRecMsg->flag24H = pModifiedstats->iUsed24Hour;
		}
	else 
		return STATS_ERR;
	
	pstatsRecMsg->bucketNum15m= bucketNum;
	pstatsRecMsg->actcode = STATS_15MIN_SET;
	
	aulMsg[3] = (ULONG)pstatsRecMsg;
	lRes = VOS_QueSend(gMQidsStats, aulMsg,  NO_WAIT,MSG_PRI_NORMAL);

	if(VOS_ERROR == lRes)
		{
		ASSERT( 0 );
		VOS_Free((VOID *)pstatsRecMsg);
		pstatsRecMsg = NULL;		
		return iRes;
		}
	return iRes;

}


/**********************************************************
* HisStatsOnuModified
*	说明:该函数用于网管设置统计计数的使能和去使能,以及统计计数的最大历史数据
*	ponId ,onuId
*	bucketNum: 历史统计的最大数量
*	flag15M	  :TRUE -使能15M的历史统计,FALSE : 关闭15M的历史统计
*	flag24H	  :该参数同上.但该参数只有使能15M的功能时该参数的TRUE的设置才有效果.
*
*
*************************************************************/

int HisStatsOnu24HModified(short int ponId, short int onuId, unsigned int bucketNum, BOOL flag24H)
{
	statistic_PON_OID  *pTempstats = gpONUHisStatsHead;
	statistic_PON_OID  *pModifiedstats = NULL;
	int iRes = STATS_OK;
	historyStatsMsg *pstatsRecMsg = NULL;
	long		lRes = 0;
	/*bool		passave_onu = TRUE;*/
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};
	#if 0
	if ((Is_passave_onu ( ponId, 
						  onuId,
						  &passave_onu ) != STATS_OK) || 
		(!passave_onu)) 
	{
		return (STATS_ERR);
	}
	#endif
	/*if (ONUHistoryStatsCounter >= STATS_HIS_MAXONU)
		return (STATS_MAXPON_OUTRANG_ERR);*/
	if (bucketNum > STATS_MAX24HOUR_BUCKETNUM)
		return (STATS_ERR);		
	pstatsRecMsg = (historyStatsMsg *)VOS_Malloc((ULONG)sizeof(historyStatsMsg), MODULE_STATSTICS);
	if (NULL == pstatsRecMsg)
		return STATS_ERR;	
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));	
	pstatsRecMsg->slotId = 0;
	pstatsRecMsg->ponId = ponId;
	pstatsRecMsg->onuId = onuId;
	pstatsRecMsg->statsType = STATS_ONUOBJ_MODIFIED;
	while(pTempstats != NULL)
		{
			if((pTempstats->ponIdx == ponId) && (pTempstats->onuIdx == onuId))
				{
				pModifiedstats = pTempstats;
				break;
				}
			/*pPreModif = pTempstats;*/
			pTempstats = pTempstats->pNext;
		}
	if(pTempstats == NULL)
		{
			pstatsRecMsg->flag15M = FALSE;
			pstatsRecMsg->flag24H = flag24H;		
		}	
	else if (NULL != pModifiedstats)
		{
			pstatsRecMsg->flag15M = pModifiedstats->iUsed15Min;
			pstatsRecMsg->flag24H = flag24H;
		}
	else 
		return STATS_ERR;	
	if (NULL != pModifiedstats)
		{

		}

	pstatsRecMsg->bucketNum24h= bucketNum;
	pstatsRecMsg->actcode = STATS_24HOUR_SET;
	
	aulMsg[3] = (ULONG)pstatsRecMsg;
	lRes = VOS_QueSend(gMQidsStats, aulMsg,  NO_WAIT,MSG_PRI_NORMAL);

	if(VOS_ERROR == lRes)
		{
		ASSERT( 0 );
		VOS_Free((VOID *)pstatsRecMsg);
		pstatsRecMsg = NULL;		
		return iRes;
		}
	return 0;
}

/**********************************************
* HisStatsOnuCtrlTablePonIdGet
* 描述 : 该函数从特定的监控端口表项中获取slotid和ponid
*
*
*
*
*************************************************/
STATUS HisStatsOnuCtrlTablePonIdGet(statistic_PON_OID *pHisStatsPon, short int *pSlotId, short int *pPonId, short int *pOnuId)
{
	if (pHisStatsPon== NULL)
		return (STATS_POINT_NULL_ERR);
	*pSlotId = pHisStatsPon->slotIdx;
	*pPonId = pHisStatsPon->ponIdx;
	*pOnuId = pHisStatsPon->onuIdx;
	return (STATS_OK);
}


/**********************************************
* HisStatsOnuCtrlEntryGet
* 描述 : 该函数从以ponid为条件获取该ponid下的24hour的历史统计链表头地址。
*
* 输入 : slotid,ponid
* 输出 : ppHisStatsChainHead - 该ponid的24Hour历史统计消息的链表头地址
*
*************************************************/
STATUS HisStatsOnuCtrlEntryGet(short int SlotId, short int PonId,short int OnuId, statistc_HISTORY_CHAIN **ppHisStats24HHead, statistc_HISTORY_CHAIN **ppHisStats15MHead)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	while(pstatsPon != NULL)
		{
			if ((PonId == pstatsPon->ponIdx) && (OnuId == pstatsPon->onuIdx))
				break;
			else
				pstatsPon = pstatsPon->pNext;
		}
	if (pstatsPon == NULL)
		return (STATS_NOFINE_CHAIN_ERR);
	
	*ppHisStats24HHead = pstatsPon->pstats24HourHead;
	*ppHisStats15MHead = pstatsPon->pstats15MinHead;
	return (STATS_OK);
}


/*还没改完*/
/****************************************************
* HisStatsOnuCtrlTableFirstIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,statsType
*
*	输出: 
*
*	返回: 
******************************************************/
STATUS HisStatsOnuCtrlTableFirstIndexGet(/*short int SlotId, short int PonId,*/ VOID **ppStatsIdx)
{
	statistic_PON_OID	*pstatsPon = gpONUHisStatsHead ;
	if (NULL == gpONUHisStatsHead)
		return STATS_POINT_NULL_ERR ;
	/*while ((pstatsPon != NULL) && (pstatsPon->ponId == PonId))
		{
			pstatsPon = pstatsPon->pNext;
		}*/
	if (pstatsPon == NULL)
		return (STATS_NOFINE_CHAIN_ERR);
	*ppStatsIdx = pstatsPon;
	return (STATS_OK);
}

/****************************************************
* HisStatsOnuCtrlTableNextIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*	输入: soltid,ponid,onuid,statsType
*	输出: 
*	返回: 
******************************************************/
STATUS HisStatsOnuCtrlTableNextIndexGet(/*short int SlotId, short int PonId,*/ statistic_PON_OID *pstatsPon, statistic_PON_OID **ppStatsIdx)
{
	if (pstatsPon->pNext == NULL)
		return (STATS_END_OBJ_ERR);
	*ppStatsIdx = pstatsPon->pNext;
	return (STATS_OK);
}

/****************************************************
* HisStatsOnuCtrlTableGet
* 描述: 该函数用于读取所有的历史统计对象的具体值
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*	输入: soltid,ponid,onuid,StatsObjType - 获取统计计数的类型
*	输出:
*	返回: 
******************************************************/
STATUS HisStatsOnuCtrlTableGet(/*short int SlotId, short int PonId, */statistic_PON_OID	*pstatsPon, statistic_PON_OID **ppStatsData)
{
	if (pstatsPon == NULL)
		return (STATS_NOFINE_CHAIN_ERR);
	*ppStatsData = pstatsPon;
	return (STATS_OK);
}

/****************************************************
* HisStatsOnuCtrlTableFirstIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*	输入: soltid,ponid,onuid,statsType
*	输出: 
*	返回: 
******************************************************/
STATUS HisStatsOnu15MTableFirstIndexGet(statistic_PON_OID **ppStatsOnuIdx, statistc_HISTORY_CHAIN **ppStatsChainIdx)
{
	statistic_PON_OID	*pstatsOnu = gpONUHisStatsHead ;
	if (NULL == pstatsOnu)
		return STATS_POINT_NULL_ERR ;

	if (pstatsOnu->iUsed15Min)
		*ppStatsOnuIdx = pstatsOnu;
	else{/*如果当前pon对象的该15分钟的历史统计并没有使能,
		则循环取下一个pon对象,直到下一个pon使能该15分钟的历史统计*/
			while ((pstatsOnu != NULL) && (!(pstatsOnu->iUsed15Min)))
				pstatsOnu = pstatsOnu->pNext;
			
			if (NULL == pstatsOnu)
				return STATS_ERR;
			if (!(pstatsOnu->iUsed15Min))
				return STATS_ERR;
			*ppStatsOnuIdx = pstatsOnu;
		}

	*ppStatsChainIdx = pstatsOnu->pCurrent15M;
	
	return (STATS_OK);
}


/****************************************************
* HisStatsPonCtrlTableNextIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*	输入: soltid,ponid,onuid,statsType
*	输出: 
*	返回: 
******************************************************/
STATUS HisStatsOnu15MTableNextIndexGet(statistic_PON_OID *pstatOnu, statistc_HISTORY_CHAIN *pStatsChain, 
								statistic_PON_OID **ppstatOnuIdx, statistc_HISTORY_CHAIN **ppStatsChainIdx)
{
	statistic_PON_OID *pstatTemp = NULL;
	
	if (pstatOnu->pstats15MinHead== pStatsChain)
		{
			if (pstatOnu->pNext == NULL)
				return (STATS_END_OBJ_ERR);
			pstatTemp = pstatOnu->pNext;
			/*如果当前pon对象的15分钟的历史统计并没有使能,则循环
			取下一个pon对象,直到下一个pon使能该15分钟的历史统计*/
			while ((pstatTemp != NULL) && (!(pstatTemp->iUsed15Min)))
				pstatTemp = pstatTemp->pNext;
			
			if (NULL == pstatTemp)
				return (STATS_END_OBJ_ERR);
			if (!(pstatTemp->iUsed15Min))
				return (STATS_END_OBJ_ERR);
			
			*ppstatOnuIdx = pstatTemp;
			*ppStatsChainIdx = pstatTemp->pCurrent15M;
		}
	else
		{
			*ppstatOnuIdx = pstatOnu;
			*ppStatsChainIdx = pStatsChain->pPre;
		}
	return (STATS_OK);
}

/****************************************************
* HisStatsPonCtrlTableNextIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*	输入: soltid,ponid,onuid,statsType
*	输出: 
*	返回: 
******************************************************/
STATUS CliHisStatsOnu15MTableNextIndexGet(statistic_PON_OID *pstatOnu, statistc_HISTORY_CHAIN *pStatsChain, 
								statistic_PON_OID **ppstatOnuIdx, statistc_HISTORY_CHAIN **ppStatsChainIdx)
{
	if ( (pstatOnu->pstats15MinHead == pStatsChain) || (FALSE == pStatsChain->bstatus) )
		{
			return (STATS_END_OBJ_ERR);
		}
	else
		{
			*ppstatOnuIdx = pstatOnu;
			*ppStatsChainIdx = pStatsChain->pPre;
		}

	return (STATS_OK);
}


/****************************************************
* HisStatsOnuStatsTableGet
* 描述: 该函数用于读取所有的历史统计对象的具体值
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*	输入: soltid,ponid,onuid,StatsObjType - 获取统计计数的类型
*	输出: 
*	返回: 
******************************************************/
STATUS HisStatsOnu15MTableGet(short int StatsObjType,statistc_HISTORY_CHAIN *pstatsIdx, unsigned long *pStatsData)
{
	statistc_HISTORY_CHAIN	*pstatsPon = pstatsIdx ;
	if (NULL == pstatsPon)
		return STATS_POINT_NULL_ERR ;
	if (NULL == pStatsData)
		return STATS_POINT_NULL_ERR ;
	
	switch (StatsObjType)
		{
		case STATS_DROPEVENTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonDropEvents ;
			break;
		case STATS_OCTETS_TYPE:
			*pStatsData = (unsigned long)pstatsPon->hisStats.hisMonOctets ;
			break;
		case STATS_PKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonPkts ;
			break;
		case STATS_BROADCASTPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonBroadcastPkts ;
			break;
		case STATS_MULTICASTPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonMulticastPkts ;
			break;
		case STATS_CRCALIGNERR_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonCRCAlignErrors ;
			break;
		case STATS_UNDERSIZEPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonUndersizePkts ;
			break;
		case STATS_OVERSIZEPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonOversizePkts ;
			break;
		case STATS_FRAGMENTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonFragments ;
			break;
		case STATS_JABBERS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonJabbers ;
			break;
		case STATS_COLLISIONS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonCollisions ;
			break;
		default:
			return (STATS_INVALUE_OBJ_ERR);
		}
	return (STATS_OK);
}



/****************************************************
* HisStatsOnu24HTableFirstIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*	输入: soltid,ponid,onuid,statsType
*
*	输出: ppstatPonIdx指向当前ponid的控制表项
*			ppStatsChainIdx - 当前ponid的历史统计数据表的头地址
*
*	返回: 
******************************************************/
STATUS HisStatsOnu24HTableFirstIndexGet(statistic_PON_OID **ppstatPonIdx ,statistc_HISTORY_CHAIN **ppStatsChainIdx)
{
	statistic_PON_OID	*pstatsOnu = gpONUHisStatsHead ;
	if (NULL == pstatsOnu)
		return STATS_POINT_NULL_ERR ;
	if (pstatsOnu->iUsed24Hour)
		*ppstatPonIdx = pstatsOnu;
	else{/*如果当前pon对象的该24Hour的历史统计并没有使能,
		则循环取下一个pon对象,直到下一个pon使能该24Hour的历史统计*/
			while ((pstatsOnu != NULL) && (!(pstatsOnu->iUsed24Hour)))
				pstatsOnu = pstatsOnu->pNext;
			
			if (NULL == pstatsOnu)
				return STATS_ERR;
			if (!(pstatsOnu->iUsed24Hour))
				return STATS_ERR;
			*ppstatPonIdx = pstatsOnu;
		}
	
	/*modified by wutu at July eighted*/
	*ppStatsChainIdx = pstatsOnu->pCurrent24H;
	
	return (STATS_OK);

}

/****************************************************
* HisStatsOnu24HTableNextIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,statsType
*
*	输出: pstatOnuIdx指向当前ponid的控制表项
*			ppStatsIdx - 当前ponid的历史统计数据表的下地址
*
*	返回: 
******************************************************/
STATUS HisStatsOnu24HTableNextIndexGet(statistic_PON_OID *pstatOnu, statistc_HISTORY_CHAIN *pStatsChain, 
								statistic_PON_OID **ppstatOnuIdx, statistc_HISTORY_CHAIN **ppStatsChainIdx)
{
	statistic_PON_OID *pstatTemp = NULL;

	if (pstatOnu->pstats24HourHead == pStatsChain)
		{
			if (pstatOnu->pNext == NULL)
				return (STATS_END_OBJ_ERR);
			pstatTemp = pstatOnu->pNext;
		
			/*如果当前pon对象的24Hour的历史统计并没有使能,则循环
			取下一个pon对象,直到下一个pon使能该24Hour的历史统计*/
			while ((pstatTemp != NULL) && (!(pstatTemp->iUsed24Hour)))
				pstatTemp = pstatTemp->pNext;
			
			if (NULL == pstatTemp)
				return (STATS_END_OBJ_ERR);
			if (!(pstatTemp->iUsed24Hour))
				return (STATS_END_OBJ_ERR);	
			*ppstatOnuIdx = pstatTemp;
			*ppStatsChainIdx = pstatTemp->pCurrent24H;
		}
	else
		{
			*ppstatOnuIdx = pstatOnu;
			*ppStatsChainIdx = pStatsChain->pPre;
		}
	return (STATS_OK);
}


/****************************************************
* CliHisStatsOnu24HTableNextIfdexGet
* 描述: 该函数用于读取所有的历史统计对象的索引
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,statsType
*
*	输出: pstatOnuIdx指向当前ponid的控制表项
*			ppStatsIdx - 当前ponid的历史统计数据表的下地址
*
*	返回: 
******************************************************/
STATUS CliHisStatsOnu24HTableNextIndexGet(statistic_PON_OID *pstatOnu, statistc_HISTORY_CHAIN *pStatsChain, 
								statistic_PON_OID **ppstatOnuIdx, statistc_HISTORY_CHAIN **ppStatsChainIdx)
{
	if ( (pstatOnu->pstats24HourHead == pStatsChain) || (FALSE == pStatsChain->bstatus) )
		{
			return (STATS_END_OBJ_ERR);
		}
	else
		{
			*ppstatOnuIdx = pstatOnu;
			*ppStatsChainIdx = pStatsChain->pPre;
		}
	
	return (STATS_OK);
}



/****************************************************
* HisStatsOnu24HTableGet
* 描述: 该函数用于读取所有的历史统计对象的具体值
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,StatsObjType - 获取统计计数的类型
*			pstatsIdx - 当前pon的历史统计表的表项地址
*	输出: pStatsData - 当前所要获取的历史统计
*
*	返回: 
******************************************************/
STATUS HisStatsOnu24HTableGet(short int StatsObjType,statistc_HISTORY_CHAIN *pstatsIdx, unsigned long *pStatsData)
{
	statistc_HISTORY_CHAIN *pstatsPon = pstatsIdx ;
	if (NULL == pstatsPon)
		return STATS_POINT_NULL_ERR ;
	if (NULL == pStatsData)
		return STATS_POINT_NULL_ERR ;	
	
	switch (StatsObjType)
		{
		case STATS_DROPEVENTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonDropEvents ;
			break;
		case STATS_OCTETS_TYPE:
			*pStatsData = (unsigned long)(pstatsPon->hisStats.hisMonOctets);
			break;
		case STATS_PKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonPkts ;
			break;
		case STATS_BROADCASTPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonBroadcastPkts ;
			break;
		case STATS_MULTICASTPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonMulticastPkts ;
			break;
		case STATS_CRCALIGNERR_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonCRCAlignErrors ;
			break;
		case STATS_UNDERSIZEPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonUndersizePkts ;
			break;
		case STATS_OVERSIZEPKTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonOversizePkts ;
			break;
		case STATS_FRAGMENTS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonFragments ;
			break;
		case STATS_JABBERS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonJabbers ;
			break;
		case STATS_COLLISIONS_TYPE:
			*pStatsData = pstatsPon->hisStats.hisMonCollisions ;
			break;
		default:
			return (STATS_INVALUE_OBJ_ERR);
		}
	return (STATS_OK);
}


/***********************************************
* HisStats15MinMaxRecordSet
*
* decription : 该函数针对用户设置的时间参数,对已经使能进行统计的对象
*			分别修改.
* input	:   bucket	,
*
************************************************/
STATUS HisStats15MinMaxRecordSet(unsigned short bucket)
{
	unsigned short oldRecordNum = 0;
	historyStatsMsg *pstatsRecMsg; 
	long		lRes = 0;
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};	
	
	/*syscle time can not be greater than two days*/
	/* if (sys_time > (STATS_MAX15MIN_BUCKETNUM*15))
		return (STATS_ERR); */
	 if (bucket > (STATS_MAX15MIN_BUCKETNUM))
		return (STATS_ERR); 	 
	oldRecordNum = gHis15MinMaxRecord;
	gHis15MinMaxRecord = bucket;

	pstatsRecMsg = (historyStatsMsg *)VOS_Malloc(sizeof(historyStatsMsg), MODULE_STATSTICS);
	if (NULL == pstatsRecMsg)
		return STATS_ERR;
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));
	pstatsRecMsg->statsType = STATS_RECORD_MODIFIED;	
	pstatsRecMsg->flag15M = 1;
	pstatsRecMsg->bucketNum15m = gHis15MinMaxRecord;
	pstatsRecMsg->bcktNum15mOld = oldRecordNum;
	/*pstatsRecMsg->bucketNum = gHis15MinMaxRecord;*/
	/*pstatsRecMsg->userd = oldRecordNum;*/

	aulMsg[3] = (ULONG)(pstatsRecMsg);
	/*sys_console_printf(" sendTimerOut!\r\n");*/
	lRes = VOS_QueSend(gMQidsStats, aulMsg, NO_WAIT,MSG_PRI_NORMAL);

	if (VOS_ERROR == lRes)
		{
		ASSERT( 0 );
		VOS_Free((VOID *) pstatsRecMsg);
		pstatsRecMsg = NULL;
		return VOS_ERROR;
		}

	return STATS_OK;	
}
/***********************************************
* HisStats15MinMaxRecordSet
*
* decription : 该函数针对用户设置的时间参数,对已经使能进行统计的对象
*			分别修改.
* input	:   sys_time	,时间值,	单位为秒
*
************************************************/
STATUS HisStats24HoursMaxRecordSet(unsigned short sys_time)
{
	unsigned short oldRecordNum = 0;
	historyStatsMsg *pstatsRecMsg; 
	long		lRes = 0;
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};
	/*该值不能大于14个小时*/
	/*if (value>48)
		return STATS_ERR;*/
	oldRecordNum = gHis24HoursMaxRecord;
	gHis24HoursMaxRecord = sys_time;
	/*syscle time can not be greater than third days*/
	 if (sys_time > (STATS_MAX24HOUR_BUCKETNUM/**24*/))
		return (STATS_ERR); 	
	pstatsRecMsg = (historyStatsMsg *)VOS_Malloc(sizeof(historyStatsMsg), MODULE_STATSTICS);
	if (NULL == pstatsRecMsg)
		return STATS_ERR;
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));
	pstatsRecMsg->statsType = STATS_RECORD_MODIFIED;	
	pstatsRecMsg->flag24H= 1;
	pstatsRecMsg->bucketNum24h = gHis24HoursMaxRecord;
	pstatsRecMsg->bcktNum24hOld = oldRecordNum;
	/*pstatsRecMsg->userd = oldRecordNum;*/

	aulMsg[3] = (ULONG)(pstatsRecMsg);
	/*sys_console_printf(" sendTimerOut!\r\n");*/
	lRes = VOS_QueSend(gMQidsStats, aulMsg, NO_WAIT,MSG_PRI_NORMAL);

	if (VOS_ERROR == lRes)
		{
		ASSERT( 0 );
		VOS_Free((VOID *) pstatsRecMsg);
		pstatsRecMsg = NULL;
		return STATS_OK;
		}

	return STATS_OK;	
	
}

/*返回值为bucknum,而非是时间*/
STATUS HisStats24HoursMaxRecordGet(unsigned int *pValue)
{
	unsigned int bucketNum = 0;
	if (NULL == pValue)
		return STATS_ERR;
	/*该值不能大于14个小时*/
	bucketNum = gHis24HoursMaxRecord;
	*pValue = bucketNum;
	return bucketNum;
}

STATUS HisStats24HoursDefaultRecordGet(unsigned int *pValue)
{
	unsigned int bucketNum = 0;
	if (NULL == pValue)
		return STATS_ERR;
	/*该值不能大于14个小时*/
	bucketNum = STATS_DEFAULT24H_BUCKETNUM;
	*pValue = bucketNum;
	return bucketNum;
}

/*返回值为bucknum,而非是时间*/
STATUS HisStats15MinMaxRecordGet(unsigned int *pValue)
{
	unsigned int bucketNum = 0;
	if (NULL == pValue)
		return STATS_ERR;
	/*该值不能大于14个小时*/
	bucketNum = gHis15MinMaxRecord;
	*pValue = bucketNum;
	return bucketNum;
}

/*返回值为bucknum,而非是时间*/
STATUS HisStats15MinDefaultRecordGet(unsigned int *pValue)
{
	unsigned int bucketNum = 0;
	if (NULL == pValue)
		return STATS_ERR;
	/*该值不能大于14个小时*/
	bucketNum = STATS_DEFAULT15MIN_BUCKETNUM;
	*pValue = bucketNum;
	return bucketNum;
}

#if 0
STATUS CliHisStatsMaxRecordShow(void)
{
	unsigned int syscle_time24H = 0;
	unsigned int syscle_time15M = 0;
	HisStats24HoursMaxRecordGet( &syscle_time24H );
	HisStats15MinMaxRecordGet( &syscle_time15M );
	sys_console_printf("\r\n  interval type          bucket number");
	sys_console_printf("  15Min                  %d\r\n", syscle_time15M);
	sys_console_printf("  24h                    %d\r\n\r\n", syscle_time24H);
	return STATS_OK;
}

STATUS CliStatsPonDataParse(unsigned int statsType ,unsigned int statsData)
{
	switch(statsType)
			{
				case STATS_DROPEVENTS_TYPE:
					sys_console_printf("  hisMonDropEvents : %ld\r\n", statsData);
					
					break;
				case STATS_OCTETS_TYPE:
					sys_console_printf("  hisMonOctets : %ld\r\n", statsData);
					
					break;
				case STATS_PKTS_TYPE:
					sys_console_printf("  hisMonPkts : %ld\r\n", statsData);
					
					break;
				case STATS_BROADCASTPKTS_TYPE:
					sys_console_printf("  hisMonBroadcastPkts : %ld\r\n", statsData);
					
					break;
				case STATS_MULTICASTPKTS_TYPE:
					sys_console_printf("  hisMonMulticastPkts : %ld\r\n", statsData);
					
					break;
				case STATS_CRCALIGNERR_TYPE:
					sys_console_printf("  hisMonCRCAlignErrors : %ld\r\n", statsData);
					
					break;
				case STATS_UNDERSIZEPKTS_TYPE:
					sys_console_printf("  hisMonUndersizePkts : %ld\r\n", statsData);
					
					break;
				case STATS_OVERSIZEPKTS_TYPE:
					sys_console_printf("  hisMonOversizePkts : %ld\r\n", statsData);
					
					break;
				case STATS_FRAGMENTS_TYPE:
					sys_console_printf("  hisMonFragments : %ld\r\n", statsData);
					
					break;
				case STATS_JABBERS_TYPE:
					sys_console_printf("  hisMonJabbers : %ld\r\n", statsData);
					break;
				case STATS_COLLISIONS_TYPE:
					sys_console_printf("  hisMonCollisions : %ld\r\n", statsData);
					break;
				case STATS_PKT64OCTETS:
					sys_console_printf("  hisPkt64Octets : %ld\r\n", 0);
					break;
				case STATS_PKT65TO127OCTETS:
					sys_console_printf("  hisPkt65To127Octets : %ld\r\n", 0);
					break;
				case STATS_PKT128TO255OCTETS:
					sys_console_printf("  hisPkt128To255Octets : %ld\r\n", 0);
					break;
				case STATS_PKT256TO511OCTETS:
					sys_console_printf("  hisPkt256To511Octets : %ld\r\n", 0);
					break;
				case STATS_PKT512TO1023OCTETS:
					sys_console_printf("  hisPkt512To1023Octets : %ld\r\n", 0);
					break;
				case STATS_PKT1024TO1518OCTETS:
					sys_console_printf("  hisPkt1024To1518Octets : %ld\r\n", 0);
					break;				
				default:
					return (STATS_INVALUE_OBJ_ERR);
			}
	return STATS_OK;
}

STATUS CliHisStats15MinDataShow(unsigned short ponId)
{
	long lRet = 0;
	int count = 0;
	int cliFlag = STATS_ERR;
	unsigned long StatsData = 0;
	statistic_PON_OID *pstatPonIdx = NULL ;
	statistic_PON_OID *pstatPonCurrent = NULL;
	statistc_HISTORY_CHAIN *pStatsChainIdx = NULL ;
	statistic_PON_OID *pNextstatPonIdx = NULL;
	statistc_HISTORY_CHAIN *pNextStatsChainIdx = NULL;
	/*搜索该ponId 并判断该ponid是否已经使能15min性能统计*/
	
	pstatPonCurrent = gpPONHisStatsHead;
	while(pstatPonCurrent != NULL)
		{
			if ( ponId == pstatPonCurrent->ponIdx)
			{/*两种判断效果: 当找到该ponid 时，如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
				if(pstatPonCurrent->iUsed15Min)
				{
				cliFlag = STATS_OK;
				break;					
				}
				else
					return cliFlag;
			}
			
			pstatPonCurrent = pstatPonCurrent->pNext;
		}

	if (STATS_OK == cliFlag)
	{
		if (pstatPonCurrent == NULL)
			return STATS_ERR;
		pStatsChainIdx = pstatPonCurrent->pCurrent15M;
		pstatPonIdx = pstatPonCurrent;
		pNextstatPonIdx = pstatPonIdx;
		
		/*for (count = 1;count<12;count++)
			{
				StatsData = 0;
				HisStatsPon15MTableGet(count, pStatsChainIdx, &StatsData);
				CliStatsPonDataParse(count, StatsData);
			}	*/
	}
	else
		return STATS_ERR;
	
	while(pNextstatPonIdx == pstatPonIdx)
	{
		for (count = 1;count<12;count++)
			{
				StatsData = 0;
				HisStatsPon15MTableGet(count, pStatsChainIdx, &StatsData);
				CliStatsPonDataParse(count, StatsData);
			}
		
		pNextstatPonIdx = NULL;
		pNextStatsChainIdx = NULL;
		lRet = HisStatsPon15MTableNextIndexGet(pstatPonIdx, 
								pStatsChainIdx, 
								&pNextstatPonIdx, 
								&pNextStatsChainIdx);
		if (STATS_OK != lRet)
			return lRet;	
		pStatsChainIdx = pNextStatsChainIdx;
		
	}
	pNextstatPonIdx = NULL;
	pstatPonIdx = NULL;
	pstatPonCurrent = NULL;
	pStatsChainIdx = NULL;
	return STATS_OK;
}

STATUS CliHisStats24HourDataShow(unsigned short ponId)
{
	long lRet = 0;
	int count = 0;
	int cliFlag = STATS_ERR;
	unsigned long StatsData = 0;
	statistic_PON_OID *pstatPonIdx = NULL ;
	statistic_PON_OID *pstatPonCurrent = NULL;
	statistc_HISTORY_CHAIN *pStatsChainIdx = NULL ;
	statistic_PON_OID *pNextstatPonIdx = NULL;
	statistc_HISTORY_CHAIN *pNextStatsChainIdx = NULL;
	/*搜索该ponId 并判断该ponid是否已经使能15min性能统计*/
	
	pstatPonCurrent = gpPONHisStatsHead;
	while(pstatPonCurrent != NULL)
		{
			if ( ponId == pstatPonCurrent->ponIdx)
			{/*两种判断效果: 当找到该ponid 时，如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
				if(pstatPonCurrent->iUsed24Hour)
				{
				cliFlag = STATS_OK;
				break;					
				}
				else
					return cliFlag;
			}
			
			pstatPonCurrent = pstatPonCurrent->pNext;
		}
	if (STATS_OK == cliFlag)
	{
		if (pstatPonCurrent == NULL)
			return STATS_ERR;
		pStatsChainIdx = pstatPonCurrent->pCurrent24H;
		pstatPonIdx = pstatPonCurrent;
		pNextstatPonIdx = pstatPonIdx;

	}
	else
		return STATS_ERR;

	while(pNextstatPonIdx == pstatPonIdx)
	{
		for (count = 1;count<12;count++)
			{
				StatsData = 0;
				HisStatsPon24HTableGet(count, pStatsChainIdx, &StatsData);
				CliStatsPonDataParse(count, StatsData);
			}
		/*pStatsChainIdx = pNextstatPonIdx;*/
		pNextstatPonIdx = NULL;
		pNextStatsChainIdx = NULL;
		lRet = HisStatsPon24HTableNextIndexGet(pstatPonIdx, 
								pStatsChainIdx, 
								&pNextstatPonIdx, 
								&pNextStatsChainIdx);
		if (STATS_OK != lRet)
			return lRet;		
		pStatsChainIdx = pNextStatsChainIdx;
	}
	pNextstatPonIdx = NULL;
	pstatPonIdx = NULL;
	pstatPonCurrent = NULL;
	pStatsChainIdx = NULL;
	return STATS_OK;
	
}



STATUS CliHisStatsOnu15MinDataShow(unsigned short ponId,  unsigned short onuId)
{
	long lRet = 0;
	int count = 0;
	int cliFlag = STATS_ERR;
	unsigned long StatsData = 0;
	statistic_PON_OID *pstatOnuIdx = NULL ;
	statistic_PON_OID *pstatOnuCurrent = NULL;
	statistc_HISTORY_CHAIN *pStatsChainIdx = NULL ;
	statistic_PON_OID *pNextstatOnuIdx = NULL;
	statistc_HISTORY_CHAIN *pNextStatsChainIdx = NULL;
	/*搜索该ponId 并判断该ponid是否已经使能15min性能统计*/
	
	pstatOnuCurrent = gpONUHisStatsHead;
	while(pstatOnuCurrent != NULL)
		{
			if( ( ponId == pstatOnuCurrent->ponIdx) && (onuId == pstatOnuCurrent->onuIdx) )
			{/*两种判断效果: 当找到该ponid 时，如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
				if(pstatOnuCurrent->iUsed15Min)
				{
				cliFlag = STATS_OK;
				break;					
				}
				else
					return cliFlag;
			}
			
			pstatOnuCurrent = pstatOnuCurrent->pNext;
		}

	if (STATS_OK == cliFlag)
	{
		if (pstatOnuCurrent == NULL)
			return STATS_ERR;
		pStatsChainIdx = pstatOnuCurrent->pCurrent15M;
		pstatOnuIdx = pstatOnuCurrent;
		pNextstatOnuIdx = pstatOnuIdx;
		
		/*for (count = 1;count<12;count++)
			{
				StatsData = 0;
				HisStatsPon15MTableGet(count, pStatsChainIdx, &StatsData);
				CliStatsPonDataParse(count, StatsData);
			}	*/
	}
	else
		return STATS_ERR;
	
	while(pNextstatOnuIdx == pstatOnuIdx)
	{
		for (count = 1;count<12;count++)
			{
				StatsData = 0;
				HisStatsOnu15MTableGet(count, pStatsChainIdx, &StatsData);
				CliStatsPonDataParse(count, StatsData);
			}
		/*pStatsChainIdx = pNextstatOnuIdx;*/
		pNextstatOnuIdx = NULL;
		pNextStatsChainIdx = NULL;
		lRet = HisStatsOnu15MTableNextIndexGet(pstatOnuIdx, 
								pStatsChainIdx, 
								&pNextstatOnuIdx, 
								&pNextStatsChainIdx);
		pStatsChainIdx = pNextStatsChainIdx;
		if (STATS_OK != lRet)
			return lRet;		
	}
	pNextStatsChainIdx = NULL;
	pstatOnuIdx = NULL;
	pstatOnuCurrent = NULL;
	pStatsChainIdx = NULL;
	return STATS_OK;
}


STATUS CliHisStatsOnu24HourDataShow(unsigned short ponId, unsigned short onuId)
{
	long lRet = 0;
	int count = 0;
	int cliFlag = STATS_ERR;
	unsigned long StatsData = 0;
	statistic_PON_OID *pstatOnuIdx = NULL ;
	statistic_PON_OID *pstatOnuCurrent = NULL;
	statistc_HISTORY_CHAIN *pStatsChainIdx = NULL ;
	statistc_HISTORY_CHAIN *pNextStatsChainIdx = NULL;
	/*搜索该ponId 并判断该ponid是否已经使能15min性能统计*/
	
	pstatOnuCurrent = gpPONHisStatsHead;
	while(pstatOnuCurrent != NULL)
		{
			if ( ponId == pstatOnuCurrent->ponIdx)
			{/*两种判断效果: 当找到该ponid 时，如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
				if(pstatOnuCurrent->iUsed24Hour)
				{
				cliFlag = STATS_OK;
				break;					
				}
				else
					return cliFlag;
			}
			
			pstatOnuCurrent = pstatOnuCurrent->pNext;
		}
	if (STATS_OK == cliFlag)
	{
		if (pstatOnuCurrent == NULL)
			return STATS_ERR;
		pStatsChainIdx = pstatOnuCurrent->pCurrent24H;
		pstatOnuIdx = pstatOnuCurrent;
		pstatOnuCurrent = pstatOnuIdx;

	}
	else
		return STATS_ERR;

	while(pstatOnuCurrent == pstatOnuIdx)
	{
		for (count = 1;count<12;count++)
			{
				StatsData = 0;
				HisStatsOnu24HTableGet(count, pStatsChainIdx, &StatsData);
				CliStatsPonDataParse(count, StatsData);
			}
		/*pStatsChainIdx = pstatOnuCurrent;*/
		pstatOnuCurrent = NULL;
		pNextStatsChainIdx = NULL;
		lRet = HisStatsOnu24HTableNextIndexGet(pstatOnuIdx, 
								pStatsChainIdx, 
								&pstatOnuCurrent, 
								&pNextStatsChainIdx);
		if (STATS_OK != lRet)
			return lRet;
		pStatsChainIdx = pNextStatsChainIdx;
	}
	pstatOnuCurrent = NULL;
	pstatOnuIdx = NULL;
	pStatsChainIdx = NULL;
	return STATS_OK;
	
}



STATUS CliStatsPonDataParseVty(unsigned int statsType ,unsigned int statsData, struct vty* vty)
{
	switch(statsType)
	{
				case STATS_DROPEVENTS_TYPE:
					vty_out( vty, "  History DropEvents          : %lu\r\n", statsData);
					break;
				case STATS_OCTETS_TYPE:
					vty_out( vty, "  History RevOctets           : %lu\r\n", statsData);
					break;
				case STATS_PKTS_TYPE:
					vty_out( vty, "  History RevPackets          : %lu\r\n", statsData);
					break;
				case STATS_BROADCASTPKTS_TYPE:
					vty_out( vty, "  History RevBroadcastPkts    : %lu\r\n", statsData);
					break;
				case STATS_MULTICASTPKTS_TYPE:
					vty_out( vty, "  History MulticastPkts       : %lu\r\n", statsData);
					break;
				case STATS_CRCALIGNERR_TYPE:
					vty_out( vty, "  History CRCAlignErrors      : %lu\r\n", statsData);
					break;
				case STATS_UNDERSIZEPKTS_TYPE:
					vty_out( vty, "  History UndersizePkts       : %lu\r\n", statsData);	
					break;
				case STATS_OVERSIZEPKTS_TYPE:
					vty_out( vty, "  History OversizePkts        : %lu\r\n", statsData);
					break;
				case STATS_FRAGMENTS_TYPE:
					vty_out( vty, "  History Fragments           : %lu\r\n", statsData);
					break;
				case STATS_JABBERS_TYPE:
					vty_out( vty, "  History Jabbers             : %lu\r\n", statsData);
					break;
				case STATS_COLLISIONS_TYPE:
					vty_out( vty, "  History Collisions          : %lu\r\n", statsData);
					break;
				case STATS_PKT64OCTETS:
					vty_out( vty, "  History Pkt64Octets         : %lu\r\n", statsData);
					break;
				case STATS_PKT65TO127OCTETS:
					vty_out( vty, "  History Pkt65To127Octets    : %lu\r\n", statsData);
					break;
				case STATS_PKT128TO255OCTETS:
					vty_out( vty, "  History Pkt128To255Octets   : %lu\r\n", statsData);
					break;
				case STATS_PKT256TO511OCTETS:
					vty_out( vty, "  History Pkt256To511Octets   : %lu\r\n", statsData);
					break;
				case STATS_PKT512TO1023OCTETS:
					vty_out( vty, "  History Pkt512To1023Octets  : %lu\r\n", statsData);
					break;
				case STATS_PKT1024TO1518OCTETS:
					vty_out( vty, "  History Pkt1024To1518Octets : %lu\r\n", statsData);
					break;				
				default:
					return (STATS_INVALUE_OBJ_ERR);
	}
	return STATS_OK;
}
#endif

/****************************************************
* HisStatsOnuStatsTableGet
* 描述: 该函数用于读取所有的历史统计对象的具体值
*		statsType 为端口对象: 1 - 24 hour
*								2 - 15 minute
*		
*
*	输入: soltid,ponid,onuid,StatsObjType - 获取统计计数的类型
*
*	输出: 
*
*	返回: 
******************************************************/
STATUS CliHisStatsDataVty(statistc_HISTORY_CHAIN *pstatsIdx,struct vty *vty)
{
	statistc_HISTORY_CHAIN	*pstatsPon = pstatsIdx ;
	unsigned long statsData = 0;
	unsigned long long octetsData = 0;
	char statbuf[256] = {0};
	int StatsObjType = 0;
	if (NULL == pstatsPon)
		return STATS_POINT_NULL_ERR ;
	
	for(StatsObjType = 0;StatsObjType<12; StatsObjType++)
	{
		char szText[64]="";
		statsData = 0;
		switch (StatsObjType)
		{		
		case STATS_DROPEVENTS_TYPE:
			statsData = pstatsPon->hisStats.hisMonDropEvents ;
			vty_out( vty, "  History DropEvents : %lu\r\n", statsData);
			break;
		case STATS_OCTETS_TYPE:
			octetsData = pstatsPon->hisStats.hisMonOctets ;
			sprintf64Bits( szText, octetsData );
			sprintf( statbuf, "  History RevOctets : %s\r\n", szText );
			vty_out( vty, "%s",statbuf);
			break;
		case STATS_PKTS_TYPE:
			statsData = pstatsPon->hisStats.hisMonPkts ;
			vty_out( vty, "  History RevPackets : %lu\r\n", statsData);
			break;
		case STATS_BROADCASTPKTS_TYPE:
			statsData = pstatsPon->hisStats.hisMonBroadcastPkts ;
			vty_out( vty, "  History RevBroadcastPkts : %lu\r\n", statsData);
			break;
		case STATS_MULTICASTPKTS_TYPE:
			statsData = pstatsPon->hisStats.hisMonMulticastPkts ;
			vty_out( vty, "  History MulticastPkts : %lu\r\n", statsData);
			break;
		case STATS_CRCALIGNERR_TYPE:
			statsData = pstatsPon->hisStats.hisMonCRCAlignErrors ;
			vty_out( vty, "  History CRCAlignErrors : %lu\r\n", statsData);
			break;
		case STATS_UNDERSIZEPKTS_TYPE:
			statsData = pstatsPon->hisStats.hisMonUndersizePkts ;
			vty_out( vty, "  History UndersizePkts : %lu\r\n", statsData);	
			break;
		case STATS_OVERSIZEPKTS_TYPE:
			statsData = pstatsPon->hisStats.hisMonOversizePkts ;
			vty_out( vty, "  History OversizePkts : %lu\r\n", statsData);
			break;
		case STATS_FRAGMENTS_TYPE:
			statsData = pstatsPon->hisStats.hisMonFragments ;
			vty_out( vty, "  History Fragments : %lu\r\n", statsData);
			break;
		case STATS_JABBERS_TYPE:
			statsData = pstatsPon->hisStats.hisMonJabbers ;
			vty_out( vty, "  History Jabbers : %lu\r\n", statsData);
			break;
		case STATS_COLLISIONS_TYPE:
			statsData = pstatsPon->hisStats.hisMonCollisions ;
			vty_out( vty, "  History Collisions : %lu\r\n", statsData);
			break;
		default:
			break;
		}
	}
	return (STATS_OK);
}



STATUS CliHisStatsOnu15MinDataVty(unsigned short ponId,  unsigned short onuId, unsigned int bucket_num, struct vty* vty)
{
	long lRet = 0;
	unsigned int tempbucket = bucket_num;
	int cliFlag = STATS_ERR;
	statistic_PON_OID *pstatOnuIdx = NULL ;
	statistic_PON_OID *pstatOnuCurrent = NULL;
	statistc_HISTORY_CHAIN *pStatsChainIdx = NULL ;
	statistic_PON_OID *pNextstatOnuIdx = NULL;
	statistc_HISTORY_CHAIN *pNextStatsChainIdx = NULL;
	/*搜索该ponId 并判断该ponid是否已经使能15min性能统计*/
	if (tempbucket == 0)
		return STATS_OK;
	pstatOnuCurrent = gpONUHisStatsHead;
	while(pstatOnuCurrent != NULL)
		{
			if( ( ponId == pstatOnuCurrent->ponIdx) && (onuId == pstatOnuCurrent->onuIdx) )
			{/*两种判断效果: 当找到该ponid 时，如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
				if(pstatOnuCurrent->iUsed15Min)
				{
				cliFlag = STATS_OK;
				break;					
				}
				else
					return cliFlag;
			}
			
			pstatOnuCurrent = pstatOnuCurrent->pNext;
		}

	if (STATS_OK == cliFlag)
	{
		if (NULL == pstatOnuCurrent)
			return STATS_ERR;
		pStatsChainIdx = pstatOnuCurrent->pCurrent15M;
		pstatOnuIdx = pstatOnuCurrent;
		pNextstatOnuIdx = pstatOnuIdx;
	}
	else
		return STATS_ERR;
	
	while(pNextstatOnuIdx == pstatOnuIdx)
	{
		vty_out( vty, "\r\n  %d.%d.%d %d:%d:%d(MillSec %d) :\r\n",pStatsChainIdx->year,\
				pStatsChainIdx->month,pStatsChainIdx->day,\
				pStatsChainIdx->hour,pStatsChainIdx->minute,\
				pStatsChainIdx->second,pStatsChainIdx->MillSecond);
		CliHisStatsDataVty(pStatsChainIdx, vty);
		/*for (count = 1;count<12;count++)
			{
				StatsData = 0;
				HisStatsOnu15MTableGet(count, pStatsChainIdx, &StatsData);
				CliStatsPonDataParseVty(count, StatsData, vty);
			}*/
				vty_out(vty, "  \r\n\r\n");
				
		tempbucket--;
		if(tempbucket == 0)
			break;
		/*pStatsChainIdx = pNextstatOnuIdx;*/
		pNextstatOnuIdx = NULL;
		pNextStatsChainIdx = NULL;
		lRet = CliHisStatsOnu15MTableNextIndexGet(pstatOnuIdx, 
								pStatsChainIdx, 
								&pNextstatOnuIdx, 
								&pNextStatsChainIdx);
		if (STATS_END_OBJ_ERR == lRet)
			return STATS_OK;	
		pStatsChainIdx = pNextStatsChainIdx;
	}
	pNextStatsChainIdx = NULL;
	pstatOnuIdx = NULL;
	pstatOnuCurrent = NULL;
	pStatsChainIdx = NULL;
	return STATS_OK;
}


/*获取onu 24 hour 历史统计并显示，该函数有cli 模块调用*/
STATUS CliHisStatsOnu24HourDataVty(unsigned short ponId, unsigned short onuId, unsigned int bucket_num, struct vty* vty)
{
	long lRet = 0;
	int cliFlag = STATS_ERR;
	unsigned int tempbucket = bucket_num;
	statistic_PON_OID *pstatOnuIdx = NULL ;
	statistic_PON_OID *pstatOnuCurrent = NULL;
	statistc_HISTORY_CHAIN *pStatsChainIdx = NULL ;
	statistc_HISTORY_CHAIN *pNextStatsChainIdx = NULL;
	/*搜索该ponId 并判断该ponid是否已经使能15min性能统计*/

	if(tempbucket == 0)
		return STATS_OK;
	pstatOnuCurrent = gpONUHisStatsHead;
	while(pstatOnuCurrent != NULL)
		{
			if( ( ponId == pstatOnuCurrent->ponIdx) && (onuId == pstatOnuCurrent->onuIdx) )
			/*if ( ponId == pstatOnuCurrent->ponIdx)*/
			{/*两种判断效果: 当找到该ponid 时，如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
				if(pstatOnuCurrent->iUsed24Hour)
				{
				cliFlag = STATS_OK;
				break;					
				}
				else
					return cliFlag;
			}
			
			pstatOnuCurrent = pstatOnuCurrent->pNext;
		}
	if (STATS_OK == cliFlag)
	{
		if (NULL == pstatOnuCurrent)
			return STATS_ERR;
		pStatsChainIdx = pstatOnuCurrent->pCurrent24H;
		pstatOnuIdx = pstatOnuCurrent;
		pstatOnuCurrent = pstatOnuIdx;

	}
	else
		return STATS_ERR;

	while(pstatOnuCurrent == pstatOnuIdx)
	{
		vty_out( vty, "\r\n  %d.%d.%d %d:%d:%d(MillSec %d) :\r\n",pStatsChainIdx->year,\
				pStatsChainIdx->month,pStatsChainIdx->day,\
				pStatsChainIdx->hour,pStatsChainIdx->minute,\
				pStatsChainIdx->second,pStatsChainIdx->MillSecond);
		CliHisStatsDataVty(pStatsChainIdx, vty);
		/*for (count = 1;count<12;count++)
			{
				StatsData = 0;
				HisStatsOnu24HTableGet(count, pStatsChainIdx, &StatsData);
				CliStatsPonDataParseVty(count, StatsData, vty);
			}*/
		vty_out(vty, "  \r\n\r\n");
		tempbucket--;
		if(tempbucket == 0)
			return STATS_OK;
		/*pStatsChainIdx = pstatOnuCurrent;*/
		pstatOnuCurrent = NULL;
		pNextStatsChainIdx = NULL;
		lRet = CliHisStatsOnu24HTableNextIndexGet(pstatOnuIdx, 
								pStatsChainIdx, 
								&pstatOnuCurrent, 
								&pNextStatsChainIdx);
		if (STATS_OK != lRet)
			return STATS_OK;
		pStatsChainIdx = pNextStatsChainIdx;
	}
	pstatOnuCurrent = NULL;
	pstatOnuIdx = NULL;
	pStatsChainIdx = NULL;
	return STATS_OK;
	
}

/*获取pon 15 m 历史统计并显示，该函数有cli 模块调用*/
STATUS CliHisStats15MinDataVty(unsigned short ponId, unsigned int bucket_num, struct vty* vty)
{
	long lRet = 0;
	int cliFlag = STATS_ERR;
	unsigned int tempbucket = bucket_num;
	unsigned int maxBucketNum15m = 0;
	statistic_PON_OID *pstatPonIdx = NULL ;
	statistic_PON_OID *pstatPonCurrent = NULL;
	statistc_HISTORY_CHAIN *pStatsChainIdx = NULL ;
	statistic_PON_OID *pNextstatPonIdx = NULL;
	statistc_HISTORY_CHAIN *pNextStatsChainIdx = NULL;
	/*搜索该ponId 并判断该ponid是否已经使能15min性能统计*/

	if (tempbucket == 0)
		return STATS_OK;
	HisStats15MinMaxRecordGet(&maxBucketNum15m);
	if (tempbucket > maxBucketNum15m)
	{
		tempbucket = maxBucketNum15m;
	}
	pstatPonCurrent = gpPONHisStatsHead;
	while(pstatPonCurrent != NULL)
		{
			if ( ponId == pstatPonCurrent->ponIdx)
			{/*两种判断效果: 当找到该ponid 时，如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
				if(pstatPonCurrent->iUsed15Min)
				{
				cliFlag = STATS_OK;
				break;					
				}
				else
					return cliFlag;
			}
			
			pstatPonCurrent = pstatPonCurrent->pNext;
		}

	if (STATS_OK == cliFlag)
	{
		if (NULL == pstatPonCurrent)
			return STATS_ERR;
		pStatsChainIdx = pstatPonCurrent->pCurrent15M;
		pstatPonIdx = pstatPonCurrent;
		pNextstatPonIdx = pstatPonIdx;
		
		/*for (count = 1;count<12;count++)
			{
				StatsData = 0;
				HisStatsPon15MTableGet(count, pStatsChainIdx, &StatsData);
				CliStatsPonDataParse(count, StatsData);
			}	*/
	}
	else
		return STATS_OK;
	
	while(pNextstatPonIdx == pstatPonIdx)
	{
		vty_out( vty, "\r\n  %d.%d.%d %d:%d:%d(MillSec %d) :\r\n",pStatsChainIdx->year,\
				pStatsChainIdx->month,pStatsChainIdx->day,\
				pStatsChainIdx->hour,pStatsChainIdx->minute,\
				pStatsChainIdx->second,pStatsChainIdx->MillSecond);
		
		CliHisStatsDataVty(pStatsChainIdx, vty);
		/*for (count = 1;count<12;count++)
			{
				StatsData = 0;
				HisStatsPon15MTableGet(count, pStatsChainIdx, &StatsData);
				CliStatsPonDataParseVty(count, StatsData, vty);
			}*/
		vty_out(vty, "  \r\n\r\n");
		tempbucket--;
		if(tempbucket == 0)
			break;
		pNextstatPonIdx = NULL;
		pNextStatsChainIdx = NULL;
		lRet = CliHisStatsPon15MTableNextIndexGet(pstatPonIdx, 
								pStatsChainIdx, 
								&pNextstatPonIdx, 
								&pNextStatsChainIdx);
		if (STATS_END_OBJ_ERR == lRet)
			return STATS_OK;	
		pStatsChainIdx = pNextStatsChainIdx;
		
	}
	pNextstatPonIdx = NULL;
	pstatPonIdx = NULL;
	pstatPonCurrent = NULL;
	pStatsChainIdx = NULL;
	return STATS_OK;
}


STATUS CliHisStats24HourDataVty(unsigned short ponId, unsigned int bucket_num, struct vty* vty)
{
	long lRet = 0;
	int cliFlag = STATS_ERR;
	unsigned int tempbucket = bucket_num;
	unsigned int maxBucketNum24h = 0;
	statistic_PON_OID *pstatPonIdx = NULL ;
	statistic_PON_OID *pstatPonCurrent = NULL;
	statistc_HISTORY_CHAIN *pStatsChainIdx = NULL ;
	statistic_PON_OID *pNextstatPonIdx = NULL;
	statistc_HISTORY_CHAIN *pNextStatsChainIdx = NULL;
	/*搜索该ponId 并判断该ponid是否已经使能15min性能统计*/

	if (0 == tempbucket)
		return STATS_OK;
	HisStats24HoursMaxRecordGet(&maxBucketNum24h);
	if (tempbucket > maxBucketNum24h)
	{
		tempbucket = maxBucketNum24h;
	}	
	pstatPonCurrent = gpPONHisStatsHead;
	while(pstatPonCurrent != NULL)
		{
			if ( ponId == pstatPonCurrent->ponIdx)
			{/*两种判断效果: 当找到该ponid 时，如果没有使能15分钟的统计，
			则直接返回，不再继续遍历*/
				if(pstatPonCurrent->iUsed24Hour)
				{
				cliFlag = STATS_OK;
				break;					
				}
				else
					return cliFlag;
			}
			
			pstatPonCurrent = pstatPonCurrent->pNext;
		}
	if (STATS_OK == cliFlag)
	{
		if (NULL == pstatPonCurrent)
			return STATS_ERR;
		pStatsChainIdx = pstatPonCurrent->pCurrent24H;
		pstatPonIdx = pstatPonCurrent;
		pNextstatPonIdx = pstatPonIdx;

	}
	else
		return STATS_OK;

	while(pNextstatPonIdx == pstatPonIdx)
	{
		vty_out( vty, "\r\n  %d.%d.%d %d:%d:%d(MillSec %d) :\r\n",pStatsChainIdx->year,\
		pStatsChainIdx->month,pStatsChainIdx->day,\
		pStatsChainIdx->hour,pStatsChainIdx->minute,\
		pStatsChainIdx->second,pStatsChainIdx->MillSecond);

		CliHisStatsDataVty(pStatsChainIdx, vty);
		/*for (count = 1;count<12;count++)
			{
				StatsData = 0;
				HisStatsPon24HTableGet(count, pStatsChainIdx, &StatsData);
				CliStatsPonDataParseVty(count, StatsData, vty);
			}*/
		vty_out(vty, "  \r\n\r\n");
		tempbucket--;
		if(0 == tempbucket)
			return STATS_OK;
		/*pStatsChainIdx = pNextstatPonIdx;*/
		pNextstatPonIdx = NULL;
		pNextStatsChainIdx = NULL;
		lRet = CliHisStatsPon24HTableNextIndexGet(pstatPonIdx, 
								pStatsChainIdx, 
								&pNextstatPonIdx, 
								&pNextStatsChainIdx);
		if (STATS_OK != lRet)
			return STATS_OK;		
		pStatsChainIdx = pNextStatsChainIdx;
	}
	pNextstatPonIdx = NULL;
	pstatPonIdx = NULL;
	pstatPonCurrent = NULL;
	pStatsChainIdx = NULL;
	return STATS_OK;
	
}


#define PON_OLT_AND_ONU_COMBINED_ID		(-2)
STATUS CliRealTimeStatsPon( short int ponId, struct vty* vty )
{
	int iRes = 0;
	int len = 0;
	/*short int collector_id = 0;*/
	short int stat_parameter = 0;
    short int PonChipType;
	/*unsigned long	timestamp = 0 ;*/
	unsigned char 	buf[256] = {0};
	short int 	llid = 0;
	short int 	onuId = 0;
       
    /* OLT的PON口单项统计 */
	PON_total_frames_raw_data_t totalFrameRawData;
	PON_total_octets_raw_data_t totalOctetsRawData;
	PON_total_tx_dropped_frames_raw_data_t TotalDropFrameTxRawData;
	PON_total_dropped_rx_frames_raw_data_t TotalDropFrameRevRawData;
	PON_total_dropped_rx_good_frames_raw_data_t goodDropFrameRawData;
	PON_transmitted_frames_per_priority_raw_data_t cosTxOkFramePerPrioRawData;
	PON_dropped_frames_raw_data_t	        cosTxDropFramePerPrioRawData;
	PON_discarded_unknown_destination_address_raw_data_t udaDiscardRawData;
	PON_discarded_unknown_destination_address_raw_data_t TotaludaDiscardRawData;
	PON_uplink_vlan_discarded_frames_raw_data_t upVlanDiscardRawData;
	PON_uplink_vlan_discarded_frames_raw_data_t TotalupVlanDiscardRawData;
	PON_downlink_vlan_discarded_frames_raw_data_t downVlanDiscardRawData;
	PON_frame_check_sequence_errors_raw_data_t checkSeqErrRawData;
	PON_frame_check_sequence_errors_raw_data_t totalCheckSeqErrRawData;
	PON_in_range_length_errors_per_llid_raw_data_t	inRangeLengthLLIDErrRawData;
	PON_in_range_length_errors_raw_data_t	inRangeLengthErrRawData;
	PON_frame_too_long_errors_raw_data_t	frameTooLongErrRawData;
	PON_hec_frames_errors_raw_data_t 		hecFrameErrRawData;
	PON_fec_frames_raw_data_t               fecFrameRawData;
	PON_fec_frames_raw_data_t               fecTotalFrameRawData;
	PON_host_frames_raw_data_t 				hostframeRawData;
	PON_pause_frames_raw_data_t 			pauseFrameRawData;
	PON_grant_frames_raw_data_t 			grantFrameRawData;
	PON_report_frames_raw_data_t			reportFrameRawData;
	PON_unicast_frames_raw_data_t			totalUniFrameRawData;
	PON_multicast_frames_raw_data_t			mulFrameRawData;
	PON_multicast_frames_raw_data_t			totalMulFrameRawData;
	PON_broadcast_frames_raw_data_t			broadcastFramRawData;
	PON_broadcast_frames_raw_data_t			totalBroadcastFramRawData;
	PON_p2p_frames_raw_data_t				p2pFrameRawData;
	PON_p2p_frames_raw_data_t				p2pTotalFrameRawData;
	PON_p2p_global_frames_dropped_raw_data_t p2pFrameDropRawData;
#if 0
	PON_unicast_multicast_pon_frames_raw_data_t umFrameRawData;
	PON_unicast_multicast_pon_frames_raw_data_t totalUMFrameRawData;
#endif
	PON_grant_frames_raw_data_t 			grantTotalFrameRawData;
	PON_report_frames_raw_data_t 			reportTotalFrameRawData;
	PON_oam_frames_raw_data_t		        oamFrameStatRawData;
    PON_oam_statistic_raw_data_t            standOAMRawData;
    PON_mpcp_statistic_raw_data_t           standMpcpRawData;
    PON_mpcp_statistic_raw_data_t           TotalstandMpcpRawData;
    PON_dropped_frames_per_llid_raw_data_t  TotalllidDropRawData;
    PON_dropped_frames_per_llid_raw_data_t  llidDropRawData;
    PON_llid_frames_loose_data_t            llidLooseRawData;

    /* 所有ONU的单项统计总和 */
	PON_transmitted_frames_per_llid_raw_data_t llidBrdFrameSendRawData;
	PON_transmitted_frames_per_llid_raw_data_t totalLlidBrdFrameSendRawData;
#if 0
	PON_onu_fer_raw_data_t onuFerRawData;
	PON_onu_fer_raw_data_t onuTotalFerRawData;
	PON_onu_ber_raw_data_t onuBerRawData;
	PON_onu_ber_raw_data_t onuTotalBerRawData;
#endif
    
    OLT_raw_stat_item_t    stat_item;

	PonChipType = V2R1_GetPonchipType( ponId );

	vty_out( vty, "\r\n");
    
	{/*PON_RAW_STAT_TOTAL_FRAMES for pon*/
	memset( &totalFrameRawData, 0, sizeof( PON_total_frames_raw_data_t ) );
#if 1
    stat_item.collector_id = PON_OLT_ID;
    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_FRAMES;
    stat_item.statistics_parameter = OLT_PHYSICAL_PORT_PON;
    stat_item.statistics_data = &totalFrameRawData;
    stat_item.statistics_data_size = sizeof(totalFrameRawData);
    iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
	collector_id = -1;
	stat_parameter = 1;/*port id : 1- PON, 2 - system*/
	iRes = PAS_get_raw_statistics(  ponId, 
									collector_id, 
									PON_RAW_STAT_TOTAL_FRAMES, 
									stat_parameter, 
									&totalFrameRawData,
									&timestamp
									);
#endif
	}

	{/*total Octect for pon*/
	memset(&totalOctetsRawData, 0, sizeof(PON_total_octets_raw_data_t));
#if 1
    stat_item.collector_id = PON_OLT_ID;
    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_OCTETS;
    stat_item.statistics_parameter = OLT_PHYSICAL_PORT_PON;
    stat_item.statistics_data = &totalOctetsRawData;
    stat_item.statistics_data_size = sizeof(totalOctetsRawData);
    iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
	/*olt = -1*/
	collector_id = -1;	
	stat_parameter = 1;/*1-pon 2-system*/
	iRes = PAS_get_raw_statistics ( ponId, 
								   collector_id, 
								   PON_RAW_STAT_TOTAL_OCTETS, 
								   stat_parameter,
								   &totalOctetsRawData,
								   &timestamp 
									);	
#endif
	}

	memset(&totalLlidBrdFrameSendRawData, 0, sizeof(PON_transmitted_frames_per_llid_raw_data_t));
#if 1
    stat_item.collector_id = PON_OLT_ID;
    stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID;
    stat_item.statistics_data = &llidBrdFrameSendRawData;
    stat_item.statistics_data_size = sizeof(llidBrdFrameSendRawData);
#endif
	for(onuId = 0;onuId < STATS_HIS_MAXONU; onuId++)
	{
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;
		stat_parameter = llid;
		memset(&llidBrdFrameSendRawData, 0, sizeof(llidBrdFrameSendRawData));
#if 1
        stat_item.statistics_parameter = stat_parameter;
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;	
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID, 
									   stat_parameter,
									   &llidBrdFrameSendRawData,
									   &timestamp 
										);	
#endif
        if ( 0 == iRes )
        {
    		totalLlidBrdFrameSendRawData.pon_ok += llidBrdFrameSendRawData.pon_ok;
    		totalLlidBrdFrameSendRawData.system_ok += llidBrdFrameSendRawData.system_ok;
        }
	}
    stat_item.statistics_parameter = PAS_BROADCAST_LLID;
	memset(&llidBrdFrameSendRawData, 0, sizeof(llidBrdFrameSendRawData));
    iRes = OLT_GetRawStatistics(ponId, &stat_item);
    

    if ( OLT_PONCHIP_ISPAS(PonChipType) )
	{
		memset(&TotalDropFrameRevRawData, 0, sizeof(PON_total_dropped_rx_frames_raw_data_t) );
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &TotalDropFrameRevRawData;
        stat_item.statistics_data_size = sizeof(TotalDropFrameRevRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES, 
									   stat_parameter,
									   &TotalDropFrameRevRawData,
									   &timestamp 
										);
#endif
	}
	
	{
		memset(&TotalDropFrameTxRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t) );
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &TotalDropFrameTxRawData;
        stat_item.statistics_data_size = sizeof(TotalDropFrameTxRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
									   stat_parameter,
									   &TotalDropFrameTxRawData,
									   &timestamp 
										);	
#endif
	}


#if 0
	memset(&onuTotalFerRawData, 0, sizeof(PON_onu_fer_raw_data_t));
#if 1
    stat_item.collector_id = PON_OLT_ID;
    stat_item.raw_statistics_type = PON_RAW_STAT_ONU_FER;
    stat_item.statistics_data = &onuFerRawData;
    stat_item.statistics_data_size = sizeof(onuFerRawData);
#endif
	for(onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
	{
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;
		stat_parameter = llid;
		memset(&onuFerRawData, 0, sizeof(PON_onu_fer_raw_data_t));
#if 1
        stat_item.statistics_parameter = stat_parameter;
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;	
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_FER, 
									   stat_parameter,
									   &onuFerRawData,
									   &timestamp 
										);	
#endif
		if ( OLT_CALL_ISOK(iRes) )
			onuTotalFerRawData.received_error += onuFerRawData.received_error;
	}
#endif

#if 0
	memset(&onuTotalBerRawData, 0, sizeof(PON_onu_ber_raw_data_t));
#if 1
    stat_item.collector_id = PON_OLT_ID;
    stat_item.raw_statistics_type = PON_RAW_STAT_ONU_BER;
    stat_item.statistics_data = &onuBerRawData;
    stat_item.statistics_data_size = sizeof(onuBerRawData);
#endif
	for(onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
	{/*onu_ber*/
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;	
		stat_parameter = llid;

		memset(&onuBerRawData, 0, sizeof(PON_onu_ber_raw_data_t));
#if 1
        stat_item.statistics_parameter = stat_parameter;
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;	
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_BER, 
									   stat_parameter,
									   &onuBerRawData,
									   &timestamp 
										);
#endif
		if ( OLT_CALL_ISOK(iRes) )
			onuTotalBerRawData.error_bytes += onuBerRawData.error_bytes;
	}
#endif


	memset(&totalCheckSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));	
#if 1
    stat_item.collector_id = PON_OLT_ID;
    stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS;
    stat_item.statistics_data = &checkSeqErrRawData;
    stat_item.statistics_data_size = sizeof(checkSeqErrRawData);
#endif
	for(onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
	{/*check sequence error, one err of FCS err*/
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;			
		stat_parameter = llid;/*llid*/

		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
#if 1
        stat_item.statistics_parameter = stat_parameter;
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS, 
									   stat_parameter,
									   &checkSeqErrRawData,
									   &timestamp 
										);
#endif
		if ( OLT_CALL_ISOK(iRes) )
			totalCheckSeqErrRawData.received += checkSeqErrRawData.received;

	}	
    
	{/*In range length error*/
	memset(&inRangeLengthErrRawData, 0, sizeof(PON_in_range_length_errors_raw_data_t));
	if ( PONCHIP_PAS5001 != PonChipType )
    {
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &inRangeLengthErrRawData;
        stat_item.statistics_data_size = sizeof(inRangeLengthErrRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		stat_parameter = 0;
		memset(&inRangeLengthErrRawData, 0, sizeof(PON_in_range_length_errors_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS,
										stat_parameter, 
										&inRangeLengthErrRawData, 
										&timestamp
										);
#endif	
    }
    else
    {
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID;
        stat_item.statistics_data = &inRangeLengthLLIDErrRawData;
        stat_item.statistics_data_size = sizeof(inRangeLengthLLIDErrRawData);
    	for(onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
    	{/*grant frame*/
    		llid = GetLlidByOnuIdx( ponId, onuId);
    		if ((-1) == llid ) 
    			continue;		 
    		stat_parameter = llid;
    		memset(&inRangeLengthLLIDErrRawData, 0, sizeof(inRangeLengthLLIDErrRawData));
            stat_item.statistics_parameter = stat_parameter;
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
    		if ( OLT_CALL_ISOK(iRes) )
    		{
    			inRangeLengthErrRawData.pon_received += inRangeLengthLLIDErrRawData.received;
    		}
    	}
    }
	}
	
	{/*frame too long err*/
		memset( &frameTooLongErrRawData, 0, sizeof(PON_frame_too_long_errors_raw_data_t));
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_TOO_LONG_ERRORS;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &frameTooLongErrRawData;
        stat_item.statistics_data_size = sizeof(frameTooLongErrRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_FRAME_TOO_LONG_ERRORS, 
										stat_parameter, 
										&frameTooLongErrRawData,
										&timestamp
										);
#endif
	}
	
	memset(&hecFrameErrRawData, 0, sizeof(PON_hec_frames_errors_raw_data_t));
    if ( OLT_PONCHIP_ISPAS(PonChipType) )
	{/*HEC error*/
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_HEC_FRAMES_ERRORS;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &hecFrameErrRawData;
        stat_item.statistics_data_size = sizeof(hecFrameErrRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;/*olt*/
		stat_parameter = llid;/*llid*/
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_HEC_FRAMES_ERRORS, 
									   stat_parameter,
									   &hecFrameErrRawData,
									   &timestamp 
										);
#endif
	}

    if ( OLT_PONCHIP_ISPAS(PonChipType) )
	{/*host frame for pon&per llid*/
		memset(&hostframeRawData, 0, sizeof(PON_host_frames_raw_data_t));
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_HOST_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &hostframeRawData;
        stat_item.statistics_data_size = sizeof(hostframeRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_HOST_FRAMES, 
										stat_parameter, 
										&hostframeRawData,
										&timestamp
										);
#endif
	}

	{/*pause frame*/
		memset(&pauseFrameRawData, 0, sizeof(PON_pause_frames_raw_data_t));
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_PAUSE_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &pauseFrameRawData;
        stat_item.statistics_data_size = sizeof(pauseFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_PAUSE_FRAMES, 
										stat_parameter, 
										&pauseFrameRawData,
										&timestamp);
#endif
	}

	memset(&grantTotalFrameRawData, 0, sizeof(PON_grant_frames_raw_data_t));
    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
        stat_item.collector_id = PON_OLT_AND_ONU_COMBINED_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_GRANT_FRAMES;
        stat_item.statistics_data = &grantFrameRawData;
        stat_item.statistics_data_size = sizeof(grantFrameRawData);
    	for(onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
    	{/*grant frame*/
    		llid = GetLlidByOnuIdx( ponId, onuId);
    		if ((-1) == llid ) 
    			continue;		 
    		stat_parameter = llid;
    		memset(&grantFrameRawData, 0, sizeof(PON_grant_frames_raw_data_t));
#if 1
            stat_item.statistics_parameter = stat_parameter;
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = PON_OLT_AND_ONU_COMBINED_ID;
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_GRANT_FRAMES, 
    										stat_parameter, 
    										&grantFrameRawData,
    										&timestamp
    										);
#endif
    		if ( OLT_CALL_ISOK(iRes) )
    		{
    			/*grantFrameRawData.received_ok*/
    			grantTotalFrameRawData.received_ok += grantFrameRawData.received_ok;
    			grantTotalFrameRawData.transmitted_ctrl_ok += grantFrameRawData.transmitted_ctrl_ok;
    			grantTotalFrameRawData.transmitted_dba_ok += grantFrameRawData.transmitted_dba_ok;
    		}
    	}
    }
    else
    {
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_GRANT_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &grantTotalFrameRawData;
        stat_item.statistics_data_size = sizeof(grantTotalFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
    }

	memset(&reportTotalFrameRawData, 0, sizeof(PON_report_frames_raw_data_t));
    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
        stat_item.collector_id = PON_OLT_AND_ONU_COMBINED_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_REPORT_FRAMES;
        stat_item.statistics_data = &reportFrameRawData;
        stat_item.statistics_data_size = sizeof(reportFrameRawData);
    	for(onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
    	{/*report frame*/
    		llid = GetLlidByOnuIdx( ponId, onuId);
    		if ((-1) == llid ) 
    			continue;			
    		stat_parameter = llid;
    		
    		memset(&reportFrameRawData, 0, sizeof(PON_report_frames_raw_data_t));
#if 1
            stat_item.statistics_parameter = stat_parameter;
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = PON_OLT_AND_ONU_COMBINED_ID;
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_REPORT_FRAMES, 
    										stat_parameter, 
    										&reportFrameRawData,
    										&timestamp
    										);
#endif
    		if( OLT_CALL_ISOK(iRes) )
    		{
    			reportTotalFrameRawData.received_ok += reportFrameRawData.received_ok;
    			reportTotalFrameRawData.transmitted_ok += reportFrameRawData.transmitted_ok;
    		}
    	}
    }
    else
    {
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_REPORT_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &reportTotalFrameRawData;
        stat_item.statistics_data_size = sizeof(reportTotalFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
    }


    if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
    	memset(&totalUniFrameRawData, 0, sizeof(totalUniFrameRawData));
        stat_item.collector_id = PON_OLT_ID;
        stat_item.statistics_parameter = 0;
        stat_item.raw_statistics_type = PON_RAW_STAT_UNICAST_FRAMES;
        stat_item.statistics_data = &totalUniFrameRawData;
        stat_item.statistics_data_size = sizeof(totalUniFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
	}


	memset(&totalMulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_MULTICAST_FRAMES;
        stat_item.statistics_data = &mulFrameRawData;
        stat_item.statistics_data_size = sizeof(mulFrameRawData);
    	for(onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
    	{/*获取接收到的组播数据*/
    		llid = GetLlidByOnuIdx( ponId, onuId);
    		if ((-1) == llid ) 
    			continue;
    		stat_parameter = llid;/*llid*/
    		memset(&mulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
#if 1
            stat_item.statistics_parameter = stat_parameter;
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = -1;		
    		iRes = PAS_get_raw_statistics ( ponId, 
    									   collector_id, 
    									   PON_RAW_STAT_MULTICAST_FRAMES, 
    									   stat_parameter,
    									   &mulFrameRawData,
    									   &timestamp 
    										);
#endif
    		if ( OLT_CALL_ISOK(iRes) )
    		{
    			totalMulFrameRawData.received_ok += mulFrameRawData.received_ok;
    			totalMulFrameRawData.transmitted_ok += mulFrameRawData.transmitted_ok;		
    		}
    	}
    }
    else
    {
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_MULTICAST_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &totalMulFrameRawData;
        stat_item.statistics_data_size = sizeof(totalMulFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
    }
    

	memset(&totalBroadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_BROADCAST_FRAMES;
        stat_item.statistics_data = &broadcastFramRawData;
        stat_item.statistics_data_size = sizeof(broadcastFramRawData);
    	for(onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
    	{/*获取接收到的广播数据*/
    		llid = GetLlidByOnuIdx( ponId, onuId);
    		if ((-1) == llid ) 
    			continue;
    		stat_parameter = llid;/*llid*/
    		memset(&broadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
#if 1
            stat_item.statistics_parameter = stat_parameter;
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = -1;	
    		iRes = PAS_get_raw_statistics ( ponId, 
    									   collector_id, 
    									   PON_RAW_STAT_BROADCAST_FRAMES, 
    									   stat_parameter,
    									   &broadcastFramRawData,
    									   &timestamp 
    										);	
#endif
    		if ( OLT_CALL_ISOK(iRes) )
    		{
    			totalBroadcastFramRawData.received_ok += broadcastFramRawData.received_ok;
    			totalBroadcastFramRawData.transmitted_ok += broadcastFramRawData.transmitted_ok;
    		}
    	}
	}
    else
    {
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_BROADCAST_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &totalBroadcastFramRawData;
        stat_item.statistics_data_size = sizeof(totalBroadcastFramRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
    }

	if ( PONCHIP_PAS5001 != PonChipType )
    {       
        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
        	memset(&cosTxOkFramePerPrioRawData, 0, sizeof(cosTxOkFramePerPrioRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY;
            stat_item.statistics_parameter = OLT_PHYSICAL_PORT_PON;
            stat_item.statistics_data = &cosTxOkFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosTxOkFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
            
        	memset(&cosTxDropFramePerPrioRawData, 0, sizeof(cosTxDropFramePerPrioRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_DROPPED_FRAMES;
            stat_item.statistics_parameter = OLT_PHYSICAL_PORT_PON;
            stat_item.statistics_data = &cosTxDropFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosTxDropFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
        
        	memset(&goodDropFrameRawData, 0, sizeof(goodDropFrameRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_RX_GOOD_FRAMES;
            stat_item.statistics_parameter = 0;
            stat_item.statistics_data = &goodDropFrameRawData;
            stat_item.statistics_data_size = sizeof(goodDropFrameRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);

        	memset(&p2pFrameDropRawData, 0, sizeof(p2pFrameDropRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_P2P_GLOBAL_FRAMES_DROPPED;
            stat_item.statistics_parameter = 0;
            stat_item.statistics_data = &p2pFrameDropRawData;
            stat_item.statistics_data_size = sizeof(p2pFrameDropRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);        

#if 0
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_UNICAST_MULTICAST_PON_FRAMES;
            stat_item.statistics_data = &umFrameRawData;
            stat_item.statistics_data_size = sizeof(umFrameRawData);
        	memset(&totalUMFrameRawData, 0, sizeof(PON_unicast_multicast_pon_frames_raw_data_t));
        	for( onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
            {
        		llid = GetLlidByOnuIdx( ponId, onuId);
        		if ((-1) == llid ) 
        			continue;	
        		stat_parameter = llid;
                
        		memset(&umFrameRawData, 0, sizeof(PON_unicast_multicast_pon_frames_raw_data_t));
                stat_item.statistics_parameter = stat_parameter;
                iRes = OLT_GetRawStatistics(ponId, &stat_item);
        		if ( OLT_CALL_ISOK(iRes) )
        		{
        			totalUMFrameRawData.rx_multicast_frames += umFrameRawData.rx_multicast_frames;
        			totalUMFrameRawData.tx_multicast_frames += umFrameRawData.tx_multicast_frames;
        			totalUMFrameRawData.rx_unicast_frames   += umFrameRawData.rx_unicast_frames;
        			totalUMFrameRawData.tx_unicast_frames   += umFrameRawData.tx_unicast_frames;
        		}
            }   
#endif

           	memset(&TotaludaDiscardRawData, 0, sizeof(TotaludaDiscardRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_DISCARDED_UNKNOWN_DESTINATION_ADDRESS;
            stat_item.statistics_data = &udaDiscardRawData;
            stat_item.statistics_data_size = sizeof(udaDiscardRawData);
        	for( onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
            {
        		llid = GetLlidByOnuIdx( ponId, onuId);
        		if ((-1) == llid ) 
        			continue;	
        		stat_parameter = llid;
                
        		memset(&udaDiscardRawData, 0, sizeof(udaDiscardRawData));
                stat_item.statistics_parameter = stat_parameter;
                iRes = OLT_GetRawStatistics(ponId, &stat_item);
        		if ( OLT_CALL_ISOK(iRes) )
        		{
        			TotaludaDiscardRawData.frames += udaDiscardRawData.frames;
        		}
            }   

           	memset(&TotalupVlanDiscardRawData, 0, sizeof(TotalupVlanDiscardRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_UPLINK_VLAN_DISCARDED_FRAMES;
            stat_item.statistics_data = &upVlanDiscardRawData;
            stat_item.statistics_data_size = sizeof(upVlanDiscardRawData);
        	for( onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
            {
        		llid = GetLlidByOnuIdx( ponId, onuId);
        		if ((-1) == llid ) 
        			continue;	
        		stat_parameter = llid;
                
        		memset(&upVlanDiscardRawData, 0, sizeof(upVlanDiscardRawData));
                stat_item.statistics_parameter = stat_parameter;
                iRes = OLT_GetRawStatistics(ponId, &stat_item);
        		if ( OLT_CALL_ISOK(iRes) )
        		{
        			TotalupVlanDiscardRawData.tagged_frames        += upVlanDiscardRawData.tagged_frames;
        			TotalupVlanDiscardRawData.untagged_frames      += upVlanDiscardRawData.untagged_frames;
        			TotalupVlanDiscardRawData.null_tagged_frames   += upVlanDiscardRawData.null_tagged_frames;
        			TotalupVlanDiscardRawData.nested_tagged_frames += upVlanDiscardRawData.nested_tagged_frames;
        			TotalupVlanDiscardRawData.discarded_frames     += upVlanDiscardRawData.discarded_frames;
        		}
            }   
            
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_DOWNLINK_VLAN_DISCARDED_FRAMES;
            stat_item.statistics_parameter = 0;
            stat_item.statistics_data = &downVlanDiscardRawData;
            stat_item.statistics_data_size = sizeof(downVlanDiscardRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);       
        }


       	memset(&fecTotalFrameRawData, 0, sizeof(fecTotalFrameRawData));
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_FEC_FRAMES;
        stat_item.statistics_data_size = sizeof(fecFrameRawData);
        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
            stat_item.statistics_data = &fecFrameRawData;
            
        	for( onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
            {
        		llid = GetLlidByOnuIdx( ponId, onuId);
        		if ((-1) == llid ) 
        			continue;	
        		stat_parameter = llid;
                
        		memset(&fecFrameRawData, 0, sizeof(fecFrameRawData));
                stat_item.statistics_parameter = stat_parameter;
                iRes = OLT_GetRawStatistics(ponId, &stat_item);
        		if ( OLT_CALL_ISOK(iRes) )
        		{
        			fecTotalFrameRawData.fixed_frames        += fecFrameRawData.fixed_frames;
        			fecTotalFrameRawData.unfixed_frames      += fecFrameRawData.unfixed_frames;
        			fecTotalFrameRawData.good_frames         += fecFrameRawData.good_frames;
        			fecTotalFrameRawData.wrong_parity_number += fecFrameRawData.wrong_parity_number;
        		}
            }   
        }
        else
        {
            stat_item.statistics_data = &fecTotalFrameRawData;
            stat_item.statistics_parameter = 0;
            
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
        }
    }   
 
	memset(&p2pTotalFrameRawData, 0, sizeof(PON_p2p_frames_raw_data_t));
#if 1
    stat_item.collector_id = PON_OLT_ID;
    stat_item.raw_statistics_type = PON_RAW_STAT_P2P_FRAMES;
    stat_item.statistics_data = &p2pFrameRawData;
    stat_item.statistics_data_size = sizeof(p2pFrameRawData);
#endif
	for( onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
	{/*p2p frame*/
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;	
		stat_parameter = llid;
		
		memset(&p2pFrameRawData, 0, sizeof(PON_p2p_frames_raw_data_t));
#if 1
        stat_item.statistics_parameter = stat_parameter;
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_P2P_FRAMES, 
										stat_parameter, 
										&p2pFrameRawData,
										&timestamp
										);
#endif
		if ( OLT_CALL_ISOK(iRes) )
		{
			p2pTotalFrameRawData.received_ok += p2pFrameRawData.received_ok;
			p2pTotalFrameRawData.transmitted_ok += p2pFrameRawData.transmitted_ok;
			p2pTotalFrameRawData.dropped_by_policer += p2pFrameRawData.dropped_by_policer;
			p2pTotalFrameRawData.dropped_by_access_control += p2pFrameRawData.dropped_by_access_control;
		}
	}


    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
    	memset(&standOAMRawData, 0, sizeof(standOAMRawData));
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_STANDARD_OAM_STATISTIC;
        stat_item.statistics_parameter = PON_STATISTICS_ALL_LLID;
        stat_item.statistics_data = &standOAMRawData;
        stat_item.statistics_data_size = sizeof(standOAMRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);        
    }
    else
	{/*oam frame Rx&Tx*/
    	memset(&oamFrameStatRawData, 0, sizeof(PON_oam_frames_raw_data_t));
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_OAM_FRAMES;
        stat_item.statistics_parameter = PON_STATISTICS_ALL_LLID;
        stat_item.statistics_data = &oamFrameStatRawData;
        stat_item.statistics_data_size = sizeof(oamFrameStatRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_OAM_FRAMES, 
										stat_parameter, 
										 
										&oamFrameStatRawData,
										&timestamp
										);
#endif		
	}


	memset(&TotalstandMpcpRawData, 0, sizeof(TotalstandMpcpRawData));
    stat_item.collector_id = PON_OLT_ID;
    stat_item.raw_statistics_type = PON_RAW_STAT_STANDARD_OAM_MPCP_STATISTIC;
    stat_item.statistics_data_size = sizeof(standMpcpRawData);
    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
        stat_item.statistics_data = &standMpcpRawData;
    	for( onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
    	{/*mpcp static*/
    		llid = GetLlidByOnuIdx( ponId, onuId);
    		if ((-1) == llid ) 
    			continue;	
    		stat_parameter = llid;
            
            stat_item.statistics_parameter = stat_parameter;
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
    		if ( OLT_CALL_ISOK(iRes) )
    		{
                TotalstandMpcpRawData.mac_ctrl_frames_transmitted     += standMpcpRawData.mac_ctrl_frames_transmitted;
                TotalstandMpcpRawData.mac_ctrl_frames_received        += standMpcpRawData.mac_ctrl_frames_received;
                TotalstandMpcpRawData.discovery_windows_sent          += standMpcpRawData.discovery_windows_sent;
                TotalstandMpcpRawData.discovery_timeout               += standMpcpRawData.discovery_timeout;
                TotalstandMpcpRawData.register_request_transmitted_ok += standMpcpRawData.register_request_transmitted_ok;
                TotalstandMpcpRawData.register_request_received_ok    += standMpcpRawData.register_request_received_ok;
                TotalstandMpcpRawData.register_ack_transmitted_ok     += standMpcpRawData.register_ack_transmitted_ok;
                TotalstandMpcpRawData.register_ack_received_ok        += standMpcpRawData.register_ack_received_ok;
                TotalstandMpcpRawData.transmitted_report              += standMpcpRawData.transmitted_report;
                TotalstandMpcpRawData.received_report                 += standMpcpRawData.received_report;
                TotalstandMpcpRawData.transmited_gate                 += standMpcpRawData.transmited_gate;
                TotalstandMpcpRawData.receive_gate                    += standMpcpRawData.receive_gate;
                TotalstandMpcpRawData.register_transmited             += standMpcpRawData.register_transmited;
                TotalstandMpcpRawData.register_received               += standMpcpRawData.register_received;
    		}
    	}
    }
    else
    {
        stat_item.statistics_data = &TotalstandMpcpRawData;
        stat_item.statistics_parameter = 0;
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
    }


    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
    	memset(&TotalllidDropRawData, 0, sizeof(TotalllidDropRawData));
		stat_item.collector_id = PON_OLT_ID;
	    stat_item.raw_statistics_type = PON_RAW_STAT_DROPPED_FRAMES_PER_LLID;
		
	    stat_item.statistics_data = &llidDropRawData;
	    stat_item.statistics_data_size = sizeof(llidDropRawData);
    	for( onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
    	{/*mpcp static*/
    		llid = GetLlidByOnuIdx( ponId, onuId);
    		if ((-1) == llid ) 
    			continue;	
    		stat_parameter = llid;
            
            stat_item.statistics_parameter = stat_parameter;
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
    		if ( OLT_CALL_ISOK(iRes) )
    		{
                TotalllidDropRawData.policer_dropped += llidDropRawData.policer_dropped;
                TotalllidDropRawData.mismatch_dropped += llidDropRawData.mismatch_dropped;
                TotalllidDropRawData.address_table_dropped += llidDropRawData.address_table_dropped;
    		}
    	}
        stat_item.statistics_parameter = PAS_BROADCAST_LLID;
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
		if ( OLT_CALL_ISOK(iRes) )
		{
            TotalllidDropRawData.policer_dropped += llidDropRawData.policer_dropped;
		}
    }
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
    	memset(&llidLooseRawData, 0, sizeof(llidLooseRawData));
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_LLID_LOOSE_FRAME_STATISTIC;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &llidLooseRawData;
        stat_item.statistics_data_size = sizeof(llidLooseRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
    }
    else
    {
        VOS_ASSERT(0);
    }


	vty_out( vty, "\r\n");
	
	vty_out( vty, "  --------------------------------------Total Sum-----------------------------------\r\n");
	vty_out( vty, "  RxTotalFrameOk         : %-20lu",totalFrameRawData.received_ok );
	if ( PONCHIP_PAS5001 != PonChipType )
    {
    	vty_out( vty, "  TxTotalFrameOk         : %-20lu\r\n",totalFrameRawData.transmitted_ok);
    }
    else
    {
    	vty_out( vty, "  TxTotalFrameOk         : --\r\n");
    }
	
	/*sprintf(buf,  "  RxOctetsOk             : 0x%016lx",(unsigned long long )totalOctetsRawData.received_ok);	*/
	len = sprintf( buf, "  RxOctetsOk             : " );
	sprintf64Bits( buf+len, (unsigned long long)totalOctetsRawData.received_ok );
	vty_out( vty, "%s",buf);

	/*sprintf(buf,  "  TxOctetsOk             :     0x%016lx\r\n",(unsigned long long )(totalOctetsRawData.transmitted_ok));		*/
	len = sprintf( buf, "  TxOctetsOk             : " );
	sprintf64Bits( buf+len, (unsigned long long)totalOctetsRawData.transmitted_ok);
	vty_out( vty, "%-10s",buf);
	vty_out( vty, "\r\n" );

#if 0
	if ( PONCHIP_PAS5001 != PonChipType )
    {
    	vty_out( vty, "  RxUnicastFrame         : %-20lu",totalUMFrameRawData.rx_unicast_frames);
    	vty_out( vty, "  TxUnicastFrame         : %-20lu\r\n",totalUMFrameRawData.tx_unicast_frames);

    	vty_out( vty, "  RxMulticastFrame       : %-20lu",totalUMFrameRawData.rx_multicast_frames);
    	vty_out( vty, "  TxMulticastFrame       : %-20lu\r\n",totalUMFrameRawData.tx_multicast_frames);
    }
#endif

    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
    	vty_out( vty, "  RxUnicastFrameOk       : %-20lu",totalLlidBrdFrameSendRawData.system_ok + p2pTotalFrameRawData.received_ok);
    	vty_out( vty, "  TxUnicastFrameOk       : %-20lu\r\n",totalLlidBrdFrameSendRawData.pon_ok);
    }
    else
    {
    	vty_out( vty, "  RxUnicastFrameOk       : %-20lu",totalUniFrameRawData.received_ok);
    	vty_out( vty, "  TxUnicastFrameOk       : %-20lu\r\n",totalUniFrameRawData.transmitted_ok);
    }

	vty_out( vty, "  RxMulticastFrameOk     : %-20lu",totalMulFrameRawData.received_ok);
	vty_out( vty, "  TxMulticastFrameOk     : %-20lu\r\n",totalMulFrameRawData.transmitted_ok);

	vty_out( vty, "  RxBroadcastFrameOk     : %-20lu",totalBroadcastFramRawData.received_ok);
	vty_out( vty, "  TxBroadcastFrameOk     : %-20lu\r\n",totalBroadcastFramRawData.transmitted_ok);

	vty_out( vty, "\r\n  TxFrameDrop            : %-20lu\r\n",TotalDropFrameTxRawData.total_pon_dropped);
	if ( PONCHIP_PAS5001 != PonChipType )
    {
/*下行由于QoS造成的丢包，保证高优先级的流先过*/
      	vty_out( vty, "  TxDropForQueuePri      : %-20lu",TotalDropFrameTxRawData.egress_pon_dropped);
/*
	设计时用外部的SDRAM做下行的Queue，由于SDRAM满了造成的下行丢包
	我们PMC 板子没有使用外部SDRAM ,deleted by liyang @2015-03-27
*/
#if 0 	
      	vty_out( vty, "  TxDropForQueueFull     : %-20lu\r\n",TotalDropFrameTxRawData.pon_ram_dropped);
#endif
        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
            vty_out( vty, "  TxDropForPolicer       : %-20lu\r\n",TotalllidDropRawData.policer_dropped);
        
        	vty_out( vty, "  TxFrameOKPerPrio0      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[0]);
        	vty_out( vty, "  TxDropPerPrio0         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[0]);
        	vty_out( vty, "  TxFrameOKPerPrio1      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[1]);
        	vty_out( vty, "  TxDropPerPrio1         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[1]);
        	vty_out( vty, "  TxFrameOKPerPrio2      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[2]);
        	vty_out( vty, "  TxDropPerPrio2         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[2]);
        	vty_out( vty, "  TxFrameOKPerPrio3      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[3]);
        	vty_out( vty, "  TxDropPerPrio3         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[3]);
        	vty_out( vty, "  TxFrameOKPerPrio4      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[4]);
        	vty_out( vty, "  TxDropPerPrio4         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[4]);
        	vty_out( vty, "  TxFrameOKPerPrio5      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[5]);
        	vty_out( vty, "  TxDropPerPrio5         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[5]);
        	vty_out( vty, "  TxFrameOKPerPrio6      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[6]);
        	vty_out( vty, "  TxDropPerPrio6         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[6]);
        	vty_out( vty, "  TxFrameOKPerPrio7      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[7]);
        	vty_out( vty, "  TxDropPerPrio7         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[7]);
        }
    }


	vty_out( vty, "\r\n");
 
    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
    	vty_out( vty, "  RxDropForSAIsOlt       : %-20lu",TotalDropFrameRevRawData.source_alert_dropped);
    	vty_out( vty, "  RxDropMpcpForQueueFull : %-20lu\r\n",TotalDropFrameRevRawData.total_control_dropped);

    	if ( PONCHIP_PAS5001 != PonChipType )
        {
            vty_out( vty, "  RxDropForUnknownSA     : %-20lu",TotalllidDropRawData.address_table_dropped);
        	vty_out( vty, "  RxDropForNotGrant      : %-20lu\r\n",TotalllidDropRawData.mismatch_dropped);

            vty_out( vty, "  RxDropForUnknownDA     : %-20lu",TotaludaDiscardRawData.frames);
            vty_out( vty, "  RxDropForUplinkUserCfg : %-20lu\r\n",goodDropFrameRawData.uplink_dropped);
        }
        else
        {
        	vty_out( vty, "  RxDropForNotGrant      : %-20lu\r\n",TotalllidDropRawData.mismatch_dropped);
        }
    }
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
      	vty_out( vty, "  RxDropForNoLLIDMatch   : %-20lu",llidLooseRawData.llid_nomatch);
      	vty_out( vty, "  RxDropForNotGrant      : %-20lu\r\n",llidLooseRawData.llid_nogrant);
    }
    else
    {
    	VOS_ASSERT(0);
    }


	vty_out( vty, "  --------------------------------------Rx Error------------------------------------\r\n");
	if ( PONCHIP_PAS5001 != PonChipType )
    {
#if 1 /*与"Total Sum" 重复deleted by liyang @2015-03-27*/
    	vty_out( vty, "  RxFrameTotal           : %-20lu",totalFrameRawData.received_ok + totalFrameRawData.received_error + hecFrameErrRawData.received );
#endif
    	vty_out( vty, "  RxFrameErrTotal        : %-20lu\r\n",totalFrameRawData.received_error );

#if 1 /*与"Total Sum" 重复deleted by liyang @2015-03-27*/
    	len = sprintf(buf, "  RxOctetsTotal          : " );	
    	sprintf64Bits( buf+len, (unsigned long long)totalOctetsRawData.received_ok + totalOctetsRawData.received_error );
		vty_out( vty, "%s",buf);
#endif

    	len = sprintf(buf, "  RxOctetsErrTotal       : " );	
    	sprintf64Bits( buf+len, (unsigned long long)totalOctetsRawData.received_error );
    	vty_out( vty, "%s\r\n",buf);
    }   
    else
    {
    	vty_out( vty, "  RxFrameTotal           : --" );
    	vty_out( vty, "  RxFrameErr             : --\r\n" );
    	vty_out( vty, "  RxOctetsTotal          : --" );
    	vty_out( vty, "  RxOctetsErr            : --\r\n" );
    }

    vty_out( vty, "  RxInRangeLengthErr     : %-20lu",inRangeLengthErrRawData.pon_received);
	vty_out( vty, "  RxFrameTooLongErr      : %-20lu\r\n",frameTooLongErrRawData.pon_received);

	vty_out( vty, "  RxFrameHECErr          : %-20lu",hecFrameErrRawData.received);
	if ( PONCHIP_PAS5001 != PonChipType )
    {
#if 0 /*deleted by liyang @2015-03-30 for  repeat with follow*/
    	vty_out( vty, "  RxFrameFCSErr          : %-20lu\r\n",totalFrameRawData.received_error);
		vty_out( vty, "  --                     : --                  ");
#endif
    	vty_out( vty, "  RxFrameFCSCrcErr       : %-20lu\r\n",totalCheckSeqErrRawData.received);

    	vty_out( vty, "  RxFrameWrongParity     : %-20lu",fecTotalFrameRawData.wrong_parity_number);
    	vty_out( vty, "  RxFrameNotNeedFECFixed : %-20lu\r\n",fecTotalFrameRawData.good_frames);
    	vty_out( vty, "  RxFrameFECFixedOK      : %-20lu",fecTotalFrameRawData.fixed_frames);
    	vty_out( vty, "  RxFrameFECFixedErr     : %-20lu\r\n",fecTotalFrameRawData.unfixed_frames);
    }
    else
    {
    	vty_out( vty, "  RxFrameFCSCrcErr       : %-20lu\r\n",totalCheckSeqErrRawData.received);
    }

    
	vty_out( vty, "  --------------------------------------MPCP----------------------------------------\r\n");
#if 0 /*deleted by liyang @2015-03-28 for expression error*/
	vty_out( vty, "  TxGrantFrameOk         : %-20lu\r\n",grantTotalFrameRawData.transmitted_dba_ok + grantTotalFrameRawData.transmitted_ctrl_ok);
#endif
	vty_out( vty, "  TxDbaGrantFrame        : %-20lu",grantTotalFrameRawData.transmitted_dba_ok);
	vty_out( vty, "  TxRegisterGrantFrame   : %-20lu\r\n",grantTotalFrameRawData.transmitted_ctrl_ok);
	vty_out( vty, "  RxReportFrameOk        : %-20lu",reportTotalFrameRawData.received_ok);
    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
    	vty_out( vty, "  RxMpcpDropForQueueFull : %-20lu\r\n",TotalDropFrameRevRawData.total_control_dropped);
    }
    else
    {
    	vty_out( vty, "  RxMpcpDropForQueueFull : --\r\n");
    }

	vty_out( vty, "\r\n  RxMpcpFrame            : %-20lu",TotalstandMpcpRawData.mac_ctrl_frames_received);
	vty_out( vty, "  TxMpcpFrame            : %-20lu\r\n",TotalstandMpcpRawData.mac_ctrl_frames_transmitted);
	vty_out( vty, "  RxMpcpDiscoveryTimeout : %-20lu",TotalstandMpcpRawData.discovery_timeout);
	vty_out( vty, "  TxMpcpDiscoveryWindow  : %-20lu\r\n",TotalstandMpcpRawData.discovery_windows_sent);
	vty_out( vty, "  RxMpcpReport           : %-20lu",TotalstandMpcpRawData.received_report);
#if 0 /*deleted by liyang @2015-03-28 for pon port stats don't include TxMpcpReport and RxMpcpGate*/
	vty_out( vty, "  TxMpcpReport           : %-20lu\r\n",TotalstandMpcpRawData.transmitted_report);
	vty_out( vty, "  RxMpcpGate             : %-20lu",TotalstandMpcpRawData.receive_gate);
#endif
	vty_out( vty, "  TxMpcpGate             : %-20lu\r\n",TotalstandMpcpRawData.transmited_gate);
	vty_out( vty, "  TxMpcpRegister         : %-20lu",TotalstandMpcpRawData.register_transmited);
	vty_out( vty, "  RxMpcpRegisterRequest  : %-20lu\r\n",TotalstandMpcpRawData.register_request_received_ok);
	vty_out( vty, "  RxMpcpRegisterAck      : %-20lu\r\n",TotalstandMpcpRawData.register_ack_received_ok);
#if 0 /*The resson as above */
	vty_out( vty, "  RxMpcpRegister         : %-20lu",TotalstandMpcpRawData.register_received);	
	vty_out( vty, "  TxMpcpRegisterRequest  : %-20lu\r\n",TotalstandMpcpRawData.register_request_transmitted_ok);
	vty_out( vty, "  TxMpcpRegisterAck      : %-20lu\r\n",TotalstandMpcpRawData.register_ack_transmitted_ok);
#endif

	vty_out( vty, "  --------------------------------------OAM-----------------------------------------\r\n");
    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
    	vty_out( vty, "  RxOamFrame             : %-20lu",standOAMRawData.information_rx
                                                        + standOAMRawData.unique_event_notification_rx
                                                        + standOAMRawData.duplicate_event_notification_rx
                                                        + standOAMRawData.oam_loopback_control_rx
                                                        + standOAMRawData.oam_variable_request_rx
                                                        + standOAMRawData.oam_variable_response_rx
                                                        + standOAMRawData.oam_organization_specific_rx
                                                        + standOAMRawData.unsupported_codes_rx
                                                        );
    	vty_out( vty, "  TxOamFrame             : %-20lu\r\n",standOAMRawData.information_tx
                                                        + standOAMRawData.unique_event_notification_tx
                                                        + standOAMRawData.duplicate_event_notification_tx
                                                        + standOAMRawData.oam_loopback_control_tx
                                                        + standOAMRawData.oam_variable_request_tx
                                                        + standOAMRawData.oam_variable_response_tx
                                                        + standOAMRawData.oam_organization_specific_tx
                                                        + standOAMRawData.unsupported_codes_tx
                                                        );

    	vty_out( vty, "  RxOamInfoCode          : %-20lu",standOAMRawData.information_rx);
    	vty_out( vty, "  TxOamInfoCode          : %-20lu\r\n",standOAMRawData.information_tx);
    	vty_out( vty, "  RxOamUniqueEventNotify : %-20lu",standOAMRawData.unique_event_notification_rx);
    	vty_out( vty, "  TxOamUniqueEventNotify : %-20lu\r\n",standOAMRawData.unique_event_notification_tx);
    	vty_out( vty, "  RxOamDupEventNotify    : %-20lu",standOAMRawData.duplicate_event_notification_rx);
    	vty_out( vty, "  TxOamDupEventNotify    : %-20lu\r\n",standOAMRawData.duplicate_event_notification_tx);
#if 0 /*The resson as above*/
    	vty_out( vty, "  RxOamLoopbackCheck     : %-20lu",standOAMRawData.oam_loopback_control_rx);
#endif
		vty_out( vty, "  TxOamLoopbackCheck     : %-20lu",standOAMRawData.oam_loopback_control_tx);
#if 0 /*The resson as above*/
    	vty_out( vty, "  RxOamVarRequest        : %-20lu",standOAMRawData.oam_variable_request_rx);
#endif
		vty_out( vty, "  TxOamVarRequest        : %-20lu\r\n",standOAMRawData.oam_variable_request_tx);
    	vty_out( vty, "  RxOamVarResponse       : %-20lu",standOAMRawData.oam_variable_response_rx);
    	vty_out( vty, "  TxOamVarResponse       : %-20lu\r\n",standOAMRawData.oam_variable_response_tx);
    	vty_out( vty, "  RxOamOrgSpec           : %-20lu",standOAMRawData.oam_organization_specific_rx);
    	vty_out( vty, "  TxOamOrgSpec           : %-20lu\r\n",standOAMRawData.oam_organization_specific_tx);
    	vty_out( vty, "  RxOamUnsupportedCode   : %-20lu",standOAMRawData.unsupported_codes_rx);
    	vty_out( vty, "  TxOamUnsupportedCode   : %-20lu\r\n",standOAMRawData.unsupported_codes_tx);
    	vty_out( vty, "  --                     : --                  ");
    	vty_out( vty, "  TxOamDropForInnerError : %-20lu\r\n",standOAMRawData.frames_lost_due_to_oam_error);
    }
    else
    {
    	vty_out( vty, "  RxOamFrame             : %-20lu",oamFrameStatRawData.received_ok);
    	vty_out( vty, "  TxOamFrame             : %-20lu\r\n",oamFrameStatRawData.transmitted_ok);
    }


	if ( OLT_PONCHIP_ISPAS(PonChipType) && (PONCHIP_PAS5001 != PonChipType) )
    {
    	vty_out( vty, "  --------------------------------------QinQ----------------------------------------\r\n");
    	vty_out( vty, "  RxDropInUplinkVlan     : %-20lu",TotalupVlanDiscardRawData.tagged_frames + TotalupVlanDiscardRawData.discarded_frames + TotalupVlanDiscardRawData.nested_tagged_frames + TotalupVlanDiscardRawData.null_tagged_frames + TotalupVlanDiscardRawData.untagged_frames);
    	vty_out( vty, "  RxDropInDownlinkVlan   : %-20lu\r\n",downVlanDiscardRawData.nested_tagged_frames + downVlanDiscardRawData.destination_discarded_frames);
    	vty_out( vty, "  RxDropUplinkNestTag    : %-20lu",TotalupVlanDiscardRawData.nested_tagged_frames);
    	vty_out( vty, "  RxDropDownlinkNestTag  : %-20lu\r\n",downVlanDiscardRawData.nested_tagged_frames);
    	vty_out( vty, "  RxDropUplinkOpIsDrop   : %-20lu",TotalupVlanDiscardRawData.discarded_frames);
    	vty_out( vty, "  RxDropDownlinkDstIsNone: %-20lu\r\n",downVlanDiscardRawData.destination_discarded_frames);
    	vty_out( vty, "  RxDropUplinkTag        : %-20lu",TotalupVlanDiscardRawData.tagged_frames);
    	vty_out( vty, "  --\r\n");

    	vty_out( vty, "  RxDropUplinkUnTag      : %-20lu",TotalupVlanDiscardRawData.untagged_frames);
    	vty_out( vty, "  --\r\n");
    	vty_out( vty, "  RxDropUplinkNullTag    : %-20lu",TotalupVlanDiscardRawData.null_tagged_frames);
    	vty_out( vty, "  --\r\n");
    }   


	vty_out( vty, "  --------------------------------------DataPath------------------------------------\r\n");
	vty_out( vty, "  RxUniLlidUplinkFrameOk : %-20lu",totalLlidBrdFrameSendRawData.system_ok);
	vty_out( vty, "  TxUniLlidFrameOk       : %-20lu\r\n",totalLlidBrdFrameSendRawData.pon_ok);
	vty_out( vty, "  RxBrdLlidUplinkFrameOk : %-20lu",llidBrdFrameSendRawData.system_ok);
	vty_out( vty, "  TxBrdLlidFrameOk       : %-20lu\r\n",llidBrdFrameSendRawData.pon_ok);   

	vty_out( vty, "\r\n  Rxp2pFrameOk           : %-20lu",p2pTotalFrameRawData.received_ok);
	vty_out( vty, "  Txp2pFrameOk           : %-20lu\r\n",p2pTotalFrameRawData.transmitted_ok);

	if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
    	vty_out( vty, "  Rxp2pFrameDropTotal    : %-20lu",p2pTotalFrameRawData.dropped_by_policer + p2pTotalFrameRawData.dropped_by_access_control);
    	vty_out( vty, "  Rxp2pDropForAccessCtrl : %-20lu\r\n",p2pTotalFrameRawData.dropped_by_access_control);
    	vty_out( vty, "  Rxp2pDropForPolicer    : %-20lu",p2pTotalFrameRawData.dropped_by_policer);
#if 0 /*没有使用外部SDRAM ,deleted by liyang @2015-03-27*/
		if ( PONCHIP_PAS5001 != PonChipType )
        {
        	vty_out( vty, "  Txp2pFrameDrop         : %-20lu\r\n",p2pFrameDropRawData.egress_dropped + p2pFrameDropRawData.ram_dropped);
        	vty_out( vty, "  Txp2pDropForQueuePri   : %-20lu",p2pFrameDropRawData.egress_dropped);
        	vty_out( vty, "  Txp2pDropForQueueFull  : %-20lu\r\n",p2pFrameDropRawData.ram_dropped);
        }
        else
#endif
        {
        	vty_out( vty, "  Txp2pFrameDrop         : %-20lu\r\n", p2pTotalFrameRawData.dropped_due_to_tx_buffer_full);
        }
    }

	vty_out( vty, "\r\n  RxPauseFrameOk         : %-20lu", pauseFrameRawData.pon_received_ok);
	vty_out( vty, "  TxPauseFrameOk         : %-20lu\r\n", pauseFrameRawData.pon_transmitted_ok);


    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
    	vty_out( vty, "  --------------------------------------ManagePath----------------------------------\r\n");
    	vty_out( vty, "  RxHostFrameOk          : %-20lu",hostframeRawData.pon_received_ok);	
    	vty_out( vty, "  TxHostframeOk          : %-20lu\r\n",hostframeRawData.pon_transmitted_ok);
#if 0 /*没有使用外部SDRAM ,deleted by liyang @2015-03-27*/
		if ( PONCHIP_PAS5001 != PonChipType )
        {
        	vty_out( vty, "\r\n  RxHostFrameDrop        : %-20lu\r\n",hostframeRawData.pon_dropped + hostframeRawData.pon_ram_dropped);
          	vty_out( vty, "  RxDropForQueuePri      : %-20lu",hostframeRawData.pon_dropped);
          	vty_out( vty, "  RxDropForQueueFull     : %-20lu\r\n",hostframeRawData.pon_ram_dropped);
        }
        else
#endif
        {
        	vty_out( vty, "  RxHostFrameDrop        : %-20lu\r\n",hostframeRawData.pon_dropped);
        }
    }

	vty_out( vty, "\r\n");

	return 0;
}


/*added by jnhl*/
STATUS CliRealTimeStatsPonForGpon( short int ponId, struct vty* vty )
{
	int iRes = 0;
	int len = 0;
	int onuId = 0;
	int llid = 0;
	int gemId = 0;
    OLT_raw_stat_item_t    stat_item;

	tOgCmPmPonLinkNewCounter counter;
	tOgCmPmPonGemCounter gemStats;
	tOgCmPmPonGemCounter allGemStats;

	
    vty_out( vty, "\r\n");
	memset(&stat_item, 0, sizeof(OLT_raw_stat_item_t));
	memset( &counter, 0, sizeof( tOgCmPmPonLinkNewCounter ) );
	memset( &gemStats, 0, sizeof( tOgCmPmPonGemCounter ) );
	memset( &allGemStats, 0, sizeof( tOgCmPmPonGemCounter ) );

    stat_item.collector_id = GPON_PM_PONLINK_ACTIVE_COUNTER;
    stat_item.statistics_data = &counter;
    stat_item.statistics_data_size = sizeof(tOgCmPmPonLinkNewCounter);
    iRes = OLT_GetRawStatistics(ponId, &stat_item);
	if(VOS_OK != iRes)
	{
		vty_out( vty, "\r\n");
		return iRes;
	}

	vty_out( vty, "  --------------------------------------Total Sum-----------------------------------\r\n");
	vty_out( vty, "  rx_gem_packets            : %-20llu",counter.rx_gem_packets);
	vty_out( vty, "  tx_gem                    : %-20llu\r\n",counter.tx_gem);
    
    vty_out( vty, "  rx_ploams                 : %-20llu",counter.rx_ploams);
	vty_out( vty, "  tx_ploams                 : %-20llu\r\n",counter.tx_ploams);
	vty_out( vty, "  rx_cpu                    : %-20llu",counter.rx_cpu);
	vty_out( vty, "  tx_cpu                    : %-20llu\r\n",counter.tx_cpu);

	vty_out( vty, "  rx_omci                   : %-20llu",counter.rx_omci);
	vty_out( vty, "  tx_omci                   : %-20llu\r\n",counter.tx_omci);

	vty_out( vty, "  rx_fec_codewords          : %-20llu\r\n",counter.fec_codewords);
	vty_out( vty, "  rx_bip8_bytes             : %-20llu\r\n",counter.bip8_bytes);
	vty_out( vty, "  rx_gem_idle               : %-20llu\r\n",counter.rx_gem_idle);
	vty_out( vty, "  rx_gem_corrected          : %-20llu\r\n",counter.rx_gem_corrected);
	vty_out( vty, "  rx_allocations_valid      : %-20llu\r\n",counter.rx_allocations_valid);
	vty_out( vty, "  rx_ploams_non_idle        : %-20llu\r\n",counter.rx_ploams_non_idle);
	vty_out( vty, "  rx_bip8_bytes             : %-20lu\r\n",counter.bip8_bytes);

	vty_out( vty, "\r\n");
 
	vty_out( vty, "  --------------------------------------Rx/Tx Error------------------------------------\r\n");
	
	vty_out( vty, "  rx_bip8_errors            : %-20llu",counter.bip8_errors );

	vty_out( vty, "  tx_dropped_illegal_length : %-20llu\r\n",counter.tx_dropped_illegal_length );

    vty_out( vty, "  rx_bip8_errors            : %-20lu",counter.bip8_errors);
	vty_out( vty, "  tx_dropped_tpid_miss      : %-20lu\r\n",counter.tx_dropped_tpid_miss);

	vty_out( vty, "  rx_gem_dropped            : %-20llu",counter.rx_gem_dropped);
	
	vty_out( vty, "  tx_dropped_vid_miss       : %-20llu\r\n",counter.tx_dropped_vid_miss);

	vty_out( vty, "  rx_gem_illegal            : %-20llu\r\n",counter.rx_gem_illegal);
	vty_out( vty, "  rx_allocations_invalid    : %-20llu\r\n",counter.rx_allocations_invalid);
	vty_out( vty, "  rx_allocations_disabled   : %-20lu",counter.rx_allocations_disabled);
	vty_out( vty, "  rx_ploams_error           : %-20llu\r\n",counter.rx_ploams_error);
	vty_out( vty, "  rx_ploams_dropped         : %-20llu\r\n",counter.rx_ploams_dropped);
	vty_out( vty, "  rx_dropped_too_short      : %-20llu\r\n",counter.rx_dropped_too_short);
	vty_out( vty, "  rx_dropped_too_long       : %-20llu\r\n",counter.rx_dropped_too_long);
	vty_out( vty, "  rx_crc_errors             : %-20llu\r\n",counter.rx_crc_errors);
	vty_out( vty, "  rx_key_errors             : %-20llu\r\n",counter.rx_key_errors);
	vty_out( vty, "  rx_fragments_errors       : %-20llu\r\n",counter.rx_fragments_errors);
	vty_out( vty, "  rx_packets_dropped        : %-20llu\r\n",counter.rx_packets_dropped);

    vty_out( vty, "\r\n");
	
	for( onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
	{
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;
		gemId = ulGPON_GEMPORT_BASE+(llid-1)*16;
		memset( &gemStats, 0, sizeof( tOgCmPmPonGemCounter ) );
		stat_item.collector_id = GPON_PM_PONGEM_ACTIVE_COUNTER;
	    stat_item.statistics_data = &gemStats;
		stat_item.statistics_parameter = gemId;
	    stat_item.statistics_data_size = sizeof(tOgCmPmPonGemCounter);
	    iRes = OLT_GetRawStatistics(ponId, &stat_item);
		if(VOS_OK != iRes)
		{
			continue;
		}
		allGemStats.rx_bytes += gemStats.rx_bytes;
		allGemStats.rx_packets += gemStats.rx_packets;
		allGemStats.tx_bytes += gemStats.tx_bytes;
		allGemStats.tx_packets += gemStats.tx_packets;
	}
#if 1
	vty_out( vty, "  --------------------------------------DataPath------------------------------------\r\n");
	
    vty_out( vty, "\r\n  rx_packets         : %-20llu", allGemStats.rx_packets);
	vty_out( vty, "  tx_packets             : %-20llu\r\n", allGemStats.tx_packets); 
	vty_out( vty, "\r\n  rx_bytes           : %-20llu", allGemStats.rx_bytes);
	vty_out( vty, "  tx_bytes               : %-20llu\r\n", allGemStats.tx_bytes); 

	vty_out( vty, "\r\n");
#endif

	return 0;
}

STATUS CliRealTimeStatsCNIForGpon( short int ponId ,struct vty* vty)
{
	int iRes = 0;
	int len = 0;
	
    OLT_raw_stat_item_t    stat_item;

	tOgCmPmPonNNICounter counter;

	
    vty_out( vty, "\r\n");
	memset(&stat_item, 0, sizeof(OLT_raw_stat_item_t));
	memset( &counter, 0, sizeof( tOgCmPmPonNNICounter ) );

    stat_item.collector_id = GPON_PM_PONNNI_ACTIVE_COUNTER;
    stat_item.statistics_data = &counter;
    stat_item.statistics_data_size = sizeof(tOgCmPmPonNNICounter);
    iRes = OLT_GetRawStatistics(ponId, &stat_item);
	if(VOS_OK != iRes)
	{
		vty_out( vty, "\r\n");
		return iRes;
	}

	vty_out( vty, "  --------------------------------------Total Sum-----------------------------------\r\n");
	vty_out( vty, "  rx_frames_64            : %-20llu",counter.rx_frames_64);
	vty_out( vty, "  tx_frames_64            : %-20llu\r\n",counter.tx_frames_64);
    
    vty_out( vty, "  rx_frames_65_127        : %-20llu",counter.rx_frames_65_127);
	vty_out( vty, "  tx_frames_65_127        : %-20llu\r\n",counter.tx_frames_65_127);
	vty_out( vty, "  rx_frames_128_255       : %-20llu",counter.rx_frames_128_255);
	vty_out( vty, "  tx_frames_128_255       : %-20llu\r\n",counter.tx_frames_128_255);
	vty_out( vty, "  rx_frames_256_511       : %-20llu",counter.rx_frames_256_511);
	vty_out( vty, "  tx_frames_256_511       : %-20llu\r\n",counter.tx_frames_256_511);
	vty_out( vty, "  rx_frames_512_1023      : %-20llu",counter.rx_frames_512_1023);
	vty_out( vty, "  tx_frames_512_1023      : %-20llu\r\n",counter.tx_frames_512_1023);
	vty_out( vty, "  rx_frames_1024_1518     : %-20llu",counter.rx_frames_1024_1518);
	vty_out( vty, "  tx_frames_1024_1518     : %-20llu\r\n",counter.tx_frames_1024_1518);
	vty_out( vty, "  rx_frames_1519_2047     : %-20llu",counter.rx_frames_1519_2047);
	vty_out( vty, "  tx_frames_1519_2047     : %-20llu\r\n",counter.tx_frames_1519_2047);
	vty_out( vty, "  rx_frames_2048_4095     : %-20llu",counter.rx_frames_2048_4095);
	vty_out( vty, "  tx_frames_2048_4095     : %-20llu\r\n",counter.tx_frames_2048_4095);
	vty_out( vty, "  rx_frames_4096_9216     : %-20llu",counter.rx_frames_4096_9216);
	vty_out( vty, "  tx_frames_4096_9216     : %-20llu\r\n",counter.tx_frames_4096_9216);
	vty_out( vty, "  rx_frames_9217_16383    : %-20llu",counter.rx_frames_9217_16383);
	vty_out( vty, "  tx_frames_9217_16383    : %-20llu\r\n",counter.tx_frames_9217_16383);
	vty_out( vty, "  rx_frames               : %-20llu",counter.rx_frames);
	vty_out( vty, "  tx_frames               : %-20llu\r\n",counter.tx_frames);
	vty_out( vty, "  rx_bytes                : %-20llu",counter.rx_bytes);
	vty_out( vty, "  tx_bytes                : %-20llu\r\n",counter.tx_bytes);
	vty_out( vty, "  rx_good_frames          : %-20llu",counter.rx_good_frames);
	vty_out( vty, "  tx_good_frames          : %-20llu\r\n",counter.tx_good_frames);
	vty_out( vty, "  rx_unicast_frames       : %-20llu",counter.rx_unicast_frames);
	vty_out( vty, "  tx_unicast_frames       : %-20llu\r\n",counter.tx_unicast_frames);
	vty_out( vty, "  rx_multicast_frames     : %-20llu",counter.rx_multicast_frames);
	vty_out( vty, "  tx_multicast_frames     : %-20llu\r\n",counter.tx_multicast_frames);
	vty_out( vty, "  rx_broadcast_frames     : %-20llu",counter.rx_broadcast_frames);
	vty_out( vty, "  tx_broadcast_frames     : %-20llu\r\n",counter.tx_broadcast_frames);
	vty_out( vty, "  rx_control_frames       : %-20llu",counter.rx_control_frames);
	vty_out( vty, "  tx_control_frames       : %-20llu\r\n",counter.tx_control_frames);
	vty_out( vty, "  rx_pause_frames         : %-20llu",counter.rx_pause_frames);
	vty_out( vty, "  tx_pause_frames         : %-20llu\r\n",counter.tx_pause_frames);
	vty_out( vty, "  rx_pfc_frames           : %-20llu",counter.tx_pfc_frames);
	vty_out( vty, "  tx_pfc_frames           : %-20llu\r\n",counter.tx_multicast_frames);
	vty_out( vty, "  rx_vlan_frames          : %-20llu",counter.rx_vlan_frames);
	vty_out( vty, "  tx_vlan_frames          : %-20llu\r\n",counter.tx_vlan_frames);
	vty_out( vty, "  rx_double_vlan_frames   : %-20llu",counter.rx_double_vlan_frames);
	vty_out( vty, "  tx_double_vlan_frames   : %-20llu\r\n",counter.tx_double_vlan_frames);

	vty_out( vty, "\r\n");
 
	vty_out( vty, "  --------------------------------------Rx/Tx Error------------------------------------\r\n");
	
	vty_out( vty, "  rx_fcs_errors           : %-20llu",counter.rx_fcs_errors);
	vty_out( vty, "  tx_fcs_errors           : %-20llu\r\n",counter.tx_fcs_errors);
	vty_out( vty, "  rx_oversized_frames     : %-20llu",counter.rx_oversized_frames);
	vty_out( vty, "  tx_oversize_frames      : %-20llu\r\n",counter.tx_oversize_frames);
	vty_out( vty, "  rx_fragmented_frames    : %-20llu",counter.rx_fragmented_frames);
	vty_out( vty, "  tx_fragmented_frames    : %-20llu\r\n",counter.tx_fragmented_frames);
	vty_out( vty, "  rx_runt_frames          : %-20llu",counter.rx_runt_frames);
	vty_out( vty, "  tx_runt_frames          : %-20llu\r\n",counter.tx_runt_frames);
	vty_out( vty, "  rx_jabber_frames        : %-20llu",counter.rx_jabber_frames);
	vty_out( vty, "  tx_jabber_frames        : %-20llu\r\n",counter.tx_jabber_frames);
	vty_out( vty, "  rx_unsupported_opcode   : %-20llu",counter.rx_unsupported_opcode);
	vty_out( vty, "  tx_error_frames         : %-20llu\r\n",counter.tx_error_frames);
	vty_out( vty, "  rx_unsupported_da       : %-20llu",counter.rx_unsupported_da);
	vty_out( vty, "  tx_underrun_frames      : %-20llu\r\n",counter.tx_underrun_frames);
	vty_out( vty, "  rx_alignment_errors     : %-20llu\r\n",counter.rx_alignment_errors);
	vty_out( vty, "  rx_length_out_of_range  : %-20llu\r\n",counter.rx_length_out_of_range);
	vty_out( vty, "  rx_code_errors          : %-20llu\r\n",counter.rx_code_errors);
	vty_out( vty, "  rx_mtu_check_errors     : %-20llu\r\n",counter.rx_mtu_check_errors);
	vty_out( vty, "  rx_promiscuous_frames   : %-20llu\r\n",counter.rx_promiscuous_frames);
	vty_out( vty, "  rx_truncated_frames     : %-20llu\r\n",counter.rx_truncated_frames);
	vty_out( vty, "  rx_undersize_frames     : %-20llu\r\n",counter.rx_undersize_frames);

    vty_out( vty, "\r\n");


	return 0;
}


STATUS CliRealTimeStatsGemForGpon( short int ponId , short int gemId, struct vty* vty)
{
	int iRes = 0;
	int len = 0;
	
    OLT_raw_stat_item_t    stat_item;

	tOgCmPmPonGemCounter counter;

	
    vty_out( vty, "\r\n");
	memset(&stat_item, 0, sizeof(OLT_raw_stat_item_t));
	memset( &counter, 0, sizeof( tOgCmPmPonGemCounter ) );

    stat_item.collector_id = GPON_PM_PONGEM_ACTIVE_COUNTER;
	stat_item.statistics_parameter = gemId;
    stat_item.statistics_data = &counter;
    stat_item.statistics_data_size = sizeof(tOgCmPmPonGemCounter);
    iRes = OLT_GetRawStatistics(ponId, &stat_item);
	if(VOS_OK != iRes)
	{
		vty_out( vty, "\r\n");
		return iRes;
	}

	vty_out( vty, "  --------------------------------------Total Sum-----------------------------------\r\n");
	vty_out( vty, "  rx_packets       : %-20llu",counter.rx_packets);
	vty_out( vty, "  tx_packets       : %-20llu\r\n",counter.tx_packets);
    
    vty_out( vty, "  rx_bytes         : %-20llu",counter.rx_bytes);
	vty_out( vty, "  tx_bytes         : %-20llu\r\n",counter.tx_bytes);
	
	vty_out( vty, "\r\n");
	return 0;
	
}
STATUS CliRealTimeStatsCNI( short int ponId ,struct vty* vty)
{
	int iRes = 0;
	ULONG len = 0;
	/*short int collector_id = 0;
	short int stat_parameter = 0;
	unsigned long	timestamp = 0 ;*/
	unsigned char 	buf[256] = {0};
	/*short int 	llid = 0;*/
    short int PonChipType;

	PON_in_range_length_errors_raw_data_t	inRangeLengthErrRawData;
	PON_frame_too_long_errors_raw_data_t	frameTooLongErrRawData;
	PON_host_frames_raw_data_t				hostframeRawData;
	PON_total_frames_raw_data_t				totalFrameRawData;
	PON_total_tx_dropped_frames_raw_data_t	TotalDropFrameTxRawData;
	PON_transmitted_frames_per_priority_raw_data_t cosTxOkFramePerPrioRawData;
	PON_dropped_frames_raw_data_t	        cosTxDropFramePerPrioRawData;
	PON_ipg_raw_data_t                      ipgFrameDropRawData;
	PON_pause_frames_raw_data_t		pauseFrameRawData;
	PON_pause_time_raw_data_t		pauseTimeRawData;
	PON_total_octets_raw_data_t		totalOctetsRawData;
	PON_discarded_unknown_destination_address_raw_data_t udaDiscardRawData;
	PON_total_dropped_rx_good_frames_raw_data_t goodDropFrameRawData;

    OLT_raw_stat_item_t    stat_item;

	PonChipType = V2R1_GetPonchipType( ponId );

	
	{/*PON_RAW_STAT_TOTAL_FRAMES for pon*/
		memset( &totalFrameRawData, 0, sizeof( PON_total_frames_raw_data_t ) );
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_FRAMES;
        stat_item.statistics_parameter = OLT_PHYSICAL_PORT_SYSTEM;
        stat_item.statistics_data = &totalFrameRawData;
        stat_item.statistics_data_size = sizeof(totalFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		stat_parameter = 2;/*port id : 1- PON, 2 - system*/
		iRes = PAS_get_raw_statistics(  ponId, 
										collector_id, 
										PON_RAW_STAT_TOTAL_FRAMES, 
										stat_parameter, 
										&totalFrameRawData,
										&timestamp
										);
#endif
	}
	
	{/*total Octect for pon*/
		/*olt = -1*/
		memset(&totalOctetsRawData, 0, sizeof(PON_total_octets_raw_data_t));
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_OCTETS;
        stat_item.statistics_parameter = OLT_PHYSICAL_PORT_SYSTEM;
        stat_item.statistics_data = &totalOctetsRawData;
        stat_item.statistics_data_size = sizeof(totalOctetsRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;	
		stat_parameter = 2;/*1-pon 2-system*/
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_OCTETS, 
									   stat_parameter,
									   &totalOctetsRawData,
									   &timestamp 
										);
#endif
	}

	
	{
		memset(&TotalDropFrameTxRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t) );
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &TotalDropFrameTxRawData;
        stat_item.statistics_data_size = sizeof(TotalDropFrameTxRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
									   stat_parameter,
									   &TotalDropFrameTxRawData,
									   &timestamp 
									   );
#endif
	}	

	if (  OLT_PONCHIP_ISPAS(PonChipType) && (PONCHIP_PAS5001 != PonChipType) )
    {
		memset(&ipgFrameDropRawData, 0, sizeof(ipgFrameDropRawData) );
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_IPG;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &ipgFrameDropRawData;
        stat_item.statistics_data_size = sizeof(ipgFrameDropRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
        
		memset(&cosTxOkFramePerPrioRawData, 0, sizeof(cosTxOkFramePerPrioRawData) );
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY;
        stat_item.statistics_parameter = OLT_PHYSICAL_PORT_SYSTEM;
        stat_item.statistics_data = &cosTxOkFramePerPrioRawData;
        stat_item.statistics_data_size = sizeof(cosTxOkFramePerPrioRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
        
		memset(&cosTxDropFramePerPrioRawData, 0, sizeof(cosTxDropFramePerPrioRawData) );
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_DROPPED_FRAMES;
        stat_item.statistics_parameter = OLT_PHYSICAL_PORT_SYSTEM;
        stat_item.statistics_data = &cosTxDropFramePerPrioRawData;
        stat_item.statistics_data_size = sizeof(cosTxDropFramePerPrioRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
        
		memset(&udaDiscardRawData, 0, sizeof(udaDiscardRawData) );
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_DISCARDED_UNKNOWN_DESTINATION_ADDRESS;
        stat_item.statistics_parameter = PON_STATISTICS_SYSTEM_LLID;
        stat_item.statistics_data = &udaDiscardRawData;
        stat_item.statistics_data_size = sizeof(udaDiscardRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
        
		memset(&goodDropFrameRawData, 0, sizeof(goodDropFrameRawData) );
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_RX_GOOD_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &goodDropFrameRawData;
        stat_item.statistics_data_size = sizeof(goodDropFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
    }   
    
	{/*In range length error*/
		memset(&inRangeLengthErrRawData, 0, sizeof(PON_in_range_length_errors_raw_data_t));
#if 1/*-------modified by wangjiah@2016-07-28---begin*/
		if(OLT_PONCHIP_ISBCM(PonChipType))
		{
			stat_item.collector_id = 0;
		}
		/*-------modified by wangjiah@2016-07-28---end*/
		else
		{
			stat_item.collector_id = PON_OLT_ID;
		}

        stat_item.raw_statistics_type = PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &inRangeLengthErrRawData;
        stat_item.statistics_data_size = sizeof(inRangeLengthErrRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS,
										stat_parameter, 
										&inRangeLengthErrRawData, 
										&timestamp
										);
#endif	
	}

	{/*frame too long err*/
		memset( &frameTooLongErrRawData, 0, sizeof(PON_frame_too_long_errors_raw_data_t));
#if 1
		/*-------modified by wangjiah@2016-07-28---begin*/
		if(OLT_PONCHIP_ISBCM(PonChipType))
		{
			stat_item.collector_id = 0;
		}
		/*-------modified by wangjiah@2016-07-28---end*/
		else
		{
			stat_item.collector_id = PON_OLT_ID;
		}
        stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_TOO_LONG_ERRORS;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &frameTooLongErrRawData;
        stat_item.statistics_data_size = sizeof(frameTooLongErrRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_FRAME_TOO_LONG_ERRORS, 
										stat_parameter, 
										&frameTooLongErrRawData,
										&timestamp
										);
#endif	
	}	

    if ( OLT_PONCHIP_ISPAS(PonChipType) )
	{/*host frame for pon&per llid*/
		memset(&hostframeRawData, 0, sizeof(PON_host_frames_raw_data_t));
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_HOST_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &hostframeRawData;
        stat_item.statistics_data_size = sizeof(hostframeRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_HOST_FRAMES, 
										stat_parameter, 
										&hostframeRawData,
										&timestamp
										);
#endif	
	}
    	
	{/*pause frame*/
		memset(&pauseFrameRawData, 0, sizeof(PON_pause_frames_raw_data_t));
#if 1
		/*-------modified by wangjiah@2016-07-28---begin*/
		if(OLT_PONCHIP_ISBCM(PonChipType))
		{
			stat_item.collector_id = 0;
		}
		/*-------modified by wangjiah@2016-07-28---end*/
		else
		{
			stat_item.collector_id = PON_OLT_ID;
		}
        stat_item.raw_statistics_type = PON_RAW_STAT_PAUSE_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &pauseFrameRawData;
        stat_item.statistics_data_size = sizeof(pauseFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_PAUSE_FRAMES, 
										stat_parameter, 
										&pauseFrameRawData,
										&timestamp);
#endif	
	}
	
    if ( OLT_PONCHIP_ISPAS(PonChipType) )
	{/*pause item*/
		memset(&pauseTimeRawData, 0, sizeof(PON_pause_time_raw_data_t));
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_PAUSE_TIME;
        stat_item.statistics_parameter = OLT_PHYSICAL_PORT_SYSTEM;
        stat_item.statistics_data = &pauseTimeRawData;
        stat_item.statistics_data_size = sizeof(pauseTimeRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_PAUSE_TIME, 
										stat_parameter, 
										&pauseTimeRawData,
										&timestamp
										);	
#endif	
	}

	vty_out( vty, "\r\n");	
	vty_out( vty, "  --------------------------------------Total Sum-----------------------------------\r\n");
	vty_out( vty, "  RxTotalFrameOk         : %-20lu",totalFrameRawData.received_ok );
	vty_out( vty, "  TxTotalFrameOk         : %-20lu\r\n",totalFrameRawData.transmitted_ok );

	len = sprintf(buf, "  RxOctetsOk             : " );	
	sprintf64Bits( buf+len, (unsigned long long)totalOctetsRawData.received_ok );
	vty_out( vty, "%s",buf);
    
	len = sprintf(buf, "  TxOctetsOk             : " );		
	sprintf64Bits( buf+len, (unsigned long long )totalOctetsRawData.transmitted_ok );
	vty_out( vty, "%s",buf);
	vty_out( vty, "\r\n" );
	
	vty_out( vty, "\r\n  TxFrameDrop            : %-20lu\r\n",TotalDropFrameTxRawData.total_system_dropped);
	if ( OLT_PONCHIP_ISPAS(PonChipType) && (PONCHIP_PAS5001 != PonChipType) )
    {
      	vty_out( vty, "  TxDropForShortIPG      : %-20lu",ipgFrameDropRawData.short_ipg);
      	vty_out( vty, "  TxDropForQueuePri      : %-20lu\r\n",TotalDropFrameTxRawData.egress_system_dropped);
#if 0  /*没有使用外部SDRAM ,deleted by liyang @2015-03-30*/
      	vty_out( vty, "  TxDropForQueueFull     : %-20lu\r\n",TotalDropFrameTxRawData.system_ram_dropped);
#endif
    	vty_out( vty, "  TxFrameOKPerPrio0      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[0]);
    	vty_out( vty, "  TxDropPerPrio0         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[0]);
    	vty_out( vty, "  TxFrameOKPerPrio1      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[1]);
    	vty_out( vty, "  TxDropPerPrio1         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[1]);
    	vty_out( vty, "  TxFrameOKPerPrio2      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[2]);
    	vty_out( vty, "  TxDropPerPrio2         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[2]);
    	vty_out( vty, "  TxFrameOKPerPrio3      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[3]);
    	vty_out( vty, "  TxDropPerPrio3         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[3]);
    	vty_out( vty, "  TxFrameOKPerPrio4      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[4]);
    	vty_out( vty, "  TxDropPerPrio4         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[4]);
    	vty_out( vty, "  TxFrameOKPerPrio5      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[5]);
    	vty_out( vty, "  TxDropPerPrio5         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[5]);
    	vty_out( vty, "  TxFrameOKPerPrio6      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[6]);
    	vty_out( vty, "  TxDropPerPrio6         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[6]);
    	vty_out( vty, "  TxFrameOKPerPrio7      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[7]);
    	vty_out( vty, "  TxDropPerPrio7         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[7]);

    	vty_out( vty, "\r\n");
    	vty_out( vty, "  RxDropForUnknownDA     : %-20lu",udaDiscardRawData.frames);
    	vty_out( vty, "  RxDropForDownlinkVlan  : %-20lu\r\n",goodDropFrameRawData.downlink_dropped);
    }


	vty_out( vty, "  --------------------------------------Rx Error------------------------------------\r\n");
#if 1  /*与"Total Sum" 重复deleted by liyang @2015-03-27*/
	vty_out( vty, "  RxFrameTotal           : %-20lu",totalFrameRawData.received_ok + totalFrameRawData.received_error );
#endif
	vty_out( vty, "  RxFrameErrTotal        : %-20lu\r\n",totalFrameRawData.received_error );

	if ( PONCHIP_PAS5001 != PonChipType )
    {
#if 1  /*与"Total Sum" 重复deleted by liyang @2015-03-27*/
    	len = sprintf(buf, "  RxOctetsTotal          : " );	
    	sprintf64Bits( buf+len, (unsigned long long)(totalOctetsRawData.received_ok + totalOctetsRawData.received_error) );
    	vty_out( vty, "%s",buf);
#endif
    	len = sprintf(buf, "  RxOctetsErr            : " );	
    	sprintf64Bits( buf+len, (unsigned long long)totalOctetsRawData.received_error );
    	vty_out( vty, "%s\r\n",buf);
    }   
    else
    {
    	vty_out( vty, "  RxOctetsTotal          : --" );
    	vty_out( vty, "  RxOctetsErr            : --\r\n" );
    }

	vty_out( vty, "  RxFrameInRangeErr      : %-20lu",inRangeLengthErrRawData.system_received);
	vty_out( vty, "  RxFrameTooLongErr      : %-20lu\r\n",frameTooLongErrRawData.system_received);
	vty_out( vty, "  RxFrameFCSErr          : %-20lu",totalFrameRawData.received_error);
	vty_out( vty, "  --\r\n");


	vty_out( vty, "  --------------------------------------DataPath------------------------------------\r\n");
	vty_out( vty, "  RxPauseFrameOk         : %-20lu", pauseFrameRawData.system_received_ok);
	vty_out( vty, "  TxPauseFrameOk         : %-20lu\r\n", pauseFrameRawData.system_transmitted_ok);
    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
    	vty_out( vty, "  --                     : --                  ");
    	vty_out( vty, "  TxPauseTime            : %-20lu\r\n", pauseTimeRawData.system_port);
    }


    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
    	vty_out( vty, "  --------------------------------------ManagePath----------------------------------\r\n");
    	vty_out( vty, "  RxHostFrameOk          : %-20lu",hostframeRawData.system_received_ok);
    	vty_out( vty, "  TxHostFrameOk          : %-20lu\r\n",hostframeRawData.system_transmitted_ok);
    	vty_out( vty, "  RxHostFrameTotal       : %-20lu",hostframeRawData.total_host_destined_frames);
    	if ( PONCHIP_PAS5001 != PonChipType )
        {
        	vty_out( vty, "  RxHostFrameDropTotal   : %-20lu\r\n",hostframeRawData.system_dropped + hostframeRawData.host_rate_limiter_dropped);
          	vty_out( vty, "  RxDropForQueuePri      : %-20lu",hostframeRawData.system_dropped);
#if 0 /*没有extern SDRAM deleted by liyang @2015-03-30 */
          	vty_out( vty, "  RxDropForQueueFull     : %-20lu\r\n",hostframeRawData.system_ram_dropped);
#endif
			vty_out( vty, "  RxDropForHostRateLimit : %-20lu\r\n",hostframeRawData.host_rate_limiter_dropped);
        	
        }
        else
        {
        	vty_out( vty, "  RxHostFrameDrop        : %-20lu\r\n",hostframeRawData.system_dropped);
        }
    }

	vty_out( vty, "\r\n");

	return iRes;
}

/*  modified by chenfj 2008-1-31
     以下三个函数执行有错, 被修改
*/
STATUS CliRealTimeStatsHostMsg( short int ponId, struct vty* vty )
{
	int iRes = 0;
	/*short int collector_id = 0;
	short int stat_parameter = 0;
	unsigned long	timestamp = 0 ;*/
	int iHaveNetStat;
	PON_host_messages_raw_data_t	hostMsgRawData;
    OLT_raw_stat_item_t    stat_item;
    short int PonManageType;
    short int PonChipType;

	PonChipType = V2R1_GetPonchipType( ponId );

    if ( OLT_PONCHIP_ISPAS(PonChipType) )
	{/*host message*/
		memset(&hostMsgRawData,0 , sizeof(PON_host_messages_raw_data_t));
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_HOST_MESSAGES;
        pon_get_init_parameters(&PonManageType);    
        switch (PonManageType)
        {
            case V2R1_PON_HOST_MANAGE_BY_ETH:
                PonManageType = PON_COMM_TYPE_ETHERNET;
                break;
            case V2R1_PON_HOST_MANAGE_BY_BUS:
                PonManageType = PON_COMM_TYPE_PARALLEL;
                break;
            case V2R1_PON_HOST_MANAGE_BY_URT:
                PonManageType = PON_COMM_TYPE_UART;
                break;
            default:
                VOS_ASSERT(0);
                PonManageType = PON_COMM_TYPE_ETHERNET;
        }
        stat_item.statistics_parameter = PonManageType;
        stat_item.statistics_data = &hostMsgRawData;
        stat_item.statistics_data_size = sizeof(hostMsgRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = -1;
		stat_parameter = 1;/*Host physical interface type (0 – Parallel, 1 – Ethernet, 2 - UART)*/
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_HOST_MESSAGES, 
										stat_parameter, 
										&hostMsgRawData,
										&timestamp
										);
#endif

		vty_out( vty, "\r\n");
		vty_out( vty, "  RxHostMsgOk  : %-20lu\r\n", hostMsgRawData.received_from_firmware_ok);
		vty_out( vty, "  TxHostMsgOk  : %-20lu\r\n", hostMsgRawData.sent_to_firmware_ok);
    	if ( PONCHIP_PAS5001 != PonChipType )
        {
    		vty_out( vty, "  TxHostMsgErr : %-20lu\r\n", hostMsgRawData.received_with_error_from_host);
        	if ( PON_COMM_TYPE_ETHERNET == PonManageType )
            {
            	PON_host_messages_octets_raw_data_t	hostMsgOctRawData;
                
        		memset(&hostMsgOctRawData,0 , sizeof(hostMsgOctRawData));
                stat_item.collector_id = PON_OLT_ID;
                stat_item.raw_statistics_type = PON_RAW_STAT_HOST_MESSAGES_OCTETS;
                stat_item.statistics_parameter = PON_COMM_TYPE_ETHERNET;
                stat_item.statistics_data = &hostMsgOctRawData;
                stat_item.statistics_data_size = sizeof(hostMsgOctRawData);
                iRes = OLT_GetRawStatistics(ponId, &stat_item);
        		vty_out( vty, "  TxHostOctetsOk  : %-20lu\r\n", hostMsgOctRawData.sent_to_firmware);
        		vty_out( vty, "  TxHostOctetsErr : %-20lu\r\n", hostMsgOctRawData.received_with_error_from_host);
            }
        }   
        
		vty_out( vty, "\r\n");
	}	
    else
    {
        vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(ponId));
    }
    
	return ROK;
}

STATUS CliRealTimeStatsOnuUpLinkBer( short int ponId, short int onuId, struct vty* vty )
{
	short int iRes = 0;
	/*short int collector_id = PON_OLT_ID;
	short int stat_parameter = 0;*/
	short int llid = 0;
    short int PonChipType;
	unsigned char buf[256] = {0};
	/*unsigned long	timestamp = 0 ;*/
	PON_onu_ber_raw_data_t	onuBerRawData;
    OLT_raw_stat_item_t    stat_item;

	llid = GetLlidByOnuIdx( ponId, onuId);
	if ((-1) == llid ) 
		return (-1);
	
	PonChipType = V2R1_GetPonchipType( ponId );

    if ( OLT_PONCHIP_ISPAS(PonChipType) )
	{
    	memset(&onuBerRawData, 0, sizeof(PON_onu_ber_raw_data_t));
#if 1
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_ONU_BER;
        stat_item.statistics_parameter = llid;
        stat_item.statistics_data = &onuBerRawData;
        stat_item.statistics_data_size = sizeof(onuBerRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    	stat_parameter = llid;
    	iRes = PAS_get_raw_statistics( ponId, 
    									collector_id, 
    									PON_RAW_STAT_ONU_BER, 
    									stat_parameter, 
    									&onuBerRawData,
    									&timestamp
    									);
#endif

    	if( OLT_CALL_ISOK(iRes) )
    	{
        	vty_out( vty, "\r\nuplink ber:\r\n");

        	memset(buf, 0, 255);
        	sprintf(buf,"  ErrorBytes : ");
        	sprintf64Bits(&buf[VOS_StrLen(buf)],(long long )onuBerRawData.error_bytes);	
        	vty_out( vty, "%s\r\n", buf );

        	memset(buf, 0, 255);
        	sprintf(buf,"  UsedBytes  : ");
        	sprintf64Bits(&buf[VOS_StrLen(buf)],(long long)onuBerRawData.used_byte_count);
        	vty_out( vty, "%s\r\n", buf);

        	memset(buf, 0, 255);
        	sprintf(buf,"  GoodBytes  : ");
        	sprintf64Bits(&buf[VOS_StrLen(buf)],onuBerRawData.good_byte_count);
        	vty_out( vty, "%s\r\n",buf);
        	
        	vty_out( vty, "\r\n");
    	}
	}
    else
    {
        vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(ponId));
    }

    return iRes;
}

STATUS CliRealTimeOltDownlinkBer( short int ponId, short int onuId, struct vty* vty  )
{
	short int iRes = 0;
	/*short int collector_id = 0;
	short int stat_parameter = 0;*/
	short int llid = 0;
    short int PonChipType;
/*	unsigned char buf[256] = {0};*/
	/*unsigned long	timestamp = 0 ;	*/
	PON_olt_ber_raw_data_t	oltBerRawData;
    OLT_raw_stat_item_t    stat_item;
    
	llid = GetLlidByOnuIdx( ponId, onuId);
	if (INVALID_LLID == llid ) 
		return (-1);
	
	PonChipType = V2R1_GetPonchipType( ponId );

    if ( OLT_PONCHIP_ISPAS(PonChipType) )
	{
    	memset(&oltBerRawData, 0, sizeof(PON_olt_ber_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_OLT_BER;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &oltBerRawData;
        stat_item.statistics_data_size = sizeof(oltBerRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    	collector_id = llid;
    	stat_parameter = 0;
    	iRes = PAS_get_raw_statistics( ponId, 
    									collector_id, 
    									PON_RAW_STAT_OLT_BER, 
    									stat_parameter, 
    									 
    									&oltBerRawData,
    									&timestamp
    									);
#endif
    	if( OLT_CALL_ISERROR(iRes) )
    		return iRes;

    	vty_out( vty, "\r\ndownlink ber\r\n");
    	vty_out( vty, "  TotalByteErr   : %-10lu\r\n", oltBerRawData.error_bytes);
    	vty_out( vty, "  TotalByteCount : %-10lu\r\n", oltBerRawData.byte_count);
    	
    	vty_out( vty, "  ByteToOnuErr   : %-10lu\r\n", oltBerRawData.error_onu_bytes);
    	vty_out( vty, "  ByteToOnuOk    : %-10lu\r\n", oltBerRawData.onu_byte_count);
    	
    	vty_out( vty, "  TotalByteToOnu : %-10lu\r\n", oltBerRawData.onu_total_byte_count);
    	vty_out( vty, "\r\n");
	}
    else
    {
        vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(ponId));
    }
	
	return PAS_EXIT_OK;
}

STATUS CliRealTimeOnuUpLinkFer( short int ponId, short int onuId, struct vty* vty )
{
	short int iRes = 0;
	/*short int collector_id = PON_OLT_ID;
	short int stat_parameter = 0;*/
	short int llid = 0;
	/*unsigned long	timestamp = 0 ;*/	
	PON_onu_fer_raw_data_t	onuFerRawData;
    OLT_raw_stat_item_t    stat_item;

	llid = GetLlidByOnuIdx( ponId, onuId);
	if ((-1) == llid ) 
		return (-1);
	
	memset(&onuFerRawData, 0, sizeof(PON_onu_fer_raw_data_t));
#if 1
    stat_item.collector_id = PON_OLT_ID;
    stat_item.raw_statistics_type = PON_RAW_STAT_ONU_FER;
    stat_item.statistics_parameter = llid;
    stat_item.statistics_data = &onuFerRawData;
    stat_item.statistics_data_size = sizeof(onuFerRawData);
    iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
	stat_parameter = llid;
	iRes = PAS_get_raw_statistics( ponId, 
									collector_id, 
									PON_RAW_STAT_ONU_FER, 
									stat_parameter, 
									&onuFerRawData,
									&timestamp
									);
#endif
	if( OLT_CALL_ISOK(iRes) )
	{
    	/*vty_out( vty, "  Onu Uplink Fer infromation\r\n");*/
    	vty_out( vty, "\r\n");
    	vty_out( vty, "  RxFrameErr     : %-10lu\r\n", onuFerRawData.received_error);
    	vty_out( vty, "  RxFrameOk      : %-10lu\r\n", onuFerRawData.received_ok);
    	vty_out( vty, "  RxByFirewareOk : %-10lu\r\n", onuFerRawData.firmware_received_ok);
    	vty_out( vty, "\r\n");
	}

    return iRes;
}

STATUS CliRealTimeOnuStatsPon( short int ponId, short int onuId, struct vty* vty  )
{
	short int iRes = 0;
	short int llid = 0;
	short int OltPonChipType;
	short int OltPonChipVendor;
	int OnuPonChipVendor;
	int OnuPonChipType;
	ULONG len = 0;
	/*short int collector_id = 0;
	short int stat_parameter = 0;*/
	unsigned char buf[256] = {0};
	/*unsigned long	timestamp = 0 ;	*/

	PON_llid_Broadcast_frames_raw_data_t  llidBrdFrameRawData;
	PON_transmitted_frames_per_llid_raw_data_t llidUniFrameSendRawData;

	PON_total_frames_raw_data_t		totalFrameRawData;
	PON_total_octets_raw_data_t		totalOctetsRawData;
	PON_total_dropped_rx_frames_raw_data_t	totalDropFrameRawData;
	PON_discarded_unknown_destination_address_raw_data_t udaDiscardRawData;
	PON_frame_check_sequence_errors_raw_data_t	frameCheckSeqRawData;
	PON_alignmnet_errors_raw_data_t		alignFrameErrRawData;
	PON_in_range_length_errors_per_llid_raw_data_t	inRangeLengthErrLlidRawData;
	PON_frame_too_long_errors_raw_data_t	frameTooLongErrRawData;
	PON_total_tx_dropped_frames_raw_data_t	totalTxDropFrameRawData;
	PON_transmitted_frames_per_priority_raw_data_t cosTxOkFramePerPrioRawData;
	PON_dropped_frames_raw_data_t	        cosTxDropFramePerPrioRawData;
	PON_host_frames_raw_data_t	hostFrameRawData;
	PON_grant_frames_raw_data_t   grantFrameRawData;
	PON_registration_frames_raw_data_t		registerFrameRawData;
	PON_unsupported_mpcp_frames_raw_data_t	mpcpUnknownFrameRawData;
	PON_oam_frames_raw_data_t		oamFrameStatRawData;
#if 1
    PON_oam_statistic_raw_data_t            standOAMRawData;
#else
	PON_oam_frames_counters_raw_data_t		oamFrameCounterRawData;
#endif
	PON_broadcast_frames_raw_data_t		broadcastFrameRawData;
	PON_multicast_frames_raw_data_t 	multicastFrameRawData;
#if 0
   	PON_unicast_multicast_pon_frames_raw_data_t umFrameRawData;
#endif
	PON_mpcp_status_raw_data_t 		MPCPstatusRawData;
	PON_received_frames_to_cpu_per_priority_raw_data_t	revFrameToCpuPrioRawData;
	PON_transmitted_frames_from_cpu_per_priority_raw_data_t		tranFrameCpuPerPrioRawData;
	PON_encrypt_frames_raw_data_t 	encryptFrameRawData;
	PON_start_end_symbols_frames_raw_data_t 	symbolFrameRawData;
	PON_total_dropped_cpu_rx_frames_raw_data_t		totalDropCpuRxFrameRawData;
	PON_hec_frames_errors_raw_data_t	FrameHecErrRawData;
	PON_host_octets_raw_data_t	hostOctetsRawData;
	PON_pause_frames_raw_data_t		pauseFrameRawData;
	PON_report_frames_raw_data_t 	reportFrameRawData;
	PON_fec_frames_raw_data_t       fecFrameRawData;
	PON_p2p_frames_raw_data_t		p2pFrameRawData;
    PON_mpcp_statistic_raw_data_t           standOAMMpcpRawData;
    PON_dropped_frames_per_llid_raw_data_t  llidDropRawData;

    OLT_raw_stat_item_t    stat_item;
	
	llid = GetLlidByOnuIdx( ponId, onuId);
	if ((-1) == llid ) 
		return (-1);

	OltPonChipType = V2R1_GetPonchipType( ponId );
    OltPonChipVendor = GetOltChipVendorID(OltPonChipType);
    OnuPonChipVendor = GetOnuChipVendor(ponId, onuId);
    OnuPonChipType = GetOnuChipType(ponId, onuId);

    if ( OltPonChipVendor != OnuPonChipVendor )
    {
    	vty_out( vty, "OltChip(%s) is not able to get the OnuChip(%s)'s statistics.\r\n", GetVendorName(OltPonChipVendor), GetVendorName(OnuPonChipVendor) );
    	return 0;
    }

	/*llid-broadcast frame*/
	{
		memset( &llidBrdFrameRawData, 0, sizeof(PON_llid_Broadcast_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_LLID_BROADCAST_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &llidBrdFrameRawData;
        stat_item.statistics_data_size = sizeof(llidBrdFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_LLID_BROADCAST_FRAMES, 
										stat_parameter, 
										&llidBrdFrameRawData,
										&timestamp
										);
#endif		
	}
    
    {/*Transmitted Frames Per LLID (8)*/
		memset( &llidUniFrameSendRawData, 0, sizeof(llidUniFrameSendRawData));
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID;
        stat_item.statistics_parameter = llid;
        stat_item.statistics_data = &llidUniFrameSendRawData;
        stat_item.statistics_data_size = sizeof(llidUniFrameSendRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
    }


	/*llid frame rev*/
	{/*PON_RAW_STAT_TOTAL_FRAMES for pon*/
		memset( &totalFrameRawData, 0, sizeof( PON_total_frames_raw_data_t ) );
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_FRAMES;
        stat_item.statistics_parameter = ONU_PHYSICAL_PORT_PON;
        stat_item.statistics_data = &totalFrameRawData;
        stat_item.statistics_data_size = sizeof(totalFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 1;/*port id : 1- PON, 2 - system*/
		iRes = PAS_get_raw_statistics(  ponId, 
										collector_id, 
										PON_RAW_STAT_TOTAL_FRAMES, 
										stat_parameter, 
										&totalFrameRawData,
										&timestamp
										);
#endif		
	}	

	
	{/*Total octets*/
		memset(&totalOctetsRawData , 0, sizeof(PON_total_octets_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_OCTETS;
        stat_item.statistics_parameter = ONU_PHYSICAL_PORT_PON;
        stat_item.statistics_data = &totalOctetsRawData;
        stat_item.statistics_data_size = sizeof(totalOctetsRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 1;/*port id : 1- PON, 2 - system*/
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_TOTAL_OCTETS, 
										stat_parameter, 
										&totalOctetsRawData,
										&timestamp
										);
#endif		
	}

    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
	{/*total Rxdrop frame-11*/
		memset(&totalDropFrameRawData, 0, sizeof(PON_total_dropped_rx_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &totalDropFrameRawData;
        stat_item.statistics_data_size = sizeof(totalDropFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES, 
										stat_parameter, 
										&totalDropFrameRawData,
										&timestamp
										);
#endif		
	}
	
	{/*total Tx drop frame-12*/
		memset(&totalTxDropFrameRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &totalTxDropFrameRawData;
        stat_item.statistics_data_size = sizeof(totalTxDropFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
										stat_parameter, 
										&totalTxDropFrameRawData,
										&timestamp
										);
#endif		
	}
    
    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
    {
    	{/*Transmitt Frame per llid priority*/
    		memset(&cosTxOkFramePerPrioRawData, 0, sizeof(PON_transmitted_frames_per_priority_raw_data_t));
#if 1
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY;
            stat_item.statistics_parameter = ONU_PHYSICAL_PORT_PON;
            stat_item.statistics_data = &cosTxOkFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosTxOkFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = llid;
    		stat_parameter = 1;/*1-pon  2- system*/
    		
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY, 
    										stat_parameter, 
    										&TxFramePerPrioRawData,
    										&timestamp
    										);
#endif		
    	}
	
    	{/*frame dropped per priorety*/
    		memset(&cosTxDropFramePerPrioRawData, 0, sizeof(PON_dropped_frames_raw_data_t));
#if 1
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_DROPPED_FRAMES;
            stat_item.statistics_parameter = ONU_PHYSICAL_PORT_PON;
            stat_item.statistics_data = &cosTxDropFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosTxDropFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = llid;
    		stat_parameter = 1;/*1- pon 2 - system*/

    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_DROPPED_FRAMES, 
    										stat_parameter, 
    										&cosTxDropFramePerPrioRawData,
    										&timestamp
    										);
#endif		
    	}

    	if ( PONCHIP_PAS5001 != OltPonChipType )
        {
    		memset(&udaDiscardRawData, 0, sizeof(udaDiscardRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_DISCARDED_UNKNOWN_DESTINATION_ADDRESS;
            stat_item.statistics_parameter = llid;
            stat_item.statistics_data = &udaDiscardRawData;
            stat_item.statistics_data_size = sizeof(udaDiscardRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
        }   
    }   
	
	{/*check sequence error*/
		memset(&frameCheckSeqRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &frameCheckSeqRawData;
        stat_item.statistics_data_size = sizeof(frameCheckSeqRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS, 
										stat_parameter, 
										&frameCheckSeqRawData,
										&timestamp
										);
#endif		
	}
	
	{/*Alignment errors*/
		memset(&alignFrameErrRawData, 0, sizeof(PON_alignmnet_errors_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_ALIGNMENT_ERRORS;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &alignFrameErrRawData;
        stat_item.statistics_data_size = sizeof(alignFrameErrRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_ALIGNMENT_ERRORS, 
										stat_parameter, 
										&alignFrameErrRawData,
										&timestamp
										);
#endif		
	}
    
	{/*In range length error*/
		memset(&inRangeLengthErrLlidRawData, 0, sizeof(PON_in_range_length_errors_per_llid_raw_data_t));
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID;
        stat_item.statistics_parameter = llid;
        stat_item.statistics_data = &inRangeLengthErrLlidRawData;
        stat_item.statistics_data_size = sizeof(inRangeLengthErrLlidRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
	}
	
	{/*Frame too long err -19*/
		memset(&frameTooLongErrRawData, 0, sizeof(PON_frame_too_long_errors_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_TOO_LONG_ERRORS;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &frameTooLongErrRawData;
        stat_item.statistics_data_size = sizeof(frameTooLongErrRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_FRAME_TOO_LONG_ERRORS, 
										stat_parameter, 
										&frameTooLongErrRawData,
										&timestamp
										);
#endif		
	}
	
	memset(&FrameHecErrRawData, 0, sizeof(PON_hec_frames_errors_raw_data_t));
    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
	{/*HEC err*/
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_HEC_FRAMES_ERRORS;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &FrameHecErrRawData;
        stat_item.statistics_data_size = sizeof(FrameHecErrRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_HEC_FRAMES_ERRORS, 
										stat_parameter, 
										&FrameHecErrRawData,
										&timestamp
										);
#endif		
	}

    {/*P2P Frames (34)*/
    	memset(&p2pFrameRawData, 0, sizeof(p2pFrameRawData));
        stat_item.collector_id = PON_OLT_ID;
        stat_item.raw_statistics_type  = PON_RAW_STAT_P2P_FRAMES;
        stat_item.statistics_parameter = llid;
        stat_item.statistics_data = &p2pFrameRawData;
        stat_item.statistics_data_size = sizeof(p2pFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
    }

	
    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
	{/*host frame*/
		memset(&hostFrameRawData, 0, sizeof(PON_host_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_HOST_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &hostFrameRawData;
        stat_item.statistics_data_size = sizeof(hostFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_HOST_FRAMES, 
										stat_parameter, 
										 
										&hostFrameRawData,
										&timestamp
										);	
#endif		
	}

	
#if 0
	{/*host octets*/
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_HOST_OCTETS;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &hostOctetsRawData;
        stat_item.statistics_data_size = sizeof(hostOctetsRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		memset(&hostOctetsRawData, 0, sizeof(PON_host_octets_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_HOST_OCTETS, 
										stat_parameter, 
										 
										&hostOctetsRawData,
										&timestamp
										);
#endif		
	}
#endif		

	

	{/*pause frame*/
		memset(&pauseFrameRawData, 0, sizeof(PON_pause_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_PAUSE_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &pauseFrameRawData;
        stat_item.statistics_data_size = sizeof(pauseFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_PAUSE_FRAMES, 
										stat_parameter, 
										 
										&pauseFrameRawData,
										&timestamp
										);
#endif		
	}
 
	
	{/*report frame transmitt*/
		memset(&reportFrameRawData, 0, sizeof(PON_report_frames_raw_data_t));
#if 1
        stat_item.collector_id = PON_OLT_AND_ONU_COMBINED_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_REPORT_FRAMES;
        stat_item.statistics_parameter = llid;
        stat_item.statistics_data = &reportFrameRawData;
        stat_item.statistics_data_size = sizeof(reportFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_REGISTRATION_FRAMES, 
										stat_parameter, 
										 
										&reportFrameRawData,
										&timestamp
										);
#endif		
	}


	
	{/*grant frame*/
		memset(&grantFrameRawData, 0, sizeof(PON_grant_frames_raw_data_t));
#if 1
        stat_item.collector_id = PON_OLT_AND_ONU_COMBINED_ID;
        stat_item.raw_statistics_type = PON_RAW_STAT_GRANT_FRAMES;
        stat_item.statistics_parameter = llid;
        stat_item.statistics_data = &grantFrameRawData;
        stat_item.statistics_data_size = sizeof(grantFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = (PON_OLT_AND_ONU_COMBINED_ID);
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_GRANT_FRAMES, 
										stat_parameter, 
										
										&grantFrameRawData,
										&timestamp
										);
#endif		
	}

	
	{/*Registration frame*/
		memset(&registerFrameRawData, 0, sizeof(PON_registration_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_REGISTRATION_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &registerFrameRawData;
        stat_item.statistics_data_size = sizeof(registerFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_REGISTRATION_FRAMES, 
										stat_parameter, 
										 
										&registerFrameRawData,
										&timestamp
										);
#endif		
	}

    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
    {
		memset(&mpcpUnknownFrameRawData, 0, sizeof(mpcpUnknownFrameRawData));
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_UNSUPPORTED_MPCP_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &mpcpUnknownFrameRawData;
        stat_item.statistics_data_size = sizeof(mpcpUnknownFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);        
    }


	memset(&standOAMMpcpRawData, 0, sizeof(standOAMMpcpRawData));
    stat_item.collector_id = llid;
    stat_item.raw_statistics_type = PON_RAW_STAT_STANDARD_OAM_MPCP_STATISTIC;
    stat_item.statistics_parameter = 0;
    stat_item.statistics_data = &standOAMMpcpRawData;
    stat_item.statistics_data_size = sizeof(standOAMMpcpRawData);
    iRes = OLT_GetRawStatistics(ponId, &stat_item);        

	
	{/*oam frame Rx&Tx*/
		memset(&oamFrameStatRawData, 0, sizeof(PON_oam_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_OAM_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &oamFrameStatRawData;
        stat_item.statistics_data_size = sizeof(oamFrameStatRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_OAM_FRAMES, 
										stat_parameter, 
										 
										&oamFrameStatRawData,
										&timestamp
										);
#endif		
	}

    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
	{/*oam information*/
#if 1
		memset(&standOAMRawData, 0, sizeof(PON_oam_statistic_raw_data_t));
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_STANDARD_OAM_STATISTIC;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &standOAMRawData;
        stat_item.statistics_data_size = sizeof(standOAMRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);        
#else       
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_OAM_FRAMES_COUNTERS;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &oamFrameCounterRawData;
        stat_item.statistics_data_size = sizeof(oamFrameCounterRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		
		memset(&oamFrameCounterRawData, 0, sizeof(PON_oam_frames_counters_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_OAM_FRAMES_COUNTERS, 
										stat_parameter, 
										 
										&oamFrameCounterRawData,
										&timestamp
										);
#endif		
#endif		
	}

	
	{/*multicast frame*/
		memset(&multicastFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_MULTICAST_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &multicastFrameRawData;
        stat_item.statistics_data_size = sizeof(multicastFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_MULTICAST_FRAMES, 
										stat_parameter, 
										 
										&multicastFrameRawData,
										&timestamp
										);
#endif		
	}

	
	{/*brd frame*/
		memset(&broadcastFrameRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_BROADCAST_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &broadcastFrameRawData;
        stat_item.statistics_data_size = sizeof(broadcastFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_BROADCAST_FRAMES, 
										stat_parameter, 
										 
										&broadcastFrameRawData,
										&timestamp
										);
#endif		
	}
    
    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
    {
        if ( PONCHIP_PAS5001 != OltPonChipType )
        {
#if 0
    		memset(&umFrameRawData, 0, sizeof(umFrameRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_UNICAST_MULTICAST_PON_FRAMES;
            stat_item.statistics_parameter = llid;
            stat_item.statistics_data = &umFrameRawData;
            stat_item.statistics_data_size = sizeof(umFrameRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#endif
    		memset(&fecFrameRawData, 0, sizeof(fecFrameRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_FEC_FRAMES;
            stat_item.statistics_parameter = llid;
            stat_item.statistics_data = &fecFrameRawData;
            stat_item.statistics_data_size = sizeof(fecFrameRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
        }
    	
    	{/*MPCP status*/
    		memset(&MPCPstatusRawData, 0, sizeof(PON_mpcp_status_raw_data_t));
#if 1
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_MPCP_STATUS;
            stat_item.statistics_parameter = 0;
            stat_item.statistics_data = &MPCPstatusRawData;
            stat_item.statistics_data_size = sizeof(MPCPstatusRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = llid;
    		stat_parameter = 0;
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_MPCP_STATUS, 
    										stat_parameter, 
    										 
    										&MPCPstatusRawData,
    										&timestamp
    										);
#endif		
    	}

    	{/*Revceived frame to CPU per priority*/
    		memset(&revFrameToCpuPrioRawData, 0, sizeof(PON_received_frames_to_cpu_per_priority_raw_data_t));
#if 1
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_RECEIVED_FRAMES_TO_CPU_PER_PRIORITY;
            stat_item.statistics_parameter = ONU_PHYSICAL_PORT_PON;
            stat_item.statistics_data = &revFrameToCpuPrioRawData;
            stat_item.statistics_data_size = sizeof(revFrameToCpuPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = llid;
    		stat_parameter = 1;/*1 - pon, 2 - system*/
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_RECEIVED_FRAMES_TO_CPU_PER_PRIORITY, 
    										stat_parameter, 
    										 
    										&revFrameToCpuPrioRawData,
    										&timestamp
    										);
#endif		
    	}
    
    	{/*Transmitt frame to CPU per priority*/
    		memset(&tranFrameCpuPerPrioRawData, 0, sizeof(PON_transmitted_frames_from_cpu_per_priority_raw_data_t));
#if 1
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_FROM_CPU_PER_PRIORITY;
            stat_item.statistics_parameter = ONU_PHYSICAL_PORT_PON;
            stat_item.statistics_data = &tranFrameCpuPerPrioRawData;
            stat_item.statistics_data_size = sizeof(tranFrameCpuPerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = llid;
    		stat_parameter = 1;/*1 - pon*/
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_TRANSMITTED_FRAMES_FROM_CPU_PER_PRIORITY, 
    										stat_parameter, 
    										 
    										&tranFrameCpuPerPrioRawData,
    										&timestamp
    										);
#endif		
    	}
    	
    	{/*cpu rx dropped*/
    		memset(&totalDropCpuRxFrameRawData, 0, sizeof(PON_total_dropped_cpu_rx_frames_raw_data_t));
#if 1
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_CPU_RX_FRAMES;
            stat_item.statistics_parameter = 0;
            stat_item.statistics_data = &totalDropCpuRxFrameRawData;
            stat_item.statistics_data_size = sizeof(totalDropCpuRxFrameRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = llid;
    		stat_parameter = 0;
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_TOTAL_DROPPED_CPU_RX_FRAMES, 
    										stat_parameter, 
    										 
    										&totalDropCpuRxFrameRawData,
    										&timestamp
    										);
#endif		
    	}
    	
    	{/*encrypt - 65*/
    		memset(&encryptFrameRawData, 0, sizeof(PON_encrypt_frames_raw_data_t));
#if 1
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_ENCRYPT_FRAMES;
            stat_item.statistics_parameter = 0;
            stat_item.statistics_data = &encryptFrameRawData;
            stat_item.statistics_data_size = sizeof(encryptFrameRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = llid;
    		stat_parameter = 0;
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_ENCRYPT_FRAMES, 
    										stat_parameter, 
    										 
    										&encryptFrameRawData,
    										&timestamp
    										);
#endif		
    	}
    	
    	{/*Start, End Symbols Frames (66)*/
    		memset(&symbolFrameRawData, 0, sizeof(symbolFrameRawData));
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_START_END_SYMBOL_FRAMES;
            stat_item.statistics_parameter = 0;
            stat_item.statistics_data = &symbolFrameRawData;
            stat_item.statistics_data_size = sizeof(symbolFrameRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
    	}
        
        {
    		memset(&llidDropRawData, 0, sizeof(llidDropRawData));
    		stat_item.collector_id = PON_OLT_ID;
    	    stat_item.raw_statistics_type = PON_RAW_STAT_DROPPED_FRAMES_PER_LLID;
            stat_item.statistics_parameter = llid;
    	    stat_item.statistics_data = &llidDropRawData;
    	    stat_item.statistics_data_size = sizeof(llidDropRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
        }
    }


	/*for Rx*/
	vty_out( vty, "\r\n");
	
	vty_out( vty, "  --------------------------------------Total Sum---------------------------------\r\n");
	vty_out( vty, "  RxTotalFrameOk         : %-20lu", totalFrameRawData.received_ok );
	vty_out( vty, "  TxTotalFrameOk         : %-20lu\r\n", totalFrameRawData.transmitted_ok);
	
	len = sprintf(buf, "  RxOctetsOk             : " );	
	sprintf64Bits( buf+len, (unsigned long long)totalOctetsRawData.received_ok );
	vty_out( vty, "%s",buf);

	len = sprintf(buf, "  TxOctetsOk             : " );		
	sprintf64Bits( buf+len, (unsigned long long )totalOctetsRawData.transmitted_ok );
	vty_out( vty, "%s",buf);
	vty_out( vty, "\r\n" );	

#if 0
	if ( PONCHIP_PAS5001 != OltPonChipType )
    {
    	vty_out( vty, "  RxUnicastFrame     : %-20lu",umFrameRawData.tx_unicast_frames);
    	vty_out( vty, "  TxUnicastFrame     : %-20lu\r\n",umFrameRawData.rx_unicast_frames);

    	vty_out( vty, "  RxMulticastFrame   : %-20lu",umFrameRawData.tx_multicast_frames);
    	vty_out( vty, "  TxMulticastFrame   : %-20lu\r\n",umFrameRawData.rx_multicast_frames);
    }
#endif

	vty_out( vty, "  RxMulticastFrameOk     : %-20lu",multicastFrameRawData.received_ok);
	vty_out( vty, "  TxMulticastFrameOk     : %-20lu\r\n",multicastFrameRawData.transmitted_ok);
	vty_out( vty, "  RxBroadcastFrameOk     : %-20lu",broadcastFrameRawData.received_ok);
	vty_out( vty, "  TxBroadcastFrameOk     : %-20lu\r\n",broadcastFrameRawData.transmitted_ok);


	vty_out( vty, "\r\n  TxFrameDrop            : %-20lu\r\n",totalTxDropFrameRawData.total_pon_dropped);
	if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
    {
        if ( PONCHIP_PAS5001 != OltPonChipType )
        {
            vty_out( vty, "  DropForUnknownSA       : %-20lu",llidDropRawData.address_table_dropped);
            vty_out( vty, "  DropForNotGrant        : %-20lu\r\n",llidDropRawData.mismatch_dropped);

            vty_out( vty, "  DropForUnknownDA       : %-20lu",udaDiscardRawData.frames);
            vty_out( vty, "  DropForInRangeErr      : %-20lu\r\n",inRangeLengthErrLlidRawData.received);

        	vty_out( vty, "  TxFrameOKPerPrio0      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[0]);
        	vty_out( vty, "  TxDropPerPrio0         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[0]);
        	vty_out( vty, "  TxFrameOKPerPrio1      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[1]);
        	vty_out( vty, "  TxDropPerPrio1         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[1]);
        	vty_out( vty, "  TxFrameOKPerPrio2      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[2]);
        	vty_out( vty, "  TxDropPerPrio2         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[2]);
        	vty_out( vty, "  TxFrameOKPerPrio3      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[3]);
        	vty_out( vty, "  TxDropPerPrio3         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[3]);
        	vty_out( vty, "  TxFrameOKPerPrio4      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[4]);
        	vty_out( vty, "  TxDropPerPrio4         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[4]);
        	vty_out( vty, "  TxFrameOKPerPrio5      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[5]);
        	vty_out( vty, "  TxDropPerPrio5         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[5]);
        	vty_out( vty, "  TxFrameOKPerPrio6      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[6]);
        	vty_out( vty, "  TxDropPerPrio6         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[6]);
        	vty_out( vty, "  TxFrameOKPerPrio7      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[7]);
        	vty_out( vty, "  TxDropPerPrio7         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[7]);
        }
        else
        {
            vty_out( vty, "  DropForNotGrant        : %-20lu\r\n",llidDropRawData.mismatch_dropped);
        }
    }

	vty_out( vty, "\r\n  RxFrameDrop            : --\r\n");
	vty_out( vty, "  RxDropForInvalidLlid   : %-20lu",llidBrdFrameRawData.invalid_llid_error_frames_received);
	vty_out( vty, "  RxDropForBrdLlidErr    : %-20lu\r\n",llidBrdFrameRawData.llid_broadcast_error_frames_received);
    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
    {
    	vty_out( vty, "  RxDropForSAInUNI       : %-20lu",totalDropFrameRawData.pon_match);
    	vty_out( vty, "  --\r\n");
    }


	vty_out( vty, "  --------------------------------------Rx Error------------------------------------\r\n");
	vty_out( vty, "  RxFrameTotal           : %-20lu",totalFrameRawData.received_ok + totalFrameRawData.received_error + FrameHecErrRawData.received );
	vty_out( vty, "  RxFrameErr             : %-20lu\r\n",totalFrameRawData.received_error + FrameHecErrRawData.received );

	len = sprintf(buf, "  RxOctetsTotal          : " );	
	sprintf64Bits( buf+len, (unsigned long long)totalOctetsRawData.received_total );
	vty_out( vty, "%s",buf);

	len = sprintf(buf, "  RxOctetsErr            : " );	
	sprintf64Bits( buf+len, (unsigned long long)totalOctetsRawData.received_error );
	vty_out( vty, "%s\r\n",buf);

	vty_out( vty, "  RxFrameTooLongErr      : %-20lu",frameTooLongErrRawData.pon_received);
	vty_out( vty, "  --\r\n");
	vty_out( vty, "  RxFrameHECErr          : %-20lu",FrameHecErrRawData.received);
	vty_out( vty, "  RxFrameFCSErr          : %-20lu\r\n",totalFrameRawData.received_error);
	vty_out( vty, "  RxFrameFCSAlignErr     : %-20lu", alignFrameErrRawData.received );
	vty_out( vty, "  RxFrameFCSCrcErr       : %-20lu\r\n",frameCheckSeqRawData.received);
    if ( OLT_PONCHIP_ISPAS(OltPonChipType) && (PONCHIP_PAS5001 != OltPonChipType) )
    {
    	vty_out( vty, "  TxNeedFECFixed         : %-20lu",fecFrameRawData.wrong_parity_number);
    	vty_out( vty, "  TxNotNeedFECFixed      : %-20lu\r\n",fecFrameRawData.good_frames);
    	vty_out( vty, "  TxFECFixedOK           : %-20lu",fecFrameRawData.fixed_frames);
    	vty_out( vty, "  TxFECFixedErr          : %-20lu\r\n",fecFrameRawData.unfixed_frames);
    }

    
	vty_out( vty, "  --------------------------------------MPCP----------------------------------------\r\n");
    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
    {
    	vty_out( vty, "  RxUnknownMPCPFrame     : %-20lu",mpcpUnknownFrameRawData.received);
    	vty_out( vty, "  RxUnsupportedMPCPFrame : %-20lu\r\n",MPCPstatusRawData.unsupported);
    }
	vty_out( vty, "  RxGrantFrameOk         : %-20lu",grantFrameRawData.received_ok);
	vty_out( vty, "  TxGrantFrameOk         : %-20lu\r\n",grantFrameRawData.transmitted_dba_ok + grantFrameRawData.transmitted_ctrl_ok);
	vty_out( vty, "  RxDbaGrantFrame        : %-20lu",grantFrameRawData.transmitted_dba_ok);
	vty_out( vty, "  RxRegisterGrantFrame   : %-20lu\r\n",grantFrameRawData.transmitted_ctrl_ok);
	vty_out( vty, "  RxReportFrameOk        : %-20lu",reportFrameRawData.received_ok);
	vty_out( vty, "  TxReportFrameOk        : %-20lu\r\n",reportFrameRawData.transmitted_ok);
	vty_out( vty, "  RxRegisterFrame        : %-20lu",registerFrameRawData.register_received_ok);
	vty_out( vty, "  TxRegisterRequest      : %-20lu\r\n",registerFrameRawData.register_request_transmitted_ok);
	vty_out( vty, "  --                     : --                  ");
	vty_out( vty, "  TxRegisterAck          : %-20lu\r\n",registerFrameRawData.register_ack_transmitted_ok);

	vty_out( vty, "\r\n  RxMpcpFrame            : %-20lu",standOAMMpcpRawData.mac_ctrl_frames_received);
	vty_out( vty, "  TxMpcpFrame            : %-20lu\r\n",standOAMMpcpRawData.mac_ctrl_frames_transmitted);
	vty_out( vty, "  RxMpcpDiscoveryTimeout : %-20lu",standOAMMpcpRawData.discovery_timeout);
	vty_out( vty, "  TxMpcpDiscoveryWindow  : %-20lu\r\n",standOAMMpcpRawData.discovery_windows_sent);
	vty_out( vty, "  RxMpcpReport           : %-20lu",standOAMMpcpRawData.received_report);
	vty_out( vty, "  TxMpcpReport           : %-20lu\r\n",standOAMMpcpRawData.transmitted_report);
	vty_out( vty, "  RxMpcpGate             : %-20lu",standOAMMpcpRawData.receive_gate);
	vty_out( vty, "  TxMpcpGate             : %-20lu\r\n",standOAMMpcpRawData.transmited_gate);
	vty_out( vty, "  RxMpcpRegister         : %-20lu",standOAMMpcpRawData.register_received);
	vty_out( vty, "  TxMpcpRegister         : %-20lu\r\n",standOAMMpcpRawData.register_transmited);
	vty_out( vty, "  RxMpcpRegisterRequest  : %-20lu",standOAMMpcpRawData.register_request_received_ok);
	vty_out( vty, "  TxMpcpRegisterRequest  : %-20lu\r\n",standOAMMpcpRawData.register_request_transmitted_ok);
	vty_out( vty, "  RxMpcpRegisterAck      : %-20lu",standOAMMpcpRawData.register_ack_received_ok);
	vty_out( vty, "  TxMpcpRegisterAck      : %-20lu\r\n",standOAMMpcpRawData.register_ack_transmitted_ok);
   

	vty_out( vty, "  --------------------------------------OAM-----------------------------------------\r\n");
	vty_out( vty, "  RxOamFrameOk           : %-20lu",oamFrameStatRawData.received_ok);
	vty_out( vty, "  TxOamFrameOk           : %-20lu\r\n",oamFrameStatRawData.transmitted_ok);	
    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
    {
    	vty_out( vty, "  RxOamInfoCode          : %-20lu",standOAMRawData.information_rx);
    	vty_out( vty, "  TxOamInfoCode          : %-20lu\r\n",standOAMRawData.information_tx);
    	vty_out( vty, "  RxOamUniqueEventNotify : %-20lu",standOAMRawData.unique_event_notification_rx);
    	vty_out( vty, "  TxOamUniqueEventNotify : %-20lu\r\n",standOAMRawData.unique_event_notification_tx);
    	vty_out( vty, "  RxOamDupEventNotify    : %-20lu",standOAMRawData.duplicate_event_notification_rx);
    	vty_out( vty, "  TxOamDupEventNotify    : %-20lu\r\n",standOAMRawData.duplicate_event_notification_tx);
    	vty_out( vty, "  RxOamLoopbackCheck     : %-20lu",standOAMRawData.oam_loopback_control_rx);
    	vty_out( vty, "  TxOamLoopbackCheck     : %-20lu\r\n",standOAMRawData.oam_loopback_control_tx);
    	vty_out( vty, "  RxOamVarRequest        : %-20lu",standOAMRawData.oam_variable_request_rx);
    	vty_out( vty, "  TxOamVarRequest        : %-20lu\r\n",standOAMRawData.oam_variable_request_tx);
    	vty_out( vty, "  RxOamVarResponse       : %-20lu",standOAMRawData.oam_organization_specific_tx);
    	vty_out( vty, "  TxOamVarResponse       : %-20lu\r\n",standOAMRawData.oam_variable_response_tx);
    	vty_out( vty, "  RxOamOrgSpec           : %-20lu",standOAMRawData.oam_organization_specific_rx);
    	vty_out( vty, "  TxOamOrgSpec           : %-20lu\r\n",standOAMRawData.oam_organization_specific_tx);
    	vty_out( vty, "  RxOamUnsupportedCode   : %-20lu",standOAMRawData.unsupported_codes_rx);
    	vty_out( vty, "  TxOamUnsupportedCode   : %-20lu\r\n",standOAMRawData.unsupported_codes_tx);
    	vty_out( vty, "  --                     : --                  ");
    	vty_out( vty, "  TxOamDropForInnerError : %-20lu\r\n",standOAMRawData.frames_lost_due_to_oam_error);
    }
	

	vty_out( vty, "  --------------------------------------DataPath------------------------------------\r\n");
	vty_out( vty, "  RxUniLlidFrameOk       : %-20lu",llidUniFrameSendRawData.pon_ok);
	vty_out( vty, "  TxUniLlidUplinkFrameOk : %-20lu\r\n",llidUniFrameSendRawData.system_ok);
	vty_out( vty, "  RxValidUniLlidFrame    : %-20lu", llidBrdFrameRawData.llid_frames_received);
	vty_out( vty, "  --\r\n");
	vty_out( vty, "  RxValidBrdLlidFrame    : %-20lu",llidBrdFrameRawData.broadcast_frames_received);	
	vty_out( vty, "  --\r\n\r\n");
	vty_out( vty, "  Txp2pFrameOk           : %-20lu",p2pFrameRawData.received_ok);
	vty_out( vty, "  Rxp2pFrameOk           : %-20lu\r\n",p2pFrameRawData.transmitted_ok);
	vty_out( vty, "  p2pFrameDrop           : %-20lu\r\n",p2pFrameRawData.dropped_by_policer + p2pFrameRawData.dropped_by_access_control);
	vty_out( vty, "  p2pDropForAccessCtrl   : %-20lu",p2pFrameRawData.dropped_by_access_control);
	vty_out( vty, "  p2pDropForPolicer      : %-20lu\r\n",p2pFrameRawData.dropped_by_policer);

	vty_out( vty, "\r\n  RxPauseFrameOk         : %-20lu", pauseFrameRawData.pon_received_ok);
	vty_out( vty, "  TxPauseFrameOk         : %-20lu\r\n", pauseFrameRawData.pon_transmitted_ok);

    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
    {
    	vty_out( vty, "  RxEncryptFrameOk       : %-20lu",encryptFrameRawData.decrypt);
    	vty_out( vty, "  TxEncryptFrameOk       : %-20lu\r\n",encryptFrameRawData.encrypt);
    	vty_out( vty, "  RxStartSymbolFrameOk   : %-20lu",symbolFrameRawData.s_symbol);
    	vty_out( vty, "  RxEndSymbolFrameOk     : %-20lu\r\n",symbolFrameRawData.t_symbol);


    	vty_out( vty, "  --------------------------------------ManagePath----------------------------------\r\n");
    	vty_out( vty, "  RxHostFrameOk          : %-20lu",hostFrameRawData.pon_received_ok);	
    	vty_out( vty, "  TxHostframeOk          : %-20lu\r\n",hostFrameRawData.pon_transmitted_ok);
    	vty_out( vty, "  RxHostFrameDropForBuff : %-20lu",hostFrameRawData.pon_dropped);
    	vty_out( vty, "  RxHostFrameDropForQueue: %-20lu\r\n",totalDropCpuRxFrameRawData.total_pon_dropped);
    	
    	vty_out( vty, "  RxHostFramePerPrio0    : %-20lu",tranFrameCpuPerPrioRawData.transmitted_ok[0]);
    	vty_out( vty, "  TxHostFramePerPrio0    : %-20lu\r\n",revFrameToCpuPrioRawData.received_ok[0]);

    	vty_out( vty, "  RxHostFramePerPrio1    : %-20lu",tranFrameCpuPerPrioRawData.transmitted_ok[1]);
    	vty_out( vty, "  TxHostFramePerPrio1    : %-20lu\r\n",revFrameToCpuPrioRawData.received_ok[1]);
    	
    	vty_out( vty, "  RxHostFramePerPrio2    : %-20lu",tranFrameCpuPerPrioRawData.transmitted_ok[2]);
    	vty_out( vty, "  TxHostFramePerPrio2    : %-20lu\r\n",revFrameToCpuPrioRawData.received_ok[2]);
    	
    	vty_out( vty, "  RxHostFramePerPrio3    : %-20lu",tranFrameCpuPerPrioRawData.transmitted_ok[3]);
    	vty_out( vty, "  TxHostFramePerPrio3    : %-20lu\r\n",revFrameToCpuPrioRawData.received_ok[3]);
    	
    	vty_out( vty, "  RxHostFramePerPrio4    : %-20lu",tranFrameCpuPerPrioRawData.transmitted_ok[4]);
    	vty_out( vty, "  TxHostFramePerPrio4    : %-20lu\r\n",revFrameToCpuPrioRawData.received_ok[4]);
    	
    	vty_out( vty, "  RxHostFramePerPrio5    : %-20lu",tranFrameCpuPerPrioRawData.transmitted_ok[5]);
    	vty_out( vty, "  TxHostFramePerPrio5    : %-20lu\r\n",revFrameToCpuPrioRawData.received_ok[5]);
    	
    	vty_out( vty, "  RxHostFramePerPrio6    : %-20lu",tranFrameCpuPerPrioRawData.transmitted_ok[6]);
    	vty_out( vty, "  TxHostFramePerPrio6    : %-20lu\r\n",revFrameToCpuPrioRawData.received_ok[6]);
    	
    	vty_out( vty, "  RxHostFramePerPrio7    : %-20lu",tranFrameCpuPerPrioRawData.transmitted_ok[7]);
    	vty_out( vty, "  TxHostFramePerPrio7    : %-20lu\r\n",revFrameToCpuPrioRawData.received_ok[7]);	
    }
       
	vty_out( vty, "\r\n");
	
	return 0;
}


STATUS CliRealTimeOnuStatsCNI( short int ponId, short int onuId, struct vty* vty  )
{
	short int iRes = 0;
	short int llid = 0;
	short int OltPonChipType;
	short int OltPonChipVendor;
	int OnuPonChipVendor;
	int OnuPonChipType;
	ULONG len = 0;
	/*short int collector_id = 0;
	short int stat_parameter = 0;*/
	unsigned char buf[256];
	/*unsigned long	timestamp = 0 ;	*/
	PON_total_frames_raw_data_t		totalFrameRawData;
	PON_total_octets_raw_data_t		totalOctetsRawData;
	PON_frame_too_long_errors_raw_data_t	frameTooLongErrRawData;
	PON_transmitted_frames_per_priority_raw_data_t cosTxOkFramePerPrioRawData;
	PON_dropped_frames_raw_data_t	        cosTxDropFramePerPrioRawData;

#if 0
	PON_host_frames_raw_data_t		hostFrameRawData;
#endif
	PON_pause_frames_raw_data_t		pauseFrameRawData;
	PON_bridge_frames_raw_data_t	bridgeFrameRawData;
	PON_received_frames_to_cpu_per_priority_raw_data_t revFrameToCpuPerPrioRawData;
	PON_total_tx_dropped_frames_raw_data_t	 totalTxDropFrameRawData;
	PON_total_dropped_cpu_rx_frames_raw_data_t	totalCpuRxFrameDropRawData;
	PON_cpu_ports_frames_raw_data_t	totalCpuFrameRawData;

    OLT_raw_stat_item_t    stat_item;

	llid = GetLlidByOnuIdx( ponId, onuId);
	if ((-1) == llid ) 
		return (-1);

	OltPonChipType = V2R1_GetPonchipType( ponId );
    OltPonChipVendor = GetOltChipVendorID(OltPonChipType);
    OnuPonChipVendor = GetOnuChipVendor(ponId, onuId);
    OnuPonChipType = GetOnuChipType(ponId, onuId);

    if ( OltPonChipVendor != OnuPonChipVendor )
    {
    	vty_out( vty, "OltChip(%s) is not able to get the OnuChip(%s)'s statistics.\r\n", GetVendorName(OltPonChipVendor), GetVendorName(OnuPonChipVendor) );
    	return 0;
    }
    
	{/*PON_RAW_STAT_TOTAL_FRAMES for pon*/
		memset(&totalFrameRawData, 0, sizeof(totalFrameRawData));
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_FRAMES;
        stat_item.statistics_parameter = ONU_PHYSICAL_PORT_SYSTEM;
        stat_item.statistics_data = &totalFrameRawData;
        stat_item.statistics_data_size = sizeof(totalFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
	}	
    
	{/*Total octets*/
		memset(&totalOctetsRawData, 0, sizeof(totalOctetsRawData));
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_OCTETS;
        stat_item.statistics_parameter = ONU_PHYSICAL_PORT_SYSTEM;
        stat_item.statistics_data = &totalOctetsRawData;
        stat_item.statistics_data_size = sizeof(totalOctetsRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
	}
	
	{/*total dropped frame*/
		memset(&totalTxDropFrameRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &totalTxDropFrameRawData;
        stat_item.statistics_data_size = sizeof(totalTxDropFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
										stat_parameter, 
										 
										&totalTxDropFrameRawData,
										&timestamp
										);
#endif
	}


	{/*Frame too long err - 19*/
		memset(&frameTooLongErrRawData, 0, sizeof(PON_frame_too_long_errors_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_TOO_LONG_ERRORS;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &frameTooLongErrRawData;
        stat_item.statistics_data_size = sizeof(frameTooLongErrRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_FRAME_TOO_LONG_ERRORS, 
										stat_parameter, 
										 
										&frameTooLongErrRawData,
										&timestamp
										);
#endif
	}


#if 0
	{/*host frame*/
		memset(&hostFrameRawData, 0, sizeof(PON_host_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_HOST_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &hostFrameRawData;
        stat_item.statistics_data_size = sizeof(hostFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_HOST_FRAMES, 
										stat_parameter, 
										 
										&hostFrameRawData,
										&timestamp
										);	
#endif
	}
#endif

	
	{/*Pause frame*/
		memset(&pauseFrameRawData, 0, sizeof(PON_pause_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_PAUSE_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &pauseFrameRawData;
        stat_item.statistics_data_size = sizeof(pauseFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_PAUSE_FRAMES, 
										stat_parameter, 
										 
										&pauseFrameRawData,
										&timestamp
										);
#endif
	}

	
	{/* bridge frame */
		memset(&bridgeFrameRawData, 0, sizeof(PON_bridge_frames_raw_data_t));
#if 1
        stat_item.collector_id = llid;
        stat_item.raw_statistics_type = PON_RAW_STAT_BRIDGE_FRAMES;
        stat_item.statistics_parameter = 0;
        stat_item.statistics_data = &bridgeFrameRawData;
        stat_item.statistics_data_size = sizeof(bridgeFrameRawData);
        iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
		collector_id = llid;
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_BRIDGE_FRAMES, 
										stat_parameter, 
										 
										&bridgeFrameRawData,
										&timestamp
										);
#endif
	}

	
    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
    {
    	{/*60 cpu per prio Frame Receive*/
    		memset(&revFrameToCpuPerPrioRawData, 0, sizeof(PON_received_frames_to_cpu_per_priority_raw_data_t));
#if 1
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_RECEIVED_FRAMES_TO_CPU_PER_PRIORITY;
            stat_item.statistics_parameter = ONU_PHYSICAL_PORT_SYSTEM;
            stat_item.statistics_data = &revFrameToCpuPerPrioRawData;
            stat_item.statistics_data_size = sizeof(revFrameToCpuPerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = llid;
    		stat_parameter = 2;/*1-pon,2-system*/
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_RECEIVED_FRAMES_TO_CPU_PER_PRIORITY, 
    										stat_parameter, 
    										 
    										&revFrameToCpuPerPrioRawData,
    										&timestamp
    										);
#endif
    	}
    	
    	{/*Transmitted Frames Per Priority (7)*/
    		memset(&cosTxOkFramePerPrioRawData, 0, sizeof(PON_transmitted_frames_per_priority_raw_data_t));
#if 1
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY;
            stat_item.statistics_parameter = ONU_PHYSICAL_PORT_SYSTEM;
            stat_item.statistics_data = &cosTxOkFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosTxOkFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = llid;
    		stat_parameter = 2;/*1-pon,2-system*/
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY, 
    										stat_parameter, 
    										 
    										&cosTxOkFramePerPrioRawData,
    										&timestamp
    										);
#endif
    	}
    	
    	{/*dropped frame per priority*/
    		memset(&cosTxDropFramePerPrioRawData, 0, sizeof(PON_dropped_frames_raw_data_t));
#if 1
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_DROPPED_FRAMES;
            stat_item.statistics_parameter = ONU_PHYSICAL_PORT_SYSTEM;
            stat_item.statistics_data = &cosTxDropFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosTxDropFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = llid;
    		stat_parameter = 2;/*1-pon,2-system*/
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_DROPPED_FRAMES, 
    										stat_parameter, 
    										 
    										&tranToSystemDropFrameRawData,
    										&timestamp
    										);
#endif
    	}

    	{/*CPU Ports Frames (63)*/
#if 1
    		memset(&totalCpuRxFrameDropRawData, 0, sizeof(PON_total_dropped_cpu_rx_frames_raw_data_t));
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_CPU_PORTS_FRAMES;
            stat_item.statistics_parameter = ONU_PHYSICAL_PORT_SYSTEM;
            stat_item.statistics_data = &totalCpuFrameRawData;
            stat_item.statistics_data_size = sizeof(totalCpuFrameRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = llid;
    		stat_parameter = 0;
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_TOTAL_DROPPED_CPU_RX_FRAMES,
    										stat_parameter, 
    										 
    										&totalCpuRxFrameDropRawData,
    										&timestamp
    										);
#endif
    	}
    	
    	{/*total drop Rx from CPU*/
    		memset(&totalCpuRxFrameDropRawData, 0, sizeof(PON_total_dropped_cpu_rx_frames_raw_data_t));
#if 1
            stat_item.collector_id = llid;
            stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_CPU_RX_FRAMES;
            stat_item.statistics_parameter = 0;
            stat_item.statistics_data = &totalCpuRxFrameDropRawData;
            stat_item.statistics_data_size = sizeof(totalCpuRxFrameDropRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
#else
    		collector_id = llid;
    		stat_parameter = 0;
    		iRes = PAS_get_raw_statistics( ponId, 
    										collector_id, 
    										PON_RAW_STAT_TOTAL_DROPPED_CPU_RX_FRAMES,
    										stat_parameter, 
    										 
    										&totalCpuRxFrameDropRawData,
    										&timestamp
    										);
#endif
    	}
    }


	/*Rx */
	vty_out( vty, "\r\n");
	vty_out( vty, "  --------------------------------------Total Sum-----------------------------------\r\n");
	vty_out( vty, "  RxTotalFrameOk         : %-20lu",totalFrameRawData.received_ok );
	vty_out( vty, "  TxTotalFrameOk         : %-20lu\r\n",totalFrameRawData.transmitted_ok + pauseFrameRawData.system_transmitted_ok );

	len = sprintf(buf, "  RxOctetsOk             : " );	
	sprintf64Bits( buf+len, (unsigned long long)totalOctetsRawData.received_ok );
	vty_out( vty, "%s",buf);

    if ( 1 /* PON_PASSAVE_ONU_6201_VERSION == OnuPonChipType */ )
    {
    	len = sprintf(buf, "  TxOctetsOk             : " );		
    	sprintf64Bits( buf+len, (unsigned long long )totalOctetsRawData.transmitted_ok );
    	vty_out( vty, "%s",buf);
    	vty_out( vty, "\r\n" );
    }
    else
    {
    	vty_out( vty, "  TxOctetsOk             : --\r\n" );
    }
	
	vty_out( vty, "\r\n  TxFrameDrop            : %-20lu\r\n",totalTxDropFrameRawData.total_system_dropped);

    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
    {
    	vty_out( vty, "  TxFrameOKPerPrio0      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[0]);
    	vty_out( vty, "  TxDropPerPrio0         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[0]);

    	vty_out( vty, "  TxFrameOKPerPrio1      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[1]);
    	vty_out( vty, "  TxDropPerPrio1         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[1]);

    	vty_out( vty, "  TxFrameOKPerPrio2      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[2]);
    	vty_out( vty, "  TxDropPerPrio2         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[2]);

    	vty_out( vty, "  TxFrameOKPerPrio3      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[3]);
    	vty_out( vty, "  TxDropPerPrio3         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[3]);

    	vty_out( vty, "  TxFrameOKPerPrio4      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[4]);
    	vty_out( vty, "  TxDropPerPrio4         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[4]);

    	vty_out( vty, "  TxFrameOKPerPrio5      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[5]);
    	vty_out( vty, "  TxDropPerPrio5         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[5]);

    	vty_out( vty, "  TxFrameOKPerPrio6      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[6]);
    	vty_out( vty, "  TxDropPerPrio6         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[6]);

    	vty_out( vty, "  TxFrameOKPerPrio7      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[7]);
    	vty_out( vty, "  TxDropPerPrio7         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[7]);
    }

	vty_out( vty, "  --------------------------------------Rx Error------------------------------------\r\n");
	vty_out( vty, "  RxFrameTotal           : %-20lu",totalFrameRawData.received_ok + totalFrameRawData.total_bad );
	vty_out( vty, "  RxFrameErr             : %-20lu\r\n",totalFrameRawData.total_bad );

    if ( PON_PASSAVE_ONU_6201_VERSION == OnuPonChipType )
    {
    	len = sprintf(buf, "  RxOctetsTotal          : " );	
    	sprintf64Bits( buf+len, (unsigned long long)totalOctetsRawData.received_total );
    	vty_out( vty, "%s",buf);
    }   
    else
    {
    	len = sprintf(buf, "  RxOctetsTotal          : " );	
    	sprintf64Bits( buf+len, (unsigned long long)(totalOctetsRawData.received_ok + totalOctetsRawData.received_error) );
    	vty_out( vty, "%s",buf);
    }
	len = sprintf(buf, "  RxOctetsErr            : " );	
	sprintf64Bits( buf+len, (unsigned long long)totalOctetsRawData.received_error );
	vty_out( vty, "%s\r\n",buf);

	vty_out( vty, "  RxFrameTooLongErr      : %-20lu",frameTooLongErrRawData.system_received);
	vty_out( vty, "  RxFrameFCSErr          : %-20lu\r\n",totalFrameRawData.received_error);


	vty_out( vty, "  --------------------------------------DataPath------------------------------------\r\n");
	vty_out( vty, "  RxPauseFrameOk         : %-20lu", pauseFrameRawData.system_received_ok);
	vty_out( vty, "  TxPauseFrameOk         : %-20lu\r\n", pauseFrameRawData.system_transmitted_ok);
	vty_out( vty, "  RxBridgeFrame          : %-20lu", bridgeFrameRawData.received);
	vty_out( vty, "  --\r\n");

    if ( OLT_PONCHIP_ISPAS(OltPonChipType) )
    {
    	vty_out( vty, "  --------------------------------------ManagePath----------------------------------\r\n");
#if 1
    	vty_out( vty, "  RxHostFrameOk          : %-20lu", totalCpuFrameRawData.from_cpu);
    	vty_out( vty, "  TxHostFrameOk          : %-20lu\r\n", totalCpuFrameRawData.to_cpu);
    	vty_out( vty, "  RxHostFrameDrop        : %-20lu",totalCpuRxFrameDropRawData.total_system_dropped);
    	vty_out( vty, "  --\r\n");
#else
    	vty_out( vty, "  RxHostFrameOk          : %-20lu",hostframeRawData.system_received_ok);
    	vty_out( vty, "  TxHostFrameOk          : %-20lu\r\n",hostframeRawData.system_transmitted_ok);
    	vty_out( vty, "  RxHostFrameTotal       : %-20lu",hostframeRawData.total_host_destined_frames);
       	vty_out( vty, "  RxHostFrameDrop        : %-20lu\r\n",hostframeRawData.system_dropped);
#endif

    	vty_out( vty, "  --                     : --                  ");
    	vty_out( vty, "  TxHostFramePerPrio0    : %-20lu\r\n",revFrameToCpuPerPrioRawData.received_ok[0]);

    	vty_out( vty, "  --                     : --                  ");
    	vty_out( vty, "  TxHostFramePerPrio1    : %-20lu\r\n",revFrameToCpuPerPrioRawData.received_ok[1]);
    	
    	vty_out( vty, "  --                     : --                  ");
    	vty_out( vty, "  TxHostFramePerPrio2    : %-20lu\r\n",revFrameToCpuPerPrioRawData.received_ok[2]);
    	
    	vty_out( vty, "  --                     : --                  ");
    	vty_out( vty, "  TxHostFramePerPrio3    : %-20lu\r\n",revFrameToCpuPerPrioRawData.received_ok[3]);
    	
    	vty_out( vty, "  --                     : --                  ");
    	vty_out( vty, "  TxHostFramePerPrio4    : %-20lu\r\n",revFrameToCpuPerPrioRawData.received_ok[4]);
    	
    	vty_out( vty, "  --                     : --                  ");
    	vty_out( vty, "  TxHostFramePerPrio5    : %-20lu\r\n",revFrameToCpuPerPrioRawData.received_ok[5]);
    	
    	vty_out( vty, "  --                     : --                  ");
    	vty_out( vty, "  TxHostFramePerPrio6    : %-20lu\r\n",revFrameToCpuPerPrioRawData.received_ok[6]);
    	
    	vty_out( vty, "  --                     : --                  ");
    	vty_out( vty, "  TxHostFramePerPrio7    : %-20lu\r\n",revFrameToCpuPerPrioRawData.received_ok[7]);	
    }

       
	vty_out( vty, "\r\n");
	
	return 0;
}

STATUS CliRealTimeStatsPonDatapath( short int ponId, struct vty* vty )
{
	int iRes = 0;
    short int PonChipType;
	   
	PON_transmitted_frames_per_priority_raw_data_t cosTxOkFramePerPrioRawData;
	PON_dropped_frames_raw_data_t	        cosTxDropFramePerPrioRawData;
	PON_received_frames_per_priority_raw_data_t cosReceivedOkFramePerPrioRawData;   
	
 
    OLT_raw_stat_item_t    stat_item;

	PonChipType = V2R1_GetPonchipType( ponId );

	vty_out( vty, "\r\n");
   
	if ( PONCHIP_PAS5001 != PonChipType )
    {       
        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
        	memset(&cosTxOkFramePerPrioRawData, 0, sizeof(cosTxOkFramePerPrioRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY;
            stat_item.statistics_parameter = OLT_PHYSICAL_PORT_PON;
            stat_item.statistics_data = &cosTxOkFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosTxOkFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);

			memset(&cosReceivedOkFramePerPrioRawData, 0, sizeof(cosReceivedOkFramePerPrioRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_RECEIVED_FRAMES_PER_PRIORITY;
            stat_item.statistics_parameter = OLT_PHYSICAL_PORT_PON;
            stat_item.statistics_data = &cosReceivedOkFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosReceivedOkFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
			
			memset(&cosTxDropFramePerPrioRawData, 0, sizeof(cosTxDropFramePerPrioRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_DROPPED_FRAMES;
            stat_item.statistics_parameter = OLT_PHYSICAL_PORT_PON;
            stat_item.statistics_data = &cosTxDropFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosTxDropFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
        }
    }   
 
	vty_out( vty, "\r\n");

	vty_out( vty, "  ------------------------------------Pon Datapath Statistics---------------------------------\r\n");
	
	if ( PONCHIP_PAS5001 != PonChipType )
    {
        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
        	vty_out( vty, "  TxFrameOKTotalDataPath : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[0]+cosTxOkFramePerPrioRawData.transmitted_ok[1] + cosTxOkFramePerPrioRawData.transmitted_ok[2]\
				                                              + cosTxOkFramePerPrioRawData.transmitted_ok[3]+ cosTxOkFramePerPrioRawData.transmitted_ok[4] + cosTxOkFramePerPrioRawData.transmitted_ok[5]\
				                                               + cosTxOkFramePerPrioRawData.transmitted_ok[6] + cosTxOkFramePerPrioRawData.transmitted_ok[7]);
        	vty_out( vty, "  RxFrameOKTotalDataPath : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[0] + cosReceivedOkFramePerPrioRawData.received_ok[1] + cosReceivedOkFramePerPrioRawData.received_ok[2]\
				                                                   + cosReceivedOkFramePerPrioRawData.received_ok[3] + cosReceivedOkFramePerPrioRawData.received_ok[4] + cosReceivedOkFramePerPrioRawData.received_ok[5]\
				                                                   + cosReceivedOkFramePerPrioRawData.received_ok[6] + cosReceivedOkFramePerPrioRawData.received_ok[7]);
			vty_out( vty, "  TxFrameOKPerPrio0      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[0]);
        	vty_out( vty, "  RxFrameOKPerPrio0      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[0]);
        	vty_out( vty, "  TxFrameOKPerPrio1      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[1]);
        	vty_out( vty, "  RxFrameOKPerPrio1      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[1]);
        	vty_out( vty, "  TxFrameOKPerPrio2      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[2]);
        	vty_out( vty, "  RxFrameOKPerPrio2      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[2]);
        	vty_out( vty, "  TxFrameOKPerPrio3      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[3]);
        	vty_out( vty, "  RxFrameOKPerPrio3      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[3]);
        	vty_out( vty, "  TxFrameOKPerPrio4      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[4]);
        	vty_out( vty, "  RxFrameOKPerPrio4      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[4]);
        	vty_out( vty, "  TxFrameOKPerPrio5      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[5]);
        	vty_out( vty, "  RxFrameOKPerPrio5      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[5]);
        	vty_out( vty, "  TxFrameOKPerPrio6      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[6]);
        	vty_out( vty, "  RxFrameOKPerPrio6      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[6]);
        	vty_out( vty, "  TxFrameOKPerPrio7      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[7]);
        	vty_out( vty, "  RxFrameOKPerPrio7      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[7]);

			vty_out( vty, "\r\n");

			vty_out( vty, "  TxDropPerPrio0         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[0]);
			vty_out( vty, "  TxDropPerPrio1         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[1]);
			vty_out( vty, "  TxDropPerPrio2         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[2]);
			vty_out( vty, "  TxDropPerPrio3         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[3]);					
			vty_out( vty, "  TxDropPerPrio4         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[4]);
			vty_out( vty, "  TxDropPerPrio5         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[5]);
			vty_out( vty, "  TxDropPerPrio6         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[6]);
			vty_out( vty, "  TxDropPerPrio7         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[7]);

			vty_out( vty, "\r\n");
		}
    }
	
	return 0;
}

STATUS CliRealTimeStatsPonDatapathForGpon( short int ponId, struct vty* vty )
{
	int iRes = 0;
    short int PonChipType;
	int onuId = 0;
	int llid = 0;
	int gemId = 0;
	tOgCmPmPonGemCounter gemStats;
	tOgCmPmPonGemCounter allGemStats;
 
    OLT_raw_stat_item_t    stat_item;

	memset(&stat_item, 0, sizeof(OLT_raw_stat_item_t));
	memset( &gemStats, 0, sizeof( tOgCmPmPonGemCounter ) );
	memset( &allGemStats, 0, sizeof( tOgCmPmPonGemCounter ) );
	
	PonChipType = V2R1_GetPonchipType( ponId );

	vty_out( vty, "\r\n");
   
	for( onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
	{
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;
		
		gemId = ulGPON_GEMPORT_BASE+(llid-1)*16;
		
		memset( &gemStats, 0, sizeof( tOgCmPmPonGemCounter ) );
		stat_item.collector_id = GPON_PM_PONGEM_ACTIVE_COUNTER;
	    stat_item.statistics_data = &gemStats;
		stat_item.statistics_parameter = gemId;
	    stat_item.statistics_data_size = sizeof(tOgCmPmPonGemCounter);
	    iRes = OLT_GetRawStatistics(ponId, &stat_item);
		if(VOS_OK != iRes)
		{
			continue;
		}
		allGemStats.rx_bytes += gemStats.rx_bytes;
		allGemStats.rx_packets += gemStats.rx_packets;
		allGemStats.tx_bytes += gemStats.tx_bytes;
		allGemStats.tx_packets += gemStats.tx_packets;
	}
       
 
	vty_out( vty, "\r\n");

	vty_out( vty, "  ------------------------------------Pon Datapath Statistics---------------------------------\r\n");
	
	vty_out( vty, "\r\n  rx_packets         : %-20llu", allGemStats.rx_packets);
	vty_out( vty, "  tx_packets             : %-20llu\r\n", allGemStats.tx_packets); 
	vty_out( vty, "\r\n  rx_bytes           : %-20llu", allGemStats.rx_bytes);
	vty_out( vty, "  tx_bytes               : %-20llu\r\n", allGemStats.tx_bytes); 

    
	
	return 0;
}


STATUS CliRealTimeStatsCniDatapath( short int ponId, struct vty* vty )
{
	int iRes = 0;
    short int PonChipType;
	   
	PON_transmitted_frames_per_priority_raw_data_t cosTxOkFramePerPrioRawData;
	PON_dropped_frames_raw_data_t	        cosTxDropFramePerPrioRawData;
	PON_received_frames_per_priority_raw_data_t cosReceivedOkFramePerPrioRawData;   
	
 
    OLT_raw_stat_item_t    stat_item;

	PonChipType = V2R1_GetPonchipType( ponId );

	vty_out( vty, "\r\n");
   
	if ( PONCHIP_PAS5001 != PonChipType )
    {       
        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
        	memset(&cosTxOkFramePerPrioRawData, 0, sizeof(cosTxOkFramePerPrioRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY;
            stat_item.statistics_parameter = OLT_PHYSICAL_PORT_SYSTEM;
            stat_item.statistics_data = &cosTxOkFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosTxOkFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);

			memset(&cosReceivedOkFramePerPrioRawData, 0, sizeof(cosReceivedOkFramePerPrioRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_RECEIVED_FRAMES_PER_PRIORITY;
            stat_item.statistics_parameter = OLT_PHYSICAL_PORT_SYSTEM;
            stat_item.statistics_data = &cosReceivedOkFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosReceivedOkFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
			
			memset(&cosTxDropFramePerPrioRawData, 0, sizeof(cosTxDropFramePerPrioRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_DROPPED_FRAMES;
            stat_item.statistics_parameter = OLT_PHYSICAL_PORT_SYSTEM;
            stat_item.statistics_data = &cosTxDropFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosTxDropFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
        }
    }   
 
	vty_out( vty, "\r\n");

	vty_out( vty, "  ------------------------------------Cni Datapath Statistics---------------------------------\r\n");
	
	if ( PONCHIP_PAS5001 != PonChipType )
    {
        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
        	vty_out( vty, "  TxFrameOKTotalDataPath : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[0]+cosTxOkFramePerPrioRawData.transmitted_ok[1] + cosTxOkFramePerPrioRawData.transmitted_ok[2]\
				                                              + cosTxOkFramePerPrioRawData.transmitted_ok[3]+ cosTxOkFramePerPrioRawData.transmitted_ok[4] + cosTxOkFramePerPrioRawData.transmitted_ok[5]\
				                                               + cosTxOkFramePerPrioRawData.transmitted_ok[6] + cosTxOkFramePerPrioRawData.transmitted_ok[7]);
        	vty_out( vty, "  RxFrameOKTotalDataPath : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[0] + cosReceivedOkFramePerPrioRawData.received_ok[1] + cosReceivedOkFramePerPrioRawData.received_ok[2]\
				                                                   + cosReceivedOkFramePerPrioRawData.received_ok[3] + cosReceivedOkFramePerPrioRawData.received_ok[4] + cosReceivedOkFramePerPrioRawData.received_ok[5]\
				                                                   + cosReceivedOkFramePerPrioRawData.received_ok[6] + cosReceivedOkFramePerPrioRawData.received_ok[7]);
			vty_out( vty, "  TxFrameOKPerPrio0      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[0]);
        	vty_out( vty, "  RxFrameOKPerPrio0      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[0]);
        	vty_out( vty, "  TxFrameOKPerPrio1      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[1]);
        	vty_out( vty, "  RxFrameOKPerPrio1      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[1]);
        	vty_out( vty, "  TxFrameOKPerPrio2      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[2]);
        	vty_out( vty, "  RxFrameOKPerPrio2      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[2]);
        	vty_out( vty, "  TxFrameOKPerPrio3      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[3]);
        	vty_out( vty, "  RxFrameOKPerPrio3      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[3]);
        	vty_out( vty, "  TxFrameOKPerPrio4      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[4]);
        	vty_out( vty, "  RxFrameOKPerPrio4      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[4]);
        	vty_out( vty, "  TxFrameOKPerPrio5      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[5]);
        	vty_out( vty, "  RxFrameOKPerPrio5      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[5]);
        	vty_out( vty, "  TxFrameOKPerPrio6      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[6]);
        	vty_out( vty, "  RxFrameOKPerPrio6      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[6]);
        	vty_out( vty, "  TxFrameOKPerPrio7      : %-20lu",cosTxOkFramePerPrioRawData.transmitted_ok[7]);
        	vty_out( vty, "  RxFrameOKPerPrio7      : %-20lu\r\n",cosReceivedOkFramePerPrioRawData.received_ok[7]);

			vty_out( vty, "\r\n");

			vty_out( vty, "  TxDropPerPrio0         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[0]);
			vty_out( vty, "  TxDropPerPrio1         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[1]);
			vty_out( vty, "  TxDropPerPrio2         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[2]);
			vty_out( vty, "  TxDropPerPrio3         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[3]);					
			vty_out( vty, "  TxDropPerPrio4         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[4]);
			vty_out( vty, "  TxDropPerPrio5         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[5]);
			vty_out( vty, "  TxDropPerPrio6         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[6]);
			vty_out( vty, "  TxDropPerPrio7         : %-20lu\r\n",cosTxDropFramePerPrioRawData.dropped[7]);

			vty_out( vty, "\r\n");
		}
    }
	
	return 0;
}

STATUS CliRealTimeStatsUplink( short int ponId, struct vty* vty )
{
	int iRes = 0;
    short int PonChipType;
	   
	PON_transmitted_frames_per_priority_raw_data_t cosCniTxOkFramePerPrioRawData;
	PON_received_frames_per_priority_raw_data_t cosPonReceivedOkFramePerPrioRawData;   
	
 
    OLT_raw_stat_item_t    stat_item;

	PonChipType = V2R1_GetPonchipType( ponId );

	vty_out( vty, "\r\n");
   
	if ( PONCHIP_PAS5001 != PonChipType )
    {       
        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
        	memset(&cosCniTxOkFramePerPrioRawData, 0, sizeof(cosCniTxOkFramePerPrioRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY;
            stat_item.statistics_parameter = OLT_PHYSICAL_PORT_SYSTEM;
            stat_item.statistics_data = &cosCniTxOkFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosCniTxOkFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);

			memset(&cosPonReceivedOkFramePerPrioRawData, 0, sizeof(cosPonReceivedOkFramePerPrioRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_RECEIVED_FRAMES_PER_PRIORITY;
            stat_item.statistics_parameter = OLT_PHYSICAL_PORT_PON;
            stat_item.statistics_data = &cosPonReceivedOkFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosPonReceivedOkFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
        }
    }   
 
	vty_out( vty, "\r\n");

	vty_out( vty, "  ------------------------------------Uplink Datapath Statistics---------------------------------\r\n");
	
	if ( PONCHIP_PAS5001 != PonChipType )
    {
        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
        	vty_out( vty, "  PonRxFrameOKTotalDataPath : %-20lu",cosPonReceivedOkFramePerPrioRawData.received_ok[0] + cosPonReceivedOkFramePerPrioRawData.received_ok[1] + cosPonReceivedOkFramePerPrioRawData.received_ok[2]\
				                                                   + cosPonReceivedOkFramePerPrioRawData.received_ok[3] + cosPonReceivedOkFramePerPrioRawData.received_ok[4] + cosPonReceivedOkFramePerPrioRawData.received_ok[5]\
				                                                   + cosPonReceivedOkFramePerPrioRawData.received_ok[6] + cosPonReceivedOkFramePerPrioRawData.received_ok[7]);


			vty_out( vty, "  CniTxFrameOKTotalDataPath : %-20lu\r\n",cosCniTxOkFramePerPrioRawData.transmitted_ok[0]+cosCniTxOkFramePerPrioRawData.transmitted_ok[1] + cosCniTxOkFramePerPrioRawData.transmitted_ok[2]\
				                                              + cosCniTxOkFramePerPrioRawData.transmitted_ok[3]+ cosCniTxOkFramePerPrioRawData.transmitted_ok[4] + cosCniTxOkFramePerPrioRawData.transmitted_ok[5]\
				                                               + cosCniTxOkFramePerPrioRawData.transmitted_ok[6] + cosCniTxOkFramePerPrioRawData.transmitted_ok[7]);
        	vty_out( vty, "  PonRxFrameOKPerPrio0      : %-20lu",cosPonReceivedOkFramePerPrioRawData.received_ok[0]);
			vty_out( vty, "  CniTxFrameOKPerPrio0      : %-20lu\r\n",cosCniTxOkFramePerPrioRawData.transmitted_ok[0]);
        	vty_out( vty, "  PonRxFrameOKPerPrio1      : %-20lu",cosPonReceivedOkFramePerPrioRawData.received_ok[1]);
        	vty_out( vty, "  CniTxFrameOKPerPrio1      : %-20lu\r\n",cosCniTxOkFramePerPrioRawData.transmitted_ok[1]);
        	vty_out( vty, "  PonRxFrameOKPerPrio2      : %-20lu",cosPonReceivedOkFramePerPrioRawData.received_ok[2]);
        	vty_out( vty, "  CniTxFrameOKPerPrio2      : %-20lu\r\n",cosCniTxOkFramePerPrioRawData.transmitted_ok[2]);
        	vty_out( vty, "  PonRxFrameOKPerPrio3      : %-20lu",cosPonReceivedOkFramePerPrioRawData.received_ok[3]);
        	vty_out( vty, "  CniTxFrameOKPerPrio3      : %-20lu\r\n",cosCniTxOkFramePerPrioRawData.transmitted_ok[3]);
        	vty_out( vty, "  PonRxFrameOKPerPrio4      : %-20lu",cosPonReceivedOkFramePerPrioRawData.received_ok[4]);
        	vty_out( vty, "  CniTxFrameOKPerPrio4      : %-20lu\r\n",cosCniTxOkFramePerPrioRawData.transmitted_ok[4]);
        	vty_out( vty, "  PonRxFrameOKPerPrio5      : %-20lu",cosPonReceivedOkFramePerPrioRawData.received_ok[5]);
        	vty_out( vty, "  CniTxFrameOKPerPrio5      : %-20lu\r\n",cosCniTxOkFramePerPrioRawData.transmitted_ok[5]);
        	vty_out( vty, "  PonRxFrameOKPerPrio6      : %-20lu",cosPonReceivedOkFramePerPrioRawData.received_ok[6]);
        	vty_out( vty, "  CniTxFrameOKPerPrio6      : %-20lu\r\n",cosCniTxOkFramePerPrioRawData.transmitted_ok[6]);
        	vty_out( vty, "  PonRxFrameOKPerPrio7      : %-20lu",cosPonReceivedOkFramePerPrioRawData.received_ok[7]);
        	vty_out( vty, "  CniTxFrameOKPerPrio7      : %-20lu\r\n",cosCniTxOkFramePerPrioRawData.transmitted_ok[7]);
        	

			vty_out( vty, "\r\n");
		}
    }
	
	return 0;
}

STATUS CliRealTimeStatsDownlink( short int ponId, struct vty* vty )
{
	int iRes = 0;
    short int PonChipType;
	   
	PON_transmitted_frames_per_priority_raw_data_t cosPonTxOkFramePerPrioRawData;
	PON_received_frames_per_priority_raw_data_t cosCniReceivedOkFramePerPrioRawData;   
	
 
    OLT_raw_stat_item_t    stat_item;

	PonChipType = V2R1_GetPonchipType( ponId );

	vty_out( vty, "\r\n");
   
	if ( PONCHIP_PAS5001 != PonChipType )
    {       
        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
        	memset(&cosPonTxOkFramePerPrioRawData, 0, sizeof(cosPonTxOkFramePerPrioRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY;
            stat_item.statistics_parameter = OLT_PHYSICAL_PORT_PON;
            stat_item.statistics_data = &cosPonTxOkFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosPonTxOkFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);

			memset(&cosCniReceivedOkFramePerPrioRawData, 0, sizeof(cosCniReceivedOkFramePerPrioRawData));
            stat_item.collector_id = PON_OLT_ID;
            stat_item.raw_statistics_type = PON_RAW_STAT_RECEIVED_FRAMES_PER_PRIORITY;
            stat_item.statistics_parameter = OLT_PHYSICAL_PORT_SYSTEM;
            stat_item.statistics_data = &cosCniReceivedOkFramePerPrioRawData;
            stat_item.statistics_data_size = sizeof(cosCniReceivedOkFramePerPrioRawData);
            iRes = OLT_GetRawStatistics(ponId, &stat_item);
			
        }
    }   
 
	vty_out( vty, "\r\n");

	vty_out( vty, "  ------------------------------------Downlink Datapath Statistics---------------------------------\r\n");
	
	if ( PONCHIP_PAS5001 != PonChipType )
    {
        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
        	vty_out( vty, "  CniRxFrameOKTotalDataPath : %-20lu",cosCniReceivedOkFramePerPrioRawData.received_ok[0] + cosCniReceivedOkFramePerPrioRawData.received_ok[1] + cosCniReceivedOkFramePerPrioRawData.received_ok[2]\
				                                                   + cosCniReceivedOkFramePerPrioRawData.received_ok[3] + cosCniReceivedOkFramePerPrioRawData.received_ok[4] + cosCniReceivedOkFramePerPrioRawData.received_ok[5]\
				                                                   + cosCniReceivedOkFramePerPrioRawData.received_ok[6] + cosCniReceivedOkFramePerPrioRawData.received_ok[7]);
        	vty_out( vty, "  PonTxFrameOKTotalDataPath : %-20lu\r\n",cosPonTxOkFramePerPrioRawData.transmitted_ok[0]+cosPonTxOkFramePerPrioRawData.transmitted_ok[1] + cosPonTxOkFramePerPrioRawData.transmitted_ok[2]\
				                                              + cosPonTxOkFramePerPrioRawData.transmitted_ok[3]+ cosPonTxOkFramePerPrioRawData.transmitted_ok[4] + cosPonTxOkFramePerPrioRawData.transmitted_ok[5]\
				                                               + cosPonTxOkFramePerPrioRawData.transmitted_ok[6] + cosPonTxOkFramePerPrioRawData.transmitted_ok[7]);
        	vty_out( vty, "  CniRxFrameOKPerPrio0      : %-20lu",cosCniReceivedOkFramePerPrioRawData.received_ok[0]);
			vty_out( vty, "  PonTxFrameOKPerPrio0      : %-20lu\r\n",cosPonTxOkFramePerPrioRawData.transmitted_ok[0]);
        	vty_out( vty, "  CniRxFrameOKPerPrio1      : %-20lu",cosCniReceivedOkFramePerPrioRawData.received_ok[1]);
        	vty_out( vty, "  PonTxFrameOKPerPrio1      : %-20lu\r\n",cosPonTxOkFramePerPrioRawData.transmitted_ok[1]);
        	vty_out( vty, "  CniRxFrameOKPerPrio2      : %-20lu",cosCniReceivedOkFramePerPrioRawData.received_ok[2]);
        	vty_out( vty, "  PonTxFrameOKPerPrio2      : %-20lu\r\n",cosPonTxOkFramePerPrioRawData.transmitted_ok[2]);
        	vty_out( vty, "  CniRxFrameOKPerPrio3      : %-20lu",cosCniReceivedOkFramePerPrioRawData.received_ok[3]);
        	vty_out( vty, "  PonTxFrameOKPerPrio3      : %-20lu\r\n",cosPonTxOkFramePerPrioRawData.transmitted_ok[3]);
        	vty_out( vty, "  CniRxFrameOKPerPrio4      : %-20lu",cosCniReceivedOkFramePerPrioRawData.received_ok[4]);
        	vty_out( vty, "  PonTxFrameOKPerPrio4      : %-20lu\r\n",cosPonTxOkFramePerPrioRawData.transmitted_ok[4]);
        	vty_out( vty, "  CniRxFrameOKPerPrio5      : %-20lu",cosCniReceivedOkFramePerPrioRawData.received_ok[5]);
        	vty_out( vty, "  PonTxFrameOKPerPrio5      : %-20lu\r\n",cosPonTxOkFramePerPrioRawData.transmitted_ok[5]);
        	vty_out( vty, "  CniRxFrameOKPerPrio6      : %-20lu",cosCniReceivedOkFramePerPrioRawData.received_ok[6]);
        	vty_out( vty, "  PonTxFrameOKPerPrio6      : %-20lu\r\n",cosPonTxOkFramePerPrioRawData.transmitted_ok[6]);
        	vty_out( vty, "  CniRxFrameOKPerPrio7      : %-20lu",cosCniReceivedOkFramePerPrioRawData.received_ok[7]);
        	vty_out( vty, "  PonTxFrameOKPerPrio7      : %-20lu\r\n",cosPonTxOkFramePerPrioRawData.transmitted_ok[7]);
        	
			vty_out( vty, "\r\n");
		}
    }
	
	return 0;
}

STATUS CliRealTimeStatsUplinkForGpon( short int ponId, struct vty* vty )
{
	int iRes = 0;
    short int PonChipType;
	int onuId = 0;
	int llid = 0;
	int gemId = 0;
	OLT_raw_stat_item_t    stat_item;

	tOgCmPmPonNNICounter counter;
	tOgCmPmPonGemCounter gemStats;
	tOgCmPmPonGemCounter allGemStats;

	
    vty_out( vty, "\r\n");
	memset(&stat_item, 0, sizeof(OLT_raw_stat_item_t));
	memset( &counter, 0, sizeof( tOgCmPmPonNNICounter ) );
	memset( &gemStats, 0, sizeof( tOgCmPmPonGemCounter ) );
	memset( &allGemStats, 0, sizeof( tOgCmPmPonGemCounter ) );

	PonChipType = V2R1_GetPonchipType( ponId );

	vty_out( vty, "\r\n");
   

	for( onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
	{
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;
		gemId = ulGPON_GEMPORT_BASE+(llid-1)*16;
		memset( &gemStats, 0, sizeof( tOgCmPmPonGemCounter ) );
		stat_item.collector_id = GPON_PM_PONGEM_ACTIVE_COUNTER;
	    stat_item.statistics_data = &gemStats;
		stat_item.statistics_parameter = gemId;
	    stat_item.statistics_data_size = sizeof(tOgCmPmPonGemCounter);
	    iRes = OLT_GetRawStatistics(ponId, &stat_item);
		if(VOS_OK != iRes)
		{
			continue;
		}
		allGemStats.rx_bytes += gemStats.rx_bytes;
		allGemStats.rx_packets += gemStats.rx_packets;
		allGemStats.tx_bytes += gemStats.tx_bytes;
		allGemStats.tx_packets += gemStats.tx_packets;
	}

	memset( &gemStats, 0, sizeof( tOgCmPmPonNNICounter ) );
	stat_item.collector_id = GPON_PM_PONNNI_ACTIVE_COUNTER;
    stat_item.statistics_data = &counter;
    stat_item.statistics_data_size = sizeof(tOgCmPmPonNNICounter);
    iRes = OLT_GetRawStatistics(ponId, &stat_item);
	 
	vty_out( vty, "  ------------------------------------Uplink Datapath Statistics---------------------------------\r\n");
	
	vty_out( vty, "  pon_rx_packets              : %-20llu\r\n",allGemStats.rx_packets);
	vty_out( vty, "  pon_rx_bytes                : %-20llu\r\n",allGemStats.rx_bytes);
    vty_out( vty, "\r\n");
	vty_out( vty, "  cni_tx_frames_64            : %-20llu\r\n",counter.tx_frames_64);
	vty_out( vty, "  cni_tx_frames_65_127        : %-20llu\r\n",counter.tx_frames_65_127);
	vty_out( vty, "  cni_tx_frames_128_255       : %-20llu\r\n",counter.tx_frames_128_255);	
	vty_out( vty, "  cni_tx_frames_256_511       : %-20llu\r\n",counter.tx_frames_256_511);	
	vty_out( vty, "  cni_tx_frames_512_1023      : %-20llu\r\n",counter.tx_frames_512_1023);
	vty_out( vty, "  cni_tx_frames_1024_1518     : %-20llu\r\n",counter.tx_frames_1024_1518);	
	vty_out( vty, "  cni_tx_frames_1519_2047     : %-20llu\r\n",counter.tx_frames_1519_2047);	
	vty_out( vty, "  cni_tx_frames_2048_4095     : %-20llu\r\n",counter.tx_frames_2048_4095);	
	vty_out( vty, "  cni_tx_frames_4096_9216     : %-20llu\r\n",counter.tx_frames_4096_9216);
	vty_out( vty, "  cni_tx_frames_9217_16383    : %-20llu\r\n",counter.tx_frames_9217_16383);	
	vty_out( vty, "  cni_tx_frames               : %-20llu\r\n",counter.tx_frames);	
	vty_out( vty, "  cni_tx_bytes                : %-20llu\r\n",counter.tx_bytes);
	vty_out( vty, "  cni_tx_good_frames          : %-20llu\r\n",counter.tx_good_frames);
	vty_out( vty, "  cni_tx_unicast_frames       : %-20llu\r\n",counter.tx_unicast_frames);
	vty_out( vty, "  cni_tx_multicast_frames     : %-20llu\r\n",counter.tx_multicast_frames);
	vty_out( vty, "  cni_tx_broadcast_frames     : %-20llu\r\n",counter.tx_broadcast_frames);
	vty_out( vty, "  cni_tx_control_frames       : %-20llu\r\n",counter.tx_control_frames);
	vty_out( vty, "  cni_tx_pause_frames         : %-20llu\r\n",counter.tx_pause_frames);
	vty_out( vty, "  cni_tx_pfc_frames           : %-20llu\r\n",counter.tx_multicast_frames);
	vty_out( vty, "  cni_tx_vlan_frames          : %-20llu\r\n",counter.tx_vlan_frames);
	vty_out( vty, "  cni_tx_double_vlan_frames   : %-20llu\r\n",counter.tx_double_vlan_frames);
	vty_out( vty, "  cni_tx_fcs_errors           : %-20llu\r\n",counter.tx_fcs_errors);	
	vty_out( vty, "  cni_tx_oversize_frames      : %-20llu\r\n",counter.tx_oversize_frames);	
	vty_out( vty, "  cni_tx_fragmented_frames    : %-20llu\r\n",counter.tx_fragmented_frames);	
	vty_out( vty, "  cni_tx_runt_frames          : %-20llu\r\n",counter.tx_runt_frames);	
	vty_out( vty, "  cni_tx_jabber_frames        : %-20llu\r\n",counter.tx_jabber_frames);	
	vty_out( vty, "  cni_tx_error_frames         : %-20llu\r\n",counter.tx_error_frames);	
	vty_out( vty, "  cni_tx_underrun_frames      : %-20llu\r\n",counter.tx_underrun_frames);
	vty_out( vty, "\r\n");
	return 0;
}

STATUS CliRealTimeStatsDownlinkForGpon( short int ponId, struct vty* vty )
{
	int iRes = 0;
    short int PonChipType;
	int onuId = 0;
	int llid = 0;
	int gemId = 0;
	OLT_raw_stat_item_t    stat_item;

	tOgCmPmPonNNICounter counter;
	tOgCmPmPonGemCounter gemStats;
	tOgCmPmPonGemCounter allGemStats;

	
    vty_out( vty, "\r\n");
	memset(&stat_item, 0, sizeof(OLT_raw_stat_item_t));
	memset( &counter, 0, sizeof( tOgCmPmPonNNICounter ) );
	memset( &gemStats, 0, sizeof( tOgCmPmPonGemCounter ) );
	memset( &allGemStats, 0, sizeof( tOgCmPmPonGemCounter ) );

	PonChipType = V2R1_GetPonchipType( ponId );

	vty_out( vty, "\r\n");
   

	for( onuId = 0; onuId < STATS_HIS_MAXONU; onuId++)
	{
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;
		gemId = ulGPON_GEMPORT_BASE+(llid-1)*16;
		memset( &gemStats, 0, sizeof( tOgCmPmPonGemCounter ) );
		stat_item.collector_id = GPON_PM_PONGEM_ACTIVE_COUNTER;
	    stat_item.statistics_data = &gemStats;
		stat_item.statistics_parameter = gemId;
	    stat_item.statistics_data_size = sizeof(tOgCmPmPonGemCounter);
	    iRes = OLT_GetRawStatistics(ponId, &stat_item);
		if(VOS_OK != iRes)
		{
			continue;
		}
		allGemStats.rx_bytes += gemStats.rx_bytes;
		allGemStats.rx_packets += gemStats.rx_packets;
		allGemStats.tx_bytes += gemStats.tx_bytes;
		allGemStats.tx_packets += gemStats.tx_packets;
	}

	memset( &gemStats, 0, sizeof( tOgCmPmPonNNICounter ) );
	stat_item.collector_id = GPON_PM_PONNNI_ACTIVE_COUNTER;
    stat_item.statistics_data = &counter;
    stat_item.statistics_data_size = sizeof(tOgCmPmPonNNICounter);
    iRes = OLT_GetRawStatistics(ponId, &stat_item);
	 
	vty_out( vty, "  ------------------------------------Downlink Datapath Statistics---------------------------------\r\n");

	vty_out( vty, "  pon_tx_packets              : %-20llu\r\n",allGemStats.tx_packets);
	vty_out( vty, "  pon_tx_bytes                : %-20llu\r\n",allGemStats.tx_bytes);
    vty_out( vty, "\r\n");
	
	vty_out( vty, "  cni_rx_frames_64            : %-20llu\r\n",counter.rx_frames_64);
    vty_out( vty, "  cni_rx_frames_65_127        : %-20llu\r\n",counter.rx_frames_65_127);	
	vty_out( vty, "  cni_rx_frames_128_255       : %-20llu\r\n",counter.rx_frames_128_255);	
	vty_out( vty, "  cni_rx_frames_256_511       : %-20llu\r\n",counter.rx_frames_256_511);	
	vty_out( vty, "  cni_rx_frames_512_1023      : %-20llu\r\n",counter.rx_frames_512_1023);	
	vty_out( vty, "  cni_rx_frames_1024_1518     : %-20llu\r\n",counter.rx_frames_1024_1518);	
	vty_out( vty, "  cni_rx_frames_1519_2047     : %-20llu\r\n",counter.rx_frames_1519_2047);	
	vty_out( vty, "  cni_rx_frames_2048_4095     : %-20llu\r\n",counter.rx_frames_2048_4095);	
	vty_out( vty, "  cni_rx_frames_4096_9216     : %-20llu\r\n",counter.rx_frames_4096_9216);	
	vty_out( vty, "  cni_rx_frames_9217_16383    : %-20llu\r\n",counter.rx_frames_9217_16383);	
	vty_out( vty, "  cni_rx_frames               : %-20llu\r\n",counter.rx_frames);	
	vty_out( vty, "  cni_rx_bytes                : %-20llu\r\n",counter.rx_bytes);
	vty_out( vty, "  cni_rx_good_frames          : %-20llu\r\n",counter.rx_good_frames);
	vty_out( vty, "  cni_rx_unicast_frames       : %-20llu\r\n",counter.rx_unicast_frames);
	vty_out( vty, "  cni_rx_multicast_frames     : %-20llu\r\n",counter.rx_multicast_frames);
	vty_out( vty, "  cni_rx_broadcast_frames     : %-20llu\r\n",counter.rx_broadcast_frames);
	vty_out( vty, "  cni_rx_control_frames       : %-20llu\r\n",counter.rx_control_frames);	
	vty_out( vty, "  cni_rx_pause_frames         : %-20llu\r\n",counter.rx_pause_frames);	
	vty_out( vty, "  cni_rx_pfc_frames           : %-20llu\r\n",counter.tx_pfc_frames);	
	vty_out( vty, "  cni_rx_vlan_frames          : %-20llu\r\n",counter.rx_vlan_frames);
	vty_out( vty, "  cni_rx_double_vlan_frames   : %-20llu\r\n",counter.rx_double_vlan_frames);
	vty_out( vty, "  cni_rx_fcs_errors           : %-20llu\r\n",counter.rx_fcs_errors);	
	vty_out( vty, "  cni_rx_oversized_frames     : %-20llu\r\n",counter.rx_oversized_frames);	
	vty_out( vty, "  cni_rx_fragmented_frames    : %-20llu\r\n",counter.rx_fragmented_frames);	
	vty_out( vty, "  cni_rx_runt_frames          : %-20llu\r\n",counter.rx_runt_frames);	
	vty_out( vty, "  cni_rx_jabber_frames        : %-20llu\r\n",counter.rx_jabber_frames);	
	vty_out( vty, "  cni_rx_unsupported_opcode   : %-20llu\r\n",counter.rx_unsupported_opcode);	
	vty_out( vty, "  cni_rx_unsupported_da       : %-20llu\r\n",counter.rx_unsupported_da);	
	vty_out( vty, "  cni_rx_alignment_errors     : %-20llu\r\n",counter.rx_alignment_errors);
	vty_out( vty, "  cni_rx_length_out_of_range  : %-20llu\r\n",counter.rx_length_out_of_range);
	vty_out( vty, "  cni_rx_code_errors          : %-20llu\r\n",counter.rx_code_errors);
	vty_out( vty, "  cni_rx_mtu_check_errors     : %-20llu\r\n",counter.rx_mtu_check_errors);
	vty_out( vty, "  cni_rx_promiscuous_frames   : %-20llu\r\n",counter.rx_promiscuous_frames);
	vty_out( vty, "  cni_rx_truncated_frames     : %-20llu\r\n",counter.rx_truncated_frames);
	vty_out( vty, "  cni_rx_undersize_frames     : %-20llu\r\n",counter.rx_undersize_frames);
	vty_out( vty, "\r\n");
	
	return 0;
}


#if 0

#endif
/****************************************************
* HisStats24HoursStatusGet
* description : 查看pon 设备24 hour 历史统计使能状态
*
*
*
*****************************************************/
BOOL HisStatsPon24HoursStatusGet(unsigned short  ponId)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;	
	while( NULL != pstatsPon)
	{
		if (pstatsPon->ponIdx == ponId)
			return pstatsPon->iUsed24Hour;
		pstatsPon = pstatsPon->pNext;
	}
	return FALSE;
}


/****************************************************
* HisStatsPon15MinStatusGet
* description : 查看pon 设备15 Min 历史统计使能状态
*
*
*
*****************************************************/
BOOL HisStatsPon15MinStatusGet(unsigned short  ponId)
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;	
	while( NULL != pstatsPon)
	{
		if (pstatsPon->ponIdx == ponId)
			return pstatsPon->iUsed15Min;
		pstatsPon = pstatsPon->pNext;
	}
	return FALSE;
}

/****************************************************
* HisStatsOnu24HoursStatusGet
* description : 查看pon 设备24 hour 历史统计使能状态
*
*
*
*****************************************************/
BOOL HisStatsOnu24HoursStatusGet(unsigned short  ponId, unsigned short  onuId)
{
	statistic_PON_OID	*pstatsOnu = gpONUHisStatsHead ;	
	while( NULL != pstatsOnu)
	{
		if ((pstatsOnu->ponIdx == ponId) && (pstatsOnu->onuIdx == onuId))
			return pstatsOnu->iUsed24Hour;
		pstatsOnu = pstatsOnu->pNext;
	}
	return FALSE;
}
/****************************************************
* HisStatsOnu15MinStatusGet
* description : 查看pon 设备15 Min 历史统计使能状态
*
*
*
*****************************************************/
BOOL HisStatsOnu15MinStatusGet(unsigned short  ponId, unsigned short  onuId)
{
	statistic_PON_OID	*pstatsOnu = gpONUHisStatsHead ;		
	while( NULL != pstatsOnu)
	{
		if ((pstatsOnu->ponIdx == ponId) && (pstatsOnu->onuIdx == onuId))
			return pstatsOnu->iUsed15Min;
		pstatsOnu = pstatsOnu->pNext;
	}
	return FALSE;
}
#if 0
#endif




/********************************************
* StatsRealTimePonPro
* 描述: 设置olt 在线的控制表
*
*
*
'*
**********************************************/
int StatsRealTimePonPro (const short int ponId, unsigned char onStatus)
{
	/*rtStatsEntry *pTempPre=NULL;
	rtStatsEntry64 *ptempBuf = NULL;
	Statistics_debug_Table *ptempDebug = NULL;*/
	
	if (onStatus == STATS_DEVICE_OFF)
	{
		gStatsPonOn[ponId] = onStatus;
		
		/*if (NULL != pgStatsRealTimeOlt[ponId])
		{
			VOS_Free((VOID *)pgStatsRealTimeOlt[ponId]);
			pgStatsRealTimeOlt[ponId] = NULL;
			VOS_Free((VOID *) pgDebug_StatsRealTimeOlt[ponId]);
			pgDebug_StatsRealTimeOlt[ponId] = NULL;
		}

		if( tpgStatsRealTimeOlt[ponId] != NULL )
		{
			VOS_Free( tpgStatsRealTimeOlt[ponId] );
			tpgStatsRealTimeOlt[ponId]=NULL;			
		}*/
	}
	else if (onStatus == STATS_DEVICE_ON)
	{

		/*if (NULL == pgStatsRealTimeOlt[ponId])
		{
			
			ptempBuf = (rtStatsEntry64*)VOS_Malloc((ULONG)sizeof(rtStatsEntry64), (ULONG)MODULE_STATSTICS);
			if (NULL == ptempBuf)
				return STATS_ERR;

			pTempPre = (rtStatsEntry*)VOS_Malloc((ULONG)sizeof(rtStatsEntry), (ULONG)MODULE_STATSTICS);
			if( pTempPre == NULL )
			{
				VOS_Free( ptempBuf );
				return STATS_ERR;
			}
			
			memset(ptempBuf, 0, sizeof(rtStatsEntry64));
			memset( pTempPre, 0, sizeof(rtStatsEntry) );
			
			pgStatsRealTimeOlt[ponId]  = ptempBuf;
			tpgStatsRealTimeOlt[ponId] = pTempPre;
			
			ptempBuf = NULL;
			pTempPre = NULL;
			
			ptempDebug =(Statistics_debug_Table*)VOS_Malloc((ULONG)sizeof(Statistics_debug_Table),(ULONG)MODULE_STATSTICS);
			if (NULL == ptempDebug)
				return STATS_ERR;
			memset(ptempDebug, 0, sizeof(Statistics_debug_Table));
			pgDebug_StatsRealTimeOlt[ponId] = ptempDebug;
			ptempDebug = NULL;
			
		}*/
		gStatsPonOn[ponId] = onStatus;
	}
	else
		return STATS_ERR;
	
	return STATS_OK;	
}


/********************************************
* StatsRealTimeOnuPro
* 描述: 设置olt 在线的控制表
*
*
* 
'*
**********************************************/
int StatsRealTimeOnuPro (const short int ponId, const short int onuId, unsigned char onStatus)
{
	/*rtStatsEntry64 *pTempRtStats = NULL;
	rtStatsEntry *pTempPre = NULL;
	Statistics_debug_Table *ptempDebug = NULL;*/
	if (onStatus == STATS_DEVICE_OFF)
	{
		gStatsOnuOn[ponId][onuId] = onStatus;
		/*if (NULL != pgStatsRealTimeOnu[ponId][onuId])
		{
			VOS_Free((VOID *)pgStatsRealTimeOnu[ponId][onuId]);
			pgStatsRealTimeOnu[ponId][onuId] = NULL;
			VOS_Free((VOID *)pgDebug_StatsRealTimeOnu[ponId][onuId]);
			pgDebug_StatsRealTimeOnu[ponId][onuId] = NULL;
		}

		if( tpgStatsRealTimeOnu[ponId][onuId]!= NULL )
		{
			VOS_Free( tpgStatsRealTimeOnu[ponId][onuId]);
			tpgStatsRealTimeOnu[ponId][onuId] = NULL;
		}*/
	}
	else if (onStatus == STATS_DEVICE_ON)
	{
		/*if (NULL == pgStatsRealTimeOnu[ponId][onuId])
		{
			pTempRtStats = (rtStatsEntry64*)VOS_Malloc((ULONG)sizeof(rtStatsEntry64), (ULONG)MODULE_STATSTICS);
			if (NULL == pTempRtStats)
				return STATS_ERR;


			pTempPre = (rtStatsEntry*)VOS_Malloc((ULONG)sizeof(rtStatsEntry), (ULONG)MODULE_STATSTICS);
			if( pTempPre == NULL )
			{
				VOS_Free( pTempRtStats );
				return STATS_ERR;
			}

			ptempDebug = (Statistics_debug_Table*)VOS_Malloc((ULONG)sizeof(Statistics_debug_Table), (ULONG)MODULE_STATSTICS);
			if (NULL == ptempDebug)
			{
				VOS_Free( pTempRtStats );
				VOS_Free( pTempPre );
				return STATS_ERR;
			}


			
			memset(pTempRtStats, 0, sizeof(rtStatsEntry64));
			pgStatsRealTimeOnu[ponId][onuId] = pTempRtStats;
			pTempRtStats = NULL;

			memset( pTempPre, 0, sizeof(rtStatsEntry));
			tpgStatsRealTimeOnu[ponId][onuId]=pTempPre;
			pTempPre = NULL;
			
			memset(ptempDebug, 0, sizeof(Statistics_debug_Table));
			pgDebug_StatsRealTimeOnu[ponId][onuId] = ptempDebug;
			ptempDebug = NULL;
			
		}*/
		gStatsOnuOn[ponId][onuId] = onStatus;
	}
	else
		return STATS_ERR;


	
	return STATS_OK;	
}


/********************************************
* StatsRealTimeOnuPro
* 描述: 设置olt 在线的控制表
*
*
* 
'*
**********************************************/
int loopStatsRealTimeOnuPro (const short int ponId, const short int onuId, unsigned char onStatus)
{
	if (onStatus == STATS_DEVICE_OFF)
		{
		gStatsOnuOn[ponId][onuId] = onStatus;
		/*if (NULL != pgStatsRealTimeOnu[ponId][onuId])
			{
			VOS_Free((VOID *)pgStatsRealTimeOnu[ponId][onuId]);
			pgStatsRealTimeOnu[ponId][onuId] = NULL;
			}*/
		}
	else if (onStatus == STATS_DEVICE_ON)
		{
		/*if (NULL == pgStatsRealTimeOnu[ponId][onuId])
			{
			pTempRtStats = (rtStatsEntry*)VOS_Malloc((ULONG)sizeof(rtStatsEntry), (ULONG)MODULE_STATSTICS);
			if (NULL == pTempRtStats)
				return STATS_ERR;
			memset(pTempRtStats, 0, sizeof(rtStatsEntry));
			pgStatsRealTimeOnu[ponId][onuId] = (char *)pTempRtStats;
			pTempRtStats = NULL;
			}*/
		gStatsOnuOn[ponId][onuId] = onStatus;
		}
	else
		return STATS_ERR;
	return STATS_OK;	
}


#ifdef __RT_ONU_RAW
/*================================================================*/
/*函数:    StatsRealTimeEthRawGet*/
/*功能:    获取以太网端口实时统计数据*/
/*参数:    slotno: 端口所在板位号，port:端口在板上的位置*/
/*返回值:  VOS_OK或VOS_ERROR,赋值在函数内部完成*/
/*================================================================*/
/* begin: added by jianght 20090814 */
 extern ULONG userSlot_userPort_2_Ifindex( ULONG ulUserSlot, ULONG ulUserPort );
/* end: added by jianght 20090814 */
#define	report64x(a,b)	sys_console_printf( "\r\n%-40s0x%016x", a, b )
extern STATUS getBoardActMode( ulong_t devIdx, ulong_t brdIdx, ulong_t *ulMode );
STATUS	StatsRealTimeEthRawGet( short int slotno, short int port )
{
	STATUS	ret = VOS_ERROR;
	ETH_PORT_STATS_S stPortStats;
	static ulong rec = 0;
	rtStatsEntry64 *pEntry = NULL;
	/*添加端口数据的’RawGet‘函数，获取数据*/
	ULONG	ulIfx = 0/*, mode=0*/;
	
	ulIfx = userSlot_userPort_2_Ifindex( slotno, port );
	if( ulIfx == 0xffff )
		return VOS_ERROR;
    /*if( ( getBoardActMode( 1, OLT_SWITCH_BRD, &mode ) == VOS_OK ) && mode == 1 )
		ulIfx =	IFM_ETH_CREATE_INDEX( OLT_SWITCH_BRD, 20+port );
	else
		ulIfx = IFM_ETH_CREATE_INDEX( PONCARD_FIRST, 20+port );*/

	pEntry = &gStatsRealTimeEth[port-1];
	
	/*added by wangxy 2007-03-11 */
	/*TODO: 添加以太网端口的实时统计数据函数调用*/
	
	if( Get_ETH_PortStatInfor( ulIfx, &stPortStats ) == VOS_ERROR )
	{
			sys_console_printf( "\r\nctss_ifm_hook return error" );
					return ret;
	}

#if 0
	pEntry->ifIndex = ulIfx;
	pEntry->rtStatsBroadcastPkts = stPortStats.ulAllBroadcastPktsRecvSum;
	pEntry->rtStatsMulticastPkts = stPortStats.ulAllMulticastPktsRecvSum;
	pEntry->rtStatsPkts = stPortStats.ulAllUnicasePktsRecvSum;

	pEntry->rtStatsOctets = stPortStats.ulAllOctetRecvSum;
	pEntry->rtStatsPkts64Octets = stPortStats.ulRxFrames64;
	pEntry->rtStatsPkts65to127Octets = stPortStats.ulRxFrames65_127;
	pEntry->rtStatsPkts128to255Octets = stPortStats.ulRxFrames128_255;
	pEntry->rtStatsPkts256to511Octets = stPortStats.ulRxFrames256_511;
	pEntry->rtStatsPkts512to1023Octets = stPortStats.ulRxFrames512_1023;
	pEntry->rtStatsPkts1024to1518Octets = stPortStats.ulRxFrames1024_MaxFrame;

	pEntry->rtStatsOversizePkts = stPortStats.ulAllOversizeFrameSumRx;
	pEntry->rtStatsUndersizePkts = stPortStats.ulAllUndersizeFrameSumRx;
	pEntry->rtStatsCRCAlignErrors = stPortStats.ulAllFCSErrorFrameSumRx;
#endif


	pEntry->ifIndex = ulIfx;
	pEntry->rtStatsDropEvents = *(unsigned long long*)&stPortStats.ulAllEtherStatsDropEvents;
	pEntry->rtStatsBroadcastPkts = *(unsigned long long*)&stPortStats.ulAllEtherStatsBroadcastPkts;
	pEntry->rtStatsMulticastPkts = *(unsigned long long*)&stPortStats.ulAllEtherStatsMulticastPkts;
	pEntry->rtStatsPkts = *(unsigned long long*)&stPortStats.ulAllPacketSumRx;/*ulAllIfInUcastPkts;*/

	pEntry->rtStatsOctets = *(unsigned long long*)&stPortStats.ulAllIfInOctets;
	pEntry->rtStatsPkts64Octets = *(unsigned long long*)&stPortStats.ulAllEtherStatsPkts64Octets;
	pEntry->rtStatsPkts65to127Octets = *(unsigned long long*)&stPortStats.ulAllEtherStatsPkts65to127Octets;
	pEntry->rtStatsPkts128to255Octets = *(unsigned long long*)&stPortStats.ulAllEtherStatsPkts128to255Octets;
	pEntry->rtStatsPkts256to511Octets = *(unsigned long long*)&stPortStats.ulAllEtherStatsPkts256to511Octets;
	pEntry->rtStatsPkts512to1023Octets = *(unsigned long long*)&stPortStats.ulAllEtherStatsPkts512to1023Octets;
	pEntry->rtStatsPkts1024to1518Octets = *(unsigned long long*)&stPortStats.ulAllEtherStatsPkts1024to1518Octets;

	pEntry->rtStatsOversizePkts = *(unsigned long long*)&stPortStats.ulAllEtherStatsOversizePkts;
	pEntry->rtStatsUndersizePkts = *(unsigned long long*)&stPortStats.ulAllEtherStatsUndersizePkts;
	pEntry->rtStatsCRCAlignErrors = *(unsigned long long*)&stPortStats.ulAllEtherStatsCRCAlignErrors;

	pEntry->rtTxOctets=*(unsigned long long*)&stPortStats.ulAllIfOutOctets;
	pEntry->rtTxPkts=*(unsigned long long*)&stPortStats.ulAllPacketSumTx;/*ulAllIfOutUcastPkts;*/
	pEntry->rtTxBroadcastFrame=*(unsigned long long*)&stPortStats.ulAllIfHCOutBroadcastPckts;
	pEntry->rtTxMulticastFrame=*(unsigned long long*)&stPortStats.ulAllIfHCOutMulticastPkts;
	pEntry->rtTxDropFrame=*(unsigned long long*)&stPortStats.ulAllIfOutDiscards;

	pEntry->rtUpBandWidth=pEntry->rtStatsOctets*10/(RealTimeStatPeriod/1000);
	pEntry->rtDownBandWidth=pEntry->rtTxOctets*10/(RealTimeStatPeriod/1000);
       /*sys_console_printf ("ulAllIfInOctets=%lu\r\n",stPortStats.ulAllIfInOctets);
	sys_console_printf(" ulAllIfOutOctets=%lu\r\n",stPortStats.ulAllIfOutOctets);
	sys_console_printf ("ulAllEtherStatsOctets=%lu\r\n",stPortStats.ulAllEtherStatsOctets);
	report64x("ulAllIfInOctets", stPortStats.ulAllIfInOctets);
	report64x("ulAllIfOutOctets", stPortStats.ulAllIfOutOctets);
	report64x("ulAllEtherStatsOctets", stPortStats.ulAllEtherStatsOctets);*/


	/* begin: added by jianght 20090904 */
	/* 问题单:8182 */
	pEntry->rtStatsFragments = *(unsigned long long*)&stPortStats.ulAllEtherStatsFragments;
	/*sys_console_printf("StatsRealTimeEthRawGet(%d,%d)     rtFragments=%d\r\n", slotno, port, stPortStats.ulAllEtherStatsFragments.low);*/
	/* 问题单:8183 */
	pEntry->rtStatsJabbers = *(unsigned long long*)&stPortStats.ulAllEtherStatsJabbers;	/*ulAllDot1dBasePortMtuExceededDiscards;*/
	/*sys_console_printf("StatsRealTimeEthRawGet(%d,%d)     rtStatsJabbers=%d\r\n", slotno, port, stPortStats.ulAllDot1dBasePortMtuExceededDiscards.low);*/
	/* 问题单:8186 */
	pEntry->rtPausePkts = *(unsigned long long*)&stPortStats.ulAllDot3InPauseFrames;
	pEntry->rtTxPausePkts = *(unsigned long long*)&stPortStats.ulAllDot3OutPauseFrames;
	/*
	sys_console_printf("StatsRealTimeEthRawGet(%d,%d)     TxPausePkts=%d  RxPausePkts=%d\r\n", slotno, port, pEntry->rtTxPausePkts, pEntry->rtPausePkts);
	sys_console_printf("ulAllDot3InPauseFrames=%d  low=%d\r\n", stPortStats.ulAllDot3InPauseFrames, stPortStats.ulAllDot3InPauseFrames.low);
	sys_console_printf("unsigned long  long=%d  struct counter64=%d  unsigned long=%d", sizeof(unsigned long  long), sizeof(struct counter64), sizeof(unsigned long));
	*/
	/* 问题单:8187 */
	/*sys_console_printf("StatsRealTimeEthRawGet(%d,%d)     ulAllEtherStatsBroadcastPkts=%d\r\n", slotno, port, stPortStats.ulAllEtherStatsBroadcastPkts.low);*/
	/* end: added by jianght 20090904 */

	ret = VOS_OK;
	
       rec++;
	/*
	sys_console_printf( "\r\nget ehternet raw data, record num is: %u", rec );
	*/
	
	return	ret;
}

 /****************************************************
* StatsPonRawGet
* 描述: 获取pon对象的实时统计数据,从硬件获取
*		
*							
*								
*							
*		
*
*	输入: 无
*
*	输出: 无
*
*	返回: 无
******************************************************/
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
STATUS StatsRealTimePonRawGet(short int PonId)
{
	/* begin: added by jianght 20090814 */
	rtStatsEntry64 *pRtEntry = pgStatsRealTimeOlt[PonId];/* 64位 */
	ETH_PORT_STATS_S stPortStats;
	short int slot = 0, port = 0;
	ULONG ulIfx = 0;
	/* end: added by jianght 20090814 */
	
	short int iRes = 0;
	short int collector_id = 0;
	short int stat_parameter = 0;
	unsigned long	timestamp = 0 ;
	rtStatsEntry rtStatsTemp, rtStatsTempBak;

#ifdef __STAT_DEBUG
	unsigned char buf[265]={0};
	int len=0;
#endif
	/*rtStatsEntry64 *pRtEntry = pgStatsRealTimeOlt[PonId];*/
	rtStatsEntry *ppreEntry = tpgStatsRealTimeOlt[PonId];
	
	PON_total_dropped_rx_frames_raw_data_t	rxDropFramesRawData;
	PON_total_tx_dropped_frames_raw_data_t txDropFramesRawData;

	PON_total_frames_raw_data_t	totalFrameRawData_5001;
	PON_total_frames_raw_data_t	totalFrameRawData_5201;
	
	PON_broadcast_frames_raw_data_t		allbrdcstFramRawData;
	PON_multicast_frames_raw_data_t		allmulFrameRawData;
	PON_single_collision_frames_raw_data_t	singleCollisionFrameRawData;
	PON_multiple_collision_frames_raw_data_t	multCollisionFrameRawData;
	PON_frame_check_sequence_errors_raw_data_t allcheckSeqErrRawData;
	PON_in_range_length_errors_per_llid_raw_data_t allinRangeLenErrRawData;
	PON_frame_too_long_errors_per_llid_raw_data_t allframeTooLongErrLlidRawData;
	PON_grant_frames_raw_data_t 			grantTotalFrameRawData;
	PON_pause_frames_raw_data_t               totalPauseFrameRawData;
#ifdef __RT_ONU_RAW
	PON_broadcast_frames_raw_data_t		broadcastFramRawData;
	PON_multicast_frames_raw_data_t		mulFrameRawData;
	PON_frame_check_sequence_errors_raw_data_t checkSeqErrRawData;
	PON_in_range_length_errors_per_llid_raw_data_t inRangeLenErrRawData;
	PON_frame_too_long_errors_per_llid_raw_data_t frameTooLongErrLlidRawData;
	PON_grant_frames_raw_data_t 			grantFrameRawData;
	PON_pause_frames_raw_data_t               pauseFrameRawData;
#endif

	PON_total_octets_raw_data_t	totalOctetsRawData_5001;
	PON_total_octets_raw_data_t	totalOctetsRawData_5201;

	PON_transmitted_frames_per_llid_raw_data_t totalLlidBrdFrameSendRawData;
#ifdef __RT_ONU_RAW
	PON_transmitted_frames_per_llid_raw_data_t llidBrdFrameSendRawData;
#endif
    OLT_raw_stat_item_t    stat_item;

	/*
	PON_onu_fer_raw_data_t				onuFerRawData,allonuFerRawData;
	PON_onu_ber_raw_data_t				onuBerRawData, allOnuBerRawData;		
	PON_frame_too_long_errors_raw_data_t	frameTooLongErrRawData;		
	PON_alignmnet_errors_raw_data_t		alignmnetErrRawData,allAlignmnetErrRawData;
	PON_frame_too_long_errors_per_llid_raw_data_t frameTooLongErrPerLlidRaw;
	*/
	/*add by wangxy 2007-05-24*/
	short int ponChipType = V2R1_GetPonchipType( PonId );
	
	memset(&rtStatsTemp, 0, sizeof(rtStatsEntry));
	/*memcpy(&rtStatsTemp, ((rtStatsEntry *)pgStatsRealTimeOlt[PonId]), sizeof(rtStatsEntry));*/
	memset( &rtStatsTempBak, 0, sizeof(rtStatsEntry) );

#ifdef __RT_ONU_RAW 	/* modified by xieshl 20090204, 暂时去掉ONU统计信息 */
	{	/*Grant Frames*/
		short int llid = 0;
		short int onuId = 0;
		memset(&grantTotalFrameRawData, 0, sizeof(PON_grant_frames_raw_data_t));
		for(onuId = 0; onuId < MAXONUPERPON; onuId++)
		{/*grant frame*/
			llid = GetLlidByOnuIdx( PonId, onuId);
			if ((-1) == llid ) 
				continue;		 
			collector_id = PON_OLT_AND_ONU_COMBINED_ID;
			stat_parameter = llid;
			memset(&grantFrameRawData, 0, sizeof(PON_grant_frames_raw_data_t));
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_GRANT_FRAMES;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &grantFrameRawData;
		    stat_item.statistics_data_size = sizeof(PON_grant_frames_raw_data_t);
		    iRes = OLT_GetRawStatistics(PonId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics( PonId, 
											collector_id, 
											PON_RAW_STAT_GRANT_FRAMES, 
											stat_parameter, 
											&grantFrameRawData,
											&timestamp
											);
			#endif
			if (iRes == 0)
			{
				/*grantFrameRawData.received_ok*/
				grantTotalFrameRawData.received_ok += grantFrameRawData.received_ok;
				grantTotalFrameRawData.transmitted_ctrl_ok += grantFrameRawData.transmitted_ctrl_ok;
				grantTotalFrameRawData.transmitted_dba_ok += grantFrameRawData.transmitted_dba_ok;
			}
		}
	 		if (iRes != 0)
			{
#ifdef __STAT_DEBUG
				if( gStatisticDebugFlag == 1 )
					sys_console_printf("  PON_RAW_STAT_GRANT_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
#endif			
			}
			else
			{
#ifdef __STAT_DEBUG
				if( gStatisticDebugFlag == 1 )
				{
					sys_console_printf("  PonId %d llid %d\r\n",PonId,stat_parameter);
					sys_console_printf("  GrantFramRawData.transmitted_ok %lu\r\n",grantTotalFrameRawData.transmitted_dba_ok);
				}
#endif
			}
	}
#endif
	{/*获取接收到的总的帧数,包裹正确和错误帧的帧数*/
		/*olt = 1*/

		if( ponChipType == PONCHIP_PAS5001 )
		{
			VOS_MemZero( &totalFrameRawData_5001, sizeof(totalFrameRawData_5001) );
			collector_id = -1;	
			stat_parameter = 1;/*1-pon 2-system*/
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_FRAMES;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &totalFrameRawData_5001;
		    stat_item.statistics_data_size = sizeof(PON_total_frames_raw_data_t);
		    iRes = OLT_GetRawStatistics(PonId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( PonId, 
										   collector_id, 
										   PON_RAW_STAT_TOTAL_FRAMES, 
										   stat_parameter,
										   
										   &totalFrameRawData_5001,
										   &timestamp 
											);	
			#endif
		}
		else
		{
			VOS_MemZero( &totalFrameRawData_5201, sizeof(totalFrameRawData_5201) );
			collector_id = -1;	
			stat_parameter = 1;/*1-pon 2-system*/
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_FRAMES;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &totalFrameRawData_5201;
		    stat_item.statistics_data_size = sizeof(PON_total_frames_raw_data_t);
		    iRes = OLT_GetRawStatistics(PonId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( PonId, 
										   collector_id, 
										   PON_RAW_STAT_TOTAL_FRAMES, 
										   stat_parameter,
										   
										   &totalFrameRawData_5201,
										   &timestamp 
											);	
			#endif
		}		
		
		if(iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_TOTAL_FRAMES ERROR. PonId %d type Pon %d\r\n",PonId,stat_parameter);
#endif
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d type Pon %d\r\n",PonId,stat_parameter);	
			
				if( ponChipType == PONCHIP_PAS5001 )
				{	memset(buf, 0,256);
					len=sprintf (buf,"totalFrameRawData_5001.received_error  ");
					sprintf64Bits(buf+len, (unsigned long long )totalFrameRawData_5001.received_error);
					sys_console_printf ("%s\r\n",buf);

					memset(buf, 0,256);
					len=sprintf (buf,"totalFrameRawData_5001.received_ok  ");
					sprintf64Bits(buf+len, (unsigned long long )totalFrameRawData_5001.received_ok);
					sys_console_printf ("%s\r\n",buf);

					memset(buf, 0,256);
					len=sprintf (buf,"totalFrameRawData_5001.total_bad  ");
					sprintf64Bits(buf+len, (unsigned long long )totalFrameRawData_5001.total_bad);
					sys_console_printf ("%s\r\n",buf);

					memset(buf, 0,256);
					len=sprintf (buf,"totalFrameRawData_5001.transmitted_ok  ");
					sprintf64Bits(buf+len, (unsigned long long )totalFrameRawData_5001.transmitted_ok);
					sys_console_printf ("%s\r\n",buf);

					sys_console_printf("  totalFrameRawData_5001.received_error %lu\r\n",totalFrameRawData_5001.received_error);
					sys_console_printf("  totalFrameRawData_5001.received_ok %lu\r\n",totalFrameRawData_5001.received_ok);
					sys_console_printf("  totalFrameRawData_5001.total_bad %lu\r\n",totalFrameRawData_5001.total_bad);
					sys_console_printf("  totalFrameRawData_5001.transmitted_ok %lu\r\n",totalFrameRawData_5001.transmitted_ok);
					
				}
				else
				{
					sys_console_printf("  totalFrameRawData_5201.received_error %lu\r\n",(ULONG)totalFrameRawData_5201.received_error);
					sys_console_printf("  totalFrameRawData_5201.received_ok %lu\r\n",(ULONG)totalFrameRawData_5201.received_ok);
					sys_console_printf("  totalFrameRawData_5201.total_bad %lu\r\n",(ULONG)totalFrameRawData_5201.total_bad);
					sys_console_printf("  totalFrameRawData_5201.transmitted_ok %lu\r\n",(ULONG)totalFrameRawData_5201.transmitted_ok);
				}
			}
#endif
		}
	}

	{/*获取接收到的总的字节数*/
		/*olt = 1*/
		if( ponChipType == PONCHIP_PAS5001 )
		{
			VOS_MemZero(&totalOctetsRawData_5001, sizeof(totalOctetsRawData_5001));
			collector_id = -1;	
			stat_parameter = 1;/*1-pon 2-system*//*OLT_PHYSICAL_PORT_PON*/
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_OCTETS;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &totalOctetsRawData_5001;
		    stat_item.statistics_data_size = sizeof(PON_total_octets_raw_data_t);
		    iRes = OLT_GetRawStatistics(PonId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( PonId, 
										   collector_id, 
										   PON_RAW_STAT_TOTAL_OCTETS, 
										   stat_parameter,
										   &totalOctetsRawData_5001,
										   &timestamp 
											);	
			#endif
			
		}

		else
		{
			VOS_MemZero(&totalOctetsRawData_5201, sizeof(totalOctetsRawData_5201));
			collector_id = -1;	
			stat_parameter = 1;/*1-pon 2-system*//*OLT_PHYSICAL_PORT_PON*/
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_OCTETS;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &totalOctetsRawData_5201;
		    stat_item.statistics_data_size = sizeof(PON_total_octets_raw_data_t);
		    iRes = OLT_GetRawStatistics(PonId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( PonId, 
										   collector_id, 
										   PON_RAW_STAT_TOTAL_OCTETS, 
										   stat_parameter,
										   &totalOctetsRawData_5201,
										   &timestamp 
											);
			#endif
		}
		
		if(iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_TOTAL_FRAMES ERROR. PonId %d type Pon %d\r\n",PonId,stat_parameter);
#endif			
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d type Pon %d\r\n",PonId,stat_parameter);	
				if( ponChipType == PONCHIP_PAS5001 )
				{	memset(buf, 0,256);
					len=sprintf (buf,"totalOctetsRawData_5001.received_error  ");
					sprintf64Bits(buf+len, (unsigned long long )totalOctetsRawData_5001.received_error);
					sys_console_printf ("%s\r\n",buf);

					memset(buf, 0,256);
					len=sprintf (buf,"totalOctetsRawData_5001.received_ok  ");
					sprintf64Bits(buf+len, (unsigned long long )totalOctetsRawData_5001.received_ok);
					sys_console_printf ("%s\r\n",buf);

					memset(buf, 0,256);
					len=sprintf (buf,"totalOctetsRawData_5001.received_total  ");
					sprintf64Bits(buf+len, (unsigned long long )totalOctetsRawData_5001.received_total);
					sys_console_printf ("%s\r\n",buf);

					memset(buf, 0,256);
					len=sprintf (buf,"totalOctetsRawData_5001.transmitted_ok  ");
					sprintf64Bits(buf+len, (unsigned long long )totalOctetsRawData_5001.transmitted_ok);
					sys_console_printf ("%s\r\n",buf);
					
					sys_console_printf("  totalOctetsRawData_5001.received_error %lu\r\n",totalOctetsRawData_5001.received_error);
					sys_console_printf("  totalOctetsRawData_5001.received_ok %lu\r\n",totalOctetsRawData_5001.received_ok);
					sys_console_printf("  totalOctetsRawData_5001.received_total %lu\r\n",totalOctetsRawData_5001.received_total);
					sys_console_printf("  totalOctetsRawData_5001.transmitted_ok %lu\r\n",totalOctetsRawData_5001.transmitted_ok);
				}
				else
				{
					sys_console_printf("  totalOctetsRawData_5201.received_error %lu\r\n",(ULONG)totalOctetsRawData_5201.received_error);
					sys_console_printf("  totalOctetsRawData_5201.received_ok %lu\r\n",(ULONG)totalOctetsRawData_5201.received_ok);
					sys_console_printf("  totalOctetsRawData_5201.received_total %lu\r\n",(ULONG)totalOctetsRawData_5201.received_total);
					sys_console_printf("  totalOctetsRawData_5201.transmitted_ok %lu\r\n",(ULONG)totalOctetsRawData_5201.transmitted_ok);
				}
			}
#endif
		}
	}

	{
		/*olt*/
		memset(&rxDropFramesRawData, 0, sizeof(PON_total_dropped_rx_frames_raw_data_t));
			collector_id = -1;	
			stat_parameter = 0;
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &rxDropFramesRawData;
		    stat_item.statistics_data_size = sizeof(PON_total_dropped_rx_frames_raw_data_t);
		    iRes = OLT_GetRawStatistics(PonId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( PonId, 
										   collector_id, 
										   PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES, 
										   stat_parameter,
										   
										   &rxDropFramesRawData,
										   &timestamp 
											);	
			#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES ERROR. olt PonId %d\r\n",PonId);
#endif
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  olt PonId %d\r\n",PonId);
				sys_console_printf("  rxDropFramesRawData.source_alert_dropped %lu\r\n",rxDropFramesRawData.source_alert_dropped);
				sys_console_printf("  rxDropFramesRawData.total_control_dropped %lu\r\n", rxDropFramesRawData.total_control_dropped);
				sys_console_printf("  rxDropFramesRawData.pon_match %lu\r\n", rxDropFramesRawData.pon_match);
			}
#endif
		}
	}

	{
		/*olt*/
		memset(&txDropFramesRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t));
			collector_id = -1;	
			stat_parameter = 0;
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &txDropFramesRawData;
		    stat_item.statistics_data_size = sizeof(PON_total_tx_dropped_frames_raw_data_t);
		    iRes = OLT_GetRawStatistics(PonId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( PonId, 
										   collector_id, 
										   PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
										   stat_parameter,
										   
										   &txDropFramesRawData,
										   &timestamp 
											);	
			#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES ERROR. olt PonId %d\r\n",PonId);
#endif			
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  olt PonId %d\r\n",PonId);
				sys_console_printf("  txDropFramesRawData.total_pon_dropped %lu\r\n",txDropFramesRawData.total_pon_dropped);
				sys_console_printf("  txDropFramesRawData.total_system_dropped %lu\r\n",txDropFramesRawData.total_system_dropped);
			}
#endif
		}
		/*
		pgDebug_StatsRealTimeOlt[PonId]->Total_PON_dropped += txDropFramesRawData.total_pon_dropped;
		pgDebug_StatsRealTimeOlt[PonId]->Total_System_dropped += txDropFramesRawData.total_system_dropped;
		*/
	}

	/*Fragments*/
	/*Jabbers*/
	{/*single collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&singleCollisionFrameRawData, 0, sizeof(PON_single_collision_frames_raw_data_t));
		collector_id = -1;	
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_SINGLE_COLLISION_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &singleCollisionFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_single_collision_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_SINGLE_COLLISION_FRAMES, 
									   stat_parameter,
									   
									   &singleCollisionFrameRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			sys_console_printf("  PON_RAW_STAT_SINGLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
#endif			
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  olt PonId %d\r\n",PonId);
				sys_console_printf("  statistics_data.single_collision_frames %lu\r\n",singleCollisionFrameRawData.single_collision_frames);
			}
#endif
		}	
	}
	
	{/*multCollisionFrameRawData collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&multCollisionFrameRawData, 0, sizeof(PON_multiple_collision_frames_raw_data_t));
		collector_id = -1;	
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &multCollisionFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_multiple_collision_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES, 
									   stat_parameter,
									   
									   &multCollisionFrameRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
#endif
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  olt PonId %d\r\n",PonId);
				sys_console_printf("  multCollisionFrameRawData.multiple_collision_frames %lu\r\n",multCollisionFrameRawData.multiple_collision_frames);
			}
#endif
		}	
	}


#ifdef __RT_ONU_RAW 	/* modified by xieshl 20090204, 暂时去掉ONU统计信息 */
	{/*check sequence error, one err of FCS err*/
		/*olt = -1*/
		short int llid = 0;
		short int onuId = 0;
		memset(&allcheckSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		collector_id = -1;
		for(onuId = 0;onuId<MAXONUPERPON;onuId++)
		{
			llid = GetLlidByOnuIdx(PonId, onuId);
			if ( STAT_INVALID_LLID == llid )
				continue;	
			stat_parameter = llid;/*llid*/
			memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &checkSeqErrRawData;
		    stat_item.statistics_data_size = sizeof(PON_frame_check_sequence_errors_raw_data_t);
		    iRes = OLT_GetRawStatistics(PonId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( PonId, 
										   collector_id, 
										   PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS, 
										   stat_parameter,
										   
										   &checkSeqErrRawData,
										   &timestamp 
											);
			#endif
			if (iRes != 0)
			{
#ifdef __STAT_DEBUG
				if( gStatisticDebugFlag == 1 )
					sys_console_printf("  PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
#endif
			}
			else
			{
#ifdef __STAT_DEBUG
				if( gStatisticDebugFlag == 1 )
				{
					sys_console_printf("  PonId %d llid %d\r\n",PonId,stat_parameter);
					sys_console_printf("  checkSeqErrRawData.received %lu\r\n",checkSeqErrRawData.received);
				}
#endif
				allcheckSeqErrRawData.received += checkSeqErrRawData.received;
			}
			
		}

		/*--llid = 127--*/
		stat_parameter = PAS_BROADCAST_LLID;/*llid*/
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &checkSeqErrRawData;
	    stat_item.statistics_data_size = sizeof(PON_frame_check_sequence_errors_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS, 
									   stat_parameter,
									   
									   &checkSeqErrRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
#endif		
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d llid %d\r\n",PonId,stat_parameter);
				sys_console_printf("  checkSeqErrRawData.received %lu\r\n",checkSeqErrRawData.received);
			}
#endif
			allcheckSeqErrRawData.received += checkSeqErrRawData.received;	
		}	
	}	
#endif
#ifdef __RT_ONU_RAW 	/* modified by xieshl 20090204, 暂时去掉ONU统计信息 */
	{/*less than 60byte Err and ere otherwise well formed*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt = -1*/
		short int llid = 0;
		short int onuId = 0;
		memset(&allinRangeLenErrRawData, 0, sizeof(PON_in_range_length_errors_per_llid_raw_data_t));
		collector_id = -1;/*olt*/
		for(onuId = 0;onuId<MAXONUPERPON;onuId++)
		{
			llid = GetLlidByOnuIdx(PonId, onuId);
			if ( STAT_INVALID_LLID == llid )
				continue;	
			stat_parameter = llid;/*llid*/
			memset(&inRangeLenErrRawData, 0, sizeof(PON_in_range_length_errors_per_llid_raw_data_t));
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &inRangeLenErrRawData;
		    stat_item.statistics_data_size = sizeof(PON_in_range_length_errors_per_llid_raw_data_t);
		    iRes = OLT_GetRawStatistics(PonId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( PonId, 
										   collector_id, 
										   PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID, 
										   stat_parameter,
										   
										   &inRangeLenErrRawData,
										   &timestamp 
											);
			#endif
			if (iRes != 0)
			{
#ifdef __STAT_DEBUG
				if( gStatisticDebugFlag == 1 )
					sys_console_printf("  PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
#endif				
			}
			else
			{
#ifdef __STAT_DEBUG
				if( gStatisticDebugFlag == 1 )
				{
					sys_console_printf("  PonId %d llid %d\r\n",PonId,stat_parameter);
					sys_console_printf("  inRangeLenErrRawData.received %lu\r\n",inRangeLenErrRawData.received);
				}
#endif
				allinRangeLenErrRawData.received += inRangeLenErrRawData.received;
			}
			
		}

		/*--llid = 127--*/
		stat_parameter = PAS_BROADCAST_LLID;/*llid*/
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &inRangeLenErrRawData;
	    stat_item.statistics_data_size = sizeof(PON_in_range_length_errors_per_llid_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID, 
									   stat_parameter,
									   
									   &inRangeLenErrRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
		if( gStatisticDebugFlag == 1 )
			sys_console_printf("  PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
#endif			
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d llid %d\r\n",PonId,stat_parameter);
				sys_console_printf("  inRangeLenErrRawData.received %lu\r\n",inRangeLenErrRawData.received);
			}
#endif
			allinRangeLenErrRawData.received += inRangeLenErrRawData.received;
		}	
		
	}
#endif
#ifdef __RT_ONU_RAW 	/* modified by xieshl 20090204, 暂时去掉ONU统计信息 */
	{/*frameTooLongErrLlidRawData*/
		short int llid = 0;
		short int onuId = 0;	
		collector_id = -1;/*olt*/
		memset(&allframeTooLongErrLlidRawData, 0, sizeof(PON_frame_too_long_errors_per_llid_raw_data_t));
		for(onuId = 0;onuId < MAXONUPERPON;onuId++)
		{
			llid = GetLlidByOnuIdx(PonId, onuId);
			if ( STAT_INVALID_LLID == llid )
				continue;	
			stat_parameter = llid;/*llid*/		
			memset(&frameTooLongErrLlidRawData, 0, sizeof(PON_frame_too_long_errors_per_llid_raw_data_t));
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_TOO_LONG_ERRORS_PER_LLID;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &frameTooLongErrLlidRawData;
		    stat_item.statistics_data_size = sizeof(PON_frame_too_long_errors_per_llid_raw_data_t);
		    iRes = OLT_GetRawStatistics(PonId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( PonId, 
										   collector_id, 
										   PON_RAW_STAT_FRAME_TOO_LONG_ERRORS_PER_LLID, 
										   stat_parameter,
										   
										   &frameTooLongErrLlidRawData,
										   &timestamp 
											);	
			#endif
			if(iRes != 0)
			{
#ifdef __STAT_DEBUG
				if( gStatisticDebugFlag == 1 )
					sys_console_printf("  PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID ERROR. PonId %d ,llid %d\r\n",PonId,stat_parameter);
#endif			
			}
			else
			{
#ifdef __STAT_DEBUG
				if( gStatisticDebugFlag == 1 )
				{
					sys_console_printf("  PonId %d ,llid %d\r\n",PonId,stat_parameter);
					sys_console_printf("  frameTooLongErrLlidRawData.received %lu\r\n",frameTooLongErrLlidRawData.received);
				}
#endif
				allframeTooLongErrLlidRawData.received += frameTooLongErrLlidRawData.received;
			}
		}

		/*--llid = 127--*/
		stat_parameter = PAS_BROADCAST_LLID;/*llid*/		
		memset(&frameTooLongErrLlidRawData, 0, sizeof(PON_frame_too_long_errors_per_llid_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_TOO_LONG_ERRORS_PER_LLID;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &frameTooLongErrLlidRawData;
	    stat_item.statistics_data_size = sizeof(PON_frame_too_long_errors_per_llid_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_FRAME_TOO_LONG_ERRORS_PER_LLID, 
									   stat_parameter,
									   
									   &frameTooLongErrLlidRawData,
									   &timestamp 
										);	
		#endif
		if(iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID ERROR. PonId %d ,llid %d\r\n",PonId,stat_parameter);
#endif
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d ,llid %d\r\n",PonId,stat_parameter);
				sys_console_printf("  frameTooLongErrLlidRawData.received %lu\r\n",frameTooLongErrLlidRawData.received);
			}
#endif
			allframeTooLongErrLlidRawData.received += frameTooLongErrLlidRawData.received;
		}
		
	}
#endif

#ifdef __RT_ONU_RAW 	/* modified by xieshl 20090204, 暂时去掉ONU统计信息 */
	{/*获取接收到的组播数据*/
		/*olt = -1*/
		short int llid = 0;
		short int onuId = 0;
		memset(&allmulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
		collector_id = -1;
		for(onuId = 0;onuId<MAXONUPERPON;onuId++)
		{
			llid = GetLlidByOnuIdx(PonId, onuId);
			if ( STAT_INVALID_LLID == llid )
				continue;		
			stat_parameter = llid;/*llid*/
			memset(&mulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_MULTICAST_FRAMES;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &mulFrameRawData;
		    stat_item.statistics_data_size = sizeof(PON_multicast_frames_raw_data_t);
		    iRes = OLT_GetRawStatistics(PonId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( PonId, 
										   collector_id, 
										   PON_RAW_STAT_MULTICAST_FRAMES, 
										   stat_parameter,
										   
										   &mulFrameRawData,
										   &timestamp 
											);
			#endif
			if (iRes != 0)
			{
#ifdef __STAT_DEBUG
				if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_MULTICAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
#endif
			}
			else
			{
#ifdef __STAT_DEBUG
				if( gStatisticDebugFlag == 1 )
				{
					sys_console_printf("  PonId %d llid %d\r\n",PonId,stat_parameter);
					sys_console_printf("  mulFrameRawData.received_ok %lu\r\n",mulFrameRawData.received_ok);
					sys_console_printf("  mulFrameRawData.transmitted_ok %lu\r\n",mulFrameRawData.transmitted_ok);
				}
#endif				
				allmulFrameRawData.received_ok += mulFrameRawData.received_ok;
				allmulFrameRawData.transmitted_ok += mulFrameRawData.transmitted_ok;
			}

		}

		/*--llid = 127--*/
		stat_parameter = PAS_BROADCAST_LLID;/*llid*/
		memset(&mulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_MULTICAST_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &mulFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_multicast_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTICAST_FRAMES, 
									   stat_parameter,
									   
									   &mulFrameRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_MULTICAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
#endif		
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d llid %d\r\n",PonId,stat_parameter);
				sys_console_printf("  mulFrameRawData.received_ok %lu\r\n",mulFrameRawData.received_ok);
				sys_console_printf("  mulFrameRawData.transmitted_ok %lu\r\n",mulFrameRawData.transmitted_ok);
			}
#endif
			allmulFrameRawData.received_ok += mulFrameRawData.received_ok;
			allmulFrameRawData.transmitted_ok += mulFrameRawData.transmitted_ok;	
		}
	
	}
#endif
	
#ifdef __RT_ONU_RAW 	/* modified by xieshl 20090204, 暂时去掉ONU统计信息 */
	{/*获取接收到的广播数据*/
		/*olt = -1*/
		short int llid = 0;
		short int onuId = 0;
		memset(&allbrdcstFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
		collector_id = -1;	
		for(onuId = 0;onuId<MAXONUPERPON;onuId++)
		{
			llid = GetLlidByOnuIdx(PonId, onuId);
			if ( STAT_INVALID_LLID == llid )
				continue;
			stat_parameter = llid;/*llid*/
			memset(&broadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_BROADCAST_FRAMES;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &broadcastFramRawData;
		    stat_item.statistics_data_size = sizeof(PON_broadcast_frames_raw_data_t);
		    iRes = OLT_GetRawStatistics(PonId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( PonId, 
										   collector_id, 
										   PON_RAW_STAT_BROADCAST_FRAMES, 
										   stat_parameter,
										   
										   &broadcastFramRawData,
										   &timestamp 
											);	
			#endif
			if (iRes != 0)
			{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_BROADCAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
#endif			
			}
			else
			{
#ifdef __STAT_DEBUG
				if( gStatisticDebugFlag == 1 )
				{
					sys_console_printf("  PonId %d llid %d\r\n",PonId,stat_parameter);
					sys_console_printf("  broadcastFramRawData.received_ok %lu\r\n",broadcastFramRawData.received_ok);
					sys_console_printf("  broadcastFramRawData.transmitted_ok %lu\r\n",broadcastFramRawData.transmitted_ok);
				}
#endif
				allbrdcstFramRawData.received_ok += broadcastFramRawData.received_ok;
				allbrdcstFramRawData.transmitted_ok += broadcastFramRawData.transmitted_ok; 
			}

			
		}/*end of for(llid = 0;...*/
		
		/*--llid = 127--*/
		stat_parameter = PAS_BROADCAST_LLID;
		memset(&broadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_BROADCAST_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &broadcastFramRawData;
	    stat_item.statistics_data_size = sizeof(PON_broadcast_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
										   collector_id, 
										   PON_RAW_STAT_BROADCAST_FRAMES, 
										   stat_parameter,
										   
										   &broadcastFramRawData,
										   &timestamp 
											);	
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_BROADCAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
#endif		
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d llid %d\r\n",PonId,stat_parameter);
				sys_console_printf("  broadcastFramRawData.received_ok %lu\r\n",broadcastFramRawData.received_ok);
				sys_console_printf("  broadcastFramRawData.transmitted_ok %lu\r\n",broadcastFramRawData.transmitted_ok);
			}
#endif
			allbrdcstFramRawData.received_ok += broadcastFramRawData.received_ok;
			allbrdcstFramRawData.transmitted_ok += broadcastFramRawData.transmitted_ok; 
		}
		
	}
#endif

#ifdef __RT_ONU_RAW 	/* modified by xieshl 20090204, 暂时去掉ONU统计信息 */
	{
	short int llid = 0;
	short int onuId = 0;
	memset(&totalLlidBrdFrameSendRawData, 0, sizeof(PON_transmitted_frames_per_llid_raw_data_t));
	for(onuId = 0;onuId < MAXONUPERPON; onuId++)
	{
		llid = GetLlidByOnuIdx( PonId, onuId);
		if ((-1) == llid ) 
			continue;
		collector_id = -1;	
		stat_parameter = llid;
		memset(&llidBrdFrameSendRawData, 0, sizeof(PON_transmitted_frames_per_llid_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &llidBrdFrameSendRawData;
	    stat_item.statistics_data_size = sizeof(PON_transmitted_frames_per_llid_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID, 
									   stat_parameter,
									   &llidBrdFrameSendRawData,
									   &timestamp 
										);	
		#endif
		
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_GRANT_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
#endif		
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d llid %d\r\n",PonId,stat_parameter);
				sys_console_printf("  NewFramRawData.transmitted_ok %lu\r\n",totalLlidBrdFrameSendRawData.pon_ok);
			}
#endif
			totalLlidBrdFrameSendRawData.pon_ok += llidBrdFrameSendRawData.pon_ok;
			totalLlidBrdFrameSendRawData.system_ok += llidBrdFrameSendRawData.system_ok;
		}
	}
}
#endif

#ifdef __RT_ONU_RAW 	/* modified by xieshl 20090204, 暂时去掉ONU统计信息 */
{/*pause frame*/
	short int llid = 0;
	short int onuId = 0;
	for(onuId = 0; onuId < MAXONUPERPON; onuId++)
	{
	llid = GetLlidByOnuIdx( PonId, onuId);
	if ((-1) == llid ) 
		continue;
	collector_id = -1;
	stat_parameter = llid;
	memset(&pauseFrameRawData, 0, sizeof(PON_pause_frames_raw_data_t));
	#if 1
	VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
	stat_item.collector_id = collector_id;
    stat_item.raw_statistics_type = PON_RAW_STAT_PAUSE_FRAMES;
    stat_item.statistics_parameter = stat_parameter;
	
    stat_item.statistics_data = &pauseFrameRawData;
    stat_item.statistics_data_size = sizeof(PON_pause_frames_raw_data_t);
    iRes = OLT_GetRawStatistics(PonId, &stat_item);
	timestamp = stat_item.timestam;
	#else
	iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_PAUSE_FRAMES, 
										stat_parameter, 
										&pauseFrameRawData,
										&timestamp);
	#endif
}
	if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_PAUSE_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
#endif		
		}
	else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d llid %d\r\n",PonId,stat_parameter);
				sys_console_printf("  PON_RAW_STAT_PAUSE_FRAMES_RECEIVE %lu\r\n",pauseFrameRawData.pon_received_ok);
				sys_console_printf("  PON_RAW_STAT_PAUSE_FRAMES_transmite %lu\r\n",pauseFrameRawData.pon_transmitted_ok);
			}
#endif
			totalPauseFrameRawData.pon_received_ok+=pauseFrameRawData.pon_received_ok;
			totalPauseFrameRawData.pon_transmitted_ok+=pauseFrameRawData.pon_transmitted_ok;
		}
}
#endif

	if( ponChipType == PONCHIP_PAS5001 )
	{
		rtStatsTemp.rtStatsOctets = totalOctetsRawData_5001.received_error+totalOctetsRawData_5001.received_ok;
		rtStatsTemp.rtStatsErrOctets = totalOctetsRawData_5001.received_error;
		rtStatsTemp.rtStatsTransmmitOctets = totalOctetsRawData_5001.transmitted_ok;
		
		
		rtStatsTemp.rtStatsPkts =  totalFrameRawData_5001.received_error+totalFrameRawData_5001.received_ok;
		rtStatsTemp.rtStatsErrPkts = totalFrameRawData_5001.received_error;
		rtStatsTemp.rtStatsTransmmitPkts = totalLlidBrdFrameSendRawData.pon_ok;
		rtStatsTemp.rtTxOctets=totalOctetsRawData_5001.transmitted_ok;
	}
	else
	{
		rtStatsTemp.rtStatsOctets = totalOctetsRawData_5201.received_error+totalOctetsRawData_5201.received_ok;
		rtStatsTemp.rtStatsErrOctets = totalOctetsRawData_5201.received_error;
		rtStatsTemp.rtStatsTransmmitOctets = totalLlidBrdFrameSendRawData.pon_ok;
		
		
		rtStatsTemp.rtStatsPkts =  totalFrameRawData_5201.received_error+totalFrameRawData_5201.received_ok;
		rtStatsTemp.rtStatsErrPkts = totalFrameRawData_5201.received_error;
		rtStatsTemp.rtStatsTransmmitPkts = totalFrameRawData_5201.transmitted_ok;		

		rtStatsTemp.rtTxOctets=totalOctetsRawData_5201.transmitted_ok;
	}
	
	rtStatsTemp.rtStatsBroadcastPkts =  allbrdcstFramRawData.received_ok;
	rtStatsTemp.rtStatsMulticastPkts = allmulFrameRawData.received_ok;	
	rtStatsTemp.rtStatsCRCAlignErrors = allcheckSeqErrRawData.received/* + aligErrRawData.received*/;
	rtStatsTemp.rtStatsUndersizePkts = allinRangeLenErrRawData.received;
	rtStatsTemp.rtStatsOversizePkts = allframeTooLongErrLlidRawData.received;
	rtStatsTemp.rtStatsCollisions = (singleCollisionFrameRawData.single_collision_frames + 	\
									multCollisionFrameRawData.multiple_collision_frames);
	rtStatsTemp.rtStatsDropEvents = rxDropFramesRawData.source_alert_dropped+rxDropFramesRawData.total_control_dropped;
	rtStatsTemp.rtStatsFragments = 0;
	rtStatsTemp.rtStatsJabbers = 0;
	
	rtStatsTemp.rtStatsPkts64Octets = 0;
	rtStatsTemp.rtStatsPkts65to127Octets = 0;
	rtStatsTemp.rtStatsPkts128to255Octets = 0;
	rtStatsTemp.rtStatsPkts256to511Octets = 0;
	rtStatsTemp.rtStatsPkts512to1023Octets = 0;
	rtStatsTemp.rtStatsPkts1024to1518Octets = 0;	

	rtStatsTemp.rtTxDropFrame=txDropFramesRawData.total_system_dropped;
	rtStatsTemp.rtTxBroadcastFrame=allbrdcstFramRawData.transmitted_ok;
	rtStatsTemp.rtTxMulticastFrame=allmulFrameRawData.transmitted_ok;
	rtStatsTemp.rtTxGrantPkts=grantTotalFrameRawData.received_ok;
	rtStatsTemp.rtTxPausePkts=totalPauseFrameRawData.pon_transmitted_ok;
	rtStatsTemp.rtPausePkts=totalPauseFrameRawData.pon_received_ok;
	
	
	VOS_MemCpy( &rtStatsTempBak, &rtStatsTemp, sizeof( rtStatsEntry) );

	if( rtStatsTemp.rtStatsOctets < ppreEntry->rtStatsOctets )
		rtStatsTemp.rtStatsOctets = 0xffffffff-ppreEntry->rtStatsOctets+rtStatsTemp.rtStatsOctets;
	else
		rtStatsTemp.rtStatsOctets = rtStatsTemp.rtStatsOctets-ppreEntry->rtStatsOctets;
	
	if( rtStatsTemp.rtStatsErrOctets < ppreEntry->rtStatsErrOctets )
		rtStatsTemp.rtStatsErrOctets = 0xffffffff-ppreEntry->rtStatsErrOctets+rtStatsTemp.rtStatsErrOctets;
	else
		rtStatsTemp.rtStatsErrOctets = rtStatsTemp.rtStatsErrOctets-ppreEntry->rtStatsErrOctets;

	if( rtStatsTemp.rtStatsTransmmitOctets < ppreEntry->rtStatsTransmmitOctets )
		rtStatsTemp.rtStatsTransmmitOctets = 0xffffffff-ppreEntry->rtStatsTransmmitOctets+rtStatsTemp.rtStatsTransmmitOctets;
	else
		rtStatsTemp.rtStatsTransmmitOctets = rtStatsTemp.rtStatsTransmmitOctets-ppreEntry->rtStatsTransmmitOctets;

	if( rtStatsTemp.rtStatsPkts < ppreEntry->rtStatsPkts )
		rtStatsTemp.rtStatsPkts = 0xffffffff-ppreEntry->rtStatsPkts+rtStatsTemp.rtStatsPkts;
	else
		rtStatsTemp.rtStatsPkts = rtStatsTemp.rtStatsPkts-ppreEntry->rtStatsPkts;		

	if( rtStatsTemp.rtStatsErrPkts < ppreEntry->rtStatsErrPkts )
		rtStatsTemp.rtStatsErrPkts = 0xffffffff-ppreEntry->rtStatsErrPkts+rtStatsTemp.rtStatsErrPkts;
	else
		rtStatsTemp.rtStatsErrPkts = rtStatsTemp.rtStatsErrPkts-ppreEntry->rtStatsErrPkts;

	if( rtStatsTemp.rtStatsTransmmitPkts < ppreEntry->rtStatsTransmmitPkts )
		rtStatsTemp.rtStatsTransmmitPkts = 0xffffffff-ppreEntry->rtStatsTransmmitPkts+rtStatsTemp.rtStatsTransmmitPkts;
	else
		rtStatsTemp.rtStatsTransmmitPkts = rtStatsTemp.rtStatsTransmmitPkts-ppreEntry->rtStatsTransmmitPkts;	

	if( rtStatsTemp.rtStatsBroadcastPkts < ppreEntry->rtStatsBroadcastPkts )
		rtStatsTemp.rtStatsBroadcastPkts = 0xffffffff-ppreEntry->rtStatsBroadcastPkts+rtStatsTemp.rtStatsBroadcastPkts;
	else
		rtStatsTemp.rtStatsBroadcastPkts = rtStatsTemp.rtStatsBroadcastPkts-ppreEntry->rtStatsBroadcastPkts;	

	if( rtStatsTemp.rtStatsMulticastPkts < ppreEntry->rtStatsMulticastPkts )
		rtStatsTemp.rtStatsMulticastPkts = 0xffffffff-ppreEntry->rtStatsMulticastPkts+rtStatsTemp.rtStatsMulticastPkts;
	else
		rtStatsTemp.rtStatsMulticastPkts = rtStatsTemp.rtStatsMulticastPkts -ppreEntry->rtStatsMulticastPkts;	

	if( rtStatsTemp.rtStatsCRCAlignErrors < ppreEntry->rtStatsCRCAlignErrors )
		rtStatsTemp.rtStatsCRCAlignErrors = 0xffffffff-ppreEntry->rtStatsCRCAlignErrors+rtStatsTemp.rtStatsCRCAlignErrors;
	else
		rtStatsTemp.rtStatsCRCAlignErrors = rtStatsTemp.rtStatsCRCAlignErrors -ppreEntry->rtStatsCRCAlignErrors;	

	if( rtStatsTemp.rtStatsUndersizePkts < ppreEntry->rtStatsUndersizePkts )
		rtStatsTemp.rtStatsUndersizePkts = 0xffffffff-ppreEntry->rtStatsUndersizePkts+rtStatsTemp.rtStatsUndersizePkts;
	else
		rtStatsTemp.rtStatsUndersizePkts = rtStatsTemp.rtStatsUndersizePkts -ppreEntry->rtStatsUndersizePkts;	

	if( rtStatsTemp.rtStatsOversizePkts < ppreEntry->rtStatsOversizePkts )
		rtStatsTemp.rtStatsOversizePkts = 0xffffffff-ppreEntry->rtStatsOversizePkts+rtStatsTemp.rtStatsOversizePkts;
	else
		rtStatsTemp.rtStatsOversizePkts = rtStatsTemp.rtStatsOversizePkts -ppreEntry->rtStatsOversizePkts;	

	if( rtStatsTemp.rtStatsCollisions < ppreEntry->rtStatsCollisions )
		rtStatsTemp.rtStatsCollisions = 0xffffffff-ppreEntry->rtStatsCollisions+rtStatsTemp.rtStatsCollisions;
	else
		rtStatsTemp.rtStatsCollisions = rtStatsTemp.rtStatsCollisions -ppreEntry->rtStatsCollisions;

	if( rtStatsTemp.rtStatsDropEvents< ppreEntry->rtStatsDropEvents )
		rtStatsTemp.rtStatsDropEvents = 0xffffffff-ppreEntry->rtStatsDropEvents+rtStatsTemp.rtStatsDropEvents;
	else
		rtStatsTemp.rtStatsDropEvents = rtStatsTemp.rtStatsDropEvents -ppreEntry->rtStatsDropEvents;	


	if(rtStatsTemp.rtTxDropFrame<ppreEntry->rtTxDropFrame)
		rtStatsTemp.rtTxDropFrame=0xffffffff-ppreEntry->rtTxDropFrame+rtStatsTemp.rtTxDropFrame;
	else
		rtStatsTemp.rtTxDropFrame=rtStatsTemp.rtTxDropFrame-ppreEntry->rtTxDropFrame;

	if(rtStatsTemp.rtTxBroadcastFrame<ppreEntry->rtTxBroadcastFrame)
		rtStatsTemp.rtTxBroadcastFrame=0xffffffff-ppreEntry->rtTxBroadcastFrame+rtStatsTemp.rtTxBroadcastFrame;
	else
		rtStatsTemp.rtTxBroadcastFrame=rtStatsTemp.rtTxBroadcastFrame-ppreEntry->rtTxBroadcastFrame;
	if(rtStatsTemp.rtTxMulticastFrame<ppreEntry->rtTxMulticastFrame)
		rtStatsTemp.rtTxMulticastFrame=0xffffffff-ppreEntry->rtTxMulticastFrame+rtStatsTemp.rtTxMulticastFrame;
	else
		rtStatsTemp.rtTxMulticastFrame=rtStatsTemp.rtTxMulticastFrame-ppreEntry->rtTxMulticastFrame;

	if(rtStatsTemp.rtTxOctets<ppreEntry->rtTxOctets)
		rtStatsTemp.rtTxOctets=0xffffffff-ppreEntry->rtTxOctets+rtStatsTemp.rtTxOctets;
	else
		rtStatsTemp.rtTxOctets=rtStatsTemp.rtTxOctets-ppreEntry->rtTxOctets;
	
	if(rtStatsTemp.rtTxGrantPkts<ppreEntry->rtTxGrantPkts)
		rtStatsTemp.rtTxGrantPkts=0xffffffff-ppreEntry->rtTxGrantPkts+rtStatsTemp.rtTxGrantPkts;
	else
		rtStatsTemp.rtTxGrantPkts=rtStatsTemp.rtTxGrantPkts-ppreEntry->rtTxGrantPkts;

	if(rtStatsTemp.rtTxPausePkts<ppreEntry->rtTxPausePkts)
		rtStatsTemp.rtTxPausePkts=0xffffffff-ppreEntry->rtTxPausePkts+rtStatsTemp.rtTxPausePkts;
	else
		rtStatsTemp.rtTxPausePkts=rtStatsTemp.rtTxPausePkts-ppreEntry->rtTxPausePkts;
	if(rtStatsTemp.rtPausePkts<ppreEntry->rtPausePkts)
		rtStatsTemp.rtPausePkts=0xffffffff-ppreEntry->rtPausePkts+rtStatsTemp.rtPausePkts;
	else
		rtStatsTemp.rtPausePkts=rtStatsTemp.rtPausePkts-ppreEntry->rtPausePkts;

	/*{
		long long rtTxGrantOctets = rtStatsTemp.rtTxGrantPkts *64;
		if( rtStatsTemp.rtStatsTransmmitPkts > rtStatsTemp.rtTxGrantPkts )
			rtStatsTemp.rtStatsTransmmitPkts -= rtStatsTemp.rtTxGrantPkts;
		else
			rtStatsTemp.rtStatsTransmmitPkts = 0;
		if( rtStatsTemp.rtStatsTransmmitOctets > rtTxGrantOctets )
			rtStatsTemp.rtStatsTransmmitOctets -= rtTxGrantOctets;
		else
			rtStatsTemp.rtStatsTransmmitOctets = 0;
	}*/
	
	pgStatsRealTimeOlt[PonId]->rtStatsOctets += rtStatsTemp.rtStatsOctets;
	pgStatsRealTimeOlt[PonId]->rtStatsErrOctets += rtStatsTemp.rtStatsErrOctets;
	pgStatsRealTimeOlt[PonId]->rtStatsTransmmitOctets += rtStatsTemp.rtStatsTransmmitOctets;
	pgStatsRealTimeOlt[PonId]->rtStatsPkts += rtStatsTemp.rtStatsPkts;
	pgStatsRealTimeOlt[PonId]->rtStatsErrPkts += rtStatsTemp.rtStatsErrPkts;
	pgStatsRealTimeOlt[PonId]->rtStatsTransmmitPkts += rtStatsTemp.rtStatsTransmmitPkts;
	pgStatsRealTimeOlt[PonId]->rtStatsCRCAlignErrors += rtStatsTemp.rtStatsCRCAlignErrors;
	pgStatsRealTimeOlt[PonId]->rtStatsDropEvents += rtStatsTemp.rtStatsDropEvents;
	pgStatsRealTimeOlt[PonId]->rtStatsBroadcastPkts += rtStatsTemp.rtStatsBroadcastPkts;
	pgStatsRealTimeOlt[PonId]->rtStatsMulticastPkts += rtStatsTemp.rtStatsMulticastPkts;
	pgStatsRealTimeOlt[PonId]->rtStatsOversizePkts += rtStatsTemp.rtStatsOversizePkts;
	pgStatsRealTimeOlt[PonId]->rtStatsUndersizePkts += rtStatsTemp.rtStatsUndersizePkts;
	pgStatsRealTimeOlt[PonId]->rtStatsCollisions += rtStatsTemp.rtStatsCollisions;

	pgStatsRealTimeOlt[PonId]->rtTxDropFrame+=rtStatsTemp.rtTxDropFrame;
	pgStatsRealTimeOlt[PonId]->rtTxBroadcastFrame+=rtStatsTemp.rtTxBroadcastFrame;
	pgStatsRealTimeOlt[PonId]->rtTxMulticastFrame+=rtStatsTemp.rtTxMulticastFrame;
	pgStatsRealTimeOlt[PonId]->rtTxOctets+=rtStatsTemp.rtTxOctets;
	pgStatsRealTimeOlt[PonId]->rtTxGrantPkts+=rtStatsTemp.rtTxGrantPkts;
	pgStatsRealTimeOlt[PonId]->rtTxPausePkts+=rtStatsTemp.rtTxPausePkts;
	pgStatsRealTimeOlt[PonId]->rtPausePkts+=rtStatsTemp.rtPausePkts;
	pgStatsRealTimeOlt[PonId]->rtTxPkts=pgStatsRealTimeOlt[PonId]->rtStatsTransmmitPkts;

#if  0
	sys_console_printf("\r\n1:zhengyt:real octets=%d\r\n",pgStatsRealTimeOlt[PonId]->rtStatsOctets);
	sys_console_printf("1:zhengyt:past octets=%d\r\n",tpgStatsRealTimeOlt[PonId]->rtStatsOctets);
	sys_console_printf("1:zhengyt:real txoctets=%d\r\n",pgStatsRealTimeOlt[PonId]->rtTxOctets);
	sys_console_printf("1:zhengyt:past txoctets=%d\r\n",tpgStatsRealTimeOlt[PonId]->rtTxOctets);

	pgStatsRealTimeOlt[PonId]->rtUpBandWidth=((pgStatsRealTimeOlt[PonId]->rtStatsOctets-tpgStatsRealTimeOlt[PonId]->rtStatsOctets)*10/(RealTimeStatPeriod/1000));
	pgStatsRealTimeOlt[PonId]->rtDownBandWidth=((pgStatsRealTimeOlt[PonId]->rtTxOctets-tpgStatsRealTimeOlt[PonId]->rtTxOctets)*10/(RealTimeStatPeriod/1000));
	


	calculateBandWidth_Pon( PonId ,&rtStatsTemp);
	if(gStatisticDebugFlag==1)
		{
			memset(buf, 0,256);
			len=sprintf (buf,"rtStatsTemp.rtUpBandWidth");
			sprintf64Bits(buf+len, (unsigned long long )rtStatsTemp.rtUpBandWidth);
			sys_console_printf ("%s\r\n",buf);
			
			memset(buf, 0,256);
			len=sprintf (buf,"rtStatsTemp.rtDownBandWidth");
			sprintf64Bits(buf+len, (unsigned long long )rtStatsTemp.rtDownBandWidth);
			sys_console_printf ("%s\r\n",buf);
		}

	pgStatsRealTimeOlt[PonId]->rtUpBandWidth=rtStatsTemp.rtUpBandWidth;
	pgStatsRealTimeOlt[PonId]->rtDownBandWidth=rtStatsTemp.rtDownBandWidth;

	sys_console_printf("\r\n3:zhengyt:up bandwidth=%d\r\n",pgStatsRealTimeOlt[PonId]->rtUpBandWidth);
	sys_console_printf("3:zhengyt:down bandwidth=%d\r\n",pgStatsRealTimeOlt[PonId]->rtDownBandWidth);
	/*pgStatsRealTimeOlt[PonId]->rtTxPkts-=pgStatsRealTimeOlt[PonId]->rtTxGrantPkts;*//*最终的发送总帧数是要减去受权帧后的帧数*/
	/*pgStatsRealTimeOlt[PonId]->rtTxMulticastFrame-=pgStatsRealTimeOlt[PonId]->rtTxGrantPkts;*//*最终的发送多播帧数是要减去受权帧后的帧数*/
	/*pgStatsRealTimeOlt[PonId]->rtTxBroadcastFrame-=pgStatsRealTimeOlt[PonId]->rtTxGrantPkts;*/

#endif

	
	VOS_MemCpy( tpgStatsRealTimeOlt[PonId], &rtStatsTempBak, sizeof( rtStatsEntry ) );

	{
		/*
		monPonAlarmCheck(PonId, 
						allonuFerRawData.received_error, rtStatsTemp.rtStatsPkts,
						allOnuBerRawData.error_bytes, (ulong_t)allOnuBerRawData.error_bytes);*/
		monPonAlarmCheck( PonId, rtStatsTemp.rtStatsErrPkts, 
							rtStatsTemp.rtStatsPkts, rtStatsTemp.rtStatsErrOctets, rtStatsTemp.rtStatsOctets );
		
	}
	/*sys_console_printf("4:zhengyt:past octets=%d\r\n",tpgStatsRealTimeOlt[PonId]->rtStatsOctets);*/
/*计算OLT的PON端口统计BER、FER以及实时带宽*/
	calculatePonBer( &rtStatsTemp );
	calculatePonFer( &rtStatsTemp );
	calculateRtBandWidth( &rtStatsTemp, OLT_PON_PORT );

	pgStatsRealTimeOlt[PonId]->rtUpBandWidth=rtStatsTemp.rtUpBandWidth;
	pgStatsRealTimeOlt[PonId]->rtDownBandWidth=rtStatsTemp.rtDownBandWidth;

	/*sys_console_printf("\r\n3:zhengyt:up bandwidth=%d\r\n",pgStatsRealTimeOlt[PonId]->rtUpBandWidth);
	sys_console_printf("3:zhengyt:down bandwidth=%d\r\n",pgStatsRealTimeOlt[PonId]->rtDownBandWidth);*/

/*	memcpy(pgStatsRealTimeOlt[PonId], &rtStatsTemp, sizeof(rtStatsEntry));*/
	if(stat_result_show == 1)
	{
	/*	#if STATS_DEBUG*/
	}

	/* begin: added by jianght 20090814 */
	/* 大部分统计项从平台中的全局结构体取得，余下7项还是从硬件取得(即不动，以上都已经赋值好了) */
	slot = GetCardIdxByPonChip(PonId);
	port = GetPonPortByPonChip(PonId);

	/*sys_console_printf("PonId=%d  slot=%d  port=%d\r\n", PonId, slot, port);*/

	ulIfx = userSlot_userPort_2_Ifindex(slot, port);
	if( ulIfx == 0xffff )
	{
		sys_console_printf( "StatsRealTimePonRawGet(%d)::userSlot_userPort_2_Ifindex(%d,%d)\r\n    error!", PonId, slot, port);
		return VOS_ERROR;
	}

	if( Get_ETH_PortStatInfor( ulIfx, &stPortStats ) == VOS_ERROR )
	{
		sys_console_printf( "StatsRealTimePonRawGet(%d)::Get_ETH_PortStatInfor()\r\n    error!", PonId);
		return VOS_ERROR;
	}

	/* 64位重新赋值 */
	pRtEntry->ifIndex = ulIfx;
	pRtEntry->rtStatsOctets = *(unsigned long long*)&stPortStats.ulAllIfInOctets;
	pRtEntry->rtStatsPkts = *(unsigned long long*)&stPortStats.ulAllPacketSumRx;
	pRtEntry->rtStatsCRCAlignErrors = *(unsigned long long*)&stPortStats.ulAllEtherStatsCRCAlignErrors;
	pRtEntry->rtStatsDropEvents = *(unsigned long long*)&stPortStats.ulAllEtherStatsDropEvents;
	pRtEntry->rtStatsBroadcastPkts = *(unsigned long long*)&stPortStats.ulAllEtherStatsBroadcastPkts;
	pRtEntry->rtStatsMulticastPkts = *(unsigned long long*)&stPortStats.ulAllEtherStatsMulticastPkts;
	pRtEntry->rtStatsOversizePkts = *(unsigned long long*)&stPortStats.ulAllEtherStatsOversizePkts;
	pRtEntry->rtStatsUndersizePkts = *(unsigned long long*)&stPortStats.ulAllEtherStatsUndersizePkts;
	pRtEntry->rtStatsCollisions = *(unsigned long long*)&stPortStats.ulAllEtherStatsCollisions;
	pRtEntry->rtTxDropFrame = *(unsigned long long*)&stPortStats.ulAllIfOutDiscards;
	pRtEntry->rtTxBroadcastFrame = *(unsigned long long*)&stPortStats.ulAllIfHCOutBroadcastPckts;
	pRtEntry->rtTxMulticastFrame = *(unsigned long long*)&stPortStats.ulAllIfHCOutMulticastPkts;
	pRtEntry->rtTxOctets = *(unsigned long long*)&stPortStats.ulAllIfOutOctets;
	pRtEntry->rtTxPkts = *(unsigned long long*)&stPortStats.ulAllPacketSumTx;
	pRtEntry->rtUpBandWidth = *(unsigned long*)&stPortStats.ulInOctetRate * 10;
	pRtEntry->rtDownBandWidth = *(unsigned long*)&stPortStats.ulOutOctetRate * 10;

	/* 32位重新赋值 */
	ppreEntry->ifIndex = ulIfx;
	ppreEntry->rtStatsOctets = pRtEntry->rtStatsOctets;
	ppreEntry->rtStatsPkts = (unsigned long)pRtEntry->rtStatsPkts;
	ppreEntry->rtStatsCRCAlignErrors = (unsigned long)pRtEntry->rtStatsCRCAlignErrors;
	ppreEntry->rtStatsDropEvents = (unsigned long)pRtEntry->rtStatsDropEvents;
	ppreEntry->rtStatsBroadcastPkts = (unsigned long)pRtEntry->rtStatsBroadcastPkts;
	ppreEntry->rtStatsMulticastPkts = (unsigned long)pRtEntry->rtStatsMulticastPkts;
	ppreEntry->rtStatsOversizePkts = (unsigned long)pRtEntry->rtStatsOversizePkts;
	ppreEntry->rtStatsUndersizePkts = (unsigned long)pRtEntry->rtStatsUndersizePkts;
	ppreEntry->rtStatsCollisions = (unsigned long)pRtEntry->rtStatsCollisions;
	ppreEntry->rtTxDropFrame = pRtEntry->rtTxDropFrame;
	ppreEntry->rtTxMulticastFrame = pRtEntry->rtTxMulticastFrame;
	ppreEntry->rtTxOctets = pRtEntry->rtTxOctets;
	ppreEntry->rtTxPkts = pRtEntry->rtTxPkts;
	ppreEntry->rtUpBandWidth = pRtEntry->rtUpBandWidth;
	ppreEntry->rtDownBandWidth = pRtEntry->rtDownBandWidth;
	/* end: added by jianght 20090814 */

	return (STATS_OK);
}

/****************************************************
* StatsOnuRawGet
* 描述: 获取onu对象的历史统计
*		
*							
*								
*							
*		
*
*	输入: 无
*
*	输出: 无
*
*	返回: 无
*****************************************************/

STATUS StatsRealTimeOnuRawGet(unsigned short ponId, unsigned short onuId)
{
	short int iRes = STATS_OK;
	short int llid = 0;
	short int collector_id = 0;
	short int stat_parameter = 0;
	rtStatsEntry *ppreEntry = tpgStatsRealTimeOnu[ponId][onuId];
	unsigned char buf[265]={0};
	int len=0;
	
	/*PON_total_dropped_rx_frames_raw_data_t	rxDropFramesRawData;
	PON_total_tx_dropped_frames_raw_data_t txDropFramesRawData;*/
	PON_total_frames_raw_data_t	totalFrameRawData_5001;
	PON_total_frames_raw_data_t	totalFrameRawData_5201;

	/*PON_total_octets_raw_data_t totalOctetsRawData;*/
	PON_total_octets_raw_data_t	totalOctetsRawData_5001;
	PON_total_octets_raw_data_t	totalOctetsRawData_5201;
	PON_broadcast_frames_raw_data_t		broadcastFramRawData;
	PON_multicast_frames_raw_data_t		mulFrameRawData;
	PON_alignmnet_errors_raw_data_t		alignmnetErrRawData;
	/*PON_frame_too_long_errors_per_llid_raw_data_t frameTooLongErrPerLlidRaw;*/
	PON_single_collision_frames_raw_data_t	singleCollisionFrameRawData;
	PON_multiple_collision_frames_raw_data_t	multCollisionFrameRawData;
	/*added by wutuw at august nine
	PON_olt_ber_raw_data_t				oltBerRawData;*/
	/*PON_onu_fer_raw_data_t				onuFerRawData;
	PON_onu_ber_raw_data_t				onuBerRawData;*/
	PON_frame_check_sequence_errors_raw_data_t checkSeqErrRawData;
	
	/*PON_in_range_length_errors_per_llid_raw_data_t inRangeLenErrRawData;*/
	/*PON_in_range_length_errors_raw_data_t	inRangeLeErrSystemRawData;*/
	PON_frame_too_long_errors_raw_data_t	frameTooLongErrRawData;
	/*PON_frame_too_long_errors_per_llid_raw_data_t 
	frameTooLongErrLlidRawData;*/
	/*PON_hec_frames_errors_raw_data_t		hecFrameErrRawData;*/

	PON_grant_frames_raw_data_t 			grantTotalFrameRawData;
	PON_grant_frames_raw_data_t 			grantFrameRawData;

	PON_pause_frames_raw_data_t               pauseFrameRawData;


	/*add by wangxy 2007-05-24*/
	short int ponChipType = V2R1_GetPonchipType( ponId );
	
	unsigned long timeStamp = 0;
	
	rtStatsEntry rtStatsTemp, rtStatsTempBak;
	OLT_raw_stat_item_t    stat_item;
	
	memset(&rtStatsTemp, 0, sizeof(rtStatsEntry));
	/*memcpy(&rtStatsTemp, (rtStatsEntry *)pgStatsRealTimeOnu[ponId][onuId], sizeof(rtStatsEntry));*/
	memset( &rtStatsTempBak, 0 , sizeof( rtStatsEntry ) );
	
	llid = GetLlidByOnuIdx(ponId, onuId);
	if ( STAT_INVALID_LLID == llid )
	{
#ifdef __STAT_DEBUG
		if( gStatisticDebugFlag == 1 )
			sys_console_printf(" ponId %d onuId %d llid %d is not online.\r\n",ponId,onuId,llid);
#endif		
		return STATS_ERR;
	}

		/*----------------------------------------------------*/	
	
	{/*获取接收到的总的帧数,包裹正确和错误帧的帧数*/
		/*--llid  = 1--*/

		if( ponChipType == PONCHIP_PAS5001 )
		{
			VOS_MemZero( &totalFrameRawData_5001, sizeof(totalFrameRawData_5001) );
			collector_id = llid;	
			stat_parameter = 1;/*1-pon 2-system*/
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_FRAMES;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &totalFrameRawData_5001;
		    stat_item.statistics_data_size = sizeof(PON_total_frames_raw_data_t);
		    iRes = OLT_GetRawStatistics(ponId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( ponId, 
										   collector_id, 
										   PON_RAW_STAT_TOTAL_FRAMES, 
										   stat_parameter,
										   
										   &totalFrameRawData_5001,
										   &timeStamp 
											);
			#endif
		}
		else
		{
			VOS_MemZero( &totalFrameRawData_5201, sizeof(totalFrameRawData_5201) );
			collector_id = llid;	
			stat_parameter = 1;/*1-pon 2-system*/
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_FRAMES;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &totalFrameRawData_5201;
		    stat_item.statistics_data_size = sizeof(PON_total_frames_raw_data_t);
		    iRes = OLT_GetRawStatistics(ponId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( ponId, 
										   collector_id, 
										   PON_RAW_STAT_TOTAL_FRAMES, 
										   stat_parameter,
										   
										   &totalFrameRawData_5201,
										   &timeStamp 
											);	
			#endif
		}		
		
		if(iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_TOTAL_FRAMES ERROR. PonId %d onuId %d\r\n",ponId,collector_id);
#endif			
			return STATS_ERR;
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d Llid %d\r\n",ponId,collector_id);
				if( ponChipType == PONCHIP_PAS5001)
				{
					memset(buf, 0,256);
					len=sprintf (buf,"totalFrameRawData_5001.received_error  ");
					sprintf64Bits(buf+len, (unsigned long long )totalFrameRawData_5001.received_error);
					sys_console_printf ("%s\r\n",buf);

					memset(buf, 0,256);
					len=sprintf (buf,"totalFrameRawData_5001.received_ok  ");
					sprintf64Bits(buf+len, (unsigned long long )totalFrameRawData_5001.received_ok);
					sys_console_printf ("%s\r\n",buf);

					memset(buf, 0,256);
					len=sprintf (buf,"totalFrameRawData_5001.total_bad  ");
					sprintf64Bits(buf+len, (unsigned long long )totalFrameRawData_5001.total_bad);
					sys_console_printf ("%s\r\n",buf);

					memset(buf, 0,256);
					len=sprintf (buf,"totalFrameRawData_5001.transmitted_ok  ");
					sprintf64Bits(buf+len, (unsigned long long )totalFrameRawData_5001.transmitted_ok);
					sys_console_printf ("%s\r\n",buf);
					
					sys_console_printf("  totalFrameRawData_5001.received_error %lu\r\n",totalFrameRawData_5001.received_error);
					sys_console_printf("  totalFrameRawData_5001.received_ok %lu\r\n",totalFrameRawData_5001.received_ok);
					sys_console_printf("  totalFrameRawData_5001.total_bad %lu\r\n",totalFrameRawData_5001.total_bad);
					sys_console_printf("  totalFrameRawData_5001.transmitted_ok %lu\r\n",totalFrameRawData_5001.transmitted_ok);
				}
				else 
				{
					sys_console_printf("  totalFrameRawData_5201.received_error %lu\r\n",totalFrameRawData_5201.received_error);
					sys_console_printf("  totalFrameRawData_5201.received_ok %lu\r\n",totalFrameRawData_5201.received_ok);
					sys_console_printf("  totalFrameRawData_5201.total_bad %lu\r\n",totalFrameRawData_5201.total_bad);
					sys_console_printf("  totalFrameRawData_5201.transmitted_ok %lu\r\n",totalFrameRawData_5201.transmitted_ok);
				}
			}
#endif
		}	
	}
			
	{
/*获取收到的总的字节数*/
		
		if( ponChipType == PONCHIP_PAS5001 )
		{
			VOS_MemZero(&totalOctetsRawData_5001, sizeof(totalOctetsRawData_5001));
			collector_id = llid;	
			stat_parameter = 1;/*1-pon 2-system*//*OLT_PHYSICAL_PORT_PON*/
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_OCTETS;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &totalOctetsRawData_5001;
		    stat_item.statistics_data_size = sizeof(PON_total_octets_raw_data_t);
		    iRes = OLT_GetRawStatistics(ponId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( ponId, 
										   collector_id, 
										   PON_RAW_STAT_TOTAL_OCTETS, 
										   stat_parameter,
										   &totalOctetsRawData_5001,
										   &timeStamp 
											);	
			#endif
		}

		else
		{
			VOS_MemZero(&totalOctetsRawData_5201, sizeof(totalOctetsRawData_5201));
			collector_id = llid;	
			stat_parameter = 1;/*1-pon 2-system*//*OLT_PHYSICAL_PORT_PON*/
			#if 1
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = collector_id;
		    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_OCTETS;
		    stat_item.statistics_parameter = stat_parameter;
			
		    stat_item.statistics_data = &totalOctetsRawData_5201;
		    stat_item.statistics_data_size = sizeof(PON_total_octets_raw_data_t);
		    iRes = OLT_GetRawStatistics(ponId, &stat_item);
			timestamp = stat_item.timestam;
			#else
			iRes = PAS_get_raw_statistics ( ponId, 
										   collector_id, 
										   PON_RAW_STAT_TOTAL_OCTETS, 
										   stat_parameter,
										   &totalOctetsRawData_5201,
										   &timeStamp 
											);
			#endif
		}			
						
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_OLT_BER ERROR.PonId %d ,Llid %d\r\n",ponId,collector_id);
#endif			
			return STATS_ERR;
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d ,Llid %d\r\n",ponId,collector_id);
				if( ponChipType == PONCHIP_PAS5001 )
				{
					memset(buf, 0,256);
					len=sprintf (buf,"totalOctetsRawData_5001.received_error  ");
					sprintf64Bits(buf+len, (unsigned long long )totalOctetsRawData_5001.received_error);
					sys_console_printf ("%s\r\n",buf);

					memset(buf, 0,256);
					len=sprintf (buf,"totalOctetsRawData_5001.received_ok  ");
					sprintf64Bits(buf+len, (unsigned long long )totalOctetsRawData_5001.received_ok);
					sys_console_printf ("%s\r\n",buf);

					memset(buf, 0,256);
					len=sprintf (buf,"totalOctetsRawData_5001.received_total  ");
					sprintf64Bits(buf+len, (unsigned long long )totalOctetsRawData_5001.received_total);
					sys_console_printf ("%s\r\n",buf);

					memset(buf, 0,256);
					len=sprintf (buf,"totalOctetsRawData_5001.transmitted_ok  ");
					sprintf64Bits(buf+len, (unsigned long long )totalOctetsRawData_5001.transmitted_ok);
					sys_console_printf ("%s\r\n",buf);
					
					sys_console_printf("  totalOctetsRawData_5001.received_ok %d\r\n",totalOctetsRawData_5001.received_ok );
					sys_console_printf("  totalOctetsRawData_5001.received_error %d\r\n",totalOctetsRawData_5001.received_error);
					sys_console_printf("  totalOctetsRawData_5001.received_total %d\r\n",totalOctetsRawData_5001.received_total );
					sys_console_printf("  totalOctetsRawData_5001.transmitted_ok %d\r\n",totalOctetsRawData_5001.transmitted_ok);
				}
				else 
				{
					sys_console_printf("  totalOctetsRawData_5201.received_ok %d\r\n",(ULONG)totalOctetsRawData_5201.received_ok );
					sys_console_printf("  totalOctetsRawData_5201.received_error %d\r\n",(ULONG)totalOctetsRawData_5201.received_error);
					sys_console_printf("  totalOctetsRawData_5201.received_total %d\r\n",(ULONG)totalOctetsRawData_5201.received_total );
					sys_console_printf("  totalOctetsRawData_5201.transmitted_ok %d\r\n",(ULONG)totalOctetsRawData_5201.transmitted_ok);
				}
			}
#endif
		}		
	}		


	{/*获取接收到的广播数据*/
		memset(&broadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
		collector_id = llid;	
		/*stat_parameter = 0; modify by wangxy 2007-03-29*/
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_BROADCAST_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &broadcastFramRawData;
	    stat_item.statistics_data_size = sizeof(PON_broadcast_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(ponId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_BROADCAST_FRAMES, 
									   stat_parameter,
									   
									   &broadcastFramRawData,
									   &timeStamp 
										);
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_BROADCAST_FRAMES ERROR.PonId %d ,Llid %d\r\n",ponId,collector_id);
#endif		
			return STATS_ERR;
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d ,Llid %d\r\n",ponId,collector_id);
				sys_console_printf("  broadcastFramRawData.received_ok %lu\r\n",broadcastFramRawData.received_ok);
				sys_console_printf("  broadcastFramRawData.transmitted_ok %lu\r\n",broadcastFramRawData.transmitted_ok);
			}
#endif
		}
	}
			#if 0
			iRes = HisStatsRawGet ( ponId, collectorId, PON_RAW_STAT_BROADCAST_FRAMES, \
											0,  (void *)&broadcastFramRawData,
											 &timeStamp );
			if (iRes != STATS_OK)
				return iRes;	
			#endif
	{/*获取接收到的广播数据*/
		memset(&mulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
		collector_id = llid;	
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_MULTICAST_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &mulFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_multicast_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(ponId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_MULTICAST_FRAMES, 
									   stat_parameter,
									   
									   &mulFrameRawData,
									   &timeStamp 
										);
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_MULTICAST_FRAMES ERROR.PonId %d ,Llid %d\r\n",ponId,collector_id);
#endif			
			return STATS_ERR;
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d ,Llid %d\r\n",ponId,collector_id);
				sys_console_printf("  mulFrameRawData.received_ok %lu\r\n",mulFrameRawData.received_ok);
				sys_console_printf("  mulFrameRawData.transmitted_ok %lu\r\n",mulFrameRawData.transmitted_ok);
			}
#endif
		}
	}		
			#if 0
			/*获取接收到的组播数据*/
			/*iRes = HisStatsRawGet ( ponId, collectorId, PON_RAW_STAT_MULTICAST_FRAMES, \
											0,  (void *)&mulFrameRawData,
											 &timeStamp );*/
			iRes = HisStatsRawGet ( ponId, (-1), PON_RAW_STAT_MULTICAST_FRAMES, \
											collectorId,  (void *)&mulFrameRawData,
											 &timeStamp );											 
			if (iRes != STATS_OK)
				return iRes;	
			#endif
	{/*check sequence error, one err of FCS err*/ 
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		collector_id = llid;	
		/*stat_parameter = 0; modify by wangxy 2007-03-29*/
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &checkSeqErrRawData;
	    stat_item.statistics_data_size = sizeof(PON_frame_check_sequence_errors_raw_data_t);
	    iRes = OLT_GetRawStatistics(ponId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS, 
									   stat_parameter,
									   
									   &checkSeqErrRawData,
									   &timeStamp 
										);
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS ERROR.PonId %d ,Llid %d\r\n",ponId,collector_id);
#endif		
			return STATS_ERR;
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d ,Llid %d\r\n",ponId,collector_id);
				sys_console_printf("  checkSeqErrRawData.received %lu\r\n",checkSeqErrRawData.received);
			}
#endif
		}
	}	


	{/*Alignment err. one err of FCS err . Only for onu*/
			
		memset(&alignmnetErrRawData, 0, sizeof(PON_alignmnet_errors_raw_data_t));
		collector_id = llid;	
		/*stat_parameter = 0; modify by wangxy 2007-03-29*/
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_ALIGNMENT_ERRORS;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &alignmnetErrRawData;
	    stat_item.statistics_data_size = sizeof(PON_alignmnet_errors_raw_data_t);
	    iRes = OLT_GetRawStatistics(ponId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_ALIGNMENT_ERRORS, 
									   stat_parameter,
									   
									   &alignmnetErrRawData,
									   &timeStamp 
										);
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_ALIGNMENT_ERRORS ERROR.PonId %d ,Llid %d\r\n",ponId,collector_id);
#endif			
			return STATS_ERR;
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d ,Llid %d\r\n",ponId,collector_id);
				sys_console_printf("  alignmnetErrRawData.received %lu\r\n",alignmnetErrRawData.received);
			}
#endif
		}
	}		



	{/*frame too long err*/
	/*system of frame too long err. for onu*/
		
		memset(&frameTooLongErrRawData, 0, sizeof(PON_frame_too_long_errors_raw_data_t));
		collector_id = llid;	
		/*stat_parameter = 0; modify by wangxy 2007-03-29*/
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_TOO_LONG_ERRORS;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &frameTooLongErrRawData;
	    stat_item.statistics_data_size = sizeof(PON_frame_too_long_errors_raw_data_t);
	    iRes = OLT_GetRawStatistics(ponId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_FRAME_TOO_LONG_ERRORS, 
									   stat_parameter,
									   
									   &frameTooLongErrRawData,
									   &timeStamp 
										);
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_FRAME_TOO_LONG_ERRORS ERROR.PonId %d ,Llid %d\r\n",ponId,collector_id);
#endif			
			return STATS_ERR;
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d ,Llid %d\r\n",ponId,collector_id);
				sys_console_printf(" frameTooLongErrRawData.pon_received %lu\r\n",frameTooLongErrRawData.pon_received);
				sys_console_printf(" frameTooLongErrRawData.system_received %lu\r\n",frameTooLongErrRawData.system_received);
			}
#endif
		}
	}	


			/*Fragments*/
			/*Jabbers*/
	{	/*获取接收到的Collisionsd 的数据包*/
			/*single collision frame*/
			
		memset(&singleCollisionFrameRawData, 0, sizeof(PON_single_collision_frames_raw_data_t));
		memset(&multCollisionFrameRawData, 0, sizeof(PON_multiple_collision_frames_raw_data_t));	
		collector_id = llid;	
		/*stat_parameter = 0; modify by wangxy 2007-03-29*/
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_SINGLE_COLLISION_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &singleCollisionFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_single_collision_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(ponId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_SINGLE_COLLISION_FRAMES, 
									   stat_parameter,
									   
									   &singleCollisionFrameRawData,
									   &timeStamp 
										);
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_SINGLE_COLLISION_FRAMES ERROR.PonId %d ,Llid %d\r\n",ponId,collector_id);
#endif			
			return STATS_ERR;
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d ,Llid %d\r\n",ponId,collector_id);
				sys_console_printf(" frameTooLongErrRawData.pon_received %lu\r\n",singleCollisionFrameRawData.single_collision_frames);
			}
#endif
		}
	}

	{

		/*multiple collision frame*/
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &multCollisionFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_multiple_collision_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(ponId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES, 
									   stat_parameter,
									   
									   &multCollisionFrameRawData,
									   &timeStamp 
										);
		#endif
		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_SINGLE_COLLISION_FRAMES ERROR.PonId %d ,Llid %d\r\n",ponId,collector_id);
#endif			
			return STATS_ERR;
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d ,Llid %d\r\n",ponId,collector_id);
				sys_console_printf("  multCollisionFrameRawData.multiple_collision_frames %lu\r\n",multCollisionFrameRawData.multiple_collision_frames);
			}
#endif
		}
	}	

{
	memset(&grantTotalFrameRawData, 0, sizeof(PON_grant_frames_raw_data_t));
	memset(&grantFrameRawData, 0, sizeof(PON_grant_frames_raw_data_t));
	collector_id = llid;
	stat_parameter = 0;
	/*grant frame*/
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_GRANT_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &grantFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_grant_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(ponId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_GRANT_FRAMES, 
										stat_parameter, 
										&grantFrameRawData,
										&timeStamp
										);
		#endif
		if (iRes == 0)
		{
			/*grantFrameRawData.received_ok*/
			grantTotalFrameRawData.received_ok = grantFrameRawData.received_ok;
			grantTotalFrameRawData.transmitted_ctrl_ok = grantFrameRawData.transmitted_ctrl_ok;
			grantTotalFrameRawData.transmitted_dba_ok = grantFrameRawData.transmitted_dba_ok;
		}
	
 		if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_GRANT_FRAMES ERROR. PonId %d llid %d\r\n",ponId,stat_parameter);
#endif		
		}
		else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d llid %d\r\n",ponId,stat_parameter);
				sys_console_printf("  GrantFramRawData.transmitted_ok %lu\r\n",grantTotalFrameRawData.transmitted_dba_ok);
			}
#endif			
		}
}


{/*pause frame*/
	collector_id = llid;
	stat_parameter = 0;
	memset(&pauseFrameRawData, 0, sizeof(PON_pause_frames_raw_data_t));
	#if 1
	VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
	stat_item.collector_id = collector_id;
    stat_item.raw_statistics_type = PON_RAW_STAT_PAUSE_FRAMES;
    stat_item.statistics_parameter = stat_parameter;
	
    stat_item.statistics_data = &pauseFrameRawData;
    stat_item.statistics_data_size = sizeof(PON_pause_frames_raw_data_t);
    iRes = OLT_GetRawStatistics(ponId, &stat_item);
	timestamp = stat_item.timestam;
	#else
	iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_PAUSE_FRAMES, 
										stat_parameter, 
										&pauseFrameRawData,
										&timeStamp);
	#endif

	if (iRes != 0)
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
				sys_console_printf("  PON_RAW_STAT_PAUSE_FRAMES ERROR. PonId %d llid %d\r\n",ponId,stat_parameter);
#endif		
		}
	else
		{
#ifdef __STAT_DEBUG
			if( gStatisticDebugFlag == 1 )
			{
				sys_console_printf("  PonId %d llid %d\r\n",ponId,stat_parameter);
				sys_console_printf("  PON_RAW_STAT_PAUSE_FRAMES_RECEIVE %lu\r\n",pauseFrameRawData.pon_received_ok);
				sys_console_printf("  PON_RAW_STAT_PAUSE_FRAMES_transmite %lu\r\n",pauseFrameRawData.pon_transmitted_ok);
			}
#endif
		}
}

	rtStatsTemp.rtStatsDropEvents = 0;

#ifdef __RT_CALCULATE_BW
	calculateBandWidth_Onu(ponId,onuId,&rtStatsTemp);
#endif 		
		if( ponChipType == PONCHIP_PAS5001 )
		{
			rtStatsTemp.rtStatsOctets = totalOctetsRawData_5001.received_error+totalOctetsRawData_5001.received_ok;
			rtStatsTemp.rtStatsErrOctets = totalOctetsRawData_5001.received_error;
			rtStatsTemp.rtStatsTransmmitOctets = totalOctetsRawData_5001.transmitted_ok;
			
			
			rtStatsTemp.rtStatsPkts =  totalFrameRawData_5001.received_error+totalFrameRawData_5001.received_ok;
			rtStatsTemp.rtStatsErrPkts = totalFrameRawData_5001.received_error;
			rtStatsTemp.rtStatsTransmmitPkts = totalFrameRawData_5001.transmitted_ok;
		}
		else
		{
			rtStatsTemp.rtStatsOctets = totalOctetsRawData_5201.received_error+totalOctetsRawData_5201.received_ok;
			rtStatsTemp.rtStatsErrOctets = totalOctetsRawData_5201.received_error;
			rtStatsTemp.rtStatsTransmmitOctets = totalOctetsRawData_5201.transmitted_ok;
			
			
			rtStatsTemp.rtStatsPkts =  totalFrameRawData_5201.received_error+totalFrameRawData_5201.received_ok;
			rtStatsTemp.rtStatsErrPkts = totalFrameRawData_5201.received_error;
			rtStatsTemp.rtStatsTransmmitPkts = totalFrameRawData_5201.transmitted_ok;		
		} 		
 				
		rtStatsTemp.rtStatsBroadcastPkts =  broadcastFramRawData.received_ok;
		rtStatsTemp.rtStatsMulticastPkts = mulFrameRawData.received_ok;	
		rtStatsTemp.rtStatsCRCAlignErrors = checkSeqErrRawData.received + alignmnetErrRawData.received;
		rtStatsTemp.rtStatsUndersizePkts = 0;
		rtStatsTemp.rtStatsOversizePkts = frameTooLongErrRawData.pon_received;
		rtStatsTemp.rtStatsFragments = 0;
		rtStatsTemp.rtStatsJabbers = 0;
		rtStatsTemp.rtStatsCollisions = singleCollisionFrameRawData.single_collision_frames + multCollisionFrameRawData.multiple_collision_frames;
		rtStatsTemp.rtStatsPkts64Octets = 0;
		rtStatsTemp.rtStatsPkts65to127Octets = 0;
		rtStatsTemp.rtStatsPkts128to255Octets = 0;
		rtStatsTemp.rtStatsPkts256to511Octets = 0;
		rtStatsTemp.rtStatsPkts512to1023Octets = 0;
		rtStatsTemp.rtStatsPkts1024to1518Octets = 0;	

		rtStatsTemp.rtTxMulticastFrame=mulFrameRawData.transmitted_ok;
		rtStatsTemp.rtTxBroadcastFrame=broadcastFramRawData.transmitted_ok;
		rtStatsTemp.rtTxPausePkts=pauseFrameRawData.pon_transmitted_ok;
		rtStatsTemp.rtPausePkts=pauseFrameRawData.pon_received_ok;
		rtStatsTemp.rtTxGrantPkts=grantFrameRawData.transmitted_ctrl_ok;

		

	VOS_MemCpy( &rtStatsTempBak, &rtStatsTemp, sizeof( rtStatsEntry) );

	if( rtStatsTemp.rtStatsOctets < ppreEntry->rtStatsOctets )
		rtStatsTemp.rtStatsOctets = 0xffffffff-ppreEntry->rtStatsOctets+rtStatsTemp.rtStatsOctets;
	else
		rtStatsTemp.rtStatsOctets = rtStatsTemp.rtStatsOctets-ppreEntry->rtStatsOctets;
	
	if( rtStatsTemp.rtStatsErrOctets < ppreEntry->rtStatsErrOctets )
		rtStatsTemp.rtStatsErrOctets = 0xffffffff-ppreEntry->rtStatsErrOctets+rtStatsTemp.rtStatsErrOctets;
	else
		rtStatsTemp.rtStatsErrOctets = rtStatsTemp.rtStatsErrOctets-ppreEntry->rtStatsErrOctets;

	if( rtStatsTemp.rtStatsTransmmitOctets < ppreEntry->rtStatsTransmmitOctets )
		rtStatsTemp.rtStatsTransmmitOctets = 0xffffffff-ppreEntry->rtStatsTransmmitOctets+rtStatsTemp.rtStatsTransmmitOctets;
	else
		rtStatsTemp.rtStatsTransmmitOctets = rtStatsTemp.rtStatsTransmmitOctets-ppreEntry->rtStatsTransmmitOctets;

	if( rtStatsTemp.rtStatsPkts < ppreEntry->rtStatsPkts )
		rtStatsTemp.rtStatsPkts = 0xffffffff-ppreEntry->rtStatsPkts+rtStatsTemp.rtStatsPkts;
	else
		rtStatsTemp.rtStatsPkts = rtStatsTemp.rtStatsPkts-ppreEntry->rtStatsPkts;		

	if( rtStatsTemp.rtStatsErrPkts < ppreEntry->rtStatsErrPkts )
		rtStatsTemp.rtStatsErrPkts = 0xffffffff-ppreEntry->rtStatsErrPkts+rtStatsTemp.rtStatsErrPkts;
	else
		rtStatsTemp.rtStatsErrPkts = rtStatsTemp.rtStatsErrPkts-ppreEntry->rtStatsErrPkts;

	if( rtStatsTemp.rtStatsTransmmitPkts < ppreEntry->rtStatsTransmmitPkts )
		rtStatsTemp.rtStatsTransmmitPkts = 0xffffffff-ppreEntry->rtStatsTransmmitPkts+rtStatsTemp.rtStatsTransmmitPkts;
	else
		rtStatsTemp.rtStatsTransmmitPkts = rtStatsTemp.rtStatsTransmmitPkts-ppreEntry->rtStatsTransmmitPkts;	

	if( rtStatsTemp.rtStatsBroadcastPkts < ppreEntry->rtStatsBroadcastPkts )
		rtStatsTemp.rtStatsBroadcastPkts = 0xffffffff-ppreEntry->rtStatsBroadcastPkts+rtStatsTemp.rtStatsBroadcastPkts;
	else
		rtStatsTemp.rtStatsBroadcastPkts = rtStatsTemp.rtStatsBroadcastPkts-ppreEntry->rtStatsBroadcastPkts;	

	if( rtStatsTemp.rtStatsMulticastPkts < ppreEntry->rtStatsMulticastPkts )
		rtStatsTemp.rtStatsMulticastPkts = 0xffffffff-ppreEntry->rtStatsMulticastPkts+rtStatsTemp.rtStatsMulticastPkts;
	else
		rtStatsTemp.rtStatsMulticastPkts = rtStatsTemp.rtStatsMulticastPkts -ppreEntry->rtStatsMulticastPkts;	

	if( rtStatsTemp.rtStatsCRCAlignErrors < ppreEntry->rtStatsCRCAlignErrors )
		rtStatsTemp.rtStatsCRCAlignErrors = 0xffffffff-ppreEntry->rtStatsCRCAlignErrors+rtStatsTemp.rtStatsCRCAlignErrors;
	else
		rtStatsTemp.rtStatsCRCAlignErrors = rtStatsTemp.rtStatsCRCAlignErrors -ppreEntry->rtStatsCRCAlignErrors;	

	if( rtStatsTemp.rtStatsUndersizePkts < ppreEntry->rtStatsUndersizePkts )
		rtStatsTemp.rtStatsUndersizePkts = 0xffffffff-ppreEntry->rtStatsUndersizePkts+rtStatsTemp.rtStatsUndersizePkts;
	else
		rtStatsTemp.rtStatsUndersizePkts = rtStatsTemp.rtStatsUndersizePkts -ppreEntry->rtStatsUndersizePkts;	

	if( rtStatsTemp.rtStatsOversizePkts < ppreEntry->rtStatsOversizePkts )
		rtStatsTemp.rtStatsOversizePkts = 0xffffffff-ppreEntry->rtStatsOversizePkts+rtStatsTemp.rtStatsOversizePkts;
	else
		rtStatsTemp.rtStatsOversizePkts = rtStatsTemp.rtStatsOversizePkts -ppreEntry->rtStatsOversizePkts;	

	if( rtStatsTemp.rtStatsCollisions < ppreEntry->rtStatsCollisions )
		rtStatsTemp.rtStatsCollisions = 0xffffffff-ppreEntry->rtStatsCollisions+rtStatsTemp.rtStatsCollisions;
	else
		rtStatsTemp.rtStatsCollisions = rtStatsTemp.rtStatsCollisions -ppreEntry->rtStatsCollisions;

	if( rtStatsTemp.rtStatsDropEvents < ppreEntry->rtStatsDropEvents )
		rtStatsTemp.rtStatsDropEvents = 0xffffffff-ppreEntry->rtStatsDropEvents+rtStatsTemp.rtStatsDropEvents;
	else
		rtStatsTemp.rtStatsDropEvents = rtStatsTemp.rtStatsDropEvents -ppreEntry->rtStatsDropEvents;
	if(rtStatsTemp.rtTxDropFrame<ppreEntry->rtTxDropFrame)
		rtStatsTemp.rtTxDropFrame=0xffffffff-ppreEntry->rtTxDropFrame+rtStatsTemp.rtTxDropFrame;
	else
		rtStatsTemp.rtTxDropFrame=rtStatsTemp.rtTxDropFrame-ppreEntry->rtTxDropFrame;

	if(rtStatsTemp.rtTxBroadcastFrame<ppreEntry->rtTxBroadcastFrame)
		rtStatsTemp.rtTxBroadcastFrame=0xffffffff-ppreEntry->rtTxBroadcastFrame+rtStatsTemp.rtTxBroadcastFrame;
	else
		rtStatsTemp.rtTxBroadcastFrame=rtStatsTemp.rtTxBroadcastFrame-ppreEntry->rtTxBroadcastFrame;
	if(rtStatsTemp.rtTxMulticastFrame<ppreEntry->rtTxMulticastFrame)
		rtStatsTemp.rtTxMulticastFrame=0xffffffff-ppreEntry->rtTxMulticastFrame+rtStatsTemp.rtTxMulticastFrame;
	else
		rtStatsTemp.rtTxMulticastFrame=rtStatsTemp.rtTxMulticastFrame-ppreEntry->rtTxMulticastFrame;

	if(rtStatsTemp.rtTxOctets<ppreEntry->rtTxOctets)
		rtStatsTemp.rtTxOctets=0xffffffff-ppreEntry->rtTxOctets+rtStatsTemp.rtTxOctets;
	else
		rtStatsTemp.rtTxOctets=rtStatsTemp.rtTxOctets-ppreEntry->rtTxOctets;
	
	if(rtStatsTemp.rtTxGrantPkts<ppreEntry->rtTxGrantPkts)
		rtStatsTemp.rtTxGrantPkts=0xffffffff-ppreEntry->rtTxGrantPkts+rtStatsTemp.rtTxGrantPkts;
	else
		rtStatsTemp.rtTxGrantPkts=rtStatsTemp.rtTxGrantPkts-ppreEntry->rtTxGrantPkts;

	if(rtStatsTemp.rtTxPausePkts<ppreEntry->rtTxPausePkts)
		rtStatsTemp.rtTxPausePkts=0xffffffff-ppreEntry->rtTxPausePkts+rtStatsTemp.rtTxPausePkts;
	else
		rtStatsTemp.rtTxPausePkts=rtStatsTemp.rtTxPausePkts-ppreEntry->rtTxPausePkts;

/*	{
		long long rtTxGrantOctets ;
		long *pl = (long*)&rtTxGrantOctets;

rtTxGrantOctets = rtStatsTemp.rtStatsTransmmitPkts;
sys_console_printf("\r\nrtStatsTransmmitPkts:%d\r\n", pl[1]);
rtTxGrantOctets = rtStatsTemp.rtStatsTransmmitOctets;
sys_console_printf("rtStatsTransmmitOctets:%d\r\n", pl[1]);
rtTxGrantOctets = rtStatsTemp.rtTxGrantPkts;
sys_console_printf("rtTxGrantPkts:%d\r\n", pl[1] );
		rtTxGrantOctets = rtStatsTemp.rtTxGrantPkts *64;
		if( rtStatsTemp.rtStatsTransmmitPkts > rtStatsTemp.rtTxGrantPkts )
			rtStatsTemp.rtStatsTransmmitPkts -= rtStatsTemp.rtTxGrantPkts;
		else
			rtStatsTemp.rtStatsTransmmitPkts = 0;
		if( rtStatsTemp.rtStatsTransmmitOctets > rtTxGrantOctets )
			rtStatsTemp.rtStatsTransmmitOctets -= rtTxGrantOctets;
		else
			rtStatsTemp.rtStatsTransmmitOctets = 0;
	}*/

	
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsOctets += rtStatsTemp.rtStatsOctets;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsErrOctets += rtStatsTemp.rtStatsErrOctets;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsTransmmitOctets += rtStatsTemp.rtStatsTransmmitOctets;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts += rtStatsTemp.rtStatsPkts;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsErrPkts += rtStatsTemp.rtStatsErrPkts;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsTransmmitPkts += rtStatsTemp.rtStatsTransmmitPkts;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsCRCAlignErrors += rtStatsTemp.rtStatsCRCAlignErrors;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsDropEvents += rtStatsTemp.rtStatsDropEvents;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsBroadcastPkts += rtStatsTemp.rtStatsBroadcastPkts;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsMulticastPkts += rtStatsTemp.rtStatsMulticastPkts;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsOversizePkts += rtStatsTemp.rtStatsOversizePkts;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsUndersizePkts += rtStatsTemp.rtStatsUndersizePkts;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsCollisions += rtStatsTemp.rtStatsCollisions;
	pgStatsRealTimeOnu[ponId][onuId]->rtStatsDropEvents += rtStatsTemp.rtStatsDropEvents;

	pgStatsRealTimeOnu[ponId][onuId]->rtTxDropFrame+=rtStatsTemp.rtTxDropFrame;
	pgStatsRealTimeOnu[ponId][onuId]->rtTxBroadcastFrame+=rtStatsTemp.rtTxBroadcastFrame;
	pgStatsRealTimeOnu[ponId][onuId]->rtTxOctets=pgStatsRealTimeOnu[ponId][onuId]->rtStatsTransmmitOctets;
	pgStatsRealTimeOnu[ponId][onuId]->rtTxPkts=pgStatsRealTimeOnu[ponId][onuId]->rtStatsTransmmitPkts;
	pgStatsRealTimeOnu[ponId][onuId]->rtTxMulticastFrame+=rtStatsTemp.rtTxMulticastFrame;
	pgStatsRealTimeOnu[ponId][onuId]->rtTxGrantPkts+=rtStatsTemp.rtTxGrantPkts;
	pgStatsRealTimeOnu[ponId][onuId]->rtTxPausePkts+=rtStatsTemp.rtTxPausePkts;
	pgStatsRealTimeOnu[ponId][onuId]->rtPausePkts+=rtStatsTemp.rtPausePkts;
	

	VOS_MemCpy( ppreEntry, &rtStatsTempBak, sizeof( rtStatsEntry ) );

		
		/*memcpy(((rtStatsEntry*)pgStatsRealTimeOlt[oltId]), &rtStatsTemp, sizeof(rtStatsEntry));*/
		calculatePonBer( &rtStatsTemp  );
		calculatePonFer( &rtStatsTemp  );
		calculateRtBandWidth( &rtStatsTemp , ONU_PON_PORT );

	pgStatsRealTimeOnu[ponId][onuId]->rtUpBandWidth=rtStatsTemp.rtUpBandWidth;
	pgStatsRealTimeOnu[ponId][onuId]->rtDownBandWidth=rtStatsTemp.rtDownBandWidth;

		
/*		memcpy(pgStatsRealTimeOnu[ponId][onuId], &rtStatsTemp, sizeof(rtStatsEntry));*/
		if(stat_result_show == 1)
		{
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsDropEvents = %lu \r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsDropEvents);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsOctets = %lu \r\n",(unsigned long)pgStatsRealTimeOnu[ponId][onuId]->rtStatsOctets);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsOctets = %.0lf \r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsOctets);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsBroadcastPkts = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsBroadcastPkts);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsMulticastPkts = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsMulticastPkts);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsCRCAlignErrors = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsCRCAlignErrors);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsUndersizePkts = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsUndersizePkts);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsOversizePkts = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsOversizePkts);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsFragments = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsFragments);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsJabbers = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsJabbers);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsCollisions = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsCollisions);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts64Octets = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts64Octets);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts65to127Octets = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts65to127Octets);
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts128to255Octets = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts128to255Octets);		
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts256to511Octets = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts256to511Octets);	
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts512to1023Octets = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts512to1023Octets);	
			sys_console_printf(" pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts1024to1518Octets = %lu\r\n",pgStatsRealTimeOnu[ponId][onuId]->rtStatsPkts1024to1518Octets);	
			
		}
	return STATS_OK;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
 /***********************************************
 * StatsRealTimeGet
 *	描述: 该函数用于对目标端口进行实时采样
 *
 *
 *
 *
 *********************************************/
 STATUS StatsRealTimeDataGet(void)
 {

 	short int oltId = 0;
#ifdef __RT_ONU_RAW	/* modified by xieshl 20090204, 暂时去掉ONU统计信息 */
	short int onuId = 0;
#endif
	int port = 0;
	
	for(oltId = 0; oltId < STATS_HIS_MAXPON; oltId++)
	{
		if(STATS_DEVICE_ON == gStatsPonOn[oltId])
		{
			StatsRealTimePonRawGet(oltId);
		}
#ifdef __RT_ONU_RAW	/* modified by xieshl 20090204, 暂时去掉ONU统计信息 */
		for(onuId = 0; onuId< STATS_HIS_MAXONU; onuId++)
		{
			if(STATS_DEVICE_ON == gStatsOnuOn[oltId][onuId])
				StatsRealTimeOnuRawGet(oltId, onuId);
		}
#endif
	}

	for( port =1; port<=4; port++ )
		StatsRealTimeEthRawGet( 1, port );

	return STATS_OK;
 }
#endif
#if 0
int __test_pon_stat_flag = 3;
int __test_pon_stat_count = 1000;
int __test_pon_stat_debug = 0;
 void __test_pon_stat(ulong arg0, ulong arg1,ulong arg2,ulong arg3,ulong arg4,ulong arg5,ulong arg6,ulong arg7,ulong arg8,ulong arg9)
 {
 	short int oltId;
	short int onuId;
	ulong totalcount=0;
	ulong count=0;
	PON_total_frames_raw_data_t	totalFrameRawData_5201;
	unsigned long	timestamp = 0 ;
	int ret, ret1;

	sys_console_printf("pon%d/%d, onu%d\r\n", arg0, arg1,arg2 );

 	oltId = GetPonPortIdxBySlot( arg0, arg1 );
	if( oltId == VOS_ERROR )
	{
		return;
	}
	if( arg2 != 0 )
	{
		onuId = GetLlidByOnuIdx( oltId, arg2-1 );
		if( onuId == VOS_ERROR )
		{
			return;
		}
	}
	else
		onuId = 1;
	
	while( __test_pon_stat_flag )
	{
		count++;

		if( __test_pon_stat_flag == 1 )
		{
			VOS_MemZero( &totalFrameRawData_5201, sizeof(totalFrameRawData_5201) );
			ret = PAS_get_raw_statistics ( oltId,  -1, PON_RAW_STAT_TOTAL_FRAMES, 1,
							&totalFrameRawData_5201, &timestamp );	
			if(  __test_pon_stat_debug )
				sys_console_printf("PAS_get_raw_statistics pon%d/%d OLT ret=%d, count=%d\r\n", arg0, arg1, ret, count);
		}
		else if( __test_pon_stat_flag == 2 )
		{
			VOS_MemZero( &totalFrameRawData_5201, sizeof(totalFrameRawData_5201) );
			ret1 = PAS_get_raw_statistics ( oltId, onuId, PON_RAW_STAT_TOTAL_FRAMES, 1,
							&totalFrameRawData_5201, &timestamp );
			if( __test_pon_stat_debug )
				sys_console_printf("PAS_get_raw_statistics pon%d/%d ONU ret=%d, count=%d\r\n", arg0, arg1, ret1, count);
		}
		else if( __test_pon_stat_flag == 3 )
		{
			VOS_MemZero( &totalFrameRawData_5201, sizeof(totalFrameRawData_5201) );
			ret = PAS_get_raw_statistics ( oltId,  -1, PON_RAW_STAT_TOTAL_FRAMES, 1,
							&totalFrameRawData_5201, &timestamp );	
			VOS_MemZero( &totalFrameRawData_5201, sizeof(totalFrameRawData_5201) );
			ret1 = PAS_get_raw_statistics ( oltId, onuId, PON_RAW_STAT_TOTAL_FRAMES, 1,
							&totalFrameRawData_5201, &timestamp );	

			if( __test_pon_stat_debug )
				sys_console_printf("PAS_get_raw_statistics pon%d/%d OLT ret=%d, ONU ret=%d, count=%d\r\n", arg0, arg1, ret, ret1, count);
		}
		else if( __test_pon_stat_flag == 4 )
		{
			ret = StatsRealTimeDataGet();
			if(arg0 == 5)
				ponPortBERAlarm_EventReport(54001,1,1,100);
		}
		else
		{
			sys_console_printf("test pon%d/%d delay %d\r\n", arg0, arg1, count);
			VOS_TaskDelay(200);
		}

		if( count % __test_pon_stat_count == 0 )
		{
			sys_console_printf("test pon%d/%d delay %d\r\n", arg0, arg1, ++totalcount);
			VOS_TaskDelay(20);
			/*count = 0;*/
		}
		VOS_TaskDelay(0);
	}
 }
void __test_pon_stat_init(int slot, int port, int onu, int pri)
{
	ulong arg[10]={8,2,1,0,0,0,0,0,0,0};
	arg[0]=slot;
	arg[1]=port;
	arg[2]=onu;
	VOS_TaskCreate("tttt", pri, __test_pon_stat, arg );
}
#endif
 
 /***********************************************
 * rtStatsDataClear
 *	描述: 该函数用于对目标端口进行实时采样
 *
 *
 *
 *
 *********************************************/
 STATUS StatsDataClear(void)
 {
 	historyStatsMsg *pstatsRecMsg; 
	long		lRes = 0;
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};	
	pstatsRecMsg = (historyStatsMsg *)VOS_Malloc(sizeof(historyStatsMsg), MODULE_STATSTICS);
	if (NULL == pstatsRecMsg)
		return STATS_ERR;
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));
	pstatsRecMsg->statsType = STATS_RTDATA_CLEAR;
	aulMsg[3] = (ULONG)(pstatsRecMsg);
	/*gRtStatisticClearStatus = STATS_CLEAR_START;*/
	lRes = VOS_QueSend(gMQidsStats, aulMsg, NO_WAIT,MSG_PRI_NORMAL);

	if (VOS_ERROR == lRes)
	{
		ASSERT( 0 );
		VOS_Free((VOID *) pstatsRecMsg);
		pstatsRecMsg = NULL;
		return lRes;
	}
	return lRes;	
 }

/**************************************************
* StatsMsgPonOnSend
*          描述: 当olt 掉线或者上线时,调用该函数向实时采样任务发
*		消息通知
*
*	参数: OnStatus -1 表示olt 在线;0 - 表示olt掉线
*
****************************/
STATUS StatsMsgPonOnSend(unsigned short oltId, unsigned char OnStatus)
{
	historyStatsMsg *pstatsRecMsg; 
	long		lRes = 0;
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};

	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )	/* modified by xieshl 20101231, pon板暂不支持统计 */
		return lRes;
	
	/* modify be chenfj 2006/09/26 */
	/* #2678 问题一定配置下，给一个trunk加入成员端口，系统出现异常*/
	pstatsRecMsg = (historyStatsMsg *)VOS_Malloc(sizeof(historyStatsMsg), MODULE_STATSTICS);
	if (NULL == pstatsRecMsg)
		return STATS_ERR;
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));	
	pstatsRecMsg->ponId = oltId;
	pstatsRecMsg->statsType = STATS_OLT_STATUS;	
	pstatsRecMsg->userd = OnStatus;
	aulMsg[3] = (ULONG)(pstatsRecMsg);
	/*sys_console_printf(" sendTimerOut!\r\n");*/
	lRes = VOS_QueSend(gMQidsStats, aulMsg, NO_WAIT,MSG_PRI_NORMAL);

	if (VOS_ERROR == lRes)
	{
		ASSERT( 0 );
		VOS_Free((VOID *) pstatsRecMsg);
		pstatsRecMsg = NULL;
		return lRes;
	}
	return lRes;
}


/**************************************************
* StatsMsgOnuOnSend
*          描述: 当olt 掉线或者上线时,调用该函数向实时采样任务发
*		消息通知
*
*	参数: OnStatus -1 表示olt 在线;0 - 表示olt掉线
*
****************************/
STATUS StatsMsgOnuOnSend(unsigned short oltId, unsigned short onuId, unsigned char OnStatus)
{
	historyStatsMsg *pstatsRecMsg; 
	long		lRes = 0;
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};
	return VOS_OK; /* modified by xieshl 20111111*/
	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )	/* modified by xieshl 20101231, pon板暂不支持统计 */
		return lRes;

	pstatsRecMsg = (historyStatsMsg *)VOS_Malloc(sizeof(historyStatsMsg), MODULE_STATSTICS);
	if ( NULL == pstatsRecMsg )
		return STATS_ERR;
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));	
	pstatsRecMsg->ponId = oltId;
	pstatsRecMsg->onuId = onuId;
	pstatsRecMsg->statsType = STATS_ONU_STATUS;	
	pstatsRecMsg->userd = OnStatus;
	aulMsg[3] = (ULONG)(pstatsRecMsg);
	lRes = VOS_QueSend(gMQidsStats, aulMsg, NO_WAIT,MSG_PRI_NORMAL);

	if (VOS_ERROR == lRes)
	{
		ASSERT( 0 );
		VOS_Free((VOID *) pstatsRecMsg);
		pstatsRecMsg = NULL;
		return lRes;
	}
	return lRes;
	
}

/**************************************************
* loopStatsMsgOnuOnSend
*          描述: 当进行onu环回测试时,调用该函数向实时采样任务发
*		消息通知,暂时停止对该onu和olt进行采样.
*
*	参数: OnStatus -1 表示olt 在线;0 - 表示olt掉线
*	被函数loopStatsMsgOnuOn()和loopStatsMsgOnuOff()调用
****************************/
STATUS loopStatsMsgSend(unsigned short oltId, unsigned short onuId, unsigned char OnStatus)
{
	historyStatsMsg *pstatsRecMsg; 
	long		lRes = 0;
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};

	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )	/* modified by xieshl 20101231, pon板暂不支持统计 */
		return lRes;

	pstatsRecMsg = (historyStatsMsg *)VOS_Malloc(sizeof(historyStatsMsg), MODULE_STATSTICS);
	if (NULL == pstatsRecMsg)
		return STATS_ERR;
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));	
	pstatsRecMsg->ponId = oltId;
	pstatsRecMsg->onuId = onuId;
	pstatsRecMsg->statsType = STATS_LOOP_ONU_PRO;	
	pstatsRecMsg->userd = OnStatus;
	aulMsg[3] = (ULONG)(pstatsRecMsg);
	lRes = VOS_QueSend(gMQidsStats, aulMsg, NO_WAIT,MSG_PRI_NORMAL);

	if (VOS_ERROR == lRes)
	{
		ASSERT( 0 );
		VOS_Free((VOID *) pstatsRecMsg);
		pstatsRecMsg = NULL;
		return lRes;
	}
	return lRes;
	
}
/**************************************************
* loopStatsMsgOnuOnSend
*          描述: 当进行onu环回测试时,调用该函数向实时采样任务发
*		消息通知,重新开始对该onu和olt进行采样.
*
*	参数: OnStatus -1 表示olt 在线;0 - 表示olt掉线
*
****************************/
STATUS loopStatsMsgOnuOn(unsigned short oltId, unsigned short onuId)
{
	loopStatsMsgSend(oltId, onuId, STATS_DEVICE_ON);
	return STATS_OK;
}
/**************************************************
* loopStatsMsgOnuOffSend
*          描述: 当进行onu环回测试时,调用该函数向实时采样任务发
*		消息通知,暂时停止对该onu和olt进行采样.
*
*	参数: OnStatus -1 表示olt 在线;0 - 表示olt掉线
*
****************************/
STATUS loopStatsMsgOnuOff(unsigned short oltId, unsigned short onuId)
{
	loopStatsMsgSend(oltId, onuId, STATS_DEVICE_OFF);
	return STATS_OK;
}


#if 0
 /**************************************************
* HisStatsMsgTimeOutSend
*
*
*
*
*
****************************/
short int StatsMsgRealTimeSend(void)
{
	historyStatsMsg *pstatsRecMsg; 
	long		lRes = 0;
	ULONG	aulMsg[4] = {MODULE_STATSTICS,0,0,0};

	
	if(FALSE == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
	{
		/*reports( "StatsMsgRealTimeSend:", "sys module is not running" );*/
		return STATS_ERR;	
	}

	if( VOS_QueNum( gMQidsStats) > 2 )
	{
		/*reports( "StatsMsgRealTimeSend", "gMQidsStats is busy" );*/
		return STATS_ERR;
	}
	
	pstatsRecMsg = (historyStatsMsg *)VOS_Malloc(sizeof(historyStatsMsg), MODULE_STATSTICS);
	if (NULL == pstatsRecMsg)
	{
		ASSERT( 0 );
		return STATS_ERR;
	}
	memset(pstatsRecMsg, 0, sizeof(historyStatsMsg));	

	
	pstatsRecMsg->statsType = STATS_REALTIME_TIMEOUT;	
	aulMsg[3] = (ULONG)(pstatsRecMsg);
	lRes = VOS_QueSend(gMQidsStats, aulMsg, NO_WAIT, MSG_PRI_URGENT );
	if (VOS_ERROR == lRes)
	{
		ASSERT( 0 );
		VOS_Free((VOID *) pstatsRecMsg);
		pstatsRecMsg = NULL;
		
#ifdef __STAT_DEBUG
		if( gStatisticDebugFlag == 1 )		
			sys_console_printf("VOS_Quesend ERROR!\r\n");
#endif
		return STATS_ERR;
	}
	
 	return STATS_OK;
	
}
#endif

/******************************************************
* rtStatsClearStatusGet
* DES: 获取清除实时统计 数据时的清除状态
*
*
*
*
*
********************************************************/
/*STATUS mibRtStatsClearStatusGet(unsigned int *pStatus)
{
	if (NULL == pStatus)
		return STATS_ERR;
	*pStatus = gRtStatisticClearStatus;
	return STATS_OK;
}*/

#if 0
STATUS mibRtStatsEthernetCheck( ulong_t slot, ulong_t port, ulong_t onuId)
{
	/*1.判断该主控卡是否为主用主控卡，如果否，则
	返回错误；
	如果正确，则进行下一步*/

	if ((0 == gStatsEthernetOn[port]) && (onuId > STATS_HIS_MAXONU))
		return STATS_ERR;
	return STATS_OK;	
}
#endif


STATUS mibRtStatsOltCheck(	ulong_t        slot,
							ulong_t        port,
							ulong_t        onuId)
{
	short int oltid = 0;
	oltid = GetPonPortIdxBySlot(slot, port );

	if ((oltid>=MAXPON) || (oltid<0))
		return STATS_ERR;	
	
	if (STATS_DEVICE_OFF == gStatsPonOn[oltid])
		return STATS_ERR;

	return STATS_OK;	
}


STATUS mibRtStatsOnuCheck(	ulong_t        slot,
							ulong_t        port,
							ulong_t        onuId)
{
	short oltid = 0;
	oltid = GetPonPortIdxBySlot(slot, port );
	if( oltid == VOS_ERROR )	/* modified by xieshl 20080812 */
		return STATS_ERR;
	if ((0 == gStatsOnuOn[oltid][onuId]) || (onuId > (STATS_HIS_MAXONU-1)))
		return STATS_ERR;
	return STATS_OK;
}




/*注意: 此处一定要搞清出oltid 是从0 还是从1开始*/
STATUS rtStatsOltDataFirstIndex(unsigned short *pOltId)
{
	unsigned short oltid = 0;
	while((!gStatsPonOn[oltid]) && (oltid<(STATS_HIS_MAXPON-1)))
		oltid++;
	if ((!gStatsPonOn[oltid]) && (oltid<STATS_HIS_MAXPON))
		*pOltId = oltid;
	else
		return STATS_ERR;
	return STATS_OK;
}



STATUS rtStatsOnuDataFirstIndex(unsigned short *pOltId, unsigned short *pOnuId)
{
	unsigned short oltid = 0;
	unsigned short onuid = 0;

	while( oltid<STATS_HIS_MAXPON )
		{
		while ((!gStatsOnuOn[oltid][onuid]) && (onuid < STATS_HIS_MAXONU))
			{
				onuid++;
				if(onuid>=STATS_HIS_MAXONU)
					break;
			}
		if ((onuid < STATS_HIS_MAXONU))
			{
				/*sys_console_printf(" oltId %d  onuId %d",oltid, onuid);*/
				*pOltId = oltid;
				*pOnuId = onuid;
				break;
			}
		onuid = 0;
		oltid++;
		if (oltid>=STATS_HIS_MAXPON)
			break;
		}
	
	if ((oltid<STATS_HIS_MAXPON) && (onuid < STATS_HIS_MAXONU))
		return STATS_OK;
	else
		return STATS_ERR;

}

STATUS rtStatsOltDataNextIndexGet(unsigned short oltid, unsigned short *pNextOltId)
{
	unsigned short tempId =(oltid+1);
	while((!gStatsPonOn[tempId]) && (tempId<STATS_HIS_MAXPON))
	{
		tempId++;
		if (tempId >= STATS_HIS_MAXPON)
		break;
	}
	if (tempId<STATS_HIS_MAXPON)
		*pNextOltId = tempId;
	else
		return STATS_ERR;
	return STATS_OK;
}

STATUS rtStatsOnuDataNextIndexGet(unsigned short oltid, unsigned short onuid, unsigned short *pNextOltId, unsigned short *pNextOnuId)
{
	unsigned short tempOltId = oltid;
	unsigned short tempOnuId = (onuid+1);
	if (tempOnuId>= STATS_HIS_MAXONU)
		{
			tempOltId += 1;
			tempOnuId = 0;
		}
	if (tempOltId >= STATS_HIS_MAXPON)
		return STATS_ERR;
	
	while( tempOltId<STATS_HIS_MAXPON)
		{
		while ((!gStatsOnuOn[tempOltId][tempOnuId]) && (tempOnuId < STATS_HIS_MAXONU))
			{
				tempOnuId++;
				if (tempOnuId >= STATS_HIS_MAXONU)
					break;
			}
		
		if ((tempOnuId < STATS_HIS_MAXONU))
			{
				*pNextOltId = tempOltId;
				*pNextOnuId = tempOnuId;
				break;
			}
		tempOnuId = 0;
		tempOltId++;
		if (tempOltId >= STATS_HIS_MAXPON)
			break;
		}
	
	if ((tempOltId<STATS_HIS_MAXPON) && (tempOnuId < STATS_HIS_MAXONU))
		return STATS_OK;
	else
		return STATS_ERR;

}

#if 0
STATUS mibRtStatsEthernetDataGet(short int StatsObjType,     
							ulong_t        slot,
							ulong_t        port,
							unsigned long long *pStatsData)
{
	rtStatsEntry64 *pTempRtStats = NULL;
	#if 0
	if (broadType == MODULE_E_EPON3_PON)
		{
			/*1.判断该主控卡是否为主用主控卡，如果否，则
			返回错误；
			如果正确，则进行下一步*/

			if (port > STATS_RT_MAXEHTER)
				return STATS_ERR;
			pTempRtStats = pgStatsRealTimeEth[STATS_RT_MASTERBRD][port];
		}
	else if (broadType == MODULE_E_EPON3_SW)
		{
			oltid = GetPonPortIdxBySlot(slot, port );
			if ((oltid<0) || (oltid>19))
				return STATS_ERR;	
			pTempRtStats = (rtStatsEntry *)pgStatsRealTimeOlt[oltid];
		}
	#endif
	
	pTempRtStats = &gStatsRealTimeEth[port];
	if (pTempRtStats == NULL)
		return STATS_ERR;
	
	switch(StatsObjType)
		{
		case STATS_DROPEVENTS_TYPE:
			*pStatsData = pTempRtStats->rtStatsDropEvents;
			break;
		case STATS_OCTETS_TYPE:
			*pStatsData = pTempRtStats->rtStatsOctets;
			break;
		case STATS_PKTS_TYPE:
			*pStatsData = pTempRtStats->rtStatsPkts;
			break;
		case STATS_BROADCASTPKTS_TYPE:
			*pStatsData =pTempRtStats->rtStatsBroadcastPkts;
			break;
		case STATS_MULTICASTPKTS_TYPE:
			*pStatsData = pTempRtStats->rtStatsMulticastPkts;
			break;
		case STATS_CRCALIGNERR_TYPE:
			*pStatsData = pTempRtStats->rtStatsCRCAlignErrors;
			break;
		case STATS_UNDERSIZEPKTS_TYPE:
			*pStatsData = pTempRtStats->rtStatsUndersizePkts;
			break;
		case STATS_OVERSIZEPKTS_TYPE:
			*pStatsData = pTempRtStats->rtStatsOversizePkts;
			break;
		case STATS_FRAGMENTS_TYPE:
			*pStatsData = pTempRtStats->rtStatsFragments;
			break;
		case STATS_JABBERS_TYPE:
			*pStatsData = pTempRtStats->rtStatsJabbers;
			break;
		case STATS_COLLISIONS_TYPE:
			*pStatsData = pTempRtStats->rtStatsCollisions;
			break;		
		case STATS_PKT64OCTETS: 	/*12*/
			*pStatsData = pTempRtStats->rtStatsPkts64Octets;
			break;
		case STATS_PKT65TO127OCTETS	: /*	13*/
			*pStatsData = pTempRtStats->rtStatsPkts65to127Octets;
			break;
		case STATS_PKT128TO255OCTETS : /*	14*/
			*pStatsData = pTempRtStats->rtStatsPkts128to255Octets;
			break;
		case  STATS_PKT256TO511OCTETS:/*	15*/
			*pStatsData = pTempRtStats->rtStatsPkts256to511Octets;
			break;
		case  STATS_PKT512TO1023OCTETS:/*	16*/
			*pStatsData = pTempRtStats->rtStatsPkts512to1023Octets;
			break;
		case  STATS_PKT1024TO1518OCTETS:/*	17*/
			*pStatsData = pTempRtStats->rtStatsPkts1024to1518Octets;
			break;
		case STATS_TXDROPPKTS:
			*pStatsData=pTempRtStats->rtTxDropFrame;
			break;
		case STATS_TXOCTETS:
			*pStatsData=pTempRtStats->rtTxOctets;
			break;
		case STATS_TXBROADCASTPKTS:
			*pStatsData=pTempRtStats->rtTxBroadcastFrame;
			break;
		case STATS_TXPKTS:
			*pStatsData=pTempRtStats->rtTxPkts;
			break;
		case STATS_TXMULTICASTPKTS:
			*pStatsData=pTempRtStats->rtTxMulticastFrame;
			break;
		case STATS_TXGRANT:
			*pStatsData=pTempRtStats->rtTxGrantPkts;
			break;
		case STATS_TXPAUSEPKTS:
			*pStatsData=pTempRtStats->rtTxPausePkts;
			break;
		case STATS_PAUSEPKTS:
			*pStatsData=pTempRtStats->rtPausePkts;
			break;
		case STATS_UPBANDWIDTH:
			*pStatsData=((pTempRtStats->rtUpBandWidth)/1000);
			break;
		case STATS_DOWNBANDWIDTH:
			*pStatsData=((pTempRtStats->rtDownBandWidth)/1000);
			break;
		default:
			return (STATS_INVALUE_OBJ_ERR);
		}

	return STATS_OK;
}

STATUS mibRtStatsOltDataGet(short int StatsObjType,     
							ulong_t        slot,
							ulong_t        port,
							/*ulong_t        onuId, */
							unsigned long long*pStatsData)
{
	rtStatsEntry64 *pTempRtStats = NULL;
	short int oltid = 0;
	/*unsigned long *pRtPausePkts = NULL;*/

	if ( 0 == slot )
	{
		/* 增加上联板的支持 */
		pTempRtStats = (rtStatsEntry64 *)&gStatsRealTimeEth[port];
		/*sys_console_printf("mibRtStatsOltDataGet():: port=%d\r\n", port);*/
	}
	else
	{
		oltid = GetPonPortIdxBySlot(slot, port );
		if ((oltid<0) || (oltid>19))
			return STATS_ERR;

		if( !gStatsPonOn[oltid] )
		{
			*pStatsData = 0;
			return STATS_OK;
		}
	
		pTempRtStats = (rtStatsEntry64 *)pgStatsRealTimeOlt[oltid];
	}
	
	if (pTempRtStats == NULL)
	{
		/*sys_console_printf("mibRtStatsOltDataGet()::return Error!\r\n");*/
		VOS_ASSERT(0);
		return STATS_ERR;
	}

	switch(StatsObjType)
		{
		case STATS_DROPEVENTS_TYPE:
			*pStatsData = pTempRtStats->rtStatsDropEvents;
			break;
		case STATS_OCTETS_TYPE:
			*pStatsData = pTempRtStats->rtStatsOctets;
			break;
		case STATS_PKTS_TYPE:
			*pStatsData = pTempRtStats->rtStatsPkts;
			break;
		case STATS_BROADCASTPKTS_TYPE:
			*pStatsData =pTempRtStats->rtStatsBroadcastPkts;
			break;
		case STATS_MULTICASTPKTS_TYPE:
			*pStatsData = pTempRtStats->rtStatsMulticastPkts;
			break;
		case STATS_CRCALIGNERR_TYPE:
			*pStatsData = pTempRtStats->rtStatsCRCAlignErrors;
			break;
		case STATS_UNDERSIZEPKTS_TYPE:
			*pStatsData = pTempRtStats->rtStatsUndersizePkts;
			break;
		case STATS_OVERSIZEPKTS_TYPE:
			*pStatsData = pTempRtStats->rtStatsOversizePkts;
			break;
		case STATS_FRAGMENTS_TYPE:
			*pStatsData = pTempRtStats->rtStatsFragments;
			break;
		case STATS_JABBERS_TYPE:
			*pStatsData = pTempRtStats->rtStatsJabbers;
			break;
		case STATS_COLLISIONS_TYPE:
			*pStatsData = pTempRtStats->rtStatsCollisions;
			break;		
		case STATS_PKT64OCTETS: 	/*12*/
			*pStatsData = pTempRtStats->rtStatsPkts64Octets;
			break;
		case STATS_PKT65TO127OCTETS	: /*	13*/
			*pStatsData = pTempRtStats->rtStatsPkts65to127Octets;
			break;
		case STATS_PKT128TO255OCTETS : /*	14*/
			*pStatsData = pTempRtStats->rtStatsPkts128to255Octets;
			break;
		case  STATS_PKT256TO511OCTETS:/*	15*/
			*pStatsData = pTempRtStats->rtStatsPkts256to511Octets;
			break;
		case  STATS_PKT512TO1023OCTETS:/*	16*/
			*pStatsData = pTempRtStats->rtStatsPkts512to1023Octets;
			break;
		case  STATS_PKT1024TO1518OCTETS:/*	17*/
			*pStatsData = pTempRtStats->rtStatsPkts1024to1518Octets;
			break;
		case STATS_TXDROPPKTS:
			*pStatsData=pTempRtStats->rtTxDropFrame;
			break;
		case STATS_TXOCTETS:
			*pStatsData=pTempRtStats->rtStatsTransmmitOctets;
			break;
		case STATS_TXPKTS:
			*pStatsData=pTempRtStats->rtStatsTransmmitPkts;
			break;
		case STATS_TXBROADCASTPKTS:
			*pStatsData=pTempRtStats->rtTxBroadcastFrame;
			break;
		case STATS_TXMULTICASTPKTS:
			*pStatsData=pTempRtStats->rtTxMulticastFrame;
		case STATS_TXGRANT:
			*pStatsData=pTempRtStats->rtTxGrantPkts;
			break;
		case STATS_TXPAUSEPKTS:
			*pStatsData=pTempRtStats->rtTxPausePkts;
			break;
		case STATS_PAUSEPKTS:
			*pStatsData=pTempRtStats->rtPausePkts;
			/*pRtPausePkts = (unsigned long *)&pTempRtStats->rtPausePkts;
			sys_console_printf("mibRtStatsOltDataGet()::rtPausePkts+1=%d  rtPausePkts-1=%d\r\n", *(pRtPausePkts + 1), *(pRtPausePkts - 1));*/
			break;
		case STATS_UPBANDWIDTH:
			*pStatsData=((pTempRtStats->rtUpBandWidth)/1000);
			break;
		case STATS_DOWNBANDWIDTH:
			*pStatsData=((pTempRtStats->rtDownBandWidth)/1000);
			break;
		default:
			return (STATS_INVALUE_OBJ_ERR);
		}

	/*sys_console_printf("mibRtStatsOltDataGet()::return OK!\r\n");*/
	return STATS_OK;
}

STATUS mibRtStatsOnuDataGet(short int StatsObjType, 
							ulong_t        slot,
							ulong_t        port,
							ulong_t        onuId,  
							unsigned long long *pStatsData)
{
	rtStatsEntry64 *pTempStats = NULL;
	short int oltId = 0;
	
	oltId = GetPonPortIdxBySlot(slot, port );
	if ((oltId<0) || (oltId>19))
		return STATS_ERR;

	if( !gStatsOnuOn[oltId][onuId] )
	{
		*pStatsData = 0;
		return STATS_OK ;
	}
	
	pTempStats = (rtStatsEntry64*)pgStatsRealTimeOnu[oltId][onuId];	
	
	switch(StatsObjType)
		{
		case STATS_DROPEVENTS_TYPE:
			*pStatsData = pTempStats->rtStatsDropEvents;
			break;
		case STATS_OCTETS_TYPE:
			*pStatsData = pTempStats->rtStatsOctets;
			break;
		case STATS_PKTS_TYPE:
			*pStatsData = pTempStats->rtStatsPkts;
			break;
		case STATS_BROADCASTPKTS_TYPE:
			*pStatsData =pTempStats->rtStatsBroadcastPkts;
			break;
		case STATS_MULTICASTPKTS_TYPE:
			*pStatsData = pTempStats->rtStatsMulticastPkts;
			break;
		case STATS_CRCALIGNERR_TYPE:
			*pStatsData = pTempStats->rtStatsCRCAlignErrors;
			break;
		case STATS_UNDERSIZEPKTS_TYPE:
			*pStatsData = pTempStats->rtStatsUndersizePkts;
			break;
		case STATS_OVERSIZEPKTS_TYPE:
			*pStatsData = pTempStats->rtStatsOversizePkts;
			break;
		case STATS_FRAGMENTS_TYPE:
			*pStatsData = pTempStats->rtStatsFragments;
			break;
		case STATS_JABBERS_TYPE:
			*pStatsData = pTempStats->rtStatsJabbers;
			break;
		case STATS_COLLISIONS_TYPE:
			*pStatsData = pTempStats->rtStatsCollisions;
			break;		
		case STATS_PKT64OCTETS: 	/*12*/
			*pStatsData = pTempStats->rtStatsPkts64Octets;
			break;
		case STATS_PKT65TO127OCTETS	: /*	13*/
			*pStatsData = pTempStats->rtStatsPkts65to127Octets;
			break;
		case STATS_PKT128TO255OCTETS : /*	14*/
			*pStatsData = pTempStats->rtStatsPkts128to255Octets;
			break;
		case  STATS_PKT256TO511OCTETS:/*	15*/
			*pStatsData = pTempStats->rtStatsPkts256to511Octets;
			break;
		case  STATS_PKT512TO1023OCTETS:/*	16*/
			*pStatsData =pTempStats->rtStatsPkts512to1023Octets;
			break;
		case  STATS_PKT1024TO1518OCTETS:/*	17*/
			*pStatsData = pTempStats->rtStatsPkts1024to1518Octets;
			break;
		/*用于环回测试的数据获取*/
		case  STATS_ONULOOPBACK_RXOK:/*	18*/
			*pStatsData = pTempStats->rtStatsRxOk;
			break;	
		case  STATS_ONULOOPBACK_TXOK:/*	19*/
			*pStatsData = pTempStats->rtStatsTxOk;
			break;	
		case  STATS_ONULOOPBACK_RXERR:/*	20*/
			*pStatsData = pTempStats->rtStatsRxErr;
			break;	
		case  STATS_ONULOOPBACK_TOTALRXBAD:/*	21*/
			*pStatsData = pTempStats->rtStatsRxTotalBad;
			break;
		case STATS_TXDROPPKTS:
			*pStatsData=pTempStats->rtTxDropFrame;
			break;
		case STATS_TXOCTETS:
			*pStatsData=pTempStats->rtTxOctets;
			break;
		case STATS_TXPKTS:
			*pStatsData=pTempStats->rtTxPkts;
			break;
		case STATS_TXBROADCASTPKTS:
			*pStatsData=pTempStats->rtTxBroadcastFrame;
			break;
		case STATS_TXMULTICASTPKTS:
			*pStatsData=pTempStats->rtTxMulticastFrame;
			break;
		case STATS_TXPAUSEPKTS:
			*pStatsData=pTempStats->rtTxPausePkts;
			break;
		case STATS_TXGRANT:
			*pStatsData=pTempStats->rtTxGrantPkts;
			break;
		case STATS_PAUSEPKTS:
			*pStatsData=pTempStats->rtPausePkts;
			break;
		case STATS_UPBANDWIDTH:
			*pStatsData=((pTempStats->rtUpBandWidth)/1000);
			break;
		case STATS_DOWNBANDWIDTH:
			*pStatsData=((pTempStats->rtDownBandWidth)/1000);
			break;
		default:
			return (STATS_INVALUE_OBJ_ERR);
		}
	return STATS_OK;
}
#endif

STATUS rtStatsOltDataGet(short int StatsObjType,     
							ETH_PORT_STATS_S *pEthInterfaceStats, 
							unsigned long long *pStatsData)
{
	if( (NULL == pEthInterfaceStats) || (NULL == pStatsData) )
		return VOS_ERROR;
	
	switch(StatsObjType)
	{
		case STATS_DROPEVENTS_TYPE:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsDropEvents;
			break;
		case STATS_OCTETS_TYPE:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllIfHCInOctets;/*ulAllIfInOctets;*/
			break;
		case STATS_PKTS_TYPE:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllPacketSumRx;
			break;
		case STATS_BROADCASTPKTS_TYPE:
			*pStatsData =*(unsigned long long*)&pEthInterfaceStats->ulAllIfHCInBroadcastPkts;/*ulAllEtherStatsBroadcastPkts;*/	/* modified by xieshl 20110321, 问题单12259 */
			break;
		case STATS_MULTICASTPKTS_TYPE:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllIfHCInMulticastPkts;/*ulAllEtherStatsMulticastPkts;*/
			break;
		case STATS_CRCALIGNERR_TYPE:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsCRCAlignErrors;
			break;
		case STATS_UNDERSIZEPKTS_TYPE:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsUndersizePkts;
			break;
		case STATS_OVERSIZEPKTS_TYPE:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsOversizePkts;
			break;
		case STATS_FRAGMENTS_TYPE:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsFragments;
			break;
		case STATS_JABBERS_TYPE:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsJabbers;
			break;
		case STATS_COLLISIONS_TYPE:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsCollisions;
			break;		
		case STATS_PKT64OCTETS:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsPkts64Octets;
			break;
		case STATS_PKT65TO127OCTETS	:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsPkts65to127Octets;
			break;
		case STATS_PKT128TO255OCTETS :
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsPkts128to255Octets;
			break;
		case  STATS_PKT256TO511OCTETS:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsPkts256to511Octets;
			break;
		case  STATS_PKT512TO1023OCTETS:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsPkts512to1023Octets;
			break;
		case  STATS_PKT1024TO1518OCTETS:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllEtherStatsPkts1024to1518Octets;
			break;
		case STATS_TXDROPPKTS:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllIfOutDiscards;
			break;
		case STATS_TXOCTETS:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllIfHCOutOctets;/*ulAllIfOutOctets;*/
			break;
		case STATS_TXBROADCASTPKTS:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllIfHCOutBroadcastPckts;
			break;
		case STATS_TXPKTS:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllPacketSumTx;
			break;
		case STATS_TXMULTICASTPKTS:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllIfHCOutMulticastPkts;
			break;
		case STATS_TXGRANT:
			*pStatsData = 0;
			break;
		case STATS_TXPAUSEPKTS:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllDot3OutPauseFrames;
			break;
		case STATS_PAUSEPKTS:
			*pStatsData = *(unsigned long long*)&pEthInterfaceStats->ulAllDot3InPauseFrames;
			break;
		case STATS_UPBANDWIDTH:
			*pStatsData = (((*(unsigned long long*)&pEthInterfaceStats->ulInOctetRate) * 8 + 500) /1000);
			break;
		case STATS_DOWNBANDWIDTH:
			*pStatsData = (((*(unsigned long long*)&pEthInterfaceStats->ulOutOctetRate) * 8 + 500) /1000);
			break;
		default:
			return VOS_ERROR;
	}
	return STATS_OK;
}

STATUS rtStatsOnuDataGet(short int StatsObjType, 
							short int oltId,  
							short int onuId,  
							unsigned long *pStatsData)
{
#if 0
	rtStatsEntry *pTempStats = NULL;
	/*short int oltId = 0;
	oltId = GetPonPortIdxBySlot(slot, port );
	if ((oltId<0) || (oltId>19))
		return STATS_ERR;*/
	pTempStats = (rtStatsEntry*)pgStatsRealTimeOnu[oltId][onuId];
	if (pTempStats == NULL)
		return STATS_ERR;
	switch(StatsObjType)
		{
		case STATS_DROPEVENTS_TYPE:
			*pStatsData = pTempStats->rtStatsDropEvents;
			break;
		case STATS_OCTETS_TYPE:
			*pStatsData = (unsigned long)pTempStats->rtStatsOctets;
			break;
		case STATS_PKTS_TYPE:
			*pStatsData = pTempStats->rtStatsPkts;
			break;
		case STATS_BROADCASTPKTS_TYPE:
			*pStatsData =pTempStats->rtStatsBroadcastPkts;
			break;
		case STATS_MULTICASTPKTS_TYPE:
			*pStatsData = pTempStats->rtStatsMulticastPkts;
			break;
		case STATS_CRCALIGNERR_TYPE:
			*pStatsData = pTempStats->rtStatsCRCAlignErrors;
			break;
		case STATS_UNDERSIZEPKTS_TYPE:
			*pStatsData = pTempStats->rtStatsUndersizePkts;
			break;
		case STATS_OVERSIZEPKTS_TYPE:
			*pStatsData = pTempStats->rtStatsOversizePkts;
			break;
		case STATS_FRAGMENTS_TYPE:
			*pStatsData = pTempStats->rtStatsFragments;
			break;
		case STATS_JABBERS_TYPE:
			*pStatsData = pTempStats->rtStatsJabbers;
			break;
		case STATS_COLLISIONS_TYPE:
			*pStatsData = pTempStats->rtStatsCollisions;
			break;		
		case STATS_PKT64OCTETS:
			*pStatsData = pTempStats->rtStatsPkts64Octets;
			break;
		case STATS_PKT65TO127OCTETS	:
			*pStatsData = pTempStats->rtStatsPkts65to127Octets;
			break;
		case STATS_PKT128TO255OCTETS :
			*pStatsData = pTempStats->rtStatsPkts128to255Octets;
			break;
		case  STATS_PKT256TO511OCTETS:
			*pStatsData = pTempStats->rtStatsPkts256to511Octets;
			break;
		case  STATS_PKT512TO1023OCTETS:
			*pStatsData =pTempStats->rtStatsPkts512to1023Octets;
			break;
		case  STATS_PKT1024TO1518OCTETS:
			*pStatsData = pTempStats->rtStatsPkts1024to1518Octets;
			break;
		/*用于环回测试的数据获取*/
		case  STATS_ONULOOPBACK_RXOK:
			*pStatsData = pTempStats->rtStatsRxOk;
			break;	
		case  STATS_ONULOOPBACK_TXOK:/*	19*/
			*pStatsData = pTempStats->rtStatsTxOk;
			break;	
		case  STATS_ONULOOPBACK_RXERR:/*	20*/
			*pStatsData = pTempStats->rtStatsRxErr;
			break;	
		case  STATS_ONULOOPBACK_TOTALRXBAD:/*	21*/
			*pStatsData = pTempStats->rtStatsRxTotalBad;
			break;				
		default:
			return (STATS_INVALUE_OBJ_ERR);
		}
#else
	*pStatsData = 0;
#endif
	return STATS_OK;
}

/*modified by zhengyt 2008-5-22 ,解决mib中清除pon端口数据无效的问题。*/
int rtStatsDataEthClear(void)
{
	/*int i=0;
	for(;i<STATS_RT_MAXEHTER;i++)
	{
		VOS_MemSet(&gStatsRealTimeEth[i], 0, sizeof(rtStatsEntry64));
	}*/
	
#if  0
	VOS_MemZero(&pstPortStats, sizeof(ETH_PORT_STATS_S));
	for(port=1;port<=STATS_RT_MAXEHTER;port++)
		
	{
		ulIfindex =	IFM_ETH_CREATE_INDEX( 1, 20+port );
   		/*IFM_config(ulIfindex,IFM_CONFIG_ETH_CLRSTATS,&ulTemp,NULL);*/

	Set_ETH_PortStatInfor( ulIfindex,  pstPortStats);
	}
#endif
	return STATS_OK;
}

/***********************************************
* rtStatsDataPonClear
* 清除实时统计
* 参数: ponId ,当为－1时，表示清除所有的olt
*
*
*
*
*
****************************************************/
int rtStatsDataPonClear(short int ponId)
{
#if 0
	unsigned short tempOltId = 0;
	if ((ponId<STATS_OLT_ALL) || (ponId >= STATS_HIS_MAXPON))
		return STATS_ERR;
	
	if((STATS_OLT_ALL) == ponId)
		{
		while(tempOltId <STATS_HIS_MAXPON)
			{
					memset(((rtStatsEntry64*)pgStatsRealTimeOlt[tempOltId]), 0, sizeof(rtStatsEntry64));
				tempOltId++;
			}
			
		}
	else if (STATS_DEVICE_ON == gStatsPonOn[ponId])
		{
			memset(((rtStatsEntry64*)pgStatsRealTimeOlt[ponId]), 0, sizeof(rtStatsEntry64));
		}
	else
		return STATS_ERR;
#endif
	return STATS_OK;
}

/***********************************************
* rtStatsDataOnuClear
* 清除实时统计
* 参数:(1) ponId ,当为－1时，表示清除所有的onu 实时统计数据
*		(2)当ponId 不为－1，但onuId 为－1 时，表示清除该olt下
*		所有onu 的实时统计
*		(3) ponId 和onuId 为固定值
*
*
****************************************************/
int rtStatsDataOnuClear(short int ponId,short OnuId)
{
#if 0
	unsigned short tempOltId = 0;
	unsigned short tempOnuId = 0;
	if ((ponId<STATS_OLT_ALL) || (OnuId <STATS_ONU_ALL) || (ponId >= STATS_HIS_MAXPON) || (OnuId >= STATS_HIS_MAXONU))
		return STATS_ERR;
	/*情况(1)*/
	if((STATS_OLT_ALL) == ponId)
		{
		while(tempOltId <STATS_HIS_MAXPON)
			{
			while(tempOnuId <STATS_HIS_MAXONU)
				{
					memset((rtStatsEntry *)pgStatsRealTimeOnu[tempOltId][tempOnuId], 0, sizeof(rtStatsEntry));
					tempOnuId++;
				}
				tempOnuId = 0;
				tempOltId++;
			}
			
		}
	/*情况(2)*/
	else if (STATS_ONU_ALL == OnuId)
		{
			while((tempOnuId <STATS_HIS_MAXONU) &&(STATS_DEVICE_ON == gStatsOnuOn[ponId][tempOnuId]))
				{
					memset((rtStatsEntry *)pgStatsRealTimeOnu[ponId][tempOnuId], 0, sizeof(rtStatsEntry));
					tempOnuId++;
				}
		}
	/*情况(3)*/
	else if (STATS_DEVICE_ON == gStatsOnuOn[ponId][OnuId])
		{
			memset((rtStatsEntry *)pgStatsRealTimeOnu[ponId][OnuId], 0, sizeof(rtStatsEntry));
		}
	else
		return STATS_ERR;
#endif
	return STATS_OK;
}


STATUS HisStatsMaxRecordInit(void)
{
	gHis15MinMaxRecord = STATS_DEFAULT15MIN_BUCKETNUM;/*默认为取5个小时内的历史统计数据*/
	gHis24HoursMaxRecord = STATS_DEFAULT24H_BUCKETNUM;/*默认为取20天内的历史统计数据*/
	return STATS_OK;
}

/*added by wutongwu at 29 september for coding show run*/
STATUS HisStatsDefaultRecordGet(unsigned int *pDefBucket_15M, unsigned int *pDefBucket_24H)
{
	*pDefBucket_15M = STATS_DEFAULT15MIN_BUCKETNUM;/*默认为取5个小时内的历史统计数据*/
	*pDefBucket_24H = STATS_DEFAULT24H_BUCKETNUM;/*默认为取20天内的历史统计数据*/
	if (*pDefBucket_15M > STATS_MAX15MIN_BUCKETNUM)
		return STATS_ERR;
	if (*pDefBucket_24H > STATS_MAX24HOUR_BUCKETNUM)
		return STATS_ERR;
	return STATS_OK;
}

int rtStatsDataInit(void)
{
    ULONG ulSize, ulTotalSize;
    unsigned char *pByteBuf;
	/*unsigned oltId = 0;
	unsigned onuId = 0;*/

#if 1 
    ulSize = STATS_HIS_MAXPON * sizeof(unsigned char);
    ulTotalSize = ulSize + /*STATS_HIS_MAXONU*/MAXONUPERPONNOLIMIT * STATS_HIS_MAXPON * sizeof(unsigned char) + STATS_RT_MAXEHTER;
    if ( NULL == (pByteBuf = (unsigned char *)VOS_Malloc(ulTotalSize, MODULE_STATSTICS)) )
    {
        VOS_ASSERT(0);
    	return STATS_ERR;
    }
	VOS_MemZero(pByteBuf, ulTotalSize);
    
    /*gStatsEthernetOn = pByteBuf;
    pByteBuf += STATS_RT_MAXEHTER;*/
    gStatsPonOn = pByteBuf;
    pByteBuf += ulSize;

    gStatsOnuOn = pByteBuf;

    /*ulSize = STATS_RT_MAXEHTER * sizeof(rtStatsEntry64);
    ulTotalSize = ulSize + STATS_RT_MAXEHTER * sizeof(rtStatsEntry);
    if ( NULL == (pByteBuf = (unsigned char *)VOS_Malloc(ulTotalSize, MODULE_STATSTICS)) )
    {
        VOS_Free(gStatsEthernetOn);
        
        VOS_ASSERT(0);
    	return STATS_ERR;
    }
    gStatsRealTimeEth = (rtStatsEntry64*)pByteBuf;*/
    /*tgStatsRealTimeEth = (rtStatsEntry*)(pByteBuf + ulSize);
	VOS_MemZero(pByteBuf, ulTotalSize);

    ulSize = STATS_HIS_MAXPON * sizeof(rtStatsEntry64*);
    ulTotalSize = ulSize * (1 + STATS_HIS_MAXONU);
    if ( NULL == (pByteBuf = (unsigned char *)VOS_Malloc(ulTotalSize, MODULE_STATSTICS)) )
    {
        VOS_Free(gStatsEthernetOn);
        VOS_Free(gStatsRealTimeEth);
        
        VOS_ASSERT(0);
    	return STATS_ERR;
    }
    pgStatsRealTimeOlt = (rtStatsEntry64**)pByteBuf;
    pgStatsRealTimeOnu = pByteBuf + ulSize;
	VOS_MemZero(pByteBuf, ulTotalSize);*/

    /*ulSize = STATS_HIS_MAXPON * sizeof(rtStatsEntry*);
    ulTotalSize = ulSize * (1 + STATS_HIS_MAXONU);
    if ( NULL == (pByteBuf = (unsigned char *)VOS_Malloc(ulTotalSize, MODULE_STATSTICS)) )
    {
        VOS_Free(gStatsEthernetOn);
        VOS_Free(gStatsRealTimeEth);
        VOS_Free(pgStatsRealTimeOlt);
        
        VOS_ASSERT(0);
    	return STATS_ERR;
    }
    tpgStatsRealTimeOlt = (rtStatsEntry**)pByteBuf;
    tpgStatsRealTimeOnu = pByteBuf + ulSize;
	VOS_MemZero(pByteBuf, ulTotalSize);*/

    /*ulSize = STATS_HIS_MAXPON * sizeof(Statistics_debug_Table*);
    ulTotalSize = ulSize * (1 + STATS_HIS_MAXONU);
    if ( NULL == (pByteBuf = (unsigned char *)VOS_Malloc(ulTotalSize, MODULE_STATSTICS)) )
    {
        VOS_Free(gStatsEthernetOn);
        VOS_Free(gStatsRealTimeEth);
        VOS_Free(pgStatsRealTimeOlt);
        VOS_Free(tpgStatsRealTimeOlt);
        
        VOS_ASSERT(0);
    	return STATS_ERR;
    }
	VOS_MemZero(pByteBuf, ulTotalSize);
    pgDebug_StatsRealTimeOlt = (Statistics_debug_Table**)pByteBuf;
    pgDebug_StatsRealTimeOnu = pByteBuf + ulSize;*/
#else
	VOS_MemZero(gStatsPonOn, sizeof(gStatsPonOn));
	VOS_MemZero(gStatsOnuOn, sizeof(gStatsOnuOn));
    
	for (oltId = 0;oltId<STATS_HIS_MAXPON;oltId++)
	{
		pgStatsRealTimeOlt[oltId] = NULL;
		pgDebug_StatsRealTimeOlt[oltId] = NULL;
		for(onuId = 0; onuId<STATS_HIS_MAXONU; onuId++)
		{
			pgStatsRealTimeOnu[oltId][onuId] = NULL;
			pgDebug_StatsRealTimeOnu[oltId][onuId] = NULL;
		}
	}
#endif

	return STATS_OK;
}


/*****************************************
* HisStatsInit
* 说明: 初始化历史统计相关的变量和统计信息对象控制表
*		历史统计数据采集任务;
*
*
*
*/
int StatsInit(VOID)
{
	char *pTaskName = "tSTATHIS";
	/*ULONG lPriority= 100;*/

    if (gHisStatsInitStatus != 0)
	{
		sys_console_printf(" StatsInit task is already running!\r\n");
		return STATS_OK;
	}
	gpPONHisStatsHead = NULL ;
	gpPONHisStatsTail = gpPONHisStatsHead ;
	gpONUHisStatsHead = NULL ;
	gpONUHisStatsTail = gpONUHisStatsHead ;

	rtStatsDataInit();	
	HisStatsMaxRecordInit();

	/*以下三个函数顺序不能改变*/	
	/*BEGIN modified by wangxy 2007-06-28 
	VOS_MSG_Q_FIFO ->- VOS_MSG_Q_PRIORITY
	*/
	gMQidsStats = VOS_QueCreate(500, VOS_MSG_Q_PRIORITY);
	if (gMQidsStats==0)  
	{
#ifdef __STAT_DEBUG
		if( gStatisticDebugFlag == 1 )
			sys_console_printf("msgQCreate Error!\r\n");
#endif
		return (FALSE);
	}

	/*历史数据采样任务*/
	statsTaskId = VOS_TaskCreate( pTaskName, (ULONG)STATS_TASK_PRIO, (VOS_TASK_ENTRY)StatsTask, NULL);
	if(statsTaskId == 0)
	{
		if( gMQidsStats ) VOS_QueDelete(gMQidsStats);

		return (STATS_TASKSPAWN_FAILUE_ERR);
	}
	VOS_QueBindTask( (void*)statsTaskId, gMQidsStats );

	gHisStatsInitStatus = 1;
	sys_console_printf("StatsInit task is running!\r\n");
	return (STATS_OK);
}


int StatPasSystemParamterInit(void)
{
#if 0
	PAS_system_parameters_t system_parameters;
	memset(&system_parameters, 0, sizeof(PAS_system_parameters_t));
	/*system_parameters.TBD = -1;ENABLE*/
	system_parameters.statistics_sampling_cycle = 60*1000*2;/*2s*/
	system_parameters.monitoring_cycle = 60*1000*2;/*2s*/
	system_parameters.host_olt_msgs_timeout = -1;
	system_parameters.olt_reset_timeout = -1;
	system_parameters.automatic_authorization_policy = -1;
#endif
	return( pon_set_system_parameters ( 60*1000*2, 60*1000*2, -1, -1 ) );
}



/****************************************************
* HisStatsTask
* 描述: 
*		
*							
*								
*							
*		
*
*	输入: 无
*
*	输出: 无
*
*	返回: 无
******************************************************/
extern void V2R1_enable_watchdog(void);
void StatsTask(void)
{
	int	errorCode = STATS_ERR;
	historyStatsMsg *pstatsRecMsg;
	LONG 	lRes = STATS_OK;
	ULONG aulMsg[4] = {0, 0, 0, 0};

	unsigned int bucket = 0;
	BOOL	flag = FALSE;

	while( !SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
	{
		VOS_TaskDelay( VOS_TICK_SECOND);
	}

#if 1
/*#ifdef V2R1_WATCH_DOG*/
	/*V2R1_init_watchdog();*/
	V2R1_enable_watchdog();
#endif
	
	/* sleep 5 minutes */
	VOS_TaskDelay( VOS_TICK_SECOND * 60 );
#if 0
	/*实时采样超时消息*/
	RealTimeStatsTimerId = VOS_TimerCreate((ULONG)MODULE_STATSTICS, (ULONG)NULL, (LONG)RealTimeStatPeriod/* 30秒 */,\
			    ( VOID ( * ) ( VOID * ) )StatsMsgRealTimeSend, NULL, VOS_TIMER_LOOP);
	if (VOS_ERROR == RealTimeStatsTimerId)
	{
		VOS_QueDelete(gMQidsStats);
		gMQidsStats = 0;
		/*VOS_TaskDelete( statsTaskId );*/
		statsTaskId = 0;
		return ;
	}
#endif
	/*历史统计采样超时消息*/
	HisStatsTimerId = VOS_TimerCreate((ULONG)MODULE_STATSTICS, (ULONG)NULL, (LONG)HisStatPeriod/* 15分钟 */,\
			    ( VOID ( * ) ( VOID * ) )HisStatsMsgTimeOutSend, NULL, VOS_TIMER_LOOP);
	if (VOS_ERROR == HisStatsTimerId)
	{
		/*VOS_TimerDelete((ULONG)MODULE_STATSTICS, (ULONG)RealTimeStatsTimerId);
		RealTimeStatsTimerId = 0;*/
		VOS_QueDelete(gMQidsStats);
		gMQidsStats = 0;
		/*VOS_TaskDelete( statsTaskId );*/
		statsTaskId = 0;
		return ;
	}

	/*PonpoweringTimerStart();
	UplinkLosTimerStart();*/

	StatPasSystemParamterInit();

	while(1)
	{
		lRes = VOS_QueReceive(gMQidsStats, aulMsg, WAIT_FOREVER);
		if ( lRes == VOS_ERROR )   
		{
			VOS_TaskDelay( 50 );
			continue;
		}
		if( lRes == VOS_NO_MSG ) 
			continue;
	
		/*if( aulMsg[1] == FC_V2R1_TIMEOUT )
		{
			PonpoweringTimerHandler();
			continue;
		}

		if( aulMsg[1] == FC_V2R1_TIMEOUT_UPLINKLOS )
		{
			UplinkSFPLOSState_Report();
			continue;
		}*/

		pstatsRecMsg = (historyStatsMsg *)aulMsg[3];
		if( NULL == pstatsRecMsg )
		{
			VOS_ASSERT(0);
			continue;
		}
		switch(pstatsRecMsg->statsType)
		{
			/*需增加新的内容如下: 
			当某olt 或者onu 掉电不在线时的处理*/
			case STATS_PONOBJ_MODIFIED:
				bucket = (pstatsRecMsg->actcode == STATS_15MIN_SET )?pstatsRecMsg->bucketNum15m:pstatsRecMsg->bucketNum24h;
				flag = (pstatsRecMsg->actcode == STATS_15MIN_SET )?pstatsRecMsg->flag15M:pstatsRecMsg->flag24H;
				errorCode = HisStatsPonPro( pstatsRecMsg->ponId, bucket, flag, pstatsRecMsg->actcode );
				break;
				
			case STATS_ONUOBJ_MODIFIED:			
				bucket = (pstatsRecMsg->actcode == STATS_15MIN_SET )?pstatsRecMsg->bucketNum15m:pstatsRecMsg->bucketNum24h;
				flag = (pstatsRecMsg->actcode == STATS_15MIN_SET )?pstatsRecMsg->flag15M:pstatsRecMsg->flag24H;
				errorCode = HisStatsOnuPro(pstatsRecMsg->ponId, pstatsRecMsg->onuId,\
									bucket,  flag, pstatsRecMsg->actcode );
				break;

			case STATS_ETHPORT_MODIFIED:
					bucket = (pstatsRecMsg->actcode == STATS_15MIN_SET )?pstatsRecMsg->bucketNum15m:pstatsRecMsg->bucketNum24h;
					flag = (pstatsRecMsg->actcode == STATS_15MIN_SET )?pstatsRecMsg->flag15M:pstatsRecMsg->flag24H;
					errorCode = HisStatsEthPro( pstatsRecMsg->slotId, pstatsRecMsg->onuId, 
						bucket, flag, pstatsRecMsg->actcode );
				break;
				
			case STATS_TIMER_TIMEOUT:
				/*超时信息轮询并读取所有的统计信息*/
				/*历史统计采样*/
				HisStatsEthRawGet();
				HisStatsPonRawGet();
				HisStatsOnuRawGet();
				break;
				
			/*增加实时采样的内容: 
			1. 当新添加一个设备OLT 处理
			2. 新添加ONU 处理
			3. 删除OLT 时,处理为: 先删除该olt 下所有onu 的历史统计控制表(15分钟和24小时),
									删除该olt下所有onu的实时采样控制表;
									然后删除该olt的所有历史统计控制表15min和24Hour
									再删除该olt实时采样的控制表
			4. 当删除onu时,处理为: 删除onu 的历史统计控制表(15分钟和24小时)*
									再删除该onu的实时统计控制*/
			/*现在先决定如下: 当某个olt或者onu掉线时,并不清除该olt下所有onu和该olt的历
									史统计状态,仅仅停止实时采样*/
			case STATS_ONU_STATUS:
				StatsRealTimeOnuPro (pstatsRecMsg->ponId, pstatsRecMsg->onuId, pstatsRecMsg->userd);
				break;
				
			case STATS_OLT_STATUS:
				StatsRealTimePonPro (pstatsRecMsg->ponId, pstatsRecMsg->userd);
				
				break;
			/*case STATS_REALTIME_TIMEOUT:
				StatsRealTimeDataGet();
				break;*/
				
			case STATS_RTDATA_CLEAR:
				/*sys_console_printf("STATS RTPDATA_CLEAR\r\n");*/
				/*gRtStatisticClearStatus = STATS_CLEAR_INPROCESS;*/
				rtStatsDataEthClear();
				rtStatsDataPonClear(STATS_OLT_ALL);
				rtStatsDataOnuClear(STATS_OLT_ALL, STATS_ONU_ALL);
				/*gRtStatisticClearStatus = STATS_CLEAR_NOOP;*/
				break;	
				
			case STATS_LOOP_ONU_PRO:
				loopStatsRealTimeOnuPro (pstatsRecMsg->ponId, pstatsRecMsg->onuId, pstatsRecMsg->userd);
				break;
				
			case STATS_RECORD_MODIFIED:
				if (pstatsRecMsg->flag15M == 1)
				{
					if (pstatsRecMsg->bucketNum15m > pstatsRecMsg->bcktNum15mOld)
					{
						/*bucketsNum = (pstatsRecMsg->bucketNum15m - pstatsRecMsg->bcktNum15mOld)*//**60/15*/;
						HisStatsChain15MinAdd(pstatsRecMsg->bucketNum15m - pstatsRecMsg->bcktNum15mOld);
					}
					else
					{
						/*bucketsNum = (pstatsRecMsg->bcktNum15mOld - pstatsRecMsg->bucketNum15m)*/ /**60/15*/;
						HisStatsChain15MinDel(pstatsRecMsg->bcktNum15mOld - pstatsRecMsg->bucketNum15m);
					}
				}
				else if (pstatsRecMsg->flag24H == 1)
				{
					if (pstatsRecMsg->bucketNum24h> pstatsRecMsg->bcktNum24hOld)
					{
						/*bucketsNum = (pstatsRecMsg->bucketNum24h -pstatsRecMsg->bcktNum24hOld)*//**24/24*/;
						HisStatsChain24HoursAdd(pstatsRecMsg->bucketNum24h -pstatsRecMsg->bcktNum24hOld);
					}
					else
					{
						/*bucketsNum = (pstatsRecMsg->bcktNum24hOld -pstatsRecMsg->bucketNum24h)*//**24/24*/;
						HisStatsChain24HoursDel(pstatsRecMsg->bcktNum24hOld -pstatsRecMsg->bucketNum24h);
					}
				}
				break;
				
			case STATS_HISDATA_CLEAR:
				break;
				
			default:
				break;
		}
		
		VOS_Free((VOID *)pstatsRecMsg);
		pstatsRecMsg = NULL;
		aulMsg[0] = 0;
		aulMsg[1] = 0;
		aulMsg[2] = 0;
		aulMsg[3] = 0;
		
	}
	return;
}



#if 0
#endif

int CliStatsRealTimePonGet(unsigned short ponId)
{
		if (STATS_DEVICE_ON== gStatsPonOn[ponId])
			return STATS_OK;
		else
			return STATS_ERR;

}

/*显示所有在线的onu*/
int CliStatsRealTimeOnuGet(unsigned short ponId, unsigned short onuId)
{
	short int llid = 0;
	llid = GetLlidByOnuIdx(ponId, onuId);
	if ( STAT_INVALID_LLID == llid )
		return STATS_ERR;
	if (1== gStatsOnuOn[ponId][onuId] )
		return STATS_OK;
	else
		return STATS_ERR;
	
}


/*显示olt所有历史统计的控制列表*/
STATUS CliHisStatsPonStatusGet(short int ponId,unsigned int *pStatus15m,unsigned int *pStatus24h) 
{
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	if ((pStatus15m == NULL) || (pStatus24h == NULL))
		return STATS_ERR;
	
	if (pstatsPon == NULL)
		{
		return STATS_ERR;
		}
	/*sys_console_printf("  total OBJ %d\r\n", PONHistoryStatsCounter);*/
	while(pstatsPon != NULL)
		{
 			if ( ponId == pstatsPon->ponIdx)
 			{
 				(pstatsPon->iUsed15Min)?(*pStatus15m= 1):(*pStatus15m = 2);
				(pstatsPon->iUsed24Hour)?(*pStatus24h= 1):(*pStatus24h = 2);
				return STATS_OK;
 			}
			pstatsPon = pstatsPon->pNext;
		}
	return STATS_PONOBJ_NULL_ERR;
} 

STATUS CliHisStatsONUStatusGet(short int ponId,short int onuId,unsigned int *pStatus15m,unsigned int *pStatus24h) 
{
	statistic_PON_OID	*pstatsPon = gpONUHisStatsHead ;
	if ((pStatus15m == NULL) || (pStatus24h == NULL))
		return STATS_ERR;
		
	if (pstatsPon == NULL)
		{
		return STATS_ERR;
		}
	/*sys_console_printf("  total OBJ %d\r\n", PONHistoryStatsCounter);*/
	while(pstatsPon != NULL)
		{
 			if ( (ponId == pstatsPon->ponIdx) && (onuId == pstatsPon->onuIdx))
 			{
 				(pstatsPon->iUsed15Min)?(*pStatus15m= 1):(*pStatus15m = 2);
				(pstatsPon->iUsed24Hour)?(*pStatus24h= 1):(*pStatus24h = 2);
				return STATS_OK;
 			}
			pstatsPon = pstatsPon->pNext;
		}
	
	return STATS_PONOBJ_NULL_ERR;
} 



/*显示olt所有历史统计的控制列表*/
STATUS CliHisStatsPonCtrlAllGet(struct vty* vty) 
{
	short int  slotId = 0;
	short int PonChipIdx = 0;
	short int port = 0;
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;

	if (pstatsPon == NULL)
		{
		vty_out ( vty, "  NULL\r\n");
		return STATS_ERR;
		}
	/*sys_console_printf("  total OBJ %d\r\n", PONHistoryStatsCounter);*/
	while(pstatsPon != NULL)
		{
 			PonChipIdx = pstatsPon->ponIdx;
			/* modified by chenfj 2008-7-8 */
			slotId = GetCardIdxByPonChip(PonChipIdx);
			if((slotId <  PONCARD_FIRST ) || (slotId > PONCARD_LAST))
				{
				vty_out( vty, "  %% Wrong.\r\n");
				return ( STATS_ERR );
				}
			port = GetPonPortByPonChip(PonChipIdx);
			/*port = PonChipIdx % PONPORTPERCARD + 1;*/
			vty_out ( vty, "  %d/%d     ",slotId, port);
			(pstatsPon->iUsed15Min) ? vty_out ( vty, "history 15Min  is enable \r\n") : vty_out ( vty, "history  15Min  is disable \r\n");

			(pstatsPon->iUsed24Hour) ? vty_out ( vty, "          history 24Hour is enable \r\n") : vty_out ( vty, "          history 24Hour is disable \r\n");


			pstatsPon = pstatsPon->pNext;
			
		}
	return STATS_OK;
} 

/*显示onu历史统计的控制列表*/
STATUS CliHisStatsONUCtrlAllGet(struct vty* vty) 
{
	short int  slotId = 0;
	short int PonChipIdx = 0;
	short int port = 0;	
	statistic_PON_OID	*pstatsPon = gpONUHisStatsHead ;
	/*statistc_HISTORY_CHAIN *pTemp15 = NULL;
	statistc_HISTORY_CHAIN *pTemp24 = NULL;*/
	if (pstatsPon == NULL)
		{
		vty_out ( vty, "  NULL\r\n");
		return STATS_ERR;
		}
	/*sys_console_printf("  total OBJ %d\r\n", PONHistoryStatsCounter);*/
	while(pstatsPon != NULL)
		{
 			PonChipIdx = pstatsPon->ponIdx;
			/* modified by chenfj 2008-7-8 */
			slotId = GetCardIdxByPonChip(PonChipIdx);
			if((slotId <  PONCARD_FIRST ) || (slotId > PONCARD_LAST))
				{
				vty_out( vty, "  %% Wrong.\r\n");
				return ( STATS_ERR );
				}
			port = GetPonPortByPonChip(PonChipIdx);
			/*port = PonChipIdx % PONPORTPERCARD + 1;*/
			vty_out ( vty, "  %d/%d/%d",slotId, port,pstatsPon->onuIdx+1);
			
			if ((pstatsPon->onuIdx+1) > 9)
				{
				(pstatsPon->iUsed15Min) ? vty_out ( vty, "  history 15Min  is enable \r\n") : vty_out ( vty, "  history 15Min  is disable \r\n");
				(pstatsPon->iUsed24Hour) ? vty_out ( vty, "          history 24Hour is enable \r\n") : vty_out ( vty,"          history 24Hour is disable \r\n");
				}
			else
				{
				(pstatsPon->iUsed15Min) ? vty_out ( vty, "   history 15Min  is enable \r\n") : vty_out ( vty, "   history 15Min  is disable \r\n");
				(pstatsPon->iUsed24Hour) ? vty_out ( vty, "          history 24Hour is enable \r\n") : vty_out ( vty,"          history 24Hour is disable \r\n");
				}
			pstatsPon = pstatsPon->pNext;
			
		}
	return STATS_OK;
} 



/*显示当前pon端口的历史统计使能情况*/
STATUS CliHisStatsPonCtrlGet( short int ponId, struct vty* vty ) 
{
	short int brdIdx = 0;
	short int PonChipIdx = 0;
	short int onuId = 0;
	short int port = 0;
	statistic_PON_OID	*pstatsPon = gpPONHisStatsHead ;
	statistic_PON_OID	*pstatsOnu = gpONUHisStatsHead ;

	if ((pstatsPon == NULL) && (pstatsOnu == NULL))
		{
		vty_out ( vty, "  NULL\r\n");
		return STATS_OK;
		}
	
 	PonChipIdx = ponId;

	/* modified by chenfj 2008-7-8 */
	brdIdx = GetCardIdxByPonChip(PonChipIdx);
	if((brdIdx <  PONCARD_FIRST ) || (brdIdx >  PONCARD_LAST))
		{
		vty_out( vty, "  %% Wrong.\r\n");
		return ( STATS_ERR );
		}
	port = GetPonPortByPonChip(PonChipIdx);
			
	/*sys_console_printf("  total OBJ %d\r\n", PONHistoryStatsCounter);*/
	while(pstatsPon != NULL)
	{
		 vty_out ( vty, "\r\n");
 		if ( ponId == pstatsPon->ponIdx ) 
 		{
			(pstatsPon->iUsed15Min) ?   vty_out ( vty, "  pon%d/%d history 15Min  is enable \r\n" ,brdIdx,port) : vty_out ( vty, "  pon%d/%d history 15Min  is disable \r\n" ,brdIdx, port);
			(pstatsPon->iUsed24Hour) ? vty_out ( vty, "  pon%d/%d history 24Hour is enable \r\n" ,brdIdx,port) : vty_out ( vty, "  pon%d/%d history 24Hour is disable \r\n" ,brdIdx, port);

 		}
		pstatsPon = pstatsPon->pNext;
	}
	vty_out ( vty, "  onuIdx  history enable status\r\n");

	for( onuId = 0; onuId < MAXONUPERPON; onuId++)
	{
		pstatsOnu = gpONUHisStatsHead ;
		while(pstatsOnu != NULL)
		{
			if (( ponId == pstatsOnu->ponIdx ) && (onuId == pstatsOnu->onuIdx ))
			{
				vty_out ( vty, "  %d/%d/%-2d  ", brdIdx, port, onuId+1);
				(pstatsOnu->iUsed15Min) ? vty_out ( vty, "history 15Min  is enable \r\n") : vty_out ( vty, "history 15Min  is disable \r\n");
				(pstatsOnu->iUsed24Hour) ? vty_out ( vty, "          history 24Hour is enable \r\n") : vty_out ( vty,"          history 24Hour is disable \r\n");
			}
		pstatsOnu = pstatsOnu->pNext;
		}
	}	
	vty_out ( vty, "\r\n");
	return STATS_OK;
}

/*显示onu所有历史统计的控制列表*/
STATUS CliHisStatsONUCtrlGet(short int ponId, short int onuId, struct vty* vty) 
{
	short int stats_flag = 0;
	statistic_PON_OID	*pstatsPon = gpONUHisStatsHead ;
	/*statistc_HISTORY_CHAIN *pTemp15 = NULL;
	statistc_HISTORY_CHAIN *pTemp24 = NULL;*/
	if (pstatsPon == NULL)
		{
		vty_out ( vty, "  statistic-history disable.\r\n");
		return STATS_OK;
		}
	/*sys_console_printf("  total OBJ %d\r\n", PONHistoryStatsCounter);*/
	while(pstatsPon != NULL)
	{
		if (( ponId == pstatsPon->ponIdx ) && (onuId == pstatsPon->onuIdx ))
		{
			vty_out ( vty, "\r\n");
			stats_flag++;
			(pstatsPon->iUsed15Min) ? vty_out ( vty, "  history 15Min  is enable \r\n") : vty_out ( vty, "  history 15Min  is disable \r\n");
			(pstatsPon->iUsed24Hour) ? vty_out ( vty, "  history 24Hour is enable \r\n") : vty_out ( vty,"  history 24Hour is disable \r\n");
			vty_out ( vty, "\r\n");
			return STATS_OK;
		}
		pstatsPon = pstatsPon->pNext;
	}	
	if(stats_flag == 0)
		vty_out ( vty, "  statistic-history disable.\r\n");
	vty_out ( vty, "\r\n");
	return STATS_OK;
} 


#if BER_AND_FER
void	calculatePonBer(  rtStatsEntry *pCurStatsEntry )
{

	long long curErrRecord = (long long)pCurStatsEntry->rtStatsErrOctets;
	long long curRecord = (long long )pCurStatsEntry->rtStatsOctets;

	if(  pCurStatsEntry==NULL || curRecord==0 )
		return;

	pCurStatsEntry->rtBerRecord = (long double)curErrRecord/curRecord;
	
}

void	calculatePonFer( rtStatsEntry *pCurStatsEntry )
{
	long	long	curErrRecord = (long long )pCurStatsEntry->rtStatsErrPkts;
	long long curRecord = ( long long )pCurStatsEntry->rtStatsPkts;

	if(  pCurStatsEntry==NULL || curRecord==0 )
		return;

	pCurStatsEntry->rtFerRecord = (long double)curErrRecord/curRecord;

}

int  calculateBandWidth_Onu(short int PonId,short int OnuId,rtStatsEntry *rtStatsTemp)
{
#if 0
	rtStatsEntry64 * pTempRtStats=NULL;

	pTempRtStats=(rtStatsEntry64 *) pgStatsRealTimeOnu[PonId][OnuId];

	pTempRtStats->rtUpBandWidth=(unsigned long )(pTempRtStats->rtStatsOctets*10/(RealTimeStatPeriod/1000));
	pTempRtStats->rtDownBandWidth=(unsigned long )(pTempRtStats->rtStatsTransmmitOctets*10/(RealTimeStatPeriod/1000));

/*#ifdef __STAT_DEBUG
	if(gStatisticDebugFlag==1)
	{sys_console_printf("onu upbandwidth=%lu\r\n",pTempRtStats->rtUpBandWidth);
	sys_console_printf ("onu downbandwidth=%lu\r\n",pTempRtStats->rtDownBandWidth);
	sys_console_printf ("pgStatsRealTimeOlt[%d][%d].upbandwidth=%d\r\n",PonId,OnuId,pgStatsRealTimeOlt[PonId]->rtUpBandWidth);
	sys_console_printf ("pgStatsRealTimeOlt[%d][%d].downbandwidth=%d\r\n",PonId,OnuId,pgStatsRealTimeOlt[PonId]->rtDownBandWidth);
	}
#endif*/
	rtStatsTemp->rtUpBandWidth=pTempRtStats->rtUpBandWidth;
	rtStatsTemp->rtDownBandWidth=pTempRtStats->rtDownBandWidth;
#endif
	return 0;
}
#endif

int  calculateBandWidth_Pon(short int PonId,rtStatsEntry *rtStatsTemp)
{
#if 0
	rtStatsEntry64 * pTempRtStats=NULL;
	rtStatsEntry64 * tPTempRtSats=NULL;
	
	pTempRtStats =  (rtStatsEntry64 *)pgStatsRealTimeOlt[PonId];
	tPTempRtSats =  (rtStatsEntry64 *)tpgStatsRealTimeOlt[PonId];


	pTempRtStats->rtUpBandWidth = (unsigned long)((pTempRtStats->rtStatsOctets-tPTempRtSats->rtStatsOctets)*10/(RealTimeStatPeriod/1000));
	pTempRtStats->rtDownBandWidth = (unsigned long)((pTempRtStats->rtTxOctets-tPTempRtSats->rtTxOctets)*10/(RealTimeStatPeriod/1000));


	rtStatsTemp->rtUpBandWidth=pTempRtStats->rtUpBandWidth;
	rtStatsTemp->rtDownBandWidth=pTempRtStats->rtDownBandWidth;
#endif
	return 0;
}

/*void calculateRtBandWidth( rtStatsEntry *pCurStatsEntry, const char type )
{
#ifdef __RT_CALCULATE_BW
	long long curRecvBytes = ( long long )pCurStatsEntry->rtStatsOctets;
	long long curTransBytes = ( long long )pCurStatsEntry->rtStatsTransmmitOctets;
#ifdef __STAT_DEBUG
	if(gStatisticDebugFlag==1)
	{
		sys_console_printf ("rtstatsOctets=%lu\r\n",curRecvBytes);
		sys_console_printf ("rtStatsTransmit=%lu\r\n",curTransBytes);
	}
#endif
	if( type == OLT_PON_PORT )
	{
		pCurStatsEntry->rtUpBandWidth = (unsigned long)curRecvBytes*10/(RealTimeStatPeriod/1000);
		pCurStatsEntry->rtDownBandWidth = (unsigned long)curTransBytes*10/(RealTimeStatPeriod/1000);

#ifdef __STAT_DEBUG
		if(gStatisticDebugFlag==1)
		{sys_console_printf("Ponupbandwidth=%lu\r\n",pCurStatsEntry->rtUpBandWidth);
		sys_console_printf ("Pondownbandwidth=%lu\r\n",pCurStatsEntry->rtDownBandWidth);}
#endif
	}
	else
	{
		pCurStatsEntry->rtUpBandWidth = (unsigned long ) curTransBytes*10/(RealTimeStatPeriod/1000);
		pCurStatsEntry->rtDownBandWidth = (unsigned long)curRecvBytes*10/(RealTimeStatPeriod/1000);

#ifdef __STAT_DEBUG
		if(gStatisticDebugFlag==1)
		{sys_console_printf("Onuupbandwidth=%lu\r\n",pCurStatsEntry->rtUpBandWidth);
		sys_console_printf ("Onudownbandwidth=%lu\r\n",pCurStatsEntry->rtDownBandWidth);}
#endif		
	}
#endif
}*/

/* added by xieshl 20091010, 用作5001PON 上行无流量监测 */
int getPonToEthRxPkts( int slotno, int portno, int ponId, ULONGLONG *pRxPkts64 )
{
	ETH_PORT_STATS_S stPortStats;
	ULONG ulIfx = 0;

	if( pRxPkts64 == NULL )
		return VOS_ERROR;
	ulIfx = userSlot_userPort_2_Ifindex( slotno, portno );
	if( ulIfx == 0xffff )
		return VOS_ERROR;

	if( Get_ETH_PortStatInfor(ulIfx, &stPortStats) == VOS_ERROR )
	{
		return VOS_ERROR;
	}

	VOS_MemCpy( pRxPkts64, &stPortStats.ulAllPacketSumRx, sizeof(ULONGLONG) );
	
	return VOS_OK;
}

#if 0
void testUllong( unsigned  long low, unsigned long high, long opval )
{
	union{
		unsigned long long tv;
		struct{
			ULONG high;
			ULONG low;
		}long_st;
	}un_llong;

	un_llong.long_st.low = low;
	un_llong.long_st.high = high;

	sys_console_printf("\r\n var len is %d", sizeof( un_llong.tv ) );
	

	un_llong.tv += opval;
	
	sys_console_printf("\r\n test value high is %d", un_llong.long_st.high );
	sys_console_printf("\r\n test value low is %d", un_llong.long_st.low );
}


void Stats_printf(unsigned char *inputDesc,long double  inputData)
{
	unsigned char buf[256]={0};
	int len=0;

	memset(buf ,0,256);
	VOS_StrCpy(buf, inputDesc);
	len=strlen(buf);
	sprintf64Bits(buf+len, (unsigned long long) inputData);
	sys_console_printf (":%s\r\n",buf);
	sys_console_printf ("inputdata=%lu\r\n",inputData);

	/*
	VOS_MemCpy(bit32, &inputData, 8);
	sys_console_printf ("%s:%x%x\r\n",inputDesc,bit32[1],bit32[2]);*/
}

int  test_pirntf ()
{
	unsigned char abc[10]={"12@ab"};
	long double inputdata=1894350126385563;
	Stats_printf(abc,inputdata);

	return 0;
}
#endif

/*****************************************************
 *
 *    Function: int ClearOltPonPortStatisticCounter(short int PonPortIdx )
 *
 *    Param:  short int PonPortIdx -- the specific pon port 
 *
 *    Desc:  OLT 中各统计计数器清 0
 *
 *    Return:  OK
 *
 ******************************************************/
 int  ClearOltPonPortStatisticCounter(short int PonPortIdx )
{
	CHECK_PON_RANGE

	if( OLT_CALL_ISOK ( OLT_ResetCounters(PonPortIdx) ) )
		return(ROK);
	return(RERROR);
}

/*****************************************************
 *
 *    Function: int ClearOnuPonPortStatisticCounter(short int PonPortIdx, short int OnuIdx)
 *
 *    Param:  short int PonPortIdx -- the specific pon port 
 *			   short int OnuIdx -- Onu 
 *
 *    Desc:  ONU 上各统计计数器清 0
 *
 *    Return:  若ONU 不在线, 返回RERROR;
 *                 若函数执行正确, 返回 ROK
 *
 ******************************************************/

int  ClearOnuPonPortStatisticCounter(short int PonPortIdx, short int OnuIdx )
{
	short int Llid;
	CHECK_PON_RANGE

#if 1
    if ( OLT_CALL_ISOK(OnuMgt_ResetCounters(PonPortIdx, OnuIdx)) )
    {
		return(ROK);
    }
#else
	Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if(Llid == INVALID_LLID)
		return(RERROR);
	if(REMOTE_PASONU_clear_all_statistics(PonPortIdx, Llid) == PAS_EXIT_OK)
		return(ROK);
#endif    
	return(RERROR);
}

