/***************************************************************
*
*						Module Name:  OltIfAdapter_Gpon.c
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
*   Date: 			2015/09/11
*   Author:		jinhl
*   content:
**  History:
**   Date        |    Name       |     Description
**---- ----- |-----------|------------------ 
**  15/09/11  |   jinhl    |     create 
**----------|-----------|------------------
***************************************************************/

#if defined(_GPON_BCM_SUPPORT_)            

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
#include  "includeFromGpon.h"
#include  "ponEventHandler.h"
#include  "../../driver/pon/gpon/gponadp/include/GponOnuAdp.h"

#include  "../../monitor/monitor.h"

extern char *g_cszPonChipTypeNames[PONCHIP_TYPE_MAX];    
extern long double g_afLevelConst[16];
extern ponAlarmInfo		**gpOnuAlarm;
extern ponThreasholdInfo gPonThreashold;

extern  int OnuMacCheckEnable(ULONG  enable);


/*-----------------------------内部适配----------------------------------------*/

static int GPON_OK(short int olt_id)
{
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_OK(%d)'s result(0).\r\n", olt_id);

    return OLT_ERR_OK;
}

static int GPON_ERROR(short int olt_id)
{
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ERROR(%d)'s result(%d).\r\n", olt_id, OLT_ERR_NOTEXIST);

    debug_stub();

    VOS_ASSERT(0);

    return OLT_ERR_NOTEXIST;
}

#if 1
/* -------------------OLT基本API------------------- */

static int GPON_IsExist(short int olt_id, bool *status)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(status); 
    *status = gponOltAdp_IsExist(olt_id);
    return OLT_ERR_OK;
}

static int GPON_GetChipTypeID(short int olt_id, int *type)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(type);

    *type = PONCHIP_GPON;
    return OLT_ERR_OK;
}

static int GPON_GetChipTypeName(short int olt_id, char typename[OLT_CHIP_NAMELEN])
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(typename);
    VOS_ASSERT(g_cszPonChipTypeNames[PONCHIP_GPON]);

    VOS_MemZero(typename, OLT_CHIP_NAMELEN);
    VOS_StrnCpy(typename, g_cszPonChipTypeNames[PONCHIP_GPON], OLT_CHIP_NAMELEN - 1);
    return OLT_ERR_OK;
}

int resetGpon = 0;
extern void GW_DevResetHard1();
extern void GW_DevResetHard2();
extern void ocsResetDataPerDevice(void);

