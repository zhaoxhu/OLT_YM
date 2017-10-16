/***************************************************************
*
*						Module Name:  PonHandler.c
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
*   Date: 			2006/04/27
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |--------------|------------------ 
**  06/04/27  |   chenfj           |     create 
**----------|--------------|------------------
** modification history:
**
** 1  modify by chenfj 2006/09/21
**  #2606 问题在手工添加64个ONU后，再实际注册新ONU时，出现异常
** 2  modify by chenfj 2006/09/22  
**  #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册
**  #2619  问题自动注册限制使能后，手动注册ONU时，出现异常
** 3  modify by chenfj 2006/09/30
**   #2624 问题ONU注册或离线时，控制台上没有提示了。
** 4 add by chenfj 2006/10/11
**    注册冲突ONU处理:
**    a, 当用于制造测试时，不检测冲突；
**    b, 当用于正常运行时，检测冲突；当发现冲突时，发送Trap信息；并
**        Pending这个ONU；当检测到发生冲突的ONU被删除时，则激活这个ONU
** 5 modified by chenfj 2006/10/30 
**    onu带宽分配：
**      ONU注册时，若已分配带宽，使用已分配的带宽
**       反之，使用默认带宽
**    onu被删除时，收回已分配的带宽。对未注册过或未配置或被删除的ONU，不分配带宽
** 6 added by chenfj 2006/12/06
**	  set onu peer-to-peer 在ONU注册时，设置ONU  peer-to-peer 通信
** 7 added by chenfj 2006-12-21 
**	  设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参
** 8 modified by chenfj 2007/03/16 
**		#3806问题单:PON口下插上第65个ONU时，串口打印大量信息，显示速度太快
**	9 modified by chenfj 2008-7-9
**         增加GFA6100 产品支持	
**		
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#include  "ponEventHandler.h"
#include "Cdp_pub.h"
#include "ifm_pub.h"
#include "ifm_def.h"
#include "ifm_aux.h"
#include "ifm_ntfy.h"
#include "ifm_netd.h"
#include "eth_type.h"
#include "onu\ExtBoardType.h"
#include "sys\devsm\devsm_hot.h"
#include "sys\devsm\devsm_switchhover.h"

#define PON_ALARM_TIME_GAP  10  /* unit: second */

int  EVENT_DEBUG = V2R1_DISABLE;
int  EVENT_ENCRYPT = V2R1_DISABLE;
int  EVENT_REGISTER = V2R1_DISABLE;
int  EVENT_ALARM = V2R1_DISABLE;
int  EVENT_RESET = V2R1_DISABLE;
int  EVENT_PONADD = V2R1_DISABLE;
int  EVENT_ASSIGN = V2R1_DISABLE;
int  EVENT_OAM_DEBUG = V2R1_DISABLE;
/*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
int  EVENT_OAM_SEND_DEBUG = V2R1_DISABLE;
int  EVENT_UPDATE_ONU_FILE = V2R1_DISABLE;
int  MAKEING_TEST_FLAG = V2R1_ENABLE;
int  downlinkBWlimit = V2R1_DISABLE;

/* B--added by liwei056@2010-5-26 for LLID-DownLineRate BUG */
int  downlinkBWlimitBurstSize  = ONU_DOWNLINK_POLICE_BURSESIZE_DEFAULT;
int  downlinkBWlimitPreference = DISABLE;
int  downlinkBWlimitDefault = V2R1_DISABLE;
/* E--added by liwei056@2010-5-26 for LLID-DownLineRate BUG */

/* B--added by liwei056@2010-12-20 for P2P-64KLineRate-1G BUG */
int  uplinkBWPacketUnitSize = ONU_UPLINK_DBA_PACKETSIZE_DEFAULT;
int  uplinkBWlimitBurstSize = ONU_UPLINK_POLICE_BURSESIZE_DEFAULT;
/* E--added by liwei056@2010-12-20 for P2P-64KLineRate-1G BUG */
/* B--added by liwei056@2013-8-2 for TK-BW-Fix */
int  uplinkBWWeight = ONU_UPLINK_DBA_WEIGHT_DEFAULT;
int  downlinkBWWeight = ONU_DOWNLINK_POLICE_WEIGHT_DEFAULT;
/* E--added by liwei056@2013-8-2 for TK-BW-Fix */

/* B--added by wangjiah@2017-04-14 for ONU GT524_B */
int DBA_POLLING_LEVEL = POLLING_LEVEL_DEFAULT;
/* E--added by wangjiah@2017-04-14 for ONU GT524_B */

int  EVENT_TRAP = V2R1_ENABLE;
int  EVENT_EUQ = V2R1_DISABLE;
int  EVENT_TESTPONPORT = V2R1_DISABLE;

int  MAKEING_TEST_FLAG_DEFAULT = V2R1_ENABLE;
/* B--added by liwei056@2010-6-1 for 6900-Prj */
int  EVENT_RPC  = V2R1_DISABLE;
int  EVENT_NULL = V2R1_DISABLE;
int  EVENT_GW   = V2R1_DISABLE;
int  EVENT_PAS  = V2R1_DISABLE;
int  EVENT_TK   = V2R1_DISABLE;
int  EVENT_BCM  = V2R1_DISABLE;
#if defined(_GPON_BCM_SUPPORT_)
int  EVENT_GPON  = V2R1_DISABLE;
int  EVENT_GPON_UPDATE = V2R1_DISABLE;
#endif
int  EVENT_SYN  = V2R1_DISABLE;
int  EVENT_PONSWITCH = V2R1_DISABLE;
int  EVENT_REMOTE = V2R1_DISABLE;
/* E--added by liwei056@2010-6-1 for 6900-Prj */
/*for onu swap by jinhl@2013-06-14*/
int  EVENT_ONUSWITCH = V2R1_DISABLE;

int  g_iLogTypePasSoft  = LOG_TYPE_PAS;
int  g_iLogTypeTkSoft   = LOG_TYPE_TK;
int  g_iLogTypeBcmSoft  = LOG_TYPE_EMMI;
int  g_iLogTypeCmcSoft  = LOG_TYPE_DOCSIS;
int  g_iLogLevelPonSoft = LOG_WARNING;

#if defined(_GPON_BCM_SUPPORT_)
int  g_iLogTypeOcsSoft  = LOG_TYPE_OCS;
int  g_iLogTypeBcm68620Soft   = LOG_TYPE_BCM68620;
int  g_iLogTypeGponAdpSoft  = LOG_TYPE_GPONADP;
#endif

/*B--added by liyang@2015-4-25 for syslog olt&onu filter */
short int g_sLogPonSoftLLID = -1;
short int g_sLogPonSoftOltID = -1;
unsigned char  g_ucLogPonSoftEnable = 0;
/*--added by liyang@2015-4-25 for syslog olt&onu filter */

static PON_handlers_t pon_event_handlers;


short int number_of_test_frames = 100;

extern OLT_onu_table_t       olt_onu_global_table;

#if( RPU_MODULE_IGMP_TVM == RPU_YES )
extern STATUS Register_Tvm_Send();
#endif
#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
int sendOnuSwitchEventMsg( short int olt_id, short int switch_event, PON_redundancy_msg_olt_failure_info_t  failure_info, int source, unsigned short seq, unsigned short flag);
#endif
ONU_REGISTER_HANDLER_hook_t  Register_Handler_for_Onu_Auto_Load = NULL;
ONU_DEREGISTER_HANDLER_hook_t  Deregister_Handler_for_Onu_Auto_Load = NULL;

extern unsigned int V2R1_AutoProtect_Interval;
#if 0
unsigned char *PonAlarmArray[ PON_ALARM_LAST_ALARM] = {
		(unsigned char *)"PON_ALARM_BER",
		(unsigned char *)"PON_ALARM_FER",
		(unsigned char *)"PON_ALARM_SOFTWARE_ERROR",
		(unsigned char *)"PON_ALARM_LOCAL_LINK_FAULT",
		(unsigned char *)"PON_ALARM_DYING_GASP",
		(unsigned char *)"PON_ALARM_CRITICAL_EVENT",
		(unsigned char *)"PON_ALARM_REMOTE_STABLE",
		(unsigned char *)"PON_ALARM_LOCAL_STABLE",
		(unsigned char *)"PON_ALARM_OAM_VENDOR_SPECIFIC",
		(unsigned char *)"PON_ALARM_ERRORED_SYMBOL_PERIOD",
		(unsigned char *)"PON_ALARM_ERRORED_FRAME",
		(unsigned char *)"PON_ALARM_ERRORED_FRAME_PERIOD",
		(unsigned char *)"PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY",
		(unsigned char *)"PON_ALARM_ONU_REGISTRATION_ERROR",
		(unsigned char *)"PON_ALARM_OAM_LINK_DISCONNECTION",
		(unsigned char *)"PON_ALARM_BAD_ENCRYPTION_KEY",
		(unsigned char *)"PON_ALARM_LLID_MISMATCH",
   	 	(unsigned char *)"PON_ALARM_TOO_MANY_ONU_REGISTERING"
/*
		(unsigned char *)"PON_ALARM_PORT_BER",
    		(unsigned char *)"PON_ALARM_DEVICE_FATAL_ERROR",
    		(unsigned char *)"PON_ALARM_VIRTUAL_SCOPE_ONU_LASER_ALWAYS_ON",
    		(unsigned char *)"PON_ALARM_VIRTUAL_SCOPE_ONU_SIGNAL_DEGRADATION",
    		
    		(unsigned char *)""  */
};
#endif
unsigned char *PonFileType[] = {
	(unsigned char *)"unknown",
	(unsigned char *)"OLT firmware",
	(unsigned char *)"DBA algorithm formatted as DLL",
	(unsigned char *)"ONU firmware"
};

unsigned char *StartEncryptRetCode[] = {
	(unsigned char *)"start Encryption-success ",
	(unsigned char *)"start Encryption-error "
};

unsigned char *StopEncryptRetCode[] = {
	(unsigned char *)"stop Encryption-success",
	(unsigned char *)"stop Encryption-error"
};

unsigned char *UpdateEncryptKeyRetCode[] = {
	(unsigned char *)"update Encryption Key-success",
	(unsigned char *)"update Encryption Key-error"
};

unsigned char *OAMVersion_s[] = {
	(unsigned char *)"",
	(unsigned char *)"OAM standard according to 802.3ah draft 1.2",
	(unsigned char *)"OAM standard according to 802.3ah draft 2.0",
	(unsigned char *)"OAM standard according to 802.3ah Standard 3.0"
};

unsigned char *PonPortType_s[] = {
	(unsigned char *)"EponMauType-unknown",
	(unsigned char *)"EponMauType1000Base-PXOLT",
	(unsigned char *)"EponMauType1000Base-PXONU",
	(unsigned char *)"EponMauType1000Base-PX10DOLT",
	(unsigned char *)"EponMauType1000Base-PX10DONU",
	(unsigned char *)"EponMauType1000Base-PX10UOLT",
	(unsigned char *)"EponMauType1000Base-PX10UONU",
	(unsigned char *)"EponMauType1000Base-PX20DOLT",
	(unsigned char *)"EponMauType1000Base-PX20DONU",
	(unsigned char *)"EponMauType1000Base-PX20UOLT",
	(unsigned char *)"EponMauType1000Base-PX20UONU",
	(unsigned char *)"EponMauType10GBase-PRX30UONU", /*asymmetric 10G-EPON */
	(unsigned char *)"EponMauType10GBase-PRX30DOLT", /*asymmetric 10G-EPON */
	(unsigned char *)"EponMauType10GBase-PR30UONU", /*symmetric 10G-EPON */
	(unsigned char *)"EponMauType10GBase-PR30DOLT", /*symmetric 10G-EPON */
	(unsigned char *)"GPON-OLT(2G/1G)", /*asymmetric GPON olt */
	(unsigned char *)"GPON-ONU(2G/1G)", /*asymmetric GPON onu*/
	(unsigned char *)"GPON-OLT(2G/2G)", /*symmetric GPON olt*/
	(unsigned char *)"GPON-ONU(2G/2G)" /*symmetric GPON onu*/
	/*stay consistent with PONPORTTYPE enum in OltGeneral.h*/
};


unsigned char *PON_olt_physical_port_s[] = {
	(unsigned char *)"Default selection of physical port",
	(unsigned char *)"PON port",
	(unsigned char *)"a.k.a. CNI, Network, upstream port",
	(unsigned char *)"Both PON and System ports"
};
 

unsigned char *PON_frame_type_s[] = {
	(unsigned char *)"Hardware determines the frame type automatically",
	(unsigned char *)"Regular Ethernet frame, containing high layers data ",
	(unsigned char *)"60 bytes (net) PON control frame"
};

/*typedef enum{
	ONU_NOT_EXIST,
	ONU_EXIST
}OnuStatus1;*/

/*extern STATUS setOnuStatus( const ulong_t slot, const ulong_t port, const ulong_t onuid, OnuStatus1 status );*/
extern short int CommEhtFrameReveive (
                                 const short int                 OltId,
                                 const short int                 OnuId,
                                 const unsigned short            llid,
                                 const unsigned short            length,
                                 void                     *content );
extern CHAR *macAddress_To_Strings(UCHAR *pMacAddr);


extern int CommOnuOamMsgFreeAll(unsigned short OltId, unsigned short OnuId);

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
extern STATUS onuDevIdxIsSupportE1(ULONG onuDevId);
extern STATUS RestoreOnuE1LinkAll(unsigned long onuDevIdx);
#endif
extern int OnuEvent_ClearRunningDataMsg_Send(short int PonPortIdx);


/*short int Ping_Pong_Test( const short int olt, const short int llid, const unsigned long  response_time)
{

	sys_console_printf("\r\nThe Pong event Recved \r\n");
	sys_console_printf("pon %d/%d llid %d Response time %d \r\n", GetCardIdxByPonChip(olt), GetPonPortByPonChip(olt), llid, response_time );
	return( ROK );

}*/

/* Frame received handler
**
** An event indicating Ethernet frame received from remote destination (PON / System).
** The frame is formatted as a valid Ethernet frame, excluding Preamble, SFD, and FCS fields.
** The frame may be an OAM frame, which have an OAM code not managed by PAS-SFOT.
** Frame content shouldn't be freed by the handler function.
** After the event function returns, the memory used for the frame content may be overrun.
**
** Input Parameters:
**			    olt_id	 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				port	 : Physical OLT source port (PON / System), 
**						   values: OLT_PHYSICAL_PORT_PON, OLT_PHYSICAL_PORT_SYSTEM
**				type	 : One of the frame types supported by OLT hardware, range: enumerated range
**						   port - frame types configurations:
**						   PON port     - Data, MAC control frames types
**						   System port  - Default frame type
**				llid	 : Source LLID, 
**						   range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT  - Source LLID
**							      PON_LLID_NOT_AVAILABLE					   - LLID not available+
**				length	 : Frame length in bytes (excluding Preamble, SFD, and FCS), 
**						   range: MIN_ETHERNET_FRAME_SIZE - MAX_ETHERNET_FRAME_SIZE
**				content  : Pointer to the first byte of the frame
**
** Return codes:
**				Return codes are not specified for an event
**
** +Couldn't be determined by the OLT hardware
*/

/* 2007-5-25 增加ONU 类型标识，用于区分是否是DAYA ONU；IGMP处理时会有不同的方式*/
extern LONG Igmp_Snoop_AuthRequire(long lOltPort, long lOltNo, long lOnuNo, long IsDayaOnu,
                            long lOnuPort, char *buffer, long ldatLen, long lModId);

int  ReceivedFrameIsOAM( unsigned char *content )
{
	if( content == NULL ) return (FALSE );

	if((content[12] == 0x88 ) && (content[13] == 0x09 )  && (content[14] == 0x03 ) 
		&& (content[18] == 0 ) && (content[19] == 0x0f) && (content[20] == 0xe9))
		return( TRUE );
	else if((content[12] == 0x88 ) && (content[13] == 0x09 )  && (content[14] == 0x03 ) 
		&& (content[18] == 0x0c) && (content[19] == 0x7c) && (content[20] == 0x7d))
		return( TRUE );
	else return ( FALSE );

}

extern long glIgmpAuthDebug;

