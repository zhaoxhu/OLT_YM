/***************************************************************
*
*      Module Name:  Onu_api.c
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
*   Date:    2010/05/13
*   Author:  shixh
*   content:
**  History:
**   Date        |    Name       |     Description
**---- ----- |-----------|------------------ 
**  10/05/1    |   shixh   |     create 
**----------|-----------|------------------
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "gwEponSys.h"

#include "onuConfMgt.h"
#include "onuOamUpd.h"
#include "Onu_oam_comm.h"
#include "ct_manage/CT_Onu_Voip.h"

extern ULONG	gLibEponMibSemId;
#ifdef __pending_onu_list__	
extern pendingOnu_S ConfOnuPendingQueue[500];
#endif

char cli_recvbuf[64*1024];


#if 1
/* -------------------ONU  API的安装及初始化接口------------------- */
static OnuMgmtIFs *s_aOnuIfs[ONU_ADAPTER_MAX];
static const OnuMgmtIFs *s_pRemoteOnuIfs = NULL;
static int               s_iRemoteOnuRlt;

#if 0
static onu_conf_mgt_if_t *s_pRemoteOnuConfMgtIfs = NULL;
static onu_conf_mgt_if_t *s_pLocalOnuConfMgtIfs = NULL;
#endif

ULONG onu_mgt_log_level = LOG_DEBUG;
int IsCtcOnu(ULONG pon_id, ULONG onu_id)
{
    int ret = FALSE, type;
    if(GetOnuType(pon_id, onu_id, &type) == VOS_OK && ((type == V2R1_ONU_CTC)  || (type == V2R1_ONU_GPON)/*|| type == V2R1_OTHER)*/))
        ret = TRUE;
    return ret;
}
const CHAR *gpGponOnuAppFileShortName = "gpon_app.bin";
extern UCHAR getOnuConfUndoFlag(short int ponid, short int onuid);
extern char *getOnuConfNamePtrByPonId(short int ponid, short int onuid);
extern LONG localDelOnuFromPon( short int pon_id, short int onu_idx );
/*add by luh 2011-9-24 检测是否需要在本地写数据；针对GW 完全透传时，不再本地保存*/
#define CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) (((!OnuTransmission_flag)||(IsCtcOnu(olt_id, onu_id)))&&(!getOnuConfRestoreFlag(olt_id,onu_id))&& (!onuConfIsShared(olt_id,onu_id)) &&(VOS_StrCmp(getOnuConfNamePtrByPonId(olt_id, onu_id), DEFAULT_ONU_CONF)))
/*qos 规则目前需要修改配置文件进行最后的下发*/
#define CHECK_ONU_LOCAL_CONF_ENABLE1(olt_id, onu_id) (((!OnuTransmission_flag)||(IsCtcOnu(olt_id, onu_id)))&&(!getOnuConfUndoFlag(olt_id,onu_id))&& (VOS_StrCmp(getOnuConfNamePtrByPonId(olt_id, onu_id), DEFAULT_ONU_CONF)))

void ONU_RegisterAdapter(ONU_ADAPTER_TYPE adater_type, OnuMgmtIFs *onuMgt_ifs)
{
    VOS_ASSERT((adater_type >= 0) && (adater_type < ONU_ADAPTER_MAX));
    VOS_ASSERT(onuMgt_ifs);

    /* sys_console_printf("ONU_RegisterAdapter\r\n"); */
    s_aOnuIfs[adater_type] = onuMgt_ifs;

    return;
}

void DEBUG_print_IFAddr(short int olt_id, short int onu_id)
{
	sys_console_printf("\r\nponidx:%d, onuidx:%d, tableAddr:0x%08x, addr:0x%08x\r\n",
			olt_id, onu_id, 
			(unsigned long)(&OnuMgmtTable[olt_id * MAXONUPERPON + onu_id]), 
			(unsigned long)(OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].OnuIFs));
}

int ONU_SetupIFs(short int olt_id, short int onu_id, ONU_ADAPTER_TYPE adater_type)
{
    int iRlt  = OLT_ERR_OK;
    int onuid = onu_id;
   
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT((adater_type >= 0) && (adater_type < ONU_ADAPTER_MAX));

    if ( NULL != s_aOnuIfs[adater_type] )
    {
        /* sys_console_printf("test ONU_SetupIFs onuid=%d \r\n",onuid); */
        ONU_MGMT_SEM_TAKE;
        OnuMgmtTable[olt_id * MAXONUPERPON + onuid].OnuIFs = s_aOnuIfs[adater_type];
        ONU_MGMT_SEM_GIVE;
    }
    else
    {
        iRlt = OLT_ERR_NOTSUPPORT;
    }

    return iRlt;
}

OnuMgmtIFs* ONU_GetIFsByType(int chip_type, int remote_type)
{
    static int s_chip2adaptergw[16] = { ONU_ADAPTER_NULL,       /*  ponChipType_null */
                                      ONU_ADAPTER_PAS5001_GW,   /*  ponChipType_PAS5001 */
                                      ONU_ADAPTER_PAS5201_GW,   /*  ponChipType_PAS5201 */
                                      ONU_ADAPTER_PAS5204_GW,   /*  ponChipType_PAS5204 */
                                      ONU_ADAPTER_PAS8411_GW,   /*  ponChipType_PAS8411 */
                                      ONU_ADAPTER_NULL,         /*  5 */
                                      ONU_ADAPTER_NULL,         /*  6 */
                                      ONU_ADAPTER_TK3723_GW,    /*  ponChipType_TK3723 */
                                      ONU_ADAPTER_BCM55524_GW,  /*  ponChipType_BCM55524 */
                                      ONU_ADAPTER_BCM55538_GW,  /*  ponChipType_BCM55538 */
                                      ONU_ADAPTER_NULL,         /*  10 */
                                      ONU_ADAPTER_NULL,         /*  11 */
                                      ONU_ADAPTER_NULL,         /*  12 */
                                      ONU_ADAPTER_NULL,         /*  13 */
                                      ONU_ADAPTER_NULL,         /*  14 */
                                      ONU_ADAPTER_GW };         /*  ponChipType_GW */

    static int s_chip2adapterctc[16] = { ONU_ADAPTER_NULL,      /*  ponChipType_null */
                                      ONU_ADAPTER_NULL,         /*  ponChipType_PAS5001 */
                                      ONU_ADAPTER_PAS5201_CTC,  /*  ponChipType_PAS5201 */
                                      ONU_ADAPTER_PAS5204_CTC,  /*  ponChipType_PAS5204 */
                                      ONU_ADAPTER_PAS8411_CTC,  /*  ponChipType_PAS8411 */
                                      ONU_ADAPTER_NULL,         /*  5 */
                                      ONU_ADAPTER_NULL,         /*  6 */
                                      ONU_ADAPTER_TK3723_CTC,   /*  ponChipType_TK3723 */
                                      ONU_ADAPTER_BCM55524_CTC, /*  ponChipType_BCM55524 */
                                      ONU_ADAPTER_BCM55538_CTC, /*  ponChipType_BCM55538 */
                                      ONU_ADAPTER_NULL,         /*  10 */
                                      ONU_ADAPTER_NULL,         /*  11 */
                                      ONU_ADAPTER_NULL,         /*  12 */
                                      ONU_ADAPTER_NULL,         /*  13 */
                                      ONU_ADAPTER_NULL,         /*  14 */
                                      ONU_ADAPTER_GW };         /*  ponChipType_GW */

    OnuMgmtIFs* pIFOnuMgmt = NULL;

    if ((chip_type > 0) && (chip_type < ponChipType_max))
    {
        switch (remote_type)
        {
            case ONU_MANAGE_GW:
                /* 类型为GW */
        	    pIFOnuMgmt = s_aOnuIfs[s_chip2adaptergw[chip_type]];
                break;
            case ONU_MANAGE_CTC:
                /* 类型为CTC */
        	    pIFOnuMgmt = s_aOnuIfs[s_chip2adapterctc[chip_type]];
                break;
            case ONU_MANAGE_GPON:
        	    pIFOnuMgmt = s_aOnuIfs[ONU_ADAPTER_BCM_GPON];
        	    break;
            default:
        	    pIFOnuMgmt = s_aOnuIfs[ONU_ADAPTER_NULL];
                break;
        }
    }

    return pIFOnuMgmt;
}

int ONU_SetupIFsByType(short int olt_id, short int onu_id, int chip_type, int remote_type)
{
    OnuMgmtIFs* pIFOnuMgmt;

    if ( NULL != (pIFOnuMgmt = ONU_GetIFsByType(chip_type, remote_type)) )
    {
        OLT_LOCAL_ASSERT(olt_id);
        ONU_ASSERT(onu_id);
        
        ONU_MGMT_SEM_TAKE;
        OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].OnuIFs = pIFOnuMgmt;
        ONU_MGMT_SEM_GIVE;
    }
    else
    {
        return OLT_ERR_NOTSUPPORT;
    }

    return 0;     
}

int ONU_GetIFType(short int olt_id, short int onu_id)
{
    const OnuMgmtIFs* pIFOnuMgmt;
    int iIfType = -1;
    int i;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    ONU_MGMT_SEM_TAKE;
    if ( NULL != (pIFOnuMgmt = OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].OnuIFs) )
    {
        for (i=0; i<ONU_ADAPTER_MAX; i++)
        {
            if (pIFOnuMgmt == s_aOnuIfs[i])
            {
                iIfType = i;
                break;
            }
        }
    }
    ONU_MGMT_SEM_GIVE;

    return iIfType;
}

extern void ONU_NULL_Support();
extern void ONU_RPC_Support();

extern void GWONU_Pas5001_Support();
extern void GWONU_Pas5201_Support();
extern void GWONU_Pas5204_Support();
#if defined(_EPON_10G_PMC_SUPPORT_)            
extern void GWONU_Pas8411_Support();/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
extern void CTCONU_Pas8411_Support();/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif
extern void CTCONU_Pas5201_Support();
extern void CTCONU_Pas5204_Support();
extern void GWONU_TK3723_Support();
extern void CTCONU_TK3723_Support();
extern void GWONU_BCM55524_Support();
extern void CTCONU_BCM55524_Support();
extern void GWONU_BCM55538_Support();
extern void CTCONU_BCM55538_Support();
extern void GPON_BCM_Support();


/* ONU远程管理的接口 */
void ONU_REMOTE_Support()
{
    s_pRemoteOnuIfs = s_aOnuIfs[ONU_ADAPTER_RPC];
}

void ONUmgt_API_Init()
{
    VOS_ASSERT(SYS_LOCAL_MODULE_TYPE_IS_PON_MANAGER);

    /* ONU支持预配置 */
    ONU_NULL_Support();

    /* ONU支持分布式配置 */
    ONU_RPC_Support();

    /* ONU支持远程配置 */
    ONU_REMOTE_Support();

    /* ONU支持的芯片类型 */
    if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
    {
    	GWONU_Pas5001_Support();
        
    	GWONU_Pas5201_Support();
    	GWONU_Pas5204_Support();
    	CTCONU_Pas5201_Support();
    	CTCONU_Pas5204_Support();

#if defined(_EPON_10G_PMC_SUPPORT_)            
		GWONU_Pas8411_Support();/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	    CTCONU_Pas8411_Support();/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif

        GWONU_TK3723_Support();
        CTCONU_TK3723_Support();
        GWONU_BCM55524_Support();
        CTCONU_BCM55524_Support();
        
#if defined(_EPON_10G_BCM_SUPPORT_)            
        GWONU_BCM55538_Support();
        CTCONU_BCM55538_Support();
#endif
        GPON_BCM_Support();
    }
}
#endif



#if 1
/* -------------------ONU API的部分本地GW实现------------------- */

extern LONG onu_mac_check_counter_get( SHORT PonPortIdx, SHORT OnuIdx, ULONG *pCounter );
extern long UpdateOnuAppReq(short int usPonID, short int usOnuID, char *type, char *name, long ltype, struct vty *vty );
extern int CTC_getAppNameWithModel( unsigned long model, char *name, int *pLen );

extern int portListLongToString(ULONG list, char *str);    /*added by luh 2012- 3-12*/
extern int VlanListArrayToString(ULONG *list, int length, char *str);

extern int onuconf_qosRuleFieldValue_ValToStr_parase( unsigned char select, qos_value_t *pMatchVal, unsigned char *pMatchStr );
extern char* qos_rule_field_sel_str ( int field_sel );
extern char* qos_rule_field_operator_str( int opr );


#if 1
/* -------------------ONU基本API------------------- */
int GWONU_OnuIsValid(short int olt_id, short int onu_id, int *status)
{
    int onuMgtIdx;
    unsigned char *MacAddr;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(status);

    onuMgtIdx = olt_id*MAXONUPERPON+onu_id;
    ONU_MGMT_SEM_TAKE;
    MacAddr   = OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr;
    if( MAC_ADDR_IS_INVALID(MacAddr) )
    {
        *status = 0;
    }
    else
    {
        *status = 1;
    }   
    ONU_MGMT_SEM_GIVE;

    /* OLT_GW_DEBUG(OLT_GW_TITLE"GW_OnuIsValid(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *status, 0, SYS_LOCAL_MODULE_SLOTNO); */
    
    return 0;
}

