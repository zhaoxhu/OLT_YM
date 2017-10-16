/*--------------------------------------------------------------------*\
 *  File name:  access_iden_cli.c 
 *  Author:     wugang
 *  Version:    V1.0
 *  Date:       2009-06-05
 *  Description: ONU线路接入命令
\*--------------------------------------------------------------------*/

#include "access_id.h"
#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "cli/cl_buf.h"
#include "cli/cl_cmd.h"
#include "sys/devsm/devsm_cli.h"

#include "V2R1_product.h"

#define ACCESS_ID_TASK_NAME "tAccessId"

LONG ACCESS_ID_TaskID = 0;

extern char Access_id_module_state;

#ifdef ONU_PPPOE_RELAY
DEFUN(onu_pppoe_relay_disable,
	onu_pppoe_relay_disable_cmd,
	"pppoe relay disable",
	"Config pppoe\n"
	"Config pppoe relay\n"
	"Disable onu pppoe relay\n")
{
  ULONG ulArgv[10] = {RELAY_TYPE_PPPOE,0,0,0,0,0,0,0,0,0};

  if (PPPOE_RELAY_DISABLE == g_PPPOE_relay)
  {
    vty_out(vty, "  Onu pppoe relay is disabled already.\r\n");
  }
  else
  {
    if (ACCESS_ID_MODULE_BUSY == Access_id_module_state)
    {
      vty_out(vty, "  Access identifier module is busy now, please wait for 1 minute and try again.\r\n");
    }
    else
    {
#if 1
      OLT_SetPPPoERelayMode(OLT_ID_ALL, PPPOE_RELAY_DISABLE, 0);
#else
      Access_id_module_state = ACCESS_ID_MODULE_BUSY;
      g_PPPOE_relay = PPPOE_RELAY_DISABLE;
      ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
      ASSERT( ACCESS_ID_TaskID != 0 );
#endif
    }
  }
	return CMD_SUCCESS;
}

