/* ================================================ *
* FileName: TransFile.c                            *
* Author  : suxiqing                               *
* Date    : 2006-10-10                             *
* Description: 此文件仅为EPON设计, 实现通过OAM通道 *
*              向ONU发送或接收文件                 *
*
* modify history:
* 1 added by chenfj 2006/10/30
*    在ONU APP 文件传送结束后，修改相关状态及上报Trap信息
* 2 modified by chenfj 2006/10/31, for using onu app updateing handling
*     #3048 问题单: 修改ONU APP文件传送结束后的回显信息
* ================================================ */

#include "include/linux/list.h"
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

#include "transfile.h"

#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include  "V2R1_product.h"

#include "cli\cl_vty.h"

#include "lib_gwEponOnuMib.h"
#include "onu/onuOamUpd.h"

/*=================================================================================================================== */
/*=================================================================================================================== */
/*#define LOG_TYPE_OAM_FILETRANS       21 */

#define OAM_FILETX_FILETYPE_LEN      16          /* 文件类型长度     */
#define OAM_FILETX_FILENAME_LEN      128         /* 文件名长度       */
#define OAM_FILETX_FILELEN_MAX       0x400000 

#define OAM_FILETX_TIMER_LEN         1000        /* 定时器时长1秒    */
#define OAM_FILETX_IDLE_TOUT         40          /* 远端无回应超时 15  */
#define OAM_FILETX_TOTAL_TOUT        (OAM_FILETX_WAITTIME  - 5)


#define OAM_FILETX_TASKNAME          "tOamFileTrans"
#define OAM_FILETX_QUEUE_LEN         128         /* 队列长度         */
#define OAM_FILETX_TASKPRI           130         /* 任务优先级       */

/* added by xieshl 20100722,增加ONU删除处理消息 */
#define MSG_OAM_FILE_TX			0
#define MSG_ONU_DELETE_EVENT		1

/*=================================================================================================================== */
#define OAM_FILETX_PKTBUF_LEN        1500        /* 报文缓存         */
#define OAM_FILETX_PKTDAT_LEN        224         /* 报文数据长度   1400  *//*temp test 2007-03-06*/

/*=================================================================================================================== */
/*====================================================================================================================*/
#define OAM_FILETX_PKTDATA_HEAD_LEN  16    /*payload 中data之前的字节数*/

/*====================================================================================================================*/
/* IF DEBUG */
/* modified by chenfj 2008-3-20
	修改debug 信息的输出调用, 使用sys_console_printf(), 这样在输出时,不会同别的信息混乱
*/
#define OAM_FILETX_DEBUG(x)          do{ \
                                          if(glTransFileDebug) {sys_console_printf x;} \
                                       }while(0)   
#define OAM_FILETX_DEBUG_1(y)  do{ \
                                          if(glTransFileDataDebug) {VOS_SysLog y;} \
                                       }while(0)

/* ELSE 
#define OAM_FILETX_DEBUG(x)
*/

/*=================================================================================================================== */
/* OAM 帧操作码  */
#define OAM_FILETX_OPCODE_RWREQ      5           /* 读写请求             */
#define OAM_FILETX_OPCODE_TRDAT      7           /* 数据传送             */
#define OAM_FILETX_OPCODE_TRACK      8           /* 数据传送ACK          */

/*=================================================================================================================== */
/* 模块定义消息  */
#define OAM_FILETX_MSGCODE_TRREQ     (AWMC_PRV_BASE | 0x0a01)    
                                                 /* LOC传送请求         */
#define OAM_FILETX_MSGCODE_RWREQ     (AWMC_PRV_BASE | 0x0a05) 
                                                 /* REM读写请求         */
#define OAM_FILETX_MSGCODE_TRDAT     (AWMC_PRV_BASE | 0x0a07)  
                                                 /* 数据传送            */
#define OAM_FILETX_MSGCODE_TRACK     (AWMC_PRV_BASE | 0x0a08)   
                                                 /* 数据传送ACK         */
#define OAM_FILETX_MSGCODE_TIMER     (AWMC_PRV_BASE | 0x0a09)   
                                                 /* TIMER               */

/*=================================================================================================================== */

/* 状态机  */
#define OAM_FILETX_SMC_LO_START      1           /* 发送/接收启动      */
/* 本端写请求状态机*/
#define OAM_FILETX_SMC_LW_REQACK     2           /* 等待请求应答       */
#define OAM_FILETX_SMC_LW_DATACK     3           /* 等待数据应答       */
#define OAM_FILETX_SMC_LW_ENDACK     4           /* 等待结束应答       */

/* 本端读请求状态机*/
#define OAM_FILETX_SMC_LR_REQACK     2           /* 等待请求应答       */
#define OAM_FILETX_SMC_LR_REMSTA     3           /* 等待远端开始       */
#define OAM_FILETX_SMC_LR_REMDAT     4           /* 等待远端数据       */
#define OAM_FILETX_SMC_LR_ENDACK     5           /* 等待结束应答       */

#define OAM_FILETX_SMC_LO_SUCESS     9           /* 结束应答           */

/*=================================================================================================================== */
/* 报文应答代码 */
#define OAM_FILETX_ACKCODE_RDREJ     0x100       /* 读拒绝             */
#define OAM_FILETX_ACKCODE_RDACC     0x101       /* 读允许             */
#define OAM_FILETX_ACKCODE_WRREJ     0x200       /* 写拒绝             */
#define OAM_FILETX_ACKCODE_WRACC     0x201       /* 写允许             */
#define OAM_FILETX_ACKCODE_TRERR     0x300       /* 传送错误           */
#define OAM_FILETX_ACKCODE_TRSTA     0x301       /* 传送开始           */
#define OAM_FILETX_ACKCODE_TRRUN     0x302       /* 传送中             */
#define OAM_FILETX_ACKCODE_TREND     0x303       /* 传送结束           */

/*=================================================================================================================== */
/* 发送错误代码     */
#define OAM_FILETX_ERRCODE_SUCCESS   0x00        /* 成功               */

#define OAM_FILETX_ERRCODE_SYSBUSY   0x01        /* 系统忙             */
#define OAM_FILETX_ERRCODE_NORES     0x02        /* 系统资源不足       */
#define OAM_FILETX_ERRCODE_PROCERR   0x03        /* 系统处理错误       */
#define OAM_FILETX_ERRCODE_FLOWERR   0x04        /* 流程错误           */
#define OAM_FILETX_ERRCODE_NOFILE    0x05        /* 文件不存在         */
#define OAM_FILETX_ERRCODE_TOOLONG   0x06        /* 文件太长           */
#define OAM_FILETX_ERRCODE_TOOSHORT  0x07        /* 文件太短           */
#define OAM_FILETX_ERRCODE_LONOMACHT 0x08        /* 长度或偏移匹配错误 */
#define OAM_FILETX_ERRCODE_FCSERR    0x09        /* 数据校验错误       */
#define OAM_FILETX_ERRCODE_SAVEERR   0x0A        /* 文件保存错误       */

#define OAM_FILETX_ERRCODE_NOMEM     0x0B        /* 内存不足           */
#define OAM_FILETX_ERRCODE_NOSEM     0x0C        /* 信号量不足         */
#define OAM_FILETX_ERRCODE_SENDQUE   0x0D        /* 发送消息失败       */
#define OAM_FILETX_ERRCODE_PTIMEOUT  0x0E        /* 接收报文超时       */
#define OAM_FILETX_ERRCODE_FTIMEOUT  0x0F        /* 发送文件超时       */
#define OAM_FILETX_ERRCODE_TRYFAIL   0x10        /* 发送多次尝试失败   */
#define OAM_FILETX_ERRCODE_TRANERR   0x11        /* 传送出现错误       */
#define OAM_FILETX_ERRCODE_NOREQACK  0x12        /* 无请求应答         */
#define OAM_FILETX_ERRCODE_REQREJ    0x13        /* 请求被拒绝         */
#define OAM_FILETX_ERRCODE_NOENDACK  0x14        /* 无结束应答         */
#define OAM_FILETX_ERRCODE_ENDACKERR 0x15        /* 结束应答错误       */
#define OAM_FILETX_ERRCODE_NOREMSTA  0x16        /* 未收到远端的开始报文*/

#define OAM_FILETX_ERRCODE_HAVEINS   0x17        /* 该ONU已经在传送    */
#define OAM_FILETX_ERRCODE_READFILE  0x18        /* 读文件失败         */

#define OAM_FILETX_ERRCODE_MAX       0x19        

/*=================================================================================================================== */
typedef struct _OAM_FILETX_REQ_S_
{
    unsigned short        usPonID;                      
    unsigned short        usOnuID;

    long                  lReqType;
    long                  lCallType;

    long                 *plError;

    void                 *pArgVoid;
	
    unsigned long         ulSemID;

    char                  bFileType[OAM_FILETX_FILETYPE_LEN + 4];

    char                  bFileName[OAM_FILETX_FILENAME_LEN + 4];

    POAMTRANSFILECALLBACK pReqCallBack;

    POAMRECVFILECALLBACK  pFileCallBack;

}OAM_FILETX_REQ_S;


typedef struct _OAM_FILETX_CTRL_S_
{
    struct list_head   sLinkList;       

    long                 lTermType;            /* 终端类型 1 主 2从   */
    long                 lErrorNo;             /* 错误代码             */

    /*long                 lOffset;              发送或接收偏移   modify by luj 2006/10/23 改为无符号型避免负数出现    */
    /*long                 lSendLen;              当前发送报文的长度modify by luj 2006/10/23 改为无符号型避免负数出现   */
    /*long                 lFileLen;              文件数据总长      modify by luj 2006/10/23 改为无符号型避免负数出现   */
    /*long                 lMaxLen;              数据最大长度  modify by luj 2006/10/23 改为无符号型避免负数出现       */

    unsigned long                 lOffset;              /* 发送或接收偏移       */
    unsigned long                 lSendLen;             /* 当前发送报文的长度   */
    unsigned long                 lFileLen;              /*文件数据总长*/
    unsigned long                 lMaxLen;              /* 数据最大长度         */
    
    long                 lRepCoun;             /* 重发次数             */
    long                 lTimeOut;             /* 应答等待超时         */
    long                 lTotTout;             /* 总的超时控制         */
    long                 lRemIdle;             /* 远端无反映计时       */
    long                 lPrintTime;           /* 定时进程输出         */
	
    long                 lStateMc;             /* 当前状态             */

    char                 bSessId[8];           /* 会话句柄  只用四字节 */

    char                *pFileBuf;             /* 接收或发送缓存       */
    char                *pPktBuff;             /* 接收或发送缓存       */
   
    OAM_FILETX_REQ_S     sReqInfo;             /* 高层请求信息         */

}OAM_FILETX_CTRL_S;

typedef struct _OAM_FILETX_PKTHD_S_
{
    /* long    lFileLen;                          文件长度  modify by luj 2006/10/23  改为无符号型避免出现负数           */     
     /* long    lFileOff;                         文件偏移    modify by luj 2006/10/23  改为无符号型避免出现负数         */
  /*   long    lDataLen;                          数据长度      modify by luj 2006/10/23  改为无符号型避免出现负数         */

   unsigned long    lFileLen;                          /* 文件长度             */  
   unsigned long    lFileOff;                          /* 文件偏移             */
   unsigned  long    lDataLen;                          /* 数据长度             */	

    long    lDataFcs;                          /* 报文校验合           */
    char    pData[OAM_FILETX_PKTDAT_LEN];                     /*add by luj 2006/10/21*/

}OAM_FILETX_PKTHD_S;

typedef struct _OAM_FILETX_ACKHD_S_
{
    short   sAckCode;
    short   sErrCode;

     /* long    lFileLen;                        文件长度    modify by luj 2006/10/23 改为无符号型避免负数出现        */
    /*long    lFileOff;                           文件偏移     modify by luj 2006/10/23 改为无符号型避免负数出现          */
    /*long    lDataLen;                           数据长度    modify by luj 2006/10/23    改为无符号型避免负数出现      */
    unsigned  long    lFileLen;           /*文件长度*/
    unsigned  long    lFileOff;             /* 文件偏移             */
    unsigned  long    lDataLen;             /* 数据长度             */	
    long    lDataFcs;                          /* 报文校验合           */

}OAM_FILETX_ACKHD_S;


/*=================================================================================================================== */

static char *gpTransFileErrorInf[] =
{
    "Sucessfully",
    "System busy",
    "No enough resource",
    "System process error",
    "Control flow error",
    "No this file",
    "File too long",
    "File too short",
    "File length or offset match error",
    "Packet fcs error",
    "Save file error",
    "No enough memory or g_malloc memory fail",
    "No enough sem or create sem fail",
    "Send message to transfile queue fail",
    "Receive pkt time out",
    "Send or receive file timeout",
    "Send trying fail",
    "Transfer error",
    "Not received req ack",
    "Req be rejected",
    "Not received end ack",
    "End ack have error",
    "Not recieved remote starting pkt",
    "This ONU is transfering another file", 
    "Read file error",
    "",
};

struct vty *ONU_app_transfer_vty;

/*=================================================================================================================== */

static long             glTransSessionId     = 0;
long             glTransFileDebug     = 0;    /* 调试开关       */
long             glTransFileDataDebug = 0;
static long             glTransFileTimId     = 0;    
static unsigned long    gulTransFileQid      = 0;    /* 传送任务队列   */
static struct list_head gsTransFileLink      = {0};  /* 文件传送链     */

/*-----------------------------以下仅用于调试-------------------------------------------------------------*****/
/*static long             glTransFileDetailDebug   = 0; */   /* 调试开关      add by luj temp 2006/10/21 */