int GWONU_CmdIsSupported(short int olt_id, short int onu_id, short int *cmd)
{
    static short int aiCmdSpans[][2] = {
                                  {ONU_CMD_SPLIT_LINE_FRAME_BEGIN, ONU_CMD_SPLIT_LINE_FRAME_END},
                                  {ONU_CMD_SPLIT_LINE_AUTH_FUN_BEGIN, ONU_CMD_SPLIT_LINE_AUTH_FUN_END},
                                  {ONU_CMD_SPLIT_LINE_SERVICE_FUN_BEGIN, ONU_CMD_SPLIT_LINE_SERVICE_FUN_END},
                                  {ONU_CMD_SPLIT_LINE_MONITOR_FUN_BEGIN, ONU_CMD_SPLIT_LINE_MONITOR_FUN_END},
                                  {ONU_CMD_SPLIT_LINE_ENCRYPT_FUN_BEGIN, ONU_CMD_SPLIT_LINE_ENCRYPT_FUN_END},
                                  {ONU_CMD_SPLIT_LINE_ADDRTBL_FUN_BEGIN, ONU_CMD_SPLIT_LINE_ADDRTBL_FUN_END},
                                  {ONU_CMD_SPLIT_LINE_OPTICS_FUN_BEGIN, ONU_CMD_SPLIT_LINE_OPTICS_FUN_END},
                                  {ONU_CMD_SPLIT_LINE_HOTSWAP_FUN_BEGIN, ONU_CMD_SPLIT_LINE_HOTSWAP_FUN_END},
                                  {ONU_CMD_SPLIT_LINE_DEVICE_FUN_BEGIN, ONU_CMD_SPLIT_LINE_DEVICE_FUN_END},
                                  {ONU_CMD_SPLIT_LINE_CTC_FUN_BEGIN, ONU_CMD_SPLIT_LINE_CTC_FUN_END},
                                  {ONU_CMD_SPLIT_LINE_REMOTE_FUN_BEGIN, ONU_CMD_SPLIT_LINE_REMOTE_FUN_END},
                                  {ONU_CMD_SPLIT_LINE_CMC_FUN_BEGIN, ONU_CMD_SPLIT_LINE_CMC_FUN_END},

                                  {ONU_CMD_MAX, ONU_CMD_MAX}
                                 };
    OnuMgmtIFs *pOnuIfs;
    int iChipTypeId;
    int iRlt;
    short int sCmdID;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmd);

    iRlt = OLT_ERR_NOTSUPPORT;
    if ( ONU_CMD_MAX > (sCmdID = *cmd) )
    {
        iChipTypeId = OLTAdv_GetChipTypeID(olt_id);
    
        /* 检查本地的芯片类型函数表，获知是否支持特定的命令 */
        pOnuIfs = ONU_GetIFsByType(iChipTypeId, ONU_MANAGE_GW);
        if ( NULL != pOnuIfs )
        {
            int i;
            int iCmdIdx = -1;

            for (i=ARRAY_SIZE(aiCmdSpans)-2; i>=0; i--)
            {
                if ( sCmdID > aiCmdSpans[i][0] )
                {
                    if ( sCmdID < aiCmdSpans[i][1] )
                    {
                        iCmdIdx = sCmdID - aiCmdSpans[i][0];
                    }

                    break;
                }
            }

            if (iCmdIdx >= 0)
            {
                int iCmdNum = 0;
                    
                for (--i; i>=0; i--)
                {
                    iCmdNum += aiCmdSpans[i][1] - aiCmdSpans[i][0] - 1;
                }

                iCmdIdx += iCmdNum - 1;    
                if ( 0 != *((int*)pOnuIfs + iCmdIdx) )
                {
                    return OLT_ERR_OK;
                }
            }
        }
    }

    if ( 0 != iRlt )
    {
        *cmd = -sCmdID;
    }
    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_CmdIsSupported(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *cmd, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GWONU_CopyOnu(short int olt_id, short int onu_id, short int dst_olt_id, short int dst_onu_id, int copy_flags)
{
    int iRlt;

    iRlt = CopyOneOnu(dst_olt_id, dst_onu_id, olt_id, onu_id, copy_flags);
    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_CopyOnu(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, dst_olt_id, dst_onu_id, copy_flags, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GWONU_SetIFType(short int olt_id, short int onu_id, int chip_type, int remote_type)
{
    int iRlt;

    if ( PONCHIP_UNKNOWN == chip_type )
    {
        chip_type = OLTAdv_GetChipTypeID(olt_id);
    }
    
    if ( ONU_MANAGE_UNKNOWN == remote_type )
    {
        remote_type = ONU_MANAGE_GW;
    }

    iRlt = ONU_SetupIFsByType(olt_id, onu_id, chip_type, remote_type);
    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetIFType(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, chip_type, remote_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif


#if 1
/* -------------------ONU 业务管理API------------------- */
int GWONU_GetBWInfo(short int olt_id, short int onu_id, PonPortBWInfo_S *ponBW, ONUBWInfo_S *onuBW)
{
    int onuEntry = 0;
   
    OLT_LOCALID_ASSERT(olt_id);
	if( (NULL == ponBW) || (NULL == onuBW))
	{
	    ASSERT(0);
		return VOS_ERROR;
 	   
 	}

	onuEntry = olt_id * MAXONUPERPON + onu_id;

	ponBW->ActiveBW = PonPortTable[olt_id].ActiveBW;
	ponBW->BWExceedFlag = PonPortTable[olt_id].BWExceedFlag;
	ponBW->BWMode = PonPortTable[olt_id].BWMode;
	ponBW->DefaultOnuBW = PonPortTable[olt_id].DefaultOnuBW;
	ponBW->DownlinkActiveBw = PonPortTable[olt_id].DownlinkActiveBw;
	ponBW->DownlinkBWExceedFlag = PonPortTable[olt_id].DownlinkBWExceedFlag;
	ponBW->DownlinkMaxBw = PonPortTable[olt_id].DownlinkMaxBw;
	ponBW->DownlinkPoliceMode = PonPortTable[olt_id].DownlinkPoliceMode;
	ponBW->DownlinkProvisionedBW = PonPortTable[olt_id].DownlinkProvisionedBW;
	ponBW->DownlinkRemainBw = PonPortTable[olt_id].DownlinkRemainBw;
	ponBW->MaxBW = PonPortTable[olt_id].MaxBW;
	ponBW->ProvisionedBW = PonPortTable[olt_id].ProvisionedBW;
	ponBW->RemainBW = PonPortTable[olt_id].RemainBW;

	onuBW->FinalDownlinkBandwidth_be = OnuMgmtTable[onuEntry].FinalDownlinkBandwidth_be;
	onuBW->FinalDownlinkBandwidth_gr = OnuMgmtTable[onuEntry].FinalDownlinkBandwidth_gr;
	onuBW->FinalUplinkBandwidth_be = OnuMgmtTable[onuEntry].FinalUplinkBandwidth_be;
	onuBW->FinalUplinkBandwidth_fixed = OnuMgmtTable[onuEntry].FinalUplinkBandwidth_fixed;
	onuBW->FinalUplinkBandwidth_gr = OnuMgmtTable[onuEntry].FinalUplinkBandwidth_gr;

	return VOS_OK;
	 
}
#endif
/*End:for onu swap by jinhl@2013-04-27*/


#if 1
/* -------------------ONU加密管理API------------------- */
int GWONU_GetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int *key_change_time, int *encrypt_status)
{
    int onuMgtIdx;
    int encDir, keyTime, status;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    onuMgtIdx = olt_id * MAXONUPERPON + onu_id;
    ONU_MGMT_SEM_TAKE;
    encDir  = OnuMgmtTable[onuMgtIdx].EncryptDirection;
    keyTime = OnuMgmtTable[onuMgtIdx].EncryptKeyTime;
    status = OnuMgmtTable[onuMgtIdx].EncryptStatus;
    ONU_MGMT_SEM_GIVE;

    if ( NULL != encrypt_dir )
    {
        *encrypt_dir = encDir;
    }
    
    if ( NULL != key_change_time )
    {
        *key_change_time = keyTime;
    }
    if( NULL != encrypt_status )
	 *encrypt_status = status;

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_GetOnuEncryptParams(%d,%d,%d,%d,%d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, encDir, keyTime, status, 0, SYS_LOCAL_MODULE_SLOTNO);

    return 0;
}
#endif


#if 1
/* -------------------ONU 地址管理API------------------- */

/*获取超过ONU 最大门限的ONU 标志*//*add by shixh20120206*/
int GWONU_GetOnuMacCheckFlag(short int olt_id, short int onu_id, ULONG  *flag)
{
    int iRlt;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(flag);

    iRlt = onu_mac_check_counter_get(olt_id, onu_id, flag);
	OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_GetOnuMacCheckFlag(%d, %d)'s  on slot %d.\r\n", olt_id, onu_id, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif


#if 1
/* -------------------ONU 设备管理API------------------- */

int GPONONU_OnuSwUpdate(short int olt_id, short int onu_id,int update_flags, unsigned int update_filetype)
{
	int llid,iRlt=VOS_ERROR;
	OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	/*if(GetOnuSWUpdateStatus(olt_id,onu_id) == ONU_SW_UPDATE_STATUS_INPROGRESS)
	{
		ONU_UPDATE_DEBUG("onu %d/%d/%d image is updating\r\n", slot, port, onu );
		return( OLT_ERR_UPGRADE_INBUSY );
	}*/
	llid = GetLlidByOnuIdx(olt_id, onu_id);
	if(llid != INVALID_LLID)
	{
		ONU_UPDATE_DEBUG("GPONONU_OnuSwUpdate phyPonId=%d, userOnuId=%d",olt_id, llid);
		iRlt = gponOnuAdp_StartUpgrade(olt_id,llid,gpGponOnuAppFileShortName);
		return iRlt;
	}
	return iRlt;
}
/* gw-onu app自定义OAM 升级*/
int GWONU_OnuSwUpdate(short int olt_id, short int onu_id, int update_flags, unsigned int update_filetype)
{
	int slot, port, onu;
	char onuTypeString[ONU_TYPE_LEN+2]= {'\0'};
	int typeLen, ret;
	char OnuCurVersion[ONU_VERSION_LEN +1]= {'\0'};
	int VersionLen;
	char OnuFlashVersion[ONU_VERSION_LEN+1] ={'\0'};
	int offset=0, file_len=0, compress_flag=0, file_fact_len=0;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	slot = GetCardIdxByPonChip(olt_id);
	port = GetPonPortByPonChip(olt_id);
    onu  = onu_id + 1;

	if( GetOnuSWUpdateStatus(olt_id, onu_id ) == ONU_SW_UPDATE_STATUS_INPROGRESS )
	{
		ONU_UPDATE_DEBUG("onu %d/%d/%d image is updating\r\n", slot, port, onu );
		return( OLT_ERR_UPGRADE_INBUSY );
	}

	OnuMgmtTable[olt_id*MAXONUPERPON + onu_id].transFileLen = 0;
	
	VOS_MemSet( onuTypeString, 0, sizeof(onuTypeString) );

	/* 查找出要升级的程序文件名*/
	if( update_filetype == IMAGE_TYPE_APP )
	{
		ret = GetOnuAppTypeString(olt_id,onu_id, onuTypeString, &typeLen);
		if(  ret != ROK )
		{
			ONU_UPDATE_DEBUG("get onu %d/%d/%d %s app type err \r\n",  slot, port, onu, OnuImageType[update_filetype]);
			return (OLT_ERR_UPGRADE_FAILED);
		}
	}
	else if( update_filetype == IMAGE_TYPE_VOIP )
	{
		ret = GetOnuVoiceTypeString(olt_id,onu_id, onuTypeString, &typeLen);
		if(  ret != ROK )
		{
			ONU_UPDATE_DEBUG("get onu %d/%d/%d %s voice type err \r\n",  slot, port, onu, OnuImageType[update_filetype] );
			return (OLT_ERR_UPGRADE_FAILED);
		}
	}
	/* added by chenfj 2008-4-29
	     增加ONU FPGA 升级*/
	else if( update_filetype == IMAGE_TYPE_FPGA)
	{	
		ret = GetOnuFPGATypeString(olt_id,onu_id, onuTypeString, &typeLen);
		if(  ret != ROK )
		{
			ONU_UPDATE_DEBUG("get onu %d/%d/%d %s image type err \r\n",  slot, port, onu, OnuImageType[update_filetype] );
			return (OLT_ERR_UPGRADE_FAILED);
		}
	}
	else 
	{	
		ONU_UPDATE_DEBUG("image type wanted to be update not defined \r\n");
		return( OLT_ERR_UPGRADE_FAILED );
	}
	
	/* 比较当前运行版本与FLASH 中保存程序版本是否一致*/
	if((update_filetype == IMAGE_TYPE_APP ) || (update_filetype == IMAGE_TYPE_VOIP ))
	{
		ret = GetOnuSWVersion(olt_id,  onu_id, OnuCurVersion, &VersionLen);
		if( ret == ERROR )
		{
			ONU_UPDATE_DEBUG("Get onu%d/%d/%d %s image version err\r\n", slot, port, onu, OnuImageType[update_filetype] );
			return( OLT_ERR_UPGRADE_FAILED );
		}	
		
		if( OnuEponAppHasVoiceApp(olt_id, onu_id) == ROK )
		{
			char *Ptr = NULL;
			if( update_filetype == IMAGE_TYPE_APP )
			{
				Ptr = VOS_StrStr(OnuCurVersion, "/");
				if(Ptr != NULL )
				OnuCurVersion[Ptr -&OnuCurVersion[0]] ='\0';
			}
			else if( update_filetype == IMAGE_TYPE_VOIP)
			{
				unsigned int len = 0;
				Ptr = VOS_StrStr( OnuCurVersion, "/");
				if(Ptr != NULL )
				{
					len = VOS_StrLen( Ptr+1 );
					VOS_StrCpy(OnuCurVersion, (Ptr+1) );
					OnuCurVersion[len ]='\0';
				}
				else  OnuCurVersion[0] = '\0';
					
				ONU_UPDATE_DEBUG("onu %d/%d/%d %s image version: %s\r\n", slot,port, onu, &(OnuCurVersion[0]),OnuImageType[update_filetype] );
			}
		}
	}
	/* added by chenfj 2008-4-29
	     增加ONU FPGA 升级*/
	else if( update_filetype == IMAGE_TYPE_FPGA)
	{
		ret = GetOnuFWVersion(olt_id,  onu_id, OnuCurVersion, &VersionLen);
		if( ret == ERROR )
		{
			ONU_UPDATE_DEBUG("Get onu%d/%d/%d %s image version err\r\n", slot, port, onu, OnuImageType[update_filetype] );
			return( OLT_ERR_UPGRADE_FAILED );
		}

		if( OnuFirmwareHasFpgaApp(olt_id, onu_id) == ROK )
		{
			unsigned int len = 0;
			char *Ptr = NULL;
			Ptr = VOS_StrStr( OnuCurVersion, "/");
			if(Ptr != NULL )
			{
				len = VOS_StrLen( Ptr+1 );
				VOS_StrCpy(OnuCurVersion, (Ptr+1) );		
				OnuCurVersion[len ]='\0';
			}
			else OnuCurVersion[0]='\0';
			
			ONU_UPDATE_DEBUG("onu %d/%d/%d %s image version: %s\r\n", slot,port, onu, &(OnuCurVersion[0]),OnuImageType[update_filetype] );
		}
			
	}

	{
		ret = get_ONU_Info( (char *)&(onuTypeString[0]), &offset, &file_len, &compress_flag, &file_fact_len, (int*)&(OnuFlashVersion[0]) );
		if(( ret != VOS_OK ) || ( file_len == 0 ))
		{
			ONU_UPDATE_DEBUG("there is no %s image for onu%d/%d/%d in flash\r\n",  onuTypeString, slot, port, onu );
			return( OLT_ERR_UPGRADE_NOFILE );
		}
	}
	
	/* #3277 问题单: ONU软件升级时，放开对版本是否一致的检查 */
	if( VOS_StrCmp( OnuCurVersion, OnuFlashVersion ) != 0 )
	{
		char *OnuFileType;
		
		SetOnuSwUpdateStatus(olt_id, onu_id , ONU_SW_UPDATE_STATUS_INPROGRESS, update_filetype );
		ONU_UPDATE_DEBUG("send %s image to onu %d/%d/%d Start.....\r\n", OnuImageType[update_filetype], slot, port, onu ) ;
		switch ( update_filetype )
		{
			case  IMAGE_TYPE_APP:
				OnuFileType = ONU_APP_NAME;
				break;
			case IMAGE_TYPE_VOIP:
				OnuFileType = ONU_VOICE_NAME;
				break;
			case IMAGE_TYPE_FPGA:
				OnuFileType = ONU_FPGA_NAME;
				break;
			default:				
				ONU_UPDATE_DEBUG("Onu file image type err\r\n");
				return( OLT_ERR_UPGRADE_FAILED );
		}

		if(UpdateOnuAppReq(olt_id, onu, onuTypeString, OnuFileType, update_flags, NULL) != ROK)
			return( OLT_ERR_UPGRADE_FAILED );
	}
	else
    {
		ONU_UPDATE_DEBUG("onu %d/%d/%d %s Image running currently is the newest one\r\n",  slot, port, onu, OnuImageType[update_filetype] );
		return( OLT_ERR_UPGRADE_VERION );
	}
	
	return(OLT_ERR_OK);
}

int CTCONU_OnuSwUpdate(short int olt_id, short int onu_id, int update_flags, unsigned int update_filetype)
{
	int OnuType;
	int slot, port, onu;
	
	unsigned char  onuTypeString[ONU_TYPE_LEN+2] ={'\0'};
	int length = 0;
	int file_len = 0;
		
	int ret;
	long retval;

#if 0		
	int FE_num=0, POTS_num=0, E1_num=0;
#endif

		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	slot = GetCardIdxByPonChip(olt_id);
	port = GetPonPortByPonChip(olt_id);
    onu  = onu_id + 1;
	
	if( GetOnuSWUpdateStatus(olt_id, onu_id ) == ONU_SW_UPDATE_STATUS_INPROGRESS )
	{
		ONU_UPDATE_DEBUG("onu %d/%d/%d image is updating\r\n", slot, port, onu );
		return( OLT_ERR_UPGRADE_INBUSY );
	}

	ret = GetOnuType(olt_id, onu_id, &OnuType );	
	if( ret != ROK )
	{
		ONU_UPDATE_DEBUG("get onu %d/%d/%d type err \r\n",  slot, port, onu );
		return (OLT_ERR_UPGRADE_FAILED);
	}

	if( OnuType == V2R1_ONU_CTC )
	{
#if 0		
		CTC_getDeviceCapEthPortNum(olt_id, onu_id, &FE_num);
		CTC_getDeviceCapiadPotsPortNum(olt_id, onu_id, &POTS_num);
		CTC_getDeviceCapE1PortNum( olt_id, onu_id, &E1_num);

		if(( FE_num <= 4 ) && ( E1_num == 0 ) && ( POTS_num == 0))
			{
			unsigned char *Ptr;
			
			Ptr = V2R1_CTC_ONU_4FE;
			length = VOS_StrLen( Ptr );
			VOS_MemCpy( &(onuTypeString[0]), Ptr, length);
			}
		else if(( FE_num <= 4 ) && (E1_num == 0 ) && ( POTS_num == 2))
			{
			unsigned char *Ptr;

			Ptr = V2R1_CTC_ONU_4FE_2VOIP;
			length = VOS_StrLen( Ptr );
			VOS_MemCpy( &(onuTypeString[0]), Ptr, length);
			}
		else{
			ONU_UPDATE_DEBUG("ctc onu %d/%d/%d type mismatch \r\n",  slot, port, onu );
			return( OLT_ERR_UPGRADE_FAILED );
			}
#else
#if 0
		VOS_StrCpy( onuTypeString, V2R1_CTC_ONU_4FE );
		length = VOS_StrLen(onuTypeString);
#else
		CTC_getAppNameWithModel(OnuMgmtTable[olt_id*MAXONUPERPON+onu_id].onu_model, onuTypeString, &length);
#endif
#endif
	}
	
	else GetOnuTypeString(olt_id, onu_id, onuTypeString, &length);

	retval = get_ONU_Info( &(onuTypeString[0]), NULL, &file_len, NULL, NULL, NULL );
	if(( retval != VOS_OK ) || (file_len == 0 ))
	{
		ONU_UPDATE_DEBUG("no %s image in flash,transfer onu %d/%d/%d app image err\r\n", &(onuTypeString[0]), slot, port, onu);
		return( OLT_ERR_UPGRADE_NOFILE );
	}

	SendUpdateOnuImageMsg( olt_id, onu_id, onuTypeString, length  );
	return( ONU_UPDATE_OK );
}

/* 这个函数用于ONU程序在GW模式和CTC模式之间切换*/
int GWONU_OnuGwCtcSwConvert(short int olt_id, short int onu_id, char file_id[ONU_TYPE_LEN + 4])
{
	int update_mode;
	int slot, port, onu;
#if 0
	unsigned char onuTypeString[ONU_TYPE_LEN+2] ={'\0'};
#else
    char *onuTypeString;
#endif
	int length ;
	int file_len = 0;
#if 0
    int file_len1 = 0;
	int OnuType;
	int FE_num=0, POTS_num=0, E1_num=0;
	short int ret;
#endif

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(file_id);
	
	slot = GetCardIdxByPonChip(olt_id);
	port = GetPonPortByPonChip(olt_id);
    onu  = onu_id + 1;

	if( GetOnuSWUpdateStatus(olt_id, onu_id ) == ONU_SW_UPDATE_STATUS_INPROGRESS )
	{
		ONU_UPDATE_DEBUG("onu %d/%d/%d app image update already started\r\n", slot, port, onu );
		return( ONU_UPDATE_INPROCESS);
	}

	update_mode = get_onu_image_update_mode( NULL, olt_id, onu_id );
	if(( update_mode != V2R1_UPDATE_ONU_IMAGE_CTC ) &&  ( update_mode != V2R1_UPDATE_ONU_IMAGE_GW )) return( OLT_ERR_UPGRADE_FAILED );

    onuTypeString = file_id;
    length = VOS_StrLen( onuTypeString );

#if 0
	ret = GetOnuType(olt_id, onu_id, &OnuType );	
	if(  ret != ROK )
		{
		OLT_PAS_DEBUG("get onu %d/%d/%d type err \r\n",  slot, port, onu );
		return (OLT_ERR_UPGRADE_FAILED);
		}

	/* 将ONU 程序升级到GW私有模式*/
	if(( UpdateFileType == V2R1_UPDATE_ONU_IMAGE_GW )/* ||((IsSupportCTCOnu(PonPortIdx) == TRUE )&&(GetOnuExtOAMStatus(PonPortIdx, OnuIdx) == V2R1_ENABLE) && ())*/)
		{
		/* 原来运行的是CTC 程序*/
		if( OnuType == V2R1_ONU_CTC )
			{
			CTC_getDeviceCapEthPortNum(olt_id, onu_id, &FE_num);
			CTC_getDeviceCapiadPotsPortNum(olt_id, onu_id, &POTS_num);
			CTC_getDeviceCapE1PortNum( olt_id, onu_id, &E1_num);

			if(( FE_num == 1 ) && ( E1_num == 0 ) && ( POTS_num == 0)) /* GT816/810 */
				{
				length = VOS_StrLen(GetOnuEponAppString(V2R1_ONU_GT816));
				VOS_MemCpy( &(onuTypeString[0]), GetOnuEponAppString(V2R1_ONU_GT816)/* V2R1_CTC_ONU_4FE*/, length );
				}
			else if(( FE_num == 4 ) && ( E1_num == 0 ) && ( POTS_num == 0)) /* GT811_A */
				{
				length = VOS_StrLen(GetOnuEponAppString(V2R1_ONU_GT811_A));
				VOS_MemCpy( &(onuTypeString[0]), GetOnuEponAppString(V2R1_ONU_GT811_A)/* V2R1_CTC_ONU_4FE*/, length );
				}
			else if(( FE_num == 4 ) && ( E1_num == 0 ) && ( POTS_num == 2)) /* GT831_A */
				{
				length = VOS_StrLen(GetOnuEponAppString(V2R1_ONU_GT831_A));
				VOS_MemCpy( &(onuTypeString[0]), GetOnuEponAppString(V2R1_ONU_GT831_A)/* V2R1_CTC_ONU_4FE*/, length );
				}
			else{
				OLT_PAS_DEBUG("ctc onu %d/%d/%d type mismatch,no corresponding %s file for this onu\r\n",  slot, port, onu, typesdb_product_corporation_AB_name() );
				return( OLT_ERR_UPGRADE_FAILED );
				}
			}

		/* 原来运行的是GW私有模式,  处理待增加*/
		else {
			/* modified by chenfj 2007-10-22
			问题单#5556: [GT812_A]执行命令update onu file [ct2gw|gw2ct],提示不匹配
			在信息提示中，增加更详细的说明
			*/
			OLT_PAS_DEBUG("%%mismatch cli parameter, used when Updating Onu file from CT mode to %s mode\r\n", typesdb_product_corporation_AB_name());
			return( OLT_ERR_UPGRADE_FAILED );
			}		
			
		}

	/* 将ONU 程序升级到CTC 模式*/
	else if( UpdateFileType == V2R1_UPDATE_ONU_IMAGE_CTC )
		{
		if( GetOnuTypeString(olt_id, onu_id, onuTypeString, &length) != ROK )
			{
			OLT_PAS_DEBUG("get onu %d/%d/%d type err \r\n",  slot, port, onu );
			return (OLT_ERR_UPGRADE_FAILED);
			}

		/*  当前运行程序为gt810/816/GT811_A, 则升级程序CTC_4FE文件*/
		if(( VOS_StrCmp(onuTypeString, GetDeviceTypeString(V2R1_ONU_GT810)) == 0) || ( VOS_StrCmp(onuTypeString, GetDeviceTypeString(V2R1_ONU_GT816)) == 0) || ( VOS_StrCmp(onuTypeString, GetDeviceTypeString(V2R1_ONU_GT811_A)) == 0) ||
			( VOS_StrCmp(onuTypeString, GetDeviceTypeString(V2R1_ONU_GT811_B)) == 0))
			{
			unsigned char *Ptr;
			
			Ptr = V2R1_CTC_ONU_4FE;
			length = VOS_StrLen( Ptr );			
			VOS_MemCpy( &(onuTypeString[0]), Ptr, length);		
			}

		/* 当前运行其他程序, 处理待增加*/
		else {
			/* modified by chenfj 2007-10-22
			问题单#5556: [GT812_A]执行命令update onu file [ct2gw|gw2ct],提示不匹配
			在信息提示中，增加更详细的说明
			*/
			OLT_PAS_DEBUG("%% mismatch cli parameter, used when Updating Onu file from %s mode to CTC mode for %s/%s currently\r\n", PRODUCT_CORPORATION_AB_NAME,GetDeviceTypeString(V2R1_ONU_GT810),GetDeviceTypeString(V2R1_ONU_GT816)/*DEVICE_TYPE_NAME_GT810_STR,DEVICE_TYPE_NAME_GT816_STR*/);
			return( OLT_ERR_UPGRADE_FAILED );
			}
		
		}
#endif

#if 0
	/* modified by chenfj 2007-8-16 
		GT816/GT810 可能使用同一个文件(当前),也可能使用不同文件(以后); 故特殊判断*/
	if(( VOS_StrCmp( (char *)&(onuTypeString[0]), GetDeviceTypeString(V2R1_ONU_GT816)) == 0)
		|| ( VOS_StrCmp( (char *)&(onuTypeString[0]), GetDeviceTypeString(V2R1_ONU_GT810)) == 0))
		
		{
		long ret1, ret2;
		ret1 = get_ONU_Info( GetDeviceTypeString(V2R1_ONU_GT810), NULL, &file_len, NULL, NULL, NULL );
		ret2 = get_ONU_Info( GetDeviceTypeString(V2R1_ONU_GT816), NULL, &file_len1, NULL, NULL, NULL );

		/* 两个文件都没有,报错*/
		if((( ret1 != VOS_OK) && ( ret2 != VOS_OK )) ||((file_len == 0 ) && (file_len1 == 0)))
			{
			v2r1_printf(vty, "  no %s image in flash\r\n",  onuTypeString );
			return( ONU_UPDATE_ERR );
			}

		/* 两个文件都有,各使用各的*/
		else if(( file_len != 0) && ( file_len1 !=  0 )) 
			{

			}

		/* 以下为只有一个文件*/
		else if( file_len != 0)
			{
			VOS_MemCpy((char *)&(onuTypeString[0]), GetDeviceTypeString(V2R1_ONU_GT810), VOS_StrLen(GetDeviceTypeString(V2R1_ONU_GT810)));
			}
		else if( file_len1 !=  0 )
			{
			VOS_MemCpy((char *)&(onuTypeString[0]), GetDeviceTypeString(V2R1_ONU_GT816), VOS_StrLen(GetDeviceTypeString(V2R1_ONU_GT816)));
			}
		
		else {
			return( OLT_ERR_UPGRADE_FAILED );
			}
			
		}

	/* 其他类型不做修改*/

	else
#endif
    {
		long ret1;
		ret1 = get_ONU_Info( &(onuTypeString[0]), NULL, &file_len, NULL, NULL, NULL );
		if(( ret1 != VOS_OK ) || ( file_len == 0 ))
		{
			ONU_UPDATE_DEBUG("no %s image in flash, update onu %d/%d/%d app image err\r\n", &(onuTypeString[0]),slot, port, onu);
			return( OLT_ERR_UPGRADE_NOFILE );
		}
	}
	
	/*if(VOS_StrCmp( onuTypeString, V2R1_CTC_ONU ) == 0)*/
	if( update_mode == V2R1_UPDATE_ONU_IMAGE_CTC )
		SendUpdateOnuImageMsg( olt_id, onu_id, onuTypeString, length );
	else
	{
		SetOnuSwUpdateStatus(olt_id, onu_id, ONU_SW_UPDATE_STATUS_INPROGRESS, IMAGE_TYPE_APP);
		ONU_UPDATE_DEBUG("send app image to onu %d/%d/%d Start...\r\n", slot, port, onu) ;
		
		if(UpdateOnuAppReq(olt_id, onu, onuTypeString, ONU_APP_NAME, V2R1_NO_WAIT_RETURN, NULL) != ROK )
			return( OLT_ERR_UPGRADE_FAILED );
	}
	
	return( ONU_UPDATE_OK );
}

int GWONU_SetOnuDeviceName(short int olt_id, short int onu_id, char *Name, int NameLen)
{
    int iRlt = OLT_ERR_UNKNOEWN;
    int length = 0;
    
    char pBuf[EUQ_MAX_OAM_PDU];
    int len = 0;
    char pRxBuf[EUQ_MAX_OAM_PDU];
    short int Rxlen = 0;
#ifdef ONU_COMM_SEMI
	int GetSemId;
#endif
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(Name);
    
#if 0
    do{
#ifdef ONU_COMM_SEMI
    	GetSemId = VOS_SemTake( OnuEUQCommSemId, WAIT_FOREVER );
    	if(GetSemId == VOS_ERROR )
        {
    		VOS_ASSERT(0);
    		/*sys_console_printf( "  get SemId error(SetOnuEUQInfo)\r\n" );*/
    		break;
    	}
#endif

    	if( GetOnuOperStatus(olt_id, onu_id) != ONU_OPER_STATUS_UP )
    	{
    		/*DelOneNodeFromGetEUQ( PonPortIdx, OnuIdx );*/
    		sys_console_printf("\r\n  pon%d/%d Onu %d Not in Online status\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), (onu_id+1));
#ifdef ONU_COMM_SEMI
    		VOS_SemGive(OnuEUQCommSemId);
#endif
            iRlt = OLT_ERR_NOTEXIST;
    		break;
    	}
	
        {
    	char pBuf[EUQ_MAX_OAM_PDU];
    	int len;
        
        len = 0;
        VOS_MemSet( pBuf, 0,  EUQ_MAX_OAM_PDU );

        /* modified by chenfj 2007-7-3,当长度为0时，则为清除*/
        if( NameLen != 0 )
        {
            pBuf[0] = NameLen;
            VOS_MemCpy( &pBuf[1], Name, NameLen );

            len = NameLen + 3;
        }
        else
        {
            pBuf[0] = 1;
            pBuf[1] = 0;
            len = 3+1;
        }

    	if ( ROK != (iRlt = EQU_SendMsgToOnu_New( olt_id, onu_id, SET_ONU_SYS_INFO_REQ, pBuf, len)) )
    	{
    		sys_console_printf("\r\nSet ONU device Info, wait ...  Timeout\r\n");
    	}
        
#ifdef ONU_COMM_SEMI
    	VOS_SemGive(OnuEUQCommSemId);
#endif
        }
    }while(0);
#else
    VOS_MemSet( pBuf, 0,  EUQ_MAX_OAM_PDU );
    VOS_MemSet( pRxBuf, 0,  EUQ_MAX_OAM_PDU );

    if( NameLen != 0 )
    {
        pBuf[0] = NameLen;
        VOS_MemCpy( &pBuf[1], Name, NameLen );
        len = NameLen + 3;
    }
    else
    {
        pBuf[0] = 1;
        pBuf[1] = 0;
        len = 3+1;
    }
    /*modified by luh 2012-9-17*/
    iRlt = Oam_Session_Send(olt_id, onu_id, SET_ONU_SYS_INFO_REQ, OAM_SYNC, 0, 0, NULL, pBuf, len, pRxBuf, &Rxlen);
    if(iRlt == VOS_OK)
    {
        DEBUG_OAM_SESSION_PRINTF("GWONU_SetOnuDeviceName: length = %d msgtype = %d result = %d\r\n", Rxlen, pRxBuf[0], pRxBuf[1]);        
        if(pRxBuf[1] != 1)
           iRlt = VOS_ERROR; 
    }
#endif
    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetOnuDeviceName(%d, %d, %s, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, Name, NameLen, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

int GWONU_SetOnuDeviceDesc(short int olt_id, short int onu_id, char *Desc, int DescLen)
{
    int iRlt = OLT_ERR_UNKNOEWN;
    char pBuf[EUQ_MAX_OAM_PDU];
    int len = 0;
    char pRxBuf[EUQ_MAX_OAM_PDU];
    short int Rxlen = 0;
#ifdef ONU_COMM_SEMI
	int GetSemId;
#endif
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(Desc);
    
#if 0
    do{
#ifdef ONU_COMM_SEMI
    	GetSemId = VOS_SemTake( OnuEUQCommSemId, WAIT_FOREVER );
    	if(GetSemId == VOS_ERROR )
        {
    		VOS_ASSERT(0);
    		/*sys_console_printf( "get SemId error(SetOnuEUQInfo)\r\n" );*/
    		break;
    	}
#endif

    	if( GetOnuOperStatus( olt_id, onu_id) != ONU_OPER_STATUS_UP )
    	{
    		/*DelOneNodeFromGetEUQ( PonPortIdx, OnuIdx );*/
    		sys_console_printf("\r\n  pon%d/%d Onu %d Not in Online status\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), (onu_id+1));
#ifdef ONU_COMM_SEMI
    		VOS_SemGive(OnuEUQCommSemId);
#endif
            iRlt = OLT_ERR_NOTEXIST;
    		break;
    	}
    
    	{
    	char pBuf[EUQ_MAX_OAM_PDU];
    	int len;
        
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
    	else
        {
    		pBuf[0] = 0;
    		pBuf[1] = 1;
    		pBuf[2] = 0;
    		len = 1 + 3;
    	}

    	if ( ROK != (iRlt = EQU_SendMsgToOnu_New( olt_id, onu_id, SET_ONU_SYS_INFO_REQ, pBuf, len)) )
		{
    		sys_console_printf("\r\nSet ONU device Info, wait ...  Timeout\r\n");
		}

#ifdef ONU_COMM_SEMI
    	VOS_SemGive(OnuEUQCommSemId);
#endif
    	}
    }while(0);
#else
    VOS_MemSet( pBuf, 0,  EUQ_MAX_OAM_PDU );
    VOS_MemSet( pRxBuf, 0,  EUQ_MAX_OAM_PDU );

    if( DescLen != 0 )
    {
        pBuf[0] = 0;
        pBuf[1] = DescLen;
        VOS_MemCpy( &pBuf[2] , Desc, DescLen );

        len = DescLen + 3;
    }
    else
    {
        pBuf[0] = 0;
        pBuf[1] = 1;
        pBuf[2] = 0;
        len = 1 + 3;
    }
    /*modified by luh 2012-9-17*/
    iRlt = Oam_Session_Send(olt_id, onu_id, SET_ONU_SYS_INFO_REQ, OAM_SYNC, 0, 0, NULL, pBuf, len, pRxBuf, &Rxlen);
    if(iRlt == VOS_OK)
    {
        DEBUG_OAM_SESSION_PRINTF("GWONU_SetOnuDeviceDesc: length = %d msgtype = %d, result = %d\r\n", Rxlen, pRxBuf[0], pRxBuf[1]);
        if(pRxBuf[1] != 1)
           iRlt = VOS_ERROR; 
    }    
#endif
    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_PAS5001_SetOnuDeviceDesc(%d, %d, %s, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, Desc, DescLen, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

int GWONU_SetOnuDeviceLocation(short int olt_id, short int onu_id, char *Location, int LocationLen)
{
    int iRlt = OLT_ERR_UNKNOEWN;
    char pBuf[EUQ_MAX_OAM_PDU];
    int len = 0;
    char pRxBuf[EUQ_MAX_OAM_PDU];
    short int Rxlen = 0;
#ifdef ONU_COMM_SEMI
	int GetSemId;
#endif
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(Location);
#if 0
    do{
#ifdef ONU_COMM_SEMI
    	GetSemId = VOS_SemTake( OnuEUQCommSemId, WAIT_FOREVER );
    	if(GetSemId == VOS_ERROR ) 
    	{
    		VOS_ASSERT(0);
    		/*sys_console_printf( "get SemId error(SetOnuEUQInfo)\r\n" );*/
    		break;
    	}
#endif

    	if( GetOnuOperStatus( olt_id, onu_id) != ONU_OPER_STATUS_UP )
    	{
    		/*DelOneNodeFromGetEUQ( PonPortIdx, OnuIdx );*/
    		sys_console_printf("\r\n  pon%d/%d Onu %d Not in Online status\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), (onu_id+1));
#ifdef ONU_COMM_SEMI
    		VOS_SemGive(OnuEUQCommSemId);
#endif
            iRlt = OLT_ERR_NOTEXIST;
    		break;
    	}
    
    	{
    	char pBuf[EUQ_MAX_OAM_PDU];
    	int len;
        
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
        else
        {
            pBuf[2] = 1;
            pBuf[3] = 0;
            len = 1 + 3;
        }

    	if ( ROK != (iRlt = EQU_SendMsgToOnu_New( olt_id, onu_id, SET_ONU_SYS_INFO_REQ, pBuf, len)) )
    	{
    		sys_console_printf("\r\nSet ONU device Info, wait ...  Timeout\r\n");
    	}

#ifdef ONU_COMM_SEMI
    	VOS_SemGive(OnuEUQCommSemId);
#endif
    	}
    }while(0);
#else
        /* modified by chenfj 2007-7-3,当长度为0时，则为清除*/
            
        VOS_MemSet( pBuf, 0,  EUQ_MAX_OAM_PDU );
        VOS_MemSet( pRxBuf, 0,  EUQ_MAX_OAM_PDU );

        pBuf[0] = 0;
        pBuf[1] = 0;
        if( LocationLen != 0 )
        {
            pBuf[2] = LocationLen;
            VOS_MemCpy( &pBuf[3] , Location, LocationLen );
            len = LocationLen + 3;
        }
        else
        {
            pBuf[2] = 1;
            pBuf[3] = 0;
            len = 1 + 3;
        }
    /*modified by luh 2012-9-17*/        
    iRlt = Oam_Session_Send(olt_id, onu_id, SET_ONU_SYS_INFO_REQ, OAM_SYNC, 0, 0, NULL, pBuf, len, pRxBuf, &Rxlen);
    if(iRlt == VOS_OK)
    {
        DEBUG_OAM_SESSION_PRINTF("GWONU_SetOnuDeviceLocation: length = %d msgtype = %d result = %d\r\n", Rxlen, pRxBuf[0], pRxBuf[1]);
        if(pRxBuf[1] != 1)
           iRlt = VOS_ERROR; 
    }    
#endif
    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetOnuDeviceLocation(%d, %d, %s, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, Location, LocationLen, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

int GWONU_GetOnuAllPortStatisticData(short int olt_id, short int onu_id, OnuStatisticData_S* data)
{
    int iRlt = OLT_ERR_UNKNOEWN;
    char pBuf[EUQ_MAX_OAM_PDU];
    int len = 0;
    char pRxBuf[EUQ_MAX_OAM_PDU];
    short int Rxlen = 0;
	ULONG portMask = 0;
    int portNum = 0;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemZero(pBuf, EUQ_MAX_OAM_PDU);
    VOS_MemZero(pRxBuf, EUQ_MAX_OAM_PDU);
    /*前8个字节的首bit位代表字节数统计*/
    pBuf[0] = 0x80;
    /*后4  字节全FF代表所有端口*/
    VOS_MemSet(&pBuf[8], 0xff, 4);
    len = 12;
    iRlt = Oam_Session_Send(olt_id, onu_id, GET_ONU_ALLPORT_STATISTIC, OAM_SYNC, 0, 0, NULL, pBuf, len, pRxBuf, &Rxlen);
    if(iRlt == VOS_OK)
    {
        DEBUG_OAM_SESSION_PRINTF("GWONU_GetOnuAllPortStatisticData: length = %d msgtype = %d result = %d\r\n", Rxlen, pRxBuf[0], pRxBuf[1]);
		if(Rxlen <= 14)
			return VOS_ERROR;
        if(pRxBuf[1] != 1)
           iRlt = VOS_ERROR; 
        else
        {
            int dataLen = Rxlen-14;
            portMask = *(ULONG *)&pRxBuf[10];
            while(portMask)
            {
                if(portMask&1)
                    portNum++;
                portMask >>= 1;
            }
            if(portNum*sizeof(INT64U)*2 == dataLen)
            {
                int port = 0;
                char* pdata = &pRxBuf[14];
                data->portNum = portNum;
                if(OnuSupportSnmpPonStatistic(olt_id, onu_id) == VOS_OK)
                {
                    for(port = 0; port<portNum; port++)
                    {
                        VOS_MemCpy(&data->data[port].upStreamOctets, pdata, 8);
                        pdata += sizeof(INT64U);
                        VOS_MemCpy(&data->data[port].downStreamOctets, pdata, 8);
                        pdata += sizeof(INT64U);
                    }                
                }
                else
                {
                    for(port = 1; port<=portNum; port++)
                    {
                        VOS_MemCpy(&data->data[port].upStreamOctets, pdata, 8);
                        pdata += sizeof(INT64U);
                        VOS_MemCpy(&data->data[port].downStreamOctets, pdata, 8);
                        pdata += sizeof(INT64U);
                    }
                }
            }
            else
            {
               OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_GetOnuAllPortStatisticData(%d, %d)'s result(%d) on slot %d. data_len %d\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO, dataLen);
               iRlt = VOS_ERROR; 
            }
        }
    }    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_GetOnuAllPortStatisticData(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}
int GWONU_GetAllEthPortMacCounter(short int olt_id, short int onu_id, OnuEthPortCounter_t *data)
{
    int iRlt = OLT_ERR_UNKNOEWN;
    char pBuf[EUQ_MAX_OAM_PDU];
    int len = 0;
    char pRxBuf[EUQ_MAX_OAM_PDU];
    short int Rxlen = 0;
	ULONG portMask = 0;
    int portNum = 0;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemZero(pBuf, EUQ_MAX_OAM_PDU);
    VOS_MemZero(pRxBuf, EUQ_MAX_OAM_PDU);

    pBuf[0] = 0xFF;
    len = 1;
    iRlt = Oam_Session_Send(olt_id, onu_id, GET_ONU_ETH_PORT_MAC_NUM, OAM_SYNC, 0, 0, NULL, pBuf, len, pRxBuf, &Rxlen);
    if(iRlt == VOS_OK)
    {
        DEBUG_OAM_SESSION_PRINTF("GWONU_GetAllEthPortMacCounter: length = %d msgtype = %d result = %d\r\n", Rxlen, pRxBuf[0], pRxBuf[1]);
        if(pRxBuf[1] != 1)
           iRlt = VOS_ERROR; 
        else
        {
            int port_loop = 0;
            if(pRxBuf[2] < 32)
                data->OnuPortNum = pRxBuf[2];
            else
                data->OnuPortNum = 32;
                
            for(port_loop = 0; port_loop<data->OnuPortNum; port_loop++)
            {
                /*第四个字节开始是*/
                VOS_MemCpy(&data->MacNum[port_loop], &pRxBuf[4+5*port_loop], 4);
            }
        }
    }    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_GetAllEthPortMacCounter(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

#endif


#if 1
/* -------------------ONU 远程管理API------------------- */

int GWONU_Execute_CliCommand(short int olt_id, short int onu_id, char *buffer, const int len, char **pRecv, unsigned short int *recvlen)
{
    int iRlt = 0;
    LONG lRlt;
    unsigned short int usRecvLen = 0;
    char *pRecvBuf = NULL;
    char *szSessionName = VOS_Malloc(16, MODULE_ONU);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(buffer);

    if ( NULL != szSessionName )
    {
        VOS_StrCpy(szSessionName, "RPC_CMD");

        lRlt = lCli_SendbyOam_Novty(olt_id, onu_id+1, buffer, len, &pRecvBuf, &usRecvLen, szSessionName);
        if ( VOS_OK == lRlt )
        {
            if ( pRecvBuf != NULL )
            {
                if ( (pRecv != NULL) && (recvlen != NULL) )
                {
                    if ( usRecvLen >= sizeof(cli_recvbuf) )
                    {
                        sys_console_printf("\r\n GWONU(%d/%d) execute cli:%s return(%u) is too long to handle.\r\n", olt_id, onu_id, buffer, usRecvLen);

                        usRecvLen = sizeof(cli_recvbuf) - 1;
                    }
                    
                    VOS_MemCpy(cli_recvbuf, pRecvBuf, usRecvLen);
                    if ( '\0' != cli_recvbuf[usRecvLen] )
                    {
                        cli_recvbuf[usRecvLen++] = '\0';
                    }

                    *pRecv = cli_recvbuf;
                    *recvlen = usRecvLen;
                }
                
                VOS_Free(pRecvBuf);
            }
            else
            {
                if ( pRecv != NULL )
                {
                    *pRecv = NULL;
                }
            }
        }
        else
        {
            iRlt = OLT_ERR_PARAM;
        }
    }
    else
    {
        iRlt = OLT_ERR_MALLOC;
    }

    return iRlt;
}

int GWONU_Resume_CliCommand(short int olt_id, short int onu_id, char *buffer, int len)
{
    int iRlt;
    
    if ( GWONU_Execute_CliCommand(olt_id, onu_id, buffer, len, NULL, NULL) == OLT_ERR_OK )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }
    
    return iRlt;
}


int GWONU_SetEthPortAdminStatus(short int olt_id, short int onu_id, int port_id, int enable)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "port en %d %d ", port_id, enable);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetEthPortAdminStatus(%d, %d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetEthPortPause(short int olt_id, short int onu_id, int port_id, int enable)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "port pause %d %d ", port_id, enable);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetEthPortPause(%d, %d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetEthPortAutoNegotiationRestart(short int olt_id, short int onu_id, int port_id)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "port an restart %d ", port_id);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetEthPortAutoNegotiationRestart(%d, %d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port_id, int num)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "igmpsnooping group-num %d %d ", port_id, num);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetEthPortMulticastGroupMaxNumber(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int tag)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "igmpsnooping tag strip %d %d ", port_id, tag);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetEthPortMulticastTagStrip(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t oper, CTC_STACK_multicast_vlan_switching_t *sw)
{
    int iRlt;
    int len;
    char *pszStr;
    char cli_str[RPC_CMD_BUF_LEN];
    char tag_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( 0 < (len = sw->number_of_entries) )
    {
        int i, n;
        CTC_STACK_multicast_vlan_switching_entry_t *pstEntry;   

        n = len;
        pszStr = tag_str;
        pstEntry = sw->entries;
        for ( i = 0; i < n; i++ )
        {
            pszStr += VOS_Sprintf(pszStr, " iptv-vid %d mc-vid %d", pstEntry->iptv_user_vlan, pstEntry->multicast_vlan);
            pstEntry++;
        }
    }
    else
    {
        return OLT_ERR_PARAM;
    }

    len = VOS_Sprintf(cli_str, "igmpsnooping tag oper %d %s ", port_id, tag_str);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetEthPortMulticastTagOper(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}



int GWONU_CliCall(short int olt_id, short int onu_id, char *str, short int len, char **pRbuf, unsigned short int *plen)
{
    int iRlt = VOS_OK;
    char * pRecv = NULL;
    unsigned short int rlen = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, str, len, &pRecv, &rlen) )
    {
        *pRbuf = pRecv;
        *plen = rlen;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    return iRlt;
}

int GWONU_SetMgtReset(short int olt_id, short int onu_id, int en)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( en )
    {
        len = VOS_Sprintf(cli_str, "mgt reset def ");
    }
    else
    {
        len = VOS_Sprintf(cli_str, "mgt reset ");
    }

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetMgtReset(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetMgtConfig(short int olt_id, short int onu_id, int save)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( save )
    {
        len = VOS_Sprintf(cli_str, "mgt config save ");
    }
    else
    {
        len = VOS_Sprintf(cli_str, "mgt config clear ");
    }

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetMgtConfig(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetMgtLaser(short int olt_id, short int onu_id, int en)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "mgt laser %d ", en);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetMgtLaser(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetTemperature(short int olt_id, short int onu_id, int temp)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "mgt temperature %d ", temp);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetTemperature(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPasFlush(short int olt_id, short int onu_id, int act)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "stat pas_flush ");

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPasFlush(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


int GWONU_SetAtuAgingTime(short int olt_id, short int onu_id, int aging)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "atu aging %d ", aging);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetAtuAgingTime(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetAtuLimit(short int olt_id, short int onu_id, int limit)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "atu limit %d ", limit);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetAtuLimit(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortLinkMon(short int olt_id, short int onu_id, int mon)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "port link_mon %d ", mon);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortLinkMon(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortModeMon(short int olt_id, short int onu_id, int mon)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "port mode_mon %d ", mon);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortModeMon(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortIsolate(short int olt_id, short int onu_id, int enable)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "vlan port_isolate  %d ", enable);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortIsolate(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetVlanEnable(short int olt_id, short int onu_id, int enable)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    /*GW ONU在下发dot1q使能时应该下发一次disable，清掉vlan配置*/
    if(enable)
    {
        len = VOS_Sprintf(cli_str, "vlan dot1q 0 ");        
        if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
        {
            iRlt = VOS_OK;
        }
        else
        {
            iRlt = VOS_ERROR;
        }
    }
    
    len = VOS_Sprintf(cli_str, "vlan dot1q %d ", enable);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetVlanEnable(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetVlanMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    static char *acszVlanModeStr[] = {"transparent", "tag", "translation", "agg", "trunk"};
    int iRlt = VOS_OK;
    
    mode &= (~ONU_SEPCAL_FUNCTION);    
    if ( 1 == port_id )
    {
        int len;
        char cli_str[RPC_CMD_BUF_LEN];

        OLT_LOCAL_ASSERT(olt_id);
        ONU_ASSERT(onu_id);

        if ( (mode < 0) || (mode >= ARRAY_SIZE(acszVlanModeStr)) )
        {
            mode = ARRAY_SIZE(acszVlanModeStr) - 1;
        }

        len = VOS_Sprintf(cli_str, "vlan mode %s ", acszVlanModeStr[mode]);

        iRlt = GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL);

        OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetVlanMode(%d, %d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    }
    else
    {
        OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetVlanMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    }

    return iRlt;
}

int GWONU_AddVlan(short int olt_id, short int onu_id, int vid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "vlan dot1q_add %d ", vid);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_AddVlan(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_DelVlan(short int olt_id, short int onu_id, int vid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "vlan dot1q_del %d ", vid);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_DelVlan(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortPvid(short int olt_id, short int onu_id, int port_id, int pvid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "vlan pvid %d %d ", port_id, pvid);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortPvid(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


int GWONU_AddVlanPort(short int olt_id, short int onu_id, int vid, int portlist, int tag)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];
    char portstr[80];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    portListLongToString(portlist, portstr);
    len = VOS_Sprintf(cli_str, "vlan dot1q_port_add %d %s %d ", vid, portstr, tag);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_AddVlanPort(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_DelVlanPort(short int olt_id, short int onu_id, int vid, int portlist)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];
    char portstr[80];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    portListLongToString(portlist, portstr);
    len = VOS_Sprintf(cli_str, "vlan dot1q_port_del %d %s ", vid, portstr);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_DelVlanPort(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

/*vlan transation*/
int GWONU_SetVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid, ULONG newVid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "vlan-translation %d old-vid %d new-vid %d ", port_id, inVid, newVid);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetVlanTran(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_DelVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "vlan-translation-del %d old-vid %d ", port_id, inVid);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_DelVlanTran(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

/*vlan aggregation*/
int GWONU_SetVlanAgg(short int olt_id, short int onu_id, int port_id, USHORT inVid[8], USHORT targetVid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];
    ULONG aulVlanList[8];
    char vlanstr[80];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    for ( len = 0; len < 8; len++ )
    {
        if ( 0 == (aulVlanList[len] = inVid[len]) )
        {
            break;
        }
    }

    VlanListArrayToString(aulVlanList, len, vlanstr);
    len = VOS_Sprintf(cli_str, "vlan-aggregation %d old-vid %s target-vid %d ", port_id, vlanstr, targetVid);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetVlanAgg(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_DelVlanAgg(short int olt_id, short int onu_id, int port_id, ULONG targetVid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "vlan-aggregation-del %d target-vid %d ", port_id, targetVid);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_DelVlanAgg(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


int GWONU_SetPortQinQEnable(short int olt_id,short int onu_id, int port_id, int enable)
{
    static char *acszQinQ[] = {"disable", "pport", "pvlan"};
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( (enable < 0) || (enable >= ARRAY_SIZE(acszQinQ)) )
    {
        enable = ARRAY_SIZE(acszQinQ) - 1;
    }

    len = VOS_Sprintf(cli_str, "qinq-config %s %d ", acszQinQ[enable], port_id);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortQinQEnable(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_AddQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG cvlan, ULONG svlan)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "qinq-config vlan-tag-add %d c-vid %d s-vid %d ", port_id, cvlan, svlan);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_AddQinQVlanTag(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_DelQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG svlan)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "qinq-config vlan-tag-del %d s-vid %d ", port_id, svlan);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_DelQinQVlanTag(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


int GWONU_SetPortVlanFrameTypeAcc(short int olt_id, short int onu_id, int port_id, int acc)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "vlan acceptable_frame_types %d %s ", port_id, (0 == acc) ? "tagged" : "all");

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortVlanFrameTypeAcc(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortIngressVlanFilter(short int olt_id, short int onu_id, int port_id, int enable)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "vlan ingress_filtering %d %d ", port_id, enable);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortIngressVlanFilter(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


int GWONU_SetPortMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "port mode %d %d ", port_id, mode);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortMode(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortFcMode(short int olt_id, short int onu_id, int port_id, int fc)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "port fc %d %d ", port_id, fc);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortFcMode(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortAtuLearn(short int olt_id, short int onu_id, int portlist, int enable)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];
    char portstr[80];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    portListLongToString(portlist, portstr);
    len = VOS_Sprintf(cli_str, "atu learning %s %d ", portstr, enable);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortAtuLearn(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortAtuFlood(short int olt_id, short int onu_id, int portlist, int enable)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];
    char portstr[80];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    portListLongToString(portlist, portstr);
    len = VOS_Sprintf(cli_str, "atu flood %s %d ", portstr, enable);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortAtuFlood(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortLoopDetect(short int olt_id, short int onu_id, int portlist, int enable)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];
    char portstr[80];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    portListLongToString(portlist, portstr);
    len = VOS_Sprintf(cli_str, "eth-loop port_en %s %d ", portstr, enable);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortLoopDetect(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortStatFlush(short int olt_id, short int onu_id, int portlist, int enable)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];
    char portstr[80];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    portListLongToString(portlist, portstr);
    len = VOS_Sprintf(cli_str, "stat port_flush %s ", portstr);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortStatFlush(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


int GWONU_SetIngressRateLimitBase(short int olt_id, short int onu_id, int uv)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "port ingress_rate_limit_base %d ", uv);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetIngressRateLimitBase(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortIngressRate(short int olt_id, short int onu_id, int port_id, int type, int rate, int action, int burstmode)
{
    static char *acszBurstMode[5] = {"0", "12", "24", "48", "96"};
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( (ONU_CONF_PORT_INGRESS_ACT_NONE == action) || (ONU_CONF_PORT_INGRESS_BURST_NONE == burstmode) )
    {
        len = VOS_Sprintf(cli_str, "port ingress_rate %d %d %d ", port_id, type, rate);
    }
    else
    {
        if ( (burstmode < 0) || (burstmode >= ARRAY_SIZE(acszBurstMode)) )
        {
            burstmode = ARRAY_SIZE(acszBurstMode) - 1;
        }

        len = VOS_Sprintf(cli_str, "port ingress_rate %d %d %d %s %s ", port_id, type, rate
                                    , (ONU_CONF_PORT_INGRESS_ACT_PAUSE == action) ? "pause" : "drop"
                                    , acszBurstMode[burstmode]);
    }

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortIngressRate(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortEgressRate(short int olt_id, short int onu_id, int port_id, int rate)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "port egress_rate %d %d ", port_id, rate);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortEgressRate(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


int GWONU_SetQosClass(short int olt_id, short int onu_id, int qossetid, int ruleid, int classid, int field, int oper, char *val, int val_len)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];
    char vtext[80];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( 0 == (iRlt = onuconf_qosRuleFieldValue_ValToStr_parase((unsigned char)field, (qos_value_t*)val, vtext)) )
    {
        len = VOS_Sprintf(cli_str, "class-match %d %d %d select %s %s operator %s", qossetid, ruleid, classid, qos_rule_field_sel_str(field+1), vtext, qos_rule_field_operator_str(oper+1));

        if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
        {
            iRlt = VOS_OK;
        }
        else
        {
            iRlt = VOS_ERROR;
        }
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetQosClass(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GWONU_ClrQosClass(short int olt_id, short int onu_id, int qossetid, int ruleid, int classid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "undo class-match %d %d %d ", qossetid, ruleid, classid);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_ClrQosClass(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GWONU_SetQosRule(short int olt_id, short int onu_id, int qossetid, int ruleid, int queue, int prio)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "policy-map %d %d queue-map %d priority-mark %d ", qossetid, ruleid, queue, prio);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetQosRule(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GWONU_ClrQosRule(short int olt_id, short int onu_id, int qossetid, int ruleid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "undo policy-map %d %d ", qossetid, ruleid);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_ClrQosRule(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GWONU_SET_QosMode(short int olt_id, short int onu_id, int direct, unsigned char mode)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "qos vlan priority_mode %s %s\r\n", direct==QOS_RULE_UP_DIRECTION?"up":"down", 
        mode==QOS_MODE_PRIO_TRANS?"priority_translation":"priority_vid");

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SET_QosMode(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    return iRlt;
}
int GWONU_SET_Rule(short int olt_id, short int onu_id, int direct, int code, gw_rule_t rule)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if(code == SET_QOS_RULE || code == CLR_QOS_RULE)
    {       
        if(rule.qos_rule.mode == BASE_ON_ETHER_TYPE)
        {
            len = VOS_Sprintf(cli_str, "qos classifier ether-type 0x%04x %s %d %d %d\r\n",                 
                rule.qos_rule.value, direct==QOS_RULE_UP_DIRECTION?"up":"down", 
                rule.qos_rule.priority_mark, rule.qos_rule.queue_mapped, code == SET_QOS_RULE?1:0);
        }
        else
        {
            len = VOS_Sprintf(cli_str, "qos classifier %s %d %s %d %d %d\r\n", 
               rule.qos_rule.mode == BASE_ON_IP_PROTOCAL?"ip-protocol":"vlan-id", 
                 rule.qos_rule.value, direct==QOS_RULE_UP_DIRECTION?"up":"down", 
                 rule.qos_rule.priority_mark, rule.qos_rule.queue_mapped, code == SET_QOS_RULE?1:0);
        }
    }
    else if(code == SET_PAS_RULE || code == CLR_PAS_RULE)
    {
        if(rule.pas_rule.mode == BASE_ON_ETHER_TYPE)
        {
            len = VOS_Sprintf(cli_str, "vlan pas-rule ether-type 0x%04x %s %s %d %04x %s %d\r\n",                 
                rule.pas_rule.value, direct==QOS_RULE_UP_DIRECTION?"up":"down", 
                rule.pas_rule.action?(PAS_RULE_ACTION_ATTACH==rule.pas_rule.action?"attach":"exchange"):"none", 
                rule.pas_rule.new_vid, rule.pas_rule.vlan_type, rule.pas_rule.prio_source == PAS_RULE_PRIO_SOURCE_CLASSIFIER?"classifier":"original", 
                code == SET_PAS_RULE?1:0);
        }
        else
        {
            len = VOS_Sprintf(cli_str, "vlan pas-rule %s %d %s %s %d %04x %s %d\r\n", 
                rule.pas_rule.mode == BASE_ON_IP_PROTOCAL?"ip-protocol":"vlan-id", 
                rule.pas_rule.value, direct==QOS_RULE_UP_DIRECTION?"up":"down", 
                rule.pas_rule.action?(PAS_RULE_ACTION_ATTACH==rule.pas_rule.action?"attach":"exchange"):"none", 
                rule.pas_rule.new_vid, rule.pas_rule.vlan_type, rule.pas_rule.prio_source == PAS_RULE_PRIO_SOURCE_CLASSIFIER?"classifier":"original", 
                code == SET_PAS_RULE?1:0);
        }

        
    }
    
    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SET_Rule(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    return iRlt;
}

int GWONU_SetPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "service-policy port %d policy %d ", port_id, qossetid);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortQosRule(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GWONU_ClrPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "undo service-policy port %d ", port_id);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_ClrPortQosRule(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GWONU_SetPortQosRuleType(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "qos rule %d %s ", port_id, (0 == mode) ? "ip" : "user");

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortQosRuleType(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


int GWONU_SetPortDefPriority(short int olt_id, short int onu_id, int port_id, int prio)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "qos def_pri %d %d ", port_id, prio);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortDefPriority(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortNewPriority(short int olt_id, short int onu_id, int port_id, int oldprio, int newprio)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "qos user_pri_reg %d %d %d ", port_id, oldprio, newprio);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortNewPriority(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


int GWONU_SetQosPrioToQueue(short int olt_id, short int onu_id, int prio, int queue)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "qos user_pri_tc %d %d ", prio, queue);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetQosPrioToQueue(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetQosDscpToQueue(short int olt_id, short int onu_id, int dscpnum, int queue)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "qos dscp_tc %d %d ", dscpnum, queue);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetQosDscpToQueue(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortUserPriorityEnable(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "qos user_pri_en %d %d ", port_id, mode);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortUserPriorityEnable(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortIpPriorityEnable(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "qos ip_pri_en %d %d ", port_id, mode);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortIpPriorityEnable(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetQosAlgorithm(short int olt_id, short int onu_id, int uv)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "qos algorithm %s ", (0 == uv) ? "wrr" : "spq");

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetQosAlgorithm(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


int GWONU_SetIgmpEnable(short int olt_id, short int onu_id, int enable)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "igmpsnooping enable %d ", enable);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetIgmpEnable(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetIgmpAuth(short int olt_id, short int onu_id, int en)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "igmpsnooping auth enable %d ", en);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetIgmpAuth(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetIgmpHostAge(short int olt_id, short int onu_id, int age)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "igmpsnooping host_aging_time %d ", age);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetIgmpHostAge(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetIgmpGroupAge(short int olt_id, short int onu_id, int age)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "igmpsnooping group_aging_time %d ", age);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetIgmpGroupAge(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetIgmpMaxResTime(short int olt_id, short int onu_id, int tm)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "igmpsnooping max_response_time %d ", tm);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetIgmpMaxResTime(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetIgmpMaxGroup(short int olt_id, short int onu_id, int number)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "igmpsnooping max_group %d ", number);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetIgmpMaxGroup(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_AddIgmpGroup(short int olt_id, short int onu_id, int portlist, ULONG addr,ULONG vid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];
    char ip_str[32];
    char portstr[80];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    portListLongToString(portlist, portstr);
    get_ipdotstring_from_long(ip_str, addr);
    len = VOS_Sprintf(cli_str, "igmpsnooping gda_add %s %d %s ", ip_str, vid, portstr);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_AddIgmpGroup(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_DeleteIgmpGroup(short int olt_id, short int onu_id, ULONG addr)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];
    char ip_str[32];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    get_ipdotstring_from_long(ip_str, addr);
    len = VOS_Sprintf(cli_str, "igmpsnooping gda_del %s ", ip_str);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_DeleteIgmpGroup(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortIgmpFastLeave(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "igmpsnooping fast_leave %d %d ", port_id, mode);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortIgmpFastLeave(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortMulticastVlan(short int olt_id, short int onu_id, int port_id, int vid)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    len = VOS_Sprintf(cli_str, "multicast vlan %d %d ", port_id, vid);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortMulticastVlan(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


static char *s_acszMirrorDir[4] = {"x", "i", "e", "a"};

int GWONU_SetPortMirrorFrom(short int olt_id, short int onu_id, int port_id, int mode, int type)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( type >= ARRAY_SIZE(s_acszMirrorDir) )
    {
        type = ARRAY_SIZE(s_acszMirrorDir) - 1;
    }

    len = VOS_Sprintf(cli_str, "port mirror_from %s %d %d ", s_acszMirrorDir[type], port_id, mode);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortMirrorFrom(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_SetPortMirrorTo(short int olt_id, short int onu_id, int port_id, int type)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( type >= ARRAY_SIZE(s_acszMirrorDir) )
    {
        type = ARRAY_SIZE(s_acszMirrorDir) - 1;
    }

    len = VOS_Sprintf(cli_str, "port mirror_to %s %d ", s_acszMirrorDir[type], port_id);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_SetPortMirrorTo(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

int GWONU_DeleteMirror(short int olt_id, short int onu_id, int type)
{
    int iRlt;
    int len;
    char cli_str[RPC_CMD_BUF_LEN];

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( type >= ARRAY_SIZE(s_acszMirrorDir) )
    {
        type = ARRAY_SIZE(s_acszMirrorDir) - 1;
    }

    len = VOS_Sprintf(cli_str, "port mirror_to %s 0 ", s_acszMirrorDir[type]);

    if ( OLT_ERR_OK == GWONU_Execute_CliCommand(olt_id, onu_id, cli_str, len, NULL, NULL) )
    {
        iRlt = VOS_OK;
    }
    else
    {
        iRlt = VOS_ERROR;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GWONU_DeleteMirror(%d, %d)[%s]'s result(%d) on slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


#endif

#endif



#if 1
/* -------------------ONU API接口并且封装共有实现------------------- */
#define ONUMGT_LOCAL_CALL(oltid, onuid, fun, params)      (( (NULL != OnuMgmtTable[(oltid)*MAXONUPERPON + (onuid)].OnuIFs) && (NULL != OnuMgmtTable[(oltid)*MAXONUPERPON + (onuid)].OnuIFs->fun) ) ? (*OnuMgmtTable[(oltid)*MAXONUPERPON + (onuid)].OnuIFs->fun) params : OLT_ERR_NOTSUPPORT)
#define ONUMGT_REMOTE_CALL(oltid, onuid, fun, params)     (( (NULL != s_pRemoteOnuIfs) && (NULL != s_pRemoteOnuIfs->fun) ) ? ((0 == (iRlt = (*s_pRemoteOnuIfs->fun) params)) ? OLT_ERR_REMOTE_OK : iRlt) : OLT_ERR_NOTSUPPORT)
#define ONUMGT_API_CALL(oltid, onuid, fun, params)        ( OLT_ISLOCAL(oltid) ? ONUMGT_LOCAL_CALL(oltid, onuid, fun, params) : ONUMGT_REMOTE_CALL(oltid, onuid, fun, params) )

#define ONUMGT_REMOTE_CALL2(oltid, onuid, fun, params)    (( (NULL != s_pRemoteOnuIfs) && (NULL != s_pRemoteOnuIfs->fun) ) ? (*s_pRemoteOnuIfs->fun) params : OLT_ERR_NOTSUPPORT)
#define ONUMGT_API_CALL2(oltid, onuid, fun, params)        ( OLT_ISLOCAL(oltid) ? ONUMGT_LOCAL_CALL(oltid, onuid, fun, params) : ONUMGT_REMOTE_CALL2(oltid, onuid, fun, params) )

/** added by wangjiah@2017-04-27 to support configuration copy between pon-protect pair
 * _PARTNER_CALL do as follows:
 * -----------------------------------------------
 *  ACTIVE          master              PASSIVE		
 *                  REMOTE_CALL
 *                  (do something)  
 *			  <-----			
 *	LOCAL_CALL
 *	(do something)
 *					REMOTE_CALL
 *					(do something)
 *								----->
 *									   LOCAL_CALL
 *									   (do something)
 */

#define ONUMGT_PARTNER_CALL_BEGIN		if((SYS_LOCAL_MODULE_ISMASTERACTIVE \
			&& (VOS_OK == (PonPortSwapPortQuery(olt_id, &olt_id_swap))) \
			&& (V2R1_PON_PORT_SWAP_ACTIVE == (PonPortHotStatusQuery(olt_id))))) {	

#define ONUMGT_PARTNER_CALL_END }


#if 0
#define ONUCONF_LOCAL_CALL(oltid, onuid, fun, params)     (((NULL != s_pLocalOnuConfMgtIfs)  && (NULL != s_pLocalOnuConfMgtIfs->fun))  ? (*s_pLocalOnuConfMgtIfs->fun) params  : OLT_ERR_NOTSUPPORT)
#define ONUCONF_A_MASTER_CALL(oltid, onuid, fun, params)    (((NULL != s_pRemoteOnuConfMgtIfs) && (NULL != s_pRemoteOnuConfMgtIfs->fun)) ? (*s_pRemoteOnuConfMgtIfs->fun) params : OLT_ERR_NOTSUPPORT)
#define ONUCONF_S_MASTER_CALL(oltid, onuid, fun, params)    (OLT_ERR_OK)

#define ONUCONF_REMOTE_CALL(oltid, onuid, fun, params)     (SYS_LOCAL_MODULE_ISMASTERACTIVE?ONUCONF_A_MASTER_CALL(oltid, onuid, fun, params) : ONUCONF_S_MASTER_CALL(oltid, onuid, fun, params))

#define ONUCONF_API_CALL(oltid, onuid, fun, params)       ( ((SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER&&(GetCardIdxByPonChip(oltid) == SYS_LOCAL_MODULE_SLOTNO))|| (SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER && SYS_LOCAL_MODULE_ISMASTERACTIVE)) ? ONUCONF_LOCAL_CALL(oltid, onuid, fun, params) : ONUCONF_REMOTE_CALL(oltid, onuid, fun, params) )
#endif

#if 1
/* -------------------ONU基本API------------------- */

int OnuMgt_OnuIsValid(short int olt_id, short int onu_id, int *status)
{
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(status);
	
    return ONUMGT_API_CALL2( olt_id, onu_id, OnuIsValid, (olt_id, onu_id, status) ); 
}

int OnuMgt_OnuIsOnline(short int olt_id, short int onu_id, int *status)
{
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(status);
	
    return ONUMGT_API_CALL2( olt_id, onu_id, OnuIsOnline, (olt_id, onu_id, status) ); 
}

/* 手动添加ONU */
int OnuMgt_AddOnuByManual( short int olt_id, short int onu_id, unsigned char *MacAddr )
{
	int iRlt;
	int slot;
	int port;
	short int olt_id_swap = 0;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	VOS_ASSERT(MacAddr);

	if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, AddOnuByManual, (olt_id, onu_id, MacAddr) )) )
	{
		int iIsNewOnu;
		int onuMgtIdx = olt_id * MAXONUPERPON + onu_id;
		short int OtherOnuIdx = RERROR;
		short int llid = 0;
		OtherOnuIdx = GetOnuIdxByMacPerPon( olt_id, MacAddr );                        
		if ( ROK != ThisIsValidOnu(olt_id, onu_id) )
		{
			iIsNewOnu = 1;
		}
		else
		{
			/*if (0 == VOS_MemCmp(OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr, MacAddr, BYTES_IN_MAC_ADDRESS))*/
			if( MAC_ADDR_IS_EQUAL(OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr, MacAddr) )
			{
				return 4;
			}
			iIsNewOnu = 0;
		}

		/* B--added by liwei056@2010-12-9 for D11577 */
		if ( iIsNewOnu )
			/* E--added by liwei056@2010-12-9 for D11577 */
		{  
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[onuMgtIdx].OperStatus = ONU_OPER_STATUS_DOWN;
			VOS_MemCpy(OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr, MacAddr, BYTES_IN_MAC_ADDRESS );
			OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus = LLID_ENTRY_NOT_IN_ACTIVE;
			/* add by chenfj 2006-10-30 */
			GetOnuDefaultBWByPonRate(olt_id, onu_id, OLT_CFG_DIR_UPLINK, &OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_gr, &OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_be);
			GetOnuDefaultBWByPonRate(olt_id, onu_id, OLT_CFG_DIR_DOWNLINK, &OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkBandwidth_gr, &OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkBandwidth_be);
#ifdef PLATO_DBA_V3
			OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_fixed = 0;
#endif
			/*added by chenfj 2006-12-21 
			  设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参
			  */
			OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkClass = OnuConfigDefault.UplinkClass;
			OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkDelay = OnuConfigDefault.UplinkDelay;
			OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkClass = OnuConfigDefault.DownlinkClass;
			OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkDelay = OnuConfigDefault.DownlinkDelay;
			OnuMgmtTable[onuMgtIdx].LlidTable[0].MaxMAC = MaxMACDefault;
			OnuEvent_UpDate_RunningUpBandWidth(olt_id, onu_id);
			OnuEvent_UpDate_RunningDownBandWidth(olt_id, onu_id);

			OnuMgmtTable[onuMgtIdx].RegisterFlag = NOT_ONU_FIRST_REGISTER_FLAG;
			/*moved by luh 2012-9-29*/
			/*RecordOnuUpTime( onuMgtIdx );*/
			OnuMgmtTable[onuMgtIdx].DeviceInfo.SysUptime = time(0);
			/* add by chenfj 2006/12/13 */
			/* 3mib port status */
			/*{
			  slot = GetCardIdxByPonChip(olt_id);
			  port = GetPonPortByPonChip(olt_id);		
			  setOnuStatus( slot,  port, onu_id, ONU_ONLINE );
			  }*/	/* removed by xieshl 20120605 */
			ONU_MGMT_SEM_GIVE;
		}
		else
		{
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[onuMgtIdx].OperStatus = ONU_OPER_STATUS_DOWN;                
			VOS_MemCpy(OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr, MacAddr, BYTES_IN_MAC_ADDRESS );
			ONU_MGMT_SEM_GIVE;
		}
		if( OtherOnuIdx != RERROR ) 
		{
			if(OtherOnuIdx != onu_id) 
			{
				short int slotno = GetCardIdxByPonChip(olt_id);
				if(SYS_MODULE_SLOT_ISHAVECPU(slotno) && slotno != SYS_LOCAL_MODULE_SLOTNO)
				{
				}
				else
				{
					if(GetOnuOperStatus(olt_id, OtherOnuIdx) == ONU_OPER_STATUS_UP)
					{
						/*需要调用统一接口进行去注册，Q.25608*/
						OnuMgt_DeregisterOnu(olt_id, OtherOnuIdx);
					}
				}
				localDelOnuFromPon(olt_id, OtherOnuIdx);                
			}
			else
			{
			}
		}           

		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_AddOnuByManual(olt_id_swap, onu_id, MacAddr);
		if(OLT_CALL_ISERROR(iRlt))
		{
			VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_AddOnuByManual(%d, %d) Error Rlt(%d).", olt_id_swap, onu_id, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
	}
	else
	{
		if ( OLT_CALL_ISERROR(iRlt) )
		{
			VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_AddOnuByManual(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
		}
		else
		{
			iRlt = OLT_ERR_OK;
		}
	}

	return iRlt;
}
int OnuMgt_ModifyOnuByManual( short int olt_id, short int onu_id, unsigned char *MacAddr )
{
	int iRlt;
	int slot;
	int port;
	short int olt_id_swap = 0;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	VOS_ASSERT(MacAddr);

	if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, ModifyOnuByManual, (olt_id, onu_id, MacAddr) )) )
	{
		int onuMgtIdx = olt_id * MAXONUPERPON + onu_id;

		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[onuMgtIdx].OperStatus = ONU_OPER_STATUS_DOWN;
		VOS_MemCpy(OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr, MacAddr, BYTES_IN_MAC_ADDRESS );
		ONU_MGMT_SEM_GIVE;

		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_ModifyOnuByManual(olt_id_swap, onu_id, MacAddr);
		if(OLT_CALL_ISERROR(iRlt))
		{
			VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_ModifyOnuByManual(%d, %d) Error Rlt(%d).", olt_id_swap, onu_id, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
	}
	else
	{
		if ( OLT_CALL_ISERROR(iRlt) )
		{
			VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_ModifyOnuByManual(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
		}
		else
		{
			iRlt = OLT_ERR_OK;
		}
	}

	return iRlt;
}



/* 手动删除ONU */
int OnuMgt_DelOnuByManual(short int olt_id, short int onu_id)
{
	int iRlt;
	short int olt_id_swap = 0;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	/* 问题单11882 */	
	if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, DelOnuByManual, (olt_id, onu_id) )) )
	{
#if 0	/* modified by xieshl 20110510, 解决主控上有但PON板上没有的ONU无法删除问题 */
		int onuMgtIdx = olt_id * MAXONUPERPON + onu_id;
		int slot = GetCardIdxByPonChip(olt_id);
		int port = GetPonPortByPonChip(olt_id);
		/*UCHAR macAddress[8];*/


		ONU_MGMT_SEM_TAKE;

		if( SYS_LOCAL_MODULE_ISMASTERACTIVE/*SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER*/ )
		{
			/*VOS_MemCpy( macAddress, OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS );
			  onuDeletingNotify_EventReport( OLT_DEV_ID, slot, port, (onu_id+1), macAddress );*/	/* added by xieshl 20110428 */
			onuDeletingNotify_EventReport( OLT_DEV_ID, slot, port, (onu_id+1), OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr );
		}

		if ( (ONU_OPER_STATUS_UP == OnuMgmtTable[onuMgtIdx].OperStatus)
				|| (ONU_OPER_STATUS_PENDING == OnuMgmtTable[onuMgtIdx].OperStatus) )
		{
			/* 在线 ONU等待离线事件的处理*/
			OnuMgmtTable[onuMgtIdx].NeedDeleteFlag = TRUE;	
		}
		else
		{
			/* 不在线 ONU，直接清空其配置对象*/
			InitOneOnuByDefault(olt_id, onu_id);
			setOnuStatus( slot,  port, onu_id, ONU_OFFLINE );
		}
		ONU_MGMT_SEM_GIVE;
#else
		localDelOnuFromPon(olt_id, onu_id);
		/*commented by wangxiaoyu 20111117, 删除ONU时不自动删除ONU ID关联关系
		  onuconfUndoAssociateOnuId(olt_id, onu_id);*/
#endif
		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_DelOnuByManual(olt_id_swap, onu_id);
		if(OLT_CALL_ISERROR(iRlt) && OLT_ERR_NOTEXIST != iRlt)
		{
			VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_DelOnuByManual(%d, %d) Error Rlt(%d).", olt_id_swap, onu_id, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END

	}
	else	
	{
		if ( OLT_CALL_ISERROR(iRlt) )
		{
			if ( OLT_ERR_NOTEXIST != iRlt )
			{
				VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DelOnuByManual(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
			}
		}
		else
		{
			iRlt = OLT_ERR_OK;
		}
	}

	return iRlt;
}

int OnuMgt_AddGponOnuByManual( short int olt_id, short int onu_id, unsigned char *sn )
{
    int iRlt;
    int slot;
    int port;
	UCHAR icount;
	UCHAR SerialNoAfterSIXDigit[BYTES_IN_MAC_ADDRESS];
	UCHAR series_tmp[BYTES_IN_MAC_ADDRESS];
	char equipmentID[MAXEQUIPMENTIDLEN];
	char pModel[80]="";
	ULONG pLen=0;
	ULONG typeid=0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sn);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, AddGponOnuByManual, (olt_id, onu_id, sn) )) )
    {
        int iIsNewOnu;
        int onuMgtIdx = olt_id * MAXONUPERPON + onu_id;
        short int OtherOnuIdx = RERROR;
        short int llid = 0;
    	OtherOnuIdx = GetOnuIdxBySnPerPon( olt_id, sn );                        
        if ( ROK != ThisIsValidOnu(olt_id, onu_id) )
        {
            iIsNewOnu = 1;
        }
        else
        {
            /*if (0 == VOS_MemCmp(OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr, MacAddr, BYTES_IN_MAC_ADDRESS))*/
#if 0            
            if( MAC_ADDR_IS_EQUAL(OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr, MacAddr) )
            {
                iIsNewOnu = 0;
            }
            else
            {
                /* 删除旧的ONU */
        		InitOneOnuByDefault(olt_id, onu_id);

                /* add by chenfj 2006/12/13 */
                /* 3mib port status */
            	/*{

            		slot = GetCardIdxByPonChip(olt_id);
                    ONU_MGMT_SEM_TAKE;
            		setOnuStatus( slot,  port, onu_id, ONU_OFFLINE );
                    ONU_MGMT_SEM_GIVE;
            	}*/	/* removed by xieshl 20120605 */

                /* 添加新的ONU */
                iIsNewOnu = 1;
            }
#else
            if( VOS_StrCmp(OnuMgmtTable[onuMgtIdx].DeviceInfo.DeviceSerial_No, sn) == 0)
            {
                return 4;
            }
            iIsNewOnu = 0;
#endif
        }
        
        /* B--added by liwei056@2010-12-9 for D11577 */
        if ( iIsNewOnu )
        /* E--added by liwei056@2010-12-9 for D11577 */
        {  
            ONU_MGMT_SEM_TAKE;
            OnuMgmtTable[onuMgtIdx].OperStatus = ONU_OPER_STATUS_DOWN;
            OnuMgmtTable[onuMgtIdx].IsGponOnu = 1;
        	VOS_MemCpy(OnuMgmtTable[onuMgtIdx].DeviceInfo.DeviceSerial_No, sn, GPON_ONU_SERIAL_NUM_STR_LEN-1);
			/*B--mod by liub, 2017-5-15 。其他厂商mac按sn号后六位填0的规则设置，私有GW按文档要求*/
			GPON_OnuModel_Translate(equipmentID, &typeid, pModel, &pLen);
			if( typeid == V2R1_ONU_GPON ) /*其他厂商*/
			{
				for(icount = 0;icount < 6;icount += 1)
		  		{
					sn[GPON_ONU_SERIAL_NUM_STR_LEN-1]=0;
					if(sn[10+icount] >= 'A' && sn[10+icount] <= 'F')
						SerialNoAfterSIXDigit[icount] = sn[10+icount]-'A' + 10;
					else if(sn[10+icount] >= 'a' && sn[10+icount] <= 'f')
						SerialNoAfterSIXDigit[icount] = sn[10+icount]-'a' + 10;
					else
						SerialNoAfterSIXDigit[icount] = sn[10+icount] - '0';
		    	}
			}
	        else
			{
				SerialNoAfterSIXDigit[0]=0x00;
				SerialNoAfterSIXDigit[1]=0x0F;
				SerialNoAfterSIXDigit[2]=0xE9;
				for(icount = 0;icount < 6;icount += 1)
		  		{
		  			if(sn[10+icount] >= 'A' && sn[10+icount] <= 'F')
						series_tmp[icount] = sn[10+icount]-'A' + 10;
					else if(sn[10+icount] >= 'a' && sn[10+icount] <= 'f')
						series_tmp[icount] = sn[10+icount]-'a' + 10;
					else
						series_tmp[icount] = sn[10+icount] - '0';
		    	}
				
				SerialNoAfterSIXDigit[3] = ((series_tmp[0]) << 4) | series_tmp[1];	
				SerialNoAfterSIXDigit[4] = ((series_tmp[2]) << 4) | series_tmp[3];
				SerialNoAfterSIXDigit[5] = ((series_tmp[4]) << 4) | series_tmp[5];
			}
			/*E--mod by liub, 2017-5-15*/
			VOS_MemCpy(OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr, SerialNoAfterSIXDigit, BYTES_IN_MAC_ADDRESS);
			OnuMgmtTable[onuMgtIdx].DeviceInfo.DeviceSerial_NoLen = GPON_ONU_SERIAL_NUM_STR_LEN;
        	OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus = LLID_ENTRY_NOT_IN_ACTIVE;
        	/* add by chenfj 2006-10-30 */

			GetOnuDefaultBWByPonRate(olt_id, onu_id, OLT_CFG_DIR_UPLINK, &OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_gr, &OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_be);
			GetOnuDefaultBWByPonRate(olt_id, onu_id, OLT_CFG_DIR_DOWNLINK, &OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkBandwidth_gr, &OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkBandwidth_be);
#ifdef PLATO_DBA_V3
        	OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_fixed = 0;
#endif
        	/*added by chenfj 2006-12-21 
        	    设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参
        	*/
        	OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkClass = OnuConfigDefault.UplinkClass;
        	OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkDelay = OnuConfigDefault.UplinkDelay;
        	OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkClass = OnuConfigDefault.DownlinkClass;
        	OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkDelay = OnuConfigDefault.DownlinkDelay;
        	OnuMgmtTable[onuMgtIdx].LlidTable[0].MaxMAC = MaxMACDefault;
            OnuEvent_UpDate_RunningUpBandWidth(olt_id, onu_id);
            OnuEvent_UpDate_RunningDownBandWidth(olt_id, onu_id);

        	OnuMgmtTable[onuMgtIdx].RegisterFlag = NOT_ONU_FIRST_REGISTER_FLAG;
            /*moved by luh 2012-9-29*/
        	/*RecordOnuUpTime( onuMgtIdx );*/
            OnuMgmtTable[onuMgtIdx].DeviceInfo.SysUptime = time(0);
        	/* add by chenfj 2006/12/13 */
        	/* 3mib port status */
        	/*{
        		slot = GetCardIdxByPonChip(olt_id);
        		port = GetPonPortByPonChip(olt_id);		
        		setOnuStatus( slot,  port, onu_id, ONU_ONLINE );
        	}*/	/* removed by xieshl 20120605 */
            ONU_MGMT_SEM_GIVE;
        }
        else
        {
            ONU_MGMT_SEM_TAKE;
            OnuMgmtTable[onuMgtIdx].OperStatus = ONU_OPER_STATUS_DOWN;                
        	VOS_MemCpy(OnuMgmtTable[onuMgtIdx].DeviceInfo.DeviceSerial_No, sn, GPON_ONU_SERIAL_NUM_STR_LEN-1);
        	OnuMgmtTable[onuMgtIdx].DeviceInfo.DeviceSerial_NoLen = GPON_ONU_SERIAL_NUM_STR_LEN;        	
            ONU_MGMT_SEM_GIVE;
        }
    	if( OtherOnuIdx != RERROR ) 
    	{
    		if(OtherOnuIdx != onu_id) 
    		{
                short int slotno = GetCardIdxByPonChip(olt_id);
                if(SYS_MODULE_SLOT_ISHAVECPU(slotno) && slotno != SYS_LOCAL_MODULE_SLOTNO)
                {
                }
                else
                {
                    if(GetOnuOperStatus(olt_id, OtherOnuIdx) == ONU_OPER_STATUS_UP)
                    {
#if 0                    
            	        if( INVALID_LLID != (llid = GetLlidByOnuIdx(olt_id, OtherOnuIdx)) )
            	        {
            	            PAS_deregister_onu( olt_id, llid, FALSE );
            	        }
#else
                        /*需要调用统一接口进行去注册，Q.25608*/
                        OnuMgt_DeregisterOnu(olt_id, OtherOnuIdx);
#endif
                    }
                }
                localDelOnuFromPon(olt_id, OtherOnuIdx);                
    		}
            else
            {
            }
    	}           
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_AddOnuByManual(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	
    return iRlt;
}

int OnuMgt_CmdIsSupported(short int olt_id, short int onu_id, short int *cmd)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmd);
    
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, CmdIsSupported, (olt_id, onu_id, cmd) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTSUPPORT != iRlt )
    	{
            VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_CmdIsSupported(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, *cmd, iRlt);
        }
    }

    return iRlt;
}

int OnuMgt_CopyOnu(short int olt_id, short int onu_id, short int dst_olt_id, short int dst_onu_id, int copy_flags)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    OLT_ASSERT(dst_olt_id);
    ONU_ASSERT(onu_id);
    ONU_ASSERT(dst_onu_id);

    dst_olt_id = GetBasePonPortIdx(olt_id, dst_olt_id);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, CopyOnu, (olt_id, onu_id, dst_olt_id, dst_onu_id, copy_flags) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_CopyOnu(%d, %d, %d, %d, %d) Error Rlt(%d).", olt_id, onu_id, dst_olt_id, dst_onu_id, copy_flags, iRlt);
    }
    
    return iRlt;
}

int OnuMgt_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetIFType, (olt_id, onu_id, chip_type, remote_type) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetIFType(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"OnuMgt_GetIFType(%d, %d, %d, %d)'s result(%d).\r\n", olt_id, onu_id, *chip_type, *remote_type, iRlt );
    
    return iRlt;
}

int OnuMgt_SetIFType(short int olt_id, short int onu_id, int chip_type, int remote_type)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetIFType, (olt_id, onu_id, chip_type, remote_type) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetIFType(%d, %d, %d, %d) Error Rlt(%d).", olt_id, onu_id, chip_type, remote_type, iRlt);
    }
    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"OnuMgt_SetIFType(%d, %d, %d, %d)'s result(%d).\r\n", olt_id, onu_id, chip_type, remote_type, iRlt );
    
    return iRlt;
}
#endif


#if 1
/* -------------------ONU 认证管理API------------------- */

/*onu deregister*//*add by shixh20100823*/
int OnuMgt_DeregisterOnu(short int olt_id, short int onu_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DeregisterOnu, (olt_id, onu_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DeregisterOnu(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
     
    return iRlt;
}

int OnuMgt_SetMacAuthMode(short int olt_id, short int onu_id, int auth_mode, mac_address_t auth_mac)
{
    int iRlt;
	short int olt_id_swap = 0;
        
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(auth_mac);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetMacAuthMode, (olt_id, onu_id, auth_mode, auth_mac) )) )
    {
        ulong_t Slot, Port, Uid;
        LogicEntity *le;

        Slot = GetCardIdxByPonChip(olt_id);
        if( Slot == RERROR ) return( OLT_ERR_PARAM);
        Port = GetPonPortByPonChip(olt_id);
        if( Port == RERROR ) return( OLT_ERR_PARAM );

        Uid = onu_id + 1;

		VOS_SemTake( gLibEponMibSemId, WAIT_FOREVER );
        if ( NULL != (le = getLogicEntityBySlot(Slot, Port, Uid)) )
        {
            if (V2R1_ENTRY_CREATE_AND_GO == auth_mode)
            {               
                if ( le->onu_auth_status == 0 )
                {
                    le->onu_auth_status = 1;
                }
                
                VOS_MemCpy(le->onu_auth_mac, auth_mac, 6);
            }
            else if (V2R1_ENTRY_DESTORY == auth_mode)
            {
                if ( le->onu_auth_status == 1 )
                {
                    le->onu_auth_status = 0;
                }
                
                VOS_MemSet( le->onu_auth_mac, 0, 6 );
            }
        }
        else
        {
            iRlt = OLT_ERR_PARAM;
        }

		VOS_SemGive( gLibEponMibSemId );


		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetMacAuthMode(olt_id_swap, onu_id, auth_mode, auth_mac);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetMacAuthMode(%d, %d, %d) Error Rlt(%d).", olt_id_swap, onu_id, auth_mode, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetMacAuthMode(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, auth_mode, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

/*(set)del binding onu */
int OnuMgt_DelBindingOnu(short int olt_id, short int onu_id, PON_onu_deregistration_code_t deregistration_code, int code_param)
{
    int iRlt = 0;
#ifdef __pending_onu_list__	

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, DelBindingOnu, (olt_id, onu_id, deregistration_code, code_param) )) )
    {
        if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
        {
            short int Onuentry;
            short int Llid;
            
            Llid = GetLlidByOnuIdx(olt_id, onu_id);
            if (Llid != INVALID_LLID )
            {
                pendingOnu_S *CurOnu, *PreOnu;	
                unsigned char *pbOnuMac;
                int Entry;

                Entry = 0;
                Onuentry = olt_id*MAXONUPERPON + onu_id;
                pbOnuMac = OnuMgmtTable[Onuentry].DeviceInfo.MacAddr;
                VOS_ASSERT(pbOnuMac);
                
                VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
                CurOnu = PonPortTable[olt_id].PendingOnu_Conf.Next;
                PreOnu = NULL;

                while( CurOnu != NULL )
                {
                    if( MAC_ADDR_IS_EQUAL(CurOnu->OnuMarkInfor.OnuMark.MacAddr, pbOnuMac) )
                    {
                        CurOnu->Llid = Llid;
                        CurOnu->otherPonIdx = (short int)code_param;
                        CurOnu->counter = 0;

                        Entry = 1;
                        break;  
                    }
                    
                    PreOnu = CurOnu;
                    CurOnu = CurOnu->Next;
                }

                if ( 0 == Entry )
                {
                    Entry = FindFreeEntryForConfOnu();
                    if (Entry == RERROR )
                    {
                        sys_console_printf("\r\n No Free Entry to save conflict Onu \r\n");
                        VOS_SemGive( OnuPendingDataSemId );
                        return ( RERROR );
                    }

                    CurOnu = &(ConfOnuPendingQueue[Entry]);
                    CurOnu->otherPonIdx = (short int)code_param;
                    CurOnu->Llid = Llid; 
                    VOS_MemCpy( CurOnu->OnuMarkInfor.OnuMark.MacAddr, pbOnuMac, BYTES_IN_MAC_ADDRESS );
                    CurOnu->counter = 0;
                    CurOnu->Next = NULL;

                    if( PreOnu != NULL )
                        PreOnu->Next = CurOnu;
                    else
                        PonPortTable[olt_id].PendingOnu_Conf.Next = CurOnu;
                }
                VOS_SemGive( OnuPendingDataSemId );
            }   
        }
        
        ClearOnuRunningData(olt_id, onu_id, 0);
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DelBindingOnu Error Rlt(%d) on slot(%d) onu(%d)", olt_id, onu_id, deregistration_code, code_param, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
#endif
    
    return iRlt;
}

#if 0
/*add pending onu*/
int OnuMgt_AddPendingOnu( short int olt_id, short int onu_id, unsigned char *mac_address )
{
	pendingOnu_S *CurOnu, *PreOnu;
	int iRlt=0;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
    VOS_ASSERT(mac_address);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, AddPendingOnu, (olt_id, onu_id,mac_address) )) )
    {
    	CurOnu = PonPortTable[olt_id].PendingOnu.Next;
    	PreOnu = NULL;

    	while( CurOnu != NULL )
		{
    		if(CurOnu->Llid == onu_id ) return( ROK );	
    		PreOnu = CurOnu;
    		CurOnu = CurOnu->Next;
		}
    	
    	CurOnu = VOS_Malloc( sizeof(pendingOnu_S),  MODULE_PON );
    	if( CurOnu == NULL )
		{
    		ASSERT(0);		
    		return ( RERROR );
		}
    	CurOnu->Llid = onu_id; 
    	VOS_MemCpy( CurOnu->MacAddr, mac_address, BYTES_IN_MAC_ADDRESS );
    	CurOnu->Next = NULL;
    	CurOnu->counter = 0;

    	if( PreOnu != NULL )
    		PreOnu->Next = CurOnu;
    	else
            PonPortTable[olt_id].PendingOnu.Next = CurOnu;
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_AddPendingOnu Error Rlt(%d) on slot(%d) onu(%d)", iRlt, SYS_LOCAL_MODULE_SLOTNO,onu_id);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	
	return( iRlt );
}

int OnuMgt_DelPendingOnu( short int olt_id, short int onu_id )
{
	pendingOnu_S *CurOnu, *PreOnu;
	int iRlt=0;
	short int Llid;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, DelPendingOnu, (olt_id, onu_id) )) )
    {	
    	Llid = GetLlidByOnuIdx(olt_id, onu_id);
    	if(Llid != INVALID_LLID )
    	{
	    	CurOnu = PonPortTable[olt_id].PendingOnu.Next;
	    	PreOnu = NULL;
	     
	    	while(CurOnu != NULL )
	    	{
	    		if( CurOnu->Llid == Llid)
	    		{
	    			if( PreOnu == NULL )
	    			{
	    				PonPortTable[olt_id].PendingOnu.Next = CurOnu->Next;				
	    			}
	    			else
	    			{
	    				PreOnu->Next = CurOnu->Next;
	    			}
	    			VOS_Free( (void *)CurOnu );

        			break;
        		}
                
        		PreOnu = CurOnu;
        		CurOnu = CurOnu->Next;
        	}
        }
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
        	if ( OLT_ERR_NOTEXIST != iRlt )
        	{
                VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DelPendingOnu Error Rlt(%d) on slot(%d) onu(%d)", iRlt, SYS_LOCAL_MODULE_SLOTNO,onu_id);
            }
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	
    return( iRlt);
}

int OnuMgt_DelConfPendingOnu( short int olt_id, short int onu_id )
{
	pendingOnu_S *CurOnu, *PreOnu;
	int iRlt=0;
	short int Llid;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
		
	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, DelConfPendingOnu, (olt_id, onu_id) )) )
    {
    	Llid = GetLlidByOnuIdx(olt_id, onu_id);
    	if(Llid == INVALID_LLID )	
    	{
        	CurOnu = PonPortTable[olt_id].PendingOnu_Conf.Next;
        	PreOnu = NULL;
        	
        	while(CurOnu != NULL )
	    	{
	    		if( CurOnu->Llid ==Llid )
	    		{
	    			if( PreOnu == NULL )
	                {
	    				PonPortTable[olt_id].PendingOnu_Conf.Next = CurOnu->Next;				
	    			}
	    			else
	                {
	    				PreOnu->Next = CurOnu->Next;
	    			}
	    			CurOnu->Llid = 0;
	    			CurOnu->Next = NULL;
	    			CurOnu->otherPonIdx = MAXPON;
	                
	    			break;
	    		}
	    		PreOnu = CurOnu;
	    		CurOnu = CurOnu->Next;
	    	}
    	}
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
        	if ( OLT_ERR_NOTEXIST != iRlt )
        	{
                VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DelConfPendingOnu Error Rlt(%d) on slot(%d) onu(%d)", iRlt, SYS_LOCAL_MODULE_SLOTNO,onu_id);
            }
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	VOS_SemGive(OnuPendingDataSemId);
	
    return( iRlt);
}
#endif

int OnuMgt_AuthorizeOnu(short int olt_id, short int onu_id, bool auth_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
   
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, AuthorizeOnu, (olt_id, onu_id, auth_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_AuthorizeOnu(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, auth_mode, iRlt);
    }

    return iRlt;
}


/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
int OnuMgt_AuthRequest ( short int olt_id, short int onu_id, CTC_STACK_auth_response_t *auth_response)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    
	OLT_ASSERT(olt_id);
	
	if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, AuthRequest, (olt_id, onu_id, auth_response) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_AuthRequest(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, *auth_response, iRlt);
    }

	return iRlt;
    
}
int OnuMgt_AuthSuccess ( short int olt_id, short int onu_id)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    

	OLT_ASSERT(olt_id);
	
	if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, AuthSucess, (olt_id, onu_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_AuthSuccess(%d, %d) Error Rlt(%d).", olt_id, onu_id,  iRlt);
    }

	return iRlt;
   
}

int OnuMgt_AuthFailure (short int olt_id, short int onu_id, CTC_STACK_auth_failure_type_t failure_type )
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
   
	OLT_ASSERT(olt_id);
	if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, AuthFail, (olt_id, onu_id, failure_type) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_AuthFailure(%d, %d) Error Rlt(%d).", olt_id, onu_id,  iRlt);
    }
	
	return iRlt;
   
}

/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
#endif


#if 1
/* -------------------ONU 业务管理API------------------- */

int OnuMgt_SetOnuTrafficServiceMode(short int olt_id, short int onu_id, int service_mode)
{
    int iRlt;
	short int olt_id_swap = 0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetOnuTrafficServiceMode, (olt_id, onu_id, service_mode) )) )
    {
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].TrafficServiceEnable = service_mode;
		ONU_MGMT_SEM_GIVE;

		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetOnuTrafficServiceMode(olt_id_swap, onu_id, service_mode);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetOnuTrafficServiceMode(%d, %d, %d) Error Rlt(%d).", olt_id_swap, onu_id, service_mode, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
		
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuTrafficServiceMode(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, service_mode, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
    
    return iRlt;
}

int OnuMgt_SetOnuPeerToPeer(short int olt_id, short int onu_id1, short int onu_id2, short int enable)
{
    int iRlt;
	short int olt_id_swap = 0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id1);
    ONU_ASSERT(onu_id2);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id1, SetOnuPeerToPeer, (olt_id, onu_id1, onu_id2,enable) )) )
    {
        if (V2R1_ENABLE == enable)
        {
            RecordOnuPeerToPeer(olt_id, onu_id1, onu_id2);
        }
        else
        {
            ClearOnuPeerToPeer(olt_id, onu_id1, onu_id2);
        }
		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetOnuPeerToPeer(olt_id_swap, onu_id1, onu_id2, enable);
		if(OLT_CALL_ISERROR(iRlt) && OLT_ERR_NOTEXIST != iRlt)
		{
			VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetOnuPeerToPeer(%d, %d, %d, %d) Error Rlt(%d).", olt_id_swap, onu_id1, onu_id2, enable, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
        	if ( OLT_ERR_NOTEXIST != iRlt )
        	{
                VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuPeerToPeer(%d, %d, %d, %d) Error Rlt(%d).", olt_id, onu_id1, onu_id2, enable, iRlt);
        	}
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	
    return iRlt;
}

int OnuMgt_SetOnuPeerToPeerForward(short int olt_id, short int onu_id, int address_not_found, int broadcast)
{
    int iRlt;
    int onuEntry;
	short int olt_id_swap = 0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetOnuPeerToPeerForward, (olt_id, onu_id, address_not_found, broadcast) )) )
    {
    	onuEntry = olt_id * MAXONUPERPON + onu_id;
        ONU_MGMT_SEM_TAKE;
    	OnuMgmtTable[onuEntry].address_not_found_flag = address_not_found;	
    	OnuMgmtTable[onuEntry].broadcast_flag = broadcast;
        ONU_MGMT_SEM_GIVE;

		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetOnuPeerToPeerForward(olt_id_swap, onu_id, address_not_found, broadcast);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetOnuPeerToPeerForward(%d, %d, %d, %d) Error Rlt(%d).", olt_id_swap, onu_id, address_not_found, broadcast, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuPeerToPeerForward(%d, %d, %d, %d) Error Rlt(%d).", olt_id, onu_id, address_not_found, broadcast, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	
    return iRlt;

}

