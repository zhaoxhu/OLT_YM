
/**************************************************************
*
*    Onuloop.h -- ONU loop back module high level Application functions General header
*
*  
*    Copyright (c)  2006.7 , GW Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	     |    Change			     	|    Author	  
*   ---------|-----------|---------------------|------------
*	1.00	  | 19/05/2006 |   Creation				| wu tw
*
***************************************************************/


#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#ifdef __cplusplus
extern "C" {
#endif


#define LOOP_OK								(0)
#define LOOP_ERR							(-1)
#define LOOP_DEVICE_INVALUE				(-2)
#define LOO_PARAM_ERR						(-3)
#define LOOP_ONULOOP_EXIST					(-4)
#define LOOP_PARAM_ERR						(-5)
#define LOOP_TIMEOUT_ERR					(-6)
#define LOOP_GETMEM_ERR					(-7)

#define LOOP_MAX_ONU_NUM 						MAXONUPERPON
#define LOOP_MAX_PON_NUM						MAXPON

#define LOOP_ON								1
#define LOOP_OFF							0
#define LOOP_SPEED_100M					100
/*#define LOOP_PON_OLT_ID					(-1)*/
#define LOOP_SET_MODE						1
#define LOOP_LINK_TEST						2
#define LOOP_TASK_PRIO						150
#define LOOP_MIN_ETHERNET_FRAME_SIZE			60
#define LOOP_MAX_ETHERNET_FRAME_SIZE_STANDARDIZED	1514
/*INTEGER  { noop ( 1 ) , lpbStart ( 2 ) , lpbStop ( 3 ) , inProcess ( 4 ) */
#define LOOP_NOOP							1
#define LOOP_START							2
#define LOOP_STOP							3
#define LOOP_PROCESS						4
#define LOOP_ALLTIME						0

/*{ internal ( 1 ) , external ( 2 ) } */
#define LOOP_INTERNAL						1
#define LOOP_EXTERNAL						2	

#define LOOP_TIMEOUT						4

#define LOOP_ONULOOPBACK_RXOK	18
#define LOOP_ONULOOPBACK_TXOK	19
#define LOOP_ONULOOPBACK_RXERR	20
#define LOOP_ONULOOPBACK_TOTALRXBAD	21


/* =============���⺯���ӿ�˵��=============*/

/**************************************************
* OnuLoopInit
* 	����: onu���ز���ģ���ʼ��������������ʱ����
*
*
*
*
*
***************************************************/
int OnuLoopInit(void);


/**************************************************
* OnuLoopPhyTestLpbCtrlSet
* ����: �ú������û���
*
*
*
*
*
***************************************************/
int OnuLoopPhyTestLpbCtrlSet( short int PonId, short int OnuId , short int loopEn);

/**************************************************
* OnuLoopPhyTestStatusGet
* ����: �ú�����ȡ����״̬
*
*
*
*
*
***************************************************/
int OnuLoopPhyTestLpbCtrlGet( short int PonId, short int OnuId , short int *pLoopEn);

/**************************************************
* OnuLoopLinkTestRxStatGet
* ����: �ú�����ȡ���ز����н��յ������ݰ�
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestRxStatGet(short int PonId, short int OnuId , long int *pRxStats);

/**************************************************
* OnuLoopLinkTestTxStatGet
* ����: �ú�����ȡ���ز����з��͵����ݰ�
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestTxStatGet(short int PonId, short int OnuId , long int *pRxStats);


/**************************************************
* OnuLoopLinkTestLpbScrSet
* 	����: �ú������û��ز����е�loopback Scr
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestLpbScrSet(short int PonId, short int OnuId , unsigned int lpbScr);

/**************************************************
* OnuLoopLinkTestLpbScrGet
* ����: �ú�����ȡ���ز����е�loopback Scr
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestLpbScrGet(short int PonId, short int OnuId , unsigned int *pLpbScr);

/**************************************************
*  OnuLoopLinkTestLpbTimeSet
* 	����: �ú������û��ز����е�loopback time
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestLpbTimeSet(short int PonId, short int OnuId , unsigned int lpbTime);

/**************************************************
* OnuLoopLinkTestLpbTimeGet
* 	����: �ú�����ȡ���ز����е�loopback time
*
*
*
*
*
***************************************************/
int OnuLoopLinkTestLpbTimeGet(short int PonId, short int OnuId , unsigned int *pLpbTime);




#ifdef __cplusplus
}
#endif
#endif

