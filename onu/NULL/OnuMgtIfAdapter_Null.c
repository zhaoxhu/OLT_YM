/***************************************************************
*
*						Module Name:  OnuMgtIfAdapter_Null.c
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
*   Author:		shixh
*   content:
**  History:
**   Date        |    Name       |     Description
**---- ----- |-----------|------------------ 
**  10/05/1    |   shixh    |     create 
**----------|-----------|------------------
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif


#include  "OnuGeneral.h"
#include  "OltGeneral.h"


/********************************************内部适配*******************************************/

static int NULL_ONU_OK( short int olt_id, short int onu_id )
{
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    OLT_NULL_DEBUG(OLT_NULL_TITLE"NULL_OK(%d, %d)'s result(0).\r\n", olt_id, onu_id);

    return OLT_ERR_OK;
}

static int NULL_ONU_error( short int olt_id, short int onu_id )
{
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    OLT_NULL_DEBUG(OLT_NULL_TITLE"NULL_Error(%d, %d)'s result(%d).\r\n", olt_id, onu_id, OLT_ERR_NOTEXIST);

    return OLT_ERR_NOTEXIST;
}

static int NULL_OnuIsOnline(short int olt_id, int onu_id, int *status)
{
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(status);

    *status = 0;      

    return 0;
}

