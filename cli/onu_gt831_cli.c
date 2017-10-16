
#ifndef GT831_ONU_CLI
#define GT831_ONU_CLI

/* added by ZhengYT 2007-7-5 */

/*********  ONU GT831/GT821 命令行, 用于VOIP & CATV **************/

DEFUN(config_cmd2lic_mgcp_show_mgcp_config_func,
	config_cmd2lic_mgcp_show_mgcp_config_cmd,
	"show mgcp", 
	"Show running system MGCP information\n" 
	MGCP_STR
    /*"ONU_DEVICE_NODE\n"*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show mgcp");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show mgcp failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}



DEFUN(config_cmd2lic_mgcp_acl_config_func,
	config_cmd2lic_mgcp_acl_config_cmd,
	"mgcp acl enable", 
	MGCP_STR
	"MGCP ACL.\n" 
	"Enable MGCP ACL\n"
	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp acl enable");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp acl enable failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}




DEFUN(config_cmd2lic_mgcp_no_config_func,
    	config_cmd2lic_mgcp_no_config_cmd,
    	"undo mgcp [acl|authentication|heartbeat|ncs|persist-event|wildcard]",
	"Negate a command or set its defaults.\n"
    	MGCP_STR
	"MGCP ACL\n"
	"MGCP authenticate\n"
	"MGCP Heartbeat\n" 
	"MGCP NCS Protocol\n" 
	"MGCP Persist Event\n" 
	"MGCP Wildcard\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "undo mgcp");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%undo mgcp failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_authentication_mac_config_func,
    	config_cmd2lic_mgcp_authentication_mac_config_cmd,
    	"mgcp authentication mac", 
    	MGCP_STR
    	"MGCP Authentication.\n" 
    	"Use Physical Address as MG ID and Ki \n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp authentication mac");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp authentication mac failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_authentication_config_func,
    	config_cmd2lic_mgcp_authentication_config_cmd,
    	"mgcp authentication <mgid> <ki>", 
     	MGCP_STR
   	"MGCP Authentication.\n" 
   	"MG ID (2-16)\n"
   	"ki (1-16)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp authentication");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp authentication failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_heartbeat_enable_config_func,
    	config_cmd2lic_mgcp_heartbeat_enable_config_cmd,
    	"mgcp heartbeat enable", 
    	MGCP_STR
    	"MGCP Heartbeat.\n" 
    	"Enable MGCP Heartbeat\n"
    	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp heartbeat enable");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp heartbeat enable failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_heartbeat_timer_config_func,
    	config_cmd2lic_mgcp_heartbeat_timer_config_cmd,
    	"mgcp heartbeat timer <1-600>",
    	MGCP_STR
	"MGCP Heartbeat.\n" 
       "MGCP Heartbeat Timer\n" 
       "MGCP Heartbeat Timer (seconds 1-600)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp heartbeat timer");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp heartbeat timer failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_heartbeat_type_config_func,
    	config_cmd2lic_mgcp_heartbeat_type_config_cmd,
    	"mgcp heartbeat type [ntfy|rsip]",
    	MGCP_STR
	"MGCP Heartbeat.\n"
	"MGCP Heartbeat Type\n"
	"MGCP Heartbeat Type - NTFY\n"
	"MGCP Heartbeat Type - RSIP\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp heartbeat type");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp heartbeat type failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_mg_domainname_config_func,
    	config_cmd2lic_mgcp_mg_domainname_config_cmd,
    	"mgcp mg domain-name <domainname>", 
     	MGCP_STR
   	"Media Gateway.\n" 
   	"Domain Name\n" 
   	"Domain name (1-32)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp mg domain-name");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp mg domain-name failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_mg_localport_config_func,
    	config_cmd2lic_mgcp_mg_localport_config_cmd,
    	"mgcp mg local-name [1|2] <localname>",
    	MGCP_STR
	"Media Gateway.\n"
	"Local Name\n"
	"Endpoint Index is 1\n"
	"Endpoint Index is 2\n"
	"Local name (lenght:1-32)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp mg local-name");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp mg local-name failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_mg_localname_config_func,
    	config_cmd2lic_mgcp_mg_localname_config_cmd,
    	"mgcp mg port <1-65535>", 
    	MGCP_STR
    	"Media Gateway.\n"
    	"MGCP Port\n"
    	"Port Number (1-65535)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp mg port");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp mg port failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_mgc_addr_config_func,
    	config_cmd2lic_mgcp_mgc_addr_config_cmd,
    	"mgcp mgc address [primary|secondary] <A.B.C.D>",
    	MGCP_STR
	"Media Gateway Controller.\n"
	"MGC Address\n"
	"Primary MGC Address\n" 
	"Secondary MGC address\n"
	"MGC Address\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp mgc address");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp mgc address failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_mgc_port_config_func,
    	config_cmd2lic_mgcp_mgc_port_config_cmd,
    	"mgcp mgc port <1-65535>", 
    	MGCP_STR
    	"Media Gateway Controller.\n" 
    	"MGCP Port\n" 
    	"Port Number (1-65535)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp mgc port");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp mgc port failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_mgc_type_config_func,
    	config_cmd2lic_mgcp_mgc_type_config_cmd,
    	"mgcp mgc type [default|newtone|up5016]",
    	MGCP_STR
	"Media Gateway Controller.\n"
	"MGC Type\n" 
	"Default\n"
	"Newtone Call agent\n" 
	"Uptech 5016 TG\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp mgc type");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp mgc type failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_ncs_config_func,
    	config_cmd2lic_mgcp_ncs_config_cmd,
    	"mgcp ncs enable", 
    	MGCP_STR
    	"MGCP NCS Protocol.\n"
    	"Enable MGCP NCS Protocl\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp ncs enable");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp ncs enable failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_persist_event_config_func,
    	config_cmd2lic_mgcp_persist_event_config_cmd,
    	"mgcp persist-event enable", 
    	MGCP_STR
    	"MGCP Persist Event.\n"
    	"Enable Persist Event\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp persist-event enable");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp persist-event enable failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_quarantine_config_func,
    	config_cmd2lic_mgcp_quarantine_config_cmd,
    	"mgcp quarantine [loop|step]", 
     	MGCP_STR
   	"MGCP Quarantine\n"
   	"Loop Mode\n"
   	"Step Mode\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp quarantine");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp quarantine failed!\r\n");	
		return CMD_WARNING;
		}
   	 return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_wildcard_config_func,
    	config_cmd2lic_mgcp_wildcard_config_cmd,
    	"mgcp wildcard enable", 
    	MGCP_STR
    	"MGCP Wildcard.\n"
    	"Enable MGCP Wildcard\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgcp wildcard enable");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgcp wildcard enable failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_mgcp_regstatus_show,
	config_cmd2lic_mgcp_regstatus_show_cmd,
	"show mgcp regstatus",
	"Show running system information.\n" 
	MGCP_STR
	"Show mg registration status.\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show mgcp regstatus");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show mgcp regstatus failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

/* RTP Associated commands
**/
DEFUN(config_cmd2lic_rtp_show_rtp_config_func,
    	config_cmd2lic_rtp_show_rtp_config_cmd,
    	"show rtp", 
    	"Show running system information.\n"
    	"RTP configuration\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show rtp");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show rtp failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_rtp_start_port_config_func,
    	config_cmd2lic_rtp_start_port_config_cmd,
    	"rtp start-port <1-65535>",
    	"Set RTP configuration.\n"
    	"Set RTP start port configuration.\n" 
    	"Port number(1-65535)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "rtp start-port");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%rtp start-port failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

/* EPT Associated commands
**/
DEFUN(config_cmd2lic_ept_hookflash_func,
    	config_cmd2lic_ept_hookflash_cmd,
    	/*"ept hookflash <80-1300>", */		/* modified by xieshl 20090430 */
	"ept hookflash {min <80-1300> max <80-1300>}*1",
    	"Endpoint configuration\n"
    	"Set hookflash time\n"
    	"min time\n"
    	"min time value, unit:million seconds\n"
    	"max time\n"
    	"max time value, unit:million seconds\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "ept hookflash");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%ept hookflash failed!\r\n");	
		return CMD_WARNING;
		} 	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_ept_tone_test_func,
    	config_cmd2lic_ept_tone_test_cmd,
    	"debug ept-tone [<0-86>|list]", 
    	"Debug\n" 
    	"Debug EPT-TONE\n" 
    	"Tone number.\n"
    	"Tone list.\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "debug ept-tone");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%debug ept-tone failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

	/* modified by chenfj 2007-11-12
	问题单#5725: voice vad enable命令执行无效
	原命令格式为:voice vad [enalbe|disable]
	修改后，GT831上需要做同步修改
	*/
DEFUN(config_cmd2lic_ept_vad_enable,
	config_cmd2lic_ept_vad_enable_cmd, 
	"voice vad [enable|disable]", 
	"Voice module configuration\n" 
	"Voice Active Detect\n" 
	"Enable VAD\n"
	"Disable VAD\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice vad");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice vad failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}
	/* modified by chenfj 2007-11-12
	问题单#5728: voice dtmf-relay enable命令执行无效
	原命令格式为:voice dtmf-relay [enalbe|disable]
	修改后，GT831上需要做同步修改
	*/
DEFUN(ept_dtmf_relay_enable,
	config_cmd2lic_ept_dtmf_relay_enable_cmd, 
	"voice dtmf-relay [enable|disable]", 
	"Voice module configuration\n" 
	"Voice DTMP relay via telephone-event (rfc2833)\n" 
	"Enable dtmf relay\n"
	"Disable dtmf relay\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice dtmf-relay");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice dtmf-relay failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

	/* modified by chenfj 2007-11-12
	问题单#5726: voice ecan enable命令执行无效
	原命令格式为:voice ecan [enalbe|disable]
	修改后，GT831上需要做同步修改
	*/
DEFUN(ept_ecan_enable,
	config_cmd2lic_ept_ecan_enable_cmd, 
	"voice ecan [enable|disable]", 
	"Voice module configuration\n" 
	"Voice echo canceller \n" 
	"Enable dtmf relay\n"
	"Disable dtmf relay\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice ecan");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice ecan failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

DEFUN(ept_gain_control,
	config_cmd2lic_ept_gain_control_cmd, 
	"voice phone <1-2> [rx-gain|tx-gain] [m|p] [<0-18>|default]", 
	"Voice module configuration\n" 
	"Phone associated configuration \n" 
	"Please input the phone number\n"
	"Receive gain (heard)\n"
	"Transmit gain (speak)\n"
	"Minus value\n"
	"Positive value\n"
	"Gain in DB\n"
	"Gain default\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice phone");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice phone failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

DEFUN(ept_fax_mode,
	config_cmd2lic_ept_fax_mode_cmd, 
	"voice fax [t38|pass-through]", 
	"Voice module configuration\n" 
	"Fax mode \n" 
	"T.38 relay\n"
	"Pass through in band\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice fax");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice fax failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}


