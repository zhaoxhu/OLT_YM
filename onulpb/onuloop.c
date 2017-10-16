
/***************************************************************
*
*						Module Name:  onuloop.c 
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
** DESCRIPTION : 处理与ONU环回相关的各种事件。
*
* modified by wutw at 16 October 
*	1.修改当onu处于环回状态时，设置停止环回，从而引起别的
*	onu不可以设置环回测试的问题;
*	2.当onu处于环回测试状态时，忽然发生onu deregister，onu
*	配置失效，软件配置确仍为环回状态，正常统计数据包。修改
*	此部分代码
* modified by wutw at 19 October 
*	修改当设置环回时间为非4s的倍数时,当环回结束后不能归零的问题
* modified by wutw at 23 October 
* 	pclint代码检查
* modified wutw 2006/12/05
*	解决当启动外部环回,并且环回时间已经完成之后,命令不能再启动环回
*	的问题.当外部环回的时间结束时,没有有效置值为非环回状态,导致再设置
*	环回时,该pon已经检测到有环回的onu pon端口
*	增加查看处于环回状态的onu信息的接口
***************************************************************/
#include "OltGeneral.h"
#if( EPON_MODULE_ONU_LOOP == EPON_MODULE_YES )
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "gwEponMibData.h"
#include "Onuloop.h"
#include "gwEponSys.h"

/*#undef LOOP_DEBUG
#define LOOP_DEBUG*/


#define LOOP_INVALID_LLID 	INVALID_LLID /*(-1)*/

typedef struct{
	BOOL	loopEn;
	short int onuId;
	short int loopType;
	short int loopMode;
	short int loopLinkSpeed;
}LOOP_llid_info;


typedef struct{
	
	BOOL loopEn;
	short int onuId;
	
	short int OnuLpbCtrl;
	short int OnuLpbSource;
	unsigned int OnuLpbTime; 
	unsigned int OnuLpbDefaultTime; 
	short int loopType;
	short int loopMode;
	short int loopLinkSpeed;
	short int loopFrameLen;	
	/*LOOP_llid_info loopOnuInfo[LOOP_MAX_ONU_NUM];*/
}LOOP_Pon_link_info;

/* Link test results structure */
typedef struct
{
	
	long int			numberOfSentFrames;				/* Number of measurement frames sent during link  */
	/* test											  */
	long int			numberOfReturnedFrames;			/* Number of measurement frames received during   */
	/* link test									  */
	long int			numberOfErroredReturnFrames;	/* Number of measurement error frames received	  */	
	long int			numberOfBadReturnFrames;
	/* during link test								  */
	unsigned long int	minimalDelay;						/* The minimal delay collected from link delay    */
	/* measurements. Stated in TQ units. Valid only if*/
	/* link delay measurement is requested.			  */
	unsigned long int	meanDelay;							/* The mean delay collected from link delay 	  */
	/* measurements. Stated in TQ units. Valid only if*/
	/* link delay measurement is requested.			  */
	unsigned long int   maximalDelay;						/* The maximal delay collected from link delay    */
	/* measurements. Stated in TQ units. Valid only if*/
	/* link delay measurement is requested.			  */
	long int			BaseNumberOfSentFrames;		/* 当使用外部数据源时该参数为基准参考参数
	即: 环回第一次读到的统计技术值。以下同*/		
	long int			BaseNumberOfReturnedFrames;	
	
	long int			BaseNumberOfErroredReturnFrames;	
	
	long int			BaseNumberOfBadReturnFrames;	
} LOOP_PON_link_test_results_t;
#if 0
/* Link test VLAN configuration structure */
typedef struct
{
	BOOL				 vlan_frame_enable;	/* The frames in the link test include vlan tag.			  */
	/* Values: DISABLE, ENABLE.									  */
	/* When eanbled - measurment frame size increases by		  */
	/* VLAN_TAG_SIZE bytes.										  */
	char  vlan_priority;	 /*VLAN priorty range: VLAN_MIN_TAG_PRIORITY-VLAN_MAX_TAG_PRIORITY*/
	short int		     vlan_tag;			/* the vlan tag id of the frame:							  */
	/* Range: VLAN_MIN_TAG_ID - VLAN_MAX_TAG_ID					  */
	
	
} LOOP_PON_link_test_vlan_configuration_t;
#endif
typedef struct
{
	short int			 oltId;
	short int 			onuId;
	
	short int 			OnuLpbSource;
	unsigned int		OnuLpbTime;
	short int			macMode;
	short int			  loopbackMode;
	short int 			paramType;
	short int 			loopFrameLen;
	short int			OnuLpbCtrl;
	PON_loopback_t		  loopbackType;
	PON_link_rate_t	    onuPhyLoopSpeed ;
	
	
}LOOP_msg_t;



/* Link test */
#define LOOP_PON_LINK_TEST_CONTINUOUS  (-1)    /* Start a continuous link test 			     */
#define LOOP_PON_LINK_TEST_OFF		  0     /* Stop an already running continous link test */
#define LOOP_PON_LINK_TEST_MAX_FRAMES  (32766) /* For specified number of frames test		 */

/*onu环回的统计变量*/
#if 0
LOOP_Pon_link_info *pgLoopOnuInfo[LOOP_MAX_PON_NUM][LOOP_MAX_ONU_NUM];
LOOP_PON_link_test_results_t *pgLoopStat[LOOP_MAX_PON_NUM][LOOP_MAX_ONU_NUM];
#else
LOOP_Pon_link_info* (*pgLoopOnuInfo)[MAXONUPERPONNOLIMIT];/*LOOP_MAX_ONU_NUM*/
LOOP_PON_link_test_results_t* (*pgLoopStat)[MAXONUPERPONNOLIMIT];/*LOOP_MAX_ONU_NUM*/
#endif

ULONG OnuLoopQueue = 0;
short int gOnuLoopInitEn = 0;
LONG OnuLoopTimerId = 0;
unsigned int OnuLoopPeriod = 1000*LOOP_TIMEOUT;/*2 s*/
short int looperrorCode = LOOP_OK;
VOS_HANDLE gLoopTaskId = 0;
/*int PonStatisticData[5];
static unsigned long timestamp = 0;
long double  statistics_data = 0;*/


/*外部函数声明*/
extern short int GetLlidByOnuIdx( short int PonPortIdx, short int OnuIdx );
extern STATUS loopStatsMsgOnuOff(unsigned short oltId, unsigned short onuId);
extern STATUS loopStatsMsgOnuOn(unsigned short oltId, unsigned short onuId);
 /* deleted by chenfj 20008-7-9 
     this definitation is imported from includeFromPas.h 
     */
#if 0  
typedef short int  PON_device_id_t;  /* C-type representation of device index */
typedef enum 
{
	PAS_MAC_MODE_NOT_CHANGED = (-1),   /* MAC mode not changed					   */
		PAS_PON_MAC				 = 0,							/* Regular 802.3ah MAC					   */
		PAS_ETHERNET_MAC										/* OLT - regular Ethernet switch, with no  */
		/*       802.3ah extensions				   */
		/* ONU - Not supported					   */
} PON_mac_test_modes_t;
typedef enum
{
	PON_LOOPBACK_MAC = 1, /* Loopback is done inside MAC chip   */
		PON_LOOPBACK_PHY	  /* Loopback is done inside PHY device */
} PON_loopback_t;
/* Link rate */
typedef enum 
{
	PON_10M   = 10,	  /* MB/Sec				 */
		PON_100M  = 100,  /* MB/Sec				 */
		PON_1000M = 1000  /* MB/Sec (= 1 GB/Sec) */
} PON_link_rate_t;
#endif

/*
extern short int PAS_set_test_mode_v4 ( const short int			  olt_id, 
									   const PON_device_id_t		  device_id,
									   const PON_mac_test_modes_t  mac_mode,
									   const long int			  loopback_mode,
									   const PON_loopback_t		  loopback_type,
									   const PON_link_rate_t	      onu_phy_loopback_speed_limitation 
									   );

*/
									   /*其类行为: 		
									   case  STATS_ONULOOPBACK_RXOK:收到的ok数据包
									   
										 case  STATS_ONULOOPBACK_TXOK:发出的ok的数据包
										 
										   case  STATS_ONULOOPBACK_RXERR:收到的错误数据包
										   
		case  STATS_ONULOOPBACK_TOTALRXBAD:收到的总的坏的数据包*/
extern STATUS rtStatsOnuDataGet(short int StatsObjType, unsigned short oltid, 
								unsigned short onuid, unsigned long *pStatsData);
/*内部函数声明*/
void OnuLoopTask();
int OnuLoopInit(void);
int OnuLoopTimeOutSend(void);

int OnuLoopIntLimitTime(short int ponId, short int onuId ,short int frame_size);
int OnuLoopIntAllTime(short int ponId, short int onuId ,short int frame_size);


int OnuLoopExtLimitTime(short int ponId, short int onuId );
int OnuLoopExtAllTime(short int ponId, short int onuId);
int OnuLoopPhyLinkModeSet(short int ponId,
						  short int onuId,
						  short int loopBackMode,
						  short int loopBackType,
						  short int loopSpeedLimited,
						  short int loopFrameLen,
						  short int onuLpbSource,
						  unsigned int onuLpbTime,
						  short int OnuLpbCtrl
						  );
int OnuLoopPhyLinkTest(void);

#if 0
#endif


/**************************************************
* OnuLoopPhyTestLpbCtrlSet
* 描述: 该函数设置环回
*
*
*
*
*
***************************************************/
int OnuLoopPhyTestLpbCtrlSet( short int PonId, short int OnuId , /*short int onuLpbSrc, short int onuLbpTime, */short int loopEn)
{
/*LOOP_mac_test_modes_t mac_mode;
	LOOP_loopback_t loopback_type;*/
	/*LOOP_PON_link_rate_t onu_phy_loopback_speed_limitation;*/
	/*int  loopback_mode;*/
	ULONG ulMsg[4] = {0};
	short int Ret = LOOP_OK;
	LOOP_msg_t	*plpMsg = NULL;
	short int count = 0;
	short int loopFrameLen = 100;
	short int llid = 0;
	/*	short int OnuLpbCtrl = 0;*/
	unsigned int onuLpbSrc= 0;
	unsigned int onuLbpTime = 0;
	short int lpBckMode = 0;
	if ((PonId>(LOOP_MAX_PON_NUM-1)) || (OnuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}
	
	llid = GetLlidByOnuIdx( PonId, OnuId);
	if ( LOOP_INVALID_LLID == llid )
		return LOOP_ERR;
	if ((loopEn != LOOP_START) && (loopEn != LOOP_STOP))
		return LOOP_ERR;
	
	lpBckMode = 32767;
	if (loopEn == LOOP_START)
	{
		if (TRUE == pgLoopOnuInfo[PonId][OnuId]->loopEn)
		{
			return LOOP_ONULOOP_EXIST;
		}
	}
	else if (loopEn == LOOP_STOP/*LOOP_NOOP*/)
	{
		if (FALSE == pgLoopOnuInfo[PonId][OnuId]->loopEn)
			return LOOP_ONULOOP_EXIST;
		lpBckMode = LOOP_OFF;
		
	}
	else
		return LOOP_ERR;
	
	if ( LOOP_OK != OnuLoopLinkTestLpbScrGet(PonId, OnuId , &onuLpbSrc))
	{
		return LOOP_ERR;
	}
	
	if ((onuLpbSrc != LOOP_INTERNAL) && (onuLpbSrc != LOOP_EXTERNAL))
		return LOOP_ERR;
	
	if ( LOOP_OK != OnuLoopLinkTestLpbTimeGet(PonId, OnuId , &onuLbpTime))
	{
		return LOOP_ERR;
	}
	
	plpMsg = (LOOP_msg_t *)VOS_Malloc((ULONG)sizeof(LOOP_msg_t), (ULONG)MODULE_LOOPBACK);
	if (NULL == plpMsg)
	{
		return LOOP_ERR;
	}
	
	plpMsg->loopbackMode = lpBckMode;
	plpMsg->OnuLpbCtrl = loopEn;
	plpMsg->oltId = PonId;
	plpMsg->onuId = OnuId;
	plpMsg->macMode = (short int)PAS_PON_MAC;
	plpMsg->loopbackType = PON_LOOPBACK_PHY;
	plpMsg->onuPhyLoopSpeed = PON_100M;
	plpMsg->loopFrameLen = 	loopFrameLen;
	plpMsg->paramType = LOOP_SET_MODE;
	plpMsg->OnuLpbSource = onuLpbSrc;
	
	/*if (onuLbpTime >= 0)*/
	plpMsg->OnuLpbTime = onuLbpTime;
	/*	else
	{
	return LOOP_ERR;
}*/
	
	ulMsg[3] = (ULONG)(plpMsg);
	
	looperrorCode = LOOP_TIMEOUT_ERR;
	
	Ret = VOS_QueSend(OnuLoopQueue, ulMsg, NO_WAIT,MSG_PRI_NORMAL);
	
	if (VOS_OK != Ret)
	{
		ASSERT( 0 );
#ifdef	LOOP_DEBUG		
		sys_console_printf("VOS_Quesend ERROR!\r\n");
#endif
		VOS_Free((VOID *)plpMsg);
		plpMsg = NULL;
		return Ret;
	}	
	
	while((looperrorCode != LOOP_OK))
	{
		VOS_TaskDelay(20);
		if (count>=3)
		{
			return looperrorCode;
		}
		count++;
	}
#ifdef LOOP_DEBUG
	sys_console_printf(" looperrorCode = %d\r\n",looperrorCode);
#endif
	return looperrorCode;
	
	
}



/**************************************************
* OnuLoopPhyTestStatusGet
* 描述: 该函数获取环回状态
*
*
*
*
*
***************************************************/
int OnuLoopPhyTestLpbCtrlGet( short int PonId, short int OnuId , short int *pLoopEn)
{
	short int onulpStatus = LOOP_NOOP;
	if (NULL == pLoopEn)
		return LOOP_ERR;
	if ((PonId>(LOOP_MAX_PON_NUM-1)) || (OnuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}
	
	/*if (NULL != pgLoopOnuInfo[PonId][OnuId])
	{*/
	onulpStatus = pgLoopOnuInfo[PonId][OnuId]->OnuLpbCtrl;
	/*if ((onulpStatus == LOOP_PROCESS) || (onulpStatus == LOOP_START))*/
	if( onulpStatus != LOOP_NOOP )
		*pLoopEn = LOOP_PROCESS;
	else
		*pLoopEn = LOOP_NOOP;
	/**pLoopEn = pgLoopOnuInfo[PonId][OnuId]->OnuLpbCtrl;*/
	return LOOP_OK;
	/*		}
	else 
	*pLoopEn = LOOP_NOOP;*/
	
	
}

/**************************************************
* OnuLoopLinkTestRxStatGet
* 描述: 该函数获取环回测试中接收到的数据包
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestRxStatGet(short int PonId, short int OnuId , long int *pRxStats)
{
	if (NULL == pRxStats)
		return LOOP_ERR;
	if ((PonId>(LOOP_MAX_PON_NUM-1)) || (OnuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}
/*	*pRxStats = pgLoopStat[PonId][OnuId]->numberOfReturnedFrames;*/	/* zhengyt 20080905 */
	*pRxStats = pgLoopStat[PonId][OnuId]->numberOfSentFrames;
	return LOOP_OK;
}

/**************************************************
* OnuLoopLinkTestTxStatGet
* 描述: 该函数获取环回测试中发送的数据包
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestTxStatGet(short int PonId, short int OnuId , long int *pRxStats)
{
	if (NULL == pRxStats)
		return LOOP_ERR;
	if ((PonId>(LOOP_MAX_PON_NUM-1)) || (OnuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}
	*pRxStats = pgLoopStat[PonId][OnuId]->numberOfSentFrames;
	return LOOP_OK;
}


/**************************************************
* OnuLoopLinkTestLpbScrSet
* 	描述: 该函数设置环回测试中的loopback Scr
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestLpbScrSet(short int PonId, short int OnuId , unsigned int lpbScr)
{
	if ((PonId>(LOOP_MAX_PON_NUM-1)) || (OnuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}
	if ( ( LOOP_INTERNAL != lpbScr) && (LOOP_EXTERNAL != lpbScr) )
		return LOOP_ERR;
	pgLoopOnuInfo[PonId][OnuId]->OnuLpbSource = lpbScr;
	return LOOP_OK;
	
}

/**************************************************
* OnuLoopLinkTestLpbScrGet
* 描述: 该函数获取环回测试中的loopback Scr
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestLpbScrGet(short int PonId, short int OnuId , unsigned int *pLpbScr)
{
	
	if (NULL == pLpbScr)
		return LOOP_ERR;
	if ((PonId>(LOOP_MAX_PON_NUM-1)) || (OnuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}
	*pLpbScr = pgLoopOnuInfo[PonId][OnuId]->OnuLpbSource;
	return LOOP_OK;
	
}

/**************************************************
*  OnuLoopLinkTestLpbTimeSet
* 	描述: 该函数设置环回测试中的loopback time
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestLpbTimeSet(short int PonId, short int OnuId , unsigned int lpbTime)
{
	
	if ((PonId>(LOOP_MAX_PON_NUM-1)) || (OnuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}
	/*0xfffffff ,268435455,时间参数限制在该范围值内*/
	if (lpbTime > 0xfffffff)
		return LOOP_ERR;
	/*pgLoopOnuInfo[PonId][OnuId]->OnuLpbTime = lpbTime;*/
	pgLoopOnuInfo[PonId][OnuId]->OnuLpbDefaultTime = lpbTime;
	return LOOP_OK;
	
}

