#include "syscfg.h"

#ifdef __cplusplus
extern"C"
{
#endif

#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_syslog.h"
#include "vos/vospubh/vos_arg.h"
#include "vos/vospubh/vos_time.h"
#include "vos/vospubh/vos_sem.h"
#include "vos/vospubh/vos_task.h"
#include "vos/vospubh/vos_que.h"
#include "vos/vospubh/vos_ctype.h"

#include "manage/snmp/sn_impl.h"
#include "manage/snmp/sn_conf.h"
#include "manage/snmp/sn_lib.h"
#include "manage/snmp/sn_sys.h"
#include "manage/snmp/sn_a_lib.h"
#include "manage/snmp/sn_a_trp.h"
#include "manage/snmp/sn_cvtovos.h"
#include "manage/snmp/cl_sn.h"
#include "manage/snmp/sn_mt.h"
#include "manage/snmp/sn_api.h"
#include "cli/cl_vect.h"
#include "cli/cl_vty.h"
#include "cli/cl_cmd.h"
#include "ifm/ifm_pub.h"
#include "ifm_api.h"
#include "ifm/ifm_def.h"
#include "ifm/eth/eth_type.h"
#include "ifm/vlan/vlan_type.h"
#include "sys/devsm/devsm_cli.h"
#include "switch/rstp/stp_vos.h"
#include "switch/rstp/stp_def.h"
#include "switch/rstp/stp_types.h"
#include "switch/rstp/stp_handler_inc.h"
#include "switch/rstp/stp_stpm.h"
#include "switch/rstp/stp_management_inc.h"
#include "switch/rstp/stp_hash_inc.h"
#include "switch/rstp/stp_cli.h"
#include "mn_oam.h"

#if(PRODUCT_EPON3_ONU == RPU_YES)  
#include "cl_buf.h"
#include "cl_oam.h"
#include "ifm/mac_cli.h"
#include "route/mrt_api.h"
#include "route/ipmc_drv_api.h"
#include "switch/igmp_snoop/igmp_snoop.h"
#include "sys/main/sys_main.h"
#include "bcm/port.h"
#include "qos/qos_include.h"
#endif
#include "OltGeneral.h"
#include "PonGeneral.h"
#include "V2R1_product.h"
#include "V2R1General.h"
#include "gwttoam/OAM_gw.h"

/****************************************************/
/*文件里面需要增加向OLT侧的回送内容*/
/****************************************************/

extern long g_lVlanRunMode;
extern long g_lVlanOutTpid ;
extern long g_lVlanInnTpid;
extern ULONG g_ulIFMQue;
extern ULONG STP_QueueID ;

extern ULONG g_ulIgmp_Snoop_MSQ_id;
extern ULONG g_ulQosQueId;
extern int g_SystemLoadConfComplete;

#if (PRODUCT_EPON3_ONU == RPU_YES)  
/*Product software version information*/
extern const UCHAR PRODUCT_SOFTWARE_MAJOR_VERSION_NO;
extern const UCHAR PRODUCT_SOFTWARE_RELEASE_VERSION_NO;
extern const UCHAR PRODUCT_SOFTWARE_BUILD_VERSION_NO ; 
extern const UCHAR PRODUCT_SOFTWARE_DEBUG_VERSION_NO ;
extern const UCHAR PRODUCT_SOFTWARE_SP_VERSION_NO ;

extern VOID STP_CLI_CfgPort( const LONG argc, CHAR * argv[], struct vty * vty );
extern VOID STP_CLI_CfgPortDef( const LONG argc, CHAR * argv[], struct vty * vty );
extern VOID STP_CLI_CfgBridge( const LONG argc, CHAR * argv[], struct vty * vty );
extern VOID STP_CLI_CfgBridgeDef( const LONG argc, CHAR * argv[], struct vty * vty );
extern VOID STP_CLI_ShowBridge( const LONG argc, CHAR * argv[], struct vty * vty );
extern VOID STP_CLI_ShowBridgePort( const LONG argc, CHAR * argv[], struct vty * vty );
extern int cl_do_reboot_system( struct vty * vty );

extern int cl_login_auth_needed;
extern ULONG  g_ulIgmp_Auth_Enabled ;
extern ULONG semForwardEntry;
extern struct ForwardEntryItem *g_stStaticForwardEntryList ;
extern INT32 g_iMacAgingTime;
extern QOS_PORT_QOS_INFO_S stPortQosGInfo;
extern QOS_SIMPLE_MODE_S  g_Qos_Port[27];/*0不用，从1-26*/
extern CHAR g_cDscpTos;
extern UCHAR g_ucDscp[64];
extern UCHAR g_ucTos[8];
#ifndef QOS_SIMPLE_SELECT_DSCP
#define QOS_SIMPLE_SELECT_DSCP   1
#endif 
#ifndef QOS_SIMPLE_SELECT_TOS
#define QOS_SIMPLE_SELECT_TOS   2
#endif

#define STATE_PS 1
#define STATE_PE 2
#define MAX_PORT_NUMBER_IN_PORT_LIST	26
#define START_PORT			1
#define END_PORT			MAX_PORT_NUMBER_IN_PORT_LIST
ULONG * ONU_Parse_Port_List( CHAR * pcPort_List )
{
    ULONG ulState = STATE_PS;
    CHAR digit_temp[ 12 ];
    ULONG ulInterfaceList[ MAX_PORT_NUMBER_IN_PORT_LIST ];
    ULONG ulPortS = 0;
    ULONG ulPortE = 0;
    CHAR cToken;
    ULONG iflist_i = 0;
    ULONG list_i = 0;
    ULONG temp_i = 0;
    ULONG ulListLen = 0;
    CHAR * list;
    
    VOS_MemZero( ulInterfaceList, MAX_PORT_NUMBER_IN_PORT_LIST * 4 );
    ulListLen = VOS_StrLen( pcPort_List );
    list = VOS_Malloc( ulListLen + 2, MODULE_RPU_CLI );
    if ( list == NULL )
    {
        return NULL;
    }
    VOS_StrnCpy( list, pcPort_List, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];

    while ( cToken != 0 )
    {
        switch ( ulState )
        {            
            case STATE_PS:
                if ( vos_isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( vos_isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );

                    if ( ulPortS > MAX_PORT_NUMBER_IN_PORT_LIST )
                    {
                        goto error;
                    }
			/*输出端口号**/
                    if ( 0 != ulPortS )
                    {
                        ulInterfaceList[ iflist_i ] = ulPortS;
                        iflist_i++;
                    }
                    if ( iflist_i > MAX_PORT_NUMBER_IN_PORT_LIST )
                    {
                        goto error;
                    }
                    temp_i = 0;
                    ulState = STATE_PS;
                }
                else if ( cToken == '-' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );
                    temp_i = 0;
                    ulState = STATE_PE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_PE:
                if ( vos_isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( vos_isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                    ULONG i;
                    ULONG i_s, i_e;
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortE = ( ULONG ) VOS_AtoL( digit_temp );
                    if ( ulPortE > ulPortS )
                    {
                        i_s = ulPortS;
                        i_e = ulPortE;
                    }
                    else
                    {
                        i_s = ulPortE;
                        i_e = ulPortS;
                    }
                    
                    for ( i = i_s;i <= i_e;i++ )
                    {
                        if ( i<START_PORT || i > END_PORT )
                        {
                            goto error;
                        }

                        if ( 0 != i )
                        {
                            ulInterfaceList[ iflist_i ] = i;
                            iflist_i++;
                        }
                        if ( iflist_i > MAX_PORT_NUMBER_IN_PORT_LIST )
                        {
                            goto error;
                        }
                    }
                    temp_i = 0;
                    ulState = STATE_PS;
                }
                else
                {
                    goto error;
                }
                break;
            default:
                goto error;
        }
        list_i++;
        cToken = list[ list_i ];
    }
    VOS_Free( list );
    if ( iflist_i == 0 )
    {
        return NULL;
    }
    else
    {
        list = VOS_Malloc( ( iflist_i + 1 ) * 4, MODULE_RPU_IFM );
        if ( list == NULL )
        {
            return NULL;
        }
        VOS_MemZero( list, ( iflist_i + 1 ) * 4 );
        VOS_MemCpy( list, ulInterfaceList, iflist_i * 4 );
        return ( ULONG * ) list;
    }
error:
    VOS_Free( list );
    return NULL;
}

enum match_type Onu_Check_Port_List( char * port_list )
{
    int len = VOS_StrLen( port_list );
    ULONG interface_list[ MAX_PORT_NUMBER_IN_PORT_LIST ];
    int j, if_num = 0;
    int ret = 0;
    ULONG ulPort=0;

    char *plistbak = NULL;

    if ( ( !port_list ) || ( len < 1 ) )
    {
        return incomplete_match;
    }

    VOS_MemZero( ( char * ) interface_list, MAX_PORT_NUMBER_IN_PORT_LIST * sizeof( ULONG ) );
    plistbak = ( char * ) VOS_Malloc( len + 1, MODULE_RPU_CLI );
	if ( plistbak == NULL )
	{
	    return no_match;
	}

	VOS_StrCpy( plistbak, port_list );

    BEGIN_PARSE_PORT_LIST_TO_PORT( plistbak, ulPort )
    {
        for ( j = 0; j <= if_num; j++ )
        {
            if ( interface_list[ j ] == ulPort )
            {
                VOS_Free( plistbak );
                plistbak = NULL;
                RETURN_PARSE_PORT_LIST_TO_PORT( no_match );
            }
        }
        interface_list[ if_num ] = ulPort;
        if_num ++;
        if ( if_num > MAX_PORT_NUMBER_IN_PORT_LIST )
        {
            VOS_Free( plistbak );
            plistbak = NULL;
            
            RETURN_PARSE_PORT_LIST_TO_PORT( no_match );
        }
        ret = 1;
    }
    END_PARSE_PORT_LIST_TO_PORT();

    VOS_Free( plistbak );

    if ( ret == 0 )
        return incomplete_match;
    else
        return exact_match;
}

LONG ONU_CheckPortListValid( const char * pcPortList )
{
    ULONG * p = NULL;

    p = ONU_Parse_Port_List( ( CHAR * ) pcPortList );

    if ( p == NULL )
    {
        return 0;
    }
    else
    {
        VOS_Free( p );
        return 1;
    }
}
#endif


/*Port CLI*/

QDEFUN ( onu_auto_enable_if,
         onu_auto_enable_if_cmd,
         "port <1-26> auto [enable|disable]",
         "Config onu port parameter\n"         
         "Input the onu port number\n"
         "Auto negotiation setting\n"
         "Auto negotiation enable\n"
         "Auto negotiation disable\n",
    &g_ulIFMQue  )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG ulIfIndex = 0;
    ULONG ulAuto = 0;
    ULONG   ulOldAuto = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    if ( !VOS_StrCmp( argv[ 1 ], "enable" ) )
    {
        ulAuto = AUTO_ENABLE;
    }
    else if ( !VOS_StrCmp( argv[ 1 ], "disable" ) )
    {
        ulAuto = AUTO_DISABLE;
    }
    else
    {
        vty_out( vty, "%% Parameter is error.\r\n" );
        return CMD_SUCCESS;
    }
    
    lRet = IFM_get_info ( ulIfIndex, IFM_CONFIG_ETH_AUTONEGOTATION, &ulOldAuto, NULL );
    if ( VOS_OK != lRet )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
    else
    {
        if ( ulOldAuto == ulAuto )
        {
            if ( ulAuto == AUTO_ENABLE )
            {
                vty_out( vty, "%% Auto negotiation has been enabled already.\r\n" );
            }
            
            if ( ulAuto == AUTO_DISABLE )
            {
                vty_out( vty, "%% Auto negotiation has been disabled already.\r\n" );
            }
            
            return CMD_WARNING;
        }
    }

    lRet = IFM_config( ulIfIndex, IFM_CONFIG_ETH_AUTONEGOTATION, &ulAuto, NULL );

    if ( VOS_OK != lRet )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
    
    vty->node = ONU_DEVICE_NODE; 
    vty_out( vty, " Port %d auto negotiation set %s !\r\n" , lPort, argv[1]);
#endif
    return CMD_SUCCESS;

}


QDEFUN ( onu_duplex_enable_if,
         onu_duplex_enable_if_cmd,
         "port <1-26> duplex [full|half]",
         "Config onu port parameter\n"         
         "Input the onu port number\n"
         "Duplex setting\n"
         "Duplex enable full\n"
         "Duplex enable half\n" ,
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG ulIfIndex = 0;
    ULONG ulDuplex = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    if ( !VOS_StrCmp( argv[ 1 ], "full" ) )
    {
        ulDuplex = DUPLEX_FULL;
    }
    else if ( !VOS_StrCmp( argv[ 1 ], "half" ) )
    {
        ulDuplex = DUPLEX_HALF;
    }
    else
    {
        vty_out( vty, "%% Parameter is error.\r\n" );
        return CMD_SUCCESS;
    }

    lRet = IFM_config( ulIfIndex, IFM_CONFIG_ETH_DUPLEX, &ulDuplex, NULL );
    if ( VOS_OK != lRet )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
    else
    	vty_out( vty, " Port %d duplex set %s !\r\n", lPort, argv[1] );
    vty->node = ONU_DEVICE_NODE; 
    
#endif
    return CMD_SUCCESS;
}

QDEFUN ( onu_flowcontrol_enable_if,
         onu_flowcontrol_enable_if_cmd,
         "port <1-26> flowcontrol [enable|disable]",
         "Config onu port parameter\n"         
         "Input the onu port number\n"
         "Flowcontrol setting\n"
         "Flowcontrol enable\n"
         "Flowcontrol disable\n",
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG ulFlowctr = 1;
    ULONG ulOldFlowctr = 0;
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    if ( !VOS_StrCmp( argv[ 1 ], "enable" ) )
    {
        ulFlowctr = FLOWCTL_ENABLE;
    }
    else if ( !VOS_StrCmp( argv[ 1 ], "disable" ) )
    {
        ulFlowctr = FLOWCTL_DISABLE;
    }
    else
    {
        vty_out( vty, "%% Parameter is error.\r\n" );
        return CMD_SUCCESS;
    }

    lRet = IFM_get_info ( ulIfIndex, IFM_CONFIG_ETH_FLOWCONTROL, &ulOldFlowctr, NULL );
    if ( VOS_OK != lRet )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
    else
    {
        if ( ulOldFlowctr == ulFlowctr )
        {
            if ( ulFlowctr == FLOWCTL_ENABLE )
            {
                vty_out( vty, "%% Flow control has been enabled already.\r\n" );
            }
            
            if ( ulFlowctr == FLOWCTL_DISABLE )
            {
                vty_out( vty, "%% Flow control has been disabled already.\r\n" );
            }
            
            return CMD_WARNING;
        }
    }

    lRet = IFM_config( ulIfIndex, IFM_CONFIG_ETH_FLOWCONTROL, &ulFlowctr, NULL );
    if ( VOS_OK != lRet )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
    else
    	vty_out( vty, " Port %d flowcontrol set %s!\r\n", lPort, argv[1] );
    vty->node = ONU_DEVICE_NODE;     

#endif
    return CMD_SUCCESS;
}


QDEFUN ( onu_speed_if,
         onu_speed_if_cmd,
         "port <1-26> speed [10|100|1000]",
         "Config onu port parameter\n"         
         "Input the onu port number\n"
         "Speed setting\n"
         "Set port's speed to 10Mbps\n"
         "Set port's speed to 100Mbps\n"
         "Set port's speed to 1000Mbps\n" ,
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG ulBaud = 0;
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    ulBaud = ( ULONG ) VOS_AtoL( argv[ 1 ] );

    lRet = IFM_config( ulIfIndex, IFM_CONFIG_ETH_BAUD, &ulBaud, NULL );
    if ( VOS_OK != lRet )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
    else
    	vty_out( vty, " Port %d speed set %s Mbps!\r\n", lPort, argv[1] );
    vty->node = ONU_DEVICE_NODE; 
    
#endif
    return CMD_SUCCESS;
}

QDEFUN( onu_Encap_if_Func,
        onu_encap_if_cmd,
        "port <1-26> encapsulation [snap|dix]" ,
        "Config onu port parameter\n" 
        "Input the onu port number\n"
        "Config port encapsulation\n"
        "Set port's encapsulation to SNAP\n"
        "Set port's encapsulation to DIX\n",
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG ulEncap = 0;
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );


    if ( !VOS_StrCmp( "snap", argv[ 1 ] ) )
    {
        ulEncap = ETH_ENCAPSULATION_TYPE_SNAP;
    }
    else if ( !VOS_StrCmp( "dix", argv[ 1 ] ) )
    {
        ulEncap = ETH_ENCAPSULATION_TYPE_DIX;
    }
    else
    {
        vty_out( vty, "%% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

    lRet = IFM_config( ulIfIndex, IFM_CONFIG_ETH_ENCAP, &ulEncap, NULL );
    if ( VOS_OK != lRet )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
    else
    	vty_out( vty, " Port %d encapsulation set %s!\r\n", lPort, argv[1] );
    vty->node = ONU_DEVICE_NODE; 
    
#endif
    return CMD_SUCCESS;
}


QDEFUN( onu_port_learn_func,
        onu_port_learn_cmd,
        "port <1-26> learning [enable|disable]",
        "Config onu port parameter\n" 
        "Input the onu port number\n"
        "Enable or disable learning\n"
        "Enable learning\n"
        "Disable learning\n" ,
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG     ulEnable = 0;
    ULONG     usGetLeval = 0;
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    if(!VOS_StrCmp(argv[1],"enable"))
    {
         ulEnable = LEARN_ENABLE;
    }
    else if(!VOS_StrCmp(argv[1],"disable"))
    {
         ulEnable = LEARN_DISABLE;
    }
    else
    {
        vty_out(vty,"%% Parameter is error.\r\n");
        return CMD_WARNING;
    }

    lRet = IFM_get_info(ulIfIndex,IFM_CONFIG_ETH_LEARN,&usGetLeval,NULL);
    if(VOS_OK != lRet)
    {
        IFM_PError(lRet, vty);
        return CMD_WARNING;
    }

    if(usGetLeval == ulEnable)
    {
        vty_out(vty,"%% This port has already been %s.\r\n",argv[1]);
        return CMD_WARNING;
    }
    
    lRet = IFM_config(ulIfIndex,IFM_CONFIG_ETH_LEARN,(UCHAR *)&ulEnable,NULL);
    if(VOS_OK != lRet)
    {
        IFM_PError(lRet, vty);
        return CMD_WARNING;
    }
    else
    	vty_out( vty, " Port %d learning set %s!\r\n", lPort, argv[1] );
    vty->node = ONU_DEVICE_NODE; 
    
    
#endif    
    return CMD_SUCCESS;
}

QDEFUN ( onu_port_desc_fun,
         onu_port_desc_cmd,
         "port <1-26> description <string> {<string>}*29",
         "Config onu port parameter\n"
         "Input the onu port number\n"
         "Config interface specific description\n"
         "Characters describing this interface\n"
         "Characters describing this interface\n",
    &g_ulIFMQue  )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    CHAR strTemp[ IFM_MAX_DESCR_LEN + 1 ];
    INT strLen = 0;
    INT i = 0;
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    if( 1==argc )
    {
        vty_out( vty, 
            " %% Please input describing.\r\n" );

        return CMD_WARNING;
    }
    else if( argc > IFM_MAX_DESCR_NUM +1)
    {
        vty_out( vty, 
            " %% The number of interface description string must not be more than %d.\r\n", IFM_MAX_DESCR_NUM );

        return CMD_WARNING;
    }

    VOS_MemZero( strTemp, IFM_MAX_DESCR_LEN + 1 );
    for ( i = 1; i < argc; i++ )
    {
        if ( VOS_StrLen( argv[ i ] ) == 0 )
            break;
        if ( ( strLen + VOS_StrLen( argv[ i ] ) ) > IFM_MAX_DESCR_LEN )
        {
            vty_out( vty, " %% The length of interface description string must not be more than %d.\r\n", IFM_MAX_DESCR_LEN );
            return CMD_WARNING;
        }
        VOS_StrCat( strTemp, argv[ i ] );
        VOS_StrCat( strTemp, " " );
        strLen += ( VOS_StrLen( argv[ i ] ) + 1 );
    }

    strTemp[ strLen - 1 ] = '\0';

    if ( ( strLen - 1 ) > IFM_MAX_DESCR_LEN )
    {
        vty_out( vty, " %% The length of interface description string must not be more than %d.\r\n", IFM_MAX_DESCR_LEN );
        return CMD_WARNING;
    }

    lRet = IFM_config( ulIfIndex, IFM_CONFIG_GLOBAL_SPEC, ( VOID* ) strTemp, NULL );
    if ( VOS_OK != lRet )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
    vty->node = ONU_DEVICE_NODE; 
    vty_out( vty, " Port %d specific description:\r\n", lPort );
    vty_out( vty, " %s \r\n", strTemp );
#endif

    return CMD_SUCCESS;
}

QDEFUN ( no_onu_port_desc_fun,
         no_onu_port_desc_cmd,
         "undo port <1-26> description",
         NO_STR
         "Config onu port parameter\n"
         "Input the onu port number\n"         
         "Set default interface specific description\n" ,
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    CHAR strTemp[ IFM_MAX_DESCR_LEN + 1 ];
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    VOS_MemZero( strTemp, IFM_MAX_DESCR_LEN + 1 );

    lRet = IFM_config_global( ulIfIndex, IFM_CONFIG_GLOBAL_SPEC, ( VOID* ) strTemp, NULL );
    if ( VOS_OK != lRet )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
    vty->node = ONU_DEVICE_NODE; 
    vty_out( vty, " Undo port %d description set OK!\r\n", lPort );
#endif

    return CMD_SUCCESS;
}

QDEFUN ( onu_shutdown_if,
         onu_shutdown_if_cmd,
         "port <1-26> shutdown",         
         "Config onu port parameter\n"
         "Input the onu port number\n"
         "Shutdown the interface\n",
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    lRet = IFM_admin_down( ulIfIndex, NULL, NULL );
    if ( VOS_OK != lRet )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
    vty->node = ONU_DEVICE_NODE; 
    vty_out( vty, " Port %d shutdown set OK!\r\n" , lPort );
#endif

    return CMD_SUCCESS;
}

QDEFUN ( onu_Line_Rate_ingress,
         onu_Line_Rate_ingress_Cmd,
         "port <1-26> line-rate ingress cir [0|<64-1024000>] cbs <0-1040>",
         "Config onu port parameter\n"
         "Input the onu port number\n"
         "Line_Rate control \n"
         "Ingress line rate control\n"
         "Committed information rate\n"
         "Cancle Line_rate control\n"
         "The value of cir(kbps):64kbps-100000kbps for 100M port,64kbps-1024000kbps for 1000M port \n"
         "The Conform burst size\n"
         "The value of Conform burst size(kbit):0kbit-1040kbit for 100M/1000M port\n",
          &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    LINERATE_INFO sLineRate ={0};
    ULONG   ulIfIndex = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    vty->node = ONU_DEVICE_NODE; 

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );


    sLineRate.ulCir = ( ULONG ) VOS_AtoL( argv[ 1 ] );
    sLineRate.ulCbs = ( ULONG )VOS_AtoL( argv[2] );

    lRet = IFM_config( ulIfIndex, IFM_CONFIG_ETH_INGRESS_LINERATE, &sLineRate, NULL );
    if ( VOS_OK != lRet )
    {        
#ifdef _DISTRIBUTE_PLATFORM_ 
	 vty_out( vty, " %% line-rate ingress set Failed!\r\n" );
        return CMD_WARNING;
#endif
    }

#ifdef _DISTRIBUTE_PLATFORM_
    if(VOS_OK == lRet)
    {
        IFM_InterfaceActSyncSlave(
            IFM_MSG_INTERFACE_CONFIG,\
            IFM_CONFIG_ETH_INGRESS_LINERATE, ulIfIndex, 0, 
           &sLineRate, sizeof( sLineRate )
            );
    }
#endif	
	vty_out( vty, " Line-rate ingress set OK!\r\n" );
#endif
	return CMD_SUCCESS;
}


QDEFUN ( onu_Line_Rate_egress,
         onu_Line_Rate_egress_Cmd,
         "port <1-26> line-rate egress cir [0|<64-1024000>] cbs <0-1040>",
         "Config onu port parameter\n"
         "Input the onu port number\n"
         "Line_Rate control \n"
         "Egress line rate control\n"
         "Committed information rate\n"
         "Cancle Line_rate control\n"
         "The value of cir(kbps):64kbps-100000kbps for 100M port,64kbps-1024000kbps for 1000M port \n"
         "The Conform burst size\n"
         "The value of Conform burst size(kbit):0kbit-1040kbit for 100M/1000M port\n",
          &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    LINERATE_INFO sLineRate ={0};
    ULONG   ulIfIndex = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    vty->node = ONU_DEVICE_NODE; 
    
    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    sLineRate.ulCir = ( ULONG ) VOS_AtoL( argv[ 1 ] );
    sLineRate.ulCbs = ( ULONG )VOS_AtoL( argv[2] );

    lRet = IFM_config( ulIfIndex, IFM_CONFIG_ETH_EGRESS_LINERATE, &sLineRate, NULL );
    if ( VOS_OK != lRet )
    {        
#ifdef _DISTRIBUTE_PLATFORM_ 

	 vty_out( vty, " %% Line-rate egress set Failed!\r\n" );
        return CMD_WARNING;
#endif
    }

#ifdef _DISTRIBUTE_PLATFORM_
    if(VOS_OK == lRet)
    {
        IFM_InterfaceActSyncSlave(
            IFM_MSG_INTERFACE_CONFIG,\
            IFM_CONFIG_ETH_EGRESS_LINERATE, ulIfIndex, 0, 
           &sLineRate, sizeof( sLineRate )
            );
    }
#endif
	vty_out( vty, " Line-rate egress set OK!\r\n" );
#endif
	return CMD_SUCCESS;
}