/*static long             glReceiveMesCount = 0;*/         /*统计受到的有效消息add by luj 2006/10/22*/
/*static long             glReceiveMesTRREQ = 0; 
static long             glReceiveMesDATA = 0; 
static long             glReceiveMesACK = 0; */

char               DATALEN_E  = 0;        /*add by luj 2006/10/23 仅用于调试-强行将数据长度置错*/
char                OFFSET_E  =   0;
unsigned long                FILELEN_E  =  0 ;
char               ACK_LOCK = 0;
char               PRINT_FILE= 0;
char               SEND_WRONG_DATA_ACK=0;
char               TEST_MODE =0;     /*0:NO TEST；1:重复测试;2:单次测试*/
char               dubleTime = 0;   /*add by luj 2006/10/27*/
/*---------------------------------------------------------------------------------------------*/

/*=================================================================================================================== */

extern int  CommOltMsgRvcCallbackInit(unsigned char GwProId, void *Function);

extern short int  FileRequestTransmit(const unsigned short PonId, const unsigned short OnuId, unsigned char  *pfileBuf,
				                        const unsigned int fileBufSize, unsigned char *psessionIdField);

extern short int FileAckTransmit(const unsigned short PonId, const unsigned short OnuId, unsigned char  *pfileBuf,
				                        const unsigned int fileBufSize, unsigned char *psessionIdField);
				                        
extern short int FileDataTransmit(const unsigned short PonId, const unsigned short OnuId, unsigned char  *pfileBuf,
				                        const unsigned int fileBufSize, unsigned char *psessionIdField);


/*
int FileRequestTransmit(const unsigned short PonId, const unsigned short OnuId, unsigned char  *pfileBuf,
				                        const unsigned short fileBufSize, unsigned char *psessionIdField)
{
   OAM_FILETX_DEBUG((LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,"FileRequestTransmit, PonId = %d, OnuId = %d, psessionIdField = %d\r\n", 
                     PonId, OnuId, *(long*)psessionIdField));
   return 0;
}


int FileAckTransmit(const unsigned short PonId, const unsigned short OnuId, unsigned char  *pfileBuf,
				                        const unsigned short fileBufSize, unsigned char *psessionIdField)
{
   OAM_FILETX_DEBUG((LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,"FileAckTransmit, PonId = %d, OnuId = %d, psessionIdField = %d\r\n", 
                     PonId, OnuId, *(long*)psessionIdField));
   return 0;
}

int FileDataTransmit(const unsigned short PonId, const unsigned short OnuId, unsigned char  *pfileBuf,
				                        const unsigned short fileBufSize, unsigned char *psessionIdField)
{
   OAM_FILETX_DEBUG((LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,"FileDataTransmit, PonId = %d, OnuId = %d, psessionIdField = %d\r\n", 
                     PonId, OnuId, *(long*)psessionIdField));
   return 0;
}
*/

extern long get_ONU_file(char *onu_Type, char *buffer, int *file_len, char *target_type);
extern long get_ONU_Info(char *onu_Type, int *offset,  int *file_len, int *compress_flag, int *file_fact_len, int * version);
/*extern void delayforwhil();*/

/*=================================================================================================================== */
/*=================================================================================================================== */
/*API 部分   */
/*================================================*/
/*name: funOamTransFileLocReqApi                  */
/*para: lReqType  指示是否为发送请求1 read/2 write*/ 
/*para: lOnuID    ONU 号                          */ 
/*para: lFileType 需要发送的文件类型              */
/*para: lCallType 函数的调用方式 1 normal 2block  */
/*para: pCallback 回调函数                        */
/*retu: ERROR CODE (0 no error >0 have error)     */
/*desc: 调用该函数向指定ONU发送指定类型文件       */
/*================================================*/
   
long funOamTransFileLocReqApi(long  lReqType,   unsigned short usPonID, unsigned short usOnuID, 
                              char *pFileType,  char *pFileName, long lCallType, void *pArgVoid,
                              POAMTRANSFILECALLBACK pReqCallback, POAMRECVFILECALLBACK pFileCallBack)
{
    long                   lRetError    = OAM_FILETX_ERRCODE_SUCCESS;
    unsigned long          ulWaitSem    = 0;      
    unsigned long          ulMessage[4] = {MODULE_RPU_TRANSFILE, MSG_OAM_FILE_TX, 0, 0};
    SYS_MSG_S             *pWholeMsg    = NULL;
    OAM_FILETX_REQ_S      *pReqInfor    = NULL;

    pWholeMsg = (SYS_MSG_S*)VOS_Malloc(sizeof(SYS_MSG_S) + sizeof(OAM_FILETX_REQ_S), MODULE_RPU_TRANSFILE);

    if(pWholeMsg == NULL) 
    {
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */"Malloc memory fail in funEponOamTransFileToOnu, onu%d/%d\r\n",usPonID,usOnuID));
        return(OAM_FILETX_ERRCODE_NOMEM);
    }
    
    pReqInfor = (OAM_FILETX_REQ_S*)(pWholeMsg + 1);

    VOS_MemZero(pWholeMsg, sizeof(SYS_MSG_S));

    pWholeMsg->ucMsgType     = MSG_REQUEST;
    pWholeMsg->usMsgCode     = OAM_FILETX_MSGCODE_TRREQ;
    pWholeMsg->ucMsgBodyStyle= MSG_BODY_INTEGRATIVE;

    pReqInfor->usPonID       = usPonID;
    pReqInfor->usOnuID       = usOnuID;
    pReqInfor->lCallType     = lCallType;
    pReqInfor->lReqType      = lReqType;
	pReqInfor->pArgVoid      = pArgVoid;

    /* B--modified by liwei056@2011-2-15 for CodeCheckBug */
#if 0
    VOS_MemCpy(pReqInfor->bFileType, pFileType, OAM_FILETX_FILETYPE_LEN);
    VOS_MemCpy(pReqInfor->bFileName, pFileName, OAM_FILETX_FILENAME_LEN);  /* 内存读越界 */
#else
    VOS_StrnCpy(pReqInfor->bFileType, pFileType, OAM_FILETX_FILETYPE_LEN);
    VOS_StrnCpy(pReqInfor->bFileName, pFileName, OAM_FILETX_FILENAME_LEN);
