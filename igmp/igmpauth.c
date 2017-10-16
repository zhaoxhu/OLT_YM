/* ================================================ */
/* FileName: IgmpAuth.c                             */
/* Author  : suxiqing                               */
/* Date    : 2006-10-10                             */
/* Description: 此文件仅为EPON设计, 实现通过OAM通道 */
/*              发送IGMP认证消息                    */
/* ================================================ */

#include "vos/vospubh/Vos_typevx.h"
#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_types.h"
#include "vos/vospubh/vos_sysmsg.h"
#include "vos/vospubh/vos_sem.h"
#include "vos/vospubh/vos_que.h"
#include "vos/vospubh/vos_string.h"
#include "vos/vospubh/vos_task.h"
#include "vos/vospubh/vos_que.h"
#include "vos/vospubh/vos_syslog.h"
#include "vos/vospubh/vos_timer.h"

#include "igmpauth.h"

#if 0 /* IGMP_AUTH_FOR_CTC */

#include "include/linux/list.h"

/*=================================================================================================================== */
/*=================================================================================================================== */
/*
#define LOG_TYPE_AUTH_IGMP           40*/

#define IGMP_AUTH_TIMER_LEN          1000  /* 定时器 */ 

#define IGMP_AUTH_RETRY_COU          3     /* 重发3次    */
#define IGMP_AUTH_RETRY_INT          5     /* 重发间隔5秒*/
/*=================================================================================================================== */
/* OAM帧类型定义  */

#define IGMP_AUTH_OPCODE_REQ         0x12
#define IGMP_AUTH_OPCODE_ACK         0x13

/*=================================================================================================================== */
/* 模块定义消息  */
#define IGMP_AUTH_MSGCODE_REQ        (AWMC_PRV_BASE | 0x0b0c)    
                                                 /* 认证请求         */
#define IGMP_AUTH_MSGCODE_ACK        (AWMC_PRV_BASE | 0x0b0d) 
                                                 /* 认证应答         */
#define IGMP_AUTH_MSGCODE_LEV        (AWMC_PRV_BASE | 0x0b0e)
                                                 /* 强制离开         */ 
#define IGMP_AUTH_MSGCODE_TIMER      (AWMC_PRV_BASE | 0x0b0f)

#define IGMP_AUTH_OPTYPE_REGREQ      1   /* 注册请求  */
#define IGMP_AUTH_OPTYPE_UNREGREQ    2   /* 注小请求  */
#define IGMP_AUTH_OPTYPE_LEAVEREQ    3   /* 强制离开  */

#define IGMP_AUTH_OPTYPE_REGACK      11  /* 注册请求  */
#define IGMP_AUTH_OPTYPE_UNREGACK    12  /* 注小请求  */
#define IGMP_AUTH_OPTYPE_LEAVEACK    13  /* 强制离开  */

#define IGMP_AUTH_PKTLEN_REQ         24
#define IGMP_AUTH_PKTLEN_ACK         26

#define IGMP_AUTH_REQRES_AGREE       1
#define IGMP_AUTH_REQRES_REJECT      2 

/*=================================================================================================================== */

/* IF DEBUG */
#define IGMP_AUTH_DEBUG(x)           do{ \
                                          if(glIgmpAuthDebug) {VOS_SysLog x;} \
                                       }while(0)
/* ELSE 
#define IGMP_AUTH_DEBUG(x)
*/
/*
#define IGMP_AUTH_PRINTPKT(type, buff, len) { \
	                                     long iLoop;\
	                                     if(type == 1) \
										 sys_console_printf("\r\nreceive pkt: \r\n"); \
										 else \
										 sys_console_printf("\r\nsend pkt:\r\n"); \
                                         for(iLoop = 0; iLoop < len; iLoop ++) \
										 sys_console_printf("%02x ", buff[iLoop]);\
										 sys_console_printf("\r\n"); \
                                       }
*/

#define IGMP_AUTH_PRINTPKT(type, buff, len)  

/*=================================================================================================================== */
/* 实际报文字段偏移  */
#define IGMP_AUTH_FILED_OFF_TYPE     0
#define IGMP_AUTH_FILED_OFF_ONUID    2
#define IGMP_AUTH_FILED_OFF_VID      4
#define IGMP_AUTH_FILED_OFF_MCIP     6
#define IGMP_AUTH_FILED_OFF_SPORT    10
#define IGMP_AUTH_FILED_OFF_SIP      14
#define IGMP_AUTH_FILED_OFF_SMAC     18
#define IGMP_AUTH_FILED_OFF_RESU     24

/*=================================================================================================================== */

typedef struct _IGMP_AUTH_REQ_S_
{
    unsigned short       usType;    
    
    unsigned short       usPonId;
	unsigned short       usOnuId;
	unsigned short       usVlanId;
	unsigned long        ulMcIP;
    unsigned long        ulSrcPort;
    unsigned long        ulSrcIP;
	unsigned char        bMacAddr[6];
	unsigned short       usResult;

    char                 bSessId[8];           /* 会话句柄  只用四字节 */

    IGMPAUTHLEAVECALLBACK pCallBack;

}IGMP_AUTH_REQ_S;

typedef struct _IGMP_AUTH_CTL_S_
{
    struct list_head     sLinkList;       
    

    long                 lRepCoun;
    long                 lTimeOut;

    long                 lErrorNo;

    IGMP_AUTH_REQ_S      sReqInfo;
		
}IGMP_AUTH_CTL_S;

/*=================================================================================================================== */

static long             glIgmpAuthDebug     = 0;    /* 调试开关       */
static long             gulIgmpAuthSeID     = 0;    /* 会话ID         */
static long             glIgmpTimerId       = 0;    
static unsigned long    gulIgmpAuthQid      = 0;    /* 传送任务队列   */

static struct list_head gsIgmpAuthLink;

/*=================================================================================================================== */

extern int  CommOltMsgRvcCallbackInit(unsigned char GwProId, void *Function);

extern int  Comm_IGMPAuth_Reponse(const unsigned short PonId, const unsigned short OnuId,
								  unsigned char *pIgmpAuth,	const unsigned short igmp_data_size,
								  unsigned char *psessionIdField);
extern int  Comm_IGMPAuth_Requst(const unsigned short PonId, const unsigned short OnuId,
								 unsigned char *pIgmpAuth, const unsigned short igmp_data_size,
								 unsigned char *psessionIdField);
uint_t checkIgmpSnoopAuthItem( ulong_t act, ulong_t ponid, ulong_t onuid, ulong_t vlanid, 
	                                   ulong_t groupip, uchar_t usermac[], ulong_t srcPort );
/*=================================================================================================================== */
/*=================================================================================================================== */
/*API 部分   */
/*================================================*/
/*name: funIgmpAuthOnuLevCfgApi                   */
/*para: usPonID    PON 号                         */ 
/*para: usOnuID    ONU 号                         */ 
/*para: usVid      VLAN 号                        */ 
/*para: ulMcIP     组播组地址                     */
/*para: ulScrPort  ONU上的源端口号                */
/*para: ulScrIP    ONU上的源端口的IP地址          */
/*para: ulScrMac   ONU上的源端口的MAC地址         */
/*retu: VOS_OK / VOS_ERROR                        */
/*desc: 调用该函数向指定ONU发送强制离开消息       */
/*================================================*/
   
long funIgmpAuthUsrLevCfgApi(unsigned short usPonID, unsigned short usOnuID,   unsigned short usVid,  
                             unsigned long  ulMcIP,  unsigned long  ulScrPort, unsigned long  ulScrIP, 
                             unsigned char *pScrMac, IGMPAUTHLEAVECALLBACK pCallBack)
{
    unsigned long     ulMessage[4] = {0};
    SYS_MSG_S        *pQueMsg      = NULL;
    IGMP_AUTH_REQ_S  *pAuthReq     = NULL;

    pQueMsg = (SYS_MSG_S*)VOS_Malloc(sizeof(SYS_MSG_S) + sizeof(IGMP_AUTH_REQ_S) , MODULE_IGMPAUTH);
    
    if(pQueMsg == NULL)
    {
        IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Malloc memory faile in API fun.\r\n"));

		/*add by wangxy 2006-11-23*/
        return VOS_ERROR;
    }

    pAuthReq  = (IGMP_AUTH_REQ_S*)(pQueMsg + 1);
    
    VOS_MemZero(pQueMsg, sizeof(SYS_MSG_S) + sizeof(IGMP_AUTH_REQ_S));

    pQueMsg->ucMsgType      = MSG_NOTIFY;
    pQueMsg->ulSrcSlotID    = 0;
    pQueMsg->ulDstSlotID    = 0; 
    pQueMsg->usMsgCode      = IGMP_AUTH_MSGCODE_LEV;
    pQueMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;
    pQueMsg->usFrameLen     = 0;
    pQueMsg->ptrMsgBody     = 0;

	pAuthReq->usType     = IGMP_AUTH_OPTYPE_LEAVEREQ;
    pAuthReq->usPonId    = usPonID;
    pAuthReq->usOnuId    = usOnuID;
    pAuthReq->usVlanId   = usVid;
    pAuthReq->ulMcIP     = ulMcIP;
    pAuthReq->ulSrcPort  = ulScrPort;
    pAuthReq->ulSrcIP    = ulScrIP;
    pAuthReq->usResult   = 0;
    pAuthReq->pCallBack  = pCallBack;
	
	*((unsigned long*)pAuthReq->bSessId) = ++gulIgmpAuthSeID;

	VOS_MemCpy(pAuthReq->bMacAddr, pScrMac, 6);

    ulMessage[3] = (unsigned long)pQueMsg;

    if(VOS_QueSend(gulIgmpAuthQid, ulMessage, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
    {
        VOS_Free(pQueMsg);
         
        IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Send msg  fail in API\r\n"));

        return(VOS_ERROR);
    }
    else 
	{
        IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Send leave msg \r\n"));
	
		return (VOS_OK);
    }
}

/*=================================================================================================================== */
/*=================================================================================================================== */
/* OAM接收回调函数  */
/*================================================*/
/*name: funIgmpAuthRevPktCheck                    */
/*para: lDatLen  数据长度                         */
/*para: pDatBuf  数据指针                         */
/*para: pcSessId 会话ID                           */
/*retu: VOS_OK/VOS_ERROR                          */
/*desc: 组播认证报文的回调函数                    */
/*================================================*/
static long funIgmpAuthRevPktCheck(unsigned short usReqTyp, unsigned short usDatLen, 
                                   unsigned char *pDatBuf,  unsigned char *pSessId)
{
    if((pDatBuf == NULL) || (pSessId == NULL))
    {
        IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "pDatBuf=NULL  or pSessId=NULL in callback fun.\r\n"));
        
        return(VOS_ERROR);
    }

    if((usReqTyp == IGMP_AUTH_MSGCODE_REQ) && (usDatLen != IGMP_AUTH_PKTLEN_REQ))
    {
        IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Receive req pkt dataLen error.\r\n"));
        VOS_Free(pDatBuf);
        VOS_Free(pSessId);
        return(VOS_ERROR);
    }

    if((usReqTyp == IGMP_AUTH_MSGCODE_ACK) && (usDatLen != IGMP_AUTH_PKTLEN_ACK))
    {
        IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Receive ack pkt dataLen error.\r\n"));
        VOS_Free(pDatBuf);
        VOS_Free(pSessId);
        return(VOS_ERROR);
    }
	
    
    return(VOS_OK);
}

