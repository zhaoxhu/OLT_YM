/***************************************************************
*
*						Module Name:  OltIfAdapter_Null.c
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
*   Date: 			2010/05/11
*   Author:		liwei056
*   content:
**  History:
**   Date        |    Name       |     Description
**---- ----- |-----------|------------------ 
**  10/05/1    |   liwei056    |     create 
**----------|-----------|------------------
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif


#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "../../onu/OnuPortStatsMgt.h"


/*-----------------------------内部适配----------------------------------------*/

static int NULL_IsExist(short int olt_id, bool *status)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(status);

    /* OLT_NULL_DEBUG(OLT_NULL_TITLE"NULL_IsExist(%d)'s result(0).\r\n", olt_id); */
    *status = 0;
    return OLT_ERR_OK;
}

static int NULL_Error(short int olt_id)
{
    OLT_LOCAL_ASSERT(olt_id);
    
    OLT_NULL_DEBUG(OLT_NULL_TITLE"NULL_Error(%d)'s result(%d).\r\n", olt_id, OLT_ERR_NOTEXIST);

    return OLT_ERR_NOTEXIST;
}

static int NULL_OK(short int olt_id)
{
    OLT_LOCAL_ASSERT(olt_id);
    
    OLT_NULL_DEBUG(OLT_NULL_TITLE"NULL_OK(%d)'s result(0).\r\n", olt_id);

    return OLT_ERR_OK;
}



/*-----------------------------外部接口----------------------------------------*/