#endif
    /* E--modified by liwei056@2011-2-15 for CodeCheckBug */
    
    pReqInfor->bFileType[OAM_FILETX_FILETYPE_LEN] = 0;
    pReqInfor->bFileName[OAM_FILETX_FILENAME_LEN] = 0;

    if(lCallType != OAM_FILETX_CALL_BLOCK)
    {
        pReqInfor->pReqCallBack = pReqCallback;
        pReqInfor->plError      = NULL;
        pReqInfor->ulSemID      = 0;
    }
    else
    {
        ulWaitSem = VOS_SemBCreate(VOS_SEM_Q_FIFO, VOS_SEM_EMPTY);

        if(ulWaitSem == 0)
        {
            /*VOS_Free(pReqInfor);*/	/* modified by xieshl 20100722 */
            VOS_Free(pWholeMsg);
            OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Create sem fail in funEponOamTransFileToOnu,onu%d/%d\r\n",usPonID,usOnuID));
            return(OAM_FILETX_ERRCODE_NOSEM);
        }

        pReqInfor->pReqCallBack = pReqCallback;
        pReqInfor->plError      = &lRetError;
        pReqInfor->ulSemID      = ulWaitSem;
    }
    
    if(lReqType == OAM_FILETX_REQ_READ) pReqInfor->pFileCallBack = pFileCallBack;

    ulMessage[3] = (unsigned long)pWholeMsg;

    if(VOS_QueSend(gulTransFileQid, ulMessage, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
    {
        VOS_Free(pWholeMsg);

        if(lCallType == OAM_FILETX_CALL_BLOCK) VOS_SemDelete(ulWaitSem);
            
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Send msg fail in funEponOamTransFileToOnu,onu%d/%d\r\n",usPonID,usOnuID));

        return(OAM_FILETX_ERRCODE_SENDQUE);
    }
    else
    {
        if(lCallType == OAM_FILETX_CALL_BLOCK) 
        {
            long lRetSta = 0;

            lRetSta = VOS_SemTake(ulWaitSem, VOS_TICK_SECOND * OAM_FILETX_WAITTIME);

            if(lRetSta != VOS_OK)
            {
                lRetError = OAM_FILETX_ERRCODE_FTIMEOUT;
                OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Send file timeout in funEponOamTransFileToOnu,onu%d/%d\r\n",usPonID,usOnuID));
            }
           
            VOS_SemDelete(ulWaitSem);

            OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Return error code %s, onu%d/%d\r\n", funOamTransFileGetErrStrApi(lRetError),usPonID,usOnuID));
        }
        else lRetError = OAM_FILETX_ERRCODE_SUCCESS;

        return(lRetError);
    }
}

/*================================================*/
/*name: funOamTransFileGetErrStrApi               */
/*para: lErrorNo  错误号                          */ 
/*retu: error inf string                          */
/*desc: 调用该函数获取发送文件的错误信息串        */
/*================================================*/
char *funOamTransFileGetErrStrApi(long lErrorNo)
{
   if((lErrorNo < 0) || (lErrorNo >= OAM_FILETX_ERRCODE_MAX))
   {
       lErrorNo = OAM_FILETX_ERRCODE_MAX;
   }

   return(gpTransFileErrorInf[lErrorNo]);
}


/*=================================================================================================================== */
/*=================================================================================================================== */
/* 定时器回调 */
/*================================================*/
/*name: funOamTransFileTimerCallback              */
/*para: void *                                    */
/*retu: none                                      */
/*desc: 任务定时器回调函数                        */
/*================================================*/
static void funOamTransFileTimerCallback(void *pArg)
{
    SYS_MSG_S    *pSysMsgA = NULL;
    unsigned long ulMessage[4] = {MODULE_RPU_TRANSFILE, MSG_OAM_FILE_TX, 0, 0};

    if( VOS_QueNum(gulTransFileQid) > 5 )	/* modified by xieshl 20100722, 当任务忙时，定时消息暂停发送 */
    {
    	return;
    }
	
    pSysMsgA = (SYS_MSG_S*)VOS_Malloc(sizeof(SYS_MSG_S), MODULE_RPU_TRANSFILE);
    
    if(NULL == pSysMsgA)
    {
        ASSERT(0);
        return;
    }

    VOS_MemZero(pSysMsgA, sizeof(SYS_MSG_S));

    pSysMsgA->ucMsgType       = MSG_TIMER;
    pSysMsgA->usMsgCode       = OAM_FILETX_MSGCODE_TIMER;
    pSysMsgA->ucMsgBodyStyle  = MSG_BODY_INTEGRATIVE;
    
    ulMessage[3] = (unsigned long)pSysMsgA;

    if(VOS_QueSend(gulTransFileQid,  ulMessage, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
    {
        VOS_Free(pSysMsgA);
        ASSERT(0);
        return;
    }
}

/*=================================================================================================================== */
/*=================================================================================================================== */
/* 处理过程部分 */
/*================================================*/
/*name: funOamTransFileFindInstance               */
/*para: usPonID  PONID                            */
/*para: usOnuID  ONUID                            */
/*retu: OAM_FILETX_CTRL_S 指针                    */
/*desc: 查找该ONU是否已有一个传送实例             */
/*================================================*/

static OAM_FILETX_CTRL_S *funOamTransFileFindInstance(unsigned short usPonID, unsigned short usOnuID)
{
    struct list_head       *pListhead = &gsTransFileLink;
    struct list_head       *pListItem = NULL;
    OAM_FILETX_CTRL_S      *pTransCtl = NULL;
   
    if(list_empty(pListhead)) return(NULL);

    for(pListItem = pListhead->next; pListItem != pListhead; pListItem = pListItem->next)
    {
        pTransCtl = list_entry(pListItem, OAM_FILETX_CTRL_S, sLinkList);
        
        if((pTransCtl->sReqInfo.usPonID == usPonID) && 
           (pTransCtl->sReqInfo.usOnuID == usOnuID)) return(pTransCtl);
    }

    return(NULL);
}

/*================================================*/
/*name: funOamTransFileMatchInstance              */
/*para: usPonID  PONID                            */
/*para: usOnuID  ONUID                            */
/*para: ulSesID  session id                       */
/*retu: OAM_FILETX_CTRL_S 指针                    */
/*desc: 查找该ONU是否已有一个传送实例             */
/*================================================*/

static OAM_FILETX_CTRL_S *funOamTransFileMatchInstance(unsigned short usPonID, unsigned short usOnuID, 
                                                       unsigned long  ulSesID)
{
    struct list_head       *pListhead = &gsTransFileLink;
    struct list_head       *pListItem = NULL;
    OAM_FILETX_CTRL_S      *pTransCtl = NULL;
   
    if(list_empty(pListhead)) return(NULL);

    for(pListItem = pListhead->next; pListItem != pListhead; pListItem = pListItem->next)
    {
        pTransCtl = list_entry(pListItem, OAM_FILETX_CTRL_S, sLinkList);
        
        if((pTransCtl->sReqInfo.usPonID == usPonID) && 
           (pTransCtl->sReqInfo.usOnuID == usOnuID) &&
           (*((unsigned long*)pTransCtl->bSessId) == ulSesID)) 
        {
            return(pTransCtl);
        }
    }

    return(NULL);
}

/*================================================*/
/*name: funOamTransFileAppReqAck                  */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 管理层发送或接收请求的应答处理            */
/*================================================*/

static void funOamTransFileAppReqAck(OAM_FILETX_REQ_S *pReqInf, long lFileLen, long lErrNo)
{
    if(pReqInf->pReqCallBack != NULL) 
    {
        pReqInf->pReqCallBack(pReqInf->lReqType, pReqInf->usPonID, pReqInf->usOnuID, 
                              pReqInf->bFileName, pReqInf->pArgVoid, lFileLen, lFileLen, lErrNo);
    }

    if(pReqInf->lCallType == OAM_FILETX_CALL_BLOCK)
    {
        if(pReqInf->plError != NULL)  *pReqInf->plError = lErrNo;

        if(pReqInf->ulSemID != 0)      VOS_SemGive(pReqInf->ulSemID);
    }
}

/*================================================*/
/*name: funOamTransFileEndInstance                */
/*para: pTransCtl                                 */
/*retu: NONE                                      */
/*desc: 结束一个传送实例                          */
/*================================================*/

static void funOamTransFileEndInstance(OAM_FILETX_CTRL_S *pTransCtl)
{
	
	/*CHAR * FileCopyP=NULL;*/
       OAM_FILETX_DEBUG(("funOamTransFileEndInstance:%d-%d\r\n", pTransCtl->sReqInfo.usPonID, pTransCtl->sReqInfo.usOnuID));
	
    if(pTransCtl->pPktBuff != NULL) VOS_Free(pTransCtl->pPktBuff);

    if(pTransCtl->sReqInfo.lReqType == OAM_FILETX_REQ_WRITE)
    {                                                    /* 直接释放资源  */       
        if(pTransCtl->pFileBuf != NULL) g_free(pTransCtl->pFileBuf);
    }
    else
    {
        if(pTransCtl->lStateMc == OAM_FILETX_SMC_LO_SUCESS)
        {
            if(pTransCtl->sReqInfo.pFileCallBack != NULL) /*发送读取数据到相关处理模块。*/
            {
                pTransCtl->sReqInfo.pFileCallBack(pTransCtl->sReqInfo.usPonID,   pTransCtl->sReqInfo.usOnuID,
                                                  pTransCtl->sReqInfo.bFileName, pTransCtl->pFileBuf, pTransCtl->lFileLen);
            }
        }
        else
        {
            if(pTransCtl->pFileBuf != NULL) g_free(pTransCtl->pFileBuf);
        }
    }

    funOamTransFileAppReqAck(&pTransCtl->sReqInfo, pTransCtl->lFileLen, pTransCtl->lErrorNo);

#if 0
   /*********************add by luj 2006/10/23*****only for debug **********/
   if((PRINT_FILE==1)&&(pTransCtl->sReqInfo.lReqType == OAM_FILETX_REQ_READ))
   	{
		if((pTransCtl->lFileLen!=0)&&(pTransCtl->lFileLen<OAM_FILETX_FILELEN_MAX))
		FileCopyP =(char *) g_malloc(pTransCtl->lFileLen);
		if(FileCopyP!=NULL)
		{
			VOS_MemCpy(FileCopyP, pTransCtl->pFileBuf, pTransCtl->lFileLen);
		}
		printf("\r\n^^^^^^ FILE ADDRESS = %X\r\n",FileCopyP);
   	}

   /*******************************************************************/
 #endif 
 
    list_del(&pTransCtl->sLinkList);

    if(list_empty(&gsTransFileLink))
    {
       OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Control list is empty\r\n"));
    }


    VOS_Free(pTransCtl);
}

/*=================================================================================================================== */
/*=================================================================================================================== */
/* 读写通用发送处理部分  */
/*================================================*/
/*name: funOamTransFileSendReq                    */
/*para: pCtrItem  消息缓冲区                      */
/*retu: NONE                                      */
/*desc: 本地请求启动, 向对端发送请求              */
/*================================================*/

static void funOamTransFileSendReq(OAM_FILETX_CTRL_S *pCtrItem)
{
 short int result ;
    pCtrItem->lRepCoun ++;

    if(pCtrItem->lRepCoun > OAM_FILETX_RETRY_COU)
    {
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Sent req to Remote trying fail,onu%d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));

        pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_NOREQACK;
        funOamTransFileEndInstance(pCtrItem);
    }
    else
    {
        pCtrItem->lTimeOut    = OAM_FILETX_RETRY_INT; /* 启动定时器  */
        pCtrItem->pPktBuff[0] = (char)pCtrItem->sReqInfo.lReqType;

        VOS_MemCpy((pCtrItem->pPktBuff + 1), pCtrItem->sReqInfo.bFileName, OAM_FILETX_FILENAME_LEN);

	/*FileRequestTransmit(pCtrItem->sReqInfo.usPonID,  pCtrItem->sReqInfo.usOnuID,
                            pCtrItem->pPktBuff, VOS_StrLen(pCtrItem->sReqInfo.bFileName) + 1, pCtrItem->bSessId);
                            modify by luj 2006/10/22 判断处理结果*/
    
       result= FileRequestTransmit(pCtrItem->sReqInfo.usPonID,  pCtrItem->sReqInfo.usOnuID,
                            pCtrItem->pPktBuff, VOS_StrLen(pCtrItem->sReqInfo.bFileName) + 1, pCtrItem->bSessId);
	   
        
 /**add by luj only for debug 2006/10/22*
 printf("ponId =%x;onu = %x,bSessId = %x\r\n ",pCtrItem->sReqInfo.usPonID,pCtrItem->sReqInfo.usOnuID,pCtrItem->bSessId);
 printf("oam_trans_result = %d\r\n",result);
 	printf("send over ! pCtrItem->pPktBuff  = %x\r\n",pCtrItem->pPktBuff );
 ******************************************************************************/ 
       OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Sent req to Remote count = %d,onu%d/%d\r\n", pCtrItem->lRepCoun,pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
    }
}

/*=================================================*/
/*name: funOamTransFileSendErrAck                  */
/*para: pCtrItem  消息缓冲区                       */
/*para: pAckHead  应答报文头                       */ 
/*retu: NONE                                       */
/*desc: 本地发现错误向对端发送错误ACK, 然后终止流程*/
/*=================================================*/

static void funOamTransFileSendErrAck(OAM_FILETX_CTRL_S *pCtrItem) 
{
    OAM_FILETX_ACKHD_S   *pAckHead;

    pCtrItem->lTimeOut = OAM_FILETX_RETRY_NO;    /* 数据应答不重发*/

    pAckHead = (OAM_FILETX_ACKHD_S*)pCtrItem->pPktBuff;

    pAckHead->lFileOff = pCtrItem->lOffset;
    pAckHead->lDataLen = 0;
    pAckHead->lDataFcs = 0;
    pAckHead->sErrCode = pCtrItem->lErrorNo;
    pAckHead->lFileLen = pCtrItem->lFileLen;
    pAckHead->sAckCode = OAM_FILETX_ACKCODE_TRERR;

    FileAckTransmit(pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID, 
                    pCtrItem->pPktBuff, sizeof(OAM_FILETX_ACKHD_S), pCtrItem->bSessId);

    OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Sent error ack errcode = %d to onu%d/%d\r\n", pCtrItem->lErrorNo,pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));

	funOamTransFileEndInstance(pCtrItem);
}

/*================================================*/
/*name: funOamTransFileSendEndAck                 */
/*para: pCtrItem  消息缓冲区                      */
/*retu: NONE                                      */
/*desc: 本地写请求情况下, 发送数据                */
/*================================================*/

static void funOamTransFileSendEndAck(OAM_FILETX_CTRL_S *pCtrItem)
{
    pCtrItem->lRepCoun ++;

    if(pCtrItem->lRepCoun > OAM_FILETX_RETRY_COU)
    {
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Send end ack to Remote, retrying fail,onu%d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));

        pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_NOENDACK;

        funOamTransFileEndInstance(pCtrItem);
    }
    else
    {
        OAM_FILETX_ACKHD_S *pAckHead;
        
        pAckHead = (OAM_FILETX_ACKHD_S*)pCtrItem->pPktBuff;

        pAckHead->sAckCode = OAM_FILETX_ACKCODE_TREND;
        pAckHead->sErrCode = OAM_FILETX_ERRCODE_SUCCESS;
        pAckHead->lFileLen = pCtrItem->lFileLen; 
	 pAckHead->lDataFcs = 0;
        if(pCtrItem->sReqInfo.lReqType == OAM_FILETX_REQ_READ)
        {
             pAckHead->lDataLen = pCtrItem->lSendLen;
             pAckHead->lFileOff    = pCtrItem->lFileLen - pCtrItem->lSendLen;
        }
        else
        {
        	pAckHead->lDataLen = 0;
              pAckHead->lFileOff   = pCtrItem->lFileLen;
        }
        pCtrItem->lTimeOut = OAM_FILETX_RETRY_INT; 

#if 0		

	 /*******add by luj 2006/10/23 only for debug *****强制发送错误ACK******/
        if(TEST_MODE == 1)
        	{
				if(SEND_WRONG_DATA_ACK==1)
			 	{

					pAckHead->lFileOff = 0xffff;
				    pAckHead->lDataLen = 0xffff;
			 	}

	 		}
		else if(TEST_MODE == 2)
			{
				if(SEND_WRONG_DATA_ACK==1)
			 	{

					pAckHead->lFileOff = 0xffff;
				    pAckHead->lDataLen = 0xffff;
			 	}

			 	SEND_WRONG_DATA_ACK = 0;
				TEST_MODE = 0;
	 		}

   /****************************************************/

#endif		

        FileAckTransmit(pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID, 
                        pCtrItem->pPktBuff, sizeof(OAM_FILETX_ACKHD_S), pCtrItem->bSessId);
   
   	/***add by luj 2006/10/24 only for debug*****
		printf(" %d,",pCtrItem->lRemIdle);
	****************************************/

		OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Send end ack to Remote onu %d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
    }
}



/*=================================================================================================================== */
/*=================================================================================================================== */
/* 本地文件写操作过程处理  */
/*================================================*/
/*name: funOamTransFileSendWrData                 */
/*para: pCtrItem  消息缓冲区                      */
/*retu: NONE                                      */
/*desc: 本地写请求情况下, 发送数据                */
/*================================================*/

static void funOamTransFileSendWrData(OAM_FILETX_CTRL_S *pCtrItem)
{
    pCtrItem->lRepCoun ++;

    if(pCtrItem->lRepCoun > OAM_FILETX_RETRY_COU)
    {
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Trying Send file data pkt fail to onu%d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));

        pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_TRYFAIL;
        funOamTransFileEndInstance(pCtrItem);
    }
    else
    {
        OAM_FILETX_PKTHD_S *pPktHead = NULL;
        pPktHead = (OAM_FILETX_PKTHD_S*)pCtrItem->pPktBuff;
    
        pPktHead->lFileLen = pCtrItem->lFileLen;
        pPktHead->lFileOff = pCtrItem->lOffset;
        pPktHead->lDataLen = pCtrItem->lSendLen;
        pPktHead->lDataFcs = 0;

        pCtrItem->lTimeOut = OAM_FILETX_RETRY_INT; /* 启动定时器 */
	/*add by luj 2006/10/21 备份数据块*/
	/*VOS_MemCpy( &(pPktHead->pData[0]), (pCtrItem->pFileBuf),  pPktHead->lDataLen);temp*/
	VOS_MemCpy( &(pPktHead->pData[0]), (pCtrItem->pFileBuf + pCtrItem->lOffset),  pPktHead->lDataLen);

#if 0

	/****************add by luj debug **********************/

	/****2006/10/23****used to send wrong pkt*********/
	if(TEST_MODE == 1)
		{
			if(DATALEN_E == 1)
				pPktHead->lDataLen = 0xFFFF;
			if(OFFSET_E == 1)
				pPktHead->lFileOff = 0xFFFF;
			if(FILELEN_E!=0)
				pPktHead->lFileLen = FILELEN_E;
		}
    else if(TEST_MODE == 2)
    	{
			if(DATALEN_E == 1)
				{
				  pPktHead->lDataLen = 0xFFFF;
				  DATALEN_E = 0;
				}
			if(OFFSET_E == 1)
				{
				  pPktHead->lFileOff = 0xFFFF;
				  OFFSET_E =0;
				}
			if(FILELEN_E!=0)
				{
				  pPktHead->lFileLen = FILELEN_E;
					FILELEN_E = 0;
				}
			TEST_MODE = 0;
    	}
	/**********************************************/
	

	if(glTransFileDetailDebug)
		{
			printf("\r\n---data information----\r\n");
			printf("DataLen =%4x \r\n",(pPktHead->lDataLen));
			for(iLoop=0;iLoop<(pPktHead->lDataLen);iLoop++)
				{
					printf("%2x ",pPktHead->pData[iLoop]);
				}
			printf("\r\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			for(iLoop=0;iLoop<(pPktHead->lDataLen);iLoop++)
				{
					printf("%2x ",pCtrItem->pFileBuf[iLoop]);
				}
			printf("\r\n---------------------------------\r\n");


		}

	/****************************************************/
#endif

	/****modify by luj 2006/10/21*********************/
    /* 真不明白在干啥  suxq 2007-06-1 
        VOS_MemCpy((pCtrItem->pPktBuff + sizeof(OAM_FILETX_PKTHD_S)),
                   (unsigned char *) pPktHead, (pCtrItem->lSendLen +OAM_FILETX_PKTDATA_HEAD_LEN));
    */
        FileDataTransmit(pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID,
                         pCtrItem->pPktBuff, (pCtrItem->lSendLen+OAM_FILETX_PKTDATA_HEAD_LEN), pCtrItem->bSessId);

	/***add by luj 2006/10/24 only for debug*****
		printf(" %d,",pCtrItem->lRemIdle);
	****************************************/

	/**************************/

	/*****debug 2006/10/22 by luj *********/
	#if 0
	if(glTransFileDetailDebug)
	{

		printf("\r\n---send information----\r\n");
		printf("payloadLen =%4x \r\n",(pCtrItem->lSendLen+OAM_FILETX_PKTDATA_HEAD_LEN));
		for(iLoop=0;iLoop<(pCtrItem->lSendLen+OAM_FILETX_PKTDATA_HEAD_LEN);iLoop++)
			{
				printf("%2x ",pCtrItem->pPktBuff[iLoop]);
			}
		printf("\r\n-------------\r\n");
	}
      #endif
	/***********************************/

	#if 0
	/*发送buf中未含代文件长度和偏移量等信息modify by luj 2006/10/21*/
        VOS_MemCpy((pCtrItem->pPktBuff + sizeof(OAM_FILETX_PKTHD_S)),
                   (pCtrItem->pFileBuf + pCtrItem->lOffset), pCtrItem->lSendLen);

        FileDataTransmit(pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID,
                         pCtrItem->pFileBuf, pCtrItem->lSendLen, pCtrItem->bSessId);
	#endif

 /*       OAM_FILETX_DEBUG((LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,
		                 "Send data pkt offset = %x (%d), filelen = %x, datalen = %x(%d)\r\n",
                          pPktHead->lFileOff, pPktHead->lFileOff,  pPktHead->lFileLen, pPktHead->lDataLen,pPktHead->lDataLen));
                          close by luj 2006/10/25 当大数据量输出时有输出消息丢失现象缩减数据字符增加效率*/
		/* modified by chenfj 2008-3-20
		     数据格式与参数不符,多了一个
		     */
		OAM_FILETX_DEBUG_1((LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,
		                 "T_D offset = %x (%d), FL = %x, DL = %x\r\n",
                          pPktHead->lFileOff, pPktHead->lFileOff,  pPktHead->lFileLen, pPktHead->lDataLen/*,pPktHead->lDataLen*/));

/***add by luj 2006/10/25 only for debug **********
	printf( "\r\nT_D offset = %x (%d), FL = %x, DL = %x\r\n",
                          pPktHead->lFileOff, pPktHead->lFileOff,  pPktHead->lFileLen, pPktHead->lDataLen);    
	   delayforwhil();
	******************************************/

    }
}