QDEFUN ( onu_storm_control,
         onu_storm_control_cmd,
         "port <1-26> storm-control [bcast|mcast|dlf] <100-100000>",
         "Config onu port parameter\n"
         "Input the onu port number\n"
         "Storm control \n"
         "Broadcast storm control\n"
         "Multicast storm control\n"
         "Destination look-up fail (DLF) storm control\n"
         "Please input the control value\n",
          &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG lRet ;
    ULONG   ulIfIndex = 0;
    ULONG ulControlValue = 0;
    ULONG ulCode = 0;
    LONG lPort = 0;

    vty->node = ONU_DEVICE_NODE; 
    
    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    if ( !VOS_StrCmp( argv[ 1 ], "bcast" ) )
    {
    	ulCode = IFM_CONFIG_ETH_STORMCRTL_BCAST;
    }
    else if ( !VOS_StrCmp( argv[ 1 ], "mcast" ) )
    {
    	ulCode = IFM_CONFIG_ETH_STORMCRTL_MCAST;
    }
    else if ( !VOS_StrCmp( argv[ 1 ], "dlf" ) )
    {
    	ulCode = IFM_CONFIG_ETH_STORMCRTL_DLF;
    }
    else
    {
        vty_out( vty, " %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }    

    ulControlValue = ( ULONG ) VOS_AtoL( argv[ 2 ] );       

    lRet = IFM_config( ulIfIndex, ulCode, &ulControlValue, NULL );
    if ( VOS_OK != lRet )
    {        
#ifdef _DISTRIBUTE_PLATFORM_ /* add by wood */
        return CMD_WARNING;
#endif
    }

#ifdef _DISTRIBUTE_PLATFORM_
    if(VOS_OK == lRet)
    {
        IFM_InterfaceActSyncSlave(
            IFM_MSG_INTERFACE_CONFIG,\
            ulCode, ulIfIndex, 0, 
           &ulControlValue, sizeof( ulControlValue )
            );
    }
#endif
	vty_out( vty, " Storm control set OK!\r\n" );
#endif
	return CMD_SUCCESS;
}

QDEFUN ( onu_undo_storm_control,
         onu_undo_storm_control_cmd,
         "port <1-26> undo storm-control [bcast|mcast|dlf]",
         NO_STR
         "Undo Storm control \n"
         "Broadcast storm control\n"
         "Multicast storm control\n"
         "Destination look-up fail (DLF) storm control\n",
          &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG lRet ;
    ULONG   ulIfIndex = 0;
    ULONG ulControlValue = 0;
    ULONG ulCode = 0;
    LONG lPort = 0;

    vty->node = ONU_DEVICE_NODE; 
    
    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    if ( !VOS_StrCmp( argv[ 1 ], "bcast" ) )
    {
    	ulCode = IFM_CONFIG_ETH_STORMCRTL_BCAST;
    }
    else if ( !VOS_StrCmp( argv[ 1 ], "mcast" ) )
    {
    	ulCode = IFM_CONFIG_ETH_STORMCRTL_MCAST;
    }
    else if ( !VOS_StrCmp( argv[ 1 ], "dlf" ) )
    {
    	ulCode = IFM_CONFIG_ETH_STORMCRTL_DLF;
    }
    else
    {
        vty_out( vty, " %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }    

    lRet = IFM_config( ulIfIndex, ulCode, &ulControlValue, NULL );
    if ( VOS_OK != lRet )
    {        
#ifdef _DISTRIBUTE_PLATFORM_ /* add by wood */
        return CMD_WARNING;
#endif
    }

#ifdef _DISTRIBUTE_PLATFORM_
    if(VOS_OK == lRet)
    {
        IFM_InterfaceActSyncSlave(
            IFM_MSG_INTERFACE_CONFIG,\
            ulCode, ulIfIndex, 0, 
           &ulControlValue, sizeof( ulControlValue )
            );
    }
#endif
	vty_out( vty, " Undo storm control set OK!\r\n" );
#endif
	return CMD_SUCCESS;
}


QDEFUN ( onu_no_shutdown_if,
         onu_no_shutdown_if_cmd,
         "undo port <1-26> shutdown",
         NO_STR         
         "Config onu port parameter\n"
         "Input the onu port number\n"
         "No shutdown the interface\n",
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    lRet = IFM_admin_up( ulIfIndex, NULL, NULL );
    if ( VOS_OK != lRet )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
    vty->node = ONU_DEVICE_NODE; 
    vty_out( vty, " Undo port %d shutdown set OK!\r\n", lPort );
#endif

    return CMD_SUCCESS;
}

/*This cli need to do more work.*/
QDEFUN( showt_onu_eth_func,
        show_onu_eth_cmd,
        "show port {<1-26>}*1",
        DescStringCommonShow
        "Show onu port\n"
        "Input onu port number\n" ,
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    LONG    lRet;
    ULONG   ulIfIndex = 0;
    int     i = 0;
    CHAR    *pScreenBuffer = NULL;
    ULONG   ulShowType = IFM_SHOW_INTERFACE_CONFIG;
    LONG lPort = 0;

    ulShowType |= IFM_SHOW_INTERFACE_STATIS;
        
    if ( argc == 0 )
    {
        for ( i = 1; i <= 26; i ++ )
        {
            lPort = ( LONG ) i;
            ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

            lRet = IFM_show( ulIfIndex, ulShowType, &pScreenBuffer, NULL, vty );
            if ( lRet < 0 )
            {
                IFM_PError( lRet, vty );
                return CMD_WARNING;
            }
            IFM_BIGSHOW( vty, pScreenBuffer );
        }
        return CMD_SUCCESS;
    }
    else
    {
        lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
        ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );
        lRet = IFM_show( ulIfIndex, ulShowType, &pScreenBuffer, NULL, vty);
        if ( lRet < 0 )
        {
                IFM_PError( lRet, vty );
                return CMD_WARNING;
        }
        IFM_BIGSHOW( vty, pScreenBuffer );
        return CMD_SUCCESS;
    }
    vty->node = ONU_DEVICE_NODE; 
#endif
    return CMD_SUCCESS;
}


QDEFUN( clear_onu_eth_statistic_func,
        clear_onu_eth_statistic_cmd,
        "clear port <1-26> statistics",
        "Clear\n"
        "Clear onu port statistics\n" 
        "Input onu port number please\n"
        "Clear onu port statistics\n",
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG     ulTemp = 1;
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
    ulIfIndex = IFM_ETH_CREATE_INDEX( 1, lPort );

    lRet = IFM_config(ulIfIndex,IFM_CONFIG_ETH_CLRSTATS,&ulTemp,NULL);
    if(VOS_OK != lRet)
    {
        IFM_PError(lRet, vty);
        return CMD_WARNING;
    }
    vty->node = ONU_DEVICE_NODE; 
    vty_out( vty, " Clear Port %d statistics OK!\r\n", lPort );
#endif

    return CMD_SUCCESS;
}

QDEFUN ( onu_GT813_Isolate,
         onu_GT813_Isolate_Cmd,
         "isolate <port_list>",
         "Port isolate\n"
         "Specify port number list you want to isolate(e.g.: 1-3,6 )\n" ,
          &g_ulIFMQue )
{	    
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG lRet ;
    ULONG ulPortIfindex = 0;
    ULONG ulIsolate = 1; 
    ULONG ulPort = 0;

    vty->node = ONU_DEVICE_NODE; 
    if ( ONU_CheckPortListValid( argv[0] ) != 1 )
    {
        vty_out( vty, "  %% Invalid port list <%s>.\r\n", argv[ 0 ] );
        return CMD_WARNING;
    }

    BEGIN_PARSE_PORT_LIST_TO_PORT( argv[0], ulPort )
    {
    		/* modified by xieshl 20120906, 解决丢内存问题 */
           if( ulPort <= 24 )
           {
	           ulPortIfindex = IFM_ETH_CREATE_INDEX( 1, ulPort );
	    	    lRet = IFM_config( ulPortIfindex, IFM_CONFIG_ETH_ISOLATE, &ulIsolate, NULL );
	    	    if ( VOS_OK != lRet )
	    	    {        
			vty_out( vty, " %% Isolate set Failed!\r\n" );
	              RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
#ifdef _DISTRIBUTE_PLATFORM_
	    		if(VOS_OK == lRet)
	    		{
	    			IFM_InterfaceActSyncSlave(
	    				IFM_MSG_INTERFACE_CONFIG,\
	    				IFM_CONFIG_ETH_ISOLATE, ulPortIfindex, 0, 
	    				&ulIsolate, sizeof( ulIsolate )
	    				);
	    		}
#endif
           }
    }
    END_PARSE_PORT_LIST_TO_PORT();
    
    vty_out( vty, " Isolate set OK!\r\n" );
#endif
	return CMD_SUCCESS;
}

QDEFUN ( onu_GT813_unIsolate,
         onu_GT813_unIsolate_Cmd,
         "undo isolate <port_list>",
         NO_STR
         "Port isolate\n"
         "Specify port number list you want to cancle isolate(e.g.: 1/1-3,1/6 )\n" ,
          &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG lRet ;
    ULONG ulPortIfindex = 0;
    ULONG ulIsolate = 0; 
    ULONG ulPort = 0;

    vty->node = ONU_DEVICE_NODE;

    if ( ONU_CheckPortListValid( argv[0] ) != 1 )
    {
        vty_out( vty, "  %% Invalid port list <%s>.\r\n", argv[ 0 ] );
        return CMD_WARNING;
    }

    BEGIN_PARSE_PORT_LIST_TO_PORT( argv[0], ulPort )
    {
           ulPortIfindex = IFM_ETH_CREATE_INDEX( 1, ulPort );
    	    lRet = IFM_config( ulPortIfindex, IFM_CONFIG_ETH_ISOLATE, &ulIsolate, NULL );
    	    if ( VOS_OK != lRet )
    	    {        
		vty_out( vty, " %% undo isolate set Failed !\r\n" );
	       RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
		}
#ifdef _DISTRIBUTE_PLATFORM_
    		if(VOS_OK == lRet)
    		{
    			IFM_InterfaceActSyncSlave(
    				IFM_MSG_INTERFACE_CONFIG,\
    				IFM_CONFIG_ETH_ISOLATE, ulPortIfindex, 0, 
    				&ulIsolate, sizeof( ulIsolate )
    				);
    		}
#endif
    }
    END_PARSE_PORT_LIST_TO_PORT();
     
    vty_out( vty, " undo isolate set OK!\r\n" );
#endif

	return CMD_SUCCESS;
}
/*查看端口隔离，add by shixh20090507*/
QDEFUN ( onu_GT813_show_Isolate,
         onu_GT813_show_Isolate_Cmd,
         "isolate_show",
         "show port isolate information\n" ,
          &g_ulIFMQue )
{
return CMD_SUCCESS;
}
DEFUN ( onu_Cable_Test_Port,
         onu_Cable_Test_Port_Cmd,
         "cable test port <1-26> ",
         "calbe test \n"
         "calbe test \n"
         "cable test port \n"
         "port number\n"
          )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    int unit = 0,port = 0,rv = 0;
    bcm_port_cable_diag_t diag_sta = {0};
	
    if(argc==1)
     {  
        port = ( int ) VOS_AtoL(argv[0])-1;
        if( port >23 )
        {
    		vty_out( vty, " %% You can not test port %s \r\n" , argv[0] );
    		return CMD_WARNING ;
    	}
        rv = bcm_port_cable_diag(unit,port,&diag_sta);
	  if (rv<0)
	  	{
	  	   vty_out( vty, "\r\nbcm_port_cable_diag error!rv=%d\r\n",rv);
		   return VOS_ERROR;
	  	}
	  if (BCM_PORT_CABLE_STATE_OK == diag_sta.state)
	  	{
                vty_out( vty, "\r\nport %s cable status is OK \r\n",argv[0]);
		   return CMD_SUCCESS;
	      }
	  else if (BCM_PORT_CABLE_STATE_OPEN == diag_sta.state)
	  	{
                vty_out( vty, "\r\nport %s cable status is OPEN \r\n",argv[0]);
		   vty_out( vty, "pair length = %d m\r\n",diag_sta.pair_len[0]);
	       }
	  else if (BCM_PORT_CABLE_STATE_SHORT == diag_sta.state)
	  	{
                vty_out( vty, "\r\nport %s cable status is SHORT \r\n",argv[0]);
		   vty_out( vty, "pair length = %d m\r\n",diag_sta.pair_len[0]);
	       }	
	  else 
	  	{
                vty_out( vty, "\r\nport %d cable status is UNKNOWN \r\n",port);
                return CMD_WARNING;
	       }	  
     }
#endif
    return CMD_SUCCESS;
}

/*
DEFUN ( config_onu_sys_contact_func,
        config_onu_sys_contact_cmd,
        "config syscontact <.contact>",
        CONFIG_STR
        "Comment the person who manage this host and how to contact this person\n"
        "Please specify contact string (up to 255 characters)\n" )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    int i;
    char *errmsg = NULL;
    int ret;
    struct buffer *b;
    char *new_str;
   
    b = buffer_new( 0, 256 );
    if ( b == NULL )           
        return CMD_ERR_NOTHING_TODO;                 

    for ( i = 0; i < argc; i++ )
    {
        if(i>0) { buffer_putc( b, ' ' );}   
        buffer_putstr( b, ( unsigned char * ) argv[ i ] );
    }

    buffer_putc( b, '\0' );
    new_str = buffer_getstr( b );
    ret = mn_set_syscontact( new_str, &errmsg, 1 );
    if ( ret )
    {
        vty_out( vty, " System contact is set to:%s %s%s", VTY_NEWLINE , new_str, VTY_NEWLINE);        
    }
    else
    {
        vty_out( vty, "  %% Error:%s%s", errmsg, VTY_NEWLINE );
    }

    buffer_free( b );
    VOS_Free( new_str );
    
    vty->node = ONU_DEVICE_NODE; 
    
#endif

    return CMD_SUCCESS;
}

DEFUN( show_onu_sys_contact_func,
       show_onu_sys_contact_cmd,
       "show syscontact",
       SHOW_STR
       "Show the person who manage this host and how to contact this person\n" )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    char * syscontact;
    syscontact = (char *)mn_get_syscontact();
    vty_out( vty, "  %s%s", syscontact, VTY_NEWLINE );
    vty->node = ONU_DEVICE_NODE; 

#endif

    return CMD_SUCCESS;
}


DEFUN ( config_onu_sys_location_func,
        config_onu_sys_location_cmd,
        "config syslocation <.location>",
        CONFIG_STR
        "Comment the physical location of this host\n"
        "Please specify system's location (up to 255 characters)\n" )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    int i;
    int ret;
    char *errmsg = NULL;
    struct buffer *b;
    char *new_str;
    
    b = buffer_new( 0, 256 );
    if ( b == NULL )             
        return CMD_ERR_NOTHING_TODO;                  
    
    for ( i = 0; i < argc; i++ )
    {
        if(i>0) {buffer_putc( b, ' ' );}
        buffer_putstr( b, ( unsigned char * ) argv[ i ] );
    }
    buffer_putc( b, '\0' );
    new_str = buffer_getstr( b );
    ret = mn_set_syslocation( new_str, &errmsg, 1 );
    if ( ret )
    {
        vty_out( vty, "  System location is set to:%s  %s%s", VTY_NEWLINE, new_str, VTY_NEWLINE );        
    }
    else
    {
        vty_out( vty, "  %% Error:%s%s", errmsg, VTY_NEWLINE );
    }
    VOS_Free( new_str );
    buffer_free( b );
    vty->node = ONU_DEVICE_NODE; 
    
#endif
    return CMD_SUCCESS;
}

DEFUN( show_onu_sys_location_func,
       show_onu_sys_location_cmd,
       "show syslocation",
       SHOW_STR
       "Show the physical location of this host\n" )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    char * syslocation;
    syslocation = ( char *)mn_get_syslocation();
    vty_out( vty, "  %s%s", syslocation, VTY_NEWLINE );
    vty->node = ONU_DEVICE_NODE; 
#endif
    return CMD_SUCCESS;
}
*/

DEFUN( cl_onu_reboot,
       cl_onu_reboot_cmd,
       "reboot",
       "Reboot the ONU\n" )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  	
extern void mn_reset();    
    mn_reset();
#else
	LONG lRet;
	CHAR pBuff[512]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	/*int i;*/

	ulIfIndex =(ULONG) vty->index;

	
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;
	
	ponID = GetPonPortIdxBySlot(ulSlot, ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( lRet !=1 )
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;	
	lRet = Comm_Cli_request_transmit(vty, ponID, ulOnuid, pBuff, length , stPayload );
	VOS_Free(stPayload);
	/*if( lRet != VOS_OK )
	{
	       vty_out( vty, " %% Reboot onu error!\r\n" ); 
		return CMD_WARNING;
	}*/
	vty_out( vty, " Please wait for the onu reboot about 1~2 minutes ...... \r\n" );
#endif

    return CMD_SUCCESS;

}

/* B--added by liwei056@2009-3-5 for BroadCast-Cli */
DEFUN( cl_onu_cmdbroadcast,
       cl_onu_cmdbroadcast_cmd,
       "command broadcast [enable|disable] [<slot/port>|all]",
       "The command options setting\n"
       "BroadCast the command to all ONU\n"
       "Enabled Command's BroadCast\n"
       "Disabled Command's BroadCast\n"
       "The selected PON of the OLT\n"
       "All PONs of the OLT\n")
{
    if ( 0 == VOS_StrCmp("all", argv[1]) )
    {
        if ( 'e' == argv[0][0] )
        {
            if ( !BROADCAST_VTY_ISSET(vty, MAXPON) )
            {
                BROADCAST_VTY_SETMAXID(vty, MAXPON);
            }

            BROADCAST_VTY_SETALL(vty);
        }
        else
        {
            BROADCAST_VTY_CLRALL(vty);
        }
    }
    else
    {
    	ULONG   ulSlot = 0, ulPort = 0;
    	int ponID;

        VOS_Sscanf(argv[1], "%d/%d", &ulSlot, &ulPort);
    	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
    		return(CMD_WARNING);
        
    	ponID = GetPonPortIdxBySlot(ulSlot, ulPort);
    	if(ponID == (VOS_ERROR))
    	{
    		vty_out( vty, "  %% Parameter error\r\n");
    		return CMD_WARNING;
    	}

        if ( 'e' == argv[0][0] )
        {
            if ( !BROADCAST_VTY_ISSET(vty, MAXPON) )
            {
                BROADCAST_VTY_SETMAXID(vty, MAXPON);
            }
            
            BROADCAST_VTY_SETID(vty, ponID);
        }
        else
        {
            BROADCAST_VTY_CLRID(vty, ponID);
        }
    }

    if ( vty->node == DEBUG_HIDDEN_NODE )
    {
        /* 隐藏节点下的命令行广播，设置为非安全模式 */
        if (BROADCAST_VTY_IS_SAFE(vty))
        {
            BROADCAST_VTY_CLR_SAFE(vty);
        }
    }
    else
    {
        /* 其它节点下的命令行广播，设置为安全模式 */
        BROADCAST_VTY_SET_SAFE(vty);
    }

    return CMD_SUCCESS;
}

DEFUN (cl_onu_cmdbroadcast_show,
       cl_onu_cmdbroadcast_show_cmd,
       "show command-options broadcast",
       SHOW_STR
       "Show command's options setting\n"
       "Show command's broadcast setting\n")
{
    int i, m;

    if ( BROADCAST_VTY_ISEMPTY(vty) )
    {
    	vty_out(vty, "PON: None in broadcast.\r\n");
        return CMD_SUCCESS;
    }

    if ( BROADCAST_VTY_ISFULL(vty) )
    {
    	vty_out(vty, "PON: All in broadcast.\r\n");
        return CMD_SUCCESS;
    }

    vty_out(vty, "PON: \r\n");
    m = BROADCAST_VTY_GETMAXNUM(vty);
    for (i=0; i<m; ++i)
    {
        if ( BROADCAST_VTY_GETID(vty, i) )
        {
            vty_out(vty, "  pon%d/%d\r\n", GetCardIdxByPonChip(i), GetPonPortByPonChip(i) );
        }
    }
    vty_out(vty, "in broadcast.\r\n");

    return CMD_SUCCESS;
}
/* E--added by liwei056@2009-3-5 for BroadCast-Cli */


DEFUN (onu_mac_show,
       onu_mac_show_cmd,
       "show sysmac",
       SHOW_STR
       "Show onu's mac setting\n")
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    Enet_MACAdd_Infor macStructure;
    LONG lRet = 0;

    vty->node = ONU_DEVICE_NODE;
    lRet = device_get_sys_mac_addr_( &macStructure ); 
    if( lRet == VOS_OK )
    	vty_out(vty, " The sysmac is: %2.2X%2.2X.%2.2X%2.2X.%2.2X%2.2X \r\n", 
    	macStructure.bMacAdd[ 0 ], 	macStructure.bMacAdd[ 1 ], macStructure.bMacAdd[ 2 ], 
    	macStructure.bMacAdd[ 3 ], 	macStructure.bMacAdd[ 4 ], macStructure.bMacAdd[ 5 ] );
    else
    	vty_out(vty, " %%The sysmac is invalid! Please config the mac address !\r\n ");
#endif
    return CMD_SUCCESS;
}
/*
QDEFUN( Show_onu_interface_brief_Func,
        Show_onu_interface_brief_CMD,
        "show interface brief",
        SHOW_STR
        "Show interfaces in the onu\n"
        "Show interfaces' brief information\n",
        &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    CHAR * szTypeInfo = NULL ;
    LONG lRet ;

    lRet = Interfce_Type_Show( &szTypeInfo );
    if ( lRet < 0 )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }

    if ( szTypeInfo != NULL )
    {
        vty_big_out( vty, lRet, szTypeInfo );
        VOS_Free( szTypeInfo );
    }
#endif
    return CMD_SUCCESS;
}
*/

/*VLAN CLI*/

QDEFUN ( onu_create_vlan_interface,
         onu_create_vlan_interface_cmd,
         "interface vlan <vlanname> {<1-4094>}*1",
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Vlan's name(First character must be 'a'-'z' or 'A'-'Z', fallowed by 'a'-'z','A'-'Z','0'-'9'or'_', not more than 30)\n"
         "Specify vlan's ID\n" ,
    &g_ulIFMQue  )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    LONG lRet = VOS_OK;
    VLAN_ELEMENT_S stVLanIData;
    CHAR prompt[ 64 ] = {0};
    ULONG ulIfindex;
    ULONG ulVID;
	UCHAR ucForwardType;

    lRet = ifm_check_vlan_name( argv[0] );

    GOTO_LRET( lRet, error );

    VOS_MemZero( &stVLanIData, sizeof( VLAN_ELEMENT_S ) );

    if ( argc == 2 )
    {
        stVLanIData.ulVLanID = ( ULONG ) VOS_AtoL( argv[ 1 ] );
    }

    ulIfindex = IFM_GetIfindexByName( argv[ 0 ] );
    if ( ulIfindex == 0 )
    {
	    if(g_lVlanRunMode == IFM_VLAN_MODE_TRSP)
	    {
            vty_out( vty, " %%Can't create vlan transparent vlan mode.\r\n" );
		    return  CMD_WARNING;
	    }
	
        lRet = vlan_check_name( argv[ 0 ] );
        GOTO_LRET( lRet, error );

#ifdef _DISTRIBUTE_PLATFORM_
        stVLanIData.ulFlag |= VLAN_ACTUAL;
#endif
        VOS_StrnCpy( stVLanIData.szName, argv[ 0 ], IFM_NAME_SIZE );
        stVLanIData.ucType = IFM_VLAN_STATIC;
        stVLanIData.ucForwardType = VLAN_PORT_BASED;
        lRet = IFM_register( IFM_VLAN_TYPE, &ulIfindex, ( VOID * ) & stVLanIData, NULL );
        GOTO_LRET( lRet, error );
        
	ucForwardType = VLAN_PORT_BASED;
	vty_out( vty, " Create vlan %s OK.\r\n", argv[0] );
    }
    else 
    	vty_out( vty, "  %% Vlan %s has been created!\r\n", argv[0] );
   
    vty->node = ONU_DEVICE_NODE;     

    return CMD_SUCCESS;
    
error:
    IFM_PError( lRet, vty );
    return CMD_WARNING;
    
#endif    
    return CMD_SUCCESS;
}

QDEFUN( onu_normal_vlan_vlanmode_FUN,
        onu_normal_vlan_vlanmode_CMD,
        "vlanmode [801q|transparent]",
        "Config vlan mode\n"
        "Config vlan to 801.Q mode\n"
        "Config vlan to transparent mode\n",     
        &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
	ULONG lstate = 0;
	ULONG lTpid  = 0;
	LONG lRet   = VOS_OK;

	vty->node = ONU_DEVICE_NODE;
	if ( 0 == VOS_StrCmp( argv[ 0 ], "801q" ) )
	{
		if(g_lVlanRunMode == IFM_VLAN_MODE_801Q)
		{
			vty_out( vty, "Vlan mode is already 801q.\r\n" );
		}
		else
		{
			lstate         = IFM_VLAN_MODE_801Q;
			g_lVlanRunMode = IFM_VLAN_MODE_801Q;
			lRet = VLAN_ext( 0, IFM_MSG_VLANIF_VLANMODE, NULL, &lstate ,NULL );
		}
	}
	else if ( 0 == VOS_StrCmp( argv[ 0 ], "transparent" ) )
	{
		if(g_lVlanRunMode == IFM_VLAN_MODE_TRSP)
		{
			vty_out( vty, "Vlan mode is already transparent.\r\n" );
		}
		else
		{	
			ULONG          iIfIdx;
			LONG           lRet;
			VLAN_ELEMENT_S stVLanIData;

			lstate         = IFM_VLAN_MODE_TRSP;
			VOS_MemZero( &stVLanIData, sizeof( VLAN_ELEMENT_S ) );
			stVLanIData.ucType = IFM_VLAN_ALL;
			/*删除所有的VLAN*/
			for ( iIfIdx = IFM_GET_FIRST_INTERFACE( IFM_VLAN_TYPE ); 
				iIfIdx != 0; iIfIdx = IFM_GET_NEXT_INTERFACE( iIfIdx ) )
			{
				if(IFM_VLAN_GET_VLANID(iIfIdx) == IFM_VLAN_DEFAULT_VLAN_ID ) 
					continue;
				lRet = IFM_unregister( iIfIdx, &stVLanIData, NULL );
				if ( lRet != VOS_OK )
				{
					lstate = g_lVlanRunMode;
					vty_out( vty, "Delete Vlan fail.\r\n" );
				}
			}
			if(lstate == IFM_VLAN_MODE_TRSP)
			{
				g_lVlanRunMode = IFM_VLAN_MODE_TRSP;
				lRet = VLAN_ext( 0, IFM_MSG_VLANIF_VLANMODE, NULL, &lstate ,NULL );
			}
		}
	}
	
	if ( lRet != VOS_OK ) 
		return CMD_WARNING;
	vty_out( vty, " Vlanmode set OK!\r\n");
#endif
    return CMD_SUCCESS;
}

DEFUN( show_onu_normal_vlan_vlanmode_FUN,
        show_onu_normal_vlan_vlanmode_CMD,
        "show vlanmode",
        "show Config vlan mode\n")
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    vty->node = ONU_DEVICE_NODE;
    if(g_lVlanRunMode == IFM_VLAN_MODE_801Q)
    {
        vty_out( vty, "Vlan mode is 801q.\r\n" );
    }
    else if (g_lVlanRunMode == IFM_VLAN_MODE_TRSP)
	{
       vty_out( vty, "Vlan mode is transparent.\r\n" );
    }
    else
	{
       vty_out( vty, "Vlan mode is vlanstack.\r\n" );
    }
#endif    
    return CMD_SUCCESS;
}

/*创建组播VLAN 组播VLAN和协议VLAN在一个节点下*/
QDEFUN ( onu_create_vlan_multicast_interface,
         onu_create_vlan_multicast_interface_cmd,
         "interface mvlan <vlanname> {<1-4094>}*1",
         "Select an interface to config\n"
         "Config multicast vlan interface\n"
         "Vlan's name(First character must be 'a'-'z' or 'A'-'Z', fallowed by 'a'-'z','A'-'Z','0'-'9'or'_', not more than 30)\n"
         "Specify vlan's ID\n",
         &g_ulIFMQue )
{
#if( PRODUCT_CLASS == GT813 )
    LONG lRet = VOS_OK;
    VLAN_ELEMENT_S stVlanElement;
    CHAR prompt[ 64 ] = {0};
    ULONG ulIfindex;
    ULONG ulVID;

#ifndef _DISTRIBUTE_PLATFORM_    
    lRet = ifm_check_valid_interface_name( argv[ 0 ] );
#else
    lRet = ifm_check_vlan_name( argv[0] );
#endif
    GOTO_LRET( lRet, error );

    VOS_MemZero( &stVlanElement, sizeof( VLAN_ELEMENT_S ) );

    if ( argc == 2 )
    {
        stVlanElement.ulVLanID = ( ULONG ) VOS_AtoL( argv[ 1 ] );
    }

    ulIfindex = IFM_GetIfindexByName( argv[ 0 ] );
    if ( ulIfindex == 0 )
    {        
	    if(g_lVlanRunMode == IFM_VLAN_MODE_TRSP)
	    {
            vty_out( vty, " %% Can't create vlan transparent vlan mode.\r\n" );
		    return  CMD_WARNING;
	    }
	
#ifdef _DISTRIBUTE_PLATFORM_
        stVlanElement.ulFlag |= VLAN_ACTUAL;
#endif
        VOS_StrnCpy( stVlanElement.szName, argv[ 0 ], IFM_NAME_SIZE );
        stVlanElement.ucForwardType = VLAN_MVLAN_BASED;
        stVlanElement.ucType = IFM_VLAN_STATIC;
        lRet = IFM_register( IFM_VLAN_TYPE, &ulIfindex, ( VOID * ) & stVlanElement, NULL );
        GOTO_LRET( lRet, error );

#ifdef _DISTRIBUTE_PLATFORM_
        if(VOS_OK == lRet)
        {
            vty_out( vty, " Create mvlan %s OK.\r\n", argv[0] );
        }
#endif
    }
    else
    {
        struct net_device   *pNetDev      = NULL;
        VLAN_P_DATA_S       *pstVLanPData = NULL;

        if ( !IFM_IS_VLAN_INDEX( ulIfindex ) )
        {
            vty_out( vty, "  %% Interface %s is not VLAN.\r\n", argv[ 0 ] );
            return CMD_SUCCESS;
        }
		
		if(IFM_find_netdev( ulIfindex, &pNetDev, NULL) != VOS_OK) return (CMD_WARNING);

        pstVLanPData = ( VLAN_P_DATA_S * )pNetDev->pPrivateData;
		
        if(VLAN_IS_AMVLAN(pstVLanPData))
        {
            pstVLanPData->ucForwardType = VLAN_MVLAN_BASED;
        }
		else
		{  
		    if(!VLAN_IS_MVLAN(pstVLanPData))
		    {
                vty_out( vty, "  %% This VLAN has been created, but MVLAN.\r\n" );
		        return(CMD_SUCCESS);
		    }
		}

        if( argc == 2 )
        {
            ulVID = IFM_VLAN_GET_VLANID( ulIfindex );
            if ( ulVID != stVlanElement.ulVLanID )
            {
                vty_out( vty, "  %% ID of this VLAN is %d.\r\n", ulVID );
                return CMD_SUCCESS;
            }
        }
    }
    vty->node = ONU_DEVICE_NODE;    
    
    return CMD_SUCCESS;

error:
    IFM_PError( lRet, vty );

#endif    
    return CMD_SUCCESS;
}