DEFUN(onu_pppoe_relay_enable,
	onu_pppoe_relay_enable_cmd,
	"pppoe relay enable {[gwd|dsl-forum|fpt]}*1", 
	"Config pppoe\n"
	"Config pppoe relay\n"
	"Enable onu pppoe relay\n"
	"Set onu pppoe relay mode to GWD private mode\n" 
	"Set onu pppoe relay mode to DSL Forum mode\n"
	"Set onu pppoe relay mode to FPT private mode\n")
{
  ULONG ulArgv[10] = {RELAY_TYPE_PPPOE,0,0,0,0,0,0,0,0,0};
  char Is_chaged = FALSE;/*标识配置是否被改变了*/
  
  if (0 == argc)
  {
    if (PPPOE_RELAY_ENABLE == g_PPPOE_relay)
    {
      vty_out(vty, "  Onu pppoe relay is enabled already.\r\n");
    }
    else
    {    
      if (ACCESS_ID_MODULE_BUSY == Access_id_module_state)
      {
        vty_out(vty, "  Access identifier module is busy now, please wait for 1 minute and try again.\r\n");
      }
      else
      {
#if 1
        OLT_SetPPPoERelayMode(OLT_ID_ALL, PPPOE_RELAY_ENABLE, 0);
#else
        Access_id_module_state = ACCESS_ID_MODULE_BUSY;
        g_PPPOE_relay = PPPOE_RELAY_ENABLE;
        ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
        ASSERT( ACCESS_ID_TaskID != 0 );
#endif
      }
    }
  }
  else
  {
    if (ACCESS_ID_MODULE_BUSY == Access_id_module_state)
    {
      vty_out(vty, "  Access identifier module is busy now, please wait for 1 minute and try again.\r\n");
    }
    else
    {
      if (PPPOE_RELAY_ENABLE == g_PPPOE_relay)
      {
        Is_chaged = FALSE;/*enabled arleady*/
      }   
      else
      {
        g_PPPOE_relay = PPPOE_RELAY_ENABLE;
        Is_chaged = TRUE;
      }
      
      if (0 == VOS_StrCmp(argv[0], "gwd"))
      {
        if (PPPOE_RELAY_GWD_PRIVATE_MODE == g_PPPOE_relay_mode)
        {
          if(TRUE == Is_chaged)/*虽然mode没改，但是由disable变成了enable*/
          {
#if 1
            OLT_SetPPPoERelayMode(OLT_ID_ALL, PPPOE_RELAY_ENABLE, PPPOE_RELAY_GWD_PRIVATE_MODE);
#else
            Access_id_module_state = ACCESS_ID_MODULE_BUSY;      
            g_PPPOE_relay_mode = PPPOE_RELAY_GWD_PRIVATE_MODE;
            ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
            ASSERT( ACCESS_ID_TaskID != 0 );
#endif
          }
          else
          {
            Is_chaged = FALSE;
          }
        }
        else/*此时的配置肯定被修改了*/
        {
#if 1
          OLT_SetPPPoERelayMode(OLT_ID_ALL, PPPOE_RELAY_ENABLE, PPPOE_RELAY_GWD_PRIVATE_MODE);
#else
          Access_id_module_state = ACCESS_ID_MODULE_BUSY;      
          g_PPPOE_relay_mode = PPPOE_RELAY_GWD_PRIVATE_MODE;
          ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
          ASSERT( ACCESS_ID_TaskID != 0 );
#endif
          Is_chaged = TRUE;
        }
      }
      else if(0 == VOS_StrCmp(argv[0], "dsl-forum"))
      {
        if (PPPOE_RELAY_DSL_RORUM_MODE == g_PPPOE_relay_mode)
        {
          if(TRUE == Is_chaged)/*虽然mode没改，但是由disable变成了enable*/
          {
#if 1
            OLT_SetPPPoERelayMode(OLT_ID_ALL, PPPOE_RELAY_ENABLE, PPPOE_RELAY_DSL_RORUM_MODE);
#else
            Access_id_module_state = ACCESS_ID_MODULE_BUSY;      
            g_PPPOE_relay_mode = PPPOE_RELAY_DSL_RORUM_MODE;
            ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
            ASSERT( ACCESS_ID_TaskID != 0 );
#endif
          }
          else
          {
            Is_chaged = FALSE;
          }
        }
        else/*此时的配置肯定被修改了*/
        {
#if 1
          OLT_SetPPPoERelayMode(OLT_ID_ALL, PPPOE_RELAY_ENABLE, PPPOE_RELAY_DSL_RORUM_MODE);
#else
          Access_id_module_state = ACCESS_ID_MODULE_BUSY;
          g_PPPOE_relay_mode = PPPOE_RELAY_DSL_RORUM_MODE;
          ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
          ASSERT( ACCESS_ID_TaskID != 0 );
#endif
          Is_chaged = TRUE;
        }
      }    
      else
      {
        if (PPPOE_RELAY_FPT_PRIVATE_MODE == g_PPPOE_relay_mode)
        {
          if(TRUE == Is_chaged)/*虽然mode没改，但是由disable变成了enable*/
          {
#if 1
            OLT_SetPPPoERelayMode(OLT_ID_ALL, PPPOE_RELAY_ENABLE, PPPOE_RELAY_FPT_PRIVATE_MODE);
#else
            Access_id_module_state = ACCESS_ID_MODULE_BUSY;      
            g_PPPOE_relay_mode = PPPOE_RELAY_DSL_RORUM_MODE;
            ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
            ASSERT( ACCESS_ID_TaskID != 0 );
#endif
          }
          else
          {
            Is_chaged = FALSE;
          }
        }
        else/*此时的配置肯定被修改了*/
        {
#if 1
          OLT_SetPPPoERelayMode(OLT_ID_ALL, PPPOE_RELAY_ENABLE, PPPOE_RELAY_FPT_PRIVATE_MODE);
#else
          Access_id_module_state = ACCESS_ID_MODULE_BUSY;
          g_PPPOE_relay_mode = PPPOE_RELAY_DSL_RORUM_MODE;
          ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
          ASSERT( ACCESS_ID_TaskID != 0 );
#endif
          Is_chaged = TRUE;
        }
      }
    }

    if(FALSE == Is_chaged)
    {
      vty_out(vty,"  Onu pppoe relay configuration doesn't be changed!\r\n");
    }
    else
    {
      vty_out(vty,"  Set Onu pppoe relay configuration.\r\n");
    }
  }
	return CMD_SUCCESS;
}