/*================================================*/
/*name: funIgmpAuthAllCallBack                    */
/*para: usReqTyp                                  */ 
/*para: usPonId                                   */ 
/*para: usOnuId                                   */ 
/*para: lDatLen  数据长度                         */
/*para: pDatBuf  数据指针                         */
/*para: pcSessId 会话ID                           */
/*retu: none                                      */
/*desc: 组播认证报文的回调函数                    */
/*================================================*/
static void funIgmpAuthAllCallBack(unsigned short usReqTyp, unsigned short usPonId,  unsigned short usOnuId,
                                   unsigned short usDatLen, unsigned char *pDatBuf,  unsigned char *pSessId)
{
    unsigned long     ulMessage[4] = {0};
    SYS_MSG_S        *pQueMsg      = NULL;
    IGMP_AUTH_REQ_S  *pAuthReq     = NULL;

    IGMP_AUTH_PRINTPKT(1, pSessId, 8);
    IGMP_AUTH_PRINTPKT(1, pDatBuf, usDatLen);

    if(funIgmpAuthRevPktCheck(usReqTyp, usDatLen, pDatBuf, pSessId) == VOS_ERROR) return;

    pQueMsg = (SYS_MSG_S*)VOS_Malloc(sizeof(SYS_MSG_S) + sizeof(IGMP_AUTH_REQ_S) , MODULE_IGMPAUTH);
    
    if(pQueMsg == NULL)
    {
        IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Malloc memory faile in API fun.\r\n"));

        return;
    }

    pAuthReq  = (IGMP_AUTH_REQ_S*)(pQueMsg + 1);
    
    VOS_MemZero(pQueMsg, sizeof(SYS_MSG_S) + sizeof(IGMP_AUTH_REQ_S));

    pQueMsg->ucMsgType      = MSG_NOTIFY;
    pQueMsg->ulSrcSlotID    = 0;
    pQueMsg->ulDstSlotID    = 0; 
    pQueMsg->usMsgCode      = usReqTyp;
    pQueMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;
    pQueMsg->usFrameLen     = 0;
    pQueMsg->ptrMsgBody     = 0;

    pAuthReq->usPonId    = usPonId;
    pAuthReq->usOnuId    = usOnuId;
	pAuthReq->usType     = *(unsigned short*)(pDatBuf + IGMP_AUTH_FILED_OFF_TYPE);
    pAuthReq->usVlanId   = *(unsigned short*)(pDatBuf + IGMP_AUTH_FILED_OFF_VID);
    pAuthReq->ulMcIP     = *(unsigned long *)(pDatBuf + IGMP_AUTH_FILED_OFF_MCIP);
    pAuthReq->ulSrcPort  = *(unsigned long *)(pDatBuf + IGMP_AUTH_FILED_OFF_SPORT);
    pAuthReq->ulSrcIP    = *(unsigned long *)(pDatBuf + IGMP_AUTH_FILED_OFF_SIP); 

	if(usReqTyp == IGMP_AUTH_MSGCODE_ACK) 
    {
        pAuthReq->usResult = *(unsigned short*)(pDatBuf + IGMP_AUTH_FILED_OFF_RESU);
	}

    VOS_MemCpy(pAuthReq->bSessId,  pSessId, 8);  
	VOS_MemCpy(pAuthReq->bMacAddr, pDatBuf + IGMP_AUTH_FILED_OFF_SMAC, 6);

    ulMessage[3] = (unsigned long)pQueMsg;

    if(VOS_QueSend(gulIgmpAuthQid, ulMessage, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
    {
        VOS_Free(pQueMsg);
         
        IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Send msg  fail in API\r\n"));

    }

	IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Send req Or ack msg to queue, type = 0x%04x\r\n", usReqTyp));
	
    VOS_Free(pDatBuf);
    VOS_Free(pSessId);
}

/*================================================*/
/*name: funIgmpAuthReqCallBack                    */
/*para: usPonId                                   */ 
/*para: usOnuId                                   */ 
/*para: lDatLen  数据长度                         */
/*para: pDatBuf  数据指针                         */
/*para: pcSessId 会话ID                           */
/*retu: none                                      */
/*desc: 组播认证报文的回调函数                    */
/*================================================*/
void funIgmpAuthReqCallBack(unsigned short usPonId,  unsigned short usOnuId,
                            unsigned short usDatLen, unsigned char *pDatBuf,  unsigned char *pSessId)
{
    funIgmpAuthAllCallBack(IGMP_AUTH_MSGCODE_REQ, usPonId, usOnuId, usDatLen, pDatBuf, pSessId);
}

/*================================================*/
/*name: funIgmpAuthAckCallBack                    */
/*para: usPonId                                   */ 
/*para: usOnuId                                   */ 
/*para: lDatLen  数据长度                         */
/*para: pDatBuf  数据指针                         */
/*para: pcSessId 会话ID                           */
/*retu: none                                      */
/*desc: 组播认证报文的回调函数                    */
/*================================================*/
void funIgmpAuthAckCallBack(unsigned short usPonId,  unsigned short usOnuId,
                            unsigned short usDatLen, unsigned char *pDatBuf,  unsigned char *pSessId)
{
    funIgmpAuthAllCallBack(IGMP_AUTH_MSGCODE_ACK, usPonId, usOnuId, usDatLen, pDatBuf, pSessId);
}

/*=================================================================================================================== */
/*=================================================================================================================== */
/* 定时器回调 */
/*================================================*/
/*name: funIgmpAuthTimerCallback                  */
/*para: void *                                    */
/*retu: none                                      */
/*desc: 任务定时器回调函数                        */
/*================================================*/
static void funIgmpAuthTimerCallback(void *pArg)
{
    SYS_MSG_S    *pSysMsg = NULL;
    unsigned long ulMessage[4] = {0};

    pSysMsg = (SYS_MSG_S*)VOS_Malloc(sizeof(SYS_MSG_S), MODULE_IGMPAUTH);
    
    if(NULL == pSysMsg)
    {
        ASSERT(0);
        return;
    }

    VOS_MemZero(pSysMsg, sizeof(SYS_MSG_S));

    pSysMsg->ucMsgType       = MSG_TIMER;
    pSysMsg->usMsgCode       = IGMP_AUTH_MSGCODE_TIMER;
    pSysMsg->ucMsgBodyStyle  = MSG_BODY_INTEGRATIVE;
    
    ulMessage[3] = (unsigned long)pSysMsg;

    if(VOS_QueSend(gulIgmpAuthQid,  ulMessage, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
    {
        VOS_Free(pSysMsg);
        ASSERT(0);
        return;
    }
}


/*=================================================================================================================== */
/*=================================================================================================================== */
/* 发送消息 */
/*================================================*/
/*name: funIgmpAuthSendAckPkt                     */
/*para: pReqInfo 消息缓冲区                       */
/*retu: none                                      */
/*desc: 发送应答报文                              */
/*================================================*/
static void funIgmpAuthSendAckPkt(IGMP_AUTH_REQ_S *pReqInfo)
{
    unsigned char bPktBuf[32];
    
    *((unsigned short *)(bPktBuf + IGMP_AUTH_FILED_OFF_TYPE))  = pReqInfo->usType;
    *((unsigned short *)(bPktBuf + IGMP_AUTH_FILED_OFF_ONUID)) = pReqInfo->usOnuId;
    *((unsigned short *)(bPktBuf + IGMP_AUTH_FILED_OFF_VID))   = pReqInfo->usVlanId;
    *((unsigned long  *)(bPktBuf + IGMP_AUTH_FILED_OFF_MCIP))  = pReqInfo->ulMcIP;
    *((unsigned long  *)(bPktBuf + IGMP_AUTH_FILED_OFF_SPORT)) = pReqInfo->ulSrcPort;
    *((unsigned long  *)(bPktBuf + IGMP_AUTH_FILED_OFF_SIP))   = pReqInfo->ulSrcIP;
    *((unsigned short *)(bPktBuf + IGMP_AUTH_FILED_OFF_RESU))  = pReqInfo->usResult;

    VOS_MemCpy(bPktBuf + IGMP_AUTH_FILED_OFF_SMAC, pReqInfo->bMacAddr, 6);

    Comm_IGMPAuth_Reponse(pReqInfo->usPonId, pReqInfo->usOnuId, bPktBuf, IGMP_AUTH_PKTLEN_ACK, pReqInfo->bSessId);
    
    IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Send ack to remote\r\n"));

    /*VOS_TaskDelay(1);*/ 

	IGMP_AUTH_PRINTPKT(2, pReqInfo->bSessId, 8);

	IGMP_AUTH_PRINTPKT(2, bPktBuf, 26);
}

/*================================================*/
/*name: funIgmpAuthEndLevReqInst                  */
/*para: pCtrItem 消息缓冲区                       */
/*retu: none                                      */
/*desc: 结束一个发起离开请求实例                  */
/*================================================*/
static void funIgmpAuthEndLevReqInst(IGMP_AUTH_CTL_S *pCtrItem)
{
    if(pCtrItem->sReqInfo.pCallBack != NULL)  
    {
        pCtrItem->sReqInfo.pCallBack(pCtrItem->sReqInfo.usPonId,   pCtrItem->sReqInfo.usOnuId,
                                     pCtrItem->sReqInfo.usVlanId,  pCtrItem->sReqInfo.ulMcIP,
                                     pCtrItem->sReqInfo.ulSrcPort, pCtrItem->sReqInfo.ulSrcIP,
                                     pCtrItem->sReqInfo.bMacAddr,  pCtrItem->sReqInfo.usResult);
    }

    IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Delete leave req instance \r\n"));

    list_del(&pCtrItem->sLinkList);

    if(list_empty(&gsIgmpAuthLink))
    {
       IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Leave req control list is empty\r\n"));
    }

    VOS_Free(pCtrItem);
}

/*================================================*/
/*name: funIgmpAuthSendLevPkt                     */
/*para: pReqInfo 消息缓冲区                       */
/*retu: none                                      */
/*desc: 发送应答报文                              */
/*================================================*/

static void funIgmpAuthSendLevPkt(IGMP_AUTH_CTL_S *pCtrItem)
{
    pCtrItem->lRepCoun ++;

    if(pCtrItem->lRepCoun > IGMP_AUTH_RETRY_COU)
    {
        IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Send leave req to remote trying fail\r\n"));

        pCtrItem->lErrorNo = IGMP_AUTH_ERROR_LEVERR;

        funIgmpAuthEndLevReqInst(pCtrItem);
    }
    else
    {
        unsigned char bPktBuf[32];

        pCtrItem->lTimeOut    = IGMP_AUTH_RETRY_INT; /* 启动定时器  */

        *((unsigned short *)(bPktBuf + IGMP_AUTH_FILED_OFF_TYPE))  = pCtrItem->sReqInfo.usType;
        *((unsigned short *)(bPktBuf + IGMP_AUTH_FILED_OFF_ONUID)) = pCtrItem->sReqInfo.usOnuId;
        *((unsigned short *)(bPktBuf + IGMP_AUTH_FILED_OFF_VID))   = pCtrItem->sReqInfo.usVlanId;
        *((unsigned long  *)(bPktBuf + IGMP_AUTH_FILED_OFF_MCIP))  = pCtrItem->sReqInfo.ulMcIP;
        *((unsigned long  *)(bPktBuf + IGMP_AUTH_FILED_OFF_SPORT)) = pCtrItem->sReqInfo.ulSrcPort;
        *((unsigned long  *)(bPktBuf + IGMP_AUTH_FILED_OFF_SIP))   = pCtrItem->sReqInfo.ulSrcIP;

        VOS_MemCpy(bPktBuf + IGMP_AUTH_FILED_OFF_SMAC, pCtrItem->sReqInfo.bMacAddr, 6);

        Comm_IGMPAuth_Requst(pCtrItem->sReqInfo.usPonId, pCtrItem->sReqInfo.usOnuId, bPktBuf, 
			                 IGMP_AUTH_PKTLEN_REQ, pCtrItem->sReqInfo.bSessId);

        IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Send leave req to remote try cou = %d \r\n", pCtrItem->lRepCoun));

        /*VOS_TaskDelay(1); */

		IGMP_AUTH_PRINTPKT(2, pCtrItem->sReqInfo.bSessId, 8);

	    IGMP_AUTH_PRINTPKT(2, bPktBuf, 24);
    }
}


/*=================================================================================================================== */
/*=================================================================================================================== */

/* 消息处理函数  */
/*================================================*/
/*name: funIgmpAuthProcRegReq                     */
/*para: pReqInfo 消息缓冲区                       */
/*retu: none                                      */
/*desc: 注册请求处理函数                          */
/*================================================*/
void funIgmpAuthProcRegReq(IGMP_AUTH_REQ_S *pReqInfo)
{
    if(checkIgmpSnoopAuthItem(1, (unsigned long)pReqInfo->usPonId,
		                     (unsigned long)pReqInfo->usOnuId,
                             (unsigned long)pReqInfo->usVlanId,
                              pReqInfo->ulMcIP, pReqInfo->bMacAddr,
                              pReqInfo->ulSrcPort) == VOS_OK)
    {
        pReqInfo->usResult = IGMP_AUTH_REQRES_AGREE;
    }
    else
    {
        pReqInfo->usResult = IGMP_AUTH_REQRES_REJECT;

    }
	pReqInfo->usType = IGMP_AUTH_OPTYPE_REGACK;
    
    funIgmpAuthSendAckPkt(pReqInfo);
}

/*================================================*/
/*name: funIgmpAuthProcUnRegReq                   */
/*para: pReqInfo 消息缓冲区                       */
/*retu: none                                      */
/*desc: 注销请求处理函数                          */
/*================================================*/
void funIgmpAuthProcUnRegReq(IGMP_AUTH_REQ_S *pReqInfo)
{
    if(checkIgmpSnoopAuthItem(2, (unsigned long)pReqInfo->usPonId,
		                      (unsigned long)pReqInfo->usOnuId,
                              (unsigned long)pReqInfo->usVlanId,
                               pReqInfo->ulMcIP, pReqInfo->bMacAddr,
                               pReqInfo->ulSrcPort) == VOS_OK)
    {        
        pReqInfo->usResult = IGMP_AUTH_REQRES_AGREE;
    }
    else
    {
        pReqInfo->usResult = IGMP_AUTH_REQRES_REJECT;
    }
     
  	pReqInfo->usType = IGMP_AUTH_OPTYPE_UNREGACK;

    funIgmpAuthSendAckPkt(pReqInfo);
}

/*================================================*/
/*name: funIgmpAuthProcLevAck                     */
/*para: pReqInfo 消息缓冲区                       */
/*retu: none                                      */
/*desc: 强制离开应答处理函数                      */
/*================================================*/
void funIgmpAuthProcLevAck(IGMP_AUTH_REQ_S *pReqInfo)
{
    struct list_head       *pListhead = &gsIgmpAuthLink;
    struct list_head       *pListItem = NULL;
    IGMP_AUTH_CTL_S        *pCtrlItem = NULL;
   
    if(list_empty(pListhead)) return;

    for(pListItem = pListhead->next; pListItem != pListhead; pListItem = pListItem->next)
    {
        pCtrlItem = list_entry(pListItem, IGMP_AUTH_CTL_S, sLinkList);
        
        if(VOS_MemCmp(pCtrlItem->sReqInfo.bSessId, pReqInfo->bSessId, 8) == 0) break;
    }

    if(pListItem != pListhead)
    {
		IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Match a leave req instance\r\n"));

        pCtrlItem->lErrorNo = IGMP_AUTH_ERROR_NO;
        
        funIgmpAuthEndLevReqInst(pCtrlItem);
    }
	else
    {
		IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Not Match a leave req instance\r\n"));
    }
}

/*================================================*/
/*name: funIgmpAuthProcLevReq                     */
/*para: pReqInfo 消息缓冲区                       */
/*retu: none                                      */
/*desc: 强制离开应答处理函数                      */
/*================================================*/
void funIgmpAuthProcLevReq(IGMP_AUTH_REQ_S *pReqInfo)
{
    IGMP_AUTH_CTL_S *pCtrlItem = NULL;

    pCtrlItem = (IGMP_AUTH_CTL_S*)VOS_Malloc(sizeof(IGMP_AUTH_CTL_S), MODULE_IGMPAUTH);

    if(pCtrlItem == NULL)
    {
        IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Malloc mem fail"));
        
        if(pReqInfo->pCallBack != NULL)
        {
            pReqInfo->pCallBack(pReqInfo->usPonId,  pReqInfo->usOnuId,   pReqInfo->usVlanId,
                                pReqInfo->ulMcIP,   pReqInfo->ulSrcPort, pReqInfo->ulSrcIP,
                                pReqInfo->bMacAddr, IGMP_AUTH_ERROR_LEVERR);
        }
    }
    else
    {
        VOS_MemZero(pCtrlItem, sizeof(IGMP_AUTH_CTL_S));
       
        VOS_MemCpy(&pCtrlItem->sReqInfo, pReqInfo, sizeof(IGMP_AUTH_REQ_S));

        list_add_tail(&pCtrlItem->sLinkList, &gsIgmpAuthLink);

        funIgmpAuthSendLevPkt(pCtrlItem);
    }
}
/*================================================*/
/*name: funIgmpAuthProcReqMsg                     */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: ONU请求消息处理                           */
/*================================================*/
void funIgmpAuthProcReqMsg(SYS_MSG_S *pMsgBuf)
{
     IGMP_AUTH_REQ_S *pReqInf = NULL;
     
     pReqInf = (IGMP_AUTH_REQ_S*)(pMsgBuf + 1);

     IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Req ponid = %d. onuid= %d\r\n", pReqInf->usPonId,pReqInf->usOnuId));

     switch(pReqInf->usType)
     {
         case IGMP_AUTH_OPTYPE_REGREQ:
              funIgmpAuthProcRegReq(pReqInf);
              break;

         case IGMP_AUTH_OPTYPE_UNREGREQ:
              funIgmpAuthProcUnRegReq(pReqInf);
              break;

         default:
              IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Receive error req type code = %d\r\n", pReqInf->usType));
              break;
     }
}

/*================================================*/
/*name: funIgmpAuthProcAckMsg                     */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: ONU应答消息处理                           */
/*================================================*/
void funIgmpAuthProcAckMsg(SYS_MSG_S *pMsgBuf)
{
     IGMP_AUTH_REQ_S *pReqInf = NULL;
     
     pReqInf = (IGMP_AUTH_REQ_S*)(pMsgBuf + 1);

     switch(pReqInf->usType)
     {
         case IGMP_AUTH_OPTYPE_LEAVEACK:
              funIgmpAuthProcLevAck(pReqInf);
              break;

         case IGMP_AUTH_OPTYPE_REGACK:
              /* 不能收到该类型消息 */

         case IGMP_AUTH_OPTYPE_UNREGACK:
              /* 不能收到该类型消息 */

         default:
              IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Receive error ack type code = %d\r\n", pReqInf->usType));
              break;
     }
}

/*================================================*/
/*name: funIgmpAuthProcLevMsg                     */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 网管强制离开消息处理                      */
/*================================================*/
void funIgmpAuthProcLevMsg(SYS_MSG_S *pMsgBuf)
{
     IGMP_AUTH_REQ_S *pReqInf = NULL;
     
     pReqInf = (IGMP_AUTH_REQ_S*)(pMsgBuf + 1);

     switch(pReqInf->usType)
     {
         case IGMP_AUTH_OPTYPE_LEAVEREQ:
              funIgmpAuthProcLevReq(pReqInf);
              break;

         default:
              IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Receive error req type code = %d\r\n", pReqInf->usType));
              break;
     }
}


/*================================================*/
/*name: funOamTransFileProcTimerMsg               */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 接收的ACK报文消息处理函数                 */
/*================================================*/
static void funIgmpAuthProcTimerMsg(SYS_MSG_S *pMsgBuf)
{
    struct list_head       *pListhead = &gsIgmpAuthLink;
    struct list_head       *pListNext = NULL;
    struct list_head       *pListCurr = NULL;
    IGMP_AUTH_CTL_S        *pCtrlItem = NULL;

    if(list_empty(pListhead)) return;

    pListCurr = pListhead->next;

    while(pListCurr != pListhead)
    {
        pListNext = pListCurr->next;             /* 先保存下一个, 避免但前删除而出现错误 */ 
        
        pCtrlItem = list_entry(pListCurr, IGMP_AUTH_CTL_S, sLinkList);

        if(pCtrlItem->lTimeOut > 0)
        {
            pCtrlItem->lTimeOut --;             /* 重发超时      */

            if(pCtrlItem->lTimeOut == 0)  funIgmpAuthSendLevPkt(pCtrlItem);
        }

        pListCurr = pListNext;
    }
}

/*=================================================================================================================== */
/*=================================================================================================================== */

/*================================================*/
/*name: funIgmpAuthProcMessages                   */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 任务的输入消息处理主函数                  */
/*================================================*/
static void funIgmpAuthProcMessages(unsigned char *pMsgBuf)
{
    SYS_MSG_S *pSysMsg = (SYS_MSG_S *)pMsgBuf;
    
    switch(pSysMsg->usMsgCode)   
    {
        case IGMP_AUTH_MSGCODE_REQ:
             funIgmpAuthProcReqMsg(pSysMsg); 
             break;

        case IGMP_AUTH_MSGCODE_ACK:
             funIgmpAuthProcAckMsg(pSysMsg); 
             break;

        case IGMP_AUTH_MSGCODE_LEV:
             funIgmpAuthProcLevMsg(pSysMsg);
             break;
        case IGMP_AUTH_MSGCODE_TIMER:
             funIgmpAuthProcTimerMsg(pSysMsg);
             break;
        default: break;
    }

    if((pSysMsg->ptrMsgBody != NULL ) && 
       (pSysMsg->ucMsgBodyStyle == MSG_BODY_MULTI)) VOS_Free(pSysMsg->ptrMsgBody);

    VOS_Free(pMsgBuf);
}


/*=================================================================================================================== */
/*=================================================================================================================== */
/* 任务入口           */
/*================================================*/
/*name: funIgmpAuthTask                           */
/*retu: none                                      */
/*desc: 任务入口                                  */
/*================================================*/
DECLARE_VOS_TASK(funIgmpAuthTask)
{
    unsigned long ulMessage[4] = {0};
	/*
    VOS_TaskDelay(500);
    CommOltMsgRvcCallbackInit(IGMP_AUTH_OPCODE_REQ, funIgmpAuthReqCallBack);
    CommOltMsgRvcCallbackInit(IGMP_AUTH_OPCODE_ACK, funIgmpAuthAckCallBack);
    */
    while(1)
    {
        if(VOS_QueReceive(gulIgmpAuthQid, ulMessage, WAIT_FOREVER) != VOS_ERROR)
        {
            if(ulMessage[3] != 0)
            {
                funIgmpAuthProcMessages((char*)ulMessage[3]);
            }
            else
            {
                IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT,
                                 "Recv Message with no any data para in ulMessage[3]\r\n"));
            }
        }
        else
        {
            IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT, "Recv Message error\r\n"));
        }
    }
}


