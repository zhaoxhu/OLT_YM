/***************************************************************
*
*						Module Name:  OnuHandler.c
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
*   Date: 			2006/05/16
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |-----------|------------------ 
**  06/05/16  |   chenfj          |     create 
**----------|-----------|------------------
**
** 1 modified by chenfj 2006-11-16
**   #3196问题单:加密去使能命令出错；保存错误
** 2 add by chenfj 2006-11-17 
**	 #3176 问题单PON节点下，有时执行加密使能，去使能会导致ONU离线
** 3  modified by chenfj 2006/12/15
**	ONU不停的离线注册，可能导致OLT出现问题的一个隐患?
** 4 added by chenfj 2007/01/17
**   增加向ONU 注册和定时(每隔30分钟)发送系统时间
** 5 modified by chenfj 2007/03/15 
**		3795.更改下面的光路拓扑结果后，发现有些ONU的状态是Down的，但能上网
**		将消息向队列中的发送顺序由MSG_PRI_URGENT 改为MSG_PRI_NORMAL 
**  6 add by chenfj 2007/5/18
**    增加设置PAS5201下ONU FEC 
**  7  add by chenfj 2007/5/28
**    增加设置/清除ONU 数据原MAC数据包过滤
** 8 added by chenfj 2007-6-8 
**       增加ONU 数据流IP/PORT过滤
** 9  added by chenfj 2007-6-12 
**       增加ONU 数据流vlan id 过滤
** 10 added by chenfj 2007-6-14 
**       增加ONU 数据流ETHER TYPE / IP PROTOCOL 过滤
** 11 modified by chenfj 2007-7-3
**      在设置ONU设备信息时，若当长度为0，则为清除
** 12 modified by chenfj 2008-7-9
**         增加GFA6100 产品支持
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
#include  "Onu_manage.h"
#include  "Onu_oam_comm.h"
#include  "onuConfMgt.h"
/*#include "ct_manage/CT_RMan_EthPort.h"*/
#include "Cdp_pub.h"
#if ((defined ONU_PPPOE_RELAY)||(defined ONU_DHCP_RELAY))
#include  "access_identifier/access_id.h"/*wugang add for pppoe relay*/
#endif
#include "onu\ExtBoardType.h"

/*
#include "vos/vospubh/Vos_sem.h"
#include "vos/vospubh/vos_types.h"
#include  "Device_manage.h"

#include <stdio.h>

#include "vos_global.h"

#include "vos/vospubh/vos_types.h"
#include "vos/vospubh/vos_error.h"
#include "vos/vospubh/vos_mem.h"
#include "vos/vospubh/vos_string.h"
#include "vos/vospubh/vos_limits.h"
#include "vos/vospubh/vos_assert.h"
#include "vos/vospubh/vos_time.h"
#include "cpi/cdsms/cdsms_main.h"
#include "vos/vospubh/vos_task.h"
#include "vos/vospubh/vos_sem.h"
#include "vos/vospubh/vos_que.h"

#include "vos/vospubh/cdp_pub.h"
#include "msgcode.h"
#include "cli/cli.h"
#include "sys/main/sys_main.h"
#include "device_manage.h"
#include "assert.h"
#include "syscfg.h"
#include "vos_global.h"
#include "cpi/typesdb/typesdb_product.h"
#include "vos/vospubh/vos_sysmsg.h"
#include "cpi/cdsms/cdsms_hardware_env.h"
#include "sys/version/sys_version.h"
#include <Mod_flag.h>

*/

/*#define  ONU_COMM_SEMI  */

int EncryptKeyDelayTime = VOS_TICK_SECOND/5;
int  UpdateKeyFlag = V2R1_ENABLE;
/*int Seed =0;*/
#if 0
unsigned char  *OnuDeregistrationReason[PON_ONU_DEREGISTRATION_LAST_CODE-1] = {
	(unsigned char  *)"ONU Deregistration_Report Timeout(MPCP)",   /* Timeout has expired since the last MPCP REPORT   */
										     /* frame received from the ONU					   */
	(unsigned char  *)"ONU Deregistration_OAM Link Disconnection", /* ONU didn't reply to several OAM Information      */
										    /* frames, hence OAM link is disconnected		   */
	(unsigned char  *)"ONU Deregistration_Host Request",		   /* Response to previous PAS_deregister_onu or       */
										   /* PAS_shutdown_onu API calls				   */
	(unsigned char  *)"ONU Deregistration_Double Registration",	   /* ONU registered twice without deregistering	   */
										   /* This is an error handling for extrem cases.	   */
										   /* If the ONU should be registered, the following   */
										   /* REGISTER_REQ will cause a correct registration.  */
	(unsigned char  *)"ONU Deregistration_Timestamp Drift",		   /* when a timestamp drift of 16nsec is identified   */
                                                                         /* by the HW                                        */
	(unsigned char  *)"ONU Deregistration_Unknown"               /* unknown reason, (recovery after missing event)   */

	/* "ONU Deregistration_Last code"  */
};
#endif
OnuIdentifier_S  OnuWaitForGetEUQ = {0,0,NULL,NULL};
unsigned long OnuEUQDataSemId = 0;
unsigned long OnuEUQCommSemId=0;
unsigned long OnuEUQRecvSemId = 0;
char RecvMsgFromOnu[EUQ_MAX_OAM_PDU] /*= {0}*/;
char RecvMsgFromOnuSessionId[8]= {0};
int  RecvMsgFromOnuLen=0;
static unsigned int WaitOnuEUQMsgFlag = 0; 
static unsigned short int ONUEUQMsgWaitPonPortIdx = 20 ; /* MAXPON */
static unsigned short int ONUEUQMsgWaitOnuIdx= MAXONUPERPONNOLIMIT;
static unsigned char ONUEUQMsgWaitMsgType = 0;


extern int OnuEvent_Set_WaitOnuEUQMsgFlag(short int PonPortIdx, short int OnuIdx, UCHAR status);
extern int Comm_EUQ_SCB_systemTime_transmit (
					const unsigned short PonId, 
					unsigned char		*pSysTimeBuf,
					const unsigned short sysTimeLen,
					unsigned char *psessionIdField);
extern int Comm_EUQ_systime_Require_transmit(
				const unsigned short PonId, 
				const unsigned short OnuId,
				unsigned char *pSysTimeBuf,
				const unsigned short sysTimeBufLen,
				unsigned char *psessionIdField
				);

extern int CTC_ResetONU(short int PonPortIdx, short int OnuIdx );

extern unsigned int   inet_atonl_v4( char* ipaddr );
extern void  inet_ntoa_v4( unsigned int  lip, char* ipaddr ) ;
extern LONG devsm_sys_is_switchhovering();
extern int  resetOnuOperStatusAndUpTime( ULONG OnuEntry, int OperStatus );

extern STATUS isHaveAuthMacAddress( ULONG slot, ULONG pon, const char* mac,  ULONG *onu );
extern STATUS getOnuAuthEnable( ULONG slot, ULONG port, ULONG *enable );
extern STATUS setOnuAuthEnable(ULONG slot, ULONG port, ULONG	 enable );
extern STATUS setOnuAuthMacAddress( ULONG slot, ULONG pon, ULONG onu, CHAR *macbuf );
extern STATUS getOnuAuthStatus( ULONG slot, ULONG pon, ULONG onu, ULONG *st );
extern STATUS setOnuAuthStatus( ULONG slot, ULONG pon, ULONG onu,  ULONG st );
extern int GetOnuPeerToPeerInOnuEntry( short int OnuEntry,short int OnuIdx );

extern int  CTC_getDeviceCapEthPortNum( short int PonPortIdx, short int OnuIdx,  int *value );
extern int  CTC_GetOnuCapability(short int PonPortIdx, short int OnuIdx );

extern int  DeleteMacAuthentication( short int PonPortIdx, mac_address_t mac );
extern int  AddMacAuthentication( short int PonPortIdx,  mac_address_t mac );
/*extern LONG (*assign_onuBwBasedMac_hookrtn)(SHORT onuMgtIdx, UCHAR * pMacAddr);*/

 int SyncSysTimeToOnu_New( short int PonPortIdx, short int OnuIdx );

unsigned int DEBUG_ONU_TDM_VOICE = V2R1_DISABLE;


#ifndef __ONU_CONFIG_AUTO
#define __ONU_CONFIG_AUTO
#endif

static int UpdateOnuEncryptKey_1( short int PonPortIdx, short int OnuIdx /*, PON_encryption_key_t PON_encryption_key*/);
static int StopOnuEncrypt( short int PonPortIdx, short int OnuIdx );
	
/*extern CT_Onu_EthPortItem_t  onuEthPort[20][MAXONUPERPON][MAX_ONU_ETHPORT];*/


#if( EPON_MODULE_ONU_REG_FILTER	 == EPON_MODULE_YES )
typedef struct OnuRegisterFilter{
	unsigned char valid;
	unsigned char RegisterFlag;
	unsigned char DeregisterFlag;
	unsigned char OamDiscoveryFlag;
	unsigned char agingtime;
	ONURegisterInfo_S  RegisterData;
	ONUDeregistrationInfo_S DeregisterData;
	short int last;
	short int next;
}OnuRegisterFilter_S;

#define  INVALID_ENTRY  0x1fffaaaa  

static unsigned long  OnuRegisterFilterSemId = 0;
static unsigned int  OnuRegisterFilter_head=INVALID_ENTRY;
static unsigned int  OnuRegisterFilter_tail =INVALID_ENTRY;
static unsigned int  OnuRegisterFilter_WaitTicks = 0;
#if 0
static OnuRegisterFilter_S  OnuRegisterFilterQueue[20 *(PON_PORT_ID_MAX_LLID_SUPPORTED+1)];
#else
static OnuRegisterFilter_S  *OnuRegisterFilterQueue;
#endif

void  InitOnuRegisterFilterQueue()
{
	int OltId;
	int OnuId;
	int entryOlt, entryOnu;
    ULONG ulSize;

#if 1
/*
#ifdef g_malloc
#undef g_malloc
#endif
*/
    ulSize = sizeof(OnuRegisterFilter_S) * MAXPON * (PON_PORT_ID_MAX_LLID_SUPPORTED+1);
    if ( NULL == (OnuRegisterFilterQueue = (OnuRegisterFilter_S*)g_malloc(ulSize)) )
    {
        VOS_ASSERT(0);
        sys_console_printf("\r\nFailed to g_malloc [OnuRegisterFilterQueue]!\r\n");
        VOS_TaskDelay(100);
        return RERROR;
    }
	VOS_MemZero(OnuRegisterFilterQueue, ulSize);
#endif

	for(OltId = 0; OltId < MAXPON; OltId ++)
	{
	    entryOlt = OltId * (PON_PORT_ID_MAX_LLID_SUPPORTED+1);
		for(OnuId = 0; OnuId < (PON_PORT_ID_MAX_LLID_SUPPORTED+1); OnuId++)
		{
			entryOnu = entryOlt + OnuId;
			OnuRegisterFilterQueue[entryOnu].RegisterFlag = V2R1_DISABLE;
			OnuRegisterFilterQueue[entryOnu].DeregisterFlag = V2R1_DISABLE;
			OnuRegisterFilterQueue[entryOnu].next = INVALID_ENTRY;
			OnuRegisterFilterQueue[entryOnu].last = INVALID_ENTRY;			
		}
	}
	if(OnuRegisterFilterSemId == 0 )
		OnuRegisterFilterSemId = VOS_SemMCreate(VOS_SEM_Q_PRIORITY);

	OnuRegisterFilter_head=INVALID_ENTRY;
	OnuRegisterFilter_tail =INVALID_ENTRY;

	OnuRegisterFilter_WaitTicks = VOS_Ticks_Per_Seconds * 20;

	return;
}


/*****************************************************
 *
 *    Function:  sendOnuRegistrationMsg()
 *
 *    Param:    
 *                 
 *    Desc:   ONU register event callback function;send a msg to onu handler task
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/

static void  AddNodeToRegisterFilterQueue(int entry)
{
	if(entry > (MAXPON * (PON_PORT_ID_MAX_LLID_SUPPORTED+1)))
		return;

	/* 队列为空*/
	if( OnuRegisterFilter_head == INVALID_ENTRY )
		{
		OnuRegisterFilter_head = entry;
		OnuRegisterFilter_tail = entry;
		OnuRegisterFilterQueue[entry].last = entry;
		OnuRegisterFilterQueue[entry].next = entry;
		}
	
	/*  将节点加入到队列未尾*/
	else{
		OnuRegisterFilterQueue[entry].last = OnuRegisterFilter_tail;
		OnuRegisterFilterQueue[OnuRegisterFilter_tail].next = entry;
		OnuRegisterFilter_tail = entry;
		OnuRegisterFilterQueue[entry].next = entry;
		}
}

static void  DeleteNodeFromRegisterFilterQueue(int entry)
{
	if(entry > (MAXPON * (PON_PORT_ID_MAX_LLID_SUPPORTED+1)))
		return;

	/*  将节点从队列中删除*/

	/* 1 队列为空*/
	if( OnuRegisterFilter_head == INVALID_ENTRY )
		{
		OnuRegisterFilter_tail = INVALID_ENTRY;
		}

	/* 2 队列中只有一个节点*/
	else if(OnuRegisterFilter_head == OnuRegisterFilter_tail)
		{
		OnuRegisterFilter_head = INVALID_ENTRY;
		OnuRegisterFilter_tail = INVALID_ENTRY;
		}

	/* 3 被删除节点是头节点*/
	else if(OnuRegisterFilter_head == entry)
		{
		OnuRegisterFilter_head = OnuRegisterFilterQueue[entry].next;
		OnuRegisterFilterQueue[OnuRegisterFilter_head].last = OnuRegisterFilter_head;
		}

	/* 4 被删除节点是尾节点*/
	else if(OnuRegisterFilter_tail == entry)
		{
		OnuRegisterFilter_tail = OnuRegisterFilterQueue[entry].last;
		OnuRegisterFilterQueue[OnuRegisterFilter_tail].next = OnuRegisterFilter_tail;
		}

	/* 5 被删除节点是中间节点*/
	else {
		int last, next;
		last = OnuRegisterFilterQueue[entry].last;
		next = OnuRegisterFilterQueue[entry].next;
		OnuRegisterFilterQueue[last].next = next;
		OnuRegisterFilterQueue[next].last = last;
		}

	/* 清空节点*/
	OnuRegisterFilterQueue[entry].RegisterFlag = V2R1_DISABLE;
	OnuRegisterFilterQueue[entry].DeregisterFlag = V2R1_DISABLE;
	OnuRegisterFilterQueue[entry].next = INVALID_ENTRY;
	OnuRegisterFilterQueue[entry].last = INVALID_ENTRY;
	
}

static int sendOnuRegistrationMsg_static( const short int		  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const mac_address_t				  mac_address,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard )
{
	int Entry;

	if(( olt_id < 0) ||(olt_id > MAXPON )) return ( RERROR );
	if( onu_id > PON_PORT_ID_MAX_LLID_SUPPORTED ) return ( RERROR );	

	Entry = olt_id*(PON_PORT_ID_MAX_LLID_SUPPORTED+1)+onu_id;

	/* 保存注册参数*/
	OnuRegisterFilterQueue[Entry].RegisterData.olt_id = olt_id;
	OnuRegisterFilterQueue[Entry].RegisterData.onu_id = onu_id;
	VOS_MemCpy(&(OnuRegisterFilterQueue[Entry].RegisterData.mac_address[0]), mac_address, BYTES_IN_MAC_ADDRESS);
	VOS_MemCpy(&(OnuRegisterFilterQueue[Entry].RegisterData.authenticatioin_sequence[0]), authentication_sequence, PON_AUTHENTICATION_SEQUENCE_SIZE );
	OnuRegisterFilterQueue[Entry].RegisterData.supported_oam_standard = supported_oam_standard;

	/* 将节点加入到过滤队列*/
	/* 1 -- 队列中没有未处理的注册消息*/
	if( OnuRegisterFilterQueue[Entry].RegisterFlag != V2R1_ENABLE)
	{
		OnuRegisterFilterQueue[Entry].RegisterFlag = V2R1_ENABLE;
		OnuRegisterFilterQueue[Entry].OamDiscoveryFlag = V2R1_DISABLE;
		
		/* 队列中是否有未处理的离线消息*/
		if(OnuRegisterFilterQueue[Entry].DeregisterFlag != V2R1_ENABLE)
		{
			AddNodeToRegisterFilterQueue(Entry);
		}
		/* 
			表明，过滤队列中先来了一个离线消息，又来了一个注册消息
			此时，队列中节点位置不做改动; 节点中包含离线和注册两个消息
		*/
		else{ /* nothing need to do */
		}
	}

	/*  2 -- 队列中有未处理的注册消息
		应该说，若出现这种情况，表明出现了异常: 连续来了注册消息，而中间没有离线消息
		做冗余处理，用新的注册参数替换之前的注册参数
		队列中节点位置不做改动
	*/
	else{ /* nothing need to do */
	}
	return (ROK);
}

int sendOnuRegistrationMsg( const short int		  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const mac_address_t				  mac_address,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard )
{
	int Ret;
	
	if(VOS_SemTake(OnuRegisterFilterSemId, OnuRegisterFilter_WaitTicks) != VOS_OK)
		{
		AddPendingOnu(olt_id, -1, onu_id, (unsigned char *)mac_address, 0);
		return (VOS_ERROR);
		}
	
	Ret = sendOnuRegistrationMsg_static(olt_id, onu_id, mac_address, authentication_sequence, supported_oam_standard);

	VOS_SemGive(OnuRegisterFilterSemId);

	return(Ret);
}

void showOnuRegisterFilterQueue()
{
	int entry;
	
	for( entry=0; entry < (MAXPON * (PON_PORT_ID_MAX_LLID_SUPPORTED+1)); entry++)
		{
		if(OnuRegisterFilterQueue[entry].RegisterFlag == V2R1_ENABLE)
			sys_console_printf("register flag = 1 entry = %d\r\n", entry);
		if(OnuRegisterFilterQueue[entry].DeregisterFlag == V2R1_ENABLE)
			sys_console_printf("deregister flag = 1 entry = %d\r\n", entry);
		}

	sys_console_printf(" filter queue head %d tail %d\r\n", OnuRegisterFilter_head, OnuRegisterFilter_tail);

}

static int sendOnuDeregistrationMsg_static(const short int olt_id,  const PON_onu_id_t onu_id, PON_onu_deregistration_code_t deregistration_code)
{
	int Entry;

	if(( olt_id < 0) ||(olt_id > MAXPON )) return ( RERROR );
	if( onu_id > PON_PORT_ID_MAX_LLID_SUPPORTED ) return ( RERROR );	

	Entry = olt_id*(PON_PORT_ID_MAX_LLID_SUPPORTED+1)+onu_id;

	/* 1 -- 队列中有未处理的注册消息*/
	if( OnuRegisterFilterQueue[Entry].RegisterFlag == V2R1_ENABLE)
	{
		/* 清除注册消息*/
		OnuRegisterFilterQueue[Entry].RegisterFlag = V2R1_DISABLE;
		OnuRegisterFilterQueue[Entry].OamDiscoveryFlag = V2R1_DISABLE;
		/* 判断队列中是否有离线消息。若没有，则将节点从队列中删除*/
		if(OnuRegisterFilterQueue[Entry].DeregisterFlag != V2R1_ENABLE )
		{
			DeleteNodeFromRegisterFilterQueue(Entry);
		}
	}

	/* 2 -- 队列中没有注册消息*/
	else
	{
		/* 判断队列中是否有离线消息。
			若没有，则保存离线参数; 并将节点保存到过滤队列*/
		if(OnuRegisterFilterQueue[Entry].DeregisterFlag != V2R1_ENABLE )
		{
			OnuRegisterFilterQueue[Entry].DeregisterFlag = V2R1_ENABLE;
			OnuRegisterFilterQueue[Entry].DeregisterData.deregistration_code = deregistration_code;
			OnuRegisterFilterQueue[Entry].DeregisterData.olt_id = olt_id;
			OnuRegisterFilterQueue[Entry].DeregisterData.onu_id = onu_id;
			AddNodeToRegisterFilterQueue(Entry);
		}
		
		/* 若有，则说明出现异常，连续来了离线消息，而中间没有注册消息*/
		else{
		}
	}

	return (ROK);
}

int sendOnuDeregistrationMsg(const short int olt_id,  const PON_onu_id_t onu_id, PON_onu_deregistration_code_t deregistration_code)
{
	int Ret;
	
	if(VOS_SemTake(OnuRegisterFilterSemId, OnuRegisterFilter_WaitTicks) != VOS_OK)
		{
		return (VOS_ERROR);
		}
	
	Ret = sendOnuDeregistrationMsg_static(olt_id, onu_id, deregistration_code);

	VOS_SemGive(OnuRegisterFilterSemId);

	return(Ret);
}

void  HandlerRegisterFilterQueue()
{
	int  count;
	int entry;
	
	ONURegisterInfo_S  RegisterData;
	ONUDeregistrationInfo_S DeregisterData;
	int RegisterFlag=V2R1_DISABLE, DeregisterFlag = V2R1_DISABLE;
	
	
	/* 若队列中没有未处理的消息，则返回*/
	if( OnuRegisterFilter_head == INVALID_ENTRY )
		return;

	/* 处理ONU 注册/ 离线；
		此函数每秒调用一次；为了不至于长时间占用CPU，限制每次处理最多15 个消息
		按FIFO 原则，从队列头指针开始处理
		*/
	count = 0;
	while(( OnuRegisterFilter_head != INVALID_ENTRY ) && (count < 15))
	{
		if(VOS_SemTake(OnuRegisterFilterSemId, 10) != VOS_OK)
			return;
		
		entry = OnuRegisterFilter_head;
		if(entry > (MAXPON * (PON_PORT_ID_MAX_LLID_SUPPORTED+1)))
		{
			OnuRegisterFilter_head = INVALID_ENTRY;
			return;
		}
		
		if(OnuRegisterFilterQueue[entry].DeregisterFlag == V2R1_ENABLE)
		{
			DeregisterData.olt_id = OnuRegisterFilterQueue[entry].DeregisterData.olt_id;
			DeregisterData.onu_id = OnuRegisterFilterQueue[entry].DeregisterData.onu_id;
			DeregisterData.deregistration_code = OnuRegisterFilterQueue[entry].DeregisterData.deregistration_code;
			DeregisterFlag = V2R1_ENABLE;
		}

		if(OnuRegisterFilterQueue[entry].RegisterFlag == V2R1_ENABLE)
		{
			if( V2R1_CTC_STACK )
			{
			}
			else
			{
				RegisterData.olt_id = OnuRegisterFilterQueue[entry].RegisterData.olt_id;
				RegisterData.onu_id = OnuRegisterFilterQueue[entry].RegisterData.onu_id;
				VOS_MemCpy(&(RegisterData.mac_address[0]), &(OnuRegisterFilterQueue[entry].RegisterData.mac_address[0]), BYTES_IN_MAC_ADDRESS);
				VOS_MemCpy(&(RegisterData.authenticatioin_sequence[0]), &(OnuRegisterFilterQueue[entry].RegisterData.authenticatioin_sequence[0]), PON_AUTHENTICATION_SEQUENCE_SIZE);
				RegisterData.supported_oam_standard = OnuRegisterFilterQueue[entry].RegisterData.supported_oam_standard;
				RegisterFlag = V2R1_ENABLE;
			}
		}

		DeleteNodeFromRegisterFilterQueue(entry);		
		VOS_SemGive(OnuRegisterFilterSemId);

		if(DeregisterFlag == V2R1_ENABLE)
			OnuDeregisterHandler( &DeregisterData );

		if(RegisterFlag == V2R1_ENABLE)
			OnuRegisterHandler( &RegisterData );
		
		count++;
	}

}


#else

int sendOnuRegistrationMsg( const short int		  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 const mac_address_t				  mac_address,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard,
					 unsigned int event_flags)
{
	ONURegisterInfo_S *OnuRegisterData;
	LONG ret;
	int OltProtectMode = 0;
	/*GeneralMsgBody_S OnuRegisterMsg; */
	unsigned long aulMsg[4] = { MODULE_PON, FC_ONU_REGISTER, 0,0 };
	
	if( !OLT_LOCAL_ISVALID(olt_id) )
	{
		VOS_ASSERT(0);
		return ( RERROR );
	}
	if( !LLID_ISUNICAST(onu_id) )
	{
		sys_console_printf(" No enough LLID for onu register\r\n");
		return( RERROR );
	}
    
	/* 当PON 端口使能了保护切换，处于备用状态的PON 端口不处理ＯＮＵ注册*/
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
	/* clear pon port swap counter */	
	if( PonPortSwapEnableQuery( olt_id ) == V2R1_PON_PORT_SWAP_ENABLE )
	{
	    /*for onu swap by jinhl@2013-02-22*/
	    OltProtectMode = GetPonPortHotSwapMode( olt_id );
	    if ( PON_SWAPMODE_ISOLT(OltProtectMode) )
    	{
			
			if( PonPortHotStatusQuery( olt_id ) == V2R1_PON_PORT_SWAP_PASSIVE )
			{
				if( (V2R1_ENABLE == EVENT_PONSWITCH) || (EVENT_REGISTER == V2R1_ENABLE) )
				{
					sys_console_printf("\r\n find onu is registered on the passive pon\r\n" );
				}
				AddPendingOnu( olt_id, -1, onu_id, mac_address, 0 );
				/*PAS_deregister_onu(PonPort_id,onu_id, FALSE);*/
				return( RERROR );
			}
    	}
	}
#endif	
	OnuRegisterData = ( ONURegisterInfo_S *) VOS_Malloc( sizeof(ONURegisterInfo_S),  MODULE_ONU ); 
	if( OnuRegisterData == NULL )
	{
		/*sys_console_printf("Error, Malloc buffer not satified ( sendOnuRegistrationMsg())\r\n" );*/
		return( RERROR );
	}

	VOS_MemSet( (char *)OnuRegisterData, 0, sizeof( ONURegisterInfo_S ) );
	OnuRegisterData->olt_id = olt_id;
	OnuRegisterData->onu_id = onu_id;
	VOS_MemCpy(OnuRegisterData->mac_address, mac_address, BYTES_IN_MAC_ADDRESS );
	VOS_MemCpy(OnuRegisterData->authenticatioin_sequence, authentication_sequence, PON_AUTHENTICATION_SEQUENCE_SIZE );
	OnuRegisterData->supported_oam_standard = supported_oam_standard;
	OnuRegisterData->event_flags = event_flags;
	
	aulMsg[2] = sizeof(ONURegisterInfo_S);

    if ( SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER )
    {
        /* BCM PON板需要走PON的CTC及其他扩展OAM的发现处理 */
    	ONURegisterInfo_S *OnuRegisterData2;
        
    	if ( NULL != (OnuRegisterData2 = ( ONURegisterInfo_S *) VOS_Malloc( sizeof(ONURegisterInfo_S),  MODULE_ONU )) )
        {
            VOS_MemCpy(OnuRegisterData2, OnuRegisterData, sizeof(ONURegisterInfo_S));
        
        	aulMsg[3] = ( int )OnuRegisterData2;
        	ret = VOS_QueSend( g_Pon_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
        	if( ret !=  VOS_OK )
        	{
        		if ( EVENT_REGISTER == V2R1_ENABLE )
        		{
        			sys_console_printf("PON task is too busy to discard reg_msg, PONid:%d,llid:%d\r\n", OnuRegisterData2->olt_id, OnuRegisterData2->onu_id );
        		}

        		VOS_Free( (void *) OnuRegisterData2 );
        		VOS_Free( (void *) OnuRegisterData );
        		return( RERROR );
        	}	
        }   
        else
    	{
    		VOS_Free( (void *) OnuRegisterData );
    		return( RERROR );
    	}
    }

	aulMsg[3] = ( int )OnuRegisterData;
	ret = VOS_QueSend( g_Onu_Queue_Id/*g_Pon_Queue_Id*/, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK )
	{
		if ( EVENT_REGISTER == V2R1_ENABLE )
		{
			sys_console_printf("ONU task is too busy to discard reg_msg, PONid:%d,llid:%d\r\n", OnuRegisterData->olt_id, OnuRegisterData->onu_id );
		}

		VOS_Free( (void *) OnuRegisterData );
		return( RERROR );
	}	
	return ( ROK );
	
}
int sendGponOnuRegistrationMsg( const short int		  olt_id, 
					 const PON_onu_id_t				      onu_id, 
					 char*              				  sn,
					 const PON_authentication_sequence_t  authentication_sequence,
					 const OAM_standard_version_t		  supported_oam_standard,
					 unsigned int event_flags)
{
	gONURegisterInfo_S *OnuRegisterData;
	LONG ret;
	int OltProtectMode = 0;
	/*GeneralMsgBody_S OnuRegisterMsg; */
	unsigned long aulMsg[4] = { MODULE_PON, FC_ONU_REGISTER, 0,0 };
	
	if( !OLT_LOCAL_ISVALID(olt_id) )
	{
		VOS_ASSERT(0);
		return ( RERROR );
	}
	if( !LLID_ISUNICAST(onu_id) )
	{
		sys_console_printf(" No enough LLID for onu register\r\n");
		return( RERROR );
	}
    
	/* 当PON 端口使能了保护切换，处于备用状态的PON 端口不处理ＯＮＵ注册*/
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
	/* clear pon port swap counter */	
	if( PonPortSwapEnableQuery( olt_id ) == V2R1_PON_PORT_SWAP_ENABLE )
	{
	    /*for onu swap by jinhl@2013-02-22*/
	    OltProtectMode = GetPonPortHotSwapMode( olt_id );
	    if ( PON_SWAPMODE_ISOLT(OltProtectMode) )
    	{
			
			if( PonPortHotStatusQuery( olt_id ) == V2R1_PON_PORT_SWAP_PASSIVE )
			{
				if( (V2R1_ENABLE == EVENT_PONSWITCH) || (EVENT_REGISTER == V2R1_ENABLE) )
				{
					sys_console_printf("\r\n find onu is registered on the passive pon\r\n" );
				}
				/*AddPendingOnu( olt_id, -1, onu_id, mac_address, 0 );*/
				/*PAS_deregister_onu(PonPort_id,onu_id, FALSE);*/
				return( RERROR );
			}
    	}
	}
#endif	
	OnuRegisterData = ( gONURegisterInfo_S *) VOS_Malloc( sizeof(gONURegisterInfo_S),  MODULE_ONU ); 
	if( OnuRegisterData == NULL )
	{
		/*sys_console_printf("Error, Malloc buffer not satified ( sendOnuRegistrationMsg())\r\n" );*/
		return( RERROR );
	}

	VOS_MemSet( (char *)OnuRegisterData, 0, sizeof( gONURegisterInfo_S ) );
	OnuRegisterData->olt_id = olt_id;
	OnuRegisterData->onu_id = onu_id;
	VOS_MemCpy(OnuRegisterData->series_number, sn, GPON_ONU_SERIAL_NUM_STR_LEN );
	VOS_MemCpy(OnuRegisterData->authenticatioin_sequence, authentication_sequence, PON_AUTHENTICATION_SEQUENCE_SIZE );
	OnuRegisterData->supported_oam_standard = supported_oam_standard;
	OnuRegisterData->event_flags = event_flags;
	
	aulMsg[2] = sizeof(gONURegisterInfo_S);
	aulMsg[3] = ( int )OnuRegisterData;
	ret = VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK )
	{
		if ( EVENT_REGISTER == V2R1_ENABLE )
		{
			sys_console_printf("ONU task is too busy to discard reg_msg, PONid:%d,llid:%d\r\n", OnuRegisterData->olt_id, OnuRegisterData->onu_id );
		}

		VOS_Free( (void *) OnuRegisterData );
		return( RERROR );
	}	
	return ( ROK );
	
}

int sendOnuDeregistrationMsg(const short int olt_id, const PON_onu_id_t onu_id, PON_onu_deregistration_code_t deregistration_code, unsigned int event_flags)
{
	ONUDeregistrationInfo_S  *OnuDeregistrationData;
	LONG ret;
	unsigned long  aulMsg[4] = { MODULE_PON, FC_ONU_DEREGISTER, 0, 0 };

	if( !OLT_LOCAL_ISVALID(olt_id) )
	{
		VOS_ASSERT(0);
		return ( RERROR );
	}
	if( !LLID_ISVALID(onu_id) )
	{
		sys_console_printf(" Invalid LLID for onu deregister\r\n");
		return( RERROR );
	}

	OnuDeregistrationData = (ONUDeregistrationInfo_S *) VOS_Malloc( sizeof(ONUDeregistrationInfo_S), MODULE_ONU );
	if( OnuDeregistrationData  == NULL )
	{
		/*sys_console_printf("Error, Malloc buffer not satified ( sendOnuDeregistrationMsg())\r\n" );*/
		return( RERROR );
	}

	VOS_MemSet( (char *)OnuDeregistrationData, 0, sizeof(ONUDeregistrationInfo_S ) );
	OnuDeregistrationData->olt_id = olt_id;
	OnuDeregistrationData->onu_id = onu_id;
	OnuDeregistrationData->deregistration_code = deregistration_code;
	OnuDeregistrationData->event_flags = event_flags;

	aulMsg[2] = sizeof( ONUDeregistrationInfo_S );

    if ( SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER )
    {
        /* BCM PON板需要走PON的CTC及其他扩展OAM的发现处理 */
    	ONUDeregistrationInfo_S *OnuDeregistrationData2;
        
    	if ( NULL != (OnuDeregistrationData2 = ( ONUDeregistrationInfo_S *) VOS_Malloc( sizeof(ONUDeregistrationInfo_S),  MODULE_ONU )) )
        {
            VOS_MemCpy(OnuDeregistrationData2, OnuDeregistrationData, sizeof(ONUDeregistrationInfo_S));
        
        	aulMsg[3] = ( int )OnuDeregistrationData2;
        	ret = VOS_QueSend( g_Pon_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
        	if( ret !=  VOS_OK )
        	{
        		if ( EVENT_REGISTER == V2R1_ENABLE )
        		{
        			sys_console_printf("PON task is too busy to discard dereg_msg, PONid:%d,llid:%d\r\n", OnuDeregistrationData2->olt_id, OnuDeregistrationData2->onu_id );
        		}

        		VOS_Free( (void *) OnuDeregistrationData2 );
        		VOS_Free( (void *) OnuDeregistrationData );
        		return( RERROR );
        	}	
        }   
        else
    	{
    		VOS_Free( (void *) OnuDeregistrationData );
    		return( RERROR );
    	}
    }
	
	aulMsg[3] = ( int ) OnuDeregistrationData;
	ret = VOS_QueSend( g_Onu_Queue_Id/*g_Pon_Queue_Id*/, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK )
	{
		if ( EVENT_REGISTER == V2R1_ENABLE )
		{
			sys_console_printf("ONU task is too busy to discard dereg_msg, PONid:%d,llid:%d\r\n", OnuDeregistrationData->olt_id, OnuDeregistrationData->onu_id );
		}
		VOS_Free( (void *)OnuDeregistrationData );
		return( RERROR );
	}	
	return ( ROK );
	
}

int sendOnuDeregistrationEvent(const short int olt_id, const short int onu_idx, PON_onu_deregistration_code_t deregistration_code, unsigned int event_flags)
{
	ONUDeregistrationInfo_S  *OnuDeregistrationData;
	LONG ret;
	unsigned long  aulMsg[4] = { MODULE_PON, FC_ONU_DEREGISTER, 0, 0 };

	if( !OLT_LOCAL_ISVALID(olt_id) )
	{
		VOS_ASSERT(0);
		return ( RERROR );
	}
	if( !ONU_IDX_ISVALID(onu_idx) )
	{
		sys_console_printf(" Invalid ONUIDX for onu deregister\r\n");
		return( RERROR );
	}
    if ( deregistration_code < 0 )
    {
		VOS_ASSERT(0);
        deregistration_code = -deregistration_code;
    }

	OnuDeregistrationData = (ONUDeregistrationInfo_S *) VOS_Malloc( sizeof(ONUDeregistrationInfo_S), MODULE_ONU );
	if( OnuDeregistrationData  == NULL )
	{
		/*sys_console_printf("Error, Malloc buffer not satified ( sendOnuDeregistrationMsg())\r\n" );*/
		return( RERROR );
	}

	VOS_MemSet( (char *)OnuDeregistrationData, 0, sizeof(ONUDeregistrationInfo_S ) );
	OnuDeregistrationData->olt_id = olt_id;
	OnuDeregistrationData->onu_id = onu_idx;
	OnuDeregistrationData->deregistration_code = -deregistration_code;
	OnuDeregistrationData->event_flags = event_flags;

	aulMsg[2] = sizeof( ONUDeregistrationInfo_S );
	aulMsg[3] = ( int ) OnuDeregistrationData;

	ret = VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK )
	{
		if ( EVENT_REGISTER == V2R1_ENABLE )
		{
			sys_console_printf("ONU task is too busy to discard dereg_msg, PONid:%d, OnuIdx:%d\r\n", OnuDeregistrationData->olt_id, OnuDeregistrationData->onu_id );
		}
		VOS_Free( (void *)OnuDeregistrationData );
		return( RERROR );
	}	
    
	return ( ROK );	
}

int sendOnuExtOamDiscoveryMsg(const short int olt_id, const PON_onu_id_t onu_id, CTC_STACK_discovery_state_t result, unsigned char Number_of_records, CTC_STACK_oui_version_record_t *onu_version_records_list, unsigned int event_flags)
{
	ExtOAMDiscovery_t  *ExtOAMDiscovery_Data;
	LONG ret;
	unsigned long aulMsg[4] = { MODULE_PON, FC_EXTOAMDISCOVERY, 0,0 };

	if( !OLT_LOCAL_ISVALID(olt_id) )
	{
		VOS_ASSERT(0);
		return ( RERROR );
	}
	if( !LLID_ISVALID(onu_id) )
	{
		sys_console_printf(" Invalid LLID for onu ext-discover\r\n");
		return( RERROR );
	}
    if ( NULL == onu_version_records_list )
    {
#if 1
        /* modified by liwei056@2012-11-29 for 回调进入Pending队列，可能回早于前一个注册消息的处理，得不到其MAC地址 */
        sys_console_printf("\r\nolt_id = %d, llid = %d, ExtOamDiscovery fail for loose oui(%d)\r\n", olt_id, onu_id, Number_of_records);

        /* 认为这也是CTC扩展发现失败的一种情况 */
        result = CTC_DISCOVERY_STATE_IN_PROGRESS;
        
        /*Number_of_records = 0;*/
#else
        /*modified by luh 2012-10-12 ，12 EPON板有onu不停离线注册时，当发现失败不再打印断言，而是直接进pending队列*/
		/*VOS_ASSERT(0);*/
        char local_mac[6]={0};
        short int OnuIdx = GetOnuIdxByLlid(olt_id, onu_id);
        if(ONU_IDX_ISVALID(OnuIdx))
        {
            ONU_MGMT_SEM_TAKE;
            VOS_MemCpy(local_mac, OnuMgmtTable[olt_id*MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, 6);
            ONU_MGMT_SEM_GIVE;

            AddPendingOnu(olt_id, OnuIdx, onu_id, local_mac, PENDING_REASON_CODE_ExtOAM_FAIL);
        }
        else
        {
            AddPendingOnu(olt_id, OnuIdx, onu_id, local_mac, PENDING_REASON_CODE_ExtOAM_FAIL);
        }
        
		return( RERROR );
#endif        
    }
	
	ExtOAMDiscovery_Data = ( ExtOAMDiscovery_t *) VOS_Malloc( sizeof(ExtOAMDiscovery_t),  MODULE_ONU ); 
	if( ExtOAMDiscovery_Data  == NULL )
	{
		/* sys_console_printf("Error,Malloc buffer Err(Onu_ExtOAMDiscovery_Handler)\r\n" ); */
		return( RERROR );
	}

	VOS_MemSet( (char *)ExtOAMDiscovery_Data, 0, sizeof( ExtOAMDiscovery_t ) );
	ExtOAMDiscovery_Data->event_flags = event_flags;
	ExtOAMDiscovery_Data->olt_id = olt_id;
	ExtOAMDiscovery_Data->llid = onu_id;
	ExtOAMDiscovery_Data->result = result;
	ExtOAMDiscovery_Data->Number_of_records = Number_of_records;
	for(ret =0; ret < Number_of_records; ret ++ )
	{
		ExtOAMDiscovery_Data->onu_version_records_list[ret].oui[0] = onu_version_records_list[ret].oui[0];
		ExtOAMDiscovery_Data->onu_version_records_list[ret].oui[1] = onu_version_records_list[ret].oui[1];
		ExtOAMDiscovery_Data->onu_version_records_list[ret].oui[2] = onu_version_records_list[ret].oui[2];
		ExtOAMDiscovery_Data->onu_version_records_list[ret].version = onu_version_records_list[ret].version;
	}
	
	aulMsg[2] = sizeof(ExtOAMDiscovery_t);
	aulMsg[3] = ( int )ExtOAMDiscovery_Data;

	ret = VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK )
	{
		if ( EVENT_REGISTER == V2R1_ENABLE )
		{
			sys_console_printf("ONU task is too busy to discard ext_discover_msg, PONid:%d, llid:%d\r\n", olt_id, onu_id );
		}
		VOS_Free( (void *) ExtOAMDiscovery_Data );
		return( RERROR );
	}
    
	return ( ROK );
}

#endif

int sendOnuExtOamOverMsg(const short int olt_id, const PON_onu_id_t onu_id, const mac_address_t	mac_address, CTC_STACK_discovery_state_t result)
{
	OnuEventData_s  *ExtOAMDiscovery_Data;
	LONG ret;
	unsigned long aulMsg[4] = { MODULE_PON, FC_EXTOAMDISCOVERY, 0,0 };

	if( !OLT_LOCAL_ISVALID(olt_id) )
	{
		VOS_ASSERT(0);
		return ( RERROR );
	}
	if( !LLID_ISVALID(onu_id) )
	{
		sys_console_printf(" Invalid LLID for onu ext-discover\r\n");
		return( RERROR );
	}
	
	ExtOAMDiscovery_Data = ( OnuEventData_s *) VOS_Malloc( sizeof(OnuEventData_s),  MODULE_ONU ); 
	if( ExtOAMDiscovery_Data  == NULL )
	{
		/* sys_console_printf("Error,Malloc buffer Err(Onu_ExtOAMDiscovery_Handler)\r\n" ); */
		return( RERROR );
	}

	VOS_MemSet( (char *)ExtOAMDiscovery_Data, 0, sizeof( OnuEventData_s ) );
	ExtOAMDiscovery_Data->PonPortIdx = olt_id;
	ExtOAMDiscovery_Data->llid = onu_id;
    VOS_MemCpy(ExtOAMDiscovery_Data->onu_mac, mac_address, 6);
	
	aulMsg[2] = sizeof(OnuEventData_s);
	aulMsg[3] = ( int )ExtOAMDiscovery_Data;

	ret = VOS_QueSend( g_Pon_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK )
	{
		if ( EVENT_REGISTER == V2R1_ENABLE )
		{
			sys_console_printf("PON task is too busy to discard ext_discover_msg, PONid:%d, llid:%d\r\n", olt_id, onu_id );
		}
		VOS_Free( (void *) ExtOAMDiscovery_Data );
		return( RERROR );
	}
	return ( ROK );
}

/* 参见 onu_ext_disc_evt_rtnhook */

/*从PON板给主控板发注册同步消息*//*add by shixh20100525*/
/* modified by xieshl 20110614, 生成CDP报文后直接发送，不需要再拷贝一次，
    ONU Deregister同步消息重新定义，以减少报文分片 */
/* modified by xieshl 20111129, 需要支持pon-主控-备用主控同步 */
static int send_onuSyncMessage_Register( ULONG dst_slotno, short int PonPortIdx, short int OnuIdx )
{
	LONG ret;
	LONG msglen=sizeof(onu_sync_reg_msg_t);
	onu_sync_reg_msg_t  * pMsg = NULL;
	int Onuentry;
   
	/*CHECK_ONU_RANGE;

	if ( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return VOS_OK;*/

	pMsg = ( onu_sync_reg_msg_t * ) CDP_AllocMsg( msglen, MODULE_ONU );              
	if(pMsg == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	/*将一部分管理信息从PON板上传到 主控板上*/
	pMsg->OnuSyncMsgHead.ponSlotIdx = GetCardIdxByPonChip(PonPortIdx);/*SYS_LOCAL_MODULE_SLOTNO;*/
	pMsg->OnuSyncMsgHead.portIdx = GetPonPortByPonChip(PonPortIdx);
	pMsg->OnuSyncMsgHead.onuIdx = OnuIdx;
	pMsg->OnuSyncMsgHead.onuEventId = ONU_EVENT_REGISTER;
	
	VOS_MemZero( &(pMsg->DeviceInfo), sizeof(DeviceInfo_S));
	Onuentry = PonPortIdx*MAXONUPERPON+OnuIdx;
	
	ONU_MGMT_SEM_TAKE;
	pMsg->IsGponOnu = OnuMgmtTable[Onuentry].IsGponOnu;
	pMsg->TcontNum = OnuMgmtTable[Onuentry].TcontNum;
	pMsg->GEMPortNum = OnuMgmtTable[Onuentry].GEMPortNum;
	pMsg->OmccVersion = OnuMgmtTable[Onuentry].OmccVersion;
	pMsg->PmEnable = OnuMgmtTable[Onuentry].PmEnable;
	pMsg->GemPortId = OnuMgmtTable[Onuentry].GemPortId;
	pMsg->BerInterval = OnuMgmtTable[Onuentry].BerInterval;
	pMsg->ImageValidIndex = OnuMgmtTable[Onuentry].ImageValidIndex;
    
	VOS_StrnCpy( pMsg->DeviceInfo.equipmentID,OnuMgmtTable[Onuentry].DeviceInfo.equipmentID, MAXEQUIPMENTIDLEN);
	pMsg->DeviceInfo.equipmentIDLen = OnuMgmtTable[Onuentry].DeviceInfo.equipmentIDLen;

	VOS_StrnCpy( pMsg->DeviceInfo.DevicePassward,OnuMgmtTable[Onuentry].DeviceInfo.DevicePassward, MAXDEVICEPWDLEN);
	pMsg->DeviceInfo.DevicePasswardLen = OnuMgmtTable[Onuentry].DeviceInfo.DevicePasswardLen;
		
	VOS_StrnCpy( pMsg->DeviceInfo.VendorInfo,OnuMgmtTable[Onuentry].DeviceInfo.VendorInfo, MAXVENDORINFOLEN);
	pMsg->DeviceInfo.VendorInfoLen = OnuMgmtTable[Onuentry].DeviceInfo.VendorInfoLen;/*VOS_StrLen(OnuMgmtTable[Onuentry].DeviceInfo.VendorInfo);*/

	VOS_StrnCpy( pMsg->DeviceInfo.DeviceName,OnuMgmtTable[Onuentry].DeviceInfo.DeviceName, MAXDEVICENAMELEN);
	pMsg->DeviceInfo.DeviceNameLen = OnuMgmtTable[Onuentry].DeviceInfo.DeviceNameLen;/*VOS_StrLen(OnuMgmtTable[Onuentry].DeviceInfo.DeviceName);*/

	VOS_StrnCpy( pMsg->DeviceInfo.DeviceDesc,OnuMgmtTable[Onuentry].DeviceInfo.DeviceDesc, MAXDEVICEDESCLEN);
	pMsg->DeviceInfo.DeviceDescLen = OnuMgmtTable[Onuentry].DeviceInfo.DeviceDescLen;/*VOS_StrLen(OnuMgmtTable[Onuentry].DeviceInfo.DeviceDesc);*/

	VOS_StrnCpy( pMsg->DeviceInfo.Location,OnuMgmtTable[Onuentry].DeviceInfo.Location, MAXLOCATIONLEN);
	pMsg->DeviceInfo.LocationLen = OnuMgmtTable[Onuentry].DeviceInfo.LocationLen;/*VOS_StrLen(OnuMgmtTable[Onuentry].DeviceInfo.Location);*/

	VOS_MemCpy( pMsg->DeviceInfo.MacAddr,OnuMgmtTable[Onuentry].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS);

	VOS_StrnCpy( pMsg->DeviceInfo.SwVersion,OnuMgmtTable[Onuentry].DeviceInfo.SwVersion, MAXSWVERSIONLEN);
	pMsg->DeviceInfo.SwVersionLen = OnuMgmtTable[Onuentry].DeviceInfo.SwVersionLen;/*VOS_StrLen(OnuMgmtTable[Onuentry].DeviceInfo.SwVersion);*/
	VOS_StrnCpy( pMsg->DeviceInfo.SwVersion1,OnuMgmtTable[Onuentry].DeviceInfo.SwVersion1, MAXSWVERSIONLEN);

	VOS_StrnCpy( pMsg->DeviceInfo.HwVersion,OnuMgmtTable[Onuentry].DeviceInfo.HwVersion, MAXHWVERSIONLEN);
	pMsg->DeviceInfo.HwVersionLen = OnuMgmtTable[Onuentry].DeviceInfo.HwVersionLen;/*VOS_StrLen(OnuMgmtTable[Onuentry].DeviceInfo.HwVersion);*/

	VOS_StrnCpy( pMsg->DeviceInfo.BootVersion,OnuMgmtTable[Onuentry].DeviceInfo.BootVersion, MAXBOOTVERSIONLEN);
	pMsg->DeviceInfo.BootVersionLen = OnuMgmtTable[Onuentry].DeviceInfo.BootVersionLen;/*VOS_StrLen(OnuMgmtTable[Onuentry].DeviceInfo.BootVersion);*/

	VOS_StrnCpy( pMsg->DeviceInfo.FwVersion,OnuMgmtTable[Onuentry].DeviceInfo.FwVersion, MAXFWVERSIONLEN);
	pMsg->DeviceInfo.FwVersionLen = OnuMgmtTable[Onuentry].DeviceInfo.FwVersionLen;/*VOS_StrLen(OnuMgmtTable[Onuentry].DeviceInfo.FwVersion);*/

	VOS_StrnCpy( pMsg->DeviceInfo.DeviceSerial_No,OnuMgmtTable[Onuentry].DeviceInfo.DeviceSerial_No, MAXSNLEN);
	pMsg->DeviceInfo.DeviceSerial_NoLen = OnuMgmtTable[Onuentry].DeviceInfo.DeviceSerial_NoLen;/*VOS_StrLen(OnuMgmtTable[Onuentry].DeviceInfo.DeviceSerial_No);*/

	VOS_StrnCpy( pMsg->DeviceInfo.MadeInDate,OnuMgmtTable[Onuentry].DeviceInfo.MadeInDate, MAXDATELEN);
	pMsg->DeviceInfo.MadeInDateLen = OnuMgmtTable[Onuentry].DeviceInfo.MadeInDateLen; /*VOS_StrLen(OnuMgmtTable[Onuentry].DeviceInfo.MadeInDate);*/

	pMsg->DeviceInfo.AutoConfigFlag = OnuMgmtTable[Onuentry].DeviceInfo.AutoConfigFlag;
	pMsg->DeviceInfo.type = OnuMgmtTable[Onuentry].DeviceInfo.type;
	/* B--added by liwei056@2011-2-23 for OnuPonChipType */
	pMsg->DeviceInfo.PonChipVendor = OnuMgmtTable[Onuentry].DeviceInfo.PonChipVendor;
	pMsg->DeviceInfo.PonChipType = OnuMgmtTable[Onuentry].DeviceInfo.PonChipType;
	/* E--added by liwei056@2011-2-23 for OnuPonChipType */
	VOS_MemCpy( pMsg->DeviceInfo.OUI,OnuMgmtTable[Onuentry].DeviceInfo.OUI, 3);

	/*add by shixh20101119*/
	#if 1  /*问题单11299*/
	pMsg->DeviceInfo.SysUptime = OnuMgmtTable[Onuentry].DeviceInfo.SysUptime;
	pMsg->DeviceInfo.RelativeUptime = OnuMgmtTable[Onuentry].DeviceInfo.RelativeUptime;
	VOS_MemCpy((char *) &pMsg->DeviceInfo.SysLaunchTime.year,(char *) &(OnuMgmtTable[Onuentry].DeviceInfo.SysLaunchTime.year), sizeof( Date_S));
	VOS_MemCpy((char *) &pMsg->DeviceInfo.SysOfflineTime.year,(char *) &(OnuMgmtTable[Onuentry].DeviceInfo.SysOfflineTime.year), sizeof( Date_S));
	#endif
    
    /*new added by luh 2012-5-10, 状态机控制变量*/
    /*pMsg->OnuhandlerStatus = OnuMgmtTable[Onuentry].OnuhandlerStatus;*/
    
	pMsg->PonType = OnuMgmtTable[Onuentry].PonType;
	pMsg->PonRate = OnuMgmtTable[Onuentry].PonRate;
	pMsg->OperStatus = OnuMgmtTable[Onuentry].OperStatus ;
	/*pMsg->TrafficServiceEnable=OnuMgmtTable[Onuentry].TrafficServiceEnable;*/

	pMsg->ExtOAM = OnuMgmtTable[Onuentry].ExtOAM;

	/*pMsg->SoftwareUpdateCtrl=OnuMgmtTable[Onuentry].SoftwareUpdateCtrl;
	pMsg->SoftwareUpdateStatus=OnuMgmtTable[Onuentry].SoftwareUpdateStatus;
	pMsg->SoftwareUpdateType=OnuMgmtTable[Onuentry].SoftwareUpdateType;*/

	pMsg->NeedDeleteFlag = OnuMgmtTable[Onuentry].NeedDeleteFlag;
    pMsg->OnuAbility = OnuMgmtTable[Onuentry].OnuAbility;
	pMsg->GrantNumber = OnuMgmtTable[Onuentry].GrantNumber;
	pMsg->RTT = OnuMgmtTable[Onuentry].RTT;   
	pMsg->AlarmStatus = OnuMgmtTable[Onuentry].AdminStatus;
	/*pMsg->AlarmMask=OnuMgmtTable[Onuentry].AlarmMask;
	pMsg->devAlarmMask=OnuMgmtTable[Onuentry].devAlarmMask;*/	/* 问题单11717 */

	pMsg->UsedFlag = OnuMgmtTable[Onuentry].UsedFlag;
	pMsg->RegisterFlag = OnuMgmtTable[Onuentry].RegisterFlag;
	pMsg->RegisterTrapFlag = OnuMgmtTable[Onuentry].RegisterTrapFlag;

	VOS_MemCpy( pMsg->SequenceNo,OnuMgmtTable[Onuentry].SequenceNo, PON_AUTHENTICATION_SEQUENCE_SIZE);
	pMsg->OAM_Ver = OnuMgmtTable[Onuentry].OAM_Ver;

	/*onu capability*/
	pMsg->Laser_ON = OnuMgmtTable[Onuentry].Laser_ON;
	pMsg->Laser_OFF = OnuMgmtTable[Onuentry].Laser_OFF;
	pMsg->AGC_Time = OnuMgmtTable[Onuentry].AGC_Time;
	pMsg->CDR_Time = OnuMgmtTable[Onuentry].CDR_Time;

	pMsg->onuLlid = OnuMgmtTable[Onuentry].LLID;
	pMsg->llidNum = OnuMgmtTable[Onuentry].llidNum;
	pMsg->llidMax = OnuMgmtTable[Onuentry].llidMax;
	pMsg->onuLlidRowStatus = OnuMgmtTable[Onuentry].LlidTable[0].EntryStatus;
#if 0	
/*LlidTable*/
	pMsg->LlidTable[0].EntryStatus=OnuMgmtTable[Onuentry].LlidTable[0].EntryStatus;
	pMsg->LlidTable[0].LlidType=OnuMgmtTable[Onuentry].LlidTable[0].LlidType;
	VOS_StrnCpy(pMsg->LlidTable[0].LlidDesc,OnuMgmtTable[Onuentry].LlidTable[0].LlidDesc,MAXLLIDDESCLEN);
	pMsg->LlidTable[0].LlidDescLen=OnuMgmtTable[Onuentry].LlidTable[0].LlidDescLen;
	pMsg->LlidTable[0].Llid=OnuMgmtTable[Onuentry].LlidTable[0].Llid;
	pMsg->LlidTable[0].MaxMAC=OnuMgmtTable[Onuentry].LlidTable[0].MaxMAC;
	
	pMsg->LlidTable[0].UplinkBandwidth_gr=OnuMgmtTable[Onuentry].LlidTable[0].UplinkBandwidth_gr;
	pMsg->LlidTable[0].UplinkBandwidth_be=OnuMgmtTable[Onuentry].LlidTable[0].UplinkBandwidth_be;
	pMsg->LlidTable[0].DownlinkBandwidth_gr=OnuMgmtTable[Onuentry].LlidTable[0].DownlinkBandwidth_gr;
	pMsg->LlidTable[0].DownlinkBandwidth_be=OnuMgmtTable[Onuentry].LlidTable[0].DownlinkBandwidth_be;

#ifdef  PLATO_DBA_V3
	pMsg->LlidTable[0].UplinkBandwidth_fixed=OnuMgmtTable[Onuentry].LlidTable[0].UplinkBandwidth_fixed;
#endif

	pMsg->LlidTable[0].UplinkClass=OnuMgmtTable[Onuentry].LlidTable[0].UplinkClass;
	pMsg->LlidTable[0].UplinkDelay=OnuMgmtTable[Onuentry].LlidTable[0].UplinkDelay;
	pMsg->LlidTable[0].DownlinkClass=OnuMgmtTable[Onuentry].LlidTable[0].DownlinkClass;
	pMsg->LlidTable[0].DownlinkDelay=OnuMgmtTable[Onuentry].LlidTable[0].DownlinkDelay;
	
	pMsg->LlidTable[0].ActiveUplinkBandwidth=OnuMgmtTable[Onuentry].LlidTable[0].ActiveUplinkBandwidth;
	pMsg->LlidTable[0].ActiveDownlinkBandwidth=OnuMgmtTable[Onuentry].LlidTable[0].ActiveDownlinkBandwidth;

	pMsg->LlidTable[0].ActiveUUplinkBandwidth_be=OnuMgmtTable[Onuentry].LlidTable[0].ActiveUUplinkBandwidth_be;

	pMsg->LlidTable[0].llidOltBoard=OnuMgmtTable[Onuentry].LlidTable[0].llidOltBoard;
	pMsg->LlidTable[0].llidOltPort=OnuMgmtTable[Onuentry].LlidTable[0].llidOltPort;
	pMsg->LlidTable[0].llidOnuBoard=OnuMgmtTable[Onuentry].LlidTable[0].llidOnuBoard;
	pMsg->LlidTable[0].llidOnuPort=OnuMgmtTable[Onuentry].LlidTable[0].llidOnuPort;

#ifdef	CTC_EXT_OID
	pMsg->LlidTable[0].llidCtcFecAbility=OnuMgmtTable[Onuentry].LlidTable[0].llidCtcFecAbility;
	pMsg->LlidTable[0].llidCtcFecMode=OnuMgmtTable[Onuentry].LlidTable[0].llidCtcFecMode;
	pMsg->LlidTable[0].llidCtcEncrypCtrl=OnuMgmtTable[Onuentry].LlidTable[0].llidCtcEncrypCtrl;
	pMsg->LlidTable[0].llidCtcDbaQuesetNum=OnuMgmtTable[Onuentry].LlidTable[0].llidCtcDbaQuesetNum;
	pMsg->LlidTable[0].llidCtcDbaQuesetCfgStatus=OnuMgmtTable[Onuentry].LlidTable[0].llidCtcDbaQuesetCfgStatus;
#endif
#endif
/*	OnuLLIDTable_S  LlidTable[MAXLLIDPERONU];*/
	
	/*pMsg->swFileLen=OnuMgmtTable[Onuentry].swFileLen;
	pMsg->transFileLen=OnuMgmtTable[Onuentry].transFileLen;
	pMsg->BurningFlash=OnuMgmtTable[Onuentry].BurningFlash;
	pMsg->BurningWait=OnuMgmtTable[Onuentry].BurningWait;*/
	
	/*VOS_MemCpy( pMsg->PeerToPeer,OnuMgmtTable[Onuentry].PeerToPeer, 8);
	pMsg->address_not_found_flag=OnuMgmtTable[Onuentry].address_not_found_flag;
	pMsg->broadcast_flag=OnuMgmtTable[Onuentry].broadcast_flag;*/

	pMsg->LastFerAlarmTime = OnuMgmtTable[Onuentry].LastFerAlarmTime;
	pMsg->LastBerAlarmTime = OnuMgmtTable[Onuentry].LastBerAlarmTime;

	VOS_MemCpy( pMsg->device_vendor_id,OnuMgmtTable[Onuentry].device_vendor_id, 4);
	pMsg->onu_model = OnuMgmtTable[Onuentry].onu_model;
	VOS_MemCpy( pMsg->chip_vendor_id,OnuMgmtTable[Onuentry].chip_vendor_id, CTC_VENDOR_ID_LENGTH);
	
	pMsg->chip_model = OnuMgmtTable[Onuentry].chip_model;
	pMsg->revision = OnuMgmtTable[Onuentry].revision;
	VOS_MemCpy( pMsg->date,OnuMgmtTable[Onuentry].date, 3);
	VOS_MemCpy( pMsg->hardware_version,OnuMgmtTable[Onuentry].hardware_version, 9);
	VOS_MemCpy( pMsg->software_version,OnuMgmtTable[Onuentry].software_version, 17);
	pMsg->firmware_version = OnuMgmtTable[Onuentry].firmware_version;
	VOS_MemCpy( pMsg->extendedModel,OnuMgmtTable[Onuentry].extendedModel, 17);
	
	pMsg->GE_supporting = OnuMgmtTable[Onuentry].GE_supporting;
	pMsg->FE_supporting = OnuMgmtTable[Onuentry].FE_supporting;
	pMsg->VoIP_supporting = OnuMgmtTable[Onuentry].VoIP_supporting;
	pMsg->TDM_CES_supporting = OnuMgmtTable[Onuentry].TDM_CES_supporting;

	pMsg->GE_Ethernet_ports_number = OnuMgmtTable[Onuentry].GE_Ethernet_ports_number;
	pMsg->FE_Ethernet_ports_number = OnuMgmtTable[Onuentry].FE_Ethernet_ports_number;
	pMsg->POTS_ports_number = OnuMgmtTable[Onuentry].POTS_ports_number;
	pMsg->E1_ports_number = OnuMgmtTable[Onuentry].E1_ports_number;

	pMsg->ADSL_ports_number = OnuMgmtTable[Onuentry].ADSL_ports_number;
	pMsg->VDSL_ports_number = OnuMgmtTable[Onuentry].VDSL_ports_number;
	pMsg->WLAN_ports_number = OnuMgmtTable[Onuentry].WLAN_ports_number;
	pMsg->USB_ports_number = OnuMgmtTable[Onuentry].USB_ports_number;
	pMsg->CATV_ports_number = OnuMgmtTable[Onuentry].CATV_ports_number;

	VOS_MemCpy( pMsg->Ports_distribution,OnuMgmtTable[Onuentry].Ports_distribution, 16);

	pMsg->Upstream_queues_number = OnuMgmtTable[Onuentry].Upstream_queues_number;
	pMsg->Max_upstream_queues_per_port = OnuMgmtTable[Onuentry].Max_upstream_queues_per_port;
	pMsg->Upstream_queues_allocation_increment = OnuMgmtTable[Onuentry].Upstream_queues_allocation_increment;
	pMsg->Downstream_queues_number = OnuMgmtTable[Onuentry].Downstream_queues_number;
	pMsg->Max_downstream_queues_per_port = OnuMgmtTable[Onuentry].Max_downstream_queues_per_port;

	pMsg->ctc_version = OnuMgmtTable[Onuentry].onu_ctc_version;
    
    /*for onu swap by jinhl@2013-04-27*/
	pMsg->ProtectType = OnuMgmtTable[Onuentry].ProtectType;

	/*pMsg->multicastSwitch=OnuMgmtTable[Onuentry].multicastSwitch;
	pMsg->fastleaveControl=OnuMgmtTable[Onuentry].fastleaveControl;
	pMsg->fastleaveAbility=OnuMgmtTable[Onuentry].fastleaveAbility;
	pMsg->stpEnable=OnuMgmtTable[Onuentry].stpEnable;
	pMsg->CTC_EncryptCtrl=OnuMgmtTable[Onuentry].CTC_EncryptCtrl;
	pMsg->CTC_EncryptStatus=OnuMgmtTable[Onuentry].CTC_EncryptStatus;
	pMsg->FEC_ability=OnuMgmtTable[Onuentry].FEC_ability;
	pMsg->FEC_Mode=OnuMgmtTable[Onuentry].FEC_Mode;*/

       /*pMsg->ONUMeteringTable = OnuMgmtTable[Onuentry].ONUMeteringTable;*/

	ONU_MGMT_SEM_GIVE;

	ret = CDP_Send( RPU_TID_CDP_ONU, dst_slotno,  RPU_TID_CDP_ONU,  CDP_MSG_TM_ASYNC, pMsg, msglen, MODULE_ONU );

	if( ret !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pMsg );
		VOS_ASSERT(0);
		return( RERROR );
	}	
	 
	return ROK;
}

static int send_onuSyncMessage_Deregister( ULONG to_slotno, short int PonPortIdx, short int OnuIdx )
{
	LONG ret;
	LONG msglen=sizeof(onu_sync_dereg_msg_t);
	onu_sync_dereg_msg_t  * pMsg = NULL;
	int Onuentry;
   
	/*CHECK_ONU_RANGE;

	if ( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return VOS_OK;*/

	pMsg = ( onu_sync_dereg_msg_t * ) CDP_AllocMsg( msglen, MODULE_ONU );              
	if(pMsg == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	/*将一部分管理信息从PON板上传到 主控板上*/
	pMsg->OnuSyncMsgHead.ponSlotIdx = GetCardIdxByPonChip(PonPortIdx);/*SYS_LOCAL_MODULE_SLOTNO;*/
	pMsg->OnuSyncMsgHead.portIdx = GetPonPortByPonChip(PonPortIdx);
	pMsg->OnuSyncMsgHead.onuIdx = OnuIdx;
	pMsg->OnuSyncMsgHead.onuEventId = ONU_EVENT_DEREGISTER;
	Onuentry = PonPortIdx*MAXONUPERPON+OnuIdx;
	
	ONU_MGMT_SEM_TAKE;
	/*pMsg->OnuhandlerStatus = OnuMgmtTable[Onuentry].OnuhandlerStatus;*/
	pMsg->OperStatus = OnuMgmtTable[Onuentry].OperStatus ;/*将ONU的操作状态传递给主控板*/
         /*问题单11299*/
       pMsg->SysUptime = OnuMgmtTable[Onuentry].DeviceInfo.SysUptime;
    	pMsg->RelativeUptime = OnuMgmtTable[Onuentry].DeviceInfo.RelativeUptime;
    	VOS_MemCpy((char *) &pMsg->SysLaunchTime, (char *)&(OnuMgmtTable[Onuentry].DeviceInfo.SysLaunchTime), sizeof( Date_S));
	/*不需要携带ONU管理表中改变的变量，可以在此消息到达主控板时，直接给这个ONU初始化默认值*/
    	VOS_MemCpy((char *) &pMsg->SysOfflineTime, (char *)&(OnuMgmtTable[Onuentry].DeviceInfo.SysOfflineTime), sizeof( Date_S));
	ONU_MGMT_SEM_GIVE;

	ret = CDP_Send( RPU_TID_CDP_ONU, to_slotno,  RPU_TID_CDP_ONU,  CDP_MSG_TM_ASYNC, pMsg, msglen,MODULE_ONU);

	if( ret !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pMsg );
		return( RERROR );
	}	
	return ROK;
}

/* added by xieshl 20111129, 支持pon-主控-备用主控之间ONU信息实时同步 */
static int relay_onuSyncMessage_Register( ULONG to_slotno, onu_sync_reg_msg_t  *pOnuSynData )
{
	LONG ret = VOS_ERROR;
	LONG msglen=sizeof(onu_sync_reg_msg_t);
	onu_sync_reg_msg_t  * pMsg = NULL;

	if( pOnuSynData == NULL )
		return ret;
	pMsg = ( onu_sync_reg_msg_t * ) CDP_AllocMsg( msglen, MODULE_ONU );              
	if(pMsg == NULL)
	{
		VOS_ASSERT( 0 );
		return ret;
	}
	VOS_MemCpy( pMsg, pOnuSynData, msglen );
	
	ret = CDP_Send( RPU_TID_CDP_ONU, to_slotno,  RPU_TID_CDP_ONU,  CDP_MSG_TM_ASYNC, pMsg, msglen,MODULE_ONU);
	if( ret !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pMsg );
	}	
	return ret;
}
static int relay_onuSyncMessage_Deregister( ULONG to_slotno, onu_sync_dereg_msg_t  *pOnuSynData )
{
	LONG ret = VOS_ERROR;
	LONG msglen=sizeof(onu_sync_dereg_msg_t);
	onu_sync_dereg_msg_t  * pMsg = NULL;

	if( pOnuSynData == NULL )
		return ret;
	pMsg = ( onu_sync_dereg_msg_t * ) CDP_AllocMsg( msglen, MODULE_ONU );              
	if(pMsg == NULL)
	{
		VOS_ASSERT( 0 );
		return ret;
	}
	VOS_MemCpy( pMsg, pOnuSynData, msglen );
	
	ret = CDP_Send( RPU_TID_CDP_ONU, to_slotno,  RPU_TID_CDP_ONU,  CDP_MSG_TM_ASYNC, pMsg, msglen,MODULE_ONU);
	if( ret !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pMsg );
	}	
	return ret;
}

LONG OnuMgtSyncDataSend_Register( short int PonPortIdx, short int OnuIdx )
{
	LONG ret = VOS_OK;
	ULONG to_slotno;
	CHECK_ONU_RANGE;
	
	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		to_slotno = device_standby_master_slotno_get();
		if( to_slotno == 0 )
			return ret;
	}
	else if ( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
	{
		return ret;
	}
	else
	{
		to_slotno = SYS_MASTER_ACTIVE_SLOTNO;
	}

	if( !(SYS_MODULE_IS_READY(to_slotno) || devsm_sys_is_switchhovering()) )
		return ret;

	ret = send_onuSyncMessage_Register( to_slotno, PonPortIdx, OnuIdx );
	
	if( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf(" onu%d/%d/%d register data sync to slot%d %s\r\n", GetCardIdxByPonChip(PonPortIdx), 
			GetPonPortByPonChip( PonPortIdx ), OnuIdx+1, to_slotno, (ret == VOS_OK ? "OK" : "ERR") );
	}
	return ret;
}

LONG OnuMgtSyncDataSend_Deregister( short int PonPortIdx, short int OnuIdx )
{
	LONG ret = VOS_OK;
	ULONG to_slotno;
	CHECK_ONU_RANGE;
	
	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		to_slotno = device_standby_master_slotno_get();
		if( to_slotno == 0 )
			return ret;
	}
	else if ( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
	{
		return ret;
	}
	else
	{
		to_slotno = SYS_MASTER_ACTIVE_SLOTNO;
	}

	ret = send_onuSyncMessage_Deregister( to_slotno, PonPortIdx, OnuIdx );
	
	if( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf(" onu%d/%d/%d deregister data sync to slot%d %s\r\n", GetCardIdxByPonChip(PonPortIdx), 
			GetPonPortByPonChip( PonPortIdx ), OnuIdx+1, to_slotno, (ret == VOS_OK ? "OK" : "ERR") );
	}
	return ret;
}

static int OnuMgtSyncDataHandler_Register(onu_sync_reg_msg_t  *pOnuSynData)
{
	short int  pon_slotno;
	short int  pon_portno;
	short int  PonPortIdx;
	short int  OnuIdx;
	short int  Onuentry;
	short int  IsNewOnu;
    OnuEventData_s data;

	if( pOnuSynData == NULL )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	if ( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
	{
		VOS_ASSERT(0);
		return VOS_OK;
	}
	pon_slotno = pOnuSynData->OnuSyncMsgHead.ponSlotIdx;
	pon_portno = pOnuSynData->OnuSyncMsgHead.portIdx;
	OnuIdx    = pOnuSynData->OnuSyncMsgHead.onuIdx;
	
	PonPortIdx = GetPonPortIdxBySlot(pon_slotno, pon_portno);
    
    VOS_MemZero(&data, sizeof(OnuEventData_s));
    data.PonPortIdx = PonPortIdx;
    data.OnuIdx = OnuIdx;
    data.llid = pOnuSynData->onuLlid;
    VOS_MemCpy(data.onu_mac, pOnuSynData->DeviceInfo.MacAddr, 6); 

	/*
	** 2010-11-30, modified by zhangxinhui
	** 应该判断ponPortIdx的返回值，他可能返回-1
	*/
	if( (!OLT_LOCAL_ISVALID(PonPortIdx)) || (!ONU_IDX_ISVALID(OnuIdx)) )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	
	Onuentry = PonPortIdx * MAXONUPERPON + OnuIdx;

	/*
	** 2010-11-30, modified by zhangxinhui
	** 应该判断Onuentry索引，虽然不一定错
	*/
	if (MAXONU <= Onuentry)
	{
		VOS_ASSERT(0);
		return RERROR;
	}

	ONU_MGMT_SEM_TAKE;

    /* B--added by liwei056@2011-3-11 for D11957 */
    if ( V2R1_ONU_NOT_EXIST == ThisIsValidOnu(PonPortIdx, OnuIdx) )
    {
        IsNewOnu = 1;
    }
    else
    {
        IsNewOnu = 0;
    }
    /* E--added by liwei056@2011-3-11 for D11957 */

	OnuMgmtTable[Onuentry].IsGponOnu = pOnuSynData->IsGponOnu;
	OnuMgmtTable[Onuentry].TcontNum = pOnuSynData->TcontNum;
	OnuMgmtTable[Onuentry].GEMPortNum = pOnuSynData->GEMPortNum;
	OnuMgmtTable[Onuentry].OmccVersion = pOnuSynData->OmccVersion;
	
	OnuMgmtTable[Onuentry].PmEnable = pOnuSynData->PmEnable;
	OnuMgmtTable[Onuentry].GemPortId = pOnuSynData->GemPortId;
	OnuMgmtTable[Onuentry].BerInterval = pOnuSynData->BerInterval;
	OnuMgmtTable[Onuentry].ImageValidIndex = pOnuSynData->ImageValidIndex;

	VOS_StrnCpy(OnuMgmtTable[Onuentry].DeviceInfo.equipmentID, pOnuSynData->DeviceInfo.equipmentID, MAXEQUIPMENTIDLEN);
	OnuMgmtTable[Onuentry].DeviceInfo.equipmentIDLen = pOnuSynData->DeviceInfo.equipmentIDLen;

	VOS_StrnCpy(OnuMgmtTable[Onuentry].DeviceInfo.DevicePassward, pOnuSynData->DeviceInfo.DevicePassward, MAXDEVICEPWDLEN);
	OnuMgmtTable[Onuentry].DeviceInfo.DevicePasswardLen = pOnuSynData->DeviceInfo.DevicePasswardLen;

	VOS_MemCpy(OnuMgmtTable[Onuentry].DeviceInfo.MacAddr, pOnuSynData->DeviceInfo.MacAddr,6);
	/*OnuMgmtTable[Onuentry].LlidTable[0].Llid=pOnuSynData->LlidTable[0].Llid;
	OnuMgmtTable[Onuentry].LLID = pOnuSynData->LlidTable[0].Llid;*/
	OnuMgmtTable[Onuentry].LlidTable[0].Llid=pOnuSynData->onuLlid;
	OnuMgmtTable[Onuentry].LLID = pOnuSynData->onuLlid;
	OnuMgmtTable[Onuentry].LlidTable[0].EntryStatus=pOnuSynData->onuLlidRowStatus;

    OnuMgmtTable[Onuentry].llidNum = pOnuSynData->llidNum;
    OnuMgmtTable[Onuentry].llidMax = pOnuSynData->llidMax;

	/*OnuMgmtTable[Onuentry].OperStatus=pOnuSynData->OperStatus;*/
	/*updateOnuOperStatusAndUpTime( Onuentry, pOnuSynData->OperStatus );*/	/* 问题单11835 */
	resetOnuOperStatusAndUpTime( Onuentry, pOnuSynData->OperStatus );

    /*new added by luh 2012-5-10, 状态机控制变量*/
    /*OnuMgmtTable[Onuentry].OnuhandlerStatus = pOnuSynData->OnuhandlerStatus;*/

#if 0	/* removed by xieshl 20110212, 放在ONU开始注册处理时，由PON板发起检查 */
	/*检查此ONU在别的PON口是否有绑定*/  /*add by shixh20100914*/
	if(PONid_ONUMAC_BINDING == V2R1_ENABLE)
	{
		/*if(OnuBindToOtherPon(ponPortIdx,pOnuSynData->DeviceInfo.MacAddr,&ret_PonPort)==TRUE)*/
		if(Is_Onu_Binding_with_Other_Pon(ponPortIdx,pOnuSynData->DeviceInfo.MacAddr,&ret_PonPort)==TRUE)
		{
			OnuMgt_DelBindingOnu(ponPortIdx, OnuIdx,PON_ONU_DEREGISTRATION_HOST_REQUEST, (int)ret_PonPort);/*去注册后，将此ONU放在PENDING-CONF队列中，之后不会在注册*/

			VOS_SysLog(LOG_TYPE_ONU, LOG_INFO, "Onu%d/%d/%d is bindded to another pon port.", ponSlotIdx, port, OnuIdx);
			/*AddPendingOnu_conf(ponPortIdx, onuIdx, pOnuSynData->DeviceInfo.MacAddr, ret_PonPort);*/
			
			/*发此ONU 去注册消息*/
				
			/*DelOnuFromPonPort(ponPortIdx, onuIdx);*//*删除，但还会在次注册*/
			return ( ROK );
		}
		/* zhangxinhui 2010-11-30
		else
			sys_console_printf("the onu not bind to other pon !\r\n");
		*/
	}
#endif

	#if 0
	if(OnuMgmtTable[Onuentry].UsedFlag != ONU_USED_FLAG)                     /*add by shixh 20101110 for mib get onu onuidx */
		setOnuStatus( ponSlotID, ponPortID, OnuIdx, ONU_ONLINE );
			
	if( pOnuSynData->RegisterFlag== ONU_FIRST_REGISTER_FLAG )
	{/* send onu first registration trap to NMS */
				Trap_OnuRegister( ponPortIdx, OnuIdx, ONU_FIRST_REGISTER_FLAG );
				
				OnuMgmtTable[Onuentry].RegisterFlag = NOT_ONU_FIRST_REGISTER_FLAG;
				OnuMgmtTable[Onuentry].UsedFlag = ONU_USED_FLAG;
	}
	else
	{
			/* send onu Re_registration trap to NMS */
			Trap_OnuRegister( ponPortIdx, OnuIdx, NOT_ONU_FIRST_REGISTER_FLAG );
	}
	#endif
	/*onu device information*/

	/* modified by xieshl 20111123, 防止结束符丢失，注意VOS_StrnCpy是不自动加结束符的 */
	OnuMgmtTable[Onuentry].DeviceInfo.VendorInfoLen = pOnuSynData->DeviceInfo.VendorInfoLen;
	if( OnuMgmtTable[Onuentry].DeviceInfo.VendorInfoLen >= MAXVENDORINFOLEN )
		OnuMgmtTable[Onuentry].DeviceInfo.VendorInfoLen = MAXVENDORINFOLEN;
	VOS_StrnCpy(OnuMgmtTable[Onuentry].DeviceInfo.VendorInfo, pOnuSynData->DeviceInfo.VendorInfo, MAXVENDORINFOLEN);
	OnuMgmtTable[Onuentry].DeviceInfo.VendorInfo[OnuMgmtTable[Onuentry].DeviceInfo.VendorInfoLen] = 0;

/*  comment by wangxiaoyu 2011-10-19 for cortina act as gwd onu for device name
	if(pOnuSynData->DeviceInfo.type >= V2R1_ONU_GT811 && 
		pOnuSynData->DeviceInfo.type < V2R1_ONU_MAX &&
		pOnuSynData->DeviceInfo.type != V2R1_ONU_CTC)*/
	{
	OnuMgmtTable[Onuentry].DeviceInfo.DeviceNameLen=pOnuSynData->DeviceInfo.DeviceNameLen;
	if( OnuMgmtTable[Onuentry].DeviceInfo.DeviceNameLen >= MAXDEVICENAMELEN )
		OnuMgmtTable[Onuentry].DeviceInfo.DeviceNameLen = MAXDEVICENAMELEN - 1;
	VOS_StrnCpy(OnuMgmtTable[Onuentry].DeviceInfo.DeviceName, pOnuSynData->DeviceInfo.DeviceName,MAXDEVICENAMELEN);
	OnuMgmtTable[Onuentry].DeviceInfo.DeviceName[OnuMgmtTable[Onuentry].DeviceInfo.DeviceNameLen] = 0;
	}
	
 	OnuMgmtTable[Onuentry].DeviceInfo.DeviceDescLen=pOnuSynData->DeviceInfo.DeviceDescLen;
	if( OnuMgmtTable[Onuentry].DeviceInfo.DeviceDescLen >= MAXDEVICEDESCLEN )
		OnuMgmtTable[Onuentry].DeviceInfo.DeviceDescLen = MAXDEVICEDESCLEN - 1;
	VOS_StrnCpy(OnuMgmtTable[Onuentry].DeviceInfo.DeviceDesc, pOnuSynData->DeviceInfo.DeviceDesc, MAXDEVICEDESCLEN );
	OnuMgmtTable[Onuentry].DeviceInfo.DeviceDesc[OnuMgmtTable[Onuentry].DeviceInfo.DeviceDescLen] = 0;
	
	OnuMgmtTable[Onuentry].DeviceInfo.LocationLen = pOnuSynData->DeviceInfo.LocationLen;
	if( OnuMgmtTable[Onuentry].DeviceInfo.LocationLen >= MAXLOCATIONLEN )
		OnuMgmtTable[Onuentry].DeviceInfo.LocationLen = MAXLOCATIONLEN;
	VOS_StrnCpy(OnuMgmtTable[Onuentry].DeviceInfo.Location, pOnuSynData->DeviceInfo.Location,MAXLOCATIONLEN);
	OnuMgmtTable[Onuentry].DeviceInfo.Location[OnuMgmtTable[Onuentry].DeviceInfo.LocationLen] = 0;
	
	VOS_MemCpy(OnuMgmtTable[Onuentry].DeviceInfo.MacAddr, pOnuSynData->DeviceInfo.MacAddr,6);
	
	OnuMgmtTable[Onuentry].DeviceInfo.SwVersionLen=pOnuSynData->DeviceInfo.SwVersionLen;
	if( OnuMgmtTable[Onuentry].DeviceInfo.SwVersionLen >= MAXSWVERSIONLEN )
		OnuMgmtTable[Onuentry].DeviceInfo.SwVersionLen = MAXSWVERSIONLEN -1;
	VOS_StrnCpy(OnuMgmtTable[Onuentry].DeviceInfo.SwVersion, pOnuSynData->DeviceInfo.SwVersion,MAXSWVERSIONLEN);
	OnuMgmtTable[Onuentry].DeviceInfo.SwVersion[OnuMgmtTable[Onuentry].DeviceInfo.SwVersionLen] = 0;
	VOS_StrnCpy(OnuMgmtTable[Onuentry].DeviceInfo.SwVersion1, pOnuSynData->DeviceInfo.SwVersion1,MAXSWVERSIONLEN);
	OnuMgmtTable[Onuentry].DeviceInfo.SwVersion1[OnuMgmtTable[Onuentry].DeviceInfo.SwVersionLen] = 0;
	
	OnuMgmtTable[Onuentry].DeviceInfo.HwVersionLen=pOnuSynData->DeviceInfo.HwVersionLen;
	if( OnuMgmtTable[Onuentry].DeviceInfo.HwVersionLen >= MAXHWVERSIONLEN )
		OnuMgmtTable[Onuentry].DeviceInfo.HwVersionLen = MAXHWVERSIONLEN - 1;
	VOS_StrnCpy(OnuMgmtTable[Onuentry].DeviceInfo.HwVersion, pOnuSynData->DeviceInfo.HwVersion,MAXHWVERSIONLEN);
	OnuMgmtTable[Onuentry].DeviceInfo.HwVersion[OnuMgmtTable[Onuentry].DeviceInfo.HwVersionLen] = 0;
	
	OnuMgmtTable[Onuentry].DeviceInfo.BootVersionLen=pOnuSynData->DeviceInfo.BootVersionLen;
	if( OnuMgmtTable[Onuentry].DeviceInfo.BootVersionLen >= MAXBOOTVERSIONLEN )
		OnuMgmtTable[Onuentry].DeviceInfo.BootVersionLen = MAXBOOTVERSIONLEN - 1;
	VOS_StrnCpy(OnuMgmtTable[Onuentry].DeviceInfo.BootVersion, pOnuSynData->DeviceInfo.BootVersion,MAXBOOTVERSIONLEN);
	OnuMgmtTable[Onuentry].DeviceInfo.BootVersion[OnuMgmtTable[Onuentry].DeviceInfo.BootVersionLen] = 0;
	
	OnuMgmtTable[Onuentry].DeviceInfo.FwVersionLen=pOnuSynData->DeviceInfo.FwVersionLen;
	if( OnuMgmtTable[Onuentry].DeviceInfo.FwVersionLen >= MAXFWVERSIONLEN )
		OnuMgmtTable[Onuentry].DeviceInfo.FwVersionLen = MAXFWVERSIONLEN -1;
	VOS_StrnCpy(OnuMgmtTable[Onuentry].DeviceInfo.FwVersion, pOnuSynData->DeviceInfo.FwVersion,MAXFWVERSIONLEN);
	OnuMgmtTable[Onuentry].DeviceInfo.FwVersion[OnuMgmtTable[Onuentry].DeviceInfo.FwVersionLen] = 0;
	
	OnuMgmtTable[Onuentry].DeviceInfo.DeviceSerial_NoLen=pOnuSynData->DeviceInfo.DeviceSerial_NoLen;
	if( OnuMgmtTable[Onuentry].DeviceInfo.DeviceSerial_NoLen >= MAXSNLEN )
		OnuMgmtTable[Onuentry].DeviceInfo.DeviceSerial_NoLen = MAXSNLEN - 1;
	VOS_StrnCpy(OnuMgmtTable[Onuentry].DeviceInfo.DeviceSerial_No, pOnuSynData->DeviceInfo.DeviceSerial_No,MAXSNLEN);
	OnuMgmtTable[Onuentry].DeviceInfo.DeviceSerial_No[OnuMgmtTable[Onuentry].DeviceInfo.DeviceSerial_NoLen] = 0;

	OnuMgmtTable[Onuentry].DeviceInfo.MadeInDateLen=pOnuSynData->DeviceInfo.MadeInDateLen;
	if( OnuMgmtTable[Onuentry].DeviceInfo.MadeInDateLen >= MAXDATELEN )
		OnuMgmtTable[Onuentry].DeviceInfo.MadeInDateLen = MAXDATELEN - 1;
	VOS_MemCpy(OnuMgmtTable[Onuentry].DeviceInfo.MadeInDate, pOnuSynData->DeviceInfo.MadeInDate,MAXDATELEN);
	OnuMgmtTable[Onuentry].DeviceInfo.MadeInDate[OnuMgmtTable[Onuentry].DeviceInfo.MadeInDateLen] = 0;

	OnuMgmtTable[Onuentry].DeviceInfo.AutoConfigFlag=pOnuSynData->DeviceInfo.AutoConfigFlag;

	OnuMgmtTable[Onuentry].DeviceInfo.type= pOnuSynData->DeviceInfo.type;
	/* B--added by liwei056@2011-2-23 for OnuPonChipType */
	OnuMgmtTable[Onuentry].DeviceInfo.PonChipVendor = pOnuSynData->DeviceInfo.PonChipVendor;
	OnuMgmtTable[Onuentry].DeviceInfo.PonChipType = pOnuSynData->DeviceInfo.PonChipType;
	/* E--added by liwei056@2011-2-23 for OnuPonChipType */
	VOS_MemCpy(OnuMgmtTable[Onuentry].DeviceInfo.OUI, pOnuSynData->DeviceInfo.OUI,3);

	/*add by shixh20101119*/ /*问题单11299*/
   	/*OnuMgmtTable[Onuentry].DeviceInfo.SysUptime=pOnuSynData->DeviceInfo.SysUptime;
    	OnuMgmtTable[Onuentry].DeviceInfo.RelativeUptime=pOnuSynData->DeviceInfo.RelativeUptime;*/	/* removed by xieshl 20110104, 这2项均是相对时间，用本板时间即可，防止热拔插等导致时间错乱，问题单11835 */
	VOS_MemCpy((char *) &(OnuMgmtTable[Onuentry].DeviceInfo.SysLaunchTime.year),(char *) &(pOnuSynData->DeviceInfo.SysLaunchTime.year), sizeof( Date_S));
	/*end*/
	VOS_MemCpy((char *) &(OnuMgmtTable[Onuentry].DeviceInfo.SysOfflineTime.year),(char *) &(pOnuSynData->DeviceInfo.SysOfflineTime.year), sizeof( Date_S));
	OnuMgmtTable[Onuentry].PonType=pOnuSynData->PonType;
	OnuMgmtTable[Onuentry].PonRate=pOnuSynData->PonRate;
	/*OnuMgmtTable[Onuentry].TrafficServiceEnable=pOnuSynData->TrafficServiceEnable;*/
	OnuMgmtTable[Onuentry].ExtOAM=pOnuSynData->ExtOAM;

	/*OnuMgmtTable[Onuentry].SoftwareUpdateCtrl=pOnuSynData->SoftwareUpdateCtrl;
	OnuMgmtTable[Onuentry].SoftwareUpdateStatus=pOnuSynData->SoftwareUpdateStatus;
	OnuMgmtTable[Onuentry].SoftwareUpdateType=pOnuSynData->SoftwareUpdateType;*/
		
	OnuMgmtTable[Onuentry].NeedDeleteFlag=pOnuSynData->NeedDeleteFlag;
    OnuMgmtTable[Onuentry].OnuAbility = pOnuSynData->OnuAbility;
    
	OnuMgmtTable[Onuentry].GrantNumber=pOnuSynData->GrantNumber;
	OnuMgmtTable[Onuentry].RTT=pOnuSynData->RTT;  
	OnuMgmtTable[Onuentry].AlarmStatus=pOnuSynData->AlarmStatus;
	/*OnuMgmtTable[Onuentry].AlarmMask=pOnuSynData->devAlarmMask;
	OnuMgmtTable[Onuentry].devAlarmMask=pOnuSynData->devAlarmMask;*/	/* removed by xieshl 20101227，问题单11717 */

	OnuMgmtTable[Onuentry].UsedFlag=pOnuSynData->UsedFlag;
	OnuMgmtTable[Onuentry].RegisterFlag=pOnuSynData->RegisterFlag;
	if( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
		OnuMgmtTable[Onuentry].RegisterTrapFlag=ONU_REGISTER_TRAP;	/* modified by xieshl 20120202, 备用主控不需同步，否则会导致倒换后trap不能上报，问题单14311 */
	else
		OnuMgmtTable[Onuentry].RegisterTrapFlag=pOnuSynData->RegisterTrapFlag;

	VOS_MemCpy(OnuMgmtTable[Onuentry].SequenceNo, pOnuSynData->SequenceNo, PON_AUTHENTICATION_SEQUENCE_SIZE);
	OnuMgmtTable[Onuentry].OAM_Ver=pOnuSynData->OAM_Ver;

	/*onu capability*/
	OnuMgmtTable[Onuentry].Laser_ON=pOnuSynData->Laser_ON;
	OnuMgmtTable[Onuentry].Laser_OFF=pOnuSynData->Laser_OFF;
	OnuMgmtTable[Onuentry].AGC_Time=pOnuSynData->AGC_Time;
	OnuMgmtTable[Onuentry].CDR_Time=pOnuSynData->CDR_Time;

    /* 从业务板无需上报ONU配置，只需上报最新状态 */
#if 0	
	/*LlidTable*/
	OnuMgmtTable[Onuentry].LlidTable[0].EntryStatus=pOnuSynData->LlidTable[0].EntryStatus;
	OnuMgmtTable[Onuentry].LlidTable[0].LlidType=pOnuSynData->LlidTable[0].LlidType;
	VOS_StrnCpy(OnuMgmtTable[Onuentry].LlidTable[0].LlidDesc, pOnuSynData->LlidTable[0].LlidDesc,MAXLLIDDESCLEN);
	OnuMgmtTable[Onuentry].LlidTable[0].LlidDescLen=pOnuSynData->LlidTable[0].LlidDescLen;
	OnuMgmtTable[Onuentry].LlidTable[0].MaxMAC=pOnuSynData->LlidTable[0].MaxMAC;
	
	OnuMgmtTable[Onuentry].LlidTable[0].UplinkBandwidth_gr=pOnuSynData->LlidTable[0].UplinkBandwidth_gr;
	OnuMgmtTable[Onuentry].LlidTable[0].UplinkBandwidth_be=pOnuSynData->LlidTable[0].UplinkBandwidth_be;
	OnuMgmtTable[Onuentry].LlidTable[0].DownlinkBandwidth_gr=pOnuSynData->LlidTable[0].DownlinkBandwidth_gr;
	OnuMgmtTable[Onuentry].LlidTable[0].DownlinkBandwidth_be=pOnuSynData->LlidTable[0].DownlinkBandwidth_be;

#ifdef  PLATO_DBA_V3
	OnuMgmtTable[Onuentry].LlidTable[0].UplinkBandwidth_fixed=pOnuSynData->LlidTable[0].UplinkBandwidth_fixed;
#endif
	OnuMgmtTable[Onuentry].LlidTable[0].UplinkClass=pOnuSynData->LlidTable[0].UplinkClass;
	OnuMgmtTable[Onuentry].LlidTable[0].UplinkDelay=pOnuSynData->LlidTable[0].UplinkDelay;
	OnuMgmtTable[Onuentry].LlidTable[0].DownlinkClass=pOnuSynData->LlidTable[0].DownlinkClass;
	OnuMgmtTable[Onuentry].LlidTable[0].DownlinkDelay=pOnuSynData->LlidTable[0].DownlinkDelay;
	
	OnuMgmtTable[Onuentry].LlidTable[0].ActiveUplinkBandwidth=pOnuSynData->LlidTable[0].ActiveUplinkBandwidth;
	OnuMgmtTable[Onuentry].LlidTable[0].ActiveDownlinkBandwidth=pOnuSynData->LlidTable[0].ActiveDownlinkBandwidth;

	OnuMgmtTable[Onuentry].LlidTable[0].ActiveUUplinkBandwidth_be=pOnuSynData->LlidTable[0].ActiveUUplinkBandwidth_be;

	OnuMgmtTable[Onuentry].LlidTable[0].llidOltBoard=pOnuSynData->LlidTable[0].llidOltBoard;
	OnuMgmtTable[Onuentry].LlidTable[0].llidOltPort=pOnuSynData->LlidTable[0].llidOltPort;
	OnuMgmtTable[Onuentry].LlidTable[0].llidOnuBoard=pOnuSynData->LlidTable[0].llidOnuBoard;
	OnuMgmtTable[Onuentry].LlidTable[0].llidOnuPort=pOnuSynData->LlidTable[0].llidOnuPort;

#ifdef	CTC_EXT_OID
	OnuMgmtTable[Onuentry].LlidTable[0].llidCtcFecAbility=pOnuSynData->LlidTable[0].llidCtcFecAbility;
	OnuMgmtTable[Onuentry].LlidTable[0].llidCtcFecMode=pOnuSynData->LlidTable[0].llidCtcFecMode;
	OnuMgmtTable[Onuentry].LlidTable[0].llidCtcEncrypCtrl=pOnuSynData->LlidTable[0].llidCtcEncrypCtrl;
	OnuMgmtTable[Onuentry].LlidTable[0].llidCtcDbaQuesetNum=pOnuSynData->LlidTable[0].llidCtcDbaQuesetNum;
	OnuMgmtTable[Onuentry].LlidTable[0].llidCtcDbaQuesetCfgStatus=pOnuSynData->LlidTable[0].llidCtcDbaQuesetCfgStatus;
#endif
#else
    /*  带宽状态同步 */
    /* modified by xieshl 20110915, 需求11112, ONU注册时优先取基于MAC地址的带宽配置，同时覆盖原基于ONU ID的配置 */
    {
    /* B--added by liwei056@2011-5-27 for D12903 */
        if ( ONU_OPER_STATUS_UP == pOnuSynData->OperStatus )
        /* E--added by liwei056@2011-5-27 for D12903 */
        /* B--added by liwei056@2011-2-18 for CodeTestBug */
        {
#if 0                
            OnuLLIDTable_S * pstLLIDAttr;
    
            pstLLIDAttr = &OnuMgmtTable[Onuentry].LlidTable[0];
            /* B--added by liwei056@2011-3-11 for D11957 */
            if ( IsNewOnu )
            {
                /* 新增ONU，需更新变化的默认配置 */
                pstLLIDAttr->UplinkBandwidth_gr = OnuConfigDefault.UplinkBandwidth;
                pstLLIDAttr->UplinkBandwidth_be = OnuConfigDefault.UplinkBandwidthBe;
                
                pstLLIDAttr->DownlinkBandwidth_gr = OnuConfigDefault.DownlinkBandwidth;
                pstLLIDAttr->DownlinkBandwidth_be = OnuConfigDefault.DownlinkBandwidthBe;
            }
            /* E--added by liwei056@2011-3-11 for D11957 */

	    /* modified by xieshl 20110915, 需求11112, ONU注册时优先取基于MAC地址的带宽配置，同时覆盖原基于ONU ID的配置 */
    	    if( assign_onuBwBasedMac_hookrtn )
    		(*assign_onuBwBasedMac_hookrtn)(Onuentry, pOnuSynData->DeviceInfo.MacAddr);
        
            pstLLIDAttr->ActiveUplinkBandwidth   = pstLLIDAttr->UplinkBandwidth_gr;
            pstLLIDAttr->ActiveDownlinkBandwidth = pstLLIDAttr->DownlinkBandwidth_gr;
        
#else
            OnuLLIDTable_S * pLlidCfg;
    
            pLlidCfg = &OnuMgmtTable[Onuentry].LlidTable[0];
            if(pLlidCfg->BandWidthIsDefault & OLT_CFG_DIR_UPLINK)
            {
				GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK,
						&pLlidCfg->UplinkBandwidth_gr, &pLlidCfg->UplinkBandwidth_be);
                pLlidCfg->UplinkClass = OnuConfigDefault.UplinkClass;
                pLlidCfg->UplinkDelay = OnuConfigDefault.UplinkDelay;
            }   
            if(pLlidCfg->BandWidthIsDefault & OLT_CFG_DIR_DOWNLINK)
            {
				GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK,
						&pLlidCfg->DownlinkBandwidth_gr, &pLlidCfg->DownlinkBandwidth_be);
                pLlidCfg->DownlinkClass = OnuConfigDefault.DownlinkClass;
                pLlidCfg->DownlinkDelay = OnuConfigDefault.UplinkDelay;
            }        
            
            OnuEvent_UpDate_RunningBandWidth(PonPortIdx, OnuIdx);
#if 1
            if(ONU_NOT_DEFAULT_MAX_MAC_FLAG & pLlidCfg->MaxMAC)
            {
                /*do nothing*/
            }
            else
            {
                pLlidCfg->MaxMAC = MaxMACDefault;
            }
#endif
#endif
        }
    }
    /* B--added by liwei056@2011-2-18 for CodeTestBug */
#endif	
	UpdateProvisionedBWInfo( PonPortIdx );	/* modified by xieshl 20120705, 问题单14190 */

	/*OnuMgmtTable[Onuentry].swFileLen=pOnuSynData->swFileLen;
	OnuMgmtTable[Onuentry].transFileLen=pOnuSynData->transFileLen;
	OnuMgmtTable[Onuentry].BurningFlash=pOnuSynData->BurningFlash;
	OnuMgmtTable[Onuentry].BurningWait=pOnuSynData->BurningWait;*/
	
	/*VOS_MemCpy( OnuMgmtTable[Onuentry].PeerToPeer, pOnuSynData->PeerToPeer,8);
	OnuMgmtTable[Onuentry].address_not_found_flag=pOnuSynData->address_not_found_flag;
	OnuMgmtTable[Onuentry].broadcast_flag=pOnuSynData->broadcast_flag;*/

	OnuMgmtTable[Onuentry].LastFerAlarmTime=pOnuSynData->LastFerAlarmTime;
	OnuMgmtTable[Onuentry].LastBerAlarmTime=pOnuSynData->LastBerAlarmTime;

	

	VOS_MemCpy(OnuMgmtTable[Onuentry].device_vendor_id, pOnuSynData->device_vendor_id, 4);
	OnuMgmtTable[Onuentry].onu_model=pOnuSynData->onu_model;
	VOS_MemCpy( OnuMgmtTable[Onuentry].chip_vendor_id, pOnuSynData->chip_vendor_id,CTC_VENDOR_ID_LENGTH);
	
	OnuMgmtTable[Onuentry].chip_model=pOnuSynData->chip_model;
	OnuMgmtTable[Onuentry].revision=pOnuSynData->revision;
	VOS_MemCpy(OnuMgmtTable[Onuentry].date,  pOnuSynData->date,3);
	VOS_StrnCpy( OnuMgmtTable[Onuentry].hardware_version, pOnuSynData->hardware_version,9);
	OnuMgmtTable[Onuentry].hardware_version[8] = 0;
	VOS_StrnCpy( OnuMgmtTable[Onuentry].software_version, pOnuSynData->software_version,17);
	OnuMgmtTable[Onuentry].software_version[16] = 0;
	OnuMgmtTable[Onuentry].firmware_version=pOnuSynData->firmware_version;
	VOS_MemCpy( OnuMgmtTable[Onuentry].extendedModel,pOnuSynData->extendedModel, 17);
	
	OnuMgmtTable[Onuentry].GE_supporting=pOnuSynData->GE_supporting;
	OnuMgmtTable[Onuentry].FE_supporting=pOnuSynData->FE_supporting;
	OnuMgmtTable[Onuentry].VoIP_supporting=pOnuSynData->VoIP_supporting;
	OnuMgmtTable[Onuentry].TDM_CES_supporting=pOnuSynData->TDM_CES_supporting;

	OnuMgmtTable[Onuentry].GE_Ethernet_ports_number = pOnuSynData->GE_Ethernet_ports_number;
	OnuMgmtTable[Onuentry].FE_Ethernet_ports_number = pOnuSynData->FE_Ethernet_ports_number;
	OnuMgmtTable[Onuentry].POTS_ports_number = pOnuSynData->POTS_ports_number;
	OnuMgmtTable[Onuentry].E1_ports_number = pOnuSynData->E1_ports_number;

	OnuMgmtTable[Onuentry].ADSL_ports_number=pOnuSynData->ADSL_ports_number;
	OnuMgmtTable[Onuentry].VDSL_ports_number=pOnuSynData->VDSL_ports_number;
	OnuMgmtTable[Onuentry].WLAN_ports_number=pOnuSynData->WLAN_ports_number;
	OnuMgmtTable[Onuentry].USB_ports_number=pOnuSynData->USB_ports_number;
	OnuMgmtTable[Onuentry].CATV_ports_number=pOnuSynData->CATV_ports_number;

	VOS_MemCpy( OnuMgmtTable[Onuentry].Ports_distribution, pOnuSynData->Ports_distribution,16);

	OnuMgmtTable[Onuentry].Upstream_queues_number=pOnuSynData->Upstream_queues_number;
	OnuMgmtTable[Onuentry].Max_upstream_queues_per_port=pOnuSynData->Max_upstream_queues_per_port;
	OnuMgmtTable[Onuentry].Upstream_queues_allocation_increment=pOnuSynData->Upstream_queues_allocation_increment;
	OnuMgmtTable[Onuentry].Downstream_queues_number=pOnuSynData->Downstream_queues_number;
	OnuMgmtTable[Onuentry].Max_downstream_queues_per_port=pOnuSynData->Max_downstream_queues_per_port;

	OnuMgmtTable[Onuentry].onu_ctc_version = pOnuSynData->ctc_version;
    
    /*for onu swap by jinhl@2013-04-27*/
	OnuMgmtTable[Onuentry].ProtectType = pOnuSynData->ProtectType;

	/*OnuMgmtTable[Onuentry].multicastSwitch=pOnuSynData->multicastSwitch;
	OnuMgmtTable[Onuentry].fastleaveControl=pOnuSynData->fastleaveControl;
	OnuMgmtTable[Onuentry].fastleaveAbility=pOnuSynData->fastleaveAbility;
	OnuMgmtTable[Onuentry].stpEnable=pOnuSynData->stpEnable;
	OnuMgmtTable[Onuentry].CTC_EncryptCtrl=pOnuSynData->CTC_EncryptCtrl;
	OnuMgmtTable[Onuentry].CTC_EncryptStatus=pOnuSynData->CTC_EncryptStatus;
	OnuMgmtTable[Onuentry].FEC_ability=pOnuSynData->FEC_ability;
	OnuMgmtTable[Onuentry].FEC_Mode=pOnuSynData->FEC_Mode;*/

	/*OnuMgmtTable[Onuentry].ONUMeteringTable = pOnuSynData->ONUMeteringTable;*/

	/* B--added by liwei056@2010-12-31 for D11817 */
	if( OnuMgmtTable[Onuentry].RTT > PonPortTable[PonPortIdx].CurrRTT )
		PonPortTable[PonPortIdx].CurrRTT = OnuMgmtTable[Onuentry].RTT;
    /* E--added by liwei056@2010-12-31 for D11817 */

	OnuMgmtTable[Onuentry].PowerOn = V2R1_POWER_ON;
	OnuMgmtTable[Onuentry].PowerOffCounter = 0;

	ONU_MGMT_SEM_GIVE;

	/* modified by xieshl 20111129, 支持pon-主控-备用主控之间ONU信息实时同步 */
	if( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf(" onu%d/%d/%d register data sync from slot%d\r\n", pon_slotno, pon_portno, OnuIdx+1, pon_slotno );
	}
    
    /*added  by luh 2012-9-14 注册回调函数，通知其他模块，ONU 注册成功*/
    handler_onuevent_callback(ONU_EVENT_CODE_REGISTER, data);

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		ULONG to_slotno;
		to_slotno = device_standby_master_slotno_get();
		if( to_slotno )
			relay_onuSyncMessage_Register( to_slotno, pOnuSynData );
	}
	
    return ( ROK );
}

static int OnuMgtSyncDataHandler_Deregister(onu_sync_dereg_msg_t  *pOnuSynData)
{
	short int   pon_slotno;
	short int   pon_portno; 
	short int   PonPortIdx;
	short int   OnuIdx;
	short int  onuEntry;
	unsigned int	def_up_bw_gr = 0;
	unsigned int	def_down_bw_gr = 0;
    OnuEventData_s data;
	if( pOnuSynData == NULL )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	if ( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return VOS_OK;

	pon_slotno = pOnuSynData->OnuSyncMsgHead.ponSlotIdx;
	pon_portno = pOnuSynData->OnuSyncMsgHead.portIdx;
	OnuIdx = pOnuSynData->OnuSyncMsgHead.onuIdx;

	PonPortIdx = GetPonPortIdxBySlot(pon_slotno, pon_portno);

	if( (!OLT_LOCAL_ISVALID(PonPortIdx)) || (!ONU_IDX_ISVALID(OnuIdx)) )
	{
		VOS_ASSERT(0);
		return RERROR;
	}

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    
    data.PonPortIdx = PonPortIdx;
    data.OnuIdx = OnuIdx;
    data.llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(data.onu_mac, OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, 6);        
	ONU_MGMT_SEM_GIVE;    
    handler_onuevent_callback(ONU_EVENT_CODE_DEREGISTER, data);

    /* B--added by liwei056@2010-12-31 for D11817 */
	if( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UP )
	{
		DecreasePonPortRunningData( PonPortIdx, OnuIdx);
	}
    /* E--added by liwei056@2010-12-31 for D11817 */

	/*add by shixh20100921*/
	/*DelPendingOnu( ponPortIdx, onuIdx );
	DelPendingOnu_conf(ponPortIdx, onuIdx);*/
	
	ONU_MGMT_SEM_TAKE;
	if( OnuMgmtTable[onuEntry].NeedDeleteFlag == TRUE )
	{

		GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &def_up_bw_gr, NULL);
		GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK, &def_down_bw_gr, NULL);
		PonPortTable[PonPortIdx].ProvisionedBW += OnuMgmtTable[onuEntry].LlidTable[0].UplinkBandwidth_gr - def_up_bw_gr; 
		PonPortTable[PonPortIdx].DownlinkProvisionedBW += OnuMgmtTable[onuEntry].LlidTable[0].DownlinkBandwidth_gr - def_down_bw_gr;
#ifdef PLATO_DBA_V3
		PonPortTable[PonPortIdx].ProvisionedBW += OnuMgmtTable[onuEntry].LlidTable[0].UplinkBandwidth_fixed;
#endif
		InitOneOnuByDefault(PonPortIdx, OnuIdx);		
	}
    else
    {
        /*OnuMgmtTable[Onuentry].OperStatus=pOnuSynData->OperStatus;*/
        updateOnuOperStatusAndUpTime( onuEntry, pOnuSynData->OperStatus );	/* 问题单11835 */
        OnuEvent_Set_RegisterStatus(PonPortIdx, OnuIdx, 0, pOnuSynData->OnuhandlerStatus);
        ClearOnuRunningData(PonPortIdx, OnuIdx, 0);
        
    	VOS_MemCpy((char *) &(OnuMgmtTable[onuEntry].DeviceInfo.SysLaunchTime.year),(char *) &(pOnuSynData->SysLaunchTime.year), sizeof( Date_S));
    	/*end*/
    	VOS_MemCpy((char *) &(OnuMgmtTable[onuEntry].DeviceInfo.SysOfflineTime.year),(char *) &(pOnuSynData->SysOfflineTime.year), sizeof( Date_S));
        
        OnuMgmtTable[onuEntry].RegisterFlag = NOT_ONU_FIRST_REGISTER_FLAG;

        /* 问题单11299 */
        /*OnuMgmtTable[Onuentry].DeviceInfo.SysUptime = pOnuSynData->SysUptime;
        	OnuMgmtTable[Onuentry].DeviceInfo.RelativeUptime = pOnuSynData->RelativeUptime;*/
        VOS_MemCpy((char *) &(OnuMgmtTable[onuEntry].DeviceInfo.SysLaunchTime),(char *) &(pOnuSynData->SysLaunchTime), sizeof( Date_S));
    }
	ONU_MGMT_SEM_GIVE;

	/* modified by xieshl 20111129, 支持pon-主控-备用主控之间ONU信息实时同步 */
	if( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf(" onu%d/%d/%d deregister data sync from slot%d\r\n", pon_slotno, pon_portno, OnuIdx+1, pon_slotno );
	}

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		ULONG to_slotno;
		to_slotno = device_standby_master_slotno_get();
		if( to_slotno )
			relay_onuSyncMessage_Deregister( to_slotno, pOnuSynData );
	}

    return  ROK;
}


/* added by xieshl 20110602,  自动删除MAC地址相同的离线ONU 使能开关同步到PON板 */
ULONG g_onu_repeated_del_enable = 0;
LONG OnuMgtSync_OnuRepeatedDelEnable( ULONG enable )
{
	LONG rc = VOS_OK;
	ULONG slotno;
	
	g_onu_repeated_del_enable = enable;

	if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		onu_sync_del_msg_t  * pMsg = NULL;
		LONG msglen=sizeof(onu_sync_del_msg_t);

		for( slotno=1; slotno<=SYS_CHASSIS_SWITCH_SLOTNUM; slotno++ )
		{		
		    /*modi by luh@2015-04-09. For Q.24181*/
			if( !SYS_MODULE_IS_CPU_PON(slotno) || (slotno == SYS_LOCAL_MODULE_SLOTNO) )
				continue;

			pMsg = ( onu_sync_del_msg_t * ) CDP_AllocMsg( msglen, MODULE_ONU );              
			if(pMsg == NULL)
			{
				VOS_ASSERT( 0 );
				return VOS_ERROR;
			}
			
			pMsg->OnuSyncMsgHead.onuEventId = ONU_EVENT_DEL_ENABLE;
			pMsg->OnuSyncMsgHead.ponSlotIdx = slotno;
			pMsg->OnuSyncMsgHead.portIdx = 0;
			pMsg->OnuSyncMsgHead.onuIdx = 0;
			pMsg->syncValue = enable;

			rc = CDP_Send( RPU_TID_CDP_ONU, slotno,  RPU_TID_CDP_ONU,  CDP_MSG_TM_ASYNC, pMsg, msglen, MODULE_ONU );
			if( rc !=  VOS_OK )
			{
				CDP_FreeMsg( (void *) pMsg );
			}
		}
	}
	return rc;
}

/* added by xieshl 20110727, 主控通知PON板删除ONU，问题单12871 */
extern LONG localDelOnuFromPon( short int pon_id, short int onu_idx );
LONG OnuMgtSync_OnuDeletedNotify( ULONG slotno, ULONG portno, SHORT OnuIdx )
{
	LONG rc = VOS_OK;
	SHORT PonPortIdx;

	if( g_onu_repeated_del_enable == 0 )
		return VOS_OK;

	if( SYS_SLOTNO_IS_ILLEGAL(slotno) /*|| !SYS_MODULE_IS_PON(slotno)*/ )	/* modified by xieshl 20111118, 只要查到有重复ONU就删，这个槽位不一定非得插PON板 */
		return VOS_ERROR;
	
	PonPortIdx = GetPonPortIdxBySlot( slotno, portno );
	if( localDelOnuFromPon(PonPortIdx, OnuIdx) == VOS_ERROR )
		return VOS_ERROR;

	if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		onu_sync_del_msg_t  * pMsg = NULL;
		LONG msglen=sizeof(onu_sync_del_msg_t);

		pMsg = ( onu_sync_del_msg_t * ) CDP_AllocMsg( msglen, MODULE_ONU );              
		if(pMsg == NULL)
		{
			VOS_ASSERT( 0 );
			return VOS_ERROR;
		}
		
		pMsg->OnuSyncMsgHead.onuEventId = ONU_EVENT_REPEATED_DEL;
		pMsg->OnuSyncMsgHead.ponSlotIdx = slotno;
		pMsg->OnuSyncMsgHead.portIdx = portno;
		pMsg->OnuSyncMsgHead.onuIdx = OnuIdx;
		pMsg->syncValue = 0;

		rc = CDP_Send( RPU_TID_CDP_ONU, slotno,  RPU_TID_CDP_ONU,  CDP_MSG_TM_ASYNC, pMsg, msglen, MODULE_ONU );
		if( rc !=  VOS_OK )
		{
			CDP_FreeMsg( (void *) pMsg );
		}	
	}
	
	return rc;
}

/* modified by xieshl 20120319, 增加向备用主控板同步，应使用最大的PON口数，问题单14775 */
LONG onu_status_debug_flag = 0;
#define onu_status_debug(x)	if(onu_status_debug_flag) sys_console_printf x
extern LONG device_standby_master_slotno_get();
ULONG OnuMgtSync_Module_PonPortNum( ULONG slotno )
{
	ULONG num = 0;

    if ( PRODUCT_IS_H_Series(SYS_PRODUCT_TYPE) )
    {
		if( (slotno >= 1) && (slotno <= SYS_CHASSIS_SWITCH_SLOTNUM) )
		{
			/*if( (MODULE_TYPE_NULL != __SYS_MODULE_TYPE__(slotno)) && 
				(MODULE_TYPE_UNKNOW != __SYS_MODULE_TYPE__(slotno) &&
				(MODULE_E_GFA6900_SW != __SYS_MODULE_TYPE__(slotno)) )
				num = SYS_MODULE_SLOT_PORT_NUM(slotno);*/
			if ( (MODULE_E_GFA6900_SW != __SYS_MODULE_TYPE__(slotno)) && (MODULE_E_GFA8000_SW != __SYS_MODULE_TYPE__(slotno)) )
				num = CARD_MAX_PON_PORTNUM;
		}
    }
    else
    {
    	switch( SYS_PRODUCT_TYPE )
    	{
    		case PRODUCT_E_GFA6100:
    			if( (slotno == 2) || (slotno == 3) )
    				num = 2;	/*SYS_MODULE_SLOT_PORT_NUM(slotno);*/
    			break;
    		case PRODUCT_E_EPON3:
    			if( (slotno >= 4) || (slotno <= 8)  )
    				num = SYS_MODULE_PORT_NUM(MODULE_E_GFA_EPON);
    			break;
    		default:
    			break;
    	}
    }

	return num;
}

/* added by xieshl 20110728, 检查主控和PON板间ONU状态，由主控定时上报ONU状态，PON板收到后检查是否和本板数据一致，
    不一致时则向主控同步ONU数据，但需要避免在ONU注册或离线过程中发生 */
LONG OnuMgtSync_OnuStatusReport()
{
	LONG rc = VOS_OK;
	ULONG slotno, portno, standby_master_slotno = 0;
	ULONG sync_pon_num;
	LONG ponid, onuid;
	int onuStatus/*, ponStatus*/;
	int onuEntry;

	LONG msglen=sizeof(onu_sync_status_msg_t);
	onu_sync_status_msg_t *pMsg = NULL;
	onu_sync_status_msg_t *pToMasterMsg = NULL;

	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER/*SYS_LOCAL_MODULE_ISMASTERACTIVE*/)
		return rc;

	/*standby_master_slotno = device_standby_master_slotno_get();*/
	/* 主控定时向各PON板/备用主控板发送ONU 状态 */
	for( slotno=1; slotno<=SYS_CHASSIS_SWITCH_SLOTNUM; slotno++ )
	{
		if( (slotno == SYS_LOCAL_MODULE_SLOTNO))
			continue;
		if( (!SYS_MODULE_IS_CPU_PON(slotno)) && SYS_LOCAL_MODULE_ISMASTERACTIVE)
			continue;

		sync_pon_num = OnuMgtSync_Module_PonPortNum(slotno);	/* modified by xieshl 20120417, 直接跳过没有PON口的板卡，问题单14880/14890 */
		if( sync_pon_num == 0 )
			continue;
		
		pMsg = ( onu_sync_status_msg_t * ) CDP_AllocMsg( msglen, MODULE_ONU );              
		if(pMsg == NULL)
		{
			VOS_ASSERT( 0 );
			return VOS_ERROR;
		}
		
		VOS_MemZero( pMsg, msglen );
		pMsg->OnuSyncMsgHead.onuEventId = ONU_EVENT_STATUS_REPORT;
		pMsg->OnuSyncMsgHead.ponSlotIdx = slotno;
		/*pMsg->OnuSyncMsgHead.portIdx = 0;
		pMsg->OnuSyncMsgHead.onuIdx = 0;*/

		/*if( SYS_MODULE_IS_RUNNING(slotno) )*/
		{
			for( portno=1; portno<=sync_pon_num; portno++ )
			{
				ponid = GetPonPortIdxBySlot( slotno, portno );
				if( ponid == RERROR )
					continue;
				/*if( !PonPortIsWorking(ponid) )
					continue;*/
				if( ponid >= MAXPON )
				{
					VOS_ASSERT(0);
					CDP_FreeMsg( (void *) pMsg );
					return VOS_ERROR;
				}
				
				onu_status_debug( ("\r\n %% sync pon %d/%d onu status:\r\n", slotno, portno) );
				
				for( onuid=0; onuid<MAXONUPERPON; onuid++ )
				{
					onuEntry = ponid*MAXONUPERPON+onuid;

                    if(ThisIsGponOnu(ponid, onuid))
                    {
                        onuStatus = ((ONU_OPER_STATUS_UP == OnuMgmtTable[onuEntry].OperStatus) ? ONU_OPER_STATUS_UP : ONU_OPER_STATUS_DOWN);                    
                    }
                    else
                    {
                        ONU_MGMT_SEM_TAKE;
                        if( !MAC_ADDR_IS_INVALID(OnuMgmtTable[onuEntry].DeviceInfo.MacAddr) )
                        {
                            onuStatus = ((ONU_OPER_STATUS_UP == OnuMgmtTable[onuEntry].OperStatus) ? ONU_OPER_STATUS_UP : ONU_OPER_STATUS_DOWN);
                        }
                        else
                            onuStatus = 0;
                        ONU_MGMT_SEM_GIVE;                    
                    }

					pMsg->onuStatusList[portno-1][onuid] = onuStatus;

					onu_status_debug( (" %d", pMsg->onuStatusList[portno-1][onuid]) );
				}
				onu_status_debug( ("\r\n") );
			}
		}
		
		if( /*SYS_MODULE_IS_6900_EPON(slotno)*/SYS_LOCAL_MODULE_ISMASTERACTIVE )
		{
			/*if( standby_master_slotno )
			{
				pToMasterMsg = ( onu_sync_status_msg_t * ) CDP_AllocMsg( msglen, MODULE_ONU );              
				if( pToMasterMsg )
				{
					VOS_MemCpy( pToMasterMsg, pMsg, msglen );
				}
			}*/
			if(SYS_MODULE_SLOT_ISHAVECPU(slotno))
			{
    			rc = CDP_Send( RPU_TID_CDP_ONU, slotno,  RPU_TID_CDP_ONU,  CDP_MSG_TM_ASYNC, pMsg, msglen, MODULE_ONU );
    			if( rc !=  VOS_OK )
    			{
    				CDP_FreeMsg( (void *) pMsg );
    			}
			}
            else
			{
				CDP_FreeMsg( (void *) pMsg );
			}
		}
		else
		{
			rc = CDP_Send( RPU_TID_CDP_ONU, SYS_MASTER_ACTIVE_SLOTNO,  RPU_TID_CDP_ONU,  CDP_MSG_TM_ASYNC, pMsg, msglen, MODULE_ONU );
			if( rc !=  VOS_OK )
			{
				CDP_FreeMsg( (void *) pMsg );
			}
            
			/*if( standby_master_slotno )
				pToMasterMsg = pMsg;*/
		}
	}
	return rc;
}

LONG OnuMgtSync_OnuStatusHandle( onu_sync_status_msg_t * pOnuStatusMsg )
{
	#define ONU_STATUS_SYNC_DIFF_NUM		3	/* 状态连续不一致的最大次数 */
	static UCHAR *pOnuStatusDiffCounter = NULL;	/* ONU状态不一致计数，超过ONU_STATUS_SYNC_DIFF_NUM 次后则重新向主控发送不一致的ONU数据 */
	
	LONG rc = VOS_OK;
	ULONG slotno, portno, sync_pon_num = 0;
	LONG ponid, onuid;
	int onuStatus, ponStatus;
	int onuEntry;

	/*if( devsm_sys_is_switchhovering() )
		return VOS_OK;*/
	
	if( pOnuStatusMsg == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	slotno = pOnuStatusMsg->OnuSyncMsgHead.ponSlotIdx;

	if( SYS_MODULE_IS_PON(SYS_LOCAL_MODULE_SLOTNO) )
	{
		if( slotno != SYS_LOCAL_MODULE_SLOTNO )
			return VOS_ERROR;
		if(!SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
			return VOS_OK;

		if( pOnuStatusDiffCounter == NULL )
		{
			ULONG size = CARD_MAX_PON_PORTNUM * MAXONUPERPON * sizeof(UCHAR);
			pOnuStatusDiffCounter = VOS_Malloc( size, MODULE_ONU );
			if( pOnuStatusDiffCounter )
				VOS_MemZero( pOnuStatusDiffCounter, size );
			else
			{
				VOS_ASSERT(0);
				return VOS_ERROR;
			}
		}

		/* 比较PON板本地保存的ONU状态，是否和主控发过来的状态一致 */
		sync_pon_num = OnuMgtSync_Module_PonPortNum(slotno);
		for( portno=1; portno<=sync_pon_num; portno++ )
		{
			ponid = GetPonPortIdxBySlot( slotno, portno );
			if( ponid == RERROR )
				continue;
			if( ponid > CARD_MAX_PON_PORTNUM )
			{
				VOS_ASSERT(0);
				return VOS_ERROR;
			}
			ponStatus = PonPortIsWorking(ponid);

			onu_status_debug( ("\r\n %% sync pon %d/%d onu status:", slotno, portno) );
			
			for( onuid=0; onuid<MAXONUPERPON; onuid++ )
			{
				if( (onuid & 0x1f) == 0 )
				{
					onu_status_debug( ("\r\n") );
				}

				onuEntry = ponid*MAXONUPERPON+onuid;

				/* 取本PON板上的ONU状态 */
				if( ponStatus )
				{
				    if(ThisIsGponOnu(ponid, onuid))
				    {
                        ONU_MGMT_SEM_TAKE;
                        onuStatus = ((ONU_OPER_STATUS_UP == OnuMgmtTable[onuEntry].OperStatus) ? ONU_OPER_STATUS_UP : ONU_OPER_STATUS_DOWN);
                        ONU_MGMT_SEM_GIVE;
				    }
				    else
				    {
    					ONU_MGMT_SEM_TAKE;
    					if( MAC_ADDR_IS_INVALID(OnuMgmtTable[onuEntry].DeviceInfo.MacAddr) )
    						onuStatus = 0;
    					else
    					{
    						onuStatus = ((ONU_OPER_STATUS_UP == OnuMgmtTable[onuEntry].OperStatus) ? ONU_OPER_STATUS_UP : ONU_OPER_STATUS_DOWN);
    					}
    					ONU_MGMT_SEM_GIVE;
					}
				}
				else
					onuStatus = 0;
				
				onu_status_debug( (" %d,%d", onuStatus, pOnuStatusMsg->onuStatusList[portno-1][onuid]) );

				/* 如果和主控上的ONU 状态不一致 */
				if( pOnuStatusMsg->onuStatusList[portno-1][onuid] != onuStatus )
				{
					if( (onuStatus == 0) || (onuStatus == ONU_OPER_STATUS_DOWN) )
					{
						/* 如果ONU 在主控上UP，而在PON板上为DOWN或者在PON板上不存在，应通知主控状态为DOWN */
						if( pOnuStatusMsg->onuStatusList[portno-1][onuid] == ONU_OPER_STATUS_UP )
						{
							pOnuStatusDiffCounter[onuEntry]++;
							if( pOnuStatusDiffCounter[onuEntry] > ONU_STATUS_SYNC_DIFF_NUM )
							{
								OnuMgtSyncDataSend_Deregister( ponid, onuid );
                                if(!SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
    								onu_status_debug( (" %% onu %d/%d/%d re-sync DEREG-status to master!\r\n", slotno, portno, onuid+1 ));
								pOnuStatusDiffCounter[onuEntry] = 0;
							}
							else
							{
                                if(!SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
    								onu_status_debug( (" %% find onu %d/%d/%d REG-status different(%d)\r\n", slotno, portno, onuid+1, pOnuStatusDiffCounter[onuEntry] ));
							}
						}
						else
						{
							/* 如果ONU 在主控上DOWN，而在PON板上不存在，不做操作 */
							pOnuStatusDiffCounter[onuEntry] = 0;
						}
					}
					else if( onuStatus == ONU_OPER_STATUS_UP )
					{
						/* 如果ONU在PON板上UP，而在主控板上不存在，则重新同步到主控 */
						pOnuStatusDiffCounter[onuEntry]++;
						if( pOnuStatusDiffCounter[onuEntry] > ONU_STATUS_SYNC_DIFF_NUM )
						{
							OnuMgtSyncDataSend_Register( ponid, onuid );
							onu_status_debug( (" %% onu %d/%d/%d re-sync REG-status to master!\r\n", slotno, portno, onuid+1) );
							pOnuStatusDiffCounter[onuEntry] = 0;
						}
						else
							onu_status_debug( (" %% find onu %d/%d/%d REG-status different(%d)\r\n", slotno, portno, onuid+1, pOnuStatusDiffCounter[onuEntry] ));
					}
					else
					{
						/*localDelOnuFromPon( ponid, onuid );*/
						pOnuStatusDiffCounter[onuEntry] = 0;
					}
				}
				else
					pOnuStatusDiffCounter[onuEntry] = 0;
			}
			onu_status_debug( ("\r\n") );
		}
	}
	else if( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
	{
#if 0        
		sync_pon_num = OnuMgtSync_Module_PonPortNum(slotno);
		for( portno=1; portno<=sync_pon_num; portno++ )
		{
			ponid = GetPonPortIdxBySlot( slotno, portno );
			if( ponid == RERROR )
				continue;
			if( ponid >= MAXPON )
			{
				onu_status_debug( ("\r\n %% sync pon %d/%d make ponid=%d err\r\n", slotno, portno, ponid) );
				return VOS_ERROR;
			}
			
			onu_status_debug( ("\r\n %% sync pon %d/%d onu status:\r\n", slotno, portno) );
			
			for( onuid=0; onuid<MAXONUPERPON; onuid++ )
			{
				onuEntry = ponid*MAXONUPERPON+onuid;
				onuStatus = pOnuStatusMsg->onuStatusList[portno-1][onuid];

				ONU_MGMT_SEM_TAKE;
				if( onuStatus != OnuMgmtTable[onuEntry].OperStatus )
				{
					if( onuStatus == 0 )	/* 主控上无效的ONU，备用上直接清除 */
					{
						VOS_MemSet( OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, 0xff, 6 );
						OnuMgmtTable[onuEntry].OperStatus = onuStatus;
					}
					else
					{
						if( !MAC_ADDR_IS_INVALID(OnuMgmtTable[onuEntry].DeviceInfo.MacAddr) )
						{
							/* 主控上有效的ONU，备用上直接同步状态 */
							if( onuStatus != ONU_OPER_STATUS_UP )
								onuStatus = ONU_OPER_STATUS_DOWN;
							OnuMgmtTable[onuEntry].OperStatus = onuStatus;
						}
						else	
						{
							/* 主控上有效的ONU，但备用上不存在，主控应重新同步ONU注册信息 */
							/* todo */
							OnuMgmtTable[onuEntry].OperStatus = 0;
						}
					}
				}
				ONU_MGMT_SEM_GIVE;

				onu_status_debug( (" %d", onuStatus) );
			}
			onu_status_debug( ("\r\n") );
		}
#endif        
	}
	else if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
		if( pOnuStatusDiffCounter == NULL )
		{
			ULONG size = MAXPON * MAXONUPERPON * sizeof(UCHAR);
			pOnuStatusDiffCounter = VOS_Malloc( size, MODULE_ONU );
			if( pOnuStatusDiffCounter )
				VOS_MemZero( pOnuStatusDiffCounter, size );
			else
			{
				VOS_ASSERT(0);
				return VOS_ERROR;
			}
		}

		/* 比较PON板本地保存的ONU状态，是否和主控发过来的状态一致 */
		sync_pon_num = OnuMgtSync_Module_PonPortNum(slotno);
		for( portno=1; portno<=sync_pon_num; portno++ )
		{
			ponid = GetPonPortIdxBySlot( slotno, portno );
			if( ponid == RERROR )
				continue;
            
			ponStatus = PonPortIsWorking(ponid);
			onu_status_debug( ("\r\n %% sync pon %d/%d onu status:", slotno, portno) );
			
			for( onuid=0; onuid<MAXONUPERPON; onuid++ )
			{
				if( (onuid & 0x1f) == 0 )
				{
					onu_status_debug( ("\r\n") );
				}

				onuEntry = ponid*MAXONUPERPON+onuid;

				/* 取本PON板上的ONU状态 */
				if( ponStatus )
				{
				    if(ThisIsGponOnu(ponid, onuid))
				    {
                        ONU_MGMT_SEM_TAKE;
                        onuStatus = ((ONU_OPER_STATUS_UP == OnuMgmtTable[onuEntry].OperStatus) ? ONU_OPER_STATUS_UP : ONU_OPER_STATUS_DOWN);
                        ONU_MGMT_SEM_GIVE;
				    }
                    else
                    {
    					ONU_MGMT_SEM_TAKE;
    					if( MAC_ADDR_IS_INVALID(OnuMgmtTable[onuEntry].DeviceInfo.MacAddr) )
    						onuStatus = 0;
    					else
    					{
    						onuStatus = ((ONU_OPER_STATUS_UP == OnuMgmtTable[onuEntry].OperStatus) ? ONU_OPER_STATUS_UP : ONU_OPER_STATUS_DOWN);
    					}
    					ONU_MGMT_SEM_GIVE;
					}
				}
				else
					onuStatus = 0;
				
				onu_status_debug( (" %d,%d", onuStatus, pOnuStatusMsg->onuStatusList[portno-1][onuid]) );

				/* 如果和主控上的ONU 状态不一致 */
				if( pOnuStatusMsg->onuStatusList[portno-1][onuid] != onuStatus )
				{
					if( (onuStatus == 0) || (onuStatus == ONU_OPER_STATUS_DOWN) )
					{
						/* 如果ONU 在主控上UP，而在PON板上为DOWN或者在PON板上不存在，应通知主控状态为DOWN */
						if( pOnuStatusMsg->onuStatusList[portno-1][onuid] == ONU_OPER_STATUS_UP )
						{
							pOnuStatusDiffCounter[onuEntry]++;
							if( pOnuStatusDiffCounter[onuEntry] > ONU_STATUS_SYNC_DIFF_NUM )
							{
								OnuMgtSyncDataSend_Deregister( ponid, onuid );
								pOnuStatusDiffCounter[onuEntry] = 0;
							}
						}
						else
						{
							/* 如果ONU 在主控上DOWN，而在PON板上不存在，不做操作 */
							pOnuStatusDiffCounter[onuEntry] = 0;
						}
					}
					else if( onuStatus == ONU_OPER_STATUS_UP )
					{
						/* 如果ONU在PON板上UP，而在主控板上不存在，则重新同步到主控 */
						pOnuStatusDiffCounter[onuEntry]++;
						if( pOnuStatusDiffCounter[onuEntry] > ONU_STATUS_SYNC_DIFF_NUM )
						{
							OnuMgtSyncDataSend_Register( ponid, onuid );
							pOnuStatusDiffCounter[onuEntry] = 0;
						}
					}
					else
					{
						/*localDelOnuFromPon( ponid, onuid );*/
						pOnuStatusDiffCounter[onuEntry] = 0;
					}
				}
				else
					pOnuStatusDiffCounter[onuEntry] = 0;
			}
			onu_status_debug( ("\r\n") );
		}
	}
	return rc;
}

extern LONG OnuMgtSync_OnuBwBasedMacProc( VOID *pMsg );
int recv_onuSyncMessage(onu_sync_msg_head_t  *pOnuSynData)
{
	if( pOnuSynData == NULL )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	switch( pOnuSynData->onuEventId )
	{
		case ONU_EVENT_REGISTER:
			OnuMgtSyncDataHandler_Register( (onu_sync_reg_msg_t *)pOnuSynData);
			break;
		case  ONU_EVENT_DEREGISTER:
			OnuMgtSyncDataHandler_Deregister( (onu_sync_dereg_msg_t *)pOnuSynData);
			break;
		case ONU_EVENT_DEL_ENABLE:
			OnuMgtSync_OnuRepeatedDelEnable( ((onu_sync_del_msg_t *)pOnuSynData)->syncValue );
			break;
		case ONU_EVENT_REPEATED_DEL:
			OnuMgtSync_OnuDeletedNotify( pOnuSynData->ponSlotIdx, pOnuSynData->portIdx, pOnuSynData->onuIdx );
			break;
		case ONU_EVENT_STATUS_REPORT:
			OnuMgtSync_OnuStatusHandle( (onu_sync_status_msg_t *)pOnuSynData );
			break;
		case ONU_EVENT_BW_BASED_MAC:
			OnuMgtSync_OnuBwBasedMacProc( (VOID*)pOnuSynData );
			break;
		case ONU_EVENT_EXT_MGT_SYNC:
			OnuExtMgmt_SyncRecv_Callback( (VOID*)pOnuSynData );
			break;
		case ONU_EVENT_EXT_MGT_REQ:
			OnuExtMgmt_SyncReqRecv_Callback( (VOID*)pOnuSynData );
			break;
		default:
			break;	
	}
	CDP_FreeMsg((void*)pOnuSynData);
	return ROK;
}

LONG resumeOnuRepeatedDelEnable()
{
	return OnuMgtSync_OnuRepeatedDelEnable( g_onu_repeated_del_enable );
}


/*****************************************************
 *
 *    Function:  sendOnuDeregistrationMsg()
 *
 *    Param:    
 *                 
 *    Desc:   ONU Deregister event callback function;send a msg to pon handler task
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
/*****************************************************
 *
 *    Function:  Onu_ShowDown( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:    
 *                 
 *    Desc:   show down the specific onu
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int Onu_ShutDown( short int PonPortIdx, short int OnuIdx )
{
   
	short int PonChipType/*, PonChipVer = RERROR*/;
	
	short int onu_id;
	short int OnuCurrStatus;

	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if(onu_id  == INVALID_LLID ) return( RERROR );
	
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
   
	if( onu_id != INVALID_LLID ){	
		/*PonChipType = PONCHIP_PAS;*/
		#if 1
		if(OLT_PONCHIP_ISPAS(PonChipType))
		#else
		if( PonChipType == PONCHIP_PAS )
		#endif
			{
			if( GetOnuOperStatus_1(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_DOWN )
			/*if( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].OperStatus != ONU_OPER_STATUS_DOWN )*/
				{
				#if 1
				OnuCurrStatus = OLT_GetOnuMode(PonPortIdx, onu_id );
				if( OnuCurrStatus != PON_ONU_MODE_OFF )
					OLT_DeregisterLLID( PonPortIdx,  onu_id, TRUE );
				#else
				OnuCurrStatus = PAS_get_onu_mode(PonPortIdx, onu_id );
				if( OnuCurrStatus != PON_ONU_MODE_OFF )
					PAS_deregister_onu( PonPortIdx,  onu_id, TRUE ); /* PAS_shoutdown_onu(); modified for PAS-SOFT V5.1*/
				#endif
					/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
				}
			}

		/* other pon chip type handler */
		else { 

			}
		}

	/*OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].OperStatus = ONU_OPER_STATUS_DOWN;*/
	
	return( ROK );

}

/*****************************************************
 *
 *    Function: DeregisterAllOnu(short int PonPortIdx)
 *
 *    Param:   short int PonPortIdx -- the specific pon port
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
 int Onu_deregister( short int PonPortIdx, short int OnuIdx )
{

	/*short int PonChipType;
	short int onu_id;
	short int OnuCurrStatus;
	int OperStatus;*/

	CHECK_ONU_RANGE;

#if 1
    /* add by shixh20100823 */
    OnuMgt_DeregisterOnu(PonPortIdx, OnuIdx);
#else
    PonChipType = GetPonChipTypeByPonPort(  PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
	{
		/*PonChipVer = PonChipType;*/
		PonChipType = PONCHIP_PAS;
	}
	
	onu_id = GetLlidByOnuIdx(PonPortIdx,  OnuIdx);
	if(onu_id == INVALID_LLID ) return( RERROR );
	OperStatus = GetOnuOperStatus_1( PonPortIdx, OnuIdx );

	if( onu_id != INVALID_LLID ){
		if(  OperStatus != ONU_OPER_STATUS_DOWN ){ 
			if( PonChipType == PONCHIP_PAS ){			
				OnuCurrStatus = PAS_get_onu_mode( PonPortIdx, onu_id );
				if( OnuCurrStatus != PON_ONU_MODE_OFF )
					PAS_deregister_onu( PonPortIdx, onu_id, TRUE );
#if 0				
				/* send onu deregister msg to NMS */
				if(OperStatus==ONU_OPER_STATUS_UP)
					{
					Trap_OnuDeregister( PonPortIdx, OnuIdx );
					DecreasePonPortRunningData(PonPortIdx, OnuIdx );
					}
				
				/* send onu alarm clear msg to NMS */

					
				ClearOnuRunningData(PonPortIdx, OnuIdx, 0);
#endif				
				}

			/* other pon chip type handle */
			else{
				}
			}
	
		else {

			}
		}
	
	/*OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].OperStatus = ONU_OPER_STATUS_DOWN;*/
#endif

	return( ROK );
}

 int  DeregisterAllOnu(short int PonPortIdx)
{
	short int i;

	for( i=0; i< MAXONUPERPON; i++){

		Onu_deregister( PonPortIdx,  i );
		}

	return( ROK );
}

 int ResetOnu( short int PonPortIdx, short int OnuIdx )
{
    
	short int PonChipType;
	
	short int onuId;
	short int OnuCurrStatus;
	
	CHECK_ONU_RANGE

	
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
		
	onuId = GetLlidByOnuIdx(PonPortIdx,  OnuIdx);

	if( onuId == INVALID_LLID ) return( RERROR );
	
	
	if(OLT_PONCHIP_ISPAS(PonChipType))
	{
		
		if(OLT_PONCHIP_ISPAS5001(PonChipType) ||
			( GetOnuVendorType( PonPortIdx, OnuIdx ) == ONU_VENDOR_GW ))
			{
			if( GetOnuOperStatus_1(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_DOWN )
			{
				#if 1	
				OnuCurrStatus = OLT_GetOnuMode( PonPortIdx, onuId );
				#else
				OnuCurrStatus = PAS_get_onu_mode( PonPortIdx, onuId );
				#endif
				if( OnuCurrStatus != PON_ONU_MODE_OFF )
				{
				    #if 1
					OnuMgt_ResetOnu( PonPortIdx, OnuIdx );
					#else
					GW10G_REMOTE_PASONU_reset_device( PonPortIdx, onuId, PON_RESET_SW );
					#endif
					/* Onu Mgmt table data may be handle in the callback function */
					return( ROK );
				}
				/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
				else{
					return( RERROR );
					}
			}
			else return (RERROR );
			}
		
		else if( GetOnuVendorType( PonPortIdx, OnuIdx ) == ONU_VENDOR_CT )
			return( CTC_ResetONU( PonPortIdx, OnuIdx ));
		}
	
	else { /* other pon chip handler */

		}

	return( RERROR );
}


 /*****************************************************
 *
 *    Function: SetOnuEncryption( unsigned char PonPortIdx, unsigned char OnuIdx, unsigned int cryptionDirection  )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                short int OnuIdx -- the specific Onu
 *                  unsigned int cryptionDirection  -- encryption direction: down or all
 *    Desc:   ONU加密相关函数，有启动、停止、更新密钥、
 *
 *    Return:    
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 int V2R1_OnuStartEncrypt( short int PonPortIdx, short int OnuIdx )
{
	ULONG aulMsg[4] = { MODULE_OLT, FC_V2R1_START_ENCRYPT, 0, 0};
	ULONG enable;
	CHECK_ONU_RANGE;

	ONU_MGMT_SEM_TAKE;
	enable = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].EncryptEnable;
	ONU_MGMT_SEM_GIVE;
	if( enable == V2R1_ENABLE ) 
	{
		aulMsg[2] = PonPortIdx;
		aulMsg[3] = OnuIdx;
		
	/*if( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].EncryptEnable == V2R1_ENABLE ) */
		if( VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			VOS_ASSERT(0);
			/*sys_console_printf("  error: VOS send message err\r\n"  );*/
		}
	}
	return( ROK);
}
 
int GetOnuEncryptStatus( short int PonPortIdx, short int OnuIdx, unsigned int *Status)
{
	CHECK_ONU_RANGE

	if( Status == NULL ) return( RERROR );
	ONU_MGMT_SEM_TAKE;
	*Status = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].EncryptStatus ;
	ONU_MGMT_SEM_GIVE;
	
	return( ROK );
}

static int  StartOnuEncryption( short int PonPortIdx,  short int OnuIdx, unsigned int cryptionDirection )
{
	short int onu_id/*, Llid*/;
	/*short int PonChipType, PonChipVer;*/
	short int ret;
	short int counter=0;
	PON_llid_parameters_t  llid_parameters;
	UCHAR old_enc_status, old_enc_dir;
	int OnuEntry;
	
	/* parameter check */
	CHECK_ONU_RANGE

	/*OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].EncryptKeyCounter=0;*/

	/* 判断要启动的加密(方向) 是否与PMC 提供的加密能力兼容*/
	if(( cryptionDirection != PON_ENCRYPTION_DIRECTION_DOWN ) && ( cryptionDirection != PON_ENCRYPTION_DIRECTION_ALL)) return( RERROR );

	/* 判断ONU是否已经启动加密；
		1) 若没启动加密，则继续向下执行；
		2) 若启动加密，则判断已启动的加密是否与将要启动的加密方向是否一致；
			若一致，则返回成功；
			若不一致，则先停止当前加密
	*/

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	OnuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;
#if 0
	if(onu_id != INVALID_LLID)
		{
		llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION;
		PAS_get_llid_parameters(PonPortIdx, onu_id, &llid_parameters);
		if((llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_DOWNLINK) || (llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_UPLINK_AND_DOWNLINK))
			PAS_stop_encryption( PonPortIdx, onu_id );
		OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].EncryptStatus == V2R1_NOTSTARTED;
		}
#endif
	ONU_MGMT_SEM_TAKE;
	old_enc_status = OnuMgmtTable[OnuEntry].EncryptStatus;
	old_enc_dir = OnuMgmtTable[OnuEntry].EncryptDirection;
	ONU_MGMT_SEM_GIVE;
	
	if( old_enc_status == V2R1_STARTED )
	{
		if( old_enc_dir == cryptionDirection )
		{
			/* modified by xieshl 20110318, 问题单12334，PON芯片上的实际加密状态和ONU表中的加密状态存在不一致的情况，
			    特别是ONU刚注册时，容易产生加密自动停止的情况，怀疑跟ONU密钥交互失败有关，因此软件记录
			    的状态是不可靠的，上层会定时检查，不一致时会重新配置，因此这里不能返回 */
			/*return( ROK );*/	
		}
		else
		{
			StopOnuEncrypt( PonPortIdx, OnuIdx);
			VOS_TaskDelay(EncryptKeyDelayTime ); /*延时0.5秒*/
		}
	}

#if 0
	/* deleted by chenfj 2006-11-16
	#3196问题单:加密去使能命令出错；保存错误*/
	/*
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].EncryptDirection = cryptionDirection;
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].EncryptEnable = V2R1_ENABLE;
	*/
	/*
	result = GetPonPortOperStatus(  PonPortIdx );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||(result == PONPORT_DEL)||( result == RERROR )) 
	*/
	if(PonPortIsWorking(PonPortIdx) != TRUE )
	{
		/* add by chenfj 2006-11-16 #3196问题单*/
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[OnuEntry].EncryptDirection = cryptionDirection;
		OnuMgmtTable[OnuEntry].EncryptEnable = V2R1_ENABLE;
		ONU_MGMT_SEM_GIVE;
		return( ROK );
	}

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
	{
		/* add by chenfj 2006-11-16 #3196问题单*/
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[OnuEntry].EncryptDirection = cryptionDirection;
		OnuMgmtTable[OnuEntry].EncryptEnable = V2R1_ENABLE;
		ONU_MGMT_SEM_GIVE;
		return( ROK );
	}
	
	if( onu_id == INVALID_LLID )
	{
		if(EVENT_ENCRYPT == V2R1_ENABLE)
			sys_console_printf("\r\n  %s/port%d onu%d not registered(StartOnuEncryption()) \r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
		return( ROK );
	}

	/* add by chenfj 2006-11-16 #3196问题单*/
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[OnuEntry].EncryptDirection = cryptionDirection;
	OnuMgmtTable[OnuEntry].EncryptEnable = V2R1_ENABLE;
	ONU_MGMT_SEM_GIVE;

	/*Llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx );*/
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        ) 
	{
		/*PonChipVer = PonChipType;*/
		PonChipType = PONCHIP_PAS;
	}
	
	if( PonChipType == PONCHIP_PAS )
	{
		/*
		onuMode = PAS_get_onu_mode(PonPortIdx, onu_id );
		if(  onuMode != PON_ONU_MODE_ON )
			{
			if(EVENT_ENCRYPT == V2R1_ENABLE)
				sys_console_printf("  %s/port%d onu %d current status:%s\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], (GetPonPortByPonChip(PonPortIdx) +1), (OnuIdx+1), OnuActivatedMode[onuMode-PON_ONU_MODE_ON] );
			return( RERROR );
			}
		*/

		if(( onu_id > 0 )||( onu_id < 127 ))
		{
			/* add by chenfj 2006-11-17 
			问题单3176.PON节点下，有时执行加密使能，去使能会导致ONU离线*/
#if 0
			if( VOS_SemTake(OnuEncryptSemId, WAIT_FOREVER ) ==  VOS_ERROR ) 
				{
				sys_console_printf( "get SemId error(StartOnuEncryption)\r\n" );
				return ( RERROR );
				}
#endif		
			/*if( OnuMgmtTable[OnuEntry].EncryptStatus == V2R1_STARTED )
				{
				PAS_stop_encryption( PonPortIdx, onu_id );
				OnuMgmtTable[OnuEntry].EncryptStatus = V2R1_NOTSTARTED;
				VOS_TaskDelay(EncryptKeyDelayTime);
				}
			*/
			/*
			VOS_MemSet( PON_encryption_key, 0,  PON_ENCRYPTION_KEY_SIZE );
			*(int *) PON_encryption_key = onu_id;
			PAS_update_encryption_key(PonPortIdx, onu_id, PON_encryption_key, PON_ENCRYPTION_UPDATE_PASSAVE );
			
			VOS_TaskDelay(EncryptKeyDelayTime);
			*/
#if 0
#ifndef  PON_ENCRYPTION_HANDLER
			VOS_SemGive(OnuEncryptSemId);
#endif
#endif
			/* add by chenfj 2006-11-17 
			问题单3176.PON节点下，有时执行加密使能，去使能会导致ONU离线*/
#if 0
			if( VOS_SemTake(OnuEncryptSemId, WAIT_FOREVER ) ==  VOS_ERROR ) 
				{
				sys_console_printf( "get SemId error(StartOnuEncryption)\r\n" );
				return ( RERROR );
				}
#endif		
			ONU_MGMT_SEM_TAKE;
			if(OnuMgmtTable[OnuEntry].EncryptFirstTime == V2R1_ENABLE)
			{
				OnuMgmtTable[OnuEntry].EncryptFirstTime = V2R1_DISABLE;
				ONU_MGMT_SEM_GIVE;
			}
			else
			{
				ONU_MGMT_SEM_GIVE;
				UpdateOnuEncryptKey_1(PonPortIdx, OnuIdx);
				VOS_TaskDelay(EncryptKeyDelayTime);
			}
			
			do{
#if 0
				ret = PAS_start_encryption( PonPortIdx, onu_id, (PON_encryption_direction_t)( cryptionDirection -PON_ENCRYPTION_PURE) );
#else
                ret = OnuMgt_StartEncryption(PonPortIdx, OnuIdx, &cryptionDirection);
#endif                        
				if( ret != OLT_ERR_OK /*PAS_EXIT_OK */)
				{
					VOS_TaskDelay(EncryptKeyDelayTime);
					llid_parameters.encryption_mode = PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION;
					/*PAS_get_llid_parameters(PonPortIdx, onu_id, &llid_parameters);*/
                     OnuMgt_GetLLIDParams(PonPortIdx, OnuIdx, &llid_parameters);/*add by shixh20101216*/
					if((llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_DOWNLINK) || (llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_UPLINK_AND_DOWNLINK))
						ret = PAS_EXIT_OK;
					/*sys_console_printf(" %s/port%d onu%d start encrypt err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), ret );*/
					VOS_TaskDelay(EncryptKeyDelayTime);
				}
				counter++;
			}while(( counter <=3) &&( ret != PAS_EXIT_OK ));
#if 0
#ifndef  PON_ENCRYPTION_HANDLER
				VOS_SemGive(OnuEncryptSemId);
#endif

			if( ret  != PAS_EXIT_OK ) 
			{
				sys_console_printf(" %s/port%d onu%d start encrypt err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), ret );
				return ( RERROR );
			}
#endif
		}
	}
	else
    { /* other pon chip handler */
	}
	
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[OnuEntry].EncryptStatus = V2R1_STARTED;
	ONU_MGMT_SEM_GIVE;
#else
	if(OnuMgmtTable[OnuEntry].EncryptFirstTime == V2R1_ENABLE)
	{
		OnuMgmtTable[OnuEntry].EncryptFirstTime = V2R1_DISABLE;
	}
	else
    {
		UpdateOnuEncryptKey_1(PonPortIdx, OnuIdx);
		VOS_TaskDelay(EncryptKeyDelayTime);
	}

	do{
        ret = OnuMgt_StartEncryption(PonPortIdx, OnuIdx, &cryptionDirection);
		if( ret != 0 )
		{
			VOS_TaskDelay(EncryptKeyDelayTime);
			llid_parameters.encryption_mode = PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION;
            OnuMgt_GetLLIDParams(PonPortIdx, OnuIdx, &llid_parameters);/*add by shixh20101216*/
			if((llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_DOWNLINK) || (llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_UPLINK_AND_DOWNLINK))
				ret = PAS_EXIT_OK;
			/*sys_console_printf(" %s/port%d onu%d start encrypt err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), ret );*/
			VOS_TaskDelay(EncryptKeyDelayTime);
		}
		counter++;
	}while((counter <= 3) &&( ret != PAS_EXIT_OK ));
#endif

	/* start the encrypt timer , 下行加密，由OLT负责更新密钥及密钥的产生
	if( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].EncryptDirection == ENCRYPT_DOWN ){
		StartEncryptTimer( PonPortIdx, OnuIdx );*/

	VOS_TaskDelay(EncryptKeyDelayTime);
	
	return( ROK);

}

static int StopOnuEncrypt( short int PonPortIdx, short int OnuIdx )
{
	int result;
	/*short int onu_id;
	short int Llid;*/
	/*short int PonChipType, PonChipVer;*/
	PON_llid_parameters_t  llid_parameters;
	/*short int onuMode;*/
	/*int OnuEntry;*/
	
	CHECK_ONU_RANGE

#if 0
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx; 
    
	/* deleted by chenfj 2006-11-16
	#3196问题单:加密去使能命令出错；保存错误*/
	/*
	OnuMgmtTable[OnuEntry].EncryptDirection = PON_ENCRYPTION_PURE;
	OnuMgmtTable[OnuEntry].EncryptEnable = V2R1_DISABLE;
	*/
	ONU_MGMT_SEM_TAKE;
	if( OnuMgmtTable[OnuEntry].EncryptStatus != V2R1_STARTED ) 
	{
		/* add by chenfj 2006-11-16 #3196问题单*/
		OnuMgmtTable[OnuEntry].EncryptDirection = PON_ENCRYPTION_PURE;
		OnuMgmtTable[OnuEntry].EncryptKeyTime = OnuConfigDefault.EncryptKeyTime;
		OnuMgmtTable[OnuEntry].EncryptEnable = V2R1_DISABLE;
		ONU_MGMT_SEM_GIVE;
		return( ROK );
	}
	ONU_MGMT_SEM_GIVE;

	/* stop the encrypt timer 
	StopEncryptTimer( PonPortIdx, OnuIdx ); 
	
	result = GetPonPortOperStatus(  PonPortIdx );
	*/
	
	if(PonPortIsWorking(PonPortIdx) != TRUE )
	{
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[OnuEntry].EncryptStatus = V2R1_NOTSTARTED;
		/* add by chenfj 2006-11-16 #3196问题单*/
		OnuMgmtTable[OnuEntry].EncryptDirection = PON_ENCRYPTION_PURE;
		OnuMgmtTable[OnuEntry].EncryptKeyTime = OnuConfigDefault.EncryptKeyTime;
		OnuMgmtTable[OnuEntry].EncryptEnable = V2R1_DISABLE;
		ONU_MGMT_SEM_GIVE;
		return( ROK );
	}
	
	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
	{
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[OnuEntry].EncryptStatus = V2R1_NOTSTARTED;
		/* add by chenfj 2006-11-16 #3196问题单*/
		OnuMgmtTable[OnuEntry].EncryptDirection = PON_ENCRYPTION_PURE;
		OnuMgmtTable[OnuEntry].EncryptKeyTime = OnuConfigDefault.EncryptKeyTime;
		OnuMgmtTable[OnuEntry].EncryptEnable = V2R1_DISABLE;
		ONU_MGMT_SEM_GIVE;
		return( ROK );	
	}
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID )
	{
		if(EVENT_ENCRYPT == V2R1_ENABLE)
			sys_console_printf("\r\n  %s/port%d onu %d is not registered(StopOnuEncryption()) \r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));		
		return( RERROR );
	}

	/*Llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx );*/

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        ) 
	{
		/*PonChipVer = PonChipType;*/
		PonChipType = PONCHIP_PAS;
	}
	
	if( PonChipType == PONCHIP_PAS )
	{
		/*
		onuMode = PAS_get_onu_mode(PonPortIdx, onu_id );
		if(  onuMode != PON_ONU_MODE_ON )
			{
			if(EVENT_ENCRYPT == V2R1_ENABLE)
				sys_console_printf("  %s/port%d onu%d is current status %s(StopOnuEncryption)\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), OnuActivatedMode[onuMode-PON_ONU_MODE_ON] );
			return( RERROR );
			}
		*/
		if(( onu_id > 0 )||( onu_id < 127 ))
		{	
			/* add by chenfj 2006-11-17 
			问题单3176.PON节点下，有时执行加密使能，去使能会导致ONU离线*/
#if 0
			if( VOS_SemTake(OnuEncryptSemId, WAIT_FOREVER) ==  VOS_ERROR ) 
			{
				sys_console_printf( "get SemId error(StopOnuEncrypt)\r\n" );
				return ( RERROR );
			}
#endif				
/*			
			llid_parameters.encryption_mode = PON_ENCRYPTION_DIRECTION_NO_ENCRYPTOIN;			
			PAS_get_llid_parameters(PonPortIdx, onu_id, &llid_parameters);

			if((llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_DOWNLINK) || (llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_UPLINK_AND_DOWNLINK))
				{
				VOS_TaskDelay(EncryptKeyDelayTime);
				result = PAS_stop_encryption( PonPortIdx, onu_id );
				}
*/
			/*
			if(OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].EncryptKeyCounter % 2 != 0 )
				{
				UpdateOnuEncryptKey_1(PonPortIdx, OnuIdx);
				VOS_TaskDelay(EncryptKeyDelayTime);
				}
			*/
		#if 0	
			result = PAS_stop_encryption( PonPortIdx, onu_id );
			result = PAS_EXIT_OK;
            #endif

                        result=OnuMgt_StopEncryption(PonPortIdx, OnuIdx);
#if 0			
#ifndef  PON_ENCRYPTION_HANDLER
			VOS_SemGive(OnuEncryptSemId);
#endif
#endif
			if( result != 0 )
			{
				VOS_TaskDelay(EncryptKeyDelayTime);
				llid_parameters.encryption_mode = PON_ENCRYPTION_DIRECTION_NO_ENCRYPTOIN;			
				/*PAS_get_llid_parameters(PonPortIdx, onu_id, &llid_parameters);*/
                OnuMgt_GetLLIDParams(PonPortIdx, OnuIdx, &llid_parameters);/*add by shixh20101216*/
				if((llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_DOWNLINK) || (llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_UPLINK_AND_DOWNLINK))
				{
					VOS_TaskDelay(EncryptKeyDelayTime);
					/*PAS_stop_encryption( PonPortIdx, onu_id );*/
                    OnuMgt_StopEncryption(PonPortIdx, OnuIdx);
				}
				/*sys_console_printf(" %s/port%d onu%d stop encrypt err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), result );
				return ( RERROR );*/
			}

		}
	
	}
	else
    { /* other pon chip handler */
	}
	
	/* add by chenfj 2006-11-16 #3196问题单*/
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[OnuEntry].EncryptDirection = PON_ENCRYPTION_PURE;
	OnuMgmtTable[OnuEntry].EncryptEnable = V2R1_DISABLE; 
	/*OnuMgmtTable[OnuEntry].EncryptKeyTime = OnuConfigDefault.EncryptKeyTime;*/
	OnuMgmtTable[OnuEntry].EncryptStatus = V2R1_NOTSTARTED;
	/*OnuMgmtTable[OnuEntry].EncryptKeyCounter = 0;*/
	ONU_MGMT_SEM_GIVE;
#else
    result = OnuMgt_StopEncryption(PonPortIdx, OnuIdx);
    if( result != 0 )
    {
        VOS_TaskDelay(EncryptKeyDelayTime);
        llid_parameters.encryption_mode = PON_ENCRYPTION_DIRECTION_NO_ENCRYPTOIN;			
        /*PAS_get_llid_parameters(PonPortIdx, onu_id, &llid_parameters);*/
        OnuMgt_GetLLIDParams(PonPortIdx, OnuIdx, &llid_parameters);/*add by shixh20101216*/
        if((llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_DOWNLINK) || (llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_UPLINK_AND_DOWNLINK))
        {
            VOS_TaskDelay(EncryptKeyDelayTime);
            /*PAS_stop_encryption( PonPortIdx, onu_id );*/
            OnuMgt_StopEncryption(PonPortIdx, OnuIdx);
        }
        
        /* sys_console_printf(" pon%d/%d onu%d stop encrypt err %d\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), result); */
        /* return ( RERROR ); */
    }
#endif

	VOS_TaskDelay(EncryptKeyDelayTime);
	
	return( ROK );	

}


static int OnuEncryptionOperation_1( short int PonPortIdx, short int OnuIdx, unsigned int cryptionDirection)
{	
	/*if( ThisIsValidOnu(PonPortIdx, OnuIdx ) == ROK )*/
		/*
		if( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].EncryptDirection == cryptionDirection )
			return( ROK );
		*/
		/* added by chenfj 2007-9-13
			在对ONU加密时，判断ONU与OLT类型是否匹配*/
	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
	{
		if( EncryptIsSupported( PonPortIdx, OnuIdx) != ROK ) 
			return( RERROR );
	}

	PonPortTable[PonPortIdx].EncryptEnable = V2R1_ENABLE;
	if( cryptionDirection != PON_ENCRYPTION_PURE ){
		return(StartOnuEncryption( PonPortIdx, OnuIdx, cryptionDirection ));
	}
	else{
		/* 问题单#5758
		更改ONU的PON口加密方向值，会影响密钥更新时间设置
		先启动ONU加密为下行加密，之后再将加密设置成上下行加密；程序在实现时，需要先停止ONU加密，然后再将加密设置成上下行加密；且在停止ONU加密时，关联将ONU密钥更新时间恢复成默认值。故出现问题单所述；
		修改: 将停止ONU加密，与ONU密钥更新时间恢复成默认值 不关联，作为两个独立的操作
		*/
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].EncryptKeyTime = OnuConfigDefault.EncryptKeyTime;
		ONU_MGMT_SEM_GIVE;
		return( StopOnuEncrypt( PonPortIdx,  OnuIdx ));	
	}
	/*else return( RERROR );*/
}

int OnuEncryptionOperation( short int PonPortIdx, short int OnuIdx, unsigned int EncryptionDirection)
{
	unsigned long aulMsg[4] = { MODULE_PON, FC_START_ENCRYPT, 0,0 };
	LONG ret;

	CHECK_ONU_RANGE

	aulMsg[2] =(unsigned long)PonPortIdx;
	aulMsg[3] = (aulMsg[2] << 16) +  (unsigned long )OnuIdx;

	switch (EncryptionDirection)
		{
		case  PON_ENCRYPTION_PURE:
			aulMsg[1] = FC_STOP_ENCRYPT;
			aulMsg[2] = PON_ENCRYPTION_PURE;
			break;
		case  PON_ENCRYPTION_DIRECTION_ALL:
			aulMsg[2] = PON_ENCRYPTION_DIRECTION_ALL;
			break;
		case  PON_ENCRYPTION_DIRECTION_DOWN:
			aulMsg[2] = PON_ENCRYPTION_DIRECTION_DOWN;
			break;
		default: 
			return(RERROR);
		}
	
	ret = VOS_QueSend( g_Olt_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK ){

		return( RERROR );
		}	

	return( ROK );
	
}

int  OnuEncrypt( short int PonPortIdx, short int OnuIdx, unsigned int EncryptionDirection)
{
	LONG retSem;
	int ret; 
	retSem = VOS_SemTake(OnuEncryptSemId, WAIT_FOREVER);
	ret = OnuEncryptionOperation_1(PonPortIdx, OnuIdx, EncryptionDirection);
	if(retSem == VOS_OK)
		VOS_SemGive(OnuEncryptSemId);
	return(ret);
}

int GetOnuEncrypt( short int PonPortIdx, short int OnuIdx, unsigned int *cryptionDirection )
{
	CHECK_ONU_RANGE

	if(cryptionDirection == NULL ) return( RERROR );

	ONU_MGMT_SEM_TAKE;
	*cryptionDirection = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].EncryptDirection;
	ONU_MGMT_SEM_GIVE;
	return( ROK );
}
/*
static int SetOnuEncryptKey( short int PonPortIdx, short int OnuIdx, PON_encryption_key_t PON_encryption_key )
{
	CHECK_ONU_RANGE
		
	VOS_MemCpy( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].EncryptKey ,PON_encryption_key, PON_ENCRYPTION_KEY_SIZE);
	UpdateOnuEncryptKey( PonPortIdx, OnuIdx );
	return( ROK );
}
*/
/***
   注释:  数据库中的时间参数单位为毫秒，设置时注意单位换算, 应乘以SENOND*1
***/
int  SetOnuEncryptKeyExchagetime(short int PonPortIdx, short int OnuIdx, unsigned int timeLen)
{
    /* B--modified by liwei056@2011-2-12 for OnuCfgFullResume */
#if 1
    return OnuMgt_SetOnuEncryptParams(PonPortIdx, OnuIdx, NULL, (int)timeLen);
#else
	int onuEntry;
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].EncryptKeyTime = timeLen;
	OnuMgmtTable[onuEntry].EncryptCounter = (OnuMgmtTable[onuEntry].EncryptKeyTime/SECOND_1);
	ONU_MGMT_SEM_GIVE;

	return( ROK );
#endif
    /* E--modified by liwei056@2011-2-12 for OnuCfgFullResume */
}

int GetOnuEncryptKeyExchagetime( short int PonPortIdx, short int OnuIdx, unsigned int  *timeLen )
{
	CHECK_ONU_RANGE

	if( timeLen == NULL ) return( RERROR );
	ONU_MGMT_SEM_TAKE;
	*timeLen = (OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].EncryptKeyTime/SECOND_1);
	ONU_MGMT_SEM_GIVE;

	return( ROK );
}

unsigned int GetOnuEncryptDefault(void)
{
	return(PON_ENCRYPTION_PURE);
}

/* 取 ONU 加密密钥更新时间默认值*/
unsigned int GetOnuEncryptKeyExchagetimeDefault(void)
{
	return(OnuConfigDefault.EncryptKeyTime);
}

/* B--added by liwei056@2011-2-12 for CfgFullResume */
int CopyOnuEncryptParams(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iRlt = 0;
    int iSrcEntry;
    int iSrcCfg[2];

    iSrcEntry  = SrcPonPortIdx * MAXONUPERPON + SrcOnuIdx;
    ONU_MGMT_SEM_TAKE;
    iSrcCfg[0] = OnuMgmtTable[iSrcEntry].EncryptDirection;
    iSrcCfg[1] = OnuMgmtTable[iSrcEntry].EncryptKeyTime;
    ONU_MGMT_SEM_GIVE;
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OnuMgt_SetOnuEncryptParams(DstPonPortIdx, DstOnuIdx, &iSrcCfg[0], iSrcCfg[1]);
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        int *piEncDir = NULL;
        
        if ( iSrcCfg[0] != GetOnuEncryptDefault() )
        {
            piEncDir = &iSrcCfg[0];
        }

        if ( GetOnuEncryptKeyExchagetimeDefault() == iSrcCfg[1] )
        {
            iSrcCfg[1] = -1;
        }

        if ( (NULL != piEncDir)
            || (iSrcCfg[1] > 0) )
        {
            iRlt = OnuMgt_SetOnuEncryptParams(DstPonPortIdx, DstOnuIdx, piEncDir, iSrcCfg[1]);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            int iDstEntry;
            int iDstCfg[2];
            
            iDstEntry  = DstPonPortIdx * MAXONUPERPON + DstOnuIdx;
            ONU_MGMT_SEM_TAKE;
            iDstCfg[0] = OnuMgmtTable[iDstEntry].EncryptDirection;
            iDstCfg[1] = OnuMgmtTable[iDstEntry].EncryptKeyTime;
            ONU_MGMT_SEM_GIVE;
            if ( (iDstCfg[0] != iSrcCfg[0])
                || (iDstCfg[1] != iSrcCfg[1]) )
            {
                iRlt = OnuMgt_SetOnuEncryptParams(DstPonPortIdx, DstOnuIdx, &iSrcCfg[0], iSrcCfg[1]);
            }
        }
        else
        {
            iRlt = OnuMgt_SetOnuEncryptParams(DstPonPortIdx, DstOnuIdx, &iSrcCfg[0], iSrcCfg[1]);
        }
    }

    return iRlt;
}
/* E--added by liwei056@2011-2-12 for CfgFullResume */

int GenerationRandom()
{

	int Random;

	Random = rand();
	/*sys_console_printf("\r\n the Random is %x \r\n ", Random );*/

	return( Random );
}

/*
int setSeed()
{

	Seed = 1;
	srand();
	return( ROK );
}
*/

int GenerationEncryptKey( short int PonPortIdx, short int OnuIdx )
{
	int Random1, Random2, onuEntry;
	
	CHECK_ONU_RANGE	
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	
	Random1 = GenerationRandom();
	Random2 = GenerationRandom();

	ONU_MGMT_SEM_TAKE;
	*(int *)&OnuMgmtTable[onuEntry].EncryptKey[8] = Random1;
	*(int *)&OnuMgmtTable[onuEntry].EncryptKey[12] = Random2;
	ONU_MGMT_SEM_GIVE;

	return( ROK );

}


static int UpdateOnuEncryptKey_1( short int PonPortIdx, short int OnuIdx /*, PON_encryption_key_t PON_encryption_key*/)
{
	short int onu_id;
	short int llid;
	short int PonChipType/*, PonChipVer*/;

	PON_encryption_key_t PON_encryption_key;
	short int ret;

	CHECK_ONU_RANGE

	/* start the encrypt timer, and generation new encryption key */
	GenerationEncryptKey(PonPortIdx, OnuIdx );

	ONU_MGMT_SEM_TAKE;
	VOS_MemCpy( PON_encryption_key, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].EncryptKey, PON_ENCRYPTION_KEY_SIZE );
	ONU_MGMT_SEM_GIVE;
	/*if( OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].EncryptEnable  != V2R1_ENABLE ) return( RERROR );
	if( OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].EncryptStatus != V2R1_STARTED) return( RERROR );*/

	/*
	result = GetPonPortOperStatus(  PonPortIdx );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR )) return( RERROR );
	*/
	if( PonPortIsWorking(PonPortIdx) != TRUE ) return( RERROR );
	
	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx);
		
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID )
		{
		sys_console_printf("\r\n  %s/port%d onu %d is not registered(UpdateOnuEncryptKey())\r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
		return( RERROR );
		}
	llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if((llid < 0 )||( llid > 127))
		{
		sys_console_printf("\r\n  %s/port%d onu %d wrong llid %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), llid );
		return( RERROR );
		}

	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if(( llid > 0 )&&(llid <=127 ))
			{
			/* add by chenfj 2006-11-17 
			问题单3176.PON节点下，有时执行加密使能，去使能会导致ONU离线*/
#if 0
			if( VOS_SemTake(OnuEncryptSemId, WAIT_FOREVER) ==  VOS_ERROR ) 
				{
				sys_console_printf( "get SemId error(UpdateOnuEncryptKey)\r\n" );
				return ( RERROR );
				}
#endif
            #if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = OnuMgt_UpdateEncryptionKey( PonPortIdx, OnuIdx, PON_encryption_key/*PON_encryption_key*/, PON_ENCRYPTION_UPDATE_PASSAVE );
			#else
			ret = PAS_update_encryption_key( PonPortIdx, llid, PON_encryption_key/*PON_encryption_key*/, PON_ENCRYPTION_UPDATE_PASSAVE );
			#endif
			/*OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].EncryptKeyCounter++;*/
#if 0
#ifndef  PON_ENCRYPTION_HANDLER
			VOS_SemGive(OnuEncryptSemId);
#endif
#endif
			if(  ret != PAS_EXIT_OK ) 
				{
				if(EVENT_ENCRYPT == V2R1_ENABLE )
				sys_console_printf(" %s/port%d onu%d update encrypt key err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), ret );
				return ( RERROR );
				}
			}
		}

	else{
		}

	/*VOS_TaskDelay(EncryptKeyDelayTime);*/

	return( ROK );	
	
}

int UpdateEncryptKey( short int PonPortIdx, short int OnuIdx)
{
	unsigned long aulMsg[4] = { MODULE_PON, FC_ENCRYPT_KEY, 0,0 };
	LONG ret;

	CHECK_ONU_RANGE

	aulMsg[2] = (unsigned long )PonPortIdx;
	aulMsg[3] = (aulMsg[2] << 16 ) + (unsigned long )OnuIdx;

	if(VOS_QueNum(g_Olt_Queue_Id) > 50) return(ROK);
	ret = VOS_QueSend( g_Olt_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK ){

		return( RERROR );
		}	

	return( ROK );
	
}

void UpdateOnuEncryptKey( short int PonPortIdx, short int OnuIdx)
{
	int ret;
	LONG retSem;
	
	if(UpdateKeyFlag == V2R1_ENABLE)
		{
		retSem = VOS_SemTake(OnuEncryptSemId, WAIT_FOREVER);
		ret = UpdateOnuEncryptKey_1(PonPortIdx, OnuIdx);
		if(retSem == VOS_OK)
			VOS_SemGive(OnuEncryptSemId);
		}
}

 /*****************************************************
 *
 *    Function: GetOnuDeviceVersion( short int PonPortIdx, short int OnuIdx)
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                short int OnuIdx -- the specific Onu
 *                 
 *    Desc:   get onu info, the onu must be registered
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 /*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
 /*此函数未被调用*/
 #if 0
 int GetOnuDeviceVersion( short int PonPortIdx, short int OnuIdx)
 {
	short int onu_id;
	short int ret;
	short int PonChipType;
	short int PonChipVer = RERROR;

	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx,  OnuIdx);
	if( onu_id == INVALID_LLID )return( RERROR );

	/* modified for PAS-SOFT V5.1*/ 
	PonChipType = V2R1_GetPonchipType(PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        || ( PonChipType == PONCHIP_PAS8411 )/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
	
	if( PonChipType == PONCHIP_PAS )
		{
		/*
		PonChipVer = V2R1_GetPonChipVersion(PonPortIdx);
		*/
		if( PonChipVer == PONCHIP_PAS5001 )
			{
			PAS_device_versions_t  device_versions;
			ret = PAS_get_device_versions_v4( PonPortIdx,  onu_id, &device_versions);

			if( ret == PAS_EXIT_OK )
				{
				/* print these info */
				ONU_MGMT_SEM_TAKE;
				OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].chip_model = device_versions.device_id;
				ONU_MGMT_SEM_GIVE;
#if 0
				if( EVENT_DEBUG == V2R1_ENABLE)
					{
					sys_console_printf("\r\n%s/port%d Onu %d device version:\r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
					sys_console_printf("  deivce id %d\r\n", device_versions.device_id );
					sys_console_printf("  host sw version, major %d minor %d \r\n", device_versions.host_major, device_versions.host_minor );
					sys_console_printf("  host compliant %d \r\n", device_versions.host_compilation );
					sys_console_printf("  firmware major %d minor %d build %d maintance %d \r\n", device_versions.firmware_major, device_versions.firmware_minor, device_versions.build_firmware, device_versions.maintenance_firmware );
					sys_console_printf("  hardware major %x minor %x \r\n", device_versions.hardware_major, device_versions.hardware_minor );
					sys_console_printf("  uni mac type %s \r\n", PON_mac_s[device_versions.system_mac] );
					sys_console_printf("  supported ports %d\r\n", device_versions.ports_supported );
					}
#endif
				return( ROK );
				}
			sys_console_printf("Get %s/port%d onu%d Device version Err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) , ret );
			}
		
		else if(V2R1_NOT_CTC_STACK)
			{
			PON_device_version_t  device_versions1;
			PAS_device_versions_t  device_versions;
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
			if(PONCHIP_PAS8411 == PonChipVer)
			{
				ret = GW10G_REMOTE_PASONU_get_device_version(PonPortIdx, onu_id, &device_versions1);
				ret = GW10G_REMOTE_PASONU_get_onu_versions(PonPortIdx, onu_id, &device_versions);
			}
			else
			{
				ret = REMOTE_PASONU_get_device_version(PonPortIdx, onu_id, &device_versions1);
				ret = REMOTE_PASONU_get_onu_versions(PonPortIdx, onu_id, &device_versions);
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
			if( ret == PAS_EXIT_OK )
				{
				ONU_MGMT_SEM_TAKE;
				OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].chip_model = device_versions.device_id;
				ONU_MGMT_SEM_GIVE;
				/* print these info */				
#if 0
				if( EVENT_DEBUG == V2R1_ENABLE)
					{
					sys_console_printf("\r\n%s/port%d Onu %d device version:\r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
					sys_console_printf("  deivce id %d\r\n", device_versions.device_id );
					sys_console_printf("  hardware major %x minor %x\r\n", device_versions1.hw_major_ver, device_versions1.hw_minor_ver);
					sys_console_printf("  firmware major %d minor %d build %d maintance %d\r\n", device_versions1.fw_major_ver, device_versions1.fw_minor_ver, device_versions1.fw_build_num, device_versions1.fw_maintenance_ver);
					sys_console_printf("  supported OAM ver:%d\r\n", device_versions1.oam_version);

					/*
					sys_console_printf("  host sw version, major %d minor %d \r\n", device_versions.host_major, device_versions.host_minor );
					sys_console_printf("  host compliant %d \r\n", device_versions.host_compilation );
					sys_console_printf("  firmware major %d minor %d build %d maintance %d \r\n", device_versions.firmware_major, device_versions.firmware_minor, device_versions.build_firmware, device_versions.maintenance_firmware );
					sys_console_printf("  hardware major %x minor %x \r\n", device_versions.hardware_major, device_versions.hardware_minor );
					*/
					sys_console_printf("  uni mac type %s \r\n", PON_mac_s[device_versions.system_mac] );
					sys_console_printf("  supported ports %d\r\n", device_versions.ports_supported );
					}			
#endif
				return( ROK );
				}
			sys_console_printf("Get %s/port%d onu%d Device version Err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) , ret );
			}
		
		}

	/* other pon chip type handler */
	else {

		}

	return( RERROR );
 }
 #endif

 short int GetOnuDeviceChipId(short int PonPortIdx, short int OnuIdx, short int *ChipId)
 {
	/*
	short int PonChipVer;
	short int PonChipType;
	*/
	
	CHECK_ONU_RANGE

	if(ChipId == NULL ) return( RERROR );

	if( OLTAdv_IsExist(PonPortIdx) != TRUE ) return( RERROR );

	/*	if( V2R1_CTC_STACK == TRUE ) return( RERROR );
	if(GetOnuVendorType( PonPortIdx, OnuIdx ) == ONU_VENDOR_CT )
		return( RERROR );*/

	ONU_MGMT_SEM_TAKE;
	*ChipId = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].chip_model;
	ONU_MGMT_SEM_GIVE;
	return( ROK );

	/*
	PonChipVer= V2R1_GetPonchipType(PonPortIdx);
	
	if(( PonChipVer == PONCHIP_PAS5001) ||( PonChipVer == PONCHIP_PAS5201))
		{
		if(( OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].chip_model == ONUCHIP_PAS6201) ||
			( OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].chip_model == ONUCHIP_PAS6301))
			{
			*ChipId = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].chip_model;
			return( ROK );
			}
		}
	
	return( RERROR );
	*/
 }

int GetOnuCapability(short int PonPortIdx, short int OnuIdx )
{
	short int PonChipType;
	
	short int onu_id;
	short int ret = PAS_EXIT_ERROR;
	int onuEntry;
	
	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );	

	/* modified for PAS-SOFT V5.1*/ 
	PonChipType = V2R1_GetPonchipType(PonPortIdx);
	
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if (OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			PAS_physical_capabilities_t device_capabilities;

			ret = PAS_get_device_capabilities_v4( PonPortIdx,  onu_id, &device_capabilities);

			if( ret == PAS_EXIT_OK )
				{
				/* printf these info */
#if 0
				if( EVENT_DEBUG == V2R1_ENABLE)
					{
					sys_console_printf("\r\n%s/port%d Onu %d capabilities\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
					sys_console_printf("   Laser on/off timer %d/%d\r\n", device_capabilities.laser_on_time, device_capabilities.laser_off_time );
					sys_console_printf("   Max grant number %d\r\n", device_capabilities.onu_grant_fifo_depth );
					sys_console_printf("   AGC/CDR lock time %d/%d\r\n", device_capabilities.agc_lock_time, device_capabilities.cdr_lock_time );
					sys_console_printf("   pon tx signal %d\r\n", device_capabilities.pon_tx_signal );
					}
#endif
				/* save these info to OnuMgmtTable[] */
				onuEntry = MAXONUPERPON * PonPortIdx + OnuIdx;
				ONU_MGMT_SEM_TAKE;
				OnuMgmtTable[onuEntry].Laser_ON = device_capabilities.laser_on_time;
				OnuMgmtTable[onuEntry].Laser_OFF = device_capabilities.laser_off_time;
				OnuMgmtTable[onuEntry].GrantNumber = device_capabilities.onu_grant_fifo_depth;
				OnuMgmtTable[onuEntry].AGC_Time = device_capabilities.agc_lock_time;
				OnuMgmtTable[onuEntry].CDR_Time = device_capabilities.cdr_lock_time;
				ONU_MGMT_SEM_GIVE;
				return( ROK );
				}
			else return ( RERROR );
			}
		
		else
			{
			ret = PAS_EXIT_OK;
			/*
			PON_olt_optics_configuration_t  optics_configuration;
			bool   pon_tx_signal;
			*/
			/*PAS_get_raw_statistics(PonPortIdx, onu_id, PON_RAW_STAT_STANDARD_OAM_MPCP_STATUS, const short int statistics_parameter, void * statistics_data, PON_timestamp_t * timestamp)*/
			/*PAS_get_olt_optics_parameters( PonPortIdx, &optics_configuration, &pon_tx_signal);*/
			return( ROK );
			}			
		}

	/* other pon chip type handler */
	else {

		}
	
	return( RERROR );

}

int GetOnuOamInformation( short int PonPortIdx, short int OnuIdx)
{
	short int Onu_id;
	short int PonChipType;
	short int ret;
	
	CHECK_ONU_RANGE

	Onu_id = GetLlidByOnuIdx(  PonPortIdx,   OnuIdx);
	if( Onu_id == INVALID_LLID ) return( RERROR );

	PonChipType = GetPonChipTypeByPonPort(  PonPortIdx);
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		PON_oam_information_t oam_information;
		OAM_3_3_tlv_t  OAM_3_3_info;

		oam_information.oam_standard_information = &OAM_3_3_info;
		/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		ret = OLT_GetOamInformation( PonPortIdx, Onu_id, &oam_information );

		if( ret != PAS_EXIT_OK )
			{
			sys_console_printf(" Get %s/port%d Onu %d OAM information err %d \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), ret );
			return( RERROR );
			}
#if 0
		if( EVENT_DEBUG == V2R1_ENABLE)
			{
			sys_console_printf("%s/port%d Onu %d OAM information \r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
			sys_console_printf("    the OAM information is originated from PASSAVE onu");
			if( oam_information.passave_originated == TRUE )
				sys_console_printf(" True \r\n");
			else sys_console_printf(" False \r\n");
			
			sys_console_printf("    the OAM version: %d %s \r\n", oam_information.oam_information_reference_standard, OAMVersion_s[oam_information.oam_information_reference_standard] );
			sys_console_printf("    the OAM standard info : \r\n");
			if( OAM_3_3_info.state.multiplexer_action == OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_SUPPORTED)
				sys_console_printf("        DTE supports sending Variable Response OAMPDUs\r\n");
			else if( OAM_3_3_info.state.multiplexer_action == OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_NOT_SUPPORTED)
				sys_console_printf("         DTE does not support sending Variable Response OAMPDUs\r\n");
			sys_console_printf("    the configuration: \r\n");
			if( OAM_3_3_info.configuration.variable_retrieval == OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_SUPPORTED	)
				sys_console_printf("        DTE supports sending Variable Response OAMPDUs\r\n");
			else if( OAM_3_3_info.configuration.variable_retrieval == OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_NOT_SUPPORTED)
				sys_console_printf("        DTE does not support sending Variable Response OAMPDUs \r\n");
			if( OAM_3_3_info.configuration.link_events == OAM_INTERPRETING_LINK_EVENTS_SUPPORTED )
				sys_console_printf("        DTE supports interpreting Link Events\r\n");
			else if( OAM_3_3_info.configuration.link_events == OAM_INTERPRETING_LINK_EVENTS_NOT_SUPPORTED)
				sys_console_printf("        DTE does not support interpreting Link Events\r\n");
			if( OAM_3_3_info.configuration.loopback_support == OAM_LOOPBACK_SUPPORTED)
				sys_console_printf("        DTE is capable of OAM loopback mode\r\n");
			else if( OAM_3_3_info.configuration.loopback_support == OAM_LOOPBACK_NOT_SUPPORTED)
				sys_console_printf("        DTE is not capable of OAM loopback mode\r\n");

			if(OAM_3_3_info.configuration.unidirectional_support == OAM_UNIDIRECTIONAL_SUPPORTED )
				sys_console_printf("        DTE is capable of sending OAMPDUs when the receive path is non-operational\r\n");
			else if (OAM_3_3_info.configuration.unidirectional_support ==  OAM_UNIDIRECTIONAL_NOT_SUPPORTED )
				sys_console_printf("        DTE is not capable of sending OAMPDUs when the receive path is non-operational\r\n");
			if( OAM_3_3_info.configuration.oam_mode == OAM_MODE_ACTIVE )
				sys_console_printf("        DTE configured in Active mode \r\n");
			else if(OAM_3_3_info.configuration.oam_mode == OAM_MODE_PASSIVE)
				sys_console_printf("        DTE configured in Passive mode\r\n");
	
			sys_console_printf("    The Pdu Size: %d \r\n", OAM_3_3_info.pdu_configuration.maximum_pdu_size );
			sys_console_printf("    vendor_identifier: \r\n");
			sys_console_printf("        identifier: %x \r\n", OAM_3_3_info.vendor_identifier.identifier );
			sys_console_printf("        OUI: %x \r\n", OAM_3_3_info.vendor_identifier.oui );
			
			}
#endif
		return( ROK );
		}

	/* other pon chip type handler */
	else if( PonChipType == PONCHIP_GW )
		{
		
		}
	else if( PonChipType == PONCHIP_TK )
		{

		}

	return( RERROR );
}

int GetOnuRegisterData( short int PonPortIdx, short int OnuIdx )
{
	int ret;
	int OnuEntryIdx;
	short int PonChipType = V2R1_GetPonchipType( PonPortIdx );/*Q.25126、Q25371*/

#if 0
	unsigned short OnuPonChip;
    short int onu_id;
	onu_registration_data_record_t onu_registration_data;
	PON_onu_versions onu_version_info;

	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if(onu_id == INVALID_LLID ) return ( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        ) 
		{
		/*PonChipVer = PonChipType;*/
		PonChipType = PONCHIP_PAS;
		}

	if(( PonChipType == PONCHIP_PAS )/* && ( V2R1_NOT_CTC_STACK )*/)
    {   
    	ret = PAS_get_onu_registration_data( PonPortIdx, onu_id, &onu_registration_data);
    	if( ret == 0 )
    	{
            /* B--added by liwei056@2011-2-23 for OnuPonChipType */
            if ( 0 == PAS_get_onu_version( PonPortIdx, onu_id, &onu_version_info) )
            {
                OnuPonChip = onu_version_info.hardware_version;
            }
            else
            {
                OnuPonChip = 0;
            }
            /* E--added by liwei056@2011-2-23 for OnuPonChipType */
    	}
    }
	/* other pon chip type handler */
	else
    {
        ret = RERROR;
    }
#else
    onu_registration_info_t onu_registration_data;

	CHECK_ONU_RANGE

    ret = OnuMgt_GetOnuRegisterInfo( PonPortIdx, OnuIdx, &onu_registration_data );
#endif

	if( ret == 0 )
    {
		/* print these info  */
		/*if( EVENT_REGISTER == V2R1_ENABLE)
			{
			sys_console_printf("\r\n%s/port%d Onu %d register data \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
			sys_console_printf("   Laser on/off time %d / %d \r\n", onu_registration_data.laser_on_time, onu_registration_data.laser_off_time );
			sys_console_printf("   OAM version: %s \r\n", OAMVersion_s[onu_registration_data.oam_version] );
			sys_console_printf("   the round trip time %d(m) \r\n", (onu_registration_data.rtt *16/10));
			}*/
			
		OnuEntryIdx = MAXONUPERPON * PonPortIdx + OnuIdx;
        
		ONU_MGMT_SEM_TAKE;

        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
    		OnuMgmtTable[OnuEntryIdx].RTT = (onu_registration_data.rtt *16/10);
    		OnuMgmtTable[OnuEntryIdx].DeviceInfo.PonChipType = onu_registration_data.productVersion;
        }
        /*Begin----modified by wangjiah@2016-07-19 to solve Issue:30605--*/        	
		else if( OLT_PONCHIP_ISBCM(PonChipType) )
		{
    		OnuMgmtTable[OnuEntryIdx].RTT = onu_registration_data.rtt; 
    		OnuMgmtTable[OnuEntryIdx].DeviceInfo.PonChipType = onu_registration_data.productCode;
		}
        /*End----modified by wangjiah@2016-07-19--*/        	
        else
        {
        	/*modified by liyang 2014-12-04*/        	
    		OnuMgmtTable[OnuEntryIdx].RTT = (onu_registration_data.rtt *16/10 - 50)>0?(onu_registration_data.rtt *16/10 - 50):0;/*Q.25126、Q25371*/
    		OnuMgmtTable[OnuEntryIdx].DeviceInfo.PonChipType = onu_registration_data.productCode;
        }
		OnuMgmtTable[OnuEntryIdx].DeviceInfo.PonChipVendor = GetOnuChipVenderID(onu_registration_data.vendorType);
		OnuMgmtTable[OnuEntryIdx].OAM_Ver = onu_registration_data.oam_version;
		OnuMgmtTable[OnuEntryIdx].llidMax = onu_registration_data.max_links_support;
        VOS_ASSERT(OnuMgmtTable[OnuEntryIdx].llidNum == onu_registration_data.curr_links_num);
		OnuMgmtTable[OnuEntryIdx].cmMax = onu_registration_data.max_cm_support;
		OnuMgmtTable[OnuEntryIdx].PonRate = onu_registration_data.pon_rate_flags;
		OnuMgmtTable[OnuEntryIdx].Laser_ON = onu_registration_data.laser_on_time;
		OnuMgmtTable[OnuEntryIdx].Laser_OFF = onu_registration_data.laser_off_time;

		if(OnuMgmtTable[OnuEntryIdx].RTT > PonPortTable[PonPortIdx].CurrRTT )
			PonPortTable[PonPortIdx].CurrRTT = OnuMgmtTable[OnuEntryIdx].RTT;
		ONU_MGMT_SEM_GIVE;
	
		return( ROK );
    }   
    
	if( EVENT_REGISTER == V2R1_ENABLE)
	sys_console_printf("Get %s/port%d onu%d register data Err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) , ret );

	return( RERROR );
}

int GetOnuConfigPara( short int PonPortIdx, short int OnuIdx )
{
    
	short int PonChipType;
	short int ret;
	short int onu_id;

	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if(onu_id == INVALID_LLID ) return ( RERROR );
   
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if(OLT_PONCHIP_ISPAS(PonChipType))
		{
		PON_llid_parameters_t  llid_parameters ;
		#if 1
		ret = OnuMgt_GetLLIDParams( PonPortIdx,  OnuIdx, &llid_parameters);
		#else
 		ret = PAS_get_llid_parameters( PonPortIdx,  onu_id, &llid_parameters);
		#endif
		/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if( ret == PAS_EXIT_OK )
			{
			/* print these info  */
#if 0
			if( EVENT_DEBUG == V2R1_ENABLE)
				{
				sys_console_printf("\r\n%s/port%d Onu %d config Para:\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
				sys_console_printf("  MAC Addr:%02x-%02x-%02x-%02x-%02x-%02x\r\n", llid_parameters.mac_address[0], llid_parameters.mac_address[1], llid_parameters.mac_address[2], llid_parameters.mac_address[3], llid_parameters.mac_address[4], llid_parameters.mac_address[5] );

				if(( llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_DOWNLINK ) ||( llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_UPLINK_AND_DOWNLINK ))
					llid_parameters.encryption_mode ++;
				else if( llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION )
					llid_parameters.encryption_mode = 1;
				else
					llid_parameters.encryption_mode = 0;
				sys_console_printf("  encrypt direction:%s\r\n", v2r1EncryptDirection[llid_parameters.encryption_mode] );
				sys_console_printf("  OAM ver:%d\r\n", llid_parameters.oam_version );
				}
#endif
			return( ROK );
			}
		}

	/* other pon chip type handler */
	else {

		}

	return( RERROR );
}

int GetOnuDownlinkBufferConfig(short int PonPortIdx, short int OnuIdx )
{
	short int PonChipType;
	
	short int ret;
	short int onu_id;
	short int i;

	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if(onu_id == INVALID_LLID ) return ( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		PON_downlink_buffer_priority_limits_t  priority_limits;
		/*
		PonChipVer = V2R1_GetPonChipVersion( PonPortIdx );
		*/
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		ret = OLT_GetDownlinkBufferConfiguration( PonPortIdx , &priority_limits);

		if( ret == PAS_EXIT_OK )
			{
			sys_console_printf("\r\n%s/port%d Onu%d downlink buffer config:\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
			for(i=0;i<=MAX_PRIORITY_QUEUE; i++)
				sys_console_printf("priority %d buffer size %d\r\n", i, priority_limits.priority_limit[i]);
			return( ROK );
			}
		}

	/* other pon chip type handler */
	else{

		}

	return( RERROR );

}


int ScanOnuMgmtTableTimer( ULONG timer_counter )
{
	int OnuAuthTimer;
	short  int OnuEntry, OnuEntryBase; 
	short int  slot=0, port=0;
	short int PonPortIdx, OnuIdx;
	int pon_insert, pon_status;
	static ULONG v2r1_sys_timer_count= 0;

	v2r1_sys_timer_count++;
    
	/* deleted by chenfj 2009-3-16
		只在PON板启动时执行PON 端口版本比较；
	if(((v2r1_sys_timer_count+1) % 120) == 0)
		AllPonPortFirmwareAndDBAVersionMatchByVty(NULL);
	*/

	OnuAuthTimer = ( timer_counter + 3 ) % PRIVATE_TIME;
	for ( PonPortIdx = 0; PonPortIdx< MAXPON; PonPortIdx++)
	{
		/* B--moved by liwei056@2010-1-25 from ScanPonPortHotSwap() for Pon-FastSwicthHover & Pon-BackupMonitor  */
		if( 0 == OnuAuthTimer )
		{
			/* 清除PendingOnu在线状态 */
			ScanAuthOnuCounter(PonPortIdx );
		}   
		/* E--moved by liwei056@2010-1-25 from ScanPonPortHotSwap() for Pon-FastSwicthHover & Pon-BackupMonitor  */

		if( v2r1_sys_timer_count >= V2R1_SYS_TIME_PERIOD )
		{
			OnLineOnuCounter(PonPortIdx );
			/*SyncSysTimeToOnu( PonPortIdx, MAXONUPERPON );*/
		}

		port = (PonPortIdx % PONPORTPERCARD )+1;
		slot  = (short int )GetCardIdxByPonChip(PonPortIdx)+1;
		pon_insert = getPonChipInserted(slot, port );
		pon_status = PonPortIsWorking(PonPortIdx);

		OnuEntryBase = PonPortIdx * MAXONUPERPON;
		for(OnuIdx=0; OnuIdx < MAXONUPERPON; OnuIdx++)
		{				
			OnuEntry = OnuEntryBase + OnuIdx;	

			/*if( GetOnuOperStatus(PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )*/
			if( ONU_OPER_STATUS_UP == OnuMgmtTable[OnuEntry].OperStatus )
			{
				if( pon_insert != PONCHIP_EXIST )
					continue;
				if( pon_status != TRUE )
					continue;
				if( GetOnuVendorType ( PonPortIdx, OnuIdx ) == ONU_VENDOR_CT )
					continue;

				if( v2r1_sys_timer_count >= V2R1_SYS_TIME_PERIOD )
					SyncSysTimeToOnuMsg( PonPortIdx, OnuIdx );
				
				ONU_MGMT_SEM_TAKE;

				if( OnuMgmtTable[OnuEntry].EncryptEnable == V2R1_ENABLE )
				{
				    /* 密钥定时更换 */
					OnuMgmtTable[OnuEntry].EncryptCounter --;
					if( OnuMgmtTable[OnuEntry].EncryptCounter == 0 )
					{
						UpdateEncryptKey( PonPortIdx, OnuIdx );
						OnuMgmtTable[OnuEntry].EncryptCounter = (OnuMgmtTable[OnuEntry].EncryptKeyTime/SECOND_1);
					}
					
					if( 0 == OnuAuthTimer )
					{
						/* 问题单12334, ONU刚注册时下发加密配置可能是无效的，实际测试大部分情况下不成功 */
						int iEncDir = 0, iKeyTime = 0, status = 0;
						if ( 0 == OnuMgt_GetOnuEncryptParams(PonPortIdx, OnuIdx, &iEncDir, &iKeyTime, &status) )
						{
							if( (iEncDir == PON_ENCRYPTION_DIRECTION_DOWN) || (iEncDir == PON_ENCRYPTION_DIRECTION_ALL) )
							{
								if( status == V2R1_NOTSTARTED )
								{
		    							OnuEncrypt(PonPortIdx, OnuIdx, OnuMgmtTable[OnuEntry].EncryptDirection);
										
									sys_console_printf(" onu%d/%d/%d encrypt restart (EN=%d,ST=%d,DIR=%d)\r\n", 
										GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuIdx+1,
										OnuMgmtTable[OnuEntry].EncryptEnable, status, iEncDir );
								}
							}
						}
					}
				}
				ONU_MGMT_SEM_GIVE;
			}
		}
	}

	if( v2r1_sys_timer_count >= V2R1_SYS_TIME_PERIOD )
	{
		v2r1_sys_timer_count = 0;
	}

	return( ROK );
}


/*Begin:for onu swap by jinhl@2013-04-27*/
extern int __PON_SWAP_DELAY;
int OnuSwapRegHandler(OnuEventData_s *OnuRegData)
{
    short int PonPortIdx_swap, PonPortIdx;
	int slot_swap = 0;
	int port_swap = 0;
	short int llid,onu_idx,standby_onu_idx, standby_llid;
	int iWaitTimes = 0;
	int iRlt = VOS_ERROR;
	int reg_flag = -1;
	short int standby_llid_idx = 0;
	short int PonChipVer;
	int OnuEntry = 0;
	int failTimes = 0;
	
	PonPortIdx      = OnuRegData->PonPortIdx;
	llid            = OnuRegData->llid;
    standby_llid    = OnuRegData->llid;
	onu_idx         = OnuRegData->OnuIdx;
	
    
    OLT_LOCAL_ASSERT(PonPortIdx);
    ONU_ASSERT(onu_idx);
    LLID_ASSERT(standby_llid);

	iRlt = PonPortSwapPortQuery(PonPortIdx, &PonPortIdx_swap);
	
    if((iRlt != VOS_OK) || (PonPortIdx_swap < 0) )
	{
	    VOS_ASSERT(0);
		return VOS_ERROR;
	}

    slot_swap = GetCardIdxByPonChip(PonPortIdx_swap);
	port_swap = GetPonPortByPonChip(PonPortIdx_swap);
	PonChipVer = GetPonChipTypeByPonPort(PonPortIdx_swap);
	if( (slot_swap < 0) || (port_swap < 0))
	{
	    VOS_ASSERT(0);
		return VOS_ERROR;
	}
	
	standby_onu_idx = OLTAdv_SearchFreeOnuIdx(PonPortIdx_swap, OnuRegData->onu_mac, &reg_flag);
	ONU_REGISTER_DEBUG("Enter into OnuSwapRegHandler,standby_onu_idx:%d\r\n",standby_onu_idx);
	if( standby_onu_idx <= RERROR )	/* 注册满 */
	{
		/*#2606 问题在手工添加64个ONU后，再实际注册新ONU时，出现异常*/
		ONU_REGISTER_DEBUG("\r\n no g_free entry for this onu(pon=%d/%d,llid=%d\r\n", slot_swap, port_swap, llid );
				
		Trap_PonPortFull(PonPortIdx_swap);
		
		return( RERROR );
	}
	else
	{
		OnuEntry = PonPortIdx_swap * MAXONUPERPON + standby_onu_idx;
		if( OnuEntry < 0 || OnuEntry > MAXONU )
		{
			VOS_ASSERT(0);
			return RERROR;
		}
		ONU_REGISTER_DEBUG("\r\n onu %d/%d/%d is %s-register_redundancy, entry is %d \r\n", slot_swap, port_swap, (standby_onu_idx+1), (reg_flag ? "new" : "re"), OnuEntry);
        
        /* 支持onu倒换的，肯定都是CTC onu */
        ONU_SetupIFsByType(PonPortIdx_swap, standby_onu_idx, PonChipVer, ONU_MANAGE_CTC); 
  
		if( reg_flag == 0 )		/* 0-re-register, 1-new register, 2-replaced */
		{
			ONU_MGMT_SEM_TAKE;
       
			if( ONU_OPER_STATUS_UP == OnuMgmtTable[OnuEntry].OperStatus )
			{               
               
                if ( RERROR == (standby_llid_idx = GetOnuLLIDIdx( PonPortIdx_swap, standby_onu_idx, standby_llid )) )
                {
    			    standby_llid_idx = OnuMgmtTable[OnuEntry].llidNum++;
                }
                else if ( 0 == standby_llid_idx )
                {
    			    
    			    ONU_REGISTER_DEBUG("pon%d simulate onu%d's deregister event for double-register.\r\n", PonPortIdx_swap, standby_onu_idx+1);
    			    
    			    OLT_ResumeLLIDStatus(PonPortIdx_swap, standby_llid, PON_ONU_DEREGISTRATION_DOUBLE_REGISTRATION, OLT_RESUMEMODE_FORCEDOWN);

                    /* 强制离线后，继续新的注册 */
                    OnuMgmtTable[OnuEntry].llidNum = 1;
                }
                else
                {
                    /* 重新发现Link，只恢复LLID相关配置，暂无其它处理 */
                }
			}
			else
            {
               

                if ( RERROR == (standby_llid_idx = GetOnuLLIDIdx( PonPortIdx_swap, standby_onu_idx, standby_llid )) )
                {
    			    standby_llid_idx = OnuMgmtTable[OnuEntry].llidNum++;
                }
                else
                {
                    /* 重新发现Link，只恢复LLID相关配置，暂无其它处理 */
                }
            }
            
			ONU_MGMT_SEM_GIVE;
		}
		else			/* 1-new register, 2-replaced */
		{
			if( reg_flag == 2 )	/* 0-re-register, 1-new register, 2-replaced */
			{
				InitOneOnuByDefault(PonPortIdx_swap, standby_onu_idx);
			}

			ONU_MGMT_SEM_TAKE;
            OnuMgmtTable[OnuEntry].llidNum = 1;
			ONU_MGMT_SEM_GIVE;
		}
	} 

	iRlt = OLT_RdnLLIDAdd(PonPortIdx, llid);
				
	ONU_SWITCH_DEBUG("\r\n This llid(%d/%d/%d:%d) add rdninfo  %s,virRegFailTimes:%d\r\n", slot_swap, port_swap, (standby_onu_idx+1), standby_llid, (iRlt == VOS_OK ? "ok" : "err"), OnuMgmtTable[OnuEntry].virRegFailTimes);
	if(VOS_OK != iRlt)
	{
	    #if 1/*存在问题:一旦出现频繁RdnLLIDAdd失败的情况，则会一直无法注册*/
		ONU_MGMT_SEM_TAKE;
        OnuMgmtTable[OnuEntry].virRegFailTimes++;
		failTimes = OnuMgmtTable[OnuEntry].virRegFailTimes;
		ONU_MGMT_SEM_GIVE;
		if(failTimes >= VIRREG_FAILTIMES)
		{
		    ONU_MGMT_SEM_TAKE;
		    OnuMgmtTable[OnuEntry].virRegFailTimes = 0;
			ONU_MGMT_SEM_GIVE;
		    OLT_DeregisterLLID(PonPortIdx_swap, standby_llid, 0);
			OLT_DeregisterLLID(PonPortIdx, llid, 0);
		}
		#endif
		return VOS_ERROR;
	}

	if (standby_onu_idx >= 0)
	{
	    
        int copy_flags;
        unsigned char *MacAddr;
        MacAddr = OnuRegData->onu_mac;
        {
		    /* 等待ONU在对端PON口上的虚注册成功*/ 
            iWaitTimes = 5;
		    do
            {            
			    VOS_TaskDelay(__PON_SWAP_DELAY);
            	if( 0 != OLTAdv_LLIDIsExist(PonPortIdx_swap, standby_llid) )
                    break;
            } while(--iWaitTimes > 0);

            ONU_REGISTER_DEBUG("Onu%d(%02x.%02x.%02x.%02x.%02x.%02x-llid:%d) is %s to wait for sync register from M_pon%d to S_pon%d on the slot%d.\r\n"
                    , standby_onu_idx + 1
                    , MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]
        		    , standby_llid
        		    , (0 < iWaitTimes) ? "successed" : "failed"
        		    , PonPortIdx, PonPortIdx_swap
        		    , SYS_LOCAL_MODULE_SLOTNO);

            copy_flags = OLT_COPYFLAGS_SYNC;
            if (0 < iWaitTimes)
            {
                copy_flags |= OLT_COPYFLAGS_WITHLLID;
            }
			iRlt = OLT_AuthorizeLLID(PonPortIdx_swap, standby_llid, TRUE);
			if(VOS_OK != iRlt )
			{
			    ONU_REGISTER_DEBUG("OLT_AuthorizeLLID err\r\n");
				return VOS_ERROR;
			}
            CopyOnuConfigFromPon1ToPon2(PonPortIdx, onu_idx, PonPortIdx_swap, standby_onu_idx, copy_flags);
            
        }   
        
    }

	return VOS_OK;
}
/*End:for onu swap by jinhl@2013-04-27*/

/* modified by xieshl 20110519, 自动删除不在线ONU PR11109 */
ULONG g_onu_agingtime = 0;	/* 0:never be aged, range 10 *60 - 365 * 24 * 3600 seconds, default 2 days */
const ULONG g_onu_agingtime_default = (15 * 24 * 3600);		/* 15 days */	/* modified by xieshl 20111010, 默认关闭ONU离线删除功能，问题单13581 */

int ScanOfflineOnuAgingTimer()
{
	short int OnuEntry, OnuEntryBase; 
	short int PonPortIdx, OnuIdx;
	short int olt_master, onu_idx;
	int olt_isbackup;
	short int ret;
	int onu_status, onu_is_valid,onu_isautodelete=0;
	ULONG onuDownTime;
	ULONG curTime;
	unsigned char onuMacAddr[BYTES_IN_MAC_ADDRESS];
	unsigned char SN[GPON_ONU_SERIAL_NUM_STR_LEN];
	unsigned char name[256];
	ONUConfigData_t *pl;
	ULONG TableIdx;
	UCHAR Isgpon;
	gpon_onu_auth_t  *pAuthData = NULL;
	/*static ULONG onu_aging_count = 0;*/

	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return ROK;

	if( g_onu_agingtime == 0 )
		return ROK;

	/*onu_aging_count++;
	if( onu_aging_count >= 5 )
		return ROK;*/

	curTime = time(0);
/*sys_console_printf("ScanOfflineOnuAgingTimer,%ul\r\n", curTime );*/

	for ( PonPortIdx = 0; PonPortIdx< MAXPON; PonPortIdx++)
	{
		UCHAR slot = GetCardIdxByPonChip( PonPortIdx );
		UCHAR port = GetPonPortByPonChip( PonPortIdx );
	    /* B--added by liwei056@2012-8-8 for D15437 */
	    if ( (PonPortSwapPortQuery(PonPortIdx, &olt_master) == ROK)
            && (PonPortHotStatusQuery(PonPortIdx) == V2R1_PON_PORT_SWAP_PASSIVE)
            && OLT_LOCAL_ISVALID(olt_master) )
        {
            olt_isbackup = 1;
        }
        else
        {
            olt_isbackup = 0;
        }
	    /* E--added by liwei056@2012-8-8 for D15437 */

		OnuEntryBase = PonPortIdx * MAXONUPERPON;
		for(OnuIdx=0; OnuIdx < MAXONUPERPON; OnuIdx++)
		{				
			OnuEntry = OnuEntryBase + OnuIdx;	
			ONU_MGMT_SEM_TAKE;
			onu_status = OnuMgmtTable[OnuEntry].OperStatus;
			onu_isautodelete = OnuMgmtTable[OnuEntry].IsAutoDelete;
			onuDownTime = OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
			Isgpon = OnuMgmtTable[OnuEntry].IsGponOnu;
            VOS_MemCpy(onuMacAddr, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS);
			VOS_MemCpy(SN, OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, GPON_ONU_SERIAL_NUM_STR_LEN);
			ONU_MGMT_SEM_GIVE;

			if(onu_isautodelete == 1)
			{
				onu_isautodelete = 0;
				continue;
			}
			onu_is_valid = ThisIsValidOnu(PonPortIdx,OnuIdx);
			if( (!onu_is_valid)&& (onu_status != ONU_OPER_STATUS_UP) && 
				(onu_status != ONU_OPER_STATUS_PENDING) && (onu_status != ONU_OPER_STATUS_DORMANT) )
			{
				if( (curTime - onuDownTime) >= g_onu_agingtime )
				{
				    /* B--added by liwei056@2012-8-8 for D15437 */
				    if ( olt_isbackup )
                    {
                        /* 此ONU在主OLT不为DOWN，则不删 */\
						if(Isgpon)
						{
							if ( RERROR != (onu_idx = GetOnuIdxBySnPerPon(olt_master, SN)) )
	                        {
	                			ONU_MGMT_SEM_TAKE;
	                			onu_status = OnuMgmtTable[olt_master * MAXONUPERPON + onu_idx].OperStatus;
	                			ONU_MGMT_SEM_GIVE;

	                            if ( (onu_status == ONU_OPER_STATUS_UP)
	                                || (onu_status == ONU_OPER_STATUS_PENDING)
	                                || (onu_status == ONU_OPER_STATUS_DORMANT) )
	                            {
	                                continue;
	                            }
	                        }
						}
						else
						{
	                        if ( RERROR != (onu_idx = GetOnuIdxByMacPerPon(olt_master, onuMacAddr)) )
	                        {
	                			ONU_MGMT_SEM_TAKE;
	                			onu_status = OnuMgmtTable[olt_master * MAXONUPERPON + onu_idx].OperStatus;
	                			ONU_MGMT_SEM_GIVE;

	                            if ( (onu_status == ONU_OPER_STATUS_UP)
	                                || (onu_status == ONU_OPER_STATUS_PENDING)
	                                || (onu_status == ONU_OPER_STATUS_DORMANT) )
	                            {
	                                continue;
	                            }
	                        }
						}
                    }
				    /* E--added by liwei056@2012-8-8 for D15437 */
					ret = DelOnuFromPonPort( PonPortIdx, OnuIdx );
					if (ret == ROK && conf_associate_share)
					{
						VOS_Sprintf(name,"pon%d/%d",slot,port);
						pl = getOnuConfFromHashBucket(name);
						if(pl)
							onu_profile_associate_by_index(NULL, slot, port, OnuIdx+1,  name);
					}
					if(ret == ROK && Timeout_delete_authentry)
					{
						if(Isgpon)
						{
							pAuthData = gonu_auth_entry_list_seach(slot, port, SN);
							if(pAuthData != NULL)
							{
								DeleteGponAuthBySn(PonPortIdx,SN);
							}
						}
						else
						{
							ret = isHaveAuthMacAddress(slot, port, onuMacAddr, &TableIdx );
							if(VOS_OK == ret)
							{
								setOnuAuthStatus(slot,port,TableIdx,V2R1_ENTRY_DESTORY);
							}
						}
					}
				}
			}
		}
	}

	/*onu_aging_count = 0;*/

	return( ROK );
}

int ScanOnuMgmtTableBerFer()
{
	short int PonPortIdx, OnuIdx;
	short int OnuEntryBase, OnuEntry;
	int ticksNum;
	int  LastedTime;

	ticksNum = VOS_GetTick();
	for(PonPortIdx =0;  PonPortIdx< MAXPONCHIP; PonPortIdx ++ )
	{
		if ( PonPortTable[PonPortIdx].PortWorkingStatus != PONPORT_UP ) continue;

		OnuEntryBase = PonPortIdx *  MAXONUPERPON;
		for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++ )
		{
			if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP ) continue;
			
			OnuEntry = OnuEntryBase + OnuIdx;
			
			if( OnuMgmtTable[OnuEntry].BerFlag == V2R1_TRAP_SEND )
			{
				LastedTime = ( ticksNum - OnuMgmtTable[OnuEntry].LastBerAlarmTime ) / ( VOS_TICK_SECOND ); 
               		 /* B--modified by liwei056@2010-1-20 for D9624 */
#if 0
				if( LastedTime >= 10 ) 
					Trap_OnuPonPortBER( PonPortIdx, OnuIdx, 0, V2R1_TRAP_CLEAR );
#else
				if( LastedTime >= SECOND_PON_BER_ALERM_CLEAR_TIME ) 
					Trap_OnuPonPortBER( PonPortIdx, OnuIdx, 0, V2R1_TRAP_CLEAR );
#endif
                		/* E--modified by liwei056@2010-1-20 for D9624 */
			}
		
			if( OnuMgmtTable[OnuEntry].FerFlag == V2R1_TRAP_SEND )
			{
				LastedTime = ( ticksNum - OnuMgmtTable[OnuEntry].LastFerAlarmTime ) / ( VOS_TICK_SECOND ); 
	              	 /* B--modified by liwei056@2010-1-20 for D9624 */
				if( LastedTime >= SECOND_PON_FER_ALERM_CLEAR_TIME  ) 
				{
					Trap_OnuPonPortFER( PonPortIdx, OnuIdx, 0, V2R1_TRAP_CLEAR );
				}
               		 /* E--modified by liwei056@2010-1-20 for D9624 */
			}
		}
	}		

	return( ROK );

}

/*****************************************************
 *
 *    Function: GetOnuDeviceStatus (short int PonPortIdx, short int OnuIdx)
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
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
#if 0
int GetOnuDeviceStatus (short int PonPortIdx, short int OnuIdx)
{

	PON_onu_device_status_t  status;
	short int PonChipType;
	short int onu_id,  ret,i;

	CHECK_ONU_RANGE

	PonChipType = GetPonChipTypeByPonPort(  PonPortIdx);
	onu_id = GetLlidByOnuIdx(  PonPortIdx,   OnuIdx);
	if( onu_id !=  INVALID_LLID){
		if( PonChipType == PONCHIP_PAS  ){
			ret =  REMOTE_PASONU_get_device_status(PonPortIdx,  onu_id, &status);
			if( ret != PAS_EXIT_OK  ){
				sys_console_printf("%s/port%d Onu %d :get Onu device status err %d \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), ret );
				return( RERROR );
				}
			sys_console_printf("Get %s/port%d Onu %d Device status :\r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
			
			sys_console_printf("   connect status ");
			if( status.connection == TRUE ){ sys_console_printf(": TRUE \r\n"); }
			else { sys_console_printf(": FALSE \r\n"); }
			
			sys_console_printf("   Oam link established: ");
			if( status.oam_link_established == TRUE ){ sys_console_printf(": TRUE \r\n"); }
			else { sys_console_printf(": FALSE \r\n"); }

			sys_console_printf("   authorization state: ");
			if( status.authorization_state == TRUE ){ sys_console_printf(": TRUE \r\n"); }
			else { sys_console_printf(": FALSE \r\n"); }

			sys_console_printf("   pon loopback: ");
			if( status.pon_loopback == TRUE ){ sys_console_printf(": TRUE \r\n"); }
			else { sys_console_printf(": FALSE \r\n"); }

			sys_console_printf("   mac addr: %02x ", status.mac_addr[0]);
			for(i=1;i<BYTES_IN_MAC_ADDRESS; i++ )
				sys_console_printf("-%02x", status.mac_addr[i] );
			sys_console_printf("\r\n   Llid : %d \r\n", status.onu_llid );
			return( ROK );			
			}
		else if( PonChipType == PONCHIP_GW ){
			return( ROK );
			}
		else if( PonChipType == PONCHIP_TK ){
			return( ROK );
			}
		return( RERROR );		
		}

	return( RERROR );

}	
#endif


/*** the follow is for Get Onu Device Info by OAM channel ***/

/*****************************************************
 *
 *    Function: OnuWaitForGetEUQInit()
 *
 *    Param:   none
 *               
 *    Desc:   Initialize the OnuWaitForGetEUQ for zero
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int  OnuWaitForGetEUQInit()
{
	OnuWaitForGetEUQ.PreNode = NULL;
	OnuWaitForGetEUQ.NextNode = NULL;
	OnuEUQDataSemId = VOS_SemMCreate( VOS_SEM_Q_PRIORITY );
	OnuEUQCommSemId = VOS_SemBCreate(VOS_SEM_Q_FIFO,VOS_SEM_FULL);
	OnuEUQRecvSemId = VOS_SemBCreate( VOS_SEM_Q_FIFO, VOS_SEM_EMPTY);

	CommOltMsgRvcCallbackInit((unsigned char )GW_CALLBACK_EUQINFO, (void *)/*OnuEUQRecvMsgCallBack_New*/Oam_Session_RecieveCallBack);
	
	return( ROK );
}

/*****************************************************
 *
 *    Function: AddOneNodeToGetEUQ( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
 *               
 *    Desc:   when onu is register, call this function to add one node to the Queue;
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int ThisIsTheNode(short int PonPortIdx, short int OnuIdx, OnuIdentifier_S *CurNode)
{
	CHECK_ONU_RANGE

	if( CurNode == NULL ) return( RERROR );

	if(( PonPortIdx == CurNode->PonPortIdx ) &&( OnuIdx == CurNode->OnuIdx )) return( ROK );
	else return( RERROR );
}

/* modified by xieshl 20111130, 读ONU设备信息，不受次数限制 */
int  Send_Onu_EUQ_Msg( short int PonPortIdx, short int OnuIdx )
{
	unsigned long aulMsg[4] = { MODULE_ONU, FC_ONU_EUQ_INFO, 0, 0};
	unsigned char *pBuf;
	
	CHECK_ONU_RANGE;
		
	/*GetOnuEUQInfo(PonPortIdx, OnuIdx );
	return( ROK );*//*在这里已经获取到ONU 的信息并返回了*/

	aulMsg[2] =(unsigned long) PonPortIdx;
	aulMsg[3] = (unsigned long )OnuIdx;
	aulMsg[2] = ( aulMsg[2] << 16 ) + aulMsg[3];

	/* 5 modified by chenfj 2007/03/15 
	3795.更改下面的光路拓扑结果后，发现有些ONU的状态是Down的，但能上网
	将消息向队列中的发送顺序由MSG_PRI_URGENT 改为MSG_PRI_NORMAL 
	*/
	pBuf = (unsigned char *)VOS_Malloc(8, MODULE_ONU);
	if( pBuf == NULL )
	{
		ASSERT(0);
		return( RERROR );
	}
	aulMsg[3] = (unsigned long)pBuf;
	pBuf[0] = GET_ONU_SYS_INFO_REQ;
	*(LONG *)&pBuf[4] = 0;

	if( VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL/*MSG_PRI_URGENT*/) != VOS_OK )
	{
		VOS_ASSERT(0);
		VOS_Free(pBuf);
		return( RERROR );
	}

	return( ROK );

}
/* modified by chenfj 2006/12/15
	ONU不停的离线注册，可能导致OLT出现问题的一个隐患；
	将数据传递由链表改为消息传递
*/
int  AddOneNodeToGetEUQ( short int PonPortIdx, short int OnuIdx )
{
#if 0
	unsigned long aulMsg[4] = { MODULE_ONU, FC_ONU_EUQ_INFO, 0, 0};
	int flag = 0x11;
	unsigned char *pBuf;
	int OnuEntryIdx;
	UCHAR GetEUQflag, GetEUQCounter, MacAddr[6];
	
	CHECK_ONU_RANGE
		
	/*GetOnuEUQInfo(PonPortIdx, OnuIdx );
	return( ROK );*//*在这里已经获取到ONU 的信息并返回了*/

	aulMsg[2] =(unsigned long) PonPortIdx;
	aulMsg[3] = (unsigned long )OnuIdx;
	aulMsg[2] = ( aulMsg[2] << 16 ) + aulMsg[3];

	OnuEntryIdx = PonPortIdx*MAXONUPERPON + OnuIdx;

	ONU_MGMT_SEM_TAKE;
	GetEUQflag = OnuMgmtTable[OnuEntryIdx].GetEUQflag;
	GetEUQCounter = OnuMgmtTable[OnuEntryIdx].GetEUQCounter;
	VOS_MemCpy( MacAddr, OnuMgmtTable[OnuEntryIdx].DeviceInfo.MacAddr, 6 );
	ONU_MGMT_SEM_GIVE;

	if( GetEUQflag ) 
		return ( ROK );
	
	if( GetEUQCounter >= MAX_GETEUQINFO_COUNTER )
	{
		short int Llid;
		Llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);	
		if( Llid == INVALID_LLID ) return( RERROR );
		AddPendingOnu( PonPortIdx, OnuIdx, Llid, MacAddr );
		sys_console_printf("\r\n  Get ONU device info from pon%d/%d onu%d Timeout\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx +1));

		return( ROK );
	}
	
	if( flag == 0x11 )
	{
		/* 5 modified by chenfj 2007/03/15 
		3795.更改下面的光路拓扑结果后，发现有些ONU的状态是Down的，但能上网
		将消息向队列中的发送顺序由MSG_PRI_URGENT 改为MSG_PRI_NORMAL 
		*/
		pBuf = (unsigned char *)VOS_Malloc(8, MODULE_ONU);
		if( pBuf == NULL )
		{
			ASSERT(0);
			return( RERROR );
		}
		aulMsg[3] = (unsigned long)pBuf;
		pBuf[0] = GET_ONU_SYS_INFO_REQ;
		*(int *)&pBuf[4] = 0;
	
		if( VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL/*MSG_PRI_URGENT*/) != VOS_OK )
		{
			VOS_ASSERT(0);
			/*sys_console_printf("error: VOS send message err(AddOneNodeToGetEUQ())\r\n" );*/
			VOS_Free(pBuf);
			/*DelOneNodeFromGetEUQ(PonPortIdx, OnuIdx );*/
			return( RERROR );
		}
		
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[OnuEntryIdx].GetEUQflag = TRUE;
		OnuMgmtTable[OnuEntryIdx].GetEUQCounter++;
		ONU_MGMT_SEM_GIVE;
	}
#else
	int flag = 0x11;
	/*unsigned char *pBuf;*/
	int OnuEntryIdx;
	UCHAR GetEUQflag, GetEUQCounter, MacAddr[6];
	
	CHECK_ONU_RANGE;
		
	/*GetOnuEUQInfo(PonPortIdx, OnuIdx );
	return( ROK );*//*在这里已经获取到ONU 的信息并返回了*/

	OnuEntryIdx = PonPortIdx*MAXONUPERPON + OnuIdx;

	ONU_MGMT_SEM_TAKE;
	GetEUQflag = OnuMgmtTable[OnuEntryIdx].GetEUQflag;
	GetEUQCounter = OnuMgmtTable[OnuEntryIdx].GetEUQCounter;
	VOS_MemCpy( MacAddr, OnuMgmtTable[OnuEntryIdx].DeviceInfo.MacAddr, 6 );
	ONU_MGMT_SEM_GIVE;

	if( GetEUQflag ) 
		return ( ROK );
	
	if( GetEUQCounter >= MAX_GETEUQINFO_COUNTER )
	{
		short int Llid;
		Llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);	
		if( Llid == INVALID_LLID ) return( RERROR );
		AddPendingOnu( PonPortIdx, OnuIdx, Llid, MacAddr, 0 );
		sys_console_printf("\r\n  Get ONU device info from pon%d/%d onu%d Timeout\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx +1));

		return( ROK );
	}
	
	if( flag == 0x11 )
	{
		Send_Onu_EUQ_Msg(PonPortIdx, OnuIdx);
		
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[OnuEntryIdx].GetEUQflag = TRUE;
		OnuMgmtTable[OnuEntryIdx].GetEUQCounter++;
		ONU_MGMT_SEM_GIVE;
	}
#endif

	return( ROK );
}


/*****************************************************
 *
 *    Function: DelOneNodeToGetEUQ( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
 *               
 *    Desc:   when onu is Deregister, or Pon chip is down , call this function to del one node from the Queue;
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
/* modified by chenfj 2006/12/15
	ONU不停的离线注册，可能导致OLT出现问题的一个隐患；
*/
int DelOneNodeFromGetEUQ( short int PonPortIdx, short int OnuIdx )
{
	CHECK_ONU_RANGE

	/*OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].WaitGetInfo = FALSE;*/

	return( ROK );
}

int DelOneNodeFromGetEUQ_bak( short int PonPortIdx, short int OnuIdx )
{
	OnuIdentifier_S *PreNode,  *CurNode, *NextNode;
	bool  Searchedflag = FALSE;;

	CHECK_ONU_RANGE

#if 0
	VOS_SemTake( OnuEUQDataSemId, WAIT_FOREVER );
#endif

	CurNode = &OnuWaitForGetEUQ;

	while( CurNode->NextNode != NULL )
		{
		CurNode = CurNode->NextNode;
		if( ThisIsTheNode(PonPortIdx, OnuIdx, CurNode) == ROK )
			{
			Searchedflag = TRUE;
			break;
			}
		}
	
	if( Searchedflag == TRUE )
		{
		PreNode = CurNode->PreNode;
		PreNode->NextNode = CurNode->NextNode;
		if( CurNode->NextNode != NULL )
			{
			NextNode = CurNode->NextNode;
			NextNode->PreNode = PreNode;
			}		
		VOS_Free((void *)CurNode);		
		}
#if 0
	VOS_SemGive( OnuEUQDataSemId );
#endif
	return( ROK );

}

/*****************************************************
 *
 *    Function: DisplayAllNode()
 *
 *    Param:   none
 *               
 *    Desc:   display all node in the queue, sorted by pon; only for debugging use
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 #if 0	/* removed by xieshl 20091216 */
int  DisplayAllNode()
{
	short int PonPortIdx;
	OnuIdentifier_S   *CurNode;
	int counter;
	
	sys_console_printf("\r\n  display all Onu to wait for get EUQ info\r\n");
	for(PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx ++ )
		{
		sys_console_printf("\r\n  %s/port%d :\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));

		CurNode = &OnuWaitForGetEUQ;
		counter= 0;
		
		while( CurNode->NextNode != NULL )
			{
			CurNode = CurNode->NextNode;
			if( CurNode->PonPortIdx == PonPortIdx )
				{
				sys_console_printf("%02d ", (CurNode->OnuIdx+1) );
				counter++;
				if(( counter % 16) == 0 ) sys_console_printf("\r\n");
				}
			}
		sys_console_printf("\r\n");
		}
	return( ROK );
}

int  DisplayNode(short int PonPortIdx )
{
	OnuIdentifier_S  *CurNode;
	int counter;
	
	CHECK_PON_RANGE
		
	sys_console_printf("\r\n  display all Onu to wait for get EUQ info\r\n");
	
	sys_console_printf("\r\n  %s/port%d :\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));

	CurNode = &OnuWaitForGetEUQ;
	counter= 0;
		
	while( CurNode->NextNode != NULL )
		{
		CurNode = CurNode->NextNode;
		if( CurNode->PonPortIdx == PonPortIdx )
			{
			sys_console_printf("%02d ", (CurNode->OnuIdx+1) );
			counter++;
			if(( counter % 16) == 0 ) sys_console_printf("\r\n");
			}
		}
	sys_console_printf("\r\n");
	
	return( ROK );
}
#endif

/*****************************************************
 *
 *    Function: V2R1_SendMsgToONU( short int PonPortIdx, short int OnuIdx, char  MsgType, unsigned char *DataBuf, int len)
 *
 *    Param:  short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
 *                char MsgType  -- 这是一个子消息类型，从属于 ONU 设备信息，如查询ONU设备信息，或设置ONU设备信息
 *                unsigned char *DataBuf -- 数据指针 ，当设置ONU设备信息时有效
 *                int len -- 数据长度；当设置ONU设备信息时有效               
 *    Desc:   将消息以扩展私有OAM帧的形式发送到ONU
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 #if 0
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
int V2R1_SendMsgToONU( short int PonPortIdx, short int OnuIdx, char  MsgType, unsigned char *pBuf, int length)
{
	unsigned long aulMsg[4] = { MODULE_ONU, FC_ONU_EUQ_INFO, 0, 0};
	unsigned char *SendBuf = NULL;

	aulMsg[2] = PonPortIdx;
	aulMsg[3] = OnuIdx;

	aulMsg[2] = (aulMsg[2] << 16 ) + aulMsg[3];

	if(( length != 0 ) && ( pBuf == NULL )) 
		return( RERROR );
	/*
	if(( MsgType == GET_ONU_SYS_INFO_REQ ) || (MsgType == SET_ONU_SYS_INFO_REQ ) || ( MsgType == SYNC_ONU_SYS_TIME_REQ )) 
		aulMsg[1]  = FC_ONU_EUQ_INFO;
	else aulMsg[1] = FC_ONU_TDM_SERVICE ;
	*/
	switch ( MsgType )
		{
		case SET_ONU_VOICE_MAC_REQ:
		case GET_ONU_VOICE_MAC_REQ:
		case SET_ONU_VOICE_VLAN_REQ:
		case GET_ONU_VOICE_VLAN_REQ:
		case SET_ONU_VOICE_POTS_REQ:
		case GET_ONU_VOICE_POTS_REQ:
		case GET_ONU_VOICE_POTS_STATUS_REQ:
		case SET_ONU_VOICE_LOOP_REQ:
		case GET_ONU_VOICE_LOOP_REQ:
		case SET_ONU_VOICE_EXT_EUQ_REQ:
		case SET_ONU_VOICE_SERVICE_REQ:
		case GET_ONU_VOICE_SERVICE_REQ:
			aulMsg[1] = FC_ONU_TDM_SERVICE ;
			break;
		default:
			return( RERROR );
		}
	
	SendBuf = (unsigned char *)VOS_Malloc((length+8) , MODULE_ONU );
	if( SendBuf == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}
	
	if( aulMsg[1] == FC_ONU_TDM_SERVICE )
		{
		if( DEBUG_ONU_TDM_VOICE  == V2R1_ENABLE )
			{
			int  i = 0;
			sys_console_printf("Send msg to onu %d/%d\r\n", PonPortIdx, (OnuIdx+1));
			sys_console_printf("Msgtype=%d,PDU len=%d\r\n", MsgType, length);
			if( length != 0 )
				{
				while ( i < length )
					{
					sys_console_printf("%02x ", pBuf[i]);
					i ++;
					if( (i % 16 ) == 0)
						sys_console_printf("\r\n");
					}
				}
			sys_console_printf("\r\n");
			}
		}
	
	aulMsg[3] = (unsigned long)SendBuf;
	SendBuf[0] = MsgType;
	*(unsigned int *)&SendBuf[4] = length;
	if(length != 0 )
		VOS_MemCpy( &SendBuf[8], pBuf, length );

	if( VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL/*MSG_PRI_URGENT*/) != VOS_OK )
			{
			VOS_ASSERT(0);
			sys_console_printf("error: VOS send message err(SendMsgToOnu)\r\n" );
			VOS_Free(SendBuf);
			/*DelOneNodeFromGetEUQ(PonPortIdx, OnuIdx );*/
			return( RERROR );
			}

	return( ROK );

}
#endif
#endif
/*  modified by chenfj 2007-11-31
	问题单#5664: SNMP任务优先级低于OAM通信任务，导致在发送消息后，还没将发送标志置位，就收到了返回来的响应；将这段程序用VOS_TaskLock()锁存也不解决问题；最后解决是先将发送标志置位，然后再发送消息
*/
static int EQU_SendMsgToOnu_1( short int PonPortIdx, short int OnuIdx, char  MsgType, unsigned char *pBuf, int length)
{
	unsigned char sessionId[8] = {0};
	unsigned short int len;
	unsigned char Databuf[EUQ_MAX_OAM_PDU];
	short int Llid;

	VOS_MemSet( Databuf, 0, sizeof(Databuf) );
	VOS_MemSet(sessionId, 0, sizeof( sessionId ));

	if(( MsgType == GET_ONU_SYS_INFO_REQ ) ||(MsgType == SET_ONU_SYS_INFO_REQ  ))
		{
		CHECK_ONU_RANGE;

		/*	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_DOWN )
		if(GetOnuOperStatus_1(PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_DOWN ) 
			{
			sys_console_printf("Onu%d/%d is off-line\r\n",PonPortIdx, (OnuIdx+1));
			return( RERROR );
			}
		*/
		Llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	
		if( Llid == INVALID_LLID ) 
			{
			if( EVENT_REGISTER ==V2R1_ENABLE)
			sys_console_printf("Get Onu%d/%d llid err\r\n",PonPortIdx, (OnuIdx+1));
			return( RERROR );
			}

		if(MsgType == GET_ONU_SYS_INFO_REQ )	
			{
			if( EVENT_REGISTER ==V2R1_ENABLE)
				{
				sys_console_printf("  %s/port%d onu %d registered, send Get EUQ, len=%d \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1),length);
				}
			
			/*VOS_TaskLock();*/
			RecvMsgFromOnuLen=0;
			WaitOnuEUQMsgFlag = V2R1_ONU_EUQ_MSG_WAIT;
			ONUEUQMsgWaitMsgType = MsgType;
			ONUEUQMsgWaitPonPortIdx = PonPortIdx;
			ONUEUQMsgWaitOnuIdx = OnuIdx;
			
			Databuf[OAMMSG_MSGTYPESTART] = MsgType;		
			len = 1;		
			if(Comm_EUQ_info_request_transmit(PonPortIdx, (OnuIdx+1), Databuf, len, sessionId ) != ROK)
				return(RERROR);
			/*VOS_TaskUnlock();*/
			}

		else if( MsgType == SET_ONU_SYS_INFO_REQ )
			{
			if ((pBuf == NULL ) ||(length == 0)) return( RERROR );
			if( EVENT_EUQ==V2R1_ENABLE)
				{
				sys_console_printf("%s/port%d onu%d device info changed, send set device info msg to onu, len=%d \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1), length);
				}	
			
			/*VOS_TaskLock();*/
			RecvMsgFromOnuLen = 0;
			WaitOnuEUQMsgFlag = V2R1_ONU_EUQ_MSG_WAIT;
			ONUEUQMsgWaitMsgType = MsgType;
			ONUEUQMsgWaitPonPortIdx = PonPortIdx;
			ONUEUQMsgWaitOnuIdx = OnuIdx;
			
			Databuf[OAMMSG_MSGTYPESTART] = MsgType;
			if( length > EUQ_MAX_OAM_PDU ) len = EUQ_MAX_OAM_PDU;
			
			VOS_MemCpy( &Databuf[OAMMSG_MSGTYPESTART+OAMMSG_MSGTYPELEN], pBuf, length );

			len = length + 1;		
			if(Comm_EUQ_info_request_transmit(PonPortIdx, (OnuIdx+1), Databuf, len , sessionId) != ROK)
				return(RERROR);
			/*VOS_TaskUnlock();*/
			}
		
		/*VOS_MemSet( (char *)&RecvMsgFromOnu[0], 0, sizeof( RecvMsgFromOnu ));*/
		return( ROK );
		}	
	else if( MsgType == SYNC_ONU_SYS_TIME_REQ  )
		{
		Date_S  CurrentTime;
		CHECK_PON_RANGE;
			
		if( OnuIdx < MAXONUPERPON ) 
			{
			CHECK_ONU_RANGE;
			if(GetOnuOperStatus_1(PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_DOWN ) return( RERROR );
			}
		
		/*VOS_TaskLock();*/
		GetSysCurrentTime( &CurrentTime );

		Databuf[SYNC_ONU_SYS_TIME_REQ_TYPE] = MsgType;
		*(short int *)&Databuf[SYNC_ONU_SYS_TIME_REQ_YEAR] = CurrentTime.year;
		Databuf[SYNC_ONU_SYS_TIME_REQ_MONTH] = CurrentTime.month;
		Databuf[SYNC_ONU_SYS_TIME_REQ_DAY] = CurrentTime.day;
		Databuf[SYNC_ONU_SYS_TIME_REQ_HOUR] = CurrentTime.hour;
		Databuf[SYNC_ONU_SYS_TIME_REQ_MINUTE] = CurrentTime.minute;
		Databuf[SYNC_ONU_SYS_TIME_REQ_SECOND] = CurrentTime.second;
		len = SYNC_ONU_SYS_TIME_REQ_SECOND+1;
		/*
		RecvMsgFromOnuLen = 0;
		WaitOnuEUQMsgFlag = V2R1_ONU_EUQ_MSG_WAIT;
		ONUEUQMsgWaitMsgType = MsgType;
		ONUEUQMsgWaitPonPortIdx = PonPortIdx;
		ONUEUQMsgWaitOnuIdx = OnuIdx;
		*/
		if( EVENT_EUQ==V2R1_ENABLE)
			{
			sys_console_printf("send SYS time to %s/port%d onu%d,len=%d \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1), len);
			sys_console_printf("year %d month %d day %d hour %d min %d second %d\r\n", CurrentTime.year, CurrentTime.month, CurrentTime.day, CurrentTime.hour, CurrentTime.minute, CurrentTime.second );
			}	
		if( OnuIdx < MAXONUPERPON )
			{
			Llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	
			if( Llid == INVALID_LLID ) 
				{
				if( EVENT_REGISTER ==V2R1_ENABLE)
				sys_console_printf("Get Onu%d/%d llid err\r\n",PonPortIdx, (OnuIdx+1));
				return( RERROR );
				}
			
			if(Comm_EUQ_systime_Require_transmit( PonPortIdx, (OnuIdx+1), Databuf, len , sessionId) != ROK)
				return(RERROR);
			}
		else{ 
			if(Comm_EUQ_SCB_systemTime_transmit( PonPortIdx, Databuf, len , sessionId) != ROK)
				return(RERROR);
			}
		/*VOS_TaskUnlock();*/
		
		return( ROK );
		}

/*#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES ) */
		else /*if ( MsgType == FC_ONU_TDM_SERVICE )*/
			{
			CHECK_ONU_RANGE

			/*	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_DOWN )*/
			if(GetOnuOperStatus_1(PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_DOWN ) 
				{
				if( EVENT_REGISTER ==V2R1_ENABLE)
				sys_console_printf("Onu%d/%d is off-line\r\n",PonPortIdx, (OnuIdx+1));
				return( RERROR );
				}
			Llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	
			if( Llid == INVALID_LLID ) 
				{
				if( EVENT_REGISTER ==V2R1_ENABLE)
				sys_console_printf("Get Onu%d/%d llid err\r\n",PonPortIdx, (OnuIdx+1));
				return( RERROR );
				}
			
			if( EVENT_REGISTER ==V2R1_ENABLE)
				{
				sys_console_printf("onu %d/%d voice service\r\n",PonPortIdx,(OnuIdx+1));
				}
			
			/*VOS_TaskLock();*/
			RecvMsgFromOnuLen=0;
			WaitOnuEUQMsgFlag = V2R1_ONU_EUQ_MSG_WAIT;
			ONUEUQMsgWaitMsgType = MsgType;
			ONUEUQMsgWaitPonPortIdx = PonPortIdx;
			ONUEUQMsgWaitOnuIdx = OnuIdx;

			if(Comm_EUQ_info_request_transmit(PonPortIdx, (OnuIdx+1), pBuf, length, sessionId ) != ROK)
				return(RERROR);
			/*VOS_TaskUnlock();*/

		return( ROK );
		}

/*#endif*/	
	return( RERROR );
}
static int EQU_SendMsgToOnu_2( short int PonPortIdx, short int OnuIdx, char  MsgType, unsigned char *pBuf, int length)
{
	unsigned char sessionId[8] = {0};
	unsigned short int len;
	unsigned char Databuf[EUQ_MAX_OAM_PDU];
	short int Llid;

	VOS_MemSet( Databuf, 0, sizeof(Databuf) );
	VOS_MemSet(sessionId, 0, sizeof( sessionId ));
    CHECK_ONU_RANGE;

    Llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
    if( Llid == INVALID_LLID ) 
    {
        if( EVENT_REGISTER ==V2R1_ENABLE)
            sys_console_printf("Get Onu%d/%d llid err\r\n",PonPortIdx, (OnuIdx+1));
        return( RERROR );
    }
    
	if(( MsgType == GET_ONU_SYS_INFO_REQ ) ||(MsgType == SET_ONU_SYS_INFO_REQ))
	{

		if(MsgType == GET_ONU_SYS_INFO_REQ )	
		{
			if( EVENT_REGISTER ==V2R1_ENABLE)
			{
				sys_console_printf("  %s/port%d onu %d registered, send Get EUQ, len=%d \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1),length);
			}
			
			/*VOS_TaskLock();*/
            OnuEvent_Set_WaitOnuEUQMsgFlag(PonPortIdx, OnuIdx, V2R1_ONU_EUQ_MSG_WAIT);

			Databuf[OAMMSG_MSGTYPESTART] = MsgType;		
			len = 1;		
			if(Comm_EUQ_info_request_transmit(PonPortIdx, (OnuIdx+1), Databuf, len, sessionId ) != ROK)
				return(RERROR);
			return ROK;
		}
		else
        {
			if ((pBuf == NULL ) ||(length == 0)) 
                return( RERROR );
			if( EVENT_EUQ==V2R1_ENABLE)
            {
				sys_console_printf("%s/port%d onu%d device info changed, send set device info msg to onu, len=%d \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1), length);
            }	
			
			/*VOS_TaskLock();*/
			/*RecvMsgFromOnuLen = 0;
			WaitOnuEUQMsgFlag = V2R1_ONU_EUQ_MSG_WAIT;
			ONUEUQMsgWaitMsgType = MsgType;
			ONUEUQMsgWaitPonPortIdx = PonPortIdx;
			ONUEUQMsgWaitOnuIdx = OnuIdx;*/
            OnuEvent_Set_WaitOnuEUQMsgFlag(PonPortIdx, OnuIdx, V2R1_ONU_EUQ_MSG_WAIT);
			Databuf[OAMMSG_MSGTYPESTART] = MsgType;
			if( length > EUQ_MAX_OAM_PDU ) len = EUQ_MAX_OAM_PDU;
			
			VOS_MemCpy( &Databuf[OAMMSG_MSGTYPESTART+OAMMSG_MSGTYPELEN], pBuf, length );

			len = length + 1;		
			if(Comm_EUQ_info_request_transmit(PonPortIdx, (OnuIdx+1), Databuf, len , sessionId) != ROK)
				return(RERROR);
			return ROK;            
			/*VOS_TaskUnlock();*/
		}
	}
    else if( MsgType == SYNC_ONU_SYS_TIME_REQ  )
	{
    		Date_S  CurrentTime;
    		CHECK_PON_RANGE;
    			
    		if( OnuIdx < MAXONUPERPON ) 
    		{
    			CHECK_ONU_RANGE;
    			if(GetOnuOperStatus_1(PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_DOWN ) 
                    return( RERROR );
    		}
		
    		/*VOS_TaskLock();*/
    		GetSysCurrentTime( &CurrentTime );

    		Databuf[SYNC_ONU_SYS_TIME_REQ_TYPE] = MsgType;
    		*(short int *)&Databuf[SYNC_ONU_SYS_TIME_REQ_YEAR] = CurrentTime.year;
    		Databuf[SYNC_ONU_SYS_TIME_REQ_MONTH] = CurrentTime.month;
    		Databuf[SYNC_ONU_SYS_TIME_REQ_DAY] = CurrentTime.day;
    		Databuf[SYNC_ONU_SYS_TIME_REQ_HOUR] = CurrentTime.hour;
    		Databuf[SYNC_ONU_SYS_TIME_REQ_MINUTE] = CurrentTime.minute;
    		Databuf[SYNC_ONU_SYS_TIME_REQ_SECOND] = CurrentTime.second;
    		len = SYNC_ONU_SYS_TIME_REQ_SECOND+1;
            
    		if( EVENT_EUQ==V2R1_ENABLE)
    		{
    			sys_console_printf("send SYS time to %s/port%d onu%d,len=%d \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1), len);
    			sys_console_printf("year %d month %d day %d hour %d min %d second %d\r\n", CurrentTime.year, CurrentTime.month, CurrentTime.day, CurrentTime.hour, CurrentTime.minute, CurrentTime.second );
    		}	
    		if( OnuIdx < MAXONUPERPON )
    		{
    			Llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
    	
    			if( Llid == INVALID_LLID ) 
    			{
    				if( EVENT_REGISTER ==V2R1_ENABLE)
    				sys_console_printf("Get Onu%d/%d llid err\r\n",PonPortIdx, (OnuIdx+1));
    				return( RERROR );
    			}
    			
    			if(Comm_EUQ_systime_Require_transmit( PonPortIdx, (OnuIdx+1), Databuf, len , sessionId) != ROK)
    				return(RERROR);
    		}
    		else
    		{
    			if(Comm_EUQ_SCB_systemTime_transmit( PonPortIdx, Databuf, len , sessionId) != ROK)
    				return(RERROR);
    		}   		
    		return( ROK );
    }
	return( RERROR );
}
/*added by luh 2012-9-17*/
int EQU_SendMsgToOnu_ASYNC(short int PonPortIdx, short int OnuIdx, char  MsgType, unsigned char *pBuf, int length)
{
	int GetSemId;
	int  ret = VOS_ERROR;
    ret = Oam_Session_Send(PonPortIdx, OnuIdx, MsgType, OAM_ASYNC, OAM_NOTI_VIA_QUEUE, g_Onu_Queue_Id, NULL, pBuf, length, NULL, NULL);
	return ret;
}

int EQU_SendMsgToOnu_New(short int PonPortIdx, short int OnuIdx, char  MsgType, unsigned char *pBuf, int length)
{
	int GetSemId;
	int  ret;
	
	GetSemId = VOS_SemTake( OnuEUQCommSemId, WAIT_FOREVER );
	if(GetSemId == VOS_ERROR )
    {
		ASSERT(0);
		return ( RERROR );
    }

	/*GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);*/
	ret = EQU_SendMsgToOnu_2( PonPortIdx, OnuIdx, MsgType,pBuf, length);
	if(ret != ROK)
    {
		VOS_SemGive(OnuEUQCommSemId);
		return(RERROR);
    }
    VOS_SemGive(OnuEUQCommSemId);

	return( ret);
}

/* modified by chenfj 2008-3-5
     修改ONU 设备管理OAM 通信, 使用信号来保证每次OAM 通信的完整( 发/ 收完成).
     可供多个不同任务调用, 支持ONU 管理和ONU 语音,ONU 自动配置等
    */
int EQU_SendMsgToOnu( short int PonPortIdx, short int OnuIdx, char  MsgType, unsigned char *pBuf, int length)
{
	int GetSemId;
	int  ret;
	
	GetSemId = VOS_SemTake( OnuEUQCommSemId, WAIT_FOREVER );
	if(GetSemId == VOS_ERROR )
		{
		ASSERT(0);
		return ( RERROR );
		}

	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	ret = EQU_SendMsgToOnu_1( PonPortIdx, OnuIdx, MsgType,pBuf, length);
	if(ret != ROK)
		{
		VOS_SemGive(OnuEUQCommSemId);
		return(RERROR);
		}
	if( MsgType != SYNC_ONU_SYS_TIME_REQ )
		{
		GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
		if( GetSemId != RERROR )
			ret  = ROK ;
		else ret = RERROR;
		}
	else ret = ROK;

	VOS_SemGive(OnuEUQCommSemId);

	return( ret);
}

/*****************************************************
 *
 *    Function: OnuAppImageVersionComp(short int PonPortIdx,short int OnuIdx)
 *
 *    Param:  short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
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
#ifdef  ONU_VERSION_COMPARE
#endif

int OnuAppImageVersionComp(short int PonPortIdx, short int OnuIdx)
{
	char *Type = NULL;
	char OnuType[ONU_TYPE_LEN+2]= {'\0'};
	char OnuFlashVersion[ONU_VERSION_LEN + 1] = {'\0'};
	char OnuCurVersion[ONU_VERSION_LEN +1] = {'\0'};
	char *Version = NULL;
	int VersionLen = 0;
	int len=0;
	int ret= ROK;
	int offset=0, file_len=0, compress_flag=0, file_fact_len=0;
	
	CHECK_ONU_RANGE;

	len = 0;
	
	/*lRet = StartOnuSWUpdate(phyPonId, userOnuId ); */
	if( GetOnuSWUpdateStatus(PonPortIdx, OnuIdx ) == ONU_SW_UPDATE_STATUS_INPROGRESS )
	{
		if(EVENT_REGISTER == V2R1_ENABLE)
			sys_console_printf( " onu%d/%d/%d app image update already started\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx +1) );
		return (ROK);
	}
	if( GetOnuSWUpdateCtrl(PonPortIdx, OnuIdx ) == ONU_SW_UPDATE_DISABLE )
		{
		if(EVENT_REGISTER == V2R1_ENABLE)
			sys_console_printf(" onu%d/%d/%d app update is disabled\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx +1) );
		return (RERROR);
		}

	Type = &(OnuType[0]);
	ret = GetOnuTypeString(PonPortIdx, OnuIdx, Type, &len );
	if( ret == RERROR  )
		{
		if(EVENT_REGISTER == V2R1_ENABLE)
			sys_console_printf("Get onu%d/%d/%d type(string) err\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx +1) );
		return( RERROR );
		}

	ret = GetOnuSWVersion(PonPortIdx,  OnuIdx, OnuCurVersion, &VersionLen);
	if( ret == ERROR )
		{
		if(EVENT_REGISTER == V2R1_ENABLE)
			sys_console_printf("Get onu%d/%d/%d software version err\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx +1) );
		return( RERROR );
		}	
	Version = &(OnuCurVersion[0]);

	get_ONU_Info( (char *)&(OnuType[0]), &offset, &file_len, &compress_flag, &file_fact_len, (int*)&(OnuFlashVersion[0]) );
	Version = &(OnuFlashVersion[0]);
	/*sys_console_printf("flash software version:%s\r\n", Version );*/


	if(VOS_StrCmp(&(OnuCurVersion[0]), Version/*OnuFlashVersion */) != 0 )
	{
		/*unsigned long devIdx;
		int slot, port;
		
		slot = GetCardIdxByPonChip(PonPortIdx);
		port = GetPonPortByPonChip(PonPortIdx);
		devIdx = MAKEDEVID(slot, port, OnuIdx +1 );*/

		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].vty = NULL;
		ONU_MGMT_SEM_GIVE;
		
		/*sendOnuUpdateMsg(V2R1_UPDATE_SIGNAL_ONU,  devIdx );*/
		return( V2R1_VERSION_NO_CONSISTENT );
		/*
		SetOnuSwUpdateStatus(PonPortIdx, OnuIdx , ONU_SW_UPDATE_STATUS_INPROGRESS );

		sys_console_printf("send app image to %s/port%d onu%d Start.....\r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1) );
		UpdateOnuAppReq(PonPortIdx,(OnuIdx+1), Type , ONU_APP_NAME, NO_WAIT_RETURN);
		*/
	}
	else return( V2R1_VERSION_CONSISTENT );
	
}


/*****************************************************
 *
 *    Function: GetOnuEUQInfoHandle(short int PonPortIdx,short int OnuIdx)
 *
 *    Param:  short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
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
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES)
extern LONG onuAutoLoadOnuRequestCallback(ULONG devIdx);
#endif
extern LONG onuExtMgmt_OnuEquInfoCallback( ULONG devIdx, UCHAR *pBrdList );
/* modified by xieshl 20110106 */

static int OnuRegisterHandler_EUQ( SHORT PonPortIdx, SHORT OnuIdx, ULONG slot, ULONG port, int OnuEntry )
{
	/*short int i,j;*/

	CHECK_ONU_RANGE;

	/*OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_ONU_GT811; */
	
	/* 1 */		
	/*RecordOnuUpTime(  PonPortIdx,  OnuIdx);*/
	/*if(GetOnuRegisterData( PonPortIdx, OnuIdx ) != ROK ) return (RERROR );*/

	/* 2 */
#if 0	/* modified by xieshl 20111102, 有重复记录或漏记录的情况，不应防止这里 */
	PonPortTable[PonPortIdx].CurrOnu ++;
	PonPortTable[PonPortIdx].CurLLID ++;

	/*OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;*/
	i = OnuIdx & 0x07; /*OnuIdx % 8*/
	j = OnuIdx >> 3;   /*OnuIdx / 8*/
	PonPortTable[PonPortIdx].OnuCurStatus[j] = PonPortTable[PonPortIdx].OnuCurStatus[j] |( 1 << i ) ;
#endif
#if 0		
	if( OnuMgmtTable[OnuEntry].OperStatus == ONU_OPER_STATUS_UP)
		{
		OnuMgmtTable[OnuEntry].OperStatus = ONU_OPER_STATUS_DOWN;
		Trap_OnuDeregister(PonPortIdx, OnuIdx, PON_ONU_DEREGISTRATION_REPORT_TIMEOUT );
		}
#endif		
	/*OnuMgmtTable[OnuEntry].OperStatus = ONU_OPER_STATUS_UP;*/

    /*del by luh 2012-10-31， 等onu全部处理完毕状态才能变为up*/
	/*updateOnuOperStatusAndUpTime( OnuEntry, ONU_OPER_STATUS_UP );*/	/* modified by xieshl 20101220,问题单11740 */
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus = LLID_ENTRY_ACTIVE;
	ONU_MGMT_SEM_GIVE;

#if 0	
	/* 2' 向ONU发送系统时间*/
	SyncSysTimeToOnuMsg( PonPortIdx, OnuIdx );
#else
    SyncSysTimeToOnu_New(PonPortIdx, OnuIdx);
#endif
	
	/* 3mib port status */
	/*slot = GetCardIdxByPonChip( PonPortIdx );
	port = GetPonPortByPonChip( PonPortIdx );*/
	/*ONU_MGMT_SEM_TAKE;
	setOnuStatus( slot,  port, OnuIdx, ONU_ONLINE );
	ONU_MGMT_SEM_GIVE;*/

#ifdef STATISTIC_TASK_MODULE
	{ /* 4 statistics port status */
	if(OnuMgmtTable[OnuEntry].UsedFlag != ONU_USED_FLAG)
		StatsMsgOnuOnSend( PonPortIdx, OnuIdx, ONU_ONLINE );
	}
#endif

#if 0
	/*5: set the corresponding LLID bandwidth & policeing parameters*/
	ActiveOnuDownlinkBW( PonPortIdx,  OnuIdx);

	ActiveOnuUplinkBW( PonPortIdx, OnuIdx);
#endif

	/* 6: set alarm parameters */
	/*set_ONU_alarm_configuration( PonPortIdx, OnuIdx );*/

	/* 7: start encrypt if needed */
	/*if( OnuMgmtTable[OnuEntry].EncryptEnable == V2R1_ENABLE ) 
		OnuEncryptionOperation( PonPortIdx, OnuIdx, OnuMgmtTable[OnuEntry].EncryptDirection ) ;
	*/
	if( OnuMgmtTable[OnuEntry].EncryptEnable == V2R1_ENABLE ) 
		V2R1_OnuStartEncrypt(PonPortIdx, OnuIdx );

	
	/* added by chenfj 2007-8-9 
		支持GT813/GT816能在CT模式下注册，在GW模式下管理
		增加判断，若扩展OAM发现已完成，说明ONU为CT模式下的GT813，或GT816；
		此时不再上报ONU注册TRAP
		*/
	/*if(GetOnuExtOAMStatus( PonPortIdx,OnuIdx) != V2R1_ENABLE )*/
#if 0/* removed by xieshl 20110216 */
		{
		if( OnuMgmtTable[OnuEntry].RegisterFlag == ONU_FIRST_REGISTER_FLAG )
			{
			/* send onu first registration trap to NMS */
			Trap_OnuRegister( PonPortIdx, OnuIdx, ONU_FIRST_REGISTER_FLAG );
					
			OnuMgmtTable[OnuEntry].RegisterFlag = NOT_ONU_FIRST_REGISTER_FLAG;
			OnuMgmtTable[OnuEntry].UsedFlag = ONU_USED_FLAG;
			}
		else/* if( OnuMgmtTable[OnuEntry].RegisterFlag == NOT_ONU_FIRST_REGISTER_FLAG )*/
			{
			/* send onu Re_registration trap to NMS */
			Trap_OnuRegister( PonPortIdx, OnuIdx, NOT_ONU_FIRST_REGISTER_FLAG );
			}
		}
#endif	
	/* 8: 记录ONU注册信息*/
	{
	Date_S LaunchDate;
	PonPortTable[PonPortIdx].OnuRegisterMsgCounter ++;
	VOS_MemCpy( (void *)PonPortTable[PonPortIdx].LastRegisterMacAddr, (void *)OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS );
	if( GetSysCurrentTime( &LaunchDate ) == RERROR ) return ( RERROR );
	VOS_MemCpy ((char *) &(PonPortTable[PonPortIdx].LastOnuRegisterTime), (char *)&(LaunchDate.year), sizeof( Date_S) );
	}
	
	/* 9: 设置ONU 支持的最大MAC 数 
	if( 	V2R1_NOT_CTC_STACK )
		SetOnuMaxMacNum( PonPortIdx, OnuIdx, 0, OnuMgmtTable[OnuEntry].LlidTable[0].MaxMAC );
	*/
	if( OnuMgmtTable[OnuEntry].FEC_Mode == STD_FEC_MODE_ENABLED )
		SetOnuFecMode(PonPortIdx, OnuIdx,STD_FEC_MODE_ENABLED);
	
	
	/* 10: 设置ONU数据包原MAC过滤
	EnableOnuSAMacFilter( PonPortIdx , OnuIdx );
	 11:设置ONU 数据包VLANID/ IP地址/原UDP端口/原TCP端口过滤
	EnableOnuFilter(PonPortIdx, OnuIdx );*/
	/* 12:  added by chenfj 2007-6-27 
	 		   ONU	 device  de-active,  增加ONU 去激活操作*/
	/*if(GetOnuTrafficServiceEnable( PonPortIdx, OnuIdx )  == V2R1_DISABLE )*/
	if(OnuMgmtTable[OnuEntry].TrafficServiceEnable  == V2R1_DISABLE )
		SetOnuTrafficServiceEnable( PonPortIdx, OnuIdx, V2R1_DISABLE ) ;
	     /* OnuMgt_SetOnuTrafficServiceEnable( PonPortIdx, OnuIdx, V2R1_DISABLE);*/

	/*  added by chenfj 2006/12/06
	set onu peer-to-peer  在ONU注册时，设置ONU  peer-to-peer 通信
	*/
#ifdef ONU_PEER_TO_PEER
	{
	int ret;
	short int p2pOnuIdx;
	
	EnableOnuPeerToPeerForward( PonPortIdx, OnuIdx );
	for( p2pOnuIdx= 0; p2pOnuIdx < MAXONUPERPON; p2pOnuIdx++)
		{
		if( OnuIdx == p2pOnuIdx )
			continue;
		ret = GetOnuPeerToPeerInOnuEntry(OnuEntry, p2pOnuIdx);	/*GetOnuPeerToPeer( PonPortIdx, OnuIdx, OnuIdx1 );*/
		if( ret == V2R1_ENABLE )
			EnableOnuPeerToPeer( PonPortIdx, OnuIdx, p2pOnuIdx);
		}
	}
#endif	
	/* modify by chenfj 2006/09/30*/
	/* #2624 问题ONU注册或离线时，控制台上没有提示了。*/
	/*if(GetOnuExtOAMStatus( PonPortIdx,OnuIdx) != V2R1_ENABLE )*/
		{
		/*if( EVENT_REGISTER == V2R1_ENABLE )
			sys_console_printf("\r\n  onu %d/%d/%d registered\r\n", slot, port, (OnuIdx+1) );*/
		}

	/* added by chenfj 2008-3-28
	     恢复ONU 上行业务优先级配置*/
	/*RestoreOnuUplinkVlanPriority(PonPortIdx, OnuIdx );*/
#if 0    
#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
	RestoreUplinkVlanManipulationToPon(PonPortIdx, OnuIdx );
#endif
#endif
	/* 配置ONU 语音业务*/
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	{
	unsigned long OnuDevIdx;
	bool SupportVoice = FALSE;

	if(get_gfa_sg_slotno())
		{
		OnuDevIdx = MAKEDEVID(slot, port, OnuIdx+1);
		if((OnuIsSupportVoice(OnuDevIdx, &SupportVoice) == ROK ) && ( SupportVoice == TRUE ))
			ConfigOnuVoiceService(OnuDevIdx);
		}
	else if(get_gfa_e1_slotno())
		{
		OnuDevIdx = MAKEDEVID(slot, port, OnuIdx+1);
		if(onuDevIdxIsSupportE1(OnuDevIdx) == ROK )
			RestoreOnuE1LinkAll(OnuDevIdx);
		}
	}
#endif
	/* 复位PON 芯片MAC 地址表
	ResetPonPortMacTable(PonPortIdx);*/
#ifdef ONU_PPPOE_RELAY
	if (g_PPPOE_relay == PPPOE_RELAY_ENABLE)
	{
		onuIdx_Is_Support_Relay(PonPortIdx, OnuIdx, RELAY_TYPE_PPPOE);
	}
#endif
#ifdef ONU_DHCP_RELAY
	if (DHCP_RELAY_ENABLE == g_DHCP_relay)
	{
		onuIdx_Is_Support_Relay(PonPortIdx, OnuIdx, RELAY_TYPE_DHCP);
	}
#endif
#if 0
#if( RPU_MODULE_IGMP_TVM == RPU_YES )
	Register_Tvm_Send();
#endif
#endif
	return( ROK );
}

/* modified by xieshl 20110609,  	64个ONU在多个PON口间切换，会出现ONU离线慢的情况，问题单12971, 12977 */
#define ONU_EUQ_STR_CPY( dInfoName, dInfo, sInfo, dLen, s_Len, maxLen ) \
{\
    int c = 0, sLen = s_Len;\
    char *p = sInfo;\
    while( c < sLen && *p >= 0x20 && *p <= 0x7f)\
    {\
        c++;\
        p++;\
    }\
        sLen = c;\
	if( sLen >= maxLen )\
	{\
        /* \
        ONU_MGMT_SEM_GIVE;\
        return RERROR;\*/ \
        if( EVENT_EUQ == V2R1_ENABLE)\
        {\
            sys_console_printf(" %s length %d is too long!\r\n", dInfoName, sLen );\
        }\
        dLen = maxLen-1; /* modified by xieshl 20111125, 要留出一个结束符 */\
        VOS_MemCpy( dInfo, sInfo, dLen );\
	}\
	else if( sLen > 0)\
	{\
		dLen = sLen;\
		VOS_MemCpy( dInfo, sInfo, dLen );\
	}\
	else\
	{\
		dLen = 1;\
		dInfo[0] = '-';\
	}\
	/*VOS_MemZero( &dInfo[sLen], maxLen - dLen );\*/ dInfo[dLen] = 0;\
	if( EVENT_EUQ == V2R1_ENABLE)\
	{\
		sys_console_printf(" %s(%d):%s\r\n", dInfoName, dLen, dInfo );\
	}\
}
#define ONU_EUQ_INT_CPY( dInfoName, dInfo, sInfo ) \
{\
	dInfo = sInfo;\
	if( EVENT_EUQ == V2R1_ENABLE)\
	{\
		sys_console_printf(" %s:%d\r\n", dInfoName, dInfo );\
	}\
}

#if 0
static int GetOnuEUQInfoHandle(short int PonPortIdx,short int OnuIdx)
{
	/*int i;*/
	LONG valLen;
	SHORT FixLen=0;
	SHORT DevType;
	ULONG slot, port;
	UCHAR *pVendor;
	UCHAR AutoConfig = NO_V2R1_ONU_AUTO_CONFIG_FLAG;

	char *pRxOamMsg;
	/*int ret = ROK;*/
	int onuEntry;
	
	CHECK_ONU_RANGE;

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	if( (EVENT_REGISTER ==V2R1_ENABLE) || (EVENT_EUQ == V2R1_ENABLE) )
		sys_console_printf( "\r\n Recv EUQ OAM Msg From onu %d/%d/%d, length=%d\r\n", slot, port, (OnuIdx +1), RecvMsgFromOnuLen );

	FixLen = OAMMSG_MSGTYPELEN + OAMMSG_DEVICETYPELEN + OAMMSG_OUILEN;
	if(RecvMsgFromOnuLen <= (FixLen /*+ OAMMSG_GET_INFO_LEN_MIN*/))
		return( RERROR );

	pRxOamMsg = RecvMsgFromOnu;

	/* OAM类型 */
	if( *pRxOamMsg != GET_ONU_SYS_INFO_RSP) return( RERROR );
	if( EVENT_EUQ == V2R1_ENABLE)
		sys_console_printf( " MsgType:%d\r\n", *pRxOamMsg);
	pRxOamMsg++;

	onuEntry = PonPortIdx*MAXONUPERPON +OnuIdx;

	ONU_MGMT_SEM_TAKE;

	/* ONU设备类型 */
	DevType = *(short int *)pRxOamMsg;
	if( (DevType < V2R1_OTHER) || (DevType >= V2R1_ONU_MAX) )  DevType = V2R1_OTHER;
	ONU_EUQ_INT_CPY("DeviceType", OnuMgmtTable[onuEntry].DeviceInfo.type, DevType );
	pRxOamMsg += 2;

	/* OUI */
	VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.OUI , pRxOamMsg, sizeof(OnuMgmtTable[0].DeviceInfo.OUI));
	if( EVENT_EUQ == V2R1_ENABLE)
		sys_console_printf( " OUI:%02x%02x%02x\r\n", pRxOamMsg[0], pRxOamMsg[1], pRxOamMsg[2] );
	pRxOamMsg += OAMMSG_OUILEN;

	/* 硬件版本信息 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Hardware Ver", OnuMgmtTable[onuEntry].DeviceInfo.HwVersion, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.HwVersionLen, valLen, MAXHWVERSIONLEN );
	pRxOamMsg += valLen;

	/* Boot版本 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Bootware Ver", OnuMgmtTable[onuEntry].DeviceInfo.BootVersion, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.BootVersionLen, valLen, MAXBOOTVERSIONLEN );
	pRxOamMsg += valLen;

	/* 软件版本 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Software Ver", OnuMgmtTable[onuEntry].DeviceInfo.SwVersion, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.SwVersionLen, valLen, MAXSWVERSIONLEN );
	pRxOamMsg += valLen;

	/* 固件版本 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Firmware Ver", OnuMgmtTable[onuEntry].DeviceInfo.FwVersion, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.FwVersionLen, valLen, MAXFWVERSIONLEN );
	pRxOamMsg += valLen;

	/* 名称 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Device Name", OnuMgmtTable[onuEntry].DeviceInfo.DeviceName, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.DeviceNameLen, valLen, MAXDEVICENAMELEN );
	pRxOamMsg += valLen;

	/* 描述 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Device Desc", OnuMgmtTable[onuEntry].DeviceInfo.DeviceDesc, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.DeviceDescLen, valLen, MAXDEVICEDESCLEN );
	pRxOamMsg += valLen;

	/* 位置 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Location", OnuMgmtTable[onuEntry].DeviceInfo.Location, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.LocationLen, valLen, MAXLOCATIONLEN );
	pRxOamMsg += valLen;

	/* 制造商 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	/* modified by chenfj 2008-5-9
	    修改ONU vendor 信息, 为适应不同OEM 及GW产品, 此信息不再从ONU 读取,
	    而是出自OLT 侧*/
	pVendor = typesdb_pruduct_onu_vendor_info();
	ONU_EUQ_STR_CPY( "VendorInfo", OnuMgmtTable[onuEntry].DeviceInfo.VendorInfo, pVendor, 
			OnuMgmtTable[onuEntry].DeviceInfo.VendorInfoLen, VOS_StrLen(pVendor), MAXVENDORINFOLEN );
	pRxOamMsg += valLen;

	/* added by chenfj 2007-11-1
	     ONU设备中增加序列号和生产日期等信息
	     需要考虑到兼容以前的ONU( 其设备信息中没有序列号和生产日期)*/
	/* SN */
	if( (pRxOamMsg - RecvMsgFromOnu) >= RecvMsgFromOnuLen )
	{
		valLen = 0;
	}
	else
	{
		valLen = *pRxOamMsg;
		pRxOamMsg++;
	}
	ONU_EUQ_STR_CPY( "Serial No", OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_No, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_NoLen, valLen, MAXSNLEN );
	pRxOamMsg += valLen;

	/* 生产日期 */
	if( (pRxOamMsg - RecvMsgFromOnu) >= RecvMsgFromOnuLen )
	{
		valLen = 0;
	}
	else
	{
		valLen = *pRxOamMsg;
		pRxOamMsg++;
	}
	ONU_EUQ_STR_CPY( "Product date", OnuMgmtTable[onuEntry].DeviceInfo.MadeInDate, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.MadeInDateLen, valLen, MAXDATELEN );
	pRxOamMsg += valLen;

	/* added by chenfj 2008-3-5
	     ONU设备中增加ONU 自动配置请求
	      需要考虑到兼容以前的ONU ( 其设备信息中没有自动配置请求)
	     */
	/* 自动配置请求选项 */
	if( (pRxOamMsg - RecvMsgFromOnu) >= RecvMsgFromOnuLen )
	{
		valLen = 0;
	}
	else
	{
		valLen = *pRxOamMsg;
		pRxOamMsg++;
	}
	/*if( SetOnuAutoConfigFlag( PonPortIdx, OnuIdx, (valLen ? *pRxOamMsg : NO_V2R1_ONU_AUTO_CONFIG_FLAG)) == RERROR )
		return RERROR;*/
	if( (valLen != 0) && (*pRxOamMsg == V2R1_ONU_AUTO_CONFIG_FLAG) )
	{
		AutoConfig = *pRxOamMsg;
	}
	/*OnuMgmtTable[onuEntry].DeviceInfo.AutoConfigFlag = AutoConfig;
	if( EVENT_EUQ == V2R1_ENABLE )
		sys_console_printf(" Auto config req(%d):%d\r\n", valLen, AutoConfig );*/
	ONU_EUQ_INT_CPY("Auto config req", OnuMgmtTable[onuEntry].DeviceInfo.AutoConfigFlag, AutoConfig );

	pRxOamMsg += valLen;

	ONU_MGMT_SEM_GIVE;

	/* added by chenfj 2008-6-12
	扩展多子卡ONU设备管理,增加子卡版本/状态等管理信息 */
	/* 支持槽位 */
	if( (pRxOamMsg - RecvMsgFromOnu) >= RecvMsgFromOnuLen )
	{
		valLen = 0;
	}
	else
	{
		valLen = *pRxOamMsg;
		onuExtMgmt_OnuEquInfoCallback( MAKEDEVID(slot, port, (OnuIdx+1)), pRxOamMsg );
		pRxOamMsg += valLen;
	}
	if( EVENT_EUQ == V2R1_ENABLE )
		sys_console_printf(" Max sub-slot(%d):", valLen+1 );


	/* send  onu register trap to NMS */
	OnuRegisterHandler_EUQ( PonPortIdx, OnuIdx, slot, port, onuEntry );

	/* ONU 软件运行版本比较*/
#ifdef ONU_VERSION_COMPARE
	/*ret = OnuAppImageVersionComp(PonPortIdx, OnuIdx);*/
#endif

#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES)
	if( AutoConfig == V2R1_ONU_AUTO_CONFIG_FLAG )
	{		
		ULONG DevIdx = MAKEDEVID(slot, port, OnuIdx +1);
		onuAutoLoadOnuRequestCallback(DevIdx);
	}
#endif

	/* ONU 语音配置
	{
	int slot, port;
	unsigned int  onuDevIdx;
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	onuDevIdx = slot * 10000 + port * 1000 + (OnuIdx +1);
	StartOnuTdmVoiceConfigByIdx(onuDevIdx);
	}
	*/

	/*  added by chenfj 2008-3-5
	    启动ONU 自动配置入口
	  */

	return( ROK );
}
#else
int GetOnuEUQInfoHandle_New(short int PonPortIdx, short int OnuIdx, char *pRxOamMsg)
{
    char *pReleaseRx = pRxOamMsg;
	LONG valLen;
	SHORT FixLen=0;
	SHORT DevType;
	ULONG slot, port;
	UCHAR *pVendor;
	UCHAR AutoConfig = NO_V2R1_ONU_AUTO_CONFIG_FLAG;
    short int RecvMsgLen = 0;
	/*int ret = ROK;*/
	int onuEntry;
	int onu_model;
	int current_status;
    UCHAR OnuAbilityType = 0;
    UCHAR OnuAbility = 0;
	CHECK_ONU_RANGE;


	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	onuEntry = PonPortIdx*MAXONUPERPON +OnuIdx;
    
    RecvMsgLen = *(short int *)pRxOamMsg;
	pRxOamMsg += 2;
    
	if( (EVENT_REGISTER ==V2R1_ENABLE) || (EVENT_EUQ == V2R1_ENABLE) )
		sys_console_printf( "\r\n Recv EUQ OAM Msg From onu %d/%d/%d, length=%d\r\n", slot, port, (OnuIdx +1), RecvMsgLen );

	FixLen = OAMMSG_MSGTYPELEN + OAMMSG_DEVICETYPELEN + OAMMSG_OUILEN;
	if(RecvMsgLen <= (FixLen /*+ OAMMSG_GET_INFO_LEN_MIN*/))
	{
        if(pReleaseRx)
            VOS_Free(pReleaseRx);        
		return( RERROR );
	}

	/* OAM类型 */
	if( *pRxOamMsg != GET_ONU_SYS_INFO_RSP) 
	{
        if(pReleaseRx)
            VOS_Free(pReleaseRx);        
        return( RERROR );
	}
	if( EVENT_EUQ == V2R1_ENABLE)
		sys_console_printf( " MsgType:%d\r\n", *pRxOamMsg);
	pRxOamMsg++;


	ONU_MGMT_SEM_TAKE;

	/* ONU设备类型 */
	DevType = *(short int *)pRxOamMsg;
	if( (DevType < V2R1_OTHER) || (DevType >= V2R1_ONU_MAX) )  
        DevType = V2R1_OTHER;
	ONU_EUQ_INT_CPY("DeviceType", OnuMgmtTable[onuEntry].DeviceInfo.type, DevType );
	pRxOamMsg += 2;

    if ( (0 == OnuMgmtTable[onuEntry].onu_model) && (0 != (onu_model = GetOnuCtcRegisterId(DevType))) )
    {
        OnuMgmtTable[onuEntry].onu_model = onu_model;
    }

	/* OUI */
	VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.OUI , pRxOamMsg, sizeof(OnuMgmtTable[0].DeviceInfo.OUI));
	if( EVENT_EUQ == V2R1_ENABLE)
		sys_console_printf( " OUI:%02x%02x%02x\r\n", pRxOamMsg[0], pRxOamMsg[1], pRxOamMsg[2] );
	pRxOamMsg += OAMMSG_OUILEN;

	/* 硬件版本信息 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Hardware Ver", OnuMgmtTable[onuEntry].DeviceInfo.HwVersion, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.HwVersionLen, valLen, MAXHWVERSIONLEN );
	pRxOamMsg += valLen;

	/* Boot版本 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Bootware Ver", OnuMgmtTable[onuEntry].DeviceInfo.BootVersion, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.BootVersionLen, valLen, MAXBOOTVERSIONLEN );
	pRxOamMsg += valLen;

	/* 软件版本 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Software Ver", OnuMgmtTable[onuEntry].DeviceInfo.SwVersion, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.SwVersionLen, valLen, MAXSWVERSIONLEN );
	pRxOamMsg += valLen;

	/* 固件版本 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Firmware Ver", OnuMgmtTable[onuEntry].DeviceInfo.FwVersion, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.FwVersionLen, valLen, MAXFWVERSIONLEN );
	pRxOamMsg += valLen;

	/* 名称 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Device Name", OnuMgmtTable[onuEntry].DeviceInfo.DeviceName, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.DeviceNameLen, valLen, MAXDEVICENAMELEN );
	pRxOamMsg += valLen;

	/* 描述 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Device Desc", OnuMgmtTable[onuEntry].DeviceInfo.DeviceDesc, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.DeviceDescLen, valLen, MAXDEVICEDESCLEN );
	pRxOamMsg += valLen;

	/* 位置 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	ONU_EUQ_STR_CPY( "Location", OnuMgmtTable[onuEntry].DeviceInfo.Location, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.LocationLen, valLen, MAXLOCATIONLEN );
	pRxOamMsg += valLen;

	/* 制造商 */
	valLen = *pRxOamMsg;
	pRxOamMsg++;
	/* modified by chenfj 2008-5-9
	    修改ONU vendor 信息, 为适应不同OEM 及GW产品, 此信息不再从ONU 读取,
	    而是出自OLT 侧*/
	pVendor = typesdb_pruduct_onu_vendor_info();
	ONU_EUQ_STR_CPY( "VendorInfo", OnuMgmtTable[onuEntry].DeviceInfo.VendorInfo, pVendor, 
			OnuMgmtTable[onuEntry].DeviceInfo.VendorInfoLen, VOS_StrLen(pVendor), MAXVENDORINFOLEN );
	pRxOamMsg += valLen;

	/* added by chenfj 2007-11-1
	     ONU设备中增加序列号和生产日期等信息
	     需要考虑到兼容以前的ONU( 其设备信息中没有序列号和生产日期)*/
	/* SN */
	if( (pRxOamMsg - pReleaseRx) >= RecvMsgLen )
	{
		valLen = 0;
	}
	else
	{
		valLen = *pRxOamMsg;
		pRxOamMsg++;
	}
	ONU_EUQ_STR_CPY( "Serial No", OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_No, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_NoLen, valLen, MAXSNLEN );
	pRxOamMsg += valLen;

	/* 生产日期 */
	if( (pRxOamMsg - pReleaseRx) >= RecvMsgLen )
	{
		valLen = 0;
	}
	else
	{
		valLen = *pRxOamMsg;
		pRxOamMsg++;
	}
	ONU_EUQ_STR_CPY( "Product date", OnuMgmtTable[onuEntry].DeviceInfo.MadeInDate, pRxOamMsg, 
			OnuMgmtTable[onuEntry].DeviceInfo.MadeInDateLen, valLen, MAXDATELEN );
	pRxOamMsg += valLen;

	/* added by chenfj 2008-3-5
	     ONU设备中增加ONU 自动配置请求
	      需要考虑到兼容以前的ONU ( 其设备信息中没有自动配置请求)
	     */
	/* 自动配置请求选项 */
	if( (pRxOamMsg - pReleaseRx) >= RecvMsgLen )
	{
		valLen = 0;
	}
	else
	{
		valLen = *pRxOamMsg;
		pRxOamMsg++;
	}
	/*if( SetOnuAutoConfigFlag( PonPortIdx, OnuIdx, (valLen ? *pRxOamMsg : NO_V2R1_ONU_AUTO_CONFIG_FLAG)) == RERROR )
		return RERROR;*/
	if( (valLen != 0) && (*pRxOamMsg == V2R1_ONU_AUTO_CONFIG_FLAG) )
	{
		AutoConfig = *pRxOamMsg;
	}

	/*OnuMgmtTable[onuEntry].DeviceInfo.AutoConfigFlag = AutoConfig;
	if( EVENT_EUQ == V2R1_ENABLE )
		sys_console_printf(" Auto config req(%d):%d\r\n", valLen, AutoConfig );*/
	ONU_EUQ_INT_CPY("Auto config req", OnuMgmtTable[onuEntry].DeviceInfo.AutoConfigFlag, AutoConfig );

	pRxOamMsg += valLen;

	ONU_MGMT_SEM_GIVE;

	/* added by chenfj 2008-6-12
	扩展多子卡ONU设备管理,增加子卡版本/状态等管理信息 */
	/* 支持槽位 */
	if( (pRxOamMsg - pReleaseRx) >= RecvMsgLen )
	{
		valLen = 0;
	}
	else
	{
		valLen = *pRxOamMsg;
		onuExtMgmt_OnuEquInfoCallback( MAKEDEVID(slot, port, (OnuIdx+1)), pRxOamMsg );
		pRxOamMsg += valLen;
	}
	if( EVENT_EUQ == V2R1_ENABLE )
		sys_console_printf(" Max sub-slot(%d):\r\n", valLen+1 );
    pRxOamMsg++;

    /*added by luh 2012-10-30，获取onu能力*/
    OnuAbilityType = *(unsigned char*)pRxOamMsg;
    if(OnuAbilityType == 0xfe)
    {
        pRxOamMsg++;
		valLen = *pRxOamMsg;
        pRxOamMsg++;
        OnuAbility = *(unsigned char*)pRxOamMsg;       
    	ONU_EUQ_INT_CPY("Onu Ability", OnuMgmtTable[onuEntry].OnuAbility, OnuAbility );          
    	pRxOamMsg += valLen;       
    }
    
	/* send  onu register trap to NMS */
	OnuRegisterHandler_EUQ( PonPortIdx, OnuIdx, slot, port, onuEntry );

	/* ONU 软件运行版本比较*/
#ifdef ONU_VERSION_COMPARE
	/*ret = OnuAppImageVersionComp(PonPortIdx, OnuIdx);*/
#endif

#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES)
	if( AutoConfig == V2R1_ONU_AUTO_CONFIG_FLAG )
	{		
		ULONG DevIdx = MAKEDEVID(slot, port, OnuIdx +1);
		onuAutoLoadOnuRequestCallback(DevIdx);
	}
#endif

	/* ONU 语音配置
	{
	int slot, port;
	unsigned int  onuDevIdx;
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	onuDevIdx = slot * 10000 + port * 1000 + (OnuIdx +1);
	StartOnuTdmVoiceConfigByIdx(onuDevIdx);
	}
	*/
    /*OnuEvent_Set_EQUMsgLength(PonPortIdx, OnuIdx, 0);*/
    if(pReleaseRx)
        VOS_Free(pReleaseRx);
	return( ROK );
}
#endif


/*****************************************************
 *
 *    Function:  GetOnuEUQInfo( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:  short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
 *                           
 *    Desc:   获取ONU 设备信息；
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/

int  GetOnuEUQInfo( short int PonPortIdx, short int OnuIdx )
{
	int ret = RERROR, ret1 = RERROR, ret2 = ROK;
	int count= 0;
	int OnuEntryIdx;
	int onuStatus;

	CHECK_ONU_RANGE;

	OnuEntryIdx = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[OnuEntryIdx].GetEUQflag = FALSE;
	onuStatus = OnuMgmtTable[OnuEntryIdx].OperStatus;
	ONU_MGMT_SEM_GIVE;

	/*if( GetOnuOperStatus_1( PonPortIdx, OnuIdx) == ONU_OPER_STATUS_DOWN )*/
	if( onuStatus == ONU_OPER_STATUS_DOWN )
	{
		/*DelOneNodeFromGetEUQ( PonPortIdx, OnuIdx );*/
		if( EVENT_EUQ == V2R1_ENABLE)
			sys_console_printf("\r\n  onu %d/%d/%d Not in Online status \r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx +1));
		return( RERROR );
	}

	do{
		ret = RERROR ;
		ret1 = RERROR;
		/*if(GetOnuOperStatus_1( PonPortIdx, OnuIdx) != ONU_OPER_STATUS_DOWN)*/
		if( onuStatus != ONU_OPER_STATUS_DOWN )
		{
			ret = EQU_SendMsgToOnu( PonPortIdx, OnuIdx, GET_ONU_SYS_INFO_REQ,0,0);
			/*if( ret != RERROR )
				ret1 = GetOnuEUQInfoHandle(PonPortIdx, OnuIdx);*/
		}
		else
		{ 
			count = MAX_GETEUQINFO_COUNTER;
			ret2 = RERROR ;
		}
		count ++;
	}while((( ret != ROK ) ||(ret1 != ROK)) && ( count < MAX_GETEUQINFO_COUNTER));
	
	if(( ret != ROK ) && ( ret2 == ROK ))
	{
		if(EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf("\r\n  Get onu %d/%d/%d device info Timeout\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx +1));
	}
	else if(ret1 != ROK )
	{
		if(EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf("\r\n  Get onu %d/%d/%d device info response ERR\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx +1));
	}
	if(( ret != ROK ) ||( ret1 != ROK )) 
	{
		if ( ret2 == ROK )
		{				
			/* modified by chenfj 2008-6-3
			    问题单6707: 问题1
			在获取ONU设备信息失败时(GW OAM注册失败,但标准MPCP和OAM注册成功),为防止ONU注册状态出现死锁,deregister ONU
			*/
			short int Llid;
			/*Llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);*/
			ONU_MGMT_SEM_TAKE;
			Llid = OnuMgmtTable[OnuEntryIdx].LLID;
			ONU_MGMT_SEM_GIVE;
			if( Llid == INVALID_LLID ) return( RERROR );
			/*if(GetLlidByOnuIdx(PonPortIdx, OnuIdx) != INVALID_LLID)
				Onu_deregister(PonPortIdx, OnuIdx);*/
			AddPendingOnu( PonPortIdx, OnuIdx, Llid, OnuMgmtTable[OnuEntryIdx].DeviceInfo.MacAddr, 0);
		}
		return( RERROR );
	}

	{ 
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[OnuEntryIdx].GetEUQCounter = 0;	/* modified by xieshl 20101228, 问题单11762, 没清0是引起onu pending的主要原因 */
		ONU_MGMT_SEM_GIVE;
		if( EVENT_EUQ == V2R1_ENABLE)
		{
			sys_console_printf("\r\n  Get onu %d/%d/%d device info OK\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx +1)); 
		}
	}
	
#if 0	
	if (EQU_SendMsgToOnu( PonPortIdx, OnuIdx, GET_ONU_SYS_INFO_REQ,0,0) == VOS_ERROR )
		{
		/*sys_console_printf("\r\n  Get ONU device info from %s/port%d onu%d Timeout\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1));*/
		if(GetOnuOperStatus_1( PonPortIdx, OnuIdx) != ONU_OPER_STATUS_DOWN)
			{
			AddOneNodeToGetEUQ(PonPortIdx, OnuIdx );
			}
		}
	else{
		if( GetOnuEUQInfoHandle(PonPortIdx, OnuIdx) == RERROR )
			{
			sys_console_printf("\r\n  Get ONU device info from %s/port%d onu%d Response err\r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1));
			if(GetOnuOperStatus_1( PonPortIdx, OnuIdx) != ONU_OPER_STATUS_DOWN )
				{
				AddOneNodeToGetEUQ(PonPortIdx, OnuIdx );
				}
			}
		else{ 
			if( EVENT_EUQ == V2R1_ENABLE)
				{
				sys_console_printf("\r\n  Get %s/port%d onu%d device information ok \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1)); 
				}
			}
		}
#endif
	return( ROK );
}


/*****************************************************
 *
 *    Function:  int SyncSysTimeToOnu( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:  short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu, or multi onu
 *                           
 *    Desc:   同步系统时间；支持单一ONU ，或多个ONU (同一PON 端口下)
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 int  SyncSysTimeToOnuMsg( short int PonPortIdx, short int OnuIdx )
{
	unsigned long aulMsg[4] = { MODULE_ONU, FC_ONU_EUQ_INFO, 0, 0};
	unsigned char *SendBuf = NULL;

	aulMsg[2] = PonPortIdx;
	aulMsg[3] = OnuIdx;

	aulMsg[2] = (aulMsg[2] << 16 ) + aulMsg[3];
	
	SendBuf = (unsigned char *)VOS_Malloc(8 , MODULE_ONU );
	if( SendBuf == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}

	aulMsg[3] = (unsigned long)SendBuf;
	SendBuf[0] = SYNC_ONU_SYS_TIME_REQ;
	*(int *)&SendBuf[4] = 0;
	
	if( VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL/*MSG_PRI_URGENT*/) != VOS_OK )
			{
			VOS_ASSERT(0);
			/*sys_console_printf("error: VOS send message err(SendMsgToOnu)\r\n" );*/
			VOS_Free(SendBuf);
			/*DelOneNodeFromGetEUQ(PonPortIdx, OnuIdx );*/
			return( RERROR );
			}

	return( ROK );

}


 int SyncSysTimeToOnu( short int PonPortIdx, short int OnuIdx )
{
	int ret = RERROR;
	if( OnuIdx < MAXONUPERPON )
	{
		CHECK_ONU_RANGE;
	}
	else
	{
		CHECK_PON_RANGE;
	}

	if( OnuIdx < MAXONUPERPON )
	{
		if( GetOnuOperStatus_1( PonPortIdx, OnuIdx) != ONU_OPER_STATUS_DOWN )
		{
			/*DelOneNodeFromGetEUQ( PonPortIdx, OnuIdx );*/
			/* added by chenfj 2008-2-22
			     在ONU 软件升级时, 不向ONU发送时间同步
			     */
			if( GetOnuSWUpdateStatus(PonPortIdx, OnuIdx ) != ONU_SW_UPDATE_STATUS_INPROGRESS )
			{
				ret = ROK;
			}
		}
	}
	else
		ret = ROK;

	if( ret == ROK )
	{
		ret = EQU_SendMsgToOnu( PonPortIdx, OnuIdx, SYNC_ONU_SYS_TIME_REQ, 0, 0 );
	}
	
	if( EVENT_EUQ == V2R1_ENABLE)
	{
		sys_console_printf("\r\n onu %d/%d/%d network time sync %s\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx +1),
							(ret == ROK ? "ok" : "err") );
	}
	return ret;
}
 int SyncSysTimeToOnu_New( short int PonPortIdx, short int OnuIdx )
{
	int ret = RERROR;
	if( OnuIdx < MAXONUPERPON )
	{
		CHECK_ONU_RANGE;
	}
	else
	{
		CHECK_PON_RANGE;
	}

	if( OnuIdx < MAXONUPERPON )
	{
		if( GetOnuOperStatus_1( PonPortIdx, OnuIdx) != ONU_OPER_STATUS_DOWN )
		{
			/*DelOneNodeFromGetEUQ( PonPortIdx, OnuIdx );*/
			/* added by chenfj 2008-2-22
			     在ONU 软件升级时, 不向ONU发送时间同步
			     */
			if( GetOnuSWUpdateStatus(PonPortIdx, OnuIdx ) != ONU_SW_UPDATE_STATUS_INPROGRESS )
			{
				ret = ROK;
			}
		}
	}
	else
		ret = ROK;

	if( ret == ROK )
	{
		ret = EQU_SendMsgToOnu_New( PonPortIdx, OnuIdx, SYNC_ONU_SYS_TIME_REQ, 0, 0 );
	}
	
	if( EVENT_EUQ == V2R1_ENABLE)
	{
		sys_console_printf("\r\n onu %d/%d/%d network time sync %s\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx +1),
							(ret == ROK ? "ok" : "err") );
	}
	return ret;
}

/*****************************************************
 *
 *    Function:  SetOnuSysInfoRspHandle( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:  short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
 *                           
 *    Desc:    收到从ONU来的设置ONU设备信息相应；若成功，则将信息保存在
 *                OLT中；若失败，则丢弃信息；并向CLI报告结果
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 /* deleted by chenfj 2008-3-11 */
#if 0
int SetOnuSysInfoRspHandle(short int PonPortIdx, short int OnuIdx )
{

	int i,counter;
	
	CHECK_ONU_RANGE

	sys_console_printf("\r\nRecv Msg From %s/port%d onu %d(SetOnuSysInfo()) \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1));
	counter = 0;
	for(i=0; i<RecvMsgFromOnuLen;i++)
		{
		sys_console_printf("%02x ", RecvMsgFromOnu[i]);
		counter ++;
		if( (counter % 16)==0) {sys_console_printf("\r\n");}
		else if(( counter % 8)==0){sys_console_printf("-- ");}
		}
	sys_console_printf("\r\n");

	if(RecvMsgFromOnuLen <= (/*OAMMSG_SESSIONIDLEN +*/ OAMMSG_MSGTYPELEN /*+ OAMMSG_SET_INFO_LEN_MIN*/ )) 
		{
		return( RERROR );
		}

	{
	unsigned char MsgType ;
	MsgType = 	RecvMsgFromOnu[OAMMSG_MSGTYPESTART];
	if( MsgType != SET_ONU_SYS_INFO_RSP) return( RERROR );
	}

	{
	

	}	
	return( ROK );
}
#endif

int  SetOnuDeviceName( short int PonPortIdx, short int OnuIdx,  char *Name,  int  NameLen )
{
	unsigned long aulMsg[4] = { MODULE_ONU, FC_ONU_EUQ_INFO, 0, 0};
	unsigned char *SendBuf = NULL;

	aulMsg[2] = PonPortIdx;
	aulMsg[3] = OnuIdx;

	aulMsg[2] = (aulMsg[2] << 16 ) + aulMsg[3];

	if(( NameLen != 0 ) && ( Name == NULL )) 
		return( RERROR );
	
	SendBuf = (unsigned char *)VOS_Malloc((NameLen+9) , MODULE_ONU );
	if( SendBuf == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}

	aulMsg[3] = (unsigned long)SendBuf;
	SendBuf[0] = SET_ONU_SYS_INFO_REQ;
	SendBuf[2] = SET_ONU_SYS_INFO_NAME;
	*(unsigned int *)&SendBuf[4] = NameLen;
	if(NameLen != 0 )
    {
		VOS_MemCpy( &SendBuf[8], Name, NameLen );
        SendBuf[8 + NameLen] = '\0';
		/*VOS_StrCpy( &SendBuf[8], Name);*/
    }   

	if( VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL/*MSG_PRI_URGENT*/) != VOS_OK )
			{
			VOS_ASSERT(0);
			/*sys_console_printf("error: VOS send message err(SendMsgToOnu)\r\n" );*/
			VOS_Free(SendBuf);
			/*DelOneNodeFromGetEUQ(PonPortIdx, OnuIdx );*/
			return( RERROR );
			}

	return( ROK );

}

int  SetOnuDeviceNameMsg( short int PonPortIdx, short int OnuIdx,  char *Name,  int  NameLen )
{
#ifdef ONU_COMM_SEMI
	int GetSemId;
#endif
	/*char pBuf[EUQ_MAX_OAM_PDU];
	int len;*/
	
	CHECK_ONU_RANGE

	if((Name == NULL ) && (NameLen != 0)) return( RERROR );
	if (NameLen >= MAXDEVICENAMELEN ) NameLen = MAXDEVICENAMELEN - 1;

#if 1
    OnuMgt_SetOnuDeviceName(PonPortIdx, OnuIdx, Name, NameLen);
#else    
	/*Added by suipl 2007/03/22*/
	if( GetOnuVendorType(PonPortIdx, OnuIdx) == ONU_VENDOR_CT )
		{
		VOS_MemSet( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceName, 0, MAXDEVICENAMELEN );
		OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceNameLen = 0;
		if(( NameLen != 0 ) && ( Name != NULL ))
			{
			VOS_StrCpy(OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceName, Name);
			OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceNameLen = NameLen;
			}
		return ROK;
	}
	/****End*************/

#ifdef ONU_COMM_SEMI
	GetSemId = VOS_SemTake( OnuEUQCommSemId, WAIT_FOREVER );
	if(GetSemId == VOS_ERROR ) {
		ASSERT(0);
		/*sys_console_printf( "  get SemId error(SetOnuEUQInfo)\r\n" );*/
		return ( RERROR );
		}
#endif

	if( GetOnuOperStatus( PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
		{
		/*DelOneNodeFromGetEUQ( PonPortIdx, OnuIdx );*/
		sys_console_printf("\r\n  %s/port%d Onu %d Not in Online status\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1));
#ifdef ONU_COMM_SEMI
		VOS_SemGive(OnuEUQCommSemId);
#endif
		return( RERROR );
		}
	{
	len = 0;
	VOS_MemSet( pBuf, 0,  EUQ_MAX_OAM_PDU );

	/* modified by chenfj 2007-7-3,当长度为0时，则为清除*/
	if( NameLen != 0 )
		{
		pBuf[0] = NameLen;
		VOS_MemCpy( &pBuf[1] , Name, NameLen );

		len = NameLen + 3;
		}
	else {
		pBuf[0] = 1;
		pBuf[1] =0;
		len = 3+1;

		}
	}

	/* only for test
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	ret = EQU_SendMsgToOnu( PonPortIdx, OnuIdx, SET_ONU_SYS_INFO_REQ, pBuf , len);	
		
	if(ret == RERROR ) 
		{
#ifdef ONU_COMM_SEMI
		VOS_SemGive(OnuEUQCommSemId);
#endif
		sys_console_printf("Send %s/port%d onu%d device name err\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1));
		return( RERROR );
		}
	
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT); */

	if (EQU_SendMsgToOnu( PonPortIdx, OnuIdx, SET_ONU_SYS_INFO_REQ, pBuf , len) == VOS_ERROR )
		{
		sys_console_printf("\r\nSet ONU device Info, wait ...  Timeout\r\n");
#ifdef ONU_COMM_SEMI
		VOS_SemGive(OnuEUQCommSemId);
#endif
		return( RERROR );
		}

	SetOnuDeviceName_1( PonPortIdx, OnuIdx, Name, NameLen); 

#ifdef ONU_COMM_SEMI
	VOS_SemGive(OnuEUQCommSemId);
#endif

#endif
	return( ROK );
}

int  SetOnuDeviceDesc( short int PonPortIdx, short int OnuIdx,  char *Desc, int DescLen )
{
	unsigned long aulMsg[4] = { MODULE_ONU, FC_ONU_EUQ_INFO, 0, 0};
	unsigned char *SendBuf = NULL;

	aulMsg[2] = PonPortIdx;
	aulMsg[3] = OnuIdx;

	aulMsg[2] = (aulMsg[2] << 16 ) + aulMsg[3];

	if(( DescLen != 0 ) && ( Desc == NULL )) 
		return( RERROR );
	
	SendBuf = (unsigned char *)VOS_Malloc((DescLen+9) , MODULE_ONU );
	if( SendBuf == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}

	aulMsg[3] = (unsigned long)SendBuf;
	SendBuf[0] = SET_ONU_SYS_INFO_REQ;
	SendBuf[2] = SET_ONU_SYS_INFO_DESC;
	*(unsigned int *)&SendBuf[4] = DescLen;
	if(DescLen != 0 )
    {
		VOS_MemCpy( &SendBuf[8], Desc, DescLen );
        SendBuf[8 + DescLen] = '\0';
		/*VOS_StrCpy( &SendBuf[8], Desc );*/
    }   
	if( VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL/*MSG_PRI_URGENT*/) != VOS_OK )
			{
			VOS_ASSERT(0);
			/*sys_console_printf("error: VOS send message err(SendMsgToOnu)\r\n" );*/
			VOS_Free(SendBuf);
			/*DelOneNodeFromGetEUQ(PonPortIdx, OnuIdx );*/
			return( RERROR );
			}

	return( ROK );

}

int  SetOnuDeviceDescMsg( short int PonPortIdx, short int OnuIdx,  char *Desc, int DescLen )
{
#ifdef ONU_COMM_SEMI
	int GetSemId;
#endif
	/*char pBuf[EUQ_MAX_OAM_PDU];
	int len;*/
	
	CHECK_ONU_RANGE

	if((Desc == NULL ) && (DescLen != 0)) return( RERROR );
	if (DescLen >= MAXDEVICEDESCLEN ) DescLen = MAXDEVICEDESCLEN - 1;

#if 1
	OnuMgt_SetOnuDeviceDesc(PonPortIdx, OnuIdx, Desc, DescLen);
#else
	/*Added by suipl 2007/03/22*/
	if( GetOnuVendorType(PonPortIdx, OnuIdx) == ONU_VENDOR_CT )
	{
		VOS_MemSet( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceDesc, 0, MAXDEVICEDESCLEN );
		OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceDescLen = 0;
		if(( DescLen != 0 ) &&( Desc != NULL ))
			{
			VOS_StrCpy(OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceDesc, Desc);
			OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceDescLen = DescLen;
			}
		return ROK;
	}
	/****End***********/
#ifdef ONU_COMM_SEMI
	GetSemId = VOS_SemTake( OnuEUQCommSemId, WAIT_FOREVER );
	if(GetSemId == VOS_ERROR ) {
		ASSERT(0);
		/*sys_console_printf( "get SemId error(SetOnuEUQInfo)\r\n" );*/
		return ( RERROR );
		}
#endif

	if( GetOnuOperStatus( PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
		{
		/*DelOneNodeFromGetEUQ( PonPortIdx, OnuIdx );*/
		sys_console_printf("\r\n%s/port%d Onu %d Not in Online status\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1));
#ifdef ONU_COMM_SEMI
		VOS_SemGive(OnuEUQCommSemId);
#endif
		return( RERROR);
		}
	{
	len = 0;
	VOS_MemSet( pBuf, 0,  EUQ_MAX_OAM_PDU );

	/* modified by chenfj 2007-7-3,当长度为0时，则为清除*/
	if( DescLen != 0 )
		{
		pBuf[0] = 0;
		pBuf[1] = DescLen;
		VOS_MemCpy( &pBuf[2] , Desc, DescLen );

		len = DescLen + 3;
		}
	else {
		pBuf[0] = 0;
		pBuf[1] = 1;
		pBuf[2] = 0;
		len = 1+3;
		}
	}

	/* only for test 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	
	if(EQU_SendMsgToOnu( PonPortIdx, OnuIdx, SET_ONU_SYS_INFO_REQ, pBuf , len)==RERROR)
		{
#ifdef ONU_COMM_SEMI
		VOS_SemGive(OnuEUQCommSemId);
#endif
		sys_console_printf("Send %s/port%d onu%d device description err\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1));
		return( RERROR );
		}

	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);*/

	if (EQU_SendMsgToOnu( PonPortIdx, OnuIdx, SET_ONU_SYS_INFO_REQ, pBuf , len)==RERROR)
		{
		sys_console_printf("\r\nSet ONU device Info, wait ...  Timeout\r\n");
#ifdef ONU_COMM_SEMI
		VOS_SemGive(OnuEUQCommSemId);
#endif
		return( RERROR );
		}

	SetOnuDeviceDesc_1( PonPortIdx, OnuIdx, Desc, DescLen); 
#ifdef ONU_COMM_SEMI
	VOS_SemGive(OnuEUQCommSemId);
#endif

#endif
	return( ROK );
}

int  SetOnuLocation( short int PonPortIdx, short int OnuIdx,  char *Location, int LocationLen )
{
	unsigned long aulMsg[4] = { MODULE_ONU, FC_ONU_EUQ_INFO, 0, 0};
	unsigned char *SendBuf = NULL;

	aulMsg[2] = PonPortIdx;
	aulMsg[3] = OnuIdx;

	aulMsg[2] = (aulMsg[2] << 16 ) + aulMsg[3];

	if(( LocationLen != 0 ) && ( Location == NULL )) 
		return( RERROR );
	
	SendBuf = (unsigned char *)VOS_Malloc((LocationLen+9) , MODULE_ONU );
	if( SendBuf == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}

	aulMsg[3] = (unsigned long)SendBuf;
	SendBuf[0] = SET_ONU_SYS_INFO_REQ;
	SendBuf[2] = SET_ONU_SYS_INFO_LOCATION;
	*(unsigned int *)&SendBuf[4] = LocationLen;
	if(LocationLen != 0 )
    {
		VOS_MemCpy( &SendBuf[8], Location, LocationLen );
        SendBuf[8 + LocationLen] = '\0';
		/*VOS_StrCpy( &SendBuf[8], Location );*/
    }   
	if( VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL/*MSG_PRI_URGENT*/) != VOS_OK )
			{
			VOS_ASSERT(0);
			/*sys_console_printf("error: VOS send message err(SendMsgToOnu)\r\n" );*/
			VOS_Free(SendBuf);
			/*DelOneNodeFromGetEUQ(PonPortIdx, OnuIdx );*/
			return( RERROR );
			}

	return( ROK );

}

int  SetOnuLocationMsg( short int PonPortIdx, short int OnuIdx,  char *Location, int LocationLen )
{
#ifdef ONU_COMM_SEMI
	int GetSemId;
#endif
	/*char pBuf[EUQ_MAX_OAM_PDU];
	int len;*/
	
	CHECK_ONU_RANGE

	if((Location == NULL ) && (LocationLen != 0)) return( RERROR );
	if (LocationLen >= MAXDEVICEDESCLEN ) LocationLen = MAXDEVICEDESCLEN - 1;

#if 1
    OnuMgt_SetOnuDeviceLocation(PonPortIdx, OnuIdx, Location, LocationLen);    
#else
	/*Added by suipl 2007/03/22*/
	if( GetOnuVendorType(PonPortIdx, OnuIdx) ==  ONU_VENDOR_CT )
	{
		VOS_MemSet( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.Location, 0, MAXDEVICEDESCLEN );
		OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.LocationLen = 0;
		if( LocationLen != 0 )
			{
			VOS_StrCpy(OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.Location, Location);
			OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.LocationLen= LocationLen;
			}
		return ROK;
	}
	/*end*/
#ifdef ONU_COMM_SEMI
	GetSemId = VOS_SemTake( OnuEUQCommSemId, WAIT_FOREVER );
	if(GetSemId == VOS_ERROR ) 
		{
		ASSERT(0);
		/*sys_console_printf( "get SemId error(SetOnuEUQInfo)\r\n" );*/
		return ( RERROR );
		}
#endif

	if( GetOnuOperStatus( PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
		{
		/*DelOneNodeFromGetEUQ( PonPortIdx, OnuIdx );*/
		sys_console_printf("\r\n%s/port%d Onu %d Not in Online status\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1));
#ifdef ONU_COMM_SEMI
		VOS_SemGive(OnuEUQCommSemId);
#endif
		return( RERROR);
		}
	{
	len = 0;
	VOS_MemSet( pBuf, 0,  EUQ_MAX_OAM_PDU );

	/* modified by chenfj 2007-7-3,当长度为0时，则为清除*/
	pBuf[0] = 0;
	pBuf[1] = 0;
	if( LocationLen != 0 )
		{
		pBuf[2] = LocationLen;
		VOS_MemCpy( &pBuf[3] , Location, LocationLen );
		len = LocationLen + 3;
		}
	else{
		pBuf[2] = 1;
		pBuf[3] = 0;
		len = 1+3;
		}
	}

	/* only for test */
	/*GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	
	if(EQU_SendMsgToOnu( PonPortIdx, OnuIdx, SET_ONU_SYS_INFO_REQ, pBuf , len)==RERROR) 
		{
#ifdef ONU_COMM_SEMI
		VOS_SemGive(OnuEUQCommSemId);
#endif
		sys_console_printf("Send %s/port%d onu%d device location err\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx +1));
		return( RERROR );
		}

	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);*/

	if (EQU_SendMsgToOnu( PonPortIdx, OnuIdx, SET_ONU_SYS_INFO_REQ, pBuf , len)==RERROR)
		{
		sys_console_printf("\r\nSet ONU device Info, wait ...  Timeout\r\n");
#ifdef ONU_COMM_SEMI
		VOS_SemGive(OnuEUQCommSemId);
#endif
		return( RERROR );
		}

	SetOnuLocation_1( PonPortIdx, OnuIdx, Location, LocationLen); 
#ifdef ONU_COMM_SEMI
	VOS_SemGive(OnuEUQCommSemId);
#endif

#endif
	return( ROK );
}

/*****************************************************
 *
 *    Function:  OnuEUQRecvMsgCallBack( short int PonPortIdx, short int OnuIdx, short int len, unsigned char *pDataBuf)
 *
 *    Param:  short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
 *                           
 *    Desc:   在设置ONU设备信息，或查询ONU设备信息时，收到从ONU返回的相应，则调用本函数
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
#if 0
int OnuEUQRecvMsgCallBack( short int PonPortIdx, short int OnuLlid,/* unsigned char GwProId,*/ short int len, unsigned char *pDataBuf, unsigned char  *pSessionField)
{
	short int OnuIdx;

	/*if(EVENT_REGISTER ==V2R1_ENABLE )
	sys_console_printf("\r\n  --1 ONU EUQ msg Recv callback pon%d  onu%d\r\n", PonPortIdx, OnuLlid);*/

	/*OnuIdx = GetOnuIdxByLlid(PonPortIdx, OnuLlid );*/

	if(( pDataBuf == NULL ) ||(pSessionField == NULL))
		{
		VOS_ASSERT(0);
		if( pDataBuf != NULL )
			VOS_Free(pDataBuf);
		if(pSessionField != NULL )
			VOS_Free( pSessionField );
		VOS_SemGive(OnuEUQRecvSemId);
		return( RERROR );
		}

	if(( PonPortIdx < 0 ) ||(PonPortIdx >= MAXPON))
		{
		if( pDataBuf != NULL )
			VOS_Free( pDataBuf );
		if( pSessionField != NULL )
			VOS_Free( pSessionField );
		sys_console_printf("\r\n rxOnuEUQMsg err:Pon%d out of range\r\n", (PonPortIdx+1));
		VOS_SemGive(OnuEUQRecvSemId);
		return( RERROR);
		}
		
	OnuIdx = OnuLlid-1;
	if( OnuIdx == RERROR ) 
		{
		if( pDataBuf != NULL )
			VOS_Free( pDataBuf );
		if( pSessionField != NULL )
			VOS_Free( pSessionField );
		
		sys_console_printf(" rxOnuEUQMsg err onu%d/%d/%d out of range\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
		VOS_SemGive(OnuEUQRecvSemId);
		return( RERROR );
		}

	if(pDataBuf[0] == SYNC_ONU_SYS_TIME_RSP ) /* ONU系统时间同步，不要求ONU响应*/
		{
		if( pDataBuf != NULL )
			VOS_Free( pDataBuf );
		if( pSessionField != NULL )
			VOS_Free( pSessionField );
		VOS_SemGive(OnuEUQRecvSemId);
		return( ROK );
		}

			
	if((PonPortIdx != ONUEUQMsgWaitPonPortIdx) ||( OnuIdx != ONUEUQMsgWaitOnuIdx ) ||(WaitOnuEUQMsgFlag != V2R1_ONU_EUQ_MSG_WAIT) /*|| ( ONUEUQMsgWaitMsgType != pDataBuf[0])*/)
		{
		if( pDataBuf != NULL )
			VOS_Free( pDataBuf );
		if( pSessionField != NULL )
			VOS_Free( pSessionField );
		if(  EVENT_EUQ == V2R1_ENABLE )
			{
			if(PonPortIdx != ONUEUQMsgWaitPonPortIdx)
				{
				sys_console_printf("pon1=%d, %d\r\n", PonPortIdx, ONUEUQMsgWaitPonPortIdx);
				}
			if( OnuIdx != ONUEUQMsgWaitOnuIdx )
				{
				sys_console_printf("onu1=%d, %d\r\n",(OnuIdx+1), ONUEUQMsgWaitOnuIdx);
				}
			if(WaitOnuEUQMsgFlag != V2R1_ONU_EUQ_MSG_WAIT)
				{
				sys_console_printf("onu%d/%d,waitflag=%d\r\n",PonPortIdx,(OnuIdx+1), WaitOnuEUQMsgFlag);
				}
			}
		VOS_SemGive(OnuEUQRecvSemId);
		return( RERROR );
		}

	if(EVENT_REGISTER ==V2R1_ENABLE )
		sys_console_printf("\r\n ONU devcie info msg Recv callback\r\n");
	
	if((len +8)> EUQ_MAX_OAM_PDU) len = EUQ_MAX_OAM_PDU -8;	
	VOS_MemCpy( &RecvMsgFromOnuSessionId[0], pSessionField, 8);
	VOS_MemCpy( &RecvMsgFromOnu[0], pDataBuf, len );
	RecvMsgFromOnuLen = len;
	
	WaitOnuEUQMsgFlag = V2R1_ONU_EUQ_MSG_OK;

	if( pDataBuf != NULL )
		VOS_Free( pDataBuf );
	if( pSessionField != NULL )
		VOS_Free( pSessionField );

	VOS_SemGive(OnuEUQRecvSemId);
	/*VOS_TaskDelay(30);*/
	return( ROK );
}
int OnuEUQRecvMsgCallBack_New( short int PonPortIdx, short int OnuLlid,/* unsigned char GwProId,*/ short int len, unsigned char *pDataBuf, unsigned char  *pSessionField)
{
	short int OnuIdx;
    char *pdata;
	/*if(EVENT_REGISTER ==V2R1_ENABLE )
	sys_console_printf("\r\n  --1 ONU EUQ msg Recv callback pon%d  onu%d\r\n", PonPortIdx, OnuLlid);*/

	/*OnuIdx = GetOnuIdxByLlid(PonPortIdx, OnuLlid );*/

	if(( pDataBuf == NULL ) ||(pSessionField == NULL))
    {
		VOS_ASSERT(0);
		if( pDataBuf != NULL )
			VOS_Free(pDataBuf);
		if(pSessionField != NULL )
			VOS_Free( pSessionField );
		return( RERROR );
    }

	if(( PonPortIdx < 0 ) ||(PonPortIdx >= MAXPON))
    {
		if( pDataBuf != NULL )
			VOS_Free( pDataBuf );
		if( pSessionField != NULL )
			VOS_Free( pSessionField );
		sys_console_printf("\r\n rxOnuEUQMsg err:Pon%d out of range\r\n", (PonPortIdx+1));
		return( RERROR);
    }
		
	OnuIdx = OnuLlid-1;
	if( OnuIdx == RERROR ) 
	{
		if( pDataBuf != NULL )
			VOS_Free( pDataBuf );
		if( pSessionField != NULL )
			VOS_Free( pSessionField );
		
		sys_console_printf(" rxOnuEUQMsg err onu%d/%d/%d out of range\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
		return( RERROR );
	}

	if(pDataBuf[0] == SYNC_ONU_SYS_TIME_RSP ) /* ONU系统时间同步，不要求ONU响应*/
	{
		if( pDataBuf != NULL )
			VOS_Free( pDataBuf );
		if( pSessionField != NULL )
			VOS_Free( pSessionField );
		return( ROK );
	}
		
	if(EVENT_REGISTER ==V2R1_ENABLE )
		sys_console_printf("\r\n ONU devcie info msg Recv callback\r\n");
	
	if((len +8)> EUQ_MAX_OAM_PDU) len = EUQ_MAX_OAM_PDU -8;	
	/*VOS_MemCpy( &RecvMsgFromOnuSessionId[0], pSessionField, 8);
	VOS_MemCpy( &RecvMsgFromOnu[0], pDataBuf, len );*/
	if(OnuEvent_Get_WaitOnuEUQMsgFlag(PonPortIdx, OnuIdx) == V2R1_ONU_EUQ_MSG_WAIT)
	{
    	pdata = (char*)VOS_Malloc(len, MODULE_ONU);
        if(pdata == NULL)
        {
            VOS_ASSERT(0);
        	if( pDataBuf != NULL )
        		VOS_Free( pDataBuf );
        	if( pSessionField != NULL )
        		VOS_Free( pSessionField );    
            
            return VOS_ERROR;
        }
        VOS_MemCpy(pdata, pDataBuf, len);
    	if( pDataBuf != NULL )
    		VOS_Free( pDataBuf );
    	if( pSessionField != NULL )
    		VOS_Free( pSessionField );    
        
    	OnuEvent_Set_EQUMsgLength(PonPortIdx, OnuIdx, len);
        OnuEvent_Set_WaitOnuEUQMsgFlag(PonPortIdx, OnuIdx, V2R1_ONU_EUQ_MSG_OK);
        if(OnuEvent_InprogressMsg_Send(PonPortIdx, OnuIdx, pdata) != VOS_OK)
        {
            VOS_Free(pdata);
            return VOS_ERROR;
        }
	}
    else
    {
		if( pDataBuf != NULL )
			VOS_Free( pDataBuf );
		if( pSessionField != NULL )
			VOS_Free( pSessionField );
		return( ROK );
    }
    
	return( ROK );
}
#endif
int OnuEUQRecvMsgTest(short int PonPortIdx, short int OnuIdx)
{
	OnuEUQInfo_S *RecvOnuMsg;
	short int OnuLlid;
	char string[20]="12345abcde67890fghij";

	char *Session;
	char *DataBuf;

	RecvOnuMsg = (OnuEUQInfo_S *)VOS_Malloc( sizeof(OnuEUQInfo_S), MODULE_ONU );

	if(RecvOnuMsg == NULL) return( RERROR );

	OnuLlid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( OnuLlid == INVALID_LLID ) return( RERROR );

	VOS_MemSet( RecvOnuMsg->SessionId, 0x55, sizeof( RecvOnuMsg->SessionId ));
	RecvOnuMsg->MsgType = 1;
	RecvOnuMsg->DeviceType = 0x1234;
	RecvOnuMsg->OUI[0] = 0;
	RecvOnuMsg->OUI[1]= 0x0f;
	RecvOnuMsg->OUI[2]=0xe9;
	RecvOnuMsg->HwVerLen = 10;
	VOS_MemCpy(RecvOnuMsg->HwVer,string, RecvOnuMsg->HwVerLen);
	RecvOnuMsg->BootVerLen = 15;
	VOS_MemCpy(RecvOnuMsg->BootVer, string, RecvOnuMsg->BootVerLen);
	RecvOnuMsg->SwVerLen = 15;
	VOS_MemCpy(RecvOnuMsg->SwVer, &string[5], RecvOnuMsg->SwVerLen );
	RecvOnuMsg->FwVerLen = 20;
	VOS_MemCpy(RecvOnuMsg->FwVer, string, RecvOnuMsg->FwVerLen );
	RecvOnuMsg->NameLen = 10;
	VOS_MemCpy(RecvOnuMsg->Name, &string[10], RecvOnuMsg->NameLen );
	RecvOnuMsg->DescLen = 10;
	VOS_MemCpy(RecvOnuMsg->Desc,&string[5], RecvOnuMsg->DescLen );
	RecvOnuMsg->LocationLen = 10;
	VOS_MemCpy(RecvOnuMsg->Location,&string[4], RecvOnuMsg->LocationLen );
	RecvOnuMsg->VendorInfoLen = 20;
	VOS_MemCpy(RecvOnuMsg->VendorInfo,string, RecvOnuMsg->VendorInfoLen );

	Session = (unsigned char *)VOS_Malloc( 8, MODULE_ONU );
	if( Session == NULL )
		{
		ASSERT(0);
		VOS_Free(RecvOnuMsg);
		return( RERROR );
		}

	DataBuf = (unsigned char *)VOS_Malloc( sizeof(OnuEUQInfo_S), MODULE_ONU );
	if( DataBuf == NULL )
		{
		ASSERT(0);
		VOS_Free(RecvOnuMsg);
		VOS_Free(Session) ;
		return( RERROR);
		}

	VOS_MemCpy( Session, (unsigned char *)RecvOnuMsg, 8);
	VOS_MemCpy(DataBuf, (unsigned char *)&(RecvOnuMsg->MsgType), sizeof(OnuEUQInfo_S)-8);
#if 0	
	OnuEUQRecvMsgCallBack(PonPortIdx, OnuLlid, /*4,*/ sizeof(OnuEUQInfo_S)-8, (unsigned char *)&(RecvOnuMsg->MsgType), (unsigned char *)RecvOnuMsg);
#endif
	VOS_Free(RecvOnuMsg);
	return( ROK );

}

/*****************************************************
 *
 *    Function:  RecordOnuEUQInfo(short int PonPortIdx, short int OnuIdx, unsigned char *Info, int Len )
 *
 *    Param:  short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
 *                           
 *    Desc:   在收到ONU设备信息修改事件后, 调用本函数保存设备信息
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int   RecordOnuEUQInfo(short int PonPortIdx, short int OnuIdx, unsigned char *Info, int Len )
{
	int CurLen;
	
 	CHECK_ONU_RANGE

	if( Info == NULL ) return( RERROR );
	if( Len < 3 ) return( RERROR ); 

	CurLen = 0;
	
	if( Info[CurLen] != 0 )
		{
		SetOnuDeviceName_1( PonPortIdx, OnuIdx, &Info[CurLen+1], Info[CurLen] );
		CurLen += Info[CurLen] ;
		}
	CurLen += 1;

	if( Info[CurLen] != 0 )
		{
		SetOnuDeviceDesc_1( PonPortIdx, OnuIdx, &Info[CurLen+1], Info[CurLen]);
		CurLen += Info[CurLen];
		}
	CurLen += 1;

	if( Info[CurLen] != 0 )
		{
		SetOnuLocation_1( PonPortIdx, OnuIdx, &Info[CurLen+1], Info[CurLen]);
		CurLen += Info[CurLen];
		}
	CurLen += 1;

	return( ROK );
}

/* add by chenfj 2007/5/18
   增加设置PAS5201下ONU FEC
*/
/*****************************************************
 *
 *    Function:  SetOnuFecMode(short int PonPortIdx, short int OnuIdx, int  mode )
 *
 *    Param:  short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
 *                 int mode -- FEC mode , ENABLE/DISABLE
 *                           
 *    Desc:   PAS5001不支持FEC；在调用这个函数前，应判断PON 芯片类型
 *               由于PAS5201下可能有PAS6201，所以在PAS5201下，这个函数也可能返回错误
 *
 *    Return:    ROK/RERROR
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int  SetOnuFecMode(short int PonPortIdx, short int OnuIdx, int  mode )
{
	/*short int PonChipType;
	short int PonChipVer = RERROR;
	short int onu_id;
	short int ret= PAS_EXIT_ERROR;*/
	int retVal ;
	
	CHECK_ONU_RANGE

	if(( mode != STD_FEC_MODE_ENABLED) && ( mode != STD_FEC_MODE_DISABLED))
	 	return( RERROR );

	retVal = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( retVal != ROK )  return (retVal);

#if 1
    retVal = OnuMgt_SetOnuFecMode(PonPortIdx,OnuIdx,mode);
    if ( OLT_CALL_ISERROR(retVal) )
    {
        switch(retVal)
        {
            case OLT_ERR_NOTEXIST:
                retVal = V2R1_ONU_OFF_LINE;
                break;
            default:   
                retVal = RERROR;
        }

        return retVal;
    }

    return ROK;
#else
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
	
	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		{
		if( PonChipVer == PONCHIP_PAS5001) return( RERROR );
		
		/*OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].FEC_Mode = mode;*/
		return( V2R1_ONU_OFF_LINE );
		}
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );

	if(PonChipType == PONCHIP_PAS )
		{
		/*
		PonChipVer = V2R1_GetPonChipVersion( PonPortIdx );
		*/
		if( PonChipVer != PONCHIP_PAS5001 )
			{	
			if( mode == STD_FEC_MODE_ENABLED )
				{
				ret = PAS_set_llid_fec_mode(PonPortIdx, PON_BROADCAST_LLID,  ENABLE );
				ret = PAS_set_llid_fec_mode(PonPortIdx, onu_id,  ENABLE );
				}
			else{
				PON_fec_status_raw_data_t fec_status;
#if 1
                OLT_raw_stat_item_t       stat_item;
#else
				PON_timestamp_t			  timestamp; 
#endif
                
				ret = PAS_set_llid_fec_mode( PonPortIdx, onu_id,  DISABLE );
#if 1
                stat_item.collector_id = PON_OLT_ID;
                stat_item.raw_statistics_type = PON_RAW_STAT_FEC_STATUS;
                stat_item.statistics_parameter = 0;
                stat_item.statistics_data = &fec_status;
                stat_item.statistics_data_size = sizeof(fec_status);
                ret = OLT_GetRawStatistics(PonPortIdx, &stat_item);
#else
				ret = PAS_get_raw_statistics( PonPortIdx,
                                   	PON_OLT_ID,
                                   	PON_RAW_STAT_FEC_STATUS,
                                   	0,
                                  		&fec_status,
                                   	&timestamp);
#endif
				if( ret !=  PAS_EXIT_OK )  return( ROK );
				if( fec_status.number_of_llids_with_fec == 0 ) /* No ONUs with FEC disable broadcast LLID */
					ret = PAS_set_llid_fec_mode(PonPortIdx, PON_BROADCAST_LLID,  DISABLE );
				
				}

			if( ret == PAS_EXIT_OK ) 
				{
				OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].FEC_Mode = mode;
				return( ROK );
				}
			else  return( RERROR );			
			}
		}

	else { /* other pon chip handler */

		}

	return( RERROR );
#endif
}

int  GetOnuFecMode(short int PonPortIdx, short int OnuIdx, int  *mode )
{
	int retVal;

	CHECK_ONU_RANGE

	if( mode == NULL )
	 	return( RERROR );

	retVal = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( retVal != ROK )  return (retVal );
	
	ONU_MGMT_SEM_TAKE;
	*mode = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].FEC_Mode;
	ONU_MGMT_SEM_GIVE;
	/*if( *mode == STD_FEC_MODE_UNKNOWN ) *mode = STD_FEC_MODE_DISABLED;*/
	return( ROK );
	
#if 0	
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
	
	if(PonChipType == PONCHIP_PAS )
		{
		/*
		PonChipVer = V2R1_GetPonChipVersion( PonPortIdx );
		*/
		if(( PonChipVer  !=  PONCHIP_PAS5001 ) && ( PonChipVer != PONCHIP_PAS5201 ) ) return( RERROR );
		if( PonChipVer == PONCHIP_PAS5001 ) return (RERROR );
		if( PonChipVer == PONCHIP_PAS5201 )
			{			
			ret = PAS_get_llid_fec_mode( PonPortIdx, onu_id,  mode );
			if( ret == PAS_EXIT_OK ) 
				{
				OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].FEC_Mode = *mode;
				return( ROK );
				}
			else  return( RERROR );			
			}
		}

	else { /* other pon chip handler */

		}

	return( RERROR );
#endif
}

int CopyOnuFecMode(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iRlt = 0;
    int iSrcEntry;
    int iSrcCfg;

    iSrcEntry = SrcPonPortIdx * MAXONUPERPON + SrcOnuIdx;
    ONU_MGMT_SEM_TAKE;
    iSrcCfg   = OnuMgmtTable[iSrcEntry].FEC_Mode;
    ONU_MGMT_SEM_GIVE;
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OnuMgt_SetOnuFecMode(DstPonPortIdx, DstOnuIdx, iSrcCfg);
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( STD_FEC_MODE_ENABLED == iSrcCfg )
        {
            iRlt = OnuMgt_SetOnuFecMode(DstPonPortIdx, DstOnuIdx, iSrcCfg);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            int iDstEntry;
            int iDstCfg;
            
            iDstEntry = DstPonPortIdx * MAXONUPERPON + DstOnuIdx;
            ONU_MGMT_SEM_TAKE;
            iDstCfg   = OnuMgmtTable[iDstEntry].FEC_Mode;
            ONU_MGMT_SEM_GIVE;
            if ( iDstCfg != iSrcCfg )
            {
                iRlt = OnuMgt_SetOnuFecMode(DstPonPortIdx, DstOnuIdx, iSrcCfg);
            }
        }
        else
        {
            iRlt = OnuMgt_SetOnuFecMode(DstPonPortIdx, DstOnuIdx, iSrcCfg);
        }
    }

    return iRlt;
}

#ifdef  ONU_PON_LOOPBACK 
#endif

/* added by xieshl 20120828, 电科院测试 */
int  SetOnuPonLoopbackEnable(short int PonPortIdx, short int OnuIdx, int  enable )
{
	int retVal ;
	
	CHECK_ONU_RANGE;

	if(( enable != V2R1_ENABLE) && ( enable != V2R1_DISABLE))
	 	return( RERROR );

	retVal = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( retVal != ROK )  return (retVal);

    retVal = OnuMgt_SetPonLoopback(PonPortIdx,OnuIdx, enable);
    if ( OLT_CALL_ISERROR(retVal) )
    {
        switch(retVal)
        {
            case OLT_ERR_NOTEXIST:
                retVal = V2R1_ONU_OFF_LINE;
                break;
            default:   
                retVal = RERROR;
        }

        return retVal;
    }

    return ROK;
}

int  GetOnuPonLoopbackEnable(short int PonPortIdx, short int OnuIdx, int  *pEnable )
{
	int retVal;

	CHECK_ONU_RANGE;

	if( pEnable == NULL )
	 	return( RERROR );

	retVal = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( retVal != ROK )  return (retVal );
	
	ONU_MGMT_SEM_TAKE;
	*pEnable = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].LoopbackEnable;
	ONU_MGMT_SEM_GIVE;
	return( ROK );
}

#ifdef  ONU_UPSTREAM_SA_MAC_FILTER 
#endif

/*for 10G EPON of PMC8411 Filter by jinhl @2012-11-12 此段代码的被调用处，相关宏均未打开，所以，此段代码
内部调用的Remote接口暂不整改*/
#if 1
/* add by chenfj 2007/5/28
   增加设置/清除ONU 上行数据包原MAC 过滤
*/
/*****************************************************
 *
 *    Function:  AddOnuSAMacFilter( short int PonPortIdx ,short int OnuIdx, mac_address_t MacAddress )
 *
 *    Param:  short int PonPortIdx -- the specific pon port
 *			  short int OnuIdx -- the specific onu 
 *                mac_address_t MacAddress  -- MAC address, frmaes with this mac as Source Add will not be passed to data path
 *                           
 *    Desc:   设置ONU 原 MAC 数据过滤；
 *               
 *
 *    Return:    ROK/RERROR
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int AddOnuSAMacFilter( short int PonPortIdx ,short int OnuIdx, mac_address_t MacAddress )
{
	int ret;
	CHECK_ONU_RANGE

	if( MacAddress == NULL ) return( RERROR );
	/*if ( CompTwoMacAddress( MacAddress, Invalid_Mac_Addr ) == ROK ) return( V2R1_ONU_FILTER_SA_MAC_NOT_VALID );
	if ( CompTwoMacAddress( MacAddress, Invalid_Mac_Addr1 ) == ROK ) return( V2R1_ONU_FILTER_SA_MAC_NOT_VALID );*/
	if( MAC_ADDR_IS_ZERO(MacAddress) || MAC_ADDR_IS_BROADCAST(MacAddress) )
		return( V2R1_ONU_FILTER_SA_MAC_NOT_VALID );

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  ret != ROK ) return( ret );
		
	ret = AddFilterSAMacToTable( PonPortIdx, OnuIdx, MacAddress );
	if( ret !=  ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		EnableOnuSAMacFilter( PonPortIdx, OnuIdx/*, MacAddress*/);
	return( ROK );
}

int  ClearOnuSAMacFilter(short int PonPortIdx ,short int OnuIdx, mac_address_t MacAddress )
{
	short  ret;
	
	CHECK_ONU_RANGE

	if( MacAddress == NULL ) return( RERROR );
	/*if ( CompTwoMacAddress( MacAddress, Invalid_Mac_Addr ) == ROK ) return( V2R1_ONU_FILTER_SA_MAC_NOT_VALID );
	if ( CompTwoMacAddress( MacAddress, Invalid_Mac_Addr1 ) == ROK ) return( V2R1_ONU_FILTER_SA_MAC_NOT_VALID );*/
	if( MAC_ADDR_IS_ZERO(MacAddress) || MAC_ADDR_IS_BROADCAST(MacAddress) )
		return( V2R1_ONU_FILTER_SA_MAC_NOT_VALID );

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
		 DisableOnuSAMacFilter( PonPortIdx, OnuIdx, MacAddress );
	
	ret = DelFilterSAMacFromTable(PonPortIdx, OnuIdx, MacAddress);
	if( ret != ROK ) return( ret );
	
	else return( ROK );
}

int  ClearOnuSAMacFilterAll( short int PonPortIdx, short int OnuIdx )
{
	MacTable_S  *FilterMac, *NextPtr;
	
	CHECK_ONU_RANGE;

	/*ONU_MGMT_SEM_TAKE;*/
	FilterMac = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].FilterSaMac;
	while( FilterMac != 0 ) 
	{
		NextPtr = (MacTable_S *)FilterMac->nextMac;
		/*sys_console_printf(" pon %d, onu %d addr 0x%x,mac:%02x%02x.%02x%02x.%02x%02x \r\n",PonPortIdx, (OnuIdx+1), &(FilterMac->MAC[0]), FilterMac->MAC[0],FilterMac->MAC[1],FilterMac->MAC[2],FilterMac->MAC[3],FilterMac->MAC[4],FilterMac->MAC[5]);*/
		ClearOnuSAMacFilter( PonPortIdx, OnuIdx, &(FilterMac->MAC[0]) );
		FilterMac = NextPtr;
	}
	/*ONU_MGMT_SEM_GIVE;*/

	return( ROK );
}

int EnableOnuSAMacFilter( short int PonPortIdx ,short int OnuIdx/*, mac_address_t MacAddress*/ )
{
	short int PonChipType;
	short int Onu_id;
	short int ret;
	MacTable_S *MacAddress;
	
	CHECK_ONU_RANGE
	/*
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue);
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
	{
#if defined(_EPON_10G_PMC_SUPPORT_)            
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			GW10G_REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			TK_PASONU_address_table_clear( PonPortIdx, Onu_id );
        }
        else
        {
            VOS_ASSERT(0);
        	return( RERROR );
        }

		/*ONU_MGMT_SEM_TAKE;*/

		MacAddress = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].FilterSaMac;
		while( MacAddress != NULL )
		{
			/*
			sys_console_printf("enable filter mac address : 0x%x\r\n", MacAddress->MAC );
			sys_console_printf("mac %02x%02x.%02x%02x.%02x%02x\r\n", MacAddress->MAC[0], MacAddress->MAC[1],MacAddress->MAC[2],MacAddress->MAC[3],MacAddress->MAC[4],MacAddress->MAC[5]);
			*/
			VOS_TaskDelay(10);
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_add_source_address_filter( PonPortIdx, Onu_id, MacAddress->MAC,  PON_DONT_PASS );
			}
    		else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_add_source_address_filter( PonPortIdx, Onu_id, MacAddress->MAC,  PON_DONT_PASS );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_add_source_address_filter( PonPortIdx, Onu_id, MacAddress->MAC,  PON_DONT_PASS );
            }
            else
            {
                break;
            }
            
			MacAddress =(MacTable_S*) MacAddress->nextMac;
		}
		/*ONU_MGMT_SEM_GIVE;*/
		if(( ret == PAS_EXIT_OK ) || ( ret == REMOTE_MANAGEMENT_ALREADY_EXISTS )) return( ROK ) ;
		else return( RERROR );
	}
	else { /* other pon chip handler */

	}
	return( RERROR );

}

int  DisableOnuSAMacFilter( short int PonPortIdx ,short int OnuIdx, mac_address_t MacAddress )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;

	CHECK_ONU_RANGE
	/*
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue!= ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*	ret = REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id ); */
		/*
		sys_console_printf("delete filter mac address : 0x%x\r\n", MacAddress );
		sys_console_printf("mac %02x%02x.%02x%02x.%02x%02x\r\n", MacAddress[0], MacAddress[1],MacAddress[2],MacAddress[3],MacAddress[4],MacAddress[5]);
		*/
		VOS_TaskDelay(10);
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_remove_da_filter( PonPortIdx, Onu_id, MacAddress );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_remove_da_filter( PonPortIdx, Onu_id, MacAddress );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_remove_da_filter( PonPortIdx, Onu_id, MacAddress );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
        
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK ) ;
		else return( RERROR );
		}
	
	else { /* other pon chip handler */

		}
	return( RERROR );
}

int  AddFilterSAMacToTable(short int PonPortIdx, short int OnuIdx, mac_address_t MacAddress )
{
	MacTable_S  *FilterMac, *PreFilter;
	
	CHECK_ONU_RANGE

	/*ONU_MGMT_SEM_TAKE;*/
	FilterMac = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].FilterSaMac ;
	PreFilter = FilterMac;

	while ( FilterMac  != 0 )
	{
		if( MAC_ADDR_IS_EQUAL( FilterMac->MAC, MacAddress ) ) 
		{
			/*ONU_MGMT_SEM_GIVE;*/
			return ( V2R1_ONU_FILTER_SA_MAC_EXIST);
		}
		PreFilter = FilterMac;
		FilterMac = (MacTable_S *) FilterMac->nextMac ;
	}
	
	FilterMac = ( MacTable_S *) VOS_Malloc( sizeof( MacTable_S ),  MODULE_ONU );
	if( FilterMac == NULL ) 
	{
		/*ONU_MGMT_SEM_GIVE;*/
		ASSERT(0);
		return( RERROR );
	}
	/*sys_console_printf("add mac filter to 0x%x, 0x%x\r\n", FilterMac, &(FilterMac->MAC[0]) );*/
	VOS_MemCpy( &(FilterMac->MAC[0]), MacAddress, BYTES_IN_MAC_ADDRESS );
	FilterMac->nextMac = 0;
	if( PreFilter == 0 )  OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].FilterSaMac = FilterMac;
	else PreFilter->nextMac = (unsigned int)FilterMac;
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].FilterSaMacNum ++;
	/*ONU_MGMT_SEM_GIVE;*/

	return( ROK );
}

int DelFilterSAMacFromTable(short int PonPortIdx, short int OnuIdx, mac_address_t MacAddress )
{
	MacTable_S  *FilterMac, *PreFilter;
	
	CHECK_ONU_RANGE

	/*ONU_MGMT_SEM_TAKE;*/
	FilterMac = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].FilterSaMac ;
	PreFilter = FilterMac;
	while ( FilterMac  != 0 )
	{
		if( MAC_ADDR_IS_EQUAL( FilterMac->MAC, MacAddress ) ) 
		{
			if( PreFilter == FilterMac )  /* 链表的第一个节点*/
			{
				OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].FilterSaMac  = (MacTable_S *)FilterMac->nextMac;				
			}
			else PreFilter->nextMac = FilterMac->nextMac;
			VOS_Free( (void *) FilterMac );
			OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].FilterSaMacNum -- ;
			
			/*ONU_MGMT_SEM_GIVE;*/
			return ( ROK);
		}
		PreFilter = FilterMac;
		FilterMac = (MacTable_S *)FilterMac->nextMac ;
	}
	/*ONU_MGMT_SEM_GIVE;*/
	
	return( V2R1_ONU_FILTER_SA_MAC_NOT_EXIST );

}

int  ShowOnuFilterSAMacByVty1( short int PonPortIdx, short int OnuIdx, struct vty *vty )
{
	MacTable_S  *FilterMac;
	/*int  i;*/

	CHECK_ONU_RANGE

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) 
		{
		vty_out(vty, "%s/port%d onu%d is not exist\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1));
		return( RERROR );
		}

	/*ONU_MGMT_SEM_TAKE;*/

	FilterMac = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].FilterSaMac;
	if( FilterMac != 0 )
		vty_out( vty, " %s/port%d onu%d source mac filter list:\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1));
	else{ 
		vty_out( vty, " %s/port%d onu%d source mac filter list is null\r\n\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1));
		/*ONU_MGMT_SEM_GIVE;*/
		return( ROK );
	}
	while( FilterMac != 0 )
	{
		vty_out( vty,"      %02x%02x.%02x%02x.%02x%02x\r\n", FilterMac->MAC[0], FilterMac->MAC[1],FilterMac->MAC[2],FilterMac->MAC[3],FilterMac->MAC[4],FilterMac->MAC[5] );
		FilterMac = (MacTable_S *)FilterMac->nextMac;	
	}
	/*ONU_MGMT_SEM_GIVE;*/
	vty_out( vty, "\r\n");
	
	return( ROK );
}


int  ShowOnuFilterSAMacByVty( short int PonPortIdx, short int OnuIdx, struct vty *vty )
{
	MacTable_S  *FilterMac;
	/*int  i;*/

	CHECK_ONU_RANGE

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
		{
		vty_out(vty,"onu %d/%d/%d not exist\r\n",GetCardIdxByPonChip(PonPortIdx),GetPonPortByPonChip(PonPortIdx),( OnuIdx+1));
		return( RERROR );
		}
	/*ONU_MGMT_SEM_TAKE;*/

	FilterMac = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].FilterSaMac;
	if( FilterMac != 0 )
		vty_out( vty, " %s/port%d onu%d source mac filter list:\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1));
	else{ 
		/*vty_out( vty, " %s/port%d onu%d no source mac filter\r\n\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1));*/
		/*ONU_MGMT_SEM_GIVE;*/
		return( ROK );
	}
	
	while( FilterMac != 0 )
	{
		vty_out( vty,"      %02x%02x.%02x%02x.%02x%02x\r\n", FilterMac->MAC[0], FilterMac->MAC[1],FilterMac->MAC[2],FilterMac->MAC[3],FilterMac->MAC[4],FilterMac->MAC[5] );
		FilterMac = (MacTable_S *)FilterMac->nextMac;	
	}
	/*ONU_MGMT_SEM_GIVE;*/
	vty_out( vty, "\r\n");
	
	return( ROK );
}

int  ShowOnuFilterSAMacByVtyUnderDebug( short int PonPortIdx, short int OnuIdx, struct vty *vty )
{
	MacTable_S  *FilterMac;
	/*int  i;*/

	CHECK_ONU_RANGE

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) return( RERROR );

	/*ONU_MGMT_SEM_TAKE;*/
	FilterMac = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].FilterSaMac;
	if( FilterMac != 0 )
		vty_out( vty, " %s/port%d onu%d source mac filter list:\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1));
	else{ 
		/*vty_out( vty, " %s/port%d onu%d no source mac filter\r\n\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1));*/
		/*ONU_MGMT_SEM_GIVE;*/
		return( ROK );
	}
	
	while( FilterMac != 0 )
	{
		vty_out( vty,"      %02x%02x.%02x%02x.%02x%02x\r\n", FilterMac->MAC[0], FilterMac->MAC[1],FilterMac->MAC[2],FilterMac->MAC[3],FilterMac->MAC[4],FilterMac->MAC[5] );
		FilterMac = (MacTable_S *)FilterMac->nextMac;	
	}
	/*ONU_MGMT_SEM_GIVE;*/
	vty_out( vty, "\r\n");
	
	return( ROK );
}

int AddOnuSAMacFilter1( short int PonPortIdx ,short int OnuIdx, mac_address_t MacAddress )
{
	short int Onu_id;
	short int ret;
	short int PonChipType = 0;/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	CHECK_ONU_RANGE

	if( MacAddress == NULL ) return( RERROR );
	
	
	MacAddress[0] = 0;
	MacAddress[1] =8;
	MacAddress[2] = 0xa1;
	MacAddress[3] = 0x29;
	MacAddress[4] = 0xb9;
	MacAddress[5] = 0x91;
	
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );
	/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
		GW10G_REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );
		ret = GW10G_REMOTE_PASONU_classifier_add_da_filter( PonPortIdx, Onu_id, MacAddress,  PON_DONT_PASS );
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS(PonChipType))
	{
		REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );
		ret = REMOTE_PASONU_classifier_add_da_filter( PonPortIdx, Onu_id, MacAddress,  PON_DONT_PASS );
	}
	/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
		TK_PASONU_address_table_clear( PonPortIdx, Onu_id );
		ret = TK_PASONU_classifier_add_da_filter( PonPortIdx, Onu_id, MacAddress,  PON_DONT_PASS );
    }
    else
    {
        ret = PAS_PARAMETER_ERROR;
    }
	if( ret == PAS_EXIT_OK ) return( ROK ) ;
	else return( RERROR );
}

#if 0

LONG config_aclf_ip_rule( LONG argc, CHAR **argv, struct vty * vty ,ULONG ulFlag)
    {
        SHORT sSrcRet, sDstRet;
        CHAR * pcTemp = NULL;
        QOS_ACL_IPMASK_S stSrcIp ;
        QOS_ACL_IPMASK_S stDstIp ;
        QOS_ACL_NODE_S * pstAclNode = NULL ;

        VOS_MemSet( &stSrcIp, 0, sizeof( QOS_ACL_IPMASK_S ) );
        VOS_MemSet( &stDstIp, 0, sizeof( QOS_ACL_IPMASK_S ) );
        if ( argv == NULL )
        {
            ASSERT( 0 );
            return VOS_ERROR;
        }

        if ( QOS_DEBUG )
        {
            VOS_SysLog( LOG_TYPE_QOS, LOG_INFO, "***FUNCTION***  config_aclf_ip_rule() is called." );
        }        

 
        /* search the list to find whether the node which has the same ID exists in the system */
        if ( qos_acl_search_acl_node( ( ULONG ) VOS_StrToUL( argv[ 0 ], &pcTemp, 10 ), &pstAclNode, vty ) == VOS_ERROR )
        {
            if ( QOS_DEBUG )
            {
                QOS_SYSLOG( LOG_DEBUG, "config_aclf_ip_rule():Searching for acl node failed." );
            }
            
            return VOS_ERROR;
        }
       

        /* no same node so create a new node */
        if ( pstAclNode == NULL )
        {
            if ( qos_acl_create_acl_node( &pstAclNode, ( ULONG ) VOS_StrToUL( argv[ 0 ], &pcTemp, 10 ), vty ) == VOS_ERROR )
            {
                vty_out( vty, "%% Creating new access-list node failed. \r\n" );
                return VOS_ERROR;
            }
        }
        else
        {
            vty_out( vty, "%% Access-list %u has existed in the system. \r\n", ( ULONG ) VOS_StrToUL( argv[ 0 ], &pcTemp, 10 ) );
            return VOS_ERROR;
        }

        if ( !( VOS_StrCmp( argv[ 1 ] , "permit" ) ) )
            pstAclNode->lDrop = ( LONG ) QOS_FALSE ; /* permit the packets meeting the conditions to pass */
        else
            pstAclNode->lDrop = ( LONG ) QOS_TRUE ; /* deny the packets meeting the conditions to pass */

        /* when inputed dst IP parameter is not "any" */
        if ( VOS_StrCmp( argv[ 2 ] , "any" ) != 0 )
        {
            sDstRet = str_to_ipmask( argv[ 2 ], &stDstIp ) ;

            if ( sDstRet == ( SHORT ) VOS_ERROR )
            {
                qos_delete_access_list( pstAclNode );
                vty_out( vty, "%% Destination IP address is illegal.\r\n" ) ;
                return VOS_ERROR ;
            }
            pstAclNode->stACL.ulDestIP = stDstIp.ulIpAddress;
            pstAclNode->stACL.ucDestMaskLen = stDstIp.ucMaskLen;
            pstAclNode->stACL.ulFlag = pstAclNode->stACL.ulFlag | QOS_FILTER_DIP_USED;
        }


        /* when inputed src IP parameter is not "any" */
        if ( VOS_StrCmp( argv[ 3 ], "any" ) != 0 )
        {
            sSrcRet = str_to_ipmask( argv[ 3 ], &stSrcIp ) ;
            if ( sSrcRet == ( SHORT ) VOS_ERROR )
            {
                qos_delete_access_list( pstAclNode );
                vty_out( vty, "%% Source IP address is illegal.\r\n" ) ;
                return VOS_ERROR ;
            }
            pstAclNode->stACL.ulSrcIP = stSrcIp.ulIpAddress;
            pstAclNode->stACL.ucSrcMaskLen = stSrcIp.ucMaskLen;
            pstAclNode->stACL.ulFlag = pstAclNode->stACL.ulFlag | QOS_FILTER_SIP_USED;
        }


        /* no acl priority is inputed */
        if ( argc == 4 )
        {
            pstAclNode->ulPriority = 1;/*为兼容认证设定最小ACL 优先级为20*/
        }
        /* when acl priority is inputed */
        else
        {
           /* if ( ( VOS_StrToUL( argv[ 4 ], &pcTemp, 10 ) < 0 ) || ( VOS_StrToUL( argv[ 4 ], &pcTemp, 10 ) > QOS_ACL_MAX_PRECEDENCE ) )*/
	if  ( VOS_StrToUL( argv[ 4 ], &pcTemp, 10 ) > QOS_ACL_MAX_PRECEDENCE ) 
            {
                qos_delete_access_list( pstAclNode );
                vty_out( vty, "%% Precedence of access-list can only be 0-255!\r\n" ) ;
                return VOS_ERROR ;
            }
            pstAclNode->ulPriority = ( ULONG ) VOS_StrToUL( argv[ 4 ], &pcTemp, 10 );
        }
        /* set the IP protocol type */
        /* pstAclNode->stACL.sIpProtocl = 1; */ /* tcp:6   udp:17   icmp: 1*/
		if(QOS_TRUE==ulFlag)
			{
				pstAclNode->stACL.ulPppoeFlag=ulFlag;
				pstAclNode->stACL.usP2PProtocol=QOS_POINT_TO_POINT_PROTOCOL_FOR_PPPOE;
				pstAclNode->stACL.lEthType=QOS_ETHERNET_TYPE_IP_FOR_PPPOE;
 				pstAclNode->stACL.ulFlag |= QOS_FILTER_PPPOE;
			}

		/* added by fengna 11-29 */
		if (VOS_OK == qos_acl_search_acl_node_in_same_rule(pstAclNode, vty))
		{
			qos_delete_access_list(pstAclNode);
			vty_out(vty, "%% There is another access-list with the same rule!\r\n");
			return VOS_ERROR;
		}
        return VOS_OK;
    }



#endif

#ifdef  ONU_ACCESS_CONTROL_LIST_FILTER
#endif

#ifdef  ONU_IP_AND_PORT_FILTER
#endif

/** added by chenfj 2007-6-8 
       增加ONU 数据流IP/PORT过滤*/

int  V2R1_GetLongFromIpdotstring( unsigned char  * ipaddr, int  * pulMask )
{
	char bak[ 17 ];
	char *cp, *cp1, *cptail;
	unsigned char  ip[ 4 ];
	unsigned int i, j;
	int digit=0;


	/*first check whether it's a legal ipdot string */
	j = VOS_StrLen( ( CHAR* ) ipaddr );
	if ( ( j < 7 ) || ( j > 15 ) )
		return (RERROR);

	for ( i = 0;i < j;i++ )
		if ( !( ( *( ipaddr + i ) == '.' ) || ( ( *( ipaddr + i ) >= '0' ) && ( *( ipaddr + i ) <= '9' ) ) ) )
			return (RERROR);


	VOS_StrCpy( bak, ( CHAR* ) ipaddr );
	i = 0;
	cptail = bak;
	while ( i < 4 )
        {
          cp = cptail;
          if ( i != 3 )
            {
              cp1 = ( char * ) VOS_StrChr( cp, '.' );
              if ( !cp1 )
                return -1;
              cptail = cp1 + 1;
              *cp1 = 0;
            }
	digit = VOS_AtoI( cp );
	if( digit > 255 ) return( RERROR );
          ip[ i ] = ( UCHAR ) digit;
          i++;
        }

	i = 0;
	VOS_MemCpy( ( char * ) & i, ( ( char * ) ( &ip[ 0 ] ) ), 1 );
	VOS_MemCpy( ( ( char * ) & i ) + 1, ( ( char * ) ( &ip[ 1 ] ) ), 1 );
	VOS_MemCpy( ( ( char * ) & i ) + 2, ( ( char * ) ( &ip[ 2 ] ) ), 1 );
	VOS_MemCpy( ( ( char * ) & i ) + 3, ( ( char * ) ( &ip[ 3 ] ) ), 1 );
	*pulMask = i;

	return ROK;
}

int AddOnuSIpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr )
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		{
		ret = EnableOnuSIpFilter(PonPortIdx, OnuIdx, IpAddr);
		if( ret == ROK )
			ret = AddFilterSIpToTable( PonPortIdx, OnuIdx, IpAddr );
		}
	else {
		ret = AddFilterSIpToTable( PonPortIdx, OnuIdx, IpAddr );
		}

	return( ret );
}

int ClearOnuSIpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr )
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		DisableOnuSIpFilter( PonPortIdx, OnuIdx, IpAddr);

	ret = DelFilterSIpFromTable( PonPortIdx, OnuIdx, IpAddr);
	return( ret );
}

int ClearOnuSIpFilterAll( short int PonPortIdx, short int OnuIdx )
{
	int ret ;
	IpTable_S   *SIpAddr, *NextSIp;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	/*ONU_MGMT_SEM_TAKE;*/

	SIpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_SIp;
	while ( SIpAddr != NULL ) 
	{
		NextSIp = (IpTable_S *)SIpAddr->NextIp;
		ClearOnuSIpFilter( PonPortIdx, OnuIdx, SIpAddr->ipAddr);
	
		SIpAddr = NextSIp;
	}
	/*ONU_MGMT_SEM_GIVE;*/
	
	return( ret );
}

int AddOnuDIpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr )
{
	int ret = ROK ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		{
		ret = EnableOnuDIpFilter(PonPortIdx, OnuIdx, IpAddr);
		if( ret == ROK )
			ret = AddFilterDIpToTable( PonPortIdx, OnuIdx, IpAddr );
		}	
	else {
		ret = AddFilterDIpToTable( PonPortIdx, OnuIdx, IpAddr );
		}

	return( ret );
}

int ClearOnuDIpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr )
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		DisableOnuDIpFilter( PonPortIdx, OnuIdx, IpAddr);

	ret = DelFilterDIpFromTable( PonPortIdx, OnuIdx, IpAddr);
	return( ret );
}

int ClearOnuDIpFilterAll( short int PonPortIdx, short int OnuIdx )
{
	int ret ;
	IpTable_S   *DIpAddr, *NextDIp;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	/*ONU_MGMT_SEM_TAKE;*/

	DIpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_DIp;
	while ( DIpAddr != NULL ) 
	{
		NextDIp = (IpTable_S *)DIpAddr->NextIp;
		ClearOnuDIpFilter( PonPortIdx, OnuIdx, DIpAddr->ipAddr);
	
		DIpAddr = NextDIp;
	}
	/*ONU_MGMT_SEM_GIVE;*/
	
	return( ret );
}

int AddOnuSIpUdpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int  udp_port )
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
		{
		ret = EnableOnuSIpUdpFilter(PonPortIdx, OnuIdx, IpAddr, udp_port);
		if( ret == ROK ) 
			ret = AddFilterSIpUdpToTable( PonPortIdx, OnuIdx, IpAddr, udp_port );
		}
	else{
		ret = AddFilterSIpUdpToTable( PonPortIdx, OnuIdx, IpAddr, udp_port );
		}

	return( ret );
}

int ClearOnuSIpUdpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_port )
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		DisableOnuSIpUdpFilter( PonPortIdx, OnuIdx, IpAddr, udp_port);

	ret = DelFilterSIpUdpFromTable( PonPortIdx, OnuIdx, IpAddr, udp_port);
	return( ret );
}

int ClearOnuSIpUdpFilterAll( short int PonPortIdx, short int OnuIdx )
{
	int ret ;
	Ip_Port_Table_S   *SIpAddr, *NextSIp;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	/*ONU_MGMT_SEM_TAKE;*/

	SIpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_SIp_udp;
	while ( SIpAddr != NULL ) 
	{
		NextSIp = (Ip_Port_Table_S *)SIpAddr->Next_Ip_Port;
		ClearOnuSIpUdpFilter( PonPortIdx, OnuIdx, SIpAddr->ipAddr, SIpAddr->port);
	
		SIpAddr = NextSIp;
	}
	/*ONU_MGMT_SEM_GIVE;*/
	
	return( ret );
}

int AddOnuDIpUdpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_port)
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
		{
		ret = EnableOnuDIpUdpFilter(PonPortIdx, OnuIdx, IpAddr, udp_port );
		if( ret == ROK ) 
			ret = AddFilterDIpUdpToTable( PonPortIdx, OnuIdx, IpAddr, udp_port );
		}
	else{
		ret = AddFilterDIpUdpToTable( PonPortIdx, OnuIdx, IpAddr, udp_port );
		}

	return( ret );
}

int ClearOnuDIpUdpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_port )
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		DisableOnuDIpUdpFilter( PonPortIdx, OnuIdx, IpAddr, udp_port);

	ret = DelFilterDIpUdpFromTable( PonPortIdx, OnuIdx, IpAddr, udp_port);
	return( ret );
}

int ClearOnuDIpUdpFilterAll( short int PonPortIdx, short int OnuIdx )
{
	int ret ;
	Ip_Port_Table_S   *DIpAddr, *NextDIp;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	/*ONU_MGMT_SEM_TAKE;*/

	DIpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_DIp_udp;
	while ( DIpAddr != NULL ) 
	{
		NextDIp = (Ip_Port_Table_S *)DIpAddr->Next_Ip_Port;
		ClearOnuDIpUdpFilter( PonPortIdx, OnuIdx, DIpAddr->ipAddr, DIpAddr->port);
	
		DIpAddr = NextDIp;
	}
/*	ONU_MGMT_SEM_GIVE;*/
	
	return( ret );
}

int AddOnuSIpTcpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_port)
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		{
		ret = EnableOnuSIpTcpFilter(PonPortIdx, OnuIdx, IpAddr, tcp_port );
		if( ret == ROK )
			ret = AddFilterSIpTcpToTable( PonPortIdx, OnuIdx, IpAddr, tcp_port );
		}
	else{
		ret = AddFilterSIpTcpToTable( PonPortIdx, OnuIdx, IpAddr, tcp_port );
		}

	return( ret );
}

int ClearOnuSIpTcpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_port )
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		DisableOnuSIpTcpFilter( PonPortIdx, OnuIdx, IpAddr, tcp_port);

	ret = DelFilterSIpTcpFromTable( PonPortIdx, OnuIdx, IpAddr, tcp_port);
	return( ret );
}

int ClearOnuSIpTcpFilterAll( short int PonPortIdx, short int OnuIdx )
{
	int ret ;
	Ip_Port_Table_S   *DIpAddr, *NextDIp;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

/*	ONU_MGMT_SEM_TAKE;*/

	DIpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_SIp_tcp;
	while ( DIpAddr != NULL ) 
	{
		NextDIp = (Ip_Port_Table_S *)DIpAddr->Next_Ip_Port;
		ClearOnuSIpTcpFilter( PonPortIdx, OnuIdx, DIpAddr->ipAddr, DIpAddr->port);
	
		DIpAddr = NextDIp;
	}
/*	ONU_MGMT_SEM_GIVE;*/
	
	return( ret );
}

int AddOnuDIpTcpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_port)
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
		{
		ret = EnableOnuDIpTcpFilter(PonPortIdx, OnuIdx, IpAddr, tcp_port );
		if(ret == ROK )
			ret = AddFilterDIpTcpToTable( PonPortIdx, OnuIdx, IpAddr, tcp_port );
		}
	else{
		ret = AddFilterDIpTcpToTable( PonPortIdx, OnuIdx, IpAddr, tcp_port );
		}

	return( ret );
}

int ClearOnuDIpTcpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_port )
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		DisableOnuDIpTcpFilter( PonPortIdx, OnuIdx, IpAddr, tcp_port);

	ret = DelFilterDIpTcpFromTable( PonPortIdx, OnuIdx, IpAddr, tcp_port);
	return( ret );
}

int ClearOnuDIpTcpFilterAll( short int PonPortIdx, short int OnuIdx )
{
	int ret ;
	Ip_Port_Table_S   *DIpAddr, *NextDIp;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

/*	ONU_MGMT_SEM_TAKE;*/

	DIpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_DIp_tcp;
	while ( DIpAddr != NULL ) 
	{
		NextDIp = (Ip_Port_Table_S *)DIpAddr->Next_Ip_Port;
		ClearOnuDIpTcpFilter( PonPortIdx, OnuIdx, DIpAddr->ipAddr, DIpAddr->port);
	
		DIpAddr = NextDIp;
	}
/*	ONU_MGMT_SEM_GIVE;*/
	
	return( ret );
}

int EnableOnuSIpFilter( short int PonPortIdx ,short int OnuIdx , unsigned int  IpAddr )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr,  0, PON_DONT_PASS );
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr,  0, PON_DONT_PASS );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr,  0, PON_DONT_PASS );
			ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr,  0, PON_DONT_PASS );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr,  0, PON_DONT_PASS );
			ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr,  0, PON_DONT_PASS );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK )||( ret == REMOTE_MANAGEMENT_ALREADY_EXISTS )) return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int EnableOnuSIpFilterAll( short int PonPortIdx ,short int OnuIdx )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret = VOS_ERROR;
	
	IpTable_S  *IpAddr;
	
	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
	{
		/*REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );*/
/*		ONU_MGMT_SEM_TAKE;*/

		IpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_SIp;
		while( IpAddr != NULL )
		{
			/*sys_console_printf("add source filter IP addr: 0x%x\r\n", IpAddr->ipAddr );*/
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr->ipAddr,  0, PON_DONT_PASS );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr->ipAddr,  0, PON_DONT_PASS );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr->ipAddr,  0, PON_DONT_PASS );
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr->ipAddr,  0, PON_DONT_PASS );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr->ipAddr,  0, PON_DONT_PASS );
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr->ipAddr,  0, PON_DONT_PASS );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			IpAddr = (IpTable_S *)IpAddr->NextIp;
		}
/*		ONU_MGMT_SEM_GIVE;*/
		if(( ret == PAS_EXIT_OK ) || ( ret == REMOTE_MANAGEMENT_ALREADY_EXISTS ) ) return( ROK ) ;
		else return( RERROR );
	}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int  DisableOnuSIpFilter( short int PonPortIdx ,short int OnuIdx, unsigned int  IpAddr )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*	ret = REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id ); */
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr,  0 );
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr,  0 );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr,  0 );
			ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr,  0 );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr,  0 );
			ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr,  0 );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK ) ;
		else return( RERROR );
		}
	
	else { /* other pon chip handler */

		}
	return( RERROR );
}

int  DisableOnuSIpFilterAll( short int PonPortIdx ,short int OnuIdx )
{
	short int PonChipType;
	short int Onu_id;
	short int ret = RERROR;
	IpTable_S  *IpAddr;
	
	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
	{
		/*REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );*/
/*		ONU_MGMT_SEM_TAKE;*/

		IpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_SIp;
		while( IpAddr != NULL )
		{
			/*sys_console_printf("del source filter IP addr: 0x%x\r\n", IpAddr->ipAddr );*/
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr->ipAddr,  0 );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr->ipAddr,  0 );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr->ipAddr,  0 );
				ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr->ipAddr,  0 );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr->ipAddr,  0 );
				ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_SOURCE, IpAddr->ipAddr,  0 );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			IpAddr = (IpTable_S *)IpAddr->NextIp;
		}
/*		ONU_MGMT_SEM_GIVE;*/
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED ) )return( ROK ) ;
		else return( RERROR );
	}
	else { /* other pon chip handler */

	}
	return( RERROR );

}

int EnableOnuDIpFilter( short int PonPortIdx ,short int OnuIdx , unsigned int  IpAddr )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr,  0, PON_DONT_PASS );
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr,  0, PON_DONT_PASS );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr,  0, PON_DONT_PASS );
			ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr,  0, PON_DONT_PASS );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr,  0, PON_DONT_PASS );
			ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr,  0, PON_DONT_PASS );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK )||( ret == REMOTE_MANAGEMENT_ALREADY_EXISTS )) return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int EnableOnuDIpFilterAll( short int PonPortIdx ,short int OnuIdx )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret = RERROR;
	IpTable_S  *IpAddr;
	
	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );*/
/*		ONU_MGMT_SEM_TAKE;*/

		IpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_DIp;
		while( IpAddr != NULL )
			{
			/*sys_console_printf("add destination filter IP addr: 0x%x\r\n", IpAddr->ipAddr);*/
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr->ipAddr,  0, PON_DONT_PASS );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr->ipAddr,  0, PON_DONT_PASS );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr->ipAddr,  0, PON_DONT_PASS );
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr->ipAddr,  0, PON_DONT_PASS );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr->ipAddr,  0, PON_DONT_PASS );
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr->ipAddr,  0, PON_DONT_PASS );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			IpAddr = (IpTable_S *)IpAddr->NextIp;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if( ret == PAS_EXIT_OK ) return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int  DisableOnuDIpFilter( short int PonPortIdx ,short int OnuIdx, unsigned int  IpAddr )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*	
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*	ret = REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id ); */
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr,  0 );
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr,  0 );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr,  0 );
			ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr,  0 );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr,  0 );
			ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr,  0 );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK ) ;
		else return( RERROR );
		}
	
	else { /* other pon chip handler */

		}
	return( RERROR );
}

int  DisableOnuDIpFilterAll( short int PonPortIdx ,short int OnuIdx )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret = RERROR;

	IpTable_S  *IpAddr;
	
	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );*/
/*		ONU_MGMT_SEM_TAKE;*/

		IpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_DIp;
		while( IpAddr != NULL )
			{
			/*sys_console_printf("del destination filter IP addr: 0x%x\r\n", IpAddr->ipAddr );*/
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr->ipAddr,  0 );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr->ipAddr,  0 );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr->ipAddr,  0 );
				ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr->ipAddr,  0 );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr->ipAddr,  0 );
				ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_IP_ADDRESS, PON_TRAFFIC_DESTINATION, IpAddr->ipAddr,  0 );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			IpAddr = (IpTable_S *)IpAddr->NextIp;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED ) )return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int EnableOnuSIpUdpFilter( short int PonPortIdx ,short int OnuIdx , unsigned int  IpAddr, unsigned short int  udp_port )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
		
	IpAddr = INVALID_IP;
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		if( IpAddr == INVALID_IP )
			{
			    IpAddr = 0;
			}
        
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_IP_AND_UDP_PORT, PON_TRAFFIC_SOURCE, IpAddr,  udp_port, PON_DONT_PASS );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_IP_AND_UDP_PORT, PON_TRAFFIC_SOURCE, IpAddr,  udp_port, PON_DONT_PASS );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_IP_AND_UDP_PORT, PON_TRAFFIC_SOURCE, IpAddr,  udp_port, PON_DONT_PASS );
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_IP_AND_UDP_PORT, PON_TRAFFIC_SOURCE, IpAddr,  udp_port, PON_DONT_PASS );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_IP_AND_UDP_PORT, PON_TRAFFIC_SOURCE, IpAddr,  udp_port, PON_DONT_PASS );
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_IP_AND_UDP_PORT, PON_TRAFFIC_SOURCE, IpAddr,  udp_port, PON_DONT_PASS );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
            }

		if( (ret == PAS_EXIT_OK )||( ret == REMOTE_MANAGEMENT_ALREADY_EXISTS )) return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int EnableOnuSIpUdpFilterAll( short int PonPortIdx ,short int OnuIdx )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret = RERROR;
	Ip_Port_Table_S  *IpAddr;
	
	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );*/
/*		ONU_MGMT_SEM_TAKE;*/

		IpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_SIp_udp;
		while( IpAddr != NULL )
		{
			/*sys_console_printf("add source filter IP addr: 0x%x, udp port %d\r\n", IpAddr->ipAddr, IpAddr->Next_Ip_Port);*/
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			IpAddr = (Ip_Port_Table_S *)IpAddr->Next_Ip_Port;
		}
/*		ONU_MGMT_SEM_GIVE;*/
		if(( ret == PAS_EXIT_OK ) || ( ret == REMOTE_MANAGEMENT_ALREADY_EXISTS ) ) return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int  DisableOnuSIpUdpFilter( short int PonPortIdx ,short int OnuIdx, unsigned int  IpAddr, unsigned short int udp_port  )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	IpAddr = INVALID_IP;
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		if( IpAddr == INVALID_IP )
			{
			IpAddr = 0;
		}
        
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_IP_AND_UDP_PORT, PON_TRAFFIC_SOURCE, IpAddr,  udp_port );
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_IP_AND_UDP_PORT, PON_TRAFFIC_SOURCE, IpAddr,  udp_port );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_IP_AND_UDP_PORT, PON_TRAFFIC_SOURCE, IpAddr,  udp_port );
			ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_IP_AND_UDP_PORT, PON_TRAFFIC_SOURCE, IpAddr,  udp_port );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_IP_AND_UDP_PORT, PON_TRAFFIC_SOURCE, IpAddr,  udp_port );
			ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_IP_AND_UDP_PORT, PON_TRAFFIC_SOURCE, IpAddr,  udp_port );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK ) ;
		else return( RERROR );
		}
	
	else { /* other pon chip handler */

		}
	return( RERROR );
}

int  DisableOnuSIpUdpFilterAll( short int PonPortIdx ,short int OnuIdx )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret = RERROR;
	Ip_Port_Table_S  *IpAddr;
	
	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );*/
/*		ONU_MGMT_SEM_TAKE;*/

		IpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_SIp_udp;
		while( IpAddr != NULL )
			{
			/*sys_console_printf("del source filter IP addr: 0x%x, udp port %d\r\n", IpAddr->ipAddr, IpAddr->Next_Ip_Port );*/
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
				ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
				ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			IpAddr = (Ip_Port_Table_S *)IpAddr->Next_Ip_Port;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED ) )return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int EnableOnuDIpUdpFilter( short int PonPortIdx ,short int OnuIdx , unsigned int  IpAddr, unsigned short int  udp_port )
{
	short int PonChipType;
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  udp_port, PON_DONT_PASS );
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  udp_port, PON_DONT_PASS );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  udp_port, PON_DONT_PASS );
			ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  udp_port, PON_DONT_PASS );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  udp_port, PON_DONT_PASS );
			ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  udp_port, PON_DONT_PASS );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK )||( ret == REMOTE_MANAGEMENT_ALREADY_EXISTS )) return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int EnableOnuDIpUdpFilterAll( short int PonPortIdx ,short int OnuIdx )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret = RERROR;
	Ip_Port_Table_S  *IpAddr;
	
	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );*/
/*		ONU_MGMT_SEM_TAKE;*/

		IpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_DIp_udp;
		while( IpAddr != NULL )
			{
			/*sys_console_printf("add destination filter IP addr: 0x%x, udp port %d\r\n", IpAddr->ipAddr, IpAddr->Next_Ip_Port);*/
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			IpAddr = (Ip_Port_Table_S *)IpAddr->Next_Ip_Port;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if(( ret == PAS_EXIT_OK ) || ( ret == REMOTE_MANAGEMENT_ALREADY_EXISTS ) ) return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int  DisableOnuDIpUdpFilter( short int PonPortIdx ,short int OnuIdx, unsigned int  IpAddr, unsigned short int udp_port  )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*	ret = REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id ); */
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  udp_port );
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  udp_port );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  udp_port );
			ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  udp_port );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  udp_port );
			ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  udp_port );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK ) ;
		else return( RERROR );
		}
	
	else { /* other pon chip handler */

		}
	return( RERROR );
}

int  DisableOnuDIpUdpFilterAll( short int PonPortIdx ,short int OnuIdx )
{
	short int PonChipType;
	short int Onu_id;
	short int ret = RERROR;
	Ip_Port_Table_S  *IpAddr;
	
	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType))
		{
		/*REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );*/
/*		ONU_MGMT_SEM_TAKE;*/

		IpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_DIp_udp;
		while( IpAddr != NULL )
			{
			/*sys_console_printf("del destination filter IP addr: 0x%x, udp port %d\r\n", IpAddr->ipAddr, IpAddr->Next_Ip_Port );*/
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType))
			{
				ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
				ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
				ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_UDP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			IpAddr = (Ip_Port_Table_S *)IpAddr->Next_Ip_Port;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED ) )return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int EnableOnuSIpTcpFilter( short int PonPortIdx ,short int OnuIdx , unsigned int  IpAddr, unsigned short int  tcp_port )
{
	short int PonChipType;
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr*/, tcp_port, PON_DONT_PASS );
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr*/, tcp_port, PON_DONT_PASS );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr*/, tcp_port, PON_DONT_PASS );
			ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr*/, tcp_port, PON_DONT_PASS );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr*/, tcp_port, PON_DONT_PASS );
			ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr*/, tcp_port, PON_DONT_PASS );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK )||( ret == REMOTE_MANAGEMENT_ALREADY_EXISTS )) return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int EnableOnuSIpTcpFilterAll( short int PonPortIdx ,short int OnuIdx )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	Ip_Port_Table_S  *IpAddr;
	
	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );*/
/*		ONU_MGMT_SEM_TAKE;*/

		IpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_SIp_tcp;
		while( IpAddr != NULL )
			{
			/*sys_console_printf("add source filter IP addr: 0x%x, tcp port %d\r\n", IpAddr->ipAddr, IpAddr->Next_Ip_Port);*/
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			IpAddr = (Ip_Port_Table_S *)IpAddr->Next_Ip_Port;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if(( ret == PAS_EXIT_OK ) || ( ret == REMOTE_MANAGEMENT_ALREADY_EXISTS ) ) return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int  DisableOnuSIpTcpFilter( short int PonPortIdx ,short int OnuIdx, unsigned int  IpAddr, unsigned short int tcp_port  )
{
	short int PonChipType;
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	

	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*	ret = REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id ); */
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr*/,  tcp_port );
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr*/,  tcp_port );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr*/,  tcp_port );
			ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr*/,  tcp_port );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr*/,  tcp_port );
			ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr*/,  tcp_port );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK ) ;
		else return( RERROR );
		}
	
	else { /* other pon chip handler */

		}
	return( RERROR );
}

int  DisableOnuSIpTcpFilterAll( short int PonPortIdx ,short int OnuIdx )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret = RERROR;
	Ip_Port_Table_S  *IpAddr;
	
	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );*/
/*		ONU_MGMT_SEM_TAKE;*/

		IpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_SIp_tcp;
		while( IpAddr != NULL )
			{
			/*sys_console_printf("del source filter IP addr: 0x%x, tcp port %d\r\n", IpAddr->ipAddr, IpAddr->Next_Ip_Port );*/
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
				ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
				ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_SOURCE, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			IpAddr = (Ip_Port_Table_S *)IpAddr->Next_Ip_Port;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED ) )return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int EnableOnuDIpTcpFilter( short int PonPortIdx ,short int OnuIdx , unsigned int  IpAddr, unsigned short int  tcp_port )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/, tcp_port, PON_DONT_PASS );
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/, tcp_port, PON_DONT_PASS );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/, tcp_port, PON_DONT_PASS );
			ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/, tcp_port, PON_DONT_PASS );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/, tcp_port, PON_DONT_PASS );
			ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/, tcp_port, PON_DONT_PASS );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK )||( ret == REMOTE_MANAGEMENT_ALREADY_EXISTS )) return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int EnableOnuDIpTcpFilterAll( short int PonPortIdx ,short int OnuIdx )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret = RERROR;
	Ip_Port_Table_S  *IpAddr;
	
	CHECK_ONU_RANGE;
	/*
	int  retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );*/
/*		ONU_MGMT_SEM_TAKE;*/

		IpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_DIp_tcp;
		while( IpAddr != NULL )
			{
			/*sys_console_printf("add destination filter IP addr: 0x%x, tcp port %d\r\n", IpAddr->ipAddr, IpAddr->Next_Ip_Port);*/
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
				ret = REMOTE_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
				ret = TK_PASONU_classifier_l3l4_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port, PON_DONT_PASS );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			IpAddr = (Ip_Port_Table_S *)IpAddr->Next_Ip_Port;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if(( ret == PAS_EXIT_OK ) || ( ret == REMOTE_MANAGEMENT_ALREADY_EXISTS ) ) return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}

int  DisableOnuDIpTcpFilter( short int PonPortIdx ,short int OnuIdx, unsigned int  IpAddr, unsigned short int tcp_port  )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE

	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType))
		{
		/*	ret = REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id ); */
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  tcp_port );
			ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  tcp_port );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType))
		{
			ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  tcp_port );
			ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  tcp_port );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  tcp_port );
			ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr*/,  tcp_port );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK ) ;
		else return( RERROR );
		}
	
	else { /* other pon chip handler */

		}
	return( RERROR );
}

int  DisableOnuDIpTcpFilterAll( short int PonPortIdx ,short int OnuIdx )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret = RERROR;
	Ip_Port_Table_S  *IpAddr;
	
	CHECK_ONU_RANGE;
	/*
	int  retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*REMOTE_PASONU_address_table_clear( PonPortIdx, Onu_id );*/
/*		ONU_MGMT_SEM_TAKE;*/

		IpAddr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_DIp_tcp;
		while( IpAddr != NULL )
			{
			/*sys_console_printf("del destination filter IP addr: 0x%x, tcp port %d\r\n", IpAddr->ipAddr, IpAddr->Next_Ip_Port );*/
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
				ret = GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
				ret = REMOTE_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK,PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
				ret = TK_PASONU_classifier_l3l4_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK,PON_TRAFFIC_TCP_PORT, PON_TRAFFIC_DESTINATION, 0/*IpAddr->ipAddr*/,  IpAddr->Next_Ip_Port );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			IpAddr = (Ip_Port_Table_S *)IpAddr->Next_Ip_Port;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if( (ret == PAS_EXIT_OK ) || ( ret == REMOTE_PASONU_QUERY_FAILED ) )return( ROK ) ;
		else return( RERROR );
		}
	else { /* other pon chip handler */

		}
	return( RERROR );

}


int  AddFilterSIpToTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr )
{
	int onuEntry;
	IpTable_S  *Ip_Ptr, *Ip_Ptr_cur;

	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue =ThisIsValidOnu(PonPortIdx, OnuIdx);
	if(  retValue != ROK ) return( retValue );
	*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Ptr = OnuMgmtTable[onuEntry].Filter_SIp;
	Ip_Ptr_cur = Ip_Ptr;
	while( Ip_Ptr != NULL )
		{
		if( Ip_Ptr->ipAddr == IpAddr ) 
		{
/*			ONU_MGMT_SEM_GIVE;*/
			return( V2R1_ONU_FILTER_IP_EXIST );
		}
		Ip_Ptr_cur = Ip_Ptr;
		Ip_Ptr = ( IpTable_S *)Ip_Ptr->NextIp;
		}
/*	ONU_MGMT_SEM_GIVE;*/
	Ip_Ptr = ( IpTable_S *) VOS_Malloc( sizeof(IpTable_S ), MODULE_ONU );
	if( Ip_Ptr == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}
	
	Ip_Ptr ->ipAddr = IpAddr;
	Ip_Ptr->NextIp = 0;

	/*sys_console_printf("IP addr: 0x%x\r\n", (int)Ip_Ptr->ipAddr);*/

/*	ONU_MGMT_SEM_TAKE;*/
	if( Ip_Ptr_cur == 0 ) OnuMgmtTable[onuEntry].Filter_SIp = Ip_Ptr;
	else  Ip_Ptr_cur ->NextIp = (unsigned int ) Ip_Ptr;
/*	ONU_MGMT_SEM_GIVE;*/

	return( ROK );	

}

int DelFilterSIpFromTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr)
{
	int onuEntry;
	IpTable_S  *Ip_Ptr,*Ip_Ptr_cur;

	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue =ThisIsValidOnu(PonPortIdx, OnuIdx);
	if(  retValue != ROK ) return( retValue );
	*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Ptr = OnuMgmtTable[onuEntry].Filter_SIp;
	Ip_Ptr_cur = Ip_Ptr;
	while( Ip_Ptr != NULL )
		{
		if( Ip_Ptr->ipAddr == IpAddr ) 
			{
			if(  Ip_Ptr_cur ==  Ip_Ptr )  /* 链表的第一个节点*/
				{ 
				OnuMgmtTable[onuEntry].Filter_SIp = ( IpTable_S *)Ip_Ptr->NextIp;
				}
			else {
				Ip_Ptr_cur->NextIp = Ip_Ptr->NextIp;
				}
			VOS_Free((void *)Ip_Ptr );
/*			ONU_MGMT_SEM_GIVE;*/
			return( ROK );
			}
		Ip_Ptr_cur = Ip_Ptr;
		Ip_Ptr = (IpTable_S *)Ip_Ptr->NextIp;
		}
/*	ONU_MGMT_SEM_GIVE;*/

	return( V2R1_ONU_FILTER_IP_NOT_EXIST );
}

int  AddFilterDIpToTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr )
{
	int onuEntry;
	IpTable_S  *Ip_Ptr, *Ip_Ptr_cur;

	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue =ThisIsValidOnu(PonPortIdx, OnuIdx);
	if(  retValue != ROK ) return( retValue );
	*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Ptr = OnuMgmtTable[onuEntry].Filter_DIp;
	Ip_Ptr_cur = Ip_Ptr;
	while( Ip_Ptr != NULL )
		{
		if( Ip_Ptr->ipAddr == IpAddr ) 
		{
/*			ONU_MGMT_SEM_GIVE;*/
			return( V2R1_ONU_FILTER_IP_EXIST );
		}
		Ip_Ptr_cur = Ip_Ptr;
		Ip_Ptr = ( IpTable_S *)Ip_Ptr->NextIp;
		}
/*	ONU_MGMT_SEM_GIVE;*/
	Ip_Ptr = ( IpTable_S *) VOS_Malloc( sizeof(IpTable_S ), MODULE_ONU );
	if( Ip_Ptr == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}
	/*sys_console_printf("IP addr: 0x%x\r\n", (int)Ip_Ptr);*/
	Ip_Ptr ->ipAddr = IpAddr;
	Ip_Ptr->NextIp = 0;

/*	ONU_MGMT_SEM_TAKE;*/
	if( Ip_Ptr_cur == 0 ) OnuMgmtTable[onuEntry].Filter_DIp = Ip_Ptr;
	else  Ip_Ptr_cur ->NextIp = (unsigned int )Ip_Ptr;
/*	ONU_MGMT_SEM_GIVE;*/

	return( ROK );	
}

int DelFilterDIpFromTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr)
{
	int onuEntry;
	IpTable_S  *Ip_Ptr, *Ip_Ptr_cur;

	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue =ThisIsValidOnu(PonPortIdx, OnuIdx);
	if(  retValue != ROK ) return( retValue );
	*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Ptr = OnuMgmtTable[onuEntry].Filter_DIp;
	Ip_Ptr_cur = Ip_Ptr;
	while( Ip_Ptr != NULL )
		{
		if( Ip_Ptr->ipAddr == IpAddr ) 
			{
			if(  Ip_Ptr_cur ==  Ip_Ptr )  /* 链表的第一个节点*/
				{ 
				OnuMgmtTable[onuEntry].Filter_DIp = (IpTable_S *)Ip_Ptr->NextIp;
				}
			else {
				Ip_Ptr_cur->NextIp = Ip_Ptr->NextIp;
				}
			VOS_Free((void *)Ip_Ptr );
/*			ONU_MGMT_SEM_GIVE;*/
			return( ROK );
			}
		Ip_Ptr_cur = Ip_Ptr;
		Ip_Ptr = (IpTable_S *)Ip_Ptr->NextIp;
		}
/*	ONU_MGMT_SEM_GIVE;*/

	return( V2R1_ONU_FILTER_IP_NOT_EXIST );
}

int  AddFilterSIpUdpToTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_Port )
{
	int onuEntry;
	Ip_Port_Table_S  *Ip_Port_Ptr, *Ip_Port_cur;

	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue =ThisIsValidOnu(PonPortIdx, OnuIdx);
	if(  retValue != ROK ) return( retValue );
	*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Port_Ptr = OnuMgmtTable[onuEntry].Filter_SIp_udp;
	Ip_Port_cur = Ip_Port_Ptr;
	while( Ip_Port_Ptr != NULL )
		{
		if(( Ip_Port_Ptr->ipAddr == IpAddr ) &&(Ip_Port_Ptr->port ==  udp_Port ))
		{
/*			ONU_MGMT_SEM_GIVE;*/
			return( V2R1_ONU_FILTER_IP_UDP_EXIST );
		}
		Ip_Port_cur = Ip_Port_Ptr;
		Ip_Port_Ptr = (Ip_Port_Table_S *)Ip_Port_Ptr->Next_Ip_Port;
		}
/*	ONU_MGMT_SEM_GIVE;*/
	Ip_Port_Ptr = ( Ip_Port_Table_S *) VOS_Malloc( sizeof(Ip_Port_Table_S ), MODULE_ONU );
	if( Ip_Port_Ptr == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}
	Ip_Port_Ptr ->ipAddr = IpAddr;
	Ip_Port_Ptr->port = udp_Port;
	Ip_Port_Ptr->Next_Ip_Port = 0;
	/*sys_console_printf("\r\n Sip Ipaddr 0x%8x, udp %d \r\n", IpAddr, udp_Port );*/

/*	ONU_MGMT_SEM_TAKE;*/
	if( Ip_Port_cur == 0 ) OnuMgmtTable[onuEntry].Filter_SIp_udp = Ip_Port_Ptr;
	else  Ip_Port_cur ->Next_Ip_Port = (unsigned int )Ip_Port_Ptr;
/*	ONU_MGMT_SEM_GIVE;*/

	return( ROK );	

}

int DelFilterSIpUdpFromTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_port)
{
	int onuEntry;
	Ip_Port_Table_S  *Ip_Port_Ptr, *Ip_Port_cur;

	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue =ThisIsValidOnu(PonPortIdx, OnuIdx);
	if(  retValue != ROK ) return( retValue );
	*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Port_Ptr = OnuMgmtTable[onuEntry].Filter_SIp_udp;
	Ip_Port_cur = Ip_Port_Ptr;
	while( Ip_Port_Ptr != NULL )
		{
		if(( Ip_Port_Ptr->ipAddr == IpAddr ) && (Ip_Port_Ptr->port == udp_port ))
			{
			if(  Ip_Port_cur ==  Ip_Port_Ptr )  /* 链表的第一个节点*/
				{ 
				OnuMgmtTable[onuEntry].Filter_SIp_udp = (Ip_Port_Table_S *)Ip_Port_Ptr->Next_Ip_Port;
				}
			else {
				Ip_Port_cur->Next_Ip_Port = Ip_Port_Ptr->Next_Ip_Port;
				}
			VOS_Free((void *)Ip_Port_Ptr );
/*			ONU_MGMT_SEM_GIVE;*/
			return( ROK );
			}
		Ip_Port_cur = Ip_Port_Ptr;
		Ip_Port_Ptr = (Ip_Port_Table_S *)Ip_Port_Ptr->Next_Ip_Port;
		}
/*	ONU_MGMT_SEM_GIVE;*/

	return( V2R1_ONU_FILTER_IP_UDP_NOT_EXIST );
}

int  AddFilterDIpUdpToTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_Port )
{
	int onuEntry;
	Ip_Port_Table_S  *Ip_Port_Ptr, *Ip_Port_cur;

	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue =ThisIsValidOnu(PonPortIdx, OnuIdx);
	if(  retValue != ROK ) return( retValue );
	*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Port_Ptr = OnuMgmtTable[onuEntry].Filter_DIp_udp;
	Ip_Port_cur = Ip_Port_Ptr;
	while( Ip_Port_Ptr != NULL )
		{
		if(( Ip_Port_Ptr->ipAddr == IpAddr ) &&(Ip_Port_Ptr->port ==  udp_Port ))
		{
/*			ONU_MGMT_SEM_GIVE;*/
			return( V2R1_ONU_FILTER_IP_UDP_EXIST );
		}
		Ip_Port_cur = Ip_Port_Ptr;
		Ip_Port_Ptr = (Ip_Port_Table_S *)Ip_Port_Ptr->Next_Ip_Port;
		}
/*	ONU_MGMT_SEM_GIVE;*/
	Ip_Port_Ptr = ( Ip_Port_Table_S *) VOS_Malloc( sizeof(Ip_Port_Table_S ), MODULE_ONU );
	if( Ip_Port_Ptr == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}
	Ip_Port_Ptr ->ipAddr = IpAddr;
	Ip_Port_Ptr->port = udp_Port;
	Ip_Port_Ptr->Next_Ip_Port = 0;
	/*sys_console_printf("\r\n Dip Ipaddr 0x%8x, udp %d \r\n", IpAddr, udp_Port );*/

/*	ONU_MGMT_SEM_TAKE;*/
	if( Ip_Port_cur == 0 ) OnuMgmtTable[onuEntry].Filter_DIp_udp = Ip_Port_Ptr;
	else  Ip_Port_cur ->Next_Ip_Port = (unsigned int )Ip_Port_Ptr;
/*	ONU_MGMT_SEM_GIVE;*/

	return( ROK );	

}

int DelFilterDIpUdpFromTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_port)
{
	int onuEntry;
	Ip_Port_Table_S  *Ip_Port_Ptr, *Ip_Port_cur;

	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue =ThisIsValidOnu(PonPortIdx, OnuIdx);
	if(  retValue != ROK ) return( retValue );
	*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Port_Ptr = OnuMgmtTable[onuEntry].Filter_DIp_udp;
	Ip_Port_cur = Ip_Port_Ptr;
	while( Ip_Port_Ptr != NULL )
		{
		if(( Ip_Port_Ptr->ipAddr == IpAddr ) && (Ip_Port_Ptr->port == udp_port ))
			{
			if(  Ip_Port_cur ==  Ip_Port_Ptr )  /* 链表的第一个节点*/
				{ 
				OnuMgmtTable[onuEntry].Filter_DIp_udp = (Ip_Port_Table_S *)Ip_Port_Ptr->Next_Ip_Port;
				}
			else {
				Ip_Port_cur->Next_Ip_Port = Ip_Port_Ptr->Next_Ip_Port;
				}
			VOS_Free((void *)Ip_Port_Ptr );
/*			ONU_MGMT_SEM_GIVE;*/
			return( ROK );
			}
		Ip_Port_cur = Ip_Port_Ptr;
		Ip_Port_Ptr = (Ip_Port_Table_S *)Ip_Port_Ptr->Next_Ip_Port;
		}
/*	ONU_MGMT_SEM_GIVE;*/

	return( V2R1_ONU_FILTER_IP_UDP_NOT_EXIST );
}

int  AddFilterSIpTcpToTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_Port )
{
	int onuEntry;
	Ip_Port_Table_S  *Ip_Port_Ptr, *Ip_Port_cur;

	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue =ThisIsValidOnu(PonPortIdx, OnuIdx);
	if(  retValue != ROK ) return( retValue );
	*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Port_Ptr = OnuMgmtTable[onuEntry].Filter_SIp_tcp;
	Ip_Port_cur = Ip_Port_Ptr;
	while( Ip_Port_Ptr != NULL )
		{
		if(( Ip_Port_Ptr->ipAddr == IpAddr ) &&(Ip_Port_Ptr->port ==  tcp_Port ))
		{
			return( V2R1_ONU_FILTER_IP_TCP_EXIST );
/*			ONU_MGMT_SEM_GIVE;*/
		}
		Ip_Port_cur = Ip_Port_Ptr;
		Ip_Port_Ptr = (Ip_Port_Table_S *)Ip_Port_Ptr->Next_Ip_Port;
		}
/*	ONU_MGMT_SEM_GIVE;*/
	Ip_Port_Ptr = ( Ip_Port_Table_S *) VOS_Malloc( sizeof(Ip_Port_Table_S ), MODULE_ONU );
	if( Ip_Port_Ptr == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}
	Ip_Port_Ptr ->ipAddr = IpAddr;
	Ip_Port_Ptr->port = tcp_Port;
	Ip_Port_Ptr->Next_Ip_Port = 0;
	/*sys_console_printf("\r\n Sip Ipaddr 0x%8x, tcp %d \r\n", IpAddr, tcp_Port );*/

/*	ONU_MGMT_SEM_TAKE;*/
	if( Ip_Port_cur == 0 ) OnuMgmtTable[onuEntry].Filter_SIp_tcp = Ip_Port_Ptr;
	else  Ip_Port_cur ->Next_Ip_Port = (unsigned int )Ip_Port_Ptr;
/*	ONU_MGMT_SEM_GIVE;*/

	return( ROK );	

}

int DelFilterSIpTcpFromTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_port)
{
	int onuEntry;
	Ip_Port_Table_S  *Ip_Port_Ptr, *Ip_Port_cur;

	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue =ThisIsValidOnu(PonPortIdx, OnuIdx);
	if(  retValue != ROK ) return( retValue );
	*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Port_Ptr = OnuMgmtTable[onuEntry].Filter_SIp_tcp;
	Ip_Port_cur = Ip_Port_Ptr;
	while( Ip_Port_Ptr != NULL )
		{
		if(( Ip_Port_Ptr->ipAddr == IpAddr ) && (Ip_Port_Ptr->port == tcp_port ))
			{
			if(  Ip_Port_cur ==  Ip_Port_Ptr )  /* 链表的第一个节点*/
				{ 
				OnuMgmtTable[onuEntry].Filter_SIp_tcp = (Ip_Port_Table_S *)Ip_Port_Ptr->Next_Ip_Port;
				}
			else {
				Ip_Port_cur->Next_Ip_Port = Ip_Port_Ptr->Next_Ip_Port;
				}
			VOS_Free((void *)Ip_Port_Ptr );
/*			ONU_MGMT_SEM_GIVE;*/
			return( ROK );
			}
		Ip_Port_cur = Ip_Port_Ptr;
		Ip_Port_Ptr = (Ip_Port_Table_S *)Ip_Port_Ptr->Next_Ip_Port;
		}
/*	ONU_MGMT_SEM_GIVE;*/

	return( V2R1_ONU_FILTER_IP_TCP_NOT_EXIST );
}


int  AddFilterDIpTcpToTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_Port )
{
	int onuEntry;
	Ip_Port_Table_S  *Ip_Port_Ptr, *Ip_Port_cur;

	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue =ThisIsValidOnu(PonPortIdx, OnuIdx);
	if(  retValue != ROK ) return( retValue );
	*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Port_Ptr = OnuMgmtTable[onuEntry].Filter_DIp_tcp;
	Ip_Port_cur = Ip_Port_Ptr;
	while( Ip_Port_Ptr != NULL )
		{
		if(( Ip_Port_Ptr->ipAddr == IpAddr ) &&(Ip_Port_Ptr->port ==  tcp_Port ))
		{
			return( V2R1_ONU_FILTER_IP_TCP_EXIST );
/*			ONU_MGMT_SEM_GIVE;*/
		}
		Ip_Port_cur = Ip_Port_Ptr;
		Ip_Port_Ptr = (Ip_Port_Table_S *)Ip_Port_Ptr->Next_Ip_Port;
		}
/*	ONU_MGMT_SEM_GIVE;*/
	Ip_Port_Ptr = ( Ip_Port_Table_S *) VOS_Malloc( sizeof(Ip_Port_Table_S ), MODULE_ONU );
	if( Ip_Port_Ptr == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}
	Ip_Port_Ptr ->ipAddr = IpAddr;
	Ip_Port_Ptr->port = tcp_Port;
	Ip_Port_Ptr->Next_Ip_Port = 0;
	/*sys_console_printf("\r\n Dip Ipaddr 0x%8x, tcp %d \r\n", IpAddr, tcp_Port );*/

/*	ONU_MGMT_SEM_TAKE;*/
	if( Ip_Port_cur == 0 ) OnuMgmtTable[onuEntry].Filter_DIp_tcp = Ip_Port_Ptr;
	else  Ip_Port_cur ->Next_Ip_Port = (unsigned int )Ip_Port_Ptr;
/*	ONU_MGMT_SEM_GIVE;*/

	return( ROK );	

}

int DelFilterDIpTcpFromTable( short int PonPortIdx, short int OnuIdx,  unsigned int IpAddr, unsigned short int tcp_port)
{
	int onuEntry;
	Ip_Port_Table_S  *Ip_Port_Ptr, *Ip_Port_cur;

	CHECK_ONU_RANGE
	/*
	int  retValue;
	retValue =ThisIsValidOnu(PonPortIdx, OnuIdx);
	if(  retValue != ROK ) return( retValue );
	*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Port_Ptr = OnuMgmtTable[onuEntry].Filter_DIp_tcp;
	Ip_Port_cur = Ip_Port_Ptr;
	while( Ip_Port_Ptr != NULL )
		{
		if(( Ip_Port_Ptr->ipAddr == IpAddr ) && (Ip_Port_Ptr->port == tcp_port ))
			{
			if(  Ip_Port_cur ==  Ip_Port_Ptr )  /* 链表的第一个节点*/
				{ 
				OnuMgmtTable[onuEntry].Filter_DIp_tcp = (Ip_Port_Table_S *)Ip_Port_Ptr->Next_Ip_Port;
				}
			else {
				Ip_Port_cur->Next_Ip_Port = Ip_Port_Ptr->Next_Ip_Port;
				}
			VOS_Free((void *)Ip_Port_Ptr );
/*			ONU_MGMT_SEM_GIVE;*/
			return( ROK );
			}
		Ip_Port_cur = Ip_Port_Ptr;
		Ip_Port_Ptr = (Ip_Port_Table_S *)Ip_Port_Ptr->Next_Ip_Port;
		}
/*	ONU_MGMT_SEM_GIVE;*/

	return( V2R1_ONU_FILTER_IP_TCP_NOT_EXIST );
}

int ShowOnuSIpFilterByVty( short int PonPortIdx, short int OnuIdx, struct  vty  *vty )
{
	int rc;
	IpTable_S  *Ip_Ptr;
	unsigned char IpAddrStr[16];
	
	CHECK_ONU_RANGE;

	vty_out( vty, " onu %d/%d/%d src-ip filter ", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1) );
/*	ONU_MGMT_SEM_TAKE;*/
	Ip_Ptr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].Filter_SIp;
	if( Ip_Ptr ) 
	{
		vty_out( vty, "list:\r\n" );
		while( Ip_Ptr )
		{
			VOS_MemSet( IpAddrStr, 0, sizeof( IpAddrStr ));
			inet_ntoa_v4( Ip_Ptr->ipAddr, IpAddrStr );		
			vty_out( vty, "    %s\r\n", IpAddrStr );
			Ip_Ptr = ( IpTable_S *) Ip_Ptr->NextIp;
		}
		rc = ROK;
		vty_out(vty, "\r\n");
	}
	else
	{
		vty_out( vty, " is NULL\r\n" );
		rc = RERROR;
	}
/*	ONU_MGMT_SEM_GIVE;*/
	return rc;
}

int ShowOnuDIpFilterByVty( short int PonPortIdx, short int OnuIdx, struct  vty  *vty  )
{
	int rc;
	IpTable_S  *Ip_Ptr;
	unsigned char IpAddrStr[16];
	
	CHECK_ONU_RANGE;

	vty_out( vty, " onu %d/%d/%d dst-ip filter ", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1) );
/*	ONU_MGMT_SEM_TAKE;*/

	Ip_Ptr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].Filter_DIp;
	if( Ip_Ptr )
	{
		vty_out( vty, "list:\r\n");
		while( Ip_Ptr )
		{
			VOS_MemSet( IpAddrStr, 0, sizeof( IpAddrStr ));
			inet_ntoa_v4( Ip_Ptr->ipAddr, IpAddrStr );		
			vty_out( vty, "    %s\r\n", IpAddrStr );
			Ip_Ptr = ( IpTable_S *) Ip_Ptr->NextIp;
		}
		rc = ROK;
	}
	else
	{
		vty_out( vty, "is NULL\r\n" );
		rc = RERROR;
	}

/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty, "\r\n");
	return rc;	
}

int ShowOnuUdpFilterByVty( short int PonPortIdx, short int OnuIdx , struct  vty  *vty )
{
	int rc;
	Ip_Port_Table_S  *Ip_Port_Ptr;
	
	CHECK_ONU_RANGE;

	vty_out( vty, " onu %d/%d/%d src-udp filter ", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1) );
/*	ONU_MGMT_SEM_TAKE;*/

	Ip_Port_Ptr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].Filter_SIp_udp;
	if( Ip_Port_Ptr )
	{
		vty_out( vty, "list:\r\n" );
		while( Ip_Port_Ptr )
		{
			vty_out( vty, "    %d\r\n", Ip_Port_Ptr->port );
			Ip_Port_Ptr = ( Ip_Port_Table_S *) Ip_Port_Ptr->Next_Ip_Port;
		}
		rc = ROK;
	}
	else
	{
		vty_out( vty, "is NULL\r\n" );
		rc = RERROR;
	}

/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty, "\r\n");
	return rc;

}

int ShowOnuTcpFilterByVty( short int PonPortIdx, short int OnuIdx , struct  vty  *vty )
{
	int rc;
	Ip_Port_Table_S  *Ip_Port_Ptr;
	
	CHECK_ONU_RANGE;

	vty_out( vty, " onu %d/%d/%d src-tcp filter ", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1) );
/*	ONU_MGMT_SEM_TAKE;*/
	
	Ip_Port_Ptr = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].Filter_SIp_tcp;
	if( Ip_Port_Ptr )
	{
		vty_out( vty, "list:\r\n" );
		while( Ip_Port_Ptr != NULL )
		{
			vty_out( vty, "    %d\r\n", Ip_Port_Ptr->port );
			Ip_Port_Ptr = ( Ip_Port_Table_S *) Ip_Port_Ptr->Next_Ip_Port;
		}
		rc = ROK;
	}
	else
	{
		vty_out( vty, "is NULL\r\n" );
		rc = RERROR;
	}
/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty, "\r\n");
	return rc;
}


#ifdef  ONU_VLAN_ID_FILTER
#endif
/** added by chenfj 2007-6-12 
       增加ONU 数据流vlan id 过滤*/

int AddOnuVlanIdFilter(short int PonPortIdx, short int OnuIdx, unsigned short int VlanId )
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		{
		ret = EnableOnuVlanIdFilter(PonPortIdx, OnuIdx, VlanId);
		if( ret == ROK )
			ret = AddFilterVlanIdToTable( PonPortIdx, OnuIdx, VlanId );
		}
	else {
		ret = AddFilterVlanIdToTable( PonPortIdx, OnuIdx, VlanId );
		}

	return( ret );
}

int  ClearOnuVlanIdFilter( short int PonPortIdx, short int OnuIdx, unsigned short int VlanId )
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		DisableOnuVlanIdFilter( PonPortIdx, OnuIdx, VlanId);

	ret = DelFilterVlanIdFromTable( PonPortIdx, OnuIdx, VlanId);
	return( ret );
}

int ClearOnuVlanIdFilterAll( short int PonPortIdx, short int OnuIdx )
{
	int ret ;
	VlanId_Table_S  *Filter_Vid, *Next_Filter_Vid;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

/*	ONU_MGMT_SEM_TAKE;*/

	Filter_Vid = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_Vid;
	while ( Filter_Vid != NULL ) 
		{
		Next_Filter_Vid = (VlanId_Table_S *)Filter_Vid->NextVid;
		ClearOnuVlanIdFilter( PonPortIdx, OnuIdx, Filter_Vid->Vid );
	
		Filter_Vid = Next_Filter_Vid;
		}
/*	ONU_MGMT_SEM_GIVE;*/
	
	return( ret );
}

int  EnableOnuVlanIdFilter( short int PonPortIdx, short int OnuIdx, unsigned short int VlanId )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_VLAN, VlanId, PON_DONT_PASS);
			ret = GW10G_REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_VLAN, VlanId, PON_DONT_PASS);
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_VLAN, VlanId, PON_DONT_PASS);
			ret = REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_VLAN, VlanId, PON_DONT_PASS);
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_VLAN, VlanId, PON_DONT_PASS);
			ret = TK_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_VLAN, VlanId, PON_DONT_PASS);
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( ( ret == PAS_EXIT_OK ) ||( ret == REMOTE_PASONU_ALREADY_EXISTS ))return( ROK );
		}
	
	else {	/* other pon chip type handler */

		}
	
	return( RERROR );

}

int  EnableOnuVlanIdFilterAll( short int PonPortIdx, short int OnuIdx)
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret = RERROR;

	VlanId_Table_S *Filter_Vid;
	
	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
		
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
/*		ONU_MGMT_SEM_TAKE;*/

		Filter_Vid = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_Vid;
		while( Filter_Vid != NULL )
			{
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_VLAN, Filter_Vid->Vid, PON_DONT_PASS);
				ret = GW10G_REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_VLAN, Filter_Vid->Vid, PON_DONT_PASS);
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_VLAN, Filter_Vid->Vid, PON_DONT_PASS);
				ret = REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_VLAN, Filter_Vid->Vid, PON_DONT_PASS);
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_VLAN, Filter_Vid->Vid, PON_DONT_PASS);
				ret = TK_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_VLAN, Filter_Vid->Vid, PON_DONT_PASS);
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			Filter_Vid = ( VlanId_Table_S *) Filter_Vid->NextVid;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if( ( ret == PAS_EXIT_OK ) ||( ret == REMOTE_PASONU_ALREADY_EXISTS ))return( ROK );
		}
	
	else {	/* other pon chip type handler */

		}
	
	return( RERROR );

}

int DisableOnuVlanIdFilter( short int PonPortIdx, short int OnuIdx, unsigned short int VlanId )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_VLAN, VlanId );
			ret = GW10G_REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_VLAN, VlanId );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_VLAN, VlanId );
			ret = REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_VLAN, VlanId );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_VLAN, VlanId );
			ret = TK_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_VLAN, VlanId );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK ) ||(ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK );
		}

	else{ /* other pon chip handler */

		}

	return( RERROR );
}

int DisableOnuVlanIdFilterAll( short int PonPortIdx, short int OnuIdx)
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret = RERROR;
	VlanId_Table_S  *Filter_Vid;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
/*		ONU_MGMT_SEM_TAKE;*/

		Filter_Vid = OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].Filter_Vid;
		while( Filter_Vid  != NULL )
			{
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_VLAN, Filter_Vid->Vid);
				ret = GW10G_REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_VLAN, Filter_Vid->Vid);
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_VLAN, Filter_Vid->Vid);
				ret = REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_VLAN, Filter_Vid->Vid);
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_VLAN, Filter_Vid->Vid);
				ret = TK_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_VLAN, Filter_Vid->Vid);
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			Filter_Vid = ( VlanId_Table_S *) Filter_Vid->NextVid;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if( (ret == PAS_EXIT_OK ) ||(ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK );
		}

	else{ /* other pon chip handler */

		}

	return( RERROR );
}
       
int  AddFilterVlanIdToTable( short int PonPortIdx, short int OnuIdx,  unsigned short int VlanId)
{
	int onuEntry;
	VlanId_Table_S  *Filter_Vid, *Filter_Vid_pre;
	CHECK_ONU_RANGE

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Filter_Vid = OnuMgmtTable[onuEntry].Filter_Vid ;
	Filter_Vid_pre= Filter_Vid;
	while( Filter_Vid != NULL )
	{
		if( Filter_Vid->Vid == VlanId ) 
		{
/*			ONU_MGMT_SEM_GIVE;*/
			return( V2R1_ONU_FILTER_VLAN_ID_EXIST );
		}
		Filter_Vid_pre = Filter_Vid;
		Filter_Vid = ( VlanId_Table_S  * )Filter_Vid->NextVid ;
	}
/*	ONU_MGMT_SEM_GIVE;*/

	Filter_Vid = ( VlanId_Table_S  * ) VOS_Malloc( sizeof( VlanId_Table_S ), MODULE_ONU );
	if( Filter_Vid == NULL )
	{
		ASSERT(0);
		return( RERROR );
	}
	Filter_Vid->Vid = VlanId;
	Filter_Vid->NextVid = 0; 
	
/*	ONU_MGMT_SEM_TAKE;*/
	if( Filter_Vid_pre == NULL )  OnuMgmtTable[onuEntry].Filter_Vid = Filter_Vid;
	else  Filter_Vid_pre ->NextVid = ( unsigned int )Filter_Vid;
/*	ONU_MGMT_SEM_GIVE;*/

	return( ROK );

}

int  DelFilterVlanIdFromTable( short int PonPortIdx, short int OnuIdx,  unsigned short int VlanId)
{
	int onuEntry;
	VlanId_Table_S  *Filter_Vid, *Filter_Vid_pre;
	CHECK_ONU_RANGE

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Filter_Vid = OnuMgmtTable[onuEntry].Filter_Vid ;
	Filter_Vid_pre= Filter_Vid;
	while( Filter_Vid  != NULL )
		{
		if( Filter_Vid->Vid == VlanId )
			{
			if( Filter_Vid == Filter_Vid_pre )  /* 链表的第一个节点*/
				OnuMgmtTable[onuEntry].Filter_Vid = ( VlanId_Table_S *)Filter_Vid->NextVid;
			else Filter_Vid_pre ->NextVid = Filter_Vid->NextVid;
			VOS_Free( (void *) Filter_Vid );
/*			ONU_MGMT_SEM_GIVE;*/
			return( ROK );
			}
		Filter_Vid_pre = Filter_Vid;
		Filter_Vid =( VlanId_Table_S *) Filter_Vid->NextVid;
		}
/*	ONU_MGMT_SEM_GIVE;*/

	return( V2R1_ONU_FILTER_VLAN_ID_NOT_EXIST );
	
}


int ShowOnuVlanIdFilterByVty( short int PonPortIdx, short int OnuIdx , struct  vty  *vty )
{
	int rc;
	VlanId_Table_S  *Filter_Vid;
	
	CHECK_ONU_RANGE;

	vty_out( vty, "  onu %d/%d/%d vlanId filter ", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1) );
/*	ONU_MGMT_SEM_TAKE;*/
	
	Filter_Vid = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].Filter_Vid;
	if( Filter_Vid )
	{
		vty_out( vty, "list:\r\n" );
		while( Filter_Vid )
		{
			vty_out( vty, "    %d\r\n", Filter_Vid->Vid );
			Filter_Vid = ( VlanId_Table_S *) Filter_Vid->NextVid ;
		}
		rc = ROK;
	}
	else
	{
		vty_out( vty, "is NULL\r\n" );
		rc = RERROR;
	}

/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty, "\r\n");
	return rc;
}

int  EnableOnuFilter(short int PonPortIdx, short int OnuIdx )
{
	EnableOnuSIpFilterAll( PonPortIdx, OnuIdx );
	EnableOnuDIpFilterAll( PonPortIdx, OnuIdx );
	EnableOnuSIpUdpFilterAll( PonPortIdx, OnuIdx );
	EnableOnuSIpTcpFilterAll( PonPortIdx, OnuIdx );
	EnableOnuVlanIdFilterAll( PonPortIdx, OnuIdx );
	EnableOnuEtherTypeFilterAll( PonPortIdx, OnuIdx );
	EnableOnuIpTypeFilterAll( PonPortIdx, OnuIdx );
	return( ROK );
}

#ifdef  ONU_ETHER_TYPE_and_IP_PROTOCOL_FILTER
#endif
/** added by chenfj 2007-6-14 
       增加ONU 数据流ETHER TYPE /IP PROTOCOL 过滤*/

int AddOnuEtherTypeFilter(short int PonPortIdx, short int OnuIdx, unsigned short int  EtherType)
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		{
		ret = EnableOnuEtherTypeFilter(PonPortIdx, OnuIdx, EtherType);
		if( ret == ROK )
			ret = AddFilterEtherTypeToTable( PonPortIdx, OnuIdx, EtherType );
		}
	else {
		ret = AddFilterEtherTypeToTable( PonPortIdx, OnuIdx, EtherType );
		}

	return( ret );
}

int  ClearOnuEtherTypeFilter( short int PonPortIdx, short int OnuIdx, unsigned short int  EtherType )
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		DisableOnuEtherTypeFilter( PonPortIdx, OnuIdx, EtherType);

	ret = DelFilterEtherTypeFromTable( PonPortIdx, OnuIdx, EtherType);
	return( ret );
}

int ClearOnuEtherTypeFilterAll( short int PonPortIdx, short int OnuIdx )
{
	int ret ;
	Ether_Type_Table_S  *Filter_EtherType, *Next_Filter_EtherType;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

/*	ONU_MGMT_SEM_TAKE;*/

	Filter_EtherType = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_Ether_Type;
	while ( Filter_EtherType != NULL ) 
		{
		Next_Filter_EtherType = (Ether_Type_Table_S *)Filter_EtherType->Next_EthType;
		ClearOnuEtherTypeFilter( PonPortIdx, OnuIdx, Filter_EtherType->EtherType);
	
		Filter_EtherType = Next_Filter_EtherType ;
		}
/*	ONU_MGMT_SEM_GIVE;*/
	
	return( ret );
}

       
int  EnableOnuEtherTypeFilter( short int PonPortIdx, short int OnuIdx, unsigned short int EtherType )
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_ETHERTYPE, EtherType, PON_DONT_PASS);
			ret = GW10G_REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_ETHERTYPE, EtherType, PON_DONT_PASS);
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_ETHERTYPE, EtherType, PON_DONT_PASS);
			ret = REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_ETHERTYPE, EtherType, PON_DONT_PASS);
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_ETHERTYPE, EtherType, PON_DONT_PASS);
			ret = TK_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_ETHERTYPE, EtherType, PON_DONT_PASS);
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( ( ret == PAS_EXIT_OK ) ||( ret == REMOTE_PASONU_ALREADY_EXISTS ))return( ROK );
		}
	
	else {	/* other pon chip type handler */

		}
	
	return( RERROR );

}

int  EnableOnuEtherTypeFilterAll( short int PonPortIdx, short int OnuIdx)
{
	short int PonChipType;
	
	short int Onu_id;
	short int ret = RERROR;

	Ether_Type_Table_S *Filter_EtherType;
	
	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
		
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
/*		ONU_MGMT_SEM_TAKE;*/

		Filter_EtherType = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_Ether_Type;
		while( Filter_EtherType != NULL )
			{
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_ETHERTYPE, Filter_EtherType->EtherType, PON_DONT_PASS);
				ret = GW10G_REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_ETHERTYPE, Filter_EtherType->EtherType, PON_DONT_PASS);
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_ETHERTYPE, Filter_EtherType->EtherType, PON_DONT_PASS);
				ret = REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_ETHERTYPE, Filter_EtherType->EtherType, PON_DONT_PASS);
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_ETHERTYPE, Filter_EtherType->EtherType, PON_DONT_PASS);
				ret = TK_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_ETHERTYPE, Filter_EtherType->EtherType, PON_DONT_PASS);
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			Filter_EtherType = ( Ether_Type_Table_S *) Filter_EtherType->Next_EthType;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if( ( ret == PAS_EXIT_OK ) ||( ret == REMOTE_PASONU_ALREADY_EXISTS ))return( ROK );
		}
	
	else {	/* other pon chip type handler */

		}
	
	return( RERROR );

}

int DisableOnuEtherTypeFilter( short int PonPortIdx, short int OnuIdx, unsigned short int EtherType )
{
	short int PonChipType;
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_ETHERTYPE, EtherType );
			ret = GW10G_REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_ETHERTYPE, EtherType );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_ETHERTYPE, EtherType );
			ret = REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_ETHERTYPE, EtherType );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_ETHERTYPE, EtherType );
			ret = TK_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_ETHERTYPE, EtherType );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK ) ||(ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK );
		}

	else{ /* other pon chip handler */

		}

	return( RERROR );
}

int DisableOnuEtherTypeFilterAll( short int PonPortIdx, short int OnuIdx)
{
	short int PonChipType;
	short int Onu_id;
	short int ret = RERROR;
	Ether_Type_Table_S  *Filter_EtherType;
	
	CHECK_ONU_RANGE;
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
/*		ONU_MGMT_SEM_TAKE;*/

		Filter_EtherType = OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].Filter_Ether_Type;
		while( Filter_EtherType  != NULL )
			{
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_ETHERTYPE, Filter_EtherType->EtherType );
				ret = GW10G_REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_ETHERTYPE, Filter_EtherType->EtherType );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_ETHERTYPE, Filter_EtherType->EtherType );
				ret = REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_ETHERTYPE, Filter_EtherType->EtherType );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_ETHERTYPE, Filter_EtherType->EtherType );
				ret = TK_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_ETHERTYPE, Filter_EtherType->EtherType );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			Filter_EtherType = ( Ether_Type_Table_S *) Filter_EtherType->Next_EthType;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if( (ret == PAS_EXIT_OK ) ||(ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK );
		}

	else{ /* other pon chip handler */

		}

	return( RERROR );
}

int  AddFilterEtherTypeToTable( short int PonPortIdx, short int OnuIdx,  unsigned short int  EthType)
{
	Ether_Type_Table_S  *Filter_EthType, *Filter_EthType_pre;
	CHECK_ONU_RANGE;

/*	ONU_MGMT_SEM_TAKE;*/
	Filter_EthType = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].Filter_Ether_Type;
	Filter_EthType_pre= Filter_EthType;
	while( Filter_EthType != NULL )
		{
		if( Filter_EthType->EtherType ==  EthType )
		{
/*			ONU_MGMT_SEM_GIVE;*/
			return( V2R1_ONU_FILTER_ETHER_TYPE_EXIST );
		}
		Filter_EthType_pre = Filter_EthType;
		Filter_EthType = ( Ether_Type_Table_S  * )Filter_EthType->Next_EthType;
		}
/*	ONU_MGMT_SEM_GIVE;*/

	Filter_EthType = ( Ether_Type_Table_S  * ) VOS_Malloc( sizeof( Ether_Type_Table_S ), MODULE_ONU );
	if( Filter_EthType == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}
	Filter_EthType->EtherType = EthType;
	Filter_EthType->Next_EthType = 0; 
	
/*	ONU_MGMT_SEM_TAKE;*/
	if( Filter_EthType_pre == NULL )  OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].Filter_Ether_Type = Filter_EthType;
	else  Filter_EthType_pre ->Next_EthType= ( unsigned int )Filter_EthType;
/*	ONU_MGMT_SEM_GIVE;*/

	return( ROK );

}

int  DelFilterEtherTypeFromTable( short int PonPortIdx, short int OnuIdx,  unsigned short int  EthType )
{
	int onuEntry;
	Ether_Type_Table_S  *Filter_EthType, *Filter_EthType_pre;
	CHECK_ONU_RANGE

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/
	Filter_EthType = OnuMgmtTable[onuEntry].Filter_Ether_Type;
	Filter_EthType_pre = Filter_EthType;

	while( Filter_EthType  != NULL )
		{
		if( Filter_EthType->EtherType ==  EthType  )
			{
			if( Filter_EthType == Filter_EthType_pre )  /* 链表的第一个节点*/
				OnuMgmtTable[onuEntry].Filter_Ether_Type = ( Ether_Type_Table_S *)Filter_EthType->Next_EthType;
			else Filter_EthType_pre ->Next_EthType= Filter_EthType->Next_EthType;
			VOS_Free( (void *) Filter_EthType );
/*			ONU_MGMT_SEM_GIVE;*/
			return( ROK );
			}
		Filter_EthType_pre = Filter_EthType;
		Filter_EthType =( Ether_Type_Table_S *) Filter_EthType->Next_EthType;
		}
/*	ONU_MGMT_SEM_GIVE;*/

	return( V2R1_ONU_FILTER_ETHER_TYPE_NOT_EXIST );
	
}

int AddOnuIpTypeFilter(short int PonPortIdx, short int OnuIdx, unsigned short int  IpType)
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		{
		ret = EnableOnuIpTypeFilter(PonPortIdx, OnuIdx, IpType);
		if( ret == ROK )
			ret = AddFilterIpTypeToTable( PonPortIdx, OnuIdx, IpType );
		}
	else {
		ret = AddFilterIpTypeToTable( PonPortIdx, OnuIdx, IpType );
		}

	return( ret );
}

int  ClearOnuIpTypeFilter( short int PonPortIdx, short int OnuIdx, unsigned short int  IpType )
{
	int ret ;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP ) 
		DisableOnuIpTypeFilter( PonPortIdx, OnuIdx, IpType);

	ret = DelFilterIpTypeFromTable( PonPortIdx, OnuIdx, IpType);
	return( ret );
}

int ClearOnuIpTypeFilterAll( short int PonPortIdx, short int OnuIdx )
{
	int ret ;
	Ip_Type_Table_S  *Filter_IpType, *Next_Filter_IpType;

	CHECK_ONU_RANGE

	ret = ThisIsValidOnu( PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

/*	ONU_MGMT_SEM_TAKE;*/

	Filter_IpType = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_Ip_Type;
	while ( Filter_IpType != NULL ) 
		{
		Next_Filter_IpType = (Ip_Type_Table_S *)Filter_IpType->Next_IpType;
		ClearOnuIpTypeFilter( PonPortIdx, OnuIdx, Filter_IpType->IpType);
	
		Filter_IpType = Next_Filter_IpType ;
		}
/*	ONU_MGMT_SEM_GIVE;*/
	
	return( ret );
}

int  EnableOnuIpTypeFilter( short int PonPortIdx, short int OnuIdx, unsigned short int IpType )
{
	short int PonChipType;
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_IPV4, IpType, PON_DONT_PASS);
			ret = GW10G_REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_IPV4, IpType, PON_DONT_PASS);
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_IPV4, IpType, PON_DONT_PASS);
			ret = REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_IPV4, IpType, PON_DONT_PASS);
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_IPV4, IpType, PON_DONT_PASS);
			ret = TK_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_IPV4, IpType, PON_DONT_PASS);
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( ( ret == PAS_EXIT_OK ) ||( ret == REMOTE_PASONU_ALREADY_EXISTS ))return( ROK );
		}
	
	else {	/* other pon chip type handler */

		}
	
	return( RERROR );

}

int  EnableOnuIpTypeFilterAll( short int PonPortIdx, short int OnuIdx)
{
	short int PonChipType;
	short int Onu_id;
	short int ret = RERROR;

	Ip_Type_Table_S *Filter_IpType;
	
	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
		
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
/*		ONU_MGMT_SEM_TAKE;*/

		Filter_IpType = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].Filter_Ip_Type;
		while( Filter_IpType != NULL )
			{
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_IPV4, Filter_IpType->IpType, PON_DONT_PASS);
				ret = GW10G_REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_IPV4, Filter_IpType->IpType, PON_DONT_PASS);
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_IPV4, Filter_IpType->IpType, PON_DONT_PASS);
				ret = REMOTE_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_IPV4, Filter_IpType->IpType, PON_DONT_PASS);
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_IPV4, Filter_IpType->IpType, PON_DONT_PASS);
				ret = TK_PASONU_classifier_add_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_IPV4, Filter_IpType->IpType, PON_DONT_PASS);
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			Filter_IpType = ( Ip_Type_Table_S *) Filter_IpType->Next_IpType;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if( ( ret == PAS_EXIT_OK ) ||( ret == REMOTE_PASONU_ALREADY_EXISTS ))return( ROK );
		}
	
	else {	/* other pon chip type handler */

		}
	
	return( RERROR );

}

int DisableOnuIpTypeFilter( short int PonPortIdx, short int OnuIdx, unsigned short int  IpType )
{
	short int PonChipType;
	short int Onu_id;
	short int ret;
	
	CHECK_ONU_RANGE
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_IPV4, IpType );
			ret = GW10G_REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_IPV4,  IpType );
		}
		else
#endif
        if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
			ret = REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_IPV4, IpType );
			ret = REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_IPV4,  IpType );
		}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( OLT_PONCHIP_ISTK(PonChipType) )
        {
			ret = TK_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_IPV4, IpType );
			ret = TK_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_IPV4,  IpType );
        }
        else
        {
            ret = PAS_PARAMETER_ERROR;
        }
		if( (ret == PAS_EXIT_OK ) ||(ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK );
		}

	else{ /* other pon chip handler */

		}

	return( RERROR );
}

int DisableOnuIpTypeFilterAll( short int PonPortIdx, short int OnuIdx)
{
	short int PonChipType;
	short int Onu_id;
	short int ret = RERROR;
	Ip_Type_Table_S  *Filter_IpType;
	
	CHECK_ONU_RANGE;
	/*
	int retValue;
	retValue = ThisIsValidOnu( PonPortIdx, OnuIdx); 
	if(  retValue != ROK ) return( retValue );
	*/
	
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) !=  ONU_OPER_STATUS_UP ) return( RERROR );
	Onu_id = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_id == INVALID_LLID  ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISVALID(PonChipType) )
		{
/*		ONU_MGMT_SEM_TAKE;*/

		Filter_IpType = OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].Filter_Ip_Type;
		while( Filter_IpType  != NULL )
			{
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
			if(OLT_PONCHIP_ISPAS10G(PonChipType))
			{
				ret = GW10G_REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_IPV4, Filter_IpType->IpType );
				ret = GW10G_REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_IPV4, Filter_IpType->IpType );
			}
			else
#endif
            if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				ret = REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_IPV4, Filter_IpType->IpType );
				ret = REMOTE_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_IPV4, Filter_IpType->IpType );
			}
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            else if ( OLT_PONCHIP_ISTK(PonChipType) )
            {
				ret = TK_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_DOWNLINK, PON_FRAME_IPV4, Filter_IpType->IpType );
				ret = TK_PASONU_classifier_remove_filter( PonPortIdx, Onu_id, PON_DIRECTION_UPLINK, PON_FRAME_IPV4, Filter_IpType->IpType );
            }
            else
            {
                ret = PAS_PARAMETER_ERROR;
                break;
            }
			Filter_IpType = ( Ip_Type_Table_S  *) Filter_IpType->Next_IpType;
			}
/*		ONU_MGMT_SEM_GIVE;*/
		if( (ret == PAS_EXIT_OK ) ||(ret == REMOTE_PASONU_QUERY_FAILED )) return( ROK );
		}

	else{ /* other pon chip handler */

		}

	return( RERROR );
}

int  AddFilterIpTypeToTable( short int PonPortIdx, short int OnuIdx,  unsigned short int  IpType)
{
	Ip_Type_Table_S  *Filter_IpType, *Filter_IpType_pre;
	CHECK_ONU_RANGE

/*	ONU_MGMT_SEM_TAKE;*/
	Filter_IpType = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].Filter_Ip_Type;
	Filter_IpType_pre= Filter_IpType;
	while( Filter_IpType != NULL )
		{
		if( Filter_IpType->IpType ==  IpType )
		{
/*			ONU_MGMT_SEM_GIVE;*/
			return( V2R1_ONU_FILTER_IP_PROT_EXIST );
		}
		Filter_IpType_pre = Filter_IpType;
		Filter_IpType = ( Ip_Type_Table_S  * )Filter_IpType->Next_IpType;
		}
/*	ONU_MGMT_SEM_GIVE;*/

	Filter_IpType = ( Ip_Type_Table_S  * ) VOS_Malloc( sizeof( Ip_Type_Table_S ), MODULE_ONU );
	if( Filter_IpType == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}
	Filter_IpType->IpType = IpType;
	Filter_IpType->Next_IpType = 0; 
	
/*	ONU_MGMT_SEM_TAKE;*/
	if( Filter_IpType_pre == NULL )  OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].Filter_Ip_Type = Filter_IpType;
	else  Filter_IpType_pre ->Next_IpType= ( unsigned int )Filter_IpType;
/*	ONU_MGMT_SEM_GIVE;*/

	return( ROK );

}

int  DelFilterIpTypeFromTable( short int PonPortIdx, short int OnuIdx,  unsigned short int  IpType )
{
	int onuEntry;
	Ip_Type_Table_S  *Filter_IpType, *Filter_IpType_pre;
	CHECK_ONU_RANGE

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
/*	ONU_MGMT_SEM_TAKE;*/

	Filter_IpType = OnuMgmtTable[onuEntry].Filter_Ip_Type;
	Filter_IpType_pre = Filter_IpType;
	while( Filter_IpType  != NULL )
		{
		if( Filter_IpType->IpType ==  IpType  )
			{
			if( Filter_IpType == Filter_IpType_pre )  /* 链表的第一个节点*/
				OnuMgmtTable[onuEntry].Filter_Ip_Type = ( Ip_Type_Table_S *)Filter_IpType->Next_IpType;
			else Filter_IpType_pre ->Next_IpType= Filter_IpType->Next_IpType;
			VOS_Free( (void *) Filter_IpType );
/*			ONU_MGMT_SEM_GIVE;*/
			return( ROK );
			}
		Filter_IpType_pre = Filter_IpType;
		Filter_IpType =( Ip_Type_Table_S *) Filter_IpType->Next_IpType;
		}
/*	ONU_MGMT_SEM_GIVE;*/

	return( V2R1_ONU_FILTER_ETHER_TYPE_NOT_EXIST );
	
}

int ShowOnuEtherTypeFilterByVty( short int PonPortIdx, short int OnuIdx , struct  vty  *vty )
{
	int rc;
	Ether_Type_Table_S  *Filter_Ethertype;
	
	CHECK_ONU_RANGE;

	vty_out( vty, "  onu %d/%d/%d ether type filter ", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1) );
/*	ONU_MGMT_SEM_TAKE;*/
	
	Filter_Ethertype = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].Filter_Ether_Type;
	if( Filter_Ethertype )
	{
		vty_out( vty, "list:\r\n" );
		while( Filter_Ethertype )
		{
			vty_out( vty, "    %d\r\n", Filter_Ethertype->EtherType);
			Filter_Ethertype = ( Ether_Type_Table_S *) Filter_Ethertype->Next_EthType;
		}
		rc = ROK;
	}
	else
	{
		vty_out( vty, "is NULL\r\n" );
		rc = RERROR;
	}
/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty, "\r\n");
	return rc;
}

int ShowOnuIpTypeFilterByVty( short int PonPortIdx, short int OnuIdx , struct  vty  *vty )
{
	int rc;
	Ip_Type_Table_S  *Filter_Iptype;
	
	CHECK_ONU_RANGE;

	vty_out( vty, "  onu %d/%d/%d ip protocol type filter ", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1) );
/*	ONU_MGMT_SEM_TAKE;*/
	
	Filter_Iptype = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].Filter_Ip_Type;
	if( Filter_Iptype )
	{
		vty_out( vty, "list:\r\n" );
		while( Filter_Iptype )
		{
			vty_out( vty, "    %d\r\n", Filter_Iptype->IpType);
			Filter_Iptype = ( Ip_Type_Table_S *) Filter_Iptype->Next_IpType;
		}
		rc = ROK;
	}
	else
	{
		vty_out( vty, "is NULL\r\n" );
		rc = RERROR;
	}
/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty, "\r\n");
	return rc;
}

#endif
#ifdef CTC_STACK_ONU_
#endif

int GetOnuVendorType( short int PonPortIdx, short int OnuIdx )
{
	int type = ONU_VENDOR_GW;
	/*
	unsigned char  MacAddr[BYTES_IN_MAC_ADDRESS];
	int  len;
	int ret;
	CHECK_ONU_RANGE;

	if( ThisIsValidOnu( PonPortIdx, OnuIdx) != ROK ) return( RERROR );
	ret = GetOnuMacAddr( PonPortIdx, OnuIdx, MacAddr, &len );

	if( ret == ROK )
		{
		if( ( MacAddr[0] == GW_private_MAC[0]) && ( MacAddr[1] == GW_private_MAC[1] ) && ( MacAddr[2] == GW_private_MAC[2] ) &&( ( MacAddr[3] != GW_private_MAC[3] ) && ( MacAddr[3] != 0xf0 ) ))
			return(  ONU_VENDOR_GW );
		else return( ONU_VENDOR_CT );
		}
	*/
	/* 
	modified by chenfj 2007/4/28
	讨论决定:在PAS5201下，可以连CTC_ONU 和私有ONU 。但两种ONU 不可以混接；就是在某一时刻，要么接CT ONU ，要么接私有ONU
	在I2C中增加信息，这个信息用于标识PON 端口下当前连接的ONU 类型；
	于是通过这个信息，就可以知道这个PON 口下的ONU 类型了
	*/
	/*if( ThisIsValidOnu( PonPortIdx, OnuIdx) != ROK ) return( RERROR );*/

	/*if( IsSupportCTCOnu( PonPortIdx ) == FALSE )  return( ONU_VENDOR_GW );*/

	/* added  by chenfj 2007-8-1
	    增加GT813/GT816 作为CTC ONU注册，但仍作为私有ONU来 管理*/
	
	if( IsSupportCTCOnu( PonPortIdx ) == TRUE )
	{		
		if( GetOnuExtOAMStatus( PonPortIdx, OnuIdx ) != V2R1_ENABLE )	
	  		type = ONU_VENDOR_CT;
		else 
		{
			/* modified by xieshl 20070930 */
			int suffix = PonPortIdx * MAXONUPERPON + OnuIdx;
			/*
			if( (OnuMgmtTable[suffix].onu_model ==  CTC_GT813_MODEL) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT816_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT810_MODEL ) ||
				
				(OnuMgmtTable[suffix].onu_model ==  CTC_GT811_MODEL) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT812_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT831_A_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT831_A_CATV_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT861_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT812B_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT815_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT866_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT831B_MODEL ) ||

				( OnuMgmtTable[suffix].onu_model ==  CTC_GT811B_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT851_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT813B_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT862_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT863_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT892_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT835_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT831_B_CATV_MODEL ) ||
				( OnuMgmtTable[suffix].onu_model ==  CTC_GT815_B_MODEL ) 
				)*/
			ONU_MGMT_SEM_TAKE;
			if(CompareCtcRegisterId(PonPortIdx,OnuIdx,OnuMgmtTable[suffix].onu_model) == RERROR)
			 	type = ONU_VENDOR_CT;
			ONU_MGMT_SEM_GIVE;
		}
	}
	return type;
}

int GetOnuProtectType( short int PonPortIdx, short int OnuIdx )
{
    int iPT;

    CHECK_ONU_RANGE

	ONU_MGMT_SEM_TAKE;
	iPT = OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].ProtectType;
	ONU_MGMT_SEM_GIVE;

    return iPT;
}

int CTC_SetOnuMulticastSwitchProtocol(short int PonPortIdx, short int OnuIdx )
{
    
	short int PonChipType;
	short int onu_id;
	short int ret;
	ULONG mc_switch;
    int cur_onustatus = 0;
	CHECK_ONU_RANGE;
   
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	cur_onustatus = GetOnuOperStatus_1(PonPortIdx, OnuIdx );
	if(cur_onustatus != ONU_OPER_STATUS_UP && cur_onustatus!= ONU_OPER_STATUS_DORMANT) return( RERROR );
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );
	
	
   if(OLT_PONCHIP_ISPAS(PonChipType) && !OLT_PONCHIP_ISPAS5001(PonChipType))
   	{
			ONU_MGMT_SEM_TAKE;
			mc_switch = OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].multicastSwitch;
			ONU_MGMT_SEM_GIVE;
			if( mc_switch ==  1 )
				#if 1
				ret = OnuMgt_SetMulticastSwitch(PonPortIdx, OnuIdx, CTC_STACK_PROTOCOL_IGMP_SNOOPING );
				#else
				ret = CTC_STACK_set_multicast_switch(PonPortIdx, onu_id, CTC_STACK_PROTOCOL_IGMP_SNOOPING );
				#endif
			else if( mc_switch ==  2)
				#if 1
				ret = OnuMgt_SetMulticastSwitch(PonPortIdx, OnuIdx, CTC_STACK_PROTOCOL_CTC);
				#else
				ret = CTC_STACK_set_multicast_switch(PonPortIdx, onu_id, CTC_STACK_PROTOCOL_CTC);
				#endif

			if( ret == PAS_EXIT_OK ) return( ROK );
				else return ( RERROR );
		}
	   /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	else{ /* other pon chip handler */

		}

	return( RERROR );
}

int CTC_GetOnuMulticastSwitchProtocol(short int PonPortIdx, short int OnuIdx , int *MulticastProtocol )
{
    
	short int PonChipType;
	short int onu_id;
	short int ret = PAS_EXIT_ERROR;

	CHECK_ONU_RANGE

	if( MulticastProtocol == NULL ) return( RERROR );
	
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP ) return( RERROR );
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );
	
   if(OLT_PONCHIP_ISPAS(PonChipType)&& !OLT_PONCHIP_ISPAS5001(PonChipType))
   	{
		if( V2R1_CTC_STACK )
			#if 1
			ret = OnuMgt_GetMulticastSwitch(PonPortIdx, OnuIdx, (CTC_STACK_multicast_protocol_t *)MulticastProtocol );
			#else
			ret = CTC_STACK_get_multicast_switch(PonPortIdx, onu_id, (CTC_STACK_multicast_protocol_t *)MulticastProtocol );
			#endif
		if( ret == PAS_EXIT_OK ) return( ROK );
		else return ( RERROR );
		}
	   /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	
	else{ /* other pon chip handler */

	}

	return( RERROR );
}


#if 0
int CTC_GetOnuSN( short int PonPortIdx, short int OnuIdx )
{
	short int PonChipType;
	short int PonChipVer = RERROR;
	short int Onu_id;
	CTC_STACK_onu_serial_number_t  onu_serial_number;
	bool  Init;
	
	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx )  != ONU_OPER_STATUS_UP )
		{
		sys_console_printf("%s/port%d onu%d off-line\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
		return( RERROR );
		}

	Onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( Onu_id  == INVALID_LLID ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if( OLT_PONCHIP_ISPAS(PonChipType) ) 
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
	}

	if( PonChipType == PONCHIP_PAS )
		{
		if(( PonChipVer != PONCHIP_PAS5001 ) && ( V2R1_CTC_STACK ))
			{
			/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
			if( OLT_CTCSTACKIsInit( &Init) !=  CTC_STACK_EXIT_OK ) 
				{
				sys_console_printf("CTC STACK status Err \r\n");
				return( RERROR );
				}
			if( Init != TRUE )
				{
				sys_console_printf("CTC STACK not Initialize\r\n");
				return( RERROR );
				}
			#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
			if( PAS_EXIT_OK != OnuMgt_GetSerialNumber( PonPortIdx, OnuIdx, &onu_serial_number))
			#else
			if( PAS_EXIT_OK != CTC_STACK_get_onu_serial_number( PonPortIdx, Onu_id, &onu_serial_number))
			#endif
				{
				sys_console_printf("Get %s/port%d onu%d CTC_EXT OAM SN Err\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
				return( RERROR );
				}
			else{
				sys_console_printf("%s/port%d onu%d CTC_EXT OAM SN \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
				sys_console_printf("vendor id %c%c%c%c\r\n",onu_serial_number.vendor_id[0],onu_serial_number.vendor_id[1],onu_serial_number.vendor_id[2],onu_serial_number.vendor_id[3]);
				sys_console_printf("onu_model 0x%x\r\n", onu_serial_number.onu_model);
				sys_console_printf("onu_id:%02x-%02x-%02x-%02x-%02x-%02x\r\n", onu_serial_number.onu_id[0], onu_serial_number.onu_id[1], onu_serial_number.onu_id[2], onu_serial_number.onu_id[3], onu_serial_number.onu_id[4], onu_serial_number.onu_id[5]);
				sys_console_printf("hardware ver:%c%c%c%c%c%c%c%c\r\n", onu_serial_number.hardware_version[0], onu_serial_number.hardware_version[1], onu_serial_number.hardware_version[2], onu_serial_number.hardware_version[3], onu_serial_number.hardware_version[4], onu_serial_number.hardware_version[5], onu_serial_number.hardware_version[6], onu_serial_number.hardware_version[7]);
				sys_console_printf("software ver: %c%c%c%c%c%c%c%c", onu_serial_number.software_version[0], onu_serial_number.software_version[1], onu_serial_number.software_version[2], onu_serial_number.software_version[3], onu_serial_number.software_version[4], onu_serial_number.software_version[5], onu_serial_number.software_version[6], onu_serial_number.software_version[7]);
				sys_console_printf("%c%c%c%c%c%c%c%c\r\n", onu_serial_number.software_version[8], onu_serial_number.software_version[9], onu_serial_number.software_version[10], onu_serial_number.software_version[11], onu_serial_number.software_version[12], onu_serial_number.software_version[13], onu_serial_number.software_version[14], onu_serial_number.software_version[15]);
				return( ROK );
				}
			}
		}

	else{

		}
	
	return( RERROR );
}
#endif


int  CTC_StartLlidEncrypt( short int PonPortIdx, short int OnuIdx )
{
    
	short int PonChipType;
	short int onu_id;
	short int ret = PAS_EXIT_ERROR;

	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP ) return( RERROR );
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );
	
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
		
	if(OLT_PONCHIP_ISPAS(PonChipType))
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) ) return (RERROR );

		if( V2R1_CTC_STACK )
		{
		    #if 1
			ret = OnuMgt_StartEncrypt( PonPortIdx, OnuIdx );
			#else
			ret = CTC_STACK_start_encryption( PonPortIdx, onu_id );
			#endif
		}
        /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
		if( ret == PAS_EXIT_OK ) 
			{
			/*
			OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].CTC_EncryptCtrl = V2R1_CTC_ENCRYPT_START;
			OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].CTC_EncryptStatus = V2R1_CTC_ENCRYPT_ENCRYPTING;
			*/
			return( ROK );
			}
		else  return( RERROR );			
		}

	else { /* other pon chip handler */

		}

	return( RERROR );
}

int  CTC_StopLlidEncrypt( short int PonPortIdx, short int OnuIdx )
{
   
	short int PonChipType;
	short int onu_id;
	short int ret = PAS_EXIT_ERROR;

	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP ) return( RERROR );
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );

	
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
		
	if(OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) ) return (RERROR );

		if( V2R1_CTC_STACK )
		{
		    #if 1
			ret = OnuMgt_StopEncrypt( PonPortIdx, OnuIdx );
			#else
			ret = CTC_STACK_stop_encryption( PonPortIdx, onu_id );
			#endif
		}
        /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
		if( ret == PAS_EXIT_OK ) 
			{
			/*
			OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].CTC_EncryptCtrl = V2R1_CTC_ENCRYPT_STOP;
			OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].CTC_EncryptStatus = V2R1_CTC_ENCRYPT_NOOP;
			*/
			return( ROK );
			}
		else  return( RERROR );			
		}

	else { /* other pon chip handler */

		}

	return( RERROR );
}


int  CTC_GetLlidEncryptStatus( short int PonPortIdx, short int OnuIdx )
{
	int status;
	CHECK_ONU_RANGE;
	ONU_MGMT_SEM_TAKE;
	status = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].CTC_EncryptStatus;
	ONU_MGMT_SEM_GIVE;
	return status;
}

int CTC_SetLlidFecMode( short int PonPortIdx, short int OnuIdx, int  mode )
{
	short int PonChipType;
	short int onu_id;
	short int ret;

	CHECK_ONU_RANGE

	if(( mode != STD_FEC_MODE_ENABLED) && ( mode != STD_FEC_MODE_DISABLED))
	 	return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
		
	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType)) return( RERROR );
		
		/*OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].FEC_Mode = mode;*/
		return( V2R1_ONU_OFF_LINE );
		}
	
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );
	
	if(OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( !OLT_PONCHIP_ISPAS5001(PonChipType) )
			{	
			/*int  Fec_Ability ;*/
			if( IsSupportCTCOnu( PonPortIdx ) != TRUE ) return( RERROR );
			
			ret = OnuMgt_SetOnuFecMode(PonPortIdx, OnuIdx, mode);

			if( ret == PAS_EXIT_OK ) 
			{
				ONU_MGMT_SEM_TAKE;
				OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].FEC_Mode = mode;
				ONU_MGMT_SEM_GIVE;
				return( ROK );
			}
			else  return( RERROR );			
			}
		}

	else { /* other pon chip handler */

		}

	return( RERROR );
}

int CTC_GetLlidFecMode( short int PonPortIdx, short int OnuIdx, int  *mode )
{
	short int PonChipType;
	short int onu_id;
	/*short int ret;*/
	int retval;

	CHECK_ONU_RANGE

	if( mode == NULL )
	 	return( RERROR );

	retval =  ThisIsValidOnu( PonPortIdx, OnuIdx);
	if ( retval != ROK ) return( retval );
	
	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP ) return( V2R1_ONU_OFF_LINE );
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
		
	if(OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( !OLT_PONCHIP_ISPAS5001(PonChipType) )
			{			
			/*ret = CTC_STACK_get_fec_mode( PonPortIdx, onu_id, (CTC_STACK_standard_FEC_mode_t *)mode );
			if( ret == PAS_EXIT_OK ) */
				{
				ONU_MGMT_SEM_TAKE;
				*mode = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].FEC_Mode;
				ONU_MGMT_SEM_GIVE;
				return( ROK );
				}
			/*else  return( RERROR );		*/
			}
		}

	else { /* other pon chip handler */

		}

	return( RERROR );
}

/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
int CTC_GetLlidFecAbility( short int PonPortIdx, short int OnuIdx, int  *Fec_Ability )
{
    
	short int PonChipType;
	short int onu_id;
	short int ret;

	CHECK_ONU_RANGE

	if( Fec_Ability == NULL ) return( RERROR );
	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP ) return( RERROR );
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );

	*Fec_Ability = STD_FEC_ABILITY_UNKNOWN;
	
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
		
	if(OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( !OLT_PONCHIP_ISPAS5001(PonChipType) )
			{	
			
			#if 1
			ret = OnuMgt_GetFecAbility( PonPortIdx, OnuIdx, (CTC_STACK_standard_FEC_ability_t *)Fec_Ability );
			#else
			ret = CTC_STACK_get_fec_ability( PonPortIdx, onu_id, (CTC_STACK_standard_FEC_ability_t *)Fec_Ability );
			#endif
					

			if( ret == PAS_EXIT_OK ) 
				{
				ONU_MGMT_SEM_TAKE;
				OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].FEC_ability = *Fec_Ability;
				ONU_MGMT_SEM_GIVE;
				return( ROK );
				}
			else  return( RERROR );			
			}
		}

	else { /* other pon chip handler */

		}

	return( RERROR );
}

int CTC_UpOnuOneEthPort(short int PonPortIdx, short int OnuIdx, unsigned char EthPort)
{
	short int llid;
	short int ret;
	
	CHECK_ONU_RANGE

	/*onuEthPort[PonPortIdx][OnuIdx][EthPort-1].ethPortAdminStatus = TRUE;*/

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP)
		return( RERROR );
	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return ( RERROR );

	if( EthPort > CTC_ONU_MAX_ETHPORT )
		return( RERROR );

    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_SetEthPortAdminStatus( PonPortIdx, OnuIdx, EthPort, TRUE);
	#else
	ret = CTC_STACK_set_phy_admin_control( PonPortIdx, llid, EthPort, TRUE);
	#endif

	if( ret == PAS_EXIT_OK ) return( ROK );
	else return( RERROR );	
}


int CTC_DownOnuOneEthPort(short int PonPortIdx, short int OnuIdx, unsigned char EthPort)
{
	short int llid;
	short int ret;
	
	CHECK_ONU_RANGE

	/*onuEthPort[PonPortIdx][OnuIdx][EthPort-1].ethPortAdminStatus = FALSE;*/

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP)
		return( RERROR );
	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return ( RERROR );

	if( EthPort > CTC_ONU_MAX_ETHPORT )
		return( RERROR );

    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_SetEthPortAdminStatus( PonPortIdx, OnuIdx, EthPort, FALSE);
	#else
	ret = CTC_STACK_set_phy_admin_control( PonPortIdx, llid, EthPort, FALSE);
	#endif

	if( ret == PAS_EXIT_OK ) return( ROK );
	else return( RERROR );	
}

int CTC_UpOnuAllEthPort(short int PonPortIdx, short int OnuIdx )
{
	int retVal;
	int value;
	short int OnuEntry;
	unsigned char EthPortIdx, EthPortNum, PortCount=0 ;
	
	CHECK_ONU_RANGE	

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP)
		return( RERROR );

	retVal = CTC_getDeviceCapEthPortNum( PonPortIdx, OnuIdx,  &value );
	if( retVal != ROK ) return( RERROR );

	if( value > CTC_ONU_MAX_ETHPORT ) 
		{
		sys_console_printf("  onu %d/%d/%d capability err,eht port num=%d;try again\r\n",GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1),value);
		CTC_GetOnuCapability( PonPortIdx, OnuIdx );
		}
	retVal = CTC_getDeviceCapEthPortNum( PonPortIdx, OnuIdx,  &value );
	if( retVal != ROK ) return( RERROR );
	if( value > CTC_ONU_MAX_ETHPORT ) 
		{
		sys_console_printf("  onu %d/%d/%d capability err,eht port num=%d\r\n",GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1),value);
		return( RERROR );
		}

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
	ONU_MGMT_SEM_TAKE;
	EthPortNum = OnuMgmtTable[OnuEntry].FE_Ethernet_ports_number;
	ONU_MGMT_SEM_GIVE;

	if( EthPortNum != 0 )
	{
		for( EthPortIdx = 0; EthPortIdx < CTC_ONU_MAX_ETHPORT; EthPortIdx ++ )
		{
			if(((OnuMgmtTable[OnuEntry].Ports_distribution[EthPortIdx/4] >> ( ( EthPortIdx%4 )*2 ) ) & 3 ) == FE_INTERFACE )
			{
				PortCount++;
				CTC_UpOnuOneEthPort( PonPortIdx, OnuIdx, (EthPortIdx+1));
				if( PortCount == EthPortNum ) break;
			}
		}
	}
	
	ONU_MGMT_SEM_TAKE;
	EthPortNum = OnuMgmtTable[OnuEntry].GE_Ethernet_ports_number;
	ONU_MGMT_SEM_GIVE;
	if( EthPortNum != 0 )
	{
		for( EthPortIdx = 0; EthPortIdx < CTC_ONU_MAX_ETHPORT; EthPortIdx ++ )
		{
			if(((OnuMgmtTable[OnuEntry].Ports_distribution[EthPortIdx/4] >> ( ( EthPortIdx%4 )*2 ) ) & 3 ) == GE_INTERFACE)
			{
				PortCount++;
				CTC_UpOnuOneEthPort( PonPortIdx, OnuIdx, (EthPortIdx+1));
				if( PortCount == EthPortNum ) break;
			}
		}
	}

	return(ROK );	
	
}

int CTC_DownOnuAllEthPort(short int PonPortIdx, short int OnuIdx )
{
	int retVal;
	int value;
	short int OnuEntry;
	unsigned char EthPortIdx, EthPortNum, PortCount=0 ;
	
	CHECK_ONU_RANGE	

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP)
		return( RERROR );

	retVal = CTC_getDeviceCapEthPortNum( PonPortIdx, OnuIdx,  &value );
	if( retVal != ROK ) return( RERROR );

	if( value > CTC_ONU_MAX_ETHPORT ) 
		{
		sys_console_printf("  onu %d/%d/%d capability err,eht port num=%d;try again\r\n",GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1),value);
		CTC_GetOnuCapability( PonPortIdx, OnuIdx );
		}
	retVal = CTC_getDeviceCapEthPortNum( PonPortIdx, OnuIdx,  &value );
	if( retVal != ROK ) return( RERROR );
	if( value > CTC_ONU_MAX_ETHPORT ) 
		{
		sys_console_printf("  onu %d/%d/%d capability err,eht port num=%d\r\n",GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1),value);
		return( RERROR );
		}

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
	ONU_MGMT_SEM_TAKE;
	EthPortNum = OnuMgmtTable[OnuEntry].FE_Ethernet_ports_number;
	ONU_MGMT_SEM_GIVE;

	if( EthPortNum != 0 )
		{
		for( EthPortIdx = 0; EthPortIdx < CTC_ONU_MAX_ETHPORT; EthPortIdx ++ )
			{
			if(((OnuMgmtTable[OnuEntry].Ports_distribution[EthPortIdx/4] >> ( ( EthPortIdx%4 )*2 ) ) & 3 ) == FE_INTERFACE )
				{
				PortCount++;
				CTC_DownOnuOneEthPort( PonPortIdx, OnuIdx, (EthPortIdx+1));
				if( PortCount == EthPortNum ) break;
				}
			}
		}
	
	ONU_MGMT_SEM_TAKE;
	EthPortNum = OnuMgmtTable[OnuEntry].GE_Ethernet_ports_number;
	ONU_MGMT_SEM_GIVE;
	if( EthPortNum != 0 )
		{
		for( EthPortIdx = 0; EthPortIdx < CTC_ONU_MAX_ETHPORT; EthPortIdx ++ )
			{
			if(((OnuMgmtTable[OnuEntry].Ports_distribution[EthPortIdx/4] >> ( ( EthPortIdx%4 )*2 ) ) & 3 ) == GE_INTERFACE)
				{
				PortCount++;
				CTC_DownOnuOneEthPort( PonPortIdx, OnuIdx, (EthPortIdx+1));
				if( PortCount == EthPortNum ) break;
				}
			}
		}

	return( ROK );
}


int SetOnuEthPortVlanTranslation( short int PonPortIdx, short int OnuIdx, unsigned char port_number, unsigned long def_vlan, int old_vlanId, int new_vlanId )
{
	short int ret;
	short int llid;
	CTC_STACK_port_vlan_configuration_t   port_configuration;

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	port_configuration.mode = CTC_VLAN_MODE_TRANSLATION;
	port_configuration.default_vlan = def_vlan;
	port_configuration.vlan_list[0] = old_vlanId;
	port_configuration.vlan_list[1] = new_vlanId;
	port_configuration.number_of_entries = 1;

	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_SetEthPortVlanConfig ( PonPortIdx, OnuIdx, port_number, &port_configuration);
	#else
	ret = CTC_STACK_set_vlan_port_configuration ( PonPortIdx, llid, port_number, port_configuration);
    #endif
	if( ret != PAS_EXIT_OK) return( RERROR );
	return( ROK );

}

int SetOnuEthPortVlanTag( short int PonPortIdx, short int OnuIdx, unsigned char port_number, int Tag )
{
	short int ret;
	short int llid;
	CTC_STACK_port_vlan_configuration_t   port_configuration;

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	port_configuration.mode = CTC_VLAN_MODE_TAG;
	/*port_configuration.default_vlan = 5;	
	port_configuration.vlan_list[1] = new_vlanId; */
	port_configuration.vlan_list[0] = Tag;
	port_configuration.number_of_entries = 1;

	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_SetEthPortVlanConfig( PonPortIdx, OnuIdx, port_number, &port_configuration);
	#else
	ret = CTC_STACK_set_vlan_port_configuration ( PonPortIdx, llid, port_number, port_configuration);
    #endif
	if( ret != PAS_EXIT_OK) return( RERROR );
	return( ROK );

}

int SetOnuEthPortVlanTransparent( short int PonPortIdx, short int OnuIdx, unsigned char port_number)
{
	short int ret;
	short int llid;
	CTC_STACK_port_vlan_configuration_t   port_configuration;

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&port_configuration, 0, sizeof(CTC_STACK_port_vlan_configuration_t));

	port_configuration.mode = CTC_VLAN_MODE_TRANSPARENT;
	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_SetEthPortVlanConfig ( PonPortIdx, OnuIdx, port_number, &port_configuration);
	#else
	ret = CTC_STACK_set_vlan_port_configuration ( PonPortIdx, llid, port_number, port_configuration);
    #endif
	if( ret != PAS_EXIT_OK) return( RERROR );
	return( ROK );

}

int Qos_flag=0;
CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];

int SetOnuEthPortQosPriMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned char pri, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition )
{
	short int ret;
	short int llid;
	int i;
	/*int j;*/

	/*CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];*/
	

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&(classification_and_marking[0]), 0, (sizeof(CTC_STACK_classification_rule_t ) * CTC_MAX_CLASS_RULES_COUNT ));

	for(i=0; i < CTC_MAX_CLASS_RULES_COUNT; i++)
		classification_and_marking[i].valid = FALSE;

	classification_and_marking[1].valid = TRUE;
	classification_and_marking[1].queue_mapped =mapping_Que;
	classification_and_marking[1].priority_mark = mapping_pri;
	classification_and_marking[1].num_of_entries = 1;
	
	classification_and_marking[1].entries[0].field_select = CTC_FIELD_SEL_ETHERNET_PRIORITY;
	classification_and_marking[1].entries[0].value.match_value = pri;
	classification_and_marking[1].entries[0].validation_operator = Match_condition;

	/*
	j = 2;
	for( i=0; i<=7; i++)
		{
		if (i == pri ) continue;
		classification_and_marking[j].valid = TRUE;
		classification_and_marking[j].queue_mapped = mapping_Que;
		if( Qos_flag == 0 )
			classification_and_marking[j].priority_mark = i;
		else 
			classification_and_marking[j].priority_mark = 255;
		classification_and_marking[j].num_of_entries = 1;

		classification_and_marking[j].entries[0].field_select = CTC_FIELD_SEL_ETHERNET_PRIORITY;
		classification_and_marking[j].entries[0].value.match_value = i;
		classification_and_marking[j].entries[0].validation_operator = Match_condition;

		j++;

		}
	*/
	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret =  OnuMgt_SetEthPortClassificationAndMarking ( PonPortIdx, OnuIdx,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
	#else
	ret =  CTC_STACK_set_classification_and_marking ( PonPortIdx, llid,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #endif
	if( ret != PAS_EXIT_OK)
		{
		sys_console_printf("pas-soft errId %d\r\n", ret );
		return( RERROR );
		}
	return( ROK );
}

int SetOnuEthPortQosDMacMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned char *MacAddr, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition )
{
	short int ret;
	short int llid;
	int i;

	/*CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];*/
	
	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&(classification_and_marking[0]), 0, (sizeof(CTC_STACK_classification_rule_t ) * CTC_MAX_CLASS_RULES_COUNT ));
	
	for(i=0; i < CTC_MAX_CLASS_RULES_COUNT; i++)
		classification_and_marking[i].valid = FALSE;

	classification_and_marking[1].valid = TRUE;
	classification_and_marking[1].queue_mapped =mapping_Que;
	classification_and_marking[1].priority_mark = mapping_pri;
	classification_and_marking[1].num_of_entries = 1;
	
	classification_and_marking[1].entries[0].field_select = CTC_FIELD_SEL_DA_MAC;
	VOS_MemCpy(classification_and_marking[1].entries[0].value.mac_address, MacAddr, BYTES_IN_MAC_ADDRESS );
	classification_and_marking[1].entries[0].validation_operator = Match_condition;
	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret =  OnuMgt_SetEthPortClassificationAndMarking ( PonPortIdx, OnuIdx,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
	#else
	ret =  CTC_STACK_set_classification_and_marking ( PonPortIdx, llid,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #endif
	if( ret != PAS_EXIT_OK)
		{
		sys_console_printf("pas-soft errId %d\r\n", ret );
		return( RERROR );
		}
	return( ROK );
}

int SetOnuEthPortQosSMacMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned char *MacAddr, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition )
{
	short int ret;
	short int llid;
	int i;

	/*CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];*/
	

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&(classification_and_marking[0]), 0, (sizeof(CTC_STACK_classification_rule_t ) * CTC_MAX_CLASS_RULES_COUNT ));

	for(i=0; i < CTC_MAX_CLASS_RULES_COUNT; i++)
		classification_and_marking[i].valid = FALSE;

	classification_and_marking[1].valid = TRUE;
	classification_and_marking[1].queue_mapped =mapping_Que;
	classification_and_marking[1].priority_mark = mapping_pri;
	classification_and_marking[1].num_of_entries = 1;
	
	classification_and_marking[1].entries[0].field_select = CTC_FIELD_SEL_SA_MAC;
	VOS_MemCpy(classification_and_marking[1].entries[0].value.mac_address, MacAddr, BYTES_IN_MAC_ADDRESS );
	classification_and_marking[1].entries[0].validation_operator = Match_condition;
	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret =  OnuMgt_SetEthPortClassificationAndMarking ( PonPortIdx, OnuIdx,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
	#else
	ret =  CTC_STACK_set_classification_and_marking ( PonPortIdx, llid,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #endif
	if( ret != PAS_EXIT_OK)
		{
		sys_console_printf("pas-soft errId %d\r\n", ret );
		return( RERROR );
		}
	return( ROK );
}


int SetOnuEthPortQosVidMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int Vid, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition )
{
	short int ret;
	short int llid;
	int i;

	/*CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];*/
	

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&(classification_and_marking[0]), 0, (sizeof(CTC_STACK_classification_rule_t ) * CTC_MAX_CLASS_RULES_COUNT ));

	for(i=0; i < CTC_MAX_CLASS_RULES_COUNT; i++)
		classification_and_marking[i].valid = FALSE;

	classification_and_marking[1].valid = TRUE;
	classification_and_marking[1].queue_mapped =mapping_Que;
	classification_and_marking[1].priority_mark = mapping_pri;
	classification_and_marking[1].num_of_entries = 1;
	
	classification_and_marking[1].entries[0].field_select = CTC_FIELD_SEL_VLAN_ID;
	classification_and_marking[1].entries[0].value.match_value = Vid;
	classification_and_marking[1].entries[0].validation_operator = Match_condition;
	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret =  OnuMgt_SetEthPortClassificationAndMarking ( PonPortIdx, OnuIdx,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
	#else
	ret =  CTC_STACK_set_classification_and_marking ( PonPortIdx, llid,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #endif
	if( ret != PAS_EXIT_OK)
		{
		sys_console_printf("pas-soft errId %d\r\n", ret );
		return( RERROR );
		}
	return( ROK );
}


int SetOnuEthPortQosEthTypeMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int EthType, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition )
{
	short int ret;
	short int llid;
	int i;

	/*CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];*/
	

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&(classification_and_marking[0]), 0, (sizeof(CTC_STACK_classification_rule_t ) * CTC_MAX_CLASS_RULES_COUNT ));

	for(i=0; i < CTC_MAX_CLASS_RULES_COUNT; i++)
		classification_and_marking[i].valid = FALSE;

	classification_and_marking[1].valid = TRUE;
	classification_and_marking[1].queue_mapped =mapping_Que;
	classification_and_marking[1].priority_mark = mapping_pri;
	classification_and_marking[1].num_of_entries = 1;
	
	classification_and_marking[1].entries[0].field_select = CTC_FIELD_SEL_ETHER_TYPE;
	classification_and_marking[1].entries[0].value.match_value = EthType;
	classification_and_marking[1].entries[0].validation_operator = Match_condition;
	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret =  OnuMgt_SetEthPortClassificationAndMarking ( PonPortIdx, OnuIdx,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
	#else
	ret =  CTC_STACK_set_classification_and_marking ( PonPortIdx, llid,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #endif
	if( ret != PAS_EXIT_OK)
		{
		sys_console_printf("pas-soft errId %d\r\n", ret );
		return( RERROR );
		}
	return( ROK );
}

int SetOnuEthPortQosDipMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int ipAddr, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition )
{
	short int ret;
	short int llid;
	int i;

	/*CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];*/
	

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&(classification_and_marking[0]), 0, (sizeof(CTC_STACK_classification_rule_t ) * CTC_MAX_CLASS_RULES_COUNT ));

	for(i=0; i < CTC_MAX_CLASS_RULES_COUNT; i++)
		classification_and_marking[i].valid = FALSE;

	classification_and_marking[1].valid = TRUE;
	classification_and_marking[1].queue_mapped =mapping_Que;
	classification_and_marking[1].priority_mark = mapping_pri;
	classification_and_marking[1].num_of_entries = 1;
	
	classification_and_marking[1].entries[0].field_select = CTC_FIELD_SEL_DEST_IP;
	classification_and_marking[1].entries[0].value.match_value = ipAddr;
	classification_and_marking[1].entries[0].validation_operator = Match_condition;
	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret =  OnuMgt_SetEthPortClassificationAndMarking ( PonPortIdx, OnuIdx,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
	#else
	ret =  CTC_STACK_set_classification_and_marking ( PonPortIdx, llid,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #endif
	if( ret != PAS_EXIT_OK)
		{
		sys_console_printf("pas-soft errId %d\r\n", ret );
		return( RERROR );
		}
	return( ROK );
}


int SetOnuEthPortQosSipMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int ipAddr, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition )
{
	short int ret;
	short int llid;
	int i;

	/*CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];*/
	

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&(classification_and_marking[0]), 0, (sizeof(CTC_STACK_classification_rule_t ) * CTC_MAX_CLASS_RULES_COUNT ));

	for(i=0; i < CTC_MAX_CLASS_RULES_COUNT; i++)
		classification_and_marking[i].valid = FALSE;

	classification_and_marking[1].valid = TRUE;
	classification_and_marking[1].queue_mapped =mapping_Que;
	classification_and_marking[1].priority_mark = mapping_pri;
	classification_and_marking[1].num_of_entries = 1;
	
	classification_and_marking[1].entries[0].field_select = CTC_FIELD_SEL_SRC_IP;
	classification_and_marking[1].entries[0].value.match_value = ipAddr;
	classification_and_marking[1].entries[0].validation_operator = Match_condition;
	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret =  OnuMgt_SetEthPortClassificationAndMarking ( PonPortIdx, OnuIdx,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #else
	ret =  CTC_STACK_set_classification_and_marking ( PonPortIdx, llid,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #endif
	if( ret != PAS_EXIT_OK)
		{
		sys_console_printf("pas-soft errId %d\r\n", ret );
		return( RERROR );
		}
	return( ROK );
}

int SetOnuEthPortQosIpTypeMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int Iptype, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition )
{
	short int ret;
	short int llid;
	int i;

	/*CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];*/
	

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&(classification_and_marking[0]), 0, (sizeof(CTC_STACK_classification_rule_t ) * CTC_MAX_CLASS_RULES_COUNT ));

	for(i=0; i < CTC_MAX_CLASS_RULES_COUNT; i++)
		classification_and_marking[i].valid = FALSE;

	classification_and_marking[1].valid = TRUE;
	classification_and_marking[1].queue_mapped =mapping_Que;
	classification_and_marking[1].priority_mark = mapping_pri;
	classification_and_marking[1].num_of_entries = 1;
	
	classification_and_marking[1].entries[0].field_select = CTC_FIELD_SEL_IP_PROTOCOL_TYPE;
	classification_and_marking[1].entries[0].value.match_value = Iptype;
	classification_and_marking[1].entries[0].validation_operator = Match_condition;
	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret =  OnuMgt_SetEthPortClassificationAndMarking ( PonPortIdx, OnuIdx,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #else
	ret =  CTC_STACK_set_classification_and_marking ( PonPortIdx, llid,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #endif
	if( ret != PAS_EXIT_OK)
		{
		sys_console_printf("pas-soft errId %d\r\n", ret );
		return( RERROR );
		}
	return( ROK );
}

int SetOnuEthPortQosIpv4DSCPMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int Tos_Dscp, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition )
{
	short int ret;
	short int llid;
	int i;

	/*CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];*/
	

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&(classification_and_marking[0]), 0, (sizeof(CTC_STACK_classification_rule_t ) * CTC_MAX_CLASS_RULES_COUNT ));

	for(i=0; i < CTC_MAX_CLASS_RULES_COUNT; i++)
		classification_and_marking[i].valid = FALSE;

	classification_and_marking[1].valid = TRUE;
	classification_and_marking[1].queue_mapped =mapping_Que;
	classification_and_marking[1].priority_mark = mapping_pri;
	classification_and_marking[1].num_of_entries = 1;
	
	classification_and_marking[1].entries[0].field_select = CTC_FIELD_SEL_IPV4_TOS_DSCP;
	classification_and_marking[1].entries[0].value.match_value = Tos_Dscp;
	classification_and_marking[1].entries[0].validation_operator = Match_condition;
	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret =  OnuMgt_SetEthPortClassificationAndMarking ( PonPortIdx, OnuIdx,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #else
	ret =  CTC_STACK_set_classification_and_marking ( PonPortIdx, llid,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #endif
	if( ret != PAS_EXIT_OK)
		{
		sys_console_printf("pas-soft errId %d\r\n", ret );
		return( RERROR );
		}
	return( ROK );
}


int SetOnuEthPortQosIpv6PrecMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int  precedence, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition )
{
	short int ret;
	short int llid;
	int i;

	/*CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];*/
	

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&(classification_and_marking[0]), 0, (sizeof(CTC_STACK_classification_rule_t ) * CTC_MAX_CLASS_RULES_COUNT ));

	for(i=0; i < CTC_MAX_CLASS_RULES_COUNT; i++)
		classification_and_marking[i].valid = FALSE;

	classification_and_marking[1].valid = TRUE;
	classification_and_marking[1].queue_mapped =mapping_Que;
	classification_and_marking[1].priority_mark = mapping_pri;
	classification_and_marking[1].num_of_entries = 1;
	
	classification_and_marking[1].entries[0].field_select = CTC_FIELD_SEL_IPV6_TRAFFIC_CLASS;
	classification_and_marking[1].entries[0].value.match_value = precedence;
	classification_and_marking[1].entries[0].validation_operator = Match_condition;
	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret =  OnuMgt_SetEthPortClassificationAndMarking ( PonPortIdx, OnuIdx,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #else
	ret =  CTC_STACK_set_classification_and_marking ( PonPortIdx, llid,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #endif
	if( ret != PAS_EXIT_OK)
		{
		sys_console_printf("pas-soft errId %d\r\n", ret );
		return( RERROR );
		}
	return( ROK );
}


int SetOnuEthPortQosIpSrcPortMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int  SrcPort, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition )
{
	short int ret;
	short int llid;
	int i;

	/*CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];*/
	

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&(classification_and_marking[0]), 0, (sizeof(CTC_STACK_classification_rule_t ) * CTC_MAX_CLASS_RULES_COUNT ));

	for(i=0; i < CTC_MAX_CLASS_RULES_COUNT; i++)
		classification_and_marking[i].valid = FALSE;

	classification_and_marking[1].valid = TRUE;
	classification_and_marking[1].queue_mapped =mapping_Que;
	classification_and_marking[1].priority_mark = mapping_pri;
	classification_and_marking[1].num_of_entries = 1;
	
	classification_and_marking[1].entries[0].field_select = CTC_FIELD_SEL_L4_SRC_PORT;
	classification_and_marking[1].entries[0].value.match_value = SrcPort;
	classification_and_marking[1].entries[0].validation_operator = Match_condition;
	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret =  OnuMgt_SetEthPortClassificationAndMarking ( PonPortIdx, OnuIdx,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #else
	ret =  CTC_STACK_set_classification_and_marking ( PonPortIdx, llid,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #endif
	if( ret != PAS_EXIT_OK)
		{
		sys_console_printf("pas-soft errId %d\r\n", ret );
		return( RERROR );
		}
	return( ROK );
}


int SetOnuEthPortQosIpDestPortMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int  DesPort, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition )
{
	short int ret;
	short int llid;
	int i;

	/*CTC_STACK_classification_rule_t  classification_and_marking[CTC_MAX_CLASS_RULES_COUNT];*/
	

	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( RERROR );

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	VOS_MemSet((void *)&(classification_and_marking[0]), 0, (sizeof(CTC_STACK_classification_rule_t ) * CTC_MAX_CLASS_RULES_COUNT ));

	for(i=0; i < CTC_MAX_CLASS_RULES_COUNT; i++)
		classification_and_marking[i].valid = FALSE;

	classification_and_marking[1].valid = TRUE;
	classification_and_marking[1].queue_mapped =mapping_Que;
	classification_and_marking[1].priority_mark = mapping_pri;
	classification_and_marking[1].num_of_entries = 1;
	
	classification_and_marking[1].entries[0].field_select = CTC_FIELD_SEL_L4_DEST_PORT;
	classification_and_marking[1].entries[0].value.match_value = DesPort;
	classification_and_marking[1].entries[0].validation_operator = Match_condition;
	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret =  OnuMgt_SetEthPortClassificationAndMarking ( PonPortIdx, OnuIdx,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #else
	ret =  CTC_STACK_set_classification_and_marking ( PonPortIdx, llid,  port,  CTC_CLASSIFICATION_ADD_RULE, classification_and_marking);
    #endif
	if( ret != PAS_EXIT_OK)
		{
		sys_console_printf("pas-soft errId %d\r\n", ret );
		return( RERROR );
		}
	return( ROK );
}


int DelOnuEthPortQosRule(short int PonPortIdx, short int OnuIdx, unsigned char port )
{
	short int retVal;
	short int llid;

	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
		return( RERROR );
	llid = GetLlidByOnuIdx( PonPortIdx,  OnuIdx);
	if( llid == INVALID_LLID) return( RERROR );

	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	retVal = OnuMgt_ClearEthPortClassificationAndMarking ( PonPortIdx, OnuIdx, port );
	#else
	retVal = CTC_STACK_delete_classification_and_marking_list ( PonPortIdx, llid, port );
	#endif
	if( retVal != PAS_EXIT_OK ) return( RERROR );
	else return( ROK );
}



static unsigned char  num = 2, portNum =1;
CTC_STACK_onu_queue_set_thresholds_t  Queue_threshold[8];

int SetOnuDBAParameter( short int PonPortIdx, short int OnuIdx )
{

	short int onu_id, ret;
	unsigned char  i,j;
	/*CTC_STACK_onu_queue_set_thresholds_t  Queue_threshold[8];*/
	
	CHECK_ONU_RANGE

	VOS_MemSet(Queue_threshold, 0 , sizeof( CTC_STACK_onu_queue_set_thresholds_t ) *8);

	onu_id = GetLlidByOnuIdx(  PonPortIdx,   OnuIdx);
	if(onu_id == INVALID_LLID ) return( RERROR );

	for( i=0; i< num; i++)
		for(j=0;j<8;j++)
		{
		Queue_threshold[i].queue[j].state = j%2;
		Queue_threshold[i].queue[j].threshold = (i+1) * 1000+(j+1)*50;
		}
	/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_SetDbaReportThresholds( PonPortIdx, OnuIdx, &num, &Queue_threshold[0] );
	if( ret == PAS_EXIT_OK ) return( ROK );
	else {
		sys_console_printf("\r\nSet DBA Threshold param Err %d \r\n", ret );
		return( RERROR );
		}

}


	
int GetOnuDBAParameter( short int PonPortIdx, short int OnuIdx )
{

	short int onu_id, ret;
	unsigned char  i,j;

	
	CHECK_ONU_RANGE


	onu_id = GetLlidByOnuIdx(  PonPortIdx,   OnuIdx);
	if(onu_id == INVALID_LLID ) return( RERROR );

	VOS_MemSet( Queue_threshold, 0, sizeof(CTC_STACK_onu_queue_set_thresholds_t) * 8 );

	/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_GetDbaReportThresholds( PonPortIdx, OnuIdx, &num, Queue_threshold );
	if( ret != PAS_EXIT_OK ) 
		{
		sys_console_printf("\r\nSet DBA Threshold param Err %d \r\n", ret );
		return( RERROR );
		}
	sys_console_printf(" DBA Thershold Param Num=%d \r\n", num );
	if( num != 0 )
		{
		for(i=0; i< num; i++)
			{
			sys_console_printf(" queue set %d\r\n", (i+1) );
			for(j=0;j<8;j++)
				{
				sys_console_printf(" queue%d active %d Threshold %d \r\n", (j+1), Queue_threshold[i].queue[j].state, Queue_threshold[i].queue[j].threshold );
				}
			}
		}

	return( ROK );

}


int SetOnuMulticastVlan(short int PonPortIdx, short int OnuIdx )
{
	short int ret;
	short int i;
	short int onu_id;
	CTC_STACK_multicast_vlan_t  Multicast_Vlan;
	CHECK_ONU_RANGE
		
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if(onu_id == INVALID_LLID ) return( RERROR );

	for( i=0; i<num; i++)
		{
		Multicast_Vlan.vlan_operation = CTC_MULTICAST_VLAN_OPER_ADD;
		Multicast_Vlan.vlan_id[i] = (i+1)+500;
		}
	Multicast_Vlan.num_of_vlan_id = num;
	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_SetEthPortMulticastVlan( PonPortIdx, OnuIdx, portNum, &Multicast_Vlan);
	#else
	ret = CTC_STACK_set_multicast_vlan( PonPortIdx, onu_id, portNum, Multicast_Vlan);
	#endif
	if( ret == PAS_EXIT_OK ) return( ROK );
	else {
		sys_console_printf(" set multicast vlan err %d \r\n", ret );
		return( RERROR );
		}
}


int GetOnuMulticastVlan(short int PonPortIdx, short int OnuIdx )
{
	short int ret;
	short int i;
	short int onu_id;
	CTC_STACK_multicast_vlan_t  Multicast_Vlan;
	CHECK_ONU_RANGE
		
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if(onu_id == INVALID_LLID ) return( RERROR );

    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_GetEthPortMulticastVlan( PonPortIdx, OnuIdx, portNum, &Multicast_Vlan);
	#else
	ret = CTC_STACK_get_multicast_vlan( PonPortIdx, onu_id, portNum, &Multicast_Vlan);
	#endif
	if( ret != PAS_EXIT_OK ) 
		{
		sys_console_printf(" get multicast vlan err %d \r\n", ret );
		return( RERROR );
		}
	sys_console_printf(" multicast vlan Num=%d \r\n", Multicast_Vlan.num_of_vlan_id );
	if(  Multicast_Vlan.num_of_vlan_id != 0 ) 
		for( i =0; i< Multicast_Vlan.num_of_vlan_id; i++ )
			sys_console_printf(" vlan%d Id=%d\r\n", (i+1), Multicast_Vlan.vlan_id[i] );

	return( ROK );
			
}

/*
int SetOnuMulticastVlanControl( short int PonPortIdx, short int OnuIdx )
{

	CTC_STACK_set_multicast_control(const PON_olt_id_t olt_id, const PON_onu_id_t onu_id, const CTC_STACK_multicast_control_t multicast_control);
	CTC_STACK_set_multicast_group_num(const PON_olt_id_t olt_id, const PON_onu_id_t onu_id, const unsigned char port_number, const unsigned char group_num)
	CTC_STACK_get_multicast_group_num(const PON_olt_id_t olt_id, const PON_onu_id_t onu_id, const unsigned char port_number, unsigned char * group_num)
}

int GetOnuMulticastVlanControl( short int PonPortIdx, short int OnuIdx )
{

CTC_STACK_get_multicast_control(const PON_olt_id_t olt_id, const PON_onu_id_t onu_id, CTC_STACK_multicast_control_t * multicast_control)


}
*/


#ifdef  ONU_DEVICE_DEACTIVE 
#endif
	/* added by chenfj 2007-6-27 
	    ONU	 device  de-active */

int DeactivateOnu_bak( short int PonPortIdx, short int OnuIdx )
{
	int ret;
	short int onuId;
	short int PonChipType;
		
	CHECK_ONU_RANGE

	ret = ThisIsValidOnu(PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );
	
	ret = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if( ret != ONU_OPER_STATUS_UP ) return( ret );

	onuId = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( onuId == INVALID_LLID ) return( RERROR );

	PonChipType = V2R1_GetPonchipType ( PonPortIdx );
    	
	if(OLT_PONCHIP_ISPAS(PonChipType))
	{
	    CTC_SetOnuUniPort( PonPortIdx, onuId, 1, 0 );
	}
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	else{ /* other pon chip type handler */

		}
	
	return(ROK );

}

int ActivateOnu_bak( short int PonPortIdx, short int OnuIdx )
{
	int ret;
	short int onuId;
	short int PonChipType;
	
	
	CHECK_ONU_RANGE

	ret = ThisIsValidOnu(PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );
	
	ret = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if( ret != ONU_OPER_STATUS_UP ) return( ret );

	onuId = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( onuId == INVALID_LLID ) return( RERROR );

    
	PonChipType = V2R1_GetPonchipType ( PonPortIdx );
    
	if(OLT_PONCHIP_ISPAS(PonChipType))
	{
	    CTC_SetOnuUniPort( PonPortIdx, onuId, 1, 1 );
	}/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	else{ /* other pon chip type handler */

		}
	return(ROK );

}


int DeactivateOnu( short int PonPortIdx, short int OnuIdx )
{
	int ret;
	/*short int onuId;*/
	short int PonChipType;
	/*unsigned long auth_mode;*/
	UCHAR MacAddr[6];
	
	CHECK_ONU_RANGE

	ret = ThisIsValidOnu(PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );

	PonChipType = V2R1_GetPonchipType ( PonPortIdx );

	/*if( V2R1_CTC_STACK)*/
	if(GetOnuVendorType( PonPortIdx, OnuIdx ) == ONU_VENDOR_CT )
		{
		/*
		getOnuAuthEnable(&auth_mode);
		if( auth_mode != V2R1_ONU_AUTHENTICATION_DISABLE )	
		*/
		ONU_MGMT_SEM_TAKE;
		VOS_MemCpy( MacAddr, OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, 6 );
		ONU_MGMT_SEM_GIVE;
		DeleteMacAuthentication( PonPortIdx, MacAddr );

		ret =  GetOnuOperStatus(PonPortIdx, OnuIdx );
		if( ret == ONU_OPER_STATUS_UP )
			Onu_deregister( PonPortIdx, OnuIdx );
		}

	if(OLT_PONCHIP_ISPAS(PonChipType) ) 
		{
		ret = GetOnuOperStatus(PonPortIdx, OnuIdx );
		/*sys_console_printf("test shixh 20100618,ret=%d\r\n",ret);*/
		if( ret == ONU_OPER_STATUS_UP )
			Onu_deregister( PonPortIdx, OnuIdx );

		/*
		onuId = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
		if( onuId == INVALID_LLID ) return( RERROR );

		REMOTE_PASONU_uni_set_port( PonPortIdx, onuId, 1, 0 );
		*/
		}

	else{  /* other pon chip type handler */

		}
	
	return(ROK );

}

int ActivateOnu( short int PonPortIdx, short int OnuIdx )
{
	int ret;
	/*short int onuId;*/
	short int PonChipType;
	/*unsigned long auth_mode;*/
	UCHAR MacAddr[6];
	
	CHECK_ONU_RANGE

	ret = ThisIsValidOnu(PonPortIdx, OnuIdx );
	if( ret != ROK ) return( ret );


	PonChipType = V2R1_GetPonchipType ( PonPortIdx );


	/*if( V2R1_CTC_STACK) */
	if(GetOnuVendorType( PonPortIdx, OnuIdx ) == ONU_VENDOR_CT )
	{
		/*
		getOnuAuthEnable(&auth_mode);
		if( auth_mode == V2R1_ONU_AUTHENTICATION_DISABLE ) 
			return( RERROR );
		*/
		ONU_MGMT_SEM_TAKE;
		VOS_MemCpy( MacAddr, OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, 6 );
		ONU_MGMT_SEM_GIVE;
		AddMacAuthentication( PonPortIdx, MacAddr );
	}
	
	else if(OLT_PONCHIP_ISPAS(PonChipType) ) 
		{
		unsigned char MacAddr[6];
		int len=0;
		
		GetOnuMacAddr( PonPortIdx, OnuIdx, MacAddr,&len);	
		ActivateOnePendingOnu( PonPortIdx, MacAddr );		

		/*
		ret = GetOnuOperStatus( PonPortIdx, OnuIdx ) ;
		if( ret != ONU_OPER_STATUS_UP )
			return( RERROR );
		onuId = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
		if( onuId == INVALID_LLID ) return( RERROR );
	
		REMOTE_PASONU_uni_set_port( PonPortIdx, onuId, 1, 1 );
		*/
		}

	else{ /* other pon chip type handler */

		}
	return(ROK );

}

int  SetOnuTrafficServiceEnable( short int PonPortIdx, short int OnuIdx, int TrafficServiceEnable )
{
	int ret;
	
	CHECK_ONU_RANGE

	if( ThisIsValidOnu( PonPortIdx, OnuIdx ) != ROK ) return( RERROR );

#if 1
    ret = OnuMgt_SetOnuTrafficServiceMode(PonPortIdx, OnuIdx, TrafficServiceEnable);
#else
	/*if( V2R1_CTC_STACK )*/
	if(GetOnuVendorType( PonPortIdx, OnuIdx ) == ONU_VENDOR_CT )
	{
		if( TrafficServiceEnable == V2R1_ENABLE )
		{
			/*if( GetOnuTrafficServiceEnable( PonPortIdx, OnuIdx ) == V2R1_ENABLE ) return( ROK );*/
			ret = ActivateOnu( PonPortIdx, OnuIdx );
			if( ret == ROK )
				OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].TrafficServiceEnable = V2R1_ENABLE;

		}
		else if( TrafficServiceEnable == V2R1_DISABLE )
		{
			/*if( GetOnuTrafficServiceEnable( PonPortIdx, OnuIdx ) == V2R1_DISABLE ) return( ROK );
			VOS_TaskLock();*/
			ret = DeactivateOnu(PonPortIdx, OnuIdx);
			if ( ret == ROK )
				OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].TrafficServiceEnable = V2R1_DISABLE;
			/*VOS_TaskUnlock();
			Onu_ShutDown( PonPortIdx, OnuIdx );*/
		}
		else
            return( RERROR );

		return( ret );
	}
	else
    {
		if( GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP )
			{
			if( TrafficServiceEnable == V2R1_ENABLE )
				{
				/*if( GetOnuTrafficServiceEnable( PonPortIdx, OnuIdx ) == V2R1_ENABLE ) return( ROK );*/
				ret = ActivateOnu( PonPortIdx, OnuIdx );
				if( ret == ROK )
					OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].TrafficServiceEnable = V2R1_ENABLE;
				}
			else if( TrafficServiceEnable == V2R1_DISABLE )
				{
				/*if( GetOnuTrafficServiceEnable( PonPortIdx, OnuIdx ) == V2R1_DISABLE ) return( ROK );
				VOS_TaskLock();*/
				ret = DeactivateOnu(PonPortIdx, OnuIdx);
				if ( ret == ROK )
					OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].TrafficServiceEnable = V2R1_DISABLE;
				/*Onu_ShutDown( PonPortIdx, OnuIdx );
				VOS_TaskUnlock();*/
				}
			else return( RERROR );
			return( ret );
			}

		else/*if(OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].OperStatus == ONU_OPER_STATUS_DOWN )*/
			{
			if( TrafficServiceEnable == V2R1_ENABLE ) 
				{
				ActivateOnu( PonPortIdx, OnuIdx );
				OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].TrafficServiceEnable = V2R1_ENABLE;
				}
			else  if( TrafficServiceEnable == V2R1_DISABLE ) 
				{
				/*VOS_TaskLock();*/
				DeactivateOnu(PonPortIdx, OnuIdx);
				OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].TrafficServiceEnable = V2R1_DISABLE;
				/*VOS_TaskUnlock();*/
				}
			else return( RERROR );
			}
		}
#endif

	return ( ret );
}


int  GetOnuTrafficServiceEnable( short int PonPortIdx, short int OnuIdx )
{
	int enable;
	CHECK_ONU_RANGE;

	if( ThisIsValidOnu( PonPortIdx, OnuIdx ) != ROK ) return( V2R1_ENABLE );
	ONU_MGMT_SEM_TAKE;
	enable = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].TrafficServiceEnable;
	ONU_MGMT_SEM_GIVE;

	return enable;
}

int CopyOnuTrafficServiceMode(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iRlt = 0;
    int iSrcEntry;
    int iSrcCfg;

    iSrcEntry = SrcPonPortIdx * MAXONUPERPON + SrcOnuIdx;
    ONU_MGMT_SEM_TAKE;
    iSrcCfg   = OnuMgmtTable[iSrcEntry].TrafficServiceEnable;
    ONU_MGMT_SEM_GIVE;
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OnuMgt_SetOnuTrafficServiceMode(DstPonPortIdx, DstOnuIdx, iSrcCfg);
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( V2R1_DISABLE == iSrcCfg )
        {
            iRlt = OnuMgt_SetOnuTrafficServiceMode(DstPonPortIdx, DstOnuIdx, iSrcCfg);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            int iDstEntry;
            int iDstCfg;
            
            iDstEntry = DstPonPortIdx * MAXONUPERPON + DstOnuIdx;
            ONU_MGMT_SEM_TAKE;
            iDstCfg   = OnuMgmtTable[iDstEntry].TrafficServiceEnable;
            ONU_MGMT_SEM_GIVE;
            if ( iDstCfg != iSrcCfg )
            {
                iRlt = OnuMgt_SetOnuTrafficServiceMode(DstPonPortIdx, DstOnuIdx, iSrcCfg);
            }
        }
        else
        {
            iRlt = OnuMgt_SetOnuTrafficServiceMode(DstPonPortIdx, DstOnuIdx, iSrcCfg);
        }
    }

    return iRlt;
}

int  AuthorizeOnuToNetwork(short int PonPortIdx, short int OnuIdx )
{
	short int onu_id;
	short int PonChipType;
	UCHAR MacAddr[6];

	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
	if(onu_id == INVALID_LLID) return(RERROR);
	
	if( EVENT_REGISTER== V2R1_ENABLE )
		sys_console_printf("\r\n Authorize this onu(%d/%d/%d) ...", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuIdx+1);

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(OLT_PONCHIP_ISPAS(PonChipType)) 
	{
	    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if( OnuMgt_AuthorizeOnu ( PonPortIdx,  OnuIdx, TRUE ) == PAS_EXIT_OK )
		{
			if( EVENT_REGISTER == V2R1_ENABLE)
				sys_console_printf(" ok\r\n");
			return(ROK);
		}
		else
		{
			if( EVENT_REGISTER == V2R1_ENABLE )
				sys_console_printf(" err\r\n");
			ONU_MGMT_SEM_TAKE;
			VOS_MemCpy( MacAddr, OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].DeviceInfo.MacAddr, 6 );
			ONU_MGMT_SEM_GIVE;
			AddPendingOnu( PonPortIdx, OnuIdx, onu_id, MacAddr, 0);
			/*PAS_deregister_onu( PonPortIdx, onu_id, FALSE );*/
			return( RERROR );
		}
	}
	return(RERROR);
}

int  DenyOnuFromNetWork(short int PonPortIdx, short int OnuIdx )
{
	short int onu_id;
	short int PonChipType;
	UCHAR MacAddr[6];

	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
	if(onu_id == INVALID_LLID) return(RERROR);
	
	if( EVENT_REGISTER== V2R1_ENABLE )
		sys_console_printf("\r\n deny this onu ...");

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(OLT_PONCHIP_ISPAS(PonChipType)) 
		{
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if( OnuMgt_AuthorizeOnu ( PonPortIdx,  OnuIdx, FALSE ) == PAS_EXIT_OK )
			{
			if( EVENT_REGISTER == V2R1_ENABLE)
				sys_console_printf(" ok\r\n");
			return(ROK);
			}
		else {
			if( EVENT_REGISTER == V2R1_ENABLE )
				sys_console_printf(" err\r\n");
			ONU_MGMT_SEM_TAKE;
			VOS_MemCpy( MacAddr, OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].DeviceInfo.MacAddr, 6 );
			ONU_MGMT_SEM_GIVE;
			AddPendingOnu( PonPortIdx, OnuIdx, onu_id, MacAddr, 0);
			/*PAS_deregister_onu( PonPortIdx, onu_id, FALSE );*/
			return( RERROR );
			}
		}
	return( RERROR );
}

int  EnableOnuOamSlowProtocolLimit(short int PonPortIdx, short int OnuIdx)
{
	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	return(OnuMgt_SetSlowProtocolLimit(PonPortIdx, OnuIdx, ENABLE));
	#else
	short int onu_id;

	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
	if(onu_id == INVALID_LLID) return(RERROR);
	
	return(REMOTE_PASONU_set_slow_protocol_limit(PonPortIdx, onu_id, ENABLE));
	#endif
}

int  DisableOnuOamSlowProtocolLimit(short int PonPortIdx, short int OnuIdx)
{
	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	return(OnuMgt_SetSlowProtocolLimit(PonPortIdx, OnuIdx, DISABLE));
	#else
	short int onu_id;

	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
	if(onu_id == INVALID_LLID) return(RERROR);
	
	return(REMOTE_PASONU_set_slow_protocol_limit(PonPortIdx, onu_id, DISABLE));
	#endif
}

int  GetOnuOamSlowProtocolLimit(short int PonPortIdx, short int OnuIdx, bool *enable)
{
	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	return(OnuMgt_GetSlowProtocolLimit(PonPortIdx, OnuIdx, enable));
	#else
	short int onu_id;

	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
	if(onu_id == INVALID_LLID) return(RERROR);
	
	return(REMOTE_PASONU_get_slow_protocol_limit(PonPortIdx, onu_id, enable));
	#endif
}

/* added by xieshl 20120206, 自动修复ONU Emapper参数 */
extern int OnuMgt_GetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long *size);
extern int OnuMgt_SetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long size);
static LONG resetOnuEmapperParameter( short int PonPortIdx, short int OnuIdx, int resetflag )
{
	const UCHAR default_arbdelta = 4;
	const UCHAR default_txdly = -6;
	const UCHAR default_rxdly = -1;
	
	UCHAR timestamp_delta,clk_calib_tx,clk_calib_rx;
	ULONG  size=1;
	int setflag = 0;

	timestamp_delta = 0;
	clk_calib_tx = 0;
	clk_calib_rx = 0;

	OnuMgt_GetOnuI2CInfo( PonPortIdx, OnuIdx, EEPROM_MAPPER_ARB_PON_TIMESTAMP_DELTA, (void*)&timestamp_delta, &size );
	if(timestamp_delta!=default_arbdelta)
	{
		if( OnuMgt_SetOnuI2CInfo(PonPortIdx, OnuIdx,EEPROM_MAPPER_ARB_PON_TIMESTAMP_DELTA, (void*)&default_arbdelta, 1) != 0 )
			return -1;
		setflag = 1;
	}
					
	OnuMgt_GetOnuI2CInfo( PonPortIdx, OnuIdx, EEPROM_MAPPER_PON_CLK_CALIB_TX, (void*)&clk_calib_tx, &size );
	if(clk_calib_tx!=default_txdly)
	{
		if( OnuMgt_SetOnuI2CInfo(PonPortIdx, OnuIdx, EEPROM_MAPPER_PON_CLK_CALIB_TX, (void*)&default_txdly,1) != 0 )
			return -1;
		setflag = 1;
	}

	OnuMgt_GetOnuI2CInfo( PonPortIdx, OnuIdx, EEPROM_MAPPER_PON_CLK_CALIB_RX, (void*)&clk_calib_rx, &size );
	if(clk_calib_rx!=default_rxdly)
	{
		if( OnuMgt_SetOnuI2CInfo(PonPortIdx, OnuIdx, EEPROM_MAPPER_PON_CLK_CALIB_RX, (void*)&default_rxdly,1) != 0 )
		{
			return -1;
		}
		setflag = 1;
	}

       if(resetflag==1)
       {
		timestamp_delta = 0;
		clk_calib_tx = 0;
		clk_calib_rx = 0;

              OnuMgt_GetOnuI2CInfo( PonPortIdx, OnuIdx, EEPROM_MAPPER_ARB_PON_TIMESTAMP_DELTA, (void*)&timestamp_delta, &size );
              OnuMgt_GetOnuI2CInfo( PonPortIdx, OnuIdx, EEPROM_MAPPER_PON_CLK_CALIB_TX, (void*)&clk_calib_tx, &size );
              OnuMgt_GetOnuI2CInfo( PonPortIdx, OnuIdx, EEPROM_MAPPER_PON_CLK_CALIB_RX, (void*)&clk_calib_rx, &size );
		if( (timestamp_delta != default_arbdelta) && (clk_calib_tx != default_txdly) && (clk_calib_rx != default_rxdly) )
              {
			/*v2r1_printf(0,"onu %d/%d/%d eeprom mapper set failure!\r\n", slotno, portno, OnuIdx + 1);*/
			return -2;
		}
		else
		{
			if( setflag )	/*复位ONU*/
			{
				if( OnuMgt_ResetOnu( PonPortIdx, OnuIdx) != 0 )
				{
					/*v2r1_printf(0,"reset onu %d/%d/%d failure\r\n", slotno, portno, OnuIdx + 1);*/
					return -3;
				}
			}
		}
       }
       if( setflag )
       {
		/*v2r1_printf(0, "set onu %d/%d/%d eeprom mapper sucess!\r\n", slotno, portno, OnuIdx + 1);*/
		return 0;
       }
	return 1;
}

/* GT812A/GT815/GT815_B/GT871/GT871P/GT871R/GT872/GT872P/GT872R/GT873/GT873P/GT873R */
/* GT816/812PB/815PB */
LONG checkOnuEmapperParameter( short int PonPortIdx, short int OnuIdx, int resetflag )
{
	LONG rc = 1;
	int type = V2R1_DEVICE_UNKNOWN;
	
	if( GetOnuType(PonPortIdx, OnuIdx, &type) != ROK )
		return rc;
	
	switch( type )	/* 问题单14708 */
	{
		case V2R1_ONU_GT812_A:
		case V2R1_ONU_GT815:
		case V2R1_ONU_GT815_B:
		case V2R1_ONU_GT871:
		case V2R1_ONU_GT871_P:
		case V2R1_ONU_GT871_R:
		case V2R1_ONU_GT872:
		case V2R1_ONU_GT872_P:
		case V2R1_ONU_GT872_R:
		case V2R1_ONU_GT873:
		case V2R1_ONU_GT873_P:
		case V2R1_ONU_GT873_R:
		case V2R1_ONU_GT816:
		case V2R1_ONU_GT812_B:
			rc = resetOnuEmapperParameter(PonPortIdx, OnuIdx, 1);
			break;
		default:
			break;
	}
	return rc;
}

#ifdef __cplusplus
}
#endif
