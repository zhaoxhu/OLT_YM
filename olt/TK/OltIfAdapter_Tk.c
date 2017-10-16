/***************************************************************
*
*						Module Name:  OltIfAdapter_TK.c
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
*   Date: 			2012/10/08
*   Author:		liwei056
*   content:
**  History:
**   Date        |    Name       |     Description
**---- ----- |-----------|------------------ 
**  12/10/08  |   liwei056    |     create 
**----------|-----------|------------------
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif

#include "syscfg.h"
#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_task.h"
#include "vos/vospubh/vos_que.h"
#include "vos/vospubh/cdp_syn.h"
#include "vos_global.h"
#include "vos/vospubh/vos_types.h"
#include "vos/vospubh/vos_string.h"
#include "vos/vospubh/vos_sem.h"

#include  "OltGeneral.h"
#include  "OnuGeneral.h"
#include  "PonGeneral.h"
#include  "includefromPas.h"
#include  "includefromTk.h"
#include  "includefromCmc.h"
#include  "ponEventHandler.h"

#include  "../../monitor/monitor.h"

extern char *g_cszPonChipTypeNames[PONCHIP_TYPE_MAX];    
extern long double g_afLevelConst[16];
extern ponAlarmInfo		**gpOnuAlarm;
extern ponThreasholdInfo gPonThreashold;

extern  int OnuMacCheckEnable(ULONG  enable);

/*-----------------------------内部适配----------------------------------------*/

static int TK_OK(short int olt_id)
{
    OLT_TK_DEBUG(OLT_TK_TITLE"TK_OK(%d)'s result(0).\r\n", olt_id);

    return OLT_ERR_OK;
}

static int TK_ERROR(short int olt_id)
{
    OLT_TK_DEBUG(OLT_TK_TITLE"TK_ERROR(%d)'s result(%d).\r\n", olt_id, OLT_ERR_NOTEXIST);

    debug_stub();

    VOS_ASSERT(0);

    return OLT_ERR_NOTEXIST;
}

#if 1
/* -------------------OLT基本API------------------- */

static int TK_IsExist(short int olt_id, bool *status)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(status);
    
    *status = OltIsExist(olt_id);
    return OLT_ERR_OK;
}

static int TK3723_GetChipTypeID(short int olt_id, int *type)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(type);

    *type = PONCHIP_TK3723;
    return OLT_ERR_OK;
}

static int TK3723_GetChipTypeName(short int olt_id, char typename[OLT_CHIP_NAMELEN])
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(typename);
    VOS_ASSERT(g_cszPonChipTypeNames[PONCHIP_TK3723]);

    VOS_MemZero(typename, OLT_CHIP_NAMELEN);
    VOS_StrnCpy(typename, g_cszPonChipTypeNames[PONCHIP_TK3723], OLT_CHIP_NAMELEN - 1);
    return OLT_ERR_OK;
}

static int BCM55524_GetChipTypeID(short int olt_id, int *type)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(type);

    *type = PONCHIP_BCM55524;
    return OLT_ERR_OK;
}

static int BCM55524_GetChipTypeName(short int olt_id, char typename[OLT_CHIP_NAMELEN])
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(typename);
    VOS_ASSERT(g_cszPonChipTypeNames[PONCHIP_BCM55524]);

    VOS_MemZero(typename, OLT_CHIP_NAMELEN );
    VOS_StrnCpy(typename, g_cszPonChipTypeNames[PONCHIP_BCM55524], OLT_CHIP_NAMELEN - 1);
    return OLT_ERR_OK;
}

