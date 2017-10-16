/**************************************************************************************
* OAM_gw.h - .定义了GW 的扩展OAM的相关数据结构定义，对外接口函数声明，
* 			宏定义,测试函数 
* Copyright (c) 2006 GW Technologies Co., LTD.
* All rights reserved.
* 
* create: 
* 2006-06-222  wutw
* 
* 文件描述：
*
*
* modified by chenfj 2008-7-1
*    增加vconsole 传输通道( oam 层)
*
*  modified by chenfj 2008-7-9
*      增加GFA6100 产品支持
* 
**************************************************************************************/

#ifndef __OAM_GW_H__
#define __OAM_GW_H__

#ifdef    __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "vos_typevx.h"
#include "VOS_types.h"


/*
GW OpCode				Code value	note

Reserved		0x00			
EQU information Request		0x01			设备信息请求
EQU information Respone	0x02			设备信息应答
		
ALARM Transmit			0x03			告警上报透传
ALARM reserved			0x04			告警备用
		
FILE Read/Write Request  	0x05			文件读写请求
FILE Reserved 				0x06			文件操作保留
FILE data transfer  			0x07			文件传送
FILE transfer ACK  			0x08			文件传送应答
FILE Reserved 				0x09-0x0a	为文件传送保留
		
SNMP Request Transmit		0x0b	Snmp 	请求透明传送
SNMP Reponse Transmit		0x0c	Snmp 	请求应答透明传送
SNMP Trap Transmit			0x0d	Snmp 	TRAP透明传送
Reserved For SNMP			0x0e-0x0f	为SNMP传送备用
		
CLI Request Transmit		0x10	CLI 		请求透明传送
CLI Reponse Transmit		0x11	CLI 		请求应答透明传送
		
IGMP Auth Request Transmit	0x12	IGMP 	组播认证请求透明传送
IGMP Auth Reponse Transmit	0x13	IGMP 	组播认证请求应答透明传送
		
Reserved	0x10-0xff	备用
*/
typedef enum{
	GW_OPCODE_RESERVED				=	0x0,
	GW_OPCODE_EUQ_INFO_REQUESET		=	0x01,
	GW_OPCODE_EUQ_INFO_RESPONSE		=	0x02,
	GW_OPCODE_ALARM_OR_LOG_TRAN		=	0x03,
	GW_OPCODE_ALARM_OR_LOG_RESPONSE	=	0x04,
	GW_OPCODE_FILE_READ_AND_WRITE_REQUEST =	0x05,
	GW_OPCODE_FILE_RESERVED_0		=	0x06,
	GW_OPCODE_FILE_DATA_TRAN		=	0x07,
	GW_OPCODE_FILE_TRANSFER_ACK		=	0x08,
	GW_OPCODE_FILE_RESERVED_1		=	0x09/**/,
	GW_OPCODE_FILE_RESERVED_2		=	0x0a/**/,
	GW_OPCODE_SNMP_REQUEST		=	0x0b,
	GW_OPCODE_SNMP_RESPONSE		=	0x0c,
	GW_OPCODE_SNMP_TRAP				=	0x0d,
	GW_OPCODE_SNMP_RESERVED_1	=	0x0e,
	GW_OPCODE_SNMP_RESERVED_2	=	0x0f,
	GW_OPCODE_CLI_REQUEEST			=	0x10,
	GW_OPCODE_CLI_RESPONSE			=	0x11,
	GW_OPCODE_IGMPAUTH_REQUEST		=	0x12,
	Gw_OPCODE_IGMPAUTH_REPONSE		=	0x13,
	GW_OPCODE_VCONSOLE			=	0x14,
	Gw_OPCODE_OAM_DEBUG			=	0x15,
	GW_OPCODE_TVM_REQUEST			=	0x16,
	GW_OPCODE_TVM_RESPONSE			=	0x17,
	
	GW_OPCODE_LAST				
}GW_OPCODE_types;

typedef enum {
	GW_CALLBACK_RESERVED=0x0,
	GW_CALLBACK_EQU_REQUEST=0x01,
	GW_CALLBACK_EUQINFO=0x02,/*GW_CALLBACK_EQU_RESPONE*/
	GW_CALLBACK_ALARMORLOG=0x03,/*request*/
	GW_CALLBACK_ALARMORLOG_REPONSE=0x04,
	GW_CALLBACK_FILE_RD_WRT_REQUEST=0x05,
	GW_CALLBACK_FILE_RESERVED=0x06,
	GW_CALLBACK_FILE_DATA_TRA=0x07,
	GW_CALLBACK_FILE_TRA_ACK=8,
	GW_CALLBACK_RESERVED_1=9,
	GW_CALLBACK_RESERVED_2=10,
	GW_CALLBACK_SNMP_REQUEST_1=0xb,
	GW_CALLBACK_SNMP_REPONSE_1=0xc,
	GW_CALLBACK_SNMP_TRAP_1=0x0d,
	GW_CALLBACK_RESERVED_3=0xe,
	GW_CALLBACK_RESERVED_4=0xf, 
	GW_CALLBACK_CLI_REQUEST_1=0x10,
	GW_CALLBACK_CLI_1	=0x11,/*cli reponse*/
	GW_CALLBACK_IGMPAUTH_REQUEST=0x12,
	GW_CALLBACK_IGMPAUTH_REPONST=0x13,
	GW_CALLBACK_VCONSOLE=0x14,
	GW_CALLBACK_OAM_DEBUG= 0x15,
	GW_CALLBACK_TVM_REQUEST=0x16,
	GW_CALLBACK_TVM_RESPONSE=0x17,

	GW_CALLBACK_LAST
}GW_CALLBACK_types;