/*创建PVLAN PVLAN和协议VLAN在一个节点下*/
QDEFUN ( onu_create_vlan_portstream_interface,
         onu_create_vlan_portstream_interface_cmd,
         "interface pvlan <vlanname> {<1-4094>}*1",
         "Select an interface to config\n"
         "Config port vlan interface only for down stream packets\n"
         "Vlan's name(First character must be 'a'-'z' or 'A'-'Z', fallowed by 'a'-'z','A'-'Z','0'-'9'or'_', not more than 30)\n"
         "Specify vlan's ID\n",
         &g_ulIFMQue )
{
#if( PRODUCT_CLASS == GT813 )
    LONG lRet = VOS_OK;
    VLAN_ELEMENT_S stVlanElement;
    CHAR prompt[ 64 ] = {0};
    ULONG ulIfindex;
    ULONG ulVID;

#ifndef _DISTRIBUTE_PLATFORM_    
    lRet = ifm_check_valid_interface_name( argv[ 0 ] );
#else
    lRet = ifm_check_vlan_name( argv[0] );
#endif
    GOTO_LRET( lRet, error );

    VOS_MemZero( &stVlanElement, sizeof( VLAN_ELEMENT_S ) );

    if ( argc == 2 )
    {
        stVlanElement.ulVLanID = ( ULONG ) VOS_AtoL( argv[ 1 ] );
    }

    ulIfindex = IFM_GetIfindexByName( argv[ 0 ] );
    if ( ulIfindex == 0 )
    {     
	    if(g_lVlanRunMode == IFM_VLAN_MODE_TRSP)
	    {
            vty_out( vty, " %% Can't create vlan transparent vlan mode.\r\n" );
		    return  CMD_WARNING;
	    }
	
#ifdef _DISTRIBUTE_PLATFORM_
        stVlanElement.ulFlag |= VLAN_ACTUAL;
#endif
        VOS_StrnCpy( stVlanElement.szName, argv[ 0 ], IFM_NAME_SIZE );
        stVlanElement.ucForwardType = VLAN_PVLAN_BASED;
        stVlanElement.ucType = IFM_VLAN_STATIC;
        lRet = IFM_register( IFM_VLAN_TYPE, &ulIfindex, ( VOID * ) & stVlanElement, NULL );
        GOTO_LRET( lRet, error );

#ifdef _DISTRIBUTE_PLATFORM_
        if(VOS_OK == lRet)
        {
            vty_out( vty, " Create pvlan %s OK.\r\n", argv[0] );
        }
#endif
    }
    else
    {
        UCHAR ucForwardType;

        if ( !IFM_IS_VLAN_INDEX( ulIfindex ) )
        {
            vty_out( vty, "  %% Interface %s is not VLAN.\r\n", argv[ 0 ] );
            return CMD_WARNING;
        }

        lRet = VLAN_get_info( ulIfindex, IFM_CONFIG_VLAN_FORWARD_TYPE, ( VOID * ) & ucForwardType, NULL );
        ASSERT( lRet == VOS_OK );
        GOTO_LRET( lRet, error );
        if ( ucForwardType != VLAN_PVLAN_BASED )
        { 
            vty_out( vty, "  %% This VLAN has been created, but not multi-cast VLAN.\r\n" );
            return CMD_WARNING;
        }

        if ( argc == 2 )
        {
            ulVID = IFM_VLAN_GET_VLANID( ulIfindex );
            if ( ulVID != stVlanElement.ulVLanID )
            {
                vty_out( vty, "  %% ID of this VLAN is %d.\r\n", ulVID );
                return CMD_WARNING;
            }
        }
    }
    vty->node = ONU_DEVICE_NODE;    

    return CMD_SUCCESS;

error:
    IFM_PError( lRet, vty );

#endif    
    return CMD_SUCCESS;
}
      
QDEFUN( onu_vlan_del_add_port_func,
        onu_vlan_del_add_port_cmd,
        "vlan <vlanname> [add|delete] port <port_list> [tagged|untagged]",
        "Config vlan interface\n"
        "Vlan's name(First character must be 'a'-'z' or 'A'-'Z', fallowed by 'a'-'z','A'-'Z','0'-'9'or'_', not more than 30)\n"
        "Add ports into vlan\n"
        "Delete ports from vlan\n"
        "Change ports in the vlan\n"
        "Specify port number list you want to change(e.g.: 1-3, 5,6-9)\n"
        "IEEE 802.1q tagged mode\n"
        "IEEE 802.1q untagged mode\n",
    &g_ulIFMQue )      
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    LONG lRet = VOS_OK;
    VLAN_ELEMENT_S stVLanIData;
    CHAR prompt[ 64 ] = {0};
    ULONG ulIfindex;
    ULONG ulVID;
    UCHAR ucForwardType;
    ULONG               ulVlanIfindex;
    IFM_NET_DEVICE_S    *pNetd = NULL;
    VLAN_P_DATA_S       *pstVlanPData = NULL;
    ULONG               ulTagged;
    ULONG               ulAction;
    ULONG               ulPortIfindex;  
    ULONG               ulPort;
#ifdef _DISTRIBUTE_PLATFORM_
    CHAR                *pszError = NULL;   
#endif
    ULONG ulUserSlot = 0;
    ULONG ulUserPort = 0;

    lRet = ifm_check_vlan_name( argv[0] );    
    if( lRet != VOS_OK )
    {
        vty_out( vty, " %% Can not find the vlan%s, \r\n", argv[0]);
        return CMD_WARNING ;
    }
    
    VOS_MemZero( &stVLanIData, sizeof( VLAN_ELEMENT_S ) );    

    ulIfindex = IFM_GetIfindexByName( argv[ 0 ] );
    if ( ulIfindex == 0 )
    {
    	vty_out( vty, "  %% Can not find the vlan %s, you should use command:[ interface vlan] to create it at first. \r\n",  argv[0] );
    	return CMD_WARNING;
    }
   
    vty->node = ONU_DEVICE_NODE; 
    
    ulVlanIfindex = ulIfindex;

    /**
     * 找到vlan私有信息结构
     */
    lRet = IFM_find_netdev( ulVlanIfindex, &pNetd, NULL );
    if ( lRet != VOS_OK || pNetd == NULL )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
    pstVlanPData = ( VLAN_P_DATA_S * ) pNetd->pPrivateData;

    /**
     * 不能在ip-based 和 default vlan中执行添加/删除端口
     */
    if ( VLAN_IS_IPSUB( pstVlanPData ) )
    {
        vty_out( vty, "  %% This vlan is ip-based, can't join or leave with tag or untag mode.\r\n" );
        return CMD_WARNING;
    }
	if(VLAN_IS_AMVLAN( pstVlanPData ))
	{
        vty_out( vty, "  %% Ports can not join or leave AMVLAN.\r\n" );
        return CMD_WARNING;
	}
	
    if ( !VOS_StrCmp( "default", pNetd->szName ) )
    {
        vty_out( vty, "  %% Can not change port information of default vlan.\r\n" );
        return CMD_WARNING;
    }

    if ( VOS_StrCmp( "add", argv[1] ) == 0 )
    {
        ulAction = 0;
    }
    else
    {
        ulAction = 1;
    }

    if ( VOS_StrCmp( "tagged", argv[argc - 1] ) == 0 )
    {
        ulTagged = IFM_VLAN_PORT_TAGGED;
    }
    else
    {
        ulTagged = IFM_VLAN_PORT_UNTAGGED;
    }

    if ( ONU_CheckPortListValid( argv[2] ) != 1 )
    {
        vty_out( vty, "  %% Invalid port list <%s>.\r\n", argv[ 2 ] );
        return CMD_WARNING;
    }

#ifdef _DISTRIBUTE_PLATFORM_
    {
        /* VLAN上使能了PPPOE后不能添加删除端口 */
        ULONG ulEnablePPPoE = 0;
        ULONG ulCanOrCannot = 0;
        LONG  lReason       = 0;
        
        lRet = IFM_get_info( ulVlanIfindex, IFM_CONFIG_VLAN_OVERRIDE, &ulEnablePPPoE, NULL );
        if ( VOS_OK != lRet )
        {
            IFM_PError( lRet, vty );
            return CMD_WARNING;
        }
        if ( 0 != ulEnablePPPoE )
        { 
            vty_out( vty, "  %% Can not change port information because PPPoE has been enabled.\r\n");
            return CMD_WARNING;
        }
        lRet = IFM_InterfaceAbilityCheck(ulVlanIfindex, IFM_CAN_BIND_VPN, &ulCanOrCannot, &lReason);
        CHECK_LRET(lRet);
        if((ulCanOrCannot == IFM_ABILITY_CANOT)
           && (lReason == IFM_E_VLAN_JL2VPN) )
        {
            vty_out( vty, "  %% Can not change port information because L2VPN has been enabled.\r\n");
            return CMD_WARNING;
        }
        
    }
#endif

    BEGIN_PARSE_PORT_LIST_TO_PORT( argv[2], ulPort )
    {
        UCHAR   aucString[128];
        VOS_MemZero( aucString, 128 );
        
        if ( ulAction == 0 ) /* 添加 */
        {
            ulUserSlot = /*IFM_ETH_GET_SLOT( ulPortIfindex )*/1;
            ulUserPort = /*IFM_ETH_GET_PORT( ulPortIfindex )*/ ulPort;
            ulPortIfindex = IFM_ETH_CREATE_INDEX( 1, ulPort );

            lRet = IFM_find_netdev( ulPortIfindex, &pNetd, NULL );
            if ( lRet != VOS_OK )
            {
                VOS_Snprintf( aucString, 127, " %%Port %d join VLAN failed", ulUserPort );
                IFM_PError2( aucString, lRet, vty );
                continue;
            }
            
#ifndef _DISTRIBUTE_PLATFORM_
            lRet = VLAN_join( ulPortIfindex, ulVlanIfindex, ( VOID * ) &ulTagged, NULL );
            if ( lRet != VOS_OK )
            {
                IFM_PError2( aucString, lRet, vty );
            }
#else
            lRet = VLAN_join( ulPortIfindex, ulVlanIfindex, ( VOID * ) &ulTagged, &pszError );
            if ( lRet != VOS_OK )
            {
                if ( pszError != NULL )
                {
                    vty_out( vty, "  %s", pszError );
                }
                else
                {
                    IFM_PError( lRet, vty );
                }
                
                RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
            }
            else
            {
                /*IFM_InterfaceActSyncSlave(
                    IFM_MSG_INTERFACE_JOIN,
                    0,
                    ulPortIfindex,
                    ulVlanIfindex,
                    ( VOID * )&ulTagged,
                    sizeof( ulTagged ) );*/
            }
#endif
        }
        else /* 删除 */
        {
            ulUserSlot = /*IFM_ETH_GET_SLOT( ulPortIfindex )*/1;
            ulUserPort = /*IFM_ETH_GET_PORT( ulPortIfindex )*/ ulPort;
            ulPortIfindex = IFM_ETH_CREATE_INDEX( 1, ulPort );

            lRet = IFM_find_netdev( ulPortIfindex, &pNetd, NULL );
            if ( lRet != VOS_OK )
            {
                VOS_Snprintf( aucString, 127, " %% Port %d leave VLAN failed", ulUserPort );
                IFM_PError2( aucString, lRet, vty );
                continue;
            }
            lRet = VLAN_leave( ulPortIfindex, ulVlanIfindex, ( VOID * ) &ulTagged, NULL );
            if ( lRet != VOS_OK )
            {
                IFM_PError2( aucString, lRet, vty );
            }
        }
    }
    END_PARSE_PORT_LIST_TO_PORT();
    vty_out( vty, " Vlan %s port operation OK!\r\n", argv[1] );
    return lRet;
    
#endif
    return CMD_SUCCESS;
}

/*This cli need to do more work.*/
QDEFUN( show_onu_vlan_Func,
        show_onu_vlan_cmd,
        "show interface vlan {[count]}*1 {[port-based|pvlan|mvlan]}*1 {<name>}*1",
        DescStringCommonShow
        "Show interfaces in the onu\n"
        "Show interfaces vlan in the onu\n"
        "Show interfaces vlan's count in the onu\n"        
        "Show port-based vlan in the onu\n"
        "Show pvlan in the onu\n"
        "Show mvlan in the onu\n"
        "Specify vlan's name\n",
        &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    LONG lRet           = VOS_OK;
    ULONG ulBase        = 0;
    ULONG i             = 0;
    ULONG ulIfindex     = 0;
    ULONG ulCount       = 0;
    ULONG ulShowCount   = FALSE;
    ULONG ulShowType    = IFM_VLAN_ALL;
    ULONG ulShowFwType  = VLAN_ALL_BASED;
    CHAR *pszName       = NULL;
    CHAR *pcPrint       = NULL;
    VLAN_P_DATA_S * pstVLanPData = NULL;

    if( argc==ulBase ) { goto _SHOWVLAN; }
    if( 0==VOS_StrCmp( "count", argv[ulBase] ) )
    {
        ulShowCount = TRUE;
        ulBase++;
    }
    
    if( argc==ulBase ) { goto _SHOWVLAN; }
#if 0
    if( 0==VOS_StrCmp( "static", argv[ulBase] ) )
    {
        ulShowType = IFM_VLAN_STATIC;
        ulBase++;
    }
    else if( 0==VOS_StrCmp( "dynamic", argv[ulBase] ) )
    {
        ulShowType = IFM_VLAN_DYNAMIC;
        ulBase++;
    }
#endif    
    if( argc==ulBase ) { goto _SHOWVLAN; }
    /*if( 0==VOS_StrCmp( "protocol-based", argv[ulBase] ) )
    {
        ulShowFwType = VLAN_PROTOCAL_BASED;   
        ulBase++;
    }
    else*/ if( 0==VOS_StrCmp( "port-based", argv[ulBase] ) )
    {
        ulShowFwType = VLAN_PORT_BASED;       
        ulBase++;
    }
    else if( 0==VOS_StrCmp( "ip-based", argv[ulBase] ) )
    {
        ulShowFwType = VLAN_IP_BASED;         
        ulBase++;
    }
	else  if( 0==VOS_StrCmp( "mvlan", argv[ulBase] ) )
    {
        ulShowFwType = VLAN_MVLAN_BASED;         
        ulBase++;
    }
	else if( 0==VOS_StrCmp( "pvlan", argv[ulBase] ) )
    {
        ulShowFwType = VLAN_PVLAN_BASED;         
        ulBase++;
    }
	else if( 0==VOS_StrCmp( "amvlan", argv[ulBase] ) )
    {
        ulShowFwType = VLAN_AMVLAN_BASED;         
        ulBase++;
    }

    if( argc==ulBase ) { goto _SHOWVLAN; }
    pszName = argv[ulBase]; ulBase++;
_SHOWVLAN:    
    if( NULL!=pszName )
    {
        lRet = ifm_check_vlan_name( pszName );
        if( VOS_OK!=lRet )
        {
            IFM_PError( lRet, vty );
            return CMD_WARNING;
        }

        ulIfindex = IFM_GetIfindexByName( pszName );
        if ( ulIfindex == 0 )
        {
            vty_out( vty, "  %% VLAN %s does not exist.\r\n", pszName );
            return CMD_WARNING;
        }

        if ( !IFM_IS_VLAN_INDEX( ulIfindex ) )
        { /*interface type check*/
            vty_out( vty, "  %% Interface %s is not VLAN.\r\n", pszName );
            return CMD_WARNING;
        }
        
        pstVLanPData = IFM_get_private_data( ulIfindex );
        if( NULL==pstVLanPData )
        {
            ASSERT( 0 );
            return CMD_WARNING;
        }

        if( IFM_VLAN_ALL!=ulShowType && ulShowType!=pstVLanPData->ucType )
        {
            vty_out( vty, "  %% Interface %s is not ", pszName );
            switch( ulShowType )
            {
                case IFM_VLAN_STATIC:
                    vty_out( vty, "static " );
                    break;
                case IFM_VLAN_DYNAMIC:
                    vty_out( vty, "dynamic" );
                    break;
                default:
                    ASSERT( 0 );
            }
            vty_out( vty, " VLAN.\r\n" );

            return CMD_WARNING;
        }
        
        if( VLAN_ALL_BASED!=ulShowFwType && ulShowFwType!=pstVLanPData->ucForwardType )
        {
            vty_out( vty, "  %% Interface %s is not ", pszName );
            switch( ulShowFwType )
            {
                case VLAN_PORT_BASED:
                    vty_out( vty, "port-based" );
                    break;
                case VLAN_PROTOCAL_BASED:
                    vty_out( vty, "protocol-based" );
                    break;
                case VLAN_IP_BASED:
                    vty_out( vty, "ip-based" );
                    break;
                default:
                    ASSERT( 0 );
            }
            vty_out( vty, " VLAN.\r\n" );

            return CMD_WARNING;
        }

        if( TRUE==ulShowCount )
        {
            vty_out( vty, "  \r\nTotal vlan num is: 1\r\n\r\n" );
        }
        else
        {
            lRet = VLAN_show( ulIfindex, 0, &pcPrint, NULL , vty);
            if( lRet > 0 && NULL!=pcPrint )
            {
                vty_big_out( vty, lRet, "%s", pcPrint );
            }
            if( NULL!=pcPrint )
            {
                VOS_Free( pcPrint );
            }
        }
    }
    else
    {
        for ( i = IFM_GET_FIRST_INTERFACE( IFM_VLAN_TYPE );i != 0;i = IFM_GET_NEXT_INTERFACE( i ) )
        {
            pstVLanPData = IFM_get_private_data( i );
            if( NULL==pstVLanPData )
            {
                continue;
            }
            if( IFM_VLAN_ALL!=ulShowType && ulShowType!=pstVLanPData->ucType )
            {
                continue;
            }
            if( VLAN_ALL_BASED!=ulShowFwType && ulShowFwType!=pstVLanPData->ucForwardType )
            {
                continue;
            }
            
            ulCount++;
            if( FALSE==ulShowCount )
            {
                pcPrint = NULL;
                lRet = VLAN_show( i, 0, &pcPrint, NULL , vty );
                if( lRet > 0 && NULL!=pcPrint )
                {
                    vty_big_out( vty, lRet, "%s", pcPrint );
                }
                if( NULL!=pcPrint )
                {
                    VOS_Free( pcPrint );
                }
            }
        }

        if( TRUE==ulShowCount )
        {
            vty_out( vty, "  \r\nTotal vlan num is: %d\r\n\r\n", ulCount );
        }
    }
#endif

    return CMD_SUCCESS;
}


QDEFUN( onu_IpAddr_if_Func,
        onu_IpAddr_if_CMD,
        "vlan <vlanname> ip address <A.B.C.D/M>" ,
        "Config vlan interface\n"
        "Vlan's name(First character must be 'a'-'z' or 'A'-'Z', fallowed by 'a'-'z','A'-'Z','0'-'9'or'_', not more than 30)\n"
        "IP information\n"
        "IP address information\n"
        "IP address and length of mask\n" ,
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    LONG    ipaddr, mask;
    LONG lRet = VOS_OK;
    VLAN_ELEMENT_S stVLanIData;
    ULONG ulIfindex;
    ULONG               ulVlanIfindex;

    lRet = ifm_check_vlan_name( argv[0] );
    if( lRet != VOS_OK )
    {
        vty_out( vty, " %%Can not find the vlan%s, \r\n", argv[0]);
        return CMD_WARNING ;
    }

    VOS_MemZero( &stVLanIData, sizeof( VLAN_ELEMENT_S ) );    

    ulIfindex = IFM_GetIfindexByName( argv[ 0 ] );
    if ( ulIfindex == 0 )
    {
    	vty_out( vty, "  %%Can not find the vlan %s, you should use command:[ interface vlan] to create it at first. \r\n",  argv[0] );
    	return CMD_WARNING;
    }
   
    vty->node = ONU_DEVICE_NODE; 
    
    ulVlanIfindex = ulIfindex;
  
    if ( VOS_OK != IpListToIp( argv[1], &ipaddr, &mask ) )
    {
        vty_out( vty, "%% Invalid IP address.\r\n" );
        return CMD_WARNING;
    }

#ifndef _DISTRIBUTE_PLATFORM_
    lRet = config_ip( ulVlanIfindex, (ULONG)ipaddr, (ULONG)mask, IFM_CONFIG_IP_MASTER );
    if ( lRet != VOS_OK )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
#else
    config_ip_ex( vty, ulVlanIfindex, ipaddr, mask, IFM_CONFIG_IP_MASTER );
#endif

    vty_out( vty, " Vlan %s set ip address OK!\r\n", argv[0] );

#endif
    return CMD_SUCCESS;
}


QDEFUN( onu_NoIPAddr_if_Func,
        onu_NoIpAddr_if_CMD,
        "undo vlan <vlanname> ip address {<A.B.C.D>}*1 " ,        
        NO_STR
        "Config vlan interface\n"
        "Vlan's name(First character must be 'a'-'z' or 'A'-'Z', fallowed by 'a'-'z','A'-'Z','0'-'9'or'_', not more than 30)\n"
        "IP information\n"
        "IP address information\n"
        "IP address and length of mask\n",
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG ulIfIndex;
    long ipaddr;
    IFM_CONFIG_IP stConfigIP;
#ifdef _DISTRIBUTE_PLATFORM_
    CHAR *pszError = NULL;
#endif
    LONG lRet = VOS_OK;
    VLAN_ELEMENT_S stVLanIData;

    lRet = ifm_check_vlan_name( argv[0] );
    if( lRet != VOS_OK )
    {
        vty_out( vty, " %%Can not find the vlan%s, \r\n", argv[0]);
        return CMD_WARNING ;
    }
    
    VOS_MemZero( &stVLanIData, sizeof( VLAN_ELEMENT_S ) );    

    ulIfIndex = IFM_GetIfindexByName( argv[ 0 ] );
    if ( ulIfIndex == 0 )
    {
    	vty_out( vty, " %%Can not find the vlan %s, you should use command:[ interface vlan] to create it at first. \r\n",  argv[0] );
    	return CMD_WARNING;
    }
   
    vty->node = ONU_DEVICE_NODE; 
    
    if ( argc == 2 )
    {
        ipaddr = get_long_from_ipdotstring( argv[ 1 ] );
        stConfigIP.ulIP = ( ULONG ) ipaddr;
        stConfigIP.ulMask = ( ULONG ) 0;
        stConfigIP.ulVPNID = ( ULONG ) 0;
        stConfigIP.ulFlag = IFM_CONFIG_NO_IP_ONE;
    }
    else
    {
        stConfigIP.ulIP = ( ULONG ) 0;
        stConfigIP.ulMask = ( ULONG ) 0;
        stConfigIP.ulVPNID = ( ULONG ) 0;
        stConfigIP.ulFlag = IFM_CONFIG_NO_IP_ALL;
    }

#ifndef _DISTRIBUTE_PLATFORM_
    lRet = IFM_config( ulIfIndex, IFM_CONFIG_GLOBAL_NO_IP, ( VOID* ) & stConfigIP, NULL );

    if ( lRet != VOS_OK )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }
#else

    lRet = IFM_config( ulIfIndex, IFM_CONFIG_GLOBAL_NO_IP, ( VOID* ) & stConfigIP, &pszError);
    if(pszError != NULL)
    {
        vty_out( vty, " %% %s.\r\n", pszError );
        return CMD_WARNING;
    }
    
    if ( lRet != VOS_OK )
    {
        IFM_PError( lRet, vty );
        return CMD_WARNING;
    }   
#endif
    vty_out( vty, " Vlan %s delete ip address OK!\r\n", argv[0] );

#endif
    return CMD_SUCCESS;
}

QDEFUN ( onu_vlan_multicaststate_set,
         onu_vlan_multicaststate_set_cmd,
         "vlan <vlanname> mcastmode <modelvalue> ",
         "Config vlan interface\n"
         "Vlan's name(First character must be 'a'-'z' or 'A'-'Z', fallowed by 'a'-'z','A'-'Z','0'-'9'or'_', not more than 30)\n"
         "set muliticast mode for a vlanl\n"
         "set muliticast mode for a vlanl, 0 muliticast all ; 1 muliticast unregister; 2 drop unregister\n" ,
    &g_ulIFMQue  )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG               ulVlanIfindex;
    ULONG               lstate = 0;
    LONG    ipaddr, mask;
    LONG lRet = VOS_OK;
    VLAN_ELEMENT_S stVLanIData;
    ULONG ulIfindex;

    lRet = ifm_check_vlan_name( argv[0] );
    if( lRet != VOS_OK )
    {
        vty_out( vty, " %% Can not find the vlan%s, \r\n", argv[0]);
        return CMD_WARNING ;
    }

    VOS_MemZero( &stVLanIData, sizeof( VLAN_ELEMENT_S ) );    

    ulIfindex = IFM_GetIfindexByName( argv[ 0 ] );
    if ( ulIfindex == 0 )
    {
    	vty_out( vty, " %% Can not find the vlan %s, you should use command:[ interface vlan] to create it at first. \r\n",  argv[0] );
    	return CMD_WARNING;
    }
   
    vty->node = ONU_DEVICE_NODE; 
    
    ulVlanIfindex = ulIfindex;    
    
    if ( IFM_IFINDEX_GET_TYPE( ulVlanIfindex ) != IFM_VLAN_TYPE )
    {
        vty_out( vty, "  %% This is not a vlan interface.\r\n", argv[0] );
        return CMD_WARNING;
    }

    lstate =(LONG ) VOS_AtoL( argv[ 0 ] );
    if(lstate >=0 && lstate<=2)
    {
        lRet = VLAN_ext( ulVlanIfindex, IFM_MSG_VLANIF_MCASTCTL, NULL, &lstate ,NULL );
        if ( lRet != VOS_OK )
        {
           vty_out( vty, " %%Vlan %s set mcastmode failed!\r\n", argv[0] );
           return CMD_WARNING;
        }
    }
    else 
    	return CMD_WARNING; 
#endif
    vty_out( vty, " Vlan %s set mcastmode %s OK!\r\n", argv[0], argv[1] );

    return CMD_SUCCESS;
}


QDEFUN ( no_onu_vlan_interface_func,
         no_onu_vlan_interface_cmd,
         "undo interface vlan <vlanname>",
         NO_STR
         "Config an interface\n"
         "Config a vlan interface\n"
         "Specify vlan's name(First character must be 'a'-'z' or 'A'-'Z', fallowed by 'a'-'z','A'-'Z','0'-'9'or'_', not more than 30)\n" ,
    &g_ulIFMQue )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    LONG lRet = VOS_OK;
    ULONG ulIfindex;
    UCHAR ucType;
    VLAN_ELEMENT_S stVLanIData;

#ifndef _DISTRIBUTE_PLATFORM_    
    lRet = ifm_check_valid_interface_name( argv[ 0 ] );
#else
    lRet = ifm_check_vlan_name( argv[0] );
#endif
    GOTO_LRET( lRet, error );

    if ( !VOS_StrCmp( IFM_VLAN_DEFAULT_VLAN_NAME, argv[ 0 ] ) )
    {
        vty_out( vty, "  %% Can not delete default VLAN.\r\n" );
        return CMD_WARNING;
    }
    ulIfindex = IFM_GetIfindexByName( argv[ 0 ] );

    if ( ulIfindex == 0 )
    {
        vty_out( vty, "  %% VLAN %s does not exist.\r\n", argv[ 0 ] );
        return CMD_WARNING;
    }
   
    vty->node = ONU_DEVICE_NODE;         

    lRet = VLAN_get_info( ulIfindex, IFM_CONFIG_VLAN_FORWARD_TYPE, ( VOID * ) & ucType, NULL );
    GOTO_LRET( lRet, error );
    if(ucType == VLAN_AMVLAN_BASED)
    {
       vty_out( vty, "  %% Can't Delete AMVLAN \r\n");
       return CMD_WARNING;
    }

    if ( !IFM_IS_VLAN_INDEX( ulIfindex ) )
    {
        vty_out( vty, "  %% Interface %s is not VLAN.\r\n", argv[ 0 ] );
        return CMD_WARNING;
    }

    /*
     *vlan是errp某个使能域的control/protect vlan，则不允许删除
     */
#ifdef _DISTRIBUTE_PLATFORM_  
    {
        ULONG ulRet = 0;
        ULONG ulVirtualTemplateif = 0;
#if (RPU_MODULE_PPPOE == RPU_YES)        
        /*如果vlan使能了pppoe，不允许no vlan，必须先pppoe disable*/
        lRet = IFM_get_info( ulIfindex, IFM_CONFIG_VLAN_OVERRIDE, &ulVirtualTemplateif, NULL );
        if(VOS_OK != lRet)
        {
            GOTO_LRET( lRet, error );
            return CMD_WARNING;
        }
        if(0 != ulVirtualTemplateif)
        {
            vty_out( vty, "  %% Can not change vlan information because PPPoE has been enabled.\r\n");
            return CMD_WARNING;
        }
#endif
        
    }