int  OnuMgt_SetOnuBW(short int olt_id, short int onu_id, ONU_bw_t *BW)
{
    int iRlt;
    int onuMgtIdx;
	short int olt_id_swap = 0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(BW);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetOnuBW, (olt_id, onu_id, BW) )) )
    {
        if(OLT_ISLOCAL(olt_id))/*for onu swap by jinhl@2013-04-27*/
    	{
	        OnuLLIDTable_S *pstLlidCfg;
	    
	        onuMgtIdx = olt_id * MAXONUPERPON + onu_id;

	        ONU_MGMT_SEM_TAKE;
	        pstLlidCfg = &(OnuMgmtTable[onuMgtIdx].LlidTable[0]);
	        
	        if(BW->bw_direction & OLT_CFG_DIR_BY_ONUID)
	        {            
	            if(pstLlidCfg->EntryStatus == LLID_ENTRY_UNKNOWN )
	                pstLlidCfg->EntryStatus = LLID_ENTRY_NOT_IN_ACTIVE;
	            
	            if ( OLT_CFG_DIR_BOTH == (OLT_CFG_DIR_BOTH & BW->bw_direction) )
	            {
	                if ( OLT_CFG_DIR_UNDO & BW->bw_direction )
	                {
	                    if ( 0 != pstLlidCfg->BandWidthIsDefault )
	                    {
	                        /* 修改默认带宽 */                
	                        if ( OLT_CFG_DIR_UPLINK & pstLlidCfg->BandWidthIsDefault )
	                        {
	                            pstLlidCfg->UplinkBandwidth_gr = BW->bw_gr;
	                            pstLlidCfg->UplinkBandwidth_be = BW->bw_be;
	                        }

	                        if ( OLT_CFG_DIR_DOWNLINK & pstLlidCfg->BandWidthIsDefault )
	                        {
	                            pstLlidCfg->DownlinkBandwidth_gr = BW->bw_fixed;
	                            pstLlidCfg->DownlinkBandwidth_be = BW->bw_actived;
	                        }
	                    }
	                    else
	                    {
	                        /* 指定带宽的ONU，无需理会默认带宽的变化 */
	                    }
	                }
	                else
	                {
	                    /* 尚不支持双向指定带宽的设置 */
	                    VOS_ASSERT(0);
	                }
	            }
	            else
	            {
	                if ( OLT_CFG_DIR_INACTIVE & BW->bw_direction )
	                {
	                    /*do nothing*/
	                }
	                else
	                {
	                    if ( OLT_CFG_DIR_UPLINK & BW->bw_direction )
	                    {
	                        if ( OLT_CFG_DIR_UNDO & BW->bw_direction )
	                        {
	                            /* 恢复默认带宽 */
	                            pstLlidCfg->BandWidthIsDefault |= OLT_CFG_DIR_UPLINK;

	                            pstLlidCfg->UplinkBandwidth_fixed = 0;

								GetOnuDefaultBWByPonRate(olt_id, onu_id, OLT_CFG_DIR_UPLINK, &pstLlidCfg->UplinkBandwidth_gr, &pstLlidCfg->UplinkBandwidth_be);
	                            pstLlidCfg->UplinkClass = OnuConfigDefault.UplinkClass;
	                            pstLlidCfg->UplinkDelay = OnuConfigDefault.UplinkDelay;
	                        }
	                        else
	                        {
	                            /* 设置新带宽 */
	                            pstLlidCfg->BandWidthIsDefault &= ~OLT_CFG_DIR_UPLINK;
	                        }

	                        if ( 0 < BW->bw_gr )
	                        {
	                            pstLlidCfg->UplinkBandwidth_gr = BW->bw_gr;
	                            pstLlidCfg->ActiveUplinkBandwidth = BW->bw_actived;
	                        }
	                        
	                        if ( 0 < BW->bw_be )
	                        {
	                            pstLlidCfg->UplinkBandwidth_be = BW->bw_be;
	                        }
	                        
#ifdef  PLATO_DBA_V3
	                        pstLlidCfg->UplinkBandwidth_fixed = BW->bw_fixed;
#endif

	                        if ( -1 != BW->bw_class )
	                        {
	                            pstLlidCfg->UplinkClass = BW->bw_class;
	                        }
	                        
	                        if ( (-1 != BW->bw_delay) && (0 < BW->bw_delay) )
	                        {
	                            pstLlidCfg->UplinkDelay = BW->bw_delay;
	                        }
	                    }
	                    else
	                    {
	                        if ( OLT_CFG_DIR_UNDO & BW->bw_direction )
	                        {
	                            /* 恢复默认带宽 */
	                            pstLlidCfg->BandWidthIsDefault |= OLT_CFG_DIR_DOWNLINK;

								GetOnuDefaultBWByPonRate(olt_id, onu_id, OLT_CFG_DIR_DOWNLINK, &pstLlidCfg->DownlinkBandwidth_gr, &pstLlidCfg->DownlinkBandwidth_be);
	                            pstLlidCfg->DownlinkClass = OnuConfigDefault.DownlinkClass;
	                            pstLlidCfg->DownlinkDelay = OnuConfigDefault.DownlinkDelay;
	                        }
	                        else
	                        {
	                            /* 设置新带宽 */
	                            pstLlidCfg->BandWidthIsDefault &= ~OLT_CFG_DIR_DOWNLINK;
	                        }

	                        if ( 0 < BW->bw_gr )
	                        {
	                            pstLlidCfg->DownlinkBandwidth_gr = BW->bw_gr;
	                            pstLlidCfg->ActiveDownlinkBandwidth = BW->bw_actived;
	                        }
	                        
	                        if ( 0 < BW->bw_be )
	                        {
	                            pstLlidCfg->DownlinkBandwidth_be = BW->bw_be;
	                        }
	                        
	                        if ( -1 != BW->bw_class )
	                        {
	                            pstLlidCfg->DownlinkClass = BW->bw_class;
	                        }
	                        
	                        if ( (-1 != BW->bw_delay) && (0 < BW->bw_delay) )
	                        {
	                            pstLlidCfg->DownlinkDelay = BW->bw_delay;
	                        }
	                    }
	                }
	                
	        		if( EVENT_REGISTER == V2R1_ENABLE)
	        			sys_console_printf("\r\n  set pon%d/%d onu %d %slink config-bandwidth %dkbit/s OK\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), (onu_id+1), (OLT_CFG_DIR_UPLINK & BW->bw_direction) ? "up" : "down", BW->bw_gr );
	            }
	        }

	        if(OLT_CFG_DIR_DO_ACTIVE & BW->bw_direction)
	        {
	            /*更新onu的实际配置带宽*/
	            if ( OLT_CFG_DIR_BOTH == (OLT_CFG_DIR_BOTH & BW->bw_direction) )
	            {
	                if ( OLT_CFG_DIR_UNDO & BW->bw_direction )
	                {
	                    if ( 0 != pstLlidCfg->BandWidthIsDefault )
	                    {
	                        if ( OLT_CFG_DIR_UPLINK & pstLlidCfg->BandWidthIsDefault )
	                        {
	                            OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_gr = BW->bw_gr;
	                            OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_be = BW->bw_be;
	                            /*更新onu的激活带宽*/
	                            if ( -1 != BW->bw_class )
	                            {
	                                OnuMgmtTable[onuMgtIdx].ActiveUplinkBandwidth = BW->bw_class;
	                            }                            
	                        }

	                        if ( OLT_CFG_DIR_DOWNLINK & pstLlidCfg->BandWidthIsDefault )
	                        {
	                            OnuMgmtTable[onuMgtIdx].FinalDownlinkBandwidth_gr = BW->bw_fixed;
	                            OnuMgmtTable[onuMgtIdx].FinalDownlinkBandwidth_be = BW->bw_actived;
	                            if ( -1 != BW->bw_delay )
	                            {
	                                OnuMgmtTable[onuMgtIdx].ActiveDownlinkBandwidth = BW->bw_delay;
	                            }                            
	                        }
	                    }
	                    else
	                    {
	                        /* 指定带宽的ONU，无需理会默认带宽的变化 */
	                    }
	                }
	                else
	                {
	                    /* 尚不支持双向指定带宽的设置 */
	                    VOS_ASSERT(0);
	                }
	            }   
	            else
	            {
	                if ( OLT_CFG_DIR_INACTIVE & BW->bw_direction )
	                {
	                    /* 带宽去激活，无需保存去激活的带宽配置 */
	                    if ( OLT_CFG_DIR_UPLINK & BW->bw_direction )
	                    {
	                        OnuMgmtTable[onuMgtIdx].ActiveUplinkBandwidth = 0;
	                    }
	                    else
	                    {
	                        OnuMgmtTable[onuMgtIdx].ActiveDownlinkBandwidth = 0;
	                    }
	                }
	                else
	                {
	                    if ( OLT_CFG_DIR_UPLINK & BW->bw_direction )
	                    {
	                        if ( OLT_CFG_DIR_UNDO & BW->bw_direction )
	                        {
	                            /* 恢复默认带宽 */
	                            if(pstLlidCfg->BandWidthIsDefault & OLT_CFG_DIR_UPLINK)
	                                OnuMgmtTable[onuMgtIdx].BandWidthIsDefault |= OLT_CFG_DIR_UPLINK; 
	                            OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_fixed = pstLlidCfg->UplinkBandwidth_fixed;
	                            OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_gr = pstLlidCfg->UplinkBandwidth_gr;
	                            OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_be = pstLlidCfg->UplinkBandwidth_be;
	                            OnuMgmtTable[onuMgtIdx].FinalUplinkClass = pstLlidCfg->UplinkClass;
	                            OnuMgmtTable[onuMgtIdx].FinalUplinkDelay = pstLlidCfg->UplinkDelay;
	                        }
	                        else
	                        {
	                            /* 设置新带宽 */
	                            OnuMgmtTable[onuMgtIdx].BandWidthIsDefault &= ~OLT_CFG_DIR_UPLINK;
	                            OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_fixed = BW->bw_fixed;
	                            OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_gr = BW->bw_gr;
	                            OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_be = BW->bw_be;
	                            OnuMgmtTable[onuMgtIdx].FinalUplinkClass = BW->bw_class;
	                            OnuMgmtTable[onuMgtIdx].FinalUplinkDelay = BW->bw_delay;                            
	                        }
	                        
	                        OnuMgmtTable[onuMgtIdx].ActiveUplinkBandwidth = BW->bw_actived;
	                    }
	                    else
	                    {
	                        if ( OLT_CFG_DIR_UNDO & BW->bw_direction )
	                        {
	                            /* 恢复默认带宽 */
	                            if(pstLlidCfg->BandWidthIsDefault & OLT_CFG_DIR_DOWNLINK)
	                                OnuMgmtTable[onuMgtIdx].BandWidthIsDefault |= OLT_CFG_DIR_DOWNLINK; 
	                            OnuMgmtTable[onuMgtIdx].FinalDownlinkBandwidth_gr = pstLlidCfg->DownlinkBandwidth_gr;
	                            OnuMgmtTable[onuMgtIdx].FinalDownlinkBandwidth_be = pstLlidCfg->DownlinkBandwidth_be;
	                            OnuMgmtTable[onuMgtIdx].FinalDownlinkClass = pstLlidCfg->DownlinkClass;
	                            OnuMgmtTable[onuMgtIdx].FinalDownlinkDelay = pstLlidCfg->DownlinkDelay;
	                            
	                        }
	                        else
	                        {
	                            /* 设置新带宽 */
	                            OnuMgmtTable[onuMgtIdx].BandWidthIsDefault &= ~OLT_CFG_DIR_DOWNLINK;
	                            OnuMgmtTable[onuMgtIdx].FinalDownlinkBandwidth_gr = BW->bw_gr;
	                            OnuMgmtTable[onuMgtIdx].FinalDownlinkBandwidth_be = BW->bw_be;
	                            OnuMgmtTable[onuMgtIdx].FinalDownlinkClass = BW->bw_class;
	                            OnuMgmtTable[onuMgtIdx].FinalDownlinkDelay = BW->bw_delay;
	                        }

	                        OnuMgmtTable[onuMgtIdx].ActiveDownlinkBandwidth = BW->bw_actived;
	                    }
	                }
	                
	        		if( EVENT_REGISTER == V2R1_ENABLE)
	        			sys_console_printf("\r\n  set pon%d/%d onu %d %slink bandwidth %dkbit/s OK\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), (onu_id+1), (OLT_CFG_DIR_UPLINK & BW->bw_direction) ? "up" : "down", BW->bw_gr );
	            }                    
	        }
	        ONU_MGMT_SEM_GIVE;  
    	}

		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetOnuBW(olt_id_swap, onu_id, BW);
		if(OLT_CALL_ISERROR(iRlt))
		{   
		   	if( EVENT_REGISTER == V2R1_ENABLE)
				sys_console_printf("\r\n  Partner set pon%d/%d onu %d %slink bandwidth %dkbit/s err, errId %d\r\n", GetCardIdxByPonChip(olt_id_swap), GetPonPortByPonChip(olt_id_swap), (onu_id+1), (OLT_CFG_DIR_UPLINK & BW->bw_direction) ? "up" : "down", BW->bw_gr, iRlt);
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetOnuBW(%d, %d) Error Rlt(%d).", olt_id_swap, onu_id, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            if( EVENT_REGISTER == V2R1_ENABLE)
        		sys_console_printf("\r\n  set pon%d/%d onu %d %slink bandwidth %dkbit/s err, errId %d\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), (onu_id+1), (OLT_CFG_DIR_UPLINK & BW->bw_direction) ? "up" : "down", BW->bw_gr, iRlt);

            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuBW(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	
    return iRlt;
}

int  OnuMgt_GetOnuSLA(short int olt_id, short int onu_id, ONU_SLA_INFO_t *sla_info)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sla_info);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOnuSLA, (olt_id, onu_id, sla_info) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOnuSLA(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
	
    return iRlt;
}

int OnuMgt_SetOnuFecMode(short int olt_id, short int onu_id, int fec_mode)
{
    int iRlt=0;
	short int olt_id_swap = 0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetOnuFecMode, (olt_id, onu_id, fec_mode) )) )
    {
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].FEC_Mode = fec_mode;
		ONU_MGMT_SEM_GIVE;

		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetOnuFecMode(olt_id_swap, onu_id, fec_mode);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetOnuFecMode(d, %d, %d) Error Rlt(%d).", olt_id_swap, onu_id, fec_mode, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuFecMode(d, %d, %d) Error Rlt(%d).", olt_id, onu_id, fec_mode, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
    
    if(iRlt == VOS_OK)
    {
        if(fec_mode == STD_FEC_MODE_ENABLED)
            setOnuConfSimpleVar(olt_id, onu_id, sv_enum_onu_fec_enable, 1);
        if(fec_mode == STD_FEC_MODE_DISABLED)
            setOnuConfSimpleVar(olt_id, onu_id, sv_enum_onu_fec_enable, 0);
    }

    return iRlt;
}

/*get onu vlan mode*/
int OnuMgt_GetOnuVlanMode(short int olt_id, short int onu_id, PON_olt_vlan_uplink_config_t *vlan_uplink_config)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(vlan_uplink_config);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOnuVlanMode, (olt_id, onu_id, vlan_uplink_config) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOnuVlanMode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
int OnuMgt_SetUniPort(short int olt_id, short int onu_id, bool enable_cpu, bool enable_datapath)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetUniPort, (olt_id, onu_id, enable_cpu, enable_datapath) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetUniPort(%d, %d, %d, %d) Error Rlt(%d).", olt_id, onu_id, enable_cpu, enable_datapath, iRlt);
    }
    
    return iRlt;
}