/*====================================================================== */

/*================================================*/
/*name: funOamTransFileRecvWrDataAck              */
/*para: pCtrItem  消息缓冲区                      */
/*para: pAckHead  应答报文头                      */ 
/*retu: NONE                                      */
/*desc: 本地写请求情况下, 收到数据应答            */
/*================================================*/

static void funOamTransFileRecvWrDataAck(OAM_FILETX_CTRL_S  *pCtrItem, OAM_FILETX_ACKHD_S *pAckHead)
{
   /* OAM_FILETX_DEBUG((LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, 
                      "Receive data ack offset = %x(%d), filelen = %x, datalen = %x(%d)\r\n",
                      pAckHead->lFileOff, pAckHead->lFileOff,pAckHead->lFileLen, pAckHead->lDataLen,pAckHead->lDataLen));
		close by luj 2006/10/25 当大数据量输出时有输出消息丢失现象缩减数据字符增加效率*/
	/* modified by chenfj 2008-3-20
		     数据格式与参数不符,多了一个
		     */
       OAM_FILETX_DEBUG_1((LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, 
                      "R_D_ACK offset = %x(%d), FL = %x, DL = %x\r\n",
                      pAckHead->lFileOff, pAckHead->lFileOff,pAckHead->lFileLen, pAckHead->lDataLen/*,pAckHead->lDataLen*/));


  /****add by luj 2006/10/25 only for debug **********
  if(dubleTime!=0)
  	{
	printf( "\r\nR_D_ACK offset = %x(%d), FL = %x, DL = %x,dubleTime=%x\r\n",
                      pAckHead->lFileOff, pAckHead->lFileOff,pAckHead->lFileLen, pAckHead->lDataLen,dubleTime);
	dubleTime --;
  	}

*	 delayforwhil();
 ********************************************/

    switch(pAckHead->sAckCode)
    {
        /*
        case OAM_FILETX_ACKCODE_TRERR:

             funOamTransFileRecvErrAck(pCtrItem, pAckHead);
             break;
        */
        /* 
           added by chenfj 2008-3-19
           用于写文件时控制层消息容错处理
           在等待ONU 的数据应答时, 增加对写允许, 及传送结束两个事件的处理
          */
	 case OAM_FILETX_ACKCODE_WRACC:
	 case OAM_FILETX_ACKCODE_TREND:
	 	if( glTransFileDebug )
		sys_console_printf("file:%s, line:%d, AckCode=%x from onu%d/%d\r\n",__FILE__, __LINE__, pAckHead->sAckCode, pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID );
       case OAM_FILETX_ACKCODE_TRSTA:  
        case OAM_FILETX_ACKCODE_TRRUN: 
 
             if((pAckHead->lFileLen == pCtrItem->lFileLen) &&
                (pAckHead->lDataLen == pCtrItem->lSendLen) && (pAckHead->lFileOff == pCtrItem->lOffset))
             {
                 pCtrItem->lOffset += pCtrItem->lSendLen;
                 pCtrItem->lRepCoun = 0;

                 if(pCtrItem->lOffset >= pCtrItem->lFileLen)
                 {
                     pCtrItem->lStateMc = OAM_FILETX_SMC_LW_ENDACK;
                     funOamTransFileSendEndAck(pCtrItem);
                 }
                 else
                 {
                     pCtrItem->lSendLen = OAM_FILETX_PKTDAT_LEN;  
                    
                     if((pCtrItem->lSendLen + pCtrItem->lOffset) > pCtrItem->lFileLen)  
                     {
                         pCtrItem->lSendLen = pCtrItem->lFileLen - pCtrItem->lOffset;  
                     }

                     funOamTransFileSendWrData(pCtrItem);
                 }
             }           
             else                                /* else  其他情况下 不理会收到的ACK报文 重发上一个报文 */
             {
                 funOamTransFileSendWrData(pCtrItem);
                
                 OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */
                                  "6 Receive ack with error data length or file length from onu%d/%d\r\n", pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID));
             }

             break;

	/* 
	    added by chenfj 2008-3-19
          用于写文件时控制层消息容错处理
	    若收到读允许,或读拒绝, 则忽略, 不做处理
	*/
	case OAM_FILETX_ACKCODE_RDACC:
	case OAM_FILETX_ACKCODE_RDREJ:
		if (glTransFileDebug )
		sys_console_printf("file:%s, line:%d, AckCode=%x from onu%d/%d\r\n",__FILE__, __LINE__, pAckHead->sAckCode, pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID );
		break;

	 case OAM_FILETX_ACKCODE_WRREJ:
        default:                                 /* 流程错误  */             

             pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_FLOWERR;

             funOamTransFileSendErrAck(pCtrItem);

             OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */
                              "7 Receive data ack from onu%d/%d with flow control error ackcode = %d\r\n",pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID, pAckHead->sAckCode));
             break;
    }
}
       

/*=================================================================================================================== */
/*=================================================================================================================== */
/* 本地文件读操作过程处理  */
/*================================================*/
/*name: funOamTransFileSendRdStartAck             */
/*para: pCtrItem  消息缓冲区                      */
/*retu: NONE                                      */
/*desc: 本地文件对操作, 发送报文数据开始发送应答  */
/*================================================*/
static void funOamTransFileSendRdStartAck(OAM_FILETX_CTRL_S  *pCtrItem) 
{
    pCtrItem->lRepCoun ++;

    if(pCtrItem->lRepCoun > OAM_FILETX_RETRY_COU)
    {
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/ "Trying sent start ack fail to onu %d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));

        pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_NOREMSTA;

        funOamTransFileEndInstance(pCtrItem);
    }
    else
    {
        OAM_FILETX_ACKHD_S   *pAckHead;

        pAckHead = (OAM_FILETX_ACKHD_S*)pCtrItem->pPktBuff;

        pAckHead->lFileOff = 0;
        pAckHead->lDataLen = 0;
        pAckHead->lDataFcs = 0;
        pAckHead->lFileLen = pCtrItem->lFileLen;
        pAckHead->sErrCode = OAM_FILETX_ERRCODE_SUCCESS;
        pAckHead->sAckCode = OAM_FILETX_ACKCODE_TRSTA;

        pCtrItem->lTimeOut = OAM_FILETX_RETRY_INT;   /* 可以重发  */

        FileAckTransmit(pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID, 
                        pCtrItem->pPktBuff, sizeof(OAM_FILETX_ACKHD_S), pCtrItem->bSessId);

        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */"Send read start ack to onu%d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));

    }
}


/*================================================*/
/*name: funOamTransFileSendRdDataAck              */
/*para: pCtrItem  消息缓冲区                      */
/*para: pAckHead  应答报文头                      */ 
/*retu: NONE                                      */
/*desc: 本地文件读操作, 发送报文数据应答          */
/*================================================*/
static void funOamTransFileSendRdDataAck(OAM_FILETX_CTRL_S *pCtrItem, long lOffSet, long lDataLen) 
{
    OAM_FILETX_ACKHD_S   *pAckHead;

    pAckHead = (OAM_FILETX_ACKHD_S*)pCtrItem->pPktBuff;

    pAckHead->lFileOff = lOffSet;
    pAckHead->lDataLen = lDataLen;   
    pAckHead->lDataFcs = 0;
    pAckHead->sErrCode = 0;
    pAckHead->lFileLen = pCtrItem->lFileLen;
    
    pAckHead->sAckCode = OAM_FILETX_ACKCODE_TRRUN;

    pCtrItem->lTimeOut = OAM_FILETX_RETRY_NO;    /* 数据应答不重发*/

#if 0	

   /*******add by luj 2006/10/23 only for debug *****强制发送错误ACK******/
        if(TEST_MODE == 1)
        	{
				if(SEND_WRONG_DATA_ACK==1)
			 	{

					pAckHead->lFileOff = 0xffff;
				    pAckHead->lDataLen = 0xffff;
			 	}

	 		}
		else if(TEST_MODE == 2)
			{
				if(SEND_WRONG_DATA_ACK==1)
			 	{

					pAckHead->lFileOff = 0xffff;
				    pAckHead->lDataLen = 0xffff;
			 	}

			 	SEND_WRONG_DATA_ACK = 0;
				TEST_MODE = 0;
	 		}
   /****************************************************/
 #endif

    FileAckTransmit(pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID, 
                    pCtrItem->pPktBuff, sizeof(OAM_FILETX_ACKHD_S), pCtrItem->bSessId);

	OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Send read data ack to onu %d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
}

/*================================================*/
/*name: funOamTransFileRecvRdData                 */
/*para: pCtrItem  消息缓冲区                      */
/*para: pAckHead  应答报文头                      */ 
/*retu: NONE                                      */
/*desc: 本地读请求情况下, 收到数据应答            */
/*================================================*/
static void funOamTransFileRecvRdData(OAM_FILETX_CTRL_S *pCtrItem, char *pDataPkt) 
{
/*
  unsigned long i;
  unsigned long dataAddr;
*/
    OAM_FILETX_PKTHD_S *pDatHead = (OAM_FILETX_PKTHD_S*)pDataPkt;

	OAM_FILETX_DEBUG_1((LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, 
                      "Receive read data pkt Offset = %d, len = %d \r\n", pDatHead->lFileOff, pDatHead->lDataLen));

    if((pDatHead->lFileOff < 0) || (pDatHead->lDataLen <= 0))  /* 报文长度不允许为 0 */
    {
        pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_LONOMACHT;

        funOamTransFileSendErrAck(pCtrItem);

        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Receive data pkt with datalen <= 0 from onu%d/%d \r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));

        return;
    }
                                                               /* 在此没有判断文件的长度字段 */
    if(pDatHead->lFileOff == pCtrItem->lOffset)
    {
        long  lTmpLen = pDatHead->lFileOff + pDatHead->lDataLen;

        if(lTmpLen <= pCtrItem->lFileLen)
        {
			VOS_MemCpy(pCtrItem->pFileBuf + pCtrItem->lOffset, 
                       pDataPkt + OAM_FILETX_PKTDATA_HEAD_LEN, pDatHead->lDataLen);
         /*         modify by luj 2006/10/23 */
         /*         	dataAddr = sizeof(OAM_FILETX_PKTHD_S) - OAM_FILETX_PKTDAT_LEN;
		                                                      这种修改方式是错误的
			VOS_MemCpy(pCtrItem->pFileBuf + pCtrItem->lOffset, 
                       pDataPkt + dataAddr, pDatHead->lDataLen);*/
		
            
            pCtrItem->lOffset += pDatHead->lDataLen;

            if(lTmpLen == pCtrItem->lFileLen) 
            {
                pCtrItem->lRepCoun = 0;
                pCtrItem->lSendLen = pDatHead->lDataLen;
                pCtrItem->lStateMc = OAM_FILETX_SMC_LR_ENDACK;
                funOamTransFileSendEndAck(pCtrItem);
            }
            else 
            {
                if(pCtrItem->lStateMc == OAM_FILETX_SMC_LR_REMSTA) 
                {
                    pCtrItem->lStateMc = OAM_FILETX_SMC_LR_REMDAT; 
                }
                funOamTransFileSendRdDataAck(pCtrItem, pDatHead->lFileOff, pDatHead->lDataLen);
            }
        }
        else  
        {
            pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_LONOMACHT;

            funOamTransFileSendErrAck(pCtrItem);

            OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Receive data pkt length error(overflow filelen) from onu%d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
        }
    }
    else if(pDatHead->lFileOff < pCtrItem->lOffset)
    {
        if((pDatHead->lFileOff + pDatHead->lDataLen) == pCtrItem->lOffset)
        {
            funOamTransFileSendRdDataAck(pCtrItem, pDatHead->lFileOff, pDatHead->lDataLen);
        }
        else
        {
            pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_LONOMACHT;

            funOamTransFileSendErrAck(pCtrItem);

            OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Receive data resend pkt with offset or datalen match error from onu%d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
        }
    }
    else 
    {                                            /* 如果偏移错误 终止流程*/
        pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_LONOMACHT;

        funOamTransFileSendErrAck(pCtrItem);

        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Receive data pkt with offset or datalen match error from onu%d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
    }
}


