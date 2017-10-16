/****************************************************************************
*
*        Copyright(C), 2000-2006 GW TECHNOLOGIES CO.,LTD.
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*              GW Technologies Co.,Ltd.
*
*     All information contained in this document is    GW Technologies Co.,Ltd.
*     company private, proprietary, and trade secret.
*
*	modified :wutw 2006-10-18
*			针对dya方修改的命令,进行相应的修改,并去除onu-fe config的showrun代码
*****************************************************************************/
#include "syscfg.h"

#ifdef	__cplusplus
extern "C"
{
#endif


#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_io.h"
#include "cli/cli.h"
#include "cli/cl_cmd.h"
#include "cli/cl_mod.h"
#include "addr_aux.h"
#include "linux/inetdevice.h"

#include "ifm/ifm_type.h"
#include "ifm/ifm_pub.h"
#include "ifm/ifm_gtable.h"
#include "ifm/ifm_aux.h"
#include "ifm/ifm_act.h"
#include "ifm/ifm_cli.h"
#include "ifm/ifm_task.h"
#include "ifm/ifm_lock.h"
#include "ifm/ifm_debug.h"
#include "ifm/eth/eth_aux.h"
#include "vos_ctype.h"

#include "interface/interface_task.h"
#include "sys/console/sys_console.h"
#include "sys/main/sys_main.h"
#include "cpi/ctss/ctss_ifm.h"
/*added by wutw 2006/11/10*/
#include "olt_cli.h"

/******UNI port configration****************************/


QDEFUN ( into_epon_onu_fe_node,
         into_epon_onu_fe_node_cmd,
         "fe <1-64>",
         "Into the onu_fe node\n"
         "Please input the port number\n",
         &g_ulIFMQue )
{
    CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };
    ULONG   ulIfIndex = 0;
    ULONG   ulFeIndex=0;
    /*ULONG   ulState = 0;*/
    ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
    CHAR    prompt[64] = { 0 };
    
    ulIfIndex =(ULONG) vty->index;
    PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
    ulOnuFeid= ( ULONG ) VOS_AtoL( argv[ 0 ] );
    ulFeIndex = IFM_PON_CREATE_INDEX( ulSlot, ulPort, ulOnuid, ulOnuFeid);	
    vty->index = (void *)ulFeIndex;
    vty->node = ONU_FE_NODE;

    VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
    VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%d/%d/%d:%s", "onu", ulSlot, ulPort, ulOnuid, argv[ 0 ] );
    
    VOS_StrCpy( prompt, "%s(epon-" );
    VOS_StrCat( prompt, ifName );
    VOS_StrCat( prompt, ")#" );
    vty_set_prompt( vty, prompt );
      
    return CMD_SUCCESS;
}


QDEFUN ( uni_duplex_config,
         uni_duplex_config_cmd,
         "duplex [half|full]",
         "Duplex setting\n"
         "Duplex enable half\n"
         "Duplex enable full\n",
         &g_ulIFMQue )
{
        
    return CMD_SUCCESS;
}

QDEFUN ( uni_speed_config,
         uni_speed_config_cmd,
         "speed [10|100|1000]",
         "Config port speed\n"
         "Set port's speed to 10Mbps\n"
         "Set port's speed to 100Mbps\n"
         "Set port's speed to 1000Mbps\n",
         &g_ulIFMQue )
{
        
    return CMD_SUCCESS;
}

 QDEFUN ( uni_auto_enable_if,
         uni_auto_enable_if_cmd,
         "auto [enable|disable]",
         "Auto negotiation setting\n"
         "Auto negotiation enable\n"
         "Auto negotiation disable\n",
         &g_ulIFMQue )
{
        
    return CMD_SUCCESS;
}


QDEFUN ( uni_acl_enable_config,
         uni_acl_enable_config_cmd,
         "acl [enable|disable]",
         "Acl setting\n"
         "Acl enable\n"
         "Acl disable\n",
         &g_ulIFMQue )
{
        
    return CMD_SUCCESS;
}


QDEFUN ( uni_acl_type_config,
         uni_acl_type_config_cmd,
         "acl-type <type>",
         "Acl type setting\n"
         "Please input acl type\n",
         &g_ulIFMQue )
{
        
    return CMD_SUCCESS;
}