#endif

    lRet = VLAN_get_info( ulIfindex, IFM_CONFIG_VLAN_TYPE, ( VOID * ) & ucType, NULL );
    GOTO_LRET( lRet, error );

    if ( ucType == IFM_VLAN_DYNAMIC )
    {
        vty_out( vty, "  %% Can not change configuration of this VLAN.\r\n" );
        return CMD_WARNING;
    }

    VOS_MemZero( &stVLanIData, sizeof( VLAN_ELEMENT_S ) );
    stVLanIData.ucType = IFM_VLAN_STATIC;

    cl_vty_syn_index_later( vty , ( VOID* ) ulIfindex , VLAN_NODE, CONFIG_NODE,
                            "\r\n%% Warning : This VLAN has been deleted by other vty terminal.\r\n" );

    lRet = IFM_unregister( ulIfindex, &stVLanIData, NULL );
    GOTO_LRET( lRet, error );

    vty_out( vty, " Delete vlan %s OK!\r\n", argv[0] );

    return CMD_SUCCESS;
error:
    IFM_PError( lRet, vty );

#endif    
    return CMD_SUCCESS;

}


/*STP CLI */
QDEFUN( ConfigOnuBridgeEnalbe_Func,
        ConfigOnuCstBridgeEnalbe_CMD,
        "spanning-tree [enable|disable]" ,
        DescSTP
        "Start STP\n"
        "Stop STP\n",
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    /*STP_CLI_CfgBridgeRun( argc, argv, vty );*/
    enum node_type node;
    LONG OldMode = 0;
    LONG NewMode = STP_MODE_CST;
    LONG lRet = 0;
    USHORT VID = 0;
    ULONG ulValidMode = STP_MODE_ALL;
    Bool bEnable = True;
    STPM_T *br = NULL;

    vty->node = ONU_DEVICE_NODE;  
    
    /*获得启动、停止标记*/
    if ( !VOS_StrCmp( "enable", ( CHAR* ) argv[ 0 ] ) )
    {
        bEnable = True;
    }
    else
    {
        bEnable = False;
    }

    if ( STP_Lock( ) != STP_OK )
    {
        vty_out( vty, "  %% Failed to lock the bridge.Please try later.\r\n" );
        return CMD_WARNING;
    }

    if ( STP_GetCurrentSTPMode() != NewMode )
    {
        /*调用事件处理。注意子类型为CLI_CMD_CHANGE_MODE*/
        lRet = STP_Handler_CLI( CLI_CFG_RSTP + ( ulValidMode << 16 ), 0, 0, CLI_CMD_CHANGE_MODE, NewMode );
        if ( lRet != STP_OK )
        {
            /*cli_error(lRet, "Config stp mode error", vty);*/
            switch ( lRet )
            {
                case - ELOCKFAIL:
                	vty_out( vty, "  %% Failed to lock global stp domain.%s", VTY_NEWLINE );
                	break;
                case - EBRCREATEFAIL:
                	vty_out( vty, "  %% Failed to create bridge.Possible no memory.%s", VTY_NEWLINE );
                	break;
                default:
                	cli_error( vty, "  %% Failed to config stp mode" );
                	break;
                }
            }
        }

    STP_UnLock( );

    /*获得域ID*/
    /*
    if ( 2 == argc )
    {
        if ( VOS_CheckNumString( ( UCHAR* ) argv[ 1 ], VOS_StrLen( ( CHAR* ) argv[ 1 ] ) ) != VOS_OK )
        {
            cli_error( -EINVAL, ( const CHAR * ) argv[ 1 ], vty );
            return ;
        }
        VID = ( USHORT ) VOS_AtoL( ( CHAR* ) argv[ 1 ] );
#ifndef _DISTRIBUTE_PLATFORM_    
        if ( VID >= MAXVlanNum )
#else
        if ( FALSE==SysIsValidVID( VID ) )
#endif
        {
            cli_error( -EINVAL, argv[ 1 ], vty );
            return ;
        }
    }
    else
    {
        VID = g_Default_VLAN_ID;
    }*/

    VID = g_Default_VLAN_ID;

    br = STP_Hash_FindBridge( VID );
    if ( NULL == br )
    {
        lRet = -EBRNOFOUND;
        if ( STP_GetCurrentSTPMode() == STP_MODE_CST && VID != CST_VID )
        {
            vty_out( vty, "  %% STP domain id must be %d in cst mode.\r\n", CST_VID );
            return CMD_WARNING;
        }

        /*i_error(lRet, "Config bridge state error", vty);*/
        vty_out( vty, "  %% STP on vlan %d does not exist or the vlan does not exist.%s", VID, VTY_NEWLINE );
        return CMD_WARNING;
    }
    else
    {
        if ( br->admin_state == bEnable )
        {
            vty_out(vty,"  %% Bridge is %sd already.\r\n",argv[0]);
            return CMD_WARNING;
        }
    }

    /*调用事件处理*/
    lRet = STP_Handler_CLI( CLI_CFG_BRIDGE + ( ulValidMode << 16 ), VID, 0, 1 << CLI_BR_CFG_STATE, ( LONG ) bEnable );
#ifdef _DISTRIBUTE_PLATFORM_
    if(STP_OK == lRet)
    {
        lRet = STP_HwEnable( VID, (UCHAR)bEnable );
    }
#endif
    if ( lRet != STP_OK )
    {
        switch ( lRet )
        {
            case - EBRNOFOUND:
                if ( STP_GetCurrentSTPMode() == STP_MODE_CST && VID != CST_VID )
                {
                    vty_out( vty, "  %% STP domain id must be %d in cst mode.\r\n", CST_VID );
                    break ;
                }
                vty_out( vty, "  %% STP domain %d does not exist or the vlan does not exist.%s", VID, VTY_NEWLINE );
                break;
            case - ELOCKFAIL:
                vty_out( vty, "  %% Failed to lock stp domain %d.%s", VID, VTY_NEWLINE );
                break;
            default :
                vty_out( vty, "  %% Failed to config the bridge state.%s", VTY_NEWLINE );
                break;
        }
        return CMD_WARNING;
    }
    vty_out( vty, " Spanning tree %s OK!\r\n", argv[0] );    
#endif
    return CMD_SUCCESS;
}

QDEFUN( ConfigOnuBridgeParameterForward_Func,
        ConfigOnuCstBridgeParameterForward_CMD,
        "spanning-tree [forward-delay] <4-30>" ,
        DescSTP
        "Set forward delay\n"
        "Value ( 2*(Hello-time +1) <= Max-Age <= 2*(Forward-Delay - 1) )\n",
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_CfgBridge( argc, argv, vty );
#endif
    return CMD_SUCCESS;
}

/*hello-time*/
QDEFUN( ConfigOnuBridgeParameterHelloTime_Func,
        ConfigOnuCstBridgeParameterHelloTime_CMD,
        "spanning-tree [hello-time] <1-10>" ,
        DescSTP
        "Set hello time\n"
        "Value ( 2*(Hello-time +1) <= Max-Age <= 2*(Forward-Delay - 1) )\n",
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_CfgBridge( argc, argv, vty );
#endif
    return CMD_SUCCESS;
}


/*maximum-age*/
QDEFUN( ConfigOnuBridgeParameterMaxage_Func,
        ConfigOnuCstBridgeParameterMaxage_CMD,
        "spanning-tree [maximum-age] <6-40>" ,
        DescSTP
        "Set maximum age\n"
        "Value ( 2*(Hello-time +1) <= Max-Age <= 2*(Forward-Delay - 1) )\n",
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_CfgBridge( argc, argv, vty );
#endif
    return CMD_SUCCESS;
}


/*priority*/
QDEFUN( ConfigOnuBridgeParameterPriority_Func,
        ConfigOnuCstBridgeParameterPriority_CMD,
        "spanning-tree [priority] <0-61440>" ,
        DescSTP
        "Set bridge priority\n"
        "Value (multiple of 4096)\n",
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_CfgBridge( argc, argv, vty );
#endif
    return CMD_SUCCESS;
}


/*edge*/
QDEFUN( ConfigOnuPortParameterEdge_Func,
        ConfigOnuCstPortParameterEdge_CMD,
        "spanning-tree port <1-26> [edge] [yes|no]" ,
        DescSTP
        DescPort
        DescPortIndex
        "Edge port\n"
        "Set edge parameter true\n"
        "Set edge parameter false\n",
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
     if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_CfgPort( argc, argv, vty );
#endif
    return 0;
}

QDEFUN( ConfigOnuPortParameterNoneSTP_Func,
        ConfigOnuCstPortParameterNoneSTP_CMD,
        "spanning-tree port <1-26> [non-stp] [yes|no]" ,
        DescSTP
        DescPort
        DescPortIndex
        "Do not join STP calculation\n"
        "Do not join STP\n"
        "Join STP\n",
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
     if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_CfgPort( argc, argv, vty );
#endif
    return 0;
}

/*p2p*/
QDEFUN( ConfigOnuPortParameterP2P_Func,
        ConfigOnuCstPortParameterP2P_CMD,
        "spanning-tree port <1-26> [p2p] [yes|no|auto]" ,
        DescSTP
        DescPort
        DescPortIndex
        "Point to point port\n"
        "Set p2p parameter true\n"
        "Set p2p parameter false\n"
        "Let system detect automatically\n",
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
     if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_CfgPort( argc, argv, vty );
#endif
    return 0;
}

/*path-cost*/
QDEFUN( ConfigOnuPortParameterPathCost_Func,
        ConfigOnuCstPortParameterPathCost_CMD,
        "spanning-tree port <1-26> [path-cost] [auto|<1-200000000>]" ,
        DescSTP
        DescPort
        DescPortIndex
        "Set path cost\n"
        "Path cost auto assigned\n"
        "Path cost Value\n",
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
     if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_CfgPort( argc, argv, vty );
#endif
    return 0;
}


/*priority*/
QDEFUN( ConfigOnuPortParameterPriority_Func,
        ConfigOnuCstPortParameterPriority_CMD,
        "spanning-tree port <1-26> [priority] <0-240>" ,
        DescSTP
        DescPort
        DescPortIndex
        "Set priority\n"
        "Priority Value (multiple of 16)\n",
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
     if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_CfgPort( argc, argv, vty );
#endif
    return 0;
}


QDEFUN( ConfigOnuDefPortParametere_Func,
        ConfigOnuCstDefPortParameter_CMD,
        "undo spanning-tree port <1-26> [edge|non-stp|p2p|path-cost|priority]" ,
        DescDefaultConfig
        DescSTP
        DescPort
        DescPortIndex
        "Recover edge port parameter to default\n"
        "Recover non-stp parameter to default\n"
        "Recover point to point parameter to default\n"
        "Recover path cost parameter to default\n"
        "Recover priority parameter to default\n",
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    LONG lPort = 0;

    lPort = ( LONG )VOS_AtoL( argv[ 0 ] );
     if( lPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , lPort );
    	return CMD_WARNING ;
    }
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_CfgPortDef( argc, argv, vty );
#endif
    return 0;
}

/*桥默认参数*/
QDEFUN( ConfigOnuDefBridgeParameter_Func,
        ConfigOnuCstDefBridgeParameter_CMD,
        "undo spanning-tree [forward-delay|hello-time|maximum-age|priority]" ,
        DescDefaultConfig
        DescSTP
        "Recover forward-delay to default\n"
        "Recover hello-time to default\n"
        "Recover Maximum age to default\n"
        "Recover Priority to default\n",
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_CfgBridgeDef( argc, argv, vty );
#endif    
    return 0;
}

QDEFUN( ShowOnuBridge_Func,
        ShowOnuCstBridge_CMD,
        "show spanning-tree" ,
        DescShow
        DescSTP,
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_ShowBridge( argc, argv, vty );
#endif    
    return 0;
}


QDEFUN( ShowOnuBridgePort_Func,
        ShowOnuCstBridgePort_CMD,
        "show spanning-tree port <1-26>" ,
        DescShow
        DescSTP
        DescShowPort
        DescPortIndex,
        & STP_QueueID )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    vty->node = ONU_DEVICE_NODE; 
    STP_CLI_ShowBridgePort( argc, argv, vty );
#endif    
    return CMD_SUCCESS;
}

/**Config info相关命令**/
DEFUN ( config_save_onu_config_file,
        config_save_onu_config_file_cmd,
        "save configuration",
        "Save system information to onu flash\n"
        "Save current running configuration to onu flash\n" )

{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    vty->node = ONU_DEVICE_NODE; 
    VOS_StrCpy( vty->user_name, "OAM_CLI");
    VOS_StrCpy( vty->address, "oam_console" );
    cl_do_save_config( vty );
#endif
    return CMD_SUCCESS;
}

#if(PRODUCT_EPON3_ONU == RPU_YES) 
typedef unsigned short     INT16U;
typedef enum
{
    PASONU_OAM_AUTO,
    PASONU_OAM_DRAFT_2_0,
    PASONU_OAM_STANDARD
} PASONU_oam_version_t;

typedef  struct PASONU_version_s
{
    INT16U hw_major_ver;
    INT16U hw_minor_ver;
    INT16U fw_major_ver;
    INT16U fw_minor_ver;
    INT16U fw_build_num;
    INT16U fw_maintenance_ver;
    PASONU_oam_version_t OAM_version;
}  PASONU_version_t;

extern int PASONU_get_versions( PASONU_version_t* versions );
#endif

DEFUN ( show_onu_sys_version,
        show_onu_sys_version_cmd,
        "show version",
        SHOW_STR
        "Show version information\n"
        "Please input the card number you want to display\n" )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    PASONU_version_t  sFirmwareVersion;
    LONG lRet = 0;
    vty->node = ONU_DEVICE_NODE;
    vty_out( vty, "\r\n" );
    sys_show_version_banner_in_vty( vty );
    vty_out( vty, "\r\n" );
	/* modified by chenfj 2009-6-10 */
	/* vty_out( vty, "GT813 Appsoftware Version : V%dR0%dB0%d%d(Build on %s %s )\r\n",*/
    vty_out( vty, "Appsoftware Version : V%dR0%dB0%d%d(Build on %s %s )\r\n",
    			PRODUCT_SOFTWARE_MAJOR_VERSION_NO, 
                     PRODUCT_SOFTWARE_RELEASE_VERSION_NO,
                     PRODUCT_SOFTWARE_BUILD_VERSION_NO ,
                     PRODUCT_SOFTWARE_DEBUG_VERSION_NO,
    			__TIME__, __DATE__);    	
    sys_show_version_local_module( vty );
    vty_out( vty, "\r\n" );
    lRet = PASONU_get_versions( &sFirmwareVersion );
    if( lRet == 0 )
    {
    	vty_out( vty, "Firmware Version Information:\r\n");
    	vty_out( vty, "Major Version   : %d \r\n",  sFirmwareVersion.fw_major_ver );
    	vty_out( vty, "Minor Version   : %d \r\n", sFirmwareVersion.fw_minor_ver );
    	vty_out( vty, "Build No.       : %d \r\n", sFirmwareVersion.fw_build_num );
    	vty_out( vty, "Maintenance No. : %d \r\n", sFirmwareVersion.fw_maintenance_ver );
    }
#endif    
    return CMD_SUCCESS;
}

#if(PRODUCT_EPON3_ONU == RPU_YES)  
DEFUN ( show_onu813_sys_version,
        show_onu813_sys_version_cmd,
        "show version",
        SHOW_STR
        "Show version information\n"
        "Please input the card number you want to display\n" )
{
    PASONU_version_t  sFirmwareVersion;
    LONG lRet = 0;
    
    vty_out( vty, "\r\n" );
    sys_show_version_banner_in_vty( vty );
    vty_out( vty, "\r\n" );
    /* modified by chenfj 2009-6-10 */
    /*vty_out( vty, "GT813 Appsoftware Version : V%dR0%dB0%d%d(Build on %s %s )\r\n", */
    vty_out( vty, "Appsoftware Version : V%dR0%dB0%d%d(Build on %s %s )\r\n", 
    			PRODUCT_SOFTWARE_MAJOR_VERSION_NO, 
                     PRODUCT_SOFTWARE_RELEASE_VERSION_NO,
                     PRODUCT_SOFTWARE_BUILD_VERSION_NO ,
                     PRODUCT_SOFTWARE_DEBUG_VERSION_NO,
    			__TIME__, __DATE__);    	
    sys_show_version_local_module( vty );
    vty_out( vty, "\r\n" );
    lRet = PASONU_get_versions( &sFirmwareVersion );
    if( lRet == 0 )
    {
    	vty_out( vty, "Firmware Version Information:\r\n");
    	vty_out( vty, "Major Version   : %d \r\n",  sFirmwareVersion.fw_major_ver );
    	vty_out( vty, "Minor Version   : %d \r\n", sFirmwareVersion.fw_minor_ver );
    	vty_out( vty, "Build No.       : %d \r\n", sFirmwareVersion.fw_build_num );
    	vty_out( vty, "Maintenance No. : %d \r\n", sFirmwareVersion.fw_maintenance_ver );
    }
    return CMD_SUCCESS;
}
#endif

DEFUN ( erase_onu_config_file,
        erase_onu_config_file_cmd,
        "erase config-file",
        "Erase information from flash\n"
        "Erase all startup configuration information from flash\n"
        )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    /*vty_out( vty, " This operation will erase onu's config-file, you need to wait for about 1~2 minutes.\r\n " );*/
    vty->node = ONU_DEVICE_NODE;    
    cl_do_erase_startup_config( vty );
#endif
    return CMD_SUCCESS;
}

DEFUN ( show_onu_image_num,
        show_onu_image_num_cmd,
        "show image",
        SHOW_STR
        "Show image number information\n" )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    unsigned char active_image_index = 0;
    LONG lRet = 0;
    vty->node = ONU_DEVICE_NODE;
    
    lRet = PASONU_FLASH_image_get_active( &active_image_index );
    if( lRet != 0 )
    {
    	vty_out( vty, " %Can not get the image number information!\r\n");
    	return CMD_WARNING;
    }
    vty_out( vty, " Active image index is : %d \r\n", active_image_index);
    
#endif    
    return CMD_SUCCESS;
}

DEFUN ( active_onu_image,
        active_onu_image_cmd,
        "active image [0|1]",
        "Active\n"
        "Active image \n"
        "Please select the image you want to active\n")
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    unsigned char active_image_index = 0;
    LONG lRet = 0;
    vty->node = ONU_DEVICE_NODE;

    active_image_index = (unsigned char)VOS_AtoL( argv[ 0 ] );
   
    lRet = PASONU_FLASH_image_set_active( active_image_index );
    if( lRet != 0 )
    {
    	vty_out( vty, " %Can not do this operation!\r\n");
    	return CMD_WARNING;
    }
    if( active_image_index == 0 )
    	vty_out( vty, " Active image : 0 OK! You need to reset the onu device! \r\n");
    else if( active_image_index == 1 )
    	vty_out( vty, " Active image : 1 OK! You need to reset the onu device! \r\n");
   
#endif    
    return CMD_SUCCESS;
}

DEFUN( onu_cl_login_auth,
       onu_cl_login_auth_cmd,       
       "config login-authentication [enable|disable]",
       DescStringCommonConfig
       "Login and authentication by console\n"
       "User needs to input login username and password by console\n"
       "User doesn't need to input login username and password by console\n"
     )
{
#if(PRODUCT_EPON3_ONU != RPU_YES) 
    if ( ( vty->conn_type != _CL_VTY_CONSOLE_ ) &&
            ( g_SystemLoadConfComplete == _VOS_SYSTEM_RUNNING_ ) )  /*系统已经起来之后判断是否console登录*/
    {                                                                                                     /*因为telnet必须是用户名、密码登录方式*/
        vty_out( vty, "%% You didn't login by console, so you can not configure this.%s", VTY_NEWLINE );
        return 1;
    }
#endif    
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    vty->node = ONU_DEVICE_NODE;

    /*don't need to auth*/
    if ( 0 == VOS_StrnCmp( "disable", argv[ 0 ], strlen( argv[ 0 ] ) ) )
    {
        cl_login_auth_needed = 0;
    }

    if ( 0 == VOS_StrnCmp( "enable", argv[ 0 ], strlen( argv[ 0 ] ) ) )
    {
        cl_login_auth_needed = 1;
    }
    vty_out( vty, " config login-authentication set OK!\r\n");
#endif
    return CMD_SUCCESS;
}

/**Forward-entry**/
DEFUN( ShowONUMacByPort_Func,
        ShowONUMacByPort_CMD,
        "show forward-entry port <1-26> {[static|dynamic]}*1",
        DescStringCommonShow
        "Show onu forwarding entry information\n"
        "Show onu forwarding entry information by port\n"
        "Please input onu port number\n"
        "Show static forwarding entry information\n"
        "Show dynamic forwarding entry information\n" )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
	int unit = 0;
	int count = 0;	
	int p = 0;
	int port=0;
	CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };
	ULONG   ulIFIndex = 0;
	ULONG   ulState = 0;
	int index_min,index_max,i;
	cli_l2_addr_t l2addr={0};
	int rv;
	CHAR    *pScreenBuffer = NULL;
	LONG lRet = VOS_OK;
	LONG lPort = 0;

	port = ( LONG )VOS_AtoL( argv[ 0 ] );
	ulIFIndex = IFM_ETH_CREATE_INDEX( 1, lPort );
	vty->node = ONU_DEVICE_NODE;
	if ( ulIFIndex == 0 )
	{/******changed by suipl 2006-09-29 for slot/port与用户使用习惯一致****************/
		vty_out( vty, "  %% Can not find port %s.\r\n", argv[0] );
		return CMD_WARNING;	
	/****************/
	}
	/*port= IFM_ETH_GET_PORT( ulIFIndex );*/
	/*p=L2_2_PHY_PORT(port);*/
	/*p=port-1;*//*delete by lxl 2006/09/21端口转换移到bms层*/   

	if( argc == 1)
	{
		lRet = forward_entry_print_by_port( unit, &pScreenBuffer, -1, port, vty );
		if( lRet == VOS_ERROR )
			return CMD_WARNING;
		
		vty_big_out( vty, lRet, "%s", pScreenBuffer );      
		VOS_Free( pScreenBuffer );
		return CMD_SUCCESS;
	}

	if( VOS_StrCmp( argv[1], "static")==0)
	{
		lRet = forward_entry_print_by_port( unit, &pScreenBuffer, 1, port , vty);		
		if( lRet == VOS_ERROR )
			return CMD_WARNING;
		
		vty_big_out( vty, lRet, "%s", pScreenBuffer );      
		VOS_Free( pScreenBuffer );
		return CMD_SUCCESS;
	}
	else if( VOS_StrCmp( argv[1], "dynamic")==0)
	{
		lRet = forward_entry_print_by_port( unit, &pScreenBuffer, 0, port , vty );		
		if( lRet == VOS_ERROR )
			return CMD_WARNING;
		
		vty_big_out( vty, lRet, "%s", pScreenBuffer );      
		VOS_Free( pScreenBuffer );
		return CMD_SUCCESS;
	}
#endif	
	return CMD_SUCCESS;
}



DEFUN( ShowONUMac_byMac_Vlan_Func,
        ShowONUMac_byMac_vlan_CMD,
        "show forward-entry {[mac] <H.H.H>}*1 {[vlan] <vlanname>}*1",
        DescStringCommonShow
        "Show onu forwarding entry information\n"
        "Show onu forwarding entry information by MAC address\n"
        "Please input MAC address\n"
        "Show onu forwarding entry information by VLAN\n"
        "Please input VLAN name\n"
        )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
	  int unit = 0;
	  ULONG		ulBase = 0;
	  int count = 0;
	  LONG	lRet;
	  ULONG ulIfindex;
	  int arg_vlan;
	  CHAR Mac[ 6 ] = {0, 0, 0, 0, 0, 0};
	  int arg_static = -1;
	  int index_min,index_max,i;
	  cli_l2_entry_t l2addr={0};
	  int rv;
	  CHAR    *pScreenBuffer = NULL;

	  vty->node = ONU_DEVICE_NODE;
	  
	if( 0==argc )
	{
		lRet = forward_entry_print(unit, &pScreenBuffer, arg_static, vty );
		if( lRet == VOS_ERROR )
			return CMD_WARNING;
		
		vty_big_out( vty, lRet, "%s", pScreenBuffer );      
		VOS_Free( pScreenBuffer );
		return CMD_SUCCESS;
	} 
	if ( VOS_StrCmp( argv[ 0 ], "mac" ) ==0)
	{
		if ( GetMacAddr( ( CHAR* ) argv[ 1 ], Mac ) != VOS_OK )
		{		
			vty_out( vty, "  %% Invalid MAC address.\r\n" );
			return CMD_WARNING;
		}
						
		if(2==argc)
		{
			lRet = forward_entry_print_by_mac( unit, &pScreenBuffer, arg_static, Mac , vty );
			if( lRet == VOS_ERROR )
				return CMD_WARNING;
		
			vty_big_out( vty, lRet, "%s", pScreenBuffer );      
			VOS_Free( pScreenBuffer );
			return CMD_SUCCESS;
		}
		if( VOS_StrCmp( argv[2], "vlan")==0)
		{
			lRet = ifm_check_valid_interface_name( argv[ 3 ] );
			if(lRet != VOS_OK)
			{
				IFM_PError( lRet, vty );
			}
			ulIfindex = IFM_GetIfindexByName( argv[ 3 ] );
			if ( ulIfindex == 0 )
			{
				vty_out( vty, "  %% VLAN %s does not exist.\r\n", argv[ 3 ] );
				return CMD_WARNING;
			}
			arg_vlan = IFM_VLAN_GET_VLANID(ulIfindex);
			lRet = forward_entry_print_by_vlan_mac( unit, &pScreenBuffer, arg_static, Mac, arg_vlan, vty );
			if( lRet == VOS_ERROR )
				return CMD_WARNING;
		
			vty_big_out( vty, lRet, "%s", pScreenBuffer );      
			VOS_Free( pScreenBuffer );
			return CMD_SUCCESS;
		}
	}
	else if( VOS_StrCmp( argv[0], "vlan")==0)
	{
		lRet = ifm_check_valid_interface_name( argv[ 1 ] );
		if(lRet != VOS_OK)
		{ 
			IFM_PError( lRet, vty );
		}
		ulIfindex = IFM_GetIfindexByName( argv[ 1 ] );
		if ( ulIfindex == 0 )
		{
			vty_out( vty, "  %% VLAN %s does not exist.\r\n", argv[ 1 ] );
			return CMD_WARNING;
		}
		arg_vlan = IFM_VLAN_GET_VLANID(ulIfindex);
		lRet = forward_entry_print_by_vlan( unit, &pScreenBuffer, arg_static, arg_vlan, vty );		
		if( lRet == VOS_ERROR )
			return CMD_WARNING;
		
		vty_big_out( vty, lRet, "%s", pScreenBuffer );      
		VOS_Free( pScreenBuffer );
		return CMD_SUCCESS;
	} 
#endif	
	return CMD_SUCCESS;
}


DEFUN(ShowOnuMacCount_Func,
    ShowOnuMacCount_Cmd,
    "show forward-entry count {[static|dynamic]}*1 ", 
    DescStringCommonShow
    "Show forwarding entry information\n"
    "Show the count of forwarding entries\n"  
    "All static forwarding entries count\n"
    "All dynamic forwarding entries count\n"
    "All mac entry\n"
  )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
	int unit = 0;
	int count = 0;

	vty->node = ONU_DEVICE_NODE;    
	if(0==argc)
	{
		count = bms_l2_show_count(unit,-1);
		vty_out(vty,"  All mac entry count is:  %d\r\n",count);          
	}
	if( VOS_StrCmp( argv[0], "static")==0)
	{
		count = bms_l2_show_count(unit,1);
		vty_out(vty,"  Static mac entry count is:  %d\r\n",count);
	}
	else if( VOS_StrCmp( argv[0], "dynamic")==0)
	{	   
		count = bms_l2_show_count(unit,0);	
		vty_out(vty,"  Dynamic mac entry count is:  %d\r\n",count);
	}
#endif

	return CMD_SUCCESS;
}  