short int Ethernet_frame_received_handler(
									const short int				   olt_id,
									const PON_olt_physical_port_t    port,
									const PON_frame_type_t		   type, 
									const short int				   llid,
									const short int				   length, 
									const void					  *content )
{
	unsigned long aulMsg[4] = { MODULE_PON, FC_OAM_COMM_RECV, 0, 0};
	OamMsgRecv_S  *RecvOamMsg;
	unsigned char *MsgBuf;

	RecvOamMsg = (OamMsgRecv_S *)VOS_Malloc(sizeof(OamMsgRecv_S),MODULE_PON );
	if(RecvOamMsg == NULL )
		{
		VOS_ASSERT(0);
		return( RERROR );
		}
	MsgBuf = (unsigned char *)VOS_Malloc(length+2,MODULE_PON );
	if(MsgBuf == NULL )
		{
		VOS_ASSERT(0);
		VOS_Free(RecvOamMsg);
		return( RERROR );
		}
	VOS_MemZero(MsgBuf, length+2);
	
	RecvOamMsg->olt_id = olt_id;
	RecvOamMsg->port = port;
	RecvOamMsg->type = type;
	RecvOamMsg->llid = llid;
	RecvOamMsg->length = length;
	RecvOamMsg->content = MsgBuf;

	VOS_MemCpy(MsgBuf, content, length);

	aulMsg[3] = (unsigned long)RecvOamMsg;
	

	if( VOS_QueSend( g_Pon_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		/*VOS_ASSERT(0);*/ /* 问题单: 16439 modified by duzhk 该问题主要是因为倒换期间因收到太多管理报文等原因导致
        					CDP通道太忙，参考解指导的意见，注释掉断言*/
		VOS_Free(RecvOamMsg);
		VOS_Free(MsgBuf);
		/*sys_console_printf("  error: VOS send message err\r\n"  );*/
		}

	return( ROK );
}

extern LONG Igmp_Snoop_AuthRequire_Cdp(long lOltPort, long lPonId, long lOnuNo, long lOnuType, 
                            long lOnuPort, char *buffer, long ldatLen, long lModId);
short int Ethernet_frame_received_handler_1 (
									const short int				   olt_id,
									const PON_olt_physical_port_t    port,
									const PON_frame_type_t		   type, 
									const short int				   llid,
									const short int				   length, 
									const void					  *content )
{
	short int OnuIdx;
	short int PonPortIdx;
	/*short int  i;	*/
	unsigned char *data/*,*p*/;

	if( content == NULL  )return( RERROR );
	data = (unsigned char *)content;
		
	PonPortIdx = olt_id;	 
	 
	OnuIdx = GetOnuIdxByLlid(olt_id, llid);

	if(OnuIdx == RERROR ) return( RERROR );

	CHECK_ONU_RANGE
	
	if(EVENT_OAM_DEBUG == V2R1_ENABLE)
		{
#if 0
         	sys_console_printf("\r\n ether frame received form pon %d onu %d\r\n",olt_id, (OnuIdx+1));
		if(OnuIdx == RERROR ) 
			{
			sys_console_printf(" llid err\r\n");
			return( RERROR );
			}
#endif
		    if(data[18] != 0x01)
		    {
        		sys_console_printf(" ether frame received form onu%d/%d/%d port%s\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), (OnuIdx+1), PON_olt_physical_port_s[port]);
        		sys_console_printf("  the frame len=%d,type=%d:%s\r\n",  length, type, PON_frame_type_s[type] );

        		/*
        		if( length > 60 )
        		   counter = 60;
		else counter = length;
		*/
		/*for( i = 0; i< length; i++ )
			{
			if( (i %20 )== 0 )
				{
				sys_console_printf("\r\n");
        				sys_console_printf("        ");
        				}
        			sys_console_printf(" %02x", *(unsigned char *)(data+i) );            
        			}
        		sys_console_printf("\r\n");*/
    			pktDataPrintf(data, length);
		    }
		}
	
	if ( ReceivedFrameIsOAM((unsigned char *) content ) == TRUE )	
		{
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
		if(ReceivedFrameIsGwOAM_OpticalScope_Rsp((unsigned char *) content ) == TRUE)
			GwOamMsg_OpticalScope_Handler(PonPortIdx,OnuIdx,length,(unsigned char *)content);
		else
#endif
#if 0
#if( EPON_MODULE_USER_TRACE == EPON_MODULE_YES )
		if(ReceivedFrameIsGwOAM_UserTrace_Rsp((unsigned char *) content ) == TRUE)
			GwOamMsg_UserTrace_Handler(PonPortIdx,OnuIdx,length,(unsigned char *)content);
		else
#endif
#endif
		 CommEhtFrameReveive (olt_id,OnuIdx,llid,length,(void *)content);
		 return( ROK );
		}
	
	else{
		long lOltPort;
		int  IsDayaOnu= TRUE ;
		
			if(!((data[0] == 0x01) && (data[1] == 0x00) && (data[2] == 0x5e)))
				return(RERROR);
		lOltPort = IFM_ETH_CREATE_INDEX( GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id));

			if(GetOnuVendorType(PonPortIdx, OnuIdx) == ONU_VENDOR_CT )
				IsDayaOnu = FALSE;
			/*
			if( GetOnuType(  PonPortIdx, OnuIdx, &IsDayaOnu) != ROK ) 
				return( RERROR );
			if(( IsDayaOnu == V2R1_ONU_GT811) ||( IsDayaOnu == V2R1_ONU_GT812))
				IsDayaOnu = TRUE;
			else IsDayaOnu = FALSE;
			*/
		if(glIgmpAuthDebug == V2R1_ENABLE)
			{
			       sys_console_printf("Send IGMP Msg To AuthRequire\r\n");
		        	sys_console_printf("lOltPort=%d,lOltNo=%d,lOnuNo=%d,IsDayaOnu=%d,lOnuPort=0,ldataLen=%d,lModId=0x8700\r\n",lOltPort, olt_id, OnuIdx,IsDayaOnu, length);
                      }
                if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
        		Igmp_Snoop_AuthRequire(lOltPort, olt_id, (OnuIdx+1), IsDayaOnu,  0, (char *)content, length, MODULE_PON );
                else
                    Igmp_Snoop_AuthRequire_Cdp(lOltPort, olt_id+1, (OnuIdx+1), IsDayaOnu,  0, (char *)content, length, MODULE_PON);
                return( ROK );
		}
	return( ROK );
}

/*****************************************************
 *
 *    Function:  SendDataFrameToOnu( short int PonPortIdx, short int OnuIdx, int length, unsigned char *content )
 *
 *    Param:  short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
 *                           
 *    Desc:   这个函数用来测试在PAS5001上将CT OAM 帧以数据包的形式发送给ONU
 *               这个CT OAM 帧的作用是开始发起CT 扩展OAM 发现
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
/*unsigned char  TestData[] = {
	0x01, 0x80, 0xc2, 0x00, 0x00, 0x02, 0x00, 0x0f, 0xe9, 0x0f, 0xff, 0xff, 0x88, 0x09, 0x03, 0x00,
	0x50, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00, 0x06, 0x0b, 0x05, 0xee, 0x00, 0x0f, 0xe9, 0x47, 0x57,
	0x31, 0x30, 0x02, 0x10, 0x01, 0x00, 0x01, 0x00, 0x14, 0x05, 0xee, 0xff, 0xff, 0xff, 0x63, 0x01, 
	0x00, 0x00, 0xfe, 0x0b, 0x11, 0x11, 0x11, 0x01, 0x01, 0x11, 0x11, 0x11, 0x01, 0x00
};*/

/*int SendDataFrameToOnu( short int PonPortIdx, int length, unsigned char *content )*/
int BroadcastDataFrameToOnu( short int PonPortIdx, int length, unsigned char *content )
{
	short int ret;
	/*
	short int llid;

	CHECK_ONU_RANGE

	if( content == NULL ) return( RERROR );
	VOS_MemCpy( content, TestData, length );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if(llid == INVALID_LLID ) return( RERROR );
	
	if( GetOnuOperStatus( PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) 
		return( RERROR );
	*/
#if 0
	ret = PAS_send_frame( PonPortIdx, length, PON_PORT_PON, PON_LLID_NOT_AVAILABLE, PON_EVERY_LLID, content );
#else
    ret = OLT_SendFrame2PON( PonPortIdx, PON_LLID_NOT_AVAILABLE, (void*)content, length );
#endif

	if( ret == 0 ) return( ROK );
	else return( RERROR );
}

int BroadcastOamFrameToOnu( short int PonPortIdx, int length, unsigned char *content )
{
	short int ret;
	if(EVENT_OAM_DEBUG == V2R1_ENABLE)
	{
		/*int i;
		sys_console_printf("   %s/port%d onu %d port%s \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		sys_console_printf("   the gw oam-frame len %d \r\n",  length );

		for( i = 0; i< length; i++ ){
			if( (i %20 )== 0 )
				{
				sys_console_printf("\r\n");
				sys_console_printf("        ");
				}
			sys_console_printf(" %02x", *(unsigned char *)(content+i) );            
			}
		sys_console_printf("\r\n");*/
		sys_console_printf("   PON%d/%d:Broadcast OAM frame-len=%d\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), length);
		pktDataPrintf( content, length );
	}


#if 0
	ret = PAS_send_frame( PonPortIdx, length, PON_PORT_SYSTEM, PON_LLID_NOT_AVAILABLE, PON_EVERY_LLID, content );
#else
    ret = OLT_SendFrame2CNI( PonPortIdx, PON_LLID_NOT_AVAILABLE, (void*)content, length );
#endif

	if( ret == 0 ) return( ROK );
	else return( RERROR );
}

/*added by wangjiah@2016-9-22:begin*/
int UnicastOamFrameToOnu( short int PonPortIdx, short int llid, int length, unsigned char *content )
{
	short int ret;
	if(EVENT_OAM_DEBUG == V2R1_ENABLE)
	{
		sys_console_printf("   PON%d/%d:Broadcast OAM frame-len=%d\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), length);
		pktDataPrintf( content, length );
	}

    ret = OLT_SendFrame2CNI( PonPortIdx, llid, (void*)content, length );

	if( ret == 0 ) return( ROK );
	else return( RERROR );
}
/*added by wangjiah@2016-9-22:end*/

int  SendOamFrameToOnu( short int PonPortIdx, int length, unsigned char *content )
{
	short int ret;
	short int Llid;
	short int OnuIdx;


	for(OnuIdx=0;OnuIdx<MAXONUPERPON;OnuIdx++)
		{
		Llid = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
		if(Llid == INVALID_LLID) continue;
		if(EVENT_OAM_DEBUG == V2R1_ENABLE)
			{
			/*int i;*/
			sys_console_printf("   send gw oam-frame to pon%d/%d onu %d\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
			/*for( i = 0; i< length; i++ ){
				if( (i %20 )== 0 )
					{
					sys_console_printf("\r\n");
					sys_console_printf("        ");
					}
				sys_console_printf(" %02x", *(unsigned char *)(content+i) );            
				}
			sys_console_printf("\r\n");*/
			pktDataPrintf( content, length );
			}
#if 0
		ret = PAS_send_frame( PonPortIdx, length, PON_PORT_SYSTEM, Llid, 0, content );
#else
        ret = OLT_SendFrame2CNI( PonPortIdx, Llid, (void*)content, length );
#endif
		}
	return( ROK );
}


/* Load OLT binary completed handler
**
** Event indicating a binary loading process has finished. This event always proceeds a 
** PAS_load_olt_binary function call. Error return code indicates OLT file processing error 
** (e.g. decryption).
**
** Input Parameters:
**			    olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				type		: OLT Binary type, values: enumerated values
**				return_code : Load OLT binary process completion status, 
**							  values: PAS_EXIT_OK, PAS_EXIT_ERROR
**				
** Return codes:
**				Return codes are not specified for an event
*/
short int PAS_load_olt_binary_completed_handler ( const short int			olt_id, 
															   const PON_binary_type_t  type,
															   const short int			return_code )

{
	SendLoadBinaryCompletedMsg( olt_id, type, return_code );
	return( PAS_EXIT_OK );
}

/* ONU registration handler
** 
** An event indicating an ONU is introduction into the network; It is raised after MPCP registration, and OAM link is established. 
**
** Input Parameters:
**			olt_id					 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		      onu_id					 : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  
**			mac_address			 : ONU MAC address, valid unicast MAC address value	
**			authentication_sequence	 : Authentication bit sequence, taken from the ONU's EEPROM, 
**									   range: (PON_AUTHENTICATION_SEQUENCE_SIZE*BITS_IN_BYTE) bit value
**			supported_oam_standard	 : OAM standrd the ONU supports. Values: enum values
**
** Return codes:
**			Return codes are not specified for an event
**
*/
short int  Onu_registration_handler 
				   ( const short int				 olt_id, 
					 const PON_onu_id_t		 onu_id, 
					 const mac_address_t		 mac_address,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard )
{
    
	if ( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf("PON_HANDLER_ONU_registration\r\n");
		sys_console_printf(" PONid:%d,llid:%d,MACaddr:%02x%02x.%02x%02x.%02x%02x\r\n", olt_id, onu_id, 
			mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]); 
	}

	sendOnuRegistrationMsg( olt_id, onu_id, mac_address, authentication_sequence, supported_oam_standard, ONU_EVENTFLAG_REAL );
	return( PAS_EXIT_OK );
}
short int  GponOnu_registration_handler 
				   ( const short int				 olt_id, 
					 const PON_onu_id_t		         onu_id, 
					 char*		                     sn,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard )
{    
	/* sys_console_printf("GponOnu_registration_handler, olt:%d, onu:%d, sn:%s\r\n",
		olt_id, onu_id, sn); */
	sendGponOnuRegistrationMsg( olt_id, onu_id, sn, authentication_sequence, supported_oam_standard, ONU_EVENTFLAG_REAL );
	return( PAS_EXIT_OK );
}


/*Begin: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
short int  Onu_registration_handler_8411 
				   ( const short int					  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const GW10G_PON_registration_data_t registration_data)
{
	int i = 0;
	if ( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf("PAS_HANDLER_ONU_registration\r\n");
		sys_console_printf("olt_id:%d, onu_id:%d, MACaddr:%02x%02x.%02x%02x.%02x%02x,",olt_id,onu_id,registration_data.mac_address[0],
			registration_data.mac_address[1],registration_data.mac_address[2],registration_data.mac_address[3],
			registration_data.mac_address[4],registration_data.mac_address[5]);
	
		if(registration_data.supported_oam_standard == OAM_STANDARD_VERSION_1_2) 
		{
			sys_console_printf(" OAM V1.2,");
		}
		else if (registration_data.supported_oam_standard == OAM_STANDARD_VERSION_2_0) 
		{
			sys_console_printf(" OAM V2.0,");
		}
		else if (registration_data.supported_oam_standard == OAM_STANDARD_VERSION_3_3) 
		{
			sys_console_printf(" OAM Standard version,");
		}
		else
		{
			sys_console_printf(" unsupported OAM version,");
		}
		if(registration_data.onu_type == GW10G_PON_1G_ONU)            
		{
			sys_console_printf(" 1G/1G ONU");
		}
		else if (registration_data.onu_type == GW10G_PON_ASYMMETRIC_10G_ONU) 
		{
			sys_console_printf(" 1G/10G ONU");
		}
		else if (registration_data.onu_type == GW10G_PON_SYMMETRIC_10G_ONU)  
		{
			sys_console_printf(" 10G/10G ONU");
		}
		else	
		{
			sys_console_printf(" unsupported ONU type");
		}

		sys_console_printf("\r\n");

		for(i=0;i<PON_AUTHENTICATION_SEQUENCE_SIZE;i++) 
		{
			sys_console_printf("0x%02x ",registration_data.authentication_sequence[i]);
		}
		sys_console_printf("\r\n");
	}
	
	sendOnuRegistrationMsg( olt_id, onu_id, registration_data.mac_address, registration_data.authentication_sequence, registration_data.supported_oam_standard, ONU_EVENTFLAG_REAL );
	
	return PAS_EXIT_OK;
}

short int Olt_added_handler ( const PON_olt_id_t  olt_id )
{
    if ( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf("Enter into Olt_added_handler\r\n");
	}
	return PAS_EXIT_OK;
}

short int Pon_cni_link_handler_8411( const short int PonPortIdx, const GW10G_PON_cni_id_e cni_port, const bool status )
{
    if ( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf("\r\n%% pon%d/%d cni link status changed to %s\r\n",GetCardIdxByPonChip(PonPortIdx),GetPonPortByPonChip(PonPortIdx),
 				( (status == TRUE)  ? "up" : "down") );
		
	}
	return PAS_EXIT_OK;
}
/*End: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/

/* ONU deregistration handler
**
** An event indicating ONU was deregistered from the network. See API document for event handling 
** guidelines.
** Assumptions: ONU id may be reallocated in the future, either for the same physical ONU or for 
** another ONU.
**
** Input Parameters:
**			      olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				onu_id		: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  
**	deregistration_Code		: Reason for the deregistration event. Values: enum values
**
** Return codes:
**				Return codes are not specified for an event
**
*/

/*short int  Onu_deregistration_handler ( const short int olt_id,
								 const PON_onu_id_t onu_id,
								 const PON_onu_deregistration_code_t  deregistration_code )
{	
	if( deregistration_code >= PON_ONU_DEREGISTRATION_LAST_CODE ){
		if(EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf("Onu deregistration,reason code=%d(Onu_deregistration_handler)\r\n", deregistration_code  );
		return( PAS_EXIT_ERROR );
		}
	sendOnuDeregistrationMsg( olt_id, onu_id, deregistration_code );
	return( PAS_EXIT_OK );
}*/


short int  Onu_deregistration_handler( const short int olt_id,
								 const PON_onu_id_t onu_id,
								 const PON_onu_deregistration_code_t  deregistration_code)
{
    
	if ( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf("PON_HANDLER_ONU_deregistration\r\n");
		sys_console_printf(" PONid:%d,llid:%d,reason code=%d\r\n", olt_id, onu_id, deregistration_code ); 
	}

	if( deregistration_code >= PON_ONU_DEREGISTRATION_LAST_CODE )
	{
		return( PAS_EXIT_ERROR );
	}
	sendOnuDeregistrationMsg( olt_id, onu_id, deregistration_code, ONU_EVENTFLAG_REAL );
	#if 0/*for onu swap by jinhl@2013-02-22*/
	/*在pas_soft中已经对standby的离线做过处理，此处不需要处理*/
	{
	    short int partner_olt_id = 0;
		int swap_mode = 0;
		int swap_status = 0;
		int ret = 0;
		PON_redundancy_llid_redundancy_mode_t mode = 0;
		ret = OLTAdv_GetHotSwapMode(olt_id, &partner_olt_id, &swap_mode, &swap_status);
		if((OLT_ERR_OK == ret) && (V2R1_PON_PORT_SWAP_ONU == swap_mode ) )
		{
		    ret = OLT_GetRdnLLIDMode(partner_olt_id, onu_id, &mode);
			if((OLT_ERR_OK == ret) && (PON_REDUNDANCY_LLID_REDUNDANCY_STANDBY == mode))
			{
				OLT_DeregisterLLID(partner_olt_id, onu_id, 0);
				
				if ( EVENT_REGISTER == V2R1_ENABLE )
				{
					sys_console_printf("force deregister  %d/%d\r\n",olt_id, onu_id);
				}
			}
		}
	}
	
	#endif
	
	return( PAS_EXIT_OK );
}


/* Alarm handler
**
** Event indicating an alarm has occurred. The alarm may be triggered by an execution failure or by 
** threshold value meeting.
**
** Input Parameters:
**		 olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID;
**                           PON_PAS_SOFT_ID: alarm is not OLT specific;
**		 source_id	: Alarm source, values: 
**					  PON_PAS_SOFT_ID								   - PAS-SOFT
**					  PON_OLT_ID									   - OLT device
**					  PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  - ONU device
**        type		: Alarm type, range: enumerated values
**		parameter	: Parameter required for alarm specification, see API document and 
**					  'Alarms details' definitions for details
**		data		: Relevant data for the alarm (PON_*_alarm_data_t casted to (void *) ): 
**					  including counters measurement, exceeded threshold value, error codes etc.,
**					  see API document and 'Alarms details' definitions for details  and castings
**					  (different for every alarm type)
**				
** Return codes:
**		Return codes are not specified for an event
**
*/

short int  Alarm_handler ( const short int	    olt_id, 
				    const short int	    alarm_source_id, 
				    const PON_alarm_t   alarm_type, 
				    const short int	    alarm_parameter, 
				    const void         *alarm_data )
{
	sendPonAlarmMsg( olt_id, alarm_source_id, alarm_type, alarm_parameter, alarm_data );
	return( PAS_EXIT_OK );
}

short int PAS_pon_loss_handler( short int olt_id, short int PON_loss )
{
	/* modified by wangjiahe@2017-06-26
	 * prevent loss event from reporting when pon protection is configured.*/
	if(V2R1_PON_PORT_SWAP_ENABLE != PonPortSwapEnableQuery(olt_id))
	{
		sendPonLossMsg( olt_id, PON_loss);
	}
/*	if( 1 == PON_loss)
		ClearOnuLaserAlwaysOnAlarmsWhenPon_Loss( olt_id );*/
	return( PAS_EXIT_OK);
}

short int  OnuDeniedByMacAddrTable( short int PonPortIdx, mac_address_t  Onu_MacAddr)
{
	/*sys_console_printf("Mac is:%02x-%02x-%02x-%02x-%02x-%02x", Onu_MacAddr[0], Onu_MacAddr[1], Onu_MacAddr[2], Onu_MacAddr[3], Onu_MacAddr[4], Onu_MacAddr[5] );*/
	sendOnuDeniedMsg( PonPortIdx, Onu_MacAddr );
	return( PAS_EXIT_OK );
}

short int OnuRegisterDeniedHandler( OnuRegisterDenied_S  *OnuRegisterDeniedData )
{
	short int PonPortIdx;
	short int OnuIdx;
	unsigned char  *MacAddr;

	PonPortIdx = OnuRegisterDeniedData->olt_id;

	CHECK_PON_RANGE;

	MacAddr = &(OnuRegisterDeniedData->Onu_MacAddr[0]);

	if(  EVENT_REGISTER == V2R1_ENABLE )
		{	
		sys_console_printf("\r\n pon%d/%d Onu Register Denied\r\n",  GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
		sys_console_printf(" illegal Mac is:%02x%02x.%02x%02x.%02x%02x", MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5] );
		}

	OnuIdx = GetOnuIdxByMacPerPon(PonPortIdx, MacAddr);
	if( OnuIdx != RERROR )
	{
		if( GetOnuTrafficServiceEnable( PonPortIdx, OnuIdx) != V2R1_DISABLE )
			Trap_OnuAuthFailure( PonPortIdx,  MacAddr );
	}	
	else 
		Trap_OnuAuthFailure( PonPortIdx,  MacAddr );

	PonPortTable[PonPortIdx].PendingOnuCounter = DEFAULT_COUNTER;
	
	return( ROK );
}


/* OLT reset handler
**
** An event indicating OLT was reset. The reset may result from hardware reset, OLT firmware reset 
** or PAS-SOFT reset (e.g. communication problems).
** The OLT may be active again at the time of the event, or may be not (depends on the specific 
** problem and the specific setup). See the API document for event handling guidelines.
**
** Note: The code implemented in this event handler, unlike other events handlers, is free of API 
** commands usage limitations. 
**
** Input Parameters:
**			    olt_id	   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				code	   : Reset code. Values: enum values
**
** Return codes:
**				Return codes are not specified for an event
**
*/
short int  Olt_reset_handler ( const short int	olt_id,
						const PON_olt_reset_code_t  code )
{
	sendPonResetEventMsg( olt_id, code );
	/*if( code != PON_OLT_RESET_OLT_EVENT )
		PAS_remove_olt( olt_id ); */

	return (PAS_EXIT_OK);
}

/* B--added by liwei056@2010-1-26 for Pon-FastSwicthHover */
short int  Olt_redundancy_swap_begin_handler ( const short int	olt_id,
						const PON_redundancy_olt_failure_reason_t  reason )
{
	sendPonSwitchEventMsg( olt_id, PROTECT_SWITCH_EVENT_START, reason, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, V2R1_PON_PORT_SWAP_PASSIVE, PonPortSwapTimesQuery(olt_id) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Olt_redundancy_swap_begin_handler(%d, %d)", olt_id, reason);

	return (PAS_EXIT_OK);
}

short int  Olt_redundancy_swap_end_handler ( const short int	olt_id,
						const PON_redundancy_switch_over_status_t  status )
{
	sendPonSwitchEventMsg( olt_id, PROTECT_SWITCH_EVENT_OVER, status, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, V2R1_PON_PORT_SWAP_ACTIVE, PonPortSwapTimesQuery(olt_id) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Olt_redundancy_swap_end_handler(%d, %d)", olt_id, status);

	return (PAS_EXIT_OK);
}
/* E--added by liwei056@2010-1-26 for Pon-FastSwicthHover */

/* B--added by liwei056@2011-12-21 for Pon-FastSwicthHover-CodeTest */
/*Begin:for onu swap by jinhl@2013-02-22*/
short int  Olt_redundancy_swaponu_begin_handler ( const short int	olt_id,
						const PON_redundancy_olt_failure_info_t  failure_info )
{

#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
    int i = 0;
    PON_redundancy_msg_olt_failure_info_t fail_info; 
	VOS_MemSet(&fail_info, 0, sizeof(fail_info));
	
	ONU_SWITCH_DEBUG("Olt_redundancy_swaponu_begin_handler(%d, %d, %d)\r\n", olt_id, failure_info.reason, failure_info.llid_n);
	
	if(V2R1_PON_PORT_SWAP_ONU != GetPonPortHotSwapMode(olt_id))
	{
	    sys_console_printf("This is not onu swap-mode\r\n");
		return PAS_EXIT_ERROR;
	}
	
	#if 1
	fail_info.reason = failure_info.reason;
	fail_info.llid_n = failure_info.llid_n;
	if(PON_REDUNDANCY_OLT_ONUSWAPOVER == fail_info.reason)
	{	    
		if( (fail_info.llid_n > 0) && (fail_info.llid_n < PON_REDUNDANCY_ONU_LLID_ALL))
		{
		    ONU_SWITCH_DEBUG("Failed llids are:");
		    for(i = 0; i < failure_info.llid_n; i++)
			{
				if(i%16 == 0)
				{
					OLT_SWITCH_DEBUG("\r\n");
				}
				
				fail_info.llid_list_marker[i] = failure_info.llid_list_marker[i];
				ONU_SWITCH_DEBUG("%d ",fail_info.llid_list_marker[i]);
			}
			ONU_SWITCH_DEBUG("\r\n");
		}
		
	    sendOnuSwitchEventMsg( olt_id, PROTECT_SWITCHONU_EVENT_OVER, fail_info, PROTECT_SWITCH_EVENT_SRC_HARDWARE, PonPortSwapTimesQuery(olt_id) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
		
	}
	else
	{
	    ONU_SWITCH_DEBUG("onu swap start\r\n");
		/*sendOnuSwitchEventMsg( olt_id, PROTECT_SWITCHONU_EVENT_START, fail_info, PROTECT_SWITCH_EVENT_SRC_HARDWARE, PonPortSwapTimesQuery(olt_id) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );*/
	}
	
	#endif
#else
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Olt_redundancy_swaponu_begin_handler(%d, %d, %d)", olt_id, failure_info.reason, failure_info.llid_n);
	sendPonSwitchEventMsg( olt_id, PROTECT_SWITCH_EVENT_START, failure_info.reason, PROTECT_SWITCH_EVENT_SRC_HARDWARE, V2R1_PON_PORT_SWAP_ACTIVE, 0, PROTECT_SWITCH_EVENT_FLAGS_NONE );
	sendPonSwitchEventMsg( olt_id, PROTECT_SWITCH_EVENT_OVER, PROTECT_SWITCH_RESULT_SUCCEED, PROTECT_SWITCH_EVENT_SRC_HARDWARE, V2R1_PON_PORT_SWAP_ACTIVE, 0, PROTECT_SWITCH_EVENT_FLAGS_NONE );
#endif

    return (PAS_EXIT_OK);
}
/*End:for onu swap by jinhl@2013-02-22*/
short int  Olt_redundancy_swap_between_handler ( const PON_olt_id_t   olt_id,
						PON_redundancy_olt_failure_reason_t   reason)
{
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Olt_redundancy_swap_between_handler(%d, %d) is not supported.", olt_id, reason);

    return (PAS_EXIT_OK);
}

short int  Olt_redundancy_slave_unavail_handler ( const PON_olt_id_t   slave_olt_id)
{
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Olt_redundancy_slave_unavail_handler(%d) is not supported.", slave_olt_id);

    return (PAS_EXIT_OK);
}

short int  Olt_redundancy_swap_success_handler (const PON_olt_id_t   master_olt_id, 
                                                          const PON_olt_id_t   slave_olt_id)
{
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Olt_redundancy_swap_success_handler(%d, %d) is not supported.", master_olt_id, slave_olt_id);

    return (PAS_EXIT_OK);
}

short int  Olt_redundancy_swap_fail_handler (const PON_olt_id_t   master_olt_id, 
                                                         const PON_olt_id_t   slave_olt_id)
{
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Olt_redundancy_swap_fail_handler(%d, %d) is not supported.", master_olt_id, slave_olt_id);

    return (PAS_EXIT_OK);
}

short int  Olt_redundancy_swap_opticalquery_handler (const PON_olt_id_t   master_olt_id, 
                                                                        const PON_olt_id_t   slave_olt_id)
{
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Olt_redundancy_swap_opticalquery_handler(%d, %d) is not supported.", master_olt_id, slave_olt_id);

    return (PAS_EXIT_OK);
}
/* E--added by liwei056@2011-12-21 for Pon-FastSwicthHover-CodeTest */

/* ONU authorization
**
** An ONU was authorized / denied to the network as a result of an authentication process. This event 
** raises in a authentication server including (e.g. RADIUS server) setup, where the OLT communicates 
** with the authentication server. The event indicates the authorization changes were decided by an 
** authentication server and handled by the OLT. PAS_authorize_onu may override ONU authorization mode
** as updated by this event and vice verse, though typically mixing between the two authorization 
** mechanisms is undesired.
**
** Input Parameters:
**		olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id			: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  
**		authorize_mode	: Authorize / deny to the network. values: enumerated values
**		
** Return codes:
**		Return codes are not specified for an event
*/
short int  Onu_authorization_handler( const short int	 olt_id,
									 const PON_onu_id_t	 onu_id,
									 const PON_authorize_mode_t  authorize_mode )
{
	if( EVENT_DEBUG == V2R1_ENABLE) {
		sys_console_printf("Auth Err:pon%d/%d llid %d, ", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id);
		if ( authorize_mode == PON_DENY_FROM_THE_NETWORK )
			sys_console_printf("ONU deny from the network\r\n");
		else if ( authorize_mode == PON_AUTHORIZE_TO_THE_NETWORK )
			sys_console_printf("ONU authorize to the network\r\n");
		else
			sys_console_printf("\r\n");
	}
	return (PAS_EXIT_OK);
}

/* A function used for handling start encryption acknowledge */
short int  Start_encryption_acknowledge_handler
					( const short int	 olt_id, 
					const PON_llid_t	llid, 
					const PON_start_encryption_acknowledge_codes_t  return_code )
{
	OnuStartEncryptionInfo_S *OnuStartEncrypionData;
	LONG ret;

	/*GeneralMsgBody_S OnuRegisterMsg; */
	unsigned long aulMsg[4] = { MODULE_PON, FC_STARTENCRYPTION_COMP, 0,0 };
#if 0
#ifdef  PON_ENCRYPTION_HANDLER
	VOS_SemGive(OnuEncryptSemId);
#endif
#endif
return( PAS_EXIT_OK );
	if(( olt_id < 0) ||(olt_id > MAXPON )) return ( RERROR );
	if(( llid < 0 ) || ( llid > MAXONUPERPON )) return ( RERROR );

	/*
	if( EVENT_ENCRYPT == V2R1_ENABLE ){
		sys_console_printf("\r\nStart Encryption\r\n");
		sys_console_printf("    Pon %d llid %d return code: %s\r\n", olt_id, llid, StartEncryptRetCode[return_code]);
		}
	*/	
	OnuStartEncrypionData = ( OnuStartEncryptionInfo_S *) VOS_Malloc( sizeof(OnuStartEncryptionInfo_S),  MODULE_PON ); 
	if( OnuStartEncrypionData  == NULL ){
		/*sys_console_printf("Error, Malloc buffer not satified (Start_encryption_acknowledge_handler)\r\n" );*/
		VOS_ASSERT( 0 );
		return( RERROR );
		}

	VOS_MemSet( (char *)OnuStartEncrypionData, 0, sizeof( OnuStartEncryptionInfo_S ) );
	OnuStartEncrypionData->olt_id = olt_id;
	OnuStartEncrypionData->llid = llid;
	OnuStartEncrypionData->ret_code = return_code;
	
	aulMsg[2] = sizeof(OnuStartEncryptionInfo_S);
	aulMsg[3] = ( int )OnuStartEncrypionData;

	ret = VOS_QueSend( g_Olt_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK )
	{
		ASSERT( 0 );
		VOS_Free( (void *) OnuStartEncrypionData );
		return( RERROR );
		}	

	return( PAS_EXIT_OK );
}


/* A function used for handling stop encryption acknowledge */
short int Stop_encryption_acknowledge_handler
				  ( const short int 	 olt_id, 
					const PON_llid_t	 llid, 
					const PON_stop_encryption_acknowledge_codes_t  return_code )
{
	OnuStopEncryptionInfo_S *OnuStopEncrypionData;
	LONG ret;

	/*GeneralMsgBody_S OnuRegisterMsg; */
	unsigned long aulMsg[4] = { MODULE_PON, FC_STOPENCRYPTION_COMP, 0,0 };
#if 0
#ifdef  PON_ENCRYPTION_HANDLER
	VOS_SemGive(OnuEncryptSemId);
#endif
#endif
return( PAS_EXIT_OK );	
	if(( olt_id < 0) ||(olt_id > MAXPON )) return ( RERROR );
	if(( llid < 0 ) || ( llid > MAXONUPERPON )) return ( RERROR );

	/*
	if(EVENT_ENCRYPT == V2R1_ENABLE){
		sys_console_printf("\r\nStop Encryption\r\n");
		sys_console_printf("    Pon %d llid %d return code: %s\r\n", olt_id, llid, StopEncryptRetCode[return_code]);
		}
	*/
	OnuStopEncrypionData = ( OnuStopEncryptionInfo_S *) VOS_Malloc( sizeof(OnuStopEncryptionInfo_S),  MODULE_PON ); 
	if( OnuStopEncrypionData  == NULL ){
		/*sys_console_printf("Error, Malloc buffer not satified (Stop_encryption_acknowledge_handler)\r\n" );*/
		VOS_ASSERT( 0 );
		return( RERROR );
		}

	VOS_MemSet( (char *)OnuStopEncrypionData, 0, sizeof( OnuStopEncryptionInfo_S ) );
	OnuStopEncrypionData->olt_id = olt_id;
	OnuStopEncrypionData->llid = llid;
	OnuStopEncrypionData->ret_code = return_code;
	
	aulMsg[2] = sizeof(OnuStopEncryptionInfo_S);
	aulMsg[3] = ( int )OnuStopEncrypionData;

	ret = VOS_QueSend( g_Olt_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK ){

		ASSERT( 0 );
		VOS_Free( (void *) OnuStopEncrypionData );
		return( RERROR );
		}	

	return( PAS_EXIT_OK );
}


/* A function used for handling Update encryption Key acknowledge */
short int  Update_encryption_key_acknowledge_handler
				  ( const short int	  olt_id, 
					const PON_llid_t	   llid, 
					const PON_update_encryption_key_acknowledge_codes_t  return_code )
{
	OnuUpdateEncryptkeyInfo_S *OnuUpdateEncrypionKeyData;
	LONG ret;

	/*GeneralMsgBody_S OnuRegisterMsg; */
	unsigned long aulMsg[4] = { MODULE_PON, FC_UPDATEENCRYPTIONKEY_COMP, 0,0 };
#if 0
#ifdef  PON_ENCRYPTION_HANDLER
	VOS_SemGive(OnuEncryptSemId);
#endif
#endif
return( PAS_EXIT_OK );
	if(( olt_id < 0) ||(olt_id > MAXPON )) return ( RERROR );
	if(( llid < 0 ) || ( llid > MAXONUPERPON )) return ( RERROR );
	/*
	if( EVENT_ENCRYPT  == V2R1_ENABLE){
		sys_console_printf("\r\nUpdate Encryption key\r\n");
		sys_console_printf("    Pon %d llid %d return code: %s\r\n", olt_id, llid, UpdateEncryptKeyRetCode[return_code]);;
		}
	*/
	OnuUpdateEncrypionKeyData = ( OnuUpdateEncryptkeyInfo_S *) VOS_Malloc( sizeof(OnuUpdateEncryptkeyInfo_S),  MODULE_PON ); 
	if( OnuUpdateEncrypionKeyData  == NULL ){
		/*sys_console_printf("Error, Malloc buffer not satified (Stop_encryption_acknowledge_handler)\r\n" );*/
		VOS_ASSERT( 0 );
		return( RERROR );
		}

	VOS_MemSet( (char *)OnuUpdateEncrypionKeyData, 0, sizeof( OnuUpdateEncryptkeyInfo_S ) );
	OnuUpdateEncrypionKeyData->olt_id = olt_id;
	OnuUpdateEncrypionKeyData->llid = llid;
	OnuUpdateEncrypionKeyData->ret_code = return_code;
	
	aulMsg[2] = sizeof(OnuStopEncryptionInfo_S);
	aulMsg[3] = ( int )OnuUpdateEncrypionKeyData;

	ret = VOS_QueSend( g_Olt_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK ){

		ASSERT( 0 );
		VOS_Free( (void *) OnuUpdateEncrypionKeyData );
		return( RERROR );
		}	

	return( PAS_EXIT_OK );
}


/*****************************************************
 *
 *    Function:  SendLoadBinaryCompletedMsg( olt_id, type, return_code )()
 *
 *    Param:    
 *                 
 *    Desc:   pon file load complete; send a msg to pon handler task
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int SendLoadBinaryCompletedMsg( const short int olt_id, const PON_binary_type_t  type, const short int  return_code )
{
	PonFileLoadCompleted_S *PonFileLoadResult;
	LONG ret;
	unsigned long aulMsg[4] = { MODULE_PON, FC_PON_FILE_LOADED, 0, 0 };
	
	PonFileLoadResult = VOS_Malloc( sizeof(PonFileLoadCompleted_S),  MODULE_PON );
	if( PonFileLoadResult == NULL ){
		/*sys_console_printf("Error, Malloc buffer not satified (SendLoadBinaryCompletedMsg)\r\n"  );*/
		VOS_ASSERT( 0 );
		return( RERROR );
		}
	VOS_MemSet( (char *)PonFileLoadResult, 0, sizeof( PonFileLoadCompleted_S ) );
	PonFileLoadResult->olt_id = olt_id;
	PonFileLoadResult->type = type;
	PonFileLoadResult->return_code = return_code;

	aulMsg[2] = sizeof( PonFileLoadCompleted_S );
	aulMsg[3] = ( int ) PonFileLoadResult;

	ret = VOS_QueSend(  g_Pon_Queue_Id, aulMsg,  NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK ){

		ASSERT( 0 );
		VOS_Free( (void *)PonFileLoadResult );
		return( RERROR );
		}	
		
	return ( ROK );

}

/*****************************************************
 *
 *    Function:  sendPonAlarmMsg()
 *
 *    Param:    
 *                 
 *    Desc:   pon alarm callback function; send a msg to pon handler task
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int sendPonAlarmMsg(  const short int	olt_id, 
				    const short int	    alarm_source_id, 
				    const PON_alarm_t   alarm_type, 
				    const short int	    alarm_parameter, 
				    const void         *alarm_data )
{
	PonAlarmInfo_S  *PonAlarmData;
	LONG ret;
	unsigned long aulMsg[4] = { MODULE_PON, FC_PON_ALARM, 0, 0 };
	
	PonAlarmData = (PonAlarmInfo_S*)VOS_Malloc( sizeof(PonAlarmInfo_S), MODULE_PON ); 
	if( PonAlarmData == NULL ){
		/*sys_console_printf("Error, Malloc buffer not satified (Alarm_handler())\r\n"  );*/
		VOS_ASSERT( 0 );
		return( RERROR );
		}

	if( EVENT_ALARM  == V2R1_ENABLE)
	{
		sys_console_printf("EVENT:pon %d/%d, source_id %d,alarm_parameter %d alarm type %d\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id)
			, alarm_source_id, alarm_parameter, alarm_type );
	}
	/*if(  EVENT_ALARM == V2R1_ENABLE ){
		sys_console_printf(" \r\nGet Alarm event \r\n");
		sys_console_printf("    pon %d/%d llid %d, ", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), alarm_source_id );
		sys_console_printf(" Alarm type %d \r\n", alarm_type);
		}*/
		
	VOS_MemSet( ( char *)PonAlarmData, 0, sizeof( PonAlarmInfo_S ) );
	PonAlarmData->olt_id = olt_id;
	PonAlarmData->alarm_source_id = alarm_source_id;
	PonAlarmData->alarm_type = alarm_type;
	PonAlarmData->alarm_parameter = alarm_parameter;
	if( alarm_data != NULL )
		VOS_MemCpy( PonAlarmData->alarm_data, alarm_data, sizeof(PonAlarmData->alarm_data ) );

	aulMsg[2] = sizeof(PonAlarmInfo_S);
	aulMsg[3] = ( int ) PonAlarmData;

	ret = VOS_QueSend( g_Pon_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK ){

		VOS_ASSERT( 0 );
		VOS_Free( (void *)PonAlarmData );
		return( RERROR );
		}	
	return ( ROK );
	
}

/*****************************************************
 *
 *    Function:  sendPonLossMsg()
 *
 *    Param:    
 *                 
 *    Desc:   pon loss callback function,send a msg to pon handler task; 
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int sendPonLossMsg( short int olt_id, short int PON_loss )
{
	LONG  ret;
	unsigned long aulMsg[4] = { MODULE_PON, FC_PONLOSS, 0, 0 };
	aulMsg[2] = olt_id;
	aulMsg[3] = PON_loss;

	ret = VOS_QueSend( g_Pon_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK ){
		ASSERT( 0 );
		return( RERROR );
		}	
	return ( ROK );
}

int sendSfpLossMsg( short int olt_id, bool SFP_loss )
{
	LONG  ret;
	unsigned long aulMsg[4] = { MODULE_PON, FC_SFPLOSS, 0, 0 };
	aulMsg[2] = olt_id;
	aulMsg[3] = SFP_loss;

	ret = VOS_QueSend( g_Pon_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK ){
		ASSERT( 0 );
		return( RERROR );
		}	
	return ( ROK );
}

/*****************************************************
 *
 *    Function:  sendOnuDeniedMsg()
 *
 *    Param:    
 *                 
 *    Desc:   ONU register denied callback function,send a msg to pon handler task; 
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int sendOnuDeniedMsg( short int PonPortIdx, mac_address_t  Onu_MacAddr )
{
	LONG ret;
	unsigned long aulMsg[4] = { MODULE_PON, FC_ONUDENIED, 0,0 };
	OnuRegisterDenied_S  *OnuDeniedData;
	
	OnuDeniedData = (OnuRegisterDenied_S*)VOS_Malloc( sizeof(OnuRegisterDenied_S), MODULE_PON ); 
	if( OnuDeniedData == NULL )
		{
		/*sys_console_printf("Error, Malloc buffer not satified (sendOnuDeniedMsg())\r\n"  );*/
		VOS_ASSERT( 0 );
		return( RERROR );
		}

	OnuDeniedData->olt_id = PonPortIdx;
	VOS_MemCpy( &(OnuDeniedData->Onu_MacAddr[0]), &(Onu_MacAddr[0]), BYTES_IN_MAC_ADDRESS );

	/*sys_console_printf("Mac is:%02x-%02x-%02x-%02x-%02x-%02x", OnuDeniedData->Onu_MacAddr[0], OnuDeniedData->Onu_MacAddr[1], OnuDeniedData->Onu_MacAddr[2], OnuDeniedData->Onu_MacAddr[3], OnuDeniedData->Onu_MacAddr[4], OnuDeniedData->Onu_MacAddr[5] );*/
	aulMsg[2] = sizeof(OnuRegisterDenied_S);
	aulMsg[3] = (int)OnuDeniedData;

	ret = VOS_QueSend( g_Pon_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK ){

		ASSERT( 0 );
		VOS_Free( (void *)OnuDeniedData );
		return( RERROR );
		}	
	return ( ROK );

}


/*****************************************************
 *
 *    Function:  sendPonResetEventMsg()
 *
 *    Param:    
 *                 
 *    Desc:   pon reset callback function,send a msg to pon handler task; the reset may result 
 *              from hardware reset, olt firmware reset or PAS-SOFT decision to reset( e.g. link communication problems )
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int sendPonResetEventMsg( const short int olt_id, const PON_olt_reset_code_t  code )
{
	LONG  ret;
	unsigned long aulMsg[4] = { MODULE_PON, FC_PON_RESET, 0, 0 };
	PonResetInfo_S   *PonResetData;
#if 0	
	if( EVENT_DEBUG == V2R1_ENABLE){
		sys_console_printf("PON reset event ( reset_handler() )\r\n" );
		sys_console_printf("PonPort %d, reset type: %s \r\n",
			olt_id,  PON_olt_reset_code_s[code] );
		}
#endif

	PonResetData = (PonResetInfo_S *)VOS_Malloc( sizeof(PonResetInfo_S), MODULE_PON );
	if( PonResetData == NULL ){
		/*sys_console_printf("Error, Malloc buffer not satified ( reset_handler())\r\n"  );*/
		VOS_ASSERT( 0 );
		return( RERROR );
		}
	VOS_MemSet( (char *)PonResetData, 0, sizeof( PonResetInfo_S ) );
	PonResetData->olt_id = olt_id;
	PonResetData->code = code;

	aulMsg[2] = sizeof( PonResetInfo_S );
	aulMsg[3] = ( int ) PonResetData;

	ret = VOS_QueSend( g_Pon_Queue_Id, aulMsg,  NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK ){

		ASSERT( 0 );
		VOS_Free( (void *)PonResetData );
		return( RERROR );
		}	
	return ( ROK );
	
}

/* B--added by liwei056@2010-1-26 for Pon-FastSwicthHover */
int sendPonSwitchEventMsg( short int olt_id, int switch_event, int code, int source, int slot, int status, int seq, int flag)
{
	LONG  ret;
	unsigned long aulMsg[4] = { MODULE_PON, FC_PONSWITCH, 0, 0 };
	PonSwitchInfo_S   *PonSwitchData;

	PonSwitchData = (PonSwitchInfo_S *)VOS_Malloc( sizeof(PonSwitchInfo_S), MODULE_PON );
	if( PonSwitchData == NULL ){
		/*sys_console_printf("Error, Malloc buffer not satified ( switch_handler())\r\n"  );*/
		VOS_ASSERT( 0 );
		return( RERROR );
		}
    
	VOS_MemSet( (char *)PonSwitchData, 0, sizeof( PonResetInfo_S ) );
	PonSwitchData->olt_id       = olt_id;
	PonSwitchData->event_id     = (short int)switch_event;
	PonSwitchData->event_code   = (short int)code;
    PonSwitchData->new_status   = (short int)status;
    PonSwitchData->event_source = (unsigned short)source;
    PonSwitchData->slot_source  = (unsigned short)((0 < slot) ? slot : SYS_LOCAL_MODULE_SLOTNO);
    PonSwitchData->event_seq    = (unsigned short)seq;
    PonSwitchData->event_flags  = (unsigned short)flag;

	aulMsg[2] = sizeof( PonSwitchData );
	aulMsg[3] = ( int ) PonSwitchData;

	ret = VOS_QueSend( g_Pon_Queue_Id, aulMsg,  NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK ){
		VOS_ASSERT( 0 );
		VOS_Free( (void *)PonSwitchData );
		return( RERROR );
		}	
	return ( ROK );
	
}
/* E--added by liwei056@2010-1-26 for Pon-FastSwicthHover */
/*Begin:for onu swap by jinhl@2013-02-22*/
#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
int sendOnuSwitchEventMsg( short int olt_id, short int switch_event, PON_redundancy_msg_olt_failure_info_t  failure_info, int source, unsigned short seq, unsigned short flag)
{
	LONG  ret;
	unsigned long aulMsg[4] = { MODULE_PON, FC_ONUSWITCH, 0, 0 };
	OnuSwitchInfo_S   *OnuSwitchData = NULL;

	OnuSwitchData = (OnuSwitchInfo_S *)VOS_Malloc( sizeof(OnuSwitchInfo_S), MODULE_PON );
	if( OnuSwitchData == NULL ){
		sys_console_printf("Error, Malloc buffer not satified ( switch_handler())\r\n"  );
		VOS_ASSERT( 0 );
		return( RERROR );
		}
	VOS_MemSet( (char *)OnuSwitchData, 0, sizeof( OnuSwitchInfo_S ) );
	OnuSwitchData->olt_id       = olt_id;
	OnuSwitchData->event_id     = switch_event;
	OnuSwitchData->fail_info    = failure_info;
	OnuSwitchData->event_source = source;
    OnuSwitchData->event_seq    = seq;
    OnuSwitchData->event_flags  = flag;

	aulMsg[2] = sizeof( OnuSwitchInfo_S );
	aulMsg[3] = ( int ) OnuSwitchData;

	ret = VOS_QueSend( g_Pon_Queue_Id, aulMsg,  NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK ){
		VOS_ASSERT( 0 );
		VOS_Free( (void *)OnuSwitchData );
		return( RERROR );
		}	
	OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"sendOnuSwitchEventMsg over\r\n");
	
	return ( ROK );
	
}
#endif
/*End:for onu swap by jinhl@2013-02-22*/
/*****************************************************
 *
 *    Function:  LoadFileCompletedHandler( PonFileLoadCompleted_S *PonFileLoadResult )
 *
 *    Param:    
 *                 
 *    Desc:    this is a callback function; when loading DBA file to PAS5001 is completed, this function is called 
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int  LoadFileCompletedHandler( PonFileLoadCompleted_S *PonFileLoadResult )
{
	short int olt_id;
	short int return_code ;
	PON_binary_type_t  type;
	int CardSlot, CardPort;
	/*int ret;*/


	if( NULL == PonFileLoadResult )
	{
		VOS_ASSERT( 0 );
		return( RERROR );
	}

	olt_id = PonFileLoadResult->olt_id;
	type = PonFileLoadResult->type;
	return_code = PonFileLoadResult->return_code;

	if( !OLT_LOCAL_ISVALID(olt_id) ) return( RERROR );
	if((type < 0 ) || (type >= ARRAYSIZE(PonFileType))) return( RERROR );
    
	CardSlot = GetCardIdxByPonChip(olt_id);
	CardPort = GetPonPortByPonChip(olt_id);

	if(EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf("\r\npon%d/%d download file completed\r\n", CardSlot, CardPort);
		sys_console_printf("file type:'%s', ret code %d \r\n\r\n", PonFileType[type], return_code );
	}
	
	if( GetOltCardslotInserted( CardSlot ) == CARDNOTINSERT ) return( RERROR );

	/*Now, only external DAB dll file was handled */
	if( PON_OLT_BINARY_DBA_DLL == type )
	{
	    if( PAS_EXIT_OK == return_code )
	    {
#if 0            
            /* 在激活之前就启动DBA，使得可以恢复DBA配置 */
    	    return_code = PAS_start_dba_algorithm(olt_id, FALSE, 0, NULL);
            if ( PAS_EXIT_OK == return_code )
            {
                /* 测试发现:启动DBA后，即使没打开光口发送，也会有ONU注册。*/

                /*
                            modified by chenfj 2009-3-16
                            当监测到PON 口版本不匹配时，关闭该端口
                            */
                if(PonPortFirmwareAndDBAVersionMatch(olt_id) == V2R1_ENABLE)
                {
                    ShutdownPonPort(olt_id);
                    return_code = PAS_DEVICE_VERSION_MISMATCH_ERROR;
                }
            }
            else if ( PAS_DBA_ALREADY_RUNNING == return_code )
            {
        		/*** modified by chenfj 2009-1-7
            			返回值是PAS_DBA_ALREADY_RUNNING 的这样情况从逻辑推断上应该不会出现;
            			但为了程序的冗余判断, 也加上了
            			*/
                return_code = PAS_EXIT_OK;
            }
            else
            {
				/* 产生不匹配告警*/
				FlashAlarm_PonPortVersionMismatch(olt_id, PON_OLT_BINARY_DBA_DLL, V2R1_ENABLE);
                ShutdownPonPort(olt_id);
                return_code = PAS_DEVICE_VERSION_MISMATCH_ERROR;
            }

            if ( return_code == PAS_EXIT_OK )
#endif
            {
    			/*1 send external DBA dll file download completed trap to NMS */
    			Trap_DBALoad( olt_id, V2R1_LOAD_FILE_SUCCESS );

    			/*2 set status to 休眠*/
    			PonChipMgmtTable[olt_id].operStatus = PONCHIP_DORMANT;
    			/*PonPortTable[olt_id].PortWorkingStatus = PONPORT_UP;*/

                /* B--added by liwei056@2011-1-11 for SyncMasterStatus */
                if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
                {
                    OLT_SYNC_Data(olt_id, FC_ADD_PONPORT, NULL, 0);
                }
                /* E--added by liwei056@2011-1-11 for SyncMasterStatus */
    			
    			/*2' startup and activate the external DBA  */
    			if( PonChipActivatedFlag[CardSlot] == TRUE )
    			{
    				PonPortActivated( olt_id );
    			}
            }
		}
		
		if ( PAS_EXIT_OK != return_code )
		{        
			/*1 send external DBA dll file download failed trap to NMS 
			Trap_DBALoad( olt_id, V2R1_LOAD_FILE_FAILURE );*/
			if(PonChipMgmtTable[olt_id].operStatus != PONCHIP_ERR)
			{
				PonPortTable[olt_id].PortWorkingStatus  = PONPORT_DOWN;
				PonChipMgmtTable[olt_id].operStatus = PONCHIP_ERR;
				Trap_DBALoad( olt_id, V2R1_LOAD_FILE_FAILURE );
				sys_console_printf("\r\npon%d/%d download dba error %d\r\n", CardSlot, CardPort,return_code );
			}

            /* B--added by liwei056@2011-1-13 for SyncMasterStatus */
            if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
            {
                OLT_SYNC_Data(olt_id, FC_DEL_PONPORT, NULL, 0);
            }
            /* E--added by liwei056@2011-1-13 for SyncMasterStatus */
            
			return( RERROR );
			
			/*2 startup the internal DBA 
			PAS_start_dba_algorithm(olt_id, TRUE, 0, NULL );
			PonPortTable[olt_id].external_DBA_used  = FALSE;
			PonPortTable[olt_id].DBA_mode = OLT_INTERNAL_DBA;
			*/
		}
	}
	else
    {
        /* the pas5001 firmware file */
	}
	/*SetPonMsgDebug( olt_id, V2R1_DISABLE );*/

    return( ROK );
}

