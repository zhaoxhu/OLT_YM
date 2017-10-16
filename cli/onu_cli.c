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
*	  modified wutw 18 October
*		增加对ONU节点的范围限制,有效PON 卡号为4-7,其中4如果为主控卡,则同样无效,
*		增加对进入ONU节点时对Port的运行状态进行判断,同时修改no命令为undo
*		增加对onu name， location，description配置时输入字符串长度的判断
*	  modified wutw 10 November
*		删除onu节点下的encrypt [up-down|down|disable]命令,onu的加密方向已由pon
*		节点下配置,为全局变量,固在此节点下不应被配置
*		2006/11/23 : pclint 检查，修改部分代码		
*	  modified by wutw 28 November
*		修改部分cli命令：显示onu文件升级的两条命令
*		将software auto-update [eanble|disable]拆分为software auto-update与undo 
*		software auto-update两条命令
*	  modified by wutw 2006/12/15
*		增加onu p2p转发规则设置命令行
*	  modified by wutw 2007/01/23
*		增加onu pon端口,sni端口的统计命令
*		增加从config节点与pon节点使用onu名字进入onu节点的命令
*      modified by chenfj  2007/04/26 
*         问题单#4299: P2P第二个命令有待改进,将广播包转发功能隐含到配置p2p链接的命令中去， 而未知单播包的转发则可以另外配置
*      
*     added by chenfj 2007-5-23 , 
*       增加ONU 支持的最大MAC 设置及 FEC 设置
*     added by chenfj 2007-6-8 
*        增加ONU 数据流IP/PORT过滤
*    added by chenfj 2007-6-12 
*     增加ONU 数据流vlan id 过滤
*    added by chenfj 2007-6-15 
*       增加ONU 数据流ETHER TYPE /IP PROTOCOL 过滤
*    modified by chenfj 2007-6-18
*      urTracker 问题单:4692  ,  徐州现场,OLTpon8/1下ONU无法修改名字
*	 modified by chenfj 2007-8-20
*		urTracker问题单#4819:
*		对于基于EtherType过滤数据【源MAC地址后的类型域】，
*		建议命令参数改成16进制
*
*    added by chenfj 2007-9-13
*	    在对ONU加密时，判断ONU与OLT类型是否匹配
*	modified by chenfj 2007-9-25
*		ONU加密，密钥更新时间在show run中保存时保存了错误的
*		字符串"encrypt-keytime"（不是配置更新密钥时间的命令），
*		导致配置数据未能恢复
*
*   added by chenfj 2007-9-24
*		问题单#5396:
*		在进入已设置了保护切换，且当前状态为passive 的PON节点或
*		ONU节点时增加提示信息，对用户配置passive端口时作出限制提示
*
*   modified by chenfj 2008-7-11
*         增加GFA6100 产品支持; 
*
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
/*#include "../EPONV2R1/onulpb/Onuloop.h"*/
/*added by wutw at 18 October*/
#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include  "V2R1_product.h"
#include "../superset/platform/sys/main/Sys_main.h" 
#include "../superset/cpi/typesdb/Typesdb_module.h"
#include "../superset/platform/manage/Mn_set.h"
/*added by wutw 2006/11/10*/
#include "olt_cli.h"

#include "lib_gwEponOnuMib.h"
#include "onu/onuOamUpd.h"
#include "onu/onuConfMgt.h"

extern unsigned int   inet_atonl_v4( char* ipaddr );
extern void  inet_ntoa_v4( unsigned int  lip, char* ipaddr ) ;
extern LONG Onustats_CommandInstall();  

/*added by wutw at 13 september*/
#define	CLI_EPON_DEFAULTLLID			0
#define	CLI_EPON_ONUUP				ONU_OPER_STATUS_UP	/*1*/
#define	CLI_EPON_ONUDOWN				ONU_OPER_STATUS_DOWN	/*2*/
#define	CLI_EPON_ONUUPDATE_EN		ONU_SW_UPDATE_ENABLE  /*1*/
#define	CLI_EPON_ONUUPDATE_DIS		ONU_SW_UPDATE_DISABLE  /*2*/
#define	CLI_EPON_STATS15MIN			1
#define	CLI_EPON_STATS24HOUR			2
#define	CLI_EPON_ONUMIN				0
#define	CLI_EPON_ONUMAX				(MAXONUPERPON-1)	/*63*/
#define	CLI_EPON_PONMIN				0
#define	CLI_EPON_PONMAX				(MAXPON-1)	/*19*/
#define	CLI_ONU_NAME_MAXLEN			MAXDEVICENAMELEN  /*256*/
#define	CLI_EPON_ENCRYPT_ALL			PON_ENCRYPTION_DIRECTION_ALL  /*3*/
#define	CLI_EPON_ENCRYPT_DOWN		PON_ENCRYPTION_DIRECTION_DOWN /*2*/
#define	CLI_EPON_ENCRYPT_NONE		PON_ENCRYPTION_PURE /*1*/
#define	CLI_EPON_LOOP_INTERNAL		1
#define	CLI_EPON_LOOP_EXTERNAL		2
#define	CLI_EPON_LOOP_START			2
#define	CLI_EPON_LOOP_STOP			3
#define	CLI_EPON_LOOP_ONULOOP_EXIST	(-4)
#define	CLI_EPON_ONU_PONDEV			1
#define	CLI_EPON_ONU_ONUDEV			2
/*added by wutw at 13 september*/
#define	CLI_EPON_CARDINSERT			CARDINSERT /*1*/

#define CLI_ONU_NOT_EXIST		V2R1_ONU_NOT_EXIST  /*3*/

#ifndef CLI_EPON_ALARM_DISABLE
#define  CLI_EPON_ALARM_DISABLE		V2R1_DISABLE  /*2*/
#define  CLI_EPON_ALARM_ENABLE		V2R1_ENABLE  /*1*/
#endif
#define CTC_ONU_TELNET_UP   0
#define CTC_ONU_TELNET_DOWN 1
/*typedef enum{
	PONPORT_UP=1,
	PONPORT_DOWN,
	PONPORT_UNKNOWN,
	PONPORT_LOOP,
	PONPORT_UPDATE,
	PONPORT_INIT,
	PONPORT_DEL
}CLI_ONU_PONPORT_OPER;*/




/*#undef CLI_EPON_DEBUG*/
#define CLI_SLOT_SYSCLE    for(slotId = PONCARD_FIRST;slotId <= PONCARD_LAST; slotId ++)
#define CLI_PORT_SYSCLE	for(port = 1; port <= PONPORTPERCARD; port ++)
#define CLI_ONU_SYSCLE	for(onuId = 0; onuId < MAXONUPERPON; onuId++)
extern LONG ONUDAYA_CommandInstall(enum node_type  node);
extern LONG ONUDAYA_CommandInstallTogether(enum node_type  node);

extern LONG PON_ParseSlotPortOnu( CHAR * szName, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern short int GetPonPortIdxBySlot( short int slot, short int port );
extern int ShowOnuEncryptInfoByVty( short int PonPortIdx, short int OnuIdx ,struct vty *vty);
extern ULONG IFM_PON_CREATE_INDEX( ULONG ulSlot, ULONG ulPort , ULONG ulOnuId, ULONG ulOnuFeId);
extern LONG PON_GetSlotPortOnu( ULONG ulIfIndex, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern int ShowOnuDeviceInfoByVty(short int PonPortIdx, short int OnuIdx, struct vty *vty );
extern STATUS HisStatsOnu15MModified(short int ponId, short int onuId, unsigned int bucketNum, BOOL flag15M);
extern STATUS HisStatsOnu24HModified(short int ponId, short int onuId, unsigned int bucketNum, BOOL flag24H);
extern STATUS CliHisStatsOnu15MinDataVty(unsigned short ponId,  unsigned short onuId, unsigned int bucket_num, struct vty* vty);
extern STATUS CliHisStatsOnu24HourDataVty(unsigned short ponId, unsigned short onuId, unsigned int bucket_num, struct vty* vty);
extern STATUS HisStats15MinMaxRecordSet(unsigned short value);
extern STATUS HisStats24HoursMaxRecordSet(unsigned short value);
extern STATUS HisStats24HoursMaxRecordGet(unsigned int *pValue);
extern STATUS HisStats15MinMaxRecordGet(unsigned int *pValue);
extern STATUS HisStatsOnu15MinRawClear(short int ponId, short int onuId);
extern STATUS HisStatsOnu24HourRawClear(short int ponId, short int onuId);
extern int HisStatsOnuStatsStart  (short int ponId, short int onuId, BOOL Done);
extern STATUS CliHisStatsONUCtrlGet(short int ponId, short int onuId, struct vty* vty) ;
extern int GetOnuEncrypt( short int PonPortIdx, short int OnuIdx, unsigned int *cryptionDirection );
extern STATUS CliHisStatsONUStatusGet(short int ponId,short int onuId,unsigned int *pStatus15m,unsigned int *pStatus24h);
extern int SetOnuPeerToPeerForward( short int PonPortIdx, short int OnuIdx , int address_not_found, int broadcast );
extern STATUS CliRealTimeOnuStatsPon( short int ponId, short int onuId, struct vty* vty  );
extern STATUS CliRealTimeOnuStatsCNI( short int ponId, short int onuId, struct vty* vty  );
extern STATUS CliRealTimeOltDownlinkBer( short int ponId, short int onuId, struct vty* vty  );
extern int checkVtyPonIsValid( struct vty *vty, ULONG ulSlot, ULONG ulPort );

extern STATUS	getOnuAuthEnable( ULONG slot, ULONG port, ULONG *enable );

extern LONG onu_Qos_CommandInstall(enum node_type  node );
/*extern LONG event_show_run( struct vty * vty );*/
extern PON_onu_address_table_record_t  MAC_Address_Table[8192];
extern LONG ctcOnu_alarm_showrun( struct vty * vty );

CHAR * g_aszOltLocalOnuCliString[  ] = {
"clear statistic-history ",/*0*/
"deregister",
"device description ",
"device location ",
"device name ",
"encrypt update-key-time ",/*5*/
"encrypt ",
"exit",
"help",
"list ",
"list",/*10*/
"logout",
"onu [src-ip|dst-ip] filter",/*45*/
"onu dst-ip filter",
"onu ethertype filter",
"fec-mode",
"onu iptype filter",
"onu max-mac ",
"onu src-mac filter ",
"onu tcp filter",
"onu udp filter",
"onu vlanid filter"
"p2p forward address-not-found ",/*15*/
#if( EPON_MODULE_ONU_LOOP == EPON_MODULE_YES )
"pon-loop source ",
#endif
"quit",
"remote reset",
"show command-history",
"show device information",/*20*/
"show encrypt information",
"show fdbentry mac",
"show fdbentry mac counter",
/*"show olt-downlink ber",*/
"show onu",
/*
"show onu fec",
"show onu max-mac",
"show onu src-mac filter",
"show onu src-ip filter",
"show onu dst-ip filter",
"show onu src-udp filter",
"show onu src-tcp filter",
"show onu vlanid filter",
"show onu ethtype filter",
"show onu iptype filter",
*/
"show p2p forward rule",
#if( EPON_MODULE_ONU_LOOP == EPON_MODULE_YES )
"show pon-loop parameter",
"show pon-loop result",/*30*/
#endif
"show software update",
"show statistic-history",
"show statistic ",
"software update",
"statistic-history",/*35*/
"undo encrypt",
"undo onu",
/*
"undo onu [src-ip|dst-ip] filter",
"undo onu dst-ip filter",
"undo onu ethertype filter",
"undo onu fec",
"undo onu iptype filter",
"undo onu max-mac",
"undo onu src-mac filter",
"undo onu tcp filter",
"undo onu udp filter",
"undo onu vlanid filter"
*/
#if( EPON_MODULE_ONU_LOOP == EPON_MODULE_YES )
"undo pon-loop",/*40*/
#endif
"undo software update",
"undo statistic-history",
"update onu",
/*50*/
"deactivate onu",
"undo deactivate onu",
/*  */
#ifdef  CNC_2007_10_TEST 
"show interface qvlan",
"interface qvlan",
"undo interface qvlan",
"interface qvlan_port",
"undo interface qvlan_port",
#endif

/*"reboot",*/
"show running-config",/*55*/
"pty",
"grosadvdebug",
"show fec-mode",
"show fec-ability",
"convert onu file",/*60*/
"update onu file",
NULL,
NULL/*63*/
};


/*强制使用透传命令行管理的ONU类型列表,V2R1_ONU_GT811..V2R1_ONU_MAX占bit位的方式来表示*/
char g_onu_relay_cli_mode[V2R1_ONU_MAX/8];
#define CTC_ONU_MAX_PTY_CONNECTION  8
#define CTC_PTY_CONNECTION_IS_UP 1
#define CTC_PTY_CONNECTION_IS_DOWN 0
#define CTC_PTY_CONNECTION_FULL_ERROR 2
typedef struct
{
    ULONG Onu_DevId;
    ULONG OLT_IpAddr;
}g_ctconu_pty_limit_t;/*参与建立telnet 连接时的Onu 设备索引和Olt此时的IP 地址*/
g_ctconu_pty_limit_t g_ctconu_pty_limit[CTC_ONU_MAX_PTY_CONNECTION];
void init_pty_connection_status()
{
    VOS_MemZero(g_ctconu_pty_limit, sizeof(g_ctconu_pty_limit_t)*CTC_ONU_MAX_PTY_CONNECTION);
}
int set_pty_connection_status(ULONG pon_id, ULONG onu_id, ULONG ipaddr)/*onu_id 下标从1开始*/
{
    int slot = GetCardIdxByPonChip((short int)pon_id);
    int port = GetPonPortByPonChip((short int)pon_id);
    int loop = 0, ret = VOS_OK;
    ULONG devidx = 0;
    
    for(loop=0;loop<CTC_ONU_MAX_PTY_CONNECTION;loop++)
    {
        if(g_ctconu_pty_limit[loop].Onu_DevId == 0)
            break;
    }
    if(loop<CTC_ONU_MAX_PTY_CONNECTION)
    {
        devidx = MAKEDEVID(slot, port, onu_id);
        g_ctconu_pty_limit[loop].Onu_DevId = devidx;
        g_ctconu_pty_limit[loop].OLT_IpAddr = ipaddr;
    }
    else
        ret = VOS_ERROR;
    return ret;
}
int clr_pty_connection_status(ULONG pon_id, ULONG onu_id)
{
    int slot = GetCardIdxByPonChip((short int)pon_id);
    int port = GetPonPortByPonChip((short int)pon_id);
    int loop = 0;
    for(loop=0;loop<CTC_ONU_MAX_PTY_CONNECTION;loop++)
    {
        if(g_ctconu_pty_limit[loop].Onu_DevId)
        {
            int g_slot = 0, g_port = 0, g_onuid = 0;
            
            g_slot = GET_PONSLOT(g_ctconu_pty_limit[loop].Onu_DevId);
            g_port = GET_PONPORT(g_ctconu_pty_limit[loop].Onu_DevId);
            g_onuid = GET_ONUID(g_ctconu_pty_limit[loop].Onu_DevId);
            if(g_slot == slot && g_port == port && g_onuid == onu_id)
            {
                VOS_MemZero(&g_ctconu_pty_limit[loop], sizeof(g_ctconu_pty_limit_t));
                break;
            }
        }
    }
    return VOS_OK;
}
int ctc_pty_is_connected(ULONG slot, ULONG port, ULONG onuid)
{
    int ret = CTC_PTY_CONNECTION_IS_DOWN;
    int loop = 0;
    ULONG devidx = 0;
    for(loop=0;loop<CTC_ONU_MAX_PTY_CONNECTION;loop++)
    {
        if(g_ctconu_pty_limit[loop].Onu_DevId)
        {
            int g_slot = 0, g_port = 0, g_onuid = 0;
            
            g_slot = GET_PONSLOT(g_ctconu_pty_limit[loop].Onu_DevId);
            g_port = GET_PONPORT(g_ctconu_pty_limit[loop].Onu_DevId);
            g_onuid = GET_ONUID(g_ctconu_pty_limit[loop].Onu_DevId);
            if(g_slot == slot && g_port == port && g_onuid == onuid)
                break;
        }
    }
    
    if(loop < CTC_ONU_MAX_PTY_CONNECTION)
    {
        ret = CTC_PTY_CONNECTION_IS_UP;
    }
    else
    {
        for(loop=0;loop<CTC_ONU_MAX_PTY_CONNECTION;loop++)
        {
            if(g_ctconu_pty_limit[loop].Onu_DevId == 0)
                break;
        }
        if(loop>=CTC_ONU_MAX_PTY_CONNECTION)
            ret = CTC_PTY_CONNECTION_FULL_ERROR;
    }
    return ret;
}
typedef struct{
    int type;
    char typestring[16];
}clirelay_onu_t;
static clirelay_onu_t g_cliRelay_type_string[] ={
        {V2R1_ONU_GT813, "GT813"},
        {V2R1_ONU_GT813_B, "GT813_B"},
        {V2R1_ONU_GT865,"GT865"},
        {V2R1_ONU_GT861, "GT861"}
};

void initOnuCliRelayMode()
{

    VOS_MemSet(g_onu_relay_cli_mode, 0, sizeof(g_onu_relay_cli_mode));
    setOnuCliRelayMode(V2R1_ONU_GT813);
    setOnuCliRelayMode(V2R1_ONU_GT813_B);
    setOnuCliRelayMode(V2R1_ONU_GT865);
    setOnuCliRelayMode(V2R1_ONU_GT861);

}

static int sfun_getCliRelayOnuType(const char *typestring)
{
    int i;
    int num = sizeof(g_cliRelay_type_string)/sizeof(char *);

    for(i=0; i<num; i++)
    {
        if(!VOS_StriCmp(g_cliRelay_type_string[i].typestring, typestring))
            return g_cliRelay_type_string[i].type;
    }

    if(i==num)
        return V2R1_ONU_MAX;
}

static char * sfun_getCliRelayOnuTypeString(int type)
{
    int i;
    int num = sizeof(g_cliRelay_type_string)/sizeof(char *);

    for(i=0; i<num; i++)
    {
        if(type == g_cliRelay_type_string[i].type)
            return g_cliRelay_type_string[i].typestring;
    }

    if(i==num)
        return NULL;
}

char *getCliRelayOnuTypeString(int type)
{
    return sfun_getCliRelayOnuTypeString(type);
}

int setOnuCliRelayMode(int onutype)
{
    if(onutype >= V2R1_ONU_GT811 && onutype < V2R1_ONU_MAX)
    {
        int offset = onutype/8;
        int bitnum = onutype & 7;
        g_onu_relay_cli_mode[offset] |= (1<<bitnum);
    }
    else
        return VOS_ERROR;
}

int clrOnuCliRelayMode(int onutype)
{
    if(onutype >= V2R1_ONU_GT811 && onutype < V2R1_ONU_MAX)
    {
        int offset = onutype/8;
        int bitnum = onutype & 7;
        g_onu_relay_cli_mode[offset] &= ~(1<<bitnum);
    }
    else
        return VOS_ERROR;
}

int isOnuCliRelayMode(int onutype)
{
    if(onutype >= V2R1_ONU_GT811 && onutype < V2R1_ONU_MAX)
    {
        int offset = onutype/8;
        int bitnum = onutype & 7;
        if(g_onu_relay_cli_mode[offset] &(1<<bitnum))
            return TRUE;
        else
            return FALSE;
    }
    else
        return FALSE;
}

DEFUN(onu_cli_relay_mode,
        onu_cli_relay_mode_cmd,
        "onu cli-relay [GT813|GT865|GT813_B|GT861|ALL]",
        "onu config\n"
        "cli command relay to PON slot to execute\n"
        "GT813\n"
        "GT865\n"
        "GT813_B\n"
        "GT861\n"
        "all onu type surpported\n")
{
    int ret = CMD_WARNING;

    if(VOS_StriCmp(argv[0], "ALL") == 0)
    {
        setOnuCliRelayMode(V2R1_ONU_GT813);
        setOnuCliRelayMode(V2R1_ONU_GT813_B);
        setOnuCliRelayMode(V2R1_ONU_GT865);
        setOnuCliRelayMode(V2R1_ONU_GT861);
        ret = CMD_SUCCESS;
    }
    else
    {
        int type = sfun_getCliRelayOnuType(argv[0]);
        if(type != V2R1_ONU_MAX)
        {
            setOnuCliRelayMode(type);
            ret = CMD_SUCCESS;
        }
    }

    return ret;

}

DEFUN(undo_onu_cli_relay_mode,
        undo_onu_cli_relay_mode_cmd,
        "undo onu cli-relay [GT813|GT865|GT813_B|GT861|ALL]",
        "undo config\n"
        "onu config\n"
        "cli command relay to PON slot to execute\n"
        "GT813\n"
        "GT865\n"
        "GT813_B\n"
        "GT861\n"
        "all onu type surpported\n")
{
    int ret = CMD_WARNING;

    if(VOS_StriCmp(argv[0], "ALL") == 0)
    {
        clrOnuCliRelayMode(V2R1_ONU_GT813);
        clrOnuCliRelayMode(V2R1_ONU_GT813_B);
        clrOnuCliRelayMode(V2R1_ONU_GT865);
        clrOnuCliRelayMode(V2R1_ONU_GT861);
        ret = CMD_SUCCESS;
    }
    else
    {
        int type = sfun_getCliRelayOnuType(argv[0]);
        if(type != V2R1_ONU_MAX)
        {
            clrOnuCliRelayMode(type);
            ret = CMD_SUCCESS;
        }
    }

    return ret;

}

DEFUN(show_onu_cli_relay_mode,
        show_onu_cli_relay_mode_cmd,
        "show onu cli-relay",
        SHOW_STR
        "onu config\n"
        "cli command relay to PON slot to execute\n"
        )
{

    int i = V2R1_ONU_GT811;

    for(i=V2R1_ONU_GT811; i<V2R1_ONU_MAX; i++)
    {
        if(isOnuCliRelayMode(i))
        {
            char *typestring = sfun_getCliRelayOnuTypeString(i);
            if(typestring)
                vty_out(vty, "\r\n%s\r\n", typestring);
        }
    }

    return CMD_SUCCESS;

}

int cliCheckOnuMacValid_onuNode( short int PonPortIdx, short int OnuIdx )
{
	int ret = VOS_OK;
	ONU_MGMT_SEM_TAKE;
	if( ThisIsInvalidMacAddr(OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].DeviceInfo.MacAddr) )
		ret = V2R1_ONU_NOT_EXIST;
	ONU_MGMT_SEM_GIVE;
	return ret;
}

static int into_onu_node( struct vty *vty, int prev_prev_node, ULONG ulSlot, ULONG ulPort, ULONG ulOnuid )
{
	CHAR 	ifName[IFM_NAME_SIZE + 1];
	ULONG   ulIFIndex = 0;
	CHAR    prompt[64] = { 0 };
	LONG	lRet;
	INT16 phyPonId = 0;
	int onuType = 0;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if( phyPonId == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	

	VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
	VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%d/%d/%d", "onu", ulSlot, ulPort, ulOnuid);

	ulIFIndex = IFM_PON_CREATE_INDEX( ulSlot, ulPort, ulOnuid, 0);
	if ( ulIFIndex == 0 )
	{
		vty_out( vty, "%% Can not find interface %s.\r\n", ifName );
		return CMD_WARNING;
	}

	if(cli_onuDumpConfigForOnuNodeVty(vty, ulSlot, ulPort, ulOnuid) != VOS_OK)
	{
	    vty_out(vty, "onu config profiles can't be prepared!\r\n");
	    return CMD_WARNING;
	}

	vty->index = ( VOID * ) ulIFIndex;
	vty->prev_node = vty->node;
	if( prev_prev_node )
		vty->prev_prev_node = vty->node;
	
	lRet = GetOnuType( phyPonId, ulOnuid-1, &onuType );
	if( lRet != VOS_OK )
		return CMD_WARNING;
      
	if( onuType >= V2R1_ONU_MAX ) /* 需考虑到ONU 未曾注册过的情况*/
		return CMD_WARNING;
	
	if(( onuType == V2R1_ONU_GT813 )||( onuType == V2R1_ONU_GT865)/*||( onuType == V2R1_ONU_GT866)*/ /*||
		(onuType == V2R1_ONU_GT813_B)*/ /*|| (onuType == V2R1_ONU_GT862) || (onuType == V2R1_ONU_GT863)*/ )
	{
	    if(isOnuCliRelayMode(onuType))
	        vty->node = ONU_GT813_NODE;
	    else
	        vty->node = ONU_NODE;
	}
	else if(( onuType == V2R1_ONU_GT831) ||( onuType == V2R1_ONU_GT831_CATV) || ( onuType == V2R1_ONU_GT831_A) ||( onuType == V2R1_ONU_GT831_A_CATV))
		vty->node = ONU_GT821_GT831_NODE;
	/*
	else if( onuType == V2R1_ONU_GT865)
		vty->node = ONU_GT865_NODE;
	*/
	else if( onuType == V2R1_ONU_CTC)
		vty->node = ONU_CTC_NODE;
	else if( onuType == V2R1_ONU_CMC)
		vty->node = ONU_CMC_NODE;
	else if( (onuType == V2R1_ONU_GT861) || (onuType == V2R1_ONU_GT892) )
	        vty->node = ONU_GT861_NODE;
	else if( (onuType == V2R1_ONU_GT831_B) || (onuType == V2R1_ONU_GT831_B_CATV) || (onuType == V2R1_ONU_GT835) )
		vty->node = ONU_GT831B_NODE;
    else if( onuType == V2R1_ONU_GPON )
        vty->node = ONU_GPON_NODE;
    /*modified by luh 2013-5-17,  通用onu节点设置为ONU_NODE, 保留节点下命令行不完全*/
#if 0	
	else if( (onuType == V2R1_ONU_GT811) || (onuType == V2R1_ONU_GT812) || (onuType == V2R1_ONU_GT810) || (onuType == V2R1_ONU_GT816)
	   		|| (onuType == V2R1_ONU_GT811_A) || (onuType == V2R1_ONU_GT812_A) || (onuType == V2R1_ONU_GT815) || (onuType == V2R1_ONU_GT812_B)
	   		|| (onuType == V2R1_ONU_GT811_B) || (onuType == V2R1_ONU_GT851) || (onuType == V2R1_ONU_GT815_B) || (onuType == V2R1_ONU_GT871)
	   		||(onuType == V2R1_ONU_GT873)||(onuType == V2R1_ONU_GT871_R)||(onuType == V2R1_ONU_GT872) )
    		vty->node = ONU_NODE;
	 else
	 	vty->node = NEW_ONU_TYPE_CLI_NODE;
#else
	 else
		vty->node = ONU_NODE;
#endif
    if(vty->node == ONU_GPON_NODE || SYS_MODULE_IS_8000_GPON(ulSlot))
    {
        VOS_StrCpy( prompt, "%s(gpon-" );    
    }
    else
    {
        VOS_StrCpy( prompt, "%s(epon-" );
    }	
	VOS_StrCat( prompt, ifName );
	VOS_StrCat( prompt, ")#" );
	vty_set_prompt( vty, prompt );   

	/* added by chenfj 2007-9-24
		问题单#5396:
		在进入已设置了保护切换，且当前状态为passive 的PON节点或
		ONU节点时增加提示信息，对用户配置passive端口时作出限制提示
	*/
	if(( PonPortSwapEnableQuery( phyPonId ) == V2R1_PON_PORT_SWAP_ENABLE ) &&
		( PonPortHotStatusQuery( phyPonId ) == V2R1_PON_PORT_SWAP_PASSIVE )&&
		( PON_SWAPMODE_ISOLT( GetPonPortHotSwapMode( phyPonId ))))/*for onu swap by jinhl@2013-02-22*/
	{
		int ret;
		unsigned int PartnerSlot, PartnerPort;
		ret = PonPortAutoProtectPortQuery(ulSlot, ulPort, &PartnerSlot, &PartnerPort );
		if( ret == ROK )
			vty_out(vty,"\r\nNOTE:pon%d/%d is PASSIVE now;if want to do config,Please goto CLI node onu%d/%d/%d\r\n", ulSlot, ulPort, PartnerSlot, PartnerPort, ulOnuid );
	}

#ifndef ONUID_MAP
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        int ponid = GetPonPortIdxBySlot(ulSlot, ulPort);

        if(ponid != -1)
        {
            int entry = ponid*MAXONUPERPON+ulOnuid-1;
            if(!OnuMgmtTable[entry].configFileName[0])
            {
                char sz[80]="";
                int type = 0,share = 0;

                if(getOnuConfFromHashBucket(sz))
                {
                    ONU_MGMT_SEM_TAKE
                    {
                        VOS_StrCpy(OnuMgmtTable[entry].configFileName, sz);
                    }
                    ONU_MGMT_SEM_GIVE
                }
                else
                {
                    ONUConfigData_t *pd = VOS_Malloc(sizeof(ONUConfigData_t), MODULE_RPU_ONU);
                    if(pd)
                    {
                        VOS_MemZero(pd, sizeof(ONUConfigData_t));
                        VOS_StrCpy(pd->confname, sz);
                        pd->share = share;
                        OnuProfile_Action_ByCode(OnuProfile_Add, 0, 0, 0, sz, NULL, pd);
                        ONU_MGMT_SEM_TAKE
                        {
                            VOS_StrCpy(OnuMgmtTable[entry].configFileName, sz);
                        }
                        ONU_MGMT_SEM_GIVE

                        if(SYS_MODULE_IS_PON(ulSlot))       
                            OnuProfile_Action_ByCode(OnuProfile_Add_SyncBroadcast, 0, 0, 0, sz, NULL, pd);

                    }

                }

            }

        }
    }
#else
    /*cli_onuDumpConfigForOnuNodeVty(vty, ulSlot, ulPort, ulOnuid);*/
#endif
        
	return CMD_SUCCESS;
}

DEFUN  (
    config_auto_delete_onu,
    config_auto_delete_onu_cmd,
    "onu isautodelete {[enable|disable]}",
    "Select an onu to config\n"
    "isautodelete\n"
    "enable or disable"
    )
{
	LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;	
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	int OnuEntry;
	ulIfIndex = ( ULONG ) ( vty->index ) ;
	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    	return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	userOnuId = onuId - 1;
	OnuEntry = phyPonId*MAXONUPERPON + userOnuId;
	ONU_MGMT_SEM_TAKE
	if(argc == 1)
	{	
		
		if(VOS_StrCmp(argv[0],"enable") == 0)
		{
			OnuMgmtTable[OnuEntry].IsAutoDelete = 0;
		}
		else
		{
        	OnuMgmtTable[OnuEntry].IsAutoDelete = 1;
		}
	}
	else
	{
		if(0 == OnuMgmtTable[OnuEntry].IsAutoDelete)
			sys_console_printf(" \r\nonu autodelete is enable! \r\n");
		else
			sys_console_printf(" \r\nonu autodelete is disable! \r\n");
	}
	ONU_MGMT_SEM_GIVE
	return CMD_SUCCESS;
}

DEFUN  (
    into_epon_onu_node,
    into_epon_onu_node_cmd,
    "onu <slot/port/onuid>",
    "Select an onu to config\n"
    "Specify onu interface's onuid\n"
    )
{
	LONG	lRet;
	ULONG   ulSlot, ulPort, ulOnuid;

	int ponid;

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	
	if( PonCardSlotPortCheckByVty(ulSlot, ulPort, vty) != ROK )
		return(CMD_WARNING );
	if(SlotCardMayBePonBoardByVty(ulSlot, vty)  != ROK )
		return(CMD_WARNING);

	if ((ulOnuid<(CLI_EPON_ONUMIN+1)) || (ulOnuid>(CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onu %d/%d/%d error\r\n",ulSlot, ulPort, ulOnuid );
		return CMD_WARNING;	
	}

	return into_onu_node( vty, 0, ulSlot, ulPort, ulOnuid );
}


DEFUN  (
    pon_into_onu_node,
    pon_into_onu_node_cmd,
    "onu <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">",
    "Select an onu to config\n"
    "Specify onu interface's onuid\n"
    )
{
	ULONG   ulSlot, ulPort, ulOnuid;
	LONG   lRet = PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnuid );
    	if( lRet != VOS_OK )
    		return CMD_WARNING;
		
	ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] ) ;
    CHECK_CMD_ONU_RANGE(vty, ulOnuid-1);
	
   	return into_onu_node( vty, 1, ulSlot, ulPort, ulOnuid );
}

DEFUN  (
    pon_into_onu_name_node,
    pon_into_onu_name_node_cmd,
    "onu <name>",
    "Select an onu to config\n"
    "Specify onu interface's onu name\n"
    )
{
	ULONG   ulSlot, ulPort, ulOnuid;
	LONG   lRet;
	LONG   len;
	short int onuId = 0;
	
	lRet = PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnuid );
    	if( lRet != VOS_OK )
    		return CMD_WARNING;

	len = VOS_StrLen(argv[0]);
	lRet = GetOnuDeviceIdxByName_OnePon( argv[0], len, (short  int)ulSlot, (short int)ulPort, &onuId );
	if ( CLI_ONU_NOT_EXIST == lRet )
	{
		vty_out( vty, "  %% Onu name is not exist\r\n");
		return CMD_WARNING;
	}
	ulOnuid = (ULONG)onuId+1;
   	return into_onu_node( vty, 1, ulSlot, ulPort, ulOnuid );
}

DEFUN(
        onu_profile_info_show,
        onu_profile_info_show_cmd,
/*
        "show onu profile list {pon [all|<slot/port>] [all|<onuid_list>]}*1",
        SHOW_STR
        "onu configuration\n"
        "profile infor\n"
        "file list\n"
        "show pon port file list\n"
        "all pon port\n"
        "specified pon port\n"
        "all onu\n"
        "onu list\n"
*/
        "show onu-profile list {counter}*1",
        SHOW_STR
        "profile infor\n"
        "file list\n"
        "file counter\n"
        )
{

    if (VOS_StrRChr(vty->buf, 'c'))
    {
        ULONG counter = getOnuConfFileCounter();
        vty_out(vty, "\r\nonu config file counter:\t%d\r\n\r\n", counter);
    }
    else
        show_onu_conf_list(vty);

    return CMD_SUCCESS;
}

DEFUN(onu_profile_association_show,
        onu_profile_association_show_cmd,
        "show onu-profile pon [all|<slot/port>] [all|<onuid_list>]",
        SHOW_STR
        "profile infor\n"
        "show pon port file list\n"
        "all pon port\n"
        "specified pon port\n"
        "all onu\n"
        "onu list\n"
        )
{
    int slot, port, ponid;

    if(VOS_StrCmp(argv[0], "all"))
    {
        VOS_Sscanf(argv[0], "%d/%d", &slot, &port);
        /*69的PON板仅支持跟自己相关的映射关系的显示2012-10-31*/
        if(!SYS_LOCAL_MODULE_WORKMODE_ISMASTER && slot != SYS_LOCAL_MODULE_SLOTNO)
        {
            vty_out(vty, " PonCard only supports what related to itself!!\r\n");
            return CMD_WARNING;
        }
        /*2012-12-7，6700 pon口输入异常导致访问越界，问题单16490*/
    	if(PonCardSlotPortCheckWhenRunningByVty(slot, port, vty) != ROK)
    		return(CMD_WARNING);
        
        ponid = GetPonPortIdxBySlot(slot, port);

        if(ponid != -1 && (SlotCardIsPonBoard(slot) == VOS_OK)&& port <= /*CARD_MAX_PON_PORTNUM*/PONPORTPERCARD)
        {

            if(!VOS_StrCmp(argv[1], "all"))
                return show_onu_conf_list_by_pon(vty, ponid);
            else
            {
                int onu;
                int MaxOnu = GetMaxOnuByPonPort(ponid)&0xff;

				show_onu_conf_Dataheader(vty,slot, port);
                BEGIN_PARSE_ONUID_LIST_TO_ONUID(argv[1], onu)
                {
                    /*不再多余显示，根据实际注册的onu数目进行显示*/
                    if(onu > MaxOnu)
                        break;
                        
                    show_onu_conf_list_by_onu(vty, ponid, onu-1);
                }
                END_PARSE_ONUID_LIST_TO_ONUID()
            }
        }
        else
        {
            vty_out(vty, "PON port parameter error!\r\n");
            return CMD_WARNING;
        }
    }
    else
    {
        for(slot = 1; slot <= SYS_CHASSIS_SLOTNUM; slot++)
        {
            /*69的PON板仅支持跟自己相关的映射关系的显示2012-10-31*/            
            if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER && slot!= SYS_LOCAL_MODULE_SLOTNO)
            {
                continue;
            }
            
            if(SlotCardIsPonBoard(slot) == VOS_OK)
            {
                for(port = 1; port <= PONPORTPERCARD; port++)
                {
                    ponid = GetPonPortIdxBySlot(slot, port);
                    if(ponid != -1 /*&& PonPortIsWorking(ponid)*/)
                    {
                        /*if(AllOnuCounter(ponid))
                           show_onu_conf_list_by_pon(vty, ponid);*/

                        if(!VOS_StrCmp(argv[1], "all"))
                            show_onu_conf_list_by_pon(vty, ponid);
                        else
                        {
                            int onu;

                            show_onu_conf_Dataheader(vty,slot, port);
                            BEGIN_PARSE_ONUID_LIST_TO_ONUID(argv[1], onu)
                            {
                                /*
                                ONU_CONF_SEM_TAKE
                                {
                                    char * name = getOnuConfNamePtrByPonId(ponid, onu-1);
                                    if(name)
                                        vty_out(vty, "%-8d\t%-20s\t%-20s\r\n", onu, name, onuConfIsShared(ponid, onu-1)?"yes":"no");
                                    ONU_CONF_SEM_GIVE
                                }
                                */
                                show_onu_conf_list_by_onu(vty, ponid, onu-1);
                            }
                            END_PARSE_ONUID_LIST_TO_ONUID()
                        }
                    }
                }
            }
        }
    }

    return CMD_SUCCESS;
}

DEFUN(onu_profile_association_show_oneonu,
        onu_profile_association_show_oneonu_cmd,
        "show onu-profile onu <slot/port/onuid>",
        SHOW_STR
        "profile infor\n"
        "show pon port file list\n"
        "onu operation\n"
        "onu id\n"
        )
{
    int ret = CMD_SUCCESS;
	LONG	lRet = 0;
	ULONG   ulSlot = 0, ulPort = 0, ulOnuid = 0;
	int ponid = 0;
    
	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
    
    /*防止数组下标越界，2014-04-02*/
	if(SlotCardMayBePonBoardByVty(ulSlot, vty)  != ROK )
		return(CMD_WARNING);
        
    /*69的PON板仅支持跟自己相关的映射关系的显示2012-10-31*/
    if(!SYS_LOCAL_MODULE_WORKMODE_ISMASTER && ulSlot != SYS_LOCAL_MODULE_SLOTNO)
    {
        vty_out(vty, " PonCard only supports what related to itself!!\r\n");
        return CMD_WARNING;
    }

    ponid = GetPonPortIdxBySlot(ulSlot, ulPort);
    if(ponid != -1 && ( SlotCardIsPonBoard(ulSlot) == VOS_OK )&& ulPort <= PONPORTPERCARD/*CARD_MAX_PON_PORTNUM*/ &&
            ulOnuid >=1 && ulOnuid <= MAXONUPERPON)
    {
        show_onu_conf_Dataheader(vty,ulSlot, ulPort);
        show_onu_conf_list_by_onu(vty, ponid, ulOnuid-1);
    }
    else
    {
        vty_out(vty, "onu parameters error!\r\n");
        ret = CMD_WARNING;
    }

    return ret;
}