int OnuMgt_SetSlowProtocolLimit(short int olt_id, short int onu_id, bool enable)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetSlowProtocolLimit, (olt_id, onu_id, enable) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetSlowProtocolLimit(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, enable, iRlt);
    }
    
    return iRlt;
}

int OnuMgt_GetSlowProtocolLimit(short int olt_id, short int onu_id, bool *enable)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetSlowProtocolLimit, (olt_id, onu_id, enable) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetSlowProtocolLimit(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, *enable, iRlt);
    }
    
    return iRlt;
}
/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/


/*Begin:for onu swap by jinhl@2013-04-27*/
int OnuMgt_GetBWInfo(short int olt_id, short int onu_id, PonPortBWInfo_S *ponBW, ONUBWInfo_S *onuBW)
{
    int iRlt;
	
    OLT_ASSERT(olt_id);
	if((NULL == ponBW) || (NULL == onuBW))
	{
	    VOS_ASSERT(0);
		return VOS_ERROR;
	}
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetBWInfo, (olt_id, onu_id, ponBW, onuBW) )) )
    {
        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_GetBWInfo(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}
/*End:for onu swap by jinhl@2013-04-27*/


int OnuMgt_GetOnuB2PMode(short int olt_id, short int onu_id, int *b2p_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOnuB2PMode, (olt_id, onu_id, b2p_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_GetOnuB2PMode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetOnuB2PMode(short int olt_id, short int onu_id, int b2p_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetOnuB2PMode, (olt_id, onu_id, b2p_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_SetOnuB2PMode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}
#endif


#if 1
/* -------------------ONU 监控统计管理API--------------- */

int OnuMgt_ResetCounters(short int olt_id, short int onu_id)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, ResetCounters, (olt_id, onu_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_ResetCounters(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetPonLoopback(short int olt_id, short int onu_id, int enable)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetPonLoopback, (olt_id, onu_id, enable) )) )
    {
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].LoopbackEnable = enable;
		ONU_MGMT_SEM_GIVE;
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
	        VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_PonLoopback(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, enable, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

#endif


#if 1
/* -------------------ONU加密管理API-------------------- */

/*get onu llid paramrters*/
int OnuMgt_GetLLIDParams(short int olt_id, short int onu_id, PON_llid_parameters_t *llid_parameters)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(llid_parameters);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetLLIDParams, (olt_id, onu_id, llid_parameters) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetLLIDParams(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}

/*onu start encryption*/
int OnuMgt_StartEncryption(short int olt_id, short int onu_id, int *encrypt_dir)
{
    int iRlt;
    int enc_dir;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(encrypt_dir);

    enc_dir = *encrypt_dir;
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, StartEncryption, (olt_id, onu_id, encrypt_dir) )) )
    {
        int onuMgtIdx;

        onuMgtIdx = olt_id * MAXONUPERPON + onu_id;
        ONU_MGMT_SEM_TAKE;
        OnuMgmtTable[onuMgtIdx].EncryptDirection = enc_dir;
        OnuMgmtTable[onuMgtIdx].EncryptEnable = V2R1_ENABLE;

        if ( 0 == *encrypt_dir )
        {
            /* 标识加密开始状态 */
            OnuMgmtTable[onuMgtIdx].EncryptStatus = V2R1_STARTED;
        }

        if ( V2R1_ENABLE == OnuMgmtTable[onuMgtIdx].EncryptFirstTime )
        {
            OnuMgmtTable[onuMgtIdx].EncryptFirstTime = V2R1_DISABLE;
        }
        ONU_MGMT_SEM_GIVE;
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_StartEncryption(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, enc_dir, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
    
    return iRlt;
}

/*onu stop encryption*/
int OnuMgt_StopEncryption(short int olt_id, short int onu_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, StopEncryption, (olt_id, onu_id) )) )
    {
        int onuMgtIdx;

        onuMgtIdx = olt_id * MAXONUPERPON + onu_id;
        ONU_MGMT_SEM_TAKE;
        OnuMgmtTable[onuMgtIdx].EncryptDirection = PON_ENCRYPTION_PURE;
        OnuMgmtTable[onuMgtIdx].EncryptEnable = V2R1_DISABLE;

        OnuMgmtTable[onuMgtIdx].EncryptStatus  = V2R1_NOTSTARTED;
        OnuMgmtTable[onuMgtIdx].EncryptCounter = (OnuMgmtTable[onuMgtIdx].EncryptKeyTime / SECOND_1);
        ONU_MGMT_SEM_GIVE;
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_StopEncryption(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
    
    return iRlt;
}

int OnuMgt_SetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int key_change_time)
{
    int iRlt;
    int enc_dir;
	short int olt_id_swap = 0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( NULL != encrypt_dir )
    {
        enc_dir = *encrypt_dir;
    }
    else
    {
        enc_dir = -1;
    }
    
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetOnuEncryptParams, (olt_id, onu_id, encrypt_dir, key_change_time) )) )
    {	
        int onuMgtIdx;

        onuMgtIdx = olt_id * MAXONUPERPON + onu_id;
        ONU_MGMT_SEM_TAKE;
        if ( enc_dir > 0 )
        {
           OnuMgmtTable[onuMgtIdx].EncryptDirection = enc_dir;
            if ( PON_ENCRYPTION_PURE == enc_dir )
            {
                OnuMgmtTable[onuMgtIdx].EncryptEnable = V2R1_DISABLE;
                OnuMgmtTable[onuMgtIdx].EncryptStatus = V2R1_NOTSTARTED;
            }
            else
            {
                OnuMgmtTable[onuMgtIdx].EncryptEnable = V2R1_ENABLE;
                if ( 0 == *encrypt_dir )
                {
                    /* 标识加密开始状态 */
                    OnuMgmtTable[onuMgtIdx].EncryptStatus = V2R1_STARTED;
                }
            }
        }
        
        if ( key_change_time > 0 )
        {
            OnuMgmtTable[onuMgtIdx].EncryptKeyTime = key_change_time;
            OnuMgmtTable[onuMgtIdx].EncryptCounter = (key_change_time / SECOND_1);
        }
		ONU_MGMT_SEM_GIVE;

		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetOnuEncryptParams(olt_id_swap, onu_id, encrypt_dir, key_change_time);
		if(OLT_CALL_ISERROR(iRlt))
		{
			VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetOnuEncryptParams(%d, %d, %d, %d) Error Rlt(%d).", olt_id_swap, onu_id, enc_dir, key_change_time, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END	
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuEncryptParams(%d, %d, %d, %d) Error Rlt(%d).", olt_id, onu_id, enc_dir, key_change_time, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	
    return iRlt;
}

int OnuMgt_GetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int *key_change_time, int *encrypt_status)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOnuEncryptParams, (olt_id, onu_id, encrypt_dir, key_change_time, encrypt_status) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOnuEncryptParams(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
int OnuMgt_UpdateEncryptionKey(short int olt_id, short int onu_id, PON_encryption_key_t encryption_key, PON_encryption_key_update_t key_update_method)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, UpdateEncryptionKey, (olt_id, onu_id, encryption_key, key_update_method) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_UpdateEncryptionKey(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------ONU 地址管理API------------------- */

int OnuMgt_GetOnuMacAddrTbl(short int olt_id, short int onu_id, long *EntryNum, PON_onu_address_table_record_t *addr_table)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOnuMacAddrTbl, (olt_id, onu_id, EntryNum, addr_table) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOnuMacAddrTbl(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}

int OnuMgt_GetOltMacAddrTbl(short int olt_id, short int onu_id, short int *EntryNum, PON_address_table_t addr_table)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOltMacAddrTbl, (olt_id, onu_id, EntryNum, addr_table) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOltMacAddrTbl(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}

int OnuMgt_GetOltMacAddrVlanTbl(short int olt_id, short int onu_id, short int *EntryNum, PON_address_vlan_table_t addr_table)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOltMacAddrVlanTbl, (olt_id, onu_id, EntryNum, addr_table) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOltMacAddrVlanTbl(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}
/*onu max mac*/
int OnuMgt_SetOnuMaxMac(short int olt_id, short int onu_id, short int llid_id, unsigned int *val )
{
    int iRlt;
    int default_flag = 1;
    int undo_flag = 0;
	short int olt_id_swap = 0;
	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
    LLID_ID_ASSERT(llid_id);
    VOS_ASSERT(val);
	int val_tmp = *val;
#if 1
    if(*val & ONU_NOT_DEFAULT_MAX_MAC_FLAG)
        default_flag = 0;
    else if(*val & ONU_UNDO_MAX_MAC_FLAG)
        undo_flag = 1;
#endif
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetOnuMaxMac, (olt_id, onu_id, llid_id, val) )) )
    {	
#if 1      
        if(!default_flag)
        {
        	ONU_MGMT_SEM_TAKE;
        	OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].LlidTable[llid_id].MaxMAC = (*val)|ONU_NOT_DEFAULT_MAX_MAC_FLAG;			
        	ONU_MGMT_SEM_GIVE;
        }
        else if(undo_flag)
        {
        	ONU_MGMT_SEM_TAKE; 
        	OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].LlidTable[llid_id].MaxMAC =  (*val)&(~ONU_UNDO_MAX_MAC_FLAG);			
        	ONU_MGMT_SEM_GIVE;
        }
        else
        {
        	ONU_MGMT_SEM_TAKE;
        	if(OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].LlidTable[llid_id].MaxMAC & ONU_NOT_DEFAULT_MAX_MAC_FLAG)	
        	{
                /*do nothing*/
        	}
            else
            {
            	OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].LlidTable[llid_id].MaxMAC = *val;			
            }
        	ONU_MGMT_SEM_GIVE;
        }