DEFUN( create_onu_forward_entry_func,
        create_onu_forward_entry_cmd,
        "forward-entry mac <H.H.H> vlan <vlanname> <1-26> ",
        "Create a permanent mac forwarding entry \n"
        "MAC address \n"
        "Please input MAC address \n"
        "VLAN associated with MAC address \n"
        "Please input VLAN name \n"
        "Please input port number associated with MAC address\n"
        "Qos queue id\n"
        "Please input the Qos queue id\n"
         )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
	ULONG lRet ;
	CHAR Mac[ 6 ] = {0, 0, 0, 0, 0, 0};
	CHAR stMac[15]={0};
	int		arg_static = 1, arg_trunk = 0, 	arg_vlan = 1, arg_tgid = 0;
	ULONG		ulBase = 0;
      cli_l2_entry_t l2entry = {0};
	int port=0;
	int unit = 0;
	CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };
	ULONG   ulIfindex = 0;
	ULONG   ulState = 0;
	ULONG   ulVlanIfIndex =0;
	ULONG   ulTrunkIfindex=0;
	UCHAR pName[IFM_API_NAMELEN]= {0};
	ULONG ulPort = 0, ulSlot = 0;
	ULONG ulTagged = 0;    
	USHORT usVid = 0;
	struct ForwardEntryItem *temp = NULL;
	struct ForwardEntryItem *tempTail = NULL;
	struct ForwardEntryItem *temp0 = NULL;
	struct ForwardEntryItem *temp1 = NULL;
	int count=0;
	CHAR bMac[ 6 ];
	/*ULONG ulPri = 0;*/

	vty->node = ONU_DEVICE_NODE;    

	memcpy( bMac, SYS_PRODUCT_BASEMAC, 6 );		
	if( ulBase==argc ) { return CMD_SUCCESS;  }
	/*mac address*/
	if ( GetMacAddr( ( CHAR* ) argv[0], Mac ) != VOS_OK )
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
	}

	if((bMac[0]==Mac[0]) && (bMac[1]==Mac[1]) &&
		(bMac[2]==Mac[2]) && (bMac[3]==Mac[3]) &&
		(bMac[4]==Mac[4]) && (bMac[5]==Mac[5]))
	{
		vty_out(vty," %% Can't set MAC address with System Device's MAC. \r\n" );       
		return VOS_ERROR;
	}
	
	VOS_StrCpy( stMac, argv[0]);
	/*vlanname*/
	ulBase = ulBase+1;
	lRet = ifm_check_valid_interface_name( argv[ ulBase ] );
       if(lRet != VOS_OK){
            IFM_PError( lRet, vty );
	}			
       ulVlanIfIndex = IFM_GetIfindexByName( argv[ ulBase ] );
       if ( ulVlanIfIndex == 0 )
       {
           vty_out( vty, "  %% VLAN %s does not exist.\r\n", argv[ ulBase ] );
           return CMD_WARNING;
       }
       /*不能为Super VLAN创建静态转发表项*/ 
       if(!IFM_IsSuperVlan_Api(ulVlanIfIndex))
       {
       	vty_out(vty,"  %% Super VLAN %s can not do this operation. \r\n", argv[ulBase]);
       	return CMD_WARNING;
       }

	arg_vlan = IFM_VLAN_GET_VLANID(ulVlanIfIndex);
	usVid=arg_vlan;
	/*port*/
	ulBase = ulBase+1;
#if 0	
	VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
	VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%s", ETH_SHORT_NAME, argv[ulBase] );
	ulIfindex = IFM_GetIfindexByNameExt( ifName, &ulState );
	if ( ulIfindex == 0 )
	{/******changed by suipl 2006-09-29 for slot/port与用户使用习惯一致****************/
		VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
		VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%s", "pon", /*argv[0]*/argv[ulBase] );/*changed by suipl 2006/10/09 for ID 2754 标题:
		添加错误的静态mac地址项时，系统给出的提示不对*/
		ulIfindex = IFM_GetIfindexByNameExt( ifName, &ulState );
		if ( ulIfindex == 0 )
		{
			vty_out( vty, "  %% Can not find interface %s.\r\n", ifName );
			return CMD_WARNING;
		}      
	/****************/
	}
#endif	
	ulSlot = 1;
	ulPort = ( LONG )VOS_AtoL( argv[2]);
	if( ulPort >24 )
	{
		vty_out( vty, " %% You can not config port %d \r\n" , ulPort );
		return CMD_WARNING ;
	}
	ulIfindex = IFM_ETH_CREATE_INDEX( 1, ulPort );
	
	/*判断接口是否Group到Trunk中去了.*/
	if(VOS_OK == IFM_PhyIsMerged2TrunkApi(ulIfindex))
	{
		if(VOS_OK != IFM_GetTrunkIfIndexByPhyIdxApi(ulIfindex,&ulTrunkIfindex))
		{
			ASSERT(0);
			return CMD_WARNING;
		}
		IFM_GetIfNameApi( ulTrunkIfindex, pName, IFM_API_NAMELEN );
		vty_out(vty,"  %% Port %d has been grouped in trunk %s.\r\n", ulPort, pName);
		return CMD_WARNING;
        }
    
	port=  ( LONG )VOS_AtoL( argv[2]);
	
	/*判断接口是否加入到Vlan中去了*/
	if ( VOS_OK != IFM_VlanPortRelationApi( ulIfindex, usVid, &ulTagged ) )
	{
		if ( SYS_IS_TRUNK_IF( ulIfindex ) )
		{
			vty_out( vty, "  %% Trunk %s does not belong to VLAN %s.\r\n", argv[ 2 ], argv[ 1 ] );
		}
		else
		{
			vty_out( vty, "  %% Port %s does not belong to VLAN %s.\r\n", argv[ 2 ], argv[ 1 ] );
		}
		return CMD_WARNING;
	}
	if ( 0 == ulTagged )
	{
		if ( SYS_IS_TRUNK_IF( ulIfindex ) )
		{
			vty_out( vty, "  %% Trunk %s does not belong to VLAN %s.\r\n", argv[ 2 ], argv[ 1 ] );
		}
		else
		{
			vty_out( vty, "  %% Port %s does not belong to VLAN %s.\r\n", argv[ 2 ], argv[ 1 ] );
		}

		return CMD_WARNING;
	}
	
	if(VOS_SemTake( semForwardEntry , 1000 )!=VOS_OK)
	{
		vty_out(vty , "  %% Can't do this operation at this time.%s" , VTY_NEWLINE);
		return CMD_WARNING;
	}

	/*if( argc == 4 )
	{
		ulPri = ( ULONG )VOS_AtoL( argv[ 3 ] );
	}*/

	/*先判断是否达到最大值*/
	count = bms_l2_show_count(unit,1);
	if( count >= FORWARD_MAX_NUM )
	{
		vty_out(vty, " %%The Max Static number is %d\r\n", FORWARD_MAX_NUM);
		VOS_Free( temp );
		VOS_SemGive( semForwardEntry );
		return CMD_WARNING;
	}	

	temp = ( struct ForwardEntryItem * ) VOS_Malloc( sizeof( struct ForwardEntryItem ), MODULE_RPU_CLI );
	if ( temp == NULL )
	{
		vty_out( vty, "  %% Not enough memory.%s", VTY_NEWLINE, VTY_NEWLINE );
		VOS_SemGive( semForwardEntry );
		return CMD_WARNING;
	}
	memset( ( CHAR * ) temp, 0, sizeof( struct ForwardEntryItem ) );
	/*sys_console_printf( "Sizeof( struct ForwardEntryItem ) is %d\r\n", sizeof( struct ForwardEntryItem ) );*//*52*/

	temp->next = 0;
	temp->usVlanID = usVid;
	temp->ulVlanIfIndex = ulVlanIfIndex;
	VOS_MemCpy(temp->aucMacAddr, Mac, 6);
	VOS_StrCpy(temp->stMac, argv[0]);
	temp->ulIfindex = ulIfindex;
	temp->usTrunkFlag = 0;
	temp->usTrunkID = 0;
	temp->ulTrunkIfIndex = 0;
	temp->usDropFlag = 0;
/*	temp->ulPri = ulPri;*/

	if ( g_stStaticForwardEntryList == NULL )
	{
		/*******先设置后插入*****************/
		VOS_MemCpy(l2entry.mac, Mac, 6);
		l2entry.tgid_port = port;
		l2entry.vid = arg_vlan;
		l2entry.tgid = arg_tgid;
		l2entry.trunk = arg_trunk;
		l2entry.l2_static = arg_static;
		/*l2entry.pri = ulPri;*/

		lRet = bms_l2_add(unit,&l2entry);
		if( lRet != VOS_OK )
		{
			VOS_Free( temp );
			VOS_SemGive( semForwardEntry );
			return CMD_WARNING;
		}
				
		g_stStaticForwardEntryList = temp;
		/*g_iForwardEntryCount++;*/
		VOS_SemGive( semForwardEntry );
		vty_out( vty, " forward-entry created OK!\r\n");
		
		return CMD_SUCCESS;
	}
	else
	{
		temp0 = g_stStaticForwardEntryList;
		while ( temp0 != 0 )
		{
			/*if ((VOS_StrCmp(temp0->stMac, stMac) == 0) && (temp0->usVlanID == arg_vlan) &&( temp0->ulIfindex == ulIfindex))*/
			/*Changed by suipl 2007/08/16*/
			if ((VOS_MemCmp(temp0->aucMacAddr, Mac, 6) == 0) && (temp0->usVlanID == arg_vlan) &&( temp0->ulIfindex == ulIfindex)/*&&( temp0->ulPri == ulPri )*/)
			{				
				vty_out( vty, "  %%Forward-entry mac %2.2X%2.2X.%2.2X%2.2X.%2.2X%2.2X vlan %s %s had been added!\r\n",
					Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5], argv[1], argv[2]);
				VOS_Free( temp );
				VOS_SemGive( semForwardEntry );
				return CMD_WARNING;
			}/*delete the item*/
			temp0=temp0->next;
		}

		temp1 = g_stStaticForwardEntryList;
		while ( temp1 != 0 )
		{
			/*if ((VOS_StrCmp(temp1->stMac, stMac) == 0)&& (temp1->usVlanID == arg_vlan ))*//*Changed by suipl 2007/08/16*/
			if ((VOS_MemCmp(temp1->aucMacAddr, Mac, 6) == 0)&& (temp1->usVlanID == arg_vlan ))
			{				
				temp1->ulIfindex = ulIfindex;
				temp1->usTrunkFlag=0;
				temp1->usTrunkID = 0;
				temp1->ulTrunkIfIndex = 0;
				temp1->usDropFlag =0;
			/*	temp1->ulPri = ulPri;*/
				/*******先设置后插入*****************/
				VOS_MemCpy(l2entry.mac, Mac, 6);
				l2entry.tgid_port = port;
				l2entry.vid = arg_vlan;
				l2entry.tgid = arg_tgid;
				l2entry.trunk = arg_trunk;
				l2entry.l2_static = arg_static;
				/*l2entry.pri = ulPri;*/

				lRet = bms_l2_add(unit,&l2entry);
				if( lRet != VOS_OK )
				{
					vty_out( vty, "  %%Forward-entry mac %s vlan %s %s be added FAILED!\r\n", argv[0], argv[1], argv[2]);
					VOS_Free( temp );
					VOS_SemGive( semForwardEntry );
					return CMD_WARNING;
				}
				VOS_Free( temp );
				VOS_SemGive( semForwardEntry );	
				vty_out( vty, " forward-entry created OK!\r\n");
				return CMD_SUCCESS;
				/************************************************/
			}/*delete the item*/
			temp1=temp1->next;
		}
		
		/*******先设置后插入*****************/	
		VOS_MemCpy(l2entry.mac, Mac, 6);
		l2entry.tgid_port = port;
		l2entry.vid = arg_vlan;
		l2entry.tgid = arg_tgid;
		l2entry.trunk = arg_trunk;
		l2entry.l2_static = arg_static;
		/*l2entry.pri = ulPri;*/

		lRet = bms_l2_add(unit,&l2entry);
		if( lRet != VOS_OK )
		{
			vty_out( vty, "  %%Forward-entry mac %s vlan %s %s be added FAILED!\r\n", argv[0], argv[1], argv[2]);
			VOS_Free( temp );
			VOS_SemGive( semForwardEntry );
			return CMD_WARNING;
		}		
		/**********************************/

		tempTail = g_stStaticForwardEntryList;
		while ( tempTail->next != NULL )
			tempTail = tempTail->next;
		tempTail->next = temp;
		
		VOS_SemGive( semForwardEntry );
		vty_out( vty, " forward-entry created OK!\r\n");
		/*g_iForwardEntryCount++;*/
		return CMD_SUCCESS;
	}
#endif

	return CMD_SUCCESS;
}         


DEFUN( delete_onu_forward_entry_func,
        delete_onu_forward_entry_func_cmd,
        "delete forward-entry mac <H.H.H> vlan <vlanname>",
        DescStringCommonDelete
        "Delete a permanent mac forwarding entry\n"
        "MAC address \n"
        "Please input MAC address\n"
        "VLAN associated with MAC address\n"
        "Please input VLAN name\n"
       )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
	ULONG lRet ;
	CHAR Mac[ 6 ] = {0, 0, 0, 0, 0, 0};
	CHAR stMac[15]={0};
	int		arg_vlan = 1;
	ULONG		ulBase = 0;
	int unit = 0;
	ULONG ulVlanIfindex=0;
	struct ForwardEntryItem *temp0=NULL, *temp1=NULL;
	ULONG ulFindNum=0;
	
	vty->node = ONU_DEVICE_NODE;    
	if ( GetMacAddr( ( CHAR* ) argv[ 0], Mac ) != VOS_OK )
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
	}
	VOS_StrCpy( stMac, argv[0]);
	
	/*vlanname*/
	ulBase = ulBase+1;
	lRet = ifm_check_valid_interface_name( argv[ ulBase ] );
       if(lRet != VOS_OK){
            IFM_PError( lRet, vty );
	}			
       ulVlanIfindex = IFM_GetIfindexByName( argv[ ulBase ] );
       if ( ulVlanIfindex == 0 )
       {
           vty_out( vty, "  %% VLAN %s does not exist.\r\n", argv[ ulBase ] );
           return CMD_WARNING;
       }
	arg_vlan = IFM_VLAN_GET_VLANID(ulVlanIfindex);

	temp0 = g_stStaticForwardEntryList;
	/*temp1 = g_stStaticForwardEntryList;*/

	if(VOS_SemTake( semForwardEntry , 1000 )!=VOS_OK)
	{
		vty_out(vty , "  %% Can't do this operation at this time.%s" , VTY_NEWLINE);
		return CMD_WARNING;
	}

	if( temp0 == 0)
	{
		vty_out( vty, "  %% Can not find forward_entry mac %s vlan %s \r\n", stMac, argv[1]);
		VOS_SemGive( semForwardEntry );
		return CMD_WARNING;
	}
	
	while ( temp0 != 0 )
	{
		/*if ((VOS_StrCmp(temp0->stMac, stMac) == 0)&& (temp0->ulVlanIfIndex == ulVlanIfindex))*//*Changed by suipl 2007/08/16*/
		if ((VOS_MemCmp(temp0->aucMacAddr, Mac, 6) == 0)&& (temp0->ulVlanIfIndex == ulVlanIfindex))
		{	
			if( temp0 == g_stStaticForwardEntryList )
				g_stStaticForwardEntryList = temp0->next;
			else
			{
				temp1->next = temp0->next;
				VOS_Free( temp0);
				temp0= temp1->next;
			/*VOS_Free( temp0);*/
			}
			/*g_iForwardEntryCount--;*/
			ulFindNum++;
		}/*delete the item*/
		else
		{
			temp1=temp0;
			temp0=temp0->next;
		}
	}
	VOS_SemGive( semForwardEntry );	
	if( ulFindNum != 0)
	{
		bms_l2_delete(unit,arg_vlan,Mac,0);
		vty_out( vty, " Delete forward-entry mac OK!\r\n");
	}
	else 
		vty_out( vty, "  %% Can not find forward_entry %s vlan %s \r\n", stMac, argv[1]);

	ulFindNum=0;
#endif
	
	return CMD_SUCCESS;
}       



DEFUN (
    onu_mac_set_agingtime,
    onu_mac_set_agingtime_cmd,
    "forward-entry agingtime [0|<10-600>]",
    "Set the forward-entry information\n"
    "The forward-entry aging time\n"
    "0 indicate the mac will never be aged\n"
    "The range is 10 through 600 seconds. The default value is 300 seconds.\n"
 )
{	
#if(PRODUCT_EPON3_ONU == RPU_YES)  
      int agingtime;

	vty->node = ONU_DEVICE_NODE;    
      agingtime=( int ) VOS_AtoL( argv[0] );
      g_iMacAgingTime=agingtime;

	bms_AgeTimer_set(agingtime);
	vty_out( vty, " forward-entry agingtime set OK!\r\n");
#endif

	return CMD_SUCCESS;
}

DEFUN (
    onu_mac_no_agingtime,
    onu_mac_no_agingtime_cmd,
    "undo forward-entry agingtime",
    NO_STR
    "The forward-entry information\n"
    "The forward-entry aging time\n"
 )/*changed by suipl 2006-10-03 for no->delete*/
{	
#if(PRODUCT_EPON3_ONU == RPU_YES)  
	vty->node = ONU_DEVICE_NODE;    
      bms_AgeTimer_set(FORWARD_DEFAULT_AGING_TIME);/*set to default value*/
      g_iMacAgingTime=FORWARD_DEFAULT_AGING_TIME;
      vty_out( vty, " Undo forward-entry agingtime OK!\r\n");
#endif      
	return CMD_SUCCESS;
}

DEFUN (
    onu_mac_show_agingtime,
    onu_mac_show_agingtime_cmd,
    "show forward-entry agingtime",
    SHOW_STR
    "The forward-entry information\n"
    "The forward-entry aging time\n"
)
{	
#if(PRODUCT_EPON3_ONU == RPU_YES)  
      int age_seconds = 0;

	vty->node = ONU_DEVICE_NODE;    

      age_seconds = g_iMacAgingTime;     
      if (0==age_seconds)
      {
      		vty_out(vty,"  Age timer disable\r\n");
    	}
    	else
    		vty_out(vty, "  The mac aging time is %d seconds\r\n", age_seconds);
#endif    	
	return CMD_SUCCESS;
}

#if 0	/* removed by xieshl 20100806 */
/*#if(PRODUCT_EPON3_ONU == RPU_NO)  */
DEFUN (show_onu_infor,
       show_onu_infor_cmd,
       "show info",
       SHOW_STR
       "Show onu system's info\n" )
{ 
	LONG lRet;
	CHAR pBuff[512]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	/*int i;*/

	if( vty->prev_node != ONU_GT813_NODE )
	{
		/* modified by chenfj 2009-6-10 */
		vty_out( vty, " %%This command can be excuted only when you come into this node from %s onu node!\r\n", GetGT813TypeString());
		return CMD_WARNING;
	}
	
	ulIfIndex =(ULONG) vty->index;
	
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;
	
	ponID = GetPonPortIdxBySlot(ulSlot, ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}

	lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
	if ( lRet !=1 )
       {
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
	   return CMD_WARNING;
       }
	
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%show info FAILED!\r\n");	
		return CMD_WARNING;
	}

    return CMD_SUCCESS;
}
#endif
/*DEFUN (
    test_onu_port_list,
    test_onu_port_list_cmd,
    "test onu_port_list <onuid_list>",
    "Test \n"
    "Test onu_port_list\n"
    "Please input the port_list\n" )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
	ULONG ulOnuId=0;
	
	if ( !Onu_Check_Port_List( argv[ 0 ] ) )
	{
		vty_out( vty, "%% Invalid onuid list <%s>. \r\n", argv[ 0 ] );
		return CMD_SUCCESS;
	}

	vty_out( vty, "Input onuid is :" );

	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0 ], ulOnuId )
	{
		vty_out( vty, "%d  ", ulOnuId );
	}
	END_PARSE_PORT_LIST_TO_PORT();

	vty_out( vty, "\r\n");
#endif
	return CMD_SUCCESS;
}
*/

/*Igmp Snooping*/
/*Added by suipl 2007/08/10*/
QDEFUN( onu_IgmpSnoopEnable_Func,
        onu_Igmp_snoop_enable_Cmd,
        "igmp-snooping [enable|disable]",
        "IGMP Snooping function\n"
        "Enable IGMP Snooping\n"
        "Disable IGMP Snooping\n",
        &g_ulIgmp_Snoop_MSQ_id
)
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    vty->node = ONU_DEVICE_NODE; 
    if ( 0 == VOS_StrCmp( argv[ 0 ], "enable" ) )
    {
        if ( Igmp_Snoop_IsEnable() )
        {
            vty_out( vty, " %% igmp-snooping is already started or is stopping.\r\n" );
            return CMD_WARNING;
        }

        if ( VOS_OK != Igmp_Snoop_init( ) )
        {
            vty_out( vty, " %% Failed to start igmp-snooping. %s \r\n", VTY_NEWLINE );
            return VOS_ERROR;
        }
	vty_out( vty, " Igmp snooping Enable!\r\n");
    }
    else if ( 0 == VOS_StrCmp( argv[ 0 ], "disable" ) )
    {
        if ( !Igmp_Snoop_IsEnable() )
        {
            vty_out( vty, " %% igmp-snooping is not started or starting.\r\n" );
            return CMD_WARNING;
        }
        /* Stop igmp snoop task */
        if ( VOS_OK != Igmp_Snoop_stop() )
        {
            vty_out( vty, " %% Failed to stop igmp-snooping. %s \r\n", VTY_NEWLINE );
            return VOS_ERROR;
        }
        vty_out( vty, " Igmp snooping Disable!\r\n" ); 
    }
    else
    {
        vty_out( vty, " %% Invalid command.\r\n" );
    }
#endif
    return CMD_SUCCESS;
}

QDEFUN( onu_IgmpSnoopAuthEnable_Func,
        onu_Igmp_snoop_Authenable_Cmd,
        "igmp-snooping auth [enable|disable]",
        "IGMP snooping authentication config\n"
        "IGMP Snooping auth function\n"
        "Enable IGMP Snooping auth\n"
        "Disable IGMP Snooping auth\n",
        &g_ulIgmp_Snoop_MSQ_id
)
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    vty->node = ONU_DEVICE_NODE; 
    if ( 0 == VOS_StrCmp( argv[ 0 ], "enable" ) )
    {
        if ( g_ulIgmp_Auth_Enabled )
        {
            vty_out( vty, " %% igmp-snooping auth is already enabled.\r\n" );
            return CMD_WARNING;
        }

        g_ulIgmp_Auth_Enabled = 1;
        vty_out( vty, " Igmp snooping auth Enable!\r\n");
    }
    else if ( 0 == VOS_StrCmp( argv[ 0 ], "disable" ) )
    {
        if ( !g_ulIgmp_Auth_Enabled )
        {
            vty_out( vty, " %% igmp-snooping auth is already disable.\r\n" );
            return CMD_WARNING;
        }
        g_ulIgmp_Auth_Enabled = 0;
        vty_out( vty, " Igmp snooping auth Disable!\r\n");
    }
    else
    {
        vty_out( vty, " %% Invalid command.\r\n" );
    }
#endif
    return CMD_SUCCESS;
}


QDEFUN( onu_IgmpSnoopSetSGroup_Func,
        onu_Igmp_snoop_SetSGroup_Cmd,
        "igmp-snooping [add|del] group  <vlanname>  <A.B.C.D>",
        "IGMP Snooping command\n"
        "Add static Multicast group\n"
        "Del static Multicast Group\n"
        "Add or Del static Multicast group\n"
        "A exist Vlan name\n"
        "Multicast Ip address\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    ULONG ulSlot, ulPort;
    ULONG ipaddr;
    SYS_IF_INDEX_U unIfIndex;
    USHORT usVid;
    MC_Group_t * pMcGroup = NULL;
    MC_Group_t * pNexMcGroup = NULL;
    MC_Group_Vlan_t *pPrevGroupVlan = NULL;

    vty->node = ONU_DEVICE_NODE; 

    if(VOS_StrCmp( argv[0], "add") == 0)
    {
        if(g_ulIgmp_GroupCount >= IGMP_SNOOP_GROUP_MAX)
        {
           vty_out( vty, "\r\n %%Igmp snooping module NO enough table items.\r\n" );
           return VOS_ERROR;
        }
    }

    unIfIndex = IFM_GetIfindexByNameApi(argv[1]);
    /* Get vlan id from vlan name */
    if ( !SYS_IS_VLAN_IF( unIfIndex ) ) 
    {
        vty_out( vty, " %% Vlan %s does not exist. \r\n", argv[ 1] );
        return VOS_ERROR;
    }
    else
    {                                   /*不能在AMVLAN中创建和删除静态组 */
        struct net_device   *pNetDev      = NULL;
        VLAN_P_DATA_S       *pstVLanPData = NULL;
		
	if(IFM_find_netdev( unIfIndex, &pNetDev, NULL) != VOS_OK) 
		return CMD_WARNING;

        pstVLanPData = ( VLAN_P_DATA_S * )pNetDev->pPrivateData;
		
        if(VLAN_IS_AMVLAN(pstVLanPData))
        {
           vty_out( vty, " %% Can't add or delete Group in AMVALN!\r\n");
           return VOS_ERROR;;
        }
    }

    usVid = SYS_IF_VLAN_ID( unIfIndex );

    ipaddr = get_long_from_ipdotstring( argv[ 2 ] );
	ipaddr = VOS_NTOHL(ipaddr);    /* add by suxq 2007-06-19 */
   /*group address check ,add by sxj 060627*/
    if ( VOS_OK != Igmp_Snoop_Addr_Check( ipaddr )  )
    {
        vty_out( vty, " %% Group address %s invalid . \r\n", argv[ 2] );
#if 0
    	VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Group address invalid. 0x%-8x ", ulGroup );
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: group address invalid. 0x%-8x \r\n", ulGroup ) );
#endif
    	return VOS_ERROR;
    }
    if ( VOS_OK !=Igmp_SearchVlanGroup( usVid, ipaddr, &pMcGroup, &pPrevGroupVlan ))
    {
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: search failed\r\n" ) );

        VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Search failed." );

        return VOS_ERROR;
    }

    
    if(VOS_StrCmp( argv[0], "add") == 0)
    {
        if ( NULL == pMcGroup )
        {
            if ( VOS_OK != Igmp_Creat_Group_Node( usVid, ipaddr, NULL, &pMcGroup, &pPrevGroupVlan ) )  
      	    {
                IGMP_SNOOP_TEMPGROUP_DEBUG( ( "Igmp_Recv_Report: create  group error. \r\n" ) );
                VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Create group error. %x." );

                return VOS_ERROR;
            }
            pMcGroup->uStatic_type = IGMP_SNOOP_STATIC_GROUP ;
        }
	 else
	 {
	        if (pMcGroup->uStatic_type == IGMP_SNOOP_STATIC_GROUP)
	        {
                    vty_out( vty, " %% This static Group is already exist. \r\n" );
	        }
		 else
		 {
                    pMcGroup->uStatic_type = IGMP_SNOOP_STATIC_GROUP ;
                    vty_out( vty, " %% This dynamic Group is already exist, and be changed to static. \r\n" );
		 }
	 }
    }
    else if (VOS_StrCmp( argv[0], "del") ==0)
    {
        if (( NULL != pMcGroup ) && (pMcGroup->uStatic_type == IGMP_SNOOP_STATIC_GROUP))
        {
            if ( VOS_OK != Igmp_Snoop_DelGroup( pPrevGroupVlan,pMcGroup, &pNexMcGroup, IGMP_SNOOP_STATIC_GROUP ))
     	    {
                 IGMP_SNOOP_TEMPGROUP_DEBUG( ( "Igmp_Recv_Report: delete  group error. \r\n" ) );
                 VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: delete group error. %x." );

                 return VOS_ERROR;
            }
        }
	 else
	 {
            vty_out( vty, " %% This static Group is not exist. \r\n" );
	 }
    }
    else return CMD_WARNING;

    vty_out( vty, " Igmp snooping Group operation OK!\r\n");
#endif
	return CMD_SUCCESS;
}