/* 顶级空接口(与PON板完全无关) */
static const OltMgmtIFs s_top_nullIfs = {
#if 1
/* -------------------OLT基本API------------------- */
    NULL_IsExist,  /* IsExist */
    GW_GetChipTypeID,
    GW_GetChipTypeName,
    NULL_Error,    /* ResetPon */
    NULL_Error,    /* RemoveOlt */
    
    GW_CopyOlt,
    GW_CmdIsSupported,
    NULL_OK,       /* SetDebugMode */
    NULL_OK,       /* SetInitParams */
    NULL_OK,       /* SetSystemParams */

    NULL_Error,    /* SetPonI2CExtInfo */
    NULL_Error,    /* GetPonI2CExtInfo */
    NULL_Error,    /* SetCardI2CInfo */
    NULL_Error,    /* GetCardI2CInfo */
   /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /*WriteMdioRegister*/

    NULL_Error,    /*ReadMdioRegister*/
    NULL_Error,    /*ReadI2CRegister*/
    NULL_Error,    /*GpioAccess*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /* ReadGpio */
    NULL_Error,    /* WriteGpio */

    NULL_Error,    /* SendChipCli */
    NULL_OK,       /* SetDeviceName*/
    NULL_OK,       /*ResetPonChip*/
#endif
    
#if 1
/* -------------------OLT PON管理API--------------- */
    NULL_Error,    /* GetVersion */
    NULL_Error,    /* GetDBAVersion */
    NULL_Error,    /* ChkVersion */
    NULL_Error,    /* ChkDBAVersion */
    NULL_Error,    /* GetCniLinkStatus */

    GW_GetPonWorkStatus,
    NULL_OK,       /* SetAdminStatus */
    GW_GetAdminStatus,
    NULL_OK,       /* SetVlanTpid */
    NULL_OK,       /* SetVlanQinQ */
    
    NULL_OK,       /* SetPonFrameSizeLimit */
    NULL_Error,    /* GetPonFrameSizeLimit */
    NULL_Error,    /* OamIsLimit */
    NULL_OK,       /* UpdatePonParams */
    NULL_OK,       /* SetPPPoERelayMode */

    NULL_OK,       /* SetPPPoERelayParams */
    NULL_OK,       /* SetDhcpRelayMode */
    NULL_OK,       /* SetIgmpAuthMode */
    NULL_Error,    /* SendFrame2PON */
    NULL_Error,    /* SendFrame2CNI */

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /*GetVidDownlinkMode*/
    NULL_Error,    /*DelVidDownlinkMode*/
    NULL_Error,    /*GetOltParameters*/
    NULL_Error,    /*SetOltIgmpSnoopingMode*/
    NULL_Error,    /*GetOltIgmpSnoopingMode*/

    NULL_Error,    /*SetOltMldForwardingMode*/
    NULL_Error,    /*GetOltMldForwardingMode*/
    NULL_Error,    /*SetDBAReportFormat*/
    NULL_Error,    /*GetDBAReportFormat*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-04-27*/
    NULL_Error,    /*UpdateProvBWInfo*/
    /*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    NULL_IsExist,  /* LLIDIsExist */
    NULL_OK,       /* DeregisterLLID */
    NULL_Error,    /* GetLLIDMac */
    NULL_Error,    /* GetLLIDRegisterInfo */
    NULL_Error,    /* AuthorizeLLID */

    NULL_Error,    /* SetLLIDSLA */
    NULL_Error,    /* GetLLIDSLA */
    NULL_Error,    /* SetLLIDPolice */
    NULL_Error,    /* GetLLIDPolice */
    NULL_Error,    /* SetLLIDdbaType */
    
    NULL_Error,    /* GetLLIDdbaType */
    NULL_Error,    /* SetLLIDdbaFlags */
    NULL_Error,    /* GetLLIDdbaFlags */
    NULL_Error,    /* GetLLIDHeartbeatOam */
    NULL_Error,    /* SetLLIDHeartbeatOam */
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    NULL_Error,    /* GetOnuNum */
    NULL_Error,    /* GetAllOnus */
    NULL_OK,       /* ClearAllOnus */
    NULL_Error,    /* ResumeAllOnuStatus */
    NULL_OK,       /* SetAllOnuAuthMode */
    
    NULL_OK,       /* SetOnuAuthMode */
    NULL_OK,       /* SetMacAuth */
    NULL_OK,       /* SetAllOnuBindMode */
    NULL,          /* CheckOnuRegisterControl */
    NULL_OK,       /* SetAllOnuDefaultBW */
    
    NULL_OK,       /* SetAllOnuDownlinkPoliceMode */
    NULL_OK,       /* SetOnuDownlinkPoliceMode */
    NULL_OK,       /* SetAllOnuDownlinkPoliceParam */
    NULL_OK,       /* SetAllOnuUplinkDBAParam */
    NULL_OK,       /* SetOnuDownlinkPri2CoSQueueMap */

    NULL_Error,    /* ActivePendOnu */    
    NULL_Error,    /* ActiveOnePendingOnu */
    NULL_Error,    /* ActiveConfPendingOnu */
    NULL_Error,    /* ActiveOneConfPendingOnu */
    NULL_Error,    /* GetPendingOnu */

    NULL_Error,    /* GetUpdatingOnu */
    NULL_Error,    /* GetUpdatedOnu */
    NULL_Error,    /* GetOnuUpdatingStatusLocal */
    NULL_Error,    /* SetOnuUpdateMsg */
    NULL_Error,    /* GetOnuUpdateWaiting */

    NULL_OK,       /* SetAllOnuAuthMode2 */
    NULL_OK,       /* SetAllOnuBWParams */
    NULL_OK,       /* SetOnuP2PMode */
    NULL_Error,    /* GetOnuB2PMode */
    NULL_OK,       /* SetOnuB2PMode */

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /* GetOnuMode */
    NULL_Error,    /* GetMACAddressAuthentication */
    NULL_Error,    /* SetAuthorizeMacAddressAccordingListMode */
    NULL_Error,    /* GetAuthorizeMacAddressAccordingListMode */
    NULL_Error,    /* GetDownlinkBufferConfiguration */

    NULL_Error,    /* GetOamInformation */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
/*Begin:for onu swap by jinhl@2013-02-22*/
    NULL_Error,    /* ResumeLLIDStatus */
    NULL_Error,    /* SearchFreeOnuIdx */
    NULL_Error,    /* GetActOnuIdxByMac */
/*End:for onu swap by jinhl@2013-02-22*/
    NULL_OK,       /* BroadCastCliCommand */

    NULL_OK,       /*SetAuthEntry*/    
    NULL_OK,       /*SetOnuDefaultMaxMac*/  /*modi by luh@2015-04-22; Q25076*/          
    NULL_OK,      /*GW_SetCTCOnuPortStatsTimeOut*/
    NULL_OK,       /* SetMaxMac */
    NULL_OK,       /* GetOnuConfDelStatus*/
    NULL_OK,       /* SetCTCOnuTxPowerSupplyControl*/
    NULL_OK,       /* SetCTCOnuTxPowerSupplyControl*/
#endif
    
#if 1
/* -------------------OLT 加密管理API----------- */
    NULL_OK,       /* SetEncryptMode */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_OK,       /*SetEncryptionPreambleMode*/
    NULL_OK,       /*GetEncryptionPreambleMode*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /* GetLLIDEncryptMode */
    NULL_Error,    /* StartLLIDEncrypt */

    NULL_Error,    /* FinishLLIDEncrypt */
    NULL_Error,    /* StopLLIDEncrypt */
    NULL_Error,    /* SetLLIDEncryptKey */
    NULL_Error,    /* FinishLLIDEncryptKey */
#endif
       
#if 1
/* -------------------OLT 地址表管理API-------- */
    NULL_OK,       /* SetMacAgingTime */
    NULL_OK,       /* SetAddrTblCfg */
    NULL_Error,    /* GetAddrTblCfg */
    NULL_Error,    /* GetMacAddrTbl */
    NULL_OK,       /* AddMacAddrTbl */
    
    NULL_OK,       /* DelMacAddrTbl */
    NULL_Error,    /* RemoveMac */
    NULL_Error,    /* ResetAddrTbl */
    NULL_OK,       /* SetOnuMacThreshold */
    NULL_OK,       /* SetOnuMacCheckEnable */
    
    NULL_OK,       /* SetOnuMacCheckPeriod */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /*SetAddressTableFullHandlingMode*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,
    NULL_Error,/* GetMacAddrVlanTbl */
#endif
    
#if 1
/* -------------------OLT 光路管理API----------- */
    NULL_Error,    /* GetOpticalCapability */
    NULL_Error,    /* GetOpticsDetail */
    NULL_OK,       /* SetPonRange */
    NULL_OK,       /* SetOpticalTx */
    NULL_Error,    /* GetOpticalTx */
    
    NULL_Error,    /* SetVirtualScopeAdcConfig */
    NULL_Error,    /* GetVirtualScopeMeasurement */
    NULL_Error,    /* GetVirtualScopeRssiMeasurement */
    NULL_Error,    /* GetVirtualScopeOnuVoltage */
    NULL_Error,    /* SetVirtualLLID */
    
    NULL_OK,       /* SetOpticalTxMode2 */
#endif
    
#if 1
/* -------------------OLT 监控统计管理API---- */
    NULL_Error,    /* GetRawStatistics */
    NULL_Error,    /* ResetCounters */
    NULL_OK,       /* SetBerAlarm */
    NULL_OK,       /* SetFerAlarm */
    NULL_OK,       /* SetPonBerAlarm */
    
    NULL_OK,       /* SetPonFerAlarm */
    NULL_OK,       /* SetBerAlarmParams */
    NULL_OK,       /* SetFerAlarmParams */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /*SetAlarmConfig*/
    NULL_Error,    /*GetAlarmConfig*/
    
    NULL_Error,    /*GetStatistics*/
    NULL_Error,    /*OltSelfTest*/
    NULL_Error,    /*LinkTest*/
    NULL_Error,    /* SetLLIDFecMode */
    NULL_Error,    /* GetLLIDFecMode */

    NULL_Error,    /*SysDump*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif
    
#if 1
/* -------------------OLT 倒换API---------------- */
    NULL_Error,    /* GetHotSwapCapability */
    GW_GetHotSwapMode,
    NULL_OK,       /* SetHotSwapMode */
    NULL_Error,    /* ForceHotSwap */
    NULL_OK,       /* SetHotSwapParam */
    
    NULL_Error,    /* RdnOnuRegister */
    NULL_Error,    /* SetRdnConfig */
    NULL_Error,    /* RdnSwitchOver */
    NULL_Error,    /* RdnIsExist */
    NULL_Error,    /* ResetRdnOltRecord */
    
    NULL_Error,    /* GetRdnState */
    NULL_Error,    /* SetRdnState */
    NULL_Error,    /* GetRdnAddrTbl */
    NULL_Error,    /* RemoveRdnOlt */
    NULL_Error,    /* GetLLIDRdnDB */

    NULL_Error,    /* SetLLIDRdnDB */
    NULL_OK,       /* RdnRemoveOlt */
    NULL_Error,    /* RdnSwapOlt */
    NULL_Error,    /* AddSwapOlt */
    NULL_OK,       /* RdnLooseOlt */
    
    /*Begin:for onu swap by jinhl@2013-02-22*/
    NULL_Error,     /*RdnLLIDAdd*/
    NULL_Error,     /*GetRdnLLIDMode*/
    NULL_Error,     /*SetRdnLLIDMode*/
    NULL_Error,     /*SetRdnLLIDStdbyToAct*/
    NULL_Error,     /*SetRdnLLIDRtt*/
    /*End:for onu swap by jinhl@2013-02-22*/
    
    NULL_Error,    /* RdnIsReady */
    NULL_Error,    /* GetLLIDRdnRegisterInfo */
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */
    NULL_Error,    /* DumpAllCmc */
    NULL_OK,       /* SetCmcServiceVID */
#endif

    NULL_Error     /* LastFun */
};