QDEFUN ( fe_flowcontrol_enable_if,
         fe_flowcontrol_enable_if_cmd,
         "flowcontrol [enable|disable]",
         "Flowcontrol setting\n"
         "Flowcontrol enable\n"
         "Flowcontrol disable\n",
         &g_ulIFMQue )
{
        
    return CMD_SUCCESS;
}

QDEFUN( fe_port_learn_func,
        fe_port_learn_cmd,
        "learning [enable|disable]",
        "Enable or disable learning\n"
        "Enable learning\n"
        "Disable learning\n",
        &g_ulIFMQue  
        )
{
        
    return CMD_SUCCESS;
}


QDEFUN( uni_show_acl,
        uni_show_acl_cmd,
        "show port information",
        DescStringCommonShow
        "Show the port\n"
        "Show the information of the port \n",
        &g_ulIFMQue  
        )
{
        
    return CMD_SUCCESS;
}

QDEFUN( show_fe,
        show_fe_cmd,
        "show onu ethernet information",
        DescStringCommonShow
        "Show the onu\n"
        "Show the onu ethernet\n"
        "Show the information of the onu ethernet\n",
        &g_ulIFMQue  
        )
{
        
    return CMD_SUCCESS;
}


LONG ONUFE_show_run( ULONG ulIfindex, VOID * p )
{
	/*struct vty          *vty = p;*/
	    
	/*vty_out( vty, "!Onu-fe config\r\n" );*/

	/*此处用来取出onu 的配置数据与默认数据之间进行比较，有差异打印出来
	注意打印格式其实是相应的设置命令。因为ONU的数目多,需要一个遍历过程,
	把所有与默认配置不同的内容,都要采用相应的配置命令来打印输出一遍,
	相当于完成了一次配置.*/

	
	/*vty_out( vty, "!\r\n\r\n" );*/

	return VOS_OK;
}


int onu_fe_init_func()
{
    return VOS_OK;
}

int onu_fe_showrun( struct vty * vty )
{
    IFM_READ_LOCK;
    ONUFE_show_run( 0, vty );
    IFM_READ_UNLOCK;
    
    return VOS_OK;
}


int onu_fe_config_write ( struct vty * vty )
{
    return VOS_OK;
}




struct cmd_node onu_fe_node =
{
    ONU_FE_NODE,
    NULL,
    1
};


LONG onu_fe_node_install()
{
    install_node( &onu_fe_node, onu_fe_config_write);
    onu_fe_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_ONU);
    if ( !onu_fe_node.prompt )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }
    install_default( ONU_FE_NODE );
    return VOS_OK;
}

LONG onu_fe_module_init()
{
    struct cl_cmd_module * onu_fe_module = NULL;

    onu_fe_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_ONU);
    if ( !onu_fe_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) onu_fe_module, sizeof( struct cl_cmd_module ) );

    onu_fe_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_ONU);
    if ( !onu_fe_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( onu_fe_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( onu_fe_module->module_name, "onufe" );

    onu_fe_module->init_func = onu_fe_init_func;
    onu_fe_module->showrun_func = onu_fe_showrun;
    onu_fe_module->next = NULL;
    onu_fe_module->prev = NULL;

    cl_install_module( onu_fe_module );

    return VOS_OK;
}


LONG ONUFE_CommandInstall()
{    
    install_element ( ONU_NODE, &into_epon_onu_fe_node_cmd);    
    install_element ( ONU_FE_NODE, &uni_duplex_config_cmd);
    install_element ( ONU_FE_NODE, &uni_speed_config_cmd);
    install_element ( ONU_FE_NODE, &uni_auto_enable_if_cmd);
    install_element ( ONU_FE_NODE, &uni_acl_enable_config_cmd);
    install_element ( ONU_FE_NODE, &uni_acl_type_config_cmd);
    install_element ( ONU_FE_NODE, &fe_flowcontrol_enable_if_cmd);
    install_element ( ONU_FE_NODE, &fe_port_learn_cmd);
    install_element ( ONU_FE_NODE, &uni_show_acl_cmd);
    install_element ( ONU_FE_NODE, &show_fe_cmd);
   
    return VOS_OK;
}

LONG ONUFE_CliInit()
{  
/*modified by wutw 2006/11/28
    onu_fe_node_install();
    onu_fe_module_init();
    ONUFE_CommandInstall();
 */   
    return VOS_OK;
}



#ifdef	__cplusplus
}
#endif/* __cplusplus */