QDEFUN( onu_IgmpSnoopSetSMem_Func,
        onu_Igmp_snoop_SetSMem_Cmd,
        "igmp-snooping [add|del] member  <vlanname>  <A.B.C.D>  <port_list>",
        "IGMP Snooping command\n"
        "Add  static muticast group member\n"
        "Del   static muticast group  member\n"
        "Add or Del static muticast group member\n"
        "Vlan name\n"
        "Muticast IP address\n"
        "Specify port number list you want to change(e.g.: 1-3, 5,6-9)\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    ULONG ulSlot, ulPort;
    ULONG ipaddr;
    SYS_IF_INDEX_U unIfIndex;
    ULONG iAction = 0;
    ULONG               ulPortIfindex;
    USHORT usVid;
    MC_Group_t * pMcGroup = NULL;
    MC_Group_Vlan_t *pPrevGroupVlan = NULL;
    struct MC_Port_State *pstPort = NULL;
    Igmp_Snoop_pkt_t stPkt;
    LONG lRet = VOS_OK;
    ULONG ulUserSlot = 0;
    ULONG ulUserPort = 0;

    vty->node = ONU_DEVICE_NODE; 
    if (VOS_StrCmp(argv[ 0 ] ,"add") == 0)  iAction =1;
    else if (VOS_StrCmp( argv[0], "del") ==0) iAction =2;
    else return CMD_WARNING;
 
    unIfIndex = IFM_GetIfindexByNameApi(argv[1]);
    /* Get vlan id from vlan name */
    if ( !SYS_IS_VLAN_IF( unIfIndex ) ) 
    {
        vty_out( vty, " %% Vlan %s does not exist. \r\n", argv[ 1] );
        return VOS_ERROR;
    }

    usVid = SYS_IF_VLAN_ID( unIfIndex );

    ipaddr = get_long_from_ipdotstring( argv[ 2 ] );
    ipaddr = VOS_NTOHL(ipaddr);    /* add by suxq 2007-06-19 */

    if ( VOS_OK !=Igmp_SearchVlanGroup( usVid, ipaddr, &pMcGroup, &pPrevGroupVlan ))
    {
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp add static member: search failed\r\n" ) );
        VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to add IGMP static member: Search failed." );

        return VOS_ERROR;
    }

    if (( NULL == pMcGroup )||(pMcGroup->uStatic_type != IGMP_SNOOP_STATIC_GROUP))
    {
        vty_out( vty, "\r\n %%The static group doesn't exist or group isn't static.\r\n" );
        return VOS_ERROR;
    }

    if ( ONU_CheckPortListValid( argv[3] ) != 1 )
    {
        vty_out( vty, "  %% Invalid port list <%s>.\r\n", argv[ 3 ] );
        return CMD_WARNING;
    }

    BEGIN_PARSE_PORT_LIST_TO_PORT( argv[3], ulPort )    
    {
        UCHAR   aucString[128];
	    LONG    lRelation;
        VOS_MemZero( aucString, 128 );		

        ulUserSlot = 1;
        ulUserPort = ulPort;

        ulPortIfindex = IFM_ETH_CREATE_INDEX( 1, ulPort );

	    if( Igmp_Snoop_IF_RouterPort(usVid,ulPortIfindex) == 1)
        {
            vty_out( vty, " %% this port %d is router port.\r\n", ulUserPort);
            continue;
        }

        if ( iAction == 1 ) /* 添加 */
        {
            lRelation = vlan_if_relation(unIfIndex,ulPortIfindex);
		    if((lRelation == IFM_VLAN_PORT_UNTAGGED ) || (lRelation == IFM_VLAN_PORT_TAGGED))
		    {
		         lRet = Igmp_Snoop_SearchPortList( &( pMcGroup->pstPortStateList ), ulPortIfindex, IGMP_PORT_ADD, &pstPort,IGMP_SNOOP_STATIC_MEM );
                 if ( VOS_OK != lRet )
                 {
                      IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: search port failed. 0x%x\r\n", pstPort ) );

                      VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Search port failed 0x%x.\r\n", pstPort );

                      RETURN_PARSE_PORT_LIST_TO_PORT( VOS_ERROR );
                 }

                 stPkt.lVid = usVid;
                 stPkt.ulGroupAddr = ipaddr;
                 stPkt.ulIfIndex = ulPortIfindex;
                 stPkt.ulIndex = pMcGroup->ulIndex;
                 stPkt.ulIsNewEntry = 0;
                 stPkt.usType = IGMP_ADDR_ADD;

                 if ( VOS_OK != ( lRet = Igmp_Snoop_Mod_Addr( pMcGroup, &stPkt, IGMP_ADDR_UPDATE) ) )
                 {
                     IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: failed in add address.\r\n" ) );
 
                     VOS_SysLog( LOG_TYPE_IGMP, LOG_ERR, "Fail to report IGMP receive: Failed in add address." );

                     RETURN_PARSE_PORT_LIST_TO_PORT( VOS_ERROR );
                 }
	        }
        }
        else /* 删除 */
        {
		    lRet = Igmp_Snoop_SearchPortList( &( pMcGroup->pstPortStateList ), ulPortIfindex, IGMP_PORT_DEL, &pstPort,IGMP_SNOOP_STATIC_MEM );
            if ( VOS_OK != lRet )
            {
                IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: search port failed. 0x%x\r\n", pstPort ) );

                VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Search port failed 0x%x.\r\n", pstPort );

                RETURN_PARSE_PORT_LIST_TO_PORT( VOS_ERROR );
            }

            stPkt.lVid = usVid;
            stPkt.ulGroupAddr = ipaddr;
            stPkt.ulIfIndex = ulPortIfindex;
            stPkt.ulIndex = pMcGroup->ulIndex;
            stPkt.ulIsNewEntry = 0;
            stPkt.usType = IGMP_ADDR_DEL;

            if ( VOS_OK != ( lRet = Igmp_Snoop_Mod_Addr( pMcGroup, &stPkt, IGMP_ADDR_UPDATE) ) )
            {
                IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: failed in add address.\r\n" ) );

                VOS_SysLog( LOG_TYPE_IGMP, LOG_ERR, "Fail to report IGMP receive: Failed in add address." );

                RETURN_PARSE_PORT_LIST_TO_PORT( VOS_ERROR );
            }
        }
    }
    END_PARSE_PORT_LIST_TO_PORT();
    vty_out( vty, " Igmp snooping set member OK!\r\n");
#endif

    return CMD_SUCCESS;
}

QDEFUN( onu_IgmpSnoopShow_Func,
        onu_Igmp_snoop_Show_Cmd,
        "show igmp-snooping",
        DescStringCommonShow
        "IGMP Snooping table\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    vty->node = ONU_DEVICE_NODE; 
    return IgmpSnoop_Show( vty );
#endif
	return CMD_SUCCESS;
}


QDEFUN( onu_IgmpSnoopDelSGroup_Func,
        onu_Igmp_snoop_dels_Cmd,
        "igmp-snooping deldyngroup <vlan> [<A.B.C.D>|all]",
        "IGMP Snooping command\n"
        "Delete dynamic group for test\n"
        "Vlan name\n"
        "Group address  Ex. 224.1.1.5\n"
        "All group\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    LONG ipaddr;
    MC_Group_Vlan_t *pVlan = NULL;
    MC_Group_Vlan_t *pNextVlan = NULL;
    MC_Group_t *pMcGroup = NULL;
    MC_Group_t *pNext = NULL;
    LONG i = 1;
    ULONG ulRet;
    SYS_IF_INDEX_U unIfIndex;
    USHORT usVid;

    vty->node = ONU_DEVICE_NODE; 
    if ( !Igmp_Snoop_IsEnable() )
    {

        vty_out( vty, "\r\n %% IGMP snooping module does not enable.\r\n" );

        return VOS_ERROR;
    }

    unIfIndex = IFM_GetIfindexByNameApi(argv[0]);
    /* Get vlan id from vlan name */
    if ( !SYS_IS_VLAN_IF( unIfIndex ) ) 
    {
        vty_out( vty, " %% Vlan %s does not exist. \r\n", argv[ 0 ] );
        return VOS_ERROR;
    }

    usVid = SYS_IF_VLAN_ID( unIfIndex );

    if ( 0 == VOS_StrCmp( argv[ 1 ], "all" ) )
    {
#if 0  /*add by sxj 06-6-6*/
	Igmp_Snoop_DelVlan( ( LONG ) usVid ); 
#endif 

	    Igmp_SearchVlanGroup( usVid, ipaddr, &pMcGroup, &pVlan );
	    pMcGroup = pVlan->pstGroup ;
	    while((pVlan != NULL) && (NULL != pMcGroup))
	    {
	        if(pMcGroup->uStatic_type==IGMP_SNOOP_DYNAMIC_GROUP)
	     	{
      	        Igmp_Snoop_DelGroup( pVlan, pMcGroup, &pNext ,IGMP_SNOOP_DYNAMIC_DELETE);
 		        pMcGroup = pNext ;
		    }
	        else
	        {
	     	    vty_out( vty, " %% Group %x is static,can't be deleted.\r\n",pMcGroup->ulMc_IP);
       	        pMcGroup = pMcGroup->NextGroup ;
	        }
	    }
        if((pVlan != NULL) && ( pVlan->pstGroup == NULL ))
        {
            Igmp_Snoop_DelVlan( pVlan->lVid ,&pNextVlan);
        }
    }
    else
    {
        ipaddr = get_long_from_ipdotstring( argv[ 1 ] );
    	ipaddr = VOS_NTOHL(ipaddr);    /* add by suxq 2007-06-19 */
		
        IGMP_SNOOP_GLOBAL_DEBUG( ( "Group: 0x%.8x\r\n", ipaddr ) );

        Igmp_SearchVlanGroup( usVid, ipaddr, &pMcGroup, &pVlan );
        if ( NULL == pMcGroup )
        {
            vty_out( vty, " %% Group %s does not exist in Vlan %s.\r\n", argv[ 1 ], argv[ 0 ] );
 		    return VOS_ERROR;
        }
	    if(pMcGroup->uStatic_type==IGMP_SNOOP_DYNAMIC_GROUP)
      	        Igmp_Snoop_DelGroup( pVlan, pMcGroup, &pNext ,IGMP_SNOOP_DYNAMIC_DELETE);
	    else
	    {
            vty_out( vty, " %% Group %s is static,can't be deleted.\r\n",argv[ 1 ]);
 		    return VOS_ERROR;
	    }
        if((pVlan != NULL) && ( pVlan->pstGroup == NULL ))
        {
            Igmp_Snoop_DelVlan( pVlan->lVid, &pNextVlan );
        }
    }
    vty_out( vty, " Igmp snooping delete dyngroup OK!\r\n" );
#endif
    return CMD_SUCCESS;

}


QDEFUN( onu_IgmpSnoopShowCount_Func,
        onu_Igmp_snoop_ShowCount_Cmd,
        "show igmp-snooping groupcount",
        DescStringCommonShow
        "IGMP Snooping command\n"
        "The count of groups presenting\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    vty->node = ONU_DEVICE_NODE; 
    vty_out( vty, " The count of groups is %d. \r\n", g_ulIgmp_GroupCount );
#endif
    return CMD_SUCCESS;

}

QDEFUN( onu_IgmpSnoopShowHostTimeout_Func,
        onu_Igmp_snoop_ShowHostTimeout_Cmd,
        "show igmp-snooping hosttimeout",
        DescStringCommonShow
        "IGMP Snooping command\n"
        "Host timeout value\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    vty->node = ONU_DEVICE_NODE; 
    vty_out( vty, " The host timeout value is %d seconds. \r\n", g_ulIgmp_MemAge_Time);
#endif
    return CMD_SUCCESS;

}


QDEFUN( onu_IgmpSnoopGroupLife_Func,
        onu_Igmp_snoop_GroupLife_Cmd,
        "igmp-snooping grouplife [<10-1000>|default]",
        "IGMP Snooping command\n"
        "Change group life time\n"
        "Group life time value,the range is <10-1000> seconds\n"
        "Set to default value\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    vty->node = ONU_DEVICE_NODE; 
    if ( 0 == VOS_StrCmp( argv[ 0 ], "default" ) )
    {
        g_ulIgmp_GroupLife = IGMP_GROUP_LIFETIME_DEFAULT;
    }
    else
    {
        g_ulIgmp_GroupLife = VOS_AtoL( argv[ 0 ] ) ;
    }
    vty_out( vty, " Igmp snooping gouplife set OK!\r\n");
#endif    
    return CMD_SUCCESS;

}


QDEFUN( onu_IgmpSnoopShowGroupLife_Func,
        onu_Igmp_snoop_ShowGroupLife_Cmd,
        "show igmp-snooping grouplife",
        DescStringCommonShow
        "IGMP Snooping command\n"
        "IGMP Snooping group life time value\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    vty->node = ONU_DEVICE_NODE; 
    vty_out( vty, " The group life is %d seconds. \r\n", g_ulIgmp_GroupLife );
#endif
    return CMD_SUCCESS;

}


QDEFUN( onu_IgmpSnoopFilterCfg_Func,
        onu_Igmp_snoop_FilterCfg_Cmd,
        "igmp-snooping filter [<0-100>|default]",
        "IGMP Snooping command\n"
        "Change receive packet filter max variable\n"
        "Recv Packet Filter value, max packets/perport/persecond <0-100> 0 nolimit\n"
        "Set to default value\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    vty->node = ONU_DEVICE_NODE; 
    if ( 0 == VOS_StrCmp( argv[ 0 ], "default" ) )
    {
        g_ulIgmp_Packet_Filter = IGMP_PACKET_FILTER_DEFAULT;
    }
    else
    {
        g_ulIgmp_Packet_Filter = VOS_AtoL( argv[ 0 ] ) ;
    }
    vty_out( vty, " Igmp snooping filter set OK!\r\n");
#endif
    return CMD_SUCCESS;
}


QDEFUN( onu_IgmpSnoopShowFilterCfg_Func,
        onu_Igmp_snoop_ShowFilterCfg_Cmd,
        "show igmp-snooping filter",
        DescStringCommonShow
        "IGMP Snooping command\n"
        "Packet filter variable\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    vty->node = ONU_DEVICE_NODE; 
    vty_out( vty, " The Receive Packet Filter value is %d. \r\n", g_ulIgmp_Packet_Filter );
#endif
    return CMD_SUCCESS;
}

QDEFUN( onu_IgmpSnoopFastleaveCfg_Func,
        onu_Igmp_snoop_FastLeaveCfg_Cmd,
        "igmp-snooping fastleave [enable|disable] <port_list>",
        "IGMP Snooping command\n"
        "Change ports' fast leave config(when snooping)\n"
        "Enable fastleave\n"
        "Disable fastleave\n"
        "Specify port number list you want to change(e.g.: 1-3, 5,6-9)\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    long   state         = 0;
	ULONG  ulPortIfindex = 0;
	ULONG  ulUserSlot    = 0;
    ULONG  ulUserPort    = 0;
    ULONG ulPort = 0;

    vty->node = ONU_DEVICE_NODE; 	
    if ( 0 == VOS_StrCmp( argv[ 0 ], "enable" ) )
    {
        state = 1;
    }
    else
    {
        state = 0;
    }

    if ( ONU_CheckPortListValid( argv[1] ) != 1 )
    {
        vty_out( vty, "  %% Invalid port list <%s>.\r\n", argv[ 1 ] );
        return CMD_WARNING;
    }

    BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], ulPort )
    {
        ulUserSlot = 1;
        ulUserPort = ulPort;

	if((ulUserSlot > 0) && (ulUserSlot <= MAX_SLOT_NUM) &&
    	   (ulUserPort > 0) && (ulUserPort <= IGMP_SNOOP_ONU_PORTMAX))
	{
             g_sPortSpecCfg[ulUserSlot][ulUserPort].lFastLeave = state;
	}

    }
    END_PARSE_PORT_LIST_TO_PORT();
    vty_out( vty, " Igmp snooping fastleave set OK!\r\n" );
#endif
    return CMD_SUCCESS;
}

QDEFUN( onu_IgmpSnoopShowFastleaveCfg_Func,
        onu_Igmp_snoop_ShowFastleaveCfg_Cmd,
        "show igmp-snooping fastleave",
        DescStringCommonShow
        "IGMP Snooping command\n"
        "Ports' fast leave config\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    long iLoop  = 0;

    vty->node = ONU_DEVICE_NODE; 
	for(iLoop = 1; iLoop <= IGMP_SNOOP_ONU_PORTMAX; iLoop++)
	{
	   vty_out( vty, " Port %2d: %s\r\n", iLoop, g_sPortSpecCfg[1][iLoop].lFastLeave ? "enable" : "disable");
	}
#endif
    return CMD_SUCCESS;
}


QDEFUN( onu_IgmpSnoopHosttimeoutCfg_Func,
        onu_Igmp_snoop_HosttimeoutCfg_Cmd,
        "igmp-snooping hosttimeout <20-700>",
        "IGMP Snooping command\n"
        "Config host time out(when snooping)\n"
        "Time seconds\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    vty->node = ONU_DEVICE_NODE; 
     g_ulIgmp_MemAge_Time = VOS_AtoL( argv[ 0 ] ) ;
     vty_out( vty, " Igmp snooping hosttimeout set OK!\r\n" );
#endif
     return CMD_SUCCESS;
}


QDEFUN( onu_IgmpSnoopSetRouterVlan_Func,
        onu_Igmp_snoop_SetRouterVlan_Cmd,
        "igmp-snooping addrouter <1-26> vlan <vlanname>",
        "IGMP Snooping command\n"
        "Set router port\n"
        "Port number\n"
        "VLAN which the group belongs to\n"
        "VLAN name\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    ULONG ulSlot, ulPort;
    SYS_IF_INDEX_U unIfIndex, unPhyIfIndex;
    USHORT usVid;
    LONG lRet;
    ULONG ulTagged = 0;
    ULONG ulGroup;
    MC_Group_t **pMcGroup ;
    MC_Group_Vlan_t  **pVlan ;
    struct MC_Port_State **ppPortList ;

    vty->node = ONU_DEVICE_NODE; 
    /* argment validation */
    if ( argc != 2 )
    {
        vty_out( vty, " %% Invalid argment number.\r\n" );

        return VOS_ERROR;
    }
    ulSlot = 1;
    ulPort = VOS_AtoL( argv[ 0 ] ) ;
     if( ulPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , ulPort );
    	return CMD_WARNING ;
    }
    unPhyIfIndex = IFM_ETH_CREATE_INDEX(ulSlot,ulPort);    

    unIfIndex = IFM_GetIfindexByNameApi(argv[1]);
    /* Get vlan id from vlan name */
    if ( !SYS_IS_VLAN_IF( unIfIndex ) ) 
    {
        vty_out( vty, " %% Vlan %s does not exist. \r\n", argv[ 1 ] );
        return VOS_ERROR;
    }

    usVid = SYS_IF_VLAN_ID( unIfIndex );
    /* Check port vlan relation */
    IFM_VlanPortRelationApi( unPhyIfIndex, usVid, &ulTagged );
    if ( ulTagged == 0 )
    {
        vty_out( vty, " %% Port %s does not belong to vlan %s.\r\n", argv[ 0 ], argv[ 1 ] );
        return VOS_ERROR;
    }

    /* Add the router port in group */
    lRet = Igmp_Snoop_Add_Router( usVid, 0, unPhyIfIndex );
    if ( VOS_OK != lRet )
    {
        vty_out( vty, " %%Set port %s as vlan %s router port failed.\r\n", argv[ 0 ], argv[ 1 ] );
        return VOS_ERROR;
    }
    vty_out( vty, " Set Port %s as vlan %s router port OK!\r\n", argv[0], argv[1] );
#endif
    return CMD_SUCCESS;

}


QDEFUN( onu_IgmpSnoopDelRouterVlan_Func,
        onu_Igmp_snoop_DelRouterVlan_Cmd,
        "igmp-snooping delrouter <1-26> vlan <vlanname>",
        "IGMP Snooping command\n"
        "Delete router port\n"
        "Port number\n"
        "VLAN which the group belongs to\n"
        "VLAN name\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    ULONG ulSlot, ulPort;
    SYS_IF_INDEX_U unIfIndex, unPhyIfIndex;
    USHORT usVid;
    LONG lRet;
    ULONG ulTagged = 0;
    ULONG ulGroup;

    vty->node = ONU_DEVICE_NODE; 
    /* argment validation */
    if ( argc != 2 )
    {
        vty_out( vty, " %% Invalid argment number.\r\n" );
        return VOS_ERROR;
    }

    ulSlot = 1;
    ulPort = VOS_AtoL( argv[ 0 ] ) ;
    if( ulPort >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , ulPort );
    	return CMD_WARNING ;
    }
    unPhyIfIndex = IFM_ETH_CREATE_INDEX(ulSlot,ulPort);   
    
    unIfIndex = IFM_GetIfindexByNameApi(argv[1]);
    /* Get vlan id from vlan name */
    if ( !SYS_IS_VLAN_IF( unIfIndex ) ) 
    {
        vty_out( vty, " %% Vlan %s does not exist. \r\n", argv[ 1 ] );
        return VOS_ERROR;
    }

    usVid = SYS_IF_VLAN_ID( unIfIndex );

    /* Check port vlan relation */
    IFM_VlanPortRelationApi( unPhyIfIndex, usVid, &ulTagged );
    if ( ulTagged == 0 )
    {
        vty_out( vty, " %% Port %s does not belong to vlan %s.\r\n", argv[ 0 ], argv[ 1 ] );
        return VOS_ERROR;
    }

    /* delete the router port in group */
    lRet = Igmp_Snoop_Del_Router( usVid, 0, unPhyIfIndex );
    if ( VOS_OK != lRet )
    {
        vty_out( vty, " %% Delete port %s as vlan %s router port failed.\r\n", argv[ 0 ], argv[ 1 ] );
        return VOS_ERROR;
    }
    vty_out( vty, " Delete port %s as vlan %s router port OK!\r\n");
#endif
    return CMD_SUCCESS;

}

QDEFUN( onu_IgmpSnoopShowRouterVlan_Func,
        onu_Igmp_snoop_ShowRouterVlan_Cmd,
        "show igmp-snooping router",
        DescStringCommonShow
        "IGMP Snooping command\n"
        "Multicast router port\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
    ULONG usSlot, usPort;
    SYS_IF_INDEX_U unIfIndex;
    USHORT usVid;
    Igmp_RouterPort_t *pRouter = NULL;
    CHAR acName[ 32 ];

    vty->node = ONU_DEVICE_NODE; 
#if 0 /* del by suxq */

    if ( !Igmp_Snoop_IsEnable() )
    {

        vty_out( vty, "\r\n%% IGMP snooping module does not enable.\r\n" );

        return VOS_ERROR;
    }
#endif
#if PRODUCT_EPON3_ONU == RPU_YES
    if(g_ulIgmpPublicRouter)
    {
        vty_out( vty, " Sys router port = Port %d\r\n", IFM_ETH_GET_PORT(g_ulIgmpPublicRouter));
    }	
	else
	{
        vty_out( vty, " Sys have no router port\r\n");
    }
	
#else

    vty_out( vty, "\r\n VLAN             router port  \r\n" );

    pRouter = g_pstRouterList;
    while ( NULL != pRouter )
    {
        IFM_GetVlanNameApi( ( USHORT ) ( pRouter->lVid ), acName, 32 );

        unIfIndex = pRouter->ulIfIndex;

        usSlot = SYS_IF_SLOT_ID( unIfIndex );
        usPort = SYS_IF_PORT_ID( unIfIndex );
        vty_out( vty, " %-18s  %d\r\n", acName, usPort );

        pRouter = pRouter->next;
    }
#endif
#endif
    return CMD_SUCCESS;

}


/*Qos*/ /*Added by suipl 2007/09/24 */

DEFUN( onu_qos_forward_entry_func,
        onu_qos_forward_entry_cmd,
        "qos mac <H.H.H> vlan <vlanname> <1-26>  queueid <0-3>",
        "Create a permanent mac forwarding entry \n"
        "MAC address \n"
        "Please input MAC address \n"
        "VLAN associated with MAC address \n"
        "Please input VLAN name \n"
        "Please input port number associated with MAC address\n"
        "Qos queue id\n"
        "Please input the Qos queue id\n"
         )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
	ULONG lRet ;
	CHAR Mac[ 6 ] = {0, 0, 0, 0, 0, 0};
	CHAR stMac[15]={0};
	int		arg_static = 1, arg_trunk = 0, 	arg_vlan = 1, arg_tgid = 0;
	ULONG		ulBase = 0;
      cli_l2_entry_t l2entry = {0};
	int port=0;
	int unit = 0;
	CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };
	ULONG   ulIfindex = 0;
	ULONG   ulState = 0;
	ULONG   ulVlanIfIndex =0;
	ULONG   ulTrunkIfindex=0;
	UCHAR pName[IFM_API_NAMELEN]= {0};
	ULONG ulPort = 0, ulSlot = 0;
	ULONG ulTagged = 0;    
	USHORT usVid = 0;
	struct ForwardEntryItem *temp = NULL;
	struct ForwardEntryItem *tempTail = NULL;
	struct ForwardEntryItem *temp0 = NULL;
	struct ForwardEntryItem *temp1 = NULL;
	int count=0;
	CHAR bMac[ 6 ];
	ULONG ulPri = 0;

	vty->node = ONU_DEVICE_NODE;    
	
	memcpy( bMac, SYS_PRODUCT_BASEMAC, 6 );		
	if( ulBase==argc ) { return CMD_SUCCESS;  }
	/*mac address*/
	if ( GetMacAddr( ( CHAR* ) argv[0], Mac ) != VOS_OK )
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
	}

	if((bMac[0]==Mac[0]) && (bMac[1]==Mac[1]) &&
		(bMac[2]==Mac[2]) && (bMac[3]==Mac[3]) &&
		(bMac[4]==Mac[4]) && (bMac[5]==Mac[5]))
	{
		vty_out(vty," %% Can't set MAC address with System Device's MAC. \r\n" );       
		return VOS_ERROR;
	}
	
	VOS_StrCpy( stMac, argv[0]);
	/*vlanname*/
	ulBase = ulBase+1;
	lRet = ifm_check_valid_interface_name( argv[ ulBase ] );
       if(lRet != VOS_OK){
            IFM_PError( lRet, vty );
	}			
       ulVlanIfIndex = IFM_GetIfindexByName( argv[ ulBase ] );
       if ( ulVlanIfIndex == 0 )
       {
           vty_out( vty, "  %% VLAN %s does not exist.\r\n", argv[ ulBase ] );
           return CMD_WARNING;
       }
       /*不能为Super VLAN创建静态转发表项*/ 
       if(!IFM_IsSuperVlan_Api(ulVlanIfIndex))
       {
       	vty_out(vty,"  %% Super VLAN %s can not do this operation. \r\n", argv[ulBase]);
       	return CMD_WARNING;
       }

	arg_vlan = IFM_VLAN_GET_VLANID(ulVlanIfIndex);
	usVid=arg_vlan;
	/*port*/
	ulBase = ulBase+1;
#if 0	
	VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
	VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%s", ETH_SHORT_NAME, argv[ulBase] );
	ulIfindex = IFM_GetIfindexByNameExt( ifName, &ulState );
	if ( ulIfindex == 0 )
	{/******changed by suipl 2006-09-29 for slot/port与用户使用习惯一致****************/
		VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
		VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%s", PON_SHORT_NAME, /*argv[0]*/argv[ulBase] );/*changed by suipl 2006/10/09 for ID 2754 标题:
		添加错误的静态mac地址项时，系统给出的提示不对*/
		ulIfindex = IFM_GetIfindexByNameExt( ifName, &ulState );
		if ( ulIfindex == 0 )
		{
			vty_out( vty, "  %% Can not find interface %s.\r\n", ifName );
			return CMD_WARNING;
		}      
	/****************/
	}
	IFM_GetSlotAndPortApi( argv[ ulBase ], &ulSlot, &ulPort );