/*by jnhl*/
void OltDevResetSync(int olt_id)
{
	#if 0/*为了其它gpon板卡而改，如8100 by jinhl@2016.08.30*/
	if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
	#else
	if ( SYS_LOCAL_MODULE_TYPE_IS_8000_GPON )
	#endif
    {
        OLT_SYNC_Data(olt_id, FC_PONDEV_RESET, NULL, 0);
    }
	/*8100等同时是主控+PON板的情况，OLT_SYNC_Data发送会失败，所以增加如下分支以解决gpon芯片不自动重启的问题by jinhl@2016.09.08*/
	else if( SYS_LOCAL_MODULE_TYPE_IS_8100_GPON )
	{
		PonDevReset(olt_id);
	}
}

/*****************************************************
 *
 *    Function:  OnuRegisterHandler( ONURegisterInfo_S *OnuRegisterData)
 *
 *    Param:    
 *                 
 *    Desc:   this is a callback function; when onu register is happened, this function is called;
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
/* int test_debug = 0;*/
/*add by shixh20100913*/
int   Check_Onu_Is_In_PendingConf_Queue(short int PonPort_id, mac_address_t  mac_address)
{
    int rc = RERROR;
    pendingOnu_S *CurOnu, *NextOnu;

    VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
    CurOnu  = PonPortTable[PonPort_id].PendingOnu_Conf.Next;

    while( CurOnu != NULL )
	{			
		NextOnu = CurOnu->Next;

		if(MAC_ADDR_IS_EQUAL(CurOnu->OnuMarkInfor.OnuMark.MacAddr,mac_address) )
		{	
			if(CurOnu->otherPonIdx!=PonPort_id)   /*检查此MAC的ONU是否为原绑定PON口上注册过的ONU*/
			{
				/*sys_console_printf("CurOnu->otherPonIdx=%d,the  onu  in pending-conflict  queue!refused by register!!\r\n",CurOnu->otherPonIdx);*/
				rc = ROK;
				break;
			}
			else
			{
				sys_console_printf("the onu have registered in this pon port!!\r\n");
				break;
			}
		}
		CurOnu = NextOnu;
	}
    VOS_SemGive( OnuPendingDataSemId );

    return  rc;
}

int CheckOnuIsInPendingConfQueue(short int PonPort_id, mac_address_t mac_address)
{
    pendingOnu_S *CurOnu;

    VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
    CurOnu = PonPortTable[PonPort_id].PendingOnu_Conf.Next;
    while( CurOnu != NULL )
    {			
    	if(0 == VOS_MemCmp(CurOnu->OnuMarkInfor.OnuMark.MacAddr, mac_address, 6))
        {
            return 1;
        }   
        
        CurOnu = CurOnu->Next;
    }
    VOS_SemGive( OnuPendingDataSemId );

    return 0;
}

int CheckOnuIsInPendingQueue(short int PonPort_id, mac_address_t mac_address)
{
    int rc = 0;
    pendingOnu_S *CurOnu;

    VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
    CurOnu = PonPortTable[PonPort_id].PendingOnu.Next;
    while( CurOnu != NULL )
	{			
		if(0 == VOS_MemCmp(CurOnu->OnuMarkInfor.OnuMark.MacAddr, mac_address, 6))
		{
			rc = 1;
			break;
		}   
		CurOnu = CurOnu->Next;
	}
    VOS_SemGive( OnuPendingDataSemId );

    return  rc;
}