#else
    	ONU_MGMT_SEM_TAKE;
    	OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].LlidTable[llid_id].MaxMAC = *val;			
    	ONU_MGMT_SEM_GIVE;
#endif
		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetOnuMaxMac(olt_id_swap, onu_id, llid_id, &val_tmp);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetOnuMaxMac(%d, %d, %d, %d) Error Rlt(%d).", olt_id_swap, onu_id, llid_id, *val, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuMaxMac(%d, %d, %d, %d) Error Rlt(%d).", olt_id, onu_id, llid_id, *val, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	
    return iRlt;
}

/*get onu uni port mac configuration*/
int OnuMgt_GetOnuUniMacCfg(short int olt_id, short int onu_id, PON_oam_uni_port_mac_configuration_t *mac_config)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(mac_config);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOnuUniMacCfg, (olt_id, onu_id, mac_config) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOnuUniMacCfg(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}

int OnuMgt_GetOnuMacCheckFlag(short int olt_id, short int onu_id, ULONG  *flag)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(flag);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOnuMacCheckFlag, (olt_id, onu_id, flag) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOnuMacCheckFlag(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}
#endif


#if 1
/* -------------------ONU 光路管理API------------------- */

int OnuMgt_GetOnuDistance(short int olt_id, short int onu_id, int *rtt)
{
    int iRlt = 0;
    int onuMgtIdx;
    int iRtt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(rtt);

    if ( OLT_ISLOCAL(olt_id) )
    {
        onuMgtIdx = olt_id * MAXONUPERPON + onu_id;
        if ( NOT_KNOWN != (iRtt = OnuMgmtTable[onuMgtIdx].RTT) )
        {
            *rtt = iRtt;
        }
        else
        {
            if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, GetOnuDistance, (olt_id, onu_id, rtt) )) )
            {
			ONU_MGMT_SEM_TAKE;
        		OnuMgmtTable[onuMgtIdx].RTT = *rtt;
			ONU_MGMT_SEM_GIVE;
            }
        }
    }
    else
    {
        iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOnuDistance, (olt_id, onu_id, rtt) );
    }

    if ( OLT_CALL_ISERROR(iRlt) )
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOnuDistance(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    else
    {
        iRlt = 0;
    }
	
    return iRlt;
}

int OnuMgt_GetOpticalCapability(short int olt_id, short int onu_id, ONU_optical_capability_t *optical_capability)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(optical_capability);
    
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOpticalCapability, (olt_id, onu_id, optical_capability) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOpticalCapability(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, optical_capability->pon_tx_signal, iRlt);
    }

    return iRlt;
}

#endif


#if 1
/* -------------------ONU 倒换API---------------- */

int OnuMgt_SetOnuLLID(short int olt_id, short int onu_id, short int llid)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    LLID_ASSERT(llid);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetOnuLLID, (olt_id, onu_id, llid) )) )
    {
        int iOnuMgtIdx = olt_id * MAXONUPERPON + onu_id;

        ONU_MGMT_SEM_TAKE;
        OnuMgmtTable[iOnuMgtIdx].LLID = llid;
        OnuMgmtTable[iOnuMgtIdx].LlidTable[0].Llid = llid;
		/*for onu swap by jinhl@2013-02-22*/
		if(INVALID_LLID != llid)
		{
			OnuMgmtTable[iOnuMgtIdx].llidNum = 1;
		}
		else
		{
		    OnuMgmtTable[iOnuMgtIdx].llidNum = 0;
		}
        ONU_MGMT_SEM_GIVE;
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuLLID(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, llid, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"OnuMgt_SetOnuLLID(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

#endif


#if 1
/* -------------------ONU 设备管理API------------------- */

/*get onu ver*/
int OnuMgt_GetOnuVer(short int olt_id, short int onu_id, PON_onu_versions *onu_versions)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(onu_versions);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOnuVer, (olt_id, onu_id, onu_versions) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOnuVer(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}

/*get onu pon chip ver*/
int OnuMgt_GetOnuPonVer(short int olt_id, short int onu_id, PON_device_versions_t *device_versions)
{
    int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
    VOS_ASSERT(device_versions);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOnuPonVer, (olt_id, onu_id, device_versions) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOnuPonVer(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}


int OnuMgt_GetOnuRegisterInfo(short int olt_id, short int onu_id, onu_registration_info_t *onu_info)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(onu_info);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetOnuRegisterInfo, (olt_id, onu_id, onu_info) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetOnuRegisterInfo(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}


int OnuMgt_GetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long *size)
{
	int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(data);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetOnuI2CInfo, (olt_id, onu_id, info_id, data, size));

    return iRlt;
}

int OnuMgt_SetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long size)
{
	int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(data);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetOnuI2CInfo, (olt_id, onu_id, info_id, data, size));

    return iRlt;
}


int OnuMgt_ResetOnu(short int olt_id, short int onu_id)
{
	int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, ResetOnu, (olt_id, onu_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_ResetOnu(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
	
    return iRlt;
}


/* B--added by liwei056@2011-2-12 for D11920 */
int OnuMgt_SetOnuSWUpdateMode(short int olt_id, short int onu_id, int update_mode)
{
    int iRlt;
	short int olt_id_swap = 0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetOnuSWUpdateMode, (olt_id, onu_id, update_mode) )) )
    {
        int onuMgtIdx = olt_id * MAXONUPERPON + onu_id;

        ONU_MGMT_SEM_TAKE;
        OnuMgmtTable[onuMgtIdx].SoftwareUpdateCtrl = (UCHAR)update_mode;
        ONU_MGMT_SEM_GIVE;
		
		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetOnuSWUpdateMode(olt_id_swap, onu_id, update_mode);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetOnuSWUpdateMode(%d, %d, %d) Error Rlt(%d).", olt_id_swap, onu_id, update_mode, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuSWUpdateMode(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, update_mode, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	
    return iRlt;
}
/* E--added by liwei056@2011-2-12 for D11920 */