DEFUN(onu_pppoe_circuitid_set,
	onu_pppoe_circuitid_set_cmd,
	"pppoe relay circuitid-set <str>", 
	"Config pppoe\n"
	"Config pppoe relay\n" 
	"Set circuit id value\n"
	"please put in circuit id value(up to 20 characters)\n")
{
  ULONG ulArgv[10] = {RELAY_TYPE_PPPOE,0,0,0,0,0,0,0,0,0};
  if (ACCESS_ID_MODULE_BUSY == Access_id_module_state)
  {
    vty_out(vty, "  Access identifier module is busy now, please wait for 1 minute and try again.\r\n");
  }
  else
  {
    int str_lenth = 0;
    int i;
	struct buffer *b;
	char   *new_str;

    Access_id_module_state = ACCESS_ID_MODULE_BUSY;
	b = buffer_new(0, 50);
	if (b == NULL)
    {
      vty_out(vty,"\r\n can't g_malloc enongh space ! \r\n");
      Access_id_module_state = ACCESS_ID_MODULE_FREE;
      return CMD_SUCCESS;
    }
	  for (i = 0; i < argc; i++)
	  {
#ifndef _DISTRIBUTE_PLATFORM_
		  buffer_putstr(b, (unsigned char *) argv[i]);
		  buffer_putc(b, ' ');
#else
		  if (i > 0)
		  {
			  buffer_putc(b, ' ');
		  }
		  buffer_putstr(b, (unsigned char *) argv[i]);
#endif
	  }
	  buffer_putc(b, '\0');
  
	new_str = buffer_getstr(b);  
    str_lenth = strlen(new_str);
  
    if(str_lenth > 30)
    {
      vty_out(vty,"\r\n the value input is too long ! \r\n");
      Access_id_module_state = ACCESS_ID_MODULE_FREE;
      return CMD_SUCCESS;
    }

#if 1
    OLT_SetPPPoERelayParams(OLT_ID_ALL, PPPOE_RELAY_PARAMID_STRHEAD, new_str, str_lenth);
    Access_id_module_state = ACCESS_ID_MODULE_FREE;
#else
    VOS_MemCpy(PPPOE_Relay_Maual_String_head, new_str, str_lenth+1);/*把字符串终止符一起复制过去，方便打印*/
	PPPOE_Relay_Maual_String_Len = str_lenth;
	
	if (PPPOE_RELAY_ENABLE == g_PPPOE_relay)
	{
	    ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
	    ASSERT( ACCESS_ID_TaskID != 0 );
	}
	else
	{
		Access_id_module_state = ACCESS_ID_MODULE_FREE;
	}
#endif

    vty_out(vty, "  Pppoe relay circuit id value is:%s", VTY_NEWLINE);
    vty_out(vty, "  %s%s", new_str, VTY_NEWLINE);
  
	VOS_Free(new_str);
	buffer_free(b);
  }
  
  return CMD_SUCCESS;
  
}

/*删除手动配置的字符串
*/
DEFUN(onu_pppoe_relay_circiutid_send,
	undo_onu_pppoe_relay_circuitid_cmd,
	"undo pppoe relay manually-set-circuitid\n",
	"Delete system's setting\n"
	"Delete pppoe config\n"
	"Delete pppoe relay config\n"
	"Delete manually-set circuit id value\n" )
{
	ULONG ulArgv[10] = {RELAY_TYPE_PPPOE,0,0,0,0,0,0,0,0,0};
	if (ACCESS_ID_MODULE_BUSY == Access_id_module_state)
	{
		vty_out(vty, "  Access identifier module is busy now, please wait for 1 minute and try again.\r\n");
	}
	else
	{
#if 1
        OLT_SetPPPoERelayParams(OLT_ID_ALL, PPPOE_RELAY_PARAMID_STRHEAD, NULL, 0);
#else
		Access_id_module_state = ACCESS_ID_MODULE_BUSY;
		*PPPOE_Relay_Maual_String_head = '\0';
		if (PPPOE_RELAY_DISABLE == g_PPPOE_relay)
		{
			Access_id_module_state = ACCESS_ID_MODULE_FREE;
		}
		else
		{
			ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
			ASSERT( ACCESS_ID_TaskID != 0 );
		}
#endif
	}
	return CMD_SUCCESS;
}

