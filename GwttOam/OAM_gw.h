/**************************************************************************************
* OAM_gw.h - .������GW ����չOAM��������ݽṹ���壬����ӿں���������
* 			�궨��,���Ժ��� 
* Copyright (c) 2006 GW Technologies Co., LTD.
* All rights reserved.
* 
* create: 
* 2006-06-222  wutw
* 
* �ļ�������
*
*
* modified by chenfj 2008-7-1
*    ����vconsole ����ͨ��( oam ��)
*
*  modified by chenfj 2008-7-9
*      ����GFA6100 ��Ʒ֧��
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
EQU information Request		0x01			�豸��Ϣ����
EQU information Respone	0x02			�豸��ϢӦ��
		
ALARM Transmit			0x03			�澯�ϱ�͸��
ALARM reserved			0x04			�澯����
		
FILE Read/Write Request  	0x05			�ļ���д����
FILE Reserved 				0x06			�ļ���������
FILE data transfer  			0x07			�ļ�����
FILE transfer ACK  			0x08			�ļ�����Ӧ��
FILE Reserved 				0x09-0x0a	Ϊ�ļ����ͱ���
		
SNMP Request Transmit		0x0b	Snmp 	����͸������
SNMP Reponse Transmit		0x0c	Snmp 	����Ӧ��͸������
SNMP Trap Transmit			0x0d	Snmp 	TRAP͸������
Reserved For SNMP			0x0e-0x0f	ΪSNMP���ͱ���
		
CLI Request Transmit		0x10	CLI 		����͸������
CLI Reponse Transmit		0x11	CLI 		����Ӧ��͸������
		
IGMP Auth Request Transmit	0x12	IGMP 	�鲥��֤����͸������
IGMP Auth Reponse Transmit	0x13	IGMP 	�鲥��֤����Ӧ��͸������
		
Reserved	0x10-0xff	����
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


/*=======����ӿں���=========*/
/*oam ģ�鴦��ע�ắ��*/
extern int  CommOltMsgRvcCallbackInit(unsigned char ModuleProId, void *Function);

/*=====================CLI�ӿڶ���=========================*/
/***********************************************************
* Comm_Cli_request_transmit
*
* description : �ú������ڷ���cli������onu�����øú��������ݵ�ָ����ռ�õĿռ䣬
*			�ɵ������ͷš��ú�������0 ʱΪ��ȷ������ֵ��Ϊ����
* parameter : 
*			PonId -ֵ: 0 - 19 ����Ӧponid ���øýӿں���ǰ����ͨ�����ú���
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) ��ȡ�õ�ponid 
*			OnuId - ֵ: 1  - 64, ��Ӧÿ��ponid�µ�onuid �������ٽ���ת����
*			pClibuf - ָ��cli �������Ч�ռ䣬��ָ�����ϲ��ͷš�
*			cli_data_size	- ��cli �������Ч����
*			psessioinIdField - ָ��sessionId ����Ч�ռ䣬8���ֽڣ���ָ�����ϲ��ͷš�
*  return	     : ��Ϊ0 ʱ��Ϊ��ȷ������Ϊ����
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


/*=====================snmp�ӿڶ���=========================*/
/***********************************************************
* Comm_snmp_request_transmit
*
* description : �ú������ڷ���snmp������Ϣ��onu�����øú��������ݵ�ָ����ռ�õĿռ䣬
*			�ɵ������ͷš��ú�������0 ʱΪ��ȷ������ֵ��Ϊ����
*			PonId -ֵ: 0 - 19 ����Ӧponid ���øýӿں���ǰ����ͨ�����ú���
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) ��ȡ�õ�ponid 
*			OnuId - ֵ: 1  - 64, ��Ӧÿ��ponid�µ�onuid �������ٽ���ת��?
*			pSnmpbuf - ָ����Ϣ���ݵ���Ч�ռ䣬��ָ�����ϲ��ͷš�
*			Snmp_data_size	- ����Ϣ����Ч����
*			psessioinIdField - ָ��sessionId ����Ч�ռ䣬8���ֽڣ���ָ�����ϲ��ͷš�
*  return	     : ��Ϊ0 ʱ��Ϊ��ȷ������Ϊ����
*
*
************************************************************/
extern int Comm_snmp_request_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				void *pSnmpbuf,
				const unsigned short Snmp_data_size,
				unsigned char *psessionIdField);