/* added by xieshl 20110509, PON板上在进行合法性检查时，同时删除其它PON下该mac地址的ONU */
extern ULONG g_onu_repeated_del_enable;
extern int g_mac_auth_result_failure;
LONG checkOnuRegisterControl( short int PonChipType, short int pon_id, short int llid, unsigned char *MacAddress )
{
	/*short int PonPortIdx, OnuIdx;
	short int onuEntryBase, onuEntry;
	CHAR mac[6];*/
	short int  binding_PonPort;
	LONG ret = ROK;
	int onu_idx = 0;
	/*int onuStatus;*/
	OnuRegisterDenied_S  RegisterDenied_Data;

	if( MacAddress == NULL ) return( RERROR );
    /*for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
    onu_idx = GetOnuIdxByLlid(pon_id, llid);
	#if 0/*for onu swap by jinhl@2013-02-22*/
	if(RERROR != onu_idx)
	{
	    return RERROR;
	}
	#endif
#if 0
	/* 判断该ONU  是否为不合法*/
	if( CheckIsInvalidOnu( pon_id, MacAddress ) == ROK )
	{ 
		if( EVENT_REGISTER == V2R1_ENABLE )
		{
			sys_console_printf("\r\n find invalid onu is registered\r\n" );
		}
		/* added by chenfj 2006/09/22 */ 
		/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
		/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/
        g_mac_auth_result_failure = V2R1_ENABLE;
	}
	else /*if( ret == RERROR )*/
	{
		if( EVENT_REGISTER == V2R1_ENABLE )
		{
			sys_console_printf("\r\n onu mac-auth is OK\r\n" );
		}
#else
	if( MAC_ADDR_IS_INVALID(MacAddress) )
	{
        if( EVENT_REGISTER == V2R1_ENABLE )
		{
			sys_console_printf("\r\n onu mac adress is invalid\r\n" );
		}
		return RERROR;
	}
#endif
		/* modified by xieshl 20110510, ONU-PON绑定和ONU MAC地址唯一检查执行过程相似，可合并，以提高执行效率，
		    注意，这两个功能是互斥的 */
#if 0	
		/* added by chenfj 2008-8-25
		    若使能了ONU 与PON 端口的绑定, 
		    则检查在别的PON 口下是否也有此ONU 的记录*/
		/* modified by xieshl 20110201, 向主控请求onu-pon绑定认证 */
		if(PONid_ONUMAC_BINDING == V2R1_ENABLE)
		{

			if( -1 != (binding_PonPort = OLTAdv_OnuIsBinding(pon_id, MacAddress)) )
			{
				AddPendingOnu_conf( pon_id, onu_id , MacAddress, binding_PonPort);
				/*sys_console_printf( "\r\n onu-pon binding:pon%d/%d,mac=%02x%02x.%02x%02x.%02x%02x conflict witch onu%d/%d/%d\r\n",
					slot, port,
					OnuRegisterData->mac_address[0], OnuRegisterData->mac_address[1], OnuRegisterData->mac_address[2], 
					OnuRegisterData->mac_address[3], OnuRegisterData->mac_address[4], OnuRegisterData->mac_address[5],
					GetGlobalCardIdxByPonChip(binding_PonPort), GetGlobalPonPortByPonChip(binding_PonPort), onu_id+1 );*/
				
				RegisterDenied_Data.olt_id = pon_id;
				VOS_MemCpy( RegisterDenied_Data.Onu_MacAddr, MacAddress, BYTES_IN_MAC_ADDRESS );
				OnuRegisterDeniedHandler( &RegisterDenied_Data);
				return RERROR;
			}					
			if( EVENT_REGISTER == V2R1_ENABLE )
			{
				sys_console_printf("\r\n onu-pon binding is OK\r\n" );
			}
		}			

		for( PonPortIdx=0; PonPortIdx< MAXPON; PonPortIdx++ )
		{
			if( pon_id == PonPortIdx )
				continue;

			onuEntryBase = PonPortIdx * MAXONUPERPON;
			for( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++ )
			{
				onuEntry = onuEntryBase + OnuIdx;

				ONU_MGMT_SEM_TAKE;
				VOS_MemCpy( mac, OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, 6 );
				onuStatus = OnuMgmtTable[onuEntry].OperStatus;
				ONU_MGMT_SEM_GIVE;
				
				if( MAC_ADDR_IS_UNEQUAL(mac, MacAddress) )
					continue;
				
				/* modified by xieshl 20110322, 解决无法检出ONU重复注册的问题 */
				if( ((onuStatus == ONU_OPER_STATUS_UP) || (onuStatus == ONU_OPER_STATUS_PENDING) || (onuStatus == ONU_OPER_STATUS_DORMANT) )
#ifdef  PON_PORT_HOT_SWAP
					&& (ThisIsHotSwapPort(PonPortIdx, pon_id ) == V2R1_ISNOT_HOT_SWAP_PORT)
#endif
				  ) 
				{
					if( EVENT_REGISTER == V2R1_ENABLE )
					{
						sys_console_printf("\r\n find onu has been registeded in pon%d/%d before\r\n", \
							GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
					}
					/* send trap to NMS: ONU注册冲突 */
					Trap_OnuRegisterConflict(PonPortIdx, OnuIdx/*, PonPort_id, OnuRegisterData->mac_address*/ );

					/* deny this Onu */
					if( PonChipType == PONCHIP_PAS )
					{
						PAS_authorize_onu ( pon_id,  onu_id, PON_DENY_FROM_THE_NETWORK );
					}
				
					/*PAS_shutdown_onu( PonPort_id, onu_id, TRUE );

					add by chenfj 2006/10/11 */
					if(PonPortIdx != pon_id )
					{
						AddPendingOnu_conf( pon_id, onu_id , MacAddress, PonPortIdx);
					}
					else
					{
						AddPendingOnu(pon_id, onu_id, MacAddress);
					}

					ret = RERROR;
				}
				else
				{
					DelOnuFromPonPort( PonPortIdx, OnuIdx );
				}
			}
		}
#else
		/* modified by xieshl 20110602, PON口注册64个ONU，多次切换PON口，ONU注册会出现异常，问题单12971 */
		if( (PONid_ONUMAC_BINDING == V2R1_ENABLE) || (g_onu_repeated_del_enable) )
		{
			binding_PonPort = OLT_ID_INVALID;
			if( ROK != OLTAdv_ChkOnuRegisterControl(pon_id, llid, MacAddress, &binding_PonPort) )
			{
				if( binding_PonPort != OLT_ID_INVALID )
					AddPendingOnu_conf( pon_id, llid , MacAddress, binding_PonPort);
				else
					AddPendingOnu(pon_id, -1, llid, MacAddress, 0);
				
				RegisterDenied_Data.olt_id = pon_id;
				VOS_MemCpy( RegisterDenied_Data.Onu_MacAddr, MacAddress, BYTES_IN_MAC_ADDRESS );
				OnuRegisterDeniedHandler( &RegisterDenied_Data);
				return RERROR;
			}
			if(PONid_ONUMAC_BINDING == V2R1_ENABLE)
			{
				if( EVENT_REGISTER == V2R1_ENABLE )
				{
					sys_console_printf("\r\n onu-pon binding is OK\r\n" );
				}
			}			
		}
		
#endif
		if( ret == ROK )
		{
			if( (MAKEING_TEST_FLAG == V2R1_DISABLE) && (PonPortTable[pon_id].AutoRegisterFlag == V2R1_ENABLE) )	/* onu注册限制目前未用 */
			{
				if( EVENT_REGISTER ==V2R1_ENABLE)
				{
					sys_console_printf("\r\n find onu is deny for limit-register\r\n");
				}
			
				/* deny this Onu */
				if( PonChipType == PONCHIP_PAS )
				{
				    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
					OnuMgt_AuthorizeOnu ( pon_id,  onu_idx, FALSE );
				}
			
				/* added by chenfj 2006/09/22 */ 
				/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
				/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/
				
				AddPendingOnu( pon_id, -1, llid, MacAddress, 0);
				ret = RERROR;
			}	
		}

#if 0				
		if( EVENT_REGISTER==V2R1_ENABLE )
			sys_console_printf("\r\n  Authorize this onu ...");
			
		if( PAS_authorize_onu ( PonPort_id,  onu_id, PON_AUTHORIZE_TO_THE_NETWORK ) == PAS_EXIT_OK ){
			if( EVENT_REGISTER == V2R1_ENABLE )
				sys_console_printf(" ok \r\n");
			}
		else {
			if( EVENT_REGISTER == V2R1_ENABLE)
				sys_console_printf(" err \r\n");
			return( RERROR );
			}
#endif

	
	return ret;
}

/* B--added by liwei056@2011-10-11 for RegisterBW */
int SetOnuRegisterDefaultDownlinkBW(short int olt_id, short int llid)
{
    PON_policing_parameters_t BW;
    
    VOS_MemZero(&BW, sizeof(BW));

    return OLTAdv_SetLLIDPolice(olt_id, llid, PON_POLICER_DOWNSTREAM_TRAFFIC, TRUE, BW);
}

int SetOnuRegisterDefaultUplinkBW(short int olt_id)
{
#if 0/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
    int iRlt;
    PLATO3_SLA_t BW;
    short int dba_error;
    
    VOS_MemZero(&BW, sizeof(BW));
    BW.max_gr_bw_fine = 1;
    BW.max_be_bw_fine = 1;

    iRlt = OLTAdv_SetLLIDSLA(olt_id, PON_DISCOVERY_LLID, &BW, &dba_error);
#else
    int iRlt = RERROR;
    unsigned short max_gr_bw_fine = 0;
	unsigned short max_be_bw_fine = 0;
    short int dba_error;
    
    max_gr_bw_fine = 1;
    max_be_bw_fine = 1;

    iRlt = Pon_SetLLIDSLA(olt_id, PON_DISCOVERY_LLID, max_gr_bw_fine, max_be_bw_fine, &dba_error);
#endif
    if ( 0 == iRlt )
    {
        if ( dba_error > 0 )
        {
            iRlt = -dba_error;
        }
        else
        {
            iRlt = dba_error;
        }
    }

    return iRlt;
}

int SetOnuPendingDefaultBW(short int olt_id, short int llid)
{
    int iRlt;

    /* 下行默认不占用带宽 */    
    iRlt = SetOnuRegisterDefaultDownlinkBW(olt_id, llid);

    /* 上行默认不占用带宽 */    
    {
#if 0/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        PLATO3_SLA_t BW;
        short int dba_error;

        VOS_MemZero(&BW, sizeof(BW));
#if 1
        /* ONU上行带宽设为0，则离线，最小64K的管理信道 */
        BW.max_gr_bw_fine = 1;
        BW.max_be_bw_fine = 1;
#endif
        iRlt = OLTAdv_SetLLIDSLA(olt_id, llid, &BW, &dba_error);
#else
        
	    unsigned short max_gr_bw_fine = 0;
		unsigned short max_be_bw_fine = 0;
	    short int dba_error;
	    
	    max_gr_bw_fine = 1;
	    max_be_bw_fine = 1;

	    iRlt = Pon_SetLLIDSLA(olt_id, llid, max_gr_bw_fine, max_be_bw_fine, &dba_error);
#endif
        if ( 0 == iRlt )
        {
            if ( dba_error > 0 )
            {
                iRlt = -dba_error;
            }
            else
            {
                iRlt = dba_error;
            }
        }
    }

    return iRlt;
}
/* E--added by liwei056@2011-10-11 for RegisterBW */


LONG (*assign_onuBwBasedMac_hookrtn)(SHORT onuMgtIdx, UCHAR * pMacAddr) = NULL;
int OnuRegisterHandler( ONURegisterInfo_S *OnuRegisterData )
{
	PON_olt_id_t  PonPort_id; 
	PON_llid_t onu_id;
	PON_onu_id_t onu_idx, standby_onu_idx;
	int vendorType = ONU_VENDOR_GW;
	unsigned int event_flags;

	int OltProtectMode;
	int ret, i,j;
	int iOnuBWSetRlt;
	short int  PonChipType;
	short int  PonChipVer;
	short int  PonChipIdx_swap;	
	short int  reg_flag = 0;
	int  OnuEntry;

	int slot, port;
	int oam_status, traffic_enable;

	if( OnuRegisterData == NULL )
	{
		VOS_ASSERT( 0 );
		return( RERROR );
	}

	/* 1: extract the onu register parameter from the buffer */
	PonPort_id = OnuRegisterData->olt_id;
	onu_id = OnuRegisterData->onu_id;
	event_flags = OnuRegisterData->event_flags;
    
	if( (!OLT_LOCAL_ISVALID(PonPort_id) ) || (!LLID_ISVALID(onu_id)) )
	{
		if( EVENT_REGISTER == V2R1_ENABLE )
		{
			sys_console_printf("\r\n ONU Reg. unknown LLID (pon=%d,llid=%d)\r\n", PonPort_id, onu_id );
		}
		return( RERROR );
	}

	/* modified by xieshl 20111103, 解决PON LOS clear告警漏报问题，如果ONU是第一个注册则上报一次，问题单13210 */
	/*if( PonPortTable[PonPort_id].SignalLossFlag == V2R1_ENABLE )
	{
		if( OnRegOnuCounter(PonPort_id) == 0 )
		{
			Trap_PonPortSignalLoss( PonPort_id, V2R1_DISABLE );
		}
	}*/	/* modified by xieshl 20120620，解决误报问题，去掉后会重新引入漏报，问题单14114 */

	slot = GetCardIdxByPonChip( PonPort_id );
	port = GetPonPortByPonChip( PonPort_id );
	if( (!OLT_SLOT_ISVALID(slot)) || (!OLT_PORT_ISVALID(port)) )
	{
		if( EVENT_REGISTER == V2R1_ENABLE )
		{
			sys_console_printf("\r\n ONU Reg. unknown slot%d, port%d (pon=%d,llid=%d)\r\n", slot, port, PonPort_id, onu_id );
		}
		VOS_ASSERT(0);
		return( RERROR );
	}

	if(EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf("\r\n onu %d/%d/llid(%d) registration, mac[%02x%02x.%02x%02x.%02x%02x], event_flags(%d).\r\n", slot, port, onu_id,
		                OnuRegisterData->mac_address[0], OnuRegisterData->mac_address[1], OnuRegisterData->mac_address[2], 
						OnuRegisterData->mac_address[3], OnuRegisterData->mac_address[4], OnuRegisterData->mac_address[5],
                        event_flags );

    /* B--modified by liwei056@2011-07-19 for D12860 */
#if 0
    /* B--added by liwei056@2010-10-19 for Redundancy-RegisterBUG */
    if ( ROK != CheckIsInvalidOnuId(PonPort_id, onu_id, OnuRegisterData->mac_address) )
    {
        if ( ROK != CheckIsInvalidOnuId(PonPort_id, onu_id, OnuRegisterData->mac_address) )
        {
            VOS_ASSERT(0);
        }
    }
    /* E--added by liwei056@2010-10-19 for Redundancy-RegisterBUG */
#else
    /* 确保此LLID已经无其它ONU占用 */
    while ( RERROR != (onu_idx = GetOnuIdxByLlid(PonPort_id, onu_id)) )
    {
        int OnuStatus;
        bool DoubleRegFlag;
        
        OnuEntry = PonPort_id * MAXONUPERPON + onu_idx;

        ONU_MGMT_SEM_TAKE;
        OnuStatus = OnuMgmtTable[OnuEntry].OperStatus;
        DoubleRegFlag = MAC_ADDR_IS_EQUAL(OnuRegisterData->mac_address, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr);
        ONU_MGMT_SEM_GIVE;

        if ( DoubleRegFlag )
        {
            /* 注册的LLID已被ONU自己占用 */
            if( ONU_OPER_STATUS_DOWN == OnuStatus )
            {
                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d resume onu%d's real registering status by %s-register.\r\n", PonPort_id, onu_idx+1, (event_flags & ONU_EVENTFLAG_REAL) ? "real" : "virtual");
                break;
            }
            else
            {
                if (event_flags & ONU_EVENTFLAG_REAL)
                {
                    if ( ONU_OPER_STATUS_UP == OnuStatus )
                    {
                        ONUDeregistrationInfo_S stDeregEvt;

                        /* 真实注册的LLID已被ONU自己占用，则需重新注册此LLID */
                        OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event by double-register(llid%d).\r\n", PonPort_id, onu_idx+1, onu_id);
                        stDeregEvt.olt_id = PonPort_id;
                        stDeregEvt.onu_id = onu_id;
                        stDeregEvt.deregistration_code = PON_ONU_DEREGISTRATION_DOUBLE_REGISTRATION;
                        stDeregEvt.event_flags = ONU_EVENTFLAG_VIRTAL;
                        OnuDeregisterHandler(&stDeregEvt);
                    }
                    /* B--added by liwei056@2012-7-23 for D15483 */
                    else
                    {
                        OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d clear onu%d's virtual registering status by double-register(llid%d).\r\n", PonPort_id, onu_idx+1, onu_id);
                        ClearOnuRunningData( PonPort_id, onu_idx, 0 );
                    }
                    /* E--added by liwei056@2012-7-23 for D15483 */
                }
                else
                {
                    /* 虚拟注册的LLID已经UP，则此虚拟注册无效 */
                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d loose llid%d's virtual register event for onu%d have register the llid by real-register.\r\n", PonPort_id, onu_id, onu_idx+1);
            		return( RERROR );
                }
            }
        }
        else
        {
            /* 注册的LLID被其他ONU占用 */
            if (event_flags & ONU_EVENTFLAG_REAL)
            {
                /* 真实注册的LLID被别的ONU占用，则需抢夺此LLID */
                if( ONU_OPER_STATUS_UP == OnuStatus )
                {
                    ONUDeregistrationInfo_S stDeregEvt;

                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event by real-register(llid%d).\r\n", PonPort_id, onu_idx+1, onu_id);
                    stDeregEvt.olt_id = PonPort_id;
                    stDeregEvt.onu_id = onu_id;
                    stDeregEvt.deregistration_code = PON_ONU_DEREGISTRATION_DOUBLE_REGISTRATION;
                    stDeregEvt.event_flags = ONU_EVENTFLAG_VIRTAL;
                    OnuDeregisterHandler(&stDeregEvt);
                }
                else
                {
                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d clear onu%d's virtual registering status by real-register(llid%d).\r\n", PonPort_id, onu_idx+1, onu_id);
                    ClearOnuRunningData( PonPort_id, onu_idx, 0 );
                }
            }
            else
            {
                /* 虚拟注册的LLID被别的ONU占用，则此虚拟注册无效 */
                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d loose llid%d's virtual register event for onu%d have occupied the llid by real-register.\r\n", PonPort_id, onu_id, onu_idx+1);
        		return( RERROR );
            }
        }
    }
#endif
    /* E--modified by liwei056@2011-07-19 for D12860 */

#if 0
	if((PAS_get_onu_mode( PonPort_id, onu_id) != PON_ONU_MODE_PENDING) && (PAS_get_onu_mode( PonPort_id, onu_id) != PON_ONU_MODE_ON ))
	{
		if( EVENT_REGISTER == V2R1_ENABLE )
			sys_console_printf("  Onu(%02x%02x.%02x%02x.%02x%02x-llid %d) register,but not on-line\r\n",mac_address[0],mac_address[1],mac_address[2],mac_address[3],mac_address[4],mac_address[5], onu_id);
		return(RERROR);
	}

	/* 用于判断这个注册消息是否是一个积攒下未能及时处理而过期的消息*/
	{
		PON_llid_parameters_t llid_parameters;
		if(PAS_get_llid_parameters( PonPort_id, onu_id, &llid_parameters) != ROK )
			return(RERROR);
		if(CompTwoMacAddress(mac_address, llid_parameters.mac_address) != ROK )
			return(RERROR);
	}
#endif		

	onu_idx         = -1;
	OnuEntry        = -1;
	PonChipIdx_swap = -1;
	standby_onu_idx = -1;

	/* 当PON 端口使能了保护切换，处于备用状态的PON 端口不处理ＯＮＵ注册*/
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
	/* clear pon port swap counter */	
	if( PonPortSwapEnableQuery( PonPort_id ) == V2R1_PON_PORT_SWAP_ENABLE )
	{
        OltProtectMode = GetPonPortHotSwapMode( PonPort_id );
        
        if ( PON_SWAPMODE_ISOLT(OltProtectMode) )
        {
    		if( PonPortHotStatusQuery( PonPort_id ) == V2R1_PON_PORT_SWAP_PASSIVE )
    		{
    			if( (V2R1_ENABLE == EVENT_PONSWITCH) || (EVENT_REGISTER == V2R1_ENABLE) )
    			{
    				sys_console_printf("\r\n find onu is registered on the passive pon\r\n" );
    			}
    			AddPendingOnu( PonPort_id, -1, onu_id, OnuRegisterData->mac_address, PENDING_REASON_CODE_PON_PASSIVE );
    			/*PAS_deregister_onu(PonPort_id,onu_id, FALSE);*/
    			return( RERROR );
    		}
    		else
    		{
    			if ( V2R1_PON_PORT_SWAP_QUICKLY == OltProtectMode )
    			{
    				PonPortSwapPortQuery(PonPort_id, &PonChipIdx_swap);
    			}
    		}
        }
	}
#endif	


	/*PonChipIdx = GetPonChipIdx( PonPort_id );*/
	/*判断此ONU是否在PENDING-CONF 队列中，如果在，立刻退出注册处理流程*/
	if(PONid_ONUMAC_BINDING == V2R1_ENABLE)
	{
		if(Check_Onu_Is_In_PendingConf_Queue(PonPort_id, OnuRegisterData->mac_address)==ROK)
		{
			sys_console_printf( "\r\n find onu-pon binding:pon%d/%d,mac %02x%02x.%02x%02x.%02x%02x conflict pending\r\n",
						slot, port,
						OnuRegisterData->mac_address[0], OnuRegisterData->mac_address[1], OnuRegisterData->mac_address[2], 
						OnuRegisterData->mac_address[3], OnuRegisterData->mac_address[4], OnuRegisterData->mac_address[5] );

			return ROK;
		}
	}   

	/* 2: judge whether onu authentication should be handled manually */ 

	PonChipType = GetPonChipTypeByPonPort(PonPort_id );
	
	
	/* modified by xieshl 20110509, PON板上在进行合法性检查时，同时删除其它PON下该mac地址的ONU */
	if( checkOnuRegisterControl( PonChipType, PonPort_id, onu_id, OnuRegisterData->mac_address) == VOS_ERROR )
		return VOS_ERROR;

	/* modified by xieshl 20110610, 将ONU重新注册检查和空闲ONU检查合并，提高onu-list满时的检索效率 */
	onu_idx = SearchFreeOnuIdxForRegister(PonPort_id, OnuRegisterData->mac_address, &reg_flag);
	if( onu_idx == RERROR )	/* 注册满 */
	{
		/* modify by chenfj 2006/09/21 */
		/*#2606 问题在手工添加64个ONU后，再实际注册新ONU时，出现异常*/
		if( EVENT_REGISTER == V2R1_ENABLE )
			sys_console_printf("\r\n no g_free entry for this onu(pon=%d/%d,llid=%d\r\n", slot, port, onu_id );
		
		/* LOG info : too many onu registered */

		if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			OnuMgt_AuthorizeOnu(PonPort_id, onu_idx, FALSE );
			/*PAS_shutdown_onu( PonPort_id, onu_id, TRUE );*/
		}
		
		/* added by chenfj 2006/09/22 */ 
		/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
		/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/
		
		AddPendingOnu( PonPort_id, -1, onu_id, OnuRegisterData->mac_address, 0);

		/* added by chenfj 2009-5-14 , 当这个PON 口下有多于 64个ONU 时，产生告警*/
		Trap_PonPortFull(PonPort_id);
		
		return( RERROR );
	}
	else
	{
		OnuEntry = PonPort_id * MAXONUPERPON + onu_idx;
		if( OnuEntry < 0 || OnuEntry > MAXONU )
		{
			VOS_ASSERT(0);
			AddPendingOnu( PonPort_id, -1, onu_id, OnuRegisterData->mac_address, 0);
			return RERROR;
		}

		if( EVENT_REGISTER == V2R1_ENABLE )
			sys_console_printf("\r\n onu %d/%d/%d is %s-register, entry is %d \r\n", slot, port, (onu_idx+1), (reg_flag ? "new" : "re"), OnuEntry);

		if( reg_flag == 0 )		/* 0-re-register, 1-new register, 2-replaced */
		{
			ONU_MGMT_SEM_TAKE;

			/* B--added by liwei056@2011-3-14 for CodeCheck */
			if( ONU_OPER_STATUS_UP == OnuMgmtTable[OnuEntry].OperStatus )
			{
			    ONUDeregistrationInfo_S stDeregEvt;

			    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event for double-register.\r\n", PonPort_id, onu_idx+1);
			    stDeregEvt.olt_id = PonPort_id;
			    stDeregEvt.onu_id = OnuMgmtTable[OnuEntry].LLID;
			    stDeregEvt.deregistration_code = PON_ONU_DEREGISTRATION_DOUBLE_REGISTRATION;
                stDeregEvt.event_flags = ONU_EVENTFLAG_VIRTAL;
			    OnuDeregisterHandler(&stDeregEvt);
			}
			/* E--added by liwei056@2011-3-14 for CodeCheck */

			ONU_MGMT_SEM_GIVE;
		}
		else			/* 1-new register, 2-replaced */
		{
			/* modified by xieshl 20110512, 覆盖旧ONU时上报告警 */
			if( reg_flag == 2 )	/* 0-re-register, 1-new register, 2-replaced */
			{
				ONU_MGMT_SEM_TAKE;
				onuDeletingNotify_EventReport( OLT_DEV_ID, slot, port, (onu_idx+1), OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr );
				ONU_MGMT_SEM_GIVE;

				clearDeviceAlarmStatus( MAKEDEVID(slot, port, (onu_idx+1)) );
				InitOneOnuByDefault(PonPort_id, onu_idx);
			}

			ONU_MGMT_SEM_TAKE;
	    		VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, OnuRegisterData->mac_address, BYTES_IN_MAC_ADDRESS );

	    		/* add by chenfj 2006-10-30  */
	    		/*added by chenfj 2006-12-21 
	    	   	 设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参
	    		*/
				GetOnuDefaultBWByPonRate(PonPort_id, onu_idx, OLT_CFG_DIR_UPLINK, &OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_gr, &OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_be);
				GetOnuDefaultBWByPonRate(PonPort_id, onu_idx, OLT_CFG_DIR_DOWNLINK, &OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkBandwidth_gr, &OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkBandwidth_be);
#ifdef PLATO_DBA_V3
	    		OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_fixed = 0;
#endif
	    		OnuMgmtTable[OnuEntry].LlidTable[0].UplinkClass = OnuConfigDefault.UplinkClass;
	    		OnuMgmtTable[OnuEntry].LlidTable[0].UplinkDelay = OnuConfigDefault.UplinkDelay;
	    		OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkClass = OnuConfigDefault.DownlinkClass;
	    		OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkDelay = OnuConfigDefault.DownlinkDelay;

			ONU_MGMT_SEM_GIVE;
		}
	}

	/* modified by xieshl 20111102, 记录不是很准确，不能反映部分pending ONU的情况，后续可废弃 */
	PonPortTable[PonPort_id].CurrOnu ++;
	PonPortTable[PonPort_id].CurLLID ++;

	i = onu_idx & 0x07; /*onu_idx % 8*/;
	j = onu_idx >> 3; /*onu_idx / 8;*/
	PonPortTable[PonPort_id].OnuCurStatus[j] = PonPortTable[PonPort_id].OnuCurStatus[j] |( 1 << i ) ;


	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[OnuEntry].OperStatus = ONU_OPER_STATUS_DORMANT;
	VOS_MemCpy( OnuMgmtTable[OnuEntry].SequenceNo, OnuRegisterData->authenticatioin_sequence, PON_AUTHENTICATION_SEQUENCE_SIZE );
	OnuMgmtTable[OnuEntry].OAM_Ver = OnuRegisterData->supported_oam_standard;
	OnuMgmtTable[OnuEntry].LLID  = onu_id;
	OnuMgmtTable[OnuEntry].LlidTable[0].Llid = onu_id;
	OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus = LLID_ENTRY_NOT_IN_ACTIVE;

	OnuMgmtTable[OnuEntry].Index.slot = slot;
	OnuMgmtTable[OnuEntry].Index.port = port;
	OnuMgmtTable[OnuEntry].Index.subNo = /* MAXLLID + */onu_id;
	OnuMgmtTable[OnuEntry].Index.Onu_FE = 0;
    
	VOS_MemCpy( (void * )OnuMgmtTable[OnuEntry].EncryptKey, (void *)OnuRegisterData->mac_address, BYTES_IN_MAC_ADDRESS );

	oam_status = OnuMgmtTable[OnuEntry].ExtOAM;
	traffic_enable = OnuMgmtTable[OnuEntry].TrafficServiceEnable;
	ONU_MGMT_SEM_GIVE;

	/*  for PAS-SOFT5201  V5.1 */
	vendorType = GetOnuVendorType( PonPort_id, onu_idx );
	if( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf("\r\n onu %d/%d/%d's vendor is %s\r\n", slot, port, (onu_idx + 1), (vendorType == ONU_VENDOR_GW) ? "GW" : ( (vendorType == ONU_VENDOR_CT) ? "CTC": "UNKNOWN" ) );
	}
	if( vendorType == ONU_VENDOR_CT )
	{
		if( reg_flag == 0 )
		{
			/*SetOnuOperStatus( PonPort_id, onu_idx ,ONU_OPER_STATUS_UP);*/ /* modified by xieshl 20101220, 问题单11740 */
			updateOnuOperStatusAndUpTime( OnuEntry, ONU_OPER_STATUS_UP );
			/*OnuMgmtTable[OnuEntry].OperStatus = ONU_OPER_STATUS_UP;*/
		}
		else
		{
			RecordOnuUpTime( OnuEntry );
		}
		
 		ONU_MGMT_SEM_TAKE;
   		OnuMgmtTable[OnuEntry].OperStatus = ONU_OPER_STATUS_UP;
		if(OnuMgmtTable[OnuEntry].UsedFlag != ONU_USED_FLAG)
		{
			/*setOnuStatus( slot,  port, onu_idx, ONU_ONLINE );*/
    			OnuMgmtTable[OnuEntry].UsedFlag = ONU_USED_FLAG;
		}
		OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus =  LLID_ENTRY_ACTIVE;
		ONU_MGMT_SEM_GIVE;

		/*
		GetOnuDeviceVersion( PonPort_id, 	(OnuEntry % MAXONUPERPON ));		
		GetOnuRegisterData( PonPort_id, (OnuEntry % MAXONUPERPON ));
		*/
	}

	/* B--added by liwei056@2011-1-30 for CodeCheck  */
	/* CTC发现未成功之前，都是GW接口 */
	if( oam_status != V2R1_ENABLE )	
	/* E--added by liwei056@2011-1-30 for CodeCheck */
	{
		/* add 20100823,ONU管理接口挂接*/
		ONU_SetupIFsByType(PonPort_id, onu_idx, PonChipType, ONU_MANAGE_GW);  /* modied by shixh 20101117  问题单11170 */
    	if( EVENT_REGISTER == V2R1_ENABLE )
        {
            sys_console_printf("\r\n onu %d/%d/%d get the LOCAL-GW-ONU service at the OnuRegister time.\r\n", slot, port, (onu_idx + 1) );
        }   
	}

	/* 判断ONU业务是否去激活，若去激活，则不让ONU注册*/
	if( traffic_enable == V2R1_DISABLE )
	{
		if( EVENT_REGISTER == V2R1_ENABLE)
			sys_console_printf(" onu %d/%d/%d service deactive\r\n", slot, port, (onu_idx + 1));
		
		AddPendingOnu( PonPort_id, onu_idx, onu_id, OnuRegisterData->mac_address, 0);
		
		/*REMOTE_PASONU_uni_set_port( PonPort_id, onu_id, 1, 0 );*/
		/*SetOnuOperStatus(PonPort_id,(OnuEntry % MAXONUPERPON), ONU_OPER_STATUS_DOWN);*/
		return( RERROR );					
	}		

	/* 4: Pon user data table */
	OnLineOnuCounter( PonPort_id );
    
	/*UpdateProvisionedBWInfo( PonPort_id );*/
#if 0
	if( (PonPortTable[PonPort_id].CurrOnu+1) > PonPortTable[PonPort_id].MaxOnu )
		{
		if( EVENT_REGISTER == V2R1_ENABLE)
			sys_console_printf("\r\n  Too many onu registered in %s/port%d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPort_id)], GetPonPortByPonChip(PonPort_id) );
		if( PonChipType == PONCHIP_PAS )
			{
			PAS_authorize_onu(PonPort_id, onu_id, PON_DENY_FROM_THE_NETWORK );
			/*PAS_shutdown_onu( PonPort_id, onu_id, TRUE );*/
			}
		else{

			}

		/*PonPortTable[PonPort_id].CurrOnu --;
		PonPortTable[PonPort_id].CurLLID --;*/

		/* add this onu to pending Queue */
		AddPendingOnu( PonPort_id, onu_id );
		
		/* send trap to NMS : too many onu registered */
		
		return( RERROR );
		}
#endif


	/* modified by xieshl 20110613, 上层API需要获取LLID，注册处理这样调用有风险，而且效率低，同时这本来是底层驱动回调的函数，直接调pas驱动即可 */
	/* ONU认证入网后，一些配置才能下发 */
	/*if( AuthorizeOnuToNetwork(PonPort_id, onu_idx) != ROK ) return(RERROR);
	DisableOnuOamSlowProtocolLimit(PonPort_id, onu_idx);*/
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_AuthorizeOnu ( PonPort_id,  onu_idx, TRUE );
	if( EVENT_REGISTER== V2R1_ENABLE )
		sys_console_printf("\r\n Authorize this onu(%d/%d/%d) ... %s", slot, port, onu_idx+1, (ret == PAS_EXIT_OK ? "ok" : "err") );
	if( ret != PAS_EXIT_OK )
	{
		AddPendingOnu( PonPort_id, onu_idx, onu_id, OnuRegisterData->mac_address, 0);
		return( RERROR );
	}
	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	OnuMgt_SetSlowProtocolLimit(PonPort_id, onu_idx, DISABLE);
	#else
	REMOTE_PASONU_set_slow_protocol_limit(PonPort_id, onu_id, DISABLE);
	#endif
    /*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/

	/* 限制ONU在OLT上占用的MAC地址表资源 */
	SetOnuMaxMacNum( PonPort_id, onu_idx, 0, OnuMgmtTable[OnuEntry].LlidTable[0].MaxMAC );

	/* 分配ONU在OLT上占用的上下行带宽资源 */
	/* modified by chenfj 2006/11/1
		ONU注册后，先分配带宽，仅用于测试#3026.   */
	/** 9 modified by chenfj 2007-03-25
 	 **	#3907 问题单: 关于ONU带宽分配的两个问题
 	 **  调试时将带宽激活关闭了；现在打开
 	 **/
	/*5: set the corresponding LLID bandwidth & policeing parameters */
	/*  for PAS-SOFT5201  V5.1 */
	/* if( vendorType != ONU_VENDOR_CT ) */

	/* modified by xieshl 20110914, 需求11112, ONU注册时优先取基于MAC地址的带宽配置，同时覆盖原基于ONU ID的配置 */
	if( (assign_onuBwBasedMac_hookrtn ? (*assign_onuBwBasedMac_hookrtn)(OnuEntry, OnuRegisterData->mac_address) : VOS_OK) == VOS_OK )
	{
		/* B--modified by liwei056@2009-11-12 for D9143 */
		iOnuBWSetRlt = ActiveOnuDownlinkBW( PonPort_id,  onu_idx);
		if (V2R1_EXCEED_RANGE != iOnuBWSetRlt)
		{
			short int iCurrOperStatus;
			short int iCurrRegStatus;

			ONU_MGMT_SEM_TAKE;
			iCurrOperStatus = OnuMgmtTable[OnuEntry].OperStatus;
			iCurrRegStatus  = OnuMgmtTable[OnuEntry].RegisterTrapFlag;
			ONU_MGMT_SEM_GIVE;

			/* B--added by liwei056@2010-1-15 for D9550 */
			if( (EVENT_REGISTER == V2R1_ENABLE) && (0 != iOnuBWSetRlt) )
				sys_console_printf( " onu %d/%d/%d failed to set dwlink-bw (err_code=%d)\r\n", slot, port, onu_idx+1, iOnuBWSetRlt );
			/* E--added by liwei056@2010-1-15 for D9550 */

			/* 即使带宽设置失败，也继续注册 */
			if ( 0 != iOnuBWSetRlt )
			{
				if ( NO_ONU_REGISTER_TRAP == iCurrRegStatus )
				{
					/* 之前未上报过注册事件，*/
					/* 直接清空其注册数据 */
					if ( reg_flag )
					{
						/* 清空其配置记录 */
						InitOneOnuByDefault(PonPort_id, onu_idx);
					}
					else
					{
						/* 清空其注册状态 */
						ClearOnuRunningData(PonPort_id, onu_idx, 0);
					}
				}
				else
				{
					/* 之前上报过注册事件，*/
					/* 要保留注册数据来上报离线事件，*/
					/* 并由其清空注册数据 */
				}

				/* 带宽设置失败，则令其重新注册 */
				#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
				OLT_DeregisterLLID(PonPort_id, onu_id, FALSE);
				#else
				PAS_deregister_onu(PonPort_id, onu_id, FALSE);
				#endif

				return( RERROR );					
			}

			iOnuBWSetRlt = ActiveOnuUplinkBW( PonPort_id,  onu_idx);
			if (V2R1_EXCEED_RANGE != iOnuBWSetRlt)
			{
				/* B--added by liwei056@2010-1-15 for D9550 */
				if( (EVENT_REGISTER == V2R1_ENABLE) && (0 != iOnuBWSetRlt) )
					sys_console_printf( " onu %d/%d/%d failed to set uplink-bw(err_code=%d)\r\n", slot, port, onu_idx+1, iOnuBWSetRlt );
				/* E--added by liwei056@2010-1-15 for D9550 */

				if ( 0 != iOnuBWSetRlt )
				{
					if ( NO_ONU_REGISTER_TRAP == iCurrRegStatus )
					{
						/* 之前未上报过注册事件，*/
						/* 直接清空其注册数据 */
						if ( reg_flag )
						{
							/* 清空其配置记录 */
							InitOneOnuByDefault(PonPort_id, onu_idx);
						}
						else
						{
							/* 清空其注册状态 */
							ClearOnuRunningData(PonPort_id, onu_idx, 0);
						}
					}
					else
					{
						/* 之前上报过注册事件，*/
						/* 要保留注册数据来上报离线事件，*/
						/* 并由其清空注册数据 */
					}

					/* 带宽设置失败，则令其重新注册 */
					#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
					OLT_DeregisterLLID(PonPort_id, onu_id, FALSE);
					#else
					PAS_deregister_onu(PonPort_id, onu_id, FALSE);
					#endif

					return( RERROR );					
				}
			}
			else
			{
				/* 去激活之前刚刚激活成功的下行带宽 */
				InActiveOnuDownlinkBW( PonPort_id,  onu_idx );

				/* send trap to NMS: ONU注册带宽超限 */
				llidActBWExceeding_EventReport( (unsigned long)onuIdToOnuIndex(PonPort_id, onu_idx+1), 1, 1, 1 ); 

				/* B--added by liwei056@2010-4-27 for D9670 */
				if ( reg_flag )
				{
					/* 清空其配置记录 */
					InitOneOnuByDefault(PonPort_id, onu_idx);
					AddPendingOnu(PonPort_id, -1, onu_id, OnuRegisterData->mac_address, 0);
				}
				else
				{
					/* 清空其注册状态 */
					ClearOnuRunningData(PonPort_id, onu_idx, 0);
					AddPendingOnu(PonPort_id, onu_idx, onu_id, OnuRegisterData->mac_address, 0);
				}
				/* E--added by liwei056@2010-4-27 for D9670 */

				return( RERROR );
			}
		}
		else
		{
			/* send trap to NMS: ONU注册带宽超限 */
			llidActBWExceeding_EventReport( (unsigned long)onuIdToOnuIndex(PonPort_id, onu_idx+1), 1, 1, 1 ); 

			/* B--added by liwei056@2010-4-27 for D9670 */
			if ( reg_flag )
			{
				/* 清空其配置记录 */
				InitOneOnuByDefault(PonPort_id, onu_idx);
				AddPendingOnu(PonPort_id, -1, onu_id, OnuRegisterData->mac_address, 0);
			}
			else
			{
				/* 清空其注册状态 */
				ClearOnuRunningData(PonPort_id, onu_idx, 0);
				AddPendingOnu(PonPort_id, onu_idx, onu_id, OnuRegisterData->mac_address, 0);
			}
			/* E--added by liwei056@2010-4-27 for D9670 */

			return( RERROR );
		}
		/* E--modified by liwei056@2009-11-12 for D9143 */
	}


#if 0
	if( vendorType != ONU_VENDOR_CT )
	{
	/* added by chenfj 2007-04-12 当限制ONU注册时，同时关闭其数据通道,当ONU注册时,确保其数据通道是打开的*/
	/* modified by chenfj 2009-2-19 
		经与ONU 沟通，得知ONU注册时，都能保证在收到GW OAM OamInformationRequest后打开数据通道
		故OLT 侧不再对此进行设置
		*/
		{ 
		if( PonChipType == PONCHIP_PAS )
			{
			if((PAS_get_onu_mode( PonPort_id, onu_id) != PON_ONU_MODE_PENDING)&&  (PAS_get_onu_mode( PonPort_id, onu_id) != PON_ONU_MODE_ON))
				{
				if( EVENT_REGISTER == V2R1_ENABLE )
					sys_console_printf("  Onu(%02x%02x.%02x%02x.%02x%02x-llid %d) register,but not on-line\r\n",mac_address[0],mac_address[1],mac_address[2],mac_address[3],mac_address[4],mac_address[5], onu_id);
				return(RERROR);
				}
			if(test_debug1 == 1 )
			REMOTE_PASONU_uni_set_port( PonPort_id, onu_id, 1, 1 );
			}
		else{ /* other pon chip handler */

			}
		}
	}
#endif

	/*  for PAS-SOFT5201  V5.1 */
	if( vendorType == ONU_VENDOR_CT )
	{
#if 0	/* modified by xieshl 20111102, 有重复记录或漏记录的情况，不应防止这里 */
		PonPortTable[PonPort_id].CurrOnu ++;
		PonPortTable[PonPort_id].CurLLID ++;

		i = onu_idx & 0x07; /*onu_idx % 8*/;
		j = onu_idx >> 3; /*onu_idx / 8;*/
		PonPortTable[PonPort_id].OnuCurStatus[j] = PonPortTable[PonPort_id].OnuCurStatus[j] |( 1 << i ) ;
#endif	
		/* mib port status */
		/*ONU_MGMT_SEM_TAKE;
		if(OnuMgmtTable[OnuEntry].UsedFlag != ONU_USED_FLAG)
			setOnuStatus( slot,  port, onu_idx, ONU_ONLINE );
		ONU_MGMT_SEM_GIVE;*/

#ifdef STATISTIC_TASK_MODULE
		StatsMsgOnuOnSend( PonPort_id, onu_idx, ONU_ONLINE );
#endif

		/* 6: set alarm parameters */
		set_ONU_alarm_configuration( PonPort_id, onu_idx ) ;

		/* 7: start encrypt if needed */
		/*if( OnuMgmtTable[OnuEntry].EncryptEnable == V2R1_ENABLE ) 
			OnuEncryptionOperation( PonPort_id, (OnuEntry % MAXONUPERPON ), OnuMgmtTable[OnuEntry].EncryptDirection ) ;
		
		V2R1_OnuStartEncrypt(PonPort_id, (OnuEntry% MAXONUPERPON));
		*/

		/*  modified by chenfj 2007-10-31
	           问题单#5684 : 建议将两条ONU的注册/离线打印信息屏蔽
		*/
		if( EVENT_REGISTER == V2R1_ENABLE )
    			sys_console_printf("\r\n  onu %d/%d/%d registered at llid(%d) \r\n", slot, port, onu_idx+1, onu_id );

		/* 8: 记录ONU注册信息*/
		{
			Date_S LaunchDate;
			PonPortTable[PonPort_id].OnuRegisterMsgCounter ++;
			VOS_MemCpy( (void *)PonPortTable[PonPort_id].LastRegisterMacAddr, (void *)OnuRegisterData->mac_address, BYTES_IN_MAC_ADDRESS );
			if( GetSysCurrentTime( &LaunchDate ) == RERROR ) return ( RERROR );
			VOS_MemCpy ((char *) &(PonPortTable[PonPort_id].LastOnuRegisterTime), (char *)&(LaunchDate.year), sizeof( Date_S) );
		}

		/* 9:  added by chenfj 2007-6-27 
 		   ONU	 device  de-active,  增加ONU 去激活操作*/

		if(traffic_enable == V2R1_DISABLE )
		{
			SetOnuTrafficServiceEnable( PonPort_id, onu_idx, traffic_enable ) ;
		}
	}

	/* 9: get onu device info */
	/*  for PAS-SOFT5201  V5.1 */
	else /*if( vendorType == ONU_VENDOR_GW )*/	/* 默认直接按GW模式处理 */
	{
		/*OnuMgmtTable[OnuEntry].GetEUQCounter = 0;*/
		/*modified by chenfj 2008-6-3 
		    问题单6707: 问题1 
		    在获取ONU测距值等信息的ONU注册处理过程中,若有错误返回,则终止此次注册处理;这样应会避免ONU在线,但ONU测距值为零的情况
		*/ /*问题单11299*/
		if(GetOnuRegisterData( PonPort_id, onu_idx ) != ROK ) 
		{
			AddPendingOnu( PonPort_id, onu_idx, onu_id, OnuRegisterData->mac_address, 0); 
			/*Onu_deregister( PonPort_id, (OnuEntry % MAXONUPERPON ) );*/
			return (RERROR );
		}
        
#if 0	/* modified by xieshl 20110216, 直接读设备信息， 然后上报告警 */
		AddOneNodeToGetEUQ(PonPort_id, onu_idx );
#else
		if( (ret = GetOnuEUQInfo(PonPort_id, onu_idx)) == ROK )
		{
			if( EVENT_REGISTER == V2R1_ENABLE )
			{
				sys_console_printf(" onu %d/%d/%d get device info. OK\r\n", slot, port, onu_idx+1 );
			}
			/*if ( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )*/		/* modified by xieshl 20111129, 增加主备同步 */
			{
				ret = OnuMgtSyncDataSend_Register( PonPort_id, onu_idx);
				/*if( EVENT_REGISTER == V2R1_ENABLE )
				{
					sys_console_printf(" onu %d/%d/%d device info. sync to master %s\r\n", slot, port, onu_idx+1, (ret == VOS_OK ? "OK" : "ERR") );
				}*/
			}
		}
		else
		{
			if( EVENT_REGISTER == V2R1_ENABLE )
			{
				sys_console_printf(" onu %d/%d/%d get device info. ERR\r\n", slot, port, onu_idx+1 );
			}
		}
		
		Trap_OnuRegister( PonPort_id, onu_idx );
			
#endif
	}

    /* B--added by liwei056@2010-1-29 for Pon-FastSwicthHover */
#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
    if (PonChipIdx_swap >= 0)
    {
        /* 需要等待备用PON口上此ONU的虚注册成功后，再同步带宽等设置 */
        if ( PonPortIsNotSwaping(PonPort_id) )
        {
            /* 倒换期间的注册，无需同步 */
            unsigned long aulMsg[4] = { MODULE_PON, FC_ONU_PARTNER_REG, 0, 0 };

            /* 备用ONU记录的端口下索引必须与主用一致，且它们的onu注册ID和MAC必须相同 */
            /* 使得从主用ONU记录可以得到其备用ONU记录 */
            standby_onu_idx = onu_idx;
            
            aulMsg[2] = (PonChipIdx_swap << 16 ) | (PonPort_id & 0xFFFF);  
            aulMsg[3] = (standby_onu_idx << 16 ) | (onu_id & 0xFFFF);

            ret = VOS_QueSend( g_Olt_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
            if( ret !=  VOS_OK ){
        			VOS_ASSERT( 0 );
            }	

            OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Onu%d(%02x.%02x.%02x.%02x.%02x.%02x-llid:%d) is syncing register from M_pon%d to S_pon%d on the slot%d.\r\n",
			onu_idx + 1,
			OnuRegisterData->mac_address[0], OnuRegisterData->mac_address[1], OnuRegisterData->mac_address[2], 
			OnuRegisterData->mac_address[3], OnuRegisterData->mac_address[4], OnuRegisterData->mac_address[5],
			onu_id, PonPort_id, PonChipIdx_swap, SYS_LOCAL_MODULE_SLOTNO);
        }
    }
#endif            
    /* E--added by liwei056@2010-1-29 for Pon-FastSwicthHover */

	return( ROK );
}

int OnuDeregisterEvent(short int PonPortIdx, short int OnuIdx, int reason_code, unsigned int event_flags)
{
    int iRlt;
    short int llid;
    ULONG slotno, portno;
    ULONG onuno, onuEntry;
	unsigned int def_up_bw_gr = 0;
	unsigned int def_down_bw_gr = 0;
	

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if( onuEntry >= MAXONU )
	{
		VOS_ASSERT(0);
		return RERROR;
	}

	slotno = GetCardIdxByPonChip(PonPortIdx);
	portno = GetPonPortByPonChip(PonPortIdx);
    onuno  = OnuIdx + 1;
    llid   = OnuMgmtTable[onuEntry].LLID;

	if(EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf(" onu %d/%d/%d deregistration, reason code(%d), flags(%d).\r\n", slotno, portno, onuno, reason_code, event_flags );

	/* modified by xieshl 20111103, 解决PON LOS告警漏报问题，如果ONU全部离线则上报一次，问题单13210 */
	/*if( PonPortTable[PonPortIdx].SignalLossFlag == V2R1_DISABLE )
	{
		if( OnRegOnuCounter(PonPortIdx) <= 1 )
		{
			Trap_PonPortSignalLoss( PonPortIdx, V2R1_ENABLE );
		}
	}*/	/* modified by xieshl 20120620，解决误报问题，去掉后会重新引入漏报，问题单14114 */

#if 0
	iRlt = PAS_reset_address_table(PonPort_id, onu_id, ADDR_DYNAMIC);
	if(EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf(" reset pon%d/%d mac entry, LLID=%d\r\n", slotno, portno, llid );
#endif

	/* add by wangxy 2011-07-15 由等待升级队列中删除*/
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
	    delOnuFromRestoreQueue(PonPortIdx, OnuIdx);

	/* add by shixh20100823,取消ONU管理接口挂接*/
	ONU_SetupIFs(PonPortIdx, OnuIdx, ONU_ADAPTER_NULL);
    OnuMgt_SetOnuLLID(PonPortIdx, OnuIdx, INVALID_LLID);
	if( EVENT_REGISTER == V2R1_ENABLE )
    {
        sys_console_printf("\r\n onu %d/%d/%d loose the LOCAL-ONU service at the OnuDeregister time.\r\n", slotno, portno, onuno );
    }   
       
    /* B--added by liwei056@2010-10-18 for RedundancySwitch-BUG */
    iRlt = GetOnuOperStatus_1(PonPortIdx, OnuIdx);
	if( EVENT_REGISTER == V2R1_ENABLE )
    {
        sys_console_printf("\r\n onu %d/%d/%d deregister at the OnuStatus(%d).\r\n", slotno, portno, onuno, iRlt );
    }   
    if ( iRlt != ONU_OPER_STATUS_UP )
    {
		/* UP的ONU，必定不在Pending队列里 */
		UpdatePendingOnu(PonPortIdx, llid, 0);
		UpdatePendingConfOnu(PonPortIdx, llid, 0);

        /* 没有上报过注册事件的ONU，其离线事件也不能上报 */
    	if ( iRlt == ONU_OPER_STATUS_DOWN )
        {
        	if(EVENT_REGISTER == V2R1_ENABLE )
    			sys_console_printf("\r\n onu %d/%d/%d deregister ERR(onu device status=%d)\r\n", slotno, portno, onuno, iRlt);
    		return( RERROR );
        }
    }
    /* E--added by liwei056@2010-10-18 for RedundancySwitch-BUG */


	/* added by chenfj 2006/09/22 */ 
	/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
	/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/

#if 0
	/*add by shixh20100921,6700应该在这是删除，6900要挪到主控板上删除，这样才能保证PON板和主控的pending onu 一致*/
	if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )    /*add by shixh20101124*/
    {
        DelPendingOnu( PonPortIdx, llid );
        DelPendingOnu_conf( PonPortIdx, llid );
    }
#endif
	
	/* added by chenfj 2009-5-14 , 当这个PON 口下有多于 64个ONU 时，产生告警*/
	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	iRlt = (PonPortTable[PonPortIdx].PendingOnu_Conf.Next == NULL) && (PonPortTable[PonPortIdx].PendingOnu.Next == NULL);
	VOS_SemGive( OnuPendingDataSemId );
	if( iRlt )
	{
		Trap_PonPortFullClear(PonPortIdx);
	}		
	
#if 0
	if(( reason_code < PON_ONU_DEREGISTRATION_REPORT_TIMEOUT) ||(reason_code >= PON_ONU_DEREGISTRATION_LAST_CODE ))
	{
		if( EVENT_REGISTER == V2R1_ENABLE )
			sys_console_printf("  error: pon%d llid%d deregister reason %d\r\n",PonPortIdx, llid, reason_code );
	}
#endif

    
	if( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UP )
	{
		DecreasePonPortRunningData( PonPortIdx,  OnuIdx);
	}

    /*  modify by chenfj 2006/09/20  */
    /**  #2606 问题在手工添加64个ONU后，再实际注册新ONU时，出现异常*/
#if 0
	if( EVENT_REGISTER == V2R1_ENABLE)
	{
		sys_console_printf("  onu %d/%d/%d Deregistration\r\n", slotno, portno , onuno);
		sys_console_printf("  deregistration reason %s\r\n",  PON_onu_deregistration_code_s[reason_code]);
	}
#endif

	/*1 send ONU Not present Trap */
	Trap_OnuDeregister( PonPortIdx, OnuIdx, reason_code, 0 );

	/*RecordOnuUpTime( onuEntry );*/
	
	/*2 if ONU existing alarm , send alarm clear Trip to NMS */

	/*3 Clear the onu mgmt table running data */
	/*sys_console_printf(" onu status %d \r\n", GetOnuOperStatus(PonPort_id, OnuIdx));*/

	/*OnuEncryptionOperation(PonPort_id, OnuIdx,  PON_ENCRYPTION_PURE);*/
	
	/*if(GetOnuOperStatus(PonPort_id, OnuIdx ) == ONU_OPER_STATUS_UP )
		{
		if( EVENT_REGISTER == V2R1_ENABLE )
			sys_console_printf("\r\n   onu %d/%d/%d Deregistered\r\n", slotno, portno, onuno, onu_id );
		}*/
	if(GetOnuOperStatus_1(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_DOWN )
		PonPortTable[PonPortIdx].OnuDeregisterMsgCounter ++;
	
	Trap_OnuPonPortBER( PonPortIdx, OnuIdx, 0, V2R1_BER_CLEAR );
	Trap_OnuPonPortFER( PonPortIdx, OnuIdx, 0, V2R1_FER_CLEAR );
	/* modified by xieshl 20101220, 问题单11740 */
	updateOnuOperStatusAndUpTime(onuEntry, ONU_OPER_STATUS_DOWN);
	/* modified for 共进测试chenfj 2007-8-27*/
#ifndef GONGJIN_VERSION
	ClearOnuRunningData( PonPortIdx, OnuIdx, 0 );
#else 
	InitOneOnuByDefault( PonPortIdx, OnuIdx );

	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].RegisterFlag = NOT_ONU_FIRST_REGISTER_FLAG;
	ONU_MGMT_SEM_GIVE;
#endif

	/*4  release onu bandwidth ?  not needed*/
	if( OnuMgmtTable[onuEntry].NeedDeleteFlag == TRUE )
	{
		/*
		   OnuMgmtTable[PonPort_id * MAXONUPERPON + OnuIdx ].NeedDeleteFlag == FALSE;
		   VOS_MemSet( OnuMgmtTable[PonPort_id * MAXONUPERPON + OnuIdx ].DeviceInfo.MacAddr, 0xff, BYTES_IN_MAC_ADDRESS );
		   */
		GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &def_up_bw_gr, NULL);
		GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK, &def_down_bw_gr, NULL);
		PonPortTable[PonPortIdx].ProvisionedBW += OnuMgmtTable[onuEntry].LlidTable[0].UplinkBandwidth_gr - def_up_bw_gr; 
		PonPortTable[PonPortIdx].DownlinkProvisionedBW += OnuMgmtTable[onuEntry].LlidTable[0].DownlinkBandwidth_gr - def_down_bw_gr; 
		
#ifdef PLATO_DBA_V3
		PonPortTable[PonPortIdx].ProvisionedBW += OnuMgmtTable[onuEntry].LlidTable[0].UplinkBandwidth_fixed;
#endif
		InitOneOnuByDefault(PonPortIdx, OnuIdx);		
	}

	/* added by chenfj 2007-12-11
	 在ONU离线时，清除可能存在的OAM msg buf(主要用于消息分片重组) 
	CommOnuOamMsgFreeAll(PonPort_id, OnuIdx );*/

	/* clear MAC-addr learned from onu when onu is deregister*/
#if 0
	{
	short int Active_num= 0;
	short int i = 0;
	short int PonChipType;

	PonChipType = GetPonChipTypeByPonPort(PonPort_id);

	if( PonChipType == PONCHIP_PAS5201 )
		{
		PAS_get_address_table( PonPort_id, &Active_num, Mac_addr_table);
		VOS_MemSet(Mac_addr_table, 0, sizeof(Mac_addr_table));
		while(( Active_num != 0) /*&& (i < PON_ADDRESS_TABLE_SIZE )*/)
			{
			if(  onu_id  == Mac_addr_table[i].logical_port )
				PAS_remove_address_table_record(PonPort_id, Mac_addr_table[i].mac_address );
			i++;
			Active_num --;
			}
		}
	}
#endif
/*add by shixh20100604*/

	if(EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf(" onu %d/%d/%d manage data init OK\r\n", slotno, portno, onuno );

/* if ( !SYS_LOCAL_MODULE_ISMASTERACTIVE )*/		/* modified by xieshl 20111129, 增加主备同步 */
 {
	OnuMgtSyncDataSend_Deregister( PonPortIdx, OnuIdx );
	/*if(EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf(" onu %d/%d/%d deregister data sync to master\r\n", slotno, portno, onuno );*/
 }
	OltOamPtyOnuLoseNoti(PonPortIdx, onuno);
	
	return( ROK );
}

/*****************************************************
 *
 *    Function:  OnuDeregisterHandler( ONUDeregistrationInfo_S *OnuDeregisterData)
 *
 *    Param:    
 *                 
 *    Desc:   this is a callback function; when onu deregister is happened, this function is called;
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int OnuDeregisterHandler( ONUDeregistrationInfo_S *OnuDeregisterData)
{
	short int PonPortIdx;
	short int OnuIdx;
	PON_onu_id_t  onu_id;
	PON_onu_deregistration_code_t  deregistration_code;
	unsigned int event_flags;

	if( OnuDeregisterData == NULL ){
		VOS_ASSERT( 0 );
		return( RERROR );
	}

	PonPortIdx = OnuDeregisterData->olt_id;
	onu_id = OnuDeregisterData->onu_id;
	deregistration_code = OnuDeregisterData->deregistration_code;
    event_flags = OnuDeregisterData->event_flags;

	if( (!OLT_LOCAL_ISVALID(PonPortIdx) ) || (!LLID_ISVALID(onu_id)) )
	{
		if( EVENT_REGISTER == V2R1_ENABLE )
		{
			sys_console_printf("\r\n ONU DeReg. unknown LLID (pon=%d, llid=%d)\r\n", PonPortIdx, onu_id );
		}
		return( RERROR );
	}
	
	if(EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf("\r\n onu %d/%d/llid(%d) deregistration, reason_code(%d), event_flags(%d).\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), onu_id, deregistration_code, event_flags );
	
	/* modified by chenfj 2007/4/6 */
	/* 问题单#3994: 徐州一台OLT上发现，有时ONU离线消息到不了软件应用层，造成ONU计数错误。*/
	/* 将这个数据处理提到前面来*/
	/* pon data table */
	OnuIdx = GetOnuIdxByLlid( PonPortIdx, onu_id );
	if( OnuIdx == RERROR ) 
	{
		/* llid未记录的ONU，必定在Pending队列里 */
		UpdatePendingOnu(PonPortIdx, onu_id, 0);
		UpdatePendingConfOnu(PonPortIdx, onu_id, 0);

		if(EVENT_REGISTER == V2R1_ENABLE )
			sys_console_printf(" onu deregister ERR(get onuid failed)\r\n");

		return( RERROR );
	}
    
	return OnuDeregisterEvent(PonPortIdx, OnuIdx, deregistration_code, event_flags);
}

/*****************************************************
 *
 *    Function:  PonResetHandler( PonResetData_S *PonResetData )
 *
 *    Param:    
 *                 
 *    Desc:   
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
extern LONG device_chassis_is_slot_inserted( ULONG slotno );

/* B--added by liwei056@2012-3-8 for Uplink's PonMonitorSwitch */
VOID Pon_Monitor_Eth( VOID * pData, ULONG event, VOID * pData2 )
{
    ULONG ulInterfaceType;
    ULONG ifIndex;
    ULONG ulSlotId, ulPortId;
    ULONG ulProtectFlag;
    IFM_DEVNOTIFYMSG *ifm_notify = ( IFM_DEVNOTIFYMSG * ) pData2;
    IFM_NET_DEVICE_S *pstNetDev;
    ETH_P_DATA_S *pstETH_P_DATA;
    ETH_PROTECT_S *pstEthProtectInfo;
    int iNewStatus;
    int iRevsStatus;
    short int PonPortIdx;
    short int EthPortIdx;

    ifIndex = ifm_notify->ulIfIndex;
    ulInterfaceType = IFM_IFINDEX_GET_TYPE( ifIndex );
    if( ulInterfaceType != IFM_ETH_TYPE ) 
    {
      	return;
    }

    switch ( event )
    {
        case NETDEV_UP:
            iNewStatus  = V2R1_PON_PORT_SWAP_ACTIVE;
            iRevsStatus = V2R1_PON_PORT_SWAP_PASSIVE;
            break;
        case NETDEV_DOWN:
            iNewStatus  = V2R1_PON_PORT_SWAP_PASSIVE; 
            iRevsStatus = V2R1_PON_PORT_SWAP_ACTIVE;
            break;
    	default:
    		return;
    }
    
    if ( 0 != IFM_find_netdev( ifIndex, &pstNetDev, NULL ) )
    {
        if ( NETDEV_DOWN == event )
        {
            pstEthProtectInfo = NULL;
            ulProtectFlag = 0;
        }
        else
        {
            /* IF不存在时，只有LinkDown事件有效 */
          	return;
        }
    }
    else
    {
        pstETH_P_DATA = ( ETH_P_DATA_S * ) pstNetDev->pPrivateData;
        if ( pstETH_P_DATA == NULL )
        {
            return;
        }

        pstEthProtectInfo = &pstETH_P_DATA->stProtectInfo;
        ulProtectFlag = pstEthProtectInfo->ulProtectFlag;
    }

    if ( ETH_IS_PROTECTED(ulProtectFlag) )
    {
        if ( !ETH_IS_PROTECT_FORBID(ulProtectFlag) )
        {
            /* 正向从Eth保护配置触发PON倒换 */              
            if ( VOS_OK != ulIfindex_2_userSlot_userPort( ifIndex, &ulSlotId, &ulPortId ) )
            {
                return;
            }
            
            if ( ETH_IS_PROTECT_PARTNER(ulProtectFlag) )
            {
                PonPortIdx = GetPonPortIdxBySlot((short int)pstEthProtectInfo->ucPartnerSlot, (short int)pstEthProtectInfo->ucPartnerPort);
                if ( RERROR != PonPortIdx )
                {
                    /* Partner is Pon Port */
                    if ( OLT_ISLOCAL(PonPortIdx) )
                    {
                        if ( GetPonPortHotSwapTriggers(PonPortIdx) & PROTECT_SWITCH_TRIGGER_UPLINKDOWN )
                        {
                            if ( PonPortIsWorking(PonPortIdx) )
                            {
                            	sendPonSwitchEventMsg( PonPortIdx, PROTECT_SWITCH_EVENT_NOTIFY, PROTECT_SWITCH_REASON_UPLINKCNG, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, iNewStatus, PonPortSwapTimesQuery(PonPortIdx) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
                            }
                        }
                    }
                    else
                    {
                        /* 不属于本地的PON，交由高层负责 */
                    }
                }
                else
                {
                    /* Partner is Eth Port */
                    ulSlotId = pstEthProtectInfo->ucPartnerSlot;
                    ulPortId = pstEthProtectInfo->ucPartnerPort;
                    if ( (ULONG)-1 != (ifIndex = userSlot_userPort_2_Ifindex(ulSlotId, ulPortId)) )
                    {
                        if ( NULL != IFM_GetNameByIndex(ifIndex) )
                        {
                            EthPortIdx = OLT_DEVICE_ID(ulSlotId, ulPortId);
                        	sendPonSwitchEventMsg( EthPortIdx, PROTECT_SWITCH_EVENT_NOTIFY, PROTECT_SWITCH_REASON_UPLINKCNG, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, iNewStatus, PonPortSwapTimesQuery(PonPortIdx) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
                        }
                    }
                }
            }

            if ( ETH_IS_PROTECT_BACKUP(ulProtectFlag) )
            {
                PonPortIdx = GetPonPortIdxBySlot((short int)pstEthProtectInfo->ucProtectSlot, (short int)pstEthProtectInfo->ucProtectPort);
                if ( RERROR != PonPortIdx )
                {
                    /* Partner is Pon Port */
                    if ( OLT_ISLOCAL(PonPortIdx) )
                    {
                        if ( GetPonPortHotSwapTriggers(PonPortIdx) & PROTECT_SWITCH_TRIGGER_ETHLINKCNG )
                        {
                            if ( PonPortIsWorking(PonPortIdx) )
                            {
                            	sendPonSwitchEventMsg( PonPortIdx, PROTECT_SWITCH_EVENT_NOTIFY, PROTECT_SWITCH_REASON_ETHLINKCNG, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, iRevsStatus, PonPortSwapTimesQuery(PonPortIdx) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
                            }
                        }
                    }
                    else
                    {
                        /* 不属于本地的PON，交由高层负责 */
                    }
                }
                else
                {
                    /* Backer is Eth Port */
                    ulSlotId = pstEthProtectInfo->ucProtectSlot;
                    ulPortId = pstEthProtectInfo->ucProtectPort;
                    if ( (ULONG)-1 != (ifIndex = userSlot_userPort_2_Ifindex(ulSlotId, ulPortId)) )
                    {
                        if ( NULL != IFM_GetNameByIndex(ifIndex) )
                        {
                            EthPortIdx = OLT_DEVICE_ID(ulSlotId, ulPortId);
                        	sendPonSwitchEventMsg( EthPortIdx, PROTECT_SWITCH_EVENT_NOTIFY, PROTECT_SWITCH_REASON_ETHLINKCNG, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, iRevsStatus, PROTECT_SWITCH_EVENT_SEQ_NONE, PROTECT_SWITCH_EVENT_FLAGS_NONE );
                        }
                    }
                }
            }        
        }
    }
    else
    {
        /* 反向从PON恢复Eth保护配置 */
        if ( VOS_OK != ulIfindex_2_userSlot_userPort( ifIndex, &ulSlotId, &ulPortId ) )
        {
            return;
        }
        
        EthPortIdx = OLT_DEVICE_ID(ulSlotId, ulPortId);
        if ( ROK == PonPortProtectedPortLocalQuery(EthPortIdx, &PonPortIdx) )
        {
            /* Partner is Pon Port */
            if ( OLT_ISLOCAL(PonPortIdx) )
            {
                if ( PonPortIsWorking(PonPortIdx) )
                {
                	sendPonSwitchEventMsg( PonPortIdx, PROTECT_SWITCH_EVENT_NOTIFY, PROTECT_SWITCH_REASON_UPLINKCNG, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, iNewStatus, PonPortSwapTimesQuery(PonPortIdx) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
                }
            }
            else
            {
                /* 不属于本地的PON，交由高层负责 */
            }
            
            if ( NULL != pstEthProtectInfo )
            {
                ulSlotId = GetCardIdxByPonChip(PonPortIdx);
                ulPortId = GetPonPortByPonChip(PonPortIdx);
                (void)IFM_SetIfPartnerApi(ifIndex, (UCHAR)ulSlotId, (UCHAR)ulPortId);
            }
        }
        else
        {
            /* Partner Maybe Eth Port */
            if ( NULL != pstEthProtectInfo )
            {
                (void)IFM_SetIfPartnerApi(ifIndex, (UCHAR)0, (UCHAR)0);
            }
        }
    }

    return;
}
/* E--added by liwei056@2012-3-8 for Uplink's PonMonitorSwitch */

int PonResetHandler( PonResetInfo_S *PonResetData )
{
	short int PonPort_id;
	short int OnuIdx;
	PON_olt_reset_code_t  Reset_Code;
	/*int slot;*/
	int counter = 1, test_result = RERROR;
    int CardSlot;
    int CardPort;
	int onuStatus;

	if( PonResetData == NULL )
    {
		VOS_ASSERT( 0 );
		return( RERROR );
	}
		
	PonPort_id = PonResetData->olt_id;
	Reset_Code = PonResetData->code;

	if(PonPortIsWorking(PonPort_id) != TRUE )
		return( RERROR );

    CardSlot = GetCardIdxByPonChip(PonPort_id);
    CardPort = GetPonPortByPonChip(PonPort_id);

	if( EVENT_RESET == V2R1_ENABLE)
    {
		sys_console_printf("Event: pon%d/%d reset,reason:%s\r\n", CardSlot, CardPort, PON_olt_reset_code_s[Reset_Code]);
	}

	/* only for test by chenfj 2009-2-11*/
	while((test_result != ROK) &&(counter <= 2))
	{
		test_result = GetDBAInfo(PonPort_id);
		counter++;
		if(test_result != ROK )
            SysLog_PonPortAbnormal(PonPort_id, test_result);
		else
            SysLog_PonPortAbnormal(PonPort_id, ROK);

        VOS_TaskDelay(VOS_TICK_SECOND);
	}
	if(test_result == ROK ) return(ROK);
	/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	Pon_RemoveOlt(PonPort_id, FALSE, FALSE);	
	
	/*1  send PON abnormal trap to NMS 
	slot = GetCardIdxByPonChip(PonPort_id);
	if(device_chassis_is_slot_inserted(CardSlot) == PONCHIP_EXIST )
		{
		if(getPonChipInserted(CardSlot, CardPort) == PONCHIP_EXIST )
			PonAbnormalHandler(PonPort_id, Reset_Code);
		}
	*/
	PonAbnormalHandler(PonPort_id, Reset_Code);

    if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
    {
        OLT_SYNC_Data(PonPort_id, FC_PON_RESET, NULL, 0);
    }
	
	if( GetPonChipTypeByPonPort(PonPort_id) != PONCHIP_PAS5001 )
	{
	    /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		if(OLTAdv_IsExist(PonPort_id) == TRUE )
			Pon_RemoveOlt( PonPort_id, FALSE, FALSE);	
		ClearPonRunningData( PonPort_id );
#if 0
		Hardware_Reset_olt1(CardSlot, CardPort, 0, 0);
		VOS_TaskDelay(VOS_TICK_SECOND);
		Hardware_Reset_olt2(CardSlot, CardPort, 1, 0);
#endif
		Add_PonPort( PonPort_id );
	}
	
	return( ROK );
	
	/*2 send Pon Alarm Clear msg to NMS */
#if 0
	/*3 clear onu running data */
	for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
    {
		/* shutDown the registered onu */
		onuStatus = GetOnuOperStatus_1(PonPort_id, OnuIdx);
		if( onuStatus != ONU_OPER_STATUS_DOWN)
		{
			/* send onu deregister msg to NMS */
			if( onuStatus == ONU_OPER_STATUS_UP)			
				Trap_OnuDeregister( PonPort_id, OnuIdx, PON_ONU_DEREGISTRATION_HOST_REQUEST, 0 );
				
			/* send onu alarm clear msg to NMS */
				
			/* modified by xieshl 20110624, 问题单13090 */
			SetOnuOperStatus(PonPort_id, OnuIdx, ONU_OPER_STATUS_DOWN );
			ClearOnuRunningData(  PonPort_id,  OnuIdx, 0 );		
		}
	}
#else
    OnuEvent_ClearRunningDataMsg_Send(PonPort_id);
#endif
	/*4  clear PON running data */
	ClearPonPortRunningData( PonPort_id );
	PonChipMgmtTable[PonPort_id].operStatus = PONCHIP_ONLINE;
	PonChipMgmtTable[PonPort_id].Err_counter = 0;

	/*
	Remove_olt( PonPort_id, FALSE, FALSE);
	ClearPonRunningData( PonPort_id );
	*/

	if( V2R1_GetPonchipType(PonPort_id) != PONCHIP_PAS5001 )
	{
#if 0	
		Hardware_Reset_olt1(CardSlot, CardPort, 0, 0);
		VOS_TaskDelay(VOS_TICK_SECOND);
		Hardware_Reset_olt2(CardSlot, CardPort, 1, 0);
#endif
		Add_PonPort( PonPort_id );
	}
		

#if 0	
	/*5 Physically reset the pon chip should be done in some cases, if the Pon card existing  */
	if( GetOltCardslotInserted( GetCardIdxByPonChip ( PonPort_id) ) == CARDINSERT )
		{
		/*a. physically reset the pon chip */
		
		VOS_TaskDelay(VOS_TICK_SECOND/5 );

		/* b. add the olt to PAS-SOFT and app soft */
		Add_PonPort(PonPort_id);	
		}	
#endif

	if( Reset_Code == PON_OLT_RESET_HOST_TIMEOUT ){
		}
	
	else if( Reset_Code == PON_OLT_RESET_OLT_EVENT ){
		}

	else if( Reset_Code == PON_OLT_RESET_OLT_NOT_INITED_RESPONSE ){
		}

	return( ROK );

}

/* B--added by liwei056@2010-1-26 for Pon-FastSwicthHover */
int PonSwitchHandler(PonSwitchInfo_S *PonSwitchData)
{
    short int olt_id;
    short int part_olt_id;
    short int swap_olt_id;
    short int event_id;
    short int event_code;
    unsigned short event_seq;
    unsigned short event_flags;
    unsigned short event_source;
	unsigned short SrcSlot;
    int CardSlot;
    int CardPort;
    int PartSlot;
    int PartPort;
    int SwapSlot;
    int SwapPort;
    int SwapMode;
    int swap_status, swap_status2;
    int DstSlot, DstStatus;
    static const char *acszSwitchStartReason[] = {"host-req", "pon-loss", "llid-fail", "olt-remove", "onu-none", "optic-power", "optic-error", "uplink-cng", "eth-linkcng"};
    static const char *acszSwitchFinishedResult[] = {"successed", "failed"};

    /* 1.获取倒换消息，及消息的有效性过滤 */
    olt_id       = PonSwitchData->olt_id;
    event_id     = PonSwitchData->event_id;
    event_code   = PonSwitchData->event_code;
    event_source = PonSwitchData->event_source;
    swap_status  = PonSwitchData->new_status;
    swap_status2 = OLT_SWAP_STATUS_SWITCHOVER(swap_status);
    event_seq    = PonSwitchData->event_seq;
    event_flags  = PonSwitchData->event_flags;
    SrcSlot      = PonSwitchData->slot_source;
	OLT_SWITCH_DEBUG("\r\n master :%d, local: %d, olt_id: %d, event_id: %d, event_code: %d\r\n",SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER,OLT_ISLOCAL(olt_id),olt_id,event_id,event_code);
	OLT_SWITCH_DEBUG("\r\n event_source: %d, new_status: %d, event_seq: %d, event_flags: %d, slot_source: %d\r\n",event_source,swap_status,event_seq,event_flags,SrcSlot);

    if((event_id < PROTECT_SWITCH_EVENT_NOTIFY) || (event_id > PROTECT_SWITCH_EVENT_START))
        return( RERROR );
    if(event_code<0)
        return( RERROR );
    if ((PROTECT_SWITCH_EVENT_OVER == event_id) && (event_code > PROTECT_SWITCH_RESULT_MAX))
        return( RERROR );
    if ((PROTECT_SWITCH_EVENT_START == event_id) && (event_code > PROTECT_SWITCH_REASON_MAX))
        return( RERROR );
    if ((PROTECT_SWITCH_EVENT_NOTIFY == event_id) && (event_code > PROTECT_SWITCH_REASON_MAX))
        return( RERROR );
    if ((event_source > PROTECT_SWITCH_EVENT_SRC_REMOTE) || (event_source < PROTECT_SWITCH_EVENT_SRC_SOFTWARE))
        return( RERROR );

    /* 2.转换倒换消息为本地倒换事件 */
    SwapSlot = 0;
    SwapPort = 0;

    if (  OLT_ISREMOTE(olt_id) )
    {
        CardSlot = OLT_SLOT_ID(olt_id);
        CardPort = OLT_PORT_ID(olt_id);

        if ( RERROR != (swap_olt_id = GetPonPortIdxBySlot(CardSlot, CardPort)) )
        {
            if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER
                && SYS_MODULE_IS_REMOTE(CardSlot) )
            {
                if( ROK != PonPortSwapPortQuery(olt_id, &part_olt_id) )
                {
                    return( RERROR );
                }
                
                PartSlot = GetCardIdxByPonChip(part_olt_id);
                PartPort = GetPonPortByPonChip(part_olt_id);
                
                if ( PROTECT_SWITCH_EVENT_SRC_REMOTE == event_source )
                {
                    /* 改为本地的倒换状态变化 */
                    OLT_LOCAL_ASSERT(part_olt_id);
                    
                    olt_id       = part_olt_id;
                    part_olt_id  = OLT_DEVICE_ID(PartSlot, PartPort);

                    if ( PROTECT_SWITCH_EVENT_START == event_id )
                    {
                        SwapSlot = CardSlot;
                        SwapPort = CardPort;
                    }
                    else
                    {
                        SwapSlot = PartSlot;
                        SwapPort = PartPort;
                    }
                }
            }
            else
            {
                olt_id = swap_olt_id;

                if( ROK != PonPortSwapPortQuery(olt_id, &part_olt_id) )
                {
                    return( RERROR );
                }
                
                PartSlot = GetCardIdxByPonChip(part_olt_id);
                PartPort = GetPonPortByPonChip(part_olt_id);
            }

            if ( OLT_ISREMOTE(olt_id) )
            {
                /* 不可能两个都是外部OLT */
                OLT_LOCAL_ASSERT(part_olt_id);

                /* 翻转报告 */
                swap_olt_id = olt_id;
                olt_id = part_olt_id;
                part_olt_id = swap_olt_id;
                
                swap_status  = swap_status2;
                swap_status2 = OLT_SWAP_STATUS_SWITCHOVER(swap_status);

                PonSwitchData->new_status = swap_status;

                CardSlot = PartSlot;
                CardPort = PartPort;

                PartSlot = OLT_SLOT_ID(part_olt_id);
                PartPort = OLT_PORT_ID(part_olt_id);
            }
        }
        else
        {
            /* 以太口的自倒换 */
        }
    }
    else
    {
        CardSlot = GetCardIdxByPonChip(olt_id);
        CardPort = GetPonPortByPonChip(olt_id);

        if( ROK == PonPortSwapPortQuery(olt_id, &part_olt_id) )
        {
            PartSlot = GetCardIdxByPonChip(part_olt_id);
            PartPort = GetPonPortByPonChip(part_olt_id);
        }
        else
        {
            /* 自倒换 */
            PartSlot = CardSlot;
            PartPort = CardPort;

            part_olt_id = OLT_ID_NULL;
        }
    }

    /* 3.获知倒换模式，及本地倒换事件的有效性过滤 */
    if( (olt_id >= 0) && (olt_id < MAXPON) )
    {
        SwapMode = PonPortTable[olt_id].swap_mode;

        if ( event_seq <= PonPortTable[olt_id].swap_times )
        {
            /* 丢掉此过期的同步事件 */
            if ( PROTECT_SWITCH_EVENT_OVER == event_id )
            {
                OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"pon%d/%d loose switch-hover over(%s) event\r\n"
                    , (0 < SwapSlot) ? (SwapSlot) : ((V2R1_PON_PORT_SWAP_ACTIVE == swap_status) ? CardSlot : PartSlot)
                    , (0 < SwapPort) ? (SwapPort) : ((V2R1_PON_PORT_SWAP_ACTIVE == swap_status) ? CardPort : PartPort)
                    , acszSwitchFinishedResult[event_code]);
            }
            else if ( PROTECT_SWITCH_EVENT_START == event_id )
            {
        	    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"pon%d/%d loose switch-hover start(%s) event\r\n"
                    , (0 < SwapSlot) ? (SwapSlot) : ((V2R1_PON_PORT_SWAP_PASSIVE == swap_status) ? CardSlot : PartSlot)
                    , (0 < SwapPort) ? (SwapPort) : ((V2R1_PON_PORT_SWAP_PASSIVE == swap_status) ? CardPort : PartPort)
                    , acszSwitchStartReason[event_code]);
            }
            else
            {
            }

            return( RERROR );
        }
    }
    else
    {
        SwapMode = V2R1_PON_PORT_SWAP_DISABLED;
    }
    
    if ( PROTECT_SWITCH_EVENT_NOTIFY != event_id )
    {
        if ( V2R1_PON_PORT_SWAP_DISABLED == SwapMode )
        {
            /* 本地倒换设置已经取消，不再接收倒换事件 */
            VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "PonSwitchHandler(olt:%d) Recv SwitchEvent(%d<-%d) at NoRdnCfg state on slot(%d)", olt_id, event_id, event_source, SYS_LOCAL_MODULE_SLOTNO);
            return( RERROR );
        }
    }
    else
    {
        /* 倒换触发事件，无需同步到备用PON口 */
        PartSlot = CardSlot;
        PartPort = CardPort;
    }

    /* 4.本地倒换事件的状态同步 */
    if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
    {
        if ( PartSlot != CardSlot )
        {
            if ( SYS_MODULE_IS_LOCAL(PartSlot) )
            {
                if ( PRODUCT_IS_DISTRIBUTE )
                {
					if(SYS_MODULE_IS_REMOTE(CardSlot))
					{
						swap_olt_id = olt_id;
						DstStatus = swap_status;
						/* 主控板需要向下同步虚拟倒换事件到另一块带CPU的PON板 */
						PonSwitchData->event_source = PROTECT_SWITCH_EVENT_SRC_SOFTWARE;
						PonSwitchData->new_status = DstStatus;
						OLT_SYNC_Data(swap_olt_id, FC_PONSWITCH, PonSwitchData, sizeof(PonSwitchInfo_S));
					}
					else
					{
						/* B--added by liwei056@2013-3-29 for D16711 */
						if ( SrcSlot == CardSlot )
						{
							DstSlot = PartSlot;

							swap_olt_id = part_olt_id;
							DstStatus = swap_status2;
						}
						else if ( SrcSlot == PartSlot )
						{
							DstSlot = CardSlot;
							swap_olt_id = olt_id;
							DstStatus = swap_status;
						}
						else
						{
							VOS_ASSERT(0);
							OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"pon%d/%d<->pon%d/%d is unknowned to event(%d)'s src-slot(%d)\r\n", CardSlot, CardPort, PartSlot, PartPort, event_id, SrcSlot);

							DstSlot = SYS_LOCAL_MODULE_SLOTNO;
						}
						/* E--added by liwei056@2013-3-29 for D16711 */
						if ( SYS_MODULE_IS_PONCARD_LOCAL_MANAGER(DstSlot)
								/* B--added by liwei056@2013-1-11 for D16685 */
								&& (DstSlot != SYS_LOCAL_MODULE_SLOTNO)
								/* E--added by liwei056@2013-1-11 for D16685 */
						   )
						{
							/* 主控板需要向下同步虚拟倒换事件到另一块带CPU的PON板 */
							PonSwitchData->event_source = PROTECT_SWITCH_EVENT_SRC_SOFTWARE;
							PonSwitchData->new_status = DstStatus;
							OLT_SYNC_Data(swap_olt_id, FC_PONSWITCH, PonSwitchData, sizeof(PonSwitchInfo_S));
						}
					}
                
                    
                }
            }
            else
            {
                if ( PROTECT_SWITCH_EVENT_SRC_REMOTE != event_source )
                {
                    /* 主控板需要向外同步虚拟倒换事件到另一台设备 */
                    PonSwitchData->new_status = swap_status2;
                    PonSwitchData->event_source = PROTECT_SWITCH_EVENT_SRC_REMOTE;
                    OLT_SYNC_Data(part_olt_id, FC_PONSWITCH, PonSwitchData, sizeof(PonSwitchInfo_S));
                }
            }
        }
    }
    else
    {
        /* 带CPU的PON板需要向上同步硬件倒换事件到主控板 */
        if ( PROTECT_SWITCH_EVENT_SRC_HARDWARE == event_source )
        {
            PonSwitchData->event_source = PROTECT_SWITCH_EVENT_SRC_SOFTWARE;
            OLT_SYNC_Data(olt_id, FC_PONSWITCH, PonSwitchData, sizeof(PonSwitchInfo_S));
        }
    }


    /* 5.倒换事件的本地处理*/
    switch (event_id)
    {
        case PROTECT_SWITCH_EVENT_OVER:
            /* B--added by liwei056@2011-12-19 for D13656 */
            if ( V2R1_SWAPPING_NO == PonPortTable[olt_id].swapping )
            {
                /* 未收到倒换开始消息，就收到结束消息 */
                PonSwitchInfo_S PonSwitchStart;

                /* 构造本地的倒换开始消息 */
                PonSwitchStart.olt_id       = olt_id;
                PonSwitchStart.new_status   = V2R1_PON_PORT_SWAP_ACTIVE;
                PonSwitchStart.event_id     = PROTECT_SWITCH_EVENT_START;
                PonSwitchStart.event_code   = PROTECT_SWITCH_REASON_HOSTREQUEST;
                PonSwitchStart.event_source = PROTECT_SWITCH_EVENT_SRC_SOFTWARE;
                PonSwitchStart.slot_source  = SYS_LOCAL_MODULE_SLOTNO;
                PonSwitchStart.event_seq    = event_seq;
                PonSwitchStart.event_flags  = PROTECT_SWITCH_EVENT_FLAGS_NONE;
            
                PonSwitchHandler(&PonSwitchStart);
            }
            /* E--added by liwei056@2011-12-19 for D13656 */
            
            /* ---收到倒换结果通知--- */
            OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"pon%d/%d %s to finish switch-hover\r\n"
                , (0 < SwapSlot) ? (SwapSlot) : ((V2R1_PON_PORT_SWAP_ACTIVE == swap_status) ? CardSlot : PartSlot)
                , (0 < SwapPort) ? (SwapPort) : ((V2R1_PON_PORT_SWAP_ACTIVE == swap_status) ? CardPort : PartPort)
                , acszSwitchFinishedResult[event_code]);

            /* 无论倒换成功与否，都是一样的后期配置同步处理(确保配置、状态一致) */
            switch ( SwapMode )
            {
                case V2R1_PON_PORT_SWAP_QUICKLY:
                {
                    if ( (PROTECT_SWITCH_EVENT_SRC_HARDWARE == event_source)
                         && (PROTECT_SWITCH_REASON_ONUNONE != PonPortTable[olt_id].swap_reason) )
                    {
                        int iResumeReason;
                    
                        /* 测试发现有时倒换之后,停止MAC学习 */
                        ResumeOltAddressTableConfig(olt_id);

                        /* 配置同步处理(确保配置、状态一致) */
                        //CopyAllConfigFromPon1ToPon2(part_olt_id, olt_id, OLT_COPYFLAGS_SYNC | OLT_COPYFLAGS_WITHLLID);

                        /* 快倒换的软、硬倒换告警 */
                        Trap_PonAutoProtectSwitch(olt_id);

                        iResumeReason = ( 0 == PonPortTable[olt_id].swap_reason ) ? PON_ONU_DEREGISTRATION_HOST_REQUEST : PON_ONU_DEREGISTRATION_REPORT_TIMEOUT;

                        /* 恢复原主口上的ONU离线信息及日志 */
                        OLT_ResumeAllOnuStatus(part_olt_id, iResumeReason, OLT_RESUMEMODE_FORCEDOWN);

                        /* 激活新主口上的ONU注册状态 */
                        OLT_ResumeAllOnuStatus(olt_id, iResumeReason, OLT_RESUMEMODE_ACTIVEHARD);

                        /* 通知新主口倒换后的ONU状态及倒换设置的恢复处理 */
                        PonPortTable[olt_id].swapHappened = TRUE;
                    }
                    else
                    {
                        if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
                        {
                            if ( PROTECT_SWITCH_EVENT_FLAGS_NEEDRESUME & event_flags )
                            {
                                /* 通知新主口倒换后的ONU状态及倒换设置的恢复处理 */
                                PonPortTable[olt_id].swapHappened = TRUE;
                            }
                        }
                    }
                }
                break;
                case V2R1_PON_PORT_SWAP_SLOWLY:
                {
                    if ( (PROTECT_SWITCH_EVENT_SRC_SOFTWARE == event_source)
                        && (V2R1_PON_PORT_SWAP_ACTIVE == swap_status) )
                    {
                        /* 慢倒换的软倒换告警 */
                        Trap_PonAutoProtectSwitch(olt_id);
                    }
                }
                break;
                /* B--added by liwei056@2012-8-7 for D15475 */
                case V2R1_PON_PORT_SWAP_AUTO:
                /* 主备倒换时，同时发生PON倒换，则备用上倒换模式为auto */
                break;
                /* E--added by liwei056@2012-8-7 for D15475 */
                default:
                    VOS_ASSERT(0);
            }

            /* 标识倒换过程结束 */
            PonPortTable[olt_id].swap_times++;
    	    PonPortTable[olt_id].swap_reason = 0;
    	    PonPortTable[olt_id].swap_timer  = 0;
            PonPortTable[olt_id].swapping    = V2R1_SWAPPING_NO;
            if ( OLT_ISLOCAL(part_olt_id) )
            {
                PonPortTable[part_olt_id].swap_times++;
                PonPortTable[part_olt_id].swap_reason = 0;
                PonPortTable[part_olt_id].swap_timer  = 0;
                PonPortTable[part_olt_id].swapping    = V2R1_SWAPPING_NO;
            }
			PonPortTable[olt_id].swap_interval = V2R1_AutoProtect_Interval;

#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
            if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
            {
                SetPonPortAutoProtectAcitve(olt_id);               
                if ( OLT_ISLOCAL(part_olt_id) )
                {
                    SetPonPortAutoProtectPassive(part_olt_id);
                }
            }
#endif
            break;
        case PROTECT_SWITCH_EVENT_START:
            /* ---收到倒换开始通知--- */
            /* B--added by liwei056@2011-12-19 for D13656 */
            if ( (V2R1_SWAPPING_NOW == PonPortTable[olt_id].swapping)
                || (TRUE == PonPortTable[olt_id].swapHappened) )
            {           
                /* 倒换未结束，又收到倒换开始消息 */
                if ( PROTECT_SWITCH_EVENT_SRC_HARDWARE != event_source )
                {
                    /* 丢掉此同步事件 */
            	    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"pon%d/%d loose switch-hover start event for the %s \r\n"
                        , (0 < SwapSlot) ? (SwapSlot) : ((V2R1_PON_PORT_SWAP_PASSIVE == swap_status) ? CardSlot : PartSlot)
                        , (0 < SwapPort) ? (SwapPort) : ((V2R1_PON_PORT_SWAP_PASSIVE == swap_status) ? CardPort : PartPort)
                        , acszSwitchStartReason[event_code]);
                    return( RERROR );
                }
            }
            /* E--added by liwei056@2011-12-19 for D13656 */
            
            if ( OLT_ISLOCAL(part_olt_id) )
            {
                /* 刷新倒换状态 */
                PonPortTable[part_olt_id].swap_use    = swap_status2;
                PonPortTable[part_olt_id].swapping    = V2R1_SWAPPING_NOW;
                PonPortTable[part_olt_id].swap_reason = event_code;
                PonPortTable[part_olt_id].swap_timer  = 0;
            }
            
            /* 标识倒换过程开始 */
            PonPortTable[olt_id].swap_use    = swap_status;           
            PonPortTable[olt_id].swapping    = V2R1_SWAPPING_NOW;
            PonPortTable[olt_id].swap_reason = event_code;
            PonPortTable[olt_id].swap_timer  = 0;

            switch ( PonPortTable[olt_id].swap_mode )
            {
                case V2R1_PON_PORT_SWAP_SLOWLY:
                    /* 慢倒换需关闭备用PON直连交换口，否则下行带宽不能保证(D13221) */
                    if ( V2R1_PON_PORT_SWAP_PASSIVE == swap_status )
                    {
                        DisablePonPortLinkedSwPort(olt_id);
                        if ( OLT_ISLOCAL(part_olt_id) )
                        {
                            EnablePonPortLinkedSwPort(part_olt_id);
                        }
                    }
                    else
                    {
                        if ( OLT_ISLOCAL(part_olt_id) )
                        {
                            DisablePonPortLinkedSwPort(part_olt_id);
                        }
                        EnablePonPortLinkedSwPort(olt_id);
                    }

                    break;
                case V2R1_PON_PORT_SWAP_QUICKLY:
                    /* 快倒换需清空PON直连以太口的MAC学习表 */
                    if ( V2R1_PON_PORT_SWAP_PASSIVE == swap_status )
                    {
                        RefreshPonPortLinkedSwPort(olt_id);
                    }
                    else
                    {
                        if ( OLT_ISLOCAL(part_olt_id) )
                        {
                            RefreshPonPortLinkedSwPort(part_olt_id);
                        }
                    }

                    break;
                /* B--added by liwei056@2012-8-7 for D15475 */
                case V2R1_PON_PORT_SWAP_AUTO:
                /* 主备倒换时，同时发生PON倒换，则备用上倒换模式为auto */
                break;
                /* E--added by liwei056@2012-8-7 for D15475 */
                default:
                    VOS_ASSERT(0);
            }

    	    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"pon%d/%d start switch-hover for the %s event\r\n"
                , (0 < SwapSlot) ? (SwapSlot) : ((V2R1_PON_PORT_SWAP_PASSIVE == swap_status) ? CardSlot : PartSlot)
                , (0 < SwapPort) ? (SwapPort) : ((V2R1_PON_PORT_SWAP_PASSIVE == swap_status) ? CardPort : PartPort)
                , acszSwitchStartReason[event_code]);
            break;
        case PROTECT_SWITCH_EVENT_NOTIFY:
            switch ( SwapMode )
            {
                case V2R1_PON_PORT_SWAP_SLOWLY:
                case V2R1_PON_PORT_SWAP_QUICKLY:
                    if ( PROTECT_SWITCH_EVENT_SRC_HARDWARE == event_source )
                    {
                        if ( OLT_ISLOCAL(olt_id) )
                        {
                            if ( PonPortTable[olt_id].swap_use != swap_status )
                            {
                                if ( V2R1_SWAPPING_NO == PonPortTable[olt_id].swapping )
                                {
                                    if ( V2R1_PON_PORT_SWAP_PASSIVE == swap_status )
                                    {
                                        AsyncHotSwapPort(part_olt_id);
                                    }
                                    else
                                    {
                                        AsyncHotSwapPort(olt_id);
                                    }
                                }
                            }
                        }
                    }

                    break;
                case V2R1_PON_PORT_SWAP_AUTO:
                case V2R1_PON_PORT_SWAP_DISABLED:
                    if ( OLT_ISLOCAL(olt_id) )
                    {
                        if ( V2R1_PON_PORT_SWAP_PASSIVE == swap_status )
                        {
                            if ( PROTECT_SWITCH_EVENT_SRC_HARDWARE == event_source )
                            {
                                OLTAdv_SetOpticalTxMode2(olt_id, 0, PONPORT_TX_SWITCH);
                            }
                            DisablePonPortLinkedSwPort(olt_id);
                        }
                        else
                        {
                            if ( PROTECT_SWITCH_EVENT_SRC_HARDWARE == event_source )
                            {
                                OLTAdv_SetOpticalTxMode2(olt_id, 1, PONPORT_TX_SWITCH);
                            }
                            EnablePonPortLinkedSwPort(olt_id);
                        }
                    }
                    else
                    {
                        ULONG iSwSlotDrvId, iSwPortDrvId;
                        
                        if ( VOS_OK == slot_port_2_swport(OLT_SLOT_ID(olt_id), OLT_PORT_ID(olt_id), &iSwSlotDrvId, &iSwPortDrvId) )
                        {
                            if ( V2R1_PON_PORT_SWAP_PASSIVE == swap_status )
                            {
                                bcm_port_enable_set( 0, iSwPortDrvId, 0 );
                            }
                            else
                            {
                                bcm_port_enable_set( 0, iSwPortDrvId, 1 );
                            }
                        }
                    }

                    break;
                default:
                    VOS_ASSERT(0);
            }

    	    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"port%d/%d trigger switch-hover to be %s for the %s event\r\n"
                , CardSlot, CardPort
                , (V2R1_PON_PORT_SWAP_PASSIVE == swap_status) ? "passived" : "actived"
                , acszSwitchStartReason[event_code]);
            break;
        default:
            VOS_ASSERT(0);
            return( RERROR );
    }
		
    return( ROK );
}
/* E--added by liwei056@2010-1-26 for Pon-FastSwicthHover */