DEFUN(
  onu_pppoe_relay_show,
  onu_pppoe_relay_show_cmd,
  "show pppoe-relay {[mode|circuit-id-value]}*1",
  DescStringCommonShow
  "Show onu pppoe relay information\n"
  "Show onu pppoe relay mode\n"
  "Show onu circuit id value\n"
  )
{
  char *str;
  char oltName[MAXDEVICENAMELEN];
  ULONG nameLen = 0;
  
  /*显示pppoe relay是否使能*/
  if(0 == argc)
  {
    if (PPPOE_RELAY_DISABLE == g_PPPOE_relay)
		vty_out(vty, "  Onu pppoe relay is disabled.\r\n");
    else
		vty_out(vty, "  Onu pppoe relay is enabled.\r\n");
  }
  else
  {
    /*显示pppoe relay代理模式*/
    if (0 == VOS_StrCmp(argv[0], "mode"))
    {
      if (PPPOE_RELAY_GWD_PRIVATE_MODE == g_PPPOE_relay_mode)
        vty_out(vty, "  Onu pppoe relay current mode is GWD private mode.\r\n");
      else if(PPPOE_RELAY_DSL_RORUM_MODE == g_PPPOE_relay_mode)
        vty_out(vty, "  Onu pppoe relay current mode is DSL Forum mode.\r\n");
      else
        vty_out(vty, "  Onu pppoe relay current mode is FPT private mode.\r\n");
    }
    
    /*显示pppoe relay获取字符串的方式*/
    else
    {
      if ('\0' != *PPPOE_Relay_Maual_String_head)
      {
        vty_out(vty, "  Onu pppoe relay get circuit id value manually.\r\n");
        str = PPPOE_Relay_Maual_String_head;
        vty_out(vty, "  Pppoe relay circuit id value is:%s", VTY_NEWLINE);
        vty_out(vty, "  %s%s", str, VTY_NEWLINE);
      }
	  else
  	  {
  	  /* modified by xieshl 20100928 */
        UCHAR *pSysMac;
        char Tem_string[24];
        char *epon_tech = " epon ";/*空格是要求的*/ /*epon技术标识*/
        
        /*获取OLT的MAC地址*/
        /*funReadMacAddFromNvram( &macStructure );*/	/* modified by xieshl 20091216, 防止读eeprom */
	/*VOS_MemCpy( &macStructure, SYS_PRODUCT_BASEMAC, 6 );*/
	pSysMac = SYS_PRODUCT_BASEMAC;

	GetOltDeviceName( oltName, &nameLen);
	if( nameLen >20 )
		nameLen = 20;
	oltName[nameLen] = '\0';
    
        /*mac地址*/
          VOS_Sprintf( Tem_string,"%02x%02x%02x%02x%02x%02x", pSysMac[0], pSysMac[1], pSysMac[2], pSysMac[3], pSysMac[4], pSysMac[5]);

        vty_out(vty, "  Onu pppoe relay get circuit id value automatically.\r\n");
        vty_out(vty, "  Pppoe relay circuit id value is:%s", VTY_NEWLINE);
        vty_out(vty, "  <%s>%s%s%s", oltName,Tem_string, epon_tech, VTY_NEWLINE);        
      }    
    }
  }
  return CMD_SUCCESS;
}
#endif