/*=====================�澯����־����ӿڶ���=========================*/

/******************************************************
* Comm_IGMPAuth_Reponse
*
* description : �ú������ڷ���igmp ��֤��Ϣ��onu�����øú��������ݵ�ָ����ռ�õĿռ䣬
*			�ɵ������ͷš��ú�������0 ʱΪ��ȷ������ֵ��Ϊ����
* parameter : 
*			PonId -ֵ: 0 - 19 ����Ӧponid ���øýӿں���ǰ����ͨ�����ú���
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) ��ȡ�õ�ponid 
*			OnuId - ֵ: 1  - 64, ��Ӧÿ��ponid�µ�onuid �������ٽ���ת����
*			pIgmpAuth - ָ��igmp��֤��Ϣ���ݵ���Ч�ռ䣬��ָ�����ϲ��ͷš�
*			igmp_data_size	- ����Ϣ����Ч����
*			psessioinIdField - ָ��sessionId ����Ч�ռ䣬8���ֽڣ���ָ�����ϲ��ͷš�
*  return	     : ��Ϊ0 ʱ��Ϊ��ȷ������Ϊ����
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
* description : �ú������ڷ���igmp ��֤��Ϣ��onu�����øú��������ݵ�ָ����ռ�õĿռ䣬
*			�ɵ������ͷš��ú�������0 ʱΪ��ȷ������ֵ��Ϊ����
* parameter : 
*			PonId -ֵ: 0 - 19 ����Ӧponid ���øýӿں���ǰ����ͨ�����ú���
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) ��ȡ�õ�ponid 
*			OnuId - ֵ: 1  - 64, ��Ӧÿ��ponid�µ�onuid �������ٽ���ת����
*			pIgmpAuth - ָ��igmp��֤��Ϣ���ݵ���Ч�ռ䣬��ָ�����ϲ��ͷš�
*			igmp_data_size	- ����Ϣ����Ч����
*			psessioinIdField - ָ��sessionId ����Ч�ռ䣬8���ֽڣ���ָ�����ϲ��ͷš�
*  return	     : ��Ϊ0 ʱ��Ϊ��ȷ������Ϊ����
*
********************************************************/
extern int Comm_IGMPAuth_Requst(const unsigned short PonId, 
								const unsigned short OnuId,
								unsigned char *pIgmpAuth,
								const unsigned short igmp_data_size,
								unsigned char *psessionIdField);

/*=====================�豸��Ϣ����ӿڶ���=========================*/
/***********************************************************
* Comm_EUQ_info_request_transmit
*
* description : �ú������ڷ���EUQ_info_request��Ϣ��onu�����øú��������ݵ�ָ����ռ�õĿռ䣬
*			�ɵ������ͷš��ú�������0 ʱΪ��ȷ������ֵ��Ϊ����
*			PonId -ֵ: 0 - 19 ����Ӧponid ���øýӿں���ǰ����ͨ�����ú���
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) ��ȡ�õ�ponid 
*			OnuId - ֵ: 1  - 64, ��Ӧÿ��ponid�µ�onuid �������ٽ���ת��?
*			pEuqinfobuf - ָ����Ϣ���ݵ���Ч�ռ䣬��ָ�����ϲ��ͷš�
*			Euqinfo_data_size	- ����Ϣ����Ч����
*			psessioinIdField - ָ��sessionId ����Ч�ռ䣬8���ֽڣ���ָ�����ϲ��ͷš�
*  return	     : ��Ϊ0 ʱ��Ϊ��ȷ������Ϊ����
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
* description : �ú������ڷ�����Ϣ��onu�����øú��������ݵ�ָ����ռ�õĿռ䣬
*			�ɵ������ͷš��ú�������0 ʱΪ��ȷ������ֵ��Ϊ����
*			PonId -ֵ: 0 - 19 ����Ӧponid ���øýӿں���ǰ����ͨ�����ú���
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) ��ȡ�õ�ponid 
*			OnuId - ֵ: 1  - 64, ��Ӧÿ��ponid�µ�onuid �������ٽ���ת��?
*			p**buf - ָ����Ϣ���ݵ���Ч�ռ䣬��ָ�����ϲ��ͷš�
*			**_data_size	- ����Ϣ����Ч����
*			psessioinIdField - ָ��sessionId ����Ч�ռ䣬8���ֽڣ���ָ�����ϲ��ͷš�
*  return	     : ��Ϊ0 ʱ��Ϊ��ȷ������Ϊ����
**********************************************************/
extern  int Comm_Alarm_Response_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char *pAlarmbuf,
				const unsigned short Alarm_data_size,
				unsigned char *psessionIdField);