static int NULL_DelOnuByManual(short int olt_id, short int onu_id)
{
    int iRlt = 0;
    int slot;
    int port;
    int is_valid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    /* modified by xieshl 20110509, 不存在也要返回成功，否则数据不一致时无法删除主控数据 */
	(void)activeOneLocalPendingOnu(olt_id, onu_id);
#if 0	
    if ( (0 == GWONU_OnuIsValid(olt_id, onu_id, &is_valid))
        && (0 < is_valid) )
    {
    }
    else
    {
        /*iRlt = OLT_ERR_NOTEXIST;*/	
    }
#endif
	
    OLT_NULL_DEBUG(OLT_NULL_TITLE"NULL_DelOnuByManual(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* B--added by liwei056@2012-8-22 for PendingOnu应该挂接空接口 */
static int NULL_SetOnuTrafficServiceMode(short int olt_id, short int onu_id, int service_mode)
{
    int iRlt = 0;
    int slot;
    int port;
    int is_valid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( V2R1_ENABLE == service_mode )
    {
        (void)ActivateOnu(olt_id, onu_id);
    }
	
    OLT_NULL_DEBUG(OLT_NULL_TITLE"NULL_SetOnuTrafficServiceMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, service_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
/* E--added by liwei056@2012-8-22 for PendingOnu应该挂接空接口 */

static int NULL_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(chip_type);
    VOS_ASSERT(remote_type);
    
    *chip_type   = PONCHIP_UNKNOWN;
    *remote_type = ONU_MANAGE_UNKNOWN;
    
    return OLT_ERR_OK;
}

/*******************************************外部接口*********************************************/
static const OnuMgmtIFs sonu_nullIfs = {
#if 1
/* -------------------ONU基本API------------------- */
    GWONU_OnuIsValid,  /* OnuIsValid */
    NULL_OnuIsOnline,  /* OnuIsOnline */
    NULL_ONU_OK,       /* AddOnuByManual */
    NULL_ONU_OK,      /*ModifyOnuByManual*/    
	NULL_DelOnuByManual,  /* DelOnuByManual */
	NULL_ONU_OK,
	GWONU_CmdIsSupported, /* CmdIsSupported */

	GWONU_CopyOnu,     /* CopyOnu */
	NULL_GetIFType,    /* GetIFType */
	GWONU_SetIFType,   /* SetIFType */
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
	NULL_ONU_OK,    /* DeregisterOnu */
	NULL_ONU_OK,    /* SetMacAuthMode */
	NULL_ONU_OK,    /* DelBindingOnu */
#if 0
    NULL_ONU_OK,    /* AddPendingOnu */
    NULL_ONU_error, /* DelPendingOnu */
    NULL_ONU_error, /* DelConfPendingOnu */
#endif
    NULL_ONU_error, /* AuthorizeOnu */
    NULL_ONU_error,/*AuthRequest;   */
    NULL_ONU_error,/*AuthSucess;  */
    NULL_ONU_error,/*AuthFail;*/        
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    NULL_SetOnuTrafficServiceMode,
    NULL_ONU_OK,    /* SetOnuPeerToPeer */
    NULL_ONU_OK,    /* SetOnuPeerToPeerForward */
    NULL_ONU_OK,    /* SetOnuBW */   
    NULL_ONU_error, /* GetOnuSLA */   

   	NULL_ONU_OK,    /* SetOnuFecMode */
	NULL_ONU_error, /* GetOnuVlanMode */
	/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	NULL_ONU_error, /* SetUniPort */
	NULL_ONU_error, /* SetSlowProtocolLimit */
	NULL_ONU_error, /* GetSlowProtocolLimit */
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	NULL_ONU_error, /* GetBWInfo */
    NULL_ONU_error, /* GetOnuB2PMode */   
   	NULL_ONU_OK,    /* SetOnuB2PMode */
#endif

#if 1
/* -------------------ONU 监控统计管理API--------------- */
    NULL_ONU_error, /* ResetCounters */
    NULL_ONU_OK,     /* SetPonLoopback */
#endif

#if 1
/* -------------------ONU加密管理API-------------------- */
	NULL_ONU_OK,    /* GetLLIDParams */	/* 问题单12334 */
	NULL_ONU_OK,    /* StartEncryption */
	NULL_ONU_OK,    /* StopEncryption */
    NULL_ONU_OK,    /* SetOnuEncryptParams */   
    GWONU_GetOnuEncryptParams,   
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	NULL_ONU_error, 
	/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
	NULL_ONU_error, /* GetOnuMacAddrTbl */
	NULL_ONU_error, /* GetOltMacAddrTbl */
	NULL_ONU_error, /*GetOltMacAddrVlanTbl*/
	NULL_ONU_OK,    /* SetOnuMaxMac */
	NULL_ONU_error, /* GetOnuUniMacCfg */
	NULL_ONU_error, /* GetOnuMacCheckFlag */
	NULL_ONU_error, /* GetOnuAllPortMacCounter */
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    NULL_ONU_error, /* GetOnuDistance */
    NULL_ONU_error, /* GetOpticalCapability */
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
	NULL_ONU_OK,    /* SetOnuLLID */
#endif

#if 1
/* -------------------ONU 设备管理API------------------- */
	NULL_ONU_error, /* GetOnuVer */	
	NULL_ONU_error, /* GetOnuPonVer */
	NULL_ONU_error, /* GetOnuRegisterInfo */	
    NULL_ONU_error, /* GetOnuI2CInfo */
    NULL_ONU_error, /* SetOnuI2CInfo */
	
    NULL_ONU_error, /* ResetOnu */
    NULL_ONU_OK,    /* SetOnuSWUpdateMode */
    NULL_ONU_error, /* OnuSwUpdate */
    NULL_ONU_error, /* OnuGwCtcSwConvert */
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	NULL_ONU_error, /*GetBurnImageComplete*/
	/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    
    NULL_ONU_OK,    /* SetOnuDeviceName */
    NULL_ONU_OK,    /* SetOnuDeviceDesc */
    NULL_ONU_OK,    /* SetOnuDeviceLocation */
    NULL_ONU_error,    /* GetAllStaticData */
#endif

#if 1
/* -------------------ONU CTC-PROTOCOL API---------- */
    NULL_ONU_error, /* GetCtcVersion */
    NULL_ONU_error, /* GetFirmwareVersion */
    NULL_ONU_error, /* GetSerialNumber */
    NULL_ONU_error, /* GetChipsetID */

    NULL_ONU_error, /* GetOnuCap1 */
    NULL_ONU_error, /* GetOnuCap2 */
    NULL_ONU_error, /* GetOnuCap3 */
    
    NULL_ONU_error, /* UpdateOnuFirmware */
    NULL_ONU_error, /* ActiveOnuFirmware */
    NULL_ONU_error, /* CommitOnuFirmware */
    
    NULL_ONU_OK,    /* StartEncrypt */
    NULL_ONU_OK,    /* StopEncrypt */
    
    NULL_ONU_error, /* GetEthPortLinkState */
    NULL_ONU_error, /* GetEthPortAdminStatus */
    NULL_ONU_OK,    /* SetEthPortAdminStatus */
    
    NULL_ONU_error, /* GetEthPortPause */
    NULL_ONU_OK,    /* SetEthPortPause */
    
    NULL_ONU_error, /* GetEthPortAutoNegotiation */
    NULL_ONU_OK,    /* SetEthPortAutoNegotiation */
    NULL_ONU_OK,    /* SetEthPortAutoNegotiationRestart */
    NULL_ONU_error, /* GetEthPortAnLocalTecAbility */
    NULL_ONU_error, /* GetEthPortAnAdvertisedTecAbility */
    
    NULL_ONU_error, /* GetEthPortPolicing */
    NULL_ONU_OK,    /* SetEthPortPolicing */
    
    NULL_ONU_error, /* GetEthPortDsPolicing */
    NULL_ONU_OK,    /* SetEthPortDsPolicing */

    NULL_ONU_error, /* GetEthPortVlanConfig */
    NULL_ONU_OK,    /* SetEthPortVlanConfig */
    NULL_ONU_error, /* GetAllPortVlanConfig */
    
    NULL_ONU_error, /* GetEthPortClassificationAndMark */
    NULL_ONU_OK,    /* SetEthPortClassificationAndMark */
    NULL_ONU_OK,    /* ClearEthPortClassificationAndMarking */
    
    NULL_ONU_error, /* GetEthPortMulticatVlan */
    NULL_ONU_OK,    /* SetEthPortMulticatVlan */
    NULL_ONU_OK,    /* ClearEthPortMulticastVlan */

    NULL_ONU_error, /* GetEthPortMulticastGroupMaxNumber */
    NULL_ONU_OK,    /* SetEthPortMulticastGroupMaxNumber */

    NULL_ONU_error, /* GetEthPortMulticastTagStrip */
    NULL_ONU_OK,    /* SetEthPortMulticastTagStrip */
    NULL_ONU_OK,    /* GetAllPortMulticastTagStrip */
    
    NULL_ONU_error, /* GetEthPortMulticastTagOper */
    NULL_ONU_OK,    /* SetEthPortMulticastTagOper */
    NULL_ONU_OK,    /* SetObjMulticastTagOper */
    
    NULL_ONU_error, /* GetMulticatControl */
    NULL_ONU_OK,    /* SetMulticatControl */
    NULL_ONU_OK,    /* ClearMulticastControl */

    NULL_ONU_error, /* GetMulticatSwitch */
    NULL_ONU_OK,    /* SetMulticatSwitch */
    
    NULL_ONU_error, /* GetMulticastFastLeaveAbility */
    NULL_ONU_error, /* GetMulticastFastLeave */
    NULL_ONU_OK,    /* SetMulticastFastLeave */
    
    NULL_ONU_error, /* GetOnuPortStatisticData */
    NULL_ONU_error, /* GetOnuPortStatisticState */
    NULL_ONU_OK,    /* SetOnuPortStatisticState */
    
    NULL_ONU_OK,    /* SetObjAlarmAdminState */
    NULL_ONU_OK,    /* SetObjAlarmThreshold */
    NULL_ONU_error, /* GetDbaReportThreshold */
    NULL_ONU_OK,    /* SetDbaReportThreshold */
    
    NULL_ONU_error, /* GetMngGlobalConfig */
    NULL_ONU_OK,    /* SetMngGlobalConfig */
    NULL_ONU_error, /* GetMngSnmpConfig */
    NULL_ONU_OK,    /* SetMngSnmpConfig */
    
    NULL_ONU_error, /* GetHoldOver */
    NULL_ONU_OK,    /* SetHoldOver */
    NULL_ONU_error, /* GetOptTransDiag */
    NULL_ONU_OK,    /* SetTxPowerSupplyControl */
    
    NULL_ONU_error, /* GetFecAbility */

    NULL_ONU_error, /* GetIADInfo */
    NULL_ONU_error, /* GetVoipIadOperation */
    NULL_ONU_OK,    /* SetVoipIadOperation */
    NULL_ONU_error, /* GetVoipGlobalConfig */
    NULL_ONU_OK,    /* SetVoipGlobalConfig */
    NULL_ONU_error, /* GetVoipFaxConfig */
    NULL_ONU_OK,    /* SetVoipFaxConfig */

    NULL_ONU_error, /* GetVoipPotsStatus */
    NULL_ONU_error, /* GetVoipPort */
    NULL_ONU_OK,    /* SetVoipPort */
    NULL_ONU_error, /* GetVoipPort2 */
    NULL_ONU_OK,    /* SetVoipPort2 */

    NULL_ONU_error, /* GetH248Config */
    NULL_ONU_OK,    /* SetH248Config */
    NULL_ONU_error, /* GetH248UserTidConfig */
    NULL_ONU_OK,    /* SetH248UserTidConfig */
    NULL_ONU_error, /* GetH248RtpTidConfig */
    NULL_ONU_OK,    /* SetH248RtpTidConfig */

    NULL_ONU_error, /* GetSipConfig */
    NULL_ONU_OK,    /* SetSipConfig */
    NULL_ONU_OK,    /* SetSipDigitMap */
    NULL_ONU_error, /* GetSipUserConfig */
    NULL_ONU_OK,    /* SetSipUserConfig */
    NULL_ONU_OK,    /*SetCTCOnuPortStats*/
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */
    NULL_ONU_OK,    /* CliCall */

    NULL_ONU_error, /* SetMgtReset */
    NULL_ONU_error, /* SetMgtConfig */
    NULL_ONU_error, /* SetMgtLaser */
    NULL_ONU_error, /* SetTemperature */
    NULL_ONU_error, /* SetPasFlush */

    NULL_ONU_OK,    /* SetAtuAgingTime */
    NULL_ONU_OK,    /* SetAtuLimit */

    NULL_ONU_OK,    /* SetPortLinkMon */
    NULL_ONU_OK,    /* SetPortModeMon */
    NULL_ONU_OK,    /* SetPortIsolate */

    NULL_ONU_OK,    /* SetVlanEnable */
    NULL_ONU_OK,    /* SetVlanMode */
    NULL_ONU_OK,    /* AddVlan */
    NULL_ONU_OK,    /* DelVlan */
    NULL_ONU_OK,    /* SetPortPvid */

    NULL_ONU_OK,    /* AddVlanPort */
    NULL_ONU_OK,    /* DelVlanPort */
    NULL_ONU_OK,    /* SetEthPortVlanTran */
    NULL_ONU_OK,    /* DelEthPortVlanTran */
    NULL_ONU_OK,    /* SetEthPortVlanAgg */
    NULL_ONU_OK,    /* DelEthPortVlanAgg */

    NULL_ONU_OK,    /* SetPortQinQEnable */
    NULL_ONU_OK,    /* AddQinQVlanTag */
    NULL_ONU_OK,    /* DelQinQVlanTag */
    
    NULL_ONU_OK,    /* SetPortVlanFrameTypeAcc */
    NULL_ONU_OK,    /* SetPortIngressVlanFilter */

    NULL_ONU_OK,    /* SetPortMode */
    NULL_ONU_OK,    /* SetPortFcMode */
    NULL_ONU_OK,    /* SetPortAtuLearn */
    NULL_ONU_OK,    /* SetPortAtuFlood */
    NULL_ONU_OK,    /* SetPortLoopDetect */
    NULL_ONU_OK,    /* SetPortStatFlush */

    NULL_ONU_OK,    /* SetIngressRateLimitBase */
    NULL_ONU_OK,    /* SetPortIngressRate */
    NULL_ONU_OK,    /* SetPortEgressRate */

    NULL_ONU_OK,    /* SetQosClass */
    NULL_ONU_OK,    /* ClrQosClass */
    NULL_ONU_OK,    /* SetQosRule */
    NULL_ONU_OK,    /* ClrQosRule */

    NULL_ONU_OK,    /* SetPortQosRule */
    NULL_ONU_OK,    /* ClrPortQosRule */
    NULL_ONU_OK,    /* SetPortQosRuleType */

    NULL_ONU_OK,    /* SetPortDefPriority */
    NULL_ONU_OK,    /* SetPortNewPriority */
    NULL_ONU_OK,    /* SetQosPrioToQueue */
    NULL_ONU_OK,    /* SetQosDscpToQueue */
    
    NULL_ONU_OK,    /* SetPortUserPriorityEnable */
    NULL_ONU_OK,    /* SetPortIpPriorityEnable */
    NULL_ONU_OK,    /* SetQosAlgorithm */
    NULL_ONU_OK,    /* SetQosMode*/
    NULL_ONU_OK,    /* SetRule */

    NULL_ONU_OK,    /* SetIgmpEnable */
    NULL_ONU_OK,    /* SetIgmpAuth */
    NULL_ONU_OK,    /* SetIgmpHostAge */
    NULL_ONU_OK,    /* SetIgmpGroupAge */
    NULL_ONU_OK,    /* SetIgmpMaxResTime */
    
    NULL_ONU_OK,    /* SetIgmpMaxGroup */
    NULL_ONU_OK,    /* AddIgmpGroup */
    NULL_ONU_OK,    /* DeleteIgmpGroup */
    NULL_ONU_OK,    /* SetPortIgmpFastLeave */
    NULL_ONU_OK,    /* SetPortMulticastVlan */

    NULL_ONU_OK,    /* SetPortMirrorFrom */
    NULL_ONU_OK,    /* SetPortMirrorTo */
    NULL_ONU_OK,    /* DeleteMirror */
#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */
    NULL_ONU_error, /* RegisterCMC */
    NULL_ONU_error, /* UnregisterCMC */
    NULL_ONU_error, /* DumpCmc */
    NULL_ONU_error, /* DumpCmcAlarms */
    NULL_ONU_error, /* DumpCmcLogs */
    
    NULL_ONU_error, /* ResetCmcBoard */
    NULL_ONU_error, /* GetCmcVersion */
    NULL_ONU_error, /* GetCmcMaxMulticasts */
    NULL_ONU_error, /* GetCmcMaxCm */
    NULL_ONU_error, /* SetCmcMaxCm */
    
    NULL_ONU_error, /* GetCmcTime */
    NULL_ONU_error, /* SetCmcTime */
    NULL_ONU_error, /* LocalCmcTime */
    NULL_ONU_error, /* SetCmcCustomConfig */
    NULL_ONU_error, /* DumpCmcCustomConfig */

    NULL_ONU_error, /* DumpCmcDownChannel */
    NULL_ONU_error, /* DumpCmcUpChannel */
    NULL_ONU_error, /* GetCmcDownChannelMode */
    NULL_ONU_error, /* SetCmcDownChannelMode */
    NULL_ONU_error, /* GetCmcUpChannelMode */
    
    NULL_ONU_error, /* SetCmcUpChannelMode */
    NULL_ONU_error, /* GetCmcUpChannelD30Mode */
    NULL_ONU_error, /* SetCmcUpChannelD30Mode */
    NULL_ONU_error, /* GetCmcDownChannelFreq */
    NULL_ONU_error, /* SetCmcDownChannelFreq */

    NULL_ONU_error, /* GetCmcUpChannelFreq */
    NULL_ONU_error, /* SetCmcUpChannelFreq */
    NULL_ONU_error, /* GetCmcDownAutoFreq */
    NULL_ONU_error, /* SetCmcDownAutoFreq */
    NULL_ONU_error, /* GetCmcUpAutoFreq */
    
    NULL_ONU_error, /* SetCmcUpAutoFreq */
    NULL_ONU_error, /* GetCmcUpChannelWidth */
    NULL_ONU_error, /* SetCmcUpChannelWidth */
    NULL_ONU_error, /* GetCmcDownChannelAnnexMode */
    NULL_ONU_error, /* SetCmcDownChannelAnnexMode */

    NULL_ONU_error, /* GetCmcUpChannelType */
    NULL_ONU_error, /* SetCmcUpChannelType */
    NULL_ONU_error, /* GetCmcDownChannelModulation */
    NULL_ONU_error, /* SetCmcDownChannelModulation */
    NULL_ONU_error, /* GetCmcUpChannelProfile */

    NULL_ONU_error, /* SetCmcUpChannelProfile */
    NULL_ONU_error, /* GetCmcDownChannelInterleaver */
    NULL_ONU_error, /* SetCmcDownChannelInterleaver */
    NULL_ONU_error, /* GetCmcDownChannelPower */
    NULL_ONU_error, /* SetCmcDownChannelPower */

    NULL_ONU_error, /* GetCmcUpChannelPower */
    NULL_ONU_error, /* SetCmcUpChannelPower */
    NULL_ONU_error, /* DumpCmcUpChannelPower */
    NULL_ONU_error, /* DumpCmcUpChannelSignalQuality */
    NULL_ONU_error, /* DumpCmcInterfaceUtilization */

    NULL_ONU_error, /* DumpCmcInterfaceStatistics */
    NULL_ONU_error, /* DumpCmcMacStatistics */
    NULL_ONU_error, /* DumpCmcAllInterface */   
    NULL_ONU_error, /* DumpCmcAllLoadBalancingGrp */
    NULL_ONU_error, /* DumpCmcLoadBalancingGrp */

    NULL_ONU_error, /* DumpCmcLoadBalancingGrpDownstream */
    NULL_ONU_error, /* DumpCmcLoadBalancingGrpUpstream */
    NULL_ONU_error, /* DumpCmcLoadBalancingDynConfig */   
    NULL_ONU_error, /* SetCmcLoadBalancingDynMethod */
    NULL_ONU_error, /* SetCmcLoadBalancingDynDynPeriod */

    NULL_ONU_error, /* SetCmcLoadBalancingDynWeightedAveragePeriod */
    NULL_ONU_error, /* SetCmcLoadBalancingDynOverloadThresold */
    NULL_ONU_error, /* SetCmcLoadBalancingDynDifferenceThresold */
    NULL_ONU_error, /* SetCmcLoadBalancingDynMaxMoveNumber */
    NULL_ONU_error, /* SetCmcLoadBalancingDynMinHoldTime */

    NULL_ONU_error, /* SetCmcLoadBalancingDynRangeOverrideMode */
    NULL_ONU_error, /* SetCmcLoadBalancingDynAtdmaDccInitTech */
    NULL_ONU_error, /* SetCmcLoadBalancingDynScdmaDccInitTech */
    NULL_ONU_error, /* SetCmcLoadBalancingDynAtdmaDbcInitTech */
    NULL_ONU_error, /* SetCmcLoadBalancingDynScdmaDbcInitTech */

    NULL_ONU_error, /* CreateCmcLoadBalancingGrp */
    NULL_ONU_error, /* DestroyCmcLoadBalancingGrp */
    NULL_ONU_error, /* AddCmcLoadBalancingGrpDownstream */
    NULL_ONU_error, /* RemoveCmcLoadBalancingGrpDownstream */
    NULL_ONU_error, /* AddCmcLoadBalancingGrpUpstream */

    NULL_ONU_error, /* RemoveCmcLoadBalancingGrpUpstream */
    NULL_ONU_error, /* AddCmcLoadBalancingGrpModem */
    NULL_ONU_error, /* RemoveCmcLoadBalancingGrpModem */
    NULL_ONU_error, /* AddCmcLoadBalancingGrpExcludeModem */
    NULL_ONU_error, /* RemoveCmcLoadBalancingGrpExcludeModem */

    NULL_ONU_error, /* DumpCmcLoadBalancingGrpModem */
    NULL_ONU_error, /* DumpCmcLoadBalancingGrpActivedModem */
    NULL_ONU_error, /* DumpCmcLoadBalancingGrpExcludeModem */
    NULL_ONU_error, /* DumpCmcLoadBalancingGrpExcludeActivedModem */
    NULL_ONU_error, /* ReserveCmcLoadBalancingGrp */

    NULL_ONU_error, /* DumpAllCm */
    NULL_ONU_error, /* DumpCm */
    NULL_ONU_error, /* DumpAllCmHistory */
    NULL_ONU_error, /* DumpCmHistory */
    NULL_ONU_error, /* ClearAllCmHistory */

    NULL_ONU_error, /* ResetCM */
    NULL_ONU_error, /* DumpCmDownstream */
    NULL_ONU_error, /* DumpCmUpstream */
    NULL_ONU_error, /* SetCmDownstream */
    NULL_ONU_error, /* SetCmUpstream */

    NULL_ONU_error, /* CreateCmServiceFlow */
    NULL_ONU_error, /* ModifyCmServiceFlow */
    NULL_ONU_error, /* DestroyCmServiceFlow */
    NULL_ONU_error, /* DumpCmClassifier */
    NULL_ONU_error, /* DumpCmServiceFlow */
    
    NULL_ONU_error, /* DumpCmcClassifier */
    NULL_ONU_error, /* DumpCmcServiceFlow */
    NULL_ONU_error, /* DumpCmcServiceFlowStatistics */
    NULL_ONU_error, /* DumpCmcDownChannelBondingGroup */
    NULL_ONU_error, /* DumpCmcUpChannelBondingGroup */
    
    NULL_ONU_error, /* CreateCmcServiceFlowClassName */
    NULL_ONU_error, /* DestroyCmcServiceFlowClassName */
    NULL_ONU_error, /* GetCmcMacAddrTbl */
    NULL_ONU_error, /* GetCmMacAddrTbl */
    NULL_ONU_error, /* ResetCmAddrTbl */
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif
   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,

   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,
   
   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,

   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,

   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,
   NULL_ONU_error,
#if 0
    /*----------------GPON OMCI----------------------*/
    NULL,
    NULL,
#endif
/* --------------------------END-------------------------- */
    
    NULL_ONU_error  /* LastFun */
};


void ONU_NULL_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_NULL, &sonu_nullIfs);
}



#ifdef __cplusplus

}

#endif