static int GPON_ResetPon(short int olt_id)
{
	int i = 0;
    OLT_LOCAL_ASSERT(olt_id);
	#if 0
	if(resetGpon)
	{
		GW_GPONTxEnable(olt_id, 0);
	    gponOltAdp_Reset(olt_id);
		GW_GPONTxEnable(olt_id, 1);
	}
	#endif
	#if 1/*GPON复位一个PON口为PON的frimware复位，复位完后直接激活就可以，不需要重新加载PON*/
	
	if(0 == olt_id)
	{
		gponOltAdp_DevDisconnect();
		for(i = 0; i < 16; i++)
		{
			GW_GPONTxEnable(i, 0);
			gponOltAdp_Reset(i);
		}
		
		GW_DevResetHard1();
		GW_DevResetHard2();
		gponOltAdp_DevReInit();
        ocsResetDataPerDevice();
	}
	GW_ResetPon(olt_id);
	#endif

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ResetPon(%d)'s result(%d) on slot %d.\r\n", olt_id, 0, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

static int GPON_ResetPonChip(short int olt_id)
{
	int i = 0;
    OLT_LOCAL_ASSERT(olt_id);
	
	
	if(0 == olt_id)
	{
		gponOltAdp_DevDisconnect();
		for(i = 0; i < 16; i++)
		{
			GW_GPONTxEnable(i, 0);
			gponOltAdp_Reset(i);
		}
		
		GW_DevResetHard1();
		GW_DevResetHard2();
		gponOltAdp_DevReInit();
        ocsResetDataPerDevice();

		for(i = 0; i < 16; i++)
		{
			GW_ResetPon(i);
		}
		
	}
	
	

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ResetPonChip(%d)'s result(%d) on slot %d.\r\n", olt_id, 0, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

static int GPON_RemoveOlt(short int olt_id, bool send_shutdown_msg_to_olt, bool reset_olt)
{
    OLT_LOCAL_ASSERT(olt_id);

    gponOltAdp_RemoveOlt(olt_id, send_shutdown_msg_to_olt, reset_olt);


    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_RemoveOlt(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, send_shutdown_msg_to_olt, reset_olt, 0, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

int GPON_SetInitParams(short int olt_id, unsigned short host_olt_manage_type, unsigned short host_olt_manage_address)
{
    int iRlt;

    OLT_LOCALID_ASSERT(olt_id);
   	/*GPON 68620 的PCIE基地址是由BSP获取的，这里不进行设置*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetInitParams(%d, %d, 0x%x)'s result(%d) on slot %d.\r\n", olt_id, host_olt_manage_type, host_olt_manage_address, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return OLT_ERR_OK;
}

int GPON_SetSystemParams(short int olt_id, long int statistics_sampling_cycle, long int monitoring_cycle, short int host_olt_msg_timeout, short int olt_reset_timeout)
{
    int iRlt;

    OLT_LOCALID_ASSERT(olt_id);

	/*一期不处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetSystemParams(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, statistics_sampling_cycle, monitoring_cycle, host_olt_msg_timeout, olt_reset_timeout, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GPON_WriteMdioRegister(short int olt_id, short int phy_address, short int reg_address, unsigned short int value )
{
    int iRlt;
    /*GPON不支持*/
    OLT_LOCAL_ASSERT(olt_id);
	return OLT_ERR_NOTSUPPORT;
}

static int GPON_ReadMdioRegister(short int olt_id, short int phy_address, short int reg_address, unsigned short int *value )
{
    int iRlt;
    unsigned long ulValue; 
    /*GPON不支持*/
    OLT_LOCAL_ASSERT(olt_id);

	return OLT_ERR_NOTSUPPORT;
}

static int GPON_ReadI2CRegister(short int olt_id, short int device, short int register_address, short int *data )
{
    int iRlt;
    unsigned short numBytesToRead;
    unsigned char ucBusId;
    unsigned char ucRegAddr;
    unsigned char ucRegVal;
    UINT slot,port;
    UCHAR buf;

    #define I2C_BASE_SFP 0x00
    #define I2C_BASE_SFP2 0xc0
	
    OLT_LOCAL_ASSERT(olt_id);

    slot = GetCardIdxByPonChip(olt_id);
    port = GetPonPortByPonChip(olt_id);
	
    if(device == A0H_1010000X )
    {
        iRlt = i2c_data_get(slot,I2C_BASE_SFP + port -1,register_address,&buf,1);
	 *data = (short int)buf;
        if(iRlt == FALSE)
        {
              return OLT_ERR_UNKNOEWN;
	  }
    }
    else if(device == A2H_1010001X)
    {
        iRlt = i2c_data_get(slot,I2C_BASE_SFP2 + port - 1,register_address,&buf,1);
	 *data = (short int)buf;
	 if(iRlt == FALSE)
        {
              return OLT_ERR_UNKNOEWN;
	  }
    }
	
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ReadI2CRegister(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, device, register_address, *data, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return OLT_ERR_OK;
}

static int GPON_GetGpioFuncId(short int olt_id, int func_id, int gpio_dir)
{
    int line_number = OLT_ERR_OK;
	/*暂不支持*/
    return line_number;
}

static int GPON_ReadGpio(short int olt_id, int func_id, bool *value)
{
    int iRlt;
   
	OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(value);
	/*暂不支持*/
	return OLT_ERR_OK;
}

static int GPON_WriteGpio(short int olt_id, int func_id, bool value)
{
    int iRlt;
   
	OLT_ASSERT(olt_id);
	/*暂不支持*/
	return OLT_ERR_OK;
}

static int GPON_SendChipCli(short int olt_id, unsigned short size, unsigned char *command)
{
    int iRlt;
    unsigned char cli_enabled;
   
    OLT_ASSERT(olt_id);
	/*GPON芯片不支持*/
	return OLT_ERR_NOTSUPPORT;
}
#endif


#if 1
/* -------------------OLT PON管理API--------------- */

static int GPON_GetVersion(short int olt_id, PON_device_versions_t *device_versions)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(device_versions);
   
	iRlt = gponOltAdp_GetVersion(olt_id, device_versions);
	
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetVersion(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GPON_GetDBAVersion(short int olt_id, OLT_DBA_version_t *dba_version)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(dba_version);
    
	iRlt = gponOltAdp_GetDbaVersion( olt_id, dba_version );
	
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetDBAVersion(%d, %s, %s)'s result(%d) on slot %d.\r\n", olt_id, dba_version->szDBAname, dba_version->szDBAversion, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_ChkVersion(short int olt_id, bool *is_compatibled)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(is_compatibled);
    /*不需要*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ChkVersion(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *is_compatibled, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    *is_compatibled = TRUE;
	return OLT_ERR_OK;
}

static int GPON_SetAdminStatus(short int olt_id, int admin_status)
{
    int iRlt;
    bool tx_mode;
        
    OLT_LOCAL_ASSERT(olt_id);
	tx_mode = (V2R1_ENABLE == admin_status) ? 1 : 0;
	
	#if 0
	
	iRlt = gponOltAdp_SetAdmin(olt_id, tx_mode);
	#endif
	iRlt = GW_GPONTxEnable(olt_id, tx_mode);

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetAdminStatus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, admin_status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


/*33103,FW 1.9.1.1 支持获取CNI/nni的up/down by jinhl@2016.11.07*/
static int GPON_GetCniLinkStatus(short int olt_id, bool *status)
{
    int iRlt = OLT_ERR_PARAM;
	OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(status);
	
	iRlt = gponOltAdp_GetCniLinkStatus(olt_id, status);
	
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetCniLinkStatus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int GPON_SetVlanTpid(short int olt_id, unsigned short int vlan_tpid)
{
    int iRlt;
    unsigned short int tpid_outer, tpid_inner;
        
    OLT_LOCAL_ASSERT(olt_id);

	/*当前版本不设置*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetVlanTpid(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, vlan_tpid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return OLT_ERR_OK;
}


ULONG UNDO_UPLINK = 1;
ULONG UNDO_DOWNLINK  = 1;
ULONG GPON_QINQ_DEBUG = 0;
extern int vlanportid[16][2000];
extern LONG devsm_sys_is_switchhovering();
static int GPON_SetVlanQinQ(short int olt_id, OLT_vlan_qinq_t *vlan_qinq_config)
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
	int i;
    PON_olt_vlan_uplink_config_t *pQinQCfg;
    PON_vlan_tag_t newvlan = 0;
	PON_vlan_tag_t prevlan = 0;
	Olt_llid_vlan_manipulation vlan_qinq_policy;
	int preoriention = 0;
	
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(vlan_qinq_config);

    brdIdx = GetCardIdxByPonChip(olt_id);
    ponIdx = GetPonPortByPonChip(olt_id);

	
	unit = USER_PORT_2_SWITCH_UNIT(brdIdx, ponIdx);
	port = USER_PORT_2_SWITCH_PORT(brdIdx, ponIdx);
	if(0xff==unit || 0xff==port)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n%s, brdIdx:%d, switchoverring:%d \r\n",__FUNCTION__,  brdIdx, devsm_sys_is_switchhovering);}
	if(devsm_sys_is_switchhovering())
	{
		return VOS_OK;
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
		gemvid[0] = 200 + (llid-1)*16;

		if(GPON_QINQ_DEBUG) {sys_console_printf("\r\n &gemvid:0x%x\r\n", gemvid);}

		
		if(vlan_qinq_config->qinq_cfg.up_cfg.vlan_manipulation == PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED)
		{
			GetPonUplinkVlanQinQ(olt_id, onuIdx, &vlan_qinq_policy);
			prevlan = vlan_qinq_policy.new_vlan_id;
			preoriention = vlan_qinq_policy.vlan_manipulation;
			
			if(GPON_QINQ_DEBUG) {sys_console_printf("\r\ line:%d, nslot = %d,port = %d,prevlan = %d, prepolicy = %d \r\n",__LINE__, brdIdx,ponIdx,prevlan,preoriention);}
			
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
				if(GPON_QINQ_DEBUG) {sys_console_printf("\r\nqinq_set u:%d, p:%d, gvid:%d, nvid:%d\r\n",unit, port, gemvid[i], (int)newvlan);}
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
					if(GPON_QINQ_DEBUG) {sys_console_printf("\r\nundo uplink  u:%d, p:%d, gvid:%d, nvid:%d\r\n",unit, port, gemvid[i], (int)newvlan);}
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
		GetPonDownlinkVlanQinQ(olt_id, onuIdx, &vlan_qinq_policy);
		newvlan = vlan_qinq_policy.new_vlan_id;

		if(vlan_qinq_config->qinq_cfg.up_cfg.vlan_manipulation  == PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG)
		{
			iRlt = OLT_ERR_OK;
		}
		else if(vlan_qinq_config->qinq_cfg.up_cfg.vlan_manipulation == PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION)
		{
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




static int GPON_UpdatePonParams(short int olt_id, int max_range, int mac_agetime)
{
    int iRlt = 0;
	#if 0
	iRlt = gponOltAdp_SetMaxRange(olt_id, max_range);
	#endif
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_UpdatePonParams(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, max_range, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_SetIgmpAuthMode(short int olt_id, int auth_mode)
{
    int iRlt = OLT_ERR_OK;
    PON_pon_network_traffic_direction_t pkt_dir;
        
    OLT_LOCAL_ASSERT(olt_id);

    if ( auth_mode < 0 )
    {
        auth_mode = -auth_mode;
    }

    /*组播使能二期处理*/

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetIgmpAuthMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, auth_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_SendFrame2PON(short int olt_id, short int llid, void *eth_frame, int frame_len)
{
    int iRlt = OLT_ERR_OK;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(eth_frame);
	/*暂不支持，后期有可能支持*/
    return iRlt;
}

static int GPON_SendFrame2CNI(short int olt_id, short int llid, void *eth_frame, int frame_len)
{
    int iRlt = OLT_ERR_OK;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(eth_frame);
	/*暂不支持，后期有可能支持*/
    return iRlt;
}

static int GPON_DelVidDownlinkMode(short int olt_id, PON_vlan_tag_t vid)
{
    int iRlt = OLT_ERR_OK;
    PON_olt_vid_downlink_config_t stQinQCfg;

	OLT_LOCAL_ASSERT(olt_id);

	/*QINQ规则，当前在交换芯片上实现，后续可能支持*/
   

	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_DelVidDownlinkMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif


#if 1
/* -------------------OLT LLID 管理API, GPON对应的LLID为ONU ID-------------- */

static int GPON_LLIDIsExist(short int olt_id, short int llid, bool *status)
{
	int iRlt = 0;
	uint8 serialNumber[ONU_SEQUENCENUM_SIZE];

	VOS_MemSet(serialNumber, 0, ONU_SEQUENCENUM_SIZE);
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(status);
    iRlt = gponOnuAdp_GetSn(olt_id,llid,serialNumber);
	if(VOS_OK == iRlt)
	{
		*status = TRUE;
	}
	else
	{
		*status = FALSE;
	}
    return OLT_ERR_OK;
}

static int GPON_DeregisterLLID(short int olt_id, short int llid, bool iswait)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
#if 0    
    iRlt = gponOnuAdp_DeregOnu(olt_id, llid);
#else
    iRlt = gponOnuAdp_ResetOnu(olt_id, llid);
#endif
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_DeregisterLLID(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_GetLLIDMac(short int olt_id, short int llid, mac_address_t onu_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_mac);
    
    iRlt = gponOnuAdp_GetOnuMacAddress(olt_id, llid, onu_mac);
	
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetLLIDMac(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_GetLLIDRegisterInfo(short int olt_id, short int llid, onu_registration_info_t *onu_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
#if 0/*need to do*/
    bcmEmmiLinkInfo link_info;
		
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_info);

    if ( 0 == (iRlt = BcmOLT_GetLinkInfo ( olt_id, llid, &link_info)) )
    {
        onu_info->oam_version = OAM_STANDARD_VERSION_3_3;
        onu_info->rtt = link_info.rangeValue;
        
        onu_info->laser_on_time  = link_info.onuLaserOnTime;
        onu_info->laser_off_time = link_info.onuLaserOffTime;

        onu_info->vendorType     = 0;
        onu_info->productVersion = 0;
        onu_info->productCode    = 0;
        (void)PON_CTC_STACK_get_onu_ponchip_vendor(olt_id, llid, &onu_info->vendorType, &onu_info->productCode, &onu_info->productVersion);

        onu_info->max_links_support = 1;
        onu_info->curr_links_num = 1;
        onu_info->max_cm_support = 0;

        switch ( link_info.rate )
        {
            case bcmEmmiEponRateTenTen:
                onu_info->pon_rate_flags = PON_RATE_10_10G;
            break;    
            case bcmEmmiEponRateTenOne:
                onu_info->pon_rate_flags = PON_RATE_1_10G;
            break;    
            case bcmEmmiEponRateOneOne:
                onu_info->pon_rate_flags = PON_RATE_NORMAL_1G;
            break;    
            default:
                onu_info->pon_rate_flags = PON_RATE_NORMAL_1G;
                VOS_ASSERT(0);
        }
    }
#endif
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetLLIDRegisterInfo(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}


static int GPON_AuthorizeLLID(short int olt_id, short int llid, bool auth_mode)
{
    int iRlt = OLT_ERR_OK;
        
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(olt_id);
	/*GPON不处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_AuthorizeLLID(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, auth_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int GPON_SetLLIDSLA(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA)
{
    int iRlt = 0;
	GPONADP_ONU_BW bw = {0};

	#if 0
	VOS_MemSet(&bw, 0, sizeof(GPONADP_ONU_BW));
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(SLA);

    if ( 7 == SLA->SLA_Ver )
    {
    	bw.bwFixed = (SLA->SLA.SLA7.fixed_bw)*1024 + (SLA->SLA.SLA7.fixed_bw_fine)*64;
		bw.bwAssured = (SLA->SLA.SLA7.max_gr_bw)*1024 + (SLA->SLA.SLA7.max_gr_bw_fine)*64;
		bw.bwMax = (SLA->SLA.SLA7.max_be_bw)*1024 + (SLA->SLA.SLA7.max_be_bw_fine)*64;
		
		if ( 0 == (iRlt = gponOnuAdp_SetUpBW(olt_id, llid, bw)) )
        {
            SLA->DBA_ErrCode = 0;
        }
    }
    else
    {
        iRlt = OLT_ERR_PARAM;
    }
	#endif
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetLLIDSLA(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int GPON_GetLLIDSLA(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA)
{
    int iRlt = 0;
	GPONADP_ONU_BW bw = {0};
#if 0
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(SLA);

    if ( 0 == (iRlt = gponOnuAdp_GetUpBW(olt_id, llid, &bw)) )
    {
        SLA->SLA_Ver = 7;
        SLA->DBA_ErrCode = 0;
		SLA->SLA.SLA7.fixed_bw = bw.bwFixed/1024;
		SLA->SLA.SLA7.fixed_bw_fine = (bw.bwFixed%1024)/64;
		SLA->SLA.SLA7.max_gr_bw= bw.bwAssured/1024;
		SLA->SLA.SLA7.max_gr_bw_fine = (bw.bwAssured%1024)/64;
		SLA->SLA.SLA7.max_be_bw = bw.bwMax/1024;
		SLA->SLA.SLA7.max_be_bw_fine = (bw.bwMax%1024)/64;
		
    }
	#endif
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetLLIDSLA(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int GPON_SetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police)
{
    int iRlt = OLT_ERR_OK;
	/*下行限速暂不处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetLLIDPolice(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int GPON_GetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(police);

	/*下行限速暂不处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetLLIDPolice(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

#endif


#if 1
/* -------------------OLT ONU 管理API-------------- */

static int GPON_GetAllOnus(short int olt_id, OLT_onu_table_t *onu_table)
{
    int iRlt;
    short int llid_num;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(onu_table);
#if 0/*need to do*/
    if ( 0 == (iRlt = BcmAdapter_PAS5201_GetAllOnuParams(olt_id, &llid_num, onu_table->onus)) )
    {
        onu_table->onu_num = llid_num;
    }
#endif
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetAllOnus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int ResumeAllOnuStatus(short int olt_id, int olt_chipid, int resume_reason, int resume_mode)
{
    extern CTC_STACK_oui_version_record_t PAS_oui_version_records_list[];
    extern int PAS_oui_version_records_num;
    int OnuEntry, OnuIdx;
    int i;
    int resume_up_num, resume_dn_num, resume_er_num, resume_uk_num;
    short int checkin_num, onu_num;
    short int llid, ponIsOK;
    PAS_onu_parameters_t onu_list;
	int iRlt = 0;

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

#if 0/*GPON暂不考虑*/    
    /* 恢复原PON口上的ONU在线状态信息及补报ONU日志 */
#ifdef OLT_RECOVERY_BY_EVENT
    if ( (OLT_RESUMEMODE_SYNCSOFT == resume_mode) && SYS_LOCAL_MODULE_ISMASTERACTIVE )
    {
        if ( 0 == BcmAdapter_PAS5201_GetAllOnuParams(olt_id, &onu_num, onu_list) )
        {
            if ( onu_num > 0 )
            {
                if ( 0 < OLTAdv_GetOpticalTxMode(olt_id) )
                {
                    for (i=0; i<onu_num; ++i)
                    {
                        ++resume_uk_num;
                        llid = onu_list[i].llid;
                        
                        if ( 0 == BcmGetOnuMacAddress(olt_id, llid, MacAddr) )
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

                            BcmOLT_DeregisterLink( olt_id, llid );
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
                        if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
                        {
                            if ( onu_num < 0 )
                            {
                                /* 得到在线ONU的MAC-LLID列表 */
                                if ( 0 != BcmAdapter_PAS5201_GetAllOnuParams(olt_id, &onu_num, onu_list) )
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

                                                BcmOLT_DeregisterLink( olt_id, llid );
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
                                if ( 0 != BcmGetOnuMacAddress(olt_id, llid, MacAddr) )
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
                        if ( 0 != BcmAdapter_PAS5201_GetAllOnuParams(olt_id, &onu_num, onu_list) )
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
                                        BcmOLT_DeregisterLink( olt_id, llid );
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

    if ( checkin_num < onu_num )
    {
        /* ONU MAC列表里不存在的在线ONU的注册状态恢复 */
        for (i=0; i<onu_num; ++i)
        {
            if ( onu_list[i].rtt >= 0 )
            {
                ++resume_uk_num;
                llid = onu_list[i].llid;
                
                if ( 0 == BcmGetOnuMacAddress(olt_id, llid, MacAddr) )
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

                    BcmOLT_DeregisterLink( olt_id, llid );
                }
            }
        }    
    }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif

    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"PAS_ResumeAllOnuStatus(%d, %d, %d, %d)'s result(up:%d, down:%d, err:%d, unknown:%d) on slot %d.\r\n", olt_id, olt_chipid, resume_reason, resume_mode, resume_up_num, resume_dn_num, resume_er_num, resume_uk_num, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

static int GPON_ResumeAllOnuStatus(short int olt_id, int resume_reason, int resume_mode)
{
    int iRlt = ResumeAllOnuStatus(olt_id, PONCHIP_GPON, resume_reason, resume_mode);

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ResumeAllOnuStatus(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, resume_reason, resume_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int GPON_SetOnuAuthMode(short int olt_id, int auth_switch)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    {
        /*由上层软件处理*/
        iRlt = 0;
    }


    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetOnuAuthMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, auth_switch, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

/* added by xieshl 20110330, 问题单12139 */
extern ULONG	gLibEponMibSemId;
int GPON_SetAllOnuAuthMode2(short int olt_id, int enable)
{
    int iRlt = OLT_ERR_OK;
    int auth_mode = 0;
    int OnuIdx = 0;
	int localSN[GPON_ONU_SERIAL_NUM_STR_LEN];
	int ret;
	gpon_onu_auth_t  *pAuthData = NULL;	
    short int ul_slot = GetCardIdxByPonChip(olt_id);
    short int ul_port = GetPonPortByPonChip(olt_id);
    
    OLT_LOCAL_ASSERT(olt_id);
	/*由上层软件处理*/
	if( enable == V2R1_ONU_AUTHENTICATION_ALL)
    {
    	int OnuStatus;
    	short int llid;

        for( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
        {
        	OnuStatus = GetOnuOperStatus_1(olt_id, OnuIdx);
         	if(OnuStatus == ONU_OPER_STATUS_UP)	
          	{
              	llid = GetLlidByOnuIdx(olt_id, OnuIdx);
              	if(llid != INVALID_LLID)
            	gponOnuAdp_ResetOnu(olt_id,llid);
         	}
    	}
    }
	else if(enable == V2R1_ONU_AUTHENTICATION_DISABLE)
	{
		ActivatePendingOnuMsg( olt_id );
	}
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetAllOnuAuthMode2(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    return iRlt;
}

static int GPON_SetMacAuth(short int olt_id, int mode, mac_address_t mac)
{
    int iRlt = 0;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(mac);

	/*由上层软件处理*/
    iRlt = 0;
    
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetMacAuth(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_SetOnuDownlinkPri2CoSQueueMap(short int olt_id, OLT_pri2cosqueue_map_t *map)
{
    int iRlt;

    iRlt = OLT_ERR_NOTSUPPORT;
	/*下行暂不处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetOnuDownlinkPri2CoSQueueMap(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* 激活ONU */
static int GPON_ActivePendingOnu(short int olt_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    pendingOnu_S *CurOnu;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
    CurOnu = PonPortTable[olt_id].PendingOnu.Next;
    while( CurOnu != NULL )
    {	
    
        (void)gponOnuAdp_ResetOnu( olt_id, CurOnu->Llid );
        iRlt = 0;
        
        CurOnu = CurOnu->Next;
    }
    VOS_SemGive( OnuPendingDataSemId );

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ActivePendingOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, CurOnu->Llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;	
}

/* 激活一个ONU */
static int GPON_ActiveOnePendingOnu(short int olt_id, char*SN)
{
    int iRlt = OLT_ERR_NOTEXIST;
    pendingOnu_S *CurOnu;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(SN);

    VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
    CurOnu = PonPortTable[olt_id].PendingOnu.Next;
    while( CurOnu != NULL )
    {		
        if( VOS_StrCmp(CurOnu->OnuMarkInfor.OnuMark.serial_no, SN) == 0  )
        {
        
            (void)gponOnuAdp_ResetOnu( olt_id, CurOnu->Llid );
		
            iRlt = 0;
            break;
        }
        CurOnu = CurOnu->Next;
    }	
    VOS_SemGive( OnuPendingDataSemId );

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ActivateOnePendingOnu(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_ActiveConfPendingOnu(short int olt_id, short int conf_olt_id)
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
            
                (void)gponOnuAdp_ResetOnu( olt_id, CurOnu->Llid );
			
                iRlt = 0;
            }

            CurOnu = CurOnu->Next;
        }
        VOS_SemGive( OnuPendingDataSemId );
    }

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ActiveConfPendingOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, conf_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_ActiveOneConfPendingOnu(short int olt_id, short int conf_olt_id, UCHAR *mac)
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
            if( (PonIdx == conf_olt_id) && (VOS_StrCmp(mac, CurOnu->OnuMarkInfor.OnuMark.MacAddr)) == 0 )
            {	
            			
                (void)gponOnuAdp_ResetOnu( olt_id, CurOnu->Llid );
				
                iRlt = 0;
                
                break;					
            }

            CurOnu = CurOnu->Next;
        }
        VOS_SemGive( OnuPendingDataSemId );
    }

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ActiveOneConfPendingOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, conf_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_SetOnuP2PMode(short int olt_id, int p2p_mode)
{
    int iRlt = OLT_ERR_OK;
    int p2p_num;
        
    OLT_LOCAL_ASSERT(olt_id);
	/*暂不处理，需要了解GPON下ONU有无此概念*/
   
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetOnuP2PMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, p2p_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int GPON_GetOnuB2PMode(short int olt_id, int *b2p_mode)
{
    int iRlt = OLT_ERR_OK;
        
    OLT_LOCAL_ASSERT(olt_id);

   /*暂不处理，需要了解GPON下ONU有无此概念*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetOnuB2PMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *b2p_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_SetOnuB2PMode(short int olt_id, int b2p_mode)
{
    int iRlt = OLT_ERR_OK;
        
    OLT_LOCAL_ASSERT(olt_id);

    /*暂不处理，需要了解GPON下ONU有无此概念*/

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetOnuB2PMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, b2p_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_GetOnuMode(short int olt_id, short int llid)
{
    int iRlt;
    
    OLT_LOCALID_ASSERT(olt_id);
    if ( gponOnuAdp_isOnline(olt_id, llid) )
    {
        iRlt = PON_ONU_MODE_ON;
    }
    else
    {
        iRlt = PON_ONU_MODE_OFF;
    }

	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetOnuMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
}

static int GPON_SetOnuTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    char onuSn[GPONADP_ONU_SERIAL_NUMBER_LEN];
    int snLen;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if (parameter->onu_sn[0] == 0xff 
     && parameter->onu_sn[1] == 0xff
     && parameter->onu_sn[2] == 0xff
     && parameter->onu_sn[3] == 0xff
     && parameter->onu_sn[4] == 0xff
     && parameter->onu_sn[5] == 0xff)
    {
        /* 不支持all操作 */
        return OLT_ERR_PARAM;
    }

    iRlt = GetOnuSerialNo(olt_id, onu_id, onuSn, &snLen);
    if (iRlt == RERROR)
    {
        return OLT_ERR_PARAM;
    }

    llid = GetLlidByOnuIdx(olt_id, onu_id);

    if (parameter->action == 0)
    {
        /*enable*/
        iRlt = gponOnuAdp_TxPowerEnableByOnuSn(olt_id, llid, onuSn, 1);
    }
    else 
    {
        /* disable */
        iRlt = gponOnuAdp_TxPowerEnableByOnuSn(olt_id, llid, onuSn, 0);
        
    }

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetOnuTxPowerSupplyControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


#endif


#if 1
/* -------------------OLT 加密管理API----------- */

static int GPON_SetEncryptMode(short int olt_id, int encrypt_mode)
{
    int iRlt = OLT_ERR_OK;
    short int port_id;
    TkEncryptionInfo encry_info;
       
    OLT_LOCAL_ASSERT(olt_id);
	/*一期不处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetEncryptMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, encrypt_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_GetLLIDEncryptMode(short int olt_id, short int llid, bool *encrypt_mode)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(encrypt_mode);

	/*一期不处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetLLIDEncryptMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, *encrypt_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_StartLLIDEncrypt(short int olt_id, short int llid)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

	/*一期不处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_StartLLIDEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_FinishLLIDEncrypt(short int olt_id, short int llid, short int status)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

	/*一期不处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_FinishLLIDEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_StopLLIDEncrypt(short int olt_id, short int llid)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

	/*一期不处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_StopLLIDEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_SetLLIDEncryptKey(short int olt_id, short int llid, PON_encryption_key_t key)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(key);

	/*一期不处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetLLIDEncryptKey(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_FinishLLIDEncryptKey(short int olt_id, short int llid, short int status)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

	/*一期不处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_FinishLLIDEncryptKey(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

#endif


#if 1
/* -------------------OLT 地址表管理API-------- */

static int GPON_SetMacAgingTime(short int olt_id, int aging_time)
{
    int iRlt = OLT_ERR_OK;
	
    OLT_LOCAL_ASSERT(olt_id);
    
    /*稍后处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetMacAgingTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, aging_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GPON_SetAddressTableConfig(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg)
{
    int iRlt = 0;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addrtbl_cfg);

    /*稍后处理*/
    
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetAddressTableConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, addrtbl_cfg->removed_when_full, addrtbl_cfg->discard_llid_unlearned_sa, addrtbl_cfg->discard_unknown_da, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GPON_GetAddressTableConfig(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg)
{
    int iRlt = OLT_ERR_OK;
    unsigned long ulValue;	
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addrtbl_cfg);
    
    /*稍后处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetAddressTableConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, addrtbl_cfg->removed_when_full, addrtbl_cfg->discard_llid_unlearned_sa, addrtbl_cfg->discard_unknown_da, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GPON_GetMacAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl)
{
    int iRlt = OLT_ERR_OK;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num);
	
    iRlt = gponOltAdp_GetMacAddrTbl(olt_id, addr_num, addr_tbl);
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetMacAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_GetMacAddrVlanTbl(short int olt_id, short int *addr_num, PON_address_vlan_table_t addr_tbl)
{
    int iRlt = OLT_ERR_OK;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num);
	
    iRlt = gponOltAdp_GetMacAddrVlanTbl(olt_id, addr_num, addr_tbl);
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetMacAddrVlanTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int  GPON_AddMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num != 0);
    VOS_ASSERT(addr_tbl);

   /*稍后处理*/

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_AddMacAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_DelMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num != 0);
    VOS_ASSERT(addr_tbl);

    /*稍后处理*/

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_DelMacAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_RemoveMac(short int olt_id, mac_address_t mac)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(mac);

    /*稍后处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_RemoveMac(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_ResetAddrTbl(short int olt_id, short int llid, int addr_type)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

    /*稍后处理*/
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ResetAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, addr_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

#endif


#if 1
/* -------------------OLT 光路管理API----------- */

static int GPON_GetOpticalCapability(short int olt_id, OLT_optical_capability_t *optical_capability)
{
    int iRlt = 0;
    int tx_mode;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(optical_capability);
	#if 0

    if ( 0 == (iRlt = gponOltAdp_GetOpticalMode(olt_id, &tx_mode)) )
    {
      
        VOS_MemZero(optical_capability, sizeof(OLT_optical_capability_t));
        
        if ( tx_mode )    
        {
            optical_capability->pon_tx_signal = TRUE;
        }
        else
        {
            optical_capability->pon_tx_signal = FALSE;
        }

		/*其它参数在GPON中没有对应值，需要考虑GPON支持的光参数*/
      
    }
	#endif

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetOpticalCapability(%d, %d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, optical_capability->agc_lock_time, optical_capability->cdr_lock_time, optical_capability->laser_on_time, optical_capability->laser_off_time, optical_capability->pon_tx_signal, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_GetOpticsDetail(short int olt_id, OLT_optics_detail_t *optics_params) 
{
    int iRlt;
    int tx_mode;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(optics_params);

    VOS_MemZero(optics_params, sizeof(OLT_optics_detail_t));

    if ( 0 == (iRlt = gponOltAdp_GetOpticalMode(olt_id, &tx_mode)) )
    {        
        if ( tx_mode )    
        {
            optics_params->pon_tx_signal = TRUE;
        }
        else
        {
            optics_params->pon_tx_signal = FALSE;
        }

 		/*其它参数在GPON中没有对应值，需要考虑GPON支持的光参数*/
        
    }

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetOpticsDetail(%d, %d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, optics_params->pon_optics_params.agc_lock_time, optics_params->pon_optics_params.cdr_lock_time, optics_params->pon_optics_params.discovery_laser_on_time, optics_params->pon_optics_params.discovery_laser_off_time, optics_params->pon_tx_signal, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_SetPonRange(short int olt_id, unsigned int max_range, unsigned int max_rtt)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
	/*RTT在GPON中无意义*/
	/*31854,增加先关光后开光的操作，以避免修改pon max distance后需要拔插光纤才能上线的问题 by jinhl@2017.04.11*/
	GW_GPONTxEnable(olt_id, 0);/*先关光*/
	iRlt = gponOltAdp_SetMaxRange(olt_id,max_range);
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetPonRange(%d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, max_range, max_rtt, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	GW_GPONTxEnable(olt_id, 1);/*再开光*/
    return iRlt;
}

static int GPON_SetOpticalTxMode(short int olt_id, int tx_mode)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

    iRlt = gponOltAdp_SetOpticalMode(olt_id, tx_mode);

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetOpticalTxMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, tx_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_GetOpticalTxMode(short int olt_id, int *tx_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(tx_mode);

    iRlt = gponOltAdp_GetOpticalMode(olt_id, tx_mode);

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetOpticalTxMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *tx_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_SetVirtualScopeAdcConfig(short int olt_id, PON_adc_config_t *adc_config)
{
    int iRlt = OLT_ERR_NOTSUPPORT;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(adc_config);
	/*不支持*/

	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetVirtualScopeAdcConfig(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_GetVirtualScopeMeasurement(short int olt_id, short int llid, PON_measurement_type_t measurement_type, 
	void *configuration, short int config_len, void *result, short int res_len)
{
    int iRlt = OLT_ERR_NOTSUPPORT;

	OLT_LOCAL_ASSERT(olt_id);
	/*不支持*/
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetVirtualScopeMeasurement(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_GetVirtualScopeRssiMeasurement(short int olt_id, short int llid, PON_rssi_result_t *rssi_result)
{
    int iRlt = OLT_ERR_NOTSUPPORT;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = gponOltAdp_SetRssi(olt_id, llid);
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetVirtualScopeRssiMeasurement(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, (int)rssi_result->dbm, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_GetVirtualScopeOnuVoltage(PON_olt_id_t olt_id, short int llid, float *voltage,unsigned short int *sample, float *dbm)
{
    int iRlt = OLT_ERR_NOTSUPPORT;

	OLT_LOCAL_ASSERT(olt_id);
	/*不支持*/
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetVirtualScopeOnuVoltage(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, (int)*dbm, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_SetVirtualLLID(short int olt_id, short int llid, PON_virtual_llid_operation_t operation)
{
    int iRlt;
	/*暂不处理*/
	iRlt = OLT_ERR_NOTSUPPORT;
	OLT_GPON_DEBUG(OLT_GPON_TITLE"OLT_SetVirtualLLID(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, operation, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int GPON_SetOpticalTxMode2(short int olt_id, int tx_mode, int tx_reason)
{
    int iRlt = OLT_ERR_OK;
    volatile UCHAR *pCPLDRegPonTx;
	volatile UCHAR *pCPLDRegPonVccT;
	volatile UCHAR *pCPLDRegProtect;
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
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetOpticalTxMode2(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, tx_mode, tx_reason, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif


#if 1
/* -------------------OLT 监控统计管理API---- */

static int GPON_GetRawStatistics(short int olt_id, OLT_raw_stat_item_t *stat_item)
{
    int iRlt = OLT_ERR_OK;
    tOgCmPmPonLinkNewCounter *stats = NULL;
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(stat_item);
    VOS_ASSERT(stat_item->statistics_data);
    VOS_ASSERT(stat_item->statistics_data_size > 0);
	switch(stat_item->collector_id)
	{
		case GPON_PM_PONLINK_ACTIVE_COUNTER:
			iRlt = gponOltAdp_GetPonLinkCounter(olt_id,(tOgCmPmPonLinkNewCounter *)stat_item->statistics_data);
			stats = (tOgCmPmPonLinkNewCounter *)stat_item->statistics_data;
			break;
		case GPON_PM_PONNNI_ACTIVE_COUNTER:
			iRlt = gponOltAdp_GetPonNNICounter(olt_id,stat_item->statistics_data);
			break;
		case GPON_PM_PONGEM_ACTIVE_COUNTER:/*这里statistics_parameter作gemId用*/
			iRlt = gponOltAdp_GetPonGemCounter(olt_id,stat_item->statistics_parameter,stat_item->statistics_data);
			break;
		default:
			return OLT_ERR_PARAM;
	}
 
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetRawStatistics(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, stat_item->collector_id, stat_item->raw_statistics_type, stat_item->statistics_parameter, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

extern ULONG ulGPON_GEMPORT_BASE;
static int GPON_ResetCounters(short int olt_id)
{
    int iRlt = OLT_ERR_OK;
	int onuId = 0;
	int llid = 0;
	int gemId = 0;    
    OLT_LOCAL_ASSERT(olt_id);
	gponOltAdp_ClearPonLinkCounter(olt_id);
	gponOltAdp_ClearPonNNICounter(olt_id);
	for( onuId = 0; onuId < MAXONUPERPON; onuId++)
	{
		llid = GetLlidByOnuIdx( olt_id, onuId);
		if ((-1) == llid ) 
			continue;
		gemId = ulGPON_GEMPORT_BASE+(llid-1)*16;
		gponOltAdp_ClearPonGemCounter(olt_id, gemId);
				
	}

	
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ResetCounters(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_SetBerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_bytes)
{
    int iRlt = OLT_ERR_OK;
    bool activate;
    PON_ber_alarm_configuration_t alarm_cfg;
	
    OLT_LOCAL_ASSERT(olt_id);

     /*无berAlarm*/
	
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetBerAlarmConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, alarm_switch, alarm_thresold, alarm_min_error_bytes, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GPON_SetFerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_frames)
{
    int iRlt = OLT_ERR_OK;
    bool activate;
    PON_fer_alarm_configuration_t alarm_cfg;
	
    OLT_LOCAL_ASSERT(olt_id);

    /*无ferAlarm*/
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetFerAlarmConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, alarm_switch, alarm_thresold, alarm_min_error_frames, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GPON_SetAlarmConfig(short int olt_id, short int source, PON_alarm_t type, bool activate, void	*configuration, int length)
{
    int iRlt = ROK;

	OLT_LOCAL_ASSERT(olt_id);
	 /*暂不处理*/
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetAlarmConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, source, type, activate, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GPON_GetAlarmConfig(short int olt_id, short int source, PON_alarm_t type, bool *activate, void	*configuration)
{
    int iRlt = ROK;

	OLT_LOCAL_ASSERT(olt_id);
   	/*暂不处理*/
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetAlarmConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, source, type, *activate, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GPON_GetStatistics(short int olt_id, short int collector_id, PON_statistics_t statistics_type, short int statistics_parameter, long double *statistics_data)
{
    int iRlt = OLT_ERR_OK;
        
    OLT_LOCAL_ASSERT(olt_id);
	/*此处只针对BER,FER统计，一期不处理*/
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetStatistics(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, collector_id, statistics_type, statistics_parameter, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_OltSelfTest(short int olt_id)
{
    int iRlt = OLT_ERR_OK;

	OLT_LOCAL_ASSERT(olt_id);
	/*一期目标暂不支持*/
	
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_OltSelfTest(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_LinkTest(short int olt_id, short int llid, short int number_of_frames, short int frame_size, bool link_delay_measurement, PON_link_test_vlan_configuration_t *vlan_configuration, PON_link_test_results_t *test_results)
{
    int iRlt = RERROR;
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_LinkTest(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	/*一期目标不支持*/
	return iRlt;
}

static int GPON_SetLLIDFecMode(short int olt_id, short int llid, bool downlink_fec)
{
    int iRlt = OLT_ERR_OK;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

	/*GPON只有针对onu设置上行FEC*/
	if(downlink_fec)
	{
		iRlt = gponOnuAdp_SetFec(olt_id, llid, GPONADP_ONU_STATE_ENABLE);
	}
	else
	{
		iRlt = gponOnuAdp_SetFec(olt_id, llid, GPONADP_ONU_STATE_DISABLE);
	}
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetLLIDFecMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, downlink_fec, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_GetLLIDFecMode(short int olt_id, short int llid, bool *downlink_fec, bool *uplink_fec, bool *uplink_lastframe_fec)
{
    int iRlt = OLT_ERR_OK;
	GPONADP_ONU_STATE_E fecMode = 0;
	
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(downlink_fec);
    VOS_ASSERT(uplink_fec);
    VOS_ASSERT(uplink_lastframe_fec);

	/*GPON只有针对onu设置上行FEC*/
	iRlt = gponOnuAdp_GetFec(olt_id, llid, &fecMode);
	if(GPONADP_ONU_STATE_ENABLE == fecMode)
	{
		*downlink_fec = TRUE;
		*uplink_fec = TRUE;
		*uplink_lastframe_fec = TRUE;
	}
	else
	{
		*downlink_fec = FALSE;
		*uplink_fec = FALSE;
		*uplink_lastframe_fec = FALSE;
	}
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetLLIDFecMode(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, *downlink_fec, *uplink_fec, *uplink_lastframe_fec, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

#endif


#if 1
/* -------------------OLT 倒换API---------------- */

static int GPON_GetHotSwapCapability(short int olt_id, int *swap_cap)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(swap_cap);

    *swap_cap = V2R1_PON_PORT_SWAP_SLOWLY;
    
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetHotSwapCapability(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *swap_cap, 0, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

static int GPON_GetHotSwapMode(short int olt_id, short int *partner_olt_id, int *swap_mode, int *swap_status)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
 #if  0/*一期目标暂不支持*/
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(partner_olt_id);
    VOS_ASSERT(swap_mode);
    VOS_ASSERT(swap_status);

    if ( OLT_ERR_OK == (iRlt = GW_GetHotSwapMode(olt_id, partner_olt_id, swap_mode, swap_status)) )
    {
        int tx_mode;

        *swap_mode = V2R1_PON_PORT_SWAP_AUTO;
        if ( OLT_ERR_OK == (iRlt = BCM55538_GetOpticalTxMode(olt_id, &tx_mode)) )
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

    OLT_BCM_DEBUG(OLT_BCM_TITLE"BCM55538_GetHotSwapMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, *partner_olt_id, *swap_mode, *swap_status, iRlt, SYS_LOCAL_MODULE_SLOTNO);
#endif
    return iRlt;
}

static int GPON_SetHotSwapMode(short int olt_id, short int partner_olt_id, int swap_mode, int swap_status, int swap_flags)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
    int iPartRlt;
    int new_swap_mode, old_swap_mode;
#if 0/*一期目标暂不支持*/
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
                /* 5001的取消倒换 */
    			iRlt = BCM55538_SetOpticalTxMode( olt_id, TRUE );
                if ( 0 == iRlt )
                {
                    PonPortTable[olt_id].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
                    if ( OLT_ISLOCAL(partner_olt_id)
                        /* B--added by liwei056@2011-11-22 for D13979 */
                        && OltIsExist(partner_olt_id)
                        /* E--added by liwei056@2011-11-22 for D13979 */
                        )
                    {
            			if ( 0 == (iRlt = BCM55538_SetOpticalTxMode( partner_olt_id, TRUE )) )
                        {
                            PonPortTable[partner_olt_id].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
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
        			iRlt = BCM55538_SetOpticalTxMode( olt_id, FALSE );
                }
                else
                {
            		(void)BCM55538_ResetAddrTbl(olt_id, PON_ALL_ACTIVE_LLIDS,  ADDR_DYNAMIC);
        			if ( 0 == (iRlt = BCM55538_SetOpticalTxMode( olt_id, TRUE )) )
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

    OLT_BCM_DEBUG(OLT_BCM_TITLE"BCM55538_SetHotSwapMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, swap_mode, swap_status, iRlt, SYS_LOCAL_MODULE_SLOTNO);
 #endif   
    return iRlt;
}

static int GPON_ForceHotSwap(short int olt_id, short int partner_olt_id, int swap_status, int swap_flags)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
#if 0/*一期目标暂不支持*/
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
                PonSwitchInfo_S switch_event;

                switch_event.olt_id       = olt_id;
                switch_event.new_status   = V2R1_PON_PORT_SWAP_PASSIVE;
                switch_event.event_id     = PROTECT_SWITCH_EVENT_START;
                switch_event.event_code   = PROTECT_SWITCH_REASON_HOSTREQUEST;
                
                switch_event.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
                switch_event.event_seq    = PonPortTable[olt_id].swap_times + 1;
                switch_event.event_flags  = PROTECT_SWITCH_EVENT_FLAGS_NONE;
                PonSwitchHandler(&switch_event);

                iRlt = OLT_SetHotSwapMode(olt_id, partner_olt_id, V2R1_PON_PORT_SWAP_SLOWLY, PROTECT_SWITCH_STATUS_PASSIVE, OLT_SWAP_FLAGS_ONLYSETTING);
#if 0
                if ( 0 == iRlt )
                {
                    if ( OLT_ISREMOTE(partner_olt_id) )
                    {
                        iRlt = iPartRlt;
                    }
                }
#endif

                switch_event.olt_id       = partner_olt_id;
                switch_event.new_status   = V2R1_PON_PORT_SWAP_ACTIVE;
                switch_event.event_id     = PROTECT_SWITCH_EVENT_OVER;
                switch_event.event_code   = (0 == iRlt) ? PROTECT_SWITCH_RESULT_SUCCEED : PROTECT_SWITCH_RESULT_FAILED;;
                
                switch_event.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
                switch_event.event_seq    = PonPortTable[olt_id].swap_times + 1;
                switch_event.event_flags  = PROTECT_SWITCH_EVENT_FLAGS_NONE;
                PonSwitchHandler(&switch_event);
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

    OLT_BCM_DEBUG(OLT_BCM_TITLE"BCM55538_ForceHotSwap(%d, %d, %d, %d)'s result(%d) at mode(%d) on slot %d.\r\n", olt_id, partner_olt_id, swap_status, swap_flags, iRlt, local_mode, SYS_LOCAL_MODULE_SLOTNO);
#endif
    return iRlt;
}

static int GPON_RdnOnuRegister(short int olt_id, PON_redundancy_onu_register_t *onu_reg_info)
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
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_RdnOnuRegister(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_reg_info->onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_SetRdnConfig(short int olt_id, int rdn_status, int gpio_num, int rdn_type, int rx_enable)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#if 0    
    iRlt = PAS_set_redundancy_config(olt_id, rdn_status, (unsigned short)gpio_num, rdn_type, rx_enable);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetRdnConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, rdn_status, rdn_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_RdnSwitchOver(short int olt_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#if 0    
    iRlt = PAS_redundancy_switch_over(olt_id);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_RdnSwitchOver(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_RdnIsExist(short int olt_id, bool *status)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(status);

#if 0    
    *status = redundancy_olt_exists(olt_id);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    /* OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_RdnIsExist(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *status, 0, SYS_LOCAL_MODULE_SLOTNO); */
    
    return 0;    
}

static int GPON_ResetRdnRecord(short int olt_id, int rdn_state)
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
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_ResetRdnRecord(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, rdn_state, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_GetRdnState(short int olt_id, int *state)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(state);

#if 0    
    iRlt = get_redundancy_state(olt_id, state);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetRdnState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *state, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_SetRdnState(short int olt_id, int state)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#if 0    
    iRlt = set_redundancy_state(olt_id, state);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetRdnState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, state, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_GetRdnAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl)
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
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetRdnAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_RemoveRdnOlt(short int olt_id)
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

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_RemoveRdnOlt(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_GetLLIDRdnDB(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db)
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
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetLLIDRdnDB(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_SetLLIDRdnDB(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db)
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
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_SetLLIDRdnDB(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int GPON_RdnRemoveOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#if 0    
    if ( OLT_ISREMOTE(partner_olt_id) )
    {
        iRlt = remote_olt_removed_handler(olt_id, partner_olt_id);
    }
    else
    {
        OLT_LOCAL_ASSERT(partner_olt_id);
        
        iRlt = Remove_olt(partner_olt_id, FALSE, FALSE);
    }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_RdnRemoveOlt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_RdnSwapOlt(short int olt_id, short int partner_olt_id)
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

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_RdnSwapOlt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_AddSwapOlt(short int olt_id, short int partner_olt_id)
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

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_AddSwapOlt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_RdnLooseOlt(short int olt_id, short int partner_olt_id)
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
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_RdnLooseOlt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPON_GetLLIDRdnRegisterInfo(short int olt_id, short int llid, PON_redundancy_onu_register_t *onu_reginfo)
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
    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPON_GetLLIDRdnRegisterInfo(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif

#if 0/*GPON无此功能*/
static int BCM55538_SetCTCOnuTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = -1;
	 
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id); 

	iRlt = PON_CTC_STACK_onu_tx_power_supply_control(olt_id, llid, *parameter, TRUE);
 
    OLT_BCM_DEBUG(OLT_BCM_TITLE"BCM55538_SetCTCOnuTxPowerSupplyControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif
/*-----------------------------外部接口----------------------------------------*/

static const OltMgmtIFs s_gponIfs = {
#if 1
/* -------------------OLT基本API------------------- */
    GPON_IsExist,
    GPON_GetChipTypeID,
    GPON_GetChipTypeName,
    GPON_ResetPon,
    GPON_RemoveOlt,
    
    GW_CopyOlt,
    GW_CmdIsSupported,
    GPON_OK,   /* SetDebugMode */
    GPON_SetInitParams,
    GPON_SetSystemParams,

    GW_SetPonI2CExtInfo,
    GW_GetPonI2CExtInfo,
    GW_SetCardI2CInfo,
    GW_GetCardI2CInfo,
    GPON_WriteMdioRegister,

    GPON_ReadMdioRegister,
    GPON_ReadI2CRegister,
    NULL,     /*GpioAccess*/
    GPON_ReadGpio,
    GPON_WriteGpio,

    GPON_SendChipCli,
    GPON_OK,   /* SetDeviceName*/
    GPON_ResetPonChip,/*ResetPonChip*/
#endif

#if 1
/* -------------------OLT PON管理API--------------- */
    GPON_GetVersion,
    GPON_GetDBAVersion,
    GPON_ChkVersion,
    GW_ChkDBAVersion,   /* ChkDBAVersion */
    GPON_GetCniLinkStatus,

    GW_GetPonWorkStatus, 
    GPON_SetAdminStatus,
    GW_GetAdminStatus,   
    GPON_SetVlanTpid,
    GPON_OK,/*GPON_SetVlanQinQ*/

    NULL,     /* SetPonFrameSizeLimit */
    NULL,     /* GetPonFrameSizeLimit */
    NULL,     /*OamIsLimit*/
    GPON_UpdatePonParams,
    GW_SetPPPoERelayMode,

    GW_SetPPPoERelayParams,
    GW_SetDhcpRelayMode,
    GPON_SetIgmpAuthMode,
    GPON_SendFrame2PON,
    GPON_SendFrame2CNI,

    
    NULL,     /*GetVidDownlinkMode*/
    GPON_DelVidDownlinkMode,
    NULL,     /*GetOltParameters*/
    NULL,     /*SetOltIgmpSnoopingMode*/
    NULL,     /*GetOltIgmpSnoopingMode*/

    NULL,     /*SetOltMldForwardingMode*/
    NULL,     /*GetOltMldForwardingMode*/
    NULL,     /*SetDBAReportFormat*/
    NULL,     /*GetDBAReportFormat*/
    
    /*Begin:for onu swap by jinhl@2013-04-27*/
	GW_UpdateProvBWInfo, 
    /*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    GPON_LLIDIsExist,
    GPON_DeregisterLLID,
    GPON_GetLLIDMac,
    GPON_GetLLIDRegisterInfo,
    GPON_AuthorizeLLID,

    GPON_SetLLIDSLA,
    GPON_GetLLIDSLA,
    GPON_SetLLIDPolice,
    GPON_GetLLIDPolice,
    NULL,/*SetLLIDdbaType*/
    
    NULL,/*GetLLIDdbaType*/
    NULL,/*SetLLIDdbaFlags*/
    NULL,/*GetLLIDdbaFlags*/
    NULL,/*GetLLIDHeartbeatOam*/
    NULL,/*SetLLIDHeartbeatOam*/
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    GW_GetOnuNum,
    GPON_GetAllOnus,
    GPON_OK,   /* ClearAllOnus */
    GPON_ResumeAllOnuStatus,
    GPON_OK,   /* SetAllOnuAuthMode */

    GPON_SetOnuAuthMode,
    GPON_SetMacAuth,
    GPON_OK,   /* SetAllOnuBindMode */
    GPON_ERROR,/* CheckOnuRegisterControl */
    GPON_OK,   /* SetAllOnuDefaultBW */

    GPON_OK,   /* SetAllOnuDownlinkPoliceMode */
    GPON_OK,   /* SetOnuDownlinkPoliceMode */
    GPON_OK,   /* SetAllOnuDownlinkPoliceParam */
    GPON_OK,   /* SetAllOnuUplinkDBAParam */
    GPON_SetOnuDownlinkPri2CoSQueueMap,

    GPON_ActivePendingOnu,
    GPON_ActiveOnePendingOnu,
    GPON_ActiveConfPendingOnu,
    GPON_ActiveOneConfPendingOnu,
    GW_GetPendingOnu,

    GW_GetUpdatingOnu,
    GW_GetUpdatedOnu,
    GW_GetOnuUpdatingStatusLocal,
    GW_SetOnuUpdateMsg,
    GW_GetOnuUpdateWaiting,

    GPON_SetAllOnuAuthMode2,
    GPON_OK,   /* SetAllOnuBWParams */
    GPON_SetOnuP2PMode,
    GPON_GetOnuB2PMode,
    GPON_SetOnuB2PMode,

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    GPON_GetOnuMode,
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

    GPON_OK,   /*SetAuthEntry*/ 
    GPON_OK,	/*SetGponAuthEntry*/
    GPON_OK,   /*SetOnuDefaultMaxMac*/   
    GPON_OK,  /*GW_SetCTCOnuPortStatsTimeOut*/
    GPON_OK,    /* SetMaxOnu*/
    GPON_OK,    /* GetOnuConfDelStatus*/        
    GPON_SetOnuTxPowerSupplyControl,/*SetCTCOnuTxPowerSupplyControl*/
#endif

#if 1
/* -------------------OLT 加密管理API----------- */
    GPON_SetEncryptMode,
    NULL,     /*SetEncryptionPreambleMode*/
    NULL,     /*GetEncryptionPreambleMode*/
    GPON_GetLLIDEncryptMode,
    GPON_StartLLIDEncrypt,
    
    GPON_FinishLLIDEncrypt,
    GPON_StopLLIDEncrypt,
    GPON_SetLLIDEncryptKey,
    GPON_FinishLLIDEncryptKey,
#endif

#if 1
/* -------------------OLT 地址表管理API-------- */
    GPON_SetMacAgingTime,
    NULL,     /* SetAddressTableConfig */
    GPON_GetAddressTableConfig,
    GPON_GetMacAddrTbl,
    GPON_AddMacAddrTbl,

    GPON_DelMacAddrTbl,
    GPON_RemoveMac,
    GPON_ResetAddrTbl,
    GPON_OK,   /*SetOnuMacThreshold*/
    GW_OnuMacCheckEnable,
    
    GW_OnuMacCheckPeriod,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,     /*SetAddressTableFullHandlingMode*/
   /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,
    GPON_GetMacAddrVlanTbl,/*GetMacAddrVlanTbl*/
#endif

#if 1
/* -------------------OLT 光路管理API----------- */
    GPON_GetOpticalCapability,
    GPON_GetOpticsDetail,
    GPON_SetPonRange,
    GPON_SetOpticalTxMode,
    GPON_GetOpticalTxMode,

    GPON_SetVirtualScopeAdcConfig,
    GPON_GetVirtualScopeMeasurement,
    GPON_GetVirtualScopeRssiMeasurement,
    GPON_GetVirtualScopeOnuVoltage,
    GPON_SetVirtualLLID,
    
    GPON_SetOpticalTxMode2,
#endif

#if 1
/* -------------------OLT 监控统计管理API---- */
    GPON_GetRawStatistics,
    GPON_ResetCounters,
    GPON_SetBerAlarm,
    GPON_SetFerAlarm,
    GPON_OK,   /* SetPonBerAlarm */                      

    GPON_OK,   /* SetPonFerAlarm */                      
    GPON_OK,   /* SetBerAlarmParams */
    GPON_OK,   /* SetFerAlarmParams */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    GPON_SetAlarmConfig,
    GPON_GetAlarmConfig,
    
    GPON_GetStatistics,
    GPON_OltSelfTest,
    GPON_LinkTest,
    GPON_SetLLIDFecMode,
    GPON_GetLLIDFecMode,

    NULL,     /* SysDump */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif
    
#if 1
/* -------------------OLT 倒换API---------------- */
    GPON_GetHotSwapCapability,
    GPON_GetHotSwapMode,
    GPON_SetHotSwapMode,
    GPON_ForceHotSwap,
    GW_SetHotSwapParam,

    GPON_RdnOnuRegister,
    GPON_SetRdnConfig,
    GPON_RdnSwitchOver,
    GPON_RdnIsExist,
    GPON_ResetRdnRecord,

    GPON_GetRdnState,
    GPON_SetRdnState,
    GPON_GetRdnAddrTbl,
    GPON_RemoveRdnOlt,
    GPON_GetLLIDRdnDB,

    GPON_SetLLIDRdnDB,
    GPON_RdnRemoveOlt,
    GPON_RdnSwapOlt,
    GPON_AddSwapOlt,
    GPON_RdnLooseOlt,

    /*Begin:or onu swap */
    GPON_ERROR,/*RdnLLIDAdd*/
    GPON_ERROR,/*GetRdnLLIDMode*/
    GPON_ERROR,/*SetRdnLLIDMode*/
    GPON_ERROR,/*SetRdnLLIDStdbyToAct*/
    GPON_ERROR,/*SetRdnLLIDRtt*/
    /*End:for onu swap*/

    GW_RdnIsReady,
    GPON_GetLLIDRdnRegisterInfo,
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */
    NULL,     /* DumpAllCmc */
    NULL,     /* SetCmcServiceVID */
#endif

    GPON_ERROR /* LastFun */
};


void OLT_GPON_Support()
{
    OLT_RegisterAdapter(OLT_ADAPTER_GPON, &s_gponIfs);
    g_cszPonChipTypeNames[PONCHIP_GPON] = "GPON";
}


#ifdef __cplusplus

}

#endif


#endif