/*=================================================================================================================== */
/* 通用接收处理过程函数  */
/*================================================*/
/*name: funOamTransFileRecvErrAck                 */
/*para: pCtrItem                                  */
/*para: pAckHead                                  */
/*retu: none                                      */
/*desc: 读写流程控制中, 本地收到对端错误ACK处理   */
/*================================================*/

static void funOamTransFileRecvErrAck(OAM_FILETX_CTRL_S *pCtrItem, OAM_FILETX_ACKHD_S *pAckHead)
{                                                 /* 直接终止该传送实例 */
     OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/
                       "0 Receive error Ack(code=300) from onu%d/%d, errno = %d\r\n", pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID, pAckHead->sErrCode));

     if((pAckHead->sErrCode > 0) && (pAckHead->sErrCode <= OAM_FILETX_ERRCODE_SAVEERR))
     {
          pCtrItem->lErrorNo = pAckHead->sErrCode;
     }
     else pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_TRANERR;

     funOamTransFileEndInstance(pCtrItem);
}

/*================================================*/
/*name: funOamTransFileRecvReqAck                 */
/*para: pCtrItem  实例项                          */
/*para: pAckHead  应答报文头                      */ 
/*retu: NONE                                      */
/*desc: 本地读写请求, 收到请求应答                */
/*================================================*/

static void funOamTransFileRecvReqAck(OAM_FILETX_CTRL_S *pCtrItem, OAM_FILETX_ACKHD_S *pAckHead)
{
    OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */
                     "Receive ReqAck, code = %d, flen = %d from onu%d/%d\r\n", pAckHead->sAckCode, pAckHead->lFileLen,pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
    
    switch(pAckHead->sAckCode)
    {
        /*
        case OAM_FILETX_ACKCODE_TRERR:
                                                          
             funOamTransFileRecvErrAck(pCtrItem, pAckHead);
             break;
        */
        case OAM_FILETX_ACKCODE_WRACC:

             if(pCtrItem->sReqInfo.lReqType == OAM_FILETX_REQ_WRITE)
             {
                 pCtrItem->lRepCoun = 0;
                 pCtrItem->lStateMc = OAM_FILETX_SMC_LW_DATACK;
                 pCtrItem->lSendLen = OAM_FILETX_PKTDAT_LEN;  
                    
                 if((pCtrItem->lSendLen + pCtrItem->lOffset) > pCtrItem->lFileLen)  
                 {
                     pCtrItem->lSendLen = pCtrItem->lFileLen - pCtrItem->lOffset;  
                 }
                                                  /* 判断对端允许的文件最大长度 */
		#if 0
		  第一个数据帧时无需判断长度，因为未给对端发送长度del by luj 2006/10/21
                 if(pAckHead->lFileLen < pCtrItem->lFileLen) pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_TOOLONG;
		#endif

                 if(pCtrItem->lErrorNo != OAM_FILETX_ERRCODE_SUCCESS)
                      funOamTransFileSendErrAck(pCtrItem);
                 else funOamTransFileSendWrData(pCtrItem);
             }
             else
             {
                 pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_FLOWERR;

                 funOamTransFileSendErrAck(pCtrItem);

                 OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */
                                   "1 Receive ack with flow control error ackcode = %d from onu%d/%d\r\n", pAckHead->sAckCode, pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
             }
             
             break;

        case OAM_FILETX_ACKCODE_WRREJ:

             if(pCtrItem->sReqInfo.lReqType == OAM_FILETX_REQ_WRITE)
             {
                 pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_REQREJ;

                 funOamTransFileEndInstance(pCtrItem);

                 OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */"Write file is rejected by remote onu %d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
             }
             else
             {
                 pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_FLOWERR;

                 funOamTransFileSendErrAck(pCtrItem);

                 OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */
                                   "2 Receive ack with flow control error ackcode = %d from onu%d/%d\r\n", pAckHead->sAckCode, pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
             }

             break;

        case OAM_FILETX_ACKCODE_RDACC:

             if(pCtrItem->sReqInfo.lReqType == OAM_FILETX_REQ_READ)
             {
                 pCtrItem->lRepCoun = 0;
                 pCtrItem->lOffset  = 0;
                 pCtrItem->lFileLen = pAckHead->lFileLen; 
                 pCtrItem->lStateMc = OAM_FILETX_SMC_LR_REMSTA;
                 pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_SUCCESS;

                 if(pCtrItem->lFileLen <= 0)                pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_TOOSHORT;
                 if(pCtrItem->lFileLen > pCtrItem->lMaxLen) pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_TOOLONG;

                 if(pCtrItem->lErrorNo != OAM_FILETX_ERRCODE_SUCCESS)
                 {
                     funOamTransFileSendErrAck(pCtrItem);
                 }
                 else
                 {
                     pCtrItem->pFileBuf = (char*)g_malloc(pCtrItem->lFileLen + 1024);
                     pCtrItem->pPktBuff = (char*)VOS_Malloc(OAM_FILETX_PKTBUF_LEN, MODULE_RPU_TRANSFILE);

                     if((pCtrItem->pFileBuf == NULL) || (pCtrItem->pPktBuff == NULL))
                     {
                         pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_NORES;
                         funOamTransFileSendErrAck(pCtrItem);
                     }
                     else funOamTransFileSendRdStartAck(pCtrItem);
                 }
             }
             else
             {
                 pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_FLOWERR;

                 funOamTransFileSendErrAck(pCtrItem);
                 OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */
                                   "3 Receive ack with flow control error ackcode = %d from onu%d/%d\r\n", pAckHead->sAckCode,pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
             }

             break;

        case OAM_FILETX_ACKCODE_RDREJ:

             if(pCtrItem->sReqInfo.lReqType == OAM_FILETX_REQ_READ)
             {
                 pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_REQREJ;
                 funOamTransFileEndInstance(pCtrItem);
                 OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */"Read file is rejected by remote onu %d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
             }
             else
             {
                 pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_FLOWERR;

                 funOamTransFileSendErrAck(pCtrItem);
                 OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/ 
                                   "4 Receive ack with flow control error ackcode = %d from onu %d/%d\r\n", pAckHead->sAckCode, pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
             }
             break;
	     /* 
           added by chenfj 2008-3-19
           用于等待读写文件请求时控制层消息容错处理
           在等待读写文件应答时, 增加对传送开始,传送中,传送结束三个事件的处理
          */
	  case OAM_FILETX_ACKCODE_TRSTA:
	  case OAM_FILETX_ACKCODE_TRRUN:
	  case OAM_FILETX_ACKCODE_TREND:
	  	if( glTransFileDebug )
	  	sys_console_printf("file:%s, line:%d, AckCode=%x from onu%d/%d\r\n",__FILE__, __LINE__, pAckHead->sAckCode, pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID );
	  	break;

        default:
                 
             pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_FLOWERR;

             funOamTransFileSendErrAck(pCtrItem);
             OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */
                              "5 Receive ack with flow control error ackcode = %d from onu %d/%d\r\n", pAckHead->sAckCode, pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
             break;
    }
}  

/*================================================*/
/*name: funOamTransFileRecvEndAck                 */
/*para: pCtrItem  消息缓冲区                      */
/*para: pAckHead  应答报文头                      */ 
/*retu: NONE                                      */
/*desc: 本地写请求情况下, 收到结束应答            */
/*================================================*/

static void funOamTransFileRecvEndAck(OAM_FILETX_CTRL_S  *pCtrItem, OAM_FILETX_ACKHD_S *pAckHead)
{
    OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Receive end ack from onu%d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));

    switch(pAckHead->sAckCode)
    {
        /*
        case OAM_FILETX_ACKCODE_TRERR:

             funOamTransFileRecvErrAck(pCtrItem, pAckHead);
             break;
        */
        case OAM_FILETX_ACKCODE_TREND:
 
             if((pAckHead->lFileLen == pCtrItem->lFileLen)) /* && (pAckHead->lFileOff == pCtrItem->lOffSet)) */
             {
                 pCtrItem->lStateMc = OAM_FILETX_SMC_LO_SUCESS;
                 pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_SUCCESS;
             }     
             else
             {
                 pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_ENDACKERR;

                 OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */"8 Receive end ack with some error from onu %d/%d\r\n", pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
             }
          
             funOamTransFileEndInstance(pCtrItem);

             break;

      /*
        added by chenfj 2008-3-19
        用于传输层容错控制处理
        在等待文件传输结束应答时,若收到其他消息, 忽略之,不做处理
        */
        case OAM_FILETX_ACKCODE_RDACC:
	  case OAM_FILETX_ACKCODE_WRACC:
	  case OAM_FILETX_ACKCODE_TRSTA:
	  case OAM_FILETX_ACKCODE_TRRUN:
	  	if(glTransFileDebug)
	  	sys_console_printf("file:%s, line:%d, AckCode=%x from onu%d/%d\r\n",__FILE__, __LINE__, pAckHead->sAckCode, pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID );
	  	break;

	  case OAM_FILETX_ACKCODE_RDREJ:
	  case OAM_FILETX_ACKCODE_WRREJ:
	  	if( glTransFileDebug )
	  	sys_console_printf("file:%s, line:%d, AckCode=%x from onu%d/%d\r\n",__FILE__, __LINE__, pAckHead->sAckCode, pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID );
        default:

             pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_FLOWERR;

             funOamTransFileSendErrAck(pCtrItem);

             OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */
                              "9 Receive ack with flow control error ackcode = %d from onu%d/%d\r\n", pAckHead->sAckCode, pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
             break;
    }
}

/*===================================================================================================================*/
/*===================================================================================================================*/
/*================================================*/
/*name: funOamTransFileProcWrRecvAckPkt           */
/*para: pCtrItem                                  */
/*para: pAckHead                                  */
/*retu: none                                      */
/*desc: 写文件流程收到对端ACK报文处理             */
/*================================================*/

static void funOamTransFileProcWrRecvAckPkt(OAM_FILETX_CTRL_S *pCtrItem, OAM_FILETX_ACKHD_S *pAckHead)
{
    if(pAckHead->sAckCode == OAM_FILETX_ACKCODE_TRERR)  
    {                                            /* 收到错误应答直接退出 */
        funOamTransFileRecvErrAck(pCtrItem, pAckHead);
    }
    else
    {
        switch(pCtrItem->lStateMc)
        {
            case OAM_FILETX_SMC_LW_REQACK:

                 funOamTransFileRecvReqAck(pCtrItem, pAckHead);
                 break;

            case OAM_FILETX_SMC_LW_DATACK:
		#if 0
                 if(ACK_LOCK == 0)  /* ADD BY LUJ 2006/10/23  only used to debug*/
	       #endif 
                 funOamTransFileRecvWrDataAck(pCtrItem, pAckHead);
                 break;

            case OAM_FILETX_SMC_LW_ENDACK:
                 
                 funOamTransFileRecvEndAck(pCtrItem, pAckHead);
                 break;

            default: break;
        }
    }
}


/*================================================*/
/*name: funOamTransFileProcRdRecvAckPkt           */
/*para: pCtrItem                                  */
/*para: pAckHead                                  */
/*retu: none                                      */
/*desc: 读文件流程收到对端ACK报文处理函数         */
/*================================================*/
/*????*/
static void funOamTransFileProcRdRecvAckPkt(OAM_FILETX_CTRL_S *pCtrItem, OAM_FILETX_ACKHD_S *pAckHead)
{
    if(pAckHead->sAckCode == OAM_FILETX_ACKCODE_TRERR)
    {
        funOamTransFileRecvErrAck(pCtrItem, pAckHead);
    }
    else
    {
        switch(pCtrItem->lStateMc)
        {
            case OAM_FILETX_SMC_LR_REQACK:

                 funOamTransFileRecvReqAck(pCtrItem, pAckHead);
                 break;

            case OAM_FILETX_SMC_LR_REMSTA:       /* 该状态下不能接收ACK */
            case OAM_FILETX_SMC_LR_REMDAT:
                
                 pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_FLOWERR;

                 funOamTransFileSendErrAck(pCtrItem);

                 OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/ "RD:Must receive data when LR_REMSTA or LR_REMDAT state, onu%d/%d\r\n", pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));

                 break;

            case OAM_FILETX_SMC_LR_ENDACK:
                 
                 funOamTransFileRecvEndAck(pCtrItem, pAckHead);
                 break;

            default: break;
        }
    } 
}
 

/*================================================*/
/*name: funOamTransFileProcRdRecvDataPkt          */
/*para: pCtrItem                                  */
/*para: pAckHead                                  */
/*retu: none                                      */
/*desc: 读文件流程收到对端数据报文处理函数        */
/*================================================*/
/*????*/
static void funOamTransFileProcRdRecvDataPkt(OAM_FILETX_CTRL_S *pCtrItem, char *pDataPkt)
{
    switch(pCtrItem->lStateMc)
    {
        case OAM_FILETX_SMC_LR_REQACK:

             pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_FLOWERR;

             funOamTransFileSendErrAck(pCtrItem);

             OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"RD:Must receive ACK when LR_REQACK state,onu%d/%d\r\n",pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));
             break;

        case OAM_FILETX_SMC_LR_REMSTA:        
        case OAM_FILETX_SMC_LR_REMDAT:
            
             funOamTransFileRecvRdData(pCtrItem, pDataPkt);
             break;

        case OAM_FILETX_SMC_LR_ENDACK:  /* 该状态下收到报文直接回复ENDACK*/
             
             funOamTransFileSendEndAck(pCtrItem);
             break;

        default: break;
    }
}