/*================================================*/
/*name: funIgmpAuthInit                           */
/*retu: VOS_OK / VOS_ERROR                        */
/*desc: 该函数用于初始化文件传送程序              */
/*================================================*/
long funIgmpAuthInit(void)
{
    INIT_LIST_HEAD(&gsIgmpAuthLink);

    gulIgmpAuthQid = VOS_QueCreate(IGMP_AUTH_QUEUE_LEN, VOS_MSG_Q_FIFO);

    if(gulIgmpAuthQid == 0)    
    {
        sys_console_printf("Init Igmp auth Module fail\r\n");
        return(VOS_ERROR);
    }
    
    CommOltMsgRvcCallbackInit(IGMP_AUTH_OPCODE_REQ, funIgmpAuthReqCallBack);
    CommOltMsgRvcCallbackInit(IGMP_AUTH_OPCODE_ACK, funIgmpAuthAckCallBack);
    
    if(VOS_TaskCreate(IGMP_AUTH_TASKNAME, IGMP_AUTH_TASKPRI, funIgmpAuthTask, NULL) == 0)
    {
        sys_console_printf("Create Igmp auth task fail\r\n");
        return(VOS_ERROR);
    } 
    
    glIgmpTimerId = VOS_TimerCreate(MODULE_IGMPAUTH, 0, IGMP_AUTH_TIMER_LEN,  
                                    funIgmpAuthTimerCallback, NULL, VOS_TIMER_LOOP);

    if(glIgmpTimerId == VOS_ERROR) return(VOS_ERROR);

    return(VOS_OK);
}

/*=================================================================================================================== */
/*=================================================================================================================== */
/* 调试程序  */

void sendonureq(char type,long len)
{
   unsigned char *buf = NULL;
   unsigned char *sid = NULL;
   
   buf = (char*) VOS_Malloc(32, MODULE_IGMPAUTH);
   sid = (char*) VOS_Malloc(8, MODULE_IGMPAUTH);

   buf[0] = 0;
   buf[1] = type;
   buf[2] = 0;
   buf[3] = 1;
   buf[4] = 0;
   buf[5] = 2;
   buf[6] = 224;
   buf[7] = 1;
   buf[8] = 2;
   buf[9] = 3;
   buf[10] = 0;
   buf[11] = 0;
   buf[12] = 0;
   buf[13] = 1;
   buf[14] = 192;
   buf[15] = 168;
   buf[16] = 1;
   buf[17] = 3;
   buf[18] = 0;
   buf[19] = 2;
   buf[20] = 2;
   buf[21] = 2;
   buf[22] = 3;
   buf[23] = 3;

   *(unsigned long*)sid = 1;
   *(unsigned long*)(sid + 4) = 0;

   funIgmpAuthReqCallBack(1,  1, len , buf,  sid);
}

void sendonuack(char type,long len)
{
   unsigned char *buf = NULL;
   unsigned char *sid = NULL;
   
   buf = (char*) VOS_Malloc(32, MODULE_IGMPAUTH);
   sid = (char*) VOS_Malloc(8, MODULE_IGMPAUTH);

   buf[0] = 0;
   buf[1] = type;
   buf[2] = 0;
   buf[3] = 1;
   buf[4] = 0;
   buf[5] = 2;
   buf[6] = 224;
   buf[7] = 7;
   buf[8] = 7;
   buf[9] = 7;
   buf[10] = 0;
   buf[11] = 0;
   buf[12] = 0;
   buf[13] = 1;
   buf[14] = 192;
   buf[15] = 168;
   buf[16] = 1;
   buf[17] = 3;
   buf[18] = 0;
   buf[19] = 2;
   buf[20] = 2;
   buf[21] = 2;
   buf[22] = 3;
   buf[23] = 3;
   buf[24] = 0;
   buf[25] = 1;

   *(unsigned long*)sid = gulIgmpAuthSeID;
   *(unsigned long*)(sid + 4) = 0;

   funIgmpAuthAckCallBack(1,  1, len , buf,  sid);
}

void onucfg(short vid )
{
   unsigned char mac[6]={0};
   funIgmpAuthUsrLevCfgApi(1, 1,  vid,  3758556935, 1, 122222, mac, NULL);
}

/*=================================================================================================================== */
/*=================================================================================================================== */

#else

#include "switch/igmp_snoop/igmp_snoop.h"
#include "includeFromPas.h"
#include "ct_manage/CT_RMan_Main.h"
/*#include "ct_manage/CT_RMan_Multicast.h"*/

/*=================================================================================================================== */
/*=================================================================================================================== */
/* GW OAM FRAME */
#define IGMP_AUTH_FILED_OFF_TYPE     0
#define IGMP_AUTH_FILED_OFF_ONUID    2
#define IGMP_AUTH_FILED_OFF_VID      4
#define IGMP_AUTH_FILED_OFF_MCIP     6
#define IGMP_AUTH_FILED_OFF_SPORT    10
#define IGMP_AUTH_FILED_OFF_SIP      14
#define IGMP_AUTH_FILED_OFF_SMAC     18
#define IGMP_AUTH_FILED_OFF_RESU     24

/*=================================================================================================================== */
/* GW RESPOND TYPE*/
#define IGMP_AUTH_OAM_REG_ACK        11  /* 注册请求  */
#define IGMP_AUTH_OAM_UNREG_ACK      12  /* 注小请求  */
/*
#define IGMP_AUTH_GWRESP_LEAVEACK    13     强制离开  */

#define IGMP_AUTH_OAM_PKTLEN_REQ     24
#define IGMP_AUTH_OAM_PKTLEN_ACK     26

/*=================================================================================================================== */
/*=================================================================================================================== */
/* OAM 帧类型定义   */
#define IGMP_AUTH_OAM_OPCODE_REQ     0x12
#define IGMP_AUTH_OAM_OPCODE_ACK     0x13

/*=================================================================================================================== */
/*=================================================================================================================== */
#define IGMP_AUTH_MSGCODE_MANREQ    (AWMC_PRV_BASE | 0x0b0a)    
                                              /* 网管设置请求     */
#define IGMP_AUTH_MSGCODE_OAMREQ    (AWMC_PRV_BASE | 0x0b0c)    
                                              /* 认证请求         */
#define IGMP_AUTH_MSGCODE_OAMACK    (AWMC_PRV_BASE | 0x0b0d) 
                                              /* 认证应答         */
/*
#define IGMP_AUTH_MSGCODE_GWLEV     (AWMC_PRV_BASE | 0x0b0e)
*/
#define IGMP_AUTH_MSGCODE_TIMER     (AWMC_PRV_BASE | 0x0b0f)

/*=================================================================================================================== */
/*=================================================================================================================== */

#define IGMP_AUTH_TIMER_LEN         1000  /* 定时器 */ 

long                    glIgmpAuthDebug = 0;    /* 调试开关       */
long             glIgmpTimerId   = 0;    
unsigned long    gulIgmpAuthQid  = 0;    /* 传送任务队列   */


/*=================================================================================================================== */
/*=================================================================================================================== */


/* IF DEBUG */
/*#define IGMP_AUTH_DEBUG(x)           do{ \
                                           if(glIgmpAuthDebug) {sys_console_printf x;} \
                                       }while(0)*/
#define IGMP_AUTH_DEBUG(x)           do{ \
                                           if(glIgmpAuthDebug) {sys_console_printf x;} \
                                       }while(0)
/* ELSE 
#define IGMP_AUTH_DEBUG(x)
*/
/*
#define IGMP_AUTH_PRINTPKT(type, buff, len) { \
	                                     long iLoop;\
	                                     if(type == 1) \
										 sys_console_printf("\r\nreceive pkt: \r\n"); \
										 else \
										 sys_console_printf("\r\nsend pkt:\r\n"); \
                                         for(iLoop = 0; iLoop < len; iLoop ++) \
										 sys_console_printf("%02x ", buff[iLoop]);\
										 sys_console_printf("\r\n"); \
                                       }


#define IGMP_AUTH_PRINTPKT(type, buff, len)  
*/
/*=================================================================================================================== */
/*=================================================================================================================== */
extern LONG checkIgmpSnoopAuthItem(LONG  lPonID,    LONG *pOnuID, 
                                   LONG  lReqType,  LONG  lUserID, 
                                   ULONG ulGroupIP, LONG *pMVlan,
                                   LONG  *pOnuType, char *ucMac); /* modi by suxq 2007-05-08*/
/*函数返回认证结果 0 无相关项目  1  同意 2 拒绝  3 同意但不需要通知ONU*/
/*pPonID 输入PONID*/
/*pOnuID 输入输出ONUID*/
/*lReqType 输入类型 1 加入 2 离开*/
/*lUserID 输入CVLAN */
/*ulGroupIP 输人组播地址*/
/*pMVlan输入注册VLAN(大亚ONU)输出组播VLAN*/
/*pOnuType输出ONU类型*/
/*ucMac 输入注册MAC*/

extern LONG setIgmpSnoopAuthItem(LONG  lPonID,    LONG lOnuID, 
                                 LONG  lReqType,  LONG lUserID, 
                                 ULONG ulGroupIP, LONG lMVlan,
                                 char *ucMac,     LONG lResult);

extern LONG callIgmpSnoopAuthTimer(void);

/*LONG lResult 1 设置 ONU 成功 0 设置ONU 失败*/
void funIgmpAuthDebugSetApi(int iEnable)  
{
   glIgmpAuthDebug = iEnable;
}
/*=================================================================================================================== */
/*=================================================================================================================== */