DEFUN(
        onu_profile_info_show_byname,
        onu_profile_info_show_byname_cmd,
        "show onu-profile name <name>",
        SHOW_STR
        /*"onu configuration\n"*/
        "profile infor\n"
        /*"file list\n"*/
        "show association by name\n"
        "specified file name\n"
        )
{

    int ret = CMD_WARNING;

    if(!isOnuConfExist(argv[0]))
        vty_out(vty, "file not exist!\r\n");
    else
    {
        ONU_CONF_SEM_TAKE
        {
            int banner = 1;
            ONUConfigData_t *pd = getOnuConfFromHashBucket(argv[0]);
            if(pd){
                int ponid = 0, portidx = 0;
                for(ponid = 0; ponid<MAXPON; ponid++)
                {
                    char sz[512] = "";
                    int onuid = 0;
                    int slot = GetCardIdxByPonChip(ponid);
                    int port = GetPonPortByPonChip(ponid);
                    int globle_ponid = GetGlobalPonPortIdxBySlot(slot, port);
                    int len = getBitMaskString(&pd->onulist[globle_ponid][0], MAXONUPERPONNOLIMIT/8+1, MAXONUPERPONNOLIMIT, sz, 512);
                    if(len)
                    {
                        /*int slot = GetGlobalCardIdxByPonChip(ponid);
                        int port = GetPonPortByPonChip(ponid);*/

                      if(!SYS_LOCAL_MODULE_WORKMODE_ISMASTER/*SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER*/)
                      {
                          if(slot != SYS_LOCAL_MODULE_SLOTNO)
                              continue;
                          else
                          {
                              portidx = GetPonPortIdxBySlot(slot, port);
                          }
                      }
                      else
                          portidx = ponid;

                      if(banner)
                      {
							vty_out(vty, "\r\n");
                          	/*
                          				vty_out(vty, "slot/pon\t\tonulist\r\n");
                         				 */
                         	vty_out(vty, "slot/pon    onuidx    mac addr/SN          name\r\n");
                          	vty_out(vty, "\r\n");
                        	banner = 0;
                      }
/*
                      vty_out(vty, "%4d/%-3d\t\t%s\r\n", slot, port, sz);
*/
                      BEGIN_PARSE_ONUID_LIST_TO_ONUID(sz, onuid)
                      {
                   		int MaxOnu = GetMaxOnuByPonPort(portidx)&0xff;
                        int entry = portidx*MAXONUPERPON+onuid-1;
                        UCHAR *mac = NULL;
                          
                        if(entry >= MAXONU || onuid > /*MAXONUPERPON*/MaxOnu )
                        	break;

                        mac = OnuMgmtTable[entry].DeviceInfo.MacAddr;
                        ONU_MGMT_SEM_TAKE
                        {
                        if(SYS_MODULE_IS_GPON(slot))
						{
							vty_out(vty, "%4d/%-3d    %-6d    %16s    %s\r\n", slot, port, onuid,
					        	OnuMgmtTable[entry].DeviceInfo.DeviceSerial_No,OnuMgmtTable[entry].DeviceInfo.DeviceName);
						}
						else
						{
					    	vty_out(vty, "%4d/%-3d    %-6d    %02x%02x.%02x%02x.%02x%02x    %s\r\n", slot, port, onuid,
                            	mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], OnuMgmtTable[entry].DeviceInfo.DeviceName);
						}
                              
                        }
                        ONU_MGMT_SEM_GIVE
                      }
                      END_PARSE_ONUID_LIST_TO_ONUID()
                    }
                }
                vty_out(vty, "\r\n");
                ret = CMD_SUCCESS;
            }
        }
        ONU_CONF_SEM_GIVE
    }

    return ret;
}

extern int g_SystemLoadConfComplete;

DEFUN  (
    onu_profile_create,
    onu_profile_create_cmd,
    /*"onu profile <name>  {[share|private]}*1",*/
    "config onu-profile <name>",
    /*"onu configuration\n"*/
    CONFIG_STR
    "profile config\n"
    "Specify onu profile name\n"
    /*
    "the file is shared\n"
    "the file is private\n"*/
    )
{

    LONG    lRet = VOS_OK;

    int share = 1;
    char szPrompt[256]="onu-profile";

    ONUConfigData_t *pdata = NULL;
    ONUConfigData_t *pnew = NULL;

    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return CMD_WARNING;
    
    if(VOS_StrLen(argv[0]) > ONU_CONFIG_NAME_LEN_CLI)
    {
        vty_out(vty, "profile name is too long, maximum length is %d charactors\r\n", ONU_CONFIG_NAME_LEN_CLI);
        return CMD_WARNING;
    }

    if(!isOnuConfExist(argv[0]) && isOnuConfFileTableFull())
    {
        vty_out(vty, "no resource to create a new config file!\r\n");
        return CMD_WARNING;
    }

    /* comment by wangxiaoyu 2011-10-18
    if(argc == 2)
        share = VOS_StriCmp(argv[1], "share")?0:1;
*/
    ONU_CONF_SEM_TAKE
    {

        pdata = getOnuConfFromHashBucket(argv[0]);
		/*第一次创建*/
        if(!pdata)
        {

            /*保留的配置文件名，只有在系统启动时允许创建，否则禁止创建*/
            if(g_SystemLoadConfComplete == _VOS_SYSTEM_RUNNING_ && VOS_StrLen(argv[0]) >= 3 && !VOS_StrnCmp(argv[0], "onu", 3) && VOS_StrCmp(argv[0], DEFAULT_ONU_CONF))
            {
                ONU_CONF_SEM_GIVE
                vty_out(vty, "the config file names leading with onu has been reserved by system!\r\n");
                return CMD_WARNING;
            }

            /*pdata = VOS_Malloc(sizeof(ONUConfigData_t), MODULE_RPU_ONU);*/
/*        pdata = onuconf_malloc(ONU_CONF_MEM_DATA_ID);*/
#if 0
        	pdata = OnuConfigProfile_init();
#else
        	pdata = openOnuConfigFile(argv[0], ONU_CONF_OPEN_FLAG_WRITE);
#endif
            if (pdata)
            {
                int chip, type;

                if(!VOS_StrnCmp(argv[0], "onu", 3) && VOS_StrCmp(argv[0], DEFAULT_ONU_CONF) ) /*onu开头的文件为私有文件，其它可创建的均为公有文件*/
                    share = 0;

            /*VOS_MemSet(pdata, 0, sizeof(ONUConfigData_t));*/
#if 0
                VOS_StrCpy(pdata->confname, argv[0]);
#endif

                if(!VOS_StrCmp(argv[0], DEFAULT_ONU_CONF))
                    VOS_MemSet(pdata->onulist, 0xff, sizeof(pdata->onulist));

                pdata->share = share;
#if 0
                if (setOnuConfToHashBucket(argv[0], pdata))
                {
                    /*VOS_Free(pdata);*/
                    onuconf_free(pdata, ONU_CONF_MEM_DATA_ID);
                    lRet = VOS_ERROR;
                }
                else
#endif
                {
                    vty->onuconfptr = pdata;
                    vty->index = IFM_ONU_PROFILE_CREATE_INDEX(argv[0]);
                    vty->orig_profileptr = NULL;
                    /*
                                    setOnuConfVlanModeByPtr(getOnuConfHashIndex(argv[0]), pdata, ONU_CONF_VLAN_MODE_TRUNK);*/
                }
            }
            else
            {
                vty_out(vty, "create onu profile %s fail!\n", argv[0]);
                lRet = VOS_ERROR;
            }
        }
        else /*已存在的文件进入修改模式*/
        {
            /*
             * 已存在同名配置文件，允许修改
             */

            /*区分参数来决定同名文件存在时的处理方式:
             * 创建私有文件时，如果有同名文件存在，则放弃;
             * 否则认为是要修改已存在的文件  wangxy 2011-06-30*/
            /*
            if(onuconfHaveAssociatedOnu(argv[0]))
            {
                vty_out(vty, "profile have associations, please undo association first!\r\n");
                return CMD_WARNING;
            }
            */

            if(!VOS_StrCmp(pdata->confname, DEFAULT_ONU_CONF))
            {
                vty_out(vty, "default onu config file can't be modified!\r\n");
                lRet = VOS_ERROR;
            }
            else
            {
#if 0
                pnew = onuconf_malloc(ONU_CONF_MEM_DATA_ID);
                if(pnew)
                {
                    char filename[80] = "";
                    VOS_Sprintf(filename, "*auto%s", pdata->confname);
                    VOS_MemCpy(pnew, pdata, sizeof(ONUConfigData_t));
                    VOS_StrCpy(pnew->confname, filename);

                    if(setOnuConfToHashBucket(pnew->confname, pnew))
                    {
                        onuconf_free(pnew, ONU_CONF_MEM_DATA_ID);
                        lRet = VOS_ERROR;
                    }
                    else
                    {
                        vty->index = IFM_ONU_PROFILE_CREATE_INDEX(pnew->confname);
                        vty->onuconfptr = pnew;
                        vty->orig_profileptr = pdata;
                    }
                }
                else
                    lRet = VOS_ERROR;
#else
                char filename[80] = "";
				VOS_Sprintf(filename, "*auto%s", argv[0]);
				if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
				{
					if(CheckOnuConfRestoreQueueByName(argv[0]) == 0 || CheckOnuConfRestoreQueueByName(filename) == 0)
					{
						sys_console_printf("\r\nThe profile is in use!");						
					    ONU_CONF_SEM_GIVE;
						return CMD_SUCCESS;
					}
				}
                pdata = openOnuConfigFile(argv[0], ONU_CONF_OPEN_FLAG_WRITE);
                pnew = openOnuConfigFile(filename, ONU_CONF_OPEN_FLAG_WRITE);
                if(pdata && pnew)
                {
                    VOS_MemCpy(pnew, pdata, sizeof(ONUConfigData_t));
                    VOS_StrCpy(pnew->confname, filename);
                    vty->index = IFM_ONU_PROFILE_CREATE_INDEX(pnew->confname);
                    vty->onuconfptr = pnew;
                    vty->orig_profileptr = pdata;
                }
                else
                {
                    vty_out(vty, "the profile maybe reserved by other task!\r\n");
                    if(pnew)
                        OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, filename, NULL, NULL);                                    
                    lRet = VOS_ERROR;
                }
#endif
            }
        }
    }
    ONU_CONF_SEM_GIVE

    if(lRet == VOS_ERROR)
        return CMD_WARNING;
    else
    {
        vty->node = ONU_PROFILE_NODE;

        VOS_Sprintf(szPrompt, "%s(%s)#", szPrompt, argv[0]);
        vty_set_prompt(vty, szPrompt);
    }

    return CMD_SUCCESS;

}
extern int parseOnuPrivateConfigByOnuId(ULONG slot, ULONG port, ULONG onuid, ONUConfigData_t *pOld, ONUConfigData_t *pNew);

DEFUN  (
    onu_profile_create_private,
    onu_profile_create_private_cmd,
    "private onu-profile <slot/port/onuid>",
    "Private onu config\n"
    "Onu profile config\n"
    "Onu index\n"
    )
{

    int             lRet = VOS_OK;
    int             isExist = 0;
    int             share = 1;
    char            sz[80];
    ULONG           ulSlot = 0, ulPort = 0, ulOnuid = 0;
    short int       PonPortIdx = 0;
    char            *name = NULL;    
    ONUConfigData_t *pdata = NULL;
    ONUConfigData_t *pnew = NULL;
    
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return CMD_WARNING;

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if( SlotCardMayBePonBoardByVty(ulSlot, vty)  != ROK )
		return(CMD_WARNING);
		
	if( PonCardSlotPortCheckByVty(ulSlot, ulPort, vty) != ROK )
		return(CMD_WARNING );

    VOS_MemZero(sz, 80);
    VOS_Sprintf(sz, "onu%d/%d/%d", ulSlot, ulPort, ulOnuid);
    
    isExist = isOnuConfExist(sz);
    if(!isExist && isOnuConfFileTableFull())
    {
        vty_out(vty, "no resource to create a new config file!\r\n");
        return CMD_WARNING;
    }

    PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);    
	name = getOnuConfNamePtrByPonId(PonPortIdx, ulOnuid-1);/*缓存当前关联的配置文件*/    
    if(!onuConfIsSharedByName(name))
    {
        vty_out(vty, " The specific onu is already associated with private profile!\r\n");
        return CMD_WARNING;
    }
       
    ONU_CONF_SEM_TAKE
    {

        pdata = getOnuConfFromHashBucket(name);
        if(!pdata)
        {
            vty_out(vty, " The profile that onu %s associated is not exist!\t\n", argv[0]);
        }
        else /*已存在的文件进入修改模式*/
        {
            LONG standBySlotNo = device_standby_master_slotno_get();
            pnew = openOnuConfigFile(sz, ONU_CONF_OPEN_FLAG_WRITE);
            if(pnew)
            {
                /*VOS_MemCpy(pnew, pdata, sizeof(ONUConfigData_t));*/
                if(parseOnuPrivateConfigByOnuId(ulSlot, ulPort, ulOnuid, pdata, pnew) == VOS_OK)
                {
                    if(OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, ulSlot, 0, 0, sz, NULL, pnew) == VOS_OK)
                    {   
                        if(standBySlotNo)
                            OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, standBySlotNo, 0, 0, sz, NULL, pnew);
                        OnuProfile_Action_ByCode(OnuMap_Update, 0, PonPortIdx, ulOnuid-1, sz, NULL, NULL);/*直接更新master 板卡上的关联关系*/
                        OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, ulSlot, PonPortIdx, 0, NULL, NULL, NULL);  /*直接更新slave 板卡上整个PON 口的关联关系*/      
                        if(standBySlotNo)
                            OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, standBySlotNo, PonPortIdx, 0, NULL, NULL, NULL);  /*直接更新standby 板卡上整个PON 口的关联关系*/                              
                    }
                }
                closeOnuConfigFile(sz);            
            }
            else
            {
                vty_out(vty, " Can not created the specific profile!\r\n");
            }
        }
    }
    ONU_CONF_SEM_GIVE

    return CMD_SUCCESS;

}

DEFUN  (
    onu_profile_edit,
    onu_profile_edit_cmd,
    "onu profile edit <name>",
    "onu configuration\n"
    "profile config\n"
    "edit\n"
    "Specify onu profile name\n"
    )
{

    LONG    lRet;

    int share = 1;
    char szPrompt[256]="onu-profile";

    ONUConfigData_t *pdata = NULL;
    ONUConfigData_t *pnew = NULL;

    if(VOS_StrLen(argv[0]) > ONU_CONFIG_NAME_LEN_CLI)
    {
        vty_out(vty, "profile name is too long, maximum length is %d charactors\r\n", ONU_CONFIG_NAME_LEN_CLI);
        return CMD_WARNING;
    }

#if 0
    if(onuconfHaveAssociatedOnu(argv[0]))
    {
        vty_out(vty, "profile have associations, please undo association first!\r\n");
        return CMD_WARNING;
    }
#endif
    pdata = getOnuConfFromHashBucket(argv[0]);

    if(!pdata)
    {
        vty_out(vty, "file <%s> doesn't exist!\r\n", argv[0]);
        return CMD_WARNING;
    }

    pnew = onuconf_malloc(ONU_CONF_MEM_DATA_ID);
    if(pnew)
    {
        VOS_MemCpy(pnew, pdata, sizeof(ONUConfigData_t));
        VOS_StrCpy(pnew->confname, "*autoprofile");
    }
    else
        return CMD_WARNING;

    vty->node = ONU_PROFILE_NODE;
    vty->index = IFM_ONU_PROFILE_CREATE_INDEX(argv[0]);
#if 0
    vty->onuconfptr = pdata;
#else
    vty->orig_profileptr = pdata;
    vty->onuconfptr = pnew;
#endif

    VOS_Sprintf(szPrompt, "%s(%s)#", szPrompt, argv[0]);
    vty_set_prompt(vty, szPrompt);

    return CMD_SUCCESS;

}


DEFUN  (
    onu_profile_delete,
    onu_profile_delete_cmd,
    "delete onu-profile name <name>",
    "delete onu profile\n"
    /*"onu configuration\n"**/
    "profile config\n"
    "specific name\n"
    "Specify onu profile name\n"
    )
{
    short int ulSlot = 0;
    short int PonPortIdx = 0;
    short int ulDelFlag = 0;
    int       status = 0;
    
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)    
        return CMD_WARNING;
    
    if(!isOnuConfExist(argv[0]))
    {
        vty_out(vty, "the file does not exist!\r\n");
        return CMD_WARNING;
    }
    else if(!VOS_StriCmp(argv[0], DEFAULT_ONU_CONF))/*added by wangxy 2011-07-05 can't delete default config data*/
    {
        vty_out(vty, "default onu config file can't be deleted!\r\n");
        return CMD_WARNING;
    }
    else if(onuconfHaveAssociatedOnu(argv[0]))
    {
        vty_out(vty, "this confile file has associated with some onus, please undo the association first!\r\n");
        return CMD_WARNING;
    }

    if(/*SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER*/0)
    {
        status = GetOnuConfRestoreQueueStatus(argv[0]);       
        if(status)
        {
            ulDelFlag = 1;
        }           
    }
    else
    {        
        for(ulSlot=PONCARD_FIRST; ulSlot<=PONCARD_LAST; ulSlot++)
        {        
             if(!SYS_MODULE_SLOT_ISHAVECPU(ulSlot) || !SYS_MODULE_IS_READY(ulSlot))
                continue;
            
            PonPortIdx = GetPonPortIdxBySlot(ulSlot, 1);
            if(PonPortIdx == VOS_ERROR)        
                continue;
            if(OLT_GetOnuConfDelStatus(PonPortIdx, argv[0], &status) == VOS_OK)
            {
                if(status)
                {
                    ulDelFlag = 1;
                    break;
                }
            }
            
        }
    }
    /*Pon板或67、61主控板上，该文件已经不再被使用时才允许删除*/
    if(!ulDelFlag)
    {
        OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, argv[0], NULL, NULL);                                    
        OnuProfile_Action_ByCode(OnuProfile_Delete_SyncBroadcast, 0, 0, 0, argv[0], NULL, NULL);
    }
    else
    {
        if(ulSlot)
            vty_out(vty, " The profile ( %s ) is used by SLOT %d to restore! Please wait for a while!\r\n", argv[0], ulSlot);
        else
            vty_out(vty, " The profile ( %s ) is used to restore! Please wait for a while!\r\n", argv[0]);
           
    }
    return CMD_SUCCESS;
}

DEFUN  (
    onu_profile_delete_disused_conf,
    onu_profile_delete_disused_conf_cmd,
    "delete onu-profile all-disused",
    "delete onu profile\n"
    /*"onu configuration\n"**/
    "profile config\n"
    "all disused files\n"
    )
{
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return CMD_WARNING;
    
    deleteAllDisusedConfigFile();
    return CMD_SUCCESS;
}


DEFUN(
	onu_profile_copy,
	onu_profile_copy_cmd,
	"onu-profile copy file <soure> <destination>",
	"onu configuration\n"
	"copy configuration\n"
	"copy profile configuration\n"
    "source profile name\n"
    "destination profile name\n"
	)
{

	int ret = CMD_WARNING;
	
	ONUConfigData_t *pd = NULL, *po = NULL;

	char szname[ONU_CONFIG_NAME_LEN] = "";
    
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return CMD_WARNING;
    
	if( !isOnuConfExist(argv[0]))
	{
		vty_out(vty, "file %s not exist!\r\n", argv[0]);
		return ret;
	}

	if(!isOnuConfExist(argv[1]))
	{
		vty_out(vty, "file %s not exist!\r\n", argv[1]);
		return ret;
	}
    if(VOS_StriCmp(argv[1], DEFAULT_ONU_CONF)==0)
    {
		vty_out(vty, "destination profile name can not be %s!\r\n", argv[1]);
        return ret;
    }
	ONU_CONF_SEM_TAKE
	{
		ONUConfigData_t * pauto = NULL;
		
		po = getOnuConfFromHashBucket(argv[0]);
		pd = getOnuConfFromHashBucket(argv[1]);

		if(po && pd)
		{
			VOS_Sprintf(szname, "*auto%s", pd->confname);
			pauto = onuconf_malloc(ONU_CONF_MEM_DATA_ID);

			if(pauto)
			{
				VOS_StrCpy(pauto->confname, szname);
				VOS_MemCpy(pauto->onulist, pd->onulist, sizeof(pd->onulist));
                OnuProfile_Action_ByCode(OnuProfile_Add, 0, 0, 0, szname, NULL, pauto);
#if 0                
				if(VOS_OK == onuconfCopy(pauto, po))
				{
					if(SYS_LOCAL_MODULE_ISMASTERACTIVE && !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
					{
						/*sendOnuConfSyndBroadcastReqMsg(szname);*/
                        OnuProfile_Action_ByCode(OnuProfile_Add_SyncBroadcast, 0, 0, 0, szname, NULL, NULL); 
					}
					if(onu_profile_rename(szname, pd->confname) != VOS_OK)
                        OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, szname, NULL, NULL);                                    
					ret = CMD_SUCCESS;
				}
				else
                    OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, szname, NULL, NULL);                                    
#else
				if(VOS_OK == onuconfCopy(pauto, po))
				{
                    OnuProfile_Action_ByCode(OnuProfile_Add_SyncBroadcast, 0, 0, 0, szname, NULL, NULL);
                    OnuProfile_Action_ByCode(OnuProfile_Modify_SyncBroadcast, 0, 0, 0, szname, pd->confname, NULL);
                        
                    if(OnuProfile_Action_ByCode(OnuProfile_Modify, 0, 0, 0, szname, pd->confname, NULL)!=VOS_OK)
                        OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, szname, NULL, NULL);
					ret = CMD_SUCCESS;
				}
				else
                    OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, szname, NULL, NULL);

#endif
			}
			else
			{
				vty_out(vty, "copy profile fail!\r\n");
			}
		}
	}
	ONU_CONF_SEM_GIVE

	return ret;

	
}

DEFUN(onu_profile_show,
      onu_profile_show_cmd,
      "show onu-profile text <name>",
      SHOW_STR
      "onu configuration\n"
      "profile information\n"
      "text of the profile\n"
      "specify onu profile name\n")
{
    int ret = CMD_SUCCESS;

    if(isOnuConfExist(argv[0]))
    {

        char *file = NULL/*cl_config_mem_file_init()*/;

        /*if (file)*/
        {
            if (!generateOnuConfClMemFile(argv[0], -1, vty, file,0,0,0))
            {
                vty_out(vty, "generate config file fail!\n");
                ret = CMD_WARNING;
            }

#if 0
#ifdef _ROUTER_
            free(file);
#else /* platform */
#ifndef _DISTRIBUTE_PLATFORM_
            VOS_Free(file);
#else
            free(file);
#endif
#endif /* #ifdef _ROUTER_ */
#endif
        }
        /*
        else
        {
            vty_out(vty, "alloc file memory fail!\n");
            ret = CMD_WARNING;
        }
        */
    }
    else
    {
        vty_out(vty, "the file doesn't exist!\r\n");
        ret = CMD_WARNING;
    }

    return ret;
}

DEFUN ( onu_profile_switch_one_pon,
        onu_profile_switch_one_pon_cmd,
        "onu-profile switch <slot/port> <slot/port>",
        "onu configuration profile\n"
        "switch onu config for a pon\n"
        "source pon\n"
        "destination pon\n"
        )
{
    ULONG ulsslot, ulsport, uldslot, uldport;

	short int s_pon, d_pon;
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return CMD_WARNING;
        
    VOS_Sscanf(argv[0], "%d/%d", &ulsslot, &ulsport);
    VOS_Sscanf(argv[1], "%d/%d", &uldslot, &uldport);

    if(SlotCardIsPonBoard(ulsslot) != VOS_OK)
    {
        vty_out(vty, "source slot parameter error!\r\n");
        return CMD_WARNING;
    }

    if(ulsport > MAX_PONPORT_PER_BOARD || ulsport == 0)
    {
        vty_out(vty, "source pon port error!\r\n");
        return CMD_WARNING;
    }

    if(SlotCardIsPonBoard(uldslot) != VOS_OK)
    {
        vty_out(vty, "destination slot parameter error!\r\n");
        return CMD_WARNING;
    }

    if(uldport > MAX_PONPORT_PER_BOARD || uldport == 0)
    {
        vty_out(vty, "destination pon port error!\r\n");
        return CMD_WARNING;
    }

	s_pon = GetPonPortIdxBySlot(ulsslot, ulsport);
	if(PonPortSwapEnableQuery(s_pon) !=  V2R1_PON_PORT_SWAP_DISABLE)
	{
		vty_out(vty, "switch profile fail because PON %d/%d is in auto_protect mode!\r\n", ulsslot, ulsport);
		return CMD_WARNING;
	}

	d_pon = GetPonPortIdxBySlot(uldslot, uldport);
	if(PonPortSwapEnableQuery(d_pon) !=  V2R1_PON_PORT_SWAP_DISABLE)
	{
		vty_out(vty, "switch profile fail because PON %d/%d is in auto_protect mode!\r\n", uldslot, uldport);
		return CMD_WARNING;
	}

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        if(onuconfSwitchOnPonPort(ulsslot, ulsport, uldslot, uldport) == VOS_OK)
        {
            if(SYS_LOCAL_MODULE_ISMASTERACTIVE && !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
            {
                startOnuConfSyndReqByCard(ulsslot);
                if(ulsslot != uldslot)
                    startOnuConfSyndReqByCard(uldslot);
            }

			/*添加向备用主控同步，因为备用主控上无ONU的MAC，会导致现在的迁移指令失败 2011-11-29*/
			
			ulsslot = device_standby_master_slotno_get();
			if(ulsslot)
			{
				startOnuConfSyndReqByCard(ulsslot);
			}
			
        }
        else
        {
            vty_out(vty, "onu profiles switch pon fail!\r\n");
            return CMD_WARNING;
        }
    }


    return CMD_SUCCESS;
}

DEFUN ( onu_config_copy_one_onu,
        onu_config_copy_one_onu_cmd,
        "onu-profile copy onu <slot/port/onuid> <slot/port> <onu_list>",
        "onu configuration profile\n"
        "copy configuration\n"
        "copy onu configuration\n"
        "source onu id\n"
        "destination pon id\n"
        "destination onu id list\n"
        )
{
    ULONG ulsslot, ulsport, uldslot, uldport;
    ULONG ulsonu_id, uldonu_id;
	short int s_pon, d_pon;
    
    VOS_Sscanf(argv[0], "%d/%d/%d", &ulsslot, &ulsport, &ulsonu_id);
    VOS_Sscanf(argv[1], "%d/%d", &uldslot, &uldport);

    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return CMD_WARNING;
    
    if(SlotCardIsPonBoard(ulsslot) != VOS_OK)
    {
        vty_out(vty, "source slot parameter error!\r\n");
        return CMD_WARNING;
    }

    if(ulsport > MAX_PONPORT_PER_BOARD)
    {
        vty_out(vty, "source pon port error!\r\n");
        return CMD_WARNING;
    }

    if(ulsonu_id > MAXONUPERPON)
    {
        vty_out(vty, "source onu id error!\r\n");
        return CMD_WARNING;
    }
    if(SlotCardIsPonBoard(uldslot) != VOS_OK)
    {
        vty_out(vty, "destination slot parameter error!\r\n");
        return CMD_WARNING;
    }

    if(uldport > MAX_PONPORT_PER_BOARD)
    {
        vty_out(vty, "destination pon port error!\r\n");
        return CMD_WARNING;
    }

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[2], uldonu_id )
    if(uldonu_id > MAXONUPERPON)
    {
        vty_out(vty, "destination onu id error!\r\n");
        RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
    }
	END_PARSE_ONUID_LIST_TO_ONUID();

	s_pon = GetPonPortIdxBySlot(ulsslot, ulsport);
	if(PonPortSwapEnableQuery(s_pon) !=  V2R1_PON_PORT_SWAP_DISABLE)
	{
		vty_out(vty, "copy profile fail because PON %d/%d is in auto_protect mode!\r\n", ulsslot, ulsport);
		return CMD_WARNING;
	}

	d_pon = GetPonPortIdxBySlot(uldslot, uldport);
	if(PonPortSwapEnableQuery(d_pon) !=  V2R1_PON_PORT_SWAP_DISABLE)
	{
		vty_out(vty, "copy profile fail because PON %d/%d is in auto_protect mode!\r\n", uldslot, uldport);
		return CMD_WARNING;
	}
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[2], uldonu_id )
    if(/*SYS_LOCAL_MODULE_ISMASTERACTIVE*/SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
    {
        if(onuconfSwitchOnuOverPon(vty, ulsslot, ulsport, ulsonu_id-1, uldslot, uldport, uldonu_id-1) != VOS_OK)
        {
            vty_out(vty, "onu profiles copy fail!\r\n");
            RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
        }
    }
	END_PARSE_ONUID_LIST_TO_ONUID();


    return CMD_SUCCESS;
}

DEFUN(onu_profile_starup_show,
        onu_profile_starup_show_cmd,
        "show onu-profile startup",
        SHOW_STR
        /*"onu config\n"*/
        "profile config\n"
        "startup configuration from flash\n")
{

    int ret = CMD_SUCCESS;

    ret = onuConfShowStartupByVty(vty);

    return ret;
}

#if 0
DEFUN  (
    onu_profile_undo_associate,
    onu_profile_undo_associate_cmd,
    "undo onu profile associate {<slot/pon> <onu_list>}*1",
    "undo\n"
    "onu configuration\n"
    "profile create\n"
    "association\n"
    "pon port parameter\n"
    OnuIDStringDesc
    )
{

    ULONG suffix = 0;

    if (argc == 0)
    {
        if (IsProfileNodeVty(vty->index, &suffix))
        {
            ONUConfigData_t *pd = (ONUConfigData_t*) vty->onuconfptr;

            ONU_CONF_SEM_TAKE
            {
                if (onuConfCheckByPtr(suffix, pd) == VOS_OK)
                {
                    ONU_CONF_SEM_GIVE
                    onuconfUndoAssociationByName(pd->confname);
                    startOnuConfSyndBroadcastUndoAssociationByNameMsg(pd->confname);
                    vty_out(vty, "onu config file %s undo association OK\r\n", pd->confname);
                }
                else
                    ONU_CONF_SEM_GIVE
            }

            return CMD_SUCCESS;
        }
        else
        {
            short int ponid, onuid, llid;
            if (parse_onu_command_parameter(vty, &ponid, &onuid, &llid) == VOS_OK)
            {
                ULONG slot, pon, onu;
#ifndef ONUID_MAP
                ONU_MGMT_SEM_TAKE
#endif

                onuconfAssociateOnuId(ponid, onuid, DEFAULT_ONU_CONF);
                /*
                 clrOnuConfNameMapByPonId(getOnuConfNamePtrByPonId(ponid, onuid), ponid, onuid);
                 */

#ifndef ONUID_MAP
                ONU_MGMT_SEM_GIVE
#endif

                if (PON_GetSlotPortOnu(vty->index, &slot, &pon, &onu) == VOS_OK)
                    sendOnuConfSyndUndoAssociationByOnuIdMsg(slot, pon, onu);

                vty_out(vty, "onu undo association OK!\r\n");

                return CMD_SUCCESS;
            }
            else
                return CMD_WARNING;
        }
    }
    else
    {
        ULONG slot, pon, onu;
        short int ponid, onuid;

        VOS_Sscanf(argv[0], "%d/%d", &slot, &pon);

        ponid = GetGlobalPonPortIdxBySlot(slot, pon);

        if (ponid == VOS_ERROR || (!SYS_MODULE_IS_PON(slot)))
        {
            vty_out(vty, "invalid pon port parameter!\r\n");
            return CMD_WARNING;
        }

        BEGIN_PARSE_ONUID_LIST_TO_ONUID(argv[1], onu)

                        onuid = onu - 1;

#ifdef ONUID_MAP

                        onuconfAssociateOnuId(ponid, onuid, DEFAULT_ONU_CONF);

#else
                        ONU_MGMT_SEM_TAKE
                        onuconfAssociateOnuId(ponid, onuid, DEFAULT_ONU_CONF);
                        ONU_MGMT_SEM_GIVE
#endif

                        sendOnuConfSyndUndoAssociationByOnuIdMsg(slot, pon, onu);

                    END_PARSE_ONUID_LIST_TO_ONUID()

        vty_out(vty, "onu undo association OK!\r\n");
    }

    return CMD_SUCCESS;

}
#else
DEFUN  (
    onu_profile_undo_associate,
    onu_profile_undo_associate_cmd,
    "undo onu-profile associate <slot/port> <onuid_list>",
    "undo\n"
    "onu configuration\n"
    "association\n"
    "pon port parameter\n"
    OnuIDStringDesc
    )
{

    ULONG slot, pon, onu;
    short int ponid, onuid;

    VOS_Sscanf(argv[0], "%d/%d", &slot, &pon);

    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return CMD_WARNING;
    
    ponid = GetPonPortIdxBySlot(slot, pon);
    if (ponid == VOS_ERROR || (SlotCardIsPonBoard(slot)) || pon > MAX_PONPORT_PER_BOARD)
    {
        vty_out(vty, "invalid pon port parameter!\r\n");
        return CMD_WARNING;
    }

    BEGIN_PARSE_ONUID_LIST_TO_ONUID(argv[1], onu)
    {
        onuid = onu - 1;
    	/*ULONG ulIfidx = IFM_PON_CREATE_INDEX(slot, pon, onu, 0);*/
		if(checkOnuIsIntoOnuNode(slot, pon, onu) == VOS_ERROR)
		{
			vty_out(vty, " %d/%d/%d is used by other client now!\r\n", slot, pon, onu);
			continue;
		}
    	onu_profile_associate_by_index(vty, slot, pon, onu,  DEFAULT_ONU_CONF);
    }
    END_PARSE_ONUID_LIST_TO_ONUID()

    return CMD_SUCCESS;

}

DEFUN  (
    onu_profile_undo_associate_by_name,
    onu_profile_undo_associate_by_name_cmd,
    "undo onu-profile associate  <name>",
    "undo\n"
    "onu configuration\n"
    "association\n"
    "file name\n"
    )
{

    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return CMD_WARNING;
    
    if(!VOS_StrCmp(argv[0], DEFAULT_ONU_CONF))
    {
        vty_out(vty, "don't undo association with default config file!\r\n");
        return CMD_WARNING;
    }

    if(!isOnuConfExist(argv[0]))
    {
        vty_out(vty, "the file doesn't exist!\r\n");
        return CMD_WARNING;
    }

#if 0
    onuconfUndoAssociationByName(argv[0]);
    startOnuConfSyndBroadcastUndoAssociationByNameMsg(argv[0]);
    vty_out(vty, "onu config file %s undo association OK\r\n", argv[0]);
#else
	ONU_CONF_SEM_TAKE
	{
		short int ponid, onuid;
		ONUConfigData_t *pd = getOnuConfFromHashBucket(argv[0]);
		if(pd)
		{
			for(ponid=0; ponid<SYS_MAX_PON_PORTNUM; ponid++)
			{
				for(onuid=0; onuid<MAXONUPERPON; onuid++)
				{
					short int off = onuid/8;
					short int bitnum = onuid&7;

					if(pd->onulist[ponid][off] & (1<<bitnum))
					{
						int slot = GetCardIdxByPonChip(ponid);
						int port = GetPonPortByPonChip(ponid);
						/*ULONG ulIfidx = IFM_PON_CREATE_INDEX(slot, port, onuid+1, 0);*/
						onu_profile_associate_by_index(vty, slot, port, onuid+1, DEFAULT_ONU_CONF);
					}
				}
			}
		}
		
		
	}
	ONU_CONF_SEM_GIVE
#endif

    return CMD_SUCCESS;

}

#endif

DEFUN  (
    config_into_onu_name_node,
    config_into_onu_name_node_cmd,
    "onu <name>",
    "Select an onu to config\n"
    "Specify onu interface's onu name\n"
    )
{
	SHORT slot=0,port=0,onuid=0;
	LONG   lRet;
	LONG   len = 0;

	len = VOS_StrLen(argv[0]);
	if ( len > MAXDEVICENAMELEN )
	{
		vty_out( vty, "  %% onu name is too long\r\n");
		return CMD_WARNING;
	}
	
	lRet = GetOnuDeviceIdxByName( argv[0], len, &slot, &port, &onuid );
	if ( CLI_ONU_NOT_EXIST == lRet )
	{
		vty_out( vty, "  %% Onu name is not exist\r\n");
		return CMD_WARNING;
	}
   	return into_onu_node( vty, 0, slot, port, onuid+1 );
}

DEFUN ( onu_conf_debug_func,
        onu_conf_debug_cmd,
        "debug onu_conf",
        DEBUG_STR
        "onu config sem\n")
{
    onu_conf_sem_deb = 1;
    return CMD_SUCCESS;
}