/* onu app升级*/
int OnuMgt_OnuSwUpdate(short int olt_id, short int onu_id, int update_flags, unsigned int update_filetype)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, OnuSwUpdate, (olt_id, onu_id, update_flags, update_filetype) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_OnuSwUpdate(%d, %d, %d, %d) Error Rlt(%d).", olt_id, onu_id, update_flags, update_filetype, iRlt);
    }
	
    return iRlt;
}

/* gw 和ctc 的ONU软件互升级*/
int OnuMgt_OnuGwCtcSwConvert(short int olt_id, short int onu_id, char file_id[ONU_TYPE_LEN + 4])
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(file_id);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, OnuGwCtcSwConvert, (olt_id, onu_id, file_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_OnuGwCtcSwConvert(%d, %d, %s) Error Rlt(%d).", olt_id, onu_id, file_id, iRlt);
    }
	
    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
int OnuMgt_GetBurnImageComplete(short int olt_id, short int onu_id, bool *complete)
{
	int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetBurnImageComplete, (olt_id, onu_id, complete) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetBurnImageComplete(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
	
    return iRlt;
}
/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/


int OnuMgt_SetOnuDeviceName(short int olt_id, short int onu_id, char *Name, int NameLen)
{
    int iRlt;
	short int olt_id_swap = 0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(Name);

    if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetOnuDeviceName, (olt_id, onu_id, Name, NameLen) )) )
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuDeviceName"
                "(%d, %d, %s, %d) Error Rlt(%d).", olt_id, onu_id, Name, NameLen, iRlt);
        }

        iRlt = OLT_ERR_OK;
    }

    /*begin: added by liub 2017-5-17. 只要存在set name操作，均保存在OLT 本地，并返回OK */
	if(OLT_ISLOCAL(olt_id))
	{
		iRlt = SetOnuDeviceName_1( olt_id, onu_id, Name, NameLen );
        if (OLT_CALL_ISERROR(iRlt))
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuDeviceName SetOnuDeviceName"
                "(%d, %d, %s, %d) Error Rlt(%d).", olt_id, onu_id, Name, NameLen, iRlt);
            return iRlt;
        }

        ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetOnuDeviceName(olt_id_swap, onu_id, Name, NameLen);
		if (OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetOnuDeviceName"
                "(%d, %d, %s, %d) Error Rlt(%d).", olt_id_swap, onu_id, Name, NameLen, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
	}
	/*end*/

    return iRlt;
}

int OnuMgt_SetOnuDeviceDesc(short int olt_id, short int onu_id, char *Desc, int DescLen)
{
	int iRlt;
	short int olt_id_swap = 0;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	VOS_ASSERT(Desc);

	if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetOnuDeviceDesc, (olt_id, onu_id, Desc, DescLen) )) )
	{
		int onuMgtIdx = olt_id * MAXONUPERPON + onu_id;

		SetOnuDeviceDesc_1(olt_id, onu_id, Desc, DescLen);

		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetOnuDeviceDesc(olt_id_swap, onu_id, Desc, DescLen);
		if(OLT_CALL_ISERROR(iRlt))
		{
			VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetOnuDeviceDesc(%d, %d, %s, %d) Error Rlt(%d).", olt_id_swap, onu_id, Desc, DescLen, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
	}
	else
	{
		if ( OLT_CALL_ISERROR(iRlt) )
		{
			VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuDeviceDesc(%d, %d, %s, %d) Error Rlt(%d).", olt_id, onu_id, Desc, DescLen, iRlt);
		}
		else
		{
			iRlt = OLT_ERR_OK;
		}
	}

	return iRlt;
}

int OnuMgt_SetOnuDeviceLocation(short int olt_id, short int onu_id, char *Location, int LocationLen)
{
	int iRlt;
	short int olt_id_swap = 0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(Location);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL( olt_id, onu_id, SetOnuDeviceLocation, (olt_id, onu_id, Location, LocationLen) )) )
	{
		int onuMgtIdx = olt_id * MAXONUPERPON + onu_id;

		SetOnuLocation_1(olt_id, onu_id, Location, LocationLen);
		
		ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetOnuDeviceLocation(olt_id_swap, onu_id, Location, LocationLen);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetOnuDeviceLocation(%d, %d, %s, %d) Error Rlt(%d).", olt_id_swap, onu_id, Location, LocationLen, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
	}
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetOnuDeviceLocation(%d, %d, %s, %d) Error Rlt(%d).", olt_id, onu_id, Location, LocationLen, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	
    return iRlt;
}
int OnuMgt_GetOnuAllPortStatisticData(short int olt_id, short int onu_id, OnuStatisticData_S* data)
{
	int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(data);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetOnuAllPortStatisticData, (olt_id, onu_id, data));

    return iRlt;
}

#endif
#if 1
/************Dr.Peng**********************/
int OnuMgt_DrPengSetOnuPortSaveConfig(short int olt_id, short int onu_id, short int port_id,unsigned char action )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,SetOnuPortSaveConfig, (olt_id,onu_id,port_id,action))))
   	{
   		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengSetOnuPortSaveConfig(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
   	}

    return iRlt;
}

int OnuMgt_DrPengSetOnuLoopDetectionTime(short int olt_id, short int onu_id, unsigned short port_downtime,unsigned short restart_port_times )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,SetOnuLoopDetectionTime, (olt_id,onu_id,port_downtime,restart_port_times))))
   	{
   		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengSetOnuLoopDetectionTime(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
   	}

    return iRlt;
}

int OnuMgt_DrPengGetOnuLoopDetectionTime(short int olt_id, short int onu_id, unsigned short *port_downtime,unsigned short *restart_port_times )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetOnuLoopDetectionTime, (olt_id,onu_id,port_downtime,restart_port_times))))
   	{
   		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengGetOnuLoopDetectionTime(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
   	}

    return iRlt;
}
int OnuMgt_DrPengSetOnuPortMode(short int olt_id, short int onu_id, unsigned short port_id,unsigned char mode )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,SetOnuPortMode, (olt_id,onu_id,port_id,mode))))
   	{
   		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengSetOnuPortMode(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
   	}

    return iRlt;
}

int OnuMgt_DrPengGetOnuPortMode(short int olt_id, short int onu_id, unsigned short port_id,unsigned char *mode )
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetOnuPortMode, (olt_id,onu_id,port_id,mode))))
	{
		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengGetOnuPortMode(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}

	return iRlt;
}
int OnuMgt_DrPengSetOnuPortStormStatus(short int olt_id, short int onu_id, unsigned short port_id,OnuPortStorm_S * status )
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,SetOnuPortStormStatus, (olt_id,onu_id,port_id,status))))
	{
		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengSetOnuPortStormStatus(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}

	return iRlt;
}
int OnuMgt_DrPengGetOnuPortStormStatus(short int olt_id, short int onu_id, unsigned short port_id,OnuPortStorm_S * status )
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetOnuPortStormStatus, (olt_id,onu_id,port_id,status))))
	{
		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengGetOnuPortStormStatus(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}

	return iRlt;
}


int OnuMgt_DrPengSetOnuDeviceLocation(short int olt_id, short int onu_id, unsigned char* device_location,unsigned char  device_location_len )
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,PengSetOnuDeviceLocation, (olt_id,onu_id,device_location,device_location_len))))
	{
		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengSetOnuDeviceLocation(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}

	return iRlt;
}

int OnuMgt_DrPengGetOnuDeviceLocation(short int olt_id, short int onu_id, unsigned char* device_location)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetOnuDeviceLocation, (olt_id,onu_id,device_location))))
	{
		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengGetOnuDeviceLocation(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}

	return iRlt;
}

int OnuMgt_DrPengSetOnuDeviceDescription(short int olt_id, short int onu_id, unsigned char* name,unsigned char  name_len )
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,SetOnuDeviceDescription, (olt_id,onu_id,name,name_len))))
	{
		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengSetOnuDeviceDescription(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}

	return iRlt;
}

int OnuMgt_DrPengGetOnuDeviceDescription(short int olt_id, short int onu_id, unsigned char* name )
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetOnuDeviceDescription, (olt_id,onu_id,name))))
	{
		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengGetOnuDeviceDescription(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}

	return iRlt;
}

int OnuMgt_DrPengSetOnuDeviceName(short int olt_id, short int onu_id, unsigned char* device_name,unsigned char  device_name_len )
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,PengSetOnuDeviceName, (olt_id,onu_id,device_name,device_name_len))))
	{
		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengSetOnuDeviceName(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}

	return iRlt;
}

int OnuMgt_DrPengGetOnuDeviceName(short int olt_id, short int onu_id, unsigned char* device_name )
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,PengGetOnuDeviceName, (olt_id,onu_id,device_name))))
	{
		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengGetOnuDeviceName(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}

	return iRlt;
}

int OnuMgt_DrPengGetOnuPortMacNumber(short int olt_id, short int onu_id,  short int port_id, unsigned short  *mac_address_number )
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetOnuPortMacNumber, (olt_id,onu_id,port_id,mac_address_number))))
	{
		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengGetOnuPortMacNumber(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}

	return iRlt;
}

int OnuMgt_DrPengGetOnuPortLocationByMAC(short int olt_id, short int onu_id, mac_address_t mac, short int vlan_id,OnuPortLacationEntry_S *port_location_infor )
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetOnuPortLocationByMAC, (olt_id,onu_id,mac,vlan_id,port_location_infor))))
	{
		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengGetOnuPortLocationByMAC(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}

	return iRlt;
}

int OnuMgt_DrPengGetOnuExtendAttribute(short int olt_id, short int onu_id, char *SupportAttribute)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetOnuSupportExtendAtt, (olt_id,onu_id,SupportAttribute))))
	{
		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_DrPengGetOnuSupportExtendAttribute(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}

	return iRlt;
}

int OnuMgt_DrPengSetOnuPortIsolation( short int olt_id, short int onu_id,unsigned char state)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,SetOnuPortIsolation, (olt_id,onu_id,state))))
   	{
   		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_SetOnuPortIsolation(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
   	}

    return iRlt;
}

int OnuMgt_DrPengGetOnuPortIsolation( short int olt_id, short int onu_id,unsigned char * state)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK != (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetOnuPortIsolation, (olt_id,onu_id ,state))))
   	{
   		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_GetOnuPortIsolation(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
   	}

    return iRlt;
}


int OnuMgt_GetOnuMacEntry(short int olt_id, short int onu_id, ULONG mactype ,OnuPortLacationInfor_S *table)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetOnuMacEntry, (olt_id,onu_id,mactype ,table))))
	{

	}
	else
	{
	VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "GetOnuMacEntry(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
	}
	return iRlt;
}
#endif


#if 1
/* -------------------ONU CTC-PROTOCOL API---------- */
int OnuMgt_GetCtcVersion( short int olt_id, short int onu_id, unsigned char *version )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(version);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetCtcVersion, (olt_id, onu_id, version));

	return iRlt;
}

int OnuMgt_GetFirmwareVersion( short int olt_id, short int onu_id, unsigned short int *version )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(version);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetFirmwareVersion, (olt_id, onu_id, version));

	return iRlt;
}

int OnuMgt_GetSerialNumber( short int olt_id, short int onu_id, CTC_STACK_onu_serial_number_t *serial_number )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(serial_number);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetSerialNumber, (olt_id, onu_id, serial_number));

	return iRlt;
}

int OnuMgt_GetChipsetID( short int olt_id, short int onu_id, CTC_STACK_chipset_id_t *chipset_id )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(chipset_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetChipsetID, (olt_id, onu_id, chipset_id));

	return iRlt;
}


int OnuMgt_GetOnuCap1( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_t *caps )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetOnuCap1, (olt_id, onu_id, caps));

	return iRlt;
}

int OnuMgt_GetOnuCap2( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_2_t *caps )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetOnuCap2, (olt_id, onu_id, caps));

	return iRlt;
}

int OnuMgt_GetOnuCap3( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_3_t *caps )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetOnuCap3, (olt_id, onu_id, caps));

	return iRlt;
}

int OnuMgt_UpdateOnuFirmware( short int olt_id, short int onu_id, void *file_start, int file_len, char *file_name )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(file_start);
    VOS_ASSERT(file_len > 0);
    VOS_ASSERT(file_name);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, UpdateOnuFirmware, (olt_id, onu_id, file_start, file_len, file_name));

	return iRlt;
}

int OnuMgt_ActiveOnuFirmware( short int olt_id, short int onu_id )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, ActiveOnuFirmware, (olt_id, onu_id));

	return iRlt;
}

int OnuMgt_CommitOnuFirmware( short int olt_id, short int onu_id )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, CommitOnuFirmware, (olt_id, onu_id));

	return iRlt;
}

int OnuMgt_StartEncrypt(short int olt_id, short int onu_id)
{
	int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, StartEncrypt, (olt_id, onu_id));

    return iRlt;
}

int OnuMgt_StopEncrypt(short int olt_id, short int onu_id)
{
	int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, StopEncrypt, (olt_id, onu_id));

    return iRlt;
}

int OnuMgt_GetEthPortLinkState(short int olt_id, short int onu_id, int port_id, int *link)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortLinkState, (olt_id, onu_id, port_id, link));

	return iRlt;
}

int OnuMgt_GetEthPortAdminStatus(short int olt_id, short int onu_id, int port_id, int *enable)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortAdminStatus, (olt_id, onu_id, port_id, enable));

	return iRlt;
}

int OnuMgt_SetEthPortAdminStatus(short int olt_id, short int onu_id, int port_id, int enable)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL(olt_id, onu_id, SetEthPortAdminStatus, (olt_id, onu_id, port_id, enable));
    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_enable, enable);
    }

	return iRlt;
}

int OnuMgt_GetEthPortPause(short int olt_id, short int onu_id, int port_id, int *enable)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortPause, (olt_id, onu_id, port_id, enable));

	return iRlt;
}

int OnuMgt_SetEthPortPause(short int olt_id, short int onu_id, int port_id, int enable)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL(olt_id, onu_id, SetEthPortPause, (olt_id, onu_id, port_id, enable));
    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_pause_enable, enable);
    }

	return iRlt;
}

int OnuMgt_GetEthPortAutoNegotiationAdmin(short int olt_id, short int onu_id, int port_id, int *an)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortAutoNegotiation, (olt_id, onu_id, port_id, an));

	return iRlt;
}

int OnuMgt_SetEthPortAutoNegotiationAdmin(short int olt_id, short int onu_id, int port_id, int an)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetEthPortAutoNegotiation, (olt_id, onu_id, port_id, an));

	return iRlt;
}

int OnuMgt_SetEthPortAutoNegotiationRestart(short int olt_id, short int onu_id, int port_id)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetEthPortAutoNegotiationRestart, (olt_id, onu_id, port_id));

	return iRlt;
}

int OnuMgt_GetEthPortAnLocalTecAbility(short int olt_id, short int onu_id, int port_id, CTC_STACK_auto_negotiation_technology_ability_t *ability)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortAnLocalTecAbility, (olt_id, onu_id, port_id, ability));

    return iRlt;
}

int OnuMgt_GetEthPortAnAdvertisedTecAbility(short int olt_id, short int onu_id, int port_id, CTC_STACK_auto_negotiation_technology_ability_t *ability)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortAnAdvertisedTecAbility, (olt_id, onu_id, port_id, ability));

	return iRlt;
}

int OnuMgt_GetEthPortPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_policing_entry_t *policing)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortPolicing, (olt_id, onu_id, port_id, policing));

	return iRlt;
}

int OnuMgt_SetEthPortPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_policing_entry_t *policing)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetEthPortPolicing, (olt_id, onu_id, port_id, policing));

	return iRlt;
}

int OnuMgt_GetEthPortDownstreamPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortDsPolicing, (olt_id, onu_id, port_id, policing));

	return iRlt;
}

int OnuMgt_SetEthPortDownstreamPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetEthPortDsPolicing, (olt_id, onu_id, port_id, policing));

	return iRlt;
}

int OnuMgt_GetEthPortVlanConfig(short int olt_id, short int onu_id,  int port_id, CTC_STACK_port_vlan_configuration_t * vconf)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortVlanConfig, (olt_id, onu_id, port_id, vconf));

	return iRlt;
}

int OnuMgt_SetEthPortVlanConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_port_vlan_configuration_t *vconf)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetEthPortVlanConfig, (olt_id, onu_id, port_id, vconf));

	return iRlt;
}

int OnuMgt_GetAllPortVlanConfig(short int olt_id, short int onu_id, unsigned char *number_of_entries, CTC_STACK_vlan_configuration_ports_t ports_info)
{
	int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    
	OLT_ASSERT(olt_id);
	
	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetAllPortVlanConfig, (olt_id, onu_id, number_of_entries,ports_info));

	return iRlt;
}

int OnuMgt_GetEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id, CTC_STACK_classification_rules_t cam )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortClassificationAndMark, (olt_id, onu_id, port_id, cam));

    return iRlt;
}

int OnuMgt_SetEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id, int mode, CTC_STACK_classification_rules_t cam)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetEthPortClassificationAndMark, (olt_id, onu_id, port_id, mode, cam));

    return iRlt;
}

int OnuMgt_ClearEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, ClearEthPortClassificationAndMarking, (olt_id, onu_id, port_id));

    return iRlt;
}

int OnuMgt_GetEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id,	CTC_STACK_multicast_vlan_t *mv )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortMulticatVlan, (olt_id, onu_id, port_id, mv));

    return iRlt;
}

int OnuMgt_SetEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id, CTC_STACK_multicast_vlan_t *mv)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetEthPortMulticatVlan, (olt_id, onu_id, port_id, mv));

    return iRlt;
}

int OnuMgt_ClearEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, ClearEthPortMulticastVlan, (olt_id, onu_id, port_id));

    return iRlt;
}

int OnuMgt_GetEthPortMulticastGroupMaxNumber (short int olt_id, short int onu_id, int port_id, int *num)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortMulticastGroupMaxNumber , (olt_id, onu_id, port_id, num));

	return iRlt;
}

int OnuMgt_SetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port_id, int num)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetEthPortMulticastGroupMaxNumber , (olt_id, onu_id, port_id, num));
    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_igmp_max_group, num);
    }

	return iRlt;
}

int OnuMgt_GetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int *tag)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortMulticastTagStrip, (olt_id, onu_id, port_id, tag));

	return iRlt;
}

int OnuMgt_SetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int tag)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetEthPortMulticastTagStrip, (olt_id, onu_id, port_id, tag));
    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_igmp_tag_strip, tag);
    }

	return iRlt;
}

int OnuMgt_GetAllPortMulticastTagStrip ( short int olt_id, short int onu_id, unsigned char *number_of_entries, CTC_STACK_multicast_ports_tag_strip_t ports_info )
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    
	OLT_ASSERT(olt_id);
	
	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetAllPortMulticastTagStrip, (olt_id, onu_id, number_of_entries,ports_info));

	return iRlt;
    
}

int OnuMgt_GetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t *oper, CTC_STACK_multicast_vlan_switching_t *sw)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetEthPortMulticastTagOper, (olt_id, onu_id, port_id, oper, sw));

	return iRlt;
}

int OnuMgt_SetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t oper, CTC_STACK_multicast_vlan_switching_t *sw)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetEthPortMulticastTagOper, (olt_id, onu_id, port_id, oper, sw));

	return iRlt;
}

int OnuMgt_SetObjMulticastTagOper ( short int olt_id,short int onu_id, CTC_management_object_t *management_object, CTC_STACK_tag_oper_t tag_oper, CTC_STACK_multicast_vlan_switching_t *sw)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    
	OLT_ASSERT(olt_id);
	
	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetObjMulticastTagOper, (olt_id, onu_id, management_object, tag_oper, sw));

	return iRlt;
    
}

int OnuMgt_GetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetMulticastControl, (olt_id, onu_id, mc));

    return iRlt;
}

int OnuMgt_SetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetMulticastControl, (olt_id, onu_id, mc));

    return iRlt;
}

int OnuMgt_ClearMulticastControl(short int olt_id, short int onu_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, ClearMulticastControl, (olt_id, onu_id));

    return iRlt;
}

int OnuMgt_GetMulticastSwitch(short int olt_id, short int onu_id, int *sw)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetMulticatSwitch, (olt_id, onu_id, sw));

	return iRlt;
}

int OnuMgt_SetMulticastSwitch(short int olt_id, short int onu_id, int sw)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetMulticatSwitch, (olt_id, onu_id, sw));

	return iRlt;
}

int OnuMgt_GetMulticastFastleaveAbility(short int olt_id, short int onu_id, CTC_STACK_fast_leave_ability_t *ability)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetMulticastFastLeaveAbility, (olt_id, onu_id, ability));

	return iRlt;
}

int OnuMgt_GetMulticastFastleave(short int olt_id, short int onu_id, int *fl)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetMulticastFastLeave, (olt_id, onu_id, fl));

	return iRlt;
}

int OnuMgt_SetMulticastFastleave(short int olt_id, short int onu_id, int fl)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetMulticastFastLeave, (olt_id, onu_id, fl));

	if(iRlt == VOS_OK)
	    setOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_fastleave_enable, (fl ==CTC_STACK_FAST_LEAVE_ADMIN_STATE_ENABLED)?1:0);

	return iRlt;
}


/*获取ONU端口的统计数据*/
int OnuMgt_GetOnuPortStatisticData(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_data_t *data)
{
	int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(data);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetOnuPortStatisticData, (olt_id, onu_id, port_id, data));

    return iRlt;
}

int OnuMgt_GetCTCOnuPortStatsData(short int olt_id, short int onu_id, int port_id, OnuPortStats_ST *data)
{
	int iRlt;

	OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(data);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetOnuPortStatsData, (olt_id, onu_id, port_id, data));

    return iRlt;
}


/*获取ONU端口的统计状态*/
int OnuMgt_GetOnuPortStatisticState(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(state);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetOnuPortStatisticState, (olt_id, onu_id, port_id, state));

    return iRlt;
}

/*设置ONU端口的统计状态*/
int OnuMgt_SetOnuPortStatisticState(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetOnuPortStatisticState, (olt_id, onu_id, port_id, state));

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
int OnuMgt_SetAlarmAdminState(short int olt_id, short int onu_id,  CTC_management_object_t *management_object,
												 CTC_STACK_alarm_id_t alarm_id, bool enable)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetObjAlarmAdminState, (olt_id, onu_id, management_object, alarm_id, enable));

    return iRlt;
}

int OnuMgt_SetAlarmThreshold (short int olt_id, short int onu_id, CTC_management_object_t *management_object,
			CTC_STACK_alarm_id_t alarm_id, unsigned long alarm_threshold, unsigned long	clear_threshold )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetObjAlarmThreshold, (olt_id, onu_id, management_object, alarm_id, alarm_threshold,clear_threshold));

	return iRlt;
}

int OnuMgt_GetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetDbaReportThreshold, (olt_id, onu_id, number_of_queue_sets,queues_sets_thresholds));

	return iRlt;
}

int OnuMgt_SetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetDbaReportThreshold, (olt_id, onu_id, number_of_queue_sets,queues_sets_thresholds));

	return iRlt;
}


int OnuMgt_GetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetMngGlobalConfig, (olt_id, onu_id, mxu_mng));
    
    return iRlt;
}

int OnuMgt_SetMxuMngGlobalConfig(short int olt_id, short int onu_id, RPC_CTC_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt;
    short int olt_id_swap = 0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL(olt_id, onu_id, SetMngGlobalConfig, (olt_id, onu_id, mxu_mng));
    /*begin: 支持管理IP的配置保存，mod by liuyh, 2017-5-4*/
    if(mxu_mng->needSaveOlt && (iRlt == VOS_OK))
    {
        int onuMgtIdx = olt_id * MAXONUPERPON + onu_id;
        
        ONU_MGMT_SEM_TAKE;
    	OnuMgmtTable[onuMgtIdx].mngIp.ip = mxu_mng->mxu_mng.mng_ip;
        OnuMgmtTable[onuMgtIdx].mngIp.mask = mxu_mng->mxu_mng.mng_mask;
        OnuMgmtTable[onuMgtIdx].mngIp.gw = mxu_mng->mxu_mng.mng_gw;
        OnuMgmtTable[onuMgtIdx].mngIp.cVlan = mxu_mng->mxu_mng.data_cvlan;
        OnuMgmtTable[onuMgtIdx].mngIp.sVlan = mxu_mng->mxu_mng.data_svlan;
        OnuMgmtTable[onuMgtIdx].mngIp.pri = mxu_mng->mxu_mng.data_priority;
    	ONU_MGMT_SEM_GIVE;

        ONUMGT_PARTNER_CALL_BEGIN
		iRlt = OnuMgt_SetMxuMngGlobalConfig(olt_id_swap, onu_id, mxu_mng);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "Partner Call OnuMgt_SetMxuMngGlobalConfig(%d, %d) "
                "Error Rlt(%d).", olt_id_swap, onu_id, iRlt);
			iRlt = OLT_ERR_OK;
		}
		ONUMGT_PARTNER_CALL_END
    }
    else if (iRlt != VOS_OK)
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetMxuMngGlobalConfig(%d, %d) "
                "Error Rlt(%d).", olt_id, onu_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }        
    }
    /*end: mod by liuyh, 2017-5-4*/
    
    return iRlt;
}

int OnuMgt_GetMxuMngSnmpConfig ( short int olt_id, short int onu_id, CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter)
{
    int iRlt;
    
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetMngSnmpConfig, (olt_id, onu_id, parameter));

	return iRlt;
}

int OnuMgt_SetMxuMngSnmpConfig(short int olt_id, short int onu_id,CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetMngSnmpConfig, (olt_id, onu_id, parameter));

	return iRlt;
}
    

int OnuMgt_GetHoldOver(short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(holdover);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetHoldOver, (olt_id, onu_id, holdover));

    return iRlt;
}

int OnuMgt_SetHoldOver(short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL(olt_id, onu_id, SetHoldOver, (olt_id, onu_id, holdover));
    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_onu_holdover_time, holdover->holdover_time);
    }

    return iRlt;
}

int OnuMgt_GetOptTransDiag ( short int olt_id, short int onu_id,
		   CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis )
{
    int iRlt;
    
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetOptTransDiag, (olt_id, onu_id, optical_transceiver_diagnosis));

	return iRlt;
}

int OnuMgt_SetTxPowerSupplyControl(short int olt_id, short int onu_id,  CTC_STACK_onu_tx_power_supply_control_t *parameter)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetTxPowerSupplyControl, (olt_id, onu_id, parameter));

    return iRlt;
}

int OnuMgt_GetFecAbility(short int olt_id, short int onu_id,  CTC_STACK_standard_FEC_ability_t  *fec_ability)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetFecAbility, (olt_id, onu_id, fec_ability));

    return iRlt;
}
/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
int OnuMgt_SetQosRuleMode(short int olt_id, short int onu_id, int direct, unsigned char mode)
{
    int iRlt,i = 0;
    int mode1 = 0;
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetQosRuleMode, (olt_id, onu_id, direct, mode));
    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = SetOnuConf_Qos_Mode(olt_id, onu_id, direct, mode);
    }
    return iRlt;
}
int OnuMgt_SetRule(short int olt_id, short int onu_id, int direct, int code, gw_rule_t rule)
{
    int iRlt,i = 0;
    int mode1 = 0;
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetRule, (olt_id, onu_id, direct, code, rule));
    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        switch(code)
        {
            case SET_QOS_RULE:
                iRlt = SetOnuConf_Qos_Rule(olt_id, onu_id, direct, rule.qos_rule);
                break;
            case CLR_QOS_RULE:
                iRlt = ClrOnuConf_Qos_Rule(olt_id, onu_id, direct, rule.qos_rule);
                break;
            case SET_PAS_RULE:
                iRlt = SetOnuConf_Pas_Rule(olt_id, onu_id, direct, rule.pas_rule);
                break;
            case CLR_PAS_RULE:
                iRlt = ClrOnuConf_Pas_Rule(olt_id, onu_id, direct, rule.pas_rule);
                break;
            default:
                iRlt = VOS_ERROR;
                break;
        }
    }
    return iRlt;
}

int OnuMgt_GetAllEthPortMacCounter(short int olt_id, short int onu_id, OnuEthPortCounter_t *data)
{
    int iRlt,i = 0;
    int mode1 = 0;
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetAllEthPortMacCounter, (olt_id, onu_id, data));
    return iRlt;
}

#if 1
int OnuMgt_GetIADInfo(short int olt_id, short int onu_id, CTC_STACK_voip_iad_info_t* iad_info)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetIADInfo, (olt_id, onu_id, iad_info));    
	return iRlt;
}

/*H.248协议下IAD的运行状态*/
int OnuMgt_GetVoipIadOperStatus(short int olt_id, short int onu_id, CTC_STACK_voip_iad_oper_status_t *iad_oper_status)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetVoipIadOperation, (olt_id, onu_id, iad_oper_status));
	return iRlt;
}

int OnuMgt_SetVoipIadOperation(short int olt_id, short int onu_id, CTC_STACK_operation_type_t operation_type)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetVoipIadOperation, (olt_id, onu_id, operation_type));
	return iRlt;
}


/*语音模块全局参数配置*/
int OnuMgt_GetVoipGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_voip_global_param_conf_t *global_param)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetVoipGlobalConfig, (olt_id, onu_id, global_param));   
	return iRlt;
}

int OnuMgt_SetVoipGlobalConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_global_param_conf_t *global_param)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetVoipGlobalConfig, (olt_id, onu_id, code, global_param));
	return iRlt;
}

int OnuMgt_GetVoipFaxConfig(short int olt_id, short int onu_id, CTC_STACK_voip_fax_config_t *voip_fax)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetVoipFaxConfig, (olt_id, onu_id, voip_fax));
	return iRlt;
}

