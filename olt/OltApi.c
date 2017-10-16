/***************************************************************
*
*						Module Name:  OltApi.c
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

#include  "../monitor/monitor.h"
#include  "../access_identifier/access_id.h"
#define ACCESS_ID_TASK_NAME "tAccessId"
#include "i2c.h"
#include "onu/onuOamUpd.h"
#include "gwEponSys.h"

extern unsigned int PON_Jumbo_frame_length;
extern ULONG g_onuAuthEnable;
extern ULONG g_ulIgmp_Auth_Enabled;
extern ULONG g_ulRPCCall_mode;
extern unsigned char PON_priority_queue_mapping[];
extern LONG ACCESS_ID_TaskID;
extern int pon_swap_switch_enable;
extern ponAlarmInfo		**gpPonAlarm;
extern ponAlarmInfo		**gpOnuAlarm;
extern ponThreasholdInfo gPonThreashold;
extern ULONG	gLibEponMibSemId;
#if(EPON_MODULE_ONU_MAC_OVERFLOW_CHECK==EPON_MODULE_YES)
/*extern ULONG  Onu_user_max_all;*/
extern ULONG  mac_check_exit_flag;
extern ULONG  OnuMacTable_check_interval;
extern LONG onu_mac_check_alarm_threshold_set( ULONG threshold );
#endif
extern STATUS	setOnuAuthEnableForSinglePon(ULONG slot, ULONG port, ULONG enable );
extern int olt_ctc_auth_mode;
extern unsigned short cmc_service_vlanid;
extern int GetOnuConfRestoreQueueStatus(char *name);
#if 0
extern int bcm_port_enable_set(int	unit, int port, int enable);
#endif
int OLT_SetGponAuthEntry(short int olt_id, gpon_onu_auth_entry_t *entry);
extern int i2c_data_set( UINT slot_id, UINT type, UINT reg, UCHAR *pdata, UINT len);
extern int set_board_i2cinfo( UCHAR slot_id, USHORT boardtype, UCHAR *boardsn, UCHAR *boardver, UCHAR *boarddate );
extern int olt_remotemange_init();
extern int Onustats_SetPortStatsTimeOut(short int olt_id,ONU_PORTSTATS_TIMER_NAME_E timer_name,ULONG timeout);

int OLTAdv_NotifyOltInvalid(short int olt_id);

char *g_cszPonChipTypeNames[PONCHIP_TYPE_MAX];    

ULONG olt_mgt_log_level = LOG_DEBUG;

#if 1
/* -------------------OLT  API的安装及初始化接口------------------- */
static const OltMgmtIFs *s_aOltIfs[OLT_ADAPTER_MAX];
static const OltMgmtIFs *s_pSysOltIfs = NULL;
static const OltMgmtIFs *s_pRemoteOltIfs = NULL;
static int               s_iRemoteOltRlt = 0;
static unsigned char     s_aucOltIDSpans[256];

#define OLT_ID_SPAN(olt_id)    s_aucOltIDSpans[olt_id]


void OLT_RegisterAdapter(OLT_ADAPTER_TYPE adater_type, OltMgmtIFs *olt_ifs)
{
    VOS_ASSERT(olt_ifs);
    VOS_ASSERT((adater_type >= 0) && (adater_type < OLT_ADAPTER_MAX));

    s_aOltIfs[adater_type] = olt_ifs;

    return;
}

int OLT_SetupIFs(short int olt_id, OLT_ADAPTER_TYPE adater_type)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT((adater_type >= 0) && (adater_type < OLT_ADAPTER_MAX));

    if ( NULL != s_aOltIfs[adater_type] )
    {
        PonPortTable[olt_id].OltIFs = s_aOltIfs[adater_type];
    }
    else
    {
        iRlt = OLT_ERR_NOTSUPPORT;
    }

    return iRlt;
}

int OLT_SetupIFsByChipType(short int olt_id, int chip_type)
{
    static int s_chip2adapter[17] = { OLT_ADAPTER_NULL,      /*  ponChipType_null */
                                      OLT_ADAPTER_PAS5001,   /*  ponChipType_PAS5001 */
                                      OLT_ADAPTER_PAS5201,   /*  ponChipType_PAS5201 */
                                      OLT_ADAPTER_PAS5204,   /*  ponChipType_PAS5204 */
                                      OLT_ADAPTER_PAS8411,   /*  ponChipType_PAS8411 */
                                      OLT_ADAPTER_NULL,      /*  5 */
                                      OLT_ADAPTER_NULL,      /*  6 */
                                      OLT_ADAPTER_TK3723,    /*  ponChipType_TK3723 */
                                      OLT_ADAPTER_BCM55524,  /*  ponChipType_BCM55524 */
                                      OLT_ADAPTER_BCM55538,  /*  ponChipType_BCM55538 */
                                      OLT_ADAPTER_NULL,      /*  10 */
                                      OLT_ADAPTER_NULL,      /*  11 */
                                      OLT_ADAPTER_NULL,      /*  12 */
                                      OLT_ADAPTER_NULL,      /*  13 */
                                      OLT_ADAPTER_NULL,      /*  14 */
                                      OLT_ADAPTER_GW,     /*15*//*  ponChipType_GW */
									#if 1
									#if defined(_GPON_BCM_SUPPORT_)
										OLT_ADAPTER_GPON       /*16*/
									#endif
									#endif
                                      };       

    if ( (chip_type < 0) || (chip_type >= PONCHIP_TYPE_MAX))
    {
        return OLT_ERR_NOTSUPPORT;
    }

    return OLT_SetupIFs(olt_id, s_chip2adapter[chip_type]);
}

int OLT_GetIFType(short int olt_id)
{
    const OltMgmtIFs  *pOltIFs;
    int iIfType = -1;

    OLT_LOCAL_ASSERT(olt_id);
    if ( NULL != (pOltIFs = PonPortTable[olt_id].OltIFs) )
    {
        int i;
        
        for (i=0; i<OLT_ADAPTER_MAX; i++)
        {
            if (pOltIFs == s_aOltIfs[i])
            {
                iIfType = i;
                break;
            }
        }
    }

    return iIfType;
}

int OLT_GetIFChipType(short int olt_id)
{
    static int s_adapter2chip[16] = { ponChipType_null,      /* OLT_ADAPTER_NULL */
                                      -1,                    /* OLT_ADAPTER_GLOBAL */
                                      -1,                    /* OLT_ADAPTER_RPC */
                                      ponChipType_PAS5001,   /* OLT_ADAPTER_PAS5001 */
                                      ponChipType_PAS5201,   /* OLT_ADAPTER_PAS5201 */
                                      ponChipType_PAS5204,   /* OLT_ADAPTER_PAS5204 */
                                      ponChipType_PAS8411,   /* OLT_ADAPTER_PAS8411 */
                                      ponChipType_TK3723,    /* OLT_ADAPTER_TK3723 */
                                      ponChipType_BCM55524,  /* OLT_ADAPTER_BCM55524 */
                                      ponChipType_BCM55538,  /* OLT_ADAPTER_BCM55538 */
                                      ponChipType_GW};       /* OLT_ADAPTER_GW */
    int iIfType = OLT_GetIFType(olt_id);

    if ( iIfType >= 0 )
    {
        VOS_ASSERT(iIfType < ARRAY_SIZE(s_adapter2chip));
        iIfType = s_adapter2chip[iIfType];
    }
    
    return iIfType;
}

extern void OLT_NULL_Support();
extern void OLT_RPC_Support();
extern void OLT_PasGlobal_Support();

extern void OLT_Pas5001_Support();
extern void OLT_Pas5201_Support();
extern void OLT_Pas5204_Support();
#if defined(_EPON_10G_PMC_SUPPORT_)            
extern void OLT_Pas8411_Support();/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif
extern void OLT_Tk3723_Support();
extern void OLT_Bcm55524_Support();
extern void OLT_Bcm55538_Support();
#if defined(_GPON_BCM_SUPPORT_)
extern void OLT_GPON_Support();
#endif
/* OLT全局管理的接口 */
void OLT_Global_Support(int isLocal)
{

	int i = 0;

    if ( isLocal )
    {
        /* OLT本地集中式管理的接口 */
        s_pSysOltIfs = s_aOltIfs[OLT_ADAPTER_GLOBAL];

        /* OLT本地遍历间隔为1 */
        VOS_MemSet(s_aucOltIDSpans, 1, sizeof(s_aucOltIDSpans));
    }
    else
    {
        /* OLT全局分布式管理的接口 */
        s_pSysOltIfs = NULL;

        /* OLT全局遍历间隔初始化 */
        if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
        {
            short int PonPortIdx = GetPonPortIdxBySlot( SYS_LOCAL_MODULE_SLOTNO, FIRSTPONPORTPERCARD );

            /* OLT远程板卡遍历间隔为板卡最大端口数 */
            VOS_MemSet(s_aucOltIDSpans, (CHAR)PONPORTPERCARD, sizeof(s_aucOltIDSpans));

            if ( OLT_LOCAL_ISVALID(PonPortIdx) )
            {			
				/* OLT本地遍历间隔为1 */
                VOS_MemSet(&s_aucOltIDSpans[PonPortIdx], 1, PONPORTPERCARD);

            }
            else
            {
                VOS_ASSERT(0);
            }
        }
        else
        {
            /* OLT远程板卡遍历间隔为板卡最大端口数 */
            VOS_MemSet(s_aucOltIDSpans, (CHAR)PONPORTPERCARD, sizeof(s_aucOltIDSpans));
        }
    }

    return;
}

/* OLT远程管理的接口 */
void OLT_REMOTE_Support()
{
    s_pRemoteOltIfs = s_aOltIfs[OLT_ADAPTER_RPC];

#if ( EPON_MODULE_PON_REMOTE_MANAGE == EPON_MODULE_YES )
    if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
    {
        olt_remotemange_init();
    }
#endif    
}

void OLT_PonChipType_Init()
{
    VOS_MemZero(g_cszPonChipTypeNames, sizeof(g_cszPonChipTypeNames));

    OLT_Pas5001_Support();
    OLT_Pas5201_Support();
    OLT_Pas5204_Support();
#if defined(_EPON_10G_PMC_SUPPORT_)            
    OLT_Pas8411_Support();/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif
    OLT_Tk3723_Support();
    OLT_Bcm55524_Support();
#if defined(_EPON_10G_BCM_SUPPORT_)            
    OLT_Bcm55538_Support();
#endif

#if defined(_GPON_BCM_SUPPORT_)
	OLT_GPON_Support();
#endif
    if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
    {
        if ( SYS_LOCAL_MODULE_TYPE_IS_PAS_PONCARD_MANAGER )
        {
            OLT_PasGlobal_Support();
        }
        else if ( SYS_LOCAL_MODULE_TYPE_IS_TK_PONCARD_MANAGER )
        {
            OLT_TkGlobal_Support();
        }
		#if 0
		#if defined(_GPON_BCM_SUPPORT_)
		else if(SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER)
		{
			OLT_GPONGlobal_Support();
		}
		#endif
		#endif
        else
        {
            VOS_ASSERT(0);
        }
    }    
}

void OLT_API_Init()
{
    VOS_ASSERT(SYS_LOCAL_MODULE_TYPE_IS_PON_MANAGER);

    VOS_MemZero(s_aOltIfs, sizeof(s_aOltIfs));
    
    /* OLT支持预配置 */
    OLT_NULL_Support();

    /* OLT支持分布式配置 */
    OLT_RPC_Support();

    /* OLT支持远程配置 */
    OLT_REMOTE_Support();

    /* OLT支持的芯片类型 */
    OLT_PonChipType_Init();
    
    if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
    {
        /* B--added by liwei056@2013-1-11 for D16719 */
        if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
        {
            if ( PRODUCT_IS_DISTRIBUTE )
            {
                /* 分布式设备:OLT全局配置模式为分布 */
                OLT_Global_Support(FALSE);
            }
            else
            {
                /* 集中式设备:OLT全局配置模式为本地 */
                OLT_Global_Support(TRUE);
            }
        }
        else
        /* E--added by liwei056@2013-1-11 for D16719 */
        {
            /* OLT全局配置模式为本地 */
            OLT_Global_Support(TRUE);
        }
    }
    else
    {
        /* OLT全局配置模式为分布 */
        OLT_Global_Support(FALSE);
    }
}
#endif


#if 1
/* -------------------OLT API的部分本地GW实现------------------- */

#if 1
/* -------------------OLT基本API------------------- */
int GW_GetChipTypeID(short int olt_id, int *type)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(type);

    *type = getPonChipType(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id));
    OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetChipTypeID(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *type, 0, SYS_LOCAL_MODULE_SLOTNO);

    return 0;
}

