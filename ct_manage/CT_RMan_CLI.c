#include  "OltGeneral.h"
#ifdef CTC_OBSOLETE		/* removed by xieshl 20120607 */
#if 0
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#include "CT_RMan_Main.h"


/*----------------------------------------------------------------------------*/
extern enum match_type IFM_CheckSlotPortOnu( char * cPort );
LONG CT_Pon_Init();
extern LONG CT_RMan_ONU_Init();
extern LONG CT_RMan_ONU_QoS_Init();
extern LONG CT_RMan_EthPort_Init( void );

extern LONG PON_GetSlotPortOnu( ULONG ulIfIndex, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);


int CT_RMan_CLI_Init ()
{
	/*CT_RMan_Node_Install();
	CT_RMan_Module_Init();*/
	
	CT_Pon_Init();

	CT_RMan_ONU_Init();
	CT_RMan_ONU_QoS_Init();
	CT_RMan_EthPort_Init();

    CT_EthCli_Init();
    CT_VlanCli_Init();
   CT_RMan_Multicast_Command_Install();

   /*print_tdm_Init();*//*add by shixh@20071128*/

	return VOS_OK;
}

int parse_onu_devidx_command_parameter( struct vty *vty, ULONG *devIdx, ULONG *slotno, ULONG *pon, ULONG *onu)
{
    ULONG ulIfIndex = 0;	

 	if( devIdx == NULL || slotno==NULL || pon == NULL || onu == NULL )
		return VOS_ERROR;
	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	if( PON_GetSlotPortOnu( ulIfIndex, slotno, pon, onu) == VOS_OK )
	{
		*devIdx = MAKEDEVID(*slotno, *pon, *onu);
		return VOS_OK;
	}
	return VOS_ERROR;
}

int parse_onu_command_parameter( struct vty *vty, PON_olt_id_t *pPonPortIdx, PON_onu_id_t *pOnuIdx, PON_onu_id_t *pOnuId)
{
	ULONG ulSlot, ulPort, ulOnu;
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id;
	int err_rc = VOS_ERROR;
	
	if( (pPonPortIdx == NULL) || (pOnuId == NULL) )
	{
		VOS_ASSERT(0);
    		return err_rc;
	}

	if( PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnu ) != VOS_OK )
	{
		vty_out( vty, "  PON_GetSlotPortOnu error\r\n");
    		return err_rc;
	}
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (olt_id == err_rc )
	{
		vty_out( vty, "  GetPonPortIdxBySlot error\r\n");
    		return err_rc;
	}

	/*if( !PonPortIsWorking(olt_id) )
	{
		vty_out( vty, "  PonPortIsWorking error\r\n");
    		return err_rc;
	}*/

	onu_id = ulOnu - 1;
	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
		vty_out( vty, "  onu is off-line\r\n");
    		return err_rc;
	}

	if( pOnuIdx )  *pOnuIdx = onu_id;
	
	onu_id = GetLlidByOnuIdx( olt_id, onu_id );
	if( onu_id == -1 )
	{
		vty_out( vty, "  GetLlidByOnuIdx error\r\n");
		return VOS_ERROR;
	}

	*pPonPortIdx = olt_id;
	*pOnuId = onu_id;
	
	return VOS_OK;
}

#if 0
LONG CT_RMan_Node_Install();
LONG CT_RMan_Module_Init();
struct cmd_node ct_rman_node =
{
	ONU_CTC_NODE,
	NULL,
	1
};

int CT_RMan_config_write ( struct vty * vty )
{
	return VOS_OK;
}
int CT_RMan_Init_Func()
{
	return VOS_OK;
}

int CT_RMan_ShowRun( struct vty * vty )
{
	 /*IFM_READ_LOCK;
	CT_RMan_show_run( 0, vty );
	IFM_READ_UNLOCK;*/
    
	return VOS_OK;
}



LONG CT_RMan_Node_Install()
{
	install_node( &ct_rman_node, CT_RMan_config_write);
	ct_rman_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_CT_EXT);
	if( !ct_rman_node.prompt )
	{
		ASSERT( 0 );
		return -IFM_E_NOMEM;
	}
	install_default( ONU_CTC_NODE );
	return VOS_OK;
}