DEFUN ( release_onu_debug_func,
        release_onu_debug_cmd,
        "release onu <slot/port/onuid>",
        "release profile related to onu\n"        
        "onu config sem\n"
        "please input the slot/port/onuid\n"        
        )
{
	LONG	lRet;
	ULONG   ulSlot, ulPort, ulOnuid;
	CHAR 	ifName[IFM_NAME_SIZE + 1];
	INT16 phyPonId = 0;
    
    if(!SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
        return CMD_WARNING;
    
	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	
	if( PonCardSlotPortCheckByVty(ulSlot, ulPort, vty) != ROK )
		return(CMD_WARNING );
	if(SlotCardMayBePonBoardByVty(ulSlot, vty)  != ROK )
		return(CMD_WARNING);

	if ((ulOnuid<(CLI_EPON_ONUMIN+1)) || (ulOnuid>(CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onu %d/%d/%d error\r\n",ulSlot, ulPort, ulOnuid );
		return CMD_WARNING;	
	}

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if( phyPonId == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	
    
    ReleasePermissionByOnuid(phyPonId, ulOnuid-1);
    return CMD_SUCCESS;
}

DEFUN ( release_profile_debug_func,
        release_profile_debug_cmd,
        "release profile <name>",
        "release profile related to onu\n"        
        "onu config sem\n"
        "please input the profile name\n"
        )
{
    if(!SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
        return CMD_WARNING;    
    ReleasePermissionByName(argv[0]);
    return CMD_SUCCESS;
}
extern int g_onu_temp_profile_num;
DEFUN(
        onu_profile_info_debug_show,
        onu_profile_info_debug_show_cmd,
        "show temp-onu-profile list {counter}*1",
        SHOW_STR
        "temporary onu profile infor\n"
        "file list\n"
        "file counter\n"
        )
{

    if (VOS_StrRChr(vty->buf, 'c'))
    {
        vty_out(vty, "\r\ntemporary onu config file counter:\t%d\r\n\r\n", g_onu_temp_profile_num);
    }
    else
        show_onu_temp_conf_list(vty);

    return CMD_SUCCESS;
}

DEFUN ( undo_onu_conf_debug_func,
        undo_onu_conf_debug_cmd,
        "undo debug onu_conf",
        "undo operation\n"
        DEBUG_STR
        "onu config sem\n")
{
    onu_conf_sem_deb = 0;
    return CMD_SUCCESS;
}

DEFUN ( onu_conf_debug_timer,
        onu_conf_debug_timer_cmd,
        "onu conf timer <1-3600>",
        "onu\n"
        "config debug\n"
        "timer debug\n"
        "timeout value\n")
{
    changeSyncTimer(VOS_StrToUL(argv[0], NULL, 10));
    return CMD_SUCCESS;
}

DEFUN( onu_conf_debug_level_set,
        onu_conf_debug_level_set_cmd,
        "onu-conf debug [gen|cdp]",
        "onu config\n"
        "debug level set\n"
        "general level\n"
        "cdp level\n")
{
    int level = ONU_CONF_DEBUG_LVL_GENERAL;

    if(!VOS_StrCmp(argv[0], "gen"))
        level = ONU_CONF_DEBUG_LVL_GENERAL;
    else
        level = ONU_CONF_DEBUG_LVL_CDP;

    onuconf_DebugLevelSet(level);

    return CMD_SUCCESS;
}

DEFUN( undo_onu_conf_debug_level_set,
        undo_onu_conf_debug_level_set_cmd,
        "undo onu-conf debug [gen|cdp]",
        "undo operation\n"
        "onu config\n"
        "debug level set\n"
        "general level\n"
        "cdp level\n")
{
    int level = ONU_CONF_DEBUG_LVL_GENERAL;

    if(!VOS_StrCmp(argv[0], "gen"))
        level = ONU_CONF_DEBUG_LVL_GENERAL;
    else
        level = ONU_CONF_DEBUG_LVL_CDP;

    onuconf_DebugLevelClr(level);

    return CMD_SUCCESS;
}

/***********ONU POINT****************/
/* added by xieshl 20110112, 集中检查ONU是否有效 */
LONG checkVtyOnuIsValid( struct vty *vty, ULONG ulSlot, ULONG ulPort, ULONG onuId, SHORT *pPhyPonId, SHORT *pUserOnuId )
{
	SHORT phyPonId = 0;
	SHORT userOnuId = 0;
	LONG ret = VOS_ERROR;

	if( (NULL == pPhyPonId) || (NULL == pUserOnuId) )
		return ret;
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return ret;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if ((phyPonId < (CLI_EPON_PONMIN)) || (phyPonId > (CLI_EPON_PONMAX)))
	{
		vty_out( vty, "  %% pon Parameter is error.\r\n" );
		return ret;    	
	}
	userOnuId = onuId - 1;
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d is not exist. \r\n",ulSlot, ulPort, onuId);
		return ret;	
	}
	
	/* commented by wangxiaoyu 2011-08-30 for the onu is not online while ctc-onu's name restoring
	 * */
#if 0
	/*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	if ( CLI_EPON_ONUDOWN == GetOnuOperStatus( phyPonId, userOnuId))
       {
		vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, onuId) ;
		return ret;
       }
#endif

	*pPhyPonId = phyPonId;
	*pUserOnuId = userOnuId;
	
	return VOS_OK;
}

static LONG set_onu_device_info_in_vty( struct vty *vty, CHAR *data, FUNCPTR func )
{
	LONG lRet = VOS_OK;	
	LONG len;	
	ULONG ulSlot = 0;
	ULONG ulPort = 0;	
	ULONG onuId = 0; 
	ULONG ulIfIndex;	
	SHORT phyPonId;
	SHORT userOnuId;

	len = VOS_StrLen(data);	
	if( len > MAXDEVICENAMELEN)
	{
		vty_out( vty, "  %% Err:parameter is too long.\r\n" );
		return CMD_WARNING;
	}
	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;

	if( checkVtyOnuIsValid(vty, ulSlot, ulPort, onuId, &phyPonId, &userOnuId) == VOS_ERROR )
		return CMD_WARNING;
		
	lRet = func( phyPonId, userOnuId,  data, len );
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}
/* 20110112 */

DEFUN  (
    onu_customer_location_config,
    onu_customer_location_config_cmd,
    "device location <location>",
    "Config onu device info\n"
    "Config onu location\n"
    "Please input the onu location( no more than 128 characters)\n"
    )
{
#if 1
    LONG lRet = VOS_OK;	
	LONG len;	
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    ULONG ulIfIndex;	
    INT16 phyPonId;
    INT16 userOnuId;

	len = strlen(argv[0]);	
	if( len > MAXLOCATIONLEN)
    {
       vty_out( vty, "  %% Err:location is too long.\r\n" );
	   return CMD_WARNING;
    }	
	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, slotId, port) != VOS_OK )
		return CMD_WARNING;

    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );

	/*   问题单:4692
       徐州现场,OLTpon8/1下ONU无法修改名
          if ((phyPonId < (CLI_EPON_PONMIN+1)) || (phyPonId > (CLI_EPON_PONMAX+1))) */
    if ((phyPonId < (CLI_EPON_PONMIN)) || (phyPonId > (CLI_EPON_PONMAX)))
    {
	    vty_out( vty, "  %% pon Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	userOnuId = onuId - 1;
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}
	
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
   	   vty_out( vty, "  %% %d/%d/%d is off-line.\r\n",slotId,  port, onuId) ;
		return CMD_WARNING;
       }
#if 0			 
	lRet = SetOnuLocation( phyPonId, userOnuId,  argv[0], len );
#else
	lRet = OnuMgt_SetOnuDeviceLocation( phyPonId, userOnuId,  argv[0],  len);
#endif
        if (lRet != VOS_OK)
        {
            vty_out( vty, "  %% Executing error.\r\n" );
    	    return CMD_WARNING;
        }	
    return CMD_SUCCESS;
#else
	return set_onu_device_info_in_vty( vty, argv[0], (FUNCPTR)SetOnuLocation );
#endif
}
	
extern int g_SystemLoadConfComplete;
DEFUN  (
    onu_device_name_config,
    onu_device_name_config_cmd,
    "device name <name>",
    "Config onu device info\n"
    "Config onu name \n"
    "Please input the device name( no more than 127 characters),\n"
    )
{
#if 1
    LONG lRet = VOS_OK;	
	INT32 len = 0;	
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    ULONG ulIfIndex = 0;	
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;

	len = strlen(argv[0]);	
	if( len > MAXDEVICENAMELEN )
    {
       vty_out( vty, "  %% Err:name is too long.\r\n" );
	   return CMD_WARNING;
    }		
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, slotId, port) != VOS_OK )
		return CMD_WARNING;

    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
		/*   问题单:4692
       徐州现场,OLTpon8/1下ONU无法修改名
          if ((phyPonId < (CLI_EPON_PONMIN+1)) || (phyPonId > (CLI_EPON_PONMAX+1))) */
    if ((phyPonId < (CLI_EPON_PONMIN)) || (phyPonId > (CLI_EPON_PONMAX)))
    {
	    vty_out( vty, "  %% pon Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	userOnuId = onuId - 1;
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
   	   vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",slotId,  port, onuId) ;
		return CMD_WARNING;
       }
#if 0		 
	lRet = SetOnuDeviceName( phyPonId, userOnuId,  argv[0],  len );
#else
	lRet = OnuMgt_SetOnuDeviceName( phyPonId, userOnuId,  argv[0],  len);
#endif
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }
		
    return CMD_SUCCESS;
#else
	if( VOS_StrLen(argv[0]) >= MAXDEVICENAMELEN )	/* modified by xieshl 20111129，问题单14033 */
	{
		vty_out( vty, "  %% Err:name is too long.\r\n" );
		return CMD_WARNING;
	}		

    if( g_SystemLoadConfComplete == _VOS_SYSTEM_STARTING_ )
    {
        ULONG slot, port, onuid;
        if(PON_GetSlotPortOnu(vty->index, &slot, &port, &onuid) == VOS_OK)
        {
            int ponportidx = GetPonPortIdxBySlot(slot, port);
            /*int onuEntry = ponportidx*MAXONUPERPON+onuid-1;*/

            ONU_MGMT_SEM_TAKE;
            /*VOS_StrnCpy(OnuMgmtTable[onuEntry].DeviceInfo.DeviceName, argv[0], VOS_StrLen(argv[0]));
            OnuMgmtTable[onuEntry].DeviceInfo.DeviceNameLen = VOS_StrLen(argv[0]);*/
            SetOnuDeviceName_1( ponportidx, onuid-1, argv[0], VOS_StrLen(argv[0]) );	/* modified by xieshl 20111122, 防止丢掉结束符，问题单13917 */
            ONU_MGMT_SEM_GIVE;
        }
    }
    else
    	return set_onu_device_info_in_vty( vty, argv[0], (FUNCPTR)SetOnuDeviceName );
#endif
}

DEFUN  (
    onu_device_description_config,
    onu_device_description_config_cmd,
    "device description <description>",
    "Config onu device info\n"
    "Config onu description\n"
    "Please input the device description( no more than 128 characters)\n"
    )
{
#if 1
    LONG lRet = VOS_OK;	
	INT32 len = 0;	
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    ULONG ulIfIndex = 0;	
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;

	len = strlen(argv[0]);	
	if( len > MAXDEVICEDESCLEN )
        {
            vty_out( vty, "  %% Err:description is too long.\r\n" );
    	    return CMD_WARNING;
        }	
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, slotId, port) != VOS_OK )
		return CMD_WARNING;

    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
		/*   问题单:4692
       徐州现场,OLTpon8/1下ONU无法修改名
          if ((phyPonId < (CLI_EPON_PONMIN+1)) || (phyPonId > (CLI_EPON_PONMAX+1))) */
    if ((phyPonId < (CLI_EPON_PONMIN)) || (phyPonId > (CLI_EPON_PONMAX)))
    {
	    vty_out( vty, "  %% pon Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	userOnuId = onuId - 1;
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
   	   vty_out( vty, "  %% %d/%d/%d is off-line.\r\n",slotId,  port, onuId) ;
		return CMD_WARNING;
       }
#if 0		 
	lRet = SetOnuDeviceDesc( phyPonId, userOnuId,  argv[0],  len );
#else
	lRet = OnuMgt_SetOnuDeviceDesc( phyPonId, userOnuId,  argv[0],  len);
#endif
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }
		        
    return CMD_SUCCESS;
#else
	return set_onu_device_info_in_vty( vty, argv[0], (FUNCPTR)SetOnuDeviceDesc );
#endif
}

/* modified by chenfj 2007-9-10 ,将此命令移到DEBUG节点下
	问题单#5273 :deregister命令在目前的ONU认证模式下，不再有用处了，建议取消。*/
DEFUN  (
    onu_deregister,
    onu_deregister_cmd,
    "onu deregister <slot/port/onuid>",
    "onu deregister\n"
     "onu deregister\n"
     "please input the slot/port/onuid\n"
    )
{
    LONG lRet = VOS_OK;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG onuId = 0;	
    INT16 phyPonId = 0;
	INT16 	userOnuId ;
		
	lRet = PON_ParseSlotPortOnu( argv[0], (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
	if( lRet != VOS_OK )
		return CMD_WARNING;


	IFM_ParseSlotPort( argv[0], (ULONG *)&slotId, (ULONG *)&port );

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);

	if( checkVtyOnuIsValid(vty, slotId, port, onuId, &phyPonId, &userOnuId) == VOS_ERROR )
		return CMD_WARNING;

	lRet = Onu_deregister( phyPonId, (short int)userOnuId );	

    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	

    return CMD_SUCCESS;
}

DEFUN  (
    onu_remote_reset,
    onu_remote_reset_cmd,
    "ctc-onu reset <slot/port> <llid>",
    "CTC Onu\n"
    "Reset onu\n"
    "please input the slot/port\n"
    "please input the llid\n"
    )
{
    LONG lRet = VOS_OK;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG llid = 0;	
    INT16 phyPonId = 0;
	INT16 	userOnuId ;

	IFM_ParseSlotPort( argv[0], (ULONG *)&slotId, (ULONG *)&port );
	llid = VOS_AtoL(argv[1]);	

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);

	if( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
        return CMD_WARNING;
	}

    if(SYS_MODULE_IS_CPU_PON(slotId) && slotId != SYS_LOCAL_MODULE_SLOTNO)
    {
        return CMD_WARNING;
    }  
    
    phyPonId = GetPonPortIdxBySlot(slotId, port);
	lRet = CTC_STACK_reset_onu(phyPonId, llid);	
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
    	return CMD_WARNING;
    }	

    return CMD_SUCCESS;
}


DEFUN  (
    onu_auto_update_config,
    onu_auto_update_config_cmd,
    "software update enable",
    "Config the onu software\n"
    "Config the onu software update enable\n"
    "Config the onu software update enable\n"
    )
{
    LONG lRet = VOS_OK;		
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG onuId = 0;
    ULONG ulIfIndex = 0;
	INT16 userOnuId = 0;	
    INT16 phyPonId = 0;
	unsigned char EnableFlag = CLI_EPON_ONUUPDATE_EN;
	#if 0
    if ( !VOS_StrCmp( argv[ 0 ], "enable" ) )
			EnableFlag = CLI_EPON_ONUUPDATE_EN;
	else if ( !VOS_StrCmp( argv[ 0 ], "disable" ) )
		EnableFlag = CLI_EPON_ONUUPDATE_DIS;
	else
    {  
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	#endif
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
		
	userOnuId = onuId - 1;
	
	#if 0
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId,  userOnuId) ;
       #endif
		vty_out( vty, "  %% %d/%d/%d is off-line.\r\n",slotId, port, onuId) ;		return CMD_WARNING;
       }
	#endif
	
	if(ThisIsValidOnu( phyPonId, userOnuId) != ROK)
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,onuId);
		return CMD_WARNING;  
		}
	lRet = SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	

    return CMD_SUCCESS;
}


/*added by wutw 2006/11/28*/
DEFUN  (
    onu_auto_update_disable,
    onu_auto_update_disable_cmd,
    "undo software update enable",
    NO_STR
    "Config the onu software\n"
    "Config the onu software update disable\n"
    "Config the onu software update disable\n"
    )
{
    LONG lRet = VOS_OK;		
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG onuId = 0;
    ULONG ulIfIndex = 0;
	INT16 userOnuId = 0;	
    INT16 phyPonId = 0;
	unsigned char EnableFlag =CLI_EPON_ONUUPDATE_DIS;
	#if 0
    if ( !VOS_StrCmp( argv[ 0 ], "enable" ) )
			EnableFlag = CLI_EPON_ONUUPDATE_EN;
	else if ( !VOS_StrCmp( argv[ 0 ], "disable" ) )
		EnableFlag = CLI_EPON_ONUUPDATE_DIS;
	else
    {  
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	#endif
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
		
	userOnuId = onuId - 1;
	
	#if 0
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId,  userOnuId) ;
       #endif
		vty_out( vty, "  %% %d/%d/%d is off-line.\r\n",slotId, port, onuId) ;		return CMD_WARNING;
       }
	#endif

	if(ThisIsValidOnu( phyPonId, userOnuId) != ROK)
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,onuId);
		return CMD_WARNING;  
		}
	lRet = SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	

    return CMD_SUCCESS;
}






DEFUN  (
    onu_reset,
    onu_reset_cmd,
    "remote reset",
    "Reset the onu pon from olt \n"
    "Reset the onu pon from olt \n"
    )
{
    LONG lRet = VOS_OK;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG onuId = 0;
    ULONG ulIfIndex = 0;	
    INT16 phyPonId = 0;
	INT16 userOnuId = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;

	if( checkVtyOnuIsValid(vty, slotId, port, onuId, &phyPonId, &userOnuId) == VOS_ERROR )
		return CMD_WARNING;
	
	/*lRet = ResetOnu( phyPonId, userOnuId );	*/
	lRet=OnuMgt_ResetOnu( phyPonId, userOnuId);
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	
		
    return CMD_SUCCESS;
}

#ifdef  _ONU_ENCRYPT_
#endif
/*added by wutw at 20 september*/
/* modified by chenfj 2007-10-19
    问题单:5602.ONU节点下命令encrypt disable和undo encrypt的作用好像是一样的
    处理: 两个功能是一样的，去掉encrypt [up-down|down|disable]中的disable参数，保留undo encrypt
	*/    
DEFUN  (
    onu_encrypt_config,
    onu_encrypt_config_cmd,
    "encrypt direction [up-down|down] update-key-interval [default|<30-10000>]",
    "Config onu's encrypt attribute\n"
    "encrypt direction\n"
    "Both up and down\n"
    "Only down\n"
    "encrypt key update interval\n"
    "restore to deault value\n"
    "Please input the time length(unit:second)\n"
    /*&g_Olt_Queue_Id*/
    )
{
    LONG lRet = VOS_OK;
    unsigned int  ulEncrypt = 0;
    ULONG ulIfIndex = 0;
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
	INT16 	userOnuId = 0;	
	/*unsigned int EncryptStatus;*/
	unsigned int UpdateKeyTime = 0;
		
    ulIfIndex = ( ULONG ) ( vty->index ) ;

    if ( !VOS_StrCmp( argv[ 0 ], "up-down" ) )
    {
        ulEncrypt = PON_ENCRYPTION_DIRECTION_ALL;
    }
    else /*if ( !VOS_StrCmp( argv[ 0 ], "down" ) )*/
    {
         ulEncrypt = PON_ENCRYPTION_DIRECTION_DOWN;
    }
/*
    else if ( !VOS_StrCmp( argv[ 0 ], "disable") )
    {
        ulEncrypt = PON_ENCRYPTION_PURE;
    }
    
    else
    {
    #ifdef CLI_EPON_DEBUG
	 vty_out(vty, "  %% DEBUG: Error direction : %s.\r\n",argv[ 0 ]);
    #endif       
        vty_out( vty, "  %% Parameter is error. %s\r\n" , argv[ 0 ] );
        return CMD_WARNING;
    }*/

	if(VOS_StrCmp( argv[ 1 ], "default" ) == 0 )
		UpdateKeyTime = GetOnuEncryptKeyExchagetimeDefault();
	else 
		UpdateKeyTime = VOS_AtoL( argv[1] )*SECOND_1;
	
	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out( vty, "  %% DEBUG: phyPonId %d  slotId %d port %d  onuId \r\n",phyPonId, slotId, port, onuId);
    #endif    
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
		
	userOnuId = onuId - 1;

	if( ThisIsValidOnu((short int)phyPonId, (short int)userOnuId ) != ROK )
		{
		vty_out(vty, "  %% onu%d/%d/%d not exist\r\n", slotId, port, onuId);
		return( CMD_WARNING);
		}
	
	/* added by chenfj 2007-9-13
	在对ONU加密时，判断ONU与OLT类型是否匹配*/
	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( EncryptIsSupported((short int)phyPonId, (short int)userOnuId ) != ROK )
			{
			vty_out(vty, "  %%  PON chip mismatch, encrypt is failed\r\n");
			return( CMD_WARNING );
			}
		}
	/* modified by chenfj 2008-5-21
	    问题单6711: ONU下作encrypt down与encrypt up-down直接互变出现ONU离线
	    修改:  ONU 加密 方向不能直接变换, 必须先停止,再启动
	    
	if((GetOnuEncryptStatus((short int)phyPonId, (short int)userOnuId , &EncryptStatus) == ROK )
		&&(EncryptStatus == V2R1_STARTED))
		{
		if( OnuMgmtTable[phyPonId * MAXONUPERPON + userOnuId].EncryptDirection == ulEncrypt )
			{
			SetOnuEncryptKeyExchagetime((short int)phyPonId, userOnuId, UpdateKeyTime);
			return( CMD_SUCCESS );
			}
		else {
			vty_out(vty,"onu%d/%d/%d encrypt is started, direction is %s; if want to start %s encrypt, should stop encrypt first\r\n", slotId, port, onuId, v2r1EncryptDirection[OnuMgmtTable[phyPonId * MAXONUPERPON + userOnuId].EncryptDirection], v2r1EncryptDirection[ulEncrypt]);
			return( CMD_WARNING );
			}
		}
	*/
	lRet = OnuEncryptionOperation( (short int) phyPonId, (short int)(userOnuId), ulEncrypt);	
    if (lRet != VOS_OK)
    {
    #ifdef CLI_EPON_DEBUG
       vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d ulEncrypt %d.\r\n",lRet, phyPonId, ulEncrypt);
    #endif
    	vty_out( vty, "  %% encrypt Executing error.\r\n" );
	return CMD_WARNING;
    }	
	SetOnuEncryptKeyExchagetime((short int)phyPonId, userOnuId, UpdateKeyTime);
	
    return CMD_SUCCESS;
}

/*当少于100millisecond时,则该密钥不起作用*/
DEFUN  (
    onu_encrypt_keytime_config,
    onu_encrypt_keytime_config_cmd,
    "encrypt update-key-interval [default|<30-10000>]",
    "Config onu's encrypt\n"
    "encrypt key update interval\n"
    "restore to deault value\n"
    "Please input the time length(second)\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;	
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
	INT16 	userOnuId = 0;
    unsigned int  time_len = 0;

   	if(VOS_StrCmp( argv[ 0 ], "default" ) == 0 )
		time_len = GetOnuEncryptKeyExchagetimeDefault();
	else 
		time_len = VOS_AtoL( argv[0] )*SECOND_1;
	
    if (time_len <= 0)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: time_len %d  cmd encrypt-keytime failed\r\n",time_len);
    #endif         
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_SUCCESS;
    }
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;   
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
		
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
    #endif      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	userOnuId = onuId - 1;
	if( ThisIsValidOnu((short int)phyPonId, (short int)userOnuId ) != ROK )
		{
		vty_out(vty, "  %% onu%d/%d/%d not exist\r\n", slotId, port, onuId);
		return( CMD_WARNING);
		}

	/*added by wutongwu 26 October*/

	lRet = SetOnuEncryptKeyExchagetime((short int )phyPonId, (short int )userOnuId, time_len);
	if (lRet != VOS_OK)
	    {
	    #ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d userOnuId %d time_len %d.\r\n",lRet, phyPonId, onuId,  time_len);
	    #endif         
	    	vty_out( vty, "  %% Executing error.\r\n");
		return CMD_WARNING;
	    }	
	
    return CMD_SUCCESS;
}


QDEFUN  (
    onu_show_encrypt_config,
	onu_show_encrypt_config_cmd,
    "show encrypt information",
    DescStringCommonShow
    "Show onu's encrypt attribute\n"
    "Show onu's encrypt information\n",
    &g_Olt_Queue_Id
    )
{
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;	
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
	INT16 	userOnuId = 0;
		
    ulIfIndex = ( ULONG ) ( vty->index ) ;   
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
    #endif      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	userOnuId = onuId - 1;

	if( ThisIsValidOnu((short int)phyPonId, (short int)userOnuId ) != ROK )
		{
		vty_out(vty, "  %% onu%d/%d/%d not exist\r\n", slotId, port, onuId);
		return( CMD_WARNING);
		}
	
	/*for debug */

	lRet = ShowOnuEncryptInfoByVty(phyPonId, userOnuId, vty);
	if (lRet != VOS_OK)
	    {
	    #ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d userOnuId %d time_len %d.\r\n",lRet, phyPonId, onuId,  time_len);
	    #endif         
	    	vty_out( vty, "  %% Executing error.\r\n");
		return CMD_WARNING;
	    }
	 
    return CMD_SUCCESS;
}


DEFUN  (
    onu_no_encrypt_config,
    onu_no_encrypt_config_cmd,
    "undo encrypt",
    NO_STR
    "Config onu's encrypt\n"
   /* &g_Olt_Queue_Id*/
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;	
	INT16 userOnuId = 0;
    unsigned int ulEncrypt = PON_ENCRYPTION_PURE;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	

    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	userOnuId = onuId - 1;

	/* added by chenfj 2007-9-13
	在对ONU加密时，判断ONU与OLT类型是否匹配*/
	if( ThisIsValidOnu((short int)phyPonId, (short int)userOnuId ) != ROK )
		{
		vty_out(vty, "  %% onu%d/%d/%d not exist\r\n", slotId, port, onuId);
		return( CMD_WARNING);
		}
	if( EncryptIsSupported((short int)phyPonId, (short int)userOnuId ) != ROK )
		{
		vty_out(vty, "  %%  PON chip mismatch, encrypt is failed\r\n");
		return( CMD_WARNING );
		}

	
	lRet = OnuEncryptionOperation( (short int) phyPonId, (short int)userOnuId, ulEncrypt);
	if (lRet != VOS_OK)
	    {
	    	vty_out( vty, "  %% undo-encrypt Executing error.\r\n" );
			return CMD_WARNING;
	    }	
	SetOnuEncryptKeyExchagetime((short int) phyPonId, (short int)userOnuId, GetOnuEncryptKeyExchagetimeDefault());
		
    return CMD_SUCCESS;
}


/*该命令行*/

/* modified by chenfj 2007-8-17 
	在命令行中增加将要升级的APP类型
	
	modified by chenfj 2007-10-24
	单独做一个命令，用于ONU APP文件在GW模式和CTC模式之间切换
	当前支持的ONU类型: 
	   在GW模式下，向CTC模式切换时，ONU 类型有GT810/GT816；升级的文件为GT_CTC_4FE.   其余类型均不升级(待扩展);
	   在CTC模式下，向GW模式切换时，如果ONU的FE端口等于1，VOIP与E1端口均为0，则升级GT816/810文件（GT816与GT810目前是同一个文件），否则不升级
	*/

extern char convert_onu_file_type[ONU_TYPE_LEN +4];
DEFUN  (
    onu_file_convert,
    onu_file_convert_cmd,
    "convert onu-file <onu_file_id>",
    "convert onu file command\n"
    "convert onu file\n"
    "onu file identify\n"	/*if running non-ctc mode, then update to CTC file; and if running CTC mode, then update to non-ctc file\n*/
    )
{
	/*LONG lRet = VOS_OK;	*/
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	ULONG ulIfIndex = 0;
	INT16 phyPonId = 0;	
	INT16 userOnuId = 0;	
	/*ulong_t act = 0;
	ulong_t para = 0;
	int update_mode = IMAGE_UPDATE_MODE_UNKNOWN;*/
	/*ULONG file_spec;
	int OnuType;
	int FE_num, POTS_num, E1_num;*/
	onu_update_msg_t updMsg;
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;	

	if( VOS_StrLen(argv[0]) > ONU_TYPE_LEN )
	{
		vty_out( vty, " file_type is too long\r\n" );
		return CMD_WARNING;
	}
	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	    if (phyPonId == VOS_ERROR)
	    {
		#ifdef CLI_EPON_DEBUG
		    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
		#endif   
	        vty_out( vty, "  %% Parameter is error.\r\n" );
	        return CMD_WARNING;
	    }

	userOnuId = (INT16)(ulOnuId - 1);

	VOS_MemZero( convert_onu_file_type, sizeof(convert_onu_file_type) );
	VOS_StrCpy( convert_onu_file_type, argv[0] );

#if 0	/* modified by xieshl 20100304, 放开GW<->CTC ONU 文件转换限制*/
	if( ThisIsValidOnu((short int)phyPonId, (short int)userOnuId ) != ROK )
		{
		vty_out(vty, "  %% onu%d/%d/%d not exist\r\n", ulSlot, ulPort, ulOnuId);
		return( CMD_WARNING);
		}
	/*vty_out(vty,"onuid=%d,%d\r\n", userOnuId, ulOnuId );*/
	/*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	lRet = GetOnuOperStatus( phyPonId, userOnuId);
	if (CLI_EPON_ONUDOWN == lRet)
		{
		vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, ulOnuId) ;
		return( CMD_WARNING );
		}	
	lRet = GetOnuType(phyPonId, userOnuId, &OnuType );
	if( lRet != ROK )
		{
		vty_out(vty, "  %% Get onu%d/%d/%d type err\r\n", ulSlot,  ulPort, ulOnuId) ;
		return( CMD_WARNING );
		}
	if(( OnuType == V2R1_ONU_GT810) || ( OnuType == V2R1_ONU_GT816) || ( OnuType == V2R1_ONU_GT811_A))
		{
		file_spec=  MSG_SPEC_CTC_TYPE;
		}
	else if( OnuType == V2R1_ONU_CTC )
		{
		CTC_getDeviceCapEthPortNum(phyPonId, userOnuId, &FE_num);
		CTC_getDeviceCapiadPotsPortNum(phyPonId, userOnuId, &POTS_num);
		CTC_getDeviceCapE1PortNum( phyPonId, userOnuId, &E1_num);

		if( (( FE_num == 1 ) && ( E1_num == 0 ) && ( POTS_num == 0)) /* GT816/810 */
			|| (( FE_num == 4 ) && ( E1_num == 0 ) && ( POTS_num == 0)) ) /* GT811_A */
			{
			file_spec = MSG_SPEC_GW_TYPE;
			}
		
		else{
			vty_out(vty, "  %%ctc onu %d/%d/%d type mismatch,%s<->CTC file convert is not defined for this onu\r\n",  ulSlot,  ulPort, ulOnuId, PRODUCT_CORPORATION_AB_NAME);
			return( CMD_WARNING );
			}
		}
	else {
		vty_out(vty," %%onu %d/%d/%d type mismatch,%s<->CTC file convert is not defined for this onu\r\n",  ulSlot,  ulPort, ulOnuId, PRODUCT_CORPORATION_AB_NAME);
		return( CMD_WARNING);
		}
#else
		/*update_mode = get_onu_image_update_mode( vty, phyPonId, userOnuId );
		if( update_mode == IMAGE_UPDATE_MODE_UNKNOWN )
			return CMD_WARNING;*/
#endif
	/*act:操作码， 0 -- 升级单个ONU；1 -- 升级全部ONU
	para: 操作数， 当act为0时，该值为要升级的ONU索引，当act为 1 时，该值为0
	*/
	/*act = MSGTYPE_UPDATE_ONE;
	para = 10000*ulSlot + 1000*ulPort + ulOnuId;*/
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[phyPonId*MAXONUPERPON+userOnuId].vty = vty;
	ONU_MGMT_SEM_GIVE;
	/*sendOnuOamUpdMsg( update_mode, act, para, IMAGE_TYPE_APP);*/
	VOS_MemCpy( updMsg.reserved, convert_onu_file_type, ONU_TYPE_LEN );	/* modified by xieshl 20110802, 问题单12878 */
	updMsg.upd_mode = IMAGE_UPDATE_MODE_UNKNOWN;
	updMsg.msg_type = MSGTYPE_UPDATE_ONE;
	updMsg.onuDevIdx = MAKEDEVID(ulSlot,ulPort,ulOnuId)/*10000*ulSlot + 1000*ulPort + ulOnuId*/;
	updMsg.file_type = 0;
	if( OLT_SetOnuUpdateMsg( phyPonId, &updMsg ) != OLT_ERR_OK )
		vty_out( vty, "perhaps update task is busy now!\r\n" );

	return( CMD_SUCCESS);
	
}

/* modified by chenfj 2007-10-25
	参数中增加ONU侧epon程序，ONU侧的语音程序(voice) 可选项	
	当不带任何参数时，对GT831类型ONU，会同时升级其epon和voice 	
	对其他类型ONU，只升级epon；
	当带有可选项时，则升级指定的程序
	以后还会有ONU的FPGA程序 ，待扩展
	*/
/* modified by chenfj 2008-2-18
     GT831 内部发布技术评审时, 讨论决定,简化ONU 程序升级命令, 
     不再带有epon  和voice 可选参数;
     这样就避免了只升级GT831 语音程序后, ONU不重启,从而OLT 上保存的ONU
     语音程序版本与ONU实际运行版本不一致的可能
     */
/* modified by chenfj 2008-5-23
    扩充命令update onu file, 后增加{[epon]}*1可选参数;主要是因为近期增加了
    GT865的FPGA升级,而GT865以前的版本是不支持升级FPGA的,增加这个参数
    可增加一些灵活性, 不至于在现场由于错误的升级顺序, 导致无法升级ONU
    */
DEFUN  (
    onu_update_file,
    onu_update_file_cmd,
    /*"update onu software {[epon|voice]}*1",*/
    "update onu file {epon}*1",
    "software update command\n"
    "onu software update\n"
    "onu software update\n"
    "onu app image\n"
    /*"onu voice image\n"
    "onu fgpa image\n"*/
    )
{
    /*LONG lRet = VOS_OK;	*/
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    ULONG ulIfIndex = 0;	
    INT16 phyPonId = 0;
	INT16 userOnuId = 0;
	/*ulong_t act = 0;
	ulong_t para = 0;*/
	onu_update_msg_t updMsg;
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;

	if( checkVtyOnuIsValid(vty, ulSlot, ulPort, ulOnuId, &phyPonId, &userOnuId) == VOS_ERROR )
		return CMD_WARNING;

	/* added by chenfj 2008-2-21
	启动ONU软件升级时,判断ONU是否正处于升级状态;若ONU正升级,
	则不将此ONU 添加到等待软件升级队列中
	*/
	if( GetOnuSWUpdateStatus(phyPonId, userOnuId ) == ONU_SW_UPDATE_STATUS_INPROGRESS )
		{
		vty_out( vty, "  onu %d/%d/%d image is updating\r\n", ulSlot, ulPort, (userOnuId+1) );
		return( CMD_SUCCESS);
		}

	ONU_MGMT_SEM_TAKE;
 	OnuMgmtTable[phyPonId*MAXONUPERPON + userOnuId].vty = vty;
	ONU_MGMT_SEM_GIVE;
	/*act  = MSGTYPE_UPDATE_ONE;
	para = 10000*slotId + 1000*port + onuId;*/

	/*if( argc == 0)
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, act, para, IMAGE_TYPE_ALL);
	else if( VOS_StrCmp(argv[0], "epon") == 0)
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, act, para, IMAGE_TYPE_APP);
	else if( VOS_StrCmp(argv[0], "voice") == 0)
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, act, para, IMAGE_TYPE_VOIP);
	else if(VOS_StrCmp(argv[0], "fpga") == 0)
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, act, para, IMAGE_TYPE_FPGA);
	else{
		vty_out(vty, "  %% parameter err\r\n");
		return( CMD_WARNING );
		}*/
	VOS_MemZero( updMsg.reserved, ONU_TYPE_LEN );
	updMsg.upd_mode = IMAGE_UPDATE_MODE_INVALID;
	updMsg.msg_type = MSGTYPE_UPDATE_ONE;
	updMsg.onuDevIdx = MAKEDEVID(ulSlot,ulPort,ulOnuId)/*10000*ulSlot + 1000*ulPort + ulOnuId*/;
	if( argc == 0)
		updMsg.file_type = IMAGE_TYPE_ALL;
	else /*if( VOS_StrCmp(argv[0], "epon") == 0)*/
		updMsg.file_type = IMAGE_TYPE_APP;
	if( OLT_SetOnuUpdateMsg( phyPonId, &updMsg ) != OLT_ERR_OK )
		vty_out( vty, "perhaps update task is busy now!\r\n" );

	return( CMD_SUCCESS);
}


DEFUN  (
    onu_show_software_auto_update,
    onu_show_software_auto_update_cmd,
    "show software update",
    DescStringCommonShow
    "Show onu software information\n"
    "Show onu software update enable\n"
    )
{
    LONG lRet = VOS_OK;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG onuId = 0;
    ULONG ulIfIndex = 0;	
    INT16 phyPonId = 0;
	INT16 userOnuId = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	userOnuId = onuId - 1;
	
	#if 0
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
       #endif
		vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",slotId, port, onuId) ;		return CMD_WARNING;
		return CMD_WARNING;
       }
	#endif
	
	/*lRet = ShowOnuSWUpdateEnableFlag( phyPonId, onuId);		*/
	lRet = ShowOnuSWUpdateInfoByVty( phyPonId, userOnuId , vty);
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	

   
    return CMD_SUCCESS;
}

DEFUN  (
    onu_show_device_information,
    onu_show_device_information_cmd,
    "show device information",
    DescStringCommonShow
    "Show the onu device information\n"
    "Show the onu device information\n"
    )
{
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	userOnuId = onuId - 1;
	/*ShowOnuDeviceInfo( phyPonId, userOnuId );	*/
	if(SYS_MODULE_IS_GPON(slotId))
	OnuEvent_ShowDeviceInfomation( vty, phyPonId, userOnuId);
	else
	ShowOnuDeviceInfoByVty(phyPonId, userOnuId, vty );
    return CMD_SUCCESS;
}


#if 0
/*added by wutw at 23 september
show onu device information by mac*/
DEFUN  (
    onu_show_device_mac_information,
    onu_show_device_mac_information_cmd,
    "show onu  device information mac <H.H.H>",
    DescStringCommonShow
    "Show the onu information\n"
    "Show the onu device information\n"
    "Show the onu device information\n"
    "Show the onu device information by mac\n"
    "Please input Mac address\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	short int llid = 0;
    CHAR MacAddr[6] = {0,0,0,0,0,0};
	int iTemp = 0;
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
 
	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
	    {
	        vty_out( vty, "  %% Invalid MAC address.\r\n" );
	        return CMD_WARNING;
	    }  

	onuId = GetOnuEntryByMac( MacAddr);
	if((-1) == onuId)
    {
        vty_out( vty, "  %% The mac is not exist.\r\n" );
        return CMD_WARNING;
    }
	
	/*此处onu 的值是0-63*/
	userOnuId = onuId % 64;	
	/*ShowOnuDeviceInfo( phyPonId, userOnuId );	*/	
	ShowOnuDeviceInfoByVty(phyPonId, userOnuId, vty );
    return CMD_SUCCESS;
}


/*show onu device information onuIdx*/
DEFUN  (
    onu_show_device_index_information,
    onu_show_device_index_information_cmd,
    "show onu  device information onuIdx <1-64>",
    DescStringCommonShow
    "Show the onu information\n"
    "Show the onu device information\n"
    "Show the onu device information\n"
    "Show the onu device information by index\n"
    "Please input the onu ID\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;

    CHAR MacAddr[6] = {0,0,0,0,0,0};
	int iTemp = 0;
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	onuId = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;
	userOnuId = onuId %64;

	/*ShowOnuDeviceInfo( phyPonId, userOnuId );	*/
	ShowOnuDeviceInfoByVty(phyPonId, userOnuId, vty );
    return CMD_SUCCESS;
}