/*================================================*/
/*name: funOamTransFileSendPktRetry               */
/*para: pCtrItem                                  */
/*retu: none                                      */
/*desc: 报文重发                                  */
/*================================================*/
/*
extern int CommOltMsgRvcDebugTurnOn(unsigned char GwProId);
extern int CommOltMsgRvcDebugTurnOff(unsigned char GwProId);
extern int  GetSysCurrentTime( Date_S *CurrentTime);
*/
static void funOamTransFileSendPktRetry(OAM_FILETX_CTRL_S *pCtrItem) 
{
/*Date_S CurrentTime;*/
    if(pCtrItem->sReqInfo.lReqType == OAM_FILETX_REQ_WRITE)
    {
        switch(pCtrItem->lStateMc)
        {
            case OAM_FILETX_SMC_LW_REQACK:
                 funOamTransFileSendReq(pCtrItem);
                 break;
            case OAM_FILETX_SMC_LW_DATACK:
			/*
		    if((pCtrItem->lRepCoun+1) > OAM_FILETX_RETRY_COU)
		    	{
		    	
			CommOltMsgRvcDebugTurnOff(GW_DEBUG_FILE);
			SetOamCommFlag(V2R1_DISABLE );
			glTransFileDebug = 0;
		    	}
			else 
				{				
				CommOltMsgRvcDebugTurnOn(GW_DEBUG_FILE);
				SetOamCommFlag(V2R1_ENABLE );
				glTransFileDebug = 1;
				}
			
			GetSysCurrentTime(&CurrentTime);
			sys_console_printf("\r\ncurrent time:%d.%d.%d  %d.%d.%d\r\n",CurrentTime.year,CurrentTime.month, CurrentTime.day,CurrentTime.hour,CurrentTime.minute,CurrentTime.second);
			sys_console_printf("onu%d/%d---reTry:%d, offset:%d\r\n",pCtrItem->sReqInfo.usPonID, pCtrItem->sReqInfo.usOnuID,
																			pCtrItem->lRepCoun, pCtrItem->lOffset );
			*/
                 funOamTransFileSendWrData(pCtrItem);
                 break;
            case OAM_FILETX_SMC_LW_ENDACK:
                 funOamTransFileSendEndAck(pCtrItem);
                 break;
            default: break;
        }
    }
    else
    {
        switch(pCtrItem->lStateMc)
        {
            case OAM_FILETX_SMC_LR_REQACK:
                 funOamTransFileSendReq(pCtrItem);
                 break;
            case OAM_FILETX_SMC_LR_REMSTA:  
                 funOamTransFileSendRdStartAck(pCtrItem);
                 break;
            case OAM_FILETX_SMC_LR_REMDAT:
                 break;
            case OAM_FILETX_SMC_LR_ENDACK:   
                 funOamTransFileSendEndAck(pCtrItem);
                 break;
            default: break;
        }
    }
}


/*=================================================================================================================== */
/*=================================================================================================================== */
/* 任务队列接收消息处理 */
/*================================================*/
/*name: funOamTransFileProcAppReqMsg              */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 任务队列接收写或读文件请求消息处理        */
/*================================================*/

static void funOamTransFileProcAppReqMsg(SYS_MSG_S *pMsgBuf)
{
    long               lErrorNo  = OAM_FILETX_ERRCODE_SUCCESS; 
    char              *pTmpFile  = NULL;
    char              *pTmpPktB  = NULL;
    OAM_FILETX_REQ_S  *pReqInfor = NULL;
    OAM_FILETX_CTRL_S *pCtrItem  = NULL;

 
    pReqInfor = (OAM_FILETX_REQ_S*)(pMsgBuf + 1);

    pCtrItem  = funOamTransFileFindInstance(pReqInfor->usPonID, pReqInfor->usOnuID);

    if(pCtrItem != NULL)               /* 该ONU已经存在一个传输实例 */
    {
        lErrorNo = OAM_FILETX_ERRCODE_HAVEINS;
        /*goto EponAppReqProcError;*/
	 funOamTransFileEndInstance( pCtrItem );		/* modified by xieshl 20100722, pCtrItem不能直接释放，会破坏链表导致任务异常 */
	 return;
    }
    
    pCtrItem = (OAM_FILETX_CTRL_S*)VOS_Malloc(sizeof(OAM_FILETX_CTRL_S), MODULE_RPU_TRANSFILE);
    if(pCtrItem == NULL)
    {
        lErrorNo = OAM_FILETX_ERRCODE_NOMEM;
        goto EponAppReqProcError;
    }

    VOS_MemZero(pCtrItem, sizeof(OAM_FILETX_CTRL_S));
         
    VOS_MemCpy(&pCtrItem->sReqInfo, pReqInfor, sizeof(OAM_FILETX_REQ_S));

    if(pReqInfor->lReqType == OAM_FILETX_REQ_WRITE)
    {
        if(get_ONU_Info(pReqInfor->bFileType, NULL, (int*)&pCtrItem->lMaxLen, NULL, NULL, NULL) != VOS_OK) 
        {
            lErrorNo = OAM_FILETX_ERRCODE_READFILE;
            goto EponAppReqProcError;
        }

        if(pCtrItem->lMaxLen > (OAM_FILETX_FILELEN_MAX - 1024))
        {
            lErrorNo = OAM_FILETX_ERRCODE_TOOLONG;
            goto EponAppReqProcError;
        }
        if(pCtrItem->lMaxLen < 1) 
        {
            lErrorNo = OAM_FILETX_ERRCODE_TOOSHORT;
            goto EponAppReqProcError;
        }
       
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"WR:Get file len = %d, onu%d/%d\r\n", pCtrItem->lMaxLen, pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));

        pCtrItem->lMaxLen += 1024;

        pTmpFile = (char*)g_malloc(pCtrItem->lMaxLen);
        pTmpPktB = (char*)VOS_Malloc(OAM_FILETX_PKTBUF_LEN, MODULE_RPU_TRANSFILE);

        if((pTmpFile == NULL) || (pTmpPktB == NULL))
        {
            lErrorNo = OAM_FILETX_ERRCODE_NOMEM;
            goto EponAppReqProcError;
        }

        pCtrItem->lFileLen = pCtrItem->lMaxLen;

        if(get_ONU_file(pReqInfor->bFileType, pTmpFile, (int*)&pCtrItem->lFileLen, NULL) != VOS_OK)
        {
            lErrorNo = OAM_FILETX_ERRCODE_READFILE;
            goto EponAppReqProcError;
        }      

        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */"WR:Read file data len = %d,onu%d/%d\r\n", pCtrItem->lFileLen, pCtrItem->sReqInfo.usPonID,   pCtrItem->sReqInfo.usOnuID));

        if((pCtrItem->lFileLen < 1) || (pCtrItem->lFileLen > pCtrItem->lMaxLen))
        {
            lErrorNo = OAM_FILETX_ERRCODE_READFILE;
            goto EponAppReqProcError;
        }
    }
    else 
	{

		pCtrItem->lMaxLen = OAM_FILETX_FILELEN_MAX - 1024;
		/*****add by luj 2006/10/22**pkB为空OAM 不发送************/
		pTmpPktB = (char*)VOS_Malloc(OAM_FILETX_PKTBUF_LEN, MODULE_RPU_TRANSFILE);

	        if((pTmpPktB == NULL))
	        {
	            lErrorNo = OAM_FILETX_ERRCODE_NOMEM;
	            goto EponAppReqProcError;
	        }
		/************************************************************/

   	}
     
    pCtrItem->lOffset   = 0;      
    pCtrItem->lRepCoun  = 0;       
    pCtrItem->lTotTout  = 0; 
    pCtrItem->pFileBuf  = pTmpFile;
    pCtrItem->pPktBuff  = pTmpPktB;
    pCtrItem->lPrintTime= OAM_FILETX_RATETIME + 2;
    pCtrItem->lErrorNo  = OAM_FILETX_ERRCODE_SUCCESS;     
    pCtrItem->lStateMc  = OAM_FILETX_SMC_LW_REQACK; /* OAM_FILETX_SMC_LR_REQACK */

	/***add by luj 2006/10/21 print onu file content*****
	printf("\r\n------ pCtrItem read from flash---fileLen = %4x\r\n",pCtrItem->lFileLen);
		for(iLoop=0;iLoop<pCtrItem->lFileLen;iLoop++)
		{
			printf("%2x ",pCtrItem->pFileBuf[iLoop]);
		}
		printf("\r\n \r\n");
		printf("pCtrItem->pPktBuff  = %x\r\n",pCtrItem->pPktBuff );

	************************************************/
    
    *((long*)pCtrItem->bSessId) = ++glTransSessionId;

    list_add_tail(&pCtrItem->sLinkList, &gsTransFileLink);

    funOamTransFileSendReq(pCtrItem);

    return;

EponAppReqProcError:

    if(pTmpFile != NULL) g_free(pTmpFile);
    if(pTmpPktB != NULL) VOS_Free(pTmpPktB);
    if(pCtrItem != NULL) VOS_Free(pCtrItem);

    funOamTransFileAppReqAck(pReqInfor, 0, lErrorNo);

    return;
}


/*================================================*/
/*name: funOamTransFileProcRecvAckMsg             */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 任务队列接收到远端的ACK报文消息处理       */
/*================================================*/

static void funOamTransFileProcRecvAckMsg(SYS_MSG_S *pMsgBuf)
{
    unsigned long       ulSenId;
    unsigned short      usDatLen;
    unsigned char      *pcDatBuf;    
    unsigned short      usPonId, usOnuId;
    OAM_FILETX_CTRL_S  *pCtrItem = NULL;


    usDatLen =  pMsgBuf->usFrameLen; 

    if(usDatLen < sizeof(OAM_FILETX_ACKHD_S))       /* 对于长度错误报文不处理直接丢弃*/
    {
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */
                         "Receive ack packet with frame length < ack head length = %d\r\n", usDatLen));
        return;   
    }

    usPonId  = (pMsgBuf->ulSrcSlotID >> 16);
    usOnuId  = (pMsgBuf->ulSrcSlotID & 0x0000ffff);
    ulSenId  =  pMsgBuf->ulDstSlotID;
    pcDatBuf =  pMsgBuf->ptrMsgBody; 

    pCtrItem = funOamTransFileMatchInstance(usPonId, usOnuId, ulSenId);
                                                        
    if(pCtrItem == NULL) 
    {
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */
                         "Match transfer instance error ponid = %d onuid = %d\r\n", usPonId, usOnuId));
        return;
    }

	/***add by luj 2006/10/24 only for debug*****
		printf(" %d;",pCtrItem->lRemIdle);
	****************************************/

    pCtrItem->lRemIdle = 0;                      /* 接收超时清零 */
	
	/***add by luj 2006/10/24 only for debug*****
		printf(" %d,",pCtrItem->lRemIdle);
	****************************************/

    if(pCtrItem->sReqInfo.lReqType == OAM_FILETX_REQ_WRITE)
    {
        funOamTransFileProcWrRecvAckPkt(pCtrItem, (OAM_FILETX_ACKHD_S*)pcDatBuf);
    }
    else
    {
        funOamTransFileProcRdRecvAckPkt(pCtrItem, (OAM_FILETX_ACKHD_S*)pcDatBuf);
    }
}


/*================================================*/
/*name: funOamTransFileProcRecvDataMsg            */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 接收的DATA 报文消息处理函数                 */
/*================================================*/
static void funOamTransFileProcRecvDataMsg(SYS_MSG_S *pMsgBuf)
{
    unsigned long       ulSenId;
    unsigned short      usDatLen;
    unsigned char      *pcDatBuf;    
    unsigned short      usPonId, usOnuId;
    OAM_FILETX_CTRL_S  *pCtrItem = NULL;
    OAM_FILETX_PKTHD_S *pDatHead = NULL;
 /*   unsigned long i;*/
	
    pcDatBuf =  pMsgBuf->ptrMsgBody; 

    pDatHead = (OAM_FILETX_PKTHD_S*)pcDatBuf;
    usDatLen =  pMsgBuf->usFrameLen; 

  /* if((usDatLen < sizeof(OAM_FILETX_PKTHD_S)) ||  
       (usDatLen < (pDatHead->lDataLen +  sizeof(OAM_FILETX_PKTHD_S)))) 
       modify by luj 2006/10/25 sizeof(OAM_FILETX_PKTHD_S)中含DatHead->lDataLen*/
      if((usDatLen > sizeof(OAM_FILETX_PKTHD_S)) || (usDatLen <OAM_FILETX_PKTDATA_HEAD_LEN))
    {
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */
                         "RD:Receive data packet with length error data len = %d framelen = %d\r\n",
                         pDatHead->lDataLen, usDatLen));
        return;   
    }  

    usPonId  = (pMsgBuf->ulSrcSlotID >> 16);
    usOnuId  = (pMsgBuf->ulSrcSlotID & 0x0000ffff);
    ulSenId  =  pMsgBuf->ulDstSlotID;
    pcDatBuf =  pMsgBuf->ptrMsgBody; 

	/******add by luj 2006/10/23***only for debug***********
	printf("\r\n%%%%%%receive pMsgBody%%%%%\r\n");
	for(i=0;i<27;i++)
		printf("%2x",pcDatBuf[i]);
       printf("\r\n%%%%%%%%%%%%%%%%%%\r\n");
     *******************************************************/
    pCtrItem = funOamTransFileMatchInstance(usPonId, usOnuId, ulSenId);
                                                        
    if(pCtrItem == NULL) 
    {
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */
                         "RD:Match transfer instance error ponid = %d onuid = %d\r\n", usPonId, usOnuId));
        return;
    }

    pCtrItem->lRemIdle = 0;                      /* 接收超时清零 */

    if(pCtrItem->sReqInfo.lReqType == OAM_FILETX_REQ_WRITE)
    {
        pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_FLOWERR;

        funOamTransFileSendErrAck(pCtrItem);
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT, */"RD:Receive data pkt when write file to remote\r\n"));
    }
    else
    {
        funOamTransFileProcRdRecvDataPkt(pCtrItem, pcDatBuf);
    }
}