#ifdef ONU_DHCP_RELAY
DEFUN(onu_dhcp_relay_enable,
	onu_dhcp_relay_enable_cmd,
	"dhcp relay [enable|disable]",
	"Config dhcp\n"
	"Config dhcp relay\n"
	"Enable onu dhcp relay\n"
	"Disable onu dhcp relay\n")
{
	ULONG ulArgv[10] = {RELAY_TYPE_DHCP,0,0,0,0,0,0,0,0,0};

	if (0 == VOS_StrCmp(argv[0], "enable"))
	{
		if (DHCP_RELAY_ENABLE == g_DHCP_relay)
		{
			vty_out(vty, "  Onu dhcp relay is enabled already.\r\n");
		}
		else
		{    
			if (ACCESS_ID_MODULE_BUSY == Access_id_module_state)
			{
				vty_out(vty, "  Access identifier module is busy now, please wait for 1 minute and try again.\r\n");
			}
			else
			{
#if 1
                OLT_SetDhcpRelayMode(OLT_ID_ALL, DHCP_RELAY_ENABLE, 0);
#else
				Access_id_module_state = ACCESS_ID_MODULE_BUSY;
				g_DHCP_relay = DHCP_RELAY_ENABLE;
				ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
				ASSERT( ACCESS_ID_TaskID != 0 );
#endif
			}
		}
	}
	else
	{
		if (DHCP_RELAY_DISABLE == g_DHCP_relay)
		{
			vty_out(vty, "  Onu dhcp relay is disabled already.\r\n");
		}
		else
		{
			if (ACCESS_ID_MODULE_BUSY == Access_id_module_state)
			{
				vty_out(vty, "  Access identifier module is busy now, please wait for 1 minute and try again.\r\n");
			}
			else
			{
#if 1
                OLT_SetDhcpRelayMode(OLT_ID_ALL, DHCP_RELAY_DISABLE, 0);
#else
				Access_id_module_state = ACCESS_ID_MODULE_BUSY;
				g_DHCP_relay = DHCP_RELAY_DISABLE;
				ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
				ASSERT( ACCESS_ID_TaskID != 0 );
#endif
			}
		}
	}
	return CMD_SUCCESS;
}

DEFUN(onu_dhcp_relay_mode,
	onu_dhcp_relay_mode_cmd,
	"dhcp relay mode {[ctc|standard]}*1", 
	"Config dhcp\n"
	"Config dhcp relay\n"
	"Config dhcp relay mode\n"
	"Set onu dhcp relay mode to CTC mode\n" 
	"Set onu dhcp relay mode to standard mode\n" )
{
	ULONG ulArgv[10] = {RELAY_TYPE_DHCP,0,0,0,0,0,0,0,0,0};
	
	if(0 == VOS_StrCmp(argv[0], "ctc"))
	{
		if (DHCP_RELAY_CTC_MODE == g_DHCP_relay_mode)
		{
			vty_out(vty, "  Onu dhcp relay mode is CTC already.\r\n");
		}
		else
		{
#if 1
            OLT_SetDhcpRelayMode(OLT_ID_ALL, -1, DHCP_RELAY_CTC_MODE);
#else

			if(DHCP_RELAY_DISABLE == g_DHCP_relay)/*enable才发OAM*/
			{
				g_DHCP_relay_mode = DHCP_RELAY_CTC_MODE;
			}
			else
			{
				if (ACCESS_ID_MODULE_BUSY == Access_id_module_state)
				{
					vty_out(vty, "  Access identifier module is busy now, please wait for 1 minute and try again.\r\n");
				}
				else
				{
					Access_id_module_state = ACCESS_ID_MODULE_BUSY;      
					g_DHCP_relay_mode = DHCP_RELAY_CTC_MODE;
					ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
					ASSERT( ACCESS_ID_TaskID != 0 );
				}
			}
#endif
		}
	}
	else if(0 == VOS_StrCmp(argv[0], "standard"))
	{
		if (DHCP_RELAY_STD_MODE == g_DHCP_relay_mode)
		{
			vty_out(vty, "  Onu dhcp relay mode is standard already.\r\n");
		}
		else
		{
#if 1
            OLT_SetDhcpRelayMode(OLT_ID_ALL, -1, DHCP_RELAY_STD_MODE);
#else
			if(DHCP_RELAY_DISABLE == g_DHCP_relay)/*enable 才发OAM*/
			{
				g_DHCP_relay_mode = DHCP_RELAY_STD_MODE;
			}
			else
			{
				if (ACCESS_ID_MODULE_BUSY == Access_id_module_state)
				{
					vty_out(vty, "  Access identifier module is busy now, please wait for 1 minute and try again.\r\n");
				}
				else
				{
					Access_id_module_state = ACCESS_ID_MODULE_BUSY;      
					g_DHCP_relay_mode = DHCP_RELAY_STD_MODE;
					ACCESS_ID_TaskID = VOS_TaskCreate( ACCESS_ID_TASK_NAME, 189, PON_Port_Send_Oam, ulArgv );
					ASSERT( ACCESS_ID_TaskID != 0 );
				}
			}
#endif
		}
	}
	return CMD_SUCCESS;
}