/* SIP Associated commands
**/
DEFUN(config_cmd2lic_show_sip_config_func,
    	config_cmd2lic_show_sip_config_cmd,
    	"show sip [phone1|phone2|phoneall]",
	"Show running system information\n"
	SIP_STR
	"Show SIP configuration for phone1\n"
	"Show SIP configuration for phone2\n" 
	"Show SIP configuration for both phone1 and phone2\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show sip failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_display_name_config_func,
    	config_cmd2lic_sip_display_name_config_cmd,
    	"sip [phone1|phone2|phoneall] displayname <name>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 config\n" 
	"User DisplayName config\n" 
	"Display Name (1-64)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip displayname failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_user_name_config_func,
    	config_cmd2lic_sip_user_name_config_cmd,
    	"sip [phone1|phone2|phoneall] username <name>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"UserName config\n"
	"User Name (1-64)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip username failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_password_config_func,
    	config_cmd2lic_sip_password_config_cmd,
    	"sip [phone1|phone2|phoneall] password <pass>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n" 
	"PassWord config\n" 
	"User passwords (1-64)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip password failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_account_config_func,
    	config_cmd2lic_sip_account_config_cmd,
    	"sip [phone1|phone2|phoneall] account <account>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n" 
	"User Account config\n"
	"User Account (1-64)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip account failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_reg_realm_config_func,
    	config_cmd2lic_sip_reg_realm_config_cmd,
    	"sip [phone1|phone2|phoneall] registar realm <realm>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Registar IP(Domain-Name)\n"
	"Register Realm config\n"
	"Register Realm LEN(1-64)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip registar realm failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_reg_port_config_func,
    	config_cmd2lic_sip_reg_port_config_cmd,
    	"sip [phone1|phone2|phoneall] registar port <1-65535>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Registar PORT\n"
	"Register Port config\n"
	"Register Port (1-65535)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip registar port failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_outbound_proxy_address_config_func,
    	config_cmd2lic_sip_outbound_proxy_address_config_cmd,
    	"sip [phone1|phone2|phoneall] outboundProxy address <adress>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Outbound Proxy!\n"
	"Config for Outbound Proxy IP(Domain-Name)\n"
	"Outbound Proxy address config\n" "Outbound Proxy Address LEN (1-64)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip outboundProxy address failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_outbound_proxy_port_config_func,
    	config_cmd2lic_sip_outbound_proxy_port_config_cmd,
    	"sip [phone1|phone2|phoneall] outboundProxy port <1-65535>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 config\n"
	"Config for Outbound Proxy PORT\n"
	"Outbound Proxy config\n"
	"Outbound Proxy Port (1-65535)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip outboundProxy port failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_server_address_config_func,
    	config_cmd2lic_sip_server_address_config_cmd,
    	"sip [phone1|phone2|phoneall] server address <address>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Server!\n"
	"Config for Server IP(Domain-Name)\n"
	"Server address config\n" "Server address LEN (1-64)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip server address failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_server_port_config_func,
    	config_cmd2lic_sip_server_port_config_cmd,
    	"sip [phone1|phone2|phoneall] server port <1-65535>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Server PORT\n" 
	"Server Port config\n" 
	"Server Port (1-65535)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %% sip server port failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_local_port_config_func,
    	config_cmd2lic_sip_local_port_config_cmd,
    	"sip [phone1|phone2|phoneall] local port <1-65535>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Local PORT\n" 
	"Local Port config\n" 
	"Local Port (1-65535)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip local port failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_expires_config_func,
    	config_cmd2lic_sip_expires_config_cmd,
    	"sip [phone1|phone2|phoneall] expires <60-3600>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Expires\n" 
	"Local Port (1-65535)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip expires failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}


LDEFUN(config_cmd2lic_mgcp_show_mgcp_config_func,
	config_cmd2lic_mgcp_show_mgcp_config_cmd,
	"show mgcp", 
	"Show running system MGCP information\n" 
	MGCP_STR,
	ONU_NODE
    /*"ONU_DEVICE_NODE\n"*/)
{
	return CMD_SUCCESS;
}



LDEFUN(config_cmd2lic_mgcp_acl_config_func,
	config_cmd2lic_mgcp_acl_config_cmd,
	"mgcp acl enable", 
	MGCP_STR
	"MGCP ACL.\n" 
	"Enable MGCP ACL\n",
	ONU_NODE
	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}




LDEFUN(config_cmd2lic_mgcp_no_config_func,
    	config_cmd2lic_mgcp_no_config_cmd,
    	"undo mgcp [acl|authentication|heartbeat|ncs|persist-event|wildcard]",
	"Negate a command or set its defaults.\n"
    	MGCP_STR
	"MGCP ACL\n"
	"MGCP authenticate\n"
	"MGCP Heartbeat\n" 
	"MGCP NCS Protocol\n" 
	"MGCP Persist Event\n" 
	"MGCP Wildcard\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_authentication_mac_config_func,
    	config_cmd2lic_mgcp_authentication_mac_config_cmd,
    	"mgcp authentication mac", 
    	MGCP_STR
    	"MGCP Authentication.\n" 
    	"Use Physical Address as MG ID and Ki \n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_authentication_config_func,
    	config_cmd2lic_mgcp_authentication_config_cmd,
    	"mgcp authentication <mgid> <ki>", 
     	MGCP_STR
   	"MGCP Authentication.\n" 
   	"MG ID (2-16)\n"
   	"ki (1-16)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_heartbeat_enable_config_func,
    	config_cmd2lic_mgcp_heartbeat_enable_config_cmd,
    	"mgcp heartbeat enable", 
    	MGCP_STR
    	"MGCP Heartbeat.\n" 
    	"Enable MGCP Heartbeat\n",
	ONU_NODE
    	)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_heartbeat_timer_config_func,
    	config_cmd2lic_mgcp_heartbeat_timer_config_cmd,
    	"mgcp heartbeat timer <1-600>",
    	MGCP_STR
	"MGCP Heartbeat.\n" 
       "MGCP Heartbeat Timer\n" 
       "MGCP Heartbeat Timer (seconds 1-600)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_heartbeat_type_config_func,
    	config_cmd2lic_mgcp_heartbeat_type_config_cmd,
    	"mgcp heartbeat type [ntfy|rsip]",
    	MGCP_STR
	"MGCP Heartbeat.\n"
	"MGCP Heartbeat Type\n"
	"MGCP Heartbeat Type - NTFY\n"
	"MGCP Heartbeat Type - RSIP\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_mg_domainname_config_func,
    	config_cmd2lic_mgcp_mg_domainname_config_cmd,
    	"mgcp mg domain-name <domainname>", 
     	MGCP_STR
   	"Media Gateway.\n" 
   	"Domain Name\n" 
   	"Domain name (1-32)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_mg_localport_config_func,
    	config_cmd2lic_mgcp_mg_localport_config_cmd,
    	"mgcp mg local-name [1|2] <localname>",
    	MGCP_STR
	"Media Gateway.\n"
	"Local Name\n"
	"Endpoint Index is 1\n"
	"Endpoint Index is 2\n"
	"Local name (lenght:1-32)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_mg_localname_config_func,
    	config_cmd2lic_mgcp_mg_localname_config_cmd,
    	"mgcp mg port <1-65535>", 
    	MGCP_STR
    	"Media Gateway.\n"
    	"MGCP Port\n"
    	"Port Number (1-65535)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_mgc_addr_config_func,
    	config_cmd2lic_mgcp_mgc_addr_config_cmd,
    	"mgcp mgc address [primary|secondary] <A.B.C.D>",
    	MGCP_STR
	"Media Gateway Controller.\n"
	"MGC Address\n"
	"Primary MGC Address\n" 
	"Secondary MGC address\n"
	"MGC Address\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_mgc_port_config_func,
    	config_cmd2lic_mgcp_mgc_port_config_cmd,
    	"mgcp mgc port <1-65535>", 
    	MGCP_STR
    	"Media Gateway Controller.\n" 
    	"MGCP Port\n" 
    	"Port Number (1-65535)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_mgc_type_config_func,
    	config_cmd2lic_mgcp_mgc_type_config_cmd,
    	"mgcp mgc type [default|newtone|up5016]",
    	MGCP_STR
	"Media Gateway Controller.\n"
	"MGC Type\n" 
	"Default\n"
	"Newtone Call agent\n" 
	"Uptech 5016 TG\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_ncs_config_func,
    	config_cmd2lic_mgcp_ncs_config_cmd,
    	"mgcp ncs enable", 
    	MGCP_STR
    	"MGCP NCS Protocol.\n"
    	"Enable MGCP NCS Protocl\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_persist_event_config_func,
    	config_cmd2lic_mgcp_persist_event_config_cmd,
    	"mgcp persist-event enable", 
    	MGCP_STR
    	"MGCP Persist Event.\n"
    	"Enable Persist Event\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_quarantine_config_func,
    	config_cmd2lic_mgcp_quarantine_config_cmd,
    	"mgcp quarantine [loop|step]", 
     	MGCP_STR
   	"MGCP Quarantine\n"
   	"Loop Mode\n"
   	"Step Mode\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_wildcard_config_func,
    	config_cmd2lic_mgcp_wildcard_config_cmd,
    	"mgcp wildcard enable", 
    	MGCP_STR
    	"MGCP Wildcard.\n"
    	"Enable MGCP Wildcard\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_mgcp_regstatus_show,
	config_cmd2lic_mgcp_regstatus_show_cmd,
	"show mgcp regstatus",
	"Show running system information.\n" 
	MGCP_STR
	"Show mg registration status.\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

/* RTP Associated commands
**/
LDEFUN(config_cmd2lic_rtp_show_rtp_config_func,
    	config_cmd2lic_rtp_show_rtp_config_cmd,
    	"show rtp", 
    	"Show running system information.\n"
    	"RTP configuration\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_rtp_start_port_config_func,
    	config_cmd2lic_rtp_start_port_config_cmd,
    	"rtp start-port <1-65535>",
    	"Set RTP configuration.\n"
    	"Set RTP start port configuration.\n" 
    	"Port number(1-65535)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

/* EPT Associated commands
**/
LDEFUN(config_cmd2lic_ept_hookflash_func,
    	config_cmd2lic_ept_hookflash_cmd,
    	/*"ept hookflash <80-1300>", */		/* modified by xieshl 20090430 */
	"ept hookflash {min <80-1300> max <80-1300>}*1",
    	"Endpoint configuration\n"
    	"Set hookflash time\n"
    	"min time\n"
    	"min time value, unit:million seconds\n"
    	"max time\n"
    	"max time value, unit:million seconds\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_ept_tone_test_func,
    	config_cmd2lic_ept_tone_test_cmd,
    	"debug ept-tone [<0-86>|list]", 
    	"Debug\n" 
    	"Debug EPT-TONE\n" 
    	"Tone number.\n"
    	"Tone list.\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

	/* modified by chenfj 2007-11-12
	问题单#5725: voice vad enable命令执行无效
	原命令格式为:voice vad [enalbe|disable]
	修改后，GT831上需要做同步修改
	*/
LDEFUN(config_cmd2lic_ept_vad_enable,
	config_cmd2lic_ept_vad_enable_cmd, 
	"voice vad [enable|disable]", 
	"Voice module configuration\n" 
	"Voice Active Detect\n" 
	"Enable VAD\n"
	"Disable VAD\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}
	/* modified by chenfj 2007-11-12
	问题单#5728: voice dtmf-relay enable命令执行无效
	原命令格式为:voice dtmf-relay [enalbe|disable]
	修改后，GT831上需要做同步修改
	*/
LDEFUN(ept_dtmf_relay_enable,
	config_cmd2lic_ept_dtmf_relay_enable_cmd, 
	"voice dtmf-relay [enable|disable]", 
	"Voice module configuration\n" 
	"Voice DTMP relay via telephone-event (rfc2833)\n" 
	"Enable dtmf relay\n"
	"Disable dtmf relay\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

	/* modified by chenfj 2007-11-12
	问题单#5726: voice ecan enable命令执行无效
	原命令格式为:voice ecan [enalbe|disable]
	修改后，GT831上需要做同步修改
	*/
LDEFUN(ept_ecan_enable,
	config_cmd2lic_ept_ecan_enable_cmd, 
	"voice ecan [enable|disable]", 
	"Voice module configuration\n" 
	"Voice echo canceller \n" 
	"Enable dtmf relay\n"
	"Disable dtmf relay\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(ept_gain_control,
	config_cmd2lic_ept_gain_control_cmd, 
	"voice phone <1-2> [rx-gain|tx-gain] [m|p] [<0-18>|default]", 
	"Voice module configuration\n" 
	"Phone associated configuration \n" 
	"Please input the phone number\n"
	"Receive gain (heard)\n"
	"Transmit gain (speak)\n"
	"Minus value\n"
	"Positive value\n"
	"Gain in DB\n"
	"Gain default\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}
LDEFUN(ept_fax_mode,
	config_cmd2lic_ept_fax_mode_cmd, 
	"voice fax [t38|pass-through]", 
	"Voice module configuration\n" 
	"Fax mode \n" 
	"T.38 relay\n"
	"Pass through in band\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}


/* SIP Associated commands
**/
LDEFUN(config_cmd2lic_show_sip_config_func,
    	config_cmd2lic_show_sip_config_cmd,
    	"show sip [phone1|phone2|phoneall]",
	"Show running system information\n"
	SIP_STR
	"Show SIP configuration for phone1\n"
	"Show SIP configuration for phone2\n" 
	"Show SIP configuration for both phone1 and phone2\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_display_name_config_func,
    	config_cmd2lic_sip_display_name_config_cmd,
    	"sip [phone1|phone2|phoneall] displayname <name>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 config\n" 
	"User DisplayName config\n" 
	"Display Name (1-64)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_user_name_config_func,
    	config_cmd2lic_sip_user_name_config_cmd,
    	"sip [phone1|phone2|phoneall] username <name>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"UserName config\n"
	"User Name (1-64)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_password_config_func,
    	config_cmd2lic_sip_password_config_cmd,
    	"sip [phone1|phone2|phoneall] password <pass>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n" 
	"PassWord config\n" 
	"User passwords (1-64)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_account_config_func,
    	config_cmd2lic_sip_account_config_cmd,
    	"sip [phone1|phone2|phoneall] account <account>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n" 
	"User Account config\n"
	"User Account (1-64)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_reg_realm_config_func,
    	config_cmd2lic_sip_reg_realm_config_cmd,
    	"sip [phone1|phone2|phoneall] registar realm <realm>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Registar IP(Domain-Name)\n"
	"Register Realm config\n"
	"Register Realm LEN(1-64)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_reg_port_config_func,
    	config_cmd2lic_sip_reg_port_config_cmd,
    	"sip [phone1|phone2|phoneall] registar port <1-65535>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Registar PORT\n"
	"Register Port config\n"
	"Register Port (1-65535)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_outbound_proxy_address_config_func,
    	config_cmd2lic_sip_outbound_proxy_address_config_cmd,
    	"sip [phone1|phone2|phoneall] outboundProxy address <adress>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Outbound Proxy!\n"
	"Config for Outbound Proxy IP(Domain-Name)\n"
	"Outbound Proxy address config\n" "Outbound Proxy Address LEN (1-64)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_outbound_proxy_port_config_func,
    	config_cmd2lic_sip_outbound_proxy_port_config_cmd,
    	"sip [phone1|phone2|phoneall] outboundProxy port <1-65535>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 config\n"
	"Config for Outbound Proxy PORT\n"
	"Outbound Proxy config\n"
	"Outbound Proxy Port (1-65535)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
    return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_server_address_config_func,
    	config_cmd2lic_sip_server_address_config_cmd,
    	"sip [phone1|phone2|phoneall] server address <address>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Server!\n"
	"Config for Server IP(Domain-Name)\n"
	"Server address config\n" "Server address LEN (1-64)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
    return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_server_port_config_func,
    	config_cmd2lic_sip_server_port_config_cmd,
    	"sip [phone1|phone2|phoneall] server port <1-65535>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Server PORT\n" 
	"Server Port config\n" 
	"Server Port (1-65535)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
    return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_local_port_config_func,
    	config_cmd2lic_sip_local_port_config_cmd,
    	"sip [phone1|phone2|phoneall] local port <1-65535>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Local PORT\n" 
	"Local Port config\n" 
	"Local Port (1-65535)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
    return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_expires_config_func,
    	config_cmd2lic_sip_expires_config_cmd,
    	"sip [phone1|phone2|phoneall] expires <60-3600>",
	SIP_STR
	"SIP phone1 Config\n"
	"SIP phone2 Config\n"
	"SIP both phone1 and phone2 Config\n"
	"Config for Expires\n" 
	"Local Port (1-65535)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
    return CMD_SUCCESS;
}



DEFUN(config_cmd2lic_show_sip_phone_reg_status_func,
    	config_cmd2lic_show_sip_phone_reg_status_cmd,
    	"show sip regstatus",
	"Show running system information.\n" 
	SIP_STR
	"Show SIP phone registration status.\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show sip regstatus");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show sip regstatus failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_dial_plans_func,
    	config_cmd2lic_sip_dial_plan_cmd,
    	"sip dial-plan <dialplan>", 
    	"Dial plan configuration\n" 
	SIP_STR
    	"Dial plan string(1-20)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip dial-plan");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip dial-plan failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_no_dial_plan_func,
    	config_cmd2lic_sip_no_dial_plan_cmd,
    	"undo sip dial-plan", 
    	" Negate a command or set its defaults.\n" 
	SIP_STR
    	"Dial plan configuration.\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "undo sip dial-plan");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%undo sip dial-plan failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_user_agent_config_func,
    	config_cmd2lic_sip_user_agent_config_cmd,
    	"sip user-agent <string>", 
	SIP_STR
    	"SIP user-agent config\n"
    	"user-agent string(1-64)\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "sip user-agent");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%sip user-agent failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_show_dial_plan_func,
    	config_cmd2lic_sip_show_dial_plan_cmd,
    	"show sip dial-plan", 
    	"Show running system information.\n" 
	SIP_STR
    	"Dial plan configuration.\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show sip dial-plan");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show sip dial-plan failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_show_dialogs_func,
    	config_cmd2lic_sip_show_dialogs_cmd,
    	"show sip dialog",
	"Show running system information.\n" 
	SIP_STR
	"Dialog information.\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show sip dialog");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show sip dialog failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_show_transactions_func,
    	config_cmd2lic_sip_show_transactions_cmd,
    	"show sip transaction [ict|nict|ist|nist|all]",
	"Show running system information.\n"
	SIP_STR
	"Transactions information.\n"
	"ICT transaction information.\n"
	"NICT transaction information.\n"
	"IST transaction information.\n"
	"NIST transaction information.\n" 
	"All transaction information.\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show sip transaction");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show sip transaction failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_iad_service_select,
	config_cmd2lic_iad_service_select_cmd,
	"[mgcp|sip|h323|h248] enable",
	MGCP_STR
	SIP_STR 
	H323_STR 
	H248_STR 
	"Start up service\n"
	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice service_select");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%[mgcp|sip|h323|h248] enable failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_iad_show_current_protocol,
	config_cmd2lic_iad_show_current_protocol_cmd,
	"show voice signal-protocol",
	"Show running system information\n" 
	"Voice module information\n"
	"Show active voip protocol running at current\n"
	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice signal-protocol");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice signal-protocol failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

/* DNS Associated commands
**/
DEFUN(config_cmd2lic_cli_config_voice_dns_server_func,
    	config_cmd2lic_cli_config_voice_dns_server_cmd,
    	"voice dns server add <A.B.C.D> {[primary]}*1",
       "Config voice module information\n"
	"Config dns information\n"
	"Config dns server information\n"
	"Config dns add server information\n"
	"Please input dns server's ip\n"
	"Added to dns server table as primary server\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice dns server add");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%add voice dns server failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_cli_config_voice_dns_server_del_func,
    	config_cmd2lic_cli_config_voice_dns_server_del_cmd,
    	"voice dns server del {<A.B.C.D>}*1",
       "Config voice module information\n"
	"Config dns information\n"
	"Config dns server information\n"
	"Config dns delete server information\n"
	"Please input dns server's ip\n"
	"Added to dns server table as primary server\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice dns server del");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%del voice dns server del failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_serv_voice_dns_enable_func,
      config_cmd2lic_serv_voice_dns_enable_cmd,
      "voice dns [enable|disable]",
       "Config voice module information\n"
      "Config voice-dns service\n"
      "Set dns service to up\n"
      "Set dns service to down\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice dns");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice dns failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_cli_show_voice_dns_client_info_func,
      config_cmd2lic_cli_show_voice_dns_client_info_cmd,
      "show voice dns info",
	"Show system information\n"
	"Voice module information\n"
	"Show voice-dns information\n"
	"Show voice-dns information\n"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice dns info");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice dns info failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

/* IP Associated commands
**/
DEFUN(config_cmd2lic_IpAddr_voice_if_dhcpc_func,
      config_cmd2lic_IpAddr_voice_if_dhcpc_cmd,
      "voice ip address dhcp",
       "Config voice module information\n"
      IP_STR
      IPADDR_STR
      "IP address from dhcp server"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice ip address dhcp");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice ip address dhcp failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_NoIpAddr_voice_if_dhcpc_func,
      config_cmd2lic_NoIpAddr_voice_if_dhcpc_cmd,
      "undo voice ip address dhcp" ,
        NO_STR
        "Config voice module information\n"
        IP_STR
        IPADDR_STR
        "IP address from dhcp server"
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "undo voice ip address dhcp");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%undo voice ip address dhcp failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_IpAddr_voice_if_func,
      config_cmd2lic_IpAddr_voice_if_cmd,
      "voice ip address <A.B.C.D/M>" ,
       "Config voice module information\n"
        IP_STR
        IPADDR_STR
        ABCDM_STR
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice ip address");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice ip address failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_IpAddrMask_voice_if_func,
      config_cmd2lic_IpAddrMask_voice_if_cmd,
      "voice ip address <A.B.C.D> <A.B.C.D>" ,
       "Config voice module information\n"
        IP_STR
        IPADDR_STR
        ABCD_STR
        MASK_STR
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice ip address");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice ip address failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_NoIpAddr_voice_if_func,
      config_cmd2lic_NoIpAddr_voice_if_cmd,
      "undo voice ip address {<A.B.C.D>}*1" ,
        NO_STR
       "Config voice module information\n"
        IP_STR
        IPADDR_STR
        ABCD_STR
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "undo voice ip address");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%undo voice ip address failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_IpAddr_voice_ip_show_func,
      config_cmd2lic_IpAddr_voice_ip_show_cmd,
      "show voice ip address" ,
        SHOW_STR
	 "Voice module information\n"
        IP_STR
        IPADDR_STR
        ABCD_STR 
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice ip address");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice ip address failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