/*show onu device information llid*/
DEFUN  (
    onu_show_device_llid_information,
    onu_show_device_llid_information_cmd,
    "show onu  device information llid <1-127>",
    DescStringCommonShow
    "Show the onu information\n"
    "Show the onu device information\n"
    "Show the onu device information\n"
    "Show the onu device information by llid\n"
    "Please input the llid\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	short int llid = 0;
    CHAR MacAddr[6] = {0,0,0,0,0,0};
	int iTemp = 0;
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	llid = (( INT16 ) VOS_AtoL( argv[ 0 ] ));
	onuId = GetOnuIdxByLlid( phyPonId, llid );
	userOnuId = onuId % 64;	

		
	/*ShowOnuDeviceInfo( phyPonId, userOnuId );		*/
	ShowOnuDeviceInfoByVty(phyPonId, userOnuId, vty );
    return CMD_SUCCESS;
}



DEFUN  (
    onu_show_device_name_information,
    onu_show_device_name_information_cmd,
    "show onu  device information name <devicename>",
    DescStringCommonShow
    "Show the onu information\n"
    "Show the onu device information\n"
    "Show the onu device information\n"
    "Show the onu device information by name\n"
    "Please input the device name\n",
    &g_ulIFMQue )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;

    CHAR MacAddr[6] = {0,0,0,0,0,0};
	int iTemp = 0;
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
		
		onuId = GetOnuIdxByName(argv[0] );
		if((-1) == onuId)
		{
			vty_out(vty, "  %% The name is not exist.\r\n");
		}
		userOnuId = onuId % 64;

		
	/*ShowOnuDeviceInfo( phyPonId, userOnuId );	*/
	ShowOnuDeviceInfoByVty(phyPonId, userOnuId, vty );
    return CMD_SUCCESS;
}
#endif

#if 0 
  /* deleted by chenfj 2008-7-18
       these two cli are not registered to onu cli node, so deleted it
       */
/******History statistic*******************/
DEFUN  (
    onu_history_15m_time_interval_config,
    onu_history_15m_time_interval_config_cmd,
    "statistic-history time-interval 15m <bucket>",
    /*"history-statistic time-interval [15|24] <bucket>",*/
    "Set the history statistic attribute\n"
    "Time interval\n"
    "The statistic time-interval is 15 minutes\n"
    "Please input the number of statistic cycles\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    UINT32 timeStat = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

    timeStat = ( UINT16 ) VOS_AtoL( argv[ 0 ] );  
    lRet = HisStats15MinMaxRecordSet(timeStat);

    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
    }	   	
    return CMD_SUCCESS;
}

DEFUN  (
    onu_history_time_interval_config,
    onu_history_24h_time_interval_config_cmd,
    "statistic-history time-interval 24h <bucket>",
    /*"history-statistic time-interval [15|24] <bucket>",*/
    "Set the history statistic attribute\n"
    "Time interval\n"
    "The statistic time-interval is 24 hours\n"
    "Please input the number of statistic cycles\n" )
{
        
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    UINT32 timeStat = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;		
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

    timeStat = ( UINT16 ) VOS_AtoL( argv[ 0 ] );  
    lRet = HisStats24HoursMaxRecordSet(timeStat);
  
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
    }	   	
    return CMD_SUCCESS;
}
#endif


DEFUN  (
    onu_start_statistic_config,
    onu_start_statistic_config_cmd,
    "statistic-history ",
    "Enable the history statistic attribute of the onu\n" )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;	
    INT16 userOnuId = 0;
    ULONG ulIfIndex = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	userOnuId = onuId - 1;	
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}
	#if 0
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, onuId) ;
       #endif
   	   vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",slotId, phyPonId, onuId) ;
		return CMD_WARNING;
       }
	#endif
	lRet = HisStatsOnuStatsStart ( phyPonId,  userOnuId, TRUE);
	if (lRet != CMD_SUCCESS)
	{
		vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
	}
 	
    return CMD_SUCCESS;
}



DEFUN  (
    onu_start_statistic_cycle_config,
    onu_start_statistic_cycle_config_cmd,
    "statistic-history [15m|24h]",
    "Enable the history statistic attribute of the onu\n" 
    "15 minuties\n"
    "24 hours\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;	
    INT16 userOnuId = 0;
    ULONG ulIfIndex = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	userOnuId = onuId - 1;	
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}

	if (!VOS_StrCmp((CHAR *)argv[0], "15m"))
	{
		unsigned int bucketNum15M = 0;
		HisStats15MinMaxRecordGet(&bucketNum15M);
		lRet = HisStatsOnu15MModified( phyPonId, userOnuId, bucketNum15M, TRUE);
	}
	else if (!VOS_StrCmp((CHAR *)argv[0], "24h"))
	{
		unsigned int bucketNum24h = 0;
		HisStats24HoursMaxRecordGet( &bucketNum24h);
		lRet = HisStatsOnu24HModified( phyPonId, userOnuId, bucketNum24h, TRUE);
	}

	if (lRet != CMD_SUCCESS)
	{
		vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
	}
 	
    return CMD_SUCCESS;
}




DEFUN  (
    onu_no_statistic_config,
    onu_no_statistic_config_cmd,
    "undo statistic-history",
    NO_STR
    "Disable the history statistic attribute of the onu\n" )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 userOnuId = 0;		
    INT16 phyPonId = 0;		
    ULONG ulIfIndex = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	userOnuId = onuId - 1;
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}
	#if 0
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId,userOnuId) ;
       #endif
     	   vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",slotId, phyPonId,onuId) ;
		return CMD_WARNING;
       }
	#endif
	
	lRet = HisStatsOnuStatsStart ( phyPonId,  userOnuId, FALSE) ;
     if (lRet != CMD_SUCCESS)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
    }	   	
    return CMD_SUCCESS;
}



DEFUN  (
    onu_no_statistic_cycle_config,
    onu_no_statistic_cycle_config_cmd,
    "undo statistic-history [15m|24h]",
    NO_STR
    "Disable the history statistic attribute of the onu\n" 
    "Disable 15 minutes sycle history statistics\n"
    "Disable 24 hours sycle history statistics\n" 
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 userOnuId = 0;		
    INT16 phyPonId = 0;		
    ULONG ulIfIndex = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	userOnuId = onuId - 1;	
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}
	
	if (!VOS_StrCmp((CHAR *)argv[0], "15m"))
	{
		unsigned int bucketNum15M = 0;
		HisStats15MinMaxRecordGet(&bucketNum15M);
		lRet = HisStatsOnu15MModified( phyPonId, userOnuId, bucketNum15M, FALSE);
	}
	else if (!VOS_StrCmp((CHAR *)argv[0], "24h"))
	{
		unsigned int bucketNum24h = 0;
		HisStats24HoursMaxRecordGet( &bucketNum24h);
		lRet = HisStatsOnu24HModified( phyPonId, userOnuId, bucketNum24h, FALSE);
	}
	
    if (lRet != CMD_SUCCESS)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
    }	   	
    return CMD_SUCCESS;
}

#if 0 
  /* deleted by chenfj 2008-7-18
       these two cli are not registered to onu cli node, so deleted it
       */
DEFUN  (
    onu_show_statistic_bucket,
    onu_show_statistic_bucket_cmd,
    "show statistic-history bucket-num ",
    DescStringCommonShow
    "Show the history statistic\n"
    "Show the time interval\n" )
{
    unsigned int syscle_time24H = 0;
    unsigned int syscle_time15M = 0;
    HisStats24HoursMaxRecordGet( &syscle_time24H );
    HisStats15MinMaxRecordGet( &syscle_time15M );
    vty_out( vty, "\r\n  interval type          bucket number\r\n");
    vty_out( vty, "  15Min                  %d\r\n", syscle_time15M);
    vty_out( vty, "  24h                    %d\r\n\r\n", syscle_time24H);
        
    return CMD_SUCCESS;
}
#endif

/*显示15minute 或者24hour 历史统计*/
DEFUN  (
    onu_show_statistic_24h_data,
    onu_show_statistic_24h_data_cmd,
    "show statistic-history 24h-data <time_length>",
    DescStringCommonShow
    "Show the history statistic\n"
    "Show the history statistic data of 24 hour`s cycle\n"
    "Please input how long the time(hour) of the history statistic is\n")
{
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
	INT16 userOnuId = 0;
    INT16 phyPonId = 0;	
	unsigned int time_len = 0;
	unsigned int bucket_num = 0;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
		
	userOnuId = onuId - 1;	
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}
	#if 0
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
       #endif
		vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",slotId, port, onuId) ;		return CMD_WARNING;
		return CMD_WARNING;
       }
	#endif
	
	time_len = (( unsigned int ) VOS_AtoL( argv[ 0 ] ));
	/*当时间少于24 小时，默认为显示第一个*/
	if(time_len < 24)
		bucket_num = 1;
	/*当时间*/
	else if (time_len%24 == 0)
		bucket_num = time_len/24;
	else 
		bucket_num = time_len/24 +1;
	
	#ifdef CLI_EPON_DEBUG
	vty_out( vty, "  %% bucket_num = %d. argc = %d, time_len = %d\r\n",bucket_num, argc, time_len);
	#endif

	lRet = CliHisStatsOnu24HourDataVty(phyPonId, userOnuId, bucket_num,  vty);

	    if (lRet != VOS_OK)
	    {
	    	vty_out( vty, "  %% 24-hour statistic is disable.\r\n" );
		return CMD_WARNING;
	    }	          
    return CMD_SUCCESS;

		 
}


DEFUN  (
    onu_show_statistic_24hall_data,
    onu_show_statistic_24hall_data_cmd,
    "show statistic-history 24h-data",
    DescStringCommonShow
    "Show the history statistic\n"
    "Show the history statistic all of 24 hour`s cycle\n")
{
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
	INT16 userOnuId = 0;
    INT16 phyPonId = 0;	
	unsigned int bucket_num = 0;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
		
	userOnuId = onuId - 1;
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}
	#if 0
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
       #endif
		vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",slotId, port, onuId) ;		return CMD_WARNING;
		return CMD_WARNING;
       }
	#endif
	
	HisStats24HoursMaxRecordGet(&bucket_num);
	#ifdef CLI_EPON_DEBUG
	vty_out( vty, "  %% bucket_num = %d. argc = %d\r\n",bucket_num, argc);
	#endif	
	lRet = CliHisStatsOnu24HourDataVty(phyPonId, userOnuId, bucket_num,  vty);

	    if (lRet != VOS_OK)
	    {
	    	vty_out( vty, "  %% 24-hour statistic is disable.\r\n" );
		return CMD_WARNING;
	    }	          
    return CMD_SUCCESS;

		 
}


DEFUN  (
    onu_show_statistic_15m_data,
    onu_show_statistic_15m_data_cmd,
    "show statistic-history 15m-data <time_len>",
    DescStringCommonShow
    "Show the history statistic\n"
    "Show the history statistic data of 15 minutes`s cycle\n"
    "Please input how long the time(hours) of the history statistic\n")
{
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
	INT16 userOnuId = 0;
    INT16 phyPonId = 0;	
	unsigned int  time_len = 0;
	unsigned int bucket_num = 0;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
		
	userOnuId = onuId - 1;	
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}
	#if 0
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
       #endif
		vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",slotId, port, onuId) ;		return CMD_WARNING;
		return CMD_WARNING;
       }
	#endif	 


	time_len = (( unsigned int ) VOS_AtoL( argv[ 0 ] ));
	bucket_num = time_len*60/15;
	#ifdef CLI_EPON_DEBUG
	vty_out( vty, "  %% bucket_num = %d. argc = %d, time_len = %d\r\n",bucket_num, argc, time_len);
	#endif
    	lRet = CliHisStatsOnu15MinDataVty(phyPonId,  userOnuId, bucket_num, vty);

	    if (lRet != VOS_OK)
	    {
	    	vty_out( vty, "  %% 15-minute statistic is disable.\r\n" );
		return CMD_WARNING;
	    }	          
    return CMD_SUCCESS;


}

DEFUN  (
    onu_show_statistic_15mall_data,
    onu_show_statistic_15mall_data_cmd,
    "show statistic-history 15m-data",
    DescStringCommonShow
    "Show the history statistic\n"
    "Show the history statistic all of 15 minutes`s cycle\n" )
{
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
	INT16 userOnuId = 0;
    INT16 phyPonId = 0;	
	unsigned int bucket_num = 0;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
		
	userOnuId = onuId - 1;	
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}
	#if 0
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
       #endif
		vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",slotId, port, onuId) ;		return CMD_WARNING;
		return CMD_WARNING;
       }
	#endif 
	HisStats15MinMaxRecordGet( &bucket_num );
	
	#ifdef CLI_EPON_DEBUG
	vty_out( vty, "  %% bucket_num = %d. argc = %d\r\n",bucket_num, argc);
	#endif
	
    	lRet = CliHisStatsOnu15MinDataVty(phyPonId,  userOnuId, bucket_num, vty);

	    if (lRet != VOS_OK)
	    {
	    	vty_out( vty, "  %% 15-minute statistic is disable.\r\n" );
		return CMD_WARNING;
	    }	          
    return CMD_SUCCESS;


}


DEFUN  (
    onu_show_statistic,
    onu_show_statistic_cmd,
    "show statistic-history",
    DescStringCommonShow
    "Show the history statistic\n"
    )
{
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
	INT16 userOnuId = 0;
    INT16 phyPonId = 0;	
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
		
	userOnuId = onuId - 1;
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}
	lRet = CliHisStatsONUCtrlGet(phyPonId, userOnuId, vty);
	if (lRet != VOS_OK)
	{
	   	vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
	}	
	
    return CMD_SUCCESS;

		 
}

DEFUN  (
    onu_clear_statistic_data,
    onu_clear_statistic_data_cmd,
    "clear statistic-history [15m|24h]",
    NO_STR
    "Clear the history statistic data\n"
    "Clear the history statistic data of 15 minute\n"
    "Clear the history statistic data of 24 hour\n" )
{
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
	INT16 userOnuId = 0;		
    INT16 phyPonId = 0;	

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	userOnuId = onuId - 1;
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",slotId, port, onuId);
		return CMD_WARNING;	
		}
	#if 0
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, onuId) ;
       #endif
		vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",slotId, port, onuId) ;		return CMD_WARNING;
		return CMD_WARNING;
       }
	#endif
	
	if (!VOS_StrCmp((CHAR *)argv[0], "15m"))
		 lRet = HisStatsOnu15MinRawClear(phyPonId, userOnuId) ;
	else if (!VOS_StrCmp((CHAR *)argv[0], "24h"))
		lRet = HisStatsOnu24HourRawClear(phyPonId, userOnuId);

     if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	         
    return CMD_SUCCESS;
}


/*begin : add by zhengyt@09-06-03,clear onu real time data statistic*/
DEFUN (
    clear_onu_real_time_statistic_data,
    clear_onu_real_time_statistic_data_cmd,
    "clear onu statistic",
    NO_STR
    "onu statistic\n"
    "Clear the real time statistic data\n"
    )
{
	ULONG ulIfIndex = 0;
    ULONG ulSlot = 0;
    ULONG ulPort = 0;
    ULONG ulOnuId = 0; 
    INT16 userOnuId = 0;		
    INT16 phyPonId = 0;	


    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
    
	if( checkVtyOnuIsValid(vty, ulSlot, ulPort, ulOnuId, &phyPonId, &userOnuId) == VOS_ERROR )
		return CMD_WARNING;

	if(ClearOnuPonPortStatisticCounter(phyPonId, userOnuId)==VOS_OK)
		return CMD_SUCCESS;

	return CMD_WARNING;	
}
/*end*/

#if 0 
  /* deleted by chenfj 2008-7-18
       these two cli are not registered to onu cli node, so deleted it
       */
/****ALARM****************/
/*已被注释*/
DEFUN  (
    onu_alarm_config,
    onu_alarm_config_cmd,
    "alarm <alarm_name> [enable|disable]",
    "Set alarm\n"
    "Alarm name in the system\n"
    "Enable/disable the alarm mask\n"
    "Disable/disable the alarm mask\n")
{
        
    return CMD_SUCCESS;
}

/*已被注释*/
DEFUN  (
    onu_alarm_threshold_config,
    onu_alarm_threshold_config_cmd,
    "alarm <alarm_name> threshold <threshold_value>",
    "Set alarm\n"
    "Please input the alarm name in the system\n"
    "Set the alarm threshold\n"
    "Please input the threshold threshold-value\n" )
{
        
    return CMD_SUCCESS;
}


/*已被注释，该cli 属于dayacli命令部分*/
DEFUN  (
    onu_show_alarm_info,
    onu_show_alarm_info_cmd,
    "show alarm <alarm_name> information",
    DescStringCommonShow
    "Show alarm information\n"
    "Please input the alarm name in the system\n"
    "Show alarm information\n" )
{
        
    return CMD_SUCCESS;
}
#endif
/*******LOOP**************************/
/* modified by chenfj 2007-11-12
      问题单#5753 建议将目前OLT中的pon-loop source outer命令屏蔽
      */


#ifdef ONU_MAC_LEARING_
#endif
DEFUN  (
    onu_show_fdb_mac_config,
    onu_show_fdb_mac_config_cmd,
    "show fdbentry mac",
    DescStringCommonShow
    "Show fdbentry \n"
    "Show fdbentry MAC address\n"
    )
{
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    ULONG ulIfIndex = 0;	
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
	long  entryNum=0;
	PON_onu_address_table_record_t  *address_table;
	int i;
	LONG lRet = VOS_OK;
		
    ulIfIndex = ( ULONG ) ( vty->index ) ;	

    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;

	if( checkVtyOnuIsValid(vty, slotId, port, onuId, &phyPonId, &userOnuId) == VOS_ERROR )
		return CMD_WARNING;

	/*lRet = ShowPAS6201MacLearningByVty( phyPonId, onuId-1 , vty );*/
#if 1
       address_table = MAC_Address_Table;
	lRet=OnuMgt_GetOnuMacAddrTbl(phyPonId, userOnuId, &entryNum, address_table);
	if(lRet==0)
		{
		vty_out(vty, "\r\n  onu %d/%d/%d mac entry list\r\n", GetCardIdxByPonChip(phyPonId), GetPonPortByPonChip(phyPonId), onuId);
		if( entryNum == 0) 
			{
			vty_out(vty, "  total Learned mac counter=0\r\n");
			return( ROK );
			}
		vty_out(vty, "  total Learned mac counter=%d\r\n", entryNum );
		vty_out(vty,  "    macAddr     type   dataPath    agingTime\r\n");
		vty_out(vty,  " --------------------------------------------\r\n");
		
		for(i=0;i< entryNum;i++)
			{
			vty_out(vty, " %02x%02x.%02x%02x.%02x%02x", address_table->addr[0],address_table->addr[1],address_table->addr[2],address_table->addr[3],address_table->addr[4],address_table->addr[5]);

			if( address_table->type == ADDR_DYNAMIC )
				vty_out(vty, "  D     ");
			else if( address_table->type == ADDR_STATIC )
				vty_out(vty, "  S     " );
			else if( address_table->type == ADDR_DYNAMIC_AND_STATIC )
				vty_out(vty, "  D|S   ");
			else vty_out(vty, "  N/A   ");

			if(address_table->action == PON_DONT_PASS )
				vty_out(vty, "no path     ");
			else if( address_table->action == PON_PASS_DATAPATH ) vty_out(vty, "  data      " );
			else if( address_table->action == PON_PASS_CPU ) vty_out(vty, "  cpu       ");
			else if( address_table->action == PON_PASS_BOTH ) vty_out(vty, "cpu&data    ");
			
			vty_out(vty, "  %d\r\n", address_table->age );
			
			address_table++;
			}
		vty_out( vty,"\r\n");
		/* vty_out(vty, "    total Learned mac counter=%d\r\n", EntryNum ); */
		}
	else 
		{
		vty_out(vty, "    total Learned mac counter=0\r\n");
		return( ROK );
		}
#endif
	/*lRet = ShowPonMacLearningByVty( phyPonId, vty );*/
	if (CMD_SUCCESS != lRet)
		vty_out(vty, "  %% Executing error.\r\n");
	/*ShowPonPortOnLineOnuByVty( phyPonId, vty );*/
    return CMD_SUCCESS;
		
}

DEFUN  (
    onu_show_fdb_mac_counter,
    onu_show_fdb_mac_counter_cmd,
    "show fdbentry mac counter",
    DescStringCommonShow
    "Show fdbentry \n"
    "Show fdbentry MAC address\n"
    "show mac learned counter\n"
    )
{
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    ULONG ulIfIndex = 0;	
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
	long  entryNum=0;
	LONG lRet = VOS_OK;
		
    ulIfIndex = ( ULONG ) ( vty->index ) ;	

    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;

	if( checkVtyOnuIsValid(vty, slotId, port, onuId, &phyPonId, &userOnuId) == VOS_ERROR )
		return CMD_WARNING;

	/*lRet = ShowPAS6201MacLearningCounterByVty( phyPonId, onuId-1 , vty );*/
	
	lRet = OnuMgt_GetOnuMacAddrTbl(phyPonId, userOnuId, &entryNum, NULL);
	if( 0 == lRet )
	{
		vty_out(vty, "\r\n  onu %d/%d/%d learned mac counter=%d\r\n\r\n", GetCardIdxByPonChip(phyPonId), GetPonPortByPonChip(phyPonId), (onuId+1), entryNum);
	}
	/*lRet = ShowPonMacLearningByVty( phyPonId, vty );*/
	if (CMD_SUCCESS != lRet)
		vty_out(vty, "  %% Executing error.\r\n");
	/*ShowPonPortOnLineOnuByVty( phyPonId, vty );*/
    return CMD_SUCCESS;
		
}

#if 0
/********Config the onu mac*****************/

DEFUN  (
    onu_create_fdb,
    onu_create_fdb_cmd,
    "fdbentry mac <H.H.H>",
    "Create a permanent FDB entry \n"
    "MAC address \n"
    "Please input MAC address \n",
    &g_ulIFMQue )
{
        
    return CMD_SUCCESS;
}

DEFUN  (
    onu_show_mac_information,
    onu_show_mac_information_cmd,
    "show onu mac",
    DescStringCommonShow
    "Show onu information\n"
    "Show onu mac information\n",
    &g_ulIFMQue )
{
        
    return CMD_SUCCESS;
}


/*added by wutw at 20 september*/
DEFUN  (
    onu_show_fdb_llid_config,
    onu_show_fdb_llid_config_cmd,
    "show fdb logical-link <1-8>",
    DescStringCommonShow
    "Fdb mac information\n"
    "Logical link\n"
    "Please input the OnuId\n"
    "Please input the LLID of onu\n",
    &g_ulIFMQue )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;	
    UINT16 userOnuId = 0;
    INT16   llidIndex = 0;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	

    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    /*if (VOS_OK != PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&ponId , (ULONG *)&onuId))
	return CMD_WARNING; */
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	
    userOnuId = onuId - 1;
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
    {
	vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",phyPonId, userOnuId) ;
	return CMD_WARNING;
    }
    else if (VOS_ERROR == lRet)
    {
       vty_out( vty, "  %% Parameter is error.\r\n" );
	 return CMD_WARNING;
    }	
	
    llidIndex = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;
    if (CLI_EPON_DEFAULTLLID != llidIndex)
	{
	   vty_out( vty, "  %% Parameter is error. llid must be 1\r\n");
	   return CMD_WARNING;
	}	

    ShowLlidMacInfo( (short int )phyPonId, (short int )userOnuId, (short int )llidIndex );
	
    return CMD_SUCCESS;
}

#endif


#ifdef ONU_PEER_TO_PEER	
#define EPON_P2P_UNICAST_ENABLE 	1
#define EPON_P2P_UNICAST_DISABLE	2
#define EPON_P2P_BRDCAST_ENABLE		1
#define EPON_P2P_BRDCAST_DISABLE	2

/* modified by chenfj  2007/04/26 
*         问题单#4299: P2P第二个命令有待改进,将广播包转发功能隐含到配置p2p链接的命令中去， 而未知单播包的转发则可以另外配置
*/
DEFUN(
	peer_to_peer_rule, 
	peer_to_peer_rule_cmd,
	"p2p forward address-not-found [enable|disable]",
	/*"p2p forward address-not-found [enable|disable] broadcast [enable|disable]", */
	"Config onu peer to peer attribute\n"
	"Config onu peer to peer forward rule\n"
	"Config address-not-found frame transfers\n"
	"Enable address-not-found frame transfers\n"
	"Disable address-not-fount frame transfers\n"
	/*
	"Config broadcast frame transfers\n"
	"Enable broadcast frame transfers\n"
	"Disable broadcast frame transfers\n"
	*/
	)
{
	LONG lRet = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
	int unicastFlag = 0;
	int brdFlag = 0;
    ulIfIndex = ( ULONG ) ( vty->index ) ;	

	lRet = PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&ulOnuId);	
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
	    vty_out( vty, "  %% Executing error\r\n");
	    return CMD_WARNING;
	}
	#if 0
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, (ulOnuId-1));
    if ( CLI_EPON_ONUDOWN == lRet)
    {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, ulOnuId-1) ;
       #endif
		vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",ulSlot, ulPort, ulOnuId) ;	
		return CMD_WARNING;
    }
	#endif
	if (VOS_OK != cliCheckOnuMacValid_onuNode( phyPonId, (ulOnuId-1)))
    {
		vty_out( vty, "  %% onu%d/%d/%d is not exist.\r\n",ulSlot, ulPort, ulOnuId) ;	
		return CMD_WARNING;
    }
	if (!VOS_StrCmp((CHAR *)argv[0], "enable"))
		unicastFlag = EPON_P2P_UNICAST_ENABLE;
	else /*if (!VOS_StrCmp((CHAR *)argv[0], "disable"))*/
		unicastFlag = EPON_P2P_UNICAST_DISABLE;
	/*else 
	{
		vty_out( vty, "  %% Parameter error!\r\n");
		return CMD_WARNING;
	}*/

	brdFlag = EPON_P2P_BRDCAST_ENABLE;
	
	lRet = (LONG)SetOnuPeerToPeerForward( phyPonId, (short int)(ulOnuId-1), unicastFlag, brdFlag );
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}		
	return CMD_SUCCESS;
}


DEFUN(
	peer_to_peer_rule_show, 
	peer_to_peer_rule_show_cmd,
	"show p2p forward rule",
	DescStringCommonShow
	"Show onu peer to peer information\n"
	"Show onu peer to peer forward rule\n"
	"Config address-not-fount frame transfers\n"
	)
{
	LONG lRet = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
	int unicastFlag = 0;
	int brdFlag = 0;
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
	lRet = PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&ulOnuId);	
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
	    vty_out( vty, "  %% Executing error\r\n");
	    return CMD_WARNING;
	}
	#if 0
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, (ulOnuId-1));
    if ( CLI_EPON_ONUDOWN == lRet)
    {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, ulOnuId-1) ;
       #endif
		vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",ulSlot, ulPort, ulOnuId) ;	
		return CMD_WARNING;
    }
	#endif
	if (VOS_OK != cliCheckOnuMacValid_onuNode( phyPonId, (ulOnuId-1)))
    {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, ulOnuId-1) ;
       #endif
		vty_out( vty, "  %% onu %d/%d/%d is not exist.\r\n",ulSlot, ulPort, ulOnuId) ;	
		return CMD_WARNING;
    }
	lRet = GetOnuPeerToPeerForward( phyPonId, ulOnuId-1, &unicastFlag , &brdFlag );
	if (VOS_ERROR == lRet)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}

	vty_out( vty, "\r\n  Peer-to-peer forward rule:\r\n");
	vty_out( vty,"  address_not_found:");
	if(unicastFlag == EPON_P2P_UNICAST_ENABLE)
		vty_out( vty,"forward\r\n");
	else if (unicastFlag == EPON_P2P_UNICAST_DISABLE)
		vty_out( vty,"discard\r\n");
	else
		vty_out( vty, "unknown\r\n");
	
	vty_out( vty,"  broadcast        :");
	if(brdFlag == EPON_P2P_BRDCAST_ENABLE)
		vty_out( vty,"forward\r\n");
	else if (brdFlag == EPON_P2P_BRDCAST_DISABLE)
		vty_out( vty,"discard\r\n");
	else
		vty_out( vty, "unknown\r\n");	
	
	return CMD_SUCCESS;
}
#endif

DEFUN(
	onu_statistics_show, 
	onu_statistics_show_cmd,
	"show statistic [pon|sni]",
	DescStringCommonShow
	"Show onu statstics information\n"
	"Show pon statstics\n"
	"Show sni statistics\n"
	)
{
	LONG lRet = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
	short int llid = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
	lRet = PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&ulOnuId);	
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}

	if( checkVtyOnuIsValid(vty, ulSlot, ulPort, ulOnuId, &phyPonId, &userOnuId) == VOS_ERROR )
		return CMD_WARNING;

	llid = GetLlidByOnuIdx( phyPonId, (unsigned short)(ulOnuId-1));
	if (INVALID_LLID == llid ) 
	{
		vty_out( vty, "  %% %d/%d/%d is off-line\r\n",ulSlot, ulPort, ulOnuId) ;
		return CMD_WARNING;
	}
	if (!VOS_StrCmp((CHAR *)argv[0], "pon"))
		lRet = CliRealTimeOnuStatsPon( phyPonId, userOnuId, vty );
	else /*if (!VOS_StrCmp((CHAR *)argv[0], "sni"))*/
		lRet = CliRealTimeOnuStatsCNI( phyPonId, userOnuId, vty );
	
	if ( VOS_OK != lRet)
	{	
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}

#if 0	/* removed by xieshl 20100806 */
DEFUN(
	onu_olt_downlink_ber_show, 
	onu_olt_downlink_ber_show_cmd,
	"show olt-downlink ber",
	DescStringCommonShow
	"Show olt downlink information\n"
	"Show olt downlink ber\n"
	)
{
	LONG lRet = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
	short int llid = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
	lRet = PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&ulOnuId);	
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
	    vty_out( vty, "  %% Executing error\r\n");
	    return CMD_WARNING;
	}

	if(ThisIsValidOnu( phyPonId, (unsigned short)(ulOnuId-1)) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuId);
		return CMD_WARNING;	
		}
	llid = GetLlidByOnuIdx( phyPonId, (unsigned short)(ulOnuId-1));
	if (INVALID_LLID == llid ) 
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort, ulOnuId) ;
		return CMD_WARNING;
	}

	lRet = CliRealTimeOltDownlinkBer( phyPonId, (short int)(ulOnuId-1), vty );
	
	if ( VOS_OK != lRet)
	{	
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}
#endif




#ifdef  ONU_MAX_MAC_AND_FEC
#endif

static int set_onu_max_mac_cli( struct vty *vty, UINT32 number )
{
	LONG lRet = VOS_OK;
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	INT16   llidIndex = 0;
	short int PonChipType;

	ULONG ulIfIndex = ( ULONG ) ( vty->index ) ;	
	
	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	PonChipType = GetPonChipTypeByPonPort( phyPonId );

	llidIndex = CLI_EPON_DEFAULTLLID;

	if(PonChipType == PONCHIP_PAS5001 )
		vty_out(vty,"Note:this is PAS5001,setting limitation of address learned by an onu,the MAC table can lose entries after long runs of traffic and frequent aging,and also possible Data path loss\r\n");
	
	
	userOnuId = (short int)(ulOnuId - 1);
	/*lRet = SetOnuMaxMacNum((short int )phyPonId, (short int)userOnuId, llidIndex, number);*/
	lRet=OnuMgt_SetOnuMaxMac((short int )phyPonId, (short int)userOnuId, llidIndex, &number);
	if ( lRet == V2R1_ONU_NOT_EXIST)
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
	}
	if( lRet == RERROR ) 
	{
		vty_out( vty, "  %% set onu%d/%d/%d max mac err\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
	}
		
	return CMD_SUCCESS;
}

/* the following cli cmd is added by chenfj  2007-5-23 */
DEFUN  (
    onu_maxmac_config,
    onu_maxmac_config_cmd,
    "onu max-mac [0|<1-8192>]",
    "Config onu info.\n"
    "Config the max-mac number supported by per onu\n"
    "0 indicate no mac-learning limit\n"
    "Please input the max-mac number supported, default is 128\n"
    )
{
#if 0	/* modified by xieshl 20100804 */
	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	INT16   llidIndex = 0;
	UINT32 number = 0;
	short int PonChipType;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	number = ( ULONG ) VOS_AtoL( argv[ 0 ] );
	
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
    #endif      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	PonChipType = GetPonChipTypeByPonPort( phyPonId );
#if 0
	if( PonChipType == PONCHIP_PAS5001 )
		{
		if(( number != 0)&&( number != 1) && ( number != 2)&& ( number !=4) && ( number != 8)
			&& ( number != 16) && ( number != 32) && ( number != 64 ))
			{
			vty_out(vty, "  %% max macmax mac number should be 0,1,2,4,8,16,32,64\r\n");
			return( CMD_WARNING );	
			}
		}
	else if( PonChipType == PONCHIP_PAS5201 )
		{

		}
		

	/* PAS5001不支持此设置*/
	if( V2R1_GetPonchipType( phyPonId ) == PONCHIP_PAS5001 )
		{
		vty_out( vty, "  %% this pon chip is PAS5001, max mac number is not supported\r\n");
		return( CMD_SUCCESS );
		}
	
	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
	if( V2R1_GetPonchipType( phyPonId ) != PONCHIP_PAS5201 )
		{
		vty_out( vty, "  %%max mac number is not supported\r\n");
		return( CMD_WARNING );
		}
#endif

	llidIndex = CLI_EPON_DEFAULTLLID;

	if(PonChipType == PONCHIP_PAS5001 )
		vty_out(vty,"Note:this is PAS5001,setting limitation of address learned by an onu,the MAC table can lose entries after long runs of traffic and frequent aging,and also possible Data path loss\r\n");
	
	userOnuId = (short int)(ulOnuId - 1);
	lRet = SetOnuMaxMacNum((short int )phyPonId, (short int)userOnuId, llidIndex, number);
	if ( lRet == V2R1_ONU_NOT_EXIST)
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}
	if( lRet == RERROR ) 
		{
		vty_out( vty, "  %% set onu%d/%d/%d max mac err\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}
		
    return CMD_SUCCESS;
#else
	return set_onu_max_mac_cli( vty, VOS_AtoL(argv[0])|ONU_NOT_DEFAULT_MAX_MAC_FLAG );
#endif
}

DEFUN  (
    undo_onu_maxmac_config,
    undo_onu_maxmac_config_cmd,
    "undo onu max-mac",
    "clear config\n"
    "clear the current max-mac config\n"
    "restore the default value(128)\n"
    )
{
#if 0	/* modified by xieshl 20100804 */
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;	
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG ulOnuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	
    INT16   llidIndex = 0;
	unsigned int number = MaxMACDefault;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	

    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
    #endif      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
#if 0
	/* PAS5001不支持此设置*/
	if( V2R1_GetPonchipType( phyPonId ) == PONCHIP_PAS5001 )
		{
		vty_out( vty, "  %% this pon chip is PAS5001, max mac number is not supported\r\n");
		return( CMD_SUCCESS );
		}
	
	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
	if( V2R1_GetPonchipType( phyPonId ) != PONCHIP_PAS5201 )
		{
		vty_out( vty, "  %%max mac number is not supported\r\n");
		return( CMD_WARNING );
		}
#endif
	llidIndex = CLI_EPON_DEFAULTLLID;

	userOnuId = (short int)(ulOnuId - 1);
	if(GetPonChipTypeByPonPort(phyPonId) == PONCHIP_PAS5001 )
		vty_out(vty,"Note:this is PAS5001,setting limitation of address learned by an onu,the MAC table can lose entries after long runs of traffic and frequent aging,and also possible Data path loss\r\n");
	lRet = SetOnuMaxMacNum((short int )phyPonId, (short int)userOnuId, llidIndex, number);
	if ( lRet == V2R1_ONU_NOT_EXIST)
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}
	if( lRet == RERROR ) 
		{
		vty_out( vty, "  %% set onu%d/%d/%d max mac err\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}	

    return CMD_SUCCESS;
#else
	return set_onu_max_mac_cli( vty, MaxMACDefault|ONU_UNDO_MAX_MAC_FLAG );
#endif
}

DEFUN  (
    onu_maxmac_show,
    onu_maxmac_show_cmd,
    "show onu max-mac",
    DescStringCommonShow
    "Show the max-mac number\n"
    "Show the max-mac number\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;	
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
    UINT32 number = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	

    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
    #endif      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
#if 0
	/* PAS5001不支持此设置*/
	if( V2R1_GetPonchipType( phyPonId ) == PONCHIP_PAS5001 )
		{
		vty_out( vty, "  %% this pon chip is PAS5001, max mac number is not supported\r\n");
		return( CMD_SUCCESS );
		}
	
	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
	if( V2R1_GetPonchipType( phyPonId ) != PONCHIP_PAS5201 )
		{
		vty_out( vty, "  %%max mac number is not supported\r\n");
		return( CMD_WARNING );
		}
#endif

	userOnuId = (short int)(onuId - 1);
	lRet = GetOnuMaxMacNum((short int )phyPonId, (short int)userOnuId, &number);
	if ( lRet == V2R1_ONU_NOT_EXIST)
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,onuId);
		return CMD_WARNING;  
		}
	if( lRet == RERROR ) 
		{
		vty_out( vty, "  %% get onu%d/%d/%d max mac err\r\n",slotId,port,onuId);
		return CMD_WARNING;  
		}
	vty_out( vty, "   onu%d/%d/%d supported max-mac number is %d\r\n",slotId,port,onuId,number);
				
    return CMD_SUCCESS;
}

#ifdef   ONU_FEC_CONFIG
#endif