int GW_GetChipTypeName(short int olt_id, char typename[OLT_CHIP_NAMELEN])
{
    int iRlt;
    int iChipType;
    
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(typename);

    if ( 0 == (iRlt = OLT_GetChipTypeID(olt_id, &iChipType)) )
    {
        if ( OLT_PONCHIP_ISVALID(iChipType) )
        {
            if ( NULL != g_cszPonChipTypeNames[iChipType] )
            {
                VOS_MemZero( typename, OLT_CHIP_NAMELEN );
                VOS_StrnCpy(typename, g_cszPonChipTypeNames[iChipType], OLT_CHIP_NAMELEN - 1);
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
        else
        {
            iRlt = OLT_ERR_UNKNOEWN;
        }
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetChipTypeName(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GW_ResetPon(short int olt_id)
{
    int iRlt = OLT_ERR_OK;
    unsigned long ulSlot, ulPort;

    OLT_LOCAL_ASSERT(olt_id);

    if ( (RERROR == (ulSlot = GetCardIdxByPonChip(olt_id)))
        || (RERROR == (ulPort = GetPonPortByPonChip(olt_id))) )
    {
        iRlt = OLT_ERR_PARAM;
    }
    else
    {
        if ( V2R1_ENABLE != GetPonChipResetFlag(ulSlot, ulPort) )
        {
            ClearPonRunningData( olt_id );
#if 0
            Hardware_Reset_olt1(ulSlot, ulPort, 1, 0);
            Hardware_Reset_olt2(ulSlot, ulPort, 1, 0);
            SetPonChipResetFlag(ulSlot, ulPort);
#endif
            Add_PonPort( olt_id );		
        }
        else
        {
            iRlt = OLT_ERR_NOTEXIST;
        }
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GW_ResetPon(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GW_CopyOlt(short int olt_id, short int dst_olt_id, int copy_flags)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CopyOlt(dst_olt_id, olt_id, copy_flags);
    OLT_GW_DEBUG(OLT_GW_TITLE"GW_CopyOlt(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, dst_olt_id, copy_flags, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GW_CmdIsSupported(short int olt_id, short int *cmd)
{
    static short int aiCmdSpans[][2] = {
                                  {OLT_CMD_SPLIT_LINE_FRAME_BEGIN, OLT_CMD_SPLIT_LINE_FRAME_END},
                                  {OLT_CMD_SPLIT_LINE_PON_FUN_BEGIN, OLT_CMD_SPLIT_LINE_PON_FUN_END},
                                  {OLT_CMD_SPLIT_LINE_LLID_FUN_BEGIN, OLT_CMD_SPLIT_LINE_LLID_FUN_END},
                                  {OLT_CMD_SPLIT_LINE_ONU_FUN_BEGIN, OLT_CMD_SPLIT_LINE_ONU_FUN_END},
                                  {OLT_CMD_SPLIT_LINE_ENCRYPT_FUN_BEGIN, OLT_CMD_SPLIT_LINE_ENCRYPT_FUN_END},
                                  {OLT_CMD_SPLIT_LINE_ADDRTBL_FUN_BEGIN, OLT_CMD_SPLIT_LINE_ADDRTBL_FUN_END},
                                  {OLT_CMD_SPLIT_LINE_OPTICS_FUN_BEGIN, OLT_CMD_SPLIT_LINE_OPTICS_FUN_END},
                                  {OLT_CMD_SPLIT_LINE_MONITOR_FUN_BEGIN, OLT_CMD_SPLIT_LINE_MONITOR_FUN_END},
                                  {OLT_CMD_SPLIT_LINE_HOTSWAP_FUN_BEGIN, OLT_CMD_SPLIT_LINE_HOTSWAP_FUN_END},
                                  {OLT_CMD_SPLIT_LINE_CMC_FUN_BEGIN, OLT_CMD_SPLIT_LINE_CMC_FUN_END},

                                  {OLT_CMD_MAX, OLT_CMD_MAX}
                                 };
    OltMgmtIFs *pOltChipIfs;
    int iChipTypeId;
    int iRlt;
    short int sCmdID;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(cmd);

    iRlt = OLT_ERR_NOTSUPPORT;
    if ( OLT_CMD_MAX > (sCmdID = *cmd) )
    {
        iChipTypeId = OLTAdv_GetChipTypeID(olt_id);
    
        /* 检查本地的芯片类型函数表，获知是否支持特定的命令 */
        switch (iChipTypeId)
        {
            case PONCHIP_UNKNOWN:
                return OLT_ERR_UNKNOEWN;
            case PONCHIP_PAS5001:
                pOltChipIfs = s_aOltIfs[OLT_ADAPTER_PAS5001];
                break;
            case PONCHIP_PAS5201:
                pOltChipIfs = s_aOltIfs[OLT_ADAPTER_PAS5201];
                break;
            case PONCHIP_PAS5204:
                pOltChipIfs = s_aOltIfs[OLT_ADAPTER_PAS5204];
                break;
            case PONCHIP_PAS8411:
                pOltChipIfs = s_aOltIfs[OLT_ADAPTER_PAS8411];
                break;
            case PONCHIP_TK3723:
                pOltChipIfs = s_aOltIfs[OLT_ADAPTER_TK3723];
                break;
            case PONCHIP_BCM55524:
                pOltChipIfs = s_aOltIfs[OLT_ADAPTER_BCM55524];
                break;
            case PONCHIP_BCM55538:
                pOltChipIfs = s_aOltIfs[OLT_ADAPTER_BCM55538];
                break;
            default:
                pOltChipIfs = NULL;
        }

        if ( NULL != pOltChipIfs )
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
                if ( 0 != *((int*)pOltChipIfs + iCmdIdx) )
                {
                    iRlt = OLT_ERR_OK;
                }
            }
        }
    }

    if ( 0 != iRlt )
    {
        *cmd = -sCmdID;
    }
    OLT_GW_DEBUG(OLT_GW_TITLE"GW_CmdIsSupported(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *cmd, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GW_SetPonI2CExtInfo(short int olt_id, eeprom_gfa_epon_ext_t *pon_info)
{
    int iRlt = 0;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(pon_info);

	if( i2c_data_set( GetCardIdxByPonChip(olt_id), I2C_BASE_E2PROM, I2C_REG_EXT_DATA + olt_id * sizeof(eeprom_gfa_epon_ext_t), (UCHAR*)pon_info, 
		sizeof(eeprom_gfa_epon_ext_t) ) != TRUE )
	{
		iRlt = OLT_ERR_UNKNOEWN;
	}

    OLT_GW_DEBUG(OLT_GW_TITLE"GW_SetPonI2CExtInfo(%d)'s result(%d) [Inserted(%d), CTC(%d), ChipType(%d), SFP(%d)] on slot %d.\r\n", olt_id, iRlt, pon_info->ponInserted, pon_info->ponCtc, pon_info->ponChipType, pon_info->spfVendor, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GW_GetPonI2CExtInfo(short int olt_id, eeprom_gfa_epon_ext_t *pon_info)
{
    int iRlt = 0;
    eeprom_gfa_epon_ext_t *pPonExtInfo = (eeprom_gfa_epon_ext_t*)g_sysinfo_module[GetCardIdxByPonChip(olt_id)].ext_data;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(pon_info);

    VOS_MemCpy(pon_info, &pPonExtInfo[olt_id], sizeof(eeprom_gfa_epon_ext_t));

    OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetPonI2CExtInfo(%d)'s result(%d) [Inserted(%d), CTC(%d), ChipType(%d), SFP(%d)] on slot %d.\r\n", olt_id, iRlt, pon_info->ponInserted, pon_info->ponCtc, pon_info->ponChipType, pon_info->spfVendor, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GW_SetCardI2CInfo(short int olt_id, board_sysinfo_t *board_info)
{
    int iRlt = 0;
    int slot, type;
    USHORT hard_type;
    UCHAR *pszSn;
    UCHAR *pszVer;
    UCHAR *pszDate;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(board_info);

    pszSn = board_info->serialno;
    if ( '\0' == pszSn[0] )
    {
        pszSn = NULL;
    }

    pszVer = board_info->version;
    if ( '\0' == pszVer[0] )
    {
        pszVer = NULL;
    }

    pszDate = board_info->product_date;
    if ( '\0' == pszDate[0] )
    {
        pszDate = NULL;
    }

    slot = GetCardIdxByPonChip(olt_id);
    type = *((int*)board_info->ext_data);
    hard_type = (USHORT)device_soft_modtype_2_hw_modtype(type);
	if( set_board_i2cinfo(slot, hard_type, pszSn, pszVer, pszDate) != TRUE )
	{
		iRlt = OLT_ERR_UNKNOEWN;
	}

    OLT_GW_DEBUG(OLT_GW_TITLE"GW_SetCardI2CInfo(%d)'s result(%d) [slot%d: soft_type(%d)<->hard_type(%d), sn(%s), ver(%s), date(%s)] on slot %d.\r\n", olt_id, iRlt, slot, type, hard_type, board_info->serialno, board_info->version, board_info->product_date, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GW_GetCardI2CInfo(short int olt_id, board_sysinfo_t *board_info)
{
    int iRlt = 0;
    int slot, type;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(board_info);

    VOS_MemZero(board_info->serialno, sizeof(board_info->serialno));
    VOS_MemZero(board_info->version, sizeof(board_info->version));
    VOS_MemZero(board_info->product_date, sizeof(board_info->product_date));
    slot = GetCardIdxByPonChip(olt_id);
    type = device_slot_moduletype_get(slot);
    
    *((int*)board_info->ext_data) = type;
	VOS_StrnCpy(board_info->serialno, g_sysinfo_module[slot].serialno, 16 );
	VOS_StrnCpy(board_info->version, g_sysinfo_module[slot].version, 16 );
	VOS_StrnCpy(board_info->product_date, g_sysinfo_module[slot].product_date, 10 );

    OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetCardI2CInfo(%d)'s result(%d) [sn(%s), ver(%s), date(%s)] on slot %d.\r\n", olt_id, iRlt, board_info->serialno, board_info->version, board_info->product_date, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif


#if 1
/* -------------------OLT PON管理API的GW实现--------------- */
int GW_GetAdminStatus(short int olt_id, int *admin_status)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(admin_status);

    *admin_status = PonPortTable[olt_id].PortAdminStatus;
    
    /* OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetAdminStatus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *admin_status, 0, SYS_LOCAL_MODULE_SLOTNO); */
    
    return 0;
}

int GW_GetPonWorkStatus(short int olt_id, int *work_status)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(work_status);

    *work_status = PonPortTable[olt_id].PortWorkingStatus;
    
    /* OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetPonWorkStatus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *work_status, 0, SYS_LOCAL_MODULE_SLOTNO); */
    
    return 0;
}

int GW_SetPPPoERelayMode(short int olt_id, int set_mode, int relay_mode)
{
    int iRlt = 0;
    ULONG ulArgv[10] = {RELAY_TYPE_PPPOE,0,0,0,0,0,0,0,0,0};

    OLT_LOCALID_ASSERT(olt_id);

    if ( 0 == (ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv )) )
    {
        iRlt = OLT_ERR_UNKNOEWN;
    }   
    
    OLT_GW_DEBUG(OLT_GW_TITLE"GW_SetPPPoERelayMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, set_mode, relay_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

int GW_SetPPPoERelayParams(short int olt_id, short int param_id, int param_value1, int param_value2)
{
    int iRlt = 0;

    OLT_LOCALID_ASSERT(olt_id);
    
	if (PPPOE_RELAY_ENABLE == g_PPPOE_relay)
	{
        ULONG ulArgv[10] = {RELAY_TYPE_PPPOE,0,0,0,0,0,0,0,0,0};
        
	    if ( 0 == (ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv )) )
        {
            iRlt = OLT_ERR_UNKNOEWN;
        }   
	}
    
    OLT_GW_DEBUG(OLT_GW_TITLE"GW_SetPPPoERelayParams(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, param_id, param_value1, param_value2, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

int GW_SetDhcpRelayMode(short int olt_id, int set_mode, int relay_mode)
{
    int iRlt = 0;

    OLT_LOCALID_ASSERT(olt_id);

    if ((0 == relay_mode) || (DHCP_RELAY_ENABLE == g_DHCP_relay))
    {
        ULONG ulArgv[10] = {RELAY_TYPE_DHCP,0,0,0,0,0,0,0,0,0};

        if ( 0 == (ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv )) )
        {
            iRlt = OLT_ERR_UNKNOEWN;
        }   
    }
    
    OLT_GW_DEBUG(OLT_GW_TITLE"GW_SetDhcpRelayMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, set_mode, relay_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

/*Begin:for onu swap by jinhl@2013-04-27*/
int GW_UpdateProvBWInfo( short int olt_id )
{
    int iRlt = 0;
    OLT_LOCALID_ASSERT(olt_id);

	iRlt = UpdateProvisionedBWInfo(olt_id);

	return iRlt;
}
/*End:for onu swap by jinhl@2013-04-27*/


/*added by liyang 2014-11-17*/
int GW_ChkDBAVersion( short int olt_id, bool *is_compatible)
{
    int iRlt = OLT_ERR_OK;;
	OLT_LOCALID_ASSERT(olt_id);
	
	*is_compatible = TRUE; 
	
	return iRlt;
}

#if defined(_GPON_BCM_SUPPORT_)
int GW_PonLinkFlag(short int olt_id, bool flag)
{
	volatile UCHAR *pCPLDRegPonLink;
	volatile UCHAR *pCPLDRegProtect;
	unsigned char linkEnable = 0;
	unsigned char linkFlag = 0;
   
	 #if 1
	if(olt_id <= 7)
	{
		pCPLDRegPonLink = 0x200024;
	}
	else
	{
		pCPLDRegPonLink = 0x200025;
		olt_id = olt_id - 8;
	}

	pCPLDRegProtect = 0x20000F;
	ReadCPLDReg( pCPLDRegPonLink, &linkEnable );
	linkFlag = 1<< olt_id;

	if(flag)/*点亮pon*/
	{
		/*if(linkEnable & linkFlag)*//*pon灯未up，需要点亮*/
		{
			linkEnable &= (~linkFlag );/*0:link;1:不link*/
	        CPLD_WRITE_LOCK(pCPLDRegProtect);
	        VOS_TaskDelay(0);
	        WriteCPLDReg( pCPLDRegPonLink, linkEnable );
	        CPLD_WRITE_UNLOCK(pCPLDRegProtect);
		}
	}
	else/*灭pon*/
	{
		/*if(!(linkEnable & linkFlag))*//*pon灯未down，需要灭*/
		{
			linkEnable |= linkFlag;
	        CPLD_WRITE_LOCK(pCPLDRegProtect);
	        VOS_TaskDelay(0);
	        WriteCPLDReg( pCPLDRegPonLink, linkEnable );
	        CPLD_WRITE_UNLOCK(pCPLDRegProtect);
		}
	}
	#endif
	return VOS_OK;
	
}
#endif
#endif


#if 1
/* -------------------OLT ONU管理API的GW实现-------------- */
int GW_GetOnuNum(short int olt_id, int onu_flags, int *onu_number)
{
    int onu_num;
    short int OnuIdx;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(onu_number);

    onu_num = 0;
    if ( onu_flags & OLT_ONUFLAG_VALID )
    {
        for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
        {
            if( ThisIsValidOnu(olt_id, OnuIdx) == ROK )
            {
                onu_num++;
            }
        }
    }
    else
    {
        if ( onu_flags & OLT_ONUFLAG_ONREG )
        {
            int onu_status;
            pendingOnu_S *CurOnu;

            for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
            {
                if( ThisIsValidOnu(olt_id, OnuIdx) == ROK )
                {
                    onu_status = GetOnuOperStatus_1(olt_id, OnuIdx);
                    if( (ONU_OPER_STATUS_UP == onu_status)
                         || (ONU_OPER_STATUS_DORMANT == onu_status)
                         )
                    {
                        onu_num++;
                    }
                }      
            }

            VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
            CurOnu = PonPortTable[olt_id].PendingOnu.Next;
            while( CurOnu != NULL )
            {
                onu_num++;
                CurOnu = CurOnu->Next;
            }

            CurOnu = PonPortTable[olt_id].PendingOnu_Conf.Next;
            while( CurOnu != NULL )
            {
                onu_num++;
                CurOnu = CurOnu->Next;
            }
            VOS_SemGive( OnuPendingDataSemId );
        }
        else if ( onu_flags & OLT_ONUFLAG_OFFREG )
        {
            pendingOnu_S *CurOnu;
            int Olt_BaseIdx;
            int onu_status;

            Olt_BaseIdx = olt_id * MAXONUPERPON;    
            for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
            {
                if( ThisIsValidOnu(olt_id, OnuIdx) != ROK ) continue;
                
                onu_status = GetOnuOperStatus_1(olt_id, OnuIdx);
                if( (ONU_OPER_STATUS_UP == onu_status)
                    || (ONU_OPER_STATUS_PENDING == onu_status)
                    || (ONU_OPER_STATUS_DORMANT == onu_status)
                    ) continue;
                     
                VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
                CurOnu = PonPortTable[olt_id].PendingOnu.Next;
                while( CurOnu )
                {
                    if (0 == VOS_MemCmp(OnuMgmtTable[Olt_BaseIdx+OnuIdx].DeviceInfo.MacAddr, CurOnu->OnuMarkInfor.OnuMark.MacAddr, BYTES_IN_MAC_ADDRESS))
                    {
                        break;
                    }
                    CurOnu = CurOnu->Next;
                }

                if ( NULL == CurOnu )
                {
                    CurOnu = PonPortTable[olt_id].PendingOnu_Conf.Next;
                    while( CurOnu )
                    {
                        if (0 == VOS_MemCmp(OnuMgmtTable[Olt_BaseIdx+OnuIdx].DeviceInfo.MacAddr, CurOnu->OnuMarkInfor.OnuMark.MacAddr, BYTES_IN_MAC_ADDRESS))
                        {
                            break;
                        }
                        CurOnu = CurOnu->Next;
                    }
                    if ( NULL == CurOnu )
                    {
                        onu_num++;
                    }
                }      
                VOS_SemGive( OnuPendingDataSemId );
            }
        }
        else
        {
            if ( onu_flags & OLT_ONUFLAG_ONLINE )
            {
                for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
                {
                    if( ThisIsValidOnu(olt_id, OnuIdx) == ROK )
                    {
                        if(GetOnuOperStatus(olt_id, OnuIdx) == ONU_OPER_STATUS_UP)
                        {
                            onu_num++;
                        }
                    }      
                }
            }
            else
            {
                for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
                {
                    if( ThisIsValidOnu(olt_id, OnuIdx) == ROK )
                    {
                        if(GetOnuOperStatus(olt_id, OnuIdx) != ONU_OPER_STATUS_UP)
                        {
                            onu_num++;
                        }
                    }      
                }
            }
        }
    }

    *onu_number = onu_num;
    OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetOnuNum(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_flags, *onu_number, 0, SYS_LOCAL_MODULE_SLOTNO);

    return 0;
}

int GW_ChkOnuRegisterControl(short int olt_id, short int llid, mac_address_t mac_address, short int *bind_olt_id)
{
    int iRlt;
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(mac_address);
    VOS_ASSERT(bind_olt_id);

    *bind_olt_id = OLT_ID_INVALID;
    if ( (iRlt = localOnuRegisterControl(olt_id, llid, mac_address, bind_olt_id)) == RERROR )
    {
        /* 冲突OLT统一用全局OLT_ID定位 */
        *bind_olt_id = GetGlobalPonPortIdx(*bind_olt_id);
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GW_ChkOnuRegisterControl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, *bind_olt_id, 0, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GW_GetPendingOnu(short int olt_id, pendingOnuList_t *onuList)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(onuList);

    iRlt = GetPendingOnu(olt_id, onuList);
    OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetPendingOnu(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GW_GetUpdatingOnu(short int olt_id, onuUpdateStatusList_t *onuList)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(onuList);

    iRlt = getOnuOamUpdatingStatus(olt_id, onuList);
    OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetUpdatingOnu(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return 0;
}

int GW_GetUpdatedOnu(short int olt_id, onuUpdateStatusList_t *onuList)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(onuList);

    iRlt = getOnuOamUpdatedStatus(olt_id, onuList);
    OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetUpdatedOnu(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GW_GetOnuUpdatingStatusLocal( short int olt_id, onu_updating_counter_t *pList )
{
	short int PonPortIdx, OnuIdx;

    OLT_LOCAL_ASSERT(olt_id);

	if( pList == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	pList->count = 0;
	for( PonPortIdx= 0; PonPortIdx < MAXPON; PonPortIdx ++ )
	{
		for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
		{
			if( GetOnuSWUpdateStatus( PonPortIdx, OnuIdx ) == ONU_SW_UPDATE_STATUS_INPROGRESS )
			{
				pList->onuid_list[pList->count] = OnuIdx;
				pList->ponid_list[pList->count] = PonPortIdx;
				pList->count++;
				if( pList->count >= MAX_UPDATE_ONU_NUM )
					return VOS_OK;
			}
		}
	}
	return VOS_OK;
}

extern char convert_onu_file_type[];
int GW_SetOnuUpdateMsg( short int olt_id, onu_update_msg_t *pMsg )
{
    OLT_LOCAL_ASSERT(olt_id);

	if( pMsg == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	/* modified by xieshl 20110802, 问题单11878 */
	if( pMsg->reserved[0] != 0 )
	{
		VOS_StrnCpy( convert_onu_file_type, pMsg->reserved, ONU_TYPE_LEN );
		convert_onu_file_type[ONU_TYPE_LEN] = 0;
	}
	OLT_GW_DEBUG(OLT_GW_TITLE"GW_SetOnuUpdateMsg(%d)'s result(%d) on slot %d.\r\n", olt_id, 0, SYS_LOCAL_MODULE_SLOTNO);
	return sendOnuOamUpdMsg( pMsg->upd_mode, pMsg->msg_type, pMsg->onuDevIdx, pMsg->file_type);
}

int GW_GetOnuUpdateWaiting( short int olt_id, onu_update_waiting_t *pList )
{
	ULONG slot, port;
	USHORT *pWaiting;
	
    OLT_LOCAL_ASSERT(olt_id);

	if( pList == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	slot = GetCardIdxByPonChip(olt_id);
	port = GetPonPortByPonChip(olt_id);
	
	OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetOnuUpdateWaiting(%d)'s result(%d) on slot %d.\r\n", olt_id, 0, SYS_LOCAL_MODULE_SLOTNO);
	pWaiting = getOnuOamUpdWaitingOnuList(slot, port);
	if( pWaiting )
	{
		VOS_MemCpy( pList, pWaiting, sizeof(pList->wait_onu) );
		return VOS_OK;
	}
    
	return VOS_ERROR;
}
/*Begin:for onu swap by jinhl@2013-02-22*/
/*RPC查找非本板的空闲OnuIdx*/
int GW_SearchFreeOnuIdx(short int olt_id, unsigned char *MacAddress, short int *reg_flag)
{
    short int onu_idx = 0;  
    OLT_LOCAL_ASSERT(olt_id);
	if( (MacAddress == NULL) || (reg_flag == NULL) )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	
    onu_idx = SearchFreeOnuIdxForRegister(olt_id, MacAddress, reg_flag);
    OLT_GW_DEBUG(OLT_GW_TITLE"GW_SearchFreeOnuIdx(%d, %d)'s onuIdx(%d) on slot %d.\r\n", olt_id, *reg_flag, onu_idx, SYS_LOCAL_MODULE_SLOTNO);
    
	return onu_idx;
}

int GW_GetActOnuIdxByMac(short int olt_id, unsigned char *MacAddress)
{

	short int onu_idx = 0;
	if( MacAddress == NULL )
	{
		VOS_ASSERT(0);
		return RERROR;
	}

	OLT_LOCAL_ASSERT(olt_id);
	
    onu_idx = GetOnuIdxByMacPerPon(olt_id, MacAddress);
    OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetActOnuIdxByMac(%d)'s onuIdx(%d) on slot %d.\r\n", olt_id, onu_idx, SYS_LOCAL_MODULE_SLOTNO);
    if(INVALID_LLID == GetLlidActivatedByOnuIdx(olt_id, onu_idx))
	{
	    return RERROR;
	}
	return onu_idx;
}

int GW_BroadCast_CliCommand(short int olt_id, int action_code)
{
    int iRlt = 0;
    char *pRecv;
    unsigned short int recvlen = 0;
    char *szSessionName;
    OLT_LOCAL_ASSERT(olt_id);

    if(action_code == 1 || action_code == 2)
    {
        szSessionName = VOS_Malloc(16, MODULE_ONU);
        if(szSessionName)
        {
        	CHAR pBuff[256] = {0};
            int length = 0;
        	length += VOS_Sprintf( pBuff+length, "mgt config ");
        	if( action_code == 1 )
        	{
        		length += VOS_Sprintf( pBuff+length,"save");
        	}
        	else if( action_code == 2 )
        	{
        		length += VOS_Sprintf( pBuff+length, "clear");
        	}

            VOS_StrCpy(szSessionName, "RPC_CMD");
            iRlt = lCli_SendbyOam_Novty_BroadCast(olt_id, 0xFFFF, pBuff, length, &pRecv, &recvlen, szSessionName);
            if(iRlt !=VOS_OK)
            {
                iRlt = OLT_ERR_PARAM;
            }
            else
            {
                if(pRecv)
                    VOS_Free(pRecv);
            }
        }
        else
            iRlt = OLT_ERR_MALLOC;
    }
    return iRlt;
}

/*End:for onu swap by jinhl@2013-02-22*/
#endif


#if 1
/* -------------------OLT 地址表管理API的GW实现----------- */
int GW_AddMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt = 0;
    int i;
    int iAddrIdx;
    
    VOS_ASSERT(addr_num > 0);
    VOS_ASSERT(addr_tbl);

    for (i=0; i<addr_num; i++)
    {
#if 0        
        if ( OLT_ID_ALL == olt_id )
#endif
        {
            if ( 0 > (iAddrIdx = FindGlobalPonMacAddr(addr_tbl[i].mac_address)) )
            {
                if ( GlobalPonAddrTblNum < MAXOLTADDRNUM )
                {
                    VOS_MemCpy(GlobalPonStaticMacAddr[GlobalPonAddrTblNum++], addr_tbl[i].mac_address, BYTES_IN_MAC_ADDRESS);
					/*added by liyang @2015-03-14 for add mac addr port */
					iAddrIdx = GlobalPonAddrTblNum - 1;
				}
                else
                {
                    iRlt = OLT_ERR_NORESC;
                    break;
                }
            }
            GlobalPonStaticMacAddr[iAddrIdx][BYTES_IN_MAC_ADDRESS] = (unsigned char)addr_tbl[i].logical_port;                   
        }
#if 0        
        else
        {
            OLT_LOCAL_ASSERT(olt_id);
            
            if ( 0 > (iAddrIdx = FindPonPortMacAddr(olt_id, addr_tbl[i].mac_address)) )
            {
                if ( PonPortTable[olt_id].AddrTblNum < MAXOLTADDRNUM )
                {
                    VOS_MemCpy(PonPortTable[olt_id].StaticMacAddr[PonPortTable[olt_id].AddrTblNum++], addr_tbl[i].mac_address, BYTES_IN_MAC_ADDRESS);
                }
                else
                {
                    iRlt = OLT_ERR_NORESC;
                    break;
                }
            }
            PonPortTable[olt_id].StaticMacAddr[iAddrIdx][BYTES_IN_MAC_ADDRESS] = (unsigned char)addr_tbl[i].logical_port;                   
        }
#endif
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GW_AddMacAddrTbl(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int GW_DelMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt = 0;
    int i;
    int iAddrIdx;
    
    VOS_ASSERT(addr_num > 0);
    VOS_ASSERT(addr_tbl);

    for (i=0; i<addr_num; i++)
    {
#if 0        
        if ( OLT_ID_ALL == olt_id )
#endif
        {
            if ( 0 <= (iAddrIdx = FindGlobalPonMacAddr(addr_tbl[i].mac_address)) )
            {
                if ( iAddrIdx < (--GlobalPonAddrTblNum) )
                {
                    VOS_MemCpy(GlobalPonStaticMacAddr[iAddrIdx], GlobalPonStaticMacAddr[GlobalPonAddrTblNum], OLT_ADDR_BYTELEN);
                }
            }
        }
#if 0        
        else
        {
            OLT_LOCAL_ASSERT(olt_id);
            if ( 0 <= (iAddrIdx = FindPonPortMacAddr(olt_id, addr_tbl[i].mac_address)) )
            {
                if ( iAddrIdx < (--PonPortTable[olt_id].AddrTblNum) )
                {
                    VOS_MemCpy(PonPortTable[olt_id].StaticMacAddr[iAddrIdx], PonPortTable[olt_id].StaticMacAddr[PonPortTable[olt_id].AddrTblNum], OLT_ADDR_BYTELEN);
                }
            }
        }
#endif
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GW_DelMacAddrTbl(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*add by sxh20120210---------test----------*/
 int GW_OnuMacCheckEnable(short int olt_id, ULONG enable)
{
    int iRlt;
    /*OLT_LOCAL_ASSERT(olt_id);*/

    iRlt=OnuMacCheckEnable(enable);
   OLT_GW_DEBUG(OLT_GW_TITLE"GW_OnuMacCheckEnable(%d, %d)'s result(%d) on slot %d.\r\n", olt_id,enable,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
 int GW_OnuMacCheckPeriod(short int olt_id, ULONG period)
{
    int iRlt;
    /*OLT_LOCAL_ASSERT(olt_id);*/

    iRlt=OnuMacCheckPeriod(period);
   OLT_GW_DEBUG(OLT_GW_TITLE"GW_OnuMacCheckPeriod(%d, %d)'s result(%d) on slot %d.\r\n", olt_id,period,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif


#if 1
/* -------------------OLT 光路管理API------------------- */
int GW_GPONTxEnable(int olt_id, int tx_mode )
{
    int iRlt = OLT_ERR_OK;
    volatile UCHAR *pCPLDRegPonTx;
	volatile UCHAR *pCPLDRegProtect;
	volatile UCHAR *pCPLDRegPonVccT;
	unsigned char txEnable = 0;
	unsigned char vccEnable = 0;
	unsigned char txFlag = 0;
    OLT_LOCAL_ASSERT(olt_id);
	 
	if(olt_id <= 7)
	{
		pCPLDRegPonTx = 0x20000A;
		pCPLDRegPonVccT = 0x20000C;
	}
	else
	{
		pCPLDRegPonTx = 0x20000B;
		pCPLDRegPonVccT = 0x20000D;
		olt_id = olt_id -8;
	}

	pCPLDRegProtect = 0x20000F;
	ReadCPLDReg( pCPLDRegPonTx, &txEnable );
	ReadCPLDReg( pCPLDRegPonVccT, &vccEnable);
	txFlag = 1<< olt_id;
	if(tx_mode)/*打开发光*/
	{
		if(txEnable & txFlag)/*0:on;1:off*/
		{
			txEnable &= ~txFlag;
			CPLD_WRITE_LOCK(pCPLDRegProtect);
	        VOS_TaskDelay(0);
	        WriteCPLDReg( pCPLDRegPonTx, txEnable );
	        CPLD_WRITE_UNLOCK(pCPLDRegProtect);
		}

		if(!(vccEnable & txFlag))/*0:off;1:on*/
		{
			vccEnable |= txFlag;
			CPLD_WRITE_LOCK(pCPLDRegProtect);
	        VOS_TaskDelay(0);
	        WriteCPLDReg( pCPLDRegPonVccT, vccEnable );
	        CPLD_WRITE_UNLOCK(pCPLDRegProtect);
		}
	}
	else/*关闭发光*/
	{
		if(!(txEnable & txFlag))
		{
			txEnable |= txFlag;
			CPLD_WRITE_LOCK(pCPLDRegProtect);
	        VOS_TaskDelay(0);
	        WriteCPLDReg( pCPLDRegPonTx, txEnable );
	        CPLD_WRITE_UNLOCK(pCPLDRegProtect);
		}

		if(vccEnable & txFlag)/*0:off;1:on*/
		{
			vccEnable &= ~txFlag;
			CPLD_WRITE_LOCK(pCPLDRegProtect);
	        VOS_TaskDelay(0);
	        WriteCPLDReg( pCPLDRegPonVccT, vccEnable );
	        CPLD_WRITE_UNLOCK(pCPLDRegProtect);
		}
		
		
	}
	/*不支持*/
  
    return iRlt;
}
int GW_SetOpticalTxMode2(short int olt_id, int tx_mode, int tx_reason)
{
    int iRlt;
    unsigned char SetOpticalTxMode_flag = 0;
    
    OLT_LOCAL_ASSERT(olt_id);

    if ( (PONPORT_TX_DIRECT < tx_reason) && (tx_reason < PONPORT_TX_ALL) )
    {
		SetOpticalTxMode_flag = PonPortTable[olt_id].TxCtrlFlag | (~tx_reason);/*modefied by yangzl@2016-2-29*/
		SetOpticalTxMode_flag &= tx_reason;
	    if ( tx_mode )
	    {
			 if (SetOpticalTxMode_flag )
			{
			    iRlt = TRUE;
			}
			else
			{
			    iRlt = FALSE;
			}
	 	}
		else
		{
			if ( SetOpticalTxMode_flag)
			{
		        iRlt = FALSE;
			}
			else
			{
			    iRlt = TRUE;
			}
		}
	}
    else
    {
        iRlt = TRUE;
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GW_SetOpticalTxMode2(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, tx_mode, tx_reason, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif

void GW_DevResetHard1()
{
	unsigned char ResetFlag = 0;
	unsigned char ResetEnable = 0;

	volatile UCHAR *pCPLDRegProtect;
    volatile UCHAR *pCPLDRegPonReset;

	pCPLDRegProtect  = 0x0f;
    pCPLDRegPonReset = 0x05;
	
	ReadCPLDReg( pCPLDRegPonReset, &ResetEnable );
	ResetFlag = 0x7f;
	
    ResetEnable |= (~ResetFlag);

    CPLD_WRITE_LOCK(pCPLDRegProtect);
    VOS_TaskDelay(0);
    WriteCPLDReg( pCPLDRegPonReset, ResetEnable );
    CPLD_WRITE_UNLOCK(pCPLDRegProtect);
	 VOS_TaskDelay(100);
}

void GW_DevResetHard2()
{
	unsigned char ResetFlag = 0;
	unsigned char ResetEnable = 0;

	volatile UCHAR *pCPLDRegProtect;
    volatile UCHAR *pCPLDRegPonReset;

	pCPLDRegProtect  = 0x0f;
    pCPLDRegPonReset = 0x05;
	
	ReadCPLDReg( pCPLDRegPonReset, &ResetEnable );
	ResetFlag = 0x80;
	
    ResetEnable &= (~ResetFlag);

    CPLD_WRITE_LOCK(pCPLDRegProtect);
    VOS_TaskDelay(0);
    WriteCPLDReg( pCPLDRegPonReset, ResetEnable );
    CPLD_WRITE_UNLOCK(pCPLDRegProtect);
	 VOS_TaskDelay(100);
}

#if 1
/* -------------------OLT 倒换API的GW实现----------------- */
int GW_SetHotSwapParam(short int olt_id, int swap_enable, int swap_time, int rpc_mode, int swap_triggers, int trigger_param1, int trigger_param2)
{
    int iRlt = 0;

    OLT_LOCALID_ASSERT(olt_id);
    
#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
	if ((0 < swap_enable) && (pon_swap_switch_enable != swap_enable))
    {
        if ( V2R1_ENABLE == swap_enable )
        {
            iRlt = redundancy_enable();
        }
        else
        {
            iRlt = redundancy_disable();
        }
    }
#endif

    OLT_GW_DEBUG(OLT_GW_TITLE"GW_SetHotSwapParam(%d, %d, %d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, swap_enable, swap_time, rpc_mode, swap_triggers, trigger_param1, trigger_param2, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

int GW_GetHotSwapMode(short int olt_id, short int *partner_olt_id, int *swap_mode, int *swap_status)
{
    int iRlt = 0;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(partner_olt_id);
    VOS_ASSERT(swap_mode);
    VOS_ASSERT(swap_status);

    if ( V2R1_PON_PORT_SWAP_DISABLE == PonPortTable[olt_id].swap_flag )
    {
        *swap_mode      = V2R1_PON_PORT_SWAP_DISABLED;
        *swap_status    = V2R1_PON_PORT_SWAP_UNKNOWN;
        *partner_olt_id = OLT_ID_INVALID;
    }
    else
    {
        *swap_mode   = PonPortTable[olt_id].swap_mode;
        *swap_status = PonPortTable[olt_id].swap_use;
    	if ( RERROR == (*partner_olt_id = GetPonPortIdxBySlot((short int)(PonPortTable[olt_id].swap_slot), (short int)(PonPortTable[olt_id].swap_port))) )
        {
            iRlt = OLT_ERR_UNKNOEWN;
        }
    }

    /* OLT_GW_DEBUG(OLT_GW_TITLE"GW_GetHotSwapMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, *partner_olt_id, *swap_mode, *swap_status, iRlt, SYS_LOCAL_MODULE_SLOTNO); */

    return iRlt;
}

int GW_RdnIsReady(short int olt_id, int *iIsReady)
{
    int iRlt = 0;
    int status;

    OLT_LOCALID_ASSERT(olt_id);
    VOS_ASSERT(iIsReady);

    *iIsReady = FALSE;

    GW_GetAdminStatus(olt_id, &status);
    if ( PONPORT_ENABLE == status )
    {
        GW_GetPonWorkStatus(olt_id, &status);
        if ( PONPORT_UP == status )
        {
            short int partner_olt;
            int swap_mode, swap_status;
            
            GW_GetHotSwapMode(olt_id, &partner_olt, &swap_mode, &swap_status);
            if ( swap_mode >= V2R1_PON_PORT_SWAP_SLOWLY )
            {
                /* B--added by liwei056@2011-11-22 for D13656 */
                if ( (V2R1_SWAPPING_NO == PonPortTable[olt_id].swapping)
                    && (FALSE == PonPortTable[olt_id].swapHappened) )
                /* E--added by liwei056@2011-11-22 for D13656 */
                {
                    *iIsReady = TRUE;
                }
            }
        }
    }

    OLT_GW_DEBUG(OLT_GW_TITLE"GW_RdnIsReady(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *iIsReady, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif

#if 1
#define GPON_GEMPORT_BASE 289
#define MAX_GEMPORT_PER_ONU 16
extern ULONG UNDO_UPLINK;
extern ULONG UNDO_DOWNLINK;
extern ULONG GPON_QINQ_DEBUG;
extern int vlanportid[45][2350];
extern LONG devsm_sys_is_switchhovering();
ULONG ulGPON_GEMPORT_BASE = GPON_GEMPORT_BASE;

int GWGPON_SetVlanQinQ(short int olt_id, OLT_vlan_qinq_t *vlan_qinq_config)
{
    int iRlt = OLT_ERR_OK;
    short int onuIdx = 0;
	short int llid = 0;
    ULONG brdIdx = 0;   
    ULONG ponIdx = 0;
	ULONG unit = 0;
	ULONG port = 0;
	int gemvid[64] = {0};
	ULONG gemvidnum = 0;
    int gemPortList[64] = {0};
    int gemPortStatList[64] = {0};
    int gemPortNum = 0;    
	int i;
    PON_olt_vlan_uplink_config_t *pQinQCfg;
    PON_vlan_tag_t newvlan = 0;
	PON_vlan_tag_t prevlan = 0;
	Olt_llid_vlan_manipulation vlan_qinq_policy;
	int preoriention = 0;
	
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(vlan_qinq_config);

	if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n%s, brdIdx:%d, switchoverring:%d \r\n",__FUNCTION__,  brdIdx, devsm_sys_is_switchhovering);}
	if(devsm_sys_is_switchhovering())
	{
		return VOS_OK;
	}

    brdIdx = GetCardIdxByPonChip(olt_id);
    ponIdx = GetPonPortByPonChip(olt_id);

	
	unit = USER_PORT_2_SWITCH_UNIT(brdIdx, ponIdx);
	port = USER_PORT_2_SWITCH_PORT(brdIdx, ponIdx);
	if(0xff==unit || 0xff==port)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	
    if ( OLT_CFG_DIR_UPLINK == vlan_qinq_config->qinq_direction )
    {
	    onuIdx = (vlan_qinq_config->qinq_objectid);
		llid = GetLlidByOnuIdx(olt_id, onuIdx);
		newvlan = vlan_qinq_config->qinq_cfg.up_cfg.new_vlan_tag_id;
		
		if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n slot = %d,port = %d,onu = %d,llid = %d,newvlan = %d\r\n",brdIdx,ponIdx,onuIdx,llid,newvlan);}

		if(llid == INVALID_LLID)
			return VOS_ERROR;
		
		/*iRlt = gponOnuAdp_GetAllGem(olt_id, llid, &gemvidnum, gemvid);*/
		gemvidnum = 1;
		gemvid[0] = GPON_GEMPORT_BASE + (llid-1)*MAX_GEMPORT_PER_ONU;

    	/*begin: 解决ONU重新分配非缺省的gem port后qinq配置没有生效的问题，add by @liuyh, 2017-7-31*/
        iRlt = gponOnuAdp_GetAllGem(olt_id, llid, &gemPortNum, gemPortList, gemPortStatList);
        if (VOS_OK == iRlt)
        {
            if (GPON_QINQ_DEBUG) 
            {
                sys_console_printf("\r\n Gem port num: %d\r\n", gemPortNum);
                for (i = 0; i < gemPortNum; i++)
                {
                    sys_console_printf(" gem port: %d, stat: %d\r\n", gemPortList[i], gemPortStatList[i]);
                }
            }            
            
            /* 对状态为active的第一个gem port进行配置 */
            for (i = 0; i < gemPortNum; i++)
            {
                if (gemPortStatList[i] == 3/*BCMOLT_GPON_GEM_PORT_STATE_ACTIVE*/)
                {
                    gemvidnum = 1;
                    gemvid[0] = gemPortList[i];
                    break;
                }
            }
        }
        /*end: add by @liuyh, 2017-7-31*/
        
		if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n &gemvid:0x%x\r\n", gemvid);}

		
		if(vlan_qinq_config->qinq_cfg.up_cfg.vlan_manipulation == PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED)
		{
			GetPonUplinkVlanQinQ(olt_id, onuIdx, &vlan_qinq_policy);
			prevlan = vlan_qinq_policy.new_vlan_id;
			preoriention = vlan_qinq_policy.vlan_manipulation;
			for(i=0; i<gemvidnum; i++)
			{
				if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n line:%d, slot = %d,port = %d,prevlan = %d, prepolicy = %d \r\n",__LINE__,  brdIdx,ponIdx,prevlan,preoriention);}
				if((0!=prevlan) && (PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED==preoriention))
				{	
					if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n line:%d,  &vlanportid[%d][%d]:%d\r\n",__LINE__,  port,gemvid[i], vlanportid[port][gemvid[i]]);}
					if(0 != vlanportid[port][gemvid[i]])
					{
						iRlt = bms_vlan_gpon_qinq_destroy(unit, port, gemvid[i], (int)prevlan);
						if(VOS_OK != iRlt)
						{
							VOS_ASSERT(0);
						}
					}
				}
				if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n qing_set u:%d, p:%d, gvid:%d, nvid:%d\r\n",unit, port, gemvid[i], (int)newvlan);}
				iRlt = bms_vlan_gpon_qinq_set(unit, port, gemvid[i], (int)newvlan);
				if(OLT_ERR_OK != iRlt)
				{
					VOS_ASSERT( 0 );
				}
			}
		}
		else if(vlan_qinq_config->qinq_cfg.up_cfg.vlan_manipulation == PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION)
		{
			if(UNDO_UPLINK)
			{
				GetPonUplinkVlanQinQ(olt_id, onuIdx, &vlan_qinq_policy);
				newvlan = vlan_qinq_policy.new_vlan_id;
				for(i=0; i<gemvidnum; i++)
				{
					if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n undo uplink u:%d, p:%d, gvid:%d, nvid:%d\r\n",unit, port, gemvid[i], (int)newvlan);}
					iRlt = bms_vlan_gpon_qinq_undo_uplink(unit, port, gemvid[i], (int)newvlan);
					if(OLT_ERR_OK != iRlt)
					{
						VOS_ASSERT( 0 );
					}
				}
			}
			else
			{
				iRlt = OLT_ERR_OK;
			}
		}
    }
	else
	{

		if(vlan_qinq_config->qinq_cfg.up_cfg.vlan_manipulation  == PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG)
		{
			iRlt = OLT_ERR_OK;
		}
		else if(vlan_qinq_config->qinq_cfg.up_cfg.vlan_manipulation == PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION)
		{
			/*GetPonDownlinkVlanQinQ(olt_id, onuIdx, &vlan_qinq_policy);*/
			newvlan = vlan_qinq_config->qinq_objectid;
		
			if(UNDO_DOWNLINK)
			{
				if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n undo downlink u:%d, p:%d, gvid:%d, nvid:%d\r\n",unit, port, gemvid[i], (int)newvlan);}
				iRlt = bms_vlan_gpon_qinq_undo_downlink(unit, port, 0, (int)newvlan);
				if(OLT_ERR_OK != iRlt)
				{
					VOS_ASSERT( 0 );
				}
			}
			else
			{
				iRlt = OLT_ERR_OK;
			}
		}
	}

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetVlanQinQ(%d, %d, %d, %d, %d )'s result(%d) on slot %d.\r\n", 

	olt_id, brdIdx,ponIdx,onuIdx,newvlan, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
    return iRlt;
}
int GWGPON_DelSwitchVlanQinQ(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_OK;
	short int llid = 0;
    ULONG brdIdx = 0;   
    ULONG ponIdx = 0;
	ULONG unit = 0;
	ULONG port = 0;
	int gemvid[64] = {0};
	ULONG gemvidnum = 0;
    int gemPortId = 0; 
	int i = 0;
    PON_vlan_tag_t newvlan = 0;
	Olt_llid_vlan_manipulation vlan_qinq_policy;
	
    OLT_LOCAL_ASSERT(olt_id);

    brdIdx = GetCardIdxByPonChip(olt_id);
    ponIdx = GetPonPortByPonChip(olt_id);

	
	unit = USER_PORT_2_SWITCH_UNIT(brdIdx, ponIdx);
	port = USER_PORT_2_SWITCH_PORT(brdIdx, ponIdx);
	if(0xff==unit || 0xff==port)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	
	GetPonUplinkVlanQinQ(olt_id, onu_id, &vlan_qinq_policy);
	newvlan = vlan_qinq_policy.new_vlan_id;

	if(vlan_qinq_policy.vlan_manipulation == PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED)
	{
		llid = GetLlidByOnuIdx(olt_id, onu_id);
		
		if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n slot = %d,port = %d,onu = %d,llid = %d,newvlan = %d\r\n",brdIdx,ponIdx,onu_id+1,llid,newvlan);}

		if(llid == INVALID_LLID)
			return VOS_ERROR;
		
		/*iRlt = gponOnuAdp_GetAllGem(olt_id, llid, &gemvidnum, gemvid);*/
        /*begin: 解决ONU重新分配非缺省的gem port后qinq配置没有生效的问题，add by @liuyh, 2017-7-31*/
        gemPortId = GPON_GEMPORT_BASE + (llid-1)*MAX_GEMPORT_PER_ONU;
        gemvid[0] = gemPortId;
        for (i = 0; i < MAX_GEMPORT_PER_ONU; i++)
        {
            gemPortId += i;
            if (0 != vlanportid[port][gemPortId])
            {
                if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n gemvid[0]: %d, vlanportid %d\r\n", gemPortId, vlanportid[port][gemPortId]);}
                
                gemvid[0] = gemPortId;
                break;
            }
        }        
		/*end: add by @liuyh, 2017-7-31*/

		if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n &gemvid:0x%x\r\n", gemvid);}
	
		if(UNDO_DOWNLINK && (0 != vlanportid[port][gemvid[0]]))/*modeified by yangzl@2016-9-2 Q:32363&28931*/
		{
			if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n u:%d, p:%d, gvid:%d, nvid:%d\r\n",unit, port, gemvid[0], (int)newvlan);}			
			iRlt = bms_vlan_gpon_qinq_destroy(unit, port, gemvid[0], (int)newvlan);
			if(OLT_ERR_OK != iRlt)
			{
				VOS_ASSERT( 0 );
				return iRlt;
			}
		}
		else
		{
			iRlt = OLT_ERR_OK;
		}
	}

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetVlanQinQ(%d, %d, %d, %d, %d )'s result(%d) on slot %d.\r\n", 

	olt_id, brdIdx,ponIdx,onu_id,newvlan, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
    return iRlt;
}
#endif

#endif



#if 1
/* -------------------OLT API接口并且封装共有实现------------------- */
#define OLT_SYS_CALL(fun, params)               (( (NULL != s_pSysOltIfs) && (NULL != s_pSysOltIfs->fun) ) ? (*s_pSysOltIfs->fun) params : OLT_ERR_NOTSUPPORT)
#define OLT_LOCAL_CALL(oltid, fun, params)      (( (NULL != PonPortTable[oltid].OltIFs) && (NULL != PonPortTable[oltid].OltIFs->fun) ) ? (*PonPortTable[oltid].OltIFs->fun) params : OLT_ERR_NOTSUPPORT)
#define OLT_REMOTE_CALL(oltid, fun, params)     (( (NULL != s_pRemoteOltIfs) && (NULL != s_pRemoteOltIfs->fun) ) ? ((0 == (iRlt = (*s_pRemoteOltIfs->fun) params)) ? OLT_ERR_REMOTE_OK : iRlt) : OLT_ERR_NOTSUPPORT)
#define OLT_API_CALL(oltid, fun, params)        ( OLT_ISLOCAL(oltid) ? OLT_LOCAL_CALL(oltid, fun, params) : OLT_REMOTE_CALL(oltid, fun, params) )

#define OLT_REMOTE_CALL2(oltid, fun, params)    (( (NULL != s_pRemoteOltIfs) && (NULL != s_pRemoteOltIfs->fun) ) ? (*s_pRemoteOltIfs->fun) params : OLT_ERR_NOTSUPPORT)
#define OLT_API_CALL2(oltid, fun, params)        ( OLT_ISLOCAL(oltid) ? OLT_LOCAL_CALL(oltid, fun, params) : OLT_REMOTE_CALL2(oltid, fun, params) )

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

#define OLT_PARTNER_CALL_BEGIN		if((SYS_LOCAL_MODULE_ISMASTERACTIVE \
			&& (VOS_OK == (PonPortSwapPortQuery(olt_id, &olt_id_swap))) \
			&& (V2R1_PON_PORT_SWAP_ACTIVE == (PonPortHotStatusQuery(olt_id))))) {	

#define OLT_PARTNER_CALL_END }



#if 1
/* -------------------OLT基本API------------------- */

int OLT_IsExist(short int olt_id, bool *status)
{
    OLT_ASSERT(olt_id);
    VOS_ASSERT(status);
    
    return OLT_API_CALL2( olt_id, IsExist, (olt_id, status) ); 
}

int OLT_GetChipTypeID(short int olt_id, int *type)
{
    int iRlt = 0;
    int iType;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(type);
    if ( OLT_ISLOCAL(olt_id) )
    {
        if ( PONCHIP_UNKNOWN != (iType = PonChipMgmtTable[olt_id].Type) )
        {
            *type = iType;
        }
        else
        {
            if ( 0 == (iRlt = OLT_API_CALL2( olt_id, GetChipTypeID, (olt_id, type) )) )
            {
                PonChipMgmtTable[olt_id].Type = *type;
            }
        }
    }
    else
    {
        iRlt = OLT_API_CALL2( olt_id, GetChipTypeID, (olt_id, type) ); 
    }

    if ( 0 != iRlt )
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetChipTypeID(%d) Error Rlt(%d).", olt_id, iRlt);
    }
    
    return iRlt; 
}

int OLT_GetChipTypeName(short int olt_id, char typename[OLT_CHIP_NAMELEN])
{
    int iRlt = 0;
    char *pszTypeName;

    OLT_ASSERT(olt_id);
    if ( OLT_ISLOCAL(olt_id) )
    {
        pszTypeName = PonChipMgmtTable[olt_id].TypeName;
        if ( '\0' != pszTypeName[0] )
        {
            VOS_MemZero(typename, OLT_CHIP_NAMELEN );
            VOS_StrnCpy(typename, pszTypeName, OLT_CHIP_NAMELEN-1);
        }
        else
        {
            if ( 0 == (iRlt = OLT_API_CALL2( olt_id, GetChipTypeName, (olt_id, typename) )) )
            {
            	  VOS_MemZero(PonChipMgmtTable[olt_id].TypeName,OLT_CHIP_NAMELEN);
                VOS_StrnCpy(PonChipMgmtTable[olt_id].TypeName, typename, OLT_CHIP_NAMELEN-1);
            }
        }
    }
    else
    {
        iRlt = OLT_API_CALL2( olt_id, GetChipTypeName, (olt_id, typename) ); 
    }

    if ( 0 != iRlt )
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetChipTypeName(%d) Error Rlt(%d).", olt_id, iRlt);
    }
    
    return iRlt; 
}

int OLT_ResetPon(short int olt_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int OnuIdx;
    int iErrRlt;
    int OltSlot;
    int i, n;
    short int oltid,partner_olt_id = 0;
    short int olt_ids[MAXOLTPERPONCHIP];
        
    OLT_ASSERT(olt_id);

    n = GetPonChipPonPorts(olt_id, olt_ids);
    if ( n > 0 )
    {
        OltSlot = GetCardIdxByPonChip(olt_id);
		if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && ((OltSlot == SYS_LOCAL_MODULE_SLOTNO) || (!SYS_MODULE_SLOT_ISHAVECPU(OltSlot))) )
        {
			/*B:added by wangjiah@2017-05-26 to solve issue 38656*/
			if(SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
			{
				/*make 8exp support single pon port resetting*/
				n = 1;
				olt_ids[0] = olt_id;
			}
			for ( i = 0; i < n; i++ )
			{
				oltid = olt_ids[i];
				if(OLT_ERR_OK == PonPortSwapPortQuery(oltid, &partner_olt_id)
						&& V2R1_PON_PORT_SWAP_ACTIVE == PonPortHotStatusQuery(oltid)
						&& V2R1_PON_PORT_SWAP_SLOWLY == GetPonPortHotSwapMode(oltid))
				{
					if(SYS_LOCAL_MODULE_TYPE_IS_16EPON && !IsPonPortPairOnSameChip(oltid, partner_olt_id))
					{/*do nothing*/}
					else if(SYS_LOCAL_MODULE_TYPE_IS_12EPON || SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
					{/*do nothing*/}
					else
					{
						continue;
					}
					ProcessHotSwap(oltid, partner_olt_id, PROTECT_SWITCH_REASON_OLTREMOVE);
				}
				else{/*do nothing*/}
			}
			/*E:added by wangjiah@2017-05-26 to solve issue 38656*/

            for ( i = 0; i < n; i++ )
            {
                oltid = olt_ids[i];
                if ( OLT_ERR_OK == (iErrRlt = OLT_API_CALL( oltid, ResetPon, (oltid) )) )
                {
                    /* 同一芯片的OLT，有一个复位成功，就认为全都复位成功 */
                    iRlt = 0;
                }
            }
        }
        else
        {
			iRlt = OLT_API_CALL( olt_id, ResetPon, (olt_id) );
        }
    }
    #if 0
	if(PONCHIP_GPON != OLTAdv_GetChipTypeID)/*by jinhl*/
	#endif
	{
		/* added by wangjiah@2016-08-12 to solve issue-23072 : begin*/
		if ( (0 == iRlt) && (PONCHIP_BCM55538 == OLTAdv_GetChipTypeID(olt_id)) )
		{
			OnuEvent_ClearRunningDataMsg_Send(olt_id);
			ClearPonPortRunningData( olt_id );
			PonChipMgmtTable[olt_id].operStatus = PONCHIP_ONLINE;
			PonChipMgmtTable[olt_id].Type = PONCHIP_UNKNOWN;
			PonChipMgmtTable[olt_id].version = PONCHIP_UNKNOWN;
			PonChipMgmtTable[olt_id].Err_counter = 0;
			OLTAdv_NotifyOltInvalid(olt_id);
		}
		/* added by wangjiah@2016-08-12 to solve issue-23072 : end*/
		else if ( (0 == iRlt) || (PONCHIP_GPON == OLTAdv_GetChipTypeID(olt_id)) )
		{
			delPonPortFromLoopListByOnu(olt_id);
			for ( i = 0; i < n; i++ )
			{
				oltid = olt_ids[i];

				/* modified by xieshl 20110624, 问题单13090 */
				/*modi by luh 2012-11-15,只有6900主控板会直接清onu状态，6900epon和6700、6100均是发送到onu任务清状态及上报trap*/
				OnuEvent_ClearRunningDataMsg_Send(oltid);
				ClearPonPortRunningData( oltid );
				PonChipMgmtTable[oltid].operStatus = PONCHIP_ONLINE;
				PonChipMgmtTable[oltid].Type = PONCHIP_UNKNOWN;
				PonChipMgmtTable[oltid].version = PONCHIP_UNKNOWN;
				PonChipMgmtTable[oltid].Err_counter = 0;

				/* B--added by liwei056@2011-11-22 for D13853 */
				OLTAdv_NotifyOltInvalid(oltid);
				/* E--added by liwei056@2011-11-22 for D13853 */
			}
		}
		else
		{
			if ( OLT_CALL_ISERROR(iRlt) )
			{
				VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ResetPon(%d) Error Rlt(%d).", olt_id, iRlt);
			}
			else
			{
				iRlt = OLT_ERR_OK;
			}
		}
	}
    return iRlt;
}

/*olt_id为每个pon芯片的第一个口 by jinhl*/
int OLT_ResetPonChip(short int olt_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int OnuIdx;
    int iErrRlt;
    int OltSlot;
    int i, n;
    short int oltid;
    short int olt_ids[MAXOLTPERPONCHIP];
        
    n = GetPonChipPonPorts(olt_id, olt_ids);
    if ( n > 0 )
    {
		/* added by wangjiah@2016-08-12 to support 8xep pon chip reset : begin*/
		if ( SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON )
		{
   			if ( OLT_ERR_OK == (iErrRlt = OLT_API_CALL( olt_id, ResetPonChip, (olt_id) )) )
			{
				iRlt = 0;
            }
		}
		/* added by wangjiah@2016-08-12 to support 8xep pon chip reset : end*/
		else
		{
			iRlt = OLT_API_CALL( olt_id, ResetPonChip, (olt_id) );
		}
        
    }
    
    if ( (0 == iRlt) || (PONCHIP_GPON == OLTAdv_GetChipTypeID(olt_id)) )
    {
        delPonPortFromLoopListByOnu(olt_id);
        for ( i = 0; i < n; i++ )
        {
            oltid = olt_ids[i];
	        
        	/* modified by xieshl 20110624, 问题单13090 */
            /*modi by luh 2012-11-15,只有6900主控板会直接清onu状态，6900epon和6700、6100均是发送到onu任务清状态及上报trap*/
       	    OnuEvent_ClearRunningDataMsg_Send(oltid);
        	ClearPonPortRunningData( oltid );
        	PonChipMgmtTable[oltid].operStatus = PONCHIP_ONLINE;
        	PonChipMgmtTable[oltid].Type = PONCHIP_UNKNOWN;
        	PonChipMgmtTable[oltid].version = PONCHIP_UNKNOWN;
        	PonChipMgmtTable[oltid].Err_counter = 0;

            /* B--added by liwei056@2011-11-22 for D13853 */
            OLTAdv_NotifyOltInvalid(oltid);
            /* E--added by liwei056@2011-11-22 for D13853 */
        }
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ResetPonChip(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	
    return iRlt;
}

int OLT_RemoveOlt(short int olt_id, bool send_shutdown_msg_to_olt, bool reset_olt)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, RemoveOlt, (olt_id, send_shutdown_msg_to_olt, reset_olt) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_RemoveOlt(%d, %d, %d) Error Rlt(%d).", olt_id, send_shutdown_msg_to_olt, reset_olt, iRlt);
    }

    return iRlt;
}

int OLT_CopyOlt(short int olt_id, short int dst_olt_id, int copy_flags)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    OLT_ASSERT(dst_olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, CopyOlt, (olt_id, dst_olt_id, copy_flags) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_CopyOlt(%d, %d, %d) Error Rlt(%d).", olt_id, dst_olt_id, copy_flags, iRlt);
    }

    return iRlt;
}

int OLT_CmdIsSupported(short int olt_id, short int *cmd)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(cmd);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, CmdIsSupported, (olt_id, cmd) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTSUPPORT != iRlt )
    	{
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_CmdIsSupported(%d, %d) Error Rlt(%d).", olt_id, *cmd, iRlt);
        }
    }

    return iRlt;
}

int OLT_SetDebugMode(short int olt_id, int debug_flags, int debug_mode)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetDebugMode, (olt_id, debug_flags, debug_mode) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetDebugMode, (olt_id, debug_flags, debug_mode) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetDebugMode, (olt_id, debug_flags, debug_mode) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        if ( debug_flags & PON_DEBUGFLAG_DEBUG )
        {
            SetGeneralEvent(debug_mode);
        }

        if ( debug_flags & PON_DEBUGFLAG_ENCRYPT )
        {
            SetEncryptEvent(debug_mode);
        }

        if ( debug_flags & PON_DEBUGFLAG_REGISTER )
        {
            SetRegisterEvent(debug_mode);
        }

        if ( debug_flags & PON_DEBUGFLAG_ALARM )
        {
            SetAlarmEvent(debug_mode);
        }

        if ( debug_flags & PON_DEBUGFLAG_RESET )
        {
            SetResetEvent(debug_mode);
        }

        if ( debug_flags & PON_DEBUGFLAG_ADD )
        {
            SetAddPonEvent(debug_mode);
        }

        if ( debug_flags & PON_DEBUGFLAG_SWITCH )
        {
            SetSwitchPonEvent(debug_mode);
        }

        if ( debug_flags & PON_DEBUGFLAG_FILE )
        {
            EVENT_UPDATE_ONU_FILE = debug_mode;
        }

        if ( debug_flags & PON_DEBUGFLAG_RPC )
        {
            SetRpcEvent(debug_mode);
        }

        if ( debug_flags & PON_DEBUGFLAG_REMOTE )
        {
            SetRemoteEvent(debug_mode);
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetDebugMode(%d, %d, %d) Error Rlt(%d).", olt_id, debug_flags, debug_mode, iRlt);
    }

    return iRlt;
}

int OLT_SetInitParams(short int olt_id, unsigned short host_olt_manage_type, unsigned short host_olt_manage_address)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetInitParams, (olt_id, host_olt_manage_type, host_olt_manage_address) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL(SetInitParams, (olt_id, host_olt_manage_type, host_olt_manage_address) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡上的PAS_SOFT系统参数 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetInitParams, (olt_id, host_olt_manage_type, host_olt_manage_address) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
    	PON_init_params.host_manage_iftype  = host_olt_manage_type;
    	PON_init_params.host_manage_address = host_olt_manage_address;
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetInitParams(%d, %d, 0x%x) Error Rlt(%d).", olt_id, host_olt_manage_type, host_olt_manage_address, iRlt);
    }

    return iRlt;
}

int OLT_SetSystemParams(short int olt_id, long int statistics_sampling_cycle, long int monitoring_cycle, short int host_olt_msg_timeout, short int olt_reset_timeout)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetSystemParams, (olt_id, statistics_sampling_cycle, monitoring_cycle, host_olt_msg_timeout, olt_reset_timeout) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetSystemParams, (olt_id, statistics_sampling_cycle, monitoring_cycle, host_olt_msg_timeout, olt_reset_timeout) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡上的PAS_SOFT系统参数 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetSystemParams, (olt_id, statistics_sampling_cycle, monitoring_cycle, host_olt_msg_timeout, olt_reset_timeout) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        if (statistics_sampling_cycle > 0)
        	PAS_system_parameters.statistics_sampling_cycle  = statistics_sampling_cycle ;

        if (monitoring_cycle > 0)
        	PAS_system_parameters.monitoring_cycle           = monitoring_cycle ;

        if (host_olt_msg_timeout > 0)
        	PAS_system_parameters.host_olt_msgs_timeout      = host_olt_msg_timeout;	

        if (olt_reset_timeout > 0)
        	PAS_system_parameters.olt_reset_timeout          = olt_reset_timeout ;  

        PAS_system_parameters.automatic_authorization_policy = PAS_init_para.automatic_authorization_policy;
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetSystemParams(%d, %d, %d, %d, %d) Error Rlt(%d).", olt_id, statistics_sampling_cycle, monitoring_cycle, host_olt_msg_timeout, olt_reset_timeout, iRlt);
    }

    return iRlt;
}


int OLT_SetPonI2CExtInfo(short int olt_id, eeprom_gfa_epon_ext_t *pon_info)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(pon_info);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetPonI2CExtInfo, (olt_id, pon_info) )) )
    {
        int slot, port;
        eeprom_gfa_epon_ext_t *pPonExtInfo;
        
        slot = GetCardIdxByPonChip(olt_id);
        port = GetPonPortByPonChip(olt_id);
        
        pPonExtInfo = (eeprom_gfa_epon_ext_t*)g_sysinfo_module[slot].ext_data;
		VOS_MemCpy( (VOID*)&pPonExtInfo[port-1], (VOID*)pon_info, sizeof(eeprom_gfa_epon_ext_t) );
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetPonI2CExtInfo(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_GetPonI2CExtInfo(short int olt_id, eeprom_gfa_epon_ext_t *pon_info)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(pon_info);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetPonI2CExtInfo, (olt_id, pon_info) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTSUPPORT != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetPonI2CExtInfo(%d) Error Rlt(%d).", olt_id, iRlt);
        }   
    }

    return iRlt;
}

int OLT_SetCardI2CInfo(short int olt_id, board_sysinfo_t *board_info)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(board_info);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetCardI2CInfo, (olt_id, board_info) )) )
    {
        int slot = GetCardIdxByPonChip(olt_id);
        
		if( '\0' != board_info->serialno[0] )
		{
			VOS_MemZero( g_sysinfo_module[slot].serialno, I2C_REG_MODULE_SN_LEN );
			VOS_StrnCpy( g_sysinfo_module[slot].serialno, board_info->serialno, I2C_REG_MODULE_SN_LEN );
		}
		if( '\0' != board_info->version[0] )
		{
			VOS_MemZero( g_sysinfo_module[slot].version, I2C_REG_MODULE_VER_LEN );
			VOS_StrnCpy( g_sysinfo_module[slot].version, board_info->version, I2C_REG_MODULE_VER_LEN );
		}
		if( '\0' != board_info->product_date[0] )
		{
			VOS_MemZero( g_sysinfo_module[slot].product_date, I2C_REG_MODULE_DATE_LEN );
			VOS_StrnCpy( g_sysinfo_module[slot].product_date, board_info->product_date, I2C_REG_MODULE_DATE_LEN );
		}
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetCardI2CInfo(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_GetCardI2CInfo(short int olt_id, board_sysinfo_t *board_info)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(board_info);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetCardI2CInfo, (olt_id, board_info) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTSUPPORT != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetCardI2CInfo(%d) Error Rlt(%d).", olt_id, iRlt);
        }
    }

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
int OLT_WriteMdioRegister(short int olt_id, short int phy_address, short int reg_address, unsigned short int value )
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, WriteMdioRegister, (olt_id, phy_address, reg_address, value) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTSUPPORT != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_WriteMdioRegister(%d, %d, %d) Error Rlt(%d).", olt_id, phy_address, reg_address, iRlt);
        }
    }

    return iRlt;
}

int OLT_ReadMdioRegister(short int olt_id, short int phy_address, short int reg_address, unsigned short int *value )
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(value);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, ReadMdioRegister, (olt_id, phy_address, reg_address, value) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTSUPPORT != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ReadMdioRegister(%d, %d, %d) Error Rlt(%d).", olt_id, phy_address, reg_address, iRlt);
        }
    }

    return iRlt;
}

int OLT_ReadI2CRegister(short int olt_id, short int device, short int register_address, short int *data )
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(data);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, ReadI2CRegister, (olt_id, device, register_address, data) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTSUPPORT != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ReadI2CRegister(%d, %d, %d) Error Rlt(%d).", olt_id, device, register_address, iRlt);
        }
    }

    return iRlt;
}

int OLT_GpioAccess(short int olt_id, short int line_number, PON_gpio_line_io_t set_direction, short int set_value, PON_gpio_line_io_t *direction, bool *value)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
   
	OLT_ASSERT(olt_id);

	if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GpioAccess, (olt_id, line_number, set_direction, set_value, direction, value) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTSUPPORT != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GpioAccess(%d, %d, %d) Error Rlt(%d).", olt_id, line_number, set_direction, iRlt);
        }
    }

	return iRlt;
}

int OLT_ReadGpio(short int olt_id, int func_id, bool *value)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
   
	OLT_ASSERT(olt_id);
    VOS_ASSERT(value);

	if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, ReadGpio, (olt_id, func_id, value) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTSUPPORT != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ReadGpio(%d, %d, %d) Error Rlt(%d).", olt_id, func_id, *value, iRlt);
        }
    }

	return iRlt;
}

int OLT_WriteGpio(short int olt_id, int func_id, bool value)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
   
	OLT_ASSERT(olt_id);

	if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, WriteGpio, (olt_id, func_id, value) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTSUPPORT != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_WriteGpio(%d, %d, %d) Error Rlt(%d).", olt_id, func_id, value, iRlt);
        }
    }

	return iRlt;
}

/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

int OLT_SendChipCli(short int olt_id, unsigned short size, unsigned char *command)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
   
	OLT_ASSERT(olt_id);
    VOS_ASSERT(size > 0);
    VOS_ASSERT(command);

	if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SendChipCli, (olt_id, size, command) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTSUPPORT != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SendChipCli(%d, %d, %s) Error Rlt(%d).", olt_id, size, command, iRlt);
        }
    }

	return iRlt;
}