#endif
	ulSlot = 1;
	ulPort = ( LONG )VOS_AtoL( argv[2]);
	if( ulPort >24 )
	{
		vty_out( vty, " %% You can not config port %d \r\n" , ulPort );
		return CMD_WARNING ;
	}
	ulIfindex = IFM_ETH_CREATE_INDEX( 1, ulPort );
	
	/*判断接口是否Group到Trunk中去了.*/
	if(VOS_OK == IFM_PhyIsMerged2TrunkApi(ulIfindex))
	{
		if(VOS_OK != IFM_GetTrunkIfIndexByPhyIdxApi(ulIfindex,&ulTrunkIfindex))
		{
			ASSERT(0);
			return CMD_WARNING;
		}
		IFM_GetIfNameApi( ulTrunkIfindex, pName, IFM_API_NAMELEN );
		vty_out(vty,"  %% Port %d/%d has been grouped in trunk %s.\r\n",ulSlot, ulPort, pName);
		return CMD_WARNING;
        }
    
	port= IFM_ETH_GET_PORT( ulIfindex );
	/*p=L2_2_PHY_PORT(port);*/
	/*p=port-1;   *//*delete by lxl 2006/09/21端口转换移到bms层*/

	/*判断接口是否加入到Vlan中去了*/
	if ( VOS_OK != IFM_VlanPortRelationApi( ulIfindex, usVid, &ulTagged ) )
	{
		if ( SYS_IS_TRUNK_IF( ulIfindex ) )
		{
			vty_out( vty, "  %% Trunk %s does not belong to VLAN %s.\r\n", argv[ 2 ], argv[ 1 ] );
		}
		else
		{
			vty_out( vty, "  %% Port %s does not belong to VLAN %s.\r\n", argv[ 2 ], argv[ 1 ] );
		}
		return CMD_WARNING;
	}
	if ( 0 == ulTagged )
	{
		if ( SYS_IS_TRUNK_IF( ulIfindex ) )
		{
			vty_out( vty, "  %% Trunk %s does not belong to VLAN %s.\r\n", argv[ 2 ], argv[ 1 ] );
		}
		else
		{
			vty_out( vty, "  %% Port %s does not belong to VLAN %s.\r\n", argv[ 2 ], argv[ 1 ] );
		}

		return CMD_WARNING;
	}
	
	if(VOS_SemTake( semForwardEntry , 1000 )!=VOS_OK)
	{
		vty_out(vty , "  %% Can't do this operation at this time.%s" , VTY_NEWLINE);
		return CMD_WARNING;
	}

      	ulPri = ( ULONG )VOS_AtoL( argv[ 3 ] );
	
	/*先判断是否达到最大值*/
	count = bms_l2_show_count(unit,1);
	if( count >= FORWARD_MAX_NUM )
	{
		vty_out(vty, " %%The Max Static number is %d\r\n", FORWARD_MAX_NUM);
		VOS_Free( temp );
		VOS_SemGive( semForwardEntry );
		return CMD_WARNING;
	}	

	temp = ( struct ForwardEntryItem * ) VOS_Malloc( sizeof( struct ForwardEntryItem ), MODULE_RPU_CLI );
	if ( temp == NULL )
	{
		vty_out( vty, "  %% Not enough memory.%s", VTY_NEWLINE, VTY_NEWLINE );
		VOS_SemGive( semForwardEntry );
		return CMD_WARNING;
	}
	memset( ( CHAR * ) temp, 0, sizeof( struct ForwardEntryItem ) );
	/*sys_console_printf( "Sizeof( struct ForwardEntryItem ) is %d\r\n", sizeof( struct ForwardEntryItem ) );*//*52*/

	temp->next = 0;
	temp->usVlanID = usVid;
	temp->ulVlanIfIndex = ulVlanIfIndex;
	VOS_MemCpy(temp->aucMacAddr, Mac, 6);
	VOS_StrCpy(temp->stMac, argv[0]);
	temp->ulIfindex = ulIfindex;
	temp->usTrunkFlag = 0;
	temp->usTrunkID = 0;
	temp->ulTrunkIfIndex = 0;
	temp->usDropFlag = 0;
	temp->ulPri = ulPri;


	if ( g_stStaticForwardEntryList == NULL )
	{
		/*******先设置后插入*****************/
		VOS_MemCpy(l2entry.mac, Mac, 6);
		l2entry.tgid_port = port;
		l2entry.vid = arg_vlan;
		l2entry.tgid = arg_tgid;
		l2entry.trunk = arg_trunk;
		l2entry.l2_static = arg_static;
		l2entry.pri = ulPri;


		lRet = bms_l2_add(unit,&l2entry);
		if( lRet != VOS_OK )
		{
			VOS_Free( temp );
			VOS_SemGive( semForwardEntry );
			return CMD_WARNING;
		}
				
		g_stStaticForwardEntryList = temp;
		/*g_iForwardEntryCount++;*/
		VOS_SemGive( semForwardEntry );
		
		return CMD_SUCCESS;
	}
	else
	{
		temp0 = g_stStaticForwardEntryList;
		while ( temp0 != 0 )
		{
			/*if ((VOS_StrCmp(temp0->stMac, stMac) == 0) && (temp0->usVlanID == arg_vlan) &&( temp0->ulIfindex == ulIfindex))*/
			/*Changed by suipl 2007/08/16*/
			if ((VOS_MemCmp(temp0->aucMacAddr, Mac, 6) == 0) && (temp0->usVlanID == arg_vlan) &&( temp0->ulIfindex == ulIfindex)&&( temp0->ulPri == ulPri ))
			{				
				vty_out( vty, "  %%qos mac %2.2X%2.2X.%2.2X%2.2X.%2.2X%2.2X vlan %s port %s queue-no %s had been added!\r\n",
					Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5], argv[1], argv[2], argv[3]);
				VOS_Free( temp );
				VOS_SemGive( semForwardEntry );
				return CMD_WARNING;
			}/*delete the item*/
			temp0=temp0->next;
		}

		temp1 = g_stStaticForwardEntryList;
		while ( temp1 != 0 )
		{
			/*if ((VOS_StrCmp(temp1->stMac, stMac) == 0)&& (temp1->usVlanID == arg_vlan ))*//*Changed by suipl 2007/08/16*/
			if ((VOS_MemCmp(temp1->aucMacAddr, Mac, 6) == 0)&& (temp1->usVlanID == arg_vlan ))
			{				
				temp1->ulIfindex = ulIfindex;
				temp1->usTrunkFlag=0;
				temp1->usTrunkID = 0;
				temp1->ulTrunkIfIndex = 0;
				temp1->usDropFlag =0;
				temp1->ulPri = ulPri;
				/*******先设置后插入*****************/
				VOS_MemCpy(l2entry.mac, Mac, 6);
				l2entry.tgid_port = port;
				l2entry.vid = arg_vlan;
				l2entry.tgid = arg_tgid;
				l2entry.trunk = arg_trunk;
				l2entry.l2_static = arg_static;
				l2entry.pri = ulPri;

				lRet = bms_l2_add(unit,&l2entry);
				if( lRet != VOS_OK )
				{
					vty_out( vty, "  %%Forward-entry mac %s vlan %s %s be added FAILED!\r\n", argv[0], argv[1], argv[2]);
					VOS_Free( temp );
					VOS_SemGive( semForwardEntry );
					return CMD_WARNING;
				}
				VOS_Free( temp );
				VOS_SemGive( semForwardEntry );	
				return CMD_SUCCESS;
				/************************************************/
			}/*delete the item*/
			temp1=temp1->next;
		}
		
		/*******先设置后插入*****************/	
		VOS_MemCpy(l2entry.mac, Mac, 6);
		l2entry.tgid_port = port;
		l2entry.vid = arg_vlan;
		l2entry.tgid = arg_tgid;
		l2entry.trunk = arg_trunk;
		l2entry.l2_static = arg_static;
		l2entry.pri = ulPri;

		lRet = bms_l2_add(unit,&l2entry);
		if( lRet != VOS_OK )
		{
			vty_out( vty, "  %%Forward-entry mac %s vlan %s %s be added FAILED!\r\n", argv[0], argv[1], argv[2]);
			VOS_Free( temp );
			VOS_SemGive( semForwardEntry );
			return CMD_WARNING;
		}		
		/**********************************/

		tempTail = g_stStaticForwardEntryList;
		while ( tempTail->next != NULL )
			tempTail = tempTail->next;
		tempTail->next = temp;
		
		VOS_SemGive( semForwardEntry );
		vty_out( vty, " qos forward-entry created OK!\r\n");
		/*g_iForwardEntryCount++;*/
		return CMD_SUCCESS;
	}
#endif

	return CMD_SUCCESS;
}         

DEFUN( onu_qos_port_based_set_func,
        onu_qos_port_based_set_cmd,
        "qos port-based [enable|disable] port [all|<1-26>] ",
        "set qos \n"
        "set port_based qos \n"
        "port_based mode enable\n"
        "port_based mode disable\n"
        "port number \n"
        "apply to all port\n"
        "apply to a specified port\r\n"
         )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    QOS_SIMPLE_RULE_S qos_set={};
    int port=0,i;
    CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };
    ULONG   ulIFIndex = 0;
    ULONG   ulState = 0;
    LONG lRet = 0;
	
    qos_set.cPort_based=-1;
    qos_set.cMac=-1;
    qos_set.cDscp_Tos=-1;
    qos_set.cDscpTos_en=-1;
    qos_set.cDscp=-1;	
    qos_set.cTos=-1;
    qos_set.cQueue_no=-1;
    qos_set.cPort=-1;	

    vty->node = ONU_DEVICE_NODE;    

    if ( VOS_StrCmp( argv[0], "enable")==0)
    {
    	qos_set.cPort_based = 2;
    }
    else if ( VOS_StrCmp( argv[0], "disable")==0)
    {
    	qos_set.cPort_based = 1;
    }
    if ( VOS_StrCmp( argv[1], "all")==0)
    {
    	qos_set.cPort = 0xff;
    	for (i=0;i<27;i++)
    	{
    		g_Qos_Port[i].cPort_based = qos_set.cPort_based;
    	}
    }
    else
    {
	 port = ( LONG )VOS_AtoL( argv[1]);
	 if( port >24 )
	 {
	 	vty_out( vty, " %% You can not config port %d \r\n" , port );
	 	return CMD_WARNING ;
	 }
	 qos_set.cPort = port;
	 g_Qos_Port[port].cPort_based = qos_set.cPort_based;
    }
    lRet = ctss_qos_simple_process_hook(&qos_set,NULL,NULL,0);
    if( lRet == VOS_OK )
    	vty_out( vty, " qos port-based set OK!\r\n" );
    else
    {
    	vty_out( vty, " qos port-based set ERROR!\r\n");
    	return CMD_WARNING;
    }
    
#endif
    return CMD_SUCCESS;
}

DEFUN( onu_qos_8021p_set_func,
        onu_qos_8021p_set_cmd,
        "qos 8021p [enable|disable] port [all|<1-26>] ",
        "set qos \n"
        "set 802.1p qos \n"
        "802.1p mode enable\n"
        "802.1p mode disable\n"
        "port number \n"
        "apply to all port\n"
        "apply to a specified port\r\n"
         )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    QOS_SIMPLE_RULE_S qos_set={};
    int port=0,i;
    CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };
    ULONG   ulIFIndex = 0;
    ULONG   ulState = 0;
    LONG lRet = 0;

    qos_set.cPort_based=-1;
    qos_set.cMac=-1;
    qos_set.cDscp_Tos=-1;
    qos_set.cDscpTos_en=-1;
    qos_set.cDscp=-1;	
    qos_set.cTos=-1;
    qos_set.cQueue_no=-1;
    qos_set.cPort=-1;	    

    vty->node = ONU_DEVICE_NODE;    
    if ( VOS_StrCmp( argv[0], "enable")==0)
    {
    	qos_set.c8021p= 2;
    }
    else if ( VOS_StrCmp( argv[0], "disable")==0)
    {
    	qos_set.c8021p = 1;
    }

    if ( VOS_StrCmp( argv[1], "all")==0)
    {
    	qos_set.cPort = 0xff;
    	for (i=0;i<27;i++)
    	{
    		g_Qos_Port[i].c8021p= qos_set.c8021p;
    	}
    }
    else
    {
    	port = ( LONG )VOS_AtoL( argv[1]);
    	if( port >24 )
	 {
	 	vty_out( vty, " %% You can not config port %d \r\n" , port );
	 	return CMD_WARNING ;
	 }
    	qos_set.cPort = port;
    	g_Qos_Port[port].c8021p= qos_set.c8021p;
    }
    lRet = ctss_qos_simple_process_hook(&qos_set,NULL,NULL,0);
    if( lRet == VOS_OK )
    	vty_out( vty, " qos 8021p set OK!\r\n" );
    else
    {
    	vty_out( vty, " qos 8021p set ERROR!\r\n");
    	return CMD_WARNING;
    }
#endif
    return CMD_SUCCESS;
}

    

DEFUN( onu_qos_dscp_tos_select_func,
        onu_qos_dscp_tos_select_cmd,
        "qos dscp-tos select [dscp|tos] ",
        "set qos \n"
        "set dscp/tos qos \n"
        "select dscp or tos\n"
        "select dscp\n"
        "select tos \n"
         )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    QOS_SIMPLE_RULE_S qos_set={};
    ULONG   ulState = 0;
    LONG     lRet = 0;
    qos_set.cPort_based=-1;
    qos_set.cMac=-1;
    qos_set.cDscp_Tos=-1;
    qos_set.cDscpTos_en=-1;
    qos_set.cDscp=-1;	
    qos_set.cTos=-1;
    qos_set.cQueue_no=-1;
    qos_set.cPort=-1;	

    vty->node = ONU_DEVICE_NODE;    
    if ( VOS_StrCmp( argv[0], "dscp")==0)
    {
    	qos_set.cDscp_Tos= QOS_SIMPLE_SELECT_DSCP;
    }
    else if ( VOS_StrCmp( argv[0], "tos")==0)
    {
    	qos_set.cDscp_Tos= QOS_SIMPLE_SELECT_TOS;
    }
    g_cDscpTos = qos_set.cDscp_Tos;
    lRet = ctss_qos_simple_process_hook(&qos_set,NULL,NULL,0);
     if( lRet == VOS_OK )
    	vty_out( vty, " qos dscp-tos select  OK!\r\n" );
    else
    {
    	vty_out( vty, " qos dscp-tos select ERROR!\r\n");
    	return CMD_WARNING;
    }
#endif    
    return CMD_SUCCESS;
}

DEFUN( onu_qos_port_dscp_tos_set_func,
        onu_qos_port_dscp_tos_set_cmd,
        "qos dscp-tos [enable|disable] port [all|<1-26>]",
        "set qos \n"
        "set dscp/tos qos \n"
        "dscp/tos mode enable\n"
        "dscp/tos mode disable\n"         
        "port number \n"
        "apply to all port\n"
        "apply to a specified port\r\n"       
         )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    QOS_SIMPLE_RULE_S qos_set={};
    int port=0,i;
    CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };
    ULONG   ulIFIndex = 0;
    ULONG   ulState = 0;
    LONG lRet = 0;
    
    qos_set.cPort_based=-1;
    qos_set.cMac=-1;
    qos_set.cDscp_Tos=-1;
    qos_set.cDscpTos_en=-1;
    qos_set.cDscp=-1;	
    qos_set.cTos=-1;
    qos_set.cQueue_no=-1;
    qos_set.cPort=-1;	

    vty->node = ONU_DEVICE_NODE;    
    if ( VOS_StrCmp( argv[0], "enable")==0)
    {
    	qos_set.cDscpTos_en= 2;
    }
    else if ( VOS_StrCmp( argv[0], "disable")==0)
    {
    	qos_set.cDscpTos_en= 1;
    }

    if ( VOS_StrCmp( argv[1], "all")==0)
    {
    	qos_set.cPort = 0xff;
    	for (i=0;i<27;i++)
    	{
    		g_Qos_Port[i].cDscpTos_en= qos_set.cDscpTos_en;
    	}
    }
    else
    {
    	port = ( LONG )VOS_AtoL( argv[1]);
    	if( port >24 )
	 {
	 	vty_out( vty, " %% You can not config port %d \r\n" , port );
	 	return CMD_WARNING ;
	 }
    	qos_set.cPort = port;
    	g_Qos_Port[port].cDscpTos_en= qos_set.cDscpTos_en;		   
    } 
    lRet = ctss_qos_simple_process_hook(&qos_set,NULL,NULL,0);    
    if( lRet == VOS_OK )
    	vty_out( vty, " qos dscp-tos set  OK!\r\n" );
    else
    {
    	vty_out( vty, " qos dscp-tos set ERROR !\r\n");
    	return CMD_WARNING;
    }
#endif
    return CMD_SUCCESS;
}

DEFUN( onu_qos_dscp_map_set_func,
        onu_qos_dscp_map_set_cmd,
        "qos dscp-tos map dscp <0-63> queue-no <0-3>",
        "set qos \n"
        "set dscp/tos qos \n"
        "map dscp/tos to queue \n"
        "dscp map to queue\n"
        "dscp value\n"
        "queue number\n"
        "queue number value\n"
         )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    QOS_SIMPLE_RULE_S qos_set={};
    LONG lRet = 0;
    
    qos_set.cPort_based=-1;
    qos_set.cMac=-1;
    qos_set.cDscp_Tos=-1;
    qos_set.cDscpTos_en=-1;
    qos_set.cDscp=-1;	
    qos_set.cTos=-1;
    qos_set.cQueue_no=-1;
    qos_set.cPort=-1;	

    vty->node = ONU_DEVICE_NODE;   

    qos_set.cDscp = ( CHAR ) VOS_AtoL( argv[0] );
    qos_set.cQueue_no = ( CHAR ) VOS_AtoL( argv[1] );	  
    g_ucDscp[qos_set.cDscp] = qos_set.cQueue_no;
    lRet = ctss_qos_simple_process_hook(&qos_set,NULL,NULL,0);
    if( lRet == VOS_OK )
    	vty_out( vty, " qos dscp-tos map dscp set  OK!\r\n" );
    else
    {
    	vty_out( vty, " qos dscp-tos map dscp set ERROR !\r\n");
    	return CMD_WARNING;
    }
#endif
    return CMD_SUCCESS;
}

DEFUN( onu_qos_tos_map_set_func,
        onu_qos_tos_map_set_cmd,
        "qos dscp-tos map tos <0-7> queue-no <0-3>",
        "set qos \n"
        "set dscp/tos qos \n"
        "map dscp/tos to queue \n"
        "tos map to queue\n"
        "tos value\n"
        "queue number\n"
        "queue number value\n"
         )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    QOS_SIMPLE_RULE_S qos_set={};
    LONG lRet = 0;
	
    qos_set.cPort_based=-1;
    qos_set.cMac=-1;
    qos_set.cDscp_Tos=-1;
    qos_set.cDscpTos_en=-1;
    qos_set.cDscp=-1;	
    qos_set.cTos=-1;
    qos_set.cQueue_no=-1;
    qos_set.cPort=-1;	

    vty->node = ONU_DEVICE_NODE; 
    qos_set.cTos = ( CHAR ) VOS_AtoL( argv[0] );
    qos_set.cQueue_no = ( CHAR ) VOS_AtoL( argv[1] );	
    g_ucTos[qos_set.cTos] = qos_set.cQueue_no;
    lRet = ctss_qos_simple_process_hook(&qos_set,NULL,NULL,0);
    if( lRet == VOS_OK )
    	vty_out( vty, " qos dscp-tos map tos set  OK!\r\n" );
    else
    {
    	vty_out( vty, " %%qos dscp-tos map tos set ERROR !\r\n");
    	return CMD_WARNING;
    }
#endif
    return CMD_SUCCESS;
}

DEFUN( show_onu_qos_by_port_func,
        show_onu_qos_by_port_cmd,
        "show qos port <1-26>",
        DescStringCommonShow
        "show qos \n"
        "show port qos status"
        "port number\n"
         )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    ULONG ulSlot = 1;
    int port=0;
    CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };
    ULONG   ulIFIndex = 0;
    ULONG   ulState = 0;

    vty->node = ONU_DEVICE_NODE; 
    port = ( LONG )VOS_AtoL( argv[1]);
    if( port >24 )
    {
    	vty_out( vty, " %% You can not config port %d \r\n" , port );
    	return CMD_WARNING ;
    }

    vty_out(vty," Interface port%-3d \r\n  qos status :\r\n",  port);
    if (g_Qos_Port[port].cPort_based==1)
    	vty_out(vty,"     port_based disable\r\n");
    else if (g_Qos_Port[port].cPort_based==2)
    	vty_out(vty,"     port_based enable\r\n");

    if (g_Qos_Port[port].cDscpTos_en==1)
    {
    	if (g_cDscpTos ==QOS_SIMPLE_SELECT_DSCP)
    	{
    		vty_out(vty,"     dscp       disable\r\n");
    	}
    	else if (g_cDscpTos ==QOS_SIMPLE_SELECT_TOS)
    	{
    		vty_out(vty,"     tos        disable\r\n");
    	}
    }
    else if (g_Qos_Port[port].cDscpTos_en==2)
    {
    	if (g_cDscpTos ==QOS_SIMPLE_SELECT_DSCP)
    	{
    		vty_out(vty,"     dscp       enable\r\n");
    	}
    	else if (g_cDscpTos ==QOS_SIMPLE_SELECT_TOS)
    	{
    		vty_out(vty,"     tos        enable\r\n");
    	}
    }

    if (g_Qos_Port[port].c8021p==1)
    	vty_out(vty,"     802.1p     disable\r\n");
    else if (g_Qos_Port[port].c8021p==2)
    	vty_out(vty,"     802.1p     enable\r\n \r\n");   
#endif
   return CMD_SUCCESS;
}

DEFUN( show_onu_qos_dscp_tos_map_func,
        show_onu_qos_dscp_tos_map_cmd,
        "show qos dscp-tos map [dscp|tos]",
        DescStringCommonShow
        "show qos information\n"
        "show qos dscp-tos information\n"
        "show qos dscp-tos map table\n"
        "dscp map table\n"
        "tos map table\n"
         )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    int i;

    vty->node = ONU_DEVICE_NODE; 
    if ( VOS_StrCmp( argv[0], "dscp")==0)
    {
    	vty_out(vty," dscp map to queue table :\r\n");
    	for (i=0;i<64;i++)
    	{
    		vty_out(vty,"     dscp[%2d]:    queue-no[%d]\r\n",i,g_ucDscp[i]);
    	}
    }
    else if ( VOS_StrCmp( argv[0], "tos")==0)
    {
    	vty_out(vty," tos map to queue table :\r\n");
    	for (i=0;i<8;i++)
    	{
    		vty_out(vty,"     tos[%d]:    queue-no[%d]\r\n",i,g_ucTos[i]);
    	}
    }  
#endif
    return CMD_SUCCESS;
}

DEFUN( show_onu_qos_dscp_tos_select_func,
        show_onu_qos_dscp_tos_select_cmd,
        "show qos dscp-tos select",
        DescStringCommonShow
        "show qos information\n"
        "show qos dscp-tos information\n"
        "show qos dscp-tos selection\n"
         )
{
#if(PRODUCT_EPON3_ONU == RPU_YES)  
    vty_out(vty," The current qos dscp-tos selection is : ");
    if (g_cDscpTos == QOS_SIMPLE_SELECT_DSCP)
        vty_out(vty,"dscp\r\n");
    else if (g_cDscpTos == QOS_SIMPLE_SELECT_TOS)
        vty_out(vty,"tos\r\n");	
#endif
    return CMD_SUCCESS;
}



#if 0
QDEFUN( ONU_Qos_Port_PriQueue_G_Func,
		ONU_Qos_Port_PriQueue_G_Cmd,
		"config queue-mode pq" ,
		DescStringCommonConfig
		"Config COS queues' shceduling information\n"
		"Config priority queue mode\n",
		&g_ulQosQueId )

 {
#if(PRODUCT_EPON3_ONU == RPU_YES) 
	SYS_IF_INDEX_U unIfIndex;
	UCHAR ucSlot = 0;
	UCHAR ucPort = 0;
	UCHAR ucTempSlot=0;
	UCHAR ucTempPort=0;
	ULONG	i=0,j=0;
	ULONG ulAclConfigCount=0;
	LONG lRet;
	QOS_ACL_NODE_S *p_stAcl = NULL;
	QOS_PORT_BIND_INPOLICY_OR_ACL_NODE_S *pAclBind = NULL;
	QOS_PORT_BIND_INPOLICY_OR_ACL_NODE_S *pNewNode = NULL;

	vty->node = ONU_DEVICE_NODE; 
	for ( i = IFM_GET_FIRST_INTERFACE( IFM_ETH_TYPE );i ;i = IFM_GET_NEXT_INTERFACE( i ) )
	{
		ucTempSlot = SYS_IF_SLOT_ID(i);
		ucTempPort = SYS_IF_PORT_ID(i);	 
		if (( Qos_Port_Config_G_PriQueue( vty, i )) != VOS_OK )
		{
			vty_out( vty, " %% Invalid parameter.\r\n" );
			return CMD_WARNING;
		}
	}

	stPortQosGInfo.queueSheduleMode.isSet = QOS_TRUE;   /* 12-2 */
	stPortQosGInfo.queueSheduleMode.ulQueueMode = QOS_PORT_QUEUE_PQ;
	vty_out( vty, " Config queue-mode pq OK!\r\n" );
#endif				

	return CMD_SUCCESS;	 
  }

QDEFUN( ONU_Qos_Port_WrrQueue_G_Func,
		  ONU_Qos_Port_WrrQueue_G_Cmd,
		  "config queue-mode wrr <weight0,weight1,weight2,weight3,weight4,weight5,weight6,weight7>" ,
		  DescStringCommonConfig
		  "Config COS queues' shceduling information\n"
		  "Config WRR queue mode\n"
		  "Input each COS queue's weight <1-55>\n",
		  &g_ulQosQueId )

   {
#if(PRODUCT_EPON3_ONU == RPU_YES)    
	SYS_IF_INDEX_U unIfIndex;
	UCHAR ucSlot = 0;
	UCHAR ucPort = 0;
	UCHAR ucTempSlot=0;
	UCHAR ucTempPort=0;
	ULONG  i=0,j=0;
	ULONG ulAclConfigCount=0;
	LONG lRet;  

	vty->node = ONU_DEVICE_NODE;
	for ( i = IFM_GET_FIRST_INTERFACE( IFM_ETH_TYPE );i ;i = IFM_GET_NEXT_INTERFACE( i ) )
	{
		ucTempSlot = SYS_IF_SLOT_ID(i);
		ucTempPort = SYS_IF_PORT_ID(i);    
		if (( Qos_Port_Config_G_WrrQueue( vty, argv[ 0 ], i )) != VOS_OK)
		{
			return CMD_WARNING;
		}
	}

	stPortQosGInfo.queueSheduleMode.isSet = QOS_TRUE;   /* 12-2 */
	stPortQosGInfo.queueSheduleMode.ulQueueMode = QOS_PORT_QUEUE_WRR;
	vty_out( vty, " Config queue-mode wrr OK!\r\n" );
#endif
	return CMD_SUCCESS;
}

QDEFUN( ONU_Qos_Port_Show_CosqueWeight_G_Func,
		  ONU_Qos_Port_Show_CosqueWeight_G_Cmd,
		  "show queue-mode" ,
		  DescStringCommonShow
		  "Show COS queues'shceduling information\n" ,
		  &g_ulQosQueId)
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
	vty->node = ONU_DEVICE_NODE;
	Qos_Port_Show_G_Cosque_Weight(vty);
#endif
	return CMD_SUCCESS;
}


QDEFUN( ONU_Qos_Interface_Set_UserPri_Func,
           ONU_Qos_Interface_Set_UserPri_Cmd,
           "port <1-26> set user-priority <0-7>",
           "Config onu port parameter\n"  
           "Input the onu port number\n"
           "Set some value\n"
           "802.1p user priority\n"
           "The value of 802.1p user priority\n",
           &g_ulQosQueId )
    {
#if(PRODUCT_EPON3_ONU == RPU_YES)     
	SYS_IF_INDEX_U unIfIndex = 0;
	UCHAR ucSlot = 1;
	UCHAR ucPort = 0;
	QOS_PORT_DSCP_AND_USERPRI_S stDscpUserPri;
	LONG lRet = VOS_OK;	

	vty->node = ONU_DEVICE_NODE;
	ucPort = ( UCHAR )VOS_AtoL( argv[ 0 ] );
	if( ucPort >24 )
	{
    		vty_out( vty, " %% You can not config port %d \r\n" , ucPort );
    		return CMD_WARNING ;
    	}
	unIfIndex = IFM_ETH_CREATE_INDEX( 1, ucPort );

	VOS_SemTake( g_SemId_PortQos, WAIT_FOREVER );
	stPortQosInfo[ ucSlot ][ ucPort ].cUserPriority = ( CHAR ) VOS_AtoL( argv[ 1 ] );
	stDscpUserPri.ucSlot = ucSlot;
	stDscpUserPri.ucPort = ucPort;
	stDscpUserPri.cPortUserPri = ( CHAR ) VOS_AtoL( argv[ 1 ] );
	if ( Qos_Interface_Cmd_Process( QOS_PORT_SET_USERPRI, ( VOID * ) & stDscpUserPri, ( VOID * ) vty ) != VOS_OK )
	{
		if ( QOS_DEBUG )
		{
			QOS_SYSLOG( LOG_DEBUG, "Call Qos_Interface_Cmd_Process() set port user priority unsuccessful." );
		}
		stPortQosInfo[ ucSlot ][ ucPort ].cUserPriority = (CHAR)QOS_NO_THIS_ITEM;
		VOS_SemGive( g_SemId_PortQos );
		return CMD_WARNING;
	}
	VOS_SemGive( g_SemId_PortQos );
	vty_out( vty, " Set user-priority OK!\r\n" );
#endif         
        return CMD_SUCCESS;
    }