/*---------------------------------------------*/
/*name: funIgmpAuthManagerSetApi               */
/*para: lReqType 1 join 2 leave                */
/*para: lUserID  用户端口或用户CVLANID         */
/*para: ulGroupIP 组播地址                     */
/*para: lMVlan    组播VLAN                     */
/*para: lOnuType  ONU类型 1 CTC 2 GW           */
/*para: bUsrMac   用户MAC                      */
/*desc: 网管修改用户权限表时, 同步ONU的API     */
/*---------------------------------------------*/
LONG funIgmpAuthManagerSetApi(LONG  lPonID,  LONG lOnuID,     LONG  lReqType,  
                              LONG  lUserID, ULONG ulGroupIP, LONG  lMVlan, LONG lOnuType, char *bUsrMac)
{

    ULONG            ulMsg[ 4 ] = { 0 };
    SYS_MSG_S       *pReqMsg    = NULL;
	Igmp_AuthInfo_t *pAuthIfo   = NULL;


	pReqMsg = ( SYS_MSG_S * ) VOS_Malloc( sizeof( SYS_MSG_S ) + sizeof(Igmp_AuthInfo_t), MODULE_IGMPAUTH );
    if(NULL == pReqMsg)
    {
        IGMP_AUTH_DEBUG(("Remote config ONU(manager API) g_malloc mem error\r\n"));
        return(VOS_ERROR);
    }
	
    VOS_MemZero(pReqMsg, sizeof( SYS_MSG_S ) + sizeof(Igmp_AuthInfo_t));

    pAuthIfo =(Igmp_AuthInfo_t*)(pReqMsg + 1);

    pAuthIfo->lIfIdx  = 0;
    pAuthIfo->lType   = lReqType;
	pAuthIfo->lResult = IGMP_AUTH_RESULT_NONE;
	pAuthIfo->lCvlan  = lUserID;
	pAuthIfo->lMvlan  = lMVlan;
	pAuthIfo->ulCip   = 0;
    pAuthIfo->lDevType= lOnuType;
	pAuthIfo->lUserA  = lPonID;
	pAuthIfo->lUserB  = lOnuID;
	pAuthIfo->ulMip   = ulGroupIP;
	pAuthIfo->pSkBuf  = NULL;

    memcpy(pAuthIfo->ucCmac, bUsrMac, 6); 
	
    pReqMsg->ptrMsgBody     = pAuthIfo;
    pReqMsg->usMsgCode      = IGMP_AUTH_MSGCODE_MANREQ;
    pReqMsg->ucMsgType      = MSG_NOTIFY;
    pReqMsg->ulDstModuleID  = MODULE_RPU_IGMPSNOOP;
    pReqMsg->ulSrcModuleID  = MODULE_IGMPAUTH;
    pReqMsg->ulSrcSlotID    = DEV_GetPhySlot();
    pReqMsg->ulSrcSlotID    = DEV_GetPhySlot();
    pReqMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;

    ulMsg[ 0 ] = pReqMsg->ulSrcModuleID;
    ulMsg[ 1 ] = 0;     /* not used now */
    ulMsg[ 2 ] = 0;
    ulMsg[ 3 ] = ( ULONG )pReqMsg;

    if(VOS_QueSend(gulIgmpAuthQid,  ulMsg, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
    {
         VOS_Free(pReqMsg);
         return(VOS_ERROR);
    }
	else return(VOS_OK);
}

/*=================================================================================================================== */
/*=================================================================================================================== */
/*================================================*/
/*name: funIgmpAuthTimerCallback                  */
/*para: void *                                    */
/*retu: none                                      */
/*desc: 任务定时器回调函数                        */
/*================================================*/

static void funIgmpAuthTimerCallback(void *pArg)
{

    SYS_MSG_S    *pSysMsg = NULL;
    unsigned long ulMessage[4] = {0};

    pSysMsg = (SYS_MSG_S*)VOS_Malloc(sizeof(SYS_MSG_S), MODULE_IGMPAUTH);
    
    if(NULL == pSysMsg)
    {
        ASSERT(0);
        return;
    }

    VOS_MemZero(pSysMsg, sizeof(SYS_MSG_S));

    pSysMsg->ucMsgType       = MSG_TIMER;
    pSysMsg->usMsgCode       = IGMP_AUTH_MSGCODE_TIMER;
    pSysMsg->ucMsgBodyStyle  = MSG_BODY_INTEGRATIVE;
    
    ulMessage[3] = (unsigned long)pSysMsg;

    if(VOS_QueSend(gulIgmpAuthQid,  ulMessage, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
    {
        VOS_Free(pSysMsg);
        /*ASSERT(0);*/
        return;
    }
}


/*================================================*/
/*name: funOamTransFileProcTimerMsg               */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 接收的ACK报文消息处理函数                 */
/*================================================*/

static void funIgmpAuthProcTimerMsg(SYS_MSG_S *pMsgBuf)
{
     /* 处理定时事件 处理认证表等*/

	 /* 在此处增加接口函数   */
    
    callIgmpSnoopAuthTimer();
    
	if((pMsgBuf->ptrMsgBody != NULL ) &&
	   (pMsgBuf->ucMsgBodyStyle == MSG_BODY_MULTI)) 
	{
	 	VOS_Free(pMsgBuf->ptrMsgBody);
	}

	VOS_Free(pMsgBuf);
}
/*=================================================================================================================== */
/*=================================================================================================================== */

/*================================================*/
/*name: funIgmpAuthCtcOnuReqResp                  */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: ONU的请求应答CTC类                        */
/*================================================*/
static int Ctc_Stack_Set_Multicast_Resp(LONG lType,  LONG lPonID, LONG lOnuID, 
                                     LONG lCvlan, LONG lMvlan, ULONG ulMIP, char *ucMac)
{
    SHORT llid;
    LONG  lOnuRet;
    char umMac[6];
    CTC_STACK_multicast_control_t sOnuBlk;
    if(glIgmpAuthDebug)
    {
        sys_console_printf("In Function:Ctc_Stack_Set_Multicast_Resp\r\n");
        sys_console_printf("The Para is :lType: %d,lPonID: %d,lOnuID: %d ,lCvlan: %d ,lMvlan: %d,ulMIP: %x",lType,lPonID,lOnuID,lCvlan,lMvlan,ulMIP);
    }
    
     IGMP_AUTH_DEBUG(("lPonID is %d,lOnuID is %d\r\n",lPonID,lOnuID));
     llid = GetLlidByOnuIdx(lPonID, lOnuID);
     IGMP_AUTH_DEBUG(( "GetLlidByOnuIdx= %d\r\n", llid));

     if( llid == -1 ) return VOS_ERROR;
     
     umMac[0]= 0x1;
     umMac[1]= 0x0;
     umMac[2]= 0x5e;
     umMac[3]= (ulMIP >> 16) & 0x7f;
     umMac[4]= (ulMIP >> 8)  & 0xff;
     umMac[5]= ulMIP  & 0xff;

	 if(lType == IGMP_AUTH_REQTYPE_JOIN) 
	      sOnuBlk.action = CTC_MULTICAST_CONTROL_ACTION_ADD;
	 else sOnuBlk.action = CTC_MULTICAST_CONTROL_ACTION_DELETE;
	
	 sOnuBlk.control_type= CTC_STACK_MULTICAST_CONTROL_TYPE_DA_VID;


     sOnuBlk.num_of_entries     = 1;
     sOnuBlk.entries[0].vid     = lMvlan;
     sOnuBlk.entries[0].user_id = lCvlan;
	 
	 memcpy(sOnuBlk.entries[0].da, umMac,6);

     #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	 lOnuRet = OnuMgt_SetMulticastControl ((SHORT)lPonID, (SHORT)lOnuID, &sOnuBlk);
	 #else
	 lOnuRet = CTC_STACK_set_multicast_control ((USHORT)lPonID, llid, sOnuBlk);
     #endif
        return lOnuRet;
}

void IgmpAuth_CTC_CDP_Callback_Pon(ULONG ulFlag,	ULONG ulChID, ULONG ulDstNode, ULONG ulDstChId, VOID  *pData, ULONG ulDataLen)
{
        IgmpAuthCDPMsgHead_t * revBuf = NULL;
        ULONG SlotnoOnPon = 0, PortnoOnPon = 0;
        unsigned char *pDatBuf=NULL; 
        long ret = -1;
	switch( ulFlag )
	{
		case CDP_NOTI_FLG_RXDATA: /* 收到数据*/
                    /*long  lNewTyp = 0;*/
                    
                    if( pData == NULL )
                    {
                        VOS_ASSERT( 0 );
                        return ;
                    }
                    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
                    {
                        CDP_FreeMsg(pData);
                        IGMP_AUTH_DEBUG(("IgmpAuth_CTC_CDP_Callback_Pon Error!\r\n"));
                        return;
                    }
                    if(ulChID != RPU_TID_CDP_IGMPAUTH_CTC || ulDstChId != RPU_TID_CDP_IGMPAUTH_CTC)
                    {
                        CDP_FreeMsg(pData);
                        return ;
                    }

                    revBuf=(IgmpAuthCDPMsgHead_t *)(( SYS_MSG_S * )pData + 1);
                    pDatBuf = (unsigned char*)((IgmpAuthCDPMsgHead_t*)revBuf+1);

                    if(SYS_MASTER_ACTIVE_SLOTNO != revBuf->ussrcslot)
                        return ;

                    SlotnoOnPon = GetCardIdxByPonChip(revBuf->usPonId);
                    PortnoOnPon = GetPonPortByPonChip(revBuf->usPonId);
                    IGMP_AUTH_DEBUG(("In the received CDP packet(IgmpAuth CTC), PonId is %d\r\n",revBuf->usPonId));
                    
                    revBuf->usPonId = GetPonPortIdxBySlot(SlotnoOnPon,PortnoOnPon);
                    IGMP_AUTH_DEBUG(("SlotnoOnPon is %d,PortnoOnPon is %d,PonId is %d\r\n",SlotnoOnPon,PortnoOnPon,revBuf->usPonId));

                    ret =Ctc_Stack_Set_Multicast_Resp(*((LONG*)pDatBuf),revBuf->usPonId,*((LONG*)(pDatBuf+8)),*((LONG*)(pDatBuf+12)),
                        *((LONG*)(pDatBuf+16)),*((LONG*)(pDatBuf+20)),pDatBuf+24);
                    if(glIgmpAuthDebug)
                        sys_console_printf("The ret is %d\r\n",ret);
                    CDP_FreeMsg(pData);
			break;
		case CDP_NOTI_FLG_SEND_FINISH:/*异步发送时*/
			CDP_FreeMsg(pData);		
			break;
		default:
			ASSERT(0);
			CDP_FreeMsg(pData);
			break;
	}
    
    return ;
    
}

static int Ctc_Stack_Set_Multicast_Resp_Cdp(LONG lType,  LONG lPonID, LONG lOnuID, 
                                     LONG lCvlan, LONG lMvlan, ULONG ulMIP, char *ucMac)
{
     unsigned char bPktBuf[32];
     IgmpAuthCDPMsgHead_t *bDatPdpBuf = NULL;
    unsigned int ulLen,Slotno;
    SYS_MSG_S *pMsg = NULL;

    Slotno = GetCardIdxByPonChip(lPonID);

    if ( !SYS_MODULE_IS_READY(Slotno) ) 
    {
    	 IGMP_AUTH_DEBUG(("SYS MODULE %d IS NOT READY\r\n",Slotno));
        return VOS_ERROR;
    }
	
    *((LONG *)(bPktBuf + 0))  = lType;
    *((LONG *)(bPktBuf + 4)) = lPonID;
    *((LONG *)(bPktBuf + 8))   = lOnuID;
    *((LONG *)(bPktBuf + 12))  = lCvlan;
    *((LONG *)(bPktBuf + 16)) = lMvlan;
    *(( ULONG *)(bPktBuf + 20))   = ulMIP;

    VOS_MemCpy(bPktBuf + 24, ucMac, 6);
    ulLen=sizeof(IgmpAuthCDPMsgHead_t)+32+sizeof(SYS_MSG_S);
    pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_IGMPAUTH);
    if( NULL == pMsg )
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }
    VOS_MemZero((CHAR *)pMsg, ulLen );

    SYS_MSG_SRC_ID( pMsg )       = MODULE_IGMPAUTH;
    SYS_MSG_DST_ID( pMsg )       = MODULE_IGMPAUTH;
    SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
    SYS_MSG_FRAME_LEN( pMsg )    = ulLen;
    SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
    SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
    SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
    SYS_MSG_DST_SLOT( pMsg ) = Slotno;

    bDatPdpBuf= (IgmpAuthCDPMsgHead_t * ) ( pMsg + 1 );
    bDatPdpBuf->usReqTyp = lType;
    bDatPdpBuf->usPonId = lPonID;
    bDatPdpBuf->usOnuId = lOnuID;
    bDatPdpBuf->usDatLen = 32;
    bDatPdpBuf->pDatBuf = (unsigned char *)(bDatPdpBuf+1);
    bDatPdpBuf->ussrcslot = SYS_LOCAL_MODULE_SLOTNO ;

    VOS_MemCpy( bDatPdpBuf->pDatBuf,  bPktBuf, 32);
    
    if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_IGMPAUTH_CTC,Slotno, RPU_TID_CDP_IGMPAUTH_CTC, /*CDP_MSG_TM_ASYNC*/ 0,\
                 (VOID *)pMsg, ulLen, MODULE_IGMPAUTH ) )
    {
        VOS_ASSERT(0); 
        IGMP_AUTH_DEBUG(("\r\nSend the IgmpAuth CTC Oam(with authentication result) to the PonCard %d Failed!\r\n",Slotno));
        CDP_FreeMsg(pMsg);
        return VOS_ERROR;
    }
    IGMP_AUTH_DEBUG(("\r\nSend the IgmpAuth CTC Oam(with authentication result) to the PonCard %d Success!\r\n",Slotno));
    return VOS_OK;
}