int OLT_SetDeviceName(short int olt_id, char* pValBuf, int valLen)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetDeviceName, (olt_id, pValBuf, valLen) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetDeviceName, (olt_id, pValBuf, valLen) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡上的PAS_SOFT系统参数 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetDeviceName, (olt_id, pValBuf, valLen) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        SetOltDeviceName(pValBuf, valLen);
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetDeviceName(%d, %s, %d) Error Rlt(%d).", olt_id, pValBuf, valLen, iRlt);
    }

    return iRlt;
}

#endif


#if 1
/* -------------------OLT PON管理API------------------- */

int OLT_GetVersion(short int olt_id, PON_device_versions_t *device_versions)
{
    OLT_ASSERT(olt_id);
    VOS_ASSERT(device_versions);
    return OLT_API_CALL2( olt_id, GetVersion, (olt_id, device_versions) ); 
}

int OLT_GetDBAVersion(short int olt_id, OLT_DBA_version_t *dba_version)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(dba_version);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetDBAVersion, (olt_id, dba_version) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetDBAVersion(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_ChkVersion(short int olt_id, bool *is_compatibled)
{
    int iRlt;
	
    OLT_ASSERT(olt_id);
    VOS_ASSERT(is_compatibled);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, ChkVersion, (olt_id, is_compatibled) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ChkVersion(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_ChkDBAVersion(short int olt_id, bool *is_compatibled)
{
    int iRlt;
	
    OLT_ASSERT(olt_id);
    VOS_ASSERT(is_compatibled);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, ChkDBAVersion, (olt_id, is_compatibled) )) )
    {
    	
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ChkDBAVersion(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetCniLinkStatus(short int olt_id, bool *status)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(status);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetCniLinkStatus, (olt_id, status) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetCniLinkStatus(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetPonWorkStatus(short int olt_id, int *work_status)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(work_status);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetPonWorkStatus, (olt_id, work_status) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTEXIST != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetPonWorkStatus(%d) Error Rlt(%d).", olt_id, iRlt);
        }
    }

    return iRlt;
}

int OLT_SetAdminStatus(short int olt_id, int admin_status)
{
    int iRlt;
	short int partner_olt_id = 0;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetAdminStatus, (olt_id, admin_status) )) )
    {
        PonPortTable[olt_id].PortAdminStatus = admin_status;
        if ( V2R1_ENABLE == admin_status )
    	{
    	    PonPortTable[olt_id].TxCtrlFlag &= ~PONPORT_TX_SHUTDOWN;
    	}
		else
		{
    	    PonPortTable[olt_id].TxCtrlFlag |= PONPORT_TX_SHUTDOWN;
			/*added by wangjiah@2017-06-22 :begin
			 * To make sure that swapping happens as soon as active pon port shutdown*/
			if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER
					&& OLT_ERR_OK == PonPortSwapPortQuery(olt_id, &partner_olt_id)
					&& V2R1_PON_PORT_SWAP_ACTIVE == PonPortHotStatusQuery(olt_id)
					&& V2R1_PON_PORT_SWAP_SLOWLY == GetPonPortHotSwapMode(olt_id))
			{
				ProcessHotSwap(olt_id, partner_olt_id, PROTECT_SWITCH_REASON_OLTREMOVE);
			}
			/*added by wangjiah@2017-06-22 :end*/
		}
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAdminStatus(%d, %d) Error Rlt(%d).", olt_id, admin_status, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_GetAdminStatus(short int olt_id, int *admin_status)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(admin_status);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetAdminStatus, (olt_id, admin_status) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTEXIST != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetAdminStatus(%d) Error Rlt(%d).", olt_id, iRlt);
        }
    }

    return iRlt;
}

int OLT_SetVlanTpid(short int olt_id, unsigned short int vlan_tpid)
{
    int iRlt;
	short int olt_id_swap = 0;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetVlanTpid, (olt_id, vlan_tpid) )) )
    {
        PonPortTable[olt_id].vlan_tpid = vlan_tpid;

		OLT_PARTNER_CALL_BEGIN		
		iRlt = OLT_SetVlanTpid(olt_id_swap, vlan_tpid);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "Partner Call OLT_SetVlanTpid(%d, %d) Error Rlt(%d).", olt_id_swap, vlan_tpid, iRlt);
			iRlt = OLT_ERR_OK;
		}
		OLT_PARTNER_CALL_END
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetVlanTpid(%d, %d) Error Rlt(%d).", olt_id, vlan_tpid, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_SetVlanQinQ(short int olt_id, OLT_vlan_qinq_t *vlan_qinq_config)
{
    int iRlt;
	short int olt_id_swap = 0;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(vlan_qinq_config);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetVlanQinQ, (olt_id, vlan_qinq_config) )) )
	{

#ifdef BCM_DRV_646
		if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
		{
			iRlt = GWGPON_SetVlanQinQ(olt_id, vlan_qinq_config);
		}
#endif

		if (OLT_CFG_DIR_UPLINK == vlan_qinq_config->qinq_direction)
		{
			PON_olt_vlan_uplink_config_t *pQinQ = &(vlan_qinq_config->qinq_cfg.up_cfg);

			iRlt = SavePonUplinkVlanManipulation(olt_id, vlan_qinq_config->qinq_objectid, pQinQ);
		}
		else
		{
			PON_olt_vid_downlink_config_t *pQinQ = &(vlan_qinq_config->qinq_cfg.down_cfg);

			if (PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION != pQinQ->vlan_manipulation)
			{
				iRlt = SavePonDownlinkVlanManipulation(olt_id, vlan_qinq_config->qinq_objectid, pQinQ);
			}
			else
			{
				iRlt = DeletePonDownlinkVlanManipulation(olt_id, vlan_qinq_config->qinq_objectid);
			}
		}

		if (0 != iRlt)
		{
			iRlt = OLT_ERR_NORESC;
		}

		OLT_PARTNER_CALL_BEGIN
		iRlt = OLT_SetVlanQinQ(olt_id_swap, vlan_qinq_config);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "Partne Call OLT_SetVlanQinQ(%d) Error Rlt(%d).", olt_id_swap, iRlt);
			iRlt = OLT_ERR_OK;
		}
		OLT_PARTNER_CALL_END
	}
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetVlanQinQ(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }


    return iRlt;
}