/* PON板空接口也要挂上PAS-SOFT的全局实现 */
static int NULL_SetInitParams(short int olt_id, short int host_olt_manage_type, unsigned short host_olt_manage_address)
{
    int iRlt;

    if ( SYS_LOCAL_MODULE_TYPE_IS_PAS_PONCARD_MANAGER )
    {
        extern int PAS_SetInitParams(short int olt_id, short int host_olt_manage_type, unsigned short host_olt_manage_address);

        iRlt = PAS_SetInitParams(olt_id, host_olt_manage_type,host_olt_manage_address);
    }
    else if ( SYS_LOCAL_MODULE_TYPE_IS_TK_PONCARD_MANAGER )
    {
        if ( SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER )
        {
            iRlt = 0;
        }
        else
        {
            extern int TK_SetInitParams(short int olt_id, short int host_olt_manage_type, unsigned short host_olt_manage_address);

            iRlt = TK_SetInitParams(olt_id, host_olt_manage_type,host_olt_manage_address);
        }
    }
    else
    {
        iRlt = 0;
    }

    OLT_NULL_DEBUG(OLT_NULL_TITLE"NULL_SetInitParams(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, host_olt_manage_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int NULL_SetSystemParams(short int olt_id, short int statistics_sampling_cycle, short int monitoring_cycle, short int host_olt_msg_timeout, short int olt_reset_timeout)
{
    int iRlt;

    if ( SYS_LOCAL_MODULE_TYPE_IS_PAS_PONCARD_MANAGER )
    {
        extern int PAS_SetSystemParams(short int olt_id, short int statistics_sampling_cycle, short int monitoring_cycle, short int host_olt_msg_timeout, short int olt_reset_timeout);

        iRlt = PAS_SetSystemParams(olt_id, statistics_sampling_cycle, monitoring_cycle, host_olt_msg_timeout, olt_reset_timeout);
    }
    else
    {
        iRlt = 0;
    }

    OLT_NULL_DEBUG(OLT_NULL_TITLE"NULL_SetSystemParams(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, statistics_sampling_cycle, monitoring_cycle, host_olt_msg_timeout, olt_reset_timeout, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* pon板级空接口 (与PON芯片无关) */
static const OltMgmtIFs s_card_nullIfs = {
#if 1
/* -------------------OLT基本API------------------- */
    NULL_IsExist,  /* IsExist */
    GW_GetChipTypeID,
    GW_GetChipTypeName,
    GW_ResetPon,   
    NULL_Error,    /* RemoveOlt */

    GW_CopyOlt,
    GW_CmdIsSupported,
    NULL_OK,       /* SetDebugMode */
    NULL_SetInitParams,
    NULL_SetSystemParams,

    GW_SetPonI2CExtInfo,
    GW_GetPonI2CExtInfo,
    GW_SetCardI2CInfo, 
    GW_GetCardI2CInfo,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /*WriteMdioRegister*/

    NULL_Error,    /*ReadMdioRegister*/
    NULL_Error,    /*ReadI2CRegister*/
    NULL_Error,    /*GpioAccess*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /* ReadGpio */
    NULL_Error,    /* WriteGpio */

    NULL_Error,    /* SendChipCli */
    NULL_OK,       /*SetDeviceName*/
    NULL_OK,	   /*ResetPonChip*/
#endif
    
#if 1
/* -------------------OLT PON管理API--------------- */
    NULL_Error,    /* GetVersion */
    NULL_Error,    /* GetDBAVersion */
    NULL_Error,    /* ChkVersion */
    NULL_Error,    /* ChkDBAVersion */
    NULL_Error,    /* GetCniLinkStatus */

    GW_GetPonWorkStatus,
    NULL_OK,       /* SetAdminStatus */
    GW_GetAdminStatus,
    NULL_OK,       /* SetVlanTpid */
    NULL_OK,       /* SetVlanQinQ */
    
    NULL_OK,       /* SetPonFrameSizeLimit */
    NULL_Error,    /* GetPonFrameSizeLimit */
    NULL_Error,    /* OamIsLimit */
    NULL_OK,       /* UpdatePonParams */
    GW_SetPPPoERelayMode,

    GW_SetPPPoERelayParams,
    GW_SetDhcpRelayMode,
    NULL_OK,       /* SetIgmpAuthMode */
    NULL_Error,    /* SendFrame2PON */
    NULL_Error,    /* SendFrame2CNI */

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /*GetVidDownlinkMode*/
    NULL_Error,    /*DelVidDownlinkMode*/
    NULL_Error,    /*GetOltParameters*/
    NULL_Error,    /*SetOltIgmpSnoopingMode*/
    NULL_Error,    /*GetOltIgmpSnoopingMode*/

    NULL_Error,    /*SetOltMldForwardingMode*/
    NULL_Error,    /*GetOltMldForwardingMode*/
    NULL_Error,    /*SetDBAReportFormat*/
    NULL_Error,    /*GetDBAReportFormat*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-04-27*/
    NULL_Error,    /*UpdateProvBWInfo*/
    /*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    NULL_IsExist,  /* LLIDIsExist */
    NULL_OK,       /* DeregisterLLID */
    NULL_Error,    /* GetLLIDMac */
    NULL_Error,    /* GetLLIDRegisterInfo */
    NULL_Error,    /* AuthorizeLLID */

    NULL_Error,    /* SetLLIDSLA */
    NULL_Error,    /* GetLLIDSLA */
    NULL_Error,    /* SetLLIDPolice */
    NULL_Error,    /* GetLLIDPolice */
    NULL_Error,    /* SetLLIDdbaType */
    
    NULL_Error,    /* GetLLIDdbaType */
    NULL_Error,    /* SetLLIDdbaFlags */
    NULL_Error,    /* GetLLIDdbaFlags */
    NULL_Error,    /* GetLLIDHeartbeatOam */
    NULL_Error,    /* SetLLIDHeartbeatOam */
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    NULL_Error,    /* GetOnuNum */
    NULL_Error,    /* GetAllOnus */
    NULL_OK,       /* ClearAllOnus */
    NULL_Error,    /* ResumeAllOnuStatus */
    NULL_OK,       /* SetAllOnuAuthMode */
    
    NULL_OK,       /* SetOnuAuthMode */
    NULL_OK,       /* SetMacAuth */
    NULL_OK,       /* SetAllOnuBindMode */
    NULL,          /* CheckOnuRegisterControl */
    NULL_OK,       /* SetAllOnuDefaultBW */
    
    NULL_OK,       /* SetAllOnuDownlinkPoliceMode */
    NULL_OK,       /* SetOnuDownlinkPoliceMode */
    NULL_OK,       /* SetAllOnuDownlinkPoliceParam */
    NULL_OK,       /* SetAllOnuUplinkDBAParam */
    NULL_OK,       /* SetOnuDownlinkPri2CoSQueueMap */

    NULL_Error,    /* ActivePendOnu */    
    NULL_Error,    /* ActiveOnePendingOnu */
    NULL_Error,    /* ActiveConfPendingOnu */
    NULL_Error,    /* ActiveOneConfPendingOnu */
    GW_GetPendingOnu,

    GW_GetUpdatingOnu,   
    GW_GetUpdatedOnu,
    GW_GetOnuUpdatingStatusLocal,
    GW_SetOnuUpdateMsg,
    GW_GetOnuUpdateWaiting,

    NULL_OK,       /* SetAllOnuAuthMode2 */
    NULL_OK,       /* SetAllOnuBWParams */
    NULL_OK,       /* SetOnuP2PMode */
    NULL_Error,    /* GetOnuB2PMode */
    NULL_OK,       /* SetOnuB2PMode */
    
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /* GetOnuMode */
    NULL_Error,    /* GetMACAddressAuthentication */
    NULL_Error,    /* SetAuthorizeMacAddressAccordingListMode */
    NULL_Error,    /* GetAuthorizeMacAddressAccordingListMode */
    NULL_Error,    /* GetDownlinkBufferConfiguration */

    NULL_Error,    /* GetOamInformation */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
/*Begin:for onu swap by jinhl@2013-02-22*/
    NULL_Error,    /* ResumeLLIDStatus */
    NULL_Error,    /* SearchFreeOnuIdx */
    NULL_Error,    /* GetActOnuIdxByMac */
/*End:for onu swap by jinhl@2013-02-22*/
    NULL_Error,       /* BroadCastCliCommand */

    NULL_Error,       /*SetAuthEntry*/    
    NULL_OK,       /*SetOnuDefaultMaxMac*/   /*modi by luh@2015-04-22; Q25076*/          
    NULL_OK,       /*SetOnuDefaultMaxMac*/   /*modi by luh@2015-04-22; Q25076*/          

    NULL_OK,      /*GW_SetCTCOnuPortStatsTimeOut*/
    NULL_OK,       /* SetMaxOnu */    
    NULL_OK,       /* GetOnuConfDelStatus*/    
    NULL_OK,	   /* SetCTCOnuTxPowerSupplyControl*/
#endif
    
#if 1
/* -------------------OLT 加密管理API----------- */
    NULL_OK,       /* SetEncryptMode */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_OK,       /*SetEncryptionPreambleMode*/
    NULL_OK,       /*GetEncryptionPreambleMode*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /* GetLLIDEncryptMode */
    NULL_Error,    /* StartLLIDEncrypt */
    
    NULL_Error,    /* FinishLLIDEncrypt */
    NULL_Error,    /* StopLLIDEncrypt */
    NULL_Error,    /* SetLLIDEncryptKey */
    NULL_Error,    /* FinishLLIDEncryptKey */
#endif
       
#if 1
/* -------------------OLT 地址表管理API-------- */
    NULL_OK,       /* SetMacAgingTime */
    NULL_OK,       /* SetAddrTblCfg */
    NULL_Error,    /* GetAddrTblCfg */
    NULL_Error,    /* GetMacAddrTbl */
    NULL_OK,       /* AddMacAddrTbl */
    
    NULL_OK,       /* DelMacAddrTbl */
    NULL_Error,    /* RemoveMac */
    NULL_Error,    /* ResetAddrTbl */
    NULL_OK,       /* SetOnuMacThreshold */
    GW_OnuMacCheckEnable,  

    NULL_OK,       /* SetOnuMacCheckPeriod */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /*SetAddressTableFullHandlingMode*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,
    NULL_Error,
#endif
    
#if 1
/* -------------------OLT 光路管理API----------- */
    NULL_Error,    /* GetOpticalCapability */
    NULL_Error,    /* GetOpticsDetail */
    NULL_OK,       /* SetPonRange */
    NULL_OK,       /* SetOpticalTx */
    NULL_Error,    /* GetOpticalTx */

    NULL_Error,    /* SetVirtualScopeAdcConfig */
    NULL_Error,    /* GetVirtualScopeMeasurement */
    NULL_Error,    /* GetVirtualScopeRssiMeasurement */
    NULL_Error,    /* GetVirtualScopeOnuVoltage */
    NULL_Error,    /* SetVirtualLLID */
    
    NULL_OK,       /* SetOpticalTxMode2 */
#endif
    
#if 1
/* -------------------OLT 监控统计管理API---- */
    NULL_Error,    /* GetRawStatistics */
    NULL_Error,    /* ResetCounters */
    NULL_OK,       /* SetBerAlarm */
    NULL_OK,       /* SetFerAlarm */
    NULL_OK,       /* SetPonBerAlarm */
    
    NULL_OK,       /* SetPonFerAlarm */
    NULL_OK,       /* SetBerAlarmParams */
    NULL_OK,       /* SetFerAlarmParams */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL_Error,    /*SetAlarmConfig*/
    NULL_Error,    /*GetAlarmConfig*/
    
    NULL_Error,    /*GetStatistics*/
    NULL_Error,    /*OltSelfTest*/
    NULL_Error,    /*LinkTest*/
    NULL_Error,    /* SetLLIDFecMode */
    NULL_Error,    /* GetLLIDFecMode */

    NULL_Error,    /*SysDump*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif
    
#if 1
/* -------------------OLT 倒换API---------------- */
    NULL_Error,    /* GetHotSwapCapability */
    GW_GetHotSwapMode,
    NULL_OK,       /* SetHotSwapMode */
    NULL_Error,    /* ForceHotSwap */
    GW_SetHotSwapParam,
    
    NULL_Error,    /* RdnOnuRegister */
    NULL_Error,    /* SetRdnConfig */
    NULL_Error,    /* RdnSwitchOver */
    NULL_Error,    /* RdnIsExist */
    NULL_Error,    /* ResetRdnOltRecord */
    
    NULL_Error,    /* GetRdnState */
    NULL_Error,    /* SetRdnState */
    NULL_Error,    /* GetRdnAddrTbl */
    NULL_Error,    /* RemoveRdnOlt */
    NULL_Error,    /* GetLLIDRdnDB */
    
    NULL_Error,    /* SetLLIDRdnDB */
    NULL_OK,       /* RdnRemoveOlt */
    NULL_Error,    /* RdnSwapOlt */
    NULL_Error,    /* AddSwapOlt */
    NULL_OK,       /* RdnLooseOlt */
    
    /*Begin:for onu swap by jinhl@2013-02-22*/
    NULL_Error,    /*RdnLLIDAdd*/
    NULL_Error,    /*GetRdnLLIDMode*/
    NULL_Error,    /*SetRdnLLIDMode*/
    NULL_Error,    /*SetRdnLLIDStdbyToAct*/
    NULL_Error,    /*SetRdnLLIDRtt*/
    /*End:for onu swap by jinhl@2013-02-22*/
    
    NULL_Error,    /* RdnIsReady */
    NULL_Error,    /* GetLLIDRdnRegisterInfo */
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */
    NULL_Error,    /* DumpAllCmc */
    NULL_OK,       /* SetCmcServiceVID */
#endif

    NULL_Error     /* LastFun */
};


void OLT_NULL_Support()
{
    if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
    {
        OLT_RegisterAdapter(OLT_ADAPTER_NULL, &s_card_nullIfs);
    }
    else
    {
        OLT_RegisterAdapter(OLT_ADAPTER_NULL, &s_top_nullIfs);
    }
}


#ifdef __cplusplus

}

#endif