/*Begin:for onu swap by jinhl@2013-02-22*/
#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
int OnuSwitchHandler(OnuSwitchInfo_S *OnuSwitchData)
{
    short int olt_id;
    short int part_olt_id;
	short int onuIdx = RERROR;
	short int part_onuIdx = RERROR;
    short int event_id;
	PON_redundancy_msg_olt_failure_info_t fail_info;
    unsigned short event_seq;
    unsigned short event_flags;
    int event_code;
    int event_source;
    int CardSlot;
    int CardPort;
    int PartSlot;
    int PartPort;
    int SwapMode;
	int iRlt = VOS_OK;
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE" Enter into OnuSwitchHandler\r\n");

    /* 1.获取倒换消息，及消息的有效性过滤 */
    olt_id       = OnuSwitchData->olt_id;
    event_id     = OnuSwitchData->event_id;
    fail_info    = OnuSwitchData->fail_info;
	event_source = OnuSwitchData->event_source;
    event_seq    = OnuSwitchData->event_seq;
    event_flags  = OnuSwitchData->event_flags;
    OLT_SWITCH_DEBUG("switch handler,olt_id:%d,event_id:%d,src:%d\r\n",olt_id,event_id,event_source);
    if((event_id < PROTECT_SWITCHONU_EVENT_START) || (event_id > PROTECT_SWITCHONU_EVENT_OVER))
        return( RERROR );
    if((fail_info.reason < PON_REDUNDANCY_OLT_FAILURE_HOST_REQUEST) || 
		(fail_info.reason > PON_REDUNDANCY_OLT_ONUSWAPOVER) )
        return( RERROR );

	/* 将本地倒换事件同步到主控去处理*/
    if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
    {
        /* 带CPU的PON板需要向上同步硬件倒换事件到主控板 */
        if ( PROTECT_SWITCH_EVENT_SRC_HARDWARE == OnuSwitchData->event_source )
        {
            OnuSwitchData->event_source = PROTECT_SWITCH_EVENT_SRC_SOFTWARE;
            OLT_SYNC_Data(olt_id, FC_ONUSWITCH, OnuSwitchData, sizeof(OnuSwitchInfo_S));
        }
		return VOS_OK;
    }

	CardSlot = GetCardIdxByPonChip(olt_id);
    CardPort = GetPonPortByPonChip(olt_id);
   
    if( ROK == PonPortSwapPortQuery(olt_id, &part_olt_id) )
    {
        PartSlot = GetCardIdxByPonChip(part_olt_id);
        PartPort = GetPonPortByPonChip(part_olt_id);
    }
	else
	{
	    sys_console_printf("switch handler,no swap\r\n");
	    VOS_ASSERT(0);
		return( RERROR );
	}

	ONU_SWITCH_DEBUG("switch handler,part_olt_id:%d,partslot:%d,partport:%d\r\n",part_olt_id,PartSlot,PartPort);
    
    /* 3.获知倒换模式，及本地倒换事件的有效性过滤 */
    if( (olt_id >= 0) && (olt_id < MAXPON) )
    {
        SwapMode = PonPortTable[olt_id].swap_mode;
    }
    else
    {
        SwapMode = V2R1_PON_PORT_SWAP_DISABLED;
    }
    
    
    if ( V2R1_PON_PORT_SWAP_DISABLED == SwapMode )
    {
        /* 本地倒换设置已经取消，不再接收倒换事件 */
		VOS_ASSERT(0);
        VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "OnuSwitchHandler(olt:%d) Recv SwitchEvent(%d<-%d) at NoRdnCfg state on slot(%d)", olt_id, event_id, event_source, SYS_LOCAL_MODULE_SLOTNO);
        return( RERROR );
    }

     
    
    /* 5.倒换事件处理*/
    switch (event_id)
    {
        case PROTECT_SWITCHONU_EVENT_OVER:
            
            /* ---收到倒换结果通知--- */
            ONU_SWITCH_DEBUG("pon%d/%d to finish switchonu-hover\r\n"
                , CardSlot, CardPort);
           
            switch ( SwapMode )
            {
                case V2R1_PON_PORT_SWAP_ONU:
                {
					int iResumeReason;
                 
                    int i = 0;
					short int 			  result = 0;
					short int number = 0;
					bool exist = FALSE;
					int llid_n = 0;
					int llid = 0;
					int OnuEntry = 0;
					int status = 0;
					VOS_MemSet(&olt_onu_global_table,0, sizeof(OLT_onu_table_t));
                    /*针对板拔出，复位PON口等olt removed的处理*/
					/*在onu注册时已经拷贝了llid,mac,在倒换时，不拷贝llid
					并且，针对板拔出，pon口复位等原olt不在位的情况，
					需要通过对端mac,llid得到本端OnuIdx*/
					#if 1
					if(PON_REDUNDANCY_OLT_Removed == fail_info.reason)
					{
					    llid_n = 0;
					    for(i = 0; i < MAXONUPERPON; i++)
						{
						    
						    OnuEntry = part_olt_id*MAXONUPERPON + i;
							ONU_MGMT_SEM_TAKE;
					        llid = OnuMgmtTable[OnuEntry].LLID;
							status = OnuMgmtTable[OnuEntry].OperStatus;
					    	ONU_MGMT_SEM_GIVE;
							part_onuIdx = GetOnuIdxByLlid(part_olt_id, llid);
							if((RERROR != part_onuIdx) && (ThisIsValidOnu(part_olt_id, part_onuIdx) == ROK) &&
								(ONU_OPER_STATUS_DOWN == status || ONU_OPER_STATUS_POWERDOWN == status))
							{
							    fail_info.llid_list_marker[llid_n] = llid;
								llid_n++;
																	
							}
							
						}
						fail_info.llid_n = llid_n;
						ONU_SWITCH_DEBUG("olt removed,llid_n:%d\r\n",fail_info.llid_n);
					}
					
					if(fail_info.llid_n <= 0)
					{ 
                        
						break;
					}
					#endif
					#if 1
					if(PON_REDUNDANCY_OLT_Removed == fail_info.reason)
					{
						/*激活对端的llid*/
	                    result = OLT_SetRdnLLIDStdbyToAct(part_olt_id, fail_info.llid_n, fail_info.llid_list_marker);
						if (VOS_OK != result)
						{
						    VOS_ASSERT(0);
							sys_console_printf("OLT_SetRdnLLIDStdbyToAct err,result:%d,%s %d\r\n",result,__FILE__,__LINE__);

							if( fail_info.llid_n == PON_REDUNDANCY_ONU_LLID_ALL )
							{
							    result = OLT_GetAllOnus( olt_id, &olt_onu_global_table); 
								number = olt_onu_global_table.onu_num;
						 	   	for (i = 0; i < number; i++)
					 	   		{
					 	   		    OLT_DeregisterLLID(olt_id, olt_onu_global_table.onus[i].llid, 0);
									OLT_DeregisterLLID(part_olt_id, olt_onu_global_table.onus[i].llid, 0);
					 	   		}
							}
							else
							{
								for(i = 0; i < fail_info.llid_n; i++)
								{
								    OLT_DeregisterLLID(olt_id, fail_info.llid_list_marker[i], 0);
									OLT_DeregisterLLID(part_olt_id, fail_info.llid_list_marker[i], 0);
								}
							}
							return result;
						}
					}
					#endif
                    /* 配置同步处理(确保配置、状态一致) */
					if( fail_info.llid_n == PON_REDUNDANCY_ONU_LLID_ALL )
					{
					    
                        result = OLT_GetAllOnus( olt_id, &olt_onu_global_table); 
						number = olt_onu_global_table.onu_num;
				 	   	for (i = 0; i < number; i++)
						{
                            result = OLT_IsExist(part_olt_id, &exist);
							/*对端pon口不存在或不发光，则onu离线*/
							if((VOS_OK != result) || !exist ||
								(PONPORT_ENABLE != OLTAdv_GetAdminStatus(part_olt_id)))
							{
							    ONU_SWITCH_DEBUG(OLT_SWITCH_TITLE"partern pon%d/%d not exist or not enable\r\n", PartSlot, PartPort );
								OLT_DeregisterLLID(olt_id, olt_onu_global_table.onus[i].llid, 0);
								OLT_DeregisterLLID(part_olt_id, olt_onu_global_table.onus[i].llid, 0);
								continue;
							}
							#if 1
							result = OLT_SetRdnLLIDMode (part_olt_id, olt_onu_global_table.onus[i].llid, PON_REDUNDANCY_LLID_REDUNDANCY_ACTIVE);
                            /*若为板拔出等olt remove事件，本端olt不在位，不去设置*/
							if(PON_REDUNDANCY_OLT_Removed != fail_info.reason)
							{
							    result = OLT_SetRdnLLIDMode (olt_id, olt_onu_global_table.onus[i].llid, PON_REDUNDANCY_LLID_REDUNDANCY_STANDBY);
							}
							if (result != REDUNDANCY_MNG_EXIT_OK)
							{
							    sys_console_printf("OLT_SetRdnLLIDMode err,result:%d\r\n",result);
								OLT_DeregisterLLID(olt_id, olt_onu_global_table.onus[i].llid, 0);
								OLT_DeregisterLLID(part_olt_id, olt_onu_global_table.onus[i].llid, 0);
								continue;
							}
							#endif
							
						    /*onuIdx = GetOnuIdxByLlid(olt_id, onu_paratbl.onus[i].llid);*/
							part_onuIdx = GetOnuIdxByLlid(part_olt_id, olt_onu_global_table.onus[i].llid);
							if(	(RERROR != part_onuIdx) && (ThisIsValidOnu(part_olt_id, part_onuIdx) == ROK))
							{
                                OnuEntry = part_olt_id*MAXONUPERPON + part_onuIdx;
							    onuIdx = GetOnuIdxByMacPerPon(olt_id, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr);
								if(RERROR == onuIdx)
								{
								    VOS_ASSERT(0);
									OLT_DeregisterLLID(olt_id, olt_onu_global_table.onus[i].llid, 0);
									OLT_DeregisterLLID(part_olt_id, olt_onu_global_table.onus[i].llid, 0);
									continue;
								}
								/*倒换发生拷贝时，不在拷贝llid*/
								/*CopyOnuConfigFromPon1ToPon2(olt_id, onuIdx, part_olt_id, part_onuIdx, OLT_COPYFLAGS_SYNC | OLT_COPYFLAGS_WITHLLID);*/
								CopyOnuConfigFromPon1ToPon2(olt_id, onuIdx, part_olt_id, part_onuIdx, OLT_COPYFLAGS_SYNC);
                                ONU_SWITCH_DEBUG("copy over from pon %d/%d/%d to pon %d/%d/%d\r\n", CardSlot, CardPort, onuIdx, PartSlot, PartPort, part_onuIdx );

								iResumeReason = ( 0 == PonPortTable[olt_id].swap_reason ) ? PON_ONU_DEREGISTRATION_HOST_REQUEST : PON_ONU_DEREGISTRATION_REPORT_TIMEOUT;
								/* 恢复原主口上的ONU离线信息及日志 */
								/*若为板拔出等olt remove事件，本端olt不在位，不去设置*/
							    if((PON_REDUNDANCY_OLT_Removed != fail_info.reason) && (PON_REDUNDANCY_OLT_CardPull!= fail_info.reason))
						    	{
			                        iRlt = OLT_ResumeLLIDStatus(olt_id, olt_onu_global_table.onus[i].llid, iResumeReason, OLT_RESUMEMODE_FORCEDOWN);
									if(VOS_ERROR == iRlt)
									{
									    OLT_DeregisterLLID(part_olt_id, olt_onu_global_table.onus[i].llid, 0);
										continue;
									}
						    	}
		                        /* 激活新主口上的ONU注册状态 */
		                        OLT_ResumeLLIDStatus(part_olt_id, olt_onu_global_table.onus[i].llid, iResumeReason, OLT_RESUMEMODE_SYNCHARD);
								 /* 快倒换的软、硬倒换告警 */
		                        Trap_PonAutoProtectSwitch(olt_id);
							}
							else
							{
							    VOS_ASSERT(0);
								sys_console_printf("The partern pon port%d  has not this llid:%d,%s %d\r\n",part_olt_id, olt_onu_global_table.onus[i].llid,__FILE__,__LINE__);
								OLT_DeregisterLLID(olt_id, olt_onu_global_table.onus[i].llid, 0);
								OLT_DeregisterLLID(part_olt_id, olt_onu_global_table.onus[i].llid, 0);
								continue;
							}
													
						}
					
					    
					}
					else if( fail_info.llid_n < PON_REDUNDANCY_ONU_LLID_ALL )
					{
						for(i = 0;i < fail_info.llid_n; i++)
						{
						    result = OLT_IsExist(part_olt_id, &exist);
							if((VOS_OK != result) || !exist ||
								(PONPORT_ENABLE != OLTAdv_GetAdminStatus(part_olt_id)))
							{
							    ONU_SWITCH_DEBUG("partern pon%d/%d not exist or not enable\r\n", PartSlot, PartPort );
								OLT_DeregisterLLID(olt_id,fail_info.llid_list_marker[i], 0);
								OLT_DeregisterLLID(part_olt_id, fail_info.llid_list_marker[i], 0);
								continue;
							}

							result = OLT_SetRdnLLIDMode (part_olt_id, fail_info.llid_list_marker[i], PON_REDUNDANCY_LLID_REDUNDANCY_ACTIVE);
							if((PON_REDUNDANCY_OLT_Removed != fail_info.reason) && (PON_REDUNDANCY_OLT_CardPull!= fail_info.reason))
							{
							    result = OLT_SetRdnLLIDMode (olt_id, fail_info.llid_list_marker[i], PON_REDUNDANCY_LLID_REDUNDANCY_STANDBY);
							}
							if (result != REDUNDANCY_MNG_EXIT_OK)
							{
							    sys_console_printf("OLT_SetRdnLLIDMode err,result:%d\r\n",result);
								OLT_DeregisterLLID(olt_id, fail_info.llid_list_marker[i], 0);
								OLT_DeregisterLLID(part_olt_id, fail_info.llid_list_marker[i], 0);
								continue;
							}
						    /*onuIdx = GetOnuIdxByLlid(olt_id, fail_info.llid_list_marker[i]);*/
							
							part_onuIdx = GetOnuIdxByLlid(part_olt_id, fail_info.llid_list_marker[i]);
							if((RERROR != part_onuIdx) && (ThisIsValidOnu(part_olt_id, part_onuIdx) == ROK))
							{
							    OnuEntry = part_olt_id*MAXONUPERPON + part_onuIdx;
							    onuIdx = GetOnuIdxByMacPerPon(olt_id, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr);
								if(RERROR == onuIdx)
								{
								    VOS_ASSERT(0);
									OLT_DeregisterLLID(olt_id, fail_info.llid_list_marker[i], 0);
									OLT_DeregisterLLID(part_olt_id, fail_info.llid_list_marker[i], 0);
									continue;
								}
								/*CopyOnuConfigFromPon1ToPon2(olt_id, onuIdx, part_olt_id, part_onuIdx, OLT_COPYFLAGS_SYNC | OLT_COPYFLAGS_WITHLLID);*/
								CopyOnuConfigFromPon1ToPon2(olt_id, onuIdx, part_olt_id, part_onuIdx, OLT_COPYFLAGS_SYNC );
                                ONU_SWITCH_DEBUG(OLT_SWITCH_TITLE"copy over from pon %d/%d/%d to pon %d/%d/%d\r\n", CardSlot, CardPort, onuIdx, PartSlot, PartPort, part_onuIdx );
								
                                iResumeReason = ( 0 == PonPortTable[olt_id].swap_reason ) ? PON_ONU_DEREGISTRATION_HOST_REQUEST : PON_ONU_DEREGISTRATION_REPORT_TIMEOUT;
								/* 恢复原主口上的ONU离线信息及日志 */
								/*若为板拔出等olt remove事件，本端olt不在位，不去设置*/
							    if((PON_REDUNDANCY_OLT_Removed != fail_info.reason) && (PON_REDUNDANCY_OLT_CardPull!= fail_info.reason))
						    	{
			                        iRlt = OLT_ResumeLLIDStatus(olt_id, fail_info.llid_list_marker[i], iResumeReason, OLT_RESUMEMODE_FORCEDOWN);
                                    if(VOS_ERROR == iRlt)
									{
									    OLT_DeregisterLLID(part_olt_id, fail_info.llid_list_marker[i], 0);
										continue;
									}
								}
		                        /* 激活新主口上的ONU注册状态 */
		                        OLT_ResumeLLIDStatus(part_olt_id, fail_info.llid_list_marker[i], iResumeReason, OLT_RESUMEMODE_SYNCHARD);
								
								 /* 快倒换的软、硬倒换告警 */
		                        Trap_PonAutoProtectSwitch(olt_id);
							}
							else
							{
							    VOS_ASSERT(0);
								sys_console_printf("The partern pon port%d (olt_id:%d) has not this llid:%d,%s %d\r\n",part_olt_id, olt_id, fail_info.llid_list_marker[i],__FILE__,__LINE__);
								result = OLT_SetRdnLLIDMode (olt_id, fail_info.llid_list_marker[i], PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL);
								result = OLT_SetRdnLLIDMode (part_olt_id, fail_info.llid_list_marker[i], PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL);
								OLT_DeregisterLLID(olt_id, fail_info.llid_list_marker[i], 0);
								OLT_DeregisterLLID(part_olt_id, fail_info.llid_list_marker[i], 0);
								continue;
							}
							
							
							
						}
					
   
					}
                    
                    
                }
                break;
                
                default:
                    VOS_ASSERT(0);
            }
                        
            break;
       
        default:
            VOS_ASSERT(0);
            return( RERROR );
    }
		
    return( ROK );
}