DEFUN  (
    onu__fec_config,
    onu__fec_config_cmd,
    "fec-mode [enable|disable]",
    "config onu fec mode\n" 
    "enable onu fec\n"
    "disable onu fec\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0, suffix;	
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG ulOnuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	int fec_mode = STD_FEC_MODE_ENABLED; /*CLI_EPON_ALARM_ENABLE + 1;*/

    if( VOS_StrCmp(argv[0], "enable") == 0 )
        fec_mode = STD_FEC_MODE_ENABLED;
    else /*if( VOS_StrCmp(argv[0], "disable") == 0 )*/
        fec_mode = STD_FEC_MODE_DISABLED;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {

    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
    #endif      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	/* PAS5001不支持此设置*/
	if( V2R1_GetPonchipType( phyPonId ) == PONCHIP_PAS5001 )
		{
		vty_out( vty, "  FEC is Unsupported\r\n" );
		/*vty_out( vty, "  %% this pon chip is PAS5001, FEC is not supported\r\n");*/
		return( CMD_SUCCESS );
		}
	
#if 0
	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
	if( V2R1_GetPonchipType( phyPonId ) != PONCHIP_PAS5201 )
		{
		vty_out( vty, "  %% FEC is not supported\r\n");
		return( CMD_WARNING );
		}
#endif


	/*else{
		vty_out(vty, "  %% param err\r\n");
		return( CMD_WARNING );
		}*/

	userOnuId = (short int)(ulOnuId - 1);
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}

	/* 此处应增加对ONU 类型的判断，只有PAS6301 ONU 才能支持FEC 设置*/
	if( GetOnuVendorType(phyPonId, userOnuId) ==  ONU_VENDOR_GW)
		{
/*
		short int ChipId;
		lRet = GetOnuDeviceChipId( phyPonId, (short int)userOnuId, &ChipId);
		if( lRet != ROK ) 
			{
			vty_out( vty, "  %% Executing err\r\n");
			return( CMD_WARNING );
			}
		if(ChipId == ONUCHIP_PAS6201 )
			{
			vty_out(vty, "  %% fec is not supported\r\n");
			return( CMD_WARNING );
			}
*/			
		lRet = SetOnuFecMode((short int )phyPonId, (short int)userOnuId, fec_mode );
		if ( lRet == V2R1_ONU_NOT_EXIST )
			{
			vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}
		if( lRet == V2R1_ONU_OFF_LINE ) 
			{
			vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}
		if( lRet == RERROR ) 
			{
#if 0                
			vty_out( vty, "  %% set onu%d/%d/%d FEC err\r\n",slotId,port,ulOnuId);
#else
            vty_out(vty, ONU_CMD_ERROR_STR);
#endif
			return CMD_WARNING;  
			}
		}
	else {
		lRet = CTC_SetLlidFecMode( phyPonId,userOnuId, fec_mode); 
		if ( lRet == V2R1_ONU_NOT_EXIST )
			{
			vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}
		if( lRet == V2R1_ONU_OFF_LINE ) 
			{
			vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}
		if( lRet == RERROR ) 
			{
#if 0                
			vty_out( vty, "  %% set onu%d/%d/%d FEC err\r\n",slotId,port,ulOnuId);
#else
            vty_out(vty, ONU_CMD_ERROR_STR);
#endif
			return CMD_WARNING;  
			}
		}
    }
    else
    {
        fec_mode = fec_mode == STD_FEC_MODE_ENABLED?1:0;
        if(setOnuConfSimpleVarByPtr(suffix, vty->onuconfptr, sv_enum_onu_fec_enable, fec_mode) != VOS_OK)
        {
            vty_out(vty,"set onu FEC error!\r\n");
            return CMD_WARNING;
        }
    }
	
	return CMD_SUCCESS;
}


DEFUN  (
    show_onu__fec_config,
    show_onu__fec_config_cmd,
    "show fec-mode",
    "show onu fec mode\n"
    "show onu fec mode\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;	
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG ulOnuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	int fec_mode = STD_FEC_MODE_UNKNOWN ;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	

    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
    #endif      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	/* PAS5001不支持此设置*/
	if( V2R1_GetPonchipType( phyPonId ) == PONCHIP_PAS5001 )
		{
		vty_out( vty, "  FEC is Unsupported\r\n" );
		/*vty_out( vty, "  %% this pon chip is PAS5001, FEC is not supported\r\n");*/
		return( CMD_SUCCESS );
		}
	
#if 0
	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
	if( V2R1_GetPonchipType( phyPonId ) != PONCHIP_PAS5201 )
		{
		vty_out( vty, "  %% FEC is not supported\r\n");
		return( CMD_WARNING );
		}
#endif

	userOnuId = (short int)(ulOnuId - 1);
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}

	/* 此处应增加对ONU 类型的判断，只有PAS6301 ONU 才能支持此设置*/
	if( GetOnuOperStatus(phyPonId, userOnuId )  != ONU_OPER_STATUS_UP )
		{
		vty_out( vty, "FEC Mode : Unknown\r\n" );
		return CMD_WARNING ;
		}
			
	if( GetOnuVendorType(phyPonId, userOnuId) ==  ONU_VENDOR_GW)
		{
		lRet = GetOnuFecMode((short int )phyPonId, (short int)userOnuId, &fec_mode );
		if ( lRet == V2R1_ONU_NOT_EXIST )
			{
			vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}
		if( lRet == RERROR ) 
			{
			vty_out( vty, "  %% get onu%d/%d/%d FEC err\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}
		}
	
	else{
		lRet = CTC_GetLlidFecMode( phyPonId, userOnuId, &fec_mode);
		if ( lRet == V2R1_ONU_NOT_EXIST )
			{
			vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}
		if( lRet == V2R1_ONU_OFF_LINE )
			{
			vty_out(vty,"  %% onu %d/%d/%d is off-line\r\n",slotId,port,ulOnuId);
			return( CMD_WARNING );
			}
		if( lRet == RERROR ) 
			{
			vty_out( vty, "  %% get onu%d/%d/%d FEC err\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}
		}

	vty_out( vty, "FEC Mode : " );
	if( fec_mode == STD_FEC_MODE_UNKNOWN )
		vty_out( vty, "Unknown\r\n" );
	else if( fec_mode == STD_FEC_MODE_DISABLED )
		vty_out( vty, "Disable\r\n" );
	else if( fec_mode == STD_FEC_MODE_ENABLED)
		vty_out( vty, "Enable\r\n" );
	else
		vty_out( vty, "Invalid(%d)\r\n", fec_mode );

    return CMD_SUCCESS;
}

DEFUN(
	show_fec__ability,
	show_fec__ability_cmd,
	"show fec-ability",
	SHOW_STR
	"Display fec ability\n"
	)
{
	
	PON_olt_id_t	olt_id;
	unsigned long ulSlot, ulPort, ulOnu;
	PON_onu_id_t	onu_id;
	short int		return_result;
	CTC_STACK_standard_FEC_ability_t	fec_ability;
	short int PonChipType;

	if( PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnu ) != VOS_OK )
		{
		vty_out( vty, "  PON_GetSlotPortOnu error\r\n");
    		return CMD_WARNING;
		}
	
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (olt_id == (-1) )
		{
		vty_out( vty, "  GetPonPortIdxBySlot error\r\n");
    		return CMD_WARNING;
		}

	PonChipType = V2R1_GetPonchipType( olt_id );
	/* PAS5001不支持此设置*/
	if( PonChipType == PONCHIP_PAS5001 )
		{
		vty_out( vty, "  FEC is Unsupported\r\n" );
		/*vty_out( vty, "  %% this pon chip is PAS5001, FEC is not supported\r\n");*/
		return( CMD_SUCCESS );
		}
	
#if 0
	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
	if( PonChipType != PONCHIP_PAS5201 )
		{
		vty_out( vty, "  %% FEC is not supported\r\n");
		return( CMD_WARNING );
		}
#endif

	if(ThisIsValidOnu(olt_id, (ulOnu-1) ) != ROK )
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot,ulPort,ulOnu);
		return CMD_WARNING;  
		}
	
	if( GetOnuOperStatus(olt_id, (ulOnu-1) )  != ONU_OPER_STATUS_UP )
		{
		vty_out( vty, "  FEC Ability : Unknown\r\n" );
		return CMD_WARNING;
		}
	/*
	if( GetOnuOperStatus(olt_id, (ulOnu-1)) != 1 )
		{
		vty_out( vty, "  %% onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnu);
    		return CMD_WARNING;
		}
	*/
	onu_id = GetLlidByOnuIdx( olt_id, (ulOnu-1));
	if( onu_id == INVALID_LLID) return( CMD_WARNING );

	/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	if(OLT_PONCHIP_ISPAS(PonChipType)&& (!OLT_PONCHIP_ISPAS5001(PonChipType)) && ( GetOnuVendorType(olt_id, (ulOnu-1)) ==  ONU_VENDOR_GW))
		{
		vty_out( vty, "  FEC Ability : Supported\r\n" );
		return( CMD_SUCCESS );
		}
	/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	if(OLT_PONCHIP_ISPAS(PonChipType)&& (!OLT_PONCHIP_ISPAS5001(PonChipType)) && ( GetOnuVendorType(olt_id, (ulOnu-1)) ==  ONU_VENDOR_CT))
		{	
		#if 1
		return_result = OnuMgt_GetFecAbility(olt_id, ulOnu-1, &fec_ability);
		#else
		return_result = CTC_STACK_get_fec_ability(olt_id, onu_id, &fec_ability);
		#endif
		if ( return_result == CTC_STACK_EXIT_OK )
		{
			vty_out( vty, "  FEC Ability : " );
			if( fec_ability == STD_FEC_ABILITY_UNKNOWN )
				vty_out( vty, "Unknown\r\n" );
			else if( fec_ability == STD_FEC_ABILITY_SUPPORTED )
				vty_out( vty, "Supported\r\n" );
			else if( fec_ability == STD_FEC_ABILITY_UNSUPPORTED )
				vty_out( vty, "Unsupported\r\n" );
			else	
				vty_out( vty, "Invalid(%d)\r\n", fec_ability );

			return CMD_SUCCESS;
		}
		else
			vty_out( vty, "%% Executing error\r\n" );
		}

	return CMD_WARNING;
}

#ifdef  ONU_PON_LOOPBACK 
#endif

/* added by xieshl 20120828, 电科院测试 */
DEFUN  (
    onu_pon_loopback_config,
    onu_pon_loopback_config_cmd,
    "pon-loopback [enable|disable]",
    "config onu pon loop mode\n" 
    "enable onu pon loopback\n"
    "disable onu pon loopback\n"
    )
{
	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int enable = V2R1_DISABLE;

	if( VOS_StrCmp(argv[0], "enable") == 0 )
		enable = V2R1_ENABLE;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );

	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	/* 因适配层没有做，PAS5001暂不支持，后续考虑增加*/
	if( V2R1_GetPonchipType( phyPonId ) == PONCHIP_PAS5001 )
	{
		vty_out( vty, "  PAS5001 is Unsupported\r\n" );
		return( CMD_SUCCESS );
	}
	
	userOnuId = (short int)(ulOnuId - 1);
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
	}

	/* 此处应增加对ONU 类型的判断，PMC ONU和CTC ONU应区分处理*/
	/*if( GetOnuVendorType(phyPonId, userOnuId) ==  ONU_VENDOR_GW)*/
	{
		lRet = SetOnuPonLoopbackEnable((short int )phyPonId, (short int)userOnuId, enable );
		if ( lRet == V2R1_ONU_NOT_EXIST )
		{
			vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
		}
		if( lRet == V2R1_ONU_OFF_LINE ) 
		{
			vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
		}
		if( lRet == RERROR ) 
		{
			vty_out( vty, "  %% set onu%d/%d/%d loopback err\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
		}
	}
	
	return CMD_SUCCESS;
}

DEFUN  (
    onu_pon_loopback_show,
    onu_pon_loopback_show_cmd,
    "show pon-loopback",
    "show onu loop mode\n"
    "show onu loop mode\n"
    )
{
	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int enable = V2R1_DISABLE ;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	/* 因适配层没有做，PAS5001暂不支持，后续考虑增加*/
	if( V2R1_GetPonchipType( phyPonId ) == PONCHIP_PAS5001 )
		{
		vty_out( vty, "  PAS5001 is Unsupported\r\n" );
		/*vty_out( vty, "  %% this pon chip is PAS5001, FEC is not supported\r\n");*/
		return( CMD_SUCCESS );
		}
	
	userOnuId = (short int)(ulOnuId - 1);
	if(ThisIsValidOnu(phyPonId, userOnuId) != ROK )
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}

	if( GetOnuOperStatus(phyPonId, userOnuId )  != ONU_OPER_STATUS_UP )
		{
		vty_out( vty, "  %% onu%d/%d/%d not online.\r\n",slotId,port,ulOnuId);
		return CMD_WARNING ;
		}
			
	/* 此处应增加对ONU 类型的判断，PMC ONU和CTC ONU应区分处理*/
	/*if( GetOnuVendorType(phyPonId, userOnuId) ==  ONU_VENDOR_GW)*/
	{
		lRet = GetOnuPonLoopbackEnable((short int )phyPonId, (short int)userOnuId, &enable );
		if ( lRet == V2R1_ONU_NOT_EXIST )
		{
			vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
		}
		if( lRet == RERROR ) 
		{
			vty_out( vty, "  %% get onu%d/%d/%d FEC err\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
		}
	}

	vty_out( vty, "  Loopback : %s\r\n", (enable == V2R1_ENABLE ? "enable" : "disable") );

    return CMD_SUCCESS;
}



#ifdef  ONU_UPSTREAM_SA_MAC_FILTER

/* the following cli is add by chenfj 2007-6-1 
	设置ONU 上行数据包原目的MAC 过滤
*/
DEFUN  (
	onu_SAmac_filter_config,
	onu_SAmac_filter_config_cmd,
	"onu src-mac filter <H.H.H>",
	"Config onu upstream source mac filter\n"
	"Config onu upstream source mac filter\n"
	"Config onu upstream source mac filter\n"
	"Please input the source mac to be filtered\n"
	)
{
	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	unsigned char MacAddr[6] = {0,0,0,0,0,0};

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
		{
       	 vty_out( vty, "  %% Invalid MAC address.\r\n" );
       	 return CMD_WARNING;
		}	
	 		
	userOnuId = (short int)(ulOnuId - 1);
	lRet = AddOnuSAMacFilter((short int )phyPonId, (short int)userOnuId, MacAddr);
	if( lRet == V2R1_ONU_NOT_EXIST )
		{
		vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
		return CMD_WARNING; 
		}
	else  if( lRet == V2R1_ONU_FILTER_SA_MAC_NOT_VALID )
		{
		vty_out( vty, " %% the mac address is invalid \r\n");
		return CMD_WARNING;  
		}
	else if( lRet == V2R1_ONU_FILTER_SA_MAC_EXIST )
		{
		vty_out( vty, " %% the mac address is already in filter table \r\n");
		return CMD_SUCCESS; 
		}			
	else if( lRet == RERROR ) 
		{
		vty_out( vty, "  %% set onu%d/%d/%d source mac filter err\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}			

    return CMD_SUCCESS;
}

DEFUN  (
	undo_onu_SAmac_filter_config,
	undo_onu_SAmac_filter_config_cmd,
	"undo onu src-mac filter {<H.H.H>}*1",
	"clear onu upstream source mac filter\n"
	"clear onu upstream source mac filter\n"
	"clear onu upstream source mac filter\n"
	"clear onu upstream source mac filter\n"
	"Please input the source mac to be un-filtered\n"
	)
{
	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	unsigned char MacAddr[6] = {0,0,0,0,0,0};

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}
	
	if( argc == 1 )
	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
		{
       	 vty_out( vty, "  %% Invalid MAC address.\r\n" );
       	 return CMD_WARNING;
		}
	
	userOnuId = (short int)(ulOnuId - 1);
	if( argc == 1 )
		{
		lRet = ClearOnuSAMacFilter((short int )phyPonId, (short int)userOnuId, MacAddr);
		if( lRet == V2R1_ONU_NOT_EXIST )
			{
			vty_out( vty, " %% onu %d/%d/%d not exist\r\n", slotId, port, ulOnuId );
			return CMD_WARNING; 
			}
		else if( lRet == V2R1_ONU_FILTER_SA_MAC_NOT_VALID )
			{
			vty_out( vty, " %% the mac address is invalid \r\n");
			return CMD_WARNING;  
			}
		else if( lRet == V2R1_ONU_FILTER_SA_MAC_NOT_EXIST )
			{
			vty_out( vty, " %% the mac address isn't in filter table \r\n");
			return CMD_SUCCESS; 
			}			
		else if( lRet == RERROR ) 
			{
			vty_out( vty, "  %% clear onu%d/%d/%d source mac filter err\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}
		}
	else if( argc == 0 )
		{
		ClearOnuSAMacFilterAll( (short int )phyPonId, (short int)userOnuId );
		}
			
    return CMD_SUCCESS;
}

DEFUN  (
	show_onu_SAmac_filter_config,
	show_onu_SAmac_filter_config_cmd,
	"show onu src-mac filter",
	"show onu upstream source mac filter\n"
	"show onu upstream source mac filter\n"
	"show onu upstream source mac filter\n"
	"show onu upstream source mac filter\n"
	)
{
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if(PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}
	
	userOnuId = (short int)(ulOnuId - 1);
	ShowOnuFilterSAMacByVty1( phyPonId, userOnuId, vty );

	return( CMD_SUCCESS );

}
#endif

#ifdef  ONU_IP_AND_PORT_FILTER

/*   added by chenfj 2007-6-8 
        增加ONU 数据流IP/PORT过滤 */

DEFUN( 
	onu_Ipfilter_config,
	onu_Ipfilter_config_cmd,
	"onu [src-ip|dst-ip] filter <A.B.C.D>",
	"Create a filter rule\n"
	"onu packet ip Address option:source ip\n"
	"onu packet ip Address option:destination ip\n"
	"onu packet ip address filter\n"
	"Please input the IP address\n"
	)
    {
	unsigned int  IpFilter= 0 ;
	
	unsigned int  IpAddr;

	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if(PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}	

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}


#if 0		
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter input err\r\n");
		return( CMD_WARNING);
		}
#endif

	if ( !( VOS_StrCmp( argv[ 0 ] , "src-ip" ) ) )
		IpFilter = V2R1_ONU_FILTER_IP_SOURCE ; 
	else /*if ( !( VOS_StrCmp( argv[ 0 ] , "dst-ip" ) ) )*/
		IpFilter =  V2R1_ONU_FILTER_IP_DEST ; 
	/*else 
		{
		vty_out( vty, " %% src-ip|dst-ip Parameter input err\r\n");
		return( CMD_WARNING);
		}*/

	if( V2R1_GetLongFromIpdotstring( argv[1], &IpAddr )  != ROK )
		{
		vty_out( vty, " %% IpAddr Parameter err\r\n");
		return( CMD_WARNING);
		}

	if ( IpFilter ==  V2R1_ONU_FILTER_IP_SOURCE  )
		ret = AddOnuSIpFilter( phyPonId, userOnuId , IpAddr );
	else if ( IpFilter ==  V2R1_ONU_FILTER_IP_DEST)
		ret = AddOnuDIpFilter( phyPonId, userOnuId , IpAddr );
	
	if( ret ==  V2R1_ONU_NOT_EXIST ) 
		{
		vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
		return( CMD_WARNING );
		}
	else if( ret == V2R1_ONU_FILTER_IP_EXIST ) 
		{
		vty_out( vty, " %% the ipAddr 0x%8x is already in filter table\r\n", IpAddr );
		return( CMD_WARNING );
		}
	else if( ret == RERROR ) 
		{
		vty_out( vty, "  %% set onu%d/%d/%d ip filter err\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}				

	return  CMD_SUCCESS;
}

DEFUN( 
	undo_onu_Ipfilter_config,
	undo_onu_Ipfilter_config_cmd,
	"undo onu [src-ip|dst-ip] filter {<A.B.C.D>}*1",
	"clear a filter rule\n"
	"clear a filter rule\n"
	"onu packet ip Address option:source ip\n"
	"onu packet ip Address option:destination ip\n"
	"onu packet ip address filter\n"
	"Please input the IP address\n"
	)
    {
	/*LONG FilterFlag ;*/
	unsigned int  IpFilter= 0 ;
	unsigned int  IpAddr;

	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

#if 0		
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter input error \r\n");
		return( CMD_WARNING);
		}
#endif

	if ( !( VOS_StrCmp( argv[ 0 ] , "src-ip" ) ) )
		IpFilter = V2R1_ONU_FILTER_IP_SOURCE ; 
	else /*if ( !( VOS_StrCmp( argv[ 0 ] , "dst-ip" ) ) )*/
		IpFilter =  V2R1_ONU_FILTER_IP_DEST ; 
	/*else 
		{
		vty_out( vty, " %% src-ip|dst-ip Parameter input err\r\n");
		return( CMD_WARNING);
		}*/

	if( argc == 2 )
		{
		if( V2R1_GetLongFromIpdotstring( argv[1], &IpAddr )  != ROK )
			{
			vty_out( vty, " %% IpAddr Parameter err\r\n");
			return( CMD_WARNING);
			}
		}

	if( argc == 2 ) /* 作用于ONU 上ACL 列表中指定节点*/
		{			
		if( IpFilter == V2R1_ONU_FILTER_IP_SOURCE )
			ret = ClearOnuSIpFilter( phyPonId, userOnuId , IpAddr );
		else if( IpFilter == V2R1_ONU_FILTER_IP_DEST)
			ret = ClearOnuDIpFilter( phyPonId, userOnuId , IpAddr );
		
		if( ret ==  V2R1_ONU_NOT_EXIST ) 
			{
			vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
			return( CMD_WARNING );
			}
		else if( ret == V2R1_ONU_FILTER_IP_NOT_EXIST ) 
			{
			vty_out( vty, " %% the ipAddr 0x%8x is not in filter table\r\n", IpAddr );
			return( CMD_WARNING );
			}
		else if( ret == RERROR ) 
			{
			vty_out( vty, "  %% clear onu%d/%d/%d ip filter err\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}				
		}

	else {  /* 作用于ONU 上ACL 列表中所有节点*/	
				
		if( IpFilter == V2R1_ONU_FILTER_IP_SOURCE )
			ClearOnuSIpFilterAll( phyPonId, userOnuId );
		else if( IpFilter == V2R1_ONU_FILTER_IP_DEST)
			ClearOnuDIpFilterAll( phyPonId, userOnuId );
		}

	return  CMD_SUCCESS;
}

DEFUN( 
	onu_SIpUdpfilter_config,
	onu_SIpUdpfilter_config_cmd,
	"onu udp filter src-port <0-65535>",
	"Create a filter rule\n"
	"create source udp port filter\n"
	"onu packet source udp port filter\n"
	"source udp port\n"
	"please input source udp port\n"
	)
    {
	/*LONG FilterFlag ;*/
	unsigned int  IpAddr;

	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int udp_port;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

	udp_port = ( unsigned short int ) VOS_AtoL(argv[0]);

	IpAddr = INVALID_IP;	
		
	ret = AddOnuSIpUdpFilter( phyPonId, userOnuId , IpAddr, udp_port );				
	if( ret ==  V2R1_ONU_NOT_EXIST ) 
		{
		vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
		return( CMD_WARNING );
		}
	else if( ret == V2R1_ONU_FILTER_IP_UDP_EXIST ) 
		{
		vty_out( vty, " %% the source udp-port %d is already in filter table\r\n", udp_port );
		return( CMD_WARNING );
		}
	else if( ret == RERROR ) 
		{
		vty_out( vty, "  %% set onu%d/%d/%d source ip-udp filter err\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}		

	return  CMD_SUCCESS;
}

DEFUN( 
	undo_onu_SIpUdpfilter_config,
	undo_onu_SIpUdpfilter_config_cmd,
	"undo onu udp filter {src-port <0-65535>}*1",
	"clear a filter rule\n"
	"clear a filter rule\n"
	"clear source udp port filter\n"
	"onu packet source udp port filter\n"
	"source udp port\n"
	"please input source udp port\n"
	)
    {
	/*LONG FilterFlag ;*/
	unsigned int  IpAddr;

	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int udp_port;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId)  == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

	if( argc == 1 )
		{
		udp_port = ( unsigned short int ) VOS_AtoL(argv[0]);
		}

	IpAddr = INVALID_IP;	

	if( argc == 0 )   /* 参数中没有特定的UDP 端口*/
		{
		ret = ClearOnuSIpUdpFilterAll( phyPonId, userOnuId );
		}
	
	else if( argc ==1)  /* 参数中有特定的UDP 端口*/
		{
		ret = ClearOnuSIpUdpFilter(phyPonId,  userOnuId, IpAddr, udp_port);
		if( ret == V2R1_ONU_NOT_EXIST ) 
			{
			vty_out( vty, " %% onu %d/%d/%d not exist\n", slotId, port, ulOnuId );
			return( CMD_WARNING );
			}
		else if( ret == V2R1_ONU_FILTER_IP_UDP_NOT_EXIST )
			{
			vty_out( vty, " %% the source udp-port %d is not in the filter table\n", udp_port);
			}
		else if( ret == RERROR ) 
			{
			vty_out( vty, "  %% clear onu%d/%d/%d source ip-udp filter err\r\n", slotId, port, ulOnuId);
			return CMD_WARNING;  
			}
		}	

	return CMD_SUCCESS;
}

DEFUN( 
	onu_SIpTcpfilter_config,
	onu_SIpTcpfilter_config_cmd,
	"onu tcp filter src-port <0-65535>",
	"Create a filter rule\n"
	"create source tcp port filter\n"
	"onu packet source tcp port filter\n"
	"source tcp port\n"
	"please input source tcp port\n"
	)
    {
	/*LONG FilterFlag ;*/
	unsigned int  IpAddr;
	
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int tcp_port;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId)  == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}
	
	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

	tcp_port = ( unsigned short int ) VOS_AtoL(argv[0]);

	IpAddr = INVALID_IP;
	
	ret = AddOnuSIpTcpFilter( phyPonId, userOnuId , IpAddr, tcp_port );				
	if( ret ==  V2R1_ONU_NOT_EXIST ) 
		{
		vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
		return( CMD_WARNING );
		}
	else if( ret == V2R1_ONU_FILTER_IP_TCP_EXIST ) 
		{
		vty_out( vty, " %% the source tcp-port %d is already in filter table\r\n", tcp_port );
		return( CMD_WARNING );
		}
	else if( ret == RERROR ) 
		{
		vty_out( vty, "  %% set onu%d/%d/%d source tcp port filter err\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}

	return CMD_SUCCESS;
}

DEFUN( 
	undo_onu_SIpTcpfilter_config,
	undo_onu_SIpTcpfilter_config_cmd,
	"undo onu tcp filter {src-port <0-65535>}*1",
	"clear a filter rule\n"
	"clear a filter rule\n"
	"clear source tcp port filter\n"
	"onu packet source tcp port filter\n"
	"source tcp port\n"
	"please input source tcp port\n"
	)
    {
	/*LONG FilterFlag ;*/
	unsigned int  IpAddr;

	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int tcp_port;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

#if 0	
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter err\r\n");
		return( CMD_WARNING);
		}
#endif

	if( argc == 1 )
		{
		tcp_port = ( unsigned short int ) VOS_AtoL(argv[0]);
		}

	IpAddr = INVALID_IP;

	if( argc == 0 )   /* 参数中没有特定的TCP 端口*/
		{
		ret = ClearOnuSIpTcpFilterAll( phyPonId, userOnuId );
		}
	
	else if( argc ==1)  /* 参数中有特定的TCP 端口*/
		{
		ret = ClearOnuSIpTcpFilter(phyPonId,  userOnuId, IpAddr, tcp_port);
		if( ret == V2R1_ONU_NOT_EXIST ) 
			{
			vty_out( vty, " %% onu %d/%d/%d not exist\n", slotId, port, ulOnuId );
			return( CMD_WARNING );
			}
		else if( ret == V2R1_ONU_FILTER_IP_TCP_NOT_EXIST )
			{
			vty_out( vty, " %% the source tcp-port %d is not in the filter table\n", tcp_port);
			}
		else if( ret == RERROR ) 
			{
			vty_out( vty, "  %% clear onu%d/%d/%d source tcp port filter err\r\n", slotId, port, ulOnuId);
			return CMD_WARNING;  
			}
		}				
		

	return CMD_SUCCESS;
}

DEFUN( 
	show_onu_IPUDPTCPfilter_config,
	show_onu_IPUDPTCPfilter_config_cmd,
	"show onu [src-ip|dst-ip|src-udp|src-tcp|vlanid|ethtype|iptype] filter",
	"show onu filter table\n"
	"show onu filter table\n"
	"show onu src-ip filter table\n"
	"show onu dst-ip filter table\n"
	"show onu src-udp filter table\n"
	"show onu src-tcp filter table\n"
	"show onu vlanid filter table\n"
	"show onu ether type filter table\n"
	"show onu ip protocol filter table\n"
	"show onu filter table\n"
	OnuIDStringDesc
	)
    {
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	LONG ret;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}
	if( ThisIsValidOnu( phyPonId, userOnuId ) != ROK ) 
		{
		vty_out(vty, "  onu %d/%d/%d not exist\r\n", slotId, port, ulOnuId );
		return( CMD_WARNING );
		}

	if ( !( VOS_StrCmp( argv[ 0 ] , "src-ip" ) ) )
	{
		ret = ShowOnuSIpFilterByVty( phyPonId, userOnuId, vty );
	}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "dst-ip" ) ) )
	{
		ret = ShowOnuDIpFilterByVty( phyPonId, userOnuId, vty );
	}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "src-udp" ) ) )
	{
		ret = ShowOnuUdpFilterByVty( phyPonId, userOnuId, vty );
	}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "src-tcp" ) ) )
	{
		ret = ShowOnuTcpFilterByVty( phyPonId, userOnuId, vty );
	}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "vlanid" ) ) )
	{
		ret = ShowOnuVlanIdFilterByVty( phyPonId, userOnuId, vty );
	}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "ethtype" ) ) )
	{
		ret = ShowOnuEtherTypeFilterByVty( phyPonId, userOnuId, vty );
	}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "iptype" ) ) )
	{
		ret = ShowOnuIpTypeFilterByVty( phyPonId, userOnuId, vty );
	}
	else {
		/*vty_out( vty, " %% Parameter err\r\n");*/
		return( CMD_WARNING);
		}

	if( ret == ROK )
		return CMD_SUCCESS;
	return CMD_WARNING;
}
#endif