int OLT_SetPonFrameSizeLimit(short int olt_id, short int jumbo_length)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetPonFrameSizeLimit, (olt_id, jumbo_length) )) )
    {
        PON_Jumbo_frame_length = jumbo_length;
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetPonFrameSizeLimit(%d, %d) Error Rlt(%d).", olt_id, jumbo_length, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_GetPonFrameSizeLimit(short int olt_id, short int *jumbo_length)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetPonFrameSizeLimit, (olt_id, jumbo_length) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetPonFrameSizeLimit(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}
    
int OLT_OamIsLimit(short int olt_id, bool *oam_limit)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(oam_limit);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, OamIsLimit, (olt_id, oam_limit) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_OamIsLimit(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_UpdatePonParams(short int olt_id, int max_range, int mac_agetime)
{
    int iRlt;
	short int olt_id_swap = 0;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, UpdatePonParams, (olt_id, max_range, mac_agetime) )) )
    {
		OLT_PARTNER_CALL_BEGIN
		iRlt = OLT_UpdatePonParams(olt_id_swap, max_range, mac_agetime);
		if(OLT_CALL_ISERROR(iRlt))
		{
			VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "Partner Call OLT_UpdatePonParams(%d, %d, %d) Error Rlt(%d).", olt_id_swap, max_range, mac_agetime, iRlt);
			iRlt = OLT_ERR_OK;
		}
		OLT_PARTNER_CALL_END
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_UpdatePonParams(%d, %d, %d) Error Rlt(%d).", olt_id, max_range, mac_agetime, iRlt);
    }

    return iRlt;
}

int OLT_SetPPPoERelayMode(short int olt_id, int set_mode, int relay_mode)
{
    int iRlt;

#if 0
    if ( (g_PPPOE_relay == set_mode)
        && ((0 == relay_mode) || (relay_mode == g_PPPOE_relay_mode)) )    
    {
        return 0;
    }
#endif

    if (relay_mode > 0)
    	g_PPPOE_relay_mode = relay_mode;

    g_PPPOE_relay = set_mode;
    
    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetPPPoERelayMode, (olt_id, set_mode, relay_mode) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetPPPoERelayMode, (olt_id, set_mode, relay_mode) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetPPPoERelayMode, (olt_id, set_mode, relay_mode) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
#if 0
        if (relay_mode > 0)
        	g_PPPOE_relay_mode = relay_mode;

        g_PPPOE_relay = set_mode;
#endif
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetPPPoERelayMode(%d, %d, %d) Error Rlt(%d).", olt_id, set_mode, relay_mode, iRlt);
    }

    return iRlt;
}

int OLT_SetPPPoERelayParams(short int olt_id, short int param_id, int param_value1, int param_value2)
{
    int iRlt;

    switch (param_id)
    {
        case PPPOE_RELAY_PARAMID_STRHEAD:
            if (0 != param_value1)
            {
                VOS_MemCpy(PPPOE_Relay_Maual_String_head, (VOID*)param_value1, param_value2+1);
                PPPOE_Relay_Maual_String_Len = param_value2;
            }
            else
            {
                *PPPOE_Relay_Maual_String_head = '\0';
                PPPOE_Relay_Maual_String_Len = 0;
            }
            break;
        default:
            VOS_ASSERT(0);
            return OLT_ERR_PARAM;
    }

    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetPPPoERelayParams, (olt_id, param_id, param_value1, param_value2) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetPPPoERelayParams, (olt_id, param_id, param_value1, param_value2) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetPPPoERelayParams, (olt_id, param_id, param_value1, param_value2) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
#if 0    
        switch (param_id)
        {
            case PPPOE_RELAY_PARAMID_STRHEAD:
                if (0 != param_value1)
                {
                    VOS_MemCpy(PPPOE_Relay_Maual_String_head, (VOID*)param_value1, param_value2+1);
                    PPPOE_Relay_Maual_String_Len = param_value2;
                }
                else
                {
                    *PPPOE_Relay_Maual_String_head = '\0';
                    PPPOE_Relay_Maual_String_Len = 0;
                }
                break;
            default:
                VOS_ASSERT(0);
                return OLT_ERR_PARAM;
        }
#endif        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetPPPoERelayParams(%d, %d, %d, %d) Error Rlt(%d).", olt_id, param_id, param_value1, param_value2, iRlt);
    }

    return iRlt;
}

int OLT_SetDhcpRelayMode(short int olt_id, int set_mode, int relay_mode)
{
    int iRlt;

#if 0
    if ( ((0 > set_mode) || (set_mode == g_DHCP_relay))
        && ((0 == relay_mode) || (relay_mode == g_DHCP_relay_mode)) )    
    {
        return 0;
    }
#endif

    if (relay_mode > 0)
    	g_DHCP_relay_mode = relay_mode;

    if (set_mode >= 0)
        g_DHCP_relay = set_mode;

    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetDhcpRelayMode, (olt_id, set_mode, relay_mode) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetDhcpRelayMode, (olt_id, set_mode, relay_mode) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetDhcpRelayMode, (olt_id, set_mode, relay_mode) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
#if 0
        if (relay_mode > 0)
        	g_DHCP_relay_mode = relay_mode;

        if (set_mode >= 0)
            g_DHCP_relay = set_mode;
#endif
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetDhcpRelayMode(%d, %d, %d) Error Rlt(%d).", olt_id, set_mode, relay_mode, iRlt);
    }

    return iRlt;
}

int OLT_SetIgmpAuthMode(short int olt_id, int auth_mode)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetIgmpAuthMode, (olt_id, auth_mode) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetIgmpAuthMode, (olt_id, auth_mode) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡上的PAS_SOFT系统参数 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += OLT_ID_SPAN(olt_id) )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetIgmpAuthMode, (olt_id, auth_mode) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        if ( auth_mode > 0 )
        {
            g_ulIgmp_Auth_Enabled = 2 - auth_mode;
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetIgmpAuthMode(%d, %d) Error Rlt(%d).", olt_id, auth_mode, iRlt);
    }

    return iRlt;
}
    
int OLT_SendFrame2PON(short int olt_id, short int llid, void *eth_frame, int frame_len)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(eth_frame);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SendFrame2PON, (olt_id, llid, eth_frame, frame_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SendFrame2PON(%d, %d, %d) Error Rlt(%d).", olt_id, llid, frame_len, iRlt);
    }

    return iRlt;
}
    
int OLT_SendFrame2CNI(short int olt_id, short int llid, void *eth_frame, int frame_len)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(eth_frame);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SendFrame2CNI, (olt_id, llid, eth_frame, frame_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SendFrame2CNI(%d, %d, %d) Error Rlt(%d).", olt_id, llid, frame_len, iRlt);
    }

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
int OLT_GetVidDownlinkMode(short int olt_id, PON_vlan_tag_t vid, PON_olt_vid_downlink_config_t *vid_downlink_config)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, GetVidDownlinkMode, (olt_id, vid, vid_downlink_config) )) )
    {
        
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetVidDownlinkMode(%d, %d) Error Rlt(%d).", olt_id, vid, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_DelVidDownlinkMode(short int olt_id, PON_vlan_tag_t vid)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, DelVidDownlinkMode, (olt_id, vid) )) )
    {
        
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_DelVidDownlinkMode(%d, %d) Error Rlt(%d).", olt_id, vid, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_GetOltParameters(short int olt_id, PON_olt_response_parameters_t *olt_parameters)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, GetOltParameters, (olt_id, olt_parameters) )) )
    {
        
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOltParameters(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_SetOltIgmpSnoopingMode(short int olt_id, PON_olt_igmp_configuration_t *igmp_configuration)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetOltIgmpSnoopingMode, (olt_id, igmp_configuration) )) )
    {
        
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOltIgmpSnoopingMode(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_GetOltIgmpSnoopingMode(short int olt_id, PON_olt_igmp_configuration_t *igmp_configuration)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, GetOltIgmpSnoopingMode, (olt_id, igmp_configuration) )) )
    {
        
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOltIgmpSnoopingMode(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_SetOltMldForwardingMode(short int olt_id, disable_enable_t mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetOltMldForwardingMode, (olt_id, mode) )) )
    {
        
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOltMldForwardingMode(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        
    }

    return iRlt;
}

int OLT_GetOltMldForwardingMode(short int olt_id, disable_enable_t *mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, GetOltMldForwardingMode, (olt_id, mode) )) )
    {
        
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOltMldForwardingMode(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        
    }

    return iRlt;
}

int OLT_SetDBAReportFormat(short int olt_id, PON_DBA_report_format_t report_format)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetDBAReportFormat, (olt_id, report_format) )) )
    {
        
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetDBAReportFormat(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        
    }

    return iRlt;
}

int OLT_GetDBAReportFormat(short int olt_id, PON_DBA_report_format_t *report_format)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, GetDBAReportFormat, (olt_id, report_format) )) )
    {
        
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetDBAReportFormat(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        
    }

    return iRlt;
}

/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
/*Begin:for onu swap by jinhl@2013-04-27*/
int OLT_UpdateProvBWInfo( short int olt_id )
{

    int iRlt;
	
    OLT_ASSERT(olt_id);
	
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, UpdateProvBWInfo, (olt_id) )) )
    {
        
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_UpdateProvisionedBWInfo(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        
    }

    return iRlt;
	
}
/*End:for onu swap by jinhl@2013-04-27*/
#endif


#if 1
/* -------------------OLT LLID管理API------------------- */

int OLT_LLIDIsExist(short int olt_id, short int llid, bool *status)
{
    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(status);
    
    return OLT_API_CALL2( olt_id, LLIDIsExist, (olt_id, llid, status) ); 
}

int OLT_DeregisterLLID(short int olt_id, short int llid, bool iswait)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, DeregisterLLID, (olt_id, llid, iswait) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_DeregisterLLID(%d, %d) Error Rlt(%d).", olt_id, llid, iswait, iRlt);
    }
    
    return iRlt;    
}

int OLT_GetLLIDMac(short int olt_id, short int llid, mac_address_t onu_mac)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_mac);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetLLIDMac, (olt_id, llid, onu_mac) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetLLIDMac(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_GetLLIDRegisterInfo(short int olt_id, short int llid, onu_registration_info_t *onu_info)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_info);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetLLIDRegisterInfo, (olt_id, llid, onu_info) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetLLIDRegisterInfo(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_AuthorizeLLID(short int olt_id, short int llid, bool auth_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
   
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, AuthorizeLLID, (olt_id, llid, auth_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_AuthorizeLLID(%d, %d, %d) Error Rlt(%d).", olt_id, llid, auth_mode, iRlt);
    }

    return iRlt;
}


int OLT_SetLLIDSLA(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(SLA);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetLLIDSLA, (olt_id, llid, SLA) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetLLIDSLA(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_GetLLIDSLA(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(SLA);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetLLIDSLA, (olt_id, llid, SLA) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetLLIDSLA(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_SetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(police);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetLLIDPolice, (olt_id, llid, police) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetLLIDPolice(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_GetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(police);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetLLIDPolice, (olt_id, llid, police) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetLLIDPolice(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_SetLLIDdbaType(short int olt_id, short int llid, int dba_type, short int *dba_error)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(dba_error);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetLLIDdbaType, (olt_id, llid, dba_type, dba_error) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetLLIDdbaType(%d, %d, %d) Error Rlt(%d).", olt_id, llid, dba_type, iRlt);
    }
    
    return iRlt;    
}

int OLT_GetLLIDdbaType(short int olt_id, short int llid, int *dba_type, short int *dba_error)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(dba_type);
    VOS_ASSERT(dba_error);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetLLIDdbaType, (olt_id, llid, dba_type, dba_error) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetLLIDdbaType(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_SetLLIDdbaFlags(short int olt_id, short int llid, unsigned short dba_flags)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetLLIDdbaFlags, (olt_id, llid, dba_flags) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetLLIDdbaFlags(%d, %d, %d) Error Rlt(%d)", olt_id, llid, dba_flags, iRlt);
    }
    
    return iRlt;    
}

int OLT_GetLLIDdbaFlags(short int olt_id, short int llid, unsigned short *dba_flags)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(dba_flags);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetLLIDdbaFlags, (olt_id, llid, dba_flags) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetLLIDdbaFlags(%d, %d) Error Rlt(%d)", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}


int OLT_GetLLIDHeartbeatOam(short int olt_id, short int llid, unsigned short *send_period, unsigned short *send_size, unsigned char *send_data, unsigned short *recv_timeout, unsigned short *recv_size, unsigned char *recv_data, bool *recv_IgnoreTrailingBytes)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(send_period);
    VOS_ASSERT(send_size);
    VOS_ASSERT(send_data);
    VOS_ASSERT(recv_timeout);
    VOS_ASSERT(recv_size);
    VOS_ASSERT(recv_data);
    VOS_ASSERT(recv_IgnoreTrailingBytes);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetLLIDHeartbeatOam, (olt_id, llid, send_period, send_size, send_data, recv_timeout, recv_size, recv_data, recv_IgnoreTrailingBytes) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetLLIDHeartbeatOam(%d, %d) Error Rlt(%d)", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_SetLLIDHeartbeatOam(short int olt_id, short int llid, unsigned short send_period, unsigned short send_size, unsigned char *send_data, unsigned short recv_timeout, unsigned short recv_size, unsigned char *recv_data, bool recv_IgnoreTrailingBytes)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(send_data);
    VOS_ASSERT(recv_data);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetLLIDHeartbeatOam, (olt_id, llid, send_period, send_size, send_data, recv_timeout, recv_size, recv_data, recv_IgnoreTrailingBytes) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetLLIDHeartbeatOam(%d, %d) Error Rlt(%d)", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

#endif


#if 1
/* -------------------OLT ONU 管理API------------------- */

int OLT_GetOnuNum(short int olt_id, int onu_flags, int *onu_number)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(onu_number);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetOnuNum, (olt_id, onu_flags, onu_number) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOnuNum(%d, %d) Error Rlt(%d).", olt_id, onu_flags, iRlt);
    }

    return iRlt;
}

int OLT_GetAllOnus(short int olt_id, OLT_onu_table_t *onu_table)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(onu_table);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetAllOnus, (olt_id, onu_table) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetAllOnus(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_ClearAllOnus(short int olt_id)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, ClearAllOnus, (olt_id) )) )
    {
        ClearAllOnuDataByOnePon(olt_id);
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ClearAllOnus(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_ResumeAllOnuStatus(short int olt_id, int resume_reason, int resume_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, ResumeAllOnuStatus, (olt_id, resume_reason, resume_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ResumeAllOnuStatus(%d, %d, %d) Error Rlt(%d).", olt_id, resume_reason, resume_mode, iRlt);
    }

    return iRlt;
}

int OLT_SetAllOnuAuthMode(short int olt_id, int auth_mode)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetAllOnuAuthMode, (olt_id, auth_mode) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetAllOnuAuthMode, (olt_id, auth_mode) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetAllOnuAuthMode, (olt_id, auth_mode) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        g_onuAuthEnable = auth_mode;
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAllOnuAuthMode(%d, %d) Error Rlt(%d).", olt_id, auth_mode, iRlt);
    }

    return iRlt;
}

int OLT_SetOnuAuthMode(short int olt_id, int auth_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetOnuAuthMode, (olt_id, auth_mode) )) )
    {
        PonPortTable[olt_id].OnuAuthMode = auth_mode;     
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOnuAuthMode(%d, %d) Error Rlt(%d).", olt_id, auth_mode, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_SetMacAuth(short int olt_id, int mode, mac_address_t mac)
{
    int iRlt;
	short int olt_id_swap = 0;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(mac);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetMacAuth, (olt_id, mode, mac) )) )
    {
		OLT_PARTNER_CALL_BEGIN
		iRlt = OLT_SetMacAuth(olt_id_swap, mode, mac); 
		if(OLT_CALL_ISERROR(iRlt))
		{
			VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "Partner Call OLT_SetMacAuth(%d, %d) Error Rlt(%d).", olt_id_swap, mode, iRlt);
		}
		OLT_PARTNER_CALL_END
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetMacAuth(%d, %d) Error Rlt(%d).", olt_id, mode, iRlt);
    }

    return iRlt;
}

int OLT_SetAllOnuBindMode(short int olt_id, int bind_mode)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetAllOnuBindMode, (olt_id, bind_mode) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetAllOnuBindMode, (olt_id, bind_mode) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetAllOnuBindMode, (olt_id, bind_mode) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        MAKEING_TEST_FLAG = bind_mode;
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAllOnuBindMode(%d, %d) Error Rlt(%d).", olt_id, bind_mode, iRlt);
    }

    return iRlt;
}

/* added by xieshl 20110201 */
int OLT_ChkOnuRegisterControl(short int olt_id, short int llid, mac_address_t mac_address, short int *bind_olt_id)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(mac_address);
    VOS_ASSERT(bind_olt_id);

    if( OLT_ERR_OK == (iRlt = GW_ChkOnuRegisterControl(olt_id, llid, mac_address, bind_olt_id)) )
    {
        if ( OLT_ID_INVALID == *bind_olt_id )
        {
            /* 本地绑定检查通过，仍需到上层检查绑定 */
            if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
            {
                short int remote_olt_id = OLT_DEVICE_ID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id));

                iRlt = OLT_API_CALL2( remote_olt_id, ChkOnuRegisterControl, (remote_olt_id, llid, mac_address, bind_olt_id) );
            }
        }   
    }

    if ( OLT_ERR_OK != iRlt )
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ChkOnuRegisterControl(%d, %d, [%02x-%02x-%02x-%02x-%02x-%02x]) Error Rlt(%d).", olt_id, llid, mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5], iRlt);
    }
    
    return iRlt;		
}

int OLT_SetAllOnuDefaultBW(short int olt_id, ONU_bw_t *default_bw)
{
    int iRlt;
        
    VOS_ASSERT(default_bw);

    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetAllOnuDefaultBW, (olt_id, default_bw) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetAllOnuDefaultBW, (olt_id, default_bw) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡上的PAS_SOFT系统参数 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetAllOnuDefaultBW, (olt_id, default_bw) )) )
                {
                    break;
                }
            }
        }
    }
        
    if ( OLT_ERR_OK == iRlt )
    {
        if ( OLT_CFG_DIR_UPLINK & default_bw->bw_direction )
        {
			if(PON_RATE_10_10G == default_bw->bw_rate)
			{
				OnuConfigDefault.UplinkBandwidth_XGEPON_SYM   = default_bw->bw_gr;
				OnuConfigDefault.UplinkBandwidthBe_XGEPON_SYM = default_bw->bw_be;
			}
			else if(PON_RATE_1_10G == default_bw->bw_rate)
			{
				OnuConfigDefault.UplinkBandwidth_XGEPON_ASYM = default_bw->bw_gr;
				OnuConfigDefault.UplinkBandwidthBe_XGEPON_ASYM = default_bw->bw_be;
			}
			else if(PON_RATE_NORMAL_1G == default_bw->bw_rate)
			{
				OnuConfigDefault.UplinkBandwidth   = default_bw->bw_gr;
				OnuConfigDefault.UplinkBandwidthBe = default_bw->bw_be;
			}
			else if(PON_RATE_1_2G == default_bw->bw_rate)
			{
				OnuConfigDefault.UplinkBandwidth_GPON   = default_bw->bw_gr;
				OnuConfigDefault.UplinkBandwidthBe_GPON = default_bw->bw_be;
			}

            if ( OLT_CFG_DIR_DOWNLINK & default_bw->bw_direction )
            {
				if(PON_RATE_10_10G == default_bw->bw_rate)
				{
					OnuConfigDefault.DownlinkBandwidth_XGEPON_SYM   = default_bw->bw_fixed;
					OnuConfigDefault.DownlinkBandwidthBe_XGEPON_SYM = default_bw->bw_actived;
				}
				else if(PON_RATE_1_10G == default_bw->bw_rate)
				{
					OnuConfigDefault.DownlinkBandwidth_XGEPON_ASYM   = default_bw->bw_fixed;
					OnuConfigDefault.DownlinkBandwidthBe_XGEPON_ASYM = default_bw->bw_actived;
				}
				else if(PON_RATE_NORMAL_1G == default_bw->bw_rate)
				{
					OnuConfigDefault.DownlinkBandwidth = default_bw->bw_fixed;
					OnuConfigDefault.DownlinkBandwidthBe = default_bw->bw_actived;
				}
				else if(PON_RATE_1_2G == default_bw->bw_rate)
				{
					OnuConfigDefault.DownlinkBandwidth_GPON   = default_bw->bw_fixed;
					OnuConfigDefault.DownlinkBandwidthBe_GPON = default_bw->bw_actived;
				}
            }
        }
        else if ( OLT_CFG_DIR_DOWNLINK & default_bw->bw_direction )
		{
			if(PON_RATE_10_10G == default_bw->bw_rate)
			{
				OnuConfigDefault.DownlinkBandwidth_XGEPON_SYM   = default_bw->bw_gr;
				OnuConfigDefault.DownlinkBandwidthBe_XGEPON_SYM = default_bw->bw_be;
			}
			else if(PON_RATE_1_10G == default_bw->bw_rate)
			{
				OnuConfigDefault.DownlinkBandwidth_XGEPON_ASYM   = default_bw->bw_gr;
				OnuConfigDefault.DownlinkBandwidthBe_XGEPON_ASYM = default_bw->bw_be;
			}
			else if(PON_RATE_NORMAL_1G == default_bw->bw_rate)
			{
				OnuConfigDefault.DownlinkBandwidth = default_bw->bw_gr;
				OnuConfigDefault.DownlinkBandwidthBe = default_bw->bw_be;
			}
			else if(PON_RATE_1_2G == default_bw->bw_rate)
			{
				OnuConfigDefault.DownlinkBandwidth_GPON   = default_bw->bw_gr;
				OnuConfigDefault.DownlinkBandwidthBe_GPON  = default_bw->bw_be;
			}
		}
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAllOnuDefaultBW(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_SetAllOnuDownlinkPoliceMode(short int olt_id, int police_mode)
{
    int iRlt;
    int iIsNeedSyncPon;

    if ( 0 <= police_mode )
    {
        iIsNeedSyncPon = 1;
    }
    else
    {
        iIsNeedSyncPon = 0;
        police_mode = -police_mode;
    }

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetAllOnuDownlinkPoliceMode, (olt_id, police_mode) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetAllOnuDownlinkPoliceMode, (olt_id, police_mode) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetAllOnuDownlinkPoliceMode, (olt_id, police_mode) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        downlinkBWlimit = police_mode;
        if ( iIsNeedSyncPon )
        {
        	for( olt_id = 0; olt_id < MAXPON; olt_id ++ )
        	{
                PonPortTable[olt_id].DownlinkPoliceMode = police_mode;     
        	}
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAllOnuDownlinkPoliceMode(%d, %d) Error Rlt(%d).", olt_id, police_mode, iRlt);
    }

    return iRlt;
}

int OLT_SetOnuDownlinkPoliceMode(short int olt_id, int police_mode)
{
    int iRlt;
	short int olt_id_swap = 0;
        
    OLT_ASSERT(olt_id);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetOnuDownlinkPoliceMode, (olt_id, police_mode) )) )
    {
        PonPortTable[olt_id].DownlinkPoliceMode = police_mode;     

		OLT_PARTNER_CALL_BEGIN
		iRlt = OLT_SetOnuDownlinkPoliceMode(olt_id_swap, police_mode);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "Partner Call OLT_SetOnuDownlinkPoliceMode(%d, %d) Error Rlt(%d).", olt_id_swap, police_mode, iRlt);
			iRlt = OLT_ERR_OK;
		}
		OLT_PARTNER_CALL_END
		
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOnuDownlinkPoliceMode(%d, %d) Error Rlt(%d).", olt_id, police_mode, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_SetAllOnuDownlinkPoliceParam(short int olt_id, int BwBurstSize, int BwPreference, int BwWeightSize)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetAllOnuDownlinkPoliceParam, (olt_id, BwBurstSize, BwPreference, BwWeightSize) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetAllOnuDownlinkPoliceParam, (olt_id, BwBurstSize, BwPreference, BwWeightSize) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetAllOnuDownlinkPoliceParam, (olt_id, BwBurstSize, BwPreference, BwWeightSize) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        if (BwBurstSize < 0)
        {
            downlinkBWlimitBurstSize = ONU_DOWNLINK_POLICE_BURSESIZE_DEFAULT;
        }
        else
        {
            downlinkBWlimitBurstSize = BwBurstSize;
        }

        if (BwPreference < 0)
        {
            downlinkBWlimitPreference = DISABLE;
        }
        else
        {
            downlinkBWlimitPreference = BwPreference;
        }

        if (BwWeightSize < 0)
        {
            downlinkBWWeight = ONU_DOWNLINK_POLICE_WEIGHT_DEFAULT;
        }
        else
        {
            downlinkBWWeight = BwWeightSize;
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAllOnuDownlinkPoliceParam(%d ,%d ,%d) Error Rlt(%d).", olt_id, BwBurstSize, BwPreference, BwWeightSize, iRlt);
    }

    return iRlt;
}

int OLT_SetAllOnuUplinkDBAParam(short int olt_id, int BwFixedPktSize, int BwBurstSize, int BwWeightSize)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetAllOnuUplinkDBAParam, (olt_id, BwFixedPktSize, BwBurstSize, BwWeightSize) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetAllOnuUplinkDBAParam, (olt_id, BwFixedPktSize, BwBurstSize, BwWeightSize) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetAllOnuUplinkDBAParam, (olt_id, BwFixedPktSize, BwBurstSize, BwWeightSize) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        if ( 0 > BwFixedPktSize )
        {
            uplinkBWPacketUnitSize = ONU_UPLINK_DBA_PACKETSIZE_DEFAULT;
        }
        else
        {
            uplinkBWPacketUnitSize = BwFixedPktSize;
        }

        if ( 0 > BwBurstSize )
        {
            uplinkBWlimitBurstSize = ONU_UPLINK_POLICE_BURSESIZE_DEFAULT;
        }
        else
        {
            uplinkBWlimitBurstSize = BwBurstSize;
        }

        if ( 0 > BwWeightSize )
        {
            uplinkBWWeight = ONU_UPLINK_DBA_WEIGHT_DEFAULT;
        }
        else
        {
            uplinkBWWeight = BwWeightSize;
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAllOnuUplinkDBAParam(%d, %d, %d, %d) Error Rlt(%d).", olt_id, BwFixedPktSize, BwBurstSize, BwWeightSize, iRlt);
    }

    return iRlt;
}