LONG CT_RMan_Module_Init()
{
    struct cl_cmd_module * ct_rman_module = NULL;

    ct_rman_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_CT_EXT);
    if ( !ct_rman_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) ct_rman_module, sizeof( struct cl_cmd_module ) );

    ct_rman_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_CT_EXT);
    if ( !ct_rman_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( ct_rman_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( ct_rman_module->module_name, "ont" );

    ct_rman_module->init_func = CT_RMan_Init_Func;
    ct_rman_module->showrun_func = CT_RMan_ShowRun;
    ct_rman_module->next = NULL;
    ct_rman_module->prev = NULL;

    cl_install_module( ct_rman_module );

    return VOS_OK;
}

DEFUN  (
    test_ctc_node,
    test_ctc_node_cmd,
/*    "test mode [enable|disable] {number <1-32>}*1 records {<value>}*32",*/
    "test cmd [enable|disable] records {<value>}*32",
    "Test CLI\n"
    "Command params\n"
    "Enable\n"
    "Disable\n"
    "Display records"
    "records list\n" )
{
	int i;
	int num;
	if( argc < 1 )
		return CMD_WARNING;
	num = argc - 1;
	
	vty_out( vty, "  argc = %d\r\n", argc );
	vty_out( vty, "mode : %s\r\n", argv[0] );

	vty_out( vty, "record number %d, list :\r\n", num );
	for( i=1; i<=num; i++ )
	{
		vty_out( vty, "  %s\r\n", argv[i] );
	}
	
	return CMD_SUCCESS;
}

DEFUN  (
    test_ctc_pause_node,
    test_ctc_node_pause_cmd,
    "test cmd-param [enable|disable] {port [all|<1-24>]}*1",
    "Test CLI\n"
    "Command params\n"
    "Enable\n"
    "Disable\n"
    "Display port\n"
    "All ports\n"
    "Port Number\n" )
{
	int i;
	vty_out( vty, "  argc = %d\r\n", argc );
	for( i=0; i<argc; i++ )
		vty_out( vty,  "   %s\r\n", argv[i] );
	
	return CMD_SUCCESS;
}


DEFUN  (
	root_into_ctc_onu_node,
	root_into_ctc_onu_node_cmd,
	"ont <slot/port/onuid>",
	"Select an onu to config\n"
	"Specify onu interface's onuid\n"
	)
{
	CHAR 	ifName[IFM_NAME_SIZE + 1] = {0};
	ULONG   ulIFIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	CHAR    prompt[64] = { 0 };
	LONG	lRet;
	INT16 phyPonId = 0;
	int result = 0;	
	VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
	VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%s", "ont", argv[0] );

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	/*added by wutongwu at 18 October*/
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );

	if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )
	{
		vty_out( vty, "  %% Can not find slot %d. \r\n",ulSlot);
		return CMD_WARNING;
	}
	if ( (ulPort < 1) || (ulPort > 4) )
	{
		vty_out( vty, "  %% no exist port %d/%d. \r\n",ulSlot, ulPort);
			return CMD_WARNING;
	}

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Olt not is exist.\r\n" );
		return CMD_WARNING;
	}	
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
	{
		vty_out(vty, "  %d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}

	if ((ulOnuid<1) || (ulOnuid>64))
	{
       	vty_out( vty, "  %% onuid error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
	}

	/*onu`s status of running. when status is not registered. Cann`t into the node!*/	
	lRet = 	GetOnuOperStatus( phyPonId, ulOnuid-1);
	if( (1 != lRet) && (2!= lRet))
	{
		vty_out( vty, "  %% %d/%d/%d is not exist.\r\n",ulSlot, ulPort, ulOnuid-1) ;
		return CMD_WARNING;
	}

	ulIFIndex = IFM_PON_CREATE_INDEX( ulSlot, ulPort, ulOnuid, 0);
	if ( ulIFIndex == 0 )
	{
		vty_out( vty, "%% Can not find interface %s.\r\n", ifName );
		return CMD_WARNING;
	}
	vty->prev_node = vty->node;
	vty->node = ONU_CTC_NODE;
	vty->index = ( VOID * ) ulIFIndex;
    
	VOS_StrCpy( prompt, "%s(epon-" );
	VOS_StrCat( prompt, ifName );
	VOS_StrCat( prompt, ")#" );
	vty_set_prompt( vty, prompt );    
        
	return CMD_SUCCESS;
}


DEFUN  (
    pon_into_ctc_onu_node,
    pon_into_ctc_onu_node_cmd,
    "ont <1-64>",
    "Select an onu to config\n"
    "Specify onu interface's onuid\n"
    )
{
	CHAR 	ifName[IFM_NAME_SIZE + 1] = {0};
    ULONG   ulIFIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    CHAR    prompt[64] = { 0 };
    LONG	lRet;
    
	lRet = PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnuid );
    	if( lRet != VOS_OK )
    		return CMD_WARNING;
		
	ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] ) ;
	
    VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
    VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%d/%d/%d", "ont", ulSlot, ulPort, ulOnuid);

    ulIFIndex = IFM_PON_CREATE_INDEX( ulSlot, ulPort, ulOnuid, 0);
    if ( ulIFIndex == 0 )
    {
        vty_out( vty, "%% Can not find interface %s.\r\n", ifName );
        return CMD_WARNING;
    }
    vty->prev_prev_node = vty->node;
    vty->prev_node = vty->node;
    vty->node = ONU_CTC_NODE;
    vty->index = ( VOID * ) ulIFIndex;
    
    VOS_StrCpy( prompt, "%s(epon-" );
    VOS_StrCat( prompt, ifName );
    VOS_StrCat( prompt, ")#" );
    vty_set_prompt( vty, prompt );    
        
    return CMD_SUCCESS;
}

DEFUN  (
    pon_into_ctc_onu_name_node,
    pon_into_ctc_onu_name_node_cmd,
    "ont <name>",
    "Select an onu to config\n"
    "Specify onu interface's onu name\n"
    )
{
	CHAR 	ifName[IFM_NAME_SIZE + 1] = {0};
    ULONG   ulIFIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
	short int slot=0,port=0;
    CHAR    prompt[64] = { 0 };
    LONG	lRet;
    LONG	len = 0;
	short int onuId = 0;
	lRet = PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnuid );
    	if( lRet != VOS_OK )
    		return CMD_WARNING;
	len = strlen(argv[0]);
	if ( len > 255 )
	{
		vty_out( vty, "  %% Parameter error. Character must be less than 255.\r\n");
		return CMD_WARNING;
	}
	slot = (short int)ulSlot;
	port = (short int)ulPort;

	lRet = GetOnuDeviceIdxByName_OnePon( argv[0], len, (short  int)(ulSlot-1), (short int)(ulPort-1), &onuId );
	if ( 3 == lRet )
	{
		vty_out( vty, "  %% Not exist Onu name!\r\n");
		return CMD_WARNING;
	}
	
	ulOnuid = (ULONG)onuId;

    VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
    VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%d/%d/%d", "ont", ulSlot, ulPort, ulOnuid+1);

    ulIFIndex = IFM_PON_CREATE_INDEX( ulSlot, ulPort, ulOnuid+1, 0);
    if ( ulIFIndex == 0 )
    {
        vty_out( vty, "%% Can not find interface\r\n" );
        return CMD_WARNING;
    }
    vty->prev_prev_node = vty->node;
    vty->prev_node = vty->node;
    vty->node = ONU_CTC_NODE;
    vty->index = ( VOID * ) ulIFIndex;
    
    VOS_StrCpy( prompt, "%s(epon-" );
    VOS_StrCat( prompt, ifName );
    VOS_StrCat( prompt, ")#" );
    vty_set_prompt( vty, prompt );    
        
    return CMD_SUCCESS;
}


DEFUN  (
    config_into_ctc_onu_name_node,
    config_into_ctc_onu_name_node_cmd,
    "ont <name>",
    "Select an onu to config\n"
    "Specify onu interface's onu name\n"
    )
{
	CHAR 	ifName[IFM_NAME_SIZE + 1] = {0};
	CHAR	errInfo[IFM_NAME_SIZE + 1] = {0};
    ULONG   ulIFIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
	short int slot=0,port=0,onuid=0;
    CHAR    prompt[64] = { 0 };
    LONG	lRet;
    LONG	len = 0;
	short int onuId = 0;

	len = strlen(argv[0]);
	if ( len > 255 )
	{
		vty_out( vty, "  %% Parameter error. Character must be less than 255.\r\n");
		return CMD_WARNING;
	}
	
	lRet = GetOnuDeviceIdxByName( argv[0], len, &slot, &port, &onuid );
	if ( 3 == lRet )
	{
		vty_out( vty, "  %% Not exist Onu name!\r\n");
		return CMD_WARNING;
	}
	
	ulOnuid = (ULONG)onuId;
	
	
    VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
    VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%d/%d/%d", "ont", slot+1, port+1, onuid+1);

	ulSlot = slot;
	ulPort = port;
	ulOnuid = onuid;

    ulIFIndex = IFM_PON_CREATE_INDEX( ulSlot+1, ulPort+1, ulOnuid+1, 0);
    if ( ulIFIndex == 0 )
    {
        vty_out( vty, "%% Can not find interface %s.\r\n", errInfo );
        return CMD_WARNING;
    }
    vty->prev_prev_node = vty->node;
    vty->prev_node = vty->node;
    vty->node = ONU_CTC_NODE;
    vty->index = ( VOID * ) ulIFIndex;
    
    VOS_StrCpy( prompt, "%s(epon-" );
    VOS_StrCat( prompt, ifName );
    VOS_StrCat( prompt, ")#" );
    vty_set_prompt( vty, prompt );    
        
    return CMD_SUCCESS;
}

