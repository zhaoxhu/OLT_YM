/* ================================================ */
/* FileName: TransFile.h                            */
/* Author  : suxiqing                               */
/* Date    : 2006-10-10                             */
/* Description: 此文件仅为EPON设计, 实现通过OAM通道 */
/*              向ONU发送或接收文件                 */
/* ================================================ */

/*=================================================================================================================== */
/*=================================================================================================================== */
/*
#define OAM_FILETX_MODULE_ID         0x8750*/       /* 模块ID           */
                                                 /* 需要产品统一定义模块ID */

#define OAM_FILETX_WAITTIME          900          /* 发送等待900秒    */ 

#define OAM_FILETX_RETRY_COU         6           /* 重发次数3         */
#define OAM_FILETX_RETRY_INT         6        /* 重发间隔3秒     994967290 */
#define OAM_FILETX_RETRY_NO          0           /* 不重发           */

#define OAM_FILETX_RATETIME          (OAM_FILETX_RETRY_INT * OAM_FILETX_RETRY_COU + 2)
/*=================================================================================================================== */

/* 发送函数调用方式    */
#define OAM_FILETX_CALL_NORMAL       1           /* 调用发送请求使用平常机制 调用后立即返回    */
#define OAM_FILETX_CALL_BLOCK        2           /* 调用发送请求使用阻塞机制 调用后等待发完返回*/

/* 读写请求类型        */

#define OAM_FILETX_REQ_READ          1           /* 管理层调用请求类型 接收 读                 */
#define OAM_FILETX_REQ_WRITE         2           /* 管理层调用请求类型 发送 写                 */

/*=================================================================================================================== */

                                                 /* 执行完管理层发起的读/写请求后后的回调函数,用于通知执行结果 */
                                                 /* lReqType 请求类型, pFileName 文件名, lErrCode 错误代码*/
                                                 /* lFileLen 文件长度，lSucLen 当前进度*/
                                                 /* 当 lFileLen = 0(失败的传输) 或 lFileLen > 0 且 lSucLen = lFileLen 时, 表示文件传送完毕 最后一次回调*/
typedef void (*POAMTRANSFILECALLBACK)(long  lReqType,  unsigned short usPonID, unsigned short usOnuID,
                                      char *pFileName, void *pArgVoid, long lFileLen, long lSucLen, long lErrCode); 
                                                 /* 执行从ONU读取文件请求时, 当正确接收完文件后, 执行的回调函数 */
												 /* pFileType 文件类型, pFileName 文件名, pFileBuf 文件的接收缓存 lFileLen 文件长度*/
												 /* pFileBuf 由请求发起者释放, 释放函数 free() */ 
typedef void (*POAMRECVFILECALLBACK)(unsigned short usPonID, unsigned short usOnuID, 
                                     char *pFileName, char *pFileBuf, long lFileLen);   
                                                 
/*=================================================================================================================== */
/*=================================================================================================================== */
/*API 部分   */
/*================================================*/
/*name: funOamTransFileLocReqApi                  */
/*para: lReqType  指示是否为发送请求1 read/2 write*/ 
/*para: lOnuID    ONU 号                          */ 
/*para: lFileType 需要发送的文件类型              */
/*para: lCallType 请求的执行方式 1 normal 2block  */
/*                1 立即退出 2 阻塞 请求执行完退出*/
/*para: pReqCallback 请求执行结果回调函数         */
/*para: pFileCallBack 接收文件回调函数            */
/*retu: ERROR CODE (0 no error >0 have error)     */
/*desc: 调用该函数向指定ONU发送指定类型文件       */
/*================================================*/
   
long funOamTransFileLocReqApi(long  lReqType,   unsigned short usPonID, unsigned short usOnuID, 
                              char *pcFileType, char *pcFileName, long lCallType, void *pArgVoid,
                              POAMTRANSFILECALLBACK pReqCallback, POAMRECVFILECALLBACK pFileCallBack);
 
/*================================================*/
/*name: funOamTransFileGetErrStrApi               */
/*para: lErrorNo  错误号                          */ 
/*retu: error inf string                          */
/*desc: 调用该函数获取发送文件的错误信息串        */
/*================================================*/
char *funOamTransFileGetErrStrApi(long lErrorNo);
 

/*================================================*/
/*name: funEponOamTransFileInit                   */
/*retu: VOS_OK / VOS_ERROR                        */
/*desc: 该函数用于初始化文件传送程序              */
/*================================================*/
long funOamTransFileInit(void);
/*=====================================================================================================================*/
/*=====================================================================================================================*/