#if 0
DEFUN( 
	onu_Ipfilter_config,
	onu_Ipfilter_config_cmd,
	"access-list [permit|deny] ip [dst-ip|src-ip] <A.B.C.D>",
	"Create an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	"IP protocol type is IP,representing all IP protocols\n"
	"Configure destination IP address\n"
	"Configure source IP address\n"
	"Please input the IP address\n"
	)
    {
	LONG FilterFlag ;
	unsigned int  IpFilter= 0 ;
	unsigned int  IpAddr;

	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int ret;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}	

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

        if ( ( argc != 3 )/* ||( argc > 5)*/)
		{
		vty_out( vty, "%% Unknown command.\r\n" ) ;
		return -1 ;
        	}
		
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter input err\r\n");
		return( CMD_WARNING);
		}

	if ( !( VOS_StrCmp( argv[ 1 ] , "src-ip" ) ) )
		IpFilter = V2R1_ONU_FILTER_IP_SOURCE ; 
	else if ( !( VOS_StrCmp( argv[1 ] , "dst-ip" ) ) )
		IpFilter =  V2R1_ONU_FILTER_IP_DEST ; 
	else 
		{
		vty_out( vty, " %% src-ip|dst-ip Parameter input err\r\n");
		return( CMD_WARNING);
		}

	if( !(VOS_StrCmp( argv[2], "any" ) ) )
		{
		vty_out( vty, " %% 'any' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else  if( V2R1_GetLongFromIpdotstring( argv[2], &IpAddr )  != ROK )
		{
		vty_out( vty, " %% IpAddr Parameter err\r\n");
		return( CMD_WARNING);
		}

	if( FilterFlag == V2R1_ENABLE )
		{
		if( IpFilter == V2R1_ONU_FILTER_IP_SOURCE )
			ret = AddOnuSIpFilter( phyPonId, userOnuId , IpAddr );
		else if( IpFilter == V2R1_ONU_FILTER_IP_DEST)
			ret = AddOnuDIpFilter( phyPonId, userOnuId , IpAddr );
		
		if( ret ==  V2R1_ONU_NOT_EXIST ) 
			{
			vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
			return( CMD_WARNING );
			}
		else if( ret == V2R1_ONU_FILTER_IP_EXIST ) 
			{
			vty_out( vty, " %% the ipAddr 0x%8x is already in ACL table\r\n", IpAddr );
			return( CMD_WARNING );
			}
		else if( lRet == RERROR ) 
			{
			vty_out( vty, "  %% set onu%d/%d/%d ip ACL err\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	undo_onu_Ipfilter_config,
	undo_onu_Ipfilter_config_cmd,
	"undo access-list [permit|deny] ip [dst-ip|src-ip] {<A.B.C.D>}*1",
	"clear an access-list\n"
	"clear an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	"IP protocol type is IP,representing all IP protocols\n"
	"Configure destination IP address\n"
	"Configure source IP address\n"
	"Please input the IP address\n"
	)
    {
	LONG FilterFlag ;
	unsigned int  IpFilter= 0 ;
	unsigned int  IpAddr;

	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int ret;	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}
	
	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

        if ( ( argc  !=  2 ) && ( argc !=3) )
        {
            vty_out( vty, "%% Unknown command.\r\n" ) ;
            return -1 ;
        }
		
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter input err\r\n");
		return( CMD_WARNING);
		}

	if ( !( VOS_StrCmp( argv[ 1 ] , "src-ip" ) ) )
		IpFilter = V2R1_ONU_FILTER_IP_SOURCE ; 
	else if ( !( VOS_StrCmp( argv[ 1 ] , "dst-ip" ) ) )
		IpFilter =  V2R1_ONU_FILTER_IP_DEST ; 
	else 
		{
		vty_out( vty, " %% src-ip|dst-ip Parameter input err\r\n");
		return( CMD_WARNING);
		}

	if( argc == 3 )
		{
		if( !(VOS_StrCmp( argv[2], "any" ) ) )
			{
			vty_out( vty, " %% 'any' option is not finished\r\n");
			return( CMD_WARNING );
			}
		else  if( V2R1_GetLongFromIpdotstring( argv[2], &IpAddr )  != ROK )
			{
			vty_out( vty, " %% IpAddr Parameter err\r\n");
			return( CMD_WARNING);
		}
		}

	if( argc == 3 ) /* 作用于ONU 上ACL 列表中指定节点*/
		{
		if( FilterFlag == V2R1_ENABLE )
			{
			if( IpFilter == V2R1_ONU_FILTER_IP_SOURCE )
				ret = ClearOnuSIpFilter( phyPonId, userOnuId , IpAddr );
			else if( IpFilter == V2R1_ONU_FILTER_IP_DEST)
				ret = ClearOnuDIpFilter( phyPonId, userOnuId , IpAddr );
			
			if( ret ==  V2R1_ONU_NOT_EXIST ) 
				{
				vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
				return( CMD_WARNING );
				}
			else if( ret == V2R1_ONU_FILTER_IP_NOT_EXIST ) 
				{
				vty_out( vty, " %% the ipAddr 0x%8x is not in ACL table\r\n", IpAddr );
				return( CMD_WARNING );
				}
			else if( lRet == RERROR ) 
				{
				vty_out( vty, "  %% clear onu%d/%d/%d ip ACL err\r\n",slotId,port,ulOnuId);
				return CMD_WARNING;  
				}				
			}
		
		else if( FilterFlag == V2R1_DISABLE )
			{

			}
		}

	else {  /* 作用于ONU 上ACL 列表中所有节点*/	
		
		if( FilterFlag == V2R1_ENABLE )
			{				
			if( IpFilter == V2R1_ONU_FILTER_IP_SOURCE )
				ret = ClearOnuSIpFilterAll( phyPonId, userOnuId );
			else if( IpFilter == V2R1_ONU_FILTER_IP_DEST)
				ret = ClearOnuDIpFilterAll( phyPonId, userOnuId );			
			}
		
		else if( FilterFlag == V2R1_DISABLE )
			{

			}
		}

	return lRet;
}


DEFUN( 
	onu_DIpUdpfilter_config,
	onu_DIpUdpfilter_config_cmd,
	"access-list [permit|deny] udp dst-ip <A.B.C.D> dst-port <0-65535>",
	"Create an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	"IP protocol type is udp\n"
	"Configure destination IP address\n"
	"Please input the IP address\n"
	"Please input the destination udp port\n"
	"Please input the destination udp port\n"
	)
    {
	LONG FilterFlag ;
	unsigned int  IpAddr;

	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int ret;
	unsigned short int udp_port;
	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}
	
	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

        if ( ( argc != 3 )/* ||( argc > 5)*/)
        {
            vty_out( vty, "%% Unknown command.\r\n" ) ;
            return -1 ;
        }		
		
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter input err\r\n");
		return( CMD_WARNING);
		}

	if( !(VOS_StrCmp( argv[1], "any" ) ) )
		{
		vty_out( vty, " %% 'any' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else  if( V2R1_GetLongFromIpdotstring( argv[1], &IpAddr )  != ROK )
		{
		vty_out( vty, " %% IpAddr Parameter input err\r\n");
		return( CMD_WARNING);
		}

	udp_port = ( unsigned short int ) VOS_AtoL(argv[2]);

	if( FilterFlag == V2R1_ENABLE )  /* permit / deny */
		{	
		ret = AddOnuDIpUdpFilter( phyPonId, userOnuId , IpAddr, udp_port );				
		if( ret ==  V2R1_ONU_NOT_EXIST ) 
			{
			vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
			return( CMD_WARNING );
			}
		else if( ret == V2R1_ONU_FILTER_IP_UDP_EXIST ) 
			{
			vty_out( vty, " %% the destination ipAddr 0x%8x udp-port%d is already in ACL table\r\n", IpAddr, udp_port );
			return( CMD_WARNING );
			}
		else if( lRet == RERROR ) 
			{
			vty_out( vty, "  %% set onu%d/%d/%d destination ip-udp ACL err\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}			
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	undo_onu_DIpUdpfilter_config,
	undo_onu_DIpUdpfilter_config_cmd,
	"undo access-list [permit|deny] udp dst-ip {<A.B.C.D> dst-port <0-65535>}*1",
	"clear an access-list\n"
	"clear an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	"IP protocol type is UDP\n"
	"Configure destination IP address\n"
	"Please input the IP address\n"
	"Configure destination udp port\n"
	"Please input the udp port\n"
	/*"any: any destination IP address\n"*/
	)
    {
	LONG FilterFlag ;
	unsigned int  IpAddr;

	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int ret;
	unsigned short int udp_port;

	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}
	
	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

        if ( ( argc  != 1 ) && ( argc !=3) )
        {
            vty_out( vty, "%% Unknown command.\r\n" ) ;
            return -1 ;
        }
		
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter input err\r\n");
		return( CMD_WARNING);
		}
	
	if( argc == 3 )
		{
		if( !(VOS_StrCmp( argv[1], "any" ) ) )
			{
			vty_out( vty, " %% 'any' option is not finished\r\n");
			return( CMD_WARNING );
			}
		else  if( V2R1_GetLongFromIpdotstring( argv[1], &IpAddr )  != ROK )
			{
			vty_out( vty, " %% IpAddr Parameter err\r\n");
			return( CMD_WARNING);
			}
		
		udp_port = ( unsigned short int ) VOS_AtoL(argv[2]);
		}

	if( FilterFlag == V2R1_ENABLE )
		{	
		if( argc == 1 )   /* 参数中没有有特定的IP地址及UDP 端口*/
			{
			ret = ClearOnuDIpUdpFilterAll( phyPonId, userOnuId );
			}
		
		else if( argc ==3 )  /* 参数中有特定的IP地址及UDP 端口*/
			{
			ret = ClearOnuDIpUdpFilter(phyPonId,  userOnuId, IpAddr, udp_port);
			if( ret == V2R1_ONU_NOT_EXIST ) 
				{
				vty_out( vty, " %% onu %d/%d/%d not exist\n", slotId, port, ulOnuId );
				return( CMD_WARNING );
				}
			else if( ret == V2R1_ONU_FILTER_IP_UDP_NOT_EXIST )
				{
				vty_out( vty, " %% the dest ip and udp-port are not in the ACL table\n");
				}
			else if( lRet == RERROR ) 
				{
				vty_out( vty, "  %% clear onu%d/%d/%d dest ip-udp ACL err\r\n", slotId, port, ulOnuId);
				return CMD_WARNING;  
				}
			}			
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	onu_SIpUdpfilter_config,
	onu_SIpUdpfilter_config_cmd,
	"access-list [permit|deny] udp src-ip <A.B.C.D> src-port <0-65535>",
	"Create an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	"IP protocol type is udp\n"
	"Configure source IP address\n"
	"Please input the IP address\n"
	"Please input the source udp port\n"
	"Please input the source udp port\n"
	/*"any: any destination IP address\n"*/
	)
    {
	LONG FilterFlag ;
	unsigned int  IpAddr;

	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int ret;
	unsigned short int udp_port;



	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}
	
	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

        if ( ( argc != 3 )/* ||( argc > 5)*/)
        {
            vty_out( vty, "%% Unknown command.\r\n" ) ;
            return -1 ;
        }
		
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter input err\r\n");
		return( CMD_WARNING);
		}

	if( !(VOS_StrCmp( argv[1], "any" ) ) )
		{
		vty_out( vty, " %% 'any' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else  if( V2R1_GetLongFromIpdotstring( argv[1], &IpAddr )  != ROK )
		{
		vty_out( vty, " %% IpAddr Parameter input err\r\n");
		return( CMD_WARNING);
		}

	udp_port = ( unsigned short int ) VOS_AtoL(argv[2]);

	if( FilterFlag == V2R1_ENABLE )  /* permit / deny */
		{			
		ret = AddOnuSIpUdpFilter( phyPonId, userOnuId , IpAddr, udp_port );				
		if( ret ==  V2R1_ONU_NOT_EXIST ) 
			{
			vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
			return( CMD_WARNING );
			}
		else if( ret == V2R1_ONU_FILTER_IP_UDP_EXIST ) 
			{
			vty_out( vty, " %% the source ipAddr 0x%8x udp-port%d is already in ACL table\r\n", IpAddr, udp_port );
			return( CMD_WARNING );
			}
		else if( lRet == RERROR ) 
			{
			vty_out( vty, "  %% set onu%d/%d/%d source ip-udp ACL err\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}			
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	undo_onu_SIpUdpfilter_config,
	undo_onu_SIpUdpfilter_config_cmd,
	"undo access-list [permit|deny] udp src-ip {<A.B.C.D> src-port <0-65535>}*1",
	"clear an access-list\n"
	"clear an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	"IP protocol type is UDP\n"
	"Configure source IP address\n"
	"Please input the IP address\n"
	"Configure source udp port\n"
	"Please input the udp port\n"
	/*"any: any destination IP address\n"*/
	)
    {
	LONG FilterFlag ;
	unsigned int  IpAddr;

	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int ret;
	unsigned short int udp_port;

	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}
	
        if ( ( argc  !=1 ) && ( argc != 3) )
        {
            vty_out( vty, "%% Unknown command.\r\n" ) ;
            return -1 ;
        }
		
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter input err\r\n");
		return( CMD_WARNING);
		}
	
	if( argc == 3 )
		{
		if( !(VOS_StrCmp( argv[1], "any" ) ) )
			{
			vty_out( vty, " %% 'any' option is not finished\r\n");
			return( CMD_WARNING );
			}
		else  if( V2R1_GetLongFromIpdotstring( argv[1], &IpAddr )  != ROK )
			{
			vty_out( vty, " %% IpAddr Parameter err\r\n");
			return( CMD_WARNING);
			}
		
		udp_port = ( unsigned short int ) VOS_AtoL(argv[2]);
		}

	if( FilterFlag == V2R1_ENABLE )
		{
		if( argc == 1 )   /* 参数中没有有特定的IP地址及UDP 端口*/
			{
			ret = ClearOnuSIpUdpFilterAll( phyPonId, userOnuId );
			}
		
		else if( argc ==3 )  /* 参数中有特定的IP地址及UDP 端口*/
			{
			ret = ClearOnuSIpUdpFilter(phyPonId,  userOnuId, IpAddr, udp_port);
			if( ret == V2R1_ONU_NOT_EXIST ) 
				{
				vty_out( vty, " %% onu %d/%d/%d not exist\n", slotId, port, ulOnuId );
				return( CMD_WARNING );
				}
			else if( ret == V2R1_ONU_FILTER_IP_UDP_NOT_EXIST )
				{
				vty_out( vty, " %% the source ip and udp-port are not in the ACL table\n");
				}
			else if( lRet == RERROR ) 
				{
				vty_out( vty, "  %% clear onu%d/%d/%d source ip-udp ACL err\r\n", slotId, port, ulOnuId);
				return CMD_WARNING;  
				}
			}				
			
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	onu_SIpTcpfilter_config,
	onu_SIpTcpfilter_config_cmd,
	"access-list [permit|deny] tcp src-ip <A.B.C.D> src-port <0-65535>",
	"Create an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	"IP protocol type is tcp\n"
	"Configure source IP address\n"
	"Please input the IP address\n"
	"Please input the source tcp port\n"
	"Please input the source tcp port\n"
	/*"any: any destination IP address\n"*/
	)
    {
	LONG FilterFlag ;
	unsigned int  IpAddr;

	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int ret;
	unsigned short int tcp_port;

	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port) );
	
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

        if ( ( argc != 3 )/* ||( argc > 5)*/)
        {
            vty_out( vty, "%% Unknown command.\r\n" ) ;
            return -1 ;
        }
		
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter input err\r\n");
		return( CMD_WARNING);
		}

	if( !(VOS_StrCmp( argv[1], "any" ) ) )
		{
		vty_out( vty, " %% 'any' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else  if( V2R1_GetLongFromIpdotstring( argv[1], &IpAddr )  != ROK )
		{
		vty_out( vty, " %% IpAddr Parameter err\r\n");
		return( CMD_WARNING);
		}

	tcp_port = ( unsigned short int ) VOS_AtoL(argv[2]);

	if( FilterFlag == V2R1_ENABLE )  /* permit / deny */
		{		
		ret = AddOnuSIpTcpFilter( phyPonId, userOnuId , IpAddr, tcp_port );				
		if( ret ==  V2R1_ONU_NOT_EXIST ) 
			{
			vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
			return( CMD_WARNING );
			}
		else if( ret == V2R1_ONU_FILTER_IP_TCP_EXIST ) 
			{
			vty_out( vty, " %% the source ipAddr 0x%8x tcp-port%d is already in ACL table\r\n", IpAddr, tcp_port );
			return( CMD_WARNING );
			}
		else if( lRet == RERROR ) 
			{
			vty_out( vty, "  %% set onu%d/%d/%d source ip-tcp ACL err\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}		
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	undo_onu_SIpTcpfilter_config,
	undo_onu_SIpTcpfilter_config_cmd,
	"undo access-list [permit|deny] tcp src-ip {<A.B.C.D> src-port <0-65535>}*1",
	"clear an access-list\n"
	"clear an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	"IP protocol type is TCP\n"
	"Configure source IP address\n"
	"Please input the IP address\n"
	"Configure source tcp port\n"
	"Please input the tcp port\n"
	/*"any: any destination IP address\n"*/
	)
    {
	LONG FilterFlag ;
	unsigned int  IpAddr;

	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int ret;
	unsigned short int tcp_port;

	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

        if ( ( argc  != 1 ) && ( argc !=3) )
        {
            vty_out( vty, "%% Unknown command.\r\n" ) ;
            return -1 ;
        }
		
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter input err\r\n");
		return( CMD_WARNING);
		}
	
	if( argc == 3)
		{
		if( !(VOS_StrCmp( argv[1], "any" ) ) )
			{
			vty_out( vty, " %% 'any' option is not finished\r\n");
			return( CMD_WARNING );
			}
		else  if( V2R1_GetLongFromIpdotstring( argv[1], &IpAddr )  != ROK )
			{
			vty_out( vty, " %% IpAddr Parameter err\r\n");
			return( CMD_WARNING);
			}
		
		tcp_port = ( unsigned short int ) VOS_AtoL( argv[2] );
		}

	if( FilterFlag == V2R1_ENABLE )
		{
		if( argc == 1 )   /* 参数中没有有特定的IP地址及TCP 端口*/
			{
			ret = ClearOnuSIpTcpFilterAll( phyPonId, userOnuId );
			}
		
		else if( argc ==3 )  /* 参数中有特定的IP地址及TCP 端口*/
			{
			ret = ClearOnuSIpTcpFilter(phyPonId,  userOnuId, IpAddr, tcp_port);
			if( ret == V2R1_ONU_NOT_EXIST ) 
				{
				vty_out( vty, " %% onu %d/%d/%d not exist\n", slotId, port, ulOnuId );
				return( CMD_WARNING );
				}
			else if( ret == V2R1_ONU_FILTER_IP_TCP_NOT_EXIST )
				{
				vty_out( vty, " %% the source ip and tcp-port are not in the ACL table\n");
				}
			else if( lRet == RERROR ) 
				{
				vty_out( vty, "  %% clear onu%d/%d/%d source ip-tcp ACL err\r\n", slotId, port, ulOnuId);
				return CMD_WARNING;  
				}
			}
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	onu_DIpTcpfilter_config,
	onu_DIpTcpfilter_config_cmd,
	"access-list [permit|deny] tcp dst-ip <A.B.C.D> dst-port <0-65535>",
	"Create an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	"IP protocol type is tcp\n"
	"Configure source IP address\n"
	"Please input the IP address\n"
	"Please input the destination tcp port\n"
	"Please input the dest tcp port\n"
	/*"any: any destination IP address\n"*/
	)
    {
	LONG FilterFlag ;
	unsigned int  IpAddr;

	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int tcp_port;

	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

        if ( ( argc != 3 )/* ||( argc > 5)*/)
        {
            vty_out( vty, "%% Unknown command.\r\n" ) ;
            return -1 ;
        }
		
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter input err\r\n");
		return( CMD_WARNING);
		}

	if( !(VOS_StrCmp( argv[1], "any" ) ) )
		{
		vty_out( vty, " %% 'any' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else  if( V2R1_GetLongFromIpdotstring( argv[1], &IpAddr )  != ROK )
		{
		vty_out( vty, " %% IpAddr Parameter err\r\n");
		return( CMD_WARNING);
		}

	tcp_port = ( unsigned short int ) VOS_AtoL(argv[2]);

	if( FilterFlag == V2R1_ENABLE )  /* permit / deny */
		{
		ret = AddOnuDIpTcpFilter( phyPonId, userOnuId , IpAddr, tcp_port );				
		if( ret ==  V2R1_ONU_NOT_EXIST ) 
			{
			vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
			return( CMD_WARNING );
			}
		else if( ret == V2R1_ONU_FILTER_IP_TCP_EXIST ) 
			{
			vty_out( vty, " %% the dest ipAddr 0x%8x tcp-port%d is already in ACL table\r\n", IpAddr, tcp_port );
			return( CMD_WARNING );
			}
		else if( lRet == RERROR ) 
			{
			vty_out( vty, "  %% set onu%d/%d/%d dest ip-tcp ACL err\r\n",slotId,port,ulOnuId);
			return CMD_WARNING;  
			}	
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	undo_onu_DIpTcpfilter_config,
	undo_onu_DIpTcpfilter_config_cmd,
	"undo access-list [permit|deny] tcp dst-ip {<A.B.C.D> dst-port <0-65535>}*1",
	"clear an access-list\n"
	"clear an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	"IP protocol type is TCP\n"
	"Configure source IP address\n"
	"Please input the IP address\n"
	"Configure dest tcp port\n"
	"Please input the tcp port\n"
	)
    {
	LONG FilterFlag ;
	unsigned int  IpAddr;

	LONG lRet = VOS_OK;
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int ret;
	unsigned short int tcp_port;	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

        if ( ( argc  != 1 ) && ( argc !=3) )
        {
            vty_out( vty, "%% Unknown command.\r\n" ) ;
            return -1 ;
        }
		
	if ( !( VOS_StrCmp( argv[ 0 ] , "permit" ) ) )
		{
		FilterFlag = ( LONG ) V2R1_DISABLE ; /* permit the packets matching the conditions to pass */
		vty_out( vty, " %% 'permit' option is not finished\r\n");
		return( CMD_WARNING );
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "deny" ) ) )
		FilterFlag = ( LONG ) V2R1_ENABLE; /* deny the packets matching the conditions to pass */
	else {
		vty_out( vty, " %% permit|deny Parameter input err\r\n");
		return( CMD_WARNING);
		}
	
	if( argc == 3 )
		{
		if( !(VOS_StrCmp( argv[1], "any" ) ) )
			{
			vty_out( vty, " %% 'any' option is not finished\r\n");
			return( CMD_WARNING );
			}
		else  if( V2R1_GetLongFromIpdotstring( argv[1], &IpAddr )  != ROK )
			{
			vty_out( vty, " %% IpAddr Parameter err\r\n");
			return( CMD_WARNING);
			}
		
		tcp_port = ( unsigned short int ) VOS_AtoL(argv[2]);
		}

	if( FilterFlag == V2R1_ENABLE )
		{
		if( argc == 1 )   /* 参数中没有有特定的IP地址及TCP 端口*/
			{
			ret = ClearOnuDIpTcpFilterAll( phyPonId, userOnuId );
			}
		
		else if( argc == 3 )  /* 参数中有特定的IP地址及TCP 端口*/
			{
			ret = ClearOnuDIpTcpFilter(phyPonId,  userOnuId, IpAddr, tcp_port);
			if( ret == V2R1_ONU_NOT_EXIST ) 
				{
				vty_out( vty, " %% onu %d/%d/%d not exist\n", slotId, port, ulOnuId );
				return( CMD_WARNING );
				}
			else if( ret == V2R1_ONU_FILTER_IP_TCP_NOT_EXIST )
				{
				vty_out( vty, " %% the dest ip and tcp-port are not in the ACL table\n");
				}
			else if( lRet == RERROR ) 
				{
				vty_out( vty, "  %% clear onu%d/%d/%d dest ip-tcp ACL err\r\n", slotId, port, ulOnuId);
				return CMD_WARNING;  
				}
			}			
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

#endif

#ifdef  ONU_VLAN_ID_FILTER

/** added by chenfj 2007-6-12 
       增加ONU 数据流vlan id 过滤*/
       
DEFUN( 
	onu_vlanIdfilter_config,
	onu_vlanIdfilter_config_cmd,
	"onu vlanid filter <1-4095>",
	"Create a filter rule\n"
	"create vlanid filter\n"
	"onu packet vlanid  filter\n"
	"please input vid value\n"
	)
    {

	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int vlanId;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

	vlanId = ( unsigned short int ) VOS_AtoL(argv[0]);
	
	ret = AddOnuVlanIdFilter( phyPonId, userOnuId , vlanId );				
	if( ret ==  V2R1_ONU_NOT_EXIST ) 
		{
		vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
		return( CMD_WARNING );
		}
	else if( ret == V2R1_ONU_FILTER_VLAN_ID_EXIST ) 
		{
		vty_out( vty, " %% the vlanId %d is already in filter table\r\n", vlanId );
		return( CMD_WARNING );
		}
	else if( ret == RERROR ) 
		{
		vty_out( vty, "  %% set onu%d/%d/%d vlanId filter err\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}
	
	return CMD_SUCCESS;
}

DEFUN( 
	undo_onu_vlanIdfilter_config,
	undo_onu_vlanIdfilter_config_cmd,
	"undo onu vlanid filter {<0-4095>}*1",
	"clear a filter rule\n"
	"clear a filter rule\n"
	"clear vlanid filter\n"
	"onu packet vlanid filter\n"
	"please input vid value\n"
	)
    {
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int vlanId;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port) );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

	if( argc == 1 )
		{
		vlanId = ( unsigned short int ) VOS_AtoL(argv[0]);
		}
	

	if( argc == 0 )   /* 参数中没有特定的vid */
		{
		ret = ClearOnuVlanIdFilterAll( phyPonId, userOnuId );
		}

	else if( argc ==1)  /* 参数中有特定的vid */
		{
		ret = ClearOnuVlanIdFilter(phyPonId,  userOnuId, vlanId );
		if( ret == V2R1_ONU_NOT_EXIST ) 
			{
			vty_out( vty, " %% onu %d/%d/%d not exist\n", slotId, port, ulOnuId );
			return( CMD_WARNING );
			}
		else if( ret == V2R1_ONU_FILTER_VLAN_ID_NOT_EXIST )
			{
			vty_out( vty, " %% the vlanId %d is not in the filter table\n", vlanId);
			}
		else if( ret == RERROR ) 
			{
			vty_out( vty, "  %% clear onu%d/%d/%d vlanId filter err\r\n", slotId, port, ulOnuId);
			return CMD_WARNING;  
			}
		}

	return CMD_SUCCESS;
}
#endif

#ifdef  ONU_ETHER_TYPE_and_IP_PROTOCOL_FILTER

/** added by chenfj 2007-6-15 
       增加ONU 数据流ETHER TYPE /IP PROTOCOL 过滤*/

DEFUN( 
	onu_EtherTypefilter_config,
	onu_EtherTypefilter_config_cmd,
	"onu ethertype filter <eth_type>",
	"Create a filter rule\n"
	"create ether type filter\n"
	"onu packet ether type filter\n"
	"please input ether type value,note: 0xabcd(hex valule),abcd(decimal value)\n"
	)
    {

	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	char *pToken;

	int ret;
	unsigned short int  EtherType;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

	/* modified by chenfj 2007-8-20
		urTracker问题单#4819:
		对于基于EtherType过滤数据【源MAC地址后的类型域】，
		建议命令参数改成16进制
		*/
	
	/*EtherType = ( unsigned short int ) VOS_AtoL(argv[0]);	*/

	if(( VOS_MemCmp( argv[0], "0x", 2) == 0) || ( VOS_MemCmp( argv[0], "0X", 2) == 0) )
		{
		EtherType = ( unsigned short int )VOS_StrToUL( argv[0], &pToken, 16 );
		/*sys_console_printf(" eth type 0x%x\r\n", EtherType );*/
		}
	else {
		EtherType = (unsigned short int )strtol(argv[0], NULL, 10 );
		/*sys_console_printf(" eth type 0x%x\r\n", EtherType );*/
		}

	ret = AddOnuEtherTypeFilter( phyPonId, userOnuId , EtherType );				
	if( ret ==  V2R1_ONU_NOT_EXIST ) 
		{
		vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
		return( CMD_WARNING );
		}
	else if( ret == V2R1_ONU_FILTER_ETHER_TYPE_EXIST ) 
		{
		vty_out( vty, " %% the EtherType 0x%x is already in filter table\r\n", EtherType );
		return( CMD_WARNING );
		}
	else if( ret == RERROR ) 
		{
		vty_out( vty, "  %% set onu%d/%d/%d EtherType filter err\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}
	
	return CMD_SUCCESS;
}

DEFUN( 
	undo_onu_EtherTypefilter_config,
	undo_onu_EtherTypefilter_config_cmd,
	"undo onu ethertype filter {<eth_type>}*1",
	"clear a filter rule\n"
	"clear a filter rule\n"
	"clear ether type filter\n"
	"onu packet ehter type filter\n"
	"please input ether type value,note: 0xabcd(hex valule),abcd(decimal value)\n"
	)
    {
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int EtherType;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

	if( argc == 1 )
		{
		/* modified by chenfj 2007-8-20
			urTracker问题单#4819:
			对于基于EtherType过滤数据【源MAC地址后的类型域】，
			建议命令参数改成16进制
			*/
		
		/*EtherType = ( unsigned short int ) VOS_AtoL(argv[0]);	*/

		if(( VOS_MemCmp( argv[0], "0x", 2) == 0) || ( VOS_MemCmp( argv[0], "0X", 2) == 0) )
			{
			EtherType = ( unsigned short int )VOS_StrToUL( argv[0], NULL, 16 );
			/*sys_console_printf(" eth type 0x%x\r\n", EtherType );*/
			}
		else {
			EtherType = (unsigned short int )strtol(argv[0], NULL, 10 );
			/*sys_console_printf(" eth type 0x%x\r\n", EtherType );*/
			}
		/*EtherType = ( unsigned short int ) VOS_AtoL(argv[0]);*/
		}

	if( argc == 0 )   /* 参数中没有特定的Ether Type */
		{
		ret = ClearOnuEtherTypeFilterAll( phyPonId, userOnuId );
		}
	
	else if( argc ==1)  /* 参数中有特定的EtherType */
		{
		ret = ClearOnuEtherTypeFilter(phyPonId,  userOnuId, EtherType );
		if( ret == V2R1_ONU_NOT_EXIST ) 
			{
			vty_out( vty, " %% onu %d/%d/%d not exist\n", slotId, port, ulOnuId );
			return( CMD_WARNING );
			}
		else if( ret == V2R1_ONU_FILTER_ETHER_TYPE_NOT_EXIST )
			{
			vty_out( vty, " %% the Ether type 0x%x is not in the filter table\n", EtherType );
			}
		else if( ret == RERROR ) 
			{
			vty_out( vty, "  %% clear onu%d/%d/%d Ether type filter err\r\n", slotId, port, ulOnuId);
			return CMD_WARNING;  
			}
		}				

	return CMD_SUCCESS;
}

DEFUN( 
	onu_IpTypefilter_config,
	onu_IpTypefilter_config_cmd,
	"onu iptype filter <1-65535>",
	"Create a filter rule\n"
	"create ip protocol type filter\n"
	"onu packet ip protocol type filter\n"
	"please input ip protocol type\n"
	)
    {

	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int  IpType;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}	

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}
	
	IpType = ( unsigned short int ) VOS_AtoL(argv[0]);

	ret = AddOnuIpTypeFilter( phyPonId, userOnuId , IpType );				
	if( ret ==  V2R1_ONU_NOT_EXIST ) 
		{
		vty_out( vty, " %% onu %d/%d/%d is not exist\r\n", slotId, port, ulOnuId );
		return( CMD_WARNING );
		}
	else if( ret == V2R1_ONU_FILTER_IP_PROT_EXIST ) 
		{
		vty_out( vty, " %% the Ip protocol type 0x%x is already in filter table\r\n", IpType );
		return( CMD_WARNING );
		}
	else if( ret == RERROR ) 
		{
		vty_out( vty, "  %% set onu%d/%d/%d ip protocol type filter err\r\n",slotId,port,ulOnuId);
		return CMD_WARNING;  
		}

	return CMD_SUCCESS;
}

DEFUN( 
	undo_onu_IpTypefilter_config,
	undo_onu_IpTypefilter_config_cmd,
	"undo onu iptype filter {<1-65535>}*1",
	"clear a filter rule\n"
	"clear a filter rule\n"
	"clear ip protocol type filter\n"
	"onu packet ip protocol filter\n"
	"please input ip protocol type\n"
	)
    {
	ULONG ulIfIndex = 0;	
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int IpType;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId) == VOS_ERROR )
    		return CMD_WARNING;
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	userOnuId = (short int)(ulOnuId - 1);
	if( userOnuId >( MAXONUPERPON -1) )
		{
		vty_out( vty," %% onuId parameter is error\r\n");
		return( CMD_WARNING);
		}

	if( argc == 1 )
		{
		IpType = ( unsigned short int ) VOS_AtoL(argv[0]);
		}
	
	if( argc == 0 )   /* 参数中没有特定的IP  protocol type */
		{
		ret = ClearOnuIpTypeFilterAll( phyPonId, userOnuId );
		}
	
	else if( argc ==1)  /* 参数中有特定的IP protocol type */
		{
		ret = ClearOnuIpTypeFilter(phyPonId,  userOnuId, IpType );
		if( ret == V2R1_ONU_NOT_EXIST ) 
			{
			vty_out( vty, " %% onu %d/%d/%d not exist\n", slotId, port, ulOnuId );
			return( CMD_WARNING );
			}
		else if( ret == V2R1_ONU_FILTER_IP_PROT_NOT_EXIST )
			{
			vty_out( vty, " %% the IP protocol type 0x%x is not in the filter table\n", IpType);
			}
		else if( ret == RERROR ) 
			{
			vty_out( vty, "  %% clear onu%d/%d/%d IP protocol type filter err\r\n", slotId, port, ulOnuId);
			return CMD_WARNING;  
			}
		}				
		
	return CMD_SUCCESS;
}
#endif

#ifdef  ONU_DEVICE_DEACTIVATE 
#endif

/* added by xieshl 20111011, 统一去激活ONU命令实现；不管CTC协议栈是否打开，去激活ONU时都不再依据ONU认证功能实现，
    而是直接将ONU放入pending队列，以避免ONU反复离线注册。问题单13516 */
LONG cli_set_onu_traffic_service_enable( struct vty *vty, ULONG ulSlot, ULONG ulPort, ULONG ulOnuid, LONG enable )
{
    	INT16 phyPonId = 0;
	int result = 0;

	if( V2R1_DISABLE != enable )
		enable = V2R1_ENABLE;

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
		return CMD_WARNING;
	
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	

	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
		{
		vty_out(vty, "  %d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}
	*/
		
	if ((ulOnuid == 0) || (ulOnuid > MAXONUPERPON))
		{
		vty_out( vty, "  %% onuidx%d/%d/%d error. \r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;	
		}

	if(ThisIsValidOnu( phyPonId, (ulOnuid-1))  != ROK )
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
		}

	/*if( V2R1_CTC_STACK )*/
	/*if(GetOnuVendorType( phyPonId, (ulOnuid - 1 ) ) == ONU_VENDOR_CT )
		{
		unsigned long auth_mode;
		getOnuAuthEnable(&auth_mode);
		if( auth_mode == V2R1_ONU_AUTHENTICATION_DISABLE )
			{
			vty_out(vty, " ONU authentication should be enable first\r\n");
			return( CMD_WARNING );
			}		
		}*/

	result = SetOnuTrafficServiceEnable( phyPonId, (ulOnuid - 1 ), enable );
	if( result ==  V2R1_ONU_NOT_EXIST )
		{
		vty_out( vty, "  %% onu %d/%d/%d is not exist\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	else if(  result == ONU_OPER_STATUS_DOWN  ) 
		{
		vty_out( vty, "  %% onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	else if( result == RERROR )
		{
		vty_out( vty, "  %% Executing error \r\n");
		return( CMD_WARNING );
		}

	return CMD_SUCCESS;
}
	/* added by chenfj 2007-6-27 
	    ONU	 device  de-active */

DEFUN (
	onu_node_deactivate_onu_device,
	onu_node_deactivate_onu_device_cmd,
	"deactivate onu traffic service",
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
	ULONG ulIfIndex=0;
    	/*INT16 phyPonId = 0;
	int result = 0;*/
	LONG lRet;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

/* modified by xieshl 20111011, 统一去激活ONU命令实现，问题单13516 */
#if 0
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
		}	

	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
		{
		vty_out(vty, "  %d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}
	*/
		
	if ((ulOnuid == 0) || (ulOnuid > MAXONUPERPON))
		{
		vty_out( vty, "  %% onuidx%d/%d/%d error. \r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;	
		}

	if(ThisIsValidOnu( phyPonId, (ulOnuid-1))  != ROK )
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
		}

	/* modified by xieshl 20111010, 不管CTC协议栈是否打开，去激活ONU时都不再依据ONU认证功能实现，
	    而是直接将ONU放入pending队列，以避免ONU反复离线注册。问题单13516 */
	/*if( V2R1_CTC_STACK )*/
	/*if(GetOnuVendorType( phyPonId, (ulOnuid - 1 ) ) == ONU_VENDOR_CT )
		{
		unsigned long auth_mode;
		getOnuAuthEnable(&auth_mode);
		if( auth_mode == V2R1_ONU_AUTHENTICATION_DISABLE )
			{
			vty_out(vty, " ONU authentication should be enable first\r\n");
			return( CMD_WARNING );
			}		
		}*/

	result = SetOnuTrafficServiceEnable( phyPonId, (ulOnuid - 1 ), V2R1_DISABLE );
	if( result ==  V2R1_ONU_NOT_EXIST )
		{
		vty_out( vty, "  %% onu %d/%d/%d is not exist\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	else if(  result == ONU_OPER_STATUS_DOWN  ) 
		{
		vty_out( vty, "  %% onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	else if( result == RERROR )
		{
		vty_out( vty, "  %% Executing error \r\n");
		return( CMD_WARNING );
		}

	return CMD_SUCCESS;
#else
	return cli_set_onu_traffic_service_enable( vty, ulSlot, ulPort, ulOnuid, V2R1_DISABLE );
#endif
}

DEFUN (
	undo_onu_node_deactivate_onu_device,
	undo_onu_node_deactivate_onu_device_cmd,
	"undo deactivate onu traffic service",
	NO_STR
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
	ULONG ulIfIndex=0;
    	/*INT16 phyPonId = 0;
	int result = 0;*/
	LONG lRet;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

/* modified by xieshl 20111011, 统一去激活ONU命令实现，问题单13516 */
#if 0
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
		}	

	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
		{
		vty_out(vty, "  %d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}
	*/
	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/

	if ((ulOnuid< (CLI_EPON_ONUMIN+1)) || (ulOnuid> (CLI_EPON_ONUMAX+1)))
		{
		vty_out( vty, "  %% onuidx%d/%d/%d error. \r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;	
		}

	if(ThisIsValidOnu( phyPonId, (ulOnuid-1))  != ROK )
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
		}

	/* modified by xieshl 20111010, 不管CTC协议栈是否打开，去激活ONU时都不再依据ONU认证功能实现，
	    而是直接将ONU放入pending队列，以避免ONU反复离线注册。问题单13516 */
	/*if( V2R1_CTC_STACK )*/
	/*if(GetOnuVendorType( phyPonId, (ulOnuid - 1 ) ) == ONU_VENDOR_CT )
		{
		unsigned long auth_mode;
		getOnuAuthEnable(&auth_mode);
		if( auth_mode == V2R1_ONU_AUTHENTICATION_DISABLE )
			{
			vty_out(vty, " ONU authentication should be enable first\r\n");
			return( CMD_WARNING );
			}		
		}*/

	result = SetOnuTrafficServiceEnable( phyPonId, (ulOnuid - 1 ), V2R1_ENABLE );
	if( result ==  V2R1_ONU_NOT_EXIST )
		{
		vty_out( vty, "  %% onu %d/%d/%d is not exist\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	else if(  result == ONU_OPER_STATUS_DOWN  ) 
		{
		vty_out( vty, "  %% onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	else if( result == RERROR )
		{
		vty_out( vty, "  %% Executing error \r\n");
		return( CMD_WARNING );
		}

	return CMD_SUCCESS;
#else
	return cli_set_onu_traffic_service_enable( vty, ulSlot, ulPort, ulOnuid, V2R1_ENABLE );
#endif
}

DEFUN (
	show_onu_eth_mac_counter,
	show_onu_eth_mac_counter_cmd,
	"show port-mac-counter <slot/port/onuid>",
	SHOW_STR
	"Show onu ethernet mac counter\n"
	"Please input the slot/port/onuid\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
	ULONG ulIfIndex=0;
	LONG lRet = 0;
    short int PonPortIdx = 0;
    short int OnuIdx = 0;
    char buf[200]={0};
    char num = 0xFF; 
    int i = 0;
    OnuEthPortCounter_t data;

	lRet = PON_ParseSlotPortOnu( argv[0], (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&ulOnuid);	
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot,ulPort,vty) != ROK)
		return(CMD_WARNING);

    PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
    OnuIdx = ulOnuid-1;
    
    if(OnuSupportEthPortMacCounter(PonPortIdx, OnuIdx) != VOS_OK)
    {
        vty_out(vty, " %s is not support this command!\r\n", argv[0]);
        return CMD_WARNING;
    }
    
    VOS_MemZero(&data, sizeof(OnuEthPortCounter_t));   
    if(VOS_OK == OnuMgt_GetAllEthPortMacCounter( PonPortIdx, OnuIdx, &data))
    {
        for(i=0;i<data.OnuPortNum;i++)
        {
            vty_out(vty, " port %d mac counter : %d\r\n", i+1, data.MacNum[i]);            
        }
    }
    return CMD_SUCCESS;
}
int EVENT_SYSLOG = 0;
int DebugOnuDeviceIdx = 0;
DEFUN(debug_onu_config,
        debug_onu_config_cmd,
        "debug onu-config {<slot/port/onuid>}*1",
        "Debugging functions\n"
        "Debugging onu-config\n"
        "Onu operation\n")
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
    LONG  iRlt = VOS_ERROR;
    
    if(argc)
    {	
        iRlt = PON_ParseSlotPortOnu( argv[0], (ULONG *)&ulSlot, (ULONG *)&ulPort, (ULONG *)&ulOnuid);	
    	if( iRlt != VOS_OK )
    		return CMD_WARNING;
        DebugOnuDeviceIdx = MAKEDEVID(ulSlot, ulPort, ulOnuid);
    }
    else
    {
        DebugOnuDeviceIdx = 0xFFFFFFFF;
    }
    EVENT_SYSLOG = V2R1_ENABLE;
    return CMD_SUCCESS;
}
DEFUN(nodebug_onu_config,
        nodebug_onu_config_cmd,
        "undo debug onu-config",
        "Negate a command or set its defaults\n"
        "Debugging functions\n"
        "Debugging onu-config\n")
{
    DebugOnuDeviceIdx = 0;
    EVENT_SYSLOG = 0;
    return CMD_SUCCESS;
}
DEFUN(show_debug_onu_config,
        show_debug_onu_config_cmd,
        "show debug onu-config",
        "Show running system information\n"
        "Debugging functions\n"
        "Debugging onu-config\n")
{
    if(EVENT_SYSLOG==V2R1_ENABLE)
    {
        if(DebugOnuDeviceIdx == 0xFFFFFFFF)
            vty_out(vty, "onu-config debug is enable for all onu!\r\n");
        else
            vty_out(vty, "onu-config debug is enable for onu%d/%d/%d!\r\n", GET_PONSLOT(DebugOnuDeviceIdx), GET_PONPORT(DebugOnuDeviceIdx), GET_ONUID(DebugOnuDeviceIdx));
    }
    else
    {
        vty_out(vty, "onu-config debug is disable!\r\n");
    }
    return CMD_SUCCESS;
}

DEFUN(add_onu_model_black_list_config,
        add_onu_model_black_list_config_cmd,
        "add onu-model blacklist <model>",
        "Cofig onu-model blacklist\n"
        "Cofig onu-model blacklist\n"
        "Cofig onu-model blacklist\n"
        "Please input the onu model(0x12345678)\n")
{
	ULONG onu_model = 0;
	
	if((VOS_StrnCmp(argv[0],"0x", 2) == 0 ) || (VOS_StrnCmp(argv[0],"0X", 2) == 0 ))
		onu_model = VOS_StrToUL( argv[0], NULL, 16 );
	else 
		onu_model = VOS_AtoL( argv[0] );

    if(AddOnuModelBlackList(onu_model) != VOS_OK)
    {
        vty_out(vty, "Execute command error!\r\n");
    }
    return CMD_SUCCESS;
}
DEFUN(del_onu_model_black_list_config,
        del_onu_model_black_list_config_cmd,
        "del onu-model blacklist <model>",
        "Cofig onu-model blacklist\n"
        "Cofig onu-model blacklist\n"
        "Cofig onu-model blacklist\n"
        "Please input the onu model(0x12345678)\n")
{
	ULONG onu_model = 0;
	
	if((VOS_StrnCmp(argv[0],"0x", 2) == 0 ) || (VOS_StrnCmp(argv[0],"0X", 2) == 0 ))
		onu_model = VOS_StrToUL( argv[0], NULL, 16 );
	else 
		onu_model = VOS_AtoL( argv[0] );
	
    if(DelOnuModelBlackList(onu_model) != VOS_OK)
    {
        vty_out(vty, "Execute command error!\r\n");
    }
    return CMD_SUCCESS;
}
DEFUN(show_onu_model_black_list_config,
        show_onu_model_black_list_config_cmd,
        "show onu-model blacklist",        
        "Show running system information\n"
        "show onu-model blacklist\n"
        "show onu-model blacklist\n")
{
    show_black_onu_model(vty);
    return CMD_SUCCESS;
}

#if 0
/*  
	此命令用于网通测试, 在PON 端口保护倒换时, 快速闪断ONU 数据通路
*/
DEFUN (
	pon_signalLoss_hot_swap,
	pon_signalLoss_hot_swap_cmd,
	"pon signal-loss <0-200>",
	"pon port config\n"
	"signal loss\n"
	"time length, unit:10ms\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
    	short int  PonPortIdx = 0, OnuIdx = 0;
	ULONG ulIfIndex=0;
	ULONG lRet;
	int time_length;
	short int Llid;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );

	PonPortIdx = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE
	if(GetOnuOperStatus(PonPortIdx,OnuIdx) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty," %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort, ulOnuid);
		return( CMD_WARNING);
		}
	Llid = GetLlidByOnuIdx(PonPortIdx, ulOnuid-1 );
	if(Llid == INVALID_LLID)
		return(CMD_WARNING);
	
	time_length = (int)VOS_AtoL(argv[0]);
	REMOTE_PASONU_uni_set_port( PonPortIdx, Llid, 1, 0 );
	if(time_length != 0)
	VOS_TaskDelay(time_length);
	REMOTE_PASONU_uni_set_port( PonPortIdx, Llid, 1, 1);
	        
    return CMD_SUCCESS;
}
#endif
/*  add by chenfj 2008-1-22
    ONU节点下,增加显示ONU版本的命令
    modified by chenfj 2008-2-2
    将这些信息合并到show device information中
    */
#if 0
DEFUN  (
    onu_show_device_version,
    onu_show_device_version_cmd,
    "show onu version",
    DescStringCommonShow
    "show onu version\n"
    "show onu version\n"
    )
{
	ULONG slotId = 0;
	ULONG onuId = 0; 
	ULONG port = 0;
	ULONG ulIfIndex = 0;
	short int PonPortIdx = 0;
	short int OnuIdx = 0;
	short int OnuEntry;
	unsigned char InfoString[256] = {0};
	short int llid;
	PAS_device_versions_t  device_versions_struct;
	unsigned short int PonChipType;

	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
	PonPortIdx = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	if (PonPortIdx == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	
	if( getPonChipInserted((unsigned char)(slotId),(unsigned char) (port) ) != PONCHIP_EXIST )
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", slotId, port );
		return(CMD_WARNING);
		}
	
	OnuIdx = onuId - 1;
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
		{
		vty_out(vty,"  %% onu%d/%d/%d is not exist\r\n", slotId, port, onuId );
		return( CMD_WARNING);
		}
	if(GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty,"  %% onu%d/%d/%d is off-line\r\n", slotId, port, onuId );
		return( CMD_WARNING);
		}
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	
	llid = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
	if(llid == INVALID_LLID )
		{
		vty_out(vty,"  %% onu%d/%d/%d is not exist\r\n", slotId, port, onuId );
		return( CMD_WARNING);
		}

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);

	if((PonChipType == PONCHIP_PAS5001) ||(PonChipType == PONCHIP_PAS5201))
		{
		if ( PAS_get_device_versions_v4 ( PonPortIdx,
															  llid,
															  &device_versions_struct ) == PAS_EXIT_OK)
			{
					vty_out(vty,"\r\n");
					
					strcpy (InfoString,"ponchip version");
					vty_out(vty,"%-18s", InfoString);
					vty_out(vty,": ");				
					
					if (device_versions_struct.hardware_major == 0x1)
					{
						switch(device_versions_struct.hardware_minor)
						{
						case(0x0):
							vty_out(vty,"PAS6001-A,B");
							break;
						case(0x1):
							vty_out(vty,"PAS6001-N");
							break;
						case(0x2):
							vty_out(vty,"PAS6001-N M3");
							break;
						default:
							vty_out(vty,"PAS6001 Hradware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
							break;
						}
					} else if (device_versions_struct.hardware_major == 0x6201)
					{
						switch(device_versions_struct.hardware_minor)
						{
						case(0x0):
							vty_out(vty,"PAS6201-A0");
							break;
						case(0x1):
							vty_out(vty,"PAS6201-A1");
							break;
						case(0x2):
							vty_out(vty,"PAS6201-A2");
							break;
						default:
							vty_out(vty,"PAS6201 Hradware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
							break;
						}
					}
					else vty_out(vty,"PAS6301");
			}
		}
	else {	/* other pon chip handler */

		}
	
	vty_out(vty,"\r\n");	
	/*vty_out(vty,"onu%d/%d/%d version:\r\n", slotId, port, onuId);*/
	{
		strcpy (InfoString,"boot version");
		vty_out(vty,"%-18s", InfoString);
		vty_out(vty,": ");	
		VOS_MemCpy( InfoString, OnuMgmtTable[OnuEntry].DeviceInfo.BootVersion,OnuMgmtTable[OnuEntry].DeviceInfo.BootVersionLen);
		InfoString[OnuMgmtTable[OnuEntry].DeviceInfo.BootVersionLen] = '\0';
		vty_out(vty,"%s\r\n", InfoString );
	}

	{
		strcpy (InfoString,"hardware version");
		vty_out(vty,"%-18s", InfoString);
		vty_out(vty,": ");	
		VOS_MemCpy( InfoString, OnuMgmtTable[OnuEntry].DeviceInfo.HwVersion,OnuMgmtTable[OnuEntry].DeviceInfo.HwVersionLen);
		InfoString[OnuMgmtTable[OnuEntry].DeviceInfo.HwVersionLen] = '\0';
		vty_out(vty, "%s\r\n", InfoString );
	}

	{
		strcpy (InfoString,"firmware version");
		vty_out(vty,"%-18s", InfoString);
		vty_out(vty,": ");
		VOS_MemCpy( InfoString, OnuMgmtTable[OnuEntry].DeviceInfo.FwVersion,OnuMgmtTable[OnuEntry].DeviceInfo.FwVersionLen);
		InfoString[OnuMgmtTable[OnuEntry].DeviceInfo.FwVersionLen] = '\0';
		vty_out(vty, "%s\r\n", InfoString );
	}

	{
		strcpy (InfoString,"software version");
		vty_out(vty,"%-18s", InfoString);
		vty_out(vty,": ");
		VOS_MemCpy( InfoString, OnuMgmtTable[OnuEntry].DeviceInfo.SwVersion,OnuMgmtTable[OnuEntry].DeviceInfo.SwVersionLen);
		InfoString[OnuMgmtTable[OnuEntry].DeviceInfo.SwVersionLen] = '\0';
		
		if(OnuIsGT831( PonPortIdx, OnuIdx ) == ROK )
			{
			char *Ptr;
			char temp[20];

			Ptr = VOS_StrStr( InfoString, "/");
			*Ptr = '\0';
			vty_out(vty, "%s\r\n", InfoString );
			Ptr++;
			
			strcpy (temp,"voice-app version");
			vty_out(vty,"%-18s", temp);
			vty_out(vty,": ");
			vty_out(vty, "%s\r\n", Ptr );
			}
		else
			vty_out(vty, "%s\r\n", InfoString );
	}

	vty_out(vty,"oam version:V%d.0,%s\r\n", OnuMgmtTable[OnuEntry].OAM_Ver, OAMVersion_s[OnuMgmtTable[OnuEntry].OAM_Ver]);

	vty_out(vty,"\r\n");
    return CMD_SUCCESS;
}
#endif

#ifdef  _ONU_CLI_VCONSOLE_
#endif
/*  added by chenfj 2008-10-15
      在ONU节点下(CTC ONU除外)添加命令,用于创建ONU 命令行虚拟终端
      */
extern int g_gwonu_pty_flag;           
extern int cl_pty_client_create( struct vty * vty, long lMainDev, long lSubDev, char *newPromptStr);
extern int cl_ctc_pty_client_create( struct vty * vty, USHORT vid, char * destIP , long ponid, long onuid);
extern LONG MN_Vlan_Get_AllPorts(ULONG  vlanIndex , CHAR * portList , ULONG * portlist_len);
extern LONG MN_Vlan_Get_UntagPorts(ULONG  vlanIndex , CHAR * portList , ULONG * portlist_len);
extern LONG MN_Vlan_Set_TaggedPorts(ULONG  vlanIndex , CHAR * portList , ULONG portlist_len);
extern LONG MN_Vlan_Get_Vid( USHORT* pusVid, CHAR* pcName, USHORT usLang, CHAR* pcError);
extern LONG MN_Vlan_Get_Ip_Mask( USHORT usVid, ULONG* p_Ip, ULONG* pulMask, USHORT usLang, CHAR* pcError );
extern int CTC_ONU_IS_811C(short int olt_id, short int onu_id);

/*缓存建立连接时olt的ip地址，用于异常检查2012-4-12*/
int ONU_CONNECT_IS_DOWN(long ponid, long onuid, USHORT vid)
{
    int ret = CTC_ONU_TELNET_DOWN;
    CTC_STACK_mxu_mng_global_parameter_config_t para;
    char pcError[256];
    char name[20] = "telnet";
    int slot = GetCardIdxByPonChip((short int)ponid);
    int port = GetPonPortByPonChip((short int)ponid);
    int loop = 0;
    ULONG devidx = 0, liv_ipaddr = 0;
    ULONG uInterface_ip = 0, uInterface_mask = 0;
    USHORT uInterfave_vid = 0;
    
    for(loop=0;loop<CTC_ONU_MAX_PTY_CONNECTION;loop++)
    {
        if(g_ctconu_pty_limit[loop].Onu_DevId)
        {
            int g_slot = 0, g_port = 0, g_onuid = 0;
            
            g_slot = GET_PONSLOT(g_ctconu_pty_limit[loop].Onu_DevId);
            g_port = GET_PONPORT(g_ctconu_pty_limit[loop].Onu_DevId);
            g_onuid = GET_ONUID(g_ctconu_pty_limit[loop].Onu_DevId);
            if(g_slot == slot && g_port == port && g_onuid == onuid)
                break;
        }
    }
    if(loop<CTC_ONU_MAX_PTY_CONNECTION)
        liv_ipaddr = g_ctconu_pty_limit[loop].OLT_IpAddr;
    
    /*  新增加对olt 侧vlan 检查，多用户误修改telnet vlan造成的连接断开added by luh 2012-1-17
        *  onu在线，且主控和onu 的ip、vlan信息未作修改，则表明连接未断开，继续等待*/
    if(MN_Vlan_Get_Vid(&uInterfave_vid, name, 1, pcError) == 0)
    {
        if(uInterfave_vid == vid)
        {
        	if ( ONU_OPER_STATUS_UP  == GetOnuOperStatus( ponid, onuid-1))
        	{
                if(OnuMgt_GetMxuMngGlobalConfig(ponid, onuid-1, &para) == VOS_OK)
                {
                    char ip_str[4];
                    ULONG destIP = 0;
                    if(MN_Vlan_Get_Ip_Mask(vid, &uInterface_ip, &uInterface_mask, 1, pcError) == 0 && uInterface_ip && uInterface_mask)
                    {
                        if(liv_ipaddr == uInterface_ip && uInterface_mask == 0xffff8000)/*从建立连接到现在，'telnet'  vlan 中ip是否被修改*/
                        {
                            uInterface_ip &= uInterface_mask; 
                            ip_str[2]= onuid+10;
                            ip_str[3]= ponid+1;
                            destIP = uInterface_ip |(ip_str[2]<<8)|(ip_str[3]);
                            if(vid == para.data_cvlan && destIP == para.mng_ip)
                                ret = CTC_ONU_TELNET_UP;                            
                        }
                    }
                }
            }
        }
    }
    return ret;
}
int IsAPrivateIpAddress(ULONG addr)
{
    ULONG lmax_addr = 0, lmin_addr = 0,local_mask = 0;
    
    lmax_addr = (10<<24)|(0xff<<16)|(0xff<<8)|(0xff);
    lmin_addr = (10<<24);
    if(addr<lmin_addr || addr>lmax_addr)
        return VOS_ERROR;
    return VOS_OK;
}
int IsBPrivateIpAddress(ULONG addr)
{
    ULONG lmax_addr = 0, lmin_addr = 0,local_mask = 0;
    
    lmax_addr = (172<<24)|(31<<16)|(0xff<<8)|(0xff);
    lmin_addr = (172<<24)|(16<<16);
    if(addr<lmin_addr || addr>lmax_addr)
        return VOS_ERROR;
    return VOS_OK;
}
int CheckOltTelnetVlanIpAddress(ULONG addr)
{
	int ret = VOS_ERROR;
	ULONG bit3 = (addr>>8)&0xff;
	ULONG bit4 = addr&0xff;
	if(bit3>0&&bit3<=10)
		ret = VOS_OK;
	return ret;		
}
DEFUN (
	onu_cli_vconsole_client,
	onu_cli_vconsole_client_cmd,
	"pty",
	"create onu cli vconsole client\n"
	)
{
	ULONG slotId = 0;
	ULONG onuId = 0; 
	ULONG port = 0;	
	ULONG ulIfIndex = 0;	
	short int  PonPortIdx = 0;
	short int OnuIdx= 0;
	LONG lRet = VOS_OK;
	unsigned char prompt[ONU_TYPE_LEN+20];
	int stringLen = 0;
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;	

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;
	PonPortIdx = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	OnuIdx = onuId-1;
	
	CHECK_ONU_RANGE
	lRet = 	GetOnuOperStatus( PonPortIdx, OnuIdx);
	if ( ONU_OPER_STATUS_UP  != lRet)
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",slotId,  port, onuId) ;
		return CMD_WARNING;
	}
    /*添加CTC-ONU 支持added by luh 2012-1*/
    if(IsCtcOnu(PonPortIdx,OnuIdx)&&!g_gwonu_pty_flag)
    {
        int ret,i=0,k=0;
        char ipstr[20] = "";
        char pcError[256];
        ULONG pty_ipaddr = 0;
        char name[20] = "telnet";/*系统初始化时会创建一个约定好的，名为telnet 的vlan，默认4094，vid 可修改*/
        UCHAR  allportlist[MN_SNMP_MAX_PORTLIST_LEN];
        UCHAR  untaggedportlist[MN_SNMP_MAX_PORTLIST_LEN];
        UCHAR  tagportlist[MN_SNMP_MAX_PORTLIST_LEN];
        ULONG allportlistLen,untaggedportlistLen;
    	RPC_CTC_mxu_mng_global_parameter_config_t para;
        ULONG uInterface_ip = 0,uInterface_mask = 0;
        USHORT vid = 0;
        
        if (ctc_pty_is_connected(slotId,  port, onuId) == CTC_PTY_CONNECTION_IS_UP/*||(!CTC_ONU_IS_811C(PonPortIdx, OnuIdx))*/)/*同一个ONU 只允许建立1个连接added by luh 2012-2-3*/
        {
            vty_out(vty, "connect to onu fail, maybe onu don't support or locked by other users\r\n");
            return CMD_WARNING;
        }
        else if(ctc_pty_is_connected(slotId,  port, onuId) == CTC_PTY_CONNECTION_FULL_ERROR)
        {
            vty_out(vty, "So many connections have been created!\r\n");
            return CMD_WARNING;
        }
        
        /*检查OLT 端vlan-telnet 下是否添加本pon 口,vlan在初始化时创建，并绑定ip */
        if (MN_Vlan_Get_Vid(&vid, name, 1, pcError) == 0)
        {
            if ( MN_Vlan_Get_AllPorts(vid, allportlist, &allportlistLen) == 0
                && MN_Vlan_Get_UntagPorts(vid, untaggedportlist, &untaggedportlistLen) == 0)
            {
                for ( i = 0 ; i<Mib_PortList_Len ;i++)
        	    {
            		if((allportlist[i] & untaggedportlist[i]) != untaggedportlist[i])
            		{
            			return CMD_WARNING;
            		}
            		tagportlist[i] = allportlist[i] & (~(untaggedportlist[i]));
                }
    	        k = ((slotId - 1) * MN_SNMP_MAX_PORTS_ONE_SLOT + (port-1)) / 8;  /*定位到某一个字节*/
                i = ((slotId - 1) * MN_SNMP_MAX_PORTS_ONE_SLOT + (port-1)) % 8;  /*计算某个字节的多少位*/
                if(!(tagportlist[k]&(0x80>>i)))
                {
                    /*added by luh 2012-11-16, 问题单16305*/
                    VOS_MemZero(tagportlist, Mib_PortList_Len);
                    tagportlist[k] = (0x80>>i);
                    MN_Vlan_Set_TaggedPorts(vid, tagportlist, Mib_PortList_Len);
                }
            }    
            else
            {
                return CMD_WARNING;
            }
        }
        else
        {
            vty_out(vty, "The Vlan supported this connection is not exist,please create the vlan named 'telnet' first!\r\n");            
            return CMD_WARNING;
        }
        /*end*/
        if(MN_Vlan_Get_Ip_Mask(vid, &uInterface_ip, &uInterface_mask, 1, pcError) == 0 && uInterface_ip && uInterface_mask)
        /*向onu 发送待配置的ip 、vlan_id*/
    	{  
            char ip_str[4];
            char ip_mask = 16;
            
            /*modi by luh 2012-5-8 。问题单13453*/
            /*C 网段的IP 主机最多只能容纳2^8 = 256，而规划OLT 所包含的onu远不是C网段私有IP能解决的
                      *所以排除C网段。B网段能够容纳6万台主机，而A网段更多。基于Onu索引(GFA6900) 的IP 划分
                      *最少占用14 个bit，预留一位给OLT配置。故选取17 位掩码  */
            /*modi by luh 2012-8-2  Olt vlan "telnet" 中的ip地址与规则生成的onu的ip地址做出区分，分别取不同区间
            		  *避免ip地址冲突*/
            if(IsAPrivateIpAddress(uInterface_ip) == VOS_ERROR && IsBPrivateIpAddress(uInterface_ip) == VOS_ERROR)
            {
                vty_out(vty, "The ip added to 'telnet' vlan must be a private address of the segment A/B!\r\n");            
                vty_out(vty, "e.g. x.x.1.1/17 ～ x.x.10.255/17\r\n");            
                return CMD_WARNING;
            }
            if(uInterface_mask!=0xffff8000)
            {
                vty_out(vty, "The ip added to 'telnet' vlan must be with 17-bit subnet mask!\r\n");            
                return CMD_WARNING;  
            }
			if(CheckOltTelnetVlanIpAddress(uInterface_ip) == VOS_ERROR)
			{
                vty_out(vty, "The ip address added to 'telnet' vlan must belong to x.x.1.1/17 ～ x.x.10.255/17\r\n");            
                return CMD_WARNING;
			}

            pty_ipaddr = uInterface_ip;
            uInterface_ip &= uInterface_mask;
            ip_str[2]=onuId;
            ip_str[3]=254;
        	para.mxu_mng.mng_gw = uInterface_ip |(ip_str[2]<<8)|(ip_str[3]);
            
            uInterface_ip &= uInterface_mask; 
            ip_str[2]=onuId+10;
            ip_str[3]=PonPortIdx+1;/*ONU 占用的IP 地址，最后一位从2开始算起，172.16.1.1是对应的OLT IP*/
            para.mxu_mng.mng_ip = uInterface_ip |(ip_str[2]<<8)|(ip_str[3]);
            /*获取创建telnet 任务所需的ip字符串*/
            VOS_Sprintf(ipstr, "%d.%d.%d.%d",(para.mxu_mng.mng_ip>>24)&0xff,(para.mxu_mng.mng_ip>>16)&0xff,(para.mxu_mng.mng_ip>>8)&0xff,para.mxu_mng.mng_ip&0xff);

            para.mxu_mng.mng_mask = uInterface_mask;
        	para.mxu_mng.data_cvlan = vid;
        	para.mxu_mng.data_svlan = 0;
        	para.mxu_mng.data_priority = 1;
            para.needSaveOlt = FALSE;   /* 此时不需保存配置，by liuyh, 2017-5-5 */            
            if (OnuMgt_SetMxuMngGlobalConfig(PonPortIdx, OnuIdx, &para) != VOS_OK)
            {
                return VOS_ERROR;   
            }
    	}
        else
        {
            vty_out(vty, "Please add a private address of the segment A/B to 'telnet' vlan with 17-bit subnet mask! e.g.172.16.1.1/17\r\n");            
            return CMD_WARNING;
        }
        /*end*/
        
        /*创建telnet 连接任务*/
        vty_buffer_reset(vty);
        set_pty_connection_status(PonPortIdx, onuId, pty_ipaddr); /*修改连接建立后的状态变量*/
        ret = cl_ctc_pty_client_create(vty, vid, ipstr, PonPortIdx, onuId);
        if ( ret > 0 )
        {
            return CMD_SUCCESS;
        }
        else
        {
            clr_pty_connection_status(PonPortIdx, onuId);
        }
    }
    /* modified by xieshl 20100201 */
    else
    {
    	VOS_MemZero( prompt, sizeof(prompt) );
    	if( GetOnuTypeString(PonPortIdx, OnuIdx, &prompt[0], &stringLen) == ROK )
    	{
    	       VOS_Sprintf( &prompt[stringLen],"-%d/%d/%d", slotId, port, onuId);
    		lRet = cl_pty_client_create(vty, PonPortIdx, onuId,prompt);
    		
    		if (TRUE == lRet)
    			return CMD_SUCCESS;
    	}
    }
	return CMD_WARNING;
}

/* added by xieshl 20110316, showrun */
static int onuShowRunFlag = 0;
static VOID into_onu_cfg_node( struct vty *vty, short int slotId, short int port, short int onuId )
{
	if( onuShowRunFlag == 0 )
	{
		onuShowRunFlag = 1;
		vty_out( vty, " onu %d/%d/%d\r\n", slotId, port, (onuId+1) );
	}
}

LONG ONU_show_run( ULONG ulIfindex, VOID * p )
{
	struct vty *vty = p;
	short int slotId = 0;  
	short int port = 0;
	short int onuId = 0;
	short int phyPonId = 0;
	int iRes = 0;

	MacTable_S  *FilterMac;
	IpTable_S  *FilterIp;
	Ip_Port_Table_S *FilterIpPort;
	unsigned short int udp_port, tcp_port;
	unsigned int IpAddr;
	unsigned char IpAddrStr[16];
	VlanId_Table_S  *Filter_Vid;
	Ip_Type_Table_S   *Filter_Iptype;
	Ether_Type_Table_S  *Filter_Ethertype;
	
	vty_out( vty, "!Onu config\r\n" );
	onuShowRunFlag = 0;

	/*此处用来取出onu 的配置数据与默认数据之间进行比较，有差异打印出来
	注意打印格式其实是相应的设置命令。因为ONU的数目多,需要一个遍历过程,
	把所有与默认配置不同的内容,都要采用相应的配置命令来打印输出一遍,
	相当于完成了一次配置.*/
	CLI_SLOT_SYSCLE
	{
		/*check the slot 4*/

		/*判断第四slot是否为pon板卡*/
		if(SlotCardMayBePonBoardByVty(slotId,NULL) != ROK ) continue;
		
		CLI_PORT_SYSCLE	
		{
			phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
			if (phyPonId == VOS_ERROR)
				continue;
			
			CLI_ONU_SYSCLE
			{
				if(ThisIsValidOnu(phyPonId, onuId ) != ROK )
					continue;
					
				{/*ONU 软件自动升级使能*/
					int defSoftUpdata = GetOnuSWUpdateCtrlDefault();
					int runSoftUpdata = GetOnuSWUpdateCtrl(phyPonId, onuId );

					if( (VOS_ERROR != runSoftUpdata) && (VOS_ERROR != defSoftUpdata) && (defSoftUpdata != runSoftUpdata) )
					{
						/*对比参数*/
						if(runSoftUpdata == CLI_EPON_ONUUPDATE_EN)
						{
							into_onu_cfg_node( vty, slotId, port, onuId );
							vty_out( vty, "  software update enable\r\n");	
						}
						else if (runSoftUpdata == CLI_EPON_ONUUPDATE_DIS)
						{
							into_onu_cfg_node( vty, slotId, port, onuId );
							vty_out( vty, "  undo software update enable\r\n");
						}
					}
				}
				/* modified by chenfj 2007-9-25
					ONU加密，密钥更新时间在show run中保存时保存了错误的
					字符串"encrypt-keytime"（不是配置更新密钥时间的命令），
					导致配置数据未能恢复
					modified by chenfj 2008-12-25
					将加密使能及密钥更新时间两个参数合并; 只要两个参数中有一个
					不是默认值, 那么就会在show run中显示此配置
				*/
				{
					unsigned int def_encrypt = GetOnuEncryptDefault();
					unsigned int run_encrypt = 0;
					unsigned int def_timeLen = GetOnuEncryptKeyExchagetimeDefault()/SECOND_1;
					unsigned int run_timeLen = 0;
					unsigned char runFlag = 0;
					
					GetOnuEncryptKeyExchagetime( phyPonId, onuId, &run_timeLen );
					GetOnuEncrypt( phyPonId, onuId, &run_encrypt );

					if((def_timeLen != run_timeLen ) || ( def_encrypt != run_encrypt))
					{
						if( def_encrypt != run_encrypt) runFlag =1;

						if( runFlag == 1 )
						{
							into_onu_cfg_node( vty, slotId, port, onuId );
							
							if (run_encrypt == PON_ENCRYPTION_DIRECTION_ALL)
								vty_out( vty, "  encrypt direction up-down update-key-interval");
							else if ( run_encrypt == PON_ENCRYPTION_DIRECTION_DOWN)
								vty_out( vty, "  encrypt direction down update-key-interval");

							if(def_timeLen != run_timeLen )
								vty_out(vty," %d\r\n", run_timeLen);
							else vty_out(vty," default\r\n");
						}
						else
						{
							if (def_timeLen != run_timeLen )
							{
								into_onu_cfg_node( vty, slotId, port, onuId );
								vty_out( vty, "  encrypt update-key-interval %d\r\n",run_timeLen);
							}
						}
					}
				}
				
				{/*history-statistics*/
					unsigned int status15m = 0;
					unsigned int status24h = 0;
					iRes = CliHisStatsONUStatusGet( phyPonId ,onuId,&status15m, &status24h);
					if (iRes == VOS_OK)
					{
						into_onu_cfg_node( vty, slotId, port, onuId );
						if ((status15m == 1) && (status24h == 1))
							vty_out( vty, "  statistic-history\r\n");
						else 
						{
							if (status15m == 1)
							vty_out( vty, "  statistic-history 15m\r\n");


							if (status24h == 1)
							vty_out( vty, "  statistic-history 24h\r\n");
						}
					}
				} 
				
#ifdef ONU_PEER_TO_PEER		
				{/*peer to peer*/
					int unicastFlag = 0;
					int def_unicastFlag = EPON_P2P_UNICAST_ENABLE;
					/*int def_brdFlag = EPON_P2P_BRDCAST_ENABLE;*/
					int brdFlag = 0;
					
					iRes = GetOnuPeerToPeerForward( phyPonId, onuId, &unicastFlag , &brdFlag );
					if (VOS_OK == iRes)
					{
						if ((unicastFlag ) && (brdFlag))
						{
							if ((unicastFlag == def_unicastFlag)/*||(brdFlag == def_brdFlag)*/)
							{
								into_onu_cfg_node( vty, slotId, port, onuId );
								vty_out( vty, "  p2p forward address-not-found %s\r\n", (def_unicastFlag == unicastFlag) ? "enable" : "" );
							}
						}
					}							
				}/**/
#endif		

			/* added by chenfj 2007-5-23 , ONU 支持的最大MAC 设置及FEC  设置*/
				{
					int def_MaxMac = MaxMACDefault;
					int Cur_MaxMac=0;

					/*if( V2R1_GetPonchipType(phyPonId ) == PONCHIP_PAS5201 )*/
					{
#if 0                            
						iRes = GetOnuMaxMacNum( phyPonId, onuId, &Cur_MaxMac );
						if( iRes == ROK ) 
						{
							if( Cur_MaxMac != def_MaxMac ) 
							{
								into_onu_cfg_node( vty, slotId, port, onuId );
								vty_out( vty, "  onu max-mac %d\r\n", Cur_MaxMac );
							}
						}
#else
                    	ONU_MGMT_SEM_TAKE;
                    	Cur_MaxMac = OnuMgmtTable[phyPonId*MAXONUPERPON+onuId].LlidTable[0].MaxMAC;
                    	ONU_MGMT_SEM_GIVE;
                        if(Cur_MaxMac & ONU_NOT_DEFAULT_MAX_MAC_FLAG)
                        {
							into_onu_cfg_node( vty, slotId, port, onuId );
							vty_out( vty, "  onu max-mac %d\r\n", Cur_MaxMac & (~ONU_NOT_DEFAULT_MAX_MAC_FLAG) );
                        }
#endif
					}
				}
				/*add by zhouzh 2016.6.25,onu超时自动删除设置*/
				{
					int Autodeleteflag;
					ONU_MGMT_SEM_TAKE;
                    Autodeleteflag = OnuMgmtTable[phyPonId*MAXONUPERPON+onuId].IsAutoDelete;
                    ONU_MGMT_SEM_GIVE;
					if(Autodeleteflag == 1)
					{
						into_onu_cfg_node( vty, slotId, port, onuId );
						vty_out( vty, "  onu isautodelete disable\r\n");
					}
				}

				{ /* FEC */
					int def_fec = STD_FEC_MODE_ENABLED;
					int fec_mode;

					iRes = GetOnuFecMode( phyPonId, onuId, &fec_mode );
					if( iRes == ROK )
					{
						if( fec_mode == def_fec)
						{
							into_onu_cfg_node( vty, slotId, port, onuId );
							if( fec_mode ==  STD_FEC_MODE_ENABLED )
								vty_out( vty, "  fec-mode enable\r\n");
						}
					}
				}

				/*ONU_MGMT_SEM_TAKE;*/

				/* added by chenfj 2007-6-1, ONU upstream数据包 原MAC 过滤*/
				FilterMac = OnuMgmtTable[phyPonId * MAXONUPERPON + onuId ].FilterSaMac ;

				while ( FilterMac != NULL )
				{				
					into_onu_cfg_node( vty, slotId, port, onuId );
					vty_out( vty, "  onu src-mac filter %02x%02x.%02x%02x.%02x%02x\r\n",
							FilterMac->MAC[0], FilterMac->MAC[1],FilterMac->MAC[2],FilterMac->MAC[3],FilterMac->MAC[4],FilterMac->MAC[5]);

					FilterMac = ( MacTable_S *)FilterMac->nextMac;
				}			

				/* added by chenfj 2006-6-13,  ONU 数据包IP/PORT 过滤*//* source ip */
				
				FilterIp = OnuMgmtTable[phyPonId * MAXONUPERPON + onuId].Filter_SIp;
				while( FilterIp != NULL )
				{
					into_onu_cfg_node( vty, slotId, port, onuId );
					VOS_MemSet( IpAddrStr, 0 , sizeof( IpAddrStr));
					IpAddr = FilterIp->ipAddr ;
					inet_ntoa_v4 ( IpAddr, IpAddrStr );
					vty_out(vty, "  onu src-ip filter %s\r\n", IpAddrStr );
					
					FilterIp = ( IpTable_S *) FilterIp->NextIp ;
				}

				/* destination ip */
				FilterIp = OnuMgmtTable[phyPonId * MAXONUPERPON + onuId].Filter_DIp;
				while( FilterIp != NULL )
				{
					into_onu_cfg_node( vty, slotId, port, onuId );
					VOS_MemSet( IpAddrStr, 0 , sizeof( IpAddrStr));
					IpAddr = FilterIp->ipAddr ;
					inet_ntoa_v4 ( IpAddr, IpAddrStr );
					vty_out(vty, "  onu dst-ip filter %s\r\n", IpAddrStr );
					
					FilterIp = ( IpTable_S *) FilterIp->NextIp ;
				}

				/* source udp */
				FilterIpPort = OnuMgmtTable[phyPonId * MAXONUPERPON + onuId].Filter_SIp_udp;
				while( FilterIpPort != NULL )
				{
					into_onu_cfg_node( vty, slotId, port, onuId );
					VOS_MemSet( IpAddrStr, 0 , sizeof( IpAddrStr));
					IpAddr = FilterIpPort->ipAddr ;
					udp_port = FilterIpPort->port ;
					/*inet_ntoa_v4 ( IpAddr, IpAddrStr );*/
					vty_out(vty, "  onu udp filter src-port %d\r\n", /*IpAddrStr,*/ udp_port );
					
					FilterIpPort = ( Ip_Port_Table_S *) FilterIpPort->Next_Ip_Port;
				}

				/* source tcp */
				FilterIpPort = OnuMgmtTable[phyPonId * MAXONUPERPON + onuId].Filter_SIp_tcp;
				while( FilterIpPort != NULL )
				{
					into_onu_cfg_node( vty, slotId, port, onuId );
					VOS_MemSet( IpAddrStr, 0 , sizeof( IpAddrStr));
					IpAddr = FilterIpPort->ipAddr ;
					tcp_port = FilterIpPort->port ;
					/*inet_ntoa_v4 ( IpAddr, IpAddrStr );*/
					vty_out(vty, "  onu tcp filter src-port %d\r\n", /*IpAddrStr,*/ tcp_port );
					
					FilterIpPort = ( Ip_Port_Table_S *) FilterIpPort->Next_Ip_Port;
				}
#if 0
				/* destinatioin udp */
				FilterIpPort = OnuMgmtTable[phyPonId * MAXONUPERPON + onuId].Filter_DIp_udp;
				while( FilterIpPort != NULL )
					{
					into_onu_cfg_node( vty, slotId, port, onuId )
					VOS_MemSet( IpAddrStr, 0 , sizeof( IpAddrStr));
					IpAddr = FilterIpPort->ipAddr ;
					udp_port = FilterIpPort->port;
					inet_ntoa_v4 ( IpAddr, IpAddrStr );
					vty_out(vty, "  access-list deny udp dst-ip %s dst-port %d\r\n", IpAddrStr, udp_port);
					
					FilterIpPort = ( Ip_Port_Table_S *) FilterIpPort->Next_Ip_Port ;
					}

				/* destinatioin tcp */
				FilterIpPort = OnuMgmtTable[phyPonId * MAXONUPERPON + onuId].Filter_DIp_tcp;
				while( FilterIpPort != NULL )
					{
					into_onu_cfg_node( vty, slotId, port, onuId )
					VOS_MemSet( IpAddrStr, 0 , sizeof( IpAddrStr));
					IpAddr = FilterIpPort->ipAddr ;
					tcp_port = FilterIpPort->port;
					inet_ntoa_v4 ( IpAddr, IpAddrStr );
					vty_out(vty, "  access-list deny tcp dst-ip %s dst-port %d\r\n", IpAddrStr, tcp_port);
					
					FilterIpPort = ( Ip_Port_Table_S *) FilterIpPort->Next_Ip_Port ;
					}
#endif


				/** added by chenfj 2007-6-12  增加ONU 数据流vlan id 过滤*/
				Filter_Vid = OnuMgmtTable[phyPonId * MAXONUPERPON + onuId].Filter_Vid;
				while( Filter_Vid != NULL )
				{
					into_onu_cfg_node( vty, slotId, port, onuId );
					vty_out(vty, "  onu vlanid filter %d\r\n", Filter_Vid->Vid );
					Filter_Vid = ( VlanId_Table_S *)Filter_Vid->NextVid;
				}

				/* add by chenfj 2007-6-15 增加ONU 数据流Ether type  &  ip protocol type */
				Filter_Iptype = OnuMgmtTable[phyPonId * MAXONUPERPON + onuId].Filter_Ip_Type;
				while( Filter_Iptype != NULL )
				{
					into_onu_cfg_node( vty, slotId, port, onuId );
					vty_out(vty, "  onu iptype filter %d\r\n", Filter_Iptype->IpType );
					Filter_Iptype = ( Ip_Type_Table_S   * )Filter_Iptype->Next_IpType ;
				}

				Filter_Ethertype = OnuMgmtTable[phyPonId * MAXONUPERPON + onuId].Filter_Ether_Type;
				while( Filter_Ethertype != NULL )
				{
					into_onu_cfg_node( vty, slotId, port, onuId );
					vty_out(vty, "  onu ethertype filter 0x%x\r\n", Filter_Ethertype->EtherType);
					Filter_Ethertype = ( Ether_Type_Table_S   * )Filter_Ethertype->Next_EthType;
				}

				/*ONU_MGMT_SEM_GIVE;*/

				/* added by chenfj 2007-6-27  
	     				ONU 激活、去激活操作*/
				{
				if( GetOnuTrafficServiceEnable( phyPonId, onuId ) == V2R1_DISABLE )
					{
					into_onu_cfg_node( vty, slotId, port, onuId );
					vty_out(vty, "  deactivate onu traffic service\r\n");
					}
				}

				/*added by wangxiaoyu 2011-08-30
				 * onu name save for ctc-onu*/
				/*removed by wangxiaoyu 2011-10-20
				 * cortina onu surpport set and get device name by gwd oam protocol*/
				/*
				{
				    int type = V2R1_ONU_CTC;
				    if(OnuMgmtTable[phyPonId * MAXONUPERPON + onuId].DeviceInfo.DeviceNameLen &&
				            GetOnuType(phyPonId, onuId, &type) == VOS_OK && type == V2R1_ONU_CTC)
				    {
				        into_onu_cfg_node(vty, slotId, port, onuId);
				        vty_out(vty, "device name %s\r\n", OnuMgmtTable[phyPonId * MAXONUPERPON + onuId].DeviceInfo.DeviceName);
				    }
				}
				*/

            	/*begin: 增加支持管理IP的配置加载，mod by liuyh, 2017-5-12*/
				{
                    ONUMngIPConf_t stOnuMngIp;
                    char ipStr[20] = "";
                    char gwStr[20] = "";
                    UCHAR ipM = 0;
                    
                    ONU_MGMT_SEM_TAKE;
                	memcpy(&stOnuMngIp, &OnuMgmtTable[phyPonId*MAXONUPERPON+onuId].mngIp, sizeof(stOnuMngIp));
                    ONU_MGMT_SEM_GIVE;
                    if (stOnuMngIp.ip != 0x00000000)  /* 缺省IP为0.0.0.0 */
                    {
                        get_ipdotstring_from_long(ipStr, stOnuMngIp.mask);
                        ipM = get_masklen_from_ipdotstring(ipStr);
                        if (ipM == -1)
                        {
                            ipM = 0;
                        }   

                        get_ipdotstring_from_long(ipStr, stOnuMngIp.ip);
                        get_ipdotstring_from_long(gwStr, stOnuMngIp.gw);

                        into_onu_cfg_node( vty, slotId, port, onuId );
                        vty_out(vty, "  ctc mxu-mng %s/%d %s %d %d %d\r\n", ipStr, ipM, 
                            gwStr, stOnuMngIp.cVlan, stOnuMngIp.sVlan, stOnuMngIp.pri);
                    }                	
				}
            	/*end: mod by liuyh, 2017-5-12*/                

				/*exit*/	
				if (onuShowRunFlag == 1)
				{
					onuShowRunFlag = 0;
					vty_out( vty, " exit\r\n");
				}	
			
			}/*end of CLI_ONU_SYSCLE*/
		}/*end of CLI_PORT_SYSCLE*/
	}/*end of CLI_SLOT_SYSCLE*/
	
	vty_out( vty, "!\r\n\r\n" );

	return VOS_OK;
}


int onu_init_func()
{
    return VOS_OK;
}

int onu_showrun( struct vty * vty )
{
    /*IFM_READ_LOCK;*/
    ONU_show_run( 0, vty );
	/*event_show_run(vty);*/	/* removed by xieshl 20110829 */
	/*
	CTC_onu_show_run(vty);
	*/
#if 0
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )
	onuAutoLoadShowRun(vty);/*add by shixh@20090219*//*问题单8456*/
#endif
#endif
    /*IFM_READ_UNLOCK;*/
    return VOS_OK;
}


int onu_config_write ( struct vty * vty )
{
    return VOS_OK;
}

int onu_profile_config_write ( struct vty * vty )
{
    return VOS_OK;
}



#define PON_STATE_SLOT 1
#define PON_STATE_PORT 2
#define PON_STATE_ONUID 3

LONG PON_ParseSlotPortOnu( CHAR * szName, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid)
{
    CHAR szNumStr[ 16 ];
    CHAR cToken;
    ULONG ulNamei = 0;
    ULONG ulNumi = 0;
    ULONG ulState = PON_STATE_SLOT;
    LONG lSlot = 0, lPort = 0, lOnuid=0;
    CHAR * szNewName = NULL;
    ULONG ulNameLen;

    if ( szName == NULL || pulSlot == NULL || pulPort == NULL )
    {
        return -IFM_E_NULLPINPARAM;
    }

    ulNameLen = VOS_StrLen( szName );
    szNewName = VOS_Malloc( ulNameLen + 2, MODULE_RPU_IFM );
    if ( szNewName == NULL )
    {
        return -IFM_E_NOMEM;
    }

    VOS_StrnCpy( szNewName, szName, ulNameLen + 1 );
    szNewName[ ulNameLen ] = ';'; /*和名称中的/分开*/
    szNewName[ ulNameLen + 1 ] = '\0';

    cToken = szNewName[ ulNamei ];
    while ( cToken != 0 )
    {
        switch ( ulState )
        {
            case PON_STATE_SLOT:
                if ( vos_isdigit( cToken ) )
                {
                    RECEIVE_NUMBER( cToken, szNumStr, ulNumi );
                }
                else if ( cToken == '/' )
                {
#ifndef _DISTRIBUTE_PLATFORM_
                    CALCULATE_NUMBER( lSlot, szNumStr, ulNumi, 0, ( MAXSlotNum - 1 ) );
#else
                    CALCULATE_NUMBER( lSlot, szNumStr, ulNumi, PONCARD_FIRST, PONCARD_LAST );                    
#endif
                    ulState = PON_STATE_PORT;
                }
                else
                {
                    goto error;
                }
                break;
            case PON_STATE_PORT:
                if ( vos_isdigit( cToken ) )
                {
                    RECEIVE_NUMBER( cToken, szNumStr, ulNumi );
                }
                else if ( cToken == '/' )
                {
#ifndef _DISTRIBUTE_PLATFORM_
                    CALCULATE_NUMBER( lPort, szNumStr, ulNumi , 0 , ( MAXPortOnSlotNum - 1 ) );
#else
                    CALCULATE_NUMBER( lPort, szNumStr, ulNumi , 1, PONPORTPERCARD );
#endif
                    ulState = PON_STATE_ONUID;
                }
                else
                {
                    goto error;
                }
                break;
            case PON_STATE_ONUID:
            	if ( vos_isdigit( cToken ) )
                {
                    RECEIVE_NUMBER( cToken, szNumStr, ulNumi );
                }
                else if ( cToken == ';' )
                {

                    CALCULATE_NUMBER( lOnuid, szNumStr, ulNumi , 1, MAXONUPERPONNOLIMIT);

                    ulState = 0;
                }
                else
                {
                    goto error;
                }
            	break;
            default:
                goto error;
        }
        ulNamei++;
        cToken = szNewName[ ulNamei ];
    }
    
    if( ulState != 0 )
    {
        goto error;
    }

    *pulSlot = ( ULONG ) lSlot;
    *pulPort = ( ULONG ) lPort;
    *pulOnuid = (ULONG )lOnuid;
	/*统一修改 x/x/x形式的onu索引*/
	if(lOnuid > MAXONUPERPON)
        goto error;
	
    VOS_Free( szNewName );
    return VOS_OK;

error:
    VOS_Free( szNewName );
    return -IFM_E_ETH_INVALSPORT;
}




enum match_type IFM_CheckSlotPortOnu( char * cPort )
{
    LONG lRet;
    ULONG ulSlot, ulPort, ulOnuid;

    lRet = PON_ParseSlotPortOnu( cPort, &ulSlot, &ulPort, &ulOnuid );
    if ( lRet != VOS_OK )
    {
        return no_match;
    }

    return exact_match;
}


CMD_NOTIFY_REFISTER_S stCMD_IFM_Pon_Port_Check =
{
    "<slot/port/onuid>",
    IFM_CheckSlotPortOnu,
    0
};

struct cmd_node onu_node =
{
    ONU_NODE,
    NULL,
    1
};

struct cmd_node onu_profile_node =
    { ONU_PROFILE_NODE, NULL, 1 };


#if 0
/*added by wutongw at 20 september*/
static INT16 cliOnuIdCheck(short int phyPonId, short int userOnuId)
{
    LONG lRet = VOS_OK;

    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
    {
    /*#ifdef CLI_EPON_DEBUG
    	vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
    #endif
	vty_out( vty, "  %% Power is down slot/port/onu %d/%d/%d.\r\n",phyPonId, userOnuId) ;
		*/
	return CMD_WARNING;
    }
    else if (VOS_ERROR == lRet)
    {
   /* #ifdef CLI_EPON_DEBUG
    	vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
    #endif    
       vty_out( vty, "  %% Parameter is error.\r\n" );
		*/
	 return CMD_WARNING;
    }	
	return VOS_OK;
}
#endif

#ifdef  _OEM_TYPE_CLI_
void  init_onu_cli_information(void)
{
	char *temp;
	
	/*convert onu file*/
	temp  = &convert_onu_file[0];
	temp = VOS_Sprintf(temp, "convert onu file command\n");
	temp = VOS_Sprintf(temp, "convert onu file\n");
	temp = VOS_Sprintf(temp, "if running %s mode, then update to CTC file; and if running CTC mode, then update to %s file\n", (&ProductCorporationName_AB[0]), (&ProductCorporationName_AB[0]));
}
#endif

LONG onu_node_install()
{
    install_node( &onu_node, onu_config_write);
    onu_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_ONU);
    if ( !onu_node.prompt )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    install_default( ONU_NODE );

    return VOS_OK;
}

LONG onu_module_init()
{
    struct cl_cmd_module * onu_module = NULL;

    onu_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_ONU);
    if ( !onu_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) onu_module, sizeof( struct cl_cmd_module ) );

    onu_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_ONU);
    if ( !onu_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( onu_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( onu_module->module_name, "onu" );

    onu_module->init_func = onu_init_func;
    onu_module->showrun_func = onu_showrun;
    onu_module->next = NULL;
    onu_module->prev = NULL;

    cl_install_module( onu_module );

    if ( cmd_rugular_register( &stCMD_IFM_Pon_Port_Check ) == no_match )
    {
        ASSERT( 0 );
    }

    return VOS_OK;
}

extern struct cmd_element show_running_config_cmd;
extern struct cmd_element show_onu_serial_number_cmd;

LONG ONU_NewType_CommandInstall(enum node_type node)
{
	install_element ( node, &onu_customer_location_config_cmd);
	install_element ( node, &onu_device_name_config_cmd);
	install_element ( node, &onu_device_description_config_cmd);
	install_element ( node, &onu_reset_cmd);
	install_element ( node, &onu_show_device_information_cmd);
	install_element ( node, &config_auto_delete_onu_cmd);
	
	install_element ( node, &onu_start_statistic_config_cmd);
	install_element ( node, &onu_no_statistic_config_cmd);
	install_element ( node, &onu_clear_statistic_data_cmd);
	install_element ( node, &clear_onu_real_time_statistic_data_cmd);  /*add by zhengyt@09-06-04*/
	
	install_element ( node, &onu_show_statistic_24h_data_cmd);		
	install_element ( node, &onu_show_statistic_15m_data_cmd);	
	install_element ( node, &onu_show_statistic_15mall_data_cmd);		
	install_element ( node, &onu_show_statistic_24hall_data_cmd);	
	install_element ( node, &onu_show_statistic_cmd);
	
	install_element ( node, &onu_show_fdb_mac_config_cmd);
	install_element( node, &onu_show_fdb_mac_counter_cmd);

	install_element ( node, &onu_encrypt_config_cmd);		
	install_element ( node, &onu_encrypt_keytime_config_cmd);			
	install_element ( node, &onu_no_encrypt_config_cmd);			
	install_element ( node, &onu_show_encrypt_config_cmd);

	install_element( node, &onu__fec_config_cmd );
	install_element( node, &show_fec__ability_cmd );
	install_element( node, &show_onu__fec_config_cmd );

	install_element ( node, &onu_maxmac_show_cmd);
	install_element ( node, &onu_maxmac_config_cmd);
	install_element ( node, &undo_onu_maxmac_config_cmd);

	install_element ( node, &onu_node_deactivate_onu_device_cmd );
	install_element ( node, &undo_onu_node_deactivate_onu_device_cmd );

    /* B--added by liwei056@2011-2-16 for D11920 */
	install_element ( node, &onu_auto_update_config_cmd);
  	install_element ( node, &onu_auto_update_disable_cmd);  
    /* E--added by liwei056@2011-2-16 for D11920 */

	install_element ( node, &onu_statistics_show_cmd);

#ifdef ONU_PEER_TO_PEER	
	install_element ( node, &peer_to_peer_rule_cmd);
	install_element ( node, &peer_to_peer_rule_show_cmd);
#endif
	
	install_element ( node, &onu_cli_vconsole_client_cmd );

	install_element (node, &show_onu_serial_number_cmd);

	install_element( node, &onu_pon_loopback_config_cmd );
	install_element( node, &onu_pon_loopback_show_cmd );
	
	 return VOS_OK;
}

LONG ONU_CommandInstallByType( enum node_type  node)
{

	install_element ( node, &onu_customer_location_config_cmd);
	install_element ( node, &onu_device_name_config_cmd);
	install_element ( node, &onu_device_description_config_cmd);
	/*install_element ( node, &onu_deregister_cmd);*/
	install_element ( node, &config_auto_delete_onu_cmd);
	install_element ( node, &onu_auto_update_config_cmd);
  	install_element ( node, &onu_auto_update_disable_cmd);  

	install_element ( node, &onu_reset_cmd);
	install_element ( node, &onu_update_file_cmd);
	install_element ( node, &onu_show_software_auto_update_cmd);
	/*install_element ( node, &onu_show_device_information_cmd);*/
	install_element ( node, &onu_start_statistic_config_cmd);
	install_element ( node, &onu_no_statistic_config_cmd);

	/*begin:
	commented by wangxiaoyu 2008-01-22 
	end*/
	/*install_element ( node, &onu_show_statistic_bucket_cmd);*/
		
	install_element ( node, &onu_clear_statistic_data_cmd);
	install_element ( node, &clear_onu_real_time_statistic_data_cmd);  /*add by zhengyt@09-06-04*/

	/*install_element ( node, &onu_alarm_config_cmd);*/
	/*install_element ( node, &onu_alarm_threshold_config_cmd);*/
	/*install_element ( node, &onu_show_alarm_info_cmd);*/
	/*install_element ( node, &onu_create_fdb_cmd);*/
	/*install_element ( node, &onu_show_mac_information_cmd); */  
	/*added by wutw at 13 september*/
	/*install_element ( node, &onu_show_fdb_llid_config_cmd);*/
	install_element ( node, &onu_encrypt_config_cmd);		
	install_element ( node, &onu_encrypt_keytime_config_cmd);			
	install_element ( node, &onu_no_encrypt_config_cmd);			
	install_element ( node, &onu_show_encrypt_config_cmd);

	install_element ( node, &onu_show_device_information_cmd);	

	install_element ( node, &onu_show_statistic_24h_data_cmd);		
	install_element ( node, &onu_show_statistic_15m_data_cmd);	
	install_element ( node, &onu_show_statistic_15mall_data_cmd);		
	install_element ( node, &onu_show_statistic_24hall_data_cmd);	
	/*install_element ( node, &onu_history_15m_time_interval_config_cmd);
	install_element ( node, &onu_history_24h_time_interval_config_cmd);*/

	/*added by wutw 2006/11/12*/
	install_element ( node, &onu_no_statistic_cycle_config_cmd);
	install_element ( node, &onu_start_statistic_cycle_config_cmd);

	/*added by wutw 2006/11/14*/
	install_element ( node, &onu_show_statistic_cmd);
	install_element ( node, &onu_show_fdb_mac_config_cmd);
	install_element( node, &onu_show_fdb_mac_counter_cmd);
	
	/*added by wutw 2006/11/28*/
#ifdef ONU_PEER_TO_PEER	
	install_element ( node, &peer_to_peer_rule_cmd);
	install_element ( node, &peer_to_peer_rule_show_cmd);
#endif
	/*added by wutw 2007/1/23*/
	install_element ( node, &onu_statistics_show_cmd);
	/*install_element ( node, &onu_olt_downlink_ber_show_cmd);*/

	/* added by chenfj 2007-5-23 */
	install_element( node, &onu__fec_config_cmd );
	install_element( node, &show_fec__ability_cmd );
	install_element( node, &show_onu__fec_config_cmd );

	install_element ( node, &onu_maxmac_show_cmd);
	install_element ( node, &onu_maxmac_config_cmd);
	install_element ( node, &undo_onu_maxmac_config_cmd);
#if 0	/* for CNC */
	/* add by chenfj 2007-6-1 */
	install_element( node, &onu_SAmac_filter_config_cmd );
	install_element( node, &undo_onu_SAmac_filter_config_cmd );
	install_element( node, &show_onu_SAmac_filter_config_cmd );

	/* added by chenfj 2007-6-12 */
	install_element( node, &onu_Ipfilter_config_cmd );
	install_element( node, &undo_onu_Ipfilter_config_cmd );
	install_element( node, &onu_SIpUdpfilter_config_cmd );
	install_element( node, &undo_onu_SIpUdpfilter_config_cmd );
	install_element( node, &onu_SIpTcpfilter_config_cmd );
	install_element( node, &undo_onu_SIpTcpfilter_config_cmd );
	/*
	install_element( node, &onu_DIpUdpfilter_config_cmd );
	install_element( node, &undo_onu_DIpUdpfilter_config_cmd );
	install_element( node, &onu_DIpTcpfilter_config_cmd );
	install_element( node, &undo_onu_DIpTcpfilter_config_cmd );
	*/
	
	/* added by chenfj 2006-7-14 */
	install_element( node, &onu_vlanIdfilter_config_cmd );
	install_element( node, &undo_onu_vlanIdfilter_config_cmd );

	/* added by chenfj 2006-7-15 */
	install_element( node, &onu_EtherTypefilter_config_cmd );
	install_element( node, &undo_onu_EtherTypefilter_config_cmd );
	install_element( node, &onu_IpTypefilter_config_cmd);
	install_element( node, &undo_onu_IpTypefilter_config_cmd);
	install_element( node, &show_onu_IPUDPTCPfilter_config_cmd);
#endif

	/* added by chenfj 2007-6-27  
     ONU 激活、去激活操作*/
	install_element ( node, &onu_node_deactivate_onu_device_cmd );
	install_element ( node, &undo_onu_node_deactivate_onu_device_cmd );

	/* added by chenfj 2007-10-24
		将ONU程序在GW和CTC模式之间转换从ONU程序升级命令中独立出来
		*/
/*#ifndef  _EPON_TYPE_GDC_OEM_*/
	if(( node == ONU_CTC_NODE ) || ( node == ONU_NODE ) || (node == ONU_GT861_NODE))
		install_element ( node, &onu_file_convert_cmd );
/*#endif*/
	/*install_element ( node, &onu_show_device_version_cmd );
	install_element(node, &show_running_config_cmd);*/

	install_element ( node, &onu_cli_vconsole_client_cmd );

#if( EPON_MODULE_ONU_LOOP == EPON_MODULE_YES )
	OnuLoop_CommandInstallByType( node );
#endif


	install_element (node, &show_onu_serial_number_cmd);

    install_element ( node, &onu_profile_show_cmd);
    install_element ( node, &onu_profile_info_show_cmd);
    install_element (node, &onu_profile_association_show_cmd);
    install_element (node, &onu_profile_association_show_oneonu_cmd);
    install_element ( node, &onu_profile_info_show_byname_cmd);

	install_element( node, &onu_pon_loopback_config_cmd );
	install_element( node, &onu_pon_loopback_show_cmd );

	 return VOS_OK;

}
LONG ONUCommandInstall( enum node_type  node)
{
    install_element ( node, &onu_profile_show_cmd);
    install_element ( node, &onu_profile_info_show_cmd);
    install_element (node, &onu_profile_association_show_cmd);
    install_element (node, &onu_profile_association_show_oneonu_cmd);
    install_element ( node, &onu_profile_info_show_byname_cmd);
	install_element( node, &onu_device_name_config_cmd);
    install_element ( node, &onu_maxmac_config_cmd);
    install_element ( node, &undo_onu_maxmac_config_cmd);
    install_element ( node, &onu_maxmac_show_cmd);
    return VOS_OK;
}

LONG CTCONUCommandInstall()
{
	/*install_element ( ONU_CTC_NODE, &onu_deregister_cmd);*/
	install_element ( ONU_CTC_NODE, &onu_reset_cmd);
	install_element ( ONU_CTC_NODE, &onu_device_name_config_cmd);/*add by shixh20100208*/
	install_element ( ONU_CTC_NODE, &onu_device_description_config_cmd); /* add by liwei056@2011-1-30 */
	install_element ( ONU_CTC_NODE, &onu_customer_location_config_cmd);  /* add by liwei056@2011-1-30 */
	install_element ( ONU_CTC_NODE, &config_auto_delete_onu_cmd);

	/*ONU 激活、去激活操作*/
	install_element ( ONU_CTC_NODE, &onu_node_deactivate_onu_device_cmd );
	install_element ( ONU_CTC_NODE, &undo_onu_node_deactivate_onu_device_cmd );
	install_element ( ONU_CTC_NODE, &onu_cli_vconsole_client_cmd );

    /* B--added by liwei056@2012-8-10 for D15651 */
	install_element ( ONU_CTC_NODE, &onu_maxmac_show_cmd);
	install_element ( ONU_CTC_NODE, &onu_maxmac_config_cmd);
	install_element ( ONU_CTC_NODE, &undo_onu_maxmac_config_cmd);
    /* E--added by liwei056@2012-8-10 for D15651 */

	/* added by chenfj 2007-8-8 */
	install_element( ONU_CTC_NODE, &onu__fec_config_cmd );
	install_element( ONU_CTC_NODE, &show_onu__fec_config_cmd );
	/*install_element( ONU_CTC_NODE, &show_fec__ability_cmd );*/
	
	/* added by chenfj 2007-8-16
		CTC ONU节点下,增加ONU pon 芯片学习MAC地址显示命令*/
	install_element( ONU_CTC_NODE, &onu_show_fdb_mac_config_cmd);
	install_element( ONU_CTC_NODE, &onu_show_fdb_mac_counter_cmd);
#ifdef ONU_PEER_TO_PEER	
	install_element ( ONU_CTC_NODE, &peer_to_peer_rule_cmd);
	install_element ( ONU_CTC_NODE, &peer_to_peer_rule_show_cmd);
#endif
	/* added by chenfj 2007-8-17
		CTC ONU节点下,增加升级ONU程序命令*/
	install_element( ONU_CTC_NODE, &onu_update_file_cmd );
	install_element( ONU_CTC_NODE, &onu_auto_update_config_cmd);
	install_element( ONU_CTC_NODE, &onu_show_software_auto_update_cmd);

	install_element( ONU_CTC_NODE, &onu_profile_info_show_cmd);
	install_element (ONU_CTC_NODE, &onu_profile_association_show_cmd);
	install_element (ONU_CTC_NODE, &onu_profile_association_show_oneonu_cmd);
	install_element( ONU_CTC_NODE, &onu_profile_info_show_byname_cmd);
	install_element( ONU_CTC_NODE, &onu_profile_show_cmd);

	/*install_element(ONU_CTC_NODE, &show_running_config_cmd);*/
	install_element( ONU_CTC_NODE, &onu_pon_loopback_config_cmd );
	install_element( ONU_CTC_NODE, &onu_pon_loopback_show_cmd );

	return( VOS_OK );

}


LONG ONU_CommandInstall()
{
    install_element(DEBUG_HIDDEN_NODE, &release_onu_debug_cmd);         /*added by luh 2012-11-1,增加释放onu权限调试命令，防止不能进入onu节点*/
    install_element(DEBUG_HIDDEN_NODE, &release_profile_debug_cmd);     /*added by luh 2012-11-1,增加释放profile权限调试命令，防止不能进入profile节点*/
    install_element(DEBUG_HIDDEN_NODE, &onu_profile_info_debug_show_cmd);/*added by luh 2012-11-1,增加隐藏节点下，对临时配置文件的显示*/
    install_element( DEBUG_HIDDEN_NODE, &onu_conf_debug_cmd);
    install_element(DEBUG_HIDDEN_NODE, &undo_onu_conf_debug_cmd);
    install_element( DEBUG_HIDDEN_NODE, &onu_conf_debug_timer_cmd);
    install_element(DEBUG_HIDDEN_NODE, &onu_conf_debug_level_set_cmd);
    install_element(DEBUG_HIDDEN_NODE, &undo_onu_conf_debug_level_set_cmd);

	install_element ( CONFIG_NODE, &into_epon_onu_node_cmd);

	install_element ( CONFIG_NODE, &onu_profile_create_cmd);
	/*install_element ( CONFIG_NODE, &onu_profile_edit_cmd);*/
	install_element ( CONFIG_NODE, &onu_profile_delete_cmd);
	install_element ( CONFIG_NODE, &onu_profile_delete_disused_conf_cmd);
	install_element ( CONFIG_NODE, &onu_profile_copy_cmd);
	install_element ( CONFIG_NODE, &onu_profile_switch_one_pon_cmd);
	install_element ( CONFIG_NODE, &onu_config_copy_one_onu_cmd);
    
	install_element ( CONFIG_NODE, &onu_profile_undo_associate_cmd);
	install_element ( CONFIG_NODE, &onu_profile_undo_associate_by_name_cmd);
	install_element ( CONFIG_NODE, &onu_profile_show_cmd);
	install_element ( CONFIG_NODE, &onu_profile_info_show_cmd);
	install_element ( CONFIG_NODE, &onu_profile_association_show_cmd);
	install_element ( CONFIG_NODE, &onu_profile_association_show_oneonu_cmd);
	install_element ( CONFIG_NODE, &onu_profile_info_show_byname_cmd);
	install_element ( CONFIG_NODE, &onu_profile_starup_show_cmd);

    /*added by luh@2014-10-27*/
	install_element ( VIEW_NODE, &onu_profile_show_cmd);
	install_element ( VIEW_NODE, &onu_profile_info_show_cmd);
	install_element ( VIEW_NODE, &onu_profile_association_show_cmd);
	install_element ( VIEW_NODE, &onu_profile_association_show_oneonu_cmd);
	install_element ( VIEW_NODE, &onu_profile_info_show_byname_cmd);
	install_element ( VIEW_NODE, &onu_profile_starup_show_cmd);

    /*added by luh 2014-08-05*/
    if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
    {
        install_element(LIC_NODE, &onu_profile_info_debug_show_cmd);/*added by luh 2012-11-1,增加隐藏节点下，对临时配置文件的显示*/
        install_element(LIC_NODE, &onu_conf_debug_level_set_cmd);
        install_element(LIC_NODE, &undo_onu_conf_debug_level_set_cmd);


    	install_element ( LIC_NODE, &onu_profile_show_cmd);
    	install_element ( LIC_NODE, &onu_profile_info_show_cmd);
    	install_element ( LIC_NODE, &onu_profile_association_show_cmd);
    	install_element ( LIC_NODE, &onu_profile_association_show_oneonu_cmd);
    	install_element ( LIC_NODE, &onu_profile_info_show_byname_cmd);    

    	install_element ( DEBUG_HIDDEN_NODE, &debug_onu_config_cmd);
    	install_element ( DEBUG_HIDDEN_NODE, &nodebug_onu_config_cmd);
    	install_element ( DEBUG_HIDDEN_NODE, &show_debug_onu_config_cmd);
    	
    	
    }
    else
    {
    	install_element ( DEBUG_HIDDEN_NODE, &onu_profile_show_cmd);
    	install_element ( DEBUG_HIDDEN_NODE, &onu_profile_info_show_cmd);
    	install_element ( DEBUG_HIDDEN_NODE, &onu_profile_association_show_cmd);
    	install_element ( DEBUG_HIDDEN_NODE, &onu_profile_association_show_oneonu_cmd);
    	install_element ( DEBUG_HIDDEN_NODE, &onu_profile_info_show_byname_cmd);
        
    	install_element ( DEBUG_HIDDEN_NODE, &onu_profile_starup_show_cmd);
		
    	install_element ( DEBUG_HIDDEN_NODE, &show_onu_eth_mac_counter_cmd);
		
    }
    
	install_element(CONFIG_NODE, &onu_cli_relay_mode_cmd);
	install_element(CONFIG_NODE, &undo_onu_cli_relay_mode_cmd);
	install_element(CONFIG_NODE, &show_onu_cli_relay_mode_cmd);

    install_element (PON_PORT_NODE, &onu_profile_show_cmd);

	install_element ( PON_PORT_NODE, &pon_into_onu_node_cmd);

	install_element(CONFIG_NODE, &add_onu_model_black_list_config_cmd);
	install_element(CONFIG_NODE, &del_onu_model_black_list_config_cmd);
	install_element(CONFIG_NODE, &show_onu_model_black_list_config_cmd);
	Onustats_CommandInstall();
	
#if 0
	install_element ( ONU_NODE, &onu_customer_location_config_cmd);
	install_element ( ONU_NODE, &onu_device_name_config_cmd);
	install_element ( ONU_NODE, &onu_device_description_config_cmd);
	install_element ( ONU_NODE, &onu_deregister_cmd);
	install_element ( ONU_NODE, &onu_auto_update_config_cmd);
	install_element ( ONU_NODE, &onu_reset_cmd);
	install_element ( ONU_NODE, &onu_update_file_cmd);
	install_element ( ONU_NODE, &onu_show_software_auto_update_cmd);
	/*install_element ( ONU_NODE, &onu_show_device_information_cmd);*/
	install_element ( ONU_NODE, &onu_start_statistic_config_cmd);
	install_element ( ONU_NODE, &onu_no_statistic_config_cmd);
	install_element ( ONU_NODE, &onu_show_statistic_bucket_cmd);
		
	install_element ( ONU_NODE, &onu_clear_statistic_data_cmd);

	/*install_element ( ONU_NODE, &onu_alarm_config_cmd);*/
	/*install_element ( ONU_NODE, &onu_alarm_threshold_config_cmd);*/
	/*install_element ( ONU_NODE, &onu_show_alarm_info_cmd);*/
	/*install_element ( ONU_NODE, &onu_create_fdb_cmd);*/
	/*install_element ( ONU_NODE, &onu_show_mac_information_cmd); */  
	/*added by wutw at 13 september*/
	/*install_element ( ONU_NODE, &onu_show_fdb_llid_config_cmd);*/
	install_element ( ONU_NODE, &onu_encrypt_config_cmd);		
	install_element ( ONU_NODE, &onu_encrypt_keytime_config_cmd);			
	install_element ( ONU_NODE, &onu_no_encrypt_config_cmd);			
	install_element ( ONU_NODE, &onu_show_encrypt_config_cmd);

	install_element ( ONU_NODE, &onu_show_device_information_cmd);	

	install_element ( ONU_NODE, &onu_show_statistic_24h_data_cmd);		
	install_element ( ONU_NODE, &onu_show_statistic_15m_data_cmd);	
	install_element ( ONU_NODE, &onu_show_statistic_15mall_data_cmd);		
	install_element ( ONU_NODE, &onu_show_statistic_24hall_data_cmd);	
	/*install_element ( ONU_NODE, &onu_history_15m_time_interval_config_cmd);
	install_element ( ONU_NODE, &onu_history_24h_time_interval_config_cmd);*/

	/*added by wutw 2006/11/12*/
	install_element ( ONU_NODE, &onu_no_statistic_cycle_config_cmd);
	install_element ( ONU_NODE, &onu_start_statistic_cycle_config_cmd);

	/*added by wutw 2006/11/14*/
	install_element ( ONU_NODE, &onu_show_statistic_cmd);
	install_element ( ONU_NODE, &onu_show_fdb_mac_config_cmd);
	install_element( ONU_NODE, &onu_show_fdb_mac_counter_cmd);
	
	/*added by wutw 2006/11/28*/
  	install_element ( ONU_NODE, &onu_auto_update_disable_cmd);  

	install_element ( ONU_NODE, &peer_to_peer_rule_cmd);
	install_element ( ONU_NODE, &peer_to_peer_rule_show_cmd);

	/*added by wutw 2007/1/23*/
	install_element ( ONU_NODE, &onu_statistics_show_cmd);
	install_element ( ONU_NODE, &onu_olt_downlink_ber_show_cmd);
#endif
	install_element ( PON_PORT_NODE, &pon_into_onu_name_node_cmd);
	install_element ( CONFIG_NODE, &config_into_onu_name_node_cmd);
#if 0
	/* added by chenfj 2007-5-23 */
	install_element ( ONU_NODE, &onu_maxmac_show_cmd);
	install_element ( ONU_NODE, &onu_maxmac_config_cmd);
	install_element ( ONU_NODE, &undo_onu_maxmac_config_cmd);
	install_element( ONU_NODE, &onu__fec_config_cmd );
	install_element( ONU_NODE, &show_onu__fec_config_cmd );

	/* add by chenfj 2007-6-1 */
	install_element( ONU_NODE, &onu_SAmac_filter_config_cmd );
	install_element( ONU_NODE, &undo_onu_SAmac_filter_config_cmd );
	install_element( ONU_NODE, &show_onu_SAmac_filter_config_cmd );
#endif
	/* modified by chenfj 2007-9-10
		问题单#5273 */
	install_element (DEBUG_HIDDEN_NODE, &onu_deregister_cmd);
	install_element (CONFIG_NODE, &onu_profile_create_private_cmd);/*added by luh@2015-1-6*/
	install_element (DEBUG_HIDDEN_NODE, &onu_remote_reset_cmd);/*added by luh@2015-1-6*/
    if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
    	install_element ( LIC_NODE, &onu_remote_reset_cmd);
	install_element( ONU_CTC_NODE, &onu_show_device_information_cmd);	/*added by luh@2015-10-28*/

	/* for daya ONU gt811/gt812 */
	ONU_CommandInstallByType( ONU_NODE );
	/* for GT813 */
	ONU_CommandInstallByType( ONU_GT813_NODE );
	/* for GT831/821 */
	ONU_CommandInstallByType( ONU_GT821_GT831_NODE );
	/* fot GT865 */
	ONU_CommandInstallByType( ONU_GT861_NODE );
	/* fot GT831B */
	ONU_CommandInstallByType( ONU_GT831B_NODE );
	
	/* fot CTC ONU */
	if( V2R1_CTC_STACK == TRUE )
	CTCONUCommandInstall();

	/* 新增类型ONU */
	ONU_NewType_CommandInstall(NEW_ONU_TYPE_CLI_NODE);
	
    return VOS_OK;
}
 
extern LONG  ONUGT813_CommandInstall();
extern LONG  OnuGT831_821CommandInstall(enum node_type  node);
extern LONG  OnuGT831_821CommandInstallTogether(enum node_type  node);
extern LONG  OnuGT861CommandInstall();
extern LONG  OnuCTCCommandInstall();
extern LONG  OnuNewTypeCommandInstall();

extern LONG install_show_gptyLinkTable();
extern LONG (*cdsms_epv2r1_ONU_CliInit)( VOID );
LONG ONU_LDEF_CliInit()
{
		/* GT811/812/810/816 cli node */
		onu_node_install();/* ******  未修改*********/
		onu_module_init();/* ******  未修改*********/
		ONUDAYA_CommandInstall_Ldefun( ONU_NODE);


		/* gt813 / GT865 cli node */
		ONUGT813_CommandInstall();/* ******  未修改********/
		onu_Qos_CommandInstall_Ldefun( ONU_GT813_NODE);

		/* GT831/831_A / 831_CATV / 831_A_CATV  cli node */
		OnuGT831_821CommandInstall_Ldefun(ONU_GT821_GT831_NODE);
		ONUDAYA_CommandInstall_Ldefun( ONU_GT821_GT831_NODE);

		/* GT831B  cli node */
		OnuGT831_821CommandInstall_Ldefun(ONU_GT831B_NODE);
		ONUDAYA_CommandInstall_Ldefun( ONU_GT831B_NODE);

		
		/* gt861 cli node */
		OnuGT861CommandInstall();/* ******  未修改*********/
		ONUDAYA_CommandInstall_Ldefun( ONU_GT861_NODE);
			
		/* CTC onu cli node */
		OnuCTCCommandInstall();/* ******  未修改*********/

		/* 新增类型ONU 命令节点*/
		OnuNewTypeCommandInstall_Ldefun();
		
		/* 公共 command */
		ONU_CommandInstall();/* ******  未修改*********/

		install_show_gptyLinkTable();/***Just for test duzhk***/
return VOS_OK;
}

LONG onu_profile_node_install()
{

    install_node(&onu_profile_node, onu_profile_config_write);
    onu_profile_node.prompt = ( CHAR* ) VOS_Malloc(64, MODULE_RPU_ONU);
    if(!onu_profile_node.prompt)
    {
        ASSERT(0);
        return -IFM_E_NOMEM;
    }

    install_default(ONU_PROFILE_NODE);

    return VOS_OK;
}

LONG onu_profile_module_init()
{
    struct cl_cmd_module * onu_profile_module = NULL;

    onu_profile_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_ONU);
    if ( !onu_profile_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) onu_profile_module, sizeof( struct cl_cmd_module ) );

    onu_profile_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_ONU);
    if ( !onu_profile_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( onu_profile_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( onu_profile_module->module_name, "onu_profile" );

    onu_profile_module->init_func = onu_init_func;
    onu_profile_module->showrun_func = NULL;
    onu_profile_module->next = NULL;
    onu_profile_module->prev = NULL;

    cl_install_module( onu_profile_module );

    return VOS_OK;
}

LONG ONU_CliInit()
{  
        init_pty_connection_status();/*初始化ctc pty 缓存表，added by luh 2012-2-3*/
        /*added by wangxiaoyu 2011-09-15
         * 初始化透传命令行的ONU类型配置表，默认为全部涉及类型使用透传方式*/
        initOnuCliRelayMode();

		/* GT811/812/810/816 cli node */
		onu_node_install();
		onu_module_init();

		onu_bw_based_mac_module_init();	/* added by xieshl 20110909, 支持基于onu mac地址的带宽配置 */
		
		ONUDAYA_CommandInstall( ONU_NODE);

		onu_profile_node_install();
		onu_profile_module_init();
		ONUDAYA_CommandInstall( ONU_PROFILE_NODE );

		/* gt813 / GT865 cli node */
		ONUGT813_CommandInstall();
		onu_Qos_CommandInstall( ONU_GT813_NODE);
		
		/* GT831/831_A / 831_CATV / 831_A_CATV  cli node */

		if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
		    OnuGT831_821CommandInstall( ONU_GT821_GT831_NODE );
		else
		    OnuGT831_821CommandInstall_Ldefun( ONU_GT821_GT831_NODE );
		ONUDAYA_CommandInstall( ONU_GT821_GT831_NODE);

		/* GT831B  cli node */
		if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
		    OnuGT831_821CommandInstall( ONU_GT831B_NODE );
		else
		    OnuGT831_821CommandInstall_Ldefun( ONU_GT831B_NODE );

		ONUDAYA_CommandInstall( ONU_GT831B_NODE);
		
		/* gt861 cli node */
		OnuGT861CommandInstall();
		ONUDAYA_CommandInstall( ONU_GT861_NODE );
		
		
		/* CTC onu cli node */
		OnuCTCCommandInstall();
#if( EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES )
		OnuCMCCommandInstall();
#endif
        /* added by luh@2015-11-09, GPON onu cli node*/
        OnuGPONCommandInstall();

		/* 新增类型ONU 命令节点*/
		OnuNewTypeCommandInstall();
		ONUDAYA_CommandInstall(NEW_ONU_TYPE_CLI_NODE);
		
		ONUDAYA_CommandInstall(ONU_CTC_NODE);
		ONUDAYA_CommandInstall(ONU_CMC_NODE);

		/* 公共 command */
		ONU_CommandInstall();

	return VOS_OK;
}

/* added by xieshl 20111101, 需求9283, 在VIEW节点下增加ONU-VIEW节点 */
DEFUN  (
    into_epon_onu_view_node,
    into_epon_onu_view_node_cmd,
    "onu <slot/port/onuid>",
    "Select an onu to view\n"
    "Specify onu interface's onuid\n"
    )
{
	LONG	lRet;
	ULONG   ulSlot, ulPort, ulOnuid;
	CHAR 	ifName[IFM_NAME_SIZE + 1];
	ULONG   ulIFIndex = 0;
	CHAR    prompt[64] = { 0 };
	INT16 phyPonId = 0;

	int ponid;

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	
	if( PonCardSlotPortCheckByVty(ulSlot, ulPort, vty) != ROK )
		return(CMD_WARNING );
	if(SlotCardMayBePonBoardByVty(ulSlot, vty)  != ROK )
		return(CMD_WARNING);

	if ((ulOnuid<(CLI_EPON_ONUMIN+1)) || (ulOnuid>(CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onu %d/%d/%d error\r\n",ulSlot, ulPort, ulOnuid );
		return CMD_WARNING;	
	}


	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if( phyPonId == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	

	VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
	VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%d/%d/%d", "onu", ulSlot, ulPort, ulOnuid);

	ulIFIndex = IFM_PON_CREATE_INDEX( ulSlot, ulPort, ulOnuid, 0);
	if ( ulIFIndex == 0 )
	{
		vty_out( vty, "%% Can not find interface %s.\r\n", ifName );
		return CMD_WARNING;
	}

	vty->index = ( VOID * ) ulIFIndex;
	vty->prev_node = vty->node;
	vty->node = ONU_VIEW_NODE;

	VOS_StrCpy( prompt, "%s(" );
	VOS_StrCat( prompt, ifName );
	VOS_StrCat( prompt, ")>" );
	vty_set_prompt( vty, prompt );   

	return CMD_SUCCESS;
}


struct cmd_node onu_view_node =
{
    ONU_VIEW_NODE,
    NULL,
    1
};
LONG onu_view_node_install()
{
    install_node( &onu_view_node, onu_config_write);
    onu_view_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_ONU);
    if ( !onu_view_node.prompt )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }
    install_default( ONU_VIEW_NODE );
    return VOS_OK;
}

LONG ONU_View_CommandInstall()
{
	onu_view_node_install();
	
	install_element( VIEW_NODE, &into_epon_onu_view_node_cmd);

	install_element( ONU_VIEW_NODE, &onu_show_device_information_cmd);	
	install_element( ONU_VIEW_NODE, &onu_show_fdb_mac_config_cmd);
	install_element( ONU_VIEW_NODE, &onu_show_fdb_mac_counter_cmd);
	install_element( ONU_VIEW_NODE, &onu_statistics_show_cmd);
	install_element( ONU_VIEW_NODE, &onu_show_statistic_24h_data_cmd);		
	install_element( ONU_VIEW_NODE, &onu_show_statistic_15m_data_cmd);	
	install_element( ONU_VIEW_NODE, &onu_show_statistic_15mall_data_cmd);		
	install_element( ONU_VIEW_NODE, &onu_show_statistic_24hall_data_cmd);	
	install_element( ONU_VIEW_NODE, &onu_show_statistic_cmd);
	install_element( ONU_VIEW_NODE, &show_onu_serial_number_cmd );
	install_element( ONU_VIEW_NODE, &onu_device_name_config_cmd);/*added by luh @2015-1-15*/    

	onu_view_daya_cmd_install();

	return VOS_OK;
}

#ifdef	__cplusplus
}
#endif/* __cplusplus */
