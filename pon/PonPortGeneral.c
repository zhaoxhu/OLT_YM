/***************************************************************
*
*						Module Name:  PonPortGeneral.c
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
*   Date: 			2006/04/25
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |-----------|------------------ 
**  06/04/25  |   chenfj          |     create 
**----------|-----------|------------------
**
** 1  modifiy by chenfj 2006/09/19 
**    #2604 问题  telnet用户在串口pon节点下使用命令show current-onu，输出显示在串口上了
** 2 add by chenfj 2006/10/11
**    #2606 问题 2606.在手工添加64个ONU后，再实际注册新ONU时，出现异常
** 3 modified by chenfj 2006/10/20 
**	  #2922问题单register limit的使能与show pon port info回显中的使能状态正好相反
** 4 modified by chenfj 2006/10/30 
**    onu带宽分配：
**      ONU注册时，若已分配带宽，使用已分配的带宽
**        反之，使用默认带宽
**      onu被删除时，收回已分配的带宽。对未注册过或未配置或被删除的ONU，不分配带宽
** 5 modified by chenfj 2006/11/15 
**	  #3202问题单: 下行方向不受限方式，带宽只能一次比一次小
** 6  added by chenfj 2006/12/12
**	     pon 端口下ONU最大距离设置: 1-- 20KM, 2--40KM, 3--60KM
** 7 added by chenfj 2006-12-21 
**	    设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参
** 8 modified by chenfj 2006-12-21
**    #3446 问题单:将PON板更换槽位后，删除以前PON口对应的ONU，不能在新的PON口上自动注册
** 9  added by chenfj 2007-02-07
**        增加ONU自动配置
** 10  added by chenfj 2007-4-24 
**      在显示PON 端口信息中，增加PON 芯片类型，及DBA算法版本等
** 11  added by chenfj 2007-6-19  增加PON 端口保护切换
**		设计规格:
**		1.       默认情况下，所有PON不具有保护倒换功能
**		2.       可通过网管任意指定两个PON口配对（可以选同一个PON板上的两个PON口，也可以选不同PON板上的PON口），作为干线保护端口，开始时pon_id号小的保护状态为active，pon_id号大的保护状态为passive，不发光
**		3.       自动检测，PON口自动倒换条件，在10秒内PON下ONU的注册数为0则自动倒换到另一个PON口，如此反复，直到有ONU注册上来为止。
**		4.       支持强制倒换，可通过网管发起强制倒换，强制倒换后检测定时器重新开始计时。
**		5.       管理属性一致，两个配对PON口属性及其下挂ONU属性保持一致（如：加密、带宽、ONU设备表、ONU认证表、PON性能监视等，但保护倒换状态除外），当一个端口的属性变化时会同时拷贝到另一个PON口。
**		6.       可通过网管取消两个配对的PON口，即取消保护倒换功能，取消时，保护倒换状态为active的PON口数据不变，但保护状态为passive的PON口将被重新初始化。
** 12 added by chenfj 2007-7-6
**      for onu authenticaion by PAS5001 
** 13 modified by chenfj 2007-7-19
**      问题单4912.在当前active的pon口号大的时候，在设置它和端口小的pon口保护，在线ONU会先离线，再注册
** 14 modified by chenfj 2007-9-20 
** 		问题单#5373: 设置PON 5/1和 5/2互为保护倒换。每重启一次PON板，就会引起一次保护倒换
** 15  added by chenfj 2008-8-29
**      增加PON 光功率检测
** 16  modified by chenfj 2008-7-9
*         增加GFA6100 产品支持; 
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif

#include "syscfg.h"

#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_io.h"
#include "vos/vospubh/vos_ctype.h"
#include "cli/cli.h"
#include "cli/cl_cmd.h"
#include "cli/cl_mod.h"
#include "addr_aux.h"
#include "linux/inetdevice.h"

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "includefromPas.h"
#include  "ponEventHandler.h"

#include "E1DataSave.h"
#include "../../bsp/epon3/GW405EP.h"
#include "../onu/Onu_manage.h"

#define  AUTO_PROTECT_STATUS_BASE  (NVRAM_START_ADRS + 0xd4)

#define MAX_QUEUE_SIZE_FOR_CONF_ONU 500

unsigned char PON_HOT_SWAP_WAIT_TIME= VOS_TICK_SECOND/2;

char *PendingReason_str[] = 
{
    "",
    "ONU-FULL",
    "CONFLICT",
    "SERVICE DISABLE",
    "StdOAM FAIL",
    "ExtOAM FAIL",
    "REG-INFO GET FAIL",
    "EQU GET FAIL",
    "PON PASSIVE",
    "AUTH-FAIL",
    "AUOR-FAIL",
    "BW SET FAIL",
    "BW REQUEST FAIL",    
    "Register TimeOut",
    "ONU-VER GET FAIL",
    "ONU-DENIED",
};

#ifdef __pending_onu_list__	
pendingOnu_S ConfOnuPendingQueue[MAX_QUEUE_SIZE_FOR_CONF_ONU];
#endif

int  all_online_onu=0;
int  all_offline_onu=0;
int  all_config_onu = 0;
unsigned long OnuPendingDataSemId = 0;

#if 0
typedef enum{
	ONU_NOT_EXIST,
	ONU_EXIST
}OnuStatus;
#endif

/* PON端口与交换芯片物理端口对应表*/
unsigned int *PonPortToSwPort;

/* PON端口与交换芯片逻辑端口对应表*/
unsigned int *PonPortToSwLogPort;



/*unsigned int  V2R1_TimeCounter = 0;*/
unsigned int V2R1_AutoProtect_Timer = 0;
unsigned int V2R1_AutoProtect_Trigger = 0;
unsigned int V2R1_AutoProtect_Interval = V2R1_PON_PORT_SWAP_INTERVAL;
extern ULONG g_ulRPCCall_mode;
extern int UpdatePendingQueueTime;

extern OLT_onu_table_t  olt_onu_global_table; /*added by wangjiah*/
/*extern STATUS  setOnuStatus( const ulong_t slot, const ulong_t port, const ulong_t onuindex, OnuStatus status );*/
extern STATUS	isHaveAuthMacAddress( ULONG slot, ULONG pon, const char* mac,  ULONG *onu );
extern LONG mn_getCtcOnuAuthMode( ULONG brdIdx, ULONG portIdx, ULONG *pMode );
extern STATUS	getOnuAuthEnable( ULONG slot, ULONG pon, ULONG *enable );
extern STATUS	getFirstOnuAuthEntry( ULONG *pulBrd, ULONG *pulPort, ULONG *pulUsr );
extern STATUS	getNextOnuAuthEntry( ULONG brd, ULONG port, ULONG usr, ULONG *nextbrd, ULONG *nextport, ULONG* nextusr );
extern STATUS 	getOnuAuthMacAddress( ULONG slot, ULONG pon, ULONG onu, CHAR *macbuf, ULONG *len );
extern STATUS 	setOnuAuthMacAddress( ULONG slot, ULONG pon, ULONG onu, CHAR *macbuf );
extern int  AddMacAuthentication( short int PonPortIdx,  mac_address_t mac );
extern STATUS 	setOnuAuthStatus( ULONG slot, ULONG pon, ULONG onu,  ULONG st );
#if 0
int PonPortSwapEnablePort1DisablePort2(short int PonPortIdx1, short int PonPortIdx2, int iProtectMode ) ;
#endif
int OnRegOnuCounter(short int PonPortIdx );
int  GetPonPortWorkStatus( short int PonPortIdx);
unsigned char CreateDownlinkVlanManipulation(short int PonPortIdx, Olt_llid_vlan_manipulation *vlan_Downlink_config);
unsigned char GetDownlinkVlanManipulation(short int PonPortIdx, Olt_llid_vlan_manipulation *vlan_Downlink_config);
void ReleaseDownlinkVlanManipulation(short int PonPortIdx, unsigned char vlan_Downlink_idx);

extern void (*physical_SendOam_hookrtn)(short int olt_id, unsigned char *buf, int size);
extern void (*physical_RecieveOam_hookrtn)(short int olt_id, unsigned char *buf, int size);


/* B--added by liwei056@2011-7-7 for 12EPON */
int GetPonPortLinkedSwPort( short int PonPortIdx )
{
    int iSwPortDrvId;

    OLT_LOCAL_ASSERT(PonPortIdx);
    
    if ( PRODUCT_IS_DISTRIBUTE )
    {
        if ( SYS_LOCAL_MODULE_ISHAVEPP() )
        {
            if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
            {
                int CardIdx = GetCardIdxByPonChip(PonPortIdx);
            
                /* 主控卡需判断PON板无PP,才知道PP在自己上 */
                /* PON板不存在，则认为直连的交换端口也不存在 */
                if ( (CardIdx != SYS_LOCAL_MODULE_SLOTNO) &&  /* added by liwei056@2012-2-27 for D17098 */
                      ((MODULE_TYPE_NULL == SYS_MODULE_TYPE(CardIdx))
                      || SYS_MODULE_SLOT_ISHAVEPP(CardIdx)) )
                {
                    iSwPortDrvId = -1;
                }
                else
                {
                    iSwPortDrvId = PonPortToSwPort[PonPortIdx];
                }
            }
            else
            {
                /* PON卡自带PP,直接返回其直连以太口 */
                iSwPortDrvId = PonPortToSwPort[PonPortIdx];
            }
        }
        else
        {
            iSwPortDrvId = -1;
        }
    }
    else
    {
        /* 集中式设备,直接返回PON口直连以太口 */
        iSwPortDrvId = PonPortToSwPort[PonPortIdx];
    }

    return iSwPortDrvId;
}

int DisablePonPortLinkedSwPort( short int PonPortIdx )
{
    int iRlt;
    int iSwPortDrvId;

    if ( 0 <= (iSwPortDrvId = GetPonPortLinkedSwPort(PonPortIdx)) )
    {
    	iRlt = bcm_port_enable_set( 0, iSwPortDrvId, 0 );
    }
    else
    {
        iRlt = iSwPortDrvId;
    }

    return iRlt;
}

int EnablePonPortLinkedSwPort( short int PonPortIdx )
{
    int iRlt;
    int iSwPortDrvId;

    if ( 0 <= (iSwPortDrvId = GetPonPortLinkedSwPort(PonPortIdx)) )
    {
    	iRlt = bcm_port_enable_set( 0, iSwPortDrvId, 1 );
    }
    else
    {
        iRlt = iSwPortDrvId;
    }

    return iRlt;
}

int RefreshPonPortLinkedSwPort( short int PonPortIdx )
{
    int iRlt;
    int iSwPortDrvId;

    if ( 0 <= (iSwPortDrvId = GetPonPortLinkedSwPort(PonPortIdx)) )
    {
    	iRlt = bcm_l2_addr_delete_by_port( 0, -1, iSwPortDrvId, 0 );
    }
    else
    {
        iRlt = iSwPortDrvId;
    }

    return iRlt;
}
/* E--added by liwei056@2011-7-7 for 12EPON */

int  PonPortIsWorking( short int PonPortIdx )
{
    int iWorkingStatus;

    if( GetPonPortAdminStatus( PonPortIdx) == V2R1_DISABLE ) return( FALSE );

    iWorkingStatus = GetPonPortWorkStatus(PonPortIdx);
    if( ( iWorkingStatus == PONPORT_UP )
        ||( iWorkingStatus == PONPORT_LOOP)
        || ( iWorkingStatus == PONPORT_UPDATE ) )
    	return( TRUE );

    return( FALSE );
}

/*****************************************************
 *
 *    Function: setUpdateParamToDufault( PON_olt_update_parameters_t updated_parameters )
 *
 *    Param:    PON_olt_update_parameters_t updated_parameters -- the olt system parameters
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
int setUpdateParamToDufault( PON_olt_update_parameters_t updated_parameters )
{
	updated_parameters.max_rtt = PON_VALUE_NOT_CHANGED;
	updated_parameters.address_table_aging_timer = PON_VALUE_NOT_CHANGED;
	updated_parameters.cni_port_maximum_entries = PON_VALUE_NOT_CHANGED;
	
	updated_parameters.arp_filtering_configuration.from_pon_to_firmware = PON_VALUE_NOT_CHANGED;
	updated_parameters.arp_filtering_configuration.from_cni_to_firmware = PON_VALUE_NOT_CHANGED;
	updated_parameters.arp_filtering_configuration.from_pon_to_cni = PON_VALUE_NOT_CHANGED;
	updated_parameters.arp_filtering_configuration.from_cni_to_pon = PON_VALUE_NOT_CHANGED;
	
	updated_parameters.igmp_configuration.enable_igmp_snooping = PON_VALUE_NOT_CHANGED;
	updated_parameters.igmp_configuration.igmp_timeout = PON_VALUE_NOT_CHANGED;
	updated_parameters.vlan_configuration.vlan_exchange_downlink_tag_prefix = PON_VALUE_NOT_CHANGED;
	updated_parameters.vlan_configuration.nested_mode_vlan_type = PON_VALUE_NOT_CHANGED;
	
	updated_parameters.hec_configuration.tx_hec_configuration = PON_VALUE_NOT_CHANGED;
	updated_parameters.hec_configuration.rx_hec_configuration = PON_VALUE_NOT_CHANGED;
	
	updated_parameters.vlan_tag_filtering_config.untagged_frames_filtering = PON_VALUE_NOT_CHANGED;
	updated_parameters.vlan_tag_filtering_config.multiple_copy_broadcast_enable_mode = PON_VALUE_NOT_CHANGED;
	updated_parameters.vlan_tag_filtering_config.filtering_unexpected_tagged_downstream_frames= PON_VALUE_NOT_CHANGED;
	updated_parameters.upstream_default_priority = PON_VALUE_NOT_CHANGED;
	updated_parameters.pon_tbc_polarity = PON_POLARITY_ACTIVE_LOW;

	return( ROK );
}

/*****************************************************
 *
 *    Function:  PonLLIDTableInit()
 *
 *    Param: none
 *
 *    Desc:   Initialize LLID table using default value
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
int InitPonLLIDTable()
{
#if 0
	int whichPon, whichLlid;
    ULONG ulSize;

#if 1
#ifdef g_malloc
#undef g_malloc
#endif

    VOS_ASSERT(NULL == PonLLIDTable);
    ulSize = sizeof(PonLLIDAttr_S) * MAXPON * MAXLLID;
    if ( NULL == (PonLLIDTable = g_malloc(ulSize)) )
    {
#ifdef __T_PON_MEM__        
        sys_console_printf("LlidTotalNum:%d, sizeof(PonLlidUnit)=%d, sizeof(PonLlidTbl)=%d, g_malloc Fail.\r\n", MAXPON * MAXLLID, sizeof(PonLLIDAttr_S), ulSize);
        VOS_TaskDelay(VOS_TICK_SECOND);
#endif
        VOS_ASSERT(0);
        return RERROR;
    }
#ifdef __T_PON_MEM__        
    else
    {
        sys_console_printf("LlidTotalNum:%d, sizeof(PonLlidUnit)=%d, sizeof(PonLlidTbl)=%d, g_malloc OK.\r\n", MAXPON * MAXLLID, sizeof(PonLLIDAttr_S), ulSize);
    }
#endif
#endif

	VOS_MemSet( (void *)PonLLIDTable, 0, ulSize );

	for( whichPon = 0; whichPon < MAXPON; whichPon ++ )
	{
		for( whichLlid = 0; whichLlid < MAXLLID; whichLlid++ )
		{
			/* VOS_MemSet( (char *)&(PonLLIDTable[whichPon][whichLlid].LLIDIfIndex),0,sizeof(PonLLIDAttr_S ) ); */
			PonLLIDTable[whichPon][whichLlid].LLID_id = BYTE_FF;
			PonLLIDTable[whichPon][whichLlid].LLIDEntryStatus = PONLLID_DOWN;
			PonLLIDTable[whichPon][whichLlid].LLIDUplinkBandwidth = DEFAULT_UP_BW;
			PonLLIDTable[whichPon][whichLlid].LLIDDownlinkBandwidth = DEFAULT_DOWN_BW;
			VOS_MemSet(&(PonLLIDTable[whichPon][whichLlid].ONUIndex), BYTE_FF, sizeof(IF_Index_S ) );
		}
	}
    
#endif
	return ( ROK );
}

int InitOnePonLLIDTable( short int  PonPortIdx )
{
#if 0
	short int whichLlid;
	
	CHECK_PON_RANGE
		
	for( whichLlid = 0; whichLlid< MAXLLID; whichLlid ++ )
    {
		VOS_MemSet( (char *)&(PonLLIDTable[PonPortIdx][whichLlid].LLIDIfIndex),0,sizeof(PonLLIDAttr_S ) );
		PonLLIDTable[PonPortIdx][whichLlid].LLID_id = 0xff;
		PonLLIDTable[PonPortIdx][whichLlid].LLIDEntryStatus = PONLLID_DOWN;
		PonLLIDTable[PonPortIdx][whichLlid].LLIDUplinkBandwidth = DEFAULT_UP_BW;
		PonLLIDTable[PonPortIdx][whichLlid].LLIDDownlinkBandwidth = DEFAULT_DOWN_BW;
		VOS_MemSet( &(PonLLIDTable[PonPortIdx][whichLlid].ONUIndex), BYTE_FF, sizeof( IF_Index_S ) );
	}
#endif
	return( ROK );
}

int ClearPonPortTable( short int PonPortIdx )
{
	CHECK_PON_RANGE

    VOS_MemSet((char *)&PonPortTable[PonPortIdx], 0x00, sizeof( PonPortMgmtInfo_S) );
    (void)OLT_SetupIFs(PonPortIdx, OLT_ADAPTER_NULL);   

	return( ROK );
}

int ClearPonPortTableAll()
{
	short int i;
    for( i=0; i<MAXPON;i++){ ClearPonPortTable( i ); }

	return( ROK );
}

int ClearConfOnuPendingQueue()
{
#ifdef __pending_onu_list__	
	int i;

	VOS_MemSet( ConfOnuPendingQueue, 0, MAX_QUEUE_SIZE_FOR_CONF_ONU * sizeof(pendingOnu_S ) );
	
	for( i = 0; i< MAX_QUEUE_SIZE_FOR_CONF_ONU; i++)
		{
		ConfOnuPendingQueue[i].otherPonIdx = MAXPON;
		ConfOnuPendingQueue[i].Llid = 0;
		ConfOnuPendingQueue[i].Next = NULL;
		}
#endif
	return( ROK );
}

int  FindFreeEntryForConfOnu()
{
#ifdef __pending_onu_list__	
	int i;

	for( i = 0; i< MAX_QUEUE_SIZE_FOR_CONF_ONU; i++)
		{
		if(( ConfOnuPendingQueue[i].Next == NULL ) && (ConfOnuPendingQueue[i].otherPonIdx == MAXPON )
			&& (ConfOnuPendingQueue[i].Llid == 0 ) )
			break;
		}

	if( i  < MAX_QUEUE_SIZE_FOR_CONF_ONU ) return i;
#endif
	return (RERROR );
}

#if 0
void ClearPonPortAddrTbl( short int PonPortIdx )
{
    OLT_LOCAL_ASSERT(PonPortIdx);

    PonPortTable[PonPortIdx].AddrTblNum = 0;

    return;
}

int FindPonPortMacAddr( short int PonPortIdx, unsigned char MacAddr[BYTES_IN_MAC_ADDRESS] )
{
    int i, n;
    unsigned char (*paMacAddr)[OLT_ADDR_BYTELEN];
    
    OLT_LOCAL_ASSERT(PonPortIdx);

    paMacAddr = PonPortTable[PonPortIdx].StaticMacAddr;
    n = PonPortTable[PonPortIdx].AddrTblNum;
    for (i=0; i<n; i++)
    {
        if ( 0 == VOS_MemCmp(paMacAddr[i], MacAddr, BYTES_IN_MAC_ADDRESS) )
        {
            return i;
        }
    }

    return -1;
}
#endif

void ClearGlobalPonAddrTbl()
{
    GlobalPonAddrTblNum = 0;

    return;
}

int FindGlobalPonMacAddr( unsigned char MacAddr[BYTES_IN_MAC_ADDRESS] )
{
    int i;

    for (i=0; i<GlobalPonAddrTblNum; i++)
    {
        if ( 0 == VOS_MemCmp(GlobalPonStaticMacAddr[i], MacAddr, BYTES_IN_MAC_ADDRESS) )
        {
            return i;
        }
    }

    return -1;
}

int PonStaticMacTableIsFull( unsigned char MacAddr[BYTES_IN_MAC_ADDRESS] )
{
    int iRlt;

    if ( 0 > (iRlt = FindGlobalPonMacAddr(MacAddr)) )
    {
        if ( GlobalPonAddrTblNum < MAXOLTADDRNUM )
        {
            iRlt = FALSE;
        }
        else
        {
            iRlt = TRUE;
        }
    }
    else
    {
        /* 已经存在的MAC地址，允许覆盖配置 */
        iRlt = FALSE;
    }

    return iRlt;
}

int InitPonPortConfigDefault()
{
	/* default config data init */
	PonPortConfigDefault.PortAdminStatus = PONPORT_ENABLE;
	PonPortConfigDefault.MaxRtt = PON_MAX_RTT_40KM; /*PAS_updated_parameters.max_rtt;*/
	PonPortConfigDefault.MaxOnu = MAXONUPERPON;
	PonPortConfigDefault.MaxOnu = 0;
	PonPortConfigDefault.DefaultOnuBW = ONU_DEFAULT_BW; /*7.5Mbps*/
	PonPortConfigDefault.DefaultXGOnuBW = GW10G_ONU_DEFAULT_BW; /*75Mbps*/
	PonPortConfigDefault.DefaultGPONOnuBW = GPON_ONU_DEFAULT_BW; /*15Mbps*/
	PonPortConfigDefault.MaxBW = DEFAULT_UP_BW;
	/*VOS_MemSet( PonPortConfigDefault.AlarmMask, 0, sizeof( PonPortConfigDefault.AlarmMask ) );*/
	PonPortConfigDefault.AlarmMask = 0;
	PonPortConfigDefault.MacSelfLearningCtrl = V2R1_ENABLE;
	PonPortConfigDefault.MACAgeingTime = DEFAULT_MAC_AGING_TIME;
	PonPortConfigDefault.ApsCtrl = PROTECT_SWITCH_CTRL_DISABLE;
	PonPortConfigDefault.BERThreshold_dis = PonPortConfigDefault.BERThreshold_gen = PON_MONITORING_BER_THRESHOLD_DEFAULT_VALUE;
	PonPortConfigDefault.FERThreshold_dis = PonPortConfigDefault.FERThreshold_gen = PON_MONITORING_FER_THRESHOLD_DEFAULT_VALUE;
	PonPortConfigDefault.DBA_mode = OLT_EXTERNAL_DBA;

	PonPortConfigDefault.testTimer = PONPORTTESTCYCLE;
	PonPortConfigDefault.testRemain = V2R1_PON_HOST_MSG_OUT;
	
	PonPortConfigDefault.AutoRegisterFlag = V2R1_DISABLE;
	/*PonPortConfigDefault.MaxMacNum = MaxMACDefault  MAXMACNUMBER*/;

	PonPortConfigDefault.EncryptType = EncryptTypeDefault;
	PonPortConfigDefault.EncryptEnable = EncryptEnableDefault;
	PonPortConfigDefault.EncryptDirerction = EncryptDirectionDefault;
	PonPortConfigDefault.EncryptKeyTime = EncryptKeyTimeDefault;

	/* added by chenfj 2006/12/12
	     PON MAX RANGE */
	PonPortConfigDefault.range = PON_RANGE_20KM;

	PonPortConfigDefault.InvalidOnuTablePtr = 0;

	PonPortConfigDefault.CTC_EncryptKeyUpdateTime = CTC_EncryptKeyTimeDefault;
	PonPortConfigDefault.CTC_EncryptKeyNoReplyTime = CTC_EncryptNoReplyTime;
	PonPortConfigDefault.CTC_EncryptTimeingThreshold = CTC_EncryptTimeingThreshold;
	
	InitPonAlarmConfigDefault(); 

	ClearConfOnuPendingQueue();
	
	return( ROK );

}

int InitPonPortTableByDefault(short int PonPortIdx)
{
	int CardSlot;
	unsigned int i;

	CHECK_PON_RANGE

	CardSlot = GetCardIdxByPonChip(PonPortIdx );

	if( CardSlot == RERROR ) return( RERROR );

	ClearPonPortTable(  PonPortIdx );

	/* running data table init */
	PonPortTable[PonPortIdx].IfIndex.type = PONPORT_TYPE;
	PonPortTable[PonPortIdx].IfIndex.slot = CardSlot;
	PonPortTable[PonPortIdx].IfIndex.port = PonPortIdx % PONPORTPERCARD;
	PonPortTable[PonPortIdx].IfIndex.subNo = 0xff;
	PonPortTable[PonPortIdx].IfIndex.Onu_FE = 0xf;
	
	PonPortTable[PonPortIdx].PortAdminStatus = PonPortConfigDefault.PortAdminStatus;
	PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_DOWN;
	PonPortTable[PonPortIdx].MaxOnu = PonPortConfigDefault.MaxOnu ;
    /* B--added by liwei056@2011-4-7 for GuestNeed */
	PonPortTable[PonPortIdx].DownlinkPoliceMode = downlinkBWlimit;
    /* E--added by liwei056@2011-4-7 for GuestNeed */
	PonPortTable[PonPortIdx].DefaultOnuBW = PonPortConfigDefault.DefaultOnuBW ;
	PonPortTable[PonPortIdx].MaxBW = PonPortConfigDefault.MaxBW;
	PonPortTable[PonPortIdx].ProvisionedBW = 0;
	PonPortTable[PonPortIdx].ActiveBW = 0;
	PonPortTable[PonPortIdx].RemainBW = PonPortTable[PonPortIdx].MaxBW;
	PonPortTable[PonPortIdx].DownlinkMaxBw = PonPortConfigDefault.MaxBW;
	PonPortTable[PonPortIdx].DownlinkRemainBw = PonPortTable[PonPortIdx].DownlinkMaxBw;
	PonPortTable[PonPortIdx].DownlinkProvisionedBW = 0;
	PonPortTable[PonPortIdx].DownlinkActiveBw = 0;
	PonPortTable[PonPortIdx].BWExceedFlag = FALSE;
	PonPortTable[PonPortIdx].DownlinkBWExceedFlag = FALSE;
	/*InitPonAlarmConfigDefault(); */
	VOS_MemCpy( &PonPortTable[PonPortIdx].AlarmConfigInfo, &PonDefaultAlarmConfiguration, sizeof( PON_olt_monitoring_configuration_t ) );

	PonPortTable[PonPortIdx].MacSelfLearningCtrl = PonPortConfigDefault.MacSelfLearningCtrl;
	PonPortTable[PonPortIdx].MACAgeingTime = PonPortConfigDefault.MACAgeingTime;
	PonPortTable[PonPortIdx].discard_unlearned_sa = FALSE;
	PonPortTable[PonPortIdx].discard_unlearned_da = PON_DIRECTION_NO_DIRECTION;
	PonPortTable[PonPortIdx].table_full_handle_mode = FALSE;
	PonPortTable[PonPortIdx].MaxLLID = PonPortTable[PonPortIdx].MaxOnu;

	PonPortTable[PonPortIdx].AutoRegisterFlag = PonPortConfigDefault.AutoRegisterFlag;
	/** LLID所能支持的最大MAC 数*/
	PonPortTable[PonPortIdx].MaxMacNum = MaxMACDefault /*PonPortConfigDefault.MaxMacNum*/;
#if 0
	for(i=0; i< PonPortTable[PonPortIdx].MaxLLID; i++)
		{
		PonPortTable[PonPortIdx].MaxMacPerLlid[i] = PonPortTable[PonPortIdx].MaxMacNum;
		}
#endif
	PonPortTable[PonPortIdx].PendingOnuCounter = 0;
	PonPortTable[PonPortIdx].PendingOnu.Llid = 0;
	PonPortTable[PonPortIdx].PendingOnu.Next = NULL;

	PonPortTable[PonPortIdx].PendingOnu_Conf.Llid = 0;
	PonPortTable[PonPortIdx].PendingOnu_Conf.otherPonIdx = MAXPON;
	PonPortTable[PonPortIdx].PendingOnu_Conf.Next = NULL;
	
	PonPortTable[PonPortIdx].MaxRTT = PonPortConfigDefault.MaxRtt; /*DEFAULT_RTT;*/
	PonPortTable[PonPortIdx].ApsStatus = PROTECT_SWITCH_STATUS_UNKNOWN;
	PonPortTable[PonPortIdx].ApsCtrl = PROTECT_SWITCH_CTRL_DISABLE;
	PonPortTable[PonPortIdx].DBA_mode = PonPortConfigDefault.DBA_mode;
	PonPortTable[PonPortIdx].external_DBA_used = TRUE;

	PonPortTable[PonPortIdx].testTimer = PonPortConfigDefault.testTimer;
	PonPortTable[PonPortIdx].testCounter = ( PonPortTable[PonPortIdx].testTimer / V2R1_TIMERTICK );
	PonPortTable[PonPortIdx].testRemain = PonPortConfigDefault.testRemain ;

	PonPortTable[PonPortIdx].EncryptType  = PonPortConfigDefault.EncryptType ;
	PonPortTable[PonPortIdx].EncryptEnable  = PonPortConfigDefault.EncryptEnable ;
	PonPortTable[PonPortIdx].EncryptDirection = PonPortConfigDefault.EncryptDirerction ;
	PonPortTable[PonPortIdx].EncryptKeyTime = PonPortConfigDefault.EncryptKeyTime;

	/* added by chenfj 2006/12/12
	     PON MAX RANGE */
	PonPortTable[PonPortIdx].range = PonPortConfigDefault.range;

	PonPortTable[PonPortIdx].OnuAuthMode = FALSE;
	PonPortTable[PonPortIdx].OnuRegisterMsgCounter = 0;
	PonPortTable[PonPortIdx].OnuDeregisterMsgCounter =0;
	VOS_MemSet(&(PonPortTable[PonPortIdx].LastOnuRegisterTime.year),0, sizeof(Date_S));
	VOS_MemSet(&(PonPortTable[PonPortIdx].LastRegisterMacAddr[0]), 0xff, BYTES_IN_MAC_ADDRESS );	

	PonPortTable[PonPortIdx].BerFlag = V2R1_TRAP_NOT_SEND;
	PonPortTable[PonPortIdx].FerFlag = V2R1_TRAP_NOT_SEND;	

	PonPortTable[PonPortIdx].TooManyOnuRegisterLastAlarmTime = time(0);
	PonPortTable[PonPortIdx].LastBerAlarmTime = 0;
	PonPortTable[PonPortIdx].LastFerAlarmTime = 0;

	PonPortTable[PonPortIdx].CTC_EncryptKeyUpdateTime = PonPortConfigDefault.CTC_EncryptKeyUpdateTime;
	PonPortTable[PonPortIdx].CTC_EncryptKeyNoReplyTime = PonPortConfigDefault.CTC_EncryptKeyNoReplyTime;
	PonPortTable[PonPortIdx].CTC_EncryptTimeingThreshold = PonPortConfigDefault.CTC_EncryptTimeingThreshold;

	/* added by chenfj 2007-6-19  增加PON 端口保护切换*/
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
	PonPortTable[PonPortIdx].swap_flag = V2R1_PON_PORT_SWAP_DISABLE;
	PonPortTable[PonPortIdx].swap_slot = 0;
	PonPortTable[PonPortIdx].swap_port = 0;
	PonPortTable[PonPortIdx].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
	PonPortTable[PonPortIdx].swap_timer = 0;
	PonPortTable[PonPortIdx].swapping = V2R1_SWAPPING_NO;
	PonPortTable[PonPortIdx].swapHappened = FALSE;
	PonPortTable[PonPortIdx].swap_monitor = 0;
	PonPortTable[PonPortIdx].swap_mode = V2R1_PON_PORT_SWAP_DISABLED;
	PonPortTable[PonPortIdx].swap_reason = 0;
	PonPortTable[PonPortIdx].swap_status = 0;
	PonPortTable[PonPortIdx].swap_times = 0;
	PonPortTable[PonPortIdx].swap_triggers = V2R1_PON_PORT_SWAP_TRIGGER;
	PonPortTable[PonPortIdx].swap_interval = 0;
	PonPortTable[PonPortIdx].protect_slot  = 0;
	PonPortTable[PonPortIdx].protect_port  = 0;
#endif

#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
    /* B--Modified by liwei056@2010-5-25 for D10168 */
#if 0
	for(i=0; i< MAX_VID_DOWNLINK; i++)
		PonPortTable[PonPortIdx].Downlink_vlan_manipulation[i].vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION;
#else
    PonPortTable[PonPortIdx].Downlink_vlan_manipulation[0].vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION;
    PonPortTable[PonPortIdx].Downlink_vlan_manipulation[0].original_vlan_id = 1;
    PonPortTable[PonPortIdx].Downlink_vlan_manipulation[0].new_priority = PON_VLAN_ORIGINAL_PRIORITY;
    PonPortTable[PonPortIdx].Downlink_vlan_manipulation[1].vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG;
    PonPortTable[PonPortIdx].Downlink_vlan_manipulation[1].original_vlan_id = 1;
    PonPortTable[PonPortIdx].Downlink_vlan_manipulation[1].new_priority = PON_VLAN_ORIGINAL_PRIORITY;
    PonPortTable[PonPortIdx].Downlink_vlan_Man_Max   = 1;
    PonPortTable[PonPortIdx].Downlink_vlan_Cfg_Max   = 0;
#endif
    /* E--Modified by liwei056@2010-5-25 for D10168 */
	PonPortTable[PonPortIdx].vlan_tpid = DEFAULT_PON_VLAN_TPID;
#endif

	PonPortTable[PonPortIdx].firmwareMismatchAlarm = V2R1_DISABLE;
	PonPortTable[PonPortIdx].DBAMismatchAlarm = V2R1_DISABLE;
	/* added by chenfj 2009-5-14 */
	PonPortTable[PonPortIdx].PortFullAlarm = V2R1_DISABLE;
	
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )

	/* added by xieshl 20080823, 解决MIB中默认值错误问题 */
	/*PonPortTable[PonPortIdx].recvOpticalPower = 0;	*/	/*接收光功率*/
/*	PonPortTable[PonPortIdx].powerMeteringSupport = V2R1_DISABLE;
	PonPortTable[PonPortIdx].SFPTypeMismatchAlarm = V2R1_DISABLE;
	PonPortTable[PonPortIdx].transOpticalPower = PON_OPTICAL_POWER_TRANSMIT_DEFAULT;*/		/*发送光功率*/
/*	PonPortTable[PonPortIdx].ponTemperature = 50;	*/	/*模块温度*/
/*	PonPortTable[PonPortIdx].ponVoltageApplied = 30;*/	/*模块电压*/
/*	PonPortTable[PonPortIdx].ponBiasCurrent = 20;	*/	/*偏置电流*/
	
/*	PonPortTable[PonPortIdx].OpticalPowerAlarm =0;*/
	/*PonPortTable[PonPortIdx].OpticalPowerAlarm_gen = 0;
	PonPortTable[PonPortIdx].OpticalPowerAlarm_clr = 0;*/
#endif

/*modified by wangjiah@2017-05-03 
 * For updating 10gepon pon port info.
 * This modification will trigger pon los clear when onu register on a new pon port
 * on which onu has never registered.
 * */
#if 1
	if( SYS_LOCAL_MODULE_TYPE_IS_8000_8EPON ) 
	{
		PonPortTable[PonPortIdx].SignalLossFlag = V2R1_ENABLE;
	}
	else
#endif
/*modified by wangjiah@2017-05-03  end*/
	{
		PonPortTable[PonPortIdx].SignalLossFlag = V2R1_DISABLE;
	}
	PonPortTable[PonPortIdx].TxCtrlFlag     = 0;
	PonPortTable[PonPortIdx].AbnormalFlag   = V2R1_DISABLE;
#if 0
	PonPortTable[PonPortIdx].ponAlwaysOnEnable = 2;	/*允许PON端口突发模式监视*/
	PonPortTable[PonPortIdx].ponSignalMonEnable = 2;	/*光功率监视使能*/
	PonPortTable[PonPortIdx].ponSignalMonInterval = 60;	/*监视周期*/
#endif
	return( ROK );
}

int InitOltMgmtTableByDefault()
{
    int i;
    ULONG ulSize;

#if 1
/*
#ifdef g_malloc
#undef g_malloc
#endif
*/
    VOS_ASSERT(NULL == PonPortTable);
    ulSize = sizeof(PonPortMgmtInfo_S) * MAXPON;
    if ( NULL == (PonPortTable = (PonPortMgmtInfo_S*)g_malloc(ulSize)) )
    {
#ifdef __T_PON_MEM__        
        sys_console_printf("PonTotalNum:%d, sizeof(PonMgtUnit)=%d, sizeof(PonMgtTbl)=%d, g_malloc Failed.\r\n", MAXPON, sizeof(PonPortMgmtInfo_S), ulSize);
        VOS_TaskDelay(VOS_TICK_SECOND);
#endif
        VOS_ASSERT(0);
        return RERROR;
    }
#ifdef __T_PON_MEM__        
    else
    {
        sys_console_printf("PonTotalNum:%d, sizeof(PonMgtUnit)=%d, sizeof(PonMgtTbl)=%d, g_malloc OK.\r\n", MAXPON, sizeof(PonPortMgmtInfo_S), ulSize);
    }
#endif
#endif

    ClearGlobalPonAddrTbl();
    InitPonPortConfigDefault();
    for( i=0; i<MAXPON; i++ )
    {
        InitPonPortTableByDefault( i );
    }

    return( ROK );
}

int GetPonLastAlarmTime(short int PonPortIdx )
{
	CHECK_PON_RANGE

	return( PonPortTable[PonPortIdx].TooManyOnuRegisterLastAlarmTime );
}

int ClearPonPortRunningData( short int PonPortIdx )
{
    int OnuIdx;
	short int PonPortIdx_Swap = 0;
	CHECK_PON_RANGE

    /* B--added by liwei056@2011-1-11 for CodeCheck */
    if ( OLT_ADAPTER_RPC < OLT_GetIFType(PonPortIdx) )
    {
        pon_lose_olt(PonPortIdx);
        OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) release the PON-Chip(%d)'s LOCAL-PON service at the ClearPonPortRunningData().\r\n", SYS_LOCAL_MODULE_SLOTNO, PonPortIdx );
    }
    /* E--added by liwei056@2011-1-11 for CodeCheck */

	PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_DOWN;
	PonPortTable[PonPortIdx].CurrOnu = 0;
	VOS_MemSet( PonPortTable[PonPortIdx].OnuCurStatus, 0, sizeof( PonPortTable[PonPortIdx].OnuCurStatus ));
	PonPortTable[PonPortIdx].ActiveBW = 0;
	/*PonPortTable[PonPortIdx].RemainBW = PonPortTable[PonPortIdx].MaxBW;*/
	/*PonPortTable[PonPortIdx].ProvisionedBW = 0;*/
	PonPortTable[PonPortIdx].DownlinkActiveBw = 0;
	/*PonPortTable[PonPortIdx].DownlinkRemainBw = PonPortTable[PonPortIdx].DownlinkMaxBw;*/
	/*PonPortTable[PonPortIdx].DownlinkProvisionedBW = 0; */
	PonPortTable[PonPortIdx].BWExceedFlag =0;
	PonPortTable[PonPortIdx].DownlinkBWExceedFlag = 0;
	/*VOS_MemSet( PonPortTable[PonPortIdx].AlarmStatus, 0, sizeof( PonPortTable[PonPortIdx].AlarmStatus ));*/	/* removed 20070705 */
	PonPortTable[PonPortIdx].AlarmStatus = 0;
	
	PonPortTable[PonPortIdx].CurLLID = 0;
	PonPortTable[PonPortIdx].CurrRTT = 0;
	PonPortTable[PonPortIdx].ApsStatus = PROTECT_SWITCH_STATUS_UNKNOWN;
	/* PonPortTable[PonPortIdx].BERThreshold = 0; */
	/* PonPortTable[PonPortIdx].FERThreshold = 0; */
	PonPortTable[PonPortIdx].PortSpeed = 0;
	PonPortTable[PonPortIdx].PortBER = 0;
	PonPortTable[PonPortIdx].PortPER = 0;	

	PonPortTable[PonPortIdx].type = PONPORTTYPEUNKNOWN;
#ifdef 	STATISTIC_TASK_MODULE
	StatsMsgPonOnSend( PonPortIdx, PON_OFFLINE );
#endif
	if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		SetPonNotRunningFlag(PonPortIdx );
	}

	PonPortTable[PonPortIdx].OnuRegisterMsgCounter = 0;
	PonPortTable[PonPortIdx].OnuDeregisterMsgCounter =0;
	VOS_MemSet(&(PonPortTable[PonPortIdx].LastOnuRegisterTime.year),0, sizeof(Date_S));
	VOS_MemSet(&(PonPortTable[PonPortIdx].LastRegisterMacAddr[0]),0xff, BYTES_IN_MAC_ADDRESS );

	PonPortTable[PonPortIdx].BerFlag = V2R1_TRAP_NOT_SEND;
	PonPortTable[PonPortIdx].FerFlag = V2R1_TRAP_NOT_SEND;

	PonPortTable[PonPortIdx].TooManyOnuRegisterLastAlarmTime = time(0);
	PonPortTable[PonPortIdx].LastBerAlarmTime = 0;
	PonPortTable[PonPortIdx].LastFerAlarmTime = 0;

	UpdatePonPortNum();
	UpdateAllPendingOnu(PonPortIdx );
	UpdateAllPendingConfOnu(PonPortIdx );
	/*DelAllPendingOnu_conf(PonPortIdx);*/

	/*removed by wangjiah@2016-09-22 to solve issue:32970*/
#if 0
	if( SYS_LOCAL_MODULE_TYPE_IS_8000_8EPON )
	{
	PonPortTable[PonPortIdx].SignalLossFlag = V2R1_ENABLE;	/* modified by xieshl 20160226, 8XEP需软件点灯，默认应置成LOS */
	sys_console_printf("\r\n\r\n==>clear:PonPortTable[%d].SignalLossFlag=%d\r\n\r\n", PonPortIdx, PonPortTable[PonPortIdx].SignalLossFlag);
	}
	else
#endif 
	#if 0/*35196,pon los告警标志位不清空 by jinhl@2017.03.29*/
	PonPortTable[PonPortIdx].SignalLossFlag = V2R1_DISABLE;
	#endif
	/*PonPortTable[PonPortIdx].TxCtrlFlag     = 0;del by yangzl@2016-2-29*/
	PonPortTable[PonPortIdx].AbnormalFlag   = V2R1_DISABLE;

	//removed by wangjiah@2017-05-26. 
	//Don't clear swap_use flag when reset pon or reboot pon borard
	//because it'll be used to determine ACTIVE/PASSIVE pon when active pon 
	//PonPortTable[PonPortIdx].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN; 
	
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
	ClearOpticalPowerAlarmWhenPonPortDown(PonPortIdx);
    for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		ClearOpticalPowerAlamWhenOnuOffline( PonPortIdx, OnuIdx );
	}
	
    ClearOnuLaserAlwaysOnAlarmsWhenPon_Loss( PonPortIdx );
    
	PonPortTable[PonPortIdx].PonPortmeteringInfo.powerMeteringSupport = V2R1_DISABLE;
	PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm = V2R1_DISABLE;
	PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter = 0;
	PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower = PON_OPTICAL_POWER_TRANSMIT_DEFAULT;		/*发送光功率*/
	PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature = 50;		/*模块温度*/
	PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied = 30;	/*模块电压*/
	PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent = 20;		/*偏置电流*/
	
/*	ClearOpticalPowerAlarmWhenPortDown(PonPortIdx);
	PonPortTable[PonPortIdx].OpticalPowerAlarm =0;*/
	/*PonPortTable[PonPortIdx].OpticalPowerAlarm_gen = 0;
	PonPortTable[PonPortIdx].OpticalPowerAlarm_clr = 0;*/
#endif		

	return (ROK );
}

int DecreasePonPortRunningData( short int PonPortIdx, short int OnuIdx )
{
	short int i,j;
	short int OnuEntry;
	short int status;
	
	CHECK_ONU_RANGE;

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	/* modified by xieshl 20111102, up/pending/dormant都应处理 */
	status = OnuMgmtTable[OnuEntry].OperStatus;

	/*if( GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP )*/
	if( (status == ONU_OPER_STATUS_UP) || (status == ONU_OPER_STATUS_PENDING) || (status == ONU_OPER_STATUS_DORMANT) )
	{
		if( PonPortTable[PonPortIdx].CurrOnu ) PonPortTable[PonPortIdx].CurrOnu --;
		if( PonPortTable[PonPortIdx].CurLLID ) PonPortTable[PonPortIdx].CurLLID -- ;

		i = OnuIdx & 0x07; /*OnuIdx % 8*/
		j = OnuIdx >> 3;   /*OnuIdx / 8*/
		PonPortTable[PonPortIdx].OnuCurStatus[j] = PonPortTable[PonPortIdx].OnuCurStatus[j] & ( BYTE_FF - ( 1 << i ) );
	}
#if 0	
	ONU_MGMT_SEM_TAKE;
	/* PonPortTable[PonPortIdx].RemainBW += OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_gr; */
	PonPortTable[PonPortIdx].ActiveBW -=OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_gr;
#ifdef PALTO_DBA_V3
	PonPortTable[PonPortIdx].ActiveBW -=OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_fixed;
#endif
	/* PonPortTable[PonPortIdx].DownlinkRemainBw += OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkBandwidth_gr; */
	PonPortTable[PonPortIdx].DownlinkActiveBw -= OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkBandwidth_gr;
	ONU_MGMT_SEM_GIVE;
#endif

	if(  PonPortTable[PonPortIdx].BWExceedFlag == TRUE )
    {
		PonPortTable[PonPortIdx].BWExceedFlag = FALSE;
		/* send trap to NMS */
		/* if( PonPortTable[PonPortIdx].AlarmMask[OnuIdx/8] == 0){ 
			SendTrapToNMS( PonPortIdx, OnuIdx, TripId ); 
			}
		*/
	}
	if(  PonPortTable[PonPortIdx].DownlinkBWExceedFlag == TRUE )
    {
		PonPortTable[PonPortIdx].DownlinkBWExceedFlag = FALSE;
		/* send trap to NMS */
		/*
		if( PonPortTable[PonPortIdx].AlarmMask[OnuIdx/8] == 0){
			 SendTrapToNMS( PonPortIdx, OnuIdx, TripId ); 
			}
		*/
	}

	UpdatePonPortCurrRTT( PonPortIdx, OnuIdx );
	return( ROK );
}

int UpdatePonPortCurrRTT( short int  PonPortIdx, short int OnuIdx )
{
	short int i;
	short int onuEntry;
	short int onuEntryBase;
	
	CHECK_ONU_RANGE;

	/*if( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].RTT == PonPortTable[PonPortIdx].CurrRTT )*/
	{
	    onuEntryBase = PonPortIdx * MAXONUPERPON;
		PonPortTable[PonPortIdx].CurrRTT = 0;
		ONU_MGMT_SEM_TAKE;
		for( i=0; i< MAXONUPERPON; i ++ )
		{
			if ( i != OnuIdx )
			{
				onuEntry = onuEntryBase + i;
				/*if( GetOnuOperStatus( PonPortIdx, i) != ONU_OPER_STATUS_UP ) continue;*/
				if( ONU_OPER_STATUS_UP != OnuMgmtTable[onuEntry].OperStatus ) continue;
				if( OnuMgmtTable[onuEntry].RTT > PonPortTable[PonPortIdx].CurrRTT )
					PonPortTable[PonPortIdx].CurrRTT = OnuMgmtTable[onuEntry].RTT;
			}
		}
		ONU_MGMT_SEM_GIVE;
	}
	return( ROK );
}

int InitPonPortTableByFlash(short int PonPortIdx)
{
#if 0
	int i;
	MacTable_S *MacPtr, *NextPtr;
	
#endif	
	CHECK_PON_RANGE

	ClearPonPortTable( PonPortIdx );
#if 0
	readPonPortDataFromFlash( PonPortIdx );
	PonPortTable[PonPortIdx].DBA_mode = 
	if( PonPortTable[PonPortIdx].DBA_mode == OLT_EXTERNAL_DBA ){
		PonPortTable[PonPortIdx].external_DBA_used = TRUE;
		}
	else { PonPortTable[PonPortIdx].external_DBA_used = FALSE; }
	
	PonPortTable[PonPortIdx].InvalidOnuNum = 
	for( i=0; i< PonPortTable[PonPortIdx].InvalidOnuNum; i++ ){
		MacPtr = ( MacTable_S *)VOS_Malloc( sizeof(MacTable_S ), MODULE_PON );
		if( MacPtr == NULL ){
			sys_console_printf("error: the return pointer of g_malloc is NULL ( InitPonPortTableByFlash())\r\n" );
			/* sendTripToNMS(PonPortIdx, NULL, TripId ); */
			return( RERROR );
			}
		VOS_MemCpy( MacPtr->MAC,  xx,  BYTES_IN_MAC_ADDRESS );
		if( PonPortTable[PonPortIdx].InvalidOnuTablePtr == NULL ){
			PonPortTable[PonPortIdx].InvalidOnuTablePtr = MacPtr;
			NextPtr = MacPtr;
			}
		NextPtr->nextMac = ( int ) MacPtr;
		}
	NextPtr->nextMac = 0;

	PonPortTable[PonPortIdx].ApsCtrl = 
	PonPortTable[PonPortIdx].MaxRTT = 
	PonPortTable[PonPortIdx].MaxOnu = 
	PonPortTable[PonPortIdx].MaxLLID = 
	PonPortTable[PonPortIdx].MACAgeingTime = 
	PonPortTable[PonPortIdx].AlarmMask = 
	PonPortTable[PonPortIdx].MaxBW = 
	PonPortTable[PonPortIdx].DownlinkMaxBw = 
	PonPortTable[PonPortIdx].DefaultOnuBW =
	PonPortTable[PonPortIdx].PortAdminStatus = 
	
		
		
#endif		
		

	return( ROK );
}

/*****************************************************
 *
 *    Function: SetPonPortType( unsigned short int PonPortIdx, unsigned int type  )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  unsigned int type -- the pon type 
 *    Desc:   
 *
 *    Return:    
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int SetPonPortType( short int PonPortIdx, unsigned int type )
{

	CHECK_PON_RANGE
	/*
	result = GetPonPortOperStatus(  PonPortIdx );	
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR ))return( RERROR );
	*/
	if( (type >= PONPORTTYPEMAX) ||( type < PONPORTTYPEUNKNOWN)) return ( RERROR );
	
	PonPortTable[PonPortIdx].type = type;	
	return( ROK );
}

int GetPonPortType(short int PonPortIdx )
{
	unsigned int type ;
	
	CHECK_PON_RANGE
	/*
	result = GetPonPortOperStatus(  PonPortIdx );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR ))return( RERROR );
	*/

	type = PonPortTable[PonPortIdx].type ;
	if( (type >= PONPORTTYPEMAX) ||( type < PONPORTTYPEUNKNOWN )) return ( RERROR );
	else return( type );
}

int UpdateXGEPonPortInfoByType(short int PonPortIdx, int iXfpType)
{
	CHECK_PON_RANGE
	if(iXfpType > XFP_TYPE_ASYM_10_1 || iXfpType <= XFP_TYPE_UNKNOWN)
	{
		SetPonPortType(PonPortIdx, PONPORTTYPEUNKNOWN);
		PonPortTable[PonPortIdx].MaxBW = GW10G_DEFAULT_UP_BW;
		PonPortTable[PonPortIdx].DownlinkMaxBw = GW10G_DEFAULT_DOWN_BW;
	}
	else if(XFP_TYPE_SYM_10_10 == iXfpType)
	{
		SetPonPortType(PonPortIdx, EPONMAUTYPE10GBASEPR30DOLT);
		PonPortTable[PonPortIdx].MaxBW = GW10G_DEFAULT_UP_BW;
		PonPortTable[PonPortIdx].DownlinkMaxBw = GW10G_DEFAULT_DOWN_BW;
	}
	else if(XFP_TYPE_ASYM_10_1 == iXfpType)
	{
		SetPonPortType(PonPortIdx, EPONMAUTYPE10GBASEPRX30DOLT);
		PonPortTable[PonPortIdx].MaxBW = DEFAULT_UP_BW;
		PonPortTable[PonPortIdx].DownlinkMaxBw = GW10G_DEFAULT_DOWN_BW;
	}
	return ROK;
}

int UpdateXGEPonPortInfo(short int PonPortIdx)
{
	int iRet = ROK;
	int iXfpType = XFP_TYPE_UNKNOWN;

	CHECK_PON_RANGE

	iRet = ponSfp_getXfpType(PonPortIdx, &iXfpType);
	if(ROK != iRet || XFP_TYPE_UNKNOWN == iXfpType)
	{
		SetPonPortType(PonPortIdx, PONPORTTYPEUNKNOWN);
		PonPortTable[PonPortIdx].MaxBW = GW10G_DEFAULT_UP_BW;
		PonPortTable[PonPortIdx].DownlinkMaxBw = GW10G_DEFAULT_DOWN_BW;
	}
	else 
	{
		iRet = UpdateXGEPonPortInfoByType(PonPortIdx, iXfpType);
	}
	return ROK;
}

/*****************************************************
 *
 *    Function:  GetPonLlidEntryStatus(unsigned short int PonPortIdx, unsigned char Llid)
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  short int Llid -- the specific pon llid
 *
 *    Desc:   
 *
 *    Return:   LLID status
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
int GetPonLlidEntryStatus( short int PonPortIdx,  short int  Llid)
{
	CHECK_LLID_RANGE;
#if 0
	return ( PonLLIDTable[PonPortIdx][Llid].LLIDEntryStatus );
#else
    return 0;
#endif
}

/*****************************************************
 *
 *    Function:  SetPonPortMacAutoLearningCtrl( short int PonPortIdx,  unsigned int CtrlValue )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  unsigned char Llid -- the specific pon llid
 *                  unsigned int CtrlValue -- the ctrl value ,enable/disable 
 *
 *    Desc:   
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
int  SetPonPortMacAutoLearningCtrl( short int PonPortIdx,  unsigned int CtrlValue )
{
    #if 1/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	OLT_addr_table_cfg_t addrtbl_cfg;
	#else
	PON_address_table_config_t   address_table_config;
	#endif
	short int ret;
	CHECK_PON_RANGE

	SWAP_TO_ACTIVE_PON_PORT_INDEX(PonPortIdx)
		
	if(( CtrlValue != V2R1_ENABLE ) && ( CtrlValue != V2R1_DISABLE)) return ( RERROR );

	PonPortTable[PonPortIdx].MacSelfLearningCtrl = CtrlValue;
	if( PonPortIsWorking(PonPortIdx)  != TRUE ) return( RERROR );

    #if 1
	VOS_MemZero( &addrtbl_cfg, sizeof(OLT_addr_table_cfg_t));
	ret = OLT_GetAddressTableConfig( PonPortIdx, &addrtbl_cfg );
	if( ret != PAS_EXIT_OK ) return( RERROR );

	if( CtrlValue == V2R1_ENABLE ) 
		addrtbl_cfg.allow_learning = PON_DIRECTION_UPLINK;
	else addrtbl_cfg.allow_learning = PON_DIRECTION_NO_DIRECTION;

	ret =  OLT_SetAddressTableConfig(PonPortIdx, &addrtbl_cfg );
	#else
	ret = PAS_get_address_table_configuration( PonPortIdx, &address_table_config );
	if( ret != PAS_EXIT_OK ) return( RERROR );

	if( CtrlValue == V2R1_ENABLE ) 
		address_table_config.allow_learning = PON_DIRECTION_UPLINK;
	else address_table_config.allow_learning = PON_DIRECTION_NO_DIRECTION;
    #endif
	/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	return ( ROK );
}

int  GetPonPortMacAutoLearningCtrl( short int PonPortIdx,  unsigned int *CtrlValue )
{
	CHECK_PON_RANGE

	if( CtrlValue == NULL ) return( RERROR );

	*CtrlValue = PonPortTable[PonPortIdx].MacSelfLearningCtrl ;
	return( ROK );
}

int  GetPonPortMacAutoLearningCtrDefaultl( unsigned int *CtrlValue )
{
	if( CtrlValue == NULL ) return( RERROR );

	*CtrlValue = PonPortConfigDefault.MacSelfLearningCtrl ;
	return( ROK );
}

int  SetPonPortMacAgeingTime( short int PonPortIdx,  unsigned int AgeingTime )
{
	/*short int PonChipType;
	short int PonChipVer = RERROR;
	short int ret;*/
	
	CHECK_PON_RANGE

#if 1
    if ( OLT_CALL_ISOK(OLT_SetMacAgingTime(PonPortIdx, AgeingTime)) )
    {
        return ROK;
    }
#else
	
	/*if(( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UP )||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_LOOP ) ||(PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UPDATE))*/
/*
	modified by chenfj 2008-5-20
	问题单6714: PON口更改MAC老化时间后，无法保存
	由于在系统启动,恢复PON MAC地址老化时间时,PON口还没有被激活,所以导致配置数据没有被恢复;
修改判断条件,在恢复数据时,只要PON口可以管理(固件和DBA文件都被正确加载),就恢复配置数据;
*/
	if(Olt_exists(PonPortIdx))
	/*if( PonPortIsWorking( PonPortIdx ) == TRUE )*/
		{
		PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
		
		if( PonChipType == PONCHIP_PAS )
			{
			/* modified for PAS-SOFT V5.1 
			PonChipVer = V2R1_GetPonChipVersion(PonPortIdx); */
			if((PonChipVer != PONCHIP_PAS5001 ) && ( PonChipVer != PONCHIP_PAS5201 ) ) return ( RERROR );
			
			if( PonChipVer == PONCHIP_PAS5001 )
				{
				PON_olt_update_parameters_t  pon_updated_parameters;	

				PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT(pon_updated_parameters);
				/*pon_updated_parameters.address_table_aging_timer = PonPortTable[PonPortIdx].MACAgeingTime;*/
				pon_updated_parameters.address_table_aging_timer = AgeingTime;
				/*PAS_updated_parameters.address_table_aging_timer = AgeingTime;*/
		
				if( PAS_update_olt_parameters_v4(PonPortIdx, &pon_updated_parameters ) == PAS_EXIT_OK )
					{
					PonPortTable[PonPortIdx].MACAgeingTime = AgeingTime;
					/* add by chenfj 2006-11-28 
				   	  for seting pas6201 aging timer 
					for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
					SetPas6201AgingTime(PonPortIdx, OnuIdx, (AgeingTime /SECOND_1));
					*/
					return( ROK );
					}
				else {
					return( RERROR );
					}
				}
			else if( PonChipVer == PONCHIP_PAS5201 )
				{
				PON_address_table_config_t  add_table_config;
				
				ret = PAS_get_address_table_configuration(PonPortIdx,&add_table_config);
				if( ret != PAS_EXIT_OK ) return( RERROR );
				add_table_config.aging_timer = AgeingTime;
				ret = PAS_set_address_table_configuration(PonPortIdx, add_table_config);
				if( ret == PAS_EXIT_OK )
					{
					PonPortTable[PonPortIdx].MACAgeingTime = AgeingTime;
					return( ROK );
					}
				else return( RERROR );
				}
			
			}
		
		else{/*其他PON芯片类型处理*/
			return( RERROR );
			}
		}
#endif
	return ( RERROR );
}

int  GetPonPortMacAgeingTime( short int PonPortIdx,  unsigned int *AgeingTime )
{
	CHECK_PON_RANGE

	if( AgeingTime == NULL ) return( RERROR );

	*AgeingTime = PonPortTable[PonPortIdx].MACAgeingTime;
	
	return( ROK );
}

int  GetPonPortMacAgeingTimeDefault( unsigned int *AgeingTime )
{
	if( AgeingTime == NULL ) return( RERROR );

	*AgeingTime = PonPortConfigDefault.MACAgeingTime;
	
	return( ROK );
}

/*** pon port routine started ***/

/*****************************************************
 *
 *    Function:  SetPonPortAdminStatus( unsigned short int PonPortIdx, unsigned char AdminStatus)
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  unsigned char AdminStatus -- the admin status to be set 
 *
 *    Desc:   
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 

int  SetPonPortAdminStatus( short int PonPortIdx, unsigned int AdminStatus)
{
	/*
	int PonCardIdx;
	int cardInserted;
	int PonChipStatus;
	*/
	CHECK_PON_RANGE

	/*
	PonCardIdx = GetCardIdxByPonChip( PonPortIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );
	
	PonChipStatus = GetPonchipWorkingStatus( PonPortIdx );
	if( PonChipStatus == RERROR ) return ( RERROR );

	
	if( PonChipStatus > PONCHIP_TESTING )
		
		sys_console_printf(" the PON chip current status is %s  ( SetPonPortAdminStatus() )\r\n", PONchipStatus[PonChipStatus] );
		return ( RERROR );	
		} 
	*/
	if(( AdminStatus != PONPORT_ENABLE ) && ( AdminStatus != PONPORT_DISABLE ))
		{
		sys_console_printf(" the %s/port%d Admin status %d to be set  is not correct\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), AdminStatus );
		return ( RERROR );
		}

	if( PonPortTable[PonPortIdx].PortAdminStatus == AdminStatus )
		{
		return ( ROK );
		}
		
	if( AdminStatus == PONPORT_ENABLE )
		return(PonPortAdminStatusEnable( PonPortIdx ));
	else /*if( AdminStatus == PONPORT_DISABLE )*/
		return( PonPortAdminStatusDisable( PonPortIdx ));
	return( ROK );
}

/*****************************************************
 *
 *    Function:   GetPonPortAdminStatus( unsigned short int PonPortIdx)
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *
 *    Desc:   
 *
 *    Return:   the pon port current admin status 
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 
int  GetPonPortAdminStatus( short int PonPortIdx)
{	
#if 0
	/*
	int PonCardIdx;
	int cardInserted;
	int PonChipStatus;
	*/
	CHECK_PON_RANGE
	/*
	PonCardIdx = GetCardIdxByPonChip( PonPortIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );
	
	PonChipStatus = GetPonchipWorkingStatus( PonPortIdx );
	if( PonChipStatus == RERROR ) return ( RERROR );
	if( PonChipStatus > PONCHIP_TESTING ){
		sys_console_printf(" the PON chip current status is %s  ( GetPonPortAdminStatus() )\r\n", PONchipStatus[PonChipStatus] );
		return ( RERROR );
		} 
	*/
	return ( PonPortTable[PonPortIdx].PortAdminStatus );
#else
    return OLTAdv_GetAdminStatus(PonPortIdx);
#endif
}

int  GetPonPortAdminStatusDefault( short int PonPortIdx)
{
	return( PonPortConfigDefault.PortAdminStatus );
}

int  GetPonPortWorkStatus( short int PonPortIdx)
{
    return OLTAdv_GetPonWorkStatus(PonPortIdx);
}


int  ShutdownPonPort( short int PonPortIdx )
{
	CHECK_PON_RANGE
	return( SetPonPortAdminStatus( PonPortIdx, V2R1_DISABLE ));
}

int  StartupPonPort( short int PonPortIdx )
{
	CHECK_PON_RANGE
	return( SetPonPortAdminStatus( PonPortIdx, V2R1_ENABLE ));
}

/*  added by chenfj 2008-12-10
	测试发现,若对5201 PON板上所有PON芯片连续执行reset pon命令,会导致
	设备管理(devsm)中PON板的状态机转移到一个不期望的状态,在等待
	一定的时间(60秒)后,若状态还未正常,则会执行整个PON板的复位;
	修改reset pon API,在完成复位的过程中设置复位标志;并在向设备管理
	报告PON板状态时, 检查发现有复位标志,则保持PON板当前状态不变
*/
int ResetPonPort( short int PonPortIdx )
{
	/*short int PonChipType;*/
	CHECK_PON_RANGE;

#if 1
    return OLT_CALL_ISOK(OLT_ResetPon(PonPortIdx)) ? ROK : RERROR;
#else
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(PonChipType == PONCHIP_PAS5001)
		{
		ShutdownPonPort( PonPortIdx );
		VOS_TaskDelay( VOS_TICK_SECOND*3);
		StartupPonPort( PonPortIdx );
		}
	else if(PonChipType == PONCHIP_PAS5201)
		{
		int slot, port;
		slot = GetCardIdxByPonChip(PonPortIdx);
		port = GetPonPortByPonChip(PonPortIdx);
		if(Olt_exists(PonPortIdx) == TRUE )
			Remove_olt( PonPortIdx, FALSE, FALSE);	
		Hardware_Reset_olt1(slot,port,1,0);
		/*VOS_TaskDelay( VOS_TICK_SECOND/3);*/
		Hardware_Reset_olt2(slot,port,1,0);

		ClearPonRunningData( PonPortIdx );
		SetPonChipResetFlag(slot,port);
		Add_PonPort( PonPortIdx );		
		}

	return( ROK );
#endif
}


int RestartPonPort( short int PonPortIdx )
{
	CHECK_PON_RANGE

#if 1
    return OLT_CALL_ISOK(OLT_ResetPon(PonPortIdx)) ? ROK : RERROR;
#else
	/*if(( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UP )||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_LOOP ) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UPDATE ))*/
	if( OLTAdv_IsExist( PonPortIdx ) == TRUE )
	{
		pon_reset_olt( PonPortIdx );

		Add_PonPort( PonPortIdx );		
	}
#endif
    
	return( ROK );
}


int V2R1_RestartPonPort( short int slot, short int port )
{
	short int PonPortIdx;

	if(PonCardSlotPortCheckByVty(slot, port, 0) != ROK ) return(RERROR);
	/*
	slot--;
	port--;
	if(( slot > 7 ) ||( slot < 3 ))
		{
		sys_console_printf(" slot %d is not pon card(pon card insert) \r\n", (slot +1));
		return ( -1);
		}
	if( (port < 0 ) ||(port >= 4 ))
		{
		sys_console_printf("pon port %d is out of range \r\n", (port+1) );
		return( -1);
		}
	*/
	PonPortIdx = GetPonPortIdxBySlot( slot, port);
	/*sys_console_printf("ponIdx = %d \r\n", PonPortIdx );*/
	if( PonPortIdx != (-1))
		{
		 RestartPonPort(PonPortIdx );	
		return( 0 );
		}
	else return( -1);
}

/*****************************************************
 *
 *    Function:   SetPonPortOperStatus( unsigned short int PonPortIdx, unsigned int OperStatus)
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  unsigned char OperStatus -- the oper status to be set 
 *
 *    Desc:   
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 
int  SetPonPortOperStatus( short int PonPortIdx, unsigned int OperStatus)
{
	int PonCardIdx;
	int cardInserted;
	int PonChipStatus;
	
	CHECK_PON_RANGE
	PonCardIdx = GetCardIdxByPonChip( PonPortIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );
	
	PonChipStatus = GetPonchipWorkingStatus( PonPortIdx );
	if( PonChipStatus == RERROR ) return ( RERROR );
	/*
	if( PonChipStatus > PONCHIP_TESTING )
		{
		sys_console_printf(" the PON chip current status is %s  ( SetPonPortOperStatus() )\r\n", PONchipStatus[PonChipStatus] );
		return ( RERROR );
		} 
	*/
	if(( OperStatus !=PONPORT_UP ) && (OperStatus != PONPORT_DOWN))
		{
		sys_console_printf("To set the PON chip oper status %d is not correct\r\n", OperStatus );
		return ( RERROR );
		}	

	if(( OperStatus ==PONPORT_UP )/* &&(PonPortTable[PonPortIdx].PortWorkingStatus > PONPORT_UPDATE)*/)
		{
		/* modified for PAS-SOFT V5.1 */
		/*PonPortTable[PonPortIdx].PortWorkingStatus = OperStatus ;*/
		pon_add_olt( PonPortIdx );
		/*
		PAS_add_olt( PonPortIdx, PonChipMgmtTable[PonPortIdx].DataPathMAC, &PAS_initialization_parameters_5001 );
		PAS_start_dba_algorithm( PonPortIdx, 1, 0, NULL);
		PAS_update_olt_parameters  ( PonPortIdx, &PAS_updated_parameters);
		PAS_set_system_parameters ( &PAS_system_parameters );
		*/
		}
	if((OperStatus == PONPORT_DOWN ) /*&& (PonPortTable[PonPortIdx].PortWorkingStatus< PONPORT_NOTWORKING )*/)
		{
		pon_remove_olt(PonPortIdx );
		/*PAS_remove_olt(PonPortIdx);*/
		PonPortTable[PonPortIdx].PortWorkingStatus = OperStatus;
		}

	return( ROK );
}


/*****************************************************
 *
 *    Function:   GetPonPortOperStatus( unsigned short int PonPortIdx)
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *
 *    Desc:   
 *
 *    Return:   the pon port current Oper status 
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 
int  GetPonPortOperStatus( short int PonPortIdx )
{
	int PonCardIdx;
	int PonPortId;   
	int cardInserted;
	int PonChipStatus;
	
	CHECK_PON_RANGE
	PonCardIdx = GetCardIdxByPonChip( PonPortIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
    PonPortId = GetPonPortByPonChip( PonPortIdx );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );

	if( getPonChipInserted( (unsigned char)PonCardIdx, (unsigned char )PonPortId) != PONCHIP_EXIST )
		return( RERROR );
	PonChipStatus = GetPonchipWorkingStatus( PonPortIdx );
	if( PonChipStatus == RERROR ) return ( RERROR );
	/*
	if( PonChipStatus > PONCHIP_TESTING ){
		sys_console_printf(" the PON chip current status is %s  ( GetPonPortOperStatus() )\r\n", PONchipStatus[PonChipStatus] );
		return ( RERROR );
		} 
	*/
	if( GetPonPortAdminStatus(PonPortIdx) == V2R1_DISABLE )
		return( PONPORT_DOWN );
	return ( PonPortTable[PonPortIdx].PortWorkingStatus );
}


/*****************************************************
 *
 *    Function:   SetPonPortMaxOnuNum( unsigned short int PonPortIdx, unsigned char MaxOnuNum )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  unsigned char MaxOnuNum  -- Max onu num can be supported
 *
 *    Desc:   
 *
 *    Return:   ROK 
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 
 int  SetPonPortMaxOnuNum( short int PonPortIdx, unsigned char MaxOnuNum )
{
	CHECK_PON_RANGE
		
	if( PonPortIsWorking(PonPortIdx ) != TRUE ) return( RERROR );

	if( MaxOnuNum >= MAXONUPERPON )  return ( RERROR );
	
	PonPortTable[PonPortIdx].MaxOnu = MaxOnuNum;	
	PonPortTable[PonPortIdx].MaxLLID = MaxOnuNum + 1;
	return( ROK );
}

/*****************************************************
 *
 *    Function:   GetPonPortMaxOnuNum( unsigned short int PonPortIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *
 *    Desc:   
 *
 *    Return:   the Max onu num can be supported
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 
 int  GetPonPortMaxOnuNum( short int PonPortIdx )
{
	CHECK_PON_RANGE
		
	if( PonPortIsWorking(PonPortIdx ) != TRUE ) return( RERROR );
	UpdateProvisionedBWInfo( PonPortIdx );
	
	return ( PonPortTable[PonPortIdx].MaxOnu );
}

/*****************************************************
 *
 *    Function:   GetPonPortCurrentOnuNum( unsigned short int PonPortIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *
 *    Desc:   
 *
 *    Return:   the current onu num registered now
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 
 int  GetPonPortCurrentOnuNum( short int PonPortIdx )
{
	CHECK_PON_RANGE
		
	if( PonPortIsWorking(PonPortIdx ) != TRUE ) return( RERROR );
	/*UpdateProvisionedBWInfo( PonPortIdx );*/
	OnLineOnuCounter(PonPortIdx );
	
	return ( PonPortTable[PonPortIdx].CurrOnu );
}

#if 0
/*****************************************************
 *
 *    Function: GetPonPortCurrentOnuStatus( unsigned short int PonPortIdx, unsigned char *CurrentOnuStatus )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  unsigned char *CurrentOnuStatus -- the pointer point to the onu status 
 *
 *    Desc:   
 *
 *    Return:   the current onu registered now
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 
int  GetPonPortCurrentOnuStatus( short int PonPortIdx, unsigned char *CurrentOnuStatus )
{
	int CurOnlineOnu = 0;
	int i,j;
	CHECK_PON_RANGE

	if( CurrentOnuStatus == NULL ) return( RERROR );
	VOS_MemCpy( CurrentOnuStatus , 0, 8 );
	
	if( PonPortIsWorking(PonPortIdx ) != TRUE ) return( RERROR );
	UpdateProvisionedBWInfo( PonPortIdx );
	CurOnlineOnu = GetPonPortCurrentOnuNum( PonPortIdx);

	if( CurOnlineOnu > 0 )
		{
		for( i= 0; i< 8; i++)
			for( j=0; j<8; j++)
				{
				if( GetOnuOperStatus( PonPortIdx, (i*8+j)) == ONU_OPER_STATUS_UP )
					CurrentOnuStatus[i] |= ( 1 << j );
				}
		}
	else if( CurOnlineOnu < 0 )
		{
		return( RERROR );
		}
	
	return( ROK );
}

/*****************************************************
 *
 *    Function: GetPonPortRemainBW( unsigned short int PonPortIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *
 *    Desc:   
 *
 *    Return:   the pon port remain bandwidth currently
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 
int  SetPonPortBWMode( short int PonPortIdx, int  dba_mode )
{
	unsigned int Pre_mode;
	
	CHECK_PON_RANGE

	if((dba_mode != OLT_INTERNAL_DBA ) &&( dba_mode != OLT_EXTERNAL_DBA))  return( RERROR );

	Pre_mode = PonPortTable[PonPortIdx].DBA_mode;
	if( Pre_mode == dba_mode ) return( ROK );

	if( PonPortIsWorking( PonPortIdx ) == FALSE )
		{
		PonPortTable[PonPortIdx].DBA_mode = dba_mode;
		return( ROK );
		}

	/* the port is in working status, so the previous dba mode should be stopped, and the new mode should be started  */
	if(( Pre_mode == OLT_INTERNAL_DBA ) || ( Pre_mode == OLT_EXTERNAL_DBA))
		{
		/* stop the previous dba mode */
		}

	/* start the new mode */
	
	return( ROK );
}

int  GetPonPortActBw( short int PonPortIdx )
{
	CHECK_PON_RANGE
		
	if( PonPortIsWorking(PonPortIdx ) != TRUE ) return( RERROR );
	
	UpdateProvisionedBWInfo( PonPortIdx );	
	return ( PonPortTable[PonPortIdx].ActiveBW);
}
#endif
int  GetPonPortBWMode ( short int PonPortIdx, int *dba_mode )
{
	CHECK_PON_RANGE

	if( dba_mode == NULL ) return( RERROR );
	*dba_mode = PonPortTable[PonPortIdx].DBA_mode ;

	return( ROK );
}

int  GetPonPortBWModeDefault( int *dba_mode )
{
	if( dba_mode == NULL ) return( RERROR );
	*dba_mode = PonPortConfigDefault.DBA_mode ;

	return( ROK );

}

int  GetPonPortRemainBW( short int PonPortIdx )
{
	CHECK_PON_RANGE
    /*moved by luh 2012-11-15,问题单16252*/
	/*if( PonPortIsWorking(PonPortIdx ) != TRUE ) return( RERROR );*/
	UpdateProvisionedBWInfo( PonPortIdx );
	return ( PonPortTable[PonPortIdx].RemainBW);
}

int  GetPonPortMaxBw( short int PonPortIdx )
{
	CHECK_PON_RANGE
	/*
	result = GetPonPortOperStatus(  PonPortIdx );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR ))return( RERROR );*/
	return ( PonPortTable[PonPortIdx].MaxBW);

}

int  GetPonPortProvisionedBw( short int PonPortIdx )
{
	CHECK_PON_RANGE

	UpdateProvisionedBWInfo( PonPortIdx );

	return( PonPortTable[PonPortIdx].ProvisionedBW );

	/*return ( PonPortTable[PonPortIdx].ActiveBW);*/
}

int UpdateProvisionedBWInfo( short int PonPortIdx )
{
    int OnuIdx, OnuEntry, OnuEntryBase;
    unsigned int UplinkBw, DownlinkBw, ActiveUp, ActiveDown;
    unsigned int counter;
    UCHAR *pMac;
    
	if(OLT_ISLOCAL(PonPortIdx))/*for onu swap by jinhl@2013-04-27*/
	{
		CHECK_PON_RANGE

		UplinkBw = 0;
		DownlinkBw = 0;
		ActiveUp = 0;
		ActiveDown = 0;
	    counter = 0;

	    OnuEntryBase = PonPortIdx * MAXONUPERPON;
		for( OnuIdx=0; OnuIdx< MAXONUPERPON; OnuIdx++)
		{
			OnuEntry = OnuEntryBase + OnuIdx;
	        
			ONU_MGMT_SEM_TAKE;
	        pMac = OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr;       
	        if(ROK == ThisIsValidOnu(PonPortIdx,OnuIdx))/*30807 16GPON配置带宽没有检查是否有可以分配的带宽 by jinhl@2016.08.26*/
	        {
	            if ( ONU_OPER_STATUS_UP == OnuMgmtTable[OnuEntry].OperStatus )
	            {
	                ++counter;
	            }

	    		UplinkBw += OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_gr;
	    		DownlinkBw += OnuMgmtTable[OnuEntry].FinalDownlinkBandwidth_gr;
	    		ActiveUp += OnuMgmtTable[OnuEntry].ActiveUplinkBandwidth;
	    		ActiveDown += OnuMgmtTable[OnuEntry].ActiveDownlinkBandwidth;
#ifdef  PLATO_DBA_V3
	    		UplinkBw += OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_fixed;
	    		ActiveUp += OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_fixed;
#endif
	        }
			ONU_MGMT_SEM_GIVE;
		}

		PonPortTable[PonPortIdx].ProvisionedBW = UplinkBw;
		PonPortTable[PonPortIdx].DownlinkProvisionedBW = DownlinkBw;
	    /* B--modified by liwei056@2010-1-12 for D9143 */
		PonPortTable[PonPortIdx].RemainBW = (PonPortTable[PonPortIdx].MaxBW > UplinkBw) ? PonPortTable[PonPortIdx].MaxBW - UplinkBw : 0;
		PonPortTable[PonPortIdx].DownlinkRemainBw = (PonPortTable[PonPortIdx].DownlinkMaxBw > DownlinkBw) ? PonPortTable[PonPortIdx].DownlinkMaxBw - DownlinkBw : 0 ;
	    /* E--modified by liwei056@2010-1-12 for D9143 */
		PonPortTable[PonPortIdx].ActiveBW = ActiveUp;
		PonPortTable[PonPortIdx].DownlinkActiveBw = ActiveDown;

#if 1
		PonPortTable[PonPortIdx].CurrOnu = counter;
		PonPortTable[PonPortIdx].CurLLID = counter;
		if( 0 == counter ) PonPortTable[PonPortIdx].DownlinkActiveBw = 0;
#else
		/* modified by chenfj 2006/11/15 
		  #3202问题单: 下行方向不受限方式，带宽只能一次比一次小*/
		/*if( downlinkBWlimit == V2R1_DISABLE ) 
			PonPortTable[PonPortIdx].DownlinkActiveBw = PonPortTable[PonPortIdx].DownlinkMaxBw;
		*/
		OnLineOnuCounter( PonPortIdx);
		if( PonPortTable[PonPortIdx].CurrOnu == 0 ) PonPortTable[PonPortIdx].DownlinkActiveBw = 0;
#endif
	}
	else
	{
	    OLT_UpdateProvBWInfo(PonPortIdx);
	}
		
	return( ROK );
}

/*****************************************************
 *
 *    Function: GetPonPortCurrentAlarmStatus( unsigned short int PonPortIdx , unsigned int *AlarmNum, unsigned char *CurAlarmStatus )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                unsigned int *AlarmNum -- out, the current alarm num 
 *                  unsigned char *CurAlarmStatus  -- the pointer , current alarm status 
 *
 *    Desc:   
 *
 *    Return:   the pon port alarm status  currently
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 
 int GetPonPortCurrentAlarmStatus( short int PonPortIdx , unsigned long *AlarmNum, unsigned long *CurAlarmStatus )
{
	unsigned int i, alarmCount=0;
	/*unsigned char k;*/
	
	
	CHECK_PON_RANGE
	/*
	result = GetPonPortOperStatus(  PonPortIdx );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR ))return( RERROR );
	*/
	if( PonPortIsWorking(PonPortIdx) != TRUE ) return(RERROR);
		
	if( (CurAlarmStatus == NULL) || (AlarmNum == NULL) ) return( RERROR );

	
#if 0	/* modified 20070705 */
	VOS_MemCpy(CurAlarmStatus,  PonPortTable[PonPortIdx].AlarmStatus, 4);
	*AlarmNum = 0;
#else
	for( i=0; i< sizeof(PonPortTable[PonPortIdx].AlarmStatus) * sizeof(char); i++ )
	{
		if( ((PonPortTable[PonPortIdx].AlarmStatus ) &  (1<<(31-i))) != 0 ) 
			alarmCount ++;
	}
#endif
	*CurAlarmStatus = PonPortTable[PonPortIdx].AlarmStatus;
	*AlarmNum = alarmCount;
	return( ROK );
	
}

#if 0
/* added 20070705 */
 int SetPonPortCurrentAlarmStatus( short int PonPortIdx , unsigned long status )
{
	CHECK_PON_RANGE
	/*
	result = GetPonPortOperStatus(  PonPortIdx );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR ))return( RERROR );
	*/
	if( PonPortIsWorking(PonPortIdx) != TRUE ) return(RERROR);
	
	PonPortTable[PonPortIdx].AlarmStatus = status;
	return( ROK );
}
int GetOltPonPortAlarmMask( short int PonPortIdx , unsigned long *pMask )
{
	int result;

	CHECK_PON_RANGE
	if( pMask == NULL )
		return RERROR;
	
	result = GetPonPortOperStatus(  PonPortIdx );
	if( result == RERROR )
		return( result );

	*pMask = (PonPortTable[PonPortIdx].AlarmMask & 0xf8000000);
	return( ROK );
}
int SetOltPonPortAlarmMask( short int PonPortIdx , unsigned long mask )
{
	int result;

	CHECK_PON_RANGE
	
	result = GetPonPortOperStatus(  PonPortIdx );
	if( result == RERROR )
		return( result );

	PonPortTable[PonPortIdx].AlarmMask = mask & 0xf8000000;
	return( ROK );
}

/* end 20070705 */

/*****************************************************
 *
 *    Function: GetPonPortCurrentAlarmRank( unsigned short int PonPortIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *
 *    Desc:   
 *
 *    Return:   the pon port highest alarm currently
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 
int  GetPonPortCurrentAlarmRank( short int PonPortIdx )
{
	 unsigned long CurAlarmStatus;
	 unsigned long AlarmNum;
	 AlarmClass AlarmRank = NOALARM;
	
	CHECK_PON_RANGE
		
	if( GetPonPortCurrentAlarmStatus( PonPortIdx,  &AlarmNum, &CurAlarmStatus ) == RERROR ) return( AlarmRank );

	/*
	if(( CurAlarmStatus & (1 << i ))||( CurAlarmStatus & (1<< j ))){
		AlarmRank = VITAL;
		}
	else if (( CurAlarmStatus & (1 << k )) ||( CurAlarmStatus & (1<< l))){
		AlarmRank = MAJOR;
		}
	*/
	return (int) ( AlarmRank );	
}

/*****************************************************
 *
 *    Function: SetPonPortMacAgingTime( unsigned short int PonPortIdx, unsigned int AgeingTime )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  unsigned int AgeingTime -- the dynamic MAC ageing time to be set
 *    Desc:   
 *
 *    Return:   ROK
 *
 *    Notes: the AgeingTime should be the multiple of the 5000 milliseconds, and should be less than 24 hours
 *
 *    modified:
 *
 ******************************************************/ 
 int  SetPonPortMacAgingTime( short int PonPortIdx, unsigned int AgeingTime )
{
	PON_olt_update_parameters_t  updated_parameters;
	int result;
	short int PonChipType;
	short int PonChipVer;
	
	CHECK_PON_RANGE

	if( PonPortIsWorking( PonPortIdx ) != TRUE ) return( RERROR );
	/*
	result = GetPonPortOperStatus(  PonPortIdx );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR )) return( RERROR );
	*/
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx );
	/* modified for PAS-SOFT V5.1 */
	if( PonChipType == PONCHIP_PAS )
		{
		if((( AgeingTime % SECOND_5 ) != 0 ) ||(AgeingTime > ONE_DAY )){
			sys_console_printf(" the AgeingTime %d is not consistent (SetPonPortMacAgingTime()) \r\n", AgeingTime);
			return( RERROR );
			}
		/*setUpdateParamToDufault( updated_parameters ); */
		PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT( updated_parameters )
		updated_parameters.address_table_aging_timer = AgeingTime;
	
		if( PAS_update_olt_parameters( PonPortIdx, &updated_parameters) == PAS_EXIT_OK ){
			PonPortTable[PonPortIdx].MACAgeingTime = AgeingTime;
			return( ROK );
			}
		else{ 
			return( RERROR ); 
			}
		}
	
	else{ /* other pon chip type handle */

		}

	return( ROK );
}

/*****************************************************
 *
 *    Function: GetPonPortMacAgingTime(unsigned short int PonPortIdx)
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *
 *    Desc:   
 *
 *    Return:    the dynamic MAC ageing time
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int  GetPonPortMacAgingTime( short int PonPortIdx)
{
	int result;
	
	CHECK_PON_RANGE
	result = GetPonPortOperStatus(  PonPortIdx );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR )) return( RERROR );

	return( PonPortTable[PonPortIdx].MACAgeingTime );
}

/*****************************************************
 *
 *    Function: GetPonPortMaxLLIDNum ( unsigned short int PonPortIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *
 *    Desc:   
 *
 *    Return:    the MAX number of LLID can be supported
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 int  GetPonPortMaxLLIDNum( short int PonPortIdx )
{
	CHECK_PON_RANGE
		
	return( ROK );
}

/*****************************************************
 *
 *    Function: GetPonPortCurrentLLIDUsed ( unsigned short int PonPortIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *
 *    Desc:   
 *
 *    Return:    the number of LLID currently used
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
short int  GetPonPortCurrentLLIDUsed ( short int PonPortIdx )
{
	CHECK_PON_RANGE
	/*
	result = GetPonPortOperStatus(  PonPortIdx );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR )) return( RERROR );
	*/
	if( PonPortIsWorking(PonPortIdx) != TRUE ) return(0);
	
	return( PonPortTable[PonPortIdx].CurLLID );
}

/*****************************************************
 *
 *    Function: SetPonPortMaxRTT( unsigned short int PonPortIdx, unsigned int MaxRTT  )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  unsigned int MaxRTT -- the MAX round trip time measured in TQ
 *    Desc:   
 *
 *    Return:    the max RTT can be supported
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int  SetPonPortMaxRTT( short int PonPortIdx, unsigned int MaxRTT  )
{
	int result;
	short int PonChipType;
	short int PonChipVer = RERROR;
	
	CHECK_PON_RANGE

	result = getPonChipInserted(GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
	if( result != PONCHIP_EXIST )
		{
		PonPortTable[PonPortIdx].MaxRTT = MaxRTT;
		return( ROK );
		}
	if(PonPortIsWorking(PonPortIdx) != TRUE)
		{
		PonPortTable[PonPortIdx].MaxRTT = MaxRTT;
		return( ROK );
		}
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}

	if( PonChipType == PONCHIP_PAS )
		{
		/* modified for PAS-SOFT V5.1 
		PonChipVer = V2R1_GetPonChipVersion( PonPortIdx );*/
		if((PonChipVer != PONCHIP_PAS5001 ) && ( PonChipVer != PONCHIP_PAS5201 )) return( RERROR );
		
		if( PonChipVer == PONCHIP_PAS5001 )
			{
			PON_olt_update_parameters_t  updated_parameters;
			
			PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT( updated_parameters );
			updated_parameters.max_rtt = MaxRTT;

			if( PAS_update_olt_parameters_v4( PonPortIdx, &updated_parameters) == PAS_EXIT_OK )
				{
				PonPortTable[PonPortIdx].MaxRTT = MaxRTT;
				return( ROK );
				}
			else{
				return( RERROR );
				}	
			}
		else if( PonChipVer == PONCHIP_PAS5201 )
			{
			PON_update_olt_parameters_t  updated_param;

			PON_EMPTY_UPDATE_OLT_PARAMETERS_STRUCT( updated_param );			
			updated_param.max_rtt = MaxRTT;

			if( PAS_update_olt_parameters( PonPortIdx, &updated_param) == PAS_EXIT_OK )
				{
				PonPortTable[PonPortIdx].MaxRTT = MaxRTT;
				return( ROK );
				}
			else{
				return( RERROR );
				}

			}
		}

	else{ /* other pon chip type handle */

		}

	return( RERROR );
}

int GetPonPortMaxRTT( short int PonPortIdx )
{	
	CHECK_PON_RANGE
	/*
	result = GetPonPortOperStatus(  PonPortIdx );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR )) return( RERROR );
	*/
	if( PonPortIsWorking(PonPortIdx) != TRUE ) return(RERROR);
	
	return( PonPortTable[PonPortIdx].MaxRTT );

}

/*****************************************************
 *
 *    Function: SetPonPortApsCtrl( unsigned short int PonPortIdx, unsigned int CtrlValue )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                unsigned int CtrlValue -- the protect switch control to be set 
 *    Desc:   
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int  SetPonPortApsCtrl( short int PonPortIdx, unsigned int CtrlValue )
{
	CHECK_PON_RANGE

	if(( CtrlValue == PROTECT_SWITCH_CTRL_AUTO )||(CtrlValue == PROTECT_SWITCH_CTRL_DISABLE )){
		PonPortTable[PonPortIdx].ApsCtrl = CtrlValue;
		return( ROK );
		}
	
	if( CtrlValue == PROTECT_SWITCH_CTRL_FORCE ) 
		{
		if( PonPortTable[PonPortIdx].ApsCtrl == PROTECT_SWITCH_CTRL_DISABLE) return( RERROR );
		
		/*if(( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UP )||(PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_LOOP))*/
		if( PonPortIsWorking( PonPortIdx ) == TRUE )
			{
			/* not supported in this passave pon chip
			SetPonPortSwitch(PonPortIdx); 
			ChangePonPortApsStatus(PonPortIdx, status );
			*/
			}
		PonPortTable[PonPortIdx].ApsCtrl = CtrlValue;
		return(ROK);
		}
	
	sys_console_printf(" the protect switch ctrl mode %d is not correct (SetPonPortApsCtrl())\r\n", CtrlValue );
	return( RERROR );	
}

/*****************************************************
 *
 *    Function: GetPonPortApsCtrl( unsigned short int PonPortIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  
 *    Desc:   
 *
 *    Return:    the protect switch control 
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int  GetPonPortApsCtrl( short int PonPortIdx )
{
	CHECK_PON_RANGE

	if( PonPortIsWorking( PonPortIdx ) != TRUE ) return( RERROR );
	return( PonPortTable[PonPortIdx].ApsCtrl ); /*PROTECT_SWITCH_STATUS_UNKNOWN */
}

int  GetPonPortApsCtrlDefault()
{
	return( PonPortConfigDefault.ApsCtrl ); 
}

int  GetPonPortApsStatus( short int PonPortIdx )
{
	CHECK_PON_RANGE

	if( PonPortIsWorking( PonPortIdx ) != TRUE ) return( RERROR );

	return( PonPortTable[PonPortIdx].ApsStatus); /* should be active in this v2r1 version: PROTECT_SWITCH_STATUS_ACTIVE*/
}

/*****************************************************
 *
 *    Function: SetPonPortEncrypt( short int PonPortIdx, unsigned int CtrlValue, unsigned int  cryptionDirectioin )
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
int  SetPonPortEncrypt( short int PonPortIdx,/* unsigned int CtrlValue,*/ unsigned int  EncryptionDirection )
{

	short int i;
	
	CHECK_PON_RANGE

		
	if(( EncryptionDirection != PON_ENCRYPTION_PURE)&&(EncryptionDirection != PON_ENCRYPTION_DIRECTION_DOWN)&&(EncryptionDirection != PON_ENCRYPTION_DIRECTION_ALL)) return( RERROR );

	/*if(PonPortTable[PonPortIdx].EncryptDirection == EncryptionDirection ) return( ROK );*/
	
	if( EncryptionDirection == PON_ENCRYPTION_PURE )
		{
		PonPortTable[PonPortIdx].EncryptEnable = V2R1_DISABLE;
		PonPortTable[PonPortIdx].EncryptKeyTime = PonPortConfigDefault.EncryptKeyTime;
		}
	else{
		PonPortTable[PonPortIdx].EncryptEnable = V2R1_ENABLE;
		}
	PonPortTable[PonPortIdx].EncryptDirection = EncryptionDirection;

	for( i= 0; i< MAXONUPERPON; i++)
		{
		OnuEncryptionOperation( PonPortIdx, i, EncryptionDirection );
		}

	return( ROK );
		
}

int  GetPonPortEncrypt ( short int PonPortIdx , unsigned int *cryptionDirection)
{
	CHECK_PON_RANGE

	if( cryptionDirection == NULL ) return( RERROR );

	*cryptionDirection = PonPortTable[PonPortIdx].EncryptDirection;
	return( ROK );
}

int  GetPonPortEncryptDefault ( unsigned int *cryptionDirection)
{
	if( cryptionDirection == NULL ) return( RERROR );

	*cryptionDirection = PonPortConfigDefault.EncryptDirerction;
	return( ROK );
}

int  SetPonPortEncryptKeyTime( short int PonPortIdx, unsigned int Timelen)
{
	short int i;
	
	CHECK_PON_RANGE

	PonPortTable[PonPortIdx].EncryptKeyTime = Timelen;
	for( i=0; i< MAXONUPERPON; i++)
		{
		if( ThisIsValidOnu( PonPortIdx, i ) == ROK )
			SetOnuEncryptKeyExchagetime( PonPortIdx, i, Timelen);
		}	

	return( ROK );
}

int  GetPonPortEncryptKeyTime( short int PonPortIdx , unsigned int *TimeLen)
{
	CHECK_PON_RANGE

	if( TimeLen == NULL )  return( RERROR );

	*TimeLen = (PonPortTable[PonPortIdx].EncryptKeyTime/SECOND_1);

	return( ROK );
}

int  GetPonPortEncryptKeyTimeDefault (  unsigned int *TimeLen )
{
	if( TimeLen == NULL ) return( RERROR );

	*TimeLen = ( PonPortConfigDefault.EncryptKeyTime/SECOND_1);
	return( ROK );
}

/*****************************************************
 *
 *    Function: SetPonPortBerThreshold( unsigned short int PonPortIdx, unsigned int Ber_Threshold )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                   unsigned int Ber_threshold -- the BER alarm threshold to be set 
 *    Desc:   
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int  SetPonPortBerThreshold( short int PonPortIdx, long double Ber_Threshold )
{
	short int PonChipIdx;
	short int PonChipType;
	short int PonChipVer = RERROR;
		
	CHECK_PON_RANGE

	PonChipIdx = GetPonChipIdx(  PonPortIdx );
	if( PonChipIdx == RERROR ) return ( RERROR );
	PonChipType = V2R1_GetPonchipType( PonChipIdx );
	if( PonChipType == RERROR ) return ( RERROR );
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}

	if( PonChipType == PONCHIP_PAS )
		{	
		if( PonChipVer == PONCHIP_PAS5001 )
			{
			PAS_link_alarm_params_t  link_alarm_params;
			if ((Ber_Threshold!= DEFAULT_LINK_ALARM_PARAMETER_VALUE) &&
				(Ber_Threshold != OFF) &&
				((Ber_Threshold < 0) || (Ber_Threshold > 1)))
				{
				sys_console_printf ("Error, out of bounds ber_threshold value: %Lg (SetPonPortBerThreshold()) \r\n", 
						Ber_Threshold );
				return ( RERROR );
				}

			PonPortTable[PonPortIdx].BERThreshold = Ber_Threshold;
			/*
			result = GetPonPortOperStatus(  PonPortIdx );
			if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR )) return( RERROR );
			*/
			if( PonPortIsWorking(PonPortIdx) != TRUE ) return(RERROR);
			
			link_alarm_params.ber_threshold =  Ber_Threshold;
			link_alarm_params.minimum_error_bytes_threshold = PON_VALUE_NOT_CHANGED;
			link_alarm_params.fer_threshold = PON_VALUE_NOT_CHANGED;
			link_alarm_params.minimum_error_frames_threshold = PON_VALUE_NOT_CHANGED;
			link_alarm_params.llid_mismatch_threshold = PON_VALUE_NOT_CHANGED;
	
			PAS_set_link_alarm_params_v4 ( PonPortIdx,  &link_alarm_params );
			}
		else 
			{

			}
		}

	else{/* other pon chip type handle */

		}

	return( ROK );	
}

/*****************************************************
 *
 *    Function: GetPonPortBerThreshold( unsigned short int PonPortIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *
 *    Desc:   
 *
 *    Return:   the BER alarm threshold
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
long double  GetPonPortBerThreshold( short int PonPortIdx )
{
	CHECK_PON_RANGE

	return( PonPortTable[PonPortIdx].BERThreshold );
}

/*****************************************************
 *
 *    Function: SetPonPortFerThreshold( unsigned short int PonPortIdx, long double FER_threshold )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                long double FER_threshold -- the FER alarm threshold to be set 
 *    Desc:   
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int  SetPonPortFerThreshold( short int PonPortIdx, long  double  FER_threshold )
{
	short int PonChipIdx;
	short int PonChipType;
	short int PonChipVer = RERROR;
		
	CHECK_PON_RANGE
		
	PonChipIdx = GetPonChipIdx(  PonPortIdx );
	if( PonChipIdx == RERROR ) return ( RERROR );
	PonChipType = V2R1_GetPonchipType( PonChipIdx );
	if( PonChipType == RERROR ) return ( RERROR );
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}

	if( PonChipType == PONCHIP_PAS )
		{
		if( PonChipVer == PONCHIP_PAS5001 )
			{
			PAS_link_alarm_params_t  link_alarm_params;
			
			if ((FER_threshold!= DEFAULT_LINK_ALARM_PARAMETER_VALUE) &&
				(FER_threshold != OFF) &&
				((FER_threshold < 0) || (FER_threshold > 1)))
				{
				sys_console_printf ("Error, out of bounds fer_threshold value: %Lg ( SetPonPortFerThreshold()) \r\n", 
						FER_threshold );
				return ( RERROR );
				}
		
			PonPortTable[PonPortIdx].FERThreshold = FER_threshold;
			/*
			result = GetPonPortOperStatus(  PonPortIdx );
			if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR )) return( RERROR );
			*/
			if( PonPortIsWorking(PonPortIdx) != TRUE )return(RERROR);
				
			link_alarm_params.ber_threshold = PON_VALUE_NOT_CHANGED;
			link_alarm_params.minimum_error_bytes_threshold = PON_VALUE_NOT_CHANGED;
			link_alarm_params.fer_threshold = FER_threshold ;
			link_alarm_params.minimum_error_frames_threshold = PON_VALUE_NOT_CHANGED ;
			link_alarm_params.llid_mismatch_threshold = PON_VALUE_NOT_CHANGED;
	
			PAS_set_link_alarm_params_v4 ( PonPortIdx,  &link_alarm_params );
			}
		
		else
			{

			}
		}
	
	else{ /* other pon chip type handler */

		}

	return( ROK );
}

/*****************************************************
 *
 *    Function: GetPonPortFerThreshold( unsigned short int PonPortIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *
 *    Desc:   
 *
 *    Return:   the FER alarm threshold
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
long double  GetPonPortFerThreshold( short int PonPortIdx )
{
	CHECK_PON_RANGE
	if( PonPortIdx >= MAXPON )
		{
		sys_console_printf("%s/port%d is not correct ( GetPonPortFerThreshold())\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
		return( RERROR );
		}

	return( PonPortTable[PonPortIdx].FERThreshold );
}
#endif

LONG localDelOnuFromPon( short int pon_id, short int onu_idx )
{
	int slot, port;
	short int onuMgtIdx;
	UCHAR pMacAddr[BYTES_IN_MAC_ADDRESS];
	UCHAR SerialNo[GPON_ONU_SERIAL_NUM_STR_LEN];
	UCHAR SerialNoAfterSIXDigit[BYTES_IN_MAC_ADDRESS];
	UCHAR series_tmp[BYTES_IN_MAC_ADDRESS];
	int operStatus;
	int ret;
	UCHAR IsGponOnu;

	if( !OLT_LOCAL_ISVALID(pon_id) || !ONU_IDX_ISVALID(onu_idx) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	onuMgtIdx = pon_id * MAXONUPERPON + onu_idx;
	slot = GetCardIdxByPonChip(pon_id);
	port = GetPonPortByPonChip(pon_id);
	
	ONU_MGMT_SEM_TAKE;
	operStatus = OnuMgmtTable[onuMgtIdx].OperStatus;
	VOS_MemCpy(pMacAddr, OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS);
	VOS_MemCpy(SerialNo, OnuMgmtTable[onuMgtIdx].DeviceInfo.DeviceSerial_No, GPON_ONU_SERIAL_NUM_STR_LEN);
	IsGponOnu = OnuMgmtTable[onuMgtIdx].IsGponOnu;
	ONU_MGMT_SEM_GIVE;

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE/*SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER*/ )
	{
		/* 只在主控上报，防止重复 *//*modified for GPON onu alarm Q:31046 & Q:28484 by yangzl @2016-7-1*/
		/*if( !(MAC_ADDR_IS_ZERO(pMacAddr) || MAC_ADDR_IS_BROADCAST(pMacAddr))
            ||
			((ONU_OPER_STATUS_UP == operStatus)
			    || (ONU_OPER_STATUS_PENDING == operStatus)
			    || (ONU_OPER_STATUS_DORMANT == operStatus)
			    ) )*/
		 ret = ThisIsValidOnu( pon_id, onu_idx);
		if(ret == 0)
		{
		/*begin--mod by liub, 2017-5-15. gpon onu地址在能获取并保存在管理表之后，这里不需要单独处理*/
#if 0
			if(IsGponOnu == 1)
			{
				short int icount;
				

				/*for(icount = 0;icount < 6;icount += 1)
			     	{
			     		SerialNoAfterSIXDigit[icount] = SerialNo[10+icount] - '0';
			     	}*/

				SerialNoAfterSIXDigit[0]=0x00;
				SerialNoAfterSIXDigit[1]=0x0F;
				SerialNoAfterSIXDigit[2]=0xE9;
				for(icount = 0;icount < 6;icount += 1)
		  		{
					SerialNo[GPON_ONU_SERIAL_NUM_STR_LEN-1]=0;
					if(SerialNo[10+icount] >= 'A' && SerialNo[10+icount] <= 'F')
						series_tmp[icount] = SerialNo[10+icount]-'A' + 10;
					else if(SerialNo[10+icount] >= 'a' && SerialNo[10+icount] <= 'f')
						series_tmp[icount] = SerialNo[10+icount]-'a' + 10;
					else
						series_tmp[icount] = SerialNo[10+icount] - '0';
		    	}
				
				SerialNoAfterSIXDigit[3] = ((series_tmp[0]) << 4) | series_tmp[1];	
				SerialNoAfterSIXDigit[4] = ((series_tmp[2]) << 4) | series_tmp[3];
				SerialNoAfterSIXDigit[5] = ((series_tmp[4]) << 4) | series_tmp[5];

				onuDeletingNotify_EventReport( OLT_DEV_ID, slot, port, (onu_idx+1), SerialNoAfterSIXDigit );
			}
			else
				onuDeletingNotify_EventReport( OLT_DEV_ID, slot, port, (onu_idx+1), pMacAddr );
#else
			onuDeletingNotify_EventReport( OLT_DEV_ID, slot, port, (onu_idx+1), pMacAddr );
#endif
			/*end--mod by liub, 2017-5-15. */
		}
	}

	if ( ONU_OPER_STATUS_UP == operStatus )
	{
		/* 在线 ONU等待离线事件的处理*/
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[onuMgtIdx].NeedDeleteFlag = TRUE;
		/*clearDeviceAlarmStatus( MAKEDEVID(slot, port, (onu_idx+1)) );
		ClearOnuRunningData(pon_id, onu_idx, 0);
		setOnuStatus( slot,  port, onu_idx, ONU_OFFLINE );*/
		ONU_MGMT_SEM_GIVE;
	}
	else
	{
		clearDeviceAlarmStatus( MAKEDEVID(slot, port, (onu_idx+1)) );
		
		/* 不在线 ONU，直接清空其配置对象*/
		InitOneOnuByDefault(pon_id, onu_idx);
		/*ONU_MGMT_SEM_TAKE;
		setOnuStatus( slot,  port, onu_idx, ONU_OFFLINE );
		ONU_MGMT_SEM_GIVE;*/
	}


	return VOS_OK;
}

/*****************************************************
 *
 *    Function: SetPonPortInvalidOnu( unsigned short int PonPortIdx,  unsigned char *OnuMac, unsigned int flag)
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  unsigned char *OnuMac -- the invalid onu mac address
 *                  unsigned int flag -- add or delete
 *    Desc:   
 *
 *    Return:   ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int localOnuRegisterControl( short int pon_id, short int llid, unsigned char *OnuMac, short int  *ret_PonPort )
{
	short int PonPortIdx, OnuIdx;
	short int PonChipType = PONCHIP_PAS;
	int swapPort;
	int onuStatus;
	int onuEntryBase, onuEntry;
	int ret = ROK;
	ULONG slotno, portno;
	int onu_idx = 0;
	/*for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
    onu_idx = GetOnuIdxByLlid(pon_id, llid);
	#if 0/*for onu swap by jinhl@2013-02-22*/
	if(RERROR != onu_idx)
	{
	    return RERROR;
	}
	#endif
	
	if(PONid_ONUMAC_BINDING == V2R1_DISABLE)
	{
		if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
			PonChipType = GetPonChipTypeByPonPort(pon_id );
		else
			PonChipType = 0;
	}
	/*CHECK_PON_RANGE;*/

	ONU_MGMT_SEM_TAKE;
	for( PonPortIdx=0; PonPortIdx < MAXPON; PonPortIdx++ )
	{
		if( PonPortIdx == pon_id )
			continue;
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
		swapPort = ThisIsHotSwapPort(pon_id, PonPortIdx );
		if((swapPort  == V2R1_IS_HOT_SWAP_PORT) || (swapPort == V2R1_SWAP_PORT_BUT_NOT_ACTIVE) )
			continue;
#endif		
		onuEntryBase = PonPortIdx * MAXONUPERPON;
		for(OnuIdx=0; OnuIdx< MAXONUPERPON; OnuIdx++)
		{
			onuEntry = onuEntryBase+OnuIdx;
			if( MAC_ADDR_IS_EQUAL(OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, OnuMac) )
			{
				slotno = GetCardIdxByPonChip(PonPortIdx);
				portno = GetPonPortByPonChip(PonPortIdx);
			
				if(PONid_ONUMAC_BINDING == V2R1_ENABLE)
				{
					ONU_MGMT_SEM_GIVE;
					if( ret_PonPort ) 
						*ret_PonPort = PonPortIdx;

					sys_console_printf( "\r\n onu-pon binding:pon%d/%d,mac=%02x%02x.%02x%02x.%02x%02x conflict with pon%d/%d\r\n",
						GetCardIdxByPonChip(pon_id), GetPonPortByPonChip(pon_id),
						OnuMac[0], OnuMac[1], OnuMac[2], OnuMac[3], OnuMac[4], OnuMac[5],
						slotno, portno );

					return RERROR;
				}
				else
				{
					onuStatus = OnuMgmtTable[onuEntry].OperStatus;
					if( (onuStatus == ONU_OPER_STATUS_UP) || (onuStatus == ONU_OPER_STATUS_PENDING) || (onuStatus == ONU_OPER_STATUS_DORMANT) )
					{
						if( EVENT_REGISTER == V2R1_ENABLE )
						{
							sys_console_printf( "\r\n find onu has been registeded in pon%d/%d before\r\n", slotno, portno );
						}
						/* send trap to NMS: ONU注册冲突 */
						Trap_OnuRegisterConflict(PonPortIdx, OnuIdx );

						/* deny this Onu */
						
						if( PonChipType == PONCHIP_PAS )
						{
						    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
							OnuMgt_AuthorizeOnu ( pon_id,  onu_idx, FALSE );
						}
						
						/*PAS_shutdown_onu( PonPort_id, onu_id, TRUE );

						add by chenfj 2006/10/11 */
						/*if(PonPortIdx != pon_id )
						{
							AddPendingOnu_conf( pon_id, onu_id , OnuMac, PonPortIdx);
						}
						else
						{
							AddPendingOnu(pon_id, onu_id, OnuMac);
						}*/

						ret = RERROR;
					/*VOS_SysLog( LOG_TYPE_ALARM, LOG_WARNING, "\r\n onu-double-reg:pon%d/%d,mac=%02x%02x.%02x%02x.%02x%02x conflict witch pon%d/%d\r\n",
						GetCardIdxByPonChip(pon_id), GetPonPortByPonChip(pon_id),
						OnuMac[0], OnuMac[1], OnuMac[2], OnuMac[3], OnuMac[4], OnuMac[5],
						GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx) );*/
					}
					else
					{
						/* modified by xieshl 20110727, 删除ONU时应通知PON板同步删除，问题单12871 */
						/*localDelOnuFromPon( PonPortIdx, OnuIdx );*/
						OnuMgtSync_OnuDeletedNotify( slotno, portno, OnuIdx );
					}
				}
			}
		}
 	}
	ONU_MGMT_SEM_GIVE;

	if( EVENT_REGISTER == V2R1_ENABLE )
	{
		if(PONid_ONUMAC_BINDING == V2R1_ENABLE)
		{
			if( ret == ROK )
			{
				sys_console_printf( "\r\n check onu-pon binding:pon%d/%d with onu %02x%02x.%02x%02x.%02x%02x OK\r\n",
					GetCardIdxByPonChip(pon_id), GetPonPortByPonChip(pon_id),
					OnuMac[0], OnuMac[1], OnuMac[2], OnuMac[3], OnuMac[4], OnuMac[5]);
			}
		}
	}

	return ret;
}

int CheckIsInvalidOnu( short int PonPortIdx, unsigned char *OnuMac )
{
	unsigned char *OnuMacAddr;
	MacTable_S *NextPtr;
	unsigned int slot, port;
	short int PonChipType;
	int ret;
	unsigned long OnuAuthEnable;
    ULONG OnuAuthMode = 0;
	unsigned long AuthOnuIdx;
	
	CHECK_PON_RANGE

	/*if( CompTwoMacAddress( OnuMac, Invalid_Mac_Addr ) == ROK ) return( RERROR );
	if( CompTwoMacAddress( OnuMac, Invalid_Mac_Addr1 ) == ROK )return( RERROR );*/
	if( MAC_ADDR_IS_INVALID(OnuMac) )
		return RERROR;
	
	/*if( PonPortTable[PonPortIdx].InvalidOnuNum == 0 ) return( RERROR );*/
	NextPtr = PonPortTable[PonPortIdx].InvalidOnuTablePtr;
	
	while( NextPtr != NULL )
		{
		
		OnuMacAddr = NextPtr->MAC;
		NextPtr =(MacTable_S *)NextPtr->nextMac;		
		if( MAC_ADDR_IS_EQUAL( OnuMac, OnuMacAddr ))
			{
			return ( ROK );
			}		
		}

	/* added by chenfj 2007-6-26
	     for onu authenticaion by PAS5001 */
    slot = GetCardIdxByPonChip( PonPortIdx );
    port = GetPonPortByPonChip( PonPortIdx );
	getOnuAuthEnable(slot, port, &OnuAuthEnable );
    mn_getCtcOnuAuthMode(slot, port, &OnuAuthMode);

	if(( OnuAuthEnable == V2R1_ONU_AUTHENTICATION_ENABLE ) &&( OnuAuthMode == 1))
		{
		PonChipType = V2R1_GetPonchipType( PonPortIdx );
		
		if( OLT_PONCHIP_ISPAS(PonChipType) )
			{
				{
				if(( slot != RERROR ) &&(port != RERROR ))
					{
					ret = isHaveAuthMacAddress( slot, port, OnuMac, &AuthOnuIdx );
					if( ret != ROK ) return( ROK );
					}	
				}
			}
		
		else{ /* other pon chip handler */

			}
		}

	/* 若使能了ONU 与PON 端口的绑定, 则检查在别的PON 口下是否也有此ONU 的记录
	if(PONid_ONUMAC_BINDING == V2R1_ENABLE)
		{
		if(localCheckOnuRegisterContral( PonPortIdx, OnuMac ) == TRUE )
			return(ROK);
		}
	*/
	return( RERROR );
}

#if 0
/* B--added by liwei056@2010-10-19 for Redundancy-RegisterBUG */
int CheckIsInvalidOnuId( short int PonPortIdx, short int llid, mac_address_t mac )
{
    short int OnuIdx;
    short int OnuStatus;
    bool DoubleRegFlag;

    if ( RERROR != (OnuIdx = GetOnuIdxByLlid(PonPortIdx, llid)) )
    {
        int OnuEntry;
        CHECK_ONU_RANGE;

        OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

        ONU_MGMT_SEM_TAKE;
        OnuStatus = OnuMgmtTable[OnuEntry].OperStatus;
        DoubleRegFlag = MAC_ADDR_IS_EQUAL(mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr );
        ONU_MGMT_SEM_GIVE;

        /* B--added by liwei056@2011-1-5 for D11777 */
        if( ONU_OPER_STATUS_UP == OnuStatus )
        {
            ONUDeregistrationInfo_S stDeregEvt;

            OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event by double-register.\r\n", PonPortIdx, OnuIdx+1);
            stDeregEvt.olt_id = PonPortIdx;
            stDeregEvt.onu_id = llid;
            stDeregEvt.deregistration_code = PON_ONU_DEREGISTRATION_DOUBLE_REGISTRATION;
            stDeregEvt.event_flags = ONU_EVENTFLAG_VIRTAL;
            OnuDeregisterHandler(&stDeregEvt);
        }
        else
        /* E--added by liwei056@2011-1-5 for D11777 */
        {
            if ( DoubleRegFlag )
            {
                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d resume onu%d's real registering status by double-register.\r\n", PonPortIdx, OnuIdx+1);
            	return( ROK );
            }
            else
            {
                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d clear onu%d's virtual registering status by double-register.\r\n", PonPortIdx, OnuIdx+1);
                ClearOnuRunningData( PonPortIdx, OnuIdx, 0 );
            }
        }

        return RERROR;
    }

	return( ROK );
}
/* E--added by liwei056@2010-10-19 for Redundancy-RegisterBUG */
#endif

int  SetPonPortInvalidOnu( short int PonPortIdx,  unsigned char *OnuMac, unsigned int flag )
{
	int OnuIsInvalid;
	MacTable_S *NextPtr, *LastPtr, *CurPtr;
	int counter;
	short int OnuIdx, OnuEntry;
	unsigned int def_up_bw_gr = 0;
	unsigned int def_down_bw_gr = 0;
	
	CHECK_PON_RANGE;

	if( OnuMac == NULL  )return( RERROR );
	
	if(( flag != ADD_INVALID_ONU ) && ( flag != DELETE_INVALID_ONU )) 
		{
		sys_console_printf("Error, out ot bounds flag %d ( SetPonPortInvallidOnu())\r\n", flag);
		return( RERROR );
		}

	OnuIsInvalid = CheckIsInvalidOnu( PonPortIdx, OnuMac);
	if(( OnuIsInvalid == ROK ) && ( flag == ADD_INVALID_ONU )) return( ROK );
	if(( OnuIsInvalid == RERROR ) && ( flag == DELETE_INVALID_ONU )) return( ROK );
	NextPtr = PonPortTable[PonPortIdx].InvalidOnuTablePtr;	
	
	if( flag == ADD_INVALID_ONU )
		{

		if( NextPtr == NULL )
			{
			/* The first invalid onu in the table */
			NextPtr = ( MacTable_S *) VOS_Malloc(sizeof(MacTable_S), MODULE_PON );
			if( NextPtr == NULL )
				{
				ASSERT(0);
				sys_console_printf("Error, Malloc buffer not satified ( SetPonPortInvalidOnu())\r\n" );
				return( RERROR );
				}
			VOS_MemCpy( NextPtr->MAC, OnuMac, BYTES_IN_MAC_ADDRESS );
			 NextPtr->nextMac = 0;
			PonPortTable[PonPortIdx].InvalidOnuTablePtr = NextPtr;
			PonPortTable[PonPortIdx].InvalidOnuNum = 1;
			
			}
		
		else {		
			/* the table is exist already */
			/*while( (MacTable_S *)&(NextPtr->nextMac)  != NULL ) {*/
			counter = PonPortTable[PonPortIdx].InvalidOnuNum-1;
			while(( NextPtr ->nextMac != 0 ) || ( counter != 0))
				{
				NextPtr = (MacTable_S *)NextPtr->nextMac;
				counter--;
				}
			CurPtr = (MacTable_S*) VOS_Malloc( sizeof(MacTable_S), MODULE_PON );
			if( CurPtr == NULL )
				{
				ASSERT(0);
				sys_console_printf("Error, Malloc buffer not satified (SetPonPortInvalidOnu())\r\n" );
				return( RERROR );
				}
			VOS_MemCpy( CurPtr->MAC, OnuMac, BYTES_IN_MAC_ADDRESS );
			NextPtr->nextMac = (unsigned int ) CurPtr;
			CurPtr->nextMac = 0;
			/*
			LastPtr = (MacTable_S *)&(CurPtr->nextMac );
			LastPtr = NULL;
			*/
			PonPortTable[PonPortIdx].InvalidOnuNum ++;
			
			}
		
		/* check if the MAC is in registered in this pon; if so, shutdown it */
		OnuIdx = GetOnuIdxByMacPerPon( PonPortIdx,  OnuMac );
		if( OnuIdx != RERROR )
			{
			OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;	
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[OnuEntry].NeedDeleteFlag = TRUE;
			if( OnuMgmtTable[OnuEntry].OperStatus != ONU_OPER_STATUS_DOWN)
				Onu_ShutDown(  PonPortIdx,  OnuIdx );
			else
			{
				GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &def_up_bw_gr, NULL);
				GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK, &def_down_bw_gr, NULL);
				PonPortTable[PonPortIdx].DownlinkProvisionedBW += OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkBandwidth_gr - def_down_bw_gr;
				PonPortTable[PonPortIdx].ProvisionedBW += OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_gr - def_up_bw_gr;
#ifdef PLATO_DBA_V3
				PonPortTable[PonPortIdx].ProvisionedBW += OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_fixed ;
#endif
				InitOneOnuByDefault( PonPortIdx, OnuIdx);
			}
			ONU_MGMT_SEM_GIVE;
				
			}
		}
	
	else if (flag ==  DELETE_INVALID_ONU )
		{
		LastPtr = NextPtr;
		while( NextPtr != NULL )
			{
			CurPtr = NextPtr;
			NextPtr =(MacTable_S *)NextPtr->nextMac;
			if(VOS_MemCmp( OnuMac, CurPtr->MAC, BYTES_IN_MAC_ADDRESS ) == 0 )
				{
				if( CurPtr == LastPtr ) { PonPortTable[PonPortIdx].InvalidOnuTablePtr = NextPtr;}
				if( CurPtr != LastPtr ) { LastPtr->nextMac =(unsigned int)NextPtr; }
				VOS_Free( (void *)CurPtr );
				PonPortTable[PonPortIdx].InvalidOnuNum--;
				return( ROK );
				}
			LastPtr = CurPtr;
			}
		return( ROK );
		}
	return( ROK );
}


/*****************************************************
 *
 *    Function: AddPendingOnu( short int PonPortIdx, short int Llid )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  
 *    Desc:   当ONU注册时,若ONU不能满足约束条件时,则这个ONU就不能被准许进入网络;
 *               此时ONU进入pending状态;当条件满足时,再激活
 *
 *    Return: 
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
/* added by chenfj 2006/09/22 */ 
/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/
static int __AddPendingOnu( short int PonPortIdx, short int Llid, unsigned  char *mac_address, PendingReason_S code)
{
	pendingOnu_S *CurOnu, *PreOnu;	
	short int PonChipType;
	
	/*short int  onuId;
	int ret;*/

	/*CHECK_PON_RANGE*/

	/* added by chenfj 2007-04-12 当限制ONU注册时，同时关闭其数据通道*/
	
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
		
    CTC_SetOnuUniPort( PonPortIdx, Llid, 1, 0 );
	/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

	CurOnu = PonPortTable[PonPortIdx].PendingOnu.Next;
	PreOnu = NULL;

	while( CurOnu != NULL )
	{
		if(CurOnu->Llid == Llid ) return( ROK );	
		PreOnu = CurOnu;
		CurOnu = CurOnu->Next;
	}
	
	CurOnu = VOS_Malloc( sizeof(pendingOnu_S),  MODULE_ONU );
	if( CurOnu == NULL )
	{
		ASSERT(0);		
		return ( RERROR );
	}
	VOS_MemZero(CurOnu,sizeof(pendingOnu_S));

	CurOnu->Llid = Llid; 
	VOS_MemCpy( CurOnu->OnuMarkInfor.OnuMark.MacAddr, mac_address, BYTES_IN_MAC_ADDRESS );
	CurOnu->OnuMarkInfor.pendingOnutype = PENDINGONU_EPON;
	CurOnu->Next = NULL;
	CurOnu->counter = 0;
    CurOnu->code = code;
	if( PreOnu != NULL )
		PreOnu->Next = CurOnu;
	else 
		PonPortTable[PonPortIdx].PendingOnu.Next = CurOnu;
#if 0
	onuId=GetOnuIdxByLlid(PonPortIdx,Llid);	/*有可能不对*/
	ret=AddPendingOnuMsgtoSW(PonPortIdx,onuId,mac_address);
	/*sys_console_printf("20100908:test AddPendingOnuMsgtoSW ret=%d\r\n",ret);*/
#endif
	return( ROK );
}


static int __AddPendinggponOnu( short int PonPortIdx, short int Llid, unsigned  char *SN, PendingReason_S code)
{
	pendingOnu_S *CurOnu, *PreOnu;	
	short int PonChipType;
	
	/*short int  onuId;
	int ret;*/

	/*CHECK_PON_RANGE*/

	/* added by chenfj 2007-04-12 当限制ONU注册时，同时关闭其数据通道*/
	
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
		
    CTC_SetOnuUniPort( PonPortIdx, Llid, 1, 0 );
	/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

	CurOnu = PonPortTable[PonPortIdx].PendingOnu.Next;
	PreOnu = NULL;

	while( CurOnu != NULL )
	{
		if(CurOnu->Llid == Llid ) return( ROK );	
		PreOnu = CurOnu;
		CurOnu = CurOnu->Next;
	}
	
	CurOnu = VOS_Malloc( sizeof(pendingOnu_S),  MODULE_ONU );
	VOS_MemZero(CurOnu,sizeof(pendingOnu_S));
	if( CurOnu == NULL )
	{
		ASSERT(0);		
		return ( RERROR );
	}
	CurOnu->Llid = Llid; 
	VOS_MemCpy( CurOnu->OnuMarkInfor.OnuMark.serial_no, SN, GPON_ONU_SERIAL_NUM_STR_LEN-1 );
	CurOnu->OnuMarkInfor.pendingOnutype = PENDINGONU_GPON;
	CurOnu->Next = NULL;
	CurOnu->counter = 0;
    CurOnu->code = code;
	if( PreOnu != NULL )
		PreOnu->Next = CurOnu;
	else 
		PonPortTable[PonPortIdx].PendingOnu.Next = CurOnu;
#if 0
	onuId=GetOnuIdxByLlid(PonPortIdx,Llid);	/*有可能不对*/
	ret=AddPendingOnuMsgtoSW(PonPortIdx,onuId,mac_address);
	/*sys_console_printf("20100908:test AddPendingOnuMsgtoSW ret=%d\r\n",ret);*/
#endif
	return( ROK );
}

int AddPendingOnu( short int PonPortIdx, short int OnuIdx, short int Llid, unsigned  char *mac_address, PendingReason_S code)
{
	int ret = RERROR;
	int onuEntry;
    /*unsigned char reg_flag  = 0;
    unsigned char mac_addr[6];*/
  	short int slot;
	if( !OLT_LOCAL_ISVALID(PonPortIdx) )
		return ret;
	slot = GetCardIdxByPonChip(PonPortIdx);
	if( ONU_IDX_ISVALID(OnuIdx) )
	{
        short int firstllid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
        if(firstllid == Llid)
        {
            /* B--modified by liwei056@2012-8-23 for D15700 */
#if 0
            /* B--added by liwei056@2012-8-2 for D15534 */
            /* 清空之前的注册状态，但是要为业务激活保留其接口*/
            ClearOnuRunningData(PonPortIdx, OnuIdx, 1);
            /* E--added by liwei056@2012-8-2 for D15534 */
#else
            /* 清空之前的注册状态，包括释放接口*/
            ClearOnuRunningData(PonPortIdx, OnuIdx, 0);
#endif
            /* E--modified by liwei056@2012-8-23 for D15700 */
        
    		/* modified by xieshl 20111202, 问题单13959 */
    		onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
#if 1        
    		resetOnuOperStatusAndUpTime( onuEntry, ONU_OPER_STATUS_PENDING );
    		/* modified by xieshl 20110520, 6900 PON 板上部分可能已经有效了，应该同步到主控，防止数据不一致,问题单12857 */
    		if ( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
    		{
    			OnuMgtSyncDataSend_Register( PonPortIdx, OnuIdx );			/* modified by xieshl 20111129, 增加主备同步 */
    		}
#else
            ONU_MGMT_SEM_TAKE;
            reg_flag = OnuMgmtTable[onuEntry].RegisterFlag == ONU_FIRST_REGISTER_FLAG?1:0;  
            VOS_MemCpy(mac_addr, mac_address, 6);
            ONU_MGMT_SEM_GIVE;

            if(reg_flag)
                InitOneOnuByDefault(PonPortIdx, OnuIdx);
            else
            {
                ClearOnuRunningData(PonPortIdx, OnuIdx, 1);
        		resetOnuOperStatusAndUpTime( onuEntry, ONU_OPER_STATUS_PENDING );

        		if ( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
        		{
        			OnuMgtSyncDataSend_Register( PonPortIdx, OnuIdx );
        		}
            }
#endif
        }
	}
	if(SYS_MODULE_IS_GPON(slot))
	{
		VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
		ret = __AddPendinggponOnu(PonPortIdx, Llid, mac_address, code);
		VOS_SemGive(OnuPendingDataSemId);
		sys_console_printf("pon%d/%d onu%16s register-pending\r\n  ",  GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),mac_address);
	}/**/
	else
	{
		VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
		ret = __AddPendingOnu(PonPortIdx, Llid, mac_address, code);
		VOS_SemGive(OnuPendingDataSemId);
		sys_console_printf( "pon%d/%d onu(%02x%02x.%02x%02x.%02x%02x) register-pending\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),
		mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5] );

	}
    /* 注册失败的ONU，也应该尽量使用硬心跳 */
    if( SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER )    
        sendOnuExtOamOverMsg(PonPortIdx, Llid, mac_address, CTC_DISCOVERY_STATE_UNSUPPORTED);

    /* 注册失败的ONU，尽量不占用带宽 */
	SetOnuPendingDefaultBW(PonPortIdx, Llid);
	return( ret );
}

/* added by xieshl 20111206, 基于ONU MAC地址查找pending队列是否存在ONU，问题单14007 */
int SearchPendingOnu( short int PonPortIdx, char *onuMac )
{
	pendingOnu_S *CurOnu;
	short int ret = RERROR;
	/*UCHAR macAddr[6];*/
	
	CHECK_PON_RANGE;

	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	CurOnu = PonPortTable[PonPortIdx].PendingOnu.Next;

	while(CurOnu != NULL )
	{
		if( (onuMac != NULL) && MAC_ADDR_IS_EQUAL(CurOnu->OnuMarkInfor.OnuMark.MacAddr, onuMac) )
		{
			ret = ROK;
			break;
		}
		CurOnu = CurOnu->Next;
	}
	VOS_SemGive(OnuPendingDataSemId);

	return ret;
}

/* modified by xieshl 20110525, 统一提供删除API */
/* 删除本板上pending队列中指定onu，如果指定llid，onuMac可置NULL；如果指定onu mac地址，llid可置INVALID_LLID；
    如果llid和onuMac都输入，则只要有一个匹配即删除 */
int UpdatePendingOnu( short int PonPortIdx, short int llid, char *onuMac )
{
	pendingOnu_S *CurOnu, *PreOnu, *nextOnu;
	short int OnuIdx = RERROR, ret = RERROR;
	short int slot=0;
	UCHAR macAddr[GPON_ONU_SERIAL_NUM_STR_LEN];
	VOS_MemZero(macAddr,GPON_ONU_SERIAL_NUM_STR_LEN);
	CHECK_PON_RANGE;

	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	CurOnu = PonPortTable[PonPortIdx].PendingOnu.Next;
	PreOnu = NULL;
	slot = GetCardIdxByPonChip(PonPortIdx);
	if(SYS_MODULE_IS_GPON(slot))
	{
		while(CurOnu != NULL )
		{
			if( ((onuMac != NULL) && (VOS_MemCmp(CurOnu->OnuMarkInfor.OnuMark.serial_no, onuMac,GPON_ONU_SERIAL_NUM_STR_LEN-1) == 0)) ||
				((llid != INVALID_LLID) && (llid == CurOnu->Llid)) )
			{
				if( ret == RERROR )
				{
					VOS_MemCpy( macAddr, CurOnu->OnuMarkInfor.OnuMark.serial_no,GPON_ONU_SERIAL_NUM_STR_LEN-1);
					ret = ROK;
				}
				nextOnu = CurOnu->Next;
				if( PreOnu == NULL )
					PonPortTable[PonPortIdx].PendingOnu.Next = nextOnu;
				else
					PreOnu->Next = nextOnu;

				VOS_Free( (void *)CurOnu );
				CurOnu = nextOnu;
			}
			else
			{
				PreOnu = CurOnu;
				CurOnu = CurOnu->Next;
			}
		}
	}
	else
	{
		while(CurOnu != NULL )
		{
			if( ((onuMac != NULL) && (VOS_MemCmp(CurOnu->OnuMarkInfor.OnuMark.MacAddr, onuMac,6) == 0)) ||
				((llid != INVALID_LLID) && (llid == CurOnu->Llid)) )
			{
				if( ret == RERROR )
				{
					VOS_MemCpy( macAddr, CurOnu->OnuMarkInfor.OnuMark.MacAddr,6);
					ret = ROK;
				}
				nextOnu = CurOnu->Next;
				if( PreOnu == NULL )
					PonPortTable[PonPortIdx].PendingOnu.Next = nextOnu;
				else
					PreOnu->Next = nextOnu;

				VOS_Free( (void *)CurOnu );
				CurOnu = nextOnu;
			}
			else
			{
				PreOnu = CurOnu;
				CurOnu = CurOnu->Next;
			}
		}
	}
	VOS_SemGive(OnuPendingDataSemId);

	
	/* modified by xieshl 20110525, 6900 PON 板上部分可能已经有效了，应该同步到主控，防止数据不一致,问题单12857 */
	if ((ret == ROK) &&( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER || SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER))
	{
		if(SYS_MODULE_IS_GPON(slot))
			OnuIdx= GetOnuIdxBySnPerPon(PonPortIdx, macAddr );
		else
			OnuIdx = GetOnuIdxByMacPerPon( PonPortIdx, macAddr );
		if( OnuIdx != RERROR )
		{	
			/* modified by xieshl 20111206, GFA6700上没有消除ONU pending状态 */
			resetOnuOperStatusAndUpTime( PonPortIdx * MAXONUPERPON + OnuIdx, ONU_OPER_STATUS_DOWN );
			OnuMgtSyncDataSend_Register( PonPortIdx, OnuIdx );

			/*added  2014-08-05*/
			if( PonPortTable[PonPortIdx].SignalLossFlag == V2R1_DISABLE )
			{
				if( OnRegOnuCounter(PonPortIdx) < 1 )
				{
					Trap_PonPortSignalLoss( PonPortIdx, V2R1_ENABLE );
				}
			}            
		}
	}

	return( ROK );
}

int UpdatePendingConfOnu( short int PonPortIdx, short int llid, char *onuMac )
{
	pendingOnu_S *CurOnu, *PreOnu, *nextOnu;
	short int OnuIdx = RERROR, ret = RERROR;
	short int slot=0;
	UCHAR macAddr[GPON_ONU_SERIAL_NUM_STR_LEN];
	
	CHECK_PON_RANGE;

	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	CurOnu = PonPortTable[PonPortIdx].PendingOnu_Conf.Next;
	PreOnu = NULL;
	slot = GetCardIdxByPonChip(PonPortIdx);
	if(SYS_MODULE_IS_GPON(slot))
	{
		while(CurOnu != NULL )
		{
			if( ((onuMac != NULL) && VOS_MemCmp(CurOnu->OnuMarkInfor.OnuMark.MacAddr, onuMac,GPON_ONU_SERIAL_NUM_STR_LEN)) ||
				((llid != INVALID_LLID) && (llid == CurOnu->Llid)) )
			{
				if( ret == RERROR )
				{
					VOS_MemCpy( macAddr, CurOnu->OnuMarkInfor.OnuMark.MacAddr,GPON_ONU_SERIAL_NUM_STR_LEN-1);
					ret = ROK;
				}
				nextOnu = CurOnu->Next;
				if( PreOnu == NULL )
					PonPortTable[PonPortIdx].PendingOnu_Conf.Next = nextOnu;				
				else
					PreOnu->Next = nextOnu;

				VOS_Free( (void *)CurOnu );
				/*CurOnu->Llid = 0;
				CurOnu->Next = NULL;
				CurOnu->otherPonIdx = MAXPON;*/

				CurOnu = nextOnu;
			}
			else
			{
				PreOnu = CurOnu;
				CurOnu = CurOnu->Next;
			}
		}
	}
	else
	{
		while(CurOnu != NULL )
		{
			if( ((onuMac != NULL) && VOS_MemCmp(CurOnu->OnuMarkInfor.OnuMark.MacAddr, onuMac,6)) ||
				((llid != INVALID_LLID) && (llid == CurOnu->Llid)) )
			{
				if( ret == RERROR )
				{
					VOS_MemCpy( macAddr, CurOnu->OnuMarkInfor.OnuMark.MacAddr,6);
					ret = ROK;
				}
				nextOnu = CurOnu->Next;
				if( PreOnu == NULL )
					PonPortTable[PonPortIdx].PendingOnu_Conf.Next = nextOnu;				
				else
					PreOnu->Next = nextOnu;

				VOS_Free( (void *)CurOnu );
				/*CurOnu->Llid = 0;
				CurOnu->Next = NULL;
				CurOnu->otherPonIdx = MAXPON;*/

				CurOnu = nextOnu;
			}
			else
			{
				PreOnu = CurOnu;
				CurOnu = CurOnu->Next;
			}
		}
	}
	VOS_SemGive(OnuPendingDataSemId);

	
	/* modified by xieshl 20110616, 6900 PON 板上部分可能已经有效了，应该同步到主控，防止数据不一致,问题单12857 */
	if (( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER || SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER)&& (ret == ROK) )
	{
		if(SYS_MODULE_IS_GPON(slot))
		OnuIdx= GetOnuIdxBySnPerPon(PonPortIdx, macAddr );
		else
		OnuIdx = GetOnuIdxByMacPerPon( PonPortIdx, macAddr );
			
		if( OnuIdx != RERROR )
		{
        	resetOnuOperStatusAndUpTime( PonPortIdx * MAXONUPERPON + OnuIdx, ONU_OPER_STATUS_DOWN );            
			OnuMgtSyncDataSend_Register( PonPortIdx, OnuIdx );
		}
	}

	return( ROK );
}

int UpdateAllPendingOnu( short int PonPortIdx )
{
	pendingOnu_S *CurOnu, *NextOnu;
	
	CHECK_PON_RANGE;

	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	CurOnu = PonPortTable[PonPortIdx].PendingOnu.Next;

	while(CurOnu != NULL )
	{
		NextOnu = CurOnu->Next;
        /*added 2014-08-05*/
    	if (SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER || SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER)
    	{	
    		short int OnuIdx;
    		if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
				OnuIdx = GetOnuIdxBySnPerPon( PonPortIdx, CurOnu->OnuMarkInfor.OnuMark.serial_no);
			else
    			OnuIdx = GetOnuIdxByMacPerPon( PonPortIdx, CurOnu->OnuMarkInfor.OnuMark.MacAddr);
    			
    		if( OnuIdx != RERROR )
    		{	
                /* modified by xieshl 20111206, GFA6700上没有消除ONU pending状态 */
    	        resetOnuOperStatusAndUpTime( PonPortIdx * MAXONUPERPON + OnuIdx, ONU_OPER_STATUS_DOWN );
    			OnuMgtSyncDataSend_Register( PonPortIdx, OnuIdx );
                
            	if( PonPortTable[PonPortIdx].SignalLossFlag == V2R1_DISABLE )
            	{
            		if( OnRegOnuCounter(PonPortIdx) < 1 )
            		{
            			Trap_PonPortSignalLoss( PonPortIdx, V2R1_ENABLE );
            		}
            	}                
    		}
    	}
		VOS_Free( (void *)CurOnu );
		CurOnu = NextOnu;
	}
	PonPortTable[PonPortIdx].PendingOnu.Next = NULL;				

	VOS_SemGive(OnuPendingDataSemId);

	return( ROK );
}

int UpdateAllPendingConfOnu( short int PonPortIdx )
{
	pendingOnu_S *CurOnu, *NextOnu;
	
	CHECK_PON_RANGE

	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );

	CurOnu = PonPortTable[PonPortIdx].PendingOnu_Conf.Next;
	while(CurOnu != NULL )
	{
		NextOnu = CurOnu->Next;

		/*CurOnu->Llid = 0;
		CurOnu->Next = NULL;
		CurOnu->otherPonIdx = MAXPON;*/
		VOS_Free( (void *)CurOnu );

		CurOnu = NextOnu;
	}
	PonPortTable[PonPortIdx].PendingOnu_Conf.Next = NULL;				
	VOS_SemGive(OnuPendingDataSemId);

	return( ROK );
}

int GetPendingOnu( short int PonPortIdx, pendingOnuList_t *onuList )
{
	pendingOnu_S *CurOnu, *PreOnu;
	
	CHECK_PON_RANGE
	if( onuList == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	
	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	CurOnu = PonPortTable[PonPortIdx].PendingOnu.Next;
	PreOnu = NULL;

	onuList->pendingCount = 0;
	while(CurOnu != NULL )
	{
		onuList->pendingList[onuList->pendingCount].Llid = CurOnu->Llid;
		VOS_MemCpy( onuList->pendingList[onuList->pendingCount].OnuMarkInfor.OnuMark.MacAddr, CurOnu->OnuMarkInfor.OnuMark.MacAddr, GPON_ONU_SERIAL_NUM_STR_LEN-1);
		onuList->pendingList[onuList->pendingCount].code = CurOnu->code;
		onuList->pendingList[onuList->pendingCount].OnuMarkInfor.pendingOnutype = CurOnu->OnuMarkInfor.pendingOnutype;
		onuList->pendingCount++;
		
		PreOnu = CurOnu;
		CurOnu = CurOnu->Next;
	}

	CurOnu = PonPortTable[PonPortIdx].PendingOnu_Conf.Next;
	PreOnu = NULL;

	onuList->conflictCount = 0;
	while(CurOnu != NULL )
	{
		onuList->conflictList[onuList->conflictCount].otherPonIdx = CurOnu->otherPonIdx;
		onuList->conflictList[onuList->conflictCount].Llid = CurOnu->Llid;
		VOS_MemCpy( onuList->conflictList[onuList->conflictCount].OnuMarkInfor.OnuMark.MacAddr, CurOnu->OnuMarkInfor.OnuMark.MacAddr, GPON_ONU_SERIAL_NUM_STR_LEN-1);
		onuList->pendingList[onuList->pendingCount].OnuMarkInfor.pendingOnutype = CurOnu->OnuMarkInfor.pendingOnutype;
		onuList->conflictCount++;
		
		PreOnu = CurOnu;
		CurOnu = CurOnu->Next;
	}

	VOS_SemGive(OnuPendingDataSemId);

	return( ROK );
}
/*int DelPendingOnu( short int PonPortIdx, short int Llid )
{
	int ret;
	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	ret=OnuMgt_DelPendingOnu(PonPortIdx, Llid);
	VOS_SemGive(OnuPendingDataSemId);
	return( ret );
}
 
static int DelAllPendingOnu_1( short int PonPortIdx )
{
	pendingOnu_S *CurOnu, *PreOnu;

	CHECK_PON_RANGE

	CurOnu = PonPortTable[PonPortIdx].PendingOnu.Next;
	while( CurOnu != NULL )
	{
		PreOnu = CurOnu;
		CurOnu = CurOnu->Next;
		OnuMgt_DelPendingOnu(PonPortIdx, PreOnu->Llid );
		
	}
	
	PonPortTable[PonPortIdx].PendingOnuCounter = 0;
	return( ROK );
}

int DelAllPendingOnu( short int PonPortIdx )
{
	int ret;

	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	ret = DelAllPendingOnu_1(PonPortIdx);
	VOS_SemGive(OnuPendingDataSemId);

	return( ret );
}*/

int activeOneLocalPendingOnu( short int PonPortIdx, short int OnuIdx )
{
	pendingOnu_S *CurOnu, *NextOnu, *PreOnu;
	int ret = RERROR;
	UCHAR MacAddr[GPON_ONU_SERIAL_NUM_STR_LEN];
	short int llid = -1;
	short int slot = 0;
	slot = GetCardIdxByPonChip(PonPortIdx);
	CHECK_ONU_RANGE;
	ONU_MGMT_SEM_TAKE;
	if(SYS_MODULE_IS_GPON(slot))
	{
		VOS_MemCpy( MacAddr, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceSerial_No, GPON_ONU_SERIAL_NUM_STR_LEN );
	}
	else
	{
		VOS_MemCpy( MacAddr, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS );
	}
	ONU_MGMT_SEM_GIVE;
	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	PreOnu = NULL;
	CurOnu = PonPortTable[PonPortIdx].PendingOnu.Next;
	while( CurOnu != NULL )
	{		
		NextOnu = CurOnu->Next;

		if( VOS_StrCmp(CurOnu->OnuMarkInfor.OnuMark.MacAddr, MacAddr) )
		{
			if( PreOnu == NULL )
				PonPortTable[PonPortIdx].PendingOnu.Next = CurOnu->Next;				
			else
				PreOnu->Next = CurOnu->Next;

			ret = ROK;
			llid = CurOnu->Llid;
			/*PAS_deregister_onu( PonPortIdx, CurOnu->Llid, FALSE );*/

			VOS_Free( (void *)CurOnu );
			break;
		}

		PreOnu = CurOnu;
		CurOnu = NextOnu;
	}	

	if( ret == RERROR )
	{
		PreOnu = NULL;
		CurOnu = PonPortTable[PonPortIdx].PendingOnu_Conf.Next;
		while( CurOnu != NULL )
		{		
			NextOnu = CurOnu->Next;

			if( VOS_StrCmp(CurOnu->OnuMarkInfor.OnuMark.MacAddr, MacAddr) )
			{
				if( PreOnu == NULL )
					PonPortTable[PonPortIdx].PendingOnu_Conf.Next = CurOnu->Next;				
				else
					PreOnu->Next = CurOnu->Next;

				ret = ROK;
				llid = CurOnu->Llid;
				/*PAS_deregister_onu( PonPortIdx, CurOnu->Llid, FALSE );*/

				VOS_Free( (void *)CurOnu );
				break;
			}

			PreOnu = CurOnu;
			CurOnu = NextOnu;
		}
	}	

	VOS_SemGive(OnuPendingDataSemId);

	if( (ret == ROK) && (llid != -1) )
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		OLT_DeregisterLLID( PonPortIdx, llid, FALSE );
		#else
		PAS_deregister_onu( PonPortIdx, llid, FALSE );
		#endif

	return ret;
}

int ActivatePendingOnu( short int PonPortIdx )
{
	int ret;
	
#if 0
	/*VOS_TaskLock();*/
	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	ret = ActivatePendingOnu_1(PonPortIdx);
	/*VOS_TaskUnlock();*/
	VOS_SemGive(OnuPendingDataSemId);
#else
	/* add by sxh here 20100908 */
	ret = OLT_ActivePendingOnu(PonPortIdx);
#endif
	return( ret );
}	

int ActivateOnePendingOnu( short int PonPortIdx, unsigned char *MacAddr )
{
	int ret;

#if 0
	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	/*VOS_TaskLock();*/
	ret = ActivateOnePendingOnu_1(PonPortIdx, MacAddr);
	/*VOS_TaskUnlock();*/
	VOS_SemGive(OnuPendingDataSemId);
#else
	/* 20100913,add by sxh here */
	ret = OLT_ActiveOnePendingOnu(PonPortIdx, MacAddr);
#endif
	
	return( ret );
}

int  ActivatePendingOnuMsg( short int PonPortIdx )
{
	unsigned long aulMsg[4] = { MODULE_PON, FC_ACTIVE_PENDING_ONU, 0, 0};

	CHECK_PON_RANGE

	aulMsg[2] = PonPortIdx;

	/*
	if( g_PonUpdate_Queue_Id == 0 )  g_PonUpdate_Queue_Id = VOS_QueCreate( MAXPONUPDATEMSGNUM, VOS_MSG_Q_FIFO);
	*/

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_ASSERT(0);
		/*sys_console_printf("  error:VOS send message err to ponUpdate Que\r\n"  );*/
		}
	return( ROK );
}

	/* modified by chenfj 2008-6-12
	修改ONU注册认证,在非CTC模式下,OLT侧软件启动定时器, 模拟ONU侧的注册
	静默时间(1 分钟)
	*/
static int __ScanAuthenticationPendingOnu(short int PonPortIdx )
{
	pendingOnu_S *CurOnu, *NextOnu;
	pendingOnu_S *PreOnu; 
	short int PonChipType;

	CHECK_PON_RANGE

   
		PonChipType = GetPonChipTypeByPonPort(PonPortIdx);

	if(OLT_PONCHIP_ISINVALID(PonChipType))		
		return( RERROR );

	/*VOS_TaskLock();*/
    
    /*begin: BUG 38247/38302, 优化代码实现，解决pending队列始终为空的问题，mod by @liuyh, 2017-5-17*/
	CurOnu = PonPortTable[PonPortIdx].PendingOnu.Next;
	if(CurOnu == NULL ) return( ROK );
    
    PreOnu = &PonPortTable[PonPortIdx].PendingOnu;
	NextOnu = NULL;
    
	while(CurOnu != NULL )
	{        
		CurOnu->counter += 2;
        
		if(CurOnu->counter >= /*(MINUTE_1/SECOND_1)*/UpdatePendingQueueTime/2
            && CurOnu->code != PENDING_REASON_CODE_ONU_AUTH_FAIL)
		{
#if 1
			OLT_DeregisterLLID( PonPortIdx, CurOnu->Llid, FALSE );
#else
			PAS_deregister_onu( PonPortIdx, CurOnu->Llid, FALSE );
#endif
			/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

            NextOnu = CurOnu->Next;
            
			VOS_MemSet( CurOnu, 0, sizeof(pendingOnu_S ) );
			VOS_Free( (void *)CurOnu );

            CurOnu = NextOnu;
            PreOnu->Next = CurOnu;
		}
        else
        {
            PreOnu = CurOnu;
            CurOnu = CurOnu->Next;            
        }
	}
    /*end: mod by @liuyh, 2017-5-17*/
    
	/*VOS_TaskUnlock();*/
	return( ROK );
}
	
int ScanAuthenticationPendingOnu(short int PonPortIdx )
{
	int ret;
	
	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	ret = __ScanAuthenticationPendingOnu(PonPortIdx);
	VOS_SemGive(OnuPendingDataSemId);

	return(ret);
}

int ScanAuthenticationPendingOnuAll()
{
	unsigned long OnuAuthEnable ;
	short int PonPortIdx;

	getOnuAuthEnable(0,0, &OnuAuthEnable );
	/*if( OnuAuthEnable == V2R1_ONU_AUTHENTICATION_DISABLE ) return( ROK );*/

	for(PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx ++ )
	{
		if( OLTAdv_IsExist(PonPortIdx ) )
		{
			ScanAuthenticationPendingOnu( PonPortIdx);			
		}
	}

	return( ROK );
}

/*****************************************************
 *
 *    Function: AddPendingOnu_conf( short int PonPortIdx, short int Llid , short int PonPortIdx_conf)
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  
 *    Desc:   当ONU注册时,若ONU注册冲突,则这个ONU就不能被准许进入网络;
 *               此时ONU进入pending状态;当与之发生冲突的ONU被删除时,再激活
 *
 *    Return: 
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
static int __AddPendingOnu_conf( short int PonPortIdx, short int Llid , unsigned char *macAddress, short int  PonPortIdx_conf)
{
	pendingOnu_S *CurOnu, *PreOnu;	
	/*int Entry;*/
	short int PonChipType;
	
	/*short int  onuId;
	int ret;*/

	CHECK_PON_RANGE

	/* added by chenfj 2007-04-12 当限制ONU注册时，同时关闭其数据通道*/
	
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	
	if(OLT_PONCHIP_ISPAS(PonChipType))
	{
	    CTC_SetOnuUniPort( PonPortIdx, Llid, 1, 0 );
	}/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	else{ /* other pon chip handler */
	}

	if( macAddress == NULL ) return( RERROR );
	/* modified by xieshl 20110202, PON板上应该存全局编号，以便主控识别 */
	if(( PonPortIdx_conf < 0) ||(PonPortIdx_conf >= /*MAXPON*/MAXGLOBALPON ))
	{
		sys_console_printf("\r\nerror: PonPort %d out of range\r\n", PonPortIdx_conf );
		return ( RERROR );
	}

	CurOnu = PonPortTable[PonPortIdx].PendingOnu_Conf.Next;
	PreOnu = NULL;
	
	while( CurOnu != NULL )
	{
		if(CurOnu->Llid == Llid ) return( ROK );	
		PreOnu = CurOnu;
		CurOnu = CurOnu->Next;
	}
	
#ifdef __pending_onu_list__	
	Entry = FindFreeEntryForConfOnu();
	if (Entry == RERROR )
	{
		sys_console_printf("\r\n No Free Entry to save conflict Onu \r\n");
		return ( RERROR );
	}
	CurOnu = &(ConfOnuPendingQueue[Entry]);
#else
	CurOnu = VOS_Malloc( sizeof(pendingOnu_S), MODULE_ONU );
	if( CurOnu == NULL )
		return RERROR;
#endif
	CurOnu->otherPonIdx = PonPortIdx_conf;
	CurOnu->Llid = Llid; 
	VOS_MemCpy( CurOnu->OnuMarkInfor.OnuMark.MacAddr, macAddress, BYTES_IN_MAC_ADDRESS );
	CurOnu->counter = 0;
	CurOnu->Next = NULL;

	if( PreOnu != NULL )
		PreOnu->Next = CurOnu;
	else 
		PonPortTable[PonPortIdx].PendingOnu_Conf.Next = CurOnu;

	/*ret=AddPendingOnu_Conf_MsgtoSW(PonPortIdx,Llid,macAddress,PonPortIdx_conf );*/

	return( ROK );
}

int AddPendingOnu_conf( short int PonPortIdx, short int Llid , unsigned char *mac_address, short int  PonPortIdx_conf)
{
	int ret ;

	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	ret = __AddPendingOnu_conf( PonPortIdx, Llid, mac_address, PonPortIdx_conf );
	VOS_SemGive(OnuPendingDataSemId);

    /* 注册失败的ONU，也应该尽量使用硬心跳 */
    if( SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER )    
        sendOnuExtOamOverMsg(PonPortIdx, Llid, mac_address, CTC_DISCOVERY_STATE_UNSUPPORTED);

    /* 注册失败的ONU，尽量不占用带宽 */
    SetOnuPendingDefaultBW(PonPortIdx, Llid);

	sys_console_printf( "pon%d/%d onu(%02x%02x.%02x%02x.%02x%02x) register-pending, as conflict with pon%d/%d\r\n",
			GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),
			mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5],
			GetGlobalCardIdxByPonChip(PonPortIdx_conf), GetGlobalPonPortByPonChip(PonPortIdx_conf) );
	return( ret );
}

int ActivatePendingOnu_conf( short int PonPortIdx, unsigned char *macAddress )
{
	int ret ;

#if 0
	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	/*VOS_TaskLock();*/
	ret = ActivatePendingOnu_conf_1(PonPortIdx, macAddress);
	/*VOS_TaskUnlock();*/
	VOS_SemGive(OnuPendingDataSemId);
#else
	/* 20100913, add by sxh here */
	ret = OLT_ActiveOneConfPendingOnu(OLT_ID_ALL, PonPortIdx, macAddress);
#endif

	return( ret );
}

#if 0
static int ActivatePendingOnu_conf_all_1( short int PonPortIdx )
{
	short int PonIdx, PonChipType, Curport;
	pendingOnu_S *CurOnu, *NextOnu;
	
	CHECK_PON_RANGE
		
	for( Curport = 0; Curport < MAXPON; Curport++)
		{

		if ( Curport == PonPortIdx ) continue;
		
		CurOnu  = PonPortTable[Curport].PendingOnu_Conf.Next;
		/*modified by chenfj 2006-12-21
		#3446 问题单:将PON板更换槽位后，删除以前PON口对应的ONU，不能在新的PON口上自动注册
		*/
		PonChipType = GetPonChipTypeByPonPort(Curport);
		if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
			{
			/*PonChipVer = PonChipType;*/
			PonChipType = PONCHIP_PAS;
			}

		while( CurOnu != NULL )
			{		
			
			NextOnu = CurOnu->Next;
			
			PonIdx = CurOnu->otherPonIdx ;
			/*PonChipType = GetPonChipTypeByPonPort(PonIdx);*/
			
			if(( PonIdx< 0) ||(PonIdx >= MAXPON ))
				{
				sys_console_printf("\r\nerror 2: PonPort %d out of range\r\n", (PonIdx+1) );
				continue;
				}
			
			if( PonIdx == PonPortIdx )
				{		
				if( EVENT_REGISTER == V2R1_ENABLE)
					sys_console_printf(" pon %d llid %d is to be activated \r\n", PonIdx, CurOnu->Llid );
				if( PonChipType == PONCHIP_PAS )
					{
					PAS_deregister_onu( Curport, CurOnu->Llid, FALSE );
					
					/*
					CurOnu->Llid = 0;
					CurOnu->Next = NULL;
					CurOnu->otherPonIdx = MAXPON;
					*/
					}
				else{
					}
				DelPendingOnu_conf_1( Curport, CurOnu->Llid);
				}
			CurOnu = NextOnu;
			}
		}
	return( ROK );
}
#endif
 
int ActivatePendingOnu_conf_all( short int PonPortIdx )
{
	int ret;

#if 0
	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	/*VOS_TaskLock();*/
	ret = ActivatePendingOnu_conf_all_1(PonPortIdx);
	/*VOS_TaskUnlock();*/
	VOS_SemGive(OnuPendingDataSemId);
#else
	ret = OLT_ActiveConfPendingOnu(OLT_ID_ALL, PonPortIdx);
#endif

	return( ret );
}	
int  ActivateGponPendingOnuMsg_conf( short int PonPortIdx, unsigned char *SN )
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_ACTIVE_PENDING_ONU_CONF, 0, 0};
	unsigned char *Addr_buf;

	CHECK_PON_RANGE

	aulMsg[2] = PonPortIdx;
	if(SN == NULL ) return( RERROR );

	Addr_buf = VOS_Malloc( GPON_ONU_SERIAL_NUM_STR_LEN,  MODULE_PON );
	if( Addr_buf == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}

	VOS_MemCpy( Addr_buf, SN, GPON_ONU_SERIAL_NUM_STR_LEN );

	aulMsg[3] = (int )Addr_buf;

	/*
	if( g_PonUpdate_Queue_Id == 0 )  g_PonUpdate_Queue_Id = VOS_QueCreate( MAXPONUPDATEMSGNUM, VOS_MSG_Q_FIFO);
	*/

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_ASSERT(0);
		VOS_Free( Addr_buf );
		/*sys_console_printf("  error:VOS send message err to ponUpdate Que\r\n"  );*/
		}
	return( ROK );
}
int  ActivatePendingOnuMsg_conf( short int PonPortIdx, unsigned char *macAddress )
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_ACTIVE_PENDING_ONU_CONF, 0, 0};
	unsigned char *Addr_buf;

	CHECK_PON_RANGE

	aulMsg[2] = PonPortIdx;
	if(macAddress == NULL ) return( RERROR );

	Addr_buf = VOS_Malloc( BYTES_IN_MAC_ADDRESS,  MODULE_PON );
	if( Addr_buf == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}

	VOS_MemCpy( Addr_buf, macAddress, BYTES_IN_MAC_ADDRESS );

	aulMsg[3] = (int )Addr_buf;

	/*
	if( g_PonUpdate_Queue_Id == 0 )  g_PonUpdate_Queue_Id = VOS_QueCreate( MAXPONUPDATEMSGNUM, VOS_MSG_Q_FIFO);
	*/

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_ASSERT(0);
		VOS_Free( Addr_buf );
		/*sys_console_printf("  error:VOS send message err to ponUpdate Que\r\n"  );*/
		}
	return( ROK );
}

int  ActivatePendingOnuMsg_conf_all( short int PonPortIdx )
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_ACTIVE_PENDING_ONU_CONF_ALL, 0, 0};

	CHECK_PON_RANGE

	aulMsg[2] = PonPortIdx;

	/*
	if( g_PonUpdate_Queue_Id == 0 )  g_PonUpdate_Queue_Id = VOS_QueCreate( MAXPONUPDATEMSGNUM, VOS_MSG_Q_FIFO);
	*/

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_ASSERT(0);
		sys_console_printf("  error:VOS send message err to ponUpdate Que\r\n"  );
		}
	return( ROK );
}

/*****************************************************
 *
 *    Function: GetPonPortInvalidOnuList( unsigned short int PonPortIdx, MacTable_S *InvalidOnuList )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                  MacTable_S *InvalidOnuList  -- the invalid onu mac address list
 *    Desc:   
 *
 *    Return:  the num of the invalid onu mac address
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 int  GetPonPortInvalidOnuList( short int PonPortIdx, MacTable_S *InvalidOnuList )
{
	CHECK_PON_RANGE
	if( PonPortIdx >= MAXPON )
		{
		sys_console_printf("Error, out ot bounds Pon port %d (GetPonPortInvalidOnuList())\r\n", PonPortIdx );
		return( RERROR );
		}
	
	VOS_MemCpy( InvalidOnuList , PonPortTable[PonPortIdx].InvalidOnuTablePtr, sizeof( MacTable_S )*PonPortTable[PonPortIdx].InvalidOnuNum );

	return ( PonPortTable[PonPortIdx].InvalidOnuNum );
}

/*****************************************************
 *
 *    Function: GetPonPortIdx( unsigned short int PonChipIdx )
 *
 *    Param:   short int PonChipIdx -- the Pon chip index
 *                  
 *    Desc:   
 *
 *    Return:  the pon port index of the pon chip
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 short int  GetPonPortIdx( short int PonPortIdx )
{
	CHECK_PON_RANGE
		
	if( PonPortIdx >= MAXPONCHIP )
		{
		sys_console_printf("Error, out ot bounds Pon chip %d (GetPonPortIdx())\r\n", (PonPortIdx+1));
		return( RERROR );
		}
	return( PonPortIdx );
}

 short int  GetPonChipIdx( short int PonPortIdx )
{
	CHECK_PON_RANGE
		
	return( PonPortIdx );
}

/*****************************************************
 *
 *    Function: GetOnuMacAddrFromLlid( short int PonPortIdx, short int Llid , unsigned char *MacAddr )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                short int Llid -- the specific LLID
 *                 unsigned char *MacAddr -- output, the pointer of the corresponding ONU mac 
 *    Desc:   
 *
 *    Return:  ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int  GetOnuMacAddrFromLlid(  short int PonPortIdx, short int  Llid , unsigned char *MacAddr )
{
	short int OnuIdx;
	int len;

	OnuIdx = GetOnuIdxByLlid(PonPortIdx, Llid);

	CHECK_ONU_RANGE
	if(MacAddr == NULL )
		{
		/*sys_console_printf("error: return parameter pointer is null (GetONuMacAddrFromLlid)\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	return(GetOnuMacAddr(PonPortIdx, OnuIdx,  MacAddr, &len));
	
#if 0
	int result;
	int whichLlid;

	CHECK_LLID_RANGE
	if( MacAddr == NULL ) 
		{
		sys_console_printf("error: return parameter pointer is null ( GetONuMacAddrFromLlid())\r\n" );
		return( RERROR );
		}
	/*
	result = GetPonPortOperStatus(  PonPortIdx );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR )) return( RERROR );
	*/
	if( PonPortIsWorking(PonPortIdx) != TRUE ) return(RERROR );
	
	if( GetPonLlidEntryStatus( PonPortIdx,  Llid) != PONLLID_UP )
		{
		sys_console_printf(" the LLID %d is not in working now ( GetOnuMacAddrFromLlid())\r\n", Llid );
		return( RERROR );
		}
	for(whichLlid =0; whichLlid < MAXLLID; whichLlid++ )
		{
		if( PonLLIDTable[PonPortIdx][whichLlid].LLID_id == Llid )
			{
			VOS_MemCpy( MacAddr, &(PonLLIDTable[PonPortIdx][whichLlid].ONUMAC[0]), BYTES_IN_MAC_ADDRESS );
			return( ROK );
			}
		}
	return( RERROR );
#endif
}

/*****************************************************
 *
 *    Function: GetOnuLlidListFromMacAddr( unsigned char *MacAddr, unsigned short int *PonPortIdx, unsigned char *Llid_List, unsigned char Llid_Num )
 *
 *    Param:  unsigned char *MacAddr --  the pointer of the specific ONU mac 
 *                 short int *PonPortIdx -- output, the corresponding  pon port
 *                 short int *Llid -- output, the corresponding LLID
 *    Desc:   At the assumption one onu has one llid only, such as PAS5001
 *
 *    Return:  ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int  GetOnuLlidListFromMacAddr( unsigned char *MacAddr,  short int *PonPortIdx, short int *Llid)
{
#if 0
	/*int result;*/
	short int  PonPort_id;
	short int  Llid_id;

	if(( PonPortIdx == NULL ) || ( Llid == NULL ))
	{
		/*sys_console_printf("error: return parameter pointer is null ( GetOnuLlidListFromMacAddr())\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
	}
	
	for( PonPort_id = 0; PonPort_id < MAXPON; PonPort_id++ )
	{
		/*
		result = GetPonPortOperStatus(  PonPort_id );
		if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||( result == RERROR )) continue;
		*/
		if( PonPortIsWorking(PonPort_id) != TRUE ) continue;

		for( Llid_id = 0; Llid_id < MAXLLID; Llid_id++ )
		{
			if(( GetPonLlidEntryStatus(PonPort_id,  Llid_id) == PONLLID_UP ) 
				&&( VOS_MemCmp(PonLLIDTable[PonPort_id][Llid_id].ONUMAC, MacAddr, BYTES_IN_MAC_ADDRESS ) == 0 ) )
			{
				*PonPortIdx = PonPort_id;
				*Llid = Llid_id;
				return( ROK );
			}
		}
	}
#endif	
	return( RERROR );
}


/*****************************************************
 *
 *    Function: ScanPonPortMgmtTableTimer()
 *
 *    Param:  none 
 *                 
 *    Desc:  this function is periodically called, to check if the pon is in good status 
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 
int  ScanPonPortMgmtTableTimer()
{
	short int PonPortIdx, PonChipType;
	int counter = 0;
	PAS_olt_test_results_t result;

	return( ROK );

	for(PonPortIdx =0;  PonPortIdx< MAXPONCHIP; PonPortIdx ++ )
		{
		if( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UP )
			{
			PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
            			
			if( OLT_PONCHIP_ISPAS(PonChipType) )
				{
				PonPortTable[PonPortIdx].testCounter-- ;
				if( PonPortTable[PonPortIdx].testCounter == 0 )
					{
					PonPortTable[PonPortIdx].testCounter =( PonPortTable[PonPortIdx].testTimer/V2R1_TIMERTICK);
					/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
					result = OLT_OltSelfTest( PonPortIdx );
					if( result != PAS_OLT_TEST_SUCCEEDED)
						{
						counter = counter + ( 1 << PonPortIdx ); 
#if 0
						/*1 send trap to NMS: if there are alarm existing, send alarm clear Trap to NMS, include pon and all registered ONU */ 

						/*2 clear registered onu running data */
						for(onuIdx = 0; onuIdx < MAXONUPERPON; onuIdx++){

							/*a: shutDown the registered onu */
								
								
							if( OnuMgmtTable[PonPortIdx * MAXONUPERPON + onuIdx].OperStatus == ONU_OPER_STATUS_UP ){
								/*b: send onu deregister msg to NMS */
								Trap_OnuDeregister( PonPortIdx, onuIdx );
				
								/*c: send onu alarm clear msg to NMS */

									
								/*d: clear onu running data */
								ClearOnuRunningData(  PonPortIdx,  onuIdx, 0);		
								}					
							}
							
						/*3 clear pon port running data */
						ClearPonPortRunningData( PonPortIdx );
						PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ONLINE;
						PonChipMgmtTable[PonPortIdx].Err_counter = 0;
						UpdatePonPortNum();						
#endif					
						if( EVENT_TESTPONPORT == V2R1_ENABLE )
							sys_console_printf("\r\n====== %s/port%d status test err ======\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
						PonPortTable[PonPortIdx].testRemain-- ;
						if( PonPortTable[PonPortIdx].testRemain == 0 )
							{
							PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_DOWN;
							/*
							pon_reset_olt( PonPortIdx );
							Add_PonPort(PonPortIdx);
							*/
							PonPortTable[PonPortIdx].testRemain = PonPortConfigDefault.testRemain ;
							/*Trap_PonPortAbnormal( PonPortIdx, ResetCode);*/
							}
						continue;
						
						}
					
					if( PonPortTable[PonPortIdx].testRemain != PonPortConfigDefault.testRemain )
						PonPortTable[PonPortIdx].testRemain = PonPortConfigDefault.testRemain;
					
					}
				}
			else { /* other pon type handler */
				}			
			}
		}

	return( counter );	
}

int ScanPonPortMgmtTableBerFer()
{
	int  ticksNum;
	int  LastedTime;
	short int PonPortIdx;
	
	ticksNum = VOS_GetTick();
	for(PonPortIdx =0;  PonPortIdx< MAXPONCHIP; PonPortIdx ++ )
		{
		if ( PonPortTable[PonPortIdx].PortWorkingStatus != PONPORT_UP ) continue;
		
		if( PonPortTable[PonPortIdx].BerFlag == V2R1_TRAP_SEND )
			{
			LastedTime = ( ticksNum - PonPortTable[PonPortIdx].LastBerAlarmTime ) / ( VOS_TICK_SECOND ); ;
            /* B--modified by liwei056@2010-1-20 for D9624 */
#if 0
			if( LastedTime >= 10/*( V2R1_PON_MONITORING_CYCLE * 3/1000 ) */ ) 
				Trap_PonPortFER( PonPortIdx, 0, V2R1_TRAP_CLEAR );
#else
			if( LastedTime >= SECOND_PON_BER_ALERM_CLEAR_TIME ) 
				Trap_PonPortBER( PonPortIdx, 0, V2R1_TRAP_CLEAR );
#endif
            /* E--modified by liwei056@2010-1-20 for D9624 */
			}
		
		if( PonPortTable[PonPortIdx].FerFlag == V2R1_TRAP_SEND )
			{
			LastedTime = ( ticksNum - PonPortTable[PonPortIdx].LastFerAlarmTime ) / ( VOS_TICK_SECOND ); ;
            /* B--modified by liwei056@2010-1-20 for D9624 */
#if 0
			if( LastedTime >=  10/*( V2R1_PON_MONITORING_CYCLE * 3/1000 ) */ ) 
				Trap_PonPortFER( PonPortIdx,  0, V2R1_TRAP_CLEAR );
#else
			if( LastedTime >= SECOND_PON_FER_ALERM_CLEAR_TIME ) 
				Trap_PonPortFER( PonPortIdx, 0, V2R1_TRAP_CLEAR );
#endif
            /* E--modified by liwei056@2010-1-20 for D9624 */
			}
		}		

	return( ROK );
}

/* B--added by liwei056@2011-5-6 for OnuRegister-Renew */
/* modified by xieshl 20111103, 为了解决PON LOS告警漏报问题，ONU离线也会调用该函数，提高执行效率 */
int OnRegOnuCounter(short int PonPortIdx )
{
    short int OnuIdx;
    short int onuEntryBase, onuEntry;
    int status;
    CHAR *pMac;
    int counter = 0;

    CHECK_PON_RANGE;
	
    onuEntryBase = PonPortIdx * MAXONUPERPON;
    ONU_MGMT_SEM_TAKE;
    for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
    {
        onuEntry = onuEntryBase + OnuIdx;

        /*如果是gpon onu则无条件进行状态判定*/
		if(ThisIsGponOnu(PonPortIdx, OnuIdx))
		{
		}
		else
		{
            pMac = OnuMgmtTable[onuEntry].DeviceInfo.MacAddr;
            if( MAC_ADDR_IS_ZERO(pMac) || MAC_ADDR_IS_BROADCAST(pMac) )
                continue;
        }
        /*status = GetOnuOperStatus_1(PonPortIdx, OnuIdx);*/
        status = OnuMgmtTable[onuEntry].OperStatus;
        if((ONU_OPER_STATUS_UP == status)
            || (ONU_OPER_STATUS_PENDING == status)
            || (ONU_OPER_STATUS_DORMANT == status)
            )
        {
                counter++;
        }      
    }
    ONU_MGMT_SEM_GIVE;

    return( counter );
}
/* E--added by liwei056@2011-5-6 for OnuRegister-Renew */


int OnLineOnuCounter(short int PonPortIdx )
{
	short int OnuIdx;
	short int counter =0;
	
	CHECK_PON_RANGE;

	for(OnuIdx=0;OnuIdx<MAXONUPERPON; OnuIdx++)
	{
		/*OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;*/
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		if((GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP )/*&&(OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus == LLID_ENTRY_ACTIVE)*/)
			counter++;
	}
	PonPortTable[PonPortIdx].CurrOnu = counter;
	PonPortTable[PonPortIdx].CurLLID = counter;

	return( counter );
}

int OnLineOnuCounterByType(short int PonPortIdx , unsigned char *OnuTypeString )
{
	short int OnuIdx;
	short int counter =0;
	/*int OnuType;*/
	
	CHECK_PON_RANGE

	for(OnuIdx=0;OnuIdx<MAXONUPERPON; OnuIdx++)
		{
		/*OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;*/
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		if((GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP ))
			{
			if(OnuTypeString == NULL )
				counter++;
			else {
#if 0
			    if( GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK ) continue;
				if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType)/*DeviceType_1[OnuType]*/, VOS_StrLen(OnuTypeString)) == 0)
					counter ++;
#else
			    char typeStr[ONU_TYPE_LEN+2] = "";
			    int typeStrLen = 0;
                if( (GetOnuTypeString(PonPortIdx, OnuIdx, typeStr, &typeStrLen) == ROK) || (typeStrLen != 0) )
                {
                    if( VOS_StriCmp(OnuTypeString, typeStr) == 0)
                        counter ++;
                }
#endif
				}
			}
		}
	if(OnuTypeString == NULL )
		{
		PonPortTable[PonPortIdx].CurrOnu = counter;
		PonPortTable[PonPortIdx].CurLLID = counter;
		}
	return( counter );
}
/*add by shixh20090429*/
int OffLineOnuCounter(short int PonPortIdx )
{
	short int OnuIdx;
	short int counter =0;
	
	CHECK_PON_RANGE

	for(OnuIdx=0;OnuIdx<MAXONUPERPON; OnuIdx++)
		{
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		if(GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_DOWN )
			counter++;
		}
	PonPortTable[PonPortIdx].CurrOnu = counter;
	PonPortTable[PonPortIdx].CurLLID = counter;

	return( counter );
}

int OffLineOnuCounterByType(short int PonPortIdx , unsigned char *OnuTypeString )
{
	short int OnuIdx;
	short int counter =0;
	/*int OnuType;*/
	
	CHECK_PON_RANGE

	for(OnuIdx=0;OnuIdx<MAXONUPERPON; OnuIdx++)
		{
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		if((GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_DOWN ))
			{
			if(OnuTypeString == NULL )
				counter++;
			else {
#if 0
			    if( GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK ) continue;
				if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType)/*DeviceType_1[OnuType]*/, VOS_StrLen(OnuTypeString)) == 0)
					counter ++;
#else
                char typeStr[ONU_TYPE_LEN+2] = "";
                int typeStrLen = 0;
                if( (GetOnuTypeString(PonPortIdx, OnuIdx, typeStr, &typeStrLen) == ROK) || (typeStrLen != 0) )
                {
                    if( VOS_StriCmp(OnuTypeString, typeStr) == 0)
                        counter ++;
                }
#endif
				}
			}
		}
	if(OnuTypeString == NULL )
		{
		PonPortTable[PonPortIdx].CurrOnu = counter;
		PonPortTable[PonPortIdx].CurLLID = counter;
		}
	return( counter );
}

int AllOnuCounter( short int PonPortIdx )
{
	short int OnuIdx;
	short int counter =0;
	
	CHECK_PON_RANGE

	for(OnuIdx=0;OnuIdx<MAXONUPERPON; OnuIdx++)
		{
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK ) 
			counter ++;
		}

	return( counter );
}

int AllOnuCounterBySn(short int PonPortIdx, char *SN)
{
	short int OnuIdx;
	short int counter =0;

	CHECK_PON_RANGE

	for(OnuIdx=0;OnuIdx<MAXONUPERPON; OnuIdx++)
	{
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK ) 
		{
			char local_sn[GPON_ONU_SERIAL_NUM_STR_LEN];
			VOS_MemZero(local_sn,GPON_ONU_SERIAL_NUM_STR_LEN);
			int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
			VOS_MemCpy(local_sn, OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, GPON_ONU_SERIAL_NUM_STR_LEN-1);
			if(OnuMgmtTable[OnuEntry].IsGponOnu == 0)
						continue;
			if(strstr(local_sn, SN) == NULL)
				continue;
			counter ++;
		}
	}
	return( counter );

}
int AllOnuCounterByMac( short int PonPortIdx, char *mac_address)
{
	short int OnuIdx;
	short int counter =0;
	
	CHECK_PON_RANGE

	for(OnuIdx=0;OnuIdx<MAXONUPERPON; OnuIdx++)
	{
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK ) 
		{
			char local_mac[6];
            int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
			
            VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);

            if(!mac_address[3] && !mac_address[4] && !mac_address[5])
            {
                /*可以根据oui进行筛选*/
                if((*(USHORT*) (&mac_address[0]) != *(USHORT*) (&local_mac[0])) || (mac_address[2] != local_mac[2]))
                    continue;
            }
            else
            {
                /*如果后三个字节不全为0，即需要全匹配*/
                if(MAC_ADDR_IS_UNEQUAL(mac_address, local_mac))
                    continue;
            }			
			counter ++;
		}
	}

	return( counter );
}

int AllOnuCounterByType( short int PonPortIdx, unsigned char *OnuTypeString )
{
	short int OnuIdx;
	short int counter =0;
	/*int OnuType;*/
	char typeStr[ONU_TYPE_LEN+2];
	int typeStrLen;
	
	CHECK_PON_RANGE;

	if( OnuTypeString == NULL ) return (RERROR );
	for(OnuIdx=0;OnuIdx<MAXONUPERPON; OnuIdx++)
	{
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK ) 
		{
			if(OnuTypeString == NULL )
				counter++;
			else
			{
				typeStr[0] = 0;
				typeStrLen = 0;
				if( (GetOnuTypeString(PonPortIdx, OnuIdx, typeStr, &typeStrLen) == ROK) || (typeStrLen != 0) )
				{
					if( VOS_StriCmp(OnuTypeString, typeStr) == 0)
						counter ++;
				}
				/*if( GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK ) continue;
				if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType), VOS_StrLen(OnuTypeString)) == 0)
					counter ++;*/
			}
		}
	}
	return( counter );
}


int SoftwareUpdateEnableCounter( short int PonPortIdx )
{
	short int OnuIdx;
	int counter = 0;
	CHECK_PON_RANGE

	if(AllOnuCounter( PonPortIdx ) <= 0 ) return( counter );
	for(OnuIdx=0;OnuIdx<MAXONUPERPON;OnuIdx++)
		{
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		if( GetOnuSWUpdateCtrl(PonPortIdx, OnuIdx) == ONU_SW_UPDATE_ENABLE) 
			counter ++;
		}
	return( counter);
}

int SoftwareUpdateDisableCounter(short int PonPortIdx)
{
	short int OnuIdx;
	int counter = 0;
	CHECK_PON_RANGE

	if(AllOnuCounter( PonPortIdx ) <= 0 ) return( counter );
	for(OnuIdx=0;OnuIdx<MAXONUPERPON;OnuIdx++)
		{
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		if( GetOnuSWUpdateCtrl(PonPortIdx, OnuIdx) == ONU_SW_UPDATE_DISABLE) 
			counter ++;
		}
	return( counter);
}


#ifdef __PONPORT_DEBUG
/*****************************************************
 *
 *    Function: ShowPonPortStatus( short int PonPortIdx )
 *
 *    Param:  short int PonPortIdx -- the specific pon port 
 *
 *    Desc:  print the current status of the specific pon port 
 *
 *    Return: ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 int  ShowPonPortStatus( short int PonPortIdx )
{
	CHECK_PON_RANGE

	sys_console_printf("\r\n%s/port%d Current Status %s \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), PonPortOperStatus_s[PonPortTable[PonPortIdx].PortWorkingStatus] );
	return( ROK );
}

int ShowPonPortStatusAll()
{
	short int PonPortIdx;
	
	sys_console_printf("\r\nDisplay Pon port status \r\n");
	sys_console_printf(" PonPort \t\t current status \r\n\r\n");

	for(PonPortIdx=0; PonPortIdx<MAXPON; PonPortIdx++)
		{
		sys_console_printf("%s/port%d  \t%s \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), PonPortOperStatus_s[PonPortTable[PonPortIdx].PortWorkingStatus]);
		/*sys_console_printf("   %d       \t  %s \r\n", PonPortIdx,  PonPortOperStatus_s[PonPortTable[PonPortIdx].PortWorkingStatus] );*/
		}
	sys_console_printf("\r\n");
	return( ROK );
}



/*****************************************************
 *
 *    Function: ShowPonPortBWInfo( short int PonPortIdx )
 *
 *    Param:  short int PonPortIdx -- the specific pon port 
 *
 *    Desc:  print the current bandwidth Inof of the specific pon port 
 *
 *    Return: ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 int  ShowPonPortBWInfo( short int PonPortIdx )
{
	CHECK_PON_RANGE;

	UpdateProvisionedBWInfo( PonPortIdx);
	sys_console_printf("\r\n  %s/port%d bandwidth Info \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );

	sys_console_printf("    max upstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].MaxBW );
	sys_console_printf("    allocated upstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].ProvisionedBW);
	sys_console_printf("    left upstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].RemainBW );	
	sys_console_printf("    Activated upstream bandwidth: %dKbit/s\r\n\r\n",PonPortTable[PonPortIdx].ActiveBW );
		
	sys_console_printf("    max downstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].DownlinkMaxBw );
	sys_console_printf("    allocated downstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].DownlinkProvisionedBW);
	sys_console_printf("    left downstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].DownlinkRemainBw );
	sys_console_printf("    Activated downstream bandwidth: ");

    /* B--modified by liwei056@2011-4-7 for GuestNeed */
	if( PonPortTable[PonPortIdx].DownlinkPoliceMode /* downlinkBWlimit  */== V2R1_DISABLE ){ sys_console_printf("No policer\r\n\r\n");}
	else sys_console_printf(" %dKbit/s\r\n\r\n", PonPortTable[PonPortIdx].DownlinkActiveBw );
    /* E--modified by liwei056@2011-4-7 for GuestNeed */
	
	sys_console_printf("    bandwidth allocation arithmetic:");
	(PonPortTable[PonPortIdx].external_DBA_used == TRUE ) ? sys_console_printf(" DBA\r\n\r\n") : sys_console_printf(" SBA\r\n\r\n");
	return( ROK );
}

int ShowPonPortOnuMacAddr( short int PonPortIdx )
{
	short int OnuIdx, OnuEntry,i;
	unsigned char name[MAXDEVICENAMELEN+1];
	int  nameLen;
	
	CHECK_PON_RANGE

	sys_console_printf("\r\n\r\n  %s/port%d Onu Mac address list \r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
	
	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
		{
		
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
		if(ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		/*if(CompTwoMacAddress(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr,Invalid_Mac_Addr) == RERROR)*/
			{
			sys_console_printf("  %d     ", (OnuIdx +1 ));
			for(i=0;i<( BYTES_IN_MAC_ADDRESS-1); i++ )
				{
				sys_console_printf("%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i+1] );
				i++;
				if( i != 5 )sys_console_printf(".");
				}
			GetOnuDeviceName( PonPortIdx, OnuIdx, name, &nameLen);
			name[nameLen] = '\0';
			sys_console_printf( "      valid   %s \r\n", name);
			}
		}

#if 0
	if( PonPortTable[PonPortIdx].InvalidOnuNum != 0 )
		{
		NextPtr = PonPortTable[PonPortIdx].InvalidOnuTablePtr;
	
		while( NextPtr != NULL ){
		
			OnuMacAddr = NextPtr->MAC;
			sys_console_printf("     ");
			for(i=0;i<( BYTES_IN_MAC_ADDRESS); i++ )
				{
				sys_console_printf("%02x%02x", OnuMacAddr[i],OnuMacAddr[i+1] );
				i++;
				if( i != 5 )sys_console_printf(".");
				}
			sys_console_printf("      Invalid\r\n");	
			NextPtr = (MacTable_S *)NextPtr->nextMac;
			}
		}
#endif	
	
	sys_console_printf("   End of list \r\n" );
	return( ROK );

}

int ShowPonPortOnLineOnu( short int PonPortIdx )
{
	short int OnuIdx, OnuEntry,i;
	
	CHECK_PON_RANGE

	sys_console_printf("\r\n\r\n  Display On line Onu \r\n");
	/*
	if(( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_DOWN) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UNKNOWN ) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_INIT ) )
	*/
	if( PonPortIsWorking(PonPortIdx) != TRUE )
		{
		sys_console_printf("  %s/port%d is not Working \r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		return( RERROR);
		}
	sys_console_printf("   %s/port%d \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
	sys_console_printf("     Max onu: %d \r\n", PonPortTable[PonPortIdx].MaxOnu  );
	OnLineOnuCounter( PonPortIdx);
	sys_console_printf("     current Onu: %d\r\n",PonPortTable[PonPortIdx].CurrOnu);
	sys_console_printf("     current Onu list: \r\n");
	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
		{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
		if( GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP )
			{
			sys_console_printf("     Onu index: %4d, mac addr: ", (OnuEntry % MAXONUPERPON + 1));
			for(i=0;i<( BYTES_IN_MAC_ADDRESS-1); i++ )
				{
				sys_console_printf("%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i+1] );
				i++;
				if( i != 5 )sys_console_printf(".");
				}
			sys_console_printf("\r\n");
			}
		}
	sys_console_printf("   Onu list end\r\n" );
	return( ROK );
}


int ShowPonPortInfo(short int PonPortIdx )
{
	int i,j;
	
	CHECK_PON_RANGE

#if 0
	if(( PonPortTable[PonPortIdx].PortWorkingStatus != PONPORT_UP )&&( PonPortTable[PonPortIdx].PortWorkingStatus != PONPORT_LOOP) && ( PonPortTable[PonPortIdx].PortWorkingStatus != PONPORT_UPDATE))
		{
		sys_console_printf("\r\n%s/%d status is down, No Information about it \r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		return( ROK );
		}
#endif	

	UpdateProvisionedBWInfo( PonPortIdx);
	sys_console_printf("\r\n  %s/port%d Infomation:\r\n\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
	
	sys_console_printf("    operation status: %s\r\n", PonPortOperStatus_s[PonPortTable[PonPortIdx].PortWorkingStatus]);
	sys_console_printf("    admin status: %s\r\n", PonPortAdminStatus_s[PonPortTable[PonPortIdx].PortAdminStatus] );

	sys_console_printf("    firmware ver: %s\r\n", PonChipMgmtTable[PonPortIdx].FirmwareVer );
	sys_console_printf("    host sw ver: %s\r\n", PonChipMgmtTable[PonPortIdx].HostSwVer );
	sys_console_printf("    hard ver: %s\r\n", PonChipMgmtTable[PonPortIdx].HardVer );
	sys_console_printf("    CNI port MAC bus type:%s\r\n", PON_mac_s[PonChipMgmtTable[PonPortIdx].WorkingMode.MacType ] );
	sys_console_printf("    Laser on/off time(TQ) %d/%d\r\n", PonChipMgmtTable[PonPortIdx].WorkingMode.laserOnTime, PonChipMgmtTable[PonPortIdx].WorkingMode.laserOffTime );
	sys_console_printf("    AGC/CDR lock time(TQ) %d/%d\r\n", PonChipMgmtTable[PonPortIdx].WorkingMode.AGCTime, PonChipMgmtTable[PonPortIdx].WorkingMode.CDRTime);
	
	sys_console_printf("    max onu number: %d\r\n", PonPortTable[PonPortIdx].MaxOnu);	
	OnLineOnuCounter(PonPortIdx);
	sys_console_printf("    current online Onu number: %d\r\n",PonPortTable[PonPortIdx].CurrOnu );
	/*sys_console_printf("    max llid number: %d\r\n", PonPortTable[PonPortIdx].MaxLLID);	
	sys_console_printf("    current llid number: %d\r\n", PonPortTable[PonPortIdx].CurLLID );*/

	sys_console_printf("    Current Online Onu bitMap(1-Up, 0-Down):\r\n");
	sys_console_printf("                  ");
	for( i=0; i<8; i++ )
		{		
		for( j=0; j<8; j++ )
			{
			if( GetOnuOperStatus( PonPortIdx, (i*8+j)) == ONU_OPER_STATUS_UP )
				{
				sys_console_printf("1");
				}
			else {
				sys_console_printf("0");
				}
			}
		sys_console_printf("  ");
		if((i+1)%4 ==0) sys_console_printf("\r\n");
		if((i+1) == 4 ) sys_console_printf("                  ");
		}
	/*
	sys_console_printf("    Current Online Onu map: %02d %02d %02d %02d\r\n", *Ptr, *(Ptr+1), *(Ptr+2), *(Ptr+3));
	sys_console_printf("                            %02d %02d %02d %02d\r\n", *(Ptr+4), *(Ptr+5), *(Ptr+6), *(Ptr+7));
	*/

#if 0  /* deleted by chenfj 2007-7-26 */
	sys_console_printf("    Onu Auto-register flag : ");
	if(  PonPortTable[PonPortIdx].AutoRegisterFlag == V2R1_ENABLE ) sys_console_printf("disable\r\n");
	else sys_console_printf("enable\r\n");
#endif

	sys_console_printf("    aps: %s\r\n", PonPortApsCtrl_s[ PonPortTable[PonPortIdx].ApsCtrl]);
	sys_console_printf("    mac learning enable:  ");
	( PonPortTable[PonPortIdx].MacSelfLearningCtrl == V2R1_ENABLE ) ? sys_console_printf("enable\r\n"): sys_console_printf("disable\r\n");
	sys_console_printf("    aging time: %d(ms)\r\n\r\n", PonPortTable[PonPortIdx].MACAgeingTime );

	sys_console_printf("    Encrypt Direction: %s\r\n", v2r1EncryptDirection[PonPortTable[PonPortIdx].EncryptDirection]);
	sys_console_printf("    Encrypt Key exchangeTime:%d(s) \r\n\r\n", (PonPortTable[PonPortIdx].EncryptKeyTime/SECOND_1) );

	sys_console_printf("    max upstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].MaxBW );
	sys_console_printf("    allocated upstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].ProvisionedBW);
	sys_console_printf("    left upstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].RemainBW );	
	sys_console_printf("    Activated upstream bandwidth: %dKbit/s\r\n\r\n",PonPortTable[PonPortIdx].ActiveBW );
		
	sys_console_printf("    max downstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].DownlinkMaxBw );
	sys_console_printf("    allocated downstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].DownlinkProvisionedBW);
	sys_console_printf("    left downstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].DownlinkRemainBw );
	sys_console_printf("    Activated downstream bandwidth: ");
	if( PonPortTable[PonPortIdx].DownlinkPoliceMode /* downlinkBWlimit */ == V2R1_DISABLE ){ sys_console_printf("(No policer)");}
	else sys_console_printf("%dKbit/s", PonPortTable[PonPortIdx].DownlinkActiveBw);
	sys_console_printf("\r\n\r\n");
	
	sys_console_printf("    bandwidth allocation arithmetic:");
	(PonPortTable[PonPortIdx].external_DBA_used == TRUE ) ? sys_console_printf(" DBA\r\n\r\n") : sys_console_printf(" SBA\r\n\r\n");
	
	sys_console_printf("    Onu Max Range: %d, %d(m)\r\n", PonPortTable[PonPortIdx].MaxRTT, (PonPortTable[PonPortIdx].MaxRTT *32/10));
	sys_console_printf("    Online Onu Max Range: %d(m)\r\n", PonPortTable[PonPortIdx].CurrRTT );
	
	sys_console_printf("    Pon port periodic test timer:%d , test counter:%d \r\n", PonPortTable[PonPortIdx].testTimer, PonPortTable[PonPortIdx].testCounter );
	sys_console_printf("    Type:%s \r\n\r\n", PonPortType_s[PonPortTable[PonPortIdx].type] );

	
	return( ROK );
}

int ShowPonMacLearning( short int PonPortIdx )
{
	short int  PonChipType;
	short int active_records=0;
	short int i,j;
	
	CHECK_PON_RANGE

	VOS_MemSet( Mac_addr_table, 0, sizeof(PON_address_record_t )*PON_ADDRESS_TABLE_SIZE);

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
			
	sys_console_printf("\r\n  %s/port%d Learning Mac list\r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
	sys_console_printf("    total Learning mac number:");

	if( OLT_PONCHIP_ISVALID(PonChipType))
		{
#if 1
		if( OLT_CALL_ISERROR( OLT_GetMacAddrTbl(PonPortIdx, &active_records, Mac_addr_table) ) )
#else
		if(( PAS_get_address_table(PonPortIdx, &active_records, Mac_addr_table)) != PAS_EXIT_OK )
#endif
			{
			sys_console_printf("0\r\n");
			return( ROK );
			}
		}
	else{/* 其他PON芯片类型处理*/
		sys_console_printf("0\r\n");
		return( ROK );
		}
	sys_console_printf("%d\r\n", active_records );
	
#if 0	
		llid = OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].Llid;
		counter = 0;	
		for( i=0; i< active_records; i++)
			{
			if( Mac_addr_table[i].logical_port == llid ) counter++;
			}
		sys_console_printf("%d\r\n", counter);

		if( counter == 0) return( ROK );
#endif

	sys_console_printf("    Current mac address list:\r\n");
	
	for(i=0; i< active_records; i++)
		{
		sys_console_printf("    ");
		for(j=0; j<(BYTES_IN_MAC_ADDRESS-1); j++)
			{
			sys_console_printf("%02x%02x", Mac_addr_table[i].mac_address[j], Mac_addr_table[i].mac_address[j+1] );
			j++;
			if( j != 5 )sys_console_printf(".");
			}
		sys_console_printf("     llid-%d     %s\r\n", Mac_addr_table[i].logical_port, v2r1AddrTableAgeingMode[Mac_addr_table[i].type] );
		}
	
	return( ROK );

}

int ShowOnuRegisterCounterInfo(short int PonPortIdx )
{
	Date_S  NewerDate;
	int i;
	
	CHECK_PON_RANGE

	if(PonPortIsWorking(PonPortIdx) != TRUE ) 
		{
		sys_console_printf(" %s/port%d is not in working status\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
		return( RERROR );
		}
	sys_console_printf("  %s/port%d register counter info \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
	sys_console_printf("    register-req Msg counter: %d \r\n", PonPortTable[PonPortIdx].OnuRegisterMsgCounter );
	sys_console_printf("    Deregister Msg counter: %d\r\n",PonPortTable[PonPortIdx].OnuDeregisterMsgCounter);

	if( PonPortTable[PonPortIdx].OnuRegisterMsgCounter != 0 )
		{
		sys_console_printf("    the last onu register time:");
			{
			VOS_MemCpy((char *)&(NewerDate),(char *)&( PonPortTable[PonPortIdx].LastOnuRegisterTime.year),sizeof( Date_S));
			sys_console_printf("%04d-%02d-%02d,", NewerDate.year, NewerDate.month, NewerDate.day );
			sys_console_printf("%d:%d:%d\r\n", NewerDate.hour, NewerDate.minute, NewerDate.second );
			}
		sys_console_printf("    the last onu register addr: ");
		for(i=0;i<( BYTES_IN_MAC_ADDRESS-1); i++ )
			{
			sys_console_printf("%02x%02x", PonPortTable[PonPortIdx].LastRegisterMacAddr[i],PonPortTable[PonPortIdx].LastRegisterMacAddr[i+1] );
			i++;
			if( i != 5 ) sys_console_printf(".");
			}
		}
	sys_console_printf("\r\n");

	return( ROK );
}
#endif

/* modifiy by chenfj 2006/09/19 */
/* #2604 问题  telnet用户在串口pon节点下使用命令show current-onu，输出显示在串口上了*/

 int  ShowPonPortStatusByVty( short int PonPortIdx, struct vty *vty )
{
	unsigned int status;
	CHECK_PON_RANGE

	status = PonPortTable[PonPortIdx].PortWorkingStatus;
	if( status >= PONPORT_OPER_MAX )		/* modified by xieshl 20091215, 防止数组越界 */
		status = 0;
	vty_out(vty, "  \r\n%s/port%d Current Status %s \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), PonPortOperStatus_s[status] );
	return( ROK );
}

int ShowPonPortStatusAllByVty(struct vty *vty)
{
	unsigned int status;
	short int PonPortIdx;
	
	vty_out(vty, "  \r\nDisplay Pon port status \r\n");
	vty_out(vty, "   PonPort \t\t current status \r\n\r\n");

	for(PonPortIdx=0; PonPortIdx<MAXPON; PonPortIdx++)
	{
		status = PonPortTable[PonPortIdx].PortWorkingStatus;
		if( status >= PONPORT_OPER_MAX )
			status = 0;
		vty_out(vty, "  %s/port%d  \t%s \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), PonPortOperStatus_s[status]);
	}
	vty_out(vty, "\r\n");
	return( ROK );
}

 int  ShowPonPortBWInfoByVty( short int PonPortIdx, struct vty *vty )
{
	CHECK_PON_RANGE;

	UpdateProvisionedBWInfo( PonPortIdx);
	vty_out(vty, "\r\n  %s/port%d bandwidth Info \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );

	vty_out(vty, "    max upstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].MaxBW );
	vty_out(vty, "    allocated upstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].ProvisionedBW);
#ifdef PLATO_DBA_V3
	{
	char i = 0;
	unsigned int fixedBw = 0;
/*	ONU_MGMT_SEM_TAKE;*/
	for( i=0; i< MAXONUPERPON; i++)
	{
		if(ThisIsValidOnu(PonPortIdx, i) != ROK ) continue;
		fixedBw +=  OnuMgmtTable[PonPortIdx * MAXONUPERPON + i].FinalUplinkBandwidth_fixed/*OnuMgmtTable[PonPortIdx * MAXONUPERPON + i].LlidTable[0].UplinkBandwidth_fixed */;
	}
    
/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty,"    (fixed bandwidth:%dKbit/s)\r\n",fixedBw);
	}
#endif
	vty_out(vty, "    left upstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].RemainBW );	
	vty_out(vty, "    Activated upstream bandwidth: %dKbit/s\r\n\r\n",PonPortTable[PonPortIdx].ActiveBW );
		
	vty_out(vty, "    max downstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].DownlinkMaxBw );
	vty_out(vty, "    allocated downstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].DownlinkProvisionedBW);
	vty_out(vty, "    left downstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].DownlinkRemainBw );
	vty_out(vty, "    Activated downstream bandwidth: ");
	
	if( PonPortTable[PonPortIdx].DownlinkPoliceMode /* downlinkBWlimit */ == V2R1_DISABLE ){ vty_out(vty, "No policer\r\n\r\n");}
	else vty_out(vty, "%dKbit/s\r\n\r\n", PonPortTable[PonPortIdx].DownlinkActiveBw);
	
	vty_out(vty, "    bandwidth allocation arithmetic:");
	(PonPortTable[PonPortIdx].external_DBA_used == TRUE ) ? vty_out(vty, " DBA\r\n\r\n") : vty_out(vty, " SBA\r\n\r\n");
	
	return( ROK );
}

 int ShowPonPortEncryptInfoByVty (short int PonPortIdx, struct vty *vty )
{
	short int OnuIdx ;
	short int Onu_llid;
	/*PON_llid_parameters_t llid_parameters;*/
	unsigned int dir, status;
	int iKeyTime;
	int flag= 1;
	int onuEntryBase;
	
	CHECK_PON_RANGE;

	vty_out(vty," \r\n%s/port%d Encrypt Info\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
	
	/*vty_out(vty, "    Encrypt Direction: %s\r\n", v2r1EncryptDirection[PonPortTable[PonPortIdx].EncryptDirection]);
	vty_out(vty, "    Encrypt Key exchangeTime : %d(s)\r\n\r\n", (PonPortTable[PonPortIdx].EncryptKeyTime/SECOND_1) );*/

	/* modified by chenfj 2008-6-6
	     修改ONU 加密显示格式, 区分出ONU已配置了加密后, 启动加密和未启动加密两种情况
	     */
	onuEntryBase = PonPortIdx * MAXONUPERPON;
	for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
	{
		if( ThisIsValidOnu( PonPortIdx, OnuIdx )  != ROK ) continue;
		if( OnuMgmtTable[onuEntryBase + OnuIdx].EncryptEnable == V2R1_ENABLE)
		{
			if( flag == 1 ) 
			{
				vty_out( vty, "onu encrypt enabled as follows \r\n");
				flag =2 ;
				break;
			}
			/*vty_out( vty, " Onu-%d direction:%s, key update time=%d(s)\r\n",(OnuIdx+1),v2r1EncryptDirection[onuEntryBase + OnuIdx].EncryptDirection ], (OnuMgmtTable[onuEntryBase + OnuIdx].EncryptKeyTime/SECOND_1));*/
		}		
	}
	if( flag == 1 ) 
	{
		vty_out(vty, "No onu encrypt enabled \r\n");
		return( CMD_SUCCESS );
	}
	
	vty_out( vty, "%-10s%-15s%-15s%-10s\r\n", "OnuIdx","direction","IsStarted","key-update-time(S)");
	vty_out( vty,"------------------------------------------------------------\r\n");
/*	ONU_MGMT_SEM_TAKE;*/
	for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
	{
		if( ThisIsValidOnu( PonPortIdx, OnuIdx )  != ROK )
			continue;

		if( OnuMgmtTable[onuEntryBase + OnuIdx].EncryptEnable == V2R1_ENABLE)
		{
			status = 2;
			if((Onu_llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx)) != INVALID_LLID)
			{
				/* modified by xieshl 20091215, 增加返回值和最大取值范围限制，防止数组越界 */
                /* B--modified by liwei056@2011-2-12 for NewApi */
#if 0

				/* modified by shixh20101216*/
#if 0
				if( PAS_get_llid_parameters ( PonPortIdx, Onu_llid, &llid_parameters ) != PAS_EXIT_OK )
				{
					VOS_ASSERT(0);
					return CMD_WARNING;
				}
#else
                if( OnuMgt_GetLLIDParams ( PonPortIdx, Onu_llid, &llid_parameters ) != OLT_ERR_OK )
				{
/*					ONU_MGMT_SEM_GIVE;*/
					VOS_ASSERT(0);
					return CMD_WARNING;
				}
#endif
				if(llid_parameters.encryption_mode != PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION)
				{
					dir = llid_parameters.encryption_mode+PON_ENCRYPTION_PURE;
					if( dir > 3 )
						dir = 0;
					vty_out(vty,"%-10d%-15s%-15s%-10d\r\n", (OnuIdx+1), v2r1EncryptDirection[dir], v2r1Start[V2R1_STARTED],
							(OnuMgmtTable[onuEntryBase + OnuIdx].EncryptKeyTime/SECOND_1));
					flag =1 ;
				}
#else
				if( OnuMgt_GetOnuEncryptParams ( PonPortIdx, OnuIdx, &dir, &iKeyTime, &status ) == OLT_ERR_OK )
				{
/*					ONU_MGMT_SEM_GIVE;*/
				}
#endif
                /* E--modified by liwei056@2011-2-12 for NewApi */
			}
			dir = OnuMgmtTable[onuEntryBase + OnuIdx].EncryptDirection;
			iKeyTime = OnuMgmtTable[onuEntryBase + OnuIdx].EncryptKeyTime;
			if( dir > 3 ) dir = 0;
			if( status > 2 ) status = 0;
			vty_out(vty,"%-10d%-15s%-15s%-10d\r\n", (OnuIdx+1), v2r1EncryptDirection[dir], v2r1Start[status],
					(iKeyTime / SECOND_1));
			/*{
				dir = OnuMgmtTable[onuEntryBase + OnuIdx].EncryptDirection;
				if( dir > 3 ) dir = 0;
				status = OnuMgmtTable[onuEntryBase + OnuIdx].EncryptStatus;
				if( status > 2 ) status = 0;
				vty_out( vty,"%-10d%-15s%-15s%-10d\r\n", (OnuIdx+1), v2r1EncryptDirection[dir], v2r1Start[status],
						(OnuMgmtTable[onuEntryBase + OnuIdx].EncryptKeyTime/SECOND_1) );
			}*/
		}
	}
/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty, "\r\n");
	return ( ROK );
}
#if 1
void show_onu_list_banner( struct vty *vty )
{
	int i, DeviceTypeLen = GetDeviceTypeLength();
	
	vty_out(vty, "Idx Mac addr/SN         type");
	for( i=1; i<DeviceTypeLen; i++ ) 
        vty_out(vty," ");
	vty_out(vty, "status    online-time          offline-time         userName\r\n");
	vty_out(vty, "------------------------------------------------------------------------------------------------\r\n");
}
int show_pon_port_onu_list( struct vty *vty, short int PonPortIdx, unsigned char *OnuTypeString )
{
	short int OnuIdx, OnuEntry;
	unsigned char name[MAXDEVICENAMELEN+1];
	int  nameLen=0;
	int counter= 0;
	int status;
	/*ULONG CurTicks, SysUpDelayed;
	ULONG days, hours,minutes, seconds;*/
	
	CHECK_PON_RANGE

	if ( OnuTypeString == NULL )
	{
		counter = AllOnuCounter(PonPortIdx);
		if( counter == 0 )
			return( ROK );
		vty_out(vty, "pon%d/%d [onu counter = %d]\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx) , counter);
	}
	else
	{
		counter = AllOnuCounterByType(PonPortIdx, OnuTypeString);
		if( counter == 0 )
			return( ROK );
		vty_out(vty, "pon%d/%d [onu(%s) counter = %d]\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx) , OnuTypeString, counter);
	}

/*	ONU_MGMT_SEM_TAKE;*/
	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;

		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		{
			if( OnuTypeString != NULL )
			{
			    if(OnuMgmtTable[OnuEntry].DeviceInfo.type!= V2R1_ONU_CTC && OnuMgmtTable[OnuEntry].DeviceInfo.type!= V2R1_ONU_GPON && VOS_StriCmp(OnuTypeString,GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)) != 0 ) 
			        continue;
			    else
			    {
			    	if((!VOS_MemCmp(OnuTypeString,"other",5)) && (SeachOnuIsCtcByOnuid(PonPortIdx,OnuIdx) == VOS_OK))
			    	{
			    		
			    	}
					else
					{
				        if(GetOnuModel(PonPortIdx, OnuIdx, name, &nameLen) != ROK)
				            continue;
				        else if(VOS_StriCmp(OnuTypeString, name))
				            continue;
					}
			    }
			}
			vty_out( vty, "%2d  ", (OnuIdx+1));
			if(OnuMgmtTable[OnuEntry].IsGponOnu)
			    vty_out(vty, "%16s  ", OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No);
			else
			    vty_out(vty, "  %02x%02x.%02x%02x.%02x%02x  ", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] );

			if( (GetOnuModel(PonPortIdx, OnuIdx, name, &nameLen) != ROK) || (nameLen == 0) || (nameLen > MAXDEVICENAMELEN) )
			{
				VOS_StrCpy( name, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );	
			}
			vty_out( vty, "  %-12s  ", name );	

			status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
			if( (status < 0) || (status > 5) )
				status = 0;
			vty_out(vty," %s ", OnuCurrentStatus[status]);

            if(OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.year)
            {
    			vty_out(vty,"%04d/%02d/%02d,%02d:%02d:%02d", 
                    OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.year, 
                    OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.month, 
                    OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.day, 
                    OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.hour,
                    OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.minute,
                    OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.second);    /* modified by xieshl 20100319, GWD网上问题9966 */
            }
            else
    			vty_out(vty,"%04s/%02s/%02s,%02s:%02s:%02s", 
                    "----", "--", "--", "--", "--", "--");
            
            if(OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.year)
            {
    			vty_out(vty,"  %04d/%02d/%02d,%02d:%02d:%02d", 
                    OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.year, 
                    OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.month, 
                    OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.day, 
                    OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.hour,
                    OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.minute,
                    OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.second);    /* modified by xieshl 20100319, GWD网上问题9966 */
            }
            else
    			vty_out(vty,"  %04s/%02s/%02s,%02s:%02s:%02s", 
                    "----", "--", "--", "--", "--", "--");
            
			nameLen = 0;
			GetOnuDeviceName( PonPortIdx, OnuIdx, name, &nameLen);
			if(nameLen > MAXDEVICENAMELEN ) nameLen = MAXDEVICENAMELEN;
			name[nameLen] = '\0';
			vty_out(vty, /*"   valid*/"  %s\r\n", name); 
			counter ++;
		}
	}
/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty, "\r\n");

	return( ROK );
}
#else
/* modified by xieshl 20110111, 合并onu-list显示函数 ，问题单11882*/
void show_onu_list_banner( struct vty *vty )
{
	int i, DeviceTypeLen = GetDeviceTypeLength();
	
	vty_out(vty, "Idx Mac addr        type");
	for( i=1; i<DeviceTypeLen; i++ ) vty_out(vty," ");
	vty_out(vty, "status    Lastedtime     userName\r\n");
	vty_out(vty, "---------------------------------------------------------------------\r\n");
}

int show_pon_port_onu_list( struct vty *vty, short int PonPortIdx, unsigned char *OnuTypeString )
{
	short int OnuIdx, OnuEntry;
	unsigned char name[MAXDEVICENAMELEN+1];
	int  nameLen=0;
	int counter= 0;
	int status;
	ULONG /*CurTicks,*/ SysUpDelayed;
	ULONG days, hours,minutes, seconds;
	
	CHECK_PON_RANGE

	if ( OnuTypeString == NULL )
	{
		counter = AllOnuCounter(PonPortIdx);
		if( counter == 0 )
			return( ROK );
		vty_out(vty, "pon%d/%d [onu counter = %d]\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx) , counter);
	}
	else
	{
		counter = AllOnuCounterByType(PonPortIdx, OnuTypeString);
		if( counter == 0 )
			return( ROK );
		vty_out(vty, "pon%d/%d [onu(%s) counter = %d]\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx) , OnuTypeString, counter);
	}

/*	ONU_MGMT_SEM_TAKE;*/
	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;

		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		{
			if( OnuTypeString != NULL )
			{
			    if(OnuMgmtTable[OnuEntry].DeviceInfo.type!= V2R1_ONU_CTC && VOS_StriCmp(OnuTypeString,GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)) != 0 ) continue;
			    else
			    {
			        if(GetOnuModel(PonPortIdx, OnuIdx, name, &nameLen) != ROK)
			            continue;
			        else if(VOS_StriCmp(OnuTypeString, name))
			            continue;
			    }
			}
			vty_out( vty, "%2d  ", (OnuIdx+1));
			vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] );

			if( (GetOnuModel(PonPortIdx, OnuIdx, name, &nameLen) != ROK) || (nameLen == 0) || (nameLen > MAXDEVICENAMELEN) )
			{
				VOS_StrCpy( name, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );	
			}
			vty_out( vty, "  %-12s  ", name );	

			status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
			if( (status < 0) || (status > 5) )
				status = 0;
			vty_out(vty," %s ", OnuCurrentStatus[status]);

			/*CurTicks = VOS_GetTick();
			SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;

			seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;*/
			SysUpDelayed = time(0) - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
			days = SysUpDelayed /V2R1_ONE_DAY;
			hours = (SysUpDelayed % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
			minutes = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
			seconds = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;
			vty_out(vty,"%04u:%02u:%02u:%02u ",days, hours, minutes, seconds );	/* modified by xieshl 20100319, GWD网上问题9966 */

			nameLen = 0;
			GetOnuDeviceName( PonPortIdx, OnuIdx, name, &nameLen);
			if(nameLen > MAXDEVICENAMELEN ) nameLen = MAXDEVICENAMELEN;
			name[nameLen] = '\0';
			vty_out(vty, /*"   valid*/" %s\r\n", name); 
			counter ++;
		}
	}
/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty, "\r\n");

	return( ROK );
}
#endif
/* modified by chenfj 2008-2-27 #6349
     在显示ONU 列表时, 增加ONU 类型可选参数, 这样可以只显示指定类型的ONU
     */
int ShowPonPortOnuMacAddrByVty( short int PonPortIdx , unsigned char *OnuTypeString, struct vty *vty)
{
#if 1
	int counter = 0;
	if ( OnuTypeString == NULL )
		counter = AllOnuCounter(PonPortIdx);
	else
		counter = AllOnuCounterByType(PonPortIdx, OnuTypeString);
	if( counter == 0 )
	{
		vty_out(vty, "pon%d/%d [onu counter = %d]\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx) , counter);
		return( ROK );
	}
	
	show_onu_list_banner( vty );
	return show_pon_port_onu_list( vty, PonPortIdx, OnuTypeString );
#else
	short int OnuIdx, OnuEntry,i;
	unsigned char name[MAXDEVICENAMELEN+1];
	int  nameLen=0;
	int counter= 0;
	int DeviceTypeLen = 0;
	int status;

	ULONG CurTicks, SysUpDelayed;
	ULONG days, hours,minutes, seconds;
	
	CHECK_PON_RANGE

	if( OnuTypeString == NULL )
	{
		counter = AllOnuCounter(PonPortIdx);
		if( counter >= 0 )
		{
			vty_out(vty, "\r\n%s/port%d [Onu counter = %d]\r\n\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) , counter);
			if( counter == 0 )
				return( ROK );
		}
		else return( ROK );
	}
	else
	{
		counter = AllOnuCounterByType(PonPortIdx, OnuTypeString);
		if( counter >= 0 )
		{
			vty_out(vty, "\r\n%s/port%d [Onu(%s) counter = %d]\r\n\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) , OnuTypeString, counter);
			if( counter == 0 )
				return( ROK );
		}
		else return( ROK );
	}

	DeviceTypeLen = GetDeviceTypeLength();
	vty_out(vty, "Idx Mac addr        type    ");
	for(i=0; i<(DeviceTypeLen-5); i++)
		vty_out(vty," ");
	vty_out(vty,"status    Lastedtime     userName\r\n");
	vty_out(vty, "---------------------------------------------------------------------------\r\n");
	
	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		{
			if( OnuTypeString != NULL )
				if(VOS_StriCmp(OnuTypeString,GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)) != 0) continue;
			vty_out( vty, "%2d  ", (OnuIdx+1));
			vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] );
			
			if( GetOnuModel( PonPortIdx, OnuIdx, name, &nameLen) != ROK )
				VOS_StrCpy( name, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );
			vty_out( vty, "  %-12s  ", name );
			
			/* modified by chenfj 2008-2-20
		     		显示时使用扩展的ONU操作状态
		    */
			status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
			if( (status < 0) || (status > 5) )
				status = 0;
			vty_out(vty," %s ", OnuCurrentStatus[status]);

			CurTicks = VOS_GetTick();
			SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
			seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;
			/*days = 0;
			hours = 0;
			minutes = 0;*/
			days = seconds /V2R1_ONE_DAY;
			hours = (seconds % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
			minutes = ((seconds % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
			seconds = ((seconds % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;

			vty_out(vty,"%04u:%02u:%02u:%02u ",days, hours, minutes, seconds );	/* modified by xieshl 20100319, GWD网上问题9966 */

	    		nameLen = 0;
	    		GetOnuDeviceName( PonPortIdx, OnuIdx, name, &nameLen);
	    		if(nameLen > MAXDEVICENAMELEN ) nameLen = MAXDEVICENAMELEN;
	    		name[nameLen] = '\0';
	    		vty_out(vty, /*"   valid*/" %s\r\n", name); 
	    		counter ++;
		}
	}

	vty_out(vty, "\r\n");
	/*
	vty_out(vty, "   End of list, total onu counter=%d \r\n" , counter);
	all_config_onu += counter;*/
	return( ROK );
#endif
}
#if 0
int ShowPonPortOnuMacAddrByVty_1( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty)
{
	short int OnuIdx, OnuEntry/*,i*/;
	unsigned char name[MAXDEVICENAMELEN+1];
	int  nameLen=0;
	int counter= 0;
	/*int DeviceTypeLen = 0;*/
	int status;
	
	CHECK_PON_RANGE

	if ( OnuTypeString == NULL )
		{
		counter = AllOnuCounter(PonPortIdx);
		if( counter > 0 )
			{
			vty_out(vty, "%s/port%d [Onu counter = %d]\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) , counter);
			}
		else return( ROK );
		}
	else{
		counter = AllOnuCounterByType(PonPortIdx, OnuTypeString);
		if( counter > 0 )
			{
			vty_out(vty, "%s/port%d [Onu(%s) counter = %d]\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) , OnuTypeString, counter);
			}
		else return( ROK );
		}

	/*DeviceTypeLen = GetDeviceTypeLength();*/

	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;

		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		{
			if( OnuTypeString != NULL )
				if(VOS_StrniCmp(OnuTypeString,GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type), VOS_StrLen(OnuTypeString)) != 0 ) continue;
			vty_out( vty, "%2d  ", (OnuIdx+1));
			vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] );

			if( (GetOnuModel(PonPortIdx, OnuIdx, name, &nameLen) != ROK) || (nameLen == 0) || (nameLen > MAXDEVICENAMELEN) )
			{
				VOS_StrCpy( name, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );	
			}
			vty_out( vty, "  %-12s  ", name );	

			/* modified by chenfj 2008-2-20
		          显示时使用扩展的ONU操作状态
		    */
			status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
			if( (status < 0) || (status > 5) )
				status = 0;
			vty_out(vty," %s ", OnuCurrentStatus[status]);

			{
				ULONG CurTicks, SysUpDelayed;
				ULONG days, hours,minutes, seconds;
				/*Date_S UpDate;*/
			
				CurTicks = VOS_GetTick();
				SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;

				seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;
				/*days = 0;
				hours = 0;
				minutes = 0;*/
				days = seconds /V2R1_ONE_DAY;
				hours = (seconds % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
				minutes = ((seconds % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
				seconds = ((seconds % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;
				vty_out(vty,"%04u:%02u:%02u:%02u ",days, hours, minutes, seconds );	/* modified by xieshl 20100319, GWD网上问题9966 */
			}

			nameLen = 0;
			GetOnuDeviceName( PonPortIdx, OnuIdx, name, &nameLen);
			if(nameLen > MAXDEVICENAMELEN ) nameLen = MAXDEVICENAMELEN;
			name[nameLen] = '\0';
			vty_out(vty, /*"   valid*/" %s\r\n", name); 
			counter ++;
			}
		}

	vty_out(vty, "\r\n");
	/*
	vty_out(vty, "   End of list, total onu counter=%d \r\n" , counter);
	all_config_onu += counter;*/
	return( ROK );
}
#endif

int ShowPonPortOnuMacAddrByVtyAll(unsigned char *OnuTypeString, struct vty *vty)
{
	short  int PonPortIdx;
	/*int DeviceTypeLen = 0;
	int i;*/
	/* int Product_type = GetOltType(); */
	
	all_config_onu =0;

	for( PonPortIdx = 0;PonPortIdx < MAXPON; PonPortIdx ++)
	{
		if( OnuTypeString == NULL )
			all_config_onu += AllOnuCounter(PonPortIdx );
		else 
			all_config_onu += AllOnuCounterByType(PonPortIdx,OnuTypeString);
	}
	if( OnuTypeString == NULL )
		vty_out( vty, "\r\n[TOTAL ONU COUNTER = %d]\r\n", all_config_onu);
	else 
		vty_out( vty, "\r\n[TOTAL ONU(%s) COUNTER = %d]\r\n", OnuTypeString, all_config_onu);
	if( all_config_onu == 0 ) return( ROK );

	/*DeviceTypeLen = GetDeviceTypeLength();
	vty_out(vty, "Idx Mac addr        type    ");
	for(i=0; i<(DeviceTypeLen - 5);i++)
		vty_out(vty," ");
	vty_out(vty,"status    Lastedtime     userName\r\n");
	vty_out(vty, "---------------------------------------------------------------------------\r\n");*/
	show_onu_list_banner( vty );

#if 0
	if(Product_type == V2R1_OLT_GFA6700)
		{
		for(PonPortIdx =16; PonPortIdx < MAXPON; PonPortIdx ++ )
			{
			if( SearchValidOnuEntry(PonPortIdx) == ROK )
				ShowPonPortOnuMacAddrByVty_1(  PonPortIdx, OnuTypeString, vty );
			}

		for(PonPortIdx =12; PonPortIdx < 16; PonPortIdx ++ )
			{
			if( SearchValidOnuEntry(PonPortIdx) == ROK )
				ShowPonPortOnuMacAddrByVty_1(  PonPortIdx, OnuTypeString, vty );
			}

		for(PonPortIdx =8; PonPortIdx < 12; PonPortIdx ++ )
			{
			if( SearchValidOnuEntry(PonPortIdx) == ROK )
				ShowPonPortOnuMacAddrByVty_1(  PonPortIdx, OnuTypeString, vty );
			}

		for(PonPortIdx =4; PonPortIdx < 8; PonPortIdx ++ )
			{
			if( SearchValidOnuEntry(PonPortIdx) == ROK )
				ShowPonPortOnuMacAddrByVty_1(  PonPortIdx, OnuTypeString, vty );
			}

		for(PonPortIdx =0; PonPortIdx < 4; PonPortIdx ++ )
			{
			if( SearchValidOnuEntry(PonPortIdx) == ROK )
				ShowPonPortOnuMacAddrByVty_1(  PonPortIdx, OnuTypeString, vty );
			}
		}
	else if(Product_type == V2R1_OLT_GFA6100)
#endif
	{
		for(PonPortIdx =0; PonPortIdx < MAXPON; PonPortIdx ++ )
		{
			if( SearchValidOnuEntry(PonPortIdx) == ROK )
				/*ShowPonPortOnuMacAddrByVty_1(  PonPortIdx, OnuTypeString, vty );*/
				show_pon_port_onu_list( vty, PonPortIdx, OnuTypeString );
		}
	}

	return( ROK );

}

int ShowPonPortOnuMacAddrCounterByVty( short int PonPortIdx , unsigned char *OnuTypeString, struct vty *vty)
{
	int slot, port;
	/*short int OnuIdx;*/
	int config_onu = 0;
	
	CHECK_PON_RANGE;

	slot = GetCardIdxByPonChip(PonPortIdx);
	if( slot == RERROR )
		return RERROR;
	port = GetPonPortByPonChip(PonPortIdx);

	if(OnuTypeString == NULL )
		config_onu = AllOnuCounter(PonPortIdx);
	else config_onu = AllOnuCounterByType(PonPortIdx, OnuTypeString);

	if( OnuTypeString == NULL )
		vty_out( vty, "\r\n  %s/port%d total onu counter=%d\r\n", CardSlot_s[slot], port, config_onu);
	else 
		vty_out( vty, "\r\n  %s/port%d total onu(%s) counter=%d\r\n", CardSlot_s[slot], port, OnuTypeString, config_onu);
	return( ROK );

	/*
	if( getPonChipInserted((unsigned char)slot, (unsigned char)port ) !=  PONCHIP_EXIST )
		{
		vty_out(vty, "\r\n  %s/port%d not exist\r\n", CardSlot_s[slot], (port +1) );
		return( ROK );
		}
	
		
	if( PonPortIsWorking(PonPortIdx) == TRUE )
	
			{
			for( OnuIdx =0;OnuIdx < MAXONUPERPON; OnuIdx ++)
				{
				if(ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
					{
					config_onu ++;
					}
				}
			vty_out( vty, "\r\n  %s/port%d total onu counter=%d\r\n", CardSlot_s[slot], (port +1), config_onu);
			return( ROK );
			}
	
	else{
		vty_out(vty, "\r\n  %s/port%d not working\r\n", CardSlot_s[slot], (port +1) );
		return( ROK );
		}
	*/
}

int ShowPonPortOnuMacAddrCounterByVtyAll(unsigned char *OnuTypeString, struct vty *vty)
{

	short int PonPortIdx;
	/*short int OnuIdx;*/

	all_config_onu = 0;

	for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx ++ )
		{
		/*if( PonPortIsWorking(PonPortIdx) == TRUE )*/
			{
			if( OnuTypeString == NULL )
				all_config_onu += AllOnuCounter(PonPortIdx);
			else all_config_onu += AllOnuCounterByType(PonPortIdx, OnuTypeString);
			/*
			for( OnuIdx =0;OnuIdx < MAXONUPERPON; OnuIdx ++)
				{
				if(ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
					{
					all_config_onu ++;
					}
				}
			*/
			}
		}
	if( OnuTypeString == NULL )
		vty_out(vty, "\r\n  total onu counter=%d\r\n", all_config_onu);
	else 
		vty_out(vty, "\r\n  total onu(%s) counter=%d\r\n", OnuTypeString, all_config_onu);
	return( ROK );

}

/* modified by xieshl 20110111, 合并online显示函数 */
void show_onu_online_banner( struct vty *vty )
{
	int i, DeviceTypeLen = GetDeviceTypeLength();
	
	vty_out(vty, "Idx  Mac addr/SN         type");
	for( i=0; i<DeviceTypeLen - 4; i++ )
		vty_out(vty, " ");
	vty_out(vty,"mode(Down/Up)   ");/*modified by wangjiah@2016-09-18 to append pon rate*/
	vty_out(vty,"running-time   userName\r\n");
	vty_out(vty, "----------------------------------------------------------------------------\r\n");
}
int show_pon_port_onu_online( struct vty *vty, short int PonPortIdx, unsigned char *OnuTypeString )
{
	short int OnuIdx, OnuEntry;
	unsigned char name[MAXDEVICENAMELEN+1];
	int nameLen;
	int counter=0;
	int slot, port;
	int PonChipType = 0;
	/*int OnuType;*/
	ULONG /*CurTicks,*/ SysUpDelayed;
	ULONG days, hours,minutes, seconds;
	
	CHECK_PON_RANGE;
	
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	PonChipType = V2R1_GetPonchipType(PonPortIdx); 

	if( getPonChipInserted((unsigned char)slot, (unsigned char)port) !=  PONCHIP_EXIST )
		return( ROK );
	
	if( PonPortIsWorking(PonPortIdx) != TRUE )
		return( RERROR );

	if( (counter = OnLineOnuCounterByType( PonPortIdx, OnuTypeString)) == 0 )
		return( ROK );

	if( OnuTypeString == NULL )
		vty_out(vty, "pon%d/%d [onu counter = %d]\r\n", slot, port, counter);
	else 
		vty_out(vty, "pon%d/%d [onu(%s) counter = %d]\r\n", slot, port, OnuTypeString, counter);
	
/*	ONU_MGMT_SEM_TAKE;*/
	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
		if( (GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP )/*&&(OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus ==  LLID_ENTRY_ACTIVE)*/)
		{
			if( OnuTypeString )
			{
                if(OnuMgmtTable[OnuEntry].DeviceInfo.type!= V2R1_ONU_CTC && OnuMgmtTable[OnuEntry].DeviceInfo.type!= V2R1_ONU_GPON && VOS_StriCmp(OnuTypeString,GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)) != 0 ) 
                    continue;
                else
                {
                    if(GetOnuModel(PonPortIdx, OnuIdx, name, &nameLen) != ROK)
                        continue;
                    else if(VOS_StriCmp(OnuTypeString, name))
                        continue;
                }
			    /*
			    if( GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK )
					continue;
				if( VOS_StriCmp(OnuTypeString, GetDeviceTypeString(OnuType)) != 0 ) 
					continue;
					*/
			}
			
			vty_out(vty, "%2d   ", (OnuEntry % MAXONUPERPON + 1));
			if(SYS_MODULE_IS_GPON(slot))
                vty_out(vty, "%16s  ", OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No);
			else    
			    vty_out(vty, "  %02x%02x.%02x%02x.%02x%02x  ", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] );

			if( GetOnuModel( PonPortIdx, OnuIdx, name, &nameLen) != ROK )
				/*VOS_StrCpy( name, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );*/
				VOS_StrCpy( name, "-" );
			vty_out( vty, "  %-12s  ", name );

			/*added by wangjiah@2016-09-18:begin*/
			/*append pon rate after onu type*/
			if(OnuMgmtTable[OnuEntry].DeviceInfo.type == V2R1_ONU_GPON) 
			{
				VOS_StrCpy( name, "(GPON)");
			}
			else
			{
				switch (OnuMgmtTable[OnuEntry].PonRate)
				{
					case PON_RATE_10_10G:
						VOS_StrCpy( name, "10G/10G");
						break;
					case PON_RATE_1_10G:
						VOS_StrCpy( name, "10G/1G");
						break;
					case PON_RATE_1_2G:
						VOS_StrCpy( name, "2G/1G");
						break;
					case PON_RATE_NORMAL_1G:
						VOS_StrCpy( name, "1G/1G");
						break;
					default:
						VOS_StrCpy( name, "--/--");
						break;
				}
			}
			vty_out( vty, " %-10s ", name);
			/*added by wangjiah@2016-09-18:end*/

			/*CurTicks = VOS_GetTick();
			SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;

			seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;*/
			SysUpDelayed = time(0) - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
			days = SysUpDelayed /V2R1_ONE_DAY;
			hours = (SysUpDelayed % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
			minutes = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
			seconds = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;

			vty_out(vty,"  %04u:%02u:%02u:%02u",days, hours, minutes, seconds );	/* modified by xieshl 20100319, GWD网上问题9966 */

			GetOnuDeviceName( PonPortIdx, OnuIdx, name, &nameLen);
			if(nameLen > MAXDEVICENAMELEN ) nameLen = MAXDEVICENAMELEN;
			name[nameLen] = '\0';
			vty_out(vty, "  %s\r\n", name); 
		}
	}
/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty, "\r\n");

	return( ROK );
}

/* modified by chenfj 2008-3-20 #6349
     在显示在线ONU  列表时, 增加ONU 类型可选参数, 这样可以只显示指定类型的ONU
     */
int ShowPonPortOnLineOnuByVty( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty )
{
#if 1
	int counter = OnLineOnuCounterByType( PonPortIdx, OnuTypeString);
	if( counter == 0 ) 
	{
		vty_out(vty, "\r\npon%d/%d [online onu counter = %d]\r\n\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), counter);
		return( ROK );
	}
	show_onu_online_banner( vty );
	return show_pon_port_onu_online( vty, PonPortIdx, OnuTypeString );
#else
	short int OnuIdx, OnuEntry,i;
	unsigned char name[MAXDEVICENAMELEN+1];
	int nameLen;
	int counter=0;
	int slot, port;
	unsigned int DeviceTypeLen = 0;
	int OnuType;
	
	CHECK_PON_RANGE;

	slot = GetCardIdxByPonChip(PonPortIdx );
	if( slot == RERROR )
		return RERROR;
	port = GetPonPortByPonChip(PonPortIdx);

	DeviceTypeLen = GetDeviceTypeLength();

	if( getPonChipInserted((unsigned char )slot, (unsigned char)port) !=  PONCHIP_EXIST )
		{
		vty_out(vty, "\r\n  %s/port%d not exist\r\n\r\n", CardSlot_s[slot], port );
		return( ROK );
		}
	/*
	if(( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_DOWN) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UNKNOWN ) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_INIT ) )
	*/
	if( PonPortIsWorking(PonPortIdx) != TRUE )
		{
		vty_out(vty, "\r\n  %s/port%d is not Working \r\n\r\n",CardSlot_s[slot], port);
		return( RERROR );
		}
	/*vty_out(vty, "\r\n %s/port%d Online Onu list \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], (GetPonPortByPonChip(PonPortIdx) +1));*/

	counter = OnLineOnuCounterByType( PonPortIdx, OnuTypeString);
	
	if( counter >= 0 ) 
		{
		if(OnuTypeString == NULL )
			vty_out(vty, "\r\n%s/port%d [online onu counter = %d]\r\n\r\n", CardSlot_s[slot], port, counter);
		else 
			vty_out(vty, "\r\n%s/port%d [online onu(%s) counter = %d]\r\n\r\n", CardSlot_s[slot], port, OnuTypeString, counter);	
		if( counter == 0)
			return( ROK );
		}
	else return( ROK );
	
	/*vty_out(vty, "  Max onu supported: %d \r\n", PonPortTable[PonPortIdx].MaxOnu  );*/
	/*OnLineOnuCounter( PonPortIdx) ;
	vty_out(vty, "  online Onu counter: %d\r\n",PonPortTable[PonPortIdx].CurrOnu);*/
	vty_out(vty, "Idx  Mac addr        type     ");
	for(i=0;i<(DeviceTypeLen - 5);i++)
				vty_out(vty," ");
	vty_out(vty,"running-time   userName    \r\n");
	vty_out(vty, "------------------------------------------------------------------\r\n");
	
	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
		{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
		if( (GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP )/*&&(OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus ==  LLID_ENTRY_ACTIVE)*/)
			{
			if(OnuTypeString != NULL )
				{
				if(GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK ) continue;
				if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType), VOS_StrLen(OnuTypeString)) != 0) continue;
				}
			
			vty_out(vty, "%2d   ", (OnuEntry % MAXONUPERPON + 1));
			vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] );

			if( GetOnuModel( PonPortIdx, OnuIdx, name, &nameLen) != ROK )
				VOS_StrCpy( name, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );
			vty_out( vty, "  %-12s  ", name );

			{
			ULONG CurTicks, SysUpDelayed;
			ULONG days, hours,minutes, seconds;
			/*Date_S UpDate;*/
		
			CurTicks = VOS_GetTick();
			SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;

			seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;
			days = seconds /V2R1_ONE_DAY;
			hours = (seconds % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
			minutes = ((seconds % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
			seconds = ((seconds % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;

			/*GetOnuLaunchTime( PonPortIdx, OnuIdx, &UpDate);		
			vty_out(vty,"  %04d-%02d-%02d,%d:%d:%d",UpDate.year, UpDate.month, UpDate.day, UpDate.hour, UpDate.minute, UpDate.second);*/
			vty_out(vty,"  %04u:%02u:%02u:%02u",days, hours, minutes, seconds );	/* modified by xieshl 20100319, GWD网上问题9966 */

			GetOnuDeviceName( PonPortIdx, OnuIdx, name, &nameLen);
			if(nameLen > MAXDEVICENAMELEN ) nameLen = MAXDEVICENAMELEN;
			name[nameLen] = '\0';
			vty_out(vty, "  %s\r\n", name); 
			
			}

			counter++;			
			}
		}
	vty_out(vty, "\r\n");
	/*
	vty_out(vty, "  End of list,online onu counter=%d\r\n" , counter);
	all_online_onu += counter; */
	return( ROK );
#endif
}
#if 0
/* modified by chenfj 2007/02/08 
    问题单#3634.配置节点下的命令show online-onu的显示内容需要改进。
    使用ShowPonPortOnLineOnuByVty_1() 替代ShowPonPortOnLineOnuByVty() 
    */
int ShowPonPortOnLineOnuByVty_1( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty )
{
	short int OnuIdx, OnuEntry/*,i*/;
	unsigned char name[MAXDEVICENAMELEN+1];
	int nameLen;
	int counter=0;
	int slot, port;
	int DeviceTypeLen = 0;
	int OnuType;
	
	CHECK_PON_RANGE;
	
	slot = GetCardIdxByPonChip(PonPortIdx );
	port = GetPonPortByPonChip(PonPortIdx);

	DeviceTypeLen = GetDeviceTypeLength();

	if( getPonChipInserted((unsigned char)slot, (unsigned char)port) !=  PONCHIP_EXIST )
		{
		return( ROK );
		}
	
	/*if(( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_DOWN) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UNKNOWN ) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_INIT ) )*/
	if( PonPortIsWorking(PonPortIdx) != TRUE )
		{
		return( RERROR );
		}

	counter = OnLineOnuCounterByType( PonPortIdx, OnuTypeString);
	
	if( counter > 0 )
		{
		if( OnuTypeString == NULL )
			vty_out(vty, "%s/port%d [online onu counter = %d]\r\n", CardSlot_s[slot], port, counter);
		else vty_out(vty,"%s/port%d [online onu(%s) counter = %d]\r\n", CardSlot_s[slot], port, OnuTypeString, counter);
		}
	else return( ROK );
	
	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
		{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
		if( (GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP )/*&&(OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus ==  LLID_ENTRY_ACTIVE)*/)
			{
			if(OnuTypeString != NULL )
				{
				if(GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK ) continue;
				if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType), VOS_StrLen(OnuTypeString)) != 0) continue;
				}
			
			vty_out(vty, "%2d   ", (OnuEntry % MAXONUPERPON + 1));
			vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] );

			if( GetOnuModel( PonPortIdx, OnuIdx, name, &nameLen) != ROK )
				VOS_StrCpy( name, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );
			vty_out( vty, "  %-12s  ", name );

			{
			ULONG CurTicks, SysUpDelayed;
			ULONG days, hours,minutes, seconds;
			/*Date_S UpDate;*/
		
			CurTicks = VOS_GetTick();
			SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;

			seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;
			days = seconds /V2R1_ONE_DAY;
			hours = (seconds % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
			minutes = ((seconds % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
			seconds = ((seconds % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;

			/*GetOnuLaunchTime( PonPortIdx, OnuIdx, &UpDate);		
			vty_out(vty,"  %04d-%02d-%02d,%d:%d:%d",UpDate.year, UpDate.month, UpDate.day, UpDate.hour, UpDate.minute, UpDate.second);*/
			vty_out(vty,"  %04u:%02u:%02u:%02u",days, hours, minutes, seconds );	/* modified by xieshl 20100319, GWD网上问题9966 */

			GetOnuDeviceName( PonPortIdx, OnuIdx, name, &nameLen);
			if(nameLen > MAXDEVICENAMELEN ) nameLen = MAXDEVICENAMELEN;
			name[nameLen] = '\0';
			vty_out(vty, "  %s\r\n", name); 
			
			}		
			}
		}
	/*vty_out(vty, "  End of list,online onu counter=%d\r\n" , counter);*/
	vty_out(vty, "\r\n");
	return( ROK );
}
#endif

int ShowPonPortOnLineOnuByVtyAll(unsigned char *OnuTypeString, struct vty *vty )
{
	short int PonPortIdx;
	/*int DeviceTypeLen = 0;
	int i;*/
	/* int Product_type = GetOltType(); */

	all_online_onu =0;
	
	/*DeviceTypeLen = GetDeviceTypeLength();*/

	for( PonPortIdx = 0;PonPortIdx < MAXPON; PonPortIdx ++)
	{
		all_online_onu += OnLineOnuCounterByType(PonPortIdx, OnuTypeString);
	}
	if(OnuTypeString == NULL)
		vty_out( vty, "\r\n  [TOTAL ONLINE ONU COUNTER = %d]\r\n\r\n", all_online_onu);
	else vty_out(vty,"\r\n  [TOTAL ONLINE ONU(%s) COUNTER = %d]\r\n\r\n", OnuTypeString, all_online_onu);
	
	if( all_online_onu == 0 ) return( ROK );

	/*vty_out(vty, "\r\nIdx  Mac addr        type     ");
	for(i=0;i<(DeviceTypeLen - 5);i++)
		vty_out(vty," ");
	vty_out(vty,"running-time   userName    \r\n");
	vty_out(vty, "----------------------------------------------------------------\r\n");*/
	show_onu_online_banner( vty );

#if 0
	if(Product_type == V2R1_OLT_GFA6700)
		{
		for(PonPortIdx =16; PonPortIdx < MAXPON; PonPortIdx ++ )
			{
			if( PonPortIsWorking(PonPortIdx) == TRUE )
				ShowPonPortOnLineOnuByVty_1(  PonPortIdx, OnuTypeString, vty );
			}

		for(PonPortIdx =12; PonPortIdx < 16; PonPortIdx ++ )
			{
			if( PonPortIsWorking(PonPortIdx) == TRUE )
				ShowPonPortOnLineOnuByVty_1(  PonPortIdx, OnuTypeString, vty );
			}

		for(PonPortIdx =8; PonPortIdx < 12; PonPortIdx ++ )
			{
			if( PonPortIsWorking(PonPortIdx) == TRUE )
				ShowPonPortOnLineOnuByVty_1(  PonPortIdx, OnuTypeString, vty );
			}

		for(PonPortIdx =4; PonPortIdx < 8; PonPortIdx ++ )
			{
			if( PonPortIsWorking(PonPortIdx) == TRUE )
				ShowPonPortOnLineOnuByVty_1(  PonPortIdx, OnuTypeString, vty );
			}

		for(PonPortIdx =0; PonPortIdx < 4; PonPortIdx ++ )
			{
			if( PonPortIsWorking(PonPortIdx) == TRUE )
				ShowPonPortOnLineOnuByVty_1(  PonPortIdx, OnuTypeString, vty );
			}
		}
	else if(Product_type == V2R1_OLT_GFA6100)
#endif
	{
		for(PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx++ )
		{
			if( PonPortIsWorking(PonPortIdx) == TRUE )
				/*ShowPonPortOnLineOnuByVty_1(  PonPortIdx, OnuTypeString, vty );*/
				show_pon_port_onu_online( vty, PonPortIdx, OnuTypeString );
		}
	}
	/*vty_out(vty, "\r\n  Total Online Onu counter=%d\r\n", all_online_onu );*/

	return( ROK );
}

int ShowPonPortOnLineOnuCounterByVty( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty )
{
	int slot, port;
	short int OnuIdx;
	int online_onu = 0;
	int OnuType;
	
	CHECK_PON_RANGE	

	slot = GetCardIdxByPonChip(PonPortIdx );
	if( slot == RERROR )
		return RERROR;
	port = GetPonPortByPonChip(PonPortIdx);

	if( getPonChipInserted((unsigned char)slot, (unsigned char)port) !=  PONCHIP_EXIST )
	{
		vty_out(vty, "\r\n  %s/port%d not exist\r\n", CardSlot_s[slot], port );
		return( ROK );
	}
	
		
	if( PonPortIsWorking(PonPortIdx) == TRUE )
	{
		for( OnuIdx =0;OnuIdx < MAXONUPERPON; OnuIdx ++)
		{
			if( GetOnuOperStatus( PonPortIdx , OnuIdx ) == ONU_OPER_STATUS_UP )
			{
				if( OnuTypeString == NULL )
				{
					online_onu ++;
				}
				else{
					if( GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK ) continue;
					if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType), VOS_StrLen(OnuTypeString)) == 0)
						online_onu ++;
				}
			}
		}
		if( OnuTypeString == NULL )
			vty_out( vty, "\r\n  %s/port%d total online onu counter=%d\r\n", CardSlot_s[slot], port, online_onu);
		else 
			vty_out( vty, "\r\n  %s/port%d total online onu(%s) counter=%d\r\n", CardSlot_s[slot], port, OnuTypeString, online_onu);
	}
	else{
		vty_out(vty, "\r\n  %s/port%d not working\r\n", CardSlot_s[slot], port );
	}
	return( ROK );
}

/*show online-onu count  显示当前在线的所有ONU的个数； */
int ShowPonPortOnLineOnuCounterByVtyAll(unsigned char *OnuTypeString, struct vty *vty )
{

	short int PonPortIdx;
	short int OnuIdx;
	/*int OnuType;*/

    char typeStr[ONU_TYPE_LEN+2];
    int typeStrLen;

	all_online_onu = 0;

	for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx ++ )
		{
		if( PonPortIsWorking(PonPortIdx) == TRUE )
			{
			for( OnuIdx =0;OnuIdx < MAXONUPERPON; OnuIdx ++)
				{
				if( GetOnuOperStatus( PonPortIdx , OnuIdx ) == ONU_OPER_STATUS_UP )
					{
					if( OnuTypeString == NULL )
						{
						all_online_onu ++;
						}
					else{
		                typeStr[0] = 0;
		                typeStrLen = 0;
		                if( (GetOnuTypeString(PonPortIdx, OnuIdx, typeStr, &typeStrLen) == ROK) || (typeStrLen != 0) )
		                {
		                    if( VOS_StriCmp(OnuTypeString, typeStr) == 0)
		                        all_online_onu ++;
		                }
					}
		                /*
						if( GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK ) continue;
						if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType), VOS_StrLen(OnuTypeString)) == 0)
							all_online_onu ++;
						}
						*/
					}
				}
			}
		}
	if( OnuTypeString == NULL )
		vty_out(vty, "\r\n  total online onu counter=%d\r\n", all_online_onu);
	else 
		vty_out(vty, "\r\n  total online onu(%s) counter=%d\r\n", OnuTypeString, all_online_onu);
	return( ROK );

}
/*add by shixh 20090429*/
int ShowPonPortOffLineOnuByVty( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty )
{
	short int OnuIdx, OnuEntry,i;
	unsigned char name[MAXDEVICENAMELEN+1];
	int nameLen;
	int counter=0;
	int slot, port;
	unsigned int DeviceTypeLen = 0;
	int OnuType;
	
	CHECK_PON_RANGE

	slot = GetCardIdxByPonChip(PonPortIdx );
	if( slot == RERROR )
		return RERROR;
	port = GetPonPortByPonChip(PonPortIdx);

	DeviceTypeLen = GetDeviceTypeLength();

	/*if( getPonChipInserted((unsigned char )slot, (unsigned char )port ) !=  PONCHIP_EXIST )
		{
		vty_out(vty, "\r\n  %s/port%d not exist\r\n\r\n", CardSlot_s[slot], port );
		return( ROK );
		}*/
	/*
	if(( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_DOWN) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UNKNOWN ) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_INIT ) )
	*/
	/*if( PonPortIsWorking(PonPortIdx) != TRUE )
		{
		vty_out(vty, "\r\n  %s/port%d is not Working \r\n\r\n",CardSlot_s[slot], port);
		return( RERROR );
		}*/
	/*vty_out(vty, "\r\n %s/port%d Online Onu list \r\n", CardSlot_s[slot], port);*/

	counter = OffLineOnuCounterByType( PonPortIdx, OnuTypeString);
	
	if( counter >= 0 ) 
		{
		if(OnuTypeString == NULL )
			vty_out(vty, "\r\n%s/port%d [offline onu counter = %d]\r\n\r\n", CardSlot_s[slot], port, counter);
		else 
			vty_out(vty, "\r\n%s/port%d [offline onu(%s) counter = %d]\r\n\r\n", CardSlot_s[slot], port, OnuTypeString, counter);	
		if( counter == 0)
			return( ROK );
		}
	else return( ROK );
	
	vty_out(vty, "Idx  Mac addr/SN        type     ");
	for(i=0;i<(DeviceTypeLen - 5);i++)
				vty_out(vty," ");
	vty_out(vty,"running-time   userName    \r\n");
	vty_out(vty, "------------------------------------------------------------------\r\n");
/*	ONU_MGMT_SEM_TAKE;*/
	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
		if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) continue;
		if(GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_DOWN )
			{
			if(OnuTypeString != NULL )
			{
				if(GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK ) continue;
				if(OnuType != V2R1_ONU_CTC && OnuType != V2R1_ONU_GPON)
				{
					if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType)/*DeviceType_1[OnuType]*/, VOS_StrLen(OnuTypeString)) != 0) continue;
				}
				else
				{
					if( GetOnuModel( PonPortIdx, OnuIdx, name, &nameLen) != ROK )
						continue;
					else if(VOS_StrniCmp(OnuTypeString, name, VOS_StrLen(OnuTypeString)))
						continue;						
				}
			}
			
			vty_out(vty, "%2d   ", (OnuEntry % MAXONUPERPON + 1));
			/*for(i=0;i<( BYTES_IN_MAC_ADDRESS-1); i++ )
				{
				vty_out(vty, "%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i+1] );
				i++;
				if( i != 5 )vty_out(vty, ".");
				}*/
			if(SYS_MODULE_IS_GPON(slot))
                vty_out(vty, "  %16s  ", OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No);
			else    
			    vty_out(vty, "  %02x%02x.%02x%02x.%02x%02x  ", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] );
#if 0
			if( OnuMgmtTable[OnuEntry].DeviceInfo.type != V2R1_OTHER )
				vty_out(vty, "  %s  ", GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)/*DeviceType_1[OnuMgmtTable[OnuEntry].DeviceInfo.type]*/);
			else vty_out( vty, "  %s  ",GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)/*DeviceType_1[OnuMgmtTable[OnuEntry].DeviceInfo.type]*/);
			for(i=0;i<(DeviceTypeLen - VOS_StrLen(GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)/*DeviceType_1[OnuMgmtTable[OnuEntry].DeviceInfo.type]*/));i++)
				vty_out(vty," ");
#endif			
			if( GetOnuModel( PonPortIdx, OnuIdx, name, &nameLen) != ROK )
				VOS_StrCpy( name, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );
			vty_out( vty, "  %-12s  ", name );

			{
			ULONG /*CurTicks,*/ SysUpDelayed;
			ULONG days, hours,minutes, seconds;
			/*Date_S UpDate;*/
		
			/*CurTicks = VOS_GetTick();
			SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;

			seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;
			days = 0;
			hours = 0;
			minutes = 0;*/
			SysUpDelayed = time(0) - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
			days = SysUpDelayed /V2R1_ONE_DAY;
			hours = (SysUpDelayed % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
			minutes = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
			seconds = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;

			/*GetOnuLaunchTime( PonPortIdx, OnuIdx, &UpDate);		
			vty_out(vty,"  %04d-%02d-%02d,%d:%d:%d",UpDate.year, UpDate.month, UpDate.day, UpDate.hour, UpDate.minute, UpDate.second);*/
			vty_out(vty,"  %04u:%02u:%02u:%02u",days, hours, minutes, seconds );	/* modified by xieshl 20100319, GWD网上问题9966 */

			GetOnuDeviceName( PonPortIdx, OnuIdx, name, &nameLen);
			if(nameLen > MAXDEVICENAMELEN ) nameLen = MAXDEVICENAMELEN;
			name[nameLen] = '\0';
			vty_out(vty, "  %s\r\n", name); 
			
			}
			counter++;			
		}
	}
/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty, "\r\n");
	return( ROK );
}
int ShowPonPortOffLineOnuByVty_1( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty )
{
	short int OnuIdx, OnuEntry/*,i*/;
	unsigned char name[MAXDEVICENAMELEN+1];
	int nameLen;
	int counter=0;
	int slot, port;
	int DeviceTypeLen = 0;
	/*int OnuType;*/
	
	CHECK_PON_RANGE
	
	slot = GetCardIdxByPonChip(PonPortIdx );
	if( slot == RERROR )
		return RERROR;
	port = GetPonPortByPonChip(PonPortIdx);

	DeviceTypeLen = GetDeviceTypeLength();

	/*if( getPonChipInserted((unsigned char)slot, (unsigned char)port ) !=  PONCHIP_EXIST )
		{
		return( ROK );
		}*/
	
	/*if(( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_DOWN) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UNKNOWN ) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_INIT ) )*/
	/*if( PonPortIsWorking(PonPortIdx) != TRUE )
		{
		return( RERROR );
		}*/

	counter = OffLineOnuCounterByType( PonPortIdx, OnuTypeString);
	
	if( counter > 0 )
		{
		if( OnuTypeString == NULL )
			vty_out(vty, "%s/port%d [offline onu counter = %d]\r\n", CardSlot_s[slot], port, counter);
		else vty_out(vty,"%s/port%d [offline onu(%s) counter = %d]\r\n", CardSlot_s[slot], port, OnuTypeString, counter);
		}
	else return( ROK );
	
/*	ONU_MGMT_SEM_TAKE;*/
	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
		if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) continue;
		if(GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_DOWN )
		{
			if(OnuTypeString != NULL )
				{
                if(OnuMgmtTable[OnuEntry].DeviceInfo.type!= V2R1_ONU_GPON && OnuMgmtTable[OnuEntry].DeviceInfo.type!= V2R1_ONU_CTC && VOS_StriCmp(OnuTypeString,GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)) != 0 ) continue;
                else
                {
                    if(GetOnuModel(PonPortIdx, OnuIdx, name, &nameLen) != ROK)
                        continue;
                    else if(VOS_StriCmp(OnuTypeString, name))
                        continue;
                }
#if 0
			    if(GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK ) continue;
				if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType)/*DeviceType_1[OnuType]*/, VOS_StrLen(OnuTypeString)) != 0) continue;
#endif
				}
			
			vty_out(vty, "%2d   ", (OnuEntry % MAXONUPERPON + 1));
			/*for(i=0;i<( BYTES_IN_MAC_ADDRESS-1); i++ )
				{
				vty_out(vty, "%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i+1] );
				i++;
				if( i != 5 )vty_out(vty, ".");
				}*/
			if(SYS_MODULE_IS_GPON(slot))
                vty_out(vty, "%16s  ", OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No);
			else    
			    vty_out(vty, "  %02x%02x.%02x%02x.%02x%02x  ", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] );
#if 0
			if( OnuMgmtTable[OnuEntry].DeviceInfo.type != V2R1_OTHER )
				vty_out(vty, "  %s  ", GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)/*DeviceType_1[OnuMgmtTable[OnuEntry].DeviceInfo.type]*/);
			else vty_out( vty, "  %s  ", GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)/*DeviceType_1[OnuMgmtTable[OnuEntry].DeviceInfo.type]*/);
			for(i=0; i<(DeviceTypeLen -VOS_StrLen(GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)/*DeviceType_1[OnuMgmtTable[OnuEntry].DeviceInfo.type]*/)); i++)
				vty_out(vty," ");
#endif
			if( GetOnuModel( PonPortIdx, OnuIdx, name, &nameLen) != ROK )
				VOS_StrCpy( name, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );
			vty_out( vty, "  %-12s  ", name );

			{
			ULONG /*CurTicks,*/ SysUpDelayed;
			ULONG days, hours,minutes, seconds;
			/*Date_S UpDate;*/
		
			/*CurTicks = VOS_GetTick();
			SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;

			seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;
			days = 0;
			hours = 0;
			minutes = 0;*/
			SysUpDelayed = time(0) - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
			days = SysUpDelayed /V2R1_ONE_DAY;
			hours = (SysUpDelayed % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
			minutes = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
			seconds = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;

			/*GetOnuLaunchTime( PonPortIdx, OnuIdx, &UpDate);		
			vty_out(vty,"  %04d-%02d-%02d,%d:%d:%d",UpDate.year, UpDate.month, UpDate.day, UpDate.hour, UpDate.minute, UpDate.second);*/
			vty_out(vty,"  %04u:%02u:%02u:%02u",days, hours, minutes, seconds );	/* modified by xieshl 20100319, GWD网上问题9966 */

			GetOnuDeviceName( PonPortIdx, OnuIdx, name, &nameLen);
			if(nameLen > MAXDEVICENAMELEN ) nameLen = MAXDEVICENAMELEN;
			name[nameLen] = '\0';
			vty_out(vty, "  %s\r\n", name); 
			
			}		
		}
	}
/*	ONU_MGMT_SEM_GIVE;*/
	/*vty_out(vty, "  End of list,online onu counter=%d\r\n" , counter);*/
	vty_out(vty, "\r\n");
	return( ROK );
}
int ShowPonPortOffLineOnuByVtyAll(unsigned char *OnuTypeString, struct vty *vty )
{
	short int PonPortIdx;
	int DeviceTypeLen = 0;
	int i;
	/* int Product_type = GetOltType(); */

	all_offline_onu =0;
	
	DeviceTypeLen = GetDeviceTypeLength();

	for( PonPortIdx = 0;PonPortIdx < MAXPON; PonPortIdx ++)
	{
		all_offline_onu += OffLineOnuCounterByType(PonPortIdx, OnuTypeString);
	}
	if(OnuTypeString == NULL)
		vty_out( vty, "\r\n  [TOTAL OFFLINE ONU COUNTER = %d]\r\n", all_offline_onu);
	else vty_out(vty,"\r\n  [TOTAL OFFLINE ONU(%s) COUNTER = %d]\r\n", OnuTypeString, all_offline_onu);
	
	if( all_offline_onu == 0 ) return( ROK );

	vty_out(vty, "\r\nIdx  Macaddr/SN        type     ");
	for(i=0;i<(DeviceTypeLen - 5);i++)
		vty_out(vty," ");
	vty_out(vty,"running-time   userName    \r\n");
	vty_out(vty, "----------------------------------------------------------------\r\n");

#if 0
	if(Product_type == V2R1_OLT_GFA6700)
		{
		for(PonPortIdx =16; PonPortIdx < MAXPON; PonPortIdx ++ )
			{
			if( SearchValidOnuEntry(PonPortIdx) == ROK )
				ShowPonPortOffLineOnuByVty_1(  PonPortIdx, OnuTypeString, vty );
			}

		for(PonPortIdx =12; PonPortIdx < 16; PonPortIdx ++ )
			{
			if( SearchValidOnuEntry(PonPortIdx) == ROK )
				ShowPonPortOffLineOnuByVty_1(  PonPortIdx, OnuTypeString, vty );
			}

		for(PonPortIdx =8; PonPortIdx < 12; PonPortIdx ++ )
			{
			if( SearchValidOnuEntry(PonPortIdx) == ROK )
				ShowPonPortOffLineOnuByVty_1(  PonPortIdx, OnuTypeString, vty );
			}

		for(PonPortIdx =4; PonPortIdx < 8; PonPortIdx ++ )
			{
			if( SearchValidOnuEntry(PonPortIdx) == ROK )
				ShowPonPortOffLineOnuByVty_1(  PonPortIdx, OnuTypeString, vty );
			}

		for(PonPortIdx =0; PonPortIdx < 4; PonPortIdx ++ )
			{
			if( SearchValidOnuEntry(PonPortIdx) == ROK )
				ShowPonPortOffLineOnuByVty_1(  PonPortIdx, OnuTypeString, vty );
			}
		}
	else if(Product_type == V2R1_OLT_GFA6100)
#endif
	{
		for(PonPortIdx =0; PonPortIdx < MAXPON; PonPortIdx ++ )
		{
			if( SearchValidOnuEntry(PonPortIdx) == ROK )
				ShowPonPortOffLineOnuByVty_1(  PonPortIdx, OnuTypeString, vty );
		}
	}

	return( ROK );

}

int ShowPonPortOffLineOnuCounterByVty( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty )
{
	int slot, port;
	short int OnuIdx;
	int offline_onu = 0;
	int OnuType;
	
	CHECK_PON_RANGE

	slot = GetCardIdxByPonChip(PonPortIdx );
	if( slot == RERROR )
		return RERROR;
	port = GetPonPortByPonChip(PonPortIdx);

	/*if( getPonChipInserted((unsigned char)slot, (unsigned char)port ) !=  PONCHIP_EXIST )
		{
		vty_out(vty, "\r\n  %s/port%d not exist\r\n", CardSlot_s[slot], port );
		return( ROK );
		}*/
	
		
	/*if( PonPortIsWorking(PonPortIdx) == TRUE )*/
			{
			for( OnuIdx =0;OnuIdx < MAXONUPERPON; OnuIdx ++)
				{
				if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) continue;
				if( GetOnuOperStatus( PonPortIdx , OnuIdx ) == ONU_OPER_STATUS_DOWN )
					{
					if( OnuTypeString == NULL )
						{
						offline_onu ++;
						}
					else{
						if( GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK ) continue;
						if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType)/*DeviceType_1[OnuType]*/, VOS_StrLen(OnuTypeString)) == 0)
							offline_onu ++;
						}
					}
				}
			if( OnuTypeString == NULL )
				vty_out( vty, "\r\n  %s/port%d total offline onu counter=%d\r\n", CardSlot_s[slot], port, offline_onu);
			else 
				vty_out( vty, "\r\n  %s/port%d total offline onu(%s) counter=%d\r\n", CardSlot_s[slot], port, OnuTypeString, offline_onu);
			return( ROK );
			}
	/*else{
		vty_out(vty, "\r\n  %s/port%d not working\r\n", CardSlot_s[slot], (port +1) );
		return( ROK );
		}*/
}

/*show offline-onu count 显示当前离线的所有ONU 的个数*/
int ShowPonPortOffLineOnuCounterByVtyAll(unsigned char *OnuTypeString, struct vty *vty )
{

	short int PonPortIdx;
	short int OnuIdx;
	int OnuType;

	all_offline_onu = 0;

	for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx ++ )
		{
		/*if( PonPortIsWorking(PonPortIdx) == TRUE )*/
			{
			for( OnuIdx =0;OnuIdx < MAXONUPERPON; OnuIdx ++)
				{
				if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) continue;
				if( GetOnuOperStatus( PonPortIdx , OnuIdx ) == ONU_OPER_STATUS_DOWN)
					{
					if( OnuTypeString == NULL )
						{
						all_offline_onu ++;
						}
					else{
						if( GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK ) continue;
						if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType)/*DeviceType_1[OnuType]*/, VOS_StrLen(OnuTypeString)) == 0)
							all_offline_onu ++;
						}
					}
				}
			}
		}
	if( OnuTypeString == NULL )
		vty_out(vty, "\r\n  total online onu counter=%d\r\n", all_offline_onu);
	else 
		vty_out(vty, "\r\n  total online onu(%s) counter=%d\r\n", OnuTypeString, all_offline_onu);
	return( ROK );

}
void show_olt_info_banner( struct vty *vty )
{
	vty_out(vty, "           Opearation  Authorization  Authorization  Created  Registered      \r\n");
	vty_out(vty, "Slot/Port    Status       Enable          Mode        ONUs      ONUs      SFP(E/G) \r\n");
	vty_out(vty, "------------------------------------------------------------------------------\r\n");
}
extern int GetPonPortSfpState(short int PonPortIdx);

int ShowOltPonInfoByVty(short int PonPortIdx , struct vty *vty)
{
	unsigned int status = 0;
    short int slotno = GetCardIdxByPonChip(PonPortIdx);
    short int portno = GetPonPortByPonChip(PonPortIdx);
    ULONG auth_enable = 0;
    ULONG auth_mode = 0;
    int onlineCounter = 0;
    int allCounter = 0;
    char tmp[20] = {0};
	char *pStr;
    char type[15] = {0};
    char egpon[8] = {0};

	CHECK_PON_RANGE;

    VOS_Sprintf(tmp, "%d/%d", slotno, portno);
	if( status > 7 ) status = 0;    
    getOnuAuthEnable(slotno, portno, &auth_enable);

    mn_getCtcOnuAuthMode(slotno, portno, &auth_mode);
	if(mn_ctc_auth_mode_mac == auth_mode  && SYS_MODULE_IS_GPON(slotno))
	{
		pStr = "     SN      ";
	}
	else
	{
		pStr = onu_auth_mode_to_str1(auth_mode);
	}

	onlineCounter = OnLineOnuCounter(PonPortIdx );
    allCounter = AllOnuCounter(PonPortIdx);	

	/*add by yanjy2016-12*/
	if( SFP_ONLINE == GetPonPortSfpState( PonPortIdx ) )
	{
		if(VOS_OK == ponSfp_readType(PonPortIdx,egpon))
			VOS_StrCpy(type,egpon);
		else
			VOS_StrCpy(type,"UnKnow");
	}
	else
		VOS_StrCpy(type, "null");
	/*add by yanjy2016-12*/
	
    vty_out(vty, "  %-7s  %-10s  %-13s  %-13s     %-4d      %-6d  %s\r\n", tmp, PonPortIsWorking(PonPortIdx)?"    UP    ":"   DOWN   ", 
        auth_enable==V2R1_ONU_AUTHENTICATION_ENABLE?"   ENABLE    ":"   DISABLE   ",
        pStr, allCounter, onlineCounter, /*GetPonPortSfpState(PonPortIdx)?"ok":"null"*/type);	
	return VOS_OK;
}

int ShowPonPortInfoByVty(short int PonPortIdx , struct vty *vty)
{
	int i,j;
	short int PonChipType, PonChipVer = RERROR;
	unsigned int status;

	CHECK_PON_RANGE;

	PonPortFirmwareAndDBAVersionMatchByVty(PonPortIdx, vty);

#if 0
	if( PonPortIsWorking(PonPortIdx) != TRUE ) 
		{
		vty_out( vty, "%s/port%d is not exist\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		return( ROK );
		}
#endif
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
#if 0
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
#else
    PonChipVer = PonChipType;
#endif

	
#if 0
	if(( PonPortTable[PonPortIdx].PortWorkingStatus != PONPORT_UP )&&( PonPortTable[PonPortIdx].PortWorkingStatus != PONPORT_LOOP) && ( PonPortTable[PonPortIdx].PortWorkingStatus != PONPORT_UPDATE))
		{
		vty_out(vty, "\r\n%s/%d status is down, No Information about it \r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		return( ROK );
		}
#endif	

	UpdateProvisionedBWInfo( PonPortIdx);
	vty_out(vty, "\r\n  %s/port%d Information:\r\n\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
	
	vty_out(vty, "    operation status: ");
	/* modified by xieshl 20120703, SFP不匹配时不再shutdown pon口，这里应给出提示以便问题定位，问题单13757 */
	if(PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm != V2R1_ENABLE)
	{
		if( GetPonPortAdminStatus( PonPortIdx ) == V2R1_DISABLE )
			vty_out( vty, "%s\r\n", PonPortOperStatus_s[V2R1_DISABLE]);
		else	/* modified by xieshl 20091215, 增加取值范围限制，防止数组越界 */
		{
			status = PonPortTable[PonPortIdx].PortWorkingStatus;
			if( status > 7 )
				status = 0;
			vty_out(vty,"%s\r\n", PonPortOperStatus_s[status]);
		}
	}
	else
	{
		vty_out( vty, "SFP_mismatch\r\n" );
	}
	status = PonPortTable[PonPortIdx].PortAdminStatus;
	if( status > 2 )
		status = 0;
	vty_out(vty, "    admin status: %s\r\n", PonPortAdminStatus_s[status] );
#ifdef  PAS_SOFT_VERSION_V5_3_5
	{
	bool status1;
	short int ret1 = PAS_EXIT_ERROR;
#if 1
    ret1 = OLT_GetCniLinkStatus(PonPortIdx, &status1);
#else
	if(PonChipType == PONCHIP_PAS )
		ret1  = PAS_get_cni_link_status(PonPortIdx, &status );
#endif
	if( OLT_CALL_ISOK(ret1) )
		{
		vty_out(vty,"    cni status:	%s\r\n",	((status1 == TRUE) ? "up" : "down"));
		}
	}
#endif
	vty_out(vty,"\r\n");

	/* added by chenfj 2007-4-24 在显示PON 端口信息中，增加PON 芯片类型，及DBA算法版本等*/
	if( (PonChipType >= 0) && (PonChipType <= PONCHIP_TYPE_MAX) )
    {
		vty_out(vty, "    pon chip type:%s \r\n", pon_chip_typename(PonChipType));
		vty_out(vty, "    subType:%s\r\n",pon_chip_type2name(PonChipVer));
    }   

	GetDBAInfo(PonPortIdx);
	vty_out(vty, "    %s version: V%s\r\n", DBA_name, DBA_Version );
	
	vty_out(vty, "    firmware version: %s\r\n", PonChipMgmtTable[PonPortIdx].FirmwareVer );
	vty_out(vty, "    software version: %s\r\n", PonChipMgmtTable[PonPortIdx].HostSwVer );
	vty_out(vty, "    hardware version: %s\r\n", PonChipMgmtTable[PonPortIdx].HardVer );
	vty_out(vty, "    CNI port MAC bus type: %s\r\n", PON_mac_s[PonChipMgmtTable[PonPortIdx].WorkingMode.MacType ] );
	/*vty_out(vty, "    Laser on/off time(TQ) %d/%d\r\n", PonChipMgmtTable[PonPortIdx].WorkingMode.laserOnTime, PonChipMgmtTable[PonPortIdx].WorkingMode.laserOffTime );*/
	vty_out(vty, "    AGC/CDR lock time(TQ) %d/%d\r\n\r\n", PonChipMgmtTable[PonPortIdx].WorkingMode.AGCTime, PonChipMgmtTable[PonPortIdx].WorkingMode.CDRTime);

	vty_out(vty, "    max onu number: %d\r\n", ((PonPortTable[PonPortIdx].MaxOnu > MAXONUPERPON ) ? MAXONUPERPON : PonPortTable[PonPortIdx].MaxOnu) );
	/*if(PonPortTable[PonPortIdx].MaxOnu > MAXONUPERPON )
		vty_out(vty, "    max onu number: %d\r\n", MAXONUPERPON);
	else
		vty_out(vty, "    max onu number: %d\r\n", PonPortTable[PonPortIdx].MaxOnu);*/
	/*vty_out(vty, "    max llid number: %d\r\n", PonPortTable[PonPortIdx].MaxLLID);*/
	OnLineOnuCounter(PonPortIdx );
	vty_out(vty, "    current online Onu counter: %d\r\n",PonPortTable[PonPortIdx].CurrOnu );		
	/*vty_out(vty, "    current llid number: %d\r\n", PonPortTable[PonPortIdx].CurLLID );*/

	vty_out(vty, "    Current Online Onu bitMap(1-Online, 0-Offline):\r\n");
	/*vty_out(vty, "                  ");*/

    /*modi by luh@2015-6-16, 此处应该支持允许注册的最大能力*/
	for( i=0; i<(MAXONUPERPON/8)/*8*/; i++ )
    {
    	if(i%4 == 0)
			vty_out(vty, "                  ");
		
        for( j=0; j<8; j++ )
        {
            if( GetOnuOperStatus(PonPortIdx, (i*8+j)) == ONU_OPER_STATUS_UP )
            {
                vty_out(vty, "1");
            }
            else
            {
                vty_out(vty, "0");
            }
        }
        vty_out( vty, "  ");
        if((i+1)%4 == 0) vty_out(vty, "\r\n");
        /*if((i+1) == 4 ) vty_out(vty, "                  ");*/
    }
	
	/*
	for( i=0; i<8; i++ )
		{
		CurStatus = PonPortTable[PonPortIdx].OnuCurStatus[i];
		
		for( j=0; j<8; j++ )
			{
			if( (CurStatus & (unsigned char )(1<<j)) != 0 )
				{
				vty_out(vty, "1");
				}
			else {
				vty_out(vty, "0");
				}
			}
		vty_out( vty, "  ");
		if((i+1)%4 ==0) vty_out(vty, "\r\n");
		if((i+1) == 4 ) vty_out(vty, "                  ");
		}
		*/
	/*
	vty_out(vty, "    Current Online Onu bitMap: %02d %02d %02d %02d\r\n", *Ptr, *(Ptr+1), *(Ptr+2), *(Ptr+3));
	vty_out(vty, "                               %02d %02d %02d %02d\r\n", *(Ptr+4), *(Ptr+5), *(Ptr+6), *(Ptr+7));
	*/
	/*  modified by chenfj 2006/10/20 
		#2922问题单register limit的使能与show pon port info回显中的使能状态正好相反
	*/
#if 0  /* deleted by chenfj 2007-7-26 */
		{
		int flag;
		vty_out(vty, "    onu register limit:");
		if(  GetOnuRegisterLimitFlag(PonPortIdx, &flag)  == ROK ) 
			{
			if( flag == V2R1_DISABLE )
				vty_out( vty, "disable\r\n");
			else vty_out( vty, "enable\r\n");
			}
		else vty_out(vty, "err\r\n");
		}
#endif

	/*vty_out(vty, "    aps: %s\r\n", PonPortApsCtrl_s[ PonPortTable[PonPortIdx].ApsCtrl]);*/
	vty_out(vty, "    mac learning enable:%s\r\n", 
		((PonPortTable[PonPortIdx].MacSelfLearningCtrl == V2R1_ENABLE ) ? "enable" : "disable") );
	vty_out(vty, "    mac ageing time: %d(s)\r\n\r\n", (PonPortTable[PonPortIdx].MACAgeingTime/SECOND_1) );
#if 0
	vty_out(vty, "    Encrypt Direction:%s\r\n", v2r1EncryptDirection[PonPortTable[PonPortIdx].EncryptDirection]);
	vty_out(vty, "    Encrypt Key exchangeTime:%d(s) \r\n\r\n", (PonPortTable[PonPortIdx].EncryptKeyTime/SECOND_1) );
#endif
	vty_out(vty, "    max upstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].MaxBW );
	vty_out(vty, "    allocated upstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].ProvisionedBW);
#ifdef PLATO_DBA_V3
	{
	int i, n;
	unsigned int fixedBw = 0;
/*	ONU_MGMT_SEM_TAKE;*/

    n = PonPortIdx * MAXONUPERPON;
	for( i=0; i< MAXONUPERPON; i++)
	{
		if(ThisIsValidOnu(PonPortIdx, i) != ROK ) continue;
		fixedBw += OnuMgmtTable[n + i].LlidTable[0].UplinkBandwidth_fixed ;
	}
/*	ONU_MGMT_SEM_GIVE;*/
	vty_out(vty,"    (fixed bandwidth:%dKbit/s)\r\n",fixedBw);
	}
#endif
	vty_out(vty, "    left upstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].RemainBW );	
	vty_out(vty, "    Activated upstream bandwidth: %dKbit/s\r\n\r\n",PonPortTable[PonPortIdx].ActiveBW );
		
	vty_out(vty, "    max downstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].DownlinkMaxBw );
	vty_out(vty, "    allocated downstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].DownlinkProvisionedBW);
	vty_out(vty, "    left downstream bandwidth: %dKbit/s\r\n", PonPortTable[PonPortIdx].DownlinkRemainBw );
	vty_out(vty, "    Activated downstream bandwidth:");
	if( PonPortTable[PonPortIdx].DownlinkPoliceMode /* downlinkBWlimit */ == V2R1_DISABLE ){ vty_out(vty, "(No policing)");}
	else { vty_out( vty, "%dKbit/s", PonPortTable[PonPortIdx].DownlinkActiveBw);}
	vty_out(vty, "\r\n\r\n");
	
#if 0	
	vty_out(vty, "    bandwidth allocation arithmetic:");
	(PonPortTable[PonPortIdx].external_DBA_used == TRUE ) ? vty_out(vty, " DBA\r\n\r\n") : vty_out(vty, " SBA\r\n\r\n");
#endif

	/* added by chenfj 2006/12/12
	     PON MAX RANGE */
	/*if( PonPortTable[PonPortIdx].range == PON_RANGE_20KM )
		vty_out(vty, "    Onu Max Range: 20km\r\n");
	else if( PonPortTable[PonPortIdx].range == PON_RANGE_40KM )
		vty_out(vty, "    Onu Max Range: 40km\r\n");
	if( PonPortTable[PonPortIdx].range == PON_RANGE_60KM )
		vty_out(vty, "    Onu Max Range: 60km\r\n");*/
	i = 0;
	if( PonPortTable[PonPortIdx].range == PON_RANGE_20KM )
		i = 20;
	else if( PonPortTable[PonPortIdx].range == PON_RANGE_40KM )
		i = 40;
	else if( PonPortTable[PonPortIdx].range == PON_RANGE_60KM )
		i = 60;
	else if( PonPortTable[PonPortIdx].range == PON_RANGE_80KM )
		i = 80;
	if( i ) vty_out(vty, "    Onu Max Range: %dkm\r\n", i );
		
	/*vty_out(vty, "    Onu Max Range: %d, %d(m)\r\n", PonPortTable[PonPortIdx].MaxRTT, (PonPortTable[PonPortIdx].MaxRTT *32/10));*/

	vty_out(vty, "    Online Onu Max Range: %d(m)\r\n", PonPortTable[PonPortIdx].CurrRTT );
	
	/*vty_out(vty, "    Pon port periodic test timer:%d , test counter:%d \r\n", PonPortTable[PonPortIdx].testTimer, PonPortTable[PonPortIdx].testCounter );*/
	if( PonPortTable[PonPortIdx].type < PONPORTTYPEMAX)
		vty_out(vty, "    Type:%s \r\n", PonPortType_s[PonPortTable[PonPortIdx].type] );

	if( PonChipVer != PONCHIP_PAS5001 )
	{
		if( getPonExtSdramSupported( GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx) ) != 0)
			vty_out(vty, "    downlink ext-buf is 16M\r\n" );
		else vty_out(vty, "    no downlink ext-buf\r\n");
	}
	if( PonChipVer != PONCHIP_PAS5001)
	{
		PON_encryption_type_t  encryption_type = PonPortTable[PonPortIdx].EncryptType;

#if 0        
		if(PAS_get_encryption_configuration(PonPortIdx, &encryption_type) == ROK)
#endif
		{
			if(encryption_type == PON_ENCRYPTION_TYPE_TRIPLE_CHURNING )
				vty_out(vty,"    encryption mode: Triple-churning\r\n");
			else if(encryption_type == PON_ENCRYPTION_TYPE_AES)
				vty_out(vty,"    encryption mode: AES\r\n");
		}
	}
	vty_out(vty,"\r\n");
	
	return( ROK );
}

int ShowPonMacLearningByVty( short int PonPortIdx, short int SingleOnuIdx, struct vty *vty )
{
	short int  PonChipType;
	short int active_records=0;
	short int i,j;
	short int OnuIdx = 0;
	short int Onu_llid = 0;
	int slot, port,ret;

	if(SingleOnuIdx >= MAXONUPERPON )
	{
		CHECK_PON_RANGE
	}
	else
    {
		OnuIdx = SingleOnuIdx;
		CHECK_ONU_RANGE
	}

	slot = GetCardIdxByPonChip(PonPortIdx );
	if( slot == RERROR )
		return RERROR;
	port = GetPonPortByPonChip(PonPortIdx);

	PonChipType = V2R1_GetPonchipType(PonPortIdx);
	/* 1 板在位检查*/
	/*if( __SYS_MODULE_TYPE__(slot+1) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", slot);
		return( RERROR );
		}*/
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(slot) != ROK)
	{
		vty_out(vty," %% slot %d is not pon card\r\n", slot);
		return( RERROR );
	}
	
	if( getPonChipInserted((unsigned char)slot, (unsigned char)port) !=  PONCHIP_EXIST )
	{
		vty_out(vty, "\r\n %% %s/port%d not exist\r\n", CardSlot_s[slot], port );
		return( RERROR );
	} 

	if( PonPortIsWorking(PonPortIdx) == FALSE ) 
	{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[slot], port );
		return( RERROR );
	}

	/* VOS_MemSet( Mac_addr_table, 0, sizeof(PON_address_record_t )*PON_ADDRESS_TABLE_SIZE); */

#if 0
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	if ( OLT_PONCHIP_ISPAS(PonChipType) ) 
	{
		/*PonChipVer = PonChipType;*/
		PonChipType = PONCHIP_PAS;
	}
#endif

	if(SingleOnuIdx >= MAXONUPERPON )
		vty_out(vty, "\r\n%s/port%d mac-learning list\r\n",CardSlot_s[slot], port);
	else 
		vty_out(vty, "\r\n%s/port%d mac-learning from onu%d list\r\n",CardSlot_s[slot], port,(OnuIdx+1));
	

#if 0
	if( PonChipType == PONCHIP_PAS)
	{
#if 0
		if(( PAS_get_address_table(PonPortIdx, &active_records, Mac_addr_table)) != PAS_EXIT_OK )
#endif
		if( OLT_CALL_ISERROR( OLT_GetMacAddrTbl(PonPortIdx, &active_records, Mac_addr_table) ) )
		{
			/*vty_out(vty, "total Learned mac counter=0\r\n");
			return( ROK );*/
			active_records = 0;
		}
		
		/*if( active_records == 0 ) 
		{
			vty_out(vty, "total Learned mac counter=0\r\n");
			return( ROK );
		}*/
	}
	else
	{/* 其他PON芯片类型处理*/
		/*vty_out(vty, "total Learned mac counter=0\r\n");
		return( ROK );*/
		active_records = 0;
	}
#endif
	if(SingleOnuIdx >= MAXONUPERPON)
	{
		if(OLT_PONCHIP_ISGPON(PonChipType) )
		{
			ret = OLT_GetMacAddrVlanTbl(PonPortIdx, &active_records, Mac_addr_vlan_table);
		}
		else
		{
			ret = OLT_GetMacAddrTbl(PonPortIdx, &active_records, Mac_addr_table);
			if(PonChipType == PONCHIP_BCM55538)/*add by yangzl@2017-4-28:33328*/
			{
				/* added by wangjiah@2016-09-09to solve issue:30629 -- begin*/
				/* fdbentry of 8xep doesn't store onu mac addr, add onu mac into fdbentry table manually*/
				OLT_onu_table_t *pLLIDs = &olt_onu_global_table;
				if(0 == OLT_GetAllOnus(PonPortIdx, pLLIDs))
				{
					int n = 0;
					if( 0 < (n = pLLIDs->onu_num))
					{
						int i = 0;
						for(; i < n; i++)
						{
							VOS_MemCpy(Mac_addr_table[active_records].mac_address, pLLIDs->onus[i].mac_address, 6);	
							Mac_addr_table[active_records].logical_port = pLLIDs->onus[i].llid;
							Mac_addr_table[active_records].type = ADDR_DYNAMIC;
							active_records++;
						}
					}
				}
				/* added by wangjiah@2016-09-09 to solve issue:30629 -- end*/
			}
		}
		if(ret < 0 && ret != TK_NO_RESOURCE)
			active_records = 0;
	}
	else
	{
		if(OLT_PONCHIP_ISGPON(PonChipType)) 
		{
			ret = OnuMgt_GetOltMacAddrVlanTbl(PonPortIdx, OnuIdx, &active_records, Mac_addr_vlan_table);
		}
		else
		{
			ret = OnuMgt_GetOltMacAddrTbl(PonPortIdx, OnuIdx, &active_records, Mac_addr_table);
			/* added by wangjiah@2016-08-23 to solve issue:30629 -- begin*/
			/* fdbentry of 8xep doesn't store onu mac addr, add onu mac into fdbentry table manually*/
			if( PonChipType == PONCHIP_BCM55538 )
			{
				int len = 1;
				if(ROK == GetOnuMacAddr(PonPortIdx, OnuIdx, Mac_addr_table[active_records].mac_address, &len))
				{
					unsigned int llid =  GetLlidByOnuIdx(PonPortIdx, OnuIdx);
					Mac_addr_table[active_records].logical_port = llid;
					Mac_addr_table[active_records].type = ADDR_DYNAMIC;
					active_records++;
				}
			}
			/* added by wangjiah@2016-08-23 to solve issue:30629 -- end*/
		}
		if(ret < 0)
			active_records = 0;
	}

	
	if( active_records == 0 ) 
	{
		vty_out(vty, "total Learned mac counter=0\r\n");
		return( ROK );
	}
		
	/*vty_out(vty, "%d\r\n", active_records );*/

	/*vty_out(vty, "    Learned mac address list:\r\n");*/
	if(SingleOnuIdx >= MAXONUPERPON )
	{
		if(ret != TK_NO_RESOURCE)
			vty_out(vty, "total Learned mac counter=%d\r\n\r\n",active_records);
		else
		{
			vty_out(vty, "total Learned mac counter=%d\r\n\r\n",active_records);
			return( ROK );
		}
	}
	else
    {
		Onu_llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
		if( Onu_llid ==  INVALID_LLID ) 
		{
			vty_out(vty, "total Learned mac counter from onu%d=0\r\n\r\n",(OnuIdx+1));
			return( ROK );
		}
		j = 0;
		if(OLT_PONCHIP_ISGPON(PonChipType))
		{
			for(i=0; i<active_records; i++)
			{
				if( Onu_llid == Mac_addr_vlan_table[i].logical_port )
					j++;
			}

		}
		else
		{
			for(i=0; i<active_records; i++)
			{
				if( Onu_llid == Mac_addr_table[i].logical_port )
					j++;
			}
		}
		vty_out(vty, "total Learned mac counter from onu%d=%d\r\n\r\n",(OnuIdx+1), j);
		if(j==0) return(ROK );
	}
	
	if(OLT_PONCHIP_ISGPON(PonChipType))
	{
		vty_out(vty, "   Mac addr         type      OnuIdx      vid\r\n");
		vty_out(vty, "--------------------------------------------------\r\n");
	}
	else
	{
		vty_out(vty, "   Mac addr         type      OnuIdx\r\n");
		vty_out(vty, "-----------------------------------------\r\n");
	}

	if( SingleOnuIdx >= MAXONUPERPON )
	{
		if(OLT_PONCHIP_ISGPON(PonChipType))
		{
			for(OnuIdx=0; OnuIdx< MAXONUPERPON; OnuIdx++)
			{
				Onu_llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
				if( Onu_llid == INVALID_LLID ) continue;

				for(i=0; i<active_records; i++)
				{
					if( Onu_llid == Mac_addr_vlan_table[i].logical_port )
					{
						vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", Mac_addr_vlan_table[i].mac_address[0], Mac_addr_vlan_table[i].mac_address[1],
																	Mac_addr_vlan_table[i].mac_address[2], Mac_addr_vlan_table[i].mac_address[3],
																	Mac_addr_vlan_table[i].mac_address[4], Mac_addr_vlan_table[i].mac_address[5] );
						if( Mac_addr_vlan_table[i].type <= 2 )
							vty_out(vty, "     %s  ", v2r1AddrTableAgeingMode[Mac_addr_vlan_table[i].type] );
						vty_out(vty, "   %2d", (OnuIdx+1) );
						
						vty_out(vty, "         %2d\r\n", Mac_addr_vlan_table[i].vid);
						Mac_addr_vlan_table[i].logical_port = -1;
					}
				}
			}

			for( i = 0; i<active_records; i++)
			{
				if( Mac_addr_vlan_table[i].logical_port == PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID )
				{
					vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", Mac_addr_vlan_table[i].mac_address[0], Mac_addr_vlan_table[i].mac_address[1],
																Mac_addr_vlan_table[i].mac_address[2], Mac_addr_vlan_table[i].mac_address[3],
																Mac_addr_vlan_table[i].mac_address[4], Mac_addr_vlan_table[i].mac_address[5] );
					if( Mac_addr_vlan_table[i].type <= 2 )
						vty_out(vty, "     %s  ", v2r1AddrTableAgeingMode[Mac_addr_vlan_table[i].type] );
					vty_out(vty, "   CNIn");
					vty_out(vty, "         %2d\r\n",Mac_addr_vlan_table[i].vid);

					Mac_addr_vlan_table[i].logical_port = -1;
				}
			}

			/* B--added by liwei056@2010-5-4 for D10165 */
			for( i = 0; i<active_records; i++)
			{
				if( Mac_addr_vlan_table[i].logical_port >= 0 )
				{
					vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", Mac_addr_vlan_table[i].mac_address[0], Mac_addr_vlan_table[i].mac_address[1],
																Mac_addr_vlan_table[i].mac_address[2], Mac_addr_vlan_table[i].mac_address[3],
																Mac_addr_vlan_table[i].mac_address[4], Mac_addr_vlan_table[i].mac_address[5] );
					if( Mac_addr_vlan_table[i].type <= 2 )
						vty_out(vty, "     %s  ", v2r1AddrTableAgeingMode[Mac_addr_vlan_table[i].type] );
					vty_out(vty, "  UNKNOWN-LLID(%d)", Mac_addr_vlan_table[i].logical_port);
					vty_out(vty, "         (%2d)\r\n", Mac_addr_vlan_table[i].vid);
				}
			}
			
		}
		else
		{
			
			for(OnuIdx=0; OnuIdx< MAXONUPERPON; OnuIdx++)
			{
				Onu_llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
				if( Onu_llid == INVALID_LLID ) continue;

				for(i=0; i<active_records; i++)
				{
					if( Onu_llid == Mac_addr_table[i].logical_port )
					{
						vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", Mac_addr_table[i].mac_address[0], Mac_addr_table[i].mac_address[1],
																	Mac_addr_table[i].mac_address[2], Mac_addr_table[i].mac_address[3],
																	Mac_addr_table[i].mac_address[4], Mac_addr_table[i].mac_address[5] );
						if( Mac_addr_table[i].type <= 2 )
							vty_out(vty, "     %s  ", v2r1AddrTableAgeingMode[Mac_addr_table[i].type] );
						vty_out(vty, "   %2d\r\n", (OnuIdx+1) );
						

						Mac_addr_table[i].logical_port = -1;
					}
				}
			}

			for( i = 0; i<active_records; i++)
			{
				if( Mac_addr_table[i].logical_port == PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID )
				{
					vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", Mac_addr_table[i].mac_address[0], Mac_addr_table[i].mac_address[1],
																Mac_addr_table[i].mac_address[2], Mac_addr_table[i].mac_address[3],
																Mac_addr_table[i].mac_address[4], Mac_addr_table[i].mac_address[5] );
					if( Mac_addr_table[i].type <= 2 )
						vty_out(vty, "     %s  ", v2r1AddrTableAgeingMode[Mac_addr_table[i].type] );
					vty_out(vty, "   CNI\r\n");

					Mac_addr_table[i].logical_port = -1;
				}
			}

	        /* B--added by liwei056@2010-5-4 for D10165 */
			for( i = 0; i<active_records; i++)
			{
				if( Mac_addr_table[i].logical_port >= 0 )
				{
					vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", Mac_addr_table[i].mac_address[0], Mac_addr_table[i].mac_address[1],
																Mac_addr_table[i].mac_address[2], Mac_addr_table[i].mac_address[3],
																Mac_addr_table[i].mac_address[4], Mac_addr_table[i].mac_address[5] );
					if( Mac_addr_table[i].type <= 2 )
						vty_out(vty, "     %s  ", v2r1AddrTableAgeingMode[Mac_addr_table[i].type] );
					vty_out(vty, "  UNKNOWN-LLID(%d)\r\n", Mac_addr_table[i].logical_port);
				}
			}
		
		}
        /* E--added by liwei056@2010-5-4 for D10165 */
	}
	else
	{
		if(OLT_PONCHIP_ISGPON(PonChipType))
		{
			for(i=0; i<active_records; i++)
			{
				if( Onu_llid == Mac_addr_vlan_table[i].logical_port )
				{
					vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", Mac_addr_vlan_table[i].mac_address[0], Mac_addr_vlan_table[i].mac_address[1],
																Mac_addr_vlan_table[i].mac_address[2], Mac_addr_vlan_table[i].mac_address[3],
																Mac_addr_vlan_table[i].mac_address[4], Mac_addr_vlan_table[i].mac_address[5] );
					if( Mac_addr_vlan_table[i].type <= 2 )
						vty_out(vty, "     %s  ", v2r1AddrTableAgeingMode[Mac_addr_vlan_table[i].type] );
					vty_out(vty, "   %2d", (OnuIdx+1) );
					vty_out(vty, "         %2d\r\n", Mac_addr_vlan_table[i].vid);
				}
			}

		}
		else
		{
			for(i=0; i<active_records; i++)
			{
				if( Onu_llid == Mac_addr_table[i].logical_port )
				{
					vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", Mac_addr_table[i].mac_address[0], Mac_addr_table[i].mac_address[1],
																Mac_addr_table[i].mac_address[2], Mac_addr_table[i].mac_address[3],
																Mac_addr_table[i].mac_address[4], Mac_addr_table[i].mac_address[5] );
					if( Mac_addr_table[i].type <= 2 )
						vty_out(vty, "     %s  ", v2r1AddrTableAgeingMode[Mac_addr_table[i].type] );
					vty_out(vty, "   %2d\r\n", (OnuIdx+1) );
					
				}
			}
		}
	}
#if 0
	for(i=0; i< active_records; i++)
		{
		OnuIdx = GetOnuIdxByLlid(PonPortIdx, Mac_addr_table[i].logical_port);
		/*
		if( OnuIdx == RERROR )
			continue;
		*/
		for(j=0; j<(BYTES_IN_MAC_ADDRESS -1); j++)
			{
			vty_out(vty, "%02x%02x", Mac_addr_table[i].mac_address[j], Mac_addr_table[i].mac_address[j+1] );
			j++;
			if( j != 5 )vty_out(vty, ".");
			}
		vty_out(vty, "     %s  ", v2r1AddrTableAgeingMode[Mac_addr_table[i].type] );
		
		if( Mac_addr_table[i].logical_port == PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID )
			vty_out(vty, "   CNI\r\n");
		else
			vty_out(vty, "   %2d\r\n", (OnuIdx+1) );
		}
#endif
	vty_out(vty, "\r\n");
	/*vty_out(vty, "    total Learned mac counter=%d\r\n",active_records);
	vty_out( vty, "  End of list\r\n");*/
	return( ROK );

}

int ShowPonMacLearningCounterByVty( short int PonPortIdx, struct vty *vty )
{
	short int  PonChipType;
	short int active_records;
	int slot, port;
	
	CHECK_PON_RANGE

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	/* 1 板在位检查*/
	/*if( __SYS_MODULE_TYPE__(slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", slot);
		return( CMD_WARNING );
		}*/
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(slot) != ROK)
		{
		vty_out(vty," %% slot %d is not pon card\r\n", slot);
		return( CMD_WARNING );
		}
	
	if( getPonChipInserted((unsigned char )(slot), (unsigned char )(port) ) !=  PONCHIP_EXIST )
		{
		vty_out(vty, "\r\n  pon%d/%d not exist\r\n", slot, port);
		return( ROK );
		} 

	if( PonPortIsWorking(PonPortIdx) == FALSE ) 
		{
		vty_out(vty, "\r\n  pon%d/%d not working\r\n",slot,port);
		return( ROK );
		}

	/* VOS_MemSet( Mac_addr_table, 0, sizeof(PON_address_record_t )*PON_ADDRESS_TABLE_SIZE); */

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	vty_out(vty, "\r\n  pon%d/%d mac-learning counter",slot, port);
	

	if( OLT_PONCHIP_ISVALID(PonChipType))
		{
#if 0
		if(( PAS_get_address_table(PonPortIdx, &active_records, Mac_addr_table)) != PAS_EXIT_OK )
#else
		if( OLT_CALL_ISERROR( OLT_GetMacAddrTbl(PonPortIdx, &active_records, NULL) ) )
#endif
			{
			vty_out(vty, "=0\r\n");
			return( ROK );
			}
		if( active_records == 0 ) 
			{
			vty_out(vty, "=0\r\n");
			return( ROK );
			}
		}
	else{/* 其他PON芯片类型处理*/
		vty_out(vty, "=0\r\n");
		return( ROK );
		}
	
	vty_out(vty, "=%d\r\n\r\n",active_records);
	/*vty_out( vty, "  End of list\r\n");*/
	return( ROK );

}

/* B--added by liwei056@2011-3-14 for 武汉长宽 */
int ShowPonMacLearningCounterListByVty( short int PonPortIdx, short int NumLimit, struct vty *vty )
{
    short int OltStartID, OltEndID,OnuEntry;
    short int OnuIdx;
    short int OnuHasShow;
    short int PonNeedShow;
    short int PonHasShow;
    int OltSlot, OltPort;
    short int active_records;
    char szAlignBuf[16];


    if ( OLT_ID_ALL == PonPortIdx )
    {
        OltStartID  = 0;
        OltEndID    = MAXPON;
        PonNeedShow = 0;  /* 1 */
    }
    else
    {
        OltStartID  = PonPortIdx;
        OltEndID    = PonPortIdx + 1;
        PonNeedShow = 0;
    }

    OnuHasShow = 0;
    for ( ; OltStartID < OltEndID ; OltStartID++ )
    {
        PonHasShow = 0;

        OltSlot = GetCardIdxByPonChip(OltStartID);
        OltPort = GetPonPortByPonChip(OltStartID);

        for ( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++ )
        {
              OnuEntry = OltStartID * MAXONUPERPON + OnuIdx ;
        	if( GetOnuOperStatus( OltStartID, OnuIdx ) == ONU_OPER_STATUS_UP )
            {
                if ( ROK == ThisIsValidOnu( OltStartID, OnuIdx ) )
                {
                    if( OLT_CALL_ISOK( OnuMgt_GetOltMacAddrTbl(OltStartID, OnuIdx, &active_records, NULL) ) )
                    {
                        if ( active_records >= NumLimit )
                        {
                        	unsigned char name[MAXDEVICENAMELEN+1];
                        	int nameLen;
                            
                            if ( 0 == OnuHasShow )
                            {
                                OnuHasShow = -1;
                              
                                vty_out(vty, "\r\n  ONU         Mac addr        MacLearningCounter  device name\r\n");
                                vty_out(vty, "-------------------------------------------------------------------\r\n");
                            }
                        
                            if ( 0 == PonHasShow )
                            {
                                PonHasShow = 1;

                                if ( 0 < OnuHasShow )
                                {
                                    vty_out(vty, "\r\n");
                                }
                                else
                                {
                                    OnuHasShow = 1;
                                }

                                if ( 0 != PonNeedShow )
                                {
                                    vty_out(vty, "Pon%d/%d:\r\n", OltSlot, OltPort);
                                }
                            }

                           /* VOS_Snprintf(szAlignBuf, 15, "%d/%d/%d", OltSlot, OltPort, OnuIdx + 1);
                            vty_out(vty, "  %-16s%6d\r\n", szAlignBuf, active_records);*/
                			GetOnuDeviceName( OltStartID, OnuIdx, name, &nameLen);
                			if(nameLen > MAXDEVICENAMELEN ) nameLen = MAXDEVICENAMELEN;
                			name[nameLen] = '\0';

                            VOS_Snprintf(szAlignBuf, 10, "%d/%d/%d", OltSlot, OltPort, OnuIdx + 1);      /*需求13318*/                    
            			    vty_out(vty, "%-11s %02x%02x.%02x%02x.%02x%02x    %18d   %s\r\n", szAlignBuf,OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
														OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] ,active_records,
														name);


                        }
                    }
                }
            }   
        }
    }

    vty_out(vty, "\r\n");
    
    return( ROK );
}
/* E--added by liwei056@2011-3-14 for 武汉长宽 */

int ShowPonMacLearningFromWhichOnuByVty( short int PonPortIdx, unsigned char *MacAddress, struct vty *vty )
{
	short int  PonChipType;
	short int active_records;
	int slot, port, i;
	short int OnuIdx = RERROR;

	CHECK_PON_RANGE;

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	if(MacAddress == NULL ) return(RERROR);

	/* 1 板在位检查*/
	/*if( __SYS_MODULE_TYPE__(slot) == MODULE_TYPE_NULL )
	  {
	  vty_out(vty," %% slot %d is not inserted\r\n", slot);
	  return( CMD_WARNING );
	  }*/
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(slot) != ROK)
	{
		vty_out(vty," %% slot %d is not pon card\r\n", slot);
		return( CMD_WARNING );
	}

	if( getPonChipInserted((unsigned char )(slot), (unsigned char )(port) ) !=  PONCHIP_EXIST )
	{
		vty_out(vty, "\r\n  pon%d/%d not exist\r\n", slot, port);
		return( ROK );
	} 

	if( PonPortIsWorking(PonPortIdx) == FALSE ) 
	{
		vty_out(vty, "\r\n  pon%d/%d not working\r\n", slot, port);
		return( ROK );
	}

	/* VOS_MemSet( Mac_addr_table, 0, sizeof(Mac_addr_table ) ); */

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);

	if( OLT_PONCHIP_ISVALID(PonChipType))
	{
		if(SYS_MODULE_IS_6900_16EPON(slot))
		{
			short int onu_llid = 0;
			if(VOS_OK == OLT_GetLlidByUserMacAddress(PonPortIdx, MacAddress, &onu_llid))
			{
				OnuIdx = GetOnuIdxByLlid(PonPortIdx, onu_llid);

				vty_out(vty,"MAC %02x%02x.%02x%02x.%02x%02x is learned from ",MacAddress[0],MacAddress[1],MacAddress[2],MacAddress[3],MacAddress[4],MacAddress[5]);

				if( OnuIdx == RERROR )
					vty_out(vty,"pon%d/%d Unknown-LLID(%d)\r\n", slot, port, onu_llid);
				else
					vty_out(vty,"onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));                  				
			}
		}
		else
		{		
			/* added by wangjiah@2017-06-08 to solve issue:37087 -- begin*/
			if(PONCHIP_BCM55538 == PonChipType)
			{
				OLT_onu_table_t *pLLIDs = &olt_onu_global_table;
				if(0 == OLT_GetAllOnus(PonPortIdx, pLLIDs))
				{
					int n = 0;
					if( 0 < (n = pLLIDs->onu_num))
					{
						int i = 0;
						for(; i < n; i++)
						{
							if(MAC_ADDR_IS_EQUAL(pLLIDs->onus[i].mac_address, MacAddress))
							{
								OnuIdx = GetOnuIdxByLlid(PonPortIdx, pLLIDs->onus[i].llid);
								vty_out(vty,"MAC %02x%02x.%02x%02x.%02x%02x is learned from ",
										MacAddress[0],MacAddress[1],MacAddress[2],MacAddress[3],MacAddress[4],MacAddress[5]);
								if( OnuIdx == RERROR )
									vty_out( vty,"pon%d/%d Unknown-LLID(%d)\r\n", slot, port, pLLIDs->onus[i].llid);
								else
									vty_out(vty,"onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
								return (ROK);
							}
						}
					}
				}
			}
			/* added by wangjiah@2017-06-08 to solve issue:37087 -- end*/

			if( OLT_CALL_ISERROR( OLT_GetMacAddrTbl(PonPortIdx, &active_records, Mac_addr_table) ) )
			{
				return( ROK );
			}
			if( active_records == 0 ) 
			{
				return( ROK );
			}
			for(i=0; i< active_records; i++)
			{
				if(MAC_ADDR_IS_EQUAL(Mac_addr_table[i].mac_address, MacAddress))
				{
					/* modified by xieshl 20110818, 和基于PON口的命令显示一致 */
					vty_out(vty,"MAC %02x%02x.%02x%02x.%02x%02x is learned from ",MacAddress[0],MacAddress[1],MacAddress[2],MacAddress[3],MacAddress[4],MacAddress[5]);
					if( Mac_addr_table[i].logical_port == PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID )
					{
						vty_out( vty, "pon%d/%d CNI\r\n", slot, port );
					}
					else
					{
						OnuIdx = GetOnuIdxByLlid(PonPortIdx, Mac_addr_table[i].logical_port);
						if( OnuIdx == RERROR )
							vty_out( vty,"pon%d/%d Unknown-LLID(%d)\r\n", slot, port, Mac_addr_table[i].logical_port );
						else
							vty_out(vty,"onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
					}
					break;
				}
			}		
		}		
	}    
	else
	{/* 其他PON芯片类型处理*/
		return( ROK );
	}

	return( ROK );

}

int ShowPonMacLearningFromWhichOnuByVtyAll(unsigned char *MacAddress, struct vty *vty )
{
	/*short int  PonChipType;
	short int active_records;*/
	int slot, port;
	short int PonPortIdx;

	if(MacAddress == NULL ) return(RERROR);

	for(slot = PONCARD_FIRST; slot <= PONCARD_LAST; slot++)
	{
		/* 1 板在位检查*/
		/*if( __SYS_MODULE_TYPE__(slot) == MODULE_TYPE_NULL ) continue;*/

		/* 2 pon 板类型检查*/
		if(SlotCardIsPonBoard(slot) != ROK) continue;

		for(port=1; port<=PONPORTPERCARD; port++)
		{
			if( getPonChipInserted((unsigned char )slot, (unsigned char)port) !=  PONCHIP_EXIST )
				continue; 
			
			PonPortIdx = GetPonPortIdxBySlot(slot, port);
			if( PonPortIsWorking(PonPortIdx) == FALSE ) 
				continue;
			ShowPonMacLearningFromWhichOnuByVty(PonPortIdx,MacAddress, vty);
		}
	}

	return( ROK );

}

int ShowPonPortOpticalParaByVty( short int PonPortIdx , struct vty *vty)
{
	short int ret;
	short int PonChipType;
	short int PonChipIdx;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	
	PonChipIdx = GetPonChipIdx( PonPortIdx );

	if( PonPortIsWorking(PonPortIdx) != TRUE )
		{
		vty_out(vty, "\r\n  %s/port%d not running\r\n", CardSlot_s[GetCardIdxByPonChip(PonChipIdx)], GetPonPortByPonChip(PonChipIdx) );
		return( RERROR );
		}

	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType))
			{
			PAS_physical_capabilities_t device_capabilities;
			
			ret = PAS_get_device_capabilities_v4( PonPortIdx,  PON_OLT_ID, &device_capabilities);
	
			if( ret == PAS_EXIT_OK )
				{
				/* print these info */
				vty_out(vty, "\r\n  %s/port%d Optical Param:\r\n", CardSlot_s[GetCardIdxByPonChip(PonChipIdx)], GetPonPortByPonChip(PonChipIdx) );
				vty_out(vty, "   Laser on/off timer %d/%d \r\n", device_capabilities.laser_on_time, device_capabilities.laser_off_time );
				vty_out(vty,"   Max grant number %d \r\n", device_capabilities.onu_grant_fifo_depth );
				vty_out(vty,"   AGC / CDR lock time %d / %d \r\n", device_capabilities.agc_lock_time, device_capabilities.cdr_lock_time );
				vty_out(vty,"   pon tx signal %d \r\n", device_capabilities.pon_tx_signal );
			
				/* save these info to PonChipMgmtTable[] 

				PonPortTable[PonPortIdx].MaxOnu = device_versions.ports_supported ;*/
				PonChipMgmtTable[PonChipIdx].WorkingMode.AGCTime = device_capabilities.agc_lock_time;
				PonChipMgmtTable[PonChipIdx].WorkingMode.CDRTime = device_capabilities.cdr_lock_time;
				return( ROK );
				}
			else {
				vty_out(vty, "Can't get Optical info for this pon \r\n");
				return( RERROR );
				}
			}
		else 
			{
			#if 1/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			OLT_optics_detail_t optics_params;
			VOS_MemZero(&optics_params, sizeof(OLT_optics_detail_t));
			OLT_GetOpticsDetail(PonPortIdx, &optics_params);
			#else
			PON_olt_optics_configuration_t  optics_configuration; 
			bool pon_tx_signal;
			
			ret = PAS_get_olt_optics_parameters(PonPortIdx, &optics_configuration, &pon_tx_signal);
            #endif
			if( ret == PAS_EXIT_OK )
				{
				/* print these info 
				vty_out(vty, "\r\n  %s/port%d Optical Param:\r\n", CardSlot_s[GetCardIdxByPonChip(PonChipIdx)], GetPonPortByPonChip(PonChipIdx)  );
				vty_out(vty, "   Laser on/off timer %d/%d \r\n", optics_configuration.laser_on_time, optics_configuration.laser_off_time );
				vty_out(vty,"   Max grant number %d \r\n", optics_configuration.onu_grant_fifo_depth );
				vty_out(vty,"   AGC / CDR lock time %d / %d \r\n", optics_configuration.agc_lock_time, optics_configuration.cdr_lock_time );
				vty_out(vty,"   pon tx signal %d \r\n", pon_tx_signal );
				*/
				/* save these info to PonChipMgmtTable[] 

				PonPortTable[PonPortIdx].MaxOnu = device_versions.ports_supported ;*/
				#if 1
				PonChipMgmtTable[PonChipIdx].WorkingMode.AGCTime = optics_params.pon_optics_params.agc_lock_time;
				PonChipMgmtTable[PonChipIdx].WorkingMode.CDRTime = optics_params.pon_optics_params.cdr_lock_time;
				#else
				PonChipMgmtTable[PonChipIdx].WorkingMode.AGCTime = optics_configuration.agc_lock_time;
				PonChipMgmtTable[PonChipIdx].WorkingMode.CDRTime = optics_configuration.cdr_lock_time;
				#endif
				/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
				return( ROK );
				}
			else {
				vty_out(vty, "Can't get Optical info for this pon \r\n");
				return( RERROR );
				}
			}
		}
	
	/* other PON chip type handler */
	else if( PonChipType == PONCHIP_GW)
		{  
		return( RERROR );
		}
	else if( PonChipType == PONCHIP_TK )
		{
		return( RERROR );
		}
	
	vty_out(vty, "this Pon port Type is err\r\n");

	return( RERROR );

}


int PendingOnuListByVty( short int PonPortIdx, short int *count, struct vty *vty)
{
#if 0	/* modified by xieshl 20100301 */
	pendingOnu_S *CurOnu;
	int i;

	CHECK_PON_RANGE

	if( PonPortIsWorking(PonPortIdx) != TRUE ) 
		{
		vty_out( vty, "%s/port%d is not exist\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		return( ROK );
		}
		
	i =0;
	vty_out(vty, "\r\n  %s/port%d pending onu list \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
	CurOnu = PonPortTable[PonPortIdx].PendingOnu.Next;
	if( CurOnu == NULL  ) 
		{
		vty_out(vty, "  no pending onu \r\n"); 
		}
	else {
		vty_out(vty,"LLID            macAddress\r\n");
		while( CurOnu != NULL )
			{
			vty_out(vty, "%2d  ", CurOnu->Llid );
			vty_out(vty,"            ");
			vty_out(vty,"%02x%02x.%02x%02x.%02x%02x\r\n",CurOnu->MacAddr[0],CurOnu->MacAddr[1],CurOnu->MacAddr[2],CurOnu->MacAddr[3],CurOnu->MacAddr[4],CurOnu->MacAddr[5]);
			CurOnu = CurOnu->Next;
			/*i ++;
			if(( i  % 10 ) == 0 ) vty_out(vty, "\r\n");*/
			}
		vty_out(vty, "\r\n");
		}	

	i =0;
	vty_out(vty, "\r\n  %s/port%d pending onu(conflict) list \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
	CurOnu = PonPortTable[PonPortIdx].PendingOnu_Conf.Next;
	if( CurOnu == NULL  ) 
		{
		vty_out(vty, "  no pending onu \r\n"); 
		return( ROK );
		}
	else{
		vty_out(vty,"LLID            macAddress          conflict-pon\r\n");
		while( CurOnu != NULL )
			{
			vty_out(vty, "%2d  ", CurOnu->Llid);
			vty_out(vty,"            ");
			vty_out(vty,"%02x%02x.%02x%02x.%02x%02x\r\n",CurOnu->MacAddr[0],CurOnu->MacAddr[1],CurOnu->MacAddr[2],CurOnu->MacAddr[3],CurOnu->MacAddr[4],CurOnu->MacAddr[5]);
			vty_out(vty,"        ");
			vty_out(vty, "%d/%d\r\n",GetCardIdxByPonChip(CurOnu->otherPonIdx), GetPonPortByPonChip(CurOnu->otherPonIdx));
			CurOnu = CurOnu->Next;
			/*i ++;
			if(( i  % 10 ) == 0 ) vty_out(vty, "\r\n");*/
			}
		}
	vty_out(vty, "\r\n");
	return( ROK );
#else
	int i, total;
	pendingOnuList_t onuList;
	pendingOnuNode_t *CurOnu;

	CHECK_PON_RANGE

	if( PonPortIsWorking(PonPortIdx) != TRUE ) 
	{
		vty_out( vty, "pon%d/%d is not exist\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
		return( ROK );
	}

	VOS_MemZero( &onuList, sizeof(onuList) );
	OLT_GetPendingOnuList(PonPortIdx, &onuList);

	for( i=0; i<onuList.pendingCount; i++ )
	{
		CurOnu = &onuList.pendingList[i];
		vty_out(vty, " %2d/%-6d%-7d", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), CurOnu->Llid );
		if(CurOnu->OnuMarkInfor.pendingOnutype == PENDINGONU_GPON)
		{
			vty_out(vty,"%16s", CurOnu->OnuMarkInfor.OnuMark.serial_no);/**/
		}
		else 
		{	
			vty_out(vty, "%02x%02x.%02x%02x.%02x%02x  ", CurOnu->OnuMarkInfor.OnuMark.serial_no[0],CurOnu->OnuMarkInfor.OnuMark.serial_no[1],
														CurOnu->OnuMarkInfor.OnuMark.serial_no[2],CurOnu->OnuMarkInfor.OnuMark.serial_no[3],
														CurOnu->OnuMarkInfor.OnuMark.serial_no[4],CurOnu->OnuMarkInfor.OnuMark.serial_no[5]);
		}
		vty_out(vty, "   %s\r\n", PendingReason_str[CurOnu->code]);
	}	
	total = i;

	for( i=0; i<onuList.conflictCount; i++ )
	{
		CurOnu = &onuList.conflictList[i];
		vty_out(vty, " %2d/%-6d%-7d", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), CurOnu->Llid);
		if(CurOnu->OnuMarkInfor.pendingOnutype == PENDINGONU_GPON)
		{
			vty_out(vty,"%16s", CurOnu->OnuMarkInfor.OnuMark.serial_no);/**/
		}
		else 
		{	
			vty_out(vty, "%02x%02x.%02x%02x.%02x%02x  ", CurOnu->OnuMarkInfor.OnuMark.serial_no[0],CurOnu->OnuMarkInfor.OnuMark.serial_no[1],
														CurOnu->OnuMarkInfor.OnuMark.serial_no[2],CurOnu->OnuMarkInfor.OnuMark.serial_no[3],
														CurOnu->OnuMarkInfor.OnuMark.serial_no[4],CurOnu->OnuMarkInfor.OnuMark.serial_no[5]);
		}

		vty_out(vty, "   conflict(%d/%d)\r\n", GetGlobalCardIdxByPonChip(CurOnu->otherPonIdx), GetGlobalPonPortByPonChip(CurOnu->otherPonIdx));
	}
	if( total )
		vty_out(vty, "\r\n");

	total += i;
	*count += total;

	return( ROK );
#endif
}

int ShowOnuRegisterCounterInfoByVty(short int PonPortIdx ,struct vty *vty)
{
	Date_S  NewerDate;
	/*int i;*/
	
	CHECK_PON_RANGE

	if(PonPortIsWorking(PonPortIdx) != TRUE ) 
		{
		vty_out( vty, " pon%d/%d is not in working status\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx) );
		return( RERROR );
		}
	vty_out(vty, "  pon%d/%d register counter info \r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
	vty_out(vty, "    register-req Msg counter: %d \r\n", PonPortTable[PonPortIdx].OnuRegisterMsgCounter );
	vty_out(vty, "    deregister Msg counter: %d\r\n", PonPortTable[PonPortIdx].OnuDeregisterMsgCounter);

	if( PonPortTable[PonPortIdx].OnuRegisterMsgCounter != 0 )
	{
		vty_out(vty, "    the last onu register time:");
		{
			VOS_MemCpy((char *)&(NewerDate),(char *)&( PonPortTable[PonPortIdx].LastOnuRegisterTime.year),sizeof( Date_S));
			vty_out( vty, "%04d-%02d-%02d,", NewerDate.year, NewerDate.month, NewerDate.day );
			vty_out( vty, "%d:%d:%d\r\n", NewerDate.hour, NewerDate.minute, NewerDate.second );
		}
		vty_out(vty, "    the last onu register addr:");
		vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", PonPortTable[PonPortIdx].LastRegisterMacAddr[0],PonPortTable[PonPortIdx].LastRegisterMacAddr[1],
													PonPortTable[PonPortIdx].LastRegisterMacAddr[2],PonPortTable[PonPortIdx].LastRegisterMacAddr[3],
													PonPortTable[PonPortIdx].LastRegisterMacAddr[4],PonPortTable[PonPortIdx].LastRegisterMacAddr[5] );
	}
	vty_out(vty, "\r\n");

	return( ROK );
}

#ifdef ADD_ONU
#endif

/*****************************************************
 *
 *    Function: int AddOnuToPonPort(short int PonPortIdx, short int OnuIdx, unsigned char *MacAddr )
 *
 *    Param:  short int PonPortIdx -- the specific pon port 
 *
 *    Desc:  增加或删除ONU;
 *               注:当为有效MAC 时,为增加ONU;
 *                     当无效时,为删除ONU ;功能等同于DelOnuToPonPort()
 *
 *    Return:  -1 -- 参数或执行错
 *             1 -- 要添加的ONU标识OnuIdx中已存在其它有效的ONU mac 地址
 *             2 -- 参数中指定的ONU MAC 地址已指配在其他PON端口下
 *             3 -- 参数中指定的ONU MAC地址已指配在本PON端口的其他OnuIdx下
 *             0 -- OK
 *
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/

int AddOnuToPonPort(short int PonPortIdx, short int OnuIdx, unsigned char *MacAddr )
{
	short int Entry, OtherOnuIdx;
	int ret=0;
	
	CHECK_ONU_RANGE

	if( MacAddr == NULL ) return (RERROR );

	/*if( CompTwoMacAddress(MacAddr, Invalid_Mac_Addr) == ROK )
		{
		DelOnuFromPonPort(PonPortIdx, OnuIdx);
		return( ROK );
		}
	
	if( CompTwoMacAddress(MacAddr, Invalid_Mac_Addr1) == ROK )
		{
		DelOnuFromPonPort(PonPortIdx, OnuIdx);
		return( ROK );
		}*/
	if( MAC_ADDR_IS_ZERO(MacAddr) || MAC_ADDR_IS_BROADCAST(MacAddr) )
	{
		DelOnuFromPonPort(PonPortIdx, OnuIdx);
		return( ROK );
	}

    /* B--modified by liwei056@2011-7-18 for  D12444 */
#if 0    /*moved by luh 2013-8-1*/
	OtherOnuIdx = GetOnuIdxByMacPerPon( PonPortIdx, MacAddr );
	if( OtherOnuIdx != RERROR ) 
	{
		if(OtherOnuIdx != OnuIdx)  return(3);
	}
#endif    
    /* E--modified by liwei056@2011-7-18 for  D12444 */

    Entry = PonPortIdx * MAXONUPERPON + OnuIdx;    
#if 0
	if( !(MAC_ADDR_IS_BROADCAST(OnuMgmtTable[Entry].DeviceInfo.MacAddr) || MAC_ADDR_IS_EQUAL(MacAddr, OnuMgmtTable[Entry].DeviceInfo.MacAddr)) )
		return 1;
#else
	if(MAC_ADDR_IS_EQUAL(MacAddr, OnuMgmtTable[Entry].DeviceInfo.MacAddr))
		return 4;
#endif
	/*add by shixh20100909*/
	ret = OnuMgt_AddOnuByManual(PonPortIdx, OnuIdx, MacAddr);
#if 0	
	VOS_MemCpy(OnuMgmtTable[Entry].DeviceInfo.MacAddr, MacAddr, BYTES_IN_MAC_ADDRESS );
	OnuMgmtTable[Entry].LlidTable[0].EntryStatus = LLID_ENTRY_NOT_IN_ACTIVE;
	/* add by chenfj 2006-10-30 */
	OnuMgmtTable[Entry].LlidTable[0].UplinkBandwidth_gr = OnuConfigDefault.UplinkBandwidth;
	OnuMgmtTable[Entry].LlidTable[0].DownlinkBandwidth_gr = OnuConfigDefault.DownlinkBandwidth;
#ifdef PLATO_DBA_V3
	OnuMgmtTable[Entry].LlidTable[0].UplinkBandwidth_fixed = 0;
#endif
	/*added by chenfj 2006-12-21 
	    设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参
	*/
	OnuMgmtTable[Entry].LlidTable[0].UpLinkBandwidth_be = OnuConfigDefault.UplinkBandwidthBe;
	OnuMgmtTable[Entry].LlidTable[0].DownlinkBandwidth_be = OnuConfigDefault.DownlinkBandwidthBe;
	OnuMgmtTable[Entry].LlidTable[0].UplinkClass = OnuConfigDefault.UplinkClass;
	OnuMgmtTable[Entry].LlidTable[0].UplinkDelay = OnuConfigDefault.UplinkDelay;
	OnuMgmtTable[Entry].LlidTable[0].DownlinkClass = OnuConfigDefault.DownlinkClass;
	OnuMgmtTable[Entry].LlidTable[0].DownlinkDelay = OnuConfigDefault.DownlinkDelay;

	OnuMgmtTable[Entry].RegisterFlag = NOT_ONU_FIRST_REGISTER_FLAG;
	RecordOnuUpTime(PonPortIdx, OnuIdx );

	/* add by chenfj 2006/12/13 */
	 /* 3mib port status */
	{
		int slot;
		short int port;

		slot  = GetCardIdxByPonChip(PonPortIdx);
		port = GetPonPortByPonChip( PonPortIdx );		
		setOnuStatus( slot,  port, OnuIdx, ONU_ONLINE );
	}

	/* added by chenfj 2007-02-07
          增加ONU自动配置
      */
#ifdef   V2R1_ONU_AUTO_CONFIG
	OnuMgmtTable[Entry].AutoConfig = V2R1_ONU_USING_AUTO_CONFIG;
	VOS_MemCpy( &(OnuMgmtTable[Entry].AutoConfigFile[0]), OnuDefaultConfigName,15);
	OnuMgmtTable[Entry].AutoConfigFlag = V2R1_ONU_INITIAL_STATUS;
#endif	
#endif	
	/* add by chenfj 2006-11-25 */
	if(SYS_MODULE_IS_RUNNING(GetCardIdxByPonChip(PonPortIdx)))/*To avoid the recovery of the configuration affect the Pon startup process:yangzl@2016-4-8 */
	ActivatePendingOnuMsg( PonPortIdx );
	return( ROK );
}
int AddGponOnuToPonPort(short int PonPortIdx, short int OnuIdx, unsigned char *sn )
{
	short int Entry, OtherOnuIdx;
	int ret=0;
	
	CHECK_ONU_RANGE

	if( sn == NULL ) return (RERROR );

    Entry = PonPortIdx * MAXONUPERPON + OnuIdx;    
	if(VOS_StrCmp(sn, OnuMgmtTable[Entry].DeviceInfo.DeviceSerial_No) == 0)
		return 4;

	/*add by shixh20100909*/
	ret = OnuMgt_AddGponOnuByManual(PonPortIdx, OnuIdx, sn);
	/* add by chenfj 2006-11-25 */
	ActivatePendingOnuMsg( PonPortIdx );
	return( ROK );
}
int ModifyOnuToPonPort(short int PonPortIdx, short int OnuIdx, unsigned char *MacAddr )
{
	short int Entry, OtherOnuIdx;
	int ret=0;
	
	CHECK_ONU_RANGE

	if( MacAddr == NULL ) return (RERROR );

	if( MAC_ADDR_IS_ZERO(MacAddr) || MAC_ADDR_IS_BROADCAST(MacAddr) )
	{
		return( ROK );
	}

	OtherOnuIdx = GetOnuIdxByMacPerPon( PonPortIdx, MacAddr );
	if( OtherOnuIdx != RERROR ) 
	{
		if(OtherOnuIdx != OnuIdx)  return(3);
	}

	ret = OnuMgt_ModifyOnuByManual(PonPortIdx, OnuIdx, MacAddr);
	return( ROK );
}

extern int eraseDevEventRecords( ulong_t devIdx );
int DelOnuFromPonPort(short int PonPortIdx, short int OnuIdx)
{
	/*short int OnuEntry;*/
	/*int OnuCurrStatus;*/
	/*unsigned char macAddress[BYTES_IN_MAC_ADDRESS]={0};*/
	int slot/*, tdmSlot=0*/;
	short int port;
	unsigned long devid;
	
	CHECK_ONU_RANGE;

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);	
	devid = MAKEDEVID(slot, port, (OnuIdx+1) );

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	tdmSlot=get_gfa_tdm_slotno();
	if(tdmSlot)
		{
		unsigned char tdm_slot, tdm_sg;
		unsigned short int logidOnuId;
		/*DeleteOnuFromSG(devid);*/
		if(SlotCardIsTdmSgBoard(tdmSlot) == ROK)
			{
			if(GetOnuBelongToSG(devid,&tdm_slot, &tdm_sg,&logidOnuId) == ROK )
				return( RERROR );
			}
		/* 判断ONU 上是否配置了E1 连接; 若有E1 连接, 应先删除*/
		else if(SlotCardIsTdmE1Board(tdmSlot) == ROK)
			{
			OnuE1Info  pOnuE1Info;
			VOS_MemSet(&pOnuE1Info, 0, sizeof(OnuE1Info));
			if((GetOnuE1Info(devid,&pOnuE1Info) == ROK)
				&& (pOnuE1Info.onuValidE1Count != 0 ))
				return(RERROR);
			}
		}
#endif
	eraseDevEventRecords(devid);
	/*OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;*/
	/*OnuCurrStatus = GetOnuOperStatus_1(PonPortIdx, OnuIdx );*/

	/*if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) return(V2R1_ONU_NOT_EXIST);*/

	/*ONU_MGMT_SEM_TAKE;
	VOS_MemCpy( macAddress, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS );
	ONU_MGMT_SEM_GIVE;*/
#if 1  /*关键点，以下这些信息改变SHOW ONU-LIST 的显示情况test by shixh2010*/
	OnuMgt_DelOnuByManual(PonPortIdx, OnuIdx);
#else
	if(  OnuCurrStatus != ONU_OPER_STATUS_DOWN )
		{
		/*OnuMgmtTable[OnuEntry].NeedDeleteFlag = TRUE;		*/	
		OnuMgt_DelOnuByManual(PonPortIdx, OnuIdx);
		Onu_deregister( PonPortIdx, OnuIdx );
		}
	else {
		/* added by chenfj 2009-5-14 , 当这个PON 口下有多于 64个ONU 时，产生告警*/
		if(PonPortTable[PonPortIdx].PortFullAlarm == V2R1_ENABLE)
			Trap_PonPortFullClear(PonPortIdx);
		OnuMgt_DelOnuByManual(PonPortIdx, OnuIdx);
		InitOneOnuByDefault(PonPortIdx, OnuIdx);		
		}
#endif

	/*add by chenfj 2006/10/11
	    #2606 问题 2606.在手工添加64个ONU后，再实际注册新ONU时，出现异常*/
	/*ActivatePendingOnuMsg( PonPortIdx );
	ActivatePendingOnuMsg_conf( PonPortIdx, macAddress);*/

	/*  added by chenfj 2008-8-25
	    判断是否使能了ONU - PON 口绑定, 
	    若是, 则判断是否在其它PON口下有等待注册的ONU */

	/* 增加删除LOG中相关信息*/
	
#if 0
	{
	OnuMgmtTable[OnuEntry].DeviceInfo.DeviceNameLen = OnuConfigDefault.DeviceNameLen ;
	VOS_MemCpy( OnuMgmtTable[OnuEntry].DeviceInfo.DeviceName, OnuConfigDefault.DeviceName, OnuMgmtTable[OnuEntry].DeviceInfo.DeviceNameLen );
	slot = GetCardIdxByPonChip(OnuEntry/MAXONUPERPON);
	port = GetPonPortByPonChip(OnuEntry/MAXONUPERPON);
	devIdx = slot * 10000 + port * 1000 +( OnuEntry % MAXONUPERPON ) +1;
	/*sys_console_printf(" slot%d/ port%d onu%d  \r\n",slot,port,devIdx );*/
	sprintf(&(OnuMgmtTable[OnuEntry].DeviceInfo.DeviceName[OnuConfigDefault.DeviceNameLen]), "%05d", devIdx );
	OnuMgmtTable[OnuEntry].DeviceInfo.DeviceNameLen += 5;
	}

	VOS_MemCpy( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr, BYTES_IN_MAC_ADDRESS );

#endif	
	/*InitOneOnuByDefault( PonPortIdx, OnuIdx );*/
	VOS_TaskDelay(VOS_TICK_SECOND/5);
	return( ROK );

}
/*****************************************************
 *
 *    Function: SetOnuRegisterLimitFlag( short int PonPortIdx, int flag )
 *
 *    Param:  short int PonPortIdx -- the specific pon port 
 *               int flag -- V2R1_ENABLE/V2R1_DISABLE
 *
 *    Desc:  当ONU未通过AddOnuToPonPort命令指配,且标志设为V2R1_ENABLE时,禁止新的ONU注册
 *
 *    Return: ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int SetOnuRegisterLimitFlag( short int PonPortIdx, int flag )
{	
	int old_flag;

	CHECK_PON_RANGE
	
	if((flag != V2R1_ENABLE ) && (flag != V2R1_DISABLE )) return( RERROR );
#if 0	
	for( i= 0; i< MAXPON; i++)
		{
		old_flag = PonPortTable[PonPortIdx].AutoRegisterFlag;
		PonPortTable[i].AutoRegisterFlag = flag;
		
		if(( flag == V2R1_DISABLE ) &&(  old_flag  == V2R1_ENABLE ))
			{
			PonPortTable[i].AutoRegisterFlag = flag;
			/* added by chenfj 2006/09/22 */ 
			/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
			/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/
			ActivatePendingOnuMsg(i);
			}		
		}
#endif

	/*for( i= 0; i< MAXPON; i++)*/
		{
		old_flag = PonPortTable[PonPortIdx].AutoRegisterFlag;
		PonPortTable[PonPortIdx].AutoRegisterFlag = flag;

		if(( flag == V2R1_DISABLE ) && ( old_flag == V2R1_ENABLE ))
			{
			/* added by chenfj 2006/09/22 */ 
			/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
			/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/
			ActivatePendingOnuMsg( PonPortIdx );
			}
		}
	return( ROK );
}


STATUS t_changeOnuStatus( short int ponPortIdx, short int onuIdx,  short int actcode )
{
	ULONG onuEntry = ponPortIdx*MAXONUPERPON+onuIdx;
	ULONG act = 0;
	ULONG oper = 0;
	if( actcode == 1 )
	{
		act = LLID_ENTRY_ACTIVE;
		oper = ONU_OPER_STATUS_UP;
	}
	else if ( actcode == 2 )
	{
		act = LLID_ENTRY_UNKNOWN;
		oper = ONU_OPER_STATUS_DOWN;
	}

	sys_console_printf( "\r\nchangeonustatus: ponid:%d,    onuid:%d    actcode:%d", ponPortIdx, onuIdx, actcode );

	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].LlidTable[0].EntryStatus = act;
	OnuMgmtTable[onuEntry].OperStatus = oper;
	ONU_MGMT_SEM_GIVE;

	return( ROK );
		
}

int SetOnuRegisterLimitFlagAll( int flag )
{	
	int i;
	int old_flag;
	
	if((flag != V2R1_ENABLE ) && (flag != V2R1_DISABLE )) return( RERROR );
#if 0	
	for( i= 0; i< MAXPON; i++)
		{
		old_flag = PonPortTable[PonPortIdx].AutoRegisterFlag;
		PonPortTable[i].AutoRegisterFlag = flag;
		
		if(( flag == V2R1_DISABLE ) &&(  old_flag  == V2R1_ENABLE ))
			{
			PonPortTable[i].AutoRegisterFlag = flag;
			/* added by chenfj 2006/09/22 */ 
			/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
			/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/
			ActivatePendingOnuMsg(i);
			}		
		}
#endif

	for( i= 0; i< MAXPON; i++)
		{
		old_flag = PonPortTable[i].AutoRegisterFlag;
		PonPortTable[i].AutoRegisterFlag = flag;

		if(( flag == V2R1_DISABLE ) && ( old_flag == V2R1_ENABLE ))
			{
			/* added by chenfj 2006/09/22 */ 
			/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
			/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/
			ActivatePendingOnuMsg( i );
			}
		}
	return(ROK);
}

int  GetOnuRegisterLimitFlag(short int PonPortIdx, int *flag )
{
	CHECK_PON_RANGE
		
	if( flag == NULL ) return (RERROR );
	if(PonPortTable[PonPortIdx].AutoRegisterFlag == V2R1_ENABLE)
		*flag = V2R1_ENABLE;
	else *flag = V2R1_DISABLE;
	
	return ( ROK );
}

int GetOnuRegisterLimitFlagDufault( short int PonPortIdx, int *DefaultFlag )
{
	if( DefaultFlag == NULL ) return( RERROR );

	*DefaultFlag = PonPortConfigDefault.AutoRegisterFlag;

	return( ROK );
}

/* added by chenfj 2006/12/12
	     PON MAX RANGE */
int SetPonRange(short int PonPortIdx , int range)
{
	/*PON_olt_update_parameters_t  updated_parameters;*/
	short int ret = RERROR;
	int MaxRTT;
	short int PonChipType;
	
	
	CHECK_PON_RANGE

	SWAP_TO_ACTIVE_PON_PORT_INDEX(PonPortIdx)

	/*if(( range != PON_RANGE_CLOSE) && ( range != PON_RANGE_20KM ) && (range != PON_RANGE_40KM ) && (range != PON_RANGE_60KM )) return( RERROR );*/
	if( (unsigned int)range > PON_RANGE_80KM )
		return RERROR;

	PonChipType = V2R1_GetPonchipType(PonPortIdx);
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
		/* modified for PAS-SOFT V5.1 */
		MaxRTT = PON_MAX_RTT_40KM;
		if( range == PON_RANGE_20KM )
		{
			MaxRTT = PON_MAX_RTT_40KM; 
		}
		else if( range == PON_RANGE_40KM )
		{
			/*
			if( PonChipVer == PONCHIP_PAS5001 )
				MaxRTT  = PON_MAX_RTT_120KM;
			else if( PonChipVer == PONCHIP_PAS5201 )
			*/
			MaxRTT  = PON_MAX_RTT_80KM;
		}
		else if( range == PON_RANGE_60KM )
		{
			if( OLT_PONCHIP_ISPAS5001(PonChipType) )
				MaxRTT = PON_MAX_RTT_120KM;
			else if(OLT_PONCHIP_ISPAS(PonChipType))
                return( RERROR );
		}
		else if( range == PON_RANGE_CLOSE )
		{
			MaxRTT = V2R1_REGISTER_WINDOW_CLOSE;
		}

#if 1
		/*modified by liyang @2015-06-01 for Q26213*/
		if(PonPortTable[PonPortIdx].range  != range || PonPortTable[PonPortIdx].MaxRTT != MaxRTT)
	    {
	    	if ( OLT_CALL_ISOK(OLT_SetPonRange(PonPortIdx, range, MaxRTT)) )
      	  	{
           		 ret = ROK;
        	}
		}
		else
			ret = ROK;
#else    
		/*ret = PAS_update_olt_parameters(PonPortIdx, &updated_parameters );*/
		ret = SetPonPortMaxRTT( PonPortIdx,  MaxRTT);
#endif		
		if( ret == ROK ) 
		{
#if 0
			if(  range == PON_RANGE_20KM )
			{
				PonPortTable[PonPortIdx].range = PON_RANGE_20KM;
			}
			else if(  range == PON_RANGE_40KM )
			{
				PonPortTable[PonPortIdx].range = PON_RANGE_40KM;
			}
			else if(  range == PON_RANGE_60KM )
			{
				PonPortTable[PonPortIdx].range = PON_RANGE_60KM;
			}
			if( range == PON_RANGE_CLOSE )
			{
				PonPortTable[PonPortIdx].range = PON_RANGE_CLOSE;
			}
#endif		
			return( ROK );
		}		
		else
            return( RERROR );   
	}
    else if(OLT_PONCHIP_ISTK(PonChipType)) 
	{
		if( range == PON_RANGE_20KM )
		{
			MaxRTT = 2*20*5*1000/16+3125; 
		}
		else if( range == PON_RANGE_40KM )
		{
			MaxRTT  = 2*40*5*1000/16+3125;
		}
		else if( range == PON_RANGE_60KM )
		{
			MaxRTT  = 2*60*5*1000/16+3125;
		}
		else if( range == PON_RANGE_80KM )
		{
			MaxRTT  = 2*80*5*1000/16+3125;
		}
		else if( range == PON_RANGE_CLOSE )
		{
			MaxRTT = V2R1_REGISTER_WINDOW_CLOSE;
		}

		if(PonPortTable[PonPortIdx].range  != range || PonPortTable[PonPortIdx].MaxRTT != MaxRTT)
    	{
    		if ( OLT_CALL_ISOK(OLT_SetPonRange(PonPortIdx, range, MaxRTT)) )
  	  		{
       			 ret = ROK;
    		}
		}
		else
			ret = ROK;

		return ret;
	}
	else if(OLT_PONCHIP_ISGPON(PonChipType)) /*GPON中RTT无意义，只需要range*/
	{
		if(PonPortTable[PonPortIdx].range  != range )
    	{
    		if ( OLT_CALL_ISOK(OLT_SetPonRange(PonPortIdx, range, 0)) )
  	  		{
       			 ret = ROK;
    		}
		}
        else
			ret = ROK;		

		return ret;
	}
	else
    {  /* other pon chip handler */
		return( ROK );
	}

    return( RERROR );
}

int GetPonRange( short int PonPortIdx, int *range )
{
	CHECK_PON_RANGE

	if( range == NULL ) return( RERROR );

	*range = PonPortTable[PonPortIdx].range;
	return( ROK );
}

int  GetPonRangeDefault( int *rangeDefault )
{
	if( rangeDefault == NULL ) return( RERROR );
	*rangeDefault = PonPortConfigDefault.range;
	return( ROK );
}

	/* added by chenfj 2007-6-19  增加PON 端口保护切换*/
/*****************************************************
 *
 *    Function: EnablePonPortHotSwapPort(short int PonPortIdx1, short int PonPortIdx2  )
 *
 *    Param:  short int PonPortIdx1 -- the specific pon port 
 *                 short int PonPortIdx21 -- the specific pon port
 *
 *    Desc: 指定两个PON端口互为保护；
 *
 *    Return: ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/

#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )

int  SetPonPortSwapEnable( short int PonPortIdx, unsigned int SwapFlag )
{
	OLT_LOCAL_ASSERT(PonPortIdx);

	if( ( SwapFlag !=  V2R1_PON_PORT_SWAP_ENABLE ) && ( SwapFlag != V2R1_PON_PORT_SWAP_DISABLE)) return( RERROR );
	PonPortTable[PonPortIdx].swap_flag = (unsigned char)SwapFlag;	

	return( ROK );
}

int  SetPonPortSwapStatus( short int PonPortIdx, unsigned int SwapStatus )
{
	OLT_LOCAL_ASSERT(PonPortIdx);

	if( ( SwapStatus != V2R1_PON_PORT_SWAP_ACTIVE ) && ( SwapStatus != V2R1_PON_PORT_SWAP_PASSIVE ) && ( SwapStatus != V2R1_PON_PORT_SWAP_UNKNOWN )) return( RERROR );
	PonPortTable[PonPortIdx].swap_use = (unsigned char)SwapStatus;

	return( ROK );
}

int  PonPortSwapEnableQuery( short int PonPortIdx )
{
	OLT_LOCAL_ASSERT(PonPortIdx);

	return( PonPortTable[PonPortIdx].swap_flag );
}

int  PonPortHotStatusQuery( short int PonPortIdx )
{
	short int PonPortIdx_Swap;
	int swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
	//OLT_LOCAL_ASSERT(PonPortIdx);
	OLT_ASSERT(PonPortIdx);

	if(OLT_ISLOCAL(PonPortIdx))
	{
		return( PonPortTable[PonPortIdx].swap_use );
	}
	else if(OLT_ISNET(PonPortIdx))
	{
		if (SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER)
		{
#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
			if (OLT_CALL_ISOK(OLTRM_GetRemoteOltPartner(PonPortIdx, &PonPortIdx_Swap)))
			{
				if(V2R1_PON_PORT_SWAP_UNKNOWN == (swap_use = PonPortHotStatusQuery(PonPortIdx_Swap)))
				{
					return V2R1_PON_PORT_SWAP_UNKNOWN;
				}
				return V2R1_PON_PORT_SWAP_ACTIVE == swap_use ? V2R1_PON_PORT_SWAP_PASSIVE : V2R1_PON_PORT_SWAP_ACTIVE;
			}
#endif
		}
	}
	return swap_use;
}

int  PonPortIsSwaping( short int PonPortIdx )
{
	OLT_LOCAL_ASSERT(PonPortIdx);

	return( V2R1_SWAPPING_NO == PonPortTable[PonPortIdx].swapping ? 0 : 1 );
}

int  PonPortIsNotSwaping( short int PonPortIdx )
{
	OLT_LOCAL_ASSERT(PonPortIdx);

	return( V2R1_SWAPPING_NO == PonPortTable[PonPortIdx].swapping ? 1 : 0 );
}

int  PonPortSwapTimesQuery( short int PonPortIdx )
{
	OLT_LOCAL_ASSERT(PonPortIdx);

	return( PonPortTable[PonPortIdx].swap_times );
}

/* B--added by liwei056@2011-11-3 for D13215 */
int  PonPortSwapPortLocalQuery( short int PonPortIdx1, short int *PonPortIdx_Swap )
{
	short int PonPortIdx;
	
	if ( PonPortIdx_Swap == NULL ) return( RERROR );

	*PonPortIdx_Swap = RERROR;

    if ( OLT_ISLOCAL(PonPortIdx1) )
    {
    	if( PonPortSwapEnableQuery( PonPortIdx1 ) == V2R1_PON_PORT_SWAP_ENABLE )
        {
        	PonPortIdx = GetPonPortIdxBySlot((short int )(PonPortTable[PonPortIdx1].swap_slot), (short int)(PonPortTable[PonPortIdx1].swap_port));
            if ( RERROR != PonPortIdx )
            {
            	*PonPortIdx_Swap = PonPortIdx;

#if 0
            	if( PonPortSwapEnableQuery( PonPortIdx ) != V2R1_PON_PORT_SWAP_ENABLE ) return( RERROR );
            	
            	PonPortIdx = GetPonPortIdxBySlot((short int )(PonPortTable[PonPortIdx].swap_slot), (short int)(PonPortTable[PonPortIdx].swap_port));
            	CHECK_PON_RANGE
            	if( PonPortIdx != PonPortIdx1 ) return( RERROR );
#endif
        	
            	return( ROK );
            }
        }   
    }
    else
    {
        /* 通过本地查询来定位远端的保护设置 */
        int iSwapSlot, iSwapPort; 
        int iRemoteSlot, iRemotePort; 

        iRemoteSlot = OLT_SLOT_ID(PonPortIdx1);
        iRemotePort = OLT_PORT_ID(PonPortIdx1);
        for (PonPortIdx=0; PonPortIdx<MAXPON; PonPortIdx++)
        {
            if ( ROK == PonPortSwapSlotQuery(PonPortIdx, &iSwapSlot, &iSwapPort) )
            {
                if ( (iSwapSlot == iRemoteSlot)
                    && (iSwapPort == iRemotePort) )
                {
                    *PonPortIdx_Swap = PonPortIdx;
                    
                    return( ROK );
                }
            }
        }
    }
	
    return( RERROR );
}
/* E--added by liwei056@2011-11-3 for D13215 */

int  PonPortSwapPortQuery( short int PonPortIdx1, short int *PonPortIdx_Swap )
{
	short int PonPortIdx;
	
	if ( PonPortIdx_Swap == NULL ) return( RERROR );

	*PonPortIdx_Swap = RERROR;

    if ( OLT_ISLOCAL(PonPortIdx1) )
    {
    	if( PonPortSwapEnableQuery( PonPortIdx1 ) == V2R1_PON_PORT_SWAP_ENABLE )
        {
        	PonPortIdx = GetPonPortIdxBySlot((short int )(PonPortTable[PonPortIdx1].swap_slot), (short int)(PonPortTable[PonPortIdx1].swap_port));
            if ( RERROR != PonPortIdx )
            {
            	*PonPortIdx_Swap = PonPortIdx;

#if 0
            	if( PonPortSwapEnableQuery( PonPortIdx ) != V2R1_PON_PORT_SWAP_ENABLE ) return( RERROR );
            	
            	PonPortIdx = GetPonPortIdxBySlot((short int )(PonPortTable[PonPortIdx].swap_slot), (short int)(PonPortTable[PonPortIdx].swap_port));
            	CHECK_PON_RANGE
            	if( PonPortIdx != PonPortIdx1 ) return( RERROR );
#endif
        	
            	return( ROK );
            }
        }   
    }
    else
    {
        if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
        {
#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
            if ( OLT_CALL_ISOK ( OLTRM_GetRemoteOltPartner(PonPortIdx1, PonPortIdx_Swap) ) )
            {
                return( ROK );
            }
#endif
        }

        /* 通过本地查询来定位远端的保护设置 */
        {
            int iSwapSlot, iSwapPort; 
            int iRemoteSlot, iRemotePort; 

            iRemoteSlot = OLT_SLOT_ID(PonPortIdx1);
            iRemotePort = OLT_PORT_ID(PonPortIdx1);
            for (PonPortIdx=0; PonPortIdx<MAXPON; PonPortIdx++)
            {
                if ( ROK == PonPortSwapSlotQuery(PonPortIdx, &iSwapSlot, &iSwapPort) )
                {
                    if ( (iSwapSlot == iRemoteSlot)
                        && (iSwapPort == iRemotePort) )
                    {
                        *PonPortIdx_Swap = PonPortIdx;
                        
                        return( ROK );
                    }
                }
            }
        }
    }
	
    return( RERROR );
}

int  PonPortSwapSlotQuery( short int PonPortIdx1, int *PonPortIdx2_Slot, int *PonPortIdx2_Port )
{
	short int PonPortIdx2 = 0;
    //OLT_LOCAL_ASSERT(PonPortIdx1);
	OLT_ASSERT(PonPortIdx1);

	if(OLT_ISLOCAL(PonPortIdx1))
	{
		if ( V2R1_PON_PORT_SWAP_ENABLE == PonPortTable[PonPortIdx1].swap_flag )
		{
			if ( NULL != PonPortIdx2_Slot )
			{
				*PonPortIdx2_Slot = PonPortTable[PonPortIdx1].swap_slot;
			}

			if ( NULL != PonPortIdx2_Port )
			{
				*PonPortIdx2_Port = PonPortTable[PonPortIdx1].swap_port;
			}

			return( ROK );
		}
	}
	else if(OLT_ISNET(PonPortIdx1))
	{
		if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
        {
#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
			if(OLT_CALL_ISOK(OLTRM_GetRemoteOltPartner(PonPortIdx1, &PonPortIdx2)))
			{
				if ( NULL != PonPortIdx2_Slot )
				{
					*PonPortIdx2_Slot = GetCardIdxByPonChip(PonPortIdx2);
				}

				if ( NULL != PonPortIdx2_Port )
				{
					*PonPortIdx2_Port = GetPonPortByPonChip(PonPortIdx2); 
				}
				return( ROK );
			}
#endif
        }
	}
    return( RERROR );
}

int ClearHotSwapActivePonRunningData(short int PonPortIdx)
{
	OLT_LOCAL_ASSERT(PonPortIdx);

	PonPortTable[PonPortIdx].swap_flag = V2R1_PON_PORT_SWAP_DISABLE;
	PonPortTable[PonPortIdx].swap_slot = 0;
	PonPortTable[PonPortIdx].swap_port =  0;
	PonPortTable[PonPortIdx].swap_timer = 0;
	PonPortTable[PonPortIdx].swapping = V2R1_SWAPPING_NO;
	PonPortTable[PonPortIdx].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
	PonPortTable[PonPortIdx].swapHappened = FALSE;
	PonPortTable[PonPortIdx].swap_monitor = 0;
	PonPortTable[PonPortIdx].swap_mode = V2R1_PON_PORT_SWAP_DISABLED;
	PonPortTable[PonPortIdx].swap_reason = 0;
	PonPortTable[PonPortIdx].swap_status = 0;
	PonPortTable[PonPortIdx].swap_times = 0;

    EnablePonPortLinkedSwPort(PonPortIdx);
    
	if( PonPortIsWorking( PonPortIdx ) == TRUE )
	{
		/*StartUp_olt( PonPortIdx );*/
#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
		ClearPonPortAutoPoetect(PonPortIdx);
#endif
		PONTx_Enable( PonPortIdx, PONPORT_TX_SWITCH );
	}
	
	return( ROK );
}

int ClearHotSwapPassivePonRunningData(short int PonPortIdx)
{
	short int  OnuIdx;
     
	OLT_LOCAL_ASSERT(PonPortIdx);

	PonPortTable[PonPortIdx].swap_flag = V2R1_PON_PORT_SWAP_DISABLE;
	PonPortTable[PonPortIdx].swap_slot = 0;
	PonPortTable[PonPortIdx].swap_port =  0;
	PonPortTable[PonPortIdx].swap_timer = 0;
	PonPortTable[PonPortIdx].swapping = V2R1_SWAPPING_NO;
	PonPortTable[PonPortIdx].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
	PonPortTable[PonPortIdx].swapHappened = FALSE;
	PonPortTable[PonPortIdx].swap_monitor = 0;
	PonPortTable[PonPortIdx].swap_mode = V2R1_PON_PORT_SWAP_DISABLED;
	PonPortTable[PonPortIdx].swap_reason = 0;
	PonPortTable[PonPortIdx].swap_status = 0;
	PonPortTable[PonPortIdx].swap_times = 0;

    EnablePonPortLinkedSwPort(PonPortIdx);
    
	if( PonPortIsWorking( PonPortIdx ) == TRUE )
	{
		/*StartUp_olt( PonPortIdx);*/
#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
		ClearPonPortAutoPoetect(PonPortIdx);
#endif
		PONTx_Enable( PonPortIdx, PONPORT_TX_SWITCH );
	}
	
#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
	for(OnuIdx=0; OnuIdx< MAXONUPERPON; OnuIdx++)
	{
		PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION;
	}

    /* B--Modified by liwei056@2010-5-25 for D10168 */
#if 1
	for(OnuIdx=PonPortTable[PonPortIdx].Downlink_vlan_Cfg_Max; OnuIdx>=0; OnuIdx--)
	{
		if( 0 != PonPortTable[PonPortIdx].Downlink_vlan_Config[OnuIdx] )
        {
            /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
 			OLT_DelVidDownlinkMode(PonPortIdx, OnuIdx);
        }      
	}
    PonPortTable[PonPortIdx].Downlink_vlan_manipulation[0].vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION;
    PonPortTable[PonPortIdx].Downlink_vlan_manipulation[0].original_vlan_id = 1;
    PonPortTable[PonPortIdx].Downlink_vlan_manipulation[1].vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG;
    PonPortTable[PonPortIdx].Downlink_vlan_manipulation[1].original_vlan_id = 1;
    PonPortTable[PonPortIdx].Downlink_vlan_Man_Max   = 1;
    PonPortTable[PonPortIdx].Downlink_vlan_Cfg_Max   = 0;
#else
	for(OnuIdx=0; OnuIdx<MAX_VID_DOWNLINK; OnuIdx++)
	{
		if(PonPortTable[PonPortIdx].Downlink_vlan_manipulation[OnuIdx].vlan_manipulation != PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION)
			PAS_delete_vid_downlink_mode(PonPortIdx, PonPortTable[PonPortIdx].Downlink_vlan_manipulation[OnuIdx].original_vlan_id);
		PonPortTable[PonPortIdx].Downlink_vlan_manipulation[OnuIdx].vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION;
	}
#endif
    /* E--Modified by liwei056@2010-5-25 for D10168 */

    PonPortTable[PonPortIdx].vlan_tpid = DEFAULT_PON_VLAN_TPID;
#endif

	return( ROK );

}

int  ThisIsHotSwapPort( short int PonPortIdx1, short int PonPortIdx2 )
{
	short int PonPortIdx;

	PonPortIdx = PonPortIdx1;
	CHECK_PON_RANGE

#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
#else
	PonPortIdx = PonPortIdx2;
	CHECK_PON_RANGE
#endif

	if( PonPortSwapPortQuery( PonPortIdx1, &PonPortIdx ) != ROK ) return(  V2R1_ISNOT_HOT_SWAP_PORT );

	if( PonPortIdx != PonPortIdx2 ) return(  V2R1_ISNOT_HOT_SWAP_PORT );

#if 0
	/* modified by chenfj 2007-9-20 
		问题单#5373: 设置PON 5/1和 5/2互为保护倒换。每重启一次PON板，就会引起一次保护倒换
		*/
	if(( PonPortHotStatusQuery(PonPortIdx1) != V2R1_PON_PORT_SWAP_ACTIVE ) && ( PonPortHotStatusQuery(PonPortIdx2) != V2R1_PON_PORT_SWAP_ACTIVE ))
		return( V2R1_SWAP_PORT_BUT_NOT_ACTIVE );
#endif
	
	return( V2R1_IS_HOT_SWAP_PORT );
}

/* B--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */
int GetPonPortHotSwapMode(short int PonPortIdx)
{
    OLT_LOCAL_ASSERT(PonPortIdx);
    
    return PonPortTable[PonPortIdx].swap_mode;
}

int GetPonPortHotSwapTriggers(short int PonPortIdx)
{
    OLT_LOCAL_ASSERT(PonPortIdx);

    if ( PonPortTable[PonPortIdx].swap_triggers & PROTECT_SWITCH_TRIGGER_UPLINKDOWN )
    {
        /* 矫正拷贝配置 */
        if ( (0 == PonPortTable[PonPortIdx].protect_slot)
            || (0 == PonPortTable[PonPortIdx].protect_port) )
        {
            PonPortTable[PonPortIdx].swap_triggers &= ~PROTECT_SWITCH_TRIGGER_UPLINKDOWN;
        }
    }
    
    return PonPortTable[PonPortIdx].swap_triggers;
}

int GetPonPortHotProtectedPort(short int PonPortIdx, unsigned long *ulSlot, unsigned long *ulPort)
{
    OLT_LOCAL_ASSERT(PonPortIdx);

    if ( PonPortTable[PonPortIdx].swap_triggers & PROTECT_SWITCH_TRIGGER_UPLINKDOWN )
    {
        if ( (0 != (*ulSlot = PonPortTable[PonPortIdx].protect_slot))
            && (0 != (*ulPort = PonPortTable[PonPortIdx].protect_port)) )
        {
            return ROK;
        }
    }
    
    return RERROR;
}

int  PonPortProtectedPortLocalQuery( short int PonPortIdx1, short int *PonPortIdx_Protected )
{
    ULONG ulSlot, ulPort;
	
    if ( PonPortIdx_Protected == NULL ) return( RERROR );

    *PonPortIdx_Protected = RERROR;

    if ( OLT_ISLOCAL(PonPortIdx1) )
    {
        if ( ROK == GetPonPortHotProtectedPort(PonPortIdx1, &ulSlot, &ulPort) )
        {
        	*PonPortIdx_Protected = OLT_DEVICE_ID(ulSlot, ulPort);
            return( ROK );
        }
    }
    else
    {
        /* 通过本地查询来定位远端的保护设置 */
        ULONG ulRemoteSlot, ulRemotePort; 

        ulRemoteSlot = OLT_SLOT_ID(PonPortIdx1);
        ulRemotePort = OLT_PORT_ID(PonPortIdx1);
        for (PonPortIdx1=0; PonPortIdx1<MAXPON; PonPortIdx1++)
        {
            if ( ROK == GetPonPortHotProtectedPort(PonPortIdx1, &ulSlot, &ulPort) )
            {
                if ( (ulSlot == ulRemoteSlot)
                    && (ulPort == ulRemotePort) )
                {
                    *PonPortIdx_Protected = PonPortIdx1;
                    
                    return( ROK );
                }
            }
        }
    }
	
    return( RERROR );
}


#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
/* B--added by liwei056@2010-9-29 for 5.3.12's RedundancyUpgrade */
int olt_hotpair_num;
olt_id_list_t master_list;
olt_id_list_t slave_list;
gpio_list_t   gpio_list;

void PonHotSwapInit()
{
    olt_hotpair_num = 0;
	/*for onu swap by jinhl@2013-02-22*/
    VOS_MemSet(master_list, 0xFF, sizeof(olt_id_list_t));
	VOS_MemSet(slave_list, 0xFF, sizeof(olt_id_list_t));
#if 1 /*modified by liyang @2015-04-15 for pon fast switch*/
    VOS_MemSet(gpio_list, 0xFF, sizeof(gpio_list_t));
#else
	/*for onu swap by jinhl@2013-06-14  这里,gpio_list的值一定为0，否则倒换会出问题*/
    VOS_MemSet(gpio_list, 0, sizeof(gpio_list_t));
#endif
}

#ifdef PAS_SOFT_VERSION_V5_3_12
#if ( EPON_SUBMODULE_PON_FAST_SWAP_V2 == EPON_MODULE_YES )
int FindSwapPair(short int MasterPonPortIdx, short int SlavePonPortIdx)
{
    int i;

    for (i=0; i<olt_hotpair_num; i++)
    {
        if ((master_list[i] == MasterPonPortIdx)
            || (master_list[i] == SlavePonPortIdx)
            || (slave_list[i] == MasterPonPortIdx)
            || (slave_list[i] == SlavePonPortIdx))
        {
            return i;
        }
    }

    return -1;
}

int AddSwapPair(short int MasterPonPortIdx, short int SlavePonPortIdx)
{
    int iRlt;

    VOS_TaskLock();    
    if ( 0 <= (iRlt = FindSwapPair(MasterPonPortIdx, SlavePonPortIdx)) )
    {
        if ((master_list[iRlt] == SlavePonPortIdx)
            || (slave_list[iRlt] == MasterPonPortIdx))
        {
            PON_olt_id_t swap_olt_id;
            
            /* 主备切换 */
            swap_olt_id       = master_list[iRlt];
            master_list[iRlt] = slave_list[iRlt];
            slave_list[iRlt]  = swap_olt_id;
        }
    }
    else
    {
        if ( olt_hotpair_num < OLT_ID_REDUNDANCY_LIST_MAX_SIZE )
        {
            master_list[olt_hotpair_num] = MasterPonPortIdx;
            slave_list[olt_hotpair_num]  = SlavePonPortIdx;
            iRlt = ++olt_hotpair_num;
        }
    }
    VOS_TaskUnlock();    

    return iRlt;
}

int ReleaseSwapPair(short int MasterPonPortIdx, short int SlavePonPortIdx)
{
    int iRlt;

    VOS_TaskLock();    
    if ( 0 <= (iRlt = FindSwapPair(MasterPonPortIdx, SlavePonPortIdx)) )
    {
        if ( iRlt < --olt_hotpair_num )
        {
            master_list[iRlt] = master_list[olt_hotpair_num];
            slave_list[iRlt]  = slave_list[olt_hotpair_num];
			/*for onu swap by jinhl@2013-02-22*/
			master_list[olt_hotpair_num]= REDUNDANCY_OLT_ID_INVALID;
			slave_list[olt_hotpair_num] = REDUNDANCY_OLT_ID_INVALID;
        }
		else
		{
		    /*for onu swap by jinhl@2013-02-22*/
			master_list[olt_hotpair_num]= REDUNDANCY_OLT_ID_INVALID;
			slave_list[olt_hotpair_num] = REDUNDANCY_OLT_ID_INVALID;
		}
    }
    VOS_TaskUnlock();    

    return iRlt;
}

int SetPonFastHotSwapList()
{
   
    return redundancy_master_slave_config(master_list, olt_hotpair_num, slave_list, olt_hotpair_num, gpio_list, gpio_list);
}
#endif
#endif
/* E--added by liwei056@2010-9-29 for 5.3.12's RedundancyUpgrade */
    
/* B--modified by liwei056@2010-9-29 for 5.3.12's RedundancyUpgrade */
int ActivePonFastHotSwapSetup(short int ActivePonPortIdx, short int PassivePonPortIdx)
{
    int iRlt;
    
#ifdef PAS_SOFT_VERSION_V5_3_12
#if ( EPON_SUBMODULE_PON_FAST_SWAP_V2 == EPON_MODULE_YES )
    if ( 0 <= (iRlt = AddSwapPair(ActivePonPortIdx, PassivePonPortIdx)) )
    {
        iRlt = SetPonFastHotSwapList();
    }
#else
    {
        olt_id_list_t master_list;
        olt_id_list_t slave_list;
        gpio_list_t   gpio_list;

        master_list[0] = ActivePonPortIdx;
        slave_list[0] = PassivePonPortIdx;
        gpio_list[0] = PON_REDUNDANCY_NO_GPIO;
        
        iRlt = redundancy_master_slave_config(master_list, 1, slave_list, 1, gpio_list, gpio_list);
    }
#endif
#elif defined PAS_SOFT_VERSION_V5_3_11
    {
        olt_id_list_t master_list;
        olt_id_list_t slave_list;
        gpio_list_t   gpio_list;

        master_list[0] = ActivePonPortIdx;
        slave_list[0] = PassivePonPortIdx;
        gpio_list[0] = PON_REDUNDANCY_NO_GPIO;
        
        iRlt = redundancy_master_slave_config(master_list, 1, slave_list, 1, gpio_list, gpio_list);
    }
#else
    iRlt = -1;
#endif

    return iRlt;
}

int ActivePonGpioHotSwapSetup(short int ActivePonPortIdx, short int PassivePonPortIdx)
{
    olt_id_list_t master_list;
    olt_id_list_t slave_list;
    gpio_list_t   master_gpio_list;
    gpio_list_t   slave_gpio_list;

    master_list[0] = ActivePonPortIdx;
    slave_list[0]  = PassivePonPortIdx;
    master_gpio_list[0] = 0;
    slave_gpio_list[0]  = 2;
    
    return redundancy_master_slave_config(master_list, 1, slave_list, 1, master_gpio_list, slave_gpio_list);
}

int InActivePonFastHotSwapSetup(short int ActivePonPortIdx, short int PassivePonPortIdx)
{
    int iRlt;
    
#ifdef PAS_SOFT_VERSION_V5_3_12
#if ( EPON_SUBMODULE_PON_FAST_SWAP_V2 == EPON_MODULE_YES )
    if ( 0 <= (iRlt = ReleaseSwapPair(ActivePonPortIdx, PassivePonPortIdx)) )
    {
        iRlt = SetPonFastHotSwapList();
    }
#else
   
    iRlt = redundancy_disable_olt(PassivePonPortIdx);
    iRlt = redundancy_disable_olt(ActivePonPortIdx);
#endif
#elif defined PAS_SOFT_VERSION_V5_3_11
    iRlt = redundancy_disable_olt(PassivePonPortIdx);
    iRlt = redundancy_disable_olt(ActivePonPortIdx);
	
#else
    iRlt = -1;
#endif

    return iRlt;
}
/* E--modified by liwei056@2010-9-29 for 5.3.12's RedundancyUpgrade */

int HotSwapRedundancyPon(short int oldActivePonPortIdx, short int newActivePonPortIdx)
{
    return redundancy_switch_over(oldActivePonPortIdx, newActivePonPortIdx);
}

int SetPonPortAutoProtectMode( unsigned int slot, unsigned int port, int swapMode )
{
	short int PonPortIdx;
	short int PartnerPonPortIdx;
    int oldSwapMode = 0;
	int ret = 0;
	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
    if( PonPortSwapPortQuery( PonPortIdx, &PartnerPonPortIdx ) != ROK )
    {
        return(  V2R1_PORT_HAS_NO_HOT_SWAP_PORT );
    }
    /*for onu swap by jinhl@2013-02-22*/
    oldSwapMode = GetPonPortHotSwapMode(PonPortIdx);
    if ( oldSwapMode == swapMode )
    {
        return V2R1_SWAP_MODE_ALL_ONE;
    }
    /*for onu swap by jinhl@2013-02-22*/
    if( (V2R1_PON_PORT_SWAP_ONU == swapMode) || (V2R1_PON_PORT_SWAP_ONU == oldSwapMode))
	{
	    
		if(HasOnuOnlineOrRegistering(PonPortIdx) || HasOnuOnlineOrRegistering(PartnerPonPortIdx))
		{
			
    		return V2R1_SWAP_PORT_HASONU;
		}
	}
	
	{
	    ret = ActivePonHotSwapPort( PonPortIdx, PartnerPonPortIdx, V2R1_PON_PORT_SWAP_DISABLED, V2R1_PON_PORT_SWAP_SYNC);
	}

    if(EXIT_OK == ret)
	{
	    ret = ActivePonHotSwapPort( PonPortIdx, PartnerPonPortIdx, swapMode, V2R1_PON_PORT_SWAP_SYNC);
	}
	return ret;
}

int ShowPonPortAutoProtectMode(struct vty* vty, unsigned int slot, unsigned int port )
{
    int ret;
    int swap_mode;
    short int PonPortIdx;
    short int PonPortIdx_Swap;
    const char *pcszPonProtectMode;
    int state = 0;
    PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
    ret = PonPortSwapPortQuery( PonPortIdx, &PonPortIdx_Swap);
    if( ret != ROK )  return ( V2R1_PORT_HAS_NO_HOT_SWAP_PORT );

    swap_mode = GetPonPortHotSwapMode(PonPortIdx);
    switch (swap_mode)
    {
        case V2R1_PON_PORT_SWAP_SLOWLY:
            pcszPonProtectMode = "slowness";
            break;
        case V2R1_PON_PORT_SWAP_QUICKLY:
            pcszPonProtectMode = "sensitive";
            break;
		/*for onu swap by jinhl@2013-02-22*/
		case V2R1_PON_PORT_SWAP_ONU:
			
            ret = OLT_GetRdnState(PonPortIdx, &state);
			if(EXIT_OK == ret)
			{
			    if(PON_OLT_REDUNDANCY_STATE_ASSIGN_ODD_LLID == state)
		    	{
		    	    pcszPonProtectMode = "onu-optic/odd-llid";
		    	}
				else if(PON_OLT_REDUNDANCY_STATE_ASSIGN_EVEN_LLID == state)
				{
				    pcszPonProtectMode = "onu-optic/even-llid";
				}
				else
				{
				    pcszPonProtectMode = "onu-optic/unknowed-llid";
				}
			}
			else
			{
				pcszPonProtectMode = "onu-optic/unknowed-method";
			}
            break;
        default:
            pcszPonProtectMode = "unknowned";
    }
    vty_out(vty, "  pon %d/%d's auto-protect mode is %s\r\n", slot, port, pcszPonProtectMode);

    return 0;
}

int ShowPonPortAutoProtectTrigger(struct vty* vty, unsigned int slot, unsigned int port )
{
    static const char *acszTriggerStrLst[] = {
                                    "manual",
                                    "optic-loss",
                                    "port-loose",
                                    "optic-power-abnormal",
                                    "optic-data-abnormal",
                                    "uplink-protect",
                                    "downlink-protect"
                                    };
    int ret;
    int iSwapTriggers;
    int iTriggerPos;
    int SpanLen;
    char szSpanStr[16];
    short int PonPortIdx;

    iSwapTriggers = 0;
    PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
    if ( OLT_LOCAL_ISVALID(PonPortIdx) )
    {
        iSwapTriggers = GetPonPortHotSwapTriggers(PonPortIdx);
    	if( PonPortSwapEnableQuery( PonPortIdx ) != V2R1_PON_PORT_SWAP_ENABLE )
        {
            iSwapTriggers &= ~V2R1_PON_PORT_SWAP_TRIGGER;
        }
    }

    if ( 0 == iSwapTriggers )
    {
        vty_out(vty, "  pon %d/%d no trigger-protect.\r\n", slot, port);
    }
    else
    {
        SpanLen = 0;
        
        for ( iTriggerPos = 0; iTriggerPos < ARRAY_SIZE(acszTriggerStrLst); ++iTriggerPos )
        {
            if ( iSwapTriggers & (1 << iTriggerPos) )
            {
                if ( SpanLen > 0 )
                {
                    vty_out(vty, szSpanStr, "");
                    vty_out(vty, "%s\r\n", acszTriggerStrLst[iTriggerPos]);
                }
                else
                {
                    SpanLen = vty_out(vty, "  pon %d/%d's trigger-protect: ", slot, port);
                    vty_out(vty, "%s\r\n", acszTriggerStrLst[iTriggerPos]);

                    VOS_Snprintf(szSpanStr, 15, "%%%ds", SpanLen);
                }
            }
        }

        if ( 0 == SpanLen )
        {
            vty_out(vty, "  pon %d/%d no trigger-protect.\r\n", slot, port);
        }
    }

    return 0;
}
#endif
/* E--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */

/* B--added by liwei056@2012-7-4 for Onu-OpticSwicthHover */
#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
/*Begin:for onu swap by jinhl@2013-02-22*/
int ShowAllPonPortAutoProtect(struct vty* vty )
{
    int ret;
	int flag =0;
	int iProtectMode = 0;
	int state = 0;

	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	unsigned int   ul_desSlot = 0, ul_desPort=0;
	short int  Src_PonPort, Des_PonPort;
	for( Src_PonPort = 0; Src_PonPort < MAXPON; Src_PonPort ++ )
	{
		ret = PonPortSwapEnableQuery( Src_PonPort );
		if( ret == V2R1_PON_PORT_SWAP_DISABLE ) continue;
		
		ul_srcSlot = GetCardIdxByPonChip( Src_PonPort );
		ul_srcPort = GetPonPortByPonChip( Src_PonPort );
		
		ret  = PonPortAutoProtectPortQuery( ul_srcSlot, ul_srcPort, &ul_desSlot, &ul_desPort );
		if( ret != ROK ) continue;

		Des_PonPort = GetPonPortIdxBySlot((short int )ul_desSlot, (short int)ul_desPort);
		if( Des_PonPort == RERROR ) continue;
		
        iProtectMode = GetPonPortHotSwapMode(Src_PonPort);
		if(V2R1_PON_PORT_SWAP_ONU == iProtectMode) continue;
			
		if( flag == 0 )
		{
			vty_out( vty, "PON Automatic Protection Switching\r\n");
			vty_out( vty, "-----------------------------------\r\n");
			vty_out( vty, "  ACTIVE PON       PASSIVE PON \r\n");
			flag = 1;
		}
		
		if ( Src_PonPort < Des_PonPort )
		{
#ifdef __CTC_TEST
			display_pon_aps_status( vty, Src_PonPort, ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort);
#else
			if( PonPortHotStatusQuery( Src_PonPort ) == V2R1_PON_PORT_SWAP_ACTIVE )
				vty_out( vty, "   %2d/%-2d                %2d/%-2d\r\n", ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort );
			else 
				vty_out( vty, "   %2d/%-2d                %2d/%-2d\r\n", ul_desSlot, ul_desPort, ul_srcSlot, ul_srcPort );
#endif		
		}
	
	} 
	
    return EXIT_OK;
}

int ShowAllPonPortOnuProtect(struct vty* vty )
{
    int ret;
	int flag =0;
	int iProtectMode = 0;
	int state = 0;

	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	unsigned int   ul_desSlot = 0, ul_desPort=0;
	short int  Src_PonPort, Des_PonPort;
	for( Src_PonPort = 0; Src_PonPort < MAXPON; Src_PonPort ++ )
	{
		ret = PonPortSwapEnableQuery( Src_PonPort );
		if( ret == V2R1_PON_PORT_SWAP_DISABLE ) continue;
		
		ul_srcSlot = GetCardIdxByPonChip( Src_PonPort );
		ul_srcPort = GetPonPortByPonChip( Src_PonPort );
		
		ret  = PonPortAutoProtectPortQuery( ul_srcSlot, ul_srcPort, &ul_desSlot, &ul_desPort );
		if( ret != ROK ) continue;

		Des_PonPort = GetPonPortIdxBySlot((short int )ul_desSlot, (short int)ul_desPort);
		if( Des_PonPort == RERROR ) continue;

        iProtectMode = GetPonPortHotSwapMode(Src_PonPort);
		if(V2R1_PON_PORT_SWAP_ONU != iProtectMode) continue;
		if( flag == 0 )
		{
			vty_out( vty, "PON ONU Protection Switching\r\n");
			vty_out( vty, "-----------------------------------\r\n");
			vty_out( vty, "  ODD PON           EVEN PON \r\n");
			flag = 1;
		}
		
		if ( Src_PonPort < Des_PonPort )
		{
            ret = OLT_GetRdnState(Src_PonPort, &state);
			if((EXIT_OK == ret) && (PON_OLT_REDUNDANCY_STATE_ASSIGN_ODD_LLID == state))
			{
			    vty_out( vty, "   %2d/%-2d              %2d/%-2d\r\n", ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort );
			}
			else if((EXIT_OK == ret) && (PON_OLT_REDUNDANCY_STATE_ASSIGN_EVEN_LLID == state))
			{
			 	vty_out( vty, "   %2d/%-2d              %2d/%-2d\r\n", ul_desSlot, ul_desPort, ul_srcSlot, ul_srcPort );
			}
			else
			{
			     vty_out( vty, "   %2d/%-2d              %2d/%-2d   unknowed llid-method\r\n", ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort );
			}
			
		}
	
	}  
	return EXIT_OK;
    
}
/*End:for onu swap by jinhl@2013-02-22*/
int ActivePonOnuHotSwapSetup(short int ActivePonPortIdx, short int PassivePonPortIdx)
{
    int iRlt;
    
#ifdef PAS_SOFT_VERSION_V5_3_12
    redundancy_onu_optical_olt_pair_t olt_pair;
    int state = 0;
    iRlt = OLT_GetRdnState(ActivePonPortIdx, &state);
	if( EXIT_OK == iRlt )
	{
	    if(PON_OLT_REDUNDANCY_STATE_ASSIGN_ODD_LLID == state)
    	{
    	    olt_pair.olt_assign_odd_llid  = ActivePonPortIdx;
		    olt_pair.olt_assign_even_llid = PassivePonPortIdx;
    	}
		else if(PON_OLT_REDUNDANCY_STATE_ASSIGN_EVEN_LLID== state)
		{
		    olt_pair.olt_assign_odd_llid  = PassivePonPortIdx;
		    olt_pair.olt_assign_even_llid = ActivePonPortIdx;
		}
		else
		{
		    iRlt = EXIT_ERROR;
		}
	}
	if(EXIT_OK != iRlt)
	{
	    iRlt = OLT_GetRdnState(PassivePonPortIdx, &state);
		if( EXIT_OK == iRlt )
		{
		    if(PON_OLT_REDUNDANCY_STATE_ASSIGN_ODD_LLID == state)
	    	{
	    	    olt_pair.olt_assign_odd_llid  = PassivePonPortIdx;
			    olt_pair.olt_assign_even_llid = ActivePonPortIdx;
	    	}
			else if(PON_OLT_REDUNDANCY_STATE_ASSIGN_EVEN_LLID== state)
			{
			    olt_pair.olt_assign_odd_llid  = ActivePonPortIdx;
			    olt_pair.olt_assign_even_llid = PassivePonPortIdx;
			}
			else
			{
			    iRlt = EXIT_ERROR;
			}
		}
	}
	if(EXIT_OK != iRlt)
	{
	    olt_pair.olt_assign_odd_llid  = ActivePonPortIdx;
	    olt_pair.olt_assign_even_llid = PassivePonPortIdx;
	}

#if 0
    (void)PAS_stop_external_dba_algorithm(ActivePonPortIdx);
    (void)PAS_stop_external_dba_algorithm(PassivePonPortIdx);
#endif

    iRlt = redundancy_onu_optical_config_olt_pair(olt_pair);
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"ActivePonOnuHotSwap, ActivePonPortIdx:%d, PassivePonPortIdx:%d.\r\n"
		, ActivePonPortIdx, PassivePonPortIdx);
    
#if 0
    (void)PAS_start_dba_algorithm( ActivePonPortIdx, FALSE, 0, NULL);
    (void)PAS_start_dba_algorithm( PassivePonPortIdx, FALSE, 0, NULL);
#endif
#else
    iRlt = -1;
#endif

    return iRlt;
}

int InActivePonOnuHotSwapSetup(short int ActivePonPortIdx, short int PassivePonPortIdx)
{
    int iRlt;
    
#ifdef PAS_SOFT_VERSION_V5_3_12
    redundancy_onu_optical_olt_pair_t olt_pair;
    int state = 0;
    iRlt = OLT_GetRdnState(ActivePonPortIdx, &state);
	if( EXIT_OK == iRlt )
	{
	    if(PON_OLT_REDUNDANCY_STATE_ASSIGN_ODD_LLID == state)
    	{
    	    olt_pair.olt_assign_odd_llid  = ActivePonPortIdx;
		    olt_pair.olt_assign_even_llid = PassivePonPortIdx;
			iRlt = EXIT_OK;
    	}
		else if(PON_OLT_REDUNDANCY_STATE_ASSIGN_EVEN_LLID == state)
		{
		    olt_pair.olt_assign_odd_llid  = PassivePonPortIdx;
		    olt_pair.olt_assign_even_llid = ActivePonPortIdx;
			iRlt = EXIT_OK;
		}
		else
		{
		    iRlt = EXIT_ERROR;
		}
	}

	if(EXIT_OK != iRlt)
	{
	    iRlt = OLT_GetRdnState(PassivePonPortIdx, &state);
		if( EXIT_OK == iRlt )
		{
		    if(PON_OLT_REDUNDANCY_STATE_ASSIGN_ODD_LLID == state)
	    	{
	    	    olt_pair.olt_assign_odd_llid  = PassivePonPortIdx;
			    olt_pair.olt_assign_even_llid = ActivePonPortIdx;
				iRlt = EXIT_OK;
	    	}
			else if(PON_OLT_REDUNDANCY_STATE_ASSIGN_EVEN_LLID== state)
			{
			    olt_pair.olt_assign_odd_llid  = ActivePonPortIdx;
			    olt_pair.olt_assign_even_llid = PassivePonPortIdx;
				iRlt = EXIT_OK;
			}
			else
			{
			    iRlt = EXIT_ERROR;
			}
		}
	}
	if(EXIT_OK == iRlt)
	{
	    iRlt = redundancy_onu_optical_remove_olt_pair(olt_pair);
	}
	else
	{
	    iRlt = EXIT_ERROR;
	}
	OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"InActivePonOnuHotSwap, InActivePonPortIdx:%d, PassivePonPortIdx:%d, iRlt=%d.\r\n"
		, ActivePonPortIdx, PassivePonPortIdx, iRlt);
#else
    iRlt = -1;
#endif

    return iRlt;
}

int HotSwapRedundancyOnu(short int PonPortIdx, short int OnuIdx)
{
    return -1;
}
#endif
/* E--added by liwei056@2012-7-4 for Onu-OpticSwicthHover */

/* 将本端口设置为主用PON口*/
int SyncSwapPort( short int PonPortIdx )
{
	short int PonPortIdx_Swap;
	short int PonPortIdx_Master;
	short int PonPortIdx_Slave;
	int ret;

	ret = PonPortSwapPortQuery( PonPortIdx, &PonPortIdx_Swap);
	if( ret != ROK )  return ( V2R1_PORT_HAS_NO_HOT_SWAP_PORT );
	
    if ( OLT_ISLOCAL(PonPortIdx) )
    {
    	if( PonPortHotStatusQuery( PonPortIdx ) == V2R1_PON_PORT_SWAP_ACTIVE ) 
        {
            PonPortIdx_Master = PonPortIdx;
            PonPortIdx_Slave  = PonPortIdx_Swap;
        }
        else
        {
            PonPortIdx_Master = PonPortIdx_Swap;
            PonPortIdx_Slave  = PonPortIdx;
        }
    }
    else if ( OLT_ISLOCAL(PonPortIdx_Swap) )
    {
    	if( PonPortHotStatusQuery( PonPortIdx_Swap ) == V2R1_PON_PORT_SWAP_ACTIVE ) 
        {
            PonPortIdx_Master = PonPortIdx_Swap;
            PonPortIdx_Slave  = PonPortIdx;
        }
        else
        {
            PonPortIdx_Master = PonPortIdx;
            PonPortIdx_Slave  = PonPortIdx_Swap;
        }
    }
    else
    {
        VOS_ASSERT(0);
        return V2R1_SWAP_PORT_BUT_NOT_ACTIVE;
    }

    /* 覆盖拷贝，能保证完全一致 */
    CopyAllConfigFromPon1ToPon2(PonPortIdx_Master, PonPortIdx_Slave, OLT_COPYFLAGS_COVERSYNC);
	
	return( ROK );
}

/* 将本端口设置为主用PON口*/
int HotSwapPort( short int PonPortIdx1 )
{
	short int PonPortIdx ;
	short int PonPortIdx_Swap;
	int ret;

	PonPortIdx = PonPortIdx1;

	if( PonPortIsWorking( PonPortIdx1 ) != TRUE ) return( V2R1_SWAP_PORT_ISNOT_WORKING );

	ret = PonPortSwapPortQuery( PonPortIdx1, &PonPortIdx);
	if( ret != ROK )  return ( V2R1_PORT_HAS_NO_HOT_SWAP_PORT );
	
	PonPortIdx_Swap = PonPortIdx;
	/*if( PonPortIsWorking( PonPortIdx ) !=  TRUE ) return( RERROR );*/

    /*清除目标pon口的loid 配置表added by luh 2012-2-20*/
    /*DeletePonPortLoidEntryAll(PonPortIdx1);*/
    if ( OLT_ISLOCAL(PonPortIdx1) )
    {
    	if( PonPortHotStatusQuery( PonPortIdx1 ) == V2R1_PON_PORT_SWAP_ACTIVE ) 
    		return( ROK );
    }
    else
    {
    	if( PonPortHotStatusQuery( PonPortIdx_Swap ) == V2R1_PON_PORT_SWAP_PASSIVE ) 
    		return( ROK );
    }

	/* if( PonPortHotStatusQuery( PonPortIdx1 ) == V2R1_PON_PORT_SWAP_PASSIVE ) */
	{
#if 1
        if ( 0 != OLTAdv_ForceHotSwap(PonPortIdx1, PonPortIdx_Swap) )
        {
            return V2R1_SWAP_PORT_BUT_NOT_ACTIVE;
        }
#else    
/* B--added by liwei056@2010-1-26 for Pon-FastSwicthHover */
#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
    	int PonPort_SwapMode;
        
        PonPort_SwapMode = GetPonPortHotSwapMode(PonPortIdx1);
        if (V2R1_PON_PORT_SWAP_QUICKLY == PonPort_SwapMode)
        {
            HotSwapRedundancyPon(PonPortIdx_Swap, PonPortIdx1);  
        }
        else
#endif
/* E--added by liwei056@2010-1-25 for Pon-FastSwicthHover */
        {
    		if( PonPortIsWorking( PonPortIdx_Swap ) ==  TRUE )
    		{
    			PONTx_Disable( PonPortIdx_Swap );
    			/*ShutDown_olt( PonPortIdx_Swap );*/
    		}

#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
    		SetPonPortAutoProtectPassive( PonPortIdx_Swap);
#endif
    		PonPortTable[PonPortIdx_Swap].swap_timer = 0;
    		PonPortTable[PonPortIdx].swapHappened = FALSE;
    		SetPonPortSwapStatus( PonPortIdx_Swap, V2R1_PON_PORT_SWAP_PASSIVE );
    					
    		bcm_port_enable_set( 0, PonPortToSwPort[PonPortIdx_Swap], 0 );		

    		ClearAllOnuDataByOnePon(PonPortIdx1);
    		CopyAllOnuDataFromPon1ToPon2(PonPortIdx_Swap, PonPortIdx1);

    		/*SetPonPortMACLearning(PonPortIdx_Swap, V2R1_DISABLE);*/
    		VOS_TaskDelay(PON_HOT_SWAP_WAIT_TIME);

    		/*SetPonPortMACLearning(PonPortIdx1, V2R1_ENABLE);*/
    		ResetPonPortMacTable(PonPortIdx1);
    		if( PonPortIsWorking( PonPortIdx1 ) ==  TRUE )
    			{
    			/*StartUp_olt( PonPortIdx1);*/
#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
    			SetPonPortAutoProtectAcitve( PonPortIdx1);
#endif
    			PONTx_Enable( PonPortIdx1);
    			}
    		SetPonPortSwapStatus( PonPortIdx1, V2R1_PON_PORT_SWAP_ACTIVE);		
    		PonPortTable[PonPortIdx1].swap_timer = 0;
    		PonPortTable[PonPortIdx1].swapHappened = TRUE;

    		bcm_port_enable_set( 0, PonPortToSwPort[PonPortIdx1],  1 );

            /* send TRAP info to NMS */
    		Trap_PonAutoProtectSwitch(PonPortIdx1);
        }
#endif
	}
	
	return( ROK );
}

#if 0
int HotSwapPort1ToPort2( short int PonPortIdx1 , short int PonPortIdx2)
{
	short int PonPortIdx;

	PonPortIdx = PonPortIdx1;
	CHECK_PON_RANGE
	PonPortIdx = PonPortIdx2;
	CHECK_PON_RANGE

	if(( ThisIsHotSwapPort( PonPortIdx1, PonPortIdx2 )  != V2R1_IS_HOT_SWAP_PORT ) && ( ThisIsHotSwapPort( PonPortIdx1, PonPortIdx2 )  != V2R1_SWAP_PORT_BUT_NOT_ACTIVE ))
		return( V2R1_ISNOT_HOT_SWAP_PORT );

	if( PonPortIsWorking( PonPortIdx1 )  == TRUE )
		{
		PONTx_Disable( PonPortIdx1 );
		/*ShutDown_olt( PonPortIdx1 );*/
		}
#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
	SetPonPortAutoProtectPassive( PonPortIdx1);
#endif
	PonPortTable[PonPortIdx1].swap_timer = 0;
	PonPortTable[PonPortIdx1].swapHappened = FALSE;
	SetPonPortSwapStatus( PonPortIdx1, V2R1_PON_PORT_SWAP_PASSIVE );
				
	bcm_port_enable_set( 0, PonPortToSwPort[PonPortIdx1], 0 );	

	ClearAllOnuDataByOnePon(PonPortIdx2);
	CopyAllOnuDataFromPon1ToPon2(PonPortIdx1, PonPortIdx2);

	/*SetPonPortMACLearning(PonPortIdx1, V2R1_DISABLE);*/

	VOS_TaskDelay(PON_HOT_SWAP_WAIT_TIME);

	/*SetPonPortMACLearning(PonPortIdx2, V2R1_ENABLE);*/
	ResetPonPortMacTable(PonPortIdx2);
	if( PonPortIsWorking( PonPortIdx2 )  == TRUE )
		{
		/*StartUp_olt( PonPortIdx2 );*/
#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
		SetPonPortAutoProtectAcitve( PonPortIdx2);
#endif
		PONTx_Enable( PonPortIdx2);
		}
	SetPonPortSwapStatus( PonPortIdx2, V2R1_PON_PORT_SWAP_ACTIVE);
	PonPortTable[PonPortIdx2].swap_timer = 0;
	PonPortTable[PonPortIdx2].swapHappened = TRUE;	

	bcm_port_enable_set( 0, PonPortToSwPort[PonPortIdx2],  1 );
	
	/* send TRAP info to NMS */
	Trap_PonAutoProtectSwitch(PonPortIdx2);
	
	return( ROK );
}

int GetPonPortHappened( short int PonPortIdx )
{
	OLT_LOCAL_ASSERT(PonPortIdx);
	return( PonPortTable[PonPortIdx].swapHappened );
}
#endif

/* B--added by liwei056@2011-2-28 for 国电新倒换触发条件 */
static VOID HotSwapPortMain( ULONG ulArg1, ULONG ulArg2, ULONG ulArg3, ULONG ulArg4, ULONG ulArg5, ULONG ulArg6, ULONG ulArg7, ULONG ulArg8, ULONG ulArg9, ULONG ulArg10 )
{
    HotSwapPort((short int)ulArg1);
}

int AsyncHotSwapPort(short int PonPortIdx)
{
    ULONG ulArgv[10];
    CHAR  ucTaskName[20];
    
    VOS_Snprintf(ucTaskName, 18, "tSwap0x%x", PonPortIdx);

    ulArgv[0] = (ULONG)PonPortIdx;
    return (0 != VOS_TaskCreate(ucTaskName, TASK_PRIORITY_NORMAL, HotSwapPortMain, ulArgv)) ? 0 : OLT_ERR_NORESC;
}

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
int PonOpticalPowerAbnormal(short int PonPortIdx)
{
    int iRlt = OLT_ERR_NOTEXIST;    

	OLT_LOCAL_ASSERT(PonPortIdx);
    if( GetPonPortHotSwapTriggers(PonPortIdx) & PROTECT_SWITCH_TRIGGER_OPTICPOWER )
    {
        if ( PonPortIsWorking(PonPortIdx) )
        {
        	iRlt = sendPonSwitchEventMsg( PonPortIdx, PROTECT_SWITCH_EVENT_NOTIFY, PROTECT_SWITCH_REASON_OPTICPOWER, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, PROTECT_SWITCH_STATUS_PASSIVE, PonPortSwapTimesQuery(PonPortIdx) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
        }
    }

    return iRlt;
}

int PonOpticalPowerNormal(short int PonPortIdx)
{
    return OLT_ERR_NOTSUPPORT;
}
#endif

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
int PonOpticalDataAbnormal(short int PonPortIdx)
{
    int iRlt = OLT_ERR_NOTEXIST;    

	OLT_LOCAL_ASSERT(PonPortIdx);
    if( GetPonPortHotSwapTriggers(PonPortIdx) & PROTECT_SWITCH_TRIGGER_OPTICERROR )
    {
        if ( PonPortIsWorking(PonPortIdx) )
        {
        	iRlt = sendPonSwitchEventMsg( PonPortIdx, PROTECT_SWITCH_EVENT_NOTIFY, PROTECT_SWITCH_REASON_OPTICERROR, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, PROTECT_SWITCH_STATUS_PASSIVE, PonPortSwapTimesQuery(PonPortIdx) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
        }
    }

    return iRlt;
}

int PonOpticalDataNormal(short int PonPortIdx)
{
    return OLT_ERR_NOTSUPPORT;
}
#endif
/* E--added by liwei056@2011-2-28 for 国电新倒换触发条件 */

#if ( EPON_SUBMODULE_PON_SWAP_MONITOR == EPON_MODULE_YES )
int PonSlaveLooseAlarm(short int PonPortIdx, bool alarm_status)
{
    ULONG ulSlot, ulPort, ulAlarmID;

    ulAlarmID = alarm_status ? other_pon_slave_fail : other_pon_slave_ok;

    if ( other_pon_slave_ok == ulAlarmID )
    {
        if ( 0 == PonPortTable[PonPortIdx].swap_status )
        {
            return 0;
        }
        else
        {
            PonPortTable[PonPortIdx].swap_status = 0;
        }
    }
    else
    {
        if ( 0 != PonPortTable[PonPortIdx].swap_status )
        {
            return 0;
        }
        else
        {
            PonPortTable[PonPortIdx].swap_status = 1;
        }
    }

    ulSlot = GetCardIdxByPonChip(PonPortIdx);
    ulPort = GetPonPortByPonChip(PonPortIdx);
    
    return PonPortOther_EventReport(ulSlot, ulPort, ulAlarmID);
}
#endif

void ProcessHotSwap(short int olt_id, short int partner_olt_id, int reason)
{		
	int iRlt = 0;
	PonSwitchInfo_S switch_event;

	switch_event.olt_id       = olt_id;
	switch_event.new_status   = V2R1_PON_PORT_SWAP_PASSIVE;
	switch_event.event_id     = PROTECT_SWITCH_EVENT_START;
	switch_event.event_code   = reason;

	switch_event.slot_source  = SYS_LOCAL_MODULE_SLOTNO;
	switch_event.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
	switch_event.event_seq    = PonPortTable[olt_id].swap_times + 1;
	switch_event.event_flags  = PROTECT_SWITCH_EVENT_FLAGS_NONE;
	PonSwitchHandler(&switch_event);

	iRlt = OLT_SetHotSwapMode(olt_id, partner_olt_id, V2R1_PON_PORT_SWAP_SLOWLY, PROTECT_SWITCH_STATUS_PASSIVE, OLT_SWAP_FLAGS_ONLYSETTING);

	switch_event.olt_id       = partner_olt_id;
	switch_event.new_status   = V2R1_PON_PORT_SWAP_ACTIVE;
	switch_event.event_id     = PROTECT_SWITCH_EVENT_OVER;
	switch_event.event_code   = (0 == iRlt) ? PROTECT_SWITCH_RESULT_SUCCEED : PROTECT_SWITCH_RESULT_FAILED;

	switch_event.slot_source  = SYS_LOCAL_MODULE_SLOTNO;
	switch_event.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
	switch_event.event_seq    = PonPortTable[olt_id].swap_times + 1;
	switch_event.event_flags  = PROTECT_SWITCH_EVENT_FLAGS_NONE;
	PonSwitchHandler(&switch_event);

}

/* B--modified by liwei056@2010-4-7 for D9997 */
/* 倒换的定时监控机制 */
int ScanPonPortHotSwap()
{
    /* 倒换监控全由业务板负责 */
    if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER
        && (SYS_LOCAL_MODULE_RUNNINGSTATE == MODULE_RUNNING) )
    {
        int iRlt;
        int iSwapMode;
        int iSwapResult;
        short int PonPortIdx;
        short int PonPortIdx_Swap;
        short int PonPortBegin;
        short int PonPortEnd;
        
        if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER )
        {
            PonPortBegin = 0;
            PonPortEnd   = PONPORTPERCARD;
        }
        else
        {
            PonPortBegin = 0;
            PonPortEnd   = MAXPON;
        }
    
    	for( PonPortIdx = PonPortBegin; PonPortIdx < PonPortEnd; PonPortIdx ++ )
    	{			
    	    /* 倒换相关的PON口处理 */
			if(PonPortTable[PonPortIdx].swap_interval > 0)
			{
				PonPortTable[PonPortIdx].swap_interval--;
			}
            iSwapMode = GetPonPortHotSwapMode(PonPortIdx);
    		if( PON_SWAPMODE_ISNOTOLT(iSwapMode) ) continue; /* 仅OLT倒换模式才需定时监控 */
    		if( PonPortSwapPortQuery(PonPortIdx, &PonPortIdx_Swap) != ROK ) continue; /* 非倒换口，不监控 */
    		if( PonPortHotStatusQuery(PonPortIdx) != V2R1_PON_PORT_SWAP_ACTIVE ) continue; /* 只监控激活口 */
    		if( PonPortIsSwaping(PonPortIdx) ) continue; /* 正在倒换的，不监控 */
            
            /* --- 定时倒换的逻辑--- */
            if( TRUE != PonPortIsWorking(PonPortIdx) )
            {           
                if ( PONCHIP_DORMANT != PonChipMgmtTable[PonPortIdx].operStatus ) 
                {
                    if ( TRUE == OLTAdv_RdnIsReady(PonPortIdx_Swap) )
                    {
                        /* 当前主PON口不可用，而备用PON可用，立即倒换*/
                        PonSwitchInfo_S switch_event;

                        
#if ( EPON_SUBMODULE_PON_SWAP_MONITOR == EPON_MODULE_YES )
                        /* 备用PON口可用，清除监控报警 */
                        PonSlaveLooseAlarm(PonPortIdx, FALSE);
#endif

                        switch_event.olt_id       = PonPortIdx;
                        switch_event.new_status   = V2R1_PON_PORT_SWAP_PASSIVE;
                        switch_event.event_id     = PROTECT_SWITCH_EVENT_START;
                        switch_event.event_code   = PROTECT_SWITCH_REASON_OLTREMOVE;
                        switch_event.event_flags  = PROTECT_SWITCH_EVENT_FLAGS_NONE;

                        switch_event.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
                        switch_event.slot_source  = SYS_LOCAL_MODULE_SLOTNO;
                        switch_event.event_seq    = PonPortTable[PonPortIdx].swap_times + 1;
                        PonSwitchHandler(&switch_event);

                        /* 主PON口不可用 */
                        if ( iSwapMode >= V2R1_PON_PORT_SWAP_QUICKLY )
                        {
                            /* 快倒换要求2个PON都必须在位,否则只保存倒换配置 */
                            iSwapMode = V2R1_PON_PORT_SWAP_AUTO;
                        }

                        iRlt = OLT_SetHotSwapMode(PonPortIdx_Swap, PonPortIdx, iSwapMode, V2R1_PON_PORT_SWAP_ACTIVE, OLT_SWAP_FLAGS_ONLYSETTING);
                        OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"M_pon%d-(%c)->S_pon%d switch(for master loose workstatus) to M_pon%d<-(%c)->S_pon%d result(%d).\r\n", PonPortIdx, (V2R1_PON_PORT_SWAP_SLOWLY == iSwapMode) ? 's' : 'q', PonPortIdx_Swap, PonPortIdx_Swap, (V2R1_PON_PORT_SWAP_SLOWLY == iSwapMode) ? 's' : 'q', PonPortIdx, iRlt);
                        if ( 0 != iRlt )
                        {
                            iSwapResult = PROTECT_SWITCH_RESULT_FAILED;
                                    
                            /* 翻转设置失败，先Undo再Do */
                            if ( 0 == (iRlt = OLT_SetHotSwapMode(PonPortIdx_Swap, PonPortIdx, V2R1_PON_PORT_SWAP_DISABLED, V2R1_PON_PORT_SWAP_ACTIVE,  OLT_SWAP_FLAGS_OLT_SINGLESETTING | OLT_SWAP_FLAGS_ONLYSETTING)) )
                            {
                                iRlt = OLT_SetHotSwapMode(PonPortIdx_Swap, PonPortIdx, iSwapMode, V2R1_PON_PORT_SWAP_ACTIVE,  OLT_SWAP_FLAGS_OLT_SINGLESETTING | OLT_SWAP_FLAGS_ONLYSETTING);
                            }
                            OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"M_pon%d<-(%c)->S_pon%d re-setup(for master loose workstatus) result(%d).\r\n", PonPortIdx_Swap, (V2R1_PON_PORT_SWAP_SLOWLY == iSwapMode) ? 's' : 'q', PonPortIdx, iRlt);
                        }
                        else
                        {
                            iSwapResult = PROTECT_SWITCH_RESULT_SUCCEED;
                        }

                        switch_event.olt_id       = PonPortIdx_Swap;
                        switch_event.new_status   = V2R1_PON_PORT_SWAP_ACTIVE;
                        switch_event.event_id     = PROTECT_SWITCH_EVENT_OVER;
                        switch_event.event_code   = iSwapResult;
                        if ( 0 == iRlt )
                        {
                            switch_event.event_flags = PROTECT_SWITCH_EVENT_FLAGS_NONE;
                        }
                        else
                        {
                            switch_event.event_flags = PROTECT_SWITCH_EVENT_FLAGS_NEEDRESUME;
                        }

                        switch_event.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
                        switch_event.slot_source  = SYS_LOCAL_MODULE_SLOTNO;
                        switch_event.event_seq    = PonPortTable[PonPortIdx].swap_times + 1;
                        PonSwitchHandler(&switch_event);
                    }
#if ( EPON_SUBMODULE_PON_SWAP_MONITOR == EPON_MODULE_YES )
                    else
                    {
                        /* 备用PON口不可用，备用监控报警 */
                        PonSlaveLooseAlarm(PonPortIdx, TRUE);
                    }
#endif
                }
                else
                {
                    /* PON板未激活，就有了倒换配置，暂无定时处理 */
                    /* PON板进入RUNNING状态过快所致  */
                }
            } 
            else
            {
                /* 主PON口目前可用 */
                if ( TRUE == PonPortTable[PonPortIdx].swapHappened )
                {
                    /* 倒换后，有ONU注册恢复倒换设置 */
                    if ( PonPortTable[PonPortIdx].swap_timer++ > 1 )
                    {
                        int iSwapFlags;
                    
                        /* 延时倒换设置，防止倒换频繁反复 */   
                        PonPortTable[PonPortIdx].swap_timer = 0;
                        
                        /* 恢复新主口上的ONU注册信息及日志 */
                        OLT_ResumeAllOnuStatus(PonPortIdx, PON_ONU_DEREGISTRATION_REPORT_TIMEOUT, OLT_RESUMEMODE_SYNCHARD);

                        /* 恢复倒换设置，使得倒换对继续可用 */   
                        if ( TRUE == OLTAdv_RdnIsReady(PonPortIdx_Swap) )
                        {
                            iSwapFlags = OLT_SWAP_FLAGS_ONLYSETTING;
                        }
                        else
                        {
                            /* 备用PON口不可用 */
                            if ( iSwapMode >= V2R1_PON_PORT_SWAP_QUICKLY )
                            {
                                /* 快倒换要求2个PON都必须在位,否则只保存倒换配置 */
                                iSwapMode = V2R1_PON_PORT_SWAP_AUTO;
                            }
                            iSwapFlags = OLT_SWAP_FLAGS_OLT_SINGLESETTING | OLT_SWAP_FLAGS_ONLYSETTING;
                        }

                        iRlt = OLT_SetHotSwapMode(PonPortIdx, PonPortIdx_Swap, iSwapMode, V2R1_PON_PORT_SWAP_ACTIVE, iSwapFlags);
                        OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"M_pon%d-(q)->S_pon%d switch-setup(after switchover) to M-pon%d<-(q)->S-pon%d result(%d).\r\n", PonPortIdx_Swap, PonPortIdx, PonPortIdx, PonPortIdx_Swap, iRlt);
                        if ( 0 == iRlt )
                        {
                            /* 恢复倒换设置成功，标识可以再次倒换 */
                            PonPortTable[PonPortIdx].swapHappened = FALSE;
                        }
                        else
                        {
                            /* 恢复设置失败，先Undo再Do */
                            iRlt = OLT_SetHotSwapMode(PonPortIdx, PonPortIdx_Swap, V2R1_PON_PORT_SWAP_DISABLED, V2R1_PON_PORT_SWAP_ACTIVE, iSwapFlags);
                            OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"M_pon%d<-(q)->S_pon%d un-setup(after switchover set again) result(%d).\r\n", PonPortIdx, PonPortIdx_Swap, iRlt);
                        }
                    }
                }
                else
                {
                    if ( PONCHIP_UP == PonPortTable[PonPortIdx].PortWorkingStatus )
                    {
                        /* PON口正常工作状态的ONU为0判断光纤断开，才合理 */
                        if (HasOnuOnlineOrRegistering(PonPortIdx) == FALSE)
                        {                       
                        	/* 主PON口，一直没有ONU注册，则等待一定时间；若还是没有ONU 注册，则立即翻转倒换*/
							
                            if ( ++PonPortTable[PonPortIdx].swap_timer >= V2R1_AutoProtect_Timer && PonPortTable[PonPortIdx].swap_interval == 0)
                            {
                                /* 备用PON可用，立即倒换*/
                                if ( TRUE == OLTAdv_RdnIsReady(PonPortIdx_Swap) )
                                {
                                    PonSwitchInfo_S switch_event;
                                    /*unsigned short event_seq;
                                    unsigned short event_flags;*/

#if ( EPON_SUBMODULE_PON_SWAP_MONITOR == EPON_MODULE_YES )
                                    /* 备用PON口可用，清除监控报警 */
                                    PonSlaveLooseAlarm(PonPortIdx, FALSE);
#endif

                                    switch_event.olt_id       = PonPortIdx;
                                    switch_event.new_status   = V2R1_PON_PORT_SWAP_PASSIVE;
                                    switch_event.event_id     = PROTECT_SWITCH_EVENT_START;
                                    switch_event.event_code   = PROTECT_SWITCH_REASON_ONUNONE;
                                    switch_event.event_flags  = PROTECT_SWITCH_EVENT_FLAGS_NONE;

                                    switch_event.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
                                    switch_event.slot_source  = SYS_LOCAL_MODULE_SLOTNO;
                                    switch_event.event_seq    = PonPortTable[PonPortIdx].swap_times + 1;
                                    PonSwitchHandler(&switch_event);

                                    /* B--added by liwei056@2011-4-29 for PMC's RdnHardwareBug */
                                    if ( V2R1_PON_PORT_SWAP_QUICKLY == iSwapMode )
                                    {
                                        OLTAdv_SetOpticalTxMode2(PonPortIdx, 1, PONPORT_TX_SWITCH);
                                        OLTAdv_SetOpticalTxMode2(PonPortIdx_Swap, 1, PONPORT_TX_SWITCH);
                                    }
                                    /* E--added by liwei056@2011-4-29 for PMC's RdnHardwareBug */
                                    iRlt = OLT_SetHotSwapMode(PonPortIdx, PonPortIdx_Swap, iSwapMode, V2R1_PON_PORT_SWAP_PASSIVE, OLT_SWAP_FLAGS_ONLYSETTING);
                                    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"M_pon%d-(%c)->S_pon%d switch(for no any onu) to M_pon%d<-(%c)->S_pon%d result(%d).\r\n", PonPortIdx, (V2R1_PON_PORT_SWAP_SLOWLY == iSwapMode) ? 's' : 'q', PonPortIdx_Swap, PonPortIdx_Swap, (V2R1_PON_PORT_SWAP_SLOWLY == iSwapMode) ? 's' : 'q', PonPortIdx, iRlt);
                                    if ( 0 != iRlt )
                                    {
                                        iSwapResult = PROTECT_SWITCH_RESULT_FAILED;
                                        
                                        /* 翻转设置失败，先Undo再Do */
                                        if ( 0 == (iRlt = OLT_SetHotSwapMode(PonPortIdx, PonPortIdx_Swap, V2R1_PON_PORT_SWAP_DISABLED, V2R1_PON_PORT_SWAP_PASSIVE, OLT_SWAP_FLAGS_ONLYSETTING)) )
                                        {
                                            iRlt = OLT_SetHotSwapMode(PonPortIdx, PonPortIdx_Swap, iSwapMode, V2R1_PON_PORT_SWAP_PASSIVE, OLT_SWAP_FLAGS_ONLYSETTING);
                                        }
                                        OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"M_pon%d<-(%c)->S_pon%d re-setup(for no any onu) result(%d).\r\n", PonPortIdx_Swap, (V2R1_PON_PORT_SWAP_SLOWLY == iSwapMode) ? 's' : 'q', PonPortIdx, iRlt);
                                    }
                                    else
                                    {
                                        iSwapResult = PROTECT_SWITCH_RESULT_SUCCEED;
                                    }

                                    switch_event.olt_id       = PonPortIdx_Swap;
                                    switch_event.new_status   = V2R1_PON_PORT_SWAP_ACTIVE;
                                    switch_event.event_id     = PROTECT_SWITCH_EVENT_OVER;
                                    switch_event.event_code   = iSwapResult;
                                    if ( 0 == iRlt )
                                    {
                                        switch_event.event_flags = PROTECT_SWITCH_EVENT_FLAGS_NONE;
                                    }
                                    else
                                    {
                                        switch_event.event_flags = PROTECT_SWITCH_EVENT_FLAGS_NEEDRESUME;
                                    }

                                    switch_event.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
                                    switch_event.slot_source  = SYS_LOCAL_MODULE_SLOTNO;
                                    switch_event.event_seq    = PonPortTable[PonPortIdx].swap_times + 1;
                                    PonSwitchHandler(&switch_event);
                                }
#if ( EPON_SUBMODULE_PON_SWAP_MONITOR == EPON_MODULE_YES )
                                else
                                {
									OLT_SWITCH_DEBUG("\r\n ponport %d is working!\r\n", PonPortIdx);
									OLT_SWITCH_DEBUG("\r\n swap ponport %d is not ready!\r\n", PonPortIdx_Swap);
                                    /* 备用PON口不可用，备用监控报警 */
                                    PonSlaveLooseAlarm(PonPortIdx, TRUE);
                                }
#endif
                            }
                        }
                        else
                        {
                            /* 复位此PON口的无ONU累计器 */
                            PonPortTable[PonPortIdx].swap_timer = 0;
                        }
                    }
                }
            }
    	}
    }

	return( ROK );
}
/* E--modified by liwei056@2010-4-7 for D9997 */

/* B--added by liwei056@2010-1-26 for Pon-FastSwicthHover */
int ActivePonHotSwapPort(short int PonPortIdx1, short int PonPortIdx2, int iProtectMode, int iSyncMode)
{
    int iRlt;

    iRlt = OLTAdv_SetHotSwapMode(PonPortIdx1, PonPortIdx2, iProtectMode, V2R1_PON_PORT_SWAP_UNKNOWN, iSyncMode);
    if ( OLT_CALL_ISERROR(iRlt) )
    {
        switch (iRlt)
        {
            case OLT_ERR_NOTSUPPORT:
                iRlt = V2R1_SWAP_PORT_ISUNSUPPORTED;
                break;
            case OLT_ERR_NOTEXIST:
                iRlt = V2R1_SWAP_PORT_ISNOT_WORKING;
                break;
            default:    
                iRlt = V2R1_SWAP_PORT_MODE_REPEAT;
        }
    }   

    return iRlt;
}
/* E--added by liwei056@2010-1-26 for Pon-FastSwicthHover */

void BindOnuConfOnSmallPonPort(short int PonPortIdx1, short int PonPortIdx2, int iProtectMode)
{
	/*ONU配置在较小PON ID上绑定       added by wangxiaoyu 2011-08-05*/
	/*added by wangxiaoyu 2011-11-22
	  添加PON端口本地值判断，暂时不支持设备间的配置文件PON保护倒换
	  */
	/*for onu swap by jinhl@2013-02-22*/
	short int active_pon = 0;
	short int small_pon = min(PonPortIdx1, PonPortIdx2);
	short int big_pon = max(PonPortIdx1, PonPortIdx2);

	short int ulsslot = 0, uldslot=0, ulsport = 0, uldport=0;

	if(V2R1_PON_PORT_SWAP_ONU == iProtectMode)
	{
	}
	else
	{
		if(OLT_ISLOCAL(PonPortIdx1) && OLT_ISLOCAL(PonPortIdx2))
		{

			if(PonPortHotStatusQuery(PonPortIdx1) == V2R1_PON_PORT_SWAP_ACTIVE)
				active_pon = PonPortIdx1;
			else
				active_pon = PonPortIdx2;

			if(active_pon != small_pon)
			{
				ulsslot = GetCardIdxByPonChip(active_pon);
				uldslot = GetCardIdxByPonChip(small_pon);
				ulsport = GetPonPortByPonChip(active_pon);
				uldport = GetPonPortByPonChip(small_pon);

			}
			else
			{
				ulsslot = GetCardIdxByPonChip(active_pon);
				uldslot = GetCardIdxByPonChip(big_pon);
				ulsport = GetPonPortByPonChip(active_pon);
				uldport = GetPonPortByPonChip(big_pon);
			}

			onuconfSwitchPonPort(ulsslot, ulsport, uldslot, uldport);
			OLT_SWITCH_DEBUG("\r\onuconfSwitchPonPort(%d, %d, %d, %d)\r\n", ulsslot, ulsport, uldslot, uldport);

			startOnuConfSyndReqByCard(ulsslot);
			/*同一板卡的pon保护，不需要发送2次。Q.25437*/
			if(uldslot != ulsslot)
				startOnuConfSyndReqByCard(uldslot);
			onuconf_syncStandbyMaster(NULL);
		}
	}
	return;
}

	

int  EnablePonPortHotSwapPort(short int PonPortIdx1, short int PonPortIdx2, int iProtectMode)
{
    int ret = ActivePonHotSwapPort(PonPortIdx1, PonPortIdx2, iProtectMode, V2R1_PON_PORT_SWAP_SYNC);
    if(!ret)
    {
		BindOnuConfOnSmallPonPort(PonPortIdx1, PonPortIdx2, iProtectMode);
	}
    

    return ret;

}

int  DisablePonPortHotSwap(short int PonPortIdx1 )
{
	short int PonPortIdx ;
	short int PonPortIdx_Swap;
	short int src_port, dst_port;
	int ret;
	int iSwapMode = 0;

	ret = PonPortSwapPortQuery( PonPortIdx1, &PonPortIdx_Swap);
	if( ret != ROK )  return ( V2R1_PORT_HAS_NO_HOT_SWAP_PORT );
	/*Begin:for onu swap by jinhl@2013-02-22*/
	
	iSwapMode = GetPonPortHotSwapMode(PonPortIdx1);;
	if(V2R1_PON_PORT_SWAP_ONU == iSwapMode)
	{
	    
		if(HasOnuOnlineOrRegistering(PonPortIdx1) || HasOnuOnlineOrRegistering(PonPortIdx_Swap))
		{
			return V2R1_SWAP_PORT_HASONU;
		}
	}
	/*End:for onu swap by jinhl@2013-02-22*/
	if(PonPortHotStatusQuery(PonPortIdx1) == V2R1_PON_PORT_SWAP_ACTIVE)
	{
	    dst_port = PonPortIdx1;
	    src_port = PonPortIdx_Swap;
	}
	else
	{
	    dst_port = PonPortIdx_Swap;
	    src_port = PonPortIdx1;
	}

#if 1
    if ( OLT_CALL_ISERROR( OLTAdv_SetHotSwapMode(PonPortIdx1, PonPortIdx_Swap, V2R1_PON_PORT_SWAP_DISABLED, V2R1_PON_PORT_SWAP_UNKNOWN, V2R1_PON_PORT_SWAP_NOSYNC) ) )
    {
        return RERROR;
    }


	/*added by wangxiaoyu 2011-11-22
	添加PON端口本地值判断，暂时不支持设备间的配置文件PON保护倒换
	*/
	if(V2R1_PON_PORT_SWAP_ONU == iSwapMode)
	{/*for onu swap by jinhl@2013-02-22*/
	}
	else
	{
		if(OLT_ISLOCAL(dst_port) && OLT_ISLOCAL(src_port))
		{
	    /*解除ONU配置在较小PON ID上的绑定，恢复到当前PON端口上
	     *added by wangxiaoyu 2011-08-05*/
	    if (dst_port > src_port)
	    {
	        short int ulsslot = GetCardIdxByPonChip(src_port);
	        short int ulsport = GetPonPortByPonChip(src_port);
	        short int uldslot = GetCardIdxByPonChip(dst_port);
	        short int uldport = GetPonPortByPonChip(dst_port);

	        onuconfSwitchPonPort(ulsslot, ulsport, uldslot, uldport);

	        ret = startOnuConfSyndReqByCard(uldslot);
	        
	        /*同一板卡的pon保护，不需要发送2次。Q.25437*/
            if(uldslot != ulsslot)	        
    	        ret |= startOnuConfSyndReqByCard(ulsslot);

	    }
	    else
	    {
	        short int slot1 = GetCardIdxByPonChip(dst_port);
	        short int slot2 = GetCardIdxByPonChip(src_port);
	        ret = startOnuConfSyndReqByCard(slot1);
	        
	        /*同一板卡的pon保护，不需要发送2次。Q.25437*/
	        if(slot1 != slot2)
    	        ret |= startOnuConfSyndReqByCard(slot2);
	    }
	    onuconf_syncStandbyMaster(NULL);
		}

	}
#else
    /* B--added by liwei056@2010-1-26 for Pon-FastSwicthHover */
#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
    if ( V2R1_PON_PORT_SWAP_QUICKLY == GetPonPortHotSwapMode(PonPortIdx1) )
    {
        (void)InActivePonFastHotSwapSetup(PonPortIdx1, PonPortIdx_Swap);
    }
#endif
    /* E--added by liwei056@2010-1-26 for Pon-FastSwicthHover */

	PonPortIdx = RERROR ;
	if( PonPortHotStatusQuery(PonPortIdx1) == V2R1_PON_PORT_SWAP_ACTIVE )
		PonPortIdx = PonPortIdx1;
	else if( PonPortHotStatusQuery( PonPortIdx_Swap ) == V2R1_PON_PORT_SWAP_ACTIVE )
		PonPortIdx = PonPortIdx_Swap;
	
	if( PonPortIdx != RERROR )
		{
		/*CHECK_PON_RANGE*/
		ClearHotSwapActivePonRunningData(PonPortIdx);
		}

	PonPortIdx = RERROR ;
	if( PonPortHotStatusQuery( PonPortIdx1) == V2R1_PON_PORT_SWAP_PASSIVE )
		{
		ClearHotSwapPassivePonRunningData(PonPortIdx1);
		PonPortIdx = PonPortIdx1;
		}
	if( PonPortHotStatusQuery( PonPortIdx_Swap ) == V2R1_PON_PORT_SWAP_PASSIVE )
		{
		ClearHotSwapPassivePonRunningData(PonPortIdx_Swap);
		PonPortIdx = PonPortIdx_Swap;
		}
	/*
	if( PonPortIdx != RERROR )
		{
		CHECK_PON_RANGE
		ClearHotSwapPassivePonRunningData(PonPortIdx);
		}
	else {
		ClearHotSwapPassivePonRunningData(PonPortIdx1);
		ClearHotSwapPassivePonRunningData(PonPortIdx_Swap);
		}
	*/
	/* PON端口取消保护倒换后，清除备用PON端口下的配置数据*/
	{
	/*unsigned int  WorkingStatus, AdminStatus;
	extern short int monPonFerAlmEnGet(unsigned short oltId, unsigned int *pFerAlmEn);
	extern short int monPonFerAlmEnSet(unsigned short oltId, unsigned int FerAlmEn);*/
	if( PonPortIdx != RERROR )
		ClearAllOnuDataByOnePon( PonPortIdx );

	/*
	WorkingStatus = PonPortTable[PonPortIdx].PortWorkingStatus;
	AdminStatus = PonPortTable[PonPortIdx].PortAdminStatus;
	InitPonPortTableByDefault( PonPortIdx);
	PonPortTable[PonPortIdx].PortAdminStatus = AdminStatus;
	PonPortTable[PonPortIdx].PortWorkingStatus = WorkingStatus;
	*/
	}
#endif
	return( ROK );
}


int  EnablePonPortAutoProtect( unsigned int slot, unsigned int port, unsigned int PartnerSlot, unsigned int PartnerPort, int iProtectMode )
{
	short int PonPortIdx1, PartnerPonPortIdx, PonPortIdx;
	int ret, ret1;

    if ( V2R1_ENABLE != pon_swap_switch_enable )
    {
        return V2R1_SWAP_PORT_ISDISABLED;
    }

	PonPortIdx1 = GetPonPortIdxBySlot( (short int)slot, (short int)port);
	PartnerPonPortIdx = GetPonPortIdxBySlot( (short int)PartnerSlot, (short int)PartnerPort);

	if( PonPortIdx1 == PartnerPonPortIdx ) return( V2R1_SWAP_PORT_ALL_ONE );

	PonPortIdx = PonPortIdx1;
	CHECK_PON_RANGE

#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
#else
	PonPortIdx = PartnerPonPortIdx;
	CHECK_PON_RANGE
#endif

    if ( OLT_ISLOCAL(PonPortIdx1) )
    {
    }
    else if ( OLT_ISLOCAL(PartnerPonPortIdx) )
    {
        /* 通过参数换位，来确保本地Olt走优化处理  */
        PonPortIdx        = PonPortIdx1;
        PonPortIdx1       = PartnerPonPortIdx;
        PartnerPonPortIdx = PonPortIdx;
    }
    else
    {
        /* 保护倒换的2个PON口至少有一个设备本地的  */
        return( V2R1_SWAP_PORT_ALL_REMOTE );
    }

	ret1 = ThisIsHotSwapPort( PonPortIdx1, PartnerPonPortIdx );

	if(  (ret1 == V2R1_IS_HOT_SWAP_PORT)
    /* B--Added by liwei056@2009-9-3 for D8733 */
        || (ret1 == V2R1_SWAP_PORT_BUT_NOT_ACTIVE) )
    /* E--Added by liwei056@2009-9-3 for D8733 */
    {
        return( V2R1_IS_HOT_SWAP_PORT );
    }   
	if( ret1 == RERROR ) return( RERROR );

	if( ret1 != V2R1_SWAP_PORT_BUT_NOT_ACTIVE )
	{
		/* if( PonPortSwapEnableQuery(PonPortIdx1) == V2R1_PON_PORT_SWAP_ENABLE ) */
		{
			ret = PonPortSwapPortQuery( PonPortIdx1, &PonPortIdx );
			if(( ret == ROK ) && (PonPortIdx != PartnerPonPortIdx ))			
				return( V2R1_PORT1_HAS_OTHER_HOT_SWAP_PORT );
		}

		/* if( PonPortSwapEnableQuery(PartnerPonPortIdx) == V2R1_PON_PORT_SWAP_ENABLE ) */
		{
			ret = PonPortSwapPortQuery( PartnerPonPortIdx, &PonPortIdx );
			if(( ret == ROK )&& (PonPortIdx != PonPortIdx1 ))			
				return( V2R1_PORT2_HAS_OTHER_HOT_SWAP_PORT );
		}
	}

    {
        /* remmed by liwei056@2011-1-28 for RdnInBoard's CfgSync2Master */
        if ( PRODUCT_IS_DISTRIBUTE )
        {
            if ( 0 == g_ulRPCCall_mode )
            {
#if 1
                OLT_SetHotSwapParam(-1, -1, -1, 1, -1, -1, -1);
#else
                return V2R1_SWAP_RPCMODE_NOTSUPPORT;
#endif
            }
        }
    }

     /*for onu swap by jinhl@2013-04-27*/
	if((V2R1_PON_PORT_SWAP_ONU == iProtectMode) &&
		(HasOnuOnlineOrRegistering(PonPortIdx1) || HasOnuOnlineOrRegistering(PartnerPonPortIdx)))
	{
	    return V2R1_SWAP_PORT_HASONU;
	}
#if 1
    if ( 0 != (ret = EnablePonPortHotSwapPort(PonPortIdx1, PartnerPonPortIdx, iProtectMode)) )
    {
        return ret;
    }
#else
	PonPortTable[PonPortIdx1].swap_slot = PartnerSlot;
	PonPortTable[PonPortIdx1].swap_port = PartnerPort;
	PonPortTable[PartnerPonPortIdx].swap_slot = slot;
	PonPortTable[PartnerPonPortIdx].swap_port = port;

	/*  deleted by chenfj 2008-12-22
	   在系统启动PON 激活时,  这个宏定义在6100 返回值为false, 但在6700 上返回值
	   为ture, 不一致; 省略吧
	   */
	if( SYS_LOCAL_MODULE_ISMASTERACTIVE ) 
    {   
        if ( 0 != (ret = EnablePonPortHotSwapPort(PonPortIdx1, PartnerPonPortIdx, iProtectMode)) )
        {
            return ret;
        }
        
		SetPonPortSwapEnable( PonPortIdx1, V2R1_PON_PORT_SWAP_ENABLE );
		SetPonPortSwapEnable( PartnerPonPortIdx, V2R1_PON_PORT_SWAP_ENABLE );
    }
	else
    {
		SetPonPortSwapStatus( PonPortIdx1, V2R1_PON_PORT_SWAP_PASSIVE );
		SetPonPortSwapStatus( PartnerPonPortIdx, V2R1_PON_PORT_SWAP_PASSIVE );

    	PonPortTable[PonPortIdx1].swap_mode       = iProtectMode;
    	PonPortTable[PartnerPonPortIdx].swap_mode = iProtectMode;

		SetPonPortSwapEnable( PonPortIdx1, V2R1_PON_PORT_SWAP_ENABLE );
		SetPonPortSwapEnable( PartnerPonPortIdx, V2R1_PON_PORT_SWAP_ENABLE );
	}
#endif
    
	return( ROK );
}

int DisablePonPortAutoProtect( unsigned int slot, unsigned int port )
{
	short int PonPortIdx;

	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
	return( DisablePonPortHotSwap(PonPortIdx));
}

int  ForcePonPortAutoProtect( unsigned int slot, unsigned int port )
{
    short int PonPortIdx;

    if ( V2R1_ENABLE == pon_swap_switch_enable )
    {
    	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
        return HotSwapPort( PonPortIdx );
    }

	return V2R1_SWAP_PORT_ISDISABLED;	
}

int  SyncPonPortAutoProtect( unsigned int slot, unsigned int port )
{
    short int PonPortIdx;

    if ( V2R1_ENABLE == pon_swap_switch_enable )
    {
    	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
        return SyncSwapPort( PonPortIdx );
    }

	return V2R1_SWAP_PORT_ISDISABLED;	
}

int  PonPortAutoProtectEnableQuery(unsigned int slot, unsigned int port )
{
	short int PonPortIdx;

	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);

	return( PonPortSwapEnableQuery( PonPortIdx) );

}

int  PonPortAutoProtectStatusQuery(unsigned int slot, unsigned int port )
{
	short int PonPortIdx;

	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);

	return( PonPortHotStatusQuery( PonPortIdx) );

}

int ThisIsAutoProtectPort(unsigned int slot, unsigned int port, unsigned int PartnerSlot, unsigned int PartnerPort )
{
	short int PortPortIdx, PartnerPorPortIdx;

	PortPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
	PartnerPorPortIdx = GetPonPortIdxBySlot((short int)PartnerSlot, (short int)PartnerPort);
	
	return( ThisIsHotSwapPort( PortPortIdx, PartnerPorPortIdx ));
}


int  PonPortAutoProtectPortQuery( unsigned int slot, unsigned int port, unsigned int *PartnerSlot, unsigned int *PartnerPort )
{
	short int PonPortIdx, PartnerPonPortIdx;
	int ret;
	
	if( PartnerSlot == NULL ) return( RERROR );
	if( PartnerPort == NULL ) return( RERROR );

	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);

	ret =  PonPortSwapPortQuery( PonPortIdx, &PartnerPonPortIdx );

	if( ret != ROK ) return( ret );	

	*PartnerSlot = PonPortTable[PonPortIdx].swap_slot;  /*GetCardIdxByPonChip( PartnerPonPortIdx );*/
	*PartnerPort = PonPortTable[PonPortIdx].swap_port; /*GetPonPortByPonChip( PartnerPonPortIdx );*/

	return( ROK );

}

int  EnablePonPortAutoProtectByOnePort( short int PonPortIdx )
{
	int ret;
	int iProtectMode;
	int iProtectMode2;
	int iProtectStatus;
	int iSyncMode;
	int iProtectTriggers;
	short int PonPortIdx_Partner;
	short int PonPortIdx_Partner2;
	unsigned long ulPartnerSlot, ulPartnerPort;

    iProtectStatus = V2R1_PON_PORT_SWAP_UNKNOWN;
#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_UPLINKDOWN == EPON_MODULE_YES )
    iProtectTriggers = GetPonPortHotSwapTriggers(PonPortIdx);
    if( iProtectTriggers & PROTECT_SWITCH_TRIGGER_UPLINKDOWN )
    {
        if ( ROK == GetPonPortHotProtectedPort(PonPortIdx, &ulPartnerSlot, &ulPartnerPort) )
        {
            unsigned long ulIfindex;
            unsigned long ulPortIsLinkUp;

            if ( VOS_OK == olt_eth_idx_check(ulPartnerSlot, ulPartnerPort) )
            {
                ulIfindex = userSlot_userPort_2_Ifindex(ulPartnerSlot, ulPartnerPort);
                if ( VOS_OK == IFM_GetIfStatusApi(ulIfindex, &ulPortIsLinkUp) )
                {
                    if ( ulPortIsLinkUp )
                    {
                        iProtectStatus = V2R1_PON_PORT_SWAP_ACTIVE;
                    }
                    else
                    {
                        iProtectStatus = V2R1_PON_PORT_SWAP_PASSIVE;
                    }
                }
            }
            else
            {
                iProtectStatus = V2R1_PON_PORT_SWAP_PASSIVE;
            }
        }
    }
#endif

    ret = PonPortSwapPortQuery(PonPortIdx, &PonPortIdx_Partner);
	if( ret != ROK )
    {
        /* B--added by liwei056@2012-3-12 for UplinkPonMonitorSwitch */
#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_UPLINKDOWN == EPON_MODULE_YES )
        if ( V2R1_PON_PORT_SWAP_PASSIVE == iProtectStatus )
        {
            return( ROK );
        }
        else
#endif            
        /* E--added by liwei056@2012-3-12 for UplinkPonMonitorSwitch */
        {
            return( RERROR );
        }
    }   

	if( PonPortIdx_Partner == RERROR ) return( RERROR );
    
    /* B--added by liwei056@2011-1-10 for CodeCheck */
    if ( !PonPortIsWorking(PonPortIdx_Partner) )
    {
        /* B--added by liwei056@2011-12-15 for D14069 */
        ret = OLTAdv_GetHotSwapMode(PonPortIdx, &PonPortIdx_Partner2, &iProtectMode2, &iProtectStatus);
        iProtectMode = GetPonPortHotSwapMode(PonPortIdx);
		/*for onu swap by jinhl@2013-04-27*/
		/*此处若为onu倒换，其中一个pon口不存在，但也需要继续配置*/
        if ( OLT_CALL_ISOK(ret)&& (V2R1_PON_PORT_SWAP_ONU != iProtectMode) )
        {
            if ( iProtectMode2 > V2R1_PON_PORT_SWAP_DISABLED )
            {
                if ( (PonPortIdx_Partner == PonPortIdx_Partner2) && (iProtectMode == iProtectMode2) )
                {
                    /* 底层保护对已经激活 */
                    return ROK;
                }
            }
        }
        /* E--added by liwei056@2011-12-15 for D14069 */
        /*for onu swap by jinhl@2013-02-22*/
		/*在onu保护模式下，设置完底层之前不使其发光*/
		if(V2R1_PON_PORT_SWAP_ONU != iProtectMode)
		{
	        /* 保护对只有一个PON口正常时，先令此口发光正常工作[使得单保护口可以正常工作] */
	        ret = PONTx_Enable(PonPortIdx, PONPORT_TX_ALL);
	        OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"pon%d/%d start tx(%d) for pon%d/%d not work.\r\n"
	                , GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx)
	                , ret
	                , GetCardIdxByPonChip(PonPortIdx_Partner), GetPonPortByPonChip(PonPortIdx_Partner));
		}
        
        iSyncMode = V2R1_PON_PORT_SWAP_NOSYNC;
    }
    else
    {
        /* 2个PON口都就绪，必须重新配置一次倒换，来彻底恢复PAS_SOFT的倒换状态 */
        iSyncMode = V2R1_PON_PORT_SWAP_SYNC;
        iProtectMode = GetPonPortHotSwapMode(PonPortIdx);
    }
    /* E--added by liwei056@2011-1-10 for CodeCheck */
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"EnablePonPortAutoProtectByOnePort PonPortIdx:%d, PonPortIdx_Partner:%d, iProtectMode:%d.\r\n", PonPortIdx, PonPortIdx_Partner, iProtectMode);
    /* 激活倒换设置 */
    ret = OLTAdv_SetHotSwapMode(PonPortIdx, PonPortIdx_Partner, iProtectMode, iProtectStatus, iSyncMode);
	/*added by wangjiah@2017-03-08:begin
	** to restore onu-config file when pon port or olt reset/restart */
	if(ret == ROK)
	{
		BindOnuConfOnSmallPonPort(PonPortIdx, PonPortIdx_Partner, iProtectMode);	
	}
	/*added by wangjiah@2017-03-08:end*/
	return ret;
}

/*Begin:for onu swap by jinhl@2013-04-27*/
/*在pon口激活时，判断若对端pon已经有onu注册，将其对应
注册信息虚注册到本地*/
int ActivePonPort_RegVirtual(short int PonPortIdx)
{
    int ret = 0;
	short int PonPortIdx_swap;
	int i = 0;
    int OnuEntry = 0;
	int status = 0;
	int llid = 0;
    int slot_swap = 0;
	int port_swap = 0;
	short int PonChipVer;
	unsigned char *MacAddr;
	
	ret = PonPortSwapPortQuery(PonPortIdx, &PonPortIdx_swap);
	if((VOS_OK != ret) ||  (RERROR == PonPortIdx_swap ))
	{
	    return ret;
	}

	if((V2R1_PON_PORT_SWAP_ONU != GetPonPortHotSwapMode(PonPortIdx)) ||
		(!PonPortIsWorking(PonPortIdx_swap)))
	{
	    return ret;
	}

	

	for(i = 0; i < MAXONUPERPON; i++)
	{
	    OnuEntry = PonPortIdx_swap * MAXONUPERPON + i;
		ONU_MGMT_SEM_TAKE;
		status = OnuMgmtTable[OnuEntry].OperStatus;
		llid = OnuMgmtTable[OnuEntry].LLID;
		MacAddr = OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr;
		ONU_MGMT_SEM_GIVE;
		if( (ONU_OPER_STATUS_UP == status) && 
			(ONU_PROTECT_TYPE_C == GetOnuProtectType( PonPortIdx_swap, i )))
		{
		    
            unsigned long aulMsg[4] = { MODULE_OLT, FC_ONUSWAP_PARTNER_REG, 0, 0 };
			OnuEventData_s *OnuRegData = NULL;
			
			OnuRegData= (OnuEventData_s *)VOS_Malloc( sizeof(OnuEventData_s), MODULE_OLT);
			if( OnuRegData == NULL )
			{
				sys_console_printf("Error, Malloc buffer not satified %s %d\r\n",__FILE__,__LINE__);
				VOS_ASSERT( 0 );
				return( RERROR );
			}
			OnuRegData->llid = llid;
			OnuRegData->OnuIdx = i;
			OnuRegData->PonPortIdx = PonPortIdx_swap;
			VOS_MemCpy((void *)OnuRegData->onu_mac,(void *)MacAddr, BYTES_IN_MAC_ADDRESS);
			
			aulMsg[2] = sizeof(OnuEventData_s);
			aulMsg[3] = (int)OnuRegData;

            ret = VOS_QueSend( g_Olt_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
            if( ret !=  VOS_OK )
            {
                VOS_ASSERT( 0 );
				VOS_Free((void *)OnuRegData);
            }  
			sys_console_printf("send activeponport reg virtual pon%d llid%d\r\n",PonPortIdx_swap,llid);

		}
		
	}
}
/*End:for onu swap by jinhl@2013-04-27*/
/* added by chenfj 2007-7-26 */
/*  PON端口保护倒换，用于两个PON 端口下数据同步 */


/* 清除PON端口下所有ONU数据*/
int  ClearAllOnuDataByOnePon(short int PonPortIdx)
{
	short int OnuIdx;

	OLT_LOCAL_ASSERT(PonPortIdx);

	DeletePonPortAuthEntryAll( PonPortIdx);

	for( OnuIdx=0; OnuIdx < MAXONUPERPON; OnuIdx ++)
	{
		/*if(ThisIsValidOnu( PonPortIdx, OnuIdx) == ROK )
		{
			ONU_MGMT_SEM_TAKE;
			setOnuStatus( GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),OnuIdx, ONU_OFFLINE );
			ONU_MGMT_SEM_GIVE;
		}*/	
		InitOneOnuByDefault( PonPortIdx, OnuIdx );
	}	

	return( ROK );
}

#if 1
/* 复制一个PON端口下的数据到另一个PON端口*/
int  CopyPonConfigDataFromPon1ToPon2( short int PonPortIdx1, short int PonPortIdx2)
{
	short int PonPortIdx/*, i*/;

	PonPortIdx=PonPortIdx1;
	CHECK_PON_RANGE
	PonPortIdx = PonPortIdx2;
	CHECK_PON_RANGE


	/* added by chenfj 2009-5-14 , 当这个PON 口下有多于 64个ONU 时，产生告警*/
	PonPortTable[PonPortIdx2].PortFullAlarm = PonPortTable[PonPortIdx1].PortFullAlarm;

	PonPortTable[PonPortIdx2].AlarmMask  = PonPortTable[PonPortIdx1].AlarmMask;

	/* PON 芯片告警使能*/
	VOS_MemCpy( &PonPortTable[PonPortIdx2].AlarmConfigInfo, &PonPortTable[PonPortIdx1].AlarmConfigInfo, sizeof( PON_olt_monitoring_configuration_t ) );
	/*SetPonAlarmConfig( PonPortIdx2 );*/
	
	/* MAC 地址老化时间*/
	if( PonPortTable[PonPortIdx2].MACAgeingTime != PonPortTable[PonPortIdx1].MACAgeingTime )
		{
		/*sys_console_printf(" mac aging time1 %d, 2 %d\r\n", PonPortTable[PonPortIdx1].MACAgeingTime, PonPortTable[PonPortIdx2].MACAgeingTime );*/	
		PonPortTable[PonPortIdx2].MACAgeingTime = PonPortTable[PonPortIdx1].MACAgeingTime;
		SetPonPortMacAgeingTime( PonPortIdx2, PonPortTable[PonPortIdx2].MACAgeingTime);
		}

	/* onu 注册窗口时长设置*/
	if( PonPortTable[PonPortIdx2].range != PonPortTable[PonPortIdx1].range)
		{
		/*sys_console_printf(" mac reg windown 1 %d, 2 %d\r\n", PonPortTable[PonPortIdx1].range, PonPortTable[PonPortIdx2].range);*/
		PonPortTable[PonPortIdx2].range = PonPortTable[PonPortIdx1].range;
		SetPonRange(PonPortIdx2, PonPortTable[PonPortIdx2].range);
		}

	if((PonPortTable[PonPortIdx2].discard_unlearned_sa != PonPortTable[PonPortIdx1].discard_unlearned_sa )
		||(PonPortTable[PonPortIdx2].discard_unlearned_da != PonPortTable[PonPortIdx1].discard_unlearned_da))
		{
		#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		OLT_addr_table_cfg_t addrtbl_cfg;
		PonPortTable[PonPortIdx2].discard_unlearned_sa = PonPortTable[PonPortIdx1].discard_unlearned_sa;
		PonPortTable[PonPortIdx2].discard_unlearned_da = PonPortTable[PonPortIdx1].discard_unlearned_da;
		OLT_GetAddressTableConfig(PonPortIdx2, &addrtbl_cfg);
		addrtbl_cfg.discard_llid_unlearned_sa = PonPortTable[PonPortIdx2].discard_unlearned_sa;
		addrtbl_cfg.discard_unknown_da = PonPortTable[PonPortIdx2].discard_unlearned_da ;
		OLT_SetAddressTableConfig(PonPortIdx2,&addrtbl_cfg);
		
		#else
		PON_address_table_config_t address_table_config;
		PonPortTable[PonPortIdx2].discard_unlearned_sa = PonPortTable[PonPortIdx1].discard_unlearned_sa;
		PonPortTable[PonPortIdx2].discard_unlearned_da = PonPortTable[PonPortIdx1].discard_unlearned_da;
		PAS_get_address_table_configuration(PonPortIdx2, &address_table_config);
		address_table_config.discard_llid_unlearned_sa = PonPortTable[PonPortIdx2].discard_unlearned_sa;
		address_table_config.discard_unknown_da = PonPortTable[PonPortIdx2].discard_unlearned_da ;
		PAS_set_address_table_configuration(PonPortIdx2,address_table_config);
		#endif
		}

	if(PonPortTable[PonPortIdx2].table_full_handle_mode != PonPortTable[PonPortIdx1].table_full_handle_mode)
		{
		PonPortTable[PonPortIdx2].table_full_handle_mode = PonPortTable[PonPortIdx1].table_full_handle_mode;
		SetPonPortAddressTableFullHandlingMode(PonPortIdx2,PonPortTable[PonPortIdx2].table_full_handle_mode);
		}

#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
	if(PonPortTable[PonPortIdx2].vlan_tpid != PonPortTable[PonPortIdx].vlan_tpid )
		{
		SetPonPortVlanTpid(PonPortIdx2, PonPortTable[PonPortIdx].vlan_tpid);
		}
#endif

	/* BER/FER 告警门限及使能设置*/
	{
	unsigned int AlmEn1, AlmEn2;
	
	monOnuBerAlmEnGet( PonPortIdx1, &AlmEn1);
	monOnuBerAlmEnGet( PonPortIdx2, &AlmEn2);
	if( AlmEn1 != AlmEn2 )
		monOnuBerAlmEnSet( PonPortIdx2, AlmEn1 );

	monOnuFerAlmEnGet( PonPortIdx1, &AlmEn1);
	monOnuFerAlmEnGet( PonPortIdx2, &AlmEn2);
	if( AlmEn1 != AlmEn2 )
		monOnuFerAlmEnSet( PonPortIdx2, AlmEn1 );

	monPonBerAlmEnGet( PonPortIdx1, &AlmEn1);
	monPonBerAlmEnGet( PonPortIdx2, &AlmEn2);
	if( AlmEn1 != AlmEn2 )
		monPonBerAlmEnSet( PonPortIdx2, AlmEn1 );

	monPonFerAlmEnGet( PonPortIdx1, &AlmEn1);
	monPonFerAlmEnGet( PonPortIdx2, &AlmEn2);
	if( AlmEn1 != AlmEn2 )
		monPonFerAlmEnSet( PonPortIdx2, AlmEn1 );
	}
	
	PonPortTable[PonPortIdx2].PortFullAlarm = PonPortTable[PonPortIdx1].PortFullAlarm;

	/* 加密相关配置；*/
	PonPortTable[PonPortIdx2].EncryptEnable  = PonPortTable[PonPortIdx1].EncryptEnable ;
	PonPortTable[PonPortIdx2].EncryptDirection = PonPortTable[PonPortIdx1].EncryptDirection;
	PonPortTable[PonPortIdx2].EncryptKeyTime = PonPortTable[PonPortIdx1].EncryptKeyTime;
	/*	
	PonPortTable[PonPortIdx2].CTC_EncryptKeyUpdateTime = PonPortTable[PonPortIdx1].CTC_EncryptKeyUpdateTime;
	PonPortTable[PonPortIdx2].CTC_EncryptKeyNoReplyTime = PonPortTable[PonPortIdx1].CTC_EncryptKeyNoReplyTime;
	PonPortTable[PonPortIdx2].CTC_EncryptTimeingThreshold = PonPortTable[PonPortIdx1].CTC_EncryptTimeingThreshold;
	*/

#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
#if 1
    VOS_MemCpy(PonPortTable[PonPortIdx2].Uplink_vlan_manipulation, PonPortTable[PonPortIdx1].Uplink_vlan_manipulation, sizeof(Olt_llid_vlan_manipulation) * 64);
#else
	for(i=0; i< MAXONUPERPON; i++)
	{
		PonPortTable[PonPortIdx2].Uplink_vlan_manipulation[i].vlan_manipulation = PonPortTable[PonPortIdx1].Uplink_vlan_manipulation[i].vlan_manipulation;
		PonPortTable[PonPortIdx2].Uplink_vlan_manipulation[i].original_vlan_id = PonPortTable[PonPortIdx1].Uplink_vlan_manipulation[i].original_vlan_id;
		PonPortTable[PonPortIdx2].Uplink_vlan_manipulation[i].new_vlan_id = PonPortTable[PonPortIdx1].Uplink_vlan_manipulation[i].new_vlan_id;
		PonPortTable[PonPortIdx2].Uplink_vlan_manipulation[i].new_priority = PonPortTable[PonPortIdx1].Uplink_vlan_manipulation[i].new_priority;
		PonPortTable[PonPortIdx2].Uplink_vlan_manipulation[i].ethernet_type = PonPortTable[PonPortIdx1].Uplink_vlan_manipulation[i].ethernet_type;
	}
#endif
    /* B--added by liwei056@2010-10-14 for D10542 */
	PonPortTable[PonPortIdx2].vlan_tpid = PonPortTable[PonPortIdx1].vlan_tpid;
	PonPortTable[PonPortIdx2].Downlink_vlan_Man_Max = PonPortTable[PonPortIdx1].Downlink_vlan_Man_Max;
	PonPortTable[PonPortIdx2].Downlink_vlan_Cfg_Max = PonPortTable[PonPortIdx1].Downlink_vlan_Cfg_Max;
    /* E--added by liwei056@2010-10-14 for D10542 */
    VOS_MemCpy(PonPortTable[PonPortIdx2].Downlink_vlan_manipulation, PonPortTable[PonPortIdx1].Downlink_vlan_manipulation, sizeof(Olt_llid_vlan_manipulation) * MAX_VID_DOWNLINK);
    VOS_MemCpy(PonPortTable[PonPortIdx2].Downlink_vlan_Config, PonPortTable[PonPortIdx1].Downlink_vlan_Config, 4096);
	RestoreDownlinkVlanManipulationToPon(PonPortIdx2);
#endif

    /* B--added by liwei056@2011-4-7 for GuestNeed */
	PonPortTable[PonPortIdx2].DownlinkPoliceMode = PonPortTable[PonPortIdx1].DownlinkPoliceMode;
    /* E--added by liwei056@2011-4-7 for GuestNeed */

	return( ROK );
}

/* 将一个 PON 口下的ONU 列表复制到另一个PON口下 */
int  CopyAllOnuListFromPon1ToPon2(short int PonPortIdx1, short int PonPortIdx2)
{
	short int PonPortIdx;
	short int OnuIdx;
    int iOnuBaseIdx1;
    int iOnuBaseIdx2;

	PonPortIdx = PonPortIdx1;
	CHECK_PON_RANGE
	PonPortIdx = PonPortIdx2;
	CHECK_PON_RANGE

    iOnuBaseIdx1 = PonPortIdx1 * MAXONUPERPON;
    iOnuBaseIdx2 = PonPortIdx2 * MAXONUPERPON;

	ONU_MGMT_SEM_TAKE;
	for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		if( ThisIsValidOnu(PonPortIdx1, OnuIdx) == ROK )
		{
			VOS_MemCpy( OnuMgmtTable[iOnuBaseIdx2 + OnuIdx].DeviceInfo.MacAddr, OnuMgmtTable[iOnuBaseIdx1 + OnuIdx].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS );
		}
	}
	ONU_MGMT_SEM_GIVE;

	return( ROK );
}

/* 将一个 PON 口下的ONU 设备信息复制到另一个PON口下 */
int CopyAllOnuDeviceInfoFromPon1ToPon2(short int PonPortIdx1, short int PonPortIdx2)
{
	short int PonPortIdx;
	short int OnuIdx;
    int iOnuMgt1Idx;
    int iOnuMgt2Idx;
    int iOnuMgt1BaseIdx;
    int iOnuMgt2BaseIdx;

	PonPortIdx = PonPortIdx1;
	CHECK_PON_RANGE
	PonPortIdx = PonPortIdx2;
	CHECK_PON_RANGE;

    iOnuMgt1BaseIdx = PonPortIdx1 * MAXONUPERPON;
    iOnuMgt2BaseIdx = PonPortIdx2 * MAXONUPERPON;

	ONU_MGMT_SEM_TAKE;
 	for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		if( ThisIsValidOnu(PonPortIdx1, OnuIdx) == ROK )
		{
			iOnuMgt1Idx = iOnuMgt1BaseIdx + OnuIdx;
			iOnuMgt2Idx = iOnuMgt2BaseIdx + OnuIdx;
            
			VOS_MemCpy( OnuMgmtTable[iOnuMgt2Idx].DeviceInfo.VendorInfo, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.VendorInfo, OnuMgmtTable[PonPortIdx1 * MAXONUPERPON + OnuIdx].DeviceInfo.VendorInfoLen );

			SetOnuDeviceName_1( PonPortIdx2, OnuIdx, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.DeviceName, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.DeviceNameLen);
			SetOnuDeviceDesc_1( PonPortIdx2, OnuIdx, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.DeviceDesc, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.DeviceDescLen);
			SetOnuLocation_1( PonPortIdx2, OnuIdx, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.Location, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.LocationLen);

			VOS_MemCpy( OnuMgmtTable[iOnuMgt2Idx].DeviceInfo.SwVersion, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.SwVersion, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.SwVersionLen);
			VOS_MemCpy( OnuMgmtTable[iOnuMgt2Idx].DeviceInfo.HwVersion, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.HwVersion, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.HwVersionLen);
			VOS_MemCpy( OnuMgmtTable[iOnuMgt2Idx].DeviceInfo.BootVersion, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.BootVersion, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.BootVersionLen);
			VOS_MemCpy( OnuMgmtTable[iOnuMgt2Idx].DeviceInfo.FwVersion, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.FwVersion, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.FwVersionLen);

			VOS_MemCpy( OnuMgmtTable[iOnuMgt2Idx].DeviceInfo.DeviceSerial_No, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.DeviceSerial_No, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.DeviceSerial_NoLen);
			VOS_MemCpy( OnuMgmtTable[iOnuMgt2Idx].DeviceInfo.MadeInDate, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.MadeInDate, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.MadeInDateLen);
			
			OnuMgmtTable[iOnuMgt2Idx].DeviceInfo.type = OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.type;
			VOS_MemCpy( OnuMgmtTable[iOnuMgt2Idx].DeviceInfo.OUI, OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.OUI, 3);
			OnuMgmtTable[iOnuMgt2Idx].DeviceInfo.type = OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.type;
			VOS_MemCpy( &(OnuMgmtTable[iOnuMgt2Idx].DeviceInfo.SysLaunchTime), &(OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.SysLaunchTime), sizeof( Date_S ));
			OnuMgmtTable[iOnuMgt2Idx].DeviceInfo.SysUptime = OnuMgmtTable[iOnuMgt1Idx].DeviceInfo.SysUptime;

			OnuMgmtTable[iOnuMgt2Idx].PonType = OnuMgmtTable[iOnuMgt1Idx].PonType;
			OnuMgmtTable[iOnuMgt2Idx].PonRate = OnuMgmtTable[iOnuMgt1Idx].PonRate;

			VOS_MemCpy( &(OnuMgmtTable[iOnuMgt2Idx].device_vendor_id[0]), &(OnuMgmtTable[iOnuMgt1Idx].device_vendor_id[0]), 4);
			OnuMgmtTable[iOnuMgt2Idx].onu_model = OnuMgmtTable[iOnuMgt1Idx].onu_model;
			VOS_MemCpy( &(OnuMgmtTable[iOnuMgt2Idx].chip_vendor_id[0]), &(OnuMgmtTable[iOnuMgt1Idx].chip_vendor_id[0]), CTC_VENDOR_ID_LENGTH);
			OnuMgmtTable[iOnuMgt2Idx].chip_model = OnuMgmtTable[iOnuMgt1Idx].chip_model;
			OnuMgmtTable[iOnuMgt2Idx].revision = OnuMgmtTable[iOnuMgt1Idx].revision;
			VOS_MemCpy( &(OnuMgmtTable[iOnuMgt2Idx].date[0]), &(OnuMgmtTable[iOnuMgt1Idx].date[0]), 3);
			VOS_MemCpy( &(OnuMgmtTable[iOnuMgt2Idx].hardware_version[0]), &(OnuMgmtTable[iOnuMgt1Idx].hardware_version[0]), 8);
			VOS_MemCpy( &(OnuMgmtTable[iOnuMgt2Idx].software_version[0]), &(OnuMgmtTable[iOnuMgt1Idx].software_version[0]), 16);
			OnuMgmtTable[iOnuMgt2Idx].onu_ctc_version = OnuMgmtTable[iOnuMgt1Idx].onu_ctc_version;
			OnuMgmtTable[iOnuMgt2Idx].firmware_version = OnuMgmtTable[iOnuMgt1Idx].firmware_version;
			
		}
	}
 	ONU_MGMT_SEM_GIVE;
	
	return( ROK );

}


/* 将一个 PON 口下的ONU 配置数据复制到另一个PON口下 */
int CopyAllOnuConfigDataFromPon1ToPon2(short int PonPortIdx1, short int PonPortIdx2)
{
	short int PonPortIdx;
	short int OnuIdx;
    int iOnuMgt1Idx;
    int iOnuMgt2Idx;
    int iOnuMgt1BaseIdx;
    int iOnuMgt2BaseIdx;

	PonPortIdx = PonPortIdx1;
	CHECK_PON_RANGE
	PonPortIdx = PonPortIdx2;
	CHECK_PON_RANGE;

    iOnuMgt1BaseIdx = PonPortIdx1 * MAXONUPERPON;
    iOnuMgt2BaseIdx = PonPortIdx2 * MAXONUPERPON;

	ONU_MGMT_SEM_TAKE;
 	for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		if( ThisIsValidOnu(PonPortIdx1, OnuIdx) == ROK )
		{
			iOnuMgt1Idx = iOnuMgt1BaseIdx + OnuIdx;
			iOnuMgt2Idx = iOnuMgt2BaseIdx + OnuIdx;
            
			OnuMgmtTable[iOnuMgt2Idx].AdminStatus = OnuMgmtTable[iOnuMgt1Idx].AdminStatus;
            /* B--remed by liwei056@2010-10-14 for  Pon-FastSwicthHover's Online-2SameOnus BUG */
#if 0
            /* B--added by liwei056@2010-1-29 for  Pon-FastSwicthHover */
			OnuMgmtTable[iOnuMgt2Idx].OperStatus = OnuMgmtTable[iOnuMgt1Idx].OperStatus;
			OnuMgmtTable[iOnuMgt2Idx].LLID = OnuMgmtTable[iOnuMgt1Idx].LLID;
            /* E--added by liwei056@2010-1-29 for  Pon-FastSwicthHover */
#endif
            /* E--remed by liwei056@2010-10-14 for  Pon-FastSwicthHover's Online-2SameOnus BUG */
			OnuMgmtTable[iOnuMgt2Idx].TrafficServiceEnable = OnuMgmtTable[iOnuMgt1Idx].TrafficServiceEnable;
			OnuMgmtTable[iOnuMgt2Idx].SoftwareUpdateCtrl = OnuMgmtTable[iOnuMgt1Idx].SoftwareUpdateCtrl;
			OnuMgmtTable[iOnuMgt2Idx].EncryptEnable = OnuMgmtTable[iOnuMgt1Idx].EncryptEnable;
			OnuMgmtTable[iOnuMgt2Idx].EncryptDirection = OnuMgmtTable[iOnuMgt1Idx].EncryptDirection;
			OnuMgmtTable[iOnuMgt2Idx].EncryptKeyTime = OnuMgmtTable[iOnuMgt1Idx].EncryptKeyTime;
			OnuMgmtTable[iOnuMgt2Idx].EncryptNoReplyTimeout = OnuMgmtTable[iOnuMgt1Idx].EncryptNoReplyTimeout;
			OnuMgmtTable[iOnuMgt2Idx].PonType = OnuMgmtTable[iOnuMgt1Idx].PonType;
			OnuMgmtTable[iOnuMgt2Idx].PonRate = OnuMgmtTable[iOnuMgt1Idx].PonRate;
			OnuMgmtTable[iOnuMgt2Idx].RTT = OnuMgmtTable[iOnuMgt1Idx].RTT;

			OnuMgmtTable[iOnuMgt2Idx].AlarmMask = OnuMgmtTable[iOnuMgt1Idx].AlarmMask;
			OnuMgmtTable[iOnuMgt2Idx].devAlarmMask = OnuMgmtTable[iOnuMgt1Idx].devAlarmMask;	/* added by xieshl 20070927 */

			/*OnuMgmtTable[PonPortIdx2 * MAXONUPERPON + OnuIdx].AlarmStatus = OnuMgmtTable[PonPortIdx1 * MAXONUPERPON + OnuIdx].AlarmStatus;*/
			VOS_MemCpy(&(OnuMgmtTable[iOnuMgt2Idx].AlarmConfigInfo), &(OnuMgmtTable[iOnuMgt1Idx].AlarmConfigInfo), sizeof(PON_olt_monitoring_configuration_t ));
			OnuMgmtTable[iOnuMgt2Idx].ProtectType = OnuMgmtTable[iOnuMgt1Idx].ProtectType;
			OnuMgmtTable[iOnuMgt2Idx].PowerStatus = OnuMgmtTable[iOnuMgt1Idx].PowerStatus;
			OnuMgmtTable[iOnuMgt2Idx].HaveBattery = OnuMgmtTable[iOnuMgt1Idx].HaveBattery;
			OnuMgmtTable[iOnuMgt2Idx].BatteryCapability = OnuMgmtTable[iOnuMgt1Idx].BatteryCapability;
			OnuMgmtTable[iOnuMgt2Idx].PowerOn = OnuMgmtTable[iOnuMgt1Idx].PowerOn;
			/*OnuMgmtTable[PonPortIdx2 * MAXONUPERPON + OnuIdx].PowerOffCounter = OnuMgmtTable[PonPortIdx1 * MAXONUPERPON + OnuIdx].PowerOffCounter;*/
				
			VOS_MemCpy(&(OnuMgmtTable[iOnuMgt2Idx].LlidTable[0]), &(OnuMgmtTable[iOnuMgt1Idx].LlidTable[0]), MAXLLIDPERONU * sizeof(OnuLLIDTable_S ));
			
			VOS_MemCpy(&(OnuMgmtTable[iOnuMgt2Idx].updaterec), &(OnuMgmtTable[iOnuMgt1Idx].updaterec), sizeof(onuupdateclk_t));
			VOS_MemCpy(&(OnuMgmtTable[iOnuMgt2Idx].PeerToPeer[0]), &(OnuMgmtTable[iOnuMgt1Idx].PeerToPeer[0]), 8);
			OnuMgmtTable[iOnuMgt2Idx].address_not_found_flag = OnuMgmtTable[iOnuMgt1Idx].address_not_found_flag;
			OnuMgmtTable[iOnuMgt2Idx].broadcast_flag = OnuMgmtTable[iOnuMgt1Idx].broadcast_flag;


			OnuMgmtTable[iOnuMgt2Idx].GE_supporting = OnuMgmtTable[iOnuMgt1Idx].GE_supporting;
			OnuMgmtTable[iOnuMgt2Idx].FE_supporting = OnuMgmtTable[iOnuMgt1Idx].FE_supporting;
			OnuMgmtTable[iOnuMgt2Idx].VoIP_supporting = OnuMgmtTable[iOnuMgt1Idx].VoIP_supporting;
			OnuMgmtTable[iOnuMgt2Idx].TDM_CES_supporting = OnuMgmtTable[iOnuMgt1Idx].TDM_CES_supporting;

			OnuMgmtTable[iOnuMgt2Idx].ctc_onu_type = OnuMgmtTable[iOnuMgt1Idx].ctc_onu_type;
			OnuMgmtTable[iOnuMgt2Idx].GE_Ethernet_ports_number = OnuMgmtTable[iOnuMgt1Idx].GE_Ethernet_ports_number;
			OnuMgmtTable[iOnuMgt2Idx].FE_Ethernet_ports_number = OnuMgmtTable[iOnuMgt1Idx].FE_Ethernet_ports_number;
			OnuMgmtTable[iOnuMgt2Idx].POTS_ports_number = OnuMgmtTable[iOnuMgt1Idx].POTS_ports_number;
			OnuMgmtTable[iOnuMgt2Idx].E1_ports_number = OnuMgmtTable[iOnuMgt1Idx].E1_ports_number;
			VOS_MemCpy(&(OnuMgmtTable[iOnuMgt2Idx].Ports_distribution[0]), &(OnuMgmtTable[iOnuMgt1Idx].Ports_distribution[0]), 16);
			
			OnuMgmtTable[iOnuMgt2Idx].Upstream_queues_number = OnuMgmtTable[iOnuMgt1Idx].Upstream_queues_number;
			OnuMgmtTable[iOnuMgt2Idx].Max_upstream_queues_per_port = OnuMgmtTable[iOnuMgt1Idx].Max_upstream_queues_per_port;
			OnuMgmtTable[iOnuMgt2Idx].Upstream_queues_allocation_increment = OnuMgmtTable[iOnuMgt1Idx].Upstream_queues_allocation_increment;
			OnuMgmtTable[iOnuMgt2Idx].Downstream_queues_number = OnuMgmtTable[iOnuMgt1Idx].Downstream_queues_number;
			OnuMgmtTable[iOnuMgt2Idx].Max_downstream_queues_per_port = OnuMgmtTable[iOnuMgt1Idx].Max_downstream_queues_per_port;
			OnuMgmtTable[iOnuMgt2Idx].multicastSwitch = OnuMgmtTable[iOnuMgt1Idx].multicastSwitch;
			OnuMgmtTable[iOnuMgt2Idx].stpEnable = OnuMgmtTable[iOnuMgt1Idx].stpEnable ;
			/*OnuMgmtTable[PonPortIdx2 * MAXONUPERPON + OnuIdx].CTC_EncryptCtrl = OnuMgmtTable[PonPortIdx1 * MAXONUPERPON + OnuIdx].CTC_EncryptCtrl;*/
			OnuMgmtTable[iOnuMgt2Idx].FEC_ability = OnuMgmtTable[iOnuMgt1Idx].FEC_ability;
			OnuMgmtTable[iOnuMgt2Idx].FEC_Mode = OnuMgmtTable[iOnuMgt1Idx].FEC_Mode;
			OnuMgmtTable[iOnuMgt2Idx].LoopbackEnable = OnuMgmtTable[iOnuMgt1Idx].LoopbackEnable;

			OnuMgmtTable[iOnuMgt2Idx].RegisterFlag = OnuMgmtTable[iOnuMgt1Idx].RegisterFlag;
			
			VOS_MemCpy(&(OnuMgmtTable[iOnuMgt2Idx].vlan_priority),&(OnuMgmtTable[iOnuMgt1Idx].vlan_priority), sizeof(Onu_vlan_priority_t));
			/*
			OnuMgmtTable[PonPortIdx2 * MAXONUPERPON + OnuIdx].SubSlotNumMax = OnuMgmtTable[PonPortIdx1 * MAXONUPERPON + OnuIdx].SubSlotNumMax;
			OnuMgmtTable[PonPortIdx2 * MAXONUPERPON + OnuIdx].SubSlotNum = OnuMgmtTable[PonPortIdx1 * MAXONUPERPON + OnuIdx].SubSlotNum;
			VOS_MemCpy(&(OnuMgmtTable[PonPortIdx2 * MAXONUPERPON + OnuIdx].SubSlot[0]),&(OnuMgmtTable[PonPortIdx1 * MAXONUPERPON + OnuIdx].SubSlot[0]), sizeof(Onu_SubBoard_Info_t) * ONU_SUB_BOARD_MAX_NUMBER );
			*/
		}
	}
 	ONU_MGMT_SEM_GIVE;
	return( ROK );

}

int CopyOnuAuthTableFromPon1ToPon2(short int PonPortIdx1, short int PonPortIdx2 )
{
	short int PonPortIdx;
	unsigned long Slot1, Port1, Entry1=0;
	unsigned long Slot2, Port2, Entry2=1;
	unsigned long NextSlot, NextPort, NextEntry;
	short int ret;
	/*unsigned long Auth_Enable;*/
	unsigned char MacAddr[BYTES_IN_MAC_ADDRESS];
	unsigned long length;
	

	PonPortIdx=PonPortIdx1;
	CHECK_PON_RANGE
	PonPortIdx = PonPortIdx2;
	CHECK_PON_RANGE;

	Slot1 = GetCardIdxByPonChip(PonPortIdx1);
	Port1 = GetPonPortByPonChip(PonPortIdx1);
	Slot2 = GetCardIdxByPonChip(PonPortIdx2);
	Port2 = GetPonPortByPonChip(PonPortIdx2);
	
	ret = getNextOnuAuthEntry( Slot1, Port1, Entry1, &NextSlot, &NextPort, &NextEntry );
	
	if(( ret == VOS_ERROR ) || (( ret == VOS_OK ) && ((Slot1 != NextSlot ) || ( Port1 != NextPort ))))
		{
		return( ROK );
		}
	
	if(( NextEntry < 1 ) || ( NextEntry > MAXONUPERPON ))
		{
		return( ROK );
		}
	
	/*VOS_TaskLock();*/
	do{				
		Entry1 = NextEntry;
		ret = getOnuAuthMacAddress( Slot1, Port1, Entry1, MacAddr, &length );
		
		if( ret != VOS_ERROR )
			{
			 ret = isHaveAuthMacAddress(Slot2, Port2, MacAddr, &Entry2 );

			if( ret == VOS_ERROR )
	 			{		 
				if((Entry2 >= 1 ) &&( Entry2 <= MAXONUPERPON ))
				 	{
				 	setOnuAuthStatus( Slot2, Port2, Entry2, V2R1_ENTRY_CREATE_AND_GO);	
					setOnuAuthMacAddress( Slot2, Port2,  Entry2, MacAddr );	
	 				}
				}
			}
		
		ret =  getNextOnuAuthEntry( Slot1, Port1, Entry1, &NextSlot, &NextPort, &NextEntry );		
		}while(( ret == VOS_OK ) && ( Slot1 == NextSlot ) && ( Port1 == NextPort ) );
			
	/*VOS_TaskUnlock();*/

	return( ROK );

}

int CopyAllOnuDataFromPon1ToPon2(short int PonPortIdx1, short int PonPortIdx2 )
{
	CopyPonConfigDataFromPon1ToPon2( PonPortIdx1, PonPortIdx2);
	CopyAllOnuListFromPon1ToPon2(PonPortIdx1, PonPortIdx2 );
	CopyAllOnuDeviceInfoFromPon1ToPon2(PonPortIdx1, PonPortIdx2 );
	CopyAllOnuConfigDataFromPon1ToPon2(PonPortIdx1, PonPortIdx2 );
	CopyOnuAuthTableFromPon1ToPon2(PonPortIdx1, PonPortIdx2 );

	ctcOnuIgmpAuthInfoSwitch( PonPortIdx1, PonPortIdx2);   
    /* B--remmed by liwei056@2011-11-4 for D13342 & NewCTCCfgFile */
	/* ponSwitchCtcOnuCfgData( PonPortIdx1, PonPortIdx2); */
    /* E--remmed by liwei056@2011-11-4 for D13342 & NewCTCCfgFile */

	return( ROK );
}

int CopyOltSwapParam(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    int iRlt = 0;
    int iSrcValue[3];
    
    iSrcValue[0] = PonPortTable[SrcPonPortIdx].swap_triggers;
    if ( DstPonPortIdx == SrcPonPortIdx )
    {
        /* 协同口的配置，只能恢复 */
        iSrcValue[1] = PonPortTable[SrcPonPortIdx].protect_slot;
        iSrcValue[2] = PonPortTable[SrcPonPortIdx].protect_port;
    }
    else
    {
        /* 协同口的配置，不能拷贝 */
        iSrcValue[1] = -1;
        iSrcValue[2] = -1;
    }
    
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OLT_SetHotSwapParam(DstPonPortIdx, -1, -1, -1, iSrcValue[0], iSrcValue[1], iSrcValue[2]);
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( V2R1_PON_PORT_SWAP_TRIGGER != iSrcValue[0] )
        {
            iRlt = OLT_SetHotSwapParam(DstPonPortIdx, -1, -1, -1, iSrcValue[0], iSrcValue[1], iSrcValue[2]);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            int iDstValue[1];
            
            iDstValue[0] = PonPortTable[DstPonPortIdx].swap_triggers;
            
            if ( iDstValue[0] != iSrcValue[0] )
            {
                iRlt = OLT_SetHotSwapParam(DstPonPortIdx, -1, -1, -1, iSrcValue[0], iSrcValue[1], iSrcValue[2]);
            }
        }
        else
        {
            iRlt = OLT_SetHotSwapParam(DstPonPortIdx, -1, -1, -1, iSrcValue[0], iSrcValue[1], iSrcValue[2]);
        }
    }

    return iRlt;
}
#endif

int  CopyOnuAuthTable(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx)
{
    ULONG ulSlot, ulPort;
	LogicEntity *pSrcEntry, *pDstEntry;

    ulSlot = GetCardIdxByPonChip(SrcPonPortIdx);
    ulPort = GetPonPortByPonChip(SrcPonPortIdx);
    pSrcEntry = getLogicEntityBySlot(ulSlot, ulPort, SrcOnuIdx + 1);

    if ( (DstPonPortIdx == SrcPonPortIdx)
        && (DstOnuIdx == SrcOnuIdx) )
    {
        pDstEntry = pSrcEntry;
    }
    else
    {
        ulSlot = GetCardIdxByPonChip(DstPonPortIdx);
        ulPort = GetPonPortByPonChip(DstPonPortIdx);
        pDstEntry = getLogicEntityBySlot(ulSlot, ulPort, DstOnuIdx + 1);
    }

    if ( (NULL != pSrcEntry)
        && (NULL != pDstEntry) )
    {
        if (pDstEntry != pSrcEntry)
        {
            if ( pDstEntry->onu_auth_status == 1 )
            {
                OnuMgt_SetMacAuthMode(DstPonPortIdx, DstOnuIdx, V2R1_ENTRY_DESTORY, pDstEntry->onu_auth_mac);
            }
        }
        
        if ( pSrcEntry->onu_auth_status == 1 )
        {
            OnuMgt_SetMacAuthMode(DstPonPortIdx, DstOnuIdx, V2R1_ENTRY_CREATE_AND_GO, pSrcEntry->onu_auth_mac);
        }
    }
    else
    {
        return RERROR;
    }
        
	return ROK;
}

int CopyOltAuthTable(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    int iRlt = 0;
    int i;
    ULONG ulSlot, ulPort;
    PonPortItem *pSrcPortEnt, *pDstPortEnt;
    unsigned char *pbAuthMac;

    ulSlot = GetCardIdxByPonChip(SrcPonPortIdx);
    ulPort = GetPonPortByPonChip(SrcPonPortIdx);
    if ( NULL == (pSrcPortEnt = getPonPortItemBySlot(ulSlot, ulPort)) )
    {
        return VOS_ERROR;
    }   

    if ( SrcPonPortIdx == DstPonPortIdx )
    {
        /* 自拷贝，应该是配置恢复 */
        pDstPortEnt = pSrcPortEnt;
    }
    else
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            ulSlot = GetCardIdxByPonChip(DstPonPortIdx);
            ulPort = GetPonPortByPonChip(DstPonPortIdx);
            if ( NULL == (pDstPortEnt = getPonPortItemBySlot(ulSlot, ulPort)) )
            {
                return VOS_ERROR;
            }   
        }
        else
        {
            pDstPortEnt = NULL;
        }
    }

    for (i=0; (i<MAX_ONU_PER_PONPORT) && (0 == iRlt); i++)
    {
        pbAuthMac = pSrcPortEnt->logicEntity[i].onu_auth_mac;
        if ( 1 == pSrcPortEnt->logicEntity[i].onu_auth_status )
        {
            if ( (OLT_COPYFLAGS_COVER & CopyFlags)
                || (OLT_COPYFLAGS_ONLYNEW & CopyFlags) )
            {
                /* 此API自带覆盖功能 */
                if ( 0 == (iRlt = OLT_SetMacAuth(DstPonPortIdx, ENABLE, pbAuthMac)) )
                {
                    iRlt = OnuMgt_SetMacAuthMode(DstPonPortIdx, i, V2R1_ENTRY_CREATE_AND_GO, pbAuthMac);
                }
            }
            else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
            {
                if ( NULL != pDstPortEnt )
                {
                    if ( (0 == pDstPortEnt->logicEntity[i].onu_auth_status)
                        || (0 != VOS_MemCmp(pSrcPortEnt->logicEntity[i].onu_auth_mac, pbAuthMac, BYTES_IN_MAC_ADDRESS)) )
                    {
                        if ( 0 == (iRlt = OLT_SetMacAuth(DstPonPortIdx, ENABLE, pbAuthMac)) )
                        {
                            iRlt = OnuMgt_SetMacAuthMode(DstPonPortIdx, i, V2R1_ENTRY_CREATE_AND_GO, pbAuthMac);
                        }
                    }
                }
                else
                {
                    if ( 0 == (iRlt = OLT_SetMacAuth(DstPonPortIdx, ENABLE, pbAuthMac)) )
                    {
                        iRlt = OnuMgt_SetMacAuthMode(DstPonPortIdx, i, V2R1_ENTRY_CREATE_AND_GO, pbAuthMac);
                    }
                }
            }
        }
        else
        {
            if ( OLT_COPYFLAGS_COVER & CopyFlags )
            {
                if ( 0 == (iRlt = OLT_SetMacAuth(DstPonPortIdx, DISABLE, pbAuthMac)) )
                {
                    iRlt = OnuMgt_SetMacAuthMode(DstPonPortIdx, i, V2R1_ENTRY_DESTORY, pbAuthMac);
                }
            }
            else if ( (OLT_COPYFLAGS_CHECK & CopyFlags) && (NULL != pDstPortEnt) )
            {
                if ( 1 == pDstPortEnt->logicEntity[i].onu_auth_status )
                {
                    if ( 0 == (iRlt = OLT_SetMacAuth(DstPonPortIdx, DISABLE, pbAuthMac)) )
                    {
                        iRlt = OnuMgt_SetMacAuthMode(DstPonPortIdx, i, V2R1_ENTRY_DESTORY, pbAuthMac);
                    }
                }
            }
        }
    }

    return iRlt;
}

int  CopyPonDownlinkPoliceMode(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    int iRlt = 0;
    int iSrcValue;

    iSrcValue = PonPortTable[SrcPonPortIdx].DownlinkPoliceMode;
    
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OLT_SetOnuDownlinkPoliceMode(DstPonPortIdx, iSrcValue);
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( V2R1_DISABLE != iSrcValue )
        {
            iRlt = OLT_SetOnuDownlinkPoliceMode(DstPonPortIdx, iSrcValue);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            int iDstValue = PonPortTable[DstPonPortIdx].DownlinkPoliceMode;
            
            if ( iDstValue != iSrcValue )
            {
                iRlt = OLT_SetOnuDownlinkPoliceMode(DstPonPortIdx, iSrcValue);
            }
        }
        else
        {
            iRlt = OLT_SetOnuDownlinkPoliceMode(DstPonPortIdx, iSrcValue);
        }
    }

    return iRlt;
}

int  CopyPonParams(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    int iRlt = 0;
    int iSrcValue[2];
    
    iSrcValue[0] = PonPortTable[SrcPonPortIdx].range;
    iSrcValue[1] = PonPortTable[SrcPonPortIdx].MACAgeingTime;
    
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OLT_UpdatePonParams(DstPonPortIdx, iSrcValue[0], iSrcValue[1]);
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( (DEFAULT_MAC_AGING_TIME != iSrcValue[0])
            || (PON_RANGE_20KM != iSrcValue[1]) )
        {
            iRlt = OLT_UpdatePonParams(DstPonPortIdx, iSrcValue[0], iSrcValue[1]);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            int iDstValue[2];
            
            iDstValue[0] = PonPortTable[DstPonPortIdx].range;
            iDstValue[1] = PonPortTable[DstPonPortIdx].MACAgeingTime;
            
            if ( (iDstValue[0] != iSrcValue[0])
                || (iDstValue[1] != iSrcValue[1]) )
            {
                iRlt = OLT_UpdatePonParams(DstPonPortIdx, iSrcValue[0], iSrcValue[1]);
            }
        }
        else
        {
            iRlt = OLT_UpdatePonParams(DstPonPortIdx, iSrcValue[0], iSrcValue[1]);
        }
    }

    return iRlt;
}

int  CopyPonRange(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    int iRlt = 0;
    unsigned int uiSrcValue[2];

    uiSrcValue[0] = PonPortTable[SrcPonPortIdx].range;
    uiSrcValue[1] = PonPortTable[SrcPonPortIdx].MaxRTT;
    
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OLT_SetPonRange(DstPonPortIdx, uiSrcValue[0], uiSrcValue[1]);
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( (PON_RANGE_20KM != uiSrcValue[0])
            || (PON_MAX_RTT_40KM != uiSrcValue[1]) )
        {
            iRlt = OLT_SetPonRange(DstPonPortIdx, uiSrcValue[0], uiSrcValue[1]);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            unsigned int uiDstValue[2];
            
            uiDstValue[0] = PonPortTable[DstPonPortIdx].range;
            uiDstValue[1] = PonPortTable[DstPonPortIdx].MaxRTT;

            if ( (uiDstValue[0] != uiSrcValue[0])
                || (uiDstValue[1] != uiSrcValue[1]) )
            {
                iRlt = OLT_SetPonRange(DstPonPortIdx, uiSrcValue[0], uiSrcValue[1]);
            }
        }
        else
        {
            iRlt = OLT_SetPonRange(DstPonPortIdx, uiSrcValue[0], uiSrcValue[1]);
        }
    }

    return iRlt;
}
void CopyAllConfigFromPon1ToPon2(short int PonPortIdx1, short int PonPortIdx2, int CopyFlags)
{
    if ( OLT_ISLOCAL(PonPortIdx1) )
    {
        /* B--added by liwei056@2011-1-17 for D11903 */
        if ( OLT_ISLOCAL(PonPortIdx2)
            && (!SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER) )
        {
            /* 只有板内的配置拷贝，不经过主控板 */
            /* 为了同步主控板的配置，强行令其经过主控板 */
            PonPortIdx2 = OLT_DEVICE_ID(GetCardIdxByPonChip(PonPortIdx2), GetPonPortByPonChip(PonPortIdx2));
        }
        /* E--added by liwei056@2011-1-17 for D11903 */
        
        /* 先移动OLT配置 */
        CopyOlt(PonPortIdx2, PonPortIdx1, CopyFlags);
        
        /* 再移动ONU配置[注意: ONU的配置有可能同时被其倒换后的重注册过程修改] */
        CopyOnu(PonPortIdx2, -1, PonPortIdx1, -1, CopyFlags);
    }
    else
    {
        int i;

        /* 通知远程发起OLT拷贝 */
        OLT_CopyOlt(PonPortIdx1, PonPortIdx2, CopyFlags);
        for (i=0; i<MAXONUPERPON; i++)
        {
            /* 通知远程发起ONU拷贝[本地循环，防止RPC超时] */
            if( ROK != ThisIsValidOnu(PonPortIdx1, i) ) continue;
            OnuMgt_CopyOnu(PonPortIdx1, i, PonPortIdx2, i, CopyFlags);
        }
    }

    return;
}

/*Begin:for onu swap by jinhl@2013-02-22*/
#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
void CopyOnuConfigFromPon1ToPon2(short int PonPortIdx1, short int OnuIdx1, short int PonPortIdx2, short int OnuIdx2, int CopyFlags)
{
    int CardIdx = 0;
	int PonPort = 0;

	CardIdx = GetCardIdxByPonChip(PonPortIdx2);
	PonPort = GetPonPortByPonChip(PonPortIdx2);
	
    if ( OLT_ISLOCAL(PonPortIdx1) )
    {
        /* B--added by liwei056@2011-1-17 for D11903 */
        if ( OLT_ISLOCAL(PonPortIdx2)
            && (!SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER) )
        {
            /* 只有板内的配置拷贝，不经过主控板 */
            /* 为了同步主控板的配置，强行令其经过主控板 */
            PonPortIdx2 = OLT_DEVICE_ID(GetCardIdxByPonChip(PonPortIdx2), GetPonPortByPonChip(PonPortIdx2));
        }
        /* E--added by liwei056@2011-1-17 for D11903 */
          
        /* 再移动ONU配置[注意: ONU的配置有可能同时被其倒换后的重注册过程修改] */
        CopyOnu(PonPortIdx2, OnuIdx2, PonPortIdx1, OnuIdx1, CopyFlags);
    }
    else
    {
                      
        {
            /* 通知远程发起ONU拷贝[本地循环，防止RPC超时] */
            if( ROK != ThisIsValidOnu(PonPortIdx1, OnuIdx1) ) return;
            OnuMgt_CopyOnu(PonPortIdx1, OnuIdx1, PonPortIdx2, OnuIdx2, CopyFlags);
        }
    }

    return;
}
#endif
/*End:for onu swap by jinhl@2013-02-22*/
int ScanAuthOnuCounter(short int PonPortIdx )
{
	CHECK_PON_RANGE

	if( PonPortTable[PonPortIdx].PendingOnuCounter > 0 )
		PonPortTable[PonPortIdx].PendingOnuCounter --;

	return( ROK );
}

int HasOnuOnlineOrRegistering( short int PonPortIdx )
{
	/*short int PonChipType;*/
	int ret = FALSE;
	
	CHECK_PON_RANGE;
    
	if( OnRegOnuCounter( PonPortIdx ) > 0 ) return( TRUE );

	/* modified by xieshl 20110616, 加保护 */
	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
	/*PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if( PonChipType != PONCHIP_PAS5001)*/
	{
		if( V2R1_CTC_STACK )
		{
			if(PonPortTable[PonPortIdx].PendingOnuCounter != 0 )
			{
				ret = TRUE;
			}
		}
	}

	if( !ret )
	{
		ret = (PonPortTable[PonPortIdx].PendingOnu.Next != NULL ) || (PonPortTable[PonPortIdx].PendingOnu_Conf.Next != NULL);
	}
	VOS_SemGive( OnuPendingDataSemId );
	
	return ret;
}

#ifdef __CTC_TEST

/* added by chenfj 2008-9-4
     增加50 毫秒定时器, 用于轮询PON 口是否接收到光信号
     */
int  CheckPonPortSignal(short int PonPortIdx)
{
	int flag = V2R1_ENABLE;
	bool value;
	PON_gpio_line_io_t  direction;
	
	CHECK_PON_RANGE

#if 0
    /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	if(OLT_GpioAccess(PonPortIdx, PON_GPIO_LINE_0, PON_GPIO_LINE_INPUT, PON_LED_OFF, 
 								&direction, &value ) != 0 )

#else
    if ( 0 != OLT_ReadGpio(PonPortIdx, OLT_GPIO_PON_LOSS, &value) )
#endif
    {
 		return(RERROR);
    }

/*	if((value == 1 ) &&(PonPortTable[PonPortIdx].SignalLossFlag == V2R1_DISABLE))
		{
		PonPortTable[PonPortIdx].SignalLossFlag = V2R1_ENABLE;
		flag = V2R1_DISABLE;
		}
	else if((value == 0 ) &&(PonPortTable[PonPortIdx].SignalLossFlag == V2R1_ENABLE))
		{
		PonPortTable[PonPortIdx].SignalLossFlag = V2R1_DISABLE;
		}
*/
	if( 1 == value )
		flag = V2R1_DISABLE;
    
	return(flag);
}

#if 0
int  CheckPonPortSignal_time_delay=0;
int FlashOnuDataPath(short int PonPortIdx)
{
	short int OnuIdx;
	short int Llid;
	for(OnuIdx=0; OnuIdx<MAXONUPERPON;OnuIdx++)
		{
		if(GetOnuOperStatus(PonPortIdx,OnuIdx) != ONU_OPER_STATUS_UP )continue;
		Llid = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
		if(Llid != INVALID_LLID)
			{
			VOS_TaskLock();
			REMOTE_PASONU_uni_set_port( PonPortIdx, Llid, 1, 0 );
			if(CheckPonPortSignal_time_delay != 0)
				VOS_TaskDelay(CheckPonPortSignal_time_delay);
			REMOTE_PASONU_uni_set_port( PonPortIdx, Llid, 1, 1);
			VOS_TaskUnlock();
			/*sys_console_printf("flash onu %d/%d data path\r\n",(PonPortIdx+1),(OnuIdx+1));*/
			}
		}
	return(ROK);
}
#endif
int  CheckPonPortSignal_time_delay=5;

typedef struct {
	unsigned int UplinkClass;
	unsigned int fixed;
	unsigned int assured_bw;
	unsigned int best_effort_bw;
	int delay;
	short int Llid;
}onu_up_bw_t;
onu_up_bw_t onu_up_bw[MAXONUPERPONNOLIMIT];

int FlashOnuDataPath(short int PonPortIdx)
{
	short int OnuIdx;
	short int Llid;
	int i;
	int delay_rand, delay_up;

	VOS_MemZero( onu_up_bw, sizeof(onu_up_bw) );
	for(OnuIdx=0; OnuIdx<MAXONUPERPON;OnuIdx++)
	{
		if(GetOnuOperStatus(PonPortIdx,OnuIdx) != ONU_OPER_STATUS_UP )continue;
		Llid = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
		if(Llid != INVALID_LLID)
		{
			delay_rand = (rand()*16/32767 + 4) * 10000;
			onu_up_bw[OnuIdx].delay = delay_rand;
			onu_up_bw[OnuIdx].Llid = Llid;

			VOS_TaskLock();
			#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
			CTC_SetOnuUniPort( PonPortIdx, Llid, 1, 0 );
			#else
			REMOTE_PASONU_uni_set_port( PonPortIdx, Llid, 1, 0 );
			#endif
			
			VOS_TaskUnlock();

			srand( VOS_GetTick() );
			
			VOS_TaskLock();
			GetOnuUplinkBW_2( PonPortIdx, OnuIdx, &onu_up_bw[OnuIdx].UplinkClass, &onu_up_bw[OnuIdx].fixed, &onu_up_bw[OnuIdx].assured_bw, &onu_up_bw[OnuIdx].best_effort_bw );
			SetOnuUplinkBW_2( PonPortIdx, OnuIdx, 0, 1, 0, 64, 1064);
			VOS_TaskUnlock();
		}
	}

	if( CheckPonPortSignal_time_delay )
		VOS_TaskDelay( CheckPonPortSignal_time_delay );

	for(OnuIdx=0; OnuIdx<MAXONUPERPON;OnuIdx++)
	{
		if(onu_up_bw[OnuIdx].delay != 0)
		{
			for(i=0; i<onu_up_bw[OnuIdx].delay; i++ );
			VOS_TaskLock();
			#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
			CTC_SetOnuUniPort( PonPortIdx, onu_up_bw[OnuIdx].Llid, 1, 1);
			#else
			REMOTE_PASONU_uni_set_port( PonPortIdx, onu_up_bw[OnuIdx].Llid, 1, 1);
			#endif
			
			VOS_TaskUnlock();
		}
	}

	for(OnuIdx=0; OnuIdx<MAXONUPERPON;OnuIdx++)
	{
		if(onu_up_bw[OnuIdx].delay != 0)
		{
			for(i=0; i<onu_up_bw[OnuIdx].delay; i++ );
			VOS_TaskLock();
			SetOnuUplinkBW_2( PonPortIdx, OnuIdx, onu_up_bw[OnuIdx].UplinkClass, 1, onu_up_bw[OnuIdx].fixed, onu_up_bw[OnuIdx].assured_bw, onu_up_bw[OnuIdx].best_effort_bw );
			VOS_TaskUnlock();
		}
	}

	return(ROK);
}


int  pon_force_switch_enable = V2R1_DISABLE;
extern ULONG switchover_test_oos_time_delay;
extern ULONG switchover_test_master_active_slot;
extern int swBoardProtectedSwitch_EventReport( ulong_t devIdx, ulong_t brdIdx );
void FlashAllOnuDataPath()
{
	short int PonPortIdx;
	short int OnuIdx;
	short int Llid;
	if( switchover_test_oos_time_delay != 0 )
	{
		for( PonPortIdx=0; PonPortIdx<MAXPONCHIP; PonPortIdx++)
		{
			for(OnuIdx=0; OnuIdx<MAXONUPERPON;OnuIdx++)
			{
				if(GetOnuOperStatus(PonPortIdx,OnuIdx) != ONU_OPER_STATUS_UP )continue;
				Llid = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
				if(Llid != INVALID_LLID)
				{
                    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
					CTC_SetOnuUniPort( PonPortIdx, Llid, 1, 0 );
					#else
					REMOTE_PASONU_uni_set_port( PonPortIdx, Llid, 1, 0 );
					#endif
					
				}
			}
		}

		VOS_TaskDelay( switchover_test_oos_time_delay );

		for( PonPortIdx=0; PonPortIdx<MAXPONCHIP; PonPortIdx++)
		{
			for(OnuIdx=0; OnuIdx<MAXONUPERPON;OnuIdx++)
			{
				if(GetOnuOperStatus(PonPortIdx,OnuIdx) != ONU_OPER_STATUS_UP )continue;
				Llid = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
				if(Llid != INVALID_LLID)
				{
                    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
					CTC_SetOnuUniPort( PonPortIdx, Llid, 1, 1);
					#else
					REMOTE_PASONU_uni_set_port( PonPortIdx, Llid, 1, 1);
					#endif
					
				}
			}
		}
	}
	VOS_TaskDelay( 500 );
	swBoardProtectedSwitch_EventReport( 1, switchover_test_master_active_slot );
}

unsigned int g_CheckPonSignalLoss_Task_Id=0;
int  pon_aps_flag = 1;
extern int autoProtectSwitch_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx );
void ScanPonPortSignalLoss()
{
	int slot, port;
	short int PonPortIdx;
	short int PonPortIdx_Swap;
	UCHAR ponLosFlag[20];

	VOS_MemZero( ponLosFlag, sizeof(ponLosFlag) );

	while(pon_force_switch_enable == V2R1_DISABLE)
	{
		for(slot = PONCARD_FIRST; slot <= PONCARD_LAST; slot++)
		{
			for(port=1;port<=PONPORTPERCARD;port++)
			{
				if( GetOltCardslotInserted(slot) != CARDINSERT ) continue;
				PonPortIdx = GetPonPortIdxBySlot(slot, port);
				if(OLTAdv_IsExist(PonPortIdx) != TRUE) continue;
				if( PonPortSwapEnableQuery( PonPortIdx ) != V2R1_PON_PORT_SWAP_ENABLE ) continue;
				if( PonPortHotStatusQuery(PonPortIdx) != V2R1_PON_PORT_SWAP_ACTIVE ) continue;
				if( PonPortSwapPortQuery(PonPortIdx, &PonPortIdx_Swap ) != ROK ) continue;
				if( CheckPonPortSignal(PonPortIdx_Swap) == V2R1_DISABLE)
				{
					if( ponLosFlag[PonPortIdx_Swap] == 0 )
					{
						FlashOnuDataPath(PonPortIdx);
						autoProtectSwitch_EventReport( 1, slot+1, port+1 );
						ponLosFlag[PonPortIdx_Swap] = 1;
					}
					pon_aps_flag = 1;
				}
				else
				{
					pon_aps_flag = 0;
					ponLosFlag[PonPortIdx_Swap] = 0;
				}
			}
		}
		VOS_TaskDelay(20);
	}

	g_CheckPonSignalLoss_Task_Id = 0;
    
    VOS_TaskExit(0);

	return;
}

void  V2R1CheckPonSignalLoss(void)
{
	if( (pon_force_switch_enable == V2R1_DISABLE) && (g_CheckPonSignalLoss_Task_Id == 0) )
	{
		g_CheckPonSignalLoss_Task_Id = VOS_TaskCreate( (char *)"tCheckPonSignal", (ULONG)250, ScanPonPortSignalLoss, (ULONG *)NULL );
		if(g_CheckPonSignalLoss_Task_Id == 0)
			sys_console_printf("create CheckPonSignal failed\r\n");
	}
	return;
}

#endif

/* added by chenfj 2008-12-19
	用于记录配置了保护切换PON 端口的激活状态, 供下次启动时使用
	使用nvram 0xd4-0xd8 5个字节, 每个字节表示4个PON口
	对GFA6700:
	0xd4 -- PON 8/1 -- 8/4
	0xd5 -- PON 7/1 -- 7/4
	......
	0xd8 -- PON 4/1 -- 4/4
	对GFA6100:
	0xd4 -- PON 2/1--2/2
	0xd5 -- PON 3/1--3/2

	在每个字节中, 用两位表示一个PON口, 高位表示PON 是否配置了保护切换, 
	低位表示PON 口当前是激活状态, 还是备用状态
	
	*/

#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
static unsigned int PonIdToNvramOffset(short int PonPortIdx)
{
	return (int )(PonPortIdx/PONPORTPERCARD);
}

int SetPonPortAutoProtectAcitve(short int PonPortIdx)
{
	unsigned int  Offset;
	char data;
	short int PonId;
	CHECK_PON_RANGE

	Offset = PonIdToNvramOffset(PonPortIdx);
	data = *(unsigned char *)(AUTO_PROTECT_STATUS_BASE+Offset);
	
	PonId = PonPortIdx % PONPORTPERCARD;
	*(unsigned char *)(AUTO_PROTECT_STATUS_BASE+Offset) =  data | (AUTO_PROTECT_AND_ACTIVE <<(PonId <<1));
	return(ROK);
}

int SetPonPortAutoProtectPassive(short int PonPortIdx)
{
	unsigned int  Offset;
	char data;
	short int PonId;
	CHECK_PON_RANGE

	Offset = PonIdToNvramOffset(PonPortIdx);
	data = *(unsigned char *)(AUTO_PROTECT_STATUS_BASE+Offset);
	
	PonId = PonPortIdx % PONPORTPERCARD;
	*(unsigned char *)(AUTO_PROTECT_STATUS_BASE+Offset) = data & (BYTE_FF - (AUTO_PROTECT_AND_ACTIVE << (PonId<<1)));
	data = *(unsigned char *)(AUTO_PROTECT_STATUS_BASE+Offset);
	*(unsigned char *)(AUTO_PROTECT_STATUS_BASE+Offset) = data |(AUTO_PROTECT_AND_PASSIVE << (PonId<<1));
	return(ROK);
}

int GetPonPortAutoProtectStatus(short int PonPortIdx)
{
	unsigned int  Offset;
	char data;
	short int PonId;
	CHECK_PON_RANGE

	Offset = PonIdToNvramOffset(PonPortIdx);
	data = *(unsigned char *)(AUTO_PROTECT_STATUS_BASE+Offset);
	
	PonId = PonPortIdx % PONPORTPERCARD;
	data = (data & (3 <<  (PonId <<1))) >> ( PonId << 1 );
	return data;
}

int ClearPonPortAutoPoetect(short int PonPortIdx)
{
	unsigned int  Offset;
	char data;
	short int PonId;
	CHECK_PON_RANGE

	Offset = PonIdToNvramOffset(PonPortIdx);
	data = *(unsigned char *)(AUTO_PROTECT_STATUS_BASE+Offset);
	
	PonId = PonPortIdx % PONPORTPERCARD;
	*(unsigned char *)(AUTO_PROTECT_STATUS_BASE+Offset) = data & (BYTE_FF - (AUTO_PROTECT_AND_ACTIVE << (PonId<<1)));
	return(ROK);
}


#endif
#endif

	/* added by chenfj 2007-7-6
	     注: 在PAS5001上，onu authentication是由软件来实现的；在PAS5201上，若启动CTC协议栈，onu authentication是由PON芯片硬件实现的；若没有启动CTC协议栈，onu authentication 也是由 软件来实现
	*/
#ifdef  ONU_AUTHENTICATION_CONTROL
#endif
int  ShowPonPortAuthOnuEntry( short int PonPortIdx,  struct vty *vty )
{
	unsigned long Slot, Port, Entry=0;
	unsigned long NextSlot, NextPort;
	unsigned long NextEntry;
	STATUS ret;
	unsigned char MacAddr[BYTES_IN_MAC_ADDRESS];
	unsigned long length;
	
	CHECK_PON_RANGE

	Slot = GetCardIdxByPonChip(PonPortIdx);
	if( Slot == RERROR ) return( RERROR );
	Port = GetPonPortByPonChip(PonPortIdx);

	ret = getNextOnuAuthEntry( Slot, Port, Entry, &NextSlot, &NextPort, &NextEntry );
	
	if(( ret == VOS_ERROR ) || (( ret == VOS_OK ) && ((Slot != NextSlot ) || ( Port != NextPort ))))
		{
		vty_out( vty, "  %% pon %d/%d Authentication onu table is null\r\n", Slot, Port );
		return( RERROR );
		}
	
	if((NextEntry < 1 ) || (NextEntry > MAXONUPERPON ))
		{
		vty_out(vty,"  %% get authentication onu table Index err\r\n");
		return( ROK );
		}

	vty_out(vty, "  pon %d/%d Authentication Onu table:\r\n", Slot, Port );

	/*VOS_TaskLock();*/
	do{		
		Entry = NextEntry;
		ret = getOnuAuthMacAddress( Slot, Port, Entry, MacAddr, &length );
		vty_out( vty, "  %d    %02x%02x.%02x%02x.%02x%02x\r\n", Entry, MacAddr[0],MacAddr[1],MacAddr[2],MacAddr[3],MacAddr[4],MacAddr[5] );
		ret =  getNextOnuAuthEntry( Slot, Port, Entry, &NextSlot, &NextPort, &NextEntry );
		
		}while(( ret == VOS_OK ) && ( Slot == NextSlot ) && ( Port == NextPort ) );

	/*VOS_TaskUnlock();*/

	return( ROK );

}


int  ShowPonPortAuthOnuEntryALL( struct vty *vty )
{
	unsigned long Slot, Port, Entry=0;
	unsigned long NextSlot, NextPort;
	unsigned long NextEntry;
	STATUS ret;
	unsigned char MacAddr[BYTES_IN_MAC_ADDRESS];
	unsigned long length;
	ULONG entryCounter = 0;
	/*short int PonPortIdx;*/

	
	for( Slot = PONCARD_FIRST; Slot <= PONCARD_LAST; Slot ++ )
		{
		for( Port = 1; Port <= PONPORTPERCARD; Port ++ )
			{
			Entry = 0;
			ret = getNextOnuAuthEntry( Slot, Port, Entry, &NextSlot, &NextPort, &NextEntry );
			
			if( ret == VOS_ERROR)                /*问题单8824*/
				{
				/*vty_out(vty,"onu-register authentication entry is NULL!\r\n");
				return VOS_ERROR;*/
				continue;	/* modified by xieshl 20100224 问题单9870 */
				}
			
			if(/*( ret == VOS_ERROR ) || */(( ret == VOS_OK ) && ((Slot != NextSlot ) || ( Port != NextPort ))))
				{
				continue;
				}
			
			if((NextEntry < 1 ) || (NextEntry > MAXONUPERPON ))
				{
				continue;
				}

			vty_out(vty, " pon %d/%d Authentication Onu table:\r\n", Slot, Port );

			do{				
				Entry = NextEntry;
				ret = getOnuAuthMacAddress( Slot, Port, Entry, MacAddr, &length );
				vty_out( vty, "  %d    %02x%02x.%02x%02x.%02x%02x\r\n", Entry, MacAddr[0],MacAddr[1],MacAddr[2],MacAddr[3],MacAddr[4],MacAddr[5] );
				ret =  getNextOnuAuthEntry( Slot, Port, Entry, &NextSlot, &NextPort, &NextEntry );

				entryCounter++;
				
				}while(( ret == VOS_OK ) && ( Slot == NextSlot ) && ( Port == NextPort ) );
			
			}
		}

		if( entryCounter == 0 )
			vty_out(vty,"onu-register authentication entry is NULL!\r\n");

	return( ROK );

}

int AddMacAuthenticationAll( short int PonPortIdx )
{
	unsigned long Slot, Port, Entry=0;
	unsigned long NextSlot, NextPort;
	unsigned long NextEntry;
	short int  ret;
	unsigned char MacAddr[BYTES_IN_MAC_ADDRESS];
	unsigned long length;

	CHECK_PON_RANGE

	Slot = GetCardIdxByPonChip(PonPortIdx);
	if( Slot == RERROR ) return( RERROR );
	Port = GetPonPortByPonChip(PonPortIdx);

	ret = getNextOnuAuthEntry( Slot, Port, Entry, &NextSlot, &NextPort, &NextEntry );
	
	if(( ret == VOS_ERROR ) || (( ret == VOS_OK ) && ((Slot != NextSlot ) || ( Port != NextPort ))))
		{
		return( ROK );
		}
	
	if(( NextEntry < 1 ) || ( NextEntry > MAXONUPERPON ))
		{
		return( ROK );
		}

	/*VOS_TaskLock();*/
	do{
		
		Entry = NextEntry;
		/*ret = getOnuAuthMacAddress( Slot, Port, Entry, MacAddr, &length );*/	
		ret = getOnuAuthMacAddress( Slot, Port, Entry, MacAddr, &length );
		if( ret == VOS_OK )AddMacAuthentication( PonPortIdx, MacAddr );
		
		ret =  getNextOnuAuthEntry( Slot, Port, Entry, &NextSlot, &NextPort, &NextEntry );
		
		}while(( ret == VOS_OK ) && ( Slot == NextSlot ) && ( Port == NextPort ) );

	/*VOS_TaskUnlock();*/
	
	return( ROK );

}

/*  清除PON端口下ONU Auth 数据*/
int  DeletePonPortAuthEntryAll( short int PonPortIdx )
{
	unsigned long Slot, Port, Entry=0;
	unsigned long NextSlot, NextPort;
	unsigned long NextEntry;
	STATUS ret;
	/*unsigned char MacAddr[BYTES_IN_MAC_ADDRESS];
	  unsigned long length;*/

	CHECK_PON_RANGE

		Slot = GetCardIdxByPonChip(PonPortIdx);
	if( Slot == RERROR ) return( RERROR );
	Port = GetPonPortByPonChip(PonPortIdx);

	ret = getNextOnuAuthEntry( Slot, Port, Entry, &NextSlot, &NextPort, &NextEntry );

	if(( ret == VOS_ERROR ) || (( ret == VOS_OK ) && ((Slot != NextSlot ) || ( Port != NextPort ))))
	{
		return( ROK );
	}

	if(( NextEntry < 1 ) || ( NextEntry > MAXONUPERPON ))
	{
		return( ROK );
	}

	/*VOS_TaskLock();*/
	do{

		Entry = NextEntry;
		/*ret = getOnuAuthMacAddress( Slot, Port, Entry, MacAddr, &length );*/	
		ret =  getNextOnuAuthEntry( Slot, Port, Entry, &NextSlot, &NextPort, &NextEntry );
		setOnuAuthStatus( Slot, Port, Entry, V2R1_ENTRY_DESTORY );
	}while(( ret == VOS_OK ) && ( Slot == NextSlot ) && ( Port == NextPort ) );

	/*VOS_TaskUnlock();*/
	return( ROK );

}
int OnuAuth_ActivatePendingOnuBySnMsg( short int PonPortIdx, unsigned char *SN )
{
	unsigned long aulMsg[4] = { MODULE_PON, FC_ACTIVE_FENDING_ONU_BY_SN, 0, 0};
	unsigned char *Addr_buf;

	CHECK_PON_RANGE

	if( SN == NULL ) return( RERROR );

	aulMsg[2] = PonPortIdx;

	Addr_buf = VOS_Malloc( GPON_ONU_SERIAL_NUM_STR_LEN,  MODULE_PON );
	if( Addr_buf == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}

	VOS_MemCpy( Addr_buf, SN, GPON_ONU_SERIAL_NUM_STR_LEN );

	aulMsg[3] = (int )Addr_buf;

	/*
	if( g_PonUpdate_Queue_Id == 0 )  g_PonUpdate_Queue_Id = VOS_QueCreate( MAXPONUPDATEMSGNUM, VOS_MSG_Q_FIFO);
	*/

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_Free(Addr_buf);
		VOS_ASSERT(0);
		/*sys_console_printf("  error:VOS send message err to ponUpdate Que\r\n"  );*/
		}
	return( ROK );
}

int OnuAuth_ActivatePendingOnuByMacAddrMsg( short int PonPortIdx, unsigned char *MacAddr )
{
	unsigned long aulMsg[4] = { MODULE_PON, FC_ACTIVE_FENDING_ONU_BY_MAC, 0, 0};
	unsigned char *Addr_buf;

	CHECK_PON_RANGE

	if( MacAddr == NULL ) return( RERROR );

	aulMsg[2] = PonPortIdx;

	Addr_buf = VOS_Malloc( BYTES_IN_MAC_ADDRESS,  MODULE_PON );
	if( Addr_buf == NULL )
		{
		ASSERT(0);
		return( RERROR );
		}

	VOS_MemCpy( Addr_buf, MacAddr, BYTES_IN_MAC_ADDRESS );

	aulMsg[3] = (int )Addr_buf;

	/*
	if( g_PonUpdate_Queue_Id == 0 )  g_PonUpdate_Queue_Id = VOS_QueCreate( MAXPONUPDATEMSGNUM, VOS_MSG_Q_FIFO);
	*/

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_Free(Addr_buf);
		VOS_ASSERT(0);
		/*sys_console_printf("  error:VOS send message err to ponUpdate Que\r\n"  );*/
		}
	return( ROK );
}



int OnuAuth_ActivateOnePendingOnu( unsigned long slot, unsigned long port, unsigned char *MacAddr )
{
	short int PonPortIdx;
	/*pendingOnu_S *CurOnu, *NextOnu;*/

	if( MacAddr == NULL ) return( RERROR );

	PonPortIdx = GetPonPortIdxBySlot((short int) slot, (short int) port);
	
	return(OnuAuth_ActivatePendingOnuByMacAddrMsg(PonPortIdx, MacAddr));

}

int OnuAuth_DeregisterOnu(unsigned long slot, unsigned long port, unsigned char *MacAddr )
{
	short int PonPortIdx;
	short int OnuIdx;
	int ret;

	if( MacAddr == NULL ) return( RERROR );
	PonPortIdx = GetPonPortIdxBySlot( slot, port);

	if( PonPortIdx == RERROR ) return( RERROR );
	
	ret = GetOnuOperStatusByMacAddr( PonPortIdx, &OnuIdx, MacAddr );
	if( ret == ONU_OPER_STATUS_UP )
		Onu_deregister(PonPortIdx, OnuIdx );

	return( ROK );
}

int  AddRegisterOnuToAuthEntry( short int PonPortIdx )
{
	short int OnuIdx;
	unsigned long ul_slot, ul_port;
	unsigned long TableIdx;
	unsigned char *MacAddr;
	STATUS ret;
	LogicEntity *le;
	CHECK_PON_RANGE;

	/*ONU_MGMT_SEM_TAKE;*/
	for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++ )
	{
		if( GetOnuOperStatus( PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP )
		{
			MacAddr = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].DeviceInfo.MacAddr;
			ul_slot = GetCardIdxByPonChip(PonPortIdx);
			ul_port = GetPonPortByPonChip(PonPortIdx);
			
			ret = isHaveAuthMacAddress(ul_slot, ul_port, MacAddr , &TableIdx );

	 		if( ret == VOS_OK ) 
            {
                if ( NULL != (le = getLogicEntityBySlot(ul_slot, ul_port, TableIdx)) )
                {
                    le->onu_auth_status = 0;
                    VOS_MemSet( le->onu_auth_mac, 0, 6 );
                }
            }           	 
	 	
			 /* modified by xieshl 20110804, 自动生成认证表时，主控和PON板是相互独立的，为了确保表现索引一致，
			     统一采用onuId为索引创建新行，以避免在维护过程中产生和主控不一致的情况，问题单12878 */
			TableIdx = OnuIdx + 1;
			ret = setOnuAuthStatus( ul_slot, ul_port, TableIdx, V2R1_ENTRY_CREATE_AND_GO);
			if( ret != VOS_OK ) continue;
		
#if 0
			sys_console_printf( " pon %d/%d TableIdx = %d\r\n", ul_slot, ul_port, TableIdx );
			sys_console_printf( " mac addr: %02x%02x.%02x%02x.%02x%02x\r\n",MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5] );
#endif	
			setOnuAuthMacAddress( ul_slot, ul_port, TableIdx, MacAddr );
	
	 	}
 	}
	/*ONU_MGMT_SEM_GIVE;*/
	return( ROK );

}

int  OnuAuth_DeregisterAllOnu( short int PonPortIdx )
{

	CHECK_PON_RANGE

	PONTx_Disable(PonPortIdx, PONPORT_TX_ACTIVED);

	VOS_TaskDelay(VOS_TICK_SECOND);	

	/* added by chenfj 2007-8-6 
	     在将PON端口ONU认证模式设置为全部认证时，清除软件/硬件认证表
	PAS_clear_mac_address_authentication_table( PonPortIdx );
	DeletePonPortAuthEntryAll(PonPortIdx );	
	*/
	PONTx_Enable(PonPortIdx, PONPORT_TX_ACTIVED);

	return( ROK );

}


#ifdef PON_PORT_ADMIN_STATUS 
#endif
/* modified by chenfj 2008-5-26
     若PON口配置了保护倒换,且当前为备用状态, 则在使能端口(undo shutdown pon port)
     时只保存配置, 不直接使能; 待PON 发生倒换时,再使能. 否则,会导致PON 口切换
     */
int  PonPortAdminStatusEnable( short int PonPortIdx )
{
	int ret;
    
	CHECK_PON_RANGE

#if 0
	if( Olt_exists(PonPortIdx))
	{
		if(( PonPortSwapEnableQuery( PonPortIdx ) == V2R1_PON_PORT_SWAP_ENABLE )
			&& (PonPortHotStatusQuery(PonPortIdx) == V2R1_PON_PORT_SWAP_PASSIVE))
		{
		}
		else
			 PONTx_Enable( PonPortIdx );
	}
#endif

    ret = OLT_SetAdminStatus(PonPortIdx, V2R1_ENABLE);
    if ( OLT_CALL_ISOK(ret) )
    {
        ret = ROK;
		/*Begin:for onu swap by jinhl@2013-04-27*/
		if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
		{
			(void)ActivePonPort_RegVirtual(PonPortIdx);
		}
		/*End:for onu swap by jinhl@2013-04-27*/
		OLTAdv_NotifyOltValid(PonPortIdx);//added by wangjiah@2017-06-22 to change pon port status of logical port
    }
#if 0
	if( ret == ROK )
		PonPortTable[PonPortIdx].PortAdminStatus = V2R1_ENABLE;
#endif
	
	return( ret );
}

int  PonPortAdminStatusDisable( short int PonPortIdx )
{
	int ret;
	CHECK_PON_RANGE

#if 0
	if( Olt_exists(PonPortIdx) )
		{
		/*DeregisterAllOnu(PonPortIdx);*/
		ret = PONTx_Disable( PonPortIdx );
		}
#endif

    ret = OLT_SetAdminStatus(PonPortIdx, V2R1_DISABLE);
    if ( OLT_CALL_ISOK(ret) )
    {
    	OLTAdv_NotifyOltInvalid(PonPortIdx);//added by wangjiah@2017-06-22 to change pon port status of logical port
        ret = ROK;
    }
#if 0
	if( ret == ROK )
		PonPortTable[PonPortIdx].PortAdminStatus = V2R1_DISABLE;
#endif
	return( ret );
}

int SetPonPortAddressTableFullHandlingMode(short int  PonPortIdx, unsigned char removed_when_full)
{
	short int PonChipType;
	
	CHECK_PON_RANGE

	if(( removed_when_full != TRUE ) && ( removed_when_full != FALSE ))
	{
		return( RERROR );
	}
	PonPortTable[PonPortIdx].table_full_handle_mode = removed_when_full;
	PonChipType = V2R1_GetPonchipType(PonPortIdx );
	if(OLT_PONCHIP_ISPAS(PonChipType))
		{
		if( !OLT_PONCHIP_ISPAS5001(PonChipType))
			{
			OLT_SetAddressTableFullHandlingMode( PonPortIdx, removed_when_full );	
			return( ROK );
			}
		else 
			{
			}
		}
	else { /* other pon chip type handler */

		}

	return( RERROR );

}


/* added by chenfj 2007-12-28 
	设置PON 芯片上下行最大帧长; 当前仅支持PAS5201
	此功能在PAS-SOFT V5.1.26 之后版本上支持
	*/

int SetPas5201frame_size_limits(short int PonPortIdx, unsigned int jumbo_length)
{
#if 1
    return OLT_SetPonFrameSizeLimit(PonPortIdx, (short int)jumbo_length);
#else
	CHECK_PON_RANGE
	if((jumbo_length < PON_MIN_HW_ETHERNET_FRAME_SIZE )
		||(jumbo_length > PON_MAX_HW_ETHERNET_FRAME_SIZE ))
		return( RERROR );
    
	if(jumbo_length <= (PON_MAX_HW_ETHERNET_FRAME_SIZE -4))
		tag_length = jumbo_length + 4;
	else tag_length = jumbo_length;
	untag_length = jumbo_length;

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	
	if( PonChipType == PONCHIP_PAS5201 )
		{
		ret_pas = PAS_set_frame_size_limits(PonPortIdx, PON_DIRECTION_UPLINK, untag_length, tag_length);
		if( ret_pas != PAS_EXIT_OK )
			return(RERROR);
		ret_pas = PAS_set_frame_size_limits(PonPortIdx, PON_DIRECTION_DOWNLINK, untag_length, tag_length);
		if( ret_pas != PAS_EXIT_OK )
			return(RERROR);
		return( ROK );
		}
	return( RERROR );
#endif
}

int GetPas5201frame_size_limits(short int PonPortIdx, unsigned int *jumbo_length)
{
	/*short int PonChipType;*/
	short int untag_length=0;
	/*short int tag_length=0;*/
	short int ret_pas;

	CHECK_PON_RANGE
	if( jumbo_length == NULL )
		return( RERROR );
	*jumbo_length = 0;

#if 1
	ret_pas = OLT_GetPonFrameSizeLimit(PonPortIdx, &untag_length);
	if( OLT_CALL_ISOK(ret_pas) )
	{
		*jumbo_length = untag_length;
		return(ROK);
	}
#else    
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	
	if( PonChipType == PONCHIP_PAS5201 )
	{
		ret_pas = PAS_get_frame_size_limits(PonPortIdx, PON_DIRECTION_UPLINK, &untag_length, &tag_length);
		if( ret_pas == PAS_EXIT_OK )
		{
			*jumbo_length = untag_length;
			return(ROK);
		}
	}
#endif

	return( RERROR );
}

#if 0
/* 清除PON 端口MAC 地址表*/
int addressflag = 0;
int  ResetPonPortMacTable(short int PonPortIdx )
{
	/*
	short int Active_num= 0;
	short int i = 0;
	*/
	short int PonChipType;
	
	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if( addressflag ==  0 )
		{
	if(PonChipType == PONCHIP_PAS5001 )
		PAS_reset_address_table(PonPortIdx, PON_ALL_ACTIVE_LLIDS,  ADDR_DYNAMIC);
	else 
		PAS_reset_address_table(PonPortIdx, PON_ALL_ACTIVE_LLIDS,  ADDR_DYNAMIC_AND_STATIC);
		}
	
	else{
	if(PonChipType == PONCHIP_PAS5001 )
		PAS_reset_address_table(PonPortIdx, PON_ALL_ACTIVE_LLIDS,  ADDR_DYNAMIC_AND_STATIC);
	else
		PAS_reset_address_table(PonPortIdx, PON_ALL_ACTIVE_LLIDS,  ADDR_DYNAMIC);
		}
	/*
	VOS_MemSet(Mac_addr_table, 0, sizeof(Mac_addr_table));
	PAS_get_address_table( PonPortIdx, &Active_num, Mac_addr_table);
	for( i = 0; i< Active_num; i++)
		{
		PAS_remove_address_table_record(PonPortIdx, Mac_addr_table[i].mac_address );
		}
	*/
	VOS_TaskDelay(50);
	return( ROK );
}

/*  设置PON 端口不学习MAC 地址
	请注意: PAS5001 不支持这个设置
*/
int  SetPonPortMACLearning(short int PonPortIdx, int LearningFlag )
{
	PON_address_table_config_t  add_table_config;
	short int ret;
	short int PonChipType;

	CHECK_PON_RANGE

	if((LearningFlag != V2R1_ENABLE ) && ( LearningFlag != V2R1_DISABLE )) return (RERROR );
	
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(PonChipType == PONCHIP_PAS5001 ) return (RERROR );
	
	ret = PAS_get_address_table_configuration(PonPortIdx,&add_table_config);
	if( ret != PAS_EXIT_OK ) return( RERROR );
	if(LearningFlag == V2R1_ENABLE )
		add_table_config.allow_learning = PON_DIRECTION_UPLINK;
	else
		add_table_config.allow_learning = PON_DIRECTION_NO_DIRECTION;
	ret = PAS_set_address_table_configuration(PonPortIdx, add_table_config);

	return( ROK );
}
#endif

int PonAbnormalHandler(short int PonPortIdx, int ErrCode)
{
	CHECK_PON_RANGE

	PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ERR;
	PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_DOWN;
	Trap_PonPortAbnormal(PonPortIdx, ErrCode );
	PonChipMgmtTable[PonPortIdx].Err_counter = 0;

	return( ROK );
}

#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
int GetPonUplinkVlanQinQ(short int PonPortIdx, short int OnuIdx, Olt_llid_vlan_manipulation *vlan_uplink_config)
{
    Olt_llid_vlan_manipulation *pstQinQ;

    OLT_LOCAL_ASSERT(PonPortIdx);
    ONU_ASSERT(OnuIdx);
    VOS_ASSERT(vlan_uplink_config);
	pstQinQ = &PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx];
	if( PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION != pstQinQ->vlan_manipulation )
	{
        VOS_MemCpy(vlan_uplink_config, pstQinQ, sizeof(Olt_llid_vlan_manipulation));
    }
    else
    {
        vlan_uplink_config->vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION;
    }

    return 0;
}

int SetPonUplinkVlanQinQ(short int PonPortIdx, short int OnuIdx, Olt_llid_vlan_manipulation *vlan_uplink_config)
{
    Olt_llid_vlan_manipulation *pstQinQ;

    OLT_LOCAL_ASSERT(PonPortIdx);
    ONU_ASSERT(OnuIdx);
    VOS_ASSERT(vlan_uplink_config);

	pstQinQ = &PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx];
    VOS_MemCpy(pstQinQ, vlan_uplink_config, sizeof(Olt_llid_vlan_manipulation));

    return 0;
}

int GetPonDownlinkVlanQinQ(short int PonPortIdx, short int DownVlanId, Olt_llid_vlan_manipulation *vlan_downlink_config)
{
    int iVlanManIdx;
    Olt_llid_vlan_manipulation *pstQinQ;

    OLT_LOCAL_ASSERT(PonPortIdx);
    VLAN_ASSERT(DownVlanId);
    VOS_ASSERT(vlan_downlink_config);
	if( 0 != (iVlanManIdx = PonPortTable[PonPortIdx].Downlink_vlan_Config[DownVlanId]) )
	{
		pstQinQ = &PonPortTable[PonPortIdx].Downlink_vlan_manipulation[iVlanManIdx];
        VOS_MemCpy(vlan_downlink_config, pstQinQ, sizeof(Olt_llid_vlan_manipulation));
    }
    else
    {
        vlan_downlink_config->vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION;
    }

    return 0;
}

int SetPonDownlinkVlanQinQ(short int PonPortIdx, short int DownVlanId, Olt_llid_vlan_manipulation *vlan_downlink_config)
{
    unsigned char OldVlanManIdx;
    unsigned char DstVlanManIdx;
    Olt_llid_vlan_manipulation *pstDownlinkVlanManipulation;

    OLT_LOCAL_ASSERT(PonPortIdx);
    VLAN_ASSERT(DownVlanId);
    VOS_ASSERT(vlan_downlink_config);

    if ( 0 != (OldVlanManIdx = PonPortTable[PonPortIdx].Downlink_vlan_Config[DownVlanId]) )
    {
        VOS_ASSERT(OldVlanManIdx < MAX_VID_DOWNLINK);
        pstDownlinkVlanManipulation = &PonPortTable[PonPortIdx].Downlink_vlan_manipulation[OldVlanManIdx];

        if ( pstDownlinkVlanManipulation->vlan_manipulation == vlan_downlink_config->vlan_manipulation )
        {
            if ( (pstDownlinkVlanManipulation->new_vlan_id == vlan_downlink_config->new_vlan_id)
                  && (pstDownlinkVlanManipulation->new_priority == vlan_downlink_config->new_priority) )
            {
                /* 重复配置 */           
        		return(ROK);
            }
            else if ( PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG == pstDownlinkVlanManipulation->vlan_manipulation )
            {
                /* 暂存配置 */
                pstDownlinkVlanManipulation->new_vlan_id  = vlan_downlink_config->new_vlan_id;
                pstDownlinkVlanManipulation->new_priority = vlan_downlink_config->new_priority;
        		return(ROK);
            }
            else if ( (PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID == pstDownlinkVlanManipulation->vlan_manipulation) && (pstDownlinkVlanManipulation->new_vlan_id == vlan_downlink_config->new_vlan_id) )
            {
                /* 暂存配置 */
                pstDownlinkVlanManipulation->new_priority = vlan_downlink_config->new_priority;
        		return(ROK);
            }
            else
            {}
        }

        {
            /* 配置更改 */   
            
            /* 目标规则查找 */
            if ( 0 == (DstVlanManIdx = GetDownlinkVlanManipulation(PonPortIdx, vlan_downlink_config)) )
            {
                /* 没找到相同的目标规则 */
                if ( 1 == pstDownlinkVlanManipulation->original_vlan_id )
                {
                    /* 只有本VLAN占用此规则，直接更改规则即可 */           
                    VOS_MemCpy(pstDownlinkVlanManipulation, vlan_downlink_config, sizeof(Olt_llid_vlan_manipulation));
                    
            		return(ROK);
                }
                else
                {
                    /* 创建新的目标规则 */
                    if ( 0 == (DstVlanManIdx = CreateDownlinkVlanManipulation(PonPortIdx, vlan_downlink_config)) )
                    {
                        return RERROR;
                    }
                }
            }

            ReleaseDownlinkVlanManipulation(PonPortIdx, OldVlanManIdx);                        
        }
    }
    else
    {
        /* 此VLAN原本无规则 */
    
        /* 目标规则查找 */
        if ( 0 == (DstVlanManIdx = GetDownlinkVlanManipulation(PonPortIdx, vlan_downlink_config)) )
        {
            /* 没找到相同的目标规则，创建新的目标规则 */
            if ( 0 == (DstVlanManIdx = CreateDownlinkVlanManipulation(PonPortIdx, vlan_downlink_config)) )
            {
                return RERROR;
            }
        }

        /* 更新VLAN的最大操作范围 */
        if ( DownVlanId > PonPortTable[PonPortIdx].Downlink_vlan_Cfg_Max )
        {
            PonPortTable[PonPortIdx].Downlink_vlan_Cfg_Max = DownVlanId;
        }
    }

    PonPortTable[PonPortIdx].Downlink_vlan_Config[DownVlanId] = DstVlanManIdx;

	return(ROK);
}

/*  将上行方向ONU VLAN 操作配置数据保存到数据库中
*/
int  SavePonUplinkVlanManipulation(short int PonPortIdx, short int OnuIdx, PON_olt_vlan_uplink_config_t *vlan_uplink_config)
{
	CHECK_ONU_RANGE
	PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].vlan_manipulation = vlan_uplink_config->vlan_manipulation;
	PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].original_vlan_id = vlan_uplink_config->authenticated_vid;
	PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].new_vlan_id = vlan_uplink_config->new_vlan_tag_id;
	PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].new_priority = vlan_uplink_config->vlan_priority;
	PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].ethernet_type = vlan_uplink_config->vlan_type;
	return(ROK);
}

/*  将下行行方向ONU VLAN 操作配置数据保存到数据库中
*/
unsigned char FindFreeEntryForDownlinkVlanManipulation(short int PonPortIdx)
{
	unsigned char n;

	for(n=2; n<MAX_VID_DOWNLINK; n++)
	{
		if (0 == PonPortTable[PonPortIdx].Downlink_vlan_manipulation[n].vlan_manipulation)
			return n;
	}

	return 0;
}

/* B--added by liwei056@2010-5-25 for D10163 */
unsigned char CreateDownlinkVlanManipulation(short int PonPortIdx, Olt_llid_vlan_manipulation *vlan_Downlink_config)
{
    unsigned char n;
    Olt_llid_vlan_manipulation *pstDownlinkVlanManipulation;

    if ( 0 != (n = FindFreeEntryForDownlinkVlanManipulation(PonPortIdx)) )
    {
        if ( n > PonPortTable[PonPortIdx].Downlink_vlan_Man_Max )
        {
            PonPortTable[PonPortIdx].Downlink_vlan_Man_Max = n;
        }
        
        pstDownlinkVlanManipulation = &PonPortTable[PonPortIdx].Downlink_vlan_manipulation[n];

        VOS_MemCpy(pstDownlinkVlanManipulation, vlan_Downlink_config, sizeof(Olt_llid_vlan_manipulation));
        pstDownlinkVlanManipulation->original_vlan_id = 1;
    }

    return n;       
}

void ReleaseDownlinkVlanManipulation(short int PonPortIdx, unsigned char vlan_Downlink_idx)
{
    Olt_llid_vlan_manipulation *pManipulation = &PonPortTable[PonPortIdx].Downlink_vlan_manipulation[vlan_Downlink_idx];

    VOS_ASSERT(vlan_Downlink_idx <= PonPortTable[PonPortIdx].Downlink_vlan_Man_Max);
    
    if ( 0 == (--pManipulation->original_vlan_id) )
    {
        if (vlan_Downlink_idx > 1)
        {
            VOS_MemZero(pManipulation, sizeof(Olt_llid_vlan_manipulation));

            /* 更新规则的最大操作范围 */
            if (vlan_Downlink_idx == PonPortTable[PonPortIdx].Downlink_vlan_Man_Max)
            {
                int i;
                
                for (i=vlan_Downlink_idx-1; i>1; i--)
                {
                    if ( 0 != PonPortTable[PonPortIdx].Downlink_vlan_manipulation[i].vlan_manipulation )
                    {
                        break;
                    }
                }
                
                PonPortTable[PonPortIdx].Downlink_vlan_Man_Max = i;
            }
        }
    }
}

unsigned char GetDownlinkVlanManipulation(short int PonPortIdx, Olt_llid_vlan_manipulation *vlan_Downlink_config)
{
	int i, m;
    Olt_llid_vlan_manipulation *pstDownlinkVlanManipulation;

    m = PonPortTable[PonPortIdx].Downlink_vlan_Man_Max;
    if (PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG == vlan_Downlink_config->vlan_manipulation)
    {
        /* 下行删除VLAN都公用第一条规则 */
        i = 1;
    }
    else
    {
    	for(i=2; i<=m; i++)
    	{
    	    pstDownlinkVlanManipulation = &PonPortTable[PonPortIdx].Downlink_vlan_manipulation[i];
    	    if ( 0 != pstDownlinkVlanManipulation->vlan_manipulation )
            {
                if (pstDownlinkVlanManipulation->new_vlan_id == vlan_Downlink_config->new_vlan_id)
                {
                    if ( pstDownlinkVlanManipulation->vlan_manipulation == vlan_Downlink_config->vlan_manipulation )
                    {
                        if (PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID == vlan_Downlink_config->vlan_manipulation)
                        {
                            /* 下行VLAN相同的替换规则找到 */
                            break;
                        }
                        else if (PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID_AND_PRIORITY == vlan_Downlink_config->vlan_manipulation)
                        {
                            if (pstDownlinkVlanManipulation->new_priority == vlan_Downlink_config->new_priority)
                            {
                                /* 下行VLAN相同的替换规则找到 */
                                break;
                            }
                        }
                        else
                        {}
                    }
                }
            }   
    	}
    }   

    if ( i <= m )
    {
        /* 增加规则的引用计数 */
        PonPortTable[PonPortIdx].Downlink_vlan_manipulation[i].original_vlan_id++;
    }
    else
    {
        i = 0;
    }
    
    return i;
}
/* E--added by liwei056@2010-5-25 for D10163 */

int SavePonDownlinkVlanManipulation(short int PonPortIdx, PON_vlan_tag_t original_vlan, PON_olt_vid_downlink_config_t *vlan_Downlink_config)
{
    Olt_llid_vlan_manipulation DownlinkVlanManipulation;
	
	CHECK_PON_RANGE
    VOS_ASSERT(original_vlan < 4096);

    DownlinkVlanManipulation.vlan_manipulation = vlan_Downlink_config->vlan_manipulation;
    DownlinkVlanManipulation.new_vlan_id       = vlan_Downlink_config->new_vid;
    DownlinkVlanManipulation.new_priority      = vlan_Downlink_config->new_priority;
    DownlinkVlanManipulation.original_vlan_id  = 1;

    return SetPonDownlinkVlanQinQ(PonPortIdx, original_vlan, &DownlinkVlanManipulation);
}

int  DeletePonDownlinkVlanManipulation(short int PonPortIdx, PON_vlan_tag_t original_vlan)
{
	unsigned char Entry;
	
	CHECK_PON_RANGE	

    Entry = PonPortTable[PonPortIdx].Downlink_vlan_Config[original_vlan];
    if (Entry > 0)
    {
        PonPortTable[PonPortIdx].Downlink_vlan_Config[original_vlan] = 0;
        ReleaseDownlinkVlanManipulation(PonPortIdx, Entry);

        /* 更新VLAN的最大操作范围 */
        if ( original_vlan == PonPortTable[PonPortIdx].Downlink_vlan_Cfg_Max )
        {
            int i;
            
            for (i=original_vlan-1; i>0; i--)
            {
                if ( 0 != PonPortTable[PonPortIdx].Downlink_vlan_Config[i] )
                {
                    break;
                }
            }
            
            PonPortTable[PonPortIdx].Downlink_vlan_Cfg_Max = i;
        }
    }
    
	return(ROK);
}

int  ShowPonDownlinkVlanManipulation(short int PonPortIdx, short int VlanId, struct vty *vty)
{
	Olt_llid_vlan_manipulation  *CurrentPtr;
	short int Entry, i, m;
	unsigned char HaveUsedEntry;
	unsigned char ucVlanManIdx;

	CHECK_PON_RANGE

    if ( 0 > VlanId )
    {
        i = 1;

        Entry = 0;
        m = PonPortTable[PonPortIdx].Downlink_vlan_Cfg_Max;
    }
    else
    {
        i = 0;

        Entry = VlanId;
        m = Entry;
    }

    HaveUsedEntry = 0;
	for(; Entry<=m; Entry++)
	{
		if( 0 != (ucVlanManIdx = PonPortTable[PonPortIdx].Downlink_vlan_Config[Entry]) )
		{
			if(HaveUsedEntry == 0 )
			{
				vty_out(vty,"pon%d/%d downlink vlan manipulation\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
				HaveUsedEntry = 0x11;
			}
			CurrentPtr = &PonPortTable[PonPortIdx].Downlink_vlan_manipulation[ucVlanManIdx];

            if ( 0 < i )
            {
    			vty_out(vty,"rule %d:\r\n", i);
            }
            
			if((CurrentPtr->vlan_manipulation == PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID_AND_PRIORITY)
				||(CurrentPtr->vlan_manipulation == PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID))
			{
				vty_out(vty,"  vlan-tag-exchanged, from original-vid: ");
				if(Entry == NULL_VLAN)
					vty_out(vty,"null-tag");
				else if(Entry == VLAN_UNTAGGED_ID)
					vty_out(vty,"untagged");
				else
					vty_out(vty,"%d", Entry);
					
				vty_out(vty,", to new-vid %d", CurrentPtr->new_vlan_id);
				if(CurrentPtr->vlan_manipulation == PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID_AND_PRIORITY)
					vty_out(vty,",new-priority:%d", CurrentPtr->new_priority);
			}
			else if(CurrentPtr->vlan_manipulation == PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG)
			{
				vty_out(vty,"  vlan-tag-striped, vid=");
				if(Entry == NULL_VLAN)
					vty_out(vty,"null vlan");
				else if(Entry == VLAN_UNTAGGED_ID)
					vty_out(vty,"untagged");
				else
					vty_out(vty,"%d", Entry);
			}
			vty_out(vty,"\r\n");

			i++;
		}
        else
        {
            if ( 0 <= VlanId )
            {
    			vty_out(vty,"downlink vlan manipulation: no manipulation\r\n");
            }
        }
	}
    
	return(ROK);
}

/*  将数据库中保存的ONU VLAN 操作配置数据设置到PON 芯片中*/
int  CopyUplinkVlanManipulationToOnu(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx)
{
	OLT_vlan_qinq_t vlan_qinq_config;
	PON_olt_vlan_uplink_config_t     *vlan_uplink_config = &vlan_qinq_config.qinq_cfg.up_cfg;
	Olt_llid_vlan_manipulation  *CurrentPtr;
	PON_olt_vlan_uplink_manipulation_t uplink_vlan_manipulation;

	if(ThisIsValidOnu(DstPonPortIdx, DstOnuIdx) != ROK ) return(RERROR);
    if ( (SrcPonPortIdx != DstPonPortIdx)
        || (SrcOnuIdx != DstOnuIdx) )
    {
    	if(ThisIsValidOnu(SrcPonPortIdx, SrcOnuIdx) != ROK ) return(RERROR);
    }

	VOS_MemSet(&vlan_uplink_config, 0, sizeof(PON_olt_vlan_uplink_config_t));

	uplink_vlan_manipulation = PonPortTable[SrcPonPortIdx].Uplink_vlan_manipulation[SrcOnuIdx].vlan_manipulation;
    if ( uplink_vlan_manipulation != PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION )
	{
		CurrentPtr = &PonPortTable[SrcPonPortIdx].Uplink_vlan_manipulation[SrcOnuIdx];

		vlan_uplink_config->untagged_frames_authentication_vid = PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0;
		vlan_uplink_config->authenticated_vid = CurrentPtr->original_vlan_id;
		vlan_uplink_config->discard_untagged    = FALSE;
		vlan_uplink_config->discard_tagged      = FALSE;
		vlan_uplink_config->discard_null_tagged = FALSE;
		vlan_uplink_config->discard_nested      = FALSE;
		vlan_uplink_config->vlan_manipulation = uplink_vlan_manipulation;
		vlan_uplink_config->new_vlan_tag_id   = CurrentPtr->new_vlan_id;
		vlan_uplink_config->vlan_type         = CurrentPtr->ethernet_type;
		vlan_uplink_config->vlan_priority     = CurrentPtr->new_priority;

        vlan_qinq_config.qinq_objectid  = DstOnuIdx;
        vlan_qinq_config.qinq_direction = OLT_CFG_DIR_UPLINK;
        OLT_SetVlanQinQ(DstPonPortIdx, &vlan_qinq_config);
	}

	return(ROK);	
}

/*  将数据库中保存的ONU VLAN 操作配置数据设置到PON 芯片中*/
int  RestoreUplinkVlanManipulationToPon(short int PonPortIdx, short int OnuIdx)
{
#if 0
	short int PonChipType;
	/*short int Onu_llid;*/

	PON_olt_vlan_uplink_config_t    vlan_uplink_config ;	
#else
	PON_olt_vlan_uplink_config_t    *vlan_uplink_config;
	OLT_vlan_qinq_t vlan_qinq_config;
#endif
	
	PON_olt_vlan_uplink_manipulation_t uplink_vlan_manipulation;

	CHECK_ONU_RANGE

#if 0
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(PonChipType == PONCHIP_PAS5001) return(RERROR);

	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) return(RERROR);
	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP ) return ( RERROR );
	Onu_llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_llid == INVALID_LLID ) return (RERROR );
#endif

    /* B--modified by liwei056@2011-5-10 for D12777 */
#if 0
	VOS_MemSet(&vlan_uplink_config, 0, sizeof(PON_olt_vlan_uplink_config_t));
#else
	VOS_MemSet(&vlan_qinq_config, 0, sizeof(vlan_qinq_config));
    vlan_uplink_config = &vlan_qinq_config.qinq_cfg.up_cfg;
#endif
    /* E--modified by liwei056@2011-5-10 for D12777 */

	uplink_vlan_manipulation = PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].vlan_manipulation;
	if(( uplink_vlan_manipulation ==  PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED)
		||( uplink_vlan_manipulation ==  PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_EXCHANGED)
		||( uplink_vlan_manipulation ==  PON_OLT_VLAN_UPLINK_MANIPULATION_FRAME_IS_DISCARDED))
	{
#if 0
		vlan_uplink_config.untagged_frames_authentication_vid = PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0;
		vlan_uplink_config.authenticated_vid = PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].original_vlan_id;
		vlan_uplink_config.discard_untagged = FALSE;
		vlan_uplink_config.discard_tagged = FALSE;
		vlan_uplink_config.discard_null_tagged = FALSE;
		vlan_uplink_config.discard_nested = FALSE;
		vlan_uplink_config.vlan_manipulation = uplink_vlan_manipulation;
		vlan_uplink_config.new_vlan_tag_id = PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].new_vlan_id;
		vlan_uplink_config.vlan_type = PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].ethernet_type; /*PON_OLT_VLAN_ETHERNET_TYPE_8100;*/
		vlan_uplink_config.vlan_priority = PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].new_priority;
		 
		PAS_set_llid_vlan_mode(PonPortIdx, Onu_llid, vlan_uplink_config);
#else
		vlan_uplink_config->untagged_frames_authentication_vid = PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0;
		vlan_uplink_config->authenticated_vid = PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].original_vlan_id;
		vlan_uplink_config->discard_untagged = FALSE;
		vlan_uplink_config->discard_tagged = FALSE;
		vlan_uplink_config->discard_null_tagged = FALSE;
		vlan_uplink_config->discard_nested = FALSE;
		vlan_uplink_config->vlan_manipulation = uplink_vlan_manipulation;
		vlan_uplink_config->new_vlan_tag_id = PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].new_vlan_id;
		vlan_uplink_config->vlan_type = PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].ethernet_type; /*PON_OLT_VLAN_ETHERNET_TYPE_8100;*/
		vlan_uplink_config->vlan_priority = PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].new_priority;

        vlan_qinq_config.qinq_objectid  = OnuIdx;
        vlan_qinq_config.qinq_direction = OLT_CFG_DIR_UPLINK;
        OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config);
#endif
	}

	return(ROK);	
}


extern ULONG GPON_QINQ_DEBUG;
extern LONG devsm_sys_is_switchhovering();
int  CopyUplinkVlanManipulationToPon(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
	short int SrcOnuIdx;
	/*int ManipulationIdx;*/
	OLT_vlan_qinq_t vlan_qinq_config;
	PON_olt_vlan_uplink_config_t     *vlan_uplink_config = &vlan_qinq_config.qinq_cfg.up_cfg;
	Olt_llid_vlan_manipulation  *CurrentPtr;
	PON_olt_vlan_uplink_manipulation_t uplink_vlan_manipulation;
	short int ulSlot = GetCardIdxByPonChip(DstPonPortIdx);
	if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n%s, ulSlot:%d, switchoverring:%d \r\n", __FUNCTION__,  ulSlot, devsm_sys_is_switchhovering);}
	if(SYS_MODULE_IS_8000_GPON(ulSlot) && devsm_sys_is_switchhovering())
	{
		return(ROK);
	}

    /* 目前，只支持OnlyNew拷贝模式 */
    for ( SrcOnuIdx = 0; SrcOnuIdx < MAXONUPERPON; SrcOnuIdx++ )
	{
    	uplink_vlan_manipulation = PonPortTable[SrcPonPortIdx].Uplink_vlan_manipulation[SrcOnuIdx].vlan_manipulation;
        if ( uplink_vlan_manipulation != PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION )
        {
    		CurrentPtr = &PonPortTable[SrcPonPortIdx].Uplink_vlan_manipulation[SrcOnuIdx];

    		vlan_uplink_config->untagged_frames_authentication_vid = PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0;
    		vlan_uplink_config->authenticated_vid = CurrentPtr->original_vlan_id;
    		vlan_uplink_config->discard_untagged    = FALSE;
    		vlan_uplink_config->discard_tagged      = FALSE;
    		vlan_uplink_config->discard_null_tagged = FALSE;
    		vlan_uplink_config->discard_nested      = FALSE;
    		vlan_uplink_config->vlan_manipulation = uplink_vlan_manipulation;
    		vlan_uplink_config->new_vlan_tag_id   = CurrentPtr->new_vlan_id;
    		vlan_uplink_config->vlan_type         = CurrentPtr->ethernet_type;
    		vlan_uplink_config->vlan_priority     = CurrentPtr->new_priority;

            vlan_qinq_config.qinq_objectid  = SrcOnuIdx;
            vlan_qinq_config.qinq_direction = OLT_CFG_DIR_UPLINK;
            OLT_SetVlanQinQ(DstPonPortIdx, &vlan_qinq_config);
        }
	}

	return(ROK);
}

int  CopyDownlinkVlanManipulationToPon(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
	int Entry;
	int ManipulationIdx;
	
	PON_vlan_tag_t  original_vlan;
	OLT_vlan_qinq_t vlan_qinq_config;
	PON_olt_vid_downlink_config_t    *vlan_downlink_config = &vlan_qinq_config.qinq_cfg.down_cfg;
	Olt_llid_vlan_manipulation  *CurrentPtr;

    /* 目前，只支持OnlyNew拷贝模式 */
	for(Entry=PonPortTable[SrcPonPortIdx].Downlink_vlan_Cfg_Max; Entry>=0; Entry--)
	{
	    ManipulationIdx = PonPortTable[SrcPonPortIdx].Downlink_vlan_Config[Entry];
        if ( ManipulationIdx > 0 )
        {
    		CurrentPtr = &PonPortTable[SrcPonPortIdx].Downlink_vlan_manipulation[ManipulationIdx];

			original_vlan = Entry;
            vlan_downlink_config->discard_nested = FALSE;
			vlan_downlink_config->destination = PON_OLT_VLAN_DESTINATION_ADDRESS_TABLE;
			vlan_downlink_config->llid = 0;
			vlan_downlink_config->vlan_manipulation = CurrentPtr->vlan_manipulation;
			vlan_downlink_config->new_vid = CurrentPtr->new_vlan_id;
			vlan_downlink_config->new_priority = CurrentPtr->new_priority;

            vlan_qinq_config.qinq_objectid  = original_vlan;
            vlan_qinq_config.qinq_direction = OLT_CFG_DIR_DOWNLINK;
            OLT_SetVlanQinQ(DstPonPortIdx, &vlan_qinq_config);
        }
	}

	return(ROK);
}


int  RestoreDownllinkVlanManipulationToVlan(short int PonPortIdx, unsigned short VlanIdx)
{
	int ManipulationIdx;
#if 0
	PON_olt_vid_downlink_config_t    vlan_downlink_config ;
#else
	PON_olt_vid_downlink_config_t    *vlan_downlink_config;
	OLT_vlan_qinq_t vlan_qinq_config;
#endif
	Olt_llid_vlan_manipulation  *CurrentPtr;

    ManipulationIdx = PonPortTable[PonPortIdx].Downlink_vlan_Config[VlanIdx];
	CurrentPtr = &PonPortTable[PonPortIdx].Downlink_vlan_manipulation[ManipulationIdx];

    vlan_downlink_config = &vlan_qinq_config.qinq_cfg.down_cfg;

#if 0
    vlan_downlink_config.discard_nested = FALSE;
	vlan_downlink_config.destination = PON_OLT_VLAN_DESTINATION_ADDRESS_TABLE;
	vlan_downlink_config.llid = 0;
	vlan_downlink_config.vlan_manipulation = CurrentPtr->vlan_manipulation;
	vlan_downlink_config.new_vid = CurrentPtr->new_vlan_id;
	vlan_downlink_config.new_priority = CurrentPtr->new_priority;
	 
    return PAS_set_vid_downlink_mode(PonPortIdx, original_vlan, vlan_downlink_config);
#else
    vlan_downlink_config->discard_nested = FALSE;
	vlan_downlink_config->destination = PON_OLT_VLAN_DESTINATION_ADDRESS_TABLE;
	vlan_downlink_config->llid = 0;
	vlan_downlink_config->vlan_manipulation = CurrentPtr->vlan_manipulation;
	vlan_downlink_config->new_vid = CurrentPtr->new_vlan_id;
	vlan_downlink_config->new_priority = CurrentPtr->new_priority;

    vlan_qinq_config.qinq_objectid  = VlanIdx;
    vlan_qinq_config.qinq_direction = OLT_CFG_DIR_DOWNLINK;
    return OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config);
#endif
}

int  RestoreDownlinkVlanManipulationToPon(short int PonPortIdx)
{
	short int PonChipType;
	int Entry;
	int ManipulationIdx;
	
	PON_vlan_tag_t  original_vlan;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(PonChipType == PONCHIP_PAS5001) return(RERROR);

	SetPonPortVlanTpid(PonPortIdx,PonPortTable[PonPortIdx].vlan_tpid);

	for(Entry=PonPortTable[PonPortIdx].Downlink_vlan_Cfg_Max; Entry>=0; Entry--)
	{
	    ManipulationIdx = PonPortTable[PonPortIdx].Downlink_vlan_Config[Entry];
        if ( ManipulationIdx > 0 )
        {
            RestoreDownllinkVlanManipulationToVlan(PonPortIdx, Entry);
        }
	}

	return(ROK);
}


int  ClearPonUplinkVlanManipulation(short int PonPortIdx, short int OnuIdx)
{
    int return_result;
    OLT_vlan_qinq_t vlan_qinq_config;
	PON_olt_vlan_uplink_config_t    *vlan_uplink_config;
	
	CHECK_ONU_RANGE

	/* if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) return(RERROR); */

	VOS_MemSet(&vlan_qinq_config, 0, sizeof(vlan_qinq_config));

    vlan_uplink_config = &(vlan_qinq_config.qinq_cfg.up_cfg);
	vlan_uplink_config->untagged_frames_authentication_vid = PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0;
	vlan_uplink_config->authenticated_vid = PON_ALL_FRAMES_AUTHENTICATE;
	vlan_uplink_config->discard_untagged = FALSE;
	vlan_uplink_config->discard_tagged = FALSE;
	vlan_uplink_config->discard_null_tagged = FALSE;
	vlan_uplink_config->discard_nested = FALSE;
	vlan_uplink_config->vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION;
	vlan_uplink_config->new_vlan_tag_id = 1;
	vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_8100;
	vlan_uplink_config->vlan_priority = PON_VLAN_ORIGINAL_PRIORITY;

    vlan_qinq_config.qinq_objectid  = OnuIdx;
    vlan_qinq_config.qinq_direction = OLT_CFG_DIR_UPLINK;
    return_result = OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config);

	return(return_result);	
}

int  ClearPonDownlinkVlanManipulation(short int PonPortIdx, PON_vlan_tag_t original_vlan)
{
    int return_result;
    OLT_vlan_qinq_t vlan_qinq_config;

	VOS_MemSet(&vlan_qinq_config, 0, sizeof(OLT_vlan_qinq_t));
    vlan_qinq_config.qinq_cfg.down_cfg.vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION;
    vlan_qinq_config.qinq_objectid  = original_vlan;
    vlan_qinq_config.qinq_direction = OLT_CFG_DIR_DOWNLINK;
    return_result = OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config);
    
	return(return_result);
}


/** 取PON 端口用户自定义VLAN-tpid*/
int GetPonPortVlanTpid(short int PonPortIdx)
{
#if 0
	unsigned short vlan_tpid;
	short int pasRet = PAS_EXIT_OK;
    
	CHECK_PON_RANGE

	if(Olt_exists(PonPortIdx))
		pasRet=PAS_get_vlan_recognizing(PonPortIdx, &vlan_tpid);
	else
        vlan_tpid = PonPortTable[PonPortIdx].vlan_tpid;

	if(pasRet == PAS_EXIT_OK)
		return(vlan_tpid);
	else
		return( RERROR);
#endif
	int vlan_tpid;

	CHECK_PON_RANGE

    vlan_tpid = (int)PonPortTable[PonPortIdx].vlan_tpid;

    return vlan_tpid;
}

/** 设置PON 端口用户自定义VLAN-tpid*/
int SetPonPortVlanTpid(short int PonPortIdx, unsigned short int vlan_tpid)
{
	int pasRet;
	
	CHECK_PON_RANGE

#if 0
	if(Olt_exists(PonPortIdx))
		pasRet = PAS_set_vlan_recognizing(PonPortIdx,vlan_tpid);
	PonPortTable[PonPortIdx].vlan_tpid  = vlan_tpid;
	if(pasRet == PAS_EXIT_OK)
		return(ROK);
	else
		return( RERROR);
#else    
	SWAP_TO_ACTIVE_PON_PORT_INDEX(PonPortIdx)
    pasRet = OLT_SetVlanTpid(PonPortIdx, vlan_tpid);
    if (OLT_CALL_ISOK(pasRet))
    {
        pasRet = ROK;
    }

    return pasRet;
#endif
}

int  CopyPonPortVlanTpid(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    int iRlt = 0;
    unsigned short int usSrcValue;
    unsigned short int usDstValue;

    usSrcValue = PonPortTable[SrcPonPortIdx].vlan_tpid;
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OLT_SetVlanTpid(DstPonPortIdx, usSrcValue);
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( DEFAULT_PON_VLAN_TPID != usSrcValue )
        {
            iRlt = OLT_SetVlanTpid(DstPonPortIdx, usSrcValue);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            usDstValue = PonPortTable[DstPonPortIdx].vlan_tpid;
            if ( usDstValue != usSrcValue )
            {
                iRlt = OLT_SetVlanTpid(DstPonPortIdx, usSrcValue);
            }
        }
        else
        {
            iRlt = OLT_SetVlanTpid(DstPonPortIdx, usSrcValue);
        }
    }

    return iRlt;
}

/*added by liyang @2015-06-03 for combine onu ID*/
int GetPonQinQRuleOnuList(short int PonPortIdx,short int *OnuListNum, unsigned char *OnuList)
{
	
	short int onuIndex1 = 0,onuIndex2 = 0,count = 0,groupNum = 0,onuTmp = 0,onuIndexPrev = 0;
	char OnuBitMap[MAXONUPERPONNOLIMIT/8],i =0,j= 0;
	unsigned char *tmp = NULL;

	VOS_MemSet(OnuBitMap,0,sizeof(OnuBitMap));

	if(OnuListNum == NULL || OnuList == NULL)
		return RERROR;
	
	for(onuIndex1 =0; onuIndex1 < MAXONUPERPON; onuIndex1++)
	{
		i = onuIndex1/8;
		j = onuIndex1%8;

		count = 0;
		onuTmp = 0;
		onuIndexPrev = 0;
		
		if(PonPortTable[PonPortIdx].Uplink_vlan_manipulation[onuIndex1].vlan_manipulation != PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION && (!(OnuBitMap[i] & (1<<j))))
		{
			tmp = OnuList+groupNum*256;
			
			*(tmp+0) = onuIndex1+1;

			tmp += 1;

			tmp += VOS_Sprintf(tmp, "%d",onuIndex1+1);

			OnuBitMap[i] = OnuBitMap[i]| (1<<j);

			groupNum++;

			onuIndexPrev = onuIndex1;
			onuTmp = onuIndex1 + 1;
		
			for(onuIndex2 = onuIndex1+1; onuIndex2 < MAXONUPERPON; onuIndex2++)
			{
				i = onuIndex2/8;
				j = onuIndex2%8;
			
				if(PonPortTable[PonPortIdx].Uplink_vlan_manipulation[onuIndex2].vlan_manipulation != PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION && (!(OnuBitMap[i] & (1<<j))))
				{
					if(VOS_MemCmp(&PonPortTable[PonPortIdx].Uplink_vlan_manipulation[onuIndex1], &PonPortTable[PonPortIdx].Uplink_vlan_manipulation[onuIndex2], sizeof(PonPortTable[PonPortIdx].Uplink_vlan_manipulation[onuIndex1])) == 0)
					{
						OnuBitMap[i] = OnuBitMap[i]| (1<<j);

						if(onuIndex2-onuIndexPrev == 1)
						{
							count++;
							onuIndexPrev = onuIndex2;
							continue;
						}
						else
						{	
							if(count >= 1)
							{
								tmp += VOS_Sprintf(tmp,"-%d",onuTmp+count);	
							}
							
							tmp += VOS_Sprintf(tmp,",%d",onuIndex2+1);
							onuTmp = onuIndex2+1;
							count = 0;
						}
						
						onuIndexPrev = onuIndex2;
					}
				}
			}

			if(count >= 1)
			{
				tmp += VOS_Sprintf(tmp,"-%d",onuTmp+count);
			}
		}
	}

	*OnuListNum = groupNum;

	return ROK;

}
extern short int PonOamDebugOltId;
extern short int PonOamDebugLlid;
extern unsigned char OnuMacAddrDebug[BYTES_IN_MAC_ADDRESS];
int test_debug = 0;
void PonTxOamDebugPrint(short int olt_id, unsigned char *buf, int size)
{
	int  PonChipType = V2R1_GetPonchipType( olt_id );
	
    if(( OLT_PONCHIP_ISPAS(PonChipType) && (test_debug || ((0xFE == buf[4] && size > 20)     
    && (0x11 == buf[8] && 0x11 == buf[9] && 0x11 == buf[10])))
    && (PonOamDebugOltId == olt_id && PonOamDebugLlid == buf[2])) || (OLT_PONCHIP_ISTK(PonChipType)&&!VOS_MemCmp((buf+6),OnuMacAddrDebug,BYTES_IN_MAC_ADDRESS))
			|| (OLT_PONCHIP_ISBCM(PonChipType) && !VOS_MemCmp((buf+13),OnuMacAddrDebug,BYTES_IN_MAC_ADDRESS)))
    {   
        sys_console_printf("\r\nSend OAM to onu%d/%d/%d, length=%d\r\n", GetCardIdxByPonChip(PonOamDebugOltId),
        GetPonPortByPonChip(PonOamDebugOltId), GetOnuIdxByLlid(PonOamDebugOltId, PonOamDebugLlid)+1, size);
        sys_console_printf("========================================================\r\n");
        pktDataPrintf(buf, size);
    }
}
void PonRxOamDebugPrint(short int olt_id, unsigned char *buf, int size)
{
	int  PonChipType = V2R1_GetPonchipType( olt_id );
	short int len = 0;

	if(OLT_PONCHIP_ISTK(PonChipType))
	{
		if(*((unsigned short *)(buf + 6)) == 0x000E || *((unsigned short *)(buf + 6)) == 0x000F)
			len = 10;
		else
			len = 8;
	}
	
    if(( OLT_PONCHIP_ISPAS(PonChipType) && (test_debug  || ((size >= 50)
     && ( (buf[24] == 0x88)&& (buf[25] == 0x09) &&(buf[30] != 0x01))))
     && (PonOamDebugOltId == olt_id && (buf[10] == PonOamDebugLlid))) ||(OLT_PONCHIP_ISTK(PonChipType)&&!VOS_MemCmp((buf+len),OnuMacAddrDebug,BYTES_IN_MAC_ADDRESS))
			|| (OLT_PONCHIP_ISBCM(PonChipType) && !VOS_MemCmp((buf+13),OnuMacAddrDebug,BYTES_IN_MAC_ADDRESS)))
    {
        sys_console_printf("\r\nRecv OAM from onu%d/%d/%d, length=%d\r\n", GetCardIdxByPonChip(PonOamDebugOltId),
        GetPonPortByPonChip(PonOamDebugOltId), GetOnuIdxByLlid(PonOamDebugOltId, PonOamDebugLlid)+1, size);
        sys_console_printf("========================================================\r\n");        
        pktDataPrintf(buf, size);
    }

}


void PonGeneralOamDebugInit()
{
   physical_SendOam_hookrtn = PonTxOamDebugPrint;
   physical_RecieveOam_hookrtn = PonRxOamDebugPrint;

}

/** added by wangjiah@2017-03-17
 * To check whether PonPortIdx1 and PonPortIdx2 on a same pon chip
 * return TRUE if they're on the same, otherwise return FALSE.
 */
int IsPonPortPairOnSameChip(short int PonPortIdx1, short int PonPortIdx2)
{
	int iRlt = FALSE;
	int slot1 = 0, slot2 = 0, port1 = 0, port2 = 0;
	int portsPerChip = 0; 
    short int olt_ids[MAXOLTPERPONCHIP];

	OLT_ASSERT(PonPortIdx1);
	OLT_ASSERT(PonPortIdx2);
	
	if(OLT_ISREMOTE(PonPortIdx1) || OLT_ISREMOTE(PonPortIdx2))
		return iRlt;

	slot1 = GetCardIdxByPonChip(PonPortIdx1);
	slot2 = GetCardIdxByPonChip(PonPortIdx2);

	port1 = GetPonPortByPonChip(PonPortIdx1) - 1;
	port2 = GetPonPortByPonChip(PonPortIdx2) - 1;

	if(slot1 == slot2)
	{
		portsPerChip = GetPonChipPonPorts(PonPortIdx1, olt_ids);	
		if((port1 / portsPerChip) == (port2 / portsPerChip)) iRlt = TRUE;
	}
	return iRlt;
}
#if 0
int GetPonPortVlanTpid_1(int slot, int port)
{
	/*short int vlan_tpid;*/
	short int PonPortIdx;

	PonPortIdx = GetPonPortIdxBySlot(slot, port);
	return(GetPonPortVlanTpid(PonPortIdx));
}

int SetPonPortVlanTpid_1(int slot, int port, unsigned short int vlan_tpid)
{
	short int PonPortIdx;

	PonPortIdx = GetPonPortIdxBySlot(slot, port);
	return(SetPonPortVlanTpid(PonPortIdx, vlan_tpid));
}

int  TestDownlinkVlanManipulationToPon(short int PonPortIdx, short int mode, short int original_vid, short int new_vid, short int new_priority)
{
	short int PonChipType;
	short int retval;
	PON_olt_vid_downlink_config_t    vlan_downlink_config ;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(PonChipType != PONCHIP_PAS5201) return(RERROR);
	

	vlan_downlink_config.discard_nested = FALSE;
	vlan_downlink_config.destination = PON_OLT_VLAN_DESTINATION_ADDRESS_TABLE;
	vlan_downlink_config.llid = 0;
	vlan_downlink_config.vlan_manipulation = mode;
	vlan_downlink_config.new_vid = new_vid;
	vlan_downlink_config.new_priority = new_priority;

	retval = PAS_test_vid_downlink_mode(PonPortIdx, original_vid, vlan_downlink_config);
	if(retval != PAS_EXIT_OK)
		sys_console_printf("test vid downlink err=%d\r\n", retval);
	retval = PAS_set_vid_downlink_mode(PonPortIdx, original_vid, vlan_downlink_config);
	if(retval != PAS_EXIT_OK)
		sys_console_printf("set vid downlink err=%d\r\n", retval);

	return(ROK);
}
#endif
#endif

#ifdef __cplusplus
}
#endif

