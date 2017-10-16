/***************************************************************
*
*						Module Name:  PonInit.c
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
*   Date: 			2006/04/29
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name          |     Description
**---- ------- |--------   ---|------------------ 
**  06/04/29    |   chenfj          |     create, mainly used for PASSAVE chip 5001
**------------|-------------|------------------
** 1 modified by chenfj 2007/02/06
**     #3142 问题单:重启OLT，有时ONU注册不上
** 2 modified by chenfj 2007/04/19
**     将PON芯片固件,DBA算法等文件,按ONU文件合并方式,生成一个文件,下载到FLASH中.在给PON芯片加载时,改变原有读取文件方式.
** 3 added by chenfj 2007-7-19 
**     增加PON芯片下行是否有外部数据BUF判断
** 4 added  by chenfj 2007-8-1
**  	增加GT813/GT816 作为CTC ONU注册，但仍作为私有ONU来 管理
** 5 问题单:#5376 
**    问题单:#5372
**     modified by chenfj 2007-9-20
**     先判断PON口是否使能了保护切换，之后再执行PONTx_Enable 
**	6 modified by chenfj 2008-7-9
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
#include  "PonEventHandler.h"
#include  "gwEponSys.h"

#include "onu/onuConfMgt.h"
#include "onu/Onu_manage.h"
#include "../onu/OnuPortStatsMgt.h"

int varflag = 0;
int g_mac_auth_result_failure = V2R1_DISABLE;

PON_olt_init_parameters_t	PAS_initialization_parameters_5001/*={{0}}*/;	/* 20100201 */
PON_olt_initialization_parameters_t PAS_initialization_parameters_5201/* = {{0}}*/;
/*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
GW10G_PON_olt_initialization_parameters_t PAS_initialization_parameters_8411/* = {{0}}*/;
TK_olt_initialization_parameters_t  TK_initialization_parameters_55524;
#if defined(_GPON_BCM_SUPPORT_)
GPON_olt_initialization_parameters_t  GPON_initialization_parameters;
#endif

PON_olt_cni_port_mac_configuration_t  PAS_port_cni_parameters/*={0}*/;
PON_olt_update_parameters_t  PAS_updated_parameters_5001/*={0}*/;
PON_update_olt_parameters_t PAS_updated_parameters_5201 /*= { 0 }*/;
/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
GW10G_PON_update_olt_parameters_t PAS_updated_parameters_8411 /*= { 0 }*/;

PAS_system_parameters_t  PAS_system_parameters/*={0}*/;
PAS_pon_initialization_parameters_t PAS_init_para/*={0}*/;
CTC_STACK_oui_version_record_t PAS_oui_version_records_list[MAX_OUI_RECORDS];
int PAS_oui_version_records_num;

/* B--added by liwei056@2010-7-22 for 6900-LocalBus */
PonInitParam_S PON_init_params;
/* E--added by liwei056@2010-7-22 for 6900-LocalBus */

#if 0
PonPortMgmtInfo_S PonPortTable[SYS_MAX_PON_PORTNUM];
PonLLIDAttr_S  PonLLIDTable[SYS_MAX_PON_PORTNUM][MAXLLID];
#else
PonPortMgmtInfo_S *PonPortTable;
/* PonLLIDAttr_S     (*PonLLIDTable)[MAXLLID]; */
#endif
PonConfigInfo_S PonPortConfigDefault;

/* B--added by liwei056@2011-3-9 for PonStaticMac */
int           GlobalPonAddrTblNum;
unsigned char GlobalPonStaticMacAddr[MAXOLTADDRNUM][OLT_ADDR_BYTELEN];	    
/* E--added by liwei056@2011-3-9 for PonStaticMac */

PON_olt_monitoring_configuration_t PonDefaultAlarmConfiguration/*={0}*/;

unsigned long  PonPortDataSemId = 0 ;
unsigned long  PonLLIDDataSemId = 0;

extern ULONG g_ulRPCCall_mode;

bool V2R1_discard_llid_unlearned_sa = FALSE;
PON_pon_network_traffic_direction_t  V2R1_discard_unknown_da = PON_DIRECTION_NO_DIRECTION;

unsigned char *Pon_software_error_alarm_s[] = 
{
	(unsigned char *)"PON_SOFTWARE_ALARM_SEMAPHORE_ERROR",			
	(unsigned char *)"PON_SOFTWARE_ALARM_DATA_INCONSISTENCY",		
	(unsigned char *)"PON_SOFTWARE_ALARM_MEMORY_ALLOCATION_ERROR"
};


#ifdef PLATO_DBA_V3_5
unsigned char DBA_key[] = 
{
	0xED, 0x3B, 0xC8, 0x20, 0xBA, 0xE1, 0xD1, 0x0D,
	0xED, 0x44, 0xE8, 0x3F, 0x6E, 0xEA, 0xFB, 0xBE, 
	0xAA, 0xC3, 0xFB, 0x6D, 0x0E, 0x31, 0xD9, 0xBB,
	0x89, 0xE2, 0x23, 0x9E, 0xB5, 0xF9, 0x4F, 0x4F 
};

unsigned char DBA_id[] = 
{
	0x50, 0x4d, 0x43, 0x2d, 0x53, 0x69, 0x65, 0x72,
	0x72, 0x61, 0x0a, 0x33, 0xdc, 0xe1, 0x89, 0x01
};
#elif defined PLATO_DBA_V3_3
unsigned char DBA_key[] = 
{
	0xED, 0x3B, 0xC8, 0x20, 0xBA, 0xE1, 0xD1, 0x0D,
	0xED, 0x44, 0xE8, 0x3F, 0x6E, 0xEA, 0xFB, 0xBE, 
	0xAA, 0xC3, 0xFB, 0x6D, 0x0E, 0x31, 0xD9, 0xBB,
	0x89, 0xE2, 0x23, 0x9E, 0xB5, 0xF9, 0x4F, 0x4F 
};

unsigned char DBA_id[] = 
{
	0x50, 0x4d, 0x43, 0x2d, 0x53, 0x69, 0x65, 0x72,
	0x72, 0x61, 0x0a, 0x33, 0xdc, 0xe1, 0x89, 0x01
};
#else
unsigned char DBA_key[] = 
{
	0x4A,  0x22,  0xC0,  0xA3, 0xC9, 0xBC,  0x8B,  0xC3, 
	0x6F,  0xEE,  0x91,  0xF3, 0x8C, 0x4E,  0x68,  0xEA,
	0xA4,  0xA3,  0x7B,  0x76, 0xFA, 0x0C,  0xB6,  0x6A, 
	0x38,  0x04,  0xAA,  0xF8, 0x49, 0xE9,  0xDB,  0x00
};

unsigned char DBA_id[] = 
{
	0x55, 0x54,  0x20,  0x53,  0x74,  0xdc,  0x72,  0x63,
	0x6F, 0x6D,  0xb4,  0x33,  0xdc,  0xe1,  0x89,  0x01
};
#endif


unsigned char *PonPortOperStatus_s[] = 
{
	(unsigned char *)"unknown",
	(unsigned char *)"up",
	(unsigned char *)"down",
	(unsigned char *)"unknown",
	(unsigned char *)"loop",
	(unsigned char *)"update",
	(unsigned char *)"Initing",
	(unsigned char *)"FAULT"
};

unsigned char *PonPortAdminStatus_s[] =
{
	(unsigned char *)"",
	(unsigned char *)"up",
	(unsigned char *)"down"
};

unsigned char *PonPortApsCtrl_s[] = 
{
	(unsigned char *)"",
	(unsigned char *)"disable",
	(unsigned char *)"auto",
	(unsigned char *)"forced"
};

unsigned char *PonLinkDirection_s[] = 
{
	(unsigned char *)"up-link",
	(unsigned char *)"down-link",
	(unsigned char *)"up-and-down",
	(unsigned char *)"no-direction"
};

unsigned char *PonQueuePriority_s[] = 
{
	(unsigned char *)"low",
	(unsigned char *)"high"
};

unsigned char PON_priority_queue_mapping[MAX_PRIORITY_QUEUE+1]= {0,0,0,0,1,1,1,1};
unsigned char PON_priority_queue_mapping_default[MAX_PRIORITY_QUEUE+1]= {0,0,0,0,1,1,1,1};
unsigned int PON_Jumbo_frame_length_default = 1596;
unsigned int PON_Jumbo_frame_length = 1596;

/*unsigned char  OnuMac1[6] = { 0x00, 0x08, 0x5c, 0x53, 0x12, 0x9e };*/

/* B--added by liwei056@2010-7-20 for Flash-VFS Support */
#define PON_LOAD_CLEAR_TIMESEC  60
int                 PON_LoadRescClearFlag;
static int          PON_LoadRescRefCnt;

static char        *PON_FirmwareMemBuf[PONCHIP_TYPE_MAX];
static char        *PON_DBAMemBuf[PONCHIP_TYPE_MAX];
static unsigned int PON_FirmwareMemLen[PONCHIP_TYPE_MAX];
static unsigned int PON_DBAMemLen[PONCHIP_TYPE_MAX];
/* E--added by liwei056@2010-7-20 for Flash-VFS Support */

/* modified by xieshl 20120813, 问题单15520 */
/*extern ULONG Onu_user_max_all;
extern ULONG mac_check_exit_flag;
extern ULONG OnuMacTable_check_interval;*/
#if(EPON_MODULE_ONU_MAC_OVERFLOW_CHECK==EPON_MODULE_YES)
extern LONG resumeOnuMacCheckConfig( SHORT PonPortIdx );
#endif
extern LONG OnuEventCtc_init();     /*CTC ONU 告警模块初始化*/
extern int OnuEvent_ClearRunningDataMsg_Send(short int PonPortIdx);
extern int CTC_STACK_event_notification_handler(const PON_olt_id_t   olt_id, 
					                                const PON_onu_id_t  onu_id, 
					                                const CTC_STACK_event_value_t   event_value );

extern int CTC_GetOnuDeviceInfo( short int PonPortIdx, short int OnuIdx );
extern int SetOnuAuthenticationMode( short int PonPortIdx, unsigned char mode );
extern int	retrieveCtcOneOnuAction( const ULONG devIdx );
extern STATUS	getOnuAuthEnable( ULONG slot, ULONG port, ULONG *enable );
extern LONG getCtcStackOui( ULONG slotno, UCHAR *p_oui, UCHAR *p_oui_ver );

extern void ReadCPLDReg( unsigned char * RegAddr, unsigned char * pucIntData );
extern void WriteCPLDReg( unsigned char * RegAddr, unsigned char ucIntData );

extern LONG CopyCtcOnuAuthLoid(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags);
extern LONG CopyCtcOnuAuthMode(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags);
extern LONG resumeOnuRepeatedDelEnable();

int  pon_set_init_parameters(unsigned short host_manage_iftype, unsigned short host_manage_address);
int CopyPonPortAuthEnable(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags);
int ResumeOnuDownlinkQosMapping(short int PonPortIdx);
int ResumeOLTStaticMacAddrTbl(short int PonPortIdx, short int IsNeedSave);
#if defined(_EPON_10G_BCM_SUPPORT_)            
int BCM55538_pon_add_olt(short int PonPortIdx, short int PonChipVer);
#endif
int TK3723_pon_add_olt(short int PonPortIdx, short int PonChipVer);
int GW5201_pon_add_olt(short int PonPortIdx, short int PonChipVer);
#if defined(_GPON_BCM_SUPPORT_)
int GPON_pon_add_olt(short int PonPortIdx, short int PonChipVer);
#endif
/*****************************************************
 *
 *    Function: AddInvalidOnuMac( short int PonPortIdx, unsigned char *OnuMac )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *			  unsigned char *OnuMac -- the pointer of the mac address
 *               
 *    Desc:   add or delete the invalid onu mac address to the Pon ACL 
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/

int  AddInvalidOnuMac( short int PonPortIdx, unsigned char *OnuMac )
{
	CHECK_PON_RANGE
		
	return( SetPonPortInvalidOnu( PonPortIdx,  OnuMac, ADD_INVALID_ONU) );	
}

int DelInvalidOnuMac( short int PonPortIdx, unsigned char *OnuMac )
{
	CHECK_PON_RANGE

	return( SetPonPortInvalidOnu(PonPortIdx, OnuMac, DELETE_INVALID_ONU ));

}

/*****************************************************
 *
 *    Function: CTC_stack_initialize(void)
 *
 *    Param:   none
 *                 
 *    Desc:  initialize the PAS-SOFT  CTC STACK
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int CTC_stack_initialize()
{
	short int ret;
	bool automatic_mode;

    automatic_mode = TRUE;
	ret = CTC_STACK_init(automatic_mode, PAS_oui_version_records_num, PAS_oui_version_records_list);
	if( ret != PAS_EXIT_OK ) 
	{
		sys_console_printf(" CTC STACK init Err %d \r\n", ret );
		return( RERROR );
	}
	else
    {
		/*sys_console_printf(" CTC STACK init OK\r\n");*/
	}

    return( ROK );
}

int PON_CTC_stack_initialize()
{
	short int ret;
	bool automatic_mode;

    automatic_mode = TRUE;
	ret = PON_CTC_STACK_init(automatic_mode, PAS_oui_version_records_num, PAS_oui_version_records_list);
	if( ret != PAS_EXIT_OK ) 
	{
		sys_console_printf(" EPON CTC STACK init Err %d \r\n", ret );
		return( RERROR );
	}
	else
    {
		/*sys_console_printf(" CTC STACK init OK\r\n");*/
	}

    return( ROK );
}

#if defined(_EPON_10G_PMC_SUPPORT_)      
/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
int GW8411_CTC_stack_initialize()
{
	short int ret;
	bool automatic_mode;

    automatic_mode = TRUE;
	ret = GW10G_CTC_STACK_init(automatic_mode, PAS_oui_version_records_num, PAS_oui_version_records_list);
	if( ret != PAS_EXIT_OK ) 
	{
		sys_console_printf(" CTC STACK init Err %d \r\n", ret );
		return( RERROR );
	}
	else
    {
		/*sys_console_printf(" CTC STACK init OK\r\n");*/
	}

    return( ROK );
}
#endif
 
int GW5201_CTC_stack_initialize()
{
	short int ret;
	bool automatic_mode;

    automatic_mode = TRUE;
	ret = CTC_STACK_init(automatic_mode, PAS_oui_version_records_num, PAS_oui_version_records_list);
	if( ret != PAS_EXIT_OK ) 
	{
		sys_console_printf(" CTC STACK init Err %d \r\n", ret );
		return( RERROR );
	}
	else
    {
		/*sys_console_printf(" CTC STACK init OK\r\n");*/
	}

    return( ROK );
}

int pon_assign_pashandler_CTC_STACK()
{
	short int Handler_id;
	short int ret;

	ret = GW_CTC_STACK_assign_handler_function(CTC_STACK_HANDLER_END_OAM_DISCOVERY, (void *)Onu_ExtOAMDiscovery_Handler, &Handler_id);
	if( ret != CTC_STACK_EXIT_OK )
	{
		sys_console_printf("Assign CTC-STACK Ext Oam Discovery Handler Err %d");
		return( RERROR );
	}

	ret = GW_CTC_STACK_assign_handler_function(CTC_STACK_HANDLER_EVENT_NOTIFICATION, (void *)CTC_STACK_event_notification_handler, &Handler_id);
	if( ret != CTC_STACK_EXIT_OK )
	{
		sys_console_printf("Assign CTC-STACK Ext Event Report Handler Err %d");
		return( RERROR );
	}
    
	return( ROK );
}
/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/

int pon_assign_tkhandler_CTC_STACK()
{
	short int Handler_id;
	short int ret;

	ret = TkCTC_assign_handler_function(CTC_STACK_HANDLER_END_OAM_DISCOVERY, (void *)Onu_ExtOAMDiscovery_Handler, &Handler_id);
	if( ret != CTC_STACK_EXIT_OK )
	{
		sys_console_printf("Assign CTC-STACK Ext Oam Discovery Handler Err %d");
		return( RERROR );
	}

	ret = TkCTC_assign_handler_function(CTC_STACK_HANDLER_EVENT_NOTIFICATION, (void *)CTC_STACK_event_notification_handler, &Handler_id);
	if( ret != CTC_STACK_EXIT_OK )
	{
		sys_console_printf("Assign CTC-STACK Ext Event Report Handler Err %d");
		return( RERROR );
	}
    
	return( ROK );
}

int pon_assign_ponhandler_CTC_STACK()
{
	short int Handler_id;
	short int ret;

	ret = PON_CTC_STACK_assign_handler_function(CTC_STACK_HANDLER_END_OAM_DISCOVERY, (void *)Onu_ExtOAMDiscovery_Handler, &Handler_id);
	if( ret != CTC_STACK_EXIT_OK )
	{
		sys_console_printf("Assign CTC-STACK Ext Oam Discovery Handler Err %d");
		return( RERROR );
	}

	ret = PON_CTC_STACK_assign_handler_function(CTC_STACK_HANDLER_EVENT_NOTIFICATION, (void *)CTC_STACK_event_notification_handler, &Handler_id);
	if( ret != CTC_STACK_EXIT_OK )
	{
		sys_console_printf("Assign CTC-STACK Ext Event Report Handler Err %d");
		return( RERROR );
	}
    
	return( ROK );
}

short  int Onu_ExtOAMDiscovery_Handler(PON_olt_id_t olt_id,  PON_llid_t llid, CTC_STACK_discovery_state_t result,  unsigned char Number_of_records,  CTC_STACK_oui_version_record_t *onu_version_records_list )
{
	if ( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf("Onu_ExtOAMDiscovery_Handler\r\n");
		sys_console_printf(" PONid:%d,llid:%d,ExtOamResult:%d,OUIs:%d\r\n", olt_id, llid, result, Number_of_records); 
	}

	
    sendOnuExtOamDiscoveryMsg(olt_id, llid, result, Number_of_records, onu_version_records_list, ONU_EVENTFLAG_REAL);
	return( PAS_EXIT_OK );
}
/*extern int CTC_STACK_start_loid_authentication (short int  olt_id, short int  onu_id);*/
short int OnuExtOAMDiscoveryHandler(ExtOAMDiscovery_t  *ExtOAMDiscovery_Data )
{
#if 0
	short int PonPortIdx, OnuIdx;
	short int PonChipType, vendorType;
	unsigned char record_number = 0;
	unsigned char i;
	unsigned int event_flags;
	int OnuType;
	int OnuSwitchType;
	int OnuEntry;
/* modified by xieshl 20110412, CTC ONU在扩展OAM发现完成后向主控同步 */
	int brdIdx, portIdx, onuIdx, ret;
    OnuRegisterDenied_S RegisterDenied_Data;  
	if( ExtOAMDiscovery_Data == NULL )
	{
		VOS_ASSERT(0);
		return(RERROR );
	}	

    event_flags = ExtOAMDiscovery_Data->event_flags;
	PonPortIdx = ExtOAMDiscovery_Data->olt_id;
	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return ( RERROR );

	OnuIdx = GetOnuIdxByLlid(PonPortIdx, ExtOAMDiscovery_Data->llid);
	if(( OnuIdx < 0 ) || ( OnuIdx >= MAXONUPERPON ))
		return ( RERROR );

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	brdIdx = GetCardIdxByPonChip( PonPortIdx );
	portIdx = GetPonPortByPonChip( PonPortIdx );
	onuIdx = OnuIdx + 1;

    /* modified by xieshl 20111011, 如果ONU已经被挂起，就不需要执行extOAM发现了，否则对不支持
	    静默的ONU会造成反复离线注册问题，严重时会堵塞OAM通道，问题单13516 */
	if( OnuMgmtTable[OnuEntry].OperStatus == ONU_OPER_STATUS_PENDING )
	{
		/* modified by xieshl 20111206, GFA6700存在这种情况，ONU反复注册离线时，离线事件可能比扩
		    展发现早，就会造成ONU是挂起状态，但pending队列中已经没有了，问题单14007 */
		if( SearchPendingOnu(PonPortIdx, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr) == RERROR )
		{
			OnuMgmtTable[OnuEntry].OperStatus = ONU_OPER_STATUS_DOWN;
			/*AddPendingOnu( PonPortIdx, OnuIdx, ExtOAMDiscovery_Data->llid, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr );*/
			sys_console_printf(" onu %d/%d/%d status is being off pending-queue\r\n", brdIdx, portIdx, onuIdx);
		}
		else
		{
			sys_console_printf(" onu %d/%d/%d is pending now, ext-oam discovery failure\r\n", brdIdx, portIdx, onuIdx);
		}
		return RERROR;
	}
    /* B--added by liwei056@2011-12-19 for PonQuicklySwitch-OnuRegisterRepeatBug */
    else
    {
        if ( event_flags & ONU_EVENTFLAG_VIRTAL )
        {
            if ( V2R1_ENABLE == OnuMgmtTable[OnuEntry].ExtOAM )
            {
                /* 虚拟发现的LLID已经发现完成，则此虚拟发现无效 */
                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d loose llid%d's virtual ext-discovery event for onu%d have finish the discovery by real-register.\r\n", PonPortIdx, ExtOAMDiscovery_Data->llid, onuIdx);
        		return RERROR;
            }
        }
    }
    /* E--added by liwei056@2011-12-19 for PonQuicklySwitch-OnuRegisterRepeatBug */

    /*B-基于loid 以及混合模式的认证，由此完成，认证不通过的进入pending队列。*
        *PS: 可选支持互通onu的静默机制。new add by luh 2012-3-1*/
    if(OnuEvent_Authentication(PonPortIdx, OnuIdx) != ONU_AUTH_SUCESS)
    {
        /*unsigned char luc_ver = 0;
                if(CTC_STACK_get_onu_version(PonPortIdx, ExtOAMDiscovery_Data->llid, &luc_ver) == CTC_STACK_EXIT_OK && luc_ver >= CTC_2_1_ONU_VERSION)
                {
                if(!VOS_MemCmp(OnuMgmtTable[OnuEntry].device_vendor_id, CTC_ONU_VENDORID, sizeof(OnuMgmtTable[OnuEntry].device_vendor_id)))
                AddPendingOnu(PonPortIdx, OnuIdx, ExtOAMDiscovery_Data->llid, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr);
                else
                PAS_deregister_onu( PonPortIdx, ExtOAMDiscovery_Data->llid, FALSE );
                }
                else*/                
        AddPendingOnu(PonPortIdx, OnuIdx, ExtOAMDiscovery_Data->llid, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, PENDING_REASON_CODE_ONU_AUTH_FAIL);
        RegisterDenied_Data.olt_id = PonPortIdx;
        VOS_MemCpy( RegisterDenied_Data.Onu_MacAddr, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS );
        OnuRegisterDeniedHandler( &RegisterDenied_Data);
        return VOS_ERROR;
    }
    /*End-new add by luh 2012-3-1*/
    
	if( ExtOAMDiscovery_Data->result == CTC_DISCOVERY_STATE_COMPLETE )
	{
		if( EVENT_REGISTER == V2R1_ENABLE )
		{
			record_number = ExtOAMDiscovery_Data->Number_of_records;
			
			sys_console_printf("OUI record num %d\r\n", record_number);					
			record_number = MIN( record_number, MAX_OUI_RECORDS );
			if( record_number != 0 )
			{
				for( i=0; i< record_number; i++)
				{
					sys_console_printf("%d OUI: %02x%02x%02x, version:%02x\r\n", (i+1), ExtOAMDiscovery_Data->onu_version_records_list[i].oui[0], ExtOAMDiscovery_Data->onu_version_records_list[i].oui[1], ExtOAMDiscovery_Data->onu_version_records_list[i].oui[2],  ExtOAMDiscovery_Data->onu_version_records_list[i].version);
				}
			}
			/*CTC_GetOnuSN(PonPortIdx, OnuIdx );*/
		}
		
		/*SetOnuExtOAMStatus(PonPortIdx, OnuIdx );*/
				
		/*GetOnuDeviceVersion( PonPortIdx, OnuIdx);	*/
		/*modified by chenfj 2008-6-3 
		    问题单6707: 问题1 
		    在获取ONU测距值等信息的ONU注册处理过程中,若有错误返回,则终止此次注册处理;这样应会避免ONU在线,但ONU测距值为零的情况
		*/
		if(GetOnuRegisterData( PonPortIdx, OnuIdx) != ROK )
			{
			AddPendingOnu( PonPortIdx, OnuIdx, ExtOAMDiscovery_Data->llid, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 0); 
			/*Onu_deregister( PonPortIdx,OnuIdx);*/
			return( RERROR );
			}

		SetOnuExtOAMStatus(PonPortIdx, OnuIdx );
		
		/*OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.type = V2R1_ONU_CTC;*/

		/*if( V2R1_CTC_STACK )*/
		{
		/* 获取ONU 设备信息*/
		CTC_GetOnuDeviceInfo( PonPortIdx, OnuIdx );

        /* B--added by liwei056@2011-1-30 for CodeCheck */
    	if( GetOnuVendorType( PonPortIdx, OnuIdx ) == ONU_VENDOR_CT )
        {
            vendorType = ONU_MANAGE_CTC;
        }
        else
        {
            vendorType = ONU_MANAGE_GW;
        }
    	ONU_SetupIFsByType(PonPortIdx, OnuIdx, PonChipType, (int)vendorType);
        /* E--added by liwei056@2011-1-30 for CodeCheck */

#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
    	if( ONU_PROTECT_TYPE_C == (OnuSwitchType = GetOnuProtectType( PonPortIdx, OnuIdx )) )
        {
            redundancy_onu_optical_add_onu(ExtOAMDiscovery_Data->olt_id, ExtOAMDiscovery_Data->llid);
        }   
    	OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"onu%d/%d/%d's switch cap:%d.", brdIdx, portIdx, onuIdx, OnuSwitchType);
#endif
		
		/*  设置ONU 组播协议模式*/
		VOS_TaskDelay(1);
		CTC_SetOnuMulticastSwitchProtocol(PonPortIdx, OnuIdx );
		}
		
		/*  added by chenfj 2007/7/26
		问题单: #5013.  ONU p2p功能在OLT重启后无效		
		在CTC ONU 注册分支增加ONU p2p设置 
		*/
#ifdef ONU_PEER_TO_PEER
		{
		int ret;
		short int OnuIdx1;

		EnableOnuPeerToPeerForward( PonPortIdx, OnuIdx );
		for( OnuIdx1= 0; OnuIdx1 < MAXONUPERPON; OnuIdx1++)
			{
			ret = GetOnuPeerToPeer( PonPortIdx, OnuIdx, OnuIdx1 );
			if( ret == V2R1_ENABLE )
				EnableOnuPeerToPeer( PonPortIdx, OnuIdx,OnuIdx1);
			}
		}
#endif	

		/* added  by chenfj 2007-8-2
			增加GT813/GT816 作为CTC ONU注册，但仍作为私有ONU来 管理*/

		OnuType = CompareCtcRegisterId( PonPortIdx, OnuIdx, OnuMgmtTable[OnuEntry].onu_model );
		if(OnuType != RERROR)
		{
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[OnuEntry].DeviceInfo.type = OnuType;
			ONU_MGMT_SEM_GIVE;
#if 0
			AddOneNodeToGetEUQ( PonPortIdx, OnuIdx );
#else
			/* modified by wangxiaoyu 2011-10-20, move to CTC_GetOnuDeviceInfo for get cortina onu name */
	        ret = GetOnuEUQInfo( PonPortIdx, OnuIdx );
	        if( EVENT_REGISTER == V2R1_ENABLE )
	        {
	            sys_console_printf(" onu %d/%d/%d get device info. %s\r\n", brdIdx, portIdx, onuIdx, (ret == VOS_OK ? "OK" : "ERR") );
	        }
	       
#endif
		}
		else
		{
			ONU_MGMT_SEM_TAKE;
			if(OnuMgmtTable[OnuEntry].onu_ctc_version >= CTC_2_1_ONU_VERSION)
			{
			    if(get_ctcOnuOtherVendorAccept() || !VOS_MemCmp(OnuMgmtTable[OnuEntry].device_vendor_id, CTC_ONU_VENDORID,
			            sizeof(OnuMgmtTable[OnuEntry].device_vendor_id)))
			        OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_ONU_CTC;
			    else
			        OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_DEVICE_UNKNOWN;
			}
			else
			    OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_DEVICE_UNKNOWN;
			OnuType = OnuMgmtTable[OnuEntry].DeviceInfo.type;

			ONU_MGMT_SEM_GIVE;
		}

		/*if ( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )*/		/* modified by xieshl 20111129, 增加主备同步 */
		{
			ret = OnuMgtSyncDataSend_Register( PonPortIdx, OnuIdx );
			/*if( EVENT_REGISTER == V2R1_ENABLE )
			{
				sys_console_printf(" onu %d/%d/%d device info. sync to master %s\r\n", brdIdx, portIdx, onuIdx, (ret == VOS_OK ? "OK" : "ERR") );
			}*/
		}


		/*  ctc onu register trap to NMS */
		Trap_OnuRegister( PonPortIdx, OnuIdx );
		
		if( OnuType == V2R1_ONU_CTC )
		{
			/* added by chenfj 2007-7-24 */
			/* Up onu uni port 
			CTC_UpOnuAllEthPort(PonPortIdx, OnuIdx );*/
#if 0
			devIdx = MAKEDEVID( brdIdx, portIdx, onuIdx );
			retrieveCtcOneOnuAction( devIdx );
#else
            /*如果之前升级成功，则执行commit操作*/
            if(getCtcOnuCommitFlag(PonPortIdx, OnuIdx))
            {
                int llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
				/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
                PON_STATUS ret = OnuMgt_CommitImage(PonPortIdx, OnuIdx);
				if(ret != CTC_STACK_EXIT_OK)
                {
					ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("commit ctc onu image fail!\r\n"));
                }
				else
					ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("commit ctc onu image ok!\r\n"));
                clearCtcOnuCommitFlag(PonPortIdx, OnuIdx);
            }
#endif
		}

        if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
        {
            int ponid = GetPonPortIdxBySlot(brdIdx, portIdx);

            addOnuToRestoreQueue(ponid, OnuIdx, ONU_CONF_RES_ACTION_DO);
        }

		sys_console_printf("\r\n onu%d/%d/%d CTC Ext OAM discovery Complete\r\n", brdIdx, portIdx, onuIdx );
	}
	else
	{
		/*sys_console_printf("%s/port%d onu%d CTC Ext OAM discovery failed\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );*/
		ClearOnuExtOAMStatus( PonPortIdx,  OnuIdx);
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_OTHER;
		ONU_MGMT_SEM_GIVE;
		OnuType = OnuMgmtTable[OnuEntry].DeviceInfo.type;
		sys_console_printf("\r\n onu%d/%d/%d CTC Ext OAM discovery failed(%d)\r\n", brdIdx, portIdx, onuIdx, ExtOAMDiscovery_Data->result );
	}
#if 0
	/*  ctc onu register trap to NMS */
	if( OnuMgmtTable[suffix].RegisterFlag == ONU_FIRST_REGISTER_FLAG )
		{
		/* send onu first registration trap to NMS */
		Trap_OnuRegister( PonPortIdx, OnuIdx, ONU_FIRST_REGISTER_FLAG );
				
		OnuMgmtTable[suffix].RegisterFlag = NOT_ONU_FIRST_REGISTER_FLAG;
		OnuMgmtTable[suffix].UsedFlag = ONU_USED_FLAG;
		}
	else if( OnuMgmtTable[suffix].RegisterFlag == NOT_ONU_FIRST_REGISTER_FLAG )
		{
		/* send onu Re_registration trap to NMS */
		Trap_OnuRegister( PonPortIdx, OnuIdx, NOT_ONU_FIRST_REGISTER_FLAG );
		}

	if( ExtOAMDiscovery_Data->result == CTC_DISCOVERY_STATE_COMPLETE )
		{
		sys_console_printf("\r\n onu%d/%d/%d CTC Ext OAM discovery Complete\r\n", brdIdx, portIdx, onuIdx );
		}
	else {
		sys_console_printf("\r\n onu%d/%d/%d CTC Ext OAM discovery failed(%d)\r\n", brdIdx, portIdx, onuIdx, ExtOAMDiscovery_Data->result );
		/*  chenfj 2008-10-17
		在测试时发现GT861 在连续的注册时, 会出现扩展OAM 发现失败; 一同测试的GT831没出现
		Deregister_onu(ExtOAMDiscovery_Data->olt_id, ExtOAMDiscovery_Data->llid, FALSE,FALSE);
		CTC_STACK_reset_onu(ExtOAMDiscovery_Data->olt_id,ExtOAMDiscovery_Data->llid);
		
		unsigned char MacAddr[BYTES_IN_MAC_ADDRESS] = {0};
		int len = 0;
		GetOnuMacAddr(PonPortIdx, OnuIdx,MacAddr, &len);
		AddPendingOnu(PonPortIdx, ExtOAMDiscovery_Data->llid, MacAddr);
		*/
		}
#endif
/*if( ExtOAMDiscovery_Data->Number_of_records !=0 && ExtOAMDiscovery_Data->Number_of_records < 62 )
{
for( i=0; i<ExtOAMDiscovery_Data->Number_of_records; i++ )
	sys_console_printf(" %d:oui=0x%02x%02x%02x, ver=0x%02x\r\n", i, ExtOAMDiscovery_Data->onu_version_records_list[i].oui[0],
	ExtOAMDiscovery_Data->onu_version_records_list[i].oui[1],
	ExtOAMDiscovery_Data->onu_version_records_list[i].oui[2], ExtOAMDiscovery_Data->onu_version_records_list[i].version );
}
else
	sys_console_printf(" onu_version_records_list num=%d\r\n", ExtOAMDiscovery_Data->Number_of_records );*/

	/*added by wangxy 2011-12-07
	 * 扩展发现成功但类型不识别或扩展发现失败均将ONU加入未决队列，等待下一次注册表尝试*/
#if 0
    if( OnuType == V2R1_OTHER)
        AddPendingOnu(PonPortIdx, OnuIdx, ExtOAMDiscovery_Data->llid, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 0);
    else if(OnuType == V2R1_DEVICE_UNKNOWN)
        OnuMgt_SetOnuTrafficServiceMode(PonPortIdx, OnuIdx, V2R1_DISABLE);
#else/*不再对unknown的onu关断业务，因为关断业务用户无法获知，也没有办法在pending队列恢复2012-8-2*/
    if( OnuType == V2R1_OTHER || OnuType == V2R1_DEVICE_UNKNOWN)
        AddPendingOnu(PonPortIdx, OnuIdx, ExtOAMDiscovery_Data->llid, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 0);
#endif
#endif
	return( ROK );
}

int GetOnuMacAuthorizeMode(short int PonPortIdx, bool  *mode )
{
	short int ret;
	short int PonChipType;
		
	CHECK_PON_RANGE

	if(mode == NULL ) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	

	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{

			}
		else 
			{
			/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = OLT_GetAuthorizeMacAddressAccordingListMode( PonPortIdx,  mode);
			if( ret != PAS_EXIT_OK )
				{
				sys_console_printf("Get %s/port%d ONU MAC Authorize Mode Err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ret );
				}
			else {
				sys_console_printf("Get %s/port%d ONU MAC Authorize Mode OK %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), *mode );
				return( ROK );
				}
			}		
		}
	else{ /* other pon chip type handler */

		}
	
	return( RERROR );
}

int SetOnuMacAuthorizeMode(short int PonPortIdx ,  bool  mode )
{
	short int ret;
	short int PonChipType;
	short int PonChipVer = RERROR;
	
	CHECK_PON_RANGE

	if( (mode != V2R1_ENABLE ) && ( mode != V2R1_DISABLE ))  return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{

			}
		else
			{
			/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			if( mode == V2R1_ENABLE )
				ret = OLT_SetAuthorizeMacAddressAccordingListMode( PonPortIdx,  ENABLE );
			else ret = OLT_SetAuthorizeMacAddressAccordingListMode( PonPortIdx,  DISABLE );
			/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			if( ret != PAS_EXIT_OK )
				{
				sys_console_printf("Set %s/port%d ONU MAC Authorize Mode Err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ret );
				}
			else {
				sys_console_printf("Set %s/port%d ONU MAC Authorize Mode OK\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
				return( ROK );
				}
			}		
		}
	else{ /* other pon chip type handler */

		}
	
	return( RERROR );
}

int AddOnuToAuthorizeListByIdx( short int PonPortIdx, short int  OnuIdx  )
{
	mac_addresses_list_t  MacAddr ;
	int ret, len;
	short int ret_val;
#if 0
	short int PonChipType, PonChipVer = RERROR;
#endif

	CHECK_ONU_RANGE


	if(ThisIsValidOnu( PonPortIdx, OnuIdx ) != ROK ) return( RERROR );

	ret = GetOnuMacAddr( PonPortIdx, OnuIdx, MacAddr[0], &len);
	if( ret != ROK ) 
		{
		sys_console_printf("Get %s/port%d onu%d MacAddr Err\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx +1));
		return( RERROR );
		}

#if 1
	SWAP_TO_ACTIVE_PON_PORT_INDEX_R(PonPortIdx) //PonPortIdx may excced 255
    ret_val = OLT_SetMacAuth(PonPortIdx, ENABLE, MacAddr[0]);			
	if( OLT_CALL_ISOK(ret_val) )
        return( ROK );
	else
		sys_console_printf("Set %s/port%d onu%d MacAddr to Authorize list Err%d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx +1), ret_val);
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

	if( PonChipType == PONCHIP_PAS )
		{
		    if( PonChipVer == PONCHIP_PAS5201 )
            {      
			ret_val = PAS_set_mac_address_authentication( PonPortIdx, ENABLE, 1, MacAddr);
			if( ret_val == PAS_EXIT_OK ) return( ROK );
			else {
				sys_console_printf("Set %s/port%d onu%d MacAddr to Authorize list Err%d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx +1), ret_val);
				}
			return ( RERROR );
			}
		}
		
	else { /* other pon chip type handler */
			
		}
#endif
	
	return( RERROR );	
}

int AddOnuToAuthorizeListByMac( short int PonPortIdx,  mac_address_t  MacAddr1 )
{
	short int ret_val;
#if 0
	short int PonChipType, PonChipVer = RERROR;
	mac_addresses_list_t  MacAddr ;
#endif

	CHECK_PON_RANGE

	SWAP_TO_ACTIVE_PON_PORT_INDEX_R(PonPortIdx) //PonPortIdx may excced 255
	/*if( CompTwoMacAddress( MacAddr1, Invalid_Mac_Addr ) == ROK ) 
		return( RERROR );
	if( CompTwoMacAddress( MacAddr1, Invalid_Mac_Addr1 ) == ROK ) 
		return( RERROR );*/
	if( MAC_ADDR_IS_ZERO(MacAddr1) || MAC_ADDR_IS_BROADCAST(MacAddr1) )
		return( RERROR );

#if 1
    ret_val = OLT_SetMacAuth(PonPortIdx, ENABLE, MacAddr1);			
	if( OLT_CALL_ISOK(ret_val) )
        return( ROK );
	else
		sys_console_printf("Set %s/port%d onu MacAddr to Authorize list Err%d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ret_val);
#else
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
    
	if( PonChipType == PONCHIP_PAS )
		{
		if( PonChipVer == PONCHIP_PAS5001 )
			{
			return( RERROR );
			}
		else if( PonChipVer == PONCHIP_PAS5201 )
			{
        	VOS_MemCpy( MacAddr[0], MacAddr1, BYTES_IN_MAC_ADDRESS );
			ret_val = PAS_set_mac_address_authentication( PonPortIdx, ENABLE, 1, MacAddr);
			if( ret_val == PAS_EXIT_OK ) return( ROK );
			else {
				sys_console_printf("Set %s/port%d onu MacAddr to Authorize list Err%d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ret_val);
				}
			return ( RERROR );
			}
		}
		
	else { /* other pon chip type handler */
			
		return ( RERROR );
		}
#endif

	return( RERROR );
}

int  DelOnuFromAuthorizeListByIdx( short int PonPortIdx, short int  OnuIdx  )
{
	mac_addresses_list_t  MacAddr ;
	int ret, len;
	short int ret_val;
#if 0
	short int PonChipType, PonChipVer = RERROR;
#endif

	CHECK_ONU_RANGE


	if( ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) return( RERROR );
	ret = GetOnuMacAddr( PonPortIdx, OnuIdx, MacAddr[0], &len);
	if( ret != ROK )
		{
		sys_console_printf("Get %s/port%d onu%d MacAddr Err\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx +1));
		return( RERROR );
		}
	
#if 1
	SWAP_TO_ACTIVE_PON_PORT_INDEX_R(PonPortIdx) //PonPortIdx may excced 255

    ret_val = OLT_SetMacAuth(PonPortIdx, DISABLE, MacAddr[0]);			
	if( OLT_CALL_ISOK(ret_val) )
        return( ROK );
	else
		sys_console_printf("Set %s/port%d onu%d MacAddr to Authorize list Err%d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx +1), ret_val);
#else
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
	
	if( PonChipType == PONCHIP_PAS )
		{
		/*PonChipVer = V2R1_GetPonChipVersion( PonPortIdx );*/
		if((PonChipVer != PONCHIP_PAS5001 ) && ( PonChipVer != PONCHIP_PAS5201) ) return( RERROR );

		if( PonChipVer == PONCHIP_PAS5001 )
			{
			return( RERROR );
			}
		else if( PonChipVer == PONCHIP_PAS5201 )
			{
			ret_val = PAS_set_mac_address_authentication( PonPortIdx, DISABLE, 1, MacAddr);
			if( ret_val == PAS_EXIT_OK ) return( ROK );
			else {
				sys_console_printf("Set %s/port%d onu%d MacAddr to Authorize list Err%d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx +1), ret_val);
				}
			}
		}
		
	else { /* other pon chip type handler */
			
		}		
#endif
	
	return( RERROR );	
}

int DelOnuFromAuthorizeListByMac( short int PonPortIdx,  mac_address_t  MacAddr1 )
{
	short int ret_val;
#if 0
	mac_addresses_list_t  MacAddr ;
	short int PonChipType, PonChipVer = RERROR;
#endif

	CHECK_PON_RANGE

	/*if( CompTwoMacAddress( MacAddr1, Invalid_Mac_Addr ) == ROK ) 
		return( RERROR );
	if( CompTwoMacAddress( MacAddr1, Invalid_Mac_Addr1 ) == ROK ) 
		return( RERROR );*/
	if( MAC_ADDR_IS_ZERO(MacAddr1) || MAC_ADDR_IS_BROADCAST(MacAddr1) )
		return( RERROR );

#if 1
        ret_val = OLT_SetMacAuth(PonPortIdx, DISABLE, MacAddr1);			
    	if( OLT_CALL_ISOK(ret_val) )
            return( ROK );
		else
			sys_console_printf("Set %s/port%d onu MacAddr to Authorize list Err%d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ret_val);
#else
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
	
	if( PonChipType == PONCHIP_PAS )
		{
		/*PonChipVer = V2R1_GetPonChipVersion( PonPortIdx );*/
		if((PonChipVer != PONCHIP_PAS5001 ) && ( PonChipVer != PONCHIP_PAS5201) ) return( RERROR );

		if( PonChipVer == PONCHIP_PAS5001 )
			{
			return( RERROR );
			}
		else if( PonChipVer == PONCHIP_PAS5201 )
			{
        	VOS_MemCpy( MacAddr[0], MacAddr1, BYTES_IN_MAC_ADDRESS );
			ret_val = PAS_set_mac_address_authentication( PonPortIdx, ENABLE, 1, MacAddr);
			if( ret_val == PAS_EXIT_OK ) return( ROK );
			else {
				sys_console_printf("Set %s/port%d onu MacAddr to Authorize list Err%d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ret_val);
				}
			}
		return( RERROR );
		}
		
	else { /* other pon chip type handler */
			
		}	
#endif

	return ( RERROR );
}

#if 0
/*****************************************************
 *
 *    Function: pon_init(void)
 *
 *    Param:   none
 *                 
 *    Desc:  initialize the PAS-SOFT 
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int pon_init1 ( void)
{
	short int result = PAS_EXIT_OK;
	PAS_pon_initialization_parameters_t Pas_Init_Param;
	PAS_system_parameters_t  system_parameters;	
	unsigned short  Handler_id[20];

	/**/
	/*mac_address_t olt_mac_address = {0x00,0x44,0x55,0xaa,0xbb,0xcc};*/
	
	/*for PAS_pon_initialization_parameters_t Pas_Init_Param*/
	
	/*Pas_Init_Param.TBD = 1;*/ /*modified for PAS-SOFT V5.1*/
	Pas_Init_Param.automatic_authorization_policy = 1;

	/*system_parameters.TBD = 1;ENABLE*/ /*modified for PAS-SOFT V5.1*/
	system_parameters.statistics_sampling_cycle = 0;
	system_parameters.monitoring_cycle = 0;
	system_parameters.host_olt_msgs_timeout = -1;
	system_parameters.olt_reset_timeout = 0;
	system_parameters.automatic_authorization_policy = 1;

	PAS_terminate ();
	
	/*mac_address_t olt_mac_address = {0x00,0x44,0x55,0xaa,0xbb,0xcc};*/
	/* InitOltMacaddr(); */
	if(PAS_EXIT_OK != PAS_init() ) /*  PAS_init(&Pas_Init_Param), modified for PAS-SOFT V5.1*/
		{
			sys_console_printf("PAS_init faile !\r\n");
			return (PAS_EXIT_ERROR);
		}
	else
		{
			sys_console_printf("PAS_init success!\r\n");
		}
	/* 2. PAS_assign_handler_function */
	/*modified for PAS-SOFT V5.1*/
	if ((PAS_assign_handler_function (PAS_HANDLER_FRAME_RECEIVED,					(void*)*Ethernet_frame_received_handler, &Handler_id[0] )			     == PAS_EXIT_OK) &&
		(PAS_assign_handler_function (PAS_HANDLER_ONU_REGISTRATION,					(void*)*Onu_registration_handler/* or GRANT_update*/, &Handler_id[1])    == PAS_EXIT_OK) &&
		(PAS_assign_handler_function (PAS_HANDLER_OLT_RESET,						(void*)*Olt_reset_handler, &Handler_id[2])								 == PAS_EXIT_OK) &&
		(PAS_assign_handler_function (PAS_HANDLER_ALARM,							(void*)*Alarm_handler, &Handler_id[3])									 == PAS_EXIT_OK) &&
		(PAS_assign_handler_function (PAS_HANDLER_ONU_DEREGISTRATION,				(void*)*Onu_deregistration_handler, &Handler_id[4])				         == PAS_EXIT_OK) &&
		(PAS_assign_handler_function (PAS_HANDLER_START_ENCRYPTION_ACKNOWLEDGE,		(void*)*Start_encryption_acknowledge_handler, &Handler_id[5])			 == PAS_EXIT_OK) &&
		(PAS_assign_handler_function (PAS_HANDLER_UPDATE_ENCRYPTION_KEY_ACKNOWLEDGE,(void*)*Update_encryption_key_acknowledge_handler, &Handler_id[6])		 == PAS_EXIT_OK) &&
		(PAS_assign_handler_function (PAS_HANDLER_STOP_ENCRYPTION_ACKNOWLEDGE,		(void*)*Stop_encryption_acknowledge_handler, &Handler_id[7])			 == PAS_EXIT_OK) &&
		(PAS_assign_handler_function (PAS_HANDLER_ONU_AUTHORIZATION,				(void*)*Onu_authorization_handler, &Handler_id[8])						 == PAS_EXIT_OK) )

	{
		sys_console_printf ("Handler functions assignment success\n");
	}
	else
	{
		sys_console_printf ("Handler functions assignment failure\n");
		return (RERROR);
	}
	
	result = PAS_set_system_parameters ( &system_parameters );
	if(result == PAS_EXIT_OK)
		{
		 sys_console_printf(" PAS set system parameters ok \r\n");
		 return( result );
		 
		}
	else
		{
		sys_console_printf(" PAS set system parameters err \r\n");
		return(result);
		}	
}
#endif	

extern LONG onuAuthCtc_init();

/* B--added by liwei056@2010-7-20 for Flash-VFS Support */
void pon_load_init()
{
    VOS_MemZero(PON_FirmwareMemBuf, sizeof(PON_FirmwareMemBuf));
    VOS_MemZero(PON_DBAMemBuf, sizeof(PON_DBAMemBuf));
    
    VOS_MemZero(PON_FirmwareMemLen, sizeof(PON_FirmwareMemLen));
    VOS_MemZero(PON_DBAMemLen, sizeof(PON_DBAMemLen));

    PON_LoadRescRefCnt    = 0;
    PON_LoadRescClearFlag = 0;

    return;
}

void pon_load_clear()
{
    int i;
	VOS_TaskLock();
    VOS_ASSERT(0 == PON_LoadRescClearFlag);
    VOS_ASSERT(0 == PON_LoadRescRefCnt);

    PON_LoadRescClearFlag = 0;
    PON_LoadRescRefCnt = 0;

    for (i=0; i<PONCHIP_TYPE_MAX; i++)
    {
        if (NULL != PON_FirmwareMemBuf[i])
        {
            g_free(PON_FirmwareMemBuf[i]);
            
            PON_FirmwareMemBuf[i] = NULL;
            PON_FirmwareMemLen[i] = 0;
        }

        if (NULL != PON_DBAMemBuf[i])
        {
            g_free(PON_DBAMemBuf[i]);

            PON_DBAMemBuf[i] = NULL;
            PON_DBAMemLen[i] = 0;
        }
    }
	VOS_TaskUnlock();
    return;
}

void pon_load_get(int PonChipType, char** FwLocation, unsigned int *FwSize, char** DbaLocation, unsigned int *DbaSize)
{
    OLT_PONCHIP_ASSERT(PonChipType);

    VOS_TaskLock();    
    if (NULL != FwLocation)
    {
        if (NULL != (*FwLocation = PON_FirmwareMemBuf[PonChipType]))
        {          
            *FwSize = PON_FirmwareMemLen[PonChipType];
            
            PON_LoadRescRefCnt++;  
            PON_LoadRescClearFlag = 0;
        }
    }

    if (NULL != DbaLocation)
    {
        if (NULL != (*DbaLocation = PON_DBAMemBuf[PonChipType]))
        {
            *DbaSize = PON_DBAMemLen[PonChipType];
                
            PON_LoadRescRefCnt++;  
            PON_LoadRescClearFlag = 0;
        }
    }
    VOS_TaskUnlock();

    return; 
}

void pon_load_set(int PonChipType, char* FwLocation, unsigned int FwSize, char* DbaLocation, unsigned int DbaSize)
{
    OLT_PONCHIP_ASSERT(PonChipType);

    VOS_TaskLock();    
    if (NULL != FwLocation)
    {
        VOS_ASSERT(NULL == PON_FirmwareMemBuf[PonChipType]);

        PON_FirmwareMemBuf[PonChipType] = FwLocation;
        PON_FirmwareMemLen[PonChipType] = FwSize;
    }

    if (NULL != DbaLocation)
    {
        VOS_ASSERT(NULL == PON_DBAMemBuf[PonChipType]);

        PON_DBAMemBuf[PonChipType] = DbaLocation;
        PON_DBAMemLen[PonChipType] = DbaSize;
    }
    VOS_TaskUnlock();

    return; 
}

void pon_load_release(int PonChipType, int FwFlag, int DbaFlag)
{
    OLT_PONCHIP_ASSERT(PonChipType);

    VOS_TaskLock();    
    if (0 < FwFlag)
    {
        if (NULL != PON_FirmwareMemBuf[PonChipType]);
        {
            PON_LoadRescRefCnt--;
        }
    }

    if (0 < DbaFlag)
    {
        if (NULL != PON_DBAMemBuf[PonChipType]);
        {
            PON_LoadRescRefCnt--;
        }
    }

    if ( 0 == PON_LoadRescRefCnt )
    {
        PON_LoadRescClearFlag = PON_LOAD_CLEAR_TIMESEC;
    }
    VOS_TaskUnlock();

    return; 
}

void pon_load_scan_garbage()
{
    VOS_TaskLock();
    if (( 0 == PON_LoadRescRefCnt ) && (PON_LoadRescClearFlag != 0))
    {
        if( 0 == --PON_LoadRescClearFlag )
        {
            pon_load_clear();
            OLT_ADD_DEBUG(OLT_ADD_TITLE"Pon Load End after lose %d sec!\r\n", PON_LOAD_CLEAR_TIMESEC);
        }
    }
    else
    {
        OLT_ADD_DEBUG(OLT_ADD_TITLE"New Pon Load Start after lose %d sec!\r\n", PON_LOAD_CLEAR_TIMESEC - PON_LoadRescClearFlag);
        PON_LoadRescClearFlag = 0;
    }
    VOS_TaskUnlock();

    return; 
}
/* E--added by liwei056@2010-7-20 for Flash-VFS Support */

/* B--added by liwei056@2011-1-7 for CodeCheck */
void pon_ctc_oui_init()
{
	/* modified by xieshl 20110721, 默认打开CTC使能，并从eeprom中获取扩展oui */
	UCHAR def_oui[3] = { (CTC_OUI_DEFINE >> 16) & 0xff, (CTC_OUI_DEFINE >> 8) & 0xff, CTC_OUI_DEFINE & 0xff};
	UCHAR def_ver_list[] = { 1, 0x20, 0x21, 0x30 };/*modi by luh@2016-4-15, support ctc 3.0*/
	UCHAR e2_oui[3], e2_ver;
	int ext_flag = 1;
	int i;

	PAS_oui_version_records_num = 0;
	VOS_MemZero( PAS_oui_version_records_list, sizeof(PAS_oui_version_records_list) );
#if 0
	PAS_oui_version_records_list[PAS_oui_version_records_num].oui[0] = def_oui[0];
	PAS_oui_version_records_list[PAS_oui_version_records_num].oui[1] = def_oui[1];
	PAS_oui_version_records_list[PAS_oui_version_records_num].oui[2] = def_oui[2];
	PAS_oui_version_records_list[PAS_oui_version_records_num].version = CTC_VERSION_DEFINE;
	PAS_oui_version_records_num++;
#ifdef PAS_SOFT_VERSION_V5_3_11
	PAS_oui_version_records_list[PAS_oui_version_records_num].oui[0] = def_oui[0];
	PAS_oui_version_records_list[PAS_oui_version_records_num].oui[1] = def_oui[1];
	PAS_oui_version_records_list[PAS_oui_version_records_num].oui[2] = def_oui[2];
	PAS_oui_version_records_list[PAS_oui_version_records_num].version = 0x20;
	PAS_oui_version_records_num++;
	PAS_oui_version_records_list[PAS_oui_version_records_num].oui[0] = def_oui[0];
	PAS_oui_version_records_list[PAS_oui_version_records_num].oui[1] = def_oui[1];
	PAS_oui_version_records_list[PAS_oui_version_records_num].oui[2] = def_oui[2];
	PAS_oui_version_records_list[PAS_oui_version_records_num].version = CTC_2_1_ONU_VERSION;
	PAS_oui_version_records_num++;

	PAS_oui_version_records_list[PAS_oui_version_records_num].oui[0] = def_oui[0];
	PAS_oui_version_records_list[PAS_oui_version_records_num].oui[1] = def_oui[1];
	PAS_oui_version_records_list[PAS_oui_version_records_num].oui[2] = def_oui[2];
	PAS_oui_version_records_list[PAS_oui_version_records_num].version = 0xc1;		/* added by xieshl 20110414, 联通扩展OAM版本 */
	PAS_oui_version_records_num++;
#endif
#else
	for( i=0; i<sizeof(def_ver_list)/sizeof(def_ver_list[0]); i++ )
	{
		PAS_oui_version_records_list[i].oui[0] = def_oui[0];
		PAS_oui_version_records_list[i].oui[1] = def_oui[1];
		PAS_oui_version_records_list[i].oui[2] = def_oui[2];
		PAS_oui_version_records_list[i].version = def_ver_list[i];
		PAS_oui_version_records_num++;
	}
#endif

	if( getCtcStackOui(SYS_LOCAL_MODULE_SLOTNO, e2_oui, &e2_ver) == VOS_OK )
	{
		if( VOS_MemCmp( e2_oui, def_oui, 3) == 0 )
		{
			for( i=0; i<sizeof(def_ver_list)/sizeof(def_ver_list[0]); i++ )
			{
				if( e2_ver == def_ver_list[i] )
				{
					ext_flag = 0;
					break;
				}
			}
		}
		if( ext_flag )
		{
			PAS_oui_version_records_list[PAS_oui_version_records_num].oui[0] = e2_oui[0];
			PAS_oui_version_records_list[PAS_oui_version_records_num].oui[1] = e2_oui[1];
			PAS_oui_version_records_list[PAS_oui_version_records_num].oui[2] = e2_oui[2];
			PAS_oui_version_records_list[PAS_oui_version_records_num].version = e2_ver;
			PAS_oui_version_records_num++;
		}
	}
	
}
/* E--added by liwei056@2011-1-7 for CodeCheck */

int pon_init()
{
	short int result;

	/* added by chenfj 2008-5-16 */
	InitPonMsgDebugFlag();
	ClearPonDownloadCounter();
	ClearPonActivatedFlag();
    pon_load_init();

	/*PAS_init_para.TBD = ENABLE; 	modified for PAS-SOFT V5.1*/
	PAS_init_para.automatic_authorization_policy = ONU_MANUAL_AUTHENTICATION;
	pon_initialization_parameters_init();
	pon_set_update_parameters();
	pon_cni_port_init();
    pon_ctc_oui_init();
    
    if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
    {
        if ( SYS_LOCAL_MODULE_TYPE_IS_PAS_PONCARD_MANAGER )
        {
#if defined(_EPON_10G_PMC_SUPPORT_)            
	        /*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	    	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
			{
		    	GW8411_pon_init();
			}
			else
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif
			{
				GW5201_pon_init();
	    	}
    	}
        else if ( SYS_LOCAL_MODULE_TYPE_IS_TK_PONCARD_MANAGER )
        {
#if defined(_EPON_10G_BCM_SUPPORT_)            
            if ( SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER )
            {
                BCM55538_pon_init();
            }
            else
#endif
            {
                TK3723_pon_init();
            }
        }
		#if defined(_GPON_BCM_SUPPORT_)
		else if (SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER)
		{
			GPON_pon_init();
		}
		#endif
        else
        {
            VOS_ASSERT(0);
        }

       	VOS_TaskDelay( 1 );
    }

    /* modified by xieshl 20110413, 支持LOID认证模式告警, 主控和PON板都应初始化 */
    if (V2R1_CTC_STACK)
    {
        onuAuthCtc_init();
        OnuEventCtc_init();
    }

	return ( ROK );
}



/*****************************************************
 *
 *    Function: pon_terminate(void)
 *
 *    Param:   none
 *                 
 *    Desc:  reset the PAS-SOFT 
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int pon_terminate()
{
	short int  i;

	for( i=0; i< MAXPON; i++ ){
		if(( PonPortTable[i].PortWorkingStatus  ==PONPORT_UP )||(PonPortTable[i].PortWorkingStatus  == PONPORT_LOOP) 
			||( PonPortTable[i].PortWorkingStatus  ==PONPORT_UPDATE ))
			pon_remove_olt(i);
		}

	/* release resource assigned by user routine */
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
    /*for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
    	GW10G_PAS_terminate();

	}
	else
#endif
	{
		PAS_terminate();
    	
	}

	return( ROK );
}

/*****************************************************
 *
 *    Function: pon_cni_port_init(void)
 *
 *    Param:   none
 *                 
 *    Desc:    Initialize pon CNI interface parameter
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
void pon_cni_port_init(void)
{
	PAS_port_cni_parameters.master=PON_MASTER;
	PAS_port_cni_parameters.pause= DISABLE;
	PAS_port_cni_parameters.mdio_phy_address= 0; /* = 7;*/
	PAS_port_cni_parameters.auto_negotiation=DISABLE ; /*DISABLE;*/
	PAS_port_cni_parameters.pause_thresholds.pause_set_threshold= PON_VALUE_NOT_CHANGED;
	PAS_port_cni_parameters.pause_thresholds.pause_release_threshold= PON_VALUE_NOT_CHANGED;
	 
	PAS_port_cni_parameters.advertise= ENABLE;
	PAS_port_cni_parameters.advertisement_details._1000base_tx_half_duplex=FALSE;
	PAS_port_cni_parameters.advertisement_details._1000base_tx_full_duplex=TRUE;
	PAS_port_cni_parameters.advertisement_details.preferable_port_type=PON_MASTER;
	PAS_port_cni_parameters.advertisement_details.pause = TRUE;
	PAS_port_cni_parameters.advertisement_details.asymmetric_pause= TRUE;
}

void get_cni_port_configuration(void)
{

	sys_console_printf("\r\nCNI port paramters : \r\n");
	sys_console_printf("     master=%d\r\n",PAS_port_cni_parameters.master);
	sys_console_printf("     pause=%d\r\n",PAS_port_cni_parameters.pause);
	sys_console_printf("     mdio_phy_address=%d\r\n",PAS_port_cni_parameters.mdio_phy_address);
	sys_console_printf("     auto_negotiation=%d\r\n",PAS_port_cni_parameters.auto_negotiation);
	sys_console_printf("     pause_set_threshold=%d\r\n",PAS_port_cni_parameters.pause_thresholds.pause_set_threshold);
	sys_console_printf("     pause_release_threshold=%d\r\n",PAS_port_cni_parameters.pause_thresholds.pause_release_threshold);
	sys_console_printf("     advertise=%d\r\n",PAS_port_cni_parameters.advertise);
	sys_console_printf("     advertisement_details._1000base_tx_half_duplex=%d\r\n",PAS_port_cni_parameters.advertisement_details._1000base_tx_half_duplex);
	sys_console_printf("     advertisement_details._1000base_tx_full_duplex=%d\r\n",PAS_port_cni_parameters.advertisement_details._1000base_tx_full_duplex);
	sys_console_printf("     advertisement_details.preferable_port_type=%d\r\n",PAS_port_cni_parameters.advertisement_details.preferable_port_type);
	sys_console_printf("     advertisement_details.pause=%d\r\n",PAS_port_cni_parameters.advertisement_details.pause);
	sys_console_printf("     advertisement_details.asymmetric_pause=%d\r\n\r\n",PAS_port_cni_parameters.advertisement_details.asymmetric_pause);
}

/*****************************************************
 *
 *    Function:  pon_initialization_parameters_init(void)
 *
 *    Param:   none
 *                 
 *    Desc:    Initialize pon CNI interface parameter
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
ULONG V2R1_PON_HOST_MANAGE_ADDRESS_BASE2 = 0xee40;
void pon_initialization_parameters_init(void)
{
	VOS_MemSet( &PAS_initialization_parameters_5001, 0, sizeof( PON_olt_init_parameters_t ) );
#if 1
	PAS_initialization_parameters_5001.optics_configuration.configuration_source = PON_OLT_OPTICS_CONFIGURATION_SOURCE_HOST;
	PAS_initialization_parameters_5001.optics_configuration.agc_lock_time = 0x09; /* changed from liudong 0x19*/
	PAS_initialization_parameters_5001.optics_configuration.agc_reset_configuration.gate_offset = 0x12; /* 0-4095 */
	PAS_initialization_parameters_5001.optics_configuration.agc_reset_configuration.discovery_offset = 0x12; /* 0-4095 */
	PAS_initialization_parameters_5001.optics_configuration.agc_reset_configuration.duration = 0x0;/* 0-7 */
	PAS_initialization_parameters_5001.optics_configuration.agc_reset_configuration.polarity = PON_POLARITY_ACTIVE_LOW;
	
	PAS_initialization_parameters_5001.optics_configuration.cdr_lock_time = 0x10; /*changed from liudong 0x9d*/
	PAS_initialization_parameters_5001.optics_configuration.cdr_reset_configuration.gate_offset = 0x20; /*0~4095*/
	PAS_initialization_parameters_5001.optics_configuration.cdr_reset_configuration.discovery_offset = 0x20; /*0~4095*/
	PAS_initialization_parameters_5001.optics_configuration.cdr_reset_configuration.duration = 0x0;
	PAS_initialization_parameters_5001.optics_configuration.cdr_reset_configuration.polarity = PON_POLARITY_ACTIVE_HIGH;

	/* modified for PAS-SOFT V5.1, end_of_grant_reset_configuration changed to cdr_end_of_grant_reset_configuration */
	PAS_initialization_parameters_5001.optics_configuration.cdr_end_of_grant_reset_configuration.offset = 0x24;/*changed from liudong 0;*/
	PAS_initialization_parameters_5001.optics_configuration.cdr_end_of_grant_reset_configuration.duration = 2;
	PAS_initialization_parameters_5001.optics_configuration.cdr_end_of_grant_reset_configuration.polarity = PON_POLARITY_ACTIVE_LOW;
	/* modified for PAS-SOFT V5.1, add optics_end_of_grant_reset_configuration */
	PAS_initialization_parameters_5001.optics_configuration.optics_end_of_grant_reset_configuration.offset = 0x24;
	PAS_initialization_parameters_5001.optics_configuration.optics_end_of_grant_reset_configuration.duration = 2;
	PAS_initialization_parameters_5001.optics_configuration.optics_end_of_grant_reset_configuration.polarity = PON_POLARITY_ACTIVE_LOW;
	
	PAS_initialization_parameters_5001.optics_configuration.discovery_re_locking_enable = DISABLE;
	PAS_initialization_parameters_5001.optics_configuration.discovery_laser_rx_loss_polarity = PON_POLARITY_ACTIVE_LOW;
	PAS_initialization_parameters_5001.optics_configuration.pon_tx_disable_line_polarity = PON_POLARITY_ACTIVE_LOW;
	PAS_initialization_parameters_5001.optics_configuration.optics_dead_zone = 0xa; /* 0-15*/
	PAS_initialization_parameters_5001.optics_configuration.use_optics_signal_loss = FALSE; /* changed from liudong TRUE; */
	PAS_initialization_parameters_5001.optics_configuration.polarity_configuration.pon_port_link_indication_polarity = PON_POLARITY_ACTIVE_LOW;
	PAS_initialization_parameters_5001.optics_configuration.polarity_configuration.cni_port_link_indication_polarity = PON_POLARITY_ACTIVE_HIGH;
	/* modified for PAS-SOFT V5.1*//* add cni_port_link_indication_polarity.pon_tbc_polarity */
	PAS_initialization_parameters_5001.optics_configuration.polarity_configuration.pon_tbc_polarity = PON_POLARITY_ACTIVE_LOW;
	
	PAS_initialization_parameters_5001.discovery_laser_on_time = 0x20; /* 0x20;  modified for PAS-SOFT V5.1*/
	PAS_initialization_parameters_5001.discovery_laser_off_time = 0x20; /* 0x20;  modified for PAS-SOFT V5.1*/
	
	PAS_initialization_parameters_5001.igmp_configuration.enable_igmp_snooping = DISABLE;
	PAS_initialization_parameters_5001.igmp_configuration.igmp_timeout = 0; /*PON_NO_IGMP_TIMEOUT 0-9830 */	
	PAS_initialization_parameters_5001.vlan_configuration.vlan_exchange_downlink_tag_prefix =  200; /*;modified for PAS-SOFT V5.1*/
	PAS_initialization_parameters_5001.vlan_configuration.nested_mode_vlan_type = 0x7153;/*  modified for PAS-SOFT V5.1*/ /* 0x9100, 0x88a8 */	
	PAS_initialization_parameters_5001.multiple_copy_broadcast_enable = DISABLE;/* DISABLE;*/
	PAS_initialization_parameters_5001.discard_unlearned_addresses = TRUE ; /*FALSE ; modified for PAS-SOFT V5.1*/
	/*
	PAS_initialization_parameters.firmware_image.type = PON_OLT_BINARY_FIRMWARE;
	PAS_initialization_parameters.firmware_image.source = PON_BINARY_SOURCE_MEMORY;
	PAS_initialization_parameters.firmware_image.location = Pon_Firmware_image[] (void *)( FLASH_BASE + PAS_FIRMWARE_IMAGE_OFFSET);
	PAS_initialization_parameters.firmware_image.size = 142336;
	*/
	/* set_image_src( PONCHIP_PAS5001, PON_BINARY_SOURCE_MEMORY, &(PAS_initialization_parameters_5001.firmware_image)); */
#endif

#if 1 
	/* modified for PAS-SOFT V5.1*/
	VOS_MemSet( &PAS_initialization_parameters_5201, 0, sizeof( PON_olt_initialization_parameters_t) );

	VOS_MemSet( PAS_initialization_parameters_5201.olt_mac_address, 0, BYTES_IN_MAC_ADDRESS );

	PAS_initialization_parameters_5201.optics_configuration.configuration_source = PON_OLT_OPTICS_CONFIGURATION_SOURCE_HOST;
	PAS_initialization_parameters_5201.optics_configuration.agc_lock_time = 0x09; /* changed from liudong 0x19*/
	PAS_initialization_parameters_5201.optics_configuration.agc_reset_configuration.gate_offset = 0x12; /* 0-4095 */
	PAS_initialization_parameters_5201.optics_configuration.agc_reset_configuration.discovery_offset = 0x12; /* 0-4095 */
	PAS_initialization_parameters_5201.optics_configuration.agc_reset_configuration.duration = 0x0;/* 0-7 */
	PAS_initialization_parameters_5201.optics_configuration.agc_reset_configuration.polarity = PON_POLARITY_ACTIVE_LOW;
	
	PAS_initialization_parameters_5201.optics_configuration.cdr_lock_time = 0x10; /*changed from liudong 0x9d*/
	PAS_initialization_parameters_5201.optics_configuration.cdr_reset_configuration.gate_offset = 0x20; /*0~4095*/
	PAS_initialization_parameters_5201.optics_configuration.cdr_reset_configuration.discovery_offset = 0x20; /*0~4095*/
	PAS_initialization_parameters_5201.optics_configuration.cdr_reset_configuration.duration = 0x0;
	PAS_initialization_parameters_5201.optics_configuration.cdr_reset_configuration.polarity = PON_POLARITY_ACTIVE_HIGH;

	/* modified for PAS-SOFT V5.1, end_of_grant_reset_configuration changed to cdr_end_of_grant_reset_configuration */
	PAS_initialization_parameters_5201.optics_configuration.cdr_end_of_grant_reset_configuration.offset = 0x24;/*changed from liudong 0;*/
	PAS_initialization_parameters_5201.optics_configuration.cdr_end_of_grant_reset_configuration.duration = 2;
	PAS_initialization_parameters_5201.optics_configuration.cdr_end_of_grant_reset_configuration.polarity = PON_POLARITY_ACTIVE_LOW;
	/* modified for PAS-SOFT V5.1, add optics_end_of_grant_reset_configuration */
	PAS_initialization_parameters_5201.optics_configuration.optics_end_of_grant_reset_configuration.offset = 0x24;
	PAS_initialization_parameters_5201.optics_configuration.optics_end_of_grant_reset_configuration.duration = 2;
	PAS_initialization_parameters_5201.optics_configuration.optics_end_of_grant_reset_configuration.polarity = PON_POLARITY_ACTIVE_LOW;
	
	PAS_initialization_parameters_5201.optics_configuration.discovery_re_locking_enable = DISABLE;
	PAS_initialization_parameters_5201.optics_configuration.discovery_laser_rx_loss_polarity = PON_POLARITY_ACTIVE_LOW;
	PAS_initialization_parameters_5201.optics_configuration.pon_tx_disable_line_polarity = PON_POLARITY_ACTIVE_LOW;
	PAS_initialization_parameters_5201.optics_configuration.optics_dead_zone = 0xa; /* 0-15*/
	PAS_initialization_parameters_5201.optics_configuration.use_optics_signal_loss = FALSE; /* changed from liudong TRUE; */
	PAS_initialization_parameters_5201.optics_configuration.polarity_configuration.pon_port_link_indication_polarity = PON_POLARITY_ACTIVE_LOW;
	PAS_initialization_parameters_5201.optics_configuration.polarity_configuration.cni_port_link_indication_polarity = PON_POLARITY_ACTIVE_HIGH;
	/* modified for PAS-SOFT V5.1*//* add cni_port_link_indication_polarity.pon_tbc_polarity */
	PAS_initialization_parameters_5201.optics_configuration.polarity_configuration.pon_tbc_polarity = PON_POLARITY_ACTIVE_LOW;
	
	PAS_initialization_parameters_5201.optics_configuration.discovery_laser_on_time = 0x20; /* 0x60;  modified for PAS-SOFT V5.1*/
	PAS_initialization_parameters_5201.optics_configuration.discovery_laser_off_time = 0x20; /* 0x60;  modified for PAS-SOFT V5.1*/

	PAS_initialization_parameters_5201.external_downlink_buffer_size = PON_EXTERNAL_DOWNLINK_BUFFER_0MB;
	PAS_initialization_parameters_5201.support_passave_onus = TRUE;
	PAS_initialization_parameters_5201.ram_test = TRUE;
		
	/* set_image_src( PONCHIP_PAS5201, PON_BINARY_SOURCE_MEMORY, &(PAS_initialization_parameters_5201.firmware_image)); */

#endif
/*
	PON_olt_igmp_configuration_t	 igmp_configuration;
	PON_olt_vlan_configuration_t	 vlan_configuration;
	bool							 multiple_copy_broadcast_enable;
	bool							 discard_unlearned_addresses;
*/
/*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
#if 1
    VOS_MemSet( &PAS_initialization_parameters_8411, 0, sizeof( GW10G_PON_olt_initialization_parameters_t) );

	VOS_MemSet( PAS_initialization_parameters_8411.olt_mac_address, 0, BYTES_IN_MAC_ADDRESS );

	
#endif
#if 0

	PAS_initialization_parameters.optics_configuration.configuration_source = 1;
	PAS_initialization_parameters.optics_configuration.agc_lock_time = 0x09;
	PAS_initialization_parameters.optics_configuration.agc_reset_configuration.gate_offset = 0x12;
	PAS_initialization_parameters.optics_configuration.agc_reset_configuration.discovery_offset = 0x12;
	PAS_initialization_parameters.optics_configuration.agc_reset_configuration.duration = 0x0;
	PAS_initialization_parameters.optics_configuration.agc_reset_configuration.polarity = 0;
	PAS_initialization_parameters.optics_configuration.cdr_lock_time = 0x10;
	PAS_initialization_parameters.optics_configuration.cdr_reset_configuration.gate_offset = 0x20;
	PAS_initialization_parameters.optics_configuration.cdr_reset_configuration.discovery_offset = 0x20;
	PAS_initialization_parameters.optics_configuration.cdr_reset_configuration.duration = 0x0;
	PAS_initialization_parameters.optics_configuration.cdr_reset_configuration.polarity = 1;
	PAS_initialization_parameters.optics_configuration.end_of_grant_reset_configuration.offset = 0x24;
	PAS_initialization_parameters.optics_configuration.end_of_grant_reset_configuration.duration = 2;
	PAS_initialization_parameters.optics_configuration.end_of_grant_reset_configuration.polarity = 0;
	PAS_initialization_parameters.optics_configuration.discovery_re_locking_enable = 0;
	PAS_initialization_parameters.optics_configuration.discovery_laser_rx_loss_polarity = 0;
	PAS_initialization_parameters.optics_configuration.pon_tx_disable_line_polarity = 0;
	PAS_initialization_parameters.optics_configuration.optics_dead_zone = 0xa;
	PAS_initialization_parameters.optics_configuration.use_optics_signal_loss = 0;
	/*PON_olt_optics_polarity_configuration_t */
	PAS_initialization_parameters.optics_configuration.polarity_configuration.pon_port_link_indication_polarity = 0;
	PAS_initialization_parameters.optics_configuration.polarity_configuration.cni_port_link_indication_polarity = 1;
	/**/
	PAS_initialization_parameters.discovery_laser_on_time = 0x20;
	PAS_initialization_parameters.discovery_laser_off_time = 0x20;
	PAS_initialization_parameters.igmp_configuration.enable_igmp_snooping = 0;
	PAS_initialization_parameters.igmp_configuration.igmp_timeout = 0;
	PAS_initialization_parameters.vlan_configuration.vlan_exchange_downlink_tag_prefix = 200;
	PAS_initialization_parameters.vlan_configuration.nested_mode_vlan_type = 0x7153;
	PAS_initialization_parameters.multiple_copy_broadcast_enable = 0;
	PAS_initialization_parameters.discard_unlearned_addresses = 1;
	/*modify*/
	PAS_initialization_parameters.firmware_image.type = 1;
	PAS_initialization_parameters.firmware_image.source = PON_BINARY_SOURCE_MEMORY;
	PAS_initialization_parameters.firmware_image.location = FLASH_BASE + PAS_FIRMWARE_IMAGE_OFFSET;
	PAS_initialization_parameters.firmware_image.size = 142336;


	PAS_initialization_parameters.firmware_image.type = PON_OLT_BINARY_FIRMWARE;
	PAS_initialization_parameters.firmware_image.source = PON_BINARY_SOURCE_MEMORY;
	PAS_initialization_parameters.firmware_image.location = NULL; /* a pointer to the memory */
	PAS_initialization_parameters.firmware_image.size = 0; /* the size of the firmware image */
	/*PAS_initialization_parameters.firmware_image.identification.user_id = {0};
	PAS_initialization_parameters.firmware_image.identification.key= {0};*/
#endif	

#if 1 
	VOS_MemSet( &TK_initialization_parameters_55524, 0, sizeof(TK_initialization_parameters_55524) );
    TK_initialization_parameters_55524.support_tk_onus = TRUE;
#endif	

#if defined(_GPON_BCM_SUPPORT_)
	VOS_MemSet( &GPON_initialization_parameters, 0, sizeof(GPON_initialization_parameters) );
#endif

#if 1
    /* B--added by liwei056@2010-11-22 for D11211 */
    PAS_system_parameters.statistics_sampling_cycle = V2R1_PON_SAMPLING_CYCLE;
    PAS_system_parameters.monitoring_cycle = V2R1_PON_MONITORING_CYCLE;
    PAS_system_parameters.host_olt_msgs_timeout = V2R1_PON_HOST_MSG_TIMEOUT;
    PAS_system_parameters.olt_reset_timeout= V2R1_PON_HOST_MSG_OUT;
    PAS_system_parameters.automatic_authorization_policy = ONU_MANUAL_AUTHENTICATION;
    /* E--added by liwei056@2010-11-22 for D11211 */
#endif	

    /* B--added by liwei056@2010-7-22 for 6900-LocalBus */
    {
        int result;
       	unsigned short type, base;

        base = 0;
        if ( PRODUCT_IS_H_Series(SYS_PRODUCT_TYPE) )
        {
            type = V2R1_PON_HOST_MANAGE_BY_BUS;
            
            if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
            {
                if ( SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER )
                {
                    base = 0;
                }
                else
                {
                    unsigned char bCPLDType = 0;
                    
                    /* 根据CPLD版本，采用对应的触发方式 */
                    ReadCPLDReg((volatile UCHAR*)0, &bCPLDType);
                    
				if(SYS_LOCAL_MODULE_TYPE_IS_8100_EPON)
				{
					base = V2R1_PON_HOST_MANAGE_ADDRESS_BASE2;
				}
				else
				{
                    if ( 3 > bCPLDType )
                    {
                        base = V2R1_PON_HOST_MANAGE_ADDRESS_BASE0;
                    }
                    else
                    {
                        base = V2R1_PON_HOST_MANAGE_ADDRESS_BASE1;
                    }
				}
                }
                
		   #if 0 
		   pon_int_config();
		   #endif
            }
        }
        else
        {
            type = V2R1_PON_HOST_MANAGE_BY_ETH;
        }

        result = pon_set_init_parameters(type, base);
    	if(result != 0)
    	{
    		sys_console_printf("error(%d): pon_set_init_parameters(%d, 0x%x) !(pon_initialization_parameters_init)\r\n", result, type, base);
    		VOS_ASSERT(0);
    	}
    }
    /* E--added by liwei056@2010-7-22 for 6900-LocalBus */
}

int GetPonOpticsParam( short int PonPortIdx )
{
   
	short int PonChipType;
	
	CHECK_PON_RANGE
    
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if(OLT_PONCHIP_ISPAS(PonChipType))
	{
	    if(OLT_PONCHIP_ISPAS5001(PonChipType))
		{

		}
	    else
		{   
		    #if 1
			OLT_optics_detail_t optics_params;
			VOS_MemZero(&optics_params, sizeof(OLT_optics_detail_t));
			OLT_GetOpticsDetail(PonPortIdx, &optics_params);
			#else
			PON_olt_optics_configuration_t  optics_configuration;
			bool  pon_tx_signal;
				
			PAS_get_olt_optics_parameters( PonPortIdx , &optics_configuration, &pon_tx_signal );
			#endif
		}
	
	}/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

	/* other pon chip type handler */
	else {

		}
		
	return( RERROR );
}

/*****************************************************
 *
 *    Function:  pon_set_system_parameters(short int statistics_sampling_cycle,short int monitoring_cycle,short int host_olt_msg_timeout)
 *
 *    Param:   short int statistics_sampling_cycle --
 *                short int monitoring_cycle -- 
 *                short int host_olt_msg_timeout --
 *                 
 *    Desc:    Initialize the system parameters
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int pon_set_system_parameters(long int statistics_sampling_cycle, long int monitoring_cycle, short int host_olt_msg_timeout, short int olt_reset_timeout)
{
	short int result;

#if 1
    result = OLT_SetSystemParams(OLT_ID_ALL, statistics_sampling_cycle, monitoring_cycle, host_olt_msg_timeout, olt_reset_timeout);
#else
	/* modified for PAS-SOFT V5.1*/
	/*PAS_system_parameters.TBD= 1 ; *//*PON_VALUE_NOT_CHANGED */
	
	/* statistics sampling cycle, stated in Milliseconds. Maximum value corresponds to one hour: 10 -3600000 */
	/* -1: not changed, 0: stop statistics sampling, 1: continuous statistics sampling                                 */
	PAS_system_parameters.statistics_sampling_cycle=statistics_sampling_cycle ;
	/* monitoring cycle, stated in Milliseconds. Maximum value corresponds to one hour: 10 -3600000 */
	/* -1: not changed, 0: stop monitoring, 1: continuous monitoring                                            */
	PAS_system_parameters.monitoring_cycle=  monitoring_cycle ;
	/* timeout waiting to a response form OLT firmware after sending it a msg; stated in milliseconds.  */
	/* -1: not changed, 1-10000 : Maximum value corresponds to 10 seconds                                 */
	PAS_system_parameters.host_olt_msgs_timeout=host_olt_msg_timeout;	/*milliseconds*/
	/* the number of msgs sending timeouts after which olt is reset  */
	/* -1: not changed,  0: never reset the olt, 1-32767 : the max number of msgs */
	PAS_system_parameters.olt_reset_timeout=  olt_reset_timeout /*V2R1_PON_HOST_MSG_OUT / PON_VALUE_NOT_CHANGED*/ ;  

	/* modified for PAS-SOFT V5.1*/
	PAS_system_parameters.automatic_authorization_policy= PAS_init_para.automatic_authorization_policy;
	
	result=PAS_set_system_parameters (&PAS_system_parameters );
#endif   

	if( OLT_CALL_ISOK(result) )
        return ( ROK );

	/*sys_console_printf("error: set system parameters (file %s pon_set_system_parameters())\r\n\r\n ", __FILE__ );*/
	return ( RERROR );	
}

int Pon_Get_System_parameters(short int *statistics_sampling_cycle,short int *monitoring_cycle,short int *host_olt_msg_timeout, short int  *olt_reset_timeout)
{
#if 1
	if(statistics_sampling_cycle != NULL)
        *statistics_sampling_cycle = PAS_system_parameters.statistics_sampling_cycle;

    if(monitoring_cycle != NULL)
        *monitoring_cycle = PAS_system_parameters.monitoring_cycle;

    if(host_olt_msg_timeout != NULL)
        *host_olt_msg_timeout = PAS_system_parameters.host_olt_msgs_timeout;

    if(olt_reset_timeout != NULL)
        *olt_reset_timeout = PAS_system_parameters.olt_reset_timeout;
#else
	short int result;
	PAS_system_parameters_t PAS_system_param;
	
	result=PAS_get_system_parameters (&PAS_system_param );

	if( result == PAS_EXIT_OK )
	{
		if(EVENT_DEBUG == V2R1_ENABLE )
		{
			sys_console_printf("\r\n get system parameters OK\r\n");
			/*sys_console_printf("statistic sampling:%d, monitoring: %d \r\n",PAS_system_param.statistics_sampling_cycle, PAS_system_param.monitoring_cycle );
			sys_console_printf("Host<-->OLT msg timeout: %d, reset counter: %d \r\n", PAS_system_param.host_olt_msgs_timeout, PAS_system_param.olt_reset_timeout );
			sys_console_printf("ONU register Auth:%d\r\n", PAS_system_param.automatic_authorization_policy );*/
		}
        
		if(statistics_sampling_cycle != NULL) *statistics_sampling_cycle = PAS_system_param.statistics_sampling_cycle;
		if(monitoring_cycle != NULL) *monitoring_cycle = PAS_system_param.monitoring_cycle;
		if(host_olt_msg_timeout != NULL)  *host_olt_msg_timeout = PAS_system_param.host_olt_msgs_timeout;
		if(olt_reset_timeout != NULL) *olt_reset_timeout = PAS_system_param.olt_reset_timeout;
		return( ROK );
	}
	else
	{
		sys_console_printf("\r\nget system parameters err \r\n");
		return( RERROR );
	}	
#endif
	return( ROK );
}

/* B--added by liwei056@2010-7-22 for 6900-LocalBus */
int  pon_set_init_parameters(unsigned short host_manage_iftype, unsigned short host_manage_address)
{
    int result;

    result = OLT_SetInitParams(0, host_manage_iftype, host_manage_address);

    return result;
}

int  pon_get_init_parameters(short int *host_manage_iftype)
{
    if (NULL != host_manage_iftype)
        *host_manage_iftype = PON_init_params.host_manage_iftype;
    
    return 0;
}
/* E--added by liwei056@2010-7-22 for 6900-LocalBus */

void  pon_set_update_parameters()
{
	/*PON_olt_update_parameters_t updated_parameters;*/
	
	PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT(PAS_updated_parameters_5001);
		
	PAS_updated_parameters_5001.max_rtt = PON_VALUE_NOT_CHANGED;/* PON_VALUE_NOT_CHANGED; PON_MAX_RTT_40KM;*/
	PAS_updated_parameters_5001.address_table_aging_timer = DEFAULT_MAC_AGING_TIME; /*PON_VALUE_NOT_CHANGED;*/
	PAS_updated_parameters_5001.cni_port_maximum_entries = PON_ADDRESS_TABLE_ENTRY_LIMITATION_0;
	
	PAS_updated_parameters_5001.arp_filtering_configuration.from_pon_to_firmware = DISABLE;
	PAS_updated_parameters_5001.arp_filtering_configuration.from_cni_to_firmware = DISABLE;
	PAS_updated_parameters_5001.arp_filtering_configuration.from_pon_to_cni = ENABLE;
	PAS_updated_parameters_5001.arp_filtering_configuration.from_cni_to_pon = ENABLE;
	
	PAS_updated_parameters_5001.igmp_configuration.enable_igmp_snooping = DISABLE;
	PAS_updated_parameters_5001.igmp_configuration.igmp_timeout = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_5001.vlan_configuration.vlan_exchange_downlink_tag_prefix = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_5001.vlan_configuration.nested_mode_vlan_type = PON_VALUE_NOT_CHANGED;
	
	PAS_updated_parameters_5001.hec_configuration.tx_hec_configuration = 1;
	PAS_updated_parameters_5001.hec_configuration.rx_hec_configuration = 3;
	
	PAS_updated_parameters_5001.vlan_tag_filtering_config.untagged_frames_filtering = DISABLE;
	PAS_updated_parameters_5001.vlan_tag_filtering_config.multiple_copy_broadcast_enable_mode = DISABLE;
	PAS_updated_parameters_5001.vlan_tag_filtering_config.filtering_unexpected_tagged_downstream_frames= DISABLE;
	PAS_updated_parameters_5001.upstream_default_priority = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_5001.pon_tbc_polarity = PON_POLARITY_ACTIVE_LOW;

	PON_EMPTY_UPDATE_OLT_PARAMETERS_STRUCT( PAS_updated_parameters_5201 );
	PAS_updated_parameters_5201.max_rtt = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_5201.hec_configuration.tx_hec_configuration = PON_TX_HEC_802_AH_MODE;
	PAS_updated_parameters_5201.hec_configuration.rx_hec_configuration = PON_RX_HEC_802_AH_MODE;
	PAS_updated_parameters_5201.grant_filtering = PON_VALUE_NOT_CHANGED; /* DISABLE or ENABLE, when working with PAS5001, must be not_changed */
	PAS_updated_parameters_5201.support_passave_onus = TRUE;

	/*Begin: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
	PAS_updated_parameters_8411.max_rtt = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_8411.min_rtt = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_8411.grant_filtering = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_8411.vendor_specific = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_8411.oui = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_8411.baud_rate = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_8411.llid_assignment = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_8411.sled_strobe = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_8411.sled_mute = PON_VALUE_NOT_CHANGED;
	PAS_updated_parameters_8411.i2c_speed = PON_VALUE_NOT_CHANGED;
	
	/*End: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
/*
	updated_parameters.max_rtt = -1;
	updated_parameters.address_table_aging_timer = -1;
	updated_parameters.cni_port_maximum_entries = 0;
	
	updated_parameters.arp_filtering_configuration.from_pon_to_firmware = -1;
	updated_parameters.arp_filtering_configuration.from_cni_to_firmware = -1;
	updated_parameters.arp_filtering_configuration.from_pon_to_cni = -1;
	updated_parameters.arp_filtering_configuration.from_cni_to_pon = -1;
	
	updated_parameters.igmp_configuration.enable_igmp_snooping = 0;
	updated_parameters.igmp_configuration.igmp_timeout = -1;
	updated_parameters.vlan_configuration.vlan_exchange_downlink_tag_prefix = -1;
	updated_parameters.vlan_configuration.nested_mode_vlan_type = -1;
	
	updated_parameters.hec_configuration.tx_hec_configuration = 1;
	updated_parameters.hec_configuration.rx_hec_configuration = 3;
	
	updated_parameters.vlan_tag_filtering_config.untagged_frames_filtering = -1;
	updated_parameters.vlan_tag_filtering_config.multiple_copy_broadcast_enable_mode = -1;
	updated_parameters.vlan_tag_filtering_config.filtering_unexpected_tagged_downstream_frames= -1;
	updated_parameters.upstream_default_priority = -1;
	updated_parameters.pon_tbc_polarity = PON_POLARITY_ACTIVE_LOW;

	VOS_MemCpy( &PAS_updated_parameters_5001, &updated_parameters, sizeof(PON_olt_update_parameters_t));
	*/
	return;

}

#if 1
short int update_Pon_parameters_5001(short int PonPortIdx )
{
	PON_olt_update_parameters_t  pon_updated_parameters;
		
	CHECK_PON_RANGE;

	PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT(pon_updated_parameters);
	VOS_MemCpy( &pon_updated_parameters, &PAS_updated_parameters_5001, sizeof(PON_olt_update_parameters_t ) );

	/* added by chenfj 2006/12/12
	     PON MAX RANGE */
	if( PonPortTable[PonPortIdx].range == PON_RANGE_CLOSE )
		pon_updated_parameters.max_rtt =  V2R1_REGISTER_WINDOW_CLOSE;
	if( PonPortTable[PonPortIdx].range == PON_RANGE_40KM ) 
		pon_updated_parameters.max_rtt = PON_MAX_RTT_80KM; /* PON_MAX_RTT_80KM;*/
	else if( PonPortTable[PonPortIdx].range == PON_RANGE_60KM ) 
		pon_updated_parameters.max_rtt = PON_MAX_RTT_120KM;
	
	pon_updated_parameters.address_table_aging_timer = PonPortTable[PonPortIdx].MACAgeingTime;
	
	pon_updated_parameters.cni_port_maximum_entries = PON_ADDRESS_TABLE_ENTRY_LIMITATION_0;
	/*pon_updated_parameters.max_rtt = PON_MAX_RTT_40KM;*/ 
		
	pon_updated_parameters.igmp_configuration.enable_igmp_snooping = DISABLE;
	
	/*pon_updated_parameters.hec_configuration.tx_hec_configuration = PON_VALUE_NOT_CHANGED;
	pon_updated_parameters.hec_configuration.rx_hec_configuration =PON_VALUE_NOT_CHANGED;*/


	return( PAS_update_olt_parameters_v4(PonPortIdx, &pon_updated_parameters ));
}

short int update_Pon_parameters_5201(short int PonPortIdx, short int  grant_filtering )
{
	PON_update_olt_parameters_t  pon_updated_parameters;
	short int ret;
		
	CHECK_PON_RANGE;

	PON_EMPTY_UPDATE_OLT_PARAMETERS_STRUCT(pon_updated_parameters);
#if 1
	VOS_MemCpy( &pon_updated_parameters, &PAS_updated_parameters_5201, sizeof(PON_update_olt_parameters_t ) );

	/* added by chenfj 2006/12/12
	     PON MAX RANGE */
	if( PonPortTable[PonPortIdx].range == PON_RANGE_CLOSE )
		pon_updated_parameters.max_rtt = V2R1_REGISTER_WINDOW_CLOSE;
	if( PonPortTable[PonPortIdx].range == PON_RANGE_20KM ) 
		pon_updated_parameters.max_rtt = PON_MAX_RTT_40KM;
	if( PonPortTable[PonPortIdx].range == PON_RANGE_40KM ) 
		pon_updated_parameters.max_rtt = PON_MAX_RTT_80KM;
	else if( PonPortTable[PonPortIdx].range == PON_RANGE_60KM ) 
		pon_updated_parameters.max_rtt = PON_MAX_RTT_120KM;
	
	if(grant_filtering == V2R1_ENABLE )
		pon_updated_parameters.grant_filtering = ENABLE;
	else if(grant_filtering == PON_VALUE_NOT_CHANGED)
		pon_updated_parameters.grant_filtering = PON_VALUE_NOT_CHANGED;
	else pon_updated_parameters.grant_filtering = DISABLE;	
#endif
#if 0
	pon_updated_parameters.hec_configuration.rx_hec_configuration = PAS_updated_parameters_5201.hec_configuration.rx_hec_configuration;
	pon_updated_parameters.hec_configuration.tx_hec_configuration = PAS_updated_parameters_5201.hec_configuration.tx_hec_configuration;

	pon_updated_parameters.grant_filtering = PAS_updated_parameters_5201.grant_filtering;
	pon_updated_parameters.support_passave_onus = PAS_updated_parameters_5201.support_passave_onus;

	ret =  PAS_test_update_olt_parameters(PonPortIdx, &pon_updated_parameters ) ;
	if( ret != PAS_EXIT_OK )
		{
		sys_console_printf(" pon updated parameter check Err %d \r\n", ret );
		return( RERROR );
		}
#endif
	ret = PAS_update_olt_parameters(PonPortIdx, &pon_updated_parameters );
	if( ret != PAS_EXIT_OK )
		{
		sys_console_printf(" pon updated parameter Set Err %d\r\n", ret );
		return( RERROR );
		}
	
	return( ROK );
}

#if defined(_EPON_10G_PMC_SUPPORT_)      
/*Begin: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
short int update_Pon_parameters_8411(short int PonPortIdx, short int  grant_filtering )
{
	GW10G_PON_update_olt_parameters_t  pon_updated_parameters;
	short int ret;
		
	CHECK_PON_RANGE;

#if 1
	VOS_MemCpy( &pon_updated_parameters, &PAS_updated_parameters_8411, sizeof(GW10G_PON_update_olt_parameters_t ) );

	/* added by chenfj 2006/12/12
	     PON MAX RANGE */
	if( PonPortTable[PonPortIdx].range == PON_RANGE_CLOSE )
		pon_updated_parameters.max_rtt = V2R1_REGISTER_WINDOW_CLOSE;
	if( PonPortTable[PonPortIdx].range == PON_RANGE_20KM ) 
		pon_updated_parameters.max_rtt = PON_MAX_RTT_40KM;
	if( PonPortTable[PonPortIdx].range == PON_RANGE_40KM ) 
		pon_updated_parameters.max_rtt = PON_MAX_RTT_80KM;
	else if( PonPortTable[PonPortIdx].range == PON_RANGE_60KM ) 
		pon_updated_parameters.max_rtt = PON_MAX_RTT_120KM;
	
	if(grant_filtering == V2R1_ENABLE )
		pon_updated_parameters.grant_filtering = ENABLE;
	else if(grant_filtering == PON_VALUE_NOT_CHANGED)
		pon_updated_parameters.grant_filtering = PON_VALUE_NOT_CHANGED;
	else pon_updated_parameters.grant_filtering = DISABLE;	
#endif
    ret = GW10G_PAS_update_olt_parameters(PonPortIdx, &pon_updated_parameters );
	if( ret != PAS_EXIT_OK )
	{
		sys_console_printf(" pon updated parameter Set Err %d\r\n", ret );
		return( RERROR );
	}
	
	return( ROK );
}
/*End: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
#endif

#endif

short int Get_Pon_parameters_5201(short int PonPortIdx )
{
	PON_olt_response_parameters_t  pon_updated_parameters;
	short int ret;
	unsigned char MAC_Addr[BYTES_IN_MAC_ADDRESS];
	
	CHECK_PON_RANGE
    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = 	OLT_GetOltParameters( PonPortIdx, &pon_updated_parameters );
	if( ret == PAS_EXIT_OK )
		{
		VOS_MemCpy( MAC_Addr, pon_updated_parameters.olt_mac_address, BYTES_IN_MAC_ADDRESS );
		sys_console_printf("%s/port Pon Parameters:\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
		sys_console_printf("downlink buffer size %dM \r\n", pon_updated_parameters.external_downlink_buffer_size);
		sys_console_printf("OLT MAC address:%02x-%02x-%02x-%02x-%02x-%02x\r\n",MAC_Addr[0],MAC_Addr[1],MAC_Addr[2],MAC_Addr[3],MAC_Addr[4],MAC_Addr[5]);
		sys_console_printf("Max RTT %d\r\n", pon_updated_parameters.update_olt_parameters.max_rtt );
		sys_console_printf("HEC config tx/rx %d/%d\r\n", pon_updated_parameters.update_olt_parameters.hec_configuration.tx_hec_configuration, pon_updated_parameters.update_olt_parameters.hec_configuration.rx_hec_configuration );
		sys_console_printf("grant filter %d, support passave onu %d\r\n", pon_updated_parameters.update_olt_parameters.grant_filtering, pon_updated_parameters.update_olt_parameters.support_passave_onus );
		return( ROK );
		}
	else{
		sys_console_printf("\r\nGet Pon Parameters Err %d \r\n", ret );
		return( RERROR );
		}

}

short int update_Pon_parameters_test(short int PonPortIdx  )
{
	PON_olt_update_parameters_t  pon_updated_parameters;
		
	CHECK_PON_RANGE
		
	PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT(pon_updated_parameters);
	/*VOS_MemCpy( &pon_updated_parameters, &PAS_updated_parameters, sizeof(PON_olt_update_parameters_t ) );*/
	/*pon_updated_parameters.max_rtt = PAS_updated_parameters.max_rtt ;*/

	/* added by chenfj 2006/12/12
	     PON MAX RANGE */
	if( PonPortTable[PonPortIdx].range == PON_RANGE_40KM ) 
		pon_updated_parameters.max_rtt = PON_MAX_RTT_80KM;
	else if( PonPortTable[PonPortIdx].range == PON_RANGE_60KM ) 
		pon_updated_parameters.max_rtt = PON_MAX_RTT_120KM;
	
	pon_updated_parameters.address_table_aging_timer = PAS_updated_parameters_5001.address_table_aging_timer;
	pon_updated_parameters.cni_port_maximum_entries = PAS_updated_parameters_5001.cni_port_maximum_entries ;
	
	pon_updated_parameters.arp_filtering_configuration.from_pon_to_firmware = PAS_updated_parameters_5001.arp_filtering_configuration.from_pon_to_firmware ;
	pon_updated_parameters.arp_filtering_configuration.from_cni_to_firmware = PAS_updated_parameters_5001.arp_filtering_configuration.from_cni_to_firmware ;
	pon_updated_parameters.arp_filtering_configuration.from_pon_to_cni = PAS_updated_parameters_5001.arp_filtering_configuration.from_pon_to_cni ;
	pon_updated_parameters.arp_filtering_configuration.from_cni_to_pon  = PAS_updated_parameters_5001.arp_filtering_configuration.from_cni_to_pon ;
	
	pon_updated_parameters.igmp_configuration.enable_igmp_snooping = PAS_updated_parameters_5001.igmp_configuration.enable_igmp_snooping ;
	pon_updated_parameters.igmp_configuration.igmp_timeout = PAS_updated_parameters_5001.igmp_configuration.igmp_timeout ;
	pon_updated_parameters.vlan_configuration.vlan_exchange_downlink_tag_prefix =PAS_updated_parameters_5001.vlan_configuration.vlan_exchange_downlink_tag_prefix ;
	pon_updated_parameters.vlan_configuration.nested_mode_vlan_type = PAS_updated_parameters_5001.vlan_configuration.nested_mode_vlan_type ;
	
	pon_updated_parameters.hec_configuration.tx_hec_configuration = PAS_updated_parameters_5001.hec_configuration.tx_hec_configuration ;
	pon_updated_parameters.hec_configuration.rx_hec_configuration = PAS_updated_parameters_5001.hec_configuration.rx_hec_configuration ;
	
	pon_updated_parameters.vlan_tag_filtering_config.untagged_frames_filtering = PAS_updated_parameters_5001.vlan_tag_filtering_config.untagged_frames_filtering ;
	pon_updated_parameters.vlan_tag_filtering_config.multiple_copy_broadcast_enable_mode= PAS_updated_parameters_5001.vlan_tag_filtering_config.multiple_copy_broadcast_enable_mode ;
	pon_updated_parameters.vlan_tag_filtering_config.filtering_unexpected_tagged_downstream_frames = PAS_updated_parameters_5001.vlan_tag_filtering_config.filtering_unexpected_tagged_downstream_frames;
	pon_updated_parameters.upstream_default_priority  = PAS_updated_parameters_5001.upstream_default_priority ;
	pon_updated_parameters.pon_tbc_polarity = PAS_updated_parameters_5001.pon_tbc_polarity ;

	
	return( PAS_update_olt_parameters_v4(PonPortIdx, &pon_updated_parameters ));
}


int SetCniTbc( short int PonPortIdx, unsigned long tbc )
{
	short int PonChipType;
	short int ret;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
		
	if(OLT_PONCHIP_ISPAS(PonChipType))
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			ret = PAS_set_olt_register( PonPortIdx, TBC_ADDR,  tbc );
			if( ret == PAS_EXIT_OK )
				return( ROK );
			else return( RERROR );
			}
		else
			{
			return( ROK);	
			}
		}
	else{

		}   
	return( RERROR );
}

unsigned int GetCniTbc( short int PonPortIdx )
{
	short int ret;
	unsigned long retVal = 0;
	short int PonChipType;
	
	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
		
	if(OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			ret = PAS_get_olt_register ( PonPortIdx, TBC_ADDR,  &retVal);
			if( ret == PAS_EXIT_OK )
				return( retVal );
			}
		else 
			{
			return(ROK);	
			}
		}
	else{ /* other pon chip handler */

		}
	return( RERROR );
}


int  SetPonPortTBC(  short int PonPortIdx, PON_polarity_t  tbc )
{
	short int PonChipType;
	
	PON_olt_update_parameters_t  PAS_updated_parameters1;
	
	CHECK_PON_RANGE

	if(PonPortIsWorking(PonPortIdx ) != TRUE ) return( RERROR );

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
		
	if(OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT(PAS_updated_parameters1);

			PAS_updated_parameters1.pon_tbc_polarity = tbc;
			PAS_update_olt_parameters_v4( PonPortIdx,  &PAS_updated_parameters1);
			/*update_Pon_parameters(PonPortIdx );*/
			return( ROK );
			}
		else 
			{
			return( ROK );
			}
		}

	else{
		}

	return( RERROR );
}

int SetPonGpioPin(short int PonPortIdx, PON_gpio_lines_t gpio , PON_gpio_line_io_t gpio_dir, short int value )
{
	PON_gpio_line_io_t direction;
	short int PonChipType;
	short int PonChipVer = RERROR;
	
	CHECK_PON_RANGE

	/*sys_console_printf(" \r\nthe size of the bool %d \r\n", sizeof( bool ) );*/

	if( PonPortIsWorking(PonPortIdx ) != TRUE ) return( RERROR );
	PonChipType = V2R1_GetPonchipType(PonPortIdx);
	
	if(( PonPortTable[PonPortIdx].PortWorkingStatus != PONPORT_DOWN) && (PonPortTable[PonPortIdx].PortWorkingStatus != PONPORT_UNKNOWN )&& (PonPortTable[PonPortIdx].PortWorkingStatus != PONPORT_INIT))
	{
    	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		#if 0
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{			
			PAS_gpio_access_extended_v4( PonPortIdx, gpio, gpio_dir, value, &direction, (bool *)&value );
								
			/*sys_console_printf(" GPIO line0 direction: %d   value %d \r\n", direction, value );*/
			}
		else
		#endif
			{
			/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
			OLT_GpioAccess( PonPortIdx, gpio, gpio_dir, value, &direction, (bool *)&value );
			}
		}
	else { /* other pon chip handler */

		}
	}
    
	return( ROK);
}

int V2r1PonWatchdogEnable(short int PonPortIdx, short int value)
{
#if 0
	short int PonChipType;
	
	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
		
	if(OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if(OLT_PONCHIP_ISPAS5001(PonChipType) ) 
			{
			SetPonGpioPin( PonPortIdx, PON_GPIO_LINE_2, PON_GPIO_LINE_OUTPUT, value );
			return( ROK );
			}
		else
			{
			return( ROK );
			}
		}


	else{ /* other pon chip handler */
		}
#else
    if ( 0 == OLT_WriteGpio(PonPortIdx, OLT_GPIO_PON_WATCHDOG_SWITCH, (bool)value) )
    {
		return( ROK );
    }
#endif

	return( RERROR );
}

int V2r1PonWatchdog( short int PonPortIdx, short int value )
{
#if 0
	short int PonChipType;
	
	CHECK_PON_RANGE;
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	
	if(OLT_PONCHIP_ISPAS(PonChipType) )
		{
		SetPonGpioPin(PonPortIdx, PON_GPIO_LINE_0, PON_GPIO_LINE_OUTPUT,value );
		return( ROK );
		}
	else{

		}
#else
    if ( 0 == OLT_WriteGpio(PonPortIdx, OLT_GPIO_PON_WATCHDOG, (bool)value) )
    {
		return( ROK );
    }
#endif

	return( RERROR );
}

int  SetPonRunningFlag( short int PonPortIdx)
{
#if 0
	PON_gpio_line_io_t direction;
	bool value;
	short int PonChipType;
#endif
    
	CHECK_PON_RANGE

#if 1
    if ( OLT_CALL_ISOK(OLTAdv_SetPonLED(PonPortIdx, OLT_LED_RUN, PON_LED_ON)) )
    {
        return ROK;
    }
#else
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        || ( PonChipType == PONCHIP_PAS8411 )/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        ) 
		{
		/*PonChipVer = PonChipType;*/
		PonChipType = PONCHIP_PAS;
		}
	
	if( PonChipType == PONCHIP_PAS )
		{
		if(PAS_gpio_access ( PonPortIdx, PON_GPIO_LINE_1, PON_GPIO_LINE_OUTPUT, PON_LED_ON, 
 								&direction, &value ) == PAS_EXIT_OK )
		/* modified for PAS-SOFT V5.1
 		PAS_gpio_access_extended ( PonPortIdx, PON_GPIO_LINE_1, PON_GPIO_LINE_OUTPUT, PON_LED_ON, 
 								&direction, &value );
 								*/
		/*sys_console_printf(" GPIO line1 direction: %d   value %d \r\n", direction, value );*/
		return( ROK );
		}
	else {
		}
#endif

	return( RERROR );
}

int  SetPonNotRunningFlag( short int PonPortIdx)
{
#if 0
	PON_gpio_line_io_t direction;
	bool value;
	short int PonChipType;
#endif
	
	CHECK_PON_RANGE

#if 1
    if ( OLT_CALL_ISOK(OLTAdv_SetPonLED(PonPortIdx, OLT_LED_RUN, PON_LED_OFF)) )
    {
        return ROK;
    }
#else
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        ) 
		{
		/*PonChipVer = PonChipType;*/
		PonChipType = PONCHIP_PAS;
		}
	if(PonChipType == PONCHIP_PAS )
		{
		PAS_gpio_access( PonPortIdx, PON_GPIO_LINE_1, PON_GPIO_LINE_OUTPUT, PON_LED_OFF, 
 								&direction, &value );
		/* modified for PAS-SOFT V5.1
 		PAS_gpio_access_extended ( PonPortIdx, PON_GPIO_LINE_1, PON_GPIO_LINE_OUTPUT, PON_LED_OFF, 
 								&direction, &value );
 								*/
		/*sys_console_printf(" GPIO line1 direction: %d   value %d \r\n", direction, value );*/
		return( ROK );
		}
	else{

		}
#endif

	return( RERROR );
}


int  SetPonLoosingFlag( short int PonPortIdx)
{
    if ( OLT_CALL_ISOK(OLTAdv_SetPonLED(PonPortIdx, OLT_LED_LOSS, PON_LED_OFF)) )
    {
        return ROK;
    }

	return( RERROR );
}

int  SetPonNotLoosingFlag( short int PonPortIdx)
{
    if ( OLT_CALL_ISOK(OLTAdv_SetPonLED(PonPortIdx, OLT_LED_LOSS, PON_LED_ON)) )
    {
        return ROK;
    }

	return( RERROR );
}


/*****************************************************
 *
 *    Function:  set_image_src(PON_binary_source_t  src)
 *
 *    Param:   PON_binary_source_t  src -- the type of the pon chip firmware image file 
 *                 
 *    Desc:    set the type of the pon chip firmware image file
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
void set_image_src( short int PonChipType,  PON_binary_source_t  src, PON_binary_t *pInit, unsigned char version[100] )
{
	int ret;
	int  length, location1;
	char *location;

	if( pInit == NULL && version == NULL ) return;

    if ( pInit )
    {
    	pInit->size = 0;
    	pInit->location  = 0 ;
		
		pInit->type=PON_OLT_BINARY_FIRMWARE;
		pInit->source= src;
    }

    location  = NULL;	
    location1 = 0;
    length    = 0;
	if( src == PON_BINARY_SOURCE_MEMORY )
	{	
	    if ( pInit )
        {
    		ret = GetPonChipFWImageInfo( PonChipType, &location1, &length, version );
        }
        else
        {
    		ret = GetPonChipFWImageInfo( PonChipType, NULL, NULL, version );
        }

		if( ret == ROK )
		{
			/*sys_console_printf("firmware image location 0x%x, length 0x%x\r\n", location1, length);*/
			location=(char *)location1;
		}
		else
        {
			sys_console_printf(" Get Pon firmware file err\r\n");
		}
		/*
		location = (char *) Pon_Firmware_image[PonChipType].location;
		pInit->location=(void *)location;
		pInit->size = Pon_Firmware_image[PonChipType].size;
		*/
	}
	else if(src==PON_BINARY_SOURCE_FILE)
	{			
		location="/tgtsvr/pas_firmware.bin";
		length=0;
	}

	else if(src==PON_BINARY_SOURCE_FLASH)
	{
		location=NULL;
		length=0;
	}

    if ( pInit )
    {
		pInit->location=(void*)location;
		pInit->size=length;
    }

    return;
}

/*****************************************************
 *
 *    Function:  pon_add_olt( unsigned short int PonPortIdx )
 *
 *    Param:   unsigned short int PonPortIdx -- the specific pon port  
 *                 
 *    Desc:    add an olt to PAS-SOFT
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int Add_PonPort( short int PonPortIdx )
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_ADD_PONPORT, 0, 0};

	aulMsg[3] = PonPortIdx;

	
	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS send message err( Add_PonPort())\r\n"  );*/
		return( RERROR );
	}

	return( ROK );
}

int Del_PonPort( short int PonPortIdx )
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_DEL_PONPORT, 0, 0};

	aulMsg[3] = PonPortIdx;

	
	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS send message err( Del_PonPort())\r\n"  );*/
		return( RERROR );
		}

	return( ROK );
}

int reset_6900ponPort(int PonPortIdx)
{
    unsigned char value;

    if ( (0 <= PonPortIdx) && (PonPortIdx < 4) )
    {
        value = 0xFF - (16 << PonPortIdx);
    }
    else
    {
        value = 7;
    }
    
    /*WriteCPLDReg( (UCHAR*)9, 0x55 );*/
    CPLD_WRITE_LOCK( 0x9 );
    WriteCPLDReg( (UCHAR*)4, value );
    /*WriteCPLDReg( (UCHAR*)9, 0x0 );*/
    CPLD_WRITE_UNLOCK( 0x9 );
    VOS_TaskDelay(VOS_TICK_SECOND/2);
    
    /*WriteCPLDReg( (UCHAR*)9, 0x55 );*/
    CPLD_WRITE_LOCK( 0x9 );
    WriteCPLDReg( (UCHAR*)4, 0xFF );
    /*WriteCPLDReg( (UCHAR*)9, 0x0 );*/
    CPLD_WRITE_UNLOCK( 0x9 );

    return 0;
}


/* B--added by liwei056@2009-12-08 for New-SwitchOver Project */
/* pon_test_olt()是一个调试函数，提供检测OLT存活状态的调试手段  */
int pon_test_olt(short int PonPortIdx, short int testFlag)
{
    short int send_result;
    PAS_device_versions_t olt_ver;
    PON_remote_mgnt_version_t  remote_mgnt_version;
    unsigned long int          critical_events_counter;
	bool oltExist = FALSE;

    if ( testFlag > 0 )
    {
        /* 初始化PON芯片的PAS-SOFT主机MAC地址 */
        SetPonchipMgmtPathMac(PonPortIdx);
        if ( testFlag > 1 )
        {
            /* 打开本板与PON芯片的PAS-SOFT通讯信道 */
            enablePonPort();
        }
    }

    #if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	OLT_IsExist(PonPortIdx, &oltExist);
	if(!oltExist)
	#else
	if(!Olt_exists(PonPortIdx))
	#endif
	{
    	/* Enable communication - init OLT if needed */
    	/* Init communication layer with this OLT    */
    	if ( Pon_COMMInitOlt(PonPortIdx) != PASCOMM_EXIT_OK) 
    		return (PAS_OLT_NOT_EXIST);
    }
    /*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    send_result = Pon_GetDeviceVersions ( PonPortIdx, 
								   PON_OLT_ID, 
								   &olt_ver, 
                                   &remote_mgnt_version,
                                   &critical_events_counter );
    if (PAS_EXIT_OK == send_result)
    {
        sys_console_printf("\r\n---Olt[%d] is actived by key-event(%lu)!---", PonPortIdx, critical_events_counter);
    }
    else
    {
        sys_console_printf("\r\n-X-Olt[%d] is not actived for error(%d)!-X-", PonPortIdx, send_result);
    }
    
    return send_result;
}

int pon_test_add_olt(short int PonPortIdx)
{
	short int result = PAS_EXIT_ERROR;
	/*short int  PonChipIdx;*/
	PON_add_olt_result_t add_olt_result= {{0x00, 0x0c, 0xd5, 0x00, 0x01, 0x01}};

    add_olt_result.olt_mac_address[5] = PonPortIdx;
    pon_initialization_parameters_init();
	set_image_src( PONCHIP_PAS5201, PON_BINARY_SOURCE_MEMORY, &(PAS_initialization_parameters_5201.firmware_image), NULL );
	VOS_MemCpy( PAS_initialization_parameters_5201.olt_mac_address, add_olt_result.olt_mac_address, BYTES_IN_MAC_ADDRESS );
	result = PAS_add_olt(PonPortIdx,  &PAS_initialization_parameters_5201, &add_olt_result);				
    pon_load_release(PONCHIP_PAS5201, 1, 0);
	sys_console_printf(" \r\n  pon%d load result(%d).\r\n", PonPortIdx, result);

    return result;
}

int pon_resume_olt(short int PonPortIdx)
{
	PON_olt_mode_t			            olt_mode = 0;	
	unsigned short int			        dba_mode;
	PON_llid_t				            link_test;  
	PON_active_llids_t		            active_llids;
	short int				            send_result;
    bool oltExist = FALSE;
	/*modified by liyang @2015-05-18 for warm restart to be compatible with Tk*/
#ifndef BCM_WARM_BOOT_SUPPORT 	
    if ( SYS_LOCAL_MODULE_TYPE_IS_PAS_PONCARD_MANAGER)
#else
    if ( SYS_LOCAL_MODULE_TYPE_IS_PAS_PONCARD_MANAGER || SYS_LOCAL_MODULE_TYPE_IS_TK_PONCARD_MANAGER )
#endif
    {
		short int PonChipType = 0;
		PonChipType = V2R1_GetPonchipType( PonPortIdx );
#if 1
		OLT_IsExist(PonPortIdx, &oltExist);
	   if(!oltExist)
#else
	if(!Olt_exists(PonPortIdx))
#endif
 	   {
    	    PON_olt_initialization_parameters_t initialization_configuration;
			
            VOS_ASSERT(0 != *GetPonChipMgmtPathMac(PonPortIdx));

			
			if(OLT_PONCHIP_ISPAS(PonChipType))
			{
			
				
	    		send_result   = Pon_HostRecovery( PonPortIdx,
	        										   &olt_mode,
	          									       &initialization_configuration,
	        										   &dba_mode,
	        										   &link_test,
	        										   &active_llids );
	            if(send_result != PAS_EXIT_OK)
	            {
	        		/* sys_console_printf(" Board resume pon[%d]'s host connection failed at (PAS_olt_host_recovery).\r\n", PonPortIdx); */
	                return VOS_ERROR;
	            }

			} 
			else if(OLT_PONCHIP_ISTK(PonChipType))
			{
						
#ifdef BCM_WARM_BOOT_SUPPORT
				send_result = TK_olt_host_recovery(PonPortIdx, &olt_mode);
				if(send_result != TK_EXIT_OK)
	            {
	                return VOS_ERROR;
	            }
#endif
			}
			
        }
        else
        {
            mac_address_t olt_mac;
            PON_olt_init_parameters_t  initialization_configuration;	/* modified by xieshl 20100420, 参数类型和函数原型不一致 */
        	/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
			send_result   = Pon_GetOltModeQuery(PonPortIdx,
        										   &olt_mode,
        										   olt_mac,
          									       &initialization_configuration,
        										   &dba_mode,
        										   &link_test,
        										   &active_llids );
            if(send_result != PAS_EXIT_OK)
            {
        		/* sys_console_printf(" Board board resume pon[%d]'s host connection failed at (PAS_get_olt_mode_query_v4).\r\n", PonPortIdx); */
                return VOS_ERROR;
            }
        }
    }
    else
    {
        VOS_ASSERT(0);
    }

    return olt_mode;
}
/* E--added by liwei056@2009-12-08 for New-SwitchOver Project */


/* B--added by liwei056@2010-5-14 for 6900 */
int pon_lose_onu(short int olt_id, short int onu_id)
{
    return ONU_SetupIFs(olt_id, onu_id, ONU_ADAPTER_NULL);
}

int pon_lose_olt(short int PonPortIdx)
{
    short int i;

    /* 设置OLT空管理接口 */
    OLT_SetupIFs(PonPortIdx, OLT_ADAPTER_NULL);

    /* 设置ONU空管理接口 */
    for(i=0; i<MAXONUPERPON; i++)
    {	
        pon_lose_onu(PonPortIdx, i);
	}		

    return 0;    
}

int pon_remote_onu(short int olt_id, short int onu_id)
{
    return ONU_SetupIFs(olt_id, onu_id, ONU_ADAPTER_RPC);
}

int pon_remote_olt(short int PonPortIdx)
{
    int iRlt;

    if ( OLT_ADAPTER_RPC == OLT_GetIFType(PonPortIdx) )
    {
        iRlt = 1;
    }
    else
    {
        /* 设置OLT远程管理接口 */
        if ( OLT_ERR_OK == (iRlt = OLT_SetupIFs(PonPortIdx, OLT_ADAPTER_RPC)) )
        {
            short int i;

            /* 设置ONU远程管理接口 */
        	for(i=0; i<MAXONUPERPON; i++)
        	{	
        		pon_remote_onu(PonPortIdx, i);
        	}		
        }
    }

    return iRlt;    
}

int pon_local_onu(short int olt_id, short int onu_id, int chip_type, int remote_type)
{
    return ONU_SetupIFsByType(olt_id, onu_id, chip_type, remote_type);
}

int pon_local_olt(short int PonPortIdx, int chip_type)
{
    int iRlt;

    if ( chip_type < 0 )
    {
		if ( 0 >= (chip_type = OLTAdv_GetChipTypeID(PonPortIdx)) )
        {
            chip_type = getPonChipType(GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
        }
    }

    if ( chip_type == OLT_GetIFChipType(PonPortIdx) )
    {
        iRlt = 1;
    }
    else
    {
        /* 设置OLT本地管理接口 */
        if ( OLT_ERR_OK == (iRlt = OLT_SetupIFsByChipType(PonPortIdx, chip_type)) )
        {
            short int i;

            /* 设置ONU本地管理接口 */
        	for(i=0; i<MAXONUPERPON; i++)
        	{	
        		pon_local_onu(PonPortIdx, i, chip_type, ONU_MANAGE_UNKNOWN);			
        	}		
        }
    }

    return iRlt;
}
/* E--added by liwei056@2010-5-14 for 6900 */

int hardware_reset_olt(short int PonPortIdx)
{
	int  Cardslot, CardPort;
    
	Cardslot = GetCardIdxByPonChip(PonPortIdx);
	CardPort = GetPonPortByPonChip(PonPortIdx);

	Hardware_Reset_olt1(Cardslot, CardPort, 1, 1);
	Hardware_Reset_olt2(Cardslot, CardPort, 0, 1);

    return 0;
}

int PON_ADD_TEST = V2R1_DISABLE;
int pon_add_olt(const short int PonPortIdx)
{
	short int  PonChipIdx;
	short int  PonChipType;
	short int  PonChipVer = RERROR;
	int  Cardslot, CardPort;
	int ret = -1;
		
	CHECK_PON_RANGE;

	PonChipIdx = GetPonChipIdx( PonPortIdx );
	
	Cardslot = GetCardIdxByPonChip(PonChipIdx);
	CardPort = GetPonPortByPonChip(PonChipIdx);

    switch ( PonChipMgmtTable[PonPortIdx].operStatus )
    {
        case PONCHIP_UP:
        case PONCHIP_TESTING:
        case PONCHIP_DORMANT:
    		VOS_SysLog(LOG_TYPE_OLT, LOG_WARNING, " \r\n  %s/port%d is being Up, or UP already \r\n", CardSlot_s[Cardslot], CardPort);
    		return( ROK );
        case PONCHIP_INIT:
    		VOS_SysLog(LOG_TYPE_OLT, LOG_WARNING, " \r\n  %s/port%d is being Initializing\r\n", CardSlot_s[Cardslot], CardPort);
    		return( ROK );
    }

	/*PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_INIT;*/
	PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_INIT;
	
	PonChipType = V2R1_GetPonchipType( PonChipIdx );
	if( OLT_PONCHIP_ISPAS(PonChipType) ) 
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
	}
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_TK;
    }
	#if defined(_GPON_BCM_SUPPORT_)
	else if(OLT_PONCHIP_ISGPON(PonChipType))
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_GPON;
	}
	#endif

	if( PonChipType == PONCHIP_PAS )
	{	
		if(OLT_PONCHIP_ISPAS1G(PonChipVer)) 
		{
		    ret = GW5201_pon_add_olt( PonPortIdx, PonChipVer);
		}
#if defined(_EPON_10G_PMC_SUPPORT_)            
		else if(OLT_PONCHIP_ISPAS10G(PonChipVer))
		{
		    ret = GW8411_pon_add_olt( PonPortIdx, PonChipVer);
		}
#endif

        if ( 0 == ret )
        {
#if 1		
    		VOS_TaskDelay( VOS_TICK_SECOND/4 ); 
    		if( PonPortTable[PonPortIdx].external_DBA_used == TRUE ) 
    		{
    			ret = pon_load_external_dba_file(PonPortIdx );
    			if( ret != ROK )
    			{
        			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  Download DBA image to pon%d/%d ... Err(=%d)\r\n", Cardslot, CardPort, ret);

                    if ( V2R1_FILE_NOT_FOUND == ret )
                    {
                        if(EVENT_PONADD == V2R1_ENABLE)
                        {
               				sys_console_printf("error: pon%d/%d DAB Image not found \r\n",Cardslot, CardPort);
                        }      
                        
                        ret = DBA_IS_ERROR;
                    }
                    else
                    {
                        ret = DBA_LOAD_ERROR;
                    }
                    
                    return ret;
    			}
    		}
    		else
            { 
                /* B--added by liwei056@2011-1-11 for SyncMasterStatus */
                if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
                {
                    OLT_SYNC_Data(PonPortIdx, FC_ADD_PONPORT, NULL, 0);
                }
                /* E--added by liwei056@2011-1-11 for SyncMasterStatus */
                
                /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
    			Pon_StartDbaAlgorithm( PonPortIdx, PonChipVer, TRUE, 0, NULL);
    			PonPortTable[PonPortIdx].external_DBA_used = FALSE;
    			PonPortTable[PonPortIdx].DBA_mode = OLT_INTERNAL_DBA; 
    		}
#endif

    		VOS_TaskDelay(VOS_TICK_SECOND/4); 
            return (ROK);
        }
	}
	else if( PonChipType == PONCHIP_TK )
    {
#if defined(_EPON_10G_BCM_SUPPORT_)            
        if ( OLT_PONCHIP_ISBCM(PonChipVer) )
        {
            ret = BCM55538_pon_add_olt(PonPortIdx, PonChipVer);
        }
        else
#endif
        {
            ret = TK3723_pon_add_olt(PonPortIdx, PonChipVer);
        }
	}	
	/* other type pon chip should be also handled */ 
	#if defined(_GPON_BCM_SUPPORT_)
	else if(PONCHIP_GPON == PonChipType)
	{
		ret = GPON_pon_add_olt(PonPortIdx, PonChipVer);
	}
	#endif
	else
    {
		sys_console_printf("Get pon%d/%d chip type(%d) unknowned. \r\n",Cardslot, CardPort, PonChipType);
	}
	
	return ret;
} 


int pon_load_flags;
static void pon_add_oltEx( unsigned long arg0, unsigned long arg1,unsigned long arg2, unsigned long arg3,
		unsigned long arg4, unsigned long arg5,unsigned long arg6, unsigned long arg7 , unsigned long arg8, unsigned long arg9 )
{
    short int PonPortIdx;
    short int ReLoadFlag;
    unsigned long ulSlot, ulPort;
    int PonChipType;
    int iPonReloadTimes;
    int iIsNeedReset;
    int iIsNewThread;
    int iRlt;

    ulSlot      = arg0;
    ulPort      = arg1;
    PonPortIdx  = (short int)arg2;
    PonChipType = (int)arg3;
    iIsNeedReset = (int)arg5;
    iIsNewThread = (int)arg6;

    VOS_ASSERT(ulSlot == GetCardIdxByPonChip(PonPortIdx));
    VOS_ASSERT(ulPort == GetPonPortByPonChip(PonPortIdx));
    VOS_ASSERT(PonPortIdx == GetPonPortIdxBySlot(ulSlot, ulPort));

    if( getPonChipInserted( ulSlot, ulPort ) != PONCHIP_EXIST )
    {
    	sys_console_printf("\r\n pon_add_oltEx PON %d/%d not exist\r\n", ulSlot, ulPort );
    	return /*RERROR*/;
    }

    if ( 0 <= (iPonReloadTimes = (int)arg4) )
    {
        iPonReloadTimes = PON_DOWNLOAD_MAX_COUNTER - iPonReloadTimes;
    }
    else
    {
        iPonReloadTimes = -iPonReloadTimes;
    }

    if ( 0 < iIsNeedReset )
    {
        /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		if(OLTAdv_IsExist(PonPortIdx) == TRUE )
			Pon_RemoveOlt( PonPortIdx, FALSE, FALSE);	

		ClearPonRunningData( PonPortIdx );
        
		Hardware_Reset_olt1(ulSlot, ulPort, 1, 0);
		Hardware_Reset_olt2(ulSlot, ulPort, 1, 0);
    }
    
    OLT_ADD_DEBUG(OLT_ADD_TITLE"Pon%d start loading...\r\n", PonPortIdx);
    do{
        ReLoadFlag = 0;
        iRlt = pon_add_olt(PonPortIdx);
        OLT_ADD_DEBUG(" pon_add_olt PON %d/%d(ponStatus=%d,ret=%d)\r\n", ulSlot, ulPort, PonChipMgmtTable[PonPortIdx].operStatus, iRlt );
    	 /*OLT_ADD_DEBUG(OLT_ADD_TITLE"Pon%d finish load error(%d).\r\n", PonPortIdx, iRlt);*/
        if ( (0 != iRlt) && !FILE_IS_BAD(iRlt) )
        {
            /* 加载失败，重新加载 */
    		if( (PonChipType != PONCHIP_PAS5001) && (PONCHIP_INIT == PonChipMgmtTable[PonPortIdx].operStatus) )
    		{
    		    /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
				if(OLTAdv_IsExist(PonPortIdx) == TRUE )
					Pon_RemoveOlt( PonPortIdx, FALSE, FALSE);	

				ClearPonRunningData( PonPortIdx );

				if( getPonChipInserted( ulSlot, ulPort ) != PONCHIP_EXIST )
				{
				    break;
				}

                if ( OLT_IS_NOTREADY(iRlt) )
                {
                    /* OLT未就绪，延时等待，重新加载 */
    				VOS_TaskDelay(VOS_TICK_SECOND * 3);
                }
                else
                {
                    /* 重新加载，只为获取文件的话，无需复位芯片 */
                    if ( !FILE_IS_NEED(iRlt)  )
                    {
        				Hardware_Reset_olt1(ulSlot, ulPort, 0, 0);
        				VOS_TaskDelay(VOS_TICK_SECOND);
        				Hardware_Reset_olt2(ulSlot, ulPort, 1, 0);
                    }
                }

    			if ( ++PonChipDownloadCounter[PonPortIdx] < iPonReloadTimes )
                {
                	OLT_ADD_DEBUG(OLT_ADD_TITLE"Pon%d start reloading at %dst time...\r\n", PonPortIdx, PonChipDownloadCounter[PonPortIdx]);
                    ReLoadFlag = 1;
                }
                else
                {
                    PonChipDownloadCounter[PonPortIdx] = 0;
                }
    		}
        }    
    }    while( 0 != ReLoadFlag );

	
    if ( 0 != iRlt )
    {
        if ( FIRMWARE_LOAD_ISFAILED(iRlt) )
        {
    		Trap_FirmwareLoad( PonPortIdx, V2R1_LOAD_FILE_FAILURE );            
        }
        else if ( DBA_LOAD_ISFAILED(iRlt) )
        {
            Trap_DBALoad( PonPortIdx, V2R1_LOAD_FILE_FAILURE );
        }
        else
        {
    		Trap_FirmwareLoad( PonPortIdx, V2R1_LOAD_FILE_FAILURE );            
        }

        /* B--added by liwei056@2011-1-11 for SyncMasterStatus */
    	PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_DOWN;
    	PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ERR;
        
        if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
        {
            OLT_SYNC_Data(PonPortIdx, FC_DEL_PONPORT, NULL, 0);
        }
        /* E--added by liwei056@2011-1-11 for SyncMasterStatus */
    }
    /*else
          PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_LOADCOMP;*/	/* modified by xieshl 20120426, 加载完成即改状态，不能等到PON板都加载完时改状态 ，防止设备板卡状态误判 */

    if ( iIsNewThread > 0 )
    {
        if ( V2R1_ENABLE == GetPonChipResetFlag(ulSlot, ulPort) )
        {
            ClearPonChipResetFlag(ulSlot, ulPort);
        }

        pon_load_flags |= (1 << PonPortIdx);
        VOS_TaskExit(0);
    }

    return /*iRlt*/;
}

static int pon_add_olt_startup(int SlotId, int PortId, int PonPortIdx, int PonChipType, int ReloadBase, int IsNeedReset)
{
    int add_task_id; 
    unsigned long args[VOS_MAX_TASK_ARGS]={0,0,0,0,0,0,0,0,0,0};
    CHAR ucTaskName[20];


    args[0] = SlotId;
    args[1] = PortId;
    args[2] = PonPortIdx;
    args[3] = PonChipType;
    args[4] = ReloadBase;  /* 失败重新加载次数的基数 */
    args[5] = IsNeedReset; /* 加载前是否需要复位PON */
    args[6] = 1;           /* 是否在新任务里加载 */

/*	if(tloadpon_buff==NULL)
	{
	    tloadpon_buff = g_malloc(1024*64); /* 申请64k 但是不进行释放   add by mengxsh 20141107 
	    if(tloadpon_buff==NULL)
	        {
	            sys_console_printf("alloc memory error\r\n");
	        }
	    sys_console_printf("g_malloc memory for 12pon_ex 0x%08x\r\n",tloadpon_buff);
	}
*/ 
    VOS_Snprintf(ucTaskName, 18, "tLoadPon%d", PonPortIdx);
    add_task_id = (int)VOS_TaskCreate(ucTaskName, TASK_PRIORITY_BELOW_NORMAL, pon_add_oltEx, args);

    return ( 0 != add_task_id ) ? ROK : RERROR;
}

int pon_add_oltEx2(int SlotId, int PortId, int PonPortIdx, int PonChipType, int ReloadBase, int IsNeedReset, int IsNewThread)
{
    int ret = ROK;
    if ( IsNewThread > 0 )
    {
        ret = pon_add_olt_startup(SlotId, PortId, PonPortIdx, PonChipType, ReloadBase, IsNeedReset);
    }
    else
    {
        /*ret = */pon_add_oltEx(SlotId, PortId, PonPortIdx, PonChipType, ReloadBase, IsNeedReset, 0, 0, 0, 0);
    }
    return ret;
}


#if 1
static int pon_test_oltEx( unsigned long arg0, unsigned long arg1,unsigned long arg2, unsigned long arg3,
		unsigned long arg4, unsigned long arg5,unsigned long arg6, unsigned long arg7 , unsigned long arg8, unsigned long arg9 )
{
    static const char *host_ifnames[] = {"bus", "eth", "com"}; 
    int iRlt, iResult;
    int isNewThread, iTimesLimit, iTimeDelay;
    int SlotId, PortId;
    ULONG TestTimes, FailedTimes, OltExistFailed;
    ULONG ulTicks;
    short int PonPortIdx;
    short int PonChipType;
    short int PonChipID;
    short int statistics_parameter;
    struct vty *vty;
    unsigned long hostFrameRecvOK, hostFrameSendOK, hostFrameRecvErr;
    PON_timestamp_t timestamp;
    PON_device_versions_t ponChipVer;
    PON_host_messages_raw_data_t hostMsgRawData;
    OLT_raw_stat_item_t    stat_item;
	bool oltExist = FALSE;
	
    vty = (struct vty *)arg0;
    SlotId = (int)arg1;
    PortId = (int)arg2;
    PonPortIdx  = (short int)arg3;
    iTimeDelay  = (int)arg4;
    iTimesLimit = (int)arg5;
    isNewThread = (int)arg6;

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
    switch (PonChipType)
    {
        case PONCHIP_PAS5001:
            PonChipID = 0x5001;
            break;
        case PONCHIP_PAS5201:
        case PONCHIP_PAS5204:
            PonChipID = 0x5201;
            break;
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		case PONCHIP_PAS8411:
            PonChipID = 0x8101;
            break;
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        case PONCHIP_TK3723:
            PonChipID = 0x3723;
            break;
        case PONCHIP_BCM55524:
            PonChipID = 0xa524;
            break;
        default:
            PonChipID = 0;
    }

    pon_get_init_parameters(&statistics_parameter);    
    switch (statistics_parameter)
    {
        case V2R1_PON_HOST_MANAGE_BY_ETH:
            statistics_parameter = PON_COMM_TYPE_ETHERNET;
            break;
        case V2R1_PON_HOST_MANAGE_BY_BUS:
            statistics_parameter = PON_COMM_TYPE_PARALLEL;
            break;
        case V2R1_PON_HOST_MANAGE_BY_URT:
            statistics_parameter = PON_COMM_TYPE_UART;
            break;
        default:
            VOS_ASSERT(0);
            statistics_parameter = PON_COMM_TYPE_PARALLEL;
    }

    ulTicks = (VOS_TICK_SECOND * iTimeDelay) / 1000;
    TestTimes   = 0;    
    FailedTimes = 0;
    OltExistFailed = 0;
    hostFrameRecvOK  = 0;
    hostFrameSendOK  = 0;
    hostFrameRecvErr = 0;

    vty_direct_out(vty, "\r\n-----pon%d/%d test(0/%d) HOST-IF(%s) start-----", SlotId, PortId, iTimesLimit, host_ifnames[statistics_parameter]);
    
    do
    {
        ++TestTimes;

        if ( 0 < ulTicks )
        {
            VOS_TaskDelay(ulTicks);
        }
        #if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		OLT_IsExist(PonPortIdx, &oltExist);
		if(oltExist)
		#else
        if ( Olt_exists(PonPortIdx) )
		#endif
        {
            if ( 0 != OltExistFailed )
            {
                OltExistFailed = 0;
                vty_direct_out(vty, "\r\n--O--port%d/%d is now exist test(%d/%d) start--O--", SlotId, PortId, FailedTimes, TestTimes);
            }
			#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
			stat_item.collector_id = PON_OLT_ID;
		    stat_item.raw_statistics_type = PON_RAW_STAT_HOST_MESSAGES;
		    stat_item.statistics_parameter = statistics_parameter;
			
		    stat_item.statistics_data = &hostMsgRawData;
		    stat_item.statistics_data_size = sizeof(PON_host_messages_raw_data_t);
		    iRlt = OLT_GetRawStatistics(PonPortIdx, &stat_item);
			timestamp = stat_item.timestam;
            #else
            iRlt = PAS_get_raw_statistics(PonPortIdx, PON_OLT_ID, PON_RAW_STAT_HOST_MESSAGES, statistics_parameter, &hostMsgRawData, &timestamp);  
			#endif
            if ( 0 == iRlt )
            {
                if ( 0 != hostFrameRecvOK )
                {
                    if ( hostFrameRecvOK >= hostMsgRawData.received_from_firmware_ok )
                    {
                        iRlt = OLT_ERR_DATA;
                    }
                }
                hostFrameRecvOK = hostMsgRawData.received_from_firmware_ok;
                
                if ( 0 != hostFrameSendOK )
                {
                    if ( hostFrameSendOK >= hostMsgRawData.sent_to_firmware_ok)
                    {
                        iRlt = OLT_ERR_DATA;
                    }
                }
                hostFrameSendOK = hostMsgRawData.sent_to_firmware_ok;
                
                if ( 0 != hostFrameRecvErr )
                {
                    if ( hostFrameRecvErr != hostMsgRawData.received_with_error_from_host)
                    {
                        iRlt = OLT_ERR_DATA;
                    }
                }
                hostFrameRecvErr = hostMsgRawData.received_with_error_from_host;
                #if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
				iResult = OLT_GetVersion(PonPortIdx, &ponChipVer);
				#else
            	iResult = PAS_get_olt_versions(PonPortIdx, &ponChipVer);
				#endif
                if ( 0 == iRlt )
                {
                    if ( 0 == (iRlt = iResult) )
                    {
                        if ( 0 != PonChipID )
                        {
                            if ( PonChipID != ponChipVer.hardware_major )
                            {
                                if ( 0 != ponChipVer.hardware_major )
                                {
                                    vty_direct_out(vty, "\r\n--X--port%d/%d's test(%d/%d) failed(chip:0x%04X!=0x%04X)--X--", SlotId, PortId, FailedTimes, TestTimes, PonChipID, ponChipVer.hardware_major);
                                    PonChipID = ponChipVer.hardware_major;
                                }
                                else
                                {
                                    vty_direct_out(vty, "\r\n--X--port%d/%d's test(%d/%d) failed(%d)--X--", SlotId, PortId, FailedTimes, TestTimes, OLT_ERR_PARTOK);
                                }
                                
                                iRlt = OLT_ERR_PARTOK;
                            }
                        }
                        else
                        {
                            PonChipID = ponChipVer.hardware_major;
                            vty_direct_out(vty, "\r\n--X--port%d/%d's test(%d/%d) failed(unknown chip:%d)--X--", SlotId, PortId, FailedTimes, TestTimes, PonChipType);

                            iRlt = OLT_ERR_PARTOK;
                        }
                    }
                }
            }

            if ( iRlt < 0 )
            {
                ++FailedTimes;
                
                switch (iRlt)
                {
                    case OLT_ERR_PARTOK:
                        /* 错误已经打印 */
                        break;
                    case OLT_ERR_DATA:
                        /* 轻微错误 */
                        vty_direct_out(vty, "\r\n--X--port%d/%d's test(%d/%d) failed(err:%d)--X--", SlotId, PortId, FailedTimes, TestTimes, iRlt);
                        break;
                    default:    
                        /* 严重错误 */
                        vty_direct_out(vty, "\r\n-XXX-port%d/%d's test(%d/%d) failed(err:%d)-XXX-", SlotId, PortId, FailedTimes, TestTimes, iRlt);
                }
            }
        }
        else
        {
            if ( 0 == OltExistFailed )
            {
                OltExistFailed = 1;
                iRlt = OLT_ERR_NOTEXIST;
                ++FailedTimes;
                vty_direct_out(vty, "\r\n--X--port%d/%d's test(%d/%d) failed(not exist in pas)--X--", SlotId, PortId, FailedTimes, TestTimes);

                if ( PON_COMM_TYPE_PARALLEL != statistics_parameter )
                {
                    if ( 1 == TestTimes )
                    {
                        /* 第一次测试，就不存在，无需再测 */
                        iTimesLimit = 1;
                    }
                }
            }
            else
            {
                if ( PON_COMM_TYPE_PARALLEL == statistics_parameter )
                {
                    if ( 0 != check_parallel_magic_number(PonPortIdx) )
                    {
                        ++FailedTimes;
                        vty_direct_out(vty, "\r\n-XXX-port%d/%d's test(%d/%d) failed(hardware error)-XXX-", SlotId, PortId, FailedTimes, TestTimes);
                    }
                }
            }
        }
    }while ( 0 != --iTimesLimit );

    vty_direct_out(vty, "\r\n-----pon%d/%d test(%d/%d) HOST-IF(%s) end-----\r\n", SlotId, PortId, FailedTimes, TestTimes, host_ifnames[statistics_parameter]);
    
    return iRlt;
}

static int pon_test_olt_startup(struct vty *vty, int SlotId, int PortId, short int PonPortIdx, int TestDelay, int TestTimes, int IsNewThread)
{
    int add_task_id; 
    unsigned long args[VOS_MAX_TASK_ARGS]={0,0,0,0,0,0,0,0,0,0};
    CHAR ucTaskName[20];

    args[0] = (ULONG)vty;
    args[1] = (ULONG)SlotId;
    args[2] = (ULONG)PortId;
    args[3] = (ULONG)PonPortIdx;
    args[4] = (ULONG)TestDelay;
    args[5] = (ULONG)TestTimes;
    args[6] = (ULONG)IsNewThread;
    VOS_Snprintf(ucTaskName, 18, "tTestPon%d", PonPortIdx);
    add_task_id = (int)VOS_TaskCreate(ucTaskName, TASK_PRIORITY_BELOW_NORMAL, pon_test_oltEx, args);
    if ( 0 == add_task_id )
    {
        vty_direct_out(vty, "\r\n-----port%d/%d's test failed(no task resource)-----\r\n", SlotId, PortId);
        return OLT_ERR_NORESC;
    }
        
    return OLT_ERR_OK;
}

int pon_test_oltEx2(struct vty *vty, int SlotId, int PortId, int TestDelay, int TestTimes, int IsNewThread)
{
    int iRlt;
    short int PonPortIdx;

    VOS_ASSERT(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER);
    
    PonPortIdx = GetPonPortIdxBySlot(SlotId, PortId);

    if ( OLT_ISLOCAL(PonPortIdx) )
    {
        if ( PONCHIP_EXIST == getPonChipInserted( (unsigned char)SlotId, (unsigned char)PortId ) )
        {
            if ( IsNewThread > 0 )
            {
                iRlt = pon_test_olt_startup(vty, SlotId, PortId, PonPortIdx, TestDelay, TestTimes, IsNewThread);
            }
            else
            {
                iRlt = pon_test_oltEx((ULONG)vty, (ULONG)SlotId, (ULONG)PortId, (ULONG)PonPortIdx, (ULONG)TestDelay, (ULONG)TestTimes, 0, 0, 0, 0);
            }
        }
        else
        {
            iRlt = OLT_ERR_NOTEXIST;
            vty_direct_out(vty, "\r\n-----port%d/%d's test failed(not exist in flash)-----\r\n", SlotId, PortId);
        }
    }
    else
    {
        iRlt = OLT_ERR_PARAM;
        vty_direct_out(vty, "\r\n-----port%d/%d's test failed(param error)-----\r\n", SlotId, PortId);
    }

    return iRlt;
}
#endif


#if 0
int pon_add_olt1(  short int olt_id )
	
{
	short int result = PAS_EXIT_OK;
	/*the parameters is setting for add_olt()*/
	PON_olt_init_parameters_t  olt_initialization_parameters;
	PON_olt_update_parameters_t  updated_parameters;
	PAS_system_parameters_t  system_parameters;	
	PON_binary_t  olt_binary;
	/*for PON_olt_init_parameters_t  olt_initialization_parameters*/
	olt_initialization_parameters.optics_configuration.configuration_source = 1;
	olt_initialization_parameters.optics_configuration.agc_lock_time = 0x09;
	olt_initialization_parameters.optics_configuration.agc_reset_configuration.gate_offset = 0x12;
	olt_initialization_parameters.optics_configuration.agc_reset_configuration.discovery_offset = 0x12;
	olt_initialization_parameters.optics_configuration.agc_reset_configuration.duration = 0x0;
	olt_initialization_parameters.optics_configuration.agc_reset_configuration.polarity = 0;
	olt_initialization_parameters.optics_configuration.cdr_lock_time = 0x10;
	olt_initialization_parameters.optics_configuration.cdr_reset_configuration.gate_offset = 0x20;
	olt_initialization_parameters.optics_configuration.cdr_reset_configuration.discovery_offset = 0x20;
	olt_initialization_parameters.optics_configuration.cdr_reset_configuration.duration = 0x0;
	olt_initialization_parameters.optics_configuration.cdr_reset_configuration.polarity = 1;
	olt_initialization_parameters.optics_configuration.end_of_grant_reset_configuration.offset = 0x24;
	olt_initialization_parameters.optics_configuration.end_of_grant_reset_configuration.duration = 2;
	olt_initialization_parameters.optics_configuration.end_of_grant_reset_configuration.polarity = 0;
	olt_initialization_parameters.optics_configuration.discovery_re_locking_enable = 0;
	olt_initialization_parameters.optics_configuration.discovery_laser_rx_loss_polarity = 0;
	olt_initialization_parameters.optics_configuration.pon_tx_disable_line_polarity = 0;
	olt_initialization_parameters.optics_configuration.optics_dead_zone = 0xa;
	olt_initialization_parameters.optics_configuration.use_optics_signal_loss = 0;
	/*PON_olt_optics_polarity_configuration_t */
	olt_initialization_parameters.optics_configuration.polarity_configuration.pon_port_link_indication_polarity = 0;
	olt_initialization_parameters.optics_configuration.polarity_configuration.cni_port_link_indication_polarity = 1;
	/**/
	olt_initialization_parameters.discovery_laser_on_time = 0x20;
	olt_initialization_parameters.discovery_laser_off_time = 0x20;
	olt_initialization_parameters.igmp_configuration.enable_igmp_snooping = 0;
	olt_initialization_parameters.igmp_configuration.igmp_timeout = 0;
	olt_initialization_parameters.vlan_configuration.vlan_exchange_downlink_tag_prefix = 200;
	olt_initialization_parameters.vlan_configuration.nested_mode_vlan_type = 0x7153;
	olt_initialization_parameters.multiple_copy_broadcast_enable = 0;
	olt_initialization_parameters.discard_unlearned_addresses = 1;
	/*modify*/
	olt_initialization_parameters.firmware_image.type = 1;
	olt_initialization_parameters.firmware_image.source = PON_BINARY_SOURCE_MEMORY;
	olt_initialization_parameters.firmware_image.location =(char *)(FLASH_BASE + FIRMWARE_IMAGE_OFFSET);
	olt_initialization_parameters.firmware_image.size = 142336;
	/*olt_initialization_parameters.firmware_image.identification.user_id = NULL;
	olt_initialization_parameters.firmware_image.identification.key= NULL;*/
	#if 0
	olt_initialization_parameters.firmware_image.type = 0;
	olt_initialization_parameters.firmware_image.source = NULL;
	olt_initialization_parameters.firmware_image.location = NULL;
	olt_initialization_parameters.firmware_image.size = 0;
	/*olt_initialization_parameters.firmware_image.identification.user_id = {0};
	olt_initialization_parameters.firmware_image.identification.key= {0};*/

	#endif
	/*for updating olt parameter.the value:
	cmd update_olt_parameters 0 -1 -1 0 
	NOT_CHANGED NOT_CHANGED NOT_CHANGED 
	NOT_CHANGED 
	DISABLE -1 -1 -1 	--igmp snooping parameters
	802_3_AH BOTH 	--hec_configuration
	DISABLE 	DISABLE DISABLE 
	-1 LOW*/					   
	updated_parameters.max_rtt = -1;
	updated_parameters.address_table_aging_timer = -1;
	updated_parameters.cni_port_maximum_entries = 0;
	
	updated_parameters.arp_filtering_configuration.from_pon_to_firmware = -1;
	updated_parameters.arp_filtering_configuration.from_cni_to_firmware = -1;
	updated_parameters.arp_filtering_configuration.from_pon_to_cni = -1;
	updated_parameters.arp_filtering_configuration.from_cni_to_pon = -1;
	
	updated_parameters.igmp_configuration.enable_igmp_snooping = 0;
	updated_parameters.igmp_configuration.igmp_timeout = -1;
	updated_parameters.vlan_configuration.vlan_exchange_downlink_tag_prefix = -1;
	updated_parameters.vlan_configuration.nested_mode_vlan_type = -1;
	
	updated_parameters.hec_configuration.tx_hec_configuration = 1;
	updated_parameters.hec_configuration.rx_hec_configuration = 3;
	
	updated_parameters.vlan_tag_filtering_config.untagged_frames_filtering = -1;
	updated_parameters.vlan_tag_filtering_config.multiple_copy_broadcast_enable_mode = -1;
	updated_parameters.vlan_tag_filtering_config.filtering_unexpected_tagged_downstream_frames= -1;
	updated_parameters.upstream_default_priority = -1;
	updated_parameters.pon_tbc_polarity = PON_POLARITY_ACTIVE_LOW;/*value = 0*/

	/*for setting syste_parameters.the value:
	cmd set_system_parameters ENABLE 0 0 -1 0 ENABLE*/
	/*system_parameters.TBD = 1;ENABLE*/
	system_parameters.statistics_sampling_cycle = 0;
	system_parameters.monitoring_cycle = 0;
	system_parameters.host_olt_msgs_timeout = -1;
	system_parameters.olt_reset_timeout = 0;
	system_parameters.automatic_authorization_policy = 1;
	/*return(PAS_add_olt( 0,&olt_mac_address[0], &olt_initialization_parameters ));*/


	olt_binary.type = PON_OLT_BINARY_DBA_DLL;
	olt_binary.source= PON_BINARY_SOURCE_MEMORY;
	olt_binary.location =(void *)0xfe780000;
	olt_binary.size = 16956;
	VOS_MemSet(&(olt_binary.identification),0,sizeof(PON_binary_identification_t));
	/*Set_olt_host_mii_mac_address (olt_id, host_mii_mac_address );*/

	#ifndef PAS_DEBUG
	#define PAS_DEBUG
	#endif

		{
		int i;
		sys_console_printf(" The pon chip data path mac is : \r\n");
		for( i=0; i< BYTES_IN_MAC_ADDRESS; i++ )
			sys_console_printf(" %2x ", PonChipMgmtTable[olt_id].DataPathMAC[i] );

		sys_console_printf("\r\n" );

		}

/*	VOS_MemCpy( (void *)&PAS_initialization_parameters, (void *)&olt_initialization_parameters, sizeof( PON_olt_init_parameters_t ));
*/
 	if( varflag == 0 ){
		 result = PAS_add_olt_v4( olt_id,PonChipMgmtTable[olt_id].DataPathMAC, &olt_initialization_parameters); }
	else {
		pon_initialization_parameters_init();
		result = PAS_add_olt_v4( olt_id,PonChipMgmtTable[olt_id].DataPathMAC, &PAS_initialization_parameters_5001);}

	 if( result == PAS_EXIT_OK ){
		#ifdef PAS_DEBUG
			sys_console_printf ("PAS_EXIT_OK(PAS_add_olt())!\r\r\n");
		#endif
		}
	else
		{
		#ifdef PAS_DEBUG
			sys_console_printf("PAS_EXIT_ERR(PAS_add_olt())!\r\r\n");
		#endif
			return result;
		}
	sys_console_printf ("start PAS_olt_self_test() wait ... \r\r\n");
	result = PAS_olt_self_test(olt_id);
	if(result == PASCOMM_TEST_SUCCEEDED)
		{
		#ifdef PAS_DEBUG
			sys_console_printf("PASCOMM_TEST_SUCCEESS !\r\r\n");
		#endif
		}
	else
		{
		#ifdef PAS_DEBUG
			sys_console_printf("TEST_FAILED !\r\r\n");
		#endif
			return(result);
		}
	if( varflag == 0 )
		result = PAS_update_olt_parameters_v4  ( olt_id,&updated_parameters);
	else {
		result = update_Pon_parameters_5001( olt_id );
		/*result = PAS_update_olt_parameters(olt_id, &PAS_updated_parameters );*/
		}
	if(result == PAS_EXIT_OK)
		{
		#ifdef PAS_DEBUG
			sys_console_printf("success !PAS_update_olt_parameters()!\r\r\n");
			/*GwEpon_Errsys_console_printf(result);*/
		#endif
		}
	else
		{
		#ifdef PAS_DEBUG
			sys_console_printf("ERROR! PAS_update_olt_parameters() !\r\r\n");
			/*GwEpon_Errsys_console_printf(result);*/
		#endif
		return(result);
		
		}
#if 0	

#endif	
	result = PAS_load_olt_binary_v4 ( olt_id, &olt_binary );
	if(result == PAS_EXIT_OK)
		{
		#ifdef PAS_DEBUG
			/*GwEpon_Errsys_console_printf(result);*/
		#endif
		}
	else
		{
		#ifdef PAS_DEBUG
			/*GwEpon_Errsys_console_printf(result);*/
		#endif
		return result;
		}	
	result = PAS_start_dba_algorithm( olt_id,0,0,NULL);
	if(result == PAS_EXIT_OK)
		{
		#ifdef PAS_DEBUG
			/*GwEpon_Errsys_console_printf(result);*/
		#endif
		}
	else
		{
		#ifdef PAS_DEBUG
			/*GwEpon_Errsys_console_printf(result);*/
		#endif
		return result;
		}	
	PLATO2_init();
		
	sys_console_printf("Success!Added olt %d\r\r\n",olt_id);
	return 0;
}
#endif

/*****************************************************
 *
 *    Function:  pon_remove_olt( short int PonPortIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port  
 *                 
 *    Desc:    remove an olt from PAS-SOFT
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int pon_remove_olt(const short int PonPortIdx )
{
	short int result = PAS_EXIT_ERROR;
	/*short int  PonChipIdx; */
	short int  PonChipType;
	short int  PonChipVer = RERROR;
	short int OnuIdx;
	int onuStatus;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
		
	if( OLT_PONCHIP_ISPAS5001(PonChipType))
	{	
	
		result=PAS_remove_olt_v4( PonPortIdx );
	}
	else if(OLT_PONCHIP_ISPAS1G(PonChipType))
	{
	
		result=PAS_reset_olt( PonPortIdx );
	}
#if defined(_EPON_10G_PMC_SUPPORT_)            
	else if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
		result=GW10G_PAS_reset_olt( PonPortIdx );
	}
#endif
	
	/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	if(result != PAS_EXIT_OK){
		sys_console_printf("  Remove %s/port%d failed !\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		return( RERROR );
		}
	
	/*1 if there are alarm existing, send alarm clear Trap to NMS, include pon and all registered ONU */

#if 0
	/*2  clear the onu mgmt table running data */
	for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++){

		/* shutDown the registered onu */
		onuStatus = GetOnuOperStatus_1(PonPortIdx, OnuIdx);
		if( onuStatus != ONU_OPER_STATUS_DOWN)
			{
			/* send onu deregister msg to NMS */
			if( onuStatus == ONU_OPER_STATUS_UP)
				Trap_OnuDeregister( PonPortIdx, OnuIdx , PON_ONU_DEREGISTRATION_HOST_REQUEST, 0 );
				
			/* send onu alarm clear msg to NMS */
				
			ClearOnuRunningData( PonPortIdx, OnuIdx, 0 );		
			}
		}
#else
    OnuEvent_ClearRunningDataMsg_Send(PonPortIdx);
#endif
	/*3 clear the pon mgmt table running data */
	ClearPonPortRunningData( PonPortIdx );
	PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ONLINE;
	PonChipMgmtTable[PonPortIdx].Type = PONCHIP_UNKNOWN;
	PonChipMgmtTable[PonPortIdx].version = PONCHIP_UNKNOWN;
	PonChipMgmtTable[PonPortIdx].Err_counter = 0;
	
	return( ROK );
}


int pon_reset_olt(short int PonPortIdx)
{
	short int result = PAS_EXIT_ERROR;
	/*short int  PonChipIdx; */
	short int  PonChipType;
	short int OnuIdx;
	short int onuStatus;
	
	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	if( OLT_PONCHIP_ISPAS1G(PonChipType))
	{
		result=PAS_reset_olt( PonPortIdx );
	}
#if defined(_EPON_10G_PMC_SUPPORT_)            
	else if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
	    result=GW10G_PAS_reset_olt( PonPortIdx );
	}
#endif
	/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	
	/*
	if(result != PAS_EXIT_OK){
		sys_console_printf("  Reset %s/port%d failed !\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		return( RERROR );
		}
	*/
	/*1 if there are alarm existing, send alarm clear Trap to NMS, include pon and all registered ONU */

#if 0
	/*2  clear the onu mgmt table running data */
	for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++){

		/* shutDown the registered onu */
		onuStatus = GetOnuOperStatus_1(PonPortIdx, OnuIdx);
		if( onuStatus != ONU_OPER_STATUS_DOWN)
			{
			/* send onu deregister msg to NMS */
			if( onuStatus == ONU_OPER_STATUS_UP)
				Trap_OnuDeregister( PonPortIdx, OnuIdx,PON_ONU_DEREGISTRATION_HOST_REQUEST, 0 );
				
			/* send onu alarm clear msg to NMS */
				
			ClearOnuRunningData( PonPortIdx, OnuIdx, 0 );		
			}
		}
#else
    OnuEvent_ClearRunningDataMsg_Send(PonPortIdx);
#endif
	/*3 clear the pon mgmt table running data */
	ClearPonPortRunningData( PonPortIdx );
	PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ONLINE;
	PonChipMgmtTable[PonPortIdx].Type = PONCHIP_UNKNOWN;
	PonChipMgmtTable[PonPortIdx].version = PONCHIP_UNKNOWN;
	PonChipMgmtTable[PonPortIdx].Err_counter = 0;

	return( ROK );
}

#if 0
int Hardware_Reset_olt( unsigned long ulSlot, unsigned long port)
{
	unsigned char ResetFlag=0xff;
	unsigned char ResetEnable=0xff;
	int Product_type = GetOltType();

	short int PonPortIdx, PonChipType;
	
	if(PonCardSlotPortCheckByVty(ulSlot, port, 0) != ROK ) return(RERROR);

	if(Product_type == V2R1_OLT_GFA6700)
		{
	/* 只有PAS5201PON板才可以对单个PON芯片复位*/
	PonPortIdx = GetPonPortIdxBySlot(ulSlot, port);	
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if( PonChipType != PONCHIP_PAS5201 ) return( RERROR );

	if(( ulSlot == PON1 ) || ( ulSlot == PON2 ) || ( ulSlot == PON3 ) || ( ulSlot == PON4 ))
	{
		ReadCPLDReg((unsigned char *)8, &ResetFlag);
		if(( ulSlot == PON1 ) || ( ulSlot == PON2 ))
			ReadCPLDReg((unsigned char *)10, &ResetEnable);
		else if(( ulSlot == PON3 ) || ( ulSlot == PON4 ))
			ReadCPLDReg((unsigned char *)9, &ResetEnable);
	}

	else if( ulSlot == PON5 )
		{
		ReadCPLDReg((unsigned char *)12, &ResetFlag);
		ReadCPLDReg((unsigned char *)8, &ResetEnable);
		}
	
	if( ulSlot == PON1 )
		ResetFlag = ResetFlag | 0x80;
	else if( ulSlot == PON2 )
		ResetFlag = ResetFlag | 0x40;
	else if (ulSlot == PON3 )
		ResetFlag = ResetFlag | 0x20;
	else if( ulSlot == PON4 )
		ResetFlag = ResetFlag | 0x10;
	else if( ulSlot == PON5 )
		ResetFlag = ResetFlag | 0x08;
	
	if( ulSlot != PON5 )
		WriteCPLDReg ( (unsigned char *)8, ResetFlag );
	else 
		WriteCPLDReg ( (unsigned char *)12, ResetFlag );
	

	if(( ulSlot == PON1 ) || ( ulSlot == PON3 ))
		{
		if( port == 1 )
			ResetEnable = ResetEnable | 0x10;
		else if( port == 2 )
			ResetEnable = ResetEnable | 0x20;
		else if( port == 3 )
			ResetEnable = ResetEnable | 0x40;
		else if( port == 4 )
			ResetEnable = ResetEnable | 0x80;
		}

	if(( ulSlot == PON2 ) || ( ulSlot == PON4 ) || ( ulSlot == PON5 ))
		{
		if( port == 1 )
			ResetEnable = ResetEnable | 0x01;
		else if( port == 2 )
			ResetEnable = ResetEnable | 0x02;
		else if( port == 3 )
			ResetEnable = ResetEnable | 0x04;
		else if( port == 4 )
			ResetEnable = ResetEnable | 0x08;
		}

	if(( ulSlot == PON1 ) || ( ulSlot == PON2 ))
		WriteCPLDReg((unsigned char *)10, ResetEnable);
	else if(( ulSlot == PON3 ) || ( ulSlot == PON4 ))
		WriteCPLDReg((unsigned char *)9, ResetEnable);
	else 
		WriteCPLDReg((unsigned char *)8, ResetEnable);

	VOS_TaskDelay(VOS_TICK_SECOND/3);

	if(( ulSlot == PON1 ) || ( ulSlot == PON3 ))
		{
		if( port == 1 )
			ResetEnable = ResetEnable & 0xef;
		else if( port == 2 )
			ResetEnable = ResetEnable & 0xdf;
		else if( port == 3 )
			ResetEnable = ResetEnable & 0xbf;
		else if( port == 4 )
			ResetEnable = ResetEnable & 0x7f;
		}

	if(( ulSlot == PON2 ) || ( ulSlot == PON4 ) || ( ulSlot == PON5 ))
		{
		if( port == 1 )
			ResetEnable = ResetEnable & 0xfe;
		else if( port == 2 )
			ResetEnable = ResetEnable & 0xfd;
		else if( port == 3 )
			ResetEnable = ResetEnable & 0xfb;
		else if( port == 4 )
			ResetEnable = ResetEnable & 0xf7;
		}

	if(( ulSlot == PON1 ) || ( ulSlot == PON2 ))
		WriteCPLDReg((unsigned char *)10, ResetEnable);
	else if(( ulSlot == PON3 ) || ( ulSlot == PON4 ))
		WriteCPLDReg((unsigned char *)9, ResetEnable);	
	else 
		WriteCPLDReg((unsigned char *)8, ResetEnable);

	VOS_TaskDelay(VOS_TICK_SECOND/3);

	/*PonPortIdx =  GetPonPortIdxBySlot(ulSlot, port);

	Add_PonPort( PonPortIdx );*/
		}

	else if(Product_type == V2R1_OLT_GFA6100)
		{
		if(ulSlot == PON1) 
			{
			ReadCPLDReg((unsigned char *)7, &ResetEnable);
			if(port == 1 )
				ResetEnable = ResetEnable & 0xf7;
			else if(port == 2 )
				ResetEnable = ResetEnable & 0xfb;
			}
		else if(ulSlot == PON2)
			{
			ReadCPLDReg((unsigned char *)8, &ResetEnable);
			if(port == 2 )
				ResetEnable = ResetEnable & 0xf7;
			else if(port == 1 )
				ResetEnable = ResetEnable & 0xfb;
			}

		if(ulSlot == PON1 )
			WriteCPLDReg((unsigned char *)7,ResetEnable);
		else if(ulSlot == PON2)
			WriteCPLDReg((unsigned char *)8,ResetEnable);
		
		VOS_TaskDelay(VOS_TICK_SECOND/3);

		if(ulSlot == PON1)
			{
			if(port == 1 )
				ResetEnable = ResetEnable | 0x08;
			else if(port == 2 )
				ResetEnable = ResetEnable | 0x04;
			}
		else if(ulSlot == PON2)
			{
			if(port == 2 )
				ResetEnable = ResetEnable | 0x08;
			else if(port == 1 )
				ResetEnable = ResetEnable | 0x04;
			}
		if(ulSlot == PON1 )
			WriteCPLDReg((unsigned char *)7,ResetEnable);
		else if(ulSlot == PON2)
			WriteCPLDReg((unsigned char *)8,ResetEnable);

		VOS_TaskDelay(VOS_TICK_SECOND/3);
		}

	return( ROK );		

}
#endif

int Hardware_Reset_olt1( unsigned long ulSlot, unsigned long port, unsigned long wait_delay, unsigned long force)
{
	unsigned char ResetFlag = 0;
	unsigned char ResetEnable = 0;
	unsigned char ResetFlag2 = 0;
	unsigned char ResetEnable2 = 0;
	short int PonPortIdx, PonChipType;
	int Product_type;
	LONG lLockKey;

	if(PonCardSlotPortCheckByVty(ulSlot, port, 0) != ROK ) return(RERROR);

    Product_type = GetOltType();
	if(Product_type == V2R1_OLT_GFA6700)
	{
		/* 只有PAS5201PON板才可以对单个PON芯片复位*/
		PonPortIdx = GetPonPortIdxBySlot(ulSlot, port);	
		PonChipType = V2R1_GetPonchipType( PonPortIdx );
		if( PonChipType != PONCHIP_PAS5201 )
        {
            if ( wait_delay > 0 )
            {
                VOS_TaskDelay( VOS_TICK_SECOND/2 );
            }
            
            return( RERROR );
        }      

		VOS_TaskLock();		/* added by xieshl 20100730 */
		lLockKey = VOS_IntLock();

		if( ulSlot == 4 )
        {
			ReadCPLDReg((volatile unsigned char *)12, &ResetFlag);
			ReadCPLDReg((volatile unsigned char *)8, &ResetEnable);
        }
        else
        {
			ReadCPLDReg((volatile unsigned char *)8, &ResetFlag);
			if(( ulSlot == 5 ) || ( ulSlot == 6 ))
				ReadCPLDReg((volatile unsigned char *)9, &ResetEnable);
			else if(( ulSlot == 7 ) || ( ulSlot == 8 ))
				ReadCPLDReg((volatile unsigned char *)10, &ResetEnable);
        }

        /* B--modified by liwei056@2011-12-27 for D14273 */
#if 0
		if( ulSlot == 4 )
			ResetFlag = ResetFlag | 0x08;
		else if( ulSlot == 5 )
			ResetFlag = ResetFlag | 0x10;
		else if (ulSlot == 6 )
			ResetFlag = ResetFlag | 0x20;
		else if( ulSlot == 7 )
			ResetFlag = ResetFlag | 0x40;
		else if( ulSlot == 8 )
			ResetFlag = ResetFlag | 0x80;
		
		if( ulSlot != 4 )
			WriteCPLDReg ( (volatile unsigned char *)8, ResetFlag );
		else 
			WriteCPLDReg ( (volatile unsigned char *)12, ResetFlag );
#else
		if( ulSlot == 4 )
			ResetFlag2 = 0x08;
		else if( ulSlot == 5 )
			ResetFlag2 = 0x10;
		else if (ulSlot == 6 )
			ResetFlag2 = 0x20;
		else if( ulSlot == 7 )
			ResetFlag2 = 0x40;
		else if( ulSlot == 8 )
			ResetFlag2 = 0x80;

        if ( !(ResetFlag2 & ResetFlag) )
        {
            /* 设置卡复位标志时 */
            ResetFlag |= ResetFlag2;
            
            /* 需要清空PON复位寄存器 */
            switch (ulSlot)
            {
                case 5:
                    ResetEnable2 = ResetEnable & 0xF0;
                    CPLD_WRITE_LOCK(0x1f);                                
        			WriteCPLDReg((volatile unsigned char *)9, ResetEnable2);
                    CPLD_WRITE_UNLOCK(0x1f);            
                    break;
                case 6:
                    ResetEnable2 = ResetEnable & 0x0F;
                    CPLD_WRITE_LOCK(0x1f);                                
        			WriteCPLDReg((volatile unsigned char *)9, ResetEnable2);
                    CPLD_WRITE_UNLOCK(0x1f);            
                    break;
                case 7:
                    ResetEnable2 = ResetEnable & 0xF0;
                    CPLD_WRITE_LOCK(0x1f);                                
        			WriteCPLDReg((volatile unsigned char *)10, ResetEnable2);
                    CPLD_WRITE_UNLOCK(0x1f);            
                    break;
                case 8:
                    ResetEnable2 = ResetEnable & 0x0F;
                    CPLD_WRITE_LOCK(0x1f);                                
        			WriteCPLDReg((volatile unsigned char *)10, ResetEnable2);
                    CPLD_WRITE_UNLOCK(0x1f);            
                    break;
                default:    
                    ResetEnable2 = ResetEnable & 0xF0;
                    CPLD_WRITE_LOCK(0x1f);                                
        			WriteCPLDReg((volatile unsigned char *)8, ResetEnable2);
                    CPLD_WRITE_UNLOCK(0x1f);            
            }
            ResetEnable = ResetEnable2;

    		if( ulSlot != 4 )
    			WriteCPLDReg ( (volatile unsigned char *)8, ResetFlag );
    		else 
    			WriteCPLDReg ( (volatile unsigned char *)12, ResetFlag );
        }	
#endif        
        /* E--modified by liwei056@2011-12-27 for D14273 */
		
		if(( ulSlot == 8 ) || ( ulSlot == 6 ))
		{
			if( port == 1 )
				ResetEnable = ResetEnable | 0x10;
			else if( port == 2 )
				ResetEnable = ResetEnable | 0x20;
			else if( port == 3 )
				ResetEnable = ResetEnable | 0x40;
			else if( port == 4 )
				ResetEnable = ResetEnable | 0x80;
		}

		if(( ulSlot == 5 ) || ( ulSlot == 7 ) || ( ulSlot == 4 ))
		{
			if( port == 1 )
				ResetEnable = ResetEnable | 0x01;
			else if( port == 2 )
				ResetEnable = ResetEnable | 0x02;
			else if( port == 3 )
				ResetEnable = ResetEnable | 0x04;
			else if( port == 4 )
				ResetEnable = ResetEnable | 0x08;
		}

		if(( ulSlot == 7 ) || ( ulSlot == 8 ))
			WriteCPLDReg((volatile unsigned char *)10, ResetEnable);
		else if(( ulSlot == 5 ) || ( ulSlot == 6 ))
			WriteCPLDReg((volatile unsigned char *)9, ResetEnable);
		else 
			WriteCPLDReg((volatile unsigned char *)8, ResetEnable);

		VOS_IntUnlock(lLockKey);
		VOS_TaskUnlock();
	}
	else if(Product_type == V2R1_OLT_GFA6100)
	{
		VOS_TaskLock();		/* added by xieshl 20100730 */
		lLockKey = VOS_IntLock();

		if(ulSlot == 2)
		{
			ReadCPLDReg((volatile unsigned char *)7, &ResetEnable);
			if(port == 1 )
				ResetEnable = ResetEnable & 0xf7;
			else if(port == 2 )
				ResetEnable = ResetEnable & 0xfb;
		}
		else if(ulSlot == 3)
		{
			ReadCPLDReg((volatile unsigned char *)8, &ResetEnable);
			if(port == 2 )
				ResetEnable = ResetEnable & 0xf7;
			else if(port == 1 )
				ResetEnable = ResetEnable & 0xfb;
		}

		if(ulSlot == 2 )
			WriteCPLDReg((volatile unsigned char *)7,ResetEnable);
		else if(ulSlot == 3)
			WriteCPLDReg((volatile unsigned char *)8,ResetEnable);

		VOS_IntUnlock(lLockKey);
		VOS_TaskUnlock();
	}
	else if( V2R1_PRODUCT_IS_H_Series(Product_type) )
	{
	    ULONG ulMoudleType = SYS_MODULE_TYPE(SYS_LOCAL_MODULE_SLOTNO);
        volatile UCHAR *pCPLDRegProtect;
        volatile UCHAR *pCPLDRegPonReset;

		PonPortIdx = GetPonPortIdxBySlot(ulSlot, port);
        PonChipType = PonPortIdx;
        switch (ulMoudleType)
        {
            case MODULE_E_GFA6900_4EPON:
            /*case MODULE_E_GFA8000_4EPON:*//*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
                pCPLDRegProtect  = 9;
                pCPLDRegPonReset = 4;
                
                if ( PonPortIdx >= 4 )
                {
                    VOS_ASSERT(0);
                    return RERROR;
                }

                ResetFlag = 16 << PonPortIdx;
                break;
            case MODULE_E_GFA6900_12EPON:
			case MODULE_E_GFA6900_12EPON_M:
			case MODULE_E_GFA6900_4EPON_4GE:
            /*case MODULE_E_GFA8000_12EPON:*/ /*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
			case MODULE_E_GFA8000_12EPON_M:
			case MODULE_E_GFA8000_4EPON_4GE:
                pCPLDRegProtect = 0x0b;
                if ( PonPortIdx >= 4 )
                {
                    PonPortIdx -= 4;
                }
                else
                {
                    VOS_ASSERT(0);
                    return RERROR;
                }
                
                if ( PonPortIdx < 8 )
                {
                    pCPLDRegPonReset = 0x0d;
                }
                else
                {
                    PonPortIdx -= 8;
                    if ( PonPortIdx >= 4 )
                    {
                        VOS_ASSERT(0);
                        return RERROR;
                    }
                    
                    pCPLDRegPonReset = 0x0e;
                }
                
                ResetFlag = 1 << PonPortIdx;
                
                break;
            case MODULE_E_GFA6900_12EPONB0:
            case MODULE_E_GFA6900_12EPONB0_M:
            case MODULE_E_GFA8000_12EPONB0:
            case MODULE_E_GFA8000_12EPONB0_M:
                pCPLDRegProtect  = 0x0b;
                pCPLDRegPonReset = 0x0d;
                if ( PonPortIdx >= 4 )
                {
                    if ( force )
                    {
                        /* 可以从芯片的任一个端口硬件复位芯片 */
                        if ( 2 < (PonPortIdx = ((PonPortIdx - 4) >> 2)) )
                        {
                            VOS_ASSERT(0);
                            return RERROR;
                        }
                    }
                    else
                    {
                        switch ( PonPortIdx )
                        {
                            /* 建议只有芯片的第一个端口硬件复位芯片 */
                            case 4:
                                PonPortIdx = 0;
                            break;
                            case 8:
                                PonPortIdx = 1;
                            break;
                            case 12:
                                PonPortIdx = 2;
                            break;
                            case 6:
                            case 10:
                            case 14:
                                /* 建议只有管理通道的第一个端口关中断 */
                                PonPortIdx = -1;
                            break;
                            default:
                                /* 其它端口不建议硬件复位，但是要等第一个端口的复位延时 */
                                PonPortIdx  = -1;
                            	PonChipType = -1;		
                        }
                    }
                }
                else
                {
                    VOS_ASSERT(0);
                    return RERROR;
                }
                
                ResetFlag = 1 << PonPortIdx;
                
                break;
            case MODULE_E_GFA6900_16EPONB1:
            case MODULE_E_GFA6900_16EPONB1_M:
            /*case MODULE_E_GFA8000_16EPONB1:*/  /*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
            case MODULE_E_GFA8000_16EPONB1_M:
			case MODULE_E_GFA8100_16EPONB0:
                pCPLDRegProtect  = 0x0b;
                pCPLDRegPonReset = 0x0d;
                if ( PonPortIdx >= 0 )
                {
                    if ( force )
                    {
                        /* 可以从芯片的任一个端口硬件复位芯片 */
                        if ( 3 < (PonPortIdx >>= 2) )
                        {
                            VOS_ASSERT(0);
                            return RERROR;
                        }
                    }
                    else
                    {
                        switch ( PonPortIdx )
                        {
                            /* 建议只有芯片的第一个端口硬件复位芯片 */
                            case 0:
                                PonPortIdx = 0;
                            break;
                            case 4:
                                PonPortIdx = 1;
                            break;
                            case 8:
                                PonPortIdx = 2;
                            break;
                            case 12:
                                PonPortIdx = 3;
                            break;
                            case 2:
                            case 6:
                            case 10:
                            case 14:
                                /* 建议只有管理通道的第一个端口关中断 */
                                PonPortIdx = -1;
                            break;
                            default:
                                /* 其它端口不建议硬件复位，但是要等第一个端口的复位延时 */
                                PonPortIdx  = -1;
                            	PonChipType = -1;		
                        }
                    }
                }
                else
                {
                    VOS_ASSERT(0);
                    return RERROR;
                }
                
                ResetFlag = 1 << PonPortIdx;
                
                break;
#if defined(_EPON_10G_PMC_SUPPORT_)            
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
			case MODULE_E_GFA6900_10G_EPON:
			case MODULE_E_GFA6900_10G_EPON_M:
			
				pCPLDRegProtect  = 0x0b;
                if ( PonPortIdx >= 5 )
                {
                    PonPortIdx -= 5;
                }
                else
                {
                    VOS_ASSERT(0);
                    return RERROR;
                }
                pCPLDRegPonReset = 0x0d;
                                
                ResetFlag = 1 << PonPortIdx;
                
                break;
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif
			case MODULE_E_GFA8000_10G_8EPON:
			case MODULE_E_GFA8000_10G_8EPON_M:
				/*added by liyang @2015-01-29 for 8EXP */
				pCPLDRegProtect  = 0x0b;
                pCPLDRegPonReset = 0x0d;
                if ( PonPortIdx >= 0 )
                {
                    if ( force )
                    {
                        /* 可以从芯片的任一个端口硬件复位芯片 */
                        if ( 1 < (PonPortIdx >>= 3) )
                        {
                            VOS_ASSERT(0);
                            return RERROR;
                        }
                    }
                    else
                    {
                        switch ( PonPortIdx )
                        {
							/*modified by wangjiah@2016-09-01:begin
							 * Writing register doesn't reset pon chip hardware
							 * Using emmi api BcmOLT_ResetChip to reset chip*/
                            /* 建议只有芯片的第一个端口硬件复位芯片 */
							/*
                            case 0:
                                PonPortIdx = 0;
                            break;
                            default:
							*/
							/*modified by wangjiah@2016-09-01:begin*/
                                /* 其它端口不建议硬件复位，但是要等第一个端口的复位延时 */
						 	PonPortIdx  = -1;
							PonChipType = -1;		
                        }
                    }
                }
                else
                {
                    VOS_ASSERT(0);
                    return RERROR;
                }
                
                ResetFlag = 1 << PonPortIdx;
                
                break;
            default:
		 PonPortIdx  = -1;
            	PonChipType = -1;		
				/*34001 by jinhl@2016.12.15*/
                /*VOS_ASSERT(0);*/
				
				return ( ROK );
        }

        if ( (PonChipType >= 0) || (PonPortIdx >= 0) )
        {
    		VOS_TaskLock();		/* added by xieshl 20100730 */
    		lLockKey = VOS_IntLock();

            if ( PonChipType >= 0 )
            {
                /* 复位前，应该先关中断 */
                PonIntClose(PonChipType);
            }

            if ( PonPortIdx >= 0 )
            {
        		ReadCPLDReg( pCPLDRegPonReset, &ResetEnable );
                ResetEnable &= (~ResetFlag);

                CPLD_WRITE_LOCK(pCPLDRegProtect);
                VOS_TaskDelay(0);
                WriteCPLDReg( pCPLDRegPonReset, ResetEnable );
                CPLD_WRITE_UNLOCK(pCPLDRegProtect);
            }

    		VOS_IntUnlock(lLockKey);
    		VOS_TaskUnlock();
        }
    }
    else
    {
        VOS_ASSERT(0);
    }

    if ( wait_delay > 0 )
    {
        VOS_TaskDelay(VOS_TICK_SECOND/2);
    }
    
	return( ROK );		
}

int Hardware_Reset_olt2( unsigned long ulSlot, unsigned long port, unsigned long wait_delay, unsigned long force)
{
	unsigned char ResetFlag = 0;
	unsigned char ResetEnable = 0;
	short int PonChipType;
	short int PonPortIdx;
	int Product_type;
	ULONG ulWaitReadyTicks;
	LONG lLockKey;

	if(PonCardSlotPortCheckByVty(ulSlot, port, 0) != ROK ) return(RERROR);

    ulWaitReadyTicks = VOS_TICK_SECOND / 3;
    Product_type = GetOltType();
	if(Product_type == V2R1_OLT_GFA6700)
	{
		/* 只有PAS5201PON板才可以对单个PON芯片复位*/
		PonPortIdx = GetPonPortIdxBySlot(ulSlot, port);	
		PonChipType = V2R1_GetPonchipType( PonPortIdx );
		if( PonChipType != PONCHIP_PAS5201 ) return( RERROR );

		VOS_TaskLock();		/* added by xieshl 20100730 */
		lLockKey = VOS_IntLock();

		if( ulSlot == 4 )
        {
			ReadCPLDReg((volatile unsigned char *)12, &ResetFlag);
			ReadCPLDReg((volatile unsigned char *)8, &ResetEnable);
        }
        else
        {
			ReadCPLDReg((volatile unsigned char *)8, &ResetFlag);
			if(( ulSlot == 5 ) || ( ulSlot == 6 ))
				ReadCPLDReg((volatile unsigned char *)9, &ResetEnable);
			else if(( ulSlot == 7 ) || ( ulSlot == 8 ))
				ReadCPLDReg((volatile unsigned char *)10, &ResetEnable);
        }
        
		if( ulSlot == 4 )
			ResetFlag = ResetFlag | 0x08;
		else if( ulSlot == 5 )
			ResetFlag = ResetFlag | 0x10;
		else if (ulSlot == 6 )
			ResetFlag = ResetFlag | 0x20;
		else if( ulSlot == 7 )
			ResetFlag = ResetFlag | 0x40;
		else if( ulSlot == 8 )
			ResetFlag = ResetFlag | 0x80;

		if( ulSlot != 4 )
			WriteCPLDReg ( (volatile unsigned char *)8, ResetFlag );
		else 
			WriteCPLDReg ( (volatile unsigned char *)12, ResetFlag );
		

		if(( ulSlot == 8 ) || ( ulSlot == 6 ))
		{
			if( port == 1 )
				ResetEnable = ResetEnable & 0xef;
			else if( port == 2 )
				ResetEnable = ResetEnable & 0xdf;
			else if( port == 3 )
				ResetEnable = ResetEnable & 0xbf;
			else if( port == 4 )
				ResetEnable = ResetEnable & 0x7f;
		}

		if(( ulSlot == 5 ) || ( ulSlot == 7 ) || ( ulSlot == 4 ))
		{
			if( port == 1 )
				ResetEnable = ResetEnable & 0xfe;
			else if( port == 2 )
				ResetEnable = ResetEnable & 0xfd;
			else if( port == 3 )
				ResetEnable = ResetEnable & 0xfb;
			else if( port == 4 )
				ResetEnable = ResetEnable & 0xf7;
		}

		if(( ulSlot == 7 ) || ( ulSlot == 8 ))
			WriteCPLDReg((volatile unsigned char *)10, ResetEnable);
		else if(( ulSlot == 5 ) || ( ulSlot == 6 ))
			WriteCPLDReg((volatile unsigned char *)9, ResetEnable);
		else 
			WriteCPLDReg((volatile unsigned char *)8, ResetEnable);

		VOS_IntUnlock(lLockKey);
		VOS_TaskUnlock();
	}
	else if(Product_type == V2R1_OLT_GFA6100)
	{
		VOS_TaskLock();		/* added by xieshl 20100730 */
		lLockKey = VOS_IntLock();

		if(ulSlot == 2) 
		{
			ReadCPLDReg((volatile unsigned char *)7, &ResetEnable);
			if(port == 1 )
				ResetEnable = ResetEnable | 0x08;
			else if(port == 2 )
				ResetEnable = ResetEnable | 0x04;
		}
		else if(ulSlot == 3)
		{
			ReadCPLDReg((volatile unsigned char *)8, &ResetEnable);
			if(port == 2 )
				ResetEnable = ResetEnable | 0x08;
			else if(port == 1 )
				ResetEnable = ResetEnable | 0x04;			
		}
		
		if(ulSlot == 2 )
			WriteCPLDReg((volatile unsigned char *)7,ResetEnable);
		else if(ulSlot == 3)
			WriteCPLDReg((volatile unsigned char *)8,ResetEnable);

		VOS_IntUnlock(lLockKey);
		VOS_TaskUnlock();
	}
	else if( V2R1_PRODUCT_IS_H_Series(Product_type) )
	{
	    ULONG ulMoudleType = SYS_MODULE_TYPE(SYS_LOCAL_MODULE_SLOTNO);
        volatile UCHAR *pCPLDRegProtect;
        volatile UCHAR *pCPLDRegPonReset;

		PonPortIdx = GetPonPortIdxBySlot(ulSlot, port);	
        PonChipType = PonPortIdx;
        switch (ulMoudleType)
        {
            case MODULE_E_GFA6900_4EPON:
            /*case MODULE_E_GFA8000_4EPON:*//*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
                pCPLDRegProtect  = 9;
                pCPLDRegPonReset = 4;
                
                if ( PonPortIdx >= 4 )
                {
                    VOS_ASSERT(0);
                    return RERROR;
                }

                ResetFlag = 16 << PonPortIdx;
                break;
            case MODULE_E_GFA6900_12EPON:
			case MODULE_E_GFA6900_12EPON_M:	
			case MODULE_E_GFA6900_4EPON_4GE:
            /*case MODULE_E_GFA8000_12EPON:*/ /*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
			case MODULE_E_GFA8000_12EPON_M:	
			case MODULE_E_GFA8000_4EPON_4GE:
                pCPLDRegProtect  = 0x0b;
                if ( PonPortIdx >= 4 )
                {
                    PonPortIdx -= 4;
                }
                else
                {
                    VOS_ASSERT(0);
                    return RERROR;
                }
                
                if ( PonPortIdx < 8 )
                {
                    pCPLDRegPonReset = 0x0d;
                }
                else
                {
                    PonPortIdx -= 8;
                    if ( PonPortIdx >= 4 )
                    {
                        VOS_ASSERT(0);
                        return RERROR;
                    }
                    
                    pCPLDRegPonReset = 0x0e;
                }
                
                ResetFlag = 1 << PonPortIdx;
                
                break;
            case MODULE_E_GFA6900_12EPONB0:
            case MODULE_E_GFA6900_12EPONB0_M:
            case MODULE_E_GFA8000_12EPONB0:
            case MODULE_E_GFA8000_12EPONB0_M:
                /* TK芯片硬件复位启动需要5~8秒 */
                ulWaitReadyTicks = VOS_TICK_SECOND * 8;
                
                pCPLDRegProtect  = 0x0b;
                pCPLDRegPonReset = 0x0d;
                if ( PonPortIdx >= 4 )
                {
                    if ( force )
                    {
                        /* 可以从芯片的任一个端口硬件复位芯片 */
                        if ( 2 < (PonPortIdx = ((PonPortIdx - 4) >> 2)) )
                        {
                            VOS_ASSERT(0);
                            return RERROR;
                        }
                    }
                    else
                    {
                        switch ( PonPortIdx )
                        {
                            /* 建议只有芯片的第一个端口硬件复位芯片 */
                            case 4:
                                PonPortIdx = 0;
                            break;
                            case 8:
                                PonPortIdx = 1;
                            break;
                            case 12:
                                PonPortIdx = 2;
                            break;
                            case 6:
                            case 10:
                            case 14:
                                /* 建议只有管理通道的第一个端口开中断 */
                                PonPortIdx = -1;
                            break;
                            default:
                                /* 其它端口不建议硬件复位，但是要等第一个端口的复位延时 */
                            	PonChipType = -1;		
                        }
                    }
                }
                else
                {
                    VOS_ASSERT(0);
                    return RERROR;
                }
                
                ResetFlag = 1 << PonPortIdx;
                
                break;
            case MODULE_E_GFA6900_16EPONB1:
            case MODULE_E_GFA6900_16EPONB1_M:
            /*case MODULE_E_GFA8000_16EPONB1:*/  /*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
            case MODULE_E_GFA8000_16EPONB1_M:
			case MODULE_E_GFA8100_16EPONB0:
                /* TK芯片硬件复位启动需要5~8秒 */
                ulWaitReadyTicks = VOS_TICK_SECOND * 8;
                
                pCPLDRegProtect  = 0x0b;
                pCPLDRegPonReset = 0x0d;
                if ( PonPortIdx >= 0 )
                {
                    if ( force )
                    {
                        /* 可以从芯片的任一个端口硬件复位芯片 */
                        if ( 3 < (PonPortIdx >>= 2) )
                        {
                            VOS_ASSERT(0);
                            return RERROR;
                        }
                    }
                    else
                    {
                        switch ( PonPortIdx )
                        {
                            /* 建议只有芯片的第一个端口硬件复位芯片 */
                            case 0:
                                PonPortIdx = 0;
                            break;
                            case 4:
                                PonPortIdx = 1;
                            break;
                            case 8:
                                PonPortIdx = 2;
                            break;
                            case 12:
                                PonPortIdx = 3;
                            break;
                            case 2:
                            case 6:
                            case 10:
                            case 14:
                                /* 建议只有管理通道的第一个端口开中断 */
                                PonPortIdx = -1;
                            break;
                            default:
                                /* 其它端口不建议硬件复位，但是要等第一个端口的复位延时 */
                                PonPortIdx  = -1;
                            	PonChipType = -1;		
                        }
                    }
                }
                else
                {
                    VOS_ASSERT(0);
                    return RERROR;
                }
                
                ResetFlag = 1 << PonPortIdx;
                
                break;
#if defined(_EPON_10G_PMC_SUPPORT_)            
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
			case MODULE_E_GFA6900_10G_EPON:
			case MODULE_E_GFA6900_10G_EPON_M:
				pCPLDRegProtect  = 0x0b;
                if ( PonPortIdx >= 5 )
                {
                    PonPortIdx -= 5;
                }
                else
                {
                    VOS_ASSERT(0);
                    return RERROR;
                }
                pCPLDRegPonReset = 0x0d;
                              
                ResetFlag = 1 << PonPortIdx;
                
                break;
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif
			case MODULE_E_GFA8000_10G_8EPON:
			case MODULE_E_GFA8000_10G_8EPON_M:
				/*added by liyang @2015-01-29 for 8EXP */
				/* BCM 芯片硬件复位启动需要5~8秒 */
                ulWaitReadyTicks = VOS_TICK_SECOND * 8;
                
                pCPLDRegProtect  = 0x0b;
                pCPLDRegPonReset = 0x0d;
                if ( PonPortIdx >= 0 )
                {
                    if ( force )
                    {
                        /* 可以从芯片的任一个端口硬件复位芯片 */
                        if ( 1 < (PonPortIdx >>= 3) )
                        {
                            VOS_ASSERT(0);
                            return RERROR;
                        }
                    }
                    else
                    {
                        switch ( PonPortIdx )
                        {
                            /* 建议只有芯片的第一个端口硬件复位芯片 */
                            case 0:
                                PonPortIdx = 0;
                            break;
                            default:
                                /* 其它端口不建议硬件复位，但是要等第一个端口的复位延时 */
                                PonPortIdx  = -1;
                            	PonChipType = -1;		
                        }
                    }
                }
                else
                {
                    VOS_ASSERT(0);
                    return RERROR;
                }
                
                ResetFlag = 1 << PonPortIdx;
                				
                break;
            default:
		    PonPortIdx  = -1;
            	PonChipType = -1;		
				/*34001 by jinhl@2016.12.15*/
                /*VOS_ASSERT(0);*/
				return ( ROK );
        }

        if ( (PonChipType >= 0) || (PonPortIdx >= 0) )
        {
    		VOS_TaskLock();		/* added by xieshl 20100730 */
    		lLockKey = VOS_IntLock();

            if ( PonPortIdx >= 0 )
            {
        		ReadCPLDReg(  pCPLDRegPonReset, &ResetEnable);
                ResetEnable |= ResetFlag;

                CPLD_WRITE_LOCK(pCPLDRegProtect);
                VOS_TaskDelay(0);
                WriteCPLDReg( pCPLDRegPonReset, ResetEnable );
                CPLD_WRITE_UNLOCK(pCPLDRegProtect);
            }

            if ( PonChipType >= 0 )
            {
                /* 复位后，应该立即打开中断 */
                PonIntOpen(PonChipType);
            }
            
     		VOS_IntUnlock(lLockKey);
    		VOS_TaskUnlock();
        }
    }
    else
    {
        VOS_ASSERT(0);
    }

    if ( wait_delay > 0 )
    {
    	VOS_TaskDelay( ulWaitReadyTicks );
    }

	return( ROK );		

}

int ClearPonRunningData(short int PonPortIdx)
{
	/*short int result = PAS_EXIT_ERROR;
	short int  PonChipIdx; 
	short int  PonChipType;*/
	short int OnuIdx;
	short int onuStatus;
	
	CHECK_PON_RANGE
   
    /*if( PonPortIsWorking( PonPortIdx ) == TRUE )*/
    if(OLTAdv_IsExist(PonPortIdx) == TRUE )
    {
    	PONTx_Disable( PonPortIdx, PONPORT_TX_ACTIVED );
    }   
    else
    {
        if ( OLT_ADAPTER_RPC < OLT_GetIFType(PonPortIdx) )
        {
            pon_lose_olt(PonPortIdx);
            OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) release the PON-Chip(%d)'s LOCAL-PON service at the ClearPonRunningData().\r\n", SYS_LOCAL_MODULE_SLOTNO, PonPortIdx);
        }
    }
#if 0	
	/*2  clear the onu mgmt table running data */
	for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
	{
		/* shutDown the registered onu */
		onuStatus = GetOnuOperStatus_1(PonPortIdx, OnuIdx);
		if( onuStatus != ONU_OPER_STATUS_DOWN)
			{
			/* send onu deregister msg to NMS */
			if( onuStatus == ONU_OPER_STATUS_UP)
			{
            	OltOamPtyOnuLoseNoti(PonPortIdx, OnuIdx+1);
				Trap_OnuDeregister( PonPortIdx, OnuIdx, PON_ONU_DEREGISTRATION_HOST_REQUEST, 0 );
			}	
			/* send onu alarm clear msg to NMS */
				
			/* modified by xieshl 20110624, 问题单13090 */
			SetOnuOperStatus(PonPortIdx, OnuIdx, ONU_OPER_STATUS_DOWN );

			ClearOnuRunningData( PonPortIdx,  OnuIdx, 0 );		
			}
		}
#else
    /*modi 2013-1-23*/
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        OnuEvent_ClearRunningDataMsg_Send(PonPortIdx);
#endif
	/*3 clear the pon mgmt table running data */
	PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ONLINE;
	PonChipMgmtTable[PonPortIdx].Type = PONCHIP_UNKNOWN;
	PonChipMgmtTable[PonPortIdx].version = PONCHIP_UNKNOWN;
	PonChipMgmtTable[PonPortIdx].TypeName[0] = 0;
	PonChipMgmtTable[PonPortIdx].Err_counter = 0;
	
	UpdatePonPortNum();

	return( ROK );
}

int ShutDown_olt( short int PonPortIdx )
{
	short int PonChipType/*, PonChipVer*/;
	short int CardIndex;
	unsigned char command1[] = "dba";
	unsigned char command2[] = "stop";
	unsigned char command3[] = "..";

	CHECK_PON_RANGE

	CardIndex = GetCardIdxByPonChip( PonPortIdx );
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	if( PonChipActivatedFlag[CardIndex] != TRUE ) return( RERROR);
	if( PonPortIsWorking( PonPortIdx ) != TRUE ) return( RERROR);
	
	if( OLT_PONCHIP_ISPAS(PonChipType))
	{
		
		Pon_SendCliCommand( PonPortIdx, PonChipType, 2, command3 );
		VOS_TaskDelay(1);
		Pon_SendCliCommand( PonPortIdx, PonChipType, 2, command3 );
		VOS_TaskDelay(1);
		Pon_SendCliCommand( PonPortIdx, PonChipType, 3, command1 );
		VOS_TaskDelay(1);
		Pon_SendCliCommand( PonPortIdx, PonChipType, 4, command2);
		VOS_TaskDelay(2);
		Pon_SendCliCommand( PonPortIdx, PonChipType, 2, command3 );
		VOS_TaskDelay(1);
		
	}
	/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	else { /* other pon chip handler */
		}
	/*
	PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_DOWN;
	PonPortTable[PonPortIdx].PortAdminStatus =  PONPORT_DISABLE;
	*/
	return( ROK );
}

int StartUp_olt( short int PonPortIdx )
{
	short int PonChipType/*,PonChipVer*/;
	unsigned char command1[] = "dba";
	unsigned char command2[] = "ext";
	unsigned char command3[] = "..";
	/*short int ret = PAS_EXIT_OK;*/

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	if( PonChipMgmtTable[PonPortIdx].operStatus != PONCHIP_UP ) return( RERROR );
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
		
		Pon_SendCliCommand( PonPortIdx, PonChipType, 2, command3 );
		VOS_TaskDelay(1);
		Pon_SendCliCommand( PonPortIdx, PonChipType, 2, command3 );
		VOS_TaskDelay(1);
		Pon_SendCliCommand( PonPortIdx, PonChipType, 3, command1 );
		VOS_TaskDelay(1);
		Pon_SendCliCommand( PonPortIdx, PonChipType, 3, command2);
		VOS_TaskDelay(2);
		Pon_SendCliCommand( PonPortIdx, PonChipType, 2, command3 );  
		VOS_TaskDelay(1);
		
	}
	/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	else { /* other pon chip handler */

		}
	/*
	if( ret == PAS_EXIT_OK )
		{
		PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_UP;
		PonPortTable[PonPortIdx].PortAdminStatus =  PONPORT_ENABLE;
		}
	*/
	return( ROK );
}

/* modified by chenfj 2007/02/06
   #3142 问题单:重启OLT，有时ONU注册不上
*/

int  PONTx_Disable( short int PonPortIdx, int tx_reason )
{
	int ret;
#if 0
	short int PonChipType;
#endif

	CHECK_PON_RANGE

#if 1
	ret = OLTAdv_SetOpticalTxMode2(PonPortIdx, 0, tx_reason);
#else
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
	{
		/*PonChipVer = PonChipType;*/
		PonChipType = PONCHIP_PAS;
	}
	
	if( PonChipType == PONCHIP_PAS )
	{
		/*if( OLT_IsExist( PonPortIdx) ==  TRUE )*/
    	ret = PAS_set_olt_pon_transmission(PonPortIdx, (V2R1_DISABLE-V2R1_DISABLE));
	}
	else
    { /* other pon chip handler */
	}
#endif

    /*sys_console_printf(" pon %d tx disable \r\n", PonPortIdx );*/
	if( OLT_CALL_ISOK(ret) )
		return( ROK );
	else
        return ( RERROR );
}

int  PONTx_Enable( short int PonPortIdx, int tx_reason )
{
	int ret;
#if 0
	short int PonChipType;
#endif
	
	CHECK_PON_RANGE

#if 1
	ret = OLTAdv_SetOpticalTxMode2(PonPortIdx, 1, tx_reason);
#else
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
	{
		/*PonChipVer = PonChipType;*/
		PonChipType = PONCHIP_PAS;
	}
	if( PonChipType == PONCHIP_PAS )
	{
		/*if( OLT_IsExist(PonPortIdx ) == TRUE )*/
		ret = PAS_set_olt_pon_transmission(PonPortIdx, V2R1_ENABLE );
	}
	
	else
    {  /* other pon chip handler */
	}
#endif

	/*sys_console_printf(" pon %d tx enable \r\n", PonPortIdx );*/
	if( OLT_CALL_ISOK(ret) )
		return( ROK );
	else
        return( RERROR );
}

#if 1
#define ONU_SYNC_IF  RPU_NO

int CopyOnuLlid(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iRlt = 0;
    int iSrcEntry;
    short int sSrcLLID;

    iSrcEntry = SrcPonPortIdx * MAXONUPERPON + SrcOnuIdx;
    sSrcLLID  = OnuMgmtTable[iSrcEntry].LLID;
#if ( ONU_SYNC_IF == RPU_YES )
    if ( INVALID_LLID != sSrcLLID )
    {
        if ( !OLTAdv_LLIDIsExist(DstPonPortIdx, sSrcLLID) )
        {
            OLT_SYNC_DEBUG(OLT_SYNC_TITLE"CopyOnuLlid(%d, %d, %d, %d) failed in llid[%d].\r\n", DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, sSrcLLID );
            return INVALID_LLID;
        }
    }
#endif
    
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OnuMgt_SetOnuLLID(DstPonPortIdx, DstOnuIdx, sSrcLLID);
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( INVALID_LLID != sSrcLLID )
        {
            iRlt = OnuMgt_SetOnuLLID(DstPonPortIdx, DstOnuIdx, sSrcLLID);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            int iDstEntry;
            short int sDstLLID;
            
            iDstEntry = DstPonPortIdx * MAXONUPERPON + DstOnuIdx;
            sDstLLID  = OnuMgmtTable[iDstEntry].LLID;
            if ( sSrcLLID != sDstLLID )
            {
                iRlt = OnuMgt_SetOnuLLID(DstPonPortIdx, DstOnuIdx, sSrcLLID);
            }
        }
        else
        {
            iRlt = OnuMgt_SetOnuLLID(DstPonPortIdx, DstOnuIdx, sSrcLLID);
        }
    }

    return  (0 == iRlt) ? (int)sSrcLLID : iRlt;
}

#define ONU_COPY_CHECK(copy) if ( OLT_ERR_TIMEOUT == (copy) ) return OLT_ERR_TIMEOUT
int CopyOnuConfig(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    /* 拷贝业务开关配置*/
    ONU_COPY_CHECK( CopyOnuTrafficServiceMode(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags) );

    /* 拷贝带宽配置*/
    ONU_COPY_CHECK( CopyOnuUplinkBW(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags) );
    ONU_COPY_CHECK( CopyOnuDownlinkBW(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags) );

    /* 拷贝MAX-MAC配置*/
    ONU_COPY_CHECK( CopyOnuMaxMacCfg(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags) );

    /* 拷贝FecMode配置*/
    ONU_COPY_CHECK( CopyOnuFecMode(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags) );

    /* 拷贝p2p配置 */
#ifdef ONU_PEER_TO_PEER
    ONU_COPY_CHECK( CopyOnuPeerToPeerForward(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx) );
    ONU_COPY_CHECK( CopyOnuPeerToPeer(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags) );
#endif	

    /* 拷贝ONU 软件升级配置*/
    ONU_COPY_CHECK( CopyOnuSWUpdateCfg(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags) );

    /* 拷贝ONU 加密配置*/
    ONU_COPY_CHECK( CopyOnuEncryptParams(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags) );

    /* copy ONU mng IP para */
    ONU_COPY_CHECK( CopyOnuMngIpCfg(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags));    

    return 0;
}
#undef ONU_COPY_CHECK

#define OLT_COPY_CHECK(copy) if ( OLT_ERR_TIMEOUT == (copy) ) return OLT_ERR_TIMEOUT
int CopyOltConfig(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    /* 拷贝地址表配置*/
    OLT_COPY_CHECK( CopyOltAddressTableConfig(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );

    /* 拷贝OLT上下行QinQ配置*/
#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
    OLT_COPY_CHECK( CopyPonPortVlanTpid(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );
    OLT_COPY_CHECK( CopyDownlinkVlanManipulationToPon(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );
    OLT_COPY_CHECK( CopyUplinkVlanManipulationToPon(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );
#endif

    /* 当pon口状态为PONPORT_UP 时，恢复loid 表及认证使能added by luh 2012-2-16 */
    OLT_COPY_CHECK( CopyCtcOnuAuthLoid(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );   
    OLT_COPY_CHECK( CopyCtcOnuAuthMode(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );   
    OLT_COPY_CHECK( CopyPonPortAuthEnable(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );
    
    /* 拷贝MAC认证 配置*/
    OLT_COPY_CHECK( CopyOltAuthTable(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );
    /*拷贝gpon认证配置*/
    OLT_COPY_CHECK( CopyGponOnuAuthEntry(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );

  
    /* 拷贝下行限速模式配置*/
    OLT_COPY_CHECK( CopyPonDownlinkPoliceMode(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );

    /* 拷贝PON参数配置*/
    OLT_COPY_CHECK( CopyPonParams(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );
  
    /* 拷贝RTT 配置*/
    OLT_COPY_CHECK( CopyPonRange(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );

    /* 拷贝BER/FER告警配置*/
    OLT_COPY_CHECK( CopyOltBerAlarm(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );
    OLT_COPY_CHECK( CopyOltFerAlarm(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );

    /* 拷贝倒换参数配置*/
    OLT_COPY_CHECK( CopyOltSwapParam(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );

    /* 拷贝Pon口最大onu注册数目配置*/
    OLT_COPY_CHECK( CopyMaxOnuConfig(DstPonPortIdx, SrcPonPortIdx, CopyFlags) );
    
    return 0;
}
#undef OLT_COPY_CHECK

int CopyOneOnu(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iRlt;
    int iSrcOnuEntry;
    DeviceInfo_S *pDevInfo;
    short int ulDstSlot = GetCardIdxByPonChip(DstPonPortIdx);
    OLT_LOCAL_ASSERT(SrcPonPortIdx);
    OLT_ASSERT(DstPonPortIdx);
    ONU_ASSERT(DstOnuIdx);
    ONU_ASSERT(SrcOnuIdx);

    iSrcOnuEntry = SrcPonPortIdx * MAXONUPERPON + SrcOnuIdx;
    pDevInfo = &OnuMgmtTable[iSrcOnuEntry].DeviceInfo;
    
    if(SYS_MODULE_IS_GPON(ulDstSlot))
    {
        /* 1. 拷贝MAC地址,确认配置对象 */
        iRlt = OnuMgt_AddGponOnuByManual(DstPonPortIdx, DstOnuIdx, pDevInfo->DeviceSerial_No);
        if ( OLT_CALL_ISOK(iRlt) )
        {
            /* 2. 拷贝LLID */
            if ( CopyFlags & OLT_COPYFLAGS_WITHLLID )
            {
                int llid;

            llid = CopyOnuLlid(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags);
#if ( ONU_SYNC_IF == RPU_YES )
            if ( 0 < llid )
            {
                int iRet;
                int chip_type, remote_type;
            
                /* 底层LLID拷贝成功，意味着目标ONU可以挂接实接口，来同步配置到硬件底层 */
                if ( 0 == (iRet = OnuMgt_GetIFType(DstPonPortIdx, DstOnuIdx, &chip_type, &remote_type)) )
                {
                    if ( (PONCHIP_UNKNOWN == chip_type)
                        || (ONU_MANAGE_UNKNOWN == remote_type) )
                    {
                        /* 已经挂接实接口，则无需再挂接 */
                        iRet = OnuMgt_SetIFType(DstPonPortIdx, DstOnuIdx, PONCHIP_UNKNOWN, ONU_MANAGE_UNKNOWN);
                    }
                }
                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"CopyOnuIF(%d, %d, %d, %d)'s result(%d).\r\n", DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, iRet );
            }
#endif
            OLT_SYNC_DEBUG(OLT_SYNC_TITLE"CopyOnuLlid(%d, %d, %d, %d) with llid[%d].\r\n", DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, llid );
        }

        /* 3.---拷贝ONU信息--- */
#if 0
        if ( CopyFlags & OLT_COPYFLAGS_WITHINFO )
        {
            /* 拷贝名字信息*/
            OnuMgt_SetOnuDeviceName(DstPonPortIdx, DstOnuIdx, pDevInfo->DeviceName, pDevInfo->DeviceNameLen);
            
            /* 拷贝描述信息*/
            OnuMgt_SetOnuDeviceDesc(DstPonPortIdx, DstOnuIdx, pDevInfo->DeviceDesc, pDevInfo->DeviceDescLen);
            
            /* 拷贝位置信息*/
            OnuMgt_SetOnuDeviceLocation(DstPonPortIdx, DstOnuIdx, pDevInfo->Location, pDevInfo->LocationLen);

            /* 拷贝认证序列信息*/
        }
#else
		OnuMgt_SetOnuDeviceName(DstPonPortIdx, DstOnuIdx, pDevInfo->DeviceName, pDevInfo->DeviceNameLen);
#endif
            
        /* 4. ---拷贝ONU配置--- */
        iRlt = CopyOnuConfig(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags);

            /* 5. ---拷贝ONU状态--- */
            if ( CopyFlags & OLT_COPYFLAGS_WITHSTATUS )
            {
                /* 5.1 ---拷贝ONU状态(如: rtt等值)--- */

            }
            else
            {
                if ( CopyFlags & OLT_COPYFLAGS_GETSTATUS )
                {
                    /* 5.2 ---获得ONU自身状态(如: rtt等值)--- */
                }
            }
        }
    }
    else
    {
        /* 1. 拷贝MAC地址,确认配置对象 */
        iRlt = OnuMgt_AddOnuByManual(DstPonPortIdx, DstOnuIdx, pDevInfo->MacAddr);
        if ( OLT_CALL_ISOK(iRlt) )
        {
            /* 2. 拷贝LLID */
            if ( CopyFlags & OLT_COPYFLAGS_WITHLLID )
            {
                int llid;

                llid = CopyOnuLlid(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags);
#if ( ONU_SYNC_IF == RPU_YES )
                if ( 0 < llid )
                {
                    int iRet;
                    int chip_type, remote_type;
                
                    /* 底层LLID拷贝成功，意味着目标ONU可以挂接实接口，来同步配置到硬件底层 */
                    if ( 0 == (iRet = OnuMgt_GetIFType(DstPonPortIdx, DstOnuIdx, &chip_type, &remote_type)) )
                    {
                        if ( (PONCHIP_UNKNOWN == chip_type)
                            || (ONU_MANAGE_UNKNOWN == remote_type) )
                        {
                            /* 已经挂接实接口，则无需再挂接 */
                            iRet = OnuMgt_SetIFType(DstPonPortIdx, DstOnuIdx, PONCHIP_UNKNOWN, ONU_MANAGE_UNKNOWN);
                        }
                    }
                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"CopyOnuIF(%d, %d, %d, %d)'s result(%d).\r\n", DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, iRet );
                }
#endif
                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"CopyOnuLlid(%d, %d, %d, %d) with llid[%d].\r\n", DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, llid );
            }

            /* 3.---拷贝ONU信息--- */
#if 1
            if ( CopyFlags & OLT_COPYFLAGS_WITHINFO )
            {
                /* 拷贝名字信息*/
                OnuMgt_SetOnuDeviceName(DstPonPortIdx, DstOnuIdx, pDevInfo->DeviceName, pDevInfo->DeviceNameLen);
                
                /* 拷贝描述信息*/
                //OnuMgt_SetOnuDeviceDesc(DstPonPortIdx, DstOnuIdx, pDevInfo->DeviceDesc, pDevInfo->DeviceDescLen);
                
                /* 拷贝位置信息*/
                //OnuMgt_SetOnuDeviceLocation(DstPonPortIdx, DstOnuIdx, pDevInfo->Location, pDevInfo->LocationLen);

                /* 拷贝认证序列信息*/
            }
#endif
                
            /* 4. ---拷贝ONU配置--- */
            iRlt = CopyOnuConfig(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags);

            /* 5. ---拷贝ONU状态--- */
            if ( CopyFlags & OLT_COPYFLAGS_WITHSTATUS )
            {
                /* 5.1 ---拷贝ONU状态(如: rtt等值)--- */

            }
            else
            {
                if ( CopyFlags & OLT_COPYFLAGS_GETSTATUS )
                {
                    /* 5.2 ---获得ONU自身状态(如: rtt等值)--- */
                }
            }
        }
    }
    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"CopyOneOnu(%d, %d, %d, %d)'s result(%d).\r\n", DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, iRlt);

    return iRlt;
}

int CopyOnu(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iRlt;

    OLT_LOCAL_ASSERT(SrcPonPortIdx);
    OLT_ASSERT(DstPonPortIdx);
    
    /* ONU需要拷贝配置、信息，并从底层同步状态 */
    if ( SrcOnuIdx >= 0 )
    {
        /* 同步两个PON口下的某一ONU */
        ONU_ASSERT(SrcOnuIdx);
        ONU_ASSERT(DstOnuIdx);
        
        iRlt = CopyOneOnu(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags);
    }
    else
    {
        /* 同步两个PON口下的所有ONU */
        int i;
        int iCopyNum, iErrNum;

        iRlt = 0;
        iCopyNum = 0;
        iErrNum = 0;
        for (i=0; i<MAXONUPERPON; i++)
        {
            if( ROK != ThisIsValidOnu(SrcPonPortIdx, i) ) continue;
            iCopyNum++;
            iRlt = CopyOneOnu(DstPonPortIdx, i, SrcPonPortIdx, i, CopyFlags);
            if ( ROK != iRlt )
            {
                iErrNum++;
                VOS_SysLog(LOG_TYPE_ONU, LOG_ERR, "CopyOnu(%d, %d, %d, %d, %d) Error Rlt(%d) on slot(%d)", DstPonPortIdx, i, SrcPonPortIdx, i, CopyFlags, iRlt, SYS_LOCAL_MODULE_SLOTNO);
            }
        }

        if ( iCopyNum > iErrNum )
        {
            iRlt = 0;
        }
    }

    return iRlt;
}

int MoveOnu(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    /*int iRlt;*/

    OLT_LOCAL_ASSERT(SrcPonPortIdx);
    ONU_ASSERT(SrcOnuIdx);
    OLT_ASSERT(DstPonPortIdx);
    ONU_ASSERT(DstOnuIdx);

    /* 腾空目标ONU记录 */
    if ( DstOnuIdx < 0 )
    {
        OLT_ClearAllOnus(DstPonPortIdx);
    }
    else
    {
        OnuMgt_DelOnuByManual(DstPonPortIdx, DstOnuIdx);
    }
    
    /* ONU拷贝 */
    CopyOnu(DstPonPortIdx, DstOnuIdx, SrcPonPortIdx, SrcOnuIdx, CopyFlags);

    /* 清空源ONU记录 */
    if ( SrcOnuIdx < 0 )
    {
        OLT_ClearAllOnus(SrcPonPortIdx);
    }
    else
    {
        OnuMgt_DelOnuByManual(SrcPonPortIdx, SrcOnuIdx);
    }

    return 0;
}

int CopyOlt(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    int iRlt;

    OLT_LOCAL_ASSERT(SrcPonPortIdx);
    OLT_ASSERT(DstPonPortIdx);

#if 0
    if ( OLT_ISREMOTE(DstPonPortIdx) )
    {
        /* 不同设备间的OLT，需要同步全局配置 */
        if ( SYS_MODULE_IS_REMOTE(OLT_SLOT_ID(DstPonPortIdx)) )
        {
        	/* 拷贝OLT全局设置*/
            iRlt = ResumeOltGlobalConfig(DstPonPortIdx);
            OLT_SYNC_DEBUG(OLT_SYNC_TITLE"ResumeOltGlobalConfig(%d)'s result(%d).\r\n", DstPonPortIdx, iRlt);
        }
    }
#endif

    /* OLT只需拷贝配置 */
    iRlt = CopyOltConfig(DstPonPortIdx, SrcPonPortIdx, CopyFlags);
    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"CopyOltConfig(%d, %d)'s result(%d).\r\n", DstPonPortIdx, SrcPonPortIdx, iRlt);

    return iRlt;
}
#endif

#if 1
int ResumeOnuConfig(short int PonPortIdx)
{
    int i;
    int OnuEntry;

    for (OnuEntry = PonPortIdx * MAXONUPERPON, i=0;
        i<MAXONUPERPON;
        i++, OnuEntry++)
    {
    	if( ROK != ThisIsValidOnu(PonPortIdx, i) ) continue;

        /* 拷贝恢复设置 */
        if ( 0 == CopyOnu(PonPortIdx, i, PonPortIdx, i, OLT_COPYFLAGS_RESUME|OLT_COPYFLAGS_WITHINFO) ) /*增加ONU devicename信息的恢复, add by @liuyh,2017-06-01*/
        {
        	/* 配置ONU 语音业务*/
#if 0
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
        	{
            	unsigned long OnuDevIdx;
            	bool SupportVoice = FALSE;
            	OnuDevIdx = slot*10000 + port*1000 + (i+1);

            	if(get_gfa_sg_slotno())
            	{
            		if((OnuIsSupportVoice(OnuDevIdx, &SupportVoice) == ROK ) && ( SupportVoice == TRUE ))
            			ConfigOnuVoiceService(OnuDevIdx);
            	}
            	else if(get_gfa_e1_slotno())
            	{
            		if(onuDevIdxIsSupportE1(OnuDevIdx) == ROK )
            			RestoreOnuE1LinkAll(OnuDevIdx);
            	}
        	}
#endif
#endif
        }
    }

    return 0;
}

extern ULONG  g_ulIgmp_Auth_Enabled ;
int ResumeOltGlobalConfig(short int PonPortIdx)
{
    /* 恢复静态MAC地址表 */
    ResumeOLTStaticMacAddrTbl(PonPortIdx, 0);

    /* 恢复超长帧设置*/
    OLT_SetPonFrameSizeLimit(PonPortIdx, PON_Jumbo_frame_length);
        
    /* 设置加密模式*/
    if( V2R1_CTC_STACK )
        SetPonEncryptMode( PonPortIdx, PON_ENCRYPTION_TYPE_TRIPLE_CHURNING );
    else 
        SetPonEncryptMode( PonPortIdx, PON_ENCRYPTION_TYPE_AES);

    /*设置PON 端口DBA 模式*/
    /*SetPonDBAReportMode(PonPortIdx, PON_DBA_THRESHOLD_REPORT );*/

    /* added by chenfj 2007-6-27  
    		PON 端口管理使能: up / down  
          if( GetPonPortAdminStatus( PonPortIdx ) != V2R1_DISABLE )
    	    PONTx_Enable( PonPortIdx );
    	*/

	/* 恢复ONU下行QoS映射设置*/
    ResumeOnuDownlinkQosMapping(PonPortIdx);
    /* 恢复ONU MAC 地址确认模式*/
	{
        /*unsigned long enable;
		getOnuAuthEnable(0, 0, &enable);
		if( enable == V2R1_ONU_AUTHENTICATION_NEW_ONLY || enable == V2R1_ONU_AUTHENTICATION_ALL)
		{
			OLT_SetOnuAuthMode( PonPortIdx, TRUE );
		}
		else*/
             {
			OLT_SetOnuAuthMode( PonPortIdx, FALSE );
		}
	}

	/* 恢复OLT的IGMP认证模式*/
    OLT_SetIgmpAuthMode(PonPortIdx, g_ulIgmp_Auth_Enabled - 2);

#if(EPON_MODULE_DOCSIS_MANAGE==EPON_MODULE_YES)
    /* 恢复OLT的CMC设置 */
    ResumeCmcSVlanID(PonPortIdx);
#endif

    return 0;
}

int ResumeOltConfig(short int PonPortIdx)
{
    int iRlt;
    
    /* 恢复OLT全局设置*/
    ResumeOltGlobalConfig(PonPortIdx);

    /* 拷贝自己的内存配置，实现硬件配置恢复 */
    if ( 0 == (iRlt = CopyOltConfig(PonPortIdx, PonPortIdx, OLT_COPYFLAGS_RESUME)) )
    {
#if 0
        if ( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
#else
	short int slot = GetCardIdxByPonChip(PonPortIdx);
	if(slot !=  SYS_LOCAL_MODULE_SLOTNO &&  SYS_MODULE_SLOT_ISHAVECPU(slot))
#endif
        {
            /* 恢复ONU配置 */
            ResumeOnuConfig(PonPortIdx);
        }
    }

    return iRlt;
}
int CopyPonPortAuthEnable(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    /*added by luh 2012-2-10*/
    unsigned long src_enable, gsrc_enable;
    unsigned long dst_enable;
    ULONG ulSlot, ulPort;
    int iRlt = 0;
    ulSlot = GetCardIdxByPonChip(SrcPonPortIdx);
    ulPort = GetPonPortByPonChip(SrcPonPortIdx);
    if (getOnuAuthEnable(ulSlot, ulPort, &src_enable) != VOS_OK)
    {
        return VOS_ERROR;
    }
    if (getOnuAuthEnable(0, 0, &gsrc_enable) != VOS_OK)
    {
        return VOS_ERROR;
    }
    /*if (PonPortIsWorking(PonPortIdx) != TRUE)
        return VOS_ERROR;*/
    if ( SrcPonPortIdx == DstPonPortIdx )
    {
        /* 自拷贝，应该是配置恢复 */
        if (gsrc_enable != V2R1_ONU_AUTHENTICATION_DISABLE)
            OLT_SetAllOnuAuthMode2(SrcPonPortIdx, gsrc_enable);
        else
            OLT_SetAllOnuAuthMode2(SrcPonPortIdx, src_enable);
    }
    else
    {
        /*全局使能无需要进行copy*/
        if (gsrc_enable != V2R1_ONU_AUTHENTICATION_DISABLE)
            return VOS_OK;
        
        ulSlot = GetCardIdxByPonChip(DstPonPortIdx);
        ulPort = GetPonPortByPonChip(DstPonPortIdx);
        if( getOnuAuthEnable(ulSlot, ulPort, &dst_enable) == VOS_OK)
        {
            if ( OLT_COPYFLAGS_COVER & CopyFlags || OLT_COPYFLAGS_ONLYNEW & CopyFlags)
            {
                iRlt = OLT_SetAllOnuAuthMode2(DstPonPortIdx, src_enable);
            }
            else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
            {
                if ( OLT_ISLOCAL(DstPonPortIdx) )
                {
                    if ( dst_enable != src_enable )
                    {
                        iRlt = OLT_SetAllOnuAuthMode2(DstPonPortIdx, src_enable);
                    }
                }
                else
                {
                    iRlt = OLT_SetAllOnuAuthMode2(DstPonPortIdx, src_enable);
                }
            }
        }
    }
    return VOS_OK;
}

int ResumeOltCardConfig(short int CardIndex)
{
    int iRlt;
    short int PonPortIdx = GetPonPortIdxBySlot( CardIndex, FIRSTPONPORTPERCARD );

    if ( PonPortIdx >= 0 )
    {
        if ( 0 == pon_remote_olt(PonPortIdx) )
        {
            OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) get the PON-Chip(%d/%d)'s RPC-PON service at the ResumeOltCardConfig().\r\n", SYS_LOCAL_MODULE_SLOTNO, CardIndex, FIRSTPONPORTPERCARD);
        }

        if ( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
        {
        	resumeOnuRepeatedDelEnable();

            /* 恢复全局PAS_SOFT主机超时设置 */
            {
        		short int Times;

        		if( Pon_Get_System_parameters(NULL,NULL,NULL, &Times) == ROK )
        		{
        			if( Times != V2R1_PON_HOST_MSG_OUT )
                        OLT_SetSystemParams(PonPortIdx, -1, -1, -1, Times);
                }
            }

		 /* 恢复全局Onu 端口统计 超时设置 */
        {
    		ULONG TimeOut = 0;

    		if( Onustats_GetPortStatsTimeOut(ONU_GETDATA_TIMER,&TimeOut) == ROK )
    		{
    			if( TimeOut != ONU_GETDATA_TIMER_INTERVAL)
					OLT_SetCTCOnuPortStatsTimeOut(PonPortIdx,ONU_GETDATA_TIMER,TimeOut);
            }
       
    		if( Onustats_GetPortStatsTimeOut(ONU_WAKEUP_TIMER,&TimeOut) == ROK )
    		{
    			if( TimeOut != ONU_WAKEUP_TIMER_INTERVAL)
                   OLT_SetCTCOnuPortStatsTimeOut(PonPortIdx,ONU_WAKEUP_TIMER,TimeOut);
            }
			
			if( Onustats_GetPortStatsTimeOut(ONU_PORTSTATS_TASKSTATUS,&TimeOut) == ROK )
    		{
    			if( TimeOut != 0)
                   OLT_SetCTCOnuPortStatsTimeOut(PonPortIdx,ONU_PORTSTATS_TASKSTATUS,TimeOut);
            }

			if( Onustats_GetPortStatsTimeOut(ONU_PORTSTATS_ENABLE,&TimeOut) == ROK )
    		{
    			if( TimeOut != 0)
                   OLT_SetCTCOnuPortStatsTimeOut(PonPortIdx,ONU_PORTSTATS_ENABLE,TimeOut);
            }
       
        }
            /* 恢复全局静态MAC设置 */
            ResumeOLTStaticMacAddrTbl(PonPortIdx, 1);
#if 0    
            /* 恢复全局ONU认证使能设置 */
            {
        		unsigned long enable, ponid;
                int liv_port = 0;
                ULONG old_mode = 0;
                {
                    for( liv_port=1; liv_port<=PONPORTPERCARD; liv_port++ )
                    {
                        if(userport_is_pon(CardIndex, liv_port) != VOS_YES)
                            continue;
                        ponid = GetPonPortIdxBySlot(CardIndex, liv_port);
                        if( ponid == RERROR )
                            continue;
                        getOnuAuthEnable(CardIndex, liv_port, &old_mode);
                        if(old_mode != V2R1_ONU_AUTHENTICATION_DISABLE_FOR_SINGLE_PON)
                        {
                            sys_console_printf("ResumeOltCardConfig(%d),port = %d mode = %d\r\n",CardIndex, liv_port, old_mode);
                            OLT_SetAllOnuAuthMode2(ponid, old_mode);
                        }
                    }
                }
            }
#endif
            /* 恢复全局ONU绑定使能设置 */
        	if(PONid_ONUMAC_BINDING == V2R1_ENABLE)
                OLT_SetAllOnuBindMode(PonPortIdx, V2R1_DISABLE);

            /* 恢复全局ONU默认带宽设置 */
			ONU_bw_t default_bw;
			VOS_MemZero(&default_bw, sizeof(ONU_bw_t));
                
            if((OnuConfigDefault.UplinkBandwidth != ONU_DEFAULT_BW) ||
            (OnuConfigDefault.UplinkBandwidthBe != ONU_DEFAULT_BE_BW) ||   
            (OnuConfigDefault.DownlinkBandwidth!= ONU_DEFAULT_BW) ||
            (OnuConfigDefault.DownlinkBandwidthBe != ONU_DEFAULT_BE_BW))  
            {
                default_bw.bw_direction = OLT_CFG_DIR_BOTH;
                default_bw.bw_gr = OnuConfigDefault.UplinkBandwidth;
                default_bw.bw_be = OnuConfigDefault.UplinkBandwidthBe;
                default_bw.bw_fixed   = OnuConfigDefault.DownlinkBandwidth;
                default_bw.bw_actived = OnuConfigDefault.DownlinkBandwidthBe;
				default_bw.bw_rate = PON_RATE_NORMAL_1G;
                OLT_SetAllOnuDefaultBW(PonPortIdx, &default_bw);
            }

			if((OnuConfigDefault.UplinkBandwidth_XGEPON_ASYM != ONU_DEFAULT_BW) ||
            (OnuConfigDefault.UplinkBandwidthBe_XGEPON_ASYM != ONU_DEFAULT_BE_BW) ||   
            (OnuConfigDefault.DownlinkBandwidth_XGEPON_ASYM != GW10G_ONU_DEFAULT_BW) ||
            (OnuConfigDefault.DownlinkBandwidthBe_XGEPON_ASYM != GW10G_ONU_DEFAULT_BE_BW))  
            {
                default_bw.bw_direction = OLT_CFG_DIR_BOTH;
                default_bw.bw_gr = OnuConfigDefault.UplinkBandwidth_XGEPON_ASYM;
                default_bw.bw_be = OnuConfigDefault.UplinkBandwidthBe_XGEPON_ASYM;
                default_bw.bw_fixed   = OnuConfigDefault.DownlinkBandwidth_XGEPON_ASYM;
                default_bw.bw_actived = OnuConfigDefault.DownlinkBandwidthBe_XGEPON_ASYM;
				default_bw.bw_rate = PON_RATE_1_10G;
                OLT_SetAllOnuDefaultBW(PonPortIdx, &default_bw);
            }

			if((OnuConfigDefault.UplinkBandwidth_XGEPON_SYM != GW10G_ONU_DEFAULT_BW) ||
            (OnuConfigDefault.UplinkBandwidthBe_XGEPON_SYM != GW10G_ONU_DEFAULT_BE_BW) ||   
            (OnuConfigDefault.DownlinkBandwidth_XGEPON_SYM != GW10G_ONU_DEFAULT_BW) ||
            (OnuConfigDefault.DownlinkBandwidthBe_XGEPON_SYM != GW10G_ONU_DEFAULT_BE_BW))  
            {
                default_bw.bw_direction = OLT_CFG_DIR_BOTH;
                default_bw.bw_gr = OnuConfigDefault.UplinkBandwidth_XGEPON_SYM;
                default_bw.bw_be = OnuConfigDefault.UplinkBandwidthBe_XGEPON_SYM;
                default_bw.bw_fixed   = OnuConfigDefault.DownlinkBandwidth_XGEPON_SYM;
                default_bw.bw_actived = OnuConfigDefault.DownlinkBandwidthBe_XGEPON_SYM;
				default_bw.bw_rate = PON_RATE_10_10G;
                OLT_SetAllOnuDefaultBW(PonPortIdx, &default_bw);
            }

			if((OnuConfigDefault.UplinkBandwidth_GPON != ONU_DEFAULT_BW) ||
            (OnuConfigDefault.UplinkBandwidthBe_GPON != ONU_DEFAULT_BE_BW) ||   
            (OnuConfigDefault.DownlinkBandwidth_GPON != GPON_ONU_DEFAULT_BW) ||
            (OnuConfigDefault.DownlinkBandwidthBe_GPON != GPON_ONU_DEFAULT_BE_BW))  
            {
                default_bw.bw_direction = OLT_CFG_DIR_BOTH;
                default_bw.bw_gr = OnuConfigDefault.UplinkBandwidth_GPON;
                default_bw.bw_be = OnuConfigDefault.UplinkBandwidthBe_GPON;
                default_bw.bw_fixed   = OnuConfigDefault.DownlinkBandwidth_GPON;
                default_bw.bw_actived = OnuConfigDefault.DownlinkBandwidthBe_GPON;
				default_bw.bw_rate = PON_RATE_1_2G;
                OLT_SetAllOnuDefaultBW(PonPortIdx, &default_bw);
            }

            if(MaxMACDefault != ONU_DEFAULT_MAX_MAC)
            {
                OLT_SetOnuDefaultMaxMac(PonPortIdx, MaxMACDefault); 
            }
            
            if(MaxOnuDefault != DAFAULT_MAX_ONU)
            {
                OLT_SetMaxOnu(PonPortIdx, MaxOnuDefault|CONFIG_DEFAULT_MAX_ONU_FLAG|CONFIG_CODE_ALL_PORT);
            }

            /* 恢复全局ONU下行限速设置 */
        	{
        	    /*设置downlink policer*/
        		int def_flag;
        		int run_flag;
                
        		def_flag = GetOnuPolicerFlagDefault();
        		run_flag = GetOnuPolicerFlag();
        		if ( def_flag != run_flag )
        		{
        		    OLT_SetAllOnuDownlinkPoliceMode(PonPortIdx, -run_flag);
        		}     
        	}
        	{
                /*设置downlink policer param*/
            	int iBurst;
            	int iPre;
            	int iWeight;
                
                GetOnuPolicerParam( &iBurst, &iPre, &iWeight );

                if ( (iBurst != ONU_DOWNLINK_POLICE_BURSESIZE_DEFAULT)
                    || (iPre != DISABLE)
                    || (iWeight != ONU_DOWNLINK_POLICE_WEIGHT_DEFAULT) )
        		{			
        			OLT_SetAllOnuDownlinkPoliceParam(PonPortIdx, iBurst, iPre, iWeight);
        		}     
        	}
            {/*设置uplink dba param*/
            	int iPktSize;
            	int iBurst;
            	int iWeight;
                
                GetOnuDbaParam( &iPktSize, &iBurst, &iWeight );
                
        		if ( (iPktSize != ONU_UPLINK_DBA_PACKETSIZE_DEFAULT)
                    || (iBurst != ONU_UPLINK_POLICE_BURSESIZE_DEFAULT)
                    || (iWeight != ONU_UPLINK_DBA_WEIGHT_DEFAULT) )
            	{			
            	    OLT_SetAllOnuUplinkDBAParam(PonPortIdx, iPktSize, iBurst, iWeight);
            	}     
            }
            
            /* 恢复全局PON慢倒换参数设置 */
            {
            	if( (V2R1_AutoProtect_Timer != V2R1_PON_PORT_SWAP_TIMER)
                    || (V2R1_AutoProtect_Trigger != V2R1_PON_PORT_SWAP_TRIGGER)
                    || (0 != g_ulRPCCall_mode)
                    || (V2R1_ENABLE != pon_swap_switch_enable) )
                    OLT_SetHotSwapParam(PonPortIdx, pon_swap_switch_enable, V2R1_AutoProtect_Timer, g_ulRPCCall_mode, V2R1_AutoProtect_Trigger, -1, -1);           
            }
            
            /* 恢复全局PON告警参数设置 */
            {
            	unsigned int threshold, Num;

            	monponBERThrGet(&threshold, &Num );
            	if(( threshold != DEFAULT_PON_BER_THRESHOLD_RATIO )
                    || ( Num != DEFAULT_PON_BER_THRESHOLD_MINBYTE))
                {
                    OLT_SetBerAlarmParams(PonPortIdx, threshold, Num);           
                }   

            	monponFERThrGet(&threshold, &Num );
            	if(( threshold != DEFAULT_PON_FER_THRESHOLD_RATIO )
                    || ( Num != DEFAULT_PON_FER_THRESHOLD_MINFRAME))
                {
                    OLT_SetFerAlarmParams(PonPortIdx, threshold, Num);           
                }   
            }

#ifdef ONU_PPPOE_RELAY
            ResumePPPoEDelayConfig(PonPortIdx);
#endif

#ifdef ONU_DHCP_RELAY
            ResumeDhcpDelayConfig(PonPortIdx);
#endif

/* modified by xieshl 20120813, 问题单15520 */
#if(EPON_MODULE_ONU_MAC_OVERFLOW_CHECK==EPON_MODULE_YES)
	resumeOnuMacCheckConfig( PonPortIdx );
#endif

#if(EPON_MODULE_DOCSIS_MANAGE==EPON_MODULE_YES)
            /* 恢复全局CMC设置 */
            {
                int iVlanId;
            
                GetCmcSVlanID(&iVlanId);
                if ( CMC_CFG_SVLAN_ID_DEFAULT != iVlanId )
                {
                    SetCmcSVlanID(PonPortIdx, iVlanId);
                }
            }
#endif

            iRlt = 0;
        }
    }
    else
    {
        iRlt = -1;
    }
       
    return iRlt;
}

int ResumeOltStatus(short int PonPortIdx)
{
    return OLT_ResumeAllOnuStatus(PonPortIdx, OLT_RESUMEREASON_SYNC, OLT_RESUMEMODE_SYNCSOFT);
}
#endif

void printOnuList(short int PonPortIdx)
{
	int OnuIdx, status, OnuEntry;
	sys_console_printf("\r\n pon%d/%d onulist:\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		{
			sys_console_printf("%2d  ", (OnuIdx+1));
			sys_console_printf("  %02x%02x.%02x%02x.%02x%02x  ", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
					OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
					OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] );
			status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
			if( (status < 0) || (status > 5) ) status = 0;
			sys_console_printf(" %s ", OnuCurrentStatus[status]);

			sys_console_printf("  \r\n"); 
		}
	}

}

/*****************************************************
 *
 *    Function:  ActivePonPort( short int PonPortIdx )
 *
 *    Param:    short int PonPortIdx -- the specific pon port
 *                 
 *    Desc:   at this time, the pon firmware should be loaded and running, and the DBA should 
 *               be load already also. 
 *               only when this function is called, the ONU can be registered 
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int ActivePonPort( short int PonPortIdx )
{
    int return_code = 0;
    short int PonChipType, PonChipVer = RERROR;
    int CardSlot, CardPort;
    int PonIsLocalActive = 0;
    
	CHECK_PON_RANGE

	if(( PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_UP ) && ( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UP ))
		return( ROK );

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	if( OLT_PONCHIP_ISPAS(PonChipType) ) 
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
	}
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_TK;
    }
	#if defined(_GPON_BCM_SUPPORT_)
	else if ( OLT_PONCHIP_ISGPON(PonChipType) )
    {
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_GPON;
    }
	#endif
	
	if( PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_DORMANT )
	{
		CardSlot = GetCardIdxByPonChip(PonPortIdx);
		CardPort = GetPonPortByPonChip(PonPortIdx);

		if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && ((CardSlot == SYS_LOCAL_MODULE_SLOTNO) || (!SYS_MODULE_SLOT_ISHAVECPU(CardSlot))) )
		{
			PonIsLocalActive = 1;

			/* PON卡直接管理者，安装OLT本地管理接口 */
			if ( 0 == pon_local_olt(PonPortIdx, PonChipVer) )
			{
				OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) get the PON-Chip(%d/%d)'s LOCAL-PON service at the PonPortActive time.\r\n", SYS_LOCAL_MODULE_SLOTNO, CardSlot, CardPort);
			}
		}
		else
		{
			PonIsLocalActive = 0;

			/* 非PON卡直接管理者，安装OLT远程管理接口 */
			if ( 0 == pon_remote_olt(PonPortIdx) )
			{
				OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) get the PON-Chip(%d/%d)'s RPC-PON service at the PonPortActive time.\r\n", SYS_LOCAL_MODULE_SLOTNO, CardSlot, CardPort);
			}
		}

		/*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
		Pon_InitDefaultMaxBW(PonPortIdx);

		if ( 0 == return_code )
		{
			/* ---获取芯片信息--- */
			if ( PONCHIP_UNKNOWN == PonChipMgmtTable[PonPortIdx].version )
			{
				GetPonChipInfo(CardSlot, CardPort);
			}

			/* ---向上恢复状态信息--- */
			GetPonDeviceVersion(PonPortIdx);
			GetPonDeviceCapabilities(PonPortIdx);

			if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
			{
				/* ---向下恢复OLT配置--- */
				ResumeOltConfig(PonPortIdx);

				/* ---通知底层向上恢复ONU注册信息--- */
#if 0
				if( devsm_sys_is_switchhovering() )
#endif
					/* 倒换标志消失过快，造成无法优化 */
				{
					ResumeOltStatus(PonPortIdx);
				}
			}

#if 1
			if ( PonIsLocalActive )
			{
				/* 开始启动业务端口, 允许注册 */
				if( PonChipType == PONCHIP_PAS )
				{
					/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
					return_code = Pon_StartDbaAlgorithm( PonPortIdx, PonChipVer, FALSE, 0, NULL);

					if ( return_code == PAS_EXIT_OK )
					{
					}
					else if ( return_code == PAS_DBA_ALREADY_RUNNING )
					{
						/*** modified by chenfj 2009-1-7
						  返回值是PAS_DBA_ALREADY_RUNNING 的这样情况从逻辑推断上应该不会出现;
						  但为了程序的冗余判断, 也加上了
						  */
						return_code = 0;
					}
					else
					{
						/* 产生不匹配告警*/
						FlashAlarm_PonPortVersionMismatch(PonPortIdx, PON_OLT_BINARY_DBA_DLL, V2R1_ENABLE);
						/*ShutdownPonPort(PonPortIdx);*/
						PONTx_Disable( PonPortIdx, PONPORT_TX_VERCHK );
						return_code = PAS_DEVICE_VERSION_MISMATCH_ERROR;
					}
				}
				else if( PonChipType == PONCHIP_TK )
				{
					if ( OLT_PONCHIP_ISBCM(PonChipVer) )
					{
						BcmOLT_DisablePonPort (PonPortIdx);                 
						BcmOLT_SetFecMode( PonPortIdx, 2, 0, 0 );         /* 默认关闭1G通道FEC */
						/*--modified by wangjiah@2016-08-05 begin--*/
						//BcmOLT_SetFecMode( PonPortIdx, 1, 0, 0 );         /* 默认关闭10-1G非对称通道FEC */
						//BcmOLT_SetFecMode( PonPortIdx, 0, 0, 0 );         /* 默认关闭10-10G对称通道FEC */
						BcmOLT_SetFecMode( PonPortIdx, 1, 1, 1 );         /* 开启10-1G非对称通道FEC */
						BcmOLT_SetFecMode( PonPortIdx, 0, 1, 1 );         /* 开启10-10G对称通道FEC */
						/*--modified by wangjiah@2016-08-05 end--*/
						/*BcmOLT_EnablePonPort (PonPortIdx);*/	/* modified by xieshl 20160523, 解决ONU提前注册问题，开启数据通道时会自动使能 */

						return_code = BcmOltOpenDataPath(PonPortIdx);
					}
					/*del by luh 2015-06-18*/
#if 0					
					else
					{
						return_code = TkOltOpenDataPath(PonPortIdx);
					}
#endif					
					/* 使能OLT */
					if ( 0 != return_code )
					{
						/* 产生不匹配告警*/
						FlashAlarm_PonPortVersionMismatch(PonPortIdx, PON_OLT_BINARY_FIRMWARE, V2R1_ENABLE);

						return_code = PAS_DEVICE_VERSION_MISMATCH_ERROR;
					}

				}
#if defined(_GPON_BCM_SUPPORT_)
				else if( PonChipType == PONCHIP_GPON )
				{
#if 0
					extern int GPONTxEnable(int olt_id, int tx_mode );
					GPONTxEnable(PonPortIdx,1);
#endif

				}
#endif
				else
				{
					return_code = PAS_EXIT_ERROR;
				}

				if ( 0 == return_code )
				{
					/*
					   modified by chenfj 2009-3-16
					   当监测到PON 口版本不匹配时，关闭该端口
					   */
					if(PonPortFirmwareAndDBAVersionMatch(PonPortIdx) == V2R1_ENABLE)
					{
						/*ShutdownPonPort(PonPortIdx);*/
						PONTx_Disable( PonPortIdx, PONPORT_TX_VERCHK );/* modified by xieshl 20120703, 改为关闭发光，防止PON口重启后管理状态无法恢复 */
						return_code = PAS_DEVICE_VERSION_MISMATCH_ERROR;
					}
				}

#if defined(_EPON_10G_PMC_SUPPORT_)            
				/*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
				if ( SYS_MODULE_IS_6900_10G_EPON(CardSlot) )
				{
					/*8411 10G EPON 光模块类型暂不处理*/
				}
				else
#endif
				{
					CheckSlotPonSFPTypeCallback( PonPortIdx, TRUE );	/* added by xieshl 20110705, PON板启动后先检查光模块类型是否匹配，问题单11899 */
				}
			}   
#endif
		}
	

		
		/* 非配置类的本地业务设置 */
		if ( PonIsLocalActive )
		{
			do{
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
				/* if((PonChipVer != PONCHIP_PAS5001))
				   AdcConfig_default(PonPortIdx);*/
				/*modified by yangzl@2016-6-8*/
				if((PonChipType == PONCHIP_TK)&& ( 0 != strncmp( PonChipMgmtTable[PonPortIdx].FirmwareVer,"V264" ,4) ))
					AdcConfig_BCM(PonPortIdx);
				else
					AdcConfig_default(PonPortIdx);
				/*AdcConfig_1(PonPortIdx,8,33,2,6,16,PON_POLARITY_POSITIVE,PON_POLARITY_NEGATIVE);*/
#endif
			}while(0);
#if 0
#if defined(_GPON_BCM_SUPPORT_)
			if( PonChipType == PONCHIP_GPON )
			{
				return_code = gponOltAdp_ActivePon(PonPortIdx);
			}
#endif
#endif
		}

		if ( 0 == return_code )
		{
			/* 更新PON口状态 */
			if ( PONCHIP_PAS == PonChipType )
			{
				PonPortTable[PonPortIdx].DBA_mode = OLT_EXTERNAL_DBA;
			}
			else
			{
				PonPortTable[PonPortIdx].DBA_mode = OLT_INTERNAL_DBA;
			}

			PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_UP;
			PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_UP;

			/* 更新PON相关业务状态 */
			UpdatePonPortNum();	
			UpdateProvisionedBWInfo(PonPortIdx);

			if ( PonIsLocalActive )
			{
				/* 设置ONU注册缺省带宽 */
				SetOnuRegisterDefaultUplinkBW(PonPortIdx);

				/* 激活PON相关业务 */
#ifdef 	STATISTIC_TASK_MODULE
				StatsMsgPonOnSend(PonPortIdx, PON_ONLINE);
#endif

				/* 激活PON告警业务 */
				SetPonAlarmConfig( PonPortIdx );

				/* B--added by liwei056@2011-11-25 for D13166 */
				/* 清除PON异常告警 */
				Trap_PonPortNormal( PonPortIdx );
				/* E--added by liwei056@2011-11-25 for D13166 */
			}

			/* B--added by liwei056@2011-2-15 for D12056 */
			if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
				/* E--added by liwei056@2011-2-15 for D12056 */
			{
				/* 主控点亮PON灯，标识主控打开了PON发光 */
				SetPonRunningFlag( PonPortIdx );

				/* 通知PON口可用 */
				OLTAdv_NotifyOltValid(PonPortIdx);


				/* added by chenfj 2007-6-25 设置PON端口保护倒换*/
				/*  问题单:#5376 
					问题单:#5373
					问题单:#5372
					modified by chenfj 2007-9-20
					先判断PON口是否使能了保护切换，之后再执行PONTx_Enable 
					*/			
				if( EnablePonPortAutoProtectByOnePort(PonPortIdx) != ROK )
				{
					OLT_SWITCH_DEBUG("\r\n enable pon(%d/%d) protect no OK!\r\n", CardSlot, CardPort);
					/* 打开业务端口*/
					if( GetPonPortAdminStatus( PonPortIdx ) != V2R1_DISABLE )
					{
						PONTx_Enable( PonPortIdx, PONPORT_TX_ALL );
        			}
					else/*38240,用于恢复配置,没有此代码的话，整机重启或重启pon板，shutdown 的配置不恢复，导致之前shutdown的端口undo shutdown不生效by jinhl@2017.05.24*/
					{
						PonPortAdminStatusDisable( PonPortIdx );
					}
				}
				/*Begin:for onu swap by jinhl@2013-04-27*/
				else
				{

					/*激活pon口时，若对端有onu在线，增加其
					  注册信息到本pon口*/
					OLT_SWITCH_DEBUG("\r\n enable pon(%d/%d) protect OK!\r\n", CardSlot, CardPort);
					(void)ActivePonPort_RegVirtual(PonPortIdx);
				}
			}
			/* B--added by liwei056@2010-11-18 for D11161 */
			else
			{
				/* 向上同步PON口激活状态 */
				/*if (PONCARD_ACTIVATED == PonCardStatus[SYS_LOCAL_MODULE_SLOTNO])
				  OLT_SYNC_Data(PonPortIdx, FC_PONPORT_ACTIVATED, NULL, 0);*//* removed by xieshl 20160520, 解决ONU提前注册问题，重复了*/

#if defined(_EPON_10G_PMC_SUPPORT_)            
				/*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
				Pon_SetTransparentMode(PonPortIdx, PonChipVer);
#endif




			}
			/*主控与PON板之间不是同步激活的，所以可能存在主控先于PON板而出问题by jinhl*/
			if( PonIsLocalActive )/*为了8100 移到此处by jinhl*/
			{
				if(PONCHIP_GPON == PonChipType)
				{
					/*Start*/
					gponOltAdp_StopSnAcq(PonPortIdx);
					gponOltAdp_StartSnAcq(PonPortIdx);
					/*gpon板卡启动时，若启动时间超时，会导致*/
					if(gpon_isStartExp() && (15 == PonPortIdx))
					{
						sys_console_printf("%s %d ------\r\n",__FUNCTION__,__LINE__);
						gpon_setStartExp(0);
						OltDevResetSync(0);

					}
				}
				/* modified by xieshl 2016061, 发光由主控和8XEP PON板同时控制，避免激光器被提前打开，同时解决pon link灯问题
				   主控上activePonPort由PON板通知，避免2个同时操作，导致ONU提前注册或不注册。
				   但修改后新的机制存在缺陷，即如果这个消息丢了，仍会导致ONU不能注册 */
				/*else if( OLT_PONCHIP_ISBCM(PonChipVer) )
				  {
				  PONTx_Enable( PonPortIdx, PONPORT_TX_ALL );
				  }*/
				OLT_SYNC_Data(PonPortIdx, FC_PONPORT_ACTIVATED, NULL, 0);
			}
			/* E--added by liwei056@2010-11-18 for D11161 */
			sys_console_printf("    pon%d/%d startup external DBA ok \r\n", CardSlot, CardPort );       
		}
		else
		{
			sys_console_printf("\r\n    pon%d/%d startup external DBA err %d \r\n", CardSlot, CardPort, return_code );
			PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_DOWN;
			PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ERR;

			if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
			{
				OLT_SYNC_Data(PonPortIdx, FC_DEL_PONPORT, NULL, 0);
			}

#if 0
			if ( PonIsLocalActive )
			{
				/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
				if(OLTAdv_IsExist(PonPortIdx) == TRUE )
					Pon_RemoveOlt( PonPortIdx, FALSE, FALSE);	
				ClearPonRunningData( PonPortIdx );
				Hardware_Reset_olt1(CardSlot, CardPort, 1, 0);
				Hardware_Reset_olt2(CardSlot, CardPort, 1, 0);
			}   
#endif

			return( RERROR );
		}

		PonChipDownloadCounter[PonPortIdx]=0;
		return( ROK );		
	}
	else
        return (RERROR );
}

int UpdatePonFirmware( short int PonPortIdx )
{

	CHECK_PON_RANGE

	RestartPonPort( PonPortIdx );
	
	return( ROK );
}

/*****************************************************
 *
 *    Function:  pon_start_default_dba(const short int PonPortIdx)
 *
 *    Param:   const short int PonPortIdx -- the specific pon port  
 *                 
 *    Desc:    start internal DBA algorithm
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int pon_start_default_dba(const short int PonPortIdx)
{
	short int result=PAS_EXIT_ERROR;
	short int  PonChipIdx;
	short int  PonChipType;

	CHECK_PON_RANGE

	PonChipIdx = GetPonChipIdx( PonPortIdx );
	PonChipType = V2R1_GetPonchipType( PonChipIdx );
		
	if( OLT_PONCHIP_ISPAS(PonChipType) ){
		/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		result = Pon_StartDbaAlgorithm( PonPortIdx, PonChipType, TRUE, 0, NULL);
		}
	else { /* other pon chip handler */
		}
	
	if(result !=PAS_EXIT_OK)
	{
		sys_console_printf("  Start %s/port%d internal dba algorithm failed!\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		return( RERROR );
	}
	return ( ROK );
}

int InitPonAlarmConfigDefault()
{
	/* ber alarm, set only to olt */
	/* modified for PAS-SOFT V5.1 */
	
	VOS_MemSet( &PonDefaultAlarmConfiguration, 0, sizeof(PON_olt_monitoring_configuration_t ));
	PonDefaultAlarmConfiguration.ber_alarm_active = TRUE;
	PonDefaultAlarmConfiguration.ber_alarm_configuration.ber_threshold = PON_MONITORING_BER_THRESHOLD_DEFAULT_VALUE;
	PonDefaultAlarmConfiguration.ber_alarm_configuration.direction = PON_DIRECTION_UPLINK_AND_DOWNLINK;
	PonDefaultAlarmConfiguration.ber_alarm_configuration.minimum_error_bytes_threshold = PON_MONITORING_MINIMUM_ERROR_BYTES_THRESHOLD_DEFAULT_VALUE;
	/*PonDefaultAlarmConfiguration.ber_alarm_configuration.TBD = 0;*/
	/* fer alarm, set only to olt */
	PonDefaultAlarmConfiguration.fer_alarm_active = TRUE;
	PonDefaultAlarmConfiguration.fer_alarm_configuration.fer_threshold = PON_MONITORING_FER_THRESHOLD_DEFAULT_VALUE;
	PonDefaultAlarmConfiguration.fer_alarm_configuration.direction = PON_DIRECTION_UPLINK_AND_DOWNLINK;
	PonDefaultAlarmConfiguration.fer_alarm_configuration.minimum_error_frames_threshold = PON_MONITORING_MINIMUM_ERROR_FRAMES_THRESHOLD_DEFAULT_VALUE;
	/*PonDefaultAlarmConfiguration.fer_alarm_configuration.TBD = 0;*/
	
	PonDefaultAlarmConfiguration.software_error_alarm_active = TRUE;  /* set to all active OLTs */
	PonDefaultAlarmConfiguration.local_link_fault_alarm_active = TRUE;  /*set only to olt */
	PonDefaultAlarmConfiguration.local_link_fault_alarm_configuration.nothing = 0;
	PonDefaultAlarmConfiguration.dying_gasp_alarm_active = TRUE;        /*set only to olt */
	PonDefaultAlarmConfiguration.dying_gasp_alarm_configuration.nothing = 0;
	PonDefaultAlarmConfiguration.critical_event_alarm_active = TRUE;   /*set only to olt */
	PonDefaultAlarmConfiguration.critical_event_alarm_configuration.nothing = 0;
	PonDefaultAlarmConfiguration.remote_stable_alarm_active = TRUE;  /*set only to olt */
	PonDefaultAlarmConfiguration.remote_stable_alarm_configuration.nothing =0;
	PonDefaultAlarmConfiguration.local_stable_alarm_active = TRUE;     /*set only to olt */
	PonDefaultAlarmConfiguration.local_stable_alarm_configuration.nothing = 0;
	PonDefaultAlarmConfiguration.oam_vendor_specific_alarm_active = TRUE;  /*set only to olt */
	PonDefaultAlarmConfiguration.oam_vendor_specific_alarm_configuration.nothing =0;
	/* set to Olt or to specific onu , supported only by OAM 2.0 or higher */
	PonDefaultAlarmConfiguration.errored_symbol_period_alarm_active = TRUE;
	PonDefaultAlarmConfiguration.errored_symbol_period_alarm_configuration.errored_symbol_threshold.msb = 0;
	PonDefaultAlarmConfiguration.errored_symbol_period_alarm_configuration.errored_symbol_threshold.lsb = PON_ALARM_ERRORED_SYMBOL_THRESHOLD_DEFAULT;
	PonDefaultAlarmConfiguration.errored_symbol_period_alarm_configuration.errored_symbol_window.msb = 0;
	PonDefaultAlarmConfiguration.errored_symbol_period_alarm_configuration.errored_symbol_window.lsb = PON_ALARM_ERRORED_SYMBOL_WINDOW_DEFAULT;
	/*PonDefaultAlarmConfiguration.errored_symbol_period_alarm_configuration.TBD = 0;*/
	/*  set to Olt or to specific onu , supported only by OAM 2.0 or higher */
	PonDefaultAlarmConfiguration.errored_frame_alarm_active = TRUE;
	PonDefaultAlarmConfiguration.errored_frame_alarm_configuration.errored_frame_threshold = PON_ALARM_ERRORED_FRAMED_THRESHOLD;
	PonDefaultAlarmConfiguration.errored_frame_alarm_configuration.errored_frame_window = PON_ALARM_ERRORED_FRAMED_WINDOW;
	/*PonDefaultAlarmConfiguration.errored_frame_alarm_configuration.TBD = 0;*/
	/* set to Olt or to specific onu , supported only by OAM 2.0 or higher */
	PonDefaultAlarmConfiguration.errored_frame_period_alarm_active = TRUE;
	PonDefaultAlarmConfiguration.errored_frame_period_alarm_configuration.errored_frame_threshold = PON_ALARM_ERRORED_FRAME_PERIOD_THRESHOLD;
	PonDefaultAlarmConfiguration.errored_frame_period_alarm_configuration.errored_frame_window = PON_ALARM_ERRORED_FRAME_PERIOD_WINDOW;
	/*PonDefaultAlarmConfiguration.errored_frame_period_alarm_configuration.TBD = 0;*/
	/* set to Olt or to specific onu , supported only by OAM 2.0 or higher */
	PonDefaultAlarmConfiguration.errored_frame_seconds_alarm_active = TRUE;
	PonDefaultAlarmConfiguration.errored_frame_seconds_alarm_configuration.errored_frame_seconds_summary_threshold = PON_ALARM_ERRORED_FRAME_SECONDS_THRESHOLD;
	PonDefaultAlarmConfiguration.errored_frame_seconds_alarm_configuration.errored_frame_seconds_summary_window = PON_ALARM_ERRORED_FRAME_SECONDS_WINDOW;
	/*PonDefaultAlarmConfiguration.errored_frame_seconds_alarm_configuration.TBD = 0;*/
	/* onu register error, only set to olt */
	PonDefaultAlarmConfiguration.onu_registration_error_alarm_active = TRUE;

	/* OAM link disconnection, only set to olt */
	PonDefaultAlarmConfiguration.oam_link_disconnection_alarm_active = TRUE;

	/* bad encryption key, only set to olt */
	PonDefaultAlarmConfiguration.bad_encryption_key_alarm_active = TRUE;

	/* llid mismatch, only set to olt */
	PonDefaultAlarmConfiguration.llid_mismatch_alarm_active = TRUE ; /*FALSE */
	PonDefaultAlarmConfiguration.llid_mismatch_alarm_configuration.llid_mismatch_threshold = 1; 
	/*PonDefaultAlarmConfiguration.llid_mismatch_alarm_configuration.TBD = 0;*/

	/* too many onu registeration, only set to olt */
	PonDefaultAlarmConfiguration.too_many_onus_registering_alarm_active = TRUE;

	PonDefaultAlarmConfiguration.device_fatal_error_alarm_active = TRUE;

#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
	PonDefaultAlarmConfiguration.virtual_scope_onu_laser_always_on_alarm_active =FALSE ;
	PonDefaultAlarmConfiguration.virtual_scope_onu_laser_always_on_alarm_configuration.detect_threshold = ONU_LASER_ALWAYS_ON_DEFAULT;
	PonDefaultAlarmConfiguration.virtual_scope_onu_signal_degradation_alarm_active = FALSE;
	PonDefaultAlarmConfiguration.virtual_scope_onu_signal_degradation_alarm_configuration.detect_threshold = ONU_LASER_DEGRADATION_DEFAULT;
	PonDefaultAlarmConfiguration.virtual_scope_onu_eol_alarm_active = FALSE;
	PonDefaultAlarmConfiguration.virtual_scope_onu_eol_alarm_configuration.threshold = ONU_EOL_THRESHOLD_DEFAULT ;
#endif

	return( ROK );
}

/*****************************************************
 *
 *    Function:   pon_assign_handler()
 *
 *    Param:   none
 *                 
 *    Desc:  assign the event/alarm handler function
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
 int pon_assign_handler_5001()
{
#ifdef PON_ENCRYPTION_HANDLER

	if(PAS_assign_handler_function_v4(PAS_HANDLER_START_ENCRYPTION_ACKNOWLEDGE,	 (void*)Start_encryption_acknowledge_handler) != PAS_EXIT_OK){
		sys_console_printf("register start encryption handler error (pon_assign_handler_function())\r\n" );
		}
		
	if(PAS_assign_handler_function_v4(PAS_HANDLER_STOP_ENCRYPTION_ACKNOWLEDGE,	(void*) Stop_encryption_acknowledge_handler) != PAS_EXIT_OK){
		sys_console_printf("register stop encryption ack handler error (pon_assign_handler_function())\r\n" );
		}
	if(PAS_assign_handler_function_v4 (PAS_HANDLER_UPDATE_ENCRYPTION_KEY_ACKNOWLEDGE,(void*)Update_encryption_key_acknowledge_handler) != PAS_EXIT_OK){
		sys_console_printf("register update encryption key ack handler error (pon_assign_handler_function())\r\n" );
		}
#endif	

	if(PAS_assign_handler_function_v4 (PAS_HANDLER_FRAME_RECEIVED,	(void*)Ethernet_frame_received_handler)  != PAS_EXIT_OK){
		sys_console_printf("error: register ethernet frame received handler (pon_assign_handler_function())\r\n" );
		}
	
	if(PAS_assign_handler_function_v4 (PAS_HANDLER_ONU_REGISTRATION, (void*)Onu_registration_handler ) != PAS_EXIT_OK){
		sys_console_printf("error: register onu registeration handler (pon_assign_handler_function())\r\n" );
		}
	if(PAS_assign_handler_function_v4 (PAS_HANDLER_OLT_RESET, (void*)Olt_reset_handler) != PAS_EXIT_OK){
		sys_console_printf("error: register olt reset handler (pon_assign_handler_function())\r\n" );
		}
	if(PAS_assign_handler_function_v4 (PAS_HANDLER_ALARM, (void*)Alarm_handler) != PAS_EXIT_OK){
		sys_console_printf("error: register alarm handler (pon_assign_handler_function())\r\n" );
		}	
	if(PAS_assign_handler_function_v4 (PAS_HANDLER_ONU_DEREGISTRATION, (void*)Onu_deregistration_handler)	 != PAS_EXIT_OK) {
		sys_console_printf("error: register onu deregisteration handler (pon_assign_handler_function())\r\n" );
		}
	if( PAS_assign_handler_function_v4(PAS_HANDLER_LOAD_OLT_BINARY_COMPLETED, ( void*) PAS_load_olt_binary_completed_handler) != PAS_EXIT_OK ){
		sys_console_printf("error: register load binary file complete handler (PAS_load_olt_binary_completed_handler())\r\n" );
		}
	
	if(PAS_assign_handler_function_v4 (PAS_HANDLER_ONU_AUTHORIZATION,	(void*)Onu_authorization_handler) != PAS_EXIT_OK){
		sys_console_printf("register onu authorization handler error (pon_assign_handler_function())\r\n" );
		}

	if(PAS_assign_handler_function_v4 (PAS_HANDLER_DBA_EVENT, (void *) PLATO2_algorithm_event ) != PAS_EXIT_OK ) {
		sys_console_printf("register DBA algorithm handler error (pon_assign_handler_function())\r\n" );
		}
	/*
	if( PAS_assign_handler_function_v4(PAS_HANDLER_PONG, ( void *) Ping_Pong_Test ) != PAS_EXIT_OK ){
		sys_console_printf("register Ping-Pong test handler error ( Ping_Pong_Test())\r\n" );
		}
	*/
	return( ROK );
}

#if PAS_CHIP_5201_
/*  for PAS-SOFT5201  V5.1 */
#endif

#define PON_ENCRYPTION_HANDLER
 int pon_assign_handler_5201()
{
	short int Handler_id=1;
	/*short int ret;*/
	
#ifdef PON_ENCRYPTION_HANDLER

	if(PAS_assign_handler_function(PAS_HANDLER_START_ENCRYPTION_ACKNOWLEDGE,	 (void*)Start_encryption_acknowledge_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register start encryption handler error\r\n" );
		}

	if(PAS_assign_handler_function(PAS_HANDLER_STOP_ENCRYPTION_ACKNOWLEDGE,	(void*) Stop_encryption_acknowledge_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register stop encryption ack handler error\r\n" );
		}
	if(PAS_assign_handler_function (PAS_HANDLER_UPDATE_ENCRYPTION_KEY_ACKNOWLEDGE,(void*)Update_encryption_key_acknowledge_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register update encryption key ack handler error\r\n" );
		}
#endif	

	if(PAS_assign_handler_function (PAS_HANDLER_FRAME_RECEIVED,	(void*)Ethernet_frame_received_handler, &Handler_id)  != PAS_EXIT_OK){
		sys_console_printf("register ethernet frame received handler error\r\n" );
		}
	if(PAS_assign_handler_function (PAS_HANDLER_ONU_REGISTRATION, (void*)Onu_registration_handler , &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register onu registeration handler error\r\n" );
		}
#if 1
	if(PAS_assign_handler_function (PAS_HANDLER_OLT_RESET, (void*)Olt_reset_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register olt reset handler error\r\n" );
		}
#endif
	if(PAS_assign_handler_function (PAS_HANDLER_ALARM, (void*)Alarm_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register alarm handler error\r\n" );
		}	
	if(PAS_assign_handler_function (PAS_HANDLER_ONU_DEREGISTRATION, (void*)Onu_deregistration_handler, &Handler_id)	 != PAS_EXIT_OK) {
		sys_console_printf("register onu deregisteration handler error\r\n" );
		}
	if( PAS_assign_handler_function(PAS_HANDLER_LOAD_OLT_BINARY_COMPLETED, ( void*) PAS_load_olt_binary_completed_handler, &Handler_id) != PAS_EXIT_OK ){
		sys_console_printf("register load binary file complete handler error\r\n" );
		}
/* removed by xieshl 20100608, 兼容LOID认证模式告警 */
/*	if(PAS_assign_handler_function (PAS_HANDLER_ONU_AUTHORIZATION,	(void*)Onu_authorization_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register onu authorization handler error ( pon_assign_handler_function())\r\n" );
		}*/
		
	
#ifdef PLATO_DBA_V3
	if(PAS_assign_handler_function (PAS_HANDLER_DBA_EVENT, (void *) PLATO3_algorithm_event , &Handler_id) != PAS_EXIT_OK ) {
		sys_console_printf("register DBA algorithm handler error\r\n" );
		}
#else
	if(PAS_assign_handler_function (PAS_HANDLER_DBA_EVENT, (void *) PLATO2_algorithm_event , &Handler_id) != PAS_EXIT_OK ) {
		sys_console_printf("register DBA algorithm handler error\r\n" );
		}
#endif

	if(PAS_assign_handler_function( PAS_HANDLER_PON_LOSS, (void *)PAS_pon_loss_handler, &Handler_id) != PAS_EXIT_OK ) {
		sys_console_printf("register PON loss handler error\r\n" );
		}

	if(PAS_assign_handler_function( PAS_HANDLER_ONU_DENIED, (void *)OnuDeniedByMacAddrTable, &Handler_id) != PAS_EXIT_OK ) {
		sys_console_printf("register ONU denied handler error\r\n" );
		}
	
	/*sys_console_printf(" register DBA algorithm handler %d \r\n", Handler_id );*/
	/*
	if( PAS_assign_handler_function_v4(PAS_HANDLER_PONG, ( void *) Ping_Pong_Test ) != PAS_EXIT_OK ){
		sys_console_printf("register Ping-Pong test handler error ( Ping_Pong_Test())\r\n" );
		}
	*/
#if  0  /*_PON_VIRTUAL_OPTICAL_SCOPE_*/
	if((ret = PAS_assign_handler_function(PAS_HANDLER_CONVERT_RSSI_TO_DBM, (void *)Convert_RSSI_to_dBm, &Handler_id)) != PAS_EXIT_OK ) {
		sys_console_printf("register convert rssi to dbm handler error ( pon_assign_handler_function()) %d\r\n", ret );
		}
	if((ret = PAS_assign_handler_function(PAS_HANDLER_GET_OLT_TEMPERATURE, (void *)Get_olt_temperature, &Handler_id)) != PAS_EXIT_OK ) {
		sys_console_printf("register get olt temperature handler error ( pon_assign_handler_function()) %d\r\n", ret );
		}
#endif
#ifdef PAS_SOFT_VERSION_V5_3_5
	if(PAS_assign_handler_function(PAS_HANDLER_CNI_LINK, (void *)Pon_cni_link_handler, &Handler_id) != PAS_EXIT_OK ) {
		sys_console_printf("register cni link status handler error\r\n" );
		}
#endif

#ifdef PAS_SOFT_VERSION_V5_3_11
	if(PAS_assign_handler_function (PAS_HANDLER_REDUNDANCY_OLT_FAILURE, (void*)Olt_redundancy_swap_begin_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register olt redundancy begin handler error\r\n" );
		}
	if(PAS_assign_handler_function (PAS_HANDLER_REDUNDANCY_SWITCH_OVER, (void*)Olt_redundancy_swap_end_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register olt redundancy end handler error\r\n" );
		}
#endif

#ifdef PAS_SOFT_VERSION_V5_3_12
	if(PAS_assign_handler_function (PAS_HANDLER_REDUNDANCY_OLT_FAILURE_WITH_EXTEND_INFO, (void*)Olt_redundancy_swaponu_begin_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register olt redundancy begin-ext handler error\r\n" );
		}
#endif

	return( ROK );
}


#if defined(_EPON_10G_PMC_SUPPORT_)            
int pon_assign_handler_8411()
{
	short int Handler_id=1;
	/*short int ret;*/
	
#ifdef PON_ENCRYPTION_HANDLER

	if(GW10G_PAS_assign_handler_function(GW10G_PAS_HANDLER_START_ENCRYPTION_ACKNOWLEDGE,	 (void*)Start_encryption_acknowledge_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register start encryption handler error\r\n" );
		}

	if(GW10G_PAS_assign_handler_function(GW10G_PAS_HANDLER_STOP_ENCRYPTION_ACKNOWLEDGE,	(void*) Stop_encryption_acknowledge_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register stop encryption ack handler error\r\n" );
		}
	if(GW10G_PAS_assign_handler_function (GW10G_PAS_HANDLER_UPDATE_ENCRYPTION_KEY_ACKNOWLEDGE,(void*)Update_encryption_key_acknowledge_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register update encryption key ack handler error\r\n" );
		}
#endif	

	if(GW10G_PAS_assign_handler_function (GW10G_PAS_HANDLER_FRAME_RECEIVED,	(void*)Ethernet_frame_received_handler, &Handler_id)  != PAS_EXIT_OK){
		sys_console_printf("register ethernet frame received handler error\r\n" );
		}
	if(GW10G_PAS_assign_handler_function (GW10G_PAS_HANDLER_ONU_REGISTRATION, (void*)Onu_registration_handler_8411 , &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register onu registeration handler error\r\n" );
		}
#if 1
	if(GW10G_PAS_assign_handler_function (GW10G_PAS_HANDLER_OLT_RESET, (void*)Olt_reset_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register olt reset handler error\r\n" );
		}
#endif
	if(GW10G_PAS_assign_handler_function (GW10G_PAS_HANDLER_ALARM, (void*)Alarm_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register alarm handler error\r\n" );
		}	
	if(GW10G_PAS_assign_handler_function (GW10G_PAS_HANDLER_ONU_DEREGISTRATION, (void*)Onu_deregistration_handler, &Handler_id)	 != PAS_EXIT_OK) {
		sys_console_printf("register onu deregisteration handler error\r\n" );
		}
	if( GW10G_PAS_assign_handler_function(GW10G_PAS_HANDLER_LOAD_OLT_BINARY_COMPLETED, ( void*) PAS_load_olt_binary_completed_handler, &Handler_id) != PAS_EXIT_OK ){
		sys_console_printf("register load binary file complete handler error\r\n" );
		}
/* removed by xieshl 20100608, 兼容LOID认证模式告警 */
/*	if(PAS_assign_handler_function (PAS_HANDLER_ONU_AUTHORIZATION,	(void*)Onu_authorization_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register onu authorization handler error ( pon_assign_handler_function())\r\n" );
		}*/
		
	

	if(GW10G_PAS_assign_handler_function (GW10G_PAS_HANDLER_DBA_EVENT, (void *) PLATO4_algorithm_event , &Handler_id) != PAS_EXIT_OK ) {
		sys_console_printf("register DBA algorithm handler error\r\n" );
		}


	if(GW10G_PAS_assign_handler_function( GW10G_PAS_HANDLER_PON_LOSS, (void *)PAS_pon_loss_handler, &Handler_id) != PAS_EXIT_OK ) {
		sys_console_printf("register PON loss handler error\r\n" );
		}

     if(PAS_assign_handler_function( GW10G_PAS_HANDLER_OLT_ADD, (void *)Olt_added_handler, &Handler_id) != PAS_EXIT_OK ) {
		sys_console_printf("register OLT added handler error\r\n" );
		}
	 
	if(GW10G_PAS_assign_handler_function( GW10G_PAS_HANDLER_ONU_DENIED, (void *)OnuDeniedByMacAddrTable, &Handler_id) != PAS_EXIT_OK ) {
		sys_console_printf("register ONU denied handler error\r\n" );
		}
	
	/*sys_console_printf(" register DBA algorithm handler %d \r\n", Handler_id );*/
	/*
	if( PAS_assign_handler_function_v4(PAS_HANDLER_PONG, ( void *) Ping_Pong_Test ) != PAS_EXIT_OK ){
		sys_console_printf("register Ping-Pong test handler error ( Ping_Pong_Test())\r\n" );
		}
	*/
#if  0  /*_PON_VIRTUAL_OPTICAL_SCOPE_*/
	if((ret = PAS_assign_handler_function(PAS_HANDLER_CONVERT_RSSI_TO_DBM, (void *)Convert_RSSI_to_dBm, &Handler_id)) != PAS_EXIT_OK ) {
		sys_console_printf("register convert rssi to dbm handler error ( pon_assign_handler_function()) %d\r\n", ret );
		}
	if((ret = PAS_assign_handler_function(PAS_HANDLER_GET_OLT_TEMPERATURE, (void *)Get_olt_temperature, &Handler_id)) != PAS_EXIT_OK ) {
		sys_console_printf("register get olt temperature handler error ( pon_assign_handler_function()) %d\r\n", ret );
		}
#endif
#ifdef PAS_SOFT_VERSION_V5_3_5
	if(GW10G_PAS_assign_handler_function(GW10G_PAS_HANDLER_CNI_LINK, (void *)Pon_cni_link_handler_8411, &Handler_id) != PAS_EXIT_OK ) {
		sys_console_printf("register cni link status handler error\r\n" );
		}
#endif

#ifdef PAS_SOFT_VERSION_V5_3_11
	if(GW10G_PAS_assign_handler_function (GW10G_PAS_HANDLER_REDUNDANCY_OLT_FAILURE, (void*)Olt_redundancy_swap_begin_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register olt redundancy begin handler error\r\n" );
		}
	if(GW10G_PAS_assign_handler_function (GW10G_PAS_HANDLER_REDUNDANCY_SWITCH_OVER, (void*)Olt_redundancy_swap_end_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register olt redundancy end handler error\r\n" );
		}
#endif

#ifdef PAS_SOFT_VERSION_V5_3_12
	if(GW10G_PAS_assign_handler_function (GW10G_PAS_HANDLER_REDUNDANCY_OLT_FAILURE_WITH_EXTEND_INFO, (void*)Olt_redundancy_swaponu_begin_handler, &Handler_id) != PAS_EXIT_OK){
		sys_console_printf("register olt redundancy begin-ext handler error\r\n" );
		}
#endif

	return( ROK );
}

int GW8411_redundancy_assign_handler()
{
    unsigned short Handler_id;

#ifdef PAS_SOFT_VERSION_V5_3_12
    if(GW10G_redundancy_assign_handler_function (REDUNDANCY_HANDLER_SWITCH_OVER_BETWEEN_PAS_SOFT, (void*)Olt_redundancy_swap_between_handler, &Handler_id) != PAS_EXIT_OK)
    {
        sys_console_printf("error: register olt redundancy SWITCH_OVER_BETWEEN_PAS_SOFT handler (GW10G_redundancy_assign_handler_function())\r\n" );
    }
    if(GW10G_redundancy_assign_handler_function (REDUNDANCY_HANDLER_SLAVE_OLT_NOT_AVAILABLE, (void*)Olt_redundancy_slave_unavail_handler, &Handler_id) != PAS_EXIT_OK)
    {
        sys_console_printf("error: register olt redundancy SLAVE_OLT_NOT_AVAILABLE handler (GW10G_redundancy_assign_handler_function())\r\n" );
    }
    if(GW10G_redundancy_assign_handler_function (REDUNDANCY_HANDLER_SWITCH_OVER_SUCCESS, (void*)Olt_redundancy_swap_success_handler, &Handler_id) != PAS_EXIT_OK)
    {
        sys_console_printf("error: register olt redundancy SWITCH_OVER_SUCCESS handler (GW10G_redundancy_assign_handler_function())\r\n" );
    }
    if(GW10G_redundancy_assign_handler_function (REDUNDANCY_HANDLER_REDUNDANCY_SWITCH_OVER_FAILED, (void*)Olt_redundancy_swap_fail_handler, &Handler_id) != PAS_EXIT_OK)
    {
        sys_console_printf("error: register olt redundancy SWITCH_OVER_FAILED handler (GW10G_redundancy_assign_handler_function())\r\n" );
    }
    if(GW10G_redundancy_assign_handler_function (REDUNDANCY_HANDLER_SWITCH_OPTICAL_PORT_REQUIRED, (void*)Olt_redundancy_swap_opticalquery_handler, &Handler_id) != PAS_EXIT_OK)
    {
        sys_console_printf("error: register olt redundancy SWITCH_OPTICAL_PORT_REQUIRED handler (GW10G_redundancy_assign_handler_function())\r\n" );
    }
#endif

	return( ROK );
}

#endif

int GW5201_redundancy_assign_handler()
{
    unsigned short Handler_id;

#ifdef PAS_SOFT_VERSION_V5_3_12
    if(redundancy_assign_handler_function (REDUNDANCY_HANDLER_SWITCH_OVER_BETWEEN_PAS_SOFT, (void*)Olt_redundancy_swap_between_handler, &Handler_id) != PAS_EXIT_OK)
    {
        sys_console_printf("error: register olt redundancy SWITCH_OVER_BETWEEN_PAS_SOFT handler (redundancy_assign_handler_function())\r\n" );
    }
    if(redundancy_assign_handler_function (REDUNDANCY_HANDLER_SLAVE_OLT_NOT_AVAILABLE, (void*)Olt_redundancy_slave_unavail_handler, &Handler_id) != PAS_EXIT_OK)
    {
        sys_console_printf("error: register olt redundancy SLAVE_OLT_NOT_AVAILABLE handler (redundancy_assign_handler_function())\r\n" );
    }
    if(redundancy_assign_handler_function (REDUNDANCY_HANDLER_SWITCH_OVER_SUCCESS, (void*)Olt_redundancy_swap_success_handler, &Handler_id) != PAS_EXIT_OK)
    {
        sys_console_printf("error: register olt redundancy SWITCH_OVER_SUCCESS handler (redundancy_assign_handler_function())\r\n" );
    }
    if(redundancy_assign_handler_function (REDUNDANCY_HANDLER_REDUNDANCY_SWITCH_OVER_FAILED, (void*)Olt_redundancy_swap_fail_handler, &Handler_id) != PAS_EXIT_OK)
    {
        sys_console_printf("error: register olt redundancy SWITCH_OVER_FAILED handler (redundancy_assign_handler_function())\r\n" );
    }
    if(redundancy_assign_handler_function (REDUNDANCY_HANDLER_SWITCH_OPTICAL_PORT_REQUIRED, (void*)Olt_redundancy_swap_opticalquery_handler, &Handler_id) != PAS_EXIT_OK)
    {
        sys_console_printf("error: register olt redundancy SWITCH_OPTICAL_PORT_REQUIRED handler (redundancy_assign_handler_function())\r\n" );
    }
#endif

	return( ROK );
}


int pon_assign_handler_tk2pas5201()
{
	short int Handler_id;
	
	if(TK_assign_pashandler_function (PAS_HANDLER_FRAME_RECEIVED,	(void*)Ethernet_frame_received_handler, &Handler_id) != 0){
		sys_console_printf("register ethernet frame received handler error\r\n" );
		}
	if(TK_assign_pashandler_function (PAS_HANDLER_ONU_REGISTRATION, (void*)Onu_registration_handler , &Handler_id) != 0){
		sys_console_printf("register onu registeration handler error\r\n" );
		}
	if(TK_assign_pashandler_function (PAS_HANDLER_OLT_RESET, (void*)Olt_reset_handler, &Handler_id) != 0){
		sys_console_printf("register olt reset handler error\r\n" );
		}
	if(TK_assign_pashandler_function (PAS_HANDLER_ALARM, (void*)Alarm_handler, &Handler_id) != 0){
		sys_console_printf("register alarm handler error\r\n" );
		}	
	if(TK_assign_pashandler_function (PAS_HANDLER_ONU_DEREGISTRATION, (void*)Onu_deregistration_handler, &Handler_id) != 0) {
		sys_console_printf("register onu deregisteration handler error\r\n" );
		}
	/*delete by luh@2015--6-25, 16epon ponlos告警不稳定，暂时先去掉*/
#if 0	
	if(TK_assign_pashandler_function( PAS_HANDLER_PON_LOSS, (void *)PAS_pon_loss_handler, &Handler_id) != 0 ) {
		sys_console_printf("register PON loss handler error\r\n" );
		}
#endif	
	if(TK_assign_pashandler_function( PAS_HANDLER_ONU_DENIED, (void *)OnuDeniedByMacAddrTable, &Handler_id) != 0 ) {
		sys_console_printf("register ONU denied handler error\r\n" );
		}
	if(TK_assign_pashandler_function(PAS_HANDLER_CNI_LINK, (void *)Pon_cni_link_handler, &Handler_id) != 0 ) {
		sys_console_printf("register cni link status handler error\r\n" );
		}

	return( ROK );
}

int pon_assign_handler_tk3723()
{
	unsigned short Handler_id;
	
	if(TK_assign_handler_function (TK_HANDLER_GPIO_CHANGED,	(void *)Pon_gpio_changed_handler, &Handler_id) != 0){
		sys_console_printf("register GPIO Changed Event handler error\r\n" );
		}

	return( ROK );
}


#if defined(_EPON_10G_BCM_SUPPORT_)            
int pon_assign_handler_bcm2pas5201()
{
	short int Handler_id;
	
	if(BCM_assign_pashandler_function (PAS_HANDLER_FRAME_RECEIVED,	(void*)Ethernet_frame_received_handler, &Handler_id) != 0){
		sys_console_printf("register ethernet frame received handler error\r\n" );
		}
	if(BCM_assign_pashandler_function (PAS_HANDLER_OLT_RESET, (void*)Olt_reset_handler, &Handler_id) != 0){
		sys_console_printf("register olt reset handler error\r\n" );
		}
	if(BCM_assign_pashandler_function (PAS_HANDLER_ALARM, (void*)Alarm_handler, &Handler_id) != 0){
		sys_console_printf("register alarm handler error\r\n" );
		}	
	if(BCM_assign_pashandler_function( PAS_HANDLER_PON_LOSS, (void *)PAS_pon_loss_handler, &Handler_id) != 0 ) {
		sys_console_printf("register PON loss handler error\r\n" );
		}
	if(BCM_assign_pashandler_function( PAS_HANDLER_ONU_DENIED, (void *)OnuDeniedByMacAddrTable, &Handler_id) != 0 ) {
		sys_console_printf("register ONU denied handler error\r\n" );
		}
	if(BCM_assign_pashandler_function(PAS_HANDLER_CNI_LINK, (void *)Pon_cni_link_handler, &Handler_id) != 0 ) {
		sys_console_printf("register cni link status handler error\r\n" );
		}

	return( ROK );
}

int pon_assign_handler_bcm55538()
{
	unsigned short Handler_id;
	
	if(BCM_assign_handler_function (BCM_HANDLER_LINK_DISCOVERY,	(void *)Pon_llid_discovery_handler, &Handler_id) != 0){
		sys_console_printf("register GPIO Changed Event handler error\r\n" );
		}
	
	if(BCM_assign_handler_function (BCM_HANDLER_LINK_LOSS,	(void *)Pon_llid_loss_handler, &Handler_id) != 0){
		sys_console_printf("register GPIO Changed Event handler error\r\n" );
		}
	
	if(BCM_assign_handler_function (BCM_HANDLER_OAM_FRAME_RECEIVED,	(void *)PonStdOamPktRecvHandler, &Handler_id) != 0){
		sys_console_printf("register OAM Frame Recv Event handler error\r\n" );
		}
	
	if(BCM_assign_handler_function (BCM_HANDLER_GPIO_CHANGED,	(void *)Pon_gpio_changed_handler, &Handler_id) != 0){
		sys_console_printf("register GPIO Changed Event handler error\r\n" );
		}

	return( ROK );
}
#endif


short int SetPonCniPatameter(short int PonPortIdx )
{
	short int PonChipType;
	short int PonChipVer = RERROR;
	short int ret = PAS_EXIT_OK;

	CHECK_PON_RANGE

	PonChipType = V2R1_GetPonchipType(PonPortIdx);
	/*for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	if( OLT_PONCHIP_ISPAS(PonChipType))
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
		{
			
			ret = Pon_SetCniPortMacConfiguration ( PonPortIdx, PonChipType, &PAS_port_cni_parameters );
		}
		else
		{
			PON_olt_cni_port_mac_configuration_t  Pon5201_Cni_para;
			/*PON_external_downlink_buffer_size_t    external_downlink_buffer_size;*/

			/*sys_console_printf(" PAS5201 set CNI port config \r\n");*/
			VOS_MemCpy( &Pon5201_Cni_para, &PAS_port_cni_parameters, sizeof(PON_olt_cni_port_mac_configuration_t));
#if 0
			Pon5201_Cni_para.master = PON_MASTER;
			Pon5201_Cni_para.auto_negotiation = DISABLE;
			Pon5201_Cni_para.pause_thresholds.pause_set_threshold=PON_PAUSE_SET_RELEASE_THRESHOLD_90_PERCENTS; /*PON_VALUE_NOT_CHANGED;*/
			Pon5201_Cni_para.pause_thresholds.pause_release_threshold=PON_PAUSE_SET_RELEASE_THRESHOLD_85_PERCENTS; /*PON_VALUE_NOT_CHANGED;*/
			Pon5201_Cni_para.advertisement_details.preferable_port_type = DISABLE;
			Pon5201_Cni_para.advertisement_details.pause = DISABLE;
			Pon5201_Cni_para.advertisement_details.asymmetric_pause = DISABLE;
#endif


			{
			/*
	       	 if (Get_external_downlink_buffer_size ( PonPortIdx, &external_downlink_buffer_size) != PAS_EXIT_OK )
	        		{
	           		return (PAS_QUERY_FAILED);
	        		}
			 */
			 /* added by chenfj 2007-7-19 
				增加PON芯片下行是否有外部数据BUF判断*/

			 if( getPonExtSdramSupported(GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx) ) != 0)

	       	/*if (external_downlink_buffer_size != PON_EXTERNAL_DOWNLINK_BUFFER_0MB)  */
	       		{
				Pon5201_Cni_para.pause_thresholds.pause_set_threshold     = PON_PAUSE_SET_RELEASE_THRESHOLD_90_PERCENTS; /*PON_VALUE_NOT_CHANGED;*/
				Pon5201_Cni_para.pause_thresholds.pause_release_threshold = PON_PAUSE_SET_RELEASE_THRESHOLD_85_PERCENTS; /*PON_VALUE_NOT_CHANGED;*/
	       		}
			}
		    /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	 		ret = Pon_SetCniPortMacConfiguration ( PonPortIdx, PonChipType, &Pon5201_Cni_para );
			}
		if( ret != PAS_EXIT_OK )
			{
			sys_console_printf(" Set Cni port config err %d \r\n", ret );
			return ( RERROR );
			}
		return( ROK );
		}
	
	else{ /* other pon chip handler */
		return( RERROR );
		}
	
	return( RERROR );

}

short int GetPonCniPatameter(short int PonPortIdx )
{
	PON_olt_cni_port_mac_configuration_t  olt_cni_config;
	short int PonChipType, ret;
	
 	CHECK_PON_RANGE

	PonChipType = V2R1_GetPonchipType(PonPortIdx);
	/*for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	if(OLT_PONCHIP_ISPAS(PonChipType))
        {
		
		ret = Pon_GetCniPortMacConfiguration(PonPortIdx, PonChipType, &olt_cni_config);
		if( ret == PAS_EXIT_OK )
			{
			sys_console_printf("olt CNI mac config\r\n");
			sys_console_printf("master %d, pause %d, PHY-MDIO %d, Auto_neg %d\r\n", olt_cni_config.master, olt_cni_config.pause,olt_cni_config.mdio_phy_address, olt_cni_config.auto_negotiation);
			sys_console_printf("set threshold %d, release threshold %d\r\n", olt_cni_config.pause_thresholds.pause_set_threshold, olt_cni_config.pause_thresholds.pause_release_threshold);
			sys_console_printf("advertise %d \r\n", olt_cni_config.advertise);
			sys_console_printf("tx half %d, tx full %d, M/S %d, pause %d, asymme-pause %d\r\n", olt_cni_config.advertisement_details._1000base_tx_half_duplex, olt_cni_config.advertisement_details._1000base_tx_full_duplex, olt_cni_config.advertisement_details.preferable_port_type, olt_cni_config.advertisement_details.pause, olt_cni_config.advertisement_details.asymmetric_pause );
			return( ROK );
			}
		else {
			sys_console_printf(" Get Olt CNI mac config Err %d\r\n", ret );
			return( RERROR );
			}
		}

	else { /* other pon chip type handler */
		return( RERROR );
		}
	
	return( ROK );
}



short int  SetOltIgmpSnooping( short int PonPortIdx, short int enable_flag )
{
	PON_olt_igmp_configuration_t      igmp_configuration;
	short int ret;
		
	CHECK_PON_RANGE

	if( (enable_flag != V2R1_ENABLE) && ( enable_flag != V2R1_DISABLE )) return( RERROR );

	if( enable_flag == V2R1_ENABLE )
		{
		igmp_configuration.enable_igmp_snooping = ENABLE;
		igmp_configuration.igmp_timeout = 10; /* unit: second */
		}
	else {
		igmp_configuration.enable_igmp_snooping = DISABLE;
		igmp_configuration.igmp_timeout = 0;
		}
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret =  OLT_SetOltIgmpSnoopingMode( PonPortIdx, &igmp_configuration );

	if( ret == PAS_EXIT_OK ) return( ROK );
	else{
		sys_console_printf(" set olt igmp snooping mode Err %d \r\n", ret); 
		return( RERROR );
		}
}

short int GetOltIgmpSnootping(short int PonPortIdx)
{
	PON_olt_igmp_configuration_t      igmp_configuration;
	short int ret;
		
	CHECK_PON_RANGE
    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = OLT_GetOltIgmpSnoopingMode(PonPortIdx, &igmp_configuration);
	if( ret== PAS_EXIT_OK )
		{
		sys_console_printf("%s/port%d IGMP snooping,",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		if(igmp_configuration.enable_igmp_snooping == ENABLE ) 
			sys_console_printf("Mode: enable, timeout=%d\r\n", igmp_configuration.igmp_timeout );
		else sys_console_printf("Mode: disable\r\n");
		return( ROK );
		}
	sys_console_printf("%s/port%d IGMP snooping get Err",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
	return( RERROR );
}

#if 1
int CopyOltAddressTableConfig(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    int iRlt = FALSE;
    OLT_addr_table_cfg_t src_cfg;

    src_cfg.aging_timer = PonPortTable[SrcPonPortIdx].MACAgeingTime;
    src_cfg.allow_learning = V2R1_ENABLE;
    src_cfg.discard_llid_unlearned_sa = PonPortTable[SrcPonPortIdx].discard_unlearned_sa;
    src_cfg.discard_unknown_da = PonPortTable[SrcPonPortIdx].discard_unlearned_da;
    src_cfg.removed_when_full = PonPortTable[SrcPonPortIdx].table_full_handle_mode;
    src_cfg.removed_when_aged = TRUE;
  
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = TRUE;
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        OLT_addr_table_cfg_t dft_cfg;

        dft_cfg.aging_timer = PonPortConfigDefault.MACAgeingTime;
        dft_cfg.allow_learning = V2R1_ENABLE;
        dft_cfg.discard_llid_unlearned_sa = FALSE;
        dft_cfg.discard_unknown_da = PON_DIRECTION_NO_DIRECTION;
        dft_cfg.removed_when_full = FALSE;
        dft_cfg.removed_when_aged = TRUE;

        if ( 0 != VOS_MemCmp(&dft_cfg, &src_cfg, sizeof(OLT_addr_table_cfg_t)) )
        {
            iRlt = TRUE;
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            OLT_addr_table_cfg_t dst_cfg;
        
            dst_cfg.aging_timer = PonPortTable[DstPonPortIdx].MACAgeingTime;
            dst_cfg.allow_learning = V2R1_ENABLE;
            dst_cfg.discard_llid_unlearned_sa = PonPortTable[DstPonPortIdx].discard_unlearned_sa;
            dst_cfg.discard_unknown_da = PonPortTable[DstPonPortIdx].discard_unlearned_da;
            dst_cfg.removed_when_full = PonPortTable[DstPonPortIdx].table_full_handle_mode;
            dst_cfg.removed_when_aged = TRUE;

            if ( 0 != VOS_MemCmp(&dst_cfg, &src_cfg, sizeof(OLT_addr_table_cfg_t)) )
            {
                iRlt = TRUE;
            }
        }
        else
        {
            iRlt = TRUE;
        }
    }


    if ( TRUE == iRlt )
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SetAddressTableConfig(DstPonPortIdx, &src_cfg)) )
        {
            iRlt = OLT_SetMacAgingTime(DstPonPortIdx, src_cfg.aging_timer);
        }
    }


    return iRlt;
}

int ResumeOltAddressTableConfig(short int PonPortIdx)
{
    return CopyOltAddressTableConfig(PonPortIdx, PonPortIdx, OLT_COPYFLAGS_COVER);
}
#else
short int ConfigOltAddressTable(short int PonPortIdx /*, short int direction, short int limit_entry*/ )
{
	PON_address_table_config_t  address_table_config;
	short int ret1,/* ret2,*/ ret3;

	CHECK_PON_RANGE

	/* address table aging */
	address_table_config.removed_when_aged = TRUE;
	address_table_config.aging_timer = PonPortTable[PonPortIdx].MACAgeingTime;
	address_table_config.allow_learning = /*direction; */PON_DIRECTION_UPLINK;
	address_table_config.discard_llid_unlearned_sa = PonPortTable[PonPortIdx].discard_unlearned_sa;
	address_table_config.discard_unknown_da = PonPortTable[PonPortIdx].discard_unlearned_da ;/*V2R1_discard_unknown_da;*/
	ret1 = PAS_set_address_table_configuration( PonPortIdx, address_table_config );
	if( ret1 != PAS_EXIT_OK )
		sys_console_printf(" 1 Set Olt  Address Para Err %d \r\n", ret1 );

	/* CNI port MAX entries 
	ret2 = PAS_set_port_mac_limiting( PonPortIdx, PON_PORT_ID_CNI, PON_ADDRESS_TABLE_ENTRY_LIMITATION_0);
	if( ret2 != PAS_EXIT_OK )
		sys_console_printf(" 2 Set Olt  Address Para Err %d \r\n", ret2 );
	
	default MAX entries setting for all default port 
	ret3 = PAS_set_port_mac_limiting( PonPortIdx, PON_DEFAULT_LLID, PON_ADDRESS_TABLE_ENTRY_LIMITATION_64);
	if( ret3 != PAS_EXIT_OK )
		sys_console_printf(" 3 Set Olt  Address Para Err %d \r\n", ret3 );
	*/
	ret3 = PAS_set_address_table_full_handling_mode( PonPortIdx, PonPortTable[PonPortIdx].table_full_handle_mode );

	if( ( ret1 == PAS_EXIT_OK )/* && ( ret2 == PAS_EXIT_OK )*/ && ( ret3 == PAS_EXIT_OK ) )
		return( ROK );
	else return( RERROR );
}
#endif

int GetOltAddressTable( short int PonPortIdx )
{
#if 0
	short int ret;
	short int PonChipVer = RERROR;
	short int PonChipType;
	
	CHECK_PON_RANGE

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}

	if( PonChipType == PONCHIP_PAS )
		{
		/*PonChipVer = V2R1_GetPonChipVersion( PonPortIdx );*/
		if((PonChipVer != PONCHIP_PAS5001 ) && ( PonChipVer != PONCHIP_PAS5201 ) ) return( RERROR );

		if( PonChipVer == PONCHIP_PAS5001 )
			{
			return( ROK);
			}
		else if( PonChipVer == PONCHIP_PAS5201 )
			{
			PON_address_table_config_t      address_table_config;
			short int  maximum_entries;
			
			ret = PAS_get_address_table_configuration( PonPortIdx, &address_table_config);
			if( ret == PAS_EXIT_OK )
				{
				sys_console_printf("%s/port%d address config:\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
				sys_console_printf(" aging time:%d, removed_when_aged:", address_table_config.aging_timer );
				if( address_table_config.removed_when_aged == TRUE ) sys_console_printf(" true\r\n");
				else sys_console_printf(" false\r\n");
				sys_console_printf(" address learning direction:%s\r\n", PonLinkDirection_s[address_table_config.allow_learning]);
				sys_console_printf(" discard unlearned sa: %d ", address_table_config.discard_llid_unlearned_sa );
				if( address_table_config.discard_unknown_da > 3 )
					address_table_config.discard_unknown_da = 0;
				sys_console_printf(" discard unlearned da:%s\r\n", PonLinkDirection_s[address_table_config.discard_unknown_da]);
				}
			else {
				sys_console_printf("Get %s/port%d address config Err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ret );
				}
			ret = PAS_get_port_mac_limiting( PonPortIdx, PON_PORT_ID_CNI, &maximum_entries);
			if( ret == PAS_EXIT_OK )
				sys_console_printf("CNI port Mac entry limit %d \r\n", maximum_entries );
			else sys_console_printf("Get CNI port Mac entry limit Err %d\r\n", ret );

			ret = PAS_get_port_mac_limiting( PonPortIdx, PON_DEFAULT_LLID, &maximum_entries);
			if( ret == PAS_EXIT_OK )
				sys_console_printf("default port Mac entry limit %d \r\n", maximum_entries );
			else sys_console_printf("Get default port Mac entry limit Err %d\r\n", ret );

			return( ROK );
			}
		}

	/* other pon chip type handler */
	else{

		}

	return( RERROR );
#else
	return ROK;
#endif
}


short int  ConfigOltARPFilter(short int PonPortIdx )
{
	short int ret;
	PON_olt_classification_t   classification_entity;
	PON_olt_classifier_destination_t   destination;
	PON_pon_network_traffic_direction_t direction;

	CHECK_PON_RANGE

	classification_entity = PON_OLT_CLASSIFICATION_ARP;
	
	/* from pon to firmware */	
#if 0    
	direction = PON_DIRECTION_UPLINK;
	destination = PON_OLT_CLASSIFIER_DESTINATION_HOST;	
	ret = PAS_set_classification_rule ( PonPortIdx, direction, classification_entity, NULL, destination );
#endif       

	/* from cni to firmware */
#if 0    
	direction = PON_DIRECTION_DOWNLINK;
	destination = PON_OLT_CLASSIFIER_DESTINATION_HOST;	
	ret = PAS_set_classification_rule ( PonPortIdx, direction, classification_entity, NULL, destination );
#endif             

	/* from pon to cni */
	direction = PON_DIRECTION_UPLINK;
	destination = PON_OLT_CLASSIFIER_DESTINATION_DATA_PATH;	
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = Pon_SetClassificationRule ( PonPortIdx, direction, classification_entity, NULL, destination );
	if( ret != PAS_EXIT_OK )
	{
		sys_console_printf(" Set Olt  ARP Filter Err %d \r\n", ret );
		return( RERROR );
	}
	
	/* from cni to pon */
	direction = PON_DIRECTION_DOWNLINK;
	destination = PON_OLT_CLASSIFIER_DESTINATION_DATA_PATH;	
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = Pon_SetClassificationRule ( PonPortIdx, direction, classification_entity, NULL, destination );
	if( ret != PAS_EXIT_OK )
	{
		sys_console_printf(" Set Olt  ARP Filter Err %d \r\n", ret );
		return( RERROR );
	}
    
	return( ROK );
}

short int GetOltARPFilter( short int PonPortIdx )
{
	short int ret;
	short int PonChipType;
	
	/*unsigned char para[20];*/
	
	CHECK_PON_RANGE

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( !OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			PON_olt_classification_t   classification_entity;
			PON_olt_classifier_destination_t   destination;
			PON_pon_network_traffic_direction_t direction;

			classification_entity = PON_OLT_CLASSIFICATION_ARP;
			direction = PON_DIRECTION_UPLINK;
			/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = Pon_GetClassificationRule( PonPortIdx,  direction, classification_entity, NULL, &destination );
			
			if( ret == PAS_EXIT_OK )
				{
				sys_console_printf("%s/port%d uplink ARP filter\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
				sys_console_printf("destination %d\r\n", destination );
				}
			else {
				sys_console_printf("Get uplink ARP filter Err %d \r\n", ret );
				}

			classification_entity = PON_OLT_CLASSIFICATION_ARP;
			direction = PON_DIRECTION_DOWNLINK;
			/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = Pon_GetClassificationRule( PonPortIdx,  direction, classification_entity, NULL, &destination );
			
			if( ret == PAS_EXIT_OK )
				{
				sys_console_printf("%s/port%d downlink ARP filter\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
				sys_console_printf("destination %d\r\n", destination );
				}
			else {
				sys_console_printf("Get uplink ARP filter Err %d \r\n", ret );
				}
			
			}

		}

	else{

		}
	return( RERROR );

}


/*added by liyang @2015-05-18 for Q25883*/
short int  ConfigOltDhcpFilter(short int PonPortIdx )
{
	short int ret;
	PON_olt_classification_t   classification_entity;
	PON_olt_classifier_destination_t   destination;
	PON_pon_network_traffic_direction_t direction;

	CHECK_PON_RANGE

	classification_entity = PON_OLT_CLASSIFICATION_DHCP;
	  
	/* from pon to cni */
	direction = PON_DIRECTION_UPLINK;
	destination = PON_OLT_CLASSIFIER_DESTINATION_DATA_PATH;	
	
	ret = Pon_SetClassificationRule ( PonPortIdx, direction, classification_entity, NULL, destination );
	if( ret != PAS_EXIT_OK )
	{
		sys_console_printf(" Set Olt Dhcp Filter Err %d \r\n", ret );
		return( RERROR );
	}
	
	/* from cni to pon */
	direction = PON_DIRECTION_DOWNLINK;
	destination = PON_OLT_CLASSIFIER_DESTINATION_DATA_PATH;	
	
	ret = Pon_SetClassificationRule ( PonPortIdx, direction, classification_entity, NULL, destination );
	if( ret != PAS_EXIT_OK )
	{
		sys_console_printf(" Set Olt Dhcp Filter Err %d \r\n", ret );
		return( RERROR );
	}
    
	return( ROK );
}

short int  ConfigOltVLAN( short int PonPortIdx )
{
	CHECK_PON_RANGE

	/*
	PAS_set_llid_vlan_configuration_v4
                             ( const short int							 olt_id, 
							   const PON_llid_t						     llid,
							   const PON_vlan_handling_mode_t			 handling_mode,
							   const PON_vlan_exchange_configuration_t  *exchange_configuration,
							   const PON_vlan_stack_configuration_t     *stack_configuration );
	*/
	return( ROK );
}

short int GetOltVlanConfig_Uplink(short int PonPortIdx, short int OnuIdx  )
{
	short int ret;
	short int onu_id;
	short int PonChipType;
	
	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) !=  ONU_OPER_STATUS_UP ) return (RERROR );

	onu_id = GetLlidByOnuIdx(  PonPortIdx,  OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );
	
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if(OLT_PONCHIP_ISPAS(PonChipType))
	{
			PON_olt_vlan_uplink_config_t  vlan_uplink_config;
			#if 1
			ret = OnuMgt_GetOnuVlanMode( PonPortIdx, OnuIdx, &vlan_uplink_config );
			#else
			ret = PAS_get_llid_vlan_mode( PonPortIdx, onu_id, &vlan_uplink_config );
			#endif
            
			if(  ret == PAS_EXIT_OK  )
				{
				sys_console_printf("%s/port%d onu%d uplink vlan config\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
				sys_console_printf("untagged_frames_authentication_vid %d\r\n", vlan_uplink_config.untagged_frames_authentication_vid );
				sys_console_printf("authenticated_vid %d\r\n", vlan_uplink_config.authenticated_vid );
				sys_console_printf("discard_untagged %d, discard_tagged %d, discard_null_tagged %d, discard_nested %d\r\n", vlan_uplink_config.discard_untagged,vlan_uplink_config.discard_tagged, vlan_uplink_config.discard_null_tagged, vlan_uplink_config.discard_nested );
				sys_console_printf("vlan_manipulation %d\r\n", vlan_uplink_config.vlan_manipulation );
				sys_console_printf("new_vlan_tag_id %d\r\n", vlan_uplink_config.new_vlan_tag_id );
				sys_console_printf("vlan_type 0x%x\r\n", vlan_uplink_config.vlan_type );
				sys_console_printf("vlan_priority %d\r\n", vlan_uplink_config.vlan_priority );
				return( ROK );
				}
			

		}
	/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

	else  {

		}

	return( RERROR );

}


short int GetOltVlanConfig_Downlink(short int PonPortIdx, short int OnuIdx  )
{
	short int ret;
	short int onu_id;
	short int PonChipType;
	short int PonChipVer = RERROR;
	
	CHECK_ONU_RANGE

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) !=  ONU_OPER_STATUS_UP ) return (RERROR );

	onu_id = GetLlidByOnuIdx(  PonPortIdx,  OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );
	
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	/* for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( !OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			PON_olt_vid_downlink_config_t  vlan_downlink_config;
			/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = OLT_GetVidDownlinkMode( PonPortIdx, VLAN_UNTAGGED_ID, &vlan_downlink_config );

			if(  ret == PAS_EXIT_OK  )
				{
				sys_console_printf("%s/port%d onu%d downlink vlan config, untagged frame\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
				sys_console_printf("discard_untagged %d\r\n", vlan_downlink_config.discard_nested);
				sys_console_printf("destination %d, llid %d\r\n", vlan_downlink_config.destination, vlan_downlink_config.llid);
				sys_console_printf("vlan_manipulation %d\r\n", vlan_downlink_config.vlan_manipulation );
				sys_console_printf("new_id %d, new priority %d \r\n\r\n", vlan_downlink_config.new_vid, vlan_downlink_config.new_priority );
					
				}
            /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = OLT_GetVidDownlinkMode( PonPortIdx, NULL_VLAN, &vlan_downlink_config );

			if(  ret == PAS_EXIT_OK  )
				{
				sys_console_printf("%s/port%d onu%d downlink vlan config, NULL-vlan frame\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
				sys_console_printf("discard_untagged %d\r\n", vlan_downlink_config.discard_nested);
				sys_console_printf("destination %d, llid %d\r\n", vlan_downlink_config.destination, vlan_downlink_config.llid);
				sys_console_printf("vlan_manipulation %d\r\n", vlan_downlink_config.vlan_manipulation );
				sys_console_printf("new_id %d, new priority %d \r\n", vlan_downlink_config.new_vid, vlan_downlink_config.new_priority );
					
				return( ROK );
				}
			}

		}

	else  {

		}

	return( RERROR );

}

int SetPonEncryptMode( short int PonPortIdx, int Mode )
{
	short int ret;
#if 0
	short int PonChipType;
	short int PonChipVer = RERROR;
#endif
	
	CHECK_PON_RANGE

#if 1
	if(V2R1_CTC_STACK)
    {
        ret = OLT_SetEncryptMode( PonPortIdx, Mode );
    }         
	else if(  Mode == PON_ENCRYPTION_TYPE_AES )
    {
        ret = OLT_SetEncryptMode( PonPortIdx, Mode );
    }         
	else
        return( RERROR );
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

	if( PonChipType == PONCHIP_PAS )
	{
		if( (Mode != PON_ENCRYPTION_TYPE_AES ) && (Mode != PON_ENCRYPTION_TYPE_TRIPLE_CHURNING ) ) return( RERROR );
		
		if( PonChipVer == PONCHIP_PAS5001 )
            return( ROK );
		else 
		{	
			if(V2R1_CTC_STACK)
            {
				ret = PAS_set_encryption_configuration( PonPortIdx, Mode );
            }         
			else if(  Mode == PON_ENCRYPTION_TYPE_AES )
            {
				ret = PAS_set_encryption_configuration( PonPortIdx, Mode );
            }         
			else return( RERROR );

			if( ret != PAS_EXIT_OK ) return (RERROR );

#if 0
            PonPortTable[PonPortIdx].EncryptType = Mode;     
#endif
            
			return( ROK );
		}
	}
	else
    { /* other pon chip handler */
	}
#endif

	if( OLT_CALL_ISERROR(ret) ) return (RERROR );

	return( ROK );
}

int GetPonEncryptMode( short int PonPortIdx, int  *Mode )
{
	/*short int ret;
	short int PonChipType;
	short int PonChipVer = RERROR;*/
	
	CHECK_PON_RANGE

	if( Mode == NULL ) return( RERROR );

#if 1
    *Mode = PonPortTable[PonPortIdx].EncryptType;     
	return( ROK );
#else
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
	}

	if( PonChipType == PONCHIP_PAS )
	{
		/*PonChipVer = V2R1_GetPonChipVersion( PonPortIdx );*/
		if((PonChipVer != PONCHIP_PAS5001 ) && (PonChipVer != PONCHIP_PAS5201 ) ) return( RERROR );

		if( PonChipVer == PONCHIP_PAS5001 ) 
		{
			*Mode = PON_ENCRYPTION_TYPE_AES;
			return( ROK );
		}
		else if( PonChipVer == PONCHIP_PAS5201 )
		{
			ret = PAS_get_encryption_configuration( PonPortIdx, (PON_encryption_type_t *)Mode );

			if( ret != PAS_EXIT_OK ) return (RERROR );
			return( ROK );
			}
	}
	else
    {/* other pon chip handler */
	}

	return( RERROR );
#endif
}

int  SetPonDBAReportMode( short int  PonPortIdx,  int Report_Mode )
{
	short int ret;
	short int PonChipType;
	short int PonChipVer = RERROR;

	CHECK_PON_RANGE

	if( PonPortIsWorking(PonPortIdx ) == FALSE ) return( RERROR );

	if(( Report_Mode != PON_DBA_STANDARD_REPORT ) && ( Report_Mode != PON_DBA_THRESHOLD_REPORT ))
		return( RERROR );
	PonChipType = V2R1_GetPonchipType( PonPortIdx) ;
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) ) return( RERROR );
        /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		ret = OLT_SetDBAReportFormat( PonPortIdx,  Report_Mode );

		if( ret == PAS_EXIT_OK ) 
			{
			/*PonPortTable[PonPortIdx].DBAReportMode = Report_Mode; */
			return( ROK );
			}
		else return( RERROR );

		}

	else { /* other pon chip handler */

		}
	return( RERROR );

}

int  GetPonDBAReportMode( short int  PonPortIdx,  int *Report_Mode )
{
	short int ret;
	short int PonChipType;
	short int PonChipVer = RERROR;

	CHECK_PON_RANGE

	if( PonPortIsWorking(PonPortIdx ) == FALSE ) return( RERROR );
	if( Report_Mode == NULL ) return ( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx) ;
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) ) return( RERROR );
        /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		ret = OLT_GetDBAReportFormat( PonPortIdx, ( PON_DBA_report_format_t *)Report_Mode );

		if( ret == PAS_EXIT_OK ) 
			{
			/*PonPortTable[PonPortIdx].DBAReportMode = *Report_Mode; */
			return( ROK );
			}
		else return( RERROR );

		}

	else { /* other pon chip handler */

		}
	return( RERROR );

}

#if 0	/* removed by xieshl 20110714 */
short int SetONUDefaultConfigMode( )
{
	CTC_STACK_oui_version_record_t  oui_version_records_list[MAX_OUI_RECORDS];
	unsigned char  number_of_records = 0;
	unsigned char automatic_mode = ENABLE;
	unsigned char  automatic_onu_configuration = ENABLE;

	/*ret = CTC_STACK_get_parameters( &automatic_mode, &number_of_records, oui_version_records_list, &automatic_onu_configuration);
	if( ret == PAS_EXIT_OK )*/

	oui_version_records_list[number_of_records].oui[0] = ((CTC_OUI_DEFINE >> 16) & 0xff);
	oui_version_records_list[number_of_records].oui[1] = ((CTC_OUI_DEFINE >> 8) & 0xff);
	oui_version_records_list[number_of_records].oui[2] = (CTC_OUI_DEFINE & 0xff);
	oui_version_records_list[number_of_records].version= 0x01;
	number_of_records++;
	oui_version_records_list[number_of_records].oui[0] = ((CTC_OUI_DEFINE >> 16) & 0xff);
	oui_version_records_list[number_of_records].oui[1] = ((CTC_OUI_DEFINE >> 8) & 0xff);
	oui_version_records_list[number_of_records].oui[2] = (CTC_OUI_DEFINE & 0xff);
	oui_version_records_list[number_of_records].version= 0x20;
	number_of_records++;

	oui_version_records_list[number_of_records].oui[0] = ((CTC_OUI_DEFINE >> 16) & 0xff);
	oui_version_records_list[number_of_records].oui[1] = ((CTC_OUI_DEFINE >> 8) & 0xff);
	oui_version_records_list[number_of_records].oui[2] = (CTC_OUI_DEFINE & 0xff);
	oui_version_records_list[number_of_records].version = CTC_2_1_ONU_VERSION;
	number_of_records++;

	oui_version_records_list[number_of_records].oui[0] = ((CTC_OUI_DEFINE >> 16) & 0xff);
	oui_version_records_list[number_of_records].oui[1] = ((CTC_OUI_DEFINE >> 8) & 0xff);
	oui_version_records_list[number_of_records].oui[2] = (CTC_OUI_DEFINE & 0xff);
	oui_version_records_list[number_of_records].version = 0xc1;		/* added by xieshl 20110414, 联通扩展OAM版本 */
	number_of_records++;

	return CTC_STACK_set_parameters(automatic_mode, number_of_records, oui_version_records_list, automatic_onu_configuration);
}

int   CTC_AddSupportedOUI( unsigned char *OUI, unsigned char version )
{
	unsigned char automatic_mode;
	unsigned char  number_of_records=0;
	CTC_STACK_oui_version_record_t  oui_version_records_list[MAX_OUI_RECORDS];
	unsigned char automatic_onu_configuration;
	
	short int ret;

	if(OUI == NULL ) return( RERROR );

	ret = CTC_STACK_get_parameters( &automatic_mode, &number_of_records, oui_version_records_list, &automatic_onu_configuration);
	if( ret == PAS_EXIT_OK )
		{
		oui_version_records_list[number_of_records].oui[0] = OUI[0];
		oui_version_records_list[number_of_records].oui[1] = OUI[1];
		oui_version_records_list[number_of_records].oui[2] = OUI[2];
		oui_version_records_list[number_of_records].version = version ;
		ret = CTC_STACK_set_parameters(automatic_mode, (number_of_records+1), oui_version_records_list, automatic_onu_configuration);
		if( ret == PAS_EXIT_OK )  return( ROK );
		else return( RERROR );
		}

	return( RERROR );
}

int CTC_GetSupportedOUI()
{
	unsigned char automatic_mode;
	unsigned char  number_of_records=0;
	CTC_STACK_oui_version_record_t  oui_version_records_list[MAX_OUI_RECORDS];
	unsigned char automatic_onu_configuration;
	short int ret;
	unsigned char i;
	
	/*if( oui_version_records_list == NULL ) return( RERROR );*/
	ret = CTC_STACK_get_parameters( &automatic_mode, &number_of_records, oui_version_records_list, &automatic_onu_configuration);
	if( ret == PAS_EXIT_OK )
		{
		if( EVENT_DEBUG == V2R1_ENABLE )
			{
			number_of_records = MIN( number_of_records, MAX_OUI_RECORDS );
			sys_console_printf("Supported OUI number=%d\r\n", number_of_records );
			if( number_of_records != 0 )
				{
				for( i=0; i< number_of_records; i++ )
					{
					sys_console_printf("%d OUI:0x%02x%02x%02x, version %d\r\n", (i+1), oui_version_records_list[i].oui[0], oui_version_records_list[i].oui[1], oui_version_records_list[i].oui[2], oui_version_records_list[i].version );
					}
				}

			}
		return( ROK );
		}
	return( RERROR );
}
#endif

int CTC_SetPonEncryptTimeParam(unsigned char UpdateTime, unsigned short  int  NoReplyTime )
{
	short int ret;

	if( NoReplyTime > 2550 ) return( RERROR );
	/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = CTC_SetEncryptionTiming( UpdateTime, NoReplyTime );
	if( ret == PAS_EXIT_OK ) return( ROK );
	else return ( RERROR );
}

int CTC_GetPonEncryptTimeParam(unsigned char *UpdateTime, unsigned short  int  *NoReplyTime )
{
	short int ret;

	if((UpdateTime == NULL )  || ( NoReplyTime == NULL )) return ( RERROR );
    /*for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	ret = CTC_GetEncryptionTiming( UpdateTime, NoReplyTime );
	if( ret == PAS_EXIT_OK ) return( ROK );
	else return ( RERROR );
}

int  CTC_SetPonEncryptTimingThreshold( int TimingThreshold )
{
	short int ret;
	unsigned char Threshold;

	Threshold = (unsigned  char )TimingThreshold;
	/*for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	ret = CTC_SetEncryptionTimingThreshold( Threshold );
	if( ret == PAS_EXIT_OK ) return( ROK );	
	else return( RERROR );
}

int CTC_GetPonEncryptTimingThreshold( int  *TimingThreshold )
{
	short int ret;
	unsigned char Threshold;

	if( TimingThreshold == NULL ) return( RERROR );
	/*for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	ret = CTC_GetEncryptionTimingThreshold( &Threshold );
	if( ret == PAS_EXIT_OK )
		{
		*TimingThreshold = Threshold;
		return( ROK );
		}
	else return( RERROR );
}

int  CTCEncryptKeyUpdateTime = ( SECOND_10 / SECOND_1 );
int  CTCEncryptNoReplyTime = 30;

int CTC_SetPonEncryptKeyUpdateTime( int UpdateTime )
{
	/*
	unsigned char UpdateTime1;
	unsigned short int NoReplyTime;
	int ret;

	if( CTC_GetPonEncryptTimeParam( &UpdateTime1, &NoReplyTime) == ROK )
		{
		UpdateTime1 = (unsigned  char )UpdateTime;
		ret = CTC_SetPonEncryptTimeParam(UpdateTime1, NoReplyTime );
		}
	else{
		UpdateTime1 = (unsigned  char )UpdateTime;
		ret = CTC_SetPonEncryptTimeParam(UpdateTime1, CTC_EncryptNoReplyTime );
		}
	if( ret == PAS_EXIT_OK ) return( ROK );
	else return( RERROR );
	*/
	CTCEncryptKeyUpdateTime = UpdateTime;
	return( ROK );
}

int  CTC_GetPonEncryptKeyUpdateTime( int *UpdateTime )
{
	int ret; 
	unsigned short int NoReplyTime;
	unsigned char Update;

	ret =  CTC_GetPonEncryptTimeParam( &Update, &NoReplyTime);
	if( ret == ROK )
		{
		*UpdateTime = Update;
		return( ROK );
		}
	else return( RERROR );
}


int CTC_SetPonEncryptNoReplyTime( int  NoReplyTime )
{
	/*
	unsigned char UpdateTime;
	unsigned short int NoReplyTime1;
	*/
	int ret;

	CTCEncryptNoReplyTime = NoReplyTime;

	/*
	if( CTC_GetPonEncryptTimeParam( &UpdateTime, &NoReplyTime1) == ROK )
		{
		NoReplyTime1 = (unsigned short int )NoReplyTime;
		ret = CTC_SetPonEncryptTimeParam(UpdateTime, NoReplyTime1 );
		}
	else{
		NoReplyTime1 = (unsigned short int )NoReplyTime;
		ret = CTC_SetPonEncryptTimeParam(CTC_EncryptKeyTimeDefault, NoReplyTime1 );
		}

		*/
	ret = CTC_SetPonEncryptTimeParam(CTCEncryptKeyUpdateTime, CTCEncryptNoReplyTime );
	
	if( ret == PAS_EXIT_OK ) return( ROK );
	else return( RERROR );
}


int CTC_GetPonEncryptNoReplyTime( int *NoReplyTime )
{
	int ret; 
	unsigned char UpdateTime;
	unsigned short  NoReply;

	ret =  CTC_GetPonEncryptTimeParam( &UpdateTime, &NoReply);
	if( ret == ROK )
		{
		*NoReplyTime = NoReply;
		return( ROK );
		}
	else return( RERROR );
}

int CTC_SetEncryptPreambleMode(short int PonPortIdx, char mode)
{
	short int PonChipType;
	
	short int ret;

	CHECK_PON_RANGE

	if((mode != V2R1_ENABLE) && ( mode != V2R1_DISABLE )) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			return( RERROR );
			}
		else 
			{
			if( mode == V2R1_DISABLE ) mode = DISABLE;
			/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = OLT_SetEncryptionPreambleMode( PonPortIdx, mode );
			if( ret == PAS_EXIT_OK ) return (ROK );
			else return (RERROR );
			}
		}

	else{ /* other pon chip handler */

		}
	
	return( RERROR );	

}

int CTC_GetEncryptPreambleMode(short int PonPortIdx, char *mode)
{
	short int PonChipType;
	
	short int ret;

	CHECK_PON_RANGE

	if( mode == NULL) return( RERROR );

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			return( RERROR );
			}
		else 
			{
			if( *mode == V2R1_DISABLE ) *mode = DISABLE;
			/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = OLT_GetEncryptionPreambleMode( PonPortIdx, mode );
			if( ret == PAS_EXIT_OK ) return (ROK );
			else return (RERROR );
			}
		}

	else{ /* other pon chip handler */

		}
	
	return( RERROR );	

}

int CTC_SetMLDForwardMode(short int PonPortIdx,  char mode)
{
	short int PonChipType;
	short int PonChipVer = RERROR;
	short int ret;
	
	CHECK_PON_RANGE

	if((mode != V2R1_ENABLE) && ( mode != V2R1_DISABLE )) return( RERROR );

	if( PonPortIsWorking( PonPortIdx) != TRUE) return( RERROR );
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			return( RERROR );
			}
		else
			{
			if( mode == V2R1_DISABLE ) mode = DISABLE;
			/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = OLT_SetOltMldForwardingMode( PonPortIdx, mode );
			if( ret == PAS_EXIT_OK ) return (ROK );
			else return (RERROR );
			}
		}

	else{ /* other pon chip handler */

		}	
	return( RERROR );	
}

int CTC_GetMLDForwardMode(short int PonPortIdx,  char *mode)
{
	short int PonChipType;
	short int PonChipVer = RERROR;
	short int ret;
	
	CHECK_PON_RANGE

	if( mode == NULL)  return( RERROR );

	if( PonPortIsWorking( PonPortIdx) != TRUE) return( RERROR );
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
		
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if( OLT_PONCHIP_ISPAS5001(PonChipType) )
			{
			return( RERROR );
			}
		else
			{
			if( *mode == V2R1_DISABLE ) *mode = DISABLE;
			/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			ret = OLT_GetOltMldForwardingMode( PonPortIdx, mode );
			if( ret == PAS_EXIT_OK ) return (ROK );
			else return (RERROR );
			}
		}

	else{ /* other pon chip handler */

		}	
	return( RERROR );	
}


int  Init_PAS_CTC_STACK(void )
{
	/*CTC_STACK_terminate();*/
	
	if( (CTC_STACK_is_init(&CTC_STACK_Init) != CTC_STACK_EXIT_OK) || (CTC_STACK_Init != TRUE) )
	{
		CTC_stack_initialize();
		CTC_STACK_Init = TRUE;
		pon_assign_pashandler_CTC_STACK();
	}

	return( ROK );
}

int  Init_TK_CTC_STACK(void )
{
	/*CTC_STACK_terminate();*/
	
	if( (TkCTC_Stack_isInit(&CTC_STACK_Init) != CTC_STACK_EXIT_OK) || (CTC_STACK_Init != TRUE) )
	{
	    int ret;
        
    	if( 0 != (ret = TkCTC_Stack_Init(PAS_oui_version_records_num, PAS_oui_version_records_list)) ) 
    	{
    		sys_console_printf(" CTC STACK init Err %d \r\n", ret );
    		return( RERROR );
    	}
        
		CTC_STACK_Init = TRUE;
		pon_assign_tkhandler_CTC_STACK();
	}

	return( ROK );
}

int Init_PON_CTC_STACK()
{
	if( (PON_CTC_STACK_is_init(&CTC_STACK_Init) != CTC_STACK_EXIT_OK) || (CTC_STACK_Init != TRUE) )
	{
		PON_CTC_stack_initialize();
		CTC_STACK_Init = TRUE;
		pon_assign_ponhandler_CTC_STACK();
	}

	return( ROK );
}

/*****************************************************
 *
 *    Function:   pon_load_external_dba_file(short int PonPortIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                 
 *    Desc:  load the external DBA file to the PON port
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
/*extern unsigned char DBAImage[];
extern int DBAImageLength;*/
#define  DBA_INITIAL_VER  "V2.1"

int pon_load_external_dba_file(short int PonPortIdx )
{
	short int PonChipType;
	short int  PonChipVer = RERROR;
	short int ret = PAS_EXIT_ERROR;
	short int i;
	int location=0, length=0;
	unsigned char ver[100];

	CHECK_PON_RANGE	
		
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
	}

	if( PonChipType == PONCHIP_PAS )
	{
		/*setPonTraceFlag( PonPortIdx, V2R1_ENABLE);*/
			
		/* modified for PAS-SOFT V5.1 
		PonChipVer = V2R1_GetPonChipVersion(PonPortIdx);
		if(( PonChipVer != PAS_CHIP_5001 ) && ( PonChipVer != PAS_CHIP_5201 )) 
			{
			sys_console_printf(" Err\r\n");
			return( RERROR );
			}			
		*/
		if( PonChipVer == PONCHIP_PAS5001 )
		{			
			PON_binary_t DBA_binary;
			
			DBA_binary.type = PON_OLT_BINARY_DBA_DLL;
			DBA_binary.source = PON_BINARY_SOURCE_MEMORY;
			/*
			DBA_binary.location =(void *) Pon_DBA_image[PONCHIP_PAS].location;
			DBA_binary.size = Pon_DBA_image[PONCHIP_PAS].size;
			*/
			if( GetPonChipDBAFileInfo(PonChipVer, &location, &length, ver) == ROK )
			{
				DBA_binary.location = (void *)location;
				DBA_binary.size = length;
			}
			else 
			{
				sys_console_printf(" DBA image %s not found\r\n", PONCHIP_DBA_TYPE_PAS5001);
				return( V2R1_FILE_NOT_FOUND );
			}
		
			VOS_MemSet(&(DBA_binary.identification),0,sizeof(PON_binary_identification_t));
			/* modified by chenfj 2007-10-31
				修改PAS5001 DBA文件下载，支持根据DBA版本不同，或需要key和user-id,
				或不需要(当前对V2.1版本不需要。下载V2.21.1版本DBA文件，需要输入key和user-id）
				*/
			if( VOS_StrCmp( ver, DBA_INITIAL_VER ) != 0 )
			{
				for(i=0; i<PON_BINARY_ENCRYPTION_KEY_SIZE; i++)
					DBA_binary.identification.key[i] = DBA_key[i];

				for(i=0; i<PON_BINARY_USER_ID_SIZE; i++)
					DBA_binary.identification.user_id[i] = DBA_id[i];
			}
			
			ret = PAS_load_olt_binary_v4( PonPortIdx, &DBA_binary );
		}
		else
		{
			PON_indexed_binary_t  DBA_binary1;
			

			DBA_binary1.binary.type = PON_OLT_BINARY_DBA_DLL;
			DBA_binary1.binary.source = PON_BINARY_SOURCE_MEMORY;
			/*
			DBA_binary1.binary.location = ( void *)DBAImage;
			DBA_binary1.binary.size = DBAImageLength;
			
			DBA_binary1.binary.location =(void *) Pon_DBA_image[PONCHIP_PAS].location;
			DBA_binary1.binary.size = Pon_DBA_image[PONCHIP_PAS].size;	
			*/
			if( GetPonChipDBAFileInfo(PonChipVer, &location, &length, ver) == ROK )
			{
				DBA_binary1.binary.location = (void *)location;
				DBA_binary1.binary.size = length;
			}
			else
            {
				if( V2R1_CTC_STACK )
					sys_console_printf(" DBA image %s not found\r\n",PONCHIP_DBA_TYPE_PAS5201_CTC);
				else
                    sys_console_printf(" DBA image %s not found\r\n",PONCHIP_DBA_TYPE_PAS5201);

				return( V2R1_FILE_NOT_FOUND );
			}
			
			/*
			VOS_MemSet(&(DBA_binary1.binary.identification ), '\0', sizeof( PON_binary_identification_t ) );
			DBA_binary1.binary.identification.key[0] = '0';
			DBA_binary1.binary.identification.user_id[0] = '0';
			
			VOS_MemCpy( DBA_binary1.binary.identification.key, DBA_key, PON_BINARY_ENCRYPTION_KEY_SIZE );
			VOS_MemCpy( DBA_binary1.binary.identification.user_id, DBA_id, PON_BINARY_USER_ID_SIZE );
			*/
			for(i=0; i<PON_BINARY_ENCRYPTION_KEY_SIZE; i++)
				DBA_binary1.binary.identification.key[i] = DBA_key[i];

			for(i=0; i<PON_BINARY_USER_ID_SIZE; i++)
				DBA_binary1.binary.identification.user_id[i] = DBA_id[i];
			
			DBA_binary1.index = 2;
			/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
			ret = Pon_LoadOltBinary( PonPortIdx, PonChipVer, &DBA_binary1 );
			/*setPonTraceFlag( PonPortIdx, V2R1_DISABLE);*/
		}		

        /* B--added by liwei056@2010-7-20 for Flash-VFS Support */
        pon_load_release(PonChipVer, 0, 1);
        /* E--added by liwei056@2010-7-20 for Flash-VFS Support */
		
		if(  ret == PAS_EXIT_OK )
		{
			if( EVENT_PONADD == V2R1_ENABLE )
			sys_console_printf("\r\n  Download DBA image to %s/port%d  ...  Ok \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
			/*Trap_DBALoad( PonPortIdx, V2R1_LOAD_FILE_SUCCESS );*/
			return( ROK ); 
		}
		else
        {
			if( EVENT_PONADD == V2R1_ENABLE )
			sys_console_printf("\r\n  Download DBA image to %s/port%d  ...  Err %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), ret);
			if(PonChipMgmtTable[PonPortIdx].operStatus != PONCHIP_ERR )
			{
				Trap_DBALoad( PonPortIdx, V2R1_LOAD_FILE_FAILURE);
				PonPortTable[PonPortIdx].PortWorkingStatus  = PONPORT_DOWN;
				PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ERR;
			}
			/*
			if(( ret == PAS_TIME_OUT )||( ret == PAS_PARAMETER_ERROR ))
				{
				VOS_TaskDelay( VOS_TICK_SECOND/4);
				pon_reset_olt(PonPortIdx );
				Add_PonPort(PonPortIdx );
				}
			*/
			return( RERROR );
		}

	}
	else
    {
        VOS_ASSERT(0);
	}

	return( ROK );
}


#if 0
int  SetPonFetchIGMPMsgToHostPath( short int PonPortIdx )
{

	short int PonChipType;
	short int PonChipVer = RERROR;
	short int ret;

	CHECK_PON_RANGE

	PonChipType = V2R1_GetPonchipType(PonPortIdx);
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
		if( PonChipVer == PONCHIP_PAS5001 ) return( ROK );
		
		ret = PAS_set_classification_rule ( PonPortIdx , PON_DIRECTION_UPLINK, PON_OLT_CLASSIFICATION_IGMP,
											NULL, PON_OLT_CLASSIFIER_DESTINATION_HOST );
            
		ret = PAS_set_classification_rule ( PonPortIdx , PON_DIRECTION_UPLINK, PON_OLT_CLASSIFICATION_MLD,
											NULL, PON_OLT_CLASSIFIER_DESTINATION_HOST );

		if( ret != PAS_EXIT_OK )
			{
			sys_console_printf(" PAS_set_classification_rule Err %d\r\n", ret);
			return( RERROR );
			}
		
		else return( ROK );
		}

	else{ /* other pon chip handler */

		}


	return( RERROR );
	
}

int DisablePonFetchIGMPMsg( short int PonPortIdx )
{

	short int ret;

	CHECK_PON_RANGE

	ret = PAS_set_classification_rule ( PonPortIdx , PON_DIRECTION_NO_DIRECTION, PON_OLT_CLASSIFICATION_IGMP,
											NULL, PON_OLT_CLASSIFIER_DESTINATION_NONE );
            
	if( ret != PAS_EXIT_OK )
		{
		sys_console_printf(" PAS_set_classification_rule Err %d\r\n", ret);
		return( RERROR );
		}
	else return( ROK );
	
}

int SetPonFetchIGMPMsgToAllPath( short int PonPortIdx )
{
	short int ret;

	CHECK_PON_RANGE

	ret = PAS_set_classification_rule ( PonPortIdx , PON_DIRECTION_NO_DIRECTION, PON_OLT_CLASSIFICATION_IGMP,
											NULL, PON_OLT_CLASSIFIER_DESTINATION_HOST_AND_DATA_PATH );
            
	if( ret != PAS_EXIT_OK )
		{
		sys_console_printf(" PAS_set_classification_rule Err %d\r\n", ret);
		return( RERROR );
		}
	else return( ROK );
	
}

int SetPonFetchIGMPMsgToDataPath( short int PonPortIdx )
{
	short int ret;

	CHECK_PON_RANGE

	ret = PAS_set_classification_rule ( PonPortIdx , PON_DIRECTION_NO_DIRECTION, PON_OLT_CLASSIFICATION_IGMP,
											NULL, PON_OLT_CLASSIFIER_DESTINATION_DATA_PATH );
            
	if( ret != PAS_EXIT_OK )
		{
		sys_console_printf(" PAS_set_classification_rule Err %d\r\n", ret);
		return( RERROR );
		}
	else return( ROK );
	
}
#endif

int ResumeOnuDownlinkQosMapping(short int PonPortIdx)
{
    OLT_pri2cosqueue_map_t map;

    VOS_MemCpy(map.priority, PON_priority_queue_mapping, sizeof(PON_priority_queue_mapping));
    return OLT_CALL_ISOK(OLT_SetOnuDownlinkPri2CoSQueueMap(PonPortIdx, &map)) ? ROK : RERROR;
}

int ResumeOLTStaticMacAddrTbl(short int PonPortIdx, short int IsNeedSave)
{
    int iRlt = ROK;

    if ( GlobalPonAddrTblNum > 0 )
    {
        short int i;
        /*short int addr_num;*/
        PON_address_record_t astAddr[MAXOLTADDRNUM];

        VOS_ASSERT(GlobalPonAddrTblNum <= MAXOLTADDRNUM);
        for (i=0; i<GlobalPonAddrTblNum; i++)
        {
            VOS_MemCpy(astAddr[i].mac_address, GlobalPonStaticMacAddr[i], BYTES_IN_MAC_ADDRESS);
            astAddr[i].logical_port = PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID;
            astAddr[i].type = ADDR_STATIC;
        }
        iRlt = OLT_CALL_ISOK(OLT_AddMacAddrTbl(PonPortIdx, (0 == IsNeedSave) ? -i : i, astAddr)) ? ROK : RERROR;
    }

    return iRlt;
}
int ResumeOltInformation(short int PonPortIdx)
{
    char oltName[MAXDEVICENAMELEN];
    ULONG nameLen = 0;
    if (GetOltDeviceName(oltName,  &nameLen) == ROK)
    {
        OLT_SetDeviceName(PonPortIdx, oltName, nameLen);
    }
    return VOS_OK;
}
int SetOnuDownlinkQosMapping(void)
{
	int i;
    
#if 1
    OLT_pri2cosqueue_map_t map;

    VOS_MemCpy(map.priority, PON_priority_queue_mapping, sizeof(PON_priority_queue_mapping));
	for(i=0; i<MAXPON; i++)
	{
		if( OLTAdv_IsExist(i) == TRUE )
            OLT_SetOnuDownlinkPri2CoSQueueMap((short int)i, &map);
	}
#else
	PON_high_priority_frames_t high_priority_frames;
	short int PonChipType;
    
	for(i=0; i<= MAX_PRIORITY_QUEUE; i++)
	{
		high_priority_frames.priority[i] = PON_priority_queue_mapping[i];
	}		

	for(i=0; i<MAXPON; i++)
	{
		PonChipType = V2R1_GetPonchipType(i);
		if( PonChipType == PONCHIP_PAS5201 )
		{
			if( Olt_exists (i) == TRUE )
				PAS_set_policing_thresholds( i,  PON_POLICER_DOWNSTREAM_TRAFFIC, high_priority_frames,  PON_POLICER_MAX_HIGH_PRIORITY_RESERVED );
		}
	}
#endif

    return( ROK );
}


void RestartPonPortAll( void )
{
	short int PonPortIdx;

	for(PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx++)
		{
		if(OLTAdv_IsExist(PonPortIdx))
			{
			PONTx_Disable(PonPortIdx, PONPORT_TX_ACTIVED);
			VOS_TaskDelay(1);
			PONTx_Enable(PonPortIdx, PONPORT_TX_ACTIVED);
			}
		}
}

/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/

PON_STATUS GW_CTC_STACK_assign_handler_function
                ( const CTC_STACK_handler_index_t    handler_function_index, 
				  const void					    (*handler_function)(),
				  unsigned short                     *handler_id )
{
#if defined(_EPON_10G_PMC_SUPPORT_)            
    if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
    	return GW10G_CTC_STACK_assign_handler_function(handler_function_index, (void*)handler_function,handler_id);
	}
	else
#endif
	{
		return CTC_STACK_assign_handler_function(handler_function_index, (void*)handler_function,handler_id);
	}
	
}

int  GW5201_Init_CTC_STACK(void )
{
	/*CTC_STACK_terminate();*/
	
	if(( CTC_STACK_is_init(&CTC_STACK_Init) != CTC_STACK_EXIT_OK ) ||(CTC_STACK_Init != TRUE))
	{
		GW5201_CTC_stack_initialize();
		CTC_STACK_Init = TRUE;
		pon_assign_pashandler_CTC_STACK();
	}

	return( ROK );
}


#if defined(_EPON_10G_PMC_SUPPORT_)      
int  GW8411_Init_CTC_STACK(void )
{
	/*CTC_STACK_terminate();*/
	
	if(( GW10G_CTC_STACK_is_init(&CTC_STACK_Init) != CTC_STACK_EXIT_OK ) ||(CTC_STACK_Init != TRUE))
	{
		GW8411_CTC_stack_initialize();
		CTC_STACK_Init = TRUE;
		pon_assign_pashandler_CTC_STACK();
	}

	return( ROK );
}

int GW8411_pon_init()
{
    short int result = 0;
	/*1 terminate the operation of pas API and halt the pas5001 connected devices */	
	GW10G_PAS_terminate ();
	VOS_TaskDelay( VOS_TICK_SECOND/4 );

	/*2 initialize the PAS-SOFT */
	/*result=PAS_init(&PAS_init_para);	modified for PAS-SOFT V5.1*/
	result = GW10G_PAS_init();
	if(result != PAS_EXIT_OK)
    {
		sys_console_printf("PAS-SOFT init failed(%d) (pon_init())\r\n", result);
		return( RERROR );
	}
	else
    {
		/*sys_console_printf("PAS-SOFT init success ( pon_init() )\r\n" );*/
	}
    
	/*3 initialize the PAS-SOFT system parameters */
	result = pon_set_system_parameters(V2R1_PON_SAMPLING_CYCLE,V2R1_PON_MONITORING_CYCLE,V2R1_PON_HOST_MSG_TIMEOUT, V2R1_PON_HOST_MSG_OUT);
	if(result !=PAS_EXIT_OK)
	{
		sys_console_printf("error(%d): pon_set_system_parameters !(pon_init)\r\n", result);
		return( RERROR );
	}

	/*4 register the event handler function */
	pon_assign_handler_8411();


	if( PLATO4_init() == PAS_EXIT_OK )
	{
		sys_console_printf("\r\nInitialilze the external DBA ok \r\n");
	}
	else
    {
		sys_console_printf("\r\nInitialize the external DBA err \r\n");
	}


	/* 6 Initialize Remote Management packet*/
	if( GW10G_REMOTE_PASONU_init() == PAS_EXIT_OK )
	{
		sys_console_printf("Onu remote mgmt packet init ok \r\n");
	}
	/*	REMOTE_PASONU_reset_device*/
		
	/* 7 Initialize CTC stack */
	if (V2R1_CTC_STACK)
	{
		/*sys_console_printf("==============1 \r\n");*/
		GW8411_Init_CTC_STACK();
		/* sys_console_printf("CTC remote mgmt packet init ok \r\n"); */
	}

    /* B--added by liwei056@2010-1-25 for Pon-FastSwicthHover */
	/* 8 Initialize Redundancy Management packet */
#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
#ifdef PAS_SOFT_VERSION_V5_3_11
	if( GW10G_redundancy_init() == PAS_EXIT_OK )
	{
       	sys_console_printf("Olt redundancy mgmt packet init ok \r\n");

#ifdef PAS_SOFT_VERSION_V5_3_12
        GW8411_redundancy_assign_handler();

    	/* 9 Initialize CTC Redundancy */
    	if (V2R1_CTC_STACK)
    	{
        	if( ctc_GW10G_redundancy_init() == PAS_EXIT_OK ) 
    		{
        		sys_console_printf("CTC redundancy mgmt packet init ok \r\n");
    		}
    	}
#endif

    	if( GW10G_redundancy_enable() == PAS_EXIT_OK )
        {
        	sys_console_printf("Olt redundancy mgmt startup ok \r\n");
        }   
	}
#endif
#endif
    /* E--added by liwei056@2010-1-25 for Pon-FastSwicthHover */

		
}
#endif

int GW5201_pon_init()
{
    short int result = 0;
	/*1 terminate the operation of pas API and halt the pas5001 connected devices */	
	PAS_terminate ();
	VOS_TaskDelay( VOS_TICK_SECOND/4 );

	/*2 initialize the PAS-SOFT */
	/*result=PAS_init(&PAS_init_para);	modified for PAS-SOFT V5.1*/
	result = PAS_init();
	if(result != PAS_EXIT_OK)
    {
		sys_console_printf("PAS-SOFT init failed(%d) (pon_init())\r\n", result);
		return( RERROR );
	}
	else
    {
		/*sys_console_printf("PAS-SOFT init success ( pon_init() )\r\n" );*/
	}
    
	/*3 initialize the PAS-SOFT system parameters */
	result = pon_set_system_parameters(V2R1_PON_SAMPLING_CYCLE,V2R1_PON_MONITORING_CYCLE,V2R1_PON_HOST_MSG_TIMEOUT, V2R1_PON_HOST_MSG_OUT);
	if(result !=PAS_EXIT_OK)
	{
		sys_console_printf("error(%d): pon_set_system_parameters !(pon_init)\r\n", result);
		return( RERROR );
	}

	/*4 register the event handler function */
	pon_assign_handler_5201();

#ifdef PLATO_DBA_V3
	/*5 initialize the DBA and onu Remote management packet */
	if( PLATO3_init() == PAS_EXIT_OK )
	{
		sys_console_printf("\r\nInitialilze the external DBA ok \r\n");
	}
	else
    {
		sys_console_printf("\r\nInitialize the external DBA err \r\n");
	}
#else 
	/*5 initialize the DBA and onu Remote management packet */
	if( PLATO2_init() == PAS_EXIT_OK )
	{
		sys_console_printf("\r\nInitialilze the external DBA ok \r\n");
	}
	else
    {
		sys_console_printf("\r\nInitialize the external DAB err \r\n");
	}
#endif

	/* 6 Initialize Remote Management packet*/
	if( REMOTE_PASONU_init() == PAS_EXIT_OK )
	{
		sys_console_printf("Onu remote mgmt packet init ok \r\n");
	}
	/*	REMOTE_PASONU_reset_device*/
		
	/* 7 Initialize CTC stack */
	if (V2R1_CTC_STACK)
	{
		/*sys_console_printf("==============1 \r\n");*/
		GW5201_Init_CTC_STACK();
		/* sys_console_printf("CTC remote mgmt packet init ok \r\n"); */
	}

    /* B--added by liwei056@2010-1-25 for Pon-FastSwicthHover */
	/* 8 Initialize Redundancy Management packet */
#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
#ifdef PAS_SOFT_VERSION_V5_3_11
	if( redundancy_init() == PAS_EXIT_OK )
	{
       	sys_console_printf("Olt redundancy mgmt packet init ok \r\n");

#ifdef PAS_SOFT_VERSION_V5_3_12
        GW5201_redundancy_assign_handler();

    	/* 9 Initialize CTC Redundancy */
    	if (V2R1_CTC_STACK)
    	{
        	if( ctc_redundancy_init() == PAS_EXIT_OK ) 
    		{
        		sys_console_printf("CTC redundancy mgmt packet init ok \r\n");
    		}
    	}
#endif

    	if( redundancy_enable() == PAS_EXIT_OK )
        {
        	sys_console_printf("Olt redundancy mgmt startup ok \r\n");
        }   
	}
#endif
#endif
    /* E--added by liwei056@2010-1-25 for Pon-FastSwicthHover */
}

int TK3723_pon_init()
{
    int result;

	result = TK_init();
	if( 0 == result )
    {
		/*sys_console_printf("TK-SOFT init success ( pon_init() )\r\n" );*/

        if ( 0 == (result = TkAdapter_PAS5201_Init()) )
        {
        	/*4 register the event handler function */
        	pon_assign_handler_tk2pas5201();
        }
    	else
        {
    		sys_console_printf("TK-SOFT's PasAdapter init failed(%d) (pon_init())\r\n", result);
    		return( RERROR );
    	}

        pon_assign_handler_tk3723();

        /* 使得TkOlt可以RM管理PAS的ONU */
        TK_PASONU_init();
        
    	/* 7 Initialize CTC stack */
    	if ( V2R1_CTC_STACK )
    	{
    		/*sys_console_printf("==============1 \r\n");*/
    		Init_TK_CTC_STACK();
    		/* sys_console_printf("CTC remote mgmt packet init ok \r\n"); */
    	}

#if ( EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES )
        DOCSIS_init();
#endif
	}
	else
    {
		sys_console_printf("TK-SOFT init failed(%d) (pon_init())\r\n", result);
		return( RERROR );
	}

	return (ROK);
}

#if defined(_EPON_10G_BCM_SUPPORT_)            
int BCM55538_pon_init()
{
    int result;

	result = BCM_init();
	if( 0 == result )
    {
		/*sys_console_printf("BCM-SOFT init success ( pon_init() )\r\n" );*/

        if ( 0 == (result = BcmAdapter_PAS5201_Init()) )
        {
        	/*4 register the event handler function */
        	pon_assign_handler_bcm2pas5201();
        }
    	else
        {
    		sys_console_printf("BCM-SOFT's PasAdapter init failed(%d) (pon_init())\r\n", result);
    		return( RERROR );
    	}

        pon_assign_handler_bcm55538();

        /* 此PON芯片需要扩展的软件OAM处理 */
        InitPonEventHandler();

        /* 此PON芯片需要标准802.3ah的OAM处理 */
        PonStdOamInit();

        /* 此PON芯片需要直连的PP处理 */
        InitPonPPHandler();
        
    	/* 7 Initialize CTC stack */
    	if ( V2R1_CTC_STACK )
    	{
    		/*sys_console_printf("==============1 \r\n");*/
    		Init_PON_CTC_STACK();
    		/* sys_console_printf("CTC remote mgmt packet init ok \r\n"); */
    	}

        /* 使得BcmOlt可以RM管理PAS的ONU */
        RM_PASONU_init();

        /* 使得BcmOlt可以RM管理TK的ONU */
        /* RM_TKONU_init(); */
	}
	else
    {
		sys_console_printf("BCM-SOFT init failed(%d) (pon_init())\r\n", result);
		return( RERROR );
	}

	return (ROK);
}
#endif


#if defined(_GPON_BCM_SUPPORT_)
int GPON_pon_init()
{
    int result = 0;

	result = gpon_init();
	if( 0 == result )
    {
		/*sys_console_printf("GponBcm init success ( pon_init() )\r\n" );*/
	#if 1
        if ( 0 == (result = gponAdapter_PAS5201_Init()) )
        {
        	/*4 register the event handler function */
        	pon_assign_handler_gpon2pas5201();
			sys_console_printf("assign handler gpon2pas5201 ok;\r\n");
        }
    	else
        {
    		sys_console_printf("GPONBCM's PasAdapter init failed(%d) (pon_init())\r\n", result);
    		return( RERROR );
    	}

        pon_assign_handler_gpon();
		sys_console_printf("assign handler gpon ok;\r\n");
        #endif
        
	}
	else
    {
		sys_console_printf("GPONBCM init failed(%d) (pon_init())\r\n", result);
		return( RERROR );
	}

	return (ROK);
}

int pon_assign_handler_gpon2pas5201()
{
	short int Handler_id;
	
	
	if(gpon_assign_pashandler_function (PAS_HANDLER_OLT_RESET, (void*)Olt_reset_handler, &Handler_id) != 0){
		sys_console_printf("register olt reset handler error\r\n" );
		}
	if(gpon_assign_pashandler_function (PAS_HANDLER_ALARM, (void*)Alarm_handler, &Handler_id) != 0){
		sys_console_printf("register alarm handler error\r\n" );
		}	
	if(gpon_assign_pashandler_function( PAS_HANDLER_PON_LOSS, (void *)PAS_pon_loss_handler, &Handler_id) != 0 ) {
		sys_console_printf("register PON loss handler error\r\n" );
		}
	if(gpon_assign_pashandler_function( PAS_HANDLER_ONU_DENIED, (void *)OnuDeniedByMacAddrTable, &Handler_id) != 0 ) {
		sys_console_printf("register ONU denied handler error\r\n" );
		}
	if(gpon_assign_pashandler_function(PAS_HANDLER_CNI_LINK, (void *)Pon_cni_link_handler, &Handler_id) != 0 ) {
		sys_console_printf("register cni link status handler error\r\n" );
		}

	if(gpon_assign_pashandler_function (PAS_HANDLER_ONU_REGISTRATION, (void*)GponOnu_registration_handler , &Handler_id) != 0){
		sys_console_printf("register onu registeration handler error\r\n" );
		}

	if(gpon_assign_pashandler_function (PAS_HANDLER_ONU_DEREGISTRATION, (void*)Onu_deregistration_handler , &Handler_id) != 0){
		sys_console_printf("register onu deregisteration handler error\r\n" );
		}

	return( ROK );
}

/*gpon自身事件*/
int pon_assign_handler_gpon()
{
	unsigned short Handler_id;
	
	
	if(gpon_assign_handler_function (GPON_HANDLER_LINK_LOSS,	(void *)Pon_llid_loss_handler, &Handler_id) != 0){
		sys_console_printf("register GPIO Changed Event handler error\r\n" );
		}
	
	if(gpon_assign_handler_function (GPON_HANDLER_GPIO_CHANGED,	(void *)Pon_gpio_changed_handler, &Handler_id) != 0){
		sys_console_printf("register GPIO Changed Event handler error\r\n" );
		}

	return( ROK );
}
#endif
int GW5201_pon_add_olt(short int PonPortIdx, short int PonChipVer)
{
	short int  PonChipIdx;
	int  Cardslot, CardPort;
	short int result = PAS_EXIT_ERROR;
	PON_add_olt_result_t add_olt_result= {{0}};
	PON_binary_t *firmware;
	
	int ret = -1;
		
	CHECK_PON_RANGE;

	PonChipIdx = GetPonChipIdx( PonPortIdx );
	Cardslot = GetCardIdxByPonChip(PonChipIdx);
	CardPort = GetPonPortByPonChip(PonChipIdx);
	
	if( PonChipVer == PONCHIP_PAS5001 )
	{
        firmware = &PAS_initialization_parameters_5001.firmware_image;
		set_image_src( PONCHIP_PAS5001, PON_BINARY_SOURCE_MEMORY, firmware, NULL );
        if ( (firmware->location != NULL) && (firmware->size > 0) )
        {
			result = PAS_add_olt_v4(PonPortIdx, PonChipMgmtTable[PonChipIdx].MgmtPathMAC, &PAS_initialization_parameters_5001);
        }
        else
        {
            sys_console_printf("PAS5001 failed to load for no firmware in the flash.\r\n");
            result = FIRMWARE_IS_ERROR;
        }
	}
	else 
	{
        firmware = &PAS_initialization_parameters_5201.firmware_image;
		set_image_src( PONCHIP_PAS5201, PON_BINARY_SOURCE_MEMORY, firmware, NULL );
        if ( (firmware->location != NULL) && (firmware->size > 0) )
        {
			/* added by chenfj 2007-7-19 
			            增加PON芯片下行是否有外部数据BUF判断*/		
			if ( getPonExtSdramSupported( Cardslot, CardPort ) == 1 )
				PAS_initialization_parameters_5201.external_downlink_buffer_size = PON_EXTERNAL_DOWNLINK_BUFFER_16MB;
			else
                PAS_initialization_parameters_5201.external_downlink_buffer_size = PON_EXTERNAL_DOWNLINK_BUFFER_0MB;
			
			VOS_MemCpy( &(PAS_initialization_parameters_5201.olt_mac_address), PonChipMgmtTable[PonChipIdx].MgmtPathMAC, BYTES_IN_MAC_ADDRESS );
			result = PAS_add_olt(PonPortIdx,  &PAS_initialization_parameters_5201, &add_olt_result);				
        }
        else
        {
            sys_console_printf("%s failed to load for no firmware in the flash.\r\n", pon_chip_type2name(PonChipVer));
            result = FIRMWARE_IS_ERROR;
        }
	}

    /* B--added by liwei056@2010-7-20 for Flash-VFS Support */
    pon_load_release(PonChipVer, 1, 0);
    /* E--added by liwei056@2010-7-20 for Flash-VFS Support */
	
	if(result != 0 )
	{
		VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  Download Firmware image to pon%d/%d ... Err(=%d)\r\n", Cardslot, CardPort, result);

        if ( FIRMWARE_LOAD_ERROR > result )
        {
            return result;
        }
        
        return FIRMWARE_LOAD_ERROR;
	}
	else
	{
	    /* 非配置类和业务类的本地必要设置，在这里可以自然保证设置的本地性和必要性。 */
#if 1
        if ( 0 == pon_local_olt(PonPortIdx, PonChipVer) )
        {
            OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) get the PON-Chip(%d/%d)'s LOCAL-PON service at the PonPortAdded time.\r\n", SYS_LOCAL_MODULE_SLOTNO, Cardslot, CardPort);
        }
#endif

        /* 下行: 关闭PON口光发射 */
        OLTAdv_SetOpticalTxMode2(PonPortIdx, FALSE, PONPORT_TX_ACTIVED);

        /* 上行: 禁止从CNI口学习MAC地址(D11358) */
        ResumeOltAddressTableConfig(PonPortIdx);

        /* B--added by liwei056@2010-12-2 for D11358 */
        /* 清空加载期间学习的MAC地址 */
        OLT_ResetAddrTbl(PonPortIdx, PON_ALL_ACTIVE_LLIDS, ADDR_DYNAMIC);
        OLT_ResetAddrTbl(PonPortIdx, PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID, ADDR_DYNAMIC);
        /* E--added by liwei056@2010-12-2 for D11358 */
        
		if(EVENT_PONADD == V2R1_ENABLE) 
		{
			unsigned char *pMac = NULL;
            
			sys_console_printf("\r\n  Download Firmware image to %s/port%d ... Ok\r\n", CardSlot_s[Cardslot], CardPort);			
			sys_console_printf("\r\n  the currently used data path mac address are (pon_add_olt())\r\n");
			if( PonChipVer == PONCHIP_PAS5001 )
			{
				pMac = PonChipMgmtTable[PonChipIdx].DataPathMAC;
			}
			else 
			{
				pMac = add_olt_result.olt_mac_address;
			}
			if( pMac )
				sys_console_printf("        %02x%02x.%02x%02x.%02x%02x\r\n", pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5]);
		}
        
        if(PON_ADD_TEST == V2R1_ENABLE)
            sys_console_printf(" pon %d SetPonCniPatameter\r\n",PonPortIdx);
        result = SetPonCniPatameter( PonPortIdx ); 
        if( result != PAS_EXIT_OK )
        {
            VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  error:pon%d/%d SetPonCniPatameter failed %d!\r\n", Cardslot, CardPort, result);
            return FIRMWARE_LOAD_ERROR;
        }
            
        /* 只需在PON板设置的与业务、配置均无关的必要设置 */
    	if( PonChipVer == PONCHIP_PAS5001 )
    	{
    		int  default_data;

            if(PON_ADD_TEST == V2R1_ENABLE)
                sys_console_printf(" pon %d update_Pon_parameters_5001 \r\n", PonPortIdx);
            result = update_Pon_parameters_5001(PonPortIdx );
            if(result != ROK)
            {
                VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  error:pon%d/%d update_Pon_parameters failed %d!\r\n", Cardslot, CardPort, result);
                result = update_Pon_parameters_5001(PonPortIdx );
            }

    		default_data = GetCniTbc(PonPortIdx);
    		if((default_data & 0x1) == 1 )
            {
                default_data -= 1;
            }
    		else
            {
                default_data += 1;
            }
    		SetCniTbc(PonPortIdx, (unsigned long )default_data );
    	}
        else
        {
            if(PON_ADD_TEST == V2R1_ENABLE)
                sys_console_printf(" pon %d update_Pon_parameters_5201 \r\n", PonPortIdx);

            result = update_Pon_parameters_5201( PonPortIdx, PON_VALUE_NOT_CHANGED);
            if( result != PAS_EXIT_OK )
            {
                VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  error:pon%d/%d update_Pon_parameters failed %d!\r\n", Cardslot, CardPort, result);
                return FIRMWARE_LOAD_ERROR;
            }

			if(PON_ADD_TEST == V2R1_ENABLE)
				sys_console_printf(" pon %d ConfigOltARPFilter\r\n", PonPortIdx);
            result = ConfigOltARPFilter(PonPortIdx );
            if( result != PAS_EXIT_OK )
            {
                VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  error:pon%d/%d ConfigOltARPFilter failed %d!\r\n", Cardslot, CardPort, result);
                return FIRMWARE_LOAD_ERROR;
            }

			result = ConfigOltDhcpFilter(PonPortIdx );
            if( result != PAS_EXIT_OK )
            {
                VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  error:pon%d/%d ConfigOltDhcpFilter failed %d!\r\n", Cardslot, CardPort, result);
            }

            {
                PON_high_priority_frames_t high_priority_frames;
                unsigned char i;

                for(i=0; i<= MAX_PRIORITY_QUEUE; i++)
                {
                    high_priority_frames.priority[i] = PON_priority_queue_mapping[i];
                }
                if(PON_ADD_TEST == V2R1_ENABLE)
                    sys_console_printf(" pon %d PAS_set_policing_thresholds\r\n", PonPortIdx);
                result = PAS_set_policing_thresholds( PonPortIdx,  PON_POLICER_DOWNSTREAM_TRAFFIC, high_priority_frames,  PON_POLICER_MAX_HIGH_PRIORITY_RESERVED );
                if( result != PAS_EXIT_OK )
                {
                    VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  error:pon%d/%d PAS_set_policing_thresholds() failed %d!\r\n", Cardslot, CardPort, result);
                }
            }
        }

#if 0
        {
			bool bCtcInit = FALSE;
            
            if((CTC_STACK_is_init( &bCtcInit ) == CTC_STACK_EXIT_OK) && bCtcInit )
            {
                SetPonFetchIGMPMsgToHostPath( PonPortIdx );
            }
        }
#endif

        if(PON_ADD_TEST == V2R1_ENABLE)
            sys_console_printf(" pon %d PAS_set_oam_configuration\r\n", PonPortIdx);
        result = PAS_set_oam_configuration( PonPortIdx, NO_OAM_LIMIT );
        if( result != PAS_EXIT_OK )
        {
			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  error:%s/port%d set oam No limit err %d(pon_add_olt())!\r\n", CardSlot_s[Cardslot], CardPort, result);
        }
	}

	Trap_FirmwareLoad( PonPortIdx, V2R1_LOAD_FILE_SUCCESS );

    return (ROK);
	
}

#if defined(_EPON_10G_PMC_SUPPORT_)            
int GW8411_pon_add_olt(short int PonPortIdx, short int PonChipVer)
{
	short int  PonChipIdx;
	int  Cardslot, CardPort;
	short int result = PAS_EXIT_ERROR;
	PON_add_olt_result_t add_olt_result= {{0}};
	PON_binary_t *firmware;
	
	int ret = -1;
		
	CHECK_PON_RANGE;

	PonChipIdx = GetPonChipIdx( PonPortIdx );
	Cardslot = GetCardIdxByPonChip(PonChipIdx);
	CardPort = GetPonPortByPonChip(PonChipIdx);
	
	{
		result = GW_set_CNI_interface_type(PonPortIdx);
		if(PAS_EXIT_OK != result)
		{
			pon_load_release(PonChipVer, 1, 0);
			sys_console_printf("GW_set_CNI_interface_type... Err(=%d)\r\n",result);
			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  GW_set_CNI_interface_type... Err(=%d)\r\n", result);
			return RERROR;
		}
		result = GW_set_ddr_configuration(PonPortIdx);
		if(PAS_EXIT_OK != result)
		{
			pon_load_release(PonChipVer, 1, 0);
			sys_console_printf("Set ddr configuration... Err(=%d)\r\n",result);
			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  Set ddr configuration... Err(=%d)\r\n", result);
			return RERROR;
		}
		result = GW_set_ref_clock(PonPortIdx);
		if(PAS_EXIT_OK != result)
		{
			pon_load_release(PonChipVer, 1, 0);
			sys_console_printf("Set ref clock... Err(=%d)\r\n",result);
			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  Set ref clock... Err(=%d)\r\n", result);
			return RERROR;
		}
		result = GW_set_olt_ne_mode(PonPortIdx);
		if(PAS_EXIT_OK != result)
		{
			pon_load_release(PonChipVer, 1, 0);
			sys_console_printf("Set ne mode... Err(=%d)\r\n",result);
			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  Set ne mode... Err(=%d)\r\n", result);
			return RERROR;
		}
		#if 1
		result = GW_set_olt_flavor_edk_type(PonPortIdx);
		if(PAS_EXIT_OK != result)
		{
			pon_load_release(PonChipVer, 1, 0);
			sys_console_printf("Set flavor edk type... Err(=%d)\r\n",result);
			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  Set flavor edk type... Err(=%d)\r\n", result);
			return RERROR;
		}
		#endif
		firmware = &PAS_initialization_parameters_8411.firmware_image;
		set_image_src( PonChipVer, PON_BINARY_SOURCE_MEMORY, firmware,NULL);
		if ( (firmware->location != NULL) && (firmware->size > 0) )
		{
			VOS_MemCpy( &(PAS_initialization_parameters_8411.olt_mac_address), PonChipMgmtTable[PonChipIdx].MgmtPathMAC, BYTES_IN_MAC_ADDRESS );
			result = GW10G_PAS_add_olt(PonPortIdx,  &PAS_initialization_parameters_8411, &add_olt_result);	
		}
	}

    /* B--added by liwei056@2010-7-20 for Flash-VFS Support */
    pon_load_release(PonChipVer, 1, 0);
    /* E--added by liwei056@2010-7-20 for Flash-VFS Support */
	
	if(result != PAS_EXIT_OK )
	{
		VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  Download Firmware image to pon%d/%d ... Err(=%d)\r\n", Cardslot, CardPort, result);
        return -1;
	}
	else
	{
	    /* 非配置类和业务类的本地必要设置，在这里可以自然保证设置的本地性和必要性。 */
#if 1
        if ( 0 == pon_local_olt(PonPortIdx, PonChipVer) )
        {
            OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) get the PON-Chip(%d/%d)'s LOCAL-PON service at the PonPortAdded time.\r\n", SYS_LOCAL_MODULE_SLOTNO, Cardslot, CardPort);
        }
#endif

#if 1/*deleted by jinhl @2012-09-17*/
		result = GW_Set_GPIO_AccessInit(PonPortIdx);
		if(PAS_EXIT_OK != result)
		{
			pon_load_release(PonChipVer, 1, 0);
			sys_console_printf("GW_Set_GPIO_AccessInit... Err(=%d)\r\n",result);
			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  GW_Set_GPIO_AccessInit... Err(=%d)\r\n", result);
			return RERROR;
		}
		OLT_ADD_DEBUG(OLT_ADD_TITLE"GW_Set_GPIO_AccessInit ok.\r\n");
#endif
        /*如下Set_TMSchedMode、TMOperation_CommitSched、TMOperation_TrafficEnable、set_olt_device_optics
			必须设置后 才能进行ONU的注册*/
		result = GW_Set_TMSchedMode(PonPortIdx, TM_SCHED_SIMPLE);
		if(PAS_EXIT_OK != result)
		{
			pon_load_release(PonChipVer, 1, 0);
			sys_console_printf("GW_Set_TMSchedMode... Err(=%d)\r\n",result);
			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  GW_Set_TMSchedMode... Err(=%d)\r\n", result);
			return RERROR;
		}
		OLT_ADD_DEBUG(OLT_ADD_TITLE"GW_Set_TMSchedMode ok.\r\n");

		result = GW_TMOperation_CommitSched(PonPortIdx);
		if(PAS_EXIT_OK != result)
		{
			pon_load_release(PonChipVer, 1, 0);
			sys_console_printf("GW_TMOperation_CommitSched... Err(=%d)\r\n",result);
			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  GW_TMOperation_CommitSched... Err(=%d)\r\n", result);
			return RERROR;
		}
		OLT_ADD_DEBUG(OLT_ADD_TITLE"GW_TMOperation_CommitSched ok.\r\n");

		result = GW_TMOperation_TrafficEnable(PonPortIdx);
		if(PAS_EXIT_OK != result)
		{
			pon_load_release(PonChipVer, 1, 0);
			sys_console_printf("GW_TMOperation_TrafficEnable... Err(=%d)\r\n",result);
			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  GW_TMOperation_TrafficEnable... Err(=%d)\r\n", result);
			return RERROR;
		}
		OLT_ADD_DEBUG(OLT_ADD_TITLE"GW_TMOperation_TrafficEnable ok.\r\n");

		result = GW_set_olt_device_optics(PonPortIdx, PON_10G);
		if(PAS_EXIT_OK != result)
		{
			pon_load_release(PonChipVer, 1, 0);
			sys_console_printf("GW_set_olt_device_optics... Err(=%d)\r\n",result);
			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  GW_set_olt_device_optics... Err(=%d)\r\n", result);
			return RERROR;
		}
		OLT_ADD_DEBUG(OLT_ADD_TITLE"GW_set_olt_device_optics ok.no PAS_set_olt_device_optics_by_module_and_connectivity\r\n");
        /* 下行: 关闭PON口光发射 */
		/*PAS_set_olt_pon_transmission( PonPortIdx, FALSE );*/

        /* 上行: 禁止从CNI口学习MAC地址(D11358) */
        ResumeOltAddressTableConfig(PonPortIdx);

        /* B--added by liwei056@2010-12-2 for D11358 */
        /* 清空加载期间学习的MAC地址 */
        OLT_ResetAddrTbl(PonPortIdx, PON_ALL_ACTIVE_LLIDS, ADDR_DYNAMIC);
        OLT_ResetAddrTbl(PonPortIdx, PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID, ADDR_DYNAMIC);
        /* E--added by liwei056@2010-12-2 for D11358 */
        
		if(EVENT_PONADD == V2R1_ENABLE) 
		{
			unsigned char *pMac = NULL;
            
			sys_console_printf("\r\n  Download Firmware image to %s/port%d ... Ok\r\n", CardSlot_s[Cardslot], CardPort);			
			sys_console_printf("\r\n  the currently used data path mac address are (pon_add_olt())\r\n");
			
			pMac = add_olt_result.olt_mac_address;
			
			if( pMac )
				sys_console_printf("        %02x%02x.%02x%02x.%02x%02x\r\n", pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5]);
		}
        
        if(PON_ADD_TEST == V2R1_ENABLE)
            sys_console_printf(" pon %d SetPonCniPatameter\r\n",PonPortIdx);
        result = SetPonCniPatameter( PonPortIdx ); 
        if( result != PAS_EXIT_OK )
        {
            VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  error:pon%d/%d SetPonCniPatameter failed %d!\r\n", Cardslot, CardPort, result);
            return -1;
        }
            
        /* 只需在PON板设置的与业务、配置均无关的必要设置 */
    	
        {
            if(PON_ADD_TEST == V2R1_ENABLE)
                sys_console_printf(" pon %d update_Pon_parameters_8411 \r\n", PonPortIdx);

            result = update_Pon_parameters_8411( PonPortIdx, PON_VALUE_NOT_CHANGED);
            if( result != PAS_EXIT_OK )
            {
                VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  error:pon%d/%d update_Pon_parameters failed %d!\r\n", Cardslot, CardPort, result);
                return -1;
            }

			if(PON_ADD_TEST == V2R1_ENABLE)
				sys_console_printf(" pon %d ConfigOltARPFilter\r\n", PonPortIdx);
            result = ConfigOltARPFilter(PonPortIdx );
            if( result != PAS_EXIT_OK )
            {
                VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  error:pon%d/%d ConfigOltARPFilter failed %d!\r\n", Cardslot, CardPort, result);
                return -1;
            }

            {
                PON_high_priority_frames_t high_priority_frames;
                unsigned char i;

                for(i=0; i<= MAX_PRIORITY_QUEUE; i++)
                {
                    high_priority_frames.priority[i] = PON_priority_queue_mapping[i];
                }
                if(PON_ADD_TEST == V2R1_ENABLE)
                    sys_console_printf(" pon %d PAS_set_policing_thresholds\r\n", PonPortIdx);
                result = GW10GAdp_set_policing_thresholds( PonPortIdx,  PON_POLICER_DOWNSTREAM_TRAFFIC, high_priority_frames,  PON_POLICER_MAX_HIGH_PRIORITY_RESERVED );
                if( result != PAS_EXIT_OK )
                {
                    VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  error:pon%d/%d PAS_set_policing_thresholds() failed %d!\r\n", Cardslot, CardPort, result);
                }
            }
        }



        if(PON_ADD_TEST == V2R1_ENABLE)
            sys_console_printf(" pon %d PAS_set_oam_configuration\r\n", PonPortIdx);
        result = GW10G_PAS_set_oam_configuration( PonPortIdx, NO_OAM_LIMIT );
        if( result != PAS_EXIT_OK )
        {
			VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  error:%s/port%d set oam No limit err %d(pon_add_olt())!\r\n", CardSlot_s[Cardslot], CardPort, result);
        }
	}

	Trap_FirmwareLoad( PonPortIdx, V2R1_LOAD_FILE_SUCCESS );

    	
	return (ROK);
	
}
#endif

int TK3723_pon_add_olt(short int PonPortIdx, short int PonChipVer)
{
    int ret;
	int  Cardslot, CardPort;
	short int  PonChipIdx;
    PON_binary_t *firmware;
    char *firmware_version;
    TK_olt_initialization_parameters_t add_olt_params;
    TK_add_olt_result_t add_olt_result;

	PonChipIdx = GetPonChipIdx( PonPortIdx );
	Cardslot = GetCardIdxByPonChip(PonChipIdx);
	CardPort = GetPonPortByPonChip(PonChipIdx);
    
    firmware = &PAS_initialization_parameters_5201.firmware_image;
    firmware_version = add_olt_params.firmware_version;    
    firmware_version[0] = '\0';
    ret = 0;

    VOS_MemCpy(&add_olt_params, &TK_initialization_parameters_55524, sizeof(TK_olt_initialization_parameters_t));
    if ( 0 == PonChipDownloadCounter[PonPortIdx] )
    {
        /* 第一次加载，只带版本号 */
		set_image_src( PonChipVer, PON_BINARY_SOURCE_MEMORY, NULL, firmware_version );
        if ( '\0' == firmware_version[0] )
        {
            sys_console_printf("%s failed to load for no firmware in the flash.\r\n", pon_chip_type2name(PonChipVer));
            ret = FIRMWARE_IS_ERROR;
        }
    }
    else
    {
        /* 第一次加载失败，则需带Firmware数据 */
		set_image_src( PonChipVer, PON_BINARY_SOURCE_MEMORY, firmware, firmware_version );
        if ( (firmware->location == NULL) || (firmware->size <= 0) )
        {
            sys_console_printf("%s failed to load for no firmware in the flash.\r\n", pon_chip_type2name(PonChipVer));
            ret = FIRMWARE_IS_ERROR;
        }
    }

    if ( 0 == ret )
    {
        add_olt_params.firmware_location = firmware->location;
        add_olt_params.firmware_size = firmware->size;
		VOS_MemCpy( &(add_olt_params.olt_mac_address), PonChipMgmtTable[PonChipIdx].MgmtPathMAC, BYTES_IN_MAC_ADDRESS );

    	ret = TK_add_olt(PonPortIdx, &add_olt_params, &add_olt_result);				

        if ( firmware->location != NULL )
        {
            /* B--added by liwei056@2010-7-20 for Flash-VFS Support */
            pon_load_release(PonChipVer, 1, 0);
            /* E--added by liwei056@2010-7-20 for Flash-VFS Support */
        }
    }

	if ( 0 == ret )
	{
	    if ( 0 != VOS_MemCmp(PonChipMgmtTable[PonChipIdx].MgmtPathMAC, add_olt_result.olt_mac_address, 6) )
        {
            /* PON芯片的管理MAC不可设置，则需回写 */
            /* 【可以保证GW OAM正常，但对环路检测会有影响】 */
            SetPonchipMgmtPathMac2(PonPortIdx, add_olt_result.olt_mac_address);
        }      

	    /* 非配置类和业务类的本地必要设置，在这里可以自然保证设置的本地性和必要性。 */
        if ( 0 == pon_local_olt(PonPortIdx, PonChipVer) )
        {
            OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) get the PON-Chip(%d/%d)'s LOCAL-PON service at the PonPortAdded time.\r\n", SYS_LOCAL_MODULE_SLOTNO, Cardslot, CardPort);
        }

        /* 下行: 关闭PON口光发射 */
        OLTAdv_SetOpticalTxMode2(PonPortIdx, FALSE, PONPORT_TX_ACTIVED);

        /* 上行: 禁止从CNI口学习MAC地址(D11358) */
       	ResumeOltAddressTableConfig(PonPortIdx);
		
        /* 此时，尚未创建MAC表，则无需清空加载期间学习的MAC地址 */
        NULL;

		Trap_FirmwareLoad( PonPortIdx, V2R1_LOAD_FILE_SUCCESS );
		Trap_DBALoad( PonPortIdx, V2R1_LOAD_FILE_SUCCESS );
    }
	else
    {
		VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  Download Firmware image to pon%d/%d ... Err(=%d)\r\n", Cardslot, CardPort, ret);

        switch ( ret )
        {
            case TK_OLT_NOT_EXIST:
                return OLT_ERR_NOTREADY;
            case TK_ERROR_NEED_FILE:
                return FIRMWARE_IS_NEED;
            case TK_ERROR_BAD_FILE:
                return FIRMWARE_IS_ERROR;
            default:   
                if ( FIRMWARE_LOAD_ERROR > ret )
                {
                    return ret;
                }
                
                return FIRMWARE_LOAD_ERROR;
        }
	} 
    
	/*2 set status to 休眠*/
	PonChipMgmtTable[PonChipIdx].operStatus = PONCHIP_DORMANT;
	/*PonPortTable[olt_id].PortWorkingStatus = PONPORT_UP;*/

    /* B--added by liwei056@2011-1-11 for SyncMasterStatus */
    if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
    {
        OLT_SYNC_Data(PonPortIdx, FC_ADD_PONPORT, NULL, 0);
    }
    /* E--added by liwei056@2011-1-11 for SyncMasterStatus */
	
	/*2' startup and activate the external DBA  */
	if( PonChipActivatedFlag[Cardslot] == TRUE )
	{
		PonPortActivated( PonPortIdx );
	}

    return (ROK);
}

#if defined(_EPON_10G_BCM_SUPPORT_)            
int BCM55538_pon_add_olt(short int PonPortIdx, short int PonChipVer)
{
    int ret;
	int  Cardslot, CardPort;
	short int  PonChipIdx;
    PON_binary_t *firmware;
    char *firmware_version;
    TK_olt_initialization_parameters_t add_olt_params;
    TK_add_olt_result_t add_olt_result;

	PonChipIdx = GetPonChipIdx( PonPortIdx );
	Cardslot = GetCardIdxByPonChip(PonChipIdx);
	CardPort = GetPonPortByPonChip(PonChipIdx);
    
    firmware = &PAS_initialization_parameters_5201.firmware_image;
    firmware_version = add_olt_params.firmware_version;
    firmware_version[0] = '\0';
    ret = 0;

    VOS_MemCpy(&add_olt_params, &TK_initialization_parameters_55524, sizeof(TK_olt_initialization_parameters_t));
    if ( 0 == PonChipDownloadCounter[PonPortIdx] )
    {
        /* 第一次加载，只带版本号 */
		set_image_src( PonChipVer, PON_BINARY_SOURCE_MEMORY, NULL, firmware_version );
        if ( '\0' == firmware_version[0] )
        {
            sys_console_printf("%s failed to load for no firmware in the flash.\r\n", pon_chip_type2name(PonChipVer));
#if 0
            ret = FIRMWARE_IS_ERROR;
#else
            ret = 0;
#endif
        }
    }
    else
    {
        /* 第一次加载失败，则需带Firmware数据 */
		set_image_src( PonChipVer, PON_BINARY_SOURCE_MEMORY, firmware, firmware_version );
        if ( (firmware->location == NULL) || (firmware->size <= 0) )
        {
            sys_console_printf("%s failed to load for no firmware in the flash.\r\n", pon_chip_type2name(PonChipVer));
            ret = FIRMWARE_IS_ERROR;
        }
    }

    if ( 0 == ret )
    {
        add_olt_params.firmware_location = firmware->location;
        add_olt_params.firmware_size = firmware->size;
		VOS_MemCpy( &(add_olt_params.olt_mac_address), PonChipMgmtTable[PonChipIdx].MgmtPathMAC, BYTES_IN_MAC_ADDRESS );

    	ret = BCM_add_olt(PonPortIdx, &add_olt_params, &add_olt_result);				

        if ( firmware->location != NULL )
        {
            /* B--added by liwei056@2010-7-20 for Flash-VFS Support */
            pon_load_release(PonChipVer, 1, 0);
            /* E--added by liwei056@2010-7-20 for Flash-VFS Support */
        }
    }

	if ( 0 == ret )
	{
	    if ( 0 != VOS_MemCmp(PonChipMgmtTable[PonChipIdx].MgmtPathMAC, add_olt_result.olt_mac_address, 6) )
        {
            /* PON芯片的管理MAC不可设置，则需回写 */
            /* 【可以保证GW OAM正常，但对环路检测会有影响】 */
            SetPonchipMgmtPathMac2(PonPortIdx, add_olt_result.olt_mac_address);
        }      

	    /* 非配置类和业务类的本地必要设置，在这里可以自然保证设置的本地性和必要性。 */
        if ( 0 == pon_local_olt(PonPortIdx, PonChipVer) )
        {
            OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) get the PON-Chip(%d/%d)'s LOCAL-PON service at the PonPortAdded time.\r\n", SYS_LOCAL_MODULE_SLOTNO, Cardslot, CardPort);
        }

        /* 下行: 关闭PON口光发射 */
        OLTAdv_SetOpticalTxMode2(PonPortIdx, FALSE, PONPORT_TX_ACTIVED);

        /* 上行: 禁止从CNI口学习MAC地址(D11358) */
        ResumeOltAddressTableConfig(PonPortIdx);

        /* 此时，尚未创建MAC表，则无需清空加载期间学习的MAC地址 */
        NULL;

		Trap_FirmwareLoad( PonPortIdx, V2R1_LOAD_FILE_SUCCESS );
		Trap_DBALoad( PonPortIdx, V2R1_LOAD_FILE_SUCCESS );

        /* 需要通知OLT加入事件 */
        Pon_event_handler(PON_EVT_HANDLER_OLT_ADD, DATA2PTR(PonPortIdx));
    }
	else
    {
		VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  Download Firmware image to pon%d/%d ... Err(=%d)\r\n", Cardslot, CardPort, ret);

        switch ( ret )
        {
            case TK_OLT_NOT_EXIST:
                return OLT_ERR_NOTREADY;
            case TK_ERROR_NEED_FILE:
                return FIRMWARE_IS_NEED;
            case TK_ERROR_BAD_FILE:
                return FIRMWARE_IS_ERROR;
            default:   
                if ( FIRMWARE_LOAD_ERROR > ret )
                {
                    return ret;
                }
                
                return FIRMWARE_LOAD_ERROR;
        }
	} 
    
	/*2 set status to 休眠*/
	PonChipMgmtTable[PonChipIdx].operStatus = PONCHIP_DORMANT;
	/*PonPortTable[olt_id].PortWorkingStatus = PONPORT_UP;*/

    /* B--added by liwei056@2011-1-11 for SyncMasterStatus */
    if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
    {
        OLT_SYNC_Data(PonPortIdx, FC_ADD_PONPORT, NULL, 0);
    }
    /* E--added by liwei056@2011-1-11 for SyncMasterStatus */
	
	/*2' startup and activate the external DBA  */
	if( PonChipActivatedFlag[Cardslot] == TRUE )
	{
		PonPortActivated( PonPortIdx );
	}

    return (ROK);
}
#endif

int gGponFwWrPri = 30;
#if defined(_GPON_BCM_SUPPORT_)
int GPON_pon_add_olt(short int PonPortIdx, short int PonChipVer)
{
    int ret = 0;
	int  Cardslot = 0, CardPort = 0;
	short int  PonChipIdx = 0;
    PON_binary_t *firmware = NULL;
    char *firmware_version = NULL;
    GPON_olt_initialization_parameters_t add_olt_params;
    GPON_add_olt_result_t add_olt_result;
	LONG lTid = 0;
 	 LONG lPriority = 180;
	 LONG newPri = 0;

	VOS_MemSet(&add_olt_result, 0, sizeof(GPON_add_olt_result_t));
	PonChipIdx = GetPonChipIdx( PonPortIdx );
	Cardslot = GetCardIdxByPonChip(PonChipIdx);
	CardPort = GetPonPortByPonChip(PonChipIdx);
    
    firmware = &PAS_initialization_parameters_5201.firmware_image;
    firmware_version = add_olt_params.firmware_version;    
    firmware_version[0] = '\0';
    ret = 0;
	
    VOS_MemCpy(&add_olt_params, &GPON_initialization_parameters, sizeof(GPON_olt_initialization_parameters_t));
    /*68620 16 GPON不支持spi flash自启动，只能外部启动*/
    {
        /*GPON在读FW及解析FW之前提高优先级，解析完成后再设回 by jinhl@2016.11.01*/
		lTid = VOS_TaskIdSelf();
		VOS_TaskGetPriority( lTid, &lPriority );
		VOS_TaskSetPriority( lTid, gGponFwWrPri);
		set_image_src( PonChipVer, PON_BINARY_SOURCE_MEMORY, firmware, firmware_version );
        if ( (firmware->location == NULL) || (firmware->size <= 0) )
        {
            sys_console_printf("%s failed to load for no firmware in the flash.\r\n", pon_chip_type2name(PonChipVer));
            ret = FIRMWARE_IS_ERROR;
        }
		/*GPON在读FW及解析FW之前提高优先级，解析完成后再设回 by jinhl@2016.11.01*/
		VOS_TaskSetPriority( lTid, lPriority);
    }

    if ( 0 == ret )
    {
        add_olt_params.firmware_location = firmware->location;
        add_olt_params.firmware_size = firmware->size;
		VOS_MemCpy( &(add_olt_params.olt_mac_address), PonChipMgmtTable[PonChipIdx].MgmtPathMAC, BYTES_IN_MAC_ADDRESS );

    	ret = gpon_add_olt(PonPortIdx, &add_olt_params, &add_olt_result);				

        if ( firmware->location != NULL )
        {
            /* B--added by liwei056@2010-7-20 for Flash-VFS Support */
            pon_load_release(PonChipVer, 1, 0);
            /* E--added by liwei056@2010-7-20 for Flash-VFS Support */
        }
    }

	if ( 0 == ret )
	{
	    if ( 0 != VOS_MemCmp(PonChipMgmtTable[PonChipIdx].MgmtPathMAC, add_olt_result.olt_mac_address, 6) )
        {
            /* PON芯片的管理MAC不可设置，则需回写 */
            /* 【可以保证GW OAM正常，但对环路检测会有影响】 */
            SetPonchipMgmtPathMac2(PonPortIdx, add_olt_result.olt_mac_address);
        }      

	    /* 非配置类和业务类的本地必要设置，在这里可以自然保证设置的本地性和必要性。 */
        if ( 0 == pon_local_olt(PonPortIdx, PonChipVer) )
        {
            OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) get the PON-Chip(%d/%d)'s LOCAL-PON service at the PonPortAdded time.\r\n", SYS_LOCAL_MODULE_SLOTNO, Cardslot, CardPort);
        }

        /* 下行: 关闭PON口光发射 */
        OLTAdv_SetOpticalTxMode2(PonPortIdx, FALSE, PONPORT_TX_ACTIVED);

        /* 上行: 禁止从CNI口学习MAC地址(D11358) */
       	ResumeOltAddressTableConfig(PonPortIdx);
		
        /* 此时，尚未创建MAC表，则无需清空加载期间学习的MAC地址 */
        NULL;

		Trap_FirmwareLoad( PonPortIdx, V2R1_LOAD_FILE_SUCCESS );
		Trap_DBALoad( PonPortIdx, V2R1_LOAD_FILE_SUCCESS );
    }
	else
    {
		VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  Download Firmware image to pon%d/%d ... Err(=%d)\r\n", Cardslot, CardPort, ret);

        switch ( ret )
        {
            case TK_OLT_NOT_EXIST:
                return OLT_ERR_NOTREADY;
            case TK_ERROR_NEED_FILE:
                return FIRMWARE_IS_NEED;
            case TK_ERROR_BAD_FILE:
                return FIRMWARE_IS_ERROR;
            default:   
                if ( FIRMWARE_LOAD_ERROR > ret )
                {
                    return ret;
                }
                
                return FIRMWARE_LOAD_ERROR;
        }
	} 
    
	/*2 set status to 休眠*/
	PonChipMgmtTable[PonChipIdx].operStatus = PONCHIP_DORMANT;
	/*PonPortTable[olt_id].PortWorkingStatus = PONPORT_UP;*/

    /* B--added by liwei056@2011-1-11 for SyncMasterStatus */
    if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
    {
        OLT_SYNC_Data(PonPortIdx, FC_ADD_PONPORT, NULL, 0);
    }
    /* E--added by liwei056@2011-1-11 for SyncMasterStatus */
	
	/*2' startup and activate the external DBA  */
	if( PonChipActivatedFlag[Cardslot] == TRUE )
	{
		PonPortActivated( PonPortIdx );
	}

    return (ROK);
}
#endif
/* added by xieshl 20151224,  EPON/GPON统一最大带宽获取接口，问题单28412、28411 */
LONG Pon_GetSupportMaxBw( ULONG PonPortIdx, ULONG *pUBW, ULONG *pDBW )
{
	int iPonType = PONPORTTYPEUNKNOWN;
	CHECK_PON_RANGE;
	if( (pUBW == NULL) || (pDBW == NULL) )
		return VOS_ERROR;
	iPonType = GetPonPortType(PonPortIdx);
	if(iPonType == RERROR) iPonType = PONPORTTYPEUNKNOWN;
	switch (iPonType)
	{
		case EPONMAUTYPE1000BASEPX10DOLT:
		case EPONMAUTYPE1000BASEPX10UOLT:
		case EPONMAUTYPE1000BASEPX10DONU:
		case EPONMAUTYPE1000BASEPX10UONU:
		case EPONMAUTYPE1000BASEPX20DOLT:
		case EPONMAUTYPE1000BASEPX20UOLT:
		case EPONMAUTYPE1000BASEPX20DONU:
		case EPONMAUTYPE1000BASEPX20UONU:
		case EPONMAUTYPE1000BASEPXOLT:
		case EPONMAUTYPE1000BASEPXONU:
			*pUBW = DEFAULT_UP_BW;
			*pDBW = DEFAULT_DOWN_BW;
			break;
		case EPONMAUTYPE10GBASEPR30DOLT:
		case EPONMAUTYPE10GBASEPR30UONU:
			*pUBW = GW10G_DEFAULT_UP_BW;
			*pDBW = GW10G_DEFAULT_DOWN_BW;
			break;
		case EPONMAUTYPE10GBASEPRX30DOLT:
		case EPONMAUTYPE10GBASEPRX30UONU:
			*pUBW = DEFAULT_UP_BW;
			*pDBW = GW10G_DEFAULT_DOWN_BW;
			break;
		case GPONTYPE2G1GDOLT:
		case GPONTYPE2G1GUONU:
			*pUBW = DEFAULT_UP_BW;
			*pDBW = GW2G_DEFAULT_DOWN_BW;
			break;
		case GPONTYPE2G2GDOLT:
		case GPONTYPE2G2GUONU:
			*pUBW = GW2G_DEFAULT_UP_BW;
			*pDBW = GW2G_DEFAULT_DOWN_BW;
			break;
		default:
			*pUBW = DEFAULT_UP_BW;
			*pDBW = DEFAULT_DOWN_BW;
			break;
	}
	return VOS_OK;
}
int Pon_InitDefaultMaxBW(short int PonPortIdx)
{
	CHECK_PON_RANGE

	OLT_ADD_DEBUG(OLT_ADD_TITLE"Pon_InitDefaultMaxBW, PonPortIdx = %d \r\n",PonPortIdx);
	ULONG ubw = DEFAULT_UP_BW, dbw = DEFAULT_DOWN_BW;

	Pon_GetSupportMaxBw( PonPortIdx, &ubw, &dbw );
	
	PonPortTable[PonPortIdx].MaxBW = ubw;
	PonPortTable[PonPortIdx].DownlinkMaxBw = dbw;

	PonPortTable[PonPortIdx].RemainBW = PonPortTable[PonPortIdx].MaxBW;
	PonPortTable[PonPortIdx].DownlinkRemainBw = PonPortTable[PonPortIdx].DownlinkMaxBw;
	
	return VOS_OK;
}

#if defined(_EPON_10G_PMC_SUPPORT_)            
int GW_set_ddr_configuration(const PON_olt_id_t olt_id)
{
	int i = 0;
	int ret = 0;
	
	PON_ddr_parameters_s		ddr_params ;
    sys_console_printf("GW_set_ddr_configuration,PON_DDR_RAM_SW_TEST\r\n");
	VOS_MemSet(&ddr_params, 0, sizeof(PON_ddr_parameters_s));
	ddr_params.ddr_size = PON_DDR_SIZE_1024_MBITS;
	ddr_params.mt = PON_DDR_RATE_1333;
	ddr_params.bus_configuration = PON_DDR_BUS_WIDTH_16;
	ddr_params.cas_latency = 9;
	ddr_params.trp_delay = 9;
	ddr_params.trcd_delay = 9;
	ddr_params.trc_delay = 33;
	ddr_params.ecc = FALSE;
	ddr_params.ram_test = PON_DDR_RAM_SW_TEST;

	ddr_params.lane_tof_params.cpu_ddr_lane_tof_params[0].tof_ck = 318;
	ddr_params.lane_tof_params.cpu_ddr_lane_tof_params[0].tof_dqs= 240;	
	ddr_params.lane_tof_params.cpu_ddr_lane_tof_params[1].tof_ck = 318; 
    ddr_params.lane_tof_params.cpu_ddr_lane_tof_params[1].tof_dqs = 239; 
    ddr_params.lane_tof_params.cpu_ddr_lane_tof_params[2].tof_ck = 110; 
    ddr_params.lane_tof_params.cpu_ddr_lane_tof_params[2].tof_dqs = 157; 

	
	ret = GW10G_PAS_set_ddr_configuration(olt_id, PON_DDR_TYPE_CPU, &ddr_params);
    if ( ret != PAS_EXIT_OK ) 
    { 
        sys_console_printf("%s %d return error(error code : %d)\n", __FUNCTION__,__LINE__,ret); 
    } 

    VOS_MemSet(&ddr_params, 0, sizeof(PON_ddr_parameters_s));
	ddr_params.ddr_size = PON_DDR_SIZE_1024_MBITS;
	ddr_params.mt = PON_DDR_RATE_1333;
	ddr_params.bus_configuration = PON_DDR_BUS_WIDTH_64;
	ddr_params.cas_latency = 9;
	ddr_params.trp_delay = 9;
	ddr_params.trcd_delay = 9;
	ddr_params.trc_delay = 33;
	ddr_params.ecc = FALSE;
	ddr_params.ram_test = PON_DDR_RAM_SW_TEST;
	ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[0].tof_ck = 402; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[0].tof_dqs = 308; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[1].tof_ck = 402; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[1].tof_dqs = 217; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[2].tof_ck = 557; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[2].tof_dqs = 303; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[3].tof_ck = 557; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[3].tof_dqs = 282; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[4].tof_ck = 690; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[4].tof_dqs = 169; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[5].tof_ck = 690; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[5].tof_dqs = 260; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[6].tof_ck = 819; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[6].tof_dqs = 236; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[7].tof_ck = 819; 
    ddr_params.lane_tof_params.packet_buffer_ddr_lane_tof_params[7].tof_dqs = 178; 

	ret = GW10G_PAS_set_ddr_configuration(olt_id, PON_DDR_TYPE_PACKET_BUFFER, &ddr_params);
	if ( ret != PAS_EXIT_OK ) 
    { 
        sys_console_printf("%s %d return error(error code : %d)\n", __FUNCTION__,__LINE__,ret); 
    } 
	
	return ret;
}
int GW_get_ddr_configuration(const PON_olt_id_t olt_id)
{
	int ret = 0;
	PON_ddr_parameters_s		ddr_params = {0};
	ret = GW10G_PAS_get_ddr_configuration(olt_id, PON_DDR_TYPE_CPU, &ddr_params);
	return ret;
}



int GW_set_ref_clock(const PON_olt_id_t olt_id)
{
	int ret = 0;

	ret = GW10G_PAS_set_ref_clock(olt_id, PON_REF_CLOCK_125_MHZ_SINGLE_ENDED);
    sys_console_printf("PON_REF_CLOCK_125_MHZ_SINGLE_ENDED\r\n");
	return ret;
}
int GW_get_ref_clock(const PON_olt_id_t olt_id)
{
	int ret = 0;
    int ref_clock = PON_REF_CLOCK_NOT_INITIALIZED;
	ret = GW10G_PAS_get_ref_clock(olt_id, &ref_clock);

	return ret;
}

int GW_set_olt_ne_mode(const PON_olt_id_t olt_id)
{
	int ret = 0;

	ret = GW10G_PAS_set_olt_ne_mode(olt_id,NE_MODE_DEFAULT);

	return ret;
}

int GW_get_olt_ne_mode(const PON_olt_id_t olt_id)
{
	int ret = 0;
    int mode = 0;
	ret = GW10G_PAS_get_olt_ne_mode(olt_id,&mode);

	return ret;
}

int GW_set_olt_flavor_edk_type(const PON_olt_id_t olt_id)
{
	int ret = 0;
    
	ret =GW10G_PAS_set_olt_flavor_edk_type(olt_id,GW10G_PON_EDK_TYPE_1);

	return ret;
}

int GW_set_gpio_group_mapping(const PON_olt_id_t olt_id, const PON_gpio_grp_type_e gpio_grp_type, const GW10G_PON_optics_module_type_e module_type)
{
	int ret = PAS_EXIT_OK;
	PON_gpio_grp_map_value_u gpio_mapval;
	memset(&gpio_mapval, 0, sizeof(PON_gpio_grp_map_value_u));
	switch(gpio_grp_type)
	{
		case PON_GPIO_GRP_TYPE_10G_PON_E:
			if( LIGENT_OPTICS_LTX5302B == module_type)
			{
				gpio_mapval.gpio_10G_pon_map.module_exist = 38;
				gpio_mapval.gpio_10G_pon_map.module_exist_polarity = 0;
				gpio_mapval.gpio_10G_pon_map.i2c_data = 40;
				gpio_mapval.gpio_10G_pon_map.i2c_clock = 41;
				gpio_mapval.gpio_10G_pon_map.mod_desel = 29;
				gpio_mapval.gpio_10G_pon_map.tx_1G_disable = 28;
				gpio_mapval.gpio_10G_pon_map.tx_1G_disable_polarity = 0;
				gpio_mapval.gpio_10G_pon_map.tx_10G_disable = 30;
				gpio_mapval.gpio_10G_pon_map.tx_10G_disable_polarity = 0;
				gpio_mapval.gpio_10G_pon_map.sqelch = 26;
				gpio_mapval.gpio_10G_pon_map.sqelch_polarity = 0;
				gpio_mapval.gpio_10G_pon_map.rx_loss = 27;
			
				ret = GW10G_PAS_set_gpio_group_mapping (olt_id,gpio_grp_type,PON_GPIO_GRP_LEVEL_PRIMARY_E, &gpio_mapval);

				gpio_mapval.gpio_10G_pon_map.module_exist = 39;
				gpio_mapval.gpio_10G_pon_map.module_exist_polarity = 0;
				gpio_mapval.gpio_10G_pon_map.i2c_data = 40;
				gpio_mapval.gpio_10G_pon_map.i2c_clock = 41;
				gpio_mapval.gpio_10G_pon_map.mod_desel = 34;
				gpio_mapval.gpio_10G_pon_map.tx_1G_disable = 33;
				gpio_mapval.gpio_10G_pon_map.tx_1G_disable_polarity = 0;
				gpio_mapval.gpio_10G_pon_map.tx_10G_disable = 35;
				gpio_mapval.gpio_10G_pon_map.tx_10G_disable_polarity = 0;
				gpio_mapval.gpio_10G_pon_map.sqelch = 31;
				gpio_mapval.gpio_10G_pon_map.sqelch_polarity = 0;
				gpio_mapval.gpio_10G_pon_map.rx_loss = 32;
			
				ret = GW10G_PAS_set_gpio_group_mapping (olt_id,gpio_grp_type,PON_GPIO_GRP_LEVEL_SECONDARY_E, &gpio_mapval);
			}
				
			break;
		case PON_GPIO_GRP_OTDR_PON_E:
			gpio_mapval.gpio_otdr_map.module_exist = 36;
			gpio_mapval.gpio_otdr_map.i2c_data = 40;
			gpio_mapval.gpio_otdr_map.i2c_clock = 41;
			ret = GW10G_PAS_set_gpio_group_mapping (olt_id,gpio_grp_type,PON_GPIO_GRP_LEVEL_PRIMARY_E, &gpio_mapval);

			gpio_mapval.gpio_otdr_map.module_exist = 37;
			gpio_mapval.gpio_otdr_map.i2c_data = 42;
			gpio_mapval.gpio_otdr_map.i2c_clock = 43;
			ret = GW10G_PAS_set_gpio_group_mapping (olt_id,gpio_grp_type,PON_GPIO_GRP_LEVEL_SECONDARY_E, &gpio_mapval);
			break;
		case PON_GPIO_GRP_AVS_E:
			gpio_mapval.gpio_avs_map.d_0 = 5;
			gpio_mapval.gpio_avs_map.d_1 = 6;
			ret = GW10G_PAS_set_gpio_group_mapping (olt_id,gpio_grp_type,PON_GPIO_GRP_LEVEL_NOT_RELEVANT_E, &gpio_mapval);
			break;
		case PON_GPIO_GRP_SLED_CONTROL_E:
			gpio_mapval.gpio_sled_map.oen = 7;
			gpio_mapval.gpio_sled_map.clr = 8;
			ret = GW10G_PAS_set_gpio_group_mapping (olt_id,gpio_grp_type,PON_GPIO_GRP_LEVEL_NOT_RELEVANT_E, &gpio_mapval);
			break;
		default:
			ret = PAS_EXIT_ERROR;
			break;
	}
	return ret;
}

/*For liying test@2012-08-16*/
int GW_set_gpio_group_mapping_test(const PON_olt_id_t olt_id, const PON_gpio_grp_level_e	 gpio_grp_pon_level, const int type, const int gpio,const int val)
{
	int ret = PAS_EXIT_OK;
	PON_gpio_grp_map_value_u gpio_mapval;
	memset(&gpio_mapval, 0, sizeof(PON_gpio_grp_map_value_u));

	ret = GW10G_PAS_get_gpio_group_mapping(olt_id, PON_GPIO_GRP_TYPE_10G_PON_E, gpio_grp_pon_level, &gpio_mapval);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_gpio_group_mapping err...\r\n");
		return ret;
	}
	switch(type)
	{
		case 0:
			gpio_mapval.gpio_10G_pon_map.module_exist = gpio;
			gpio_mapval.gpio_10G_pon_map.module_exist_polarity = val;
			break;
		case 1:
			gpio_mapval.gpio_10G_pon_map.i2c_data = gpio;
			break;
		case 2:
			gpio_mapval.gpio_10G_pon_map.i2c_clock = gpio;
			break;
		case 3:
			gpio_mapval.gpio_10G_pon_map.mod_desel = gpio;
			break;
		case 4:
			gpio_mapval.gpio_10G_pon_map.tx_1G_disable = gpio;
			gpio_mapval.gpio_10G_pon_map.tx_1G_disable_polarity = val;
			break;
		case 5:
			gpio_mapval.gpio_10G_pon_map.tx_10G_disable = gpio;
			gpio_mapval.gpio_10G_pon_map.tx_10G_disable_polarity = val;
			break;
		case 6:
			gpio_mapval.gpio_10G_pon_map.sqelch = gpio;
			gpio_mapval.gpio_10G_pon_map.sqelch_polarity = val;
			break;
		case 7:
			gpio_mapval.gpio_10G_pon_map.rx_loss = gpio;
			break;
		default:
			sys_console_printf("type is err\r\n");
			break;
			
	}
	ret = GW10G_PAS_set_gpio_group_mapping (olt_id,PON_GPIO_GRP_TYPE_10G_PON_E,gpio_grp_pon_level, &gpio_mapval);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_set_gpio_group_mapping err....\r\n");
	}
	else
	{
		sys_console_printf("PAS_set_gpio_group_mapping ok....\r\n");
	}
	return ret;
}
/*For liying test@2012-08-16*/
int GW_Set_TMSchedMode(const PON_olt_id_t	olt_id,const TM_sched_model_e mode)
{
	int i = 0;
	int ret = PAS_EXIT_OK;
	TM_sched_data_u data;
	memset(&data,0,sizeof(TM_sched_data_u));
	
	switch(mode)
	{
		case TM_SCHED_SIMPLE:
			for(i=0;i<SCHED_SIMPLE_DATA_ARRAY_SIZE;i++)
			{
				data.simple.down_pon0_type[i] = TM_QUEUE_TYPE_DATA;
				data.simple.down_pon1_type[i] = TM_QUEUE_TYPE_DATA;
				data.simple.down_cni0_type[i] = TM_QUEUE_TYPE_DATA;
				data.simple.down_cni1_type[i] = TM_QUEUE_TYPE_DATA;
				data.simple.p2p_type[i] = TM_QUEUE_TYPE_INVALID;
			}
			data.simple.p2p_type[6] = TM_QUEUE_TYPE_DATA_FAST;
			data.simple.p2p_type[7] = TM_QUEUE_TYPE_IMMEDIATELY;
			data.simple.down_mc_prio = 5;
			data.simple.down_mc_type = TM_QUEUE_TYPE_MC;
			data.simple.down_bc_prio = 6;
			data.simple.down_bc_type = TM_QUEUE_TYPE_MC;
			/*data.simple.down_powersave_type = TM_QUEUE_TYPE_DATA_FAST;*/
			ret = GW10G_PAS_TM_sched_mode_set(olt_id,TM_SCHED_SIMPLE,&data);

			break;

		default:
			sys_console_printf("Err mode ........\r\n");
			break;
	}

	return ret;
}

int GW_TMOperation_CommitSched(const PON_olt_id_t	olt_id)
{
	int ret = PAS_EXIT_OK;

	ret = GW10G_PAS_TM_operation(olt_id, TM_OPERATION_COMMIT_SCHED, ENABLE);

	return ret;
}

int GW_TMOperation_TrafficEnable(const PON_olt_id_t	olt_id)
{
	int ret = PAS_EXIT_OK;

	ret = GW10G_PAS_TM_operation(olt_id, TM_OPERATION_TRAFFIC_ENABLE, ENABLE);
	return ret;
}

int GW_set_olt_device_optics( const PON_olt_id_t olt_id, const PON_pon_type optics_type)
{
	int ret = PAS_EXIT_OK;
	PON_optics_param_u optics_param;
	memset(&optics_param,0,sizeof(PON_optics_param_u));

	switch(optics_type)
	{
		case PON_1G:
			optics_param.pon_1g_optics.discovery_mode_sync_time = 100;
			optics_param.pon_1g_optics.normal_mode_sync_time = 60;
			optics_param.pon_1g_optics.allocation_offset = 48;
			optics_param.pon_1g_optics.laser_on_time = 32;
			optics_param.pon_1g_optics.laser_off_time = 32;
			optics_param.pon_1g_optics.xcvr_reset_gate_offset = 1;
			optics_param.pon_1g_optics.xcvr_reset_duration = 1;
			optics_param.pon_1g_optics.xcvr_reset_polarity = PON_POLARITY_ACTIVE_HIGH;
			optics_param.pon_1g_optics.xcvr_sd_rx_polarity = PON_POLARITY_ACTIVE_LOW;
			optics_param.pon_1g_optics.xcvr_use_reset = FALSE;
			optics_param.pon_1g_optics.pon_tx_disable_line_polarity = PON_POLARITY_ACTIVE_HIGH;
			optics_param.pon_1g_optics.cdr_reset_gate_offset = 1;
			optics_param.pon_1g_optics.optics_dead_zone = 4;
			optics_param.pon_1g_optics.mac_active_data_window_left_shoulder = 16;
			optics_param.pon_1g_optics.mac_active_data_window_right_shoulder = 16;
			optics_param.pon_1g_optics.cdr_lock_window_left_shoulder = 56;
			optics_param.pon_1g_optics.cdr_lock_window_right_shoulder = 0;
			optics_param.pon_1g_optics.xcvr_reset_scheme = PON_RST_SCHEME_SD_BASED;
			optics_param.pon_1g_optics.cdr_reset_scheme = PON_RST_SCHEME_NON_SD_BASED;
			ret = GW10G_PAS_set_olt_device_optics(olt_id, PON_1G, &optics_param);
			break;
		case PON_10G:
			optics_param.pon_10g_optics.cdr_10g_reset_gate_offset = 1;
			optics_param.pon_10g_optics.ecdr_10g_reset_gate_offset = 1;
			optics_param.pon_10g_optics.ecdr_10g_reset_duration = 1;
			optics_param.pon_10g_optics.xcvr_10g_reset_gate_offset = 1;
			optics_param.pon_10g_optics.xcvr_10g_reset_duration = 1;
			optics_param.pon_10g_optics.xcvr_use_reset = FALSE;
			optics_param.pon_10g_optics.ecdr_10g_use_reset = FALSE;
			optics_param.pon_10g_optics.cdr_reset_scheme = PON_RST_SCHEME_NON_SD_BASED;
			optics_param.pon_10g_optics.ecdr_10g_reset_scheme = PON_RST_SCHEME_NON_SD_BASED;
			optics_param.pon_10g_optics.xcvr_reset_scheme = PON_RST_SCHEME_NON_SD_BASED;
			optics_param.pon_10g_optics.cdr_10g_lock_window_left_shoulder = 56;
			optics_param.pon_10g_optics.cdr_10g_lock_window_right_shoulder = 0;
			optics_param.pon_10g_optics.rx_rst_per_rate = PON_RST_SEPARATE;
			optics_param.pon_10g_optics.rx_xsbi_mode = PON_RST_RX_XSBI_DONT_APPLY;
			optics_param.pon_10g_optics.ecdr_10g_use_lock_window = FALSE;
			optics_param.pon_10g_optics.ecdr_10g_lock_disc_mode = PON_RST_STUCK_AT_0;
			optics_param.pon_10g_optics.ecdr_10g_discovery_left_shoulder = 0;
			optics_param.pon_10g_optics.ecdr_10g_discovery_right_shoulder = 0;
			optics_param.pon_10g_optics.ecdr_10g_use_disc_envelop = FALSE;
			optics_param.pon_10g_optics.ecdr_10g_rst_pol = PON_POLARITY_ACTIVE_HIGH;
			optics_param.pon_10g_optics.ecdr_10g_disc_win_pol = PON_POLARITY_ACTIVE_HIGH;
			optics_param.pon_10g_optics.normal_10G_mode_sync_time = 100;
			optics_param.pon_10g_optics.disc_10G_mode_sync_time = 12500;
			optics_param.pon_10g_optics.mac_10g_active_window_left_shoulder = 16;
			optics_param.pon_10g_optics.mac_10g_active_window_right_shoulder = 16;
			optics_param.pon_10g_optics.xcvr_user_rate_select = 0;
			optics_param.pon_10g_optics.pon_opt_rate_sel_left_shoulder = 1;
			optics_param.pon_10g_optics.pon_opt_rate_sel_right_shoulder = 1;
			optics_param.pon_10g_optics.optics_dead_zone_post_1G_to_10G = 4;
			optics_param.pon_10g_optics.optics_dead_zone_post_1G_to_1G = 4;
			optics_param.pon_10g_optics.optics_dead_zone_post_10G_to_1G = 4;
			optics_param.pon_10g_optics.optics_dead_zone_post_10G_to_10G = 4;
			optics_param.pon_10g_optics.optics_dead_zone_pre_1G = 16;
			optics_param.pon_10g_optics.optics_dead_zone_pre_10G = 4;
			optics_param.pon_10g_optics.cdr_rate_sel_left_shoulder = 1;
			optics_param.pon_10g_optics.cdr_rate_sel_right_shoulder = 1;
			optics_param.pon_10g_optics.rx_lane_per_rate = PON_RST_SEPARATE;
			#if 1
			optics_param.pon_10g_optics.pon_1g_optic.discovery_mode_sync_time = 100;
			optics_param.pon_10g_optics.pon_1g_optic.normal_mode_sync_time = 60;
			optics_param.pon_10g_optics.pon_1g_optic.allocation_offset = 48;
			optics_param.pon_10g_optics.pon_1g_optic.laser_on_time = 32;
			optics_param.pon_10g_optics.pon_1g_optic.laser_off_time = 32;
			optics_param.pon_10g_optics.pon_1g_optic.xcvr_reset_gate_offset = 1;
			optics_param.pon_10g_optics.pon_1g_optic.xcvr_reset_duration = 1;
			optics_param.pon_10g_optics.pon_1g_optic.xcvr_reset_polarity = PON_POLARITY_ACTIVE_HIGH;
			optics_param.pon_10g_optics.pon_1g_optic.xcvr_sd_rx_polarity = PON_POLARITY_ACTIVE_LOW;
			optics_param.pon_10g_optics.pon_1g_optic.xcvr_use_reset = FALSE;
			optics_param.pon_10g_optics.pon_1g_optic.pon_tx_disable_line_polarity = PON_POLARITY_ACTIVE_HIGH;
			optics_param.pon_10g_optics.pon_1g_optic.cdr_reset_gate_offset = 1;
			optics_param.pon_10g_optics.pon_1g_optic.optics_dead_zone = 4;
			optics_param.pon_10g_optics.pon_1g_optic.mac_active_data_window_left_shoulder = 16;
			optics_param.pon_10g_optics.pon_1g_optic.mac_active_data_window_right_shoulder = 16;
			optics_param.pon_10g_optics.pon_1g_optic.cdr_lock_window_left_shoulder = 56;
			optics_param.pon_10g_optics.pon_1g_optic.cdr_lock_window_right_shoulder = 0;
			optics_param.pon_10g_optics.pon_1g_optic.xcvr_reset_scheme = PON_RST_SCHEME_SD_BASED;
			optics_param.pon_10g_optics.pon_1g_optic.cdr_reset_scheme = PON_RST_SCHEME_NON_SD_BASED;
			
			optics_param.pon_10g_optics.pon_1g_10g_optic.sd_per_rate= PON_RST_SEPARATE;
			optics_param.pon_10g_optics.pon_1g_10g_optic.ecdr_1g_lock_disc_mode = PON_RST_STUCK_AT_0;
			optics_param.pon_10g_optics.pon_1g_10g_optic.ecdr_1G_reset_gate_offset = 1;
			optics_param.pon_10g_optics.pon_1g_10g_optic.ecdr_1G_reset_duration = 1;
			optics_param.pon_10g_optics.pon_1g_10g_optic.ecdr_1G_use_reset = FALSE;
			optics_param.pon_10g_optics.pon_1g_10g_optic.ecdr_1G_reset_scheme = PON_RST_SCHEME_NON_SD_BASED;
			optics_param.pon_10g_optics.pon_1g_10g_optic.ecdr_1G_use_lock_window = FALSE;
			optics_param.pon_10g_optics.pon_1g_10g_optic.ecdr_1G_discovery_left_shoulder = 0;
			optics_param.pon_10g_optics.pon_1g_10g_optic.ecdr_1G_discovery_right_shoulder = 0;
			optics_param.pon_10g_optics.pon_1g_10g_optic.ecdr_1G_use_disc_envelop = FALSE;
			optics_param.pon_10g_optics.pon_1g_10g_optic.ecdr_1G_reset_pol = PON_POLARITY_ACTIVE_HIGH;
			optics_param.pon_10g_optics.pon_1g_10g_optic.ecdr_1G_disc_win_pol = PON_POLARITY_ACTIVE_HIGH;
			#endif

			ret = GW10G_PAS_set_olt_device_optics(olt_id, PON_10G, &optics_param);
			break;
		default:
			sys_console_printf("optics_type(%d)is error\r\n",optics_type);
			break;
				
	}
	return ret;
}


int GW_set_CNI_interface_type(const PON_olt_id_t olt_id)
{
	int ret = PAS_EXIT_OK;
    /* CNI types: 0-XAUI, 1-RXAUI, 2-SGMII*/
	int cni_interface_type = 0;
	
	ret = GW10G_PAS_pre_init_simple_operation_add(olt_id, GW10G_PAS_SIMPLE_OPERATION_SET_CNI_INTERFACE, cni_interface_type);
	return ret;
}

int GW_get_olt_device_version(const PON_olt_id_t olt_id)
{
	int iRlt = PAS_EXIT_OK;
	GW10G_PAS_device_versions_t device_versions;
    INT32U critical_events_counter = 0;
	INT32U non_critical_events_counter = 0;
	memset(&device_versions,0,sizeof(GW10G_PAS_device_versions_t));
	iRlt = GW10G_PAS_get_olt_versions(olt_id, &device_versions,&critical_events_counter,&non_critical_events_counter);
	if(PAS_EXIT_OK == iRlt)
	{
		OLT_PAS_DEBUG(OLT_PAS_TITLE"device_id=%d,host_major=%d,host_minor=%d,host_compilation=%d,host_maintenance=%d\r\n",
			device_versions.device_id,device_versions.host_major,device_versions.host_minor,device_versions.host_compilation,device_versions.host_maintenance);
		OLT_PAS_DEBUG(OLT_PAS_TITLE"firmware_major=%d,firmware_minor=%d,build_firmware=%d,maintenance_firmware=%d,hardware_major=0x%x,hardware_minor=0x%x\r\n",
			device_versions.firmware_major,device_versions.firmware_minor,device_versions.build_firmware,device_versions.maintenance_firmware,device_versions.hardware_major,device_versions.hardware_minor);
		OLT_PAS_DEBUG(OLT_PAS_TITLE"system_mac=%d,ports_supported=%d,critical_events_counter=%d,non_critical_events_counter=%d\r\n",device_versions.system_mac,device_versions.ports_supported,critical_events_counter,non_critical_events_counter);
	}
}
int GW_Set_GPIO_AccessInit(const PON_olt_id_t	olt_id)
{
	int ret = 0;
	ret = GW_Set_GPIO_Access(olt_id,38,PON_GPIO_LINE_INPUT,-1);
	ret = GW_Set_GPIO_Access(olt_id,40,-1,0);
	ret = GW_Set_GPIO_Access(olt_id,41,PON_GPIO_LINE_OUTPUT,0);
	ret = GW_Set_GPIO_Access(olt_id,29,PON_GPIO_LINE_OUTPUT,0);
	ret = GW_Set_GPIO_Access(olt_id,28,PON_GPIO_LINE_OUTPUT,0);
	ret = GW_Set_GPIO_Access(olt_id,30,PON_GPIO_LINE_OUTPUT,0);
	ret = GW_Set_GPIO_Access(olt_id,26,PON_GPIO_LINE_OUTPUT,0);
	ret = GW_Set_GPIO_Access(olt_id,27,PON_GPIO_LINE_OUTPUT,0);

	ret = GW_Set_GPIO_Access(olt_id,39,PON_GPIO_LINE_INPUT,-1);
	ret = GW_Set_GPIO_Access(olt_id,34,PON_GPIO_LINE_OUTPUT,0);
	ret = GW_Set_GPIO_Access(olt_id,33,PON_GPIO_LINE_OUTPUT,0);
	ret = GW_Set_GPIO_Access(olt_id,35,PON_GPIO_LINE_OUTPUT,0);
	ret = GW_Set_GPIO_Access(olt_id,31,PON_GPIO_LINE_OUTPUT,0);
	ret = GW_Set_GPIO_Access(olt_id,32,PON_GPIO_LINE_OUTPUT,0);
	return ret;
	
}
int GW_Set_GPIO_Access( const PON_olt_id_t			olt_id, 
							    const GW10G_PON_gpio_lines_t		line_number,
							    const PON_gpio_line_io_t    set_direction,
							    const short int		  	    set_value
							    )

{
	int ret = PAS_EXIT_OK;
    PON_gpio_line_io_t direction = 0;
	bool value = 0;
	sys_console_printf("the input is olt_id=%d,line_number=%d,set_direction=%d,set_value=%d\r\n",olt_id,line_number,set_direction,set_value);
	ret = GW10G_PAS_gpio_access(olt_id, line_number, set_direction, set_value, &direction, &value);
	if(PAS_EXIT_OK == ret)
	{
		sys_console_printf("PAS_gpio_access ok,real direction=%d,value=%d\r\n",direction,value);
	}

	return ret;
	
}

int GW_Set_address_table_config(const PON_olt_id_t olt_id, const NE_addr_config_e config_type, const INT32U address_table_config)
{
	int ret = PAS_EXIT_OK;
	
	sys_console_printf("GW_Set_address_table_config,olt_id:%d,config_type:%d,valule:%d\r\n",olt_id,config_type,address_table_config);

	ret = GW10G_PAS_set_address_table_configuration(olt_id, config_type, address_table_config);
	if( PAS_EXIT_OK != ret)
	{
		sys_console_printf("set err,ret = %d\r\n",ret);
	}
	else
	{
		sys_console_printf("set ok\r\n");
	}
	
}

int GW_Get_address_table_config(const PON_olt_id_t olt_id, const NE_addr_config_e config_type)
{
	int ret = PAS_EXIT_OK;
	INT32U value = 0;
	
	sys_console_printf("GW_Get_address_table_config,olt_id:%d,config_type:%d\r\n",olt_id,config_type);

	ret = GW10G_PAS_get_address_table_configuration(olt_id, config_type, &value);
	if( PAS_EXIT_OK != ret)
	{
		sys_console_printf("get err,ret = %d\r\n",ret);
	}
	else
	{
		sys_console_printf("get ok,value:%d\r\n",value);
	}
	
}
int GW_get_address_table_countbymac(const PON_olt_id_t olt_id,const INT32U start_index,const INT32S end_index,const INT32U max_count,
INT8U MAC0,INT8U MAC1,INT8U MAC2,INT8U MAC3,INT8U MAC4,INT8U MAC5)
{
	int ret = PAS_EXIT_OK;
	INT32U count = 1;
	NE_addr_entry_s search;
	NE_addr_entry_s entries;

	memset(&search,0,sizeof(search));
	memset(&entries,0,sizeof(entries));
	search.addr_type = NE_ADDR_RULE_TYPE_MAC;
	search.addr_value.mac[0] = MAC0;
	search.addr_value.mac[1] = MAC1;
	search.addr_value.mac[2] = MAC2;
	search.addr_value.mac[3] = MAC3;
	search.addr_value.mac[4] = MAC4;
	search.addr_value.mac[5] = MAC5;
	sys_console_printf("The input is olt_id=%d,start_index=%d,end_index=%d,max_count=%d\r\n",olt_id,start_index,end_index,max_count);
	sys_console_printf("Addr mac is 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\r\n",search.addr_value.mac[0],search.addr_value.mac[1],
		search.addr_value.mac[2],search.addr_value.mac[3],search.addr_value.mac[4],search.addr_value.mac[5]);
	ret = GW10G_PAS_get_address_table(olt_id,start_index,end_index,max_count,&count,&search,NULL);
	if(PAS_EXIT_OK == ret)
	{
		sys_console_printf("GW_get_address_table_countbymac ok,real count =%d\r\n",count);
	}
	return ret;
	
}
int GW_get_address_table_countbyall(const PON_olt_id_t olt_id,const INT32U start_index,const INT32S end_index,const INT32U max_count)
{
	int ret = PAS_EXIT_OK;
	INT32U count = 1;
	NE_addr_entry_s search;

	memset(&search,0,sizeof(search));

	search.addr_type = NE_ADDR_RULE_SEARCH_ALL;
    sys_console_printf("The input is olt_id=%d,start_index=%d,end_index=%d,max_count=%d\r\n",olt_id,start_index,end_index,max_count);
	ret = GW10G_PAS_get_address_table(olt_id,start_index,end_index,max_count,&count,&search,NULL);
	if(PAS_EXIT_OK == ret)
	{
		sys_console_printf("GW_get_address_table_countbyall ok,real count =%d\r\n",count);
	}
	return ret;
	
}
int GW_get_address_table_bymac(const PON_olt_id_t olt_id,const INT32U start_index,const INT32S end_index,INT8U MAC0,INT8U MAC1,INT8U MAC2,INT8U MAC3,INT8U MAC4,INT8U MAC5)
{
    short int         return_result;
        
    INT32U            count = 1, num_of_rules = 1;
    NE_addr_entry_s   search = {0};
    NE_addr_entry_s   *entries_ptr = NULL;
	int i = 0;
	INT32U max_count = NE_MAX_ADDRESS_TABLE_ENTRIES;

    search.addr_type = NE_ADDR_RULE_TYPE_MAC;
	search.addr_value.mac[0] = MAC0;
	search.addr_value.mac[1] = MAC1;
	search.addr_value.mac[2] = MAC2;
	search.addr_value.mac[3] = MAC3;
	search.addr_value.mac[4] = MAC4;
	search.addr_value.mac[5] = MAC5;


	/* Get the number of entries */
	return_result = GW10G_PAS_get_address_table(olt_id, start_index, end_index, max_count, &num_of_rules, &search, NULL );
   	if ( return_result != PAS_EXIT_OK )
	{
		sys_console_printf("get count err\r\n");
        return (return_result);
	}

	sys_console_printf("num_of_rules :%d\r\n",num_of_rules);

	/* Set max_count to be the MIN(max_count,num_of_rules) */
	if (num_of_rules < max_count)
	{
		max_count = num_of_rules;
	}

	/* Allocate output buffer to receive response */
    /* ========================================== */
	entries_ptr = (NE_addr_entry_s *)VOS_Malloc(sizeof(NE_addr_entry_s)*max_count,MODULE_OLT);
	if(entries_ptr == NULL)
	{
		sys_console_printf("g_malloc err\r\n");
		return (PAS_MEMORY_ERROR);
	}

	return_result = GW10G_PAS_get_address_table
                             (  olt_id,
                                start_index,
								end_index,
                                max_count,
                               &count,
                               &search,
                                entries_ptr );

    if ( return_result != PAS_EXIT_OK )
	{
		sys_console_printf("get content err\r\n");
	}
	else
	{
	 /*......... */
		sys_console_printf("get content ok,count:%d\r\n",count);
	    for(i = 0;i < count;i++)
    	{
    		sys_console_printf("mac:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\r\n",
				entries_ptr[i].addr_value.mac[0],entries_ptr[i].addr_value.mac[1],entries_ptr[i].addr_value.mac[2],
				entries_ptr[i].addr_value.mac[3],entries_ptr[i].addr_value.mac[4],entries_ptr[i].addr_value.mac[5]);
			sys_console_printf("port:%d,llid:%d,age:%d,ivt_id:%d,da_discard:%d,da_sniff:%d,da_cpu:%d,sa_discard:%d,sa_sniff:%d,sa_cpu:%d,is_static:%d\r\n",
				entries_ptr[i].port,entries_ptr[i].llid,entries_ptr[i].age,entries_ptr[i].ivt_id,entries_ptr[i].da_discard,entries_ptr[i].da_to_sniff,entries_ptr[i].da_to_cpu,
				entries_ptr[i].sa_discard,entries_ptr[i].sa_to_sniff,entries_ptr[i].sa_to_cpu,entries_ptr[i].is_static);
    	}

	}

	
    return (return_result);
}

int GW_get_address_table_byall(const PON_olt_id_t olt_id,const INT32U start_index,const INT32S end_index)
{
    short int         return_result;
    
    INT32U            count = 1, num_of_rules = 1;
    NE_addr_entry_s   search = {0};
    NE_addr_entry_s   *entries_ptr = NULL;
	int i = 0;

	INT32U max_count = NE_MAX_ADDRESS_TABLE_ENTRIES;
    search.addr_type = NE_ADDR_RULE_SEARCH_ALL;
	


	/* Get the number of entries */
	return_result = GW10G_PAS_get_address_table(olt_id, start_index, end_index, max_count, &num_of_rules, &search, NULL );
   	if ( return_result != PAS_EXIT_OK )
	{
		sys_console_printf("get count err\r\n");
        return (return_result);
	}

	sys_console_printf("num_of_rules :%d\r\n",num_of_rules);

	/* Set max_count to be the MIN(max_count,num_of_rules) */
	if (num_of_rules < max_count)
	{
		max_count = num_of_rules;
	}

	/* Allocate output buffer to receive response */
    /* ========================================== */
	entries_ptr = (NE_addr_entry_s *)VOS_Malloc(sizeof(NE_addr_entry_s)*max_count,MODULE_OLT);
	if(entries_ptr == NULL)
	{
		sys_console_printf("g_malloc err\r\n");
		return (PAS_MEMORY_ERROR);
	}

	return_result = GW10G_PAS_get_address_table
                             (  olt_id,
                                start_index,
								end_index,
                                max_count,
                               &count,
                               &search,
                                entries_ptr );

    if ( return_result != PAS_EXIT_OK )
	{
		sys_console_printf("get content err\r\n");
	}
	else
	{
	 /*......... */
		sys_console_printf("get content ok,count:%d\r\n",count);
	    for(i = 0;i < count;i++)
    	{
    		sys_console_printf("mac:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\r\n",
				entries_ptr[i].addr_value.mac[0],entries_ptr[i].addr_value.mac[1],entries_ptr[i].addr_value.mac[2],
				entries_ptr[i].addr_value.mac[3],entries_ptr[i].addr_value.mac[4],entries_ptr[i].addr_value.mac[5]);
			sys_console_printf("port:%d,llid:%d,age:%d,ivt_id:%d,da_discard:%d,da_sniff:%d,da_cpu:%d,sa_discard:%d,sa_sniff:%d,sa_cpu:%d,is_static:%d\r\n",
				entries_ptr[i].port,entries_ptr[i].llid,entries_ptr[i].age,entries_ptr[i].ivt_id,entries_ptr[i].da_discard,entries_ptr[i].da_to_sniff,entries_ptr[i].da_to_cpu,
				entries_ptr[i].sa_discard,entries_ptr[i].sa_to_sniff,entries_ptr[i].sa_to_cpu,entries_ptr[i].is_static);
    	}

	}

	
    return (return_result);
}
int GW_add_address_table_record(const PON_olt_id_t olt_id)
{
	int ret = 0;
	NE_addr_entry_s entry;
    INT8U mac_tmp[6]={0x00,0x0c,0xd5,0x69,0x09,0x0b};

	
	memset(&entry,0,sizeof(entry));
	sys_console_printf("The input is olt_id=%d\r\n",olt_id);
	entry.addr_type = NE_ADDR_RULE_TYPE_MAC;
	memcpy(entry.addr_value.mac,mac_tmp,sizeof(mac_tmp));
	entry.port = TM_PORT_PON_10G;
	entry.llid = 1;
	entry.age = 20000;
	entry.ivt_id = 1;
	entry.da_discard = FALSE;
	entry.da_to_cpu = TRUE;
	entry.da_to_sniff = FALSE;
	entry.is_static = TRUE;
	entry.sa_discard = TRUE;
	entry.sa_to_cpu = TRUE;
	entry.sa_to_sniff = FALSE;

	ret =  GW10G_PAS_add_address_table_record(olt_id,&entry);

	if(PAS_EXIT_OK == ret)
	{
		sys_console_printf("PAS_add_address_table_record ok\r\n");
	}
	
    return ret;
	
	
}

int GW_get_olt_module_exist(const PON_olt_id_t olt_id)
{
	int ret = PAS_EXIT_OK;
    bool exist = 1;
	sys_console_printf("The input is olt_id=%d\r\n",olt_id);
	ret = GW10G_PAS_get_olt_pon_module_exist(olt_id,PON_GPIO_GRP_LEVEL_PRIMARY_E,&exist);
	if(PAS_EXIT_OK == ret)
	{
		sys_console_printf("get PRIMARY module ok ,exist is %d\r\n",exist);
	}
	exist = 1;
	ret = GW10G_PAS_get_olt_pon_module_exist(olt_id,PON_GPIO_GRP_LEVEL_SECONDARY_E,&exist);
	if(PAS_EXIT_OK == ret)
	{
		sys_console_printf("get SECONDARY module ok ,exist is %d\r\n",exist);
	}

	return ret;
}
/*清空所有统计*/
int GW_reset_olt_counters(const PON_olt_id_t olt_id)
{
	int ret = PAS_EXIT_OK;
	
	sys_console_printf("The input is olt_id=%d\r\n",olt_id);

	ret= GW10G_PAS_reset_olt_counters(olt_id);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_reset_olt_counters is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_reset_olt_counters is ok\r\n");
	}
}

int GW_reset_olt_statistic(const PON_olt_id_t olt_id, INT32U count, STAT_olt_entity_e	entity,INT32U entity_index, INT32U statistic_index )
{
	int ret = PAS_EXIT_OK;
	STAT_reset_field_t stat_entry = {0};
	sys_console_printf("The input is olt_id=%d,	entity=%d, entity_index=%d, statistic_index=%d\r\n",olt_id,entity,entity_index,statistic_index);

	stat_entry.entity = entity;
	stat_entry.entity_index = entity_index;
	stat_entry.statistic_index = statistic_index;
	ret = GW10G_PAS_reset_olt_statistic(olt_id, &count, &stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_reset_olt_statistic is err,err is %d,reset_status is %d\r\n", ret, stat_entry.reset_status);
	}
	else
	{
		sys_console_printf("PAS_reset_olt_statistic is ok,reset_status is %d\r\n",stat_entry.reset_status);
	}
	
	
}

int GW_get_olt_statistic(const PON_olt_id_t olt_id,	INT32U count,	STAT_olt_entity_e	entity,INT32U entity_index, INT32U statistic_index)
{
	int ret = PAS_EXIT_OK;
	STAT_field_t stat_entry = {0};

	sys_console_printf("The input is olt_id=%d,	entity=%d, entity_index=%d, statistic_index=%d\r\n",olt_id,entity,entity_index,statistic_index);

	stat_entry.entity = entity;
	stat_entry.entity_index = entity_index;
	stat_entry.statistic_index = statistic_index;

	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic is err,err is %d,statistic_value is %d\r\n", ret, stat_entry.statistic_value);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}
}


int GW_get_olt_statistic_PON10G(const PON_olt_id_t olt_id, INT32U entity_index)
{
	int ret = PAS_EXIT_OK;
	int count = 1;
	STAT_field_t stat_entry = {0};

	sys_console_printf("The input is olt_id=%d\r\n",olt_id);
	stat_entry.entity = STAT_ENTITY_PON_10G_XMT_E;
	stat_entry.entity_index = entity_index;
	count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_PKT_TX_OK_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_PKT_TX_OK_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_PKT_TX_OK_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_PKT_TX_OK_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_PKT_TX_OK_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_PKT_TX_OK_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

    count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_PKT_TX_BAD_FCS_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_PKT_TX_BAD_FCS_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_PKT_TX_BAD_FCS_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}
	
    count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_PKT_TX_BAD_FCS_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_PKT_TX_BAD_FCS_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_PKT_TX_BAD_FCS_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}


	count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_OK_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_OK_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_OK_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_OK_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_OK_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_OK_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_BAD_FCS_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_BAD_FCS_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_BAD_FCS_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_BAD_FCS_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_BAD_FCS_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_BAD_FCS_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}


	
	count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_DA_UCAST_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_DA_UCAST_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_DA_UCAST_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_DA_UCAST_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_DA_UCAST_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_DA_UCAST_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}


	count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_DA_MCAST_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_DA_MCAST_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_DA_MCAST_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}


	count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_DA_MCAST_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_DA_MCAST_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_DA_MCAST_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}


	count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_DA_BCAST_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_DA_BCAST_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_DA_BCAST_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMT_GEN_TOTAL_DA_BCAST_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_DA_BCAST_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMT_GEN_TOTAL_DA_BCAST_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}



	stat_entry.entity = STAT_ENTITY_PON_10G_XMR_E;
	stat_entry.entity_index = entity_index;
	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_PCS_RXERR_C_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_C_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_C_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_PCS_RXERR_C_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_C_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_C_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_PCS_RXERR_S_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_S_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_S_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_PCS_RXERR_S_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_S_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_S_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_PCS_RXERR_T_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_T_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_T_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_PCS_RXERR_T_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_T_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_T_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_PCS_RXERR_D_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_D_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_D_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_PCS_RXERR_D_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_D_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_D_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_PCS_RXERR_E_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_E_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_E_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_PCS_RXERR_E_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_E_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_PCS_RXERR_E_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_BAD_PKT_RX_OCTETS_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_BAD_PKT_RX_OCTETS_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_BAD_PKT_RX_OCTETS_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_BAD_PKT_RX_OCTETS_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_BAD_PKT_RX_OCTETS_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_BAD_PKT_RX_OCTETS_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_SYMBOL_ERR_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_SYMBOL_ERR_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_SYMBOL_ERR_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_SYMBOL_ERR_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_SYMBOL_ERR_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_SYMBOL_ERR_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}


	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_OVERSIZE_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_OVERSIZE_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_OVERSIZE_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_OVERSIZE_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_OVERSIZE_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_OVERSIZE_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_BAD_FCS_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_BAD_FCS_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_BAD_FCS_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_BAD_FCS_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_BAD_FCS_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_BAD_FCS_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_FIFO_ERR_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_FIFO_ERR_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_FIFO_ERR_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_FIFO_ERR_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_FIFO_ERR_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_FIFO_ERR_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}


	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_DA_UCAST_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_DA_UCAST_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_DA_UCAST_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_DA_UCAST_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_DA_UCAST_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_DA_UCAST_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_DA_MCAST_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_DA_MCAST_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_DA_MCAST_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}


	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_DA_MCAST_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_DA_MCAST_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_DA_MCAST_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_DA_BCAST_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_DA_BCAST_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_DA_BCAST_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_10G_XMR_GEN_TOTAL_DA_BCAST_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_DA_BCAST_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_10G_XMR_GEN_TOTAL_DA_BCAST_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	return ret;
}

int GW_get_olt_statistic_PON1G(const PON_olt_id_t olt_id, INT32U entity_index)
{
	int ret = PAS_EXIT_OK;
	int count = 1;
	STAT_field_t stat_entry = {0};

	sys_console_printf("The input is olt_id=%d\r\n",olt_id);
	stat_entry.entity = STAT_ENTITY_PON_E;
	stat_entry.entity_index = entity_index;
	
	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_VR1_PON_TX_PKT_TOT_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_VR1_PON_TX_PKT_TOT_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_VR1_PON_TX_PKT_TOT_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_VR1_PON_TX_PKT_TOT_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_VR1_PON_TX_PKT_TOT_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_VR1_PON_TX_PKT_TOT_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	
	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_BAD_LLID_TOT_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_BAD_LLID_TOT_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_BAD_LLID_TOT_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_BAD_LLID_TOT_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_BAD_LLID_TOT_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_BAD_LLID_TOT_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_GOOD_LLID_TOT_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_GOOD_LLID_TOT_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_GOOD_LLID_TOT_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_GOOD_LLID_TOT_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_GOOD_LLID_TOT_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_GOOD_LLID_TOT_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_PON_CAST_LLID_TOT_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_PON_CAST_LLID_TOT_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_PON_CAST_LLID_TOT_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_PON_CAST_LLID_TOT_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_PON_CAST_LLID_TOT_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_PON_CAST_LLID_TOT_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_TX_UCAST_IND_TOT_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_TX_UCAST_IND_TOT_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_TX_UCAST_IND_TOT_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_TX_UCAST_IND_TOT_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_TX_UCAST_IND_TOT_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_TX_UCAST_IND_TOT_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_TX_BROADCAST_IND_TOT_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_TX_BROADCAST_IND_TOT_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_TX_BROADCAST_IND_TOT_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_TX_BROADCAST_IND_TOT_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_TX_BROADCAST_IND_TOT_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_TX_BROADCAST_IND_TOT_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_TX_MULTICAST_IND_TOT_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_TX_MULTICAST_IND_TOT_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_TX_MULTICAST_IND_TOT_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_TX_MULTICAST_IND_TOT_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_TX_MULTICAST_IND_TOT_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_TX_MULTICAST_IND_TOT_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_RX_UCAST_IND_TOT_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_RX_UCAST_IND_TOT_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_RX_UCAST_IND_TOT_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_RX_UCAST_IND_TOT_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_RX_UCAST_IND_TOT_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_RX_UCAST_IND_TOT_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_RX_BROADCAST_IND_TOT_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_RX_BROADCAST_IND_TOT_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_RX_BROADCAST_IND_TOT_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_RX_BROADCAST_IND_TOT_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_RX_BROADCAST_IND_TOT_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_RX_BROADCAST_IND_TOT_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_RX_MULTICAST_IND_TOT_LSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_RX_MULTICAST_IND_TOT_LSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_RX_MULTICAST_IND_TOT_LSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_1G_MAC_GEN_RX_MULTICAST_IND_TOT_MSB;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_RX_MULTICAST_IND_TOT_MSB is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_1G_MAC_GEN_RX_MULTICAST_IND_TOT_MSB is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	return ret;
}

int GW_get_olt_statistic_CNI(const PON_olt_id_t olt_id, INT32U entity_index)
{
	int ret = PAS_EXIT_OK;
	int count = 1;
	STAT_field_t stat_entry = {0};

	sys_console_printf("The input is olt_id=%d\r\n",olt_id);
	stat_entry.entity = STAT_ENTITY_CNI_E;
	stat_entry.entity_index = entity_index;
	
	count = 1;
	stat_entry.statistic_index = LAG_CHNL_RLAG_FRM_IN_COUNTER;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_FRM_IN_COUNTER is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_FRM_IN_COUNTER is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_RLAG_FRM_OUT_COUNTER;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_FRM_OUT_COUNTER is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_FRM_OUT_COUNTER is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_RLAG_BYTE_IN_COUNTER_LOW;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_BYTE_IN_COUNTER_LOW is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_BYTE_IN_COUNTER_LOW is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_RLAG_BYTE_IN_COUNTER_HIGH;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_BYTE_IN_COUNTER_HIGH is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_BYTE_IN_COUNTER_HIGH is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_RLAG_BYTE_OUT_COUNTER_LOW;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_BYTE_OUT_COUNTER_LOW is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_BYTE_OUT_COUNTER_LOW is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_RLAG_BYTE_OUT_COUNTER_HIGH;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_BYTE_OUT_COUNTER_HIGH is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_BYTE_OUT_COUNTER_HIGH is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_RLAG_FRM_TOTAL_DROP_COUNTER;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_FRM_TOTAL_DROP_COUNTER is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_FRM_TOTAL_DROP_COUNTER is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_RLAG_FRM_IN_DROP_COUNTER;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_FRM_IN_DROP_COUNTER is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_RLAG_FRM_IN_DROP_COUNTER is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_TLAG_FRM_MARK_RES_COUNTER;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_FRM_MARK_RES_COUNTER is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_FRM_MARK_RES_COUNTER is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_TLAG_FRMN_COUNTER;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_FRMN_COUNTER is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_FRMN_COUNTER is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_TLAG_BYTEN_COUNTER_LOW;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_BYTEN_COUNTER_LOW is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_BYTEN_COUNTER_LOW is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_TLAG_BYTEN_COUNTER_HIGH;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_BYTEN_COUNTER_HIGH is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_BYTEN_COUNTER_HIGH is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_TLAG_FRM_OUT_COUNTER;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_FRM_OUT_COUNTER is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_FRM_OUT_COUNTER is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_TLAG_BYTE_OUT_COUNTER_LOW;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_BYTE_OUT_COUNTER_LOW is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_BYTE_OUT_COUNTER_LOW is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = LAG_CHNL_TLAG_BYTE_OUT_COUNTER_HIGH;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_BYTE_OUT_COUNTER_HIGH is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic LAG_CHNL_TLAG_BYTE_OUT_COUNTER_HIGH is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	return ret;
}

int GW_get_olt_statistic_llid_general(const PON_olt_id_t olt_id, INT32U entity_index)
{
	int ret = PAS_EXIT_OK;
	int count = 1;
	STAT_field_t stat_entry = {0};

	sys_console_printf("The input is olt_id=%d\r\n",olt_id);
	stat_entry.entity = STAT_ENTITY_LLID_E;
	stat_entry.entity_index = entity_index;
	
	count = 1;
	stat_entry.statistic_index = OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_DROP;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_DROP is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_DROP is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	count = 1;
	stat_entry.statistic_index = OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_RX;
	stat_entry.statistic_value = 0;
	ret = GW10G_PAS_get_olt_statistic(olt_id,&count,&stat_entry);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_get_olt_statistic OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_RX is err,err is %d\r\n", ret);
	}
	else
	{
		sys_console_printf("PAS_get_olt_statistic OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_RX is ok,count is %d,statistic_value is %d\r\n",count, stat_entry.statistic_value);
	}

	return ret;
}

int GW_get_cni_link_status ( const PON_olt_id_t	olt_id)
{
	int ret = PAS_EXIT_OK;
	int link_status = 0;

	ret = GW10G_PAS_get_cni_link_status(olt_id,TM_PORT_CNI0,&link_status );

	if(PAS_EXIT_OK == ret)
	{
		sys_console_printf("PAS_get_cni_link_status, cni0 link_status is %d\r\n",link_status);
	}

	link_status = 0;
	ret = GW10G_PAS_get_cni_link_status(olt_id,TM_PORT_CNI1,&link_status );

	if(PAS_EXIT_OK == ret)
	{
		sys_console_printf("PAS_get_cni_link_status, cni1 link_status is %d\r\n",link_status);
	}

	return ret;
}

int GW_set_CNI_MAC_mode(const PON_olt_id_t olt_id, const int cni_port, const int cni_conf_type, const int cni_conf_value)
{
	int ret = PAS_EXIT_OK;
    GW10G_PON_cni_config_value_u value={0};
	if(GW10G_PON_CNI_CONFIG_MAC_MODE==cni_conf_type)
	{
		value.cni_mac_mode = cni_conf_value;
	}
	else if (GW10G_PON_CNI_CONFIG_MAC_STATE==cni_conf_type)
	{
		value.cni_mac_state = cni_conf_value;
	}
	ret = GW10G_PAS_set_ethernet_cni_mac_mode(olt_id,cni_port,cni_conf_type,value);
	if ( PAS_EXIT_OK == ret)
	{
		sys_console_printf("PAS_set_ethernet_cni_mac_mode ok\r\n");
	}
	else
	{
		sys_console_printf("PAS_set_ethernet_cni_mac_mode err,ret is %d\r\n",ret);
	}
	return ret;
}
int GW_get_CNI_MAC_mode(const PON_olt_id_t olt_id, const int cni_port, const int cni_conf_type)
{
	int ret = PAS_EXIT_OK;
    GW10G_PON_cni_config_value_u value={0};
	if (GW10G_PON_CNI_CONFIG_MAC_MODE==cni_conf_type)
	{
		ret = GW10G_PAS_get_ethernet_cni_mac_mode(olt_id,cni_port,GW10G_PON_CNI_CONFIG_MAC_MODE,&value);
		if(PAS_EXIT_OK == ret)
		{
			sys_console_printf("cni_mac_mode is %d\r\n",value.cni_mac_mode);
		}
	}
	else if (GW10G_PON_CNI_CONFIG_MAC_STATE==cni_conf_type)
	{
		ret = GW10G_PAS_get_ethernet_cni_mac_mode(olt_id,cni_port,GW10G_PON_CNI_CONFIG_MAC_STATE,&value);
		if(PAS_EXIT_OK == ret)
		{
			sys_console_printf("cni_mac_stat is %d\r\n",value.cni_mac_state);
		}
	}

	return ret;
}

int GW_get_olt_pon_transmission(const PON_olt_id_t olt_id, const PON_pon_type pon_type)
{
	int ret = PAS_EXIT_OK;
	int status = 0;

	ret = GW10G_PAS_get_olt_pon_transmission(olt_id, pon_type,&status);
	if(PAS_EXIT_OK == ret)
	{
		sys_console_printf("PAS_get_olt_pon_transmission ok, bool is %d\r\n",status);
	}
	else
	{
		sys_console_printf("PAS_get_olt_pon_transmission err\r\n");
	}
	return ret;
}

int GW_set_olt_pon_transmission(const PON_olt_id_t olt_id, const PON_pon_type pon_type, const int status)
{
	int ret = PAS_EXIT_OK;
	int status = 0;

	ret = GW10G_PAS_set_olt_pon_transmission(olt_id, pon_type,status);
	if(PAS_EXIT_OK == ret)
	{
		sys_console_printf("PAS_set_olt_pon_transmission ok\r\n");
	}
	else
	{
		sys_console_printf("PAS_set_olt_pon_transmission err, ret is %d\r\n",ret);
	}
	return ret;
}
int GW_set_tranparent_mode_upstream(const PON_olt_id_t olt_id)
{
	int ret = PAS_EXIT_OK;

	NE_service_map_data_s ne_service_map_data = {0};
	INT32U					service_map_id = 0;

	/*upstream*/
	ne_service_map_data.values[0].priority_src = 0;
	ne_service_map_data.values[0].priority = 0;
	ne_service_map_data.values[0].queue_id = TM_QUEUE_ID_CNI0_PRIORITY0;
	ne_service_map_data.values[1].priority_src = 1;
	ne_service_map_data.values[1].priority = 1;
	ne_service_map_data.values[1].queue_id = TM_QUEUE_ID_CNI0_PRIORITY0;
	ne_service_map_data.values[2].priority_src = 2;
	ne_service_map_data.values[2].priority = 2;
	ne_service_map_data.values[2].queue_id = TM_QUEUE_ID_CNI0_PRIORITY1;
	ne_service_map_data.values[3].priority_src = 3;
	ne_service_map_data.values[3].priority = 3;
	ne_service_map_data.values[3].queue_id = TM_QUEUE_ID_CNI0_PRIORITY1;
	ne_service_map_data.values[4].priority_src = 4;
	ne_service_map_data.values[4].priority = 4;
	ne_service_map_data.values[4].queue_id = TM_QUEUE_ID_CNI0_PRIORITY2;
	ne_service_map_data.values[5].priority_src = 5;
	ne_service_map_data.values[5].priority = 5;
	ne_service_map_data.values[5].queue_id = TM_QUEUE_ID_CNI0_PRIORITY2;
	ne_service_map_data.values[6].priority_src = 6;
	ne_service_map_data.values[6].priority = 6;
	ne_service_map_data.values[6].queue_id = TM_QUEUE_ID_CNI0_PRIORITY3;
	ne_service_map_data.values[7].priority_src = 7;
	ne_service_map_data.values[7].priority = 7;
	ne_service_map_data.values[7].queue_id = TM_QUEUE_ID_CNI0_PRIORITY3;

	ne_service_map_data.default_tagged_queue_id = TM_QUEUE_ID_INVALID;
	ne_service_map_data.default_tagged_priority = 0;
	ne_service_map_data.default_untagged_queue_id = TM_QUEUE_ID_INVALID;
	ne_service_map_data.default_untagged_priority = 0;

	ret = GW10G_PAS_NE_service_map_create(olt_id, TM_PORT_PON_10G, NE_SERVICE_MAP_TRANSPARENT_UP, ne_service_map_data, &service_map_id);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_UP err,ret = %d \r\n",ret);
	}
	else
	{
		sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_UP ok,service_map_id = %d \r\n",service_map_id);
	}

	return ret;
	
}

int GW_set_tranparent_mode_downpon1(const PON_olt_id_t olt_id, const int port_cni)
{
	int ret = PAS_EXIT_OK;

	NE_service_map_data_s ne_service_map_data = {0};
	INT32U					service_map_id = 0;

	memset(&ne_service_map_data, 0xFF, sizeof(NE_service_map_data_s)); 
	/*PON1*/
	ne_service_map_data.values[0].priority_src = 0;
	ne_service_map_data.values[0].priority = 0;
	ne_service_map_data.values[0].queue_id = TM_QUEUE_ID_PON1_PRIORITY0;
	ne_service_map_data.values[1].priority_src = 1;
	ne_service_map_data.values[1].priority = 1;
	ne_service_map_data.values[1].queue_id = TM_QUEUE_ID_PON1_PRIORITY0;
	ne_service_map_data.values[2].priority_src = 2;
	ne_service_map_data.values[2].priority = 2;
	ne_service_map_data.values[2].queue_id = TM_QUEUE_ID_PON1_PRIORITY1;
	ne_service_map_data.values[3].priority_src = 3;
	ne_service_map_data.values[3].priority = 3;
	ne_service_map_data.values[3].queue_id = TM_QUEUE_ID_PON1_PRIORITY1;
	ne_service_map_data.values[4].priority_src = 4;
	ne_service_map_data.values[4].priority = 4;
	ne_service_map_data.values[4].queue_id = TM_QUEUE_ID_PON1_PRIORITY2;
	ne_service_map_data.values[5].priority_src = 5;
	ne_service_map_data.values[5].priority = 5;
	ne_service_map_data.values[5].queue_id = TM_QUEUE_ID_PON1_PRIORITY2;
	ne_service_map_data.values[6].priority_src = 6;
	ne_service_map_data.values[6].priority = 6;
	ne_service_map_data.values[6].queue_id = TM_QUEUE_ID_PON1_PRIORITY3;
	ne_service_map_data.values[7].priority_src = 7;
	ne_service_map_data.values[7].priority = 7;
	ne_service_map_data.values[7].queue_id = TM_QUEUE_ID_PON1_PRIORITY3;

	ne_service_map_data.default_tagged_queue_id = TM_QUEUE_ID_INVALID;
	ne_service_map_data.default_tagged_priority = 0;
	ne_service_map_data.default_untagged_queue_id = TM_QUEUE_ID_PON1_PRIORITY0;
	ne_service_map_data.default_untagged_priority = 0;
	ret = GW10G_PAS_NE_service_map_create( olt_id, port_cni, NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1, ne_service_map_data, &service_map_id);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1 err,ret = %d \r\n",ret);
	}
	else
	{
		sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1 ok,service_map_id = %d \r\n",service_map_id);
	}

	return ret;
}

int GW_set_tranparent_mode_downpon0(const PON_olt_id_t olt_id, const int port_cni)
{
	int ret = PAS_EXIT_OK;

	NE_service_map_data_s ne_service_map_data = {0};
	INT32U					service_map_id = 0;
    memset(&ne_service_map_data, 0xFF, sizeof(NE_service_map_data_s)); 
		/*PON0*/
	ne_service_map_data.values[0].priority_src = 0;
	ne_service_map_data.values[0].priority = 0;
	ne_service_map_data.values[0].queue_id = TM_QUEUE_ID_PON0_PRIORITY0;
	ne_service_map_data.values[1].priority_src = 1;
	ne_service_map_data.values[1].priority = 1;
	ne_service_map_data.values[1].queue_id = TM_QUEUE_ID_PON0_PRIORITY0;
	ne_service_map_data.values[2].priority_src = 2;
	ne_service_map_data.values[2].priority = 2;
	ne_service_map_data.values[2].queue_id = TM_QUEUE_ID_PON0_PRIORITY1;
	ne_service_map_data.values[3].priority_src = 3;
	ne_service_map_data.values[3].priority = 3;
	ne_service_map_data.values[3].queue_id = TM_QUEUE_ID_PON0_PRIORITY1;
	ne_service_map_data.values[4].priority_src = 4;
	ne_service_map_data.values[4].priority = 4;
	ne_service_map_data.values[4].queue_id = TM_QUEUE_ID_PON0_PRIORITY2;
	ne_service_map_data.values[5].priority_src = 5;
	ne_service_map_data.values[5].priority = 5;
	ne_service_map_data.values[5].queue_id = TM_QUEUE_ID_PON0_PRIORITY2;
	ne_service_map_data.values[6].priority_src = 6;
	ne_service_map_data.values[6].priority = 6;
	ne_service_map_data.values[6].queue_id = TM_QUEUE_ID_PON0_PRIORITY3;
	ne_service_map_data.values[7].priority_src = 7;
	ne_service_map_data.values[7].priority = 7;
	ne_service_map_data.values[7].queue_id = TM_QUEUE_ID_PON0_PRIORITY3;

	ne_service_map_data.default_tagged_queue_id = TM_QUEUE_ID_INVALID;
	ne_service_map_data.default_tagged_priority = 0;
	ne_service_map_data.default_untagged_queue_id = TM_QUEUE_ID_INVALID;
	ne_service_map_data.default_untagged_priority = 0;
	ret = GW10G_PAS_NE_service_map_create( olt_id, port_cni, NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0, ne_service_map_data, &service_map_id);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0 err,ret = %d \r\n",ret);
	}
	else
	{
		sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0 ok,service_map_id = %d \r\n",service_map_id);
	}

	return ret;
}
/*设置透传模式*/
int GW_set_tranparent_mode(const PON_olt_id_t olt_id)
{
	int ret = PAS_EXIT_OK;

	NE_service_map_data_s ne_service_map_data = {0};
	INT32U					service_map_id_up = 0;
    INT32U					service_map_id_down_pon1 = 0;
	INT32U					service_map_id_down_pon0 = 0;
	/*upstream*/
	ne_service_map_data.values[0].priority_src = 0;
	ne_service_map_data.values[0].priority = 0;
	ne_service_map_data.values[0].queue_id = TM_QUEUE_ID_CNI0_PRIORITY0;
	ne_service_map_data.values[1].priority_src = 1;
	ne_service_map_data.values[1].priority = 1;
	ne_service_map_data.values[1].queue_id = TM_QUEUE_ID_CNI0_PRIORITY0;
	ne_service_map_data.values[2].priority_src = 2;
	ne_service_map_data.values[2].priority = 2;
	ne_service_map_data.values[2].queue_id = TM_QUEUE_ID_CNI0_PRIORITY1;
	ne_service_map_data.values[3].priority_src = 3;
	ne_service_map_data.values[3].priority = 3;
	ne_service_map_data.values[3].queue_id = TM_QUEUE_ID_CNI0_PRIORITY1;
	ne_service_map_data.values[4].priority_src = 4;
	ne_service_map_data.values[4].priority = 4;
	ne_service_map_data.values[4].queue_id = TM_QUEUE_ID_CNI0_PRIORITY2;
	ne_service_map_data.values[5].priority_src = 5;
	ne_service_map_data.values[5].priority = 5;
	ne_service_map_data.values[5].queue_id = TM_QUEUE_ID_CNI0_PRIORITY2;
	ne_service_map_data.values[6].priority_src = 6;
	ne_service_map_data.values[6].priority = 6;
	ne_service_map_data.values[6].queue_id = TM_QUEUE_ID_CNI0_PRIORITY3;
	ne_service_map_data.values[7].priority_src = 7;
	ne_service_map_data.values[7].priority = 7;
	ne_service_map_data.values[7].queue_id = TM_QUEUE_ID_CNI0_PRIORITY3;

	ne_service_map_data.default_tagged_queue_id = TM_QUEUE_ID_INVALID;
	ne_service_map_data.default_tagged_priority = 0;
	ne_service_map_data.default_untagged_queue_id = TM_QUEUE_ID_INVALID;
	ne_service_map_data.default_untagged_priority = 0;

	ret = GW10G_PAS_NE_service_map_create(olt_id, TM_PORT_PON_10G, NE_SERVICE_MAP_TRANSPARENT_UP, ne_service_map_data, &service_map_id_up);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_UP err,ret = %d \r\n",ret);
	}
	else
	{
		sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_UP ok,service_map_id = %d \r\n",service_map_id_up);
	}

	/*PON1*/
	ne_service_map_data.values[0].priority_src = 0;
	ne_service_map_data.values[0].priority = 0;
	ne_service_map_data.values[0].queue_id = TM_QUEUE_ID_PON1_PRIORITY0;
	ne_service_map_data.values[1].priority_src = 1;
	ne_service_map_data.values[1].priority = 1;
	ne_service_map_data.values[1].queue_id = TM_QUEUE_ID_PON1_PRIORITY0;
	ne_service_map_data.values[2].priority_src = 2;
	ne_service_map_data.values[2].priority = 2;
	ne_service_map_data.values[2].queue_id = TM_QUEUE_ID_PON1_PRIORITY1;
	ne_service_map_data.values[3].priority_src = 3;
	ne_service_map_data.values[3].priority = 3;
	ne_service_map_data.values[3].queue_id = TM_QUEUE_ID_PON1_PRIORITY1;
	ne_service_map_data.values[4].priority_src = 4;
	ne_service_map_data.values[4].priority = 4;
	ne_service_map_data.values[4].queue_id = TM_QUEUE_ID_PON1_PRIORITY2;
	ne_service_map_data.values[5].priority_src = 5;
	ne_service_map_data.values[5].priority = 5;
	ne_service_map_data.values[5].queue_id = TM_QUEUE_ID_PON1_PRIORITY2;
	ne_service_map_data.values[6].priority_src = 6;
	ne_service_map_data.values[6].priority = 6;
	ne_service_map_data.values[6].queue_id = TM_QUEUE_ID_PON1_PRIORITY3;
	ne_service_map_data.values[7].priority_src = 7;
	ne_service_map_data.values[7].priority = 7;
	ne_service_map_data.values[7].queue_id = TM_QUEUE_ID_PON1_PRIORITY3;

	ne_service_map_data.default_tagged_queue_id = TM_QUEUE_ID_INVALID;
	ne_service_map_data.default_tagged_priority = 0;
	ne_service_map_data.default_untagged_queue_id = TM_QUEUE_ID_PON1_PRIORITY0;
	ne_service_map_data.default_untagged_priority = 0;
	ret = GW10G_PAS_NE_service_map_create( olt_id, TM_PORT_CNI0, NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1, ne_service_map_data, &service_map_id_down_pon1);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1 err,ret = %d \r\n",ret);
	}
	else
	{
		sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1 ok,service_map_id = %d \r\n",service_map_id_down_pon1);
	}


	/*PON0*/
	ne_service_map_data.values[0].priority_src = 0;
	ne_service_map_data.values[0].priority = 0;
	ne_service_map_data.values[0].queue_id = TM_QUEUE_ID_PON0_PRIORITY0;
	ne_service_map_data.values[1].priority_src = 1;
	ne_service_map_data.values[1].priority = 1;
	ne_service_map_data.values[1].queue_id = TM_QUEUE_ID_PON0_PRIORITY0;
	ne_service_map_data.values[2].priority_src = 2;
	ne_service_map_data.values[2].priority = 2;
	ne_service_map_data.values[2].queue_id = TM_QUEUE_ID_PON0_PRIORITY1;
	ne_service_map_data.values[3].priority_src = 3;
	ne_service_map_data.values[3].priority = 3;
	ne_service_map_data.values[3].queue_id = TM_QUEUE_ID_PON0_PRIORITY1;
	ne_service_map_data.values[4].priority_src = 4;
	ne_service_map_data.values[4].priority = 4;
	ne_service_map_data.values[4].queue_id = TM_QUEUE_ID_PON0_PRIORITY2;
	ne_service_map_data.values[5].priority_src = 5;
	ne_service_map_data.values[5].priority = 5;
	ne_service_map_data.values[5].queue_id = TM_QUEUE_ID_PON0_PRIORITY2;
	ne_service_map_data.values[6].priority_src = 6;
	ne_service_map_data.values[6].priority = 6;
	ne_service_map_data.values[6].queue_id = TM_QUEUE_ID_PON0_PRIORITY3;
	ne_service_map_data.values[7].priority_src = 7;
	ne_service_map_data.values[7].priority = 7;
	ne_service_map_data.values[7].queue_id = TM_QUEUE_ID_PON0_PRIORITY3;

	ne_service_map_data.default_tagged_queue_id = TM_QUEUE_ID_INVALID;
	ne_service_map_data.default_tagged_priority = 0;
	ne_service_map_data.default_untagged_queue_id = TM_QUEUE_ID_PON0_PRIORITY0;
	ne_service_map_data.default_untagged_priority = 0;
	ret = GW10G_PAS_NE_service_map_create( olt_id, TM_PORT_CNI0, NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0, ne_service_map_data, &service_map_id_down_pon0);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0 err,ret = %d \r\n",ret);
	}
	else
	{
		sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0 ok,service_map_id = %d \r\n",service_map_id_down_pon0);
	}

	return ret;
	
}

int pas_1g_transparent_mode(uint32 olt_device_id) 
{ 
    int rc = PAS_EXIT_OK; 
    NE_service_map_type_e map_type; 
    TM_port_e port; 
    NE_service_map_data_s vlan_down_map; 
    NE_service_map_data_s vlan_up_map; 
    INT32U down_id, up_id, queue_base, i; 

    /* upstream */ 
    { 
        map_type = NE_SERVICE_MAP_TRANSPARENT_UP; 
        port = TM_PORT_PON_1G; 
        queue_base = TM_QUEUE_ID_CNI0_PRIORITY0; 

        memset(&vlan_up_map, 0xFF, sizeof(NE_service_map_data_s)); 
        vlan_up_map.default_tagged_queue_id = TM_QUEUE_ID_INVALID; 
        vlan_up_map.default_tagged_priority = 0; 
        vlan_up_map.default_untagged_queue_id = TM_QUEUE_ID_INVALID; 
        vlan_up_map.default_untagged_priority = 0; 
        for( i = 0; i < 8 ; ++i ) 
        { 
            vlan_up_map.values[i].priority_src = i; 
            vlan_up_map.values[i].priority = i; 
            vlan_up_map.values[i].queue_id = queue_base + i; 
        } 
        
        rc |= GW10G_PAS_NE_service_map_create(olt_device_id, port, map_type, vlan_up_map, &up_id ); 
        if(PAS_EXIT_OK != rc)
		{
			sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_UP err,ret = %d \r\n",rc);
		}
		else
		{
			sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_UP ok,service_map_id = %d \r\n",up_id);
		}
     
    } 
    /* downstream */ 
    { 
        map_type = NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1; 
        port = TM_PORT_CNI0; 
        queue_base = TM_QUEUE_ID_PON1_PRIORITY0; 
        
        memset(&vlan_down_map, 0xFF, sizeof(NE_service_map_data_s)); 
        vlan_down_map.default_tagged_queue_id = TM_QUEUE_ID_INVALID; 
        vlan_down_map.default_tagged_priority = 0; 
        vlan_down_map.default_untagged_queue_id = TM_QUEUE_ID_PON1_PRIORITY0; 
        vlan_down_map.default_untagged_priority = 0; 
        for( i = 0; i < 8 ; i++ ) 
        { 
            vlan_down_map.values[i].priority_src = i; 
            vlan_down_map.values[i].priority = i; 
            vlan_down_map.values[i].queue_id = queue_base + i; 
        } 
        
        rc |= GW10G_PAS_NE_service_map_create(olt_device_id, port, map_type, vlan_down_map, &down_id ); 
        if(PAS_EXIT_OK != rc)
		{
			sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1 err,ret = %d \r\n",rc);
		}
		else
		{
			sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1 ok,service_map_id = %d \r\n",down_id);
		}
        
    } 
    { 
        map_type = NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0; 
        port = TM_PORT_CNI0; 
        queue_base = TM_QUEUE_ID_PON0_PRIORITY0; 

        memset(&vlan_down_map, 0xFF, sizeof(NE_service_map_data_s)); 
        vlan_down_map.default_tagged_queue_id = TM_QUEUE_ID_INVALID; 
        vlan_down_map.default_tagged_priority = 0; 
        vlan_down_map.default_untagged_queue_id = TM_QUEUE_ID_PON0_PRIORITY0; 
        vlan_down_map.default_untagged_priority = 0; 
        for( i = 0; i < 8 ; i++ ) 
        { 
            vlan_down_map.values[i].priority_src = i; 
            vlan_down_map.values[i].priority = i; 
            vlan_down_map.values[i].queue_id = queue_base + i; 
        } 
        
        rc |= GW10G_PAS_NE_service_map_create(olt_device_id, port, map_type, vlan_down_map, &down_id ); 
        if(PAS_EXIT_OK != rc)
		{
			sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0 err,ret = %d \r\n",rc);
		}
		else
		{
			sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0 ok,service_map_id = %d \r\n",down_id);
		}
    } 
    return rc; 
} 

int pas_10g_transparent_mode(uint32 olt_device_id) 
{ 
    int rc = PAS_EXIT_OK; 
    NE_service_map_type_e map_type; 
    TM_port_e port; 
    NE_service_map_data_s vlan_down_map; 
    NE_service_map_data_s vlan_up_map; 
    INT32U down_id, up_id, queue_base, i; 

    /* upstream */ 
    { 
        map_type = NE_SERVICE_MAP_TRANSPARENT_UP; 
        port = TM_PORT_PON_10G; 
        queue_base = TM_QUEUE_ID_CNI0_PRIORITY0; 

        memset(&vlan_up_map, 0xFF, sizeof(NE_service_map_data_s)); 
        vlan_up_map.default_tagged_queue_id = TM_QUEUE_ID_INVALID; 
        vlan_up_map.default_tagged_priority = 0; 
        vlan_up_map.default_untagged_queue_id = TM_QUEUE_ID_INVALID; 
        vlan_up_map.default_untagged_priority = 0; 
        for( i = 0; i < 8 ; ++i ) 
        { 
            vlan_up_map.values[i].priority_src = i; 
            vlan_up_map.values[i].priority = i; 
            vlan_up_map.values[i].queue_id = queue_base + i; 
        } 
        
        rc |= GW10G_PAS_NE_service_map_create(olt_device_id, port, map_type, vlan_up_map, &up_id ); 
        if(PAS_EXIT_OK != rc)
		{
			sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_UP err,ret = %d \r\n",rc);
		}
		else
		{
			sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_UP ok,service_map_id = %d \r\n",up_id);
		}
     
    } 
    /* downstream */ 
    { 
        map_type = NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1; 
        port = TM_PORT_CNI0; 
        queue_base = TM_QUEUE_ID_PON1_PRIORITY0; 
        
        memset(&vlan_down_map, 0xFF, sizeof(NE_service_map_data_s)); 
        vlan_down_map.default_tagged_queue_id = TM_QUEUE_ID_INVALID; 
        vlan_down_map.default_tagged_priority = 0; 
        vlan_down_map.default_untagged_queue_id = TM_QUEUE_ID_PON1_PRIORITY0; 
        vlan_down_map.default_untagged_priority = 0; 
        for( i = 0; i < 8 ; i++ ) 
        { 
            vlan_down_map.values[i].priority_src = i; 
            vlan_down_map.values[i].priority = i; 
            vlan_down_map.values[i].queue_id = queue_base + i; 
        } 
        
        rc |= GW10G_PAS_NE_service_map_create(olt_device_id, port, map_type, vlan_down_map, &down_id ); 
        if(PAS_EXIT_OK != rc)
		{
			sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1 err,ret = %d \r\n",rc);
		}
		else
		{
			sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1 ok,service_map_id = %d \r\n",down_id);
		}
        
    } 
    { 
        map_type = NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0; 
        port = TM_PORT_CNI0; 
        queue_base = TM_QUEUE_ID_PON0_PRIORITY0; 

        memset(&vlan_down_map, 0xFF, sizeof(NE_service_map_data_s)); 
        vlan_down_map.default_tagged_queue_id = TM_QUEUE_ID_INVALID; 
        vlan_down_map.default_tagged_priority = 0; 
        vlan_down_map.default_untagged_queue_id = TM_QUEUE_ID_PON0_PRIORITY0; 
        vlan_down_map.default_untagged_priority = 0; 
        for( i = 0; i < 8 ; i++ ) 
        { 
            vlan_down_map.values[i].priority_src = i; 
            vlan_down_map.values[i].priority = i; 
            vlan_down_map.values[i].queue_id = queue_base + i; 
        } 
        
        rc |= GW10G_PAS_NE_service_map_create(olt_device_id, port, map_type, vlan_down_map, &down_id ); 
         if(PAS_EXIT_OK != rc)
		{
			sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0 err,ret = %d \r\n",rc);
		}
		else
		{
			sys_console_printf("PAS_NE_service_map_create NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0 ok,service_map_id = %d \r\n",down_id);
		}
    } 
    return rc; 
} 
int GW_set_bw_down(const short int olt_id, const short int llid, int rate_id, const int cir, const int cbs, const int eir, const int ebs)
{
	int ret = PAS_EXIT_OK;
    TM_policy_s policy_profile;
	NE_service_search_type_e   search;
    
	NE_user_data_s rule;
	INT32U rule_id = 0;
	
    NE_service_map_type_e map_type; 
    TM_port_e port; 
    NE_service_map_data_s vlan_down_map; 
    
    INT32U down_id, queue_base, i; 
	sys_console_printf("Enter into GW_set_bw_down, olt_id:%d,llid:%d,cir:%d,cbs:%d,eir:%d,ebs:%d\r\n",
		olt_id,llid,cir,cbs,eir,ebs);

	search = NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0;
#if 0
	ret = PAS_ne_service_create( olt_id,TM_PORT_CNI0,search,&service_id );
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_ne_service_create err, ret is %d\r\n",ret);
		
		return ret;
	}
	else
	{
		sys_console_printf("PAS_ne_service_create ok, service_id is %d\r\n",service_id);
	}
#endif
	
	memset(&policy_profile,0,sizeof(TM_policy_s));
	
	ret = GW10G_PAS_TM_policy_profile_create(olt_id, &policy_profile);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_TM_policy_profile_create err, ret is %d\r\n",ret);
		
		return ret;
	}
	else
	{
		sys_console_printf("PAS_TM_policy_profile_create OK, policy_id is %d, CIR is %d, CBS is %d, EIR is %d, EBS is %d\r\n",
			policy_profile.policy_id,policy_profile.CIR,policy_profile.CBS,policy_profile.EIR,policy_profile.EBS);
	}

	policy_profile.CIR = cir;
	policy_profile.CBS = cbs;
	policy_profile.EIR = eir;
	policy_profile.EBS = ebs;

	ret = GW10G_PAS_TM_policy_profile_set(olt_id, &policy_profile);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_TM_policy_profile_set is err, ret is %d\r\n",ret);
		
		return ret;
	}
	else
	{
		sys_console_printf("PAS_TM_policy_profile_set is ok,policy_id: %d, CIR: %d, CBS: %d, EIR: %d, EBS: %d\r\n",
			policy_profile.policy_id,policy_profile.CIR,policy_profile.CBS,policy_profile.EIR,policy_profile.EBS);
	}

	ret = GW10G_PAS_TM_rate_limit_set(olt_id, NE_DIRECTION_DOWN, policy_profile.policy_id, rate_id);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_TM_rate_limit_set err, ret is %d\r\n",ret);
		
		return ret;
	}
	else
	{
		sys_console_printf("PAS_TM_rate_limit_set ok, policy_id is %d,rate_limit_id is %d\r\n",policy_profile.policy_id,rate_id);
	}

	#if 1
    { 
		
        map_type = NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0; 
        port = TM_PORT_CNI0; 
        queue_base = TM_QUEUE_ID_PON0_PRIORITY0; 

        memset(&vlan_down_map, 0xFF, sizeof(NE_service_map_data_s)); 
        vlan_down_map.default_tagged_queue_id = TM_QUEUE_ID_INVALID; 
        vlan_down_map.default_tagged_priority = 0; 
        vlan_down_map.default_untagged_queue_id = TM_QUEUE_ID_INVALID; 
        vlan_down_map.default_untagged_priority = 0; 
        for( i = 0; i < 8 ; i++ ) 
        { 
            vlan_down_map.values[i].priority_src = i; 
            vlan_down_map.values[i].priority = i; 
            vlan_down_map.values[i].queue_id = queue_base + i; 
        } 
        
        ret |= GW10G_PAS_NE_service_map_create(olt_id, port, map_type, vlan_down_map, &down_id ); 
         if(PAS_EXIT_OK != ret)
		{
			sys_console_printf("PAS_NE_service_map_create  err,ret = %d \r\n",ret);
		}
		else
		{
			sys_console_printf("PAS_NE_service_map_create  ok,service_map_id = %d \r\n",down_id);
		}
	
    } 
	

	NE_USER_DATA_DEFAULT(rule);
	rule.id = down_id;
	rule.values.search_type = NE_SERVICE_SEARCH_TYPE_SVC_ETHERTYPE_USR_LLID;
	rule.values.llid = llid;
	rule.values.ethertype = 0x0800;
	rule.values.pr_vid = TM_NE_NO_VLAN;
	rule.values.pr_prio = 3;
	rule.values.pr_vtype = NE_VLAN_TYPE_V8100;
	rule.values.sc_vid = TM_NE_NO_VLAN;
	rule.values.sc_prio = 0;
	rule.values.sc_vtype = NE_VLAN_TYPE_V9100;
	rule.user_id = llid;
	rule.action.dp_queue_id = 0;
	rule.action.sniff_queue_id = 0;
	rule.action.cpu_queue_id = TM_QUEUE_ID_INVALID;
	rule.action.dp_rate_limit0 = rate_id;
	rule.action.dp_rate_limit1 = 0;
	rule.action.dp_rate_limit2 = 0;
	rule.action.cpu_flow_id = 0;
	rule.action.ellid = llid;
	rule.action.edit.rule_type = NE_EDIT_TYPE_NONE;
	#endif

	
	ret = GW10G_PAS_NE_user_rule_add(olt_id, &rule, &rule_id);
	 
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_NE_user_rule_add err, ret is %d\r\n",ret);
		
		return ret;
	}
	else
	{
		sys_console_printf("PAS_NE_user_rule_add ok, rule_id is %d\r\n",rule_id);
	}

	return ret;
}

int GW_set_bw_down1(const short int olt_id, const short int llid, int rate_id, const int cir, const int cbs, const int eir, const int ebs)
{
	short int ret = PAS_EXIT_OK;
	TM_policy_s policy_profile = {0};
	TM_policy_s policy_profile_test = {0};
	

	NE_attributes_s attribs = {0};
	INT32U NE_attrib_id = 0;
	NE_action_value_u value = {0};
	NE_action_type_e act_type = 0;

    
	sys_console_printf("Enter into GW_set_bw_down1, olt_id:%d,llid:%d,cir:%d,cbs:%d,eir:%d,ebs:%d\r\n",
		olt_id,llid,cir,cbs,eir,ebs);
	#if 1
	ret = GW10G_PAS_TM_policy_profile_create(olt_id, &policy_profile);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_TM_policy_profile_create err, ret is %d\r\n",ret);
		
		return ret;
	}
	else
	{
		sys_console_printf("PAS_TM_policy_profile_create OK, policy_id is %d, CIR is %d, CBS is %d, EIR is %d, EBS is %d\r\n",
			policy_profile.policy_id,policy_profile.CIR,policy_profile.CBS,policy_profile.EIR,policy_profile.EBS);
	}
	

	policy_profile.CIR = cir;
	policy_profile.CBS = cbs;
	policy_profile.EIR = eir;
	policy_profile.EBS = ebs;

	ret = GW10G_PAS_TM_policy_profile_set(olt_id, &policy_profile);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_TM_policy_profile_set is err, ret is %d\r\n",ret);
		
		return ret;
	}
	else
	{
		sys_console_printf("PAS_TM_policy_profile_set is ok,policy_id: %d, CIR: %d, CBS: %d, EIR: %d, EBS: %d\r\n",
			policy_profile.policy_id,policy_profile.CIR,policy_profile.CBS,policy_profile.EIR,policy_profile.EBS);
	}

	ret = GW10G_PAS_TM_rate_limit_set(olt_id, NE_DIRECTION_DOWN, policy_profile.policy_id, rate_id);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_TM_rate_limit_set err, ret is %d\r\n",ret);
		
		return ret;
	}
	else
	{
		sys_console_printf("PAS_TM_rate_limit_set ok, policy_id is %d,rate_limit_id is %d\r\n",policy_profile.policy_id,rate_id);
	}

    attribs.num = 1;
	attribs.attributes[0] = NE_ATTRIBUTE_CHANNEL;
	attribs.values[0] = 0;

	ret = GW10G_PAS_ne_attrib_group_create(  olt_id,NE_DIRECTION_DOWN,&attribs,&NE_attrib_id);
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_ne_attrib_group_create err,ret is %d\r\n",ret);
		
		return ret;
	}
	else
	{
		sys_console_printf("PAS_ne_attrib_group_create OK,NE_attrib_id is %d\r\n",NE_attrib_id);
	}

	act_type = NE_ACTION_TYPE_ELLID;
	value.ellid = llid;
	ret =  GW10G_PAS_ne_action_set( olt_id,NE_attrib_id,act_type,5,value );
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_ne_action_set NE_ACTION_TYPE_ELLID err,ret is %d\r\n",ret);
		
	}
	else
	{
		sys_console_printf("PAS_ne_action_set NE_ACTION_TYPE_ELLID ok, NE_attrib_id is %d, llid is %d\r\n",NE_attrib_id,value.ellid);
	}

	act_type = NE_ACTION_TYPE_DP_RATE_LIMIT0;
	value.dp_rate_limit0 = rate_id;
	ret =  GW10G_PAS_ne_action_set( olt_id,NE_attrib_id,act_type,5,value );
	if(PAS_EXIT_OK != ret)
	{
		sys_console_printf("PAS_ne_action_set NE_ACTION_TYPE_DP_RATE_LIMIT0 err,ret is %d\r\n",ret);
		
		return ret;
	}
	else
	{
		sys_console_printf("PAS_ne_action_set NE_ACTION_TYPE_DP_RATE_LIMIT0 ok, NE_attrib_id is %d,dp_rate_limit0 is %d\r\n",NE_attrib_id,value.dp_rate_limit0);
	}
   
   
	#endif
	
	return ret;
	
}
/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-14*/
int GW_set_fanspeed_fast()
{
	sys_console_printf("Init fanspeed fast\r\n");
	fan_ctrl_i2c_write(15,0x11,0x2);
	fan_ctrl_i2c_write(15,0x15,0x2);
	fan_ctrl_i2c_write(15,0x26,0x70);
	fan_ctrl_i2c_write(15,0x27,0x70);

	fan_ctrl_i2c_write(16,0x11,0x2);
	fan_ctrl_i2c_write(16,0x15,0x2);
	fan_ctrl_i2c_write(16,0x26,0x70);
	fan_ctrl_i2c_write(16,0x27,0x70);

	fan_ctrl_i2c_write(17,0x11,0x2);
	fan_ctrl_i2c_write(17,0x15,0x2);
	fan_ctrl_i2c_write(17,0x26,0x70);
	fan_ctrl_i2c_write(17,0x27,0x70);
}
/*End: for 10G EPON of PMC8411 by jinhl @2012-11-14*/
#endif

/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/

/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
int Pon_RemoveOlt(short int olt_id, bool send_shutdown_msg_to_olt, bool reset_olt)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
    short int PonChipType = 0;

	OLT_ASSERT(olt_id);
	
	PonChipType = V2R1_GetPonchipType( olt_id );

#if 1
    iRlt = OLT_RemoveOlt(olt_id, send_shutdown_msg_to_olt, reset_olt);
#else
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
	    iRlt = GW10G_Remove_olt(olt_id, send_shutdown_msg_to_olt, reset_olt);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(PonChipType))
	{
	    iRlt = Remove_olt(olt_id, send_shutdown_msg_to_olt, reset_olt);
	}
#endif    

    /* 需要通知OLT移除事件 */
    if ( OLT_PONCHIP_ISBCM(PonChipType) )
    {
        Pon_event_handler(PON_EVT_HANDLER_OLT_RMV, DATA2PTR(olt_id));
    }

	return iRlt;
}

int Pon_SendCliCommand(short int olt_id, short int chip_type, unsigned short size, unsigned char *command)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
    
	OLT_ASSERT(olt_id);
	
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(chip_type))
	{
	    iRlt = GW10G_PAS_send_cli_command(olt_id, size, command);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(chip_type))
	{
	    iRlt = PAS_send_cli_command(olt_id, size, command);
	}

	return iRlt;
}

int Pon_SetCniPortMacConfiguration(short int olt_id, short int chip_type, PON_olt_cni_port_mac_configuration_t *olt_cni_port_mac_configuration)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
    
	OLT_ASSERT(olt_id);
		
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(chip_type))
	{
	    iRlt = GW10GAdp_set_olt_cni_port_mac_configuration(olt_id, olt_cni_port_mac_configuration);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(chip_type))
	{
	    iRlt = PAS_set_olt_cni_port_mac_configuration(olt_id, olt_cni_port_mac_configuration);
	}

	return iRlt;
}

int Pon_GetCniPortMacConfiguration(short int olt_id, short int chip_type, PON_olt_cni_port_mac_configuration_t *olt_cni_port_mac_configuration)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
   
	OLT_ASSERT(olt_id);
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(chip_type))
	{
	    iRlt = GW10GAdp_get_olt_cni_port_mac_configuration(olt_id, olt_cni_port_mac_configuration);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(chip_type))
	{
	    iRlt = PAS_get_olt_cni_port_mac_configuration(olt_id, olt_cni_port_mac_configuration);
	}

	return iRlt;
}

int Pon_LoadOltBinary(short int olt_id, short int chip_type, PON_indexed_binary_t  *olt_binary)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
   
	OLT_ASSERT(olt_id);
	
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(chip_type))
	{
	    iRlt = GW10G_PAS_load_olt_binary(olt_id, olt_binary);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(chip_type))
	{
	    iRlt = PAS_load_olt_binary(olt_id, olt_binary);
	}

	return iRlt;
}

int Pon_StartDbaAlgorithm(short int olt_id, short int chip_type, bool use_default_dba, short int initialization_data_size, void *initialization_data)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
    
	OLT_ASSERT(olt_id);
	
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(chip_type))
	{
	    iRlt = GW10G_PAS_start_dba_algorithm(olt_id, use_default_dba, initialization_data_size, initialization_data);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(chip_type))
	{
	    iRlt = PAS_start_dba_algorithm(olt_id, use_default_dba, initialization_data_size, initialization_data);
	}

	return iRlt;
}

int Pon_HostRecovery(short int olt_id, PON_olt_mode_t *olt_mode, PON_olt_initialization_parameters_t *initialization_configuration,
	unsigned short int *dba_mode, PON_llid_t *link_test, PON_active_llids_t *active_llids)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
    short int PonChipType = 0;

	OLT_ASSERT(olt_id);
	
	PonChipType = V2R1_GetPonchipType( olt_id );

#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
	    iRlt = GW10G_PAS_olt_host_recovery(olt_id, olt_mode, initialization_configuration, dba_mode, link_test, active_llids);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(PonChipType))
	{
	    iRlt = PAS_olt_host_recovery(olt_id, olt_mode, initialization_configuration, dba_mode, link_test, active_llids);
	}


	return iRlt;
}

int Pon_GetOltModeQuery(short int olt_id, PON_olt_mode_t *olt_mode, mac_address_t mac_address,
	PON_olt_init_parameters_t *initialization_configuration, unsigned short int *dba_mode, PON_llid_t *link_test, PON_active_llids_t *active_llids)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
    short int PonChipType = 0;

	OLT_ASSERT(olt_id);
	
	PonChipType = V2R1_GetPonchipType( olt_id );

#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
	    iRlt = GW10GAdp_get_olt_mode_query(olt_id, olt_mode, mac_address, initialization_configuration, dba_mode, link_test, active_llids);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(PonChipType))
	{
	    iRlt = PAS_get_olt_mode_query_v4(olt_id, olt_mode, mac_address, initialization_configuration, dba_mode, link_test, active_llids);
	}

	return iRlt;
}

int Pon_COMMInitOlt ( short int  olt_id)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
    short int PonChipType = 0;

	OLT_ASSERT(olt_id);
	
	PonChipType = V2R1_GetPonchipType( olt_id );

#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
	    iRlt = GW10G_PASCOMM_init_olt(olt_id);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(PonChipType))
	{
	    iRlt = PASCOMM_init_olt(olt_id);
	}

	return iRlt;
}

int Pon_GetDeviceVersions(short int olt_id, short int device_id, PAS_device_versions_t *device_versions,
	PON_remote_mgnt_version_t *remote_mgnt_version, unsigned long int *critical_events_counter)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
    short int PonChipType = 0;

	OLT_ASSERT(olt_id);
	
	PonChipType = V2R1_GetPonchipType( olt_id );

#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
	    iRlt = GW10G_Get_device_versions(olt_id, device_id, device_versions, remote_mgnt_version, critical_events_counter);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(PonChipType))
	{
	    iRlt = Get_device_versions(olt_id, device_id, device_versions, remote_mgnt_version, critical_events_counter);
	}

	return iRlt;
}

void Pon_MonitorEnable(bool status)
{
    
#if defined(_EPON_10G_PMC_SUPPORT_)            
    if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
    	GW10G_Monitor_enable(status);

	}
	else
#endif
	{
		Monitor_enable(status);
    	
	}

	
}

char *Pon_BooleanString ( const bool  parameter )
{
    int iRlt = OLT_ERR_NOTSUPPORT;
#if defined(_EPON_10G_PMC_SUPPORT_)            
    if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
    	iRlt = GW10G_PON_boolean_string(parameter);

	}
	else
#endif
	{
		iRlt = PON_boolean_string(parameter);
    	
	}

	return iRlt;
    
}

int Pon_ReceivePollingThreadFunc1(void *lpPacketRecBuf,unsigned int buf_data_size)
{
    int iRlt;
    
#if defined(_EPON_10G_PMC_SUPPORT_)            
    if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
    	iRlt = GW10G_Receive_polling_thread_func1(lpPacketRecBuf, buf_data_size);

	}
	else
#endif
	{
		iRlt = Receive_polling_thread_func1(lpPacketRecBuf, buf_data_size);
    	
	}

	return iRlt;
    
}

int Pon_SetClassificationRule(short int olt_id, PON_pon_network_traffic_direction_t direction, PON_olt_classification_t classification_entity, void *classification_parameter, PON_olt_classifier_destination_t destination)
{
    int iRlt = 0;
    short int PonChipType = 0;

	OLT_ASSERT(olt_id);
	
	PonChipType = V2R1_GetPonchipType( olt_id );

#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
	    iRlt = GW10GAdp_set_classification_rule(olt_id, direction, classification_entity, classification_parameter, destination);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(PonChipType))
	{
	    iRlt = PAS_set_classification_rule(olt_id, direction, classification_entity, classification_parameter, destination);
	}

	return iRlt;
}

int Pon_GetClassificationRule(short int olt_id, PON_pon_network_traffic_direction_t direction, PON_olt_classification_t classification_entity, void *classification_parameter, PON_olt_classifier_destination_t *destination)
{
    int iRlt;
    short int PonChipType = 0;

	OLT_ASSERT(olt_id);
	
	PonChipType = V2R1_GetPonchipType( olt_id );

#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
	    iRlt = GW10GAdp_get_classification_rule(olt_id, direction, classification_entity, classification_parameter, destination);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(PonChipType))
	{
	    iRlt = PAS_get_classification_rule(olt_id, direction, classification_entity, classification_parameter, destination);
	}

	return iRlt;
}

#if defined(_EPON_10G_PMC_SUPPORT_)            
int Pon_SetTransparentMode(short int PonPortIdx, short int chip_type)
{
    int iRlt = PAS_EXIT_OK;
    
	if(OLT_PONCHIP_ISPAS10G(chip_type))
	{
	    iRlt = pas_10g_transparent_mode(PonPortIdx);
	}
	
	return iRlt;

}
#endif

int Pon_PLATO_ErrRet(int val, int PonPortIdx)
{
    int iRlt = RERROR;
	short int PonChipType = RERROR;
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
		switch (val)
        {
            case PLATO4_ECODE_MIN_GREATER_THAN_MAX:
            case PLATO4_ECODE_MIN_TOTAL_TOO_LARGE:
            case PLATO4_ECODE_ILLEGAL_GR_BW:
            case PLATO4_ECODE_ILLEGAL_BE_BW:
            case PLATO4_ECODE_ILLEGAL_FIXED_BW:
               iRlt = V2R1_EXCEED_RANGE;
               break;
            case PLATO4_ECODE_UNKNOWN_LLID:
            case PLATO4_ECODE_TOO_MANY_LLIDS:
            case PLATO4_ECODE_ILLEGAL_LLID:
            case PAS_ONU_NOT_AVAILABLE:    
               iRlt = V2R1_ONU_OFF_LINE;
               break;
            default:
               iRlt = RERROR;
        }
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(PonChipType))
	{
	    switch (val)
        {
            case PLATO3_ECODE_MIN_GREATER_THAN_MAX:
            case PLATO3_ECODE_MIN_TOTAL_TOO_LARGE:
            case PLATO3_ECODE_ILLEGAL_GR_BW:
            case PLATO3_ECODE_ILLEGAL_BE_BW:
            case PLATO3_ECODE_ILLEGAL_FIXED_BW:
               iRlt = V2R1_EXCEED_RANGE;
               break;
            case PLATO3_ECODE_UNKNOWN_LLID:
            case PLATO3_ECODE_TOO_MANY_LLIDS:
            case PLATO3_ECODE_ILLEGAL_LLID:
            case PAS_ONU_NOT_AVAILABLE:    
               iRlt = V2R1_ONU_OFF_LINE;
               break;
            default:
               iRlt = RERROR;
        }
	}

	return iRlt;
}

void Pon_Show_SLAInfo(short int PonPortIdx, ONU_SLA_INFO_t SLA_Info)
{
    
	short int PonChipType = RERROR;
	short int DBA_error_code = RERROR;
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);

#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
	    PLATO4_SLA_t SLA;

        SLA = SLA_Info.SLA.SLA4;
		DBA_error_code = SLA_Info.DBA_ErrCode;
		sys_console_printf("\r\nGet SLA Info ok \r\n");
		sys_console_printf(" SLA.class = %d \r\n", SLA.class );
		sys_console_printf("SLA.fixed_packet_size = %d\r\n",SLA.fixed_packet_size );
		sys_console_printf("SLA.fixed_bw = %d\r\n",SLA.fixed_bw );
		sys_console_printf("SLA.fixed_bw_fine = %d\r\n",SLA.fixed_bw_fine );
		sys_console_printf(" SLA.max_gr_bw = %d \r\n", SLA.max_gr_bw );
		sys_console_printf(" SLA.max_gr_bw_fine = %d \r\n", SLA.max_gr_bw_fine );
		sys_console_printf(" SLA.max_be_bw = %d \r\n", SLA.max_be_bw );
		sys_console_printf(" SLA.max_be_bw_fine = %d \r\n", SLA.max_be_bw_fine );
		sys_console_printf(" error_code %d \r\n", DBA_error_code );

		
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(PonChipType))
	{
#ifdef PLATO_DBA_V3
		PLATO3_SLA_t SLA;

        SLA = SLA_Info.SLA.SLA3;
#else
		PLATO2_SLA_t  SLA;

        SLA = SLA_Info.SLA.SLA2;
#endif
        DBA_error_code = SLA_Info.DBA_ErrCode;

        sys_console_printf("\r\nGet SLA Info ok \r\n");
		sys_console_printf(" SLA.class = %d \r\n", SLA.class );
#ifdef PLATO_DBA_V3
		sys_console_printf("SLA.fixed_packet_size = %d\r\n",SLA.fixed_packet_size );
		sys_console_printf("SLA.fixed_bw = %d\r\n",SLA.fixed_bw );
		sys_console_printf("SLA.fixed_bw_fine = %d\r\n",SLA.fixed_bw_fine );
#else
		sys_console_printf(" SLA.delay = %d \r\n", SLA.delay );
#endif			
		sys_console_printf(" SLA.max_gr_bw = %d \r\n", SLA.max_gr_bw );
		sys_console_printf(" SLA.max_gr_bw_fine = %d \r\n", SLA.max_gr_bw_fine );
		sys_console_printf(" SLA.max_be_bw = %d \r\n", SLA.max_be_bw );
		sys_console_printf(" SLA.max_be_bw_fine = %d \r\n", SLA.max_be_bw_fine );
		sys_console_printf(" error_code %d \r\n", DBA_error_code );	
	}
	return;
}

int Pon_SetLLIDSLA(short int olt_id, short int llid, unsigned short gr_fine, unsigned short be_fine, short int *dba_error)
{
    int iRlt = RERROR;
	PLATO3_SLA_t BW3;
	PLATO4_SLA_t BW4;
	short int PonChipType = RERROR;
	
	VOS_MemZero(&BW3, sizeof(BW3));
	VOS_MemZero(&BW4, sizeof(BW4));

    PonChipType = GetPonChipTypeByPonPort(olt_id);
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(OLT_PONCHIP_ISPAS10G(PonChipType))
	{
		BW4.max_gr_bw_fine = gr_fine;
	    BW4.max_be_bw_fine = be_fine;
		iRlt = OLTAdv_PLATO4_SetLLIDSLA(olt_id, llid, &BW4, dba_error);
	}
	else
#endif
    if(OLT_PONCHIP_ISPAS1G(PonChipType))
	{
		if(GW10G_PAS_BROADCAST_LLID == llid)
		{
		    llid = GW1G_PAS_BROADCAST_LLID;
		}
		BW3.max_gr_bw_fine = gr_fine;
		BW3.max_be_bw_fine = be_fine;
		iRlt = OLTAdv_PLATO3_SetLLIDSLA(olt_id, llid, &BW3, dba_error);
	}
	return iRlt;
}

int CTC_GetExtendedOamDiscoveryTiming(unsigned short int  *discovery_timeout)
{
    int iRlt;
    
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
	    iRlt = GW10G_CTC_STACK_get_extended_oam_discovery_timing(discovery_timeout);
	}
	else 
#endif
	{
	    iRlt = CTC_STACK_get_extended_oam_discovery_timing(discovery_timeout);
	}

	return iRlt;
}

int CTC_SetExtendedOamDiscoveryTiming(unsigned short int discovery_timeout)
{
    int iRlt;
    
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
	    iRlt = GW10G_CTC_STACK_set_extended_oam_discovery_timing(discovery_timeout);
	}
	else 
#endif
	{
	    iRlt = CTC_STACK_set_extended_oam_discovery_timing(discovery_timeout);
	}

	return iRlt;
}

int CTC_GetEncryptionTiming(unsigned char *update_key_time, unsigned short int  *no_reply_timeout)
{
    int iRlt;
    
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
	    iRlt = GW10G_CTC_STACK_get_encryption_timing(update_key_time, no_reply_timeout);
	}
	else 
#endif
	{
	    iRlt = CTC_STACK_get_encryption_timing(update_key_time, no_reply_timeout);
	}

	return iRlt;
}
int CTC_SetEncryptionTiming(unsigned char update_key_time, unsigned short int  no_reply_timeout)
{
    int iRlt;
    
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
	    iRlt = GW10G_CTC_STACK_set_encryption_timing(update_key_time, no_reply_timeout);
	}
	else 
#endif
	{
	    iRlt = CTC_STACK_set_encryption_timing(update_key_time, no_reply_timeout);
	}

	return iRlt;
}

int CTC_GetEncryptionTimingThreshold(unsigned char *start_encryption_threshold)
{
    int iRlt;
    
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
	    iRlt = GW10G_CTC_STACK_get_encryption_timing_threshold(start_encryption_threshold);
	}
	else 
#endif
	{
	    iRlt = CTC_STACK_get_encryption_timing_threshold(start_encryption_threshold);
	}

	return iRlt;
}

int CTC_SetEncryptionTimingThreshold(unsigned char start_encryption_threshold)
{
    int iRlt;
    
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
	    iRlt = GW10G_CTC_STACK_set_encryption_timing_threshold(start_encryption_threshold);
	}
	else 
#endif
	{
	    iRlt = CTC_STACK_set_encryption_timing_threshold(start_encryption_threshold);
	}

	return iRlt;
}
/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */

#if 0
int  SetPonFetchDHCPMsgToHostPath( short int PonPortIdx )
{

	short int PonChipType;
	short int PonChipVer = RERROR;
	short int ret;

	CHECK_PON_RANGE

	PonChipType = V2R1_GetPonchipType(PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
	
	if( PonChipType == PONCHIP_PAS )
		{
		/*
		PonChipVer = V2R1_GetPonChipVersion( PonPortIdx );
		if(( PonChipVer != PONCHIP_PAS5001 ) && ( PonChipVer != PONCHIP_PAS5201 )) return( RERROR );
		*/
		if( PonChipVer == PONCHIP_PAS5001 ) return( ROK );
		
		ret = PAS_set_classification_rule ( PonPortIdx , PON_DIRECTION_UPLINK, PON_OLT_CLASSIFICATION_DHCP,
											NULL, PON_OLT_CLASSIFIER_DESTINATION_HOST );
            
		if( ret != PAS_EXIT_OK )
			{
			sys_console_printf(" PAS_set_classification_rule for DHCP Err %d\r\n", ret);
			return( RERROR );
			}
		
		else 
			{
			/*sys_console_printf(" PAS_set_classification_rule for DHCP success.(PONID = %d)\r\n", PonPortIdx);*/
			return( ROK );
			}
		}

	else{ /* other pon chip handler */

		}


	return( RERROR );
	
}

int DisablePonFetchDHCPMsg( short int PonPortIdx )
{

	short int ret;

	CHECK_PON_RANGE

	ret = PAS_set_classification_rule ( PonPortIdx , PON_DIRECTION_NO_DIRECTION, PON_OLT_CLASSIFICATION_DHCP,
											NULL, PON_OLT_CLASSIFIER_DESTINATION_NONE );
            
	if( ret != PAS_EXIT_OK )
		{
		sys_console_printf(" PAS_set_classification_rule DHCP-undo Err %d\r\n", ret);
		return( RERROR );
		}
	else return( ROK );
	
}
#endif


int Pon_ResumeHostIF(short int olt_id)
{
    int result = 0;
    int i;
    int iPonChipType;
    unsigned char *apszCliStr[] = {"mfi", "recover"};

    iPonChipType = OLTAdv_GetChipTypeID(olt_id);
    if ( OLT_PONCHIP_ISTK(iPonChipType) )
    {
        for( i = 0; i < ARRAY_SIZE(apszCliStr); i++ )
        {
            if ( 0 != (result = OLT_SendChipCli(olt_id, VOS_StrLen(apszCliStr[i]) + 1, apszCliStr[i])) )    
            {
                sys_console_printf("\r\nPon_ResumeHostIF(%d) failed(%d) at cli[%s]\r\n", olt_id, result, apszCliStr[i] );
                break;
            }
        }
    }

    return result;
}



#ifdef __cplusplus

}
#endif

/*
REMOTE_APPLICATIONS_set_oam_frame_size_valid
PAS_updated_parameters
PAS_set_test_mode
REMOTE_APPLICATIONS_send_receive_message
REMOTE_APPLICATIONS_get_oam_frame_size_valid
REMOTE_APPLICATIONS_init
REMOTE_APPLICATIONS_terminate
REMOTE_APPLICATIONS_set_oam_frame_size
REMOTE_APPLICATIONS_reset_onu_database

*/

