/* ================================================ */
/* FileName: IgmpAuth.h                             */
/* Author  : suxiqing                               */
/* Date    : 2006-10-25                             */
/* Description: 此文件仅为EPON设计, 实现通过OAM通道 */
/*              发送IGMP认证消息                    */
/* ================================================ */
#include "v2r1general.h"
#include "sys/main/sys_main.h"
#include	"vos/vospubh/cdp_pub.h" 
/*=================================================================================================================== */
/*=================================================================================================================== */
#define IGMP_AUTH_FOR_CTC
#undef  IGMP_AUTH_FOR_CTC
/*=================================================================================================================== */
/*=================================================================================================================== */

#if 0  /*ndef IGMP_AUTH_FOR_CTC*/

#define IGMP_AUTH_TASKNAME          "tIgmpAuth"
#define IGMP_AUTH_QUEUE_LEN          128         /* 队列长度         */
#define IGMP_AUTH_TASKPRI            150         /* 任务优先级       */

#define IGMP_AUTH_MODULE_ID          MODULE_IGMPAUTH
/*=================================================================================================================== */
#define IGMP_AUTH_ERROR_NO           0           /* no error         */
#define IGMP_AUTH_ERROR_LEVERR       1           /* 发送强制请求失败 */

/*=================================================================================================================== */
typedef void (*IGMPAUTHLEAVECALLBACK)(unsigned short usPonID,   unsigned short usOnuID, 
                                      unsigned short usVid,     unsigned long  ulMcIP,
                                      unsigned long  ulScrPort, unsigned long  ulScrIP,
                                      unsigned char *pScrMac,   unsigned long  ulResult);
                                     /* 执行强制LEAVE请求的回调 */

/*=================================================================================================================== */

long funIgmpAuthUsrLevCfgApi(unsigned short usPonID, unsigned short usOnuID,   unsigned short usVid,  
                             unsigned long  ulMcIP,  unsigned long  ulScrPort, unsigned long  ulScrIP, 
                             unsigned char *pScrMac, IGMPAUTHLEAVECALLBACK pCallBack);

/*================================================*/
/*name: funIgmpAuthInit                           */
/*retu: VOS_OK / VOS_ERROR                        */
/*desc: 该函数用于初始化IGMP认证注册任务          */
/*================================================*/
long funIgmpAuthInit(void);

/*=================================================================================================================== */
#else  /* def IGMP_AUTH_FOR_CTC*/

/*=================================================================================================================== */

#define IGMP_AUTH_TASKNAME          "tIgmpAuth"
#define IGMP_AUTH_QUEUE_LEN          128         /* 队列长度         */
#define IGMP_AUTH_TASKPRI            150         /* 任务优先级       */

/*=================================================================================================================== */
/* REQTYPE */
#define IGMP_AUTH_REQTYPE_JOIN       1
#define IGMP_AUTH_REQTYPE_LEAVE      2
#define IGMP_AUTH_REQTYPE_FORCELEA   3

/* RESULT  */
#define IGMP_AUTH_RESULT_AGREE       1
#define IGMP_AUTH_RESULT_REFUSE      2
#define IGMP_AUTH_RESULT_NONOTI      3
#define IGMP_AUTH_RESULT_NONE        0

#define IGMP_AUTH_ONUTYPE_UNK        0
#define IGMP_AUTH_ONUTYPE_CTC        1
#define IGMP_AUTH_ONUTYPE_GW         2
#define IGMP_AUTH_ONUTYPE_OAM        3


typedef struct{
    unsigned short usReqTyp;
    unsigned short usPonId;  
    unsigned short usOnuId;
    unsigned short IsDayaOnu;
    unsigned short usDatLen;
    unsigned short ussrcslot;
    unsigned char pSessId[8];
    unsigned char *pDatBuf;
}IgmpAuthCDPMsgHead_t;

/*=================================================================================================================== */

LONG funIgmpAuthManagerSetApi(LONG  lPonID,  LONG lOnuID,     LONG  lReqType,  
                              LONG  lUserID, ULONG ulGroupIP, LONG  lMVlan, LONG lOnuType, char *bUsrMac);
extern ULONG DEV_GetPhySlot(VOID);
extern short int GetPonPortIdxBySlot( short int slot, short int port );
extern int GetCardIdxByPonChip( short int PonChipIdx );
extern int GetPonPortByPonChip( short int PonChipIdx );
extern short int GetLlidByOnuIdx( short int PonPortIdx, short int OnuIdx );
extern int Comm_IGMPAuth_Requst(const unsigned short PonId, 
								const unsigned short OnuId,
								unsigned char *pIgmpAuth,
								const unsigned short igmp_data_size,
								unsigned char *psessionIdField);
extern int Comm_IGMPAuth_Reponse(const unsigned short PonId, 
								const unsigned short OnuId,
								unsigned char *pIgmpAuth,
								const unsigned short igmp_data_size,
								unsigned char *psessionIdField);
extern int  CommOltMsgRvcCallbackInit(unsigned char GwProId, void *Function);

/*================================================*/
/*name: funIgmpAuthInit                           */
/*retu: VOS_OK / VOS_ERROR                        */
/*desc: 该函数用于初始化IGMP认证注册任务          */
/*================================================*/
long funIgmpAuthInit(void);

/*=================================================================================================================== */

#endif

