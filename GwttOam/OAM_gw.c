
/**************************************************************************************
* OAM_gw.c - .
* 
* Copyright (c) 2006 GW Technologies Co., LTD.
* All rights reserved.
* 
* create: 
* 2006-05-20  wutw
* 
* 文件描述：
* modified
* 2006-10-9	 修改oam发送接收缓冲区错误分配内存的问题(使用了calloc，
*			 在平台被封装为vos_malloc)，缓冲区使用静态数组的方式。
* 2006-10-9	 修改在oam任务中判断llid为0xffff	时直接返回而没有释放缓冲区的错误
*			 并在各个接口增加判断llid是否可用的代码，使得当onu离线时，不再向
*			 oam任务发送oam帧，减少了任务负担
* 2006-10-13 增加文件传输接口,对于回调函数部分,注册码调整为opcode,详细见头文件.
* 2006-10-16 
*            增加oam部分debug调试开关,可针对cli接口,snmp接口,文件传输接口等分别设置调试开关.
* 2006-10-19 增加oam 模块err debug 开关，oam 接收数据包track debug开关
* 2006-10-19 增加oam 模块收发包统计
* 2006-10-25 调整oam 任务优先级由100为58
* 2006-10-25 针对复现的不能接收onu oam帧的现象,在oam 任务增加: 当未知opcode类型时的缓冲区释放代码
*			 修改向oam任务发送消息函数: 增加当申请一个oam消息空间失败时,返回之前释放收/发缓冲区,
*			 							修改当接收onuoam帧时,错误使用发送缓冲区的参数,错误释放发
*										送的缓冲区
* 2006-10-25 修改oam任务收发缓冲区长度,#define GW_MAX_MSG_LENGTH	1580. 该宏定义原为65535,此处考
*			虑到pas-soft 到pas5001处最大的帧长为1580,固定义为该值,当olt侧发送超长帧时,该帧拆分在oam 
*			task外进行拆分
* 2006-10-25 增加oam任务队列的缓冲区的个数,考虑到olt上电后,OAM 任务会收发较多的oam帧,任务队列由20修改
*			为50 : #define OAM_TASK_QUEUE_NUM	50
* 2006/11/23 : pclint 检查，修改部分代码
* 2006/11/23 : 检查并修改接收oam帧的函数CommOltMsgQueueSave()与CommOltMsgReveive()函数
*			并解决#3270的问题:在ONU端口上发送1200条以上的数据包,执行atu 
*			show命令出现不能响应的信息和断言.
* 2006/11/24 : 修改oam debug开关的相关接口的代码，保证能够单独打开某一项的debug开关
* 2007/01/17 : 增加设置onu系统时间的接口，支持scb通道传输，单个onu传输，修改oam任务，以
*			支持scb的传输
* 2007-09-21 modified by wangxy
*    CommOltMsgRvcCallbackDel函数去掉信息打印
*
* modified by chenfj 2007-9-24
*	问题单：＃5430，在GT816的onu节点下，执行stat port_show 1,打印断言，OLT重启
*	将OAM接收队列缓冲区加大(原值为50，现加大到512 )
*
* modified by chenfj 2007-12-14
*    增加一个老化定时，对需重组的OAM 帧，若后续分片没有收到，则在老化时间
*    到时后，不再处理这个OAM 帧，且释放占用的数据缓冲区
*
* modified by chenfj 2008-7-1
*    增加vconsole 传输通道( oam 层)
* modified by chenfj 2008-7-9
*       增加GFA6100 产品支持
***************************************************************************************/

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "PonEventHandler.h"
#include  "V2R1_product.h"

#include "OAM_gw.h"

#include "vos_base.h"
#include "vos_sem.h"
#include "vos_mem.h"
#include "vos_que.h"
#include "vos_task.h" 
#include "vos_sem.h"
#include "../superset/platform/manage/mn_oam.h"
/*#define OAM_DEBUG;*/
/*----------------------------------------------------------*/
/*						Return codes						*/
/*----------------------------------------------------------*/
#define OAM_OK										0
#define OAM_ERR										(-1)
#define OAM_TIME_OUT								(-2) 
#define OAM_INVALUEBUF_ERR							(-3)
#define OAM_NULL_POINTER							(-4)
#define OAM_OPCODE_INVALUE_ERR						(-5)
#define OAM_NOT_INIT_ERR							(-6)
#define OAM_CALLBACK_FULL_ERR						(-7)
#define OAM_SEQUENCE_ERR							(-8)
#define OAM_WHOLELENGTH_ERR							(-9)
#define OAM_INVALUE_SENDSENO_ERR					(-10)
#define OAM_MSGQSEND_ERR							(-11)
#define OAM_INVALUE_QUEUE_ERR						(-12)
#define OAM_INVALUE_MSGTYPE_ERR						(-13)
#define OAM_SESSIONFIELD_ERR						(-14)
#define OAM_ONU_DEREGISTER_ERR						(-15)
#define OAM_MALLOC_ERR								(-16)
#define OAM_TASK_MSGTYPE_UNKNOW_ERR					(-17)
#define OAM_TASK_MSGSEND_FAIL_ERR					(-18)
#define OAM_TASK_WHOLELEN_MACHT_ERR					(-19)
#define OAM_TASK_PALOADLENGTH_ERR					(-20)
#define OAM_TASK_SENDSERNO_ERR						(-21)
#define OAM_UNKOWNED_DEVTYPE_ERR			(-22)
#define OAM_MIMATCH_DEVTYPE_ERR				(-23)
#define	OAM_PONID_NOTWORK							(-24)

#define OAM_LLID_MAX						127


#if 0
#define MAX_SIZE_OF_OAM_FRAME		1464/*1514*//*1480*/	/* Regular Host - OLT MII protocol,*/
												/*ETHERNET LENGTH 1500.SO GW OAM MAX : 1500 -20 = 1480*/
#endif
/*根据现在的测试，onu侧最大只能够接收总长不超于295的数据包，目前修改为一下长度*/
#define MAX_SIZE_OF_OAM_FRAME		295/*1514*//*1480*/	/* Regular Host - OLT MII protocol,*/
												/*ETHERNET LENGTH 1500.SO GW OAM MAX : 1500 -20 = 1480*/

#define GW_OAM_DA						0
#define GW_OAM_SA						6
#define GW_OAM_ETH_TYPE				12
#define GW_OAM_SUBTYPE				14
#define GW_OAM_FLAGS				15
#define GW_OAM_CODE					17
#define GW_OAM_PAD_OUI				18

#define GW_OAM_PAD_OPCODE			21
#define GW_OAM_PAD_SENDSERNO			22
#define GW_OAM_PAD_WHOLEPKTLEN		26

#define GW_OAM_PAD_PAYLOADOFFSET	28
#define GW_OAM_PAD_PAYLOADLENGTH	30

#define DRPENG_OAM_PAD_OUI       3
#define DRPENG_OAM_PAD_PORT_TVL       8
#define DRPENG_OAM_PAD_BRANCH       22
#define DRPENG_OUI_DEFINE					0x0C7C7D

#define GW_OUI_DEFINE					0x000FE9
#define GW_OAM_HEADER_SIZE			18
#define GW_OAM_PAD_LENGTH			14
#define GW_OAM_SESSIONID				8
#define GW_BYTES_MAC_ADDRESS			BYTES_IN_MAC_ADDRESS /*6*/
#define GW_EHTERNET_HEAD_LEN			20
#define GW_ETH_HEADER_SIZE			14

/*------其他--------*/
#define GW_MAX_SENDSERNO				0xffffffff
#define GW_DEFAULT_TASKDELAY_TICK	1
#define GW_MAX_OLTID					MAXPON /*20*/
#define GW_MAX_ONUID				MAXONUPERPON  /*64*/
#define GW_MAX_QUEUEID				10
#define GW_MAX_MSG_LENGTH			65535
#define GW_MAX_ONU					MAXONUPERPON  /*64*/


#define GW_MSG_SEND		1
#define GW_MSG_REVEICE		2

#define GW_MSG_REV_QUIDX		1
#define GW_MSG_SEND_QUIDX		0

#define GW_DEVTYPE_OLT			0
#define GW_DEVTYPE_ONU			1


#define OAM_BITS_IN_BYTE    8
#define OAM_BYTES_IN_LONG   4
#define OAM_BYTES_IN_DOUBLE 8
#define OAM_BYTES_IN_SYMBOL 4
#define OAM_BYTES_IN_WORD   2
#define OAM_BYTES_IN_CHAR	1
#define MIN_ETHERNET_FRAME_SIZE					60 /* Minimal Ethernet frame length excluding Preamble,*/
#define MAX_ETHERNET_FRAME_SIZE_STANDARDIZED  1514 /* Maximal Ethernet frame length excluding Preamble */
												   /* ,SFD and FCS fields standard value, not including*/
												   /* any extensions	*/		
/*Frame sent to all LLIDs
(broadcast LLID)  : PON_EVERY_LLID
(-32767)*/
#define OAM_PON_EVERY_LLID				((-1)*32767)

/*Frame sent to the LLID
specified in LLID field   : FLASE (0)*/
#define OAM_SEND_SPECIFIED_LLID				0
/*Frame sent to the all the LLIDs except
the LLID specified in LLID field   : TRUE (1)*/
#define OAM_SEND_ALL_EXCEPT_LLID		1
/*
	modified by chenfj 2007-9-24
	问题单：＃5430，在GT816的onu节点下，执行stat port_show 1,打印断言，OLT重启
 	将OAM接收队列缓冲区加大(原值为50，现加大到512 )
*/
#define OAM_TASK_QUEUE_NUM				512
#define OAM_DATA_LEN					65535
#define OAM_VOSTASK_QUEUE_NUM			512

#define OAM_TASK_PRIO					58

#define OAM_DAYAONU_DEVTYPE	1
#define OAM_OTHERONU_DEVTYPE 	2

/* SFD and FCS fields							   */
/*============用于文件传输控制=============*/
/*
ACK代码： 
           0x100  读拒绝
	   0x101  读允许
           0x200  写拒绝
           0x201  写允许
           0x300  传送错误
           0x301  传送开始（传送过程控制应答，也表示传送正常）
           0x302  传送中（传送过程控制应答，也表示传送正常）transmit
           0x303  传送结束（传送过程控制应答，也表示传送正常） 
错误代码：（暂定）
          0x00 无错误
          0x01 系统忙
          0x02 系统资源不足
          0x03 系统处理错误
          0x04 流程错误
          0x05 文件不存在
          0x06 文件太长
          0x07 文件太短
          0x08 长度或偏移匹配错误
          0x09 数据校验错误
          0x0A 文件保存错误
          
*/
#define FILE_CTRL_ACK_RDREFUCTED			0x100
#define FILE_CTRL_ACK_RDACCEPT				0x101
#define FILE_CTRL_ACK_WRTREFUCTED			0x200
#define FILE_CTRL_ACK_WRTACCEPT				0x201
#define FILE_CTRL_ACK_TXERROR				0x300
#define FILE_CTRL_ACK_TXSTART				0x301
#define FILE_CTRL_ACK_TRANSMITTING			0x302
#define FILE_CTRL_ACK_TXOVER				0x303

#define FILE_CTRL_ERRCODE_OK				0x00
#define FILE_CTRL_ERRCODE_SYSBUSY			0x01
#define FILE_CTRL_ERRCODE_SYSNORSC			0x02
#define FILE_CTRL_ERRCODE_SYSERRO			0x03
#define FILE_CTRL_ERRCODE_FLOWERR			0x04
#define FILE_CTRL_ERRCODE_NOTFILE			0x05
#define FILE_CTRL_ERRCODE_LONGERR			0x06
#define FILE_CTRL_ERRCODE_SHORTERR			0x07
#define FILE_CTRL_ERRCODE_NOTMATCH			0x08
#define FILE_CTRL_ERRCODE_CHECKERR			0x09
#define FILE_CTRL_ERRCODE_SAVEERR			0x0a

/*
#define FILE_TRANSFER_HEAD					16
#define FILE_ACK_LENGTH						20
#define FILE_MAX_LENGTH						131070
#define FILE_MIN_LENGTH						60
#define FILE_CTRL_READ						1
#define FILE_CTRL_WRITLE						2
*/
#define OAM_INVALID_LLID					INVALID_LLID /*(-1)*/
#define OAM_PONPORT_UP					PONPORT_UP	/* 1 */

/*===========调试宏定义===========*/
unsigned int debugRevCount = 0;
unsigned int debugSendCount = 0;
/*unsigned int oam_temp_debug = 0;
unsigned int oam_temp_debug_send = 0;
unsigned int oam_temp_debug_stop = 0;
unsigned int oam_temp_debug_rev = 0;*/
/*============================== Constants ==================================*/

typedef unsigned char Oam_mac_address_t[6];
/*
typedef struct
{
    unsigned int sendSerNo;
    unsigned int fileLen;
    unsigned int fileOff;
    unsigned int dataLen;
    unsigned int datafcs;
    unsigned char fileNameLen;
    unsigned char  *pfileName;
    unsigned char *pDataBlk;
} GW_FILE_REVBUF_t ;
*/
typedef struct
{
    unsigned char  gw_opcode;
    unsigned int   sendserno;
    unsigned short llid;
    unsigned short wholepktlen;
    unsigned short payloadoffset;
    unsigned short payloadlength;
    unsigned char  sessionFieldbuf[8];
    unsigned char *pBuf;
    /*unsigned char *payloadpad;*/

}__attribute__((packed))GW_OAM_PAD_layer_msg_t ;

typedef struct 
{
	/*WDOG_ID   onuWdId;*/
	SEM_ID		onuSemId;	
	BOOL			Iused;
	unsigned char RecvMsgTime;
	unsigned char *payloadpad;
} GW_MANAGEMENT_ONU_QUEUE_INFO_t;

/*数据结构*/
 typedef struct
{
	GW_OAM_PAD_layer_msg_t	onu_msg[GW_MAX_QUEUEID];
	GW_MANAGEMENT_ONU_QUEUE_INFO_t onumsg_info[GW_MAX_QUEUEID];
} Gw_ONU_QUEUE_t,Gw_onu_t;


typedef struct
{
	unsigned short int  olt_id;
 	/*GW_ONU_MANAGEMENT_QUEUE_t onu_queue_t[GW_MAX_ONUID];*/
	Gw_ONU_QUEUE_t  onu_queue_t[MAXONUPERPONNOLIMIT] ;/*GW_MAX_ONU*/
}GW_OLT_MANAGEMENT_QUEUE_t;




typedef struct
{
    unsigned char   sub_type ;  
    unsigned short  flags;  
    unsigned char   oam_code ; 
    unsigned char   gw_oui_1;
    unsigned char   gw_oui_2;
    unsigned char   gw_oui_3;
    	
}/*GW_MANAGEMENT_oam_layer_msg_t,*/__attribute__((packed))GW_oam_layer_msg_t;



typedef  struct
{
	BOOL	   flagUsed;
	short int      OltId ;
	short int      OnuId ;
	unsigned short QueueId ;
}GW_MSG_WDINFO_t;



/*发送和接收缓冲区相关数据结构定义*/

typedef struct
{
	unsigned short			OltId;
	unsigned short			OnuId;
	unsigned long			sendSerNo;
	unsigned int			queuePro;
	short int			    unicstLlid;  /* added by liwei056@2012-12-6 for OAM's llid knowing */
	short int				brdcstLlid;
	unsigned int			bufPara;
	unsigned int			bufSize;
	unsigned char			oamOpcode;
	
}GW_OAM_MSG_t;

typedef struct 
{
	BOOL    IsUsed;     
	
	unsigned int  MesLen;                      
	unsigned char   MesData[1580];              

}STRU_OAM_FRAME_QUEUE;
/*
typedef struct 
{
	BOOL    IsUsed;     
	
	unsigned int  MesLen;                      
	unsigned char   MesData[1500];              

}STRU_OAM_REV_FRAME_QUEUE;
*/
typedef struct 
{
	UINT32     MaxSize;
	
	UINT32     WriteIndex;

	UINT32     ReadIndex;

	ULONG     DataSem;

    STRU_OAM_FRAME_QUEUE *DataBuff;
}STRU_OAM_QUEUE_CONTROL;

typedef struct
{
	unsigned short oltId;
	unsigned short onuId;	
	unsigned short ackCode;
	unsigned short ErrCode;
	unsigned int fileLen;
	unsigned int fileOff;
	unsigned int dataLen;
	unsigned int datafcs;	
	unsigned char *pFileBuf;
} GW_OAM_FILECTR_t;

/* Frame destination OLT port options */
#if 0
typedef enum
{
	OAM_PORT_PON,					   /* PON port													   */
	OAM_PORT_SYSTEM,                   /* a.k.a. Network											   */
	OAM_PORT_PON_AND_SYSTEM,		   /* Both PON and System ports									   */
	OAM_PORT_AUTOMATIC,                /* Default selection of physical port by address table / Both   */
									   /* PON and System ports (if address not found in address table) */
	OAM_PORT_AUTOMATIC_INHIBIT_ALL,	   /* Default selection / discard if address not found in address  */
									   /* table)													   */
	OAM_PORT_AUTOMATIC_INHIBIT_PON,	   /* Default selection / System port (if address not found in     */
									   /* address table)											   */
	OAM_PORT_AUTOMATIC_INHIBIT_SYSTEM  /* Default selection / PON port (if address not found in address*/
									   /* table)													   */
} OAM_PON_sent_frame_destination_port_t;
#endif
typedef  PON_sent_frame_destination_port_t  OAM_PON_sent_frame_destination_port_t;

typedef struct
{
	unsigned long oamSendSuccess;
	unsigned long oamSendFailed;
	unsigned long oamRevtotal;
	unsigned long oamRevPro;
}OAM_MIB;

unsigned int OAM_DEBUG_FLAG = V2R1_DISABLE;

#if 0
#endif
OAM_MIB *pgGwOamMib = NULL;
unsigned long 	gSendSerNo = 1;
SEM_ID		gSendNoSemId;	
SEM_ID 		gWdInfoSemId ;

VOS_HANDLE		gOamTaskId = 0;
ULONG		gOamOnuTaskId = 0;
ULONG		gFileTaskId = 0;

/*The ranger of time : 0 - 100(ticks) .default is 20 ticks*/
unsigned int	gTaskDelayTime = 0;
/*GW_MSG_WDINFO_t	gMsgWdInfo[GW_MAX_OLTID*GW_MAX_ONUID*GW_MAX_QUEUEID];*/
#if 0
GW_OLT_MANAGEMENT_QUEUE_t		goltMsg[20];  /* GW_MAX_OLTID */
#else
GW_OLT_MANAGEMENT_QUEUE_t      *goltMsg;
#endif
/*GW_OLT_MANAGEMENT_QUEUE_t		gonuMsg[20];*/ /* GW_MAX_OLTID */

/* for oam - using always this multicast address */
static unsigned char GW_oam_destination_address[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x02 };
static const unsigned short gGwEthernetType = 		0x8809;
static const unsigned char gGgwOamSubType  =  		0x03 ;
static const unsigned char gGwOamCode	=  		0xFE ;
static const unsigned short gGwOamFlags    	=  		0x50 ;
static BOOL		gboolGwInit	= FALSE;

ULONG				gOamMsgQId = 0;
ULONG				gOamOnuMsgQId = 0;
ULONG				gFileMsgQId = 0;
STRU_OAM_QUEUE_CONTROL  gOamQueueCtl[2]={{0,0,0,0,NULL},{0,0,0,0,NULL}};


/*#undef OAM_DEBUG_PRINTF*/


/*===============以下变量为文件传送控制变量============*/
unsigned char *pgFileBuf = NULL;
/*GW_FILE_REVBUF_t *pgFileRevBuf[GW_MAX_OLTID][GW_MAX_ONUID];*/
SEM_ID			gFileTickSemId;

/*static BOOL		gbFileRev[GW_MAX_OLTID][GW_MAX_ONUID];*/

#if 0
STRU_OAM_FRAME_QUEUE oamSendQueue[OAM_TASK_QUEUE_NUM];
STRU_OAM_FRAME_QUEUE oamRevQueue[OAM_TASK_QUEUE_NUM];
#else
STRU_OAM_FRAME_QUEUE *oamSendQueue;
STRU_OAM_FRAME_QUEUE *oamRevQueue;
#endif


GW_OAM_FILECTR_t gstrOamFileCtrl;

/*for debug printf info*/
int oam_debug_info[GW_OPCODE_LAST] = {0};  /* 20*/
int oam_debug_count = 0;
unsigned long gGwOamRevTempCount = 0;
unsigned long gGwOamRevCount[GW_OPCODE_LAST] = {0};  /* 20*/
unsigned long gGwOamTotalRevCount = 0;
unsigned long gGwOamSendCount[GW_OPCODE_LAST] = {0}; /* 20 */
unsigned long gGwOamTotalSendCount = 0;

int gGwOamTaskErrDebug = 0;
int gGwOamTrackDebug = 0;
int oam_debug_send = 0;
int oam_brd_debug = 0;

unsigned char *OAMCodeToString[] = 
{
	(unsigned char *)"0-reserved",
	(unsigned char *)"1-euqinfo req",
	(unsigned char *)"2-euqinfo ack",
	(unsigned char *)"3-alarm&log info",
	(unsigned char *)"4-alarm&log ack",
	(unsigned char *)"5-file r/w req",
	(unsigned char *)"6-file reserved0",
	(unsigned char *)"7-file transmit data",
	(unsigned char *)"8-file transmit ack",
	(unsigned char *)"9-file reserved1",
	(unsigned char *)"10-file reserved2",
	(unsigned char *)"11-snmp req",
	(unsigned char *)"12-snmp rsp",
	(unsigned char *)"13-snmp trap",
	(unsigned char *)"14-snmp reserved1",
	(unsigned char *)"15-snmp reserved2",
	(unsigned char *)"16-cli req",
	(unsigned char *)"17-cli ack",
	(unsigned char *)"18-igmpauth req",
	(unsigned char *)"19-igmpauth ack",
	(unsigned char *)"20-vconsole",
	(unsigned char *)"21-oam debug",
	(unsigned char *)""
};

/*
#define FILE_CTRL_ACK_RDREFUCTED			0x100
#define FILE_CTRL_ACK_RDACCEPT				0x101
#define FILE_CTRL_ACK_WRTREFUCTED			0x200
#define FILE_CTRL_ACK_WRTACCEPT				0x201
#define FILE_CTRL_ACK_TXERROR				0x300
#define FILE_CTRL_ACK_TXSTART				0x301
#define FILE_CTRL_ACK_TRANSMITTING			0x302
#define FILE_CTRL_ACK_TXOVER				0x303
*/


/*========extern funtions=========*/

/*========internal  funtions=========*/
#if (RPU_MODULE_IGMP_TVM == RPU_YES )
extern long glIgmpTvmDebug;/* add by duzhk */
extern STATUS Igmp_Tvm_Debug_Head(char *buftemp);
#endif
void (*handle_oamfuncs[GW_CALLBACK_LAST])();
short int CommOamHeadBuild ( 
                      unsigned short  OltId, 
                      unsigned short  OnuId,
                      unsigned char   *pSentData,  
                      unsigned short  *pSentDataSize);
short int CommOamPadOuiInsert  (unsigned char *pOamFrame);
short int CommOamPadOpcodeInsert  (unsigned char *pOamFrame, unsigned char OpCode);
short int CommOamPadSendsernoInsert  (unsigned char *pOamFrame, unsigned long SendSerNo);
short int CommOamPadWholepktlenInsert  (unsigned char *pOamFrame, unsigned short int WholePktLen);
short int CommOamPadPayloadoffsetInsert  (unsigned char *pOamFrame, unsigned short int PayLoadOffSet);
short int CommOamPadPayloadlenInsert  (unsigned char *pOamFrame, unsigned short int PayLoadLen);
short int CommOamSaGet(short int PonPortIdx, Oam_mac_address_t    SourceMacAddr);
short int CommOamPadSendsernoGet  (unsigned long *pSendSerNo);
int CommOamOpcodeFromOamGet(unsigned char *pGwOpCode,unsigned char *pOam);
short int CommOamPadOuiFromOamGet  (unsigned int *pOamOui,unsigned char *pOamFrame);
short int CommOamPadWholepktlenFromOamGet  (unsigned short *pOamWholeLen, unsigned char *pOamFrame);
short int CommOamPadPayloadoffsetFromOamGet  (unsigned char *pOamFrame, unsigned short int *pPayLoadOffSet);
short int CommOamPadPayloadlenFromOamGet (unsigned char *pOamFrame, unsigned short int *pPayLoadLen);
int CommOamOpcodeCheck(unsigned char gw_opcode);
extern int Oam_Session_RecieveCallBack(
    short int PonPortIdx, 
    short int OnuId, 
    short int llid, 
    short int len, 
    unsigned char *pDataBuf, 
    unsigned char  *pSessionField);

/*short int CommOamSendDelay(unsigned short ticks);*/

short int CommOltMsgSendsernoCheck(
				const unsigned short      OltId, 
				const unsigned short      OnuId,
				const unsigned int 		sendserno,
				/*GW_OAM_PAD_layer_msg_t *pOamInfo,*/
				unsigned short *pQueueId);


short int CommOltMsgEmptyQueueGet(
				const unsigned short      OltId, 
				const unsigned short      OnuId,
				unsigned short *pQueueId);

int CommOltMsgQueueFree(unsigned short OltId, unsigned short OnuId,unsigned short QueueId);

/*int CommOltMsgQueueInfoGet(unsigned short OltId, 
								unsigned short OnuId,
								unsigned short QueueId, 
								unsigned char **ppRevBuf, 
								unsigned char **pSessionField,
								unsigned short *pWhlLength,
								unsigned short *pPayloadlen);*/

int CommOltMsgQueueSave(unsigned short OltId, 
								unsigned short OnuId,
								unsigned short QueueId, 
								GW_OAM_PAD_layer_msg_t *pOamInfo
								/*unsigned char  *pOamFrame*/);

int CommOltMsgSemTake(unsigned short OltId, unsigned short OnuId, unsigned short QueueId);

int CommOltMsgSemGive(unsigned short OltId, unsigned short OnuId, unsigned short QueueId);


short int CommOltMsgReveive(
				const unsigned short      OltId, 
				const unsigned short      OnuId,
				/*unsigned char OpCode,*/
				GW_OAM_PAD_layer_msg_t *pOamInfo
				/*unsigned char *pOamFrame*/);

int CommOltMsgCallback(  
                                  const short int        OltId,
                                  const unsigned short   OnuId,
                                  const short int        llid,
                                  const unsigned char GwProId,
                                  const unsigned short   Length,
                                  unsigned char   *pFrame,
                                  unsigned char  *pSessionField);

void CommHadleReserved(const unsigned short   PonId,
                               const unsigned short	OnuId,
                               const unsigned short	llid,
                               const unsigned char 	GwProId,
                               const unsigned short   Length,
                               const unsigned char	*pFrame,
                               const unsigned char	*pSessionField);

short int CommOnuMsgEmptyQueueGet(
				const unsigned short      OltId, 
				const unsigned short      OnuId,
				unsigned short *pQueueId);
short int CommOnuMsgSendsernoCheck(
				const unsigned short      OltId, 
				const unsigned short      OnuId,
				GW_OAM_PAD_layer_msg_t *pOamInfo,
				unsigned short *pQueueId);

int CommOnuMsgQueueFree(unsigned short OltId, unsigned short OnuId,unsigned short QueueId);

int CommOnuMsgQueueInfoGet(unsigned short OltId, 
								unsigned short OnuId,
								unsigned short QueueId, 
								unsigned char **ppRevBuf, 
								unsigned short *pPayloadlen);
int CommOnuMsgSave(unsigned short OltId, 
								unsigned short OnuId,
								unsigned short QueueId, 
								GW_OAM_PAD_layer_msg_t *pOamInfo, 
								unsigned char  *pOamFrame);
int CommOnuMsgSemTake(unsigned short OltId, unsigned short OnuId, unsigned short QueueId);

int CommOnuMsgSemGive(unsigned short OltId, unsigned short OnuId, unsigned short QueueId);

short int CommOnuMsgReveive(
				const unsigned short      OltId, 
				const unsigned short      OnuId,
				GW_OAM_PAD_layer_msg_t *pOamInfo,
				unsigned char *pOamFrame);

int CommOnuMsgCallback(  
                                  const short int        OltId,
                                  const unsigned short     OnuId,
                                  const unsigned char GwOpCode,
                                  const unsigned short   Length,
                                  unsigned char   *pFrame);




unsigned short CommOnuMsgWdogInfoInit(
				unsigned short      OltId, 
				unsigned short      OnuId,
				unsigned short QueueId);
/*short int CommOnuMsgWdogTimeOutPro(int  wdInfoNos);*/

short int CommOamPayloadReveice ( 
                                    const short int        OltId,
                                    const unsigned short   OnuId,
                                    const short int        llid,
                                    const unsigned short   length,
                                    unsigned char   *content);
short int CommOamFrameReveive(  
                                  const short int        OltId,
                                  const unsigned short     OnuId,
                                  const short int          llid,
                                  const unsigned short   length,
                                  unsigned char   *content);
short int CommEhtFrameReveive (
                                 const short int                 OltId,
                                 const short int                 OnuId,
                                 const short int                 llid,
                                 const unsigned short            length,
                                 void                     *content );
short int COMM_OAM_frame_send  ( 
                      const unsigned short      OltId, 
                      const unsigned short     OnuId,
                      /*unsigned int		queuePro,*/
                      unsigned char oam_opcode,
                      const short int broadcast_llid,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size,
                      unsigned char	*pSessionIdfield,
                      unsigned short devType,
                      unsigned long sendserno );
/*short int CommOnuMagInfoInit(void);*/
#if 0    /* deleted by chenfj 2008-7-3 */
/*=====================文件传输接口定义=========================*/

int Comm_file_read_request_transmit(
				const unsigned short OltId, 
				const unsigned short OnuId,
				const short int broadcast_llid,
				void *pFilepbuf,
				const unsigned short File_data_size);
int Comm_file_write_request_transmit(
				const unsigned short OltId, 
				const unsigned short OnuId,
				const short int broadcast_llid,
				void *pFilepbuf,
				const unsigned short File_data_size);
int Comm_file_data_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char  *pfileBuf,
				const unsigned short fileBufSize,
				unsigned char *psessionIdField);

int Comm_file_transfer_ack_transmit(void *pClibuf,const unsigned short  cli_data_size);


/*============for test============*/
/*int oam_frame_build(int Tempsize,int oamsize);
int oam_debug_frame_printf(short int OltId, short int onuId, int oam_frame_length, unsigned char *OamFrame);
int oam_debug_Reveice_fram(short int OltId, short int OnuId, int oam_frame_length, unsigned char *OamFrame,unsigned int DevType);*/

/*===============文件传输接口定义================*/
int File_OAM_frame_send  ( 
                      const unsigned short      OltId, 
                      const unsigned short      OnuId,
                      unsigned long		lSendSerNo,
                      /*unsigned int		queuePro,*/
                      unsigned char oam_opcode,
                      const short int broadcast_llid,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size);
short int FileCtrlDataTransmmit( 
                      const unsigned short      OltId, 
                      const unsigned short      OnuId,
                      /*unsigned int		queuePro,*/
                      unsigned char oam_opcode,
                      const short int broadcast_llid,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size);
int FileCtrlDataTransHeadBuild(unsigned char *pBuf,
								unsigned int fileLen, 
								unsigned int fileOff, 
								unsigned int dataLen, 
								unsigned int datafcs);
short int FileCtrlTrsRspParse(unsigned short oltid, 
							unsigned short onuId, 
							const unsigned char GwOpCode, 
							unsigned long len, 
							unsigned char *pBuf,
							unsigned short *pAckCode,
							unsigned short *pErrcode);
short int CommFileTransferAck(	const unsigned short OltId, 
							const unsigned short OnuId,
							unsigned int ackCode,
							unsigned int ErrCode,
							unsigned int fileLen,
							unsigned int fileOff,
							unsigned int dataLen,
							unsigned int datafcs );
short int FileCtrlDataTransRspTickTimeInit(void);
short int FileCtrlDataTransRspTickTimeRun(unsigned short OltId, unsigned short OnuId, GW_OAM_FILECTR_t *pstrFileCtrl);
short int FileCtrlDataRev(void);
short int FileCtrlDataBufCheck(unsigned short OltId, unsigned short OnuId);
/*==================发送和接收缓冲模块=====================*/
#endif
BOOL OAMIOClearQueue( unsigned char aqueIdx,unsigned int datIdx);
unsigned int  OamIOWriteDataToQueue( unsigned char aM_uint8_QueIdx, 
										UCHAR *aP_uchar_Buff,
										unsigned int aM_uint32_Len);
short int OamIODataSend(unsigned short OltId, 
						unsigned short OnuId, 
						unsigned char *pBuf,
						unsigned int msgType,
						UINT32 bufLen,
						GW_OAM_MSG_t  *pOamMsgStru);
BOOL OamIOMsgQueueInit(void/*unsigned short maxBuf*/);
/*   deleted by chenfj 2008-7-3
short int FileCtrlDataBufFree(unsigned short OltId, unsigned short OnuId);
short int FileCtrlDataBufMalloc(
						unsigned short OltId, 
						unsigned short OnuId, 
						unsigned char *pfileName,  
						unsigned int fileNameLen, 
						unsigned int fileLens);
*/
/*=====for test====*/

/*short int OamIODataToRevQueue(unsigned short OltId, 
						unsigned short OnuId, 
						unsigned char *pBuf,
						unsigned int msgType,
						unsigned int  bufLen);

short int ONU_OAM_frame_send  ( 
                      const unsigned short      OltId, 
                      const unsigned short     OnuId,
                      unsigned int		queuePro,
                      unsigned char oam_opcode,
                      const short int broadcast_llid,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size);
short int OnuIODataToRevQueue(unsigned short OltId, 
						unsigned short OnuId, 
						unsigned char *pBuf,
						unsigned int msgType,
						unsigned int  bufLen);*/


/*=======for test========*/
/*void oamRev(short int OltId,
				short int OnuId,
				short int Length,
				char *pFrame);*/
void CommSendReserved(const unsigned short   PonId,
                               const unsigned short	OnuId,
                               const unsigned char 	GwProId,
                               const unsigned short   Length,
                               const unsigned char	*pFrame,
                               const unsigned char	*pSessionField);
#if FUNC_IMPELMENT
#endif
/****************************************************
*  CommOamHeadBuild
* 描述: 该函数封装oam的头部
* 输入参数:OltId,OnuId,
* 				pSentData - 要发送的数据缓冲区
*				pSentDataSize - 当前数据长度
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamHeadBuild ( 
                      unsigned short  OltId, 
                      unsigned short  OnuId,
                      unsigned char   *pSentData,  
                      unsigned short  *pSentDataSize)
{
    short int               result = OAM_ERR;
    /*unsigned short          oam_data_size = (unsigned short)(GW_OAM_HEADER_SIZE);*/
    Oam_mac_address_t    SourceMacAddr;

	if( (pSentData == NULL) || (pSentDataSize == NULL) )	/* modified by xieshl 20090916 */
	{
		VOS_SysLog(LOG_TYPE_DEV_CTRL, LOG_ALERT, "OAM: send data is null\r\n");
		return result;
	}
	
    /*insert destination address*/
    VOS_MemCpy( (unsigned char *)(pSentData+GW_OAM_DA), GW_oam_destination_address, GW_BYTES_MAC_ADDRESS);
    result = CommOamSaGet(OltId, SourceMacAddr);
	if (result != OAM_OK)
		return result;
    /* insert source mac address */	
    VOS_MemCpy( (unsigned char *)(pSentData+GW_OAM_SA), SourceMacAddr, GW_BYTES_MAC_ADDRESS);
    /* insert type */

    pSentData[GW_OAM_ETH_TYPE] = (unsigned char)((gGwEthernetType >> OAM_BITS_IN_BYTE) & 0xff);
    pSentData[GW_OAM_ETH_TYPE+1] = (unsigned char)((gGwEthernetType  & 0xff));
    /* Build OAM header */
    /* insert sub type */
    pSentData[GW_OAM_SUBTYPE] =  gGgwOamSubType;

    /* insert flags */
    pSentData[GW_OAM_FLAGS] = (unsigned char)((gGwOamFlags >> OAM_BITS_IN_BYTE) & 0xff);
    pSentData[GW_OAM_FLAGS+1] = (unsigned char)((gGwOamFlags & 0xff));
    /* insert code */
    pSentData[GW_OAM_CODE] =  gGwOamCode;
    *pSentDataSize = GW_OAM_HEADER_SIZE;


    return result;
}


/****************************************************
*  CommOamPadOuiInsert
* 描述: 该函数往oam的pad的oui插入gw的oui值
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamPadOuiInsert  (unsigned char *pOamFrame)
{
	if(pOamFrame == NULL)
		return OAM_NULL_POINTER ;
    /* insert Passave oui */
	pOamFrame[GW_OAM_PAD_OUI]     = ((GW_OUI_DEFINE >>(OAM_BYTES_IN_WORD*OAM_BITS_IN_BYTE))& 0xff);
	pOamFrame[GW_OAM_PAD_OUI+1]   = (unsigned char)((GW_OUI_DEFINE >>OAM_BITS_IN_BYTE)& 0xff);
	pOamFrame[GW_OAM_PAD_OUI+2]   = (unsigned char)(GW_OUI_DEFINE & 0xff);
	return OAM_OK;	
}

/****************************************************
* CommOamPadOpcodeInsert
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamPadOpcodeInsert  (unsigned char *pOamFrame, unsigned char OpCode)
{
	if(pOamFrame == NULL)
		return OAM_NULL_POINTER ;
	pOamFrame[GW_OAM_PAD_OPCODE]   = OpCode;
	return OAM_OK;
}

/****************************************************
* 
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamPadSendsernoInsert  (unsigned char *pOamFrame, unsigned long SendSerNo)
{
	if(pOamFrame == NULL)
		return OAM_NULL_POINTER ;
	pOamFrame[GW_OAM_PAD_SENDSERNO] = (unsigned char)((SendSerNo >>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD))& 0xff);
	pOamFrame[GW_OAM_PAD_SENDSERNO+1] = (unsigned char)(SendSerNo>>(OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD) & 0xff);
	pOamFrame[GW_OAM_PAD_SENDSERNO+2] = (unsigned char)(SendSerNo>>(OAM_BITS_IN_BYTE)& 0xff);
	pOamFrame[GW_OAM_PAD_SENDSERNO+3] = (unsigned char)(SendSerNo& 0xff);
	return OAM_OK;
}


/****************************************************
* 
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/	
short int CommOamPadWholepktlenInsert  (unsigned char *pOamFrame, unsigned short int WholePktLen)
{
	if(pOamFrame == NULL)
		return OAM_NULL_POINTER ;
	pOamFrame[GW_OAM_PAD_WHOLEPKTLEN] = (unsigned char)((WholePktLen >>OAM_BITS_IN_BYTE)& 0xff);
	pOamFrame[GW_OAM_PAD_WHOLEPKTLEN+1] = (unsigned char)(WholePktLen & 0xff);
	return OAM_OK;
}


/****************************************************
* 
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamPadPayloadoffsetInsert  (unsigned char *pOamFrame, unsigned short int PayLoadOffSet)
{
	if(pOamFrame == NULL)
		return OAM_NULL_POINTER ;
	pOamFrame[GW_OAM_PAD_PAYLOADOFFSET] = (unsigned char)((PayLoadOffSet >>OAM_BITS_IN_BYTE)& 0xff);
	pOamFrame[GW_OAM_PAD_PAYLOADOFFSET+1] = (unsigned char)(PayLoadOffSet & 0xff);
	return OAM_OK;
}
/****************************************************
* 
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamPadPaySessionIdnsert  (unsigned char *pOamFrame, unsigned char *pSessionIdField)
{
	if(pOamFrame == NULL)
		return OAM_NULL_POINTER ;
	if(pSessionIdField == NULL)
		return OAM_NULL_POINTER ;	
	memcpy(pOamFrame+GW_OAM_PAD_LENGTH+GW_OAM_HEADER_SIZE, pSessionIdField, GW_OAM_SESSIONID);
	return OAM_OK;
}



/****************************************************
* 
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamPadPayloadlenInsert  (unsigned char *pOamFrame, unsigned short int PayLoadLen)
{
	if(pOamFrame == NULL)
		return OAM_NULL_POINTER ;
	pOamFrame[GW_OAM_PAD_PAYLOADLENGTH] = (unsigned char)((PayLoadLen >>OAM_BITS_IN_BYTE)& 0xff);
	pOamFrame[GW_OAM_PAD_PAYLOADLENGTH+1] = (unsigned char)(PayLoadLen & 0xff);
	return OAM_OK;	
}

#if 0
#endif
/****************************************************
* 
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
/*extern int *GetPonChipMgmtPathMac(short int PonPortIdx );*/
short int CommOamSaGet(short int PonPortIdx, Oam_mac_address_t    SourceMacAddr)
{
	if((PonPortIdx < 0 ) || (PonPortIdx >= GW_MAX_OLTID )) return( OAM_ERR );
	
	if( SourceMacAddr == NULL ) return( OAM_ERR );
	
	VOS_MemCpy(SourceMacAddr, (void *)GetPonChipMgmtPathMac(PonPortIdx ), BYTES_IN_MAC_ADDRESS);
	/*Oam_mac_address_t    MacAddr = {0x1,0x2,0x3,0x4,0x5,0x6};
	memcpy(SourceMacAddr,MacAddr,6);*/
	return (OAM_OK);
}


#if 0
/***************************************************
* CommOamHeadCodeFromOamGet
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
***************************************************/
short int CommOamHeadCodeFromOamGet(unsigned char *pCode,unsigned char *pOam)
{
	*pCode = pOam[GW_OAM_CODE];
	return OAM_OK;
}


/***************************************************
*
*
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
***************************************************/
short int CommOamHeadFlagsFromOamGet(unsigned char *pFlags,unsigned char *pOam)
{
	*pFlags = (pOam[GW_OAM_FLAGS]<<OAM_BITS_IN_BYTE) +pOam[GW_OAM_FLAGS+1];
	return OAM_OK;
}


/***************************************************
*
*
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
***************************************************/
short int CommOamHeadSubTyepFromOamGet(unsigned char *pSubType,unsigned char *pOam)
{
	*pSubType = pOam[GW_OAM_SUBTYPE];
	return OAM_OK;
}

#endif

/****************************************************
*  CommOamOpcodeGet
* 描述: 从GW oam帧的opcode域中获取,并进行合法性检查
* 输入参数:
* 			gw_opcode - gw oam帧的opcode值
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
int CommOamOpcodeFromOamGet(unsigned char *pGwOpCode,unsigned char *pOam)
{
	unsigned char gw_opcode = 0;
	int iRes = OAM_OK;
	if( (pOam == NULL) || (pGwOpCode == NULL) )
	{
		VOS_SysLog(LOG_TYPE_DEV_CTRL, LOG_ALERT, "OAM: opcode is null\r\n");
		return OAM_NULL_POINTER ;
	}
	*pGwOpCode = pOam[GW_OAM_PAD_OPCODE];
	gw_opcode = *pGwOpCode;
	iRes = CommOamOpcodeCheck(gw_opcode);
	return (iRes);
}


/****************************************************
*  CommOamPadOuiFromOamGet
* 描述: 
* 输入参数:pOamFrame
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamPadOuiFromOamGet  (unsigned int *pOamOui,unsigned char *pOamFrame)
{
	unsigned int temp = 0;
	if( (pOamFrame == NULL) || (pOamOui == NULL) )
	{
		VOS_SysLog(LOG_TYPE_DEV_CTRL, LOG_ALERT, "OAM: oui is null\r\n");
		return OAM_NULL_POINTER ;
	}

       /* insert Passave oui */
	temp = ((unsigned int)(pOamFrame[GW_OAM_PAD_OUI])) 	<<(OAM_BYTES_IN_WORD*OAM_BITS_IN_BYTE);
	*pOamOui = temp;
	temp = ((unsigned int)(pOamFrame[GW_OAM_PAD_OUI+1])) <<OAM_BITS_IN_BYTE;
	*pOamOui += temp;
	temp = (unsigned int)(pOamFrame[GW_OAM_PAD_OUI+2]);
	*pOamOui += temp;
	/**pOamOui = ((unsigned int)(pOamFrame[GW_OAM_PAD_OUI])) <<(OAM_BYTES_IN_WORD*OAM_BITS_IN_BYTE) + 
						(((unsigned int)(pOamFrame[GW_OAM_PAD_OUI+1])) <<OAM_BITS_IN_BYTE) +
						(unsigned int)(pOamFrame[GW_OAM_PAD_OUI+2]);*/
	   
	return OAM_OK;	
}


/****************************************************
* CommOamPadWholepktlenFromOamGet
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/	
short int CommOamPadWholepktlenFromOamGet  (unsigned short *pOamWholeLen, unsigned char *pOamFrame)
{
	if( (pOamFrame == NULL) || (pOamWholeLen == NULL) )
		return OAM_NULL_POINTER ;
	*pOamWholeLen = (pOamFrame[GW_OAM_PAD_WHOLEPKTLEN]<<OAM_BITS_IN_BYTE)+pOamFrame[GW_OAM_PAD_WHOLEPKTLEN+1] ;
	return OAM_OK;
}

/****************************************************
* CommOamPadPayloadoffsetFromOamGet
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamPadPayloadoffsetFromOamGet  (unsigned char *pOamFrame, unsigned short int *pPayLoadOffSet)
{
	if( (pOamFrame == NULL) || (pPayLoadOffSet == NULL) )
		return OAM_NULL_POINTER ;
	*pPayLoadOffSet = (pOamFrame[GW_OAM_PAD_PAYLOADOFFSET]<<OAM_BITS_IN_BYTE)+pOamFrame[GW_OAM_PAD_PAYLOADOFFSET+1];
	return OAM_OK;
}


/****************************************************
* 
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamPadPayloadlenFromOamGet (unsigned char *pOamFrame, unsigned short int *pPayLoadLen)
{
	if( (pOamFrame == NULL) || (pPayLoadLen == NULL) )
		return OAM_NULL_POINTER ;
	*pPayLoadLen = (pOamFrame[GW_OAM_PAD_PAYLOADLENGTH]<<OAM_BITS_IN_BYTE) + pOamFrame[GW_OAM_PAD_PAYLOADLENGTH+1];
	return OAM_OK;	
}

/****************************************************
* 
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamPadSendsernoFromOamGet (unsigned char *pOamFrame, unsigned long *pSendSerNo)
{
	if( (pOamFrame == NULL) || (pSendSerNo == NULL) )
	{
		VOS_SysLog(LOG_TYPE_DEV_CTRL, LOG_ALERT, "OAM: sn is null\r\n");
		return OAM_NULL_POINTER ;
	}
	*pSendSerNo= (unsigned long)(((unsigned long)pOamFrame[GW_OAM_PAD_SENDSERNO]) <<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD))+ \
							((unsigned long)(pOamFrame[GW_OAM_PAD_SENDSERNO+1]) <<(OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD)) + \
							((unsigned long)(pOamFrame[GW_OAM_PAD_SENDSERNO+2])<<(OAM_BITS_IN_BYTE)) + pOamFrame[GW_OAM_PAD_SENDSERNO+3];
	return OAM_OK;
}

#if 0






/****************************************************
*  CommOamDataSendDelayTickSet
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamDataSendDelayTickSet  (unsigned int delayTick)
{
	gTaskDelayTime = delayTick;
	return OAM_OK;

}

/****************************************************
*  CommOamDataSendDelayTickGet
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamDataSendDelayTickGet  (unsigned int *pdelayTick)
{
	if (NULL == pdelayTick)
		return OAM_ERR;
	*pdelayTick = gTaskDelayTime;
	return OAM_OK;

}
#endif

/****************************************************
*  CommOamPadSendsernoGet
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamPadSendsernoGet  (unsigned long *pSendSerNo)
{
	if(pSendSerNo == NULL)
		return OAM_NULL_POINTER ;	

	if (VOS_OK != VOS_SemTake(gSendNoSemId, WAIT_FOREVER))
		return (OAM_ERR);

	*pSendSerNo = gSendSerNo;
	gSendSerNo ++;
	VOS_SemGive(gSendNoSemId);

	return OAM_OK;
}

/****************************************************
*  CommOamPadSendsernoInit
* 描述: 
* 输入参数:
* 
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
short int CommOamPadSendsernoInit  (void)
{
	gSendSerNo = 0;
	return OAM_OK;
}


/****************************************************
*  CommOamOpcodeCheck
* 描述: 对GW oam帧的opcode域进行合法性检查
* 输入参数:
* 			gw_opcode - gw oam帧的opcode值
*
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
int CommOamOpcodeCheck(unsigned char gw_opcode)
{
	/* modified by chenfj 2008-7-3 */
	/*int result = OAM_OK;*/
	if((gw_opcode <= GW_OPCODE_RESERVED) ||(gw_opcode >= GW_OPCODE_LAST))
		return(OAM_OPCODE_INVALUE_ERR);
	else return( OAM_OK );
#if 0	
    switch(gw_opcode)
    	{
		/*case GW_OPCODE_RESERVED	:
			break;*/
		case GW_OPCODE_EUQ_INFO_REQUESET:
			break;
		case GW_OPCODE_EUQ_INFO_RESPONSE:
			break;
		case GW_OPCODE_ALARM_OR_LOG_TRAN:
			break;
		case GW_OPCODE_ALARM_OR_LOG_RESPONSE:
			break;
		case GW_OPCODE_FILE_READ_AND_WRITE_REQUEST:
			break;
		/*case GW_OPCODE_FILE_RESERVED_0:
			break;*/
		case GW_OPCODE_FILE_DATA_TRAN:
			break;
		case GW_OPCODE_FILE_TRANSFER_ACK:
			break;
		/*case GW_OPCODE_FILE_RESERVED_1:
			break;
		case GW_OPCODE_FILE_RESERVED_2:
			break;*/
		case GW_OPCODE_SNMP_REQUEST:
			break;
		case GW_OPCODE_SNMP_RESPONSE:
			break;
		case GW_OPCODE_SNMP_TRAP:
			break;
		/*case GW_OPCODE_SNMP_RESERVED_1:
			break;
		case GW_OPCODE_SNMP_RESERVED_2:
			break;*/
		case GW_OPCODE_CLI_REQUEEST:
			break;
		case GW_OPCODE_CLI_RESPONSE:
			break;
		case GW_OPCODE_IGMPAUTH_REQUEST:
			break;
		case Gw_OPCODE_IGMPAUTH_REPONSE:
			break;
		case Gw_OPCODE_OAM_DEBUG:
			break;	
		case GW_OPCODE_VCONSOLE:
			break;
	default:
		return (OAM_OPCODE_INVALUE_ERR);
    	}
	return (result);
#endif
}


/****************************************************
* CommOamHeaderBuild
* 描述: 该函数将要发送的oam帧封装以太网头
* 输入参数:
* 			OltId 
*			pSentData - 指向操作目标数据的指针
*
* 输出参数:无
*
* 返回值: 暂定
*
******************************************************/
#if 0
short int CommOamHeaderBuild ( const PON_olt_id_t   OltId, unsigned char       *pSentData )
{
    Oam_mac_address_t    SourceMacAddr;
    short int        result = OAM_OK;
  
    /*insert destination address*/
    memcpy( (unsigned char *)(pSentData+GW_OAM_DA), GW_oam_destination_address, GW_BYTES_MAC_ADDRESS);
    CommOamSaGet(&SourceMacAddr);
    /* insert source mac address */	
    memcpy( (unsigned char *)(pSentData+GW_OAM_SA), SourceMacAddr, GW_BYTES_MAC_ADDRESS);
    /* insert type */
    pSentData[GW_OAM_ETH_TYPE] = (unsigned char)((gGwEthernetType >> BITS_IN_BYTE) & 0xff);
    pSentData[GW_OAM_ETH_TYPE+1] = (unsigned char)((gGwEthernetType  & 0xff));
    return result;	
}
#endif


/*GwProId = 
#define GW_DEBUG_RESERVED					0x0
#define GW_DEBUG_CLI						0x1
#define GW_DEBUG_SNMP						0x2
#define GW_DEBUG_IGMPAUTH					0x3
#define GW_DEBUG_EUQINFO					0x4
#define GW_DEBUG_ALARMORLOG					0x5
#define GW_DEBUG_FILE						0x6
#define GW_DEBUG_SNMP_TRAP					0x8
#define GW_DEBUG_ALL						0x7
*/

int CommOamTaskTestingInfoGet(unsigned int *pSendCount, unsigned int *pRevCount)
{
	if( (NULL == pSendCount) || (NULL == pRevCount) )
		return OAM_ERR;

	*pSendCount = debugSendCount;
	*pRevCount = debugRevCount;
	return OAM_OK;
}
#if 0
int CommOamTastQueueBufIinfo(void)
{
     UINT32            lM_uint32_Idx=0;
	 UINT32            lM_uint32_Cir=0;
	 unsigned char aM_uint8_QueIdx = 0;
	 sys_console_printf("\r\n  Oam task sending buf info(Num:%d): \r\n",gOamQueueCtl[aM_uint8_QueIdx].MaxSize);
     /*lM_uint32_Idx=gOamQueueCtl[aM_uint8_QueIdx].MaxSize;	*/
     for(lM_uint32_Cir=0;lM_uint32_Cir<gOamQueueCtl[aM_uint8_QueIdx].MaxSize;lM_uint32_Cir++)
	 {
	 	/*当为真表示该缓冲区已经被占用*/
		 if(gOamQueueCtl[aM_uint8_QueIdx].DataBuff[gOamQueueCtl[aM_uint8_QueIdx].WriteIndex].IsUsed)
		 {
		 	sys_console_printf("  %% The No.%d of sending Buff is TRUE(in using)!\r\n",gOamQueueCtl[aM_uint8_QueIdx].WriteIndex);
    		gOamQueueCtl[aM_uint8_QueIdx].WriteIndex++;

			if(gOamQueueCtl[aM_uint8_QueIdx].WriteIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            		{
				gOamQueueCtl[aM_uint8_QueIdx].WriteIndex=0;
			}
 		 }
		 else 
		 {
            lM_uint32_Idx=gOamQueueCtl[aM_uint8_QueIdx].WriteIndex;
			sys_console_printf("  QueueId %d is free\r\n",lM_uint32_Idx);
           	gOamQueueCtl[aM_uint8_QueIdx].WriteIndex++;
			if(gOamQueueCtl[aM_uint8_QueIdx].WriteIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            {
				gOamQueueCtl[aM_uint8_QueIdx].WriteIndex=0;
			}
		 }
	 }

	 sys_console_printf("\r\n  Oam task Reveiving buf info(Num:%d): \r\n",gOamQueueCtl[aM_uint8_QueIdx].MaxSize);
	 aM_uint8_QueIdx = 1;
     /*lM_uint32_Idx=gOamQueueCtl[aM_uint8_QueIdx].MaxSize;	*/
     for(lM_uint32_Cir=0;lM_uint32_Cir<gOamQueueCtl[aM_uint8_QueIdx].MaxSize;lM_uint32_Cir++)
	 {
	 	/*当为真表示该缓冲区已经被占用*/
		 if(gOamQueueCtl[aM_uint8_QueIdx].DataBuff[gOamQueueCtl[aM_uint8_QueIdx].WriteIndex].IsUsed)
		 {
		 	sys_console_printf(" %% The No.%d of receiving Buff is TRUE(in using)!\r\n",gOamQueueCtl[aM_uint8_QueIdx].WriteIndex);
    		gOamQueueCtl[aM_uint8_QueIdx].WriteIndex++;

			if(gOamQueueCtl[aM_uint8_QueIdx].WriteIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            		{
				gOamQueueCtl[aM_uint8_QueIdx].WriteIndex=0;
			}
 		 }
		 else 
		 {
            /*lM_uint32_Idx=gOamQueueCtl[aM_uint8_QueIdx].WriteIndex;*/
			sys_console_printf("  QueueId %d is free\r\n",lM_uint32_Idx);
           	gOamQueueCtl[aM_uint8_QueIdx].WriteIndex++;
			if(gOamQueueCtl[aM_uint8_QueIdx].WriteIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            {
				gOamQueueCtl[aM_uint8_QueIdx].WriteIndex=0;
			}
		 }
	 }
	 return OAM_OK;
}

int CommOamTastQueueInfoVty(struct vty *vty)
{
     /*UINT32            lM_uint32_Idx=0;*/
	 UINT32            lM_uint32_Cir=0;
	 unsigned char aM_uint8_QueIdx = 0;
	 vty_out( vty, "\r\n  Oam task sending buf info(Num:%d): \r\n",gOamQueueCtl[aM_uint8_QueIdx].MaxSize);
     /*lM_uint32_Idx=gOamQueueCtl[aM_uint8_QueIdx].MaxSize;	*/
     for(lM_uint32_Cir=0;lM_uint32_Cir<gOamQueueCtl[aM_uint8_QueIdx].MaxSize;lM_uint32_Cir++)
	 {
	 	/*当为真表示该缓冲区已经被占用*/
		 if(gOamQueueCtl[aM_uint8_QueIdx].DataBuff[gOamQueueCtl[aM_uint8_QueIdx].WriteIndex].IsUsed)
		 {
		 	vty_out( vty, "  %% The No.%d of sending Buff is TRUE(in using)!\r\n",gOamQueueCtl[aM_uint8_QueIdx].WriteIndex);
    		gOamQueueCtl[aM_uint8_QueIdx].WriteIndex++;

			if(gOamQueueCtl[aM_uint8_QueIdx].WriteIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            		{
				gOamQueueCtl[aM_uint8_QueIdx].WriteIndex=0;
			}
 		 }
		 else 
		 {
            /*lM_uint32_Idx=gOamQueueCtl[aM_uint8_QueIdx].WriteIndex;*/
			/*vty_out( vty, "  lM_uint32_Idx(%d) is free\r\n",lM_uint32_Idx);	*/	
           	gOamQueueCtl[aM_uint8_QueIdx].WriteIndex++;
			if(gOamQueueCtl[aM_uint8_QueIdx].WriteIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            {
				gOamQueueCtl[aM_uint8_QueIdx].WriteIndex=0;
			}
		 }
	 }

	 vty_out( vty, "\r\n  Oam task Reveiving buf info(Num:%d): \r\n",gOamQueueCtl[aM_uint8_QueIdx].MaxSize);
	 aM_uint8_QueIdx = 1;
     /*lM_uint32_Idx=gOamQueueCtl[aM_uint8_QueIdx].MaxSize;	*/
     for(lM_uint32_Cir=0;lM_uint32_Cir<gOamQueueCtl[aM_uint8_QueIdx].MaxSize;lM_uint32_Cir++)
	 {
	 	/*当为真表示该缓冲区已经被占用*/
		 if(gOamQueueCtl[aM_uint8_QueIdx].DataBuff[gOamQueueCtl[aM_uint8_QueIdx].WriteIndex].IsUsed)
		 {
		 	vty_out( vty, "  %% The No.%d of receiving Buff is TRUE(in using)!\r\n",gOamQueueCtl[aM_uint8_QueIdx].WriteIndex);
    		gOamQueueCtl[aM_uint8_QueIdx].WriteIndex++;

			if(gOamQueueCtl[aM_uint8_QueIdx].WriteIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            		{
				gOamQueueCtl[aM_uint8_QueIdx].WriteIndex=0;
			}
 		 }
		 else 
		 {
            /*lM_uint32_Idx=gOamQueueCtl[aM_uint8_QueIdx].WriteIndex;*/
			/*vty_out( vty, "  lM_uint32_Idx(%d) is free\r\n",lM_uint32_Idx);	*/	
           	gOamQueueCtl[aM_uint8_QueIdx].WriteIndex++;
			if(gOamQueueCtl[aM_uint8_QueIdx].WriteIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            {
				gOamQueueCtl[aM_uint8_QueIdx].WriteIndex=0;
			}
		 }
	 }
	return OAM_OK;
}

#endif

/*  modified by chenfj 2008-7-3
	这两个函数的实现太复杂,执行也有问题,下面编写代码重新实现*/
int CommOamTastQueueBufIinfo(void)
{
	UINT32            lM_uint32_Cir=0;
	unsigned char aM_uint8_QueIdx = 0;
	
	for(aM_uint8_QueIdx=0; aM_uint8_QueIdx<=1; aM_uint8_QueIdx++)
		{
		if(aM_uint8_QueIdx == 0)
			sys_console_printf("\r\n  Oam task sending buf info(MaxNum:%d): \r\n",gOamQueueCtl[aM_uint8_QueIdx].MaxSize);
		else 
			sys_console_printf("\r\n  Oam task Reveiving buf info(Num:%d): \r\n",gOamQueueCtl[aM_uint8_QueIdx].MaxSize);
		
		for(lM_uint32_Cir=0;lM_uint32_Cir<gOamQueueCtl[aM_uint8_QueIdx].MaxSize;lM_uint32_Cir++)
			{
			if(gOamQueueCtl[aM_uint8_QueIdx].DataBuff[lM_uint32_Cir].IsUsed)
				{
				if(aM_uint8_QueIdx==0)
					sys_console_printf("  %% The No.%d of sending Buff is TRUE(in using)!\r\n",lM_uint32_Cir);
				else 
					sys_console_printf(" %% The No.%d of receiving Buff is TRUE(in using)!\r\n",lM_uint32_Cir);
				 }
			else 
				{
				sys_console_printf("  QueueId %d is g_free\r\n",lM_uint32_Cir);
				}
			}
		}
	return OAM_OK;
}

int CommOamTastQueueInfoVty(struct vty *vty)
{
	UINT32            lM_uint32_Cir=0;
	unsigned char aM_uint8_QueIdx = 0;
	int count_free= 0, count_using=0;
	unsigned char row=0;

	for(aM_uint8_QueIdx = 0; aM_uint8_QueIdx <=1; aM_uint8_QueIdx++)
		{
		count_free = 0;
		count_using = 0;
		row = 0;
		for(lM_uint32_Cir=0;lM_uint32_Cir<gOamQueueCtl[aM_uint8_QueIdx].MaxSize;lM_uint32_Cir++)
			{
			 if(gOamQueueCtl[aM_uint8_QueIdx].DataBuff[lM_uint32_Cir].IsUsed)
				{
				count_using++;    		
				}
			 else 
				{
				count_free++;           
				}
			}
		
		if( aM_uint8_QueIdx == 0 )
			vty_out( vty, "\r\nOam task sending buf info(All Num:%d): \r\n",gOamQueueCtl[aM_uint8_QueIdx].MaxSize);
		else
			vty_out( vty, "\r\nOam task Reveiving buf info(All Num:%d): \r\n",gOamQueueCtl[aM_uint8_QueIdx].MaxSize);

		vty_out(vty,"  g_free buf count:%d, using buf count:%d\r\n", count_free, count_using);
		
		if(count_using != 0 )
			{
			vty_out(vty,"  using buffer list");
			for(lM_uint32_Cir=0;lM_uint32_Cir<gOamQueueCtl[aM_uint8_QueIdx].MaxSize;lM_uint32_Cir++)
				{
				if(gOamQueueCtl[aM_uint8_QueIdx].DataBuff[lM_uint32_Cir].IsUsed)
					{
					if( row % 16 == 0 ) vty_out(vty,"\r\n  ");
					vty_out(vty,"%3d ",lM_uint32_Cir);
					row++;
					}
				}
			vty_out(vty,"\r\n");
			}
		}	
	return OAM_OK;
}


int CommOamTaskQueueClear(unsigned char aM_uint8_QueIdx,UINT32  lM_uint32_Idx)
{
     if(lM_uint32_Idx>OAM_TASK_QUEUE_NUM)
     {
	 	sys_console_printf("  QueueId Err! QueueId = %d\r\n",lM_uint32_Idx);
		return OAM_ERR;
     }

	if (0 == aM_uint8_QueIdx)
	{
	OAMIOClearQueue(aM_uint8_QueIdx,lM_uint32_Idx);
	}

	else if (1 == aM_uint8_QueIdx)
	{
	OAMIOClearQueue(aM_uint8_QueIdx,lM_uint32_Idx);
	}
	 return OAM_OK;
}


int CommOltMsgQueueInfo(short int PonId)
{
	short int OnuId = 0;
	short int QueueId = 0;
	for (OnuId = 0; OnuId < GW_MAX_ONU; OnuId++)
	{
		for(QueueId = 0; QueueId < GW_MAX_QUEUEID; QueueId++)
		{
			if (goltMsg[PonId].onu_queue_t[OnuId].onumsg_info[QueueId].Iused == TRUE)
			{
			sys_console_printf("  PonId %d OnuId %d QueueId %d\r\n",PonId,OnuId,QueueId);
			sys_console_printf("  payloadlen = %d\r\n",goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength);
			sys_console_printf("  EXT_OAM_opcode = %d\r\n",goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].gw_opcode);
			sys_console_printf("  payloadoffset = %d\r\n",goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].payloadoffset);
			sys_console_printf("  sendserno = %d\r\n",goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].sendserno);
			sys_console_printf("  wholepktlen = %d\r\n",goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].wholepktlen);
			}
			{
		         unsigned short  i;
		         unsigned char *pBuf;
		         sys_console_printf("  the Queue len %d \r\n",  goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength );
		         pBuf = goltMsg[PonId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad;
				 sys_console_printf("  ----------------------------------\r\n");
		         for( i = 0; i< goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength ; i++ )
			 	 {
		             if( (i %20 )== 0 )
					  {
		                  sys_console_printf("\r\n");
		                  sys_console_printf("        ");
		              }
		              sys_console_printf(" %02x", *(unsigned char *)(pBuf+i) );                   
	              }
	    	      sys_console_printf("\r\n");				
			 }
		}	
	}
	 return OAM_OK;
}

int CommOltMsgQueueInfoVty(short int PonId,struct vty *vty)
{
	short int OnuId = 0;
	short int QueueId = 0;
	short int  i;
	unsigned char *pBuf;	
	for (OnuId = 0; OnuId < GW_MAX_ONU; OnuId++)
	{
		for(QueueId = 0; QueueId < GW_MAX_QUEUEID; QueueId++)
		{
			if (goltMsg[PonId].onu_queue_t[OnuId].onumsg_info[QueueId].Iused == TRUE)
			{
				vty_out( vty, "  PonId %d OnuId %d QueueId %d  is using, info:\r\n",PonId,OnuId,QueueId);
				vty_out( vty, "  payloadlen = %d\r\n",goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength);
				vty_out( vty, "  EXT_OAM_opcode = %d\r\n",goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].gw_opcode);
				vty_out( vty, "  payloadoffset = %d\r\n",goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].payloadoffset);
				vty_out( vty, "  sendserno = %d\r\n",goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].sendserno);
				vty_out( vty, "  wholepktlen = %d\r\n",goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].wholepktlen);

			    vty_out( vty, "  the Queue len %d \r\n",  goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength );
				/* modified by chenfj 2007-12-13
			    pBuf = (goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].pBuf);
			    */
			    pBuf = (goltMsg[PonId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad);
				vty_out( vty, "  ----------------------------------\r\n");
			    for( i = 0; i< goltMsg[PonId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength ; i++ )
				{
			      if( (i %20 )== 0 )
				  {
			         vty_out( vty, "\r\n");
			         vty_out( vty, "        ");
			      }
			      vty_out( vty, " %02x", *((unsigned char *)(pBuf+i)));                   
		        }
		    	vty_out( vty, "\r\n");				
			}
		}	
	}
	return 0;
}

int CommOltMsgQueueClear(short int PonId,short int OnuId)
{
	short int QueueId = 0;
	for(QueueId = 0; QueueId < GW_MAX_QUEUEID; QueueId++)
	{
		CommOltMsgQueueFree(PonId, OnuId,QueueId);		
	}	
	return 0;
}


int CommOltMsgRvcDebugTurnOn(unsigned char GwProId)
{

	switch(GwProId)
	{/*for debug funtion define*/
		case GW_DEBUG_RESERVED:
			sys_console_printf(" NULL\r\n");
			break;
		case GW_DEBUG_CLI:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_CLI_REQUEEST] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_CLI_REQUEEST] = 1;
			oam_debug_info[GW_OPCODE_CLI_RESPONSE] = 1;
			sys_console_printf(" CLI debug is turned on\r\n");
			break;	
		case GW_DEBUG_SNMP:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_SNMP_RESPONSE] !=1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_SNMP_RESPONSE]=1;
			oam_debug_info[GW_OPCODE_SNMP_REQUEST]=1;
			sys_console_printf("  SNMP debug is turned on\r\n");
			break;			
		case GW_DEBUG_IGMPAUTH:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_IGMPAUTH_REQUEST] !=1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_IGMPAUTH_REQUEST]=1;
			oam_debug_info[Gw_OPCODE_IGMPAUTH_REPONSE]=1;
			sys_console_printf("  IGMPAUTH debug is turned on\r\n");
			break;
		case GW_DEBUG_EUQINFO:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_EUQ_INFO_REQUESET] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_EUQ_INFO_REQUESET] = 1;
			oam_debug_info[GW_OPCODE_EUQ_INFO_RESPONSE] = 1;
			sys_console_printf("  EQU debug is turned on\r\n");
			break;
		case GW_DEBUG_ALARMORLOG:	
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_ALARM_OR_LOG_TRAN] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_ALARM_OR_LOG_TRAN] = 1;
			oam_debug_info[GW_OPCODE_ALARM_OR_LOG_RESPONSE] = 1;
			sys_console_printf("  ALARMORLOG debug is turned on\r\n");
			break;			
		case GW_DEBUG_FILE:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_FILE_READ_AND_WRITE_REQUEST] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_FILE_READ_AND_WRITE_REQUEST] = 1;
			oam_debug_info[GW_OPCODE_FILE_DATA_TRAN] = 1;
			oam_debug_info[GW_OPCODE_FILE_TRANSFER_ACK] = 1;
			sys_console_printf("  FILE debug is turned on\r\n");
			break;				
		case GW_DEBUG_SNMP_TRAP:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_SNMP_TRAP] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_SNMP_TRAP] = 1;
			sys_console_printf("  SNMP_TRAP debug is turned on\r\n");
			break;	
		case GW_DEBUG_VCONSOLE:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_VCONSOLE] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_VCONSOLE] = 1;
			sys_console_printf("  VCONSOLE debug is turned on\r\n");
			break;			
		case GW_DEBUG_ALL:
			{
				int iTemp = 0;
				for(iTemp = 1;iTemp < GW_OPCODE_LAST/*20*/;iTemp++)/*1-19分别对应opcode 的0－0x13*/
					oam_debug_info[iTemp] = 1;
				
				oam_debug_count = GW_DEBUG_ALL;
			}			
			sys_console_printf("  OAM all debug is turned on\r\n");
			break;

		default:
			sys_console_printf("  Error EXT_OAM_OpCode: %d\r\n",GwProId);
		}

	return 0;
}


int CommOltMsgRvcDebugTurnOnvty(unsigned char GwProId,struct vty *vty)
{

	switch(GwProId)
	{/*for debug funtion define*/
		case GW_DEBUG_RESERVED:
			vty_out( vty, " NULL\r\n");
			break;
		case GW_DEBUG_CLI:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_CLI_REQUEEST] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_CLI_REQUEEST] = 1;
			oam_debug_info[GW_OPCODE_CLI_RESPONSE] = 1;
			vty_out( vty, " CLI debug is turned on\r\n");
			break;	
		case GW_DEBUG_SNMP:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_SNMP_RESPONSE] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_SNMP_RESPONSE]=1;
			oam_debug_info[GW_OPCODE_SNMP_REQUEST]=1;
			vty_out( vty, "  SNMP debug is turned on\r\n");
			break;			
		case GW_DEBUG_IGMPAUTH:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_IGMPAUTH_REQUEST] !=1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_IGMPAUTH_REQUEST]=1;
			oam_debug_info[Gw_OPCODE_IGMPAUTH_REPONSE]=1;
			vty_out( vty, "  IGMPAUTH debug is turned on\r\n");
			break;
		case GW_DEBUG_EUQINFO:	
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_EUQ_INFO_REQUESET] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_EUQ_INFO_REQUESET] = 1;
			oam_debug_info[GW_OPCODE_EUQ_INFO_RESPONSE] = 1;
			vty_out( vty, "  EQU debug is turned on\r\n");
			break;
		case GW_DEBUG_ALARMORLOG:	
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_ALARM_OR_LOG_TRAN] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_ALARM_OR_LOG_TRAN] = 1;
			oam_debug_info[GW_OPCODE_ALARM_OR_LOG_RESPONSE] = 1;
			vty_out( vty, "  ALARMORLOG debug is turned on\r\n");
			break;			
		case GW_DEBUG_FILE:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_FILE_READ_AND_WRITE_REQUEST] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_FILE_READ_AND_WRITE_REQUEST] = 1;
			oam_debug_info[GW_OPCODE_FILE_DATA_TRAN] = 1;
			oam_debug_info[GW_OPCODE_FILE_TRANSFER_ACK] = 1;
			vty_out( vty, "  FILE debug is turned on\r\n");
			break;				
		case GW_DEBUG_SNMP_TRAP:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_SNMP_TRAP] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_SNMP_TRAP] = 1;
			vty_out( vty, "  SNMP_TRAP debug is turned on\r\n");
			break;	
		case GW_DEBUG_VCONSOLE:
			if((oam_debug_count < GW_DEBUG_ALL) && (oam_debug_info[GW_OPCODE_VCONSOLE] != 1))
				oam_debug_count++;
			oam_debug_info[GW_OPCODE_VCONSOLE] = 1;
			vty_out( vty, "  VCONSOLE debug is turned on\r\n");
			break;	
		case GW_DEBUG_ALL:
			{
				int iTemp = 0;
				for(iTemp = 1;iTemp < GW_OPCODE_LAST/*20*/;iTemp++)/*1-19分别对应opcode 的0－0x13*/
					oam_debug_info[iTemp] = 1;
				
				oam_debug_count = GW_DEBUG_ALL;
			}			
			vty_out( vty, "  OAM all debug is turned on\r\n");
			break;

		default:
			vty_out( vty, "  Error EXT_OAM_OpCode: %d\r\n",GwProId);
		}
	return 0;
}


int CommOltMsgRvcDebugTurnOff(unsigned char GwProId)
{
	switch(GwProId)
	{/*for debug funtion define*/
		case GW_DEBUG_RESERVED:
			sys_console_printf(" NULL\r\n");
			break;
		case GW_DEBUG_CLI:
			oam_debug_info[GW_OPCODE_CLI_REQUEEST] = 0;
			oam_debug_info[GW_OPCODE_CLI_RESPONSE] = 0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			sys_console_printf(" CLI debug is turned off\r\n");
			break;	
		case GW_DEBUG_SNMP:
			oam_debug_info[GW_OPCODE_SNMP_RESPONSE]=0;
			oam_debug_info[GW_OPCODE_SNMP_REQUEST]=0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			sys_console_printf("  SNMP debug is turned off\r\n");
			break;			
		case GW_DEBUG_IGMPAUTH:
			oam_debug_info[GW_OPCODE_IGMPAUTH_REQUEST]=0;
			oam_debug_info[Gw_OPCODE_IGMPAUTH_REPONSE]=0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			sys_console_printf("  IGMPAUTH debug is turned off\r\n");
			break;
		case GW_DEBUG_EUQINFO:		
			oam_debug_info[GW_OPCODE_EUQ_INFO_REQUESET] = 0;
			oam_debug_info[GW_OPCODE_EUQ_INFO_RESPONSE] = 0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			sys_console_printf("  EQU debug is turned off\r\n");
			break;
		case GW_DEBUG_ALARMORLOG:			
			oam_debug_info[GW_OPCODE_ALARM_OR_LOG_TRAN] = 0;
			oam_debug_info[GW_OPCODE_ALARM_OR_LOG_RESPONSE] = 0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			sys_console_printf("  ALARMORLOG debug is turned off\r\n");
			break;			
		case GW_DEBUG_FILE:			
			oam_debug_info[GW_OPCODE_FILE_READ_AND_WRITE_REQUEST] = 0;
			oam_debug_info[GW_OPCODE_FILE_DATA_TRAN] = 0;
			oam_debug_info[GW_OPCODE_FILE_TRANSFER_ACK] = 0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			sys_console_printf("  FILE debug is turned off\r\n");
			break;				
		case GW_DEBUG_SNMP_TRAP:
			oam_debug_info[GW_OPCODE_SNMP_TRAP] = 0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			sys_console_printf("  SNMP_TRAP debug is turned off\r\n");
			break;	
		case GW_DEBUG_VCONSOLE:
			oam_debug_info[GW_OPCODE_VCONSOLE] = 0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			sys_console_printf("  VCONSOLE debug is turned off\r\n");
			break;	
		case GW_DEBUG_ALL:
			{
				int iTemp = 0;
				for(iTemp = 1;iTemp < GW_OPCODE_LAST/*20*/;iTemp++)/*1-19分别对应opcode 的0－0x13*/
					oam_debug_info[iTemp] = 0;
				
				oam_debug_count = 0;
			}			
			sys_console_printf("  OAM all debug is turned off\r\n");
			break;

		default:
			sys_console_printf("  Error EXT_OAM_OpCode: %d\r\n",GwProId);
		}
	return 0;
}

int CommOltMsgRvcDebugTurnOffvty(unsigned char GwProId,struct vty *vty)
{

	switch(GwProId)
	{/*for debug funtion define*/
		case GW_DEBUG_RESERVED:
			vty_out( vty, " NULL\r\n");
			break;
		case GW_DEBUG_CLI:
			oam_debug_info[GW_OPCODE_CLI_REQUEEST] = 0;
			oam_debug_info[GW_OPCODE_CLI_RESPONSE] = 0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			vty_out( vty, " CLI debug is turned off\r\n");
			break;	
		case GW_DEBUG_SNMP:
			oam_debug_info[GW_OPCODE_SNMP_RESPONSE]=0;
			oam_debug_info[GW_OPCODE_SNMP_REQUEST]=0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			vty_out( vty, "  SNMP debug is turned off\r\n");
			break;			
		case GW_DEBUG_IGMPAUTH:
			oam_debug_info[GW_OPCODE_IGMPAUTH_REQUEST]=0;
			oam_debug_info[Gw_OPCODE_IGMPAUTH_REPONSE]=0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			vty_out( vty, "  IGMPAUTH debug is turned off\r\n");
			break;
		case GW_DEBUG_EUQINFO:		
			oam_debug_info[GW_OPCODE_EUQ_INFO_REQUESET] = 0;
			oam_debug_info[GW_OPCODE_EUQ_INFO_RESPONSE] = 0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			vty_out( vty, "  EQU debug is turned off\r\n");
			break;
		case GW_DEBUG_ALARMORLOG:			
			oam_debug_info[GW_OPCODE_ALARM_OR_LOG_TRAN] = 0;
			oam_debug_info[GW_OPCODE_ALARM_OR_LOG_RESPONSE] = 0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			vty_out( vty, "  ALARMORLOG debug is turned off\r\n");
			break;			
		case GW_DEBUG_FILE:			
			oam_debug_info[GW_OPCODE_FILE_READ_AND_WRITE_REQUEST] = 0;
			oam_debug_info[GW_OPCODE_FILE_DATA_TRAN] = 0;
			oam_debug_info[GW_OPCODE_FILE_TRANSFER_ACK] = 0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			vty_out( vty, "  FILE debug is turned off\r\n");
			break;				
		case GW_DEBUG_SNMP_TRAP:
			oam_debug_info[GW_OPCODE_SNMP_TRAP] = 0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			vty_out( vty, "  SNMP_TRAP debug is turned off\r\n");
			break;	
		case GW_DEBUG_VCONSOLE:
			oam_debug_info[GW_OPCODE_VCONSOLE] = 0;
			if (oam_debug_count > 0)
				oam_debug_count--;
			vty_out( vty, "  VCONSOLE debug is turned off\r\n");
			break;	
		case GW_DEBUG_ALL:
			{
				int iTemp = 0;
				for(iTemp = 1;iTemp < GW_OPCODE_LAST/*20*/;iTemp++)/*1-19分别对应opcode 的0－0x13*/
					oam_debug_info[iTemp] = 0;
				
				oam_debug_count = 0;
			}			
			vty_out( vty, "  OAM all debug is turned off\r\n");
			break;

		default:
			vty_out( vty, "  Error EXT_OAM_OpCode: %d\r\n",GwProId);
			
		}
	return 0;
}


int CommOltMsgSendingContentDebug(int Done)
{
	if ((Done != 0) && (Done != 1))
		return (-1);
	oam_debug_send = Done;
	
	return 0;
}

int CommOltMsgRvcDebugTurnShow(void)
{
	unsigned char gwDebug = 0;
	for(gwDebug = 1;gwDebug<=GW_DEBUG_ALL; gwDebug++)

	switch(gwDebug)
	{/*for debug funtion define*/
		case GW_DEBUG_ALL:
			if (GW_DEBUG_ALL == oam_debug_count)
				sys_console_printf("  OAM all debug is turned on:\r\n");
			break;
		case GW_DEBUG_RESERVED:
			sys_console_printf(" NULL\r\n");
			break;
		case GW_DEBUG_CLI:
			if (( 1 == oam_debug_info[GW_OPCODE_CLI_REQUEEST])
				||(1 == oam_debug_info[GW_OPCODE_CLI_RESPONSE]) )
			sys_console_printf(" CLI debug is turned on\r\n");
			break;	
		case GW_DEBUG_SNMP:
			if (( 1 == oam_debug_info[GW_OPCODE_SNMP_RESPONSE])
			||(1 == oam_debug_info[GW_OPCODE_SNMP_REQUEST]) )

			sys_console_printf("  SNMP debug is turned on\r\n");
			break;			
		case GW_DEBUG_IGMPAUTH:
			if (( 1 == oam_debug_info[GW_OPCODE_IGMPAUTH_REQUEST])
			||(1 == oam_debug_info[Gw_OPCODE_IGMPAUTH_REPONSE]))
			sys_console_printf("  IGMPAUTH debug is turned on\r\n");
			break;
		case GW_DEBUG_EUQINFO:		
			if (( 1 == oam_debug_info[GW_OPCODE_EUQ_INFO_REQUESET])
			||(1 == oam_debug_info[GW_OPCODE_EUQ_INFO_RESPONSE]))

			sys_console_printf("  EQU debug is turned on\r\n");
			break;
		case GW_DEBUG_ALARMORLOG:			
			if (( 1 == oam_debug_info[GW_OPCODE_ALARM_OR_LOG_TRAN])
			||(1 == oam_debug_info[GW_OPCODE_ALARM_OR_LOG_RESPONSE]))

			sys_console_printf("  ALARMORLOG debug is turned on\r\n");
			break;			
		case GW_DEBUG_FILE:			
			if (( 1 == oam_debug_info[GW_OPCODE_FILE_READ_AND_WRITE_REQUEST])
			||(1 == oam_debug_info[GW_OPCODE_FILE_DATA_TRAN])
			||(1 == oam_debug_info[GW_OPCODE_FILE_TRANSFER_ACK]))

			sys_console_printf("  FILE debug is turned on\r\n");
			break;				
		case GW_DEBUG_SNMP_TRAP:
			if ( 1 == oam_debug_info[GW_OPCODE_SNMP_TRAP])
				
			sys_console_printf("  SNMP_TRAP debug is turned on\r\n");
			break;	
		case  GW_DEBUG_VCONSOLE:
			if(1== oam_debug_info[GW_OPCODE_VCONSOLE])
				sys_console_printf("  VCONSOLE debug is turned on\r\n");
			break;
		default:
			sys_console_printf("  Unkowned OAM Debug type: %d\r\n",gwDebug);
		}
	return 0;
}


int CommOltMsgRvcDebugTurnVtyShow(struct vty *vty)
{
	unsigned char gwDebug = 0;
	for(gwDebug = 1;gwDebug<=GW_DEBUG_ALL; gwDebug++)

	switch(gwDebug)
	{/*for debug funtion define*/
		case GW_DEBUG_ALL:
			if (GW_DEBUG_ALL == oam_debug_count)
				vty_out( vty, "  OAM all debug is turned on:\r\n");
			break;
		case GW_DEBUG_RESERVED:
			vty_out( vty, " NULL\r\n");
			break;
		case GW_DEBUG_CLI:
			if (( 1 == oam_debug_info[GW_OPCODE_CLI_REQUEEST])
				||(1 == oam_debug_info[GW_OPCODE_CLI_RESPONSE]) )
			vty_out( vty, " CLI debug is turned on\r\n");
			break;	
		case GW_DEBUG_SNMP:
			if (( 1 == oam_debug_info[GW_OPCODE_SNMP_RESPONSE])
			||(1 == oam_debug_info[GW_OPCODE_SNMP_REQUEST]) )

			vty_out( vty, "  SNMP debug is turned on\r\n");
			break;			
		case GW_DEBUG_IGMPAUTH:
			if (( 1 == oam_debug_info[GW_OPCODE_IGMPAUTH_REQUEST])
			||(1 == oam_debug_info[Gw_OPCODE_IGMPAUTH_REPONSE]))
			vty_out( vty, "  IGMPAUTH debug is turned on\r\n");
			break;
		case GW_DEBUG_EUQINFO:		
			if (( 1 == oam_debug_info[GW_OPCODE_EUQ_INFO_REQUESET])
			||(1 == oam_debug_info[GW_OPCODE_EUQ_INFO_RESPONSE]))

			vty_out( vty, "  EQU debug is turned on\r\n");
			break;
		case GW_DEBUG_ALARMORLOG:			
			if (( 1 == oam_debug_info[GW_OPCODE_ALARM_OR_LOG_TRAN])
			||(1 == oam_debug_info[GW_OPCODE_ALARM_OR_LOG_RESPONSE]))

			vty_out( vty, "  ALARMORLOG debug is turned on\r\n");
			break;			
		case GW_DEBUG_FILE:			
			if (( 1 == oam_debug_info[GW_OPCODE_FILE_READ_AND_WRITE_REQUEST])
			||(1 == oam_debug_info[GW_OPCODE_FILE_DATA_TRAN])
			||(1 == oam_debug_info[GW_OPCODE_FILE_TRANSFER_ACK]))

			vty_out( vty, "  FILE debug is turned on\r\n");
			break;				
		case GW_DEBUG_SNMP_TRAP:
			if ( 1 == oam_debug_info[GW_OPCODE_SNMP_TRAP])
				
			vty_out( vty, "  SNMP_TRAP debug is turned on\r\n");
			break;	
		case  GW_DEBUG_VCONSOLE:
			if(1== oam_debug_info[GW_OPCODE_VCONSOLE])
				vty_out( vty, "  VCONSOLE debug is turned on\r\n");
			break;
		default:
			vty_out( vty, "  Unkowned OAM Debug type: %d\r\n",gwDebug);
		}
	
	if(oam_debug_send == 1)
		vty_out(vty,"  oam send is turned on\r\n");
	return 0;
}


 #if 0
#define GW_CALLBACK_RESERVED			0x0
#define GW_CALLBACK_EQU_REQUEST			0x01
#define GW_CALLBACK_EUQINFO				0x02/*GW_CALLBACK_EQU_RESPONE*/
#define GW_CALLBACK_ALARMORLOG			0x03/*request*/
#define GW_CALLBACK_ALARMORLOG_REPONSE	0x04
#define GW_CALLBACK_FILE_RD_WRT_REQUEST	0x05
#define GW_CALLBACK_FILE_RESERVED		0x06
#define GW_CALLBACK_FILE_DATA_TRA		0x07
#define GW_CALLBACK_FILE_TRA_ACK		0x08
#define GW_CALLBACK_SNMP_REQUEST		0x0b
#define GW_CALLBACK_SNMP_REPONSE		0x0c
#define GW_CALLBACK_SNMP_TRAP			0x0d
#define GW_CALLBACK_CLI_REQUEST			0x10
#define GW_CALLBACK_CLI					0x11/*cli reponse*/
#define GW_CALLBACK_IGMPAUTH_REQUEST	0x12
#define GW_CALLBACK_IGMPAUTH_REPONST	0x13




/*ponId : 0-19,onuId : 1-64*/
LONG OamOnuTypeCheck(ULONG PonId, ULONG onuId)
{
	ulong_t devIdx = 0;
	ulong_t devType = 0;
	ulong_t ulSlot = 0;
	ulong port = 0;
	LONG lRet = VOS_OK;
	LONG devTypeFlag = OAM_ERR;/**/
	
	if(( PonId < 4 ) && ( PonId >= 0 )) { ulSlot = 8; }
	else if(( PonId < 8 ) && ( PonId >= 4 )) { ulSlot = 7; }
	else if(( PonId < 12 ) && ( PonId >= 8 )) { ulSlot = 6; }
	else if(( PonId < 16 ) && ( PonId >= 12 )) { ulSlot = 5; }
	else if(( PonId < 20 ) && ( PonId >= 16 )) { ulSlot = 4; }
	else return OAM_ERR;

	port = ( PonId % 4 + 1 );
	
	devIdx = (ulong_t)( ulSlot*10000 + port*1000 + onuId);
	lRet = getDeviceType( devIdx, &devType );
	if(lRet != VOS_OK)
	{	
		return OAM_ERR;
	}

	switch(devType)
	{/*INTEGER  { other ( 1 ) , gfa6700 ( 2 ) , gfa6100 ( 3 ) , gt811 ( 4 ) , gt821 
	( 5 ) , gt831 ( 6 ) , gt812 ( 7 ) , gt813 ( 8 ) , gt881 ( 9 ) , gt861 ( 10 ) 
	, gt891 ( 11 ) } */
		case 1:
			return OAM_UNKOWNED_DEVTYPE_ERR;
		case 2:
			/*gfa6700*/
		case 3:
			/*6100*/
			return OAM_MIMATCH_DEVTYPE_ERR;
		case 4:/*gt811*/
		case 7:
			/*gt812*/	
			devTypeFlag = OAM_DAYAONU_DEVTYPE;
			break;
		case 5:
			/*gt821*/			
		case 6:
			/*gt831*/			
		case 8:
			/*gt813*/
		case 9:
			/*gt881*/				
		case 10:
			/*gt861*/
		case 11:
			/*gt891*/	
			devTypeFlag = OAM_OTHERONU_DEVTYPE;
		default:
			return OAM_UNKOWNED_DEVTYPE_ERR;
	}
	return devTypeFlag;

}
#endif






short int Comm_Oam_ErrParse(short int iRes)
{
	char *pErrStr;
	switch(iRes)
	{
		case OAM_TIME_OUT:
			pErrStr = "OAM_TIME_OUT EEROR";
			break;
		case OAM_INVALUEBUF_ERR:
			pErrStr = "OAM_INVALUEBUF_ERR EEROR";
			break;
		case OAM_NULL_POINTER:
			pErrStr = "OAM_NULL_POINTER EEROR";
			break;
		case OAM_OPCODE_INVALUE_ERR:
			pErrStr = "OAM_OPCODE_INVALUE_ERR EEROR";
			break;
		case OAM_NOT_INIT_ERR:
			pErrStr = "OAM_NOT_INIT_ERR EEROR";
			break;
		case OAM_CALLBACK_FULL_ERR:
			pErrStr = "OAM_TIME_OUT OAM_CALLBACK_FULL_ERR";
			break;
		case OAM_SEQUENCE_ERR:
			pErrStr = "OAM_SEQUENCE_ERR EEROR";
			break;
		case OAM_WHOLELENGTH_ERR:
			pErrStr = "OAM_WHOLELENGTH_ERR EEROR";
			break;
		case OAM_INVALUE_SENDSENO_ERR:
			pErrStr = "OAM_INVALUE_SENDSENO_ERREEROR";
			break;
		case OAM_MSGQSEND_ERR:
			pErrStr = "OAM_MSGQSEND_ERR EEROR";
			break;
		case OAM_INVALUE_QUEUE_ERR:
			pErrStr = "OAM_INVALUE_QUEUE_ERR EEROR";
			break;
		case OAM_INVALUE_MSGTYPE_ERR:
			pErrStr = "OAM_INVALUE_MSGTYPE_ERR EEROR";
			break;
		case OAM_SESSIONFIELD_ERR:
			pErrStr = "OAM_SESSIONFIELD_ERR EEROR";
			break;
		case OAM_ONU_DEREGISTER_ERR:
			pErrStr = "OAM_ONU_DEREGISTER_ERR EEROR";
			break;
		case OAM_MALLOC_ERR:
			pErrStr = "OAM_MALLOC_ERR EEROR";
			break;
		case OAM_TASK_MSGTYPE_UNKNOW_ERR:
			pErrStr = "OAM_TASK_MSGTYPE_UNKNOW_ERR EEROR";
			break;	
		case 	OAM_TASK_MSGSEND_FAIL_ERR:
			pErrStr = "OAM_TASK_MSGSEND_FAIL_ERR EEROR";
			break;	
		case 	OAM_TASK_WHOLELEN_MACHT_ERR:
			pErrStr = "OAM_TASK_WHOLELEN_MACHT_ERR EEROR";
			break;	
		case 	OAM_TASK_PALOADLENGTH_ERR:
			pErrStr = "OAM_TASK_PALOADLENGTH_ERR EEROR";
			break;	
		case 	OAM_TASK_SENDSERNO_ERR:
			pErrStr = "OAM_TASK_SENDSERNO_ERR EEROR";
			break;		
		case 	OAM_UNKOWNED_DEVTYPE_ERR:
			pErrStr = "OAM_UNKOWNED_DEVTYPE_ERR EEROR";
			break;			
		case 	OAM_MIMATCH_DEVTYPE_ERR:
			pErrStr = "OAM_MIMATCH_DEVTYPE_ERR EEROR";
			break;	
		default:
			pErrStr = "OTHER EEROR";
			break;
	}	
	sys_console_printf("  %% %s:%d\r\n\r\n", pErrStr, iRes );
	return OAM_OK;

}


#if 0
int  CommOamCallbackInofShow( void)
{
	unsigned char GwProId = 0;
	for(GwProId = 0; GwProId< 0x14; GwProId++)
	{
		if(handle_oamfuncs[GwProId] == NULL)
			continue;		
		switch(GwProId)
		{
			case GW_CALLBACK_RESERVED:
				sys_console_printf("  GW_CALLBACK_RESERVED is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_EQU_REQUEST:
				sys_console_printf("  GW_CALLBACK_EQU_REQUEST is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;				
			case GW_CALLBACK_EUQINFO:	
				sys_console_printf("  GW_CALLBACK_EUQINFO is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_ALARMORLOG:
				sys_console_printf("  GW_CALLBACK_ALARMORLOG is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_ALARMORLOG_REPONSE:	
					sys_console_printf("  GW_CALLBACK_EUQINFO is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_FILE_RD_WRT_REQUEST:	
				sys_console_printf("  GW_CALLBACK_FILE_RD_WRT_REQUEST is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_FILE_RESERVED:	
				sys_console_printf("  GW_CALLBACK_FILE_RESERVED is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_FILE_DATA_TRA:
				sys_console_printf("  GW_CALLBACK_FILE_DATA_TRA is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;	
			case GW_CALLBACK_FILE_TRA_ACK:
				sys_console_printf("  GW_CALLBACK_FILE_TRA_ACK is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_SNMP_REQUEST:		
				sys_console_printf("  GW_CALLBACK_SNMP_REQUEST is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
					/**/
			case GW_CALLBACK_SNMP_REPONSE:		
				sys_console_printf("  GW_CALLBACK_SNMP_REPONSE is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;		
			case GW_CALLBACK_SNMP_TRAP:		
				sys_console_printf("  GW_CALLBACK_SNMP_TRAP is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_CLI_REQUEST:		
				sys_console_printf("  GW_CALLBACK_CLI_REQUEST is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_CLI:		
				sys_console_printf("  GW_CALLBACK_CLI is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_IGMPAUTH_REQUEST:		
				sys_console_printf("  GW_CALLBACK_IGMPAUTH_REQUEST is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_IGMPAUTH_REPONST:		
				sys_console_printf("  GW_CALLBACK_IGMPAUTH_REPONST is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_VCONSOLE:		
				sys_console_printf("  OAM_CALLBACK_IGMPAUTH_REPONST(opcode : 0x%x) registered. funtion : 0x%x\r\n",GW_CALLBACK_VCONSOLE, (handle_oamfuncs[GwProId]));
				break;
			default:
				/*sys_console_printf("  Error GwProId(OpCode): %d\r\n",GwProId);*/
				break;
			}
	}
	return OAM_OK;
}
#endif
int  CommOamCallbackInofVty( struct vty *vty)
{
	int GwProId = 0;
	vty_out( vty, "\r\n");
	for(GwProId = 0; GwProId< GW_CALLBACK_LAST; GwProId++)
	{
		if(handle_oamfuncs[GwProId] == NULL)
			continue;
		switch(GwProId)
		{
			case GW_CALLBACK_RESERVED:
				
				vty_out( vty, "  OAM_CALLBACK_RESERVED(opcode:0x%x)  registered. funtion:0x%x\r\n",GW_CALLBACK_RESERVED,(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_EQU_REQUEST:
				vty_out( vty, "  OAM_CALLBACK_EQU_REQUEST(opcode:0x%x) registered. funtion:0x%x\r\n",GW_CALLBACK_EQU_REQUEST ,(handle_oamfuncs[GwProId]));
				break;				
			case GW_CALLBACK_EUQINFO:	
				vty_out( vty, "  OAM_CALLBACK_EUQINFO(opcode:0x%x) registered. funtion:0x%x\r\n",GW_CALLBACK_EUQINFO ,(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_ALARMORLOG:
				vty_out( vty, "  OAM_CALLBACK_ALARMORLOG(opcode:0x%x)  registered. funtion:0x%x\r\n",GW_CALLBACK_ALARMORLOG,(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_ALARMORLOG_REPONSE:	
				vty_out( vty, "  OAM_CALLBACK_ALARM_REPONSE(opcode:0x%x)  registered. funtion:0x%x\r\n",GW_CALLBACK_ALARMORLOG_REPONSE,(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_FILE_RD_WRT_REQUEST:	
				vty_out( vty, "  OAM_CALLBACK_FILE_REQUEST(opcode:0x%x)  registered. funtion:0x%x\r\n",GW_CALLBACK_FILE_RD_WRT_REQUEST,(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_FILE_RESERVED:	
				vty_out( vty, "  OAM_CALLBACK_FILE_RESERVED(opcode:0x%x)  registered. funtion:0x%x\r\n",GW_CALLBACK_FILE_RESERVED,(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_FILE_DATA_TRA:
				vty_out( vty, "  OAM_CALLBACK_FILE_DATA_TRA(opcode:0x%x)  registered. funtion:0x%x\r\n",GW_CALLBACK_FILE_DATA_TRA,(handle_oamfuncs[GwProId]));
				break;	
			case GW_CALLBACK_FILE_TRA_ACK:
				vty_out( vty, "  OAM_CALLBACK_FILE_TRA_ACK(opcode:0x%x)  registered. funtion:0x%x\r\n",GW_CALLBACK_FILE_TRA_ACK,(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_SNMP_REQUEST_1:		
				vty_out( vty, "  OAM_CALLBACK_SNMP_REQUEST(opcode:0x%x)  registered. funtion:0x%x\r\n",GW_CALLBACK_SNMP_REQUEST_1,(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_SNMP_REPONSE_1:		
				vty_out( vty, "  OAM_CALLBACK_SNMP_REPONSE(opcode:0x%x)  registered. funtion:0x%x\r\n",GW_CALLBACK_SNMP_REPONSE_1,(handle_oamfuncs[GwProId]));
				break;		
			case GW_CALLBACK_SNMP_TRAP_1:		
				vty_out( vty, "  OAM_CALLBACK_SNMP_TRAP(opcode:0x%x)  registered. funtion:0x%x\r\n",GW_CALLBACK_SNMP_TRAP_1,(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_CLI_REQUEST_1:		
				vty_out( vty, "  OAM_CALLBACK_CLI_REQUEST(opcode:0x%x)  registered. funtion:0x%x\r\n",GW_CALLBACK_CLI_REQUEST_1,(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_CLI_1:		
				vty_out( vty, "  OAM_CALLBACK_CLI(opcode:0x%x)  registered. funtion:0x%x\r\n",GW_CALLBACK_CLI_1,(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_IGMPAUTH_REQUEST:		
				vty_out( vty, "  OAM_CALLBACK_IGMPAUTH_REQUEST(opcode:0x%x) registered. funtion:0x%x\r\n",GW_CALLBACK_IGMPAUTH_REQUEST,(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_IGMPAUTH_REPONST:		
				vty_out( vty, "  OAM_CALLBACK_IGMPAUTH_REPONST(opcode:0x%x) registered. funtion:0x%x\r\n",GW_CALLBACK_IGMPAUTH_REPONST,(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_VCONSOLE:		
				vty_out( vty, "  OAM_CALLBACK_IGMPAUTH_REPONST(opcode:0x%x) registered. funtion:0x%x\r\n",GW_CALLBACK_VCONSOLE, (handle_oamfuncs[GwProId]));
				break;
			default:
				/*sys_console_printf("  Error GwProId(OpCode): %d\r\n",GwProId);*/
				break;
			}
		}
	return OAM_OK;
}

/***********************************************************
* CommOltMsgRvcCallbackInit
*
* decription : 该函数用于向OAM 模块注册回调函数。这些函数用于处理从onu 侧
*			接收到的完整的信息。
* input :		GwProId  - 	#define GW_CALLBACK_RESERVED					0x0
*						#define GW_CALLBACK_CLI							0x1
*						#define GW_CALLBACK_SNMP						0x2
*						#define GW_CALLBACK_IGMPAUTH					0x3
*						#define GW_CALLBACK_EUQINFO					0x4
*						#define GW_CALLBACK_ALARMORLOG				0x5
*						#define GW_CALLBACK_FILE						0x6
*						#define GW_CALLBACK_ALL							0x7 
*			Function - 	函数。
*
* output :
*
* return :		OAM_OK / OAM_ERR		
************************************************************/
int  CommOltMsgRvcCallbackInit(unsigned char GwProId, void *Function)
{
	if( (GwProId < GW_CALLBACK_LAST) && (handle_oamfuncs[GwProId] == NULL) )
	{
		handle_oamfuncs[GwProId] = Function;
	}
	else
		return OAM_ERR;
	
	return OAM_OK;
}

/*删除注册模块*/
int  CommOltMsgRvcCallbackDel(unsigned char GwProId/*, void *Function*/)
{
	if( (GwProId < GW_CALLBACK_LAST) && (handle_oamfuncs[GwProId] != NULL) )
	{
		handle_oamfuncs[GwProId] = NULL;
#if 0		
		switch(GwProId)
		{	
			case GW_CALLBACK_RESERVED:
				sys_console_printf("  GW_CALLBACK_RESERVED Deregister.\r\n");
				break;
			case GW_CALLBACK_EQU_REQUEST:
				sys_console_printf("  GW_CALLBACK_EQU_REQUEST Deregister.\r\n");
				break;				
			case GW_CALLBACK_EUQINFO:	
				sys_console_printf("  GW_CALLBACK_EUQINFO Deregister.\r\n");
				break;					
			case GW_CALLBACK_ALARMORLOG:
				sys_console_printf("  GW_CALLBACK_ALARMORLOG Deregister.\r\n");
				break;					
			case GW_CALLBACK_ALARMORLOG_REPONSE:	
				sys_console_printf("  GW_CALLBACK_EUQINFO Deregister.\r\n");
				break;					
			case GW_CALLBACK_FILE_RD_WRT_REQUEST:	
				sys_console_printf("  GW_CALLBACK_FILE_RD_WRT_REQUEST Deregister.\r\n");
				break;					
			case GW_CALLBACK_FILE_RESERVED:	
				sys_console_printf("  GW_CALLBACK_FILE_RESERVED Deregister.\r\n");
				break;					
			case GW_CALLBACK_FILE_DATA_TRA:
				sys_console_printf("  GW_CALLBACK_FILE_DATA_TRA Deregister.\r\n");
				break;					
			case GW_CALLBACK_SNMP_REQUEST_1:		
				sys_console_printf("  GW_CALLBACK_SNMP_REQUEST Deregister.\r\n");
				break;
					/**/
			case GW_CALLBACK_SNMP_REPONSE_1:		
				sys_console_printf("  GW_CALLBACK_SNMP_REPONSE Deregister.\r\n");
				break;		
			case GW_CALLBACK_SNMP_TRAP_1:		
				sys_console_printf("  GW_CALLBACK_SNMP_TRAP Deregister.\r\n");
				break;
			case GW_CALLBACK_CLI_REQUEST_1:		
				sys_console_printf("  GW_CALLBACK_CLI_REQUEST Deregister.\r\n");
				break;
			case GW_CALLBACK_CLI_1:		
				sys_console_printf("  GW_CALLBACK_CLI Deregister.\r\n");
				break;
			case GW_CALLBACK_IGMPAUTH_REQUEST:		
				sys_console_printf("  GW_CALLBACK_IGMPAUTH_REQUEST Deregister.\r\n");
				break;
			case GW_CALLBACK_IGMPAUTH_REPONST:		
				sys_console_printf("  GW_CALLBACK_IGMPAUTH_REPONST Deregister.\r\n");
				break;
			case GW_CALLBACK_VCONSOLE:
				sys_console_printf("  GW_CALLBACK_VCONSOLE Deregister.\r\n");
			default:
				return (-1);
		}		
#endif		
	}
	else
		return OAM_ERR;
	
	return OAM_OK;
}
#if 0
/*显示所有注册的处理模块*/
int CommOltMsgRvcCallBackShow(void)
{
	unsigned char GwProId = 0;
	for(GwProId = 0; GwProId <= GW_CALLBACK_LAST; GwProId++)
	{
		if (handle_oamfuncs[GwProId] == NULL)
			continue;
		switch(GwProId)
		{

			case GW_CALLBACK_RESERVED:
				sys_console_printf("  GW_CALLBACK_RESERVED is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_EQU_REQUEST:
				sys_console_printf("  GW_CALLBACK_EQU_REQUEST is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;				
			case GW_CALLBACK_EUQINFO:	
				sys_console_printf("  GW_CALLBACK_EUQINFO is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_ALARMORLOG:
				sys_console_printf("  GW_CALLBACK_ALARMORLOG is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_ALARMORLOG_REPONSE:	
				sys_console_printf("  GW_CALLBACK_EUQINFO is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_FILE_RD_WRT_REQUEST:	
				sys_console_printf("  GW_CALLBACK_FILE_RD_WRT_REQUEST is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_FILE_RESERVED:	
				sys_console_printf("  GW_CALLBACK_FILE_RESERVED is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;					
			case GW_CALLBACK_FILE_DATA_TRA:
				sys_console_printf("  GW_CALLBACK_FILE_DATA_TRA is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_FILE_TRA_ACK:
				sys_console_printf("  GW_CALLBACK_FILE_TRA_ACK is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;				
			case GW_CALLBACK_SNMP_REQUEST_1:		
				sys_console_printf("  GW_CALLBACK_SNMP_REQUEST is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
					/**/
			case GW_CALLBACK_SNMP_REPONSE_1:		
				sys_console_printf("  GW_CALLBACK_SNMP_REPONSE is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;		
			case GW_CALLBACK_SNMP_TRAP_1:		
				sys_console_printf("  GW_CALLBACK_SNMP_TRAP is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_CLI_REQUEST_1:		
				sys_console_printf("  GW_CALLBACK_CLI_REQUEST is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_CLI_1:		
				sys_console_printf("  GW_CALLBACK_CLI is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_IGMPAUTH_REQUEST:		
				sys_console_printf("  GW_CALLBACK_IGMPAUTH_REQUEST is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_IGMPAUTH_REPONST:		
				sys_console_printf("  GW_CALLBACK_IGMPAUTH_REPONST is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			case GW_CALLBACK_VCONSOLE:		
				sys_console_printf("  GW_CALLBACK_VCONSOLE is register. funtion : 0x%x\r\n",(handle_oamfuncs[GwProId]));
				break;
			default:
				break;
		}
		/*if (GwProId == GW_CALLBACK_SNMP_TRAP)
			sys_console_printf("  end of show oamCallBack funtion list\r\n");*/
	}
	return 0;
}
#endif
/***********************************************************
* CommOltMsgProIdGet
*
* decription : 该函数用于向OAM 模块注册回调函数。这些函数用于处理从onu 侧
*			接收到的完整的信息。
*
* Input :  	GwOpCode - Gw opCode.
*
* Output :	GwProId  - 	#define GW_CALLBACK_RESERVED					0x0
*						#define GW_CALLBACK_CLI							0x1
*						#define GW_CALLBACK_SNMP						0x2
*						#define GW_CALLBACK_IGMPAUTH					0x3
*						#define GW_CALLBACK_EUQINFO					0x4
*						#define GW_CALLBACK_ALARMORLOG				0x5
*						#define GW_CALLBACK_FILE						0x6
*						#define GW_CALLBACK_ALL							0x7 
*

*
* return :		OAM_OK / OAM_ERR		
************************************************************/
int CommOltMsgCallbackProIdGet(const unsigned char GwOpCode,
							unsigned char *GwProId)
{
	if( (GW_CALLBACK_LAST <= GwOpCode) || (NULL == GwProId) )
		return OAM_NULL_POINTER;
	*GwProId = GwOpCode;
	#if 0
    switch(GwOpCode)
    	{

	case  GW_OPCODE_CLI_REQUEEST:
	case  GW_OPCODE_CLI_RESPONSE:
		*GwProId = (unsigned char)GW_CALLBACK_CLI;
		break;

    case GW_OPCODE_SNMP_REQUEST:
    case GW_OPCODE_SNMP_RESPONSE:
    case GW_OPCODE_SNMP_RESERVED_1:
    case GW_OPCODE_SNMP_RESERVED_2:
		*GwProId = (unsigned char)GW_CALLBACK_SNMP;
		break;	
	/*added by wutw at 22 september*/
    case GW_OPCODE_SNMP_TRAP:
		*GwProId = (unsigned char)GW_CALLBACK_SNMP_TRAP;
		break;					
    case GW_OPCODE_IGMPAUTH_REQUEST:
    case Gw_OPCODE_IGMPAUTH_REPONSE:	
		*GwProId = (unsigned char)GW_CALLBACK_IGMPAUTH;
		break;	

    case GW_OPCODE_EUQ_INFO_REQUESET:
    case GW_OPCODE_EUQ_INFO_RESPONSE:
		*GwProId = (unsigned char)GW_CALLBACK_EUQINFO;
		break;	

    case GW_OPCODE_ALARM_OR_LOG_TRAN:
    case GW_OPCODE_ALARM_OR_LOG_RESPONSE:
		*GwProId = (unsigned char)GW_CALLBACK_ALARMORLOG;
		break;	

    case GW_OPCODE_FILE_READ_REQUEST:
    case GW_OPCODE_FILE_WRITE_REQUEST:
    case GW_OPCODE_FILE_DATA_TRAN:
    case GW_OPCODE_FILE_TRANSFER_ACK:
    case GW_OPCODE_FILE_RESERVED_1:
    case GW_OPCODE_FILE_RESERVED_2:
		*GwProId = (unsigned char)GW_CALLBACK_FILE;
		break;	
	default:
		return (OAM_OPCODE_INVALUE_ERR);
    	}	
	#endif
	return OAM_OK;
}

/********************************************************
* CommOamSendDelay
*
*
*
*
*
*
*
*
*********************************************************/
/*short int CommOamSendDelay(unsigned short ticks)
{
	taskDelay(ticks);
	return OAM_OK;
}*/
/********************************************************
* CommOltMsgSendsernoCheck
*
*
*
*
*
*
*
*
*********************************************************/
short int CommOltMsgSendsernoCheck(
				const unsigned short      OltId, 
				const unsigned short      OnuId,
				const unsigned int 		sendserno,
				/*GW_OAM_PAD_layer_msg_t *pOamInfo,*/
				unsigned short *pQueueId)	
{
	unsigned short TempId = 0;
	unsigned long  SerNo = sendserno/*pOamInfo->sendserno*/;
	if( pQueueId == NULL )	/* modified by xieshl 20090916 */
		return OAM_ERR;
	if( (OltId >= GW_MAX_OLTID) || (OnuId >= GW_MAX_ONU) )
		return OAM_ERR;
	for(TempId = 0; TempId<GW_MAX_QUEUEID; TempId++)
		{
		if((TRUE == goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[TempId].Iused) && \
			goltMsg[OltId].onu_queue_t[OnuId].onu_msg[TempId].sendserno == SerNo)
			{
			*pQueueId = TempId;
			return (OAM_OK);
			}
		}
	return (OAM_ERR);
}

/********************************************************
* CommOnuMsgEmptyQueueGet
*
*
*
*
*
*
*
*
*********************************************************/
short int CommOltMsgEmptyQueueGet(
				const unsigned short      OltId, 
				const unsigned short      OnuId,
				unsigned short *pQueueId)				
{
	unsigned char TempId = 0;
	unsigned char QueueId = 0xff;
	if( pQueueId == NULL )	/* modified by xieshl 20090916 */
		return OAM_ERR;
	if( (OltId >= GW_MAX_OLTID) || (OnuId >= GW_MAX_ONU) )
		return OAM_ERR;
	for(TempId = 0; TempId<GW_MAX_QUEUEID; TempId++)
	{
		if ( TRUE == goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[TempId].Iused)
			{
			goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[TempId].RecvMsgTime ++;
			if( goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[TempId].RecvMsgTime >= (GW_MAX_QUEUEID-1))
				{
				CommOltMsgQueueFree(OltId, OnuId, TempId );			
				if( QueueId == 0xff )
					{
					*pQueueId = TempId;
					QueueId = TempId;
					}
				}
			}
		else {
			if( QueueId == 0xff )
				{
				*pQueueId = TempId;
				QueueId = TempId;
				}
			}
	}

	if( QueueId  != 0xff )
		return( OAM_OK );
	
	if( gGwOamTaskErrDebug == 1 )
		sys_console_printf("\r\nNo g_free memory for onu %d/%d to buffer oam msg\r\n", OltId, OnuId); 
	return (OAM_ERR);
}

int CommOltMsgQueueFree(unsigned short OltId, unsigned short OnuId,unsigned short QueueId)
{
	if( QueueId >= GW_MAX_QUEUEID )	/* modified by xieshl 20090916 */
		return OAM_ERR;
	if( (OltId >= GW_MAX_OLTID) || (OnuId >= GW_MAX_ONU) )
		return OAM_ERR;

	if(NULL != goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad)
		VOS_Free((VOID *)goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad);	
	
	goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad = NULL;
	goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].Iused = FALSE;
	goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].RecvMsgTime  = 0;
	memset(&goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId], 0, sizeof(GW_OAM_PAD_layer_msg_t));
	return OAM_OK;

	}

/*****************************************
*
*
*
*
*
*
*
******************************************/
/*int CommOltMsgQueueInfoGet(unsigned short OltId, 
								unsigned short OnuId,
								unsigned short QueueId, 
								unsigned char **ppRevBuf, 
								unsigned char **pSessionField,
								unsigned short *pWhlLength,
								unsigned short *pPayloadlen)
{
	*ppRevBuf = goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad;
	*pWhlLength = goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].wholepktlen;
	*pPayloadlen = goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength;
	*pSessionField = goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].sessionFieldbuf;
	return OAM_OK;
}*/

/*****************************************
*
*
*
*
*
*
* modified 2006/11/23
*	增加对各个分片中帧的总长度检查
*	进行内存拷贝之前，判断拷贝目的缓冲区的长度是否足够，如果不够
*	则返回错误，避免产生内存越界的可能。
******************************************/
int CommOltMsgQueueSave(unsigned short OltId, 
								unsigned short OnuId,
								unsigned short QueueId, 
								GW_OAM_PAD_layer_msg_t *pOamInfo 
								/*unsigned char  *pOamFrame*/)
{
	unsigned int wholeLen = 0;
	unsigned int payloadlen = 0;
	unsigned int payoffset = 0;
	unsigned int sendNo = 0;
	unsigned char *pRevBuf = NULL;
	
	if (!goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].Iused)
		{
			/*modified by wutw at september 2 for modified Gw oam protocol.(add the sessionid field to gw oam protocol) */
			/*memcpy(&goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId],pOamInfo,sizeof(GW_OAM_PAD_layer_msg_t));*/

			wholeLen =pOamInfo->wholepktlen;
			payloadlen = pOamInfo->payloadlength;
			payoffset = pOamInfo->payloadoffset;
			if ((wholeLen-payoffset) < payloadlen)
				return OAM_TASK_WHOLELEN_MACHT_ERR;	
			
			pRevBuf = (unsigned char*)VOS_Malloc((ULONG)wholeLen, (ULONG)MODULE_OAM);
			if (NULL == pRevBuf)
			{
				return OAM_MALLOC_ERR;
			}
			memset(pRevBuf, 0, wholeLen);
			memcpy((pRevBuf+payoffset),pOamInfo->pBuf, payloadlen);
			
			goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength = payloadlen;
			goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].gw_opcode = pOamInfo->gw_opcode ;
			goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].payloadoffset = pOamInfo->payloadoffset ;
			goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].sendserno = pOamInfo->sendserno ;
			goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].llid = pOamInfo->llid ;
			goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].wholepktlen = pOamInfo->wholepktlen ;
			memcpy(goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].sessionFieldbuf, pOamInfo->sessionFieldbuf, GW_OAM_SESSIONID) ;

			goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad = pRevBuf;
			goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].Iused = TRUE;			
		}
	else
		{
			pRevBuf = goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad;	
			payloadlen = goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength;
			wholeLen = 	goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].wholepktlen;
			sendNo = goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].sendserno;
			/*判断该帧的长度是否与之前分片的总长度相同，不同则直接返回错
			误by wutw 2006/11/23*/
			if (wholeLen != pOamInfo->wholepktlen )
				return OAM_WHOLELENGTH_ERR;
			if (sendNo != pOamInfo->sendserno)
				return OAM_TASK_SENDSERNO_ERR;			
			/*判断是否sessionid是否相同*/
			if (0 != memcmp(goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].sessionFieldbuf, pOamInfo->sessionFieldbuf, GW_OAM_SESSIONID))
				return OAM_SESSIONFIELD_ERR;
			/*以下判断是否连续帧,中间不允许有断帧出现*/
			if((payloadlen) != pOamInfo->payloadoffset)
			{
				return OAM_SEQUENCE_ERR;
			}
			
			/*判断剩余的空间是否小于需要拷贝的长度，如果小于，则不能进行操作
			直接返回错误.by wutw 2006/11/23*/
			if ((wholeLen - payloadlen) < pOamInfo->payloadlength )
				return 	OAM_WHOLELENGTH_ERR;
			memcpy(((payloadlen)+pRevBuf),pOamInfo->pBuf, pOamInfo->payloadlength);
			goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength += pOamInfo->payloadlength;
			
		}	
	return OAM_OK;
}

/*****************************************
*
*
*
*
*
*
*
******************************************/
int CommOltMsgSemTake(unsigned short OltId, unsigned short OnuId, unsigned short QueueId)
{
#if 0
	SEM_ID		SemId ;
	if (OAM_ERR == semTake(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuSemId, WAIT_FOREVER) )
		{
			return (OAM_ERR);
		}
#endif
	if (OAM_OK != VOS_SemTake(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuSemId, WAIT_FOREVER))
		return (OAM_ERR);
	return OAM_OK;
}
/*****************************************
*
*
*
*
*
*
*
******************************************/
int CommOltMsgSemGive(unsigned short OltId, unsigned short OnuId, unsigned short QueueId)
{
#if 0
	SEM_ID		SemId ;
	if (OAM_ERR == semGive(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuSemId))
		{
			return (OAM_ERR);
		}
#endif
	if (OAM_OK != VOS_SemGive(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuSemId))
		return (OAM_ERR);
		
	return (OAM_OK);

	
}

/******************************************************************
* CommOltMsgReveive
* 描述:该函数判断是否该帧为存在在收的帧.
* 		函数操作包含:
*			1. 锁定信号量
*			2. 停止定时器
*			3. 判断是否存在该sentserno
*			4. 正常其他操作
*			5. 开始定时器
*			6. 释放信号量
* 输入:
* 输出:
* 返回:
* 修改:	增加判断wholepktlen的长度，如果大于65535，则为非法包，不允许接受。
******************************************************************/
short int CommOltMsgReveive(
				const unsigned short      OltId, 
				const unsigned short      OnuId,
				/*unsigned char OpCode,*/
				GW_OAM_PAD_layer_msg_t *pOamInfo
				/*unsigned char *pOamFrame*/)
{
	unsigned short QueueId = 0;
	unsigned short payloadlen = 0;
	unsigned result = OAM_OK;

	/*如果传送的帧大于65535，则为合法数据帧，不允许接受和发送*/
	if ((pOamInfo->wholepktlen) > OAM_DATA_LEN)
		return OAM_WHOLELENGTH_ERR;
	
    if (0 == pOamInfo->payloadoffset)
    {/*第一个数据包*/
    		/*检查是否已经存在相同的sendserno*/
		result = CommOltMsgSendsernoCheck(OltId,OnuId,pOamInfo->sendserno, &QueueId);
		if(result == OAM_OK)
			return (OAM_ERR);
		
    	/*1. 检查是否该数据包为一个独立的数据包*/
		if(pOamInfo->payloadlength == pOamInfo->wholepktlen)
		{
			unsigned char *pSessionField = NULL;
			unsigned char *pRevBuf = NULL;
			unsigned char GwProId = 0;

			/*如果该payloadlength大于1514-32-8的长度，则也为非法帧*/
			if (pOamInfo->payloadlength > (MAX_ETHERNET_FRAME_SIZE_STANDARDIZED - \
											GW_OAM_HEADER_SIZE - GW_OAM_PAD_LENGTH - \
											GW_OAM_SESSIONID) )
				{
				VOS_ASSERT(0);
				return OAM_TASK_PALOADLENGTH_ERR;
				}
			
			/*notes: that is one indepedence frame , so copy the sessionIdfield to pRevBuf.
			modified by wutw at september two*/
			payloadlen = pOamInfo->payloadlength;
			pRevBuf = (unsigned char *)VOS_Malloc((ULONG)payloadlen, (ULONG)MODULE_OAM);
			if (NULL == pRevBuf)
				{
				VOS_ASSERT(0);
				return OAM_MALLOC_ERR;
				}
			pSessionField = (unsigned char *)VOS_Malloc((ULONG)GW_OAM_SESSIONID, (ULONG)MODULE_OAM);
			if (NULL == pSessionField)
			{
				VOS_ASSERT(0);
				VOS_Free((VOID *) pRevBuf);
				return OAM_ERR;
			}
			memset(pRevBuf, 0, payloadlen);
			memset(pSessionField, 0, GW_OAM_SESSIONID);
			
			memcpy(pSessionField, pOamInfo->sessionFieldbuf, GW_OAM_SESSIONID);
			
			memcpy(pRevBuf , pOamInfo->pBuf, payloadlen);
			
			if (OAM_OK != CommOltMsgCallbackProIdGet(pOamInfo->gw_opcode, &GwProId))
				{
				VOS_Free((VOID *) pRevBuf);
				VOS_Free((VOID *) pSessionField);
				return OAM_ERR;
				}
			if(gGwOamTrackDebug == 1)
			{	
				sys_console_printf("  before CommOltMsgCallback() OAM opCode = %d\r\n",GwProId);
				sys_console_printf("  whole length : %d\r\n", pOamInfo->wholepktlen);
				sys_console_printf("  single packe!\r\n");
			}
			return (CommOltMsgCallback(OltId, OnuId, pOamInfo->llid, GwProId, payloadlen, pRevBuf, pSessionField));
		}
		else
		{
			/*2. 该数据包为非独立数据包*/
			/*是否该数据包的总长度> GW_MAX_MSG_LENGTH ?*/
			/*if(pOamInfo->wholepktlen  >OAM_DATA_LEN)
			{
				return (OAM_WHOLELENGTH_ERR);
			}*/

			/*获取空闲的queueid,用于放置新的数据包*/
			result = CommOltMsgEmptyQueueGet(OltId, OnuId, &QueueId);
			if(result != OAM_OK)
			{
				return result;
			}
			
			if(gGwOamTrackDebug == 1)
			{	
				sys_console_printf("  before CommOltMsgQueueSave() QueueId = %d\r\n",QueueId);
				sys_console_printf("  whole length : %d\r\n", pOamInfo->wholepktlen);				
				sys_console_printf("  not single packe! serNo %d offset %d\r\n",pOamInfo->sendserno,pOamInfo->payloadoffset);
			}
			result = CommOltMsgQueueSave(OltId, OnuId, QueueId, pOamInfo/*, pOamFrame*/);
			if(result != OAM_OK)
			{
				return result;
			}
		}
		/*启动定时器，时间间隔为2s，如果发生超时事件，则丢弃该数据包*/
		/*TempCount = CommOnuMsgWdogInfoInit(OltId,  OnuId, QueueId);
		wdStart (goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuWdId,sysClkRateGet()*2, 
							(FUNCPTR)CommOnuMsgWdogTimeOutPro, (int)(OnuId*OnuId*QueueId));*/
		return OAM_OK;
    }
	/*================================*/
	else
	{
		/*检查是否存在相应数据帧的缓冲区，如果不存在，则丢弃该帧*/
 		result = CommOltMsgSendsernoCheck(OltId, OnuId, pOamInfo->sendserno, &QueueId);	
		if(result != OAM_OK)
		{
			return (result);
		}
		
		if((pOamInfo->payloadoffset+ pOamInfo->payloadlength) != pOamInfo->wholepktlen)
		{
			/*停止当前msg queue的定时器*/
			/*wdCannel(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuWdId);*/

			if (OAM_ERR == CommOltMsgSemTake(OltId, OnuId, QueueId))
			{
				return (OAM_ERR);
			}			
			
			if(gGwOamTrackDebug == 1)
			{	
				sys_console_printf("  before CommOltMsgQueueSave() QueueId = %d\r\n",QueueId);
				sys_console_printf("  whole length : %d\r\n", pOamInfo->wholepktlen);
				sys_console_printf("  not single packe! serNo %d offset %d\r\n",pOamInfo->sendserno,pOamInfo->payloadoffset);
			}				

			/*以下判断是否连续帧,中间不允许有断帧出现*/
			if(OAM_OK != CommOltMsgQueueSave(OltId, OnuId, QueueId, pOamInfo/*, pOamFrame*/))
			{
				/*删除所有的接收到的数据包,并释放空间*/
				/*清空goltMsg数据结构*/
				CommOltMsgQueueFree(OltId, OnuId,QueueId);
				CommOltMsgSemGive(OltId, OnuId, QueueId);
				return (OAM_ERR);
			}

			/*启动定时器，时间间隔为2s，如果发生超时事件，则丢弃该数据包*/
			/*CommOnuMsgWdogInfoInit(OltId,  OnuId, QueueId);
			wdStart (goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuWdId,sysClkRateGet()*2, 
					(FUNCPTR)CommOnuMsgWdogTimeOutPro, (int)(OnuId*OnuId*QueueId));*/
							
			CommOltMsgSemGive(OltId, OnuId, QueueId);
			return (OAM_OK);
		}
		else
		{/*最后一个数据包*/	
			/*wdCannel(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuWdId);*/
			unsigned char *pPadBuf = NULL;
			unsigned char *pTempBuf = NULL;
			unsigned char *pTemp_SessField = NULL;
			unsigned char *pSessionField = NULL;
			unsigned short  whlLength = 0;
			unsigned char GwProId = 0;
			payloadlen = 0;
			
			if (OAM_ERR == CommOltMsgSemTake(OltId, OnuId, QueueId))
			{
				return (OAM_ERR);
			}	
				
			if(gGwOamTrackDebug == 1)
			{	
				sys_console_printf("  before CommOltMsgQueueSave() QueueId = %d\r\n",QueueId);
				sys_console_printf("  whole length : %d\r\n", pOamInfo->wholepktlen);
				sys_console_printf("  not single packe! serNo %d offset %d\r\n",pOamInfo->sendserno,pOamInfo->payloadoffset);
			}	
			
			/*modified by wutw 2006/11/23*/
			payloadlen = goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength;
			/*if (whlLength != pOamInfo->wholepktlen)
				return OAM_WHOLELENGTH_ERR;*/
			
			/*如果当前缓冲区的长度与当前分片的长度不等于总的长度，
			则返回错误by wutw 2006/11/23*/
			if ((payloadlen + pOamInfo->payloadlength ) != pOamInfo->wholepktlen)
				{
				CommOltMsgQueueFree(OltId, OnuId,QueueId);
				CommOltMsgSemGive(OltId, OnuId, QueueId);
				return OAM_SEQUENCE_ERR;
				}
			
			/*以下判断是否连续帧,中间不允许有断帧出现*/
			if(OAM_OK != CommOltMsgQueueSave(OltId, OnuId, QueueId, pOamInfo/*, pOamFrame*/))
			{
				/*删除所有的接收到的数据包,并释放空间*/
				/*清空goltMsg数据结构*/
				CommOltMsgQueueFree(OltId, OnuId,QueueId);
				CommOltMsgSemGive(OltId, OnuId, QueueId);
				return (OAM_ERR);
			}

			pTempBuf = goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad;
			whlLength = goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].wholepktlen;
			pTemp_SessField = (goltMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].sessionFieldbuf);

			pPadBuf = (unsigned char*)VOS_Malloc((ULONG)whlLength, (ULONG)MODULE_OAM);
			if (NULL == pPadBuf)
			{
				CommOltMsgQueueFree(OltId, OnuId,QueueId);
				CommOltMsgSemGive(OltId, OnuId, QueueId);
				ASSERT(0);
				return OAM_ERR;
			}
			memset(pPadBuf, 0, whlLength);
			
			pSessionField = (unsigned char *)VOS_Malloc((ULONG)GW_OAM_SESSIONID, (ULONG)MODULE_OAM);
			if (NULL == pSessionField)
			{
				VOS_Free((VOID *) pPadBuf);
				CommOltMsgQueueFree(OltId, OnuId,QueueId);
				CommOltMsgSemGive(OltId, OnuId, QueueId);
				ASSERT(0);
				return OAM_ERR;
			}
			memset(pSessionField, 0, GW_OAM_SESSIONID);

			memcpy(pSessionField, pTemp_SessField, GW_OAM_SESSIONID);
			pTemp_SessField = NULL;
			memcpy(pPadBuf, pTempBuf, whlLength);
			pTempBuf = NULL;
			
			CommOltMsgQueueFree(OltId, OnuId,QueueId);
			CommOltMsgSemGive(OltId, OnuId, QueueId);
			
			/*CommOnuMsgWdogInfoDel(OltId*OnuId*QueueId);*/
			/*wdDelete(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuWdId);*/
			if (OAM_OK != CommOltMsgCallbackProIdGet(pOamInfo->gw_opcode, &GwProId))
				{
				VOS_Free((VOID *) pPadBuf);
				VOS_Free((VOID *) pSessionField);
				return OAM_ERR;	
				}
			if(gGwOamTrackDebug == 1)
			{	
				sys_console_printf("  before CommOltMsgCallback() OAM opCode = %d\r\n",GwProId);
				sys_console_printf("  whole length : %d\r\n", pOamInfo->wholepktlen);				
				sys_console_printf("  Last packe! serNo %d offset %d\r\n",pOamInfo->sendserno,pOamInfo->payloadoffset);
				sys_console_printf("  Last packe! payloadlen %d\r\n",whlLength);
			}					
			return (CommOltMsgCallback(OltId, OnuId, pOamInfo->llid, GwProId, (whlLength), pPadBuf, pSessionField));
		}	
	}/*end of else*/
}


/**********************************************
*CommOltMsgCallback
* 描述: 该函数调用回调函数组，回调函数组被cli，
*		snmp等注册
* 输入参数:	
*				PonId - 0-19, 需要调用者调用GetCardIdxByPonChip( short int PonChipIdx )
*							获取slotid;
*							调用GetPonPortByPonChip( short int PonChipIdx ) 获取port 
*				OnuId - 	逻辑链路,使用该回调函数时,该回调函数已经将该llid
*							转换成用户熟悉的onuid
*				GwProId - 模块类型,见头文件定义,如下
*										for callback funtion define
*										#define GW_CALLBACK_RESERVED					0x0
*										#define GW_CALLBACK_CLI								0x1
*										#define GW_CALLBACK_SNMP							0x2
*										#define GW_CALLBACK_IGMPAUTH					0x3
*										#define GW_CALLBACK_EUQINFO					0x4
*										#define GW_CALLBACK_ALARMORLOG			0x5
*										#define GW_CALLBACK_FILE							0x6
*										#define GW_CALLBACK_ALL								0x7
*										#define GW_CALLBACK_SNMP_TRAP				0x8
*				Length - 		payload 数据的实际长度；
*				*pFrame - 指向payload 数据的指针,无论正确错误,均由调用者释放。
*				*pSessionField - 指向sessionfield (8bytes) 的指针,由调用者释放。
* 输出参数: 无
* 返回值: 无
*  修改纪录 : 将	tempounId = GetOnuIdxByLlid( PonId, llid );
*					OnuId = tempounId % 64;	去除，已经在CommEhtFrameReveive()
*
*************************************************/
int CommOltMsgCallback(  
                                  const short int        PonId,
                                  const unsigned short   OnuId,
                                  const short int        llid,
                                  const unsigned char GwProId,
                                  const unsigned short   Length,
                                  unsigned char   *pFrame,
                                  unsigned char  *pSessionField)
{
	unsigned short     userOnuId = 0;

	if( GW_CALLBACK_LAST <= GwProId )
	{
		if( pFrame ) VOS_Free(pFrame);
		if( pSessionField ) VOS_Free(pSessionField);
		pFrame = NULL;
		pSessionField = NULL;
		VOS_ASSERT(0);
		return (OAM_ERR);
	}
	
	/*将onuid 转换成用户习惯的useronuid*/
	userOnuId = OnuId+1;

	/*Show debug inof. Added by wutongwu at 14 October*/
	if ((oam_debug_info[GwProId] == 1) && ( handle_oamfuncs[GW_CALLBACK_RESERVED] != NULL))
		handle_oamfuncs[GW_CALLBACK_RESERVED] (PonId,  userOnuId, llid, GwProId, Length, pFrame, pSessionField);

	if(handle_oamfuncs[GwProId] == NULL)
	{
		if( pFrame ) VOS_Free(pFrame);
		if( pSessionField ) VOS_Free(pSessionField);
		pFrame = NULL;
		pSessionField = NULL;
		return (OAM_ERR);
	}
	
	/*for oam task rev testing*/
	if (GwProId == Gw_OPCODE_OAM_DEBUG)
	{
		debugRevCount++;
		if( pFrame ) VOS_Free(pFrame);
		if( pSessionField ) VOS_Free(pSessionField);
		pFrame = NULL;
		pSessionField = NULL;
		return (OAM_OK);
	}

 	handle_oamfuncs[GwProId] (PonId,  userOnuId, llid, Length, pFrame, pSessionField);

	return (OAM_OK);
}


#if 0
#endif

int DrPeng_parse_SessionField(const short int   OltId,const unsigned short   OnuId,unsigned char   *content,const unsigned short length,unsigned char pSessionField[])
{
	 if (NULL == content)
		return OAM_ERR;
	
	 pSessionField[0] = (unsigned char)((OltId>>8)&0xff);
     pSessionField[1] = (unsigned char)(OltId & 0xff);
     pSessionField[2] = (unsigned char)((OnuId>>8)&0xff);
     pSessionField[3] = (unsigned char)(OnuId & 0xff);
	 if(content[DRPENG_OAM_PAD_BRANCH] == 0x37)
	 {
	 	if(length < DRPENG_OAM_PAD_BRANCH+DRPENG_OAM_PAD_PORT_TVL+DRPENG_OAM_PAD_OUI)
			return OAM_ERR;
	 	pSessionField[4] = content[DRPENG_OAM_PAD_BRANCH+DRPENG_OAM_PAD_PORT_TVL];
        pSessionField[5] = content[DRPENG_OAM_PAD_BRANCH+DRPENG_OAM_PAD_PORT_TVL+1];
      	pSessionField[6] = content[DRPENG_OAM_PAD_BRANCH+DRPENG_OAM_PAD_PORT_TVL+2];
	 }
	else
	{
		if(length < DRPENG_OAM_PAD_BRANCH+DRPENG_OAM_PAD_OUI)
			return OAM_ERR;
		pSessionField[4] = content[DRPENG_OAM_PAD_BRANCH];
		pSessionField[5] = content[DRPENG_OAM_PAD_BRANCH+1];
		pSessionField[6] = content[DRPENG_OAM_PAD_BRANCH+2];
	}
	return OAM_OK;
}


/*******************************************************************
* CommOamPayloadReveice
*
* 描述:该函数用于接收oam的msg,解析payload中的msg信息,
*
*
* 输入:
*
* 输出:
* 
* 返回:
*
*******************************************************************/
short int CommOamPayloadReveice ( 
                                    const short int        OltId,
                                    const unsigned short   OnuId,
                                    const short int        llid,
                                    const unsigned short   length,
                                    unsigned char   *content)
{
    GW_OAM_PAD_layer_msg_t       oam_message;
    unsigned char                           *receive_data  = content;
    unsigned char                         DrPeng_opcode = 0; 
    unsigned char				  pSessionField[8] = {0};
    int result = OAM_OK;

    /*sys_console_printf("3.-----------------------------------------------\r\n");
    sys_console_printf("3.Receive_link_layer_frame\r\n");
    receive_data = content;	*/

    if (NULL == content)
		return OAM_ERR;
    /* extract message id */
    memset(&oam_message,0,sizeof(GW_OAM_PAD_layer_msg_t));
        /*Dr.Peng 不处理数据，直接透传给上层*/
   if(content[0] == 0x01 && content[1] == 0x80 && content[2] == 0xc2 &&\
   	content[GW_OAM_PAD_OUI] == 0x0c && content[GW_OAM_PAD_OUI+1] == 0x7c && content[GW_OAM_PAD_OUI+2] == 0x7d)
   {
	  DrPeng_opcode = (content[GW_OAM_PAD_OPCODE]);
	  result = DrPeng_parse_SessionField( OltId,  OnuId,content,length,pSessionField);
	   if (OAM_ERR == result)
		return OAM_ERR;
	  result = Oam_Session_RecieveCallBack(OltId, OnuId+1, llid, length, content, pSessionField);
	  return result;
   }
   else
   {

    oam_message.gw_opcode = (content[3]);
    /*检查opcode是否合法*/
    result = CommOamOpcodeCheck(oam_message.gw_opcode);    
    if (result != OAM_OK)
    {	
    		sys_console_printf("Recv Oam opCode=%d\r\n", oam_message.gw_opcode);
    		return OAM_OPCODE_INVALUE_ERR;
    }

    oam_message.llid = llid;
    /*oam_message.sendserno = (unsigned int)(*(receive_data+4))*0x100*0x100*0x100 +  (unsigned int)(*(receive_data+5))*0x100*0x100 + \
					 (unsigned int)(*(receive_data+6))*0x100 +  (unsigned int)(*(receive_data+7));*/
    oam_message.sendserno = (unsigned int)(*(receive_data+7))*0x100*0x100*0x100 +  (unsigned int)(*(receive_data+6))*0x100*0x100 + \
					 (unsigned int)(*(receive_data+5))*0x100 +  (unsigned int)(*(receive_data+4));	
    oam_message.wholepktlen = (unsigned short )(((*(receive_data+9)) * 0x100) + *(receive_data+8));
    oam_message.payloadoffset = (unsigned short )(((*(receive_data+11)) * 0x100) + *(receive_data+10));
    oam_message.payloadlength = (unsigned short )(((*(receive_data+13)) * 0x100) + *(receive_data+12));
    memcpy(oam_message.sessionFieldbuf, (receive_data+ GW_OAM_PAD_LENGTH), GW_OAM_SESSIONID); 
    oam_message.pBuf = (receive_data+GW_OAM_PAD_LENGTH + GW_OAM_SESSIONID);
   }
	if(gGwOamTrackDebug == 1)
	{	
	sys_console_printf(" oam_message.sendserno = %d\r\n",oam_message.sendserno);
	sys_console_printf(" oam_message.wholepktlen = %d\r\n",oam_message.wholepktlen);
	sys_console_printf(" oam_message.payloadlength = %d\r\n",oam_message.payloadlength);
	}
	
    switch(oam_message.gw_opcode)
    	{
		case GW_OPCODE_EUQ_INFO_RESPONSE:
		case GW_OPCODE_ALARM_OR_LOG_TRAN:
		case GW_OPCODE_ALARM_OR_LOG_RESPONSE:
		case GW_OPCODE_FILE_DATA_TRAN:
		case GW_OPCODE_FILE_TRANSFER_ACK:
		case GW_OPCODE_SNMP_RESPONSE:
		case GW_OPCODE_SNMP_TRAP:
		case GW_OPCODE_CLI_RESPONSE:
		case Gw_OPCODE_IGMPAUTH_REPONSE:
		case Gw_OPCODE_OAM_DEBUG:
		case GW_OPCODE_SNMP_REQUEST:
			result = CommOltMsgReveive(OltId,  OnuId,  &oam_message );	
			break;
		case GW_OPCODE_EUQ_INFO_REQUESET:
		case GW_OPCODE_FILE_READ_AND_WRITE_REQUEST:
		case GW_OPCODE_CLI_REQUEEST:
		case GW_OPCODE_IGMPAUTH_REQUEST:
#if (RPU_MODULE_IGMP_TVM == RPU_YES )
		case GW_OPCODE_TVM_REQUEST:
		case GW_OPCODE_TVM_RESPONSE:
#endif
		case GW_OPCODE_VCONSOLE:
			result = CommOltMsgReveive(OltId,  OnuId,  &oam_message );
			break;
		default:
			return (OAM_OPCODE_INVALUE_ERR);		
    	}
	return (result);
}


/****************************************************
* CommOamFrameReveive
*
* 描述: 该函数用于接收gw 的oam 帧,解析并判断gw oam的
*		的合法性.
*
* 输入:
*
* 输出:
*
*
******************************************************/
short int CommOamFrameReveive(  
                                  const short int        OltId,
                                  const unsigned short     OnuId,
                                  const short int          llid,
                                  const unsigned short   length,
                                  unsigned char   *content)
{
    GW_oam_layer_msg_t    oam_message;
    Oam_mac_address_t      destination_mac_address;	
    unsigned char                       first_expeced_byte,second_expeced_byte,third_expeced_byte;
    unsigned char 			     first_expeced_byte_peng,second_expeced_byte_peng,third_expeced_byte_peng;
    int iRes = 0;	
    BOOL               valid_address=TRUE;	
    short int tempIndex = 0;
    unsigned char oamOpcode = 0;
    unsigned short     received_ethernet_type;	
    unsigned char *receive_data = (unsigned char *)content;	
    GW_OAM_MSG_t  OamMsgStru;
    /*sys_console_printf("4.-----------------------------------------------\r\n");	
    sys_console_printf("4.Receive_oam_layer_frame()!\r\n");	*/

    /*检查是否content为NULL*/
     if (content == NULL)
	 	return (OAM_ERR);
    /* check illegal frame size */
    if ( (length < MIN_ETHERNET_FRAME_SIZE) || (length > MAX_ETHERNET_FRAME_SIZE_STANDARDIZED) )
    {
#ifdef OAM_DEBUG
        sys_console_printf("OAM_ERR: received wrong size frame (%d bytes )\n",length);	
#endif 	 
        return (OAM_ERR);
    }

    /* extract destination mac address */
    memcpy( destination_mac_address, (unsigned char *)(receive_data+GW_OAM_DA), 6);

    /* check destination mac address */
    for(tempIndex = 0; tempIndex < 6; tempIndex++)
    {
        if(destination_mac_address[tempIndex] != GW_oam_destination_address[tempIndex])
        {
            valid_address = FALSE;
        }
    }
    if( !valid_address )
    { 
        return (OAM_ERR);
    }
	
    received_ethernet_type = (((*(receive_data+GW_OAM_ETH_TYPE)) << 8) + *(receive_data+GW_OAM_ETH_TYPE+1));

    /* check ethernet type */
    if( received_ethernet_type != gGwEthernetType )
    {
#ifdef OAM_DEBUG
        sys_console_printf( "ERROR: received wrong ether type 0x%u, expected 0x%u \n",
                           received_ethernet_type, gGwEthernetType );
#endif 	 
        return (OAM_ERR);
    }


    /* extract sub type */
    oam_message.sub_type      = (unsigned char)  receive_data[GW_OAM_SUBTYPE];
    /* check sub type */
    if( oam_message.sub_type != gGgwOamSubType )
    {
#ifdef OAM_DEBUG
        sys_console_printf( "ERROR: received wrong OAM sub type. expected :0x%x , received 0x%x )\n",
                            gGgwOamSubType, oam_message.sub_type);
#endif 
        return (OAM_ERR);
    }
    
    /* extract flags */
   /* oam_message.flags = (unsigned short)(MAKE_UNSIGNED_SHORT_PARAM(*(receive_data+GW_OAM_FLAGS),*(receive_data+GW_OAM_FLAGS+1))); */
     oam_message.flags = (unsigned short)(((*(receive_data+GW_OAM_FLAGS)) << 8) + *(receive_data+GW_OAM_FLAGS+1));
    /* check flags */
	/* deleted by chenfj 2007-5-12 */
#if 0  
    if( oam_message.flags != gGwOamFlags )
    {
#ifdef OAM_DEBUG
        sys_console_printf( "ERROR: received wrong OAM flags. expected :0x%x , received 0x%x )\n",
                           gGwOamFlags, oam_message.flags);
#endif         
        return (OAM_ERR);
    }
#endif

    /* extract code */
    oam_message.oam_code         = (unsigned char)  receive_data[GW_OAM_CODE];

    /* check code */
    if( oam_message.oam_code != gGwOamCode )
    {
#ifdef OAM_DEBUG
        sys_console_printf( "ERROR: received wrong OAM code. expected :0x%x , received 0x%x )\n",
                           gGwOamCode, oam_message.oam_code);
#endif          
        return (OAM_ERR);
    }
  
    /* extract OUI */
    oam_message.gw_oui_1  = (unsigned char)  receive_data[GW_OAM_PAD_OUI];
    oam_message.gw_oui_2  = (unsigned char)  receive_data[GW_OAM_PAD_OUI+1];
    oam_message.gw_oui_3  = (unsigned char)  receive_data[GW_OAM_PAD_OUI+2];
    receive_data = NULL;

    /*发送到缓冲区里头*/
	gGwOamRevTempCount++;
	if (gGwOamTrackDebug == 1)
	{
		sys_console_printf("  gGwOamRevTempCount %lu\r\n",gGwOamRevTempCount);
		sys_console_printf("  CommOamFrameReveive() Rev OK!\r\n");
		         /*short int  i;
		         unsigned char *data;
		         sys_console_printf("  PonId %d onuId %d \r\n", OltId, OnuId);
		         sys_console_printf("  the frame len %d \r\n",  length );
		         data = (unsigned char *)content;
				 sys_console_printf("  ----------------------------------\r\n");
		         for( i = 0; i< length; i++ )
			 	 {
		             if( (i %20 )== 0 )
					  {
		                  sys_console_printf("\r\n");
		                  sys_console_printf("        ");
		              }
		              sys_console_printf(" %02x", *(unsigned char *)(data+i) );                   
	              }
	    	      sys_console_printf("\r\n");	*/			
	}


	
    iRes = CommOamOpcodeFromOamGet(&oamOpcode, content);
	if (OAM_OK != iRes)
		return iRes;
    memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
    OamMsgStru.OltId = OltId;
    OamMsgStru.OnuId = OnuId;
    OamMsgStru.unicstLlid = llid;
    
    OamMsgStru.oamOpcode = oamOpcode;
	
	/* added by chenfj 2008-3-19
	     仅用于测试目的,
	     接收到ONU的应答消息, 若有错误, 则打印内容
	*/
	if( OAM_DEBUG_FLAG == V2R1_ENABLE)
	{
	short int AckCode, ErrCode;
	AckCode = *((short int *)(&content[GW_OAM_PAD_PAYLOADLENGTH+10]));
	ErrCode =  *((short int *)(&content[GW_OAM_PAD_PAYLOADLENGTH+12]));
	
	if((oamOpcode == GW_OPCODE_FILE_TRANSFER_ACK) && ((AckCode == 0x300) ||((AckCode & 0x01) != 0)||(ErrCode != 0 )))
		{
		sys_console_printf("\r\nfile trans contro-flow from onu %d/%d\r\n", OltId, (OnuId+1));
		for(tempIndex= 0; tempIndex < (length-GW_OAM_PAD_OUI); tempIndex ++)
			{
			sys_console_printf(" %02x", content[GW_OAM_PAD_OUI+tempIndex]);
			if(((tempIndex +1) % 20) == 0 )
				sys_console_printf("\r\n");
			}
		sys_console_printf("\r\n");
		}
	}	
	 /* check oui */
	 first_expeced_byte  =	 (unsigned char)((GW_OUI_DEFINE >>(OAM_BYTES_IN_WORD*OAM_BITS_IN_BYTE))& 0xff);
	 second_expeced_byte =	 (unsigned char)((GW_OUI_DEFINE >>OAM_BITS_IN_BYTE)& 0xff);
	 third_expeced_byte  =	 (unsigned char)(GW_OUI_DEFINE & 0xff);
	
	  first_expeced_byte_peng  =   (unsigned char)((DRPENG_OUI_DEFINE >>(OAM_BYTES_IN_WORD*OAM_BITS_IN_BYTE))& 0xff);
	 second_expeced_byte_peng =   (unsigned char)((DRPENG_OUI_DEFINE >>OAM_BITS_IN_BYTE)& 0xff);
	 third_expeced_byte_peng  =   (unsigned char)(DRPENG_OUI_DEFINE & 0xff);
	 
	 if( ( oam_message.gw_oui_1 == first_expeced_byte ) &&
		 ( oam_message.gw_oui_2 == second_expeced_byte ) &&
		 ( oam_message.gw_oui_3 == third_expeced_byte  ) )
	 {
		iRes = OamIODataSend(OltId, OnuId, (content+GW_OAM_PAD_OUI), GW_MSG_REVEICE, (length-GW_OAM_PAD_OUI), &OamMsgStru);	 
		return (iRes);
	  }
	else if( ( oam_message.gw_oui_1 == first_expeced_byte_peng ) &&
		 ( oam_message.gw_oui_2 == second_expeced_byte_peng ) &&
		 ( oam_message.gw_oui_3 == third_expeced_byte_peng  ) )
	{
		 iRes = OamIODataSend(OltId, OnuId, (content), GW_MSG_REVEICE, (length), &OamMsgStru);	 
		return (iRes);
	}
	else
	{
   #ifdef OAM_DEBUG_PRINTF
		  /* if (PON_log_active_olt_state(OltId, PON_PAS_LOG_FLAG_ERROR))*/
		   
				 sys_console_printf( "ERROR: received wrong OUI. expected :0x%x%x%x , received 0x%x%x%x )\n",
						 first_expeced_byte, second_expeced_byte, third_expeced_byte, 
						 oam_message.gw_oui_1, oam_message.gw_oui_2, oam_message.gw_oui_3);
	
	 #endif
		 return (OAM_ERR);
	}

}
short int DRPENG_COMM_OAM_frame_send  ( 
                      const unsigned short      OltId, 
                      const unsigned short     OnuId,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size)
{
	int iRes = OAM_OK;
	unsigned short oam_frame_length = 0;
	unsigned short oamHeadAndGwField_len = 0;
	unsigned char OamFrame[MAX_SIZE_OF_OAM_FRAME] = {0};
	GW_OAM_MSG_t  OamMsgStru;
	if (gboolGwInit == FALSE)
		return OAM_NOT_INIT_ERR;
	
	iRes = CommOamHeadBuild( OltId, OnuId,OamFrame, &oamHeadAndGwField_len);
	if(OAM_OK != iRes)return iRes;
	
	/*send single oam*/
	if (sent_data_size <= (MAX_SIZE_OF_OAM_FRAME - oamHeadAndGwField_len))
	{
		memcpy((OamFrame+oamHeadAndGwField_len),sent_data,sent_data_size);
		oam_frame_length = oamHeadAndGwField_len + sent_data_size;
		memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
		OamMsgStru.brdcstLlid = OAM_SEND_SPECIFIED_LLID;
		OamMsgStru.bufSize = oam_frame_length;
		OamMsgStru.OltId = OltId;
		OamMsgStru.OnuId = OnuId;
		OamMsgStru.oamOpcode = 0x01;
		iRes = OamIODataSend(OltId, OnuId, OamFrame, GW_MSG_SEND, oam_frame_length, &OamMsgStru);
		if(iRes != OAM_OK)
		{
			return iRes;
		}
		/*added at twenty nine june.For limiting speed of sentting oam frame*/
		taskDelay(GW_DEFAULT_TASKDELAY_TICK);
		if (iRes != OAM_OK) 
		{ 
				return OAM_ERR;
		}			
	}
		
	return iRes;
}
int DrPeng_Comm_EUQ_info_request_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char *pEuqinfobuf,
				const unsigned short Euqinfo_data_size)
{
	int iRes = OAM_OK;
	short int OnuIdIndex = OnuId - 1;
	short int llid = 0;
	if(pEuqinfobuf == NULL)
		return OAM_NULL_POINTER ;
	
	llid = GetLlidByOnuIdx( PonId, OnuIdIndex);
	if ( OAM_INVALID_LLID == llid )
	{
		return OAM_ONU_DEREGISTER_ERR;
	}

	iRes = DRPENG_COMM_OAM_frame_send( 	PonId, 
									OnuIdIndex,
									pEuqinfobuf, 
									Euqinfo_data_size);	
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}	
	return iRes;
}


/****************************************************
* CommEhtFrameReveive
*
* 描述: 接收以太网帧,解析并判断以太网头部合法性
*		并在该函数进行所有的空间释放.
* 输入:
*
* 输出:
*
* 返回:
*
******************************************************/
short int CommEhtFrameReveive (
                                 const short int                 OltId,
                                 const short int                 OnuId,
                                 const short int                 llid,
                                 const unsigned short            length,
                                 void                     *content )
{
#if 0
    short int	OnuId = 0;
#endif
    unsigned char     *received_buffer = (unsigned char *)content;
    int result = OAM_OK;
    if (gboolGwInit == FALSE)
		return OAM_NOT_INIT_ERR;

    /* 前面已经有检查 */
#if 0
    /*检查是否content为NULL*/
     if (content == NULL)
	 	return (OAM_ERR);
	
    OnuId = GetOnuIdxByLlid( OltId, llid);
	if (OnuId == OAM_ERR)
		return (OAM_ERR);
#endif

    result = CommOamFrameReveive( OltId, OnuId, llid, (unsigned short)length, received_buffer);
	
	/*在此处释放所占用的数据*/
	 /*free(content) ;	
	 received_buffer = NULL;
	 content = NULL;*/
    return (result);

}

/****************************************************
* COMM_OAM_frame_send
* 描述: 该函数被cli或者snmp调用。封装GW的OAM帧
*		对于超长数据包，该函数进行分解数据包，并发送
*
*输入: OltId,OnuId,oam_opcode - 见格林威尔私有oam定义文件
*		broadcast_llid - 代表发送目的参数。当
*
*
*
*
******************************************************/
short int COMM_OAM_frame_send  ( 
                      const unsigned short      OltId, 
                      const unsigned short     OnuId,
                      /*unsigned int		queuePro,*/
                      unsigned char oam_opcode,
                      const short int broadcast_llid,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size,
                      unsigned char	*pSessionIdfield,
                      unsigned short devType,
                      unsigned long sendserno )
{
	int iRes = OAM_OK;
	unsigned short oam_frame_length = 0;
	unsigned short oamHeadAndGwField_len = 0;
	unsigned char OamFrame[MAX_SIZE_OF_OAM_FRAME] = {0};
	/*unsigned long sendserno = 0;*/
	unsigned short int payloadoffset = 0;
	GW_OAM_MSG_t  OamMsgStru;
	if (gboolGwInit == FALSE)
		return OAM_NOT_INIT_ERR;

	
	if(oam_debug_info[oam_opcode] == 1 )
	/*if (oam_debug_send == 1)*/
	{
		CommSendReserved(OltId,
                           OnuId,
                           oam_opcode,
                           sent_data_size,
                           sent_data,
                           pSessionIdfield);
	}
	CommOamHeadBuild( OltId, OnuId,OamFrame, &oamHeadAndGwField_len);
	
	/*if (iRes != OAM_OK)
		return (iRes);*/
	oamHeadAndGwField_len += GW_OAM_PAD_LENGTH;
	CommOamPadOuiInsert (OamFrame);
	
	/*check the oam/pad/opcode*/
	iRes = CommOamOpcodeCheck(oam_opcode);
	if(OAM_OK != iRes)
		return iRes;

	iRes = CommOamPadOpcodeInsert  (OamFrame, oam_opcode);
	if (iRes != OAM_OK)
		return (OAM_ERR);
	
	/*获取发送sendserno*/
	/*iRes = CommOamPadSendsernoGet  (&sendserno);
	if (iRes != OAM_OK)
		return (OAM_ERR);*/
	
	CommOamPadSendsernoInsert (OamFrame, sendserno);

	/*modified by wutongwu 09-02*/
	/*CommOamPadWholepktlenInsert  (OamFrame, sent_data_size-8);*/
	CommOamPadWholepktlenInsert  (OamFrame, sent_data_size);
	/*added by wutongwu at 22 October :每发送一个分片都需要重新插入session区域*/
	iRes = CommOamPadPaySessionIdnsert (OamFrame, pSessionIdfield);
	if (iRes != OAM_OK)
		return (OAM_ERR);
	oamHeadAndGwField_len += GW_OAM_SESSIONID;
	
	/*if oam < (60 - 18 - 14)*/
	if(sent_data_size<(MIN_ETHERNET_FRAME_SIZE-oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH-GW_OAM_SESSIONID*/))
		{
			CommOamPadPayloadoffsetInsert(OamFrame, (unsigned short int)0);
			CommOamPadPayloadlenInsert(OamFrame, sent_data_size);
			memset((OamFrame + oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH+GW_OAM_SESSIONID+sent_data_size*/),\
				0,\
				(MIN_ETHERNET_FRAME_SIZE - oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH-GW_OAM_SESSIONID-sent_data_size*/));
			memcpy((OamFrame+oamHeadAndGwField_len),sent_data,sent_data_size);
			oam_frame_length = MIN_ETHERNET_FRAME_SIZE;

			memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
			OamMsgStru.brdcstLlid = broadcast_llid;
			OamMsgStru.bufSize = oam_frame_length;
			OamMsgStru.OltId = OltId;
			OamMsgStru.OnuId = OnuId;
			OamMsgStru.oamOpcode = oam_opcode;
			iRes = OamIODataSend(OltId, OnuId, 	OamFrame, GW_MSG_SEND, oam_frame_length, &OamMsgStru);

			if(iRes != OAM_OK)
				{
				return iRes;
				}

			/*====================*/
		}
	/*if cli_data_size < (1480 - 18 - 14)*/
	/*send single oam*/
	else if (sent_data_size <= (MAX_SIZE_OF_OAM_FRAME - oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE - GW_OAM_PAD_LENGTH - GW_OAM_SESSIONID*/))
		{
			/*增加发送序列号，发送长度，偏移量*/
			CommOamPadPayloadoffsetInsert(OamFrame, (unsigned short int)0);
			CommOamPadPayloadlenInsert(OamFrame, sent_data_size);
			memcpy((OamFrame+oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH + GW_OAM_SESSIONID*/),sent_data,sent_data_size);
			oam_frame_length = oamHeadAndGwField_len + sent_data_size;
			memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
			OamMsgStru.brdcstLlid = broadcast_llid;
			OamMsgStru.bufSize = oam_frame_length;
			OamMsgStru.OltId = OltId;
			OamMsgStru.OnuId = OnuId;
			OamMsgStru.oamOpcode = oam_opcode;
			iRes = OamIODataSend(OltId, OnuId, 	OamFrame, GW_MSG_SEND, oam_frame_length, &OamMsgStru);
			if(iRes != OAM_OK)
				{
				return iRes;
				}
		
			/*added at twenty nine june.For limiting speed of sentting oam frame*/
			taskDelay(GW_DEFAULT_TASKDELAY_TICK);
			if (iRes != OAM_OK) 
			    { 
					return OAM_ERR;
				}			
		}
		/*if cli_data_size > (1480 - 18 - 14)*/
		/*send multi-oam frame*/
	else 
		{
			unsigned short tempSendSize = sent_data_size;
			payloadoffset = 0;
			while(tempSendSize>=(MAX_SIZE_OF_OAM_FRAME-oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE - GW_OAM_PAD_LENGTH - GW_OAM_SESSIONID*/)/*1500-18-14*/)
				{
					CommOamPadPayloadoffsetInsert(OamFrame, (unsigned short int)payloadoffset);
					
					CommOamPadPayloadlenInsert(OamFrame, (MAX_SIZE_OF_OAM_FRAME-oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE - GW_OAM_PAD_LENGTH - GW_OAM_SESSIONID*/));
					/*payloadlens = MAX_SIZE_OF_OAM_FRAME;*/
					memcpy((OamFrame + oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH + GW_OAM_SESSIONID*/), \
							(sent_data+payloadoffset), \
							(MAX_SIZE_OF_OAM_FRAME - oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH - GW_OAM_SESSIONID*/));
					oam_frame_length = MAX_SIZE_OF_OAM_FRAME;

					memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
					OamMsgStru.brdcstLlid = broadcast_llid;
					OamMsgStru.bufSize = oam_frame_length;
					OamMsgStru.OltId = OltId;
					OamMsgStru.OnuId = OnuId;
					OamMsgStru.oamOpcode = oam_opcode;
					iRes = OamIODataSend(OltId, OnuId, 	OamFrame, GW_MSG_SEND, oam_frame_length, &OamMsgStru);
					if(iRes != OAM_OK)
						{
						return iRes;
						}
					
					/*added at Jtwenty nine june.For limiting speed of sentting oam frame*/
					taskDelay(GW_DEFAULT_TASKDELAY_TICK);
						
					payloadoffset += MAX_SIZE_OF_OAM_FRAME-oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH - GW_OAM_SESSIONID*/;	
					tempSendSize = tempSendSize-(MAX_SIZE_OF_OAM_FRAME-oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH - GW_OAM_SESSIONID*/);

			}

			if (tempSendSize == 0)
				return OAM_OK;
			
			/*最后一个帧*/
			/*payloadlens = sent_data_size-payloadoffset + GW_OAM_HEADER_SIZE + GW_OAM_PAD_LENGTH;
			if (payloadlens == tempSendSize)
				return OAM_OK;*/
			
			CommOamPadPayloadoffsetInsert(OamFrame, (unsigned short int)payloadoffset);
			CommOamPadPayloadlenInsert(OamFrame,tempSendSize);
			memcpy((OamFrame + oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH + GW_OAM_SESSIONID*/),(sent_data+payloadoffset), tempSendSize);
			oam_frame_length = tempSendSize + oamHeadAndGwField_len/*GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH + GW_OAM_SESSIONID*/;
			if (oam_frame_length <(MIN_ETHERNET_FRAME_SIZE))
				{
					memset((OamFrame + oam_frame_length), 0, (MIN_ETHERNET_FRAME_SIZE-oam_frame_length));
					oam_frame_length = MIN_ETHERNET_FRAME_SIZE;
				}
			

			memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
			OamMsgStru.brdcstLlid = broadcast_llid;
			OamMsgStru.bufSize = oam_frame_length;
			OamMsgStru.OltId = OltId;
			OamMsgStru.OnuId = OnuId;
			OamMsgStru.oamOpcode = oam_opcode;
			iRes = OamIODataSend(OltId, OnuId, 	OamFrame, GW_MSG_SEND, oam_frame_length, &OamMsgStru);
			if(iRes != OAM_OK)
				{
				return iRes;
				}
			
		}
	return iRes;
}


/****************************************
* OamTask
* 描述: 该函数
*
*
*
* 
******************************************/
/*extern void print_pdu( const char* pdu, USHORT len, UINT width );*/	/* removed by xieshl 20100805 */
void CommOamTask(void)
{
	ULONG aulMsg[4] = {0};
	int	iRes = 0;
	GW_OAM_MSG_t	*pMsgOam = NULL;
	unsigned short      	OltId = 0;
	unsigned short     	OnuId = 0;
	short int		llid = 0;
	unsigned char 	oamOpcode;
	short int 			broadcast_llid;
	unsigned char		*sent_data;
	unsigned short	sent_data_size;
	unsigned int 		msgType = 0;
	unsigned short 	bufpara = 0;

	
	while(1)
	{

		if (VOS_ERROR == VOS_QueReceive(gOamMsgQId, aulMsg, WAIT_FOREVER))
		{
			VOS_ASSERT( 0 );
			/*break;*/
			continue;
		}
		
		pMsgOam = (GW_OAM_MSG_t *)aulMsg[3];		
		msgType = aulMsg[2];
		if( pMsgOam == NULL )
		{
			/*sys_console_printf("\r\n ** pMsgOam = NULL, msgType=%d\r\n", msgType );
			VOS_ASSERT(0);*/
			continue;
		}
		switch(msgType)
			{
				case GW_MSG_SEND:
					gGwOamTotalSendCount++;
					OltId = pMsgOam->OltId;
					OnuId = pMsgOam->OnuId;			
					oamOpcode = pMsgOam->oamOpcode;
					gGwOamSendCount[oamOpcode]++;
					broadcast_llid = pMsgOam->brdcstLlid;
					bufpara = pMsgOam->bufPara;
					sent_data = gOamQueueCtl[GW_MSG_SEND_QUIDX].DataBuff[bufpara].MesData;
					sent_data_size = gOamQueueCtl[GW_MSG_SEND_QUIDX].DataBuff[bufpara].MesLen;
					
					switch(oamOpcode)
						{
							case GW_OPCODE_EUQ_INFO_REQUESET:	
							case GW_OPCODE_EUQ_INFO_RESPONSE:	
							case GW_OPCODE_ALARM_OR_LOG_TRAN:	
							case GW_OPCODE_ALARM_OR_LOG_RESPONSE:	
							case GW_OPCODE_FILE_READ_AND_WRITE_REQUEST:	
							case GW_OPCODE_FILE_RESERVED_0:	
							case GW_OPCODE_FILE_DATA_TRAN:	
							case GW_OPCODE_FILE_TRANSFER_ACK:	
							case GW_OPCODE_FILE_RESERVED_1:	
							case GW_OPCODE_FILE_RESERVED_2:	
							case GW_OPCODE_SNMP_REQUEST	:	
							case GW_OPCODE_SNMP_RESPONSE:	
							case GW_OPCODE_SNMP_TRAP:	
							case GW_OPCODE_SNMP_RESERVED_1:	
							case GW_OPCODE_SNMP_RESERVED_2:	
							case GW_OPCODE_CLI_REQUEEST:	
							case GW_OPCODE_CLI_RESPONSE:	
							case GW_OPCODE_IGMPAUTH_REQUEST:	
							case Gw_OPCODE_IGMPAUTH_REPONSE:
#if (RPU_MODULE_IGMP_TVM == RPU_YES )
							case GW_OPCODE_TVM_REQUEST:
							case GW_OPCODE_TVM_RESPONSE:
#endif
							case GW_OPCODE_VCONSOLE:

								if(broadcast_llid != OAM_PON_EVERY_LLID )
								{
									llid = GetLlidByOnuIdx( OltId, OnuId);
									if ( OAM_INVALID_LLID == llid )
									{
										#ifdef OAM_DEBUG_PRINTF
										sys_console_printf("  -- before OAMIOClearQueue()\r\n");
										#endif
										OAMIOClearQueue(GW_MSG_SEND_QUIDX,bufpara);
										if( pMsgOam ) VOS_Free((VOID*)pMsgOam);
										pMsgOam = NULL;
										VOS_ASSERT(0);
										continue;
										/*break;*/
									}
								}
								
								if(1 == gGwOamTrackDebug)
								{
#if 0									
									unsigned short iLoop = 0;
									unsigned short tempLen = 0;
									
									if(sent_data_size > 60)
										tempLen = 60;
									else
										tempLen = sent_data_size;
#endif									
									sys_console_printf(" Send LLID(%d) OAM PAD length=%d \r\n", llid, sent_data_size);
									sys_console_printf(" Oam Content:\r\n");
#if 0									
									for(iLoop=0;iLoop<tempLen;iLoop++)
										{
											sys_console_printf("%02x ",sent_data[iLoop]);
										}
#endif
									/*print_pdu( sent_data, sent_data_size, 16 );*/
									pktDataPrintf( sent_data, sent_data_size );	/* modified by xieshl 20100805 */

									sys_console_printf("\r\n\r\n");
								}
#if 0
								/*modified by wutw 2007/01/17*/
								/*iRes = PAS_send_frame ( OltId, 
														sent_data_size, 
														OAM_PORT_SYSTEM, 
														llid, 
														OAM_SEND_SPECIFIED_LLID,
														sent_data);*/
								iRes = PAS_send_frame ( OltId, 
														sent_data_size, 
														PON_PORT_SYSTEM, 
														llid, 
														broadcast_llid,
														sent_data);
#else
								iRes = OLT_SendFrame2CNI( OltId, llid, (void*)sent_data, (int)sent_data_size );
#endif    


								if (iRes != OAM_OK)
								{
									#ifdef OAM_DEBUG_PRINTF
										sys_console_printf(" PAS_send_frame() error!\r\n");
									#endif
								}

								break;
							case Gw_OPCODE_OAM_DEBUG:
								debugSendCount++;
								break;								
							default :
								break;
						}

						/*释放缓冲区*/
						OAMIOClearQueue(GW_MSG_SEND_QUIDX,bufpara);
						if( pMsgOam ) VOS_Free((VOID*)pMsgOam);
						pMsgOam = NULL;
					
					break;

				case GW_MSG_REVEICE:
					OltId = pMsgOam->OltId;
					OnuId = pMsgOam->OnuId;		
                    llid  = pMsgOam->unicstLlid;	
					bufpara = pMsgOam->bufPara;
					oamOpcode = pMsgOam->oamOpcode;
					sent_data = gOamQueueCtl[GW_MSG_REV_QUIDX].DataBuff[bufpara].MesData;
					sent_data_size = gOamQueueCtl[GW_MSG_REV_QUIDX].DataBuff[bufpara].MesLen;	
					/*CommOamOpcodeFromOamGet(&oamOpcode, sent_data);*/
					gGwOamRevCount[oamOpcode]++;
					gGwOamTotalRevCount++;
					switch(oamOpcode)
						{	
							case GW_OPCODE_EUQ_INFO_REQUESET:	
							case GW_OPCODE_EUQ_INFO_RESPONSE:	
							case GW_OPCODE_ALARM_OR_LOG_TRAN:	
							case GW_OPCODE_ALARM_OR_LOG_RESPONSE:	
							case GW_OPCODE_FILE_READ_AND_WRITE_REQUEST:	
							case GW_OPCODE_FILE_RESERVED_0:	
							case GW_OPCODE_FILE_DATA_TRAN:	
							case GW_OPCODE_FILE_TRANSFER_ACK:	
							case GW_OPCODE_FILE_RESERVED_1:	
							case GW_OPCODE_FILE_RESERVED_2:	
							case GW_OPCODE_SNMP_REQUEST	:	
							case GW_OPCODE_SNMP_RESPONSE:	
							case GW_OPCODE_SNMP_TRAP:	
							case GW_OPCODE_SNMP_RESERVED_1:	
							case GW_OPCODE_SNMP_RESERVED_2:	
							case GW_OPCODE_CLI_REQUEEST:	
							case GW_OPCODE_CLI_RESPONSE:	
							case GW_OPCODE_IGMPAUTH_REQUEST:	
							case Gw_OPCODE_IGMPAUTH_REPONSE:
#if (RPU_MODULE_IGMP_TVM == RPU_YES )
							case GW_OPCODE_TVM_REQUEST:
							case GW_OPCODE_TVM_RESPONSE:
#endif
							case GW_OPCODE_VCONSOLE:
								if(1 == gGwOamTrackDebug)
								{
									/*int iLoop = 0;*/
									unsigned short tempLen = 0;
									if(sent_data_size > 180)
										tempLen = 180;
									else
										tempLen = sent_data_size;
									sys_console_printf(" Rev OAM PAD length=%d \r\n",sent_data_size);
									sys_console_printf(" Oam Content:\r\n");
#if 0									
									for(iLoop=0;iLoop<tempLen;iLoop++)
										{
											sys_console_printf("%02x ",sent_data[iLoop]);
										}
#endif
									/*print_pdu( sent_data, sent_data_size, 16 );
									sys_console_printf("\r\n\r\n");*/
									pktDataPrintf( sent_data, sent_data_size );	/* modified by xieshl 20100805 */
								}
								iRes = CommOamPayloadReveice ( OltId, OnuId, llid,(unsigned short)sent_data_size, sent_data) ;
								/*
								iRes = CommEhtFrameReveive ( OltId,
                                 										OnuId,
                                 										sent_data_size,
                                 										sent_data);*/
								if (iRes != OAM_OK)
									{
										if (1 == gGwOamTaskErrDebug)
										{
											sys_console_printf("  -------OAM Rev Err info -----\r\n");
											sys_console_printf("  CommOamPayloadReveice() err.PonId %d OnuId %d oamOpcode %d\r\n",OltId, OnuId,oamOpcode);
											Comm_Oam_ErrParse(iRes);

										}		
										
										#ifdef OAM_DEBUG_PRINTF
											sys_console_printf(" CommEhtFrameReveive() Error!\r\n");
										#endif
									}
								break;
							case Gw_OPCODE_OAM_DEBUG:
								iRes = CommOamPayloadReveice ( OltId, OnuId, llid, (unsigned short)(sent_data_size), (sent_data)) ;
								break;
							default:
								break;
						}/*end of switch (oamOpcode..)*/
				 
						/*释放缓冲区*/
						OAMIOClearQueue(GW_MSG_REV_QUIDX,bufpara);
						VOS_Free((VOID*)pMsgOam);
						pMsgOam = NULL;								
					break;
					
				default :
					oamOpcode = pMsgOam->oamOpcode;
					if (1 == gGwOamTaskErrDebug)
					{
						sys_console_printf("  Err! OAM Task unknow msg Type : %d \r\n",msgType);
						sys_console_printf("  PonId %d OnuId %d oamOpcode %d\r\n",OltId, OnuId,oamOpcode);
					}
					VOS_Free((VOID*)pMsgOam);
					ASSERT( 0 );
					break;
			}/*end of switch (msgType)...*/

			
	}/*end of FOREVER*/

}


/****************************************
* CommOnuMagInfoInit
* 描述: 该函数
*
*
*
*
****************************************/
short int CommOamMagInfoInit(void)
{
	unsigned int MaxHadle = 0;
	unsigned int OltId = 0;
	unsigned int QueueId = 0;
	unsigned int OnuId = 0;
	unsigned int ulSize;

	/*GW_MANAGEMENT_ONU_QUEUE_INFO_t	OnuQueue[GW_MAX_QUEUEID];*/
	/*gGwOamTaskErrDebug = 1;*/
	if (gboolGwInit == TRUE)
		{
		#ifdef OAM_DEBUG_PRINTF
		sys_console_printf(" oam was in Initail !\r\n");
		#endif
		return OAM_ERR;
		}

	if (!OamIOMsgQueueInit(/*OAM_TASK_QUEUE_NUM*/))
		{
		#ifdef OAM_DEBUG_PRINTF
		sys_console_printf("  OamIOMsgQueueInit() error!\r\n");
		#endif
		return OAM_ERR;
		}
    
#if 1
/*
#ifdef g_malloc
#undef g_malloc
#endif
*/
    ulSize = GW_MAX_OLTID * sizeof(GW_OLT_MANAGEMENT_QUEUE_t);
    if ( NULL == (goltMsg = (GW_OLT_MANAGEMENT_QUEUE_t*)g_malloc(ulSize)) )
    {
        VOS_ASSERT(0);

        return OAM_ERR;
    }
#endif
	
	gTaskDelayTime = GW_DEFAULT_TASKDELAY_TICK;
	CommOamPadSendsernoInit();
	/*memset(&gMsgWdInfo[0], 0, sizeof(GW_MSG_WDINFO_t)*(GW_MAX_OLTID*GW_MAX_ONUID*GW_MAX_QUEUEID));*/
	/*初始化回调函数组*/
	for (MaxHadle = 0; MaxHadle<GW_CALLBACK_LAST; MaxHadle++)
		handle_oamfuncs[MaxHadle]	= NULL;	

	/*初始化olt_msg数据结构用于存放接收的oam帧*/
	for (OltId = 0; OltId<GW_MAX_OLTID; OltId++)
		{
			memset(&goltMsg[OltId],0,sizeof(GW_OLT_MANAGEMENT_QUEUE_t));			
			for (OnuId = 0; OnuId<GW_MAX_ONUID; OnuId++)
				{
					for (QueueId = 0; QueueId<GW_MAX_QUEUEID; QueueId++)
						{
							goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuSemId =  VOS_SemBCreate(VOS_SEM_Q_FIFO, VOS_SEM_FULL);
							goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].Iused = FALSE;
							goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].RecvMsgTime = 0;
							goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad = NULL;
							
							/*   ---    
							gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuSemId =  VOS_SemBCreate(VOS_SEM_Q_FIFO, VOS_SEM_FULL);
							gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].Iused = FALSE;
							gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].RecvMsgTime = 0;
							gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad = NULL;
							*/
						}/*end of for (QueueId = ...*/

					/*gbFileRev[OltId][OnuId] = FALSE;*/
	
				}/*end offor (OnuId = */		
		}/*for (Count = */


		gSendNoSemId = VOS_SemBCreate(VOS_SEM_Q_FIFO, VOS_SEM_FULL);
		if (gSendNoSemId == 0)
			{
			#ifdef OAM_DEBUG_PRINTF
				sys_console_printf(" gSendNoSemId Error!\r\n");
			#endif
				return OAM_ERR;
			}
		
		gOamMsgQId = VOS_QueCreate(OAM_VOSTASK_QUEUE_NUM, VOS_MSG_Q_FIFO);
		if (gOamMsgQId == 0)
			{
			#ifdef OAM_DEBUG_PRINTF
				sys_console_printf(" gOamMsgQId Error!\r\n");
			#endif
				return OAM_ERR;
			}		
		gOamTaskId = ( VOS_HANDLE )VOS_TaskCreate( "tOamOltT", (ULONG)OAM_TASK_PRIO, (VOS_TASK_ENTRY)CommOamTask, NULL);	
		if (gOamTaskId == 0)
			{
			#ifdef OAM_DEBUG_PRINTF
				sys_console_printf(" gOamTaskId Error!\r\n");
			#endif
				return OAM_ERR;
			}

		VOS_QueBindTask(gOamTaskId, gOamMsgQId);
		pgGwOamMib = (OAM_MIB *)VOS_Malloc((ULONG)sizeof(OAM_MIB), (ULONG) MODULE_OAM);
		memset(pgGwOamMib, 0, sizeof(OAM_MIB));
		
		/* 注册debug回调函数*/
		CommOltMsgRvcCallbackInit(GW_CALLBACK_RESERVED, CommHadleReserved);


		gboolGwInit = TRUE;

#ifdef OAM_DEBUG_PRINTF
	sys_console_printf(" OAMInfoInit!\r\n");
#endif		
	return (OAM_OK);
}

void CommHadleReserved(const unsigned short   PonId,
                               const unsigned short	OnuId,
                               const unsigned short	llid,
                               const unsigned char 	GwProId,
                               const unsigned short   Length,
                               const unsigned char	*pFrame,
                               const unsigned char	*pSessionField)
{
	/*char cliTemp[1504] = {0};*/
	unsigned short   len = 0;

	if( Length > 1500 )  len = 120;
	else len = Length;
	/*memset(cliTemp, 0, 1504);		
	memcpy(cliTemp, pFrame, len);
	
	sys_console_printf("\r\n-----------------------------------------------\r\n");*/
	/*switch(GwProId)
	{
		case GW_CALLBACK_CLI_REQUEST_1:
		case GW_CALLBACK_CLI_1:
			{
		         short int  i;
				 unsigned char *pBuf = NULL;
		         sys_console_printf("  PonId %d onuId %d OAM OpCode %s\r\n", PonId,OnuId, OAMCodeToString[GwProId]);
		         sys_console_printf("  Recv msg len %d \r\n",  Length );
		         pBuf = (unsigned char *)cliTemp;
		         for( i = 0; i< len; i++ )
			 	 {
		             if( (i %20 )== 0 )
					  {
		                  sys_console_printf("\r\n");
		                  sys_console_printf("        ");
		              }
		              sys_console_printf(" %02x", *(unsigned char *)(pBuf+i) );                   
	              }
	    	      sys_console_printf("\r\n");	
			 }

			break;
		case GW_CALLBACK_RESERVED:	
		case GW_CALLBACK_EQU_REQUEST:
		case GW_CALLBACK_EUQINFO:
		case GW_CALLBACK_ALARMORLOG	:
		case GW_CALLBACK_ALARMORLOG_REPONSE:
		case GW_CALLBACK_FILE_RD_WRT_REQUEST:
		case GW_CALLBACK_FILE_RESERVED:
		case GW_CALLBACK_FILE_DATA_TRA:
		case GW_CALLBACK_FILE_TRA_ACK:
		case GW_CALLBACK_SNMP_REQUEST_1:
		case GW_CALLBACK_SNMP_REPONSE_1:
		case GW_CALLBACK_SNMP_TRAP_1:
		case GW_CALLBACK_IGMPAUTH_REQUEST:
		case GW_CALLBACK_IGMPAUTH_REPONST:
#if (RPU_MODULE_IGMP_TVM == RPU_YES )
		case GW_CALLBACK_TVM_REQUEST:
		case GW_CALLBACK_TVM_RESPONSE:
#endif
		case GW_CALLBACK_VCONSOLE:
			{
		         short int  i;
		         unsigned char *pBuf;
				 
		         sys_console_printf("  PonId %d onuId %d OAM OpCode %s\r\n", PonId,OnuId, OAMCodeToString[GwProId]);
		         sys_console_printf("  Recv msg len %d \r\n",  Length );
		         pBuf = (unsigned char *)cliTemp;
		         for( i = 0; i< len; i++ )
			 	 {
		             if( (i %20 )== 0 )
					  {
		                  sys_console_printf("\r\n");
		                  sys_console_printf("        ");
		              }
		              sys_console_printf(" %02x", *(unsigned char *)(pBuf+i) );                   
	              }
	    	      sys_console_printf("\r\n");	
				 
			 }
			break;
	    default:
			
			return;
	}*/

	if( GwProId < GW_CALLBACK_LAST )
	{
		sys_console_printf("  PonId %d onuId %d OAM OpCode %s\r\n", PonId,OnuId, OAMCodeToString[GwProId]);
		sys_console_printf("  Recv msg len %d \r\n",  Length );
		pktDataPrintf( pFrame, len );
	}
	return;
}

void CommSendReserved(const unsigned short   PonId,
                               const unsigned short	OnuId1,
                               const unsigned char 	GwProId,
                               const unsigned short   Length,
                               const unsigned char	*pFrame,
                               const unsigned char	*pSessionField)
{
	/*char cliTemp[1504] = {0};*/
	unsigned short   len = 0;
	unsigned short	OnuId = OnuId1;

	if( Length > 1500 )  len = 120;
	else len = Length;
	/*memset(cliTemp, 0, 1504);		
	memcpy(cliTemp, pFrame, len);*/

	OnuId++;
	/*sys_console_printf("\r\n-----------------------------------------------\r\n");*/
	sys_console_printf("\r\n");
	/*switch(GwProId)
	{
		case GW_CALLBACK_CLI_REQUEST_1:
		case GW_CALLBACK_CLI_1:
			{
		         short int  i;
				 unsigned char *pBuf = NULL;
		         sys_console_printf("  PonId %d onuId %d OAM OpCode %s\r\n", PonId, OnuId, OAMCodeToString[GwProId]);
		         sys_console_printf("  Sending msg len %d \r\n",  Length );
		         pBuf = (unsigned char *)cliTemp;
		         for( i = 0; i< len; i++ )
			 	 {
		             if( (i %20 )== 0 )
					  {
		                  sys_console_printf("\r\n");
		                  sys_console_printf("        ");
		              }
		              sys_console_printf(" %02x", *(unsigned char *)(pBuf+i) );                   
	              }
	    	      sys_console_printf("\r\n");	
			 }

			break;
		case GW_CALLBACK_RESERVED:	
		case GW_CALLBACK_EQU_REQUEST:
		case GW_CALLBACK_EUQINFO:
		case GW_CALLBACK_ALARMORLOG	:
		case GW_CALLBACK_ALARMORLOG_REPONSE:
		case GW_CALLBACK_FILE_RD_WRT_REQUEST:
		case GW_CALLBACK_FILE_RESERVED:
		case GW_CALLBACK_FILE_DATA_TRA:
		case GW_CALLBACK_FILE_TRA_ACK:
		case GW_CALLBACK_SNMP_REQUEST_1:
		case GW_CALLBACK_SNMP_REPONSE_1:
		case GW_CALLBACK_SNMP_TRAP_1:
		case GW_CALLBACK_IGMPAUTH_REQUEST:
		case GW_CALLBACK_IGMPAUTH_REPONST:
#if (RPU_MODULE_IGMP_TVM == RPU_YES )
		case GW_CALLBACK_TVM_REQUEST:
		case GW_CALLBACK_TVM_RESPONSE:
#endif
		case GW_CALLBACK_VCONSOLE:
			{
		         short int  i;
		         unsigned char *pBuf;
				 
		         sys_console_printf("  PonId %d onuId %d OAM OpCode %s\r\n", PonId, OnuId, OAMCodeToString[GwProId]);
		         sys_console_printf("  Sending msg len %d \r\n",  Length );
		         pBuf = (unsigned char *)cliTemp;
		         for( i = 0; i< len; i++ )
			 	 {
		             if( (i %20 )== 0 )
					  {
		                  sys_console_printf("\r\n");
		                  sys_console_printf("        ");
		              }
		              sys_console_printf(" %02x", *(unsigned char *)(pBuf+i) );                   
	              }
	    	      sys_console_printf("\r\n");	
				 
			 }

			sys_console_printf("  PonId %d onuId %d OAM OpCode %s\r\n", PonId, OnuId, OAMCodeToString[GwProId]);
			sys_console_printf("  Sending msg len %d \r\n",  Length );
			pktDataPrintf( pFrame, len );

			break;
	    default:
			return;
	}*/
	if( GwProId < GW_CALLBACK_LAST )
	{
		sys_console_printf("  PonId %d onuId %d OAM OpCode %s\r\n", PonId, OnuId, OAMCodeToString[GwProId]);
		sys_console_printf("  Sending msg len %d \r\n",  Length );
		pktDataPrintf( pFrame, len );
	}
	return;
}

#if 0
#endif



/*==================接收和发送队列模块==================*/


/*********************************************************
 函数名: 
 描  述: 清除发送或者接收缓冲区
 
 输入参数:    aM_uint8_QueIdx  --数据缓冲区类型,接收/发送
 				aM_int32_DatIdx  --缓冲区数据池号
 			     
输出参数:  

 
 返回值: 返回值:TRUE
 
 说  明：                                         
*********************************************************/
BOOL OAMIOClearQueue( unsigned char aqueIdx,unsigned int datIdx)
{
        unsigned int            lM_uint32_Idx=0;
		
	lM_uint32_Idx = datIdx;
	if( lM_uint32_Idx >= gOamQueueCtl[aqueIdx].MaxSize ) return( FALSE );

	VOS_SemTake(gOamQueueCtl[aqueIdx].DataSem,WAIT_FOREVER);
       gOamQueueCtl[aqueIdx].DataBuff[lM_uint32_Idx].IsUsed=FALSE;
	   gOamQueueCtl[aqueIdx].WriteIndex=lM_uint32_Idx;
	VOS_SemGive(gOamQueueCtl[aqueIdx].DataSem);
	 return(TRUE);
}

/*********************************************************
 函数名: 
 描  述: 该函数从公用数据接收缓冲区读数据至协议数据接收
 		buff
 
 输入参数:    aM_uint8_QueIdx  --数据缓冲区类型,接收/发送
				aP_uchar_Buff  -- 接收数据缓冲区的指针
 				aM_uint32_Len  --接收数据的长度byte单位
 			     
输出参数:  lM_uint32_Idx 缓冲区数据池号

 返回值: lM_uint32_Idx
 
 说  明：该函数不会重写环形数据缓冲区
 *			if(gOamQueueCtl[aM_uint8_QueIdx].DataBuff[gOamQueueCtl[aM_uint8_QueIdx].WriteIndex].IsUsed)
 *			if(gOamQueueCtl[aM_uint8_QueIdx].WriteIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            		{
				gOamQueueCtl[aM_uint8_QueIdx].WriteIndex=0;
			}
*			如上。如果没有释放这些缓冲区，则将不会接收任何数据
*********************************************************/
unsigned int  OamIOWriteDataToQueue( unsigned char aM_uint8_QueIdx, 
										UCHAR *aP_uchar_Buff,
										unsigned int aM_uint32_Len)
{
        UINT32            lM_uint32_Idx=0;
	 UINT32            lM_uint32_Cir=0;

     lM_uint32_Idx=gOamQueueCtl[aM_uint8_QueIdx].MaxSize;

	 if ((aM_uint32_Len<=0)||(aM_uint32_Len>1580)) 
	 	return(lM_uint32_Idx);

	 semTake(gOamQueueCtl[aM_uint8_QueIdx].DataSem,WAIT_FOREVER);
	
     for(lM_uint32_Cir=0;lM_uint32_Cir<gOamQueueCtl[aM_uint8_QueIdx].MaxSize;lM_uint32_Cir++)
	 {
	 	/*当为真表示该缓冲区已经被占用*/
		 if(gOamQueueCtl[aM_uint8_QueIdx].DataBuff[gOamQueueCtl[aM_uint8_QueIdx].WriteIndex].IsUsed)
		 {
    			gOamQueueCtl[aM_uint8_QueIdx].WriteIndex++;

			if(gOamQueueCtl[aM_uint8_QueIdx].WriteIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            		{
				gOamQueueCtl[aM_uint8_QueIdx].WriteIndex=0;
			}
 		 }
		 else 
		 {
            		lM_uint32_Idx=gOamQueueCtl[aM_uint8_QueIdx].WriteIndex;

            		gOamQueueCtl[aM_uint8_QueIdx].WriteIndex++;

			if(gOamQueueCtl[aM_uint8_QueIdx].WriteIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            		{
				gOamQueueCtl[aM_uint8_QueIdx].WriteIndex=0;
			}
			break;
		 }
	 }

	 if (lM_uint32_Idx<gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
	 {
		 memcpy(gOamQueueCtl[aM_uint8_QueIdx].DataBuff[lM_uint32_Idx].MesData,aP_uchar_Buff,aM_uint32_Len);
         	 gOamQueueCtl[aM_uint8_QueIdx].DataBuff[lM_uint32_Idx].IsUsed=TRUE;
		 gOamQueueCtl[aM_uint8_QueIdx].DataBuff[lM_uint32_Idx].MesLen=aM_uint32_Len;

     	}
	 /*IGMP_datatoqueue_end = lM_uint32_Idx;*/
     semGive(gOamQueueCtl[aM_uint8_QueIdx].DataSem);
	

	 return(lM_uint32_Idx);
}

/*********************************************************
 函数名: 
 描  述: 该函数从公用数据接收缓冲区接收数据并解析协议数据包
 
 输入参数:    
				aM_uint8_Port  --端口号
				aM_uint8_VlanId --vlan号
				aP_uchar_Buff  -- 接收数据缓冲区的指针
 				aM_uint32_Len  --接收数据的长度byte单位
 			     
输出参数:  lM_uint32_Idx 缓冲区数据池号

 返回值: lM_uint32_Idx
 
 说  明：                                         
*********************************************************/
ULONG __oam_stat_rx = 0;
ULONG __oam_stat_tx = 0;
short int OamIODataSend(unsigned short OltId, 
						unsigned short OnuId, 
						unsigned char *pBuf,
						unsigned int msgType,
						unsigned int  bufLen,
						GW_OAM_MSG_t  *pOamMsgStru)
{
	unsigned int  queueIdx = 0;
	ULONG ulMsg[4] = {0};
	GW_OAM_MSG_t  *pOamMsg;
	if (NULL == pOamMsgStru)
		{
		return OAM_NULL_POINTER;
		}

	ulMsg[2] = msgType;
	
	if (GW_MSG_SEND == msgType)
		{
		__oam_stat_tx++;
        	queueIdx=OamIOWriteDataToQueue(GW_MSG_SEND_QUIDX, pBuf,bufLen);
			if (queueIdx>= gOamQueueCtl[GW_MSG_SEND_QUIDX].MaxSize)
			{
				if(gGwOamTaskErrDebug == 1)
				{
					sys_console_printf("  PonId %d OnuId %d OAM OpCode %d\r\n",OltId,OnuId,pOamMsgStru->oamOpcode);
					sys_console_printf("  OAM_INVALUE_QUEUE_ERR error.msgType %d(1-send, 2-rev ) \r\n",msgType);		
				}
				ASSERT( 0 );
				return OAM_INVALUE_QUEUE_ERR;
			}	
		}
	else if (GW_MSG_REVEICE == msgType)
		{
		__oam_stat_rx++;
			queueIdx=OamIOWriteDataToQueue(GW_MSG_REV_QUIDX, pBuf,bufLen);
			if (queueIdx>= gOamQueueCtl[GW_MSG_REV_QUIDX].MaxSize)
			{
				if(gGwOamTaskErrDebug == 1)
				{
					sys_console_printf("  PonId %d OnuId %d OAM OpCode %d\r\n",OltId,OnuId,pOamMsgStru->oamOpcode);
					sys_console_printf("  OAM_INVALUE_QUEUE_ERR error.msgType %d(1-send, 2-rev ) \r\n",msgType);		
				}
				ASSERT( 0 );
				return OAM_INVALUE_QUEUE_ERR;
			}	
		}
	else
		{	
			if(gGwOamTaskErrDebug == 1)
			{
				sys_console_printf("  PonId %d OnuId %d OAM OpCode %d\r\n",OltId,OnuId,pOamMsgStru->oamOpcode);
				sys_console_printf("  Error. Unknow msg type :%d(1-send, 2-rev ) \r\n",msgType);
			}
			return OAM_TASK_MSGTYPE_UNKNOW_ERR;
		}
	/*if (queueIdx>= gOamQueueCtl[GW_MSG_SEND_QUIDX].MaxSize)
	{
		if(gGwOamTaskErrDebug == 1)
		{
			sys_console_printf("  PonId %d OnuId %d OpCode %d\r\n",OltId,OnuId,pOamMsgStru->oamOpcode);
			sys_console_printf("  OAM_INVALUE_QUEUE_ERR error.msgType %d(1-send, 2-rev ) \r\n",msgType);		
		}
		return OAM_INVALUE_QUEUE_ERR;
	}*/
	if(1 == gGwOamTrackDebug)
	{
		sys_console_printf("  PonId %d OnuId %d OAM OpCode %d msgType %d(1-send, 2-rev ) \r\n",\
						OltId,OnuId,pOamMsgStru->oamOpcode,msgType);
		if (GW_MSG_SEND == msgType)
			sys_console_printf("  OAM_MSG_SEND ! To Oam task\r\n");
		else if(msgType == GW_MSG_REVEICE)
			sys_console_printf("  OAM_MSG_REVEICE ! To Oam task\r\n");
		else
			sys_console_printf("  Error. Unknow msg type :%d(1-send, 2-rev ) \r\n",msgType);
	
	}
	/*获取发送sendserno*/
	/*if (OAM_OK != CommOamPadSendsernoGet  (&sendserno))
		return (FILE_CTRL_ERRCODE_FLOWERR);*/
	/*pOamMsgStru.sendSerNo = sendserno;*/	
	
	pOamMsgStru->bufPara = queueIdx;	
	pOamMsg = (GW_OAM_MSG_t *)VOS_Malloc((ULONG)sizeof(GW_OAM_MSG_t), (ULONG)MODULE_OAM);
	if (NULL == pOamMsg)
	{
		if(msgType == GW_MSG_SEND)
		{
			OAMIOClearQueue(GW_MSG_SEND_QUIDX, queueIdx);
		}
		else if(msgType == GW_MSG_REVEICE)
		{
			OAMIOClearQueue(GW_MSG_REV_QUIDX, queueIdx);
		}	
		ASSERT( 0 );
		return OAM_MALLOC_ERR;
	}
	
	memcpy(pOamMsg, pOamMsgStru, sizeof(GW_OAM_MSG_t));
	
	ulMsg[3] = (ULONG)(pOamMsg);
	
	if (OAM_OK == VOS_QueSend( gOamMsgQId, ulMsg, NO_WAIT, MSG_PRI_NORMAL ))
		{
	     	return(OAM_OK);
		}
	else
		{
			VOS_Free((VOID *)pOamMsg);
			pOamMsg = NULL;
			if(msgType == GW_MSG_SEND)
			{
				OAMIOClearQueue(GW_MSG_SEND_QUIDX, queueIdx);
			}
			else if(msgType == GW_MSG_REVEICE)
			{
				OAMIOClearQueue(GW_MSG_REV_QUIDX, queueIdx);
			}			
			if(gGwOamTaskErrDebug == 1)
			{
				if(msgType == GW_MSG_SEND)
				{
				sys_console_printf("  OamIODataSend() ERROR! Type:OAM_MSG_SEND !\r\n");
				}
				else if(msgType == GW_MSG_REVEICE)
				{
				sys_console_printf("  OamIODataSend() ERROR! Type:OAM_MSG_REVEICE !\r\n");
				}
			}	
			ASSERT( 0 );
			return(OAM_TASK_MSGSEND_FAIL_ERR);
		 }
	
}


/*********************************************************
 函数名: OnuIODataSend
 描  述: 该函数从公用数据接收缓冲区接收数据并解析协议数据包
 
 输入参数:    
				aP_uchar_Buff  -- 接收数据缓冲区的指针
 				aM_uint32_Len  --接收数据的长度byte单位
 			     
输出参数:  lM_uint32_Idx 缓冲区数据池号

 返回值: lM_uint32_Idx
 
 说  明：                                         
*********************************************************/
short int OnuIODataSend(unsigned short OltId, 
						unsigned short OnuId, 
						unsigned char *pBuf,
						unsigned int msgType,
						unsigned int  bufLen,
						GW_OAM_MSG_t  *pOamMsgStru)
{
	unsigned int  queueIdx = 0;
	ULONG ulMsg[4] = {0};
	GW_OAM_MSG_t  *pOamMsg;
	if (NULL == pOamMsgStru)
		{
		return OAM_ERR;
		}
	/*
	pOamMsgStru->OltId = OltId;
	pOamMsgStru->OnuId = OnuId;				
	pOamMsgStru->brdcstLlid= BrdcstLlid;
	pOamMsgStru->oamOpcode = oamOpcode;
	pOamMsgStru->sendSerNo = lSendSerNo;
	*/
	ulMsg[2] = msgType;
	
	if (GW_MSG_SEND == msgType)
		{
        		queueIdx=OamIOWriteDataToQueue(GW_MSG_SEND_QUIDX, pBuf,bufLen);
		}
	else if (GW_MSG_REVEICE == msgType)
		{
			queueIdx=OamIOWriteDataToQueue(GW_MSG_REV_QUIDX, pBuf,bufLen);
		}
	else
		{
			return OAM_ERR;
		}
	
	pOamMsgStru->bufPara = queueIdx;	
	pOamMsg = (GW_OAM_MSG_t *)VOS_Malloc((ULONG)sizeof(GW_OAM_MSG_t), (ULONG)MODULE_OAM);
	if( pOamMsg == NULL )		/* added by xieshl 20090729 */
	{
	  	 OAMIOClearQueue(GW_MSG_SEND_QUIDX, queueIdx);
		return OAM_ERR;
	}
	memcpy(pOamMsg, pOamMsgStru, sizeof(GW_OAM_MSG_t));
	ulMsg[3] = (ULONG)(pOamMsg);
	
	if (queueIdx<gOamQueueCtl[GW_MSG_SEND_QUIDX].MaxSize)
	 {
#ifdef OAM_DEBUG

	if (OAM_OK == msgQSend(gOamOnuMsgQId, (char *)ulMsg, (sizeof(ULONG)*4), NO_WAIT,MSG_PRI_NORMAL))
#else
		
		if (OAM_OK == VOS_QueSend( gOamOnuMsgQId, ulMsg, NO_WAIT, MSG_PRI_NORMAL ))
#endif

		  {
             		return(OAM_OK);
		  }
		  else
		  {
			VOS_Free((VOID *)pOamMsg);
			pOamMsg = NULL;		  
		  	/*sys_console_printf(" OnuIODataSend()  ERROR! \r\n");*/
		  	 OAMIOClearQueue(GW_MSG_SEND_QUIDX, queueIdx);
			 ASSERT( 0 );
			 return(OAM_ERR);
		  }
	 }
	 else
	 	{
			VOS_Free((VOID *)pOamMsg);
			pOamMsg = NULL;		 	
	 		return (OAM_ERR);
	 	}

}


/*********************************************************
 函数名: 
 描  述: 该函数消息度列,非配信号量
 
 输入参数:    
				aM_uint8_Port  --端口号
 			     
输出参数:  

 返回值: true/false
 
 说  明：                                         
*********************************************************/
BOOL OamIOMsgQueueInit(void/*unsigned short maxBuf*/)
{
	UINT8  lM_uint8_Cir=0;
    ULONG  ulSize;
    CHAR  *pByteBuf;

    ulSize = sizeof(STRU_OAM_FRAME_QUEUE)*OAM_TASK_QUEUE_NUM;
#if 1
/*
#ifdef g_malloc
#undef g_malloc
#endif
*/
    if ( NULL == (pByteBuf = (CHAR*)g_malloc(ulSize * 2)) )
    {
        VOS_ASSERT(0);
		return(FALSE);
    }
    oamSendQueue = (STRU_OAM_FRAME_QUEUE*)pByteBuf;
    oamRevQueue  = (STRU_OAM_FRAME_QUEUE*)(pByteBuf + ulSize);  
#endif
    
	VOS_MemSet(oamSendQueue, 0, ulSize);
	VOS_MemSet(oamRevQueue, 0, ulSize);
	for( lM_uint8_Cir=0;lM_uint8_Cir<2;lM_uint8_Cir++)
	{
	    gOamQueueCtl[lM_uint8_Cir].MaxSize=OAM_TASK_QUEUE_NUM;
	    gOamQueueCtl[lM_uint8_Cir].WriteIndex=0;
	    gOamQueueCtl[lM_uint8_Cir].ReadIndex=0;
		if (lM_uint8_Cir == 0)
	    	gOamQueueCtl[lM_uint8_Cir].DataBuff=&(oamSendQueue[0]);		
		else
			gOamQueueCtl[lM_uint8_Cir].DataBuff=&(oamRevQueue[0]);		
		/*if (lM_uint8_Cir == 0)
	    	gOamQueueCtl[lM_uint8_Cir].DataBuff=(STRU_OAM_FRAME_QUEUE*)calloc(maxBuf,sizeof(STRU_OAM_FRAME_QUEUE));		
		else
			gOamQueueCtl[lM_uint8_Cir].DataBuff=(STRU_OAM_FRAME_QUEUE*)calloc(maxBuf,sizeof(STRU_OAM_FRAME_QUEUE));*/
#ifdef OAM_DEBUG		
	    gOamQueueCtl[lM_uint8_Cir].DataSem=semMCreate (SEM_Q_FIFO);
	    if((gOamQueueCtl[lM_uint8_Cir].DataSem==NULL)||(gOamQueueCtl[lM_uint8_Cir].DataBuff==NULL)) 
#else
 	    gOamQueueCtl[lM_uint8_Cir].DataSem=VOS_SemMCreate(VOS_SEM_Q_FIFO);

	    if((gOamQueueCtl[lM_uint8_Cir].DataSem==0)||(gOamQueueCtl[lM_uint8_Cir].DataBuff==NULL)) 
#endif
	    	{
			return(FALSE);
	    	}
      
	}
	return (TRUE);
}


/*********************************************************
 函数名: 
 描  述: 该函数从协议接收数据缓冲区写数据到协议数据发送缓冲区
 
 输入参数:    aM_uint8_QueIdx  --数据缓冲区类型,接收/发送
				aM_uint8_Port  --端口号
				aM_uint8_VlanId --vlan号
				aP_uchar_Buff  -- 数据缓冲区的指针
 				aM_uint32_Len  --接收数据的长度byte单位
 			     
输出参数:  lM_uint32_Idx 缓冲区数据池号

 返回值: lM_uint32_Idx
 
 说  明：                                         
*********************************************************/
UINT32 OamIOReadDataFromQueue(UINT8 aM_uint8_QueIdx,
									 UCHAR *aP_uchar_Buff,
									 UINT32 aP_uint32_Len)
{

	 UINT32     lM_uint32_Cir=0;
	 UINT32		lM_uint32_Idx = 0;

	 semTake(gOamQueueCtl[aM_uint8_QueIdx].DataSem,WAIT_FOREVER);

     for(lM_uint32_Cir=0;lM_uint32_Cir<gOamQueueCtl[aM_uint8_QueIdx].MaxSize;lM_uint32_Cir++)
	 {
	 	/*当为真表示该缓冲区已经被占用*/
		 if(gOamQueueCtl[aM_uint8_QueIdx].DataBuff[gOamQueueCtl[aM_uint8_QueIdx].ReadIndex].IsUsed)
		 {
    			gOamQueueCtl[aM_uint8_QueIdx].ReadIndex++;

			if(gOamQueueCtl[aM_uint8_QueIdx].ReadIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            		{
				gOamQueueCtl[aM_uint8_QueIdx].ReadIndex=0;
			}
 		 }
		 else 
		 {    
            		lM_uint32_Idx = gOamQueueCtl[aM_uint8_QueIdx].ReadIndex;

            		gOamQueueCtl[aM_uint8_QueIdx].ReadIndex++;

			if(gOamQueueCtl[aM_uint8_QueIdx].ReadIndex>=gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
            		{
				gOamQueueCtl[aM_uint8_QueIdx].ReadIndex=0;
			}
			break;
		 }
	 }

	 if (lM_uint32_Idx<gOamQueueCtl[aM_uint8_QueIdx].MaxSize)
	 {
		 memcpy(gOamQueueCtl[aM_uint8_QueIdx].DataBuff[lM_uint32_Idx].MesData,aP_uchar_Buff,aP_uint32_Len);
         	 gOamQueueCtl[aM_uint8_QueIdx].DataBuff[lM_uint32_Idx].IsUsed=TRUE;
		 gOamQueueCtl[aM_uint8_QueIdx].DataBuff[lM_uint32_Idx].MesLen=aP_uint32_Len;

     	}
	 semGive(gOamQueueCtl[aM_uint8_QueIdx].DataSem);

	 return(lM_uint32_Idx);
}


/*********************************************************
 函数名: 
 描  述: 该函数向写协议数据发送缓冲区写数据
 
 输入参数:    aM_uint8_QueIdx  --数据缓冲区类型,接收/发送
				aM_uint8_Port  --端口号
				aM_int32_DatIdx --数据缓冲区号
 				aM_uint32_Len  --接收数据的长度byte单位
 			     
输出参数:  lM_uint32_Idx 缓冲区数据池号

 返回值: lM_uint32_Idx
 
 说  明：                                         
*********************************************************/
UINT32 OamIOReadOutIgmpsysData(UINT8 aM_uint8_QueIdx,
										UINT32 aM_int32_DatIdx,
										UINT32 aP_uint32_Len)
{
	UCHAR *aP_uchar_Buff;
	semTake(gOamQueueCtl[0].DataSem,WAIT_FOREVER);	
	aP_uchar_Buff = gOamQueueCtl[0].DataBuff[aM_int32_DatIdx].MesData;
	semGive(gOamQueueCtl[0].DataSem);	

	
	return(OamIOReadDataFromQueue(aM_uint8_QueIdx,aP_uchar_Buff,aP_uint32_Len));
}

#if 0
short int OamIODataSend(unsigned short OltId, 
						unsigned short OnuId, 
						unsigned char *pBuf,
						unsigned int msgType,
						unsigned int  bufLen)
{
	unsigned int  queueIdx = 0;
	ULONG ulMsg[4] = {0};
	GW_OAM_MSG_t  *pOamMsg;

	/*
	pOamMsgStru->OltId = OltId;
	pOamMsgStru->OnuId = OnuId;				
	pOamMsgStru->brdcstLlid= BrdcstLlid;
	pOamMsgStru->oamOpcode = oamOpcode;
	pOamMsgStru->sendSerNo = lSendSerNo;
	*/
	
	
	if (GW_MSG_SEND == msgType)
		{
        		queueIdx=OamIOWriteDataToQueue(GW_MSG_SEND_QUIDX, pBuf,bufLen);
		}
	else if (GW_MSG_REVEICE == msgType)
		{
			queueIdx=OamIOWriteDataToQueue(GW_MSG_REV_QUIDX, pBuf,bufLen);
		}
	else
		{
			return OAM_ERR;
		}
	
	pOamMsg = (GW_OAM_MSG_t *)VOS_Malloc((ULONG)sizeof(GW_OAM_MSG_t), (ULONG)MODULE_OAM);
	if (NULL == pOamMsg)
		return OAM_ERR;
	memset(pOamMsg, 0, sizeof(GW_OAM_MSG_t));
	pOamMsg->bufPara = queueIdx;
	pOamMsg->OltId = OltId;
	pOamMsg->OnuId = OnuId;
	ulMsg[2] = msgType;
	ulMsg[3] = (ULONG)(pOamMsg);
	
	if (queueIdx<gOamQueueCtl[GW_MSG_REV_QUIDX].MaxSize)
	 {
	if (OAM_OK == VOS_QueSend( gOamMsgQId, ulMsg, NO_WAIT, MSG_PRI_NORMAL ))
		  {
             		return(OAM_OK);
		  }
		  else
		  {
			 VOS_Free((VOID *)pOamMsg);
			 pOamMsg = NULL;		  
		  	 OAMIOClearQueue(GW_MSG_REV_QUIDX,queueIdx);
			 ASSERT( 0 );
			 return(OAM_ERR);
		  }
	 }
	 else
	 	{
			VOS_Free((VOID *)pOamMsg);
			pOamMsg = NULL;	 	
	 		return (OAM_ERR);
	 	}

}
#endif

int  Comm_Cli_request_transmit_new(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char  *pClibuf,
				const unsigned short cli_data_size,
				unsigned char *psessionIdField)
{
	int iRes = OAM_PONID_NOTWORK;
	unsigned long lSendSerNo = 0;
	short int broadcast_llid;
    short int iPonBase;
    short int iPonMax;
	short int tempOnuId;

	if(pClibuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;	
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;	

    if ( 0xFFFF == OnuId )
    {
        /* PON口广播*/
        broadcast_llid = OAM_PON_EVERY_LLID;
        tempOnuId = 0;
    }
    else
    {
        /* PON口单播*/
        broadcast_llid = OAM_SEND_SPECIFIED_LLID;        
        tempOnuId = OnuId - 1;
    }

    if ( 0xFFFF == PonId )
    {
        /* 全OLT的PON口广播*/
        iPonMax = MAXPON - 1;
        iPonBase = 0;
    }
    else
    {
        iPonBase = PonId;
        iPonMax = iPonBase;
    }
    
	
    for (; iPonBase<=iPonMax; ++iPonBase)
    {
        if (0xFFFF != OnuId)
        {
            /* 非广播ONU,则检查ONU连接状态 */
        	if ( OAM_INVALID_LLID == GetLlidByOnuIdx(iPonBase, tempOnuId) )
        	{
        		continue;
        	}
        }
        
        if (0xFFFF == PonId)
        {
            /* ---此PON口正常，且注册ONU数目不为0 --- */
            if( 0 >= GetPonPortCurrentOnuNum(iPonBase) )
        	{
        		continue;
        	}
        }

    	iRes = COMM_OAM_frame_send( 	iPonBase, 
    									tempOnuId,
    									GW_OPCODE_CLI_REQUEEST,
    									broadcast_llid,
    									pClibuf, 
    									cli_data_size, 
    									psessionIdField,
    									GW_DEVTYPE_OLT, 
    									lSendSerNo);	
    	if( iRes != OAM_OK)
    	{
    		if (1 == gGwOamTaskErrDebug)
    		{
    			sys_console_printf("  -------OAM Send Err info -----\r\n");
    			Comm_Oam_ErrParse(iRes);
    		}
    	}
    }

	return iRes;	
}


/*=====================CLI接口定义=========================*/
/* B--added by liwei056@2009-3-5 for BroadCast-Cli */
#if 0
#include "PonGeneral.h"

static char s_aBroadCastPonIDBuf[1 + SYS_MAX_PON_PORT_BITNUM] = {0};
char* g_paBroadCastPonIDs = s_aBroadCastPonIDBuf + 1;
int   g_iBroadCastFlags = 0;
#endif

int  Comm_Cli_request_transmit2(
                struct vty * vty,
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char  *pClibuf,
				const unsigned short cli_data_size,
				unsigned char *psessionIdField)
{
	int iRes = OAM_PONID_NOTWORK;
	unsigned long lSendSerNo = 0;
	short int broadcast_llid;
    short int iPonBase;
    short int iPonMax;
	short int curPon;
	short int curOnuId;
	short int tempOnuId;

	if(pClibuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;	
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;	

    curPon = vty->iSysCbFunArg1;
    curOnuId = vty->iSysCbFunArg2 - 1;

    if ( 0xFFFF == OnuId )
    {
        /* PON口广播*/
        broadcast_llid = OAM_PON_EVERY_LLID;

        tempOnuId = 0;
    }
    else
    {
        /* PON口单播*/
        broadcast_llid = OAM_SEND_SPECIFIED_LLID;
        
        tempOnuId = OnuId - 1;
    }

    if ( 0xFFFF == PonId )
    {
        /* 全OLT的PON口广播*/
        iPonMax = MAXPON - 1;
        iPonBase = 0;
    }
    else
    {
        iPonBase = PonId;
        iPonMax = iPonBase;
    }
    
	
    for (; iPonBase<=iPonMax; ++iPonBase)
    {
        if (0xFFFF != OnuId)
        {
            /* 非广播ONU,则检查ONU连接状态 */
        	if ( OAM_INVALID_LLID == GetLlidByOnuIdx(iPonBase, tempOnuId) )
        	{
        		continue;
        	}
        }
        
        if (0xFFFF == PonId)
        {
            /* 此PON口允许广播命令 */
            if (!BROADCAST_VTY_GETID(vty, iPonBase))
        	{
        		continue;
        	}

            /* ---此PON口正常，且注册ONU数目不为0 --- */
            if( 0 >= GetPonPortCurrentOnuNum(iPonBase) )
        	{
        		continue;
        	}
        }

        if ( curPon == iPonBase )
        {
            /* 标识当前PON口已下发过命令 */
            curPon = -1;
        }
    	iRes = COMM_OAM_frame_send( 	iPonBase, 
    									tempOnuId,
    									GW_OPCODE_CLI_REQUEEST,
    									broadcast_llid,
    									pClibuf, 
    									cli_data_size, 
    									psessionIdField,
    									GW_DEVTYPE_OLT, 
    									lSendSerNo);	
    	if( iRes != OAM_OK)
    	{
    		if (1 == gGwOamTaskErrDebug)
    		{
    			sys_console_printf("  -------OAM Send Err info -----\r\n");
    			Comm_Oam_ErrParse(iRes);
    		}
    	}
    }

    if ( curPon >= 0 )
    {
    	if ( OAM_INVALID_LLID != GetLlidByOnuIdx(curPon, curOnuId) )
        {
        	iRes = COMM_OAM_frame_send( 	curPon, 
        									curOnuId,
        									GW_OPCODE_CLI_REQUEEST,
        									OAM_SEND_SPECIFIED_LLID,
        									pClibuf, 
        									cli_data_size, 
        									psessionIdField,
        									GW_DEVTYPE_OLT, 
        									lSendSerNo);	
        	if( iRes != OAM_OK)
        	{
        		if (1 == gGwOamTaskErrDebug)
        		{
        			sys_console_printf("  -------OAM Send Err info -----\r\n");
        			Comm_Oam_ErrParse(iRes);
        		}
        	}
        }   
    }

	return iRes;	
}
/* E--added by liwei056@2009-3-5 for BroadCast-Cli */

/* B--added by liwei056@2009-3-19 for BroadCast-Cli-Commit */
int Comm_Cli_request_broadcastRefuse( struct vty * vty )
{
	int iRes;
    unsigned short PonId;
    unsigned short OnuId;
	short int llid;
	unsigned long lSendSerNo = 0;
	cliPayload stPayload;

    VOS_ASSERT(vty);
    VOS_ASSERT(vty->prev_length > 0);
    VOS_ASSERT(vty->prev_clistr);
    
    /* 为了不让ONU都回送命令响应，应该设置一个只发的FD，需要ONU的配合 */
	stPayload.fd = vty->fd;	
    PonId = vty->iSysCbFunArg1;
    OnuId = vty->iSysCbFunArg2 - 1;
	llid = GetLlidByOnuIdx( PonId, OnuId);
	if ( OAM_INVALID_LLID == llid )
	{
		return OAM_ONU_DEREGISTER_ERR;
	}

	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;	

	iRes = COMM_OAM_frame_send( 	PonId, 
									OnuId,
									GW_OPCODE_CLI_REQUEEST,
									OAM_SEND_SPECIFIED_LLID,
									vty->prev_clistr, 
									vty->prev_length, 
									(unsigned char *)&stPayload,
									GW_DEVTYPE_OLT, 
									lSendSerNo);	
    VOS_Free(vty->prev_clistr);
    vty->prev_clistr = NULL;
    vty->prev_length = 0;
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);
		}

        return iRes;
	}

    if (vty->action_SysCBfunc)
    {
        vty->iSysCbFunArg1 = stPayload.fd;
        if ( VOS_OK == (iRes = vty->action_SysCBfunc(vty)) )
        {
            if (vty->action_UsrCBfunc)
            {
                iRes = vty->action_UsrCBfunc(vty);
                vty->action_UsrCBfunc = NULL;
            }
        }
        vty->action_SysCBfunc = NULL;
    }

    return iRes;
}

int Comm_Cli_request_broadcastCommit( struct vty * vty )
{
    int iRes;
	cliPayload stPayload;

    VOS_ASSERT(vty);
    VOS_ASSERT(vty->prev_length > 0);
    VOS_ASSERT(vty->prev_clistr);
    
    /* B--modified by liwei056@2011-9-16 for D13446 */
#if 0    
    /* 为了不让ONU都回送命令响应，应该设置一个只发的FD，需要ONU的配合 */
	stPayload.fd = 0;
#else
    /* 为了得到ONU命令响应，必须设置其接收FD */
	stPayload.fd = vty->fd;	
#endif
    /* E--modified by liwei056@2011-9-16 for D13446 */

    iRes = Comm_Cli_request_transmit2(vty, 0xFFFF, 0xFFFF, vty->prev_clistr, vty->prev_length, &stPayload);
    VOS_Free(vty->prev_clistr);
    vty->prev_clistr = NULL;
    vty->prev_length = 0;
    if ( 0 == iRes )
    {
        if (BROADCAST_VTY_IS_SAFE(vty))
        {
            /* 安全模式的命令行广播只是单次有效的 */
            BROADCAST_VTY_CLRALL(vty);
        }

        if (vty->action_SysCBfunc)
        {
            if ( VOS_OK == (iRes = vty->action_SysCBfunc(vty)) )
            {
                if (vty->action_UsrCBfunc)
                {
                    iRes = vty->action_UsrCBfunc(vty);
                    vty->action_UsrCBfunc = NULL;
                }
            }
            vty->action_SysCBfunc = NULL;
        }
    }
    
    return iRes;
}
/* E--added by liwei056@2009-3-19 for BroadCast-Cli-Commit */

/***********************************************************
* Comm_Cli_request_transmit
*
* description : 该函数用于发送cli命令至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
* parameter : 
*			PonId -	0~19 
*			OnuId -  1~64
*			pClibuf - 指向cli 命令的有效空间，该指针由上层释放。
*			cli_data_size	- 该cli 命令的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
*
************************************************************/
int  Comm_Cli_request_transmit(
                struct vty * vty,
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char  *pClibuf,
				const unsigned short cli_data_size,
				unsigned char *psessionIdField)
{
	int iRes = OAM_OK;
	short int tempOnuId = OnuId - 1;
	short int llid = 0;
	unsigned long lSendSerNo = 0;
	short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;

/* B--added by liwei056@2009-3-5 for BroadCast-Cli */
    if ( !BROADCAST_VTY_ISEMPTY(vty) )
    {
        int i, l, n, m;
        
        /* 向PON口下所有ONU广播命令行*/
        vty_out( vty, "Are you sure want to broadcast the command:[%s]\r\nto all the onus of PON:[", pClibuf);
        if ( BROADCAST_VTY_ISFULL(vty) )
        {
        	vty_out(vty, "All slots");
        }
        else
        {
            n = 1;
            m = BROADCAST_VTY_GETNUM(vty);
            l = BROADCAST_VTY_GETMAXNUM(vty);
            for (i=0; i<l; ++i)
            {
                if ( BROADCAST_VTY_GETID(vty, i) )
                {
                    if (n++ < m )
                    {
                    	vty_out(vty, "pon%d/%d, ", GetCardIdxByPonChip(i), GetPonPortByPonChip(i) );
                    }
                    else
                    {
                    	vty_out(vty, "pon%d/%d", GetCardIdxByPonChip(i), GetPonPortByPonChip(i) );
                    }
                }
            }
            
        }
        vty_out( vty, "]? [Y/N]");

        if ( NULL != (vty->prev_clistr = VOS_Malloc(cli_data_size + 1, MODULE_OAM)) )
        {
            VOS_MemCpy(vty->prev_clistr, pClibuf, cli_data_size);
            vty->prev_clistr[cli_data_size] = '\0';
            vty->prev_length = cli_data_size;
            vty->prev_node = vty->node;
            vty->node = CONFIRM_ACTION_NODE;
            vty->action_func = Comm_Cli_request_broadcastCommit;
            vty->no_action_func = Comm_Cli_request_broadcastRefuse;
        
            return OAM_OK;
        }
        else
        {
    		return OAM_ERR;	
        }
    }
/* E--added by liwei056@2009-3-5 for BroadCast-Cli */

	if(pClibuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;	
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;	
	
	llid = GetLlidByOnuIdx( PonId, tempOnuId);
	if ( OAM_INVALID_LLID == llid )
	{
		return OAM_ONU_DEREGISTER_ERR;
	}
	iRes = COMM_OAM_frame_send( 	PonId, 
									tempOnuId,
									GW_OPCODE_CLI_REQUEEST,
									broadcast_llid,
									pClibuf, 
									cli_data_size, 
									psessionIdField,
									GW_DEVTYPE_OLT, 
									lSendSerNo);	
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}
	/*
	if(OAM_INVALUE_QUEUE_ERR == iRes)
	{
		sys_console_printf(  "%% Warning! Oam queue is full\r\n");
	}*/
	return iRes;	

}

int  Comm_Cli_request_transmit_novty(
                const unsigned short PonId,
                const unsigned short OnuId,
                unsigned char  *pClibuf,
                const unsigned short cli_data_size,
                unsigned char *psessionIdField)
{
    int iRes = OAM_OK;
    short int tempOnuId = OnuId - 1;
    short int llid = 0;
    unsigned long lSendSerNo = 0;
    short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;


    if(pClibuf == NULL)
        return OAM_NULL_POINTER ;
    if (NULL == psessionIdField)
        return OAM_NULL_POINTER ;
    if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
        return OAM_ERR;

    llid = GetLlidByOnuIdx( PonId, tempOnuId);
    if ( OAM_INVALID_LLID == llid )
    {
        return OAM_ONU_DEREGISTER_ERR;
    }
    iRes = COMM_OAM_frame_send(     PonId,
                                    tempOnuId,
                                    GW_OPCODE_CLI_REQUEEST,
                                    broadcast_llid,
                                    pClibuf,
                                    cli_data_size,
                                    psessionIdField,
                                    GW_DEVTYPE_OLT,
                                    lSendSerNo);
    if( iRes != OAM_OK)
    {
        if (1 == gGwOamTaskErrDebug)
        {
            sys_console_printf("  -------OAM Send Err info -----\r\n");
            Comm_Oam_ErrParse(iRes);

        }
    }
    /*
    if(OAM_INVALUE_QUEUE_ERR == iRes)
    {
        sys_console_printf(  "%% Warning! Oam queue is full\r\n");
    }*/
    return iRes;

}


/*  not used,  commented by chenfj
static void hton32_v4( ulong_t hostn, ulong_t *netn )
{
	int i=0,len=0;
	char *psrc = (char*)&hostn;
	char *pdes = (char*)netn;
	if( pdes == NULL )
		return;
	len = sizeof(ulong_t);
	for( i=0;i<len;i++)
		*(pdes+(len-i-1))=*(psrc+i);
	
}
*/
/*=====================snmp接口定义=========================*/
/***********************************************************
* Comm_snmp_request_transmit
*
* description : 该函数用于发送snmp请求消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
* parameter :  
*			PonId -	0~19 
*			OnuId -  1~64
*			pSnmpbuf - 指向消息内容的有效空间，该指针由上层释放。
*			Snmp_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
*
*
************************************************************/
int Comm_snmp_request_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				void *pSnmpbuf,
				const unsigned short Snmp_data_size,
				unsigned char *psessionIdField)
{
/*
    char sz[8]="";
    char *p = NULL;
	ulong_t devid=0;
*/	
	int iRes = OAM_OK;
	unsigned long 	lSendSerNo = 0;
	short int tempOnuId = OnuId - 1;	
	short int llid = 0;	
	short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;
	if(pSnmpbuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;
/*
	p = VOS_StrChr( pSnmpbuf, '@' );
	if( p == NULL )
		return OAM_NULL_POINTER;
    else
    {
        if( (p-(char*)pSnmpbuf)+1<6 )
			return OAM_NULL_POINTER;
		VOS_MemCpy( sz, p+1, 5 );
		devid = VOS_AtoL( sz );
		hton32_v4( devid, (ulong_t*)(psessionIdField+4) );
    }
*/	
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;	
	
	llid = GetLlidByOnuIdx( PonId, tempOnuId);
	if ( OAM_INVALID_LLID == llid )
	{
		return OAM_ONU_DEREGISTER_ERR;
	}
	
	iRes = COMM_OAM_frame_send(	 PonId, 
									tempOnuId,
									GW_OPCODE_SNMP_REQUEST,
									broadcast_llid,
									pSnmpbuf, 
									Snmp_data_size, 
									psessionIdField, 
									GW_DEVTYPE_OLT, 
									lSendSerNo);
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}

	return iRes;
}



/*=====================告警和日志传输接口定义=========================*/

/******************************************************
* Comm_IGMPAuth_Reponse
*
* description : 该函数用于发送igmp 认证消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
* parameter :  
*			PonId -	0~19 
*			OnuId -  1~64
*			pIgmpAuth - 指向igmp认证消息内容的有效空间，该指针由上层释放。
*			igmp_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
*
********************************************************/
int Comm_IGMPAuth_Reponse(const unsigned short PonId, 
								const unsigned short OnuId,
								unsigned char *pIgmpAuth,
								const unsigned short igmp_data_size,
								unsigned char *psessionIdField)
{
	int iRes = OAM_OK;
	short int tempOnuId = OnuId - 1;	
	unsigned long lSendSerNo = 0;
	short int llid = 0;
	short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;
	if(pIgmpAuth == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;	
	llid = GetLlidByOnuIdx( PonId, tempOnuId);
	if ( OAM_INVALID_LLID == llid )
	{
		return OAM_ONU_DEREGISTER_ERR;
	}	
	iRes = COMM_OAM_frame_send( 	PonId, 
									tempOnuId,
									Gw_OPCODE_IGMPAUTH_REPONSE,
									broadcast_llid,
									pIgmpAuth, 
									igmp_data_size, 
									psessionIdField,
									GW_DEVTYPE_OLT, 
									lSendSerNo);	
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}
	/*if(OAM_INVALUE_QUEUE_ERR == iRes)
	{
		sys_console_printf(  "%% Warning! Oam queue is full\r\n");
	}*/
	return iRes;
}


/******************************************************
* Comm_IGMPAuth_Requst
*
* description : 该函数用于发送igmp 认证消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
* parameter :  
*			PonId -	0~19 
*			OnuId -  1~64
*			pIgmpAuth - 指向igmp认证消息内容的有效空间，该指针由上层释放。
*			igmp_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
*
********************************************************/
int Comm_IGMPAuth_Requst(const unsigned short PonId, 
								const unsigned short OnuId,
								unsigned char *pIgmpAuth,
								const unsigned short igmp_data_size,
								unsigned char *psessionIdField)
{
	int iRes = OAM_OK;
	short int tempOnuId = OnuId - 1;	
	unsigned long lSendSerNo = 0;
	short int llid = 0;
	short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;
	if(pIgmpAuth == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;	
	llid = GetLlidByOnuIdx( PonId, tempOnuId);
	if ( OAM_INVALID_LLID == llid )
	{
		return OAM_ONU_DEREGISTER_ERR;
	}	
	iRes = COMM_OAM_frame_send( 	PonId, 
									tempOnuId,
									GW_OPCODE_IGMPAUTH_REQUEST,
									broadcast_llid,
									pIgmpAuth, 
									igmp_data_size, 
									psessionIdField,
									GW_DEVTYPE_OLT, 
									lSendSerNo);	
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}
	/*if(OAM_INVALUE_QUEUE_ERR == iRes)
	{
		sys_console_printf(  "%% Warning! Oam queue is full\r\n");
	}*/
	return iRes;
}



/*=====================设备信息传输接口定义=========================*/
/***********************************************************
* Comm_EUQ_info_request_transmit
*
* description : 该函数用于发送EUQ_info_request消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
* parameter :  
*			PonId -	0~19 
*			OnuId -  1~64
*			pEuqinfobuf - 指向消息内容的有效空间，该指针由上层释放。
*			Euqinfo_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
*
*
************************************************************/
int Comm_EUQ_info_request_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char *pEuqinfobuf,
				const unsigned short Euqinfo_data_size,
				unsigned char *psessionIdField)
{
	int iRes = OAM_OK;
	short int tempOnuId = OnuId - 1;
	short int llid = 0;
	unsigned long lSendSerNo = 0;
	short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;
	if(pEuqinfobuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;		

	llid = GetLlidByOnuIdx( PonId, tempOnuId);
	if ( OAM_INVALID_LLID == llid )
	{
		return OAM_ONU_DEREGISTER_ERR;
	}

	iRes = COMM_OAM_frame_send( 	PonId, 
									tempOnuId,
									GW_OPCODE_EUQ_INFO_REQUESET,
									broadcast_llid,
									pEuqinfobuf, 
									Euqinfo_data_size, 
									psessionIdField,
									GW_DEVTYPE_OLT, 
									lSendSerNo);	
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}	
	/*if(OAM_INVALUE_QUEUE_ERR == iRes)
	{
		sys_console_printf(  "%% Warning! Oam queue is full\r\n");
	}*/
	return iRes;
}

/********************************************************
* Comm_Alarm_Response_transmit
* description : 告警回应.
*
* input :  
*			PonId -	0~19 
*			OnuId -  1~64
*		pAlarmbuf - Payload 
*		psessioinIdField	- SessionId 8 octets
*		Alarm_data_size   - payload length
*
**********************************************************/
int Comm_Alarm_Response_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char *pAlarmbuf,
				const unsigned short Alarm_data_size,
				unsigned char *psessionIdField)
{
	int iRes = OAM_OK;
	short int tempOnuId = OnuId - 1;	
	short int llid = 0;
	unsigned long lSendSerNo = 0;
	short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;
	if(pAlarmbuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;		

	llid = GetLlidByOnuIdx( PonId, tempOnuId);

	if ( OAM_INVALID_LLID == llid )
	{
		return OAM_ONU_DEREGISTER_ERR;
	}
	
	iRes = COMM_OAM_frame_send( 	PonId, 
									tempOnuId,
									GW_OPCODE_ALARM_OR_LOG_RESPONSE,
									broadcast_llid,
									pAlarmbuf, 
									Alarm_data_size, 
									psessionIdField,
									GW_DEVTYPE_OLT, 
									lSendSerNo);	
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}
	/*if(OAM_INVALUE_QUEUE_ERR == iRes)
	{
		sys_console_printf(  "%% Warning! Oam queue is full\r\n");
	}*/
	return iRes;
}

/**********************************************************
* Comm_EUQ_SCB_systemTime_transmit
* description : 通过SCB通道向onu发送system time
*
* input : PonId 0~19
*		  pSysTimeBuf - Payload 
*		  psessioinIdField	- SessionId 8 octets
*		  sysTimeLen   - payload length
*
* Output: return iRes;
*************************************************************/
int Comm_EUQ_SCB_systemTime_transmit (
					const unsigned short PonId, 
					unsigned char		*pSysTimeBuf,
					const unsigned short sysTimeLen,
					unsigned char *psessionIdField)
{
	int iRes = OAM_OK;
	short int tempOnuId = 0;	
	unsigned long lSendSerNo = 0;
	short int broadcast_llid = OAM_PON_EVERY_LLID;	
	iRes = GetPonPortOperStatus(  PonId );
	if ( iRes != OAM_PONPORT_UP)
	/*if(( iRes == PONPORT_DOWN )|| ( iRes == PONPORT_UNKNOWN )||(iRes == PONPORT_INIT)||( iRes == RERROR ))*/
	{
		return OAM_PONID_NOTWORK;
	}
	if(pSysTimeBuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;		
	iRes = COMM_OAM_frame_send( 	PonId, 
									tempOnuId,
									GW_OPCODE_EUQ_INFO_REQUESET,
									broadcast_llid,
									pSysTimeBuf, 
									sysTimeLen, 
									psessionIdField,
									GW_DEVTYPE_OLT, 
									lSendSerNo);	
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}
	return iRes;
}



/*************************************************
* Comm_EUQ_systime_require_transmit
* description : 向单个onu发送system time
*
* input : PonId 0~19
*            OnuId -  1~64
*		  pSysTimeBuf - Payload 
*		  psessioinIdField	- SessionId 8 octets
*		  sysTimeLen   - payload length
* Output: return iRes;
***************************************************/
int Comm_EUQ_systime_Require_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char *pSysTimeBuf,
				const unsigned short sysTimeBufLen,
				unsigned char *psessionIdField
				)
{
	int iRes = OAM_OK;
	short int tempOnuId = OnuId - 1;	
	short int llid = 0;
	unsigned long lSendSerNo = 0;
	short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;		
	if(pSysTimeBuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;		

	llid = GetLlidByOnuIdx( PonId, tempOnuId);
	if ( OAM_INVALID_LLID == llid )
	{
		return OAM_ONU_DEREGISTER_ERR;
	}
	iRes = COMM_OAM_frame_send( 	PonId, 
									tempOnuId,
									GW_OPCODE_EUQ_INFO_REQUESET,
									broadcast_llid,
									pSysTimeBuf, 
									sysTimeBufLen, 
									psessionIdField,
									GW_DEVTYPE_OLT, 
									lSendSerNo);	
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}
	return iRes;	
}

/***********************************************************
* Comm_vconsole_info_request_transmit
*
* description : 该函数用于发送vconsole_info_request消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
* parameter :  
*			PonId -	0~19 
*			OnuId -  1~64
*			pEuqinfobuf - 指向消息内容的有效空间，该指针由上层释放。
*			Euqinfo_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
*
*
************************************************************/
int Comm_vconsole_info_request_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char *pEuqinfobuf,
				const unsigned short Euqinfo_data_size,
				unsigned char *psessionIdField)
{
	int iRes = OAM_OK;
	short int tempOnuId = OnuId - 1;
	short int llid = 0;
	unsigned long lSendSerNo = 0;
	short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;
	if(pEuqinfobuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;		

	llid = GetLlidByOnuIdx( PonId, tempOnuId);
	if ( OAM_INVALID_LLID == llid )
	{
		return OAM_ONU_DEREGISTER_ERR;
	}

	iRes = COMM_OAM_frame_send( 	PonId, 
									tempOnuId,
									GW_OPCODE_VCONSOLE,
									broadcast_llid,
									pEuqinfobuf, 
									Euqinfo_data_size, 
									psessionIdField,
									GW_DEVTYPE_OLT, 
									lSendSerNo);	
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}	
	/*if(OAM_INVALUE_QUEUE_ERR == iRes)
	{
		sys_console_printf(  "%% Warning! Oam queue is full\r\n");
	}*/
	return iRes;
}

/***********************************************************
* FileDataTransmit
* 描述: 该函数用于发送回应
* description : 该函数用于发送消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
*			PonId -值: 0 - 19 ，对应ponid 调用该接口函数前，可通过调用函数
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) 获取得到ponid 
*			OnuId - 值: 1  - 64, 对应每个ponid下的onuid ，不必再进行转换?
*			p**buf - 指向消息内容的有效空间，该指针由上层释放。
*			**_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
************************************************************/
short int FileDataTransmit(
									const unsigned short PonId, 
									const unsigned short OnuId,
									unsigned char *fileBuf,
									unsigned int fileBufLen,
									unsigned char *psessionIdField
									)
{
	int iRes = OAM_OK;
	short int tempOnuId = OnuId - 1;
	short int llid = 0;
	unsigned long lSendSerNo = 0;
	short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;
		
	if(fileBuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;	
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;	
	llid = GetLlidByOnuIdx( PonId, tempOnuId);
	if ( OAM_INVALID_LLID == llid )
	{
		return OAM_ONU_DEREGISTER_ERR;
	}
	iRes = COMM_OAM_frame_send( 	PonId, 
									tempOnuId,
									GW_OPCODE_FILE_DATA_TRAN,  /*modify by luj 2006/10/25*/
									broadcast_llid,
									fileBuf, 
									fileBufLen, 
									psessionIdField,
									GW_DEVTYPE_OLT, 
									lSendSerNo);	
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}
	/*if(OAM_INVALUE_QUEUE_ERR == iRes)
	{
		sys_console_printf(  "%% Warning! Oam queue is full\r\n");
	}*/
	return iRes;
	
}
/***********************************************************
* FileRequestTransmit
* description : 该函数用于发送消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
*			PonId -值: 0 - 19 ，对应ponid 调用该接口函数前，可通过调用函数
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) 获取得到ponid 
*			OnuId - 值: 1  - 64, 对应每个ponid下的onuid ，不必再进行转换?
*			p**buf - 指向消息内容的有效空间，该指针由上层释放。
*			**_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
************************************************************/
short int FileRequestTransmit(
									const unsigned short PonId, 
									const unsigned short OnuId,
									unsigned char *fileBuf,
									unsigned int fileBufLen,
									unsigned char *psessionIdField
									)
{
	int iRes = OAM_OK;
	short int tempOnuId = OnuId - 1;
	short int llid = 0;
	unsigned long lSendSerNo = 0;
	short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;
		
	if(fileBuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;	
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;	
	llid = GetLlidByOnuIdx( PonId, tempOnuId);
	if ( OAM_INVALID_LLID == llid )
	{
		return OAM_ONU_DEREGISTER_ERR;
	}	
	iRes = COMM_OAM_frame_send( 	PonId, 
									tempOnuId,
									GW_OPCODE_FILE_READ_AND_WRITE_REQUEST,
									broadcast_llid,
									fileBuf, 
									fileBufLen, 
									psessionIdField,
									GW_DEVTYPE_OLT, 
									lSendSerNo);
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}
	/*if(OAM_INVALUE_QUEUE_ERR == iRes)
	{
		sys_console_printf(  "%% Warning! Oam queue is full\r\n");
	}*/
	return iRes;	
}


/***********************************************************
* FileAckTransmit
* 描述: 该函数用于发送回应
* description : 该函数用于发送消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
*			PonId -值: 0 - 19 ，对应ponid 调用该接口函数前，可通过调用函数
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) 获取得到ponid 
*			OnuId - 值: 1  - 64, 对应每个ponid下的onuid ，不必再进行转换?
*			p**buf - 指向消息内容的有效空间，该指针由上层释放。
*			**_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
************************************************************/
short int FileAckTransmit(
									const unsigned short PonId, 
									const unsigned short OnuId,
									unsigned char *fileBuf,
									unsigned int fileBufLen,
									unsigned char *psessionIdField
									)
{
	int iRes = OAM_OK;
	short int tempOnuId = OnuId - 1;
	short int llid = 0;
	unsigned long lSendSerNo = 0;
	short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;
		
	if(fileBuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;	
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;	
	llid = GetLlidByOnuIdx( PonId, tempOnuId);
	if ( OAM_INVALID_LLID == llid ) 
	{
		return OAM_ONU_DEREGISTER_ERR;
	}	
	iRes = COMM_OAM_frame_send( 	PonId, 
									tempOnuId,
									GW_OPCODE_FILE_TRANSFER_ACK,
									broadcast_llid,
									fileBuf, 
									fileBufLen, 
									psessionIdField,
									GW_DEVTYPE_OLT, 
									lSendSerNo);
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}
	/*if(OAM_INVALUE_QUEUE_ERR == iRes)
	{
		sys_console_printf(  "%% Warning! Oam queue is full\r\n");
	}*/
	return iRes;	
	
}
#if 0
/********************************************************
* Comm_file_data_transmit
* description : 该函数用于发送文件数据消息至onu。调用该函数所传递的指针
*			所占用的空间，由调用者释放。该函数返回0 时为正确，其他值则为错误。
*			PonId -值: 0 - 19 ，对应ponid 调用该接口函数前，可通过调用函数
*				GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) 
获取得到ponid 
*			OnuId - 值: 1  - 64, 对应每个ponid下的onuid ，不必再进行转换。
*			pfileBuf - 指向消息内容的有效空间，该指针由上层释放。
*			fileBufSize	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
**********************************************************/
int Comm_file_data_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char  *pfileBuf,
				const unsigned short fileBufSize,
				unsigned char *psessionIdField)
{
	int iRes = OAM_OK;
	short int tempOnuId = OnuId - 1;
	short int llid = 0;
	unsigned long lSendSerNo = 0;
	short int broadcast_llid = OAM_SEND_SPECIFIED_LLID;
		
	if(pfileBuf == NULL)
		return OAM_NULL_POINTER ;
	if (NULL == psessionIdField)
		return OAM_NULL_POINTER ;	
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;	
	
	llid = GetLlidByOnuIdx( PonId, tempOnuId);
	if ( OAM_INVALID_LLID == llid ) 
	{
		return OAM_ONU_DEREGISTER_ERR;
	}
	VOS_TaskDelay(1);
	iRes = COMM_OAM_frame_send( 	PonId, 
									tempOnuId,
									GW_OPCODE_CLI_REQUEEST,
									broadcast_llid,
									pfileBuf, 
									fileBufSize, 
									psessionIdField,
									GW_DEVTYPE_OLT, 
									lSendSerNo);
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}	
	return iRes;
}


/***********************************************************
*
*
*
*
*
************************************************************/
int CommFileRdRspRvc(
				const unsigned short OltId, 
				const unsigned short OnuId,
				void *pFilepbuf,
				const unsigned short File_data_size)
{
	int iRes = OAM_OK;
	if(pFilepbuf == NULL)
		return OAM_NULL_POINTER ;
	
	if( iRes != OAM_OK)
	{
		if (1 == gGwOamTaskErrDebug)
		{
			sys_console_printf("  -------OAM Send Err info -----\r\n");
			Comm_Oam_ErrParse(iRes);

		}
	}
	return iRes;
	
}

#endif













/* following API is not used, deleted by chenfj 2008-7-3 */
#if 0

/********************************************************
* CommOnuMsgEmptyQueueGet
*
*
*
*
*
*
*
*
*********************************************************/
short int CommOnuMsgEmptyQueueGet(
				const unsigned short      OltId, 
				const unsigned short      OnuId,
				unsigned short *pQueueId)	
{
	unsigned char TempId = 0;
	unsigned char QueueId = 0xff;

	for(TempId = 0; TempId<GW_MAX_QUEUEID; TempId++)
	{
		if ( TRUE == gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[TempId].Iused)
			{
			gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[TempId].RecvMsgTime ++;
			if( gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[TempId].RecvMsgTime >= (GW_MAX_QUEUEID-1))
				{
				CommOnuMsgQueueFree(OltId, OnuId, TempId );			
				if( QueueId == 0xff )
					{
					*pQueueId = TempId;
					QueueId = TempId;
					}
				}
			}
		else {
			if( QueueId == 0xff )
				{
				*pQueueId = TempId;
				QueueId = TempId;
				}
			}
	}

	if( QueueId  != 0xff )
		return( OAM_OK );
	
	if( gGwOamTaskErrDebug == 1 )
		sys_console_printf("\r\nNo free memory for onu %d/%d to buffer oam msg\r\n", OltId, OnuId); 
	return (OAM_ERR);
}

/********************************************************
* CommOnuMsgSendsernoCheck
*
*
*
*
*
*
*
*
*********************************************************/
short int CommOnuMsgSendsernoCheck(
				const unsigned short      OltId, 
				const unsigned short      OnuId,
				GW_OAM_PAD_layer_msg_t *pOamInfo,
				unsigned short *pQueueId)	
{
	unsigned short TempId = 0;
	unsigned long  SerNo = pOamInfo->sendserno;
	for(TempId = 0; TempId<GW_MAX_QUEUEID; TempId++)
		{
		if((TRUE == gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[TempId].Iused) && \
			(gonuMsg[OltId].onu_queue_t[OnuId].onu_msg[TempId].sendserno == SerNo))
			{
			*pQueueId = TempId;
			return (OAM_OK);
			}
		}
	return (OAM_ERR);
}


/*******************************************
* CommOnuMsgWdogInfoInit
*
*
*
*
*
*
*
*******************************************/
unsigned short CommOnuMsgWdogInfoInit(
				unsigned short      OltId, 
				unsigned short      OnuId,
				unsigned short QueueId)
{
#if 0	
	unsigned short count = 0;
	int  wdInfoNos = OltId * OnuId * QueueId ;
	/*信号量保护*/

	if (OAM_ERR == semTake(gWdInfoSemId, WAIT_FOREVER))
		return (OAM_ERR);

	for (count = 0; count<wdInfoNos; count++)
		{
		 	if (!gMsgWdInfo[count].flagUsed)
		 		{
		 			gMsgWdInfo[OltId][OnuId][wdInfoNos].OltId = OltId;
					gMsgWdInfo[wdInfoNos].OnuId = OnuId;
					gMsgWdInfo[wdInfoNos].QueueId = QueueId;
					semGive(gWdInfoSemId);	
					return (count);
		 		}
			count ++;
		}
	semGive(gWdInfoSemId);	
#endif
	return (OAM_OK);
}



/*******************************************
* CommOnuMsgWdogTimeOutPro
*
*
*
*******************************************/
/*short int CommOnuMsgWdogTimeOutPro(int  wdInfoNos)
{
	short int      OltId  = gMsgWdInfo[wdInfoNos].OltId;
	short int      OnuId = gMsgWdInfo[wdInfoNos].OnuId;
	unsigned short QueueId = gMsgWdInfo[wdInfoNos].QueueId;
	if (OAM_ERR == semTake(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuSemId, NO_WAIT))
		{
			return (OAM_ERR);
		}

	goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].Iused = TRUE;
	VOS_Free(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad);
	semGive(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuSemId);

	return (OAM_OK);
}*/

int CommOnuMsgQueueFree(unsigned short OltId, unsigned short OnuId,unsigned short QueueId)
{

	if(gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad != NULL )
		VOS_Free((VOID *)gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad);
			
	gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad = NULL;
	gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].Iused = FALSE;
	gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].RecvMsgTime = 0;
	memset(&gonuMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId], 0, sizeof(GW_OAM_PAD_layer_msg_t));
	return OAM_OK;

	}

int CommOnuOamMsgFreeAll(unsigned short OltId, unsigned short OnuId)
{
	unsigned short int QueueId;

	for(QueueId = 0; QueueId < GW_MAX_QUEUEID; QueueId ++)
		{
		CommOnuMsgQueueFree(OltId, OnuId, QueueId );
		CommOltMsgQueueFree(OltId, OnuId, QueueId );
		}
	return( OAM_OK );
}

/*****************************************
*
*
*
*
*
*
*
******************************************/
int CommOnuMsgQueueInfoGet(unsigned short OltId, 
								unsigned short OnuId,
								unsigned short QueueId, 
								unsigned char **ppRevBuf, 
								unsigned short *pPayloadlen)
{
	*ppRevBuf = gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad;
	*pPayloadlen = (unsigned int)(gonuMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength);
	return OAM_OK;
}


/*****************************************
*
*
*
*
*
*
*
******************************************/
int CommOnuMsgSave(unsigned short OltId, 
								unsigned short OnuId,
								unsigned short QueueId, 
								GW_OAM_PAD_layer_msg_t *pOamInfo, 
								unsigned char  *pOamFrame)
{
	unsigned int wholeLen = 0;
	unsigned int payloadlen = 0;
	unsigned char *pRevBuf = NULL;
	
	if (!gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].Iused)
		{
			memcpy(&gonuMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId],pOamInfo,sizeof(GW_OAM_PAD_layer_msg_t));
			wholeLen =pOamInfo->wholepktlen;
#ifdef OAM_DEBUG	
			pRevBuf = (unsigned char*)g_malloc(wholeLen);
#else
			pRevBuf = (unsigned char*)VOS_Malloc((ULONG)wholeLen, (ULONG)MODULE_OAM);
#endif
			payloadlen = pOamInfo->payloadlength;

			memcpy(pRevBuf,pOamFrame,payloadlen);
			gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad = pRevBuf;
			gonuMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength = payloadlen;
			gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].Iused = TRUE;	
		}
	else
		{
			pRevBuf = gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].payloadpad;	
			payloadlen = gonuMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength;

			/*以下判断是否连续帧,中间不允许有断帧出现*/
			if((payloadlen) != pOamInfo->payloadoffset)
				{
					return OAM_SEQUENCE_ERR;
				}
			memcpy(((payloadlen)+pRevBuf),pOamFrame,pOamInfo->payloadlength);
			gonuMsg[OltId].onu_queue_t[OnuId].onu_msg[QueueId].payloadlength += pOamInfo->payloadlength;
			
		}
	return OAM_OK;
}

/*****************************************
*
*
*
*
*
*
*
******************************************/
int CommOnuMsgSemTake(unsigned short OltId, unsigned short OnuId, unsigned short QueueId)
{
#if 0
	SEM_ID		SemId ;
	if (OAM_ERR == semTake(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuSemId, WAIT_FOREVER) )
		{
			return (OAM_ERR);
		}
#endif
	if (OAM_OK != VOS_SemTake(gonuMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuSemId, WAIT_FOREVER))
		return (OAM_ERR);
	return OAM_OK;
}
/*****************************************
*
*
*
*
*
*
*
******************************************/
int CommOnuMsgSemGive(unsigned short OltId, unsigned short OnuId, unsigned short QueueId)
{
#if 0
	SEM_ID		SemId ;
	if (OAM_ERR == semGive(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuSemId))
		{
			return (OAM_ERR);
		}
#endif
	if (OAM_OK != VOS_SemGive(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuSemId))
		return (OAM_ERR);
	return (OAM_OK);

	
}



/******************************************************************
* CommOnuMsgReveive
* 描述:该函数判断是否该帧为存在在收的帧.
* 		函数操作包含:
*			1. 锁定信号量
*			2. 停止定时器
*			3. 判断是否存在该sentserno
*			4. 正常其他操作
*			5. 开始定时器
*			6. 释放信号量
* 输入:
* 输出:
* 返回:
******************************************************************/
short int CommOnuMsgReveive(
				const unsigned short      OltId, 
				const unsigned short      OnuId,
				/*unsigned char OpCode,*/
				GW_OAM_PAD_layer_msg_t *pOamInfo,
				unsigned char *pOamFrame)
{
	unsigned short QueueId = 0;
	unsigned char *pRevBuf = NULL;
	unsigned short payloadlen = 0;
	unsigned result = OAM_OK;

    if (0 == pOamInfo->payloadoffset)
    	{/*第一个数据包*/
    		/*检查是否已经存在相同的sendserno*/
		result = CommOnuMsgSendsernoCheck(OltId,OnuId,pOamInfo, &QueueId);
		if(result == OAM_OK)
			return (OAM_ERR);
    				
    		/*1. 检查是否该数据包为一个独立的数据包*/
		if(pOamInfo->payloadlength == pOamInfo->wholepktlen)
			{
			payloadlen = pOamInfo->payloadlength;
			pRevBuf = (unsigned char *)VOS_Malloc((ULONG)payloadlen, (ULONG)MODULE_OAM);

			/*memcpy(pRevBuf,pOamFrame,payloadlen);
			strcpy(pRevBuf,pOamFrame);*/
			VOS_MemCpy(pRevBuf,pOamFrame,payloadlen);
			if(OAM_ERR == CommOnuMsgCallback(OltId, OnuId, pOamInfo->gw_opcode, payloadlen, pRevBuf))
				{
				#ifdef OAM_DEBUG_PRINTF	
					sys_console_printf("OAM_ERR!CommOnuMsgCallback().");
				#endif
					VOS_Free((VOID *) pRevBuf);	
					pRevBuf = NULL;
				return (OAM_ERR);
				}
			return (OAM_OK);
			}
		else
			{
			/*2. 该数据包为非独立数据包*/
			/*是否该数据包的总长度> GW_MAX_MSG_LENGTH ?*/
			if(pOamInfo->wholepktlen  >GW_MAX_MSG_LENGTH)
				{
				return (OAM_ERR);
				}

			/*获取空闲的queueid,用于放置新的数据包*/
			result = CommOnuMsgEmptyQueueGet(OltId, OnuId, &QueueId);
			if(result != OAM_OK)
				{

				return result;
				}
			
			result = CommOnuMsgSave(OltId, OnuId, QueueId, pOamInfo, pOamFrame);
			if(result != OAM_OK)
				{
				return result;
				}
			}
		
		/*启动定时器，时间间隔为2s，如果发生超时事件，则丢弃该数据包*/
		/*TempCount = CommOnuMsgWdogInfoInit(OltId,  OnuId, QueueId);
		wdStart (goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuWdId,sysClkRateGet()*2, 
							(FUNCPTR)CommOnuMsgWdogTimeOutPro, (int)(OnuId*OnuId*QueueId));*/
		return OAM_OK;
    	}

	/*================================*/
	else
		{
 			result = CommOnuMsgSendsernoCheck(OltId, OnuId, pOamInfo, &QueueId);	
			if(result != OAM_OK)
				{
				return (result);
				}
			
			/*停止当前msg queue的定时器*/
			/*wdCannel(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuWdId);*/
			
			if (OAM_ERR == CommOnuMsgSemTake(OltId, OnuId, QueueId))
				{
					return (OAM_ERR);
				}			

			if((pOamInfo->payloadoffset+ pOamInfo->payloadlength) != pOamInfo->wholepktlen)
				{
					/*以下判断是否连续帧,中间不允许有断帧出现*/
					if(OAM_OK != CommOnuMsgSave(OltId, OnuId, QueueId, pOamInfo, pOamFrame))
						{
						/*删除所有的接收到的数据包,并释放空间*/
						/*清空goltMsg数据结构*/
						CommOnuMsgQueueFree(OltId, OnuId,QueueId);
						CommOnuMsgSemGive(OltId, OnuId, QueueId);
						return (OAM_ERR);
						}

					/*启动定时器，时间间隔为2s，如果发生超时事件，则丢弃该数据包*/
					/*CommOnuMsgWdogInfoInit(OltId,  OnuId, QueueId);
					wdStart (goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuWdId,sysClkRateGet()*2, 
										(FUNCPTR)CommOnuMsgWdogTimeOutPro, (int)(OnuId*OnuId*QueueId));*/
										
					CommOnuMsgSemGive(OltId, OnuId, QueueId);
					return (OAM_OK);
				}
			else
				/*最后一个数据包*/
				{		
					/*wdCannel(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuWdId);*/
					unsigned char *pTempBuf = NULL;
					payloadlen = 0;
					/*以下判断是否连续帧,中间不允许有断帧出现*/
					if(OAM_OK != CommOnuMsgSave(OltId, OnuId, QueueId, pOamInfo, pOamFrame))
						{
						/*删除所有的接收到的数据包,并释放空间*/
						/*清空goltMsg数据结构*/
						CommOnuMsgQueueFree(OltId, OnuId,QueueId);
						CommOnuMsgSemGive(OltId, OnuId, QueueId);
						return (OAM_ERR);
						}

					CommOnuMsgQueueInfoGet(OltId, OnuId, QueueId, &pRevBuf, &payloadlen);
					if (payloadlen != pOamInfo->wholepktlen)
						return OAM_ERR;
					pTempBuf = (unsigned char*)VOS_Malloc((ULONG)payloadlen, (ULONG)MODULE_OAM);
		
					memcpy(pTempBuf , pRevBuf, payloadlen);
					pRevBuf = NULL;
					CommOnuMsgQueueFree(OltId, OnuId,QueueId);
					CommOnuMsgSemGive(OltId, OnuId, QueueId);
					
					/*CommOnuMsgWdogInfoDel(OltId*OnuId*QueueId);*/
					/*wdDelete(goltMsg[OltId].onu_queue_t[OnuId].onumsg_info[QueueId].onuWdId);*/
					
					result = CommOnuMsgCallback(OltId, OnuId, pOamInfo->gw_opcode, (payloadlen),pTempBuf);
					if(result != OAM_OK)
						{
							VOS_Free((VOID *)pTempBuf);						
							pTempBuf = NULL;
							return result;
						}
					return (OAM_OK);
				}	
		}/*end of else*/
}



/**********************************************
*CommMsgTransmit
* 描述: 该函数调用回调函数组，回调函数组被cli，
*		snmp等注册
* 输入参数:	oltid,
*				onuid,
*				GwOpCode - opcode类型
*				*pFrame - 指向数据的指针,无论正确错误,均由调用者释放。
* 输出参数: 无
* 返回值: 无
*************************************************/
int CommOnuMsgCallback(  
                                  const short int        OltId,
                                  const unsigned short     OnuId,
                                  const unsigned char GwOpCode,
                                  const unsigned short   Length,
                                  unsigned char   *pFrame)
{
	int iRes = OAM_OK;

	/*handle_oamfuncs[0x11](  OltId,
                                  			OnuId,
                                  			Length,
                                  			pFrame);*/
	
#ifdef OAM_DEBUG_PRINTF
	{
		int lPortNo = 1;
		int iLoop = 0;
		sys_console_printf(" \r\n----------------------OAM Rev bpdu--------------------- \r\n");
		sys_console_printf(" OAM PAD length=%d \r\n",Length);
		
		sys_console_printf(" Oam Content:\r\n");
		
for(iLoop=0;iLoop<Length;iLoop++)
			
{
				sys_console_printf("%02x ",pFrame[iLoop]);
			
}
		sys_console_printf("\r\n");
	}
	

	sys_console_printf(" ----------------------------------------------------- \r\n");
	/*sys_console_printf(" OAM_OK!\r\n");*/
	VOS_Free((VOID *)pFrame);

#else

	if(handle_oamfuncs[GwOpCode] == NULL)
		{
		/*free(pFrame);*/
		return (OAM_ERR);
		}

#endif

/*	iRes = handle_oamfuncs[GwOpCode](OltId, OnuId, GwOpCode, Length,pFrame);*/
	return iRes;
}

#endif

#if 0

int ScbsystemTime(unsigned short PonId)
{
	int iRes = 0;
	unsigned char *pSysTimeBuf = NULL;
	unsigned short sysTimeLen = 64;
	unsigned char sessionField[8] = {0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23};
	pSysTimeBuf = (unsigned char*)VOS_Malloc((ULONG)sysTimeLen, (ULONG) MODULE_OAM);
	if(pSysTimeBuf == NULL)
		return (-1);
	memset(pSysTimeBuf, 0x55, sysTimeLen);
	iRes = Comm_EUQ_SCB_systemTime_transmit (  
					PonId, 
					pSysTimeBuf,
					sysTimeLen,
					sessionField);
	return iRes;
}
int systemTime(unsigned short PonId, unsigned short onuId)
{
	int iRes = 0;
	unsigned char *pSysTimeBuf = NULL;
	unsigned short sysTimeLen = 64;
	unsigned short sessionField[8] = {0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23};
	pSysTimeBuf = (unsigned char*)VOS_Malloc((ULONG)sysTimeLen, (ULONG) MODULE_OAM);
	if(pSysTimeBuf == NULL)
		return (-1);
	memset(pSysTimeBuf, 0x55, sysTimeLen);
	iRes = Comm_EUQ_systime_Require_transmit ( 
					PonId, 
					onuId,
					pSysTimeBuf,
					sysTimeLen,
					sessionField
					);
	return iRes;
}
#endif

int OamDebugErr(short int status)
{
	gGwOamTaskErrDebug = status;
	return 0;
}

int OamDebugTrack(short int status)
{
	gGwOamTrackDebug = status;
	return 0;
}
int statOamCounter(void)
{
	short int Temp = 0;
	/*sys_console_printf("  Oam Rev %lu\r\n",gGwOamRevTempCount);*/
	sys_console_printf("  OamTotalRev %lu\r\n",gGwOamTotalRevCount);
	for(Temp = 0; Temp < GW_OPCODE_LAST/*0x14*/;Temp++)
		sys_console_printf("  OamRev[%d] %lu\r\n",Temp,gGwOamRevCount[Temp]);
	sys_console_printf("  OamTotalSend %lu\r\n",gGwOamTotalSendCount);
	for(Temp = 0; Temp <GW_OPCODE_LAST/*0x14*/;Temp++)
		sys_console_printf("  OamSend[%d] %lu\r\n",Temp,gGwOamSendCount[Temp]);	

	return 0;
}

int statOamCounterVty(struct vty *vty)
{
#if 0
	short int Temp = 0;
	vty_out( vty, "\r\n  --------------------Rev statistics------------------\r\n");	
	/*vty_out( vty, "  Oam Rev %lu\r\n",gGwOamRevTempCount);*/
	vty_out( vty, "  OamTotalRev %lu\r\n",gGwOamTotalRevCount);	
	for(Temp = 0; Temp < GW_OPCODE_LAST/*0x14*/;Temp++)
	{
		switch(Temp)
		{
			case GW_CALLBACK_CLI_REQUEST_1:
				vty_out( vty, "  OAM_CALLBACK_CLI_REQUEST  %lu\r\n",gGwOamRevCount[Temp]);
				break;			
			case GW_CALLBACK_CLI_1:
				vty_out( vty, "  OAM_CALLBACK_CLI  %lu\r\n",gGwOamRevCount[Temp]);			
				break;

			case GW_CALLBACK_RESERVED:	
				break;

			case GW_CALLBACK_EQU_REQUEST:
				vty_out( vty, "  OAM_CALLBACK_EQU_REQUEST  %lu\r\n",gGwOamRevCount[Temp]);			
				break;
			case GW_CALLBACK_EUQINFO:
				vty_out( vty, "  OAM_CALLBACK_EUQINFO  %lu\r\n",gGwOamRevCount[Temp]);			
				break;
			case GW_CALLBACK_ALARMORLOG	:
				vty_out( vty, "  OAM_CALLBACK_ALARMORLOG  %lu\r\n",gGwOamRevCount[Temp]);			
				break;
			case GW_CALLBACK_ALARMORLOG_REPONSE:
				vty_out( vty, "  OAM_CALLBACK_ALARMORLOG_RESPONSE  %lu\r\n",gGwOamRevCount[Temp]);			
				break;
			case GW_CALLBACK_FILE_RD_WRT_REQUEST:
				vty_out( vty, "  OAM_CALLBACK_FILE_RD_WRT_REQUEST  %lu\r\n",gGwOamRevCount[Temp]);			
				break;
			case GW_CALLBACK_FILE_RESERVED:
				break;
			case GW_CALLBACK_FILE_DATA_TRA:
				vty_out( vty, "  OAM_CALLBACK_FILE_DATA_TRA  %lu\r\n",gGwOamRevCount[Temp]);			
				break;
			case GW_CALLBACK_FILE_TRA_ACK:
				vty_out( vty, "  OAM_CALLBACK_FILE_TRA_ACK  %lu\r\n",gGwOamRevCount[Temp]);			
				break;
			case GW_CALLBACK_SNMP_REQUEST_1:
				vty_out( vty, "  OAM_CALLBACK_SNMP_REQUEST  %lu\r\n",gGwOamRevCount[Temp]);			
				break;
			case GW_CALLBACK_SNMP_REPONSE_1:
				vty_out( vty, "  OAM_CALLBACK_SNMP_RESPONSE  %lu\r\n",gGwOamRevCount[Temp]);			
				break;
			case GW_CALLBACK_SNMP_TRAP_1:
				vty_out( vty, "  OAM_CALLBACK_SNMP_TRAP  %lu\r\n",gGwOamRevCount[Temp]);			
				break;
			case GW_CALLBACK_IGMPAUTH_REQUEST:
				vty_out( vty, "  OAM_CALLBACK_IGMPAUTH_REQUEST  %lu\r\n",gGwOamRevCount[Temp]);			
				break;
			case GW_CALLBACK_IGMPAUTH_REPONST:
				vty_out( vty, "  OAM_CALLBACK_IGMPAUTH_RESPONSE  %lu\r\n",gGwOamRevCount[Temp]);			
				break;
			case GW_CALLBACK_VCONSOLE:
				vty_out( vty, "  OAM_CALLBACK_VCONSOLE  %lu\r\n",gGwOamRevCount[Temp]);	
				break;
		    default:
				break;
		}
	}

	vty_out( vty, "\r\n  --------------------Send statistics------------------\r\n");
	vty_out( vty, "  OamTotalSend %lu\r\n",gGwOamTotalSendCount);
	for(Temp = 0; Temp < GW_OPCODE_LAST/*0x14*/;Temp++)
	{
		switch(Temp)
		{
			case GW_CALLBACK_CLI_REQUEST_1:
				vty_out( vty, "  OAM_CALLBACK_CLI_REQUEST  %lu\r\n",gGwOamSendCount[Temp]);
				break;			
			case GW_CALLBACK_CLI_1:
				vty_out( vty, "  OAM_CALLBACK_CLI  %lu\r\n",gGwOamSendCount[Temp]);			
				break;

			case GW_CALLBACK_RESERVED:	
				break;

			case GW_CALLBACK_EQU_REQUEST:
				vty_out( vty, "  OAM_CALLBACK_EQU_REQUEST  %lu\r\n",gGwOamSendCount[Temp]);			
				break;
			case GW_CALLBACK_EUQINFO:
				vty_out( vty, "  OAM_CALLBACK_EUQINFO  %lu\r\n",gGwOamSendCount[Temp]);			
				break;
			case GW_CALLBACK_ALARMORLOG	:
				vty_out( vty, "  OAM_CALLBACK_ALARMORLOG  %lu\r\n",gGwOamSendCount[Temp]);			
				break;
			case GW_CALLBACK_ALARMORLOG_REPONSE:
				vty_out( vty, "  OAM_CALLBACK_ALARMORLOG_RESPONSE  %lu\r\n",gGwOamSendCount[Temp]);			
				break;
			case GW_CALLBACK_FILE_RD_WRT_REQUEST:
				vty_out( vty, "  OAM_CALLBACK_FILE_RD_WRT_REQUEST  %lu\r\n",gGwOamSendCount[Temp]);			
				break;
			case GW_CALLBACK_FILE_RESERVED:
				break;
			case GW_CALLBACK_FILE_DATA_TRA:
				vty_out( vty, "  OAM_CALLBACK_FILE_DATA_TRA  %lu\r\n",gGwOamSendCount[Temp]);			
				break;
			case GW_CALLBACK_FILE_TRA_ACK:
				vty_out( vty, "  OAM_CALLBACK_FILE_TRA_ACK  %lu\r\n",gGwOamSendCount[Temp]);			
				break;
			case GW_CALLBACK_SNMP_REQUEST_1:
				vty_out( vty, "  OAM_CALLBACK_SNMP_REQUEST  %lu\r\n",gGwOamSendCount[Temp]);			
				break;
			case GW_CALLBACK_SNMP_REPONSE_1:
				vty_out( vty, "  OAM_CALLBACK_SNMP_RESPONSE  %lu\r\n",gGwOamSendCount[Temp]);			
				break;
			case GW_CALLBACK_SNMP_TRAP_1:
				vty_out( vty, "  OAM_CALLBACK_SNMP_TRAP  %lu\r\n",gGwOamSendCount[Temp]);			
				break;
			case GW_CALLBACK_IGMPAUTH_REQUEST:
				vty_out( vty, "  OAM_CALLBACK_IGMPAUTH_REQUEST  %lu\r\n",gGwOamSendCount[Temp]);			
				break;
			case GW_CALLBACK_IGMPAUTH_REPONST:
				vty_out( vty, "  OAM_CALLBACK_IGMPAUTH_RESPONSE  %lu\r\n",gGwOamSendCount[Temp]);			
				break;
			case GW_CALLBACK_VCONSOLE:
				vty_out( vty, "  OAM_CALLBACK_VCONSOLE  %lu\r\n",gGwOamSendCount[Temp]);	
				break;
		    default:
				break;
		}		
	}
#else
	int Temp = 0;
	vty_out( vty, " %-32s %-12s %-12s\r\n", "oam statistics", "rx pkts", "tx pkts" );
	vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OamTotal", gGwOamTotalRevCount, gGwOamTotalSendCount );	
	for( Temp = 0; Temp < GW_OPCODE_LAST; Temp++ )
	{
		switch(Temp)
		{
			case GW_CALLBACK_CLI_REQUEST_1:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_CLI_REQUEST", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );
				break;			
			case GW_CALLBACK_CLI_1:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_CLI", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );			
				break;

			case GW_CALLBACK_RESERVED:	
				break;

			case GW_CALLBACK_EQU_REQUEST:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_EQU_REQUEST", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );			
				break;
			case GW_CALLBACK_EUQINFO:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_EUQINFO", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );
				break;
			case GW_CALLBACK_ALARMORLOG	:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_ALARMORLOG", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );			
				break;
			case GW_CALLBACK_ALARMORLOG_REPONSE:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_ALARMORLOG_RESPONS", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );
				break;
			case GW_CALLBACK_FILE_RD_WRT_REQUEST:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_FILE_RD_WRT_REQUEST", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );			
				break;
			case GW_CALLBACK_FILE_RESERVED:
				break;
			case GW_CALLBACK_FILE_DATA_TRA:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_FILE_DATA_TRA", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );
				break;
			case GW_CALLBACK_FILE_TRA_ACK:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_FILE_TRA_ACK", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );			
				break;
			case GW_CALLBACK_SNMP_REQUEST_1:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_SNMP_REQUEST", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );			
				break;
			case GW_CALLBACK_SNMP_REPONSE_1:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_SNMP_RESPONSE", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );			
				break;
			case GW_CALLBACK_SNMP_TRAP_1:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_SNMP_TRAP", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );			
				break;
			case GW_CALLBACK_IGMPAUTH_REQUEST:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_IGMPAUTH_REQUEST", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );			
				break;
			case GW_CALLBACK_IGMPAUTH_REPONST:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_IGMPAUTH_RESPONSE", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );			
				break;
			case GW_CALLBACK_VCONSOLE:
				vty_out( vty, " %-32s %-12lu %-12lu\r\n", "OAM_CALLBACK_VCONSOLE", gGwOamRevCount[Temp], gGwOamSendCount[Temp] );	
				break;
		    default:
				break;
		}
	}
#endif
	return 0;
}

int statOamclear(void)
{
	short int Temp = 0;
	gGwOamRevTempCount = 0;
	gGwOamTotalRevCount = 0;
	for(Temp = 0; Temp < GW_OPCODE_LAST/*0x14*/;Temp++)
		gGwOamRevCount[Temp] = 0;
	gGwOamTotalSendCount = 0;
	for(Temp = 0; Temp < GW_OPCODE_LAST/*0x14*/;Temp++)
		gGwOamSendCount[Temp] = 0;	
	return OAM_OK;
}

#ifdef OAM_DEBUG
int bingotestaaa(short int ponid, short int onuid)
{
	short int  llid = 0;
	unsigned short aallid = 0;
	llid = GetLlidByOnuIdx( ponid, onuid-1);
	aallid = GetLlidByOnuIdx( ponid, onuid-1);
	sys_console_printf("  llid %d\r\n",llid);
	sys_console_printf("  aallid %d\r\n",aallid);
	return 0;
}

#if 0
/*===============for test================*/
void testCommTask(void)
{
	ULONG aulMsg[4] = {0};
	/*ULONG msgType = 0;*/
	long lRes = 0;
	/*GW_OAM_MSG_t *pMsgOam = NULL;*/
	while(1)
	    {
		lRes = VOS_QueReceive(gOamMsgQId, aulMsg, WAIT_FOREVER);
		
		if (VOS_ERROR == lRes)
			{
			sys_console_printf("|--ERROR at msgQReceive   lRes = %ld \r\n",lRes);
				break;
			}		
		/*pMsgOam = (GW_OAM_MSG_t *)aulMsg[3];		
		msgType = aulMsg[2];*/
		sys_console_printf(" CommTask() OK ~~~\r\n");
			/*end of switch (msgType)...*/
	}/*end of FOREVER*/

}


int oamtest1()
{
	gOamMsgQId = VOS_QueCreate(20, VOS_MSG_Q_FIFO);
	if (gOamMsgQId == 0)
		{
			sys_console_printf(" gSendNoSemId Error!\r\n");
			return OAM_ERR;
		}
		
	gOamTaskId = ( VOS_HANDLE )VOS_TaskCreate( "CommOlt", (ULONG)110, (VOS_TASK_ENTRY)testCommTask, NULL);
	sys_console_printf(" gOamTaskId = %d\r\n", gOamTaskId);
	return OAM_OK;
}
#endif

/****************************************
* CommOnuTask
* 描述: 该函数
*
*
*
*
******************************************/
void CommOnu(void)
{
	ULONG lRes = 0;
	ULONG aulMsg[4] = {0};
	/*int	iRes = 0;*/
	
	while(1)
	    {
		sys_console_printf(" CommOnuTask()\r\n");

		if (VOS_ERROR == VOS_QueReceive(gOamOnuMsgQId, aulMsg, WAIT_FOREVER))

			{
			sys_console_printf("|--ERROR at msgQReceive.CommOnuOamTask()  lRes = %ld\r\n",lRes);

				break;
			}
		sys_console_printf(" CommOnu() OK ~~~\r\n");
	}
	return;
}
int oamtest2()
{
	gOamOnuMsgQId = VOS_QueCreate(20, VOS_MSG_Q_FIFO);
	if (gOamOnuMsgQId == 0)
		{
			sys_console_printf(" gSendNoSemId Error!\r\n");
			return OAM_ERR;
		}
		
	gOamOnuTaskId = VOS_TaskCreate( "CommOnu", (ULONG)110, (VOS_TASK_ENTRY)CommOnu, NULL);
	sys_console_printf(" gOamOnuTaskId = %d\r\n", gOamOnuTaskId);
	return OAM_OK;
}

short int testsend(unsigned int msgType)
{
	ULONG ulMsg[4] = {0};
	GW_OAM_MSG_t  *pOamMsg;
	pOamMsg = (GW_OAM_MSG_t *)VOS_Malloc((ULONG)sizeof(GW_OAM_MSG_t), (ULONG)MODULE_OAM);
	memset(pOamMsg, 0, sizeof(GW_OAM_MSG_t));
	
	ulMsg[3] = (ULONG)(pOamMsg);
	
	if (1 == msgType)
		{
			if (OAM_OK == VOS_QueSend( gOamMsgQId, ulMsg, NO_WAIT, MSG_PRI_NORMAL ))
				{
			             return(OAM_OK);
				}
			else
				{
					VOS_Free((VOID *)pOamMsg);
					ASSERT( 0 );
					/*sys_console_printf(" OamIODataSend() ERROR! \r\n");*/
					return(FILE_CTRL_ERRCODE_FLOWERR);
				}
		}
	else if (2 == msgType)
		{
			if (OAM_OK == VOS_QueSend( gOamOnuMsgQId, ulMsg, NO_WAIT, MSG_PRI_NORMAL ))
				{
			             return(OAM_OK);
				}
			else
				{
					VOS_Free((VOID *)pOamMsg);
					pOamMsg = NULL;
					ASSERT( 0 );
					/*sys_console_printf(" OamIODataSend() ERROR! \r\n");*/
					return(FILE_CTRL_ERRCODE_FLOWERR);
				}
		}
	else
		{
		VOS_Free((VOID *)pOamMsg);
		pOamMsg = NULL;
		sys_console_printf("   msgType error!\r\n");
		}

	return OAM_OK;
}
/***********************************************************
* CommCliRspRvc
*
* 说明: 当Comm_Cli_request_transmit函数别cli模块调用时,同事必须调用本函数
*		,以用于等待响应结果.在函数cli_handler_function中可设置一个全局变量
*		当在一定时间内,如果该全局变量的值不便,则重新使用该
*	
*
************************************************************/


/*============for test============*/
STATUS oam_frame_build(int Tempsize,int oamsize)
{
       unsigned char          *sent_data = NULL;
       unsigned short    sent_data_size = oamsize;
       unsigned long lSendSerNo = 0;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;		   
	if (Tempsize== 1)
		{
		sent_data_size = 5;
		sent_data = (unsigned char *)g_malloc(sent_data_size);
		if (sent_data == NULL)
			return OAM_ERR;
		memset(sent_data,0x55,sent_data_size);
		}
	else if (Tempsize == 2)
		{
			sent_data_size = 4300;
			sent_data = (unsigned char *)g_malloc(sent_data_size);
			if (sent_data == NULL)
				return OAM_ERR;			
			memset(sent_data,0xa5,1500);
			memset((sent_data+1500), 0x55,1500);
			memset((sent_data+1500+1500),0x33,(sent_data_size-1500*2));
		}

	else if (Tempsize == 3)

		{
			sent_data_size = 2600;
			sent_data = (unsigned char *)g_malloc(sent_data_size);
			if (sent_data == NULL)
				return OAM_ERR;			
			memset(sent_data,0x66,1500);
			memset((sent_data+1500),0x99,(sent_data_size-1500));
		}
	else if (Tempsize == 4)
		{
			sent_data_size = 1469;
			sent_data = (unsigned char *)g_malloc(sent_data_size);
			if (sent_data == NULL)
				return OAM_ERR;			
			memset(sent_data,0x6F,sent_data_size);
		}
	else if (Tempsize == 5)
		{
			sent_data_size = oamsize;
			sent_data = (unsigned char *)g_malloc(sent_data_size);
			if (sent_data == NULL)
				return OAM_ERR;			
			memset(sent_data,0x13,sent_data_size);
		}
	else{
		sys_console_printf("Invalue Tempsize parameter !Error !\r\n");
		return -1;
		}
	
	/*if (OAM_OK != COMM_OAM_frame_send  ( 
                      								OltId, 
                      								OnuId,
                      								GW_OPCODE_CLI_REQUEEST,
                      								broadcast_llid,
                      								sent_data,  
                      								sent_data_size,
                      								GW_DEVTYPE_OLT,
                      								lSendSerNo))
		{
			sys_console_printf("  COMM_OAM_frame_send()!\r\n");
                      return OAM_ERR;
		}*/


 	/*COMM_OAM_frame_send  (  OltId,  OnuId, EUQ_INFO_REQUESET, broadcast_llid, sent_data, sent_data_size);*/
	g_free(sent_data);
	return OAM_OK;

}



/*============for test============*/
STATUS onu_frame_build(int Tempsize,int oamsize)
{

       unsigned short      OltId = 0;
       unsigned short      OnuId = 0;
       unsigned char          *sent_data = NULL;
       unsigned short    sent_data_size = oamsize;
       GW_OAM_MSG_t  OamMsgStru;
	if (Tempsize== 1)
		{
		sent_data_size = 5;
		sent_data = (unsigned char *)g_malloc(sent_data_size);
		if (sent_data == NULL)
			return OAM_ERR;
		memset(sent_data,0x55,sent_data_size);
		}
	else if (Tempsize == 2)
		{
			sent_data_size = 4300;
			sent_data = (unsigned char *)g_malloc(sent_data_size);
			if (sent_data == NULL)
				return OAM_ERR;			
			memset(sent_data,0xa5,1500);
			memset((sent_data+1500), 0x55,1500);
			memset((sent_data+1500+1500),0x33,(sent_data_size-1500*2));
		}

	else if (Tempsize == 3)

		{
			sent_data_size = 2600;
			sent_data = (unsigned char *)g_malloc(sent_data_size);
			if (sent_data == NULL)
				return OAM_ERR;
			memset(sent_data,0x66,1500);
			memset((sent_data+1500),0x99,(sent_data_size-1500));
		}
	else if (Tempsize == 4)
		{
			sent_data_size = 1469;
			sent_data = (unsigned char *)g_malloc(sent_data_size);
			if (sent_data == NULL)
				return OAM_ERR;			
			memset(sent_data,0x6F,sent_data_size);
		}
	else if (Tempsize == 5)
		{
			sent_data_size = oamsize;
			sent_data = (unsigned char *)g_malloc(sent_data_size);
			if (sent_data == NULL)
				return OAM_ERR;			
			memset(sent_data,0x13,sent_data_size);
		}
	else{
		sys_console_printf("Invalue Tempsize parameter !Error !\r\n");
		return -1;
		}
	
	memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
	OamMsgStru.OltId = OltId;
	OamMsgStru.OnuId = OnuId;				
	OamMsgStru.brdcstLlid= 0;
	OamMsgStru.oamOpcode = GW_OPCODE_CLI_REQUEEST;

	OnuIODataSend(OltId, OnuId, sent_data, GW_MSG_SEND, sent_data_size, &OamMsgStru);
	
	g_free(sent_data);
	return OAM_OK;

}

STATUS oam_send(unsigned long  count, int type, int oamsize)
{
	ULONG temp = 0;
	sys_console_printf("count = %d  type = %d oamsize  %d\r\n",count, type, oamsize);
	sys_console_printf("count = %d  type = %d oamsize  %d\r\n",count, type, oamsize);

	for (temp = 0; temp<count; temp++ )
		{
		oam_frame_build(type,oamsize);
		sys_console_printf("count = %d\r\n",temp);
		}
	return OAM_OK;
}

STATUS onu_send(unsigned long  count, int type, int oamsize)
{
	ULONG temp = 0;
	sys_console_printf("count = %d  type = %d oamsize  %d\r\n",count, type, oamsize);
	sys_console_printf("count = %d  type = %d oamsize  %d\r\n",count, type, oamsize);

	for (temp = 0; temp<count; temp++ )
		{
		onu_frame_build(type,oamsize);
		sys_console_printf("count = %d\r\n",temp);
		taskDelay(300);
		}
	return OAM_OK;
}

STATUS oam_debug_frame_printf(short int OltId, short OnuId, int oam_frame_length,  unsigned char *OamFrame)
{
	unsigned long sendSerNo = 0;
	unsigned int oamOui = 0;
	unsigned char gwOpCode = 0; 
	unsigned short oamWholeLen = 0;
	unsigned short int payLoadOffSet = 0;
	unsigned short int payLoadLen = 0;

	sys_console_printf("\r\n OltId : %d   OnuId : %d\r\n",OltId,OnuId);
	
	sys_console_printf(" frame length : %d\r\n",oam_frame_length);
	
	CommOamPadSendsernoFromOamGet  (OamFrame, &sendSerNo);
	sys_console_printf(" OAM sendSerNo : %d\r\n",sendSerNo);
	
	CommOamOpcodeFromOamGet(&gwOpCode,OamFrame);
	sys_console_printf(" OAM private OpCode : %d\r\n",sendSerNo);
	
	CommOamOpcodeCheck(gwOpCode);
	CommOamPadOuiFromOamGet  (&oamOui,OamFrame);
	if (oamOui == 0)
		sys_console_printf(" oamOui : NULL!\r\n");
	sys_console_printf(" OAM private OUI : %x\r\n",oamOui);
	
	CommOamPadWholepktlenFromOamGet  (&oamWholeLen, (OamFrame));
	sys_console_printf(" OAM wholelen : %d\r\n",oamWholeLen);
	
	CommOamPadPayloadoffsetFromOamGet  ((OamFrame), &payLoadOffSet);
	sys_console_printf(" OAM payloadoffset : %d\r\n",payLoadOffSet);
	
	CommOamPadPayloadlenFromOamGet ((OamFrame), &payLoadLen);
	sys_console_printf(" OAM payLoadLen : %d\r\n",payLoadLen);

	{
	int iLoop = 0;
	sys_console_printf(" OAM Send bpdu \r\n");
	sys_console_printf(" OAM PAD length=%d \r\n",oam_frame_length);
	sys_console_printf(" Oam Content:\r\n");
	for(iLoop=0;iLoop<oam_frame_length;iLoop++)
		{
			sys_console_printf("%02x ",OamFrame[iLoop]);
		}
	}
			sys_console_printf("\r\n=============================\r\n\r\n");  
			
	return 	OAM_OK;	
}


/*********************************************************
 函数名: 
 描  述: 该函数从公用数据接收缓冲区接收数据并解析协议数据包
 
 输入参数:    
				aM_uint8_Port  --端口号
				aM_uint8_VlanId --vlan号
				aP_uchar_Buff  -- 接收数据缓冲区的指针
 				aM_uint32_Len  --接收数据的长度byte单位
 			     
输出参数:  lM_uint32_Idx 缓冲区数据池号

 返回值: lM_uint32_Idx
 
 说  明：                                         
*********************************************************/
short int OamIODataToRevQueue(unsigned short OltId, 
						unsigned short OnuId, 
						unsigned char *pBuf,
						unsigned int msgType,
						unsigned int  bufLen)
{
	unsigned int  queueIdx = 0;
	ULONG ulMsg[4] = {0};
	GW_OAM_MSG_t  *pOamMsg;

	/*
	pOamMsgStru->OltId = OltId;
	pOamMsgStru->OnuId = OnuId;				
	pOamMsgStru->brdcstLlid= BrdcstLlid;
	pOamMsgStru->oamOpcode = oamOpcode;
	pOamMsgStru->sendSerNo = lSendSerNo;
	*/
	
	
	if (GW_MSG_SEND == msgType)
		{
        		queueIdx=OamIOWriteDataToQueue(GW_MSG_SEND_QUIDX, pBuf,bufLen);
		}
	else if (GW_MSG_REVEICE == msgType)
		{
			queueIdx=OamIOWriteDataToQueue(GW_MSG_REV_QUIDX, pBuf,bufLen);
		}
	else
		{
			return OAM_ERR;
		}
	
	
	pOamMsg = (GW_OAM_MSG_t *)VOS_Malloc((ULONG)sizeof(GW_OAM_MSG_t), (ULONG)MODULE_OAM);
	if (NULL == pOamMsg)
		return OAM_ERR;
	pOamMsg->bufPara = queueIdx;
	pOamMsg->OltId = OltId;
	pOamMsg->OnuId = OnuId;
	ulMsg[2] = msgType;
	ulMsg[3] = (ULONG)(pOamMsg);

	/* modified by chenfj 2008-3-14
	     以下两处地方使用msgType 替代原来的宏定义:  GW_MSG_REV_QUIDX
	    */
	if (queueIdx<gOamQueueCtl[msgType].MaxSize)
	 {	
	if (OAM_OK == VOS_QueSend( gOamOnuMsgQId, ulMsg, NO_WAIT, MSG_PRI_NORMAL ))


		  {
             		return(OAM_OK);
		  }
		  else
		  {
			VOS_Free((VOID *)pOamMsg);
			pOamMsg = NULL;			  
		  	/*sys_console_printf(" OamIODataToRevQueue() ERROR! \r\n");*/
		  	 OAMIOClearQueue(msgType,queueIdx);
			 ASSERT( 0 );
			 return(OAM_ERR);
		  }
	 }
	 else
	 	{
			VOS_Free((VOID *)pOamMsg);
			pOamMsg = NULL;		 	
	 		return (OAM_ERR);
	 	}

}


/*********************************************************
 函数名: 
 描  述: 该函数从公用数据接收缓冲区接收数据并解析协议数据包
 
 输入参数:    
				aM_uint8_Port  --端口号
				aM_uint8_VlanId --vlan号
				aP_uchar_Buff  -- 接收数据缓冲区的指针
 				aM_uint32_Len  --接收数据的长度byte单位
 			     
输出参数:  lM_uint32_Idx 缓冲区数据池号

 返回值: lM_uint32_Idx
 
 说  明：                                         
*********************************************************/

short int OnuIODataToRevQueue(unsigned short OltId, 
						unsigned short OnuId, 
						unsigned char *pBuf,
						unsigned int msgType,
						unsigned int  bufLen)
{
	unsigned int  queueIdx = 0;
	ULONG ulMsg[4] = {0};
	GW_OAM_MSG_t  *pOamMsg;

	/*
	pOamMsgStru->OltId = OltId;
	pOamMsgStru->OnuId = OnuId;				
	pOamMsgStru->brdcstLlid= BrdcstLlid;
	pOamMsgStru->oamOpcode = oamOpcode;
	pOamMsgStru->sendSerNo = lSendSerNo;
	*/
	
	
	if (GW_MSG_SEND == msgType)
		{
        		queueIdx=OamIOWriteDataToQueue(GW_MSG_SEND_QUIDX, pBuf,bufLen);
		}
	else if (GW_MSG_REVEICE == msgType)
		{
			queueIdx=OamIOWriteDataToQueue(GW_MSG_REV_QUIDX, pBuf,bufLen);
		}
	else
		{
			return OAM_ERR;
		}
	
	pOamMsg = (GW_OAM_MSG_t *)VOS_Malloc((ULONG)sizeof(GW_OAM_MSG_t), (ULONG)MODULE_OAM);
	if (NULL == pOamMsg)
		return OAM_ERR;
	memset(pOamMsg, 0, sizeof(GW_OAM_MSG_t));
	pOamMsg->bufPara = queueIdx;
	pOamMsg->OltId = OltId;
	pOamMsg->OnuId = OnuId;
	ulMsg[2] = msgType;
	ulMsg[3] = (ULONG)(pOamMsg);

	/* modified by chenfj 2008-3-14
	     以下两处地方使用msgType 替代原来的宏定义:  GW_MSG_REV_QUIDX
	    */
	if (queueIdx<gOamQueueCtl[msgType].MaxSize)
	 {
	if (OAM_OK == VOS_QueSend( gOamMsgQId, ulMsg, NO_WAIT, MSG_PRI_NORMAL ))
		  {
             		return(OAM_OK);
		  }
		  else
		  {
			 VOS_Free((VOID *)pOamMsg);
			 pOamMsg = NULL;		  
		  	 OAMIOClearQueue(msgType,queueIdx);
			 ASSERT( 0 );
			 return(OAM_ERR);
		  }
	 }
	 else
	 	{
			VOS_Free((VOID *)pOamMsg);
			pOamMsg = NULL;	 	
	 		return (OAM_ERR);
	 	}

}


int oam_debug_Reveice_fram(short int OltId, short int OnuId, int oam_frame_length, unsigned char *OamFrame, unsigned int DevType)
{
	int iRes = OAM_OK;
	/*oam_debug_frame_printf(OltId, onuId, oam_frame_length, OamFrame);*/
	/*CommEhtFrameReveive(OltId, onuId, oam_frame_length, OamFrame);*/
	/*iRes = CommOamFrameReveive(OltId, OnuId, oam_frame_length, OamFrame);*/
	if (GW_DEVTYPE_OLT == DevType)
		iRes = OamIODataToRevQueue(OltId, OnuId, OamFrame, GW_MSG_REVEICE, oam_frame_length);
	else if (GW_DEVTYPE_ONU == DevType)
		iRes = OnuIODataToRevQueue(OltId, OnuId, OamFrame,GW_MSG_REVEICE,oam_frame_length);
	else
		return OAM_ERR;
	return iRes;
}

int oamInit()
{
	CommOamMagInfoInit();
	return 0;
}



void oamRev(short int OltId,
				short int OnuId,
				short int Length,
				char *pFrame)
{

		sys_console_printf(" OltId : %d  OnuId : %d \r\n",OltId,OnuId);
		sys_console_printf(" OAM PAD length=%d \r\n",Length);
		sys_console_printf(" content : %s", pFrame);
		sys_console_printf(" OAM_OK!\r\n");
		return;
}
void oammib(void)
{
}

int sendflag_1 = 0;

int testsendoam(short int oltid, short int onuid)
{
	short int sent_data_size = 0; 
	short int llid = 0;
	int iRes = OAM_OK;
	char Content[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x02, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x88, 0x09 ,0x03, 0x00, 0x50, 0xfe, 0x00, 0x0f,                     
					0xe9, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x09, 0x61, 0x74, 0x75, 0x20, 0x61, 0x67, 0x69, 0x6e,                     
					0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	sent_data_size = strlen(sizeof(Content));
	sys_console_printf(" sent_data_size = %d\r\n",sent_data_size);
	sys_console_printf(" sent_data_size = %d\r\n",sent_data_size);
	llid = GetLlidByOnuIdx( oltid, onuid);
	if ( OAM_INVALID_LLID == llid )
		return OAM_ERR;	
	sys_console_printf(" oltid : %d   onuid : %d   llid : %d\r\n",oltid, onuid, llid);

	if( sendflag_1 == 1 )
		{
#if 0
	iRes = PAS_send_frame ( oltid, 
							60, 
							PON_PORT_SYSTEM, 
							llid, 
							OAM_SEND_SPECIFIED_LLID,
							Content);
#else
	iRes = OLT_SendFrame2CNI ( oltid, 
							llid, 
							Content, 
							60);
#endif    
	if (iRes != OAM_OK)
		{
			sys_console_printf(" iRes = %d\r\n",iRes);
			sys_console_printf(" iRes = %d\r\n",iRes);
			return (iRes);
		}
		}
	return iRes;

}

#endif

#if 0
/*==============for onu oam module test===============*/

/***********************************************************
*
*
*
*
*
************************************************************/
int Pas6201_comm_alarm_transmit(unsigned short OltId, 
									unsigned short OnuId, 
									unsigned char * pAlarmbuf,
									const unsigned short Alarm_data_size)
{
	int iRes = OAM_OK;
	GW_OAM_MSG_t  OamMsgStru;
	unsigned long lSendSerNo = 0;
	if(pAlarmbuf== NULL)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;
#ifdef OAM_DEBUG
	memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
	OamMsgStru.brdcstLlid = 0;
	OamMsgStru.bufSize = Alarm_data_size;
	OamMsgStru.OltId = OltId;
	OamMsgStru.OnuId = OnuId;
	OamMsgStru.oamOpcode = GW_OPCODE_ALARM_OR_LOG_TRAN;
	OnuIODataSend(OltId, OnuId, pAlarmbuf,GW_MSG_SEND,Alarm_data_size, &OamMsgStru);
#else
	/*iRes = PASONU_transmit_frame ((INT32U *)pAlarmbuf,Alarm_data_size,0,0 );*/
	iRes = COMM_OAM_frame_send  ( OltId, OnuId,GW_OPCODE_ALARM_OR_LOG_TRAN, 0, pAlarmbuf, Alarm_data_size, GW_DEVTYPE_ONU, lSendSerNo);
#endif
	
	if(iRes != OAM_OK)
		{
		VOS_Free((VOID*)pAlarmbuf);
		sys_console_printf("Error!comm_alarm_transmit !\r\n");
		}
	return iRes;
}

/***********************************************************
*
*
*
*
*
************************************************************/
int Pas6201_comm_log_transmit(unsigned short OltId, 
									unsigned short OnuId, 
									void *plogbuf,
									const unsigned short Log_data_size)
{
	int iRes = OAM_OK;
	GW_OAM_MSG_t  OamMsgStru;
	unsigned long lSendSerNo = 0;
	if(plogbuf== NULL)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;
#ifdef OAM_DEBUG
	memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
	OamMsgStru.brdcstLlid = 0;
	OamMsgStru.bufSize = Log_data_size;
	OamMsgStru.OltId = OltId;
	OamMsgStru.OnuId = OnuId;
	OamMsgStru.oamOpcode = GW_OPCODE_ALARM_OR_LOG_TRAN;
	OnuIODataSend(OltId, OnuId, plogbuf,GW_MSG_SEND,Log_data_size, &OamMsgStru);
#else
	/*iRes = PASONU_transmit_frame ((INT32U *)plogbuf,Log_data_size,0,0 );	*/	
	iRes = COMM_OAM_frame_send  ( OltId, OnuId,GW_OPCODE_ALARM_OR_LOG_TRAN,0, plogbuf, Log_data_size, GW_DEVTYPE_ONU, lSendSerNo);
#endif

	if(iRes != OAM_OK)
		{
		VOS_Free((VOID*)plogbuf);
		sys_console_printf("Error!comm_log_transmit !\r\n");
		}
	return iRes;
}

int Pas6201_comm_EUQ_info_respone_transmit(unsigned short OltId, 
													unsigned short OnuId, 
													void *pEuqinfobuf,
													const unsigned short Euq_info_data_size)
{

	int iRes = OAM_OK;
	GW_OAM_MSG_t  OamMsgStru;
	unsigned long lSendSerNo = 0;
	if(pEuqinfobuf== NULL)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;
#ifdef OAM_DEBUG
	memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
	OamMsgStru.brdcstLlid = 0;
	OamMsgStru.bufSize = Euq_info_data_size;
	OamMsgStru.OltId = OltId;
	OamMsgStru.OnuId = OnuId;
	OamMsgStru.oamOpcode = GW_OPCODE_EUQ_INFO_RESPONSE;
	OnuIODataSend(OltId, OnuId, pEuqinfobuf,GW_MSG_SEND,Euq_info_data_size, &OamMsgStru);
#else
	/*iRes = PASONU_transmit_frame ((INT32U *)plogbuf,Log_data_size,0,0);	*/	
	iRes = COMM_OAM_frame_send  ( OltId, OnuId,GW_OPCODE_EUQ_INFO_RESPONSE, 0, pEuqinfobuf, Euq_info_data_size, GW_DEVTYPE_ONU, lSendSerNo);
#endif
	
	if(iRes != OAM_OK)
		{
		VOS_Free((VOID*)pEuqinfobuf);
		sys_console_printf("Error!comm_log_transmit !\r\n");
		}
	return iRes;
}


/***********************************************************
*
*
*
*
*
************************************************************/
int Pas6201_comm_snmp_respone_transmit(unsigned short OltId, 
												unsigned short OnuId, 
												void *pSnmpbuf,
												const unsigned short Snmp_data_size)
{

	int iRes = OAM_OK;
	GW_OAM_MSG_t  OamMsgStru;
	unsigned long lSendSerNo = 0;
	if(pSnmpbuf== NULL)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;
#ifdef OAM_DEBUG
	memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
	OamMsgStru.brdcstLlid = 0;
	OamMsgStru.bufSize = Snmp_data_size;
	OamMsgStru.OltId = OltId;
	OamMsgStru.OnuId = OnuId;
	OamMsgStru.oamOpcode = GW_OPCODE_SNMP_RESPONSE;
	OnuIODataSend(OltId, OnuId, pSnmpbuf,GW_MSG_SEND,Snmp_data_size, &OamMsgStru);
#else
	/*iRes = PASONU_transmit_frame ((INT32U *)plogbuf,Log_data_size,0,0 );	*/
	iRes = COMM_OAM_frame_send  ( OltId, OnuId,GW_OPCODE_SNMP_RESPONSE, 0, pSnmpbuf, Snmp_data_size, GW_DEVTYPE_ONU,lSendSerNo);
#endif

	if(iRes != OAM_OK)
		{
		VOS_Free((VOID*)pSnmpbuf);
		sys_console_printf("Error!snmp_respone_transmit !\r\n");
		}
	return iRes;
}



/***********************************************************
*
*
*
*
*
************************************************************/


int Pas6201_comm_snmp_trap_frame_transmit(unsigned short OltId, 
													unsigned short OnuId, 
													void *pSnmpbuf,
													const unsigned short Snmp_data_size)
{

	int iRes = OAM_OK;
	unsigned long lSendSerNo = 0;
	GW_OAM_MSG_t  OamMsgStru;
	if(pSnmpbuf== NULL)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;
#ifdef OAM_DEBUG
	memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
	OamMsgStru.brdcstLlid = 0;
	OamMsgStru.bufSize = Snmp_data_size;
	OamMsgStru.OltId = OltId;
	OamMsgStru.OnuId = OnuId;
	OamMsgStru.oamOpcode = GW_OPCODE_SNMP_TRAP;
	OnuIODataSend(OltId, OnuId, pSnmpbuf,GW_MSG_SEND,Snmp_data_size, &OamMsgStru);
#else
	iRes = COMM_OAM_frame_send  ( OltId, OnuId,GW_OPCODE_SNMP_TRAP, 0, pSnmpbuf, Snmp_data_size, GW_DEVTYPE_ONU, lSendSerNo );
#endif
	if(iRes != OAM_OK)
		{
		VOS_Free((VOID*)pSnmpbuf);
		sys_console_printf("Error!snmp_trap_transmit !\r\n");
		}
	return iRes;
}

int Pas6201_comm_Igmp_Reponse(
				const unsigned short OltId, 
				const unsigned short OnuId,
				unsigned char  *pClibuf,
				const unsigned short cli_data_size)
{
	int iRes = OAM_OK;
	GW_OAM_MSG_t  OamMsgStru;
	unsigned long lSendSerNo = 0;
	if(pClibuf == NULL)
		return OAM_NULL_POINTER ;
	if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
		return OAM_ERR;
#ifdef OAM_DEBUG
	memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
	OamMsgStru.brdcstLlid = 0;
	OamMsgStru.bufSize = cli_data_size;
	OamMsgStru.OltId = OltId;
	OamMsgStru.OnuId = OnuId;
	OamMsgStru.oamOpcode = Gw_OPCODE_IGMPAUTH_REPONSE;
	iRes = OamIODataSend(OltId, OnuId, 	pClibuf, GW_MSG_SEND, cli_data_size, &OamMsgStru);
#else
	iRes = COMM_OAM_frame_send( OltId, OnuId,Gw_OPCODE_IGMPAUTH_REPONSE, 0, pClibuf, cli_data_size, GW_DEVTYPE_ONU, lSendSerNo);
#endif
	if(iRes != OAM_OK)
		{
		VOS_Free((VOID*)pClibuf);
		/*sys_console_printf("Error!Cli_request_transmit !\r\n");*/
		return iRes;
		}
	return iRes;
}



/****************************************
* CommOnuOamTask
* 描述: 该函数
*
*
*
*
******************************************/
void CommOnuOamTask(void)
{
	ULONG aulMsg[4] = {0};
	int	iRes = 0;
	GW_OAM_MSG_t	*pMsgOam = NULL;
	unsigned short      	OltId = 0;
        unsigned short     	OnuId = 0;
        /*unsigned int		queuePro;*/
        unsigned char 		oamOpcode;
        short int 			broadcast_llid;
        unsigned char          *sent_data;
        unsigned short		sent_data_size;
	unsigned int 		msgType = 0;
	unsigned long 		lSendSerNo = 0;
	unsigned short 	bufpara = 0;

	
	while(1)
	    {
#ifdef OAM_DEBUG
	         iRes = msgQReceive(gOamOnuMsgQId, (char*)aulMsg, sizeof(ULONG)*4,WAIT_FOREVER);
	        if (iRes == ERROR)
#else
		if (VOS_ERROR== VOS_QueReceive(gOamOnuMsgQId, aulMsg, WAIT_FOREVER))
#endif	
			{
			sys_console_printf("|--ERROR at msgQReceive.CommOnuOamTask()\r\n");
#ifdef OAM_DEBUG			
				sys_console_printf("|--ERROR at msgQReceive\r\n");
#endif
				break;
			}

		pMsgOam = (GW_OAM_MSG_t *)aulMsg[3];

		msgType = aulMsg[2];
		switch(msgType)
			{
				case GW_MSG_SEND:

					OltId = pMsgOam->OltId;
					OnuId = pMsgOam->OnuId;			
					oamOpcode = pMsgOam->oamOpcode;
					broadcast_llid = pMsgOam->brdcstLlid;
					bufpara = pMsgOam->bufPara;
					sent_data = gOamQueueCtl[GW_MSG_SEND_QUIDX].DataBuff[bufpara].MesData;
					sent_data_size = gOamQueueCtl[GW_MSG_SEND_QUIDX].DataBuff[bufpara].MesLen;
					switch(oamOpcode)
						{
							case GW_OPCODE_CLI_REQUEEST:
							case GW_OPCODE_SNMP_REQUEST:
							case GW_OPCODE_EUQ_INFO_REQUESET:
								
								if (OAM_OK != ONU_OAM_frame_send(OltId, OnuId, /*queuePro,*/\
													oamOpcode,broadcast_llid, sent_data,   sent_data_size))											
									{
#ifdef OAM_DEBUG
									sys_console_printf(" 1. PAS_send_frame() ERROR at task! \r\n");
#endif				
									}
								/*释放缓冲区*/
								OAMIOClearQueue(0,bufpara);
								VOS_Free((VOID*)pMsgOam);
								break;


							case GW_OPCODE_EUQ_INFO_RESPONSE:
							case GW_OPCODE_SNMP_RESPONSE:
							case GW_OPCODE_SNMP_TRAP:
							case GW_OPCODE_CLI_RESPONSE:

								if (OAM_OK != ONU_OAM_frame_send(OltId, OnuId, /*queuePro,*/ \
													oamOpcode,broadcast_llid, sent_data,   sent_data_size))
									{
#ifdef OAM_DEBUG
									sys_console_printf(" 1. PAS_send_frame() ERROR at task! \r\n");
#endif				
									}
								/*释放缓冲区*/
								VOS_Free((VOID*)pMsgOam);
								OAMIOClearQueue(0,bufpara);								
								break;							
		
							default :
								break;
						}
					
					break;

				case GW_MSG_REVEICE:
					OltId = pMsgOam->OltId;
					OnuId = pMsgOam->OnuId;			
					bufpara = pMsgOam->bufPara;
					sent_data = gOamQueueCtl[GW_MSG_REV_QUIDX].DataBuff[bufpara].MesData;
					sent_data_size = gOamQueueCtl[GW_MSG_REV_QUIDX].DataBuff[bufpara].MesLen;	
					
					CommOamOpcodeFromOamGet(&oamOpcode, sent_data);
					switch(oamOpcode)
						{	
							case GW_OPCODE_CLI_REQUEEST:
							case GW_OPCODE_SNMP_REQUEST:
							case GW_OPCODE_EUQ_INFO_REQUESET:
								iRes = CommOamPayloadReveice ( OltId, OnuId,   (unsigned short)(sent_data_size-GW_OAM_PAD_OUI), (sent_data+GW_OAM_PAD_OUI)) ;
								if (iRes != OAM_OK)
									{
#ifdef OAM_DEBUG
									sys_console_printf(" 3. PAS_send_frame() ERROR at task! \r\n");
#endif				
									}
								/*释放缓冲区*/
								OAMIOClearQueue(1,bufpara);
								VOS_Free((VOID*)pMsgOam);
								break;

							case GW_OPCODE_FILE_READ_REQUEST:
								break;
							case GW_OPCODE_FILE_TRANSFER_ACK:
								break;
							case GW_OPCODE_FILE_DATA_TRAN:	
								iRes = CommOamPayloadReveice ( OltId, OnuId,   (unsigned short)(sent_data_size-GW_OAM_PAD_OUI), (sent_data+GW_OAM_PAD_OUI)) ;
								if (iRes != OAM_OK)
									{
#ifdef OAM_DEBUG
									sys_console_printf(" 3. PAS_send_frame() ERROR at task! \r\n");
#endif
									}
								
								/*释放缓冲区*/
								VOS_Free((VOID*)pMsgOam);
								OAMIOClearQueue(1,bufpara);								
								break;
							case GW_OPCODE_FILE_WRITE_REQUEST:
								
								break;
							case GW_OPCODE_EUQ_INFO_RESPONSE:
							case GW_OPCODE_SNMP_RESPONSE:
							case GW_OPCODE_SNMP_TRAP:
							case GW_OPCODE_CLI_RESPONSE:
								iRes = CommOamPayloadReveice ( OltId, OnuId,   (unsigned short)(sent_data_size-GW_OAM_PAD_OUI), (sent_data+GW_OAM_PAD_OUI)) ;
								if (iRes != OAM_OK)
									{
#ifdef OAM_DEBUG
									sys_console_printf(" 3. PAS_send_frame() ERROR at task! \r\n");
#endif				
									}
								/*释放缓冲区*/
								VOS_Free((VOID*)pMsgOam);
								OAMIOClearQueue(1,bufpara);								
								break;
								
							default:
									break;
						}/*end of switch (oamOpcode..)*/
					
				default :
					break;
			}/*end of switch (msgType)...*/
	}/*end of FOREVER*/

}





/****************************************************
* ONU_OAM_frame_send
* 描述: 该函数被cli或者snmp调用。封装GW的OAM帧
*		对于超长数据包，该函数进行分解数据包，并发送
*
*输入: OltId,OnuId,oam_opcode - 见格林威尔私有oam定义文件
*		broadcast_llid - 代表发送目的参数。当
*
*
*
*
******************************************************/
short int ONU_OAM_frame_send  ( 
                      const unsigned short      OltId, 
                      const unsigned short     OnuId,
                      /*unsigned int		queuePro,*/
                      unsigned char oam_opcode,
                      const short int broadcast_llid,
                      unsigned char          *sent_data,  
                      unsigned short    sent_data_size)                   
{
	int iRes = OAM_OK;
	short int llid = 0;
	unsigned short oam_frame_length = 0;
	unsigned short WholePktLen = 0;
	unsigned char * OnuOamFrame = NULL;
	char oam_head[32] = {0};
	unsigned long sendserno = 0;
	unsigned short int payloadoffset = 0;
	unsigned short int payloadlens = 0;
	if (gboolGwInit == FALSE)
		return OAM_NOT_INIT_ERR;

	OnuOamFrame = (unsigned char *)g_malloc(1500);
	CommOamHeadBuild( OltId, OnuId,OnuOamFrame, &oam_frame_length);

	
	/*if (iRes != OAM_OK)
		return (iRes);*/
	oam_frame_length += GW_OAM_PAD_LENGTH;
	CommOamPadOuiInsert (OnuOamFrame);
	
	/*check the oam/pad/opcode*/
	iRes = CommOamOpcodeCheck(oam_opcode);
	if(OAM_OK != iRes)
		return iRes;

	iRes = CommOamPadOpcodeInsert  (OnuOamFrame, oam_opcode);
	if (iRes != OAM_OK)
		return (OAM_ERR);
	
	/*获取发送sendserno*/
	iRes = CommOamPadSendsernoGet  (&sendserno);
	if (iRes != OAM_OK)
		return (OAM_ERR);
	
	CommOamPadSendsernoInsert (OnuOamFrame, sendserno);
	CommOamPadWholepktlenInsert  (OnuOamFrame, sent_data_size);

	/*if oam < (60 - 18 - 14)*/
	if(sent_data_size<(MIN_ETHERNET_FRAME_SIZE-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH))
		{
			CommOamPadPayloadoffsetInsert(OnuOamFrame, (unsigned short int)0);
			CommOamPadPayloadlenInsert(OnuOamFrame, sent_data_size);
			memset((OnuOamFrame+GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH+sent_data_size),0,\
							(MIN_ETHERNET_FRAME_SIZE-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH-sent_data_size));
			memcpy((OnuOamFrame+oam_frame_length),sent_data,sent_data_size);
			oam_frame_length = MIN_ETHERNET_FRAME_SIZE;
#ifdef OAM_DEBUG
			/*iRes = CommOamFrameReveive(OltId, OnuId, oam_frame_length,OamFrame);*/
			iRes = CommOamFrameReveive(   OltId,   OnuId, oam_frame_length,OnuOamFrame);
			if (iRes != OAM_OK)
				return iRes;
			iRes = oam_debug_Reveice_fram(OltId, OnuId, oam_frame_length,OnuOamFrame, GW_DEVTYPE_ONU);
			free(OnuOamFrame);	
			/*iRes = oam_debug_frame_printf(OltId, OnuId, oam_frame_length,OamFrame);*/
#else
			iRes = PAS_send_frame (  OltId, oam_frame_length, OAM_PORT_PON, OnuId, broadcast_llid, (void*)OnuOamFrame);
			free(OnuOamFrame);
#endif
			if (iRes != OAM_OK) 
			    {
					return OAM_ERR;
				}
		}
	/*if cli_data_size < (1480 - 18 - 14)*/
	/*send single oam*/
	else if (sent_data_size<=(MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH))
		{
			/*增加发送序列号，发送长度，偏移量*/
			CommOamPadPayloadoffsetInsert(OnuOamFrame, (unsigned short int)0);
			CommOamPadPayloadlenInsert(OnuOamFrame, sent_data_size);
			memcpy((OnuOamFrame+GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH), sent_data, sent_data_size);
			oam_frame_length += sent_data_size;
#ifdef OAM_DEBUG
			/*iRes = oam_debug_frame_printf(OltId, OnuId, oam_frame_length,OamFrame);
			iRes = CommOamFrameReveive(OltId, OnuId, oam_frame_length,OamFrame);*/
			iRes = CommOamFrameReveive(   OltId,   OnuId, oam_frame_length,OnuOamFrame);
			if (iRes != OAM_OK)
				return iRes;
			iRes = oam_debug_Reveice_fram(OltId, OnuId, oam_frame_length,OnuOamFrame, GW_DEVTYPE_ONU);
			free(OnuOamFrame);
#else
			iRes = PAS_send_frame (  OltId, oam_frame_length, OAM_PORT_PON, OnuId, broadcast_llid, (void*)OnuOamFrame);
			free(OnuOamFrame);
#endif			
			/*added at twenty nine june.For limiting speed of sentting oam frame*/
			taskDelay(GW_DEFAULT_TASKDELAY_TICK);
			if (iRes != OAM_OK) 
			    { 
					return OAM_ERR;
				}			
		}
		/*if cli_data_size > (1480 - 18 - 14)*/
		/*send multi-oam frame*/
	else 
		{
			unsigned short tempSendSize = sent_data_size;
			payloadoffset = 0;
			while(tempSendSize>(MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH)/*1500-18-14*/)
				{
					CommOamPadPayloadoffsetInsert(OnuOamFrame, (unsigned short int)payloadoffset);
					
					CommOamPadPayloadlenInsert(OnuOamFrame, (MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH));
					/*payloadlens = MAX_SIZE_OF_OAM_FRAME;*/
					memcpy((OnuOamFrame+GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH),(sent_data+payloadoffset),(MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH));
					oam_frame_length = MAX_SIZE_OF_OAM_FRAME;
#ifdef OAM_DEBUG
					/*iRes = oam_debug_frame_printf(OltId, OnuId, oam_frame_length,OamFrame);
					
					iRes = oam_debug_Reveice_fram(OltId, OnuId, oam_frame_length,OamFrame);*/
					iRes = CommOamFrameReveive(   OltId,   OnuId, oam_frame_length,OnuOamFrame);
					if (iRes != OAM_OK)
						return iRes;
					iRes = oam_debug_Reveice_fram(OltId, OnuId, oam_frame_length,OnuOamFrame, GW_DEVTYPE_ONU);
					
#else
					iRes = PAS_send_frame (  OltId, oam_frame_length, OAM_PORT_PON, OnuId, broadcast_llid, (void*)OnuOamFrame);
					
#endif		
					/*added at Jtwenty nine june.For limiting speed of sentting oam frame*/
					taskDelay(GW_DEFAULT_TASKDELAY_TICK);

					if (iRes != OAM_OK) 
						{
							return OAM_ERR;
						}					
					payloadoffset += MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH;		
					tempSendSize = tempSendSize-(MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH);
				}
			
			/*最后一个帧*/
			payloadlens = sent_data_size-payloadoffset + GW_OAM_HEADER_SIZE + GW_OAM_PAD_LENGTH;
			if (payloadlens == tempSendSize)
				return OAM_OK;
			CommOamPadPayloadoffsetInsert(OnuOamFrame, (unsigned short int)payloadoffset);
			CommOamPadPayloadlenInsert(OnuOamFrame,tempSendSize);
			memcpy((OnuOamFrame+GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH),(sent_data+payloadoffset), tempSendSize);
			oam_frame_length = tempSendSize+GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH;
			if (oam_frame_length <(MIN_ETHERNET_FRAME_SIZE))
				{
					memset((OnuOamFrame + oam_frame_length), 0, (MIN_ETHERNET_FRAME_SIZE-oam_frame_length));
					oam_frame_length = MIN_ETHERNET_FRAME_SIZE;
				}
#ifdef OAM_DEBUG
			/*iRes = CommOamFrameReveive(OltId, OnuId, oam_frame_length ,OamFrame);*/	
			iRes = CommOamFrameReveive(   OltId,   OnuId, oam_frame_length,OnuOamFrame);
			if (iRes != OAM_OK)
				return iRes;
			iRes = oam_debug_Reveice_fram(OltId, OnuId, oam_frame_length,OnuOamFrame, GW_DEVTYPE_ONU);
			/*iRes = oam_debug_frame_printf(OltId, OnuId, oam_frame_length,OamFrame);*/
			free(OnuOamFrame);
#else
			iRes = PAS_send_frame (  OltId, oam_frame_length, OAM_PORT_PON, OnuId, broadcast_llid, (void*)OnuOamFrame);	
			free(OnuOamFrame);
#endif				
			
			if (iRes != OAM_OK) 
			    {

				return OAM_ERR;
				}
			
		}
	return iRes;
}


#endif



#if 0




/****************************************************
* File_OAM_frame_send
* 描述: 该函数被cli或者snmp调用。封装GW的OAM帧
*		对于超长数据包，该函数进行分解数据包，并发送
*
*输入: OltId,OnuId,oam_opcode - 见格林威尔私有oam定义文件
*		broadcast_llid - 代表发送目的参数。当
*
*
*
*
******************************************************/
int File_OAM_frame_send  ( 
                      const unsigned short      OltId, 
                      const unsigned short      OnuId,
                      unsigned long		lSendSerNo,
                      /*unsigned int		queuePro,*/
                      unsigned char oam_opcode,
                      const short int broadcast_llid,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size)
{
	int iRes = OAM_OK;
	short int llid = 0;
	unsigned short oam_frame_length = 0;
	unsigned short WholePktLen = 0;
	unsigned char OamFrame[1500] = {0};
	char oam_head[32] = {0};
	unsigned long sendserno = 0;
	unsigned short int payloadoffset = 0;
	unsigned short int payloadlens = 0;
	GW_OAM_MSG_t  OamMsgStru;
	if (gboolGwInit == FALSE)
		return OAM_NOT_INIT_ERR;
	CommOamHeadBuild( OltId, OnuId,OamFrame, &oam_frame_length);
	/*iRes = CommOamHeaderBuild ( OltId, OamFrame);*/
	if (iRes != OAM_OK)
		return (iRes);
	oam_frame_length += GW_OAM_PAD_LENGTH;
	CommOamPadOuiInsert (OamFrame);
	/*check the oam/pad/opcode*/
	iRes = CommOamOpcodeCheck(oam_opcode);
	if(OAM_OK != iRes)
		return iRes;

	iRes = CommOamPadOpcodeInsert  (OamFrame, oam_opcode);
	if (iRes != OAM_OK)
		return (OAM_ERR);

	CommOamPadSendsernoInsert (OamFrame, lSendSerNo);
	CommOamPadWholepktlenInsert  (OamFrame, sent_data_size);

	/*if oam < (60 -20 - 18 - 14)*/
	if(sent_data_size<(MIN_ETHERNET_FRAME_SIZE-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH-GW_EHTERNET_HEAD_LEN))
		{
			CommOamPadPayloadoffsetInsert(OamFrame, (unsigned short int)0);
			CommOamPadPayloadlenInsert(OamFrame, sent_data_size);
			memset((OamFrame+GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH+sent_data_size),0,\
							(MIN_ETHERNET_FRAME_SIZE-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH-sent_data_size));
			memcpy((OamFrame+oam_frame_length),sent_data,sent_data_size);
			oam_frame_length = MIN_ETHERNET_FRAME_SIZE;
#ifdef OAM_DEBUG
			memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
			OamMsgStru.OltId = OltId;
			OamMsgStru.OnuId = OnuId;				
			OamMsgStru.brdcstLlid= 0;
			OamMsgStru.oamOpcode = FILE_DATA_TRAN;
			OamMsgStru.sendSerNo = lSendSerNo;
			OamIODataSend(OltId, OnuId, &OamFrame, GW_DEVTYPE_OLT, oam_frame_length,	&OamMsgStru);
#else

#endif
			if (iRes != OAM_ERR) 
			    {
					return OAM_ERR;
				}
		}
	/*if cli_data_size < (1480 - 18 - 14)*/
	/*send single oam*/
	else if (sent_data_size<=(MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH))
		{
			/*增加发送序列号，发送长度，偏移量*/
			CommOamPadPayloadoffsetInsert(OamFrame, (unsigned short int)0);
			CommOamPadPayloadlenInsert(OamFrame, sent_data_size);
			memcpy((OamFrame+GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH),sent_data,sent_data_size);
			oam_frame_length += sent_data_size;
#ifdef OAM_DEBUG
			memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
			OamMsgStru.OltId = OltId;
			OamMsgStru.OnuId = OnuId;				
			OamMsgStru.brdcstLlid= 0;
			OamMsgStru.oamOpcode = FILE_DATA_TRAN;
			OamMsgStru.sendSerNo = lSendSerNo;
			iRes = OamIODataSend(OltId, OnuId, &OamFrame, GW_DEVTYPE_OLT, oam_frame_length,	&OamMsgStru);

#else

#endif			
			/*added at Jtwenty nine june.For limiting speed of sentting oam frame*/
			taskDelay(GW_DEFAULT_TASKDELAY_TICK);
			if (iRes != OAM_OK) 
			    { 
					return OAM_ERR;
				}			
		}
		/*if cli_data_size > (1480 - 18 - 14)*/
		/*send multi-oam frame*/
	else 
		{
			unsigned short tempSendSize = sent_data_size;
			payloadoffset = 0;
			while(tempSendSize>(MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH)/*1480-18-14*/)
				{
					CommOamPadPayloadoffsetInsert(OamFrame, (unsigned short int)payloadoffset);
					
					CommOamPadPayloadlenInsert(OamFrame, (MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH));
					payloadlens = MAX_SIZE_OF_OAM_FRAME;
					memcpy((OamFrame+GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH),(sent_data+payloadoffset),(MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH));
					oam_frame_length = MAX_SIZE_OF_OAM_FRAME;
#ifdef OAM_DEBUG

#else

#endif		
					/*added at Jtwenty nine june.For limiting speed of sentting oam frame*/
					taskDelay(GW_DEFAULT_TASKDELAY_TICK);

					if (iRes != OAM_OK) 
						{
							return OAM_ERR;
						}					
					payloadoffset += MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH;		
					tempSendSize = tempSendSize-(MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH);
				}
			
			/*最后一个帧*/
			CommOamPadPayloadoffsetInsert(OamFrame, (unsigned short int)payloadoffset);
			payloadlens = sent_data_size-payloadoffset + GW_OAM_HEADER_SIZE + GW_OAM_PAD_LENGTH;
			CommOamPadPayloadlenInsert(OamFrame,tempSendSize);
			memcpy((OamFrame+GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH),(sent_data+payloadoffset), tempSendSize);
			oam_frame_length = tempSendSize+GW_OAM_HEADER_SIZE+GW_OAM_PAD_LENGTH;
			if (oam_frame_length <(MIN_ETHERNET_FRAME_SIZE))
				{
					memset((OamFrame + oam_frame_length), 0, (MIN_ETHERNET_FRAME_SIZE-oam_frame_length));
					oam_frame_length = MIN_ETHERNET_FRAME_SIZE;
				}
#ifdef OAM_DEBUG

#else
			
 
#endif				
			
			if (iRes != OAM_OK) 
			    {

				return OAM_ERR;
				}
			
		}
	return iRes;
}


/****************************************
* OamTask
* 描述: 该函数
*
*
*
*
******************************************/

void CommFileTask(void)
{
	ULONG aulMsg[4] = {0};
	int	iRes = 0;
	GW_OAM_MSG_t	*pMsgOam = NULL;
	unsigned short      	OltId = 0;
        unsigned short     	OnuId = 0;
        /*unsigned int		queuePro;*/
        unsigned char 		oamOpcode;
        short int 			broadcast_llid;
        unsigned char          *sent_data;
        unsigned short		sent_data_size;
	unsigned int 		msgType = 0;
	unsigned long 		lSendSerNo = 0;
	unsigned short 	bufpara = 0;

	
	while(1)
	    {

#ifdef OAM_DEBUG
	         iRes = msgQReceive(gFileMsgQId, (char*)aulMsg, sizeof(ULONG)*4,WAIT_FOREVER);
	        if (iRes == OAM_ERR)
#else
		if (OAM_OK != VOS_QueReceive(gFileMsgQId, aulMsg, 2*100))
#endif	
			{
#ifdef OAM_DEBUG			
				sys_console_printf("|--ERROR at msgQReceive\r\n");
#endif
				break;
			}		
		pMsgOam = (GW_OAM_MSG_t *)aulMsg[3];		
		msgType = aulMsg[2];
		
		switch(msgType)
		{
				case GW_MSG_SEND:

					OltId = pMsgOam->OltId;
					OnuId = pMsgOam->OnuId;			
					oamOpcode = pMsgOam->oamOpcode;
					broadcast_llid = pMsgOam->brdcstLlid;
					bufpara = pMsgOam->bufPara;
					lSendSerNo = pMsgOam->sendSerNo;
					sent_data = gOamQueueCtl[GW_MSG_SEND_QUIDX].DataBuff[bufpara].MesData;
					sent_data_size = gOamQueueCtl[GW_MSG_SEND_QUIDX].DataBuff[bufpara].MesLen;
					/*处理流程
					1. 当正在进行文件传输时, 不允许第二个文件传输的开始;
					2. */
					switch(oamOpcode)
						{
							case GW_OPCODE_FILE_READ_REQUEST:
								/*1. 读取文件到缓冲区*/

								/*2. 回应对方,统一或者拒绝,或者返回错误*/

								/*3. 等待对方回应,如果回应正常,则开始发送文件.循环进行*/

								/*4. 结束,发送ack 结束报文*/
								break;
							case GW_OPCODE_FILE_WRITE_REQUEST:
								/**/
								
								break;
							case GW_OPCODE_FILE_TRANSFER_ACK:
								break;
							case GW_OPCODE_FILE_DATA_TRAN:	
								lSendSerNo = pMsgOam->sendSerNo;
								iRes = File_OAM_frame_send  ( OltId, OnuId,lSendSerNo,oamOpcode, broadcast_llid, sent_data, sent_data_size);
								if (iRes != OAM_OK)
									{
#ifdef OAM_DEBUG
									sys_console_printf(" 2. PAS_send_frame() ERROR at task! \r\n");
#endif				
									}
								
								/*释放缓冲区*/
								VOS_Free((VOID*)pMsgOam);
								OAMIOClearQueue(0,bufpara);								
								break;							
							default :
								break;
						}
					
					break;

				case GW_MSG_REVEICE:
					OltId = pMsgOam->OltId;
					OnuId = pMsgOam->OnuId;			
					bufpara = pMsgOam->bufPara;
					sent_data = gOamQueueCtl[GW_MSG_REV_QUIDX].DataBuff[bufpara].MesData;
					sent_data_size = gOamQueueCtl[GW_MSG_REV_QUIDX].DataBuff[bufpara].MesLen;	
					
					CommOamOpcodeFromOamGet(&oamOpcode, sent_data);
					switch(oamOpcode)
						{	
							case GW_OPCODE_CLI_REQUEEST:
							case GW_OPCODE_SNMP_REQUEST:
							case GW_OPCODE_EUQ_INFO_REQUESET:
								iRes = CommOamPayloadReveice ( OltId, OnuId,   (unsigned short)(sent_data_size-GW_OAM_PAD_OUI), (sent_data+GW_OAM_PAD_OUI)) ;
								if (iRes != OAM_OK)
									{
#ifdef OAM_DEBUG
									sys_console_printf(" 3. PAS_send_frame() ERROR at task! \r\n");
#endif				
									}
								/*释放缓冲区*/
								OAMIOClearQueue(1,bufpara);
								VOS_Free((VOID*)pMsgOam);
								break;

							case GW_OPCODE_FILE_READ_REQUEST:
							case GW_OPCODE_FILE_WRITE_REQUEST:
							case GW_OPCODE_FILE_TRANSFER_ACK:
							case GW_OPCODE_FILE_DATA_TRAN:	
								iRes = CommOamPayloadReveice ( OltId, OnuId,   (unsigned short)(sent_data_size-GW_OAM_PAD_OUI), (sent_data+GW_OAM_PAD_OUI)) ;
								if (iRes != OAM_OK)
									{
#ifdef OAM_DEBUG
									sys_console_printf(" 3. PAS_send_frame() ERROR at task! \r\n");
#endif				
									}
								
								/*释放缓冲区*/
								VOS_Free((VOID*)pMsgOam);
								OAMIOClearQueue(1,bufpara);								
								break;
								
							case GW_OPCODE_EUQ_INFO_RESPONSE:
							case GW_OPCODE_SNMP_RESPONSE:
							case GW_OPCODE_SNMP_TRAP:
							case GW_OPCODE_CLI_RESPONSE:
								iRes = CommOamPayloadReveice ( OltId, OnuId,   (unsigned short)(sent_data_size-GW_OAM_PAD_OUI), (sent_data+GW_OAM_PAD_OUI)) ;
								if (iRes != OAM_OK)
									{
#ifdef OAM_DEBUG
									sys_console_printf(" 3. PAS_send_frame() ERROR at task! \r\n");
#endif				
									}
								/*释放缓冲区*/
								VOS_Free((VOID*)pMsgOam);
								OAMIOClearQueue(1,bufpara);								
								break;
								
							default:
									break;
						}/*end of switch (oamOpcode..)*/
					
				default :
					break;
			}

		

	}/*end of FOREVER*/

}


short int FileCtrlInit(void)
{
	gFileMsgQId = VOS_QueCreate(20, VOS_MSG_Q_FIFO);
	if (gFileMsgQId == 0)
		{
			sys_console_printf(" gSendNoSemId Error!\r\n");
			return OAM_ERR;
		}
		
	gFileTaskId = VOS_TaskCreate( "tFileTast", (ULONG)110, (VOS_TASK_ENTRY)CommFileTask, NULL);
	if (gFileTaskId == 0)
		{
			return OAM_ERR;
		}
	
	return OAM_OK;
}

/***********************************************************
* FileReadTransmittingAck
* 描述: 该函数用于发送回应
* 
*
*
************************************************************/
short int FileReadTransmittingAck(
				const unsigned short OltId, 
				const unsigned short OnuId,
				unsigned int fileLens,
				unsigned int fileOff,
				unsigned int dataLens,
				GW_OAM_FILECTR_t *pstrFileCtrl
				)
{
	unsigned int datafcs = 0;	
	unsigned int iRes = OAM_OK;
	GW_OAM_FILECTR_t strFileCtrl;
	
	memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));				
	iRes = CommFileTransferAck(OltId, 
							OnuId,
							FILE_CTRL_ACK_TRANSMITTING, 
							FILE_CTRL_ERRCODE_OK, 
							fileLens, 
							fileOff, 
							dataLens, 
							0);
	if (iRes != OAM_OK)
				{/*此步如果发送不出,则直接返回*/
					/*FileCtrlDataBufFree(OltId, OnuId);*/
					return iRes;
				}	
	iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
	if (OAM_OK !=iRes) 
		{	
			memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
			iRes = CommFileTransferAck(OltId, 
									OnuId,
									FILE_CTRL_ACK_TRANSMITTING, 
									FILE_CTRL_ERRCODE_OK, 
									fileLens, 
									fileOff, 
									dataLens, 
									0);
			if (iRes != OAM_OK)					
				{
					return iRes;
				}	
			iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
			if (OAM_OK !=iRes)
				{
				return iRes;
				}
		}
	if (strFileCtrl.ackCode != FILE_CTRL_ACK_TRANSMITTING)
		return (strFileCtrl.ErrCode);/*如果此步返回得值为0的 情况*/
	memcpy(pstrFileCtrl, &strFileCtrl, sizeof(GW_OAM_FILECTR_t));
	return iRes;
}


/***********************************************************
*
*
* 
*
*
************************************************************/
short int FileReadTransmitOverAck(
				const unsigned short OltId, 
				const unsigned short OnuId,
				unsigned int fileLens,
				unsigned int fileOff,
				unsigned int dataLens)
{
	unsigned int datafcs = 0;	
	unsigned int iRes = OAM_OK;
	GW_OAM_FILECTR_t strFileCtrl;
	
	memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));				
	iRes = CommFileTransferAck(OltId, 
							OnuId,
							FILE_CTRL_ACK_TXOVER, 
							FILE_CTRL_ERRCODE_OK, 
							fileLens, 
							fileOff, 
							dataLens, 
							0);
	if (iRes != OAM_OK)
				{/*此步如果发送不出,则直接返回*/
					/*FileCtrlDataBufFree(OltId, OnuId);*/
					return iRes;
				}	
	iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
	if (OAM_OK !=iRes) 
		{	
			memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
			iRes = CommFileTransferAck(OltId, 
									OnuId,
									FILE_CTRL_ACK_TXOVER, 
									FILE_CTRL_ERRCODE_OK, 
									fileLens, 
									fileOff, 
									dataLens, 
									0);
			if (iRes != OAM_OK)					
				{
					return iRes;
				}	
			iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
			if (OAM_OK !=iRes)
				{
				return iRes;
				}
		}
	if (strFileCtrl.ackCode != FILE_CTRL_ACK_TXOVER)
		return (strFileCtrl.ErrCode);/*如果此步返回得值为0的 情况*/
	
	return iRes;
}

/***********************************************************
* FileReadRequestTransmit
* 描述:
* 
*
*
************************************************************/
int FileReadRequestTransmit(
				const unsigned short OltId, 
				const unsigned short OnuId,
				const short int broadcast_llid,
				unsigned char *pFilepbuf,
				const unsigned short File_data_size)
{
	int iRes = OAM_OK;
	GW_OAM_FILECTR_t strFileCtrl;
	GW_OAM_MSG_t  OamMsgStru;
	unsigned long lSendSerNo = 0;
	unsigned int	fileLens = 0;
	unsigned int	dataLen = 0;
	unsigned char *pfilebuf = NULL;
	unsigned char fileData[129] = {0};

	/*判断是否有文件正在接收,当前只是允许有一个文件在接收*/
	/*if (pgFileBuf != NULL)
		return OAM_ERR;*/
	iRes = FileCtrlDataBufCheck(OltId, OnuId);
	if (OAM_OK == iRes)	
		return FILE_CTRL_ERRCODE_SYSERRO;
	
	if (File_data_size>128)
		return OAM_ERR;
	fileData[0] = 1;/*1 - read from; 2 - wrt to*/
	memcpy((fileData+1), pFilepbuf, File_data_size);
	

	/*获取发送sendserno*/
	iRes = CommOamPadSendsernoGet  (&lSendSerNo);
	if (iRes != OAM_OK)
		return (OAM_ERR);
	memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
	OamMsgStru.OltId = OltId;
	OamMsgStru.OnuId = OnuId;				
	OamMsgStru.brdcstLlid= 0;
	OamMsgStru.oamOpcode = GW_OPCODE_FILE_READ_REQUEST;
	OamMsgStru.sendSerNo = lSendSerNo;
	if (OAM_OK != OamIODataSend(OltId, OnuId, fileData, GW_MSG_SEND, (1+File_data_size), &OamMsgStru))
		return OAM_ERR;
	
	memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
	iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
	if (OAM_OK !=iRes)
		{
			if (OAM_OK != OamIODataSend(OltId, OnuId, fileData, GW_MSG_SEND, (1+File_data_size), &OamMsgStru))
				return OAM_ERR;
			iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
			if (OAM_OK !=iRes)
				{
				return iRes;
				}
		}
	if (strFileCtrl.ackCode != FILE_CTRL_ACK_RDACCEPT)
		return (strFileCtrl.ErrCode);
	
	/*1.申请空间,
	2.启动文件接收任务,
	3.并向接收callback 注册接收函数
	4.发送ack,开始接收*/
	fileLens = strFileCtrl.fileLen;
	if (FILE_MAX_LENGTH < fileLens)
		return FILE_CTRL_ERRCODE_LONGERR;
	if (FILE_MIN_LENGTH > fileLens)
		return FILE_CTRL_ERRCODE_SHORTERR;
		
	iRes = FileCtrlDataBufMalloc(OltId, OnuId, pFilepbuf,  File_data_size, fileLens);
	if (OAM_OK != iRes)
		{
			iRes = CommFileTransferAck(OltId, 
									OnuId,
									FILE_CTRL_ACK_TXERROR, 
									FILE_CTRL_ERRCODE_SYSNORSC, 
									fileLens, 
									0, 
									0, 
									0);
			return iRes;
		}

	/*2.*/

	/*3.*/

	/*4.*/

	iRes = FileCtrlDataRev();
	/*一旦发生错误,则根据实际情况进行处理*/
	switch(iRes)
		{	
			case OAM_INVALUEBUF_ERR:
				iRes = CommFileTransferAck(OltId, 
										OnuId,
										FILE_CTRL_ACK_TXERROR, 
										iRes, 
										fileLens, 
										0, 
										0, 
										0);				
				break;
			case OAM_TIME_OUT:
			case FILE_CTRL_ERRCODE_FLOWERR:
			case OAM_MSGQSEND_ERR:
			case OAM_INVALUE_QUEUE_ERR:
			case OAM_INVALUE_MSGTYPE_ERR:
				iRes = CommFileTransferAck(OltId, 
										OnuId,
										FILE_CTRL_ACK_TXERROR, 
										iRes, 
										fileLens, 
										0, 
										0, 
										0);
				FileCtrlDataBufFree(OltId, OnuId);
				break;
			case OAM_OK:
				/* modified by wutongwu at september 2*/
				/*iRes = CommOltMsgCallback( OltId, OnuId, FILE_CTRL_READ,pgFileRevBuf[OltId][OnuId]->fileLen, pgFileRevBuf[OltId][OnuId]->pDataBlk);*/
				FileCtrlDataBufFree(OltId, OnuId);
				break;
			default:
				break;
		}
	return iRes;
}

/*=====================文件传输接口定义=========================*/

short int FileCtrlDataRev(void)
{
	unsigned short OltId = 0;
	unsigned short OnuId = 0;
	unsigned char *pDataBlk = NULL;
	unsigned int fileOff = 0;
	unsigned int dataLens = 0;
	unsigned int fileLens = 0;
	unsigned int datafcs = 0;	
	unsigned int iRes = OAM_OK;
	unsigned short	TempCount = 0;
	GW_OAM_FILECTR_t strFileCtrl;
	
	memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
	iRes = CommFileTransferAck(OltId, 
							OnuId,
							FILE_CTRL_ACK_TXSTART, 
							FILE_CTRL_ERRCODE_OK, 
							fileLens, 
							0, 
							0, 
							0);	
	if (iRes != OAM_OK)
		return iRes;
	
	if (OAM_OK != (iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl)))
		{
			memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
			iRes = CommFileTransferAck(OltId, 
									OnuId,
									FILE_CTRL_ACK_TXSTART, 
									FILE_CTRL_ERRCODE_OK, 
									fileLens, 
									0, 
									0, 
									0);
			if (OAM_OK != (iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl)))
				return iRes;
		}
	
	OltId = strFileCtrl.oltId;
	OnuId = strFileCtrl.onuId;
	fileOff = strFileCtrl.fileOff;
	fileLens = strFileCtrl.fileLen;
	datafcs = strFileCtrl.datafcs;
	dataLens = strFileCtrl.dataLen;
	pDataBlk = strFileCtrl.pFileBuf;
	iRes = FileCtrlDataBufCheck(OltId, OnuId);
	if (OAM_OK != iRes)	
		return iRes;

	/*判断该文件是否为接收的文件*/
	if (strFileCtrl.fileLen != pgFileRevBuf[OltId][OnuId]->fileLen)
		return FILE_CTRL_ERRCODE_FLOWERR;
	if (strFileCtrl.ackCode != FILE_CTRL_ACK_TRANSMITTING)
		return (FILE_CTRL_ERRCODE_FLOWERR );
	
	/*如果第一次收到文件分片不是第一片, 则继续等待一个15/2的周期*/
	if (0 != strFileCtrl.fileOff)
		{
			memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
			iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
			if (OAM_OK !=iRes)
				{
				return FILE_CTRL_ERRCODE_FLOWERR;
				}
		}

    if (0 == strFileCtrl.fileOff)
    	{/*第一个数据包*/
    		/*1.检查是否已经存在该文件*/
		if (pgFileRevBuf[OltId][OnuId]->fileOff != 0)
			return (FILE_CTRL_ERRCODE_FLOWERR);
		
    		/*2. 检查是否该数据包为一个独立的数据包*/
		if(strFileCtrl.dataLen == strFileCtrl.fileLen)
			{

			memcpy(pgFileRevBuf[OltId][OnuId]->pDataBlk, strFileCtrl.pFileBuf, strFileCtrl.dataLen);

			/*当文件传输完毕,发FILE_CTRL_ACK_TXOVER (0x303)类型的ack*/
			iRes = FileReadTransmitOverAck(OltId,  OnuId,fileLens,fileOff,dataLens);
			if (iRes != OAM_OK)
				{
				return iRes;
				}
			/*回调函数*/
			/**/
			
			return (iRes);
			}
		else
			{
				/*2. 该数据包为非独立数据包*/
				/*是否该文件的总长度> FILE_MAX_LENGTH ?*/
				if(fileLens>FILE_MAX_LENGTH)
					{
					return (FILE_CTRL_ERRCODE_LONGERR);
					}
				
				/*接收第一个分片*/
				memcpy(pgFileRevBuf[OltId][OnuId]->pDataBlk, strFileCtrl.pFileBuf, strFileCtrl.dataLen);
				pgFileRevBuf[OltId][OnuId]->fileOff += strFileCtrl.dataLen;
				/*Ack 回应*/
				while(OAM_OK == (iRes = FileReadTransmittingAck(OltId, OnuId,fileLens,fileOff, dataLens,&strFileCtrl) )  )
					{
						memcpy((pgFileRevBuf[OltId][OnuId]->pDataBlk+pgFileRevBuf[OltId][OnuId]->fileOff), strFileCtrl.pFileBuf, strFileCtrl.dataLen);
						if ((strFileCtrl.fileOff+strFileCtrl.dataLen) != strFileCtrl.fileLen)
							{
							iRes = OAM_OK;
							break;
							}
					}
				if (OAM_OK != iRes)
					{
						return iRes;
					}
				iRes = FileReadTransmitOverAck(OltId,  OnuId,fileLens,fileOff,dataLens);
				if (iRes != OAM_OK)
					{
						return iRes;
					}
				/*modified by wutw at september 2*/
				/*iRes = CommOltMsgCallback( OltId, OnuId, FILE_CTRL_READ,pgFileRevBuf[OltId][OnuId]->fileLen, pgFileRevBuf[OltId][OnuId]->pDataBlk);*/
				return (iRes);
			}
    	}

	/*================================*/
	else
		return FILE_CTRL_ERRCODE_SYSERRO;/*end of else*/	
}



/****************************************************
* FileCtrlDataTransmmit
* 描述: 该函数发送文件，每次发送的数据快为64k，并等待对方的ack。
*		封装文件控制格式，该函数对文件进行分解数据块操作，并发送
*
*
*输入: OltId,OnuId,oam_opcode - 见格林威尔私有oam定义文件
*		broadcast_llid - 代表发送目的参数。当
*
*
*
*
******************************************************/
short int FileCtrlDataTransmmit( 
                      const unsigned short      OltId, 
                      const unsigned short      OnuId,
                      /*unsigned int		queuePro,*/
                      unsigned char oam_opcode,
                      const short int broadcast_llid,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size)
{
	unsigned int fileLen = sent_data_size;
	unsigned int tempLen = sent_data_size;
	unsigned int fileOff = 0;
	unsigned int dataLen = 0;
	unsigned int datafcs = 0;
	unsigned long lSendSerNo = 0;
	unsigned short cycCount = 2;
	unsigned char dataBlk[MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH-FILE_TRANSFER_HEAD]= {0};
	unsigned char fileTrHead[FILE_TRANSFER_HEAD] = {0};
	int iRes = OAM_OK;
	GW_OAM_FILECTR_t strFileCtrl;
	
	iRes = CommOamPadSendsernoGet(&lSendSerNo);
	if (iRes != OAM_OK)
		return iRes;
	fileOff = 0;
	if (tempLen>(MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH-FILE_TRANSFER_HEAD))
		{
		while(tempLen>(MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH-FILE_TRANSFER_HEAD)/*1500-18-14-16*/)
			{
				dataLen = GW_MAX_MSG_LENGTH;
				memset(dataBlk, 0, MAX_SIZE_OF_OAM_FRAME);		
				FileCtrlDataTransHeadBuild(dataBlk, fileLen, fileOff, dataLen, datafcs);		

				memcpy((dataBlk+FILE_TRANSFER_HEAD), (sent_data+fileOff), (MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH-FILE_TRANSFER_HEAD));
				tempLen = tempLen - (MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH-FILE_TRANSFER_HEAD);
				fileOff += dataLen;
				
				dataLen = (MAX_SIZE_OF_OAM_FRAME-GW_OAM_HEADER_SIZE-GW_OAM_PAD_LENGTH);
				
				/*FileCtrlDataTransRspTickTimeInit();*/
				
				iRes = File_OAM_frame_send  ( OltId, OnuId,lSendSerNo,oam_opcode,broadcast_llid, dataBlk, dataLen);
				if (iRes != OAM_OK)
					return iRes;
				
				/*等待onu的回应,是否传输正确/或者在这里进行重传
				网站是做*/
				memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
				iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
				if (OAM_OK !=iRes)
					{
						/*重新发送该请求报文,并等待回应*/
						memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
						iRes = File_OAM_frame_send  ( OltId, OnuId,lSendSerNo,oam_opcode,broadcast_llid, dataBlk, dataLen);
						if (iRes != OAM_OK)
							return iRes;
						iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
						if (OAM_OK !=iRes)
							return iRes;
					}
					
				if (strFileCtrl.ackCode != FILE_CTRL_ACK_TRANSMITTING)
					{
						return (strFileCtrl.ErrCode);
					}
			}
		
			if (tempLen != 0)
			{
				memset(dataBlk, 0, MAX_SIZE_OF_OAM_FRAME);
				dataLen = tempLen;
				FileCtrlDataTransHeadBuild(dataBlk, fileLen, fileOff, dataLen, datafcs);
				memcpy(dataBlk, sent_data, tempLen);
				/*FileCtrlDataTransRspTickTimeInit();*/
				memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
				iRes = File_OAM_frame_send  ( OltId, OnuId,lSendSerNo,oam_opcode,broadcast_llid, dataBlk, dataLen);
				if (iRes != OAM_OK)
					return iRes;
				
				/*等待onu的回应,是否传输正确或者在这里进行重传*/
				iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
				if (OAM_OK !=iRes)
					{
						iRes = File_OAM_frame_send  ( OltId, OnuId,lSendSerNo,oam_opcode,broadcast_llid, dataBlk, dataLen);
						if (iRes != OAM_OK)
							return iRes;
					}
				if (strFileCtrl.ackCode != FILE_CTRL_ACK_TRANSMITTING)
					{
						return (strFileCtrl.ErrCode);
					}
			}
		}
	
	else if (tempLen != 0)
		{
			memset(dataBlk, 0, MAX_SIZE_OF_OAM_FRAME);
			dataLen = tempLen;
			FileCtrlDataTransHeadBuild(dataBlk, fileLen, fileOff, dataLen, datafcs);
			memcpy(dataBlk, sent_data, tempLen);
			iRes = File_OAM_frame_send  ( OltId, OnuId,lSendSerNo,oam_opcode,broadcast_llid, dataBlk, dataLen);
			if (iRes != OAM_OK)
				return iRes;
			
			/*等待onu的回应,是否传输正确*/
			iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
			if (OAM_OK !=iRes)
				{
					/*重新发送该请求报文,并等待回应*/
					memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
					iRes = File_OAM_frame_send  ( OltId, OnuId,lSendSerNo,oam_opcode,broadcast_llid, dataBlk, dataLen);
					if (iRes != OAM_OK)
						return iRes;					
					iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
					if (OAM_OK !=iRes)
						return iRes;
				}
			if (strFileCtrl.ackCode != FILE_CTRL_ACK_TRANSMITTING)
				{
					return (strFileCtrl.ErrCode);
				}
		}

	/*文件传送完毕的oam帧并等待对方回应*/
	iRes = FileReadTransmitOverAck(OltId, OnuId, fileLen, fileOff, 0);

	return (iRes);
}

/***********************************************************
*
*
* 
*
*
************************************************************/
short int FileWrtRequestTransmit(
				const unsigned short OltId, 
				const unsigned short OnuId,
				const short int broadcast_llid, 
				unsigned char *pFilepbuf ,
				unsigned short File_data_size )
{
	int iRes = OAM_OK;
	GW_OAM_FILECTR_t strFileCtrl;
	GW_OAM_MSG_t  OamMsgStru;
	unsigned long lSendSerNo = 0;
	unsigned int	dataLen = 0;
	unsigned char fileData[129] = {0};
	
	if (File_data_size>128)
		return OAM_ERR;
	fileData[0] = 2;/*1 - read from; 2 - wrt to*/
	memcpy((fileData+1), pFilepbuf, File_data_size);

	/*发送请求oam报文,并等待回应*/
	/*获取发送sendserno*/
	iRes = CommOamPadSendsernoGet  (&lSendSerNo);
	if (iRes != OAM_OK)
		return (OAM_ERR);
	memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
	OamMsgStru.OltId = OltId;
	OamMsgStru.OnuId = OnuId;				
	OamMsgStru.brdcstLlid= 0;
	OamMsgStru.oamOpcode = GW_OPCODE_FILE_WRITE_REQUEST;
	OamMsgStru.sendSerNo = lSendSerNo;
	memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));	
	iRes = OamIODataSend(OltId, OnuId, fileData, GW_MSG_SEND, (1+File_data_size), &OamMsgStru);	
	if (OAM_OK !=iRes)
		return iRes;	

	iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
	if ((OAM_OK !=iRes) || (strFileCtrl.ackCode != 0x201))
		{	/*重发*/
			memset(&strFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
			iRes = OamIODataSend(OltId, OnuId, fileData, GW_MSG_SEND, (1+File_data_size), &OamMsgStru);	
			if (OAM_OK !=iRes)
				return iRes;
			iRes = FileCtrlDataTransRspTickTimeRun(OltId, OnuId, &strFileCtrl);
			if (OAM_OK !=iRes)
				return iRes;
		}
	return iRes;
}


/***************************************
*
*
*
*
*
*
******************************************/
int FileCtrlDataTransHeadBuild(unsigned char *pBuf,
								unsigned int fileLen, 
								unsigned int fileOff, 
								unsigned int dataLen, 
								unsigned int datafcs)
{
	pBuf[0] = (unsigned char)((fileLen >>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD))& 0xff);
	pBuf[1] = (unsigned char)((fileLen>>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE))&0xff);
	pBuf[2] = (unsigned char)((fileLen>>OAM_BITS_IN_BYTE)&0xff);
	pBuf[3] = (unsigned char)(fileLen & 0xff);	

	/*数据块的便宜量*/
	pBuf[4] = (unsigned char)((fileOff >>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD))& 0xff);
	pBuf[5] = (unsigned char)((fileOff>>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE))&0xff);
	pBuf[6] = (unsigned char)((fileOff>>OAM_BITS_IN_BYTE)&0xff);
	pBuf[7] = (unsigned char)(fileOff & 0xff);

	/*数据块的长度*/
	pBuf[8] = (unsigned char)((dataLen >>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD))& 0xff);
	pBuf[9] = (unsigned char)((dataLen>>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE))&0xff);
	pBuf[10] = (unsigned char)((dataLen>>OAM_BITS_IN_BYTE)&0xff);
	pBuf[11] = (unsigned char)(dataLen & 0xff);	

	/*校验值目前为备用*/
	pBuf[12] = (unsigned char)((datafcs >>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD))& 0xff);
	pBuf[13] = (unsigned char)((datafcs>>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE))&0xff);
	pBuf[14] = (unsigned char)((datafcs>>OAM_BITS_IN_BYTE)&0xff);
	pBuf[15] = (unsigned char)(datafcs & 0xff);		
	return OAM_OK;
}




/**************************************
* FileCtrlTrsDataParse()
* FILE Read Request  	0x05	文件读请求
* FILE Write Request  	0x06	文件写请求
* FILE data transfer  	0x07	文件传送
* FILE transfer ACK  	0x08	文件传送应答
* FILE Reserved 		0x09-0x0a	为文件传送保留

*
* 
*
*
*
*
****************************************/
short int FileCtrlTrsDataParse(unsigned short oltId, 
							unsigned short onuId, 
							const unsigned char GwOpCode, 
							unsigned long len, 
							unsigned char *pBuf )
{
	unsigned short ackCode = 0;
	unsigned short  ErrCode = 0;
	unsigned int fileLen = 0;
	unsigned int fileOff = 0;
	unsigned int dataLen = 0;
	unsigned int datafcs = 0;	
	unsigned char *pFile = NULL;
	if (pBuf == NULL)
		return OAM_ERR;
	
	if (gbFileRev[oltId][onuId] != TRUE)
		return OAM_ERR;	

	/*文件的长度*/
	fileLen = (unsigned int)((pBuf[0] <<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD))& 0xff) +\
			(unsigned int)((pBuf[1]<<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE))&0xff) + \
			(unsigned int)((pBuf[2]<<OAM_BITS_IN_BYTE)&0xff) + \
			(unsigned int)pBuf[3] ;	

	/*获取数据块的偏移量*/
	fileOff = (unsigned int)(pBuf[4] <<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD)) + \
			(unsigned int)(pBuf[5]<<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE)) + \
			(unsigned int)(pBuf[6]<<OAM_BITS_IN_BYTE) + \
			(unsigned int)pBuf[7] ;	

	/*获取数据块的长度*/
	dataLen = (unsigned int)(pBuf[8] <<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD)) + \
			(unsigned int)(pBuf[9]<<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE)) + \
			(unsigned int)(pBuf[10]<<OAM_BITS_IN_BYTE) + \
			(unsigned int)pBuf[11] ;	

	/*获取数据块的校验值*/
	datafcs = (unsigned int)(pBuf[12] <<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD)) + \
			(unsigned int)(pBuf[13]<<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE)) + \
			(unsigned int)(pBuf[14]<<OAM_BITS_IN_BYTE) + \
			(unsigned int)pBuf[15] ;	
	
	memset(&gstrOamFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
	gstrOamFileCtrl.oltId = oltId;
	gstrOamFileCtrl.onuId = onuId;
	gstrOamFileCtrl.ackCode = FILE_CTRL_ACK_TRANSMITTING;
	gstrOamFileCtrl.ErrCode = FILE_CTRL_ERRCODE_OK;
	gstrOamFileCtrl.fileLen = fileLen;
	gstrOamFileCtrl.fileOff = fileOff;
	gstrOamFileCtrl.dataLen = dataLen;
	gstrOamFileCtrl.datafcs = datafcs;
	gstrOamFileCtrl.pFileBuf = pBuf;
	gbFileRev[oltId][onuId]  = FALSE;
	return OAM_OK;

/*
	if (0x302 == ackCode)
		{
			gbFileTrn = TRUE;
		}
 	else if (0x301 == ackCode)
 		{
 			gbFileWrtAllow = TRUE;
 		}

	gFileAckCode = ackCode;
	
	switch(ackCode)
		{
			case FILE_CTRL_ACK_RDREFUCTED:
				gbFileRdRefucted = FALSE;
				break;
			case FILE_CTRL_ACK_RDACCEPT:
				gbFileRdAllow = FALSE;
				break;
			case FILE_CTRL_ACK_WRTREFUCTED:
				gbFileWrtRefucted = FALSE;
				break;
			case FILE_CTRL_ACK_WRTACCEPT:
				gbFileWrtAllow = FALSE;
				break;
			case FILE_CTRL_ACK_TXERROR:
				gbFileTxErr = FALSE;
				break;
			case FILE_CTRL_ACK_TXSTART:
				gbFileTxStart = FALSE;
				break;
			case FILE_CTRL_ACK_TRANSMITTING:
				gbFileTransmitting = FALSE;
				break;
			case FILE_CTRL_ACK_TXOVER:
				gbFileTxOver = FALSE;
				break;
			default:
				return OAM_ERR;
			
		}*/

/*
#define FILE_CTRL_ERRCODE_OK				0x00
#define FILE_CTRL_ERRCODE_SYSBUSY			0x01
#define FILE_CTRL_ERRCODE_SYSNORSC			0x02
#define FILE_CTRL_ERRCODE_SYSERRO			0x03
#define FILE_CTRL_ERRCODE_FLOWERR			0x04
#define FILE_CTRL_ERRCODE_NOTFILE			0x05
#define FILE_CTRL_ERRCODE_LONGERR			0x06
#define FILE_CTRL_ERRCODE_SHORTERR			0x07
#define FILE_CTRL_ERRCODE_NOTMATCH			0x08
#define FILE_CTRL_ERRCODE_CHECKERR			0x09
#define FILE_CTRL_ERRCODE_SAVEERR			0x60
*/
	/*gFileErrCode  = ErrCode;
	switch(ErrCode)
		{
			case FILE_CTRL_ERRCODE_OK:
				break;
			case FILE_CTRL_ERRCODE_SYSBUSY:
				break;
			case FILE_CTRL_ERRCODE_SYSNORSC:
				break;
			case FILE_CTRL_ERRCODE_SYSERRO:
				break;
			case FILE_CTRL_ERRCODE_NOTFILE:
				break;
			case FILE_CTRL_ERRCODE_LONGERR:
				break;
			case FILE_CTRL_ERRCODE_SHORTERR:
				break;
			case FILE_CTRL_ERRCODE_NOTMATCH:
				break;
			case FILE_CTRL_ERRCODE_CHECKERR:
				break;
			case FILE_CTRL_ERRCODE_SAVEERR:
				break;
			default:
				return OAM_ERR;
		}*/
	
/*
#ifdef OAM_DEBUG
	pFile = (unsigned char *)g_malloc(fileLen);
#else
	pFile = (unsigned char *)VOS_Malloc(fileLen, (ULONG)MODULE_FILECRL);
#endif
*/
	
}

/**************************************
* FILE Read Request  	0x05	文件读请求
* FILE Write Request  	0x06	文件写请求
* FILE data transfer  	0x07	文件传送
* FILE transfer ACK  	0x08	文件传送应答
* FILE Reserved 		0x09-0x0a	为文件传送保留

*
* 
*
*
*
*
****************************************/
short int FileCtrlTrsAckParse(unsigned short oltId, 
							unsigned short onuId, 
							const unsigned char GwOpCode, 
							unsigned long len, 
							unsigned char *pBuf,
							unsigned short *pAckCode,
							unsigned short *pErrcode)
{
	unsigned short ackCode = 0;
	unsigned short  ErrCode = 0;
	unsigned int fileLen = 0;
	unsigned int fileOff = 0;
	unsigned int dataLen = 0;
	unsigned int datafcs = 0;	
	unsigned char *pFile = NULL;
	if (pBuf == NULL)
		return OAM_OK;
	
	if (gbFileRev[oltId][onuId] != TRUE)
		return OAM_ERR;	
	
	/*获取ackcode*/
	ackCode =(unsigned short)(pBuf[0] <<OAM_BITS_IN_BYTE) + (unsigned short)pBuf[1];	

	/*获取当前的errcode*/
	ErrCode =  (unsigned short)(pBuf[2]<<OAM_BITS_IN_BYTE) + (unsigned short)pBuf[3];

	/*文件的长度*/
	fileLen = (unsigned int)((pBuf[4] <<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD))& 0xff) +\
			(unsigned int)((pBuf[5]<<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE))&0xff) + \
			(unsigned int)((pBuf[6]<<OAM_BITS_IN_BYTE)&0xff) + \
			(unsigned int)pBuf[7] ;	

	/*获取数据块的偏移量*/
	fileOff = (unsigned int)(pBuf[8] <<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD)) + \
			(unsigned int)(pBuf[9]<<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE)) + \
			(unsigned int)(pBuf[10]<<OAM_BITS_IN_BYTE) + \
			(unsigned int)pBuf[11] ;	

	/*获取数据块的长度*/
	dataLen = (unsigned int)(pBuf[12] <<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD)) + \
			(unsigned int)(pBuf[13]<<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE)) + \
			(unsigned int)(pBuf[14]<<OAM_BITS_IN_BYTE) + \
			(unsigned int)pBuf[15] ;	

	/*获取数据块的校验值*/
	datafcs = (unsigned int)(pBuf[16] <<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD)) + \
			(unsigned int)(pBuf[17]<<(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE)) + \
			(unsigned int)(pBuf[18]<<OAM_BITS_IN_BYTE) + \
			(unsigned int)pBuf[19] ;	
	
	memset(&gstrOamFileCtrl, 0, sizeof(GW_OAM_FILECTR_t));
	gstrOamFileCtrl.oltId = oltId;
	gstrOamFileCtrl.onuId = onuId;
	gstrOamFileCtrl.ackCode = ackCode;
	gstrOamFileCtrl.ErrCode = ErrCode;
	gstrOamFileCtrl.fileLen = fileLen;
	gstrOamFileCtrl.fileOff = fileOff;
	gstrOamFileCtrl.dataLen = dataLen;
	gstrOamFileCtrl.datafcs = datafcs;
	gbFileRev[oltId][onuId]  = FALSE;
	return OAM_OK;

/*
	if (0x302 == ackCode)
		{
			gbFileTrn = TRUE;
		}
 	else if (0x301 == ackCode)
 		{
 			gbFileWrtAllow = TRUE;
 		}

	gFileAckCode = ackCode;
	
	switch(ackCode)
		{
			case FILE_CTRL_ACK_RDREFUCTED:
				gbFileRdRefucted = FALSE;
				break;
			case FILE_CTRL_ACK_RDACCEPT:
				gbFileRdAllow = FALSE;
				break;
			case FILE_CTRL_ACK_WRTREFUCTED:
				gbFileWrtRefucted = FALSE;
				break;
			case FILE_CTRL_ACK_WRTACCEPT:
				gbFileWrtAllow = FALSE;
				break;
			case FILE_CTRL_ACK_TXERROR:
				gbFileTxErr = FALSE;
				break;
			case FILE_CTRL_ACK_TXSTART:
				gbFileTxStart = FALSE;
				break;
			case FILE_CTRL_ACK_TRANSMITTING:
				gbFileTransmitting = FALSE;
				break;
			case FILE_CTRL_ACK_TXOVER:
				gbFileTxOver = FALSE;
				break;
			default:
				return OAM_ERR;
			
		}*/

/*
#define FILE_CTRL_ERRCODE_OK				0x00
#define FILE_CTRL_ERRCODE_SYSBUSY			0x01
#define FILE_CTRL_ERRCODE_SYSNORSC			0x02
#define FILE_CTRL_ERRCODE_SYSERRO			0x03
#define FILE_CTRL_ERRCODE_FLOWERR			0x04
#define FILE_CTRL_ERRCODE_NOTFILE			0x05
#define FILE_CTRL_ERRCODE_LONGERR			0x06
#define FILE_CTRL_ERRCODE_SHORTERR			0x07
#define FILE_CTRL_ERRCODE_NOTMATCH			0x08
#define FILE_CTRL_ERRCODE_CHECKERR			0x09
#define FILE_CTRL_ERRCODE_SAVEERR			0x60
*/
	/*gFileErrCode  = ErrCode;
	switch(ErrCode)
		{
			case FILE_CTRL_ERRCODE_OK:
				break;
			case FILE_CTRL_ERRCODE_SYSBUSY:
				break;
			case FILE_CTRL_ERRCODE_SYSNORSC:
				break;
			case FILE_CTRL_ERRCODE_SYSERRO:
				break;
			case FILE_CTRL_ERRCODE_NOTFILE:
				break;
			case FILE_CTRL_ERRCODE_LONGERR:
				break;
			case FILE_CTRL_ERRCODE_SHORTERR:
				break;
			case FILE_CTRL_ERRCODE_NOTMATCH:
				break;
			case FILE_CTRL_ERRCODE_CHECKERR:
				break;
			case FILE_CTRL_ERRCODE_SAVEERR:
				break;
			default:
				return OAM_ERR;
		}*/
	
/*
#ifdef OAM_DEBUG
	pFile = (unsigned char *)g_malloc(fileLen);
#else
	pFile = (unsigned char *)VOS_Malloc(fileLen, (ULONG)MODULE_FILECRL);
#endif
*/
	
}




short int FileCtrlDataTransRspTickTimeRun(unsigned short OltId, unsigned short OnuId, GW_OAM_FILECTR_t *pstrFileCtrl)
{
	/*GW_OAM_FILECTR_t strFileCtrl;*/
	unsigned short fileTrnTickTimes = 0;
	unsigned int 	ackCode = 0;
	if (pstrFileCtrl == NULL)
	
	/*gstrOamFileCtrl.ackCode = ackCode;
	gstrOamFileCtrl.datafcs = datafcs;
	gstrOamFileCtrl.dataLen = dataLen;
	gstrOamFileCtrl.ErrCode = ErrCode;
	gstrOamFileCtrl.fileLen = fileLen;
	gstrOamFileCtrl.fileOff = fileOff;*/

	if (gbFileRev[OltId][OnuId] == TRUE) 
		return OAM_INVALUEBUF_ERR;
	
	gbFileRev[OltId][OnuId] = TRUE;
	/*推荐时间15s的1/2*/
	fileTrnTickTimes == (15* sysClkRateGet())/2/20;
	/*等待onu的回应,是否传输正确*/
	while ((gbFileRev[OltId][OnuId] ) && (fileTrnTickTimes == 0))
		{
			taskDelay(20);
			fileTrnTickTimes--;
		}
	if (gbFileRev[OltId][OnuId] )
		{
		memcpy(pstrFileCtrl, &gstrOamFileCtrl, sizeof(GW_OAM_FILECTR_t));
		gbFileRev[OltId][OnuId] = FALSE;
		return OAM_OK;
		}
	else
		{
		gbFileRev[OltId][OnuId] = FALSE;
		return OAM_TIME_OUT;	
		}

}
/***********************************************************
*
*
*
*
*
************************************************************/
short int CommFileTransferAck(	const unsigned short OltId, 
							const unsigned short OnuId,
							unsigned int ackCode,
							unsigned int ErrCode,
							unsigned int fileLen,
							unsigned int fileOff,
							unsigned int dataLen,
							unsigned int datafcs )
{
	int iRes = OAM_OK;
	unsigned char *pFileAck = NULL;
	GW_OAM_MSG_t  OamMsgStru;
	unsigned long lSendSerNo = 0;	
#ifdef OAM_DEBUG
	pFileAck = (unsigned char *)g_malloc(FILE_ACK_LENGTH);
#else
	pFileAck = (unsigned char *)VOS_Malloc((ULONG)FILE_ACK_LENGTH, (ULONG)MODULE_OAM);
#endif
	/*ackcode*/
	pFileAck[0] = (ackCode & 0xff00) >>OAM_BITS_IN_BYTE;
	pFileAck[1] = ackCode & 0xff;
	/*errcode*/
	pFileAck[2] = (ErrCode & 0xff00)>>OAM_BITS_IN_BYTE;
	pFileAck[3] = (ErrCode & 0xff);
	/*文件的长度*/
	pFileAck[4] = (fileLen&0xff000000)>>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD);
	pFileAck[5] = (fileLen&0x00ff0000)>>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE);
	pFileAck[6] = (fileLen&0x0000ff00)>>OAM_BITS_IN_BYTE;
	pFileAck[7] = (fileLen&0x000000ff);
	/*数据块的偏移量*/
	pFileAck[8] = (fileOff&0xff000000)>>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD);
	pFileAck[9] = (fileOff&0x00ff0000)>>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE);
	pFileAck[10] = (fileOff&0x0000ff00)>>OAM_BITS_IN_BYTE;
	pFileAck[11] = (fileOff&0x000000ff);
	/*数据块的长度*/
	pFileAck[12] = (dataLen&0xff000000)>>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD);
	pFileAck[13] = (dataLen&0x00ff0000)>>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE);
	pFileAck[14] = (dataLen&0x0000ff00)>>OAM_BITS_IN_BYTE;
	pFileAck[15] = (dataLen&0x000000ff);
	/*数据块的校验值*/
	pFileAck[16] = (datafcs&0xff000000)>>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE*OAM_BYTES_IN_WORD);
	pFileAck[17] = (datafcs&0x00ff0000)>>(OAM_BITS_IN_BYTE+OAM_BITS_IN_BYTE);
	pFileAck[18] = (datafcs&0x0000ff00)>>OAM_BITS_IN_BYTE;
	pFileAck[19] = (datafcs&0x000000ff);



	iRes = CommOamPadSendsernoGet  (&lSendSerNo);
	if (iRes != OAM_OK)
		return (OAM_INVALUE_SENDSENO_ERR);	
	
	memset(&OamMsgStru, 0, sizeof(GW_OAM_MSG_t));
	OamMsgStru.OltId = OltId;
	OamMsgStru.OnuId = OnuId;				
	OamMsgStru.brdcstLlid= 0;
	OamMsgStru.oamOpcode = GW_OPCODE_FILE_TRANSFER_ACK;
	OamMsgStru.sendSerNo = lSendSerNo;
	iRes = OamIODataSend(OltId, OnuId, pFileAck, GW_MSG_SEND, FILE_ACK_LENGTH, &OamMsgStru);	
#ifdef OAM_DEBUG
	free(pFileAck);
#else
	VOS_Free((VOID *) pFileAck);
#endif

	return iRes;
}


short int FileCtrlDataBufFree(unsigned short OltId, unsigned short OnuId)
{
#ifdef OAM_DEBUG	
	sys_console_printf(" Finishing to rev of files\r\n");
	free(pgFileRevBuf[OltId][OnuId]->pfileName);
	pgFileRevBuf[OltId][OnuId]->pfileName = NULL;
	free(pgFileRevBuf[OltId][OnuId]);
	pgFileRevBuf[OltId][OnuId] = NULL;
#else
	/*高层相应接口*/
	sys_console_printf(" Finishing to rev of files\r\n");
	VOS_Free((void *)pgFileRevBuf[OltId][OnuId]->pfileName);
	pgFileRevBuf[OltId][OnuId]->pfileName = NULL;
	VOS_Free((void *)pgFileRevBuf[OltId][OnuId]);
	pgFileRevBuf[OltId][OnuId] = NULL;
#endif
	return OAM_OK;
}

short int FileCtrlDataBufMalloc(
						unsigned short OltId, 
						unsigned short OnuId, 
						unsigned char *pfileName,  
						unsigned int fileNameLen, 
						unsigned int fileLens)
{
	unsigned char *pFileName = NULL;
	unsigned char *pfilebuf = NULL;
	GW_FILE_REVBUF_t *pFileCtrl = NULL;
#ifdef OAM_DEBUG
	/*保存该文件的名称以及名称的长度*/
	pFileCtrl = (GW_FILE_REVBUF_t *)g_malloc(sizeof(GW_FILE_REVBUF_t));
	if (NULL == pFileCtrl)
		return OAM_ERR;
	pFileName = (unsigned char *)g_malloc(fileNameLen);
	if (NULL == pFileName)
		{
		free(pFileCtrl);
		return OAM_ERR;
		}
#else
	pFileCtrl = (GW_FILE_REVBUF_t *)VOS_Malloc((ULONG)sizeof(GW_FILE_REVBUF_t), (ULONG)MODULE_OAM);
	if (NULL == pFileCtrl)
		return OAM_ERR;
	pFileName = (unsigned char *)VOS_Malloc((ULONG)fileNameLen, (ULONG)MODULE_OAM);
	if (NULL == pFileName)
		{
		VOS_Free((void *)pFileCtrl);
		return OAM_ERR;
		}
#endif	
	memset(pFileCtrl, 0, sizeof(GW_FILE_REVBUF_t));
	pgFileRevBuf[OltId][OnuId] = pFileCtrl;
	pgFileRevBuf[OltId][OnuId]->pfileName = pFileName ;
	pgFileRevBuf[OltId][OnuId]->fileNameLen = fileNameLen;
	memcpy(pgFileRevBuf[OltId][OnuId]->pfileName, pfileName, fileNameLen);
	pgFileRevBuf[OltId][OnuId]->fileLen = fileLens;


	/*目前为申请一定得空间的做法*/
#ifdef OAM_DEBUG
	pfilebuf = (unsigned char *)g_malloc(fileLens);
#else
	pfilebuf = (unsigned char *)VOS_Malloc((ULONG)fileLens, (ULONG)MODULE_OAM);
#endif

	if (pfilebuf == NULL)
		{
#ifdef OAM_DEBUG
			pfilebuf = (unsigned char *)g_malloc(fileLens);
#else
			pfilebuf = (unsigned char *)VOS_Malloc((ULONG)fileLens, (ULONG)MODULE_OAM);
#endif
			if (pfilebuf == NULL)
				{
					FileCtrlDataBufFree(OltId, OnuId);
					return OAM_ERR;
						
				}
		}
	pgFileRevBuf[OltId][OnuId]->pDataBlk = pfilebuf;
	
	return OAM_OK;
}

short int FileCtrlDataBufCheck(unsigned short OltId, unsigned short OnuId)	
{
	if (pgFileRevBuf[OltId][OnuId] == NULL)	
		return FILE_CTRL_ERRCODE_SYSERRO;
	return OAM_OK;
}


#endif

/*int  GwOamRecvBufTst( void )
{
	int Buf_index;

	sys_console_printf(" oam recv max buf = %d\r\n", gOamQueueCtl[1].MaxSize );

	for( Buf_index = 0; Buf_index < gOamQueueCtl[1].MaxSize; Buf_index++ )
		{
		if( gOamQueueCtl[1].DataBuff[Buf_index].IsUsed == TRUE )
			sys_console_printf(" buf index %d is used\r\n", Buf_index );
		}

	return( OAM_OK );
}*/

#if (RPU_MODULE_IGMP_TVM == RPU_YES )

STATUS Comm_Tvm_Frame_Send(ULONG PonId, ULONG OnuId,  char * buftvm,INT lengthbuf , INT flag)
{
	ULONG PonPortIdx=0;
	unsigned char bSessid[8];
	ULONG lSendSerNo = 0;
	INT  iRes=VOS_OK;

	if( (NULL == buftvm) || (lengthbuf > MAX_SIZE_OF_OAM_FRAME) )
		return VOS_ERROR;
	VOS_MemSet(bSessid,0,sizeof(bSessid));

	if(VOS_YES == glIgmpTvmDebug)
	{
		if(TVM_UNICAST == flag)
		{
			sys_console_printf("Comm_Tvm_Frame_Send:PonId=%d,OnuId=%d\r\n",PonId,OnuId);
		}
		sys_console_printf("length of buftemp is %d\r\n",lengthbuf);
		Igmp_Tvm_Debug_Head(buftvm);
	}

	if(TVM_BROADCAST == flag)
	{
		if( VOS_YES == glIgmpTvmDebug)
		{
			sys_console_printf("Comm_Tvm_Frame_Send:Broadcast !!\r\n");
		}
		
		for(PonPortIdx =0; PonPortIdx < MAXPON; PonPortIdx ++ )
		{
			if( PonPortIsWorking(PonPortIdx) == TRUE )
			{
				if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
					return OAM_ERR;	

				iRes = COMM_OAM_frame_send(PonPortIdx, 
								0,
								GW_OPCODE_TVM_REQUEST,
								OAM_PON_EVERY_LLID,
								buftvm, 
								lengthbuf, 
								bSessid,
								GW_DEVTYPE_OLT, 
								lSendSerNo);	
			}
		}
	}
	else if(TVM_UNICAST == flag)
	{
		if(VOS_YES == glIgmpTvmDebug)
		{
			sys_console_printf("Comm_Tvm_Frame_Send:Unicast !! \r\n");
		}
		OnuId--;
		if ( OAM_INVALID_LLID == GetLlidByOnuIdx( PonId, OnuId) )
		{
			return OAM_ONU_DEREGISTER_ERR;
		}

		if (OAM_OK != CommOamPadSendsernoGet(&lSendSerNo))
			return OAM_ERR;	
		
		iRes = COMM_OAM_frame_send(PonId, 
									OnuId,
									GW_OPCODE_TVM_REQUEST,
									OAM_SEND_SPECIFIED_LLID,
									buftvm, 
									lengthbuf, 
									bSessid,
									0, 
									lSendSerNo);	

	}
	if( iRes != OAM_OK)
	{
		sys_console_printf("  -------TVM OAM Send Err info -----\r\n");
		Comm_Oam_ErrParse(iRes);
	}
	return iRes;
}

#endif