#endif /*__test_ctc*/

DEFUN(
	show_extended_oam_discovery_timing,
	show_extended_oam_discovery_timing_cmd,
	"show ctc extended-oam-discovery-timing",
	SHOW_STR
	CTC_STR
	"Display extended oam discovery timing\n")
{
	PON_STATUS	return_result;
	USHORT		discovery_timeout;
	
	return_result = CTC_STACK_get_extended_oam_discovery_timing(&discovery_timeout);

	if ( return_result == CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "\r\n  extended_oam_discovery_timing: %d\r\n", discovery_timeout );
		return CMD_SUCCESS;
	}
	else
	{
		CTC_STACK_CLI_ERROR_PRINT;
	}
	return CMD_WARNING;
}

DEFUN(
	extended_oam_discovery_timing,
	extended_oam_discovery_timing_cmd,
	"ctc extended-oam-discovery-timing <0-2550>",
	CTC_STR
	"Set extended oam discovery timing parameter\n"
	"Setting discovery timeout measured in 100 ms units\n")
{
	PON_STATUS	return_result;
	ULONG		discovery_timeout;
	
	if( argc != 1 )
	{
		/*vty_out( vty, "  %% Parameter error.\r\n");*/
		return CMD_WARNING;
	}
	discovery_timeout = (USHORT)VOS_AtoL( argv[0] );
	if( discovery_timeout > 2550 )
	{
		/*vty_out( vty, "  %% Parameter error.\r\n");*/
		return CMD_WARNING;
	}

	return_result = CTC_STACK_set_extended_oam_discovery_timing (discovery_timeout);

	if( return_result != CTC_STACK_EXIT_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}


DEFUN(
	show_ctc_oui,
	show_ctc_oui_cmd,
	"show ctc oui",
	SHOW_STR
	CTC_STR
	"Display ctc oui\n")
{
	PON_STATUS	return_result;
	ULONG	oui;

	return_result = CTC_STACK_get_oui( &oui );

	if ( return_result == CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "\r\n  CTC OUI: %06x\r\n", oui );
		return CMD_SUCCESS;
	}
	else
		CTC_STACK_CLI_ERROR_PRINT;

	return CMD_WARNING;
}