int OLT_SetOnuDownlinkPri2CoSQueueMap(short int olt_id, OLT_pri2cosqueue_map_t *map)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(map);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetOnuDownlinkPri2CoSQueueMap, (olt_id, map) )) )
    {
        VOS_MemCpy(PON_priority_queue_mapping, map->priority, sizeof(map->priority));
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOnuDownlinkPri2CoSQueueMap(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

/* add onu to pon port *//*add by shixh20100908,for onu register by manual*/
int OLT_ActivePendingOnu(short int olt_id)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);

    if( EVENT_REGISTER == V2R1_ENABLE )
           sys_console_printf(" pon %d/%d activate all pending onu\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id) );
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, ActivePendingOnu, (olt_id) )) )
    {
#if 1	/* modified by xieshl 20110525, 统一删除调用底层删除API */
        UpdateAllPendingOnu( olt_id );
#else
        pendingOnu_S *CurOnu, *NextOnu;

        VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
    	NextOnu = PonPortTable[olt_id].PendingOnu.Next;
        while( NULL != (CurOnu = NextOnu) )
        {
            NextOnu = CurOnu->Next;
            VOS_Free( (void *)CurOnu );
        }

        PonPortTable[olt_id].PendingOnu.Next = NULL;
        VOS_SemGive(OnuPendingDataSemId);
#endif
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
        	if ( OLT_ERR_NOTEXIST != iRlt )
        	{
                VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ActivePendingOnu(%d) Error Rlt(%d).", olt_id, iRlt);
            }
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

/* activate onu pending onu *//*add by shixh20100913*/
int OLT_ActiveOnePendingOnu(short int olt_id, unsigned char *mac)
{
    int iRlt = OLT_ERR_NOTEXIST;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(mac);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, ActiveOnePendingOnu, (olt_id, mac) )) )
    {
#if 1	/* modified by xieshl 20110525, 统一删除调用底层删除API */
	 UpdatePendingOnu( olt_id, INVALID_LLID, mac );
#else
        pendingOnu_S *CurOnu, *NextOnu, *PreOnu;

        VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
        PreOnu = NULL;
        CurOnu = PonPortTable[olt_id].PendingOnu.Next;
        while( CurOnu != NULL )
        {		
            NextOnu = CurOnu->Next;

            if( MAC_ADDR_IS_EQUAL(CurOnu->MacAddr, mac) )
            {
        		if( EVENT_REGISTER == V2R1_ENABLE )
        			sys_console_printf(" pon %d/%d activate pending onu %02x%02x.%02x%02x.%02x%02x\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id),
        				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );

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
            CurOnu = NextOnu;
        }	
        VOS_SemGive(OnuPendingDataSemId);
#endif
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
        	if ( OLT_ERR_NOTEXIST != iRlt )
        	{
                VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ActiveOnePendingOnu(%d) Error Rlt(%d).", olt_id, iRlt);
            }
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

/* activate conflict pending onu */
int OLT_ActiveConfPendingOnu(short int olt_id, short int conf_olt_id)
{
    int iRlt;
        
    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, ActiveConfPendingOnu, (olt_id, conf_olt_id) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( ActiveConfPendingOnu, (olt_id, conf_olt_id) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            iRlt = OLT_ERR_NOTEXIST;
            for ( olt_id = 0; olt_id < MAXPON; olt_id += OLT_ID_SPAN(olt_id) )
            {
                if ( OLT_ERR_OK == OLT_API_CALL( olt_id, ActiveConfPendingOnu, (olt_id, conf_olt_id) ) )
                {
                    iRlt = OLT_ERR_OK;
                }
            }
        }
    }
    
    if ( OLT_ERR_OK == iRlt )
    {
        short int /*PonIdx,*/ PonGlobalIdx, Curport;
        /*pendingOnu_S *CurOnu, *NextOnu, *PreOnu;*/

        PonGlobalIdx = GetGlobalPonPortIdx(0);
        for( Curport = 0; Curport < MAXPON; Curport++, PonGlobalIdx++ )
        {
            if ( PonGlobalIdx == conf_olt_id ) continue;
            if ( PonPortIsWorking(Curport) != TRUE ) continue;

#if 1	/* modified by xieshl 20110525, 统一删除调用底层删除API */
            UpdateAllPendingConfOnu( Curport );
#else
            VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
            PreOnu = NULL;
            CurOnu = PonPortTable[Curport].PendingOnu_Conf.Next;
            while( CurOnu != NULL )
            {		
                NextOnu = CurOnu->Next;

                PonIdx = CurOnu->otherPonIdx;				
                if( PonIdx == conf_olt_id )
                {		
                    if( EVENT_REGISTER == V2R1_ENABLE)
                        sys_console_printf(" pon conflict onu %d llid %d is to be activated ,conf_olt_id=%d\r\n", PonIdx, CurOnu->Llid,conf_olt_id );

                    if( PreOnu == NULL )
                    {
                        PonPortTable[Curport].PendingOnu_Conf.Next = CurOnu->Next;				
                    }
                    else
                    {
                        PreOnu->Next = CurOnu->Next;
                    }
#ifdef __pending_onu_list__	
        		CurOnu->Llid = 0;
        		CurOnu->Next = NULL;
        		CurOnu->otherPonIdx = MAXPON;
#else
                     VOS_Free( (void *)CurOnu );
#endif
                }
                else
                {
                    PreOnu = CurOnu;
                }

                CurOnu = NextOnu;
            }
            VOS_SemGive(OnuPendingDataSemId);
#endif
        }
    }
    else
    {
    	if ( OLT_ERR_NOTEXIST != iRlt )
    	{
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ActiveConfPendingOnu(%d, %d) Error Rlt(%d).", olt_id, conf_olt_id, iRlt);
        }
    }

    return iRlt;
}

int OLT_ActiveOneConfPendingOnu(short int olt_id, short int conf_olt_id, unsigned char *mac)
{
    int iRlt = OLT_ERR_NOTEXIST;
        
    VOS_ASSERT(mac);
    
    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, ActiveOneConfPendingOnu, (olt_id, conf_olt_id, mac) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( ActiveOneConfPendingOnu, (olt_id, conf_olt_id, mac) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            iRlt = OLT_ERR_NOTEXIST;
            for ( olt_id = 0; olt_id < MAXPON; olt_id += OLT_ID_SPAN(olt_id) )
            {
                if ( OLT_ERR_OK == OLT_API_CALL( olt_id, ActiveOneConfPendingOnu, (olt_id, conf_olt_id, mac) ) )
                {
                    iRlt = OLT_ERR_OK;
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        short int /*PonIdx,*/ PonGlobalIdx, Curport;
        /*pendingOnu_S *CurOnu, *NextOnu, *PreOnu;*/

        PonGlobalIdx = GetGlobalPonPortIdx(0);
        for( Curport = 0; Curport < MAXPON; Curport++, PonGlobalIdx++ )
        {
            if ( PonGlobalIdx == conf_olt_id ) continue;
            if ( PonPortIsWorking(Curport) != TRUE ) continue;

#if 1	/* modified by xieshl 20110525, 统一删除调用底层删除API */
            UpdateAllPendingConfOnu( Curport );
#else
            VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
            PreOnu = NULL;
            CurOnu = PonPortTable[Curport].PendingOnu_Conf.Next;
            while( CurOnu != NULL )
            {		
                NextOnu = CurOnu->Next;

                PonIdx = CurOnu->otherPonIdx;				
                if( (PonIdx == conf_olt_id)
                    && MAC_ADDR_IS_EQUAL(mac, CurOnu->MacAddr) )
                {		
                    if( EVENT_REGISTER == V2R1_ENABLE)
                        sys_console_printf(" pon %d llid %d is to be activated \r\n", PonIdx, CurOnu->Llid );

                    if( PreOnu == NULL )
                    {
                        PonPortTable[Curport].PendingOnu_Conf.Next = CurOnu->Next;				
                    }
                    else
                    {
                        PreOnu->Next = CurOnu->Next;
                    }
#ifdef __pending_onu_list__	
        		CurOnu->Llid = 0;
        		CurOnu->Next = NULL;
        		CurOnu->otherPonIdx = MAXPON;
#else
                    VOS_Free( (void *)CurOnu );
#endif
                    Curport = MAXPON;
                    break;					
                }

                PreOnu = CurOnu;
                CurOnu = NextOnu;
            }
	     VOS_SemGive(OnuPendingDataSemId);
#endif
        }
    }
    else
    {
    	if ( OLT_ERR_NOTEXIST != iRlt )
    	{
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ActiveOneConfPendingOnu(%d, %d) Error Rlt(%d).", olt_id, conf_olt_id, iRlt);
        }
    }
    
    return iRlt;
}

int OLT_GetPendingOnuList(short int olt_id, pendingOnuList_t *onuList)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(onuList);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetPendingOnuList, (olt_id, onuList) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetPendingOnuList(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetUpdatingOnuList(short int olt_id, onuUpdateStatusList_t *onuList)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(onuList);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetUpdatingOnuList, (olt_id, onuList) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetUpdatingOnuList(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetUpdatedOnuList(short int olt_id, onuUpdateStatusList_t *onuList)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(onuList);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetUpdatedOnuList, (olt_id, onuList) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetUpdatedOnuList(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetOnuUpdatingStatusBySlot( short int olt_id, onu_updating_counter_t *pList )
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(pList);
        
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetUpdatingOnuStatus, (olt_id, pList) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOnuUpdatingStatusBySlot(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_SetOnuUpdateMsg( short int olt_id, onu_update_msg_t *pMsg )
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(pMsg);
        
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetUpdateOnuMsg, (olt_id, pMsg) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOnuUpdateMsg(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetOnuUpdateWaiting( short int olt_id, onu_update_waiting_t *pList )
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(pList);
        
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetUpdateWaiting, (olt_id, pList) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOnuUpdateWaiting(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_SetAllOnuAuthMode2(short int olt_id, int auth_mode)
{
    int iRlt = OLT_ERR_OK, ret = 0;
	ULONG slot = 0, port = 0, onu = 0; 
    short int onuid;
    ULONG mode = 0;
	LogicEntity *le;
	UCHAR onuMac[8], macLen;
	unsigned char SN[GPON_ONU_SERIAL_NUM_STR_LEN];
	unsigned char password[GPON_ONU_PASSWARD_STR_LEN];
    ULONG TableIdx = 0;
	gpon_onu_auth_entry_t entry;
	short int olt_id_swap = 0;
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetAllOnuAuthMode2, (olt_id, auth_mode) )) )
	{
		/*更新pon 板，认证使能，不论是否能恢复到pon芯片本地数据都需要修改*/
		slot = GetCardIdxByPonChip(olt_id);
		port = GetPonPortByPonChip(olt_id);
		setOnuAuthEnableForSinglePon(slot, port, auth_mode);   

		if(SYS_MODULE_IS_GPON(slot))
		{
			if(auth_mode == V2R1_ONU_AUTHENTICATION_NEW_ONLY)
			{
				for( onu=1; onu<=MAXONUPERPON; onu++ )
				{
					onuid = onu-1;
					if( GetOnuOperStatus( olt_id, onuid) != ONU_OPER_STATUS_UP )
						continue;
					ONU_MGMT_SEM_TAKE;
					VOS_StrnCpy(password,OnuMgmtTable[olt_id*MAXONUPERPON + onuid].DeviceInfo.DevicePassward,GPON_ONU_PASSWARD_STR_LEN);
					VOS_StrnCpy(SN,OnuMgmtTable[olt_id*MAXONUPERPON + onuid].DeviceInfo.DeviceSerial_No,GPON_ONU_SERIAL_NUM_STR_LEN);
					ONU_MGMT_SEM_GIVE;
					ret=gonu_auth_entry_list_seach(slot,port,SN);
					if(ret ==NULL)
					{
						TableIdx=gonu_auth_entry_free_idx_get(slot,port);
						if( (TableIdx < 1 ) ||( TableIdx > MAXONUPERPON ))
						{
							continue;
						}
						VOS_StrnCpy(entry.authEntry.password,password,GPON_ONU_PASSWARD_STR_LEN);
						VOS_StrnCpy(entry.authEntry.serial_no,SN,GPON_ONU_SERIAL_NUM_STR_LEN);
						entry.authIdx = TableIdx;
						entry.authRowStatus = RS_CREATEANDGO;
						ret = OLT_SetGponAuthEntry(olt_id, &entry);	 
						if(ret != VOS_OK)
						{
							continue;
						}
					}
				}
			}
		}
		else
		{	
			/*修改mac地址认证表*/
			if(mn_getCtcOnuAuthMode(slot, port, &mode) == VOS_OK && mode == mn_ctc_auth_mode_mac)
			{
				if(auth_mode == V2R1_ONU_AUTHENTICATION_NEW_ONLY)
				{
					/*port = GetPonPortByPonChip(ponid);*/
					for( onu=1; onu<=MAXONUPERPON; onu++ )
					{
						onuid = onu-1;
#if 1                    
						if( GetOnuOperStatus( olt_id, onuid) != ONU_OPER_STATUS_UP )
							continue;
#else
						if(ThisIsValidOnu(olt_id, onuid) != ROK)
							continue;
#endif
						/*modi by luh Mac地址自动添加方式为填空，跟onu索引无关联*/
						VOS_SemTake( gLibEponMibSemId, WAIT_FOREVER );
						GetOnuMacAddr( olt_id, onuid, onuMac, &macLen );
						ret = isHaveAuthMacAddress(slot, port, onuMac, &TableIdx );
						if( ret == VOS_OK )
						{
							if ( NULL != (le = getLogicEntityBySlot(slot, port, TableIdx)) )
							{
								le->onu_auth_status = 0;
								VOS_MemSet( le->onu_auth_mac, 0, 6 );
							}
						}           	 
						if ( NULL != (le = getLogicEntityBySlot(slot, port, onu)) )
						{
							le->onu_auth_status = 1;
							VOS_MemCpy(le->onu_auth_mac, onuMac, 6);
						}
						VOS_SemGive( gLibEponMibSemId );                
					}
				}
			}   
		}

		OLT_PARTNER_CALL_BEGIN
		iRlt = OLT_SetAllOnuAuthMode2(olt_id_swap, auth_mode);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "Partner Call OLT_SetAllOnuAuthMode2(%d, %d) Error Rlt(%d).", olt_id_swap, auth_mode, iRlt);
			iRlt = OLT_ERR_OK;
		}
		OLT_PARTNER_CALL_END
	}
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAllOnuAuthMode2(%d, %d) Error Rlt(%d).", olt_id, auth_mode, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
    
    return iRlt;
}

int OLT_SetAllOnuBWParams(short int olt_id, int uplink_bwradio, int dwlink_bwradio)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetAllOnuBWParams, (olt_id, uplink_bwradio, dwlink_bwradio) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetAllOnuBWParams, (olt_id, uplink_bwradio, dwlink_bwradio) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetAllOnuBWParams, (olt_id, uplink_bwradio, dwlink_bwradio) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        if ( uplink_bwradio > 0 )
        {
            Bata_ratio = uplink_bwradio;
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAllOnuBWParams(%d, %d, %d) Error Rlt(%d).", olt_id, uplink_bwradio, dwlink_bwradio, iRlt);
    }

    return iRlt;
}

int OLT_SetOnuP2PMode(short int olt_id, int p2p_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetOnuP2PMode, (olt_id, p2p_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOnuP2PMode(%d, %d) Error Rlt(%d).", olt_id, p2p_mode, iRlt);
    }

    return iRlt;
}

int OLT_GetOnuB2PMode(short int olt_id, int *b2p_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetOnuB2PMode, (olt_id, b2p_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOnuB2PMode(%d, %d) Error Rlt(%d).", olt_id, b2p_mode, iRlt);
    }

    return iRlt;
}

int OLT_SetOnuB2PMode(short int olt_id, int b2p_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetOnuB2PMode, (olt_id, b2p_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOnuB2PMode(%d, %d) Error Rlt(%d).", olt_id, b2p_mode, iRlt);
    }

    return iRlt;
}


/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
int OLT_GetOnuMode( short int olt_id, short int llid )
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
        
    if ( OLT_ERR_OK > (iRlt = OLT_API_CALL2( olt_id, GetOnuMode, (olt_id, llid) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOnuMode(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetMACAddressAuthentication(short int olt_id, unsigned char	*number_of_mac_address, mac_addresses_list_t mac_addresses_list)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
        
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetMACAddressAuthentication, (olt_id, number_of_mac_address, mac_addresses_list ) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetMACAddressAuthentication(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_SetAuthorizeMacAddressAccordingListMode (short int olt_id, bool	authentication_according_to_list)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
        
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetAuthorizeMacAddressAccordingListMode, (olt_id, authentication_according_to_list) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAuthorizeMacAddressAccordingListMode(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetAuthorizeMacAddressAccordingListMode (short int olt_id, bool	*authentication_according_to_list)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
        
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetAuthorizeMacAddressAccordingListMode, (olt_id, authentication_according_to_list) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetAuthorizeMacAddressAccordingListMode(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetDownlinkBufferConfiguration(short int olt_id, PON_downlink_buffer_priority_limits_t *priority_limits)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
        
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetDownlinkBufferConfiguration, (olt_id, priority_limits) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetDownlinkBufferConfiguration(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetOamInformation(short int olt_id, short int llid, PON_oam_information_t  *oam_information)
{
    int iRlt;
	
    OLT_ASSERT(olt_id);
	
	if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetOamInformation, (olt_id, llid, oam_information) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOamInformation(%d) Error Rlt(%d).", olt_id, iRlt);
    }

	return iRlt;
	
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

/*Begin:for onu swap by jinhl@2013-02-22*/
int OLT_ResumeLLIDStatus(short int olt_id, short int llid, int resume_reason, int resume_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, ResumeLLIDStatus, (olt_id, llid, resume_reason, resume_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ResumeLLIDStatus(%d, llid:%d, %d, %d) Error Rlt(%d).", olt_id, llid, resume_reason, resume_mode, iRlt);
    }

    return iRlt;
}

int OLT_SearchFreeOnuIdx(short int olt_id, unsigned char *MacAddress, short int *reg_flag)
{

	int onuIdx;
    OLT_LOCAL_ASSERT(olt_id);
	if( (MacAddress == NULL) || (reg_flag == NULL) )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	
    if ( RERROR != (onuIdx = OLT_API_CALL2( olt_id, SearchFreeOnuIdx, (olt_id, MacAddress, reg_flag) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SearchFreeOnuIdx(%d) Error onuIdx(%d).", olt_id, onuIdx);
    }
    
	return onuIdx;
}


int OLT_GetActOnuIdxByMac(short int olt_id, unsigned char *MacAddress)
{
    int onuIdx;
	
    OLT_LOCAL_ASSERT(olt_id);
	if( MacAddress == NULL )
	{
		VOS_ASSERT(0);
		return RERROR;
	}

	
    if ( RERROR != (onuIdx = OLT_API_CALL2( olt_id, GetActOnuIdxByMac, (olt_id, MacAddress) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetActOnuIdxByMac(%d) Error onuIdx(%d).", olt_id, onuIdx);
    }
    
	return onuIdx;
}
/*End:for onu swap by jinhl@2013-02-22*/

int OLT_BroadCast_CliCommand(short int olt_id, int action_code)
{
    int iRlt;

    OLT_ASSERT(olt_id);

    iRlt = OLT_API_CALL(olt_id, BroadCastCliCommand, (olt_id, action_code));

    if(iRlt == VOS_OK)
    {
        /*
         * mtodo:添加代码，将设置参数写入配置数据表
         */
    }

    return iRlt;
}

int OLT_SetAuthEntry(short int olt_id, int code)
{
    int iRlt, ret = 0;
	ULONG slot = 0, port = 0, onu = 0; 
    short int onuid;
    ULONG mode = 0;
	LogicEntity *le;
	gpon_onu_auth_entry_t entry;
	UCHAR onuMac[8], macLen;
	unsigned char	SN[GPON_ONU_SERIAL_NUM_STR_LEN];
    unsigned char	password[GPON_ONU_PASSWARD_STR_LEN];
    ULONG TableIdx = 0;
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetAuthEntry, (olt_id, code) )) )
    {
        /*更新pon 板，认证使能，不论是否能恢复到pon芯片本地数据都需要修改*/
        slot = GetCardIdxByPonChip(olt_id);
        port = GetPonPortByPonChip(olt_id);

        /*修改mac地址认证表*/
                /*port = GetPonPortByPonChip(ponid);*/
		if(SYS_MODULE_IS_GPON(slot))
		{	
			gpon_onu_auth_t *pList;
			if(code == 1)
			{
				ULONG brdIdx,portIdx,Index=0,NextBrdIdx,NextPortIdx,NextIndex=0;
				brdIdx = slot;
				portIdx = port;
				do{
					
					Index = NextIndex;

					ret =  mn_getNextGponOnuAuthEntryIdx(brdIdx,portIdx,Index, &NextBrdIdx, &NextPortIdx, &NextIndex);
					if(ret == VOS_OK)
					{
						pList = gonu_auth_entry_list_get(NextBrdIdx,NextPortIdx,NextIndex);
						if(pList != NULL)
						{
							onu = GetOnuIdxBySnPerPon(olt_id,pList->authEntry.serial_no);
							if(onu == RERROR)
							{
								VOS_MemZero(entry.authEntry.serial_no,GPON_ONU_SERIAL_NUM_STR_LEN);
								 VOS_MemCpy(entry.authEntry.serial_no, pList->authEntry.serial_no, GPON_ONU_SERIAL_NUM_STR_LEN-1);
								ret =gonu_auth_entry_list_free(slot,port,NextIndex,entry.authEntry.serial_no);
							}
						}
					}
				}while((ret ==ROK) &&( brdIdx == NextBrdIdx ) && ( portIdx == NextPortIdx ) );

				for( onu=1; onu<=MAXONUPERPON; onu++ )
	            {
	            	int status = GetOnuOperStatus_1(olt_id, onu-1);
	                onuid = onu-1;
	                if(ThisIsValidOnu(olt_id, onuid) == ROK
	                    ||( status == ONU_OPER_STATUS_PENDING|| status == ONU_OPER_STATUS_DORMANT))
	                {
	                	ONU_MGMT_SEM_TAKE;
            			VOS_StrCpy(SN,OnuMgmtTable[olt_id*MAXONUPERPON+onuid].DeviceInfo.DeviceSerial_No);
						VOS_StrCpy(password,OnuMgmtTable[olt_id*MAXONUPERPON+onuid].DeviceInfo.DevicePassward);
        				ONU_MGMT_SEM_GIVE;
						pList = gonu_auth_entry_list_seach(slot,port, SN);
						if(pList == NULL)
						{	
							VOS_StrCpy(entry.authEntry.serial_no,SN);
							VOS_StrCpy(entry.authEntry.password,password);
							TableIdx = gonu_auth_entry_free_idx_get(slot, port);
							if( (TableIdx < 1 ) ||( TableIdx > MAXONUPERPON ))
							{
								continue;
							}
							entry.authIdx = TableIdx;
							entry.authRowStatus = RS_CREATEANDGO;
							OLT_SetGponAuthEntry(olt_id,&entry);
						}
	                }
				}
			}
		}
		else
		{
        	if(code == 1)
	        {
	            for( onu=1; onu<=MAXONUPERPON; onu++ )
	            {
	                int status = GetOnuOperStatus_1(olt_id, onu-1);
	                onuid = onu-1;
	                if(ThisIsValidOnu(olt_id, onuid) != ROK
	                    ||( status == ONU_OPER_STATUS_PENDING|| status == ONU_OPER_STATUS_DORMANT))
	                {
	                    VOS_SemTake( gLibEponMibSemId, WAIT_FOREVER );                    
	                    if((le = getLogicEntityBySlot(slot, port, onu))!=NULL)
	                    {
	                        if ( le->onu_auth_status == 1 )
	                        {
	                            le->onu_auth_status = 0;
	                        }                    
	                        VOS_MemSet( le->onu_auth_mac, 0, 6 );
	                    }
	                    VOS_SemGive( gLibEponMibSemId );                
	                    continue;
	                }
	                
	                /*modi by luh Mac地址自动添加方式为填空，跟onu索引无关联*/
	                VOS_SemTake( gLibEponMibSemId, WAIT_FOREVER );
	                GetOnuMacAddr( olt_id, onuid, onuMac, &macLen );
	                ret = isHaveAuthMacAddress(slot, port, onuMac, &TableIdx );
	                if( ret == VOS_OK )
	                {
	                    if ( NULL != (le = getLogicEntityBySlot(slot, port, TableIdx)) )
	                    {
	                        le->onu_auth_status = 0;
	                        VOS_MemSet( le->onu_auth_mac, 0, 6 );
	                    }
	                }           	 
	                if ( NULL != (le = getLogicEntityBySlot(slot, port, onu)) )
	                {
	                    le->onu_auth_status = 1;
	                    VOS_MemCpy(le->onu_auth_mac, onuMac, 6);
	                }
	                VOS_SemGive( gLibEponMibSemId );                
	            }
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
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAuthEntry(%d, %d) Error Rlt(%d).", olt_id, code, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
    
    return iRlt;
}

int OLT_SetGponAuthEntry(short int olt_id, gpon_onu_auth_entry_t *entry)
{
    int iRlt, ret = 0; 
	ULONG slot = 0, port = 0, onu = 0; 
    short int onuid;
    ULONG mode = 0;
	LogicEntity *le;
	UCHAR onuMac[8], macLen;
    ULONG TableIdx = 0;
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetGponOnuAuthEntry, (olt_id, entry) )) )
    {
        /*更新pon 板，认证使能，不论是否能恢复到pon芯片本地数据都需要修改*/
        slot = GetCardIdxByPonChip(olt_id);
        port = GetPonPortByPonChip(olt_id);

        if(entry->authRowStatus == RS_CREATEANDGO)
        {
            iRlt = gonu_auth_entry_list_add(slot, port, entry->authIdx, entry->authEntry.serial_no, entry->authEntry.password);        
        }
        else if(entry->authRowStatus == RS_DESTROY)
        {
            iRlt = gonu_auth_entry_list_free(slot, port, entry->authIdx, entry->authEntry.serial_no);            
        }        
        else if(entry->authRowStatus == RS_NOTINSERVICE)
        {
            iRlt = localsetGponOnuAuthSeriaNoAndPwd(slot, port, entry->authIdx, entry->authEntry.serial_no, entry->authEntry.password);
        }
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetGponAuthEntry(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
    
    return iRlt;
}

int OLT_SetOnuDefaultMaxMac(short int olt_id, int max_mac)
{
    int iRlt, ret = 0;
	ULONG slot = 0, port = 0, onu = 0; 
    short int onuid;
    ULONG mode = 0;
	LogicEntity *le;
	UCHAR onuMac[8], macLen;
    ULONG TableIdx = 0;

    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetOnuDefaultMaxMac, (olt_id, max_mac) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetOnuDefaultMaxMac, (olt_id, max_mac) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetOnuDefaultMaxMac, (olt_id, max_mac) )) )
                {
                    break;
                }
            }
        }
    }
        
    if ( OLT_ERR_OK == iRlt )
    {
        MaxMACDefault = max_mac;
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOnuDefaultMaxMac(%d, %d) Error Rlt(%d).", olt_id, max_mac, iRlt);
    }

    return iRlt;   
}
int OLT_SetMaxOnu(short int olt_id, int max_onu)
{
    int iRlt = 0;
	short int olt_id_swap;	

    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetMaxOnu, (olt_id, max_onu) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetMaxOnu, (olt_id, max_onu) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡上的PAS_SOFT系统参数 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetMaxOnu, (olt_id, max_onu) )) )
                {
                    break;
                }
            }
        }
    }
        
    if ( OLT_ERR_OK == iRlt )
    {
        if(max_onu & CONFIG_CODE_ALL_PORT)
        {
            int PonPortIdx = 0;
            MaxOnuDefault = max_onu&0xff;
            /*modi by luh@2015-4-8. Q.25137*/
            for(PonPortIdx=0; PonPortIdx<MAXPON; PonPortIdx++)
            {
                SetMaxOnuByPonPort(PonPortIdx, max_onu);
            }
        }
        else
        {
            SetMaxOnuByPonPort(olt_id, max_onu);
        }        

		OLT_PARTNER_CALL_BEGIN
		iRlt = OLT_SetMaxOnu(olt_id_swap, max_onu);
		if(OLT_CALL_ISERROR(iRlt))
		{
			VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "Partner Call OLT_SetDefaultMaxOnu(%d, %d) Error Rlt(%d).", olt_id_swap, max_onu, iRlt);
			iRlt = OLT_ERR_OK;
		}
		OLT_PARTNER_CALL_END
		
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetDefaultMaxOnu(%d, %d) Error Rlt(%d).", olt_id, max_onu, iRlt);
    }

    return iRlt;   
}

int OLT_GetOnuConfDelStatus(short int olt_id, char* name, int *status)
{
    int iRlt = 0;

    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, GetOnuConfDelStatus, (olt_id, name, status) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( GetOnuConfDelStatus, (olt_id, name, status) )) )
        {
            /*  ---非集中式设备--- */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, GetOnuConfDelStatus, (olt_id, name, status) )) )
                {
                    break;
                }
                else
                {
                    if(*status)
                        break;
                }
            }
        }
    }


    if ( OLT_ERR_OK != iRlt )
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOnuConfDelStatus(%d) Error Rlt(%d).", olt_id, iRlt);
    }
    else
    {
        if((SYS_LOCAL_MODULE_TYPE_IS_CPU_PON && SYS_LOCAL_MODULE_SLOTNO == GetCardIdxByPonChip(olt_id)))
        {
            *status = GetOnuConfRestoreQueueStatus(name);
        }
    }
    return iRlt;   
}
int OLT_SetCTCOnuPortStatsTimeOut(short int olt_id,ONU_PORTSTATS_TIMER_NAME_E timer_name,LONG timeout)
{
	int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
		iRlt = OLT_API_CALL(olt_id,SetOnuPortStatsTimeOut,(olt_id,timer_name,timeout));
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL(SetOnuPortStatsTimeOut,(olt_id,timer_name,timeout))))
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL(olt_id,SetOnuPortStatsTimeOut,(olt_id,timer_name,timeout))))
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
       Onustats_SetPortStatsTimeOut(olt_id,timer_name,timeout);
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetCTCOnuPortStatsTimeOut(%d, %d, %d) Error Rlt(%d).", olt_id, timer_name, timeout, iRlt);
    }

    return iRlt;   
}

#endif


#if 1
/* -------------------OLT 加密管理API------------------- */

int OLT_SetEncryptMode(short int olt_id, int encrypt_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetEncryptMode, (olt_id, encrypt_mode) )) )
    {
        PonPortTable[olt_id].EncryptType = encrypt_mode;     
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetEncryptMode(%d, %d) Error Rlt(%d).", olt_id, encrypt_mode, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}


/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
int OLT_SetEncryptionPreambleMode(short int olt_id, bool encrypt_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetEncryptionPreambleMode, (olt_id, encrypt_mode) )) )
    {
           
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetEncryptionPreambleMode(%d, %d) Error Rlt(%d).", olt_id, encrypt_mode, iRlt);
        }
        
    }

    return iRlt;
}

int OLT_GetEncryptionPreambleMode(short int olt_id, bool *encrypt_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, GetEncryptionPreambleMode, (olt_id, encrypt_mode) )) )
    {
           
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetEncryptionPreambleMode(%d, %d) Error Rlt(%d).", olt_id, encrypt_mode, iRlt);
        }
        
    }

    return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/