/*================================================*/
/*name: funOamTransFileProcTimerMsg               */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 接收的ACK报文消息处理函数                 */
/*================================================*/
static void funOamTransFileProcTimerMsg(SYS_MSG_S *pMsgBuf)
{
    struct list_head       *pListhead = &gsTransFileLink;
    struct list_head       *pListNext = NULL;
    struct list_head       *pListCurr = NULL;
    OAM_FILETX_CTRL_S      *pCtrlItem = NULL;
    OAM_FILETX_REQ_S       *pReqInfo;             /* 高层请求信息         */
	
    if(list_empty(pListhead)) return;

    pListCurr = pListhead->next;

    while(pListCurr != pListhead)
    {
        pListNext = pListCurr->next;             /* 先保存下一个, 避免但前删除而出现错误 */ 
        
        pCtrlItem = list_entry(pListCurr, OAM_FILETX_CTRL_S, sLinkList);

        pCtrlItem->lTotTout ++;
                                                 /* 总超时已到 */
        if(pCtrlItem->lTotTout > OAM_FILETX_TOTAL_TOUT)
        {
            pCtrlItem->lErrorNo = OAM_FILETX_ERRCODE_FTIMEOUT;
            funOamTransFileEndInstance(pCtrlItem);

            OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/ "Write or read file timeout,onu%d/%d\r\n", pCtrlItem->sReqInfo.usPonID,   pCtrlItem->sReqInfo.usOnuID));
        }
        else
        {
           pCtrlItem->lRemIdle ++;  
                                                 /* 未收到报文超时 */
            if(pCtrlItem->lRemIdle > OAM_FILETX_IDLE_TOUT)
            {
                pCtrlItem->lErrorNo = OAM_FILETX_ERRCODE_PTIMEOUT;
                 funOamTransFileEndInstance(pCtrlItem); 
			   
		/****add  by luj 2006/10/24 only for debug ************
			   printf("%d !  ",pCtrlItem->lRemIdle);
		************************************************/

                OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"Receive packet timeout,onu%d/%d\r\n", pCtrlItem->sReqInfo.usPonID,   pCtrlItem->sReqInfo.usOnuID));
		
            }
            else
            {
                if(pCtrlItem->lTimeOut > 0)
                {
                    pCtrlItem->lTimeOut --;            /* 重发超时      */

                    if(pCtrlItem->lTimeOut == 0)
                    {
						dubleTime ++;
						funOamTransFileSendPktRetry(pCtrlItem);
                    }
                }

				if(pCtrlItem->lPrintTime > 0)
				{
                    pCtrlItem->lPrintTime --;            /* 重发超时      */

                    if(pCtrlItem->lPrintTime == 0)
                    {
                        pReqInfo = &pCtrlItem->sReqInfo;
						
						pCtrlItem->lPrintTime = OAM_FILETX_RATETIME;

                        if(pReqInfo->pReqCallBack != NULL) 
                        {
                           pReqInfo->pReqCallBack(pReqInfo->lReqType,  pReqInfo->usPonID,   pReqInfo->usOnuID,
						   	                      pReqInfo->bFileName, pReqInfo->pArgVoid, pCtrlItem->lFileLen, pCtrlItem->lOffset, 0);
                        }
                    }
				}
            }
        }

        pListCurr = pListNext;
    }
}



/*===========================================================*/
/*================================================*/
/*name: funOamTransFileProcMessages               */
/*para: pMsgBuf  消息缓冲区                       */
/*retu: none                                      */
/*desc: 任务的输入消息处理主函数                  */
/*================================================*/

static void funOamTransFileProcMessages(unsigned char *pMsgBuf)
{
    SYS_MSG_S *pSysMsg = (SYS_MSG_S *)pMsgBuf;
    
    switch(pSysMsg->usMsgCode)   
    {
        case OAM_FILETX_MSGCODE_TRREQ:
			
			/*****add by luj 2006/10/22 统计接收的消息数**only for debug*****************
			glReceiveMesCount=glReceiveMesCount+1;
			 glReceiveMesTRREQ = glReceiveMesTRREQ+1; 

			*********************************************************************/

			 funOamTransFileProcAppReqMsg(pSysMsg);
			 
             break;

        case OAM_FILETX_MSGCODE_RWREQ:
             /* NO SUPPORT */
             break;

        case OAM_FILETX_MSGCODE_TRDAT:
			
			/*****add by luj 2006/10/22 统计接收的消息数******only for debug*************
			glReceiveMesCount=glReceiveMesCount+1;
			glReceiveMesDATA = glReceiveMesDATA+1; 
			*********************************************************************/
	
             funOamTransFileProcRecvDataMsg(pSysMsg);
             break;

        case OAM_FILETX_MSGCODE_TRACK:

			/*****add by luj 2006/10/22 统计接收的消息数**************only for debug******
			glReceiveMesCount=glReceiveMesCount+1;
			glReceiveMesACK = glReceiveMesACK+1; 
			*********************************************************************/

             funOamTransFileProcRecvAckMsg(pSysMsg);
             break;

        case OAM_FILETX_MSGCODE_TIMER:
             funOamTransFileProcTimerMsg(pSysMsg);
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
/*name: funEponOamTransFileTask                   */
/*retu: none                                      */
/*desc: 任务入口                                  */
/*================================================*/
/* modified by xieshl 20100722,增加ONU删除处理过程 */
static void funOnuDelProcMessages( ULONG msg );
DECLARE_VOS_TASK(funEponOamTransFileTask)
{
	unsigned long ulMessage[4] = {0};

	while(1)
	{
		if(VOS_QueReceive(gulTransFileQid, ulMessage, WAIT_FOREVER) == VOS_ERROR)
		{
			OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/ "Recv Message error\r\n"));
			VOS_TaskDelay(20);
			continue;
		}

		if( ulMessage[1] == MSG_ONU_DELETE_EVENT )
		{
			funOnuDelProcMessages(ulMessage[3]);
		}
		else /*if( ulMessage[1] == MSG_OAM_FILE_TX )*/
		{
			if(ulMessage[3] != 0)
			{
				funOamTransFileProcMessages((char*)ulMessage[3]);
			}
			else
			{
				OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/
							"Recv Message with on any data para in ulMessage[3]\r\n"));
			}
		}
	}
}

/*=================================================================================================================== */
/*=================================================================================================================== */
/* 初始化部分 */
/*================================================*/
/*name: funOamTransFileAllCallBack                */
/*para: usMsgCode                                 */
/*para: usPonId                                   */ 
/*para: usOnuId                                   */ 
/*para: lDatLen  数据长度                         */
/*para: pDatBuf  数据指针                         */
/*para: pcSessId 会话ID                           */
/*retu: none                                      */
/*desc: 文件传送接收的所有OAM帧回调处理           */
/*================================================*/
static void funOamTransFileAllCallBack(unsigned short usMsgCode,  unsigned short usPonId, 
                                       unsigned short usOnuId,    unsigned short usDatLen, 
                                       unsigned char *pcDatBuf,   unsigned char* pcSessId)
{
    SYS_MSG_S     *pQueueMsg    = NULL;
    unsigned long  ulMessage[4] = {MODULE_RPU_TRANSFILE, MSG_OAM_FILE_TX, 0, 0};

    if((pcDatBuf == NULL) || (pcSessId == NULL))
    {
    	  OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/ "Input pcDatBuf=NULL in callback fun, onu%d/%d\r\n",usPonId, usOnuId));
	 if(pcDatBuf != NULL)
    	      VOS_Free(pcDatBuf);
	 if(pcSessId != NULL)
            VOS_Free(pcSessId);
        return;
    }

    if(usDatLen == 0) 
    {
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/ "Input usDatLen=0 in callback fun,onu%d/%d\r\n",usPonId,usOnuId));
        VOS_Free(pcDatBuf);
        VOS_Free(pcSessId);
        return;
    }

    pQueueMsg = (SYS_MSG_S*)VOS_Malloc(sizeof(SYS_MSG_S), MODULE_RPU_TRANSFILE);

    if(pQueueMsg == NULL) 
    {
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/ "Malloc memory faile in callback fun,onu%d/%d\r\n",usPonId, usOnuId));
        VOS_Free(pcDatBuf);
        VOS_Free(pcSessId);
        return;
    }
    
    VOS_MemZero(pQueueMsg, sizeof(SYS_MSG_S));

    pQueueMsg->ucMsgType      = MSG_NOTIFY;
    pQueueMsg->ulSrcSlotID    = (usPonId << 16) | usOnuId;
    pQueueMsg->ulDstSlotID    = *((unsigned long *)pcSessId); /* 暂时使用该字段存session id */
    pQueueMsg->usMsgCode      = usMsgCode;
    pQueueMsg->ucMsgBodyStyle = MSG_BODY_MULTI;
    pQueueMsg->usFrameLen     = usDatLen;
    pQueueMsg->ptrMsgBody     = pcDatBuf;

    ulMessage[3] = (unsigned long)pQueueMsg;

    if(VOS_QueSend(gulTransFileQid, ulMessage, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
    {
        VOS_Free(pcDatBuf);
        VOS_Free(pQueueMsg);
         
        OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/ "Send msg fail in callback,onu%d/%d\r\n",usPonId,usOnuId));
    }

    VOS_Free(pcSessId);
}

/*================================================*/
/*name: funEponOamTransFileRWReqCallBack          */
/*para: usPonId                                   */ 
/*para: usOnuId                                   */ 
/*para: lDatLen  数据长度                         */
/*para: pDatBuf  数据指针                         */
/*para: pcSessId 会话ID                           */
/*retu: none                                      */
/*desc: 文件读或写传送请求回调函数                */
/*================================================*/

static void funOamTransFileRWReqCallBack(unsigned short usPonId,  unsigned short usOnuId,  unsigned short llid,
                                         unsigned short usDatLen, unsigned char *pcDatBuf,
                                         unsigned char *pcSessId)
{
    funOamTransFileAllCallBack(OAM_FILETX_MSGCODE_RWREQ,
                               usPonId, usOnuId, usDatLen, pcDatBuf, pcSessId);
}

/*================================================*/
/*name: funEponOamTransFileTRDatCallBack          */
/*para: usPonId                                   */ 
/*para: usOnuId                                   */ 
/*para: lDatLen  数据长度                         */
/*para: pDatBuf  数据指针                         */
/*para: pcSessId 会话ID                           */
/*retu: none                                      */
/*desc: 数据传送请求回调函数                      */
/*================================================*/
static void funOamTransFileTRDatCallBack(unsigned short usPonId,  unsigned short usOnuId,  unsigned short llid,
                                         unsigned short usDatLen, unsigned char *pcDatBuf,
                                         unsigned char *pcSessId)
{
    funOamTransFileAllCallBack(OAM_FILETX_MSGCODE_TRDAT,
                               usPonId, usOnuId, usDatLen, pcDatBuf, pcSessId);
}

/*================================================*/
/*name: funEponOamTransFileTRAckCallBack          */
/*para: usPonId                                   */ 
/*para: usOnuId                                   */ 
/*para: lDatLen  数据长度                         */
/*para: pDatBuf  数据指针                         */
/*para: pcSessId 会话ID                           */
/*retu: none                                      */
/*desc: 数据传送应答回调函数                      */
/*================================================*/
static void funOamTransFileTRAckCallBack(unsigned short usPonId,  unsigned short usOnuId,  unsigned short llid,
                                         unsigned short usDatLen, unsigned char *pcDatBuf, 
                                         unsigned char *pcSessId)
{
    funOamTransFileAllCallBack(OAM_FILETX_MSGCODE_TRACK,
                               usPonId, usOnuId, usDatLen, pcDatBuf, pcSessId);
}

/*================================================*/
/*name: funEponOamTransFileInit                   */
/*retu: VOS_OK / VOS_ERROR                        */
/*desc: 该函数用于初始化文件传送程序              */
/*================================================*/
long funOamTransFileInit(void)
{
	VOS_HANDLE task_id;
    gulTransFileQid = VOS_QueCreate(OAM_FILETX_QUEUE_LEN, VOS_MSG_Q_FIFO);

    if(gulTransFileQid == 0)    
    {
        sys_console_printf("Init Oam TransFile Module fail\r\n");
        return(VOS_ERROR);
    }

    INIT_LIST_HEAD(&gsTransFileLink);
    
    CommOltMsgRvcCallbackInit(OAM_FILETX_OPCODE_RWREQ, funOamTransFileRWReqCallBack);
    CommOltMsgRvcCallbackInit(OAM_FILETX_OPCODE_TRDAT, funOamTransFileTRDatCallBack);
    CommOltMsgRvcCallbackInit(OAM_FILETX_OPCODE_TRACK, funOamTransFileTRAckCallBack);
    
    if((task_id = VOS_TaskCreate(OAM_FILETX_TASKNAME, OAM_FILETX_TASKPRI, funEponOamTransFileTask, NULL)) == 0)
    {
        sys_console_printf("Create Oam TransFile task fail\r\n");
        return(VOS_ERROR);
    } 
    VOS_QueBindTask( task_id, gulTransFileQid );	/* added by xieshl 20090622 */

    glTransFileTimId = VOS_TimerCreate(MODULE_RPU_TRANSFILE, 0, OAM_FILETX_TIMER_LEN,  
                                       funOamTransFileTimerCallback, NULL, VOS_TIMER_LOOP);

    if(glTransFileTimId == VOS_ERROR) return(VOS_ERROR);

    return(VOS_OK);
}