DEFUN(
  onu_dhcp_relay_show,
  onu_dhcp_relay_show_cmd,
  "Show dhcp-relay {[mode]}*1",
  DescStringCommonShow
  "Show onu dhcp relay information\n"
  "Show onu dhcp relay mode\n"
  )
{
	/*显示dhcp relay是否使能*/
	if(0 == argc)
	{
		if (DHCP_RELAY_DISABLE == g_DHCP_relay)
			vty_out(vty, "  Onu dhcp relay is disabled.\r\n");
		else
			vty_out(vty, "  Onu dhcp relay is enabled.\r\n");
	}
	else
	{
		/*显示dhcp relay代理模式*/

		if (DHCP_RELAY_CTC_MODE == g_DHCP_relay_mode)
			vty_out(vty, "  Onu dhcp relay current mode is CTC mode.\r\n");
		else
			vty_out(vty, "  Onu dhcp relay current mode is standard mode.\r\n");
	}

	return CMD_SUCCESS;
}
#endif

/*使活动状态的PON板查在线ONU
**除第一个参数外，其余都没用*/
STATUS PON_Port_Send_Oam(ULONG Relay_type, ULONG ulArg2,
        ULONG ulArg3, ULONG ulArg4,
        ULONG ulArg5, ULONG ulArg6,
        ULONG ulArg7, ULONG ulArg8,
        ULONG ulArg9, ULONG ulArg10 )
{
	ULONG PonPortIdx;
    
	for(PonPortIdx =0; PonPortIdx < MAXPON; PonPortIdx ++ )
	{
		if( PonPortIsWorking(PonPortIdx) == TRUE )
			SEND_Access_Str_Oam( PonPortIdx, Relay_type );
	}
    
    Access_id_module_state = ACCESS_ID_MODULE_FREE;

    VOS_TaskExit(0);
    
    return VOS_OK;
}

/*查找所有在线的ONU，并调用相应函数发送OAM*/
STATUS SEND_Access_Str_Oam(ULONG PonPortIdx, ULONG Relay_type)
{
	ULONG OnuIdx;
	for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
	{
		if( (GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP ))
		{
			if ((Relay_type == RELAY_TYPE_PPPOE)||(Relay_type == RELAY_TYPE_DHCP))
			{
				onuIdx_Is_Support_Relay(PonPortIdx, OnuIdx, Relay_type);
			}
		}
	}
	return VOS_OK;
}

void ACCESS_id_CommandInstall(void)
{
#ifdef ONU_PPPOE_RELAY
	install_element ( CONFIG_NODE, &onu_pppoe_relay_disable_cmd);
	install_element ( CONFIG_NODE, &onu_pppoe_relay_enable_cmd);
	install_element ( CONFIG_NODE, &onu_pppoe_circuitid_set_cmd);
	install_element ( CONFIG_NODE, &undo_onu_pppoe_relay_circuitid_cmd);
	install_element ( CONFIG_NODE, &onu_pppoe_relay_show_cmd);
#endif
#ifdef ONU_DHCP_RELAY
	install_element (CONFIG_NODE, &onu_dhcp_relay_enable_cmd);
	install_element (CONFIG_NODE, &onu_dhcp_relay_mode_cmd);
	install_element (CONFIG_NODE, &onu_dhcp_relay_show_cmd);
#endif
}

LONG ACCESS_Id_Cli_Init(void)
{
	ACCESS_id_CommandInstall();
	return VOS_OK;
}