static void funIgmpAuthCtcOnuReqResp(LONG lType,  LONG lPonID, LONG lOnuID, 
                                     LONG lCvlan, LONG lMvlan, ULONG ulMIP, char *ucMac)
{
    LONG  lOnuRet = -1;
    ULONG dstSlot=0;
		
    IGMP_AUTH_DEBUG(("Recv  set onu pon = %d, onu = %d , cvlan = %d , type = %d, mip = 0x%x\r\n",
                         lPonID, lOnuID, lCvlan, lType,ulMIP));

    dstSlot = GetGlobalCardIdxByPonChip(lPonID);
#if 0
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	   lOnuRet = Ctc_Stack_Set_Multicast_Resp(lType,  lPonID, lOnuID, 
                                     lCvlan, lMvlan, ulMIP, ucMac);
    else
          lOnuRet = Ctc_Stack_Set_Multicast_Resp_Cdp(lType,  lPonID, lOnuID, 
                                     lCvlan, lMvlan, ulMIP, ucMac);
#else
	if( SlotCardIsPonBoard(dstSlot) != ROK )
		return;

	if( dstSlot == SYS_LOCAL_MODULE_SLOTNO || !SYS_MODULE_SLOT_ISHAVECPU( dstSlot ))
		 lOnuRet = Ctc_Stack_Set_Multicast_Resp(lType,  lPonID, lOnuID, 
                                     lCvlan, lMvlan, ulMIP, ucMac);
	else
		lOnuRet = Ctc_Stack_Set_Multicast_Resp_Cdp(lType,  lPonID, lOnuID, 
                                     lCvlan, lMvlan, ulMIP, ucMac);
#endif
    
     setIgmpSnoopAuthItem(lPonID, lOnuID, lType, lCvlan, ulMIP, lMvlan, ucMac, lOnuRet);
    
    /* if(lOnuRet == CTC_STACK_EXIT_OK)*/

}

/* 发送消息 */
/*================================================*/
/*name: funIgmpAuthGWOnuReqResp                   */
/*para: pReqInfo 消息缓冲区                       */
/*retu: none                                      */
/*desc: 发送GW类型ONU的应答报文                   */
/*================================================*/

int Comm_IGMPAuth_Requst_Cdp(const unsigned short PonId, 
								const unsigned short OnuId,
								unsigned char *pIgmpAuth,
								const unsigned short igmp_data_size,
								unsigned char *psessionIdField,
								unsigned short  lType)
{
    IgmpAuthCDPMsgHead_t *bDatPdpBuf = NULL;
    unsigned int ulLen,Slotno,i;
    SYS_MSG_S *pMsg = NULL;
    unsigned char *p;
    
    IGMP_AUTH_DEBUG(("IN Function : Comm_IGMPAuth_Requst_Cdp\r\n"));
    IGMP_AUTH_DEBUG(("PonId is %d,OnuId is %d,data_size is %d,lType is %d\r\n",PonId,OnuId,igmp_data_size,lType));

    Slotno = GetCardIdxByPonChip(PonId);

    if ( !SYS_MODULE_IS_READY(Slotno) ) 
    {
    	 IGMP_AUTH_DEBUG(("SYS MODULE %d IS NOT READY\r\n",Slotno));
        return VOS_ERROR;
    }
    
    ulLen=sizeof(IgmpAuthCDPMsgHead_t)+igmp_data_size+8+sizeof(SYS_MSG_S);
    pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_IGMPAUTH);
    if( NULL == pMsg )
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }
    VOS_MemZero((CHAR *)pMsg, ulLen );
    SYS_MSG_SRC_ID( pMsg )       = MODULE_IGMPAUTH;
    SYS_MSG_DST_ID( pMsg )       = MODULE_IGMPAUTH;
    SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
    SYS_MSG_FRAME_LEN( pMsg )    = ulLen;
    SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
    SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
    SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
    SYS_MSG_DST_SLOT( pMsg ) = Slotno;

    bDatPdpBuf= (IgmpAuthCDPMsgHead_t * ) ( pMsg + 1 );
    bDatPdpBuf->usReqTyp = lType;
    bDatPdpBuf->usPonId = PonId;
    bDatPdpBuf->usOnuId = OnuId;
    bDatPdpBuf->usDatLen = igmp_data_size+8;
    bDatPdpBuf->pDatBuf = (unsigned char *)(bDatPdpBuf+1);
    bDatPdpBuf->ussrcslot = SYS_LOCAL_MODULE_SLOTNO ;

    VOS_MemCpy( bDatPdpBuf->pDatBuf,  pIgmpAuth, igmp_data_size);
    VOS_MemCpy( bDatPdpBuf->pDatBuf+igmp_data_size,  psessionIdField, 8);

    p = (unsigned char *)pMsg;
    if(glIgmpAuthDebug)
    {
        sys_console_printf("Datelegth is %d\r\n",ulLen);
        for(i=0;i<ulLen ;i++)
        {
            sys_console_printf("%02x  " ,*p);
            if((i+1)%16==0)
                sys_console_printf("\r\n");
	     p++;
        }
    }

    if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_IGMPAUTH_GW, Slotno, RPU_TID_CDP_IGMPAUTH_GW, /*CDP_MSG_TM_ASYNC*/ 0,\
                 (VOID *)pMsg, ulLen, MODULE_IGMPAUTH ) )
    {
        VOS_ASSERT(0); 
        IGMP_AUTH_DEBUG(("\r\nSend the IgmpAuth GW Oam(with authentication result) to the PonCard %d Failed!\r\n",Slotno));
        CDP_FreeMsg(pMsg);
        return VOS_ERROR;
    }
    IGMP_AUTH_DEBUG(("\r\nSend the IgmpAuth GW Oam(with authentication result) to the PonCard %d Success!\r\n",Slotno));
    return VOS_OK;

}

/*
int Comm_IGMPAuth_Requst_CdpTest()
{
    IgmpAuthCDPMsgHead_t *bDatPdpBuf = NULL;
    unsigned int ulLen,Slotno,i;
    SYS_MSG_S *pMsg = NULL;
    unsigned char *p;
    
    IGMP_AUTH_DEBUG(("IN Function : Comm_IGMPAuth_Requst_CdpTest\r\n"));
  
    ulLen=sizeof(IgmpAuthCDPMsgHead_t)+sizeof(SYS_MSG_S);
    pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_IGMPAUTH);
    if( NULL == pMsg )
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }
    VOS_MemZero((CHAR *)pMsg, ulLen );
    Slotno = GetCardIdxByPonChip(22);
    SYS_MSG_SRC_ID( pMsg )       = MODULE_IGMPAUTH;
    SYS_MSG_DST_ID( pMsg )       = MODULE_IGMPAUTH;
    SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
    SYS_MSG_FRAME_LEN( pMsg )    = ulLen;
    SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
    SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
    SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
    SYS_MSG_DST_SLOT( pMsg ) = Slotno;

    bDatPdpBuf= (IgmpAuthCDPMsgHead_t * ) ( pMsg + 1 );
    bDatPdpBuf->usReqTyp = 11;
    bDatPdpBuf->usPonId = 22;
    bDatPdpBuf->usOnuId = 1;
    bDatPdpBuf->usDatLen = 0;
    bDatPdpBuf->pDatBuf = (unsigned char *)(bDatPdpBuf+1);
    bDatPdpBuf->ussrcslot = SYS_LOCAL_MODULE_SLOTNO ;

    p = (unsigned char *)pMsg;
    if(glIgmpAuthDebug)
    {
        sys_console_printf("Datelegth is %d\r\n",ulLen);
        for(i=0;i<ulLen ;i++)
        {
            sys_console_printf("%02x  " ,*p);
            if((i+1)%16==0)
                sys_console_printf("\r\n");
	     p++;
        }
    }

    if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_IGMPAUTH_GW, Slotno, RPU_TID_CDP_IGMPAUTH_GW, 0,\
                 (VOID *)pMsg, ulLen, MODULE_IGMPAUTH ) )
    {
        VOS_ASSERT(0); 
        IGMP_AUTH_DEBUG(("\r\nSend the IgmpAuth GW Oam(with authentication result) to the PonCard %d Failed!\r\n",Slotno));
        CDP_FreeMsg(pMsg);
        return VOS_ERROR;
    }
    IGMP_AUTH_DEBUG(("\r\nSend the IgmpAuth GW Oam(with authentication result) to the PonCard %d Success!\r\n",Slotno));
    return VOS_OK;

}
*/

static void funIgmpAuthGWOnuReqResp(LONG  lType,    LONG lPonID, LONG lOnuID, 
                                    LONG  lSrcPort, LONG lMvlan, ULONG ulMIP, 
                                    ULONG ulSrcIP,  char *ucMac, LONG lResult,ULONG ulSessIda,ULONG ulSessIdb)
{
    unsigned char bPktBuf[32];
    unsigned char bSessid[8];

    long          lNewTyp = lType;
     ULONG dstSlot=0;

    dstSlot = GetGlobalCardIdxByPonChip(lPonID);

    if(lType == IGMP_AUTH_REQTYPE_FORCELEA)
    {
        lNewTyp = IGMP_AUTH_REQTYPE_LEAVE;
    }
    else
    {
        if(lType == IGMP_AUTH_REQTYPE_JOIN) 
             lType = IGMP_AUTH_OAM_REG_ACK;
        else lType = IGMP_AUTH_OAM_UNREG_ACK;
    }

/*modified6900,修改6900，如果是主控板就发送板间通信给pon，再由pon发送给onu*/

    VOS_MemSet(bSessid, 0, 8);
    *((ULONG*)bSessid)        = ulSessIda;
    *((ULONG*)(bSessid+4))  = ulSessIdb;
    *((unsigned short *)(bPktBuf + IGMP_AUTH_FILED_OFF_TYPE))  = (unsigned short)lType;
    *((unsigned short *)(bPktBuf + IGMP_AUTH_FILED_OFF_ONUID)) = (unsigned short)lOnuID;
    *((unsigned short *)(bPktBuf + IGMP_AUTH_FILED_OFF_VID))   = (unsigned short)lMvlan;
    *((unsigned long  *)(bPktBuf + IGMP_AUTH_FILED_OFF_MCIP))  = ulMIP;
    *((unsigned long  *)(bPktBuf + IGMP_AUTH_FILED_OFF_SPORT)) = (unsigned long )lSrcPort;
    *((unsigned long  *)(bPktBuf + IGMP_AUTH_FILED_OFF_SIP))   = ulSrcIP;
    *((unsigned short *)(bPktBuf + IGMP_AUTH_FILED_OFF_RESU))  = (unsigned short)lResult;

    VOS_MemCpy(bPktBuf + IGMP_AUTH_FILED_OFF_SMAC, ucMac, 6);
#if 0
    if(SYS_LOCAL_MODULE_TYPE == MODULE_E_GFA6900_SW ||SYS_LOCAL_MODULE_TYPE == MODULE_E_GFA6900_12EPON_M )
    {
        Comm_IGMPAuth_Requst_Cdp((unsigned short)lPonID, (unsigned short)lOnuID, bPktBuf, IGMP_AUTH_OAM_PKTLEN_ACK,bSessid,(unsigned short)lType);
    }
    else
    {
        if(lType == IGMP_AUTH_REQTYPE_FORCELEA)
        {
                Comm_IGMPAuth_Requst((unsigned short)lPonID, (unsigned short)lOnuID, bPktBuf, IGMP_AUTH_OAM_PKTLEN_REQ,bSessid);
        }
        else
        {          
            Comm_IGMPAuth_Reponse((unsigned short)lPonID, (unsigned short)lOnuID, bPktBuf, IGMP_AUTH_OAM_PKTLEN_ACK,bSessid);
        }
    }
#else
	if( SlotCardIsPonBoard(dstSlot) != ROK )
		return;

	if( dstSlot == SYS_LOCAL_MODULE_SLOTNO || !SYS_MODULE_SLOT_ISHAVECPU( dstSlot ))
	{
		 if(lType == IGMP_AUTH_REQTYPE_FORCELEA)
	        {
	                Comm_IGMPAuth_Requst((unsigned short)lPonID, (unsigned short)lOnuID, bPktBuf, IGMP_AUTH_OAM_PKTLEN_REQ,bSessid);
	        }
	        else
	        {          
	            Comm_IGMPAuth_Reponse((unsigned short)lPonID, (unsigned short)lOnuID, bPktBuf, IGMP_AUTH_OAM_PKTLEN_ACK,bSessid);
	        }
	}
	else
		Comm_IGMPAuth_Requst_Cdp((unsigned short)lPonID, (unsigned short)lOnuID, bPktBuf, IGMP_AUTH_OAM_PKTLEN_ACK,bSessid,(unsigned short)lType);
#endif

	setIgmpSnoopAuthItem(lPonID, lOnuID-1, lNewTyp, lSrcPort, ulMIP, lMvlan, ucMac, lResult);

}

void  IgmpAuth_GW_CDP_Callback_Pon(ULONG ulFlag,	ULONG ulChID, ULONG ulDstNode, ULONG ulDstChId, VOID  *pData, ULONG ulDataLen)
{
       IgmpAuthCDPMsgHead_t * revBuf = NULL;
       ULONG SlotnoOnPon = 0, PortnoOnPon = 0,i;
       unsigned char *pDatBuf=NULL,*p;
       
	switch( ulFlag )
	{
		case CDP_NOTI_FLG_RXDATA: /* 收到数据*/
                     /*long  lNewTyp = 0;*/     
                    if( pData == NULL )
                    {
                        VOS_ASSERT( 0 );
                        return ;
                    }
                    
                    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
                    {
                        CDP_FreeMsg(pData);
                        IGMP_AUTH_DEBUG(("IgmpAuth_GW_CDP_Callback_Pon Error!\r\n"));
                        return;
                    }
                    
                    if(ulChID != RPU_TID_CDP_IGMPAUTH_GW || ulDstChId != RPU_TID_CDP_IGMPAUTH_GW)
                    {
                        CDP_FreeMsg(pData);
                        return ;
                    }
                    p = (unsigned char *)pData;
                    if(glIgmpAuthDebug)
                    {
                        sys_console_printf("In Function : IgmpAuth_GW_CDP_Callback_Pon\r\n");
                        sys_console_printf("Datelegth is %d\r\n",ulDataLen);
                        for(i=0;i<ulDataLen ;i++)
                        {
                            sys_console_printf("%02x  " ,*p);
                            if((i+1)%16==0)
                                sys_console_printf("\r\n");
                	     p++;
                        }
                    }
                    revBuf=(IgmpAuthCDPMsgHead_t *)(( SYS_MSG_S * )pData + 1);
                    pDatBuf = (unsigned char*)((IgmpAuthCDPMsgHead_t*)revBuf+1);

                    if(SYS_MASTER_ACTIVE_SLOTNO != revBuf->ussrcslot)
                        return ;

                    SlotnoOnPon = GetCardIdxByPonChip(revBuf->usPonId);
                    PortnoOnPon = GetPonPortByPonChip(revBuf->usPonId);
                    IGMP_AUTH_DEBUG(("In the received CDP packet(IgmpAuth GW), PonId is %d\r\n",revBuf->usPonId));
                    
                    revBuf->usPonId = GetPonPortIdxBySlot(SlotnoOnPon,PortnoOnPon);
                    IGMP_AUTH_DEBUG(("SlotnoOnPon is %d,PortnoOnPon is %d,PonIdOnPon is %d\r\n",SlotnoOnPon,PortnoOnPon,revBuf->usPonId));

                    if(revBuf->usReqTyp == IGMP_AUTH_REQTYPE_FORCELEA)
                    {
                        /*lNewTyp = IGMP_AUTH_REQTYPE_LEAVE;*/
                        Comm_IGMPAuth_Requst(revBuf->usPonId, revBuf->usOnuId,pDatBuf, IGMP_AUTH_OAM_PKTLEN_REQ,pDatBuf+IGMP_AUTH_OAM_PKTLEN_REQ);
                    }
                    else
                    {
                        /*if(revBuf->usReqTyp == IGMP_AUTH_OAM_REG_ACK)
                            lNewTyp = IGMP_AUTH_REQTYPE_JOIN;
                        else
                            lNewTyp = IGMP_AUTH_REQTYPE_LEAVE;*/        
                        Comm_IGMPAuth_Reponse(revBuf->usPonId, revBuf->usOnuId,pDatBuf, IGMP_AUTH_OAM_PKTLEN_ACK,pDatBuf+IGMP_AUTH_OAM_PKTLEN_ACK);
                    }
                    
                    CDP_FreeMsg(pData);
			break;
		case CDP_NOTI_FLG_SEND_FINISH:/*异步发送时*/
			CDP_FreeMsg(pData);		
			break;
		default:
			ASSERT(0);
			CDP_FreeMsg(pData);
			break;
	}

    return ;
    
}