int OnuMgt_SetVoipFaxConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_fax_config_t *voip_fax)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetVoipFaxConfig, (olt_id, onu_id, code, voip_fax));
	return iRlt;
}


int OnuMgt_GetVoipPortStatus(short int olt_id, short int onu_id, int port_id, CTC_STACK_voip_pots_status_array *pots_status_array)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetVoipPotsStatus, (olt_id, onu_id, port_id, pots_status_array));    
	return iRlt;
}

int OnuMgt_GetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t *port_state)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetVoipPort, (olt_id, onu_id, port_id, port_state));
	return iRlt;    
}

int OnuMgt_SetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t port_state)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetVoipPort, (olt_id, onu_id, port_id, port_state));
    
	return iRlt;    
}

int OnuMgt_GetVoipPort2(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t *port_state)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetVoipPort2, (olt_id, onu_id, slot, port, port_state));
	return iRlt;
}

int OnuMgt_SetVoipPort2(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t port_state)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetVoipPort2, (olt_id, onu_id, slot, port, port_state));
	return iRlt;
}


int OnuMgt_GetH248Config(short int olt_id, short int onu_id, CTC_STACK_h248_param_config_t *h248_param)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetH248Config, (olt_id, onu_id, h248_param));
	return iRlt;
}

int OnuMgt_SetH248Config(short int olt_id, short int onu_id, int code, CTC_STACK_h248_param_config_t *h248_param)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetH248Config, (olt_id, onu_id, code, h248_param));
	return iRlt;
}


int OnuMgt_GetH248UserTidConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_h248_user_tid_config_array *h248_user_tid_array)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetH248UserTidConfig, (olt_id, onu_id, port_id, h248_user_tid_array));
	return iRlt;
}

int OnuMgt_SetH248UserTidConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_h248_user_tid_config_t *user_tid_config)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetH248UserTidConfig, (olt_id, onu_id, port_id, user_tid_config));
	return iRlt;
}


int OnuMgt_GetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_info_t *h248_rtp_tid_info)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetH248RtpTidConfig, (olt_id, onu_id, h248_rtp_tid_info));
	return iRlt;
}

int OnuMgt_SetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_config_t *h248_rtp_tid_info)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetH248RtpTidConfig, (olt_id, onu_id, h248_rtp_tid_info));
	return iRlt;
}


int OnuMgt_GetSipConfig(short int olt_id, short int onu_id, CTC_STACK_sip_param_config_t *sip_param)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetSipConfig, (olt_id, onu_id, sip_param));
	return iRlt;
}

int OnuMgt_SetSipConfig(short int olt_id, short int onu_id, int code, CTC_STACK_sip_param_config_t *sip_param)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetSipConfig, (olt_id, onu_id, code, sip_param));
	return iRlt;
}

int OnuMgt_SetSipDigitMap(short int olt_id, short int onu_id, CTC_STACK_SIP_digit_map_t *sip_digit_map)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetSipDigitMap, (olt_id, onu_id, sip_digit_map));
	return iRlt;
}


int OnuMgt_GetSipUserConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_sip_user_param_config_array *sip_user_param_array)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, GetSipUserConfig, (olt_id, onu_id, port_id, sip_user_param_array));
	return iRlt;
}

int OnuMgt_SetSipUserConfig(short int olt_id, short int onu_id, int port_id, int code, CTC_STACK_sip_user_param_config_t *sip_user_param)
{
	int iRlt;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetSipUserConfig, (olt_id, onu_id, port_id, code, sip_user_param));
	return iRlt;
}
#endif

#endif


#if 1
/* -------------------ONU 远程管理API------------------- */

int OnuMgt_CliCall(short int olt_id, short int onu_id, char *cli_str, const int cli_len, char **rlt_str, unsigned short int *rlt_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, CliCall, (olt_id, onu_id, cli_str, cli_len, rlt_str, rlt_len));

    return iRlt;
}

int OnuMgt_SetMgtReset(short int olt_id, short int onu_id, int lv)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetMgtReset, (olt_id, onu_id, lv));

    return iRlt;
}

int OnuMgt_SetMgtConfig(short int olt_id, short int onu_id, int lv)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetMgtConfig, (olt_id, onu_id, lv));

    return iRlt;
}

int OnuMgt_SetMgtLaser(short int olt_id, short int onu_id, int lv)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetMgtLaser, (olt_id, onu_id, lv));

    return iRlt;
}

int OnuMgt_SetTemperature(short int olt_id, short int onu_id, int temp)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetTemperature, (olt_id, onu_id, temp));

    return iRlt;
}

int OnuMgt_SetPasFlush(short int olt_id, short int onu_id, int act)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPasFlush, (olt_id, onu_id, act));

    return iRlt;
}

int OnuMgt_SetAtuAgingTime(short int olt_id, short int onu_id, int aging)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetAtuAgingTime, (olt_id, onu_id, aging));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_atu_aging, aging);
    }

    return iRlt;
}

int OnuMgt_SetAtuLimit(short int olt_id, short int onu_id, int limit)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetAtuLimit, (olt_id, onu_id, limit));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_atu_limit, limit);
    }

    return iRlt;
}

int OnuMgt_SetPortLinkMon(short int olt_id, short int onu_id, int mon)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortLinkMon, (olt_id, onu_id, mon));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_port_linkmon_enable, mon);
    }

    return iRlt;
}

int OnuMgt_SetPortModeMon(short int olt_id, short int onu_id, int mon)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortModeMon, (olt_id, onu_id, mon));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_port_modemon_enable, mon);
    }

    return iRlt;
}

int OnuMgt_SetPortIsolate(short int olt_id, short int onu_id, int enable)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortIsolate, (olt_id, onu_id, enable));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_port_isolate, enable);
    }

    return iRlt;
}

int OnuMgt_SetVlanEnable(short int olt_id, short int onu_id, int enable)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetVlanEnable, (olt_id, onu_id, enable));


    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        /*
         * mtodo:添加代码，将设置参数写入配置数据表
         */
    	int mode = enable?ONU_CONF_VLAN_MODE_TRUNK:ONU_CONF_VLAN_MODE_TRANSPARENT;
#if 0
    	int num = getOnuEthPortNum(olt_id, onu_id);
    	int i = 0;

    	for(i=1; i<=num; i++)
    	    /*iRlt |= OnuMgt_SetVlanMode(olt_id, onu_id, i, mode);*/
    	    SetOnuConfPortVlanMode(olt_id, onu_id, i, mode);

    	if(!num)
    	    /*iRlt = OnuMgt_SetVlanMode(olt_id, onu_id, 1, mode);*/
    	    SetOnuConfPortVlanMode(olt_id, onu_id, 1, mode);
#else
    	int mode_cur = getOnuConfVlanMode(olt_id, onu_id);

    	if(mode_cur != mode)
    	    setOnuConfVlanMode(olt_id, onu_id, mode);
#endif

    }

    return iRlt?VOS_ERROR:VOS_OK;
}

int OnuMgt_SetVlanMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetVlanMode, (olt_id, onu_id, port_id, mode));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        /*added by luh 2012-12-14*/
        if(mode&ONU_SEPCAL_FUNCTION)
        {
            ULONG enable = (mode&(~ONU_SEPCAL_FUNCTION)) == ONU_CONF_VLAN_MODE_TRANSPARENT?1:0;
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_onu_transparent_enable, enable);
            if(enable)
                OnuConfDelVlanPort(olt_id, onu_id, port_id);
        }
        else
        {
            int curmode = getOnuConfVlanMode(olt_id, onu_id);
            if(curmode != ONU_CONF_VLAN_MODE_UNKNOWN && curmode != mode)
                setOnuConfVlanMode(olt_id, onu_id, mode);    
            
            /*添加代码，将设置参数写入配置数据表*/
            iRlt = SetOnuConfPortVlanMode(olt_id, onu_id, port_id, mode);
            if(mode == ONU_CONF_VLAN_MODE_TRANSPARENT)
                iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_onu_transparent_enable, 1);
            else if(mode == ONU_CONF_VLAN_MODE_TRUNK)
                iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_onu_transparent_enable, 0);
        }
        
    }

    return iRlt;
}

int OnuMgt_AddVlan(short int olt_id, short int onu_id, int vid)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, AddVlan, (olt_id, onu_id, vid));
    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {

        ULONG all, untag;
        if(get_onuconf_vlanPortlist(olt_id, onu_id, vid, &all, &untag) != VOS_OK)
            iRlt = set_onuconf_vlanPortlist(olt_id, onu_id, vid, 0, 0);
        else
            iRlt = VOS_ERROR;
    }

    return iRlt;
}

int OnuMgt_DelVlan(short int olt_id, short int onu_id, int vid)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, DelVlan, (olt_id, onu_id, vid));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        ULONG allports = 0, untagports = 0;
        ULONG allportMask = 0,untagportMask = 0;
        if(get_onuconf_vlanPortlist(olt_id, onu_id, vid, &allports, &untagports) == VOS_OK)
        {
            get_onuconf_vlanPortlist(olt_id, onu_id, 1, &allportMask, &untagportMask);
            untagportMask |= untagports;
            allportMask |= untagports;
            set_onuconf_vlanPortlist(olt_id, onu_id, 1, allportMask, allportMask);    
        }
        
        iRlt = del_onuconf_vlan(olt_id, onu_id, vid);
    }

    return iRlt;
}

int OnuMgt_SetPortPvid(short int olt_id, short int onu_id, int port_id, int pvid)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortPvid, (olt_id, onu_id, port_id, pvid));
    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id,sv_enum_port_default_vid, pvid);
    }

    return iRlt;
}

int OnuMgt_AddVlanPort(short int olt_id, short int onu_id, int vid, int portmask, int tag)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, AddVlanPort, (olt_id, onu_id, vid, portmask, tag));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        /*
         * ntodo:添加代码，将设置参数写入配置数据表
         */
        ULONG allports,untagports;
        ULONG untagports_temp = 0;
        ULONG untagportMask = 0;
        ULONG allportMask = 0 ,untagportMask1 = 0;
        int i=0;

        if(get_onuconf_vlanPortlist(olt_id, onu_id, vid, &allports, &untagports) == VOS_OK)
        {
            allports |= portmask;

            if(tag == 2) /*untagg ports*/
            {
                untagports |= portmask;
                if(portmask)
                {
                     get_onuconf_vlanPortlist(olt_id, onu_id, 1, &allportMask, &untagportMask);
                     untagportMask &=(~portmask);
                     allportMask &= (~portmask);
                     set_onuconf_vlanPortlist(olt_id, onu_id, 1, allportMask, untagportMask);    
                }                
                while(portmask)
                {
                    if(portmask&1)
                    {
                        setOnuConfPortSimpleVar(olt_id, onu_id, i+1, sv_enum_port_default_vid, vid);
                    }
                    portmask >>= 1;
                    i++;
                }
            }
            else
            {
                untagports_temp = untagports;
                untagports &= ~portmask;
                untagportMask1 = untagports_temp&portmask;          /*从本vlan 中，untag方式中解脱出来的portlist要恢复到缺省vlan */
                /*modi by luh 2011-10-14*/
                if(untagportMask1)
                {
                     get_onuconf_vlanPortlist(olt_id, onu_id, 1, &allportMask, &untagportMask);
                     untagportMask |=untagportMask1;
                     allportMask |= untagportMask1;
                     set_onuconf_vlanPortlist(olt_id, onu_id, 1, allportMask, untagportMask);    
                }
                /* added by wangxiaoyu 2011-08-29
                            * 以tag方式加入VLAN，PVID恢复为默认的VLAN 1*/
                while(untagportMask1)
                {
                    if(untagportMask1&1)
                    {
                        setOnuConfPortSimpleVar(olt_id, onu_id, i+1, sv_enum_port_default_vid, 1);
                    }
                    untagportMask1 >>= 1;
                    i++;
                }
            }
            iRlt = set_onuconf_vlanPortlist(olt_id, onu_id, vid, allports, untagports);
        }
        else
            iRlt = VOS_ERROR;
    }

    return iRlt;
}

int OnuMgt_DelVlanPort(short int olt_id, short int onu_id, int vid, int portmask)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, DelVlanPort, (olt_id, onu_id, vid, portmask));
    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        /*
         * ntodo:添加代码，将设置参数写入配置数据表
         */
        ULONG all,untag, i=0, untag1, untag2 = 0;
        ULONG allportMask=0,untagportMask=0;
        if(get_onuconf_vlanPortlist(olt_id, onu_id, vid, &all, &untag) == VOS_OK)
        {
            untag1 = untag;     /* 缓存当前ONU  untagged 端口列表*/
            all &= ~portmask;   /* 当前ONU 需要恢复的all 端口列表*/
            untag &= ~portmask; /* 当前ONU 需要恢复的untagged 端口列表*/
            untag1 &= portmask;   /* 需要恢复到缺省VLAN 下的untagged 列表*/
            /*恢复到缺省VLAN*/
            if(untag1)
            {
                 get_onuconf_vlanPortlist(olt_id, onu_id, 1, &allportMask, &untagportMask);
                 untagportMask |=untag1;
                 allportMask |= untag1;
                 set_onuconf_vlanPortlist(olt_id, onu_id, 1, allportMask, untagportMask);    
            }
            /*删除untag端口时，将端口的默认VLAN ID设置为vlan 1*/
            while(untag1)
            {
                if(untag1&1)
                {
                    setOnuConfPortSimpleVar(olt_id, onu_id, i+1, sv_enum_port_default_vid, 1);
                }
                untag1 >>= 1;
                i++;
            }
            /*end*/
            /*配置当前VLAN*/
            iRlt = set_onuconf_vlanPortlist(olt_id, onu_id, vid, all, untag);
        }
        else
            iRlt = VOS_ERROR;
    }

    return iRlt;
}

/*vlan transation*/
int OnuMgt_SetEthPortVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid, ULONG newVid)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetEthPortVlanTran, (olt_id, onu_id, port_id, inVid,newVid));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        /*
         * mtodo:添加代码，将设置参数写入配置数据表
         */
        iRlt = set_OnuConf_Ctc_EthPortVlanTranNewVid(olt_id, onu_id, port_id, inVid,newVid);
    }

    return iRlt;
}

int OnuMgt_DelEthPortVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, DelEthPortVlanTran, (olt_id, onu_id, port_id, inVid));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        /*
         * mtodo:添加代码，将设置参数写入配置数据表
         */
        iRlt = del_OnuConf_Ctc_EthPortVlanTranNewVid(olt_id, onu_id, port_id, inVid);
    }

    return iRlt;
}

/*vlan aggregation*/
int OnuMgt_SetEthPortVlanAgg(short int olt_id, short int onu_id, int port_id, USHORT inVid[8], USHORT targetVid)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetEthPortVlanAgg, (olt_id, onu_id, port_id, inVid,targetVid));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = set_OnuConf_Ctc_EthPortVlanAgg(olt_id, onu_id, port_id, inVid,targetVid);
    }

    return iRlt;
}

int OnuMgt_DelEthPortVlanAgg(short int olt_id, short int onu_id, int port_id, ULONG targetVid)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, DelEthPortVlanAgg, (olt_id, onu_id, port_id, targetVid));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = del_OnuConf_Ctc_EthPortVlanAgg(olt_id, onu_id, port_id,targetVid);
    }

    return iRlt;
}

/*qinq enable*/
int OnuMgt_SetPortQinQEnable(short int olt_id,short int onu_id, int port_id, int enable )
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortQinQEnable, (olt_id, onu_id, port_id, enable));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = set_OnuVlanTagConfigEnable(olt_id, onu_id, port_id, enable);
    }
    else
        sys_console_printf("\n------OnuMgt_SetPortQinQEnable rpc call fail %d------\n", iRlt);
    
    return iRlt;
}

 /*qinq port add tag*/
int OnuMgt_AddQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG cvlan, ULONG svlan)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, AddQinQVlanTag,(olt_id, onu_id, port_id,cvlan,svlan));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = set_OnuVlanAddTag(olt_id, onu_id, port_id, cvlan,svlan);
    }
    else
        sys_console_printf("\n------OnuMgt_AddQinQVlanTag rpc call fail %d------\n", iRlt);
    
    return iRlt;
}    
 
 /*qinq port del tag*/
 int OnuMgt_DelQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG svlan)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, DelQinQVlanTag,(olt_id, onu_id, port_id, svlan));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = set_OnuVlanDelTag(olt_id, onu_id, port_id, svlan);
    }
    else
        sys_console_printf("\n------OnuMgt_DelQinQVlanTag rpc call fail %d------\n", iRlt);
    
    return iRlt;
}    


int OnuMgt_SetPortVlanFrameTypeAcc(short int olt_id, short int onu_id, int port_id, int acc)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortVlanFrameTypeAcc, (olt_id, onu_id, port_id, acc));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id,sv_enum_port_vlan_accept_type, acc);
    }

    return iRlt;
}

int OnuMgt_SetPortIngressVlanFilter(short int olt_id, short int onu_id, int port_id, int enable)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortIngressVlanFilter, (olt_id, onu_id, port_id, enable));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id,sv_enum_port_ingress_vlan_filter, enable);
    }

    return iRlt;
}


int OnuMgt_SetPortMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortMode, (olt_id, onu_id, port_id, mode));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_mode, mode);
    }

    return iRlt;
}
 
int OnuMgt_SetPortFcMode(short int olt_id, short int onu_id, int port_id, int fc)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortFcMode, (olt_id, onu_id, port_id, fc));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_fec_enable, fc);
    }

    return iRlt;
}

int OnuMgt_SetPortAtuLearn(short int olt_id, short int onu_id, int portlist, int enable)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortAtuLearn, (olt_id, onu_id, portlist, enable));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        int i = 0;
        while(portlist)
        {
            i++;
            if(portlist&1)
                iRlt |= setOnuConfPortSimpleVar(olt_id, onu_id, i, sv_enum_port_atu_learn_enable, enable);
            portlist >>= 1;
        }
    }

    return iRlt;
}

int OnuMgt_SetPortAtuFlood(short int olt_id, short int onu_id, int portlist, int enable)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortAtuFlood, (olt_id, onu_id, portlist, enable));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        int i=0;
        while(portlist)
        {
            i++;
            if(portlist&1)
                iRlt |= setOnuConfPortSimpleVar(olt_id, onu_id, i, sv_enum_port_atu_flood_enable, enable);
            portlist >>= 1;
        }
    }

    return iRlt;
}

int OnuMgt_SetPortLoopDetect(short int olt_id, short int onu_id, int portlist, int enable)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortLoopDetect, (olt_id, onu_id, portlist, enable));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        int i = 0;
        while(portlist)
        {
            i++;
            if(portlist&1)
                iRlt |= setOnuConfPortSimpleVar(olt_id, onu_id, i, sv_enum_port_loop_detect_enable, enable);
            portlist >>= 1;
        }

    }

    return iRlt;
}

int OnuMgt_SetPortStatFlush(short int olt_id, short int onu_id, int portlist, int enable)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortStatFlush, (olt_id, onu_id, portlist, enable));

    return iRlt;
}


int OnuMgt_SetIngressRateLimitBase(short int olt_id, short int onu_id, int uv)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetIngressRateLimitBase, (olt_id, onu_id, uv));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_ingressRateLimitBase, uv);
    }

    return iRlt;
}

int OnuMgt_SetPortIngressRate(short int olt_id, short int onu_id, int port_id, int ratetype, int rate, int action, int burstmode)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortIngressRate, (olt_id, onu_id, port_id, ratetype, rate, action, burstmode));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_ingress_rate_limit, rate);
		setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_ingress_rate_type, ratetype);
		setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_ingress_rate_action, action);
		setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_ingress_rate_burst, burstmode);
    }

    return iRlt;
}

int OnuMgt_SetPortEgressRate(short int olt_id, short int onu_id, int port_id, int rate)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortEgressRate, (olt_id, onu_id, port_id, rate));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_egress_limit, rate);
    }

    return iRlt;
}


int OnuMgt_SetQosClass(short int olt_id, short int onu_id, int qossetid,  int ruleid, int classid, int field, int oper, char *val, int len)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetQosClass, (olt_id, onu_id, qossetid, ruleid, classid, field, oper, val, len));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE1(olt_id, onu_id))
        setOnuConfQosClass(olt_id, onu_id, qossetid, ruleid, classid, field, oper, val, len);

    return iRlt;
}

int OnuMgt_ClrQosClass(short int olt_id, short int onu_id, int qossetid, int ruleid, int classid)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, ClrQosClass, (olt_id, onu_id, qossetid, ruleid, classid));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
        clrOnuConfQosClass(olt_id, onu_id, qossetid, ruleid, classid);

    return iRlt;
}

int OnuMgt_SetQosRule(short int olt_id, short int onu_id, int qossetid, int ruleid, int queue, int prio)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetQosRule, (olt_id, onu_id, qossetid, ruleid, queue, prio));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE1(olt_id, onu_id))
        setOnuConfQosRule(olt_id, onu_id, qossetid, ruleid, queue, prio);

    return iRlt;
}

int OnuMgt_ClrQosRule(short int olt_id, short int onu_id, int qossetid, int ruleid)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, ClrQosRule, (olt_id, onu_id, qossetid, ruleid));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
        clrOnuConfQosRule(olt_id, onu_id, qossetid, ruleid);

    return iRlt;
}


int OnuMgt_SetPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortQosRule, (olt_id, onu_id, port_id, qossetid));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE1(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_qoset_idx, qossetid);
    }

    return iRlt;
}

int OnuMgt_ClrPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, ClrPortQosRule, (olt_id, onu_id, port_id, qossetid));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_qoset_idx, qossetid);
    }

    return iRlt;
}

int OnuMgt_SetPortQosRuleType(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortQosRuleType, (olt_id, onu_id, port_id, mode));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_qoset_rule, mode);
    }

    return iRlt;
}


int OnuMgt_SetPortDefPriority(short int olt_id, short int onu_id, int port_id, int prio)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortDefPriority, (olt_id, onu_id, port_id, prio));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_default_priority, prio);
    }

    return iRlt;
}

int OnuMgt_SetPortNewPriority(short int olt_id, short int onu_id, int port_id, int oldprio, int newprio)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortNewPriority, (olt_id, onu_id, port_id, oldprio, newprio));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfQosPrioReplace(olt_id, onu_id, port_id, oldprio,newprio);
    }

    return iRlt;
}

int OnuMgt_SetQosPrioToQueue(short int olt_id, short int onu_id, int prio, int queue)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetQosPrioToQueue, (olt_id, onu_id, prio, queue));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfQosPrioToQueue(olt_id, onu_id, prio,queue);
    }

    return iRlt;
}

int OnuMgt_SetQosDscpToQueue(short int olt_id, short int onu_id, int dscpnum, int queue)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetQosDscpToQueue, (olt_id, onu_id, dscpnum,queue));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfQosDscpToQueue(olt_id, onu_id,dscpnum,queue);
    }

    return iRlt;
}

int OnuMgt_SetPortUserPriorityEnable(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortUserPriorityEnable, (olt_id, onu_id, port_id, mode));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_qoset_user_enable, mode);
    }

    return iRlt;
}

int OnuMgt_SetPortIpPriorityEnable(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortIpPriorityEnable, (olt_id, onu_id, port_id, mode));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_qoset_ip_enable, mode);
    }

    return iRlt;
}

int OnuMgt_SetQosAlgorithm(short int olt_id, short int onu_id, int uv)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetQosAlgorithm, (olt_id, onu_id, uv));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_qosAlgorithm, uv);
    }

    return iRlt;
}

int OnuMgt_SetIgmpEnable(short int olt_id, short int onu_id, int en)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetIgmpEnable, (olt_id, onu_id, en));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_enable, en);
    }

    return iRlt;
}

int OnuMgt_SetIgmpAuth(short int olt_id, short int onu_id, int en)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetIgmpAuth, (olt_id, onu_id, en));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        setOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_auth_enable, en);
    }

    return iRlt;
}


int OnuMgt_SetIgmpHostAge(short int olt_id, short int onu_id, int age)
{
    int iRlt;
    ULONG groupage = 0;
    ULONG maxresponse = 0;
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetIgmpHostAge, (olt_id, onu_id, age));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_hostage,age);
        if(getOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_max_response_time, &maxresponse) == VOS_OK &&
            getOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_groupage, &groupage) == VOS_OK)
        {
            groupage = age - maxresponse;
            iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_groupage,groupage);
        }

    }

    return iRlt;
}

int OnuMgt_SetIgmpGroupAge(short int olt_id, short int onu_id, int age)
{
    int iRlt;
    ULONG hostage = 0;
    ULONG maxresponse = 0;
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetIgmpGroupAge, (olt_id, onu_id, age));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_groupage, age);
        if(getOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_max_response_time, &maxresponse) == VOS_OK &&
            getOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_hostage, &hostage) == VOS_OK)
        {
            hostage = age + maxresponse;
            iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_hostage,hostage);
        }    
    }

    return iRlt;
}

int OnuMgt_SetIgmpMaxResTime(short int olt_id, short int onu_id, int tm)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetIgmpMaxResTime, (olt_id, onu_id, tm));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_max_response_time, tm);
    }

    return iRlt;
}

int OnuMgt_SetIgmpMaxGroup(short int olt_id, short int onu_id, int number)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetIgmpMaxGroup, (olt_id, onu_id, number));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_igmp_max_group, number);
    }

    return iRlt;
}


int OnuMgt_AddIgmpGroup(short int olt_id, short int onu_id, int portlist, ULONG addr, ULONG vid)
{
    int iRlt = VOS_OK;
    int ret = 0,flag=0;
    ULONG addr1 = 0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, AddIgmpGroup, (olt_id, onu_id, portlist, addr, vid));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        for(ret=0;ret<ONU_MAX_IGMP_GROUP;ret++)
        {
           if(getOnuConf_igmp_groupdata(olt_id, onu_id,ret,sv_enum_igmp_group_gda,&addr1)==VOS_OK)
           {
              if(addr == addr1)
              {
                flag = 1;
                if(SetOnuConf_igmp_groupdata(olt_id, onu_id,ret,sv_enum_igmp_group_portmask,portlist)||
                    SetOnuConf_igmp_groupdata(olt_id, onu_id,ret,sv_enum_igmp_group_vlanId,vid))
                {
                    iRlt = VOS_ERROR;
                }
              }
           }
        }
        if(!flag)
        {
            for(ret=0;ret<ONU_MAX_IGMP_GROUP;ret++)
            {
                if(getOnuConf_igmp_groupdata(olt_id, onu_id,ret,sv_enum_igmp_group_gda,&addr1)==VOS_OK)
                {
                    if(0 == addr1)
                    {
                        if(SetOnuConf_igmp_groupdata(olt_id, onu_id,ret,sv_enum_igmp_group_gda,addr)||
                            SetOnuConf_igmp_groupdata(olt_id, onu_id,ret,sv_enum_igmp_group_portmask,portlist)||
                            SetOnuConf_igmp_groupdata(olt_id, onu_id,ret,sv_enum_igmp_group_vlanId,vid))
                        {
                            iRlt = VOS_ERROR;
                        }
                        break;
                    }
                    else
                        continue;
                }
            } 
        }
    }

    return iRlt;
}

int OnuMgt_DeleteIgmpGroup(short int olt_id, short int onu_id, ULONG addr)
{
    int iRlt = VOS_OK;
    int ret = 0,flag=0;
    ULONG vid = 0;
    ULONG addr1 = 0;
    ULONG port = 0;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, DeleteIgmpGroup, (olt_id, onu_id, addr));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        for(ret=0;ret<ONU_MAX_IGMP_GROUP;ret++)
        {
           if(getOnuConf_igmp_groupdata(olt_id, onu_id,ret,sv_enum_igmp_group_gda,&addr1)==VOS_OK)
           {
              if(addr == addr1)
              {
                flag = 1;
                addr1 = 0;
                if(SetOnuConf_igmp_groupdata(olt_id, onu_id,ret,sv_enum_igmp_group_gda,addr1)||
                    SetOnuConf_igmp_groupdata(olt_id, onu_id,ret,sv_enum_igmp_group_portmask,port)||
                    SetOnuConf_igmp_groupdata(olt_id, onu_id,ret,sv_enum_igmp_group_vlanId,vid))
                    iRlt = VOS_ERROR;
        		else
        		    iRlt = VOS_OK;
              }
           }
        }
        if(!flag)
        {
             iRlt = VOS_ERROR;
        }
    }

    return iRlt;
}

int OnuMgt_SetPortIgmpFastLeave(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortIgmpFastLeave, (olt_id, onu_id, port_id, mode));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_igmp_fastleave_enable, mode);
    }

    return iRlt;
}

int OnuMgt_SetPortMulticastVlan(short int olt_id, short int onu_id, int port_id, int vid)
{
    int iRlt = VOS_OK;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortMulticastVlan, (olt_id, onu_id, port_id, vid));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        iRlt = setOnuConfPortMulticastVlan(olt_id, onu_id, port_id, vid);
    }

    return iRlt;
}


int OnuMgt_SetPortMirrorFrom(short int olt_id, short int onu_id, int port_id, int mode, int type)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortMirrorFrom, (olt_id, onu_id, port_id, mode,type));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
       switch(type)
       {
        case 1:
             if(setOnuConfSimpleVar(olt_id, onu_id, sv_enum_port_mirror_ingress_fromlist, port_id))
                iRlt = 1;
             break;
        case 2:
             if(setOnuConfSimpleVar(olt_id,onu_id, sv_enum_port_mirror_egress_fromlist, port_id))
                iRlt = 2;
             break;
        case 3:
             if(setOnuConfSimpleVar(olt_id, onu_id, sv_enum_port_mirror_ingress_fromlist, port_id)||
                     setOnuConfSimpleVar(olt_id, onu_id, sv_enum_port_mirror_egress_fromlist, port_id))
                iRlt = VOS_ERROR;
             break; 
        default:
                iRlt = VOS_ERROR;
                break;
       }
    }

    return iRlt;
}

