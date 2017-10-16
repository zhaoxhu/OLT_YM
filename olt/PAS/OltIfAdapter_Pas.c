/***************************************************************
*
*						Module Name:  OltIfAdapter_Pas.c
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
#include  "ponEventHandler.h"

#include  "../../monitor/monitor.h"
#include  "../../onu/OnuPortStatsMgt.h"

extern char *g_cszPonChipTypeNames[PONCHIP_TYPE_MAX];    
extern long double g_afLevelConst[16];
extern ponAlarmInfo		**gpOnuAlarm;
extern ponThreasholdInfo gPonThreashold;

extern  int OnuMacCheckEnable(ULONG  enable);

/*-----------------------------内部适配----------------------------------------*/

static int PAS_OK(short int olt_id)
{
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS_OK(%d)'s result(0).\r\n", olt_id);

    return OLT_ERR_OK;
}

static int PAS_ERROR(short int olt_id)
{
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS_ERROR(%d)'s result(%d).\r\n", olt_id, OLT_ERR_NOTEXIST);

    debug_stub();

    VOS_ASSERT(0);

    return OLT_ERR_NOTEXIST;
}

/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if 0/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
int PAS_Test_RdnState(short int olt_id)
{
    int iRlt;
    PON_redundancy_olt_state_t rdn_state;
    PON_redundancy_type_t rdn_type;
    unsigned short rdn_gpio;
    bool rdn_recv;
            
    if ( 0 == (iRlt = PAS_get_redundancy_config(olt_id, &rdn_state, &rdn_gpio, &rdn_type, &rdn_recv)) )
    {
        sys_console_printf("[OLT-RDN:]olt%d's rdn-state=%d, rdn-type=%d, rdn-gpio=%d, rdn-recv=%d\r\n", olt_id, rdn_state, rdn_type, rdn_gpio, rdn_recv);
    }

    return iRlt;
}
#endif


#if 1
/* -------------------OLT基本API------------------- */

static int PAS_IsExist(short int olt_id, bool *status)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(status);
    
    *status = Olt_exists(olt_id);
    return OLT_ERR_OK;
}

static int PAS5001_GetChipTypeID(short int olt_id, int *type)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(type);

    *type = PONCHIP_PAS5001;
    return OLT_ERR_OK;
}

static int PAS5001_GetChipTypeName(short int olt_id, char typename[OLT_CHIP_NAMELEN])
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(typename);
    VOS_ASSERT(g_cszPonChipTypeNames[PONCHIP_PAS5001]);

    VOS_MemZero(typename, OLT_CHIP_NAMELEN);
    VOS_StrnCpy(typename, g_cszPonChipTypeNames[PONCHIP_PAS5001], OLT_CHIP_NAMELEN - 1);
    return OLT_ERR_OK;
}

static int PAS5201_GetChipTypeID(short int olt_id, int *type)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(type);

    *type = PONCHIP_PAS5201;
    return OLT_ERR_OK;
}

static int PAS5201_GetChipTypeName(short int olt_id, char typename[OLT_CHIP_NAMELEN])
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(typename);
    VOS_ASSERT(g_cszPonChipTypeNames[PONCHIP_PAS5201]);

    VOS_MemZero(typename, OLT_CHIP_NAMELEN );
    VOS_StrnCpy(typename, g_cszPonChipTypeNames[PONCHIP_PAS5201], OLT_CHIP_NAMELEN - 1);
    return OLT_ERR_OK;
}

static int PAS5204_GetChipTypeID(short int olt_id, int *type)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(type);

    *type = PONCHIP_PAS5204;
    return OLT_ERR_OK;
}

static int PAS5204_GetChipTypeName(short int olt_id, char typename[OLT_CHIP_NAMELEN])
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(typename);
    VOS_ASSERT(g_cszPonChipTypeNames[PONCHIP_PAS5204]);

    VOS_MemZero(typename, OLT_CHIP_NAMELEN );
    VOS_StrnCpy(typename, g_cszPonChipTypeNames[PONCHIP_PAS5204], OLT_CHIP_NAMELEN - 1);
    return OLT_ERR_OK;
}