/* IP-GATEWAY Associated commands
**/
DEFUN(config_cmd2lic_show_vocie_ip_gateway_func,
      config_cmd2lic_show_vocie_ip_gateway_cmd,
      "show voice ip gateway",
       SHOW_STR
       "Voice module information\n"
       IP_STR
       "IP gateway\n" 
    	/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice ip gateway");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice ip gateway failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_static_voice_ipgateway_func,
      config_cmd2lic_static_voice_ipgateway_cmd,
      "voice ip gateway <A.B.C.D>",
       "Config voice module information\n"
          IP_STR
          "Establish static gateway routes\n"
          "IP gateway address\n"
    	   /*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice ip gateway");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice ip gateway failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_static_no_voice_ipgateway_func,
      config_cmd2lic_static_no_voice_ipgateway_cmd,
      "undo voice ip gateway <A.B.C.D>",
        NO_STR
       "Config voice module information\n"
          IP_STR
          "Establish static gateway routes\n"
          "IP gateway address\n"
    	   /*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "undo voice ip gateway");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%undo voice ip gateway failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}


LDEFUN(config_cmd2lic_show_sip_phone_reg_status_func,
    	config_cmd2lic_show_sip_phone_reg_status_cmd,
    	"show sip regstatus",
	"Show running system information.\n" 
	SIP_STR
	"Show SIP phone registration status.\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_dial_plans_func,
    	config_cmd2lic_sip_dial_plan_cmd,
    	"sip dial-plan <dialplan>", 
    	"Dial plan configuration\n" 
	SIP_STR
    	"Dial plan string(1-20)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_no_dial_plan_func,
    	config_cmd2lic_sip_no_dial_plan_cmd,
    	"undo sip dial-plan", 
    	" Negate a command or set its defaults.\n" 
	SIP_STR
    	"Dial plan configuration.\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_user_agent_config_func,
    	config_cmd2lic_sip_user_agent_config_cmd,
    	"sip user-agent <string>", 
	SIP_STR
    	"SIP user-agent config\n"
    	"user-agent string(1-64)\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_show_dial_plan_func,
    	config_cmd2lic_sip_show_dial_plan_cmd,
    	"show sip dial-plan", 
    	"Show running system information.\n" 
	SIP_STR
    	"Dial plan configuration.\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_show_dialogs_func,
    	config_cmd2lic_sip_show_dialogs_cmd,
    	"show sip dialog",
	"Show running system information.\n" 
	SIP_STR
	"Dialog information.\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_show_transactions_func,
    	config_cmd2lic_sip_show_transactions_cmd,
    	"show sip transaction [ict|nict|ist|nist|all]",
	"Show running system information.\n"
	SIP_STR
	"Transactions information.\n"
	"ICT transaction information.\n"
	"NICT transaction information.\n"
	"IST transaction information.\n"
	"NIST transaction information.\n" 
	"All transaction information.\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_iad_service_select,
	config_cmd2lic_iad_service_select_cmd,
	"[mgcp|sip|h323|h248] enable",
	MGCP_STR
	SIP_STR 
	H323_STR 
	H248_STR 
	"Start up service\n",
	ONU_NODE
	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_iad_show_current_protocol,
	config_cmd2lic_iad_show_current_protocol_cmd,
	"show voice signal-protocol",
	"Show running system information\n" 
	"Voice module information\n"
	"Show active voip protocol running at current\n",
	ONU_NODE
	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

/* DNS Associated commands
**/
LDEFUN(config_cmd2lic_cli_config_voice_dns_server_func,
    	config_cmd2lic_cli_config_voice_dns_server_cmd,
    	"voice dns server add <A.B.C.D> {[primary]}*1",
       "Config voice module information\n"
	"Config dns information\n"
	"Config dns server information\n"
	"Config dns add server information\n"
	"Please input dns server's ip\n"
	"Added to dns server table as primary server\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_cli_config_voice_dns_server_del_func,
    	config_cmd2lic_cli_config_voice_dns_server_del_cmd,
    	"voice dns server del {<A.B.C.D>}*1",
       "Config voice module information\n"
	"Config dns information\n"
	"Config dns server information\n"
	"Config dns delete server information\n"
	"Please input dns server's ip\n"
	"Added to dns server table as primary server\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_serv_voice_dns_enable_func,
      config_cmd2lic_serv_voice_dns_enable_cmd,
      "voice dns [enable|disable]",
       "Config voice module information\n"
      "Config voice-dns service\n"
      "Set dns service to up\n"
      "Set dns service to down\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_cli_show_voice_dns_client_info_func,
      config_cmd2lic_cli_show_voice_dns_client_info_cmd,
      "show voice dns info",
	"Show system information\n"
	"Voice module information\n"
	"Show voice-dns information\n"
	"Show voice-dns information\n",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

/* IP Associated commands
**/
LDEFUN(config_cmd2lic_IpAddr_voice_if_dhcpc_func,
      config_cmd2lic_IpAddr_voice_if_dhcpc_cmd,
      "voice ip address dhcp",
       "Config voice module information\n"
      IP_STR
      IPADDR_STR
      "IP address from dhcp server",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_NoIpAddr_voice_if_dhcpc_func,
      config_cmd2lic_NoIpAddr_voice_if_dhcpc_cmd,
      "undo voice ip address dhcp" ,
        NO_STR
        "Config voice module information\n"
        IP_STR
        IPADDR_STR
        "IP address from dhcp server",
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_IpAddr_voice_if_func,
      config_cmd2lic_IpAddr_voice_if_cmd,
      "voice ip address <A.B.C.D/M>" ,
       "Config voice module information\n"
        IP_STR
        IPADDR_STR
        ABCDM_STR,
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_IpAddrMask_voice_if_func,
      config_cmd2lic_IpAddrMask_voice_if_cmd,
      "voice ip address <A.B.C.D> <A.B.C.D>" ,
       "Config voice module information\n"
        IP_STR
        IPADDR_STR
        ABCD_STR
        MASK_STR,
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_NoIpAddr_voice_if_func,
      config_cmd2lic_NoIpAddr_voice_if_cmd,
      "undo voice ip address {<A.B.C.D>}*1" ,
        NO_STR
       "Config voice module information\n"
        IP_STR
        IPADDR_STR
        ABCD_STR,
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_IpAddr_voice_ip_show_func,
      config_cmd2lic_IpAddr_voice_ip_show_cmd,
      "show voice ip address" ,
        SHOW_STR
	 "Voice module information\n"
        IP_STR
        IPADDR_STR
        ABCD_STR ,
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

/* IP-GATEWAY Associated commands
**/
LDEFUN(config_cmd2lic_show_vocie_ip_gateway_func,
      config_cmd2lic_show_vocie_ip_gateway_cmd,
      "show voice ip gateway",
       SHOW_STR
       "Voice module information\n"
       IP_STR
       "IP gateway\n" ,
	ONU_NODE
    	/*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_static_voice_ipgateway_func,
      config_cmd2lic_static_voice_ipgateway_cmd,
      "voice ip gateway <A.B.C.D>",
       "Config voice module information\n"
          IP_STR
          "Establish static gateway routes\n"
          "IP gateway address\n",
	ONU_NODE
    	   /*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_static_no_voice_ipgateway_func,
      config_cmd2lic_static_no_voice_ipgateway_cmd,
      "undo voice ip gateway <A.B.C.D>",
        NO_STR
       "Config voice module information\n"
          IP_STR
          "Establish static gateway routes\n"
          "IP gateway address\n",
	ONU_NODE
    	   /*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}


#if 0
/* H248 Commands */
DEFUN(config_cmd2lic_h248_local_port_config,
			config_cmd2lic_h248_local_port_config_cmd,
			"h248 local port <1-65535>",
			H248_STR
			"local gateway config\n"
			"local gateway port config\n"
			"local gateway port number<1-65535>\n"
			/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h248 local port");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h248 local port failed!\r\n");	
		return CMD_WARNING;
		}
	return CMD_SUCCESS;
#endif
}

DEFUN( config_cmd2lic_h248_mgc_port_config,
			config_cmd2lic_h248_mgc_port_config_cmd,
			"h248 mgc port <1-65535>",
			H248_STR
			"Media gateway controller config\n"
			"Media gateway controller port config\n"
			"Media gateway controller port number<1-65535>\n"
			/*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h248 mgc port");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h248 mgc port failed!\r\n");	
		return CMD_WARNING;
		}
	return CMD_SUCCESS;
#endif
}

DEFUN( config_cmd2lic_h248_mgc_address_config,
			config_cmd2lic_h248_mgc_address_config_cmd,
			 "h248 mgc address <address>",
			 H248_STR
			 "Media gateway controller config\n"
			 "Media gateway controller address config\n"
			 "Media gateway controller ip address (xx.xx.xx.xx)\n"
			 /*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h248 mgc address");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h248 mgc address failed!\r\n");	
		return CMD_WARNING;
		}
	return CMD_SUCCESS;
#endif
}


DEFUN( config_cmd2lic_h248_router_address_config,
			 config_cmd2lic_h248_router_address_config_cmd,
			 "h248 router address <address>",
			 H248_STR
			 "Gateway router config\n"
			 "Gateway router address config\n"
			 "Geteway router ip address(xx.xx.xx.xx)\n"
			 /*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h248 router address");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h248 router address failed!\r\n");	
		return CMD_WARNING;
		}
	return CMD_SUCCESS;
#endif
}

DEFUN( config_cmd2lic_h248_phone_ext_number_config,
			 config_cmd2lic_h248_phone_ext_number_config_cmd,
			 "h248 phoneextnumber <1-65535>",
			 H248_STR
			 "Phone ext number config\n"
			 "PHone ext number<1-65535>\n"
			 /*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h248 phoneextnumber");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h248 phoneextnumber failed!\r\n");	
		return CMD_WARNING;
		}
	return CMD_SUCCESS;
#endif
}
#endif
/* H323 Commands */

/* Complementary Service Commands */

/* Media Commands */

/* Debug Commands */
DEFUN ( config_cmd2lic_voice_task_stack_func,
        config_cmd2lic_voice_task_stack_cmd,
        "show voice task_stack <taskname>",
        SHOW_STR
        "Voice module information\n"
        "Voice module task stack\n"
        "Task name\n"
    	 /*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice task_stack");
#else
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	LONG lRet;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice task_stack failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_slab_show,
      config_cmd2lic_slab_show_cmd,
      "show voice slab mod {<modname>}*1",
      SHOW_STR
      "Voice module information\n"
      "Show information of system memory cache\n"
      "Module memory cache information\n"
      "Specify the name of the mod\n"
      /*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice slab mod");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice slab mod failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_slab_mod_show,
      config_cmd2lic_slab_mod_show_cmd,
      "show voice slab global",
      SHOW_STR
      "Voice module task stack\n"
      "Show information of system memory cache\n"
      "Global memory cache information\n"
      /*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice slab global");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice slab global failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_vos_system_usage_show,
      config_cmd2lic_vos_system_usage_show_cmd,
      "show voice system resource",
      SHOW_STR
      "Voice module information\n"
       "Show information of system resource usage\n"
       "Show information of system resource usage\n"
       /*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice system resource");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice system resource failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN ( config_cmd2lic_voice_task_list_func,
        config_cmd2lic_voice_task_list_cmd,
        "show voice task",
        SHOW_STR
        "Voice module information\n"
        "Task information\n" 
        /*ONU_DEVICE_NODE*/)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice task");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice task failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

/*
** ONU模块自身的调试命令
*/
DEFUN ( task_list_func_voip,
        task_list_cmd_voip,
        "show task",
        SHOW_STR
        "Task information\n" 
       /* CONFIG_NODE*/)
 {
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show task");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show task failed!\r\n");	
		return CMD_WARNING;
		}
    return VOS_OK;
#endif
}

DEFUN ( config_cmd2lic_task_stack_func,
        config_cmd2lic_task_stack_cmd,
        "show task_stack <taskname>",
        SHOW_STR
        "Task stack infomation\n"
        "Task name\n"
    	 /*ONU_DEVICE_NODE*/)
  {
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show task_stack");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show task_stack failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

/*
GT831单独的CATV命令：mgt catv [1|0]   1:enable, 0:disable
*/

DEFUN ( config_catv,
        config_catv_cmd,
        "mgt catv [1|0]",
        "config catv\n"
        "config catv\n"
        "1:enable\n"
        "0:diable\n"
    	 /*ONU_DEVICE_NODE*/)
  {
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgt catv");
#else
	LONG lRet;
	CHAR pBuff[256];   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK_GT831

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgt catv failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}


DEFUN ( show_voice_info,
        show_voice_info_cmd,
        "mgt voice_show",
        "show voice info\n"
        "show voice info\n"
 
    	 /*ONU_DEVICE_NODE*/)
  {
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "mgt voice_show");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%mgt voice_show failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}


LDEFUN ( config_cmd2lic_voice_task_stack_func,
        config_cmd2lic_voice_task_stack_cmd,
        "show voice task_stack <taskname>",
        SHOW_STR
        "Voice module information\n"
        "Voice module task stack\n"
        "Task name\n",
 	 ONU_NODE
    	 /*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_slab_show,
      config_cmd2lic_slab_show_cmd,
      "show voice slab mod {<modname>}*1",
      SHOW_STR
      "Voice module information\n"
      "Show information of system memory cache\n"
      "Module memory cache information\n"
      "Specify the name of the mod\n",
 	 ONU_NODE
      /*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_slab_mod_show,
      config_cmd2lic_slab_mod_show_cmd,
      "show voice slab global",
      SHOW_STR
      "Voice module task stack\n"
      "Show information of system memory cache\n"
      "Global memory cache information\n",
 	 ONU_NODE
      /*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_vos_system_usage_show,
      config_cmd2lic_vos_system_usage_show_cmd,
      "show voice system resource",
      SHOW_STR
      "Voice module information\n"
       "Show information of system resource usage\n"
       "Show information of system resource usage\n",
 	 ONU_NODE
       /*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

LDEFUN ( config_cmd2lic_voice_task_list_func,
        config_cmd2lic_voice_task_list_cmd,
        "show voice task",
        SHOW_STR
        "Voice module information\n"
        "Task information\n" ,
 	 ONU_NODE
        /*ONU_DEVICE_NODE*/)
{
	return CMD_SUCCESS;
}

/*
** ONU模块自身的调试命令
*/
LDEFUN ( task_list_func_voip,
        task_list_cmd_voip,
        "show task",
        SHOW_STR
        "Task information\n" ,
 	 ONU_NODE
       /* CONFIG_NODE*/)
 {
	return CMD_SUCCESS;
}

LDEFUN ( config_cmd2lic_task_stack_func,
        config_cmd2lic_task_stack_cmd,
        "show task_stack <taskname>",
        SHOW_STR
        "Task stack infomation\n"
        "Task name\n",
 	 ONU_NODE
    	 /*ONU_DEVICE_NODE*/)
  {
	return CMD_SUCCESS;
}

/*
GT831单独的CATV命令：mgt catv [1|0]   1:enable, 0:disable
*/

LDEFUN ( config_catv,
        config_catv_cmd,
        "mgt catv [1|0]",
        "config catv\n"
        "config catv\n"
        "1:enable\n"
        "0:diable\n",
 	 ONU_NODE
    	 /*ONU_DEVICE_NODE*/)
  {
	return CMD_SUCCESS;
}


LDEFUN ( show_voice_info,
        show_voice_info_cmd,
        "mgt voice_show",
        "show voice info\n"
        "show voice info\n",
 	 ONU_NODE
    	 /*ONU_DEVICE_NODE*/)
  {
	return CMD_SUCCESS;
}



#if 0
DEFUN ( config_igmpsnoop_auth,
        onu_config_igmpsnoop_auth_cmd,
        "igmpsnooping auth enable {[1|0]}*1",
        "config igmp snooping authentication\n"
        "config igmp snooping authentication\n"
        "config igmp snooping authentication\n"
        "1:enable\n"
        "0:diable\n"
    	 /*ONU_DEVICE_NODE*/)
  {
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "igmpsnooping auth enable");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%igmpsnooping auth enable failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}
#endif