/*for debug funtion define*/
typedef enum {
	GW_DEBUG_RESERVED = 0x0,
	GW_DEBUG_CLI  = 0x1,
	GW_DEBUG_SNMP = 0x2,
	GW_DEBUG_IGMPAUTH,
	GW_DEBUG_EUQINFO,
	GW_DEBUG_ALARMORLOG,
	GW_DEBUG_FILE,
	GW_DEBUG_SNMP_TRAP,
	GW_DEBUG_VCONSOLE,
	
	GW_DEBUG_ALL
}GW_DEBUG_types;

/* B--added by liwei056@2009-3-5 for BroadCast-Cli */
#if 0
extern char* g_paBroadCastPonIDs;
extern int   g_iBroadCastFlags;

#define ASSERT_PONLIST()       VOS_ASSERT(SYS_MAX_PON_PORTNUM >= MAXPON)
#define PONLIST_BYTENUM        ((MAXPON >> 3) + 1)
#define BROADCAST_PONLIST_ISEMPTY()       (0 == g_paBroadCastPonIDs[-1])
#define BROADCAST_PONLIST_ISFULL()        (MAXPON == g_paBroadCastPonIDs[-1])
#define BROADCAST_PONLIST_GETNUM()        g_paBroadCastPonIDs[-1]

#define BROADCAST_PONLIST_CLRALL()      \
    VOS_MemZero(g_paBroadCastPonIDs - 1, PONLIST_BYTENUM + 1)
    
#define BROADCAST_PONLIST_SETALL()        \
    VOS_MemSet(g_paBroadCastPonIDs, 0xFF, PONLIST_BYTENUM); \
    g_paBroadCastPonIDs[-1] = MAXPON
    
#define BROADCAST_PONLIST_GETPON(iPonID) ( g_paBroadCastPonIDs[(iPonID) >> 3] & (1 << ((iPonID) & 0x7)) )
#define BROADCAST_PONLIST_SETPON(iPonID)  \
do \
{ \
    if (0 == BROADCAST_PONLIST_GETPON(iPonID)) \
    { \
        g_paBroadCastPonIDs[(iPonID) >> 3] |= (1 << ((iPonID) & 0x7)); \
        ++g_paBroadCastPonIDs[-1]; \
    } \
}while(0)

#define BROADCAST_PONLIST_CLRPON(iPonID)  \
do \
{ \
    if (0 != BROADCAST_PONLIST_GETPON(iPonID)) \
    { \
        g_paBroadCastPonIDs[(iPonID) >> 3] &= (~(1 << ((iPonID) & 0x7))); \
        --g_paBroadCastPonIDs[-1]; \
    } \
}while(0)

#define BROADCAST_FLAGS_IS_SAFE()       (0 == g_iBroadCastFlags) 
#define BROADCAST_FLAGS_SET_SAFE(bVal)  g_iBroadCastFlags = (bVal) ? 0 : 1
#endif
/* E--added by liwei056@2009-3-5 for BroadCast-Cli */


/*=======对外接口函数=========*/
/*oam 模块处理注册函数*/
extern int  CommOltMsgRvcCallbackInit(unsigned char ModuleProId, void *Function);

/*=====================CLI接口定义=========================*/
/***********************************************************
* Comm_Cli_request_transmit
*
* description : 该函数用于发送cli命令至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
* parameter : 
*			PonId -值: 0 - 19 ，对应ponid 调用该接口函数前，可通过调用函数
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) 获取得到ponid 
*			OnuId - 值: 1  - 64, 对应每个ponid下的onuid ，不必再进行转换；
*			pClibuf - 指向cli 命令的有效空间，该指针由上层释放。
*			cli_data_size	- 该cli 命令的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
*
************************************************************/
#include "../superset/platform/include/man/cli/cli.h"
extern int  Comm_Cli_request_transmit(
                struct vty * vty,
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char  *pClibuf,
				const unsigned short cli_data_size,
				unsigned char *psessionIdField);