static int PAS5001_ResetPon(short int olt_id)
{
    OLT_LOCAL_ASSERT(olt_id);

	ShutdownPonPort( olt_id );
	VOS_TaskDelay( VOS_TICK_SECOND*3 );
	StartupPonPort( olt_id );

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_ResetPon(%d)'s result(%d) on slot %d.\r\n", olt_id, 0, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

static int PAS5201_ResetPon(short int olt_id)
{
	short int partner_olt_id = 0;
    OLT_LOCAL_ASSERT(olt_id);
	/*Begin:for onu swap by jinhl@2013-04-27*/
	/*若不增加此处处理，则remove olt并不能使得
	pon口不发光，直到add_olt时。*/
    if(OLTAdv_IsExist(olt_id) == TRUE )
    {
    	PONTx_Disable( olt_id, PONPORT_TX_ACTIVED );
    } 
	/*End:for onu swap by jinhl@2013-04-27*/
	
    Remove_olt(olt_id, FALSE, FALSE);	
    GW_ResetPon(olt_id);

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_ResetPon(%d)'s result(%d) on slot %d.\r\n", olt_id, 0, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

static int PAS5001_RemoveOlt(short int olt_id, bool send_shutdown_msg_to_olt, bool reset_olt)
{
    OLT_LOCAL_ASSERT(olt_id);

    Remove_olt(olt_id, send_shutdown_msg_to_olt, reset_olt);	

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_RemoveOlt(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, send_shutdown_msg_to_olt, reset_olt, 0, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
int PAS_SetInitParams(short int olt_id, unsigned short host_olt_manage_type, unsigned short host_olt_manage_address)
{
    int iRlt;
	int iState = 0;
    OLT_LOCALID_ASSERT(olt_id);

#if defined(_EPON_10G_PMC_SUPPORT_)            
	if(SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
	    iState = GW10G_Get_pas_soft_state();
	}
	else
#endif
	{
	    iState = Get_pas_soft_state();
	}
	/*注意，此处无法通过GetPonChipTypeByPonPort获取芯片类型*/
    if ( PON_MODULE_STATE_NOT_INIT == iState )
    {
        switch (host_olt_manage_type)
        {
            case V2R1_PON_HOST_MANAGE_BY_BUS:
#if defined(_EPON_10G_PMC_SUPPORT_)            
				if ( SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON )
				{
	                iRlt = GW10G_PAS_set_current_interface(olt_id, PON_COMM_TYPE_PARALLEL);
				}
				else
#endif
				{
				    if ( 0 != host_olt_manage_address )
                    {
                        if ( 0 == (iRlt = PAS_set_olt_base_address(host_olt_manage_address)) )
                        {
        					iRlt = PAS_set_current_interface(PON_COMM_TYPE_PARALLEL);
                        }
                    }
                    else
                    {
    					iRlt = PAS_set_current_interface(PON_COMM_TYPE_PARALLEL);
                    }
				}
				
                break;
            case V2R1_PON_HOST_MANAGE_BY_ETH:
#if defined(_EPON_10G_PMC_SUPPORT_)            
				if(SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
				{
	                iRlt = GW10G_PAS_set_current_interface(olt_id, PON_COMM_TYPE_ETHERNET);
				}
				else 
#endif
				{
					iRlt = PAS_set_current_interface(PON_COMM_TYPE_ETHERNET);
				}
				
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
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS_SetInitParams(%d, %d, 0x%x)'s result(%d) on slot %d.\r\n", olt_id, host_olt_manage_type, host_olt_manage_address, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

int PAS_SetSystemParams(short int olt_id, long int statistics_sampling_cycle, long int monitoring_cycle, short int host_olt_msg_timeout, short int olt_reset_timeout)
{
    int iRlt;
	
    PAS_system_parameters_t  system_params;

	int iState = 0;

    OLT_LOCALID_ASSERT(olt_id);

#if defined(_EPON_10G_PMC_SUPPORT_)            
    if(SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
	    iState = GW10G_Get_pas_soft_state();
	}
	else
#endif
	{
	    iState = Get_pas_soft_state();
	}
   
    if ( PON_MODULE_STATE_RUNNING == iState )
    {
#if defined(_EPON_10G_PMC_SUPPORT_)            
        if(SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
    	{
        	iRlt = GW10G_PAS_get_system_parameters(&system_params);
    	}
		else 
#endif
		{
			iRlt = PAS_get_system_parameters(&system_params);
		}
		
        if( 0 == iRlt )
        {
            if (statistics_sampling_cycle > 0)
            {
                system_params.statistics_sampling_cycle = statistics_sampling_cycle;
            }
            else
            {
                system_params.statistics_sampling_cycle = PON_VALUE_NOT_CHANGED;
            }

            if (monitoring_cycle > 0)
            {
                system_params.monitoring_cycle = monitoring_cycle;
            }
            else
            {
                system_params.monitoring_cycle = PON_VALUE_NOT_CHANGED;
            }
            
            if (host_olt_msg_timeout > 0)
            {
                system_params.host_olt_msgs_timeout = host_olt_msg_timeout;
            }
            else
            {
                system_params.host_olt_msgs_timeout = PON_VALUE_NOT_CHANGED;
            }

            if (olt_reset_timeout > 0)
            {
                system_params.olt_reset_timeout = olt_reset_timeout;
            }
            else
            {
                system_params.olt_reset_timeout = PON_VALUE_NOT_CHANGED;
            }

            if ( system_params.automatic_authorization_policy != PAS_init_para.automatic_authorization_policy)
            {
                system_params.automatic_authorization_policy = PAS_init_para.automatic_authorization_policy;
            }
            else
            {
                system_params.automatic_authorization_policy = PON_VALUE_NOT_CHANGED;
            }
        }
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
    	{
        	iRlt = GW10G_PAS_set_system_parameters(&system_params);
    	}
		else 
#endif
		{
			iRlt = PAS_set_system_parameters(&system_params);
		}
		
        
    }
    else
    {
        iRlt = 0;
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS_SetSystemParams(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, statistics_sampling_cycle, monitoring_cycle, host_olt_msg_timeout, olt_reset_timeout, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}
/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int PAS5001_WriteMdioRegister(short int olt_id, short int phy_address, short int reg_address, unsigned short int value )
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_write_mdio_register(olt_id, phy_address, reg_address, value);
	
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_WriteMdioRegister(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, phy_address, reg_address, value, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	return iRlt;
}

static int PAS5001_ReadMdioRegister(short int olt_id, short int phy_address, short int reg_address, unsigned short int *value )
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_read_mdio_register(olt_id, phy_address, reg_address, value);
	
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_ReadMdioRegister(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, phy_address, reg_address, *value, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	return iRlt;
}

static int PAS5001_ReadI2CRegister(short int olt_id, short int device, short int register_address, short int *data )
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_read_i2c_register(olt_id, device, register_address, data);

#if 0
	
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_ReadI2CRegister(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, device, register_address, *data, iRlt, SYS_LOCAL_MODULE_SLOTNO);
#endif

	return iRlt;
}

static int PAS5001_GpioAccess(short int olt_id, short int line_number, PON_gpio_line_io_t set_direction, short int set_value, PON_gpio_line_io_t *direction, bool *value)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_gpio_access_extended_v4(olt_id, line_number, set_direction, set_value, direction, value);
	
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GpioAccess(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	return iRlt;
}

static int PAS5201_GpioAccess(short int olt_id, short int line_number, PON_gpio_line_io_t set_direction, short int set_value, PON_gpio_line_io_t *direction, bool *value)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_gpio_access(olt_id, line_number, set_direction, set_value, direction, value);
	
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GpioAccess(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

static int PAS5001_GetGpioLineNumber(short int olt_id, int func_id, int gpio_dir)
{
    int line_number = OLT_ERR_PARAM;

    if ( PON_GPIO_LINE_INPUT == gpio_dir )
    {
        switch ( func_id )
        {
            case OLT_GPIO_PON_LOSS:
                line_number = PON_GPIO_LINE_0;
                break;
            case OLT_GPIO_SFP_LOSS:
                line_number = PON_GPIO_LINE_3;
                break;
            default:
                NULL;
        }
    }
    else
    {
        switch ( func_id )
        {
            case OLT_GPIO_PON_WATCHDOG:
                line_number = PON_GPIO_LINE_0;
                break;
            case OLT_GPIO_PON_WATCHDOG_SWITCH:
                line_number = PON_GPIO_LINE_2;
                break;
            case OLT_GPIO_LED_RUN:
                line_number = PON_GPIO_LINE_1;
                break;
            default:
                NULL;
        }
    }

    return line_number;
}

static int PAS5001_ReadGpio(short int olt_id, int func_id, bool *value)
{
    int iRlt;
    PON_gpio_line_io_t line_direction;
   
	OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(value);

    if ( 0 <= (iRlt = PAS5001_GetGpioLineNumber(olt_id, func_id, PON_GPIO_LINE_INPUT)) )
    {
    	iRlt = PAS_gpio_access_extended_v4(olt_id, iRlt, PON_GPIO_LINE_INPUT, PON_VALUE_NOT_CHANGED, &line_direction, value);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_ReadGpio(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, func_id, *value, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int PAS5001_WriteGpio(short int olt_id, int func_id, bool value)
{
    int iRlt;
    PON_gpio_line_io_t line_direction;
    bool line_value;
   
	OLT_ASSERT(olt_id);

    if ( 0 <= (iRlt = PAS5001_GetGpioLineNumber(olt_id, func_id, PON_GPIO_LINE_OUTPUT)) )
    {
    	iRlt = PAS_gpio_access_extended_v4(olt_id, iRlt, PON_GPIO_LINE_OUTPUT, (short int)value, &line_direction, &line_value);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_WriteGpio(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, func_id, value, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}


static int PAS5201_GetGpioLineNumber(short int olt_id, int func_id, int gpio_dir)
{
    int line_number = OLT_ERR_PARAM;

    if ( PON_GPIO_LINE_INPUT == gpio_dir )
    {
        switch ( func_id )
        {
            case OLT_GPIO_PON_LOSS:
                line_number = PON_GPIO_LINE_0;
                break;
            case OLT_GPIO_SFP_LOSS:
                line_number = PON_GPIO_LINE_3;
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
                line_number = PON_GPIO_LINE_1;
                break;
            default:
                NULL;
        }
    }

    return line_number;
}

static int PAS5201_ReadGpio(short int olt_id, int func_id, bool *value)
{
    int iRlt;
    PON_gpio_line_io_t line_direction;
   
	OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(value);

    if ( 0 <= (iRlt = PAS5201_GetGpioLineNumber(olt_id, func_id, PON_GPIO_LINE_INPUT)) )
    {
    	iRlt = PAS_gpio_access(olt_id, iRlt, PON_GPIO_LINE_INPUT, PON_VALUE_NOT_CHANGED, &line_direction, value);
    }

#if 0
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_ReadGpio(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, func_id, *value, iRlt, SYS_LOCAL_MODULE_SLOTNO);
#endif

	return iRlt;
}

static int PAS5201_WriteGpio(short int olt_id, int func_id, bool value)
{
    int iRlt;
    PON_gpio_line_io_t line_direction;
    bool line_value;
   
	OLT_ASSERT(olt_id);

    if ( 0 <= (iRlt = PAS5201_GetGpioLineNumber(olt_id, func_id, PON_GPIO_LINE_OUTPUT)) )
    {
    	iRlt = PAS_gpio_access(olt_id, iRlt, PON_GPIO_LINE_OUTPUT, (short int)value, &line_direction, &line_value);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_WriteGpio(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, func_id, value, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int PAS5001_SendChipCli(short int olt_id, unsigned short size, unsigned char *command)
{
    int iRlt;
   
	OLT_ASSERT(olt_id);

   	iRlt = PAS_send_cli_command(olt_id, size, command);

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SendChipCli(%d, %d, %s)'s result(%d) on slot %d.\r\n", olt_id, size, command, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}
#endif


#if 1
/* -------------------OLT PON管理API--------------- */

static int PAS5001_GetVersion(short int olt_id, PON_device_versions_t *device_versions)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(device_versions);
    
	if ( 0 == (iRlt = PAS_get_device_versions_v4(olt_id, PON_OLT_ID, (PAS_device_versions_t*)device_versions)) )
    {
        device_versions->pon_vendors = PON_VENDOR_PMC;
    }   
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetVersion(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int PAS5201_GetVersion(short int olt_id, PON_device_versions_t *device_versions)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(device_versions);
    
	if ( 0 == (iRlt = PAS_get_olt_versions(olt_id, (PAS_device_versions_t*)device_versions)) )
    {
        device_versions->pon_vendors = PON_VENDOR_PMC;
    }   
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetVersion(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    /* B--added by liwei056@2010-11-25 for PAS-Firmware's BUG */
    if ( SYS_LOCAL_MODULE_TYPE_IS_CPU_PON )
    {
        if ( 0x5201 == device_versions->hardware_major )
        {
            device_versions->hardware_major = 0x5204;
        }
    }
    /* E--added by liwei056@2010-11-25 for PAS-Firmware's BUG */
    
	return iRlt;
}

static int  PAS5001_platoDBA2_0_GetDBAVersion(short int olt_id, OLT_DBA_version_t *dba_version)
{
	return PLATO2_get_info( olt_id, dba_version->szDBAname, sizeof(dba_version->szDBAname)-1, dba_version->szDBAversion, sizeof(dba_version->szDBAname)-1 );
}

static int  PAS5001_platoDBA3_0_GetDBAVersion(short int olt_id, OLT_DBA_version_t *dba_version)
{
	return PLATO3_get_info( olt_id, dba_version->szDBAname, sizeof(dba_version->szDBAname)-1, dba_version->szDBAversion, sizeof(dba_version->szDBAname)-1 );
}

static int PAS5001_GetDBAVersion(short int olt_id, OLT_DBA_version_t *dba_version)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(dba_version);
    
#ifdef PLATO_DBA_V3
	iRlt = PAS5001_platoDBA3_0_GetDBAVersion( olt_id, dba_version );
#else
	iRlt = PAS5001_platoDBA2_0_GetDBAVersion( olt_id, dba_version );
#endif
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetDBAVersion(%d, %s, %s)'s result(%d) on slot %d.\r\n", olt_id, dba_version->szDBAname, dba_version->szDBAversion, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_SetAdminStatus(short int olt_id, int admin_status)
{
    int iRlt;
    bool tran_mode;
        
    OLT_LOCAL_ASSERT(olt_id);

    tran_mode = (V2R1_ENABLE == admin_status) ? 1 : 0;
    if ( 0 < (iRlt = GW_SetOpticalTxMode2(olt_id, (int)tran_mode, PONPORT_TX_SHUTDOWN)) )
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
    	    iRlt = PAS_set_olt_pon_transmission(olt_id, tran_mode );
        }   
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetAdminStatus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, admin_status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_GetCniLinkStatus(short int olt_id, bool *status)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(status);
    
    iRlt = PAS_get_cni_link_status(olt_id, status);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetCniLinkStatus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int PAS5201_SetPonFrameSizeLimit(short int olt_id, short int jumbo_length)
{
    int iRlt;
    short int untag_length, tag_length;
        
    OLT_LOCAL_ASSERT(olt_id);

	untag_length = jumbo_length;
	if( jumbo_length <= (PON_MAX_HW_ETHERNET_FRAME_SIZE - 4) )
		tag_length = untag_length + 4;
	else
        tag_length = untag_length;

	iRlt = PAS_set_frame_size_limits(olt_id, PON_DIRECTION_UPLINK, untag_length, tag_length);
	if( PAS_EXIT_OK == iRlt )
    {
    	iRlt = PAS_set_frame_size_limits(olt_id, PON_DIRECTION_DOWNLINK, untag_length, tag_length);
    }   
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetPonFrameSizeLimit(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, jumbo_length, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_GetPonFrameSizeLimit(short int olt_id, short int *jumbo_length)
{
    int iRlt;
    short int untag_length, tag_length;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(jumbo_length);
   
	iRlt = PAS_get_frame_size_limits(olt_id, PON_DIRECTION_UPLINK, &untag_length, &tag_length);
	if( PAS_EXIT_OK == iRlt )
    {
    	*jumbo_length = untag_length;
    }   
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetPonFrameSizeLimit(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *jumbo_length, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int PAS5201_SetVlanTpid(short int olt_id, unsigned short int vlan_tpid)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

    iRlt = PAS_set_vlan_recognizing(olt_id, vlan_tpid);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetVlanTpid(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, vlan_tpid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_SetVlanQinQ(short int olt_id, OLT_vlan_qinq_t *vlan_qinq_config)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(vlan_qinq_config);

    if ( OLT_CFG_DIR_UPLINK == vlan_qinq_config->qinq_direction )
    {
        short int llid;
        short int onu_id;
        PON_olt_vlan_uplink_config_t *pQinQCfg;

        onu_id   = vlan_qinq_config->qinq_objectid;    
        pQinQCfg = &(vlan_qinq_config->qinq_cfg.up_cfg);
        
    	llid = GetLlidActivatedByOnuIdx(olt_id, onu_id);
    	if( llid != INVALID_LLID )
    	{
    		iRlt = PAS_set_llid_vlan_mode(olt_id, llid, *pQinQCfg);
    	}
        else
        {
            /* ONU不在线，则认为其空配置成功 */
            iRlt = 0;
        }
        OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetVlanQinQ(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, pQinQCfg->vlan_manipulation, pQinQCfg->authenticated_vid, pQinQCfg->new_vlan_tag_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    }
    else
    {
        short int vlan_id;
        PON_olt_vid_downlink_config_t stQinQCfg;
        PON_olt_vid_downlink_config_t *pQinQCfg;

        vlan_id  = vlan_qinq_config->qinq_objectid;    
        pQinQCfg = &(vlan_qinq_config->qinq_cfg.down_cfg);

        if ( PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION != pQinQCfg->vlan_manipulation )
        {
            if ( (pQinQCfg->new_priority < 0) || (pQinQCfg->new_priority > 7) )
            {
                /* PAS-SOFT's BugAdapter */
                VOS_MemCpy(&stQinQCfg, pQinQCfg, sizeof(stQinQCfg));
                pQinQCfg = &stQinQCfg;
                
        		pQinQCfg->new_priority = 0; /* PON_VLAN_ORIGINAL_PRIORITY */
            }
        
            iRlt = PAS_set_vid_downlink_mode(olt_id, vlan_id, *pQinQCfg);
        }
        else
        {
            iRlt = PAS_delete_vid_downlink_mode(olt_id, vlan_id);
        }
        OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetVlanQinQ(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, vlan_id, pQinQCfg->vlan_manipulation, pQinQCfg->new_vid, pQinQCfg->new_priority, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    }

    return iRlt;
}

static int PAS5001_OamIsLimit(short int olt_id, bool *oam_limit)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(oam_limit);

    iRlt = PAS_get_oam_configuration(olt_id, oam_limit);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_OamIsLimit(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *oam_limit, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_UpdatePonParams(short int olt_id, int max_range, int mac_agetime)
{
    int iRlt;
    int i;
    int aiMapRange2rtt[3][2] = {
                                {PON_RANGE_CLOSE, V2R1_REGISTER_WINDOW_CLOSE},
                                {PON_RANGE_40KM, PON_MAX_RTT_80KM},
                                {PON_RANGE_60KM, PON_MAX_RTT_120KM}
                                };
    PON_olt_update_parameters_t  pon_updated_parameters;
    
        
    OLT_LOCAL_ASSERT(olt_id);

	PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT(pon_updated_parameters);
	VOS_MemCpy( &pon_updated_parameters, &PAS_updated_parameters_5001, sizeof(PON_olt_update_parameters_t ) );

	for(i=0; i<3; i++)
    {
        if (aiMapRange2rtt[i][0] == max_range)
        {
            pon_updated_parameters.max_rtt = aiMapRange2rtt[i][1];
        }
    }

	pon_updated_parameters.address_table_aging_timer = mac_agetime;
	pon_updated_parameters.cni_port_maximum_entries = PON_ADDRESS_TABLE_ENTRY_LIMITATION_0;
	pon_updated_parameters.igmp_configuration.enable_igmp_snooping = DISABLE;
    iRlt = PAS_update_olt_parameters_v4(olt_id, &pon_updated_parameters);
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_UpdatePonParams(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, max_range, mac_agetime, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_UpdatePonParams(short int olt_id, int max_range, int mac_agetime)
{
    int iRlt;
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

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_UpdatePonParams(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, max_range, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_SetIgmpAuthMode(short int olt_id, int auth_mode)
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

    iRlt = PAS_set_classification_rule ( olt_id , PON_DIRECTION_UPLINK, PON_OLT_CLASSIFICATION_IGMP,
    									NULL, pkt_dir );
    if (0 == iRlt)
    {
        iRlt = PAS_set_classification_rule ( olt_id , PON_DIRECTION_UPLINK, PON_OLT_CLASSIFICATION_MLD,
        									NULL, pkt_dir );
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetIgmpAuthMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, auth_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_SendFrame2PON(short int olt_id, short int llid, void *eth_frame, int frame_len)
{
    int iRlt;
    short int broadcast_llid;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(eth_frame);

    if ( llid > 0 )
    {
        broadcast_llid = 0;
    }
    else
    {
        llid = PON_LLID_NOT_AVAILABLE;
        broadcast_llid = PON_EVERY_LLID;
    }

    if ( frame_len >= MIN_ETHERNET_FRAME_SIZE )
    {
        iRlt = PAS_send_frame(olt_id, frame_len, PON_PORT_PON, llid, broadcast_llid, eth_frame);
    }
    else
    {
        char szMinFrameBuf[MIN_ETHERNET_FRAME_SIZE];

        VOS_MemCpy(szMinFrameBuf, eth_frame, frame_len);
        VOS_MemZero(&szMinFrameBuf[frame_len], MIN_ETHERNET_FRAME_SIZE - frame_len);

        iRlt = PAS_send_frame(olt_id, MIN_ETHERNET_FRAME_SIZE, PON_PORT_PON, llid, broadcast_llid, szMinFrameBuf);
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SendFrame2PON(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, frame_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_SendFrame2CNI(short int olt_id, short int llid, void *eth_frame, int frame_len)
{
    int iRlt;
    short int broadcast_llid;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(eth_frame);

    if ( llid > 0 )
    {
        broadcast_llid = 0;
    }
    else
    {
        llid = PON_LLID_NOT_AVAILABLE;
        broadcast_llid = PON_EVERY_LLID;
    }

    if ( frame_len >= MIN_ETHERNET_FRAME_SIZE )
    {
        iRlt = PAS_send_frame(olt_id, frame_len, PON_PORT_SYSTEM, llid, broadcast_llid, eth_frame);
    }
    else
    {
        char szMinFrameBuf[MIN_ETHERNET_FRAME_SIZE];

        VOS_MemCpy(szMinFrameBuf, eth_frame, frame_len);
        VOS_MemZero(&szMinFrameBuf[frame_len], MIN_ETHERNET_FRAME_SIZE - frame_len);

        iRlt = PAS_send_frame(olt_id, MIN_ETHERNET_FRAME_SIZE, PON_PORT_SYSTEM, llid, broadcast_llid, szMinFrameBuf);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SendFrame2CNI(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, frame_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int PAS5201_GetVidDownlinkMode(short int olt_id, PON_vlan_tag_t vid, PON_olt_vid_downlink_config_t *vid_downlink_config)
{
    int iRlt = 0;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_get_vid_downlink_mode(olt_id, vid, vid_downlink_config);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetVidDownlinkMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
static int PAS5201_DelVidDownlinkMode(short int olt_id, PON_vlan_tag_t vid)
{
    int iRlt = 0;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_delete_vid_downlink_mode(olt_id, vid);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_DelVidDownlinkMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_GetOltParameters(short int olt_id, PON_olt_response_parameters_t *olt_parameters)
{
    int iRlt = 0;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_get_olt_parameters(olt_id, olt_parameters);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetOltParameters(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_SetOltIgmpSnoopingMode(short int olt_id, PON_olt_igmp_configuration_t *igmp_configuration)
{
    int iRlt = 0;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_set_olt_igmp_snooping_mode(olt_id, *igmp_configuration);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetOltIgmpSnoopingMode(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
static int PAS5001_GetOltIgmpSnoopingMode(short int olt_id, PON_olt_igmp_configuration_t *igmp_configuration)
{
    int iRlt = 0;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_get_olt_igmp_snooping_mode(olt_id, igmp_configuration);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetOltIgmpSnoopingMode(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
static int PAS5001_SetOltMldForwardingMode(short int olt_id, disable_enable_t mode)
{
    int iRlt = 0;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_set_olt_mld_forwarding_mode(olt_id, mode);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetOltMldForwardingMode(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
static int PAS5001_GetOltMldForwardingMode(short int olt_id, disable_enable_t *mode)
{
    int iRlt = 0;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_get_olt_mld_forwarding_mode(olt_id, mode);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetOltMldForwardingMode(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
static int PAS5001_SetDBAReportFormat(short int olt_id, PON_DBA_report_format_t report_format)
{
    int iRlt = 0;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_set_dba_report_format(olt_id, report_format);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetDBAReportFormat(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_GetDBAReportFormat(short int olt_id, PON_DBA_report_format_t *report_format)
{
    int iRlt = 0;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_get_dba_report_format(olt_id, report_format);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetDBAReportFormat(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------OLT LLID 管理API-------------- */

static int PAS5001_LLIDIsExist(short int olt_id, short int llid, bool *status)
{
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(status);
    
    *status = Onu_exist(olt_id, llid);
    return OLT_ERR_OK;
}

static int PAS5001_DeregisterLLID(short int olt_id, short int llid, bool iswait)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    
    iRlt = PAS_deregister_onu(olt_id, llid, iswait);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_DeregisterLLID(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_GetLLIDMac(short int olt_id, short int llid, mac_address_t onu_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_mac);
    
    iRlt = Get_onu_mac_address(olt_id, llid, onu_mac);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetLLIDMac(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_GetLLIDRegisterInfo(short int olt_id, short int llid, onu_registration_info_t *onu_info)
{
	int iRlt = OLT_ERR_NOTEXIST;
	onu_registration_data_record_t onu_registration_data;
	PON_onu_versions onu_version_info;
		
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_info);

    if ( 0 == (iRlt = PAS_get_onu_version ( olt_id, llid, &onu_version_info )) )
    {
        if ( 0 == (iRlt = PAS_get_onu_registration_data ( olt_id, llid, &onu_registration_data )) )
        {
            onu_info->oam_version = onu_registration_data.oam_version;
            onu_info->rtt = onu_registration_data.rtt;
            
            onu_info->laser_on_time  = onu_registration_data.laser_on_time;
            onu_info->laser_off_time = onu_registration_data.laser_off_time;

            onu_info->vendorType     = OnuVendorTypesPmc;
            onu_info->productVersion = onu_version_info.hardware_version;
            onu_info->productCode    = onu_info->productVersion;
        
            onu_info->max_links_support = 1;
            onu_info->curr_links_num = 1;
        
            onu_info->max_cm_support = 0;
            onu_info->pon_rate_flags = PON_RATE_NORMAL_1G;
        }
    }

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetLLIDRegisterInfo(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

static int PAS5001_AuthorizeLLID(short int olt_id, short int llid, bool auth_mode)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(olt_id);

    if ( auth_mode )
    {
        iRlt = PAS_authorize_onu(olt_id, llid, PON_AUTHORIZE_TO_THE_NETWORK);
    }
    else
    {
        iRlt = PAS_authorize_onu(olt_id, llid, PON_DENY_FROM_THE_NETWORK);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_AuthorizeLLID(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, auth_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int PAS5001_SetLLIDSLA(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(SLA);

    if ( PON_DISCOVERY_LLID == llid )
    {
        /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        llid = GW1G_PAS_BROADCAST_LLID;
    }

    iRlt = PLATO3_set_SLA(olt_id, llid, &SLA->SLA.SLA3, &SLA->DBA_ErrCode);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetLLIDSLA(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int PAS5001_GetLLIDSLA(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(SLA);

    iRlt = PLATO3_get_SLA(olt_id, llid, &SLA->SLA.SLA3, &SLA->DBA_ErrCode);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetLLIDSLA(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


static int PAS5001_SetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police)
{
    int iRlt;
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
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetLLIDPolice(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int PAS5001_GetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police)
{
    int iRlt;
    PON_policing_parameters_t *policing_params;
    PON_policing_struct_t  policing_struct;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(police);

    iRlt = PAS_get_policing_parameters_v4(olt_id, llid, police->path, &police->enable, &policing_struct);
    if ( 0 == iRlt )
    {
        policing_params = &police->params;
        policing_params->maximum_bandwidth               = policing_struct.maximum_bandwidth;
        policing_params->maximum_burst_size              = policing_struct.maximum_burst_size;
        policing_params->high_priority_frames_preference = policing_struct.high_priority_frames_preference;
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetLLIDPolice(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int PAS5201_SetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(police);

    iRlt = PAS_set_policing_parameters(olt_id, llid, police->path, police->enable, police->params);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetLLIDPolice(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int PAS5201_GetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(police);

    iRlt = PAS_get_policing_parameters(olt_id, llid, police->path, &police->enable, &police->params);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetLLIDPolice(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}


static int PAS5001_SetLLIDdbaType(short int olt_id, short int llid, int dba_type, short int *dba_error)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(dba_error);

    iRlt = PLATO3_set_DBA_type(olt_id, llid, dba_type, dba_error);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetLLIDFecMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, dba_type, *dba_error, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5001_GetLLIDdbaType(short int olt_id, short int llid, int *dba_type, short int *dba_error)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(dba_type);
    VOS_ASSERT(dba_error);

    iRlt = PLATO3_get_DBA_type(olt_id, llid, dba_type, dba_error);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetLLIDdbaType(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, *dba_type, *dba_error, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5001_SetLLIDdbaFlags(short int olt_id, short int llid, unsigned short dba_flags)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = PLATO3_set_plato3_llid_flags(olt_id, llid, dba_flags);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetLLIDdbaFlags(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, dba_flags, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5001_GetLLIDdbaFlags(short int olt_id, short int llid, unsigned short *dba_flags)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(dba_flags);

    iRlt = PLATO3_get_plato3_llid_flags(olt_id, llid, dba_flags);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetLLIDdbaFlags(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, *dba_flags, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

#endif


#if 1
/* -------------------OLT ONU 管理API-------------- */

static int PAS5001_GetAllOnus(short int olt_id, OLT_onu_table_t *onu_table)
{
    int iRlt;
    short int onu_num;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(onu_table);

    if ( 0 == (iRlt = PAS_get_onu_parameters(olt_id, &onu_num, onu_table->onus)) )
    {
        onu_table->onu_num = onu_num;
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetAllOnus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS_ResumeAllOnuStatus(short int olt_id, int olt_chipid, int resume_reason, int resume_mode)
{
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

    /* 恢复原PON口上的ONU在线状态信息及补报ONU日志 */
#ifdef OLT_RECOVERY_BY_EVENT
    if ( (OLT_RESUMEMODE_SYNCSOFT == resume_mode) && SYS_LOCAL_MODULE_ISMASTERACTIVE )
    {
        if ( 0 == PAS_get_onu_parameters(olt_id, &onu_num, onu_list) )
        {
            PON_llid_parameters_t llid_params;
            onu_registration_data_record_t reg_data;
                
            if ( onu_num > 0 )
            {
                if ( 0 < OLTAdv_GetOpticalTxMode(olt_id) )
                {
                    for (i=0; i<onu_num; ++i)
                    {
                        ++resume_uk_num;
                        llid = onu_list[i].llid;
                        
                        if ( 0 == Get_llid_parameters(olt_id, llid, &llid_params, &reg_data, SequenceNo) )
                        {
                            ++resume_up_num;
                            OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate llid%d's register event for sync-soft.\r\n", olt_id, llid);

                            OAM_Ver = llid_params.oam_version;
                            sendOnuRegistrationMsg( olt_id, llid, onu_list[i].mac_address, SequenceNo, OAM_Ver, ONU_EVENTFLAG_VIRTAL );
                            if ( IsSupportCTCOnu( olt_id ) == TRUE /* GetOnuVendorType( olt_id, OnuIdx ) == ONU_VENDOR_CT */ )
                            {
                                sendOnuExtOamDiscoveryMsg(olt_id, llid, CTC_DISCOVERY_STATE_COMPLETE, PAS_oui_version_records_num, PAS_oui_version_records_list, ONU_EVENTFLAG_VIRTAL);
                            }
                        }
                        else
                        {
                            ++resume_er_num;
                            OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d force llid%d re-register for sync-soft.\r\n", olt_id, llid);

                            PAS_deregister_onu( olt_id, llid, FALSE );
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
                        if (!SYS_LOCAL_MODULE_ISMASTERSTANDBY)
#else
                        if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
#endif
                        {
                            if ( onu_num < 0 )
                            {
                                /* 得到在线ONU的MAC-LLID列表 */
                                if ( 0 != PAS_get_onu_parameters(olt_id, &onu_num, onu_list) )
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

                                                PAS_deregister_onu( olt_id, llid, FALSE );
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
                                PON_llid_parameters_t llid_params;
                                onu_registration_data_record_t reg_data;
                                
                                if ( 0 == Get_llid_parameters(olt_id, llid, &llid_params, &reg_data, SequenceNo) )
                                {
                                    VOS_MemCpy( MacAddr, llid_params.mac_address, BYTES_IN_MAC_ADDRESS );
                                    OAM_Ver = llid_params.oam_version;
                                }
                                else
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
#ifndef BCM_WARM_BOOT_SUPPORT
	                            {
                           			/* B--modified by liwei056@2012-8-23 for D15700inGFA6700 */
	                                if ( ONU_OPER_STATUS_UP == status )
	                                {
	                                    /* 备用状态UP的ONU，直接恢复其接口即可，无需重新注册 */
	                                    if( GetOnuVendorType( olt_id, OnuIdx ) == ONU_VENDOR_CT )
	                                    {
	                                        ONU_SetupIFsByType(olt_id, OnuIdx, olt_chipid, (int)ONU_MANAGE_CTC);
	                                    }
	                                    else
	                                    {
	                                        ONU_SetupIFsByType(olt_id, OnuIdx, olt_chipid, (int)ONU_MANAGE_GW);
	                                    }
	                                }
	                                else
	                                {
                                    /* 备用状态不明的ONU，都强制其入Pending状态，等待重新注册 */
	                                    AddPendingOnu(olt_id, OnuIdx, llid, MacAddr, 0);
	                                }
									/* E--modified by liwei056@2012-8-23 for D15700inGFA6700 */
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
                        if ( 0 != PAS_get_onu_parameters(olt_id, &onu_num, onu_list) )
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
                                        PAS_deregister_onu( olt_id, llid, FALSE );
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
										SetNeedToRestoreConfFile(olt_id, OnuIdx, FALSE);/*added by wangjiah@2017-01-18 for no need to restore conf file while quick swaping*/
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
    if ( checkin_num <  onu_num || (checkin_num == onu_num && resume_mode == OLT_RESUMEMODE_SYNCSOFT))
#else
    if ( checkin_num < onu_num )
#endif
    {

        /* ONU MAC列表里不存在的在线ONU的注册状态恢复 */
        PON_llid_parameters_t llid_params;
        onu_registration_data_record_t reg_data;
        for (i=0; i<onu_num; ++i)
        {
#ifdef BCM_WARM_BOOT_SUPPORT
         	if ( onu_list[i].rtt >= 0 || (resume_mode == OLT_RESUMEMODE_SYNCSOFT))
#else
            if ( onu_list[i].rtt >= 0 )
#endif
            {

            
                ++resume_uk_num;
                llid = onu_list[i].llid;
                
                if ( 0 == Get_llid_parameters(olt_id, llid, &llid_params, &reg_data, SequenceNo) )
                {
                    ++resume_up_num;
                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate llid%d's register event for register-loose.\r\n", olt_id, llid);

                    OAM_Ver = llid_params.oam_version;

                    sendOnuRegistrationMsg( olt_id, llid, onu_list[i].mac_address, SequenceNo, OAM_Ver, ONU_EVENTFLAG_VIRTAL );
                    if ( IsSupportCTCOnu( olt_id ) == TRUE /* GetOnuVendorType( olt_id, OnuIdx ) == ONU_VENDOR_CT */ )
                    {
                        sendOnuExtOamDiscoveryMsg(olt_id, llid, CTC_DISCOVERY_STATE_COMPLETE, PAS_oui_version_records_num, PAS_oui_version_records_list, ONU_EVENTFLAG_VIRTAL);
                    }
                }
                else
                {
                    ++resume_er_num;
                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d force llid%d re-register for register-loose.\r\n", olt_id, llid);

                    PAS_deregister_onu( olt_id, llid, FALSE );
                }
            }
        }    
    }

    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"PAS_ResumeAllOnuStatus(%d, %d, %d, %d)'s result(up:%d, down:%d, err:%d, unknown:%d) on slot %d.\r\n", olt_id, olt_chipid, resume_reason, resume_mode, resume_up_num, resume_dn_num, resume_er_num, resume_uk_num, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

static int PAS5001_ResumeAllOnuStatus(short int olt_id, int resume_reason, int resume_mode)
{
    int iRlt = PAS_ResumeAllOnuStatus(olt_id, ponChipType_PAS5001, resume_reason, resume_mode);

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_ResumeAllOnuStatus(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, resume_reason, resume_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int PAS5201_ResumeAllOnuStatus(short int olt_id, int resume_reason, int resume_mode)
{
    int iRlt = PAS_ResumeAllOnuStatus(olt_id, ponChipType_PAS5201, resume_reason, resume_mode);

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_ResumeAllOnuStatus(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, resume_reason, resume_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int PAS5204_ResumeAllOnuStatus(short int olt_id, int resume_reason, int resume_mode)
{
    int iRlt = PAS_ResumeAllOnuStatus(olt_id, ponChipType_PAS5204, resume_reason, resume_mode);

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5204_ResumeAllOnuStatus(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, resume_reason, resume_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

/*Begin:for onu swap by jinhl@2013-02-22*/
static int PAS5001_ResumeLLIDStatus(short int olt_id, short int llid, int resume_reason, int resume_mode)
{
#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
    extern CTC_STACK_oui_version_record_t PAS_oui_version_records_list[];
    extern int PAS_oui_version_records_num;
    int OnuEntry, OnuIdx;
    int i;
    int resume_up_num, resume_dn_num, resume_er_num, resume_uk_num;
    short int checkin_num, onu_num;
    short int llid_inmgmt, ponIsOK;
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
   
    OnuIdx = GetOnuIdxByLlid(olt_id, llid);
    if((RERROR != OnuIdx) && (ThisIsValidOnu(olt_id, OnuIdx) == ROK))
    {
        ONU_MGMT_SEM_TAKE;
		OnuEntry = olt_id*MAXONUPERPON + OnuIdx;
        VOS_MemCpy( MacAddr, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS );
        llid_inmgmt= OnuMgmtTable[OnuEntry].LLID;
    	status = OnuMgmtTable[OnuEntry].OperStatus;
        ONU_MGMT_SEM_GIVE;
		
        switch (resume_mode)
        {
            case OLT_RESUMEMODE_SYNCSOFT:
                               
                break;
            case OLT_RESUMEMODE_FORCEDOWN:
				
				{
					OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d onu%d's status%d OLT_RESUMEMODE_FORCEDOWN in onuswap.\r\n", olt_id, OnuIdx+1, status);				   
					if ( (ONU_OPER_STATUS_UP == status))
	                {
	                    ++resume_dn_num;
	                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event for user-force in onuswap.\r\n", olt_id, OnuIdx+1);

	                    if (INVALID_LLID == llid)
	                    {
	                        ++resume_uk_num;
	                        sendOnuDeregistrationEvent( olt_id, OnuIdx, resume_reason, ONU_EVENTFLAG_VIRTAL );
	                    }
	                    else
	                    {
	                        #if 0/*调试发现，onu拔掉一个光模块连线。实际pas_soft却未能去注册，需手动离线一下*/
                            
							(void)PAS_deregister_onu(olt_id, llid, 0);   /* ONU的虚实变化由PMC自己掌控，无须我们插手 */
                            sys_console_printf("deregister local onu\r\n");
							OLT_SetRdnLLIDMode(olt_id, llid, PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL);
							#else
	                        sendOnuDeregistrationMsg( olt_id, llid, resume_reason, ONU_EVENTFLAG_VIRTAL );
							#endif
							
						}
	                }
					else 
					{
					    sys_console_printf("src onu %d/%d is pending,will active\r\n",olt_id, OnuIdx+1);
						if ( (0 != CheckOnuIsInPendingQueue(olt_id, MacAddr))
                                || (0 != CheckOnuIsInPendingConfQueue(olt_id, MacAddr)) )
						{
							activeOneLocalPendingOnu(olt_id, OnuIdx);
							return VOS_ERROR;
						}
						
	                }
				}
                break;
            case OLT_RESUMEMODE_FORCEUP:
                
                break;
            case OLT_RESUMEMODE_SYNCHARD:
                if ( onu_num < 0 )
                {
                    if ( 0 != PAS_get_onu_parameters(olt_id, &onu_num, onu_list) )
                    {
                        OLT_SYNC_DEBUG(OLT_SYNC_TITLE"PAS_get_onu_parameters err.\r\n");
                        onu_num = 0;
                    }
                }
				OLT_SYNC_DEBUG(OLT_SYNC_TITLE"onu_num:%d.\r\n",onu_num);
        
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
                                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d force llid%d re-register for sync-hard in onuswap.\r\n", olt_id, llid);

                                    ++resume_er_num;
                                    PAS_deregister_onu( olt_id, llid, FALSE );
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

				OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's status%d OLT_RESUMEMODE_SYNCHARD.\r\n",olt_id, OnuIdx+1, status);

                if ( ONU_OPER_STATUS_UP == status )
                {
                    if ( INVALID_LLID == llid )
                    {
                        ++resume_dn_num;
                        OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event for sync-hard in onuswap.\r\n", olt_id, OnuIdx+1);

                        sendOnuDeregistrationEvent( olt_id, OnuIdx, resume_reason, ONU_EVENTFLAG_VIRTAL );
                    }
                }
                else
                {
                    if (INVALID_LLID != llid)
                    {
                        if ( (ONU_OPER_STATUS_UP!= status))
                        {
                            if ( (0 == CheckOnuIsInPendingQueue(olt_id, MacAddr))
                                && (0 == CheckOnuIsInPendingConfQueue(olt_id, MacAddr)) )
                            {
                                ++resume_up_num;
                                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's register event for sync-hard in onuswap.\r\n", olt_id, OnuIdx+1);

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
                
                break;
            default:
                VOS_ASSERT(0);
        }
    }
	else
	{
	    sys_console_printf("ResumeLLIDStatus, GetOnuIdxByLlid err\r\n");
	    PAS_deregister_onu( olt_id, llid, FALSE );
	}
   
    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"PAS5001_ResumeLLIDStatus(%d, %d, %d, %d)'s result(up:%d, down:%d, err:%d, unknown:%d) on slot %d.\r\n", olt_id, llid, resume_reason, resume_mode, resume_up_num, resume_dn_num, resume_er_num, resume_uk_num, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
#else
    return OLT_ERR_NOTSUPPORT;
#endif
}



/*End:for onu swap by jinhl@2013-02-22*/

static int PAS5001_SetOnuAuthMode(short int olt_id, int auth_switch)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    
#if 0
    /* 目前，只有CTC的ONU支持硬件静默 (5001不支持CTC)*/
    iRlt = PAS_set_authorize_mac_address_according_list_mode(olt_id, (bool)auth_switch);
#else
    /* 使用软件认证表，来实现OLT端的ONU静默 */
    iRlt = 0;
#endif
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetOnuAuthMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, auth_switch, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int PAS5201_SetOnuAuthMode(short int olt_id, int auth_switch)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    if ( V2R1_CTC_STACK == TRUE )
    {
        /* 目前，只有CTC的ONU支持硬件静默 */
        iRlt = PAS_set_authorize_mac_address_according_list_mode(olt_id, (bool)auth_switch);
    }
    else
    {
        /* 使用软件认证表，来实现OLT端的ONU静默 */
        iRlt = 0;
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetOnuAuthMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, auth_switch, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}
/* added by xieshl 20110330, 问题单12139 */
extern ULONG	gLibEponMibSemId;
int PAS5201_SetAllOnuAuthMode2(short int olt_id, int enable)
{
    int iRlt = OLT_ERR_OK;
    short int ul_slot = GetCardIdxByPonChip(olt_id);
    short int ul_port = GetPonPortByPonChip(olt_id);
    int auth_mode = 0;
    int OnuIdx = 0;
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
                    	for( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
                    	{
                            int OnuStatus = GetOnuOperStatus_1(olt_id, OnuIdx);
                            if(OnuStatus == ONU_OPER_STATUS_UP)	
                            {
                                int llid = GetLlidByOnuIdx(olt_id, OnuIdx);
                                if(llid != INVALID_LLID)
                            		PAS_deregister_onu( olt_id, llid, FALSE );
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
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetAllOnuAuthMode2(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

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

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetAllOnuAuthMode2(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);
#endif
    return iRlt;
}

static int PAS5201_SetMacAuth(short int olt_id, int mode, mac_address_t mac)
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
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetMacAuth(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_SetOnuDownlinkPri2CoSQueueMap(short int olt_id, OLT_pri2cosqueue_map_t *map)
{
    int iRlt;
    PON_high_priority_frames_t high_priority_frames;
    unsigned char i;
			
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(map);

    for(i=0; i<=MAX_PRIORITY_QUEUE; i++)
    {
        high_priority_frames.priority[i] = (bool)map->priority[i];
    }		

    iRlt = PAS_set_policing_thresholds(olt_id, PON_POLICER_DOWNSTREAM_TRAFFIC, high_priority_frames, PON_POLICER_MAX_HIGH_PRIORITY_RESERVED);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetOnuDownlinkPri2CoSQueueMap(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* 激活ONU */
static int PAS5001_ActivePendingOnu(short int olt_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    pendingOnu_S *CurOnu;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
    CurOnu = PonPortTable[olt_id].PendingOnu.Next;
    while( CurOnu != NULL )
    {		
        (void)PAS_deregister_onu( olt_id, CurOnu->Llid, FALSE );
        iRlt = 0;
        
        CurOnu = CurOnu->Next;
    }
    VOS_SemGive( OnuPendingDataSemId );

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_ActivePendingOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, CurOnu->Llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;	
}

/* 激活一个ONU */
static int PAS5001_ActiveOnePendingOnu(short int olt_id, UCHAR *mac)
{
    int iRlt = OLT_ERR_NOTEXIST;
    pendingOnu_S *CurOnu;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(mac);

    VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
    CurOnu = PonPortTable[olt_id].PendingOnu.Next;
    while( CurOnu != NULL )
    {		
        if( VOS_MemCmp(CurOnu->OnuMarkInfor.OnuMark.MacAddr, mac,6)== 0 )
        {
            (void)PAS_deregister_onu( olt_id, CurOnu->Llid, FALSE );
            iRlt = 0;
            break;
        }
        CurOnu = CurOnu->Next;
    }	
    VOS_SemGive( OnuPendingDataSemId );

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_ActivateOnePendingOnu(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_ActiveConfPendingOnu(short int olt_id, short int conf_olt_id)
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
                (void)PAS_deregister_onu( olt_id, CurOnu->Llid, FALSE );
                iRlt = 0;
            }

            CurOnu = CurOnu->Next;
        }
        VOS_SemGive( OnuPendingDataSemId );
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS_ActiveConfPendingOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, conf_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_ActiveOneConfPendingOnu(short int olt_id, short int conf_olt_id, mac_address_t mac)
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
                (void)PAS_deregister_onu( olt_id, CurOnu->Llid, FALSE );
                iRlt = 0;
                
                break;					
            }

            CurOnu = CurOnu->Next;
        }
        VOS_SemGive( OnuPendingDataSemId );
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS_ActiveOneConfPendingOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, conf_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int PAS5001_GetOnuMode(short int olt_id, short int llid)
{
    int iRlt = RERROR;
    OLT_LOCALID_ASSERT(olt_id);

    iRlt = PAS_get_onu_mode(olt_id, llid);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetOnuMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	return iRlt;
	
}
static int PAS5001_GetMACAddressAuthentication(short int olt_id, unsigned char	*number_of_mac_address, mac_addresses_list_t mac_addresses_list)
{
    int iRlt = RERROR;
    OLT_LOCALID_ASSERT(olt_id);

	iRlt = PAS_get_mac_address_authentication(olt_id, number_of_mac_address, mac_addresses_list);
	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetMACAddressAuthentication(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *number_of_mac_address, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	return iRlt;
}




static int PAS5001_SetAuthorizeMacAddressAccordingListMode(short int olt_id, bool	authentication_according_to_list)
{
     int iRlt = RERROR;
     OLT_LOCALID_ASSERT(olt_id);

	 iRlt = PAS_set_authorize_mac_address_according_list_mode(olt_id, authentication_according_to_list);

	 OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetAuthorizeMacAddressAccordingListMode(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	 return iRlt;
}

static int PAS5001_GetAuthorizeMacAddressAccordingListMode(short int olt_id, bool	*authentication_according_to_list)
{
     int iRlt = RERROR;
     OLT_LOCALID_ASSERT(olt_id);

	 iRlt = PAS_get_authorize_mac_address_according_list_mode(olt_id, authentication_according_to_list);

	 OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetAuthorizeMacAddressAccordingListMode(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	 return iRlt;
}

static int PAS5001_GetDownlinkBufferConfiguration(short int olt_id, PON_downlink_buffer_priority_limits_t *priority_limits)
{
     int iRlt = RERROR;
     OLT_LOCALID_ASSERT(olt_id);

	 iRlt = PAS_get_downlink_buffer_configuration(olt_id, priority_limits);

	 OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetDownlinkBufferConfiguration(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	 return iRlt;
}

static int PAS5001_GetOamInformation(short int olt_id, short int llid, PON_oam_information_t  *oam_information)
{
     int iRlt = RERROR;
     OLT_LOCALID_ASSERT(olt_id);

	 iRlt = PAS_get_oam_information(olt_id, llid, oam_information);

	 OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetOamInformation(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	 return iRlt;
}

/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

#endif


#if 1
/* -------------------OLT 加密管理API----------- */

static int PAS5001_SetEncryptMode(short int olt_id, int encrypt_mode)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

    if ( PON_ENCRYPTION_TYPE_AES == encrypt_mode )
    {
        /* 5001只支持AES，且无法设置 */
        /* iRlt = PAS_set_encryption_configuration(olt_id, encrypt_mode); */
        iRlt = 0;
    }
    else
    {
        iRlt = OLT_ERR_NOTSUPPORT;
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetEncryptMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, encrypt_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_SetEncryptMode(short int olt_id, int encrypt_mode)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

    iRlt = PAS_set_encryption_configuration(olt_id, encrypt_mode);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetEncryptMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, encrypt_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int PAS5201_SetEncryptionPreambleMode(short int olt_id, bool encrypt_mode)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

	
    iRlt = PAS_set_encryption_preamble_mode(olt_id, encrypt_mode);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetEncryptionPreambleMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, encrypt_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_GetEncryptionPreambleMode(short int olt_id, bool *encrypt_mode)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

	
    iRlt = PAS_get_encryption_preamble_mode(olt_id, encrypt_mode);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetEncryptionPreambleMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *encrypt_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/


static int PAS5001_GetLLIDEncryptMode(short int olt_id, short int llid, bool *encrypt_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(encrypt_mode);

    iRlt = Get_encryption_mode(olt_id, llid, encrypt_mode);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetLLIDEncryptMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, *encrypt_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5001_StartLLIDEncrypt(short int olt_id, short int llid)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = PAS_start_olt_encryption(olt_id, llid);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_StartLLIDEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5001_FinishLLIDEncrypt(short int olt_id, short int llid, short int status)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = PAS_finalize_start_olt_encryption(olt_id, llid, status);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_FinishLLIDEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5001_StopLLIDEncrypt(short int olt_id, short int llid)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = PAS_stop_olt_encryption(olt_id, llid);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_StopLLIDEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5001_SetLLIDEncryptKey(short int olt_id, short int llid, PON_encryption_key_t key)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(key);

    iRlt = PAS_set_olt_encryption_key(olt_id, llid, key);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetLLIDEncryptKey(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5001_FinishLLIDEncryptKey(short int olt_id, short int llid, short int status)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = PAS_finalize_set_olt_encryption_key(olt_id, llid, status);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_FinishLLIDEncryptKey(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

#endif


#if 1
/* -------------------OLT 地址表管理API-------- */

static int PAS5001_SetMacAgingTime(short int olt_id, int aging_time)
{
    int iRlt;
	PON_olt_update_parameters_t  pon_updated_parameters;	

    OLT_LOCAL_ASSERT(olt_id);
    
	PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT(pon_updated_parameters);
	pon_updated_parameters.address_table_aging_timer = aging_time;
    iRlt = PAS_update_olt_parameters_v4(olt_id, &pon_updated_parameters );
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetMacAgingTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, aging_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int PAS5201_SetMacAgingTime(short int olt_id, int aging_time)
{
    int iRlt;
	PON_address_table_config_t  add_table_config;
	
    OLT_LOCAL_ASSERT(olt_id);
    
	iRlt = PAS_get_address_table_configuration(olt_id, &add_table_config);
	if( iRlt == PAS_EXIT_OK )
    {
    	add_table_config.aging_timer = aging_time;
        iRlt = PAS_set_address_table_configuration(olt_id, add_table_config);
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetMacAgingTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, aging_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int PAS5201_SetAddressTableConfig(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg)
{
    int iRlt;
    PON_address_table_config_t address_table_config;	
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addrtbl_cfg);
    
    if ( PAS_EXIT_OK == (iRlt = PAS_set_address_table_full_handling_mode(olt_id, addrtbl_cfg->removed_when_full)) )
    {
        if ( PAS_EXIT_OK == (iRlt = PAS_get_address_table_configuration(olt_id, &address_table_config)) )
        {
            address_table_config.discard_llid_unlearned_sa = (bool)addrtbl_cfg->discard_llid_unlearned_sa;
            address_table_config.discard_unknown_da = (bool)addrtbl_cfg->discard_unknown_da;
            if (addrtbl_cfg->discard_unknown_da >= 0)
            {
                address_table_config.discard_unknown_da = addrtbl_cfg->discard_unknown_da;
            }
            if (addrtbl_cfg->aging_timer >= 0)
            {
                address_table_config.aging_timer = addrtbl_cfg->aging_timer;
            }
            if (addrtbl_cfg->allow_learning >= 0)
            {
                if ( V2R1_ENABLE == addrtbl_cfg->allow_learning )
                {
                    address_table_config.allow_learning = PON_DIRECTION_UPLINK;
                }
                else
                {
                    address_table_config.allow_learning = PON_DIRECTION_NO_DIRECTION;
                }
            }
        	iRlt = PAS_set_address_table_configuration(olt_id, address_table_config);
        }
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetAddressTableConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, addrtbl_cfg->removed_when_full, addrtbl_cfg->discard_llid_unlearned_sa, addrtbl_cfg->discard_unknown_da, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int PAS5201_GetAddressTableConfig(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg)
{
    int iRlt;
    bool bValue;
    PON_address_table_config_t address_table_config;	
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addrtbl_cfg);
    
    if ( PAS_EXIT_OK == (iRlt = PAS_get_address_table_full_handling_mode(olt_id, &bValue)) )
    {
        addrtbl_cfg->removed_when_full = (char)bValue;
        if ( PAS_EXIT_OK == (iRlt = PAS_get_address_table_configuration(olt_id, &address_table_config)) )
        {
            addrtbl_cfg->discard_llid_unlearned_sa = address_table_config.discard_llid_unlearned_sa;
            addrtbl_cfg->discard_unknown_da = address_table_config.discard_unknown_da;
            addrtbl_cfg->removed_when_aged = address_table_config.removed_when_aged;
            addrtbl_cfg->allow_learning = address_table_config.allow_learning;
            addrtbl_cfg->aging_timer = address_table_config.aging_timer;
        }
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetAddressTableConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, addrtbl_cfg->removed_when_full, addrtbl_cfg->discard_llid_unlearned_sa, addrtbl_cfg->discard_unknown_da, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int PAS5001_GetMacAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num);

    if ( NULL == addr_tbl )
    {
        addr_tbl = Mac_addr_table;
    }

    if ( 0 == (iRlt = PAS_get_address_table(olt_id, addr_num, addr_tbl)) )
    {
        if ( NULL != addr_tbl )
        {
            int i, n;

            n = *addr_num;
            for (i=0; i<n; i++)
            {
                if ( PAS_ADDRESS_TABLE_RECORD_SYSTEM_LLID == addr_tbl[i].logical_port )
                {
                    addr_tbl[i].logical_port = PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID;
                }
            }
        }
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetMacAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_AddMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;
    int i;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num != 0);
    VOS_ASSERT(addr_tbl);

    if ( addr_num < 0 )
    {
        addr_num = -addr_num;
    }

    for (i=0; i<addr_num; i++)
    {
        if ( PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID == addr_tbl[i].logical_port )
        {
            addr_tbl[i].logical_port = PAS_ADDRESS_TABLE_RECORD_SYSTEM_LLID;
        }
    
        if ( 0 != (iRlt = PAS_add_address_table_record(olt_id, &addr_tbl[i])) )
        {
            break;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_AddMacAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_AddMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;
    int i;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num != 0);
    VOS_ASSERT(addr_tbl);

    if ( addr_num < 0 )
    {
        addr_num = -addr_num;
    }

    for (i=0; i<addr_num; i++)
    {
        if ( PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID == addr_tbl[i].logical_port )
        {
            addr_tbl[i].logical_port = PAS_ADDRESS_TABLE_RECORD_SYSTEM_LLID;
        }
    }

    iRlt = PAS_add_address_table_multi_records(olt_id, addr_num, addr_tbl);

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_AddMacAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5001_DelMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt = 0;
    int i;
    short int sRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num != 0);
    VOS_ASSERT(addr_tbl);

    if ( addr_num < 0 )
    {
        addr_num = -addr_num;
    }
   
    for (i=0; i<addr_num; i++)
    {
        sRlt = PAS_remove_address_table_record(olt_id, addr_tbl[i].mac_address);
        if ( (0 != sRlt) && (-1 != sRlt) )
        {
            iRlt = sRlt;
            break;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_DelMacAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5001_RemoveMac(short int olt_id, mac_address_t mac)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(mac);

    iRlt = PAS_remove_address_table_record(olt_id, mac);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_RemoveMac(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_ResetAddrTbl(short int olt_id, short int llid, int addr_type)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = PAS_reset_address_table(olt_id, llid, addr_type);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_ResetAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, addr_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int PAS5001_SetAddressTableFullHandlingMode(short int olt_id, bool remove_entry_when_table_full)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_set_address_table_full_handling_mode(olt_id, remove_entry_when_table_full);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetAddressTableFullHandlingMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, remove_entry_when_table_full, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt; 
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------OLT 光路管理API----------- */

static int PAS5001_GetOpticalCapability(short int olt_id, OLT_optical_capability_t *optical_capability)
{
    int iRlt;
    PAS_physical_capabilities_t device_capabilities;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(optical_capability);

    VOS_MemZero(optical_capability, sizeof(OLT_optical_capability_t));
    if ( 0 == (iRlt = PAS_get_device_capabilities_v4(olt_id, PON_OLT_ID, &device_capabilities)) )
    {
        optical_capability->pon_tx_signal  = device_capabilities.pon_tx_signal;
        optical_capability->optical_capabilitys = PON_OPTICAL_CAPABILITY_LOCKTIME | PON_OPTICAL_CAPABILITY_LASERTIME;
        optical_capability->agc_lock_time  = device_capabilities.agc_lock_time;
        optical_capability->cdr_lock_time  = device_capabilities.cdr_lock_time;
        optical_capability->laser_on_time  = device_capabilities.laser_on_time;
        optical_capability->laser_off_time = device_capabilities.laser_off_time;
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetOpticalCapability(%d, %d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, device_capabilities.agc_lock_time, device_capabilities.cdr_lock_time, device_capabilities.laser_on_time, device_capabilities.laser_off_time, device_capabilities.pon_tx_signal, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_GetOpticalCapability(short int olt_id, OLT_optical_capability_t *optical_capability)
{
    int iRlt;
    PON_olt_optics_configuration_t pon_optics_params;
    bool pon_tx_signal;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(optical_capability);

    VOS_MemZero(optical_capability, sizeof(OLT_optical_capability_t));
    if ( 0 == (iRlt = PAS_get_olt_optics_parameters(olt_id, &pon_optics_params, &pon_tx_signal)) )
    {
        optical_capability->pon_tx_signal = pon_tx_signal;
        optical_capability->optical_capabilitys = PON_OPTICAL_CAPABILITY_LOCKTIME;
        optical_capability->agc_lock_time = pon_optics_params.agc_lock_time;
        optical_capability->cdr_lock_time = pon_optics_params.cdr_lock_time;
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetOpticalCapability(%d, %d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, pon_optics_params.agc_lock_time, pon_optics_params.cdr_lock_time, pon_optics_params.discovery_laser_on_time, pon_optics_params.discovery_laser_off_time, pon_tx_signal, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_GetOpticsDetail(short int olt_id, OLT_optics_detail_t *optics_params) 
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(optics_params);

    iRlt = PAS_get_olt_optics_parameters(olt_id, &optics_params->pon_optics_params, &optics_params->pon_tx_signal);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetOpticsDetail(%d, %d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, optics_params->pon_optics_params.agc_lock_time, optics_params->pon_optics_params.cdr_lock_time, optics_params->pon_optics_params.discovery_laser_on_time, optics_params->pon_optics_params.discovery_laser_off_time, optics_params->pon_tx_signal, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_SetPonRange(short int olt_id, unsigned int max_range, unsigned int max_rtt)
{
    int iRlt;
    PON_olt_update_parameters_t rtt_cfg;
        
    OLT_LOCAL_ASSERT(olt_id);

	PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT( rtt_cfg );
	rtt_cfg.max_rtt = max_rtt;
    iRlt = PAS_update_olt_parameters_v4(olt_id, &rtt_cfg);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetPonRange(%d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, max_range, max_rtt, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_SetPonRange(short int olt_id, unsigned int max_range, unsigned int max_rtt)
{
    int iRlt = OLT_ERR_OK;
    PON_update_olt_parameters_t rtt_cfg;
        
    OLT_LOCAL_ASSERT(olt_id);


    PON_EMPTY_UPDATE_OLT_PARAMETERS_STRUCT( rtt_cfg );

#if 0 
	rtt_cfg.hec_configuration.tx_hec_configuration = PON_TX_HEC_802_AH_MODE;
    rtt_cfg.hec_configuration.rx_hec_configuration = PON_TX_HEC_802_AH_MODE;
	rtt_cfg.grant_filtering = PON_VALUE_NOT_CHANGED;
	rtt_cfg.support_passave_onus = PON_VALUE_NOT_CHANGED;
#endif

	rtt_cfg.max_rtt = max_rtt;
	
    iRlt = PAS_update_olt_parameters(olt_id, &rtt_cfg);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetPonRange(%d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, max_range, max_rtt, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_SetOpticalTxMode(short int olt_id, int tx_mode)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

    iRlt = PAS_set_olt_pon_transmission(olt_id, (bool)tx_mode);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetOpticalTxMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, tx_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_GetOpticalTxMode(short int olt_id, int *tx_mode)
{
    int iRlt;
    PAS_physical_capabilities_t device_capabilities;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(tx_mode);

    if ( OLT_ERR_OK == (iRlt = PAS_get_device_capabilities_v4(olt_id, PON_OLT_ID, &device_capabilities)) )
    {
        bool pon_tx_enable = device_capabilities.pon_tx_signal;

        if ( pon_tx_enable )    
        {
            *tx_mode = 1;
        }
        else
        {
            *tx_mode = 0;
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetOpticalTxMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *tx_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_GetOpticalTxMode(short int olt_id, int *tx_mode)
{
    int iRlt;
    bool pon_tx_enable;
    PON_olt_optics_configuration_t olt_optics_param;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(tx_mode);
    
    if ( 0 == (iRlt = PAS_get_olt_optics_parameters(olt_id, &olt_optics_param, &pon_tx_enable)) )
    {
        if ( pon_tx_enable )    
        {
            *tx_mode = 1;
        }
        else
        {
            *tx_mode = 0;
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetOpticalTxMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *tx_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_SetVirtualScopeAdcConfig(short int olt_id, PON_adc_config_t *adc_config)
{
    int iRlt;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_set_virtual_scope_adc_config(olt_id, *adc_config);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetVirtualScopeAdcConfig(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_GetVirtualScopeMeasurement(short int olt_id, short int llid, PON_measurement_type_t measurement_type, 
	void *configuration, short int config_len, void *result, short int res_len)
{
    int iRlt;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_get_virtual_scope_measurement(olt_id, llid, measurement_type, configuration, result);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetVirtualScopeMeasurement(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_GetVirtualScopeRssiMeasurement(short int olt_id, short int llid, PON_rssi_result_t *rssi_result)
{
    int iRlt;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_get_virtual_scope_rssi_measurement(olt_id, llid, rssi_result);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetVirtualScopeRssiMeasurement(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, (int)rssi_result->dbm, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_GetVirtualScopeOnuVoltage(PON_olt_id_t olt_id, short int llid, float *voltage,unsigned short int *sample, float *dbm)
{
    int iRlt = 0;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = virtual_scope_get_onu_voltage(olt_id, llid, voltage, sample, dbm);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetVirtualScopeOnuVoltage(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, (int)*dbm, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_SetVirtualLLID(short int olt_id, short int llid, PON_virtual_llid_operation_t operation)
{
    int iRlt = RERROR;
    OLT_LOCALID_ASSERT(olt_id);

	iRlt = PAS_set_virtual_llid(olt_id, llid, operation);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetVirtualLLID(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	 return iRlt;
}

static int PAS5001_SetOpticalTxMode2(short int olt_id, int tx_mode, int tx_reason)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);

    if ( 0 < (iRlt = GW_SetOpticalTxMode2(olt_id, tx_mode, tx_reason)) )
    {
        iRlt = PAS_set_olt_pon_transmission(olt_id, (bool)tx_mode);
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetOpticalTxMode2(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, tx_mode, tx_reason, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif


#if 1
/* -------------------OLT 监控统计管理API---- */

static int PAS5001_GetRawStatistics(short int olt_id, OLT_raw_stat_item_t *stat_item)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(stat_item);
    VOS_ASSERT(stat_item->statistics_data);
    VOS_ASSERT(stat_item->statistics_data_size > 0);

	if(GW10G_PAS_BROADCAST_LLID == stat_item->statistics_parameter)
	{
	    stat_item->statistics_parameter = GW1G_PAS_BROADCAST_LLID;
	}
    iRlt = PAS_get_raw_statistics_v4(olt_id, stat_item->collector_id
            , stat_item->raw_statistics_type, stat_item->statistics_parameter
            , PON_STATISTICS_QUERY_HARDWARE
            , stat_item->statistics_data, &stat_item->timestam);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetRawStatistics(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, stat_item->collector_id, stat_item->raw_statistics_type, stat_item->statistics_parameter, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_GetRawStatistics(short int olt_id, OLT_raw_stat_item_t *stat_item)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(stat_item);
    VOS_ASSERT(stat_item->statistics_data);
    VOS_ASSERT(stat_item->statistics_data_size > 0);

	if(GW10G_PAS_BROADCAST_LLID == stat_item->statistics_parameter)
	{
	    stat_item->statistics_parameter = GW1G_PAS_BROADCAST_LLID;
	}
    iRlt = PAS_get_raw_statistics(olt_id, stat_item->collector_id
            , stat_item->raw_statistics_type, stat_item->statistics_parameter
            , stat_item->statistics_data, &stat_item->timestam);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetRawStatistics(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, stat_item->collector_id, stat_item->raw_statistics_type, stat_item->statistics_parameter, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_ResetCounters(short int olt_id)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    
    iRlt = PAS_reset_olt_counters(olt_id);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_ResetCounters(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_SetBerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_bytes)
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
	iRlt = PAS_set_alarm_configuration(olt_id, -1, PON_ALARM_BER, activate, &alarm_cfg);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetBerAlarmConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, alarm_switch, alarm_thresold, alarm_min_error_bytes, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int PAS5001_SetFerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_frames)
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
	iRlt = PAS_set_alarm_configuration(olt_id, -1, PON_ALARM_FER, activate, &alarm_cfg);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetFerAlarmConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, alarm_switch, alarm_thresold, alarm_min_error_frames, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int PAS5001_SetAlarmConfig(short int olt_id, short int source, PON_alarm_t type, bool activate, void	*configuration, int length)
{
    int iRlt = ROK;

	OLT_LOCAL_ASSERT(olt_id);
	iRlt = PAS_set_alarm_configuration(olt_id, source, type, activate, configuration);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetAlarmConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, source, type, activate, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int PAS5001_GetAlarmConfig(short int olt_id, short int source, PON_alarm_t type, bool *activate, void	*configuration)
{
    int iRlt = ROK;

	OLT_LOCAL_ASSERT(olt_id);
	iRlt = PAS_get_alarm_configuration(olt_id, source, type, activate, configuration);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetAlarmConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, source, type, *activate, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}
static int PAS5001_GetStatistics(short int olt_id, short int collector_id, PON_statistics_t statistics_type, short int statistics_parameter, long double *statistics_data)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
 
    
    iRlt = PAS_get_statistics(olt_id, collector_id, statistics_type, statistics_parameter, statistics_data);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetStatistics(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, collector_id, statistics_type, statistics_parameter, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_OltSelfTest(short int olt_id)
{
    int iRlt = 0;

	OLT_LOCAL_ASSERT(olt_id);

	iRlt = PAS_olt_self_test(olt_id);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_OltSelfTest(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_LinkTest(short int olt_id, short int llid, short int number_of_frames, short int frame_size, bool link_delay_measurement, PON_link_test_vlan_configuration_t *vlan_configuration, PON_link_test_results_t *test_results)
{
    int iRlt = RERROR;
    OLT_LOCALID_ASSERT(olt_id);

	iRlt = PAS_link_test(olt_id, llid, number_of_frames, frame_size, link_delay_measurement, vlan_configuration, test_results);

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_LinkTest(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	 return iRlt;
}

static int PAS5201_SetLLIDFecMode(short int olt_id, short int llid, bool downlink_fec)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = PAS_set_llid_fec_mode(olt_id, llid, downlink_fec);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetLLIDFecMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, downlink_fec, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_GetLLIDFecMode(short int olt_id, short int llid, bool *downlink_fec, bool *uplink_fec, bool *uplink_lastframe_fec)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(downlink_fec);
    VOS_ASSERT(uplink_fec);
    VOS_ASSERT(uplink_lastframe_fec);

    iRlt = PAS_get_llid_fec_mode(olt_id, llid, downlink_fec, uplink_fec, uplink_lastframe_fec);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetLLIDFecMode(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, *downlink_fec, *uplink_fec, *uplink_lastframe_fec, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5001_SysDump(short int olt_id, short int llid, int dump_type)
{
    int iRlt = RERROR;
    OLT_LOCALID_ASSERT(olt_id);

#ifdef SYS_DUMP
	iRlt = PAS_sys_dump(dump_type, olt_id, llid);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif

	OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SysDump(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	 return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------OLT 倒换API---------------- */

static int PAS5001_GetHotSwapCapability(short int olt_id, int *swap_cap)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(swap_cap);

    *swap_cap = V2R1_PON_PORT_SWAP_SLOWLY;
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetHotSwapCapability(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *swap_cap, 0, SYS_LOCAL_MODULE_SLOTNO);
    
    return 0;
}

static int PAS5201_GetHotSwapCapability(short int olt_id, int *swap_cap)
{
    int iRlt = 0;
    PAS_device_versions_t device_versions;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(swap_cap);
    
    *swap_cap = V2R1_PON_PORT_SWAP_QUICKLY;
	if ( 0 == PAS_get_olt_versions(olt_id, &device_versions) )
    {
        /* PON口的Firmware版本太低，则不支持快速倒换 */
        if (device_versions.firmware_major >= V2R1_PON_5201_SWAP_QUICKLY_SUPPORT_FIRMWAREVER_MAJOR)
        {
            if (device_versions.firmware_major == V2R1_PON_5201_SWAP_QUICKLY_SUPPORT_FIRMWAREVER_MAJOR)
            {
                if (device_versions.firmware_minor >= V2R1_PON_5201_SWAP_QUICKLY_SUPPORT_FIRMWAREVER_MINOR)
                {
                    if ( device_versions.firmware_minor == V2R1_PON_5201_SWAP_QUICKLY_SUPPORT_FIRMWAREVER_MINOR)
                    {
                        if (device_versions.build_firmware >= V2R1_PON_5201_SWAP_QUICKLY_SUPPORT_FIRMWAREVER_BUILD)
                        {
                            if (device_versions.build_firmware >= V2R1_PON_5201_SWAP_ONU_SUPPORT_FIRMWAREVER_BUILD)
                            {
                                *swap_cap = V2R1_PON_PORT_SWAP_ONU;
                            }
                            else
                            {
                                *swap_cap = V2R1_PON_PORT_SWAP_QUICKLY;
                            }
                        }
                        else
                        {
                            *swap_cap = V2R1_PON_PORT_SWAP_SLOWLY;
                        }
                    }
                    else
                    {
                        *swap_cap = V2R1_PON_PORT_SWAP_ONU;
                    }
                }
                else
                {
                    *swap_cap = V2R1_PON_PORT_SWAP_SLOWLY;
                }
            }
            else
            {
                *swap_cap = V2R1_PON_PORT_SWAP_ONU;
            }
        }
        else
        {
            *swap_cap = V2R1_PON_PORT_SWAP_SLOWLY;
        }
    }     
    else
    {
        iRlt = OLT_ERR_NOTEXIST;
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetHotSwapCapability(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *swap_cap, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int PAS5001_GetHotSwapMode(short int olt_id, short int *partner_olt_id, int *swap_mode, int *swap_status)
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
        if ( OLT_ERR_OK == (iRlt = PAS5001_GetOpticalTxMode(olt_id, &tx_mode)) )
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

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_GetHotSwapMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, *partner_olt_id, *swap_mode, *swap_status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_GetHotSwapMode(short int olt_id, short int *partner_olt_id, int *swap_mode, int *swap_status)
{
    int iRlt;
    PON_redundancy_olt_state_t fw_redundancy_state, tmp_state;
    
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(partner_olt_id);
    VOS_ASSERT(swap_mode);
    VOS_ASSERT(swap_status);

    if ( 0 == (iRlt = get_redundancy_state(olt_id, &fw_redundancy_state)) )
    {
        int local_mode, local_status;
        short int local_partner;
        
        local_mode    = V2R1_PON_PORT_SWAP_AUTO;
        local_status  = V2R1_PON_PORT_SWAP_UNKNOWN;
        local_partner = OLT_ID_NULL;
    
        if (PON_OLT_REDUNDANCY_STATE_NONE != fw_redundancy_state)
        {
            switch (fw_redundancy_state)
            {
                case PON_OLT_REDUNDANCY_STATE_MASTER:
                    local_mode = V2R1_PON_PORT_SWAP_QUICKLY;
                    local_status = V2R1_PON_PORT_SWAP_ACTIVE;
                    get_pair_slave_olt_id(olt_id, &local_partner, &tmp_state);
                    break;
                case PON_OLT_REDUNDANCY_STATE_SLAVE:
                    local_mode = V2R1_PON_PORT_SWAP_QUICKLY;
                    local_status = V2R1_PON_PORT_SWAP_PASSIVE;
                    get_pair_master_olt_id(olt_id, &local_partner, &tmp_state);
                    break;
				/*for onu swap by jinhl@2013-02-22*/
				#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
				case PON_OLT_REDUNDANCY_STATE_ASSIGN_ODD_LLID:
				case PON_OLT_REDUNDANCY_STATE_ASSIGN_EVEN_LLID:
					local_mode = V2R1_PON_PORT_SWAP_ONU;
                    local_status = V2R1_PON_PORT_SWAP_ACTIVE;
                    get_onu_optical_mapping_list_pair_by_olt_id(olt_id, &local_partner);
                    break;
				#endif
                default:
            }
        }
        
        if ( OLT_ERR_OK == (iRlt = GW_GetHotSwapMode(olt_id, partner_olt_id, swap_mode, swap_status)) )
        {
            *swap_mode = local_mode;

            if ( OLT_ID_NULL != local_partner )
            {
                *partner_olt_id = local_partner;
            }

            if (V2R1_PON_PORT_SWAP_UNKNOWN != local_status)
            {
                *swap_status = local_status;
            }
            else
            {
                int tx_mode;
        
                if ( 0 == (iRlt = PAS5201_GetOpticalTxMode(olt_id, &tx_mode)) )
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
        }
    }
    else
    {
        iRlt = GW_GetHotSwapMode(olt_id, partner_olt_id, swap_mode, swap_status);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetHotSwapMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, *partner_olt_id, *swap_mode, *swap_status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5001_SetHotSwapMode(short int olt_id, short int partner_olt_id, int swap_mode, int swap_status, int swap_flags)
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
                /* 5001的取消倒换 */
				if(PONPORT_DISABLE == GetPonPortAdminStatus(olt_id))
				{
					iRlt = PAS_set_olt_pon_transmission( olt_id, FALSE );
				}
				else
				{
					iRlt = PAS_set_olt_pon_transmission( olt_id, TRUE );
				}
                if ( 0 == iRlt )
                {
                    PonPortTable[olt_id].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
                    if ( OLT_ISLOCAL(partner_olt_id)
                        /* B--added by liwei056@2011-11-22 for D13979 */
                        && Olt_exists(partner_olt_id)
                        /* E--added by liwei056@2011-11-22 for D13979 */
                        )
                    {
						if(PONPORT_DISABLE == GetPonPortAdminStatus(partner_olt_id))
						{
							if ( 0 == (iRlt = PAS_set_olt_pon_transmission( partner_olt_id, FALSE )) )
							{
								PonPortTable[partner_olt_id].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
							}         
						}	
						else
						{
							if ( 0 == (iRlt = PAS_set_olt_pon_transmission( partner_olt_id, TRUE )) )
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
        			iRlt = PAS_set_olt_pon_transmission( olt_id, FALSE );
                }
                else
                {
            		(void)PAS_reset_address_table(olt_id, PON_ALL_ACTIVE_LLIDS,  ADDR_DYNAMIC);
        			if ( 0 == (iRlt = PAS_set_olt_pon_transmission( olt_id, TRUE )) )
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

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_SetHotSwapMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, swap_mode, swap_status, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int PAS5201_SetHotSwapMode(short int olt_id, short int partner_olt_id, int swap_mode, int swap_status, int swap_flags)
{
    int iRlt;
    int iPartRlt;
    int new_swap_mode, old_swap_mode;
    int master_id, slave_id; 

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
        if ( (0 != PAS5201_GetHotSwapCapability(olt_id, &new_swap_mode))
            || (swap_mode <= new_swap_mode) )
        {
            /* B--remmed by liwei056@2011-4-19 for RdnModeChangeBUG */
#if 0        
            if ( old_swap_mode == V2R1_PON_PORT_SWAP_SLOWLY )
            {
                /* 5201的取消慢倒换 , 备用瞬时发光会造成光路干扰 , 影响倒换 */
                (void)PAS_set_olt_pon_transmission( olt_id, TRUE );
                if ( OLT_ISLOCAL(partner_olt_id) )
                {
                    (void)PAS_set_olt_pon_transmission( partner_olt_id, TRUE );
                }
            }
#endif
            /* E--remmed by liwei056@2011-4-19 for RdnModeChangeBUG */
            /*for onu swap by jinhl@2013-02-22*/
            if(V2R1_PON_PORT_SWAP_ONU != swap_mode)
        	{
	            if( swap_status == V2R1_PON_PORT_SWAP_PASSIVE )
	            {
	                master_id = partner_olt_id;
	                slave_id  = olt_id;
	            }
	            else
	            {
	                master_id = olt_id;
	                slave_id  = partner_olt_id;
	            }
        	}
			else
			{
				master_id = olt_id;
	            slave_id  = partner_olt_id;
			}

            switch( swap_mode )
            {
                case V2R1_PON_PORT_SWAP_QUICKLY:
                    /* 5201的快倒换设置 */
                    iRlt = ActivePonFastHotSwapSetup(master_id, slave_id);

                    break;
                case V2R1_PON_PORT_SWAP_ONU:
                    /* 5201的ONU倒换设置 */
                    iRlt = ActivePonOnuHotSwapSetup(master_id, slave_id);

                    break;
                default:
                    VOS_ASSERT(0);
                    iRlt = OLT_ERR_UNKNOEWN;
            }
            
            if ( 0 == iRlt )
            {
                if ( OLT_ISLOCAL(master_id) )
                {
                    PonPortTable[master_id].swap_use = V2R1_PON_PORT_SWAP_ACTIVE;
                }
                if ( OLT_ISREMOTE(partner_olt_id) )
                {
                    iRlt = iPartRlt;
                }
            }
        }
        else
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    else
    {
        switch( old_swap_mode )
        {
            case V2R1_PON_PORT_SWAP_QUICKLY:
                /* 5201的取消快倒换 */
                (void)InActivePonFastHotSwapSetup(olt_id, partner_olt_id);

                break;
            case V2R1_PON_PORT_SWAP_ONU:
                /* 5201的取消ONU倒换 */
                (void)InActivePonOnuHotSwapSetup(olt_id, partner_olt_id);

                break;
			default:
				break;
        }

        switch (new_swap_mode)
        {
            case V2R1_PON_PORT_SWAP_AUTO:
                /* 仅内存设置倒换 */
                iRlt = 0;
                break;
            case V2R1_PON_PORT_SWAP_DISABLED:
                /* 5201的取消倒换 */
				if(PONPORT_DISABLE == GetPonPortAdminStatus(olt_id))
				{
					iRlt = PAS_set_olt_pon_transmission( olt_id, FALSE );
				}
				else
				{
					iRlt = PAS_set_olt_pon_transmission( olt_id, TRUE );
				}
             
                if ( 0 == iRlt )
                {
                    PonPortTable[olt_id].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
                    if ( OLT_ISLOCAL(partner_olt_id)
                        /* B--added by liwei056@2011-11-30 for D13979 */
                        && Olt_exists(partner_olt_id)
                        /* E--added by liwei056@2011-11-30 for D13979 */
                        )
					{

						if(PONPORT_DISABLE == GetPonPortAdminStatus(olt_id))
						{
							if ( 0 == (iRlt = PAS_set_olt_pon_transmission( partner_olt_id, FALSE )) )
							{
								PonPortTable[partner_olt_id].swap_use = V2R1_PON_PORT_SWAP_UNKNOWN;
							}
						}
						else
						{
							if ( 0 == (iRlt = PAS_set_olt_pon_transmission( partner_olt_id, TRUE )) )
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
                /* 5201的慢倒换 */
            	if( swap_status == V2R1_PON_PORT_SWAP_PASSIVE )
                {
                    iRlt = PAS_set_olt_pon_transmission( olt_id, FALSE );
                }
                else
                {
                    (void)PAS_reset_address_table(olt_id, PON_ALL_ACTIVE_LLIDS,  ADDR_DYNAMIC_AND_STATIC);
                    if ( 0 == (iRlt = PAS_set_olt_pon_transmission( olt_id, TRUE )) )
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

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetHotSwapMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, swap_mode, swap_status, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int PAS5001_ForceHotSwap(short int olt_id, short int partner_olt_id, int swap_status, int swap_flags)
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
        if (V2R1_PON_PORT_SWAP_SLOWLY == local_mode)
        {
            /* 用户强制倒换*/
            PonSwitchInfo_S switch_event;

            switch_event.olt_id       = olt_id;
            switch_event.new_status   = V2R1_PON_PORT_SWAP_PASSIVE;
            switch_event.event_id     = PROTECT_SWITCH_EVENT_START;
            switch_event.event_code   = PROTECT_SWITCH_REASON_HOSTREQUEST;
            
            switch_event.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
            switch_event.slot_source  = SYS_LOCAL_MODULE_SLOTNO;
            switch_event.event_seq    = PonPortTable[olt_id].swap_times + 1;
            switch_event.event_flags  = PROTECT_SWITCH_EVENT_FLAGS_NONE;
            PonSwitchHandler(&switch_event);

            iRlt = OLT_SetHotSwapMode(olt_id, partner_olt_id, V2R1_PON_PORT_SWAP_SLOWLY, PROTECT_SWITCH_STATUS_PASSIVE, OLT_SWAP_FLAGS_ONLYSETTING);

            switch_event.olt_id       = partner_olt_id;
            switch_event.new_status   = V2R1_PON_PORT_SWAP_ACTIVE;
            switch_event.event_id     = PROTECT_SWITCH_EVENT_OVER;
            switch_event.event_code   = (0 == iRlt) ? PROTECT_SWITCH_RESULT_SUCCEED : PROTECT_SWITCH_RESULT_FAILED;;
            
            switch_event.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
            switch_event.slot_source  = SYS_LOCAL_MODULE_SLOTNO;
            switch_event.event_seq    = PonPortTable[olt_id].swap_times + 1;
            switch_event.event_flags  = PROTECT_SWITCH_EVENT_FLAGS_NONE;
            PonSwitchHandler(&switch_event);
        }
        else
        {
            iRlt = OLT_ERR_PARAM;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_ForceHotSwap(%d, %d, %d, %d)'s result(%d) at mode(%d) on slot %d.\r\n", olt_id, partner_olt_id, swap_status, swap_flags, iRlt, local_mode, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_ForceHotSwap(short int olt_id, short int partner_olt_id, int swap_status, int swap_flags)
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
        if (V2R1_PON_PORT_SWAP_SLOWLY < local_mode)
        {
            iRlt = HotSwapRedundancyPon(olt_id, partner_olt_id);
        }
        else if (V2R1_PON_PORT_SWAP_SLOWLY == local_mode)
        {
            /* 用户强制倒换*/
            PonSwitchInfo_S switch_event;

            switch_event.olt_id       = olt_id;
            switch_event.new_status   = V2R1_PON_PORT_SWAP_PASSIVE;
            switch_event.event_id     = PROTECT_SWITCH_EVENT_START;
            switch_event.event_code   = PROTECT_SWITCH_REASON_HOSTREQUEST;
            
            switch_event.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
            switch_event.slot_source  = SYS_LOCAL_MODULE_SLOTNO;
            switch_event.event_seq    = PonPortTable[olt_id].swap_times + 1;
            switch_event.event_flags  = PROTECT_SWITCH_EVENT_FLAGS_NONE;
            PonSwitchHandler(&switch_event);

            iRlt = OLT_SetHotSwapMode(olt_id, partner_olt_id, V2R1_PON_PORT_SWAP_SLOWLY, PROTECT_SWITCH_STATUS_PASSIVE, OLT_SWAP_FLAGS_ONLYSETTING);

            switch_event.olt_id       = partner_olt_id;
            switch_event.new_status   = V2R1_PON_PORT_SWAP_ACTIVE;
            switch_event.event_id     = PROTECT_SWITCH_EVENT_OVER;
            switch_event.event_code   = (0 == iRlt) ? PROTECT_SWITCH_RESULT_SUCCEED : PROTECT_SWITCH_RESULT_FAILED;
            
            switch_event.event_source = PROTECT_SWITCH_EVENT_SRC_HARDWARE;
            switch_event.slot_source  = SYS_LOCAL_MODULE_SLOTNO;
            switch_event.event_seq    = PonPortTable[olt_id].swap_times + 1;
            switch_event.event_flags  = PROTECT_SWITCH_EVENT_FLAGS_NONE;
            PonSwitchHandler(&switch_event);
        }
        else
        {
            iRlt = OLT_ERR_PARAM;
        }
    }

#if 0
    if ( 0 == iRlt )
    {
        if ( OLT_ISREMOTE(partner_olt_id) )
        {
            iRlt = iPartRlt;
        }
    }
#endif

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_ForceHotSwap(%d, %d, %d, %d)'s result(%d) at mode(%d) on slot %d.\r\n", olt_id, partner_olt_id, swap_status, swap_flags, iRlt, local_mode, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_RdnOnuRegister(short int olt_id, PON_redundancy_onu_register_t *onu_reg_info)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(onu_reg_info);

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
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_RdnOnuRegister(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_reg_info->onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_SetRdnConfig(short int olt_id, int rdn_status, int gpio_num, int rdn_type, int rx_enable)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = PAS_set_redundancy_config(olt_id, rdn_status, (unsigned short)gpio_num, rdn_type, rx_enable);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetRdnConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, rdn_status, rdn_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_RdnSwitchOver(short int olt_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = PAS_redundancy_switch_over(olt_id);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_RdnSwitchOver(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_RdnIsExist(short int olt_id, bool *status)
{
    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(status);

    *status = redundancy_olt_exists(olt_id);
    /* OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_RdnIsExist(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *status, 0, SYS_LOCAL_MODULE_SLOTNO); */
    
    return 0;    
}

static int PAS5201_ResetRdnRecord(short int olt_id, int rdn_state)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#ifdef PAS_SOFT_VERSION_V5_3_12
    iRlt = reset_redundancy_olt_record(olt_id, rdn_state);
#else
    iRlt = reset_redundancy_record(olt_id, rdn_state);
#endif
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_ResetRdnRecord(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, rdn_state, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_GetRdnState(short int olt_id, int *state)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(state);

    iRlt = get_redundancy_state(olt_id, state);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetRdnState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *state, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_SetRdnState(short int olt_id, int state)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = set_redundancy_state(olt_id, state);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetRdnState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, state, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_GetRdnAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    VOS_ASSERT(addr_num);
    VOS_ASSERT(addr_tbl);

    iRlt = PAS_get_redundancy_address_table(olt_id, addr_num, addr_tbl);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetRdnAddrTbl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, *addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_RemoveRdnOlt(short int olt_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

#ifdef PAS_SOFT_VERSION_V5_3_12
    iRlt = remove_olt_from_mapping_list(olt_id);
#else
    iRlt = reset_redundancy_record(olt_id, PON_OLT_REDUNDANCY_STATE_NONE);
#endif

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_RemoveRdnOlt(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_GetLLIDRdnDB(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(rdn_db);

    iRlt = CTC_STACK_get_redundancy_database(olt_id, llid, rdn_db);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetLLIDRdnDB(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_SetLLIDRdnDB(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(rdn_db);

    iRlt = CTC_STACK_set_redundancy_database(olt_id, llid, *rdn_db);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetLLIDRdnDB(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5001_RdnRemoveOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    if ( OLT_ISREMOTE(partner_olt_id) )
    {
        iRlt = remote_olt_removed_handler(olt_id, partner_olt_id);
    }
    else
    {
        OLT_LOCAL_ASSERT(partner_olt_id);
        
        iRlt = Remove_olt(partner_olt_id, FALSE, FALSE);
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5001_RdnRemoveOlt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_RdnSwapOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;
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

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_RdnSwapOlt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_AddSwapOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;
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

    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_AddSwapOlt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int PAS5201_RdnLooseOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    if ( OLT_ISREMOTE(partner_olt_id) )
    {
        iRlt = remote_olt_loosed_handler(olt_id, partner_olt_id);
    }
    else
    {
        iRlt = OLT_ERR_PARAM;
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_RdnLooseOlt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, partner_olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*Begin:for onu swap by jinhl@2013-02-22*/
static int PAS5201_RdnLLIDAdd(short int olt_id, short int llid)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    
    iRlt = redundancy_onu_optical_add_onu(olt_id, llid);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_RdnLLIDAdd(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_GetRdnLLIDMode(short int olt_id, short int llid, PON_redundancy_llid_redundancy_mode_t* mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    
    iRlt = Get_redundancy_onu_mode(olt_id, llid, mode);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetRdnLLIDMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}

static int PAS5201_SetRdnLLIDMode(short int olt_id, short int llid, PON_redundancy_llid_redundancy_mode_t mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    
    iRlt = Set_redundancy_onu_mode(olt_id, llid, mode);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetRdnLLIDMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}  

static int PAS5201_SetRdnLLIDStdbyToAct(short int olt_id, short int llid_n, short int* llid_list_marker)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    /*VOS_ASSERT(llid_list_marker);*/
    
    iRlt = PAS_redundancy_llid_redund_standby_to_active(olt_id, llid_n, llid_list_marker);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetRdnLLIDStdbyToAct(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}  

static int PAS5201_SetRdnLLIDRtt(short int olt_id, short int llid, PON_rtt_t rtt)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    iRlt = Set_onu_record_rtt_value(olt_id, llid, rtt);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetRdnLLIDRtt(%d)'s result(%d) on slot %d.\r\n", olt_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;    
}  

/*End:for onu swap by jinhl@2013-02-22*/

static int PAS5201_GetLLIDRdnRegisterInfo(short int olt_id, short int llid, PON_redundancy_onu_register_t *onu_reginfo)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_reginfo);
    
    iRlt = get_onu_register_info_from_fw(olt_id, llid, onu_reginfo);
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_GetLLIDRdnRegisterInfo(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif

/*added by liyang @2015-05-18 */
static int PAS5201_SetCTCOnuTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
    short int llid = -1;
 
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	iRlt = CTC_STACK_onu_tx_power_supply_control(olt_id, llid, *parameter, TRUE);
	 	
    OLT_PAS_DEBUG(OLT_PAS_TITLE"PAS5201_SetCTCOnuTxPowerSupplyControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}



/*-----------------------------外部接口----------------------------------------*/

static const OltMgmtIFs s_pasGlobalIfs = {
#if 1
/* -------------------OLT基本API------------------- */
    PAS_ERROR, /* IsExist */
    PAS_ERROR, /* GetChipTypeID */
    PAS_ERROR, /* GetChipTypeName */
    PAS_ERROR, /* ResetPon */
    PAS_ERROR, /* RemoveOlt */
    
    PAS_ERROR, /* CopyOlt */
    PAS_ERROR, /* CmdIsSupported */
    PAS_OK,    /* SetDebugMode */
    PAS_SetInitParams,
    PAS_SetSystemParams,

    PAS_ERROR, /* SetPonI2CExtInfo */
    PAS_ERROR, /* GetPonI2CExtInfo */
    PAS_ERROR, /* SetCardI2CInfo */
    PAS_ERROR, /* GetCardI2CInfo */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS_ERROR, /*WriteMdioRegister*/

    PAS_ERROR, /*ReadMdioRegister*/
    PAS_ERROR, /*ReadI2CRegister*/
    PAS_ERROR,  /*GpioAccess*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS_ERROR, /* ReadGpio */
    PAS_ERROR, /* WriteGpio */

    PAS_ERROR, /* SendChipCli */
    PAS_OK,    /* SetDeviceName*/
    PAS_OK,    /*ResetPonChip*/
#endif

#if 1
/* -------------------OLT PON管理API--------------- */
    PAS_ERROR, /* GetVersion */
    PAS_ERROR, /* GetDBAVersion */
    PAS_ERROR, /* ChkVersion */
    PAS_ERROR, /* ChkDBAVersion */
    PAS_ERROR, /* GetCniLinkStatus */

    PAS_ERROR, /* GetPonWorkStatus */
    PAS_ERROR, /* SetAdminStatus */
    PAS_ERROR, /* GetAdminStatus */
    PAS_ERROR, /* SetVlanTpid */
    PAS_ERROR, /* SetVlanQinQ */

    PAS_ERROR, /* SetPonFrameSizeLimit */
    PAS_ERROR, /* GetPonFrameSizeLimit */
    PAS_ERROR, /* OamIsLimit */
    PAS_ERROR, /* UpdatePonParams */
    GW_SetPPPoERelayMode,

    GW_SetPPPoERelayParams,
    GW_SetDhcpRelayMode,
    NULL,      /* SetIgmpAuthMode */
    PAS_ERROR, /* SendFrame2PON */
    PAS_ERROR, /* SendFrame2CNI */

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS_ERROR, /*GetVidDownlinkMode*/
    PAS_ERROR, /*DelVidDownlinkMode*/
    PAS_ERROR, /*GetOltParameters*/
    PAS_ERROR, /*SetOltIgmpSnoopingMode*/
    PAS_ERROR, /*GetOltIgmpSnoopingMode*/

    PAS_ERROR, /*SetOltMldForwardingMode*/
    PAS_ERROR, /*GetOltMldForwardingMode*/
    PAS_ERROR, /*SetDBAReportFormat*/
    PAS_ERROR, /*GetDBAReportFormat*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-04-27*/
    PAS_ERROR, /*UpdateProvBWInfo*/
    /*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    PAS_ERROR, /* LLIDIsExist */
    PAS_ERROR, /* DeregisterLLID */
    PAS_ERROR, /* GetLLIDMac */
    PAS_ERROR, /* GetLLIDRegisterInfo */
    PAS_ERROR, /* AuthorizeLLID */

    PAS_ERROR, /* SetLLIDSLA */
    PAS_ERROR, /* GetLLIDSLA */
    PAS_ERROR, /* SetLLIDPolice */
    PAS_ERROR, /* GetLLIDPolice */
    PAS_ERROR, /* SetLLIDdbaType */
    
    PAS_ERROR, /* GetLLIDdbaType */
    PAS_ERROR, /* SetLLIDdbaFlags */
    PAS_ERROR, /* GetLLIDdbaFlags */
    PAS_ERROR, /* GetLLIDHeartbeatOam */
    PAS_ERROR, /* SetLLIDHeartbeatOam */
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    PAS_ERROR, /* GetOnuNum */
    PAS_ERROR, /* GetAllOnus */
    PAS_ERROR, /* ClearAllOnus */
    PAS_ERROR, /* ResumeAllOnuStatus */
    PAS_OK,    /* SetAllOnuAuthMode */

    PAS_ERROR, /* SetOnuAuthMode */
    PAS_ERROR, /* SetMacAuth */
    PAS_OK,    /* SetAllOnuBindMode */
    PAS_ERROR, /* CheckOnuRegisterControl */
    PAS_OK,    /* SetAllOnuDefaultBW */

    PAS_OK,    /* SetAllOnuDownlinkPoliceMode */
    PAS_ERROR, /* SetOnuDownlinkPoliceMode */
    PAS_OK,    /* SetAllOnuDownlinkPoliceParam */
    PAS_OK,    /* SetAllOnuUplinkDBAParam */
    PAS_ERROR, /* SetOnuDownlinkPri2CoSQueueMap */

    PAS_ERROR, /* ActivePendingOnu */
    PAS_ERROR, /* ActiveOnePendingOnu */
    NULL,      /* ActiveConfPendingOnu */
    NULL,      /* ActiveOneConfPendingOnu */
    PAS_ERROR, /* GetPendingOnu */

    PAS_ERROR, /* GetUpdatingOnu */
    PAS_ERROR, /* GetUpdatedOnu */
    PAS_ERROR, /* GetOnuUpdatingStatusLocal */
    PAS_ERROR, /* SetOnuUpdateMsg */
    PAS_ERROR, /* GetOnuUpdateWaiting */

    PAS_ERROR, /* SetAllOnuAuthMode2 */
    PAS_OK,    /* SetAllOnuBWParams */
    PAS_ERROR, /* SetOnuP2PMode */
    PAS_ERROR, /* GetOnuB2PMode */
    PAS_ERROR, /* SetOnuB2PMode */

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS_ERROR, /*GetOnuMode */
    PAS_ERROR, /*GetMACAddressAuthentication */
    PAS_ERROR, /*SetAuthorizeMacAddressAccordingListMode */
    PAS_ERROR, /*GetAuthorizeMacAddressAccordingListMode */
    PAS_ERROR, /*GetDownlinkBufferConfiguration */
    
    PAS_ERROR, /*GetOamInformation */
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
/*Begin:for onu swap by jinhl@2013-02-22*/
    PAS_ERROR, /*ResumeLLIDStatus*/
    PAS_ERROR, /*SearchFreeOnuIdx*/
    PAS_ERROR, /*GetActOnuIdxByMac,*/
/*End:for onu swap by jinhl@2013-02-22*/
    PAS_ERROR,    /*BroadCastCliCommand*/
    
    PAS_OK,       /*SetAuthEntry*/   
    PAS_ERROR,    /*SetGponAuthEntry*/        
    PAS_OK,       /*SetOnuDefaultMaxMac*/  
    PAS_OK,  /*GW_SetCTCOnuPortStatsTimeOut*/
    PAS_OK,    /* SetMaxOnu*/
    PAS_OK,    /* GetOnuConfDelStatus*/   
    PAS_OK,    /* SetCTCOnuTxPowerSupplyControl*/
#endif

#if 1
/* -------------------OLT 加密管理API----------- */
    PAS_ERROR, /* SetEncryptMode */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS_ERROR, /*SetEncryptionPreambleMode*/
    PAS_ERROR, /*GetEncryptionPreambleMode*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS_ERROR, /* GetLLIDEncryptMode */
    PAS_ERROR, /* StartLLIDEncrypt */
    
    PAS_ERROR, /* FinishLLIDEncrypt */
    PAS_ERROR, /* StopLLIDEncrypt */
    PAS_ERROR, /* SetLLIDEncryptKey */
    PAS_ERROR, /* FinishLLIDEncryptKey */
#endif

#if 1
/* -------------------OLT 地址表管理API-------- */
    PAS_ERROR, /* SetMacAgeingTime */
    PAS_ERROR, /* SetAddrTblCfg */                      
    PAS_ERROR, /* GetAddrTblCfg */                      
    PAS_ERROR, /* GetMacAddrTbl */
    NULL,      /* AddMacAddrTbl */

    NULL,      /* DelMacAddrTbl */
    PAS_ERROR, /* RemoveMac */
    PAS_ERROR, /* ResetAddrTbl */
    PAS_OK,    /*SetOnuMacThreshold*/
    GW_OnuMacCheckEnable,
    
    GW_OnuMacCheckPeriod,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS_ERROR, /*SetAddressTableFullHandlingMode*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS_ERROR,
    PAS_ERROR,
#endif

#if 1
/* -------------------OLT 光路管理API----------- */
    PAS_ERROR, /* GetOpticalCapability */
    PAS_ERROR, /* GetOpticsDetail */
    PAS_ERROR, /* SetPonRange */
    PAS_ERROR, /* SetOpticalTxMode */
    PAS_ERROR, /* GetOpticalTxMode */

    PAS_ERROR, /*SetVirtualScopeAdcConfig*/
    PAS_ERROR, /*GetVirtualScopeMeasurement*/
    PAS_ERROR, /*GetVirtualScopeOnuVoltage*/
    PAS_ERROR, /*SetVirtualLLID*/
    
    PAS_ERROR, /* SetOpticalTxMode2 */
#endif

#if 1
/* -------------------OLT 监控统计管理API---- */
    PAS_ERROR, /* GetRawStatistics */
    PAS_ERROR, /* ResetCounters */
    PAS_ERROR, /* SetBerAlarm */
    PAS_ERROR, /* SetFerAlarm */
    PAS_ERROR, /* SetPonBerAlarm */                      

    PAS_ERROR, /* SetPonFerAlarm */                      
    PAS_OK,    /* SetBerAlarmParams */
    PAS_OK,    /* SetFerAlarmParams */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS_ERROR, /*SetAlarmConfig*/
    PAS_ERROR, /*GetAlarmConfig*/
    
    PAS_ERROR, /*GetStatistics*/
    PAS_ERROR, /*OltSelfTest*/
    PAS_ERROR, /*LinkTest*/
    PAS_ERROR, /* SetLLIDFecMode */
    PAS_ERROR, /* GetLLIDFecMode */
    
    PAS_ERROR, /*SysDump*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------OLT 倒换API---------------- */
    PAS_ERROR, /* GetHotSwapCapability */
    PAS_ERROR, /* GetHotSwapMode */
    PAS_ERROR, /* SetHotSwapMode */
    PAS_ERROR, /* ForceHotSwap */
    GW_SetHotSwapParam,

    PAS_ERROR, /* RdnOnuRegister */
    PAS_ERROR, /* SetRdnConfig */
    PAS_ERROR, /* RdnSwitchOver */
    PAS_ERROR, /* RdnIsExist */
    PAS_ERROR, /* ResetRdnOltRecord */

    PAS_ERROR, /* GetRdnState */
    PAS_ERROR, /* SetRdnState */
    PAS_ERROR, /* GetLLIDRdnDB */
    PAS_ERROR, /* SetLLIDRdnDB */
    PAS_ERROR, /* GetRdnAddrTbl */

    PAS_ERROR, /* RemoveRdnOlt */
    PAS_ERROR, /* RdnRemoveOlt */
    PAS_ERROR, /* RdnSwapOlt */
    PAS_ERROR, /* AddSwapOlt */
    PAS_ERROR, /* RdnLooseOlt */

    /*Begin:for onu swap by jinhl@2013-02-22*/
    PAS_ERROR, /*RdnLLIDAdd*/
    PAS_ERROR, /*GetRdnLLIDMode*/
    PAS_ERROR, /*SetRdnLLIDMode*/
    PAS_ERROR, /*SetRdnLLIDStdbyToAct*/
    PAS_ERROR, /*SetRdnLLIDRtt*/
    /*End:for onu swap by jinhl@2013-02-22*/
    
    PAS_ERROR, /* RdnIsReady */
    PAS_ERROR, /* GetLLIDRdnRegisterInfo */
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */
    PAS_ERROR, /* DumpAllCmc */
    PAS_OK,    /* SetCmcServiceVID */
#endif

    PAS_ERROR /* LastFun */
};



static const OltMgmtIFs s_pas5001Ifs = {
#if 1
/* -------------------OLT基本API------------------- */
    PAS_IsExist,
    PAS5001_GetChipTypeID,
    PAS5001_GetChipTypeName,
    PAS5001_ResetPon,
    PAS5001_RemoveOlt,
    
    GW_CopyOlt,
    GW_CmdIsSupported,
    PAS_OK,   /* SetDebugMode */
    PAS_SetInitParams,
    PAS_SetSystemParams,

    GW_SetPonI2CExtInfo,
    GW_GetPonI2CExtInfo,
    GW_SetCardI2CInfo,
    GW_GetCardI2CInfo,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_WriteMdioRegister,

    PAS5001_ReadMdioRegister,
    PAS5001_ReadI2CRegister,
    PAS5001_GpioAccess,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_ReadGpio,
    PAS5001_WriteGpio,

    PAS5001_SendChipCli,
    PAS_OK,   /* SetDeviceName*/
    PAS_OK,	  /*ResetPonChip*/
#endif

#if 1
/* -------------------OLT PON管理API--------------- */
    PAS5001_GetVersion,
    PAS5001_GetDBAVersion,
    PAS_OK,   /* ChkVersion */
    GW_ChkDBAVersion,   /* ChkDBAVersion */
    PAS5001_GetCniLinkStatus,

    GW_GetPonWorkStatus, 
    PAS5001_SetAdminStatus,
    GW_GetAdminStatus,   
    NULL,     /* SetVlanTpid */
    NULL,     /* SetVlanQinQ */

    NULL,     /* SetPonFrameSizeLimit */
    NULL,     /* GetPonFrameSizeLimit */
    PAS5001_OamIsLimit,
    PAS5001_UpdatePonParams,
    GW_SetPPPoERelayMode,

    GW_SetPPPoERelayParams,
    GW_SetDhcpRelayMode,
    PAS_OK,   /* SetIgmpAuthMode */
    PAS5001_SendFrame2PON,
    PAS5001_SendFrame2CNI,

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,     /*GetVidDownlinkMode*/
    NULL,     /*DelVidDownlinkMode*/
    PAS5001_GetOltParameters,
    PAS5001_SetOltIgmpSnoopingMode,
    PAS5001_GetOltIgmpSnoopingMode,

    PAS5001_SetOltMldForwardingMode,
    PAS5001_GetOltMldForwardingMode,
    PAS5001_SetDBAReportFormat,
    PAS5001_GetDBAReportFormat,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-04-27*/
    GW_UpdateProvBWInfo,
    /*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    PAS5001_LLIDIsExist,
    PAS5001_DeregisterLLID,
    PAS5001_GetLLIDMac,
    PAS5001_GetLLIDRegisterInfo,
    PAS5001_AuthorizeLLID,
    
    PAS5001_SetLLIDSLA,
    PAS5001_GetLLIDSLA,
    PAS5001_SetLLIDPolice,
    PAS5001_GetLLIDPolice,
    PAS5001_SetLLIDdbaType,
    
    PAS5001_GetLLIDdbaType,
    PAS5001_SetLLIDdbaFlags,
    PAS5001_GetLLIDdbaFlags,
    NULL,     /* GetLLIDHeartbeatOam */
    NULL,     /* SetLLIDHeartbeatOam */
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    GW_GetOnuNum,
    PAS5001_GetAllOnus,
    PAS_OK,   /* ClearAllOnus */
    PAS5001_ResumeAllOnuStatus,
    PAS_OK,   /* SetAllOnuAuthMode */

    PAS5001_SetOnuAuthMode,   
    PAS_OK,   /* SetMacAuth */
    PAS_OK,   /* SetAllOnuBindMode */
    PAS_ERROR,/* CheckOnuRegisterControl */
    PAS_OK,   /* SetAllOnuDefaultBW */

    PAS_OK,   /* SetAllOnuDownlinkPoliceMode */
    PAS_OK,   /* SetOnuDownlinkPoliceMode */
    PAS_OK,   /* SetAllOnuDownlinkPoliceParam */
    PAS_OK,   /* SetAllOnuUplinkDBAParam */
    NULL,     /* SetOnuDownlinkPri2CoSQueueMap */

    PAS5001_ActivePendingOnu,
    PAS5001_ActiveOnePendingOnu,
    PAS5001_ActiveConfPendingOnu,
    PAS5001_ActiveOneConfPendingOnu,
    GW_GetPendingOnu,

    GW_GetUpdatingOnu,
    GW_GetUpdatedOnu,
    GW_GetOnuUpdatingStatusLocal,
    GW_SetOnuUpdateMsg,
    GW_GetOnuUpdateWaiting,

    NULL,     /* SetAllOnuAuthMode2 */
    PAS_OK,   /* SetAllOnuBWParams */
    PAS_OK,   /* SetOnuP2PMode */
    NULL,     /* GetOnuB2PMode */
    NULL,     /* SetOnuB2PMode */

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_GetOnuMode,
    PAS5001_GetMACAddressAuthentication,
    PAS5001_SetAuthorizeMacAddressAccordingListMode,
    PAS5001_GetAuthorizeMacAddressAccordingListMode,
    PAS5001_GetDownlinkBufferConfiguration,

    PAS5001_GetOamInformation,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-02-22*/
    PAS5001_ResumeLLIDStatus,
    GW_SearchFreeOnuIdx,
    GW_GetActOnuIdxByMac,
    /*End:for onu swap by jinhl@2013-02-22*/
    GW_BroadCast_CliCommand,

    PAS_OK,    /*SetAuthEntry*/   
    PAS_ERROR,    /*SetGponAuthEntry*/            
    PAS_OK,    /*SetOnuDefaultMaxMac*/        
    PAS_OK,
    PAS_OK,    /* SetMaxOnu*/ 
    PAS_OK,    /* GetOnuConfDelStatus*/    
    PAS5201_SetCTCOnuTxPowerSupplyControl, /*SetCTCOnuTxPowerSupplyControl*/
#endif

#if 1
/* -------------------OLT 加密管理API----------- */
    PAS5001_SetEncryptMode,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,             /*SetEncryptionPreambleMode*/
    NULL,             /*GetEncryptionPreambleMode*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_GetLLIDEncryptMode,
    PAS5001_StartLLIDEncrypt,
    
    PAS5001_FinishLLIDEncrypt,
    PAS5001_StopLLIDEncrypt,
    PAS5001_SetLLIDEncryptKey,
    PAS5001_FinishLLIDEncryptKey,
#endif

#if 1
/* -------------------OLT 地址表管理API-------- */
    PAS5001_SetMacAgingTime,
    NULL,     /* SetAddrTblCfg */                      
    NULL,     /* GetAddrTblCfg */                      
    PAS5001_GetMacAddrTbl,
    PAS5001_AddMacAddrTbl,

    PAS5001_DelMacAddrTbl,
    PAS5001_RemoveMac,
    PAS5001_ResetAddrTbl,
    PAS_OK,   /*SetOnuMacThreshold*/
    GW_OnuMacCheckEnable,
    
    GW_OnuMacCheckPeriod,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_SetAddressTableFullHandlingMode,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS_ERROR,
    PAS_ERROR,/*GetMacAddrVlanTbl*/
#endif

#if 1
/* -------------------OLT 光路管理API----------- */
    PAS5001_GetOpticalCapability,
    PAS5201_GetOpticsDetail,      /* 原本仅5201支持，后续的PAS版本向后兼容了5001 */
    PAS5001_SetPonRange,
    PAS5001_SetOpticalTxMode,
    PAS5001_GetOpticalTxMode,
    
    PAS5001_SetVirtualScopeAdcConfig,
    PAS5001_GetVirtualScopeMeasurement,
    PAS5001_GetVirtualScopeRssiMeasurement,
    PAS5001_GetVirtualScopeOnuVoltage,
    PAS5001_SetVirtualLLID,
    
    PAS5001_SetOpticalTxMode2,
#endif

#if 1
/* -------------------OLT 监控统计管理API---- */
    PAS5001_GetRawStatistics,
    PAS5001_ResetCounters,
    PAS5001_SetBerAlarm,
    PAS5001_SetFerAlarm,
    PAS_OK,   /* SetPonBerAlarm */                      

    PAS_OK,   /* SetPonFerAlarm */                      
    PAS_OK,   /* SetBerAlarmParams */
    PAS_OK,   /* SetFerAlarmParams */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_SetAlarmConfig,/*SetAlarmConfig*/
    PAS5001_GetAlarmConfig,
    
    PAS5001_GetStatistics,
    PAS5001_OltSelfTest,
    PAS5001_LinkTest,
    NULL,     /* SetLLIDFecMode */
    NULL,     /* GetLLIDFecMode */

    PAS5001_SysDump,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------OLT 倒换API---------------- */
    PAS5001_GetHotSwapCapability,
    PAS5001_GetHotSwapMode,
    PAS5001_SetHotSwapMode,
    PAS5001_ForceHotSwap,
    GW_SetHotSwapParam,

    NULL,     /* RdnOnuRegister */
    NULL,     /* SetRdnConfig */
    NULL,     /* RdnSwitchOver */
    NULL,     /* RdnIsExist */
    NULL,     /* ResetRdnOltRecord */

    NULL,     /* GetRdnState */
    NULL,     /* SetRdnState */
    NULL,     /* GetLLIDRdnDB */
    NULL,     /* SetLLIDRdnDB */
    NULL,     /* GetRdnAddrTbl */

    NULL,     /* RemoveRdnOlt */
    PAS5001_RdnRemoveOlt,
    NULL,     /* RdnSwapOlt */
    NULL,     /* AddSwapOlt */
    NULL,     /* RdnLooseOlt */

    /*Begin:for onu swap by jinhl@2013-02-22*/
    PAS_ERROR,/*RdnLLIDAdd*/
    PAS_ERROR,/*GetRdnLLIDMode*/
    PAS_ERROR,/*SetRdnLLIDMode*/
    PAS_ERROR,/*SetRdnLLIDStdbyToAct*/
    PAS_ERROR,/*SetRdnLLIDRtt*/
    /*End:for onu swap by jinhl@2013-02-22*/
    
    GW_RdnIsReady,
    NULL,     /* GetLLIDRdnRegisterInfo */
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */
    NULL,     /* DumpAllCmc */
    NULL,     /* SetCmcServiceVID */
#endif

    PAS_ERROR /* LastFun */
};

static const OltMgmtIFs s_pas5201Ifs = {
#if 1
/* -------------------OLT基本API------------------- */
    PAS_IsExist,
    PAS5201_GetChipTypeID,
    PAS5201_GetChipTypeName,
    PAS5201_ResetPon,
    PAS5001_RemoveOlt,
    
    GW_CopyOlt,
    GW_CmdIsSupported,
    PAS_OK,   /* SetDebugMode */
    PAS_SetInitParams,
    PAS_SetSystemParams,

    GW_SetPonI2CExtInfo,
    GW_GetPonI2CExtInfo,
    GW_SetCardI2CInfo,
    GW_GetCardI2CInfo,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_WriteMdioRegister,

    PAS5001_ReadMdioRegister,
    PAS5001_ReadI2CRegister,
    PAS5201_GpioAccess,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5201_ReadGpio,
    PAS5201_WriteGpio,

    PAS5001_SendChipCli,
    PAS_OK,   /* SetDeviceName*/
    PAS_OK,   /*ResetPonChip*/
#endif

#if 1
/* -------------------OLT PON管理API--------------- */
    PAS5201_GetVersion,
    PAS5001_GetDBAVersion,
    PAS_OK,   /* ChkVersion */
    GW_ChkDBAVersion,   /* ChkDBAVersion */
    PAS5001_GetCniLinkStatus,

    GW_GetPonWorkStatus, 
    PAS5001_SetAdminStatus,
    GW_GetAdminStatus,   
    PAS5201_SetVlanTpid,
    PAS5201_SetVlanQinQ,

    PAS5201_SetPonFrameSizeLimit,
    PAS5201_GetPonFrameSizeLimit,
    PAS5001_OamIsLimit,
    PAS5201_UpdatePonParams,
    GW_SetPPPoERelayMode,

    GW_SetPPPoERelayParams,
    GW_SetDhcpRelayMode,
    PAS5201_SetIgmpAuthMode,
    PAS5001_SendFrame2PON,
    PAS5001_SendFrame2CNI,

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5201_GetVidDownlinkMode,
    PAS5201_DelVidDownlinkMode,
    PAS5001_GetOltParameters,
    PAS5001_SetOltIgmpSnoopingMode,
    PAS5001_GetOltIgmpSnoopingMode,

    PAS5001_SetOltMldForwardingMode,
    PAS5001_GetOltMldForwardingMode,
    PAS5001_SetDBAReportFormat,
    PAS5001_GetDBAReportFormat,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-04-27*/
    GW_UpdateProvBWInfo,
    /*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    PAS5001_LLIDIsExist,
    PAS5001_DeregisterLLID,
    PAS5001_GetLLIDMac,
    PAS5001_GetLLIDRegisterInfo,
    PAS5001_AuthorizeLLID,
    
    PAS5001_SetLLIDSLA,
    PAS5001_GetLLIDSLA,
    PAS5201_SetLLIDPolice,
    PAS5201_GetLLIDPolice,
    PAS5001_SetLLIDdbaType,
    
    PAS5001_GetLLIDdbaType,
    PAS5001_SetLLIDdbaFlags,
    PAS5001_GetLLIDdbaFlags,
    NULL,     /* GetLLIDHeartbeatOam */
    NULL,     /* SetLLIDHeartbeatOam */
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    GW_GetOnuNum,
    PAS5001_GetAllOnus,
    PAS_OK,   /* ClearAllOnus */
    PAS5201_ResumeAllOnuStatus,
    PAS_OK,   /* SetAllOnuAuthMode */

    PAS5201_SetOnuAuthMode,
    PAS5201_SetMacAuth,
    PAS_OK,   /* SetAllOnuBindMode */
    PAS_ERROR,/* CheckOnuRegisterControl */
    PAS_OK,   /* SetAllOnuDefaultBW */

    PAS_OK,   /* SetAllOnuDownlinkPoliceMode */
    PAS_OK,   /* SetOnuDownlinkPoliceMode */
    PAS_OK,   /* SetAllOnuDownlinkPoliceParam */
    PAS_OK,   /* SetAllOnuUplinkDBAParam */
    PAS5201_SetOnuDownlinkPri2CoSQueueMap,

    PAS5001_ActivePendingOnu,
    PAS5001_ActiveOnePendingOnu,
    PAS5001_ActiveConfPendingOnu,
    PAS5001_ActiveOneConfPendingOnu,
    GW_GetPendingOnu,

    GW_GetUpdatingOnu,
    GW_GetUpdatedOnu,
    GW_GetOnuUpdatingStatusLocal,
    GW_SetOnuUpdateMsg,
    GW_GetOnuUpdateWaiting,

    PAS5201_SetAllOnuAuthMode2,
    PAS_OK,   /* SetAllOnuBWParams */
    PAS_OK,   /* SetOnuP2PMode */
    NULL,     /* GetOnuB2PMode */
    NULL,     /* SetOnuB2PMode */

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_GetOnuMode,
    PAS5001_GetMACAddressAuthentication,
    PAS5001_SetAuthorizeMacAddressAccordingListMode,
    PAS5001_GetAuthorizeMacAddressAccordingListMode,
    PAS5001_GetDownlinkBufferConfiguration,

    PAS5001_GetOamInformation,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-02-22*/
    PAS5001_ResumeLLIDStatus,
    GW_SearchFreeOnuIdx,
    GW_GetActOnuIdxByMac,
    /*End:for onu swap by jinhl@2013-02-22*/
    GW_BroadCast_CliCommand,

    PAS_OK,   /*SetAuthEntry*/   
    PAS_ERROR,    /*SetGponAuthEntry*/            
    PAS_OK,   /*SetOnuDefaultMaxMac*/        
    PAS_OK,
    PAS_OK,    /* SetMaxOnu*/   
    PAS_OK,    /* GetOnuConfDelStatus*/ 
    PAS5201_SetCTCOnuTxPowerSupplyControl,/*SetCTCOnuTxPowerSupplyControl*/
#endif

#if 1
/* -------------------OLT 加密管理API----------- */
    PAS5201_SetEncryptMode,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5201_SetEncryptionPreambleMode,
    PAS5201_GetEncryptionPreambleMode,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_GetLLIDEncryptMode,
    PAS5001_StartLLIDEncrypt,
    
    PAS5001_FinishLLIDEncrypt,
    PAS5001_StopLLIDEncrypt,
    PAS5001_SetLLIDEncryptKey,
    PAS5001_FinishLLIDEncryptKey,
#endif

#if 1
/* -------------------OLT 地址表管理API-------- */
    PAS5201_SetMacAgingTime,
    PAS5201_SetAddressTableConfig,
    PAS5201_GetAddressTableConfig,
    PAS5001_GetMacAddrTbl,
    PAS5201_AddMacAddrTbl,

    PAS5001_DelMacAddrTbl,
    PAS5001_RemoveMac,
    PAS5001_ResetAddrTbl,
    PAS_OK,   /*SetOnuMacThreshold*/
    GW_OnuMacCheckEnable,
    
    GW_OnuMacCheckPeriod,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_SetAddressTableFullHandlingMode,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS_ERROR,
    PAS_ERROR,/*GetMacAddrVlanTbl*/
#endif

#if 1
/* -------------------OLT 光路管理API----------- */
    PAS5201_GetOpticalCapability,
    PAS5201_GetOpticsDetail,
    PAS5201_SetPonRange,
    PAS5001_SetOpticalTxMode,
    PAS5201_GetOpticalTxMode,
    
    PAS5001_SetVirtualScopeAdcConfig,
    PAS5001_GetVirtualScopeMeasurement,
    PAS5001_GetVirtualScopeRssiMeasurement,
    PAS5001_GetVirtualScopeOnuVoltage,
    PAS5001_SetVirtualLLID,
    
    PAS5001_SetOpticalTxMode2,
#endif

#if 1
/* -------------------OLT 监控统计管理API---- */
    PAS5201_GetRawStatistics,
    PAS5001_ResetCounters,
    PAS5001_SetBerAlarm,
    PAS5001_SetFerAlarm,
    PAS_OK,   /* SetPonBerAlarm */                      

    PAS_OK,   /* SetPonFerAlarm */                      
    PAS_OK,   /* SetBerAlarmParams */
    PAS_OK,   /* SetFerAlarmParams */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_SetAlarmConfig,/*SetAlarmConfig*/
    PAS5001_GetAlarmConfig,
    
    PAS5001_GetStatistics,
    PAS5001_OltSelfTest,
    PAS5001_LinkTest,
    PAS5201_SetLLIDFecMode,
    PAS5201_GetLLIDFecMode,

    PAS5001_SysDump,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif
    
#if 1
/* -------------------OLT 倒换API---------------- */
    PAS5201_GetHotSwapCapability,
    PAS5201_GetHotSwapMode,
    PAS5201_SetHotSwapMode,
    PAS5201_ForceHotSwap,
    GW_SetHotSwapParam,

    PAS5201_RdnOnuRegister,
    PAS5201_SetRdnConfig,
    PAS5201_RdnSwitchOver,
    PAS5201_RdnIsExist,
    PAS5201_ResetRdnRecord,

    PAS5201_GetRdnState,
    PAS5201_SetRdnState,
    PAS5201_GetRdnAddrTbl,
    PAS5201_RemoveRdnOlt,
    PAS5201_GetLLIDRdnDB,

    PAS5201_SetLLIDRdnDB,
    PAS5001_RdnRemoveOlt,
    PAS5201_RdnSwapOlt,
    PAS5201_AddSwapOlt,
    PAS5201_RdnLooseOlt,

    /*Begin:for onu swap by jinhl@2013-02-22*/
    PAS5201_RdnLLIDAdd,
    PAS5201_GetRdnLLIDMode,
    PAS5201_SetRdnLLIDMode,
    PAS5201_SetRdnLLIDStdbyToAct,
    PAS5201_SetRdnLLIDRtt,
    /*End:for onu swap by jinhl@2013-02-22*/

    GW_RdnIsReady,
    PAS5201_GetLLIDRdnRegisterInfo,
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */
    NULL,     /* DumpAllCmc */
    NULL,     /* SetCmcServiceVID */
#endif

    PAS_ERROR /* LastFun */
};

static const OltMgmtIFs s_pas5204Ifs = {
#if 1
/* -------------------OLT基本API------------------- */
    PAS_IsExist,
    PAS5204_GetChipTypeID,
    PAS5204_GetChipTypeName,
    PAS5201_ResetPon,
    PAS5001_RemoveOlt,

    GW_CopyOlt,
    GW_CmdIsSupported,
    PAS_OK,   /* SetDebugMode */
    PAS_SetInitParams,
    PAS_SetSystemParams,

    GW_SetPonI2CExtInfo,
    GW_GetPonI2CExtInfo,
    GW_SetCardI2CInfo,
    GW_GetCardI2CInfo,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_WriteMdioRegister,

    PAS5001_ReadMdioRegister,
    PAS5001_ReadI2CRegister,
    PAS5201_GpioAccess,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5201_ReadGpio,
    PAS5201_WriteGpio,

    PAS5001_SendChipCli,
    PAS_OK,   /* SetDeviceName*/
    PAS_OK,   /*ResetPonChip*/
#endif

#if 1
/* -------------------OLT PON管理API--------------- */
    PAS5201_GetVersion,
    PAS5001_GetDBAVersion,
    PAS_OK,   /* ChkVersion */
    GW_ChkDBAVersion,   /* ChkDBAVersion */
    PAS5001_GetCniLinkStatus,

    GW_GetPonWorkStatus, 
    PAS5001_SetAdminStatus,
    GW_GetAdminStatus,   
    PAS5201_SetVlanTpid,
    PAS5201_SetVlanQinQ,

    PAS5201_SetPonFrameSizeLimit,
    PAS5201_GetPonFrameSizeLimit,
    PAS5001_OamIsLimit,
    PAS5201_UpdatePonParams,
    GW_SetPPPoERelayMode,

    GW_SetPPPoERelayParams,
    GW_SetDhcpRelayMode,
    PAS5201_SetIgmpAuthMode,
    PAS5001_SendFrame2PON,
    PAS5001_SendFrame2CNI,

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5201_GetVidDownlinkMode,
    PAS5201_DelVidDownlinkMode,
    PAS5001_GetOltParameters,
    PAS5001_SetOltIgmpSnoopingMode,
    PAS5001_GetOltIgmpSnoopingMode,

    PAS5001_SetOltMldForwardingMode,
    PAS5001_GetOltMldForwardingMode,
    PAS5001_SetDBAReportFormat,
    PAS5001_GetDBAReportFormat,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-04-27*/
    GW_UpdateProvBWInfo,
    /*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    PAS5001_LLIDIsExist,
    PAS5001_DeregisterLLID,
    PAS5001_GetLLIDMac,
    PAS5001_GetLLIDRegisterInfo,
    PAS5001_AuthorizeLLID,
    
    PAS5001_SetLLIDSLA,
    PAS5001_GetLLIDSLA,
    PAS5201_SetLLIDPolice,
    PAS5201_GetLLIDPolice,
    PAS5001_SetLLIDdbaType,
    
    PAS5001_GetLLIDdbaType,
    PAS5001_SetLLIDdbaFlags,
    PAS5001_GetLLIDdbaFlags,
    NULL,     /* GetLLIDHeartbeatOam */
    NULL,     /* SetLLIDHeartbeatOam */
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    GW_GetOnuNum,
    PAS5001_GetAllOnus,
    PAS_OK,   /* ClearAllOnus */
    PAS5204_ResumeAllOnuStatus,
    PAS_OK,   /* SetAllOnuAuthMode */

    PAS5201_SetOnuAuthMode,
    PAS5201_SetMacAuth,
    PAS_OK,   /* SetAllOnuBindMode */
    PAS_ERROR,/* CheckOnuRegisterControl */
    PAS_OK,   /* SetAllOnuDefaultBW */

    PAS_OK,   /* SetAllOnuDownlinkPoliceMode */
    PAS_OK,   /* SetOnuDownlinkPoliceMode */
    PAS_OK,   /* SetAllOnuDownlinkPoliceParam */
    PAS_OK,   /* SetAllOnuUplinkDBAParam */
    PAS5201_SetOnuDownlinkPri2CoSQueueMap,

    PAS5001_ActivePendingOnu,
    PAS5001_ActiveOnePendingOnu,
    PAS5001_ActiveConfPendingOnu,
    PAS5001_ActiveOneConfPendingOnu,
    GW_GetPendingOnu,

    GW_GetUpdatingOnu,
    GW_GetUpdatedOnu,
    GW_GetOnuUpdatingStatusLocal,
    GW_SetOnuUpdateMsg,
    GW_GetOnuUpdateWaiting,

    PAS5201_SetAllOnuAuthMode2,
    PAS_OK,   /* SetAllOnuBWParams */
    PAS_OK,   /* SetOnuP2PMode */
    NULL,     /* GetOnuB2PMode */
    NULL,     /* SetOnuB2PMode */

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_GetOnuMode,
    PAS5001_GetMACAddressAuthentication,
    PAS5001_SetAuthorizeMacAddressAccordingListMode,
    PAS5001_GetAuthorizeMacAddressAccordingListMode,
    PAS5001_GetDownlinkBufferConfiguration,

    PAS5001_GetOamInformation,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-02-22*/
    PAS5001_ResumeLLIDStatus,
    GW_SearchFreeOnuIdx,
    GW_GetActOnuIdxByMac,
    /*End:for onu swap by jinhl@2013-02-22*/
    GW_BroadCast_CliCommand,

    PAS_OK,    /*SetAuthEntry*/   
    PAS_ERROR,    /*SetGponAuthEntry*/            
    PAS_OK,    /*SetOnuDefaultMaxMac*/        
    PAS_OK,    /*SetOnuPortStatsTimeOut*/
    PAS_OK,    /* SetMaxOnu*/  
    PAS_OK,    /* GetOnuConfDelStatus*/
    PAS5201_SetCTCOnuTxPowerSupplyControl, /*SetCTCOnuTxPowerSupplyControl*/
#endif

#if 1
/* -------------------OLT 加密管理API----------- */
    PAS5201_SetEncryptMode,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5201_SetEncryptionPreambleMode,
    PAS5201_GetEncryptionPreambleMode,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_GetLLIDEncryptMode,
    PAS5001_StartLLIDEncrypt,
    
    PAS5001_FinishLLIDEncrypt,
    PAS5001_StopLLIDEncrypt,
    PAS5001_SetLLIDEncryptKey,
    PAS5001_FinishLLIDEncryptKey,
#endif

#if 1
/* -------------------OLT 地址表管理API-------- */
    PAS5201_SetMacAgingTime,
    PAS5201_SetAddressTableConfig,
    PAS5201_GetAddressTableConfig,
    PAS5001_GetMacAddrTbl,
    PAS5201_AddMacAddrTbl,

    PAS5001_DelMacAddrTbl,
    PAS5001_RemoveMac,
    PAS5001_ResetAddrTbl,
    PAS_OK,   /*SetOnuMacThreshold*/
    GW_OnuMacCheckEnable,
    
    GW_OnuMacCheckPeriod,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_SetAddressTableFullHandlingMode,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS_ERROR,
    PAS_ERROR,/*GetMacAddrVlanTbl*/
#endif

#if 1
/* -------------------OLT 光路管理API----------- */
    PAS5201_GetOpticalCapability,
    PAS5201_GetOpticsDetail,
    PAS5201_SetPonRange,
    PAS5001_SetOpticalTxMode,
    PAS5201_GetOpticalTxMode,
    
    PAS5001_SetVirtualScopeAdcConfig,
    PAS5001_GetVirtualScopeMeasurement,
    PAS5001_GetVirtualScopeRssiMeasurement,
    PAS5001_GetVirtualScopeOnuVoltage,
    PAS5001_SetVirtualLLID,
    
    PAS5001_SetOpticalTxMode2,
#endif

#if 1
/* -------------------OLT 监控统计管理API---- */
    PAS5201_GetRawStatistics,
    PAS5001_ResetCounters,
    PAS5001_SetBerAlarm,
    PAS5001_SetFerAlarm,
    PAS_OK,   /* SetPonBerAlarm */                      

    PAS_OK,   /* SetPonFerAlarm */                      
    PAS_OK,   /* SetBerAlarmParams */
    PAS_OK,   /* SetFerAlarmParams */
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    PAS5001_SetAlarmConfig,/*SetAlarmConfig*/
    PAS5001_GetAlarmConfig,
    
    PAS5001_GetStatistics,
    PAS5001_OltSelfTest,
    PAS5001_LinkTest,
    PAS5201_SetLLIDFecMode,
    PAS5201_GetLLIDFecMode,

    PAS5001_SysDump,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif
    
#if 1
/* -------------------OLT 倒换API---------------- */
    PAS5201_GetHotSwapCapability,
    PAS5201_GetHotSwapMode,
    PAS5201_SetHotSwapMode,
    PAS5201_ForceHotSwap,
    GW_SetHotSwapParam,

    PAS5201_RdnOnuRegister,
    PAS5201_SetRdnConfig,
    PAS5201_RdnSwitchOver,
    PAS5201_RdnIsExist,
    PAS5201_ResetRdnRecord,

    PAS5201_GetRdnState,
    PAS5201_SetRdnState,
    PAS5201_GetRdnAddrTbl,
    PAS5201_RemoveRdnOlt,
    PAS5201_GetLLIDRdnDB,

    PAS5201_SetLLIDRdnDB,
    PAS5001_RdnRemoveOlt,
    PAS5201_RdnSwapOlt,
    PAS5201_AddSwapOlt,
    PAS5201_RdnLooseOlt,
    
    /*Begin:for onu swap by jinhl@2013-02-22*/
    PAS5201_RdnLLIDAdd,
    PAS5201_GetRdnLLIDMode,
    PAS5201_SetRdnLLIDMode,
    PAS5201_SetRdnLLIDStdbyToAct,
    PAS5201_SetRdnLLIDRtt,
    /*End:for onu swap by jinhl@2013-02-22*/

    GW_RdnIsReady,
    PAS5201_GetLLIDRdnRegisterInfo,
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */
    NULL,     /* DumpAllCmc */
    NULL,     /* SetCmcServiceVID */
#endif

    PAS_ERROR /* LastFun */
};



void OLT_Pas5001_Support()
{
    OLT_RegisterAdapter(OLT_ADAPTER_PAS5001, &s_pas5001Ifs);
    g_cszPonChipTypeNames[PONCHIP_PAS5001] = "PAS5001";
}

void OLT_Pas5201_Support()
{
    OLT_RegisterAdapter(OLT_ADAPTER_PAS5201, &s_pas5201Ifs);
    g_cszPonChipTypeNames[PONCHIP_PAS5201] = "PAS5201";
}

void OLT_Pas5204_Support()
{
    OLT_RegisterAdapter(OLT_ADAPTER_PAS5204, &s_pas5204Ifs);
    g_cszPonChipTypeNames[PONCHIP_PAS5204] = "PAS5204";
}

void OLT_PasGlobal_Support()
{
    OLT_RegisterAdapter(OLT_ADAPTER_GLOBAL, &s_pasGlobalIfs);
}


#ifdef __cplusplus

}

#endif