DEFUN(onu_slab_show,
      onu_slab_show_cmd,
      "show slab mod {<modname>}*1",
      SHOW_STR
      "Show information of system memory cache\n"
      "Module memory cache information\n"
      "Specify the name of the mod\n")

{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show slab mod");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show slab mod failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
} 

DEFUN(onu_voice_slab_mod_show,
      onu_voice_slab_mod_show_cmd,
      "show slab global",
      SHOW_STR
      "Show information of system memory cache\n"
      "Global memory cache information\n")

{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show slab global");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show slab global failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}


DEFUN(config_cmd2lic_ept_show,
	config_cmd2lic_ept_show_cmd,                     
	"show voice [vad|dtmf-relay|ecan|gain|fax]", 
	SHOW_STR
	"Voice module configuration\n"
	"Voice activity detection\n" 
	"DTMF relay mode (via rfc2833 or in band)\n" 
	"Echo canceller\n"
	"Rx and tx gain setting\n"
	"Fax relay mode (via T38 or pass-through)\n"
    	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_ept_prefer_codec,
	config_cmd2lic_ept_prefer_codec_cmd,                   
	"voice prefer_codec [pcmu|pcma|g723|g726_32|g729]", 
	"Voice module configuration\n" 
	"Prefer codec configuration\n" 
	"PCM-uLaw \n"
	"PCM-ALaw \n"
	"G.723 \n"
	"G.726 (32Kbit) \n"
	"G.729a \n"
	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice prefer_codec");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice prefer_codec failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}


DEFUN(config_cmd2lic_ept_codec_show,
	config_cmd2lic_ept_codec_show_cmd,                     
	"show voice codec", 
	SHOW_STR 
	"Voice module configuration\n" 
	"Codec configuration\n"
	)
{ 
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice codec");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice codec failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_call_status_show,
	config_cmd2lic_call_status_show_cmd,                   
	"show voice call-status", 
	SHOW_STR 
	"Voice module configuration\n" 
	"Call status\n"
	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice call-status");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice call-status failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}	

DEFUN(config_cmd2lic_phone_status_show,
	config_cmd2lic_phone_status_show_cmd,                  
	"show voice phone-status", 
	SHOW_STR 
	"Voice module configuration\n" 
	"Phone call status\n"
	)
{ 
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice phone-status");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice phone-status failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_codec_statistics_show,
	config_cmd2lic_codec_statistics_show_cmd,               
	"show voice codec-statistics", 
	SHOW_STR 
	"Voice module configuration\n" 
	"Codec statistics\n"
	)
{ 
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice codec-statistics");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice codec-statistics failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_codec_statistics_clear,
	config_cmd2lic_codec_statistics_clear_cmd,            
	"voice codec-statistics clear {[phone1|phone2]}*1", 
	"Voice module configuration\n" 
	"Codec statistics\n"
	"Clear the codec statistics\n"
	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "voice codec-statistics clear");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice codec-statistics clear failed!\r\n");	
		return CMD_WARNING;
		}
    	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_sip_user_agent_show,
	config_cmd2lic_sip_user_agent_show_cmd,                 
	"show sip user-agent", 
	SHOW_STR
	SIP_STR
	"SIP user-agent config\n")
{ 
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show sip user-agent");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show sip user-agent failed!\r\n");	
		return CMD_WARNING;
		}

    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_user_name_config,
	config_cmd2lic_h323_user_name_config_cmd,                 
	"h323 [phone1|phone2|phoneall] username <name>",
	H323_STR
	"H323 phone1 Config\n"
	"H323 phone2 Config\n"
	"H323 both phone1 and phone2 Config\n" 
	"UserName config\n" 
	"User Name (1-64)\n")
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h323");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h323 username failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_password_config,
	config_cmd2lic_h323_password_config_cmd,                    
	"h323 [phone1|phone2|phoneall] password <pass>",
	H323_STR
	"H323 phone1 Config\n"
	"H323 phone2 Config\n"
	"H323 both phone1 and phone2 Config\n" 
	"PassWord config\n" 
	"User passwords (1-64)\n")
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h323");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h323 password failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_id_config,
	config_cmd2lic_h323_id_config_cmd,                         
	"h323 [phone1|phone2|phoneall] h323-id <id>",
	H323_STR
	"H323 phone1 Config\n"
	"H323 phone2 Config\n"
	"H323 both phone1 and phone2 Config\n" 
	"H323 id config\n" 
	"H323 id (1-64)\n")
{ 
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h323");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h323 h323-id failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_gatekeeper_address_config,
	config_cmd2lic_h323_gatekeeper_address_config_cmd,                    
	"h323 gatekeeper address [<A.B.C.D>|auto-discover|<domain-name>]",
	H323_STR
	"H323 gatekeeper config\n"
	"Gatekeeper address config\n"
	"IP address\n"
	"Auto discover the gatekeeper\n"
	"Gatekeeper's domain-name\n")
{ 
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h323 gatekeeper address");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h323 gatekeeper address failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_gatekeeper_discover_mode_config,
	config_cmd2lic_h323_gatekeeper_discover_mode_config_cmd,            
	"h323 gatekeeper discover-mode [broadcast|multicast]",
	H323_STR
	"H323 gatekeeper config\n"
	"Gatekeeper discover mode\n"
	"Broadcast (default)\n"
	"Multicast\n")
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h323 gatekeeper discover-mode");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h323 gatekeeper discover-mode failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_gatekeeper_port_config,
	config_cmd2lic_h323_gatekeeper_port_config_cmd,                   
	"h323 gatekeeper port <1-65535>",
	H323_STR
	"Gatekeeper ras port config\n"
	"Gatekeeper ras port (1-65535), default 1719\n")
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h323 gatekeeper port");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h323 gatekeeper port failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_gatekeeper_security_config,
	config_cmd2lic_h323_gatekeeper_security_config_cmd,                
	"h323 gatekeeper security [enable|disable]",
	H323_STR
	"Gatekeeper config\n"
	"Gatekeeper security configuration (MD5)\n"
	"Enable\n"
	"Disable\n")
{ 
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h323 gatekeeper security");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h323 gatekeeper security failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_fast_startup_config,
	config_cmd2lic_h323_fast_startup_config_cmd,                      
	"h323 fast-startup [enable|disable]",
	H323_STR
	"H323 fast startup config\n"
	"Enable\n"
	"Disable\n")
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h323 fast-startup");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h323 fast-startup failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_h245_tunnel_config,
	config_cmd2lic_h323_h245_tunnel_config_cmd,                     
	"h323 h245-tunnel [enable|disable]",
	H323_STR
	"H245 tunnel config\n"
	"Enable\n"
	"Disable\n")
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h323 h245-tunnel");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h323 h245-tunnel failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_dtmf_relay_config,
	config_cmd2lic_h323_dtmf_relay_config_cmd,                  
	"h323 dtmf-relay [in_band|rfc2833]",
	H323_STR
	"Dtmf signal relay config\n"
	"In band\n"
	"Via RFC2833 telephone-events\n")
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h323 dtmf-relay");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h323 dtmf-relay failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_show_h323_phone_reg_status,
	config_cmd2lic_show_h323_phone_reg_status_cmd,               
	"show h323 regstatus",
	"Show running system information.\n" 
	SIP_STR
	"Show SIP phone registration status.\n")
{ 
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show h323 regstatus");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show h323 regstatus failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_show_h323_config,
	config_cmd2lic_show_h323_config_cmd,                       
	"show h323 [phone1|phone2|phoneall]",
	"Show running system information\n"
	H323_STR
	"Show H323 configuration for phone1\n"
	"Show H323 configuration for phone2\n" 
	"Show H323 configuration for both phone1 and phone2\n")
{ 
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show h323");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show h323 failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_dial_plan,
	config_cmd2lic_h323_dial_plan_cmd,                           
	"h323 dial-plan <dialplan>", 
	H323_STR
	"Dial plan configuration\n" 
	"Dial plan string(1-128)\n")
{ 
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "h323 dial-plan");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%h323 dial-plan failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_no_dial_plan,
	config_cmd2lic_h323_no_dial_plan_cmd,                       
	"undo h323 dial-plan", 
	"Negate a command or set its defaults.\n" 
	H323_STR
	"Dial plan configuration.\n")
{ 
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "undo h323 dial-plan");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%undo h323 dial-plan failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_h323_show_dial_plan,
	config_cmd2lic_h323_show_dial_plan_cmd,                      
	"show h323 dial-plan", 
	"Show running system information.\n" 
	H323_STR
	"Dial plan configuration.\n")
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show h323 dial-plan");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show h323 dial-plan failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

/* Complementary Service Commands */
DEFUN(config_cmd2lic_show_voice_service,
	config_cmd2lic_show_voice_service_cmd,                  
	"show voice supplement-service", 
	SHOW_STR
	"Voice module configuration\n"
	"Voice supplement service configuration\n"
	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice supplement-service");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice supplement-service failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_voice_service,
	config_cmd2lic_voice_service_cmd,                      
	"voice supplement-service [phone1|phone2|phoneall] [fwd_busy|fwd_no_reply|fwd_uncond|hot_line] <fwd_number> [enable|disable]", 
	"Voice module configuration\n"
	"Voice supplement service configuration\n" 
	"Phone 1\n"
	"Phone 2\n"
	"All phones\n"
	"Call forwarding on busy\n"
	"Call forwarding on no reply\n"
	"Call forwarding on unconditional\n"
	"Hot line service\n"
	"Forwarding number\n"
	"Enable\n"
	"Disable\n"
	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "Voice module configuration");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice supplement-service failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_voice_service2,
	config_cmd2lic_voice_service2_cmd,                    
	"voice supplement-service [phone1|phone2|phoneall] [dont_disturb|call_waiting] [enable|disable]", 
	"Voice module configuration\n"
	"Voice supplement service configuration\n" 
	"Phone 1\n"
	"Phone 2\n"
	"All phones\n"
	"Do not disturb\n"
	"Call waiting\n"
	"Enable\n"
	"Disable\n"
	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "Voice module configuration");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%voice supplement-service failed!\r\n");	
		return CMD_WARNING;
		}

	return CMD_SUCCESS;
#endif
}


DEFUN ( config_cmd2lic_voice_syslog_start_func,
        config_cmd2lic_voice_syslog_start_cmd,                       
        "syslog <module> [start|stop] <A.B.C.D/M> {<1-65535>}*1",
	 "Syslog configuration.\n"
	 "Module name, sip, cm, etc.\n"
	 "Start module associated syslog.\n"
	 "Stop module associated syslog.\n"
	 "Syslog server address.\n"
	 "Syslog server port.\n")
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "Syslog configuration");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%syslog failed!\r\n");	
		return CMD_WARNING;
		}

    return CMD_SUCCESS;
#endif
}

DEFUN( config_cmd2lic_ShowSyslogConfServer_fun,
          config_cmd2lic_ShowSyslogConfServer_CMD,                    
          "show syslog server configuration",
          DescStringCommonShow
          "Show syslog configuration\n"
          "Show syslog server configuration\n"
          "Show syslog server configuration\n" )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show syslog server configuration");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show syslog server configuration failed!\r\n");	
		return CMD_WARNING;
		}

    return CMD_SUCCESS;
#endif
}

DEFUN( config_cmd2lic_ShowRun_fun,
          config_cmd2lic_ShowRun_CMD,
          "show voice running-config",
          SHOW_STR
          "Voice module configuration\n"
          "Show current running configuration\n"
          )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice running-config");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice running-config failed!\r\n");	
		return CMD_WARNING;
		}

    return CMD_SUCCESS;
#endif
}

DEFUN(config_cmd2lic_show_voice_startup_config,
	config_cmd2lic_show_voice_startup_config_cmd,
	"show voice startup-config", 
	SHOW_STR 
	"Voice module configuration\n"
	"Show contents of startup configuration\n"
      )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show voice startup-config");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show voice startup-config failed!\r\n");	
		return CMD_WARNING;
		}

    return CMD_SUCCESS;
#endif
}

DEFUN( config_cmd2lic_ShowSyslogConf_fun,
          config_cmd2lic_ShowSyslogConf_CMD,                        
          "show syslog configuration",
          DescStringCommonShow
          "Show syslog configuration\n"
          "Show syslog configuration\n" )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_gt831_cli_process(vty, "show syslog configuration");
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	   {
	   #ifdef CLI_EPON_DEBUG
	      vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n", PonPortIdx, OnuIdx) ;
	   #endif
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	ONU_TYPE_AND_COMMAND_CHECK
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show syslog configuration failed!\r\n");	
		return CMD_WARNING;
		}

    return CMD_SUCCESS;
#endif
}



LDEFUN(onu_slab_show,
      onu_slab_show_cmd,
      "show slab mod {<modname>}*1",
      SHOW_STR
      "Show information of system memory cache\n"
      "Module memory cache information\n"
      "Specify the name of the mod\n",
          ONU_NODE
          )

{
	return CMD_SUCCESS;
} 

LDEFUN(onu_voice_slab_mod_show,
      onu_voice_slab_mod_show_cmd,
      "show slab global",
      SHOW_STR
      "Show information of system memory cache\n"
      "Global memory cache information\n",
          ONU_NODE
          )

{
	return CMD_SUCCESS;
}


LDEFUN(config_cmd2lic_ept_show,
	config_cmd2lic_ept_show_cmd,                     
	"show voice [vad|dtmf-relay|ecan|gain|fax]", 
	SHOW_STR
	"Voice module configuration\n"
	"Voice activity detection\n" 
	"DTMF relay mode (via rfc2833 or in band)\n" 
	"Echo canceller\n"
	"Rx and tx gain setting\n"
	"Fax relay mode (via T38 or pass-through)\n",
          ONU_NODE
    	)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_ept_prefer_codec,
	config_cmd2lic_ept_prefer_codec_cmd,                   
	"voice prefer_codec [pcmu|pcma|g723|g726_32|g729]", 
	"Voice module configuration\n" 
	"Prefer codec configuration\n" 
	"PCM-uLaw \n"
	"PCM-ALaw \n"
	"G.723 \n"
	"G.726 (32Kbit) \n"
	"G.729a \n",
          ONU_NODE
	)
{
	return CMD_SUCCESS;
}


LDEFUN(config_cmd2lic_ept_codec_show,
	config_cmd2lic_ept_codec_show_cmd,                     
	"show voice codec", 
	SHOW_STR 
	"Voice module configuration\n" 
	"Codec configuration\n",
          ONU_NODE
	)
{ 
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_call_status_show,
	config_cmd2lic_call_status_show_cmd,                   
	"show voice call-status", 
	SHOW_STR 
	"Voice module configuration\n" 
	"Call status\n",
          ONU_NODE
	)
{
	return CMD_SUCCESS;
}	