int PonOltLoose_OnuSwapHandler(short int PonPortIdx, short int PonPortIdx_Swap)
{
    
	PON_redundancy_msg_olt_failure_info_t fail_info;
	OnuSwitchInfo_S data;
	int llid_n = 0; 
	int OnuEntry = 0;
	int i = 0;
	short int llid = 0;
	short int part_onuIdx = 0;
	short int number = 0;
	int result = 0;
	int status = 0;
	
		
    /*sys_console_printf("Enter into PonOltLoose_OnuSwapHandler\r\n");*/
    VOS_MemSet((void *)&fail_info, 0, sizeof(fail_info));
	VOS_MemSet((void *)&data, 0, sizeof(OnuSwitchInfo_S));
	
	if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER)
	{
	    fail_info.reason = PON_REDUNDANCY_OLT_CardPull;
	    for(i = 0; i < MAXONUPERPON; i++)
		{
		    
		    OnuEntry = PonPortIdx_Swap*MAXONUPERPON + i;
			
	        llid = OnuMgmtTable[OnuEntry].LLID;
			status = OnuMgmtTable[OnuEntry].OperStatus;
	    	
			part_onuIdx = GetOnuIdxByLlid(PonPortIdx_Swap, llid);
			if((RERROR != part_onuIdx) && (ThisIsValidOnu(PonPortIdx_Swap, part_onuIdx) == ROK) &&
				(ONU_OPER_STATUS_DOWN == status || ONU_OPER_STATUS_POWERDOWN == status))
			{
			    fail_info.llid_list_marker[llid_n] = llid;
				llid_n++;
													
			}
			
		}
		fail_info.llid_n = llid_n;
		
		if(fail_info.llid_n <= 0)
		{
		    OLT_SWITCH_DEBUG("this pon has no swap onu\r\n");
			return VOS_OK;
		}
		
		 
		/*激活对端的llid*/
	    result = OLT_SetRdnLLIDStdbyToAct(PonPortIdx_Swap, fail_info.llid_n, fail_info.llid_list_marker);
		 
		if (VOS_OK != result)
		{
		    VOS_ASSERT(0);
			sys_console_printf("OLT_SetRdnLLIDStdbyToAct err,result:%d,%s %d\r\n",result,__FILE__,__LINE__);

			if( fail_info.llid_n == PON_REDUNDANCY_ONU_LLID_ALL )
			{
			    result = OLT_GetAllOnus( PonPortIdx, &olt_onu_global_table); 
				number = olt_onu_global_table.onu_num;
		 	   	for (i = 0; i < number; i++)
	 	   		{
	 	   		    OLT_DeregisterLLID(PonPortIdx, olt_onu_global_table.onus[i].llid, 0);
	 	   		}
			}
			else
			{
				for(i = 0; i < fail_info.llid_n; i++)
				{
				    OLT_DeregisterLLID(PonPortIdx, fail_info.llid_list_marker[i], 0);
				}
			}
			return result;
		}
		sendOnuSwitchEventMsg( PonPortIdx, PROTECT_SWITCHONU_EVENT_OVER, fail_info, PROTECT_SWITCH_EVENT_SRC_HARDWARE, PonPortSwapTimesQuery(PonPortIdx) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
	}
	else
	{
	    fail_info.reason = PON_REDUNDANCY_OLT_Removed;
		data.olt_id = PonPortIdx;
		data.event_id = PROTECT_SWITCHONU_EVENT_OVER;
		data.fail_info = fail_info;
		data.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
		
	    OnuSwitchHandler(&data);
	}
	
	
	
	/*sendOnuSwitchEventMsg( PonPortIdx, PROTECT_SWITCHONU_EVENT_OVER, fail_info, PROTECT_SWITCH_EVENT_SRC_HARDWARE, PonPortSwapTimesQuery(PonPortIdx) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );*/
	return VOS_OK;
}
#endif
/*End:for onu swap by jinhl@2013-02-22*/
/*****************************************************
 *
 *    Function:  PonAlarmHandler( PonAlarmInfo_S  *PonAlarmData )
 *
 *    Param:    
 *                 
 *    Desc:   
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 int  PonAlarmHandler( PonAlarmInfo_S  *PonAlarmData )
{
	short int	olt_id;
	short int	alarm_source_id;
	PON_alarm_t   alarm_type;
	short int	alarm_parameter;
	void     *alarm_data;
	int  threshold;
	double constDouble = 1e12;
	short int OnuIdx;

	/* modified by xieshl 20110804, 调试问题单12411时修改下列提示 */
	/*if( EVENT_ALARM == V2R1_ENABLE){
		sys_console_printf("\r\nAlarm event happened\r\n");	
		}*/
	
	if( PonAlarmData == NULL )
	{
		/*sys_console_printf("error: parameters pointer null (PonAlarmHandler() )\r\n" );*/
		VOS_ASSERT( 0 );
		return( RERROR );
	}
	if( PonAlarmData->alarm_type >= PON_ALARM_LAST_ALARM )
	{
		VOS_ASSERT( 0 );
		return( RERROR );
	}

	olt_id = PonAlarmData->olt_id;
	alarm_source_id = PonAlarmData->alarm_source_id;
	alarm_type = PonAlarmData->alarm_type;
	alarm_parameter = PonAlarmData->alarm_parameter;
	alarm_data = PonAlarmData->alarm_data;
	
	switch (alarm_type)
	{
	case PON_ALARM_BER:
		/* 
		the BER alarm may be originated from either one of two sources measurements:
		OLT - Uplink measurement. The alarm parameter is the BER meter which triggered the alarm
		ONU - Downlink measurement. No alarm parameter. Supported for Passave ONUs only. 
		*/
		/*----------------|------------------------------| 
		  |  olt Index            |    olt_id ( 0 ~ PON_MAX_OLT_ID ) |
		  |----------------|------------------------------|
		  |  alarm_source      | OLT(PON_OLT_ID) |  ONU_id         |
		  |----------------|------------------------------|
		  |  type                 |          PON_ALARM_BER                |
		  |----------------|------------------------------|
		  |  parameter          |      Onu_id            |    none(0)     |
		  |----------------|------------------------------|
		  |  data                  |        BER  measurement               |
		  |-----------------------------------------------|
		  */
		 
		if (alarm_source_id == PON_OLT_ID)
		{	
			threshold = (int )(*(PON_ber_alarm_data_t *)alarm_data) * (unsigned int )constDouble;
			/* modified by chenfj 2007-8-1 */
			/*Trap_PonPortBER( olt_id, threshold, V2R1_BER_ALARM );  PON_alarm_s */
			OnuIdx = GetOnuIdxByLlid( olt_id, alarm_parameter);	
			if( OnuIdx == RERROR ) return( RERROR  );
			if( EVENT_ALARM == V2R1_ENABLE ) 
				sys_console_printf (" UP-%s alarm regarding onu %d, value: %d\r\n", PON_alarm_s[alarm_type], OnuIdx+1, threshold ); 

			Trap_OnuPonPortBER(olt_id, OnuIdx, threshold , V2R1_BER_ALARM );
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[ olt_id * MAXONUPERPON + OnuIdx ].LastBerAlarmTime = VOS_GetTick();
			ONU_MGMT_SEM_GIVE;
			/*PonPortTable[olt_id].LastBerAlarmTime = VOS_GetTick();*/
		}
		else
		{
			threshold = (int )(*(PON_ber_alarm_data_t *)alarm_data) * (unsigned int )constDouble;
			OnuIdx = GetOnuIdxByLlid( olt_id, alarm_source_id);		
			if( OnuIdx == RERROR ) return( RERROR  );
			if( EVENT_ALARM == V2R1_ENABLE ) 
				sys_console_printf (" DOWN-%s alarm from onu %d, value: %d\r\n", PON_alarm_s[alarm_type], OnuIdx+1, threshold );
			Trap_OnuPonPortBER(olt_id, OnuIdx, threshold , V2R1_BER_ALARM );
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[ olt_id * MAXONUPERPON + OnuIdx ].LastBerAlarmTime = VOS_GetTick();
			ONU_MGMT_SEM_GIVE;
		}
		
		break;
		
	case PON_ALARM_FER:
		/*
		the alarm may be originated from either one of two sources measurements:
		OLT - Uplink measurement, FCS, in range length, and frame too long violations. the alarm Parameter
		is the FER meter which triggered the alarm. PON_ALL_ACTIVE_LLIDS special value is used for global 
		PON port HEC violations counter indication .
		ONU - Downlink measurement, FCS and HEC violations. No alarm parameter. Supported for Passave 
		         ONUs only
		*/
		
		/*----------------|------------------------------| 
		  |  olt Index            |    olt_id ( 0 ~ PON_MAX_OLT_ID ) |
		  |----------------|------------------------------|
		  |  alarm_source      | OLT(PON_OLT_ID) |  ONU_id         |
		  |----------------|------------------------------|
		  |  type                 |          PON_ALARM_FER                |
		  |----------------|------------------------------|
		  |  parameter          |           llid             |    none(0)     |( (0~PON_MAX_LLID_PER_OLT ) or PON_ALL_ACTIVE_LLIDS
		  |----------------|------------------------------|
		  |  data                  |        FER  measurement               |
		  |-----------------------------------------------|
		  */
		  
		if (alarm_source_id == PON_OLT_ID)
		{
			threshold = (int )(*(PON_ber_alarm_data_t *)alarm_data) *(unsigned int ) constDouble;
			if( alarm_parameter == PON_ALL_ACTIVE_LLIDS ) 
			{
				if( EVENT_ALARM  == V2R1_ENABLE)
					sys_console_printf(" UP-%s alarm from the global OLT, value: %d\r\n", PON_alarm_s[alarm_type], threshold);
				Trap_PonPortFER( olt_id,  threshold, V2R1_FER_ALARM ); 
				PonPortTable[olt_id].LastFerAlarmTime = VOS_GetTick();
			}
			else
			{
				/* modified by chenfj 2007-8-1 */
				OnuIdx = GetOnuIdxByLlid(olt_id, alarm_parameter);
				if( OnuIdx == RERROR ) return( RERROR  );
				if( EVENT_ALARM  == V2R1_ENABLE)
					sys_console_printf (" UP-%s alarm regarding onu %d, value: %Lg\r\n", PON_alarm_s[alarm_type], OnuIdx+1, *((PON_fer_alarm_data_t *)alarm_data)); 
				Trap_OnuPonPortFER( olt_id, OnuIdx, threshold , V2R1_FER_ALARM ); 
				ONU_MGMT_SEM_TAKE;
				OnuMgmtTable[ olt_id * MAXONUPERPON + OnuIdx ].LastFerAlarmTime = VOS_GetTick();
				ONU_MGMT_SEM_GIVE;
			}
		}
		else
		{
			threshold = (int )(*(PON_ber_alarm_data_t *)alarm_data) *(unsigned int ) constDouble;
			/* modified by chenfj 2007-8-1 */
			OnuIdx = GetOnuIdxByLlid(olt_id, alarm_source_id);
			if( OnuIdx == RERROR ) return( RERROR );
			if( EVENT_ALARM  == V2R1_ENABLE)
				sys_console_printf (" DOWN-%s alarm from onu %d, value: %d\r\n", PON_alarm_s[alarm_type], OnuIdx+1,  threshold); 
			Trap_OnuPonPortFER( olt_id,OnuIdx, threshold , V2R1_FER_ALARM ); 
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[ olt_id * MAXONUPERPON + OnuIdx ].LastFerAlarmTime = VOS_GetTick();
			ONU_MGMT_SEM_GIVE;
		}
		
		break;
		
	case PON_ALARM_SOFTWARE_ERROR:
		/****** 
		PAS-SOFT internal error. 
		Note: this alarm, as any other alarm, is not active during PAS_init() time( but only after activation of PAS_assign_handler_function() with alarm handler ).
		Because this alarm indicates the possibility of an OS-related problem, it must be addressed by the user.
		*/
		/*----------------|------------------------------| 
		  |  olt Index            |       PON_PAS_SOFT_ID               |  Alarm is not olt specific 
		  |----------------|------------------------------|
		  |  alarm_source      |            PAS_SOFT                      |
		  |----------------|------------------------------|
		  |  type                 |   PON_ALARM_SOFTWARE_ERROR   |
		  |----------------|------------------------------|
		  |  parameter          |                  0                            |
		  |----------------|------------------------------|
		  |  data                  |       0      |       1      |       2      |  software error code 
		  |-----------------------------------------------|
		  */
		if( EVENT_ALARM == V2R1_ENABLE)
		{
			unsigned int reason = *(int *)alarm_data;
			if( reason > 2 ) reason = 0;
			sys_console_printf (" %s alarm from collector %d (PAS_OLT_SOFT_ID %d), reason %s\r\n", PON_alarm_s[alarm_type], 
				alarm_source_id, olt_id , Pon_software_error_alarm_s[reason] ); 
		}
	
		break;
		
	case PON_ALARM_LOCAL_LINK_FAULT:
		/* OAM alarm, indicating that local device's receive path has detected a fault. */
		/*----------------|------------------------------| 
		  |  olt Index            |   olt_id ( 0 ~ PON_MAX_OLT_ID )  | 
		  |----------------|------------------------------|
		  |  alarm_source      |  onu_id  [OLT(PON_OLT_ID) ? ]      |
		  |----------------|------------------------------|
		  |  type                 |   PON_ALARM_LOCAL_LINK_FAULT  |
		  |----------------|------------------------------|
		  |  parameter          |                  0                            |
		  |----------------|------------------------------|
		  |  data                  |               empty(null)                  |
		  |-----------------------------------------------|
		  */
		if( EVENT_ALARM == V2R1_ENABLE)
			sys_console_printf (" %s alarm from collector %d\r\n", PON_alarm_s[alarm_type], alarm_source_id ); 

		break;
		
	case PON_ALARM_DYING_GASP:
		/* OAM alarm.
		    OAM V1.2 definition: Alarm is indicating that alarm source is about to crash due to internal error,
		    system error, load of data, power failure, or unrecoveralbe state .
		    OAM V2.0 definition: Alarm is indicating that an unrecoverable local failure condition has occurred.
		*/
		/*----------------|------------------------------| 
		  |  olt Index            |   olt_id ( 0 ~ PON_MAX_OLT_ID )  | 
		  |----------------|------------------------------|
		  |  alarm_source      |             onu_id                          |
		  |----------------|------------------------------|
		  |  type                 |        PON_ALARM_DYING_GASP      |
		  |----------------|------------------------------|
		  |  parameter          |                  0                            |
		  |----------------|------------------------------|
		  |  data                  |               empty(null)                  |
		  |-----------------------------------------------|
		  */
		
		OnuIdx = GetOnuIdxByLlid(olt_id, alarm_source_id);
        if( OnuIdx == RERROR ) return( RERROR  );
		if( EVENT_ALARM == V2R1_ENABLE)
			sys_console_printf (" %s alarm from ONU %d\r\n", PON_alarm_s[alarm_type], OnuIdx+1 );
		if( OnuIdx != RERROR) 
			Trap_onuPowerOff(olt_id, OnuIdx);

		break;
		
	case PON_ALARM_CRITICAL_EVENT:
		/* Note: this alarm type is supported by OAM 2.0 or higher.
		     this is an OAM alarm, indicating that a critical event has occurred. 
		*/
		/*----------------|------------------------------| 
		  |  olt Index            |   olt_id ( 0 ~ PON_MAX_OLT_ID )  | 
		  |----------------|------------------------------|
		  |  alarm_source      |             onu_id                          |
		  |----------------|------------------------------|
		  |  type                 |    PON_ALARM_CRITICAL_EVENT     |
		  |----------------|------------------------------|
		  |  parameter          |                  0                            |
		  |----------------|------------------------------|
		  |  data                  |               empty(null)                  |
		  |-----------------------------------------------|
		  */
		if( EVENT_ALARM == V2R1_ENABLE)
			sys_console_printf (" %s alarm from ONU %d\r\n", PON_alarm_s[alarm_type], alarm_source_id); 

		break;
		
	case PON_ALARM_REMOTE_STABLE:
		/*
			OAM alarm, indicating that Remote DTE has not seen or is unsatisfied with local state information 
		*/
		/*----------------|------------------------------| 
		  |  olt Index            |   olt_id ( 0 ~ PON_MAX_OLT_ID )  | 
		  |----------------|------------------------------|
		  |  alarm_source      |             onu_id                          |
		  |----------------|------------------------------|
		  |  type                 |    PON_ALARM_REMOTE_STABLE     |
		  |----------------|------------------------------|
		  |  parameter          |                  0                            |
		  |----------------|------------------------------|
		  |  data                  |               empty(null)                  |
		  |-----------------------------------------------|
		  */
		  if( EVENT_ALARM == V2R1_ENABLE)
			sys_console_printf (" %s alarm from ONU %d\r\n", PON_alarm_s[alarm_type], alarm_source_id);

		break;
		
	case PON_ALARM_LOCAL_STABLE:
		/*
			OAM alarm, indicating that Remote DTE has not seen or is unsatisfied with remote state information 
		*/
		/*----------------|------------------------------| 
		  |  olt Index            |   olt_id ( 0 ~ PON_MAX_OLT_ID )  | 
		  |----------------|------------------------------|
		  |  alarm_source      |            PON_OLT_ID                    |
		  |----------------|------------------------------|
		  |  type                 |    PON_ALARM_LOCAL_STABLE       |
		  |----------------|------------------------------|
		  |  parameter          |                  0                            |
		  |----------------|------------------------------|
		  |  data                  |               empty(null)                  |
		  |-----------------------------------------------|
		  */
		  if( EVENT_ALARM == V2R1_ENABLE)
			sys_console_printf (" %s alarm from ONU %d\r\n", PON_alarm_s[alarm_type], alarm_source_id);

		break;
	
	case PON_ALARM_OAM_VENDOR_SPECIFIC:
		/*
			system vendor specific OAM alarm( detected from external ONU/ONU host input lines), supported for Passave ONUs only.
		*/
		/*----------------|------------------------------| 
		  |  olt Index            |   olt_id ( 0 ~ PON_MAX_OLT_ID )  | 
		  |----------------|------------------------------|
		  |  alarm_source      |                onu_id                       |
		  |----------------|------------------------------|
		  |  type                 |   PON_ALARM_OAM_VENDOR_SPECIFIC   |
		  |----------------|------------------------------|
		  |  parameter          |                  0-4                          | Number of vendor specific alarm
		  |----------------|------------------------------|
		  |  data                  |               empty(null)                  |
		  |-----------------------------------------------|
		  */
		
		if( EVENT_ALARM == V2R1_ENABLE){
			sys_console_printf (" %s %s alarm from ONU %d\r\n", PON_alarm_s[alarm_type], 
				PON_oam_vendor_specific_alarm_s[alarm_parameter], alarm_source_id); 
		}

		break;

	case PON_ALARM_ERRORED_SYMBOL_PERIOD:
		if( EVENT_ALARM == V2R1_ENABLE)
			sys_console_printf (" %s alarm from ONU %d\r\n", PON_alarm_s[alarm_type], alarm_source_id); 

		break;
		
	case PON_ALARM_ERRORED_FRAME:
		if( EVENT_ALARM == V2R1_ENABLE )
			sys_console_printf (" %s alarm from ONU %d\r\n", PON_alarm_s[alarm_type], alarm_source_id);

		break;
	case PON_ALARM_ERRORED_FRAME_PERIOD:
		if( EVENT_ALARM == V2R1_ENABLE)
			sys_console_printf (" %s alarm from ONU %d\r\n", PON_alarm_s[alarm_type], alarm_source_id);

		break;
	case PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY:
		if( EVENT_ALARM == V2R1_ENABLE)
			sys_console_printf (" %s alarm from ONU %d\r\n", PON_alarm_s[alarm_type], alarm_source_id);

		break;

	/******
	Indication that an error occurred during the ONU registration process.
	(for example, ONU MAC address already exists, or PAS-SOFT DB cannot be updated)
	In such a scenario, as a result of this error, the ONU registration event will not be raised.
     PAS-SOFT reactions to the alarm:
       1  Deregister the ONU
       2  Report the alarm.
	******/
	case PON_ALARM_ONU_REGISTRATION_ERROR:
		/* Error on ONU registration process */
		/*----------------|------------------------------| 
		  |  olt Index            |   olt_id ( 0 ~ PON_MAX_OLT_ID )  | 
		  |----------------|------------------------------|
		  |  alarm_source      |                                                |
		  |----------------|------------------------------|
		  |  type                 |   PON_ALARM_ONU_REGISTRATION_ERROR   |
		  |----------------|------------------------------|
		  |  parameter          |                  onuIdx                      | 
		  |----------------|------------------------------|
		  |  data                  |             ONU MAC addr               |
		  |-----------------------------------------------|
		  */
		if( EVENT_ALARM == V2R1_ENABLE)			
			sys_console_printf (" %s alarm from LLID %d(%s)\r\n", PON_alarm_s[alarm_type], alarm_parameter, macAddress_To_Strings(alarm_data) );
		
		break;

	case PON_ALARM_OAM_LINK_DISCONNECTION:
		if( EVENT_ALARM == V2R1_ENABLE)
			sys_console_printf (" %s alarm from ONU %d\r\n", PON_alarm_s[alarm_type], alarm_source_id );

		break;
		
	case PON_ALARM_BAD_ENCRYPTION_KEY:
		if( EVENT_ALARM == V2R1_ENABLE)
			sys_console_printf (" %s alarm regarding LLID %d\r\n", PON_alarm_s[alarm_type], alarm_parameter );

		break;
	/** 
	Indication that an LLID transmitted frame(s) to the PON when it was not granted.
	******/
	case PON_ALARM_LLID_MISMATCH:
		if( EVENT_ALARM == V2R1_ENABLE)
			sys_console_printf (" %s alarm regarding LLID %d, frames %d mismatches\r\n", PON_alarm_s[alarm_type], alarm_parameter, *(int *)alarm_data );
		
		break;
	/**** 
		This alarm is raised when (N+1) ONU attempts to register to an OLT that support only (N) ONUs.
		Default alarm activation setting: Enabled*/
	case PON_ALARM_TOO_MANY_ONU_REGISTERING:
		/* 
		modified by chenfj 2007/03/16 
		#3806问题单:PON口下插上第65个ONU时，串口打印大量信息，显示速度太快
		*/
		{
		ULONG ti = time(0);
		ULONG seconds = ti - PonPortTable[olt_id].TooManyOnuRegisterLastAlarmTime;
		if( seconds >=  PON_ALARM_TIME_GAP )
		{
			sys_console_printf (" %s alarm\r\n"); 
			PonPortTable[olt_id].TooManyOnuRegisterLastAlarmTime = ti;
		}
		}
		break;

	case PON_ALARM_PORT_BER:
		PonPortOther_EventReport( /*CardSlot_s[GetCardIdxByPonChip(olt_id)]*/GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id),  other_pon_cni_ber);
		/*sys_console_printf("%s/%d CNI ber alarm\r\n", CardSlot_s[GetCardIdxByPonChip(olt_id)], GetPonPortByPonChip(olt_id) );*/
		break;

	/*****
	Indication that the device has had a fatal error and was reset.
	******/
	case PON_ALARM_DEVICE_FATAL_ERROR:
		/*modified by chenfj 2008-10-6
	    YaoLi 的环境中出现如此现象:
	    执行show pon port information 时,会出现"pon is not working" 提示; 但ONU仍能注册/离线.
	    说明PON 口是激活的. 查看告警日志,  发现发现PON口曾经发生过abnormal
	    告警. 
	    按PAS的解释, "PON_ALARM_DEVICE_FATAL_ERROR" 应是一种致命告警.
	    但为什么ONU 仍能注册? 	    
	    */	
		if(EVENT_DEBUG == V2R1_ENABLE)
			sys_console_printf(" %s alarm para=%d, data=%d\r\n", PON_alarm_s[alarm_type], alarm_parameter, *(unsigned char *)alarm_data);
		/*
		PonAbnormalHandler(olt_id, 0);
		*/
		break;	

	/*********
	 1  alarm is disabled for default
	 2  Indication that there is ONU with laser is always on.
	     when alarm_source_id = -1, indicate more then one ONU are transmitd permanently.
	 ********/
	case PON_ALARM_VIRTUAL_SCOPE_ONU_LASER_ALWAYS_ON:
		
		if(alarm_parameter == INVALID_LLID)
			sys_console_printf(" %s alarm\r\n", PON_alarm_s[alarm_type] );
		else
		{
			OnuIdx = GetOnuIdxByLlid(olt_id, alarm_parameter);
			if( OnuIdx != RERROR) 
				sys_console_printf(" %s alarm from onu%d\r\n", PON_alarm_s[alarm_type], (OnuIdx+1)); 
		}	

		break;
		
	/********
	1  alarm is disabled for default
	2  Indication that there is an ONU with signal degradation.
	********/
	case PON_ALARM_VIRTUAL_SCOPE_ONU_SIGNAL_DEGRADATION:
		
		OnuIdx = GetOnuIdxByLlid(olt_id, alarm_parameter);
		if( OnuIdx != RERROR) 
			sys_console_printf(" %s alarm from onu%d\n", PON_alarm_s[alarm_type], (OnuIdx+1));
		else
			sys_console_printf(" %s\r\n", PON_alarm_s[alarm_type]);

		break;