/*=================================================================================================================== */
/*=================================================================================================================== */

/*================================================*/
/*name: funIgmpAuthProcOnuReqMsg                  */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 处理来自ONU的(IGMP报文)认证请求           */
/*      使用于CTC/GW后续类型ONU                   */
/*      DAYA类型来自OAM                           */
/*================================================*/
static void funIgmpAuthProcOnuReqMsg(SYS_MSG_S *pMsgBuf)
{
	 LONG                     lResult = 0;
	 LONG                     lPonID  = 0;
	 LONG                     lOnuID  = 0;
	 LONG                     lCvlan  = 0;
	 LONG                     lMvlan  = 0;
	 LONG                     lType   = 0;
     LONG                     lOnuType= 0;
	 ULONG                    ulMIP   = 0;
	 uchar                    ucMac[6]= {0};
	 /*SHORT                    llid;*/
     struct Igmp_AuthInfo_s  *pOnuReq = NULL;
     
     pOnuReq = (struct Igmp_AuthInfo_s*)(pMsgBuf + 1);

     lPonID  = pOnuReq->lUserA;
     lOnuID  = pOnuReq->lUserB;
     lType   = pOnuReq->lType;
	 lCvlan  = pOnuReq->lCvlan;
	 ulMIP   = pOnuReq->ulMip;  
     
     IGMP_AUTH_DEBUG(("Recv ONU req pon = %d, onu= %d  cvlan = %d , type = %d, mip = 0x%x\r\n",lPonID, lOnuID, lCvlan, lType, ulMIP));

	 memcpy(ucMac, pOnuReq->ucCmac,6);
	 
     lResult = checkIgmpSnoopAuthItem(lPonID,  &lOnuID, lType, lCvlan, ulMIP, &lMvlan, &lOnuType, ucMac);
     lOnuType = IGMP_AUTH_ONUTYPE_CTC;/*因为现在6900主控上不能看到CTC ONU, 暂时直接赋值为CTC类型, 2010-12-15, duzhk*/
     IGMP_AUTH_DEBUG(("lResult is %d,lMvlan is %d,lOnuType is %d\r\n,",lResult,lMvlan,lOnuType));
	 pOnuReq->lMvlan   = lMvlan;
	 pOnuReq->lDevType =  lOnuType;
	 pOnuReq->lResult  = (lResult == IGMP_AUTH_RESULT_NONOTI) ? IGMP_AUTH_RESULT_AGREE : lResult;
 
     if(lOnuType == IGMP_AUTH_ONUTYPE_UNK) pOnuReq->lResult = IGMP_AUTH_RESULT_REFUSE;

	 Igmp_Snoop_AuthRespond(pMsgBuf) ;
/*
	{
        IGMP_AUTH_DEBUG((LOG_TYPE_AUTH_IGMP, LOG_DEBUG_OUT,
                      "Recv ONU req Igmp_Snoop_AuthRespond return error\r\n"));

	    return;
	}
*/
     IGMP_AUTH_DEBUG(("Recv ONU req check rusult pon = %d, onu=%d,cvlan = %d , type = %d, mip = 0x%x result= %d\r\n",lPonID,lOnuID, lCvlan, lType, ulMIP, lResult));
	
	 if(lResult == IGMP_AUTH_RESULT_AGREE)
       {   
	      if(lOnuType == IGMP_AUTH_ONUTYPE_CTC)
	      {
              funIgmpAuthCtcOnuReqResp(lType, lPonID, lOnuID-1, lCvlan, lMvlan, ulMIP, ucMac);
	      }
          else if (lOnuType == IGMP_AUTH_ONUTYPE_GW)
          {
              funIgmpAuthGWOnuReqResp(lType, lPonID, lOnuID, lCvlan, lMvlan, ulMIP, 0, ucMac, 1, 0,0); 
          }
	 }
     else if(lResult == IGMP_AUTH_RESULT_NONOTI)
     {
	      if(lOnuType == IGMP_AUTH_ONUTYPE_CTC)	/* modified by duzhk 20120110, 问题单14389 */
	      {
              funIgmpAuthCtcOnuReqResp(lType, lPonID, lOnuID-1, lCvlan, lMvlan, ulMIP, ucMac);
	      }
          else if(lOnuType == IGMP_AUTH_ONUTYPE_GW)
          {
              funIgmpAuthGWOnuReqResp(lType, lPonID, lOnuID, lCvlan, lMvlan, ulMIP, 0, ucMac, 1, 0, 0); 
          }
     }
     else 
     {
          if(lOnuType == IGMP_AUTH_ONUTYPE_GW)
          {
              funIgmpAuthGWOnuReqResp(lType, lPonID, lOnuID, lCvlan, lMvlan, ulMIP, 0, ucMac, 2, 0, 0); 
          }
     }
}	 
/*=================================================================================================================== */
/*=================================================================================================================== */

/*================================================*/
/*name: funIgmpAuthProcManReqMsg                  */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 处理来自manager的管理请求                 */
/*================================================*/
static void funIgmpAuthProcManReqMsg(SYS_MSG_S *pMsgBuf)
{
	 /*LONG                     lResult = 0;*/
	 LONG                     lPonID  = 0;
	 LONG                     lOnuID  = 0;
	 LONG                     lCvlan  = 0;
	 LONG                     lMvlan  = 0;
	 LONG                     lType   = 0;
	 LONG                     lOnuType= 0;
	 ULONG                    ulMIP   = 0;
	 uchar                    ucMac[6]= {0};
	 /*short int 			      llid    = 0;*/
     struct Igmp_AuthInfo_s  *pOnuReq = NULL;
	 /*CTC_STACK_multicast_control_t sOnuBlk; */
     
     pOnuReq = (struct Igmp_AuthInfo_s*)(pMsgBuf + 1);

     lPonID  = pOnuReq->lUserA;
     lOnuID  = pOnuReq->lUserB;
     lType   = pOnuReq->lType;
	 lCvlan  = pOnuReq->lCvlan;
     lOnuType= pOnuReq->lDevType;
	 ulMIP   = pOnuReq->ulMip;
     lMvlan  = pOnuReq->lMvlan;

     memcpy(ucMac, pOnuReq->ucCmac, 6);
     
	 if(lOnuType == IGMP_AUTH_ONUTYPE_CTC)
	 {
         funIgmpAuthCtcOnuReqResp(lType, lPonID, lOnuID-1, lCvlan, lMvlan, ulMIP, ucMac);
	 }
     else if ((lOnuType == IGMP_AUTH_ONUTYPE_GW) || (lOnuType == IGMP_AUTH_ONUTYPE_OAM))
     {
         if((lType == IGMP_AUTH_REQTYPE_LEAVE) || (lType == IGMP_AUTH_REQTYPE_FORCELEA))
         {
            lType = IGMP_AUTH_REQTYPE_FORCELEA;
            funIgmpAuthGWOnuReqResp(lType, lPonID, lOnuID, lCvlan, lMvlan, ulMIP, 0, ucMac, 1, 0, 0); 
         }
     }

 	 if((pMsgBuf->ptrMsgBody != NULL ) && (pMsgBuf->ucMsgBodyStyle == MSG_BODY_MULTI)) 
     {
	     VOS_Free(pMsgBuf->ptrMsgBody);
     }

	 VOS_Free(pMsgBuf);
}	 

/*=================================================================================================================== */
/*=================================================================================================================== */

/* 消息处理函数  */
/*================================================*/
/*name: funIgmpAuthProcOamRegMsg                  */
/*para: pReqInfo 消息缓冲区                       */
/*retu: none                                      */
/*desc: GW OAM 请求处理函数                       */
/*================================================*/
static void funIgmpAuthProcOamRegMsg(SYS_MSG_S *pQueMsg)
{
	 LONG                     lResult = 0;
	 LONG                     lPonID  = 0;
	 LONG                     lOnuID  = 0;
	 LONG                     lCvlan  = 0;
	 LONG                     lMvlan  = 0;
	 LONG                     lType   = 0;
	 LONG                     lOnuType= 0;
	 ULONG                    ulMIP   = 0;
	 uchar                    ucMac[6]= {0};
	 ULONG                    lSessIda = 0;
	 ULONG                    lSessIdb = 0;
     struct Igmp_AuthInfo_s  *pOnuReq = NULL;
     
     pOnuReq = (struct Igmp_AuthInfo_s*)(pQueMsg + 1);

     lPonID  = pOnuReq->lUserA;
     lOnuID  = pOnuReq->lUserB;
     lType   = pOnuReq->lType;
	 lCvlan  = pOnuReq->lCvlan;
     lOnuType= pOnuReq->lDevType;
	 ulMIP   = pOnuReq->ulMip;
     lMvlan  = pOnuReq->lMvlan;

     lSessIda = pQueMsg->ulSrcSlotID;
     lSessIdb = pQueMsg->ulDstSlotID;

     memcpy(ucMac, pOnuReq->ucCmac,6);

     lResult = checkIgmpSnoopAuthItem(lPonID,  &lOnuID, lType, lCvlan, ulMIP, &lMvlan, &lOnuType, ucMac);
     
	 pOnuReq->lMvlan   = lMvlan;
	 pOnuReq->lDevType = lOnuType;
	 pOnuReq->lResult  = (lResult == IGMP_AUTH_RESULT_NONOTI) ? IGMP_AUTH_RESULT_AGREE : lResult;
        IGMP_AUTH_DEBUG(("onu oam register request result = %d\r\n", lResult));

     if(lOnuType == IGMP_AUTH_ONUTYPE_UNK) pOnuReq->lResult = IGMP_AUTH_RESULT_REFUSE;
      IGMP_AUTH_DEBUG(("onu type = %d\r\n", lOnuType));
	 if(pOnuReq->lResult == IGMP_AUTH_RESULT_AGREE)
	 {
         funIgmpAuthGWOnuReqResp(lType, lPonID, lOnuID, lCvlan, lMvlan, ulMIP, 0, ucMac, 1, lSessIda, lSessIdb); 
	 }
     else 
     {
         funIgmpAuthGWOnuReqResp(lType, lPonID, lOnuID, lCvlan, lMvlan, ulMIP, 0, ucMac, 2, lSessIda, lSessIdb); 
     }

     VOS_Free(pQueMsg);
}

/*=================================================================================================================== */
/*=================================================================================================================== */

/*================================================*/
/*name: funIgmpAuthProcMessages                   */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 任务的输入消息处理主函数                  */
/*================================================*/
static void funIgmpAuthProcMessages(unsigned char *pMsgBuf)
{
    SYS_MSG_S *pSysMsg = (SYS_MSG_S *)pMsgBuf;
    
    switch(pSysMsg->usMsgCode)   
    {
        case IGMP_AUTH_MSGCODE_OAMREQ:

             funIgmpAuthProcOamRegMsg(pSysMsg);
             break;
            
        case IGMP_AUTH_MSGCODE_MANREQ:                  /*网管设置请求*/
             funIgmpAuthProcManReqMsg(pSysMsg); 
             break;

        case AWMC_IPMC_PRIVATE_SNOOP_AUTH_REQUIRE:      /*ONU 认证请求 来自IGMPSNOOPING*/ 
			
             funIgmpAuthProcOnuReqMsg(pSysMsg);
             break;
			 
        case IGMP_AUTH_MSGCODE_TIMER:

			 funIgmpAuthProcTimerMsg(pSysMsg);
             break;
			 
        case IGMP_AUTH_MSGCODE_OAMACK:                 /* 不再处理应答报文 */
        default: 
             ASSERT(0);
             break;
    }
}
/*=================================================================================================================== */
/*=================================================================================================================== */
/* 私有OAM接收回调函数                           */
/*================================================*/
/*name: funIgmpAuthRevPktCheck                    */
/*para: lDatLen  数据长度                         */
/*para: pDatBuf  数据指针                         */
/*para: pcSessId 会话ID                           */
/*retu: VOS_OK/VOS_ERROR                          */
/*desc: 组播认证报文的回调函数                    */
/*================================================*/
static long funIgmpAuthRevPktCheck(long  usReqTyp, unsigned short usDatLen, 
                                   unsigned char *pDatBuf,  unsigned char *pSessId)
{
    if((pDatBuf == NULL) || (pSessId == NULL))
    {
        IGMP_AUTH_DEBUG(("pDatBuf=NULL  or pSessId=NULL in callback fun.\r\n"));
        if(pDatBuf != NULL)
		VOS_Free(pDatBuf);
	  if(pSessId != NULL)	
	  	VOS_Free(pSessId);
        return(VOS_ERROR);
    }

    if((usReqTyp == IGMP_AUTH_MSGCODE_OAMREQ) && (usDatLen != IGMP_AUTH_OAM_PKTLEN_REQ))
    {
        IGMP_AUTH_DEBUG(("Receive req pkt dataLen error.\r\n"));
        VOS_Free(pDatBuf);
        VOS_Free(pSessId);
        return(VOS_ERROR);
    }

    if((usReqTyp == IGMP_AUTH_MSGCODE_OAMACK) && (usDatLen != IGMP_AUTH_OAM_PKTLEN_ACK))
    {
        IGMP_AUTH_DEBUG(("Receive ack pkt dataLen error.\r\n"));
        VOS_Free(pDatBuf);
        VOS_Free(pSessId);
        return(VOS_ERROR);
    }
	
    
    return(VOS_OK);
}


