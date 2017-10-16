#ifndef _DRPENG_OAM_STACK_H
#define _DRPENG_OAM_STACK_H

#define OAM_OUI_SIZE 3
#define EXTEND_OPCODE_LEN	1
#define EXTEND_BRANCH_LEAF_LEN	3
#define VARIABLE_WIDTH_LEN  1
#define PORT_TLV_LEN 8
#define OAM_BITS_IN_BYTE    8
#define OAM_BYTES_IN_WORD   2
#define OAM_BYTES_EXTEND_OPCODE   22
#define OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR		0xc7	/* Branch value of the CTC Extended Variable Attributes */
#define CTC_2_1_MANAGEMENT_OBJECT_BRANCH		0x37	/* Branch value of the port TVL */
#define GET_SUCCESS 1 
#define GET_FAILED 2 
#define NOTFOUND 2
#define ONU_MAC_LEN 6
#define TLV_NUMBER_MAC_ADDRESS_ENTRY 24
#define ONE_TLV_MAX_LENGTH 250

#define DRPENG_EX_VAR_ONU_PORT_LOCATION_BY_MAC               0xc001 /* Onu ethernet Port Location By MAC*/
#define DRPENG_EX_VAR_ONU_PORT_MAC_NUMBER               0xc002 /* Onu  Port MAC address number*/
#define DRPENG_EX_VAR_ONU_MAC_TABLE                                   0xc003 /* Onu   MAC address table*/
#define DRPENG_EX_VAR_ONU_DEVICE_NAME                                  0xc004 /* Onu   device name*/
#define DRPENG_EX_VAR_ONU_DEVICE_DESCRIPTION                                  0xc005 /* Onu   device Description*/
#define DRPENG_EX_VAR_ONU_DEVICE_LOCATION                                  0xc006 /* Onu   device location*/
#define DRPENG_EX_VAR_ONU_PORT_ISOLATION                                  0xc007 /* Onu   port isolation*/
#define DRPENG_EX_VAR_ONU_PORT_STORM                                  0xc008 /* Onu   port storm status*/
#define DRPENG_EX_VAR_ONU_PORT_MODE                                  0xc009 /* Onu   port mode*/
#define DRPENG_EX_VAR_ONU_LOOP_DETECTION_TIME                                  0xc00a/*Onu   loop detection time */
#define DRPENG_EX_VAR_ONU_SAVE_CONFIG                                 0xc00b/* Onu  port Save the configuration*/
#define DRPENG_EX_VAR_ONU_SUPPORT_ATTRIBUTE               0xc101 /* ONU support attribute */

typedef unsigned char mac_address [6];

typedef enum{
	DRPENGONU_OPCODE_RESERVED				=	0x0,
	DRPENGONU_OPCODE_OAM_EXT_GET_REQUEST		=	0x01,
	DRPENGONU_OPCODE_OAM_EXT_GET_RESPONSE	=	0x02,
	DRPENGONU_OPCODE_OAM_EXT_SET_REQUEST		=	0x03,
	DRPENGONU_OPCODE_OAM_EXT_SET_RESPONSE	=	0x04,
	DRPENGONU_OPCODE_OAM_EXT_ONU_AUTHENTICATION = 0x05,
	
	DRPENGONU_OPCODE_LAST				
}DRPENG_OPCODE_types;

#define CHECK_COMPARSER_RESULT( comparser_result) \
{   if ( comparser_result != 0 )\
{ \
    return comparser_result;\
}\
}
#define CHECK_BUFFER_SIZE(  offset, total_length)\
{\
    if(offset  > total_length)\
    {\
        return OLT_ERR_PARAM;\
    }\
}

extern int DrPeng_Oam_Session_Send(
    short int PonPortIdx, 
    short int OnuIdx, 
    char  MsgType, /*消息类型*/
    UCHAR mode, /*同步还是异步:OAM_SYNC/OAM_ASNYC*/
    ULONG ulFlag, 
    ULONG QueId, /*消息队列id*/
    OAMRECIEVECALLBACK func, /*回调函数接口*/
    unsigned char *pSendDataBuf,
    short int pSendDataBuflen,
    unsigned char *pRecieveBuf,
    short int *pRecieveBuflen
    );

#endif