int OLT_GetLLIDEncryptMode(short int olt_id, short int llid, bool *encrypt_mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(encrypt_mode);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetLLIDEncryptMode, (olt_id, llid, encrypt_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetLLIDEncryptMode(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_StartLLIDEncrypt(short int olt_id, short int llid)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, StartLLIDEncrypt, (olt_id, llid) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_StartLLIDEncrypt(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_FinishLLIDEncrypt(short int olt_id, short int llid, short int status)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, FinishLLIDEncrypt, (olt_id, llid, status) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_FinishLLIDEncrypt(%d, %d, %d) Error Rlt(%d).", olt_id, llid, status, iRlt);
    }
    
    return iRlt;    
}

int OLT_StopLLIDEncrypt(short int olt_id, short int llid)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, StopLLIDEncrypt, (olt_id, llid) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_StopLLIDEncrypt(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_SetLLIDEncryptKey(short int olt_id, short int llid, PON_encryption_key_t key)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(key);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetLLIDEncryptKey, (olt_id, llid, key) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetLLIDEncryptKey(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_FinishLLIDEncryptKey(short int olt_id, short int llid, short int status)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, FinishLLIDEncryptKey, (olt_id, llid, status) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_FinishLLIDEncryptKey(%d, %d, %d) Error Rlt(%d).", olt_id, llid, status, iRlt);
    }
    
    return iRlt;    
}
#endif


#if 1
/* -------------------OLT 地址表管理API------------------- */

int OLT_SetMacAgingTime(short int olt_id, int aging_time)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetMacAgingTime, (olt_id, aging_time) )) )
    {
        PonPortTable[olt_id].MACAgeingTime = aging_time;
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetMacAgingTime(%d, %d) Error Rlt(%d).", olt_id, aging_time, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_SetAddressTableConfig(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg)
{
    int iRlt;
	short int olt_id_swap = 0;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(addrtbl_cfg);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetAddrTblCfg, (olt_id, addrtbl_cfg) )) )
    {
        PonPortTable[olt_id].discard_unlearned_sa   = addrtbl_cfg->discard_llid_unlearned_sa;
        PonPortTable[olt_id].table_full_handle_mode = addrtbl_cfg->removed_when_full;
        if ( 0 <= addrtbl_cfg->discard_unknown_da )
        {
            PonPortTable[olt_id].discard_unlearned_da = (unsigned char)addrtbl_cfg->discard_unknown_da;
        }
        if ( 0 <= addrtbl_cfg->aging_timer )
        {
            PonPortTable[olt_id].MACAgeingTime = (unsigned int)addrtbl_cfg->aging_timer;
        }
        if ( 0 <= addrtbl_cfg->allow_learning )
        {
            PonPortTable[olt_id].MacSelfLearningCtrl = (unsigned char)addrtbl_cfg->allow_learning;
        }

		OLT_PARTNER_CALL_BEGIN
		iRlt = OLT_SetAddressTableConfig(olt_id_swap, addrtbl_cfg);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "Partner Call OLT_SetAddressTableConfig(%d) Error Rlt(%d).", olt_id_swap, iRlt);
			iRlt = OLT_ERR_OK;
		}
		OLT_PARTNER_CALL_END

    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAddressTableConfig(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_GetAddressTableConfig(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(addrtbl_cfg);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetAddrTblCfg, (olt_id, addrtbl_cfg) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetAddressTableConfig(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetMacAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(addr_num);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetMacAddrTbl, (olt_id, addr_num, addr_tbl) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetMacAddrTbl(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

/*for GPON by jinhl*/
int OLT_GetMacAddrVlanTbl(short int olt_id, short int *addr_num, PON_address_vlan_table_t addr_tbl)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(addr_num);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetMacAddrVlanTbl, (olt_id, addr_num, addr_tbl) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetMacAddrTbl(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}
int OLT_GetLlidByUserMacAddress(short int olt_id, mac_address_t mac, short int *llid)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(llid);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetLlidByUserMacAddress, (olt_id, mac, llid) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetLlidByUserMacAddress(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}
int OLT_AddMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;

    VOS_ASSERT(addr_num != 0);
    VOS_ASSERT(addr_tbl);

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);

        iRlt = OLT_API_CALL( olt_id, AddMacAddrTbl, (olt_id, addr_num, addr_tbl) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( AddMacAddrTbl, (olt_id, addr_num, addr_tbl) )) )
        {
            short int OltID;
                
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( OltID = 0; OltID < MAXPON; OltID += OLT_ID_SPAN(OltID) )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( OltID, AddMacAddrTbl, (OltID, addr_num, addr_tbl) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        if ( addr_num > 0 )
        {
            iRlt = GW_AddMacAddrTbl(olt_id, addr_num, addr_tbl);
        }
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_AddMacAddrTbl(%d, %d) Error Rlt(%d).", olt_id, addr_num, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
    
    return iRlt;    
}

int OLT_DelMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;

    VOS_ASSERT(addr_num != 0);
    VOS_ASSERT(addr_tbl);

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);

        iRlt = OLT_API_CALL( olt_id, DelMacAddrTbl, (olt_id, addr_num, addr_tbl) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( DelMacAddrTbl, (olt_id, addr_num, addr_tbl) )) )
        {
            short int OltID;
                
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( OltID = 0; OltID < MAXPON; OltID += OLT_ID_SPAN(OltID) )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( OltID, DelMacAddrTbl, (OltID, addr_num, addr_tbl) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        if ( addr_num > 0 )
        {
            iRlt = GW_DelMacAddrTbl(olt_id, addr_num, addr_tbl);
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_DelMacAddrTbl(%d, %d) Error Rlt(%d).", olt_id, addr_num, iRlt);
    }
    
    return iRlt;    
}

int OLT_RemoveMac(short int olt_id, mac_address_t mac)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(mac);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, RemoveMac, (olt_id, mac) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_RemoveMac(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_ResetAddrTbl(short int olt_id, short int llid, int addr_type)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, ResetAddrTbl, (olt_id, llid, addr_type) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ResetAddrTbl(%d, %d, %d) Error Rlt(%d).", olt_id, llid, addr_type, iRlt);
    }
    
    return iRlt;    
}



/*设置ONU MAC 的最大门限*/
int OLT_SetOnuMacThreshold(short int olt_id,  ULONG mac_threshold)
{
    int iRlt;

    VOS_ASSERT(mac_threshold != 0);
    
    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);

        iRlt = OLT_API_CALL( olt_id, SetOnuMacThreshold, (olt_id, mac_threshold) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetOnuMacThreshold, (olt_id, mac_threshold) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetOnuMacThreshold, (olt_id, mac_threshold) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {      
        /*Onu_user_max_all=mac_threshold;*/
#if(EPON_MODULE_ONU_MAC_OVERFLOW_CHECK==EPON_MODULE_YES)
	onu_mac_check_alarm_threshold_set( mac_threshold );
#endif
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOnuMacThreshold(%d, %d) Error Rlt(%d).", olt_id, mac_threshold, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
    
    return iRlt;    
}

/*add by sxh20120210  -------test--------*/
int OLT_SetOnuMacCheckEnable(short int olt_id, ULONG  enable)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetOnuMacCheckEnable, (olt_id, enable) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetOnuMacCheckEnable, (olt_id, enable) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetOnuMacCheckEnable, (olt_id, enable) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        mac_check_exit_flag = enable;
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOnuMacCheckEnable(%d, %d) Error Rlt(%d).", olt_id, enable, iRlt);
    }

    return iRlt;
}

int OLT_SetOnuMacCheckPeriod(short int olt_id, ULONG  period)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetOnuMacCheckPeriod, (olt_id, period) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetOnuMacCheckPeriod, (olt_id, period) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetOnuMacCheckPeriod, (olt_id, period) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        OnuMacTable_check_interval = period;
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOnuMacCheckPeriod(%d, %d) Error Rlt(%d).", olt_id, period, iRlt);
    }

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
int OLT_SetAddressTableFullHandlingMode(short int olt_id, bool remove_entry_when_table_full)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetAddressTableFullHandlingMode, (olt_id, remove_entry_when_table_full) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAddressTableFullHandlingMode(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------OLT 光路管理API------------------- */

int OLT_GetOpticalCapability(short int olt_id, OLT_optical_capability_t *optical_capability)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(optical_capability);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, GetOpticalCapability, (olt_id, optical_capability) )) )
    {
        short int laserOnTime, laserOffTime, AGCTime, CDRTime;

        if ( PON_OPTICAL_CAPABILITY_LOCKTIME | optical_capability->optical_capabilitys )
        {
            AGCTime = optical_capability->agc_lock_time;
            CDRTime = optical_capability->cdr_lock_time;
        }
        else
        {
            AGCTime = 0;
            CDRTime = 0;
        }

        if ( PON_OPTICAL_CAPABILITY_LASERTIME | optical_capability->optical_capabilitys )
        {
            laserOnTime  = optical_capability->laser_on_time;
            laserOffTime = optical_capability->laser_off_time;
        }
        else
        {
            laserOnTime  = 0;
            laserOffTime = 0;
        }
        
		PonChipMgmtTable[olt_id].WorkingMode.laserOnTime  = laserOnTime;
		PonChipMgmtTable[olt_id].WorkingMode.laserOffTime = laserOffTime;
		PonChipMgmtTable[olt_id].WorkingMode.AGCTime = AGCTime;
		PonChipMgmtTable[olt_id].WorkingMode.CDRTime = CDRTime;
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOpticalCapability(%d) Error Rlt(%d).", olt_id, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_GetOpticsDetail(short int olt_id, OLT_optics_detail_t *optics_params) 
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(optics_params);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetOpticsDetail, (olt_id, optics_params) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOpticsDetail(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_SetPonRange(short int olt_id, unsigned int max_range, unsigned int max_rtt)
{
    int iRlt;
	short int olt_id_swap = 0;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetPonRange, (olt_id, max_range, max_rtt) )) )
    {
        PonPortTable[olt_id].range  = max_range;
		PonPortTable[olt_id].MaxRTT = max_rtt;
		
		OLT_PARTNER_CALL_BEGIN
		iRlt = OLT_SetPonRange(olt_id_swap, max_range, max_rtt);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "Partner Call OLT_SetPonRange(%d, %d, %d) Error Rlt(%d).", olt_id_swap, max_range, max_rtt, iRlt);
			iRlt = OLT_ERR_OK;
		}
		OLT_PARTNER_CALL_END
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetPonRange(%d, %d, %d) Error Rlt(%d).", olt_id, max_range, max_rtt, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_SetOpticalTxMode(short int olt_id, int tx_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetOpticalTxMode, (olt_id, tx_mode) )) )
    {
#if 0
        /*Begin:for onu swap by jinhl@2013-04-27*/
        if(tx_mode)
    	{
    	    PonPortTable[olt_id].PortAdminStatus = V2R1_ENABLE;
    	}
		else
		{
		    PonPortTable[olt_id].PortAdminStatus = V2R1_DISABLE;
		}
		/*End:for onu swap by jinhl@2013-04-27*/
#endif
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOpticalTxMode(%d, %d) Error Rlt(%d).", olt_id, tx_mode, iRlt);
    }

    return iRlt;
}

int OLT_GetOpticalTxMode(short int olt_id, int *tx_mode)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetOpticalTxMode, (olt_id, tx_mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetOpticalTxMode(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
int OLT_SetVirtualScopeAdcConfig(short int olt_id, PON_adc_config_t *adc_config)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetVirtualScopeAdcConfig, (olt_id, adc_config) )) )
    {
        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetVirtualScopeAdcConfig(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetVirtualScopeMeasurement(short int olt_id, short int llid, PON_measurement_type_t measurement_type, 
	void *configuration, short int config_len, void *result, short int res_len)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetVirtualScopeMeasurement, (olt_id, llid, measurement_type, configuration, config_len, result, res_len) )) )
    {
        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetVirtualScopeMeasurement(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}


int OLT_GetVirtualScopeRssiMeasurement(short int olt_id, short int llid, PON_rssi_result_t *rssi_result)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetVirtualScopeRssiMeasurement, (olt_id, llid, rssi_result) )) )
    {
        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetVirtualScopeRssiMeasurement(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}


int OLT_GetVirtualScopeOnuVoltage(PON_olt_id_t olt_id, short int llid, float *voltage,unsigned short int *sample, float *dbm)
{
    int iRlt;
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetVirtualScopeOnuVoltage, (olt_id, llid, voltage, sample, dbm) )) )
    {
        
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetVirtualScopeOnuVoltage(%d) Error Rlt(%d).", olt_id, iRlt);
    }

	return iRlt;   
}

int OLT_SetVirtualLLID(short int olt_id, short int llid, PON_virtual_llid_operation_t operation)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetVirtualLLID, (olt_id, llid, operation) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetVirtualLLID(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}
/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */

int OLT_SetOpticalTxMode2(short int olt_id, int tx_mode, int tx_reason)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetOpticalTxMode2, (olt_id, tx_mode, tx_reason) )) )
    {
        if ( (PONPORT_TX_DIRECT < tx_reason) && (tx_reason < PONPORT_TX_ALL) )
        {
            if ( tx_mode )
        	{
        	    PonPortTable[olt_id].TxCtrlFlag &= ~(unsigned char)tx_reason;
        	}
    		else
    		{
        	    PonPortTable[olt_id].TxCtrlFlag |= (unsigned char)tx_reason;
    		}
        }
        else
        {
            if ( tx_mode )
        	{
        	    PonPortTable[olt_id].TxCtrlFlag = 0;
        	}
    		else
    		{
        	    PonPortTable[olt_id].TxCtrlFlag = PONPORT_TX_ALL;
    		}
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetOpticalTxMode2(%d, %d, %d) Error Rlt(%d).", olt_id, tx_mode, tx_reason, iRlt);
    }

    return iRlt;
}

#endif


#if 1
/* -------------------OLT 监控统计管理API------------------- */

int OLT_GetRawStatistics(short int olt_id, OLT_raw_stat_item_t *stat_item)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(stat_item);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetRawStatistics, (olt_id, stat_item) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetRawStatistics(%d, %d, %d) Error Rlt(%d).", olt_id, stat_item->collector_id, stat_item->raw_statistics_type, iRlt);
    }

    return iRlt;
}

int OLT_ResetCounters(short int olt_id)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, ResetCounters, (olt_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ResetCounters(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_SetBerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_bytes)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetBerAlarm, (olt_id, alarm_switch, alarm_thresold, alarm_min_error_bytes) )) )
    {
    	gpOnuAlarm[olt_id]->ponBERAlarm = alarm_switch;
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetBerAlarm(%d, %d, %d, %d) Error Rlt(%d).", olt_id, alarm_switch, alarm_thresold, alarm_min_error_bytes, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_SetFerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_frames)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetFerAlarm, (olt_id, alarm_switch, alarm_thresold, alarm_min_error_frames) )) )
    {
    	gpOnuAlarm[olt_id]->ponFERAlarm = alarm_switch;
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetFerAlarm(%d, %d, %d, %d) Error Rlt(%d).", olt_id, alarm_switch, alarm_thresold, alarm_min_error_frames, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_SetPonBerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_bytes)
{
    int iRlt;
	short int olt_id_swap = 0;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetPonBerAlarm, (olt_id, alarm_switch, alarm_thresold, alarm_min_error_bytes) )) )
    {
    	gpPonAlarm[olt_id]->ponBERAlarm = alarm_switch;

		OLT_PARTNER_CALL_BEGIN
		iRlt = OLT_SetPonBerAlarm(olt_id_swap, alarm_switch, alarm_thresold, alarm_min_error_bytes);
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "Partner Call OLT_SetPonBerAlarm(%d, %d, %d, %d) Error Rlt(%d).", olt_id_swap, alarm_switch, alarm_thresold, alarm_min_error_bytes, iRlt);
			iRlt = OLT_ERR_OK;
		}
		OLT_PARTNER_CALL_END
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetPonBerAlarm(%d, %d, %d, %d) Error Rlt(%d).", olt_id, alarm_switch, alarm_thresold, alarm_min_error_bytes, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_SetPonFerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_frames)
{
    int iRlt;
	short int olt_id_swap = 0;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetPonFerAlarm, (olt_id, alarm_switch, alarm_thresold, alarm_min_error_frames) )) )
    {
    	gpPonAlarm[olt_id]->ponFERAlarm = alarm_switch;
		
		OLT_PARTNER_CALL_BEGIN
		iRlt = OLT_SetPonFerAlarm(olt_id_swap, alarm_switch, alarm_thresold, alarm_min_error_frames); 
		if(OLT_CALL_ISERROR(iRlt))
		{
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "Partner Call OLT_SetPonFerAlarm(%d, %d, %d, %d) Error Rlt(%d).", olt_id_swap, alarm_switch, alarm_thresold, alarm_min_error_frames, iRlt);
		}
		OLT_PARTNER_CALL_END	
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetPonFerAlarm(%d, %d, %d, %d) Error Rlt(%d).", olt_id, alarm_switch, alarm_thresold, alarm_min_error_frames, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_SetBerAlarmParams(short int olt_id, int alarm_thresold, int alarm_min_error_bytes)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetBerAlarmParams, (olt_id, alarm_thresold, alarm_min_error_bytes) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetBerAlarmParams, (olt_id, alarm_thresold, alarm_min_error_bytes) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡上的系统参数 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetBerAlarmParams, (olt_id, alarm_thresold, alarm_min_error_bytes) )) )
                {
                    break;
                }
            }
        }
    }
        
    if ( OLT_ERR_OK == iRlt )
    {
        if ( alarm_thresold >= 0 )
        {
            gPonThreashold.ponBERThreashold = alarm_thresold;
            gPonThreashold.ponBERClearThreashold = (0 < alarm_thresold) ? --alarm_thresold : 0;
        }
        
        if ( alarm_min_error_bytes >= 0 )
        {
            gPonThreashold.ponBerNum = alarm_min_error_bytes;
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetBerAlarmParams(%d, %d, %d) Error Rlt(%d).", olt_id, alarm_thresold, alarm_min_error_bytes, iRlt);
    }

    return iRlt;
}

int OLT_SetFerAlarmParams(short int olt_id, int alarm_thresold, int alarm_min_error_frames)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetFerAlarmParams, (olt_id, alarm_thresold, alarm_min_error_frames) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetFerAlarmParams, (olt_id, alarm_thresold, alarm_min_error_frames) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡上的PAS_SOFT系统参数 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetFerAlarmParams, (olt_id, alarm_thresold, alarm_min_error_frames) )) )
                {
                    break;
                }
            }
        }
    }
        
    if ( OLT_ERR_OK == iRlt )
    {
        if ( alarm_thresold >= 0 )
        {
            gPonThreashold.ponFERThreashold = alarm_thresold;
            gPonThreashold.ponFERClearThreashold = (0 < alarm_thresold) ? --alarm_thresold : 0;
        }
        
        if ( alarm_min_error_frames >= 0 )
        {
            gPonThreashold.ponFerNum = alarm_min_error_frames;
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetFerAlarmParams(%d, %d, %d) Error Rlt(%d).", olt_id, alarm_thresold, alarm_min_error_frames, iRlt);
    }

    return iRlt;
}

/*added by liyang @2015-05-18 for add set ctc onu tx power supply control to olt table*/
int OLT_SetCTCONUTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter)
{
    int iRlt;

	OLT_ASSERT(olt_id);
	
	if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetOnuTxPowerSupplyControl, (olt_id, onu_id, parameter) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetCTCONUTxPowerSupplyControl(%d) Error Rlt(%d).", olt_id, iRlt);
    }

	return iRlt;
	
}



/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
int OLT_SetAlarmConfig(short int olt_id, short int source, PON_alarm_t type, bool activate, void	*configuration, int length)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, SetAlarmConfig, (olt_id, source, type, activate, configuration, length) )) )
    {
    	
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetAlarmConfig(%d, %d, %d, %d) Error Rlt(%d).", olt_id, source, type, activate, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }

    return iRlt;
}

int OLT_GetAlarmConfig(short int olt_id, short int source, PON_alarm_t type, bool *activate, void	*configuration)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
  
	OLT_ASSERT(olt_id);
	
	if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, GetAlarmConfig, (olt_id, source, type, activate, configuration) )) )
    {
    	
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetAlarmConfig(%d, %d, %d, %d) Error Rlt(%d).", olt_id, source, type, activate, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
	

	return iRlt;    
}

int OLT_GetStatistics(short int olt_id, short int collector_id, PON_statistics_t statistics_type, short int statistics_parameter, long double *statistics_data)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetStatistics, (olt_id, collector_id, statistics_type, statistics_parameter, statistics_data) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetStatistics(%d, %d, %d) Error Rlt(%d).", olt_id, collector_id, statistics_type, iRlt);
    }

    return iRlt;
}

int OLT_OltSelfTest(short int olt_id)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL( olt_id, OltSelfTest, (olt_id) )) )
    {
        
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_OltSelfTest(%d) Error Rlt(%d).", olt_id, iRlt);
        }
    }

    return iRlt;
}

int OLT_LinkTest(short int olt_id, short int llid, short int number_of_frames, short int frame_size, bool link_delay_measurement, PON_link_test_vlan_configuration_t *vlan_configuration, PON_link_test_results_t *test_results)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
   
        
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, LinkTest, (olt_id, llid, number_of_frames, frame_size, link_delay_measurement, vlan_configuration, test_results) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_LinkTest(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}


int OLT_SetLLIDFecMode(short int olt_id, short int llid, bool downlink_fec)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetLLIDFecMode, (olt_id, llid, downlink_fec) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetLLIDFecMode(%d, %d, %d) Error Rlt(%d).", olt_id, llid, downlink_fec, iRlt);
    }
    
    return iRlt;    
}

int OLT_GetLLIDFecMode(short int olt_id, short int llid, bool *downlink_fec, bool *uplink_fec, bool *uplink_lastframe_fec)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(downlink_fec);
    VOS_ASSERT(uplink_fec);
    VOS_ASSERT(uplink_lastframe_fec);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetLLIDFecMode, (olt_id, llid, downlink_fec, uplink_fec, uplink_lastframe_fec) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetLLIDFecMode(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}


int OLT_SysDump(short int olt_id, short int llid, int dump_type)
{
    int iRlt;
    short int PonChipType = 0;

	OLT_ASSERT(olt_id);
	
	if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SysDump, (olt_id, llid, dump_type) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SysDump(%d) Error Rlt(%d).", olt_id, iRlt);
    }

	return iRlt;
	
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------OLT 倒换管理API------------------- */

int OLT_GetHotSwapCapability(short int olt_id, int *swap_cap)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(swap_cap);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetHotSwapCapability, (olt_id, swap_cap) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTEXIST != iRlt )
    	{
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetHotSwapCapability(%d) Error Rlt(%d).", olt_id, iRlt);
        }
    }

    return iRlt;
}