/*=================================================================================================================== */
/*=================================================================================================================== */
/* this part only for test */
void UpdateOnuAppCallbak(long lReqType, unsigned short usPonID, unsigned short usOnuID, 
                         char *pFileName, void *pArgVoid, long lFileLen, long lSucLen, long  lErrCode)      
{
	int onuEntry = usPonID*MAXONUPERPON +usOnuID-1;

	/* modified by xieshl 20120201, 本地发生超时等失败时也应告警，问题单14348 */
	LONG slotno, portno;
	ULONG onuDevIdx;

	if( !OLT_LOCAL_ISVALID(usPonID) || !ONU_ID_ISVALID(usOnuID) )
	{
		VOS_ASSERT(0);
		return;
	}

	slotno = GetCardIdxByPonChip(usPonID);
	portno = GetPonPortByPonChip(usPonID);
	onuDevIdx = MAKEDEVID(slotno, portno, usOnuID);

	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].swFileLen = lFileLen;
	OnuMgmtTable[onuEntry].transFileLen = lSucLen;
	ONU_MGMT_SEM_GIVE;
	
	if((lFileLen == 0) || ((lFileLen > 0) && (lFileLen == lSucLen)))
	{
		/*sysDateAndTime_t systime;
		onuupdateclk_t * pclk = &OnuMgmtTable[onuEntry].updaterec;*/

		OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"\r\ncallback lRT=%d,Onu%d/%d, FLEN = %d, SULEN= %d, Err= %s\r\n",
                         lReqType,usPonID,usOnuID,lFileLen, lSucLen,funOamTransFileGetErrStrApi(lErrCode)));
		/**add by luj 2006/10/24 only for debug******/
		/* #3048 modified by chenfj 2006/10/30, for using onu app updateing handling */
		if( pArgVoid == NULL )
		{
	       	/* modified by chenfj 2008-1-31
			    问题单6255.将调试信息Transfer onu。。。。Sucessfully 屏蔽掉
			 */
	       	if( OAM_FILETX_ERRCODE_SUCCESS != lErrCode )
	       	{
				sys_console_printf("\r\nTransfer onu %d/%d/%d app Image(name=%s):%s\r\n",
					slotno, portno, usOnuID, pFileName, funOamTransFileGetErrStrApi(lErrCode));
				onuSoftwareLoadFailure_EventReport( onuDevIdx );
			}
		}
		else if( cl_vty_valid( pArgVoid ) == VOS_OK )  /*add by wangxy 2006-11-22*/
		{ 
	       	if( OAM_FILETX_ERRCODE_SUCCESS != lErrCode )
	       	{
				vty_out((struct vty *)pArgVoid, "\r\nTransfer onu %d/%d/%d app Image(name=%s):%s\r\n",
					slotno, portno, usOnuID, pFileName, funOamTransFileGetErrStrApi(lErrCode));
				onuSoftwareLoadFailure_EventReport( onuDevIdx );
			}
			vty_event( VTY_WRITE, ((struct vty *)pArgVoid)->fd, (struct vty *)pArgVoid );
		}
		/* added by chenfj 2006/10/30 
		if( lErrCode == OAM_FILETX_ERRCODE_SUCCESS )
		{
			pclk->st_clk.result = 1;
			Trap_OnuAppImageUpdate(usPonID, (usOnuID-1), V2R1_LOAD_FILE_SUCCESS );
		}
		else{
			Trap_OnuAppImageUpdate(usPonID, (usOnuID-1), V2R1_LOAD_FILE_FAILURE);
			pclk->st_clk.result = 0;
		}*/

		SetOnuSwUpdateStatus(usPonID, (usOnuID-1) , ONU_SW_UPDATE_STATUS_NOOP, IMAGE_TYPE_APP );

		if( lErrCode == OAM_FILETX_ERRCODE_SUCCESS )
		{
#if 0
			/*add by wangxy 2006-11-16 纪录任务完成时的时间*/
		    eventGetCurTime( &systime );
			/*
			sys_console_printf( "\r\nsystime: %d-%d-%d %d:%d:%d", systime.year,
				                systime.month, systime.day, systime.hour,
				                systime.minute, systime.second );*/
			pclk->st_clk.year = systime.year-2000;    /*onuupdateclk_t中'年'只纪录了两位*/
			pclk->st_clk.month = systime.month;
			pclk->st_clk.day = systime.day;
			pclk->st_clk.hour = systime.hour;
			pclk->st_clk.minute = systime.minute;
			pclk->st_clk.second = systime.second;	
		/*
			sys_console_printf( "\r\nrecord time: %d-%d-%d %d:%d:%d", pclk->st_clk.year,
				                 pclk->st_clk.month, pclk->st_clk.day, pclk->st_clk.hour,
				                 pclk->st_clk.minute, pclk->st_clk.second );*/
#endif
		}

		/*发送升级结束的消息给更新ONU的任务 add by wangxy 2006-11-02*/

		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_OVER, onuDevIdx/*parseDevidxFromPonOnu(usPonID, usOnuID-1)*/, IMAGE_TYPE_NONE );

	}
	else
	{ 
	    /*传送任务出错，结束升级*/
		if( lErrCode != OAM_FILETX_ERRCODE_SUCCESS )
		{
			SetOnuSwUpdateStatus(usPonID, (usOnuID-1) , ONU_SW_UPDATE_STATUS_NOOP, IMAGE_TYPE_APP );
			sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_FAIL, onuDevIdx/*parseDevidxFromPonOnu(usPonID, usOnuID-1)*/, IMAGE_TYPE_NONE );
		}
#if 0		
		if( pArgVoid != NULL )
		{
			vty_out((struct vty *)pArgVoid, "\r\nTransFile pon%d/%d onu%d Image(name=%s):%s, FileLen = %d, TranLen = %d\r\n",
				GetCardIdxByPonChip(usPonID), GetPonPortByPonChip(usPonID), usOnuID,pFileName, lFileLen, lSucLen);
				vty_event( VTY_WRITE, ((struct vty *)pArgVoid)->fd, (struct vty *)pArgVoid );
		}
		else
#endif
		{
			OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"\r\ncallback lRT=%d, Onu%d/%d, FLEN = %d, SULEN= %d\r\n",
				lReqType,usPonID,usOnuID,lFileLen, lSucLen));
		}
      
    }
}

void ReadOnuLogInfoCallback(unsigned short usPonID, unsigned short usOnuID, 
                            char *pFileName, char *pFileBuf, long lFileLen) 
{

    OAM_FILETX_DEBUG((/*LOG_TYPE_OAM_FILETRANS, LOG_DEBUG_OUT,*/"recv file,onu%d/%d,filename= %s, len = %d\r\n",
                       usPonID,usOnuID, pFileName, lFileLen));

   g_free(pFileBuf);
}

long  UpdateOnuAppReq(short int usPonID, short int usOnuID, char *type, char *name, long ltype, struct vty *vty )
{
	/*OnuMgmtTable[usPonID * MAXONUPERPON + (usOnuID-1)].vty = vty; */
	return(funOamTransFileLocReqApi(2, usPonID, usOnuID, type, name, ltype, vty, UpdateOnuAppCallbak, NULL));
}

void ReadOnuLogInfoReq(short int usPonID, short int usOnuID,  char *type, char *name, long ltype)
{
    funOamTransFileLocReqApi(1, usPonID, usOnuID, type, name, ltype, NULL, NULL, ReadOnuLogInfoCallback); 
}

/* added by xieshl 20100722, 如果在升级过程中ONU被删除时，应从文件传送链表中删除该ONU对应的节点，
     防止后续再次升级时处理错误，这个问题在保加利亚曾发生过2起，导致文件传输任务异常 */
int onuDeregisterCallback( short int usPonID, short int usOnuID )
{
	unsigned long ulMessage[4] = {MODULE_RPU_TRANSFILE, MSG_ONU_DELETE_EVENT, 0, 0};

	ulMessage[3] = ((usPonID << 16) & 0xffff0000) | usOnuID;

	return VOS_QueSend(gulTransFileQid,  ulMessage, NO_WAIT, MSG_PRI_NORMAL);
}

static void funOnuDelProcMessages( ULONG msg )
{
	short int usPonID = (msg >> 16) & 0xffff;
	short int usOnuID = msg & 0xffff;
	OAM_FILETX_CTRL_S * pCtrItem  = funOamTransFileFindInstance( usPonID,  usOnuID);
	
	if( pCtrItem )
	{
		pCtrItem->lErrorNo = OAM_FILETX_ERRCODE_TRANERR;
		funOamTransFileEndInstance( pCtrItem );
	}
}


#if 0
void sendreqack(long type)
{
    OAM_FILETX_ACKHD_S *buff = NULL;
	char *seid;
    buff =(OAM_FILETX_ACKHD_S*)VOS_Malloc(sizeof(OAM_FILETX_ACKHD_S),MODULE_RPU_TRANSFILE);
    seid =(char*)VOS_Malloc(8,MODULE_RPU_TRANSFILE);
	buff->sAckCode = type;
	buff->sErrCode = type;
	buff->lDataFcs = 0;
	buff->lDataLen = 0;
	buff->lFileLen = 4200;
	buff->lFileOff = 0;

	*((long*)seid) = glTransSessionId;

	funOamTransFileTRAckCallBack(6,1,sizeof(OAM_FILETX_ACKHD_S),(char*)buff,seid);

}

void senddatack(long type, long datlen, long flen,long off)
{
    OAM_FILETX_ACKHD_S *buff = NULL;
	char *seid;
    buff =(OAM_FILETX_ACKHD_S*)VOS_Malloc(sizeof(OAM_FILETX_ACKHD_S),MODULE_RPU_TRANSFILE);
	seid =(char*)VOS_Malloc(8,MODULE_RPU_TRANSFILE);
    buff->sAckCode = 0x300 + type;
	buff->sErrCode = type;
	buff->lDataFcs = 0;
	buff->lDataLen = datlen;
	buff->lFileLen = flen;
	buff->lFileOff = off;

	*((long*)seid) = glTransSessionId;

	funOamTransFileTRAckCallBack(6,1,sizeof(OAM_FILETX_ACKHD_S), (char*)buff,seid);

}


void senddata(long datlen, long flen,long off)
{
    
    OAM_FILETX_PKTHD_S *buff = NULL;
	char *seid;
    buff =(OAM_FILETX_PKTHD_S*)VOS_Malloc(1500,MODULE_RPU_TRANSFILE);
	seid =(char*)VOS_Malloc(8,MODULE_RPU_TRANSFILE);
	buff->lDataFcs = 0;
	buff->lDataLen = datlen;
	buff->lFileLen = flen;
	buff->lFileOff = off;

	*((long*)seid) = glTransSessionId;

	funOamTransFileTRDatCallBack(1,1,datlen, (char*)buff,seid);

}


/*-----------------add by luj 2006/10/22---用于调试-----------------------------*/
void prin_receiveMesg()
{
	printf("glReceiveMesCount = %d\r\n",glReceiveMesCount);
	printf("glReceiveMesTRREQ = %d\r\n",glReceiveMesTRREQ);
	printf("glReceiveMesDATA = %d\r\n",glReceiveMesDATA);
	printf("glReceiveMesACK = %d\r\n",glReceiveMesACK);
	
}
void clear_receiveMesg()
{
	glReceiveMesCount = 0;
	glReceiveMesTRREQ=0;
		glReceiveMesDATA=0;
		glReceiveMesACK=0;
}

void openfiledetaildebug()
	{

	glTransFileDetailDebug = 1;
	}

void closefiledetaildebug()
	{

	glTransFileDetailDebug = 0;

	}
void openError_lock(char  datalen,char offset, unsigned long filelen,char acklock,char printfile,char sendWrongAck,char testmode)
{
	DATALEN_E = datalen;
	OFFSET_E = offset;
	FILELEN_E = filelen;
	ACK_LOCK = acklock;
	PRINT_FILE = printfile;
	SEND_WRONG_DATA_ACK = sendWrongAck;
	TEST_MODE = testmode;
}
void closeDATALEN_E()
{
	DATALEN_E = 0;
	OFFSET_E = 0;
	FILELEN_E = 0;
	ACK_LOCK =0;
}
void freefileadd(char * p)
{
	 free(p);
}
void seedebuglock()
{
	printf("DATALEN_E=%x\r\n  /
		OFFSET_E=%x\r\n  /
		FILELEN_E=%x\r\n /
		ACK_LOCK=%x\r\n /
		PRINT_FILE=%x\r\n /
		SEND_WRONG_DATA_ACK=%x\r\n",DATALEN_E,OFFSET_E,FILELEN_E,ACK_LOCK,PRINT_FILE,SEND_WRONG_DATA_ACK);
}
void delayforwhil()
{
  unsigned long i;
  for (i=0;i<1000;i++)
  	{
		i=i;
  	}
  
}
#endif
/*=================================================================================================================== */
/*=================================================================================================================== */