#ifdef  PAS_SOFT_VERSION_V5_3_5
	/******
	1 Default alarm activation setting: Disabled
	2 Indication that there is an ONU with optics at EOL (End of Life).
	
	******/
	case PON_ALARM_VIRTUAL_SCOPE_ONU_EOL:
        {
			PON_virtual_scope_onu_eol_t eol_entry = *((PON_virtual_scope_onu_eol_t*)alarm_data);
			sys_console_printf(" %s alarm regarding LLID %d(%s) Reference dBm:%0.1f  Elapsed sample days:%d\n",
					 PON_alarm_s[alarm_type], 
					 alarm_parameter, 
					 macAddress_To_Strings(eol_entry.mac_address), 
					 eol_entry.reference_dbm,
					 eol_entry.elapsed_sample_days); 
		}
		break;

	/********
	1  Default alarm activation setting: Enabled
	2  This alarm is raised when cannot add entry to ONU EOL database, because the database is full
	3  the max database size is 2032 entries (16 OLTs x 127 ONUs).
	*********/
	case PON_ALARM_VIRTUAL_SCOPE_ONU_EOL_DATABASE_IS_FULL:
        	{
			sys_console_printf(" %s alarm regarding LLID %d(%s). EOL database is full, cannot add this ONU to database.\n",
					 PON_alarm_s[alarm_type], 
					 alarm_parameter, 
					 macAddress_To_Strings(alarm_data) ); 
			
		    /*FILL_SW_RETURN_BUFFER_WITH_DEFAULT_ALARM_INFORMATION*/
		}
		break;
#endif
#ifdef  PAS_SOFT_VERSION_V5_3_13
	case PON_ALARM_ONU_REGISTERING_WITH_EXISTING_MAC:
		break;
#endif
	default:
		/*sys_console_printf (" Unknown alarm from ONU %d alarm type: %d\r\n"
				,alarm_source_id, alarm_type ); */
		break;
	}

	return( ROK );
	
}


#ifdef ALARM_CONFIG
#endif

/*****************************************************
 *
 *    Function:  PonBerAlarm( hort int PonPortIdx )
 *
 *    Param:    
 *                 
 *    Desc:   
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int PonBERAlarmConfigurationIsDefault(short int PonPortIdx )
{
	short int ret = ROK;

	CHECK_PON_RANGE

	if( PonDefaultAlarmConfiguration.ber_alarm_active != PonPortTable[PonPortIdx].AlarmConfigInfo.ber_alarm_active )
		return( RERROR );
	if( PonDefaultAlarmConfiguration.ber_alarm_configuration.ber_threshold != PonPortTable[PonPortIdx ].AlarmConfigInfo.ber_alarm_configuration.ber_threshold )
		return( RERROR );
	if( PonDefaultAlarmConfiguration.ber_alarm_configuration.direction != PonPortTable[PonPortIdx].AlarmConfigInfo.ber_alarm_configuration.direction )
		return( RERROR );
	if( PonDefaultAlarmConfiguration.ber_alarm_configuration.minimum_error_bytes_threshold != PonPortTable[PonPortIdx].AlarmConfigInfo.ber_alarm_configuration.minimum_error_bytes_threshold )
		return( RERROR );

	return( ret );
}

int  PonFERAlarmConfigurationIsDefault(short int PonPortIdx )
{
	short int ret = ROK;

	CHECK_PON_RANGE

	if( PonDefaultAlarmConfiguration.fer_alarm_active != PonPortTable[PonPortIdx].AlarmConfigInfo.fer_alarm_active )
		return( RERROR );
	if( PonDefaultAlarmConfiguration.fer_alarm_configuration.fer_threshold != PonPortTable[PonPortIdx].AlarmConfigInfo.fer_alarm_configuration.fer_threshold )
		return( RERROR );
	if( PonDefaultAlarmConfiguration.fer_alarm_configuration.direction != PonPortTable[PonPortIdx].AlarmConfigInfo.fer_alarm_configuration.direction )
		return( RERROR );
	if( PonDefaultAlarmConfiguration.fer_alarm_configuration.minimum_error_frames_threshold != PonPortTable[PonPortIdx].AlarmConfigInfo.fer_alarm_configuration.minimum_error_frames_threshold )
		return( RERROR );

	return( ret );
}

int SetPonBerAlarm( short int PonPortIdx )
{
	short int PonChipType;
	/*short int ret;*/
	

	CHECK_PON_RANGE
	
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	if( PonChipType == RERROR ) return( RERROR );

	/*if( PonBERAlarmConfigurationIsDefault( PonPortIdx ) == ROK ) return( ROK );*/
	
	if( PonChipType == PONCHIP_PAS ){
		/*
		PonPortTable[PonPortIdx].AlarmConfigInfo.ber_alarm_active = PonDefaultAlarmConfiguration.ber_alarm_active;
		VOS_MemCpy( &PonPortTable[PonPortIdx].AlarmConfigInfo.ber_alarm_configuration, &PonDefaultAlarmConfiguration.ber_alarm_configuration, sizeof(PON_ber_alarm_configuration_t ));
		*/
		unsigned int BerAlarmEnable;
		
		monOnuBerAlmEnGet( PonPortIdx, &BerAlarmEnable);
		monOnuBerAlmEnSet( PonPortIdx, BerAlarmEnable );

		monPonBerAlmEnGet( PonPortIdx, &BerAlarmEnable);
		monPonBerAlmEnSet( PonPortIdx, BerAlarmEnable );

		return( ROK );
		
		/*ret = PAS_set_alarm_configuration( PonPortIdx, PON_OLT_ID, PON_ALARM_BER, PonPortTable[PonPortIdx].AlarmConfigInfo.ber_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.ber_alarm_configuration);*/
		}
	else {

		}
	
	return( ROK );
}

int SetPonFerAlarm( short int PonPortIdx  )
{
	short int PonChipType;

	CHECK_PON_RANGE
	
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );	
	
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		/*
		PonPortTable[PonPortIdx].AlarmConfigInfo.fer_alarm_active = PonDefaultAlarmConfiguration.fer_alarm_active;
		VOS_MemCpy( &PonPortTable[PonPortIdx].AlarmConfigInfo.fer_alarm_configuration, &PonDefaultAlarmConfiguration.fer_alarm_configuration, sizeof(PON_fer_alarm_configuration_t ));
		*/
		unsigned int FerAlarmEnable;
		
		monOnuFerAlmEnGet( PonPortIdx, &FerAlarmEnable);
		monOnuFerAlmEnSet( PonPortIdx, FerAlarmEnable );

		monPonFerAlmEnGet( PonPortIdx, &FerAlarmEnable);
		monPonFerAlmEnSet( PonPortIdx, FerAlarmEnable );

		return( ROK );

		/*
		if( PAS_EXIT_OK == PAS_set_alarm_configuration( PonPortIdx, PON_OLT_ID, PON_ALARM_FER, PonPortTable[PonPortIdx].AlarmConfigInfo.fer_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.fer_alarm_configuration))
			return( ROK );
		else return( RERROR );
		*/
		}
	
	else{ /* other pon chip handler */

		}
	
	return( RERROR );

}


int SetPASSoftwareErrorAlarm()
{
    #if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	if( PAS_EXIT_OK == OLT_SetAlarmConfig( PON_OLT_ID, PON_OLT_ID, PON_ALARM_SOFTWARE_ERROR, TRUE, ( void *) NULL , 0))
	#else
	if( PAS_EXIT_OK == PAS_set_alarm_configuration( PON_OLT_ID, PON_OLT_ID, PON_ALARM_SOFTWARE_ERROR, TRUE, ( void *) NULL ))
	#endif
		return( ROK);
	else return( RERROR );
}

int SetPonLocalLinkAlarm( short int PonPortIdx )
{
	short int PonChipType;
	
	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if(PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_LOCAL_LINK_FAULT, PonPortTable[PonPortIdx].AlarmConfigInfo.local_stable_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.local_link_fault_alarm_configuration, sizeof(PON_link_fault_alarm_configuration_t) ) )
		#else
		if(PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_LOCAL_LINK_FAULT, PonPortTable[PonPortIdx].AlarmConfigInfo.local_stable_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.local_link_fault_alarm_configuration ) )
		#endif
			return( ROK );
		else return( RERROR );
		}
	
	else{ /* other pon chip handler */
		}
	return( RERROR );
}

int SetPonDyingGaspAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if( PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_DYING_GASP, PonPortTable[PonPortIdx].AlarmConfigInfo.dying_gasp_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.dying_gasp_alarm_configuration,sizeof(PON_dying_gasp_alarm_configuration_t)))
		#else
		if( PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_DYING_GASP, PonPortTable[PonPortIdx].AlarmConfigInfo.dying_gasp_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.dying_gasp_alarm_configuration))
		#endif
			return( ROK );
		else return( RERROR );
		}
	
	else{ /* other pon chip handler */
		}
	return( RERROR );
}

int SetPonCriticalEventAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if(OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if( PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_CRITICAL_EVENT, PonPortTable[PonPortIdx].AlarmConfigInfo.critical_event_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.critical_event_alarm_configuration, sizeof(PON_critical_event_alarm_configuration_t)))
		#else
		if( PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_CRITICAL_EVENT, PonPortTable[PonPortIdx].AlarmConfigInfo.critical_event_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.critical_event_alarm_configuration))
		#endif
			return( ROK );
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}			
	return( RERROR );
}

int SetPonRemoteStableAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if( PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_REMOTE_STABLE, PonPortTable[PonPortIdx].AlarmConfigInfo.remote_stable_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.remote_stable_alarm_configuration, sizeof(PON_remote_stable_alarm_configuration_t)))
		#else
		if( PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_REMOTE_STABLE, PonPortTable[PonPortIdx].AlarmConfigInfo.remote_stable_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.remote_stable_alarm_configuration))
		#endif
			return( ROK );
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );
}

int SetPonLocalStableAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if(PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_LOCAL_STABLE, PonPortTable[PonPortIdx].AlarmConfigInfo.local_stable_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.local_stable_alarm_configuration,sizeof(PON_local_stable_alarm_configuration_t)))
		#else
		if(PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_LOCAL_STABLE, PonPortTable[PonPortIdx].AlarmConfigInfo.local_stable_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.local_stable_alarm_configuration))
		#endif
			return( ROK );
		else return( RERROR );
		}
	else { /* other pon chip handler */
		}
	return( RERROR );
}

int SetPonOamVendorAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if( PAS_EXIT_OK  == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_OAM_VENDOR_SPECIFIC, PonPortTable[PonPortIdx].AlarmConfigInfo.oam_vendor_specific_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.oam_vendor_specific_alarm_configuration,sizeof(PON_oam_vendor_specific_alarm_configuration_t)))
		#else
		if( PAS_EXIT_OK  == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_OAM_VENDOR_SPECIFIC, PonPortTable[PonPortIdx].AlarmConfigInfo.oam_vendor_specific_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.oam_vendor_specific_alarm_configuration))
		#endif
			return( ROK );
		else return( RERROR );
		}
	else{ /* other pon chip handler */
		}
	return( RERROR );
}

/***** follow four function may be need to ONU also  *****/
int SetPonErrorSymbolAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if(PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_SYMBOL_PERIOD, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_symbol_period_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_symbol_period_alarm_configuration, sizeof(PON_errored_symbol_period_alarm_configuration_t) ))
		#else
		if(PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_SYMBOL_PERIOD, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_symbol_period_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_symbol_period_alarm_configuration ))
		#endif
			return( ROK );
		else return( RERROR );
		}
	else {  /* other pon chip handler */
		}
	return( RERROR );
}

int SetPonErrorFrameAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if(PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_FRAME, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_alarm_configuration, sizeof(PON_errored_frame_alarm_configuration_t) ))
		#else
		if(PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_FRAME, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_alarm_configuration ))
		#endif
			return( ROK );
		else{
			}
		}
	else{ /* other pon chip handler */
		}
	return( RERROR );
}

int SetPonErrorFramePeriodAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if(PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_FRAME_PERIOD, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_period_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_period_alarm_configuration, sizeof(PON_errored_frame_period_alarm_configuration_t) ))
		#else
		if(PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_FRAME_PERIOD, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_period_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_period_alarm_configuration ))
		#endif
			return( ROK );
		else return( RERROR );
		}
	else{ /* other pon chip handler */
		}
	return( RERROR );
}

int SetPonErrorFrameSecondsAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if(PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_seconds_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_seconds_alarm_configuration, sizeof(PON_errored_frame_seconds_alarm_configuration_t) ))
		#else
		if(PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_seconds_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_seconds_alarm_configuration ))
		#endif
			return( ROK );
		else return( RERROR );
		}
	else{ /* other pon chip handler */
		}
	return( RERROR ); 
}
int  PAS_set_alarm_config_ONU( short int PonPortIdx, short int OnuIdx , unsigned char active )
{
	short int ret;
	short int onu_id;
	short int PonChipType;
	
	CHECK_ONU_RANGE

	
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);

	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		ret = OLT_SetAlarmConfig(PonPortIdx, onu_id, PON_ALARM_ERRORED_SYMBOL_PERIOD, active /*PonPortTable[PonPortIdx].AlarmConfigInfo.errored_symbol_period_alarm_active*/, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_symbol_period_alarm_configuration, sizeof(PON_errored_symbol_period_alarm_configuration_t) );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n10 Set errored sysmol err\r\n");
			return( RERROR );
			}
		ret = OLT_SetAlarmConfig(PonPortIdx, onu_id, PON_ALARM_ERRORED_FRAME,  active /*PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_alarm_active*/, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_alarm_configuration, sizeof(PON_errored_frame_alarm_configuration_t) );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n11 Set errored frame err\r\n");
			return( RERROR );
			}
		ret = OLT_SetAlarmConfig(PonPortIdx, onu_id, PON_ALARM_ERRORED_FRAME_PERIOD, active /* PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_period_alarm_active*/, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_period_alarm_configuration, sizeof(PON_errored_frame_period_alarm_configuration_t) );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n12 Set errored frame period err\r\n");
			return( RERROR );
			}
		ret = OLT_SetAlarmConfig(PonPortIdx, onu_id, PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY, active /*PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_seconds_alarm_active*/, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_seconds_alarm_configuration, sizeof(PON_errored_frame_seconds_alarm_configuration_t) );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n13 Set errored frame seconds err\r\n");
			return( RERROR );
			}
		#else
		ret = PAS_set_alarm_configuration(PonPortIdx, onu_id, PON_ALARM_ERRORED_SYMBOL_PERIOD, active /*PonPortTable[PonPortIdx].AlarmConfigInfo.errored_symbol_period_alarm_active*/, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_symbol_period_alarm_configuration );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n10 Set errored sysmol err\r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration(PonPortIdx, onu_id, PON_ALARM_ERRORED_FRAME,  active /*PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_alarm_active*/, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_alarm_configuration );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n11 Set errored frame err\r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration(PonPortIdx, onu_id, PON_ALARM_ERRORED_FRAME_PERIOD, active /* PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_period_alarm_active*/, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_period_alarm_configuration );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n12 Set errored frame period err\r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration(PonPortIdx, onu_id, PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY, active /*PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_seconds_alarm_active*/, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_seconds_alarm_configuration );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n13 Set errored frame seconds err\r\n");
			return( RERROR );
			}
		 #endif
		return( ROK );
		}
	else { /* other pon chip handler */


		}
	return( RERROR );
}
/***** end *****/

int SetPonRegisterErrorAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if( PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_ONU_REGISTRATION_ERROR, PonPortTable[PonPortIdx].AlarmConfigInfo.onu_registration_error_alarm_active, (void *)NULL, 0 ))
		#else
		if( PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_ONU_REGISTRATION_ERROR, PonPortTable[PonPortIdx].AlarmConfigInfo.onu_registration_error_alarm_active, (void *)NULL ))
		#endif
			return( ROK );
		else return( RERROR );
		}
	else{ /* other pon chip handler */
		}
	return( RERROR ); 
}

int SetPonOamLinkDisconnectAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if(PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_OAM_LINK_DISCONNECTION, PonPortTable[PonPortIdx].AlarmConfigInfo.oam_link_disconnection_alarm_active, (void *)NULL, 0 ))
		#else
		if(PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_OAM_LINK_DISCONNECTION, PonPortTable[PonPortIdx].AlarmConfigInfo.oam_link_disconnection_alarm_active, (void *)NULL ))
		#endif
			return( ROK );
		else return( RERROR );
		}
	else{  /* other pon chip handler */
		}
	return( RERROR ); 
}

int SetPonBadEncryptKeyAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if(PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_BAD_ENCRYPTION_KEY, PonPortTable[PonPortIdx].AlarmConfigInfo.bad_encryption_key_alarm_active, (void *)NULL, 0 ))
		#else
		if(PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_BAD_ENCRYPTION_KEY, PonPortTable[PonPortIdx].AlarmConfigInfo.bad_encryption_key_alarm_active, (void *)NULL ))
		#endif
			return( ROK );
		else return( RERROR );
		}
	else{ /* other pon chip handler */
		}
	return( RERROR ); 
}


int SetPonLlidMismatchAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if(PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_LLID_MISMATCH, PonPortTable[PonPortIdx].AlarmConfigInfo.llid_mismatch_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.llid_mismatch_alarm_configuration, sizeof(PON_llid_mismatch_alarm_configuration_t) ))
		#else
		if(PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_LLID_MISMATCH, PonPortTable[PonPortIdx].AlarmConfigInfo.llid_mismatch_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.llid_mismatch_alarm_configuration ))
		#endif
			return( ROK );
		else return( RERROR );
		}
	else{ /* other pon chip handler */
		}
	return( RERROR); 
}

int SetPonTooManyOnuAlarm( short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if(PAS_EXIT_OK == OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_TOO_MANY_ONU_REGISTERING, PonPortTable[PonPortIdx].AlarmConfigInfo.too_many_onus_registering_alarm_active, (void *)NULL, 0))
		#else
		if(PAS_EXIT_OK == PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_TOO_MANY_ONU_REGISTERING, PonPortTable[PonPortIdx].AlarmConfigInfo.too_many_onus_registering_alarm_active, (void *)NULL))
		#endif
			return( ROK );
		else return( RERROR );
		}
	else{ /* other pon chip handler */
		}
	return( RERROR ); 
}

extern short int monOnuBerAlmEnSet(unsigned short oltId, /*unsigned short onuId,*/ unsigned int berAlmEn);
extern short int monOnuBerAlmEnGet(unsigned short oltId, /*unsigned short onuId, */unsigned int *pBerAlmEn);

int SetPonAlarmConfig( short int PonPortIdx )
{
	short int ret;
	short int PonChipType;
	/*short int PonChipVer;*/
	
	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );	
	
	if( OLTAdv_IsExist( PonPortIdx ) != TRUE ) return( RERROR );

	/*
	SetPonBerAlarm( PonPortIdx );
	SetPonFerAlarm( PonPortIdx );*/

	if( OLT_PONCHIP_ISPAS(PonChipType) ){
		

#if 0
		ret = PAS_set_alarm_configuration( PonPortIdx, PON_OLT_ID, PON_ALARM_BER, TRUE /* PonPortTable[PonPortIdx].AlarmConfigInfo.ber_alarm_active*/, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.ber_alarm_configuration);
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n1 Set Ber Alarm err \r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration( PonPortIdx, PON_OLT_ID, PON_ALARM_FER, PonPortTable[PonPortIdx].AlarmConfigInfo.fer_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.fer_alarm_configuration);
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n2 Set Fer Alarm err \r\n");
			return( RERROR );
			}

		ret = PAS_set_alarm_configuration( PON_OLT_ID, PON_OLT_ID, PON_ALARM_SOFTWARE_ERROR, TRUE, ( void *) NULL );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n3 Set Software Alarm err \r\n");
			return( RERROR );
			}
#endif
        #if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		ret = OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_LOCAL_LINK_FAULT, PonPortTable[PonPortIdx].AlarmConfigInfo.local_stable_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.local_link_fault_alarm_configuration, sizeof(PON_link_fault_alarm_configuration_t) );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n4 Set local link err\r\n");
			return( RERROR );
			}

		ret = OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_DYING_GASP, PonPortTable[PonPortIdx].AlarmConfigInfo.dying_gasp_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.dying_gasp_alarm_configuration, sizeof(PON_dying_gasp_alarm_configuration_t));
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n5 Set dying gasp err\r\n");
			return( RERROR );
			}

		ret = OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_CRITICAL_EVENT, PonPortTable[PonPortIdx].AlarmConfigInfo.critical_event_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.critical_event_alarm_configuration, sizeof(PON_critical_event_alarm_configuration_t));
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n6 Set critical event err\r\n");
			return( RERROR );
			}

#if 0
		ret = OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_REMOTE_STABLE, PonPortTable[PonPortIdx].AlarmConfigInfo.remote_stable_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.remote_stable_alarm_configuration, sizeof(PON_remote_stable_alarm_configuration_t));
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n7 Set remote stable err \r\n");
			return( RERROR );
			}

		ret = OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_LOCAL_STABLE, PonPortTable[PonPortIdx].AlarmConfigInfo.local_stable_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.local_stable_alarm_configuration, sizeof(PON_local_stable_alarm_configuration_t));
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n8 Set local stable err \r\n");
			return( RERROR );
			}

		ret = OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_OAM_VENDOR_SPECIFIC, PonPortTable[PonPortIdx].AlarmConfigInfo.oam_vendor_specific_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.oam_vendor_specific_alarm_configuration, sizeof(PON_oam_vendor_specific_alarm_configuration_t));
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n9 Set oam vendor err \r\n");
			return( RERROR );
			}
#endif

		ret = OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_SYMBOL_PERIOD, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_symbol_period_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_symbol_period_alarm_configuration, sizeof(PON_errored_symbol_period_alarm_configuration_t));
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n10 Set errored sysmol err\r\n");
			return( RERROR );
			}
		ret = OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_FRAME, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_alarm_configuration, sizeof(PON_errored_frame_alarm_configuration_t) );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n11 Set errored frame err\r\n");
			return( RERROR );
			}
		ret = OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_FRAME_PERIOD, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_period_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_period_alarm_configuration, sizeof(PON_errored_frame_period_alarm_configuration_t) );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n12 Set errored frame period err\r\n");
			return( RERROR );
			}
		ret = OLT_SetAlarmConfig(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_seconds_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_seconds_alarm_configuration, sizeof(PON_errored_frame_seconds_alarm_configuration_t));
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n13 Set errored frame seconds err\r\n");
			return( RERROR );
			}
		#else
		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_LOCAL_LINK_FAULT, PonPortTable[PonPortIdx].AlarmConfigInfo.local_stable_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.local_link_fault_alarm_configuration );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n4 Set local link err\r\n");
			return( RERROR );
			}

		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_DYING_GASP, PonPortTable[PonPortIdx].AlarmConfigInfo.dying_gasp_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.dying_gasp_alarm_configuration);
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n5 Set dying gasp err\r\n");
			return( RERROR );
			}

		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_CRITICAL_EVENT, PonPortTable[PonPortIdx].AlarmConfigInfo.critical_event_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.critical_event_alarm_configuration);
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n6 Set critical event err\r\n");
			return( RERROR );
			}
#if 0
		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_REMOTE_STABLE, PonPortTable[PonPortIdx].AlarmConfigInfo.remote_stable_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.remote_stable_alarm_configuration);
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n7 Set remote stable err \r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_LOCAL_STABLE, PonPortTable[PonPortIdx].AlarmConfigInfo.local_stable_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.local_stable_alarm_configuration);
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n8 Set local stable err \r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_OAM_VENDOR_SPECIFIC, PonPortTable[PonPortIdx].AlarmConfigInfo.oam_vendor_specific_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.oam_vendor_specific_alarm_configuration);
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n9 Set oam vendor err \r\n");
			return( RERROR );
			}
#endif

		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_SYMBOL_PERIOD, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_symbol_period_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_symbol_period_alarm_configuration );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n10 Set errored sysmol err\r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_FRAME, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_alarm_configuration );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n11 Set errored frame err\r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_FRAME_PERIOD, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_period_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_period_alarm_configuration );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n12 Set errored frame period err\r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY, PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_seconds_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.errored_frame_seconds_alarm_configuration );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n13 Set errored frame seconds err\r\n");
			return( RERROR );
			}
		#endif

#if 0
		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_ONU_REGISTRATION_ERROR, PonPortTable[PonPortIdx].AlarmConfigInfo.onu_registration_error_alarm_active, (void *)NULL );
		if(ret != PAS_EXIT_OK){ 
			sys_console_printf("\r\n14 Set onu register err \r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_OAM_LINK_DISCONNECTION, PonPortTable[PonPortIdx].AlarmConfigInfo.oam_link_disconnection_alarm_active, (void *)NULL );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n15 Set oam link disconnection err \r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_BAD_ENCRYPTION_KEY, PonPortTable[PonPortIdx].AlarmConfigInfo.bad_encryption_key_alarm_active, (void *)NULL );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n16 Set bad encryption key err \r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_LLID_MISMATCH, PonPortTable[PonPortIdx].AlarmConfigInfo.llid_mismatch_alarm_active, (void *)&PonPortTable[PonPortIdx].AlarmConfigInfo.llid_mismatch_alarm_configuration );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n17 Set llid mismatch err \r\n");
			return( RERROR );
			}
		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_TOO_MANY_ONU_REGISTERING, PonPortTable[PonPortIdx].AlarmConfigInfo.too_many_onus_registering_alarm_active, (void *)NULL);
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n18 Set too many onu register err \r\n");
			return( RERROR );
			}

		ret = PAS_set_alarm_configuration(PonPortIdx, PON_OLT_ID, PON_ALARM_DEVICE_FATAL_ERROR, PonPortTable[PonPortIdx].AlarmConfigInfo.device_fatal_error_alarm_active, NULL );
		if(ret != PAS_EXIT_OK ){ 
			sys_console_printf("\r\n12 Set errored frame period err \r\n");
			return( RERROR );
			}
#endif

#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
		/*PAS_PowerMeteringAlarmConfig(PonPortIdx);*/
#endif

		return( ROK );
		}
	else {

		}
	return( RERROR );

}

#ifdef MONITOR_AND_STATISTICS_AND_TEST
/*****************************************************
 *
 *    Function:  GetPonStatisticData( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:    
 *                 
 *    Desc:   
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int PonStatisticData[5];
/*unsigned long timestamp = 0;*/
long double  statistic_data;