int OnuMgt_SetPortMirrorTo(short int olt_id, short int onu_id, int port_id, int type)
{
    int iRlt;
    
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, SetPortMirrorTo, (olt_id, onu_id, port_id,type));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
       switch(type)
       {
        case 1:
             if(setOnuConfSimpleVar(olt_id, onu_id,  sv_enum_port_mirror_ingress_tolist, port_id))
                iRlt = 1;
             break;
        case 2:
             if(setOnuConfSimpleVar(olt_id, onu_id, sv_enum_port_mirror_egress_tolist, port_id))
                iRlt = 2;
             break;
        default:
             iRlt = VOS_ERROR;
             break;
       }
    }

    return iRlt;
}

int OnuMgt_DeleteMirror(short int olt_id, short int onu_id, int type)
{
    int iRlt;
    int mode1 = 0;
    
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONUMGT_API_CALL2(olt_id, onu_id, DeleteMirror, (olt_id, onu_id, type));

    if((iRlt == VOS_OK) && CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id))
    {
        switch(type)
        {
            case 1:
    		{
    			if(setOnuConfSimpleVar(olt_id, onu_id,  sv_enum_port_mirror_ingress_tolist, mode1) )
    			    iRlt = 1;
            }
            break;
            case 2:
    		{
    			if(setOnuConfSimpleVar(olt_id, onu_id, sv_enum_port_mirror_egress_tolist, mode1) )
    			    iRlt = 2;
            }
            break;
            default:
                iRlt = VOS_ERROR; 
                break;
        }
        
    }

    return iRlt;
}

#endif



#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */

#if 1
/* --------------------CMC管理API------------------- */
int OnuMgt_RegisterCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, RegisterCmc, (olt_id, onu_id, cmc_mac) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_RegisterCmc(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_UnregisterCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, UnregisterCmc, (olt_id, onu_id, cmc_mac) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "UnregisterCmc(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_DumpCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmc, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmc(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcAlarms(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcAlarms, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcAlarms(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcLogs(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcLogs, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcLogs(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_ResetCmcBoard(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, ResetCmcBoard, (olt_id, onu_id, cmc_mac) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_ResetCmcBoard(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_GetCmcVersion(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *version, unsigned char *len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(version);
    VOS_ASSERT(len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcVersion, (olt_id, onu_id, cmc_mac, version, len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcVersion(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcMaxMulticasts(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *max_multicasts)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(max_multicasts);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcMaxMulticasts, (olt_id, onu_id, cmc_mac, max_multicasts) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcMaxMulticasts(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcMaxCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *max_cm)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(max_cm);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcMaxCm, (olt_id, onu_id, cmc_mac, max_cm) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcMaxCm(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcMaxCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short max_cm)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcMaxCm, (olt_id, onu_id, cmc_mac, max_cm) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfSimpleVar(olt_id, onu_id, sv_enum_cmc_max_cm, (ULONG)max_cm);
        }
#endif        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcMaxCm(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, struct tm *time)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(time);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcTime, (olt_id, onu_id, cmc_mac, time) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcTime(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, struct tm *time)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(time);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcTime, (olt_id, onu_id, cmc_mac, time) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcTime(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_LocalCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, LocalCmcTime, (olt_id, onu_id, cmc_mac) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_LocalCmcTime(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_SetCmcCustomConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char cfg_id, unsigned char *cfg_data, unsigned short data_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcCustomConfig, (olt_id, onu_id, cmc_mac, cfg_id, cfg_data, data_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcCustomConfig(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcCustomConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char cfg_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcCustomConfig, (olt_id, onu_id, cmc_mac, cfg_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcCustomConfig(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

#endif


#if 1
/* --------------------CMC频道管理API------------------- */
int OnuMgt_DumpCmcDownChannel(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcDownChannel, (olt_id, onu_id, cmc_mac, channel_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcDownChannel(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcUpChannel(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcUpChannel, (olt_id, onu_id, cmc_mac, channel_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcUpChannel(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcDownChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(channel_mode);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcDownChannelMode, (olt_id, onu_id, cmc_mac, channel_id, channel_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcDownChannelMode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcDownChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcDownChannelMode, (olt_id, onu_id, cmc_mac, channel_id, channel_mode) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_DOWNCHANNEL2PORT(channel_id), sv_enum_cmc_channel_enable, (ULONG)channel_mode);
        }
#endif
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcDownChannelMode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_GetCmcUpChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(channel_mode);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcUpChannelMode, (olt_id, onu_id, cmc_mac, channel_id, channel_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcUpChannelMode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcUpChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcUpChannelMode, (olt_id, onu_id, cmc_mac, channel_id, channel_mode) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_UPCHANNEL2PORT(channel_id), sv_enum_cmc_channel_enable, (ULONG)channel_mode);
        }
#endif
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcUpChannelMode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcUpChannelD30Mode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *docsis30_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(docsis30_mode);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcUpChannelD30Mode, (olt_id, onu_id, cmc_mac, channel_id, docsis30_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcUpChannelD30Mode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcUpChannelD30Mode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char docsis30_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcUpChannelD30Mode, (olt_id, onu_id, cmc_mac, channel_id, docsis30_mode) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_UPCHANNEL2PORT(channel_id), sv_enum_cmc_channel_d30, (ULONG)docsis30_mode);
        }
#endif
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcUpChannelD30Mode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcDownChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_freq)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(channel_freq);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcDownChannelFreq, (olt_id, onu_id, cmc_mac, channel_id, channel_freq) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcDownChannelAnnexMode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcDownChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_freq)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcDownChannelFreq, (olt_id, onu_id, cmc_mac, channel_id, channel_freq) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_DOWNCHANNEL2PORT(channel_id), sv_enum_cmc_channel_freq, (ULONG)channel_freq);
        }
#endif
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcDownChannelAnnexMode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_GetCmcUpChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_freq)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(channel_freq);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcUpChannelFreq, (olt_id, onu_id, cmc_mac, channel_id, channel_freq) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcUpChannelFreq(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcUpChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_freq)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcUpChannelFreq, (olt_id, onu_id, cmc_mac, channel_id, channel_freq) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_UPCHANNEL2PORT(channel_id), sv_enum_cmc_channel_freq, (ULONG)channel_freq);
        }
#endif
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcUpChannelFreq(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcDownAutoFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long *base_freq, unsigned long *step_freq, unsigned char *step_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(base_freq);
    VOS_ASSERT(step_freq);
    VOS_ASSERT(step_mode);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcDownAutoFreq, (olt_id, onu_id, cmc_mac, base_freq, step_freq, step_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcDownAutoFreq(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcDownAutoFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long base_freq, unsigned long step_freq, unsigned char step_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcDownAutoFreq, (olt_id, onu_id, cmc_mac, base_freq, step_freq, step_mode) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            ULONG ulValues[3];
            
            ulValues[0] = base_freq;
            ulValues[1] = step_freq;
            ulValues[2] = step_mode;
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_DOWNCHANNEL2PORT(CMC_CHANNELID_ALL), sv_enum_cmc_channel_freq, (ULONG)ulValues);
        }
#endif
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcDownAutoFreq(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_GetCmcUpAutoFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long *base_freq, unsigned long *step_freq, unsigned char *step_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(base_freq);
    VOS_ASSERT(step_freq);
    VOS_ASSERT(step_mode);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcUpAutoFreq, (olt_id, onu_id, cmc_mac, base_freq, step_freq, step_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcUpAutoFreq(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcUpAutoFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long base_freq, unsigned long step_freq, unsigned char step_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcUpAutoFreq, (olt_id, onu_id, cmc_mac, base_freq, step_freq, step_mode) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            ULONG ulValues[3];
            
            ulValues[0] = base_freq;
            ulValues[1] = step_freq;
            ulValues[2] = step_mode;
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_UPCHANNEL2PORT(CMC_CHANNELID_ALL), sv_enum_cmc_channel_freq, (ULONG)ulValues);
        }
#endif        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcUpAutoFreq(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcUpChannelWidth(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_width)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(channel_width);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcUpChannelWidth, (olt_id, onu_id, cmc_mac, channel_id, channel_width) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcUpChannelWidth(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcUpChannelWidth(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_width)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcUpChannelWidth, (olt_id, onu_id, cmc_mac, channel_id, channel_width) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_UPCHANNEL2PORT(channel_id), sv_enum_cmc_channel_width, (ULONG)channel_width);
        }
#endif        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcUpChannelWidth(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcDownChannelAnnexMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *annex_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(annex_mode);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcDownChannelAnnexMode, (olt_id, onu_id, cmc_mac, channel_id, annex_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcDownChannelAnnexMode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcDownChannelAnnexMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char annex_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcDownChannelAnnexMode, (olt_id, onu_id, cmc_mac, channel_id, annex_mode) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_DOWNCHANNEL2PORT(channel_id), sv_enum_cmc_channel_annex, (ULONG)annex_mode);
        }
#endif        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcDownChannelAnnexMode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcUpChannelType(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_type)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(channel_type);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcUpChannelType, (olt_id, onu_id, cmc_mac, channel_id, channel_type) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcUpChannelType(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcUpChannelType(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_type)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcUpChannelType, (olt_id, onu_id, cmc_mac, channel_id, channel_type) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_UPCHANNEL2PORT(channel_id), sv_enum_cmc_channel_type, (ULONG)channel_type);
        }
#endif        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcUpChannelType(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcDownChannelModulation(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *modulation_type)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(modulation_type);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcDownChannelModulation, (olt_id, onu_id, cmc_mac, channel_id, modulation_type) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcDownChannelModulation(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcDownChannelModulation(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char modulation_type)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcDownChannelModulation, (olt_id, onu_id, cmc_mac, channel_id, modulation_type) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_DOWNCHANNEL2PORT(channel_id), sv_enum_cmc_channel_modulation, (ULONG)modulation_type);
        }
#endif        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcDownChannelModulation(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcUpChannelProfile(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_profile)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(channel_profile);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcUpChannelProfile, (olt_id, onu_id, cmc_mac, channel_id, channel_profile) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcUpChannelProfile(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcUpChannelProfile(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_profile)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcUpChannelProfile, (olt_id, onu_id, cmc_mac, channel_id, channel_profile) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_UPCHANNEL2PORT(channel_id), sv_enum_cmc_channel_profile, (ULONG)channel_profile);
        }
#endif        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcUpChannelProfile(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcDownChannelInterleaver(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *interleaver)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(interleaver);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcDownChannelInterleaver, (olt_id, onu_id, cmc_mac, channel_id, interleaver) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcDownChannelInterleaver(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcDownChannelInterleaver(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char interleaver)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcDownChannelInterleaver, (olt_id, onu_id, cmc_mac, channel_id, interleaver) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_DOWNCHANNEL2PORT(channel_id), sv_enum_cmc_channel_interleave, (ULONG)interleaver);
        }
#endif
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcDownChannelInterleaver(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_GetCmcDownChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int *channel_power)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(channel_power);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcDownChannelPower, (olt_id, onu_id, cmc_mac, channel_id, channel_power) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcDownChannelPower(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcDownChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int channel_power)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcDownChannelPower, (olt_id, onu_id, cmc_mac, channel_id, channel_power) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_DOWNCHANNEL2PORT(channel_id), sv_enum_cmc_channel_power, (ULONG)channel_power);
        }
#endif
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcDownChannelPower(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_GetCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int *channel_power)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(channel_power);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcUpChannelPower, (olt_id, onu_id, cmc_mac, channel_id, channel_power) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcUpChannelPower(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int channel_power)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcUpChannelPower, (olt_id, onu_id, cmc_mac, channel_id, channel_power) )) )
    {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if( CHECK_ONU_LOCAL_CONF_ENABLE(olt_id, onu_id) )
        {
            iRlt = setOnuConfPortSimpleVar(olt_id, onu_id, (USHORT)CMC_UPCHANNEL2PORT(channel_id), sv_enum_cmc_channel_power, (ULONG)channel_power);
        }
#endif
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcUpChannelPower(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcUpChannelPower, (olt_id, onu_id, cmc_mac, channel_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcUpChannelPower(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_DumpCmcUpChannelSignalQuality(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcUpChannelSignalQuality, (olt_id, onu_id, cmc_mac, channel_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcUpChannelSignalQuality(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_DumpCmcInterfaceUtilization(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_type, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcInterfaceUtilization, (olt_id, onu_id, cmc_mac, channel_type, channel_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcInterfaceUtilization(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcInterfaceStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_type, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcInterfaceStatistics, (olt_id, onu_id, cmc_mac, channel_type, channel_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcInterfaceStatistics(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_DumpCmcMacStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcMacStatistics, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcMacStatistics(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcAllInterface(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcAllInterface, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcAllInterface(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}
#endif


#if 1
/* --------------------CMC频道组管理API------------------- */

int OnuMgt_DumpCmcAllLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcAllLoadBalancingGrp, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcAllLoadBalancingGrp(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcLoadBalancingGrp, (olt_id, onu_id, cmc_mac, grp_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcLoadBalancingGrp(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcLoadBalancingGrpDownstream, (olt_id, onu_id, cmc_mac, grp_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcLoadBalancingGrpDownstream(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcLoadBalancingGrpUpstream, (olt_id, onu_id, cmc_mac, grp_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcLoadBalancingGrpUpstream(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcLoadBalancingDynConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcLoadBalancingDynConfig, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcLoadBalancingDynConfig(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_SetCmcLoadBalancingDynMethod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char method)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcLoadBalancingDynMethod, (olt_id, onu_id, cmc_mac, method) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcLoadBalancingDynMethod(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcLoadBalancingDynPeriod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long period)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcLoadBalancingDynPeriod, (olt_id, onu_id, cmc_mac, period) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcLoadBalancingDynPeriod(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcLoadBalancingDynWeightedAveragePeriod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long period)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcLoadBalancingDynWeightedAveragePeriod, (olt_id, onu_id, cmc_mac, period) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcLoadBalancingDynWeightedAveragePeriod(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcLoadBalancingDynOverloadThresold(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char percent)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcLoadBalancingDynOverloadThresold, (olt_id, onu_id, cmc_mac, percent) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcLoadBalancingDynOverloadThresold(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcLoadBalancingDynDifferenceThresold(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char percent)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcLoadBalancingDynDifferenceThresold, (olt_id, onu_id, cmc_mac, percent) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcLoadBalancingDynDifferenceThresold(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcLoadBalancingDynMaxMoveNumber(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long max_move)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcLoadBalancingDynMaxMoveNumber, (olt_id, onu_id, cmc_mac, max_move) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcLoadBalancingDynMaxMoveNumber(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcLoadBalancingDynMinHoldTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long hold_time)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcLoadBalancingDynMinHoldTime, (olt_id, onu_id, cmc_mac, hold_time) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcLoadBalancingDynMinHoldTime(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcLoadBalancingDynRangeOverrideMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char range_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcLoadBalancingDynRangeOverrideMode, (olt_id, onu_id, cmc_mac, range_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcLoadBalancingDynRangeOverrideMode(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcLoadBalancingDynAtdmaDccInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcLoadBalancingDynAtdmaDccInitTech, (olt_id, onu_id, cmc_mac, tech_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcLoadBalancingDynAtdmaDccInitTech(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcLoadBalancingDynScdmaDccInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcLoadBalancingDynScdmaDccInitTech, (olt_id, onu_id, cmc_mac, tech_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcLoadBalancingDynScdmaDccInitTech(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcLoadBalancingDynAtdmaDbcInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcLoadBalancingDynAtdmaDbcInitTech, (olt_id, onu_id, cmc_mac, tech_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcLoadBalancingDynAtdmaDbcInitTech(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmcLoadBalancingDynScdmaDbcInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmcLoadBalancingDynScdmaDbcInitTech, (olt_id, onu_id, cmc_mac, tech_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmcLoadBalancingDynScdmaDbcInitTech(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_CreateCmcLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char grp_method)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, CreateCmcLoadBalancingGrp, (olt_id, onu_id, cmc_mac, grp_id, grp_method) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_CreateCmcLoadBalancingGrp(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DestroyCmcLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DestroyCmcLoadBalancingGrp, (olt_id, onu_id, cmc_mac, grp_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DestroyCmcLoadBalancingGrp(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_AddCmcLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(ch_ids);
    VOS_ASSERT((num_of_ch > 0) && (num_of_ch < 17));
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, AddCmcLoadBalancingGrpDownstream, (olt_id, onu_id, cmc_mac, grp_id, num_of_ch, ch_ids) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_AddCmcLoadBalancingGrpDownsteam(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_RemoveCmcLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(ch_ids);
    VOS_ASSERT((num_of_ch > 0) && (num_of_ch < 17));
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, RemoveCmcLoadBalancingGrpDownstream, (olt_id, onu_id, cmc_mac, grp_id, num_of_ch, ch_ids) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_RemoveCmcLoadBalancingGrpDownstream(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_AddCmcLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(ch_ids);
    VOS_ASSERT((num_of_ch > 0) && (num_of_ch < 5));
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, AddCmcLoadBalancingGrpUpstream, (olt_id, onu_id, cmc_mac, grp_id, num_of_ch, ch_ids) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_AddCmcLoadBalancingGrpUpstream(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_RemoveCmcLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(ch_ids);
    VOS_ASSERT((num_of_ch > 0) && (num_of_ch < 5));
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, RemoveCmcLoadBalancingGrpUpstream, (olt_id, onu_id, cmc_mac, grp_id, num_of_ch, ch_ids) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_RemoveCmcLoadBalancingGrpUpstream(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_AddCmcLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(mac_start);
    VOS_ASSERT(mac_end);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, AddCmcLoadBalancingGrpModem, (olt_id, onu_id, cmc_mac, grp_id, mac_start, mac_end) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_AddCmcLoadBalancingGrpModem(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_RemoveCmcLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(mac_start);
    VOS_ASSERT(mac_end);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, RemoveCmcLoadBalancingGrpModem, (olt_id, onu_id, cmc_mac, grp_id, mac_start, mac_end) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_RemoveCmcLoadBalancingGrpModem(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_AddCmcLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(mac_start);
    VOS_ASSERT(mac_end);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, AddCmcLoadBalancingGrpExcludeModem, (olt_id, onu_id, cmc_mac, mac_start, mac_end) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_AddCmcLoadBalancingGrpExcludeModem(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_RemoveCmcLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(mac_start);
    VOS_ASSERT(mac_end);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, RemoveCmcLoadBalancingGrpExcludeModem, (olt_id, onu_id, cmc_mac, mac_start, mac_end) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_RemoveCmcLoadBalancingGrpExcludeModem(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcLoadBalancingGrpModem, (olt_id, onu_id, cmc_mac, grp_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcLoadBalancingGrpModem(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcLoadBalancingGrpActivedModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcLoadBalancingGrpActivedModem, (olt_id, onu_id, cmc_mac, grp_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcLoadBalancingGrpActivedModem(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcLoadBalancingGrpExcludeModem, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcLoadBalancingGrpExcludeModem(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcLoadBalancingGrpExcludeActivedModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcLoadBalancingGrpExcludeActivedModem, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcLoadBalancingGrpExcludeActivedModem(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

#endif


#if 1
/* --------------------CM管理API------------------- */

int OnuMgt_DumpAllCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpAllCm, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpAllCm(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(cm_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCm, (olt_id, onu_id, cmc_mac, cm_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCm(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpAllCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpAllCmHistory, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpAllCmHistory(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(cm_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmHistory, (olt_id, onu_id, cmc_mac, cm_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmHistory(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_ClearAllCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, ClearAllCmHistory, (olt_id, onu_id, cmc_mac) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_ClearAllCmHistory(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_ResetCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(cm_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, ResetCm, (olt_id, onu_id, cmc_mac, cm_mac) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_ResetCm(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_DumpCmDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(cm_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmDownstream, (olt_id, onu_id, cmc_mac, cm_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmDownstream(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(cm_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmUpstream, (olt_id, onu_id, cmc_mac, cm_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmUpstream(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(ch_ids);
    VOS_ASSERT((num_of_ch > 0) && (num_of_ch < 17));
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmDownstream, (olt_id, onu_id, cmc_mac, cm_mac, num_of_ch, ch_ids) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmDownstream(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_SetCmUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(ch_ids);
    VOS_ASSERT((num_of_ch > 0) && (num_of_ch < 5));
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetCmUpstream, (olt_id, onu_id, cmc_mac, cm_mac, num_of_ch, ch_ids) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_SetCmUpstream(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_CreateCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char cos, char *tlv_data, unsigned short tlv_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(cm_mac);
    VOS_ASSERT(tlv_data);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, CreateCmServiceFlow, (olt_id, onu_id, cmc_mac, cm_mac, cos, tlv_data, tlv_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_CreateCmServiceFlow(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_ModifyCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned long usfid, unsigned long dsfid, char *tlv_data, unsigned short tlv_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(cm_mac);
    VOS_ASSERT(tlv_data);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, ModifyCmServiceFlow, (olt_id, onu_id, cmc_mac, cm_mac, usfid, dsfid, tlv_data, tlv_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_ModifyCmServiceFlow(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DestroyCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned long usfid, unsigned long dsfid)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(cm_mac);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DestroyCmServiceFlow, (olt_id, onu_id, cmc_mac, cm_mac, usfid, dsfid) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DestroyCmServiceFlow(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_DumpCmClassifier(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(cm_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmClassifier, (olt_id, onu_id, cmc_mac, cm_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmClassifier(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(cm_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmServiceFlow, (olt_id, onu_id, cmc_mac, cm_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmServiceFlow(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

#endif


#if 1
/* --------------------QoS管理API------------------- */

int OnuMgt_DumpCmcClassifier(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT((num_of_sf > 0) && (num_of_sf < 8));
    VOS_ASSERT(sf_ids);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcClassifier, (olt_id, onu_id, cmc_mac, num_of_sf, sf_ids, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcClassifier(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT((num_of_sf > 0) && (num_of_sf < 8));
    VOS_ASSERT(sf_ids);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcServiceFlow, (olt_id, onu_id, cmc_mac, num_of_sf, sf_ids, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcServiceFlow(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcServiceFlowStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT((num_of_sf > 0) && (num_of_sf < 8));
    VOS_ASSERT(sf_ids);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcServiceFlowStatistics, (olt_id, onu_id, cmc_mac, num_of_sf, sf_ids, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcServiceFlowStatistics(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_DumpCmcDownChannelBondingGroup(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcDownChannelBondingGroup, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcDownChannelBondingGroup(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DumpCmcUpChannelBondingGroup(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DumpCmcUpChannelBondingGroup, (olt_id, onu_id, cmc_mac, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DumpCmcUpChannelBondingGroup(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}


int OnuMgt_CreateCmcServiceFlowClassName(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *class_name, char *tlv_data, unsigned short tlv_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(class_name);
    VOS_ASSERT(tlv_data);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, CreateCmcServiceFlowClassName, (olt_id, onu_id, cmc_mac, class_name, tlv_data, tlv_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_CreateCmcServiceFlowClassName(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

int OnuMgt_DestroyCmcServiceFlowClassName(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *class_name)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(class_name);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, DestroyCmcServiceFlowClassName, (olt_id, onu_id, cmc_mac, class_name) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_DestroyCmcServiceFlowClassName(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }

    return iRlt;
}

#endif


#if 1
/* --------------------地址管理API------------------- */

int OnuMgt_GetCmcMacAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *addr_num, PON_address_table_t addr_table)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(addr_num);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmcMacAddrTbl, (olt_id, onu_id, cmc_mac, addr_num, addr_table) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmcMacAddrTbl(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}

int OnuMgt_GetCmMacAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned short *addr_num, PON_address_table_t addr_table)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(cm_mac);
    VOS_ASSERT(addr_num);
	
    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetCmMacAddrTbl, (olt_id, onu_id, cmc_mac, cm_mac, addr_num, addr_table) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_ONU, onu_mgt_log_level, "OnuMgt_GetCmMacAddrTbl(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;
}

int OnuMgt_ResetCmAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, int addr_type)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmc_mac);
    VOS_ASSERT(cm_mac);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, ResetCmAddrTbl, (olt_id, onu_id, cmc_mac, cm_mac, addr_type) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_ResetCmAddrTbl(%d, %d, %d) Error Rlt(%d).", olt_id, onu_id, addr_type, iRlt);
    }
    
    return iRlt;    
}

#endif

#endif

int OnuMgt_SetMulticastTemplate( short int olt_id, short int onu_id, int prof1,  int prof2,  CTC_GPONADP_ONU_Multicast_Prof_t *parameter,unsigned char stateflag)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(parameter);
	
   if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,SetMulticastTemplate, (olt_id,onu_id,prof1, prof2, parameter,stateflag))))
   	{
   	
   	}
  else
  	{
  		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_SetMulticastTemplate(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
  	}
    return iRlt;
}
int OnuMgt_GetMulticastTemplate( short int olt_id, short int onu_id, int prof1,  int prof2,  CTC_GPONADP_ONU_Multicast_Prof_t *parameter,unsigned char stateflag)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(parameter);
	
   if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetMulticastTemplate, (olt_id,onu_id,prof1, prof2, parameter,stateflag))))
   	{
   	
   	}
  else
  	{
  		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_GetMulticastTemplate(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
  	}
    return iRlt;
}

int OnuMgt_SetMcastOperProfile( short int olt_id, short int onu_id, int prof,  CTC_GPONADP_ONU_McastOper_Prof_t *parameter)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(parameter);
	
   if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,SetMcastOperProfile, (olt_id,onu_id,prof, parameter))))
   	{
   	
   	}
  else
  	{
  		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_SetMcastOperProfile(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
  	}
    return iRlt;
}
int OnuMgt_GetMcastOperProfile( short int olt_id, short int onu_id, int prof,  CTC_GPONADP_ONU_McastOper_Prof_t *parameter)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(parameter);
	
   if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetMcastOperProfile, (olt_id,onu_id,prof, parameter))))
   	{
   	
   	}
  else
  	{
  		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_GetMcastOperProfile(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
  	}
    return iRlt;
}
int OnuMgt_SetUniPortAssociateMcastProf( short int olt_id, short int onu_id,short int portid ,unsigned char stateflag,int profIdx)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
   if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,SetUniPortAssociateMcastProf, (olt_id,onu_id,portid ,stateflag, profIdx))))
   	{
   	
   	}
  else
  	{
  		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_SetUniPortAssociateMcastProf(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
  	}
    return iRlt;
}
int OnuMgt_GetUniPortAssociateMcastProf( short int olt_id, short int onu_id, short int port_id,CTC_GPONADP_ONU_Profile_t *ProfIdx)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
   VOS_ASSERT(ProfIdx);
	
   if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2(olt_id, onu_id,GetUniPortAssociateMcastProf, (olt_id,onu_id,port_id ,ProfIdx))))
   	{
   	
   	}
  else
  	{
  		VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_GetUniPortAssociateMcastProf(%d, %d) Error Rlt(%d).", olt_id, onu_id,iRlt);
  	}
    return iRlt;
}


#if 1
/* --------------------ONU DOCSIS应用管理API------------------- */

#endif

#if 0
/*--------------------- GPON OMCI-------------------------*/
int OnuMgt_GetGponOnuCfg(short int olt_id, short int onu_id, tGponOnuConfig *onuConfig)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(onuConfig);

    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, GetGponOnuCfg, (olt_id, onu_id, onuConfig) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_GetGponOnuCfg(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;    
}
int OnuMgt_SetGponOnuCfg(short int olt_id, short int onu_id, tGponOnuConfig *onuConfig)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);


    if ( OLT_ERR_OK == (iRlt = ONUMGT_API_CALL2( olt_id, onu_id, SetGponOnuCfg, (olt_id, onu_id, onuConfig) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, onu_mgt_log_level, "OnuMgt_SetGponOnuCfg(%d, %d) Error Rlt(%d).", olt_id, onu_id, iRlt);
    }
    
    return iRlt;    
}
#endif

#undef ONUMGT_LOCAL_CALL
#undef ONUMGT_REMOTE_CALL
#undef ONUMGT_API_CALL
#undef ONUMGT_REMOTE_CALL2
#undef ONUMGT_API_CALL2
#endif


/* -------------------ONU高级API------------------- */
int OnuMgtAdv_IsValid(short int olt_id, short int onu_id)
{
    int iExist = 0;

    (void)OnuMgt_OnuIsValid(olt_id, onu_id, &iExist);
    
    return ( (0 < iExist) ? 0 : V2R1_ONU_NOT_EXIST ); 
}

int OnuMgtAdv_IsOnline(short int olt_id, short int onu_id)
{
    int iOnline = 0;

    (void)OnuMgt_OnuIsOnline(olt_id, onu_id, &iOnline);
    
    return ( (0 < iOnline) ? 0 : V2R1_ONU_OFF_LINE ); 
}

/*Begin:for onu swap by jinhl@2013-04-27*/
int OnuMgtAdv_GetBWInfo(short int olt_id, short int onu_id, PonPortBWInfo_S *ponBW, ONUBWInfo_S *onuBW)
{
    int iRlt = 0;

	if(OLT_ISLOCAL(olt_id))
	{
	    iRlt = GWONU_GetBWInfo(olt_id, onu_id, ponBW, onuBW);
	}
	else
	{
	    iRlt = OnuMgt_GetBWInfo(olt_id, onu_id, ponBW, onuBW);
	}

	return iRlt;
}
/*End:for onu swap by jinhl@2013-04-27*/


#ifdef __cplusplus

}

#endif