/***********************************************************
* FileDataTransmit
* ����: �ú������ڷ��ͻ�Ӧ
* description : �ú������ڷ�����Ϣ��onu�����øú��������ݵ�ָ����ռ�õĿռ䣬
*			�ɵ������ͷš��ú�������0 ʱΪ��ȷ������ֵ��Ϊ����
*			PonId -ֵ: 0 - 19 ����Ӧponid ���øýӿں���ǰ����ͨ�����ú���
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) ��ȡ�õ�ponid 
*			OnuId - ֵ: 1  - 64, ��Ӧÿ��ponid�µ�onuid �������ٽ���ת��?
*			p**buf - ָ����Ϣ���ݵ���Ч�ռ䣬��ָ�����ϲ��ͷš�
*			**_data_size	- ����Ϣ����Ч����
*			psessioinIdField - ָ��sessionId ����Ч�ռ䣬8���ֽڣ���ָ�����ϲ��ͷš�
*  return	     : ��Ϊ0 ʱ��Ϊ��ȷ������Ϊ����
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
* ����: �ú������ڷ��ͻ�Ӧ
* description : �ú������ڷ�����Ϣ��onu�����øú��������ݵ�ָ����ռ�õĿռ䣬
*			�ɵ������ͷš��ú�������0 ʱΪ��ȷ������ֵ��Ϊ����
*			PonId -ֵ: 0 - 19 ����Ӧponid ���øýӿں���ǰ����ͨ�����ú���
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) ��ȡ�õ�ponid 
*			OnuId - ֵ: 1  - 64, ��Ӧÿ��ponid�µ�onuid �������ٽ���ת��?
*			p**buf - ָ����Ϣ���ݵ���Ч�ռ䣬��ָ�����ϲ��ͷš�
*			**_data_size	- ����Ϣ����Ч����
*			psessioinIdField - ָ��sessionId ����Ч�ռ䣬8���ֽڣ���ָ�����ϲ��ͷš�
*  return	     : ��Ϊ0 ʱ��Ϊ��ȷ������Ϊ����
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
* description : �ú������ڷ�����Ϣ��onu�����øú��������ݵ�ָ����ռ�õĿռ䣬
*			�ɵ������ͷš��ú�������0 ʱΪ��ȷ������ֵ��Ϊ����
*			PonId -ֵ: 0 - 19 ����Ӧponid ���øýӿں���ǰ����ͨ�����ú���
*					GetPonPortIdxBySlot( (short int)slotId, (short  int)port ) ��ȡ�õ�ponid 
*			OnuId - ֵ: 1  - 64, ��Ӧÿ��ponid�µ�onuid �������ٽ���ת��?
*			p**buf - ָ����Ϣ���ݵ���Ч�ռ䣬��ָ�����ϲ��ͷš�
*			**_data_size	- ����Ϣ����Ч����
*			psessioinIdField - ָ��sessionId ����Ч�ռ䣬8���ֽڣ���ָ�����ϲ��ͷš�
*  return	     : ��Ϊ0 ʱ��Ϊ��ȷ������Ϊ����
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
* description : �ú������ڷ���vconsole_info_request��Ϣ��onu�����øú��������ݵ�ָ����ռ�õĿռ䣬
*			�ɵ������ͷš��ú�������0 ʱΪ��ȷ������ֵ��Ϊ����
* parameter :  
*			PonId -	0~19 
*			OnuId -  1~64
*			pEuqinfobuf - ָ����Ϣ���ݵ���Ч�ռ䣬��ָ�����ϲ��ͷš�
*			Euqinfo_data_size	- ����Ϣ����Ч����
*			psessioinIdField - ָ��sessionId ����Ч�ռ䣬8���ֽڣ���ָ�����ϲ��ͷš�
*  return	     : ��Ϊ0 ʱ��Ϊ��ȷ������Ϊ����
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