DEFUN(
	ctc_oui,
	ctc_oui_cmd,
	"ctc oui <hex_value>",
	CTC_STR
	"Set ctc oui\n"
	"Setting oui value in hex format\n")
{
	PON_STATUS	return_result;
	ULONG	oui;
	char *pToken;
	
	if( argc != 1 )
	{
		/*vty_out( vty, "  %% Parameter error.\r\n");*/
		return CMD_WARNING;
	}

	/*oui = VOS_AtoL( argv[0] );*/	/* modified by xieshl 转换函数错误 */
	oui = VOS_StrToUL( argv[0], &pToken, 16 );

	return_result = CTC_STACK_set_oui (oui);
	
	if( return_result != CTC_STACK_EXIT_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(
	show_ctc_version,
	show_ctc_version_cmd,
	"show ctc version",
	SHOW_STR
	CTC_STR
	"Display ctc version\n")
{
	short int		return_result;
	unsigned char	version;
	
	return_result = CTC_STACK_get_version(&version);

	if ( return_result == CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "  CTC version: %d\r\n", version );
		return CMD_SUCCESS;
	}else
		CTC_STACK_CLI_ERROR_PRINT;

	return CMD_WARNING;
}

DEFUN(
	ctc_version,
	ctc_version_cmd,
	"ctc version <0-255>",
	CTC_STR
	"Set ctc version\n"
	"Setting version value\n")
{
	short int	return_result;
	ULONG	version;
	
	if( argc != 1 )
	{
		/*vty_out( vty, "  %% Parameter error.\r\n");*/
		return CMD_WARNING;
	}
	version = VOS_AtoL( argv[0] );
	if( version > 255 )
	{
		/*vty_out( vty, "  %% Parameter error.\r\n");*/
		return CMD_WARNING;
	}

	return_result = CTC_STACK_set_version( (UCHAR)version);
	
	if( return_result != CTC_STACK_EXIT_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(
	show_ctc_params,
	show_ctc_params_cmd,
	"show ctc params",
	SHOW_STR
	CTC_STR
	"Display ctc discovery and encryption parameters\n")
{
	short int	return_result;
	short int	i;
	UCHAR	automatic_mode, automatic_onu_configuration;
	UCHAR	number_of_records;
	CTC_STACK_oui_version_record_t	records_list[MAX_OUI_RECORDS];
	char tmp_str[20];


	return_result = CTC_STACK_get_parameters(&automatic_mode, &number_of_records, records_list, &automatic_onu_configuration);

	if ( return_result == CTC_STACK_EXIT_OK )
	{
		if (automatic_mode == ENABLE)  
			strcpy(tmp_str,enum_s(enable));
		else if (automatic_mode == DISABLE)
			strcpy(tmp_str,enum_s(disable));
		else 
			strcpy(tmp_str,"Not valid value"); 
		vty_out( vty, "  automatic mode: %s\r\n", tmp_str );

		vty_out( vty, "  number_of_records: %d\r\n", number_of_records );

		if (automatic_onu_configuration == ENABLE)  
			strcpy(tmp_str,enum_s(Enable));
		else if (automatic_onu_configuration == DISABLE)
			strcpy(tmp_str,enum_s(Disable));
		else 
			strcpy(tmp_str,"Not valid value"); 
		vty_out( vty, "  automatic onu configuration: %s\r\n", tmp_str );

		for( i = 0; i < number_of_records; i++)
		{
			vty_out(vty,"  %u: oui 0x%02x%02x%02x version 0x%02x\r\n",i, records_list[i].oui[0],records_list[i].oui[1],records_list[i].oui[2], records_list[i].version);
		}
		return CMD_SUCCESS;
	}
	else
	{
		CTC_STACK_CLI_ERROR_PRINT;
	}

	return CMD_WARNING;
}

/*	"ctc set-params automatic-mode (disable|enable) number_of_records <0-62> oui-records RECORDS_LIST automatic-onu-configuration-mode (disable|enable)",*/
DEFUN(
	ctc_set_params,
	ctc_set_params_cmd,
	"ctc set-params automatic-mode [enable|disable] automatic-onu-configuration-mode [enable|disable] oui-records {<recodes_list>}*32",
	CTC_STR
	"Set discovery and encryption parameters\n"
	"encryption mode\n"
	"enable automatic mode\n"
	"disable automatic mode\n"
	"automatic onu configuration mode"
	"enable: During ONU registering,  to enable all its Eth-ports\n"
	"disable: During ONU registering,  not to enable all its Eth-ports\n"
	"oui records\n"
	"Setting oui records in hex format, and version separated by ;\n")
{
	short int		return_result, i;
	unsigned char automatic_mode, automatic_onu_configuration = 0;
	CTC_STACK_oui_version_record_t  records_list[MAX_OUI_RECORDS];
	unsigned char	number_of_records;
	char			*token, *pToken;
	unsigned long oui_as_number, oui_version;

	if( argc < 3 )
	{
		vty_out( vty, "  %% oui-records can't empty\r\n");
		return CMD_WARNING;
	}

	VOS_MemZero( records_list, sizeof(records_list) );
	if( VOS_StrCmp((CHAR *)argv[0], "enable") == 0 )
		automatic_mode = ENABLE;
	else
		automatic_mode = DISABLE;
	if( VOS_StrCmp((CHAR *)argv[1], "enable") == 0 )
		automatic_onu_configuration = ENABLE;
	else
		automatic_onu_configuration = DISABLE;

	number_of_records = argc - 2;

	for(i = 0; i < number_of_records; i++)
	{
		token = strtok( argv[i+2],  ";" );
		if( token == NULL )
		{
			vty_out(vty," error in reading oui in records string\r\n");
			return CMD_WARNING;
		}
		else
		{
			oui_as_number = VOS_StrToUL( token, &pToken, 16 );
		}

		token = strtok( NULL, ";" );

		if( token == NULL )
		{
			vty_out( vty," error in reading version in records string\r\n" );
			return CMD_WARNING;
		}
		else
		{
			oui_version = VOS_StrToUL( token, &pToken, 16 );
		}

		/*if( oui_as_number > 0xffffff || oui_version > 255 )
		{
			vty_out( vty," OUI or VERSION in records string is too big\r\n" );
			return CMD_WARNING;
		}*/
			
		records_list[i].oui[0]   = (unsigned char)((oui_as_number >> 16) & 0xff);
		records_list[i].oui[1]   = (unsigned char)((oui_as_number >> 8) & 0xff);
		records_list[i].oui[2]   = (unsigned char)(oui_as_number & 0xff);
		records_list[i].version = oui_version;
	}

	return_result = CTC_STACK_set_parameters (automatic_mode, number_of_records, records_list, automatic_onu_configuration);
	
	if( return_result != CTC_STACK_EXIT_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

#ifdef  CTC_ENCRYPT_DEFINE
#endif

DEFUN(
	show_ctc_encryption_timing,
	 show_ctc_encryption_timing_cmd,
	"show encryption-timing",
	SHOW_STR
	"show encryption timing\n")
{
	short int		 return_result;
	unsigned char	 update_key_time;
	unsigned short no_reply_timeout;

	return_result = CTC_STACK_get_encryption_timing(&update_key_time, &no_reply_timeout);

	if ( return_result == CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "  update key time: %d\r\n" ,update_key_time );
		vty_out( vty, "  no reply timeout: %d\r\n", no_reply_timeout );
		return CMD_SUCCESS;
	}
	else
	{
		CTC_STACK_CLI_ERROR_PRINT;
	}
	return CMD_WARNING;
}

DEFUN( 
	ctc_encryption_timing,
	ctc_encryption_timing_cmd,
	"encryption-timing update-key-time <0-255> no-reply-timeout <0-2550>",
	"Set encryption timing parameters\n"
	"update key time\n"
	"Setting update key time measured in seconds\n"
	"no reply timeout\n"
	"Setting no reply timeout measured in 100 ms units\n")
{
	PON_STATUS return_result;
	ULONG  update_key_time;
	ULONG no_reply_timeout;
	
	if( argc < 2 )
	{
		/*vty_out( vty, "  %% Parameter error.\r\n");*/
		return CMD_ERR_EXEED_ARGC_MAX;
	}

	update_key_time = VOS_AtoL(argv[0]);
	if( update_key_time > 255 )
	{
		/*vty_out( vty, "  %% Parameter update-key-time error.\r\n");*/
		return CMD_WARNING;
	}
	no_reply_timeout = VOS_AtoL(argv[1]);
	if( no_reply_timeout > 2550 )
	{
		/*vty_out( vty, "  %% Parameter no-reply-timeout error.\r\n");*/
		return CMD_WARNING;
	}

	return_result = CTC_STACK_set_encryption_timing (update_key_time, no_reply_timeout);
	
	if( return_result != CTC_STACK_EXIT_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN(
	show_ctc_encryption_timing_threshold,
	show_ctc_encryption_timing_threshold_cmd,
	"show encryption-timing-threshold",
	SHOW_STR
	"show encryption timing threshold\n")
{
	PON_STATUS  return_result;
	UCHAR start_encryption_threshold;
	
	return_result = CTC_STACK_get_encryption_timing_threshold(&start_encryption_threshold);
	if ( return_result == CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "  encryption timing threshold: %d\r\n", start_encryption_threshold );
		return CMD_SUCCESS;
	}
	else
		CTC_STACK_CLI_ERROR_PRINT;

	return CMD_WARNING;
}

DEFUN(
	ctc_encryption_timing_threshold,
	ctc_encryption_timing_threshold_cmd,
	"encryption-timing-threshold <0-255>",
	"Set encryption timing threshold parameters\n"
	"start encryption threshold\n"
	"Setting start encryption threshold measured in seconds\n")
{
	PON_STATUS return_result;
	ULONG  start_encryption_threshold;
	
	if( argc != 1 )
	{
		/*vty_out( vty, "  %% Parameter error.\r\n");*/
		return CMD_ERR_EXEED_ARGC_MAX;
	}

	start_encryption_threshold = VOS_AtoL(argv[0]);
	if( start_encryption_threshold > 255 )
	{
		/*vty_out( vty, "  %% Parameter error.\r\n");*/
		return CMD_WARNING;
	}

	return_result = CTC_STACK_set_encryption_timing_threshold (start_encryption_threshold);
	
	if( return_result != CTC_STACK_EXIT_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN(
	ctc_start_encryption_all,
	ctc_start_encryption_all_cmd,
	"ctc start-encryption",
	CTC_STR
	"Start encryption\n")
{
	PON_olt_id_t  olt_id;
	PON_onu_id_t	onu_id;

	ULONG onuEntry = 0;

	/*if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_ERR_EXEED_ARGC_MAX;
	}*/
	for( olt_id=0; olt_id<MAXPONCHIP; olt_id++ )
	{
		if( !PonPortIsWorking(olt_id) )
			continue;
		
		for( onu_id=0; onu_id<MAXONUPERPON; onu_id++ )
		{
			short int llid = 0;

			if( ThisIsValidOnu(olt_id, onu_id) != ROK )
				continue;
			
			if( GetOnuOperStatus(olt_id, onu_id) != 1 )
				continue;

			onuEntry = olt_id*MAXONUPERPON+onu_id;
			llid = GetLlidByOnuIdx( olt_id, onu_id );
			
			if( INVALID_LLID != llid && CTC_STACK_EXIT_OK == CTC_STACK_start_encryption (olt_id, llid ) )
				OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl = 1;
		}
	}
	return CMD_SUCCESS;
}



DEFUN(
	ctc_stop_encryption_all,
	ctc_stop_encryption_all_cmd,
	"ctc stop-encryption",
	CTC_STR
	"Stop exchange encryption key\n")
{
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id;

	ULONG onuEntry = 0;
	
	/*if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_ERR_EXEED_ARGC_MAX;
	}*/
	for( olt_id=0; olt_id<MAXPONCHIP; olt_id++ )
	{
		if( !PonPortIsWorking(olt_id) )
			continue;
		
		for( onu_id=0; onu_id<MAXONUPERPON; onu_id++ )
		{
			short int llid = 0;

			if( ThisIsValidOnu(olt_id, onu_id) != ROK )
				continue;
			
			if( GetOnuOperStatus(olt_id, onu_id) != 1 )
				continue;

			onuEntry = olt_id*MAXONUPERPON+onu_id;

			llid = GetLlidByOnuIdx( olt_id, onu_id );
			
			if( INVALID_LLID != llid && CTC_STACK_EXIT_OK == CTC_STACK_stop_encryption ( olt_id, llid ) )
				OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl = 2;
		}
	}
	return CMD_SUCCESS;
}

#if 0
DEFUN(
	ctc_start_encryption_pon,
	ctc_start_encryption_pon_cmd,
	"ctc start-encryption",
	CTC_STR
	"Start encryption\n")
{
	PON_olt_id_t  olt_id;
	PON_onu_id_t	onu_id;
	ULONG ulSlot, ulPort, ulOnu;

	ULONG onuEntry = 0;

	if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_ERR_EXEED_ARGC_MAX;
	}
	if( PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnu ) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
				
	if (olt_id == VOS_ERROR)
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	if( !PonPortIsWorking(olt_id) )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}			

	for( onu_id=0; onu_id<MAXONUPERPON; onu_id++ )
	{
		short int llid = 0;
		onuEntry = olt_id*MAXONUPERPON+onu_id;
		if( onuEntry >= MAXONU )
			return CMD_WARNING;

		if( ThisIsValidOnu(olt_id, onu_id) != ROK )
				continue;
		
		if( GetOnuOperStatus(olt_id, onu_id) != 1 )
			continue;

		llid = GetLlidByOnuIdx( olt_id, onu_id );

		if( INVALID_LLID != llid && CTC_STACK_EXIT_OK ==  CTC_STACK_start_encryption ( olt_id, llid ) )
			OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl =1;
		
	}
	return CMD_SUCCESS;
}

DEFUN(
	ctc_stop_encryption_pon,
	ctc_stop_encryption_pon_cmd,
	"ctc stop-encryption",
	CTC_STR
	"Stop exchange encryption key\n")
{
	PON_olt_id_t  olt_id;
	PON_onu_id_t	onu_id;
	ULONG ulSlot, ulPort, ulOnu;

	ULONG onuEntry = 0;

	if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_ERR_EXEED_ARGC_MAX;
	}
	if( PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnu ) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
				
	if (olt_id == VOS_ERROR)
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	if( !PonPortIsWorking(olt_id) )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}			

	for( onu_id=0; onu_id<MAXONUPERPON; onu_id++ )
	{
		short int llid = 0;

		if( ThisIsValidOnu(olt_id, onu_id) != ROK )
				continue;
		
		if( GetOnuOperStatus(olt_id, onu_id) != 1 )
			continue;

		onuEntry = olt_id*MAXONUPERPON+onu_id;

		llid = GetLlidByOnuIdx( olt_id, onu_id );

		if( INVALID_LLID != llid && CTC_STACK_EXIT_OK == CTC_STACK_stop_encryption (olt_id, llid) )
			OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl = 2;
	}
	return CMD_SUCCESS;
}
#endif
int setLlidCtcEncrypCtrl( short int PonPortIdx, short int OnuIdx, const ULONG ctrl );
/*add by shixh@2007/08/30*/
/*start or stop pon encrypt*/
DEFUN(
	start_or_stop_encryption_pon,
	start_or_stop_encryption_pon_cmd,
	"encrypt [enable|disable]",
	"start or stop encryption key\n"
	"enable\n"
	"disable\n")
{
	PON_olt_id_t  olt_id;
	PON_onu_id_t	onu_id;
	ULONG ulSlot, ulPort, ulOnu;

	ULONG onuEntry = 0;

/*	if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_ERR_EXEED_ARGC_MAX;
	}*/
	if( PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnu ) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
				
	if (olt_id == VOS_ERROR)
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	if( !PonPortIsWorking(olt_id) )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}			

	for( onu_id=0; onu_id<MAXONUPERPON; onu_id++ )
	{
		short int llid = 0;

		if( ThisIsValidOnu(olt_id, onu_id) != ROK )
				continue;
		
		if( GetOnuOperStatus(olt_id, onu_id) != 1 )
			continue;

		onuEntry = olt_id*MAXONUPERPON+onu_id;

		llid = GetLlidByOnuIdx( olt_id, onu_id );
		/*if(VOS_StriCmp(argv[0], "enable")==0)
			{
   			if( INVALID_LLID != llid && CTC_STACK_EXIT_OK ==  CTC_STACK_start_encryption ( olt_id, llid ) )
			OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl =1;
			vty_out(vty, "\r\n pon start  encrypt!\r\n");
			
			}
               else
               	{
		if( INVALID_LLID != llid && CTC_STACK_EXIT_OK == CTC_STACK_stop_encryption (olt_id, llid) )
			OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl = 2;
			vty_out(vty, "\r\n pon stop encrypt!\r\n");
               	}*/
		if(VOS_StriCmp(argv[0], "enable")==0)
			{
   			if( setLlidCtcEncrypCtrl(olt_id,llid,1)==VOS_OK )
			vty_out(vty, "\r\n pon start  encrypt!\r\n");
			
			}
               else
               	{
		if( setLlidCtcEncrypCtrl(olt_id,llid,2)==VOS_OK )
			vty_out(vty, "\r\n pon stop encrypt!\r\n");
               	}   
	}
	return CMD_SUCCESS;
}


#if 0
DEFUN(
	ctc_start_encryption_onu,
	ctc_start_encryption_onu_cmd,
	"ctc start-encryption",
	CTC_STR
	"Start encryption\n")
{
	PON_STATUS return_result;
	PON_olt_id_t  olt_id;
	PON_onu_id_t	onu_id;

	ULONG onuEntry = 0;
	ULONG slot=0, port=0, onu=0;

	if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_ERR_EXEED_ARGC_MAX;
	}

	if( VOS_OK != PON_GetSlotPortOnu( (ULONG)vty->index, &slot, &port, &onu ) )
		return CMD_WARNING;

	olt_id = GetPonPortIdxBySlot( slot, port );
	if( -1 == olt_id  )
		return CMD_WARNING;

	
	onuEntry = olt_id*MAXONUPERPON+onu-1;
	if( onu >= MAXONU )
		return CMD_WARNING;

	if( parse_onu_command_parameter( vty, &olt_id, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}


	return_result = CTC_STACK_start_encryption (olt_id, onu_id);
	
	if( return_result != CTC_STACK_EXIT_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}

	OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl = 1;
	
	return CMD_SUCCESS;
}



DEFUN(
	ctc_stop_encryption_onu,
	ctc_stop_encryption_onu_cmd,
	"ctc stop-encryption",
	CTC_STR
	"Stop exchange encryption key\n")
{
	PON_STATUS return_result;
	PON_olt_id_t  olt_id;
	PON_onu_id_t	onu_id;

	ULONG	onuEntry = 0;
	ULONG slot=0, port=0, onu=0;

	if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_ERR_EXEED_ARGC_MAX;
	}

	if( VOS_OK != PON_GetSlotPortOnu( (ULONG)vty->index, &slot, &port, &onu ) )
		return CMD_WARNING;
	

	olt_id = GetPonPortIdxBySlot( slot, port );
	if( -1 == olt_id )
		return CMD_WARNING;
	

	onuEntry = olt_id*MAXONUPERPON+onu-1;
	if( onuEntry >= MAXONU )
		return CMD_WARNING;

	
	if( parse_onu_command_parameter( vty, &olt_id, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
	
	return_result = CTC_STACK_stop_encryption (olt_id, onu_id);
	
	if( return_result != CTC_STACK_EXIT_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	
	OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl = 2;
	
	return CMD_SUCCESS;
}
#endif
/*add by shixh@2007/08/29*/
/*start or stop onu encrypt*/
DEFUN(
	start_or_stop_encryption_onu,
	start_or_stop_encryption_onu_cmd,
	"encrypt [enable|disable]",
	"start or stop encryption key\n"
	"enable\n"
	"disable\n")
{
	PON_STATUS return_result;
	PON_olt_id_t  olt_id;
	PON_onu_id_t	onu_id;

	ULONG	onuEntry = 0;
	ULONG slot=0, port=0, onu=0;

	/*if( argc == 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_ERR_EXEED_ARGC_MAX;
	}*/

	if( VOS_OK != PON_GetSlotPortOnu( (ULONG)vty->index, &slot, &port, &onu ) )
		return CMD_WARNING;
	

	olt_id = GetPonPortIdxBySlot( slot, port );
	if( -1 == olt_id )
		return CMD_WARNING;
	

	onuEntry = olt_id*MAXONUPERPON+onu-1;
	if( onuEntry >= MAXONU )
		return CMD_WARNING;

	
	if( parse_onu_command_parameter( vty, &olt_id, 0, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
	
	
	
	if(VOS_StriCmp(argv[0], "enable")==0)
		{
		return_result = CTC_STACK_start_encryption (olt_id, onu_id);
	
		if( return_result != CTC_STACK_EXIT_OK )
		{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
		}
		OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl =1;
		vty_out(vty, "\r\n onu start encrypt!\r\n");	
		}
	else
		{
		return_result = CTC_STACK_stop_encryption (olt_id, onu_id);
	
		if( return_result != CTC_STACK_EXIT_OK )
		{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
		}
		OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl =2;
	     	 vty_out(vty, "\r\n onu stop encrypt!\r\n");	
		}
	
	return CMD_SUCCESS;
}



DEFUN(
	show_ctc_encryption_pon,
	show_ctc_encryption_pon_cmd,
	"show ctc encryption information",
	CTC_STR
	"show ctc encryption information\n"
	"show ctc encryption information\n"
	"show ctc encryption information\n"
	)
{
	PON_olt_id_t  olt_id;
	PON_onu_id_t	onu_id;
	ULONG ulSlot, ulPort, ulOnu;

	short int return_result;
	unsigned char update_key_time;
	unsigned short int no_reply_timeout;

	ULONG onuEntry = 0;
	int flag = 1;

	/*if( argc != 0 )
		{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_ERR_EXEED_ARGC_MAX;
		}*/
	if( PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnu ) != VOS_OK )
		{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
		}
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
				
	if (olt_id == VOS_ERROR)
		{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
		}
	if( !PonPortIsWorking(olt_id) )
		{
		vty_out(vty, "  %% %s/port%d isnot working\r\n", CardSlot_s[ulSlot], ulPort );
		return CMD_WARNING;
		}

	return_result = CTC_STACK_get_encryption_timing(&update_key_time, &no_reply_timeout);

	vty_out(vty," \r\n%s/port%d Encrypt Info\r\n", CardSlot_s[ulSlot], ulPort );

	for( onu_id=0; onu_id<MAXONUPERPON; onu_id++ )
		{
		/*short int llid = 0;*/
		onuEntry = olt_id*MAXONUPERPON+onu_id;
		if( onuEntry >= MAXONU )
			return CMD_WARNING;

		if( ThisIsValidOnu(olt_id, onu_id) != ROK )
				continue;
		if ( OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl == V2R1_ENABLE )
			{
			if( flag == 1 ) 
				{
				vty_out( vty, "onu encrypt enabled as follows \r\n");
				flag = 2 ;
				}
			vty_out( vty, " onu-%d encrypt direction:downstream, key update time=%d(s)\r\n",(onu_id+1), update_key_time);
			}		
		}
	
	if( flag == 1 ) vty_out(vty, "No onu encrypt enabled \r\n");
	vty_out(vty, "\r\n");
	
	return CMD_SUCCESS;
}

DEFUN(
	show_ctc_encryption_onu,
	show_ctc_encryption_onu_cmd,
	"show ctc encryption information",
	CTC_STR
	"show ctc encryption information\n"
	"show ctc encryption information\n"
	"show ctc encryption information\n"
	)
{
	PON_olt_id_t  olt_id;
	PON_onu_id_t	onu_id;
	ULONG ulSlot, ulPort, ulOnu;

	short int return_result;
	unsigned char update_key_time;
	unsigned short int no_reply_timeout;

	ULONG onuEntry = 0;
	/*int flag = 1;*/

	/*if( argc != 0 )
		{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_ERR_EXEED_ARGC_MAX;
		}*/
	if( PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnu ) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    	return CMD_WARNING;
	}
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
				
	if (olt_id == VOS_ERROR)
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	if( !PonPortIsWorking(olt_id) )
	{
		vty_out(vty, "  %% %s/port%d is not working\r\n", CardSlot_s[ulSlot], ulPort );
		return CMD_WARNING;
	}

	return_result = CTC_STACK_get_encryption_timing(&update_key_time, &no_reply_timeout);

	onu_id = ulOnu  -1;
	if( ThisIsValidOnu(olt_id, onu_id) != ROK )
	{
		vty_out(vty, "  %% onu %d/%d/%d is not exist\r\n", ulSlot, ulPort, ulOnu);
		return CMD_WARNING;
	}
	
	onuEntry = olt_id*MAXONUPERPON+onu_id;
	if( onuEntry >= MAXONU )
		return CMD_WARNING;
	
	if ( OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl == V2R1_ENABLE )
	{
		vty_out( vty, " onu %d/%d/%d encrypt direction:downstream, key update time=%d(s)\r\n", ulSlot, ulPort, ulOnu, update_key_time);
	}	
	else
    {
		vty_out( vty, " onu %d/%d/%d no encrypt\r\n", ulSlot, ulPort, ulOnu);
	}
	
	vty_out(vty, "\r\n");
	
	return CMD_SUCCESS;
}

#ifdef CTC_DBA_DEFINE
#endif

DEFUN(
	show_dba_report_thresholds,
	show_dba_report_thresholds_cmd,
	"show ctc dba-reports-threshold",
	SHOW_STR
	CTC_STR
	"Display dba reports threshold\n")
{
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id;
	unsigned char number_of_queue_sets;
	CTC_STACK_onu_queue_set_thresholds_t  queues_thresholds[4];
	short int	queue_index,i;
	short int	return_result;

	/*if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_ERR_EXEED_ARGC_MAX;
	}*/
	if( parse_onu_command_parameter( vty, &olt_id, 0, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}

	return_result = CTC_STACK_get_dba_report_thresholds (olt_id, onu_id, &number_of_queue_sets, queues_thresholds);

	if ( return_result == CTC_STACK_EXIT_OK)
	{
		if( number_of_queue_sets > 1 )
		{
			for( i = 0; i < (number_of_queue_sets-1); i++ )
			{
				vty_out( vty, "  queue set%d\r\n", i+1 );
				for(queue_index = 0; queue_index < 8; queue_index++)
				{
					vty_out( vty, "  queue %d: state: %s threshold: 0x%x\r\n", queue_index,
							queues_thresholds[i].queue[queue_index].state?"TRUE":"FALSE",
							queues_thresholds[i].queue[queue_index].threshold);
				}
			}
		}
		else
			vty_out( vty, "  onu queues is null\r\n" );
		return CMD_SUCCESS;
	} 
	else
	{
		CTC_STACK_CLI_ERROR_PRINT;
	}
	return CMD_WARNING;
}


DEFUN(
	dba_report_thresholds,
	dba_report_thresholds_cmd,
	"ctc dba-reports-threshold queue-sets-number [2|4] queue-sets-list {<queuelist>}*4",
	CTC_STR
	"Set dba report threshold\n"
	"queue sets number\n"
	"Setting queue sets number\n"
	"queue sets list\n"
	"Setting queue sets list. state and threshold separated by ;\n")
{
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id;
	short int	return_result;
	char		*pToken, *token;
	unsigned char	    number_of_queue_sets;
	CTC_STACK_onu_queue_set_thresholds_t	queues_thresholds[4];
	short int	queue_index,i;
	
	if( argc < 2 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_ERR_EXEED_ARGC_MAX;
	}
	if( parse_onu_command_parameter( vty, &olt_id, 0, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}

	VOS_MemZero(queues_thresholds, sizeof(queues_thresholds) );
	number_of_queue_sets = VOS_AtoL( argv[0] );
	
	for(i = 0; i < (number_of_queue_sets-1); i++)
	{
		token = strtok( argv[i+1],  ";" );
		
		for(queue_index = 0; queue_index < 8; queue_index++)
		{
			if( token == NULL )
			{
				/*vty_out( vty,"  error reading queue state in records string\r\n" );
				return CMD_ERR_NO_MATCH;*/
				queues_thresholds[i].queue[queue_index].state = FALSE;
				queues_thresholds[i].queue[queue_index].threshold = 0;
				continue;
			}
			else
			{
				if( VOS_ToLower(token[0]) == 't' )
					queues_thresholds[i].queue[queue_index].state = TRUE;
				else if(VOS_ToLower(token[0]) == 'f' )
					queues_thresholds[i].queue[queue_index].state = FALSE;
				else
				{
					vty_out( vty,"  error reading queue state in records string\r\n" );
					return CMD_ERR_NO_MATCH;
				}
			}

			token = strtok( NULL, ";" );
			if( token == NULL )
			{
				/*vty_out(vty," error reading queue threshold in records string\r\n");
				return CMD_ERR_NO_MATCH;*/
				queues_thresholds[i].queue[queue_index].state = FALSE;
				queues_thresholds[i].queue[queue_index].threshold = 0;
				continue;
			}
			else
			{
				queues_thresholds[i].queue[queue_index].threshold = VOS_StrToUL(token, &pToken, 16);
			}

			token = strtok( NULL, ";" );
		}
	}

	return_result = CTC_STACK_set_dba_report_thresholds (olt_id, onu_id, &number_of_queue_sets, queues_thresholds);

	if ( return_result == CTC_STACK_EXIT_OK)
	{ /* do nothing */
	}
	else if ( return_result == CTC_STACK_EXIT_ERROR_ONU_DBA_THRESHOLDS )
	{
		for(i = 0; i < (number_of_queue_sets-1); i++)
		{
			vty_out( vty, "  queue set%d\r\n", i+1 );
			for(queue_index = 0; queue_index < 8; queue_index++)
			{
				vty_out(vty,"  queue %d: state: %s threshold: 0x%x\r\n", queue_index,
						queues_thresholds[i].queue[queue_index].state?"TRUE":"FALSE",
						queues_thresholds[i].queue[queue_index].threshold );
			}
		}
	}
	else
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

#if 0	/* removed by xieshl 20111118, 已经无效了，这段代码执行会有问题 */
#define weight_of_slot 10000
#define weight_of_port 1000
DEFUN(
	show_ctc_onu_config,
	show_ctc_onu_config_cmd,
	"show ctc-onu-config {<slot/port/onu>}*1",
	DescStringCommonShow
	"show ctc onu configuration data\n"
	"ctc onu index\n"
	)
{
	ULONG ulslot = 0;
	ULONG ulport = 0;
	ULONG ulonu = 0;
	ULONG dex = 0;
	ULONG  * papplyaddr = NULL;
	long  ret = CMD_WARNING;

	papplyaddr = applyforaddress(vty);
	if( papplyaddr == NULL )
	{
		return ret;
	}

	if(argc == 1)
	{
		sscanf( argv[0], "%d/%d/%d", &ulslot ,&ulport, &ulonu);
		if( PonCardSlotPortCheckWhenRunningByVty(ulslot, ulport,vty) == ROK )
		{
			if((ulonu >= 1) && (ulonu <= MAXONUPERPON ))
			{
				dex = ulslot*weight_of_slot + ulport*weight_of_port +ulonu ;

				judgePonId( (ULONG)papplyaddr , dex , vty );
				ret = CMD_SUCCESS;
			}
			else
			{
				vty_out(vty, "%s" , "The val of onu is ranged from 1 to 64\n\r");
			}
		}
	}
	else if( argc == 0 )
	{
		printCtcCfgData( vty , (unsigned char *)papplyaddr );
		ret = CMD_SUCCESS;
	}

	free( papplyaddr) ;

	return ret;
}
#endif

LONG CT_Pon_Init()
{
    	/*install_element ( CONFIG_NODE, &root_into_ctc_onu_node_cmd);
	install_element ( PON_PORT_NODE, &pon_into_ctc_onu_node_cmd);
	install_element ( PON_PORT_NODE, &pon_into_ctc_onu_name_node_cmd);
	install_element ( CONFIG_NODE, &config_into_ctc_onu_name_node_cmd);*/

	install_element ( CONFIG_NODE, &show_extended_oam_discovery_timing_cmd);
	install_element ( PON_PORT_NODE, &show_extended_oam_discovery_timing_cmd);
	install_element ( CONFIG_NODE, &extended_oam_discovery_timing_cmd);
	install_element ( PON_PORT_NODE, &extended_oam_discovery_timing_cmd);

	install_element ( CONFIG_NODE, &show_ctc_oui_cmd);
	install_element ( PON_PORT_NODE, &show_ctc_oui_cmd);
	install_element ( CONFIG_NODE, &ctc_oui_cmd);
	install_element ( PON_PORT_NODE, &ctc_oui_cmd);
	install_element ( CONFIG_NODE, &show_ctc_version_cmd);
	install_element ( PON_PORT_NODE, &show_ctc_version_cmd);
	install_element ( CONFIG_NODE, &ctc_version_cmd);
	install_element ( PON_PORT_NODE, &ctc_version_cmd);

	install_element ( CONFIG_NODE, &show_ctc_params_cmd);
	install_element ( PON_PORT_NODE, &show_ctc_params_cmd);
	install_element ( CONFIG_NODE, &ctc_set_params_cmd);
	install_element ( PON_PORT_NODE, &ctc_set_params_cmd);

	install_element ( CONFIG_NODE, &show_ctc_encryption_timing_cmd);
	install_element ( PON_PORT_NODE, &show_ctc_encryption_timing_cmd);
	install_element ( CONFIG_NODE, &ctc_encryption_timing_cmd);
	install_element ( PON_PORT_NODE, &ctc_encryption_timing_cmd);

	install_element ( CONFIG_NODE, &show_ctc_encryption_timing_threshold_cmd);
	install_element ( PON_PORT_NODE, &show_ctc_encryption_timing_threshold_cmd);
	install_element ( CONFIG_NODE, &ctc_encryption_timing_threshold_cmd);
	install_element ( PON_PORT_NODE, &ctc_encryption_timing_threshold_cmd);
	
	install_element ( CONFIG_NODE, &ctc_start_encryption_all_cmd);
	/*install_element ( PON_PORT_NODE, &ctc_start_encryption_pon_cmd);*/
	/*install_element ( ONU_CTC_NODE, &ctc_start_encryption_onu_cmd);*/
	install_element ( CONFIG_NODE, &ctc_stop_encryption_all_cmd);
	/*install_element ( PON_PORT_NODE, &ctc_stop_encryption_pon_cmd);*/
	/*install_element ( ONU_CTC_NODE, &ctc_stop_encryption_onu_cmd);*/
      install_element ( ONU_CTC_NODE, &start_or_stop_encryption_onu_cmd);
      install_element ( PON_PORT_NODE, &start_or_stop_encryption_pon_cmd);
	
	install_element ( ONU_CTC_NODE, &show_dba_report_thresholds_cmd);
	install_element ( ONU_CTC_NODE, &dba_report_thresholds_cmd);

	/* added by chenfj 2007-8-20
		增加CTC ONU 加密信息显示*/
	install_element ( PON_PORT_NODE, &show_ctc_encryption_pon_cmd );
	install_element ( ONU_CTC_NODE, &show_ctc_encryption_onu_cmd );
	
	/*install_element ( CONFIG_NODE, &show_ctc_onu_config_cmd) ;*/

#if 0 /* __test_ctc*/
	install_element ( ONU_CTC_NODE, &test_ctc_node_cmd);
	install_element ( ONU_CTC_NODE, &test_ctc_node_pause_cmd);
#endif /*__test_ctc*/

	return VOS_OK;
}
/*----------------------------------------------------------------------------*/
#endif
#endif