int  GetPonStatisticData( short int PonPortIdx, short int OnuIdx )
{
unsigned long timestamp = 0;
	short int onuId;
	short int ret;
	short int PonChipType;
	OLT_raw_stat_item_t    stat_item;
	VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
	CHECK_ONU_RANGE

	onuId = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( onuId == INVALID_LLID )return( RERROR );
    
	PonChipType = V2R1_GetPonchipType(PonPortIdx);
				
	if(OLT_PONCHIP_ISPAS(PonChipType))
	{
		if(OLT_PONCHIP_ISPAS5001(PonChipType))
		{
		    #if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			stat_item.collector_id = PON_OLT_ID;
		    stat_item.raw_statistics_type = PON_RAW_STAT_ONU_BER;
		    stat_item.statistics_parameter = onuId;
			
		    stat_item.statistics_data = PonStatisticData;
		    stat_item.statistics_data_size = sizeof(PonStatisticData);
		    ret = OLT_GetRawStatistics(PonPortIdx, &stat_item);
			timestamp = stat_item.timestam;
			#else
			ret = PAS_get_raw_statistics_v4 ( PonPortIdx, 
								   PON_OLT_ID, 
								   PON_RAW_STAT_ONU_BER, 
								   onuId,
								   PON_STATISTICS_QUERY_HARDWARE,
								   PonStatisticData,
								   &timestamp );
			#endif
			if( ret == PAS_EXIT_OK ) 
				{
				sys_console_printf("\r\n err bytes %08x, used bytes %08x %08x good bytes %08x %08x\r\n", 
								PonStatisticData[0], PonStatisticData[1], PonStatisticData[2], PonStatisticData[3], PonStatisticData[4]);
				sys_console_printf("\r\nThe timestamp is %08x \r\n", timestamp );
				}
	        /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = OLT_GetStatistics(PonPortIdx,
								PON_OLT_ID, 
								PON_STAT_ONU_BER, 
								onuId,
								&statistic_data);
			if( ret == PAS_EXIT_OK ) sys_console_printf("\r\n onu%d/%d/%d BER  is %g\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), statistic_data );

			ret = OLT_GetStatistics(PonPortIdx,
								PON_OLT_ID, 
								PON_STAT_ONU_FER, 
								onuId,
								&statistic_data);
			if( ret == PAS_EXIT_OK ) sys_console_printf("\r\n onu %d/%d/%d FER  is %g\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), statistic_data );

			ret = OLT_GetStatistics(PonPortIdx,
								onuId, 
								PON_STAT_OLT_BER, 
								0,
								&statistic_data);
			if( ret == PAS_EXIT_OK ) sys_console_printf("\r\n pon%d/%d BER  is %g\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), statistic_data );

			ret = OLT_GetStatistics(PonPortIdx,
								onuId, 
								PON_STAT_OLT_FER, 
								0,
								&statistic_data);
			/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			if( ret == PAS_EXIT_OK ) sys_console_printf("\r\n pon%d/%d FER  is %g\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), statistic_data );
			}
		else 
			{

			}
		}

	else {

		}
	
	return( ROK );
}

/*****************************************************
 *
 *    Function:  StartPonPingTest( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:    
 *                 
 *    Desc:   
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int StartPonPingTest( short int PonPortIdx, short int OnuIdx )
{	
	short int len, elapsed_time;
	short int PonChipType;
	short int PonChipVer = RERROR;
	short int onuId;
	short int OnuCurrStatus;
	char *ptr, data[50] ={'\0'};

	CHECK_ONU_RANGE

	ptr = &data[0];
	
/*#ifdef _EPON_TYPE_GDC_OEM_
	VOS_MemCpy( ptr , "L531-EPON ping simulation data (c) :-) ", 50);
#else
	VOS_MemCpy( ptr , "GWTT Pon ping simulation data (c) :-) ", 50);
#endif*/
	VOS_StrCpy(ptr, typesdb_product_corporation_AB_name());
	VOS_MemCpy(&ptr[VOS_StrLen(typesdb_product_corporation_AB_name())], " PON ping simulation data (c) :-) ", 45);

	if( PonPortIsWorking( PonPortIdx) != TRUE )
		{
		sys_console_printf(" pon%d/%d not running\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
		return( RERROR );
		}

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
		
	len = VOS_StrLen( ptr );	
	
	onuId = GetLlidByOnuIdx( PonPortIdx,  OnuIdx);
	if( onuId == INVALID_LLID ) 
		{
		sys_console_printf(" ONU is off-line\r\n");
		return( RERROR );
		}

	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
			if( OLT_PONCHIP_ISPAS5001(PonChipType) )
				{
				#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
				OnuCurrStatus = OLT_GetOnuMode(PonPortIdx, onuId);
				#else
				OnuCurrStatus = PAS_get_onu_mode(PonPortIdx, onuId);
				#endif
				if( OnuCurrStatus == PON_ONU_MODE_ON )
					{
					if( PAS_EXIT_OK ==  PAS_ping_v4 ( PonPortIdx, onuId, len, (void *)data, &elapsed_time ))
						{
						sys_console_printf("\r\nonu%d/%d/%d Ping elapsed time=%dms\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), elapsed_time );	
						return( ROK );
						}
					else{
						sys_console_printf("\r\nonu%d/%d/%d Ping Err\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (1+OnuIdx) );
						return( RERROR );
						}
					}
				else return( RERROR );
				}
			else 
				{
				#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
				OnuCurrStatus = OLT_GetOnuMode(PonPortIdx, onuId);
				#else
				OnuCurrStatus = PAS_get_onu_mode(PonPortIdx, onuId);
				#endif
				if( OnuCurrStatus == PON_ONU_MODE_ON )
					{
					if( PAS_EXIT_OK ==  REMOTE_PASONU_ping ( PonPortIdx, onuId, len, (void *)data, &elapsed_time ))
						{
						sys_console_printf("\r\nonu%d/%d/%d Ping elapsed time=%dms\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), elapsed_time );	
						return( ROK );
						}
					else{
						sys_console_printf("\r\nonu%d/%d/%d Ping Err\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (1+OnuIdx) );
						return( RERROR );
						}
					}
				else return( RERROR );
				}
			}
	
	else{ /* 其他PON 芯片处理*/
			return( RERROR );
		}	
	
	return( ROK );
}

short int device_id_test = PON_OLT_ID;

int StartPonTestMode( short int PonPortIdx, short int OnuIdx )
{
	PON_device_id_t device_id;
	PON_mac_test_modes_t mac_mode;
	PON_loopback_t loopback_type;
	PON_link_rate_t onu_phy_loopback_speed_limitation=PON_10M;
	int  loopback_mode;
	
	
	short int PonChipType, PonChipVer = RERROR;
	short int  onuId = 0;
	short int ret;
	
	CHECK_ONU_RANGE

	if( PonPortIsWorking( PonPortIdx ) != TRUE )  return( RERROR );

	PonChipType  = GetPonChipTypeByPonPort(PonPortIdx);
	

	onuId = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onuId == INVALID_LLID ) return( RERROR );

	if( OLT_PONCHIP_ISPAS(PonChipType) )
    {   
        if (OLT_PONCHIP_ISPAS5001(PonChipType))
		{
			device_id = device_id_test;
			mac_mode = PAS_PON_MAC;
			loopback_type = PON_LOOPBACK_MAC;
			if( device_id == PON_OLT_ID ){ /* OLT test */
				loopback_mode = ON;
				}
			else{ /* onu test */
				onuId = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
				if( onuId == INVALID_LLID) return( RERROR );
				device_id = onuId;
				if( loopback_type == PON_LOOPBACK_MAC )
					{
					loopback_mode = ON;
					}
				else{
					loopback_mode =  100; /*test time: 10 s = 100 * 100 ms */
					}
				}
			onu_phy_loopback_speed_limitation = PON_10M;
			ret = PAS_set_test_mode_v4( PonPortIdx,  device_id, mac_mode, loopback_mode,  loopback_type, onu_phy_loopback_speed_limitation );
			if( ret == PAS_EXIT_OK ) {
				sys_console_printf("\r\nonu%d/%d/%d test OK\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
				return ( ROK );
				}
			else{
				sys_console_printf("\r\nonu%d/%d/%d test Err %d\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1),  ret );
				return( RERROR );			
				}
		}
    	else
		{

		}
    }
	else{ /* other pon chip type handler */

		}
	
	return( RERROR );

}

int StartPonMacTest( short int PonPortIdx)
{
	PON_device_id_t device_id;
	PON_mac_test_modes_t mac_mode;
	PON_loopback_t loopback_type;
	PON_link_rate_t onu_phy_loopback_speed_limitation=PON_10M;
	int  loopback_mode;
	
	
	short int PonChipType;
	short int ret;
	
	CHECK_PON_RANGE

	PonChipType  = GetPonChipTypeByPonPort(PonPortIdx);
	
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		device_id = PON_OLT_ID;
		mac_mode = PAS_PON_MAC;
		loopback_type = PON_LOOPBACK_MAC;
		loopback_mode = ON;

		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			ret = PAS_set_test_mode_v4( PonPortIdx,  device_id, mac_mode, loopback_mode,  loopback_type, onu_phy_loopback_speed_limitation );
			if( ret == PAS_EXIT_OK ) {
				sys_console_printf("\r\nstart %s/port%d Mac test OK\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
				return ( ROK );
				}
			else{
				sys_console_printf("\r\nstart %s/port%d Mac test Err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),  ret );
				return( RERROR );			
				}
			}
		}
	else{ /* other pon chip type handler */
		}
	return( RERROR );

}

int StartPonPhyTest( short int PonPortIdx)
{
	PON_device_id_t device_id;
	PON_mac_test_modes_t mac_mode;
	PON_loopback_t loopback_type;
	PON_link_rate_t onu_phy_loopback_speed_limitation=PON_10M;
	int  loopback_mode;
	
	
	short int PonChipType, PonChipVer = RERROR;
	short int ret;
	
	CHECK_PON_RANGE

	PonChipType  = GetPonChipTypeByPonPort(PonPortIdx);
	
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			device_id = PON_OLT_ID;
			mac_mode = PAS_PON_MAC;
			loopback_type = PON_LOOPBACK_PHY;
			loopback_mode = ON;
	
			ret = PAS_set_test_mode_v4( PonPortIdx,  device_id, mac_mode, loopback_mode,  loopback_type, onu_phy_loopback_speed_limitation );
			if( ret == PAS_EXIT_OK ) {
				sys_console_printf("\r\nstart %s/port %d Phy test OK\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
				return ( ROK );
				}
			else{
				sys_console_printf("\r\nstart %s/port %d Phy test Err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),  ret );
				return( RERROR );			
				}
			}
		else 
			{

			}
		}
	else{ /* other pon chip type handler */
		}
	return( RERROR );

}

int StartOnuMacTest( short int PonPortIdx, short int OnuIdx )
{
	PON_device_id_t device_id;
	PON_mac_test_modes_t mac_mode;
	PON_loopback_t loopback_type;
	PON_link_rate_t onu_phy_loopback_speed_limitation=PON_10M;
	int  loopback_mode;
	
	
	short int PonChipType;
	short int onuId;
	short int ret;
	
	CHECK_PON_RANGE

	PonChipType  = GetPonChipTypeByPonPort(PonPortIdx);
		
	onuId = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onuId == INVALID_LLID ) return( RERROR );
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			device_id = onuId;
			mac_mode = PAS_PON_MAC;
			loopback_type = PON_LOOPBACK_MAC;
			loopback_mode = ON;
			/*onu_phy_loopback_speed_limitation = PON_10M;*/
			
			ret = PAS_set_test_mode_v4( PonPortIdx,  device_id, mac_mode, loopback_mode,  loopback_type, onu_phy_loopback_speed_limitation );
			if( ret == PAS_EXIT_OK ) {
				sys_console_printf("\r\nstart %s/port %d  Onu %d Mac test OK\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
				return ( ROK );
				}
			else{
				sys_console_printf("\r\nstart %s/port %d  Onu %d Mac test Err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1),  ret );
				return( RERROR );			
				}
			}
		else 
			{

			}
		}
	
	else{ /* other pon chip type handler */
		}
	return( RERROR );

}

int StartOnuPhyTest( short int PonPortIdx, short int OnuIdx )
{
	PON_device_id_t device_id;
	PON_mac_test_modes_t mac_mode;
	PON_loopback_t loopback_type;
	PON_link_rate_t onu_phy_loopback_speed_limitation=PON_10M;
	int  loopback_mode;
	
	
	short int PonChipType;
	short int onuId;
	short int ret;
	
	CHECK_PON_RANGE

	PonChipType  = GetPonChipTypeByPonPort(PonPortIdx);
		
	onuId = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onuId == INVALID_LLID ) return( RERROR );
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			device_id = onuId;
			mac_mode = PAS_PON_MAC;
			loopback_type = PON_LOOPBACK_PHY;
			loopback_mode = 100;
			onu_phy_loopback_speed_limitation = PON_10M;
			
			ret = PAS_set_test_mode_v4( PonPortIdx,  device_id, mac_mode, loopback_mode,  loopback_type, onu_phy_loopback_speed_limitation );
			if( ret == PAS_EXIT_OK ) {
				sys_console_printf("\r\nstart %s/port %d  Onu %d Phy test OK\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
				return ( ROK );
				}
			else{
				sys_console_printf("\r\nstart %s/port %d  Onu %d Phy test Err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1),  ret );
				return( RERROR );			
				}
			}
		else 
			{

			}
		}
	
	else{ /* other pon chip type handler */
		}
	return( RERROR );

}

int StopPonMacTest( short int PonPortIdx)
{
	PON_device_id_t device_id;
	PON_mac_test_modes_t mac_mode;
	PON_loopback_t loopback_type;
	PON_link_rate_t onu_phy_loopback_speed_limitation=PON_10M;
	int  loopback_mode;
	
	
	short int PonChipType, PonChipVer = RERROR;
	short int ret;
	
	CHECK_PON_RANGE

	PonChipType  = GetPonChipTypeByPonPort(PonPortIdx);
	
	
	if(OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			device_id = PON_OLT_ID;
			mac_mode = PAS_PON_MAC;
			loopback_type = PON_LOOPBACK_MAC;
			loopback_mode = PAS_NO_LOOPBACK;
	
			ret = PAS_set_test_mode_v4( PonPortIdx,  device_id, mac_mode, loopback_mode,  loopback_type, onu_phy_loopback_speed_limitation );
			if( ret == PAS_EXIT_OK ) {
				sys_console_printf("\r\nstop %s/port%d  Mac test OK\r\n",  CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
				return ( ROK );
				}
			else{
				sys_console_printf("\r\nstop %s/port%d  Mac test Err %d\r\n",  CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),  ret );
				return( RERROR );			
				}
			}
		else
			{

			}
		}
	else{ /* other pon chip type handler */
		}
	return( RERROR );

}

int StopPonPhyTest( short int PonPortIdx)
{
	PON_device_id_t device_id;
	PON_mac_test_modes_t mac_mode;
	PON_loopback_t loopback_type;
	PON_link_rate_t onu_phy_loopback_speed_limitation=PON_10M;
	int  loopback_mode;
	
	
	short int PonChipType;
	short int ret;
	
	CHECK_PON_RANGE

	PonChipType  = GetPonChipTypeByPonPort(PonPortIdx);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			device_id = PON_OLT_ID;
			mac_mode = PAS_PON_MAC;
			loopback_type = PON_LOOPBACK_PHY;
			loopback_mode = PAS_NO_LOOPBACK;
	
			ret = PAS_set_test_mode_v4( PonPortIdx,  device_id, mac_mode, loopback_mode,  loopback_type, onu_phy_loopback_speed_limitation );
			if( ret == PAS_EXIT_OK ) {
				sys_console_printf("\r\nstop %s/port%d Phy test OK\r\n",  CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
				return ( ROK );
				}
			else{
				sys_console_printf("\r\nstop %s/port%d Phy test Err %d\r\n",  CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),  ret );
				return( RERROR );			
				}
			}
		else
			{

			}
		}
	else{ /* other pon chip type handler */
		}
	return( RERROR );

}

int StopOnuPhyTest( short int PonPortIdx, short int OnuIdx )
{
	PON_device_id_t device_id;
	PON_mac_test_modes_t mac_mode;
	PON_loopback_t loopback_type;
	PON_link_rate_t onu_phy_loopback_speed_limitation = PON_10M;
	int  loopback_mode;
	
	
	short int PonChipType;
	short int onuId;
	short int ret;
	
	CHECK_PON_RANGE

	PonChipType  = GetPonChipTypeByPonPort(PonPortIdx);
	
	onuId = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onuId == INVALID_LLID ) return( RERROR );
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			device_id = onuId;
			mac_mode = PAS_PON_MAC;
			loopback_type = PON_LOOPBACK_PHY;
			loopback_mode = PAS_NO_LOOPBACK;
	
			ret = PAS_set_test_mode_v4( PonPortIdx,  device_id, mac_mode, loopback_mode,  loopback_type, onu_phy_loopback_speed_limitation );
			if( ret == PAS_EXIT_OK ) {
				sys_console_printf("\r\nstop %s/port%d  Onu %d Phy test OK\r\n",  CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
				return ( ROK );
				}
			else{
				sys_console_printf("\r\nstop %s/port%d  Onu %d Phy test Err %d\r\n",  CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1),  ret );
				return( RERROR );			
				}
			}
		else 
			{

			}
		}
	else{ /* other pon chip type handler */
		}
	return( RERROR );

}

int StopOnuMacTest( short int PonPortIdx, short int OnuIdx )
{
	PON_device_id_t device_id;
	PON_mac_test_modes_t mac_mode;
	PON_loopback_t loopback_type;
	PON_link_rate_t onu_phy_loopback_speed_limitation = PON_10M;
	int  loopback_mode;
	
	
	short int PonChipType, PonChipVer = RERROR;
	short int onuId;
	short int ret;
	
	CHECK_PON_RANGE

	PonChipType  = GetPonChipTypeByPonPort(PonPortIdx);
	
	onuId = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onuId == INVALID_LLID ) return( RERROR );
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			device_id = onuId;
			mac_mode = PAS_PON_MAC;
			loopback_type = PON_LOOPBACK_MAC;
			loopback_mode = PAS_NO_LOOPBACK;
            #if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = OLT_GetOnuMode( PonPortIdx, onuId );
			#else
			ret = PAS_get_onu_mode( PonPortIdx, onuId );
			#endif
			if( ret != PON_ONU_MODE_ON )
				{
				sys_console_printf("\r\n%s/port %d onu %d is Off Line, so can't have a MAC test\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
				return( ROK );
				}
	
			ret = PAS_set_test_mode_v4( PonPortIdx,  device_id, mac_mode, loopback_mode,  loopback_type, onu_phy_loopback_speed_limitation );
			if( ret == PAS_EXIT_OK ) {
				sys_console_printf("\r\nstop %s/%d  Onu %d Mac test OK\r\n",  CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
				return ( ROK );
				}
			else{
				sys_console_printf("\r\nstop %s/%d Onu %d Mac test Err %d\r\n",  CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1),  ret );
				return( RERROR );			
				}
			}
		else 
			{

			}
		}
	else{ /* other pon chip type handler */
		}
	return( RERROR );

}

int  StartPonLinkTest( short int PonPortIdx, short int OnuIdx )
{

	PON_link_test_vlan_configuration_t vlan_configuratiion;
	PON_link_test_results_t  test_results;
	
	short int frame_size;

	short int Llid;
	short int PonChipType, PonChipVer = RERROR;
	short int ret;

	CHECK_ONU_RANGE

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	
	Llid  = GetLlidByOnuIdx(PonPortIdx,  OnuIdx);

	if( Llid == INVALID_LLID) return( RERROR);
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			{
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
            #if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = OLT_GetOnuMode( PonPortIdx, Llid );
			#else
			ret = PAS_get_onu_mode( PonPortIdx, Llid );
			#endif
			if( ret != PON_ONU_MODE_ON )
				{
				sys_console_printf(" Pon %d onu %d is Off Line, so can't have a MAC test \r\n");
				return( ROK );
				}
			/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = OLT_LinkTest ( PonPortIdx, Llid,  number_of_test_frames, frame_size, TRUE, &vlan_configuratiion, &test_results );
			if( ret == PAS_EXIT_OK ) {
				if( number_of_test_frames == PON_LINK_TEST_CONTINUOUS ){
					sys_console_printf("\r\n%s/port%d onu %d Link Test start ok\r\n",  CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
					}
				else{
					sys_console_printf("\r\n%s/port%d onu %d Link Test ok\r\n",  CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
					sys_console_printf("   Total frames Send: %d\r\n", test_results.number_of_sent_frames);
					sys_console_printf("   Total frames Recv: %d\r\n", test_results.number_of_returned_frames);
					sys_console_printf("   Error frames Recv: %d\r\n", test_results.number_of_errored_return_frames );
					sys_console_printf("   Minimal delay : %d(TQ)\r\n", test_results.minimal_delay);
					sys_console_printf("   Maximal delay: %d(TQ)\r\n", test_results.maximal_delay);
					sys_console_printf("   Mean delay : %d(TQ)\r\n", test_results.mean_delay);
					}
				return( ROK );
				}
			else{
				sys_console_printf("\r\n%s/%d onu %d Link Test err %d\r\n",  CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), ret );
				return( RERROR );
				}

			}
		}
	else{ /* other pon chip type handler */

		}
	
	
	return( RERROR );
}

/*****************************************************
 *
 *    Function:  PonPortAddressTableClear( short int PonPortIdx )
 *
 *    Param:    
 *                 
 *    Desc:   
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 int PonPortAddressTableClear( short int PonPortIdx )
{
	CHECK_PON_RANGE

	
	return( ROK );
}


/* following is only for test.  added by chenfj 2007-04-12 */

long double  ber_threshold = 0.5;
/*long double tst,tst1,tst2,tst3,tst4;
float  tst5;

int test_doubledata()
{
	tst = ber_threshold;
	tst1 = (tst*10.0);
	tst2 = tst*100.0;
	tst3= tst + 10.0;
	tst4= tst + 100.0;
	sys_console_printf("%f\r\n", tst);
	sys_console_printf("%f\r\n", (tst*10.0));
	sys_console_printf("%f\r\n", (tst*100.0));
	sys_console_printf("%f\r\n", (tst+10.0));
	sys_console_printf("%f\r\n", (tst+100.0));

	return( ROK );
}*/

int SetPonBerAlarmParam( short int PonPortIdx,short int sourceId, unsigned char active,  int  threshold, int num_bytes  )
{
	short int PonChipType;
	short int ret;
	PON_ber_alarm_configuration_t  ber_config;

	CHECK_PON_RANGE

	ber_config.ber_threshold = threshold*(1.00e-9);
	ber_threshold = ber_config.ber_threshold;
	ber_config.direction = PON_DIRECTION_UPLINK_AND_DOWNLINK;
	ber_config.minimum_error_bytes_threshold = num_bytes;
	/*ber_config.TBD = 0;*/
	
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	if( PonChipType == RERROR ) return( RERROR );
	
	if(OLT_PONCHIP_ISPAS(PonChipType)) 
		{	
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		ret = OLT_SetAlarmConfig( PonPortIdx, sourceId, PON_ALARM_BER, active , (void *)&ber_config, sizeof(PON_ber_alarm_configuration_t));
		#else
		ret = PAS_set_alarm_configuration( PonPortIdx, sourceId, PON_ALARM_BER, active , (void *)&ber_config);
		#endif
		return( ret );
		}
	else {

		}
	
	return( ROK );
}

int GetPonBerAlarmParam(short int PonPortIdx, short int sourceId )
{

	/*short int PonChipType;*/
	short int ret;
	unsigned char active;
	int count=0;
	PON_ber_alarm_configuration_t  ber_config;

	CHECK_PON_RANGE

	/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	ret = OLT_GetAlarmConfig(PonPortIdx, sourceId, PON_ALARM_BER, &active, (void *)&ber_config);
	if( ret == PAS_EXIT_OK )
		{
		sys_console_printf("active =");
		if( active ==  TRUE ) sys_console_printf(" ture\r\n");
		else sys_console_printf(" false\r\n");
		sys_console_printf("direction= %s\r\n", PonLinkDirection_s[ber_config.direction ]);
		sys_console_printf("mim err bytes=%d\r\n", ber_config.minimum_error_bytes_threshold );

		count = 0;

		sys_console_printf(" ber  threshold = %lf\r\n", ber_config.ber_threshold );
		do{	
			if( ber_config.ber_threshold < 1 )
				{
				ber_config.ber_threshold = ber_config.ber_threshold * 10;
				count ++;
				}
			}while(ber_config.ber_threshold < 1);
		
		sys_console_printf("ber threshold=%d*10e-%d\r\n", ber_config.ber_threshold, count );
		}
	else return( ret );

	return( ROK );

}


unsigned  int  retValue[10];
int  GetOnuBer( short int PonPortIdx , short int OnuIdx )
{
PON_timestamp_t  timestamp = 0;
	short int onu_id;
	short int ret;
	OLT_raw_stat_item_t    stat_item;
	
	
	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );
	#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
	stat_item.collector_id = PON_OLT_ID;
    stat_item.raw_statistics_type = PON_RAW_STAT_ONU_BER;
    stat_item.statistics_parameter = onu_id;
	
    stat_item.statistics_data = retValue;
    stat_item.statistics_data_size = sizeof(retValue);
    ret = OLT_GetRawStatistics(PonPortIdx, &stat_item);
	timestamp = stat_item.timestam;
	#else
	ret = PAS_get_raw_statistics_v4( PonPortIdx, PON_OLT_ID , PON_RAW_STAT_ONU_BER, onu_id, PON_STATISTICS_QUERY_HARDWARE, retValue,  &timestamp);
	#endif
	if( ret == PAS_EXIT_OK ) return( ROK );
	return( RERROR );
}

int GetOnuFer( short int PonPortIdx, short int OnuIdx )
{

	return( RERROR );
}
#endif

#ifdef  PAS_SOFT_VERSION_V5_3_5
/*****************************************************************
Event indicating that the OLT's CNI link state is changed
*****************************************************************/
short int Pon_cni_link_handler( const short int PonPortIdx, const bool status )
{
	CHECK_PON_RANGE
		
	if(EVENT_DEBUG == V2R1_ENABLE )
		{
		sys_console_printf("\r\n%% pon%d/%d cni link status changed to %s\r\n",GetCardIdxByPonChip(PonPortIdx),GetPonPortByPonChip(PonPortIdx),
 				( (status == TRUE)  ? "up" : "down") );
		}	
	/* send trap to NMS */
	
	return( PAS_EXIT_OK );
}
#endif

int Pon_gpio_changed_handler( const short int PonPortIdx, const unsigned char gpio_id, const bool status )
{
	CHECK_PON_RANGE
		
	if(EVENT_DEBUG == V2R1_ENABLE )
	{
		sys_console_printf("\r\n%% pon%d/%d gpio(%d) is changed to state(%d)\r\n",GetCardIdxByPonChip(PonPortIdx),GetPonPortByPonChip(PonPortIdx),gpio_id,status); 
	}	
	/* send trap to NMS */

    if ( (TkOltGpioPurposeUserInput0 == gpio_id) || (TkOltGpioPurposeUserInput1 == gpio_id) )
    {
    	sendSfpLossMsg( PonPortIdx, status );
    }
	
	return( TK_EXIT_OK );
}

int Pon_llid_discovery_handler(short int olt_id, short int llid, mac_address_t link_mac, bcmEmmiLinkInfo *link_info)
{
	if ( EVENT_REGISTER == V2R1_ENABLE )
	{
        static char* aszRateDesc[4] = {"10/10", "10/1", "1/1", "err"};
        static char* aszStateDesc[5] = {"unknowned", "discovered", "protect-working", "protect-standby", "denied"};
        char *pcszRateStr, *pcszStateStr;

        if ( link_info->rate >= 0 && link_info->rate < 3 )
        {
            pcszRateStr = aszRateDesc[link_info->rate];
        }
        else
        {
            pcszRateStr = aszRateDesc[3];
        }

        if ( link_info->status & bcmEmmiLinkStatusRegistrationPrevented )
        {
            pcszStateStr = aszStateDesc[4];
        }
        else if ( link_info->status & bcmEmmiLinkStatusProtectedStandby )
        {
            pcszStateStr = aszStateDesc[3];
        }
        else if ( link_info->status & bcmEmmiLinkStatusProtectedWorking )
        {
            pcszStateStr = aszStateDesc[2];
        }
        else if ( link_info->status & bcmEmmiLinkStatusDiscovered )
        {
            pcszStateStr = aszStateDesc[1];
        }
        else
        {
            pcszStateStr = aszStateDesc[0];
        }
    
		sys_console_printf("BCM_HANDLER_ONU_discovery\r\n");
		sys_console_printf(" PONid:%d(%s),llid:%d,MAC:%02x%02x.%02x%02x.%02x%02x is %s.\r\n", olt_id, pcszRateStr, llid, 
			link_mac[0], link_mac[1], link_mac[2], link_mac[3], link_mac[4], link_mac[5], pcszStateStr); 
	}

    if ( link_info->status & bcmEmmiLinkStatusRegistrationPrevented )
    {
		AddPendingOnu( olt_id, -1, llid, link_mac, PENDING_REASON_CODE_ONU_CONFICT );
    }
    else
    {
        if ( link_info->status & bcmEmmiLinkStatusDiscovered )
        {
            /* 通知发起802.3AH发现过程 */
            if ( 0 != PonStdOamDiscovery(olt_id, llid, link_mac) )
            {
    			AddPendingOnu( olt_id, -1, llid, link_mac, PENDING_REASON_CODE_StdOAM_FAIL );
            }
        }
        else
        {
    		AddPendingOnu( olt_id, -1, llid, link_mac, PENDING_REASON_CODE_PON_PASSIVE );
        }
    }

    return 0;
}

int Pon_llid_loss_handler(short int olt_id, short int llid, mac_address_t link_mac, PON_onu_deregistration_code_t loss_reason)
{
	if ( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf("BCM_HANDLER_ONU_loose\r\n");
		sys_console_printf(" PONid:%d,llid:%d,MACaddr:%02x%02x.%02x%02x.%02x%02x,reason code=%d\r\n", olt_id, llid, 
			link_mac[0], link_mac[1], link_mac[2], link_mac[3], link_mac[4], link_mac[5], loss_reason); 
	}

    /* 通知关闭802.3AH连接 */
    PonStdOamDisconect(olt_id, llid, link_mac);

    /* 通知ONU离线处理 */
    Onu_deregistration_handler( olt_id, llid, loss_reason );

    return 0;
}

/************** end ***********/

#if 1
void InitPonEventHandler()
{
    VOS_MemZero(&pon_event_handlers, sizeof(pon_event_handlers));
}

int Pon_event_assign_handler_function
                ( const PON_event_handler_index_t      handler_function_index, 
				  const void					    (*handler_function)(),
				  unsigned short                     *handler_id )
{
	int result;
	
    result = PON_general_assign_handler_function( &pon_event_handlers, 
						(unsigned short int)handler_function_index, 
						(general_handler_function_t)handler_function,
						handler_id);

	if (result != EXIT_OK)
		return RERROR;  

    return ROK;  
}

int Pon_event_delete_handler_function ( const unsigned short  handler_id )
{
	int result;

	result = PON_general_delete_handler_function ( &pon_event_handlers, handler_id );

	if (result != EXIT_OK)
		return RERROR;  

    return ROK;  
}

int Pon_event_handler(PON_event_handler_index_t event_id, void *event_data)
{
    unsigned short client;
	general_handler_function_t the_handler;

    if ( PON_HANDLER_INDEX_ISVALID(event_id) )
    {
        for (client = 0; client < PON_MAX_CLIENT; client++)
        {
            PON_general_get_handler_function
                               ( &pon_event_handlers,
                                 event_id,
                                 client ,
                                 &the_handler );
            
            if (the_handler != NULL)
            {
                the_handler (event_id, event_data);
            }
        }
    }

	return RERROR;  
}
#endif


#if 1
#include  "cpi/ctss/ctss_pon.h"


int pon_pp_init()
{
    return ctss_pon_bridge_init();
}

int pon_pp_switch(int nni_idx)
{
    return ctss_pon_bridge_switch(nni_idx);
}


int pon_pp_add_olt(short int olt_id)
{
    return ctss_pon_add_olt(olt_id);
}

int pon_pp_remove_olt(short int olt_id)
{
    return ctss_pon_remove_olt(olt_id);
}


int pon_pp_add_link(short int olt_id, short int llid, unsigned long up_tunnel, unsigned long down_tunnel, unsigned long flags)
{
    return ctss_pon_add_link(olt_id, llid, up_tunnel, down_tunnel, flags);
}

int pon_pp_remove_link(short int olt_id, short int llid)
{
    return ctss_pon_remove_link(olt_id, llid);
}


int pon_pp_get_link_sla(short int olt_id, short int llid, int bridge_path, BcmSlaQueueParams *sla)
{
    return ctss_pon_get_link_sla(olt_id, llid, bridge_path, sla);
}

int pon_pp_set_link_sla(short int olt_id, short int llid, int bridge_path, BcmSlaQueueParams *sla)
{
    return ctss_pon_set_link_sla(olt_id, llid, bridge_path, sla);
}

/*------------ added by wangjiah@2016-07-14 to support max mac learn limit for 8xep ------------- */
int pon_pp_set_l2_mac_learn_limit(short int olt_id, short int llid, int limit_value)
{
	return ctss_pon_set_l2_mac_learn_limit(olt_id, llid, limit_value);
}
int pon_pp_get_olt_addrs(short int olt_id, short int *active_records, PON_address_table_t address_table)
{
    return ctss_pon_get_olt_addrs(olt_id, active_records, address_table);
}

int pon_pp_add_olt_addrs(short int olt_id, short int num_of_records, PON_address_table_t address_table)
{
    return ctss_pon_add_olt_addrs(olt_id, num_of_records, address_table);
}

int pon_pp_del_olt_addrs(short int olt_id, short int num_of_records, PON_address_table_t address_table)
{
    return ctss_pon_del_olt_addrs(olt_id, num_of_records, address_table);
}

int pon_pp_del_olt_addr(short int olt_id, mac_address_t mac_addr)
{
    return ctss_pon_del_olt_addr(olt_id, mac_addr);
}


int pon_pp_get_link_addrs(short int olt_id, short int llid, short int *active_records, PON_address_table_t address_table)
{
    return ctss_pon_get_link_addrs(olt_id, llid, active_records, address_table);
}

int pon_pp_clr_link_addrs(short int olt_id, short int llid, PON_address_aging_t address_type)
{
    return ctss_pon_clr_link_addrs(olt_id, llid, address_type);
}


int pon_pp_get_olt_vlan_tpid(short int olt_id, unsigned short int *tpid_outer, unsigned short int *tpid_inner)
{
    return ctss_pon_get_olt_vlan_tpid(olt_id, tpid_outer, tpid_inner);
}

int pon_pp_set_olt_vlan_tpid(short int olt_id, unsigned short int tpid_outer, unsigned short int tpid_inner)
{
    return ctss_pon_set_olt_vlan_tpid(olt_id, tpid_outer, tpid_inner);
}

int pon_pp_set_link_vlan_uplink(short int olt_id, short int llid, PON_olt_vlan_uplink_config_t *vlan_uplink_config)
{
    return ctss_pon_set_link_vlan_uplink(olt_id, llid, vlan_uplink_config);
}

int pon_pp_set_link_vlan_downlink(short int olt_id, PON_vlan_tag_t vlan_id, PON_olt_vid_downlink_config_t *vid_downlink_config)
{
    return ctss_pon_set_link_vlan_downlink(olt_id, vlan_id, vid_downlink_config);
}


int pon_pp_set_class_rule(short int olt_id, const PON_pon_network_traffic_direction_t direction, const PON_olt_classification_t classification_entity, const void *classification_parameter, const PON_olt_classifier_destination_t destination)
{
    return ctss_pon_set_class_rule(olt_id, direction, classification_entity, classification_parameter, destination);
}


int pon_pp_event_handler(PON_event_handler_index_t event_id, void *event_data)
{
    int result = RERROR;
    
    switch ( event_id )
    {
        case PON_EVT_HANDLER_OLT_ADD:
            result = pon_pp_add_olt((short int)PTR2DATA(event_data));
        break;
        case PON_EVT_HANDLER_OLT_RMV:
            result = pon_pp_remove_olt((short int)PTR2DATA(event_data));
        break;
        default:
            VOS_ASSERT(0);
    }

    return result;
}

int pon_pp_switchover_handler(devsm_switchhover_notifier_event ulEvent)
{
	if(ulEvent != switchhover_notify_start)
		return VOS_ERROR;

    if ( SYS_LOCAL_MODULE_TYPE_IS_PHY_PON )
    {
        return pon_pp_switch(-1);
    }

    return 0;
}


int InitPonPPHandler()
{
    int result;

    if ( 0 == (result = pon_pp_init()) )
    {
        unsigned short handler_id;
        
        do{

            if ( 0 != (result = Pon_event_assign_handler_function(PON_EVT_HANDLER_OLT_ADD,(void*)pon_pp_event_handler, &handler_id)) )
            {
                break; 
            }

#if 0
            if ( 0 != (result = Pon_event_assign_handler_function(PON_EVT_HANDLER_OLT_RMV,(void*)pon_pp_event_handler, &handler_id)) )
            {
                break; 
            }
#endif


        }while(0);
    }

    return result;
}


int pon_pp_get_CNI_link_status(short int olt_id, int *status)
{
	return ctss_pon_get_cni_link_status(olt_id,status);
}

#endif



#ifdef __cplusplus

}
#endif