int OLT_GetHotSwapMode(short int olt_id, short int *partner_olt_id, int *swap_mode, int *swap_status)
{
    int iRlt;
        
    OLT_ASSERT(olt_id);
    VOS_ASSERT(partner_olt_id);
    VOS_ASSERT(swap_mode);
    VOS_ASSERT(swap_status);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetHotSwapMode, (olt_id, partner_olt_id, swap_mode, swap_status) )) )
    {
        /* 本地状态同步 */
        short int local_olt_id, remote_olt_id;
        short int swap_olt_id, part_olt_id;
        int mem_mode, mem_status;
        int drv_mode, drv_status[2];
        int i;

        if ( OLT_ID_INVALID != *partner_olt_id )
        {
            /* oltID 的本地定位*/
            *partner_olt_id = GetBasePonPortIdx(olt_id, *partner_olt_id);
        }

        if (V2R1_PON_PORT_SWAP_DISABLED != (drv_mode = *swap_mode))
        {
            part_olt_id = *partner_olt_id;
        }
        else
        {
            part_olt_id = OLT_ID_NULL;
        }
        if ( V2R1_PON_PORT_SWAP_UNKNOWN == (drv_status[0] = *swap_status) )
        {
            drv_status[1] = V2R1_PON_PORT_SWAP_UNKNOWN;
        }
        else
        {
            drv_status[1] = OLT_SWAP_STATUS_SWITCHOVER(drv_status[0]);
        }
        
        if ( OLT_ISLOCAL(olt_id) )
        {
            local_olt_id = olt_id;

            if ( OLT_ISLOCAL(part_olt_id) )
            {
                remote_olt_id = part_olt_id;
            }
            else
            {
                remote_olt_id = -1;
            }
        }
        else if ( OLT_ISLOCAL(part_olt_id) )
        {
            local_olt_id = part_olt_id;
            part_olt_id  = olt_id;
            remote_olt_id = -1;

            i = drv_status[0];
            drv_status[0] = drv_status[1];
            drv_status[1] = i;
        }
        else
        {
            iRlt = OLT_ERR_PARAM;
        }

        for(i=0, olt_id = local_olt_id; (i<2) && (olt_id>=0); i++, olt_id = remote_olt_id, part_olt_id = local_olt_id)
        {
            if ( 0 == GW_GetHotSwapMode(olt_id, &swap_olt_id, &mem_mode, &mem_status) )
            {
                if ( swap_olt_id == part_olt_id )
                {
                    if (mem_mode < drv_mode)
                    {
                        PonPortTable[olt_id].swap_mode = drv_mode;
                        if (V2R1_PON_PORT_SWAP_UNKNOWN != drv_status[i])
                        {
                            PonPortTable[olt_id].swap_use = drv_status[i];
                        }
                    }
                }
            }
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetHotSwapMode(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_SetHotSwapMode(short int olt_id, short int partner_olt_id, int swap_mode, int swap_status, int swap_flags)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int old_swap_mode, old_swap_status;
    int swap_slot[2], swap_port[2], swap_state[2];
    short int local_olt_id, remote_olt_id, swap_olt_id;
    short int olt_base_id;
    short int partner_base_id;
	short int siPonPortStatus = PONPORT_UNKNOWN;
	

    OLT_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);
    PON_SWAPMODE_ASSERT(swap_mode);
    PON_SWAPSTATUS_ASSERT(swap_status);

	OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Slot%d OLT_SetHotSwapMode(%d, %d, %d, %d, %d).\r\n", SYS_LOCAL_MODULE_SLOTNO, olt_id, partner_olt_id, swap_mode, swap_status, swap_flags);

    partner_base_id = GetBasePonPortIdx(olt_id, partner_olt_id);
    if ( OLT_ISREMOTE(partner_olt_id) && OLT_ISLOCAL(partner_base_id) )
    {
        partner_olt_id = partner_base_id;
    }
    
    olt_base_id = GetBasePonPortIdx(partner_olt_id, olt_id);
    if ( OLT_ISREMOTE(olt_id) && OLT_ISLOCAL(olt_base_id) )
    {
        olt_id = olt_base_id;
    }

              
    /* 1.---参数调整,确定配置顺序--- */   
    if ( OLT_SWAP_FLAGS_OLT_SINGLESETTING & swap_flags )
    {
        /* 单PON设置，不能改变设置对象 */
    }
    else
    {
        if ( (V2R1_PON_PORT_SWAP_SLOWLY == swap_mode)
            && (V2R1_PON_PORT_SWAP_ACTIVE == swap_status)
            && !(OLT_SWAP_FLAGS_NOTALLWORKING & swap_flags) )    
        {
            /* 通过参数换位，来确保慢倒换的PassiveOlt先配置  */
            swap_olt_id = olt_id;    
            olt_id = partner_olt_id;
            partner_olt_id = swap_olt_id;
			/*added by liyang for Q25852 Q25854  */
			swap_olt_id = partner_base_id;    
            partner_base_id = olt_base_id;
            olt_base_id = swap_olt_id;
			
            swap_status = V2R1_PON_PORT_SWAP_PASSIVE;
        }
        else
        {
            /* 通过参数换位，来确保本地的Olt先配置  */
            if ( OLT_ISREMOTE(olt_id) )
            {
                swap_olt_id = olt_id;    
                olt_id = partner_olt_id;
                partner_olt_id = swap_olt_id;

				swap_olt_id = partner_base_id;    
				partner_base_id = olt_base_id;
				olt_base_id = swap_olt_id;

                swap_status = OLT_SWAP_STATUS_SWITCHOVER(swap_status);
            }
        }
    }

    
    /* 2.---本地OLT配置保存整理---  */
    if ( OLT_ISLOCAL(olt_id) )
    {
        local_olt_id  = olt_id;
        remote_olt_id = partner_olt_id;
        if ( OLT_ISLOCAL(partner_olt_id) )
        {
            swap_olt_id = partner_olt_id ;    
        }
        else
        {
            swap_olt_id = -1;
        }
        
        swap_state[0] = swap_status;
        old_swap_status = PonPortTable[olt_id].swap_use;
    }
    else if ( OLT_ISLOCAL(partner_olt_id) )
    {
        local_olt_id  = partner_olt_id;
        remote_olt_id = olt_id;
        swap_olt_id = -1;
        
        swap_state[0] = OLT_SWAP_STATUS_SWITCHOVER(swap_status);   
        old_swap_status = OLT_SWAP_STATUS_SWITCHOVER(PonPortTable[partner_olt_id].swap_use);
    }
    else
    {
        VOS_ASSERT(0);
        return OLT_ERR_PARAM;
    }

    swap_state[1] = OLT_SWAP_STATUS_SWITCHOVER(swap_state[0]);
    old_swap_mode = PonPortTable[local_olt_id].swap_mode;

    if ( RERROR == (swap_slot[0] = GetCardIdxByPonChip(remote_olt_id)) )
    {
        VOS_ASSERT(0);
        return OLT_ERR_PARAM;
    }

    if ( RERROR == (swap_port[0] = GetPonPortByPonChip(remote_olt_id)) )
    {
        VOS_ASSERT(0);
        return OLT_ERR_PARAM;
    }

    if ( RERROR == (swap_slot[1] = GetCardIdxByPonChip(local_olt_id)) )
    {
        VOS_ASSERT(0);
        return OLT_ERR_PARAM;
    }

    if ( RERROR == (swap_port[1] = GetPonPortByPonChip(local_olt_id)) )
    {
        VOS_ASSERT(0);
        return OLT_ERR_PARAM;
    }


    /* 3.---OLT配置---  */
    if ( OLT_SWAP_FLAGS_OLT_SINGLESETTING & swap_flags )
    {
        /* 仅单PON设置 */
        OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Slot%d SingleCall->OLT_SetHotSwapMode(%d, %d, %d, %d, %d).\r\n", SYS_LOCAL_MODULE_SLOTNO, olt_id, partner_base_id, swap_mode, swap_status, swap_flags);
        iRlt = OLT_API_CALL2( olt_id, SetHotSwapMode, (olt_id, partner_base_id, swap_mode, swap_status, swap_flags) );
    }
    else
    {
        /* OLT倒换保护对配置  */
        OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Slot%d DoubleCall->OLT_SetHotSwapMode(%d, %d, %d, %d, %d).\r\n", SYS_LOCAL_MODULE_SLOTNO, olt_id, partner_base_id, swap_mode, swap_status, swap_flags);
		/* first call of SetHotSwapMode is mainly to turn off current ACTIVE pon port
		 * There's no need to turn off ACTIVE pon port when olt is removed 
		 * */
		//accroding to code review: check pon port status to judge whether pon port is turned off
		if(OLT_ISLOCAL(olt_id))
		{
			siPonPortStatus = GetPonPortOperStatus(olt_id);
		}
		else 
		{
			siPonPortStatus = PONPORT_UNKNOWN;
		}
        if ( 0 != (iRlt = OLT_API_CALL2( olt_id, SetHotSwapMode, (olt_id, partner_base_id, swap_mode, swap_status, swap_flags))) 
				|| /*added by wangjiah@2017-03-23*/ PONPORT_UP != siPonPortStatus) 
        {
            OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"slot:%d, first SetHotSwapMode, olt_id:%d,partner:%d, ret:%d.\r\n", SYS_LOCAL_MODULE_SLOTNO, olt_id, partner_olt_id, iRlt);
            /* 保证保护对儿设置的完整性 */
            if ( OLT_ERR_PARTOK == iRlt || /*added by wangjiah@2017-03-23*/PONPORT_UP != siPonPortStatus)
            {
                /* 单独设置Partner状态，防止递归设置 */
                VOS_ASSERT(0 != swap_mode);                    
				/*added by wangjiah@2017-04-14 begin*/
				/*olt_base_id and parter_olt_id can both be remote*/
				if(OLT_ISREMOTE(partner_olt_id) && OLT_ISREMOTE(olt_base_id))
				{
					if(OLT_ISNET(partner_olt_id) && OLT_ISNET(olt_base_id))
					{
						/*
						if(partner_olt_id != olt_base_id)
						{
							sys_console_printf("olt are both remote net:partner(%d) <-> olt(%d)", partner_olt_id, olt_base_id );
							VOS_ASSERT(0);
						}
						*/
					}
					else
					{
						if(OLT_ISNET(olt_base_id) && !OLT_ISNET(partner_olt_id))
						{
							partner_olt_id = GetPonPortIdxBySlot(OLT_SLOT_ID(partner_olt_id), OLT_PORT_ID(partner_olt_id));	
						}
						else if(OLT_ISNET(partner_olt_id) && !OLT_ISNET(olt_base_id))
						{
							olt_base_id = GetPonPortIdxBySlot(OLT_SLOT_ID(olt_base_id), OLT_PORT_ID(olt_base_id));	
						}
					}
				}
				/*added by wangjiah@2017-04-14 end*/
                iRlt = OLT_API_CALL2( partner_olt_id, SetHotSwapMode, (partner_olt_id, olt_base_id, swap_mode, OLT_SWAP_STATUS_SWITCHOVER(swap_status), OLT_SWAP_FLAGS_OLT_SINGLESETTING | swap_flags) );
				OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"slot:%d second SetHotSwapMode, partner_olt_id:%d, olt_base_id:%d, ret:%d.\r\n", SYS_LOCAL_MODULE_SLOTNO, partner_olt_id, olt_base_id, iRlt);
                if ( 0 != iRlt )
                {
                    if ( OLT_ISNET(partner_olt_id) )
                    {
                        /* 设备间的倒换，尽量保证其成功 */
                        iRlt = 0;
                    }
                    else
                    {
                        /* 因第2个PON口设置失败，而恢复第一个PON口的原设置 */
                        int iTmpRlt = OLT_API_CALL2( olt_id, SetHotSwapMode, (olt_id, partner_base_id, swap_mode, old_swap_status, OLT_SWAP_FLAGS_OLT_SINGLESETTING | OLT_SWAP_FLAGS_OLT_RESUMEMODE | OLT_SWAP_FLAGS_ONLYSETTING | swap_flags) );
                    }
                }
            }
        }
    }


    if ( 0 == iRlt )
    {
        /* 4.---保护口的设置保存--- */
        if ( !(OLT_SWAP_FLAGS_NOTSAVECFG & swap_flags) )
        {
            short int i, sOltId;

            /* B--added by liwei056@2011-11-3 for D13215 */
#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
            if ( OLT_ISNET(remote_olt_id)
                && (V2R1_PON_PORT_SWAP_DISABLED != swap_mode) )
            {
                /* 清空此逻辑OLT相连的其它倒换配置 */
                if ( ROK == PonPortSwapPortLocalQuery(remote_olt_id, &sOltId) )
                {
                    if ( OLT_ISLOCAL(sOltId)
                        && (sOltId != local_olt_id) )
                    {
                        unsigned int old_mode = PonPortTable[sOltId].swap_mode;
                    
                        if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
                        {
                            if ( 0 != (i = OLT_SetHotSwapMode(sOltId, remote_olt_id, V2R1_PON_PORT_SWAP_DISABLED, V2R1_PON_PORT_SWAP_ACTIVE, OLT_SWAP_FLAGS_OLT_SINGLESETTING | OLT_SWAP_FLAGS_ONLYSETTING)) )
                            {
                                VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetHotSwapMode(%d, %d, %d, %d, OnlyCancelSingleCfg) Error Rlt(%d).", sOltId, remote_olt_id, V2R1_PON_PORT_SWAP_DISABLED, V2R1_PON_PORT_SWAP_ACTIVE, i);
                            }
                        }
                        
                        PonPortTable[sOltId].swap_mode = V2R1_PON_PORT_SWAP_DISABLED;
                        PonPortTable[sOltId].swap_monitor = 0;
                        
                        PonPortTable[sOltId].swap_reason = 0;
                        PonPortTable[sOltId].swap_timer = 0;
                        PonPortTable[sOltId].swapping = V2R1_SWAPPING_NO;
                        PonPortTable[sOltId].swapHappened = FALSE;

                        PonPortTable[sOltId].swap_status = 0;
                        PonPortTable[sOltId].swap_times  = 0;

                        PonPortTable[sOltId].swap_slot = 0;
                        PonPortTable[sOltId].swap_port = 0;
                        
                        PonPortTable[sOltId].swap_use  = V2R1_PON_PORT_SWAP_UNKNOWN;
                        PonPortTable[sOltId].swap_flag = V2R1_PON_PORT_SWAP_DISABLE;

                        if ( V2R1_PON_PORT_SWAP_SLOWLY == old_mode )
                        {
                            EnablePonPortLinkedSwPort(sOltId);
                        }

#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
                        if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
                        {
                            ClearPonPortAutoPoetect(sOltId);
                        }
#endif
                    }
                }
            }
#endif
            /* E--added by liwei056@2011-11-3 for D13215 */
                
            for(i=0, sOltId = local_olt_id; (i<2) && (sOltId>=0); i++, sOltId = swap_olt_id)
            {
                PonPortTable[sOltId].swap_mode = swap_mode;
                PonPortTable[sOltId].swap_monitor = 0;
                
                PonPortTable[sOltId].swap_reason = 0;
                PonPortTable[sOltId].swap_timer = 0;
                PonPortTable[sOltId].swapping = V2R1_SWAPPING_NO;
                PonPortTable[sOltId].swapHappened = FALSE;
                
                PonPortTable[sOltId].swap_status = 0;
                PonPortTable[sOltId].swap_times  = 0;
                if ( V2R1_PON_PORT_SWAP_DISABLED == swap_mode )
                {
                    PonPortTable[sOltId].swap_slot = 0;
                    PonPortTable[sOltId].swap_port = 0;
                    
                    PonPortTable[sOltId].swap_use  = V2R1_PON_PORT_SWAP_UNKNOWN;
                    PonPortTable[sOltId].swap_flag = V2R1_PON_PORT_SWAP_DISABLE;

                    if ( V2R1_PON_PORT_SWAP_SLOWLY == old_swap_mode )
                    {
                        EnablePonPortLinkedSwPort(sOltId);
                    }

#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
                    if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
                    {
                        ClearPonPortAutoPoetect(sOltId);
                    }
#endif
                }
                else
                {
                    PonPortTable[sOltId].swap_slot = swap_slot[i];
                    PonPortTable[sOltId].swap_port = swap_port[i];
                    /*for onu swap by jinhl@2013-02-22*/
					/*对于onu倒换模式，状态都是激活状态*/
					if(V2R1_PON_PORT_SWAP_ONU == swap_mode)
					{
						PonPortTable[sOltId].swap_use  = V2R1_PON_PORT_SWAP_ACTIVE; 
					}
					else
					{
					    PonPortTable[sOltId].swap_use  = swap_state[i];
					}
                    PonPortTable[sOltId].swap_flag = V2R1_PON_PORT_SWAP_ENABLE;

                    if ( V2R1_PON_PORT_SWAP_ACTIVE == swap_state[i] )
                    {
                        if ( V2R1_PON_PORT_SWAP_SLOWLY == swap_mode )
                        {
                            EnablePonPortLinkedSwPort( sOltId );
                        }
                        
#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
                        if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
                        {
                            SetPonPortAutoProtectAcitve( sOltId );
                        }
#endif
                    }
                    else
                    {
                        if ( V2R1_PON_PORT_SWAP_SLOWLY == swap_mode )
                        {
                            DisablePonPortLinkedSwPort( sOltId );
                        }
                        
#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
                        if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
                        {
                            SetPonPortAutoProtectPassive( sOltId );
                        }
#endif
                    }
                }
				OLT_SWITCH_DEBUG("\r\n olt:%d, swap_use:%d, swap_slot:%d, swap_port:%d\r\n", sOltId, PonPortTable[sOltId].swap_use, PonPortTable[sOltId].swap_slot,PonPortTable[sOltId].swap_port);
            }
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetHotSwapMode(%d, %d, %d, %d, %d) Error Rlt(%d).", olt_id, partner_olt_id, swap_mode, swap_status, swap_flags, iRlt);
    }

    return iRlt;
}

int OLT_ForceHotSwap(short int olt_id, short int partner_olt_id, int swap_status, int swap_flags)
{
    int iRlt;
    short int olt_base_id;
    short int partner_base_id;
        
    OLT_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);

    partner_base_id = GetBasePonPortIdx(olt_id, partner_olt_id);

    if ( swap_flags & OLT_SWAP_FLAGS_OLT_SINGLESETTING )
    {
        /* 仅作中转调用 */
        iRlt = OLT_API_CALL2( olt_id, ForceHotSwap, (olt_id, partner_base_id, swap_status, swap_flags) );
    }
    else
    {
        /* 保证切换对儿倒换的完整性 */
        if ( OLT_ERR_PARTOK == (iRlt = OLT_API_CALL2( olt_id, ForceHotSwap, (olt_id, partner_base_id, swap_status, swap_flags) )) )
        {
            olt_base_id = GetBasePonPortIdx(partner_olt_id, olt_id);
            iRlt = OLT_API_CALL2( partner_olt_id, ForceHotSwap, (partner_olt_id, olt_base_id, swap_status, swap_flags | OLT_SWAP_FLAGS_OLT_SINGLESETTING) );
            if ( OLT_ERR_PARTOK == iRlt )
            {
                iRlt = 0;
            }
        }
    }

    if ( 0 != iRlt )
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ForceHotSwap(%d, %d, %d, %d) Error Rlt(%d).", olt_id, partner_olt_id, swap_status, swap_flags, iRlt);
    }

    return iRlt;
}

int OLT_SetHotSwapParam(short int olt_id, int swap_enable, int swap_time, int rpc_mode, int swap_triggers, int trigger_param1, int trigger_param2)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);
        iRlt = OLT_API_CALL( olt_id, SetHotSwapParam, (olt_id, swap_enable, swap_time, rpc_mode, swap_triggers, trigger_param1, trigger_param2) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetHotSwapParam, (olt_id, swap_enable, swap_time, rpc_mode, swap_triggers, trigger_param1, trigger_param2) )) )
        {
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( olt_id, SetHotSwapParam, (olt_id, swap_enable, swap_time, rpc_mode, swap_triggers, trigger_param1, trigger_param2) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        if ( swap_enable > 0 )
        {
    		pon_swap_switch_enable = swap_enable;
        }
        
        if ( swap_time > 0 )
        {
    		V2R1_AutoProtect_Timer = (unsigned int)swap_time;
        }
        else
        {
    		V2R1_AutoProtect_Timer = V2R1_PON_PORT_SWAP_TIMER;
        }
        
        if ( rpc_mode >= 0 )
        {
    		g_ulRPCCall_mode = (ULONG)rpc_mode;
        }

        if ( swap_triggers >= 0 )
        {
            if ( OLT_ID_ALL == olt_id )
            {
                if ( swap_triggers > 0 )
                {
                    V2R1_AutoProtect_Trigger = swap_triggers;
                }
                else
                {
                    V2R1_AutoProtect_Trigger = V2R1_PON_PORT_SWAP_TRIGGER;
                }
            }
            else
            {
                if ( swap_triggers > 0 )
                {
                    PonPortTable[olt_id].swap_triggers = swap_triggers;
                    if ( swap_triggers & PROTECT_SWITCH_TRIGGER_UPLINKDOWN )
                    {
                        if ( (trigger_param1 > 0) && (trigger_param2 > 0) )
                        {
                            PonPortTable[olt_id].protect_slot = (unsigned char)trigger_param1;
                            PonPortTable[olt_id].protect_port = (unsigned char)trigger_param2;
                        }
                    }
                    else
                    {
                        PonPortTable[olt_id].protect_slot = 0;
                        PonPortTable[olt_id].protect_port = 0;
                    }
                }
                else
                {
                    PonPortTable[olt_id].swap_triggers = V2R1_PON_PORT_SWAP_TRIGGER;
                    PonPortTable[olt_id].protect_slot  = 0;
                    PonPortTable[olt_id].protect_port  = 0;
                }
            }
        }
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetHotSwapParam(%d, %d, %d, %d, %d, %d, %d) Error Rlt(%d).", olt_id, swap_enable, swap_time, rpc_mode, swap_triggers, trigger_param1, trigger_param2, iRlt);
    }

    return iRlt;
}


int OLT_RdnOnuRegister(short int olt_id, PON_redundancy_onu_register_t *onu_reg_info)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(onu_reg_info);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, RdnOnuRegister, (olt_id, onu_reg_info) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_RdnOnuRegister(%d) Error Rlt(%d).", olt_id, iRlt);
    }
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"OLT_RdnOnuRegister(%d, %d, [%02x.%02x.%02x.%02x.%02x.%02x])'s result(%d) on slot %d.\r\n", olt_id, onu_reg_info->onu_id, onu_reg_info->onu_mac[0], onu_reg_info->onu_mac[1], onu_reg_info->onu_mac[2], onu_reg_info->onu_mac[3], onu_reg_info->onu_mac[4], onu_reg_info->onu_mac[5], iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


int OLT_SetRdnConfig(short int olt_id, int rdn_status, int gpio_num, int rdn_type, int rx_enable)
{
    int iRlt;

    OLT_ASSERT(olt_id);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetRdnConfig, (olt_id, rdn_status, gpio_num, rdn_type, rx_enable) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetRdnConfig(%d, %d, %d, %d, %d) Error Rlt(%d).", olt_id, rdn_status, gpio_num, rdn_type, rx_enable, iRlt);
    }
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"OLT_SetRdnConfig(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, rdn_status, gpio_num, rdn_type, rx_enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int OLT_RdnSwitchOver(short int olt_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, RdnSwitchOver, (olt_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_RdnSwitchOver(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_RdnIsExist(short int olt_id, bool *status)
{
    OLT_ASSERT(olt_id);
    VOS_ASSERT(status);
    
    return OLT_API_CALL2( olt_id, RdnIsExist, (olt_id, status) ); 
}

int OLT_ResetRdnRecord(short int olt_id, int rdn_state)
{
    int iRlt;

    OLT_ASSERT(olt_id);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, ResetRdnRecord, (olt_id, rdn_state) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_ResetRdnOltRecord(%d, %d) Error Rlt(%d).", olt_id, rdn_state, iRlt);
    }

    return iRlt;
}


int OLT_GetRdnState(short int olt_id, int *state)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(state);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetRdnState, (olt_id, state) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTEXIST != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetRdnState(%d) Error Rlt(%d).", olt_id, iRlt);
        }
    }

    return iRlt;
}

int OLT_SetRdnState(short int olt_id, int state)
{
    int iRlt;

    OLT_ASSERT(olt_id);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetRdnState, (olt_id, state) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetRdnState(%d, %d) Error Rlt(%d).", olt_id, state, iRlt);
    }
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"OLT_SetRdnState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, state, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int OLT_RemoveRdnOlt(short int olt_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, RemoveRdnOlt, (olt_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_RemoveRdnOlt(%d) Error Rlt(%d).", olt_id, iRlt);
    }

    return iRlt;
}

int OLT_GetLLIDRdnDB(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(rdn_db);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetLLIDRdnDB, (olt_id, llid, rdn_db) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetLLIDRdnDB(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;
}

int OLT_SetLLIDRdnDB(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(rdn_db);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetLLIDRdnDB, (olt_id, llid, rdn_db) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetLLIDRdnDB(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    
    return iRlt;
}

int OLT_GetRdnAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(addr_num);
    VOS_ASSERT(addr_tbl);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetRdnAddrTbl, (olt_id, addr_num, addr_tbl) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetRdnAddrTbl(%d) Error Rlt(%d).", olt_id, iRlt);
    }
    
    return iRlt;    
}

int OLT_RdnRemoveOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);

    partner_olt_id = GetBasePonPortIdx(olt_id, partner_olt_id);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, RdnRemoveOlt, (olt_id, partner_olt_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_RdnRemoveOlt(%d, %d) Error Rlt(%d).", olt_id, partner_olt_id, iRlt);
    }

    return iRlt;
}

int OLT_RdnSwapOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);

    partner_olt_id = GetBasePonPortIdx(olt_id, partner_olt_id);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, RdnSwapOlt, (olt_id, partner_olt_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_RdnSwapOlt(%d, %d) Error Rlt(%d).", olt_id, partner_olt_id, iRlt);
    }

    return iRlt;
}

int OLT_AddSwapOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);

    partner_olt_id = GetBasePonPortIdx(olt_id, partner_olt_id);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, AddSwapOlt, (olt_id, partner_olt_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_AddSwapOlt(%d, %d) Error Rlt(%d).", olt_id, partner_olt_id, iRlt);
    }

    return iRlt;
}

int OLT_RdnLooseOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);

    partner_olt_id = GetBasePonPortIdx(olt_id, partner_olt_id);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, RdnLooseOlt, (olt_id, partner_olt_id) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_RdnLooseOlt(%d, %d) Error Rlt(%d).", olt_id, partner_olt_id, iRlt);
    }

    return iRlt;
}


/*Begin:for onu swap by jinhl@2013-02-22*/
int OLT_RdnLLIDAdd(short int olt_id, short int llid)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
   
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, RdnLLIDAdd, (olt_id, llid) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_RdnLLIDAdd(%d, %d) Error Rlt(%d)", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_GetRdnLLIDMode(short int olt_id, short int llid, PON_redundancy_llid_redundancy_mode_t* mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
   
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetRdnLLIDMode, (olt_id, llid, mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetRdnLLIDMode(%d, %d) Error Rlt(%d)", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_SetRdnLLIDMode(short int olt_id, short int llid, PON_redundancy_llid_redundancy_mode_t mode)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
   
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetRdnLLIDMode, (olt_id, llid, mode) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetRdnLLIDMode(%d, %d) Error Rlt(%d)", olt_id, llid, iRlt);
    }
    
    return iRlt;    
}

int OLT_SetRdnLLIDStdbyToAct(short int olt_id, short int llid_n, short int* llid_list_marker)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    /*VOS_ASSERT(llid_list_marker);*/
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetRdnLLIDStdbyToAct, (olt_id, llid_n, llid_list_marker) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetRdnLLIDStdbyToAct(%d) Error Rlt(%d)", olt_id, iRlt);
    }
	
    
    return iRlt;    
}
int OLT_SetRdnLLIDRtt(short int olt_id, short int llid, PON_rtt_t rtt)
{
    int iRlt;

    OLT_ASSERT(olt_id);
       
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, SetRdnLLIDRtt, (olt_id, llid, rtt) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetRdnLLIDRtt(%d) Error Rlt(%d)", olt_id, iRlt);
    }
    
    return iRlt;    
}
/*End:for onu swap by jinhl@2013-02-22*/


int OLT_RdnIsReady(short int olt_id, int *iIsReady)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(iIsReady);
    
    /* B--added by liwei056@2011-11-15 for D13810 */
    if ( OLT_ISNET(olt_id) && SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
    {
#if ( EPON_MODULE_PON_REMOTE_MANAGE == EPON_MODULE_YES )
        /* 等待逻辑槽位全部激活，再开始监控备用口 */
        if ( devsm_remote_slot_isrunning(OLT_SLOT_ID(olt_id)) )
        {
            if ( FALSE == OLTRM_RemoteOltIsValid(olt_id) )
            {
                *iIsReady = 0;

                return OLT_ERR_NOTEXIST; 
            }
        }
        else
#endif
        {
            *iIsReady = 0;

            return OLT_ERR_NOTEXIST; 
        }
    }
    /* E--added by liwei056@2011-11-15 for D13810 */

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, RdnIsReady, (olt_id, iIsReady) )) )
    {
    }
    else
    {
    	if ( OLT_ERR_NOTEXIST != iRlt )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_RdnIsReady(%d, %d) Error Rlt(%d).", olt_id, *iIsReady, iRlt);
        }
    }

    return iRlt;    
}

int OLT_GetLLIDRdnRegisterInfo(short int olt_id, short int llid, PON_redundancy_onu_register_t *onu_reginfo)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_reginfo);
    
    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, GetLLIDRdnRegisterInfo, (olt_id, llid, onu_reginfo) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_GetLLIDRdnRegisterInfo(%d, %d) Error Rlt(%d).", olt_id, llid, iRlt);
    }
    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"OLT_GetLLIDRdnRegisterInfo(%d, %d, [%02x.%02x.%02x.%02x.%02x.%02x])'s result(%d) on slot %d.\r\n", olt_id, onu_reginfo->onu_id, onu_reginfo->onu_mac[0], onu_reginfo->onu_mac[1], onu_reginfo->onu_mac[2], onu_reginfo->onu_mac[3], onu_reginfo->onu_mac[4], onu_reginfo->onu_mac[5], iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */

int OLT_DumpAllCmc(short int olt_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);

    if ( OLT_ERR_OK == (iRlt = OLT_API_CALL2( olt_id, DumpAllCmc, (olt_id, dump_buf, dump_len) )) )
    {
    }
    else
    {
        VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_DumpAllCmc(%d, %d) Error Rlt(%d).", olt_id, *dump_len, iRlt);
    }

    return iRlt;
}

int OLT_SetCmcServiceVID(short int olt_id, int svlan)
{
    int iRlt;

    if ( OLT_ID_ALL != olt_id )
    {   
        OLT_ASSERT(olt_id);

        iRlt = OLT_API_CALL( olt_id, SetCmcServiceVID, (olt_id, svlan) );
    }
    else
    {
        if ( OLT_ERR_NOTSUPPORT == (iRlt = OLT_SYS_CALL( SetCmcServiceVID, (olt_id, svlan) )) )
        {
            short int OltID;
                
            /*  ---非集中式设备--- */
            /* 遍历设置所有PON卡 */
            for ( OltID = 0; OltID < MAXPON; OltID += OLT_ID_SPAN(OltID) )
            {
                if ( OLT_ERR_OK != (iRlt = OLT_API_CALL( OltID, SetCmcServiceVID, (OltID, svlan) )) )
                {
                    break;
                }
            }
        }
    }

    if ( OLT_ERR_OK == iRlt )
    {
        if ( svlan >= 0 )
        {
            cmc_service_vlanid = (unsigned short)svlan;
        }
    }
    else
    {
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            VOS_SysLog(LOG_TYPE_OLT, olt_mgt_log_level, "OLT_SetCmcServiceVID(%d, %d) Error Rlt(%d).", olt_id, svlan, iRlt);
        }
        else
        {
            iRlt = OLT_ERR_OK;
        }
    }
    
    return iRlt;    
}

#endif


#undef OLT_SYS_CALL
#undef OLT_LOCAL_CALL
#undef OLT_REMOTE_CALL
#undef OLT_API_CALL
#undef OLT_REMOTE_CALL2
#undef OLT_API_CALL2
#endif


/* -------------------OLT高级API------------------- */
/* 为了RPC的效率，增加了此本地判断功能支持的公用函数 */
/* 目前仅供RPC调用处使用，慎用。 */
int OLTAdv_CmdIsSupported(short int olt_id, int cmd)
{
    int iRlt = 0;
    short int sIsSupported;

    sIsSupported = cmd;
    if ( 0 == OLT_CmdIsSupported(olt_id, &sIsSupported) )
    {
        if (sIsSupported == cmd)
        {
            iRlt = 1;
        }
    }
    
    return iRlt;
}

int OLTAdv_TestOltID(short int olt_id, void *vty)
{
    short int sGlobalID, sLocalID;

    if ( OLT_ISLOCAL(olt_id) )
    {
        if ( olt_id < MAXPON )
        {
            sLocalID  = olt_id;
            sGlobalID = GetGlobalPonPortIdx(olt_id);
        }
        else
        {
            sGlobalID = olt_id;
            sLocalID  = GetLocalPonPortIdx(olt_id);
        }
    }
    else
    {
        sGlobalID = GetGlobalPonPortIdxBySlot(OLT_SLOT_ID(olt_id), OLT_PORT_ID(olt_id));
        sLocalID  = GetLocalPonPortIdx(sGlobalID);
    }

    if ( NULL != vty )
    {
        vty_out(vty, "OLTAdv_TestSysLog(olt%d: globalid=%d, localid=%d) on slot(%d)\r\n", olt_id, sGlobalID, sLocalID, SYS_LOCAL_MODULE_SLOTNO);
    }
    else
    {
        sys_console_printf("OLTAdv_TestSysLog(olt%d: globalid=%d, localid=%d) on slot(%d)\r\n", olt_id, sGlobalID, sLocalID, SYS_LOCAL_MODULE_SLOTNO);
    }

    VOS_SysLog(LOG_TYPE_OLT, LOG_WARNING, "OLTAdv_TestSysLog(olt%d: globalid=%d, localid=%d) on slot(%d)", olt_id, sGlobalID, sLocalID, SYS_LOCAL_MODULE_SLOTNO);
    return 0;
}

int OLTAdv_IsExist(short int olt_id)
{
    bool bExist = 0;

    (void)OLT_IsExist(olt_id, &bExist);
    
    return (int)bExist; 
}

int OLTAdv_GetChipTypeID(short int olt_id)
{
    int type = PONCHIP_UNKNOWN;

    (void)OLT_GetChipTypeID(olt_id, &type);
    
    return type; 
}

const char* OLTAdv_GetChipTypeName(short int olt_id)
{
    static char typename[OLT_CHIP_NAMELEN];

    if ( OLT_CALL_ISERROR(OLT_GetChipTypeName(olt_id, typename)) )
    {
        return OLT_CHIP_UNKNOWN_NAME;
    }

    return typename;
}

int OLTAdv_SetCardI2CInfo(short int olt_id, int boardtype, char *boardsn, char *boardver, char *boarddate)
{
    board_sysinfo_t board_info;
        
    *((int*)board_info.ext_data) = boardtype;
    if ( NULL != boardsn )
    {
        VOS_MemZero(board_info.serialno, sizeof(board_info.serialno));
        VOS_StrnCpy(board_info.serialno, boardsn, 16);
    }
    else
    {
        board_info.serialno[0] = '\0';
    }

    if ( NULL != boardver )
    {
        VOS_MemZero(board_info.version, sizeof(board_info.version));
        VOS_StrnCpy(board_info.version, boardver, 16);
    }
    else
    {
        board_info.version[0] = '\0';
    }

    if ( NULL != boarddate )
    {
    	 VOS_MemZero(board_info.product_date, sizeof(board_info.product_date));
        VOS_StrnCpy(board_info.product_date, boarddate, 10);
    }
    else
    {
        board_info.product_date[0] = '\0';
    }

    return OLT_SetCardI2CInfo(olt_id, &board_info);
}

int OLTAdv_GetCardI2CInfo(short int olt_id, int *boardtype, char *boardsn, char *boardver, char *boarddate)
{
    int iRlt;
    board_sysinfo_t board_info;
        
    iRlt = OLT_GetCardI2CInfo(olt_id, &board_info);
    if ( 0 == iRlt )
    {
        if ( NULL != boardtype )
        {
            *boardtype = *((int*)board_info.ext_data);
        }
    
        if ( NULL != boardsn )
        {
            VOS_StrCpy(boardsn, board_info.serialno/*, 16*/);
        }

        if ( NULL != boardver )
        {
            VOS_StrCpy(boardver, board_info.version/*, 16*/);
        }

        if ( NULL != boarddate )
        {
            VOS_StrCpy(boarddate, board_info.product_date/*, 10*/);
        }
    }

    return iRlt;
}