/*================================================*/
/*name: funIgmpAuthOamAllCallBack                 */
/*para: usReqTyp                                  */ 
/*para: usPonId                                   */ 
/*para: usOnuId                                   */ 
/*para: lDatLen  数据长度                         */
/*para: pDatBuf  数据指针                         */
/*para: pcSessId 会话ID                           */
/*retu: none                                      */
/*desc: 来自OAM组播认证报文的回调函数             */
/*================================================*/
/*下面函数接收PON 板转上来的OAM 报文*/
void  IgmpAuth_GW_CDP_Callback(ULONG ulFlag,		/*	消息标志，表示消息发送完毕或接收到消息 */
							 ULONG ulChID,		/* CDP 通道号，即哪个通道报告的。如果一个
													函数被注册到多个通道，则此参数可指示是
													哪个通道报告的 */
							 ULONG ulDstNode,	/* 数据的目的节点号，在组播发送时表示组播
													组ID广播时无意义 */
							 ULONG ulDstChId,	/* 数据的目的通道号 */
							 VOID  *pData,		/* 发送的数据 */
							 ULONG ulDataLen	/* 数据的长度 */
							)
{
       IgmpAuthCDPMsgHead_t * revBuf = NULL;
       ULONG SlotnoOnOlt = 0, PortnoOnOlt = 0;
       unsigned long     ulMessage[4] = {0},i;
       SYS_MSG_S        *pQueMsg      = NULL;
       Igmp_AuthInfo_t  *pAuthReq     = NULL ;
       unsigned char *pDatBuf=NULL,*p;   
	switch( ulFlag )
	{
		case CDP_NOTI_FLG_RXDATA: /* 收到数据*/

                    /*p = (unsigned char *)pData;*/
                    if( pData == NULL )
                    {
                        VOS_ASSERT( 0 );
                        return ;
                    }

                    if(ulChID != RPU_TID_CDP_IGMPAUTH_GW || ulDstChId != RPU_TID_CDP_IGMPAUTH_GW)
                    {
                        CDP_FreeMsg(pData);
                        return ;
                    }
                    

                    revBuf=(IgmpAuthCDPMsgHead_t *)(( SYS_MSG_S * )pData + 1);
                    pDatBuf = (unsigned char*)((IgmpAuthCDPMsgHead_t*)revBuf+1);
                    
                    /*if(glIgmpAuthDebug)
                    {
                        sys_console_printf("In Function : IgmpAuth_GW_CDP_Callback\r\n");
                        sys_console_printf("Datelegth is %d\r\n",ulDataLen);
                        for(i=0;i<ulDataLen;i++)
                        {
                            sys_console_printf("%02x ",*p);
                            if((i+1)%16==0)
                                sys_console_printf("\r\n");
                	     p++;
                        }
                    }*/
                    
                    SlotnoOnOlt = revBuf->ussrcslot;
                    PortnoOnOlt = revBuf->usPonId;
                     /*IGMP_AUTH_DEBUG(("In Function : IgmpAuth_GW_CDP_Callback\r\n"));*/
                    IGMP_AUTH_DEBUG(("Function : IgmpAuth_GW_CDP_Callback . In the received CDP packet(IgmpAuth GW),Slot is %d,Port is %d,OnuId is %d\r\n",revBuf->ussrcslot,revBuf->usPonId,revBuf->usOnuId));
                    
                    revBuf->usPonId = GetPonPortIdxBySlot(SlotnoOnOlt,PortnoOnOlt);
                    IGMP_AUTH_DEBUG(("SlotnoOnOlt is %d,PortnoOnOlt is %d,PonIdOnPon is %d\r\n",SlotnoOnOlt,PortnoOnOlt,revBuf->usPonId));
                    pQueMsg = (SYS_MSG_S*)VOS_Malloc(sizeof(SYS_MSG_S) + sizeof(Igmp_AuthInfo_t) , MODULE_IGMPAUTH);
                    
                    if(pQueMsg == NULL)
                    {
                        IGMP_AUTH_DEBUG(("Malloc memory faile in API fun.\r\n"));
                        CDP_FreeMsg(pData);
                        return ;
                    }

                    pAuthReq  = (Igmp_AuthInfo_t*)(pQueMsg + 1);
                    
                    VOS_MemZero(pQueMsg, sizeof(SYS_MSG_S) + sizeof(Igmp_AuthInfo_t));

                    pQueMsg->ucMsgType      = MSG_NOTIFY;
                    pQueMsg->ulSrcSlotID    = *((ULONG*)(revBuf->pSessId));
                    pQueMsg->ulDstSlotID    = *((ULONG*)(revBuf->pSessId+4) ); 
                    pQueMsg->usMsgCode      = revBuf->usReqTyp;
                    pQueMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;
                    pQueMsg->usFrameLen     = 0;
                    pQueMsg->ptrMsgBody     = 0;

                    pAuthReq->lUserA        = revBuf->usPonId;
                    pAuthReq->lUserB        = revBuf->usOnuId;
    
                    pAuthReq->lType         = (LONG)(*(unsigned short*)(pDatBuf + IGMP_AUTH_FILED_OFF_TYPE));
                    pAuthReq->lCvlan        = (LONG)(*(unsigned long *)(pDatBuf + IGMP_AUTH_FILED_OFF_SPORT));
                    pAuthReq->lMvlan        = (LONG)(*(unsigned short*)(pDatBuf + IGMP_AUTH_FILED_OFF_VID));
                    pAuthReq->ulCip         = (LONG)(*(unsigned long *)(pDatBuf + IGMP_AUTH_FILED_OFF_SIP));
                    pAuthReq->ulMip         = (ULONG)(*(unsigned long *)(pDatBuf + IGMP_AUTH_FILED_OFF_MCIP));
                    pAuthReq->lDevType      = IGMP_AUTH_ONUTYPE_GW;
                    memcpy(pAuthReq->ucCmac, pDatBuf + IGMP_AUTH_FILED_OFF_SMAC, 6);
                    
                    if((pAuthReq->lType == IGMP_AUTH_REQTYPE_JOIN) || (pAuthReq->lType == IGMP_AUTH_REQTYPE_LEAVE))
                    {
                        ulMessage[3] = (unsigned long)pQueMsg;

                        if(VOS_QueSend(gulIgmpAuthQid, ulMessage, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
                        {
                            VOS_Free(pQueMsg);
                            CDP_FreeMsg(pData);
                            IGMP_AUTH_DEBUG(("Send igmpauth msg  failed ! \r\n"));
                            return ;
                        }
                    }
                    else
                    {
                        VOS_Free(pQueMsg);
                        CDP_FreeMsg(pData);
                        return ;
                    }

                    CDP_FreeMsg(pData);
			break;
		case CDP_NOTI_FLG_SEND_FINISH:/*异步发送时*/
			CDP_FreeMsg(pData);		/*异步发送失败暂不处理，但需要释放消息*/
			break;
		default:
			ASSERT(0);
			CDP_FreeMsg(pData);
			break;
	}

    return ;
    
}

static void funIgmpAuthOamAllCallBack(unsigned short usReqTyp, unsigned short usPonId,  
                                      unsigned short usOnuId,  unsigned short usDatLen, 
                                      unsigned char *pDatBuf,  unsigned char *pSessId)
{
    unsigned long     ulMessage[4] = {0};
    SYS_MSG_S        *pQueMsg      = NULL;
    Igmp_AuthInfo_t  *pAuthReq     = NULL;

    pQueMsg = (SYS_MSG_S*)VOS_Malloc(sizeof(SYS_MSG_S) + sizeof(Igmp_AuthInfo_t) , MODULE_IGMPAUTH);
    
    if(pQueMsg == NULL)
    {
        IGMP_AUTH_DEBUG(("Malloc memory faile in API fun.\r\n"));
	  VOS_Free(pDatBuf);
        VOS_Free(pSessId);
        return;
    }

    pAuthReq  = (Igmp_AuthInfo_t*)(pQueMsg + 1);
    
    VOS_MemZero(pQueMsg, sizeof(SYS_MSG_S) + sizeof(Igmp_AuthInfo_t));

    pQueMsg->ucMsgType      = MSG_NOTIFY;
    pQueMsg->ulSrcSlotID    = *((ULONG*)pSessId);
    pQueMsg->ulDstSlotID    = *((ULONG*)(pSessId+4) ); 
    pQueMsg->usMsgCode      = usReqTyp;
    pQueMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;
    pQueMsg->usFrameLen     = 0;
    pQueMsg->ptrMsgBody     = 0;

    pAuthReq->lUserA        = usPonId;
    pAuthReq->lUserB        = usOnuId;
    pAuthReq->lType         = (LONG)(*(unsigned short*)(pDatBuf + IGMP_AUTH_FILED_OFF_TYPE));
    pAuthReq->lCvlan        = (LONG)(*(unsigned long *)(pDatBuf + IGMP_AUTH_FILED_OFF_SPORT));
    pAuthReq->lMvlan        = (LONG)(*(unsigned short*)(pDatBuf + IGMP_AUTH_FILED_OFF_VID));
    pAuthReq->ulCip         = (LONG)(*(unsigned long *)(pDatBuf + IGMP_AUTH_FILED_OFF_SIP));
    pAuthReq->ulMip         = (ULONG)(*(unsigned long *)(pDatBuf + IGMP_AUTH_FILED_OFF_MCIP));
    pAuthReq->lDevType      = IGMP_AUTH_ONUTYPE_GW;
    memcpy(pAuthReq->ucCmac, pDatBuf + IGMP_AUTH_FILED_OFF_SMAC, 6);
    if(glIgmpAuthDebug)
    {
        sys_console_printf("In func:funIgmpAuthOamAllCallBack\r\n");
        sys_console_printf("usPonId is %d,usOnuId is %d\r\n",usPonId,usOnuId);
    }
    /*
	if(usReqTyp == IGMP_AUTH_MSGCODE_ACK) 
    {
        pAuthReq->lResult =(long)(*(unsigned short*)(pDatBuf + IGMP_AUTH_FILED_OFF_RESU));
	}

    VOS_MemCpy(pAuthReq->bSessId,  pSessId, 8);  
    */

    if((pAuthReq->lType == IGMP_AUTH_REQTYPE_JOIN) || (pAuthReq->lType == IGMP_AUTH_REQTYPE_LEAVE))
    {
        ulMessage[3] = (unsigned long)pQueMsg;

        if(VOS_QueSend(gulIgmpAuthQid, ulMessage, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
        {
            VOS_Free(pQueMsg);
         
            IGMP_AUTH_DEBUG(("Send msg  fail in API\r\n"));
        }
    }
    else
    {
        VOS_Free(pQueMsg);
    }

	IGMP_AUTH_DEBUG(("Send req Or ack msg to queue, type = 0x%04x\r\n", usReqTyp));
	
    VOS_Free(pDatBuf);
    VOS_Free(pSessId);
}


/*================================================*/
/*name: funIgmpAuthReqCallBack                    */
/*para: usPonId                                   */ 
/*para: usOnuId                                   */ 
/*para: lDatLen  数据长度                         */
/*para: pDatBuf  数据指针                         */
/*para: pcSessId 会话ID                           */
/*retu: none                                      */
/*desc: 组播认证请求报文的回调函数                */
/*================================================*/

static int funIgmpAuthOamCdp(unsigned short usReqTyp, unsigned short usPonId,  
                                      unsigned short usOnuId,  unsigned short usDatLen, 
                                      unsigned char *pDatBuf,  unsigned char *pSessId)
{
    IgmpAuthCDPMsgHead_t *bDatPdpBuf = NULL;
    unsigned int ulLen;
    SYS_MSG_S *pMsg = NULL;

    if( devsm_sys_is_switchhovering())  /* 问题单: 16439 modified by duzhk 该问题主要是因为倒换期间因收到太多管理报文等原因导致
        					CDP通道太忙，参考解指导的意见，注释掉断言*/
		return VOS_OK;

    ulLen=sizeof(IgmpAuthCDPMsgHead_t)+usDatLen+sizeof(SYS_MSG_S);
    pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_IGMPAUTH);
    /*sys_console_printf("In Function : funIgmpAuthOamCdp\r\n");
    sys_console_printf("usPonId is %d,usOnuId is %d\r\n",usPonId,usOnuId);*/
    if( NULL == pMsg )
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }
    VOS_MemZero((CHAR *)pMsg, ulLen );

    SYS_MSG_SRC_ID( pMsg )       = MODULE_IGMPAUTH;
    SYS_MSG_DST_ID( pMsg )       = MODULE_IGMPAUTH;
    SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
    SYS_MSG_FRAME_LEN( pMsg )    = ulLen;
    SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
    SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
    SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
    SYS_MSG_DST_SLOT( pMsg ) = SYS_MASTER_ACTIVE_SLOTNO;

    bDatPdpBuf= (IgmpAuthCDPMsgHead_t * ) ( pMsg + 1 );
    bDatPdpBuf->usReqTyp = usReqTyp;
    bDatPdpBuf->usPonId = usPonId;
    bDatPdpBuf->usOnuId = usOnuId;
    VOS_MemCpy(bDatPdpBuf->pSessId, pSessId, 8);
    bDatPdpBuf->usDatLen = usDatLen;
    bDatPdpBuf->pDatBuf = (unsigned char *)(bDatPdpBuf+1);
    bDatPdpBuf->ussrcslot = SYS_LOCAL_MODULE_SLOTNO ;

    VOS_MemCpy( bDatPdpBuf->pDatBuf,  pDatBuf, usDatLen);

    if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_IGMPAUTH_GW, SYS_MASTER_ACTIVE_SLOTNO, RPU_TID_CDP_IGMPAUTH_GW, /*CDP_MSG_TM_ASYNC*/ 0,\
                 (VOID *)pMsg, ulLen, MODULE_IGMPAUTH ) )
    {
        /*VOS_ASSERT(0); */ /* 问题单: 16439 modified by duzhk 该问题主要是因为倒换期间因收到太多管理报文等原因导致
        					CDP通道太忙，参考解指导的意见，注释掉断言*/
        IGMP_AUTH_DEBUG(("\r\nSend the IgmpAuth GW Oam to the Master Failed!\r\n"));
        CDP_FreeMsg(pMsg);
        return VOS_ERROR;
    }
    IGMP_AUTH_DEBUG(("\r\nSend the IgmpAuth GW Oam to the Master Success! \r\n"));
    VOS_Free(pDatBuf);
    VOS_Free(pSessId);
    return VOS_OK;

}
/*
 int funIgmpAuthOamCdpTest()
{
    IgmpAuthCDPMsgHead_t *bDatPdpBuf = NULL;
    unsigned int ulLen;
    SYS_MSG_S *pMsg = NULL;

    ulLen=sizeof(IgmpAuthCDPMsgHead_t)+sizeof(SYS_MSG_S);
    pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_IGMPAUTH);
    sys_console_printf("In Function : funIgmpAuthOamCdpTest\r\n");

    if( NULL == pMsg )
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }
    VOS_MemZero((CHAR *)pMsg, ulLen );

    SYS_MSG_SRC_ID( pMsg )       = MODULE_IGMPAUTH;
    SYS_MSG_DST_ID( pMsg )       = MODULE_IGMPAUTH;
    SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
    SYS_MSG_FRAME_LEN( pMsg )    = ulLen;
    SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
    SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
    SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
    SYS_MSG_DST_SLOT( pMsg ) = SYS_MASTER_ACTIVE_SLOTNO;

    bDatPdpBuf= (IgmpAuthCDPMsgHead_t * ) ( pMsg + 1 );
    bDatPdpBuf->usReqTyp = 1;
    bDatPdpBuf->usPonId = 10;
    bDatPdpBuf->usOnuId = 1;
    bDatPdpBuf->usDatLen = 0;
    bDatPdpBuf->pDatBuf = (unsigned char *)(bDatPdpBuf+1);
    bDatPdpBuf->ussrcslot = SYS_LOCAL_MODULE_SLOTNO ;
    sys_console_printf("length is %d\r\n",ulLen);
    sys_console_printf("sizeof(IgmpAuthCDPMsgHead_t) is %d\r\n",sizeof(IgmpAuthCDPMsgHead_t));
    if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_IGMPAUTH_GW, SYS_MASTER_ACTIVE_SLOTNO, RPU_TID_CDP_IGMPAUTH_GW,  0,\
                 (VOID *)pMsg, ulLen, MODULE_IGMPAUTH ) )
    {
        VOS_ASSERT(0); 
        sys_console_printf("\r\nSend the IgmpAuth GW Oam to the Master Failed!\r\n");
        CDP_FreeMsg(pMsg);
        return VOS_ERROR;
    }
    sys_console_printf("\r\nSend the IgmpAuth GW Oam to the Master Success! \r\n");
    return VOS_OK;

}
*/