static int TK3723_ResetPon(short int olt_id)
{
    OLT_LOCAL_ASSERT(olt_id);

    TK_remove_olt(olt_id, FALSE, FALSE);
    GW_ResetPon(olt_id);

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ResetPon(%d)'s result(%d) on slot %d.\r\n", olt_id, 0, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

static int TK3723_RemoveOlt(short int olt_id, bool send_shutdown_msg_to_olt, bool reset_olt)
{
    OLT_LOCAL_ASSERT(olt_id);

    TK_remove_olt(olt_id, send_shutdown_msg_to_olt, reset_olt);	

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_RemoveOlt(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, send_shutdown_msg_to_olt, reset_olt, 0, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

int TK_SetInitParams(short int olt_id, unsigned short host_olt_manage_type, unsigned short host_olt_manage_address)
{
    int iRlt;

    OLT_LOCALID_ASSERT(olt_id);
    
    if ( PON_MODULE_STATE_NOT_INIT == Get_tk_soft_state() )
    {
        switch (host_olt_manage_type)
        {
            case V2R1_PON_HOST_MANAGE_BY_BUS:
                if ( 0 != host_olt_manage_address )
                {
                    iRlt = Tk_set_olt_base_address(host_olt_manage_address);
                }
                else
                {
                    iRlt = 0;
                }
                
                break;
            case V2R1_PON_HOST_MANAGE_BY_ETH:
                iRlt = OLT_ERR_NOTSUPPORT;
                break;
            case V2R1_PON_HOST_MANAGE_BY_URT:
            default:    
                VOS_ASSERT(0);
                iRlt = OLT_ERR_NOTSUPPORT;
                break;
        }
    }
    else
    {
        iRlt = OLT_ERR_PARAM;
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"TK_SetInitParams(%d, %d, 0x%x)'s result(%d) on slot %d.\r\n", olt_id, host_olt_manage_type, host_olt_manage_address, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

int TK_SetSystemParams(short int olt_id, long int statistics_sampling_cycle, long int monitoring_cycle, short int host_olt_msg_timeout, short int olt_reset_timeout)
{
    int iRlt;

    OLT_LOCALID_ASSERT(olt_id);

    if ( PON_MODULE_STATE_RUNNING == Get_tk_soft_state() )
    {
        iRlt = OLT_ERR_NOTSUPPORT;
    }
    else
    {
        iRlt = 0;
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"TK_SetSystemParams(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, statistics_sampling_cycle, monitoring_cycle, host_olt_msg_timeout, olt_reset_timeout, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int TK3723_WriteMdioRegister(short int olt_id, short int phy_address, short int reg_address, unsigned short int value )
{
    int iRlt;
    short int port_id;
        
    OLT_LOCAL_ASSERT(olt_id);

    port_id = (short int)TkChip_GetOltPonPortID(olt_id);   
	iRlt = TkOLT_SetPortMdio(olt_id, port_id, (unsigned short)reg_address, value);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_WriteMdioRegister(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, phy_address, reg_address, value, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int TK3723_ReadMdioRegister(short int olt_id, short int phy_address, short int reg_address, unsigned short int *value )
{
    int iRlt;
    short int port_id;
        
    OLT_LOCAL_ASSERT(olt_id);

    port_id = (short int)TkChip_GetOltPonPortID(olt_id);   
	iRlt = TkOLT_GetPortMdio(olt_id, port_id, (unsigned short)reg_address, value);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ReadMdioRegister(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, phy_address, reg_address, *value, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int TK3723_ReadI2CRegister(short int olt_id, short int device, short int register_address, short int *data )
{
    static unsigned char aucSFF8472Addrs[2] = {0xA0, 0xA2};
    int iRlt;
    unsigned short numBytesToRead;
    unsigned char ucBusId;
    unsigned char ucRegAddr;
    unsigned char ucRegVal;
        
    OLT_LOCAL_ASSERT(olt_id);

    if ( (device >= 0) && (device < 2) )
    {
        ucBusId = (unsigned char)TkChip_GetOltPonPortID(olt_id);
        ucRegAddr = (unsigned char)register_address;
        ucRegVal = 0;
        numBytesToRead = 1;
    	iRlt = TkOLT_GetI2cDataV1(olt_id, ucBusId, 0, aucSFF8472Addrs[device], ucRegAddr, &numBytesToRead, &ucRegVal);
        *data = ucRegVal;
    }
    else
    {
        iRlt = OLT_ERR_PARAM;
    }

#if 0
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ReadI2CRegister(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, device, register_address, *data, iRlt, SYS_LOCAL_MODULE_SLOTNO);
#endif
	return iRlt;
}

static int TK3723_GetGpioFuncId(short int olt_id, int func_id, int gpio_dir)
{
    int line_number = OLT_ERR_PARAM;

    if ( PON_GPIO_LINE_INPUT == gpio_dir )
    {
        switch ( func_id )
        {
            case OLT_GPIO_PON_LOSS:
                line_number = TkOltGpioPurposeUnused;
                break;
            case OLT_GPIO_SFP_LOSS:
                if ( 0 == TkChip_GetOltPonPortID(olt_id) )
                {
                    line_number = TkOltGpioPurposeUserInput0;
                }
                else
                {
                    line_number = TkOltGpioPurposeUserInput1;
                }
                
                break;
            default:
                NULL;
        }
    }
    else
    {
        switch ( func_id )
        {
            case OLT_GPIO_LED_RUN:
                if ( 0 == TkChip_GetCommonHostID(olt_id) )
                {
                    line_number = TkOltGpioPurposeLedOltReady;
                }
                else
                {
                    line_number = TkOltGpioPurposeUnused;
                }
                break;
            default:
                NULL;
        }
    }

    return line_number;
}

static int TK3723_ReadGpio(short int olt_id, int func_id, bool *value)
{
    int iRlt;
   
	OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(value);

    if ( 0 < (iRlt = TK3723_GetGpioFuncId(olt_id, func_id, PON_GPIO_LINE_INPUT)) )
    {
    	iRlt = TkOLT_GetGpioState(olt_id, iRlt, value);
    }

#if 0
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ReadGpio(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, func_id, *value, iRlt, SYS_LOCAL_MODULE_SLOTNO);
#endif
	return iRlt;
}

static int TK3723_WriteGpio(short int olt_id, int func_id, bool value)
{
    int iRlt;
   
	OLT_ASSERT(olt_id);

    if ( 0 < (iRlt = TK3723_GetGpioFuncId(olt_id, func_id, PON_GPIO_LINE_OUTPUT)) )
    {
    	iRlt = TkOLT_SetGpioState(olt_id, iRlt, value);
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_WriteGpio(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, func_id, value, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int TK3723_SendChipCli(short int olt_id, unsigned short size, unsigned char *command)
{
    int iRlt;
    unsigned char cli_enabled;
   
    OLT_ASSERT(olt_id);

    if ( 0 == (iRlt = TkOLT_IsCliHost(olt_id, &cli_enabled)) )
    {
        if ( cli_enabled )
        {
           	iRlt = TkOLT_SendHostCli(olt_id, size, command);
        }
        else
        {
            cli_enabled = TRUE;
            if ( 0 == (iRlt = TkOLT_SetCliHost(olt_id, cli_enabled)) )
            {
               	iRlt = TkOLT_SendHostCli(olt_id, size, command);
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SendChipCli(%d, %d, %s)'s result(%d) on slot %d.\r\n", olt_id, size, command, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}
#endif


#if 1
/* -------------------OLT PON管理API--------------- */

static int TK3723_GetVersion(short int olt_id, PON_device_versions_t *device_versions)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(device_versions);
    
	iRlt = TkAdapter_PAS5201_GetOltChipVersion(olt_id, device_versions);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetVersion(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int TK3723_GetDBAVersion(short int olt_id, OLT_DBA_version_t *dba_version)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(dba_version);
    
	iRlt = TkAdapter_PAS5201_GetDBAVersion( olt_id, dba_version );
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetDBAVersion(%d, %s, %s)'s result(%d) on slot %d.\r\n", olt_id, dba_version->szDBAname, dba_version->szDBAversion, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_ChkVersion(short int olt_id, bool *is_compatibled)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(is_compatibled);
    
	iRlt = TkSoftIsCompatibledWithFirmware(olt_id, is_compatibled);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ChkVersion(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *is_compatibled, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int TK3723_SetAdminStatus(short int olt_id, int admin_status)
{
    int iRlt;
    int tx_mode;
        
    OLT_LOCAL_ASSERT(olt_id);

    tx_mode = (V2R1_ENABLE == admin_status) ? TRUE : FALSE;
    if ( 0 < (iRlt = GW_SetOpticalTxMode2(olt_id, tx_mode, PONPORT_TX_SHUTDOWN)) )
    {
    	/*for onu swap by jinhl@2013-04-27*/
    	/*这里，原双pon倒换时shutdown，未采取处理，后需要对此处理
    	而onu保护倒换，直接使其不发光即可，效果等同于干路断*/
    	if( (PonPortSwapEnableQuery( olt_id ) == V2R1_PON_PORT_SWAP_ENABLE ) &&
    		(V2R1_PON_PORT_SWAP_ONU != GetPonPortHotSwapMode(olt_id)))
    	{
    	    iRlt = 0;
    	}
    	else
        {
            if ( tx_mode )
            {
        	    iRlt = TkOltOpenDataPath(olt_id);
            }
            else
            {
        	    iRlt = TkOltCloseDataPath(olt_id);
            }

            if ( 0 == iRlt )
            {
                (void)TkOltSetOpticalTxMode(olt_id, tx_mode);
            }
        }   
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetAdminStatus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, admin_status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_GetCniLinkStatus(short int olt_id, bool *status)
{
    int iRlt;
    short int port_id;
    TkPortInfo stPortInfo;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(status);

    port_id = (short int)TkChip_GetOltEthPortID(olt_id);   
    if ( 0 == (iRlt = TkOLT_GetPort(olt_id, port_id, &stPortInfo)) )
    {
        *status = stPortInfo.linked;
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetCniLinkStatus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int TK3723_SetVlanTpid(short int olt_id, unsigned short int vlan_tpid)
{
    int iRlt;
    TkOltAdditionalVlanEthertypes types;
        
    OLT_LOCAL_ASSERT(olt_id);

    if ( 0 == (iRlt = TkOLT_GetVlanExtType(olt_id, &types)) )
    {
        if ( types.cvlanEtherType != vlan_tpid )
        {
            types.cvlanEtherType = vlan_tpid;
            iRlt = TkOLT_SetVlanExtType(olt_id, &types);
        }
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetVlanTpid(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, vlan_tpid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetVlanQinQ(short int olt_id, OLT_vlan_qinq_t *vlan_qinq_config)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(vlan_qinq_config);

    if ( OLT_CFG_DIR_UPLINK == vlan_qinq_config->qinq_direction )
    {
        short int llid;
        short int onu_id, onuid;
        PON_olt_vlan_uplink_config_t *pQinQCfg;

        onu_id   = vlan_qinq_config->qinq_objectid;    
        pQinQCfg = &(vlan_qinq_config->qinq_cfg.up_cfg);
        
    	llid = GetLlidActivatedByOnuIdx(olt_id, onu_id);
    	if( llid != INVALID_LLID )
    	{
            if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
            {
        		iRlt = TkAdapter_PAS5201_SetOnuVlanUplinkMode(olt_id, onuid, llid, pQinQCfg);
            }
            else
            {
                /* ONU不在线，则认为其空配置成功 */
                iRlt = 0;
            }
    	}
        else
        {
            /* ONU不在线，则认为其空配置成功 */
            iRlt = 0;
        }
        OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetVlanQinQ(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, pQinQCfg->vlan_manipulation, pQinQCfg->authenticated_vid, pQinQCfg->new_vlan_tag_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    }
    else
    {
        short int vlan_id;
        PON_olt_vid_downlink_config_t *pQinQCfg;

        vlan_id  = vlan_qinq_config->qinq_objectid;    
        pQinQCfg = &(vlan_qinq_config->qinq_cfg.down_cfg);

        iRlt = TkAdapter_PAS5201_SetVlanDownlinkMode(olt_id, vlan_id, pQinQCfg);
        OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetVlanQinQ(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, pQinQCfg->vlan_manipulation, vlan_id, pQinQCfg->new_vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    }

    return iRlt;
}

static int TK3723_GetPonFrameSizeLimit(short int olt_id, short int *jumbo_length)
{
	OLT_LOCAL_ASSERT(olt_id);
	VOS_ASSERT(jumbo_length);

	*jumbo_length = 1536; /*note: the PonFrameSize is determind by firmware,this value(1536) is false/add by yangzl@2016-2-17*/
	return OLT_ERR_OK;
}
static int TK3723_OamIsLimit(short int olt_id, bool *oam_limit)
{
    int iRlt;
    TkDefaultOamRate oam_rates;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(oam_limit);

    if ( 0 == (iRlt = TkOLT_GetOamRate(olt_id, &oam_rates)) )
    {
        if ( TK_OAM_RATE_MAX == oam_rates.maxRate )
        {
            *oam_limit = FALSE;
        }
        else
        {
            *oam_limit = TRUE;
        }
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_OamIsLimit(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *oam_limit, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_UpdatePonParams(short int olt_id, int max_range, int mac_agetime)
{
    int iRlt;
#if 0    
    int i;
    int aiMapRange2rtt[4][2] = {
                                {PON_RANGE_CLOSE, V2R1_REGISTER_WINDOW_CLOSE},
                                {PON_RANGE_20KM, PON_MAX_RTT_40KM},
                                {PON_RANGE_40KM, PON_MAX_RTT_80KM},
                                {PON_RANGE_60KM, PON_MAX_RTT_120KM}
                                };
	PON_update_olt_parameters_t  pon_updated_parameters;
        
    OLT_LOCAL_ASSERT(olt_id);

	PON_EMPTY_UPDATE_OLT_PARAMETERS_STRUCT(pon_updated_parameters);
	VOS_MemCpy( &pon_updated_parameters, &PAS_updated_parameters_5201, sizeof(PON_update_olt_parameters_t ) );

	for(i=0; i<4; i++)
    {
        if (aiMapRange2rtt[i][0] == max_range)
        {
            pon_updated_parameters.max_rtt = aiMapRange2rtt[i][1];
        }
    }

    iRlt = PAS_update_olt_parameters(olt_id, &pon_updated_parameters);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_UpdatePonParams(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, max_range, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetIgmpAuthMode(short int olt_id, int auth_mode)
{
    int iRlt;
    PON_pon_network_traffic_direction_t pkt_dir;
        
    OLT_LOCAL_ASSERT(olt_id);

    if ( auth_mode < 0 )
    {
        auth_mode = -auth_mode;
    }

    if ( V2R1_ENABLE == auth_mode )
    {
        pkt_dir = PON_OLT_CLASSIFIER_DESTINATION_HOST;
    }
    else
    {
        pkt_dir = PON_OLT_CLASSIFIER_DESTINATION_DATA_PATH;
    }

    iRlt = TkAdapter_PAS5201_set_classification_rule ( olt_id , PON_DIRECTION_UPLINK, PON_OLT_CLASSIFICATION_IGMP,
    									NULL, pkt_dir );
    if (0 == iRlt)
    {
        iRlt = TkAdapter_PAS5201_set_classification_rule ( olt_id , PON_DIRECTION_UPLINK, PON_OLT_CLASSIFICATION_MLD,
        									NULL, pkt_dir );
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetIgmpAuthMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, auth_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SendFrame2PON(short int olt_id, short int llid, void *eth_frame, int frame_len)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(eth_frame);

    iRlt = TK_send_frame(olt_id, llid, eth_frame, (unsigned short)frame_len);
    /* OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SendFrame2PON(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, frame_len, iRlt, SYS_LOCAL_MODULE_SLOTNO); */

    return iRlt;
}

static int TK3723_SendFrame2CNI(short int olt_id, short int llid, void *eth_frame, int frame_len)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(eth_frame);

    iRlt = TK_send_frame(olt_id, llid, eth_frame, (unsigned short)frame_len);
    /* OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SendFrame2CNI(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, frame_len, iRlt, SYS_LOCAL_MODULE_SLOTNO); */

    return iRlt;
}

static int TK3723_DelVidDownlinkMode(short int olt_id, PON_vlan_tag_t vid)
{
    int iRlt;
    PON_olt_vid_downlink_config_t stQinQCfg;

	OLT_LOCAL_ASSERT(olt_id);

    VOS_MemZero(&stQinQCfg, sizeof(stQinQCfg));
    stQinQCfg.vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION;
	iRlt = TkAdapter_PAS5201_SetVlanDownlinkMode(olt_id, vid, &stQinQCfg);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"TK3723_DelVidDownlinkMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif


#if 1
/* -------------------OLT LLID 管理API-------------- */

static int TK3723_LLIDIsExist(short int olt_id, short int llid, bool *status)
{
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(status);
    
    *status = LinkIsOnline(olt_id, llid);
    return OLT_ERR_OK;
}

static int TK3723_DeregisterLLID(short int olt_id, short int llid, bool iswait)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    
    iRlt = TkOLT_DeregisterLLID(olt_id, llid);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_DeregisterLLID(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_GetLLIDMac(short int olt_id, short int llid, mac_address_t onu_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_mac);
    
    iRlt = TkGetLinkMacAddress(olt_id, llid, onu_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetLLIDMac(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_GetLLIDRegisterInfo(short int olt_id, short int llid, onu_registration_info_t *onu_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    unsigned char curr_cm_num;
    unsigned char pon_rate_flags = PON_RATE_NORMAL_1G;
    TkOltLinkInfo link_info;
		
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_info);

    if ( 0 == (iRlt = TkLinkGetOnuRegisterInfo ( olt_id, llid
                            , NULL, NULL, NULL, &link_info
                            , &onu_info->max_links_support
                            , &onu_info->curr_links_num
                            , &onu_info->max_cm_support
                            , &curr_cm_num
                            )) )
    {
        onu_info->oam_version = OAM_STANDARD_VERSION_3_3;
        onu_info->rtt = link_info.rtt;
        
        onu_info->laser_on_time  = 0;
        onu_info->laser_off_time = 0;

        onu_info->vendorType     = link_info.vendorType;
        onu_info->productVersion = link_info.productVersion;
        onu_info->productCode    = link_info.productCode;

        /* 需要OAM查出此LLID是否可工作于下行2G模式 */
        onu_info->pon_rate_flags = pon_rate_flags;
    }

	OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetLLIDRegisterInfo(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

static int TK3723_AuthorizeLLID(short int olt_id, short int llid, bool auth_mode)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(olt_id);

    if ( auth_mode )
    {
        iRlt = TK_authorize_llid(olt_id, llid, TRUE);
    }
    else
    {
        iRlt = TK_authorize_llid(olt_id, llid, FALSE);
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_AuthorizeLLID(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, auth_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int TK3723_SetLLIDSLA(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(SLA);

    if ( 5 == SLA->SLA_Ver )
    {
        if ( 0 == (iRlt = TkOLT_SetLinkSLA(olt_id, llid, &SLA->SLA.SLA5)) )
        {
            SLA->DBA_ErrCode = 0;
        }
    }
    else
    {
        iRlt = OLT_ERR_PARAM;
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetLLIDSLA(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int TK3723_GetLLIDSLA(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(SLA);

    if ( 0 == (iRlt = TkOLT_GetLinkSLA(olt_id, llid, &SLA->SLA.SLA5)) )
    {
        SLA->SLA_Ver = 5;
        SLA->DBA_ErrCode = 0;
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetLLIDSLA(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int TK3723_SetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police)
{
    int iRlt;
#if 0    
    PON_policing_parameters_t *policing_params;
    PON_policing_struct_t  policing_struct;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(police);

    policing_params = &police->params;
    policing_struct.maximum_bandwidth                = policing_params->maximum_bandwidth;
    policing_struct.maximum_burst_size               = policing_params->maximum_burst_size;
    policing_struct.high_priority_frames_preference  = policing_params->high_priority_frames_preference;
    policing_struct.short_frames_preference          = FALSE;

    iRlt = PAS_set_policing_parameters_v4(olt_id, llid, police->path, police->enable, &policing_struct);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetLLIDPolice(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int TK3723_GetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(police);

#if 0    
    iRlt = PAS_get_policing_parameters(olt_id, llid, police->path, &police->enable, &police->params);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetLLIDPolice(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int TK3723_SetLLIDdbaType(short int olt_id, short int llid, int dba_type, short int *dba_error)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(dba_error);

#if 0    
    iRlt = PLATO3_set_DBA_type(olt_id, llid, dba_type, dba_error);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetLLIDFecMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, dba_type, *dba_error, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_GetLLIDdbaType(short int olt_id, short int llid, int *dba_type, short int *dba_error)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(dba_type);
    VOS_ASSERT(dba_error);

#if 0    
    iRlt = PLATO3_get_DBA_type(olt_id, llid, dba_type, dba_error);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetLLIDdbaType(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, *dba_type, *dba_error, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_SetLLIDdbaFlags(short int olt_id, short int llid, unsigned short dba_flags)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

#if 0    
    iRlt = PLATO3_set_plato3_llid_flags(olt_id, llid, dba_flags);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetLLIDdbaFlags(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, dba_flags, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_GetLLIDdbaFlags(short int olt_id, short int llid, unsigned short *dba_flags)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(dba_flags);

#if 0    
    iRlt = PLATO3_get_plato3_llid_flags(olt_id, llid, dba_flags);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetLLIDdbaFlags(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, *dba_flags, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

#endif


#if 1
/* -------------------OLT ONU 管理API-------------- */

static int TK3723_GetAllOnus(short int olt_id, OLT_onu_table_t *onu_table)
{
    int iRlt;
    short int llid_num;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(onu_table);

    if ( 0 == (iRlt = TkAdapter_PAS5201_GetAllOnuParams(olt_id, &llid_num, onu_table->onus)) )
    {
        onu_table->onu_num = llid_num;
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetAllOnus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK_ResumeAllOnuStatus(short int olt_id, int olt_chipid, int resume_reason, int resume_mode)
{
    int iRlt;
    extern CTC_STACK_oui_version_record_t PAS_oui_version_records_list[];
    extern int PAS_oui_version_records_num;
    int OnuEntry, OnuIdx;
    int i;
    int resume_up_num, resume_dn_num, resume_er_num, resume_uk_num;
    short int checkin_num, onu_num;
    short int llid, ponIsOK;
    PAS_onu_parameters_t onu_list;

    ULONG status;
    UCHAR MacAddr[BYTES_IN_MAC_ADDRESS];
    UCHAR SequenceNo[PON_AUTHENTICATION_SEQUENCE_SIZE];
    OAM_standard_version_t OAM_Ver;
   
    OLT_LOCAL_ASSERT(olt_id);

    ponIsOK = 1;
    onu_num = -1;
    checkin_num = 0;
    
    resume_up_num = 0;
    resume_dn_num = 0;
    resume_er_num = 0;
    resume_uk_num = 0;

    OAM_Ver = OAM_STANDARD_VERSION_3_3;
    VOS_MemZero(SequenceNo, sizeof(SequenceNo));

#if 1    
    /* 恢复原PON口上的ONU在线状态信息及补报ONU日志 */
#ifdef OLT_RECOVERY_BY_EVENT
    if ( (OLT_RESUMEMODE_SYNCSOFT == resume_mode) && SYS_LOCAL_MODULE_ISMASTERACTIVE )
    {
        if ( 0 == TkAdapter_PAS5201_GetAllOnuParams(olt_id, &onu_num, onu_list) )
        {
            if ( onu_num > 0 )
            {
                if ( 0 < OLTAdv_GetOpticalTxMode(olt_id) )
                {
                    for (i=0; i<onu_num; ++i)
                    {
                        ++resume_uk_num;
                        llid = onu_list[i].llid;
                        
                        if ( 0 == TkLinkGetOnuRegisterInfo(olt_id, llid, NULL, NULL, MacAddr, NULL, NULL, NULL, NULL, NULL) )
                        {
                            ++resume_up_num;
                            OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate llid%d's register event for sync-soft.\r\n", olt_id, llid);

                            sendOnuRegistrationMsg( olt_id, llid, MacAddr /* onu_list[i].mac_address */, SequenceNo, OAM_Ver, ONU_EVENTFLAG_VIRTAL );
                            if ( IsSupportCTCOnu( olt_id ) == TRUE /* GetOnuVendorType( olt_id, OnuIdx ) == ONU_VENDOR_CT */ )
                            {
                                sendOnuExtOamDiscoveryMsg(olt_id, llid, CTC_DISCOVERY_STATE_COMPLETE, PAS_oui_version_records_num, PAS_oui_version_records_list, ONU_EVENTFLAG_VIRTAL);
                            }
                        }
                        else
                        {
                            ++resume_er_num;
                            OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d force llid%d re-register for sync-soft.\r\n", olt_id, llid);

                            TkOLT_DeregisterLLID( olt_id, llid );
                        }
                    }    

                    /* 注册状态这里已经恢复，后面只需同步离线状态 */
                    ponIsOK = 0;
                    onu_num = 0;
                }
                else
                {
                    /* 快倒换的备用口，不发光却有虚拟ONU注册，自然无在线ONU */
                    ponIsOK = 0;
                    onu_num = 0;
                }
            }
        }
    }
#endif       

    for(OnuIdx = 0, OnuEntry = olt_id*MAXONUPERPON; OnuIdx < MAXONUPERPON; OnuIdx++, OnuEntry++)
    {       
        if(ThisIsValidOnu(olt_id, OnuIdx) == ROK)
        {
            ONU_MGMT_SEM_TAKE;
            VOS_MemCpy( MacAddr, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS );
            llid   = OnuMgmtTable[OnuEntry].LLID;
        	status = OnuMgmtTable[OnuEntry].OperStatus;
            ONU_MGMT_SEM_GIVE;
			
            switch (resume_mode)
            {
                case OLT_RESUMEMODE_SYNCSOFT:
                    /*if( ONU_OPER_STATUS_UP == status )*/	/* modified by xieshl 20110713, 只要ONU MAC地址有效即同步，解决PON板上pending onu或离线onu不能同步问题 */
                    {
#ifdef BCM_WARM_BOOT_SUPPORT
						/*modified by liyang @2015-05-18 for warm restart*/
                        if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER ) 
#else
                        if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
#endif
                        {
                            if ( onu_num < 0 )
							{
								/* 得到在线ONU的MAC-LLID列表 */
								if ( 0 != TkAdapter_PAS5201_GetAllOnuParams(olt_id, &onu_num, onu_list) )
								{
									onu_num = 0;
								}
								/* B--added by liwei056@2011-12-15 for D14069 */
								else
								{
									if ( onu_num > 0 )
									{
										if ( 0 >= OLTAdv_GetOpticalTxMode(olt_id) )
										{
											/* 快倒换的备用口，不发光却有虚拟ONU注册，自然无在线ONU */
											ponIsOK = 0;
											onu_num = 0;
										}
									}
								}
								/* E--added by liwei056@2011-12-15 for D14069 */
							}

                            /* 通过LLID或MAC地址，确定此ONU是否在线 */
                            if ( onu_num > 0 )
							{
								/* 查此MAC是否在线 */
								for (i=0; i<onu_num; ++i)
								{
									if ( MAC_ADDR_IS_EQUAL(MacAddr, onu_list[i].mac_address) )
									{                                
										if ( llid != onu_list[i].llid )
										{
											llid = onu_list[i].llid;
											if ( 0 != OnuMgt_SetOnuLLID(olt_id, OnuIdx, llid) )
											{
												++resume_er_num;
												OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d force onu%d re-register for sync-soft.\r\n", olt_id, OnuIdx+1);

												TkOLT_DeregisterLLID( olt_id, llid );
												llid = INVALID_LLID;
											}
										}

										/* 标识此MAC已经找到 */
										++checkin_num;
										onu_list[i].rtt = -1;
										break;
									}
								}

								if ( (i >= onu_num) && (llid != INVALID_LLID) )
								{
									llid = INVALID_LLID;
									(void)OnuMgt_SetOnuLLID(olt_id, OnuIdx, llid);
								}
							}
                            else
                            {
                                if ( ponIsOK )
                                {
                                    /* 查此LLID是否在线 */
                                    llid = GetLlidActivatedByOnuIdx(olt_id, OnuIdx);
                                }
                                else
                                {
                                    /* 防止被PAS_SOFT的虚拟ONU欺骗 */
                                    llid = INVALID_LLID;
                                }
                            }
                           
                            if ( INVALID_LLID != llid )
                            {
#ifdef OLT_RECOVERY_BY_EVENT
                                if ( 0 != TkLinkGetOnuRegisterInfo(olt_id, llid, NULL, NULL, MacAddr, NULL, NULL, NULL, NULL, NULL) )
                                {
                                    ONU_MGMT_SEM_TAKE;
                                    VOS_StrCpy( SequenceNo, OnuMgmtTable[OnuEntry].SequenceNo );
                                    OAM_Ver = OnuMgmtTable[OnuEntry].OAM_Ver;
                                    ONU_MGMT_SEM_GIVE;
                                }
                                
                                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's register event for sync-soft.\r\n", olt_id, OnuIdx+1);

                                sendOnuRegistrationMsg( olt_id, llid, MacAddr, SequenceNo, OAM_Ver, ONU_EVENTFLAG_VIRTAL );
                                if ( IsSupportCTCOnu( olt_id ) == TRUE /* GetOnuVendorType( olt_id, OnuIdx ) == ONU_VENDOR_CT */ )
                                {
                                    sendOnuExtOamDiscoveryMsg(olt_id, llid, CTC_DISCOVERY_STATE_COMPLETE, PAS_oui_version_records_num, PAS_oui_version_records_list, ONU_EVENTFLAG_VIRTAL);
                                }
#else
                                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d resume onu%d's register status for sync-soft.\r\n", olt_id, OnuIdx+1);

								/*modified by liyang @2015-05-18 for warm restart*/
#ifdef BCM_WARM_BOOT_SUPPORT
								if(resume_reason != OLT_RESUMEREASON_SYNC || SYS_LOCAL_MODULE_TYPE_IS_SYS_MANAGER)
								{
#endif
									if( GetOnuVendorType( olt_id, OnuIdx ) == ONU_VENDOR_CT )
									{
										ONU_SetupIFsByType(olt_id, OnuIdx, olt_chipid, (int)ONU_MANAGE_CTC);
									}
									else
									{
										ONU_SetupIFsByType(olt_id, OnuIdx, olt_chipid, (int)ONU_MANAGE_GW);
									}

									if ( ONU_OPER_STATUS_DOWN == status )
									{
										SetOnuOperStatus(olt_id, OnuIdx, ONU_OPER_STATUS_UP);
									}
#ifdef BCM_WARM_BOOT_SUPPORT
								}
#endif
#endif
								++resume_up_num;
							}
                            else
                            {
                                if ( ONU_OPER_STATUS_UP == status )
                                {
                                    ++resume_dn_num;
                                    
                                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event for sync-soft.\r\n", olt_id, OnuIdx+1);
                                    sendOnuDeregistrationEvent( olt_id, OnuIdx, resume_reason, ONU_EVENTFLAG_VIRTAL );
                                }
                            }
                        }
                        else
                        {
                            ++resume_uk_num;
                            
                            OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d sync onu%d's register status for sync-soft.\r\n", olt_id, OnuIdx+1);
                            OnuMgtSyncDataSend_Register( olt_id, OnuIdx );	/* modified by xieshl 20111129, 保持pon-主控-备用主控同步 */
                        }
                    }
                    break;
                case OLT_RESUMEMODE_FORCEDOWN:
                    if ( ONU_OPER_STATUS_UP == status )
                    {
                        ++resume_dn_num;
                        OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event for user-force.\r\n", olt_id, OnuIdx+1);

                        if (INVALID_LLID == llid)
                        {
                            ++resume_uk_num;
                            sendOnuDeregistrationEvent( olt_id, OnuIdx, resume_reason, ONU_EVENTFLAG_VIRTAL );
                        }
                        else
                        {
                            sendOnuDeregistrationMsg( olt_id, llid, resume_reason, ONU_EVENTFLAG_VIRTAL );
                            /* (void)PAS_deregister_onu(olt_id, llid, FALSE); */  /* ONU的虚实变化由PMC自己掌控，无须我们插手 */
                        }
                    }
                    break;
                case OLT_RESUMEMODE_FORCEUP:
                    if ( ONU_OPER_STATUS_DOWN == status )
                    {
                        if ( (0 == CheckOnuIsInPendingQueue(olt_id, MacAddr))
                            && (0 == CheckOnuIsInPendingConfQueue(olt_id, MacAddr)) )
                        {
                            if (INVALID_LLID != llid)
                            {
                                ++resume_up_num;
                                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's register event for user-force.\r\n", olt_id, OnuIdx+1);
                                
                                ONU_MGMT_SEM_TAKE;
                                /*VOS_StrnCpy( SequenceNo, OnuMgmtTable[OnuEntry].SequenceNo, PON_AUTHENTICATION_SEQUENCE_SIZE );*/
                                VOS_StrCpy( SequenceNo, OnuMgmtTable[OnuEntry].SequenceNo );
                                OAM_Ver = OnuMgmtTable[OnuEntry].OAM_Ver;
                                ONU_MGMT_SEM_GIVE;

                                sendOnuRegistrationMsg( olt_id, llid, MacAddr, SequenceNo, OAM_Ver, ONU_EVENTFLAG_VIRTAL );
                                if ( IsSupportCTCOnu( olt_id ) == TRUE /* GetOnuVendorType( olt_id, OnuIdx ) == ONU_VENDOR_CT */ )
                                {
                                    sendOnuExtOamDiscoveryMsg(olt_id, llid, CTC_DISCOVERY_STATE_COMPLETE, PAS_oui_version_records_num, PAS_oui_version_records_list, ONU_EVENTFLAG_VIRTAL);
                                }
                            }
                            else
                            {
                                /* 无LLID无法恢复注册状态 */
                                ++resume_er_num;
                            }
                        }
                    }
                    break;
                case OLT_RESUMEMODE_SYNCHARD:
                    if ( onu_num < 0 )
                    {
                        if ( 0 != TkAdapter_PAS5201_GetAllOnuParams(olt_id, &onu_num, onu_list) )
                        {
                            onu_num = 0;
                        }
                    }
            
                    if ( onu_num > 0 )
                    {
                        /* 查此MAC是否在线 */
                        for (i=0; i<onu_num; ++i)
                        {
                            if ( MAC_ADDR_IS_EQUAL(MacAddr, onu_list[i].mac_address) )
                            {                                
                                if ( llid != onu_list[i].llid )
                                {
                                    llid = onu_list[i].llid;
                                    if ( 0 != OnuMgt_SetOnuLLID(olt_id, OnuIdx, llid) )
                                    {
                                        OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d force llid%d re-register for sync-hard.\r\n", olt_id, llid);

                                        ++resume_er_num;
                                        TkOLT_DeregisterLLID( olt_id, llid );
                                        llid = INVALID_LLID;
                                    }
                                }

                                /* 标识此MAC已经找到 */
                                ++checkin_num;
                                onu_list[i].rtt = -1;
                                break;
                            }
                        }

                        if ( (i >= onu_num) && (llid != INVALID_LLID) )
                        {
                            llid = INVALID_LLID;
                            (void)OnuMgt_SetOnuLLID(olt_id, OnuIdx, llid);
                        }
                    }
                    else
                    {
                        /* 查此LLID是否在线 */
                        llid = GetLlidActivatedByOnuIdx(olt_id, OnuIdx);
                    }

                    if ( ONU_OPER_STATUS_UP == status )
                    {
                        if ( INVALID_LLID == llid )
                        {
                            ++resume_dn_num;
                            OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event for sync-hard.\r\n", olt_id, OnuIdx+1);

                            sendOnuDeregistrationEvent( olt_id, OnuIdx, resume_reason, ONU_EVENTFLAG_VIRTAL );
                        }
                    }
                    else
                    {
                        if (INVALID_LLID != llid)
                        {
                            if ( ONU_OPER_STATUS_DOWN == status )
                            {
                                if ( (0 == CheckOnuIsInPendingQueue(olt_id, MacAddr))
                                    && (0 == CheckOnuIsInPendingConfQueue(olt_id, MacAddr)) )
                                {
                                    ++resume_up_num;
                                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's register event for sync-hard.\r\n", olt_id, OnuIdx+1);

                                    ONU_MGMT_SEM_TAKE;
                                    /*VOS_StrnCpy( SequenceNo, OnuMgmtTable[OnuEntry].SequenceNo, PON_AUTHENTICATION_SEQUENCE_SIZE );*/
                                    VOS_StrCpy( SequenceNo, OnuMgmtTable[OnuEntry].SequenceNo );
                                    OAM_Ver = OnuMgmtTable[OnuEntry].OAM_Ver;
                                    ONU_MGMT_SEM_GIVE;
    					   
                                    sendOnuRegistrationMsg( olt_id, llid, MacAddr, SequenceNo, OAM_Ver, ONU_EVENTFLAG_VIRTAL );
                                    if ( IsSupportCTCOnu( olt_id ) == TRUE /* GetOnuVendorType( olt_id, OnuIdx ) == ONU_VENDOR_CT */ )
                                    {
                                        sendOnuExtOamDiscoveryMsg(olt_id, llid, CTC_DISCOVERY_STATE_COMPLETE, PAS_oui_version_records_num, PAS_oui_version_records_list, ONU_EVENTFLAG_VIRTAL);
                                    }
                                }
                            }
                        }
                    }
                    break;
                case OLT_RESUMEMODE_ACTIVEHARD:
                    llid = GetLlidActivatedByOnuIdx(olt_id, OnuIdx);
                    break;
                default:
                    VOS_ASSERT(0);
            }
        }
    }

#ifdef BCM_WARM_BOOT_SUPPORT
	/*modified by liyang @2015-05-18 for warm restart*/
    if ( checkin_num <  onu_num || (checkin_num == onu_num && resume_reason == OLT_RESUMEREASON_SYNC))
#else
    if ( checkin_num < onu_num )
#endif
    {

        /* ONU MAC列表里不存在的在线ONU的注册状态恢复 */
        for (i=0; i<onu_num; ++i)
        {
#ifdef BCM_WARM_BOOT_SUPPORT
         	if ( onu_list[i].rtt >= 0 || (resume_reason == OLT_RESUMEREASON_SYNC))
#else
            if ( onu_list[i].rtt >= 0 )
#endif
            {
                ++resume_uk_num;
                llid = onu_list[i].llid;
                
                if ( 0 == TkLinkGetOnuRegisterInfo(olt_id, llid, NULL, NULL, MacAddr, NULL, NULL, NULL, NULL, NULL) )
                {
                    ++resume_up_num;
                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate llid%d's register event for register-loose.\r\n", olt_id, llid);

                    sendOnuRegistrationMsg( olt_id, llid, MacAddr /* onu_list[i].mac_address */, SequenceNo, OAM_Ver, ONU_EVENTFLAG_VIRTAL );
                    if ( IsSupportCTCOnu( olt_id ) == TRUE /* GetOnuVendorType( olt_id, OnuIdx ) == ONU_VENDOR_CT */ )
                    {
                        sendOnuExtOamDiscoveryMsg(olt_id, llid, CTC_DISCOVERY_STATE_COMPLETE, PAS_oui_version_records_num, PAS_oui_version_records_list, ONU_EVENTFLAG_VIRTAL);
                    }
                }
                else
                {
                    ++resume_er_num;
                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d force llid%d re-register for register-loose.\r\n", olt_id, llid);

                    TkOLT_DeregisterLLID( olt_id, llid );
                }
            }
        }    
    }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif

    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"TK_ResumeAllOnuStatus(%d, %d, %d, %d)'s result(up:%d, down:%d, err:%d, unknown:%d) on slot %d.\r\n", olt_id, olt_chipid, resume_reason, resume_mode, resume_up_num, resume_dn_num, resume_er_num, resume_uk_num, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

static int TK3723_ResumeAllOnuStatus(short int olt_id, int resume_reason, int resume_mode)
{
    int iRlt = TK_ResumeAllOnuStatus(olt_id, ponChipType_TK3723, resume_reason, resume_mode);

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ResumeAllOnuStatus(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, resume_reason, resume_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int BCM55524_ResumeAllOnuStatus(short int olt_id, int resume_reason, int resume_mode)
{
    int iRlt = TK_ResumeAllOnuStatus(olt_id, ponChipType_BCM55524, resume_reason, resume_mode);

    OLT_TK_DEBUG(OLT_TK_TITLE"BCM55524_ResumeAllOnuStatus(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, resume_reason, resume_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int TK3723_SetOnuAuthMode(short int olt_id, int auth_switch)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#if 0    
    if ( V2R1_CTC_STACK == TRUE )
    {
        /* 目前，只有CTC的ONU支持硬件静默 */
        iRlt = PAS_set_authorize_mac_address_according_list_mode(olt_id, (bool)auth_switch);
    }
    else
#endif
    {
        /* 使用软件认证表，来实现OLT端的ONU静默 */
        iRlt = 0;
    }


    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetOnuAuthMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, auth_switch, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

/* added by xieshl 20110330, 问题单12139 */
extern ULONG	gLibEponMibSemId;
int TK3723_SetAllOnuAuthMode2(short int olt_id, int enable)
{
    int iRlt = OLT_ERR_OK;
    int auth_mode = 0;
    int OnuIdx = 0;
    short int ul_slot = GetCardIdxByPonChip(olt_id);
    short int ul_port = GetPonPortByPonChip(olt_id);
    
    OLT_LOCAL_ASSERT(olt_id);
    {
        VOS_SemTake( gLibEponMibSemId, WAIT_FOREVER );
        if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
        {
    		if ( enable == V2R1_ONU_AUTHENTICATION_NEW_ONLY || enable == V2R1_ONU_AUTHENTICATION_ALL)
            {
                if(mn_getCtcOnuAuthMode(ul_slot, ul_port, &auth_mode) == VOS_OK && auth_mode == mn_ctc_auth_mode_mac)
                {
                    if( enable == V2R1_ONU_AUTHENTICATION_ALL)
                    {
#if 0                        
                        /*OnuAuth_DeregisterAllOnu( olt_id );*/
                        PONTx_Disable(olt_id);
                        VOS_TaskDelay(100);
                        PONTx_Enable(olt_id);
#else
                        int OnuStatus;
                        short int llid;

                    	for( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
                    	{
                            OnuStatus = GetOnuOperStatus_1(olt_id, OnuIdx);
                            if(OnuStatus == ONU_OPER_STATUS_UP)	
                            {
                                llid = GetLlidByOnuIdx(olt_id, OnuIdx);
                                if(llid != INVALID_LLID)
                            		TkOLT_DeregisterLLID( olt_id, llid );
                        	}
                    	}
#endif
                    }
                }
            }
            else if (enable == V2R1_ONU_AUTHENTICATION_DISABLE)
            {
                /*STATUS ret = VOS_ERROR;

                ret = SetOnuAuthenticationMode( olt_id, FALSE );
                if( ret != VOS_OK )
                    sys_console_printf("\r\nset onu auth disable Error! pon%d/%d", SYS_LOCAL_MODULE_SLOTNO, olt_id+1);*/

                ActivatePendingOnuMsg( olt_id );
            }
        }
        VOS_SemGive( gLibEponMibSemId );
    }
    #if 0
    if ( V2R1_CTC_STACK == TRUE )
    {
        /* 目前，只有CTC的ONU支持硬件静默 */
        if (enable == V2R1_ONU_AUTHENTICATION_DISABLE)
        {
            CTC_STACK_set_auth_mode(olt_id, CTC_AUTH_MODE_MAC);
            iRlt = PAS_set_authorize_mac_address_according_list_mode( olt_id, FALSE );
            ActivatePendingOnuMsg( olt_id );
        }
    	else if ( enable == V2R1_ONU_AUTHENTICATION_NEW_ONLY || enable == V2R1_ONU_AUTHENTICATION_ALL)
    	{
            iRlt = CTC_STACK_set_auth_mode(olt_id, CTC_AUTH_MODE_MAC);
    	}
        else if (enable == V2R1_ONU_AUTHENTICATION_LOID)
        {
            iRlt = CTC_STACK_set_auth_mode(olt_id, CTC_AUTH_MODE_LOID);
        }
        else if (enable == V2R1_ONU_AUTHENTICATION_HYBIRD)
        {
            iRlt = CTC_STACK_set_auth_mode(olt_id, CTC_AUTH_MODE_HYBRID);
        }
        else if (enable == V2R1_ONU_AUTHENTICATION_LOID_NO_PWD)
        {
            iRlt = CTC_STACK_set_auth_mode(olt_id, CTC_AUTH_MODE_LOID_NOT_CHK_PWD);
        }
        else if (enable == V2R1_ONU_AUTHENTICATION_HYBIRD_NO_PWD)
        {
            iRlt = CTC_STACK_set_auth_mode(olt_id, CTC_AUTH_MODE_HYBRID_NOT_CHK_PWD);
        }
    } 
    #endif

#if 0	/* modified by xieshl 20110805, 问题单12878 */

    {
        /* 使用软件认证表，来实现OLT端的ONU静默 */
        short int ponid;

        VOS_SemTake( gLibEponMibSemId, WAIT_FOREVER );
        /*if( (iRlt = SlotCardIsPonBoard(SYS_LOCAL_MODULE_SLOTNO) ) == ROK )*/	/* modified by xieshl 20111121, 问题单13908 */
	 if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
        {
    		for( ponid=0; ponid<MAXPON; ponid++ )/* 问题单12798 */
    		{
    			if( PonPortIsWorking( ponid ) != TRUE )
    				continue;
    					
    			if(( enable == 2 ) ||(enable == 3))
    			{
    				STATUS ret = VOS_ERROR;

    				ret = SetOnuAuthenticationMode( ponid, TRUE );
    				if( ret != VOS_OK )
    					sys_console_printf("\r\nset onu auth enable Error! pon%d/%d", SYS_LOCAL_MODULE_SLOTNO, ponid+1);

    				if( enable == 2 )
    				{
    					AddRegisterOnuToAuthEntry( ponid);
    				}
    				else if( enable == 3 )
    				{
    					/*OnuAuth_DeregisterAllOnu( ponid );*/
    					PONTx_Disable(ponid);
    					VOS_TaskDelay(100);			/* 问题单13997 */
    					PONTx_Enable(ponid);
    				}
    			}
    			else if( enable == 1 )
    			{
    				STATUS ret = VOS_ERROR;

    				ret = SetOnuAuthenticationMode( ponid, FALSE );
    				if( ret != VOS_OK )
    					sys_console_printf("\r\nset onu auth disable Error! pon%d/%d", SYS_LOCAL_MODULE_SLOTNO, ponid+1);

    				ActivatePendingOnuMsg( ponid );
    			}
    		}
    	}
        VOS_SemGive( gLibEponMibSemId );
    }

    if ( V2R1_CTC_STACK == TRUE )
    {
        bool mode = FALSE;
        /* 目前，只有CTC的ONU支持硬件静默 */
        if( enable == 2 || enable == 3)
            mode = TRUE;
        iRlt = PAS_set_authorize_mac_address_according_list_mode(olt_id, mode);
    }
#endif

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetAllOnuAuthMode2(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetMacAuth(short int olt_id, int mode, mac_address_t mac)
{
    int iRlt = 0;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(mac);

    if ( V2R1_CTC_STACK )
    {
        /*removed by luh 2012-4-25  Q.15044*/
       	/*iRlt = PAS_set_mac_address_authentication( olt_id, (bool)mode, 1, (mac_address_t*)mac );
        if ( 0 != iRlt )
        {
            if ( (mode && (ENTRY_ALREADY_EXISTS == iRlt))
                || ( !mode && (PAS_QUERY_FAILED == iRlt)) )
            {
                iRlt = 0;
            }
        }*/
    }
    else
    {
        /* 非CTC模式, 由软件实现MAC认证 */
        iRlt = 0;
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetMacAuth(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetOnuDownlinkPri2CoSQueueMap(short int olt_id, OLT_pri2cosqueue_map_t *map)
{
    int iRlt;
#if 0    
    PON_high_priority_frames_t high_priority_frames;
    unsigned char i;
			
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(map);

    for(i=0; i<=MAX_PRIORITY_QUEUE; i++)
    {
        high_priority_frames.priority[i] = (bool)map->priority[i];
    }		

    iRlt = PAS_set_policing_thresholds(olt_id, PON_POLICER_DOWNSTREAM_TRAFFIC, high_priority_frames, PON_POLICER_MAX_HIGH_PRIORITY_RESERVED);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetOnuDownlinkPri2CoSQueueMap(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* 激活ONU */
static int TK3723_ActivePendingOnu(short int olt_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    pendingOnu_S *CurOnu;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
    CurOnu = PonPortTable[olt_id].PendingOnu.Next;
    while( CurOnu != NULL )
    {		
        (void)TkOLT_DeregisterLLID( olt_id, CurOnu->Llid );
        iRlt = 0;
        
        CurOnu = CurOnu->Next;
    }
    VOS_SemGive( OnuPendingDataSemId );

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ActivePendingOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, CurOnu->Llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;	
}

/* 激活一个ONU */
static int TK3723_ActiveOnePendingOnu(short int olt_id, mac_address_t mac)
{
    int iRlt = OLT_ERR_NOTEXIST;
    pendingOnu_S *CurOnu;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(mac);

    VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
    CurOnu = PonPortTable[olt_id].PendingOnu.Next;
    while( CurOnu != NULL )
    {		
        if( MAC_ADDR_IS_EQUAL(CurOnu->OnuMarkInfor.OnuMark.MacAddr, mac) )
        {
            (void)TkOLT_DeregisterLLID( olt_id, CurOnu->Llid );
            iRlt = 0;
            break;
        }
        CurOnu = CurOnu->Next;
    }	
    VOS_SemGive( OnuPendingDataSemId );

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ActivateOnePendingOnu(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_ActiveConfPendingOnu(short int olt_id, short int conf_olt_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int PonIdx, PonGlobalIdx;
    pendingOnu_S *CurOnu;

    OLT_LOCAL_ASSERT(olt_id);
	/*modified by liyang @2015-04-15 for conflict olt id in global range*/
	OLT_ID_ASSERT(conf_olt_id);

    PonGlobalIdx = GetGlobalPonPortIdx(olt_id);
    if ( PonGlobalIdx != conf_olt_id )
    {
        VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
        CurOnu = PonPortTable[olt_id].PendingOnu_Conf.Next;
        while( CurOnu != NULL )
        {		
            PonIdx = CurOnu->otherPonIdx;				
            if( PonIdx == conf_olt_id )
            {		
                (void)TkOLT_DeregisterLLID( olt_id, CurOnu->Llid );
                iRlt = 0;
            }

            CurOnu = CurOnu->Next;
        }
        VOS_SemGive( OnuPendingDataSemId );
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ActiveConfPendingOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, conf_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_ActiveOneConfPendingOnu(short int olt_id, short int conf_olt_id, mac_address_t mac)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int PonIdx, PonGlobalIdx;
    pendingOnu_S *CurOnu;

    OLT_LOCALID_ASSERT(olt_id);
    VOS_ASSERT(mac);

    PonGlobalIdx = GetGlobalPonPortIdx(olt_id);
    if ( PonGlobalIdx != conf_olt_id )
    {
        /* conf_olt_id 值实际传过来的值与当前pon口id相同，造成无法激活onu */
        /*if ( PonGlobalIdx == conf_olt_id ) continue;*/

        VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
        CurOnu = PonPortTable[olt_id].PendingOnu_Conf.Next;
        while( CurOnu != NULL )
        {		
            PonIdx = CurOnu->otherPonIdx;	
            /*modified by luh 2012-6-20 因为pending队列中mac地址的唯一性，直接到pon板上查询mac地址即可*/
            if( (PonIdx == conf_olt_id) && MAC_ADDR_IS_EQUAL(mac, CurOnu->OnuMarkInfor.OnuMark.MacAddr) )
            {					
                (void)TkOLT_DeregisterLLID( olt_id, CurOnu->Llid );
                iRlt = 0;
                
                break;					
            }

            CurOnu = CurOnu->Next;
        }
        VOS_SemGive( OnuPendingDataSemId );
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ActiveOneConfPendingOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, conf_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetOnuP2PMode(short int olt_id, int p2p_mode)
{
    int iRlt;
    int p2p_num;
        
    OLT_LOCAL_ASSERT(olt_id);

    if ( 0 == (iRlt = TkBridgeGetP2PDataPathNum(olt_id, &p2p_num)) )
    {
        if ( p2p_mode )
        {
            if ( 0 < p2p_num )
            {
                /* 已经打开P2P的OLT，需要根据配置自动刷新P2P */
                iRlt = TkBridgeUpdateP2PDataPath(olt_id, -1);
            }
            else
            {
                iRlt = TkBridgeOpenP2PDataPath(olt_id);
            }
        }
        else
        {
            if ( 0 < p2p_num )
            {
                iRlt = TkBridgeCloseP2PDataPath(olt_id);
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetOnuP2PMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, p2p_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int TK3723_GetOnuB2PMode(short int olt_id, int *b2p_mode)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

    if ( 0 <= (iRlt = TkBridgeGetOltDataPathNum(olt_id)) )
    {
        *b2p_mode = (1 < iRlt) ? TRUE : FALSE;
        iRlt = 0;
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetOnuB2PMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *b2p_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetOnuB2PMode(short int olt_id, int b2p_mode)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

    if ( b2p_mode )
    {
        iRlt = TkBridgeOpenMultiDataPath(olt_id);
    }
    else
    {
        iRlt = TkBridgeCloseMultiDataPath(olt_id);
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetOnuB2PMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, b2p_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_GetOnuMode(short int olt_id, short int llid)
{
    int iRlt;
    
    OLT_LOCALID_ASSERT(olt_id);

    if ( LinkIsOnline(olt_id, llid) )
    {
        iRlt = PON_ONU_MODE_ON;
    }
    else
    {
        iRlt = PON_ONU_MODE_OFF;
    }

	OLT_PAS_DEBUG(OLT_PAS_TITLE"TK3723_GetOnuMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
}

#endif


#if 1
/* -------------------OLT 加密管理API----------- */

static int TK3723_SetEncryptMode(short int olt_id, int encrypt_mode)
{
    int iRlt;
    short int port_id;
    TkEncryptionInfo encry_info;
       
    OLT_LOCAL_ASSERT(olt_id);

    port_id = (short int)TkChip_GetOltPonPortID(olt_id);   
#if 1	/* removed by xieshl 20160106, 不确定是否存在PON异常风险，暂不启用 */

    if ( 0 == (iRlt = TkOLT_GetPortEncryptMode(olt_id, port_id, &encry_info)) )
    {
        switch ( encrypt_mode )
        {
            case PON_ENCRYPTION_TYPE_AES:
                encry_info.mode = TkEncryptModeAes;

                break;
            case PON_ENCRYPTION_TYPE_TRIPLE_CHURNING:
                encry_info.mode = TkEncryptModeTripleChurning;

                break;
            default:    
                VOS_ASSERT(0);
                encry_info.mode = TkEncryptModeNone;
        }

        if ( TkEncryptModeNone == encry_info.mode )
        {
            encry_info.keyExchangeTimeout = 0;
        }
        else
        {
            if ( 0 == encry_info.keyExchangeTimeout )
            {
                unsigned int keyExchangeTimeout;

                if ( 0xFFFF < (keyExchangeTimeout = PonPortTable[olt_id].EncryptKeyTime / 1000) )
                {
                    keyExchangeTimeout = 0xFFFF;
                }
                else 
                {
                    if ( 0 == keyExchangeTimeout )
                    {
                        keyExchangeTimeout = EncryptKeyTimeDefault;
                    }
                }

                encry_info.keyExchangeTimeout = (U16)keyExchangeTimeout;
            }
        }
        
        iRlt = TkOLT_SetPortEncryptMode(olt_id, port_id, &encry_info);
    }
#else
    encry_info.mode = TkEncryptModeNone;
    encry_info.keyExchangeTimeout = 0;
    iRlt = TkOLT_SetPortEncryptMode(olt_id, port_id, &encry_info);
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetEncryptMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, encrypt_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_GetLLIDEncryptMode(short int olt_id, short int llid, bool *encrypt_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(encrypt_mode);

#if 0    
    iRlt = Get_encryption_mode(olt_id, llid, encrypt_mode);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetLLIDEncryptMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, *encrypt_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_StartLLIDEncrypt(short int olt_id, short int llid)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

#if 0    
    iRlt = PAS_start_olt_encryption(olt_id, llid);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_StartLLIDEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_FinishLLIDEncrypt(short int olt_id, short int llid, short int status)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

#if 0    
    iRlt = PAS_finalize_start_olt_encryption(olt_id, llid, status);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_FinishLLIDEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_StopLLIDEncrypt(short int olt_id, short int llid)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

#if 0    
    iRlt = PAS_stop_olt_encryption(olt_id, llid);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_StopLLIDEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_SetLLIDEncryptKey(short int olt_id, short int llid, PON_encryption_key_t key)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(key);

#if 0    
    iRlt = PAS_set_olt_encryption_key(olt_id, llid, key);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetLLIDEncryptKey(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_FinishLLIDEncryptKey(short int olt_id, short int llid, short int status)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

#if 0    
    iRlt = PAS_finalize_set_olt_encryption_key(olt_id, llid, status);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_FinishLLIDEncryptKey(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

#endif


#if 1
/* -------------------OLT 地址表管理API-------- */

static int TK3723_SetMacAgingTime(short int olt_id, int aging_time)
{
    int iRlt;
	
    OLT_LOCAL_ASSERT(olt_id);
    
    iRlt = TkOLT_SetMacAgingTime(olt_id, (unsigned long)aging_time);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetMacAgingTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, aging_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int TK3723_SetAddressTableConfig(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg)
{
    int iRlt = 0;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addrtbl_cfg);

    do{

    if (addrtbl_cfg->aging_timer >= 0)
    {
        if ( 0 != (iRlt = TkOLT_SetMacAgingTime(olt_id, (unsigned long)addrtbl_cfg->aging_timer)) )
        {
            break;
        }
    }

    }while(0);
    
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetAddressTableConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, addrtbl_cfg->removed_when_full, addrtbl_cfg->discard_llid_unlearned_sa, addrtbl_cfg->discard_unknown_da, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int TK3723_GetAddressTableConfig(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg)
{
    int iRlt;
    unsigned long ulValue;	
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addrtbl_cfg);
    
    if ( 0 == (iRlt = TkOLT_GetMacAgingTime(olt_id, &ulValue)) )
    {
        VOS_MemZero(addrtbl_cfg, sizeof(*addrtbl_cfg));
        
        addrtbl_cfg->aging_timer = (int)ulValue;
        addrtbl_cfg->allow_learning = PON_DIRECTION_UPLINK;
        addrtbl_cfg->removed_when_aged = TRUE;
		/*added by liyang @2015-04-10 for Q25151*/
		addrtbl_cfg->removed_when_full = TRUE;
		addrtbl_cfg->discard_llid_unlearned_sa = FALSE;
		addrtbl_cfg->discard_unknown_da = PON_DIRECTION_NO_DIRECTION;
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetAddressTableConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, addrtbl_cfg->removed_when_full, addrtbl_cfg->discard_llid_unlearned_sa, addrtbl_cfg->discard_unknown_da, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int TK3723_GetMacAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num);

    iRlt = TkAdapter_PAS5201_GetAddrTable(olt_id, addr_num, addr_tbl);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetMacAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int  TK3723_AddMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num != 0);
    VOS_ASSERT(addr_tbl);

    if ( addr_num < 0 )
    {
        addr_num = -addr_num;
    }

#if 0
    iRlt = TkAdapter_PAS5201_AddAddrTable(olt_id, addr_num, addr_tbl);
#else
    iRlt = 0;
#endif

    OLT_PAS_DEBUG(OLT_PAS_TITLE"TK3723_AddMacAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_DelMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num != 0);
    VOS_ASSERT(addr_tbl);

    if ( addr_num < 0 )
    {
        addr_num = -addr_num;
    }
   
    iRlt = TkAdapter_PAS5201_DeleteAddrTable(olt_id, addr_num, addr_tbl);

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_DelMacAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_RemoveMac(short int olt_id, mac_address_t mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(mac);

    iRlt = TkAdapter_PAS5201_DeleteMacAddr(olt_id, mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_RemoveMac(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_ResetAddrTbl(short int olt_id, short int llid, int addr_type)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = TkAdapter_PAS5201_ResetLLIDAddrTable(olt_id, llid, addr_type);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ResetAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, addr_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}
static int TK3723_GetLlidByUserMacAddress(short int olt_id, mac_address_t mac, short int *llid)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = TkAdapter_PAS5201_GetLlidByMac(olt_id, mac, llid);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetLlidByUserMacAddress(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}


#endif


#if 1
/* -------------------OLT 光路管理API----------- */

static int TK3723_GetOpticalCapability(short int olt_id, OLT_optical_capability_t *optical_capability)
{
    int iRlt;
    short int port_id;
    TkOltEponStrobesParameters stOpticParams;
    TkPortInfo stPortInfo;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(optical_capability);

    port_id = (short int)TkChip_GetOltPonPortID(olt_id);   
    if ( 0 == (iRlt = TkOLT_GetPortOpticalParams(olt_id, port_id, &stOpticParams)) )
    {
        VOS_MemZero(optical_capability, sizeof(OLT_optical_capability_t));

        optical_capability->optical_capabilitys = PON_OPTICAL_CAPABILITY_LOCKTIME;
        optical_capability->agc_lock_time = stOpticParams.agc.len;
        optical_capability->cdr_lock_time = stOpticParams.cdr.len;
        
        if ( 0 == (iRlt = TkOLT_GetPort(olt_id, port_id, &stPortInfo)) )
        {
            if ( stPortInfo.enabled )    
            {
                optical_capability->pon_tx_signal = TRUE;
            }
            else
            {
                optical_capability->pon_tx_signal = FALSE;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetOpticalCapability(%d, %d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, optical_capability->agc_lock_time, optical_capability->cdr_lock_time, optical_capability->laser_on_time, optical_capability->laser_off_time, optical_capability->pon_tx_signal, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_GetOpticsDetail(short int olt_id, OLT_optics_detail_t *optics_params) 
{
    int iRlt;
    short int port_id;
    TkOltEponStrobesParameters stOpticParams;
    TkPortInfo stPortInfo;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(optics_params);

    port_id = (short int)TkChip_GetOltPonPortID(olt_id);   
    if ( 0 == (iRlt = TkOLT_GetPortOpticalParams(olt_id, port_id, &stOpticParams)) )
    {
        PON_olt_optics_configuration_t *pstOpticParams;
        PON_agc_reset_configuration_t *pstAgcParams;
        PON_cdr_reset_configuration_t *pstCdrParams;
        TkOltStrobeSingle *pstStrobeParams;
    
        VOS_MemZero(optics_params, sizeof(OLT_optics_detail_t));

        pstOpticParams = &optics_params->pon_optics_params;
        pstOpticParams->configuration_source = PON_OLT_OPTICS_CONFIGURATION_SOURCE_EEPROM;

        pstAgcParams = &pstOpticParams->agc_reset_configuration;
        pstStrobeParams = &stOpticParams.agc;
        pstAgcParams->gate_offset = (pstStrobeParams->offset.array[1] << 8) | pstStrobeParams->offset.array[2];
        pstAgcParams->discovery_offset = pstAgcParams->gate_offset;
        pstAgcParams->polarity = pstStrobeParams->polarity;
        pstAgcParams->duration = pstStrobeParams->len;
        pstOpticParams->agc_lock_time = pstStrobeParams->len;
        
        pstCdrParams = &pstOpticParams->cdr_reset_configuration;
        pstStrobeParams = &stOpticParams.cdr;
        pstCdrParams->gate_offset = (pstStrobeParams->offset.array[1] << 8) | pstStrobeParams->offset.array[2];
        pstCdrParams->discovery_offset = pstCdrParams->gate_offset;
        pstCdrParams->polarity = pstStrobeParams->polarity;
        pstCdrParams->duration = pstStrobeParams->len;
        pstOpticParams->cdr_lock_time = pstStrobeParams->len;
        
        if ( 0 == (iRlt = TkOLT_GetPort(olt_id, port_id, &stPortInfo)) )
        {
            if ( stPortInfo.enabled )    
            {
                optics_params->pon_tx_signal = TRUE;
            }
            else
            {
                optics_params->pon_tx_signal = FALSE;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetOpticsDetail(%d, %d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, optics_params->pon_optics_params.agc_lock_time, optics_params->pon_optics_params.cdr_lock_time, optics_params->pon_optics_params.discovery_laser_on_time, optics_params->pon_optics_params.discovery_laser_off_time, optics_params->pon_tx_signal, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetPonRange(short int olt_id, unsigned int max_range, unsigned int max_rtt)
{
    int iRlt;
    short int pon_portid = TkChip_GetOltPonPortID(olt_id);
	TkDiscoveryParametersV2 discovery_parameters;
	TkEponLoopTiming loop_timing;
	
	if(TK_EXIT_OK == (iRlt = TkOLT_DisableOlt(olt_id)))
	{
		if(TK_EXIT_OK == (iRlt = TkOLT_GetDiscoveryParametersV2(olt_id, pon_portid, &discovery_parameters)))
		{
			if(max_range == 1)
				discovery_parameters.discoveryProcesses.grantLength = 16319;
			else if(max_range == 2)
				discovery_parameters.discoveryProcesses.grantLength = 30000;
			else if(max_range == 3)
				discovery_parameters.discoveryProcesses.grantLength = 55000;
			else if(max_range == 4)
				discovery_parameters.discoveryProcesses.grantLength = 80000;

			if(TK_EXIT_OK == (iRlt = TkOLT_SetDiscoveryParametersV2(olt_id, pon_portid, &discovery_parameters)))			
			{
				if(TK_EXIT_OK == (iRlt = TkOLT_GetLoopTiming(olt_id, pon_portid,&loop_timing)))				
				{
					if(max_range == 0)
						loop_timing.maxPropDelay = 0;
					else 
 						loop_timing.maxPropDelay = (max_rtt-3125)/2;
				

					if(TK_EXIT_OK == (iRlt = TkOLT_SetLoopTiming(olt_id, pon_portid,&loop_timing)))				
					{
						iRlt = TkOLT_EnableOlt(olt_id);
					}			
				}

			}
		}
	}	

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetPonRange(%d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, max_range, max_rtt, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetOpticalTxMode(short int olt_id, int tx_mode)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
	/* added by wangjiah@2017-03-17 data path must be opened before turn on optical tx*/
	if(tx_mode)
	{
	    iRlt = TkOltOpenDataPath(olt_id);
	    OLT_TK_DEBUG(OLT_TK_TITLE"TkOltOpenDataPath(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, tx_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	}
    iRlt = TkOltSetOpticalTxMode(olt_id, tx_mode);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetOpticalTxMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, tx_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_GetOpticalTxMode(short int olt_id, int *tx_mode)
{
    int iRlt;
    short int port_id;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(tx_mode);

    iRlt = TkOltGetOpticalTxMode(olt_id, tx_mode);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetOpticalTxMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *tx_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetVirtualScopeAdcConfig(short int olt_id, PON_adc_config_t *adc_config)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(adc_config);

	iRlt = TkAdapter_PAS5201_set_virtual_scope_adc_config(olt_id, adc_config);

	OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetVirtualScopeAdcConfig(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_GetVirtualScopeMeasurement(short int olt_id, short int llid, PON_measurement_type_t measurement_type, 
	void *configuration, short int config_len, void *result, short int res_len)
{
    int iRlt;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = TkAdapter_PAS5201_get_virtual_scope_measurement(olt_id, llid, measurement_type, configuration, result);

	OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetVirtualScopeMeasurement(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_GetVirtualScopeRssiMeasurement(short int olt_id, short int llid, PON_rssi_result_t *rssi_result)
{
    int iRlt;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = TkAdapter_PAS5201_get_virtual_scope_rssi_measurement(olt_id, llid, rssi_result);

	OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetVirtualScopeRssiMeasurement(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, (int)rssi_result->dbm, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_GetVirtualScopeOnuVoltage(PON_olt_id_t olt_id, short int llid, float *voltage,unsigned short int *sample, float *dbm)
{
    int iRlt;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = TkAdapter_PAS5201_get_virtual_scope_onu_voltage(olt_id, llid, voltage, sample, dbm);

	OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetVirtualScopeOnuVoltage(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, (int)*dbm, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetOpticalTxMode2(short int olt_id, int tx_mode, int tx_reason)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
	/*added by luh @2015-06-18*/
	if(tx_mode)
	{
	    iRlt = TkOltOpenDataPath(olt_id);
	    OLT_TK_DEBUG(OLT_TK_TITLE"TkOltOpenDataPath(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, tx_mode, tx_reason, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	}
    if ( 0 < (iRlt = GW_SetOpticalTxMode2(olt_id, tx_mode, tx_reason)) )
    {
		iRlt = TkOltSetOpticalTxMode(olt_id, tx_mode);
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetOpticalTxMode2(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, tx_mode, tx_reason, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif


#if 1
/* -------------------OLT 监控统计管理API---- */

static int TK3723_GetRawStatistics(short int olt_id, OLT_raw_stat_item_t *stat_item)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(stat_item);
    VOS_ASSERT(stat_item->statistics_data);
    VOS_ASSERT(stat_item->statistics_data_size > 0);
    
    iRlt = TkAdapter_PAS5201_get_raw_statistics(olt_id, stat_item->collector_id
            , stat_item->raw_statistics_type, stat_item->statistics_parameter
            , stat_item->statistics_data, &stat_item->timestam);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetRawStatistics(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, stat_item->collector_id, stat_item->raw_statistics_type, stat_item->statistics_parameter, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_ResetCounters(short int olt_id)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    
    iRlt = TkOLT_ResetAllCounter(olt_id);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ResetCounters(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetBerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_bytes)
{
    int iRlt;
    bool activate;
    PON_ber_alarm_configuration_t alarm_cfg;
	
    OLT_LOCAL_ASSERT(olt_id);

    if ( alarm_switch < 0 )
    {
        alarm_switch = gpOnuAlarm[olt_id]->ponBERAlarm;
    }
    
    if ( alarm_thresold < 0 )
    {
        alarm_thresold = gPonThreashold.ponBERThreashold;
    }
    VOS_ASSERT(alarm_thresold < ARRAY_SIZE(g_afLevelConst));

    if ( alarm_min_error_bytes < 0 )
    {
        alarm_min_error_bytes = gPonThreashold.ponBerNum;
    }

    activate = (ALARM_DISABLE == alarm_switch) ? FALSE : TRUE;  
    alarm_cfg.direction = PON_DIRECTION_UPLINK_AND_DOWNLINK;
    alarm_cfg.ber_threshold = g_afLevelConst[alarm_thresold];
    alarm_cfg.minimum_error_bytes_threshold = alarm_min_error_bytes;
	iRlt = TkAdapter_PAS5201_set_alarm_configuration(olt_id, -1, PON_ALARM_BER, activate, &alarm_cfg);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetBerAlarmConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, alarm_switch, alarm_thresold, alarm_min_error_bytes, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int TK3723_SetFerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_frames)
{
    int iRlt;
    bool activate;
    PON_fer_alarm_configuration_t alarm_cfg;
	
    OLT_LOCAL_ASSERT(olt_id);

    if ( alarm_switch < 0 )
    {
        alarm_switch = gpOnuAlarm[olt_id]->ponFERAlarm;
    }
    
    if ( alarm_thresold < 0 )
    {
        alarm_thresold = gPonThreashold.ponFERThreashold;
    }
    VOS_ASSERT(alarm_thresold < ARRAY_SIZE(g_afLevelConst));
    
    if ( alarm_min_error_frames < 0 )
    {
        alarm_min_error_frames = gPonThreashold.ponFerNum;
    }

    activate = (ALARM_DISABLE == alarm_switch) ? FALSE : TRUE; 
    alarm_cfg.direction = PON_DIRECTION_UPLINK_AND_DOWNLINK;
    alarm_cfg.fer_threshold = g_afLevelConst[alarm_thresold];
    alarm_cfg.minimum_error_frames_threshold = alarm_min_error_frames;
	iRlt = TkAdapter_PAS5201_set_alarm_configuration(olt_id, -1, PON_ALARM_FER, activate, &alarm_cfg);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetFerAlarmConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, alarm_switch, alarm_thresold, alarm_min_error_frames, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int TK3723_SetAlarmConfig(short int olt_id, short int source, PON_alarm_t type, bool activate, void	*configuration, int length)
{
    int iRlt = ROK;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = TkAdapter_PAS5201_set_alarm_configuration(olt_id, source, type, activate, configuration);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetAlarmConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, source, type, activate, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int TK3723_GetAlarmConfig(short int olt_id, short int source, PON_alarm_t type, bool *activate, void	*configuration)
{
    int iRlt = ROK;

	OLT_LOCAL_ASSERT(olt_id);
    
	iRlt = TkAdapter_PAS5201_get_alarm_configuration(olt_id, source, type, activate, configuration);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetAlarmConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, source, type, *activate, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int TK3723_GetStatistics(short int olt_id, short int collector_id, PON_statistics_t statistics_type, short int statistics_parameter, long double *statistics_data)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
 
    iRlt = TkAdapter_PAS5201_get_statistics(olt_id, collector_id, statistics_type, statistics_parameter, statistics_data);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetStatistics(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, collector_id, statistics_type, statistics_parameter, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_OltSelfTest(short int olt_id)
{
    int iRlt;
    unsigned char test_error;

	OLT_LOCAL_ASSERT(olt_id);

	if ( 0 == (iRlt = TkOLT_SelfTest(olt_id, TkOltDiagnosticTestTypeProcessorFlashCrcValidation, &test_error)) )
    {
        iRlt = (0 == test_error) ? TEST_SUCCEEDED : TEST_FAILED;
    }   

	OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_OltSelfTest(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_LinkTest(short int olt_id, short int llid, short int number_of_frames, short int frame_size, bool link_delay_measurement, PON_link_test_vlan_configuration_t *vlan_configuration, PON_link_test_results_t *test_results)
{
    int iRlt = RERROR;
    TkLoopbackTestRequestParams tk_params;
    TkLoopbackTestResult tk_results;
    
    OLT_LOCALID_ASSERT(olt_id);

    tk_params.location = TkLoopbackLocationMac;
    tk_params.numFrames = number_of_frames;
    tk_params.payloadLen = frame_size;
    tk_params.payloadType = TkLoopbackPayloadTypeIncrementing;
    if ( vlan_configuration->vlan_frame_enable )
    {
        tk_params.vid = (vlan_configuration->vlan_priority << 13) | (vlan_configuration->vlan_tag & 0xFFF);
    }
    else
    {
        tk_params.vid = 0;
    }
    tk_params.options = TkLoopbackOptionRestoreDynamicMacs;

	if ( 0 == (iRlt = TkOLT_LinkLoopbackTest(olt_id, llid, &tk_params, &tk_results)) )
    {
        test_results->number_of_sent_frames = tk_results.framesSent;
        test_results->number_of_returned_frames= tk_results.framesReceived;
        test_results->number_of_errored_return_frames = tk_results.framesCorrupted;
        
        test_results->minimal_delay = tk_results.minDelay;
        test_results->maximal_delay = tk_results.maxDelay;
        test_results->mean_delay = tk_results.averageDelay;
    }   

	OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_LinkTest(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int TK3723_SetLLIDFecMode(short int olt_id, short int llid, bool downlink_fec)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

#if 0    
    iRlt = PAS_set_llid_fec_mode(olt_id, llid, downlink_fec);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetLLIDFecMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, downlink_fec, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_GetLLIDFecMode(short int olt_id, short int llid, bool *downlink_fec, bool *uplink_fec, bool *uplink_lastframe_fec)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(downlink_fec);
    VOS_ASSERT(uplink_fec);
    VOS_ASSERT(uplink_lastframe_fec);

#if 0    
    iRlt = PAS_get_llid_fec_mode(olt_id, llid, downlink_fec, uplink_fec, uplink_lastframe_fec);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetLLIDFecMode(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, *downlink_fec, *uplink_fec, *uplink_lastframe_fec, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

#endif


#if 1
/* -------------------OLT 倒换API---------------- */

static int TK3723_GetHotSwapCapability(short int olt_id, int *swap_cap)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(swap_cap);

    *swap_cap = V2R1_PON_PORT_SWAP_SLOWLY;
    
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetHotSwapCapability(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *swap_cap, 0, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

static int TK3723_GetHotSwapMode(short int olt_id, short int *partner_olt_id, int *swap_mode, int *swap_status)
{
    int iRlt;
    
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(partner_olt_id);
    VOS_ASSERT(swap_mode);
    VOS_ASSERT(swap_status);

    if ( OLT_ERR_OK == (iRlt = GW_GetHotSwapMode(olt_id, partner_olt_id, swap_mode, swap_status)) )
    {
        int tx_mode;

        *swap_mode = V2R1_PON_PORT_SWAP_AUTO;
        if ( OLT_ERR_OK == (iRlt = TK3723_GetOpticalTxMode(olt_id, &tx_mode)) )
        {
            if ( tx_mode )    
            {
                *swap_status = V2R1_PON_PORT_SWAP_ACTIVE;
            }
            else
            {
                *swap_status = V2R1_PON_PORT_SWAP_PASSIVE;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetHotSwapMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, *partner_olt_id, *swap_mode, *swap_status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetHotSwapMode(short int olt_id, short int partner_olt_id, int swap_mode, int swap_status, int swap_flags)
{
    int iRlt;
    int iPartRlt;
    int new_swap_mode, old_swap_mode;

    OLT_LOCAL_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);
    PON_SWAPMODE_ASSERT(swap_mode);
    PON_SWAPSTATUS_ASSERT(swap_status);

    /* 单设置成功的结果认定 */
    if ( OLT_SWAP_FLAGS_OLT_SINGLESETTING & swap_flags )
    {
        iPartRlt = 0;
    }
    else
    {
        iPartRlt = OLT_ERR_PARTOK;
    }
    
    /* 除旧换新的新旧认定 */
    old_swap_mode = GetPonPortHotSwapMode(olt_id);
    if ( OLT_SWAP_FLAGS_OLT_RESUMEMODE & swap_flags )
    {
        new_swap_mode = old_swap_mode;
        old_swap_mode = swap_mode;

        iPartRlt = 0;
    }
    else
    {
        new_swap_mode = swap_mode;
    }

    if (new_swap_mode > V2R1_PON_PORT_SWAP_SLOWLY)
    {
        /* 5001不支持快倒换 */
        iRlt = OLT_ERR_NOTSUPPORT;
    }
    else
    {
        switch (new_swap_mode)
        {
            case V2R1_PON_PORT_SWAP_AUTO:
                /* 仅内存设置倒换 */
                iRlt = 0;
                break;
            case V2R1_PON_PORT_SWAP_DISABLED:
				/* modified by wangjiah@2017-04-27
				 * turn off tx on the condition that pon port is shutdown before disable auto-protect*/
				if(PONPORT_DISABLE == GetPonPortAdminStatus(olt_id))
				{
					iRlt = TK3723_SetOpticalTxMode( olt_id, FALSE );
				}
				else
				{
					iRlt = TK3723_SetOpticalTxMode( olt_id, TRUE );
				}
                if ( 0 == iRlt )
                {
                    PonPortTable[olt_id].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
                    if ( OLT_ISLOCAL(partner_olt_id)
                        /* B--added by liwei056@2011-11-22 for D13979 */
                        && OltIsExist(partner_olt_id)
                        /* E--added by liwei056@2011-11-22 for D13979 */
                        )
                    {
						if(PONPORT_DISABLE == GetPonPortAdminStatus(partner_olt_id))
						{
							if ( 0 == (iRlt = TK3723_SetOpticalTxMode( partner_olt_id, FALSE)) )
							{
								PonPortTable[partner_olt_id].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
							}         
						}
						else
						{
							if ( 0 == (iRlt = TK3723_SetOpticalTxMode( partner_olt_id, TRUE )) )
							{
								PonPortTable[partner_olt_id].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
							}         
						}
                    }
                    else
                    {
                        iRlt = iPartRlt;
                    }
                }
                
                break;
            case V2R1_PON_PORT_SWAP_SLOWLY:
            	if( swap_status == V2R1_PON_PORT_SWAP_PASSIVE )
                {
        			iRlt = TK3723_SetOpticalTxMode( olt_id, FALSE );
                }
                else
                {
            		(void)TK3723_ResetAddrTbl(olt_id, PON_ALL_ACTIVE_LLIDS,  ADDR_DYNAMIC);
        			if ( 0 == (iRlt = TK3723_SetOpticalTxMode( olt_id, TRUE )) )
                    {
                        PonPortTable[olt_id].swap_use = V2R1_PON_PORT_SWAP_ACTIVE;
                    }         
                }

                if ( 0 == iRlt )
                {
                    /* 慢倒换不能同时设置，要保证新注册在配置拷贝之后 */
                    iRlt = iPartRlt;
                }
                
                break;
            default:                
                iRlt = OLT_ERR_PARAM;
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetHotSwapMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, swap_mode, swap_status, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int TK3723_ForceHotSwap(short int olt_id, short int partner_olt_id, int swap_status, int swap_flags)
{
    int iRlt;
#if 0
    int iPartRlt;
#endif
    int local_mode, local_status;
    short int swap_olt_id;
    
    OLT_LOCAL_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);
    PON_SWAPSTATUS_ASSERT(swap_status);
    
#if 0
    if ( swap_flags & OLT_SWAP_FLAGS_OLT_SINGLESETTING )
    {
        iPartRlt = 0;
    }
    else
    {
        iPartRlt = OLT_ERR_PARTOK;
    }
#endif

    if ( 0 == (iRlt = GW_GetHotSwapMode(olt_id, &swap_olt_id, &local_mode, &local_status)) )
    {
        if (swap_olt_id == partner_olt_id)
        {
            if (V2R1_PON_PORT_SWAP_SLOWLY == local_mode)
            {
                /* 用户强制倒换*/
				ProcessHotSwap(olt_id, partner_olt_id, PROTECT_SWITCH_REASON_HOSTREQUEST);
            }
            else
            {
                iRlt = OLT_ERR_PARAM;
            }
        }
        else
        {
            iRlt = OLT_ERR_PARAM;
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ForceHotSwap(%d, %d, %d, %d)'s result(%d) at mode(%d) on slot %d.\r\n", olt_id, partner_olt_id, swap_status, swap_flags, iRlt, local_mode, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_RdnOnuRegister(short int olt_id, PON_redundancy_onu_register_t *onu_reg_info)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(onu_reg_info);

#if 0    
    iRlt = PAS_redundancy_onu_register(olt_id, *onu_reg_info);
    if (iRlt < 0)
    {
        if ( -1 == iRlt )
        {
            /* 重复虚注册ONU的底层错误，认为上层正确 */
            iRlt = 0;
#if 0
            if (V2R1_ENABLE == EVENT_PONSWITCH)
            {
                PAS_Test_RdnState(olt_id);
                sys_console_printf("onu-hwver:%d\r\n", onu_reg_info->hardware_version);
                onu_reg_info->hardware_version = PON_PASSAVE_ONU_6301_VERSION;
            }
#endif
        }
    }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_RdnOnuRegister(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_reg_info->onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_SetRdnConfig(short int olt_id, int rdn_status, int gpio_num, int rdn_type, int rx_enable)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#if 0    
    iRlt = PAS_set_redundancy_config(olt_id, rdn_status, (unsigned short)gpio_num, rdn_type, rx_enable);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetRdnConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, rdn_status, rdn_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_RdnSwitchOver(short int olt_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#if 0    
    iRlt = PAS_redundancy_switch_over(olt_id);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_RdnSwitchOver(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_RdnIsExist(short int olt_id, bool *status)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(status);

#if 0    
    *status = redundancy_olt_exists(olt_id);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    /* OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_RdnIsExist(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *status, 0, SYS_LOCAL_MODULE_SLOTNO); */
    
    return 0;    
}

static int TK3723_ResetRdnRecord(short int olt_id, int rdn_state)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#if 0    
#ifdef PAS_SOFT_VERSION_V5_3_12
    iRlt = reset_redundancy_olt_record(olt_id, rdn_state);
#else
    iRlt = reset_redundancy_record(olt_id, rdn_state);
#endif
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_ResetRdnRecord(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, rdn_state, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_GetRdnState(short int olt_id, int *state)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(state);

#if 0    
    iRlt = get_redundancy_state(olt_id, state);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetRdnState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *state, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_SetRdnState(short int olt_id, int state)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#if 0    
    iRlt = set_redundancy_state(olt_id, state);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetRdnState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, state, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_GetRdnAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num);
    VOS_ASSERT(addr_tbl);

#if 0    
    iRlt = PAS_get_redundancy_address_table(olt_id, addr_num, addr_tbl);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetRdnAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_RemoveRdnOlt(short int olt_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#if 0    
#ifdef PAS_SOFT_VERSION_V5_3_12
    iRlt = remove_olt_from_mapping_list(olt_id);
#else
    iRlt = reset_redundancy_record(olt_id, PON_OLT_REDUNDANCY_STATE_NONE);
#endif
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_RemoveRdnOlt(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_GetLLIDRdnDB(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(rdn_db);

#if 0    
    iRlt = CTC_STACK_get_redundancy_database(olt_id, llid, rdn_db);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetLLIDRdnDB(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_SetLLIDRdnDB(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(rdn_db);

#if 0    
    iRlt = CTC_STACK_set_redundancy_database(olt_id, llid, *rdn_db);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetLLIDRdnDB(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int TK3723_RdnRemoveOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);

    if ( OLT_ISREMOTE(partner_olt_id) )
	{
		/* added by wangjiah@2017-03-08:begin
		** start swap when pon card is pulled or reboot */
		if(V2R1_PON_PORT_SWAP_ACTIVE != PonPortHotStatusQuery(olt_id))
		{
			iRlt = sendPonSwitchEventMsg( partner_olt_id, PROTECT_SWITCH_EVENT_START, PON_REDUNDANCY_OLT_RESET, 
				PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, V2R1_PON_PORT_SWAP_PASSIVE, PonPortSwapTimesQuery(olt_id) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
			OLT_SWITCH_DEBUG("\r\n swap_begin_handler(%d,%d) result: %d\r\n", partner_olt_id, PON_REDUNDANCY_OLT_RESET, iRlt);
			iRlt = TK3723_SetOpticalTxMode(olt_id, 1);
			OLT_SWITCH_DEBUG("\r\n setopticaltxmode(%d,%d) result: %d\r\n", olt_id, 1, iRlt);

			iRlt = sendPonSwitchEventMsg( olt_id, PROTECT_SWITCH_EVENT_OVER, PON_REDUNDANCY_SWITCH_OVER_SUCCEED, 
				PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, V2R1_PON_PORT_SWAP_ACTIVE, PonPortSwapTimesQuery(olt_id) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
			OLT_SWITCH_DEBUG("\r\n swap_end_handler(%d,%d) result: %d\r\n", olt_id, PON_REDUNDANCY_OLT_RESET, iRlt);
		}
		/* added by wangjiah@2017-03-08:end*/
	}
    else
    {
		iRlt = OLT_ERR_NOTSUPPORT;
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_RdnRemoveOlt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_RdnSwapOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;
#if 0    
    PON_redundancy_olt_state_t local_olt_rdnstate;

    OLT_LOCAL_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);

    if ( 0 == (iRlt = get_redundancy_state(olt_id, &local_olt_rdnstate)) )
    {
        PON_olt_id_t master_olt_id; 
        PON_olt_id_t slave_olt_id;
        
        switch ( local_olt_rdnstate )
        {
            case PON_OLT_REDUNDANCY_STATE_MASTER:
            case PON_OLT_REDUNDANCY_STATE_INACTIVE_SLAVE:
                master_olt_id = olt_id;
                slave_olt_id  = partner_olt_id;

                break;
            case PON_OLT_REDUNDANCY_STATE_SLAVE:
            case PON_OLT_REDUNDANCY_STATE_INACTIVE_MASTER:
                master_olt_id = partner_olt_id;
                slave_olt_id  = olt_id;

                break;
            default:
                iRlt = OLT_ERR_PARAM;
        }

        if ( 0 == iRlt )
        {
            iRlt = swap_local_mapping_list(master_olt_id, slave_olt_id);
        }
    }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_RdnSwapOlt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_AddSwapOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;
#if 0    
    PON_redundancy_olt_state_t local_olt_rdnstate;

    OLT_LOCAL_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);

    if ( 0 == (iRlt = get_redundancy_state(olt_id, &local_olt_rdnstate)) )
    {
        PON_olt_id_t master_olt_id; 
        PON_olt_id_t slave_olt_id;
        
        switch ( local_olt_rdnstate )
        {
            case PON_OLT_REDUNDANCY_STATE_MASTER:
            case PON_OLT_REDUNDANCY_STATE_INACTIVE_SLAVE:
                master_olt_id = olt_id;
                slave_olt_id  = partner_olt_id;

                break;
            case PON_OLT_REDUNDANCY_STATE_SLAVE:
            case PON_OLT_REDUNDANCY_STATE_INACTIVE_MASTER:
                master_olt_id = partner_olt_id;
                slave_olt_id  = olt_id;

                break;
            default:
                iRlt = OLT_ERR_PARAM;
        }

        if ( 0 == iRlt )
        {
            iRlt = insert_mapping_pair_to_local_list(master_olt_id, slave_olt_id);
        }
    }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif

    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_AddSwapOlt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_RdnLooseOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#if 0    
    if ( OLT_ISREMOTE(partner_olt_id) )
    {
        iRlt = remote_olt_loosed_handler(olt_id, partner_olt_id);
    }
    else
    {
        iRlt = OLT_ERR_PARAM;
    }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_RdnLooseOlt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_GetLLIDRdnRegisterInfo(short int olt_id, short int llid, PON_redundancy_onu_register_t *onu_reginfo)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_reginfo);
    
#if 0    
    iRlt = get_onu_register_info_from_fw(olt_id, llid, onu_reginfo);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_GetLLIDRdnRegisterInfo(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */

static int TK3723_DumpAllCmc(short int olt_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);

    iRlt = CMCCTRL_DumpAllCmc(dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_DumpAllCmc(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int TK3723_SetCmcServiceVID(short int olt_id, int svlan)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    if ( svlan < 0 )
    {
        svlan = -1 - svlan;
    }

    iRlt = TkBridgeSetOltCmcSVlanID(olt_id, (unsigned short)svlan);
    OLT_TK_DEBUG(OLT_TK_TITLE"TK3723_SetCmcServiceVID(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, svlan, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif

static int TK3723_SetCTCOnuTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
    short int  port_id;
    TkCtcOnuTxPowerSupplyControl tk_tx_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	VOS_MemSet(&tk_tx_info,0,sizeof(tk_tx_info));

	tk_tx_info.action = parameter->action;
	VOS_MemCpy(tk_tx_info.onuId,parameter->onu_sn,BYTES_IN_MAC_ADDRESS);
	tk_tx_info.opticalTxId = parameter->optical_id;
	
	port_id = (short int)TkChip_GetOltPonPortID(olt_id);   

	iRlt = TkCTC_SetPonPortTxPowerControl(olt_id, port_id, &tk_tx_info);

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetTxPowerSupplyControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}



/*-----------------------------外部接口----------------------------------------*/

static const OltMgmtIFs s_tkGlobalIfs = {
#if 1
/* -------------------OLT基本API------------------- */
    TK_ERROR,  /* IsExist */
    TK_ERROR,  /* GetChipTypeID */
    TK_ERROR,  /* GetChipTypeName */
    TK_ERROR,  /* ResetPon */
    TK_ERROR,  /* RemoveOlt */
    
    TK_ERROR,  /* CopyOlt */
    TK_ERROR,  /* CmdIsSupported */
    TK_OK,     /* SetDebugMode */
    TK_OK,     /* SetInitParams */
    TK_OK,     /* SetSystemParams */

    TK_ERROR,  /* SetPonI2CExtInfo */
    TK_ERROR,  /* GetPonI2CExtInfo */
    TK_ERROR,  /* SetCardI2CInfo */
    TK_ERROR,  /* GetCardI2CInfo */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK_ERROR,  /* WriteMdioRegister */

    TK_ERROR,  /* ReadMdioRegister */
    TK_ERROR,  /* ReadI2CRegister */
    TK_ERROR,  /* GpioAccess */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK_ERROR,  /* ReadGpio */
    TK_ERROR,  /* WriteGpio */

    TK_ERROR,  /* SendChipCli */
    TK_OK,     /* SetDeviceName*/
    TK_OK,     /*ResetPonChip*/
#endif

#if 1
/* -------------------OLT PON管理API--------------- */
    TK_ERROR,  /* GetVersion */
    TK_ERROR,  /* GetDBAVersion */
    TK_ERROR,  /* ChkVersion */
    TK_ERROR,  /* ChkDBAVersion */
    TK_ERROR,  /* GetCniLinkStatus */

    TK_ERROR,  /* GetPonWorkStatus */
    TK_ERROR,  /* SetAdminStatus */
    TK_ERROR,  /* GetAdminStatus */
    TK_ERROR,  /* SetVlanTpid */
    TK_ERROR,  /* SetVlanQinQ */

    TK_ERROR,  /* SetPonFrameSizeLimit */
    TK_ERROR,  /* GetPonFrameSizeLimit */
    TK_ERROR,  /* OamIsLimit */
    TK_ERROR,  /* UpdatePonParams */
    GW_SetPPPoERelayMode,

    GW_SetPPPoERelayParams,
    GW_SetDhcpRelayMode,
    NULL,      /* SetIgmpAuthMode */
    TK_ERROR,  /* SendFrame2PON */
    TK_ERROR,  /* SendFrame2CNI */

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK_ERROR,  /*GetVidDownlinkMode*/
    TK_ERROR,  /*DelVidDownlinkMode*/
    TK_ERROR,  /* GetOltParameters */
    TK_ERROR,  /* SetOltIgmpSnoopingMode */
    TK_ERROR,  /* GetOltIgmpSnoopingMode */

    TK_ERROR,  /* SetOltMldForwardingMode */
    TK_ERROR,  /* GetOltMldForwardingMode */
    TK_ERROR,  /* SetDBAReportFormat */
    TK_ERROR,  /* GetDBAReportFormat */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-04-27*/
    TK_ERROR,  /* UpdateProvBWInfo*/
    /*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    TK_ERROR,  /* LLIDIsExist */
    TK_ERROR,  /* DeregisterLLID */
    TK_ERROR,  /* GetLLIDMac */
    TK_ERROR,  /* GetLLIDRegisterInfo */
    TK_ERROR,  /* AuthorizeLLID */

    TK_ERROR,  /* SetLLIDSLA */
    TK_ERROR,  /* GetLLIDSLA */
    TK_ERROR,  /* SetLLIDPolice */
    TK_ERROR,  /* GetLLIDPolice */
    TK_ERROR,  /* SetLLIDdbaType */
    
    TK_ERROR,  /* GetLLIDdbaType */
    TK_ERROR,  /* SetLLIDdbaFlags */
    TK_ERROR,  /* GetLLIDdbaFlags */
    TK_ERROR,  /* GetLLIDHeartbeatOam */
    TK_ERROR,  /* SetLLIDHeartbeatOam */
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    TK_ERROR,  /* GetOnuNum */
    TK_ERROR,  /* GetAllOnus */
    TK_ERROR,  /* ClearAllOnus */
    TK_ERROR,  /* ResumeAllOnuStatus */
    TK_OK,     /* SetAllOnuAuthMode */

    TK_ERROR,  /* SetOnuAuthMode */
    TK_ERROR,  /* SetMacAuth */
    TK_OK,     /* SetAllOnuBindMode */
    TK_ERROR,  /* CheckOnuRegisterControl */
    TK_OK,     /* SetAllOnuDefaultBW */

    TK_OK,     /* SetAllOnuDownlinkPoliceMode */
    TK_ERROR,  /* SetOnuDownlinkPoliceMode */
    TK_OK,     /* SetAllOnuDownlinkPoliceParam */
    TK_OK,     /* SetAllOnuUplinkDBAParam */
    TK_ERROR,  /* SetOnuDownlinkPri2CoSQueueMap */

    TK_ERROR,  /* ActivePendingOnu */
    TK_ERROR,  /* ActiveOnePendingOnu */
    NULL,      /* ActiveConfPendingOnu */
    NULL,      /* ActiveOneConfPendingOnu */
    TK_ERROR,  /* GetPendingOnu */

    TK_ERROR,  /* GetUpdatingOnu */
    TK_ERROR,  /* GetUpdatedOnu */
    TK_ERROR,  /* GetOnuUpdatingStatusLocal */
    TK_ERROR,  /* SetOnuUpdateMsg */
    TK_ERROR,  /* GetOnuUpdateWaiting */

    TK_ERROR,  /* SetAllOnuAuthMode2 */
    TK_OK,     /* SetAllOnuBWParams */
    TK_ERROR,  /* SetOnuP2PMode */
    TK_ERROR,  /* GetOnuB2PMode */
    TK_ERROR,  /* SetOnuB2PMode */

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK_ERROR,  /* GetOnuMode */
    TK_ERROR,  /* GetMACAddressAuthentication */
    TK_ERROR,  /* SetAuthorizeMacAddressAccordingListMode */
    TK_ERROR,  /* GetAuthorizeMacAddressAccordingListMode */
    TK_ERROR,  /* GetDownlinkBufferConfiguration */

    TK_ERROR,  /* GetOamInformation */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
/*Begin:for onu swap by jinhl@2013-02-22*/
    TK_ERROR,  /* ResumeLLIDStatus */
    TK_ERROR,  /* SearchFreeOnuIdx */
    TK_ERROR,  /* GetOnuIdxByMac */
/*End:for onu swap by jinhl@2013-02-22*/
    TK_ERROR, /*BroadCastCliCommand*/

    TK_OK,    /*SetAuthEntry*/   
    TK_ERROR,    /*SetGponAuthEntry*/            
    TK_OK,    /*SetOnuDefaultMaxMac*/  
    TK_OK,  /*GW_SetCTCOnuPortStatsTimeOut*/
    TK_OK,    /* SetMaxOnu*/
    TK_OK,    /* GetOnuConfDelStatus*/ 
    TK_OK,	  /* SetCTCOnuTxPowerSupplyControl*/
#endif

#if 1
/* -------------------OLT 加密管理API----------- */
    TK_ERROR,  /* SetEncryptMode */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK_OK,     /* SetEncryptionPreambleMode*/
    TK_OK,     /* GetEncryptionPreambleMode*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK_ERROR,  /* GetLLIDEncryptMode */
    TK_ERROR,  /* StartLLIDEncrypt */
    
    TK_ERROR,  /* FinishLLIDEncrypt */
    TK_ERROR,  /* StopLLIDEncrypt */
    TK_ERROR,  /* SetLLIDEncryptKey */
    TK_ERROR,  /* FinishLLIDEncryptKey */
#endif

#if 1
/* -------------------OLT 地址表管理API-------- */
    TK_ERROR,  /* SetMacAgeingTime */
    TK_ERROR,  /* SetAddrTblCfg */                      
    TK_ERROR,  /* GetAddrTblCfg */                      
    TK_ERROR,  /* GetMacAddrTbl */
    NULL,      /* AddMacAddrTbl */

    NULL,      /* DelMacAddrTbl */
    TK_ERROR,  /* RemoveMac */
    TK_ERROR,  /* ResetAddrTbl */
    TK_OK,     /*SetOnuMacThreshold*/
    GW_OnuMacCheckEnable,
    
    GW_OnuMacCheckPeriod,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK_ERROR,   /*SetAddressTableFullHandlingMode*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK_ERROR,
    TK_ERROR,/*GetMacAddrVlanTbl */
#endif

#if 1
/* -------------------OLT 光路管理API----------- */
    TK_ERROR,  /* GetOpticalCapability */
    TK_ERROR,  /* GetOpticsDetail */
    TK_ERROR,  /* SetPonRange */
    TK_ERROR,  /* SetOpticalTxMode */
    TK_ERROR,  /* GetOpticalTxMode */

    TK_ERROR,    /*SetVirtualScopeAdcConfig*/
    TK_ERROR,    /*GetVirtualScopeMeasurement*/
    TK_ERROR,    /*GetVirtualScopeOnuVoltage*/
    TK_ERROR,    /*SetVirtualLLID*/
#endif

#if 1
/* -------------------OLT 监控统计管理API---- */
    TK_ERROR,  /* GetRawStatistics */
    TK_ERROR,  /* ResetCounters */
    TK_ERROR,  /* SetBerAlarm */
    TK_ERROR,  /* SetFerAlarm */
    TK_ERROR,  /* SetPonBerAlarm */                      

    TK_ERROR,  /* SetPonFerAlarm */                      
    TK_OK,     /* SetBerAlarmParams */
    TK_OK,     /* SetFerAlarmParams */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK_ERROR,  /*SetAlarmConfig*/
    TK_ERROR,  /*GetAlarmConfig*/
    
    TK_ERROR,  /*GetStatistics*/
    TK_ERROR,  /*OltSelfTest*/
    TK_ERROR,  /*LinkTest*/
    TK_ERROR,  /* SetLLIDFecMode */
    TK_ERROR,  /* GetLLIDFecMode */

    TK_ERROR,  /*SysDump*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------OLT 倒换API---------------- */
    TK_ERROR,  /* GetHotSwapCapability */
    TK_ERROR,  /* GetHotSwapMode */
    TK_ERROR,  /* SetHotSwapMode */
    TK_ERROR,  /* ForceHotSwap */
    GW_SetHotSwapParam,

    TK_ERROR,  /* RdnOnuRegister */
    TK_ERROR,  /* SetRdnConfig */
    TK_ERROR,  /* RdnSwitchOver */
    TK_ERROR,  /* RdnIsExist */
    TK_ERROR,  /* ResetRdnOltRecord */

    TK_ERROR,  /* GetRdnState */
    TK_ERROR,  /* SetRdnState */
    TK_ERROR,  /* GetLLIDRdnDB */
    TK_ERROR,  /* SetLLIDRdnDB */
    TK_ERROR,  /* GetRdnAddrTbl */

    TK_ERROR,  /* RemoveRdnOlt */
    TK_ERROR,  /* RdnRemoveOlt */
    TK_ERROR,  /* RdnSwapOlt */
    TK_ERROR,  /* AddSwapOlt */
    TK_ERROR,  /* RdnLooseOlt */

    /*Begin:for onu swap by jinhl@2013-02-22*/
    TK_ERROR,  /*RdnLLIDAdd*/
    TK_ERROR,  /*GetRdnLLIDMode*/
    TK_ERROR,  /*SetRdnLLIDMode*/
    TK_ERROR,  /*SetRdnLLIDStdbyToAct*/
    TK_ERROR,  /*SetRdnLLIDRtt*/
    /*End:for onu swap by jinhl@2013-02-22*/
    
    TK_ERROR,  /* RdnIsReady */
    TK_ERROR,  /* GetLLIDRdnRegisterInfo */
#endif

#if 1
/* --------------------OLT CMC管理API------------------- */
    TK_ERROR,  /* DumpAllCmc */
    NULL,      /* SetCmcServiceVID */
#endif

    TK_ERROR   /* LastFun */
};



static const OltMgmtIFs s_tk3723Ifs = {
#if 1
/* -------------------OLT基本API------------------- */
    TK_IsExist,
    TK3723_GetChipTypeID,
    TK3723_GetChipTypeName,
    TK3723_ResetPon,
    TK3723_RemoveOlt,
    
    GW_CopyOlt,
    GW_CmdIsSupported,
    TK_OK,    /* SetDebugMode */
    TK_SetInitParams,
    TK_SetSystemParams,

    GW_SetPonI2CExtInfo,
    GW_GetPonI2CExtInfo,
    GW_SetCardI2CInfo,
    GW_GetCardI2CInfo,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK3723_WriteMdioRegister,

    TK3723_ReadMdioRegister,
    TK3723_ReadI2CRegister,
    NULL,     /*GpioAccess*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK3723_ReadGpio,
    TK3723_WriteGpio,

    TK3723_SendChipCli,
    TK_OK,    /* SetDeviceName*/
    TK_OK,    /*ResetPonChip*/
#endif

#if 1
/* -------------------OLT PON管理API--------------- */
    TK3723_GetVersion,
    TK3723_GetDBAVersion,
    TK3723_ChkVersion,
    GW_ChkDBAVersion,    /* ChkDBAVersion */
    TK3723_GetCniLinkStatus,

    GW_GetPonWorkStatus, 
    TK3723_SetAdminStatus,
    GW_GetAdminStatus,   
    TK3723_SetVlanTpid,
    TK3723_SetVlanQinQ,

    NULL,     /* SetPonFrameSizeLimit */
    NULL,     /* GetPonFrameSizeLimit */
    TK3723_OamIsLimit,
    TK3723_UpdatePonParams,
    GW_SetPPPoERelayMode,

    GW_SetPPPoERelayParams,
    GW_SetDhcpRelayMode,
    TK3723_SetIgmpAuthMode,
    TK3723_SendFrame2PON,
    TK3723_SendFrame2CNI,

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,     /*GetVidDownlinkMode*/
    TK3723_DelVidDownlinkMode,
    NULL,     /*GetOltParameters*/
    NULL,     /*SetOltIgmpSnoopingMode*/
    NULL,     /*GetOltIgmpSnoopingMode*/

    NULL,     /*SetOltMldForwardingMode*/
    NULL,     /*GetOltMldForwardingMode*/
    NULL,     /*SetDBAReportFormat*/
    NULL,     /*GetDBAReportFormat*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-04-27*/
    GW_UpdateProvBWInfo, 
    /*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    TK3723_LLIDIsExist,
    TK3723_DeregisterLLID,
    TK3723_GetLLIDMac,
    TK3723_GetLLIDRegisterInfo,
    TK3723_AuthorizeLLID,

    TK3723_SetLLIDSLA,
    TK3723_GetLLIDSLA,
    TK3723_SetLLIDPolice,
    TK3723_GetLLIDPolice,
    TK3723_SetLLIDdbaType,
    
    TK3723_GetLLIDdbaType,
    TK3723_SetLLIDdbaFlags,
    TK3723_GetLLIDdbaFlags,
    NULL,     /* GetLLIDHeartbeatOam */
    NULL,     /* SetLLIDHeartbeatOam */
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    GW_GetOnuNum,
    TK3723_GetAllOnus,
    TK_OK,    /* ClearAllOnus */
    TK3723_ResumeAllOnuStatus,
    TK_OK,    /* SetAllOnuAuthMode */

    TK3723_SetOnuAuthMode,
    TK3723_SetMacAuth,
    TK_OK,    /* SetAllOnuBindMode */
    TK_ERROR, /* CheckOnuRegisterControl */
    TK_OK,    /* SetAllOnuDefaultBW */

    TK_OK,    /* SetAllOnuDownlinkPoliceMode */
    TK_OK,    /* SetOnuDownlinkPoliceMode */
    TK_OK,    /* SetAllOnuDownlinkPoliceParam */
    TK_OK,    /* SetAllOnuUplinkDBAParam */
    TK3723_SetOnuDownlinkPri2CoSQueueMap,

    TK3723_ActivePendingOnu,
    TK3723_ActiveOnePendingOnu,
    TK3723_ActiveConfPendingOnu,
    TK3723_ActiveOneConfPendingOnu,
    GW_GetPendingOnu,

    GW_GetUpdatingOnu,
    GW_GetUpdatedOnu,
    GW_GetOnuUpdatingStatusLocal,
    GW_SetOnuUpdateMsg,
    GW_GetOnuUpdateWaiting,

    TK3723_SetAllOnuAuthMode2,
    TK_OK,    /* SetAllOnuBWParams */
    TK3723_SetOnuP2PMode,
    TK3723_GetOnuB2PMode,
    TK3723_SetOnuB2PMode,

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK3723_GetOnuMode,
    NULL,     /* GetMACAddressAuthentication */
    NULL,     /* SetAuthorizeMacAddressAccordingListMode */
    NULL,     /* GetAuthorizeMacAddressAccordingListMode */
    NULL,     /* GetDownlinkBufferConfiguration */

    NULL,     /* GetOamInformation */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
/*Begin:for onu swap by jinhl@2013-02-22*/
    NULL,     /* ResumeLLIDStatus */
    GW_SearchFreeOnuIdx,
    GW_GetActOnuIdxByMac,
/*End:for onu swap by jinhl@2013-02-22*/
    GW_BroadCast_CliCommand,

    TK_OK,    /*SetAuthEntry*/   
    TK_ERROR,    /*SetGponAuthEntry*/                
    TK_OK,    /*SetOnuDefaultMaxMac*/ 
    TK_OK,  /*GW_SetCTCOnuPortStatsTimeOut*/
    TK_OK,    /* SetMaxOnu*/
    TK_OK,    /* GetOnuConfDelStatus*/ 
    TK3723_SetCTCOnuTxPowerSupplyControl,/*SetCTCOnuTxPowerSupplyControl*/
#endif

#if 1
/* -------------------OLT 加密管理API----------- */
    TK3723_SetEncryptMode,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,     /* SetEncryptionPreambleMode */
    NULL,     /* GetEncryptionPreambleMode */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK3723_GetLLIDEncryptMode,
    TK3723_StartLLIDEncrypt,
    
    TK3723_FinishLLIDEncrypt,
    TK3723_StopLLIDEncrypt,
    TK3723_SetLLIDEncryptKey,
    TK3723_FinishLLIDEncryptKey,
#endif

#if 1
/* -------------------OLT 地址表管理API-------- */
    TK3723_SetMacAgingTime,
    NULL,     /* SetAddressTableConfig */
    TK3723_GetAddressTableConfig,
    TK3723_GetMacAddrTbl,
    TK3723_AddMacAddrTbl,

    TK3723_DelMacAddrTbl,
    TK3723_RemoveMac,
    TK3723_ResetAddrTbl,
    TK_OK,    /* SetOnuMacThreshold */
    GW_OnuMacCheckEnable,
    
    GW_OnuMacCheckPeriod,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,     /* SetAddressTableFullHandlingMode */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK3723_GetLlidByUserMacAddress,
    TK_ERROR,/*GetMacAddrVlanTbl */
#endif

#if 1
/* -------------------OLT 光路管理API----------- */
    TK3723_GetOpticalCapability,
    TK3723_GetOpticsDetail,
    TK3723_SetPonRange,
    TK3723_SetOpticalTxMode,
    TK3723_GetOpticalTxMode,

    TK3723_SetVirtualScopeAdcConfig,
    TK3723_GetVirtualScopeMeasurement,
    TK3723_GetVirtualScopeRssiMeasurement,
    TK3723_GetVirtualScopeOnuVoltage,
    TK_OK,    /* SetVirtualLLID */
    
    TK3723_SetOpticalTxMode2,
#endif

#if 1
/* -------------------OLT 监控统计管理API---- */
    TK3723_GetRawStatistics,
    TK3723_ResetCounters,
    TK3723_SetBerAlarm,
    TK3723_SetFerAlarm,
    TK_OK,    /* SetPonBerAlarm */                      

    TK_OK,    /* SetPonFerAlarm */                      
    TK_OK,    /* SetBerAlarmParams */
    TK_OK,    /* SetFerAlarmParams */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK3723_SetAlarmConfig,
    TK3723_GetAlarmConfig,
    
    TK3723_GetStatistics,
    TK3723_OltSelfTest,
    TK3723_LinkTest,
    TK3723_SetLLIDFecMode,
    TK3723_GetLLIDFecMode,

    NULL,     /* SysDump */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif
    
#if 1
/* -------------------OLT 倒换API---------------- */
    TK3723_GetHotSwapCapability,
    TK3723_GetHotSwapMode,
    TK3723_SetHotSwapMode,
    TK3723_ForceHotSwap,
    GW_SetHotSwapParam,

    TK3723_RdnOnuRegister,
    TK3723_SetRdnConfig,
    TK3723_RdnSwitchOver,
    TK3723_RdnIsExist,
    TK3723_ResetRdnRecord,

    TK3723_GetRdnState,
    TK3723_SetRdnState,
    TK3723_GetRdnAddrTbl,
    TK3723_RemoveRdnOlt,
    TK3723_GetLLIDRdnDB,

    TK3723_SetLLIDRdnDB,
    TK3723_RdnRemoveOlt,
    TK3723_RdnSwapOlt,
    TK3723_AddSwapOlt,
    TK3723_RdnLooseOlt,

    /*Begin:for onu swap by jinhl@2013-02-22*/
    TK_ERROR,     /*RdnLLIDAdd*/
    TK_ERROR,     /*GetRdnLLIDMode*/
    TK_ERROR,     /*SetRdnLLIDMode*/
    TK_ERROR,     /*SetRdnLLIDStdbyToAct*/
    TK_ERROR,     /*SetRdnLLIDRtt*/
    /*End:for onu swap by jinhl@2013-02-22*/

    GW_RdnIsReady,
    TK3723_GetLLIDRdnRegisterInfo,
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */
    TK3723_DumpAllCmc,
    TK3723_SetCmcServiceVID,
#endif

    TK_ERROR  /* LastFun */
};

static const OltMgmtIFs s_bcm55524Ifs = {
#if 1
/* -------------------OLT基本API------------------- */
    TK_IsExist,
    BCM55524_GetChipTypeID,
    BCM55524_GetChipTypeName,
    TK3723_ResetPon,
    TK3723_RemoveOlt,

    GW_CopyOlt,
    GW_CmdIsSupported,
    TK_OK,    /* SetDebugMode */
    TK_SetInitParams,
    TK_SetSystemParams,

    GW_SetPonI2CExtInfo,
    GW_GetPonI2CExtInfo,
    GW_SetCardI2CInfo,
    GW_GetCardI2CInfo,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK3723_WriteMdioRegister,

    TK3723_ReadMdioRegister,
    TK3723_ReadI2CRegister,
    NULL,     /*GpioAccess*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK3723_ReadGpio,
    TK3723_WriteGpio,

    TK3723_SendChipCli,
    TK_OK,     /* SetDeviceName*/
    TK_OK,     /*ResetPonChip*/
#endif

#if 1
/* -------------------OLT PON管理API--------------- */
    TK3723_GetVersion,
    TK3723_GetDBAVersion,
    TK3723_ChkVersion,
    GW_ChkDBAVersion,    /* ChkDBAVersion */
    TK3723_GetCniLinkStatus,

    GW_GetPonWorkStatus, 
    TK3723_SetAdminStatus,
    GW_GetAdminStatus,   
    TK3723_SetVlanTpid,
    TK3723_SetVlanQinQ,

    NULL,     /* SetPonFrameSizeLimit */
    TK3723_GetPonFrameSizeLimit,/*NULL,TK3723_GetPonFrameSizeLimit,*/
    TK3723_OamIsLimit,
    TK3723_UpdatePonParams,
    GW_SetPPPoERelayMode,

    GW_SetPPPoERelayParams,
    GW_SetDhcpRelayMode,
    TK3723_SetIgmpAuthMode,
    TK3723_SendFrame2PON,
    TK3723_SendFrame2CNI,

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,     /*GetVidDownlinkMode*/
    TK3723_DelVidDownlinkMode,
    NULL,     /*GetOltParameters*/
    NULL,     /*SetOltIgmpSnoopingMode*/
    NULL,     /*GetOltIgmpSnoopingMode*/

    NULL,     /*SetOltMldForwardingMode*/
    NULL,     /*GetOltMldForwardingMode*/
    NULL,     /*SetDBAReportFormat*/
    NULL,     /*GetDBAReportFormat*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-04-27*/
	GW_UpdateProvBWInfo, 
    /*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    TK3723_LLIDIsExist,
    TK3723_DeregisterLLID,
    TK3723_GetLLIDMac,
    TK3723_GetLLIDRegisterInfo,
    TK3723_AuthorizeLLID,

    TK3723_SetLLIDSLA,
    TK3723_GetLLIDSLA,
    TK3723_SetLLIDPolice,
    TK3723_GetLLIDPolice,
    TK3723_SetLLIDdbaType,
    
    TK3723_GetLLIDdbaType,
    TK3723_SetLLIDdbaFlags,
    TK3723_GetLLIDdbaFlags,
    NULL,     /* GetLLIDHeartbeatOam */
    NULL,     /* SetLLIDHeartbeatOam */
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    GW_GetOnuNum,
    TK3723_GetAllOnus,
    TK_OK,    /* ClearAllOnus */
    BCM55524_ResumeAllOnuStatus,
    TK_OK,    /* SetAllOnuAuthMode */

    TK3723_SetOnuAuthMode,
    TK3723_SetMacAuth,
    TK_OK,    /* SetAllOnuBindMode */
    TK_ERROR, /* CheckOnuRegisterControl */
    TK_OK,    /* SetAllOnuDefaultBW */

    TK_OK,    /* SetAllOnuDownlinkPoliceMode */
    TK_OK,    /* SetOnuDownlinkPoliceMode */
    TK_OK,    /* SetAllOnuDownlinkPoliceParam */
    TK_OK,    /* SetAllOnuUplinkDBAParam */
    TK3723_SetOnuDownlinkPri2CoSQueueMap,

    TK3723_ActivePendingOnu,
    TK3723_ActiveOnePendingOnu,
    TK3723_ActiveConfPendingOnu,
    TK3723_ActiveOneConfPendingOnu,
    GW_GetPendingOnu,

    GW_GetUpdatingOnu,
    GW_GetUpdatedOnu,
    GW_GetOnuUpdatingStatusLocal,
    GW_SetOnuUpdateMsg,
    GW_GetOnuUpdateWaiting,

    TK3723_SetAllOnuAuthMode2,
    TK_OK,    /* SetAllOnuBWParams */
    TK3723_SetOnuP2PMode,
    TK3723_GetOnuB2PMode,
    TK3723_SetOnuB2PMode,

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK3723_GetOnuMode,
    NULL,     /*GetMACAddressAuthentication */
    NULL,     /*SetAuthorizeMacAddressAccordingListMode */
    NULL,     /*GetAuthorizeMacAddressAccordingListMode */
    NULL,     /*GetDownlinkBufferConfiguration */

    NULL,     /*GetOamInformation */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
/*Begin:for onu swap by jinhl@2013-02-22*/
    NULL,     /*ResumeLLIDStatus*/
    GW_SearchFreeOnuIdx,
    GW_GetActOnuIdxByMac,
/*End:for onu swap by jinhl@2013-02-22*/
    GW_BroadCast_CliCommand,

    TK_OK,    /*SetAuthEntry*/   
    TK_ERROR,    /*SetGponAuthEntry*/                
    TK_OK,    /*SetOnuDefaultMaxMac*/   
    TK_OK,    /*GW_SetCTCOnuPortStatsTimeOut*/
    TK_OK,    /* SetMaxOnu*/
    TK_OK,    /* GetOnuConfDelStatus*/   
    TK3723_SetCTCOnuTxPowerSupplyControl,/*SetCTCOnuTxPowerSupplyControl*/
#endif

#if 1
/* -------------------OLT 加密管理API----------- */
    TK3723_SetEncryptMode,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,     /*SetEncryptionPreambleMode*/
    NULL,     /*GetEncryptionPreambleMode*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK3723_GetLLIDEncryptMode,
    TK3723_StartLLIDEncrypt,
    
    TK3723_FinishLLIDEncrypt,
    TK3723_StopLLIDEncrypt,
    TK3723_SetLLIDEncryptKey,
    TK3723_FinishLLIDEncryptKey,
#endif

#if 1
/* -------------------OLT 地址表管理API-------- */
    TK3723_SetMacAgingTime,
    NULL,     /* SetAddressTableConfig */
    TK3723_GetAddressTableConfig,
    TK3723_GetMacAddrTbl,
    TK3723_AddMacAddrTbl,

    TK3723_DelMacAddrTbl,
    TK3723_RemoveMac,
    TK3723_ResetAddrTbl,
    TK_OK,    /*SetOnuMacThreshold*/
    GW_OnuMacCheckEnable,
    
    GW_OnuMacCheckPeriod,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,     /*SetAddressTableFullHandlingMode*/
   /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK3723_GetLlidByUserMacAddress,
    TK_ERROR,/*GetMacAddrVlanTbl */
#endif

#if 1
/* -------------------OLT 光路管理API----------- */
    TK3723_GetOpticalCapability,
    TK3723_GetOpticsDetail,
    TK3723_SetPonRange,
    TK3723_SetOpticalTxMode,
    TK3723_GetOpticalTxMode,

    TK3723_SetVirtualScopeAdcConfig,
    TK3723_GetVirtualScopeMeasurement,
    TK3723_GetVirtualScopeRssiMeasurement,
    TK3723_GetVirtualScopeOnuVoltage,
    TK_OK,    /* SetVirtualLLID */
    
    TK3723_SetOpticalTxMode2,
#endif

#if 1
/* -------------------OLT 监控统计管理API---- */
    TK3723_GetRawStatistics,
    TK3723_ResetCounters,
    TK3723_SetBerAlarm,
    TK3723_SetFerAlarm,
    TK_OK,    /* SetPonBerAlarm */                      

    TK_OK,    /* SetPonFerAlarm */                      
    TK_OK,    /* SetBerAlarmParams */
    TK_OK,    /* SetFerAlarmParams */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    TK3723_SetAlarmConfig,
    TK3723_GetAlarmConfig,
    
    TK3723_GetStatistics,
    TK3723_OltSelfTest,
    TK3723_LinkTest,
    TK3723_SetLLIDFecMode,
    TK3723_GetLLIDFecMode,

    NULL,     /* SysDump */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif
    
#if 1
/* -------------------OLT 倒换API---------------- */
    TK3723_GetHotSwapCapability,
    TK3723_GetHotSwapMode,
    TK3723_SetHotSwapMode,
    TK3723_ForceHotSwap,
    GW_SetHotSwapParam,

    TK3723_RdnOnuRegister,
    TK3723_SetRdnConfig,
    TK3723_RdnSwitchOver,
    TK3723_RdnIsExist,
    TK3723_ResetRdnRecord,

    TK3723_GetRdnState,
    TK3723_SetRdnState,
    TK3723_GetRdnAddrTbl,
    TK3723_RemoveRdnOlt,
    TK3723_GetLLIDRdnDB,

    TK3723_SetLLIDRdnDB,
    TK3723_RdnRemoveOlt,
    TK3723_RdnSwapOlt,
    TK3723_AddSwapOlt,
    TK3723_RdnLooseOlt,
    
    /*Begin:for onu swap by jinhl@2013-02-22*/
    TK_ERROR,     /*RdnLLIDAdd*/
    TK_ERROR,     /*GetRdnLLIDMode*/
    TK_ERROR,     /*SetRdnLLIDMode*/
    TK_ERROR,     /*SetRdnLLIDStdbyToAct*/
    TK_ERROR,     /*SetRdnLLIDRtt*/
    /*End:for onu swap by jinhl@2013-02-22*/

    GW_RdnIsReady,
    TK3723_GetLLIDRdnRegisterInfo,
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */
    TK3723_DumpAllCmc,
    TK3723_SetCmcServiceVID,
#endif

    TK_ERROR  /* LastFun */
};


void OLT_Tk3723_Support()
{
    OLT_RegisterAdapter(OLT_ADAPTER_TK3723, &s_tk3723Ifs);
    g_cszPonChipTypeNames[PONCHIP_TK3723] = "TK3723";
}

void OLT_Bcm55524_Support()
{
    OLT_RegisterAdapter(OLT_ADAPTER_BCM55524, &s_bcm55524Ifs);
    g_cszPonChipTypeNames[PONCHIP_BCM55524] = "BCM55524";
}

void OLT_TkGlobal_Support()
{
    OLT_RegisterAdapter(OLT_ADAPTER_GLOBAL, &s_tkGlobalIfs);
}


#ifdef __cplusplus

}

#endif