/*=====================snmp接口定义=========================*/
/***********************************************************
* Comm_snmp_request_transmit
*
* description : 该函数用于发送snmp请求消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
*			PonId -值: 0 - 19 ，对应ponid 调用该接口函数前，可通过调用函数
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) 获取得到ponid 
*			OnuId - 值: 1  - 64, 对应每个ponid下的onuid ，不必再进行转换?
*			pSnmpbuf - 指向消息内容的有效空间，该指针由上层释放。
*			Snmp_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
*
*
************************************************************/
extern int Comm_snmp_request_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				void *pSnmpbuf,
				const unsigned short Snmp_data_size,
				unsigned char *psessionIdField);

/*=====================告警和日志传输接口定义=========================*/

/******************************************************
* Comm_IGMPAuth_Reponse
*
* description : 该函数用于发送igmp 认证消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
* parameter : 
*			PonId -值: 0 - 19 ，对应ponid 调用该接口函数前，可通过调用函数
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) 获取得到ponid 
*			OnuId - 值: 1  - 64, 对应每个ponid下的onuid ，不必再进行转换；
*			pIgmpAuth - 指向igmp认证消息内容的有效空间，该指针由上层释放。
*			igmp_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
*
********************************************************/
extern int Comm_IGMPAuth_Reponse(const unsigned short PonId, 
								const unsigned short OnuId,
								unsigned char *pIgmpAuth,
								const unsigned short igmp_data_size,
								unsigned char *psessionIdField);

/******************************************************
* Comm_IGMPAuth_Reponse
*
* description : 该函数用于发送igmp 认证消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
* parameter : 
*			PonId -值: 0 - 19 ，对应ponid 调用该接口函数前，可通过调用函数
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) 获取得到ponid 
*			OnuId - 值: 1  - 64, 对应每个ponid下的onuid ，不必再进行转换；
*			pIgmpAuth - 指向igmp认证消息内容的有效空间，该指针由上层释放。
*			igmp_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
*
********************************************************/
extern int Comm_IGMPAuth_Requst(const unsigned short PonId, 
								const unsigned short OnuId,
								unsigned char *pIgmpAuth,
								const unsigned short igmp_data_size,
								unsigned char *psessionIdField);

/*=====================设备信息传输接口定义=========================*/
/***********************************************************
* Comm_EUQ_info_request_transmit
*
* description : 该函数用于发送EUQ_info_request消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
*			PonId -值: 0 - 19 ，对应ponid 调用该接口函数前，可通过调用函数
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) 获取得到ponid 
*			OnuId - 值: 1  - 64, 对应每个ponid下的onuid ，不必再进行转换?
*			pEuqinfobuf - 指向消息内容的有效空间，该指针由上层释放。
*			Euqinfo_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
*
*
************************************************************/
extern int Comm_EUQ_info_request_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char *pEuqinfobuf,
				const unsigned short Euqinfo_data_size,
				unsigned char *psessionIdField);


/********************************************************
* Comm_Alarm_Response_transmit
* description : 该函数用于发送消息至onu。调用该函数所传递的指针所占用的空间，
*			由调用者释放。该函数返回0 时为正确，其他值则为错误。
*			PonId -值: 0 - 19 ，对应ponid 调用该接口函数前，可通过调用函数
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) 获取得到ponid 
*			OnuId - 值: 1  - 64, 对应每个ponid下的onuid ，不必再进行转换?
*			p**buf - 指向消息内容的有效空间，该指针由上层释放。
*			**_data_size	- 该消息的有效长度
*			psessioinIdField - 指向sessionId 的有效空间，8个字节，该指针由上层释放。
*  return	     : 当为0 时，为正确，否则为错误
**********************************************************/
extern  int Comm_Alarm_Response_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char *pAlarmbuf,
				const unsigned short Alarm_data_size,
				unsigned char *psessionIdField);


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
extern  short int FileDataTransmit(
									const unsigned short PonId, 
									const unsigned short OnuId,
									unsigned char *fileBuf,
									unsigned int fileBufLen,
									unsigned char *psessionIdField
									);

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
extern  short int FileAckTransmit(
									const unsigned short PonId, 
									const unsigned short OnuId,
									unsigned char *fileBuf,
									unsigned int fileBufLen,
									unsigned char *psessionIdField
									);



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
extern  short int FileRequestTransmit(
									const unsigned short PonId, 
									const unsigned short OnuId,
									unsigned char *fileBuf,
									unsigned int fileBufLen,
									unsigned char *psessionIdField
									);


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
extern int Comm_vconsole_info_request_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char *pEuqinfobuf,
				const unsigned short Euqinfo_data_size,
				unsigned char *psessionIdField);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#if (RPU_MODULE_IGMP_TVM == RPU_YES )
enum {
	TVM_BROADCAST = 1,
	TVM_UNICAST
};
STATUS Comm_Tvm_Frame_Send(ULONG PonId, ULONG OnuId,  char * buftvm,INT lengthbuf , INT flag);
#endif

#endif /* __OAM_GW_H__ */