LDEFUN(config_cmd2lic_phone_status_show,
	config_cmd2lic_phone_status_show_cmd,                  
	"show voice phone-status", 
	SHOW_STR 
	"Voice module configuration\n" 
	"Phone call status\n",
          ONU_NODE
	)
{ 
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_codec_statistics_show,
	config_cmd2lic_codec_statistics_show_cmd,               
	"show voice codec-statistics", 
	SHOW_STR 
	"Voice module configuration\n" 
	"Codec statistics\n",
          ONU_NODE
	)
{ 
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_codec_statistics_clear,
	config_cmd2lic_codec_statistics_clear_cmd,            
	"voice codec-statistics clear {[phone1|phone2]}*1", 
	"Voice module configuration\n" 
	"Codec statistics\n"
	"Clear the codec statistics\n",
          ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_sip_user_agent_show,
	config_cmd2lic_sip_user_agent_show_cmd,                 
	"show sip user-agent", 
	SHOW_STR
	SIP_STR
	"SIP user-agent config\n",
          ONU_NODE
          )
{ 
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_user_name_config,
	config_cmd2lic_h323_user_name_config_cmd,                 
	"h323 [phone1|phone2|phoneall] username <name>",
	H323_STR
	"H323 phone1 Config\n"
	"H323 phone2 Config\n"
	"H323 both phone1 and phone2 Config\n" 
	"UserName config\n" 
	"User Name (1-64)\n",
          ONU_NODE
          )
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_password_config,
	config_cmd2lic_h323_password_config_cmd,                    
	"h323 [phone1|phone2|phoneall] password <pass>",
	H323_STR
	"H323 phone1 Config\n"
	"H323 phone2 Config\n"
	"H323 both phone1 and phone2 Config\n" 
	"PassWord config\n" 
	"User passwords (1-64)\n",
          ONU_NODE
          )
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_id_config,
	config_cmd2lic_h323_id_config_cmd,                         
	"h323 [phone1|phone2|phoneall] h323-id <id>",
	H323_STR
	"H323 phone1 Config\n"
	"H323 phone2 Config\n"
	"H323 both phone1 and phone2 Config\n" 
	"H323 id config\n" 
	"H323 id (1-64)\n",
          ONU_NODE
          )
{ 
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_gatekeeper_address_config,
	config_cmd2lic_h323_gatekeeper_address_config_cmd,                    
	"h323 gatekeeper address [<A.B.C.D>|auto-discover|<domain-name>]",
	H323_STR
	"H323 gatekeeper config\n"
	"Gatekeeper address config\n"
	"IP address\n"
	"Auto discover the gatekeeper\n"
	"Gatekeeper's domain-name\n",
          ONU_NODE
	)
{ 
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_gatekeeper_discover_mode_config,
	config_cmd2lic_h323_gatekeeper_discover_mode_config_cmd,            
	"h323 gatekeeper discover-mode [broadcast|multicast]",
	H323_STR
	"H323 gatekeeper config\n"
	"Gatekeeper discover mode\n"
	"Broadcast (default)\n"
	"Multicast\n",
          ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_gatekeeper_port_config,
	config_cmd2lic_h323_gatekeeper_port_config_cmd,                   
	"h323 gatekeeper port <1-65535>",
	H323_STR
	"Gatekeeper ras port config\n"
	"Gatekeeper ras port (1-65535), default 1719\n",
          ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_gatekeeper_security_config,
	config_cmd2lic_h323_gatekeeper_security_config_cmd,                
	"h323 gatekeeper security [enable|disable]",
	H323_STR
	"Gatekeeper config\n"
	"Gatekeeper security configuration (MD5)\n"
	"Enable\n"
	"Disable\n",
          ONU_NODE
	)
{ 
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_fast_startup_config,
	config_cmd2lic_h323_fast_startup_config_cmd,                      
	"h323 fast-startup [enable|disable]",
	H323_STR
	"H323 fast startup config\n"
	"Enable\n"
	"Disable\n",
          ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_h245_tunnel_config,
	config_cmd2lic_h323_h245_tunnel_config_cmd,                     
	"h323 h245-tunnel [enable|disable]",
	H323_STR
	"H245 tunnel config\n"
	"Enable\n"
	"Disable\n",
          ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_dtmf_relay_config,
	config_cmd2lic_h323_dtmf_relay_config_cmd,                  
	"h323 dtmf-relay [in_band|rfc2833]",
	H323_STR
	"Dtmf signal relay config\n"
	"In band\n"
	"Via RFC2833 telephone-events\n",
          ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_show_h323_phone_reg_status,
	config_cmd2lic_show_h323_phone_reg_status_cmd,               
	"show h323 regstatus",
	"Show running system information.\n" 
	SIP_STR
	"Show SIP phone registration status.\n",
          ONU_NODE
	)
{ 
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_show_h323_config,
	config_cmd2lic_show_h323_config_cmd,                       
	"show h323 [phone1|phone2|phoneall]",
	"Show running system information\n"
	H323_STR
	"Show H323 configuration for phone1\n"
	"Show H323 configuration for phone2\n" 
	"Show H323 configuration for both phone1 and phone2\n",
          ONU_NODE
          )
{ 
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_dial_plan,
	config_cmd2lic_h323_dial_plan_cmd,                           
	"h323 dial-plan <dialplan>", 
	H323_STR
	"Dial plan configuration\n" 
	"Dial plan string(1-128)\n",
          ONU_NODE
          )
{ 
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_no_dial_plan,
	config_cmd2lic_h323_no_dial_plan_cmd,                       
	"undo h323 dial-plan", 
	"Negate a command or set its defaults.\n" 
	H323_STR
	"Dial plan configuration.\n",
          ONU_NODE
          )
{ 
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_h323_show_dial_plan,
	config_cmd2lic_h323_show_dial_plan_cmd,                      
	"show h323 dial-plan", 
	"Show running system information.\n" 
	H323_STR
	"Dial plan configuration.\n",
          ONU_NODE
          )
{
	return CMD_SUCCESS;
}

/* Complementary Service Commands */
LDEFUN(config_cmd2lic_show_voice_service,
	config_cmd2lic_show_voice_service_cmd,                  
	"show voice supplement-service", 
	SHOW_STR
	"Voice module configuration\n"
	"Voice supplement service configuration\n",
          ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_voice_service,
	config_cmd2lic_voice_service_cmd,                      
	"voice supplement-service [phone1|phone2|phoneall] [fwd_busy|fwd_no_reply|fwd_uncond|hot_line] <fwd_number> [enable|disable]", 
	"Voice module configuration\n"
	"Voice supplement service configuration\n" 
	"Phone 1\n"
	"Phone 2\n"
	"All phones\n"
	"Call forwarding on busy\n"
	"Call forwarding on no reply\n"
	"Call forwarding on unconditional\n"
	"Hot line service\n"
	"Forwarding number\n"
	"Enable\n"
	"Disable\n",
          ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_voice_service2,
	config_cmd2lic_voice_service2_cmd,                    
	"voice supplement-service [phone1|phone2|phoneall] [dont_disturb|call_waiting] [enable|disable]", 
	"Voice module configuration\n"
	"Voice supplement service configuration\n" 
	"Phone 1\n"
	"Phone 2\n"
	"All phones\n"
	"Do not disturb\n"
	"Call waiting\n"
	"Enable\n"
	"Disable\n",
          ONU_NODE
	)
{
	return CMD_SUCCESS;
}


LDEFUN ( config_cmd2lic_voice_syslog_start_func,
        config_cmd2lic_voice_syslog_start_cmd,                       
        "syslog <module> [start|stop] <A.B.C.D/M> {<1-65535>}*1",
	 "Syslog configuration.\n"
	 "Module name, sip, cm, etc.\n"
	 "Start module associated syslog.\n"
	 "Stop module associated syslog.\n"
	 "Syslog server address.\n"
	 "Syslog server port.\n",
          ONU_NODE)
{
	return CMD_SUCCESS;
}

LDEFUN( config_cmd2lic_ShowSyslogConfServer_fun,
          config_cmd2lic_ShowSyslogConfServer_CMD,                    
          "show syslog server configuration",
          DescStringCommonShow
          "Show syslog configuration\n"
          "Show syslog server configuration\n"
          "Show syslog server configuration\n" ,
          ONU_NODE)
{
	return CMD_SUCCESS;
}

LDEFUN( config_cmd2lic_ShowRun_fun,
          config_cmd2lic_ShowRun_CMD,
          "show voice running-config",
          SHOW_STR
          "Voice module configuration\n"
          "Show current running configuration\n",
          ONU_NODE
          )
{
	return CMD_SUCCESS;
}

LDEFUN(config_cmd2lic_show_voice_startup_config,
	config_cmd2lic_show_voice_startup_config_cmd,
	"show voice startup-config", 
	SHOW_STR 
	"Voice module configuration\n"
	"Show contents of startup configuration\n",
          ONU_NODE
      )
{
	return CMD_SUCCESS;
}

LDEFUN( config_cmd2lic_ShowSyslogConf_fun,
          config_cmd2lic_ShowSyslogConf_CMD,                        
          "show syslog configuration",
          DescStringCommonShow
          "Show syslog configuration\n"
          "Show syslog configuration\n",
          ONU_NODE
          )
{
	return CMD_SUCCESS;
}