QDEFUN( ONU_Qos_Interface_Cancel_UserPri_Func,
           ONU_Qos_Interface_Cancel_UserPri_Cmd,
           "undo port <1-26> set user-priority",
           NO_STR
           "Config onu port parameter\n"  
           "Input the onu port number\n"           
           "Set some value\n"
           "802.1p user priority\n",
           &g_ulQosQueId )
{
#if(PRODUCT_EPON3_ONU == RPU_YES) 
	SYS_IF_INDEX_U unIfIndex = 0;
	UCHAR ucSlot = 1;
	UCHAR ucPort = 0;
	QOS_PORT_DSCP_AND_USERPRI_S stDscpUserPri;

	vty->node = ONU_DEVICE_NODE;
	ucPort = ( LONG )VOS_AtoL( argv[ 0 ] );
	if( ucPort >24 )
	{
    		vty_out( vty, " %% You can not config port %d \r\n" , ucPort );
    		return CMD_WARNING ;
    	}
	unIfIndex = IFM_ETH_CREATE_INDEX( 1, ucPort );

	stDscpUserPri.ucSlot = ucSlot;
	stDscpUserPri.ucPort = ucPort;
	stDscpUserPri.cPortUserPri =(CHAR) QOS_NO_THIS_ITEM;
	VOS_SemTake( g_SemId_PortQos, WAIT_FOREVER );
	if ( stPortQosInfo[ ucSlot ][ ucPort ].cUserPriority == ( CHAR ) QOS_NO_THIS_ITEM )
	{
		VOS_SemGive( g_SemId_PortQos );
		return CMD_SUCCESS;
	}
	if ( Qos_Interface_Cmd_Process( QOS_PORT_CANCEL_USERPRI, ( VOID * ) & stDscpUserPri, ( VOID * ) vty ) != VOS_OK )
	{
		if ( QOS_DEBUG )
		{
			QOS_SYSLOG( LOG_DEBUG, "Call Qos_Interface_Cmd_Process() cancel port user priority unsuccessful." );
		}
		VOS_SemGive( g_SemId_PortQos );
		return CMD_WARNING;
	}
	stPortQosInfo[ ucSlot ][ ucPort ].cUserPriority = (CHAR)QOS_NO_THIS_ITEM;
	VOS_SemGive( g_SemId_PortQos );
	vty_out( vty, " Undo set user-priority OK!\r\n");
#endif        
        return CMD_SUCCESS;
}

 QDEFUN( Onu_Qos_Interface_Show_Qos_Func,
           Onu_Qos_Interface_Show_Qos_Cmd,
           "show port <1-26> qos",
           SHOW_STR
           "Config onu port parameter\n"  
           "Input the onu port number\n"          
           "Parameter(s) for quality of service\n",
           &g_ulQosQueId )
    {
    #if(PRODUCT_EPON3_ONU == RPU_YES) 
    	SYS_IF_INDEX_U unIfIndex = 0;
	UCHAR ucSlot = 1;
	UCHAR ucPort = 0;
	QOS_PORT_DSCP_AND_USERPRI_S stDscpUserPri;
	int i = 0;

	vty->node = ONU_DEVICE_NODE;
	ucPort = ( LONG )VOS_AtoL( argv[ 0 ] );
	if( ucPort >24 )
	{
    		vty_out( vty, " %% You can not config port %d \r\n" , ucPort );
    		return CMD_WARNING ;
    	}
	unIfIndex = IFM_ETH_CREATE_INDEX( 1, ucPort );
	
        vty_out( vty, "===============================================\r\n" );
        vty_out( vty, " Interface-Ethernet:         %u\r\n", ucPort );
        vty_out( vty, "\r\n" );

        if ( stPortQosInfo[ ucSlot ][ ucPort ].cUserPriority != ( CHAR ) QOS_NO_THIS_ITEM )
        {
			vty_out( vty, " Set 802.1p user priority:   %u\r\n", stPortQosInfo[ ucSlot ][ ucPort ].cUserPriority );
        }
        if ( stPortQosInfo[ ucSlot ][ ucPort ].cDscp != ( CHAR ) QOS_NO_THIS_ITEM )
        {
        	vty_out( vty, " Set dscp:                   %u\r\n", stPortQosInfo[ ucSlot ][ ucPort ].cDscp );
        }
        if ( stPortQosInfo[ ucSlot ][ ucPort ].cUserPriority != ( CHAR ) QOS_NO_THIS_ITEM
                || stPortQosInfo[ ucSlot ][ ucPort ].cDscp != ( CHAR ) QOS_NO_THIS_ITEM )
        {
            vty_out( vty, "\r\n" );
        }
		
        vty_out( vty, " Policy-map number:          %u\r\n", stPortQosInfo[ ucSlot ][ ucPort ].stPortIngressPolicy.ulListNodeCount );
        /* vty_out( vty, " Policy-map name:            " ); */
        Qos_Interface_Ingress_Policy_Show( &( stPortQosInfo[ ucSlot ][ ucPort ].stPortIngressPolicy ), ucSlot, ucPort, vty );

		/* add queue-mode, shape, drop-policy */
		vty_out( vty, " Drop-policy:                " );
		if ( stPortQosInfo[ ucSlot ][ ucPort ].bTailDrop == QOS_TRUE )
		{
			vty_out( vty, "tail drop\r\n\r\n" );
		}
		else if ( stPortQosInfo[ ucSlot ][ ucPort ].pst_CongAvoidREDNode != NULL )
		{	/* RED */
			vty_out( vty, "RED\r\n" );
			vty_out( vty, " RED name:                   %s\r\n\r\n", stPortQosInfo[ ucSlot ][ ucPort ].pst_CongAvoidREDNode->strCongeAvoidName );
		}
		else 
		{
			vty_out( vty, "WRED\r\n" );
			vty_out( vty, " WRED name:                  %s\r\n\r\n", stPortQosInfo[ ucSlot ][ ucPort ].pst_CongAvoidWREDNode->strCongeAvoidName );
		}

		vty_out( vty, " Queue Schedule:             " );
		if ( stPortQosInfo[ ucSlot ][ ucPort ].queueSheduleMode.ulQueueMode == QOS_PORT_QUEUE_FIFO )
		{
			vty_out( vty, "FIFO\r\n\r\n" );
		}
		else if ( stPortQosInfo[ ucSlot ][ ucPort ].queueSheduleMode.ulQueueMode == QOS_PORT_QUEUE_PQ )
		{
			vty_out( vty, "PQ\r\n" );
#if( PRODUCT_CLASS ==  GT813 )/*Added by suipl 2007/09/25*/				
			for ( i = 0; i < QOS_MAX_QUEUE_NUM; i++ )
			{
				if( i > 2 )
				{
					vty_out( vty, "   wrr weight[%d] = SP\r\n", i);
					break;
				}
				vty_out( vty, "   wrr weight[%d] = %d\r\n", i, stPortQosGInfo.queueSheduleMode.ucWrr[i] );
			}
#endif				
				vty_out( vty, "\r\n" );		
		}
		else if ( stPortQosInfo[ ucSlot ][ ucPort ].queueSheduleMode.ulQueueMode == QOS_PORT_QUEUE_WRR )
		{
			vty_out( vty, "WRR\r\n" );
			vty_out( vty, " configuration: \r\n");

			for ( i = 0; i < QOS_MAX_QUEUE_NUM; i++ )
			{
#if( PRODUCT_CLASS ==  GT813 )/*Added by suipl 2007/09/25*/
					if( i > 3 )
						break;
#endif
				vty_out( vty, "   wrr weight[%d] = %d\r\n", i, stPortQosInfo[ ucSlot ][ ucPort ].queueSheduleMode.ucWrr[i] );
			}
			vty_out( vty, "\r\n" );
		}
		else 
		{
			vty_out( vty, "HyBrid\r\n" );
			vty_out( vty, " configuration: \r\n");
			vty_out( vty, "   pq-count = %d\r\n", stPortQosInfo[ ucSlot ][ ucPort ].queueSheduleMode.ucPqCount );
			for ( i = 0; i <  QOS_MAX_QUEUE_NUM; i++ )
			{
				vty_out( vty, "   wrr weight[%d] = %d\r\n", i, stPortQosInfo[ ucSlot ][ ucPort ].queueSheduleMode.ucWrr[i] );
			}
			vty_out( vty, "\r\n" );
		}

		
		vty_out( vty, " Shape configuration:\r\n" );
		 if ( stPortQosInfo[ ucSlot ][ ucPort ].shape.bIsSet == QOS_TRUE )
	        {
	            /* Changed by liangqiong 04-11-06 */
	            if( VOS_OK != Qos_Interface_Egress_Shape_Show( ucSlot, ucPort, stPortQosInfo[ ucSlot ][ ucPort ].shape.usBandWidth, 
                        stPortQosInfo[ ucSlot ][ ucPort ].shape.usBucketSize, vty ))
                    {
                        return VOS_ERROR;
                    }                
	        }
	        else
	        {
	            vty_out( vty, "    bandwidth:         no set.\r\n" );
	            vty_out( vty, "    burstsize:         no set.\r\n" );
	        }
		
		
        vty_out( vty, "===============================================\r\n" );
 #endif
 
        return CMD_SUCCESS;
    }
#endif
#if( PRODUCT_CLASS == EPON3 )

int onu_gt813_init_func()
{
    return VOS_OK;
}

int onu_gt813_showrun( struct vty * vty )
{    
    return VOS_OK;
}


int onu_gt813_config_write ( struct vty * vty )
{
    return VOS_OK;
}

struct cmd_node onu_gt813_node =
{
    ONU_GT813_NODE,
    NULL,
    1
};

LONG onu_gt813_node_install()
{
    install_node( &onu_gt813_node, onu_gt813_config_write);
    onu_gt813_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_ONU);
    if ( !onu_gt813_node.prompt )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }
    install_default( ONU_GT813_NODE );
    return VOS_OK;
}


LONG onu_gt813_module_init()
{
    struct cl_cmd_module * onu_gt813_module = NULL;

    onu_gt813_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_ONU);
    if ( !onu_gt813_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) onu_gt813_module, sizeof( struct cl_cmd_module ) );

    onu_gt813_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_ONU);
    if ( !onu_gt813_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( onu_gt813_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( onu_gt813_module->module_name, "onu_gt813" );

    onu_gt813_module->init_func = onu_gt813_init_func;
    onu_gt813_module->showrun_func = /*onu_gt813_showrun*/NULL;
    onu_gt813_module->next = NULL;
    onu_gt813_module->prev = NULL;

    cl_install_module( onu_gt813_module );
  
    return VOS_OK;
}

#endif


#if (PRODUCT_EPON3_ONU == RPU_YES)  
CMD_NOTIFY_REFISTER_S stCMD_Onu_Port_List_Check =
{
    "<port_list>",
    Onu_Check_Port_List,
    0
};
#endif

LONG ONUGT813_CommandInstall()
{ 
#if( PRODUCT_CLASS == EPON3 )
    onu_gt813_node_install();
    onu_gt813_module_init();

	onu_share_cmd_install(ONU_GT813_NODE);	/* added by xieshl 20080603, 问题单6349 */
	
    install_element( ONU_GT813_NODE, &onu_auto_enable_if_cmd);
    install_element( ONU_GT813_NODE, &onu_duplex_enable_if_cmd );
    install_element( ONU_GT813_NODE, &onu_flowcontrol_enable_if_cmd );
    install_element( ONU_GT813_NODE, &onu_speed_if_cmd );
    install_element( ONU_GT813_NODE, &onu_encap_if_cmd );
    install_element( ONU_GT813_NODE, &onu_port_learn_cmd );
    install_element( ONU_GT813_NODE, &onu_port_desc_cmd );
    install_element( ONU_GT813_NODE, &no_onu_port_desc_cmd );
    install_element( ONU_GT813_NODE, &onu_shutdown_if_cmd );
    install_element( ONU_GT813_NODE, &onu_Line_Rate_ingress_Cmd );
    install_element( ONU_GT813_NODE, &onu_Line_Rate_egress_Cmd );
    install_element( ONU_GT813_NODE, &onu_storm_control_cmd);
    install_element( ONU_GT813_NODE, &onu_undo_storm_control_cmd );
    install_element( ONU_GT813_NODE, &onu_no_shutdown_if_cmd );
    install_element( ONU_GT813_NODE, &show_onu_eth_cmd );
    install_element( ONU_GT813_NODE, &clear_onu_eth_statistic_cmd );
    install_element( ONU_GT813_NODE, &onu_GT813_Isolate_Cmd );
    install_element( ONU_GT813_NODE, &onu_GT813_unIsolate_Cmd );
	install_element( ONU_GT813_NODE, &onu_GT813_show_Isolate_Cmd );/*add by shixh20090507*/
    install_element( ONU_GT813_NODE, &onu_Cable_Test_Port_Cmd );
    /*install_element( ONU_GT813_NODE, &onu_port_trunk_access_mode_if_cmd );*/
    /*install_element( ONU_GT813_NODE, &config_onu_sys_contact_cmd );
    install_element( ONU_GT813_NODE, &show_onu_sys_contact_cmd );
    install_element( ONU_GT813_NODE, &config_onu_sys_location_cmd );
    install_element( ONU_GT813_NODE, &show_onu_sys_location_cmd ); */   
    install_element( ONU_GT813_NODE, &cl_onu_reboot_cmd );
    install_element( ONU_GT813_NODE, &onu_mac_show_cmd );
    /*install_element( ONU_GT813_NODE, &Show_onu_interface_brief_CMD );*/
    install_element( ONU_GT813_NODE, &onu_normal_vlan_vlanmode_CMD );
    install_element( ONU_GT813_NODE, &show_onu_normal_vlan_vlanmode_CMD);
    install_element( ONU_GT813_NODE, &onu_create_vlan_interface_cmd );
    install_element( ONU_GT813_NODE, &onu_create_vlan_multicast_interface_cmd );
    install_element( ONU_GT813_NODE, &onu_create_vlan_portstream_interface_cmd );
    install_element( ONU_GT813_NODE, &onu_vlan_del_add_port_cmd );
    install_element( ONU_GT813_NODE, &show_onu_vlan_cmd );
    install_element( ONU_GT813_NODE, &onu_IpAddr_if_CMD );
    install_element( ONU_GT813_NODE, &onu_NoIpAddr_if_CMD );
	/* del by suxq 2007-08-28
    install_element( ONU_GT813_NODE, &onu_vlan_multicaststate_set_cmd );
    */
    install_element( ONU_GT813_NODE, &no_onu_vlan_interface_cmd );
    install_element( ONU_GT813_NODE, &ConfigOnuCstBridgeEnalbe_CMD );
    install_element( ONU_GT813_NODE, &ConfigOnuCstBridgeParameterForward_CMD );
    install_element( ONU_GT813_NODE, &ConfigOnuCstBridgeParameterHelloTime_CMD );
    install_element( ONU_GT813_NODE, &ConfigOnuCstBridgeParameterMaxage_CMD );
    install_element( ONU_GT813_NODE, &ConfigOnuCstBridgeParameterPriority_CMD );
    install_element( ONU_GT813_NODE, &ConfigOnuCstPortParameterEdge_CMD );
    install_element( ONU_GT813_NODE, &ConfigOnuCstPortParameterNoneSTP_CMD );
    install_element( ONU_GT813_NODE, &ConfigOnuCstPortParameterP2P_CMD );
    install_element( ONU_GT813_NODE, &ConfigOnuCstPortParameterPathCost_CMD );
    install_element( ONU_GT813_NODE, &ConfigOnuCstPortParameterPriority_CMD );
    install_element( ONU_GT813_NODE, &ConfigOnuCstDefPortParameter_CMD );
    install_element( ONU_GT813_NODE, &ConfigOnuCstDefBridgeParameter_CMD );
    install_element( ONU_GT813_NODE, &ShowOnuCstBridge_CMD );
    install_element( ONU_GT813_NODE, &ShowOnuCstBridgePort_CMD );
    install_element( ONU_GT813_NODE, &config_save_onu_config_file_cmd );
    install_element( ONU_GT813_NODE, &show_onu_sys_version_cmd );
    install_element( ONU_GT813_NODE, &erase_onu_config_file_cmd );
    install_element( ONU_GT813_NODE, &onu_cl_login_auth_cmd);
    install_element( ONU_GT813_NODE, &ShowONUMacByPort_CMD );
    install_element( ONU_GT813_NODE, &ShowONUMac_byMac_vlan_CMD );
    /*install_element( DEBUG_HIDDEN_NODE, &show_onu_infor_cmd );*/
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_enable_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_Authenable_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_SetSGroup_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_SetSMem_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_Show_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_dels_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_ShowCount_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_ShowHostTimeout_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_GroupLife_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_ShowGroupLife_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_FilterCfg_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_ShowFilterCfg_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_FastLeaveCfg_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_ShowFastleaveCfg_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_HosttimeoutCfg_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_SetRouterVlan_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_DelRouterVlan_Cmd );
    install_element( ONU_GT813_NODE, &onu_Igmp_snoop_ShowRouterVlan_Cmd );
    install_element( ONU_GT813_NODE, &ShowOnuMacCount_Cmd );
    install_element( ONU_GT813_NODE, &create_onu_forward_entry_cmd );
    install_element( ONU_GT813_NODE, &delete_onu_forward_entry_func_cmd );
    install_element( ONU_GT813_NODE, &onu_mac_set_agingtime_cmd );
    install_element( ONU_GT813_NODE, &onu_mac_no_agingtime_cmd );
    install_element( ONU_GT813_NODE, &onu_mac_show_agingtime_cmd );
    install_element( ONU_GT813_NODE, &show_onu_image_num_cmd );
    install_element( ONU_GT813_NODE, &active_onu_image_cmd );   
    install_element( ONU_GT813_NODE, &onu_qos_forward_entry_cmd);
    install_element( ONU_GT813_NODE, &onu_qos_port_based_set_cmd );
    install_element( ONU_GT813_NODE, &onu_qos_8021p_set_cmd);
    install_element( ONU_GT813_NODE, &onu_qos_dscp_tos_select_cmd);
    install_element( ONU_GT813_NODE, &onu_qos_port_dscp_tos_set_cmd);
    install_element( ONU_GT813_NODE, &onu_qos_dscp_map_set_cmd);
    install_element( ONU_GT813_NODE, &onu_qos_tos_map_set_cmd);
    install_element( ONU_GT813_NODE, &show_onu_qos_by_port_cmd);
    install_element( ONU_GT813_NODE, &show_onu_qos_dscp_tos_map_cmd);
    install_element( ONU_GT813_NODE, &show_onu_qos_dscp_tos_select_cmd);
#if 0    
    install_element( ONU_GT813_NODE, &ONU_Qos_Port_PriQueue_G_Cmd );
    install_element( ONU_GT813_NODE, &ONU_Qos_Port_WrrQueue_G_Cmd );
    install_element( ONU_GT813_NODE, &ONU_Qos_Port_Show_CosqueWeight_G_Cmd );
    install_element( ONU_GT813_NODE, &ONU_Qos_Interface_Set_UserPri_Cmd );
    install_element( ONU_GT813_NODE, &ONU_Qos_Interface_Cancel_UserPri_Cmd );
    install_element( ONU_GT813_NODE, &Onu_Qos_Interface_Show_Qos_Cmd);
#endif    
    /*install_element( ONU_GT813_NODE, & );
    install_element( ONU_GT813_NODE, & );
    install_element( ONU_GT813_NODE, & );
    install_element( ONU_GT813_NODE, & );
    install_element( ONU_GT813_NODE, & );
    install_element( ONU_GT813_NODE, & );
    install_element( ONU_GT813_NODE, & );
    install_element( ONU_GT813_NODE, & );
    install_element( ONU_GT813_NODE, & );
    install_element( ONU_GT813_NODE, & );
    install_element( ONU_GT813_NODE, & );*/
    /*install_element( ONU_GT813_NODE, &test_onu_port_list_cmd );*/
#endif

    install_element( CONFIG_NODE, &cl_onu_cmdbroadcast_cmd );
    install_element( CONFIG_NODE, &cl_onu_cmdbroadcast_show_cmd );
    install_element( DEBUG_HIDDEN_NODE, &cl_onu_cmdbroadcast_cmd );
    install_element( DEBUG_HIDDEN_NODE, &cl_onu_cmdbroadcast_show_cmd );

#if (PRODUCT_EPON3_ONU == RPU_YES)  
    if ( cmd_rugular_register( &stCMD_Onu_Port_List_Check ) == no_match )
    {
	ASSERT( 0 );
    }

    install_element( ONU_DEVICE_NODE, &onu_auto_enable_if_cmd);
    install_element( ONU_DEVICE_NODE, &onu_duplex_enable_if_cmd );
    install_element( ONU_DEVICE_NODE, &onu_flowcontrol_enable_if_cmd );
    install_element( ONU_DEVICE_NODE, &onu_speed_if_cmd );
    install_element( ONU_DEVICE_NODE, &onu_encap_if_cmd );
    install_element( ONU_DEVICE_NODE, &onu_port_learn_cmd );
    install_element( ONU_DEVICE_NODE, &onu_port_desc_cmd );
    install_element( ONU_DEVICE_NODE, &no_onu_port_desc_cmd );
    install_element( ONU_DEVICE_NODE, &onu_shutdown_if_cmd );
    install_element( ONU_DEVICE_NODE, &onu_no_shutdown_if_cmd );
    install_element( ONU_DEVICE_NODE, &onu_Line_Rate_ingress_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Line_Rate_egress_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_storm_control_cmd);
    install_element( ONU_DEVICE_NODE, &onu_undo_storm_control_cmd );
    install_element( ONU_DEVICE_NODE, &show_onu_eth_cmd );
    install_element( ONU_DEVICE_NODE, &clear_onu_eth_statistic_cmd );
    install_element( ONU_DEVICE_NODE, &onu_GT813_Isolate_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_GT813_unIsolate_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_GT813_show_Isolate_Cmd );/*add by shixh20090507*/
    install_element( ONU_DEVICE_NODE, &onu_Cable_Test_Port_Cmd );
    /*install_element( ONU_DEVICE_NODE, &onu_port_trunk_access_mode_if_cmd );*/
    /*install_element( ONU_DEVICE_NODE, &config_onu_sys_contact_cmd );
    install_element( ONU_DEVICE_NODE, &show_onu_sys_contact_cmd );
    install_element( ONU_DEVICE_NODE, &config_onu_sys_location_cmd );
    install_element( ONU_DEVICE_NODE, &show_onu_sys_location_cmd ); */   
    install_element( ONU_DEVICE_NODE, &cl_onu_reboot_cmd );
    install_element( CONFIG_NODE, &cl_onu_cmdbroadcast_cmd );
    install_element( CONFIG_NODE, &cl_onu_cmdbroadcast_show_cmd );
    install_element( DEBUG_HIDDEN_NODE, &cl_onu_cmdbroadcast_cmd );
    install_element( DEBUG_HIDDEN_NODE, &cl_onu_cmdbroadcast_show_cmd );
    install_element( ONU_DEVICE_NODE, &onu_mac_show_cmd );
    /*install_element( ONU_DEVICE_NODE, &Show_onu_interface_brief_CMD );*/
    install_element( ONU_DEVICE_NODE, &onu_create_vlan_interface_cmd );
    install_element( ONU_DEVICE_NODE, &onu_normal_vlan_vlanmode_CMD );
    install_element( ONU_DEVICE_NODE, &show_onu_normal_vlan_vlanmode_CMD);
    install_element( ONU_DEVICE_NODE, &onu_create_vlan_multicast_interface_cmd );
    install_element( ONU_DEVICE_NODE, &onu_create_vlan_portstream_interface_cmd );
    install_element( ONU_DEVICE_NODE, &onu_vlan_del_add_port_cmd );
    install_element( ONU_DEVICE_NODE, &show_onu_vlan_cmd );
    install_element( ONU_DEVICE_NODE, &onu_IpAddr_if_CMD );
    install_element( ONU_DEVICE_NODE, &onu_NoIpAddr_if_CMD );
	/* del by suxq 2007-08-28
    install_element( ONU_DEVICE_NODE, &onu_vlan_multicaststate_set_cmd );
    */
    install_element( ONU_DEVICE_NODE, &no_onu_vlan_interface_cmd );
    install_element( ONU_DEVICE_NODE, &ConfigOnuCstBridgeEnalbe_CMD );
    install_element( ONU_DEVICE_NODE, &ConfigOnuCstBridgeParameterForward_CMD );
    install_element( ONU_DEVICE_NODE, &ConfigOnuCstBridgeParameterHelloTime_CMD );
    install_element( ONU_DEVICE_NODE, &ConfigOnuCstBridgeParameterMaxage_CMD );
    install_element( ONU_DEVICE_NODE, &ConfigOnuCstBridgeParameterPriority_CMD );
    install_element( ONU_DEVICE_NODE, &ConfigOnuCstPortParameterEdge_CMD );
    install_element( ONU_DEVICE_NODE, &ConfigOnuCstPortParameterNoneSTP_CMD );
    install_element( ONU_DEVICE_NODE, &ConfigOnuCstPortParameterP2P_CMD );
    install_element( ONU_DEVICE_NODE, &ConfigOnuCstPortParameterPathCost_CMD );
    install_element( ONU_DEVICE_NODE, &ConfigOnuCstPortParameterPriority_CMD );
    install_element( ONU_DEVICE_NODE, &ConfigOnuCstDefPortParameter_CMD );
    install_element( ONU_DEVICE_NODE, &ConfigOnuCstDefBridgeParameter_CMD );
    install_element( ONU_DEVICE_NODE, &ShowOnuCstBridge_CMD );
    install_element( ONU_DEVICE_NODE, &ShowOnuCstBridgePort_CMD );
    install_element( ONU_DEVICE_NODE, &config_save_onu_config_file_cmd );
    install_element( ONU_DEVICE_NODE, &show_onu_sys_version_cmd );
    install_element( CONFIG_NODE, &show_onu813_sys_version_cmd );
    install_element( VIEW_NODE, &show_onu813_sys_version_cmd );
    install_element( ONU_DEVICE_NODE, &erase_onu_config_file_cmd );
    install_element( ONU_DEVICE_NODE, &onu_cl_login_auth_cmd);
    install_element( ONU_DEVICE_NODE, &ShowONUMacByPort_CMD );
    install_element( ONU_DEVICE_NODE, &ShowONUMac_byMac_vlan_CMD );
    /*install_element( DEBUG_HIDDEN_NODE, &show_onu_infor_cmd );
    install_element( ONU_DEVICE_NODE, &show_onu_infor_cmd );*/
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_enable_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_Authenable_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_SetSGroup_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_SetSMem_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_Show_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_dels_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_ShowCount_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_ShowHostTimeout_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_GroupLife_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_ShowGroupLife_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_FilterCfg_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_ShowFilterCfg_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_FastLeaveCfg_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_ShowFastleaveCfg_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_HosttimeoutCfg_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_SetRouterVlan_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_DelRouterVlan_Cmd );
    install_element( ONU_DEVICE_NODE, &onu_Igmp_snoop_ShowRouterVlan_Cmd );
    install_element( ONU_DEVICE_NODE, &ShowOnuMacCount_Cmd );
    install_element( ONU_DEVICE_NODE, &create_onu_forward_entry_cmd );
    install_element( ONU_DEVICE_NODE, &delete_onu_forward_entry_func_cmd );
    install_element( ONU_DEVICE_NODE, &onu_mac_set_agingtime_cmd );
    install_element( ONU_DEVICE_NODE, &onu_mac_no_agingtime_cmd );
    install_element( ONU_DEVICE_NODE, &onu_mac_show_agingtime_cmd );
    install_element( ONU_DEVICE_NODE, &show_onu_image_num_cmd );
    install_element( ONU_DEVICE_NODE, &active_onu_image_cmd );
    install_element( ONU_DEVICE_NODE, &onu_qos_forward_entry_cmd);
    install_element( ONU_DEVICE_NODE, &onu_qos_port_based_set_cmd);
    install_element( ONU_DEVICE_NODE, &onu_qos_8021p_set_cmd);
    install_element( ONU_DEVICE_NODE, &onu_qos_dscp_tos_select_cmd);
    install_element( ONU_DEVICE_NODE, &onu_qos_port_dscp_tos_set_cmd);
    install_element( ONU_DEVICE_NODE, &onu_qos_dscp_map_set_cmd);
    install_element( ONU_DEVICE_NODE, &onu_qos_tos_map_set_cmd);
    install_element( ONU_DEVICE_NODE, &show_onu_qos_by_port_cmd);
    install_element( ONU_DEVICE_NODE, &show_onu_qos_dscp_tos_map_cmd);
    install_element( ONU_DEVICE_NODE, &show_onu_qos_dscp_tos_select_cmd);
#if 0    
    install_element( ONU_DEVICE_NODE, &ONU_Qos_Port_PriQueue_G_Cmd );
    install_element( ONU_DEVICE_NODE, &ONU_Qos_Port_WrrQueue_G_Cmd );
    install_element( ONU_DEVICE_NODE, &ONU_Qos_Port_Show_CosqueWeight_G_Cmd );
    install_element( ONU_DEVICE_NODE, &ONU_Qos_Interface_Set_UserPri_Cmd );
    install_element( ONU_DEVICE_NODE, &ONU_Qos_Interface_Cancel_UserPri_Cmd );
    install_element( ONU_DEVICE_NODE, &Onu_Qos_Interface_Show_Qos_Cmd);
#endif
    /*install_element( ONU_DEVICE_NODE, & );
    install_element( ONU_DEVICE_NODE, & );
    install_element( ONU_DEVICE_NODE, & );
    install_element( ONU_DEVICE_NODE, & );
    install_element( ONU_DEVICE_NODE, & );
    install_element( ONU_DEVICE_NODE, & );
    install_element( ONU_DEVICE_NODE, & );*/
    /*install_element( ONU_DEVICE_NODE, &test_onu_port_list_cmd );*/
#endif
    return VOS_OK;
}


#ifdef __cplusplus
}
#endif