/**************************************************
* OnuLoopLinkTestLpbTimeGet
* 	描述: 该函数获取环回测试中的loopback time
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestLpbTimeGet(short int PonId, short int OnuId , unsigned int *pLpbTime)
{
	if (NULL == pLpbTime)
		return LOOP_ERR;
	if ((PonId>(LOOP_MAX_PON_NUM-1)) || (OnuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}
	*pLpbTime = pgLoopOnuInfo[PonId][OnuId]->OnuLpbDefaultTime;
	return LOOP_OK;
	
}


/**************************************************
* OnuLoopLinkTestLpbTimeGet
* 	描述: 该函数获取环回测试中的loopback time
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestLpbRunningTimeGet(short int PonId, short int OnuId , unsigned int *pLpbTime)
{
	if (NULL == pLpbTime)
		return LOOP_ERR;
	if ((PonId>(LOOP_MAX_PON_NUM-1)) || (OnuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}
	*pLpbTime = pgLoopOnuInfo[PonId][OnuId]->OnuLpbTime;
	return LOOP_OK;
	
}
/*********************************
* OnuLoopLinkTestLpbStatusGet
* description : Get the current onu loop status : process/noop.
*
*
*
***************************************/
int OnuLoopLinkTestLpbStatusGet(short int PonId, short int OnuId , short int *pLpbStatus)
{
	
	if (NULL == pLpbStatus)
		return LOOP_ERR;
	if ((PonId>(LOOP_MAX_PON_NUM-1)) || (OnuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}
	*pLpbStatus = pgLoopOnuInfo[PonId][OnuId]->OnuLpbCtrl;
	return LOOP_OK;
	
}

/********************************************
* OnuLoopLinkTestLpbOnuGet
* decription :
*	获取当前pon端口下为环回状态的ONUid
*
*
*
*
*********************************************/

int OnuLoopLinkTestLpbOnuGet(short int PonId, short int *pOnuId)
{
	short int tempOnuId = 0;
	if (NULL == pOnuId)
		return LOOP_ERR;
	if (PonId>(LOOP_MAX_PON_NUM-1))
	{
		return LOOP_DEVICE_INVALUE;
	}
	for( tempOnuId = 0;tempOnuId < LOOP_MAX_ONU_NUM; tempOnuId++)
	{
		if ( TRUE == pgLoopOnuInfo[PonId][tempOnuId]->loopEn )
		{
			*pOnuId = pgLoopOnuInfo[PonId][tempOnuId]->onuId;
			return LOOP_OK;
		}
		continue;
	}
	return LOOP_OK;
}
/**************************************************
* OnuLoopInit
* 	描述: onu环回测试模块初始化，必须在启动时调用
*
*
*
*
*
***************************************************/
int OnuLoopInit(void)
{
	short int count = 0;
	short int tempOnuId = 0;
    ULONG ulSize;
    
	if (0 != gOnuLoopInitEn)
	{
#ifdef LOOP_DEBUG
		sys_console_printf(" Onu loopback is already running!\r\n");
#endif
		return LOOP_ERR;
	}

#if 1
    ulSize = sizeof(LOOP_Pon_link_info*) * LOOP_MAX_PON_NUM * MAXONUPERPONNOLIMIT/*LOOP_MAX_ONU_NUM*/;
    if ( NULL == (pgLoopOnuInfo = VOS_Malloc(ulSize, (ULONG)MODULE_LOOPBACK)) )
    {
        VOS_ASSERT(0);
		return LOOP_ERR;
    }

    ulSize = sizeof(LOOP_PON_link_test_results_t*) * LOOP_MAX_PON_NUM * MAXONUPERPONNOLIMIT/*LOOP_MAX_ONU_NUM*/;
    if ( NULL == (pgLoopStat = VOS_Malloc(ulSize, (ULONG)MODULE_LOOPBACK)) )
    {
        VOS_ASSERT(0);
		return LOOP_ERR;
    }
#endif

	for (count = 0; count<LOOP_MAX_PON_NUM;count++)
	{
		for(tempOnuId = 0; tempOnuId < MAXONUPERPONNOLIMIT/*LOOP_MAX_ONU_NUM*/; tempOnuId++)
		{
			/*pgLoopOnuInfo[count][tempOnuId] = NULL;*/
			pgLoopOnuInfo[count][tempOnuId] = (LOOP_Pon_link_info* )VOS_Malloc((ULONG)sizeof(LOOP_Pon_link_info), (ULONG)MODULE_LOOPBACK);
			if (NULL == pgLoopOnuInfo[count][tempOnuId])
			{
				looperrorCode = LOOP_GETMEM_ERR;
				return looperrorCode;
			}
			memset(pgLoopOnuInfo[count][tempOnuId], 0, sizeof(LOOP_Pon_link_info));
			pgLoopOnuInfo[count][tempOnuId]->loopEn = FALSE;
			pgLoopOnuInfo[count][tempOnuId]->OnuLpbSource = LOOP_INTERNAL;
			pgLoopOnuInfo[count][tempOnuId]->OnuLpbTime = 0;
			pgLoopOnuInfo[count][tempOnuId]->OnuLpbDefaultTime = 10;
			pgLoopOnuInfo[count][tempOnuId]->OnuLpbCtrl = LOOP_NOOP;
			pgLoopStat[count][tempOnuId] = (LOOP_PON_link_test_results_t *)VOS_Malloc((ULONG)sizeof(LOOP_PON_link_test_results_t), (ULONG)MODULE_LOOPBACK);
			if (NULL == pgLoopStat[count][tempOnuId] )
			{
				return LOOP_ERR;
			}
		}
	}
	
	OnuLoopQueue = VOS_QueCreate(200, VOS_MSG_Q_FIFO);
	if (0 == OnuLoopQueue)
	{
#ifdef LOOP_DEBUG
		sys_console_printf(" OnuLoopQueue Create Err!\r\n");
#endif
		return LOOP_ERR;
	}
	/*历史数据采样任务*/
	gLoopTaskId = ( VOS_HANDLE )VOS_TaskCreate( "tLoop", (ULONG)LOOP_TASK_PRIO, (VOS_TASK_ENTRY)OnuLoopTask, NULL);
	if(gLoopTaskId == 0)
	{
#ifdef LOOP_DEBUG
		sys_console_printf("taskspawn Error!\r\n");
#endif
		VOS_QueDelete(OnuLoopQueue);
		
		return (LOOP_ERR);
	}
	VOS_QueBindTask(gLoopTaskId, OnuLoopQueue);
	/*环回超时消息*/
	OnuLoopTimerId = VOS_TimerCreate((ULONG)MODULE_STATSTICS, (ULONG)NULL, (LONG)OnuLoopPeriod,\
		( VOID ( * ) ( VOID * ) )OnuLoopTimeOutSend, NULL, VOS_TIMER_LOOP);
	if (VOS_ERROR == OnuLoopTimerId)
	{
#ifdef STATS_DEBUG
		sys_console_printf("msgQCreate Error!\r\n");
#endif		 
		VOS_QueDelete(OnuLoopQueue);
		VOS_TaskDelete( gLoopTaskId );
		return (LOOP_ERR);
	}
	
	OnuLoop_CommandInstall();

	gOnuLoopInitEn = 1;
	sys_console_printf("Onu loopback task is running!\r\n");
	return LOOP_OK;
	
}


/**************************************************
* OnuLoopTask
* 	描述: onu环回测试模块任务，被初始化函数调用
*
*
*
*
*
***************************************************/
void OnuLoopTask()
{
	LONG 	lRes = LOOP_OK;
	ULONG aulMsg[4] = {0};
	short int paramType = 0;
	LOOP_msg_t *pLpMsg = NULL;	
	
	while(1)
	{
		lRes = VOS_QueReceive(OnuLoopQueue, aulMsg, WAIT_FOREVER);
		if ( lRes == VOS_ERROR )   
		{
			ASSERT( 0 );
#ifdef STATS_DEBUG
			sys_console_printf("|--ERROR at msgQReceive\r\n");
#endif
			break;
		}
		
		pLpMsg = (LOOP_msg_t *)aulMsg[3];
		paramType = pLpMsg->paramType;
		
		switch(paramType)
		{
		case LOOP_SET_MODE:

			
#ifdef LOOP_DEBUG
			sys_console_printf(" LOOP_SET_MODE!\r\n");
#endif			


			OnuLoopPhyLinkModeSet(	pLpMsg->oltId,
				pLpMsg->onuId,
				pLpMsg->loopbackMode,
				pLpMsg->loopbackType,
				pLpMsg->onuPhyLoopSpeed,
				pLpMsg->loopFrameLen,
				pLpMsg->OnuLpbSource, 
				pLpMsg->OnuLpbTime,
				pLpMsg->OnuLpbCtrl);
			/*OnuLoopPhyLinkModeSet(pLpMsg);*/
			
			break;
		case LOOP_LINK_TEST:
			lRes = OnuLoopPhyLinkTest();
			
			break;
			
		default : 
			break;
		}
#ifdef LOOP_DEBUG
		if (pLpMsg == NULL)
			sys_console_printf(" NULL!\r\n");
#endif				
		
		if (pLpMsg != NULL)
		{
			VOS_Free((VOID *)pLpMsg);
			pLpMsg = NULL;
		}
		aulMsg[0] = 0;
		aulMsg[1] = 0;
		aulMsg[2] = 0;
		aulMsg[3] = 0;			
	}
	return;
}

/**************************************************
* OnuLoopTask
* 	描述: onu环回测试模块任务定时小时函数，定时向测试任务发
*	超时消息
*
*
*
*
***************************************************/
int OnuLoopTimeOutSend(void)
{
	LONG 	lRes = LOOP_OK;
	LOOP_msg_t *pLpMsg = NULL;
	ULONG ulMsg[4];
	/*short int count = 0;*/
	if(TRUE != SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		return LOOP_ERR;	
	pLpMsg = (LOOP_msg_t *)VOS_Malloc((ULONG)sizeof(LOOP_msg_t), (ULONG)MODULE_LOOPBACK);
	if (NULL == pLpMsg)
		return LOOP_ERR;
	memset(pLpMsg, 0, sizeof(LOOP_msg_t));
	pLpMsg->paramType = LOOP_LINK_TEST;
	ulMsg[3] = (ULONG)pLpMsg;
	
	lRes = VOS_QueSend(OnuLoopQueue, ulMsg, NO_WAIT,MSG_PRI_NORMAL);
	
	if (VOS_ERROR == lRes)
	{
		ASSERT( 0 );
#ifdef	LOOP_DEBUG		
		sys_console_printf("VOS_Quesend ERROR!\r\n");
#endif
		VOS_Free((VOID *)pLpMsg);
		pLpMsg = NULL;
		return lRes;
	}
	
	return lRes;
	/*looperrorCode = LOOP_TIMEOUT_ERR;
	while((looperrorCode != LOOP_OK))
	{
	taskDelay(20);
	if (count>=3)
				{
				return looperrorCode;
				}
				count++;
				}
	return looperrorCode;*/
	
}

int OnuLoopPhyLinkModeSet(short int ponId,
						  short int onuId,
						  short int loopBackMode,
						  short int loopBackType,
						  short int loopSpeedLimited,
						  short int loopFrameLen,
						  short int onuLpbSource,
						  unsigned int onuLpbTime,
						  short int OnuLpbCtrl
						  )
{
	/*LONG 	lRes = LOOP_OK;*/
	short int llid = 0;	
	short int macMode = 0;
	macMode = (short int)PAS_PON_MAC;
	
	if ((ponId>(LOOP_MAX_PON_NUM-1)) || (onuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}

	
	if (OnuLpbCtrl == LOOP_START)
	{

		int i = 0;
		for( ; i< LOOP_MAX_ONU_NUM; i++ )
			if( pgLoopOnuInfo[ponId][i]->loopEn )
				break;

		
		/*已经设置了环回模式，不能再设置
		if (TRUE == pgLoopOnuInfo[ponId][onuId]->loopEn)*/

		if( i!=LOOP_MAX_ONU_NUM )
		{
			
#ifdef LOOP_DEBUG
			sys_console_printf("  %% pgLoopOnuInfo[%d]->loopEn is true\r\n",ponId);
#endif
			looperrorCode = LOOP_ONULOOP_EXIST;
			return looperrorCode;
		}
		else
		{
			llid = GetLlidByOnuIdx( ponId, onuId);
			if ( LOOP_INVALID_LLID == llid )
			{
				looperrorCode = LOOP_DEVICE_INVALUE;
				return looperrorCode;
			}
			
			
			memset(pgLoopStat[ponId][onuId], 0, sizeof(LOOP_PON_link_test_results_t));
			
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_START;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbSource = onuLpbSource;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = (unsigned int)onuLpbTime;
			pgLoopOnuInfo[ponId][onuId]->loopLinkSpeed =loopSpeedLimited;
			pgLoopOnuInfo[ponId][onuId]->loopMode = loopBackMode;
			pgLoopOnuInfo[ponId][onuId]->loopType = loopBackType;
			pgLoopOnuInfo[ponId][onuId]->loopFrameLen = loopFrameLen;
			pgLoopOnuInfo[ponId][onuId]->onuId = onuId;
			pgLoopOnuInfo[ponId][onuId]->loopEn = TRUE;
#ifdef LOOP_DEBUG
			sys_console_printf(" LOOP set enable succed !\r\n");
#endif				
			looperrorCode = LOOP_OK;
		}
	}
	else if (LOOP_STOP == OnuLpbCtrl)
	{
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_STOP;
		/*modified by wutw at august seventeen*/
		/*pgLoopOnuInfo[ponId][OnuId]->loopEn = FALSE;	*/
		looperrorCode = LOOP_OK;
		
		/* modified by chenfj 2007-12-20
		ONU 环回过程中(未到约定时间)，此时通过CLI 停止环回，会导致原来的环回未清除，此后再也不能对此PON口下的任何ONU 设置环回*/
		if ( FALSE == pgLoopOnuInfo[ponId][onuId]->loopEn )
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
		
	}	
	return LOOP_OK;
}
/*该函数为内部函数*/
int OnuLoopPhyLinkModeSet_1(LOOP_msg_t *pLpMsg)
{
	LONG 	lRes = LOOP_OK;
	short int 	ponId = 0;
	short int 	onuId = 0;
	short int llid = 0;	
	/*short int tempPonIf = 0;*/
	
	/*param for onu pon loopmode setting*/
	short int OnuLpbCtrl = 0;
	short int macMode = 0;
	short int loopBackType = 0;
	short int loopBackMode = 0;	
	short int loopSpeedLimited = 0;
	short int loopFrameLen = 0;
	
	short int onuLpbSource = 0;
	short int onuLpbTime = 0;
	if (NULL == pLpMsg)
	{
		looperrorCode = LOOP_ERR;
		return LOOP_ERR;
	}
	ponId = pLpMsg->oltId;
	onuId = pLpMsg->onuId;
	if ((ponId>(LOOP_MAX_PON_NUM-1)) || (onuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}		
	macMode = (short int)PAS_PON_MAC;
	loopBackMode = pLpMsg->loopbackMode;
	loopBackType = (short int)pLpMsg->loopbackType;
	loopSpeedLimited = (short int)pLpMsg->onuPhyLoopSpeed;
	loopFrameLen = pLpMsg->loopFrameLen;
	onuLpbSource = pLpMsg->OnuLpbSource; 
	onuLpbTime = pLpMsg->OnuLpbTime;
	OnuLpbCtrl = pLpMsg->OnuLpbCtrl;
	if (OnuLpbCtrl == LOOP_START)
	{
		/*已经设置了环回模式，不能再设置*/
		if (TRUE == pgLoopOnuInfo[ponId][onuId]->loopEn)
		{
			looperrorCode = LOOP_PARAM_ERR;
			return looperrorCode;
		}
		else
		{
			llid = GetLlidByOnuIdx( ponId, onuId);
			if ( LOOP_INVALID_LLID == llid )
				return LOOP_ERR;
			/*loopStatsMsgOnuOff(ponId, onuId);*/
			lRes = PAS_set_test_mode_v4( ponId,  llid, (PON_mac_test_modes_t)macMode, loopBackMode,  
				(PON_loopback_t)loopBackType, 
				(PON_link_rate_t)loopSpeedLimited );
			if (LOOP_OK != lRes)
				looperrorCode = LOOP_ERR;
				/*pgLoopOnuInfo[ponId][OnuId] = (LOOP_Pon_link_info* )VOS_Malloc((ULONG)sizeof(LOOP_Pon_link_info), (ULONG)MODULE_LOOPBACK);
				if (NULL == pgLoopOnuInfo[ponId][OnuId])
				{
				looperrorCode = LOOP_GETMEM_ERR;
				return looperrorCode;
				}
				memset(pgLoopOnuInfo[ponId][OnuId], 0, sizeof(LOOP_Pon_link_info));
			memset(pgLoopStat[ponId][onuId], 0, sizeof(LOOP_PON_link_test_results_t));*/
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_START;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbSource = onuLpbSource;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = onuLpbTime;
			pgLoopOnuInfo[ponId][onuId]->loopLinkSpeed =loopSpeedLimited;
			pgLoopOnuInfo[ponId][onuId]->loopMode = loopBackMode;
			pgLoopOnuInfo[ponId][onuId]->loopType = loopBackType;
			pgLoopOnuInfo[ponId][onuId]->loopFrameLen = loopFrameLen;
			pgLoopOnuInfo[ponId][onuId]->onuId = onuId;
			pgLoopOnuInfo[ponId][onuId]->loopEn = TRUE;
#ifdef LOOP_DEBUG
			sys_console_printf(" LOOP set enable succed !\r\n");
#endif
		}
	}
	else if (LOOP_NOOP == OnuLpbCtrl)
	{
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_STOP;
		/*modified by wutw at august seventeen*/
		pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;		
#if 0
		if (FALSE == pgLoopOnuInfo[ponId][onuId])
			looperrorCode = LOOP_ERR;
		
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ( LOOP_INVALID_LLID == llid )
		{
			looperrorCode = LOOP_ERR;
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;	
			return LOOP_ERR;
		}
		/*loopStatsMsgOnuOn(ponId, onuId);*/
		lRes = PAS_set_test_mode_v4( ponId,  llid, macMode, loopBackMode,  loopBackType, loopSpeedLimited );
		if (LOOP_OK != lRes)
		{
			looperrorCode = LOOP_ERR;
			return looperrorCode;
		}
		/*modified by wutw at august seventeen*/
		pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;			
		/*if (NULL != pgLoopOnuInfo[ponId][onuId])
		{
		VOS_Free((VOID *) pgLoopOnuInfo[ponId][onuId]);
		pgLoopOnuInfo[ponId][onuId] = NULL;
				}*/
#ifdef LOOP_DEBUG
		sys_console_printf(" LOOP set disablesucced !\r\n");
#endif							
#endif
	}	
	return LOOP_OK;
}


/******************************************************************
*
*
*
*
*
*
*******************************************************************/
int OnuLoopPhyLinkTest(void)
{
	short int PonId = 0;
	short int OnuId = 0;
	int iRes = 0;
	for(PonId = 0; PonId < LOOP_MAX_PON_NUM; PonId ++)
	{
		for(OnuId = 0;OnuId < LOOP_MAX_ONU_NUM; OnuId++)
		{
			if (FALSE == pgLoopOnuInfo[PonId][OnuId]->loopEn)
				continue;
			/* modified by chenfj 2007-12-20
			先启动一个ONUx PON口环回，(未到约定时间)再接着启动同一PON口下的另一个ONUy PON口环回(y < x）;
			会造成ONUx 的环回不能停止，ONUy的环回不能启动 
			*/
			if (pgLoopOnuInfo[PonId][OnuId]->OnuLpbSource == LOOP_INTERNAL)
			{
				OnuLoopIntLimitTime(PonId, pgLoopOnuInfo[PonId][OnuId]->onuId, pgLoopOnuInfo[PonId][OnuId]->loopFrameLen);
				/*if (pgLoopOnuInfo[PonId][OnuId]->OnuLpbTime != LOOP_ALLTIME)
				iRes = OnuLoopIntLimitTime(PonId, pgLoopOnuInfo[PonId][OnuId]->onuId, pgLoopOnuInfo[PonId][OnuId]->loopFrameLen);
				else
				iRes = OnuLoopIntAllTime(PonId, pgLoopOnuInfo[PonId][OnuId]->onuId, pgLoopOnuInfo[PonId][OnuId]->loopFrameLen);
				*/
				break;
			}
			else if (pgLoopOnuInfo[PonId][OnuId]->OnuLpbSource == LOOP_EXTERNAL)
			{
				OnuLoopExtLimitTime(PonId, pgLoopOnuInfo[PonId][OnuId]->onuId );
				/*读取统计数据*/
				/*if (pgLoopOnuInfo[PonId][OnuId]->OnuLpbTime != LOOP_ALLTIME)
				iRes = OnuLoopExtLimitTime(PonId, pgLoopOnuInfo[PonId][OnuId]->onuId );
				else
				iRes = OnuLoopExtAllTime(PonId, pgLoopOnuInfo[PonId][OnuId]->onuId);
				*/
				break;
			}
			else
				continue;
			/*return LOOP_ERR;*/
			if (iRes != LOOP_OK)
				continue;
		}
	}
	/*读统计数据*/
	return LOOP_OK;
}


int OnuLoopExtAllTime(short int ponId, short int onuId)
{
	
	PON_link_test_vlan_configuration_t vlan_configuratiion;
	PON_link_test_results_t  test_results;
	long lRes = LOOP_OK;
	short int llid =0;
	short int frame_size = 0;
	if ((ponId>(LOOP_MAX_PON_NUM-1)) || (onuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}
	
	vlan_configuratiion.vlan_frame_enable = FALSE;
	vlan_configuratiion.vlan_priority = 0;
	vlan_configuratiion.vlan_tag = 0;
	llid = GetLlidByOnuIdx( ponId, onuId);
	if ( LOOP_INVALID_LLID == llid )
		return LOOP_ERR;
	VOS_MemSet(&test_results, 0, (LONG)sizeof(LOOP_PON_link_test_results_t));
	#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	lRes = OLT_GetOnuMode( ponId, llid );
	#else
	lRes = PAS_get_onu_mode( ponId, llid );
	#endif
	if( lRes != (long)PON_ONU_MODE_ON )
	{
#ifdef LOOP_DEBUG				
		sys_console_printf(" Pon %d onu %d is Off Line, so can't have a MAC test \r\n");
#endif
		return( lRes );
	}
	
	
	
	
	/*1) 如果为alltime 时间参数,以下步骤:先停止环回,读取数据,再启动环回,
	直到下一个2s,再重复该步骤;
	2) 如果为第一次环回(start状态), 则仅开始环回,并改变该环回处理状态为process,
	在下一个2s到来时,开始以上1)的处理步骤;
	3)如果为非alltime,则在2)时要进行
	a) 如果为非alltime,则其时间参数需要减去2s;
	b) 如果为非alltime,检测改时间参数小于2s,则改变环回处理状态为stop,并
	继续等待下一个2s
	
	4) 如果检测到环回处理状态为stop则读取数据,并修改环回状态为noon*/
	
	if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_START)
	{/* 2) */
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_PROCESS;
		memset(pgLoopStat[ponId][onuId], 0, sizeof(LOOP_PON_link_test_results_t));
		/*读当前统计计数*/
		/*STATS_ONULOOPBACK_RXOK:收到的ok数据包
		STATS_ONULOOPBACK_TXOK:发出的ok的数据包
		STATS_ONULOOPBACK_RXERR:收到的错误数据包
STATS_ONULOOPBACK_TOTALRXBAD:收到的总的坏的数据包*/
/*#ifdef LOOP_DEBUG
pgLoopStat[ponId][onuId]->BaseNumberOfReturnedFrames = 0xf;
pgLoopStat[ponId][onuId]->BaseNumberOfSentFrames = 0xf;
pgLoopStat[ponId][onuId]->BaseNumberOfErroredReturnFrames = 0xf;
pgLoopStat[ponId][onuId]->BaseNumberOfBadReturnFrames = 0xf;
#else*/
		{
			unsigned long  StatsData = 0;
			/*short int StatsObjType = LOOP_ONULOOPBACK_RXOK;*/
			
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_RXOK, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->BaseNumberOfReturnedFrames = StatsData;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_TXOK;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_TXOK, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->BaseNumberOfSentFrames = StatsData;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_RXERR;*/
			rtStatsOnuDataGet(LOOP_ONULOOPBACK_RXERR, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->BaseNumberOfErroredReturnFrames = StatsData;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_TOTALRXBAD;*/
			rtStatsOnuDataGet(LOOP_ONULOOPBACK_TOTALRXBAD, ponId, onuId, &StatsData);	
			pgLoopStat[ponId][onuId]->BaseNumberOfBadReturnFrames = StatsData;
		}
		/*#endif*/
		return lRes;
	}
	/* 1) */
	else if ((pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_PROCESS) )
	{
	/*#ifdef LOOP_DEBUG
	pgLoopStat[ponId][onuId]->numberOfReturnedFrames += pgLoopStat[ponId][onuId]->BaseNumberOfReturnedFrames;
	pgLoopStat[ponId][onuId]->numberOfSentFrames += pgLoopStat[ponId][onuId]->BaseNumberOfSentFrames;
	pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames += pgLoopStat[ponId][onuId]->BaseNumberOfErroredReturnFrames;
	pgLoopStat[ponId][onuId]->numberOfBadReturnFrames += pgLoopStat[ponId][onuId]->BaseNumberOfBadReturnFrames;
#else*/
		{
			unsigned long  StatsData = 0;
			/*short int StatsObjType = LOOP_ONULOOPBACK_RXOK;*/
			
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_RXOK, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->numberOfReturnedFrames = StatsData - pgLoopStat[ponId][onuId]->BaseNumberOfReturnedFrames;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_TXOK;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_TXOK, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->numberOfSentFrames = StatsData -pgLoopStat[ponId][onuId]->BaseNumberOfSentFrames;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_RXERR;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_RXERR, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames = StatsData - pgLoopStat[ponId][onuId]->BaseNumberOfErroredReturnFrames;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_TOTALRXBAD;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_TOTALRXBAD, ponId, onuId, &StatsData);	
			pgLoopStat[ponId][onuId]->numberOfBadReturnFrames = StatsData - pgLoopStat[ponId][onuId]->BaseNumberOfBadReturnFrames;
		}		
		/*#endif	*/
#ifdef LOOP_DEBUG
		sys_console_printf(" iRes is LOOP_PON_LINK_TEST_CONTINUOUS!\r\n");
		sys_console_printf("\r\nPon %d onu %d Link Test ok \r\n", ponId, onuId);
		sys_console_printf("   Total frames Send: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
		sys_console_printf("   Total frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfReturnedFrames);
		sys_console_printf("   Error frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames );
		sys_console_printf("   Minimal delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->minimalDelay);
		sys_console_printf("   Maximal delay: %d(TQ) \r\n", pgLoopStat[ponId][onuId]->maximalDelay);
		sys_console_printf("   Mean delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->meanDelay);
#endif
		return lRes;
	}
	/* 4) */
	else if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_STOP)
	{
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest( ponId, llid,  LOOP_PON_LINK_TEST_OFF, frame_size, TRUE, &vlan_configuratiion, &test_results );
		if (LOOP_OK != lRes)
		{
			return lRes;
		}	
		/*#ifdef LOOP_DEBUG
		pgLoopStat[ponId][onuId]->numberOfReturnedFrames += pgLoopStat[ponId][onuId]->BaseNumberOfReturnedFrames;
		pgLoopStat[ponId][onuId]->numberOfSentFrames += pgLoopStat[ponId][onuId]->BaseNumberOfSentFrames;
		pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames += pgLoopStat[ponId][onuId]->BaseNumberOfErroredReturnFrames;
		pgLoopStat[ponId][onuId]->numberOfBadReturnFrames += pgLoopStat[ponId][onuId]->BaseNumberOfBadReturnFrames;
#else*/
		{/*获取环回的统计数据*/
			unsigned long  StatsData = 0;
			/*short int StatsObjType = LOOP_ONULOOPBACK_RXOK;*/
			
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_RXOK, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->numberOfReturnedFrames = StatsData - pgLoopStat[ponId][onuId]->BaseNumberOfReturnedFrames;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_TXOK;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_TXOK, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->numberOfSentFrames = StatsData -pgLoopStat[ponId][onuId]->BaseNumberOfSentFrames;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_RXERR;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_RXERR, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames = StatsData - pgLoopStat[ponId][onuId]->BaseNumberOfErroredReturnFrames;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_TOTALRXBAD;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_TOTALRXBAD, ponId, onuId, &StatsData);	
			pgLoopStat[ponId][onuId]->numberOfBadReturnFrames = StatsData - pgLoopStat[ponId][onuId]->BaseNumberOfBadReturnFrames;
		}	
		/*#endif	*/
#ifdef LOOP_DEBUG
		sys_console_printf(" iRes is LOOP_PON_LINK_TEST_OFF!\r\n"); 	
		sys_console_printf("\r\nPon %d onu %d Link Test ok \r\n", ponId, onuId);
		sys_console_printf("   Total frames Send: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
		sys_console_printf("   Total frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfReturnedFrames);
		sys_console_printf("   Error frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames );
		sys_console_printf("   Minimal delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->minimalDelay);
		sys_console_printf("   Maximal delay: %d(TQ) \r\n", pgLoopStat[ponId][onuId]->maximalDelay);
		sys_console_printf("   Mean delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->meanDelay);
#endif
		
		{/*停止phy环回设置，重设为正常状态*/
			if (FALSE == pgLoopOnuInfo[ponId][onuId]->loopEn)
				looperrorCode = LOOP_ERR;
			
			llid = GetLlidByOnuIdx( ponId, onuId);
			if ( LOOP_INVALID_LLID == llid )
			{
				pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;	
				return LOOP_ERR;	
			}
			/*loopStatsMsgOnuOn(ponId, onuId);*/
			lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC/*LOOP_PON_MAC*/, LOOP_OFF,  
				PON_LOOPBACK_PHY/*LOOP_LOOPBACK_PHY*/, 
				PON_100M/*LOOP_PON_100M*/ );
			if (LOOP_OK != lRes)
			{
				looperrorCode = LOOP_ERR;
				
			}
			/*modified by wutw at august seventeen*/
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;	
			return looperrorCode;
			/*if (NULL != pgLoopOnuInfo[ponId][onuId])
			{
			VOS_Free((VOID *) pgLoopOnuInfo[ponId][onuId]);
			pgLoopOnuInfo[ponId][onuId] = NULL;
		}*/
#ifdef LOOP_DEBUG
			sys_console_printf(" LOOP set disablesucced !\r\n");
#endif							
		}
		
	}
	
	return( LOOP_OK);
}

int OnuLoopIntAllTime(short int ponId, short int onuId ,short int frame_size)
{
	
	PON_link_test_vlan_configuration_t vlan_configuratiion;
	PON_link_test_results_t  test_results;
	long lRes = LOOP_OK;
	short int llid =0;
	
	if ((ponId>(LOOP_MAX_PON_NUM-1)) || (onuId > (LOOP_MAX_ONU_NUM-1)))
		
	{
		return LOOP_DEVICE_INVALUE;
	}
	llid = GetLlidByOnuIdx( ponId, onuId);
	if ( LOOP_INVALID_LLID == llid )
	{/*当处于环回测试时，onu 发生deregister，进行相应的处理*/
		/*1. */
		if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_START)
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
		}
		else if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_STOP)
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
		}
		return LOOP_ERR;
	}
	
	vlan_configuratiion.vlan_frame_enable = FALSE;
	vlan_configuratiion.vlan_priority = 0;
	vlan_configuratiion.vlan_tag = 0;
	
	memset(&test_results, 0, sizeof(LOOP_PON_link_test_results_t));
	/* Range: MIN_ETHERNET_FRAME_SIZE - MAX_ETHERNET_FRAME_SIZE_STANDARDIZED */
	if ((frame_size < LOOP_MIN_ETHERNET_FRAME_SIZE) || (frame_size>LOOP_MAX_ETHERNET_FRAME_SIZE_STANDARDIZED))
		return LOOP_ERR;
	#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	lRes = OLT_GetOnuMode( ponId, llid );
	#else
	lRes = PAS_get_onu_mode( ponId, llid );
	#endif
	if( lRes != PON_ONU_MODE_ON )
	{
#ifdef LOOP_DEBUG				
		sys_console_printf(" Pon %d onu %d is Off Line, so can't have a MAC test \r\n");
#endif
		pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
		return( lRes );
	}
	
	/*1) 如果为alltime 时间参数,以下步骤:先停止环回,读取数据,再启动环回,
	直到下一个2s,再重复该步骤;
	2) 如果为第一次环回(start状态), 则仅开始环回,并改变该环回处理状态为process,
	在下一个2s到来时,开始以上1)的处理步骤;
	3)如果为非alltime,则在2)时要进行
	a) 如果为非alltime,则其时间参数需要减去2s;
	b) 如果为非alltime,检测改时间参数小于2s,则改变环回处理状态为stop,并
	继续等待下一个2s
	
	4) 如果检测到环回处理状态为stop则读取数据,并修改环回状态为noon*/
	
	if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_START)
	{/* 2) */
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_PROCESS;
		memset(pgLoopStat[ponId][onuId], 0, sizeof(LOOP_PON_link_test_results_t));
#ifdef LOOP_DEBUG				
		sys_console_printf(" iRes is LOOP_PON_LINK_TEST_CONTINUOUS!\r\n");
#endif		
        /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest ( ponId, llid,  LOOP_PON_LINK_TEST_CONTINUOUS, frame_size, TRUE, &vlan_configuratiion, &test_results );
		if (LOOP_OK != lRes)
		{
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;	
			lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, LOOP_OFF,  PON_LOOPBACK_PHY, PON_100M );
			if (LOOP_OK != lRes)
			{
#ifdef LOOP_DEBUG
				sys_console_printf(" LOOP_NOOP\r\n");
#endif
			}		
			sys_console_printf(" LOOP_PON_LINK_TEST_CONTINUOUS ERROR!\r\n");
			return lRes;
		}
		return lRes;
	}
	/* 1) */
	else if ((pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_PROCESS) )
	{
	    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest ( ponId, llid,  LOOP_PON_LINK_TEST_OFF, frame_size, TRUE, &vlan_configuratiion, &test_results );
		if (LOOP_OK != lRes)
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
			
			{/*停止phy环回设置，重设为正常状态*/
				if (FALSE == pgLoopOnuInfo[ponId][onuId]->loopEn)
					looperrorCode = LOOP_ERR;
				/*loopStatsMsgOnuOn(ponId, onuId);*/
				lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, LOOP_OFF,  PON_LOOPBACK_PHY, PON_100M );
				if (LOOP_OK != lRes)
				{
#ifdef LOOP_DEBUG
					sys_console_printf(" LOOP_NOOP\r\n");
#endif
					looperrorCode = LOOP_ERR;
					
				}
				/*modified by wutw at august seventeen*/
				pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
				return looperrorCode;
				
#ifdef LOOP_DEBUG
				sys_console_printf(" LOOP set disablesucced !\r\n");
#endif							
			}				
		}	
		
		if (NULL != pgLoopStat[ponId][onuId])
		{
			pgLoopStat[ponId][onuId]->numberOfBadReturnFrames += 0;
			pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames += test_results.number_of_errored_return_frames;
			pgLoopStat[ponId][onuId]->numberOfSentFrames += test_results.number_of_sent_frames;
			pgLoopStat[ponId][onuId]->numberOfReturnedFrames += test_results.number_of_returned_frames;
			pgLoopStat[ponId][onuId]->maximalDelay = test_results.maximal_delay;
			pgLoopStat[ponId][onuId]->meanDelay = test_results.mean_delay;
			pgLoopStat[ponId][onuId]->minimalDelay = test_results.minimal_delay;
		}
#ifdef LOOP_DEBUG		
		sys_console_printf("\r\nPon %d onu %d Link Test ok \r\n", ponId, onuId);
		sys_console_printf("   Total frames Send: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
		sys_console_printf("   Total frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfReturnedFrames);
		sys_console_printf("   Error frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames );
		sys_console_printf("   Minimal delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->minimalDelay);
		sys_console_printf("   Maximal delay: %d(TQ) \r\n", pgLoopStat[ponId][onuId]->maximalDelay);
		sys_console_printf("   Mean delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->meanDelay);
#endif
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest ( ponId, llid,  LOOP_PON_LINK_TEST_CONTINUOUS, frame_size, TRUE, &vlan_configuratiion, &test_results );
		
		if (LOOP_OK != lRes)
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
			{/*停止phy环回设置，重设为正常状态*/
				if (FALSE == pgLoopOnuInfo[ponId][onuId]->loopEn)
					looperrorCode = LOOP_ERR;
				
				llid = GetLlidByOnuIdx( ponId, onuId);
				if ( LOOP_INVALID_LLID == llid )
				{
					looperrorCode = LOOP_ERR;
					pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
					return LOOP_ERR;
				}
				/*loopStatsMsgOnuOn(ponId, onuId);*/
				lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, LOOP_OFF, 
					PON_LOOPBACK_PHY, PON_100M );
				if (LOOP_OK != lRes)
				{
#ifdef LOOP_DEBUG
					sys_console_printf(" LOOP_NOOP\r\n");
#endif
					looperrorCode = LOOP_ERR;
					
				}
				/*modified by wutw at august seventeen*/
				pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
				return looperrorCode;
				/*if (NULL != pgLoopOnuInfo[ponId][onuId])
				{
				VOS_Free((VOID *) pgLoopOnuInfo[ponId][onuId]);
				pgLoopOnuInfo[ponId][OnuId] = NULL;
			}*/
#ifdef LOOP_DEBUG
				sys_console_printf(" LOOP set disablesucced !\r\n");
#endif							
			}			
#ifdef LOOP_DEBUG			
			sys_console_printf(" LOOP_PON_LINK_TEST_CONTINUOUS ERROR!\r\n");
#endif
		}	
		
		return lRes;
	}
	/* 4) */
	else if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_STOP)
	{
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;	
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest ( ponId, llid,  LOOP_PON_LINK_TEST_OFF, frame_size, TRUE, &vlan_configuratiion, &test_results );
		
		if (LOOP_OK != lRes)
		{
#ifdef LOOP_DEBUG				
			sys_console_printf(" 0. ERROR set pas_link_test!\r\n");
#endif
			return lRes;
		}	
		
		if (NULL != pgLoopStat[ponId][onuId])
		{
			pgLoopStat[ponId][onuId]->numberOfBadReturnFrames += 0;
			pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames += test_results.number_of_errored_return_frames;
			pgLoopStat[ponId][onuId]->numberOfSentFrames += test_results.number_of_sent_frames;
			pgLoopStat[ponId][onuId]->numberOfReturnedFrames += test_results.number_of_returned_frames;
			pgLoopStat[ponId][onuId]->maximalDelay = test_results.maximal_delay;
			pgLoopStat[ponId][onuId]->meanDelay = test_results.mean_delay;
			pgLoopStat[ponId][onuId]->minimalDelay = test_results.minimal_delay;
		}
#ifdef LOOP_DEBUG		
		sys_console_printf("\r\nPon %d onu %d Link Test ok \r\n", ponId, onuId);
		sys_console_printf("   Total frames Send: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
		sys_console_printf("   Total frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfReturnedFrames);
		sys_console_printf("   Error frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames );
		sys_console_printf("   Minimal delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->minimalDelay);
		sys_console_printf("   Maximal delay: %d(TQ) \r\n", pgLoopStat[ponId][onuId]->maximalDelay);
		sys_console_printf("   Mean delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->meanDelay);
#endif
		
		{/*停止phy环回设置，重设为正常状态*/
			if (FALSE == pgLoopOnuInfo[ponId][onuId]->loopEn)
				looperrorCode = LOOP_ERR;
			
			llid = GetLlidByOnuIdx( ponId, onuId);
			if ( LOOP_INVALID_LLID == llid )
			{
				looperrorCode = LOOP_ERR;
				pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
				return LOOP_ERR;
			}
			/*loopStatsMsgOnuOn(ponId, onuId);*/
			lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, LOOP_OFF,  PON_LOOPBACK_PHY, PON_100M );
			if (LOOP_OK != lRes)
			{
				looperrorCode = LOOP_ERR;
				
			}
			/*modified by wutw at august seventeen*/
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;		
			return looperrorCode;
			/*if (NULL != pgLoopOnuInfo[ponId][onuId])
			{
			VOS_Free((VOID *) pgLoopOnuInfo[ponId][onuId]);
			pgLoopOnuInfo[ponId][onuId] = NULL;
		}*/
#ifdef LOOP_DEBUG
			sys_console_printf(" LOOP set disablesucced !\r\n");
#endif							
		}
	}
	return( LOOP_OK);
}

int OnuLoopExtLimitTime(short int ponId, short int onuId )
{
	
	PON_link_test_vlan_configuration_t vlan_configuratiion;
	PON_link_test_results_t  test_results;
	long lRes = LOOP_OK;
	short int llid =0;
	/*	short int frame_size = 0;*/
	if ((ponId>(LOOP_MAX_PON_NUM-1)) || (onuId > (LOOP_MAX_ONU_NUM-1)))
		
	{
		return LOOP_DEVICE_INVALUE;
	}
	llid = GetLlidByOnuIdx( ponId, onuId);
	if ( LOOP_INVALID_LLID == llid )
	{/*当处于环回测试时，onu 发生deregister，进行相应的处理*/
		/*1. */
		if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_START)
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
		}
		else if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_STOP)
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
		}
		return LOOP_ERR;
	}	
	vlan_configuratiion.vlan_frame_enable = FALSE;
	vlan_configuratiion.vlan_priority = 0;
	vlan_configuratiion.vlan_tag = 0;
	memset(&test_results, 0, sizeof(LOOP_PON_link_test_results_t));
	
	#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	lRes = OLT_GetOnuMode( ponId, llid );
	#else
	lRes = PAS_get_onu_mode( ponId, llid );
	#endif
	if( lRes != PON_ONU_MODE_ON )
	{
#ifdef LOOP_DEBUG			
		sys_console_printf(" Pon %d onu %d is Off Line, so can't have a MAC test \r\n");
#endif
		pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
		pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
		return( lRes );
	}
	
	
	
	
	/*1) 如果为alltime 时间参数,以下步骤:先停止环回,读取数据,再启动环回,
	直到下一个2s,再重复该步骤;
	2) 如果为第一次环回(start状态), 则仅开始环回,并改变该环回处理状态为process,
	在下一个2s到来时,开始以上1)的处理步骤;
	3)如果为非alltime,则在2)时要进行
	a) 如果为非alltime,则其时间参数需要减去2s;
	b) 如果为非alltime,检测改时间参数小于2s,则改变环回处理状态为stop,并
	继续等待下一个2s
	
	4) 如果检测到环回处理状态为stop则读取数据,并修改环回状态为noon*/
	
	if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_START)
	{/* 2) */
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_PROCESS;
		memset(pgLoopStat[ponId][onuId], 0, sizeof(LOOP_PON_link_test_results_t));
		/*#ifdef LOOP_DEBUG
		pgLoopStat[ponId][onuId]->BaseNumberOfReturnedFrames = 0xf;
		pgLoopStat[ponId][onuId]->BaseNumberOfSentFrames = 0xf;
		pgLoopStat[ponId][onuId]->BaseNumberOfErroredReturnFrames = 0xf;
		pgLoopStat[ponId][onuId]->BaseNumberOfBadReturnFrames = 0xf;
#else*/
		{
			unsigned long  StatsData = 0;
			/*short int StatsObjType = LOOP_ONULOOPBACK_RXOK;*/
			
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_RXOK, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->BaseNumberOfReturnedFrames = StatsData;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_TXOK;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_TXOK, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->BaseNumberOfSentFrames = StatsData;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_RXERR;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_RXERR, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->BaseNumberOfErroredReturnFrames = StatsData;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_TOTALRXBAD;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_TOTALRXBAD, ponId, onuId, &StatsData);	
			pgLoopStat[ponId][onuId]->BaseNumberOfBadReturnFrames = StatsData;
		}
		/*#endif		*/	
		if (pgLoopOnuInfo[ponId][onuId]->OnuLpbTime > LOOP_TIMEOUT)
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime -= LOOP_TIMEOUT;
		else
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_STOP;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
		}
#ifdef LOOP_DEBUG
		sys_console_printf(" iRes is LOOP_PON_LINK_TEST_CONTINUOUS!\r\n"); 	
		sys_console_printf("\r\nPon %d onu %d Link Test ok \r\n", ponId, onuId);
		sys_console_printf("   Total frames Send: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
		sys_console_printf("   Total frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfReturnedFrames);
		sys_console_printf("   Error frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames );
		sys_console_printf("   Minimal delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->minimalDelay);
		sys_console_printf("   Maximal delay: %d(TQ) \r\n", pgLoopStat[ponId][onuId]->maximalDelay);
		sys_console_printf("   Mean delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->meanDelay);
#endif	
		
#ifdef LOOP_DEBUG
		sys_console_printf(" iRes is LOOP_PON_LINK_TEST_CONTINUOUS!\r\n");
#endif		 
		return lRes;
	}
	/* 1) */
	else if ((pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_PROCESS) )
	{
		{
			unsigned long  StatsData = 0;
			/*short int StatsObjType = LOOP_ONULOOPBACK_RXOK;*/
			
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_RXOK, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->numberOfReturnedFrames = StatsData - pgLoopStat[ponId][onuId]->BaseNumberOfReturnedFrames;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_TXOK;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_TXOK, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->numberOfSentFrames = StatsData -pgLoopStat[ponId][onuId]->BaseNumberOfSentFrames;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_RXERR;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_RXERR, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames = StatsData - pgLoopStat[ponId][onuId]->BaseNumberOfErroredReturnFrames;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_TOTALRXBAD;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_TOTALRXBAD, ponId, onuId, &StatsData);	
			pgLoopStat[ponId][onuId]->numberOfBadReturnFrames = StatsData - pgLoopStat[ponId][onuId]->BaseNumberOfBadReturnFrames;
		}	
		/*#endif*/
		if (pgLoopOnuInfo[ponId][onuId]->OnuLpbTime > LOOP_TIMEOUT)
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime -= LOOP_TIMEOUT;
		else
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_STOP;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
		}
#ifdef LOOP_DEBUG
		sys_console_printf(" iRes is LOOP_PON_LINK_TEST_CONTINUOUS!\r\n"); 	
		sys_console_printf("\r\nPon %d onu %d Link Test ok \r\n", ponId, onuId);
		sys_console_printf("   Total frames Send: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
		sys_console_printf("   Total frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfReturnedFrames);
		sys_console_printf("   Error frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames );
		sys_console_printf("   Minimal delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->minimalDelay);
		sys_console_printf("   Maximal delay: %d(TQ) \r\n", pgLoopStat[ponId][onuId]->maximalDelay);
		sys_console_printf("   Mean delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->meanDelay);
#endif	
		return lRes;
	}
	/* 4) */
	else if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_STOP)
	{
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
		pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
		pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
		
#ifdef LOOP_DEBUG
		sys_console_printf(" iRes is LOOP_PON_LINK_TEST_OFF!\r\n");
#endif		
		
#if 0
		/*loopStatsMsgOnuOn(ponId, onuId);*/
		lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, LOOP_OFF,  PON_LOOPBACK_PHY, PON_100M );
		if (LOOP_OK != lRes)
		{
#ifdef LOOP_DEBUG
			sys_console_printf(" LOOP_NOOP\r\n");
#endif		
		}
#endif
#ifdef LOOP_DEBUG
		sys_console_printf(" LOOP set disablesucced !\r\n");
#endif							
		
		/*#ifdef LOOP_DEBUG
		pgLoopStat[ponId][onuId]->numberOfReturnedFrames += pgLoopStat[ponId][onuId]->BaseNumberOfReturnedFrames;
		pgLoopStat[ponId][onuId]->numberOfSentFrames += pgLoopStat[ponId][onuId]->BaseNumberOfSentFrames;
		pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames += pgLoopStat[ponId][onuId]->BaseNumberOfErroredReturnFrames;
		pgLoopStat[ponId][onuId]->numberOfBadReturnFrames += pgLoopStat[ponId][onuId]->BaseNumberOfBadReturnFrames;
#else*/
		{
			unsigned long  StatsData = 0;
			/*short int StatsObjType = LOOP_ONULOOPBACK_RXOK;*/
			
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_RXOK, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->numberOfReturnedFrames = StatsData - pgLoopStat[ponId][onuId]->BaseNumberOfReturnedFrames;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_TXOK;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_TXOK, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->numberOfSentFrames = StatsData -pgLoopStat[ponId][onuId]->BaseNumberOfSentFrames;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_RXERR;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_RXERR, ponId, onuId, &StatsData);
			pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames = StatsData - pgLoopStat[ponId][onuId]->BaseNumberOfErroredReturnFrames;
			StatsData = 0;
			/*StatsObjType = LOOP_ONULOOPBACK_TOTALRXBAD;*/
			rtStatsOnuDataGet( LOOP_ONULOOPBACK_TOTALRXBAD, ponId, onuId, &StatsData);	
			pgLoopStat[ponId][onuId]->numberOfBadReturnFrames = StatsData - pgLoopStat[ponId][onuId]->BaseNumberOfBadReturnFrames;
		}
		/*#endif*/
#ifdef LOOP_DEBUG
		sys_console_printf("   Total frames Send: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
		sys_console_printf("   Total frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfReturnedFrames);
		sys_console_printf("   Error frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames ); 
#endif
	}
	
	{/*停止phy环回设置，重设为正常状态*/
		if (FALSE == pgLoopOnuInfo[ponId][onuId]->loopEn)
			looperrorCode = LOOP_ERR;
		
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ( LOOP_INVALID_LLID == llid )
		{
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;	
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
			looperrorCode = LOOP_ERR;
			return looperrorCode;
		}				
		/*loopStatsMsgOnuOn(ponId, onuId);*/
		lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, LOOP_OFF,  PON_LOOPBACK_PHY, PON_100M );
		if (LOOP_OK != lRes)
		{
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;	
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
			looperrorCode = LOOP_ERR;
			return looperrorCode;
		}
		/*modified by wutw*/
		pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
		pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
		/*if (NULL != pgLoopOnuInfo[ponId][OnuId])
		{
		VOS_Free((VOID *) pgLoopOnuInfo[ponId][OnuId]);
		pgLoopOnuInfo[ponId][OnuId] = NULL;
				}*/
#ifdef LOOP_DEBUG
		sys_console_printf(" LOOP set disablesucced !\r\n");
#endif	
		return looperrorCode;
	}	
	/*return LOOP_OK;*/
}

int OnuLoopIntLimitTime(short int ponId, short int onuId ,short int frame_size)
{
	/*
	LOOP_PON_link_test_vlan_configuration_t vlan_configuratiion;
	LOOP_PON_link_test_results_t  test_results;
	*/
	PON_link_test_vlan_configuration_t vlan_configuratiion;
	PON_link_test_results_t  test_results;
	long lRes = LOOP_OK;
	short int llid =0;
	if ((ponId>(LOOP_MAX_PON_NUM-1)) || (onuId > (LOOP_MAX_ONU_NUM-1)))
	{
		return LOOP_DEVICE_INVALUE;
	}
	llid = GetLlidByOnuIdx( ponId, onuId);
	if ( LOOP_INVALID_LLID == llid )
	{/*当处于环回测试时，onu 发生deregister，进行相应的处理*/
		/*1. */
		if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_START)
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
		}
		else if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_STOP)
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
		}
		/* added by chenfj 2007-12-20
		处于环回测试时，onu 发生deregister，没有对LOOP_PROCESS 处理，造成别的ONU不能启动环回*/
		else if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_PROCESS)
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
		}
		
		return LOOP_ERR;
	}
	/*
	vlan_configuratiion.vlan_frame_enable = FALSE;
	vlan_configuratiion.vlan_priority = 0;
	vlan_configuratiion.vlan_tag = 0;
	*/
	memset(&vlan_configuratiion, 0, sizeof(PON_link_test_vlan_configuration_t));
	vlan_configuratiion.vlan_frame_enable = V2R1_ENABLE;
	vlan_configuratiion.vlan_priority = 7;
	vlan_configuratiion.vlan_tag = 10;
	memset(&test_results, 0, sizeof(PON_link_test_results_t));
	#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	lRes = OLT_GetOnuMode( ponId, llid );
	#else
	lRes = PAS_get_onu_mode( ponId, llid );
	#endif
	if( lRes != PON_ONU_MODE_ON )
	{
#ifdef LOOP_DEBUG				
		sys_console_printf(" Pon %d onu %d is Off Line, so can't have a MAC test \r\n");
#endif
		pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
		pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
		return( lRes );
	}
	
	
	
	
	/*1) 如果为alltime 时间参数,以下步骤:先停止环回,读取数据,再启动环回,
	直到下一个2s,再重复该步骤;
	2) 如果为第一次环回(start状态), 则仅开始环回,并改变该环回处理状态为process,
	在下一个2s到来时,开始以上1)的处理步骤;
	3)如果为非alltime,则在2)时要进行
	a) 如果为非alltime,则其时间参数需要减去2s;
	b) 如果为非alltime,检测改时间参数小于2s,则改变环回处理状态为stop,并
	继续等待下一个2s
	
	4) 如果检测到环回处理状态为stop则读取数据,并修改环回状态为noon*/
	
	if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_START)
	{/* 2) */
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_PROCESS;
		memset(pgLoopStat[ponId][onuId], 0, sizeof(LOOP_PON_link_test_results_t));
		
		/*if (pgLoopOnuInfo[ponId][onuId]->OnuLpbTime > LOOP_TIMEOUT)
		pgLoopOnuInfo[ponId][onuId]->OnuLpbTime -= LOOP_TIMEOUT;
		else
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_STOP;*/
		if (pgLoopOnuInfo[ponId][onuId]->OnuLpbTime < LOOP_TIMEOUT)
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_STOP;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
		}
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest ( ponId, llid,  LOOP_PON_LINK_TEST_CONTINUOUS, frame_size, TRUE, &vlan_configuratiion, &test_results );
		
		if (LOOP_OK != lRes)
		{
#ifdef LOOP_DEBUG	
			sys_console_printf(" LOOP_START/LOOP_PON_LINK_TEST_CONTINUOUS !\r\n");
#endif
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;	
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
			lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, LOOP_OFF,  
				PON_LOOPBACK_PHY, PON_100M );
			if (LOOP_OK != lRes)
			{
#ifdef LOOP_DEBUG
				sys_console_printf(" LOOP_NOOP\r\n");
#endif
			}				
		}
		return lRes;
	}
	/* 1) */
	else if ((pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_PROCESS) )
	{
		if (pgLoopOnuInfo[ponId][onuId]->OnuLpbTime > LOOP_TIMEOUT)
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime -= LOOP_TIMEOUT;
		else
		{
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_STOP;
			pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
		}
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest ( ponId, llid,  LOOP_PON_LINK_TEST_OFF, frame_size, TRUE, &vlan_configuratiion, &test_results );
		if (LOOP_OK != lRes)
		{	
#ifdef LOOP_DEBUG
			sys_console_printf("  %%ponId = %d   FALSE !\r\n", ponId);
			sys_console_printf("  %%llid %d  frame len %d\r\n",llid, frame_size);
			sys_console_printf("  %%LOOP_PROCESS/LOOP_PON_LINK_TEST_OFF !\r\n");
			sys_console_printf("  %% onuLpbTime = %d\r\n", pgLoopOnuInfo[ponId][onuId]->OnuLpbTime);
#endif
			
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
			{/*停止phy环回设置，重设为正常状态*/
				if (FALSE == pgLoopOnuInfo[ponId][onuId]->loopEn)
					return lRes;
				lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, LOOP_OFF,  
					PON_LOOPBACK_PHY, PON_100M );
				if (LOOP_OK != lRes)
				{
#ifdef LOOP_DEBUG
					sys_console_printf(" LOOP_NOOP\r\n");
#endif	
				}
				pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
				pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
				
#ifdef LOOP_DEBUG
				sys_console_printf(" LOOP set disablesucced !\r\n");
#endif							
			}
			
			return lRes;
		}	
		
		if (NULL != pgLoopStat[ponId][onuId])
		{
			pgLoopStat[ponId][onuId]->numberOfBadReturnFrames += 0;/*test_results.numberOfBadReturnFrames;*/
			pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames += test_results.number_of_errored_return_frames;
			pgLoopStat[ponId][onuId]->numberOfSentFrames += test_results.number_of_sent_frames;
			pgLoopStat[ponId][onuId]->numberOfReturnedFrames += test_results.number_of_returned_frames;
			pgLoopStat[ponId][onuId]->maximalDelay = test_results.maximal_delay;
			pgLoopStat[ponId][onuId]->meanDelay = test_results.mean_delay;
			pgLoopStat[ponId][onuId]->minimalDelay = test_results.minimal_delay;
		}
#ifdef LOOP_DEBUG		
		sys_console_printf("\r\nPon %d onu %d Link Test ok \r\n", ponId, onuId);
		sys_console_printf("   Total frames Send: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
		sys_console_printf("   Total frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfReturnedFrames);
		sys_console_printf("   Error frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames );
		sys_console_printf("   Minimal delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->minimalDelay);
		sys_console_printf("   Maximal delay: %d(TQ) \r\n", pgLoopStat[ponId][onuId]->maximalDelay);
		sys_console_printf("   Mean delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->meanDelay);
#endif	
			memset(&vlan_configuratiion, 0, sizeof(PON_link_test_vlan_configuration_t));
		vlan_configuratiion.vlan_frame_enable = V2R1_ENABLE;
		vlan_configuratiion.vlan_priority = 7;
		vlan_configuratiion.vlan_tag = 10;
		memset(&test_results, 0, sizeof(PON_link_test_results_t));
	    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest ( ponId, llid,  LOOP_PON_LINK_TEST_CONTINUOUS, frame_size, TRUE, &vlan_configuratiion, &test_results );
		if (LOOP_OK != lRes)
		{ 
			pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
			{/*停止phy环回设置，重设为正常状态*/
				if (FALSE == pgLoopOnuInfo[ponId][onuId]->loopEn)
					return lRes;
				
				lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, LOOP_OFF, 
					PON_LOOPBACK_PHY, PON_100M );
				if (LOOP_OK != lRes)
				{
#ifdef LOOP_DEBUG
					sys_console_printf(" LOOP_NOOP\r\n");
#endif
				}
				
				/*modified by wutw at august seventeen*/
				pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
				pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
				
#ifdef LOOP_DEBUG
				sys_console_printf(" LOOP set disablesucced !\r\n");
#endif							
			}
#ifdef LOOP_DEBUG	
			sys_console_printf(" ponId = %d   FALSE !\r\n", ponId);
			sys_console_printf("  %%llid %d  frame len %d\r\n",llid, frame_size);
			sys_console_printf("  %% onuLpbTime = %d\r\n", pgLoopOnuInfo[ponId][onuId]->OnuLpbTime);
			sys_console_printf("  %%LOOP_PROCESS/LOOP_PON_LINK_TEST_CONTINUOUS\r\n");
#endif
			return lRes;
		}	
		return lRes;
	}
	/* 4) */
	else if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_STOP)
	{
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
		/* modified by chenfj 2007-12-20
		ONU 环回过程中(未到约定时间)，此时通过CLI 停止环回，会导致原来的环回未清除，此后再也不能对此PON口下的任何ONU 设置环回*/
		/*pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;*/
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest ( ponId, llid,  LOOP_PON_LINK_TEST_OFF, frame_size, TRUE, &vlan_configuratiion, &test_results );
		
		if (LOOP_OK != lRes)
		{
#ifdef LOOP_DEBUG	
			sys_console_printf(" LOOP_STOP/LOOP_PON_LINK_TEST_OFF\r\n");
#endif
		}	
		else 
		{
			if (NULL != pgLoopStat[ponId][onuId])
			{
				/*
				pgLoopStat[ponId][onuId]->numberOfBadReturnFrames += test_results.numberOfBadReturnFrames;
				pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames += test_results.numberOfErroredReturnFrames;
				pgLoopStat[ponId][onuId]->numberOfSentFrames += test_results.numberOfSentFrames;
				pgLoopStat[ponId][onuId]->numberOfReturnedFrames += test_results.numberOfReturnedFrames;
				pgLoopStat[ponId][onuId]->maximalDelay = test_results.maximalDelay;
				pgLoopStat[ponId][onuId]->meanDelay = test_results.meanDelay;
				pgLoopStat[ponId][onuId]->minimalDelay = test_results.minimalDelay;
				*/
				pgLoopStat[ponId][onuId]->numberOfBadReturnFrames += 0;/*test_results.numberOfBadReturnFrames;*/
				pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames += test_results.number_of_errored_return_frames;
				pgLoopStat[ponId][onuId]->numberOfSentFrames += test_results.number_of_sent_frames;
				pgLoopStat[ponId][onuId]->numberOfReturnedFrames += test_results.number_of_returned_frames;
				pgLoopStat[ponId][onuId]->maximalDelay = test_results.maximal_delay;
				pgLoopStat[ponId][onuId]->meanDelay = test_results.mean_delay;
				pgLoopStat[ponId][onuId]->minimalDelay = test_results.minimal_delay;
			}
#ifdef LOOP_DEBUG		
			sys_console_printf("\r\nPon %d onu %d Link Test ok \r\n", ponId, onuId);
			sys_console_printf("   Total frames Send: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
			sys_console_printf("   Total frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfReturnedFrames);
			sys_console_printf("   Error frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames );
			sys_console_printf("   Minimal delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->minimalDelay);
			sys_console_printf("   Maximal delay: %d(TQ) \r\n", pgLoopStat[ponId][onuId]->maximalDelay);
			sys_console_printf("   Mean delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->meanDelay);
#endif
		}
		
		{/*停止phy环回设置，重设为正常状态*/
			if (FALSE == pgLoopOnuInfo[ponId][onuId]->loopEn)
				return 0;
			
			llid = GetLlidByOnuIdx( ponId, onuId);
			if ( LOOP_INVALID_LLID == llid )
			{
				pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;	
				pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
				looperrorCode = LOOP_ERR;
				return looperrorCode;
			}
			/*loopStatsMsgOnuOn(ponId, onuId);*/
			/*lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, LOOP_OFF,  PON_LOOPBACK_PHY, PON_100M );
			if (LOOP_OK != lRes)
			{
			#ifdef LOOP_DEBUG
			sys_console_printf(" LOOP_NOOP\r\n");
			#endif	
			
			  }
			  
			modified by wutw at august seventeen*/
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
			/* modified by chenfj 2007-12-20
			ONU 环回过程中(未到约定时间)，此时通过CLI 停止环回，会导致原来的环回未清除，此后再也不能对此PON口下的任何ONU 设置环回*/
			/*pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;*/
			looperrorCode = LOOP_ERR;
			/*sys_console_printf(" ponId = %d   FALSE !\r\n", ponId);*/
			/*if (NULL != pgLoopOnuInfo[ponId][OnuId])
			{
			VOS_Free((VOID *) pgLoopOnuInfo[ponId][OnuId]);
			pgLoopOnuInfo[ponId][OnuId] = NULL;
		}*/
#ifdef LOOP_DEBUG
			sys_console_printf(" LOOP set disablesucced !\r\n");
#endif							
		}			
		
		return LOOP_OK;
	}	
	return( LOOP_OK);
}


/*该函数等同OnuLoopPhyLinkTest()函数*/
int  OnuLoopPhyLinkIntTest( short int ponId, short int onuId, short int frame_size)
{
	PON_link_test_vlan_configuration_t vlan_configuratiion;
	PON_link_test_results_t  test_results;
	long lRes = LOOP_OK;
	short int llid =0;
	
	if ((ponId>(LOOP_MAX_PON_NUM-1)) || (onuId > (LOOP_MAX_ONU_NUM-1)))
		
	{
		return LOOP_DEVICE_INVALUE;
	}
	
	vlan_configuratiion.vlan_frame_enable = FALSE;
	vlan_configuratiion.vlan_priority = 0;
	vlan_configuratiion.vlan_tag = 0;
	llid = GetLlidByOnuIdx( ponId, onuId);
	if ( LOOP_INVALID_LLID == llid )
		return LOOP_ERR;
	/* Range: MIN_ETHERNET_FRAME_SIZE - MAX_ETHERNET_FRAME_SIZE_STANDARDIZED */
	if ((frame_size < LOOP_MIN_ETHERNET_FRAME_SIZE) || (frame_size>LOOP_MAX_ETHERNET_FRAME_SIZE_STANDARDIZED))
		return LOOP_ERR;
	#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	lRes = OLT_GetOnuMode( ponId, llid );
	#else
	lRes = PAS_get_onu_mode( ponId, llid );
	#endif
	if( lRes != PON_ONU_MODE_ON )
	{
#ifdef LOOP_DEBUG				
		sys_console_printf(" Pon %d onu %d is Off Line, so can't have a MAC test \r\n");
#endif
		return( lRes );
	}
	
	
	/*1) 如果为alltime 时间参数,以下步骤:先停止环回,读取数据,再启动环回,
	直到下一个2s,再重复该步骤;
	2) 如果为第一次环回(start状态), 则仅开始环回,并改变该环回处理状态为process,
	在下一个2s到来时,开始以上1)的处理步骤;
	3)如果为非alltime,则在2)时要进行
	a) 如果为非alltime,则其时间参数需要减去2s;
	b) 如果为非alltime,检测改时间参数小于2s,则改变环回处理状态为stop,并
	继续等待下一个2s
	
	4) 如果检测到环回处理状态为stop则读取数据,并修改环回状态为noon*/
	
	if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_START)
	{/* 2) */
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_PROCESS;
		/* 3) */
		if (pgLoopOnuInfo[ponId][onuId]->OnuLpbTime != LOOP_ALLTIME)
		{
			if (pgLoopOnuInfo[ponId][onuId]->OnuLpbTime > LOOP_TIMEOUT)
				pgLoopOnuInfo[ponId][onuId]->OnuLpbTime -= LOOP_TIMEOUT;
			else
				pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_STOP;
		}
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest ( ponId, llid,  LOOP_PON_LINK_TEST_CONTINUOUS, frame_size, TRUE, &vlan_configuratiion, &test_results );
		if (LOOP_OK != lRes)
		{
			return lRes;
		}
		return lRes;
	}
	/* 1) */
	else if ((pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_PROCESS) )
	{
		/* 3) */
		if (pgLoopOnuInfo[ponId][onuId]->OnuLpbTime != LOOP_ALLTIME)
		{
			if (pgLoopOnuInfo[ponId][onuId]->OnuLpbTime > LOOP_TIMEOUT)
				pgLoopOnuInfo[ponId][onuId]->OnuLpbTime -= LOOP_TIMEOUT;
			else
				pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_STOP;
		}
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest ( ponId, llid,  LOOP_PON_LINK_TEST_OFF, frame_size, TRUE, &vlan_configuratiion, &test_results );
		if (LOOP_OK != lRes)
		{
			return lRes;
		}	
		else 
		{
			if (NULL != pgLoopStat[ponId][onuId])
				memcpy(pgLoopStat[ponId][onuId], &test_results, sizeof(LOOP_PON_link_test_results_t));
#ifdef LOOP_DEBUG		
			sys_console_printf("\r\nPon %d onu %d Link Test ok \r\n", ponId, onuId);
			sys_console_printf("   Total frames Send: %d \r\n", test_results.numberOfSentFrames);
			sys_console_printf("   Total frames Recv: %d \r\n", test_results.numberOfReturnedFrames);
			sys_console_printf("   Error frames Recv: %d \r\n", test_results.numberOfErroredReturnFrames );
			sys_console_printf("   Minimal delay : %d(TQ) \r\n", test_results.minimalDelay);
			sys_console_printf("   Maximal delay: %d(TQ) \r\n", test_results.maximalDelay);
			sys_console_printf("   Mean delay : %d(TQ) \r\n", test_results.meanDelay);
#endif
		}
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest ( ponId, llid,  LOOP_PON_LINK_TEST_CONTINUOUS, frame_size, TRUE, &vlan_configuratiion, &test_results );
		if (LOOP_OK != lRes)
		{
			return lRes;
		}			
		return lRes;
	}
	/* 4) */
	else if (pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl == LOOP_STOP)
	{
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		lRes = OLT_LinkTest ( ponId, llid,  LOOP_PON_LINK_TEST_OFF, frame_size, TRUE, &vlan_configuratiion, &test_results );
		if (LOOP_OK != lRes)
		{
			return lRes;
		}	
		else 
		{
			if (NULL != pgLoopStat[ponId][onuId])
				memcpy(pgLoopStat[ponId][onuId], &test_results, sizeof(LOOP_PON_link_test_results_t));
#ifdef LOOP_DEBUG		
			sys_console_printf("\r\nPon %d onu %d Link Test ok \r\n", ponId, onuId);
			sys_console_printf("   Total frames Send: %d \r\n", test_results.numberOfSentFrames);
			sys_console_printf("   Total frames Recv: %d \r\n", test_results.numberOfReturnedFrames);
			sys_console_printf("   Error frames Recv: %d \r\n", test_results.numberOfErroredReturnFrames );
			sys_console_printf("   Minimal delay : %d(TQ) \r\n", test_results.minimalDelay);
			sys_console_printf("   Maximal delay: %d(TQ) \r\n", test_results.maximalDelay);
			sys_console_printf("   Mean delay : %d(TQ) \r\n", test_results.meanDelay);
#endif
		}
		return LOOP_OK;
	}
	return( LOOP_OK);
}


#if 0

/*================for testing================*/
int onutest(void)
{
	LONG 	lRes = LOOP_OK;
	LOOP_msg_t *pLpMsg = NULL;
	pLpMsg = (LOOP_msg_t *)VOS_Malloc((ULONG)sizeof(LOOP_msg_t), (ULONG)MODULE_LOOPBACK);
	if (NULL == pLpMsg)
		return LOOP_ERR;
	memset(pLpMsg, 0, sizeof(LOOP_msg_t));
	pLpMsg->paramType = LOOP_LINK_TEST;
	
	
	VOS_Free(pLpMsg);
	pLpMsg = NULL;
	return lRes;
}


int loopstart(unsigned short ponId, unsigned short onuId)
{
	PON_link_test_vlan_configuration_t vlan_configuratiion;
	PON_link_test_results_t  test_results;
	long lRes = LOOP_OK;
	short int llid =0;
	
	/*说明: 使能onu 环回*/
	llid = GetLlidByOnuIdx( ponId, onuId);	
	if ( LOOP_INVALID_LLID == llid )
		return LOOP_ERR;
	/*loopStatsMsgOnuOff(ponId, onuId);*/
	lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, 32767,  PON_LOOPBACK_PHY, PON_100M );
	if (LOOP_OK != lRes)
	{
		looperrorCode = LOOP_ERR;
		return LOOP_ERR;
	}
	
	if ((ponId>LOOP_MAX_PON_NUM) || (onuId > LOOP_MAX_ONU_NUM))
		
	{
		return LOOP_DEVICE_INVALUE;
	}
	
	vlan_configuratiion.vlan_frame_enable = FALSE;
	vlan_configuratiion.vlan_priority = 0;
	vlan_configuratiion.vlan_tag = 0;
	memset(&test_results, 0, sizeof(LOOP_PON_link_test_results_t));
	
	lRes = PAS_link_test ( ponId, llid,  LOOP_PON_LINK_TEST_CONTINUOUS, 200, TRUE, &vlan_configuratiion, &test_results );
	if (LOOP_OK != lRes)
				{
		return lRes;
				}
	return lRes;
	
	
}

int loopoff(unsigned short ponId, unsigned short onuId)
{
	PON_link_test_vlan_configuration_t vlan_configuratiion;
	PON_link_test_results_t  test_results;
	long lRes = LOOP_OK;
	short int llid =0;
	
	/*llid = GetLlidByOnuIdx( ponId, onuId);*/
	if ((ponId>(LOOP_MAX_PON_NUM-1)) || (onuId > (LOOP_MAX_ONU_NUM-1)))
		
	{
		return LOOP_DEVICE_INVALUE;
	}
	llid = GetLlidByOnuIdx( ponId, onuId);
	if ( LOOP_INVALID_LLID == llid )
		return LOOP_ERR;
	vlan_configuratiion.vlan_frame_enable = FALSE;
	vlan_configuratiion.vlan_priority = 0;
	vlan_configuratiion.vlan_tag = 0;
	memset(&test_results, 0, sizeof(LOOP_PON_link_test_results_t));
	
	/**/
	lRes = PAS_link_test ( ponId, llid,  LOOP_PON_LINK_TEST_OFF, 200, TRUE, &vlan_configuratiion, &test_results );
	if (LOOP_OK != lRes)
				{	
		
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
		{/*停止phy环回设置，重设为正常状态*/
			if (FALSE == pgLoopOnuInfo[ponId][onuId]->loopEn)
				looperrorCode = LOOP_ERR;
			
			llid = GetLlidByOnuIdx( ponId, onuId);
			if ( LOOP_INVALID_LLID == llid )
			{
				pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;	
				pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
				looperrorCode = LOOP_ERR;
				return looperrorCode;
			}
			/*loopStatsMsgOnuOn(ponId, onuId);*/
			lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, LOOP_OFF,  
				PON_LOOPBACK_PHY, PON_100M );
			if (LOOP_OK != lRes)
			{
				looperrorCode = LOOP_ERR;
				return looperrorCode;
			}
			
			/*modified by wutw at august seventeen*/
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
			
			/*if (NULL != pgLoopOnuInfo[ponId][OnuId])
			{
			VOS_Free((VOID *) pgLoopOnuInfo[ponId][OnuId]);
			pgLoopOnuInfo[ponId][OnuId] = NULL;
		}*/
		}			
				}
	
	lRes = PAS_link_test ( ponId, llid,  LOOP_PON_LINK_TEST_CONTINUOUS, 200, TRUE, &vlan_configuratiion, &test_results );
	if (LOOP_OK != lRes)
				{
		pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl = LOOP_NOOP;
		{/*停止phy环回设置，重设为正常状态*/
			if (FALSE == pgLoopOnuInfo[ponId][onuId]->loopEn)
				looperrorCode = LOOP_ERR;
			
			llid = GetLlidByOnuIdx( ponId, onuId);
			if ( LOOP_INVALID_LLID == llid )
			{
				pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;	
				pgLoopOnuInfo[ponId][onuId]->OnuLpbTime = 0;
				looperrorCode = LOOP_ERR;
				return looperrorCode;
			}
			/*loopStatsMsgOnuOn(ponId, onuId);*/
			lRes = PAS_set_test_mode_v4( ponId,  llid, PAS_PON_MAC, LOOP_OFF,  
				PON_LOOPBACK_PHY, PON_100M );
			if (LOOP_OK != lRes)
			{
				looperrorCode = LOOP_ERR;
				return looperrorCode;
			}
			
			/*modified by wutw at august seventeen*/
			pgLoopOnuInfo[ponId][onuId]->loopEn = FALSE;
			
			/*if (NULL != pgLoopOnuInfo[ponId][OnuId])
			{
			VOS_Free((VOID *) pgLoopOnuInfo[ponId][OnuId]);
			pgLoopOnuInfo[ponId][OnuId] = NULL;
		}*/
		}
		return lRes;
				}
	return lRes;
	
	
}


int loopallshow()
{
	short int ponId = 0;
	short int onuId = 0;
	short int loopstatus = 0;
	for(ponId = 0; ponId < 20; ponId++)
	{
		loopstatus = pgLoopOnuInfo[ponId][onuId]->OnuLpbCtrl;
		/*OnuLoopPhyTestStatusGet(ponId,onuId, &loopstatus);*/
		sys_console_printf("\r\n ponId %d  onuId %d  ", ponId, onuId, loopstatus);
		switch(loopstatus)
		{
		case LOOP_NOOP:
			sys_console_printf(" loopstatus LOOP_NOOP!\r\n");
			break;
		case LOOP_START:
			sys_console_printf(" loopstatus LOOP_START\r\n");
			break;
		case LOOP_STOP:
			sys_console_printf(" loopstatus LOOP_STOP\r\n");
			break;
		case LOOP_PROCESS:
			sys_console_printf(" loopstatus LOOP_PROCESS\r\n");
			break;
		default:
			break;
		}
	}
	return LOOP_OK;
}

int loopOnuLinkTestShow(short int ponId, short int onuId)
{
	
	/*sys_console_printf("\r\n  Pon %d onu %d Link Test ok \r\n", ponId, onuId);*/
	sys_console_printf("\r\n  %d/%d/%d Link Test result\r\n", ponId, onuId);
	sys_console_printf("   Total frames Send: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
	sys_console_printf("   Total frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfReturnedFrames);
	sys_console_printf("   Error frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames );
	sys_console_printf("   Minimal delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->minimalDelay);
	sys_console_printf("   Maximal delay: %d(TQ) \r\n", pgLoopStat[ponId][onuId]->maximalDelay);
	sys_console_printf("   Mean delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->meanDelay);	
	return LOOP_OK;
}
#endif

int loopOnuLinkTesVtyShow(short int slotId,short int port,short int ponId, short int onuId,struct vty *vty)
{
	
	unsigned int lpbTime = 0;
	unsigned int runLpbTime = 0;
	short int lpbStatus = 0;
	OnuLoopLinkTestLpbTimeGet( ponId, onuId , &lpbTime);
	OnuLoopLinkTestLpbRunningTimeGet( ponId, onuId , &runLpbTime);	
	OnuLoopLinkTestLpbStatusGet(ponId, onuId , &lpbStatus);
	/*sys_console_printf("\r\n  Pon %d onu %d Link Test ok \r\n", ponId, onuId);*/
	vty_out( vty, "\r\n  %d/%d/%d Link Test result\r\n", slotId,port,onuId+1);
	vty_out( vty, "   Total frames Send: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
	vty_out( vty, "   Total frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
	vty_out( vty, "   Error frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames );
	vty_out( vty, "   Loop status      : ");
	if ( LOOP_NOOP== lpbStatus)
		vty_out( vty, "noop\r\n");
	else if (LOOP_START == lpbStatus)
		vty_out( vty, "start\r\n");
	else if (LOOP_PROCESS == lpbStatus)
		vty_out( vty, "processing\r\n");
	else if (LOOP_STOP == lpbStatus)
		vty_out( vty, "stop\r\n");
	else
		vty_out( vty, "unknown\r\n");
		vty_out( vty, "   Loop test time   : %d(s)\r\n",lpbTime-runLpbTime);	/*sys_console_printf("   Minimal delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->minimalDelay);
																				sys_console_printf("   Maximal delay: %d(TQ) \r\n", pgLoopStat[ponId][onuId]->maximalDelay);
		sys_console_printf("   Mean delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->meanDelay);*/
		return LOOP_OK;
}

#if 0
int loopOnuLinkTesVtyAll(short int slotId,short int port,short int ponId, short int onuId,struct vty *vty)
{
	
	unsigned int lpbTime = 0;
	unsigned int runLpbTime = 0;
	short int lpbStatus = 0;
	OnuLoopLinkTestLpbTimeGet( ponId, onuId , &lpbTime);
	OnuLoopLinkTestLpbRunningTimeGet( ponId, onuId , &runLpbTime);	
	OnuLoopLinkTestLpbStatusGet(ponId, onuId , &lpbStatus);
	/*sys_console_printf("\r\n  Pon %d onu %d Link Test ok \r\n", ponId, onuId);*/
	vty_out( vty, "\r\n  %d/%d/%d Link Test result\r\n", slotId,port,onuId+1);
	vty_out( vty, "   Total frames Send: %d \r\n", pgLoopStat[ponId][onuId]->numberOfSentFrames);
	vty_out( vty, "   Total frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfReturnedFrames);
	vty_out( vty, "   Error frames Recv: %d \r\n", pgLoopStat[ponId][onuId]->numberOfErroredReturnFrames );
	vty_out( vty, "   Loop status      : ");
	if ( LOOP_NOOP== lpbStatus)
		vty_out( vty, "noop\r\n");
	else if (LOOP_START == lpbStatus)
		vty_out( vty, "start\r\n");
	else if (LOOP_PROCESS == lpbStatus)
		vty_out( vty, "processing\r\n");
	else if (LOOP_STOP == lpbStatus)
		vty_out( vty, "stop\r\n");
	else
		vty_out( vty, "unknown\r\n");
		vty_out( vty, "   Tested time      : %d(s)\r\n",lpbTime-runLpbTime);	/*sys_console_printf("   Minimal delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->minimalDelay);
																				sys_console_printf("   Maximal delay: %d(TQ) \r\n", pgLoopStat[ponId][onuId]->maximalDelay);
		sys_console_printf("   Mean delay : %d(TQ) \r\n", pgLoopStat[ponId][onuId]->meanDelay);*/
		return LOOP_OK;
}
#endif

int loopOnuLinkTesParamVtyShow(short int slotId,short int port,short int ponId, short int onuId,struct vty *vty)
{
	unsigned int lpbScr = 0;	
	unsigned int lpbTime = 0;	
	short int lpbStatus = 0;
	OnuLoopLinkTestLpbScrGet(ponId, onuId , &lpbScr) ;
	OnuLoopLinkTestLpbTimeGet(ponId, onuId , &lpbTime)	;
	OnuLoopLinkTestLpbStatusGet(ponId, onuId , &lpbStatus);
	vty_out( vty, "  Loop parameter(%d/%d/%d) : \r\n",slotId,port,onuId+1);
	vty_out( vty, "  Datasource    	: ");
	if(LOOP_INTERNAL == lpbScr)
		vty_out( vty, "inter\r\n");
	else if (LOOP_EXTERNAL == lpbScr)
		vty_out( vty, "outer\r\n");
	else
    {
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",ponId);
#endif   
        vty_out( vty, "unknown.\r\n" );
    }		
	vty_out( vty, "  Looptime       : %d(s)\r\n",lpbTime);
	vty_out( vty, "  Loop status    : ");
	if ( LOOP_NOOP== lpbStatus)
		vty_out( vty, "noop\r\n");
	else if (LOOP_START == lpbStatus)
		vty_out( vty, "start\r\n");
	else if (LOOP_PROCESS == lpbStatus)
		vty_out( vty, "processing\r\n");
	else if (LOOP_STOP == lpbStatus)
		vty_out( vty, "stop\r\n");
	else
		vty_out( vty, "unknown\r\n");
	return LOOP_OK;
}


int loopPonStatusInfoVty(short int ponId, struct vty *vty)
{
	short int tempOnuId = 0;
	short int lpbStatus = 0;
	unsigned int lpbTime = 0;
	unsigned int runLpbTime = 0;
	unsigned int lpbScr = 0;
	
	for (tempOnuId = 0;tempOnuId < LOOP_MAX_ONU_NUM; tempOnuId++)
	{
		if (FALSE == pgLoopOnuInfo[ponId][tempOnuId]->loopEn)
			continue;
		OnuLoopLinkTestLpbStatusGet(ponId, tempOnuId , &lpbStatus);
		OnuLoopLinkTestLpbTimeGet( ponId, tempOnuId , &lpbTime);
		OnuLoopLinkTestLpbRunningTimeGet( ponId, tempOnuId , &runLpbTime);	
		OnuLoopLinkTestLpbScrGet(ponId, tempOnuId , &lpbScr) ;
		vty_out( vty, "\r\n\r\n  %d loop testing \r\n", tempOnuId+1);
		vty_out( vty, "  Total frames Send: %d \r\n", pgLoopStat[ponId][tempOnuId]->numberOfSentFrames);
		vty_out( vty, "  Total frames Recv: %d \r\n", pgLoopStat[ponId][tempOnuId]->numberOfReturnedFrames);
		vty_out( vty, "  Error frames Recv: %d \r\n", pgLoopStat[ponId][tempOnuId]->numberOfErroredReturnFrames );
		vty_out( vty, "  Loop status      : ");
		if ( LOOP_NOOP== lpbStatus)
			vty_out( vty, "noop\r\n");
		else if (LOOP_START == lpbStatus)
			vty_out( vty, "start\r\n");
		else if (LOOP_PROCESS == lpbStatus)
			vty_out( vty, "processing\r\n");
		else if (LOOP_STOP == lpbStatus)
			vty_out( vty, "stop\r\n");
		else
			vty_out( vty, "unknown\r\n");
		vty_out( vty, "  Tested time      : %d(s)\r\n",lpbTime-runLpbTime);
		vty_out( vty, "  Loop time        : %d(s)\r\n",lpbTime);
		vty_out( vty, "  Datasource       : ");
		if(LOOP_INTERNAL == lpbScr)
			vty_out( vty, "inter\r\n");
		else if (LOOP_EXTERNAL == lpbScr)
			vty_out( vty, "outer\r\n");
		else
		{
#ifdef CLI_EPON_DEBUG
			vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",ponId);
#endif   
			vty_out( vty, "unknown.\r\n" );
		}	
		return LOOP_OK;
	}
	return LOOP_OK;
}


int loopAllOnuStatusInfoVty( struct vty *vty )
{
	short int tempOnuId = 0;
	short int lpbStatus = 0;
	unsigned int lpbTime = 0;
	unsigned int runLpbTime = 0;
	unsigned int lpbScr = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	short int ponId = 0;
	for ( ponId = 0; ponId < MAXPON/*LOOP_MAX_PON_NUM*/; ponId++ )
	{
		for ( tempOnuId = 0;tempOnuId < MAXONUPERPON/*LOOP_MAX_ONU_NUM*/; tempOnuId++ )
		{
			if (FALSE == pgLoopOnuInfo[ponId][tempOnuId]->loopEn)
				continue;

			ulSlot = GetCardIdxByPonChip(ponId);
			ulPort = GetPonPortByPonChip(ponId);

			OnuLoopLinkTestLpbStatusGet(ponId, tempOnuId , &lpbStatus);
			OnuLoopLinkTestLpbTimeGet( ponId, tempOnuId , &lpbTime);
			OnuLoopLinkTestLpbRunningTimeGet( ponId, tempOnuId , &runLpbTime);	
			OnuLoopLinkTestLpbScrGet(ponId, tempOnuId , &lpbScr) ;
			
			vty_out( vty, "\r\n  onu %d/%d/%d Loop status     : ", ulSlot, ulPort, tempOnuId+1);
			if ( LOOP_NOOP== lpbStatus)
				vty_out( vty, "noop\r\n");
			else if (LOOP_START == lpbStatus)
				vty_out( vty, "start\r\n");
			else if (LOOP_PROCESS == lpbStatus)
				vty_out( vty, "processing\r\n");
			else if (LOOP_STOP == lpbStatus)
				vty_out( vty, "stop\r\n");
			else
				vty_out( vty, "unknown\r\n");
			vty_out( vty, "  Tested time               : %d(s)\r\n",lpbTime-runLpbTime);
			vty_out( vty, "  Loop time                 : %d(s)\r\n",lpbTime);
			vty_out( vty, "  Datasource                : ");
			if(LOOP_INTERNAL == lpbScr)
				vty_out( vty, "inter\r\n");
			else if (LOOP_EXTERNAL == lpbScr)
				vty_out( vty, "outer\r\n");
			else
			{
#ifdef CLI_EPON_DEBUG
				vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",ponId);
#endif   
				vty_out( vty, "unknown.\r\n" );
			}	
			break;
		}
	}
	return LOOP_OK;
}

#if 0
int loopsetmode(short int Enstatus)
{
	short int lRes = LOOP_OK;
	short int macMode = PAS_PON_MAC;
	short int loopbackMode = 32767;
	short int onuPhyLoopSpeed = PON_100M;	
	short int loopBackType = PON_LOOPBACK_PHY;
	
	if (Enstatus == 0)
		loopBackType = 0;
	lRes = PAS_set_test_mode_v4( 1,  1, (PON_mac_test_modes_t)macMode, loopbackMode,  
		(PON_loopback_t)loopBackType, 
		(PON_link_rate_t)onuPhyLoopSpeed );
	if (LOOP_OK != lRes)
	{
		sys_console_printf(" set test mode failue!\r\n");
		return lRes;		
	}
	return lRes;
}


int  StartPonLinkTest_1( short int PonId, short int OnuId)
{
	
	PON_link_test_vlan_configuration_t vlan_configuratiion;
	PON_link_test_results_t  test_results;
	short int testFramesCount = -1;
	short int frame_size;
	unsigned short int Llid =1;
	short int ret = LOOP_OK;
	
	if ((PonId>LOOP_MAX_PON_NUM) || (OnuId > LOOP_MAX_ONU_NUM))
		
	{
		return LOOP_DEVICE_INVALUE;
	}
	
	vlan_configuratiion.vlan_frame_enable = FALSE;
	vlan_configuratiion.vlan_priority = 0;
	vlan_configuratiion.vlan_tag = 0;
	
	/*   number_of_frames : 
	**		Activate the test / stop running test. Values: 
	**		PON_LINK_TEST_CONTINUOUS - Start a continuous link test. Results are 
	**									  not returned.
	**		PON_LINK_TEST_OFF	- Stop an already running link test. Results 
	**								are returned.
	**		1 - PON_LINK_TEST_MAX_FRAMES	- Perform a complete test using the specified 
	**								      number of frames. Results are returned.
	*/
	/*number_of_frames = 1;*/
	
	/* Range: MIN_ETHERNET_FRAME_SIZE - MAX_ETHERNET_FRAME_SIZE_STANDARDIZED */
	frame_size = 256;
	
	if(GetOnuOperStatus(PonId, OnuId) !=ONU_OPER_STATUS_UP)
		{			
		sys_console_printf(" Pon %d onu %d is Off Line, so can't have a MAC test \r\n",PonId, OnuId);
		return( RERROR );
		}
	Llid = GetLlidByOnuIdx(PonId, OnuId);
	ret = PAS_get_onu_mode( PonId, Llid );
	if( ret != PON_ONU_MODE_ON )
	{
		return( ret );
	}
	
	ret = PAS_link_test ( PonId, Llid,  testFramesCount, frame_size, TRUE, &vlan_configuratiion, &test_results );
	if( ret == LOOP_OK ) 
	{
		if( testFramesCount == (-1) )
		{
			sys_console_printf("\r\nPon %d onu %d Link Test start ok \r\n", PonId, OnuId);
		}
		
		else{
		}
		return( ROK );
	}
	else{
		return( RERROR );
	}
	
}


int  StopPonLinkTest( short int PonId, short int OnuId)
{
	
	PON_link_test_vlan_configuration_t vlan_configuratiion;
	PON_link_test_results_t  test_results;
	short int testFramesCount = 0;
	short int frame_size;
	unsigned short int Llid =1;
	short int ret = LOOP_OK;
	
	if ((PonId>LOOP_MAX_PON_NUM) || (OnuId > LOOP_MAX_ONU_NUM))
		
	{
		return LOOP_DEVICE_INVALUE;
	}
	
	vlan_configuratiion.vlan_frame_enable = FALSE;
	vlan_configuratiion.vlan_priority = 0;
	vlan_configuratiion.vlan_tag = 0;
	
	/*   number_of_frames : 
	**		Activate the test / stop running test. Values: 
	**		PON_LINK_TEST_CONTINUOUS - Start a continuous link test. Results are 
	**									  not returned.
	**		PON_LINK_TEST_OFF	- Stop an already running link test. Results 
	**								are returned.
	**		1 - PON_LINK_TEST_MAX_FRAMES	- Perform a complete test using the specified 
	**								      number of frames. Results are returned.
	*/
	/*number_of_frames = 1;*/
	
	/* Range: MIN_ETHERNET_FRAME_SIZE - MAX_ETHERNET_FRAME_SIZE_STANDARDIZED */
	frame_size = 256;

	if(GetOnuOperStatus(PonId, OnuId) !=ONU_OPER_STATUS_UP)
		{			
		sys_console_printf(" Pon %d onu %d is Off Line, so can't have a MAC test \r\n",PonId, OnuId);
		return( RERROR );
		}
	Llid = GetLlidByOnuIdx(PonId, OnuId);
	ret = PAS_get_onu_mode( PonId, Llid );
	if( ret != PON_ONU_MODE_ON )
	{
		sys_console_printf(" Pon %d onu %d is Off Line, so can't have a MAC test \r\n",PonId, OnuId);
		return( ret );
	}
	
	ret = PAS_link_test ( PonId, Llid,  testFramesCount, frame_size, TRUE, &vlan_configuratiion, &test_results );
	if( ret == LOOP_OK ) 
	{
		if( testFramesCount == (-1) )
		{
			sys_console_printf("\r\nPon %d onu %d Link Test start ok \r\n", PonId, OnuId);
		}
		
		else{
		}
		return( ROK );
	}
	else{
		return( RERROR );
	}
	
}

int OnuLoopPhyLinkTest___0(DeviceIndex_S Ifdex, short int FrameSize, short int loopFlag, short int dataType)
{
	short int ponId = 0;
	short int onuId = 0;
	LOOP_msg_t	*plpMsg = NULL;
	ULONG ulMsg[4] = {0};
	short int Ret = LOOP_OK;
	short int count = 0;
	LOOP_PON_link_test_results_t  test_results;	
	
	plpMsg = VOS_Malloc((ULONG)sizeof(LOOP_msg_t), (ULONG)MODULE_LOOPBACK);
	if (NULL == plpMsg)
		return LOOP_ERR;
	
	plpMsg->oltId = ponId;
	plpMsg->onuId = onuId;
	plpMsg->loopbackMode = FrameSize;
	plpMsg->paramType = LOOP_LINK_TEST;
	
	/*ret = PAS_link_test ( PonId, Llid,  testFramesCount, frame_size, TRUE, &vlan_configuratiion, &test_results );*/		
	ulMsg[3] = (ULONG)(plpMsg);
	
	Ret = VOS_QueSend(OnuLoopQueue, ulMsg, NO_WAIT,MSG_PRI_NORMAL);
	
	if (VOS_ERROR == Ret)
	{
		ASSERT( 0 );
		VOS_Free((VOID *)plpMsg);
		plpMsg = NULL;
		return Ret;
	}
	if (LOOP_PON_LINK_TEST_CONTINUOUS == FrameSize)
		return LOOP_OK;
	else if (LOOP_PON_LINK_TEST_OFF == FrameSize)
	{
		looperrorCode = LOOP_TIMEOUT_ERR;
		while((looperrorCode != LOOP_OK))
		{
			taskDelay(20);
			if (count>=3)
			{
				return looperrorCode;
			}
			count++;
		}
		if (LOOP_OK == looperrorCode)
		{
			memcpy(&test_results, pgLoopStat[ponId][onuId], sizeof(LOOP_PON_link_test_results_t));
		}
		return looperrorCode;
		
	}
	else if ( (FrameSize>0) && (FrameSize<LOOP_PON_LINK_TEST_MAX_FRAMES))
	{
		looperrorCode = LOOP_TIMEOUT_ERR;
		while((looperrorCode != LOOP_OK))
		{
			taskDelay(20);
			if (count>=5)
			{
				return looperrorCode;
			}
			count++;
		}
		if (LOOP_OK == looperrorCode)
		{
			memcpy(&test_results, pgLoopStat[ponId][onuId], sizeof(LOOP_PON_link_test_results_t));
		}			
		return looperrorCode;
	}
	return LOOP_OK;
}


int testOnuLoop(short int ponId, short int onuId, short int testFramesCount)
{			
	short int llid = 0;
	LONG 	lRes = LOOP_OK;
	short frame_size = 400;
	
	PON_link_test_vlan_configuration_t vlan_configuratiion;
	PON_link_test_results_t  test_results;
	
	if ((ponId> (LOOP_MAX_PON_NUM-1)) || (onuId > (LOOP_MAX_ONU_NUM-1)))
		
	{
		return LOOP_DEVICE_INVALUE;
	}
	
	vlan_configuratiion.vlan_frame_enable = FALSE;
	vlan_configuratiion.vlan_priority = 0;
	vlan_configuratiion.vlan_tag = 0;
	
	
	llid = GetLlidByOnuIdx( ponId, onuId);
	if ( LOOP_INVALID_LLID == llid )
		return LOOP_ERR;
	/*检查是否已经为设置了回环模式*/
	/*
	if (FALSE == pgLoopOnuInfo[ponId][onuId]->loopEn)
	{
	return LOOP_ERR;
	
	  }
	*/	
	if (LOOP_PON_LINK_TEST_CONTINUOUS == testFramesCount)
	{
		lRes = PAS_link_test ( ponId, llid,  testFramesCount, frame_size, TRUE, &vlan_configuratiion, &test_results );
		if (LOOP_OK != lRes)
		{
			return LOOP_ERR;
		}
	}
	else if (LOOP_PON_LINK_TEST_OFF == testFramesCount)
	{
		lRes = PAS_link_test ( ponId, llid,  testFramesCount, frame_size, TRUE, &vlan_configuratiion, &test_results );
		if (LOOP_OK == lRes)
		{
			if (NULL != pgLoopStat[ponId][onuId])
			{
				memcpy(pgLoopStat[ponId][onuId], &test_results, sizeof(LOOP_PON_link_test_results_t));
			}
		}					
	}
	else if ((testFramesCount>0) && (testFramesCount< LOOP_PON_LINK_TEST_MAX_FRAMES))
	{
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ( LOOP_INVALID_LLID == llid )
			return LOOP_ERR;
		lRes = PAS_link_test ( ponId, llid,  testFramesCount, frame_size, TRUE, &vlan_configuratiion, &test_results );
		if (LOOP_OK == lRes)
		{
			if (NULL != pgLoopStat[ponId][onuId])
			{
				memcpy(pgLoopStat[ponId][onuId], &test_results, sizeof(LOOP_PON_link_test_results_t));
			}
		}
	}
	else
		return LOOP_ERR;
	
	return LOOP_OK;				
}

int testPasLinkTest( void )
{
	return LOOP_OK;
}
#endif

DEFUN(
	olt_loop_info_show, 
	olt_loop_info_show_cmd,
	"show pon-loop information",
	DescStringCommonShow
	"Show pon-loop information"
	"Show pon-loop information"
	)
{
	LONG lRet = 0;

	lRet = loopAllOnuStatusInfoVty( vty );
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}

DEFUN(
	pon_loop_info_show, 
	pon_loop_info_show_cmd,
	"show pon-loop information",
	DescStringCommonShow
	"Show pon-loop information\n"
	"Show pon-loop information\n"
	)
{
	LONG lRet = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	lRet = loopPonStatusInfoVty( phyPonId, vty );
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}		
	return CMD_SUCCESS;
}

DEFUN  (
    onu_loop_start,
    onu_loop_start_cmd,
    "pon-loop source inter time <4-268435455>", /* [inter|outer] */
    "Config the loop attribute\n"
    "Config the onu's pon loop operation\n"
   /* "Config the loop source\n"*/
    "Loop inter source\n"
   /* "Loop outer source\n"*/
    "Config the loop time\n"
    "Please input the loop time length(second)\n" )
{
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
	short int loopOnuId = 0;
    INT16 phyPonId = 0;	
	INT16 userOnuId = 0;
	unsigned int lpbScr = 0;
    unsigned int lpbTime = 0;	
		
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;

	if( checkVtyOnuIsValid(vty, slotId, port, onuId, &phyPonId, &userOnuId) == VOS_ERROR )
		return CMD_WARNING;

	/*
    if ( !VOS_StrCmp( argv[ 0 ], "inter" ) )
    {
		lpbScr = CLI_EPON_LOOP_INTERNAL;
    } 
	else if ( !VOS_StrCmp( argv[ 0 ], "outer" ) )
	{
		lpbScr = CLI_EPON_LOOP_EXTERNAL;
	}
	else
    {  
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	*/
	lpbScr = CLI_EPON_LOOP_INTERNAL;
	lRet = OnuLoopLinkTestLpbScrSet(phyPonId, userOnuId , lpbScr);
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	  

	lpbTime = ( unsigned int  ) VOS_AtoL( argv[ 0 ] );  	
	lRet = OnuLoopLinkTestLpbTimeSet(phyPonId, userOnuId , lpbTime)	 ;
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	 
		
	lRet = OnuLoopPhyTestLpbCtrlSet( phyPonId, userOnuId , CLI_EPON_LOOP_START);
	if (lRet == LOOP_ONULOOP_EXIST)
	{
		OnuLoopLinkTestLpbOnuGet(phyPonId, &loopOnuId);
		vty_out( vty, "  %% loop test of %d/%d/%d is executing.\r\n", slotId, port, loopOnuId+1);
	}
	else if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	 
		
    return CMD_SUCCESS;
}

DEFUN  (
    onu_loop_stop,
    onu_loop_stop_cmd,
    "undo pon-loop",
    NO_STR
    "Stop the loop attribute\n")
{
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;	
	INT16 userOnuId = 0;;
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;

	if( checkVtyOnuIsValid(vty, slotId, port, onuId, &phyPonId, &userOnuId) == VOS_ERROR )
		return CMD_WARNING;

	lRet = OnuLoopPhyTestLpbCtrlSet( phyPonId, userOnuId , CLI_EPON_LOOP_STOP);
	if (lRet == CLI_EPON_LOOP_ONULOOP_EXIST)
	{
		vty_out( vty, "  %% Loopback had stopped or the onu is not exist.\r\n");
	}
   else if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	         
    return CMD_SUCCESS;
}

/*显示参数*/
DEFUN  (
    onu_show_loop_parameter,
    onu_show_loop_parameter_cmd,
    "show pon-loop parameter",
    DescStringCommonShow
    "Show the loop attribute \n"
    "Show the loop attribute parameter of onu\n")
{
    ULONG ulIfIndex = 0;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;
	INT16 userOnuId = 0;
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	userOnuId = onuId - 1;
	loopOnuLinkTesParamVtyShow(slotId,port,phyPonId, userOnuId,vty);		 

    return CMD_SUCCESS;
	
}
/*显示环回结果*/
DEFUN  (
    onu_show_loop_result,
    onu_show_loop_result_cmd,
    "show pon-loop result",
    DescStringCommonShow
    "Show the loop result\n"
    "Show the loop result of onu\n")
{
    ULONG ulIfIndex = 0;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;	
	INT16 userOnuId = 0;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	userOnuId = onuId - 1;
	loopOnuLinkTesVtyShow(slotId,port,phyPonId, userOnuId,vty);		
    return CMD_SUCCESS;
}


LONG OnuLoop_CommandInstall()
{
	install_element ( CONFIG_NODE, &olt_loop_info_show_cmd);
	install_element ( VIEW_NODE, &olt_loop_info_show_cmd);
	install_element ( PON_PORT_NODE, &pon_loop_info_show_cmd);
}

LONG OnuLoop_CommandInstallByType( enum node_type  node )
{
	install_element ( node, &onu_loop_start_cmd);
	install_element ( node, &onu_loop_stop_cmd);
	install_element ( node, &onu_show_loop_result_cmd);
	install_element ( node, &onu_show_loop_parameter_cmd);
}

#endif