static int onu_gt831_cli_process( struct vty *vty, char *hint_str )
{
	LONG lRet;
	CHAR pBuff[256];   
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	/*int OnuType;*/

    /* B--remmed by zyt@2010-7-22 for DEBUG */
	if( check_onu_cli_is_deny(vty) )	/* modified by xieshl 20100521 */
		return CMD_SUCCESS;
    /* E--remmed by zyt@2010-7-22 for DEBUG */

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	
	 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
	if ( CLI_EPON_ONUUP != lRet)
	{
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	}
	/*
	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
	{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
	}
	*/
	if(OnuIsGT831( PonPortIdx, OnuIdx) != ROK ) 
		{
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}

	VOS_MemCpy( pBuff, vty->buf, vty->length);
    pBuff[vty->length] = '\0';
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %% %s failed!\r\n", hint_str );	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

/*1 sip dns srv-record {[enable|disable]}*1	配置SIP DNS-SRV地址选择（当DNS返回多个地址时，如果一个地址失败是否切换到另外一个地址） */
DEFUN(config_cmd2lic_sip_dns_srv_config,
	config_cmd2lic_sip_dns_srv_config_cmd,
	"sip dns srv-record {[enable|disable]}*1",
	SIP_STR
	"SIP DNS Config\n"
	"DNS srv records usage\n"
	"Enable use next dns srv records when first fail\n" 
	"Disable use next dns srv records when first fail\n"
    	)
{
    return onu_gt831_cli_process(vty, "sip dns srv-record");
}


/* 2.dial-plan [add|delete] <dialplan> 增加/删除拨号计划(在现有拨号计划基础上)*/
DEFUN(config_cmd2lic_sip_add_del_dial_plan,
	config_cmd2lic_sip_add_del_dial_plan_cmd,
	"dial-plan [add|delete] <dialplan>", 
	"Dial plan configuration\n" 
	"Add a dialplan\n"
	"Delete an exist dialplan\n"
	"Dial plan string(1-128)\n"
    	)
{
	return onu_gt831_cli_process(vty, "dial-plan");
}


/* 3.dial-plan buildin		使用系统内置的拨号计划（内置为全国的拨号计划） */
DEFUN(config_cmd2lic_sip_buildin_dial_plan,
	config_cmd2lic_sip_buildin_dial_plan_cmd,
	"dial-plan buildin", 
	"Dial plan configuration\n" 
	"Using buildin dialplan\n"    	
	)
{
	return onu_gt831_cli_process(vty, "dial-plan buildin");
}


DEFUN(h248_codec_config,
	h248_codec_config_cmd,
	"h248 codec [PCMA|PCMU|G729|G726_32|G7231]",
	H248_STR 
	"codec peference\n"
	"codec type\n"
	"codec type\n"
	"codec type\n"
	"codec type\n"
	"codec type\n")
{
	return onu_gt831_cli_process(vty, "h248 codec");
}
DEFUN(h248_mgheartbeat,
	h248_mgheartbeat_cmd,
	"h248 heartbeat mg-heartbeat [enable|disable]",
	H248_STR
	"heartbeat config\n"
	"mg-heartbeat config\n"
	"send mg-heartbeat msg\n"
	"do not send mg-heartbeat msg\n")
{
	return onu_gt831_cli_process(vty, "h248 heartbeat mg-heartbeat");
}
DEFUN(h248_heartbeatimer_config,
	h248_heartbeatimer_config_cmd, 
	"h248 heartbeat time <0-3600>", 
	H248_STR
	"heartbeat config\n"
	"heartbeat timer config\n" 
	"Heartbeat timer \n")
{
	return onu_gt831_cli_process(vty, "h248 heartbeat time");
}
DEFUN(h248_local_domain_config,
	h248_local_domain_config_cmd,
	"h248 local domain <domainname>",
	H248_STR
	"Media gateway config\n"
	"domainname config\n" 
	"domainname <domainname>\n")
{
	return onu_gt831_cli_process(vty, "h248 local domain");
}
DEFUN(h248_local_name_config,
	h248_local_name_config_cmd,
	"h248 local name <localnameprefix> <0-65535> <0-65535> <1-65535> [fix|unfix] <1-10>",
	H248_STR
	"Media gateway config\n"
	"localname config\n"
	"localnameprefix config\n"
	"start id\n"
	"end id\n"
	"id increment\n"
	"localnamesuffix fixed length or unfixed\n"
	"localnamesuffix length config\n")
{
	return onu_gt831_cli_process(vty, "h248 local name");
}
DEFUN(h248_local_port_config,
	h248_local_port_config_cmd,
	"h248 local port <1-65535>",
	H248_STR
	"Media gateway config\n"
	"port config\n" 
	"port number\n")
{
	return onu_gt831_cli_process(vty, "h248 local port");
}
DEFUN(h248_local_rtpprefix_config,
	h248_local_rtpprefix_config_cmd,
	"h248 local rtp <rtpprefix> <0-65535> <0-65535> <1-65535> [fix|unfix] <1-10>",
	H248_STR
	"Media gateway config\n"
	"rtpname config\n"
	"rtpnameprefix config\n" 
	"start id\n"
	"end id\n"
	"id increment\n"
	"rtpnamesuffix fixed length or unfixed\n"
	"rtpnamesuffix length config\n")
{
	return onu_gt831_cli_process(vty, "h248 local rtp");
}
DEFUN(h248_mgc_address_config,
	h248_mgc_address_config_cmd,
	"h248 mgc address <address>",
	H248_STR
	"Media gateway controller config\n"
	"address config\n"
	"ip address (xx.xx.xx.xx)\n")
{
	return onu_gt831_cli_process(vty, "h248 mgc address");
}
DEFUN(h248_mgc_port_config,
	h248_mgc_port_config_cmd,
	"h248 mgc port <1-65535>",
	H248_STR
	"Media gateway controller config\n"
	"port config\n" 
	"port number\n")
{
	return onu_gt831_cli_process(vty, "h248 mgc port");
}
DEFUN(h248_mgcbackup_address,
	h248_mgcbackup_address_cmd,
	"h248 mgc-backup address <address>",
	H248_STR
	"backup mgc config\n"
	"backup mgc address config\n"
	"ip address (xx.xx.xx.xx)\n")
{
	return onu_gt831_cli_process(vty, "h248 mgc-backup address");
}
DEFUN(h248_mgcbackup_port,
	h248_mgcbackup_port_cmd,
	"h248 mgc-backup port <1-65535>",
	H248_STR
	"backup mgc  config\n"
	"backup mgc port config\n"
	"Port number\n")
{
	return onu_gt831_cli_process(vty, "h248 mgc-backup port");
}
DEFUN(h248_registermode_config,
	h248_registermode_config_cmd,
	"h248 registermode [ip|domain]",
	H248_STR 
	"Registermode : ip or domain\n"
	"Using ip address\n"
	"Using local domain name\n")
{
	return onu_gt831_cli_process(vty, "h248 registermode");
}
DEFUN(h248_reporthook,
	h248_reporthook_cmd,
	"h248 reporthookstate [enable|disable]",
	H248_STR
	"report hookstate by constraint\n"
	"report hook state\n"
	"do not report hook state\n")
{
	return onu_gt831_cli_process(vty, "h248 reporthookstate");
}
DEFUN(h248_srvchng_stat,
	h248_srvchng_stat_cmd,
	"show h248 regstatus\n",
	"Show running system H.248 information\n"
	"H.248 Protocol Configuration\n"
	"Show H.248 Protocol registration status\n")
{
	return onu_gt831_cli_process(vty, "show h248 regstatus");
}
DEFUN(h248_startup, 
	h248_startup_cmd,
	"h248 startup", 
	H248_STR 
	"startup h248 service\n")
{
	return onu_gt831_cli_process(vty, "h248 startup");
}
DEFUN(h248_ctx_disp,
	h248_ctx_disp_cmd,
	"show h248 context-status",
	"Show running system H.248 information\n"
	"H.248 Protocol Configuration\n"
	"show number of terminations in each context\n")
{
	return onu_gt831_cli_process(vty, "show h248 context-status");
}
DEFUN(sip_show_dialogs,
	sip_show_dialogs_cmd,
	"show sip dialog {[new|invited|inviting|established|cancelling|cancelled|terminated|fake]}*1",
	"Show running system information.\n" 
	 SIP_STR
	"Dialog information.\n"
	"Dialog in new status.\n"
	"Dialog in invited status.\n"
	"Dialog in inviting status.\n"
	"Dialog in established status.\n"
	"Dialog in cancelling status.\n"
	"Dialog in cancelled status.\n"	
	"Dialog in terminated status.\n"	
	"Dialog in fake status.\n")
{
	return onu_gt831_cli_process(vty, "show sip dialog");
}
/*DEFUN(sip_show_transactions,
	sip_show_transactions_cmd,
	"show sip transaction [ict|nict|ist|nist|all]",
	"Show running system information.\n"
	SIP_STR
	"Transactions information.\n"
	"ICT transaction information.\n"
	"NICT transaction information.\n"
	"IST transaction information.\n"
	"NIST transaction information.\n" 
	"All transaction information.\n")
{
	return onu_gt831_cli_process(vty, "show sip transaction");
}
DEFUN(sip_dns_srv_config,
	sip_dns_srv_config_cmd,
	"sip dns srv-record {[enable|disable]}*1",
	SIP_STR
	"SIP DNS Config\n"
	"DNS srv records usage\n"
	"Enable use next dns srv records when first fail\n" 
	"Disable use next dns srv records when first fail\n")
{
	return onu_gt831_cli_process(vty, "sip dns srv-record");
}
DEFUN(sip_provisional_reliable,
	sip_provisional_reliable_cmd,
	"sip prack {[enable|disable]}*1",
	"SIP configuration.\n"
	"PRACK support.\n"
	"Enable.\n"
	"Disable.\n")
{
	return onu_gt831_cli_process(vty, "sip prack");
}*/
DEFUN(sip_supplementary_service,
	sip_supplementary_service_cmd,
	"sip supplement_service {[local|soft-switch]}*1",
	"SIP configuration.\n"
	"Supplement service support.\n"
	"Enable at local.\n"
	"Enable on softswitch.\n")
{
	return onu_gt831_cli_process(vty, "sip supplement_service");
}
DEFUN(voice_service,
	voice_service_cmd, 
	"voice supplement-service phone <1-2> [fwd_busy|fwd_no_reply|fwd_uncond|hot_line] <fwd_number> [enable|disable]", 
	"Voice module configuration\n"
	"Voice supplement service configuration\n" 
	"Phone number\n"
	"Phone 1-2\n"
	"All phones\n"
	"Call forwarding on busy\n"
	"Call forwarding on no reply\n"
	"Call forwarding on unconditional\n"
	"Hot line service\n"
	"Forwarding number\n"
	"Enable\n"
	"Disable\n")
{
	return onu_gt831_cli_process(vty, "voice supplement-service phone");
}
DEFUN(voice_service2,
	voice_service2_cmd, 
	"voice supplement-service phone <1-2> [dont_disturb|call_waiting|line_rev] [enable|disable]", 
	"Voice module configuration\n"
	"Voice supplement service configuration\n" 
	"Phone number\n"
	"Phone 1-2\n"
	"All phones\n"
	"Do not disturb\n"
	"Call waiting\n"
	"Line polarity reverse\n"
	"Enable\n"
	"Disable\n")
{
	return onu_gt831_cli_process(vty, "voice supplement-service phone");
}
/*DEFUN(show_voice_service,
	show_voice_service_cmd, 
	"show voice supplement-service", 
	SHOW_STR
	"Voice module configuration\n"
	"Voice supplement service configuration\n" )
{
	return onu_gt831_cli_process(vty, "show voice supplement-service");
}*/
DEFUN(sip_match_dial_plan,
	sip_match_dial_plan_cmd,
	"dial-plan match-mode {[longest|shortest]}*1", 
	"Dial plan configuration\n" 
	"Dialplan matching mode\n"
	"Longest"
	"Shortest")
{
	return onu_gt831_cli_process(vty, "dial-plan match-mode");
}
DEFUN(sip_re_register,
	sip_re_register_cmd,
	"sip re-register", 
	SIP_STR
	"SIP re-register\n" )
{
	return onu_gt831_cli_process(vty, "sip re-register");
}
/*DEFUN(onu_syslog_show,
	onu_syslog_show_cmd,
	"show syslog [flash|memory] {<0-3>}*1", 
	SHOW_STR 
	"Syslog\n"
	"Syslog saved in flash\n"
	"Syslog saved in memory\n"
	"Syslog block number (total syslog size is 128K, splitted into 4 block, 32K in each block)\n")
{
	return onu_gt831_cli_process(vty, "show syslog");
}
DEFUN ( show_exception_syslog_func,
        show_exception_syslog_cmd,
        "show execpt-log",
        SHOW_STR
        "Exception log file\n")
{
	return onu_gt831_cli_process(vty, "show execpt-log");
}*/
DEFUN(slic_ac_profile_rd,
	slic_ac_profile_rd_cmd, 
	"voice slic ac_profile_show", 
	"Voice module configuration\n" 
	"Slic configuration\n" 
	"Slic ac profile show\n")
{
	return onu_gt831_cli_process(vty, "voice slic ac_profile_show");
}
DEFUN(slic_impedance,
	slic_impedance_cmd, 
	"voice slic impedance [r600|china]", 
	"Voice module configuration\n" 
	"Slic configuration\n" 
	"Slic impedance configuration\n"
	"600 R\n"
	"China\n")
{
	return onu_gt831_cli_process(vty, "voice slic impedance");
}


LDEFUN(config_cmd2lic_sip_dns_srv_config,
	config_cmd2lic_sip_dns_srv_config_cmd,
	"sip dns srv-record {[enable|disable]}*1",
	SIP_STR
	"SIP DNS Config\n"
	"DNS srv records usage\n"
	"Enable use next dns srv records when first fail\n" 
	"Disable use next dns srv records when first fail\n",
	ONU_NODE
    	)
{
	return CMD_SUCCESS;
}


/* 2.dial-plan [add|delete] <dialplan> 增加/删除拨号计划(在现有拨号计划基础上)*/
LDEFUN(config_cmd2lic_sip_add_del_dial_plan,
	config_cmd2lic_sip_add_del_dial_plan_cmd,
	"dial-plan [add|delete] <dialplan>", 
	"Dial plan configuration\n" 
	"Add a dialplan\n"
	"Delete an exist dialplan\n"
	"Dial plan string(1-128)\n",
	ONU_NODE
    	)
{
	return CMD_SUCCESS;
}


/* 3.dial-plan buildin		使用系统内置的拨号计划（内置为全国的拨号计划） */
LDEFUN(config_cmd2lic_sip_buildin_dial_plan,
	config_cmd2lic_sip_buildin_dial_plan_cmd,
	"dial-plan buildin", 
	"Dial plan configuration\n" 
	"Using buildin dialplan\n",
	ONU_NODE
	)
{
	return CMD_SUCCESS;
}


LDEFUN(h248_codec_config,
	h248_codec_config_cmd,
	"h248 codec [PCMA|PCMU|G729|G726_32|G7231]",
	H248_STR 
	"codec peference\n"
	"codec type\n"
	"codec type\n"
	"codec type\n"
	"codec type\n"
	"codec type\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_mgheartbeat,
	h248_mgheartbeat_cmd,
	"h248 heartbeat mg-heartbeat [enable|disable]",
	H248_STR
	"heartbeat config\n"
	"mg-heartbeat config\n"
	"send mg-heartbeat msg\n"
	"do not send mg-heartbeat msg\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_heartbeatimer_config,
	h248_heartbeatimer_config_cmd, 
	"h248 heartbeat time <0-3600>", 
	H248_STR
	"heartbeat config\n"
	"heartbeat timer config\n" 
	"Heartbeat timer \n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_local_domain_config,
	h248_local_domain_config_cmd,
	"h248 local domain <domainname>",
	H248_STR
	"Media gateway config\n"
	"domainname config\n" 
	"domainname <domainname>\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_local_name_config,
	h248_local_name_config_cmd,
	"h248 local name <localnameprefix> <0-65535> <0-65535> <1-65535> [fix|unfix] <1-10>",
	H248_STR
	"Media gateway config\n"
	"localname config\n"
	"localnameprefix config\n"
	"start id\n"
	"end id\n"
	"id increment\n"
	"localnamesuffix fixed length or unfixed\n"
	"localnamesuffix length config\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_local_port_config,
	h248_local_port_config_cmd,
	"h248 local port <1-65535>",
	H248_STR
	"Media gateway config\n"
	"port config\n" 
	"port number\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_local_rtpprefix_config,
	h248_local_rtpprefix_config_cmd,
	"h248 local rtp <rtpprefix> <0-65535> <0-65535> <1-65535> [fix|unfix] <1-10>",
	H248_STR
	"Media gateway config\n"
	"rtpname config\n"
	"rtpnameprefix config\n" 
	"start id\n"
	"end id\n"
	"id increment\n"
	"rtpnamesuffix fixed length or unfixed\n"
	"rtpnamesuffix length config\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_mgc_address_config,
	h248_mgc_address_config_cmd,
	"h248 mgc address <address>",
	H248_STR
	"Media gateway controller config\n"
	"address config\n"
	"ip address (xx.xx.xx.xx)\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_mgc_port_config,
	h248_mgc_port_config_cmd,
	"h248 mgc port <1-65535>",
	H248_STR
	"Media gateway controller config\n"
	"port config\n" 
	"port number\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_mgcbackup_address,
	h248_mgcbackup_address_cmd,
	"h248 mgc-backup address <address>",
	H248_STR
	"backup mgc config\n"
	"backup mgc address config\n"
	"ip address (xx.xx.xx.xx)\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_mgcbackup_port,
	h248_mgcbackup_port_cmd,
	"h248 mgc-backup port <1-65535>",
	H248_STR
	"backup mgc  config\n"
	"backup mgc port config\n"
	"Port number\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_registermode_config,
	h248_registermode_config_cmd,
	"h248 registermode [ip|domain]",
	H248_STR 
	"Registermode : ip or domain\n"
	"Using ip address\n"
	"Using local domain name\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_reporthook,
	h248_reporthook_cmd,
	"h248 reporthookstate [enable|disable]",
	H248_STR
	"report hookstate by constraint\n"
	"report hook state\n"
	"do not report hook state\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_srvchng_stat,
	h248_srvchng_stat_cmd,
	"show h248 regstatus\n",
	"Show running system H.248 information\n"
	"H.248 Protocol Configuration\n"
	"Show H.248 Protocol registration status\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_startup, 
	h248_startup_cmd,
	"h248 startup", 
	H248_STR 
	"startup h248 service\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(h248_ctx_disp,
	h248_ctx_disp_cmd,
	"show h248 context-status",
	"Show running system H.248 information\n"
	"H.248 Protocol Configuration\n"
	"show number of terminations in each context\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(sip_show_dialogs,
	sip_show_dialogs_cmd,
	"show sip dialog {[new|invited|inviting|established|cancelling|cancelled|terminated|fake]}*1",
	"Show running system information.\n" 
	 SIP_STR
	"Dialog information.\n"
	"Dialog in new status.\n"
	"Dialog in invited status.\n"
	"Dialog in inviting status.\n"
	"Dialog in established status.\n"
	"Dialog in cancelling status.\n"
	"Dialog in cancelled status.\n"	
	"Dialog in terminated status.\n"	
	"Dialog in fake status.\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
/*DEFUN(sip_show_transactions,
	sip_show_transactions_cmd,
	"show sip transaction [ict|nict|ist|nist|all]",
	"Show running system information.\n"
	SIP_STR
	"Transactions information.\n"
	"ICT transaction information.\n"
	"NICT transaction information.\n"
	"IST transaction information.\n"
	"NIST transaction information.\n" 
	"All transaction information.\n")
{
	return onu_gt831_cli_process(vty, "show sip transaction");
}
DEFUN(sip_dns_srv_config,
	sip_dns_srv_config_cmd,
	"sip dns srv-record {[enable|disable]}*1",
	SIP_STR
	"SIP DNS Config\n"
	"DNS srv records usage\n"
	"Enable use next dns srv records when first fail\n" 
	"Disable use next dns srv records when first fail\n")
{
	return onu_gt831_cli_process(vty, "sip dns srv-record");
}
DEFUN(sip_provisional_reliable,
	sip_provisional_reliable_cmd,
	"sip prack {[enable|disable]}*1",
	"SIP configuration.\n"
	"PRACK support.\n"
	"Enable.\n"
	"Disable.\n")
{
	return onu_gt831_cli_process(vty, "sip prack");
}*/
LDEFUN(sip_supplementary_service,
	sip_supplementary_service_cmd,
	"sip supplement_service {[local|soft-switch]}*1",
	"SIP configuration.\n"
	"Supplement service support.\n"
	"Enable at local.\n"
	"Enable on softswitch.\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(voice_service,
	voice_service_cmd, 
	"voice supplement-service phone <1-2> [fwd_busy|fwd_no_reply|fwd_uncond|hot_line] <fwd_number> [enable|disable]", 
	"Voice module configuration\n"
	"Voice supplement service configuration\n" 
	"Phone number\n"
	"Phone 1-2\n"
	"All phones\n"
	"Call forwarding on busy\n"
	"Call forwarding on no reply\n"
	"Call forwarding on unconditional\n"
	"Hot line service\n"
	"Forwarding number\n"
	"Enable\n"
	"Disable\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(voice_service2,
	voice_service2_cmd, 
	"voice supplement-service phone <1-2> [dont_disturb|call_waiting|line_rev] [enable|disable]", 
	"Voice module configuration\n"
	"Voice supplement service configuration\n" 
	"Phone number\n"
	"Phone 1-2\n"
	"All phones\n"
	"Do not disturb\n"
	"Call waiting\n"
	"Line polarity reverse\n"
	"Enable\n"
	"Disable\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
/*DEFUN(show_voice_service,
	show_voice_service_cmd, 
	"show voice supplement-service", 
	SHOW_STR
	"Voice module configuration\n"
	"Voice supplement service configuration\n" )
{
	return onu_gt831_cli_process(vty, "show voice supplement-service");
}*/
LDEFUN(sip_match_dial_plan,
	sip_match_dial_plan_cmd,
	"dial-plan match-mode {[longest|shortest]}*1", 
	"Dial plan configuration\n" 
	"Dialplan matching mode\n"
	"Longest"
	"Shortest",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(sip_re_register,
	sip_re_register_cmd,
	"sip re-register", 
	SIP_STR
	"SIP re-register\n",
	ONU_NODE )
{
	return CMD_SUCCESS;
}
/*DEFUN(onu_syslog_show,
	onu_syslog_show_cmd,
	"show syslog [flash|memory] {<0-3>}*1", 
	SHOW_STR 
	"Syslog\n"
	"Syslog saved in flash\n"
	"Syslog saved in memory\n"
	"Syslog block number (total syslog size is 128K, splitted into 4 block, 32K in each block)\n")
{
	return onu_gt831_cli_process(vty, "show syslog");
}
DEFUN ( show_exception_syslog_func,
        show_exception_syslog_cmd,
        "show execpt-log",
        SHOW_STR
        "Exception log file\n")
{
	return onu_gt831_cli_process(vty, "show execpt-log");
}*/
LDEFUN(slic_ac_profile_rd,
	slic_ac_profile_rd_cmd, 
	"voice slic ac_profile_show", 
	"Voice module configuration\n" 
	"Slic configuration\n" 
	"Slic ac profile show\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(slic_impedance,
	slic_impedance_cmd, 
	"voice slic impedance [r600|china]", 
	"Voice module configuration\n" 
	"Slic configuration\n" 
	"Slic impedance configuration\n"
	"600 R\n"
	"China\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}


QDEFUN( IpAddr_voice_if_Func,
        IpAddr_voice_if_CMD,
        "voice ip address <A.B.C.D/M> vlan <1-4094>" ,
        "Config voice module information\n"
        IP_STR
        IPADDR_STR
        ABCDM_STR
        "Vlan interface\n"
        "Vlan id\n",
        &g_ulIFMQue )
{
	return onu_gt831_cli_process(vty, "voice ip address");
}
QDEFUN( IpAddrMask_voice_if_Func,
        IpAddrMask_voice_if_CMD,
        "voice ip address <A.B.C.D> <A.B.C.D> vlan <1-4094>" ,
       "Config voice module information\n"
        IP_STR
        IPADDR_STR
        ABCD_STR
        MASK_STR
        "Vlan interface\n"
        "Vlan id\n",
        &g_ulIFMQue )
{
	return onu_gt831_cli_process(vty, "voice ip address");
}
QDEFUN( IpAddr_voice_vlan_if_dhcpc_Func,
        IpAddr_voice_vlan_if_dhcpc_CMD,
        "voice ip address dhcp vlan <1-4094>" ,
        "Config voice module information\n"
        IP_STR
        IPADDR_STR
        "IP address from dhcp server\n"
	"Vlan interface\n"
	"Vlan id\n",
        &g_ulIFMQue )
{
	return onu_gt831_cli_process(vty, "voice ip address dhcp vlan");
}

/* PR 13624 BEGIN */
LDEFUN( ldef_IpAddr_voice_if_Func,
        IpAddr_voice_if_CMD,
        "voice ip address <A.B.C.D/M> vlan <1-4094>" ,
        "Config voice module information\n"
        IP_STR
        IPADDR_STR
        ABCDM_STR
        "Vlan interface\n"
        "Vlan id\n",
	ONU_NODE)

{
    return CMD_SUCCESS;
}
LDEFUN( ldef_IpAddrMask_voice_if_Func,
        IpAddrMask_voice_if_CMD,
        "voice ip address <A.B.C.D> <A.B.C.D> vlan <1-4094>" ,
       "Config voice module information\n"
        IP_STR
        IPADDR_STR
        ABCD_STR
        MASK_STR
        "Vlan interface\n"
        "Vlan id\n",
	ONU_NODE)

{
    return CMD_SUCCESS;
}
LDEFUN( ldef_IpAddr_voice_vlan_if_dhcpc_Func,
        IpAddr_voice_vlan_if_dhcpc_CMD,
        "voice ip address dhcp vlan <1-4094>" ,
        "Config voice module information\n"
        IP_STR
        IPADDR_STR
        "IP address from dhcp server\n"
	"Vlan interface\n"
	"Vlan id\n",
	ONU_NODE)

{
    return CMD_SUCCESS;
}
/* PR 13624 END */

DEFUN ( onu_cli_ping_cmd_func,
        onu_cli_ping_cmd,
        "ping {[-t]}*1 {[-count] <1-65535>}*1 {[-size] <8-6400>}*1 {[-waittime] <1-255>}*1 {[-i] <1-255>}*1 {[-pattern] <user_pattern>}*1 {[-source] <A.B.C.D>}*1 [<hostname>|<A.B.C.D>]",
        "Ping command to test if the net is correct\n"
        "Ping destination address until stopped,to stop type ctrl-c\n"
        "Specify the number of echo requests to send(default is 5)\n"
        "Input the number(1-65535)\n"
        "Specify the size of ping icmp packet\n"
        "Input the ping packet's size(8-6400), not include the icmp header length(8)\n"
        "Specify the timeout in seconds to wait for each reply\n"
        "Input the timeout value in seconds(1-255)\n"
        "Specify the time to live value\n"
        "Input the ttl value(1-255)\n"
        "Specify your own data that should be included in ping icmp packet\n"
        "Input your own data here(1-16 hex digits)\n"
        "Specify the source ip address\n"
        "Input the source ip address\n"
        "Input host name that you want to ping\n"
        "Input the destination ip address that you want to ping\n" )
{
	return onu_gt831_cli_process(vty, "ping");
}

DEFUN ( show_h248_termination_status_cmd_func,
       show_h248_termination_status_cmd,
	"show h248 termination-status",
	"show h248 termination-status\n"
	"show h248 termination-status\n"
	"show h248 termination-status\n")
{
	return onu_gt831_cli_process(vty, "show h248 termination-status");
}
DEFUN ( h248_servicechange_cmd_func,
       h248_servicechange_cmd,
	"h248 servicechange",
	"h248 servicechange\n"
	"h248 servicechange\n")
{
	return onu_gt831_cli_process(vty, "h248 servicechange");
}


LDEFUN ( onu_cli_ping_cmd_func,
        onu_cli_ping_cmd,
        "ping {[-t]}*1 {[-count] <1-65535>}*1 {[-size] <8-6400>}*1 {[-waittime] <1-255>}*1 {[-i] <1-255>}*1 {[-pattern] <user_pattern>}*1 {[-source] <A.B.C.D>}*1 [<hostname>|<A.B.C.D>]",
        "Ping command to test if the net is correct\n"
        "Ping destination address until stopped,to stop type ctrl-c\n"
        "Specify the number of echo requests to send(default is 5)\n"
        "Input the number(1-65535)\n"
        "Specify the size of ping icmp packet\n"
        "Input the ping packet's size(8-6400), not include the icmp header length(8)\n"
        "Specify the timeout in seconds to wait for each reply\n"
        "Input the timeout value in seconds(1-255)\n"
        "Specify the time to live value\n"
        "Input the ttl value(1-255)\n"
        "Specify your own data that should be included in ping icmp packet\n"
        "Input your own data here(1-16 hex digits)\n"
        "Specify the source ip address\n"
        "Input the source ip address\n"
        "Input host name that you want to ping\n"
        "Input the destination ip address that you want to ping\n",
	ONU_NODE )
{
	return CMD_SUCCESS;
}

LDEFUN ( show_h248_termination_status_cmd_func,
       show_h248_termination_status_cmd,
	"show h248 termination-status",
	"show h248 termination-status\n"
	"show h248 termination-status\n"
	"show h248 termination-status\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN ( h248_servicechange_cmd_func,
       h248_servicechange_cmd,
	"h248 servicechange",
	"h248 servicechange\n"
	"h248 servicechange\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}

	
int onu_gt831_init_func()
{
    return VOS_OK;
}

int onu_gt831_showrun( struct vty * vty )
{    
    return VOS_OK;
}


int onu_gt831_config_write ( struct vty * vty )
{
    return VOS_OK;
}

struct cmd_node onu_gt831_node =
{
    ONU_GT821_GT831_NODE,
    NULL,
    1
};

LONG onu_gt831_node_install()
{
    install_node( &onu_gt831_node, onu_gt831_config_write);
    onu_gt831_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_ONU);
    if ( !onu_gt831_node.prompt )
    {
        ASSERT( 0 );
        return (-IFM_E_NOMEM);
    }
    install_default( ONU_GT821_GT831_NODE );
    return VOS_OK;
}


LONG onu_gt831_module_init()
{
    struct cl_cmd_module * onu_gt831_module = NULL;

    onu_gt831_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_ONU);
    if ( !onu_gt831_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) onu_gt831_module, sizeof( struct cl_cmd_module ) );

    onu_gt831_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_ONU);
    if ( !onu_gt831_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( onu_gt831_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( onu_gt831_module->module_name, "onu_gt831" );

    onu_gt831_module->init_func = onu_gt831_init_func;
    onu_gt831_module->showrun_func = /*onu_gt813_showrun*/NULL;
    onu_gt831_module->next = NULL;
    onu_gt831_module->prev = NULL;

    cl_install_module( onu_gt831_module );
  
    return VOS_OK;
}

int onu_gt831b_init_func()
{
    return VOS_OK;
}

int onu_gt831b_showrun( struct vty * vty )
{    
    return VOS_OK;
}


int onu_gt831b_config_write ( struct vty * vty )
{
    return VOS_OK;
}

struct cmd_node onu_gt831b_node =
{
    ONU_GT831B_NODE,
    NULL,
    1
};

LONG onu_gt831b_node_install()
{
    install_node( &onu_gt831b_node, onu_gt831b_config_write);
    onu_gt831b_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_ONU);
    if ( !onu_gt831b_node.prompt )
    {
        ASSERT( 0 );
        return (-IFM_E_NOMEM);
    }
    install_default( ONU_GT831B_NODE );
    return VOS_OK;
}


LONG onu_gt831b_module_init()
{
    struct cl_cmd_module * onu_gt831b_module = NULL;

    onu_gt831b_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_ONU);
    if ( !onu_gt831b_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) onu_gt831b_module, sizeof( struct cl_cmd_module ) );

    onu_gt831b_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_ONU);
    if ( !onu_gt831b_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( onu_gt831b_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( onu_gt831b_module->module_name, "onu_gt831b" );

    onu_gt831b_module->init_func = onu_gt831b_init_func;
    onu_gt831b_module->showrun_func = /*onu_gt813_showrun*/NULL;
    onu_gt831b_module->next = NULL;
    onu_gt831b_module->prev = NULL;

    cl_install_module( onu_gt831b_module );
  
    return VOS_OK;
}


LONG  OnuGT831_821CommandInstall_Ldefun( enum node_type  node )
{
	if( node == ONU_GT821_GT831_NODE )
	{
		onu_gt831_node_install();
	    	onu_gt831_module_init();
	}
	else if( node == ONU_GT831B_NODE )
	{
		onu_gt831b_node_install();
	    	onu_gt831b_module_init();
	}
	else
	{
		return VOS_ERROR;
	}
	 
	/* added by zhengyt 2007-7-5 */
	/* the following CLI command is for GT831&GT821 ONU VOIP*/
	install_element ( node, &ldef_config_cmd2lic_mgcp_show_mgcp_config_cmd );
	install_element ( node, &ldef_config_cmd2lic_mgcp_acl_config_cmd );
	install_element ( node, &ldef_config_cmd2lic_mgcp_no_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_authentication_mac_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_authentication_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_heartbeat_timer_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_heartbeat_type_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_mg_domainname_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_mg_localport_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_mg_localname_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_mgc_addr_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_mgc_port_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_mgc_type_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_ncs_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_persist_event_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_quarantine_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_wildcard_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_mgcp_regstatus_show_cmd);
	install_element ( node, &ldef_config_cmd2lic_rtp_show_rtp_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_rtp_start_port_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_ept_hookflash_cmd);
	install_element ( node, &ldef_config_cmd2lic_ept_tone_test_cmd);
	install_element ( node, &ldef_config_cmd2lic_ept_vad_enable_cmd);
	install_element ( node, &ldef_config_cmd2lic_ept_dtmf_relay_enable_cmd);
	install_element ( node, &ldef_config_cmd2lic_ept_ecan_enable_cmd);
	install_element ( node, &ldef_config_cmd2lic_ept_gain_control_cmd);
	install_element ( node, &ldef_config_cmd2lic_ept_fax_mode_cmd);
	install_element ( node, &ldef_config_cmd2lic_show_sip_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_display_name_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_user_name_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_password_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_account_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_reg_realm_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_reg_port_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_outbound_proxy_address_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_outbound_proxy_port_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_server_address_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_server_port_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_local_port_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_expires_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_show_sip_phone_reg_status_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_dial_plan_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_no_dial_plan_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_user_agent_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_show_dial_plan_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_show_dialogs_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_show_transactions_cmd);
	install_element ( node, &ldef_config_cmd2lic_iad_service_select_cmd);
	install_element ( node, &ldef_config_cmd2lic_iad_show_current_protocol_cmd);
	install_element ( node, &ldef_config_cmd2lic_cli_config_voice_dns_server_cmd);
	install_element ( node, &ldef_config_cmd2lic_cli_config_voice_dns_server_del_cmd);
	install_element ( node, &ldef_config_cmd2lic_serv_voice_dns_enable_cmd);
	install_element ( node, &ldef_config_cmd2lic_cli_show_voice_dns_client_info_cmd);
	install_element ( node, &ldef_config_cmd2lic_IpAddr_voice_if_dhcpc_cmd);
	install_element ( node, &ldef_config_cmd2lic_NoIpAddr_voice_if_dhcpc_cmd);
	install_element ( node, &ldef_config_cmd2lic_IpAddr_voice_if_cmd);
	install_element ( node, &ldef_config_cmd2lic_IpAddrMask_voice_if_cmd);
	install_element ( node, &ldef_config_cmd2lic_NoIpAddr_voice_if_cmd);
	install_element ( node, &ldef_config_cmd2lic_IpAddr_voice_ip_show_cmd);
	install_element ( node, &ldef_config_cmd2lic_show_vocie_ip_gateway_cmd);
	install_element ( node, &ldef_config_cmd2lic_static_voice_ipgateway_cmd);
	install_element ( node, &ldef_config_cmd2lic_static_no_voice_ipgateway_cmd);
/*	install_element ( node, &config_cmd2lic_h248_local_port_config_cmd);
	install_element ( node, &config_cmd2lic_h248_mgc_port_config_cmd);
	install_element ( node, &config_cmd2lic_h248_mgc_address_config_cmd);
	install_element ( node, &config_cmd2lic_h248_router_address_config_cmd);
	install_element ( node, &config_cmd2lic_h248_phone_ext_number_config_cmd);*/
	install_element ( node, &ldef_config_cmd2lic_voice_task_stack_cmd);
	install_element ( node, &ldef_config_cmd2lic_slab_show_cmd);
	install_element ( node, &ldef_config_cmd2lic_slab_mod_show_cmd);
	 if( node != ONU_GT831B_NODE )/*问题单7746*/
	install_element ( node, &ldef_config_cmd2lic_vos_system_usage_show_cmd);
	install_element ( node, &ldef_config_cmd2lic_voice_task_list_cmd);
	install_element ( node, &ldef_task_list_cmd_voip);
	install_element ( node, &ldef_config_cmd2lic_task_stack_cmd);
	install_element ( node, &ldef_config_catv_cmd);
	/*install_element ( node, &onu_config_igmpsnoop_auth_cmd );*/
	 if( node != ONU_GT831B_NODE )/*问题单7746*/
	install_element ( node, &ldef_show_voice_info_cmd );
	 
	install_element ( node, &ldef_config_cmd2lic_mgcp_heartbeat_enable_config_cmd );
	install_element ( node, &ldef_onu_slab_show_cmd );
 	install_element ( node, &ldef_onu_voice_slab_mod_show_cmd );

	install_element ( node, &ldef_config_cmd2lic_ept_show_cmd);
	install_element ( node, &ldef_config_cmd2lic_ept_prefer_codec_cmd);
	install_element ( node, &ldef_config_cmd2lic_ept_codec_show_cmd);
	install_element ( node, &ldef_config_cmd2lic_call_status_show_cmd);
	install_element ( node, &ldef_config_cmd2lic_phone_status_show_cmd);
	install_element ( node, &ldef_config_cmd2lic_codec_statistics_show_cmd);
	install_element ( node, &ldef_config_cmd2lic_codec_statistics_clear_cmd);
	install_element ( node, &ldef_config_cmd2lic_sip_user_agent_show_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_user_name_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_password_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_id_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_gatekeeper_address_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_gatekeeper_discover_mode_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_gatekeeper_port_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_gatekeeper_security_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_fast_startup_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_h245_tunnel_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_dtmf_relay_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_show_h323_phone_reg_status_cmd);
	install_element ( node, &ldef_config_cmd2lic_show_h323_config_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_dial_plan_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_no_dial_plan_cmd);
	install_element ( node, &ldef_config_cmd2lic_h323_show_dial_plan_cmd);
	install_element ( node, &ldef_config_cmd2lic_show_voice_service_cmd);
	if( node == ONU_GT821_GT831_NODE )
	{
		install_element ( node, &ldef_config_cmd2lic_voice_service_cmd);
		install_element ( node, &ldef_config_cmd2lic_voice_service2_cmd);
	}
	install_element ( node, &ldef_config_cmd2lic_voice_syslog_start_cmd);
	install_element ( node, &ldef_config_cmd2lic_ShowSyslogConfServer_CMD);
	install_element ( node, &ldef_config_cmd2lic_ShowSyslogConf_CMD);

	install_element ( node, &ldef_config_cmd2lic_ShowRun_CMD );
	install_element ( node, &ldef_config_cmd2lic_show_voice_startup_config_cmd );

	install_element ( node, &ldef_config_cmd2lic_sip_dns_srv_config_cmd );
	install_element ( node, &ldef_config_cmd2lic_sip_add_del_dial_plan_cmd );
	install_element ( node, &ldef_config_cmd2lic_sip_buildin_dial_plan_cmd );

	/*  2008-3-28
	 按张新辉要求,在GT831节点下,增加如下命令*/
	install_element ( node, &ldef_onu_syslog_show_cmd );
	install_element ( node, &ldef_config_cmd2lic_voice_syslog_show_cmd );
	install_element ( node, &ldef_show_voice_exception_syslog_cmd );
	install_element ( node, &ldef_show_exception_syslog_cmd );
	install_element ( node, &ldef_config_cmd2lic_sip_provisional_reliable_cmd );
	install_element ( node, &ldef_config_cmd2lic_sip_registar_cid_change_cmd );
	install_element ( node, &ldef_config_cmd2lic_voice_stat_show_cmd );
	install_element ( node, &ldef_onu_event_clear_cmd );

	if( node != ONU_GT831B_NODE )	/* modified by xieshl 20090505, 问题单7734 */
	{
		install_element ( node, &ldef_config_cmd2lic_sip_supplementary_service_cmd );
	}
	else
	{
		install_element ( node, &ldef_h248_codec_config_cmd);
		install_element ( node, &ldef_h248_mgheartbeat_cmd);
		install_element ( node, &ldef_h248_heartbeatimer_config_cmd);
		install_element ( node, &ldef_h248_local_domain_config_cmd);
		install_element ( node, &ldef_h248_local_name_config_cmd);
		install_element ( node, &ldef_h248_local_port_config_cmd);
		install_element ( node, &ldef_h248_local_rtpprefix_config_cmd);
		install_element ( node, &ldef_h248_mgc_address_config_cmd);
		install_element ( node, &ldef_h248_mgc_port_config_cmd);
		install_element ( node, &ldef_h248_mgcbackup_address_cmd);
		install_element ( node, &ldef_h248_mgcbackup_port_cmd);
		install_element ( node, &ldef_h248_registermode_config_cmd);
		install_element ( node, &ldef_h248_reporthook_cmd);
		install_element ( node, &ldef_h248_srvchng_stat_cmd);
		install_element ( node, &ldef_h248_startup_cmd);
		install_element ( node, &ldef_h248_ctx_disp_cmd);
		install_element ( node, &ldef_sip_show_dialogs_cmd);
		/*install_element ( node, &sip_show_transactions_cmd);
		install_element ( node, &sip_dns_srv_config_cmd);
		install_element ( node, &sip_provisional_reliable_cmd);*/
		install_element ( node, &ldef_sip_supplementary_service_cmd);
		install_element ( node, &ldef_voice_service_cmd);
		install_element ( node, &ldef_voice_service2_cmd);
		/*install_element ( node, &show_voice_service_cmd); */
		install_element ( node, &ldef_sip_match_dial_plan_cmd);
		install_element ( node, &ldef_sip_re_register_cmd);
		install_element ( node, &ldef_slic_ac_profile_rd_cmd);
		install_element ( node, &ldef_slic_impedance_cmd);
#if 0 /* zhangxinhui 2011-10-13 PR 13624 */		
	       install_element ( node, &IpAddr_voice_if_CMD);/***QDEFUN****/
	       install_element ( node, &IpAddrMask_voice_if_CMD);/****QDEFUN***/
		install_element ( node, &IpAddr_voice_vlan_if_dhcpc_CMD);/***QDEFUN****/
#else		
		LDEFUN_INSTALL_ELEMENT ( node, IpAddr_voice_if_CMD);
		LDEFUN_INSTALL_ELEMENT ( node, IpAddrMask_voice_if_CMD);
		LDEFUN_INSTALL_ELEMENT ( node, IpAddr_voice_vlan_if_dhcpc_CMD);
#endif		
		install_element ( node, &ldef_onu_cli_ping_cmd);

		install_element ( node, &ldef_show_h248_termination_status_cmd);
		install_element ( node, &ldef_h248_servicechange_cmd);

	}
	return VOS_OK;
}


LONG  OnuGT831_821CommandInstall( enum node_type  node )
{
	if( node == ONU_GT821_GT831_NODE )
	{
		onu_gt831_node_install();
	    	onu_gt831_module_init();
	}
	else if( node == ONU_GT831B_NODE )
	{
		onu_gt831b_node_install();
	    	onu_gt831b_module_init();
	}
	else
	{
		return VOS_ERROR;
	}
	 
	/* added by zhengyt 2007-7-5 */
	/* the following CLI command is for GT831&GT821 ONU VOIP*/
	install_element ( node, &config_cmd2lic_mgcp_show_mgcp_config_cmd );
	install_element ( node, &config_cmd2lic_mgcp_acl_config_cmd );
	install_element ( node, &config_cmd2lic_mgcp_no_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_authentication_mac_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_authentication_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_heartbeat_timer_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_heartbeat_type_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_mg_domainname_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_mg_localport_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_mg_localname_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_mgc_addr_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_mgc_port_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_mgc_type_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_ncs_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_persist_event_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_quarantine_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_wildcard_config_cmd);
	install_element ( node, &config_cmd2lic_mgcp_regstatus_show_cmd);
	install_element ( node, &config_cmd2lic_rtp_show_rtp_config_cmd);
	install_element ( node, &config_cmd2lic_rtp_start_port_config_cmd);
	install_element ( node, &config_cmd2lic_ept_hookflash_cmd);
	install_element ( node, &config_cmd2lic_ept_tone_test_cmd);
	install_element ( node, &config_cmd2lic_ept_vad_enable_cmd);
	install_element ( node, &config_cmd2lic_ept_dtmf_relay_enable_cmd);
	install_element ( node, &config_cmd2lic_ept_ecan_enable_cmd);
	install_element ( node, &config_cmd2lic_ept_gain_control_cmd);
	install_element ( node, &config_cmd2lic_ept_fax_mode_cmd);
	install_element ( node, &config_cmd2lic_show_sip_config_cmd);
	install_element ( node, &config_cmd2lic_sip_display_name_config_cmd);
	install_element ( node, &config_cmd2lic_sip_user_name_config_cmd);
	install_element ( node, &config_cmd2lic_sip_password_config_cmd);
	install_element ( node, &config_cmd2lic_sip_account_config_cmd);
	install_element ( node, &config_cmd2lic_sip_reg_realm_config_cmd);
	install_element ( node, &config_cmd2lic_sip_reg_port_config_cmd);
	install_element ( node, &config_cmd2lic_sip_outbound_proxy_address_config_cmd);
	install_element ( node, &config_cmd2lic_sip_outbound_proxy_port_config_cmd);
	install_element ( node, &config_cmd2lic_sip_server_address_config_cmd);
	install_element ( node, &config_cmd2lic_sip_server_port_config_cmd);
	install_element ( node, &config_cmd2lic_sip_local_port_config_cmd);
	install_element ( node, &config_cmd2lic_sip_expires_config_cmd);
	install_element ( node, &config_cmd2lic_show_sip_phone_reg_status_cmd);
	install_element ( node, &config_cmd2lic_sip_dial_plan_cmd);
	install_element ( node, &config_cmd2lic_sip_no_dial_plan_cmd);
	install_element ( node, &config_cmd2lic_sip_user_agent_config_cmd);
	install_element ( node, &config_cmd2lic_sip_show_dial_plan_cmd);
	install_element ( node, &config_cmd2lic_sip_show_dialogs_cmd);
	install_element ( node, &config_cmd2lic_sip_show_transactions_cmd);
	install_element ( node, &config_cmd2lic_iad_service_select_cmd);
	install_element ( node, &config_cmd2lic_iad_show_current_protocol_cmd);
	install_element ( node, &config_cmd2lic_cli_config_voice_dns_server_cmd);
	install_element ( node, &config_cmd2lic_cli_config_voice_dns_server_del_cmd);
	install_element ( node, &config_cmd2lic_serv_voice_dns_enable_cmd);
	install_element ( node, &config_cmd2lic_cli_show_voice_dns_client_info_cmd);
	install_element ( node, &config_cmd2lic_IpAddr_voice_if_dhcpc_cmd);
	install_element ( node, &config_cmd2lic_NoIpAddr_voice_if_dhcpc_cmd);
	install_element ( node, &config_cmd2lic_IpAddr_voice_if_cmd);
	install_element ( node, &config_cmd2lic_IpAddrMask_voice_if_cmd);
	install_element ( node, &config_cmd2lic_NoIpAddr_voice_if_cmd);
	install_element ( node, &config_cmd2lic_IpAddr_voice_ip_show_cmd);
	install_element ( node, &config_cmd2lic_show_vocie_ip_gateway_cmd);
	install_element ( node, &config_cmd2lic_static_voice_ipgateway_cmd);
	install_element ( node, &config_cmd2lic_static_no_voice_ipgateway_cmd);
/*	install_element ( node, &config_cmd2lic_h248_local_port_config_cmd);
	install_element ( node, &config_cmd2lic_h248_mgc_port_config_cmd);
	install_element ( node, &config_cmd2lic_h248_mgc_address_config_cmd);
	install_element ( node, &config_cmd2lic_h248_router_address_config_cmd);
	install_element ( node, &config_cmd2lic_h248_phone_ext_number_config_cmd);*/
	install_element ( node, &config_cmd2lic_voice_task_stack_cmd);
	install_element ( node, &config_cmd2lic_slab_show_cmd);
	install_element ( node, &config_cmd2lic_slab_mod_show_cmd);
	 if( node != ONU_GT831B_NODE )/*问题单7746*/
	install_element ( node, &config_cmd2lic_vos_system_usage_show_cmd);
	install_element ( node, &config_cmd2lic_voice_task_list_cmd);
	install_element ( node, &task_list_cmd_voip);
	install_element ( node, &config_cmd2lic_task_stack_cmd);
	install_element ( node, &config_catv_cmd);
	/*install_element ( node, &onu_config_igmpsnoop_auth_cmd );*/
	 if( node != ONU_GT831B_NODE )/*问题单7746*/
	install_element ( node, &show_voice_info_cmd );
	 
	install_element ( node, &config_cmd2lic_mgcp_heartbeat_enable_config_cmd );
	install_element ( node, &onu_slab_show_cmd );
 	install_element ( node, &onu_voice_slab_mod_show_cmd );

	install_element ( node, &config_cmd2lic_ept_show_cmd);
	install_element ( node, &config_cmd2lic_ept_prefer_codec_cmd);
	install_element ( node, &config_cmd2lic_ept_codec_show_cmd);
	install_element ( node, &config_cmd2lic_call_status_show_cmd);
	install_element ( node, &config_cmd2lic_phone_status_show_cmd);
	install_element ( node, &config_cmd2lic_codec_statistics_show_cmd);
	install_element ( node, &config_cmd2lic_codec_statistics_clear_cmd);
	install_element ( node, &config_cmd2lic_sip_user_agent_show_cmd);
	install_element ( node, &config_cmd2lic_h323_user_name_config_cmd);
	install_element ( node, &config_cmd2lic_h323_password_config_cmd);
	install_element ( node, &config_cmd2lic_h323_id_config_cmd);
	install_element ( node, &config_cmd2lic_h323_gatekeeper_address_config_cmd);
	install_element ( node, &config_cmd2lic_h323_gatekeeper_discover_mode_config_cmd);
	install_element ( node, &config_cmd2lic_h323_gatekeeper_port_config_cmd);
	install_element ( node, &config_cmd2lic_h323_gatekeeper_security_config_cmd);
	install_element ( node, &config_cmd2lic_h323_fast_startup_config_cmd);
	install_element ( node, &config_cmd2lic_h323_h245_tunnel_config_cmd);
	install_element ( node, &config_cmd2lic_h323_dtmf_relay_config_cmd);
	install_element ( node, &config_cmd2lic_show_h323_phone_reg_status_cmd);
	install_element ( node, &config_cmd2lic_show_h323_config_cmd);
	install_element ( node, &config_cmd2lic_h323_dial_plan_cmd);
	install_element ( node, &config_cmd2lic_h323_no_dial_plan_cmd);
	install_element ( node, &config_cmd2lic_h323_show_dial_plan_cmd);
	install_element ( node, &config_cmd2lic_show_voice_service_cmd);
	if( node == ONU_GT821_GT831_NODE )
	{
		install_element ( node, &config_cmd2lic_voice_service_cmd);
		install_element ( node, &config_cmd2lic_voice_service2_cmd);
	}
	install_element ( node, &config_cmd2lic_voice_syslog_start_cmd);
	install_element ( node, &config_cmd2lic_ShowSyslogConfServer_CMD);
	install_element ( node, &config_cmd2lic_ShowSyslogConf_CMD);

	install_element ( node, &config_cmd2lic_ShowRun_CMD );
	install_element ( node, &config_cmd2lic_show_voice_startup_config_cmd );

	install_element ( node, &config_cmd2lic_sip_dns_srv_config_cmd );
	install_element ( node, &config_cmd2lic_sip_add_del_dial_plan_cmd );
	install_element ( node, &config_cmd2lic_sip_buildin_dial_plan_cmd );

	/*  2008-3-28
	 按张新辉要求,在GT831节点下,增加如下命令*/
	install_element ( node, &onu_syslog_show_cmd );
	install_element ( node, &config_cmd2lic_voice_syslog_show_cmd );
	install_element ( node, &show_voice_exception_syslog_cmd );
	install_element ( node, &show_exception_syslog_cmd );
	install_element ( node, &config_cmd2lic_sip_provisional_reliable_cmd );
	install_element ( node, &config_cmd2lic_sip_registar_cid_change_cmd );
	install_element ( node, &config_cmd2lic_voice_stat_show_cmd );
	install_element ( node, &onu_event_clear_cmd );

	if( node != ONU_GT831B_NODE )	/* modified by xieshl 20090505, 问题单7734 */
	{
		install_element ( node, &config_cmd2lic_sip_supplementary_service_cmd );
	}
	else
	{
		install_element ( node, &h248_codec_config_cmd);
		install_element ( node, &h248_mgheartbeat_cmd);
		install_element ( node, &h248_heartbeatimer_config_cmd);
		install_element ( node, &h248_local_domain_config_cmd);
		install_element ( node, &h248_local_name_config_cmd);
		install_element ( node, &h248_local_port_config_cmd);
		install_element ( node, &h248_local_rtpprefix_config_cmd);
		install_element ( node, &h248_mgc_address_config_cmd);
		install_element ( node, &h248_mgc_port_config_cmd);
		install_element ( node, &h248_mgcbackup_address_cmd);
		install_element ( node, &h248_mgcbackup_port_cmd);
		install_element ( node, &h248_registermode_config_cmd);
		install_element ( node, &h248_reporthook_cmd);
		install_element ( node, &h248_srvchng_stat_cmd);
		install_element ( node, &h248_startup_cmd);
		install_element ( node, &h248_ctx_disp_cmd);
		install_element ( node, &sip_show_dialogs_cmd);
		/*install_element ( node, &sip_show_transactions_cmd);
		install_element ( node, &sip_dns_srv_config_cmd);
		install_element ( node, &sip_provisional_reliable_cmd);*/
		install_element ( node, &sip_supplementary_service_cmd);
		install_element ( node, &voice_service_cmd);
		install_element ( node, &voice_service2_cmd);
		/*install_element ( node, &show_voice_service_cmd); */
		install_element ( node, &sip_match_dial_plan_cmd);
		install_element ( node, &sip_re_register_cmd);
		install_element ( node, &slic_ac_profile_rd_cmd);
		install_element ( node, &slic_impedance_cmd);
	       install_element ( node, &IpAddr_voice_if_CMD);/***QDEFUN****/
	       install_element ( node, &IpAddrMask_voice_if_CMD);/****QDEFUN***/
		install_element ( node, &IpAddr_voice_vlan_if_dhcpc_CMD);/***QDEFUN****/
		install_element ( node, &onu_cli_ping_cmd);

		install_element ( node, &show_h248_termination_status_cmd);
		install_element ( node, &h248_servicechange_cmd);

	}
	return VOS_OK;
}

#endif