int OLTAdv_SetPonLED(short int olt_id, short int led_id, int led_status)
{
    return OLT_WriteGpio(olt_id, led_id, led_status);
}

bool OLTAdv_ChkVersion(short int olt_id)
{
    bool is_compatibled = TRUE;

    (void)OLT_ChkVersion(olt_id, &is_compatibled);

    return is_compatibled;
}

bool OLTAdv_ChkDBAVersion(short int olt_id)
{
    bool is_compatibled = TRUE;

    (void)OLT_ChkDBAVersion(olt_id, &is_compatibled);

    return is_compatibled;
}

int OLTAdv_GetAdminStatus(short int olt_id)
{
    int admin_status = PONPORT_ENABLE;
        
    if ( OLT_ISLOCAL(olt_id) )
    {
        (void)GW_GetAdminStatus(olt_id, &admin_status);
    }
    else
    {
        (void)OLT_GetAdminStatus(olt_id, &admin_status); 
    }

    return admin_status;
}

int OLTAdv_GetPonWorkStatus(short int olt_id)
{
    int work_status;
        
    if ( OLT_ISLOCAL(olt_id) )
    {
        (void)GW_GetPonWorkStatus(olt_id, &work_status);
    }
    else
    {
        int iRlt;
        
        iRlt = OLT_GetPonWorkStatus(olt_id, &work_status); 
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            if ( OLT_ERR_NOTEXIST == iRlt )
            {
                work_status = PONPORT_DOWN;
            }
            else
            {
                work_status = PONPORT_UNKNOWN;
            }       
        }
    }

    return work_status;
}

int OLTAdv_SetIgmpAuthMode(short int olt_id, int auth_mode)
{
    return OLT_SetIgmpAuthMode(olt_id, 2 - auth_mode);
}

int OLTAdv_ChkOnuRegisterControl( short int olt_id, short int llid, mac_address_t mac_address, short int *bind_olt_id  )
{
	return OLT_ChkOnuRegisterControl(olt_id, llid, mac_address, bind_olt_id);
}

int OLTAdv_GetOpticalTxMode(short int olt_id)
{
    int tx_mode = -1;
    
    (void)OLT_GetOpticalTxMode(olt_id, &tx_mode);

    return tx_mode;
}

int OLTAdv_SetOpticalTxMode2(short int olt_id, int tx_mode, int tx_reason)
{
    OLT_ASSERT(olt_id);

    if ( (PONPORT_TX_DIRECT < tx_reason) && (tx_reason < PONPORT_TX_ALL) )
    {
        if ( tx_mode )
    	{
    	    if ( !(PonPortTable[olt_id].TxCtrlFlag & tx_reason) )
            {
                return 0;
            }   
    	}
    	else
    	{
    	    if ( PonPortTable[olt_id].TxCtrlFlag & tx_reason )
            {
                return 0;
            }   
    	}
    }

    return OLT_SetOpticalTxMode2(olt_id, tx_mode, tx_reason);
}

int OLTAdv_HotSwapSelectMaster(short int *olt_id, short int *partner_id, short int *master_onunum, short int *slave_onunum)
{
	int slot1, slot2;
	int countonu1, countonu2;
	int Pon1Status, Pon2Status;
	short int master_id, slave_id;

	VOS_ASSERT(olt_id);
	VOS_ASSERT(partner_id);

	master_id = *olt_id;
	slave_id = *partner_id;

	*master_onunum = -1;
	*slave_onunum  = -1;

	if(!(OLT_ISNET(master_id) || OLT_ISNET(slave_id)) )
	{
		/* added by wangjiah@2017-06-14 to solve issue:36735&38654:begin 
		 * First of all, select master pon according to previous config when inner-olt protection is configured.
		 * */
		Pon1Status = PonPortHotStatusQuery(master_id);
		Pon2Status = PonPortHotStatusQuery(slave_id);

		OLT_SWITCH_DEBUG("\r\nHotSwapSelectMaster: already set :pon:%d->%s and pon:%d->%s.", master_id, Pon1Status == V2R1_PON_PORT_SWAP_ACTIVE ? "Active" : "PASSIVE", slave_id, Pon2Status == V2R1_PON_PORT_SWAP_ACTIVE ? "Active" : "PASSIVE");

		if( Pon1Status == V2R1_PON_PORT_SWAP_ACTIVE && Pon2Status == V2R1_PON_PORT_SWAP_PASSIVE )
		{
			return 0;
		}

		if( Pon1Status == V2R1_PON_PORT_SWAP_PASSIVE && Pon2Status == V2R1_PON_PORT_SWAP_ACTIVE )
		{
			*olt_id = slave_id;
			*partner_id = master_id;
			return 0;
		}
		/* added by wangjiah@2017-06-14 :end*/
	}

    Pon1Status = PonPortIsWorking(master_id);
    Pon2Status = PonPortIsWorking(slave_id);

    if(( Pon1Status == TRUE ) && ( Pon2Status == TRUE ))
    {
    	/* modified by chenfj 2007-7-19
                	问题单4912.在当前active的pon口号大的时候，在设置它和端口小的pon口保护，在线ONU会先离线，再注册。
                	解决:增加两个PON口下是否有ONU 在线为条件
        	*/
    	countonu1 = 0;
    	(void)OLT_GetOnuNum(master_id, OLT_ONUFLAG_ONREG, &countonu1);
    	countonu2 = 0;
        (void)OLT_GetOnuNum(slave_id, OLT_ONUFLAG_ONREG, &countonu2);
		OLT_SWITCH_DEBUG("\r\nHotSwapSelectMaster: both true status: master %d( %d onu(s)), slave %d (%d onu(s))\r\n", master_id, countonu1, slave_id, countonu2);

        if ( (0 == countonu1) || (0 == countonu2) )
        {
            if ( 0 != countonu1 )
            {
                *master_onunum = countonu1;
                *slave_onunum  = 0;

                return 0;
            }
            else if ( 0 != countonu2 )
            {
                *olt_id     = slave_id;
                *partner_id = master_id;

                *master_onunum = countonu2;
                *slave_onunum  = 0;

                return 0;
            }
            else
            {
                *master_onunum = 0;;
                *slave_onunum  = 0;
            }
        }
        else
        {
            /* 都有ONU注册 */
            if ( countonu1 > countonu2 )
            {
                *master_onunum = countonu1;
                *slave_onunum  = countonu2;

                return 0;
            }
            else if ( countonu1 < countonu2 )
            {
                *olt_id     = slave_id;
                *partner_id = master_id;

                *master_onunum = countonu2;
                *slave_onunum  = countonu1;

                return 0;
            }
            else
            {
                *master_onunum = countonu1;
                *slave_onunum  = countonu2;
            }
        }

        /* 都无ONU注册，一个不发光，一个发光，则应翻转设置 */
    	countonu1 = 1;
        OLT_GetOpticalTxMode(master_id, &countonu1);
    	countonu2 = 1;
        OLT_GetOpticalTxMode(slave_id, &countonu2);
		OLT_SWITCH_DEBUG("\r\nHotSwapSeletctMaster: both true status: master %d %s, slave %d %s\r\n", 
				master_id, (countonu1 == 0 ? "off":"on") ,slave_id, (countonu2 == 0 ? "off":"on"));

        if ((countonu1 > 0) && (0 == countonu2))
        {
            return 0;
        }
        else if ((countonu2 > 0) && (0 == countonu1))
        {
            *olt_id = slave_id;
            *partner_id = master_id;
            return 0;
        }
    }

#ifdef  _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
    if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
    {
        if ( OLT_ISLOCAL(master_id) && OLT_ISLOCAL(slave_id) )
        {
        	if((GetPonPortAutoProtectStatus(master_id) == AUTO_PROTECT_AND_ACTIVE) && (Pon1Status == PONPORT_UP))
        	{
				OLT_SWITCH_DEBUG("\r\nHotSwapSeletctMaster: both local: master %d active\r\n", master_id); 
                return 0;
        	}
        	else if((GetPonPortAutoProtectStatus(slave_id) == AUTO_PROTECT_AND_ACTIVE) && (Pon2Status == PONPORT_UP))
        	{
				OLT_SWITCH_DEBUG("\r\nHotSwapSeletctMaster: both local: master %d active\r\n", slave_id); 
                *olt_id = slave_id;
                *partner_id = master_id;

                return 0;
        	}
        }
    }
#endif
	
	OLT_SWITCH_DEBUG("\r\nHotSwapSeletctMaster: single true status: master %d %s, slave %d %s\r\n", 
			master_id, (Pon1Status == TRUE ? "true" : "false"), slave_id, (Pon2Status == TRUE ? "true" : "false")); 
	if(( Pon1Status == TRUE ) && ( Pon2Status != TRUE ))
	{
        return 0;
	}
    
	if(( Pon1Status != TRUE ) && ( Pon2Status == TRUE ))
	{
        *olt_id = slave_id;
        *partner_id = master_id;

        return 0;
	}

	OLT_SWITCH_DEBUG("\r\nHotSwapSeletctMaster: single local: master %d %s, slave %d %s\r\n", 
			master_id, (OLT_ISLOCAL(master_id) ? "local" : "remote"), slave_id, (OLT_ISLOCAL(slave_id) ? "local" : "remote")); 

    if ( OLT_ISLOCAL(master_id) )
    {
    	Pon1Status = PonPortHotStatusQuery(master_id);
    	if( Pon1Status == V2R1_PON_PORT_SWAP_ACTIVE )
    	{
            return 0;
    	}
    }

    if ( OLT_ISLOCAL(slave_id) )
    {
    	Pon2Status = PonPortHotStatusQuery(slave_id);
    	if( Pon2Status == V2R1_PON_PORT_SWAP_ACTIVE )
    	{
            *olt_id = slave_id;
            *partner_id = master_id;

            return 0;
    	}
    }

	slot1 = GetCardIdxByPonChip( master_id );
	slot2 = GetCardIdxByPonChip( slave_id );

	OLT_SWITCH_DEBUG("\r\nHotSwapSeletctMaster: different slot: master %d slot %d, slave %d slot %d\r\n", 
			master_id, slot1, slave_id, slot2); 

	if( slot1 >  slot2 ) 
	{
        *olt_id = slave_id;
        *partner_id = master_id;

        return 0;
	}
	else if (slot1 <  slot2 )
	{
        return 0;
	}
	
	OLT_SWITCH_DEBUG("\r\nHotSwapSeletctMaster: same slot: master %d, slave %d\r\n", 
			master_id, slave_id); 

	if( master_id < slave_id ) 
	{
        return 0;
	}
	else
	{
        *olt_id = slave_id;
        *partner_id = master_id;

        return 0;
	}

    return -1;
}

int OLTAdv_SetHotSwapMode(short int olt_id, short int partner_olt_id, int swap_mode, int swap_status, int sync_mode)
{
    int iRlt = 0;
    int swap_mode_supported;
    int swap_flags = OLT_SWAP_FLAGS_NONE;
    int swap_cap[2];
    short int master_onunum, slave_onunum;
    short int master_id, slave_id;
    short int local_id, remote_id;

    OLT_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);
    
	OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"OLTAdv_SetHotSwapMode olt_id:%d, partner_olt_id:%d, swap_mode:%d, swap_status:%d, sync_mode:%d.\r\n", olt_id, partner_olt_id, swap_mode,swap_status, sync_mode);

    if ( (V2R1_PON_PORT_SWAP_AUTO == swap_mode)
        || (V2R1_PON_PORT_SWAP_SLOWLY < swap_mode) )
    {
		if ( 0 == (iRlt = OLT_GetHotSwapCapability(olt_id, &swap_cap[0])) )
        {
            if ( 0 == (iRlt = OLT_GetHotSwapCapability(partner_olt_id, &swap_cap[1])) )
            {
                /* 指定倒换模式，判断双方是否都支持 */
                if ( (swap_mode > swap_cap[0])
                    || (swap_mode > swap_cap[1]) )
                {
                    return OLT_ERR_NOTSUPPORT;
                }
                else
                {
                    if ( V2R1_PON_PORT_SWAP_AUTO == swap_mode )
                    {
						/* modified by wangjiah@2017-05-09
						 * set default swap mode as SLOWLY
                        swap_mode = MIN(swap_cap[0], swap_cap[1]);

#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
                        swap_mode_supported = V2R1_PON_PORT_SWAP_QUICKLY;
#else
                        swap_mode_supported = V2R1_PON_PORT_SWAP_SLOWLY;
#endif
                        swap_mode = MIN(swap_mode, swap_mode_supported);
						*/
						swap_mode = V2R1_PON_PORT_SWAP_SLOWLY;
                    }
                    else
                    {
#if 1
                        if ( OLT_ISNET(olt_id) || OLT_ISNET(partner_olt_id) )
                        {
                            /* 目前，设备间的倒换，只支持慢倒换 */
                            return OLT_ERR_NOTSUPPORT;
                        }
#endif
                    }
                }
            }
        }

        if ( OLT_ERR_NOTEXIST == iRlt )
        {
            /*for onu swap by jinhl@2013-02-22*/
			/*如果有一个olt启动另外一个未启动，iRlt为OLT_ERR_NOTEXIST但需要继续执行下去*/
            if ( (V2R1_PON_PORT_SWAP_SLOWLY >= swap_mode) || (V2R1_PON_PORT_SWAP_ONU == swap_mode) )
            {
                iRlt = 0;
            }
        }
    }

	/*added by wangjiah@2017-05-19*/
	if(V2R1_PON_PORT_SWAP_AUTO == swap_mode)
		swap_mode = V2R1_PON_PORT_SWAP_SLOWLY;

    if ( 0 == iRlt )
    {
        master_id = olt_id;
        slave_id  = partner_olt_id;

        master_onunum = -1;
        slave_onunum  = -1;
		/*for onu swap by jinhl@2013-02-22*/
		#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
        if(V2R1_PON_PORT_SWAP_ONU != swap_mode)
        #endif
		{
	        if ( V2R1_PON_PORT_SWAP_UNKNOWN == swap_status )
	        {
	            (void)OLTAdv_HotSwapSelectMaster(&master_id, &slave_id, &master_onunum, &slave_onunum);
	        }
	        else if ( V2R1_PON_PORT_SWAP_PASSIVE == swap_status )
	        {
	            master_id = partner_olt_id;
	            slave_id  = olt_id;
	        }
	        else
	        {}
		}

        if ( !PonPortIsWorking(slave_id) )
        {
            swap_flags |= OLT_SWAP_FLAGS_PARTNER_NOTWORKING;
        }
        
        if ( !PonPortIsWorking(master_id) )
        {
            swap_flags |= OLT_SWAP_FLAGS_OLT_NOTWORKING;
        }

        if ( swap_flags & OLT_SWAP_FLAGS_NOTALLWORKING )
        {
            /* 至少有一个PON口不可用 */
			/*for onu swap by jinhl@2013-02-22*/
			/*如果是只有一个olt存在，在onu保护模式下，不能降级*/
            if ( (swap_mode >= V2R1_PON_PORT_SWAP_QUICKLY) && (V2R1_PON_PORT_SWAP_ONU != swap_mode) )
            {
                /* 快倒换要求2个PON都必须在位,否则只保存倒换配置 */
                swap_mode = V2R1_PON_PORT_SWAP_AUTO;
            }
        }

        if ( OLT_ISNET(master_id) )
        {
            remote_id = master_id;
            local_id  = slave_id;
        }
        else if ( OLT_ISNET(slave_id) )
        {
            remote_id = slave_id;
            local_id  = master_id;
        }
        else
        {
            remote_id = OLT_ID_INVALID;
        }

#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
        if ( OLT_ID_INVALID != remote_id )
        {
#if 1
            if ( (V2R1_PON_PORT_SWAP_SLOWLY != swap_mode)
                 && (V2R1_PON_PORT_SWAP_DISABLED != swap_mode) )
            {
                /* 目前，仅支持设备间慢倒换 */
                swap_mode = V2R1_PON_PORT_SWAP_SLOWLY;
            }
#endif
        
            /* 设备间的倒换，需要逻辑端口双向激活 */
            /* 判断逻辑端口是否双向激活 */
            if ( TRUE != OLTRM_RemoteOltIsValid(remote_id) )
            {
                /* 单向激活的话，只本地设置 */
                master_id = local_id;
                slave_id  = remote_id;
                swap_flags |= OLT_SWAP_FLAGS_OLT_SINGLESETTING;
            }
        }
#endif

		/* added by wangjiah@2017-06-13 to solve issue 38388
		 * DON'T operate partner pon port when its working status is not PONPORT_UP
		 * to prevent onu register before pon port actived.
		 * */
		if(OLT_ISLOCAL(olt_id) && OLT_ISLOCAL(partner_olt_id) 
				&& PONPORT_UP != GetPonPortWorkStatus(partner_olt_id))
		{
			swap_flags |= OLT_SWAP_FLAGS_OLT_SINGLESETTING; 
			iRlt = OLT_SetHotSwapMode(olt_id, partner_olt_id, swap_mode, (master_id == olt_id ? V2R1_PON_PORT_SWAP_ACTIVE : V2R1_PON_PORT_SWAP_PASSIVE), swap_flags);
		}
		else
		{
			iRlt = OLT_SetHotSwapMode(master_id, slave_id, swap_mode, V2R1_PON_PORT_SWAP_ACTIVE, swap_flags);
		}
        OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"M_pon%d<-(%c)->S_pon%d %s-setup(for user config) swap_flags(%d) result(%d).\r\n", master_id, (V2R1_PON_PORT_SWAP_SLOWLY == swap_mode) ? 's' : 'q', slave_id, (V2R1_PON_PORT_SWAP_AUTO == swap_mode) ? "mem" : "hard", swap_flags, iRlt);
        if ( 0 == iRlt )
        {
#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
            if ( OLT_ID_INVALID != remote_id )
            {
                if ( V2R1_PON_PORT_SWAP_DISABLED == swap_mode )
                {
                    OLTRM_ClrRemoteOltPartner(remote_id);
                }
                else
                {
                    OLTRM_SetRemoteOltPartner(remote_id, local_id);

                    if ( !(swap_flags & OLT_SWAP_FLAGS_OLT_SINGLESETTING) )
                    {
                        /* 只有远端OLT可以接受配置, 才需同步配置 */
                        remote_id = OLT_ID_INVALID;
                    }
                }
            }
#endif

            if ( (V2R1_PON_PORT_SWAP_SYNC == sync_mode)
                && (V2R1_PON_PORT_SWAP_DISABLED != swap_mode)
                && (V2R1_PON_PORT_SWAP_ONU != swap_mode)/*for onu swap by jinhl@2013-02-22*/
                && (OLT_ID_INVALID == remote_id) )
            {
                if ( SYS_LOCAL_MODULE_RUNNINGSTATE == MODULE_RUNNING )
                {
                    if ( swap_mode >= V2R1_PON_PORT_SWAP_QUICKLY )
                    {
                        sync_mode = OLT_COPYFLAGS_SYNC | OLT_COPYFLAGS_WITHLLID | OLT_COPYFLAGS_WITHINFO;
                    }
                    else
                    {
                        sync_mode = OLT_COPYFLAGS_SYNC | OLT_COPYFLAGS_WITHINFO;
                    }
                    
                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"M_pon%d-(sync[%d])->S_pon%d first-config-sync: <---begin\r\n", master_id, sync_mode, slave_id);
                    CopyAllConfigFromPon1ToPon2(master_id, slave_id, sync_mode);
                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"M_pon%d-(sync[%d])->S_pon%d first-config-sync: end--->\r\n", master_id, sync_mode, slave_id);
                }
            }
			/*for onu swap by jinhl@2013-02-22*/
			/*onu倒换模式，两个olt都要发光*/
            #if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
			if(V2R1_PON_PORT_SWAP_ONU == swap_mode)
			{
			    /*if(PonPortIsWorking(slave_id))*/
		     	{
				     OLTAdv_SetOpticalTxMode2(slave_id, TRUE, PONPORT_TX_SWITCH);
		     	}
				
				/*if(PonPortIsWorking(master_id))*/
		     	{
				     OLTAdv_SetOpticalTxMode2(master_id, TRUE, PONPORT_TX_SWITCH);
		     	}
				
			}
			
			#endif
        }
    }
    
    return iRlt;
}

int OLTAdv_GetHotSwapMode(short int olt_id, short int *partner_olt_id, int *swap_mode, int *swap_status)
{
    int iRlt;

    if ( OLT_ISLOCAL(olt_id) )
    {
        iRlt = GW_GetHotSwapMode(olt_id, partner_olt_id, swap_mode, swap_status);
    }
    else
    {
#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
        if ( 0 == (iRlt = OLTRM_GetRemoteOltPartner(olt_id, partner_olt_id)) )
        {
            olt_id = *partner_olt_id;
            OLT_LOCAL_ASSERT(olt_id);

            if ( 0 == (iRlt = GW_GetHotSwapMode(olt_id, &olt_id, swap_mode, swap_status)) )
            {
                *swap_status = OLT_SWAP_STATUS_SWITCHOVER(*swap_status);
            }
        }
#else        
        iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }
    
    return iRlt;
}

int OLTAdv_RdnIsExist(short int olt_id)
{
    bool bExist = 0;

    (void)OLT_RdnIsExist(olt_id, &bExist);
    
    return (int)bExist; 
}
/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
int OLTAdv_PLATO3_SetLLIDSLA(short int olt_id, short int llid, PLATO3_SLA_t *SLA, short int *dba_error)
{
    int iRlt;
    LLID_SLA_INFO_t sla_info;

    VOS_ASSERT(SLA);
    VOS_ASSERT(dba_error);

    sla_info.DBA_ErrCode = *dba_error;
    VOS_MemCpy(&sla_info.SLA.SLA3, SLA, sizeof(PLATO3_SLA_t));

    if ( 0 == (iRlt = OLT_SetLLIDSLA(olt_id, llid, &sla_info)) )
    {
        *dba_error = sla_info.DBA_ErrCode;
    }
    
    return iRlt; 
}

int OLTAdv_PLATO3_GetLLIDSLA(short int olt_id, short int llid, PLATO3_SLA_t *SLA, short int *dba_error)
{
    int iRlt;
    LLID_SLA_INFO_t sla_info;

    VOS_ASSERT(SLA);
    VOS_ASSERT(dba_error);
    
    if ( 0 == (iRlt = OLT_GetLLIDSLA(olt_id, llid, &sla_info)) )
    {
        *dba_error = sla_info.DBA_ErrCode;
        VOS_MemCpy(SLA, &sla_info.SLA.SLA3, sizeof(PLATO3_SLA_t));
    }
    
    return iRlt; 
}

int OLTAdv_PLATO4_SetLLIDSLA(short int olt_id, short int llid, PLATO4_SLA_t *SLA, short int *dba_error)
{
    int iRlt;
    LLID_SLA_INFO_t sla_info;

    VOS_ASSERT(SLA);
    VOS_ASSERT(dba_error);

    sla_info.DBA_ErrCode = *dba_error;
    VOS_MemCpy(&sla_info.SLA.SLA4, SLA, sizeof(PLATO4_SLA_t));

    if ( 0 == (iRlt = OLT_SetLLIDSLA(olt_id, llid, &sla_info)) )
    {
        *dba_error = sla_info.DBA_ErrCode;
    }
    
    return iRlt; 
}

int OLTAdv_PLATO4_GetLLIDSLA(short int olt_id, short int llid, PLATO4_SLA_t *SLA, short int *dba_error)
{
    int iRlt;
    LLID_SLA_INFO_t sla_info;

    VOS_ASSERT(SLA);
    VOS_ASSERT(dba_error);
    
    if ( 0 == (iRlt = OLT_GetLLIDSLA(olt_id, llid, &sla_info)) )
    {
        *dba_error = sla_info.DBA_ErrCode;
        VOS_MemCpy(SLA, &sla_info.SLA.SLA4, sizeof(PLATO4_SLA_t));
    }
    
    return iRlt; 
}

int OLTAdv_GetLLIDSLA(short int olt_id, short int llid, PLATO3_SLA_t *SLA, short int *dba_error)
{
    int iRlt;
    LLID_SLA_INFO_t sla_info;

    VOS_ASSERT(SLA);
    VOS_ASSERT(dba_error);
    
    if ( 0 == (iRlt = OLT_GetLLIDSLA(olt_id, llid, &sla_info)) )
    {
        *dba_error = sla_info.DBA_ErrCode;
        VOS_MemCpy(SLA, &sla_info.SLA.SLA3, sizeof(PLATO3_SLA_t));
    }
    
    return iRlt; 
}

int OLTAdv_SetLLIDSLA(short int olt_id, short int llid, PLATO3_SLA_t *SLA, short int *dba_error)
{
    int iRlt;
    LLID_SLA_INFO_t sla_info;

    VOS_ASSERT(SLA);
    VOS_ASSERT(dba_error);

    sla_info.DBA_ErrCode = *dba_error;
    VOS_MemCpy(&sla_info.SLA.SLA3, SLA, sizeof(PLATO3_SLA_t));

    if ( 0 == (iRlt = OLT_SetLLIDSLA(olt_id, llid, &sla_info)) )
    {
        *dba_error = sla_info.DBA_ErrCode;
    }
    
    return iRlt; 
}

/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/

int OLTAdv_SetLLIDPolice(short int olt_id, short int llid, int police_path, bool enable, PON_policing_parameters_t police_params)
{
    LLID_POLICE_INFO_t police_info;

    police_info.path = police_path;
    police_info.enable = enable;
    VOS_MemCpy(&police_info.params, &police_params, sizeof(PON_policing_parameters_t));

    return OLT_SetLLIDPolice(olt_id, llid, &police_info); 
}

int OLTAdv_GetLLIDPolice(short int olt_id, short int llid, int police_path, bool *enable, PON_policing_parameters_t *police_params)
{
    int iRlt;
    LLID_POLICE_INFO_t police_info;

    police_info.path = police_path;
    if ( 0 == (iRlt = OLT_GetLLIDPolice(olt_id, llid, &police_info)) )
    {
        *enable = police_info.enable;
        VOS_MemCpy(police_params, &police_info.params, sizeof(PON_policing_parameters_t));
    }
    
    return iRlt; 
}

int OLTAdv_LLIDIsExist(short int olt_id, short int llid)
{
    bool bExist = 0;

    (void)OLT_LLIDIsExist(olt_id, llid, &bExist);
    
    return (int)bExist; 
}

int OLTAdv_LLIDIsOnline(short int olt_id, short int llid)
{
    int iRlt;

    if ( PON_ONU_MODE_OFF != (iRlt = OLT_GetOnuMode(olt_id, llid)) )
    {
        return TRUE;
    }
    
    return FALSE; 
}

int OLTAdv_ForceHotSwap(short int new_master_olt, short int new_slave_olt)
{
    int iRlt;

    if ( OLTAdv_RdnIsReady(new_master_olt) )
    {
        /* 通过参数换位，来确保MasterOlt先切换  */
        if ( OLTAdv_RdnIsReady(new_slave_olt) )
        {
            iRlt = OLT_ForceHotSwap(new_slave_olt, new_master_olt, V2R1_PON_PORT_SWAP_PASSIVE, OLT_SWAP_FLAGS_NONE);
        }
        else
        {
            iRlt = OLT_ERR_NOTEXIST;
        }
    }
    else
    {
        iRlt = OLT_ERR_NOTEXIST;
    }

    OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"M_pon%d<-(?)->S_pon%d switch(for user's force) to M_pon%d<-(?)->S_pon%d result(%d).\r\n", new_slave_olt, new_master_olt, new_master_olt, new_slave_olt, iRlt);

    return iRlt;
}

int OLTAdv_RdnIsReady(short int olt_id)
{
    int iIsReady = FALSE;

    (void)OLT_RdnIsReady(olt_id, &iIsReady);

    return iIsReady;
}

int OLTAdv_NotifyOltInvalid(short int olt_id)
{
#if ( EPON_MODULE_PON_REMOTE_MANAGE == EPON_MODULE_YES )
    if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
    {
        devsm_local_port_notifyinvalid(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id));
    }
#endif 

    return 0;    
}

int OLTAdv_NotifyOltValid(short int olt_id)
{
#if ( EPON_MODULE_PON_REMOTE_MANAGE == EPON_MODULE_YES )
    if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
    {
        devsm_local_port_notifyvalid(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id));
    }
#endif 

    return 0;    
}

/*Begin:for onu swap by jinhl@2013-04-27*/
int OLTAdv_SearchFreeOnuIdx(short int olt_id, unsigned char *MacAddress, short int *reg_flag)
{
    int iRlt = 0;
	if(OLT_ISLOCAL(olt_id))
	{
	    iRlt = GW_SearchFreeOnuIdx(olt_id, MacAddress, reg_flag);
	}
	else
	{
	    iRlt = OLT_SearchFreeOnuIdx(olt_id, MacAddress, reg_flag);
	}

	return iRlt;
}

int OLTAdv_GetActOnuIdxByMac(short int olt_id, unsigned char *MacAddress)
{
    int iRlt = 0;
    if(OLT_ISLOCAL(olt_id))
	{
	    iRlt = GW_GetActOnuIdxByMac(olt_id, MacAddress);
	}
	else
	{
	    iRlt = OLT_GetActOnuIdxByMac(olt_id, MacAddress);
	}

	return iRlt;
    
}
/*End:for onu swap by jinhl@2013-04-27*/
/*---------------------------------------------------*/



#ifdef __cplusplus

}

#endif