void funIgmpAuthOamReqCallBack(unsigned short usPonId,  unsigned short usOnuId, unsigned short llid,
                               unsigned short usDatLen, unsigned char *pDatBuf,  unsigned char *pSessId)
{
    if(funIgmpAuthRevPktCheck(IGMP_AUTH_MSGCODE_OAMREQ, usDatLen, pDatBuf, pSessId) == VOS_ERROR) 
        return;
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
        funIgmpAuthOamAllCallBack(IGMP_AUTH_MSGCODE_OAMREQ, usPonId, usOnuId, usDatLen, pDatBuf, pSessId);
    else
        funIgmpAuthOamCdp(IGMP_AUTH_MSGCODE_OAMREQ, usPonId+1, usOnuId, usDatLen, pDatBuf, pSessId);
}

/*================================================*/
/*name: funIgmpAuthAckCallBack                    */
/*para: usPonId                                   */ 
/*para: usOnuId                                   */ 
/*para: lDatLen  数据长度                         */
/*para: pDatBuf  数据指针                         */
/*para: pcSessId 会话ID                           */
/*retu: none                                      */
/*desc: 组播认证应答报文的回调函数                */
/*================================================*/
void funIgmpAuthOamAckCallBack(unsigned short usPonId,  unsigned short usOnuId,
                               unsigned short usDatLen, unsigned char *pDatBuf,  unsigned char *pSessId)
{
    VOS_Free(pDatBuf);
    VOS_Free(pSessId);

 /* funIgmpAuthOamAllCallBack(IGMP_AUTH_MSGCODE_OAMACK, usPonId, usOnuId, usDatLen, pDatBuf, pSessId);*/
}

void  IgmpAuth_CTC_CDP_Callback(ULONG ulFlag,		/*	消息标志，表示消息发送完毕或接收到消息 */
							 ULONG ulChID,		/* CDP 通道号，即哪个通道报告的。如果一个
													函数被注册到多个通道，则此参数可指示是
													哪个通道报告的 */
							 ULONG ulDstNode,	/* 数据的目的节点号，在组播发送时表示组播
													组ID广播时无意义 */
							 ULONG ulDstChId,	/* 数据的目的通道号 */
							 VOID  *pData,		/* 发送的数据 */
							 ULONG ulDataLen	/* 数据的长度 */
							)
{
        IgmpAuthCDPMsgHead_t * revBuf = NULL;
        ULONG SlotnoOnOlt = 0, PortnoOnOlt = 0;
        unsigned char *pDatBuf=NULL;   
        long lOltPort;

	switch( ulFlag )
	{
		case CDP_NOTI_FLG_RXDATA: /* 收到数据*/

                    
                    if( pData == NULL )
                    {
                        VOS_ASSERT( 0 );
                        return ;
                    }

                    if(ulChID != RPU_TID_CDP_IGMPAUTH_CTC || ulDstChId != RPU_TID_CDP_IGMPAUTH_CTC)
                    {
                        CDP_FreeMsg(pData);
                        return ;
                    }
                    
                    revBuf=(IgmpAuthCDPMsgHead_t *)(( SYS_MSG_S * )pData + 1);
                    pDatBuf = (unsigned char*)((IgmpAuthCDPMsgHead_t*)revBuf+1);
                        
                    SlotnoOnOlt = revBuf->ussrcslot;
                    PortnoOnOlt = revBuf->usPonId;
                    
                    revBuf->usPonId = GetPonPortIdxBySlot(SlotnoOnOlt,PortnoOnOlt);
                    IGMP_AUTH_DEBUG(("\r\nIn the received CDP packet(IgmpAuth CTC),Slot is %d,Port is %d,PonId is %d\r\n",SlotnoOnOlt,PortnoOnOlt,revBuf->usPonId));

			lOltPort = IFM_ETH_CREATE_INDEX(SlotnoOnOlt,PortnoOnOlt );

                    Igmp_Snoop_AuthRequire(lOltPort, revBuf->usPonId, revBuf->usOnuId, revBuf->IsDayaOnu,  0, (char *)pDatBuf, revBuf->usDatLen, MODULE_PON );

                    CDP_FreeMsg(pData);
			break;
		case CDP_NOTI_FLG_SEND_FINISH:/*异步发送时*/
			CDP_FreeMsg(pData);		/*异步发送失败暂不处理，但需要释放消息*/
			break;
		default:
			ASSERT(0);
			CDP_FreeMsg(pData);
			break;
	}

    return ;

}

LONG Igmp_Snoop_AuthRequire_Cdp(long lOltPort, long lPonId, long lOnuNo, long lOnuType, 
                            long lOnuPort, char *buffer, long ldatLen, long lModId)
{
    IgmpAuthCDPMsgHead_t *bDatPdpBuf = NULL;
    unsigned int ulLen,i;
    SYS_MSG_S *pMsg = NULL;
    char *p=NULL;
    IGMP_AUTH_DEBUG(("\r\nReceive CTC Packet,lPonId is %d,lOnuNo is %d\r\n",lPonId,lOnuNo));
    if(glIgmpAuthDebug)
    {
              /**p=(uchar*)buffer;*/
              p= buffer;
	       for(i=0;i<ldatLen ;i++)
		{
			sys_console_printf("%02x  " ,*p);
			if((i+1)%6==0)
			sys_console_printf("\r\n");
			p++;
		}
    }

    if( devsm_sys_is_switchhovering())  /* 问题单: 16439 modified by duzhk 该问题主要是因为倒换期间因收到太多管理报文等原因导致
        					CDP通道太忙，参考解指导的意见，注释掉断言*/
		return VOS_OK;
	
    ulLen=sizeof(IgmpAuthCDPMsgHead_t)+ldatLen+sizeof(SYS_MSG_S);
    pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_IGMPAUTH);

    if( NULL == pMsg )
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }
    VOS_MemZero((CHAR *)pMsg, ulLen );

    SYS_MSG_SRC_ID( pMsg )       = MODULE_IGMPAUTH;
    SYS_MSG_DST_ID( pMsg )       = MODULE_IGMPAUTH;
    SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
    SYS_MSG_FRAME_LEN( pMsg )    = ulLen;
    SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
    SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
    SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
    SYS_MSG_DST_SLOT( pMsg ) = SYS_MASTER_ACTIVE_SLOTNO;

    bDatPdpBuf= (IgmpAuthCDPMsgHead_t * ) ( pMsg + 1 );
    bDatPdpBuf->usReqTyp = 0;
    bDatPdpBuf->usPonId = lPonId;
    bDatPdpBuf->usOnuId = lOnuNo;
      IGMP_AUTH_DEBUG(("Igmp_Snoop_AuthRequire_Cdp:lOnuNo is %d\r\n",lOnuNo));
    bDatPdpBuf->IsDayaOnu = lOnuType;
    bDatPdpBuf->usDatLen = ldatLen;
    bDatPdpBuf->pDatBuf = (unsigned char *)(bDatPdpBuf+1);
    bDatPdpBuf->ussrcslot = SYS_LOCAL_MODULE_SLOTNO ;

    VOS_MemCpy( bDatPdpBuf->pDatBuf,  buffer, ldatLen);

    if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_IGMPAUTH_CTC, SYS_MASTER_ACTIVE_SLOTNO, RPU_TID_CDP_IGMPAUTH_CTC, /*CDP_MSG_TM_ASYNC*/ 0,\
                 (VOID *)pMsg, ulLen, MODULE_IGMPAUTH ) )
    {
        /*VOS_ASSERT(0);*/ /* 问题单: 16439 modified by duzhk 该问题主要是因为倒换期间因收到太多管理报文等原因导致
        					CDP通道太忙，参考解指导的意见，注释掉断言*/ 
        IGMP_AUTH_DEBUG(("\r\nSend the IgmpAuth CTC Oam to the Master Failed!\r\n"));
        CDP_FreeMsg(pMsg);
        return VOS_ERROR;
    }
    IGMP_AUTH_DEBUG(("\r\nSend the IgmpAuth CTC Oam to the Master Successed! \r\n"));
    return VOS_OK;

}


/*=================================================================================================================== */
/*=================================================================================================================== */
/* 任务入口           */
/*================================================*/
/*name: funIgmpAuthTask                           */
/*retu: none                                      */
/*desc: 任务入口                                  */
/*================================================*/
DECLARE_VOS_TASK(funIgmpAuthTask)
{
    unsigned long ulMessage[4] = {0};

    while(1)
    {
        if(VOS_QueReceive(gulIgmpAuthQid, ulMessage, WAIT_FOREVER) != VOS_ERROR)
        {
            if(ulMessage[3] != 0)
            {
		 
                funIgmpAuthProcMessages((char*)ulMessage[3]);
            }
            else
            {
                IGMP_AUTH_DEBUG(("Recv Message with no any data para in ulMessage[3]\r\n"));
            }
        }
        else
        {
            IGMP_AUTH_DEBUG(("Recv Message error\r\n"));
        }
    }
}


/*================================================*/
/*name: funIgmpAuthInit                           */
/*retu: VOS_OK / VOS_ERROR                        */
/*desc: 该函数用于初始化文件传送程序              */
/*================================================*/
/*下面函数接收主控 板转上来的OAM 报文*/

long funIgmpAuthInit(void)
{
	VOS_HANDLE task_id;
    
    gulIgmpAuthQid = VOS_QueCreate(IGMP_AUTH_QUEUE_LEN, VOS_MSG_Q_FIFO);
    if ( 0 == gulIgmpAuthQid )    
    {
        sys_console_printf("Init Igmp auth Module fail\r\n");
        return(VOS_ERROR);
    }
    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER 
	&& (SYS_LOCAL_MODULE_TYPE == MODULE_E_GFA6900_SW ||SYS_LOCAL_MODULE_TYPE == MODULE_E_GFA6900_12EPON_M ||
	   (SYS_LOCAL_MODULE_TYPE == MODULE_E_GFA8000_SW)))
    {
        CDP_Create( RPU_TID_CDP_IGMPAUTH_GW,  CDP_NOTI_VIA_FUNC, 0, IgmpAuth_GW_CDP_Callback) ;
        CDP_Create( RPU_TID_CDP_IGMPAUTH_CTC,  CDP_NOTI_VIA_FUNC, 0, IgmpAuth_CTC_CDP_Callback) ;
    }
    else
    {
        CDP_Create( RPU_TID_CDP_IGMPAUTH_GW,  CDP_NOTI_VIA_FUNC, 0, IgmpAuth_GW_CDP_Callback_Pon);
        CDP_Create( RPU_TID_CDP_IGMPAUTH_CTC,  CDP_NOTI_VIA_FUNC, 0, IgmpAuth_CTC_CDP_Callback_Pon);
    }
    CommOltMsgRvcCallbackInit(IGMP_AUTH_OAM_OPCODE_REQ, funIgmpAuthOamReqCallBack);
    /* 不处理私有OAM ACK
    CommOltMsgRvcCallbackInit(IGMP_AUTH_OPCODE_ACK, funIgmpAuthOamAckCallBack);
    */
	/* Problem number : 16991  modified by duzhk  2013-02-07*/
    /*if( (task_id = (void *)VOS_TaskCreate(IGMP_AUTH_TASKNAME, IGMP_AUTH_TASKPRI, funIgmpAuthTask, NULL)) == 0)*/
	if ( ( task_id = VOS_TaskCreateEx(IGMP_AUTH_TASKNAME, funIgmpAuthTask, IGMP_AUTH_TASKPRI, 80*1024, NULL) ) == 0)
    {
        sys_console_printf("Create Igmp auth task fail\r\n");
        return(VOS_ERROR);
    } 
     VOS_QueBindTask( task_id, gulIgmpAuthQid );	/* added by xieshl 20090622 */
   
    glIgmpTimerId = VOS_TimerCreate(MODULE_IGMPAUTH, 0, IGMP_AUTH_TIMER_LEN,  
                                    funIgmpAuthTimerCallback, NULL, VOS_TIMER_LOOP);
	
	Igmp_Snoop_AuthEna_Register(MODULE_IGMPAUTH, gulIgmpAuthQid);
 
    if(glIgmpTimerId == VOS_ERROR) return(VOS_ERROR);

    return(VOS_OK);
}

/*=================================================================================================================== */
/*=================================================================================================================== */

#endif


