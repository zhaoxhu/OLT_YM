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
*	  history :
*	  1. 现在的ONU fe 数量为4，但以后根据onu 型号进行修改。
*
*	modified
*	2006-10-20 修改atu static_add 与 atu static_del 命令的解析。
*	    	   将平台输入参数格式为:<H.H.H>,daya 的格式为 **:**:**:**:**:**
*	2006-10-30 增加uptime命令，将该命令透传给onu。
*	2006-11-06 增加port mode_show,qos user_pri_en，qos ip_pri_en和qos rule,与
*				mgt uptime,并修改port ingress_rate与port egress_rate的参数为
*				62-100000kbits/s
*	2006-11-08 将qos rule <1-24> {[tag|ip]}*1改为qos rule <1-24> {[user|ip]}*1
*	2006-11-10 增加代码,以支持onu 以太网端口列表的输入,比如:atu static_add <H.H.H> 
*				<port_list>, port_list的输入可为:1,2-3,4或者2-4,1.
*				atu static_add <H.H.H> <port_list>与igmpsnooping gda_add 
*				<xx:xx:xx:xx:xx:xx> <port_list>
*				增加代码,在执行mgt config save时,同时调用隋平礼的接口,隐式执行show run与
*				save动作;在实行mgt config clear是,隐式执行
*				onu相关配置,设置默认值,并调用隋的接口进行保存
*   2006/11/20 : 将mgt run－time改为mgt up-time
*   2006/11/20 : 修改mgt config clear命令的函数调用	
*	2006/11/23 : pclint 检查，修改部分代码
*	2006/11/26 : 删除mgt config clear一些函数的错误参数
*	2006/11/28 : 增加event show命令，增加mirror三条命令
*	2006/12/22 : 修改关于QOS的帮助说明
*	2006/12/22 : 增加p2p转发规则,并在mgt config save中增加相关代码
*	2006/12/26 : 修改port ingress_rate/egress_rate 的参数，增加了rate＝0为not limit
*	2006/12/27 : 修改port mirror_to {[i|e] [<1-24>|0]}*1命令输入0时不能被执行的问题
* 	modified by chenfj 2007-9-25 
*	       GT810/GT816支持的端口限速最小值为100，故特别加以判断
*   added by ZhengYT 2007-7-5 
*         ONU GT831/GT821 命令行, 用于VOIP & CATV
*   modified by chenfj 2007-8-30
*      使能或关闭UNI接口的MAC地址学习。
*      按照大亚方式统一一下命令。 张新辉需按照新的定义修改GT816的软件
*   added by chenfj 2007-9-17
*	  在GT811/GT812下增加如下两个命令: atu flood , atu limit
*  added by chenfj 2007-9-17
*	  DHCP relay功能，dscp_relay_agent : Enable/Disable DHCP Relay Agent ,
*	  涉及到的ONU：811/812/821/831/810/816
*  added by chenfj 2007-10-23
*      在GT810/816/811a/812a节点下增加Download命令,支持FTP方式下载程序
*  added by chenfj 2007-10-30
*      在GT811/GT812节点下，增加命令: 
*          1  cable test port <port_list> 
*          2  set ip <A.B.C.D/M>
*          3  show running-config 
*          4  show startup-config
*   modified by chenfj 2007-10-30
*     将原来ONU节点下（GT811类型，及GT831类型）命令中的FE端口都改成<port_list>形式,这样可在同一条命令中输入多个FE端口
*    【注：当前只有GT811_A/GT812_A支持此参数形式；其他不支持此参数形式的ONU，输入命令时，FE端口仍按以前单个端口方式输入】
*     关联到的命令有:
*     1  igmpsnooping fast_leave [<port_list>|all] {[0|1]}*1
*     2  vlan pvid <port_list> {<1-4094>}*1
*     3  vlan acceptable_frame_types <port_list> {[tagged|all]}*1
*     4  vlan ingress_filtering <port_list> {[0|1]}*1
*     5  vlan dot1q_port_add <2-4094> <port_list> [1|2]
*     6  vlan dot1q_port_del <2-4094> <port_list>
*     7  port en <port_list> {[0|1]}*1
*     8  port desc <port_list> {<string>}*1
*     9  port mode <port_list> {[0|8|9|10|11|12]}*1
*    10 port mode_show <port_list>
*    11 port fc <port_list> {[0|1]}*1
*    12 port ingress_rate <port_list> [0|1|2|3] [<0>|<62-100000>] [12k|24k|48k|96k]
*    13 port ingress_rate <port_list> {[0|1|2|3] [<0>|<62-100000>]}*1
*    14 port egress_rate <port_list> {[0|<62-100000>]}*1
*    15 port mirror_from {[i|e] <port_list> [0|1]}*1
*    16 port mirror_to {[i|e] [<port_list>|0]}*1
*    17 qos def_pri <port_list> {<0-7>}*1
*    18 qos user_pri_reg <port_list> <0-7> {<0-7>}*1
*    19 qos user_pri_reg <port_list>
*    20 qos rule <port_list> {[user|ip]}*1
*    21 qos ip_pri_en <port_list> {[0|1]}*1
*    22 qos user_pri_en <port_list> {[0|1]}*1
*    23 eth-loop port_en <port_list> {[0|1]}*1
*    24 stat port_flush {<port_list>}*1
*    25 stat port_show <port_list>
*    26 stat port_show <port_list>
*    27 atu learning <port_list> {[1|0]}*1
*    28 atu flood <port_list> {[1|0]}*1
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
#include "mn_oam.h"

/*added by wutw 2006/11/10*/
#include "Olt_cli.h"
#include "V2R1General.h"
#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "V2R1_product.h"
#include "onu/onuConfMgt.h"

extern LONG    lCli_SendbyOam( INT16 ponID, INT16 onuID,	UCHAR  *pClibuf,	USHORT length,cliPayload *stSessionIdPayload ,	struct vty * vty);
extern LONG PON_ParseSlotPortOnu( CHAR * szName, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern short int GetPonPortIdxBySlot( short int slot, short int port );
extern  int cl_save_runconfig_to_startupconfig(int save_way,int(*lock_func)(void *,int),int (*unlock_func)(void *,int),
														void *arg,int fd,char *savefrom_username, char *savefrom_address ,
														int lan_type, int timeout);
 
extern LONG PON_GetSlotPortOnu( ULONG ulIfIndex, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);

extern int  GetOnuOperStatus( short int PonPortIdx, short int OnuIdx );
extern int  GetOnuSWUpdateCtrlDefault();
extern int  SetOnuSWUpdateCtrl(short int PonPortIdx, short int OnuIdx, unsigned char EnableFlag);
extern int  GetOnuType( short int PonPortIdx, short int OnuIdx, int *type );
extern int  SetOnuEncryptKeyExchagetime(short int PonPortIdx, short int OnuIdx, unsigned int time);
extern int  GetPonPortEncryptKeyTimeDefault( unsigned int *TimeLen );
extern int  GetPonPortEncryptDefault ( unsigned int *cryptionDirection);
extern int  OnuEncryptionOperation(short int PonPortIdx, short int OnuIdx, unsigned int cryptionDirection);
extern STATUS HisStats15MinMaxRecordGet(unsigned int *pValue);
extern STATUS CliHisStatsONUStatusGet(short int ponId,short int onuId,unsigned int *pStatus15m,unsigned int *pStatus24h);
extern STATUS HisStats24HoursMaxRecordGet(unsigned int *pValue);
extern STATUS HisStatsOnu15MModified(short int ponId, short int onuId, unsigned int bucketNum, BOOL flag15M);
extern STATUS HisStatsOnu24HModified(short int ponId, short int onuId, unsigned int bucketNum, BOOL flag24H);
extern STATUS getDeviceType( const ulong_t devIdx, ulong_t* pValBuf );
extern int  SetOnuPeerToPeerForward( short int PonPortIdx, short int OnuIdx , int address_not_found, int broadcast );
extern long taskDelay(long);
extern int cliCheckOnuMacValid_onuNode( short int PonPortIdx, short int OnuIdx );
extern int CT_RMan_CLI_Init ();
extern LONG  CTC_AlarmCmd_Init();
extern LONG  CTC_VoipCmd_Init(enum node_type  node);

int CliOnuTypeVty(ULONG slotId , ULONG port, ULONG onuId,ULONG onuFePort,struct vty *vty);
static int onu_dev_cli_process( struct vty *vty, char *hint_str, int *onu_mask );
static int onu_gt831_cli_process( struct vty *vty, char *hint_str );

static int rpc_onu_dev_cli_process( struct vty *vty, char *hint_str, int *onu_mask );

int isCtcOnu(ULONG slotid, ULONG port, ULONG onuid)
{
    int ret = FALSE, type;
#if 1
    int ponportidx = GetPonPortIdxBySlot(slotid, port);

    if(GetOnuType(ponportidx, onuid-1, &type) == VOS_OK && (type == V2R1_ONU_GPON || type == V2R1_ONU_CTC || type == V2R1_ONU_CMC || type == V2R1_OTHER))
        ret = TRUE;
#else
    ret = TRUE;
    type = 0;
#endif
    return ret;
}

int isCmcOnu(ULONG slotid, ULONG port, ULONG onuid)
{
    int ret = FALSE, type;
#if 1
    int ponportidx = GetPonPortIdxBySlot(slotid, port);

    if(GetOnuType(ponportidx, onuid-1, &type) == VOS_OK && (type == V2R1_ONU_CMC))
        ret = TRUE;
#else
    ret = TRUE;
    type = 0;
#endif
    return ret;
}

int isGponOnu(ULONG slotid, ULONG port, ULONG onuid)
{
    int ret = FALSE, type;
    int ponportidx = GetPonPortIdxBySlot(slotid, port);

    if(GetOnuType(ponportidx, onuid-1, &type) == VOS_OK && (type == V2R1_ONU_GPON))
        ret = TRUE;

    return ret;
    
}
static int onuConfFileWriteable(ULONG slotid, ULONG port, ULONG onuid, struct vty *vty)
{
    int ret = VOS_OK;

    UCHAR rflag = 0;

    int ponportidx = GetPonPortIdxBySlot(slotid, port);

#ifdef ONUID_MAP
    ONU_CONF_SEM_TAKE
    {
        rflag = getOnuConfRestoreFlag(ponportidx, onuid-1);
        ONU_CONF_SEM_GIVE
    }
#else
    ONU_MGMT_SEM_TAKE
    rflag = OnuMgmtTable[ponportidx*MAXONUPERPON+onuid-1].configRestoreFlag;
    ONU_MGMT_SEM_GIVE
#endif

    /*恢复数据模式*/
    if(rflag)
    {
        if(vty->fd != 0) /*恢复数据 时用户输入的指令被放弃*/
            ret = VOS_ERROR;
    }
    else
    {
        if(vty->node != ONU_PROFILE_NODE && isCtcOnu(slotid, port, onuid) && onuConfIsShared(ponportidx, onuid-1) )
            ret = VOS_ERROR;
    }

    return ret;
}
int Onu_profile_is_share(struct vty *vty, short int ponportid,short int onuid)
{
		ONUConfigData_t *p ;
		p = vty->onuconfptr;
		ONU_CONF_SEM_TAKE
		if(getOnuConfRestoreFlag(ponportid,onuid) == 0 && p != NULL)
		{
			if(p->share && VOS_StrCmp(p->confname , DEFAULT_ONU_CONF) != 0)
			{
				ONU_CONF_SEM_GIVE
	            return VOS_OK;
			}
		}
		ONU_CONF_SEM_GIVE
		return VOS_ERROR;
}
#define ONU_CONF_WRITEABLE_CHECK \
    if(onuConfFileWriteable(ulSlot, ulPort, ulOnuid, vty) != VOS_OK) \
    { \
        vty_out(vty, ONUCONF_WARNING_STR); \
        return CMD_WARNING; \
    }

#if 0
#define VTY_ONU_FE_PORT_IN_RANGE(slot, pon, onu, fe_argv, port) \
    BEGIN_PARSE_PORT_LIST_TO_PORT( fe_argv, port ) \
    { \
        if (VOS_OK != CliOnuTypeVty(slot , pon, onu, port, vty)) \
        { \
            vty_out(vty, "too big port number in the input list!\r\n", port); \
            return CMD_WARNING; \
        } \
    } \
    END_PARSE_PORT_LIST_TO_PORT();
#else
	#define VTY_ONU_FE_PORT_IN_RANGE(slot, pon, onu, fe_argv, port) \
	{\
		short int olt_id = GetPonPortIdxBySlot(slot, pon);\
		short int onu_id = onu-1;\
		ULONG devidx = 0;\
		ULONG fenum = 0;\
		if(ONU_OPER_STATUS_DOWN == GetOnuOperStatus(olt_id, onu_id))\
		{\
			vty_out(vty, "onu is off-line!\r\n");\
			return CMD_WARNING;\
		}\
		devidx = MAKEDEVID(slot, pon, onu); \
		if(getDeviceCapEthPortNum(devidx, &fenum) != VOS_OK) \
		{\
			vty_out(vty, "unknown onu port num!\r\n");\
			return CMD_WARNING;\
		}\
		BEGIN_PARSE_PORT_LIST_TO_PORT(fe_argv, port)\
		if(port > fenum)\
		{\
			vty_out(vty, "too big port number in the input list! please input port in the range (1-%d)\r\n\r\n", fenum); \
			RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING);	/* modified by xieshl 20120906, 解决内存丢失问题，中间不能直接用return、continue语句，下同 */ \
		}\
		END_PARSE_PORT_LIST_TO_PORT()\
	}
#endif

#define VTY_ONU_PROFILE_FE_PORT_IN_RANGE(fe_argv, port) \
    BEGIN_PARSE_PORT_LIST_TO_PORT( fe_argv, port ) \
    { \
        if (port > ONU_MAX_PORT) \
        { \
            vty_out(vty,"too big port number in the input list! please input port number from (1~%d) \r\n", ONU_MAX_PORT); \
            RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING ); \
        } \
    } \
    END_PARSE_PORT_LIST_TO_PORT();

#define	CLI_EPON_ONUUP						1
#define	CLI_EPON_ONUDOWN					2
#define     PENGMAXDEVICENAMELEN                    256
/*
#ifndef MAXPON 
#define  MAXPON 20
#endif

#ifndef MAXONUPERPON
#define  MAXONUPERPON  64 
#endif
*/
#ifndef MAX_ETH_PORT_NUM
#define MAX_ETH_PORT_NUM 24
#endif
/*
#ifndef CHECK_PON_RANGE

#define CHECK_PON_RANGE \
	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON )){\
		sys_console_printf("\r\nerr:PonId=%d out of range\r\n", (PonPortIdx+1));\
		sys_console_printf("file:%s, line:%d\r\n",__FILE__, __LINE__ );\
		ASSERT(0);\
		return ( RERROR );\
		}
#endif

#ifndef CHECK_ONU_RANGE
#define CHECK_ONU_RANGE \
	{ \
	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON )){\
		sys_console_printf("\r\nerr:PonId=%d out of range\r\n", (PonPortIdx+1));\
		sys_console_printf("file:%s, line:%d\r\n",__FILE__, __LINE__ );\
		ASSERT(0);\
		return ( RERROR );\
		}\
	if(( OnuIdx < 0 ) || ( OnuIdx >= MAXONUPERPON )) {\
		sys_console_printf("\r\nerr:%s/port%d ",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );\
		sys_console_printf("onuId=%d out of range\r\n", (OnuIdx+1)  );\
		sys_console_printf("file:%s, line:%d\r\n",__FILE__, __LINE__ );\
		ASSERT(0);\
		return ( RERROR );\
		}\
	}

#endif
*/
#ifndef  ONU_TYPE_AND_COMMAND_CHECK
#define  ONU_TYPE_AND_COMMAND_CHECK \
	if(( OnuType != V2R1_ONU_GT831 ) && ( OnuType != V2R1_ONU_GT831_CATV ) && ( OnuType != V2R1_ONU_GT831_A) && ( OnuType != V2R1_ONU_GT831_A_CATV) && \
			( OnuType != V2R1_ONU_GT831_B ) && (OnuType != V2R1_ONU_GT831_B_CATV) )\
		{\
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );\
		return( CMD_WARNING );\
		}
#endif

/* modified by chenfj 2007-11-8
     问题单#5792.有catv命令，但是不能设置
	mgt catv 命令增加GT831_A_CATV 
*/
#ifndef  ONU_TYPE_AND_COMMAND_CHECK_GT831
#define  ONU_TYPE_AND_COMMAND_CHECK_GT831 \
	if(( OnuType != V2R1_ONU_GT831_CATV ) && ( OnuType != V2R1_ONU_GT831_A_CATV ) && ( OnuType != V2R1_ONU_GT831_B_CATV ))\
		{\
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );\
		return( CMD_WARNING );\
		}
#endif

#ifndef  ONU_TYPE_AND_COMMAND_CHECK_GT831_821_816_810
#define  ONU_TYPE_AND_COMMAND_CHECK_GT831_821_816_810 \
	if(( OnuType != V2R1_ONU_GT831 ) && ( OnuType != V2R1_ONU_GT831_CATV ) && ( OnuType != V2R1_ONU_GT831_A) && ( OnuType != V2R1_ONU_GT831_A_CATV) && ( OnuType != V2R1_ONU_GT816 ) && ( OnuType != V2R1_ONU_GT810 ) )\
		{\
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );\
		return( CMD_WARNING );\
		}
#endif

#ifndef  ONU_TYPE_AND_COMMAND_CHECK_GT816_810
#define  ONU_TYPE_AND_COMMAND_CHECK_GT816_810 \
	if(( OnuType != V2R1_ONU_GT816 ) && ( OnuType != V2R1_ONU_GT810 ) )\
		{\
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );\
		return( CMD_WARNING );\
		}
#endif


#ifndef MGCP_STR
#define MGCP_STR  "config mgcp\n"
#endif

#ifndef SIP_STR
#define SIP_STR "config sip\n"
#endif

#ifndef H323_STR 
#define H323_STR "config H323\n"
#endif

#ifndef H248_STR
#define H248_STR "config H248\n"
#endif

#ifndef DescStringPortList
#define DescStringPortList  "input the port list, eg 1,3,5-8,etc.\n"
#endif

#ifndef ONUCONF_WARNING_STR
#define ONUCONF_WARNING_STR "onu's config file is shared, please private it first!\r\n"
#endif

#ifndef ONU_CONF_OAM_ERR_STR
#define ONU_CONF_OAM_ERR_STR "OAM request error, maybe onu is off-line!\r\n"
#endif

#define __CTC_TEST_NMS_COMMAND
#undef __CTC_TEST_NMS_COMMAND

#define __RPC_ONU_COMMAND

/* 
	以下定义用于解析GT861 上ETH 端口号
	格式为slot/port, 或者: slot/port1-port2 及两者的综合, 之间用',' 隔开
	暂定义GT861 支持:
	  每子板最多16个ETH , 共有5 个子板;最多64 个ETH 口
*/
#define  MAX_EHT_PORT_NUM_FOR_861   64
#define  MAX_ETH_PORT_NUM_PER_CARD_FOR_861  16
#ifndef  STATE_ULS
#define  STATE_ULS 1
#define  STATE_ULPS  2
#define  STATE_ULPE   3
#endif
ULONG *ONU_Parse_Slot_port_List(CHAR* slotport_list)
{
	 ULONG ulState = STATE_ULS;
	 ULONG ulslot=0;
	CHAR digit_temp[ MAX_ETH_PORT_NUM_PER_CARD_FOR_861+2];
	ULONG ulInterfaceList[ MAX_EHT_PORT_NUM_FOR_861+2 ];
	ULONG ulPortS = 0;
	ULONG ulPortE = 0;
	CHAR cToken;
	ULONG iflist_i = 0;
	ULONG list_i = 0;
	ULONG temp_i = 0;
	ULONG ulListLen = 0;
	CHAR * list;


	VOS_MemZero( ulInterfaceList, MAX_EHT_PORT_NUM_FOR_861 * 4 );
	ulListLen = VOS_StrLen( slotport_list );
	list = VOS_Malloc( ulListLen + 2, MODULE_RPU_CLI );
	if ( list == NULL )
	{
	    return NULL;
	}
	VOS_StrnCpy( list, slotport_list, ulListLen + 1 );
	list[ ulListLen ] = ',';
	list[ ulListLen + 1 ] = '\0';

	cToken = list[ list_i ];
	
	while ( cToken != 0 )
	{
		switch ( ulState )
		{
			case  STATE_ULS :
				if ( vos_isdigit( cToken ) )
		                {
		                    digit_temp[ temp_i ] = cToken;
		                    temp_i++;
		                    if ( temp_i >= MAX_ETH_PORT_NUM_PER_CARD_FOR_861 )
		                    {
		                        goto error;
		                    }
		                }
	                else if ( vos_isspace( cToken ) )
	                {}
	                else if ( cToken == '/' )
	                {
/*sys_console_printf("start parse /,temp_i=%d\r\n",temp_i);*/
	                    if ( temp_i == 0 )
	                    {
	                        goto error;
	                    }
	                    digit_temp[ temp_i ] = 0;
	                    ulslot = ( ULONG ) VOS_AtoL( digit_temp );
/*sys_console_printf("slot=%d\r\n",ulslot);*/
				if((ulslot>5)||(ulslot<1))
	                    {
	                        goto error;
	                    }
				ulInterfaceList[ 0 ] = ulslot;
	                     iflist_i++;
	                    temp_i = 0;
	                    ulState = STATE_ULPS;
/*sys_console_printf("end parse /\r\n");*/
	                }
	                else if ( cToken == ',' )
	                {
	                    if ( temp_i == 0 )
	                    {
	                        goto error;
	                    }
	                    digit_temp[ temp_i ] = 0;
	                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );
	                    if( (ulPortS>MAX_ETH_PORT_NUM_PER_CARD_FOR_861)||(ulPortS<1) )
	                    {
	                        goto error;
	                    }
	                    if ( 0 != ulPortS )
	                    {if(iflist_i!=0)
	                       	{ 
	                       		ulInterfaceList[ iflist_i ] = ulPortS;
	                       		 iflist_i++;
					}
				  else
				  	{
						iflist_i++;
						ulInterfaceList[ iflist_i ] = ulPortS;
						iflist_i++;
				  	}
	                    }
	                    if ( iflist_i >= MAX_EHT_PORT_NUM_FOR_861)
	                    {
	                        goto error;
	                    }
	                    temp_i = 0;
	                    ulState = STATE_ULS;
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
	                    ulState = STATE_ULPE;
	                }
	                else
	                {
	                    goto  error;
	                }
	                break;
		case  STATE_ULPS:
			if ( vos_isdigit( cToken ) )
	                {
/*sys_console_printf("after parse /,digital start\r\n"); */
	                    digit_temp[ temp_i ] = cToken;
	                    temp_i++;
	                    if ( temp_i >= MAX_ETH_PORT_NUM_PER_CARD_FOR_861 )
	                    {
	                        goto error;
	                    }
	                }
	                else if ( vos_isspace( cToken ) )
	                {}
	                else if ( cToken == ',' )
	                {
/*sys_console_printf("start parse ,\r\n"); */
	                    if ( temp_i == 0 )
	                    {
	                        goto error;
	                    }
	                    digit_temp[ temp_i ] = 0;
	                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );
				if((ulPortS>MAX_ETH_PORT_NUM_PER_CARD_FOR_861)||(ulPortS<1))
	                    {
	                        goto error;
	                    }
				
	                    if ( 0 != ulPortS )
	                    {if(iflist_i!=0)
	                       	{ 
	                       		ulInterfaceList[ iflist_i ] = ulPortS;
	                       		 iflist_i++;
					}
				  else
				  	{
						iflist_i++;
						ulInterfaceList[ iflist_i ] = ulPortS;
						iflist_i++;
				  	}
	                    }
	                    if ( iflist_i >= MAX_EHT_PORT_NUM_FOR_861 )
	                    {
	                        goto error;
	                    }
	                    temp_i = 0;
	                    ulState = STATE_ULS;
	                }
	                else if ( cToken == '-' )
	                {
/*sys_console_printf("start parse -,temp_i=%d\r\n",temp_i); */
	                    if ( temp_i == 0 )
	                    {
	                        goto error;
	                    }
	                    digit_temp[ temp_i ] = 0;
	                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );
/*sys_console_printf("ports=%d\r\n",ulPortS);	*/
	                    temp_i = 0;
	                    ulState = STATE_ULPE;
	                }
	                else
	                {
	                    goto error;
	                }
	                break;
			case  STATE_ULPE :
				 if ( vos_isdigit( cToken ) )
		                {
/*sys_console_printf("after parse -,digital start\r\n");  */
		                    digit_temp[ temp_i ] = cToken;
		                    temp_i++;
		                    if ( temp_i >= MAX_ETH_PORT_NUM_PER_CARD_FOR_861 )
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
/*sys_console_printf("start , temp_i=%d\r\n",temp_i);*/
	                    if ( temp_i == 0 )
	                    {
	                        goto error;
	                    }
	                    digit_temp[ temp_i ] = 0;
	                    ulPortE = ( ULONG ) VOS_AtoL( digit_temp );
/*sys_console_printf("ulportE=%d ,ulportS=%d\r\n",ulPortE,ulPortS);*/
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
/*sys_console_printf("for circle i=%d,iflist_i=%d \r\n",i,iflist_i); */
	                    		if((i> MAX_ETH_PORT_NUM_PER_CARD_FOR_861)||(i<1))
		                        {
		                            goto error;
		                        }

	                        if ( 0 != i )
	                        {if(iflist_i!=0)
	                       	{ 
	                       		ulInterfaceList[ iflist_i ] = i;
	                       		 iflist_i++;
					}
				  else
				  	{
						iflist_i++;
						ulInterfaceList[ iflist_i ] = i;
						iflist_i++;
				  	}
	                    }
	                        if ( iflist_i >= MAX_EHT_PORT_NUM_FOR_861 )
	                        {
	                            goto error;
	                        }
	                    }
	                    temp_i = 0;
	                    ulState = STATE_ULS;
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
/*sys_console_printf("last:iflist_i=%d\r\n",iflist_i);*/
 	VOS_Free( list );
	    if ( iflist_i == 0 )
	    {
	        return NULL;
	    }
	    else
	    {
	        list = VOS_Malloc( ( iflist_i + 1 ) * 4, MODULE_RPU_IFM );
/*sys_console_printf("end:g_malloc list,list=%X\r\n",list);	*/
	        if ( list == NULL )
	        {
	            return NULL;
	        }
	        VOS_MemZero( list, ( iflist_i + 1 ) * 4 );
/*for(temp_i=0;temp_i<iflist_i;temp_i++)
{
	sys_console_printf("last::%d\t",ulInterfaceList[temp_i]);
}
sys_console_printf("\r\n");*/
	        VOS_MemCpy( list, ulInterfaceList, iflist_i * 4 );
	        return ( ULONG * ) list;
	    }
	error:
	    VOS_Free( list );
	    return NULL;


}

#define BEGIN_PARSE_SLOTPORT_LIST_TO_PORT(slotport_list,slot, port) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = ONU_Parse_Slot_port_List(slotport_list );\
    if(_pulIfArray != NULL)\
    {\
    	slot=_pulIfArray[0];\
        for(_i=1;_pulIfArray[_i]!=0;_i++)\
        {\
            port = _pulIfArray[_i];
           /* sys_console_printf ("port=%d\r\n",port);*/

#define END_PARSE_SLOTPORT_LIST_TO_PORT() \
        }\
        VOS_Free(_pulIfArray);\
    }\
}


enum match_type onu_Check_SlotPort_List( char * slotport_list )
{
    int len = VOS_StrLen( slotport_list );
    ULONG interface_list[ MAX_EHT_PORT_NUM_FOR_861];
    int j, if_num = 0;
    int ret = 0;
    ULONG ulPort=0,ulSlot=0;

    char *plistbak = NULL;

    if ( ( !slotport_list ) || ( len < 1 ) )
    {
        return incomplete_match;
    }

    VOS_MemZero( ( char * ) interface_list, MAX_EHT_PORT_NUM_FOR_861 * sizeof( ULONG ) );
    plistbak = ( char * ) VOS_Malloc( len + 1, MODULE_RPU_CLI );
	if ( plistbak == NULL )
	{
	    return no_match;
	}

	VOS_StrCpy( plistbak, slotport_list );

    BEGIN_PARSE_SLOTPORT_LIST_TO_PORT( plistbak, ulSlot,ulPort )
    {
    	if((ulSlot==0)||(ulSlot>5)||(ulSlot<1))
    	{
		VOS_Free(_pulIfArray);
		return no_match;
    	}
        for ( j = 0; j <= if_num; j++ )
        {
            if ( interface_list[ j ] == ulPort )
            {
                VOS_Free( plistbak );
                plistbak = NULL;

                VOS_Free(_pulIfArray);
                return no_match;  /* 写重复的端口认为是错误的语法  */
            }
        }
        interface_list[ if_num ] = ulPort;
        if_num ++;
        if ( if_num > MAX_EHT_PORT_NUM_FOR_861 )
        {
            VOS_Free( plistbak );
            plistbak = NULL;
            
            VOS_Free(_pulIfArray); 

            return no_match;
        }
        ret = 1;
    }
    END_PARSE_SLOTPORT_LIST_TO_PORT();

    VOS_Free( plistbak );

    if ( ret == 0 )
        return incomplete_match;
    else
        return exact_match;
}


CMD_NOTIFY_REFISTER_S stCMD_onu_slotport_List_Check =
{
    "<slotport_list>",
    onu_Check_SlotPort_List,
    0
};

static int onu_dev_cli_process( struct vty *vty, char *hint_str, int *onu_mask );
static int onu_cli_process( struct vty *vty, char *hint_str )
{
	return onu_dev_cli_process(vty, hint_str, 0);
}
/* added by xieshl 20100521 */
/* TRUE - deny, FALSE - permit */
BOOL check_onu_cli_is_deny( struct vty *vty )
{
	return( SYS_LOCAL_MODULE_ISMASTERSTANDBY );
}


/* added by xieshl 20090226, 根据GT815的流控问题新增2个命令 */
DEFUN(onu_cfg_uni_pause,
	 onu_cfg_uni_pause_cmd,
	 "cfg uni pause {[enable|disable]}*1 ", 
	 "UNI config\n" 
	 "Config UNI parameters\n"
	 "Config UNI pause parameters\n")
{
#ifdef __RPC_ONU_COMMAND
    return rpc_onu_dev_cli_process( vty, "config UNI pause control", NULL );
#else
	return onu_cli_process( vty, "config UNI pause control" );
#endif
}
DEFUN(onu_poe_mode,
	 onu_poe_mode_cmd,
	 "poe {[enable|disable]}*1",
	 "Power over ethernet config\n"
	 "Enable power detect per port\n"
	 "Disable power detect per port\n")
{
#ifdef __RPC_ONU_COMMAND
    return rpc_onu_dev_cli_process( vty, "config power over ETH", NULL );
#else
	return onu_cli_process( vty, "config power over ETH" );
#endif
}

LDEFUN(onu_cfg_uni_pause,
	 onu_cfg_uni_pause_cmd,
	 "cfg uni pause {[enable|disable]}*1 ", 
	 "UNI config\n" 
	 "Config UNI parameters\n"
	 "Config UNI pause parameters\n",
	 ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(onu_poe_mode,
	 onu_poe_mode_cmd,
	 "poe {[enable|disable]}*1",
	 "Power over ethernet config\n"
	 "Enable power detect per port\n"
	 "Disable power detect per port\n",
	 ONU_NODE)
{
	return CMD_SUCCESS;
}
#ifdef    _GT861_SLOTPORT_FORMAT_CLI_
#endif
/**************** GT861 命令行***************/

DEFUN(iad_vlan_set,
	iad_vlan_set_cmd,
	"voip vlan {<2-5> <1-4093>}*1",
	"voip config\n"
	"vlan config\n"
	"slot number\n"
	"vlan ID\n" )
{
	return onu_cli_process( vty, "voice vlan" );
}
DEFUN(onu_slot_mirror_from_to_set,
	onu_slot_mirror_from_to_cmd,
	"slot mirror {<2-5> to <slotport_list> [ingress|egress|both] [enable|disable]}*1",
	"Slot config\n"
	"mirror config or show\n"
	"Slot ID\n"
	"to destination port\n"
       "only support FE port 1/1 and 1/2\n"
	"Ingress\n"
	"Egress\n"
	"Both direction\n"
	"Enable mirror\n"
	"Disabel mirror\n")
{
	return onu_cli_process( vty, "slot mirror" );
}

DEFUN(onu_atu_flood_for_861,
         onu_atu_flood_cmd_for_861,
         "atu flood <slotport_list> {[1|0]}*1",
         "Mac table config\n"
         "Mac table config\n"
         "input the onu fe slot/port list\n"
         "1-enable flood mode\n"
         "0-disable flood mode\n")
{
	return onu_cli_process( vty, "atu flood" );
}

LDEFUN(iad_vlan_set,
	iad_vlan_set_cmd,
	"voip vlan {<2-5> <1-4093>}*1",
	"voip config\n"
	"vlan config\n"
	"slot number\n"
	"vlan ID\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}
LDEFUN(onu_slot_mirror_from_to_set,
	onu_slot_mirror_from_to_cmd,
	"slot mirror {<2-5> to <slotport_list> [ingress|egress|both] [enable|disable]}*1",
	"Slot config\n"
	"mirror config or show\n"
	"Slot ID\n"
	"to destination port\n"
       "only support FE port 1/1 and 1/2\n"
	"Ingress\n"
	"Egress\n"
	"Both direction\n"
	"Enable mirror\n"
	"Disabel mirror\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}

LDEFUN(onu_atu_flood_for_861,
         onu_atu_flood_cmd_for_861,
         "atu flood <slotport_list> {[1|0]}*1",
         "Mac table config\n"
         "Mac table config\n"
         "input the onu fe slot/port list\n"
         "1-enable flood mode\n"
         "0-disable flood mode\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}

DEFUN  (
    onu_igmpsnoop_gda_add_for861,
    onu_igmpsnoop_gda_add_cmd_for861,
    "igmpsnooping gda_add <A.B.C.D> <1-4094> <slotport_list>",    
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping gda_add information\n"
    "Please input the Group Ip Address (224.0.1.0~238.255.255.255)\n"
    "Please input the vlan id in which the multicast frames are forward"
    "Please input slot/port list\n"
    ) 
{
#ifdef __RPC_ONU_COMMAND
    return rpc_onu_dev_cli_process( vty, "igmpsnooping gda_add", NULL );
#else
	return onu_cli_process( vty, "igmpsnooping gda_add" );
#endif
}

LDEFUN  (
    onu_igmpsnoop_gda_add_for861,
    onu_igmpsnoop_gda_add_cmd_for861,
    "igmpsnooping gda_add <A.B.C.D> <1-4094> <slotport_list>",    
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping gda_add information\n"
    "Please input the Group Ip Address (224.0.1.0~238.255.255.255)\n"
    "Please input the vlan id in which the multicast frames are forward"
    "Please input slot/port list\n",
    ONU_NODE
    ) 
{
	return CMD_SUCCESS;
}

DEFUN  (
    onu_port_igmpsnoop_fast_leave_for_861,
    onu_port_igmpsnoop_fast_leave_cmd_for_861,
    "igmpsnooping fast_leave [<slotport_list>|all] {[0|1]}*1",
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping fastleave enable information\n"
    "Input the slot/port list\n"
    "All ports\n"
    "Snooping disable\n"
    "Snooping enable\n"
    )
{
	return onu_cli_process( vty, "igmpsnooping fast_leave" );
}

LDEFUN  (
    onu_port_igmpsnoop_fast_leave_for_861,
    onu_port_igmpsnoop_fast_leave_cmd_for_861,
    "igmpsnooping fast_leave [<slotport_list>|all] {[0|1]}*1",
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping fastleave enable information\n"
    "Input the slot/port list\n"
    "All ports\n"
    "Snooping disable\n"
    "Snooping enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

DEFUN  (
    onu_vlan_pvid_for_861,
    onu_vlan_pvid_cmd_for_861,
    "vlan pvid <slotport_list> {<1-4094>}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu vlan pvid\n"
    "Please input the slot/port list\n"
    "Please input the pvid(1-4094)\n"
    )
{
	return onu_cli_process( vty, "vlan pvid" );
}

LDEFUN  (
    onu_vlan_pvid_for_861,
    onu_vlan_pvid_cmd_for_861,
    "vlan pvid <slotport_list> {<1-4094>}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu vlan pvid\n"
    "Please input the slot/port list\n"
    "Please input the pvid(1-4094)\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


DEFUN  (
    onu_vlan_forward_type_for_861,
    onu_vlan_forward_type_cmd_for_861,
    "vlan acceptable_frame_types <slotport_list> {[tagged|all]}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu vlan forward type\n"
    "Please input the slot/port list\n"
    "Only forward tagged frames\n"
    "Forward untagged and tagged frames\n"
    )
{
	return onu_cli_process( vty, "vlan acceptable_frame_types" );
}

DEFUN  (
    onu_vlan_ingress_filter_for_861,
    onu_vlan_ingress_filter_cmd_for_861,
    "vlan ingress_filtering <slotport_list> {[0|1]}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu vlan ingress filter\n"
    "Please input the slot/port list\n"
    "Disable ingress filter\n"
    "Enable ingress filter\n"
    )
{
	return onu_cli_process( vty, "vlan ingress_filtering" );
}

DEFUN  (
    onu_vlan_dot1q_port_add_for_861,
    onu_vlan_dot1q_port_add_cmd_for_861,
    "vlan dot1q_port_add <2-4094> <slotport_list> [1|2]",    
    "Show or config onu vlan information\n"
    "Add 802.1Q Vlan port\n"
    "Please input the Vlan Vid(2-4094)\n"
    "Please input the slot/port list\n"
    "Tagged\n"
    "Untagged\n"
    )
{
	return onu_cli_process( vty, "vlan dot1q_port_add" );
}   

DEFUN (
    onu_vlan_dot1q_port_del_for_861,
    onu_vlan_dot1q_port_del_cmd_for_861,
    "vlan dot1q_port_del <2-4094> <slotport_list>",    
    "Show or config onu vlan information\n"
    "Delete 802.1Q Vlan port\n"
    "Please input the Vlan Vid\n"
    "Please input the slot/port list\n"
    )
{
	return onu_cli_process( vty, "vlan dot1q_port_del" );
}  

LDEFUN  (
    onu_vlan_forward_type_for_861,
    onu_vlan_forward_type_cmd_for_861,
    "vlan acceptable_frame_types <slotport_list> {[tagged|all]}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu vlan forward type\n"
    "Please input the slot/port list\n"
    "Only forward tagged frames\n"
    "Forward untagged and tagged frames\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_vlan_ingress_filter_for_861,
    onu_vlan_ingress_filter_cmd_for_861,
    "vlan ingress_filtering <slotport_list> {[0|1]}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu vlan ingress filter\n"
    "Please input the slot/port list\n"
    "Disable ingress filter\n"
    "Enable ingress filter\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_vlan_dot1q_port_add_for_861,
    onu_vlan_dot1q_port_add_cmd_for_861,
    "vlan dot1q_port_add <2-4094> <slotport_list> [1|2]",    
    "Show or config onu vlan information\n"
    "Add 802.1Q Vlan port\n"
    "Please input the Vlan Vid(2-4094)\n"
    "Please input the slot/port list\n"
    "Tagged\n"
    "Untagged\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}   

LDEFUN (
    onu_vlan_dot1q_port_del_for_861,
    onu_vlan_dot1q_port_del_cmd_for_861,
    "vlan dot1q_port_del <2-4094> <slotport_list>",    
    "Show or config onu vlan information\n"
    "Delete 802.1Q Vlan port\n"
    "Please input the Vlan Vid\n"
    "Please input the slot/port list\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}  

LONG GetMacAddr1( CHAR * szStr, CHAR * pucMacAddr )
{
    CHAR * p = NULL, *q = NULL, *pcTmp = NULL;
    CHAR cTmp[ 3 ];
    ULONG i = 0, j = 0, k = 0;

    /* 检查字符串长度是否正常 */
    if ( 14 != VOS_StrLen( szStr ) )
    {
        return VOS_ERROR;
    }

    p = szStr;
    for ( i = 0; i < 3; i++ )
    {
        if ( i != 2 )
        {
            /* 查看有无'.' */
            q = VOS_StrChr( p, '.' );
            if ( NULL == p )
            {
                return VOS_ERROR;
            }
        }
        else
        {
            q = szStr + VOS_StrLen( szStr );
        }

        /* 一个H不是4个字符 */
        if ( 4 != q - p )
        {
            return VOS_ERROR;
        }
        /* 检查是否是16进制的数字 */
        for ( j = 0; j < 4; j++ )
        {
            if ( !( ( *( p + j ) >= '0' && *( p + j ) <= '9' ) || ( *( p + j ) >= 'a' && *( p + j ) <= 'f' )
                    || ( *( p + j ) >= 'A' && *( p + j ) <= 'F' ) ) )
            {
                return VOS_ERROR;
            }
        }

        cTmp[ 0 ] = *p;
        cTmp[ 1 ] = *( p + 1 );
        cTmp[ 2 ] = '\0';

        pucMacAddr[ k ] = ( CHAR ) VOS_StrToUL( cTmp, &pcTmp, 16 );
        k++;

        cTmp[ 0 ] = *( p + 2 );
        cTmp[ 1 ] = *( p + 3 );
        cTmp[ 2 ] = '\0';

        pucMacAddr[ k ] = ( CHAR ) VOS_StrToUL( cTmp, &pcTmp, 16 );
        k++;

        p = q + 1;
    }

    return VOS_OK;
}

DEFUN (
    onu_atu_static_add_for_861,
    onu_atu_static_add_cmd_for_861,
    "atu static_add <H.H.H> <slotport_list> {<1-4094>}*1",    
    "Show or config onu atu information\n"
    "Add static mac\n"
    "Please input the mac address\n"
    "Please input the slot/port list\n"
    "VLAN id\n"
    )
{
	LONG lRet;
	CHAR pBuff[256];   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length = 0;
	ULONG ulRet;
	CHAR MacAddr[6] = {0,0,0,0,0,0};
	/*ULONG ulFePort = 0;*/
	
	cliPayload *stPayload=NULL;

	if( check_onu_cli_is_deny(vty) )	/* modified by xieshl 20100521 */
		return CMD_SUCCESS;
	
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;
 
	ponID = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       } 
	
	/*modified by wutw at 20 October*/
	VOS_Sprintf(pBuff,"atu static_add ");
	length = strlen(pBuff);
	
	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}
	
	VOS_Sprintf(&(pBuff[length]),"%02x.%02x.%02x.%02x.%02x.%02x 0 %s",MacAddr[0],\
				MacAddr[1],MacAddr[2],MacAddr[3],MacAddr[4],MacAddr[5],argv[1]);
	length = strlen(pBuff);
	if( argc == 3 )
		VOS_Sprintf(&(pBuff[length])," %s",argv[2]);
		
	length = strlen(pBuff);
	/*vty_out( vty, " %s\r\n",pBuff);
	vty_out( vty, "length %d\r\n",length);*/
	
	/*VOS_MemCpy( pBuff, vty->buf, vty->length);*/
	/*length = vty->length;	*/
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;
	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu static_add failed!\r\n");	

	return CMD_SUCCESS;
}

DEFUN (
    onu_atu_static_add_dup_for_861,
    onu_atu_static_add__dup_cmd_for_861,
    "atu static_add <H.H.H> <slotport_list> <1-4094> {<0-7>}*1",    
    "Show or config onu atu information\n"
    "Add static mac\n"
    "Please input the mac address\n"
    "Please input the slot/port list\n"
    "VLAN id\n"
    "MAC address's priority\n"
    /*"Please input the port_list. e.g.1,3-4\n"*/
    )
{
	LONG lRet;
	CHAR pBuff[256];   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length = 0;
	ULONG ulRet;
	CHAR MacAddr[6] = {0,0,0,0,0,0};
	/*ULONG ulFePort = 0;*/
	
	cliPayload *stPayload=NULL;

	if( check_onu_cli_is_deny(vty) )	/* modified by xieshl 20100521 */
		return CMD_SUCCESS;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;
 
	ponID = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       } 

	/*modified by wutw at 20 October*/
	VOS_Sprintf(pBuff,"atu static_add ");
	length = strlen(pBuff);
	
	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}	
	VOS_Sprintf(&(pBuff[length]),"%02x.%02x.%02x.%02x.%02x.%02x 0 %s %s",MacAddr[0],\
				MacAddr[1],MacAddr[2],MacAddr[3],MacAddr[4],MacAddr[5],argv[1],argv[2]);
	length = strlen(pBuff);
	if( argc == 4 )
		VOS_Sprintf(&(pBuff[length])," %s",argv[3]);
		
	length = strlen(pBuff);
	/*vty_out( vty, " %s\r\n",pBuff);
	vty_out( vty, "length %d\r\n",length);*/
	
	/*VOS_MemCpy( pBuff, vty->buf, vty->length);*/
	/*length = vty->length;	*/
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu static_add failed!\r\n");	

	return CMD_SUCCESS;
}

DEFUN  (
    onu_fe_port_en_set_for_861,
    onu_fe_port_en_set_cmd_for_861,
    "port en <slotport_list> {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port EN information\n"
    "Please input the slot/port list\n"
    "Disable\n"
    "Enable\n"
    )
{
	return onu_cli_process( vty, "port en" );
}

DEFUN  (
    onu_fe_port_desc_set_for_861,
    onu_fe_port_desc_set_cmd_for_861,
    "port desc <slotport_list> {<string>}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port specific description information\n"
    "Please input the slot/port list\n"
    "Please input the descriptor\n"
    )
{
	return onu_cli_process( vty, "port desc" );
}

DEFUN  (
    onu_fe_port_mode_set_for_861,
    onu_fe_port_mode_set_cmd_for_861,
    "port mode <slotport_list> {[0|8|9|10|11|12]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port mode information\n"
    "Please input the slot/port list\n"
    "Auto negotiation\n"
    "100M/FD\n"
    "100M/HD\n"
    "10M/FD\n"
    "10M/HD\n"
    "1000M/FD\n"
    /*"1000M/FD,only for "DEVICE_TYPE_NAME_GT816_STR"\n"*/
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
    	short int PonPortIdx = 0;
	short int OnuIdx=0;
	LONG lRet;
	int OnuType = 0;

	if(( argc > 1) && ((ULONG)VOS_AtoL( argv[1]) == 12 ))
		{
		lRet = PON_GetSlotPortOnu((ULONG)(vty->index ), &ulSlot, &ulPort, &ulOnuid );
		if(lRet != VOS_OK)
			return(CMD_WARNING);

		PonPortIdx = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort);
		OnuIdx=ulOnuid-1;
		CHECK_ONU_RANGE
		
		if(GetOnuType(PonPortIdx, OnuIdx, &OnuType) != VOS_OK)
			return (CMD_WARNING);
		if( OnuType != V2R1_ONU_GT816)
			{
			vty_out(vty, "1000M eth-port is not supported for this onu\r\n");
			return CMD_WARNING;
			}
		}	
	return onu_cli_process( vty, "port mode" );
}

DEFUN  (
    onu_fe_port_negoration_show_for_861,
    onu_fe_port_negoration_show_cmd_for_861,
    "port mode_show <slotport_list>",    
    "Show or config onu FE port information\n"
    "Show negoration of onu FE port mode\n"
    "Please input the slot/port list\n"
    )
{
	return onu_cli_process( vty, "port mode_show" );
}

DEFUN  (
    onu_fe_port_fc_set_for_861,
    onu_fe_port_fc_set_cmd_for_861,
    "port fc <slotport_list> {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port fc information\n"
    "Please input the slot/port list\n"
    "Disable\n"
    "Enable\n"
    )
{
	return onu_cli_process( vty, "port fc" );
}

/*
DEFUN  (
    onu_fe_port_rate_set_for_861,
    onu_fe_port_rate_set_cmd_for_861,
    "port rate <slotport_list> [1|2] {<rate>}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the slot/port list\n"
    "Ingress\n"
    "Egress\n"
    "Please input the rate( 0.1-100 Mbps)\n"
    )
{
	return onu_cli_process( vty, "port rate" );
}
*/

DEFUN  (
    onu_fe_ingress_rate_set_for_861,
    onu_fe_ingress_rate_set_cmd_for_861,
    "port ingress_rate <slotport_list> {[0|1|2|3] [<0>|<62-100000>]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the slot/port list\n"
    "0-Limit all frames\n"
	"1-Limit Broadcast, Multicast and flooded unicast frames\n"
	"2-Limit Broadcast and Multicast frames only\n"
	"3-Limit Broadcast frames only\n"
	"0-not limit\n"
     /*"port ingress rate,unit:kbps,for "DEVICE_TYPE_NAME_GT810_STR"/"DEVICE_TYPE_NAME_GT816_STR",the rate is 100-100000;for "DEVICE_TYPE_NAME_GT811A_STR"/"DEVICE_TYPE_NAME_GT812A_STR"/"DEVICE_TYPE_NAME_GT861_STR",Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64M are supported\n" */
    "port ingress rate,unit:kbps, Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64M are supported\n"
    )
{
	return onu_cli_process( vty, "port ingress_rate" );
}

DEFUN  (
    onu_fe_ingress_rate_set1_for_861,
    onu_fe_ingress_rate_set_cmd1_for_861,
    "port ingress_rate <slotport_list> [0|1|2|3] [<0>|<62-100000>] {[drop|pause] [12k|24k|48k|96k]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the slot/port list\n"
    "0-Limit all frames\n"
	"1-Limit Broadcast, Multicast and flooded unicast frames\n"
	"2-Limit Broadcast and Multicast frames only\n"
	"3-Limit Broadcast frames only\n"
	"0-not limit\n"
     /*"port ingress rate,unit:kbps,for "DEVICE_TYPE_NAME_GT810_STR"/"DEVICE_TYPE_NAME_GT816_STR",the rate is 100-100000;for "DEVICE_TYPE_NAME_GT811A_STR"/"DEVICE_TYPE_NAME_GT812A_STR"/"DEVICE_TYPE_NAME_GT861_STR",Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64M are supported\n"*/
     "port ingress rate,unit:kbps, Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64M are supported\n"
    "Frames will be dropped when exceed limit.\n"
    "Port will transmit pause frame when exceed limit.\n"
    "Burst mode : Burst Size is 12k bytes.\n" 
    "Burst mode : Burst Size is 24k bytes.\n"
    "Burst mode : Burst Size is 48k bytes.\n" 
    "Burst mode : Burst Size is 96k bytes.\n"
    )
{
	return onu_cli_process( vty, "port ingress_rate" );
}

DEFUN  (
    onu_fe_egress_rate_set_for_861,
    onu_fe_egress_rate_set_cmd_for_861,
    "port egress_rate <slotport_list> {[0|<62-100000>]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the slot/port list\n"
    "0-not limit\n"
    /*"Please input the rate( 62-100000) kbps,for "DEVICE_TYPE_NAME_GT861_STR",Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64Mbps are supported\n" */
    "Please input the rate( 62-100000) kbps,Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64Mbps are supported\n"
    )
{
	return onu_cli_process( vty, "port egress_rate" );
}

DEFUN  (
    onu_fe_port_mirror_from_set_for_861,
    onu_fe_port_mirror_from_set_cmd_for_861,
    "port mirror_from {[i|e|a] <slotport_list> [0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE mirror port enable\n"
    "ingress\n"
    "egress\n"
    "All direction\n"
    "Plseas input the slot/port list\n"
    "not a source mirror\n"
    "as a source mirror\n"
    )
{
	return onu_cli_process( vty, "port mirror_from" );
}

DEFUN  (
    onu_fe_port_mirror_to_set_for_861,
    onu_fe_port_mirror_to_set_cmd_for_861,
    "port mirror_to {[i|e] [<slotport_list>|0]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE object mirror port\n"
    "ingress\n"
    "egress\n"
    "Plseas input the slot/port list\n"
    "Delete the mirror port\n"
    )
{
	return onu_cli_process( vty, "port mirror_to" );
}

DEFUN  (
    onu_qos_def_pri_for_861,
    onu_qos_def_pri_cmd_for_861,
    "qos def_pri <slotport_list> {<0-7>}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu FE port default 802.1p priority\n"
    "Please input the slot/port list\n"
    "Please input def_pri(0-7)\n"
    )
{
	return onu_cli_process( vty, "qos def_pri" );
}

DEFUN  (
    onu_qos_user_pri_reg_for_861,
    onu_qos_user_pri_reg_cmd_for_861,
    "qos user_pri_reg <slotport_list> <0-7> {<0-7>}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu FE port replacing 802.1p priority\n"
    "Please input the slot/port list\n"
    "Please input old_pri(0-7)\n"
    "Please input new_pri(0-7)\n"
    )
{
	return onu_cli_process( vty, "qos user_pri_reg" );
}

DEFUN  (
    onu_qos_user_pri_reg_show_for_861,
    onu_qos_user_pri_reg_show_cmd_for_861,
    "qos user_pri_reg <slotport_list>",    
    "Show onu QoS information\n"
    "Show or config onu FE port replacing 802.1p priority\n"
    "Please input the slot/port list\n"
    )
{
	return onu_cli_process( vty, "qos user_pri_reg" );
}

DEFUN  (
    onu_qos_rule_for_861,
    onu_qos_rule_cmd_for_861,
    "qos rule <slotport_list> {[user|ip]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu current running Qos rule(802.1p or IP-DSCP)\n"
    "Please input the slot/port list\n"
    "IEEE tag priority\n"
    "IP priority\n"
    )
{
	return onu_cli_process( vty, "qos rule" );
}

DEFUN  (
    onu_qos_ip_pri_en_for_861,
    onu_qos_ip_pri_en_cmd_for_861,
    "qos ip_pri_en <slotport_list> {[0|1]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu IP-DSCP priority enable status\n"
    "Please input the slot/port list\n"
    "Disable\n"
    "Enable\n"
    )
{
	return onu_cli_process( vty, "qos ip_pri_en" );
}

DEFUN  (
    onu_qos_user_pri_en_for_861,
    onu_qos_user_pri_en_cmd_for_861,
    "qos user_pri_en <slotport_list> {[0|1]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu 802.1p priority enable status\n"
    "Please input the slot/port list\n"
    "Disable\n"
    "Enable\n"
    )
{
	return onu_cli_process( vty, "qos user_pri_en" );
}

DEFUN  (
    onu_loopback_port_en_for_861,
    onu_loopback_port_en_cmd_for_861,
    "eth-loop port_en <slotport_list> {[0|1]}*1",
    "Show or config onu loopback information\n"
    "Show or config onu loopback port_en information\n"
    "Please input the slot/port list\n"
    "Disable\n"
    "Enable\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	VOS_MemCpy(&vty->buf[0], "loopback", 8);
	return onu_cli_process( vty, "eth-loop port_en" );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length = 0;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
    {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
    }

	if (argc == 1)
		VOS_Sprintf(pBuff,"loopback port_en %s",argv[0]);
	else if (argc == 2)
	{
		VOS_Sprintf(pBuff,"loopback port_en %s %s",argv[0],argv[1]);
	}
	else
		vty_out( vty, "  %% Parameter error\r\n");
	
	length = VOS_StrLen(pBuff);
 
	/*
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	*/
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;
	
	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%eth-loop port_en !\r\n");	


	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_statistic_port_flush_for_861,
    onu_statistic_port_flush_cmd_for_861,
    "stat port_flush {<slotport_list>}*1",    
    "Show or config onu Statistic information\n"
    "Clear onu Statistic information\n"
    "Please input slot/port list\n"
    )
{
	return onu_cli_process( vty, "stat port_flush" );
}

DEFUN  (
	onu_statistic_port_show_for_861,
	onu_statistic_port_show_cmd_for_861,
	"stat port_show <slotport_list>", 
	"Show or config onu Statistic information\n"
	"Show onu Statistic information\n"
	"Please input slot/port list\n"
	)
{
	return onu_cli_process( vty, "stat port_show" );
}

DEFUN(onu_atu_learn_for_861,
         onu_atu_learn_cmd_for_861,
         "atu learning <slotport_list> {[1|0]}*1",
         "Mac table config\n"
         "Uni mac learn configuration\n" 
         "input the onu slot/port list\n"
         "1-learning enable\n"
         "0-learning disable\n")
{
	return onu_cli_process( vty, "atu learning" );
}

DEFUN(onu_cable_test_gt811_for_861,
		onu_cable_test_gt811_cmd_for_861,
		"cable test port <slotport_list>", 
		"Ethernet cable test\n" 
		"Test ethernet cable characters\n"
		"slot/port list\n"
		DescStringPortList
 		)
{
	return onu_cli_process( vty, "cable test port" );
}

LDEFUN (
    onu_atu_static_add_for_861,
    onu_atu_static_add_cmd_for_861,
    "atu static_add <H.H.H> <slotport_list> {<1-4094>}*1",    
    "Show or config onu atu information\n"
    "Add static mac\n"
    "Please input the mac address\n"
    "Please input the slot/port list\n"
    "VLAN id\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN (
    onu_atu_static_add_dup_for_861,
    onu_atu_static_add__dup_cmd_for_861,
    "atu static_add <H.H.H> <slotport_list> <1-4094> {<0-7>}*1",    
    "Show or config onu atu information\n"
    "Add static mac\n"
    "Please input the mac address\n"
    "Please input the slot/port list\n"
    "VLAN id\n"
    "MAC address's priority\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_port_en_set_for_861,
    onu_fe_port_en_set_cmd_for_861,
    "port en <slotport_list> {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port EN information\n"
    "Please input the slot/port list\n"
    "Disable\n"
    "Enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_port_desc_set_for_861,
    onu_fe_port_desc_set_cmd_for_861,
    "port desc <slotport_list> {<string>}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port specific description information\n"
    "Please input the slot/port list\n"
    "Please input the descriptor\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_port_mode_set_for_861,
    onu_fe_port_mode_set_cmd_for_861,
    "port mode <slotport_list> {[0|8|9|10|11|12]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port mode information\n"
    "Please input the slot/port list\n"
    "Auto negotiation\n"
    "100M/FD\n"
    "100M/HD\n"
    "10M/FD\n"
    "10M/HD\n"
    "1000M/FD\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_port_negoration_show_for_861,
    onu_fe_port_negoration_show_cmd_for_861,
    "port mode_show <slotport_list>",    
    "Show or config onu FE port information\n"
    "Show negoration of onu FE port mode\n"
    "Please input the slot/port list\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_port_fc_set_for_861,
    onu_fe_port_fc_set_cmd_for_861,
    "port fc <slotport_list> {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port fc information\n"
    "Please input the slot/port list\n"
    "Disable\n"
    "Enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

/*
DEFUN  (
    onu_fe_port_rate_set_for_861,
    onu_fe_port_rate_set_cmd_for_861,
    "port rate <slotport_list> [1|2] {<rate>}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the slot/port list\n"
    "Ingress\n"
    "Egress\n"
    "Please input the rate( 0.1-100 Mbps)\n"
    )
{
	return onu_cli_process( vty, "port rate" );
}
*/

LDEFUN  (
    onu_fe_ingress_rate_set_for_861,
    onu_fe_ingress_rate_set_cmd_for_861,
    "port ingress_rate <slotport_list> {[0|1|2|3] [<0>|<62-100000>]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the slot/port list\n"
    "0-Limit all frames\n"
	"1-Limit Broadcast, Multicast and flooded unicast frames\n"
	"2-Limit Broadcast and Multicast frames only\n"
	"3-Limit Broadcast frames only\n"
	"0-not limit\n"
     /*"port ingress rate,unit:kbps,for "DEVICE_TYPE_NAME_GT810_STR"/"DEVICE_TYPE_NAME_GT816_STR",the rate is 100-100000;for "DEVICE_TYPE_NAME_GT811A_STR"/"DEVICE_TYPE_NAME_GT812A_STR"/"DEVICE_TYPE_NAME_GT861_STR",Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64M are supported\n" */
    "port ingress rate,unit:kbps, Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64M are supported\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_ingress_rate_set1_for_861,
    onu_fe_ingress_rate_set_cmd1_for_861,
    "port ingress_rate <slotport_list> [0|1|2|3] [<0>|<62-100000>] {[drop|pause] [12k|24k|48k|96k]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the slot/port list\n"
    "0-Limit all frames\n"
	"1-Limit Broadcast, Multicast and flooded unicast frames\n"
	"2-Limit Broadcast and Multicast frames only\n"
	"3-Limit Broadcast frames only\n"
	"0-not limit\n"
     /*"port ingress rate,unit:kbps,for "DEVICE_TYPE_NAME_GT810_STR"/"DEVICE_TYPE_NAME_GT816_STR",the rate is 100-100000;for "DEVICE_TYPE_NAME_GT811A_STR"/"DEVICE_TYPE_NAME_GT812A_STR"/"DEVICE_TYPE_NAME_GT861_STR",Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64M are supported\n"*/
     "port ingress rate,unit:kbps, Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64M are supported\n"
    "Frames will be dropped when exceed limit.\n"
    "Port will transmit pause frame when exceed limit.\n"
    "Burst mode : Burst Size is 12k bytes.\n" 
    "Burst mode : Burst Size is 24k bytes.\n"
    "Burst mode : Burst Size is 48k bytes.\n" 
    "Burst mode : Burst Size is 96k bytes.\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_egress_rate_set_for_861,
    onu_fe_egress_rate_set_cmd_for_861,
    "port egress_rate <slotport_list> {[0|<62-100000>]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the slot/port list\n"
    "0-not limit\n"
    /*"Please input the rate( 62-100000) kbps,for "DEVICE_TYPE_NAME_GT861_STR",Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64Mbps are supported\n" */
    "Please input the rate( 62-100000) kbps,Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64Mbps are supported\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_port_mirror_from_set_for_861,
    onu_fe_port_mirror_from_set_cmd_for_861,
    "port mirror_from {[i|e|a] <slotport_list> [0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE mirror port enable\n"
    "ingress\n"
    "egress\n"
    "All direction\n"
    "Plseas input the slot/port list\n"
    "not a source mirror\n"
    "as a source mirror\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_port_mirror_to_set_for_861,
    onu_fe_port_mirror_to_set_cmd_for_861,
    "port mirror_to {[i|e] [<slotport_list>|0]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE object mirror port\n"
    "ingress\n"
    "egress\n"
    "Plseas input the slot/port list\n"
    "Delete the mirror port\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_qos_def_pri_for_861,
    onu_qos_def_pri_cmd_for_861,
    "qos def_pri <slotport_list> {<0-7>}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu FE port default 802.1p priority\n"
    "Please input the slot/port list\n"
    "Please input def_pri(0-7)\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_qos_user_pri_reg_for_861,
    onu_qos_user_pri_reg_cmd_for_861,
    "qos user_pri_reg <slotport_list> <0-7> {<0-7>}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu FE port replacing 802.1p priority\n"
    "Please input the slot/port list\n"
    "Please input old_pri(0-7)\n"
    "Please input new_pri(0-7)\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_qos_user_pri_reg_show_for_861,
    onu_qos_user_pri_reg_show_cmd_for_861,
    "qos user_pri_reg <slotport_list>",    
    "Show onu QoS information\n"
    "Show or config onu FE port replacing 802.1p priority\n"
    "Please input the slot/port list\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_qos_rule_for_861,
    onu_qos_rule_cmd_for_861,
    "qos rule <slotport_list> {[user|ip]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu current running Qos rule(802.1p or IP-DSCP)\n"
    "Please input the slot/port list\n"
    "IEEE tag priority\n"
    "IP priority\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_qos_ip_pri_en_for_861,
    onu_qos_ip_pri_en_cmd_for_861,
    "qos ip_pri_en <slotport_list> {[0|1]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu IP-DSCP priority enable status\n"
    "Please input the slot/port list\n"
    "Disable\n"
    "Enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_qos_user_pri_en_for_861,
    onu_qos_user_pri_en_cmd_for_861,
    "qos user_pri_en <slotport_list> {[0|1]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu 802.1p priority enable status\n"
    "Please input the slot/port list\n"
    "Disable\n"
    "Enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_loopback_port_en_for_861,
    onu_loopback_port_en_cmd_for_861,
    "eth-loop port_en <slotport_list> {[0|1]}*1",
    "Show or config onu loopback information\n"
    "Show or config onu loopback port_en information\n"
    "Please input the slot/port list\n"
    "Disable\n"
    "Enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_statistic_port_flush_for_861,
    onu_statistic_port_flush_cmd_for_861,
    "stat port_flush {<slotport_list>}*1",    
    "Show or config onu Statistic information\n"
    "Clear onu Statistic information\n"
    "Please input slot/port list\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
	onu_statistic_port_show_for_861,
	onu_statistic_port_show_cmd_for_861,
	"stat port_show <slotport_list>", 
	"Show or config onu Statistic information\n"
	"Show onu Statistic information\n"
	"Please input slot/port list\n",
    ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(onu_atu_learn_for_861,
         onu_atu_learn_cmd_for_861,
         "atu learning <slotport_list> {[1|0]}*1",
         "Mac table config\n"
         "Uni mac learn configuration\n" 
         "input the onu slot/port list\n"
         "1-learning enable\n"
         "0-learning disable\n",
    ONU_NODE)
{
	return CMD_SUCCESS;
}

LDEFUN(onu_cable_test_gt811_for_861,
		onu_cable_test_gt811_cmd_for_861,
		"cable test port <slotport_list>", 
		"Ethernet cable test\n" 
		"Test ethernet cable characters\n"
		"slot/port list\n"
		DescStringPortList,
   		ONU_NODE
 		)
{
	return CMD_SUCCESS;
}


#if 1
/**********************Dr.Peng拓展 add by yangzl@2017-4-10*************/
#define SUPPORTATTRIBUTELEN 17
#define MAXSUPPORTATTRIBUTELEN 2
#define DRPENG_ONU_PORT_LOCATION_BY_MAC               0 /* Onu ethernet Port Location By MAC*/
#define DRPENG_ONU_PORT_MAC_NUMBER                     1 /* Onu  Port MAC address number*/
#define DRPENG_ONU_MAC_TABLE                                 2 /* Onu   MAC address table*/
#define DRPENG_ONU_DEVICE_NAME                                 3 /* Onu   device name*/
#define DRPENG_ONU_DEVICE_DESCRIPTION                                  4 /* Onu   device Description*/
#define DRPENG_ONU_DEVICE_LOCATION                                  5 /* Onu   device location*/
#define DRPENG_ONU_PORT_ISOLATION                                 6 /* Onu   port isolation*/
#define DRPENG_ONU_PORT_STORM                                7 /* Onu   port storm status*/
#define DRPENG_ONU_PORT_MODE                                 8 /* Onu   port mode*/
#define DRPENG_ONU_LOOP_DETECTION_TIME                                  9/*Onu   loop detection time */
#define DRPENG_ONU_SAVE_CONFIG                                10/* Onu  port Save the configuration*/


long  CmpBit(unsigned short i,u_char   * var_val,size_t var_val_len)/*比较bit位*/
{
	unsigned short  quotient = 0;/*商数*/
	unsigned short  remainder = 0;/*余数*/
	
	if(i > var_val_len)
		return VOS_ERROR;

	quotient = i/8;
	remainder = i%8;
	switch(remainder){
		case 0:if(var_val[quotient ]&0x01)return VOS_OK;
			break;
		case 1:if(var_val[quotient]&0x02)return VOS_OK;
			break;
		case 2:if(var_val[quotient]&0x04)return VOS_OK;
			break;
		case 3:if(var_val[quotient]&0x08)return VOS_OK;
			break;
		case 4:if(var_val[quotient]&0x10)return VOS_OK;
			break;
		case 5:if(var_val[quotient]&0x20)return VOS_OK;
			break;
		case 6:if(var_val[quotient]&0x40)return VOS_OK;
			break;
		case 7:if(var_val[quotient]&0x80)return VOS_OK;
			break;
		default:
			return VOS_ERROR;
			break;
	}
	return VOS_ERROR;
}

long WhetherSupportDrPengAttri(short int PonPortIdx, short int OnuIdx,unsigned short Attri)
{
	int OnuEntryIdx;
	long ret = VOS_OK;
	unsigned char SupportAttribute[SUPPORTATTRIBUTELEN];

	VOS_MemZero(SupportAttribute,SUPPORTATTRIBUTELEN);
	OnuEntryIdx = MAXONUPERPON * PonPortIdx + OnuIdx;
	if(OnuMgmtTable[OnuEntryIdx].DrPengSupportAttribute.get_flag == 1)
	{
		VOS_MemCpy( SupportAttribute, OnuMgmtTable[OnuEntryIdx].DrPengSupportAttribute.SupportAttribute, MAXSUPPORTATTRIBUTELEN );
	}
	else
	{
		OnuMgmtTable[OnuEntryIdx].DrPengSupportAttribute.get_flag = 1;
		VOS_MemZero(OnuMgmtTable[OnuEntryIdx].DrPengSupportAttribute.SupportAttribute,MAXSUPPORTATTRIBUTELEN);
		if(VOS_OK == OnuMgt_DrPengGetOnuExtendAttribute(PonPortIdx, OnuIdx, SupportAttribute))
		{
			OnuMgmtTable[OnuEntryIdx].DrPengSupportAttribute.support_flag = 1;
			VOS_MemCpy( OnuMgmtTable[OnuEntryIdx].DrPengSupportAttribute.SupportAttribute,SupportAttribute , MAXSUPPORTATTRIBUTELEN );
		}
		else
		{
			OnuMgmtTable[OnuEntryIdx].DrPengSupportAttribute.support_flag = 0;
			return VOS_ERROR;
		}
	}
	if(OnuMgmtTable[OnuEntryIdx].DrPengSupportAttribute.support_flag == 1)
	{
		ret = CmpBit(Attri,SupportAttribute,MAXSUPPORTATTRIBUTELEN*8);
		return ret;
	}
	else
		return VOS_ERROR;
}
DEFUN  (
    DrPeng_onu_get_port_id,
    DrPeng_onu_get_port_id_cmd,
    "show port by mac <H.H.H> {vlan <1-4095>}*1",
    "show onu port by mac and vlan\n"
    "show onu port by mac and vlan\n"
    "show onu port by mac and vlan\n"
    "show onu port by mac and vlan\n"
    "please enter mac address\n"
    "show onu port by mac and vlan\n"
    "please enter vlan id\n"
    )
{
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	unsigned char state;
	mac_address_t mac;
	int vlan = 0;
	LONG lRet = VOS_OK;	
	short int vlan_id = 0;
	OnuPortLacationEntry_S port_location_infor;
	
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);
	
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != VOS_OK )
       {
	   vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
	   return CMD_WARNING;	
       }
       /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
       lRet = 	GetOnuOperStatus( PonPortIdx, OnuIdx);
       if ( CLI_EPON_ONUDOWN == lRet)
      {
   	   vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
      }
	   
	if(VOS_OK != WhetherSupportDrPengAttri(PonPortIdx, OnuIdx,DRPENG_ONU_PORT_LOCATION_BY_MAC) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
	}
	
	VOS_MemZero(mac,sizeof(mac_address_t));
	VOS_MemZero(&port_location_infor,sizeof(OnuPortLacationEntry_S));
	
       if (VOS_ERROR == GetMacAddr(argv[0], mac))
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
    		return CMD_WARNING;
	}
	   
	 if(argc > 1)
	 {
	 	vlan = VOS_StrToUL(argv[1], NULL, 10);
	 	vlan_id = (unsigned short)vlan;
	 }
	 else
	 	vlan_id = 0;
	 
        if(VOS_OK == OnuMgt_DrPengGetOnuPortLocationByMAC(PonPortIdx, OnuIdx, mac, vlan_id,&port_location_infor))
        {
        	if(port_location_infor.result_get == 0x01)
        	{
        		vty_out(vty, "%10s%5s%10s%5s%10s%5s%10s\r\n","Mac"," ", "Port", " ", "Vlan"," ","Mac_type");
        		if(port_location_infor.mac_type == 0x01)
        		{
        			vty_out(vty, "%02x%02x.%02x%02x.%02x%02x%5s%d%8s%9d%5s%10s\r\n",port_location_infor.mac[0],port_location_infor.mac[1],
					port_location_infor.mac[2],port_location_infor.mac[3],port_location_infor.mac[4],port_location_infor.mac[5],
					" ",port_location_infor.port_id," ",port_location_infor.vlan_id," ","dynamic");
        		}
			else
			{
				vty_out(vty, "%02x%02x.%02x%02x.%02x%02x%5s%d%8s%9d%5s%10s\r\n",port_location_infor.mac[0],port_location_infor.mac[1],
					port_location_infor.mac[2],port_location_infor.mac[3],port_location_infor.mac[4],port_location_infor.mac[5],
					" ",port_location_infor.port_id," ",port_location_infor.vlan_id," ","static");
			}
        	}
		else
			vty_out(vty, "this Mac was not found\r\n");
        }
     else
     	vty_out(vty, "Failed to get onu information\r\n");   

    return CMD_SUCCESS;

}
DEFUN  (
    DrPeng_onu_get_mac_number,
    DrPeng_onu_get_mac_number_cmd,
    "show mac-number <port_list>",
    "show onu port mac number\n"
    "show onu port mac number\n"
    "please input the port number\n\n"
    )
{
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	unsigned char state;
	LONG lRet = VOS_OK;	
	unsigned short  mac_address_number=0,port;
	
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);
	
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
       if(ThisIsValidOnu(PonPortIdx, OnuIdx) != VOS_OK )
       {
	   vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
	   return CMD_WARNING;	
       }
       /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
       lRet = 	GetOnuOperStatus( PonPortIdx, OnuIdx);
       if ( CLI_EPON_ONUDOWN == lRet)
      {
   	   vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
      }
	   
	if(VOS_OK != WhetherSupportDrPengAttri(PonPortIdx, OnuIdx,DRPENG_ONU_PORT_MAC_NUMBER) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
	}
	
	 BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
        {
              if(VOS_OK == OnuMgt_DrPengGetOnuPortMacNumber(PonPortIdx, OnuIdx,  port, &mac_address_number))
	        {
	       	vty_out(vty, "port %4d    mac number:%d\r\n",port,mac_address_number);   
	        }
		else
			vty_out(vty, "Failed to get port %d mac number\r\n",port);   
        }
       END_PARSE_PORT_LIST_TO_PORT();

    return CMD_SUCCESS;

}
DEFUN  (
    onu_mac_entry_show,
    onu_mac_entry_show_cmd,
    "show onu-mac-entry [dynamic|static]",    
    SHOW_STR
    "Show onu mac entry \n"
    "onu dynamic mac entry \n"
    "onu static mac entry \n"
    )
{
	ULONG   ulIfIndex = 0,mactype;
	ULONG   ulSlot, ulPort, ulOnuid;
	short int PonPortId,userOnuId;
	LONG lRet = VOS_OK;	
	OnuPortLacationInfor_S table;
	int i = 0;
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;	

	if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&ulOnuid) == VOS_ERROR )
		return CMD_WARNING;
	
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
	
	PonPortId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );

	userOnuId = (short int)(ulOnuid - 1);
       if(ThisIsValidOnu(PonPortId, userOnuId) != VOS_OK )
       {
	   vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
	   return CMD_WARNING;	
       }
       /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
       lRet = 	GetOnuOperStatus( PonPortId, userOnuId);
       if ( CLI_EPON_ONUDOWN == lRet)
      {
   	   vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
      }
	   
	if(ThisIsValidOnu(PonPortId, userOnuId) != VOS_OK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot,ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	
	if(VOS_OK != WhetherSupportDrPengAttri(PonPortId, userOnuId,DRPENG_ONU_MAC_TABLE) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
	}
	
	if( VOS_StrCmp(argv[0], "dynamic") == 0 )
		mactype = 0x01;
	else 
		mactype = 0x02;

	if(OnuMgt_GetOnuMacEntry(PonPortId,userOnuId,mactype,&table) != VOS_OK)
	{
		vty_out( vty, "  %% get onu_mac_entry error.\r\n");
		return CMD_WARNING;  
	}

	vty_out(vty, "\r\n onu%d/%d/%d mac entries: count:%d\r\n", ulSlot, ulPort, ulOnuid, table.number_entry);	

	  if(table.number_entry)
        {
            vty_out(vty, "%10s%5s%10s%5s%10s\r\n","Mac"," ", "Port", " ", "Vlan");
            for(i=0;i<table.number_entry;i++)
            {
                vty_out(vty, "%d %02x%02x.%02x%02x.%02x%02x%5s%d%8s%9d\r\n",i,table.OnuPortMacTable[i].mac[0],table.OnuPortMacTable[i].mac[1],
					table.OnuPortMacTable[i].mac[2],table.OnuPortMacTable[i].mac[3],table.OnuPortMacTable[i].mac[4],table.OnuPortMacTable[i].mac[5],
					" ",table.OnuPortMacTable[i].port_id," ",table.OnuPortMacTable[i].vlan_id);
            }	
        }

	return CMD_SUCCESS;
}

DEFUN  (
    DrPeng_onu_set_port_isolation,
    DrPeng_onu_set_port_isolation_cmd,
    "port isolation [enable|disable]",
    "onu Setting port isolation\n"
     "onu Setting port isolation\n"
    "onu Setting port isolation enalbe\n"
    "onu Setting port isolation disable\n"
    )
{
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	LONG lRet = VOS_OK;
	unsigned char state;
	short int error_flag = 0;
		
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);
	
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
       if(ThisIsValidOnu(PonPortIdx, OnuIdx) != VOS_OK )
       {
	   vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
	   return CMD_WARNING;	
       }
       /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
       lRet = 	GetOnuOperStatus( PonPortIdx, OnuIdx);
       if ( CLI_EPON_ONUDOWN == lRet)
      {
   	   vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
      }
	   
	if(VOS_OK != WhetherSupportDrPengAttri(PonPortIdx, OnuIdx,DRPENG_ONU_PORT_ISOLATION) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
	}
	
	if ( 0 == VOS_StrnCmp( "enable", argv[0],6) )
             state= 1;
       else if(0 == VOS_StrnCmp( "disable", argv[0],7))
	{
		state= 0;
    	}

        if(VOS_OK != OnuMgt_DrPengSetOnuPortIsolation(PonPortIdx,OnuIdx ,state))
        {
        	  vty_out(vty, "Failed to set onu port isolation\r\n");
        }

    return CMD_SUCCESS;
}
DEFUN  (
    DrPeng_onu_get_port_isolation,
    DrPeng_onu_get_port_isolation_cmd,
    "show port isolation",
    "show port isolation state\n"
    "show port isolation state\n"
    "show port isolation state\n"
    )
{
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	LONG lRet = VOS_OK;
	unsigned char state;
	
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);
	
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
       if(ThisIsValidOnu(PonPortIdx, OnuIdx) != VOS_OK )
       {
	   vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
	   return CMD_WARNING;	
       }
       /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
       lRet = 	GetOnuOperStatus( PonPortIdx, OnuIdx);
       if ( CLI_EPON_ONUDOWN == lRet)
      {
   	   vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
      }
	   
	 if(VOS_OK != WhetherSupportDrPengAttri(PonPortIdx, OnuIdx,DRPENG_ONU_PORT_ISOLATION) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
	}
	 
        if(VOS_OK == OnuMgt_DrPengGetOnuPortIsolation(PonPortIdx,OnuIdx ,&state))
        {
       	switch(state)
		{
			case 0:vty_out(vty, "All Ethernet port isolation of the ONU is disable\r\n"); 
				break;
			case 1:vty_out(vty, "All Ethernet port isolation of the ONU is enabled\r\n"); 
				break;
			default:vty_out(vty, "All Ethernet ports of onu isolation state:unknown\r\n"); 
				break;
		}
        }
     else
     	vty_out(vty, "Failed to get onu information\r\n");   

    return CMD_SUCCESS;

}

DEFUN  (
	DrPeng_onu_set_storm_control,
	DrPeng_onu_set_storm_control_cmd,
	"storm-control <port_list> [unicast|multicast|broadcast|uni-multicast|uni-broadcast|multi-broadcast|all] <64-100000> [kbps|pps] ",
	"Storm control \n"
	"Please input port id\n"
	"unicast storm control\n"
	 "multicast storm control\n"
	"Broadcast storm control\n"
	"unicast and multicast storm control\n"
	"unicast and Broadcast storm control\n"
	"multicast and Broadcast storm control\n"
	"all storm control\n"
	"Please input the control value\n"
	"Unit: PPS or Kbps\n"
	)
{
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG	ulIfIndex = 0;
	ULONG	ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	LONG lRet = VOS_OK;
	short int error_flag = 0;
	unsigned short port_id;
	OnuPortStorm_S  status;
		
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);
	
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != VOS_OK )
       {
	   vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
	   return CMD_WARNING;	
       }
       /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
       lRet = 	GetOnuOperStatus( PonPortIdx, OnuIdx);
       if ( CLI_EPON_ONUDOWN == lRet)
      {
   	   vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
      }
	if(VOS_OK != WhetherSupportDrPengAttri(PonPortIdx, OnuIdx,DRPENG_ONU_PORT_STORM) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
	}   
	if ( 0 == VOS_StrnCmp( "all", argv[1],3) )
			 status.storm_type= 7;
	else if(0 == VOS_StrnCmp( "unicast", argv[1],7))
	{
		status.storm_type= 1;
	}
	else if(0 == VOS_StrnCmp( "multicast", argv[1],7))
	{
		status.storm_type= 2;
	}
	else if(0 == VOS_StrnCmp( "broadcast", argv[1],7))
	{
		status.storm_type= 4;
	}
	else if(0 == VOS_StrnCmp( "uni-multicast", argv[1],7))
	{
		status.storm_type= 3;
	}
	else if(0 == VOS_StrnCmp( "uni-broadcast", argv[1],7))
	{
		status.storm_type= 5;
	}
	else if(0 == VOS_StrnCmp( "multi-broadcast", argv[1],7))
	{
		status.storm_type= 6;
	}
	else
		return CMD_ERR_NO_MATCH;

	 status.storm_rate= ( unsigned int ) VOS_AtoL( argv[ 2 ] );
	 
	if ( 0 == VOS_StrnCmp( "pps", argv[3],3) )
			 status.storm_mode= 0;
	else if(0 == VOS_StrnCmp( "kbps", argv[3],3))
	{
		status.storm_mode= 1;
	}
	else
		return CMD_ERR_NO_MATCH;

	 BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port_id)
        {
              if(VOS_OK != OnuMgt_DrPengSetOnuPortStormStatus(PonPortIdx, OnuIdx, port_id,&status ))
	      		vty_out(vty, "Failed to set onu port %d Storm control\r\n",port_id);
        }
       END_PARSE_PORT_LIST_TO_PORT();

	return CMD_SUCCESS;
}
DEFUN  (
	DrPeng_onu_get_storm_control,
	DrPeng_onu_get_storm_control_cmd,
	"show storm-control <port_list>",
	"show storm-control\n"
	"show storm-control\n"
	"Please input port id\n"
	)
{
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG	ulIfIndex = 0;
	ULONG	ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	LONG lRet = VOS_OK;
	unsigned char state;
	unsigned short port_id;
	OnuPortStorm_S  status;
	unsigned char *mode_array[] = {"pps",  "kbps", };
	
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);
	
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != VOS_OK )
       {
	   vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
	   return CMD_WARNING;	
       }
       /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
       lRet = 	GetOnuOperStatus( PonPortIdx, OnuIdx);
       if ( CLI_EPON_ONUDOWN == lRet)
      {
   	   vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
      }
	if(VOS_OK != WhetherSupportDrPengAttri(PonPortIdx, OnuIdx,DRPENG_ONU_PORT_STORM) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
	}      
	BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port_id)
	{
              if(VOS_OK == OnuMgt_DrPengGetOnuPortStormStatus(PonPortIdx, OnuIdx, port_id,&status))
              {
              	if(status.storm_mode > 1)
					return CMD_WARNING;
              	switch(status.storm_type){
					case 1:vty_out(vty, "port %d unicast storm control rate %d %s\r\n",port_id,status.storm_rate,mode_array[status.storm_mode]);break;
					case 2:vty_out(vty, "port %d multicast storm control rate %d %s\r\n",port_id,status.storm_rate,mode_array[status.storm_mode]);break;
					case 3:vty_out(vty, "port %d unicast and multicast storm control rate %d %s\r\n",port_id,status.storm_rate,mode_array[status.storm_mode]);break;
					case 4:vty_out(vty, "port %d broadcast storm control rate %d %s\r\n",port_id,status.storm_rate,mode_array[status.storm_mode]);break;
					case 5:vty_out(vty, "port %d unicast and broadcast storm control rate %d %s\r\n",port_id,status.storm_rate,mode_array[status.storm_mode]);break;
					case 6:vty_out(vty, "port %d multicast and broadcast storm control rate %d %s\r\n",port_id,status.storm_rate,mode_array[status.storm_mode]);break;
					case 7:vty_out(vty, "port %d all storm control rate %d %s\r\n",port_id,status.storm_rate,mode_array[status.storm_mode]);break;
					default:vty_out(vty, "Failed to get onu port %d Storm control rate\r\n",port_id);break;
              			}
              }
		else
			vty_out(vty, "Failed to get onu port %d Storm control\r\n",port_id);
        }
       END_PARSE_PORT_LIST_TO_PORT();
	   
	return CMD_SUCCESS;

}
DEFUN  (
	DrPeng_onu_set_port_mode,
	DrPeng_onu_set_port_mode_cmd,
	"port mode-peng <port_list> {[10m-half|10m-full|100m-half|100m-full|1000m]}*1 ",
	"Show or config onu FE port information\n"
	"Show or config onu FE port mode information\n"
	"Please input the port number\n"
	"10M-Half\n"
	"10M-Full\n"
	"100M-Half\n"
	"100M-FULL\n"
	"1000M\n"
	)
{
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG	ulIfIndex = 0;
	ULONG	ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	LONG lRet = VOS_OK;
	short int error_flag = 0;
	unsigned short port_id;
	unsigned char  port_mode,i;
	unsigned char *mode_array[] = {"10M-Half",  "10M-Full", "100M-Half","100M-FULL","1000M",};
	
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);
	
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != VOS_OK )
       {
	   vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
	   return CMD_WARNING;	
       }
       /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
       lRet = 	GetOnuOperStatus( PonPortIdx, OnuIdx);
       if ( CLI_EPON_ONUDOWN == lRet)
      {
   	   vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
      }
	 if(VOS_OK != WhetherSupportDrPengAttri(PonPortIdx, OnuIdx,DRPENG_ONU_PORT_MODE) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
	}     
	if(argc == 2)
	{
		if ( 0 == VOS_StrnCmp( "10m-half", argv[1],8) )
			 port_mode = 1;
		else if(0 == VOS_StrnCmp( "10m-full", argv[1],8))
		{
			port_mode = 2;
		}
		else if(0 == VOS_StrnCmp( "100m-half", argv[1],9))
		{
			port_mode = 3;
		}
		else if(0 == VOS_StrnCmp( "100m-full", argv[1],9))
		{
			port_mode = 4;
		}
		else if(0 == VOS_StrnCmp( "1000m", argv[1],5))
		{
			port_mode = 5;
		}
		else
			return CMD_ERR_NO_MATCH;

		 BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port_id)
	        {
	              if(VOS_OK != OnuMgt_DrPengSetOnuPortMode(PonPortIdx, OnuIdx, port_id,port_mode))
		      		vty_out(vty, "Failed to set onu port %d  mode\r\n",port_id);
	        }
	       END_PARSE_PORT_LIST_TO_PORT();

		return CMD_SUCCESS;
	}
	else
	{
		 BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port_id)
	        {
	              if(VOS_OK == OnuMgt_DrPengGetOnuPortMode(PonPortIdx, OnuIdx, port_id,&port_mode))
	              {
	              	if(port_mode > 5)
						continue;
				else
				{
					i = port_mode -1;
					vty_out(vty, "port %d  mode:%s\r\n",port_id,mode_array[i]);
				}
	              }
			else
				vty_out(vty, "Failed to get onu port %d  mode\r\n",port_id);
	        }
	       END_PARSE_PORT_LIST_TO_PORT();

		return CMD_SUCCESS;
	}
}
DEFUN  (
	DrPeng_onu_set_loop_detec,
	DrPeng_onu_set_loop_detec_cmd,
	"loop-detection downtime <10-65535> restart-times <0-65535>",
	"config port loop-detection time\n"
	"config port loop-detection downtime time\n"
	"please enter the value\n"
	"config port loop-detection restart-times time\n"
	"please enter the value\n"
	)
{
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG	ulIfIndex = 0;
	ULONG	ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	LONG lRet = VOS_OK;
	short int error_flag = 0;
	unsigned short downtime,restart_times;
	
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);
	
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != VOS_OK )
       {
	   vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
	   return CMD_WARNING;	
       }
       /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
       lRet = 	GetOnuOperStatus( PonPortIdx, OnuIdx);
       if ( CLI_EPON_ONUDOWN == lRet)
      {
   	   vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
      }
	   if(VOS_OK != WhetherSupportDrPengAttri(PonPortIdx, OnuIdx,DRPENG_ONU_LOOP_DETECTION_TIME) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
	}      
	downtime = ( unsigned short) VOS_AtoL( argv[ 0 ] );
	restart_times = ( unsigned short) VOS_AtoL( argv[ 1 ] );

      if(VOS_OK != OnuMgt_DrPengSetOnuLoopDetectionTime(PonPortIdx, OnuIdx, downtime,restart_times))
  		vty_out(vty, "Failed to set onu loop detection time\r\n");

	return CMD_SUCCESS;
}
DEFUN  (
    DrPeng_onu_get_loop_detec,
    DrPeng_onu_get_loop_detec_cmd,
    "show loop-detection",
    "show onu loop-detection time\n"
    "show onu loop-detection time\n"
    )
{
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	LONG lRet = VOS_OK;
	unsigned short downtime,restart_times;
	
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);
	
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
       if(ThisIsValidOnu(PonPortIdx, OnuIdx) != VOS_OK )
       {
	   vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
	   return CMD_WARNING;	
       }
       /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
       lRet = 	GetOnuOperStatus( PonPortIdx, OnuIdx);
       if ( CLI_EPON_ONUDOWN == lRet)
      {
   	   vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
      }
	 if(VOS_OK != WhetherSupportDrPengAttri(PonPortIdx, OnuIdx,DRPENG_ONU_LOOP_DETECTION_TIME) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
	}        
        if(VOS_OK == OnuMgt_DrPengGetOnuLoopDetectionTime(PonPortIdx,OnuIdx ,&downtime,&restart_times))
        {
		vty_out(vty, "loop-detection Downtime:%d restart-times:%d\r\n",downtime,restart_times); 
        }
     else
     	vty_out(vty, "Failed to get onu information\r\n");   

    return CMD_SUCCESS;

}
DEFUN  (
	DrPeng_onu_set_port_save,
	DrPeng_onu_set_port_save_cmd,
	"port <port_list> [save-config|restore-factory-config]",
	"save onu port config or restore factory config \n"
	"Please input port id\n"
	"save onu port config\n"
	 "restore factory config\n"
	)
{
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG	ulIfIndex = 0;
	ULONG	ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	short int error_flag = 0;
	unsigned short port_id;
	unsigned char action = 0;
	LONG lRet = VOS_OK;	
	
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);
	
	 if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
	 
	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
       if(ThisIsValidOnu(PonPortIdx, OnuIdx) != VOS_OK )
       {
	   vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
	   return CMD_WARNING;	
       }
       /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
       lRet = 	GetOnuOperStatus( PonPortIdx, OnuIdx);
       if ( CLI_EPON_ONUDOWN == lRet)
      {
   	   vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
      }
	if(VOS_OK != WhetherSupportDrPengAttri(PonPortIdx, OnuIdx,DRPENG_ONU_SAVE_CONFIG) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
	}         
	if ( 0 == VOS_StrnCmp( "save", argv[1],4) )
			action = 0x01;
	else if(0 == VOS_StrnCmp( "restore", argv[1],4))
	{
			action = 0x02;
	}
	else
		return CMD_ERR_NO_MATCH;

	 BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port_id)
        {
              if(VOS_OK != OnuMgt_DrPengSetOnuPortSaveConfig(PonPortIdx, OnuIdx, port_id,action ))
	      		vty_out(vty, "Failed to set onu port %d config\r\n",port_id);
        }
       END_PARSE_PORT_LIST_TO_PORT();

	return CMD_SUCCESS;
}


DEFUN  (
    DrPeng_onu_device_name_config,
    DrPeng_onu_device_name_config_cmd,
    "device name-peng {<name>}*1",
    "Config onu device info\n"
    "Config onu name \n"
    "Please input the device name( no more than 255 characters),\n"
    )
{

    LONG lRet = VOS_OK;	
    INT32 len = 0;	
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    ULONG ulIfIndex = 0;	
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
    unsigned char array[PENGMAXDEVICENAMELEN];

    VOS_MemSet(array,0,sizeof(array));
    array[PENGMAXDEVICENAMELEN-1] = '\0';
    len = strlen(argv[0]);	
    if( len > PENGMAXDEVICENAMELEN )
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
     if(VOS_OK != WhetherSupportDrPengAttri(phyPonId, userOnuId,DRPENG_ONU_DEVICE_NAME) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",slotId,  port, onuId) ;
		return CMD_WARNING;
	}     
     if(argc == 1)
     {
   	     lRet = OnuMgt_DrPengSetOnuDeviceName( phyPonId, userOnuId,  argv[0],  len);
	    if (lRet != VOS_OK)
	    {
	    	vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
	    }
	    return CMD_SUCCESS;
      }
     else 
     {
     	 lRet = OnuMgt_DrPengGetOnuDeviceName( phyPonId, userOnuId,  array);
	    if (lRet == VOS_OK)
	    {
	    	vty_out( vty, "device name:%s \r\n",array );
		return  CMD_SUCCESS;
	    }
	    return CMD_WARNING;
	}
}
DEFUN  (
    DrPeng_onu_location_name_config,
    DrPeng_onu_location_name_config_cmd,
    "device location-peng {<location>}*1",
    "Config onu device info\n"
    "Config onu location \n"
    "Please input the device location( no more than 255 characters),\n"
    )
{

    LONG lRet = VOS_OK;	
    INT32 len = 0;	
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    ULONG ulIfIndex = 0;	
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
    unsigned char array[PENGMAXDEVICENAMELEN];

    VOS_MemSet(array,0,sizeof(array));
    array[PENGMAXDEVICENAMELEN-1] = '\0';
    len = strlen(argv[0]);	
    if( len > PENGMAXDEVICENAMELEN )
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
      if(VOS_OK != WhetherSupportDrPengAttri(phyPonId, userOnuId,DRPENG_ONU_DEVICE_LOCATION) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",slotId,  port, onuId) ;
		return CMD_WARNING;
	}     
     if(argc == 1)
     {
   	     lRet = OnuMgt_DrPengSetOnuDeviceLocation( phyPonId, userOnuId,  argv[0],  len);
	    if (lRet != VOS_OK)
	    {
	    	vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
	    }
	    return CMD_SUCCESS;
      }
     else 
     {
     	 lRet = OnuMgt_DrPengGetOnuDeviceLocation( phyPonId, userOnuId,  array);
	    if (lRet == VOS_OK)
	    {
	    	vty_out( vty, "device location:%s \r\n",array );
		return  CMD_SUCCESS;
	    }
	    return CMD_WARNING;
	}
}
DEFUN  (
    DrPeng_onu_description_name_config,
    DrPeng_onu_description_name_config_cmd,
    "device description-peng {<description>}*1",
    "Config onu device info\n"
    "Config onu description \n"
    "Please input the device description( no more than 255 characters),\n"
    )
{

    LONG lRet = VOS_OK;	
    INT32 len = 0;	
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    ULONG ulIfIndex = 0;	
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
    unsigned char array[PENGMAXDEVICENAMELEN];

    VOS_MemSet(array,0,sizeof(array));
    array[PENGMAXDEVICENAMELEN-1] = '\0';
    len = strlen(argv[0]);	
    if( len > PENGMAXDEVICENAMELEN )
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
	if(VOS_OK != WhetherSupportDrPengAttri(phyPonId, userOnuId,DRPENG_ONU_DEVICE_DESCRIPTION) )
	{
		vty_out( vty, "  %% onu%d/%d/%d does not support Dr.Peng standards.\r\n",slotId,  port, onuId) ;
		return CMD_WARNING;
	}      
     if(argc == 1)
     {
   	     lRet = OnuMgt_DrPengSetOnuDeviceDescription( phyPonId, userOnuId,  argv[0],  len);
	    if (lRet != VOS_OK)
	    {
	    	vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
	    }
	    return CMD_SUCCESS;
      }
     else 
     {
     	 lRet = OnuMgt_DrPengGetOnuDeviceDescription( phyPonId, userOnuId,  array);
	    if (lRet == VOS_OK)
	    {
	    	vty_out( vty, "device description:%s \r\n",array );
		return  CMD_SUCCESS;
	    }
	    return CMD_WARNING;
	}
}
#endif

#ifdef  _DAYA_ONU_CLI_
#endif
/*********************大亚命令行****************************/
/*Igmpsnooping*/
DEFUN  (
    onu_show_igmpsnoop_version,
    onu_show_igmpsnoop_version_cmd,
    "igmpsnooping ver",    
    "Show or config onu igmpsnooping information\n"
    "Show onu igmpsnooping version information\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "igmpsnooping ver", NULL );
#elif defined __RPC_ONU_COMMAND
	return rpc_onu_dev_cli_process( vty, "igmpsnooping ver", NULL );
#else
	LONG lRet;
	CHAR pBuff[512]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;

	
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}

     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam( ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%igmpsnooping ver failed!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}


DEFUN  (
    onu_port_igmpsnoop_enable,
    onu_port_igmpsnoop_enable_cmd,
    /*"igmpsnooping enable [<1-24>|all] {[0|1]}*1",    */
    "igmpsnooping enable {[0|1]}*1",
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping enable information\n"
/*  "Input the port number\n"
    "All ports\n"*/
    "Snooping disable\n"
    "Snooping enable\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "igmpsnooping enable", NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0,suffix;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

    /*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {

            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK != OnuMgt_SetIgmpEnable(ponID, ulOnuid-1, VOS_StrToUL(argv[0], NULL, 10)) )
            {
#if 0                
                vty_out(vty, "igmpsnooping %s fail!\r\n", VOS_StrToUL(argv[0], NULL, 10)?"enable":"disable");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                int en = 0;

                /*if(getOnuConfSimpleVar(ponID, ulOnuid-1, sv_enum_igmp_enable, &en) == VOS_OK)*/
                if(OnuMgt_GetMulticastSwitch(ponID, ulOnuid-1, &en) == VOS_OK)
                {
                    if(en == CTC_STACK_PROTOCOL_IGMP_MLD_SNOOPING)
                        vty_out(vty, "igmp-snooping has started on all ports\r\n");
                    else
                        vty_out(vty, "igmp-snooping is not started or starting\r\n");
                }
                else
                {
#if 0                    
                    vty_out(vty,"igmp-snooping enable get fail!\r\n");
#else
                    vty_out(vty, ONU_CMD_ERROR_STR);
#endif
                }
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
	                vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG en = 0;
        int port = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

        if(argc == 1)
        {
            en = VOS_StrToUL(argv[0], NULL, 10);
            if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_igmp_enable, en))
                vty_out(vty, "igmpsnooping %s fail!\r\n", en?"enable":"disable");
            if(en)/*当igmp 使能时，igmp fastleave 使能随之变为enable ; added by luh 2011-10-19问题单13440*/
            {
                for(port=1;port<=ONU_MAX_PORT;port++)
                    setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_igmp_fastleave_enable, en);
            }       
        }
        else
        {
            if(getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_igmp_enable, &en))
                vty_out(vty, "igmpsnooping enable get fail!\r\n");
            else
                vty_out(vty, "igmpsnooping %s \r\n", en?"enable":"disable");
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid -1);*/
	
	ponID = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
/*	modify by wangxy 2007-03-12
	if ( 0 != VOS_StrCmp( argv[ 0 ], "all" ) )
    {
        ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
		if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
			return CMD_WARNING;		
    }

    */
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%igmpsnooping enable FAILED!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;    
#endif
}


/*note at 04 October ，daya要求删除该命令，因为配置该命令会影响
igmpsnooping enable的状态*/
DEFUN  (
    onu_igmpsnoop_channel,
    onu_igmpsnoop_channel_cmd,
    "igmpsnooping channel {<1-16>}*1",    
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping channel information\n"
    "Please input the max_channel number(1-16)\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "igmpsnooping channel", NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam( ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%igmpsnooping channel FAILED!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}

/*组播使用mac地址，为静态mac地址，则该老化时间同atu时间无作用，注释
modified by wutw at 22 October*/
DEFUN  (
    onu_igmpsnoop_aging,
    onu_igmpsnoop_aging_cmd,
    "igmpsnooping aging {[0|<1-3825>]}*1",    
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping aging time information\n"
    "Never aging\n"
    "Please input the aging time\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "igmpsnooping aging", NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;
    ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
    if( ulRet !=VOS_OK )
        return CMD_WARNING;

    /*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/

    ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
    if(ponID == (VOS_ERROR))
    {
        vty_out( vty, "  %% Parameter error\r\n");
        return CMD_WARNING;
    }

    if(argc == 1)
    {

        ONU_CONF_WRITEABLE_CHECK

        if(VOS_OK == OnuMgt_SetIgmpHostAge(ponID, ulOnuid-1, VOS_StrToUL(argv[0], NULL, 10)) )
        {
            vty_out(vty, "execute ok!\n");
        }
        else
        {
            vty_out(vty, "execute error!\n");
        }
    }
    else
    {
        char *pRecv;
        USHORT len;

        if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
        {
            vty_big_out(vty, len, "%s", pRecv);
        }
        else
        {
            vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam( ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%igmpsnooping aging FAILED!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}

static int rpc_onu_fe_cli_process( struct vty *vty, char *fe_argv, char *hint_str )
{
    ULONG ulFePort = 0;
    LONG lRet;
    CHAR pBuff[256];
    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    USHORT length;
    ULONG ulRet;

    cliPayload *stPayload=NULL;

    if( check_onu_cli_is_deny(vty) )    /* modified by xieshl 20100521, 问题单10260 */
        return CMD_SUCCESS;

    vty->length = VOS_StrLen(vty->buf);
    if( vty->length >= 256 )
        return (CMD_WARNING );
    if( vty->length == 0 )
        return (CMD_SUCCESS );

    ulIfIndex =(ULONG) vty->index;
    ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
    if( ulRet !=VOS_OK )
        return CMD_WARNING;

    ponID = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort);
    if(ponID == (VOS_ERROR))
    {
        vty_out( vty, "  %% Parameter error\r\n");
        return CMD_WARNING;
    }
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/
    lRet =  GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
    {
       vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
        return CMD_WARNING;
    }

    /*added by wutw 2006/11/10*/
    if( fe_argv )
    {
        BEGIN_PARSE_PORT_LIST_TO_PORT( fe_argv, ulFePort )
        {
            if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulFePort,vty))
            {
                RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
            }
        }
        END_PARSE_PORT_LIST_TO_PORT();
    }

    VOS_MemCpy( pBuff, vty->buf, vty->length );
    length = vty->length;

    if(OnuMgt_CliCall(ponID, ulOnuid-1, pBuff, length, &stPayload, &length) == VOS_OK && length)
        vty_big_out(vty, length, "%s", stPayload);
    else
        vty_out(vty, "  %% %s error!\r\n", hint_str );

    return CMD_SUCCESS;
}

static int onu_fe_cli_process( struct vty *vty, char *fe_argv, char *hint_str )
{
	ULONG ulFePort = 0;
	LONG lRet;
	CHAR pBuff[256];   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;	
	
	cliPayload *stPayload=NULL;

	if( check_onu_cli_is_deny(vty) )	/* modified by xieshl 20100521, 问题单10260 */
		return CMD_SUCCESS;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING; 
	
	ponID = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
    {
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
    }
	
	/*added by wutw 2006/11/10*/
	if( fe_argv )
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( fe_argv, ulFePort )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulFePort,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	VOS_MemCpy( pBuff, vty->buf, vty->length );
	length = vty->length;
	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %% %s error!\r\n", hint_str );	

	return CMD_SUCCESS;
}


/*added by luh for igmpsnooping_gda add&&show at July 26 2011*/

/*modified by wutw at October 16*/
DEFUN  (
    onu_igmpsnoop_gda_add,
    onu_igmpsnoop_gda_add_cmd,
    "igmpsnooping gda_add <A.B.C.D> <1-4094> <port_list>",    
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping gda_add information\n"
    "Please input the Group Ip Address (224.0.1.0~238.255.255.255)\n"
    "Please input the vlan id in which the multicast frames are forward\n"
    "Please input the port\n"
    /*"Please input the port. e.g.1,3-4\n"*/
    ) 
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[2], "igmpsnooping gda_add" );
#elif defined __RPC_ONU_COMMAND
#if 0
	return rpc_onu_fe_cli_process( vty, argv[2], "igmpsnooping gda_add" );
#endif
    int ponID=0;
    ULONG   ulIfIndex = 0, port, suffix;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
    ULONG portlist_temp = 0;
    ULONG all = 0, untag = 0;
    int flag = 0;
    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[2], port)

        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {
    		if(argc == 3)
		    {
    			ULONG addr = 0; 
                ULONG vid = VOS_StrToUL(argv[1], NULL, 10);
    			ULONG portlist = 0;
                ULONG addret = 0;
                int ret = 0;
                addr = inet_atonl_v4(argv[0]);
                addret = addr>>24;
                if(addret<224||addret>238)
                {
        			vty_out(vty, "igmp group address is illegal!\r\n");
                    return CMD_WARNING;
                }
                ONU_CONF_WRITEABLE_CHECK
                
                BEGIN_PARSE_PORT_LIST_TO_PORT( argv[2], port )
                {
                    portlist |= 1<<(port-1);
                }
                END_PARSE_PORT_LIST_TO_PORT();
                if(get_onuconf_vlanPortlist(ponID, ulOnuid-1, vid, &all, &untag) != VOS_OK)/*添加对vlan 和端口的检查added by luh 2011-10-20*/
                {
                    vty_out(vty, "vlan %d doesn't exist!\r\n", vid);
                    return CMD_WARNING;
                }
                else
                {
                    portlist_temp = portlist&(~all);
                    while(portlist_temp)
                    {
                        if(portlist_temp&1)
                        {
                            vty_out(vty, "Port 1/%d is not the member of vlan %d.\r\n", ret+1, vid);
                            vty_out(vty, "Please add port 1/%d to vlan %d first.\r\n", ret+1, vid); 
                            flag = 1;
                        }
                        portlist_temp >>= 1;
                        ret++;
                    }
                    if(flag)
                        return CMD_WARNING;
                }
                if(VOS_OK != OnuMgt_AddIgmpGroup(ponID, ulOnuid-1, portlist,addr,vid) )
        		{
#if 0                    
        			vty_out(vty, "igmp group set error!\r\n");
#else
                    vty_out(vty, ONU_CMD_ERROR_STR);
#endif
        		}
    		}
    		else
    		{
               vty_out( vty, "  %% Parameter error\r\n");
    		}


        }
        else
        {
            char *pRecv;
            USHORT len;

            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
	            vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }
    else
    {
        ULONG addr1 = 0;
        int ret = 0, check_flag = 0;
        ULONG enable = 0; 
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        ULONG addr = inet_atonl_v4(argv[0]);
        ULONG addret = addr>>24;
        USHORT vid = VOS_StrToUL(argv[1],NULL,10);
        ULONG portlist = 0; 
        
        VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[2], port)
        if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_igmp_enable, &enable) == VOS_OK && enable == 0)
        {
            vty_out(vty, "IGMP snooping module does not enable.\r\n");
            return CMD_WARNING;
        }
        if(addret<224||addret>238)
        {
            vty_out(vty, "igmp group address is illegal!\r\n");
            return CMD_WARNING;
        }
        if(argc == 3)
        {            
            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[2], port )
            {
               portlist |= 1<<(port-1);
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(get_onuconf_vlanPortlistByPtr(suffix, pd, vid, &all, &untag) != VOS_OK)/*添加对vlan 和端口的检查added by luh 2011-10-20*/
            {
                vty_out(vty, "vlan %d doesn't exist!\r\n", vid);
                return CMD_WARNING;
            }
            else
            {
                portlist_temp = portlist&(~all);
                while(portlist_temp)
                {
                    if(portlist_temp&1)
                    {
                        vty_out(vty, "Port 1/%d is not the member of vlan %d.\r\n", ret+1, vid);
                        vty_out(vty, "Please add port 1/%d to vlan %d first.\r\n", ret+1, vid);  
                        check_flag = 1;
                    }
                    portlist_temp >>= 1;
                    ret++;
                }
                if(check_flag)
                    return CMD_WARNING;
            }
            for(ret=0;ret<ONU_MAX_IGMP_GROUP;ret++)
            {
               if(getOnuConf_igmp_groupdataByPtr(suffix, pd,ret,sv_enum_igmp_group_gda,&addr1)==VOS_OK)
               {
                  if(addr == addr1)
                  {
                    flag = 1;
                    if(SetOnuConf_igmp_groupdataByPtr(suffix, pd,ret,sv_enum_igmp_group_portmask,portlist)||
                        SetOnuConf_igmp_groupdataByPtr(suffix, pd,ret,sv_enum_igmp_group_vlanId,vid))
                    {
                        vty_out(vty, "igmp group set error!\r\n");
                    }
                  }
               }
            }
            if(!flag)
            {
                for(ret=0;ret<ONU_MAX_IGMP_GROUP;ret++)
                {
                    if(getOnuConf_igmp_groupdataByPtr(suffix, pd,ret,sv_enum_igmp_group_gda,&addr1)==VOS_OK)
                    {
                        if(0 == addr1)
                        {
                            if(SetOnuConf_igmp_groupdataByPtr(suffix, pd,ret,sv_enum_igmp_group_gda,addr)||
                                SetOnuConf_igmp_groupdataByPtr(suffix, pd,ret,sv_enum_igmp_group_portmask,portlist)||
                                SetOnuConf_igmp_groupdataByPtr(suffix, pd,ret,sv_enum_igmp_group_vlanId,vid))
                            {
                                vty_out(vty, "igmp group set error!\r\n");
                            }
                            break;
                        }
                    }
                } 
            }
            
        }
        else
        {
            vty_out( vty, "  %% Parameter error\r\n");     
        }
        
    }
    return CMD_SUCCESS;
    
#else
	ULONG ulFePort = 0;
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;	
	
	cliPayload *stPayload=NULL;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING; 
	
	ponID = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
    {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
    }
	
	/*added by wutw 2006/11/10*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 2 ], ulFePort )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulFePort,vty))
		{
			RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();


	/*modified by wutw at 20 October*/
	/*VOS_Sprintf(pBuff,"atu static_add %s 1 %s",argv[0],argv[1]);*/
	
	VOS_MemCpy( pBuff, vty->buf, vty->length );
	length = vty->length;
	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		/*vty_out(vty, "  %%atu static_add !\r\n");	*/
		vty_out(vty, "  %%igmpsnooping gda_add error!\r\n");	


	return CMD_SUCCESS;
#endif
}




DEFUN  (
    onu_igmpsnoop_gda_del,
    onu_igmpsnoop_gda_del_cmd,
    "igmpsnooping gda_del <A.B.C.D>",    
    "Show or config onu igmpsnooping information\n"
    "Delete igmpsnooping gda\n"
    "Please input the Group Ip Address\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "igmpsnooping gda_del", NULL );
#elif defined __RPC_ONU_COMMAND
#if 0
	return rpc_onu_dev_cli_process( vty, "igmpsnooping gda_del", NULL );
#endif
    int ponID=0;
    ULONG   ulIfIndex = 0, suffix;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {
    		if(argc == 1)
		    {
    			ULONG addr = 0;
                ULONG addret = 0;
                addr = inet_atonl_v4(argv[0]);
                addret = addr>>24;
                if(addret<224||addret>238)
                {
        			vty_out(vty, "igmp group address is illegal!\r\n");
                       return CMD_WARNING;
                }
                ONU_CONF_WRITEABLE_CHECK

                if(VOS_OK != OnuMgt_DeleteIgmpGroup(ponID, ulOnuid-1,addr) )
        		{
#if 0                    
        			vty_out(vty, "igmp group delete error!\r\n");
#else
                    vty_out(vty, ONU_CMD_ERROR_STR);
#endif
        		}
    		}
    		else
    		{
                vty_out( vty, "  %% Parameter error\r\n");
    		}


        }
        else
        {
            char *pRecv;
            USHORT len;

            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
	            vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }
    else
    {
        ULONG addr1 = 0;
        int ret = 0,flag = 0;
        ULONG enable = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_igmp_enable, &enable) == VOS_OK && enable == 0)
        {
            vty_out(vty, "IGMP snooping module does not enable.\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {            
            ULONG addr = inet_atonl_v4(argv[0]);
            ULONG port = 0;
            USHORT vid = 0;
            for(ret=0;ret<ONU_MAX_IGMP_GROUP;ret++)
            {
               if(getOnuConf_igmp_groupdataByPtr(suffix, pd,ret,sv_enum_igmp_group_gda,&addr1)==VOS_OK)
               {
                  if(addr == addr1)
                  {
                    flag = 1;
                    addr1 = 0;
                    if(SetOnuConf_igmp_groupdataByPtr(suffix, pd,ret,sv_enum_igmp_group_gda,addr1)||
                        SetOnuConf_igmp_groupdataByPtr(suffix, pd,ret,sv_enum_igmp_group_portmask,port)||
                        SetOnuConf_igmp_groupdataByPtr(suffix, pd,ret,sv_enum_igmp_group_vlanId,vid))
                    {
                        vty_out(vty, "igmp group delete error!\r\n");
                    }
                  }
               }
            }
            if(!flag)
            {
                vty_out(vty, "igmp group is not exist!\r\n");
            }
            
        }
    }
    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;
	
	ponID = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*modified by wutw at 20 October*/
	/*VOS_Sprintf(pBuff,"atu static_del %s",argv[0]);
		length = strlen(pBuff);*/
	VOS_Sprintf( pBuff, vty->buf, vty->length );
	length = vty->length;

	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%igmpsnooping gda_del failed !\r\n");	


	return CMD_SUCCESS;
#endif
}


DEFUN  (
    onu_igmpsnoop_gda_show,
    onu_igmpsnoop_gda_show_cmd,
    "igmpsnooping gda_show",    
    "Show or config onu igmpsnooping information\n"
    "Show the igmpsnooping gda \n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "igmpsnooping gda_show", NULL );
#elif defined __RPC_ONU_COMMAND
#if 0
	return rpc_onu_dev_cli_process( vty, "igmpsnooping gda_show", NULL );
#endif
    int ponID=0,i=0;
    ULONG   ulIfIndex = 0, suffix;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {
            CTC_STACK_multicast_control_t uv;
            if(OnuMgt_GetMulticastControl(ponID, ulOnuid-1, &uv) != VOS_OK)
            {
                vty_out(vty, ONU_CMD_ERROR_STR);                
                return CMD_WARNING;
            }
            vty_out(vty, "\r\n onu%d/%d/%d multicast control entries: count:%d\r\n", ulSlot, ulPort, ulOnuid, uv.num_of_entries);	              
            if(uv.num_of_entries)
            {
                vty_out(vty, "%10s%5s%10s%5s%10s\r\n","Vlan"," ", "DA", " ", "Port");
                for(i=0;i<uv.num_of_entries;i++)
                {
                    vty_out(vty, "%d%9d%5s%02x%02x.%02x%02x.%02x%02x%8s%d\r\n",i, uv.entries[i].vid, " ",uv.entries[i].da[0],uv.entries[i].da[1],uv.entries[i].da[2],uv.entries[i].da[3],uv.entries[i].da[4],uv.entries[i].da[5]," ", uv.entries[i].user_id);
                }	
            }
        }
        else
        {
            char *pRecv;
            USHORT len;

            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
	            vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }
    else
    {
        ULONG addr = 0;
        ULONG portlist = 0;
        ULONG vid = 0;
        ULONG enable = 0;
        char portlist_str[80]="";
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

        if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_igmp_enable, &enable) == VOS_OK && enable == 0)
        {
            vty_out(vty, "IGMP snooping module does not enable.\r\n");
            return CMD_WARNING;
        }   

        vty_out(vty, "\r\n%10s%20s%20s\r\n","Vlan", "Group", "Member");
        vty_out(vty, "%40s\r\n","-------------------------------------------------------------");
    	for(i=0;i<ONU_MAX_IGMP_GROUP;i++)
    	{
            if(VOS_OK == getOnuConf_igmp_groupdataByPtr(suffix, pd, i, sv_enum_igmp_group_gda, &addr))
    		{
                if(addr)
                {
                    if((getOnuConf_igmp_groupdataByPtr(suffix, pd, i, sv_enum_igmp_group_vlanId, &vid) ==  VOS_OK)&&
                       (getOnuConf_igmp_groupdataByPtr(suffix, pd, i, sv_enum_igmp_group_portmask, &portlist) == VOS_OK))
    		        {
                        portListLongToString(portlist, portlist_str);
                        vty_out(vty, "%10d%13s%d.%d.%d.%d%12s%-10s\r\n", vid, " ", addr>>24,(addr>>16)&0xff,(addr>>8)&0xff,addr&0xff," ",portlist_str);
    		        }
                }
                else
                    continue;
            }
            else
                vty_out(vty, "\r\nigmp group information get error!\r\n");
    	}             
        vty_out(vty, "%40s\r\n","-------------------------------------------------------------");
    }
    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short  int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam( ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%igmpsnooping gda_show FAILED!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}

/*删除该命令，daya固定支持该功能，但无法对其进行配置
加以保留，但可以目前先不实现*/
DEFUN  (
    onu_igmpsnoop_last_member,
    onu_igmpsnoop_last_member_cmd,
    "igmpsnooping last_member {[0|1]}*1 ",    
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping last_member information\n"
    "Functionality disable\n"
    "Functionality enable\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "igmpsnooping last_member", NULL );
#else
 	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short  int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam( ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%igmpsnooping channel FAILED!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}


DEFUN  (
    onu_port_igmpsnoop_param,
    onu_port_igmpsnoop_param_cmd,
    "igmpsnooping param [qry_cnt|lm_q_cnt|lm_resp_t] {<value>}*1",    
    "Show or config onu igmpsnooping information\n"
    "Config igmpsnooping param\n"
    "Query timeout count\n"
    "Last member query count\n"
    "Last member response time\n"
    "Please input the value( qry_cnt:1-16; lm_q_cnt:1-16; lm_resp_t:10-255\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "igmpsnooping param", NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%igmpsnooping param FAILED!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}

DEFUN(
	onu_igmpsnooping_max_group,
	onu_igmpsnooping_max_group_cmd,
	/* modified by chenfj 2007-10-18
	 5571.GT812上设置ONU的最大组播组数量的参数不对
	 可选参数没有定义可重复次数，导致出现如问题单所述BUG
	 */
	"igmpsnooping max_group {<0-256>}*1",
	"Show or configure igmpsnooping information\n"
	"Show or configure maximum group ip number\n"
	"maximum group io number set (0~256)\n"
	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "igmpsnooping max_group", NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

    /*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {

            ULONG num;
            num = VOS_StrToUL(argv[0], NULL, 10);

            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK != OnuMgt_SetIgmpMaxGroup(ponID, ulOnuid-1, num) )
            {
#if 0                
                vty_out(vty, "igmpsnoop max group set error!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                ULONG num;


                    if(getOnuConfSimpleVar(ponID, ulOnuid-1, sv_enum_igmp_max_group, &num) == VOS_OK)
                        vty_out(vty, "igmpsnoop max group %d \r\n", num);
                    else
                        vty_out(vty, "igmpsnoop max group get fail!\r\n");

            }

            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG num;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        
        if(argc == 1)
        {
            num = VOS_StrToUL(argv[0], NULL, 10);
            if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_igmp_max_group, num) != VOS_OK)
                vty_out(vty, "igmpsnoop max group set error!\r\n");
        }
        else
        {
            if(getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_igmp_max_group, &num) == VOS_OK)
                vty_out(vty, "igmpsnoop max group %d \r\n", num);
            else
                vty_out(vty, "igmpsnoop max group get fail!\r\n");
        }

    }
    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	LONG num;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}

	if( argc == 1 )
	{
		num = VOS_AtoL( argv[0] );
		if( num >256 || num < 0 )
		{
			vty_out( vty, "invalid group ip number, pease input an integer between 0 and 256\r\n" );
			return CMD_WARNING;
		}
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%igmpsnooping max_group ip set ERROR!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}

DEFUN(
	onu_igmpsnooping_host_aging_time,
	onu_igmpsnooping_host_aging_time_cmd,
	"igmpsnooping host_aging_time {<20-3025>}*1",
	"Show or configure igmpsnooping information\n"
	"Show or configure host aging time\n"
	"host agingtime num (20~3025),unit: s\n"
	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "igmpsnooping host_aging_time", NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

    /*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {
            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK != OnuMgt_SetIgmpHostAge(ponID, ulOnuid-1, VOS_StrToUL(argv[0], NULL, 10)) )
            {
#if 0                
                vty_out(vty, "igmpsnooping host_aging_time set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {

                    ULONG age = 0;
                    if(VOS_OK == getOnuConfSimpleVar(ponID, ulOnuid-1,sv_enum_igmp_hostage, &age))
                        vty_out(vty, "\r\nigmpsnooping host_aging_time is %d\r\n",age);
                    else
                        vty_out(vty, "\r\nigmpsnooping host_aging_time get error!\r\n");

            }

            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG age = 0;
        ULONG maxresponse = 0;
        ULONG groupage = 0;
        ULONG enable = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_igmp_enable, &enable) == VOS_OK && enable == 0)
        {
            vty_out(vty, "IGMP snooping module does not enable.\r\n");
            return CMD_WARNING;
        }        
        if(argc == 1)
        {
            age = VOS_StrToUL(argv[0], NULL, 10);
            if(setOnuConfSimpleVarByPtr(suffix, pd,sv_enum_igmp_hostage, age))
                vty_out(vty, "igmpsnooping host_aging_time set fail!\r\n");
            if(getOnuConfSimpleVarByPtr(suffix, pd,sv_enum_igmp_groupage, &groupage) == VOS_OK &&
                getOnuConfSimpleVarByPtr(suffix, pd,sv_enum_igmp_max_response_time, &maxresponse) == VOS_OK)
            {
                groupage = age - maxresponse;
                setOnuConfSimpleVarByPtr(suffix, pd,sv_enum_igmp_groupage, groupage);
            }
        }
        else
        {
            if(getOnuConfSimpleVarByPtr(suffix, pd ,sv_enum_igmp_hostage, &age))
                vty_out(vty, "igmpsnooping host_aging_time get fail!\r\n");
            else
                vty_out(vty, "igmpsnooping host_aging_time %d\r\n",age);
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	LONG num;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}

	if( argc == 1 )
	{
		num = VOS_AtoL( argv[0] );
		if( num >3025 || num < 20 )
		{
			vty_out( vty, "invalid host aging time, pease input an integer between 20 and 3025\r\n" );
			return CMD_WARNING;
		}
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%igmpsnooping host agingtime set ERROR!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}

DEFUN(
	onu_igmpsnooping_group_aging_time,
	onu_igmpsnooping_group_aging_time_cmd,
	"igmpsnooping group_aging_time {<10-1000>}*1",
	"Show or configure igmpsnooping information\n"
	"Show or configure group aging time\n"
	"group agingtime num (10~1000),unit: s\n"
	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "igmpsnooping group_aging_time", NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

    /*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {

            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK != OnuMgt_SetIgmpGroupAge(ponID, ulOnuid-1, VOS_StrToUL(argv[0], NULL, 10)) )
            {
#if 0                
                vty_out(vty, "igmpsnooping group_aging_time set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                    ULONG age = 0;
                    {
                        if(VOS_OK == getOnuConfSimpleVar(ponID, ulOnuid-1,sv_enum_igmp_groupage, &age))
                            vty_out(vty, "\r\nigmpsnooping group_aging_time is %d\r\n",age);
                        else
                            vty_out(vty, "\r\nigmpsnooping group_aging_time get error!\r\n");
                    }
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG age = 0;
        ULONG hostage = 0;
        ULONG maxresponse = 0;
        ULONG enable = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_igmp_enable, &enable) == VOS_OK && enable == 0)
        {
            vty_out(vty, "IGMP snooping module does not enable.\r\n");
            return CMD_WARNING;
        }
        if(argc == 1)
        {
            age = VOS_StrToUL(argv[0], NULL, 10);

            if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_igmp_groupage, age))
                vty_out(vty, "igmpsnooping group_aging_time set fail!\r\n");
            if(getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_igmp_max_response_time, &maxresponse) == VOS_OK &&
               getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_igmp_hostage, &hostage) == VOS_OK)
            {
                hostage = age + maxresponse;
                setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_igmp_hostage, hostage);     
            }   
        }
        else
        {

            if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_igmp_groupage, &age))
                vty_out(vty, "igmpsnooping group_aging_time get fail!\r\n");
            else
                vty_out(vty, "igmpsnooping group_aging_time is %d\r\n",age);
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	LONG num;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}

	if( argc==1 )
	{
		num = VOS_AtoL( argv[0] );
		if( num >1000 || num < 10 )
		{
			vty_out( vty, "invalid group aging time, pease input an integer between 10 and 1000\r\n" );
			return CMD_WARNING;
		}
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%igmpsnooping host agingtime set ERROR!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}


DEFUN(
	onu_igmpsnooping_max_response_time,
	onu_igmpsnooping_max_response_time_cmd,
	"igmpsnooping max_response_time {<10-25>}*1",
	"Show or configure igmpsnooping information\n"
	"Show or configure maximum response time\n"
	"maximum response time (10~25),unit: s\n"
	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "igmpsnooping max_response_time", NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

    /*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {

            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK != OnuMgt_SetIgmpMaxResTime(ponID, ulOnuid-1, VOS_StrToUL(argv[0], NULL, 10)) )
            {
#if 0                
                vty_out(vty, "igmpsnooping max_response_time set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {

                    ULONG tm = 0;
                    if(VOS_OK == getOnuConfSimpleVar(ponID, ulOnuid-1,sv_enum_igmp_max_response_time, &tm))
                        vty_out(vty, "\r\nigmpsnooping max_response_time is %d\r\n",tm);
                    else
                        vty_out(vty, "\r\nigmpsnooping max_response_time get error!\r\n");

            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG tm = 0;
        ULONG groupage = 0, hostage = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(argc == 1)
        {
            tm = VOS_StrToUL(argv[0], NULL, 10);

            if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_igmp_max_response_time, tm))
            {
                vty_out(vty, "igmpsnooping max_response_time set fail!\r\n");
            }
            else
            {
                if(getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_igmp_groupage, &groupage) == VOS_OK)
                {
                    hostage = groupage + tm;
                    setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_igmp_hostage, hostage);
                }
            }
        }
        else
        {

            if(getOnuConfSimpleVarByPtr(suffix, pd ,sv_enum_igmp_max_response_time, &tm))
                vty_out(vty, "igmpsnooping max_response_time get fail!\r\n");
            else
                vty_out(vty, "igmpsnooping max_response_time is %d\r\n",tm);
        }
    }
    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	LONG num;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}

	if( argc == 1 )
	{
		num = VOS_AtoL( argv[0] );
		if( num >1000 || num < 10 )
		{
			vty_out( vty, "invalid maximum response time, pease input an integer between 10 and 25\r\n" );
			return CMD_WARNING;
		}
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%igmpsnooping host agingtime set ERROR!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;	
#endif
}

DEFUN  (
    onu_port_igmpsnoop_fast_leave,
    onu_port_igmpsnoop_fast_leave_cmd,
    /*"igmpsnooping enable [<1-24>|all] {[0|1]}*1",    */
    "igmpsnooping fast_leave [<port_list>|all] {[0|1]}*1",
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping fastleave enable information\n"
    "Input the port number\n"
    "All ports\n"
    "Snooping disable\n"
    "Snooping enable\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "igmpsnooping fast_leave" );
#elif defined __RPC_ONU_COMMAND
#if 0
	return rpc_onu_fe_cli_process( vty, argv[0], "igmpsnooping fast_leave" );
#endif
    int ponID=0,i=0;
    ULONG   ulIfIndex = 0, port, suffix;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
    ULONG mode = 0;
    
    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;        
		ULONG portnum = 0;
		ULONG devidx = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;
		
		devidx = MAKEDEVID(ulSlot, ulPort, ulOnuid);
		if(getDeviceCapEthPortNum(devidx, &portnum) != VOS_OK)
		{
			vty_out(vty,"unknown onu port num!\r\n");
			return CMD_WARNING;
		}	

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 2)
        {
            
            mode = VOS_StrToUL(argv[1], NULL, 10);
            ONU_CONF_WRITEABLE_CHECK
            if (VOS_StriCmp(argv[0], "all") == 0)
            {

                for (i = 1; i <= portnum; i++)
                {
                    if(VOS_OK != OnuMgt_SetPortIgmpFastLeave(ponID, ulOnuid-1, i, mode) )
                    {
                        
#if 0                        
                        vty_out(vty, "port mode set error!\r\n");
#else
                        error_flag = 1;
#endif
                    }
                }
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);
                    
            }
            else
            {
                ULONG portlist = 0;
                ULONG portlist_cmd = 0;
                int cmd_len = 0;
                VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

                for(i=0;i<portnum;i++)
                    portlist |= (1<<i);

                BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
                {
                    portlist_cmd |= (1<<(port-1)); 
                }
                END_PARSE_PORT_LIST_TO_PORT()
                
                if(portlist == portlist_cmd)
                {
                    if(VOS_OK != OnuMgt_SetPortIgmpFastLeave(ponID, ulOnuid-1, port, mode) )
                    {
#if 0                        
                        vty_out(vty, "port mode set error!\r\n");
#else
                        vty_out(vty, ONU_CMD_ERROR_STR);
#endif
                    }
                }
                else
                {
                    BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
                    {    
                        if(VOS_OK != OnuMgt_SetPortIgmpFastLeave(ponID, ulOnuid-1, port, mode) )
                        {
#if 0                            
                            vty_out(vty, "port mode set error!\r\n");
#else
                            error_flag = 1;
#endif
                        }
                    }
                    END_PARSE_PORT_LIST_TO_PORT();
                    if(error_flag)
                        vty_out(vty, ONU_CMD_ERROR_STR);
                }
            }

        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                    if (VOS_StriCmp(argv[0], "all") == 0)
                    {

						ULONG portnum = 0;
						ULONG devidx = MAKEDEVID(ulSlot, ulPort, ulOnuid);
						if(getDeviceCapEthPortNum(devidx, &portnum) != VOS_OK)
						{
							vty_out(vty,"unknown onu port num!\r\n");
							return CMD_WARNING;
						}	
						
                        for (i = 1; i <= portnum; i++)
                        {
                            if(VOS_OK == getOnuConfPortSimpleVar(ponID, ulOnuid-1, i, sv_enum_port_igmp_fastleave_enable, &mode))
                                vty_out(vty, "\r\nport %d igmpsnooping fastleave is %s\r\n", i, mode?"enable":"disable");
                            else
                                vty_out(vty, "\r\nport igmpsnooping fastleave get error!\r\n");
                        }
                    }
                    else
                    {

                        VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

                        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                        {
                            if(VOS_OK == getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_igmp_fastleave_enable, &mode))
                                vty_out(vty, "\r\nport %d igmpsnooping fastleave is %s\r\n", port, mode?"enable":"disable");
                            else
                                vty_out(vty, "\r\nport igmpsnooping fastleave get error!\r\n");
                        }
                        END_PARSE_PORT_LIST_TO_PORT()
                    }

            }
            else
            {
                char *pRecv;
                USHORT len;
	            if (VOS_StriCmp(argv[0], "all") == 0)
	            {
	                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
	                {
			            vty_big_out(vty, len, "%s", pRecv);
	                }
	                else
	                {
	                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
	                }
	            }
	            else
	            {
	                ULONG portlist = 0;
	                ULONG portlist_cmd = 0;
	                char igmp_str[40];
	                int cmd_len = 0;
	                VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

	                for(i=0;i<portnum;i++)
	                    portlist |= (1<<i);

	                BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
	                {
	                    portlist_cmd |= (1<<(port-1)); 
	                }
	                END_PARSE_PORT_LIST_TO_PORT()
	                
	                if(portlist == portlist_cmd)
	                {
	                    VOS_MemZero(igmp_str, sizeof(char)*40);                    
	                    cmd_len = VOS_Snprintf(igmp_str, 40, "igmpsnooping fast_leave all\r\n");
		                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, igmp_str, cmd_len, &pRecv, &len) && pRecv && len)
		                {
				            vty_big_out(vty, len, "%s", pRecv);
		                }
		                else
		                {
		                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
		                }
	                }
	                else
	                {
	                    BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
	                    {   
	                        VOS_MemZero(igmp_str, sizeof(char)*40);                                            
	                        cmd_len = VOS_Snprintf(igmp_str, 40, "igmpsnooping fast_leave %d\r\n", port);
			                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, igmp_str, cmd_len, &pRecv, &len) && pRecv && len)
			                {
					            vty_big_out(vty, len, "%s", pRecv);
			                }
			                else
			                {
			                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
			                }
	                    }
	                    END_PARSE_PORT_LIST_TO_PORT();
	                }
	            }

            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        ULONG enable= 0;
        if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_igmp_enable, &enable) == VOS_OK && enable == 0)
        {
            vty_out(vty, "IGMP snooping module does not enable.\r\n");
            return CMD_WARNING;
        }        
        if(argc == 2)
        {
            mode = VOS_StrToUL(argv[1], NULL, 10);
            if (VOS_StriCmp(argv[0], "all") == 0)
            {		
                for (i = 1; i <= ONU_MAX_PORT; i++)
                {
    				if(VOS_OK != setOnuConfPortSimpleVarByPtr(suffix, pd, i, sv_enum_port_igmp_fastleave_enable, mode) )
    				{
    				    vty_out(vty, "port %d igmpsnooping fastleave set fail!\r\n", port);
    				}
                  }
            }
            else
            {

                VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

                if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_igmp_fastleave_enable, mode))
                    vty_out(vty, "port %d igmpsnooping fastleave set fail!\r\n", port);

                END_PARSE_PORT_LIST_TO_PORT()

            }
        }
        else
        {
            if (VOS_StriCmp(argv[0], "all") == 0)
            {
                for (port = 1; port <= ONU_MAX_PORT; port++)
                {
                    if(getOnuConfPortSimpleVarByPtr(suffix, pd , port, sv_enum_port_igmp_fastleave_enable, &mode))
                        vty_out(vty, "port %d igmpsnooping fastleave get fail!\r\n", port);
                    else
        		        vty_out(vty, "\r\nport %d igmpsnooping fastleave is %s\r\n", port, mode?"enable":"disable");
                }
            }
            else
            {

                VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

                if(getOnuConfPortSimpleVarByPtr(suffix, pd , port, sv_enum_port_igmp_fastleave_enable, &mode))
                    vty_out(vty, "port %d igmpsnooping fastleave get fail!\r\n", port);
                else
    		        vty_out(vty, "\r\nport %d igmpsnooping fastleave is %s\r\n", port, mode?"enable":"disable");

                END_PARSE_PORT_LIST_TO_PORT()
            }
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid,ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid -1);*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*modify by wangxy 2007-03-12*/
	if ( 0 != VOS_StrCmp( argv[ 0 ], "all" ) )
    {
    		/* modified by chenfj 2007-10-30
    			将命令中的端口参数改为 port_list形式；参数范围的判断也做相应改动
    			下同。
    		*/
    		/*
        	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
		if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
			return CMD_WARNING;		
    		*/
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}
    
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%igmpsnooping fastleave enable FAILED!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}


/*Igmpsnooping*/
LDEFUN  (
    onu_show_igmpsnoop_version,
    onu_show_igmpsnoop_version_cmd,
    "igmpsnooping ver",    
    "Show or config onu igmpsnooping information\n"
    "Show onu igmpsnooping version information\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


LDEFUN  (
    onu_port_igmpsnoop_enable,
    onu_port_igmpsnoop_enable_cmd,
    /*"igmpsnooping enable [<1-24>|all] {[0|1]}*1",    */
    "igmpsnooping enable {[0|1]}*1",
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping enable information\n"
/*  "Input the port number\n"
    "All ports\n"*/
    "Snooping disable\n"
    "Snooping enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


/*note at 04 October ，daya要求删除该命令，因为配置该命令会影响
igmpsnooping enable的状态*/
LDEFUN  (
    onu_igmpsnoop_channel,
    onu_igmpsnoop_channel_cmd,
    "igmpsnooping channel {<1-16>}*1",    
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping channel information\n"
    "Please input the max_channel number(1-16)\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

/*组播使用mac地址，为静态mac地址，则该老化时间同atu时间无作用，注释
modified by wutw at 22 October*/
LDEFUN  (
    onu_igmpsnoop_aging,
    onu_igmpsnoop_aging_cmd,
    "igmpsnooping aging {[0|<1-3825>]}*1",    
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping aging time information\n"
    "Never aging\n"
    "Please input the aging time\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


/*modified by wutw at October 16*/
LDEFUN  (
    onu_igmpsnoop_gda_add,
    onu_igmpsnoop_gda_add_cmd,
    "igmpsnooping gda_add <A.B.C.D> <1-4094> <port_list>",    
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping gda_add information\n"
    "Please input the Group Ip Address (224.0.1.0~238.255.255.255)\n"
    "Please input the vlan id in which the multicast frames are forward"
    "Please input the port\n",
    ONU_NODE
    ) 
{
	return CMD_SUCCESS;
}




LDEFUN  (
    onu_igmpsnoop_gda_del,
    onu_igmpsnoop_gda_del_cmd,
    "igmpsnooping gda_del <A.B.C.D>",    
    "Show or config onu igmpsnooping information\n"
    "Delete igmpsnooping gda\n"
    "Please input the Group Ip Address\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


LDEFUN  (
    onu_igmpsnoop_gda_show,
    onu_igmpsnoop_gda_show_cmd,
    "igmpsnooping gda_show",    
    "Show or config onu igmpsnooping information\n"
    "Show the igmpsnooping gda \n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

/*删除该命令，daya固定支持该功能，但无法对其进行配置
加以保留，但可以目前先不实现*/
LDEFUN  (
    onu_igmpsnoop_last_member,
    onu_igmpsnoop_last_member_cmd,
    "igmpsnooping last_member {[0|1]}*1 ",    
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping last_member information\n"
    "Functionality disable\n"
    "Functionality enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


LDEFUN  (
    onu_port_igmpsnoop_param,
    onu_port_igmpsnoop_param_cmd,
    "igmpsnooping param [qry_cnt|lm_q_cnt|lm_resp_t] {<value>}*1",    
    "Show or config onu igmpsnooping information\n"
    "Config igmpsnooping param\n"
    "Query timeout count\n"
    "Last member query count\n"
    "Last member response time\n"
    "Please input the value( qry_cnt:1-16; lm_q_cnt:1-16; lm_resp_t:10-255\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN(
	onu_igmpsnooping_max_group,
	onu_igmpsnooping_max_group_cmd,
	/* modified by chenfj 2007-10-18
	 5571.GT812上设置ONU的最大组播组数量的参数不对
	 可选参数没有定义可重复次数，导致出现如问题单所述BUG
	 */
	"igmpsnooping max_group {<0-256>}*1",
	"Show or configure igmpsnooping information\n"
	"Show or configure maximum group ip number\n"
	"maximum group io number set (0~256)\n",
       ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(
	onu_igmpsnooping_host_aging_time,
	onu_igmpsnooping_host_aging_time_cmd,
	"igmpsnooping host_aging_time {<20-3025>}*1",
	"Show or configure igmpsnooping information\n"
	"Show or configure host aging time\n"
	"host agingtime num (20~3025),unit: s\n",
       ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(
	onu_igmpsnooping_group_aging_time,
	onu_igmpsnooping_group_aging_time_cmd,
	"igmpsnooping group_aging_time {<10-1000>}*1",
	"Show or configure igmpsnooping information\n"
	"Show or configure group aging time\n"
	"group agingtime num (10~1000),unit: s\n",
       ONU_NODE
	)
{
	return CMD_SUCCESS;
}


LDEFUN(
	onu_igmpsnooping_max_response_time,
	onu_igmpsnooping_max_response_time_cmd,
	"igmpsnooping max_response_time {<10-25>}*1",
	"Show or configure igmpsnooping information\n"
	"Show or configure maximum response time\n"
	"maximum response time (10~25),unit: s\n",
    ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_port_igmpsnoop_fast_leave,
    onu_port_igmpsnoop_fast_leave_cmd,
    /*"igmpsnooping enable [<1-24>|all] {[0|1]}*1",    */
    "igmpsnooping fast_leave [<port_list>|all] {[0|1]}*1",
    "Show or config onu igmpsnooping information\n"
    "Show or config onu igmpsnooping fastleave enable information\n"
    "Input the port number\n"
    "All ports\n"
    "Snooping disable\n"
    "Snooping enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


/*vlan*/
DEFUN  (
    onu_vlan_enable,
    onu_vlan_enable_cmd,
    "vlan dot1q {[0|1]}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu dot1q vlan\n"
    "Disable 802.1Q vlan\n"
    "Enable 802.1Q Vlan\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "vlan dot1q", NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0, mode = 0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    
    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }
		/*如果进入onu节点之前onu关联了公有配置文件，不允许修改vlan配置，add by zhouzh*/
		if(Onu_profile_is_share(vty,ponID,ulOnuid) == VOS_OK)
		{
			vty_out( vty, "  %% onu-profile is share! you can't modify ，please private it。\r\n");
			return CMD_WARNING;
		}
        /*mode = getOnuConfPortVlanMode(ponID, ulOnuid - 1, 1);*/

        if(argc == 1)
        {
            ULONG enable = VOS_StrToUL(argv[0], NULL, 10);

#if 0
            if(mode == ONU_CONF_VLAN_MODE_TRUNK && enable)
            {
                vty_out(vty, "vlan dot1q has been enabled already!\r\n");
                return CMD_WARNING;
            }

            if(mode == ONU_CONF_VLAN_MODE_TRANSPARENT && (!enable))
            {
                vty_out(vty, "vlan dot1q has been disable already!\r\n");
                return CMD_WARNING;
            }
#endif
            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK != OnuMgt_SetVlanEnable(ponID, ulOnuid-1, enable) )
            {
#if 0                
                vty_out(vty, "enable vlan dot1q mode FAIL!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {   
            if(isCtcOnu(ulSlot, ulPort, ulOnuid) == TRUE)
            {

                CTC_STACK_port_vlan_configuration_t Vconfig;

                if(OnuMgt_GetEthPortVlanConfig(ponID, ulOnuid - 1, 1, &Vconfig) == VOS_OK)
                {
#if 0                
                    unsigned char ver = 0;
                    
                    OnuGen_Get_CtcVersion(ponID, ulOnuid - 1, &ver); 
                    if(ver >= CTC_2_1_ONU_VERSION)
                    {
                        if(Vconfig.mode == CTC_VLAN_MODE_TRUNK)
                            mode = 1;
                        else if(Vconfig.mode == CTC_VLAN_MODE_TRANSPARENT)
                            mode = 2;
                    }
                    else
                    {
                        if(Vconfig.mode == CTC_VLAN_MODE_TAG)
                            mode = 1;
                        else if(Vconfig.mode == CTC_VLAN_MODE_TRANSPARENT)
                            mode = 2;
                    }
#endif					
                }
                else
                {
                    vty_out(vty, ONU_CMD_ERROR_STR);                    
                    return CMD_WARNING;
                }
#if 0				
                if (mode == 1)
                    vty_out(vty, "vlan dot1q enable%s", VTY_NEWLINE);
                else if(mode == 2)
                    vty_out(vty, "vlan dot1q disable%s", VTY_NEWLINE);
                else
                    vty_out(vty, "unknown error!\r\n");
#else
                if(Vconfig.mode == CTC_VLAN_MODE_TRUNK)
                    vty_out(vty, "vlan dot1q enable%s", VTY_NEWLINE);
                else
            	{
                    vty_out(vty, "vlan dot1q disable%s", VTY_NEWLINE);
					if(Vconfig.mode == CTC_VLAN_MODE_TAG)
	                    vty_out(vty, "vlan mode tag\r\n");
	                else 
	                    vty_out(vty, "unknown error!\r\n");						
            	}
#endif
#if 0
                if (mode == ONU_CONF_VLAN_MODE_TRANSPARENT)
                    vty_out(vty, "\nvlan dot1q disable\r\n");
                else if (mode == ONU_CONF_VLAN_MODE_TRUNK)
                    vty_out(vty, "\nvlan dot1q enable\r\n");
                else
                    vty_out(vty, "unknown error!\r\n");
#endif
            }
            
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
       if(argc == 1)
       {
           ULONG en = VOS_StrToUL(argv[0], NULL, 10);
           ULONG mode = en?ONU_CONF_VLAN_MODE_TRUNK:ONU_CONF_VLAN_MODE_TRANSPARENT;
           int i = 0;
           int curmode = getOnuConfVlanModeByPtr(suffix, vty->onuconfptr);

           if(curmode != mode)
           {
               if(setOnuConfVlanModeByPtr(suffix, vty->onuconfptr, mode) != VOS_OK)
                   vty_out(vty, "vlan dot1q %s ERROR!\r\n", en?"enable":"disable");
           }
           /*delete by luh@2015-6-12.vlan单端口透传与之前的命令隔离开，需要单独配置。不再关联改动，以防引入新问题*/
#if 0           
           for(i=1; i<=ONU_MAX_PORT;i++)
           {
                setOnuConfPortSimpleVarByPtr(suffix, vty->onuconfptr, i, sv_enum_onu_transparent_enable, en?0:1);
           }
#endif           
       }
       else
       {
           int mode = getOnuConfVlanModeByPtr(suffix, vty->onuconfptr);
#if 0
           if (mode == ONU_CONF_VLAN_MODE_TRANSPARENT)
               vty_out(vty, "\nvlan dot1q disable\r\n");
           else if (mode == ONU_CONF_VLAN_MODE_TRUNK)
               vty_out(vty, "\nvlan dot1q enable\r\n");
           else
               vty_out(vty, "unknown error!\r\n");
#else
           
               
           if (mode == ONU_CONF_VLAN_MODE_TRUNK)
               vty_out(vty, "\nvlan dot1q enable\r\n");
           else
           {
           	   vty_out(vty, "\nvlan dot1q disable\r\n");
           	   if (mode == ONU_CONF_VLAN_MODE_TAG)
				   vty_out(vty, "vlan mode tag\r\n");
           	   else
	               vty_out(vty, "unknown error!\r\n");
           }		                  		
#endif
       }
    }


    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%vlan dot1q ERROR!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}

#if 0
DEFUN  (
    onu_vlan_mode,
    onu_vlan_mode_cmd,
    "vlan mode <port_list> {[transparent|tag|translation|agg|trunk]}*1",
    "Show or config onu vlan information\n"
    "Show or config onu vlan mode\n"
    "inpurt port list \n"
    "transparetn vlan\n"
    "tag Vlan\n"
    "translation vlan\n"
    "agg vlan\n"
    "trunk vlan\n"
    )
{

    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;
    ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
    if( ulRet !=VOS_OK )
        return CMD_WARNING;

    ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
    if(ponID == (VOS_ERROR))
    {
        vty_out( vty, "  %% Parameter error\r\n");
        return CMD_WARNING;
    }


    if(isCtcOnu(ulSlot, ulPort, ulOnuid))
    {
        if(argc == 2)
        {
            ULONG portlist = 0;
            ULONG type = 0, mode;

            ONU_CONF_WRITEABLE_CHECK

            if(!VOS_StriCmp(argv[1], "transparent"))
                mode = CTC_VLAN_MODE_TRANSPARENT;
            else if(!VOS_StriCmp(argv[1], "tag"))
                mode = CTC_VLAN_MODE_TAG;
            else if(!VOS_StriCmp(argv[1], "translation"))
                mode = CTC_VLAN_MODE_TRANSLATION;
            else if(!VOS_StriCmp(argv[1], "agg"))
                mode = CTC_VLAN_MODE_AGGREGATION;
            else
                mode = CTC_VLAN_MODE_TRUNK;

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                if( OnuMgt_SetVlanMode(ponID, ulOnuid-1, port, mode) == VOS_OK)
                    vty_out(vty, "ctc execute ok!\r\n");
                else
                    vty_out(vty, "ctc excute err!\r\n");
            }
            END_PARSE_PORT_LIST_TO_PORT();
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {

                int mode = getOnuConfPortVlanMode(ponID, ulOnuid-1, port);

                switch(mode)
                {

                case ONU_CONF_VLAN_MODE_TRANSPARENT:
                    vty_out(vty, "port %d vlan mode %s\r\n", port, "transparent");
                    break;
                case ONU_CONF_VLAN_MODE_TAG:
                    vty_out(vty, "port %d vlan mode %s\r\n", port, "tag");
                    break;
                case ONU_CONF_VLAN_MODE_TRUNK:
                    vty_out(vty, "port %d vlan mode %s\r\n", port, "trunk");
                    break;
                case ONU_CONF_VLAN_MODE_TRANSLATION:
                    vty_out(vty, "port %d vlan mode %s\r\n", port, "translation");
                    break;
                case ONU_CONF_VLAN_MODE_AGG:
                    vty_out(vty, "port %d vlan mode %s\r\n", port, "aggregation");
                    break;
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
        }
    }

    return CMD_SUCCESS;

}
#else
DEFUN  (
    onu_vlan_mode,
    onu_vlan_mode_cmd,
    "vlan mode {[transparent|tag|translation|agg|trunk]}*1",
    "Show or config onu vlan information\n"
    "Show or config onu vlan mode\n"
    "transparetn vlan\n"
    "tag Vlan\n"
    "translation vlan\n"
    "agg vlan\n"
    "trunk vlan\n"
    )
{

    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }


        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {
            if(argc == 1)
            {
                int mode = 0, portnum = 0;

                ONU_CONF_WRITEABLE_CHECK

                if(!VOS_StriCmp(argv[0], "transparent"))
                    mode = CTC_VLAN_MODE_TRANSPARENT;
                else if(!VOS_StriCmp(argv[0], "tag"))
                    mode = CTC_VLAN_MODE_TAG;
                else if(!VOS_StriCmp(argv[0], "translation"))
                    mode = CTC_VLAN_MODE_TRANSLATION;
                else if(!VOS_StriCmp(argv[0], "agg"))
                    mode = CTC_VLAN_MODE_AGGREGATION;
                else
                    mode = CTC_VLAN_MODE_TRUNK;

				{
					ULONG devidx = MAKEDEVID(ulSlot, ulPort, ulOnuid);
					if(getDeviceCapEthPortNum(devidx, &portnum) != VOS_OK)
					{
						vty_out(vty,"unknown onu port num!\r\n");
						return CMD_WARNING;
					}					
				}

                if(portnum)
                {
                    int error_flag = 0;
                    for(port = 1; port <=portnum; port++)
                    {
                        if( OnuMgt_SetVlanMode(ponID, ulOnuid-1, port, mode) != VOS_OK)
                        {
#if 0                            
                            vty_out(vty, "port %d set vlan mode fail!\r\n", port);
#else
                            error_flag = 1;
#endif
                        }
                    }
                    if(error_flag)
                        vty_out(vty, ONU_CMD_ERROR_STR);                       
                }
                /*moved by luh 2012-12-14, 单纯修改vlan模式，其pon板的vlan不会被删掉，此处放到rpc调用中*/
#if 0
                {
                    int curmode = getOnuConfVlanMode(ponID, ulOnuid-1);
                    if(curmode != ONU_CONF_VLAN_MODE_UNKNOWN && curmode != mode)
                        setOnuConfVlanMode(ponID, ulOnuid-1, mode);
                }
#endif
            }
            else
            {
                /*int mode = getOnuConfVlanMode(ponID, ulOnuid-1);*/
                char szmode[80] = "";
                CTC_STACK_port_vlan_configuration_t Vconfig;

                if(OnuMgt_GetEthPortVlanConfig(ponID, ulOnuid - 1, 1, &Vconfig) == VOS_OK)
                {
                    switch(Vconfig.mode)
                    {
                        case ONU_CONF_VLAN_MODE_TRANSPARENT:
                            VOS_StrCpy(szmode, "transparent");
                            break;
                        case ONU_CONF_VLAN_MODE_TRANSLATION:
                            VOS_StrCpy(szmode, "translation");
                        break;
                    case ONU_CONF_VLAN_MODE_AGG:
                        VOS_StrCpy(szmode, "aggregation");
                        break;
                    case ONU_CONF_VLAN_MODE_TRUNK:
                        VOS_StrCpy(szmode, "trunk");
                            break;
                        case ONU_CONF_VLAN_MODE_TAG:
                            VOS_StrCpy(szmode, "tagged");
                            break;
                    }
                    vty_out(vty, "vlan mode is %s\r\n", szmode);
                }
                else
                {
#if 0                    
                   vty_out(vty, "vlan mode get error!\r\n"); 
#else
                    vty_out(vty, ONU_CMD_ERROR_STR);
#endif
                }
            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(argc == 1)
        {
            int mode = 0;
            int i = 0;
            if(!VOS_StriCmp(argv[0], "transparent"))
                mode = CTC_VLAN_MODE_TRANSPARENT;
            else if(!VOS_StriCmp(argv[0], "tag"))
                mode = CTC_VLAN_MODE_TAG;
            else if(!VOS_StriCmp(argv[0], "translation"))
                mode = CTC_VLAN_MODE_TRANSLATION;
            else if(!VOS_StriCmp(argv[0], "agg"))
                mode = CTC_VLAN_MODE_AGGREGATION;
            else
                mode = CTC_VLAN_MODE_TRUNK;

            if(setOnuConfVlanModeByPtr(suffix, pd, mode) != VOS_OK)
                vty_out(vty, "config vlan mode fail!\r\n");
            /*del by luh@2015-6-12 */
#if 0                
            /*同步修改vlan透传使能，2013-12-06*/
            if(mode == CTC_VLAN_MODE_TRUNK || mode == CTC_VLAN_MODE_TRANSPARENT)
            {
                for(i=1; i<=ONU_MAX_PORT;i++)
                {
                    setOnuConfPortSimpleVarByPtr(suffix, pd, i, sv_enum_onu_transparent_enable, CTC_VLAN_MODE_TRANSPARENT==mode?1:0);
                }
            }
#endif            
        }
        else
        {
            int mode = getOnuConfVlanModeByPtr(suffix, pd);
            char szmode[80] = "";

            switch(mode)
            {
               case ONU_CONF_VLAN_MODE_TRANSPARENT:
                    VOS_StrCpy(szmode, "transparent");
                        break;
                    case ONU_CONF_VLAN_MODE_TRANSLATION:
                        VOS_StrCpy(szmode, "translation");
                        break;
                    case ONU_CONF_VLAN_MODE_AGG:
                        VOS_StrCpy(szmode, "aggregation");
                        break;
                    case ONU_CONF_VLAN_MODE_TRUNK:
                        VOS_StrCpy(szmode, "trunk");
                        break;
                    case ONU_CONF_VLAN_MODE_TAG:
                        VOS_StrCpy(szmode, "tagged");
                        break;
             }
             vty_out(vty, "vlan mode is %s\r\n", szmode);
            
        }
    }
    return CMD_SUCCESS;

}

#endif
extern int getOnuConfUntagPortVlan(short int ponid, short int onuid, short int port);

DEFUN  (
    onu_vlan_pvid,
    onu_vlan_pvid_cmd,
    "vlan pvid <port_list> {<1-4094>}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu vlan pvid\n"
    "Please input the port number\n"
    "Please input the pvid(1-4094)\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	int onu_mask[] = {V2R1_ONU_GT810, V2R1_ONU_GT816, 0 };
	return onu_dev_cli_process( vty, "vlan pvid", onu_mask );
#elif defined __RPC_ONU_COMMAND

    int ponID=0,pvid = 0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        /*VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)*/

        if(argc == 2)
        {
			ULONG vid = 0;

            int lv = 0;
			
            ONU_CONF_WRITEABLE_CHECK

            vid = VOS_StrToUL(argv[1], NULL, 10);

			if(!getOnuConfRestoreFlag(ponID,ulOnuid-1))
			{
				BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            	{                        
                    lv = getOnuConfUntagPortVlan(ponID, ulOnuid-1, port);					
                    if(lv && lv != vid)
                    {
                    	vty_out(vty,"Port %d is not untagged in vlan %d\r\n",port,vid);
                        RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
                    }	
            	}
            	END_PARSE_PORT_LIST_TO_PORT();
			}
/*deleted by liyang @ 2014-09-05 */
#if 0
			BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
			{
            	if(VOS_OK != OnuMgt_SetPortPvid(ponID, ulOnuid-1, port, VOS_StrToUL(argv[1], NULL, 10)) )
            	{
                   	error_flag = 1;
            	}
			}
			END_PARSE_PORT_LIST_TO_PORT();

			if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                {
                    /*getOnuConfPortSimpleVar(ponID, ulOnuid - 1, port, sv_enum_port_default_vid, &pvid);*/
                    CTC_STACK_port_vlan_configuration_t pvc;
                    if(OnuMgt_GetEthPortVlanConfig(ponID, ulOnuid-1, port, &pvc) == VOS_OK)
                    {
                        if(pvc.mode == CTC_VLAN_MODE_TRUNK)
                        vty_out(vty, "\r\nport %d default vid is: %d\r\n", port, pvc.default_vlan&0xfff);
                        else
                        vty_out(vty, "\r\n port %d default vid get fail! onu's vlan mode is incorrect\r\n", port);
                    }
                    else
                    {
#if 0                                        
                        vty_out(vty, "\r\n port %d default vid get fail!\r\n", port);
#else
                        error_flag = 1;
#endif
                    }
                }
        		END_PARSE_PORT_LIST_TO_PORT();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);                    
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

        VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        if(argc == 2)
	    {
			ULONG vid = VOS_StrToUL(argv[1], NULL, 10);

			int lv = 0;
			
		    BEGIN_PARSE_PORT_LIST_TO_PORT( argv[0], port )
		    {
                lv = getOnuConfUntagPortVlanByPtr(suffix,pd,port);
		 				
                if(lv && lv != vid)
                {
                	vty_out(vty,"Port %d is not untagged in vlan %d\r\n",port,vid);
                    RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
                }  	
#if 0
                if(VOS_OK != setOnuConfPortSimpleVarByPtr(suffix, pd, port,sv_enum_port_default_vid, pvid) )
                    vty_out(vty, "config default vlan id fail!\r\n");
#endif
			}
		    END_PARSE_PORT_LIST_TO_PORT();
			
	    }
	    else
	    {
		    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
		    {
		        getOnuConfPortSimpleVarByPtr(suffix, pd, port,sv_enum_port_default_vid, &pvid);
		        vty_out(vty, "\nport %d default vid is: %d\r\n", port, pvid);
		    }
			END_PARSE_PORT_LIST_TO_PORT()
		    }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
	if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	/* modified by chenfj 2007-11-12
	问题单#5778: 无法将端口的PVID设置为默认1
	在GT831/821/810/816中，由于端口的PVID是直接跟端口的默认VLAN相关的，PVID命令没有用处并经常造成理解问题，建议在OLT的这四个ONU节点下去除该命令
     处理：在831节点下删除该命令。
                     在810/816节点下该命令保留，因为当前810/816与811/812共用一个命令节点，而这个命令在811/812上是有用的；但在处理时检查ONU类型；
                     若是810/816，则给出出错信息提示：GT810(或GT816)don't support this cli command
	*/	
	lRet = GetOnuType(ponID, (ulOnuid-1), &OnuType);
	if( lRet !=VOS_OK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}
	if(( OnuType < V2R1_ONU_GT811) && ( OnuType >= V2R1_ONU_MAX))
		return CMD_WARNING;
	if(( OnuType == V2R1_ONU_GT810 ) || ( OnuType == V2R1_ONU_GT816))
		{
		vty_out(vty,"%s don't support this cli command\r\n", GetDeviceTypeString(OnuType));
		return CMD_WARNING;
		}
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Vlan pvid FAILED!\r\n");	
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_vlan_forward_type,
    onu_vlan_forward_type_cmd,
    "vlan acceptable_frame_types <port_list> {[tagged|all]}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu vlan forward type\n"
    "Please input the port number\n"
    "Only forward tagged frames\n"
    "Forward untagged and tagged frames\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "vlan acceptable_frame_types" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
               if(VOS_OK != OnuMgt_SetPortVlanFrameTypeAcc(ponID, ulOnuid-1, port, !VOS_StriCmp(argv[1], "all")?1:0) )
                {
#if 0                    
                    vty_out(vty, "vlan filter config fail!\r\n");
#else
                    error_flag = 1;
#endif
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {

                 ULONG acc = 0;
                 BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                 {
                     if(getOnuConfPortSimpleVar(ponID, ulOnuid-1, port,sv_enum_port_vlan_accept_type, &acc))
                     {
                         vty_out(vty, "\r\nport %d vlan accept type get error\r\n");
                     }
                     else
                         vty_out(vty, "\r\nport %d vlan accept type is: %s\r\n",  port, acc?"all":"tagged");
                 }END_PARSE_PORT_LIST_TO_PORT()
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

        VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        if(argc == 2)
		{
			BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
			{
	           if(VOS_OK != setOnuConfPortSimpleVarByPtr(suffix, pd, port,sv_enum_port_vlan_accept_type, !VOS_StriCmp(argv[1], "all")?1:0) )
	               vty_out(vty, "vlan filter config fail!\r\n");
			}
			END_PARSE_PORT_LIST_TO_PORT();
        }
		else
		{
            ULONG acc = 0;
			BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
			{
			    if(getOnuConfPortSimpleVarByPtr(suffix, pd, port,sv_enum_port_vlan_accept_type, &acc))
			    {
                    vty_out(vty, "\r\nport %d vlan accept type get error\r\n");
			    }
                else
                    vty_out(vty, "\r\nport %d vlan accept type is: %s\r\n",  port, acc?"all":"tagged");
			}
            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
   	if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();
	
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%vlan acceptable_frame_types FAILED!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
#endif
}


DEFUN  (
    onu_vlan_ingress_filter,
    onu_vlan_ingress_filter_cmd,
    "vlan ingress_filtering <port_list> {[0|1]}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu vlan ingress filter\n"
    "Please input the port number\n"
    "Disable ingress filter\n"
    "Enable ingress filter\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "vlan ingress_filtering" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    ULONG devIdx, brdIdx, pon, onu;
    USHORT portNum;
    
    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }


        VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {

            {
                BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
                {
                    if(VOS_OK != OnuMgt_SetPortIngressVlanFilter(ponID, ulOnuid-1, port, VOS_StrToUL(argv[1], NULL, 10)) )
                    {
#if 0                        
                        vty_out(vty, "port %d vlan ingress filter config fail!\r\n", port);
#else
                        error_flag = 1;
#endif
                    }
                }
                END_PARSE_PORT_LIST_TO_PORT();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                if (parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
                    return CMD_WARNING;

                if ((getDeviceCapEthPortNum(devIdx, &portNum) != VOS_OK) || portNum > MAX_ETH_PORT_NUM)
                {
                    vty_out(vty, "no ethernet port\r\n");
                    return CMD_WARNING;
                }
                else
                {
                    ULONG mode = 0;
                    {
                        BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
                        {
                            if(VOS_OK == getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_ingress_vlan_filter, &mode))
                                vty_out(vty, "\r\nport %d vlan ingress_filtering is %s\r\n", port, mode?"Enable":"Disable");
                            else
                                vty_out(vty, "\r\nport vlan ingress_filtering get error!\r\n");
                        }
                        END_PARSE_PORT_LIST_TO_PORT();
                    }

                }
            }
            else
            {
                char *pRecv;
                USHORT len;

                if (VOS_OK == OnuMgt_CliCall(ponID, ulOnuid - 1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG mode = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

        VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        if(argc == 2)
        {
            mode = VOS_StrToUL(argv[1], NULL, 10);

            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_vlan_filter, mode))
                    vty_out(vty, "port %d vlan ingress_filtering set fail!\r\n", port);
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(getOnuConfPortSimpleVarByPtr(suffix, pd , port, sv_enum_port_ingress_vlan_filter, &mode))
                    vty_out(vty, "port %d vlan ingress_filtering get fail!\r\n", port);
                else
                    vty_out(vty, "\r\nport %d vlan ingress_filtering is %s\r\n", port, mode?"Enable":"Disable");
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
      if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	  */
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%vlan ingress_filtering FAILED !\r\n");	


	return CMD_SUCCESS;
#endif
}


DEFUN  (
    onu_vlan_dot1q_add,
    onu_vlan_dot1q_add_cmd,
    "vlan dot1q_add <2-4094>",
    "Show or config onu vlan information\n"
    "Add 802.1Q Vlan\n"
    "Please input the Vlan Vid(2-4094)\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "vlan dot1q_add",NULL );
#elif defined __RPC_ONU_COMMAND

	int ponID=0, mode;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    int vid = VOS_StrToUL(argv[0], NULL, 10);
    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }
		/*如果进入onu节点之前onu关联了公有配置文件，不允许修改vlan配置，add by zhouzh*/
		if(Onu_profile_is_share(vty,ponID,ulOnuid) == VOS_OK)
		{
			vty_out( vty, "  %% onu-profile is share! you can't modify ，please private it。\r\n");
			return CMD_WARNING;
		}
        /* vlan 批处理产生的vlan 不可被常规命令行修改 add by luh 2011-12-28*/
        /* gw onu在透传模式不需要进行配置文件的检查Q.24722*/
        if(((!OnuTransmission_flag)||(IsCtcOnu(ponID, ulOnuid-1))) && IsBelongToVlanRules(vid, ulSlot, ulPort, ulOnuid)  && !getOnuConfRestoreFlag(ponID,ulOnuid-1))
        {
            vty_out(vty, "vlan %d has created by Vlan Rules!\r\n",vid);
            return CMD_WARNING;
        }

        ONU_CONF_WRITEABLE_CHECK
        if(VOS_OK != OnuMgt_AddVlan(ponID, ulOnuid-1, vid))
        {
#if 0                
            vty_out(vty, "add vlan %s fail\r\n", argv[0]);
#else
            vty_out(vty, ONU_CMD_ERROR_STR);
#endif
        }

    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        ULONG all, untag;

        if(get_onuconf_vlanPortlistByPtr(suffix, pd, vid, &all, &untag) != VOS_OK)
        {
            if(VOS_OK != set_onuconf_vlanPortlistByPtr(suffix, pd, vid,0,0))
                vty_out(vty, "add vlan %d fail\r\n", vid);
        }
        else
            vty_out(vty, "vlan %d has exist!\r\n", vid);
    }


    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%vlan dot1q_add FAILED!\r\n");	


	return CMD_SUCCESS;
#endif
}


DEFUN  (
    onu_vlan_dot1q_port_add,
    onu_vlan_dot1q_port_add_cmd,
    "vlan dot1q_port_add <2-4094> <port_list> [1|2]",
    "Show or config onu vlan information\n"
    "Add 802.1Q Vlan port\n"
    "Please input the Vlan Vid(2-4094)\n"
    "Please input the port number\n"
    "Tagged\n"
    "Untagged\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[1], "vlan dot1q_port_add" );
#elif defined __RPC_ONU_COMMAND

	int ponID=0, mode, pvid=0, enable = 0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    ULONG i = 0, entryNum=0;
    ULONG untagmask = 0, allmask = 0,flag = 0;
    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }
		
		/*如果进入onu节点之前onu关联了公有配置文件，不允许修改vlan配置，add by zhouzh*/
		if(Onu_profile_is_share(vty,ponID,ulOnuid) == VOS_OK)
		{
			vty_out( vty, "  %% onu-profile is share! you can't modify ，please private it。\r\n");
			return CMD_WARNING;
		}
        /*自动筛选onu端口，保证配置的端口小于onu实际端口*/
        /*VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[1], port)*/

#if 0
        mode = getOnuConfVlanMode(ponID, ulOnuid-1);
        if(mode != ONU_CONF_VLAN_MODE_TRUNK)
        {
            vty_out(vty, "vlan dot1q is disable!\r\n");
            return CMD_WARNING;
        }
#endif
        if(argc == 3)
        {
            ULONG vid = 0, portlist=0, tag = 0;
            int lv = 0, gMaxPort = 0;
            ONU_CONF_WRITEABLE_CHECK

            vid = VOS_StrToUL(argv[0], NULL, 10);
            tag = VOS_StrToUL(argv[2], NULL, 10);
            if(getDeviceCapEthPortNum(MAKEDEVID(ulSlot, ulPort, ulOnuid), &gMaxPort) != VOS_OK)
            {
                vty_out(vty, "unknown onu port num!\r\n");
                return CMD_WARNING;
            }
            
            /* vlan 批处理产生的vlan 不可被常规命令行修改 add by luh 2011-12-28*/  
            /* gw onu在透传模式不需要进行配置文件的检查，统一由onu进行提示*/                
#if 0                
            if(((!OnuTransmission_flag)||(IsCtcOnu(ponID, ulOnuid-1))) &&IsBelongToVlanRules(vid, ulSlot, ulPort, ulOnuid) && !getOnuConfRestoreFlag(ponID,ulOnuid-1))
            {
                vty_out(vty, "vlan %d is created by Vlan Rules! It must be modified by Vlan Rules Command!\r\n",vid);
                return CMD_WARNING;
            }
            
            entryNum = onuconf_get_VlanEntryNum(ponID, ulOnuid-1); 
			
			/*modified by liyang @2014-09-01 for profile content error*/
            if(isCtcOnu(ulSlot, ulPort, ulOnuid) && !getOnuConfRestoreFlag(ponID,ulOnuid-1))
            {
                for(i=0;i<entryNum;i++)
                {
                    if(onuconf_get_vlanEntryByIdx(ponID, ulOnuid-1, i, &pvid, &allmask, &untagmask)==VOS_OK&&pvid == vid)
                    {
                        flag = 1;
                        break;
                    }
                }
                if(!flag)
                {
                    vty_out(vty, "vlan %d doesn't exit!\r\n",vid);
                    return CMD_WARNING;
                }
            }
            
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[1], port)
            {
        		if(!getOnuConfRestoreFlag(ponID,ulOnuid-1))
        		{                            
                    lv = getOnuConfUntagPortVlan(ponID, ulOnuid-1, port);
                    getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_onu_transparent_enable, &enable);
                    if(enable)
                    {
                        vty_out(vty, "port %d vlan mode is transparent, please undo it first!\r\n", port);
                        RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
                }
                if(tag == 2 && lv && (1 != lv) && lv != vid)
                {
                    vty_out(vty, "port %d has added to vlan %d as untagged port!\r\n", port, lv);
                    RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
                    }
                    else
                    {
                        portlist |= 1<<(port-1);
                    }
                }
                else
                {
                    portlist |= 1<<(port-1);
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
#else
            /*先初始化一下端口，后面可能针对gw 透传模式不再初始化，直接进行配置*/
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[1], port)
            {
                if(port && port <= gMaxPort)                    
                    portlist |= 1<<(port-1);
            }
            END_PARSE_PORT_LIST_TO_PORT();                     

            /*CTC ONU及GW ONU在非透传模式需要进行配置文件检测Q.24722*/
            if(((!OnuTransmission_flag)||(IsCtcOnu(ponID, ulOnuid-1))))
            {
                /*在配置恢复过程中，不需要检测配置文件，避免undo和do配置文件时有冲突*/
                if(!getOnuConfRestoreFlag(ponID,ulOnuid-1))
                {
                    if(IsBelongToVlanRules(vid, ulSlot, ulPort, ulOnuid) )
                    {
                        vty_out(vty, "vlan %d is created by Vlan Rules! It must be modified by Vlan Rules Command!\r\n",vid);
                        return CMD_WARNING;
                    }

                    entryNum = onuconf_get_VlanEntryNum(ponID, ulOnuid-1); 
                    for(i=0;i<entryNum;i++)
                    {
                        if(onuconf_get_vlanEntryByIdx(ponID, ulOnuid-1, i, &pvid, &allmask, &untagmask)==VOS_OK&&pvid == vid)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    if(!flag)
                    {
                        vty_out(vty, "vlan %d doesn't exit!\r\n",vid);
                        return CMD_WARNING;
                    }
                }
                
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[1], port)
                {
                    if(!getOnuConfRestoreFlag(ponID,ulOnuid-1))
                    {                            
                        lv = getOnuConfUntagPortVlan(ponID, ulOnuid-1, port);
                        getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_onu_transparent_enable, &enable);
                        if(enable)
                        {
                            vty_out(vty, "port %d vlan mode is transparent, please undo it first!\r\n", port);
                            RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
                        }
                        if(tag == 2 && lv && (1 != lv) && lv != vid)
                        {
                            vty_out(vty, "port %d has added to vlan %d as untagged port!\r\n", port, lv);
                            RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
                        }
                        else
                        {
                            /*此处提前初始化*/
                            /*portlist |= 1<<(port-1);*/
                        }
                    }
                    else
                    {
                        /*配置文件恢复过程也需要生成实际的端口列表，且
                                              认为配置文件已经进行过合法性检查*/
                        /*此处提前初始化*/                                              
                        /*portlist |= 1<<(port-1);*/
                    }
                }
                END_PARSE_PORT_LIST_TO_PORT();                     
            }                                                   
#endif
            if(OnuMgt_AddVlanPort(ponID, ulOnuid-1, vid, portlist, tag) !=VOS_OK)
            {
#if 0                    
                vty_out(vty, "add vlan %s port %s fail\r\n", argv[0], argv[1]);
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
                return CMD_WARNING;
            }
        }        

    }
    else
    {
        ULONG portlist=0, allportmask=0, untagportmask=0;
        ULONG untagportMask1 = 0;
        ULONG vid = VOS_StrToUL(argv[0], NULL, 10);
        ULONG tag = VOS_StrToUL(argv[2], NULL, 10);
        int lv;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;


        entryNum = onuconf_get_VlanEntryNumByPtr(suffix, pd); 

        VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[1], port)
        for(i=0;i<entryNum;i++)
        {
            if(onuconf_get_vlanEntryByIdxByPtr(suffix, pd, i, &pvid, &allmask, &untagmask)==VOS_OK&&pvid == vid)
            {
                flag = 1;
                break;
            }
        }
        if(!flag)
        {
            vty_out(vty, "vlan %d doesn't exit!\r\n",vid);
            return CMD_WARNING;
        }

        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[1], port)
        {
            lv = getOnuConfUntagPortVlanByPtr(suffix, pd, port);
            getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_onu_transparent_enable, &enable);
            if(enable)
            {
                vty_out(vty, "port %d vlan mode is transparent, please undo it first!\r\n", port);
                RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
            }
            if(tag == 2 && lv && (1 != lv)&&lv != vid)
            {
                vty_out(vty, "port %d has added to vlan %d as untagged port!\r\n", port, lv);
                RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
            }
            else
                portlist |= 1<<(port-1);
        }
        END_PARSE_PORT_LIST_TO_PORT();

        if(VOS_OK == get_onuconf_vlanPortlistByPtr(suffix, pd, vid, &allportmask, &untagportmask))
        {
            untagportMask1 = untagportmask; /*缓存当前vid 下的untag 端口列表*/
            allportmask |= portlist;        /*待恢复到当前vid 下的all 端口列表*/
            if(tag == 2)                       
                untagportmask |= portlist;  /*待恢复到当前vid 下的untagg ports 端口列表*/ 
            else
                untagportmask &= ~portlist;

            if(set_onuconf_vlanPortlistByPtr(suffix, pd, vid, allportmask, untagportmask) != VOS_OK)
                vty_out(vty, "add vlan %s port %s fail\r\n", argv[0], argv[1]);
		    if(tag == 2) /*untagg ports*/
		    {
                i = 0;
                if(portlist)
                {
                    if(VOS_OK == get_onuconf_vlanPortlistByPtr(suffix, pd, 1, &allportmask, &untagportmask))
                    {
                        allportmask &=(~portlist);
                        untagportmask &=(~portlist);
                        set_onuconf_vlanPortlistByPtr(suffix, pd, 1, allportmask, untagportmask);
                    }
                }                
		        while(portlist)
		        {
                    if(portlist&1)
                    {
		                setOnuConfPortSimpleVarByPtr(suffix, pd, i+1, sv_enum_port_default_vid, vid);
                    }
		            portlist >>= 1;
		            i++;
		        }
		    }
		    else
		    {
                
		                /* added by wangxiaoyu 2011-08-29
		                 * 以tag方式加入VLAN，PVID恢复为默认的VLAN 1*/
		        i = 0;
                untagportMask1 &= portlist; /*得到从当前vid 下需要回复到缺省vlan下的端口列表*/
                if(VOS_OK == get_onuconf_vlanPortlistByPtr(suffix, pd, 1, &allportmask, &untagportmask))
                {
                    allportmask |= untagportMask1;
                    untagportmask |= untagportMask1;
                    set_onuconf_vlanPortlistByPtr(suffix, pd, 1, allportmask, untagportmask);
                }
                while(untagportMask1>>i)
                {
                    if((untagportMask1>>i)&1)
                    {
		                setOnuConfPortSimpleVarByPtr(suffix, pd, i+1, sv_enum_port_default_vid, 1);/*modi by luh 2013-5-13*/
                    }
                    i++;
                }
		    }

        }
        else
            vty_out(vty, "add vlan %s port %s fail\r\n", argv[0], argv[1]);
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 1 ] );
   	 if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/	
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%vlan dot1q_port_add FAILED!\r\n");	

	return CMD_SUCCESS;
#endif
}


DEFUN  (
    onu_vlan_dot1q_del,
    onu_vlan_dot1q_del_cmd,
    "vlan dot1q_del <2-4094>",
    "Show or config onu vlan information\n"
    "Delete 802.1Q Vlan\n"
    "Please input the Vlan Vid\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "vlan dot1q_del",NULL );
#elif defined __RPC_ONU_COMMAND

	int ponID=0, mode;
    ULONG   ulIfIndex = 0, entryNum=0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix,  untagmask = 0, allmask = 0;
    int vid = 0, pvid = 0, flag = 0;
    int i = 0;
    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		/*如果进入onu节点之前onu关联了公有配置文件，不允许修改vlan配置，add by zhouzh*/
		if(Onu_profile_is_share(vty,ponID,ulOnuid) == VOS_OK)
		{
			vty_out( vty, "  %% onu-profile is share! you can't modify ，please private it。\r\n");
			return CMD_WARNING;
		}
		
        vid = VOS_StrToUL(argv[0], NULL, 10);
        entryNum = onuconf_get_VlanEntryNum(ponID, ulOnuid-1); 
#if 0
        /* vlan 批处理产生的vlan 不可被常规命令行修改 add by luh 2011-12-28*/
        /* gw onu在透传模式不需要进行配置文件的检查*/        
        if(((!OnuTransmission_flag)||(IsCtcOnu(ponID, ulOnuid-1))) && IsBelongToVlanRules(vid, ulSlot, ulPort, ulOnuid))
        {
            vty_out(vty, "vlan %d is created by Vlan Rules! It must be modified by Vlan Rules Command!\r\n",vid);
            return CMD_WARNING;
        }

		/* modified by liyang @2014-09-01 for profile content error */
        if(isCtcOnu(ulSlot, ulPort, ulOnuid) && !getOnuConfRestoreFlag(ponID,ulOnuid-1))
        {
            for(i=0;i<entryNum;i++)
            {
                if(onuconf_get_vlanEntryByIdx(ponID, ulOnuid-1, i, &pvid, &allmask, &untagmask)==VOS_OK&&pvid == vid)
                {
                    flag = 1;
                    break;
                }
            }
            if(!flag)
            {
                vty_out(vty, "vlan %d doesn't exit!\r\n",vid);
                return CMD_WARNING;
            }
        }
#else
        /*CTC ONU及GW ONU在非透传模式需要进行配置文件检测Q.24722*/
        if(((!OnuTransmission_flag)||(IsCtcOnu(ponID, ulOnuid-1))) && !getOnuConfRestoreFlag(ponID,ulOnuid-1))
        {
            if(IsBelongToVlanRules(vid, ulSlot, ulPort, ulOnuid))
            {
                vty_out(vty, "vlan %d is created by Vlan Rules! It must be modified by Vlan Rules Command!\r\n",vid);
                return CMD_WARNING;
            }
            for(i=0;i<entryNum;i++)
            {
                if(onuconf_get_vlanEntryByIdx(ponID, ulOnuid-1, i, &pvid, &allmask, &untagmask)==VOS_OK&&pvid == vid)
                {
                    flag = 1;
                    break;
                }
            }
            if(!flag)
            {
                vty_out(vty, "vlan %d doesn't exit!\r\n",vid);
                return CMD_WARNING;
            }
        }
#endif        
        ONU_CONF_WRITEABLE_CHECK
        if(VOS_OK != OnuMgt_DelVlan(ponID, ulOnuid-1, vid))
        {
#if 0                
            vty_out(vty, "delete vlan %s fail\r\n", argv[0]);
#else
            vty_out(vty, ONU_CMD_ERROR_STR);
#endif            
        }

    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        ULONG allports = 0, untagports = 0;
        ULONG allportMask,untagportMask = 0;

		entryNum = onuconf_get_VlanEntryNumByPtr(suffix, pd); 
        vid = VOS_StrToUL(argv[0], NULL, 10);
        for(i=0;i<entryNum;i++)
        {        
            if(onuconf_get_vlanEntryByIdxByPtr(suffix, pd, i, &pvid, &allmask, &untagmask)==VOS_OK&&pvid == vid)
            {
                flag = 1;
                break;
            }
        }
        if(!flag)
        {
            vty_out(vty, "vlan %d doesn't exit!\r\n",vid);
            return CMD_WARNING;
        }
        if(get_onuconf_vlanPortlistByPtr(suffix, pd, vid, &allports, &untagports) == VOS_OK)
        {
            get_onuconf_vlanPortlistByPtr(suffix, pd, 1, &allportMask, &untagportMask);
            untagportMask |= untagports;
            allportMask |= untagports;
            set_onuconf_vlanPortlistByPtr(suffix, pd, 1, allportMask, allportMask);    
        }
        if(del_onuconf_vlanByPtr(suffix, pd, vid)!=VOS_OK)
            vty_out(vty, "delete vlan %d fail\r\n", vid);
    }


    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );


	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%vlan dot1q_del !\r\n");	


	return CMD_SUCCESS;
#endif
}


DEFUN (
    onu_vlan_dot1q_port_del,
    onu_vlan_dot1q_port_del_cmd,
    "vlan dot1q_port_del <2-4094> <port_list>",
    "Show or config onu vlan information\n"
    "Delete 802.1Q Vlan port\n"
    "Please input the Vlan Vid\n"
    "Please input the port number\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[1], "vlan dot1q_port_del" );
#elif defined __RPC_ONU_COMMAND

	int pvid = 0, ponID=0, mode;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    ULONG vid, portlist=0;
	ULONG  entryNum = 0, flag = 0;
	ULONG allmask = 0, untagmask = 0, i = 0;
	int ret = 0;
    int mask_flag = 0;
    char portstr[80]="";
    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
		ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }
		
		/*如果进入onu节点之前onu关联了公有配置文件，不允许修改vlan配置，add by zhouzh*/
		if(Onu_profile_is_share(vty,ponID,ulOnuid) == VOS_OK)
		{
			vty_out( vty, "  %% onu-profile is share! you can't modify ，please private it。\r\n");
			return CMD_WARNING;
		}
		
        VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[1], port)

        mode = getOnuConfVlanMode(ponID, ulOnuid-1);

        {
            ONU_CONF_WRITEABLE_CHECK

            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[1], port)
            {
                portlist |= 1<<(port-1);
            }
            END_PARSE_PORT_LIST_TO_PORT();
            vid = VOS_StrToUL(argv[0], NULL, 10);
#if 0             
            entryNum = onuconf_get_VlanEntryNum(ponID, ulOnuid-1);            
            /* vlan 批处理产生的vlan 不可被常规命令行修改 add by luh 2011-12-28*/
            if(IsBelongToVlanRules(vid, ulSlot, ulPort, ulOnuid))
            {
                vty_out(vty, "vlan %d is created by Vlan Rules! It must be modified by Vlan Rules Command!\r\n",vid);
                return CMD_WARNING;
            }
            
            /*modified by liyang @2014-09-01 for profile content error*/
            if(isCtcOnu(ulSlot, ulPort, ulOnuid) && !getOnuConfRestoreFlag(ponID,ulOnuid-1))
            {
                for(i=0;i<entryNum;i++)
                {
                    if(onuconf_get_vlanEntryByIdx(ponID, ulOnuid-1, i, &pvid, &allmask, &untagmask)==VOS_OK&&pvid == vid)
                        {
                            flag = 1;
                        break;
                        }
                    }
                    if(!flag)
                {
                    vty_out(vty, "vlan %d doesn't exit!\r\n",vid);
                    return CMD_WARNING;
                }
                else/*当vlan 存在是检查所删除的端口是否存在added by luh 2011-10-14*/
                {
                    mask_flag = portlist & (~allmask);
                    if(mask_flag)
                    {
                        if(portListLongToString(mask_flag, portstr))
                            vty_out(vty, "vlan %d doesn't have port %s!\r\n",vid,portstr);
                        return CMD_WARNING;
                    }
                }
            }
#endif
			ret = OnuMgt_DelVlanPort(ponID, ulOnuid-1, vid, portlist);
			if(ret == OLT_ERR_PARAM)
				vty_out(vty, "vlan %d doesn't exit!\r\n",vid);
            else if(ret != VOS_OK )
            {
#if 0                
                vty_out(vty, "del vlan %d %s fail%s", vid, argv[1], VTY_NEWLINE);
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }

    }
    else
    {
        int vid = VOS_StrToUL(argv[0], NULL, 10);
        ULONG all,untag,untag1 = 0;
        ULONG allportmask,untagportmask;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        
		entryNum = onuconf_get_VlanEntryNumByPtr(suffix, pd); 		       

        VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[1], port)

        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[1], port)
        {
           portlist |= 1<<(port-1);
        }
        END_PARSE_PORT_LIST_TO_PORT();
        for(i=0;i<entryNum;i++)
        {        
            if(onuconf_get_vlanEntryByIdxByPtr(suffix, pd, i, &pvid, &allmask, &untagmask)==VOS_OK&&pvid == vid)
            {
                flag = 1;
                break;
            }
        }
        if(!flag)
        {
            vty_out(vty, "vlan %d doesn't exit!\r\n",vid);
            return CMD_WARNING;
        }
        else/*当vlan 存在是检查所删除的端口是否存在added by luh 2011-10-14*/
        {
            mask_flag = portlist & (~allmask);
            if(mask_flag)
            {
                if(portListLongToString(mask_flag, portstr))
                    vty_out(vty, "vlan %d doesn't have port %s!\r\n",vid,portstr);
                return CMD_WARNING;
            }
        }
        if(VOS_OK == get_onuconf_vlanPortlistByPtr(suffix, pd, vid, &all, &untag))
        {
            untag1 = (~portlist) & untag;            /*得到删除端口后, 当前 Vlan 下的untagged 列表*/
            all &= ~portlist;                        /*得到待恢复到当前VLAN 下的all 列表*/
            untag = portlist & untag;                /*得到待恢复到缺省VLAN 下的untagged 列表*/
            if(untag)
            {
                if(VOS_OK == get_onuconf_vlanPortlistByPtr(suffix, pd, 1, &allportmask, &untagportmask))
                {
                    allportmask |= untag;
                    untagportmask |= untag;
                    set_onuconf_vlanPortlistByPtr(suffix, pd, 1, allportmask, untagportmask);
                }
            }
            for(i=0; i<ONU_MAX_PORT; i++)
            {
                if((portlist&(1<<i)) && (untag & (1<<i)))
                {
                    untag |= 1<<i; 
                    setOnuConfPortSimpleVarByPtr(suffix, pd, i+1, sv_enum_port_default_vid, 1);
                }
            }
            /*end*/
            /*配置当前VLAN*/
            if(set_onuconf_vlanPortlistByPtr(suffix, pd, vid, all, untag1)!=VOS_OK)
                vty_out(vty, "del vlan %d %s fail%s", vid, argv[1], VTY_NEWLINE);
        }
        else
            vty_out(vty, "del vlan %d %s fail%s", vid, argv[1], VTY_NEWLINE);
    }

    return CMD_SUCCESS;
	
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 1 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%vlan dot1q_port_del !\r\n");	


	return CMD_SUCCESS;
#endif
}



DEFUN (
    onu_vlan_dot1q_show,
    onu_vlan_dot1q_show_cmd,
    "vlan dot1q_show {<2-4094>}*1",    
    "Show or config onu vlan information\n"
    "Show 802.1Q Vlan\n"
    "Please input the Vlan Vid\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
    return onu_dev_cli_process( vty, "vlan dot1q_show",NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID = 0;
    ULONG ulIfIndex = 0, port, vid = 0;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet, suffix;
    char strports[80], strports1[80];
    ulIfIndex = (ULONG) vty->index;

    if (!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
        if (ulRet != VOS_OK)
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
        if (ponID == (VOS_ERROR))
        {
            vty_out(vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }
        /*Q.29653*/
        if(GetOnuOperStatus(ponID, ulOnuid-1) != ONU_OPER_STATUS_UP)
        {
            vty_out(vty, "  %% %d/%d/%d is off-line.\r\n", ulSlot, ulPort, ulOnuid);
            return CMD_WARNING;
        }
        
        if (isCtcOnu(ulSlot, ulPort, ulOnuid))
        {

            if (argc == 1)
            {
                vid = VOS_StrToUL(argv[0], NULL, 10);
            }

            {
                ULONG all, untag, tag;
                int i = 0, j = 0, k = 0;
                int flag_untag, flag_all, flag_out;
                int num = 0, enable = 0;               
				int vlan_mode = 0;
                ULONG vlanIdx, transparent_portmask = 0;
                CTC_STACK_port_vlan_configuration_t Vconfig;
                ONUVlanEntryArry_t vlan;
                int maxPortNum = getOnuEthPortNum(ponID, ulOnuid - 1);
                int error_flag = 0;
                unsigned char ver = 0;
                VOS_MemZero(vlan, sizeof(ONUVlanEntryArry_t));                    

                OnuGen_Get_CtcVersion(ponID, ulOnuid - 1, &ver);
                for(port=0;port<maxPortNum;port++)
                {
                    flag_untag = 0;
                    flag_out = 0; 
                    vlanIdx = 0;
                    if(OnuMgt_GetEthPortVlanConfig(ponID, ulOnuid - 1, port+1, &Vconfig) != VOS_OK)
                        error_flag = 1;
                    else
                    {
                        if(ver >= CTC_2_1_ONU_VERSION)
                        {
                            vlanIdx = Vconfig.default_vlan & 0xfff;
                            if(Vconfig.mode == CTC_VLAN_MODE_TRUNK)
                            {
                                for(k=0;k<num;k++)
                                {
                                    if(vlan[k].vlanid == vlanIdx)
                                    {
                                        flag_untag = 1;
                                        vlan[k].untagPortMask |= 1<<port;
                                        vlan[k].allPortMask |= 1<<port;
                                        break;
                                    }
                                }
                                if(!flag_untag && vlanIdx)
                                {
                                    vlan[num].vlanid = vlanIdx;
                                    vlan[num].untagPortMask |= 1<<port;
                                    vlan[num].allPortMask |= 1<<port; 
                                    num++;
                                }
                            
                                for(j=0;j<Vconfig.number_of_entries;j++)
                                {
                                    flag_all = 0;
                                    vlanIdx = Vconfig.vlan_list[j]&0xfff;
                                    for(k=0;k<num;k++)
                                    {
                                        if(vlan[k].vlanid == vlanIdx)
                                        {
                                            flag_all = 1;
                                            vlan[k].allPortMask |= 1<<port;
                                            vlan[k].untagPortMask &= ~(1<<port);
                                            break;
                                        }
                                    }
                                    if(!flag_all)
                                    {
                                        vlan[num].vlanid = vlanIdx;
                                        vlan[num].allPortMask |= 1<<port;
                                        num++;
                                    }
                                }
                            	enable |= ENABLE;
                            }
                            else if(CTC_VLAN_MODE_TRANSPARENT == Vconfig.mode)
                            {
                                transparent_portmask |= 1<<port;
                            }
							else if(CTC_VLAN_MODE_TAG == Vconfig.mode)
							{
								vlanIdx = Vconfig.vlan_list[0]&0xfff;							
                                for(k=0;k<num;k++)
                                {
                                    if(vlan[k].vlanid == vlanIdx)
                                    {
                                        flag_untag = 1;
                                        vlan[k].untagPortMask |= 1<<port;
                                        vlan[k].allPortMask |= 1<<port;
                                        break;
                                    }
                                }
                                if(!flag_untag && vlanIdx)
                                {
                                    vlan[num].vlanid = vlanIdx;
                                    vlan[num].untagPortMask |= 1<<port;
                                    vlan[num].allPortMask |= 1<<port; 
                                    num++;
                                }
								vlan_mode = CTC_VLAN_MODE_TAG; 
							}
                        }
                        else
                        {
                            if(Vconfig.mode == CTC_VLAN_MODE_TAG)
                            {
                                vlanIdx = Vconfig.vlan_list[0]&0xfff;
                                for(k=0;k<num;k++)
                                {
                                    if(vlan[k].vlanid == vlanIdx)
                                    {
                                        flag_untag = 1;
                                        vlan[k].untagPortMask |= 1<<port;
                                        vlan[k].allPortMask |= 1<<port;
                                        break;
                                    }
                                }
                                if(!flag_untag && vlanIdx)
                                {
                                    vlan[num].vlanid = vlanIdx;
                                    vlan[num].untagPortMask |= 1<<port;
                                    vlan[num].allPortMask |= 1<<port; 
                                    num++;
                                }    
                                enable |= ENABLE;
                            }                        
                        }
                    }
                    /*else
                                        vty_out(vty, "%sPORT %d:vlan dot1q DISABLE%s", VTY_NEWLINE, VTY_NEWLINE);*/
                }
                /*moved by luh 2014-05-08*/
#if 0
                if(error_flag)
                {                    
                    vty_out(vty, ONU_CMD_ERROR_STR);
                    return CMD_WARNING;
                }   
#endif
				if(vlan_mode == CTC_VLAN_MODE_TAG)
				{
                    vty_out(vty, "%svlan mode tag%s", VTY_NEWLINE, VTY_NEWLINE);
                    if(transparent_portmask)
                    {
                        portListLongToString(transparent_portmask, strports);
                        vty_out(vty, "But for the port:%s, the vlan mode is transparent!%s", strports, VTY_NEWLINE);                            
                    }					
				}
                else if (enable == ENABLE)
                {
                    vty_out(vty, "%svlan dot1q enable%s", VTY_NEWLINE, VTY_NEWLINE);
                    if(transparent_portmask)
                    {
                        portListLongToString(transparent_portmask, strports);
                        vty_out(vty, "But for the port:%s, the vlan mode is transparent!%s", strports, VTY_NEWLINE);                            
                    }
                }
                else
                {
                    vty_out(vty, "%svlan dot1q disable%s", VTY_NEWLINE, VTY_NEWLINE);
                    return CMD_WARNING;
                }
                if (vid)
                {   
                    for(i=0;i<num;i++)
                    {
                        if (vlan[i].vlanid == vid)
                        {
                            flag_out = 1;
                            all = vlan[i].allPortMask;
                            untag = vlan[i].untagPortMask;
                            tag = all&(~untag);
                            break;
                        }
                    }
                    if(flag_out)
                    {
                        vty_out(vty, "%s %-9s%-35s %-30s", VTY_NEWLINE, "Vlan ID", "tag-portlist       ", "untag-portlist     ");
                        vty_out(vty, "%s-------------------------------------------------------------------------------------", VTY_NEWLINE);
                        portListLongToString(tag, strports);
                        portListLongToString(untag, strports1);
                        vty_out(vty, "%s %-9d%-35s %-30s", VTY_NEWLINE, vid, strports, strports1);
                        vty_out(vty, "%s-------------------------------------------------------------------------------------", VTY_NEWLINE);
                        vty_out(vty, "\r\n\r\n");                    
                    }
                    else
                       vty_out(vty, "vlan %d has not exist!\r\n", vid);
                }
                else
                {
                    vty_out(vty, "%s %-9s%-35s %-30s", VTY_NEWLINE, "Vlan ID", "tag-portlist       ", "untag-portlist     ");
                    vty_out(vty, "%s-------------------------------------------------------------------------------------", VTY_NEWLINE);
                    if (num > 0)
                    {
                        short int temp1 = 0; 
                        ULONG temp2 = 0, temp3 = 0;
                        for (i=0;i<num-1;i++)
                        {   
                            for(j=0;j<num-1-i;j++)
                            {
                                if(vlan[j].vlanid>vlan[j+1].vlanid)
                                {
                                    temp1 = vlan[j].vlanid;
                                    temp2 = vlan[j].allPortMask;
                                    temp3 = vlan[j].untagPortMask;
                                    vlan[j].vlanid = vlan[j+1].vlanid;
                                    vlan[j].allPortMask = vlan[j+1].allPortMask;
                                    vlan[j].untagPortMask = vlan[j+1].untagPortMask;                                
                                    vlan[j+1].vlanid = temp1;
                                    vlan[j+1].allPortMask = temp2;
                                    vlan[j+1].untagPortMask = temp3;   
                                }
                            }

                        }
                        
                        for (i=0;i<num;i++)
                        {
                            if (vlan[i].vlanid)
                            {
                                all = vlan[i].allPortMask;
                                untag = vlan[i].untagPortMask;
                                tag = all&(~untag);
                                portListLongToString(tag, strports);
                                portListLongToString(untag, strports1);
                                vty_out(vty, "%s %-9d%-35s %-30s", VTY_NEWLINE, vlan[i].vlanid, strports, strports1);
                            }
                        }
                        vty_out(vty, "%s-------------------------------------------------------------------------------------", VTY_NEWLINE);
                        vty_out(vty, "\r\n\r\n");                    
                    }
                }  

            }
        }
        else
        {
            char *pRecv;
            USHORT len;

            if (VOS_OK == OnuMgt_CliCall(ponID, ulOnuid - 1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
                vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }
    else
    {
        int vid = 0, port = 0, enable = 0;
        ULONG all, untag, tag, transparent_portmask = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*) vty->onuconfptr;
        if (argc == 1)
        {
            vid = VOS_StrToUL(argv[0], NULL, 10);
        }

        {
            int mode = getOnuConfVlanModeByPtr(suffix, pd);

            if (mode == ONU_CONF_VLAN_MODE_TRUNK)
            {
                vty_out(vty, "%svlan dot1q enable%s", VTY_NEWLINE, VTY_NEWLINE);
                for(port=1;port<=ONU_MAX_PORT;port++)
                {
                    getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_onu_transparent_enable, &enable);
                    if(enable)  
                        transparent_portmask |= 1<<(port-1); 
                }
                if(transparent_portmask)
                {
                    portListLongToString(transparent_portmask, strports);
                    vty_out(vty, "But for the port:%s, the vlan mode is transparent!%s", strports, VTY_NEWLINE);                            
                }
                
            }
			else if(mode == ONU_CONF_VLAN_MODE_TAG)
			{
                vty_out(vty, "%svlan mode tag%s", VTY_NEWLINE, VTY_NEWLINE);
                for(port=1;port<=ONU_MAX_PORT;port++)
                {
                    getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_onu_transparent_enable, &enable);
                    if(enable)  
                        transparent_portmask |= 1<<(port-1); 
                }
                if(transparent_portmask)
                {
                    portListLongToString(transparent_portmask, strports);
                    vty_out(vty, "But for the port:%s, the vlan mode is transparent!%s", strports, VTY_NEWLINE);                            
                }
			}
            else
            {
                vty_out(vty, "%svlan dot1q disable%s", VTY_NEWLINE, VTY_NEWLINE);
                return CMD_WARNING;
            }
            if (vid)
            {
                if (get_onuconf_vlanPortlistByPtr(suffix, pd, vid, &all, &untag) == VOS_OK)
                {
                    vty_out(vty, "%s %-9s%-35s %-30s", VTY_NEWLINE, "Vlan ID", "tag-portlist       ", "untag-portlist     ");
                     vty_out(vty, "%s-------------------------------------------------------------------------------------", VTY_NEWLINE);
                    tag = all&(~untag);
                    portListLongToString(tag, strports);
                    portListLongToString(untag, strports1);
                    vty_out(vty, "%s %-9d%-35s %-30s", VTY_NEWLINE, vid, strports, strports1);
                    vty_out(vty, "%s-------------------------------------------------------------------------------------", VTY_NEWLINE);
                    vty_out(vty, "\r\n\r\n");                    
                }
                else
                    vty_out(vty, "vlan %d has not exist!\r\n", vid);
            }
            else
            {
                ULONG num = 0;
                if(getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_VlanEntryNum, &num) == VOS_OK)
                {
                    vty_out(vty, "%s %-9s%-35s %-30s", VTY_NEWLINE, "Vlan ID", "tag-portlist       ", "untag-portlist     ");
                    vty_out(vty, "%s-------------------------------------------------------------------------------------", VTY_NEWLINE);
                    if (num > 0)
                    {
                        int i = 0, vid;
                        for (; i < num; i++)
                        {
                            all = 0;
                            if (onuconf_get_vlanEntryByIdxByPtr(suffix, pd, i, &vid, &all, &untag) == VOS_OK)
                            {
                                    tag = all&(~untag);
                                    portListLongToString(tag, strports);
                                    portListLongToString(untag, strports1);
                                    vty_out(vty, "%s %-9d%-35s %-30s", VTY_NEWLINE, vid, strports, strports1);
                            }
                        }
                    }  
                    vty_out(vty, "%s-------------------------------------------------------------------------------------", VTY_NEWLINE);
                    vty_out(vty, "\r\n\r\n");                    
				}
#if 0
            if (vid)
            {
                if (get_onuconf_vlanPortlistByPtr(suffix, pd, vid, &all, &untag) == VOS_OK)
                {
                    char strports[80];
                    vty_out(vty, "%svlan %d:%s", VTY_NEWLINE, vid, VTY_NEWLINE);
                    if (portListLongToString(all, strports))
                    {
                        vty_out(vty, "all port list : %s%s", strports, VTY_NEWLINE);
                        if (portListLongToString(untag, strports))
                            vty_out(vty, "untag port list : %s%s", strports, VTY_NEWLINE);
                    }
                    vty_out(vty, "\r\n");
                }
            }
            else
            {
                int num;
                getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_VlanEntryNum, &num);
                if (num > 0)
                {
                    int i = 0, vid;
                    for (; i < num; i++)
                    {
                        if (onuconf_get_vlanEntryByIdxByPtr(suffix, pd, i, &vid, &all, &untag) == VOS_OK)
                        {
                            char strports[80];
                            vty_out(vty, "\r\nvlan %d:\r\n", vid);
                            if (portListLongToString(all, strports))
                            {
                                vty_out(vty, "all port list : %s\r\n", strports);
                                if (portListLongToString(untag, strports))
                                    vty_out(vty, "untag port list : %s\r\n", strports);
                            }
                        }
                        vty_out(vty, "\r\n");
                    }
                }
            }
#endif
            }
        }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%vlan dot1q_show !\r\n");	


	return CMD_SUCCESS;
#endif
}

DEFUN (
    show_onu_profile,
    show_onu_profile_cmd,
    "show profile",
    SHOW_STR
    "display the profile\n"
    )
{

    int ponID = 0;
    int ret = CMD_SUCCESS;

    ULONG ulIfIndex = 0, onuEntry = 0;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex = (ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &ulRet))
    {
    ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
    if (ulRet != VOS_OK)
        return CMD_WARNING;

    ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
    if (ponID == (VOS_ERROR))
    {
        vty_out(vty, "  %% Parameter error\r\n");
        return CMD_WARNING;
    }

    onuEntry = ponID * MAXONUPERPON + ulOnuid - 1;

#ifdef ONUID_MAP
    ONU_CONF_SEM_TAKE
    {
#else
    ONU_MGMT_SEM_TAKE
#endif
    /*vty_out(vty, "\r\nprofile name is : %s\r\n\r\n", getOnuConfNamePtrByPonId(ponID, ulOnuid-1));*/
    vty_out(vty, "\r\n");

    {

        char *file = NULL/*(char*)cl_config_mem_file_init()*/;

        /*if (file)*/
        {
            if (!generateOnuConfClMemFile(getOnuConfNamePtrByPonId(ponID, ulOnuid-1), -1, vty, file,0,0,0))
            {
                vty_out(vty, "generate config file fail!\r\n");
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
            vty_out(vty, "alloc file memory fail!\r\n");
            ret = CMD_WARNING;
        }
         */
        vty_out(vty, "\r\n");
    }
#ifdef ONUID_MAP
    }
    ONU_CONF_SEM_GIVE
#else
    ONU_MGMT_SEM_GIVE
#endif
    }
    else
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

            if(onuConfCheckByPtr(ulRet, pd) == VOS_OK)
            {

                char *file = NULL/*(char*)cl_config_mem_file_init()*/;

                /*if (file)*/
                {
                    if (!generateOnuConfClMemFile(pd->confname, -1, vty, file,0,0,0))
                    {
                        vty_out(vty, "generate config file fail!\r\n");
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
                    vty_out(vty, "alloc file memory fail!\r\n");
                    ret = CMD_WARNING;
                }
                */

                vty_out(vty, "\r\n");
            }
        }
        ONU_CONF_SEM_GIVE
    }

    return ret;
}


DEFUN(show_onu_conf_name_by_id,
        show_onu_conf_name_by_id_cmd,
        "show profile name",
        SHOW_STR
        "profile\n"
        "name\n")
{
    int ret = CMD_SUCCESS;
    char *name = NULL;

    ONU_CONF_SEM_TAKE
    {
        if(vty->onuconfptr && onuConfCheckByPtrNoSuffix(vty->onuconfptr) == VOS_OK)
        {
			if(vty->node == ONU_PROFILE_NODE &&
				vty->orig_profileptr && 
				onuConfCheckByPtrNoSuffix(vty->orig_profileptr) == VOS_OK)
			{
				ONUConfigData_t *p = vty->orig_profileptr;
				name =p->confname;
			}
			else
			{
            	ONUConfigData_t *p = vty->onuconfptr;
	            name = p->confname;
			}
        }
        else
        {
            short int pon_id, onu_id, llid;
            if(parse_onu_command_parameter(vty, &pon_id, &onu_id, &llid) == VOS_OK)
            {
                name = getOnuConfNamePtrByPonId(pon_id, onu_id);
            }
            else
                ret = CMD_WARNING;
        }
    }
    ONU_CONF_SEM_GIVE

    if(name)
        vty_out(vty, "profile name: %s\r\n", name);

    return ret;
}

DEFUN (
    show_onu_version,
    show_onu_version_cmd,
    "show version",
    SHOW_STR
    "display the version\n"
    )
{

    int ponID = 0;
    int ret = CMD_SUCCESS;
    UCHAR ver[256]="";

    ULONG ulIfIndex = 0;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex = (ULONG) vty->index;
    ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
    if (ulRet != VOS_OK)
        return CMD_WARNING;

    ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
    if (ponID == (VOS_ERROR))
    {
        vty_out(vty, "  %% Parameter error\r\n");
        return CMD_WARNING;
    }


    if(OnuMgt_GetCtcVersion(ponID, ulOnuid-1, ver) == VOS_OK)
        vty_out(vty, "onu version:\t%d.%d(%02x)\r\n", ver[0]>>4, ver[0]&0xf, ver[0]);
    else
        vty_out(vty, "onu version get fail!\r\n");

    return ret;
}

DEFUN (
    onu_vlan_port_isolate,
    onu_vlan_port_isolate_cmd,
    "vlan port_isolate {[0|1]}*1",    
    "Show or config onu vlan information\n"
    "Show or config port isolate\n"
    "Disable port isolate\n"
    "Enable port isolate\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "vlan port_isolate",NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {

            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK != OnuMgt_SetPortIsolate(ponID, ulOnuid-1, VOS_StrToUL(argv[0], NULL, 10)) )
            {
#if 0                
                vty_out(vty, "vlan port_isolate set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                ULONG enable = 0;
                if (VOS_OK == getOnuConfSimpleVar(ponID, ulOnuid - 1, sv_enum_port_isolate, &enable))
                    vty_out(vty, "\r\nvlan port_isolate is %d\r\n", enable);
                else
                    vty_out(vty, "\r\nvlan port_isolate get error!\r\n");
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG enable= 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(argc == 1)
        {
            enable = VOS_StrToUL(argv[0], NULL, 10);

            if(setOnuConfSimpleVarByPtr(suffix, pd,sv_enum_port_isolate, enable))
                vty_out(vty, "vlan port_isolate set fail!\r\n");
        }
        else
        {
            if(getOnuConfSimpleVarByPtr(suffix, pd ,sv_enum_port_isolate, &enable))
                vty_out(vty, "vlan port_isolate get fail!\r\n");
            else
                vty_out(vty, "vlan port_isolate is %d\r\n",enable);
        }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%vlan port_isolate !\r\n");	


	return CMD_SUCCESS;
#endif
}

#if 0
#endif
/*vlan transation*/
DEFUN  (
    onu_vlan_translation_set,
    onu_vlan_translation_set_cmd,
    "vlan-translation <port_list> old-vid <2-4094> new-vid <2-4094>",
    "Config onu vlan translation table\n"
     "port number\n"
     "old-vid\n"
     "input old vid\n"
     "new-vid\n"
     "input new vid\n"
    )
{

    int ponID = 0, i;
    ULONG ulIfIndex = 0, port;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    ULONG portNum, inVid, newVid;
    ULONG devIdx, brdIdx, pon, onu;

    ulIfIndex = (ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
        if (ulRet != VOS_OK)
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
        if (ponID == (VOS_ERROR))
        {
            vty_out(vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)
		
        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {
            if (parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
                return CMD_WARNING;

            if ((getDeviceCapEthPortNum(devIdx, &portNum) != VOS_OK) || portNum > MAX_ETH_PORT_NUM)
            {
                vty_out(vty, "no ethernet port\r\n");
                return CMD_WARNING;
            }

            ONU_CONF_WRITEABLE_CHECK

            inVid = VOS_StrToUL(argv[1], NULL, 10);
            newVid = VOS_StrToUL(argv[2], NULL, 10);

            if ((inVid > 4094) || (inVid < 0) || (newVid > 4094) || (newVid < 0))
            {
                return CMD_WARNING;
            }

            if (VOS_StriCmp(argv[0], "all") == 0)
            {
                int error_flag = 0;
                for (i = 0; i < portNum; i++)
                {
                    if (VOS_OK != OnuMgt_SetEthPortVlanTran(ponID, ulOnuid - 1, i, inVid, newVid))
                    {
#if 0                        
                        vty_out(vty, "add vlan transation %d fail\r\n", inVid);
#else
                        error_flag = 1;
#endif
                    }
                }
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);                    
            }
            else
            {
                int error_flag = 0;
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                {
                    if (VOS_OK != OnuMgt_SetEthPortVlanTran(ponID, ulOnuid - 1, port, inVid, newVid))
                    {
#if 0                        
                        vty_out(vty, "add vlan transation %d fail\r\n", inVid);
#else
                        error_flag = 1;
#endif
                    }
                }END_PARSE_PORT_LIST_TO_PORT();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);                    
            }
        }
        else
        {
            char *pRecv;
            USHORT len;

            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
	            vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }
    else
    {
        int inVid = VOS_StrToUL(argv[1], NULL, 10);
        int newVid = VOS_StrToUL(argv[2], NULL, 10);
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
        {
           if (VOS_OK != set_OnuConf_Ctc_EthPortVlanTranNewVidByPtr(suffix,pd ,port, inVid, newVid))
               vty_out(vty, "add vlan translation %d fail\r\n", inVid);
        }
        END_PARSE_PORT_LIST_TO_PORT();
    }

    return CMD_SUCCESS;


}

DEFUN  (
    onu_vlan_translation_del,
    onu_vlan_translation_del_cmd,
    "vlan-translation-del <port_list> old-vid <2-4094>",
    "Config onu vlan translation table\n"
     "port number\n"
     "old-vid\n"
     "input old vid\n"
    )
{
    int ponID=0,i;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    ULONG portNum,oldvid;
    ULONG devIdx,brdIdx,pon,onu;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {
            if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		        return CMD_WARNING;
    
            if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		    portNum > MAX_ETH_PORT_NUM )
	        {
		        vty_out(vty, "no ethernet port\r\n");
		        return CMD_WARNING;
	        }

            ONU_CONF_WRITEABLE_CHECK

            oldvid = VOS_StrToUL(argv[1], NULL, 10);
     
	        if( (oldvid > 4094) || (oldvid < 0))
	        {
		        return CMD_WARNING;
	        }	


	        if(VOS_StriCmp(argv[0], "all")==0)
            {
                int error_flag = 0;
                for(i=0;i<portNum;i++)
                {
                    if(VOS_OK != OnuMgt_DelEthPortVlanTran(ponID, ulOnuid-1, i, oldvid))
                    {
#if 0                        
                          vty_out(vty, "del vlan transation %d fail\r\n", oldvid);
#else
                            error_flag = 1;
#endif
                    }
                }
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);                    
            }
            else
            {
                /*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
		        if(port>portNum)
		        {
			        vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);	
			        RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
		        }
		        END_PARSE_PORT_LIST_TO_PORT();*/	/* removed by xieshl 20120906, 解决内存丢失问题，同时非法端口不处理，但不报错，下同 */
                int error_flag = 0;
		        BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
		        if( port <= portNum )
		        {
		            if(VOS_OK != OnuMgt_DelEthPortVlanTran(ponID, ulOnuid-1, port, oldvid))
		            {
#if 0                        
                        vty_out(vty, "del vlan transation %d fail\r\n", oldvid);
#else
                        error_flag = 1;
#endif
		            }
		        }
		        END_PARSE_PORT_LIST_TO_PORT();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);
                    
            }
        }
        else
        {
            char *pRecv;
            USHORT len;

            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
	            vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }
    else
    {
	    ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
	    oldvid = VOS_StrToUL(argv[1], NULL, 10);

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
        
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
		{
		    if(VOS_OK != del_OnuConf_Ctc_EthPortVlanTranNewVidByPtr(suffix, pd, port, oldvid))
                vty_out(vty, "del vlan transation %d fail\r\n", oldvid);
		}
		END_PARSE_PORT_LIST_TO_PORT();

        
    }

    return CMD_SUCCESS;

}

DEFUN  (
    onu_vlan_translation_show,
    onu_vlan_translation_show_cmd,
    "show vlan-translation <port_list> ",
    SHOW_STR
    "Config onu vlan translation table\n"
     "port number\n"
    )
{

    int ponID = 0, i;
    ULONG ulIfIndex = 0, port;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    ULONG portNum;
    ULONG devIdx, brdIdx, pon, onu;

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
        if (ulRet != VOS_OK)
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
        if (ponID == (VOS_ERROR))
        {
            vty_out(vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if (parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;

        if ((getDeviceCapEthPortNum(devIdx, &portNum) != VOS_OK) || portNum > MAX_ETH_PORT_NUM)
        {
            vty_out(vty, "no ethernet port\r\n");
            return CMD_WARNING;
        }
        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {
            if (VOS_StriCmp(argv[0], "all") == 0)
            {
                for (i = 0; i < portNum; i++)
                {
                    CTC_STACK_port_vlan_configuration_t pvc;
                    if (VOS_OK == get_OnuConf_Ctc_portVlanConfig(ponID, ulOnuid - 1, i+1, &pvc))
                    {
                        int j;

                        if (pvc.mode != ONU_CONF_VLAN_MODE_TRANSLATION)
                            continue;
                        else
                        {

                            vty_out(vty, "port %d vlan translation table:\r\n\r\n", i + 1);

                            vty_out(vty, "\tinvid\toutvid\r\n");

                            for (j = 0; j < pvc.number_of_entries; j++)
                            {
                                vty_out(vty, "\t%-5d\t%-6d\r\n", pvc.vlan_list[2 * j], pvc.vlan_list[2 * j + 1]);
                            }
                        }

                    }
                    else
                        vty_out(vty, "get vlan transation fail\r\n");
                }
            }
            else
            {
                /*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
                                if (port > portNum)
                                {
                                    vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);
                                    return CMD_WARNING;
                                }END_PARSE_PORT_LIST_TO_PORT();*/

                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                                if( port <= portNum )
                                {
                                    CTC_STACK_port_vlan_configuration_t pvc;
                                    if (VOS_OK == get_OnuConf_Ctc_portVlanConfig(ponID, ulOnuid - 1, port, &pvc))
                                    {
                                        int j;

                                        if (pvc.mode == ONU_CONF_VLAN_MODE_TRANSLATION)
                                        {
                                            vty_out(vty, "port %d vlan translation table:\r\n\r\n", port);
                                            vty_out(vty, "\tinvid\toutvid\r\n");

                                            for (j = 0; j < pvc.number_of_entries; j++)
                                            {
                                                vty_out(vty, "\t%-5d\t%-6d\r\n", pvc.vlan_list[2 * j], pvc.vlan_list[2 * j + 1]);
                                            }
                                        }
                                    }
                                    else
                                        vty_out(vty, "get vlan transation fail\r\n");
                                }END_PARSE_PORT_LIST_TO_PORT();

            }
        }
        else
        {
            char *pRecv;
            USHORT len;

            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
	            vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
        
    }
    else
    {

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
	
		BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                        {
                            CTC_STACK_port_vlan_configuration_t pvc;
                            if (VOS_OK == get_OnuConf_Ctc_portVlanConfigByPtr(suffix, vty->onuconfptr, port, &pvc))
                            {
                                int j;

                                if (pvc.mode == ONU_CONF_VLAN_MODE_TRANSLATION)
                                {
                                    vty_out(vty, "port %d vlan translation table:\r\n\r\n", port);
                                    vty_out(vty, "\tinvid\toutvid\r\n");

                                    for (j = 0; j < pvc.number_of_entries; j++)
                                    {
                                        vty_out(vty, "\t%-5d\t%-6d\r\n", pvc.vlan_list[2 * j], pvc.vlan_list[2 * j + 1]);
                                    }
                                }
                            }
                            else
                                vty_out(vty, "get vlan transation fail\r\n");
             }END_PARSE_PORT_LIST_TO_PORT();
    }
    return CMD_SUCCESS;
}


/*vlan aggregation*/
DEFUN  (
    onu_vlan_aggregation_add,
    onu_vlan_aggregation_add_cmd,
    "vlan-aggregation <port_list> old-vid <vlan_list> target-vid <2-4094>",
    "Config onu vlan transation table\n"
     "port number\n"
     "old-vid\n"
     "input old vid list\n"
     "new-vid\n"
     "input target vid\n"
    )
{
    int ponID = 0, i;
    ULONG ulIfIndex = 0, port;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    ULONG portNum, tVid;
    USHORT vlanList[8];
    ULONG devIdx, brdIdx, pon, onu;

    ulIfIndex = (ULONG) vty->index;

    VOS_MemZero(vlanList, sizeof(vlanList));
    BEGIN_PARSE_VLAN_LIST_TO_VLAN(argv[1],vlanList)
    END_PARSE_VLAN_LIST_TO_VLAN();

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
        if (ulRet != VOS_OK)
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
        if (ponID == (VOS_ERROR))
        {
            vty_out(vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {
            if (parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
                return CMD_WARNING;

            if ((getDeviceCapEthPortNum(devIdx, &portNum) != VOS_OK) || portNum > MAX_ETH_PORT_NUM)
            {
                vty_out(vty, "no ethernet port\r\n");
                return CMD_WARNING;
            }

            ONU_CONF_WRITEABLE_CHECK

            tVid = VOS_StrToUL(argv[2], NULL, 10);

            if ((tVid > 4094) || (tVid < 0))
            {
                return CMD_WARNING;
            }

            if (VOS_StriCmp(argv[0], "all") == 0)
            {
                int error_flag = 0;
                for (i = 0; i < portNum; i++)
                {
                    if (VOS_OK != OnuMgt_SetEthPortVlanAgg(ponID, ulOnuid - 1, i, vlanList, tVid))
                    {
#if 0                        
                        vty_out(vty, "set target vlan %d aggregation  fail\r\n", tVid);
#else
                        error_flag = 1;
#endif
                    }
                }
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);                    
            }
            else
            {
                /*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
                        if (port > portNum)
                        {
                            vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);
                            return CMD_WARNING;
                        }END_PARSE_PORT_LIST_TO_PORT();*/
                int error_flag = 0;
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                if (port <= portNum)
                {
                    if (VOS_OK != OnuMgt_SetEthPortVlanAgg(ponID, ulOnuid - 1, port, vlanList, tVid))
                    {
#if 0                        
                        vty_out(vty, "set target vlan %d aggregation  fail\r\n", tVid);
#else
                        error_flag = 1;
#endif
                    }
                }END_PARSE_PORT_LIST_TO_PORT();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);
                    
            }
        }
        else
        {
            char *pRecv;
            USHORT len;

            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
	            vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }    
        }
    }
    else
    {
	    ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        int tVid = VOS_StrToUL(argv[2], NULL, 10);
        
        if ((tVid > 4094) || (tVid < 0))
        {
            return CMD_WARNING;
        }

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
		
        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
        {
            if (VOS_OK != set_OnuConf_Ctc_EthPortVlanAggByPtr(suffix, pd, port, vlanList, tVid))
                vty_out(vty, "set target vlan %d aggregation  fail\r\n", tVid);
        }
        END_PARSE_PORT_LIST_TO_PORT();

    }

    return CMD_SUCCESS;

}

DEFUN  (
    onu_vlan_aggregation_del,
    onu_vlan_aggregation_del_cmd,
    "vlan-aggregation-del <port_list> target-vid <2-4094>",
    "Config onu vlan aggregation table\n"
     "port number\n"
     "target-vid\n"
     "input target vid\n"
    )
{
    int ponID = 0, i;
    ULONG ulIfIndex = 0, port;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    ULONG portNum, tVid;

    ULONG devIdx, brdIdx, pon, onu;

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
        if (ulRet != VOS_OK)
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
        if (ponID == (VOS_ERROR))
        {
            vty_out(vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)
		
        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {
            if (parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
                return CMD_WARNING;

            if ((getDeviceCapEthPortNum(devIdx, &portNum) != VOS_OK) || portNum > MAX_ETH_PORT_NUM)
            {
                vty_out(vty, "no ethernet port\r\n");
                return CMD_WARNING;
            }

            ONU_CONF_WRITEABLE_CHECK

            tVid = VOS_StrToUL(argv[1], NULL, 10);

            if ((tVid > 4094) || (tVid < 0))
            {
                return CMD_WARNING;
            }

            if (VOS_StriCmp(argv[0], "all") == 0)
            {
                int error_flag = 0;
                for (i = 0; i < portNum; i++)
                {
                    if (VOS_OK != OnuMgt_DelEthPortVlanAgg(ponID, ulOnuid - 1, i, tVid))
                    {
#if 0                        
                        vty_out(vty, "del target vlan %d aggregation  fail\r\n", tVid);
#else
                        error_flag = 1;
#endif
                    }
                }
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);                    
            }
            else
            {
                /*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
                        if (port > portNum)
                        {
                            vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);
                            return CMD_WARNING;
                        }
                END_PARSE_PORT_LIST_TO_PORT();*/
                int error_flag = 0;
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                if (port <= portNum)
                {
                    if (VOS_OK != OnuMgt_DelEthPortVlanAgg(ponID, ulOnuid - 1, port, tVid))
                    {
#if 0                        
                        vty_out(vty, "set target vlan %d aggregation  fail\r\n", tVid);
#else
                        error_flag = 0;
#endif
                    }
                }
                END_PARSE_PORT_LIST_TO_PORT();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);
                    
            }
        }
        else
        {
            char *pRecv;
            USHORT len;

            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
	            vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }
    else
    {
        int tVid = VOS_StrToUL(argv[1], NULL, 10);
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if ((tVid > 4094) || (tVid < 0))
        {
            return CMD_WARNING;
        }

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
		
        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
        {
            if (VOS_OK != del_OnuConf_Ctc_EthPortVlanAggByPtr(suffix, pd, port, tVid))
                vty_out(vty, "set target vlan %d aggregation  fail\r\n", tVid);
        }
        END_PARSE_PORT_LIST_TO_PORT();
    }

    return CMD_SUCCESS;

}
/*qinq enable*/
DEFUN  (
    onu_qinq_vlan_tag_enable,
    onu_qinq_vlan_tag_enable_cmd,
    "qinq-config [pvlan|pport|disable] <port_list>",
    "config qinq vlan tag\n"
    "add tag based per-vlan\n"
    "add tag based per-port\n"
    "disable qinq\n"
    "port number\n"
    )
{
    int ponID = 0,enable, suffix;
    ULONG ulIfIndex = 0, port;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
    ULONG portNum;
    ULONG devIdx, brdIdx, pon, onu;
    int error_flag = 0;
    if (VOS_StrCmp(argv[0], "pvlan") == 0)
        enable = CTC_QINQ_MODE_PER_VLAN_C;
    else if(VOS_StrCmp(argv[0], "pport") == 0)
        enable = CTC_QINQ_MODE_PER_PORT_C;
    else
        enable = CTC_QINQ_MODE_NONE_C;

    ulIfIndex = (ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
        if (ulRet != VOS_OK)
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
        if (ponID == (VOS_ERROR))
        {
            vty_out(vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if (parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;

        if ((getDeviceCapEthPortNum(devIdx, &portNum) != VOS_OK) || portNum > MAX_ETH_PORT_NUM)
        {
            vty_out(vty, "no ethernet port\r\n");
            return CMD_WARNING;
        }

        /*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], port )
        if (port > portNum)
        {
            vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);
            return CMD_WARNING;
        }
        END_PARSE_PORT_LIST_TO_PORT();*/

        BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], port )
        if (port <= portNum)
        {
	        ulRet = OnuMgt_SetPortQinQEnable(ponID, ulOnuid - 1, port, enable);
	        if (ulRet != VOS_OK)
	        {
#if 0                
	            vty_out(vty, "set port %d qinq %s error! \r\n", port, argv[0]);
#else
                error_flag = 1;
#endif
	        }
        }
        END_PARSE_PORT_LIST_TO_PORT();
        if(error_flag)
            vty_out(vty, ONU_CMD_ERROR_STR);
            
    }
    else
    {

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[1], port)
	
              BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], port )

                        ulRet = set_OnuVlanTagConfigEnableByPtr(suffix, (void*)vty->onuconfptr, port, enable);
                        if (ulRet != VOS_OK)
                            vty_out(vty,"set port %d qinq %s error! \r\n", port, argv[0]);

             END_PARSE_PORT_LIST_TO_PORT();
    }

        return CMD_SUCCESS;
}

/*qinq vlan tag add*/
DEFUN  (
    onu_qinq_vlan_tag_add,
    onu_qinq_vlan_tag_add_cmd,
    "qinq-config vlan-tag-add <port_list> c-vid <1-4094> s-vid <1-4094>",
    "config qinq \n"
    "add vlan tag\n"
    "port number\n"
    "inner vlan tag\n"
    "inner vlan id <1-4094>\n"
    "outer vlan tag\n"
    "outer vlan id <1-4094>\n"
    )
  {
    int ponID = 0, suffix;
    ULONG ulIfIndex = 0, port;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
    ULONG portNum, cVid, sVid;

    ULONG devIdx, brdIdx, pon, onu;
    int error_flag = 0;
    ulIfIndex = (ULONG) vty->index;

    cVid = VOS_StrToUL(argv[1], NULL, 10);
    sVid = VOS_StrToUL(argv[2], NULL, 10);

    if (!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
        if (ulRet != VOS_OK)
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
        if (ponID == (VOS_ERROR))
        {
            vty_out(vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if (parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;

        if ((getDeviceCapEthPortNum(devIdx, &portNum) != VOS_OK) || portNum > MAX_ETH_PORT_NUM)
        {
            vty_out(vty, "no ethernet port\r\n");
            return CMD_WARNING;
        }


        ONU_CONF_WRITEABLE_CHECK

            /*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[0], port )
                            if (port > portNum)
                            {
                                vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);
                                return CMD_WARNING;
                            }END_PARSE_PORT_LIST_TO_PORT();*/

        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
        if (port <= portNum)
        {
            if (VOS_OK != OnuMgt_AddQinQVlanTag(ponID, ulOnuid - 1, port, cVid, sVid))
            {
#if 0                
                vty_out(vty, "set  vlan %d tag  %d fail \r\n", cVid, sVid);
#else
                error_flag = 1;
#endif            
            }
        }END_PARSE_PORT_LIST_TO_PORT();
        if(error_flag)
            vty_out(vty, ONU_CMD_ERROR_STR);
            
    }
    else
    {

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
        {
            if (VOS_OK != set_OnuVlanAddTagByPtr(suffix, (void*)vty->onuconfptr, port, cVid, sVid))
                vty_out(vty, "set  vlan %d tag  %d fail \r\n", cVid, sVid);

        }
        END_PARSE_PORT_LIST_TO_PORT();
    }

    return CMD_SUCCESS;

} 

/*qinq vlan tag del*/
DEFUN  (
    onu_qinq_vlan_tag_del,
    onu_qinq_vlan_tag_del_cmd,
    "qinq-config vlan-tag-del <port_list> {s-vid <1-4094>}*1",
    "config qinq \n"
    "del vlan tag\n"
    "port number\n"
    "outer vlan id\n"
    "vlan id <1-4094>\n"
    )
  {
    int ponID = 0;
    ULONG ulIfIndex = 0, port, suffix;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
    ULONG portNum, sVid;

    ULONG devIdx, brdIdx, pon, onu;
    int error_flag = 0;
    ulIfIndex = (ULONG) vty->index;

    if(argc == 2)
        sVid = VOS_StrToUL(argv[1], NULL, 10);
    else
        sVid = 0;

    if (!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
        if (ulRet != VOS_OK)
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
        if (ponID == (VOS_ERROR))
        {
            vty_out(vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if (parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;

        if ((getDeviceCapEthPortNum(devIdx, &portNum) != VOS_OK) || portNum > MAX_ETH_PORT_NUM)
        {
            vty_out(vty, "no ethernet port\r\n");
            return CMD_WARNING;
        }

        ONU_CONF_WRITEABLE_CHECK

            /*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
                            if (port > portNum)
                            {
                                vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);
                                return CMD_WARNING;
                            }END_PARSE_PORT_LIST_TO_PORT();*/

            if(sVid)
            {
                 BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                if (port <= portNum)
                {
                    if (VOS_OK != OnuMgt_DelQinQVlanTag(ponID, ulOnuid - 1, port, sVid))
                {
#if 0
                    vty_out(vty, "del vlan tag %d fail \r\n", sVid);
#else
                    error_flag = 0;
#endif                
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
            }
            else
            {
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                {
                    if(OnuMgt_SetPortQinQEnable(ponID, ulOnuid-1, port, CTC_QINQ_MODE_NONE_C) != VOS_OK)
                {
#if 0                    
                    vty_out(vty, "empty vlan tag for port %d fail!\r\n", port);
#else
                    error_flag = 1;
#endif
                }
            }END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);            
        }

    }
    else
    {

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
	
        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                        {
                            if (VOS_OK != set_OnuVlanDelTagByPtr(suffix, (void*)vty->onuconfptr, port, sVid))
                                vty_out(vty, "del vlan tag %d fail \r\n", sVid);

                        }END_PARSE_PORT_LIST_TO_PORT();

    }

    return CMD_SUCCESS;
} 

DEFUN  (
    onu_qinq_vlan_tag_config_show,
    onu_qinq_vlan_tag_config_show_cmd,
    "show qinq-config <port_list>",
    SHOW_STR
    "Config onu qinq  table\n"
     "port number\n"
    )
{

    int ponID = 0, i;
    ULONG ulIfIndex = 0, port, suffix;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
    ULONG portNum;
    ULONG devIdx, brdIdx, pon, onu;
    char *cstatus[]={"disable","pport","pvlan"};

    ulIfIndex = (ULONG) vty->index;
	if(!IsProfileNodeVty(ulIfIndex, &suffix))
	{
	    ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
	    if (ulRet != VOS_OK)
	        return CMD_WARNING;

	    ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
	    if (ponID == (VOS_ERROR))
	    {
	        vty_out(vty, "  %% Parameter error\r\n");
	        return CMD_WARNING;
	    }

	    if (parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
	        return CMD_WARNING;

	    if ((getDeviceCapEthPortNum(devIdx, &portNum) != VOS_OK) || portNum > MAX_ETH_PORT_NUM)
	    {
	        vty_out(vty, "no ethernet port\r\n");
	        return CMD_WARNING;
	    }

	    if (VOS_StriCmp(argv[0], "all") == 0)
	    {
            for (i = 0; i < portNum; i++)
            {
                CTC_STACK_port_qinq_configuration_t pvc;
                if (VOS_OK == get_OnuVlanTagConfig(ponID, ulOnuid-1, i+1, &pvc))
                {
                    int j;
                    if(pvc.mode)                    
                        vty_out(vty,"port %d: QinQ config is Enable, mode is %s\r\n", i+1, cstatus[pvc.mode]);
                    else
                        vty_out(vty,"port %d: QinQ config is Disable\r\n", i+1);
                    
                    if (pvc.mode != CTC_QINQ_MODE_PER_VLAN_C)
                        continue;
                    else
                    {

                        vty_out(vty, "vlan QinQ table:\r\n\r\n");

                        vty_out(vty, "\tinvid\toutvid\r\n");
                        for (j = 0; j < pvc.number_of_entries; j++)
                        {
                            vty_out(vty, "\t%-5d\t%-6d\r\n", pvc.vlan_list[2 * j], pvc.vlan_list[2 * j + 1]);
                        }
	                }

	            }
	            else
	                vty_out(vty, "get QinQ config fail\r\n");
	        }
	    }
	    else
	    {
			/*
	        BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
	                        if (port > portNum)
	                        {
	                            vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);
	                            return CMD_WARNING;
	                        }END_PARSE_PORT_LIST_TO_PORT();
	                        */

			VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                CTC_STACK_port_qinq_configuration_t pvc;
                if (VOS_OK == get_OnuVlanTagConfigByPtr(ponID, ulOnuid-1, port, &pvc))
                {
                    int j;
                    if(pvc.mode)                    
                        vty_out(vty,"port %d: QinQ config is Enable, mode is %s\r\n",port, cstatus[pvc.mode]);
                    else
                        vty_out(vty,"port %d: QinQ config is  Disable\r\n", port);

                    if (pvc.mode == CTC_QINQ_MODE_PER_VLAN_C)
                    {
                        vty_out(vty, "vlan QinQ table:\r\n\r\n");
                        vty_out(vty, "\tinvid\toutvid\r\n");

                        for (j = 0; j < pvc.number_of_entries; j++)
                        {
                            vty_out(vty, "\t%-5d\t%-6d\r\n", pvc.vlan_list[2 * j], pvc.vlan_list[2 * j + 1]);
                        }
                    }
                }
                else
                    vty_out(vty, "get QinQ config fail\r\n");
    		}
            END_PARSE_PORT_LIST_TO_PORT();
        }
	}
	else
	{
		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
        {
            CTC_STACK_port_qinq_configuration_t pvc;
            if (VOS_OK == get_OnuVlanTagConfigByPtr(suffix, vty->onuconfptr, port, &pvc))
            {
                int j;

                if(pvc.mode)                    
                    vty_out(vty,"port %d: QinQ config is Enable, mode is %s\r\n",port, cstatus[pvc.mode]);
                else
                    vty_out(vty,"port %d: QinQ config is Disable\r\n", port);
                	                                 
                if (pvc.mode == CTC_QINQ_MODE_PER_VLAN_C)
                {
                    vty_out(vty, "vlan QinQ table:\r\n\r\n");
                    vty_out(vty, "\tinvid\toutvid\r\n");

                    for (j = 0; j < pvc.number_of_entries; j++)
                    {
                    vty_out(vty, "\t%-5d\t%-6d\r\n", pvc.vlan_list[2 * j], pvc.vlan_list[2 * j + 1]);
                    }
                }

            }
            else
                vty_out(vty, "get QinQ config fail\r\n");   	                            
		}
        END_PARSE_PORT_LIST_TO_PORT();     		
	}

    return CMD_SUCCESS;

}



/*daya在2006-9-19去掉该命令*/
/*ATU*/
DEFUN (
    onu_atu_size,
    onu_atu_size_cmd,
    "atu size {[0|1|2]}*1",    
    "Show or config onu atu information\n"
    "Show or config atu size information\n"
    "256\n"
    "512\n"
    "1024\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "atu size",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
	
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu size !\r\n");	
	
	return CMD_SUCCESS;
#endif
}



DEFUN (
    onu_atu_aging,
    onu_atu_aging_cmd,
    "atu aging {[0|<15-3825>]}*1",    
    "Show or config onu atu information\n"
    "Show or config atu aging time information\n"
    "Never aging\n"
    "Please input the aging time(second)\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "atu aging",NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

    /*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {

            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK == OnuMgt_SetAtuAgingTime(ponID, ulOnuid-1, VOS_StrToUL(argv[0], NULL, 10)) )
            {
            }
            else
            {
#if 0                
                vty_out(vty, "atu aging set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                ULONG age = 0;
                if(getOnuConfSimpleVar(ponID, ulOnuid-1, sv_enum_atu_aging, &age) != VOS_OK)
                    vty_out(vty, "atu aging get fail\r\n\r\n");
                else
                    vty_out(vty, "atu aging %d\r\n", age);
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG age = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(argc == 1)
        {
            age = VOS_StrToUL(argv[0], NULL, 10);

            if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_atu_aging, age))
                vty_out(vty, "atu aging set fail!\r\n");
        }
        else
        {
            
            if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_atu_aging, &age))
                vty_out(vty, "atu aging  get fail!\r\n");
            else
                vty_out(vty, "atu aging %d\r\n",age);
        }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu aging !\r\n");	

	return CMD_SUCCESS;
#endif
}



/*由于daya目前版本尚不能解析多个端口输入，故帮助命令中暂时将e.g.1,3-4注释掉*/
/*
发件人: KD刘冬 
发送时间: 2007年11月20日 14:10
收件人: KD黄杨广; KD姚力; KD陈福军; KD盖鹏飞; KD张西昌
主题: 答复: URTracker 事务新建通知：5860.[GT812]：配置静态mac地址的命令中没有vid参数，只能配置vlan 1的mac

请OLT添加命令：（为了兼容以前的ONU，后两个参数可选）
 
DEFUN(onu_atu_static_add,
 onu_atu_static_add_cmd,
 "atu static_add <mac> [0|1] <portlist> {<1-4094>}*1 {<0-7>}*1",
 "Mac table config\n"
 "Show/Config static mac entry\n"
 "MAC address to add(i.e. aa.bb.cc.ee.00.11)\n"
 "Unicast address\n"
 "Multicast address\n"
 DescStringPortList
 "VLAN id\n"
 "MAC address's priority\n" )
*/
/*  chenfj 2007-11-20
    注:由于一个命令中不能实现两个可选参数，故将命令分为
    两个函数来实现
   */
DEFUN (
    onu_atu_static_add,
    onu_atu_static_add_cmd,
    "atu static_add <H.H.H> <port_list> {<1-4094>}*1",    
    "Show or config onu atu information\n"
    "Add static mac\n"
    "Please input the mac address\n"
    "Please input the port_list\n"
    "VLAN id\n"
    /*"Please input the port_list. e.g.1,3-4\n"*/
    )
{
	LONG lRet;
	CHAR pBuff[256];   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length = 0;
	ULONG ulRet;
	CHAR MacAddr[6] = {0,0,0,0,0,0};
	ULONG ulFePort = 0;
	


	if( check_onu_cli_is_deny(vty) )	/* modified by xieshl 20100521, 问题单10260 */
		return CMD_SUCCESS;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;
 
	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       } 

	/*added by wutw 2006/11/10*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 1 ], ulFePort )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulFePort,vty))
		{
			RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();

	/*modified by wutw at 20 October*/
	VOS_Sprintf(pBuff,"atu static_add ");
	length = strlen(pBuff);

	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}
	VOS_Sprintf(&(pBuff[length]),"%02x:%02x:%02x:%02x:%02x:%02x 0 %s",MacAddr[0],\
				MacAddr[1],MacAddr[2],MacAddr[3],MacAddr[4],MacAddr[5],argv[1]);
	length = strlen(pBuff);
	if( argc == 3 )
		VOS_Sprintf(&(pBuff[length])," %s",argv[2]);
		
	length = strlen(pBuff);
	/*vty_out( vty, " %s\r\n",pBuff);
	vty_out( vty, "length %d\r\n",length);*/
	
	/*VOS_MemCpy( pBuff, vty->buf, vty->length);*/
	/*length = vty->length;	*/
#ifdef __RPC_ONU_COMMAND
	{
		char *szRecv = NULL;
		if(OnuMgt_CliCall(ponID, ulOnuid-1, pBuff, length, &szRecv, &length) == VOS_OK && szRecv && length)
		    vty_big_out(vty, length+4, "\r\n%s\r\n", szRecv);
	  	/*vty_out(vty, "\r\n%s\r\n", stPayload);*/
		else
		    vty_out(vty, "  %%atu static_add failed!\r\n");
	}
#else
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu static_add failed!\r\n");
#endif

	return CMD_SUCCESS;
}

DEFUN (
    onu_atu_static_add_dup,
    onu_atu_static_add__dup_cmd,
    "atu static_add <H.H.H> <port_list> <1-4094> {<0-7>}*1",    
    "Show or config onu atu information\n"
    "Add static mac\n"
    "Please input the mac address\n"
    "Please input the port_list\n"
    "VLAN id\n"
    "MAC address's priority\n"
    /*"Please input the port_list. e.g.1,3-4\n"*/
    )
{
	LONG lRet;
	CHAR pBuff[256];   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length = 0;
	ULONG ulRet;
	CHAR MacAddr[6] = {0,0,0,0,0,0};
	ULONG ulFePort = 0;
	


	if( check_onu_cli_is_deny(vty) )	/* modified by xieshl 20100521, 问题单10260 */
		return CMD_SUCCESS;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;
 
	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       } 

	/*added by wutw 2006/11/10*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 1 ], ulFePort )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulFePort,vty))
		{
			RETURN_PARSE_PORT_LIST_TO_PORT (CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();

	/*modified by wutw at 20 October*/
	VOS_Sprintf(pBuff,"atu static_add ");
	length = strlen(pBuff);

	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}
	VOS_Sprintf(&(pBuff[length]),"%02x:%02x:%02x:%02x:%02x:%02x 0 %s %s",MacAddr[0],\
				MacAddr[1],MacAddr[2],MacAddr[3],MacAddr[4],MacAddr[5],argv[1],argv[2]);
	length = strlen(pBuff);
	if( argc == 4 )
		VOS_Sprintf(&(pBuff[length])," %s",argv[3]);
		
	length = strlen(pBuff);
	/*vty_out( vty, " %s\r\n",pBuff);
	vty_out( vty, "length %d\r\n",length);*/
	
	/*VOS_MemCpy( pBuff, vty->buf, vty->length);*/
	/*length = vty->length;	*/
#ifdef __RPC_ONU_COMMAND
{
	char *szRecv = NULL;
    if(OnuMgt_CliCall(ponID, ulOnuid-1, pBuff, length, &szRecv, &length) == VOS_OK && szRecv && length)
         vty_big_out(vty, length+4, "\r\n%s\r\n", szRecv);
  		/*vty_out(vty, "\r\n%s\r\n", stPayload);*/
    else
        vty_out(vty, "  %%atu static_add failed!\r\n");
}
#else
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu static_add failed!\r\n");	
#endif
	return CMD_SUCCESS;
}

/* modified by chenfj 2007-11-22
	按刘冬要求，在命令中增加可选参数: {<1-4094>}*1
	*/
DEFUN (
    onu_atu_static_del,
    onu_atu_static_del_cmd,
    "atu static_del <H.H.H> {<1-4094>}*1",    
    "Show or config onu atu information\n"
    "Delete static mac\n"
    "Please input the mac address\n"
    "the vlan id\n"
    )
{
	LONG lRet;
	CHAR pBuff[256];   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    CHAR MacAddr[6] = {0,0,0,0,0,0};	


	if( check_onu_cli_is_deny(vty) )	/* modified by xieshl 20100521, 问题单10260 */
		return CMD_SUCCESS;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	
	/*modified by wutw at 20 October*/
	VOS_Sprintf(pBuff,"atu static_del ");
	length = strlen(pBuff);

	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}
	VOS_Sprintf(&(pBuff[length]),"%02x:%02x:%02x:%02x:%02x:%02x",MacAddr[0],\
				MacAddr[1],MacAddr[2],MacAddr[3],MacAddr[4],MacAddr[5]);
	length = strlen(pBuff);
	if( argc == 2 )
		VOS_Sprintf(&(pBuff[length])," %s", argv[1] );
	length = strlen(pBuff);
	
	/*VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	*/
#ifdef __RPC_ONU_COMMAND
{
	char *szrecv = NULL;
    if(OnuMgt_CliCall(ponID, ulOnuid-1, pBuff, length, &szrecv, &length) == VOS_OK && szrecv && length)
        vty_big_out(vty, length+4, "\r\n%s\r\n", szrecv);
		/*vty_out(vty, "\r\n%s\r\n", stPayload);*/
    else
        vty_out(vty, "  %%atu static_del!\r\n");
}
#else
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu static_del!\r\n");	
#endif
	return CMD_SUCCESS;
}


#if 0
/*marvell show dynamic*/
DEFUN  (
    onu_atu_dynamic_show,
    onu_atu_dynamic_show_cmd,
    "marvell show",    
    "Show onu atu dynamic information\n"
    "Show onu atu dynamic information\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "marvell show",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu dynamic_show !\r\n");	


	return CMD_SUCCESS;
#endif
}
#endif


DEFUN  (
    onu_atu_static_show,
    onu_atu_static_show_cmd,
    "atu show {static}*1",    
    "Show onu atu information\n"
    "Show onu atu information\n"
    "Show onu static atu information\n"

    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "atu show",NULL );
#elif defined __RPC_ONU_COMMAND
	return rpc_onu_dev_cli_process( vty, "atu show",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }

    if( vty->length > 255 )
    {
	    ASSERT(0);
		return CMD_WARNING;
    }
		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	if( stPayload == NULL )
	{
	    ASSERT( 0 );
		return CMD_WARNING;
	}
	VOS_MemSet(stPayload, 0, sizeof(struct cli_payload));
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu static_show !\r\n");	


	return CMD_SUCCESS;
#endif
}

#if 0
#endif

/*Event/Alarm*/
/* 2**64-1=18446744073709551615 */
DEFUN  (
    onu_event_oam_sym_alarm,
    onu_event_oam_sym_alarm_cmd,
    "event oam_alarm sym {[0|1] <window> <threshold>}*1",   
    "Show or config onu event oam alarm information\n"
    "Show or config onu event oam alarm information\n"
     "Show or config onu event oam alarm sym information\n"
     "0: Disable\n"
     "1: Enable\n"
     "0-2**64 \n"
     "0-2**64-1\n"
     ) 
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "event oam_alarm",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu static_show !\r\n");	


	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_event_oam_frm_alarm,
    onu_event_oam_frm_alarm_cmd,
    "event oam_alarm frm {[0|1] <window> <threshold>}*1",   
    "Show or config onu event oam alarm information\n"
    "Show or config onu event oam alarm information\n"
     "Show or config onu event oam alarm frm information\n"
     "0: Disable\n"
     "1: Enable\n"
     "0-0xFFFFFFFF \n"
     "0-0xFFFFFFFF\n"
     ) 
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "event oam_alarm",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu static_show !\r\n");	


	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_event_oam_frp_alarm,
    onu_event_oam_frp_alarm_cmd,
    "event oam_alarm frp {[0|1] <window> <threshold>}*1",   
    "Show or config onu event oam alarm information\n"
    "Show or config onu event oam alarm information\n"
     "Show or config onu event oam alarm frp information\n"
     "0: Disable\n"
     "1: Enable\n"
     "0-0xFFFFFFFF \n"
     "0-0xFFFFFFFF\n"
     ) 
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "event oam_alarm",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu static_show !\r\n");	


	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_event_oam_sec_alarm,
    onu_event_oam_sec_alarm_cmd,
    "event oam_alarm sec {[0|1] <window> <threshold>}*1",   
    "Show or config onu event oam alarm information\n"
    "Show or config onu event oam alarm information\n"
     "Show or config onu event oam alarm sec information\n"
     "0: Disable\n"
     "1: Enable\n"
     "0-0xFFFF \n"
     "0-0xFFFF\n"
     ) 
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "event oam_alarm",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%atu static_show !\r\n");	


	return CMD_SUCCESS;
#endif
}


DEFUN  (
    onu_event_show,
    onu_event_show_cmd,
    "event show",    
    /*"event show <1-1000>",  */
    "Show or config onu event information\n"
    "Show onu event information\n"
    "Please input the count number\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "event show",NULL );
#elif defined __RPC_ONU_COMMAND

	return rpc_onu_dev_cli_process( vty, "event show",NULL );

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%event show !\r\n");	


	return CMD_SUCCESS;
#endif
}


/*vlan*/
LDEFUN  (
    onu_vlan_enable,
    onu_vlan_enable_cmd,
    "vlan dot1q {[0|1]}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu dot1q vlan\n"
    "Disable 802.1Q vlan\n"
    "Enable 802.1Q Vlan\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_vlan_pvid,
    onu_vlan_pvid_cmd,
    "vlan pvid <port_list> {<1-4094>}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu vlan pvid\n"
    "Please input the port number\n"
    "Please input the pvid(1-4094)\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_vlan_forward_type,
    onu_vlan_forward_type_cmd,
    "vlan acceptable_frame_types <port_list> {[tagged|all]}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu vlan forward type\n"
    "Please input the port number\n"
    "Only forward tagged frames\n"
    "Forward untagged and tagged frames\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


LDEFUN  (
    onu_vlan_ingress_filter,
    onu_vlan_ingress_filter_cmd,
    "vlan ingress_filtering <port_list> {[0|1]}*1",    
    "Show or config onu vlan information\n"
    "Show or config onu vlan ingress filter\n"
    "Please input the port number\n"
    "Disable ingress filter\n"
    "Enable ingress filter\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


LDEFUN  (
    onu_vlan_dot1q_add,
    onu_vlan_dot1q_add_cmd,
    "vlan dot1q_add <2-4094>",    
    "Show or config onu vlan information\n"
    "Add 802.1Q Vlan\n"
    "Please input the Vlan Vid(2-4094)\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


LDEFUN  (
    onu_vlan_dot1q_port_add,
    onu_vlan_dot1q_port_add_cmd,
    "vlan dot1q_port_add <2-4094> <port_list> [1|2]",    
    "Show or config onu vlan information\n"
    "Add 802.1Q Vlan port\n"
    "Please input the Vlan Vid(2-4094)\n"
    "Please input the port number\n"
    "Tagged\n"
    "Untagged\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}




LDEFUN  (
    onu_vlan_dot1q_del,
    onu_vlan_dot1q_del_cmd,
    "vlan dot1q_del <2-4094>",    
    "Show or config onu vlan information\n"
    "Delete 802.1Q Vlan\n"
    "Please input the Vlan Vid\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


LDEFUN (
    onu_vlan_dot1q_port_del,
    onu_vlan_dot1q_port_del_cmd,
    "vlan dot1q_port_del <2-4094> <port_list>",    
    "Show or config onu vlan information\n"
    "Delete 802.1Q Vlan port\n"
    "Please input the Vlan Vid\n"
    "Please input the port number\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}



LDEFUN (
    onu_vlan_dot1q_show,
    onu_vlan_dot1q_show_cmd,
    "vlan dot1q_show {<1-4094>}*1",    
    "Show or config onu vlan information\n"
    "Show 802.1Q Vlan\n"
    "Please input the Vlan Vid\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}



LDEFUN (
    onu_vlan_port_isolate,
    onu_vlan_port_isolate_cmd,
    "vlan port_isolate {[0|1]}*1",    
    "Show or config onu vlan information\n"
    "Show or config port isolate\n"
    "Disable port isolate\n"
    "Enable port isolate\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

#if 0
#endif


/*daya在2006-9-19去掉该命令*/
/*ATU*/
LDEFUN (
    onu_atu_size,
    onu_atu_size_cmd,
    "atu size {[0|1|2]}*1",    
    "Show or config onu atu information\n"
    "Show or config atu size information\n"
    "256\n"
    "512\n"
    "1024\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}



LDEFUN (
    onu_atu_aging,
    onu_atu_aging_cmd,
    "atu aging {[0|<15-3825>]}*1",    
    "Show or config onu atu information\n"
    "Show or config atu aging time information\n"
    "Never aging\n"
    "Please input the aging time(second)\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}



/*由于daya目前版本尚不能解析多个端口输入，故帮助命令中暂时将e.g.1,3-4注释掉*/
/*
发件人: KD刘冬 
发送时间: 2007年11月20日 14:10
收件人: KD黄杨广; KD姚力; KD陈福军; KD盖鹏飞; KD张西昌
主题: 答复: URTracker 事务新建通知：5860.[GT812]：配置静态mac地址的命令中没有vid参数，只能配置vlan 1的mac

请OLT添加命令：（为了兼容以前的ONU，后两个参数可选）
 
DEFUN(onu_atu_static_add,
 onu_atu_static_add_cmd,
 "atu static_add <mac> [0|1] <portlist> {<1-4094>}*1 {<0-7>}*1",
 "Mac table config\n"
 "Show/Config static mac entry\n"
 "MAC address to add(i.e. aa.bb.cc.ee.00.11)\n"
 "Unicast address\n"
 "Multicast address\n"
 DescStringPortList
 "VLAN id\n"
 "MAC address's priority\n" )
*/
/*  chenfj 2007-11-20
    注:由于一个命令中不能实现两个可选参数，故将命令分为
    两个函数来实现
   */
LDEFUN (
    onu_atu_static_add,
    onu_atu_static_add_cmd,
    "atu static_add <H.H.H> <port_list> {<1-4094>}*1",    
    "Show or config onu atu information\n"
    "Add static mac\n"
    "Please input the mac address\n"
    "Please input the port_list\n"
    "VLAN id\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN (
    onu_atu_static_add_dup,
    onu_atu_static_add__dup_cmd,
    "atu static_add <H.H.H> <port_list> <1-4094> {<0-7>}*1",    
    "Show or config onu atu information\n"
    "Add static mac\n"
    "Please input the mac address\n"
    "Please input the port_list\n"
    "VLAN id\n"
    "MAC address's priority\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

/* modified by chenfj 2007-11-22
	按刘冬要求，在命令中增加可选参数: {<1-4094>}*1
	*/
LDEFUN (
    onu_atu_static_del,
    onu_atu_static_del_cmd,
    "atu static_del <H.H.H> {<1-4094>}*1",    
    "Show or config onu atu information\n"
    "Delete static mac\n"
    "Please input the mac address\n"
    "the vlan id\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}



LDEFUN  (
    onu_atu_static_show,
    onu_atu_static_show_cmd,
    "atu show {static}*1",    
    "Show onu atu information\n"
    "Show onu atu information\n"
    "Show onu static atu information\n",
    ONU_NODE

    )
{
	return CMD_SUCCESS;
}

#if 0
#endif

/*Event/Alarm*/
/* 2**64-1=18446744073709551615 */
LDEFUN  (
    onu_event_oam_sym_alarm,
    onu_event_oam_sym_alarm_cmd,
    "event oam_alarm sym {[0|1] <window> <threshold>}*1",   
    "Show or config onu event oam alarm information\n"
    "Show or config onu event oam alarm information\n"
     "Show or config onu event oam alarm sym information\n"
     "0: Disable\n"
     "1: Enable\n"
     "0-2**64 \n"
     "0-2**64-1\n",
    ONU_NODE
     ) 
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_event_oam_frm_alarm,
    onu_event_oam_frm_alarm_cmd,
    "event oam_alarm frm {[0|1] <window> <threshold>}*1",   
    "Show or config onu event oam alarm information\n"
    "Show or config onu event oam alarm information\n"
     "Show or config onu event oam alarm frm information\n"
     "0: Disable\n"
     "1: Enable\n"
     "0-0xFFFFFFFF \n"
     "0-0xFFFFFFFF\n",
    ONU_NODE
     ) 
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_event_oam_frp_alarm,
    onu_event_oam_frp_alarm_cmd,
    "event oam_alarm frp {[0|1] <window> <threshold>}*1",   
    "Show or config onu event oam alarm information\n"
    "Show or config onu event oam alarm information\n"
     "Show or config onu event oam alarm frp information\n"
     "0: Disable\n"
     "1: Enable\n"
     "0-0xFFFFFFFF \n"
     "0-0xFFFFFFFF\n",
    ONU_NODE
     ) 
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_event_oam_sec_alarm,
    onu_event_oam_sec_alarm_cmd,
    "event oam_alarm sec {[0|1] <window> <threshold>}*1",   
    "Show or config onu event oam alarm information\n"
    "Show or config onu event oam alarm information\n"
     "Show or config onu event oam alarm sec information\n"
     "0: Disable\n"
     "1: Enable\n"
     "0-0xFFFF \n"
     "0-0xFFFF\n",
    ONU_NODE
     ) 
{
	return CMD_SUCCESS;
}


LDEFUN  (
    onu_event_show,
    onu_event_show_cmd,
    "event show",    
    /*"event show <1-1000>",  */
    "Show or config onu event information\n"
    "Show onu event information\n"
    "Please input the count number\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}





#if 0
#endif

/*Bandwidth*/
DEFUN  (
    onu_band_set,
    onu_band_set_cmd,
    "bandwidth set [down|up] <0-7> {[0|1] <100-1000000> <0-8388607>}*1",    
    "Show or config onu Bandwidth information\n"
    "Show or config onu Bandwidth information\n"
    "Down stream policer\n"
    "Up stream policer\n"
    "Please input the prio <0-7>.When up diretion,prio value range:0-3.\n"
    "Disable policer \n"
    "Enable policer \n"
    "Please input the bandwidth (100-1000000 Kbits/s)\n"
    "Please input the burst (0-8388607 bytes)\n"
    ) /*bandwidth need to add a analyse what(<0.1-100>) you input  added by suipl 2006/09/05*/
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "bandwidth set",NULL );
#elif defined __RPC_ONU_COMMAND
	return rpc_onu_dev_cli_process( vty, "bandwidth set",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	ULONG prio = 0;
	cliPayload *stPayload=NULL;

    if ( !VOS_StrCmp( argv[ 0 ], "up" ) )
    {
        prio = ( ULONG ) VOS_AtoL( argv[ 1 ] );
		if ( prio > 3 )
		{
		vty_out( vty, "  %% Parameter error.The up diretion,prio is 0-3.\r\n");
		return CMD_WARNING;
		}
    }

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%bandwidth set !\r\n");	


	return CMD_SUCCESS;
#endif
}

/*已经注销*/

/* 问题单7099   chenfj  2008-10-10

摘要： 针对广播报文的限速使用另一个命令 
内容： 
bandwidth broadcast <portlist> {[0|<64-1000000>]}*1

GT815可以分别限制广播和总流量。（该芯片每个端口有两个限速器）
*/

/*    
	modified by chenfj 2008-10-21
	GT815 V1R00B008 版本修改后，使用port ingress_rate命令可以生效. 故放弃该命令
*/
#if 0
DEFUN  (
    onu_band_broadcast_set,
    onu_band_broadcast_set_cmd,
    "bandwidth broadcast <portlist> {[0|<64-1000000>]}*1",    
    "Show or config onu Bandwidth information\n"
    "Show or config onu Bandwidth broadcast information\n"
    "Please input the port number\n"
    "Limit all broadcast frames\n"
    "broadcast frames rate\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "bandwidth broadcast" );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }


	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;



	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%bandwidth broadcast !\r\n");	


	return CMD_SUCCESS;
#endif
}
#endif

/*FE port config*/
DEFUN  (
    onu_fe_port_en_set,
    onu_fe_port_en_set_cmd,
    "port en <port_list> {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port EN information\n"
    "Please input the port number\n"
    "Disable\n"
    "Enable\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "port en" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;

    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid, suffix;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;

    if(IsProfileNodeVty(ulIfIndex, &suffix))
    {

        ONUConfigData_t *pd = (ONUConfigData_t*) vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        if (argc == 2)
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_enable, VOS_StrToUL(argv[1], NULL, 10)) != VOS_OK)
                    vty_out(vty, "port %d %s FAIL!\r\n", port, VOS_StrToUL(argv[1], NULL, 10) ? "enable" : "disable");
            }END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                ULONG en = 0;
                if(getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_enable, &en) == VOS_OK)
                                vty_out(vty, "port %d %s\r\n", port,  en? "enable" : "disable");
                else
                    vty_out(vty, "port %d get enable fail\r\n", port);
            }END_PARSE_PORT_LIST_TO_PORT()
        }

    }
    else
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {

            ONU_CONF_WRITEABLE_CHECK

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                if( OnuMgt_SetEthPortAdminStatus(ponID, ulOnuid-1, port, VOS_StrToUL(argv[1], NULL, 10)) != VOS_OK)
                {
#if 0                    
                    vty_out(vty, "port enable fail\r\n");
#else
                    error_flag = 1;
#endif
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);                
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                    int en = 0;
                    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                    {
                        /*if(getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_enable, &en) == VOS_OK)*/
                        if(OnuMgt_GetEthPortAdminStatus(ponID, ulOnuid-1, port, &en) == VOS_OK)
                            vty_out(vty, "\r\nport %d %s\r\n", port, en?"enable":"disable");
                        else
                    {
#if 0                            
                        vty_out(vty, "\r\nport enable status get fail!\r\n");
#else
                        error_flag = 1;
#endif
                    }
                }
                END_PARSE_PORT_LIST_TO_PORT()
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);                    
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port en !\r\n");	


	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_fe_port_desc_set,
    onu_fe_port_desc_set_cmd,
    "port desc <port_list> {<string>}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port specific description information\n"
    "Please input the port number\n"
    "Please input the descriptor\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "port desc" );
#elif defined __RPC_ONU_COMMAND
	return rpc_onu_fe_cli_process( vty, argv[0], "port desc" );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();
	
	if (vty->length > 255)
	{
		vty_out( vty, "  %% Parameter is error.Character string is too long.\r\n" );
		return CMD_WARNING;
	}		
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port desc !\r\n");	

	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_fe_port_mode_set,
    onu_fe_port_mode_set_cmd,
    "port mode <port_list> {[0|8|9|10|11|12]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port mode information\n"
    "Please input the port number\n"
    "Auto negotiation\n"
    "100M/FD\n"
    "100M/HD\n"
    "10M/FD\n"
    "10M/HD\n"
    "1000M/FD\n"
    /*"1000M/FD,only for "DEVICE_TYPE_NAME_GT816_STR"\n"*/
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "port mode" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port, suffix;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {

            ULONG mode = VOS_StrToUL(argv[1], NULL, 10);

            /*mode = mode?0:1;*/

            ONU_CONF_WRITEABLE_CHECK

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                if(VOS_OK == OnuMgt_SetPortMode(ponID, ulOnuid-1, port, mode) )
                {
                }
                else
                {
#if 0                    
                    vty_out(vty, "port mode set error!\r\n");
#else
                    error_flag = 1;
#endif
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                

        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                int mode = 0;
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            	{
                        /*if(VOS_OK == getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_mode, &mode))*/
                    if(VOS_OK == OnuMgt_GetEthPortAutoNegotiationAdmin(ponID, ulOnuid-1, port, &mode))
                    {
                        if(mode)
                        	vty_out(vty, "  Port 1/%d:    AutoNegotiation enabled\r\n", port);
                        else
                        	vty_out(vty, "  Port 1/%d:    AutoNSegotiation disabled\r\n", port);
                    }
                    else
                    {
#if 0                        
                        vty_out(vty, "\r\n  port mode get error!\r\n");
#else
                        error_flag = 1;
#endif
                    }
         		}
                END_PARSE_PORT_LIST_TO_PORT()
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);                    

            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG mode = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
		
        if(argc == 2)
        {
            mode = VOS_StrToUL(argv[1], NULL, 10);

            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_mode, mode))
                vty_out(vty, "port %d mode set fail!\r\n", port);

            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(getOnuConfPortSimpleVarByPtr(suffix, pd , port, sv_enum_port_mode, &mode))
                vty_out(vty, "port %d mode get fail!\r\n", port);
            else
                vty_out(vty, "port %d mode %d\r\n", port ,mode);

            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port mode !\r\n");	

	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_fe_port_an_rst,
    onu_fe_port_an_rst_cmd,
    "port an restart <port_list> ",
    "Show or config onu FE port information\n"
    "Show or config onu FE port auto negotiation\n"
    "restart\n"
    "Please input the port number\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "port an restart" );
#elif defined __RPC_ONU_COMMAND
    int ponID=0;
    ULONG   ulIfIndex = 0, port, suffix;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {

		    BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
		    {
			    if(VOS_OK == OnuMgt_SetEthPortAutoNegotiationRestart(ponID, ulOnuid-1, port) )
			    {
			    }
			    else
			    {
#if 0                    
				    vty_out(vty, "port %d an restart FAIL!\r\n", port);
#else
                    error_flag = 1;
#endif
			    }
		    }
		    END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
        }
        else
        {
            char *pRecv;
            USHORT len;

            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
		            vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }

        return CMD_SUCCESS;
    }

    return CMD_WARNING;
#endif
}

DEFUN  (
    onu_fe_port_pause_set,
    onu_fe_port_pause_set_cmd,
    "port pause <port_list> {[1|0]}*1",
    "Show or config onu FE port information\n"
    "Show or config onu FE port pause\n"
    "Please input the port number\n"
    "enable\n"
    "disable\n"
    )
{
    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid, suffix;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {
            ONU_CONF_WRITEABLE_CHECK

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                ULONG enable = VOS_StrToUL(argv[1], NULL, 10);
                if(VOS_OK == OnuMgt_SetEthPortPause(ponID, ulOnuid-1, port, enable) )
                {
                }
                else
                {
#if 0                    
                    vty_out(vty, "port %d pause %s error!\r\n", port, enable?"enable":"disable");
#else
                    error_flag = 1;
#endif
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                    {
                        int enable = 0;
                        /*if(getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_pause_enable, &enable) == VOS_OK)*/
                        if(VOS_OK == OnuMgt_GetEthPortPause(ponID, ulOnuid-1, port, &enable))
                            vty_out(vty, "\r\nport %d pause %s\r\n", port, enable?"enable":"disable");
                    }
                    END_PARSE_PORT_LIST_TO_PORT()
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        ULONG en = 0;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        if(argc == 2)
        {

            en = VOS_StrToUL(argv[1], NULL, 10);
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {

                if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_pause_enable, en) != VOS_OK)
                    vty_out(vty, "port %d pause %s error!\r\n", port, en?"enable":"disable");
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_pause_enable, &en))
                    vty_out(vty, "port %d pause enable get error!\r\n", port);
                else
                    vty_out(vty, "port %d pause %s \r\n", port, en?"enable":"disable");
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;
}

DEFUN  (
    onu_fe_port_negoration_show,
    onu_fe_port_negoration_show_cmd,
    "port mode_show <port_list>",    
    "Show or config onu FE port information\n"
    "Show negoration of onu FE port mode\n"
    "Please input the port number\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "port mode_show" );
#elif defined __RPC_ONU_COMMAND
	return rpc_onu_fe_cli_process( vty, argv[0], "port mode_show" );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length = 0;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING ); 
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port mode !\r\n");	

	return CMD_SUCCESS;
#endif
}



DEFUN  (
    onu_fe_port_fc_set,
    onu_fe_port_fc_set_cmd,
    "port fc <port_list> {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port fc information\n"
    "Please input the port number\n"
    "Disable\n"
    "Enable\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "port fc" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port, suffix;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {

            ONU_CONF_WRITEABLE_CHECK

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                if(VOS_OK == OnuMgt_SetPortFcMode(ponID, ulOnuid-1, port, VOS_StrToUL(argv[1], NULL, 10)) )
                {
                }
                else
                {
#if 0                    
                    vty_out(vty, "\r\nport %d FEC %s FAIL!\r\n", port, VOS_StrToUL(argv[1], NULL, 10)?"enable":"disable");
#else
                    error_flag = 1;
#endif
                }

            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                    {
                        ULONG en = 0;
                        if(getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_fec_enable, &en) == VOS_OK)
                               vty_out(vty, "\r\nport %d fec mode is %s\r\n", port, en?"enable":"disable");
                        else
                            vty_out(vty, "\r\nport %d fec mode get fail!\r\n", port);

                    }
                    END_PARSE_PORT_LIST_TO_PORT()
            }
            else
            {
                    char *pRecv;
                    USHORT len;

                    if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                    {
			            vty_big_out(vty, len, "%s", pRecv);
                    }
                    else
                    {
                        vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                    }
            }
        }
    }
    else
    {
        ULONG en = 0;
        ONUConfigData_t *pd = (ONUConfigData_t *)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        if(argc == 2)
        {
            en = VOS_StrToUL(argv[1], NULL, 10);
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_fec_enable, en)!= VOS_OK)
                    vty_out(vty, "port %d fc %s set fail\r\n", port, en?"enable":"disable");
            }
                    END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_fec_enable, &en)!=VOS_OK)
                    vty_out(vty, "port %d fc get fail\r\n");
                else
                    vty_out(vty, "port %d fc %s\r\n",port, en?"enable":"disable");
            }
                    END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port fc !\r\n");	

	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_fe_port_rate_set,
    onu_fe_port_rate_set_cmd,
    "port rate <port_list> [1|2] {<rate>}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the port number\n"
    "Ingress\n"
    "Egress\n"
    "Please input the rate( 0.1-100 Mbps)\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "port rate" );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
    {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
    }

	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port rate  !\r\n");	

	return CMD_SUCCESS;
#endif
}

/* modified by chenfj 2007-10-23
    修改ONU命令，增加Burst mode 参数*/
   
 /*
发件人: KD刘冬 
发送时间: 2007年11月15日 10:50
收件人: KD陈福军
主题: 请修改port ingress_rate命令

增加了drop|pause参数，做到与大亚原来的命令兼容；

*/

DEFUN  (
    onu_fe_ingress_rate_set,
    onu_fe_ingress_rate_set_cmd,
    "port ingress_rate <port_list> {[0|1|2|3] [<0>|<62-1000000>]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the port number\n"
    "0-Limit all frames\n"
	"1-Limit Broadcast, Multicast and flooded unicast frames\n"
	"2-Limit Broadcast and Multicast frames only\n"
	"3-Limit Broadcast frames only\n"
	"0-not limit\n"
	"port ingress rate,unit:kbps\n"
    /*"port ingress rate,unit:kbps,for "DEVICE_TYPE_NAME_GT810_STR"/"DEVICE_TYPE_NAME_GT816_STR",the rate is 100-100000;for "DEVICE_TYPE_NAME_GT811A_STR"/"DEVICE_TYPE_NAME_GT812A_STR",Only 64, 128, 256, 512, 1000, 2000, 4000, 8000, 16000, 32000, 64000 are supported\n"*/
  /*  "Frames will be dropped when exceed limit.\n"
    "Port will transmit pause frame when exceed limit.\n"
    */
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "port ingress_rate" );
#elif defined __RPC_ONU_COMMAND

	int ponID=0;
	ULONG   ulIfIndex = 0, port, suffix;
	ULONG   ulSlot, ulPort, ulOnuid, ulRet;
    ULONG   gMaxPort = 0;
	ULONG action = ONU_CONF_PORT_INGRESS_ACT_NONE;
	ULONG burst = ONU_CONF_PORT_INGRESS_BURST_NONE;
    char *cstatus[]={
        "all frames",
        "Broadcast, Multicast and flooded unicast frames",
        "Broadcast and Multicast frames only",
        "Broadcast frames only"};
    char *cstatus1[]={"NULL","drop.","transmit pause frame."};    
    char *cstatus2[]={"NULL","12k bytes.","24k bytes.","48k bytes.","96k bytes."};   
    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

#if 0
		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)
#else
		if(ONU_OPER_STATUS_DOWN == GetOnuOperStatus(ponID, ulOnuid-1))\
		{
			vty_out(vty, "onu is off-line!\r\n");
			return CMD_WARNING;
		}
		if(getDeviceCapEthPortNum(MAKEDEVID(ulSlot, ulPort, ulOnuid), &gMaxPort) != VOS_OK) \
		{
			vty_out(vty, "unknown onu port num!\r\n");
			return CMD_WARNING;
		}
        
#endif
        if(argc == 3)
        {

            ULONG rate = VOS_StrToUL(argv[2], NULL, 10);
            ULONG type = VOS_StrToUL(argv[1], NULL, 10);

            ONU_CONF_WRITEABLE_CHECK
#if 0
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(port > gMaxPort)
                    continue;
                else
                {
                    if(OnuMgt_SetPortIngressRate(ponID, ulOnuid-1, port, type, rate, action, burst) != VOS_OK)
                        vty_out(vty, "\r\nset port ingress rate ERROR\r\n");
                }
            }
            END_PARSE_PORT_LIST_TO_PORT()
#else
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(port > gMaxPort)
                    continue;
                else
                {
                    if(OnuMgt_SetPortIngressRate(ponID, ulOnuid-1, port, type, rate, action, burst) != VOS_OK)
                    {
#if 0                            
                        vty_out(vty, "\r\nset port ingress rate ERROR\r\n");
#else
                        error_flag = 1;
#endif
                    }
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                                
#endif
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                    {
                        ULONG uv = 0;
                        CTC_STACK_ethernet_port_policing_entry_t policing;
                        /*if(getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_ingress_rate_limit, &uv) == VOS_OK)*/
                    if(port > gMaxPort)
                        continue;
                    else
                    {
                        if(VOS_OK == OnuMgt_GetEthPortPolicing(ponID, ulOnuid-1, port, &policing))
                        {
                            vty_out(vty, "Port %d: ingress rate configuration:\r\n", port);                            
                            uv = policing.cir;
                            if(policing.operation)
                            {
                                vty_out(vty, "%s : %d kbps\r\n", "ingress rate", uv);
                            }
                            else
                                vty_out(vty, "%s : No limited\r\n", "ingress rate");
                        }
                        else
                            error_flag = 1;
                    }
                }
                END_PARSE_PORT_LIST_TO_PORT();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);                        
            }
            else
            {
                    char *pbuf = NULL;
                    USHORT len;
                    if(OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            	vty_big_out(vty, len, "%s", pbuf);
                        /*vty_out(vty, "\r\n%s\n", pbuf);*/
                    else
                    {
                        vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                    }
            }
        }
    }
    else
    {
        ULONG type=0, rate = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        if(argc == 3)
        {
            type = VOS_StrToUL(argv[1], NULL, 10);
            rate = VOS_StrToUL(argv[2], NULL, 10);
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_limit, rate)&&
                        setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_type, type) &&
                        setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_action, action)&&
                        setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_burst, burst))
                    vty_out(vty, "portd %d ingress rate limit error\r\n", port);
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_limit, &rate) == VOS_OK &&
                    getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_type, &type) == VOS_OK &&
                    getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_action, &action) == VOS_OK &&
                    getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_burst, &burst) == VOS_OK )
                {                  
                    vty_out(vty,"Port %d: ingress rate control configuration:\r\n", port);
                    vty_out(vty,"%20s : %s\r\n", "Control mode", action == ONU_CONF_PORT_INGRESS_ACT_NONE?"Priority based.":"Burst based.");
                    vty_out(vty,"%20s : %s\r\n", "Limit", cstatus[type]);
                    if(rate)
                        vty_out(vty,"%20s : %d Kbps.\r\n", "Ingress rate", rate);
                    else
                        vty_out(vty,"%20s : %s\r\n", "Ingress rate", "No limit.");

                    if(action != ONU_CONF_PORT_INGRESS_ACT_NONE && rate)
                    {
                        vty_out(vty,"%20s : %s\r\n", "Burst size", cstatus2[burst]);
                    }
                                            
                    if(action != ONU_CONF_PORT_INGRESS_ACT_NONE)
                        vty_out(vty,"%20s : %s\r\n", "When exceed limit", cstatus1[action]);
                }
                else
                    vty_out(vty, "port %d ingress rate get fail!\r\n", port);
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }

	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port ingress_rate !\r\n");	

	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_fe_ingress_rate_set1,
    onu_fe_ingress_rate_set_cmd1,
    "port ingress_rate <port_list> [0|1|2|3] [<0>|<62-1000000>] {[drop|pause] [12k|24k|48k|96k]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the port number\n"
    "0-Limit all frames\n"
	"1-Limit Broadcast, Multicast and flooded unicast frames\n"
	"2-Limit Broadcast and Multicast frames only\n"
	"3-Limit Broadcast frames only\n"
	"0-not limit\n"
    "port ingress rate,unit:kbps\n"
    /*"port ingress rate,unit:kbps,for "DEVICE_TYPE_NAME_GT810_STR"/"DEVICE_TYPE_NAME_GT816_STR",the rate is 100-100000;for "DEVICE_TYPE_NAME_GT811A_STR"/"DEVICE_TYPE_NAME_GT812A_STR",Only 64, 128, 256, 512, 1000, 2000, 4000, 8000, 16000, 32000, 64000 are supported\n" */
    "Frames will be dropped when exceed limit.\n"
    "Port will transmit pause frame when exceed limit.\n"
    "Burst mode : Burst Size is 12k bytes.\n" 
    "Burst mode : Burst Size is 24k bytes.\n"
    "Burst mode : Burst Size is 48k bytes.\n" 
    "Burst mode : Burst Size is 96k bytes.\n"
    )
{
#if 0
	return onu_fe_cli_process( vty, argv[0], "port ingress_rate" );
#elif defined __RPC_ONU_COMMAND
#if 0
	return rpc_onu_fe_cli_process( vty, argv[0], "port ingress_rate" );
#else
	int ponID=0;
	ULONG   ulIfIndex = 0, port, suffix;
	ULONG   ulSlot, ulPort, ulOnuid, ulRet, rate, type;
    ULONG   gMaxPort = 0;
	ULONG   action = ONU_CONF_PORT_INGRESS_ACT_NONE;
	ULONG   burst = ONU_CONF_PORT_INGRESS_BURST_NONE;

    ulIfIndex =(ULONG) vty->index;

	if(argc >= 3)
	{

		rate = VOS_StrToUL(argv[2], NULL, 10);
		type = VOS_StrToUL(argv[1], NULL, 10);

		if(argc == 5 )
		{
			action = VOS_StriCmp(argv[3], "drop")?ONU_CONF_PORT_INGRESS_ACT_PAUSE:ONU_CONF_PORT_INGRESS_ACT_DROP;
			if(!VOS_StriCmp(argv[4], "12k"))
				burst = ONU_CONF_PORT_INGRESS_BURST_12K;
			if(!VOS_StriCmp(argv[4], "24k"))
				burst = ONU_CONF_PORT_INGRESS_BURST_24K;
			if(!VOS_StriCmp(argv[4], "48k"))
				burst = ONU_CONF_PORT_INGRESS_BURST_48K;
			if(!VOS_StriCmp(argv[4], "96k"))
				burst = ONU_CONF_PORT_INGRESS_BURST_96K;					
			
		}

	}

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
	    ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	    if( ulRet !=VOS_OK )
	        return CMD_WARNING;

	    ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	    if(ponID == (VOS_ERROR))
	    {
	        vty_out( vty, "  %% Parameter error\r\n");
	        return CMD_WARNING;
	    }

#if 0
		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)
#else
		if(ONU_OPER_STATUS_DOWN == GetOnuOperStatus(ponID, ulOnuid-1))\
		{
			vty_out(vty, "onu is off-line!\r\n");
			return CMD_WARNING;
		}
		if(getDeviceCapEthPortNum(MAKEDEVID(ulSlot, ulPort, ulOnuid), &gMaxPort) != VOS_OK) \
		{
			vty_out(vty, "unknown onu port num!\r\n");
			return CMD_WARNING;
		}
        
#endif
        if(argc >= 3)
        {
            ONU_CONF_WRITEABLE_CHECK
#if 0
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(port > gMaxPort)
                    continue;
                else
                {
                    if(OnuMgt_SetPortIngressRate(ponID, ulOnuid-1, port, type, rate, action, burst) != VOS_OK)
                        vty_out(vty, "\r\nset port ingress rate ERROR\r\n");
                }
            }
            END_PARSE_PORT_LIST_TO_PORT()
#else
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(port > gMaxPort)
                    continue;
                else
                {
                    if(OnuMgt_SetPortIngressRate(ponID, ulOnuid-1, port, type, rate, action, burst) != VOS_OK)
                    {
#if 0                            
                        vty_out(vty, "\r\nset port ingress rate ERROR\r\n");
#else
                        error_flag = 1;
#endif
                    }
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                                           
#endif
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                {
                    ULONG uv = 0;
                    CTC_STACK_ethernet_port_policing_entry_t policing;
                    /*if(getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_ingress_rate_limit, &uv) == VOS_OK)*/
                    if(port > gMaxPort)
                        continue;
                    else
                    {
                        if(VOS_OK == OnuMgt_GetEthPortPolicing(ponID, ulOnuid-1, port, &policing))
                        {
                            uv = policing.cir;
                            if(uv)
                                vty_out(vty, "\r\nport %d ingress rate is %d kbps\r\n", port, uv);
                            else
                                vty_out(vty, "\r\nport %d ingress rate is not limited\r\n", port, uv);
                        }
                        else
                            error_flag = 1;
                    }
                }
                END_PARSE_PORT_LIST_TO_PORT();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);                        

            }
            else
            {
                    char *pbuf = NULL;
                    USHORT len;
                    if(OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            	vty_big_out(vty, len, "%s", pbuf);
                        /*vty_out(vty, "\r\n%s\n", pbuf);*/
                    else
                    {
                        vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                    }
            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        if(argc >= 3)
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_limit, rate) == VOS_OK &&
                        setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_type, type)  == VOS_OK&&
                        setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_action, action) == VOS_OK&&
                        setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_burst, burst) == VOS_OK)
                {
                }
                else
                    vty_out(vty, "portd %d ingress rate limit error\r\n", port);
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                    if(getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_ingress_rate_limit, &rate))
                        vty_out(vty, "port %d ingress rate get fail!\r\n", port);
                    else
                        vty_out(vty, "port %d ingress rate %d kbps\r\n", port, rate);
            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;

#endif
#else
	LONG lRet;
	CHAR pBuff[256];   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid = 0;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	int OnuType;
	unsigned int Policer_rate;

	if( check_onu_cli_is_deny(vty) )	/* modified by xieshl 20100521, 问题单10260 */
		return CMD_SUCCESS;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }

	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	/* modified by chenfj 2007-9-25 
		GT810/GT816支持的端口限速最小值为100，故特别加以判断*/
	Policer_rate = (ULONG)VOS_AtoL( argv[ 2 ] );
	if(GetOnuType(ponID, (ulOnuFeid-1), &OnuType ) != ROK)
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}
	if(( OnuType == V2R1_ONU_GT810 ) ||(OnuType == V2R1_ONU_GT816 ))
		{
		if(( Policer_rate < 100 ) && ( Policer_rate != 0 ))
			{
			vty_out(vty,"onu%d/%d/%d is %s, the port rate is 100-100000 kbps\r\n", ulSlot , ulPort, ulOnuid, GetDeviceTypeString(OnuType));
			return( CMD_WARNING );
			}
		}


	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port ingress_rate !\r\n");	

	return CMD_SUCCESS;
#endif
}


DEFUN  (
    onu_fe_egress_rate_set,
    onu_fe_egress_rate_set_cmd,
    "port egress_rate <port_list> {[0|<62-1000000>]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the port number\n"
    "0-not limit\n"
    "Please input the rate( 62-100000) kbps\n"
    /*"Please input the rate( 62-100000) kbps,for "DEVICE_TYPE_NAME_GT810_STR"/"DEVICE_TYPE_NAME_GT816_STR",the rate is (100-100000)kbps\n" */
    )
{
#ifdef __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port, suffix;
    ULONG   ulSlot, ulPort, ulOnuid, ulRet;
    ULONG   gMaxPort= 0;
    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

#if 0
		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)
#else
		if(ONU_OPER_STATUS_DOWN == GetOnuOperStatus(ponID, ulOnuid-1))\
		{
			vty_out(vty, "onu is off-line!\r\n");
			return CMD_WARNING;
		}
		if(getDeviceCapEthPortNum(MAKEDEVID(ulSlot, ulPort, ulOnuid), &gMaxPort) != VOS_OK) \
		{
			vty_out(vty, "unknown onu port num!\r\n");
			return CMD_WARNING;
		}
        
#endif
        if(argc == 2)
        {
            ULONG rate = VOS_StrToUL(argv[1], NULL, 10);

            ONU_CONF_WRITEABLE_CHECK
#if 0
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(port > gMaxPort)
                    continue;
                else
                {
                    if(OnuMgt_SetPortEgressRate(ponID, ulOnuid-1, port, rate) != VOS_OK)
                        vty_out(vty, "\r\nset port egress rate ERROR\r\n");
                }
            }
            END_PARSE_PORT_LIST_TO_PORT()
#else
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(port > gMaxPort)
                    continue;
                else
                {
                    if(OnuMgt_SetPortEgressRate(ponID, ulOnuid-1, port, rate) != VOS_OK)
                    {
#if 0                            
                        vty_out(vty, "\r\nset port egress rate ERROR\r\n");
#else
                        error_flag = 1;
#endif
                    }
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);                    
#endif
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                    {
                        ULONG uv = 0;
                        CTC_STACK_ethernet_port_ds_rate_limiting_entry_t policing;
                        /*if(getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_egress_limit, &uv) == VOS_OK)*/
                        if(port > gMaxPort)
                            continue;
                        else
                        {
                        if(VOS_OK == OnuMgt_GetEthPortDownstreamPolicing(ponID, ulOnuid-1, port, &policing))
                            {
                                vty_out(vty, "Port %d: egress rate configuration:\r\n", port);                            
                                
                                uv = policing.CIR;
                                if(uv)
                                    vty_out(vty, "%s : %d kbps\r\n", "egress rate", uv);
                                else
                                    vty_out(vty, "%s : No limited\r\n", "egress rate");
                            }
                            else
                            {
#if 0                                
                                vty_out(vty, "\r\n port %d egress rate get fail!\r\n");
#else
                                error_flag = 1;
#endif
                            }
                        }
                    }
                    END_PARSE_PORT_LIST_TO_PORT();
                    if(error_flag)
                        vty_out(vty, ONU_CMD_ERROR_STR);
                            
            }
            else
            {
                char *pbuf = NULL;
                USHORT len;
                if(OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            	vty_big_out(vty, len, "%s", pbuf);
                        /*vty_out(vty, "\r\n%s\n", pbuf);*/
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG rate = 0;

        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        if(argc == 2)
        {
            rate = VOS_StrToUL(argv[1], NULL, 10);
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_egress_limit, rate))
                vty_out(vty, "port %d egress rate limit fail!\r\n", port);

            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_egress_limit, &rate))
                vty_out(vty, "port %d egress rate limit get fail!\r\n", port);
            else
            {
                vty_out(vty, "Port %d: egress rate configuration:\r\n", port);                            
                
                if(rate)
                    vty_out(vty, "%s : %d kbps\r\n", "egress rate", rate);
                else
                    vty_out(vty, "%s : No limited\r\n", "egress rate");
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256];   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid = 0;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;
	unsigned int Policer_rate;
	int OnuType;

	if( check_onu_cli_is_deny(vty) )	/* modified by xieshl 20100521, 问题单10260 */
		return CMD_SUCCESS;

	vty->length = VOS_StrLen(vty->buf);
	if( vty->length >= 256 )
		return (CMD_WARNING );
	if( vty->length == 0 )
		return (CMD_SUCCESS );

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }

	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	/* modified by chenfj 2007-9-25 
		GT810/GT816支持的端口限速最小值为100，故特别加以判断*/
	if( argc == 2 )
		{
		Policer_rate = (ULONG)VOS_AtoL( argv[ 1 ] );
		if(GetOnuType(ponID, (ulOnuFeid-1), &OnuType ) != ROK)
			{
			vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
			return (CMD_WARNING );
			}
		if(( OnuType == V2R1_ONU_GT810 ) ||(OnuType == V2R1_ONU_GT816 ))
			{
			if(( Policer_rate < 100 ) && (Policer_rate != 0 ))
				{
				vty_out(vty,"onu%d/%d/%d is %s, the port rate is 100-100000 kbps\r\n",ulSlot , ulPort, ulOnuid, GetDeviceTypeString(OnuType));
				return( CMD_WARNING );
				}
			}
		}
	
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port egress_rate !\r\n");	


	return CMD_SUCCESS;
#endif
}

/*已被取消*/
DEFUN  (
    onu_fe_port_maclimit_set,
    onu_fe_port_maclimit_set_cmd,
    "port maclimit <port_list> {[0|<1-16>]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port maclimit information\n"
    "Please input the port number\n"
    "Config onu FE port not maclimit\n"    
    "Please input size(1-16)\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "port maclimit" );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port maclimit  !\r\n");	


	return CMD_SUCCESS;
#endif
}


/*added by wutw*/
DEFUN  (
    onu_fe_port_show,
    onu_fe_port_show_cmd,
    "port link_show",    
    "Show or config onu FE port information\n"
    "Show onu FE port link status\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND_1
	return onu_dev_cli_process( vty, "port link_show",NULL );
#elif defined __RPC_ONU_COMMAND
    LONG lRet, ulRet;
    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ulIfIndex =(ULONG) vty->index;
    ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
    if( ulRet !=VOS_OK )
        return CMD_WARNING;
    ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
    if(ponID == (VOS_ERROR))
    {
        vty_out( vty, "  %% Parameter error\r\n");
        return CMD_WARNING;
    }
    if(IsCtcOnu(ponID, ulOnuid-1))
    {
        ULONG devidx, fenum;
        lRet =  GetOnuOperStatus( ponID, ulOnuid-1);
        if ( CLI_EPON_ONUUP != lRet)
        {
           #ifdef CLI_EPON_DEBUG
              vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
           #endif
           vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
            return CMD_WARNING;
        }
        devidx = MAKEDEVID(ulSlot, ulPort, ulOnuid);
        if(getDeviceCapEthPortNum(devidx, &fenum) == VOS_OK)
        {
            int i=0;
            int link = 0;
            vty_out(vty, "\r\n");
            for(i=0; i<fenum; i++)
            {
                if(OnuMgt_GetEthPortLinkState(ponID, ulOnuid-1, i+1, &link) == VOS_OK)
                    vty_out(vty, "eth1/%d:\t%s\r\n", i+1, link?"up":"down");
                else
                    vty_out(vty, "eth1/%d:\tdown\r\n", i+1, link?"up":"down");
            }
            vty_out(vty, "\r\n");
        }
    }
    else
	return rpc_onu_dev_cli_process( vty, "port link_show",NULL );
    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
	        vty_out(vty, "  %% port link_show!\r\n");

	return CMD_SUCCESS;
#endif
}



/*added by wutw*/
/*  modified by chenfj 2007-11-12
	问题单#5709: mirror_from不支持mirror all 选项，ONU串口支持，OLT侧不支持
	*/
DEFUN  (
    onu_fe_port_mirror_from_set,
    onu_fe_port_mirror_from_set_cmd,
    "port mirror_from {[i|e|a] <port_list> [0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE mirror port enable\n"
    "ingress\n"
    "egress\n"
    "All direction\n"
    "Please input the port number\n"
    "not a source mirror\n"
    "as a source mirror\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[1], "port mirror_from" );
#elif defined __RPC_ONU_COMMAND
#if 0
return rpc_onu_fe_cli_process( vty, argv[1], "port mirror_from" );
#endif
    int ponID=0;
    ULONG   ulIfIndex = 0, port, suffix;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
    int flag = 0;
    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[1], port)

        if(argc == 3)
        {
            ULONG mode = VOS_StrToUL(argv[2], NULL, 10);
            int type;
            if (VOS_StriCmp(argv[0], "i") == 0)
                type = 1;
            else if(VOS_StriCmp(argv[0], "e") == 0)
                type = 2;
            else
                type = 3;
            ONU_CONF_WRITEABLE_CHECK
            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], port )
            {
            }
            END_PARSE_PORT_LIST_TO_PORT();

            flag = OnuMgt_SetPortMirrorFrom(ponID, ulOnuid-1, port, mode, type);
#if 0
            switch(flag)
            {
                case 1:
                    vty_out(vty, "port ingress mirror_fromlist set fail!\r\n");
                    break;
                case 2:
                    vty_out(vty, "port egress mirror_fromlist set fail!\r\n");
                    break;
                default:
                    vty_out(vty, "port mirror_fromlist set fail!\r\n");
                    break;
            }
#else
            if(flag != VOS_OK)
                vty_out(vty, ONU_CMD_ERROR_STR);

#endif
        }
        else
        {
            ULONG portlist = 0, cur_list = 0;
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                 if(getOnuConfSimpleVar(ponID, ulOnuid-1, sv_enum_port_mirror_egress_fromlist, &portlist) ||
                         getOnuConfSimpleVar(ponID, ulOnuid-1, sv_enum_port_mirror_ingress_fromlist, &cur_list) )
                     vty_out(vty, "port mirror-from get fail!\r\n");
                 else
                 {

                     int port = 0;
                     vty_out(vty, "Mirror from\t\tDirection\r\n\r\n");

                     while(cur_list)
                     {
                         if(cur_list&1)
                         {
                             char sz[80]= "";
                             VOS_Sprintf(sz, "1/%d", port+1);
                             vty_out(vty, "%-11s\t\t%-9s\r\n", sz, "ingress");
                         }
                         cur_list >>= 1;
                         port++;
                     }

                     port = 0;

                     while(portlist)
                     {
                         if(portlist&1)
                         {
                             char sz[80]= "";
                             VOS_Sprintf(sz, "1/%d", port+1);
                             vty_out(vty, "%-11s\t\t%-9s\r\n", sz, "egress");
                         }
                         portlist >>= 1;
                         port++;
                     }

                 }
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG mode1 = 0;

        ULONG portlist = 0, cur_list = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[1], port)
			
        if(argc == 3)
        {
            mode1 = VOS_StrToUL(argv[2], NULL, 10);
            if(VOS_StriCmp(argv[0], "i") == 0)
            {
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[1], port)

                portlist |= 1<<(port-1);

                END_PARSE_PORT_LIST_TO_PORT()

                if(getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_mirror_ingress_fromlist, &cur_list) == VOS_OK)
                {
                    if(mode1 == 0)
                        cur_list &= ~portlist;
                    else
                        cur_list |= portlist;

                    if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_mirror_ingress_fromlist, cur_list) != VOS_OK)
                        vty_out(vty, "port %s ingress mirror_fromlist set fail!\r\n", argv[1]);
                }
                else
                    vty_out(vty, "port %s ingress mirror_fromlist set fail!\r\n", argv[1]);
            }
            else if(VOS_StriCmp(argv[0], "e") == 0)
            {
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[1], port)

                portlist |= 1<<(port-1);

                END_PARSE_PORT_LIST_TO_PORT()

                if(getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_mirror_egress_fromlist, &cur_list) == VOS_OK)
                {
                    if(mode1 == 0)
                        cur_list &= ~portlist;
                    else
                        cur_list |= portlist;

                    if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_mirror_egress_fromlist, cur_list) != VOS_OK)
                        vty_out(vty, "port %s egress mirror_fromlist set fail!\r\n", argv[1]);
                }
                else
                    vty_out(vty, "port %s egress mirror_fromlist set fail!\r\n", argv[1]);
            }
            else
            {
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[1], port)

                    portlist |= 1<<(port-1);

                END_PARSE_PORT_LIST_TO_PORT()

                    if(getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_mirror_ingress_fromlist, &cur_list) == VOS_OK)
                    {
                        if(mode1 == 0)
                            cur_list &= ~portlist;
                        else
                            cur_list |= portlist;

                        if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_mirror_ingress_fromlist, cur_list) != VOS_OK)
                            vty_out(vty, "port %s ingress mirror_fromlist set fail!\r\n", argv[1]);

                        if(getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_mirror_egress_fromlist, &cur_list) == VOS_OK)
                        {
                            if(mode1 == 0)
                                cur_list &= ~portlist;
                            else
                                cur_list |= portlist;

                            if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_mirror_egress_fromlist, cur_list) != VOS_OK)
                                vty_out(vty, "port %s egress mirror_fromlist set fail!\r\n", argv[1]);
                        }
                        else
                            vty_out(vty, "port %s mirror_fromlist set fail!\r\n", argv[1]);
                    }
                    else
                    {
                        vty_out(vty, "port %s mirror_fromlist set fail!\r\n", argv[1]);
                    }
            }
        }
        else
        {
           if(getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_mirror_egress_fromlist, &portlist) ||
                   getOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_mirror_ingress_fromlist, &cur_list) )
               vty_out(vty, "port mirror-from get fail!\r\n");
           else
           {

               int port = 0;
               vty_out(vty, "Mirror from\t\tDirection\r\n\r\n");

               while(cur_list)
               {
                   if(cur_list&1)
                   {
                       char sz[80]= "";
                       VOS_Sprintf(sz, "1/%d", port+1);
                       vty_out(vty, "%-11s\t\t%-9s\r\n", sz, "ingress");
                   }
                   cur_list >>= 1;
                   port++;
               }

               port = 0;

               while(portlist)
               {
                   if(portlist&1)
                   {
                       char sz[80]= "";
                       VOS_Sprintf(sz, "1/%d", port+1);
                       vty_out(vty, "%-11s\t\t%-9s\r\n", sz, "egress");
                   }
                   portlist >>= 1;
                   port++;
               }

           }
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	
	if(argc != 0)
	{	/*
		ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 1 ] );
    		if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
			return CMD_WARNING;
		*/
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port mirror_from!\r\n");	


	return CMD_SUCCESS;
#endif
}


/*added by wutw*/
DEFUN  (
    onu_fe_port_mirror_to_set,
    onu_fe_port_mirror_to_set_cmd,
    "port mirror_to {[i|e] [<port_list>|0]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE object mirror port\n"
    "ingress\n"
    "egress\n"
    "Plseas input the port number\n"
    "Delete the mirror port\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	char *parg = NULL;
	if( VOS_AtoL( argv[1]) != 0 )
		parg = argv[1];
	return onu_fe_cli_process( vty, parg, "port mirror_to" );
#elif defined __RPC_ONU_COMMAND
#if 0
	char *parg = NULL;
	if( VOS_AtoL( argv[1]) != 0 )
		parg = argv[1];
	return rpc_onu_fe_cli_process( vty, parg, "port mirror_to" );
#endif
    int ponID=0,flag = 0;
    ULONG ulIfIndex = 0, port, suffix;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
    int type = 0;
    
    ulIfIndex =(ULONG) vty->index;

    if(argc == 2)
    {
        if(VOS_StriCmp(argv[0], "i") == 0)
            type = 1;
        else
            type = 2;

        if(!VOS_StrCmp(argv[1], "0"))
            port = 0;
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[1], port)
                END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[1], port)

        if(argc == 2)
        {

            ONU_CONF_WRITEABLE_CHECK
            if(VOS_StrToUL(argv[1],NULL,10)==0)
            {
                if(VOS_StriCmp(argv[0], "i") == 0)
                    type = 1;
                else
                    type = 2;
                flag = OnuMgt_DeleteMirror(ponID, ulOnuid-1, type);
#if 0                
				switch(flag)
                {
                    case 1:
                        vty_out(vty, "Delete the ingress mirror port ERROR!\r\n");
                        break;
                    case 2:
                        vty_out(vty, "Delete the egress mirror port ERROR!\r\n");
                        break;
                    default:
                        vty_out(vty, "Delete the mirror port ERROR!\r\n");
                        break;
                }   
#else
                if(flag != VOS_OK)
                    vty_out(vty, ONU_CMD_ERROR_STR);
                    
#endif
            }
            else
            {
                if(VOS_StriCmp(argv[0], "i") == 0)
                    type = 1;
                else
                    type = 2;
                BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], port )
                {
                    flag = OnuMgt_SetPortMirrorTo(ponID, ulOnuid-1, port,type);
#if 0
                    switch(flag)
                    {
                        case 1:
                            vty_out(vty, "port ingress mirror_tolist set error!\r\n");
                            break;
                        case 2:
                            vty_out(vty, "port ingress mirror_tolist set error!\r\n");
                            break;
                        default:
                            vty_out(vty, "port mirror_tolist set error!\r\n");
                            break;
                    }
#else
                    if(flag != VOS_OK)
                        error_flag = 1;
#endif
                }
                END_PARSE_PORT_LIST_TO_PORT();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);
                    
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {

                    ULONG mode1 = 0;
                    ULONG mode2 = 0;
                    if(getOnuConfSimpleVar(ponID, ulOnuid-1 ,sv_enum_port_mirror_ingress_tolist, &mode1)||
                        getOnuConfSimpleVar(ponID, ulOnuid-1 , sv_enum_port_mirror_egress_tolist, &mode2))
                        vty_out(vty, "port mirror_tolist get fail!\r\n");
                    else
                    {
                        if(mode1)
                            vty_out(vty, "\r\ningress mirror_tolist is %d\r\n", mode1);
                        if(mode2)
                            vty_out(vty, "\r\negress mirror_tolist is %s\r\n", mode2);
                    }
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG mode1 = 0;
        ULONG mode2 = 0;

        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[1], port)
		
        if(argc == 2)
        {

            if(type == 1)
            {
                if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_mirror_ingress_tolist, port) != VOS_OK)
                {
                    if(port)
                        vty_out(vty, "port %s ingress mirror_tolist set fail!\r\n", argv[1]);
                    else
                        vty_out(vty, "port  ingress mirror_tolist empty fail!\r\n");
                }
            }

            if(type == 2)
            {
                if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_mirror_egress_tolist, port) != VOS_OK)
                {
                    if(port)
                        vty_out(vty, "port %s egress mirror_tolist set fail!\r\n", argv[1]);
                    else
                        vty_out(vty, "port  egress mirror_tolist empty fail!\r\n");
                }
            }
        }
        else
        {
                if(getOnuConfSimpleVarByPtr(suffix, pd ,sv_enum_port_mirror_ingress_tolist, &mode1)||
                    getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_port_mirror_egress_tolist, &mode2))
                    vty_out(vty, "port mirror_tolist get fail!\r\n");
                else
                {
                    if(mode1)
                        vty_out(vty, "\r\ningress mirror_tolist is %d\r\n", mode1);
                    if(mode2)
                        vty_out(vty, "\r\negress mirror_tolist is %d\r\n", mode2);
                }
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	
	if(argc != 0)
	{
	    if (!VOS_StrCmp((CHAR *)argv[0], "0"))
	    {
	    }
		else
		{	/*
			ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 1 ] );
			if ( ulOnuFeid != 0 )
			{
	    		if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
					return CMD_WARNING;
			}
			*/
			BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], ulOnuFeid )
			{
				if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
				{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
				}
			}
			END_PARSE_PORT_LIST_TO_PORT();		
		}
	}

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port mirror_to!\r\n");	


	return CMD_SUCCESS;
#endif
}


/*added by wutw*/
DEFUN  (
    onu_fe_port_mirror_show,
    onu_fe_port_mirror_show_cmd,
    "port mirror_show",    
    "Show or config onu FE port information\n"
    "Show onu FE mirror port information\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "port mirror_show",NULL );
#elif defined __RPC_ONU_COMMAND
#if 0
	return rpc_onu_dev_cli_process( vty, "port mirror_show",NULL );
#endif
    int ponID=0,i=0,j = 0;
    ULONG ulIfIndex = 0, suffix;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
     int flag = 0;
    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }
        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {
    	    ULONG mode1 = 0;
            ULONG mode2 = 0;
            ULONG mode3 = 0;
            ULONG mode4 = 0;
            vty_out(vty, "\r\n%16s %6s%-10s %16s\r\n","Mirror from "," ", "Direction","Mirror to");
            vty_out(vty, "------------------------------------------------------------\r\n");
            if((VOS_OK == getOnuConfSimpleVar(ponID, ulOnuid-1,  sv_enum_port_mirror_egress_tolist, &mode1))&&
                (VOS_OK == getOnuConfSimpleVar(ponID, ulOnuid-1,  sv_enum_port_mirror_ingress_tolist, &mode2))&&
                (VOS_OK == getOnuConfSimpleVar(ponID, ulOnuid-1,  sv_enum_port_mirror_egress_fromlist, &mode3))&&
                (VOS_OK == getOnuConfSimpleVar(ponID, ulOnuid-1,  sv_enum_port_mirror_ingress_fromlist, &mode4)))
            {
                if(mode1)
                {
                    for(j=0;j<ONU_MAX_PORT;j++)
                    {
                        if(mode3&(1<<j))
                        {
                            vty_out(vty, "%9s%-7d %6s%-10s %11s%-5d\r\n","1/", j+1, " ","egress", "1/", mode1);
                            flag = 1;
                        }
                    }
                    if(!flag)
                    {
                        vty_out(vty, "%4s%-12s %6s%-10s %10s%-6d\r\n"," ", "Not defined", " ","egress", "1/", mode1);
                    }
                }
                else if(mode3)
                {
                    for(i=0;i<ONU_MAX_PORT;i++)
                    {
                        if(mode3&(1<<i))
                            vty_out(vty, "%9s%-7d %6s%-10s %6s%-10s\r\n","1/", i+1, " ","egress", " ","Not defined");
                    }
                }
                if(mode2)
                {
                    for(j=0;j<ONU_MAX_PORT;j++)
                    {
                        if(mode4&(1<<j))
                        {
                            vty_out(vty, "%9s%-7d %6s%-10s %11s%-5d\r\n","1/", j+1, " ","ingress", "1/", mode2);
                            flag = 1;
                        }
                    }
                    if(!flag)
                    {
                        vty_out(vty, "%4s%-12s %6s%-10s %11s%-5d\r\n"," ", "Not defined", " ", "ingress", "1/", mode2);
                    }
                }
                else if(mode4)
                {
                    for(i=0;i<ONU_MAX_PORT;i++)
                    {
                        if(mode4&(1<<i))
                            vty_out(vty, "%9s%-7d %6s%-10s %6s%-10s\r\n","1/", i+1, " ","ingress"," ", "Not defined");
                    }
                }
                vty_out(vty, "------------------------------------------------------------\r\n");
    		}
    		else
    		    vty_out(vty, "\r\nport mirror_to mode get error!\r\n");
        }
        else
        {
            char *pRecv;
            USHORT len;

            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
		            vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    } 
    else
    {
        ULONG mode1 = 0,mode2 = 0;
        ULONG mode3 = 0,mode4 = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        vty_out(vty, "\r\n%16s %6s%-10s %16s\r\n","Mirror from "," ", "Direction","Mirror to");
        vty_out(vty, "------------------------------------------------------------\r\n");
        if((VOS_OK == getOnuConfSimpleVarByPtr(suffix, pd,  sv_enum_port_mirror_egress_tolist, &mode1))&&
            (VOS_OK == getOnuConfSimpleVarByPtr(suffix, pd,  sv_enum_port_mirror_ingress_tolist, &mode2))&&
            (VOS_OK == getOnuConfSimpleVarByPtr(suffix, pd,  sv_enum_port_mirror_egress_fromlist, &mode3))&&
            (VOS_OK == getOnuConfSimpleVarByPtr(suffix, pd,  sv_enum_port_mirror_ingress_fromlist, &mode4)))
        {
                if(mode1)
                {
                    for(j=0;j<ONU_MAX_PORT;j++)
                    {
                        if(mode3&(1<<j))
                        {
                            vty_out(vty, "%9s%-7d %6s%-10s %11s%-5d\r\n","1/", j+1, " ","egress", "1/", mode1);
                            flag = 1;
                        }
                    }
                    if(!flag)
                    {
                        vty_out(vty, "%4s%-12s %6s%-10s %10s%-6d\r\n"," ", "Not defined", " ","egress", "1/", mode1);
                    }
                }
                else if(mode3)
                {
                    for(i=0;i<ONU_MAX_PORT;i++)
                    {
                        if(mode3&(1<<i))
                            vty_out(vty, "%9s%-7d %6s%-10s %6s%-10s\r\n","1/", i+1, " ","egress", " ","Not defined");
                    }
                }
                if(mode2)
                {
                    for(j=0;j<ONU_MAX_PORT;j++)
                    {
                        if(mode4&(1<<j))
                        {
                            vty_out(vty, "%9s%-7d %6s%-10s %11s%-5d\r\n","1/", j+1, " ","ingress", "1/", mode2);
                            flag = 1;
                        }
                    }
                    if(!flag)
                    {
                        vty_out(vty, "%4s%-12s %6s%-10s %11s%-5d\r\n"," ", "Not defined", " ", "ingress", "1/", mode2);
                    }
                }
                else if(mode4)
                {
                    for(i=0;i<ONU_MAX_PORT;i++)
                    {
                        if(mode4&(1<<i))
                            vty_out(vty, "%9s%-7d %6s%-10s %6s%-10s\r\n","1/", i+1, " ","ingress"," ", "Not defined");
                    }
                }
                vty_out(vty, "------------------------------------------------------------\r\n");   
        }
    	else
    	    vty_out(vty, "\r\nport mirror_to mode get error!\r\n");
    }

	return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port mirror_show!\r\n");	

	return CMD_SUCCESS;
#endif
}

/*added by wangxy 2007-05-15*/
/*FE port config*/
DEFUN  (
    onu_fe_port_ingress_rate_limit_base,
    onu_fe_port_ingress_rate_limit_base_cmd,
    "port ingress_rate_limit_base {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port ingress rate limit base mechanism\n"
    "Priority based\n"
    "Burst Size based\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "port ingress_rate_limit_base",NULL );
#elif defined __RPC_ONU_COMMAND
#if 0
	return rpc_onu_dev_cli_process(vty, "port ingress_rate_limit_base",NULL );
#endif
    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;
        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {
            ONU_CONF_WRITEABLE_CHECK
            if(VOS_OK != OnuMgt_SetIngressRateLimitBase(ponID, ulOnuid-1, VOS_StrToUL(argv[0], NULL, 10)) )
            {
#if 0                
                vty_out(vty, "ingress rate limit based action config fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {

                    ULONG uv = 0;
                    if(getOnuConfSimpleVar(ponID, ulOnuid-1, sv_enum_ingressRateLimitBase, &uv) == VOS_OK)
                        vty_out(vty, "ingress_rate_limit is %s\r\n\r\n", uv?"Burst Size based":"Priority based");
                    else
                        vty_out(vty, "ingress_rate_limit base get fail\r\n\r\n");
            }
            else
            {
               char *pRecv;
               USHORT len;
               if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
               {
		            vty_big_out(vty, len, "%s", pRecv);
               }
               else
               {
                        vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
               }
            }
        }
    }
    else
    {
        ULONG uv = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(argc == 1)
        {
            uv = VOS_StrToUL(argv[0], NULL, 10);
            if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_ingressRateLimitBase, uv)!=VOS_OK)
            {
                vty_out(vty, "ingress rate limit based action config fail!\r\n");
            }
        }
        else
        {
            if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_ingressRateLimitBase, &uv)== VOS_OK)
               vty_out(vty, "ingress_rate_limit is %s\r\n\r\n", uv?"Burst Size based":"Priority based");
            else
               vty_out(vty, "ingress_rate_limit base get fail\r\n\r\n");
        }
    }
    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid/*, ulOnuFeid*/;
	USHORT length;
	ULONG ulRet;
	/*int setval=0;*/
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }

	if( vty->length <= 256 )
	{
		VOS_MemCpy( pBuff, vty->buf, vty->length );
		length = vty->length;
		stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM );
		stPayload->fd = vty->fd;
		
	}
	else
		return CMD_WARNING;
	

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port ingress_rate_limit_base !\r\n");	


	return CMD_SUCCESS;
#endif
}

/*added by wangxy 2007-11-06*/
DEFUN  (
    onu_fe_port_link_mon,
    onu_fe_port_link_mon_cmd,
    "port link_mon {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port link monitor information\n"
    "disable the monitor function\n"
    "enable the monitor function\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	int onu_mask[] = {V2R1_ONU_GT810, V2R1_ONU_GT816, V2R1_ONU_GT811_A, V2R1_ONU_GT811_B, V2R1_ONU_GT812_A,V2R1_ONU_GT815,
	V2R1_ONU_GT815_B,V2R1_ONU_GT871, 0};
	return onu_dev_cli_process( vty, "port link_mon", onu_mask );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {

            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK == OnuMgt_SetPortLinkMon(ponID, ulOnuid-1, VOS_StrToUL(argv[0], NULL, 10)) )
            {
            }
            else
            {
#if 0                
                vty_out(vty, "port linkmon_enable set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                    ULONG mon;
                    if(VOS_OK == getOnuConfSimpleVar(ponID, ulOnuid-1,sv_enum_port_linkmon_enable, &mon))
                            vty_out(vty, "\r\nport linkmon_enable is %d\r\n",mon);
                    else
                            vty_out(vty, "\r\nport linkmon_enable get error!\r\n");
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG mon = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(argc == 1)
        {
            mon= VOS_StrToUL(argv[0], NULL, 10);

            if(setOnuConfSimpleVarByPtr(suffix, pd,sv_enum_port_linkmon_enable, mon))
                vty_out(vty, "port linkmon_enable set fail!\r\n");
        }
        else
        {
            if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_port_linkmon_enable, &mon))
                vty_out(vty, "port linkmon_enable get fail!\r\n");
            else
                vty_out(vty, "port linkmon_enable %d\r\n",mon);
        }
    }
    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid/*, ulOnuFeid*/;
	USHORT length;
	ULONG ulRet;
	/*int setval=0;*/
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}

     	/*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    	lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    	if ( CLI_EPON_ONUUP != lRet)
       {
	       #ifdef CLI_EPON_DEBUG
	          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
	       #endif
	   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
			return CMD_WARNING;
       }

	if( vty->length <= 256 )
	{
		VOS_MemCpy( pBuff, vty->buf, vty->length );
		length = vty->length;
		stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM );
		stPayload->fd = vty->fd;
		
	}
	else
		return CMD_WARNING;
	

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port link mon !\r\n");	


	return CMD_SUCCESS;
#endif
}

/*added by wangxy 2007-11-06*/
DEFUN  (
    onu_fe_port_mode_mon,
    onu_fe_port_mode_mon_cmd,
    "port mode_mon {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port mode monitor\n"
    "disable the monitor function\n"
    "enable the monitor function\n"
    )
{
/* 问题单7324   chenfj
    GT861 本地没有这条命令, 在onu_mask[]中增加GT861*/
#ifdef __CTC_TEST_NMS_COMMAND
	int onu_mask[] = {V2R1_ONU_GT810, V2R1_ONU_GT816, V2R1_ONU_GT811_A, V2R1_ONU_GT811_B, V2R1_ONU_GT812_A,V2R1_ONU_GT815,
	V2R1_ONU_GT815_B,V2R1_ONU_GT861, V2R1_ONU_GT871, 0};
	return onu_dev_cli_process( vty, "port mode_mon", onu_mask );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;


    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {

            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK == OnuMgt_SetPortModeMon(ponID, ulOnuid-1, VOS_StrToUL(argv[0], NULL, 10)) )
            {
            }
            else
            {
#if 0                
                vty_out(vty, "modemon_enable set error!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                ULONG mon;
                if (VOS_OK == getOnuConfSimpleVar(ponID, ulOnuid - 1, sv_enum_port_modemon_enable, &mon))
                    vty_out(vty, "\r\nport mode_mon is %d\r\n", mon);
                else
                    vty_out(vty, "\r\nport mode_mon get error!\r\n");
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG mon = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(argc == 1)
        {
            mon = VOS_StrToUL(argv[0], NULL, 10);

            if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_port_modemon_enable, mon))
                vty_out(vty, "modemon_enable set fail!\r\n");
        }
        else
        {
            if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_port_modemon_enable, &mon))
                vty_out(vty, "modemon_enable get fail!\r\n");
            else
                vty_out(vty, "modemon_enable %d\r\n",mon);
        }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid/*, ulOnuFeid*/;
	USHORT length;
	ULONG ulRet;
	/*int setval=0;*/
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}

     	/*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    	lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    	if ( CLI_EPON_ONUUP != lRet)
       {
	       #ifdef CLI_EPON_DEBUG
	          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
	       #endif
	   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
			return CMD_WARNING;
       }

	if( vty->length <= 256 )
	{
		VOS_MemCpy( pBuff, vty->buf, vty->length );
		length = vty->length;
		stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM );
		stPayload->fd = vty->fd;
		
	}
	else
		return CMD_WARNING;
	

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%port link mon !\r\n");	


	return CMD_SUCCESS;
#endif
}


LDEFUN  (
    onu_band_set,
    onu_band_set_cmd,
    "bandwidth set [down|up] <0-7> {[0|1] <100-1000000> <0-8388607>}*1",    
    "Show or config onu Bandwidth information\n"
    "Show or config onu Bandwidth information\n"
    "Down stream policer\n"
    "Up stream policer\n"
    "Please input the prio <0-7>.When up diretion,prio value range:0-3.\n"
    "Disable policer \n"
    "Enable policer \n"
    "Please input the bandwidth (100-1000000 Kbits/s)\n"
    "Please input the burst (0-8388607 bytes)\n",
    ONU_NODE
    ) /*bandwidth need to add a analyse what(<0.1-100>) you input  added by suipl 2006/09/05*/
{
	return CMD_SUCCESS;
}

/*已经注销*/

/* 问题单7099   chenfj  2008-10-10

摘要： 针对广播报文的限速使用另一个命令 
内容： 
bandwidth broadcast <portlist> {[0|<64-1000000>]}*1

GT815可以分别限制广播和总流量。（该芯片每个端口有两个限速器）
*/

/*    
	modified by chenfj 2008-10-21
	GT815 V1R00B008 版本修改后，使用port ingress_rate命令可以生效. 故放弃该命令
*/
#if 0
DEFUN  (
    onu_band_broadcast_set,
    onu_band_broadcast_set_cmd,
    "bandwidth broadcast <portlist> {[0|<64-1000000>]}*1",    
    "Show or config onu Bandwidth information\n"
    "Show or config onu Bandwidth broadcast information\n"
    "Please input the port number\n"
    "Limit all broadcast frames\n"
    "broadcast frames rate\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "bandwidth broadcast" );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }


	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;



	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%bandwidth broadcast !\r\n");	


	return CMD_SUCCESS;
#endif
}
#endif

/*FE port config*/
LDEFUN  (
    onu_fe_port_en_set,
    onu_fe_port_en_set_cmd,
    "port en <port_list> {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port EN information\n"
    "Please input the port number\n"
    "Disable\n"
    "Enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_port_desc_set,
    onu_fe_port_desc_set_cmd,
    "port desc <port_list> {<string>}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port specific description information\n"
    "Please input the port number\n"
    "Please input the descriptor\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_port_mode_set,
    onu_fe_port_mode_set_cmd,
    "port mode <port_list> {[0|8|9|10|11|12]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port mode information\n"
    "Please input the port number\n"
    "Auto negotiation\n"
    "100M/FD\n"
    "100M/HD\n"
    "10M/FD\n"
    "10M/HD\n"
    "1000M/FD\n",
    ONU_NODE
    /*"1000M/FD,only for "DEVICE_TYPE_NAME_GT816_STR"\n"*/
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_port_negoration_show,
    onu_fe_port_negoration_show_cmd,
    "port mode_show <port_list>",    
    "Show or config onu FE port information\n"
    "Show negoration of onu FE port mode\n"
    "Please input the port number\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}



LDEFUN  (
    onu_fe_port_fc_set,
    onu_fe_port_fc_set_cmd,
    "port fc <port_list> {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port fc information\n"
    "Please input the port number\n"
    "Disable\n"
    "Enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_port_rate_set,
    onu_fe_port_rate_set_cmd,
    "port rate <port_list> [1|2] {<rate>}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the port number\n"
    "Ingress\n"
    "Egress\n"
    "Please input the rate( 0.1-100 Mbps)\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

/* modified by chenfj 2007-10-23
    修改ONU命令，增加Burst mode 参数*/
   
 /*
发件人: KD刘冬 
发送时间: 2007年11月15日 10:50
收件人: KD陈福军
主题: 请修改port ingress_rate命令

增加了drop|pause参数，做到与大亚原来的命令兼容；

*/

LDEFUN  (
    onu_fe_ingress_rate_set,
    onu_fe_ingress_rate_set_cmd,
    "port ingress_rate <port_list> {[0|1|2|3] [<0>|<62-1000000>]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the port number\n"
    "0-Limit all frames\n"
	"1-Limit Broadcast, Multicast and flooded unicast frames\n"
	"2-Limit Broadcast and Multicast frames only\n"
	"3-Limit Broadcast frames only\n"
	"0-not limit\n"
	"port ingress rate,unit:kbps\n",
    ONU_NODE
    /*"port ingress rate,unit:kbps,for "DEVICE_TYPE_NAME_GT810_STR"/"DEVICE_TYPE_NAME_GT816_STR",the rate is 100-100000;for "DEVICE_TYPE_NAME_GT811A_STR"/"DEVICE_TYPE_NAME_GT812A_STR",Only 64, 128, 256, 512, 1000, 2000, 4000, 8000, 16000, 32000, 64000 are supported\n"*/
  /*  "Frames will be dropped when exceed limit.\n"
    "Port will transmit pause frame when exceed limit.\n"
    */
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_fe_ingress_rate_set1,
    onu_fe_ingress_rate_set_cmd1,
    "port ingress_rate <port_list> [0|1|2|3] [<0>|<62-1000000>] {[drop|pause] [12k|24k|48k|96k]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the port number\n"
    "0-Limit all frames\n"
	"1-Limit Broadcast, Multicast and flooded unicast frames\n"
	"2-Limit Broadcast and Multicast frames only\n"
	"3-Limit Broadcast frames only\n"
	"0-not limit\n"
    "port ingress rate,unit:kbps\n"
    /*"port ingress rate,unit:kbps,for "DEVICE_TYPE_NAME_GT810_STR"/"DEVICE_TYPE_NAME_GT816_STR",the rate is 100-100000;for "DEVICE_TYPE_NAME_GT811A_STR"/"DEVICE_TYPE_NAME_GT812A_STR",Only 64, 128, 256, 512, 1000, 2000, 4000, 8000, 16000, 32000, 64000 are supported\n" */
    "Frames will be dropped when exceed limit.\n"
    "Port will transmit pause frame when exceed limit.\n"
    "Burst mode : Burst Size is 12k bytes.\n" 
    "Burst mode : Burst Size is 24k bytes.\n"
    "Burst mode : Burst Size is 48k bytes.\n" 
    "Burst mode : Burst Size is 96k bytes.\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


LDEFUN  (
    onu_fe_egress_rate_set,
    onu_fe_egress_rate_set_cmd,
    "port egress_rate <port_list> {[0|<62-1000000>]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port rate information\n"
    "Please input the port number\n"
    "0-not limit\n"
    "Please input the rate( 62-100000) kbps\n",
    ONU_NODE
    /*"Please input the rate( 62-100000) kbps,for "DEVICE_TYPE_NAME_GT810_STR"/"DEVICE_TYPE_NAME_GT816_STR",the rate is (100-100000)kbps\n" */
    )
{
	return CMD_SUCCESS;
}

/*已被取消*/
LDEFUN  (
    onu_fe_port_maclimit_set,
    onu_fe_port_maclimit_set_cmd,
    "port maclimit <port_list> {[0|<1-16>]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port maclimit information\n"
    "Please input the port number\n"
    "Config onu FE port not maclimit\n"    
    "Please input size(1-16)\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


/*added by wutw*/
LDEFUN  (
    onu_fe_port_show,
    onu_fe_port_show_cmd,
    "port link_show",    
    "Show or config onu FE port information\n"
    "Show onu FE port link status\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}



/*added by wutw*/
/*  modified by chenfj 2007-11-12
	问题单#5709: mirror_from不支持mirror all 选项，ONU串口支持，OLT侧不支持
	*/
LDEFUN  (
    onu_fe_port_mirror_from_set,
    onu_fe_port_mirror_from_set_cmd,
    "port mirror_from {[i|e|a] <port_list> [0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE mirror port enable\n"
    "ingress\n"
    "egress\n"
    "All direction\n"
    "Plseas input the port number\n"
    "not a source mirror\n"
    "as a source mirror\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


/*added by wutw*/
LDEFUN  (
    onu_fe_port_mirror_to_set,
    onu_fe_port_mirror_to_set_cmd,
    "port mirror_to {[i|e] [<port_list>|0]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE object mirror port\n"
    "ingress\n"
    "egress\n"
    "Plseas input the port number\n"
    "Delete the mirror port\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


/*added by wutw*/
LDEFUN  (
    onu_fe_port_mirror_show,
    onu_fe_port_mirror_show_cmd,
    "port mirror_show",    
    "Show or config onu FE port information\n"
    "Show onu FE mirror port information\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

/*added by wangxy 2007-05-15*/
/*FE port config*/
LDEFUN  (
    onu_fe_port_ingress_rate_limit_base,
    onu_fe_port_ingress_rate_limit_base_cmd,
    "port ingress_rate_limit_base {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port ingress rate limit base mechanism\n"
    "Priority based\n"
    "Burst Size based\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

/*added by wangxy 2007-11-06*/
LDEFUN  (
    onu_fe_port_link_mon,
    onu_fe_port_link_mon_cmd,
    "port link_mon {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port link monitor information\n"
    "disable the monitor function\n"
    "enable the monitor function\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

/*added by wangxy 2007-11-06*/
LDEFUN  (
    onu_fe_port_mode_mon,
    onu_fe_port_mode_mon_cmd,
    "port mode_mon {[0|1]}*1",    
    "Show or config onu FE port information\n"
    "Show or config onu FE port mode monitor\n"
    "disable the monitor function\n"
    "enable the monitor function\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}



/*QoS*/
#if qos
#endif
DEFUN  (
    onu_qos_def_pri,
    onu_qos_def_pri_cmd,
    "qos def_pri <port_list> {<0-7>}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu FE port default 802.1p priority\n"
    "Please input the port number\n"
    "Please input def_pri(0-7)\n"
    )
{	
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "qos def_pri" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid, ulRet;
    ULONG suffix = 0;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {
            ULONG prio = VOS_StrToUL(argv[1], NULL, 10);
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(OnuMgt_SetPortDefPriority(ponID, ulOnuid-1, port, prio) != VOS_OK)
                {
#if 0                    
                    vty_out(vty, "\r\nqos port default prio ERROR\r\n");
#else
                    error_flag = 1;
#endif
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
        }
        else
        {
            if(!isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                char *pbuf = NULL;
                USHORT len;
                if(OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            vty_big_out(vty, len, "%s", pbuf);
                   /* vty_out(vty, "\r\n%s\r\n", pbuf);*/
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
            else
            {
                    ULONG prio = 0;
                    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                    {
                        if(getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_default_priority, &prio) == VOS_OK)
                            vty_out(vty, "\r\nport %d default priority %d\r\n", port, prio);
                        else
                            vty_out(vty, "\r\nretrieve port %d default prio ERROR\r\n", port);
                    }
                    END_PARSE_PORT_LIST_TO_PORT()


            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*) vty->onuconfptr;
        ULONG prio = 0;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
		
        if (argc == 2)
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_default_priority, VOS_StrToUL(argv[1], NULL, 10))!=VOS_OK)
                    vty_out(vty, "\r\nqos port default prio ERROR\r\n");
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_default_priority, &prio) == VOS_OK)
                    vty_out(vty, "\r\nport %d default priority %d\r\n", port, prio);
                else
                    vty_out(vty, "\r\nretrieve port %d default prio ERROR\r\n", port);          
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
		{
		RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos def_pri !\r\n");	

	return CMD_SUCCESS;
#endif
}

/*已注释*/
#if 0
DEFUN  (
    onu_qos_def_port_tc,
    onu_qos_def_port_tc_cmd,
    "qos def_port_tc <port_list> {<0-3>}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu FE port default traffic_calss priority\n"
    "Please input the port number\n"
    "Please input traffic_class(0-3)\n"
    )
{
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
		{
		return CMD_WARNING;
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos def_port_tc !\r\n");	


	return CMD_SUCCESS;
}
#endif

DEFUN  (
    onu_qos_user_pri_reg,
    onu_qos_user_pri_reg_cmd,
    "qos user_pri_reg <port_list> <0-7> {<0-7>}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu FE port replacing 802.1p priority\n"
    "Please input the port number\n"
    "Please input old_pri(0-7)\n"
    "Please input new_pri(0-7)\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "qos user_pri_reg" );
#elif defined __RPC_ONU_COMMAND
    ULONG devIdx;
    ULONG brdIdx;
    ULONG ethIdx;
    int oldprio;
    int newprio;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG pon;
    ULONG onu;
    int ponid = 0;
    ULONG en;
    ULONG en1;
    ULONG suffix;
	ULONG ulIfIndex =(ULONG) vty->index;
    if(IsProfileNodeVty(ulIfIndex, &suffix))
    {

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], ethIdx)
	
        if(argc == 3)
        {
            oldprio= VOS_AtoI(argv[1]);
            newprio= VOS_AtoI(argv[2]);
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], ethIdx)
            if((getOnuConfPortSimpleVarByPtr(suffix,vty->onuconfptr, ethIdx, sv_enum_port_qoset_user_enable,&en)==VOS_OK)&&
                (getOnuConfPortSimpleVarByPtr(suffix,vty->onuconfptr, ethIdx, sv_enum_port_qoset_ip_enable,&en1)==VOS_OK))
            {
                if(en&&!en1)
                {
                    if(setOnuConfQosPrioReplaceByPtr(suffix, vty->onuconfptr, ethIdx, oldprio,newprio) != VOS_OK)
                        vty_out(vty, "FE port %d replacing 802.1p priority set fail!!\r\n", ethIdx);
                }
                else
                    vty_out(vty, "FE port %d replacing 802.1p priority set fail!!\r\n", ethIdx);
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            int newpri;
            int flag = 0;
            oldprio= VOS_AtoI(argv[1]);
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], ethIdx)               
            if((getOnuConfPortSimpleVarByPtr(suffix,vty->onuconfptr, ethIdx, sv_enum_port_qoset_user_enable,&en)==VOS_OK)&&
                (getOnuConfPortSimpleVarByPtr(suffix,vty->onuconfptr, ethIdx, sv_enum_port_qoset_ip_enable,&en1)==VOS_OK))
            {
                if(en&&!en1)
                {
                    if(getOnuConfQosPrioReplaceByPtr(suffix, vty->onuconfptr, ethIdx, oldprio,&newpri) == VOS_OK)
                    {
                        flag = 1;
                        vty_out(vty, "FE port %d old priority is %d  new priority is %d\r\n",ethIdx,oldprio,newpri);      
                    }
                    if(!flag)
                        vty_out(vty, "FE port %d replacing 802.1p priority get fail!!\r\n", ethIdx);
                }
                else
                    vty_out(vty, "FE port %d replacing 802.1p priority get fail!!\r\n", ethIdx);
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
    }
    else
    {
        if (parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;
        ulSlot = brdIdx;
        ulPort = pon;
        ulOnuid = onu;
        ponid = GetPonPortIdxBySlot(brdIdx, pon);
        if(ponid == -1)
        {
            vty_out(vty, "wrong onu parameters!\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(brdIdx, pon, onu, argv[0], ethIdx)

        if(argc == 3)
        {

            ONU_CONF_WRITEABLE_CHECK

            oldprio= VOS_AtoI(argv[1]);
            newprio= VOS_AtoI(argv[2]);
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], ethIdx)
            if((getOnuConfPortSimpleVar(ponid, ulOnuid-1, ethIdx, sv_enum_port_qoset_user_enable,&en)==VOS_OK)&&
            (getOnuConfPortSimpleVar(ponid, ulOnuid-1, ethIdx, sv_enum_port_qoset_ip_enable,&en1)==VOS_OK))
            {
                if(en&&!en1)
                {
                    if(OnuMgt_SetPortNewPriority(ponid, ulOnuid-1, ethIdx, oldprio, newprio) != VOS_OK)
                        vty_out(vty, "FE port %d replacing 802.1p priority set fail!!\r\n", ethIdx);
                }
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            if (isCtcOnu(brdIdx, pon, onu))
            {

                    int newpri;
                    int flag = 0;
                    oldprio= VOS_AtoI(argv[1]);
                    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], ethIdx)
                    if((getOnuConfPortSimpleVar(ponid, ulOnuid-1, ethIdx, sv_enum_port_qoset_user_enable,&en)==VOS_OK)&&
                    (getOnuConfPortSimpleVar(ponid, ulOnuid-1, ethIdx, sv_enum_port_qoset_ip_enable,&en1)==VOS_OK))
                    {
                        if(en&&!en1)
                        {
                            if(getOnuConfQosPrioReplace(ponid, ulOnuid-1,ethIdx, oldprio,&newpri) == VOS_OK)
                            {
                                flag =1;
                                vty_out(vty, "FE port %d old priority is %d  new priority is %d\r\n",ethIdx, oldprio,newpri);
                            }
                            if(!flag)
                                vty_out(vty, "FE port %d replacing 802.1p priority get fail!!\r\n", ethIdx);
                        }
                        else
                            vty_out(vty, "FE port %d replacing 802.1p priority get fail!!\r\n", ethIdx);
                    }
                    END_PARSE_PORT_LIST_TO_PORT()

            }
            else
            {
                char *pbuf = NULL;
                USHORT len;
                if (OnuMgt_CliCall(ponid, ulOnuid - 1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            vty_big_out(vty, len, "%s", pbuf);
                   /* vty_out(vty, "\r\n%s\r\n", pbuf);*/
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
		{
		RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos user_pri_reg  !\r\n");	

	return CMD_SUCCESS;
#endif
}


DEFUN  (
    onu_qos_user_pri_reg_show,
    onu_qos_user_pri_reg_show_cmd,
    "qos user_pri_reg <port_list>",    
    "Show onu QoS information\n"
    "Show or config onu FE port replacing 802.1p priority\n"
    "Please input the port number\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "qos user_pri_reg" );
#elif defined __RPC_ONU_COMMAND
    ULONG devIdx;
    ULONG brdIdx;
    ULONG ethIdx;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG pon;
    ULONG onu;
    int ponid = 0;
    ULONG en = 0;
    ULONG en1 = 0;
    ULONG suffix;
	ULONG ulIfIndex =(ULONG) vty->index;
    if(IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int i = 0;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], ethIdx)
		
        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], ethIdx)               
        if((getOnuConfPortSimpleVarByPtr(suffix,vty->onuconfptr, ethIdx, sv_enum_port_qoset_user_enable,&en)==VOS_OK)&&
                (getOnuConfPortSimpleVarByPtr(suffix,vty->onuconfptr, ethIdx, sv_enum_port_qoset_ip_enable,&en1)==VOS_OK))
        {
            if(en&&!en1)
            {
               	ONUConfigData_t *pd = ( ONUConfigData_t *)vty->onuconfptr;
                ONU_CONF_SEM_TAKE
                {
                    if(!onuConfCheckByPtr(suffix, pd))
                    {
                        vty_out(vty, "\r\nFE port %d replacing 802.1p priority\r\n", ethIdx);               
                        for(i=0;i<8;i++)
                        {
                            if(pd->portconf[ethIdx-1].qosPrioReplace[i]!=0)
                            {
                                vty_out(vty, "old priority is %d  new priority is %d\r\n", i,pd->portconf[ethIdx-1].qosPrioReplace[i]-1);
                            }
                        }
                    }
                    else
                        vty_out(vty, "FE port %d replacing 802.1p priority get fail!!\r\n", ethIdx);

                }
                ONU_CONF_SEM_GIVE;
            }
            else
                vty_out(vty, "FE port %d replacing 802.1p priority get fail!!\r\n", ethIdx);
        }
        else
            vty_out(vty, "FE port %d replacing 802.1p priority get fail!!\r\n", ethIdx);
        END_PARSE_PORT_LIST_TO_PORT()
    }
    else
    {
        if (parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;
        ulSlot = brdIdx;
        ulPort = pon;
        ulOnuid = onu;
        ponid = GetPonPortIdxBySlot(brdIdx, pon);
        if(ponid == -1)
        {
            vty_out(vty, "wrong onu parameters!\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(brdIdx, pon, onu, argv[0], ethIdx)
		
        ONU_CONF_WRITEABLE_CHECK
        if (isCtcOnu(brdIdx, pon, onu))
        {
            int i = 0;
            int oldprio,newprio;
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], ethIdx)               
            if((getOnuConfPortSimpleVar(ponid, ulOnuid-1,ethIdx, sv_enum_port_qoset_user_enable,&en)==VOS_OK)&&
                (getOnuConfPortSimpleVar(ponid, ulOnuid-1, ethIdx,sv_enum_port_qoset_ip_enable,&en1)==VOS_OK))
            {
                if(en&&!en1)
                {
                   vty_out(vty, "\r\nFE port %d replacing 802.1p priority\r\n", ethIdx);               
                   for(i=0;i<8;i++)
                   {
                        if( showOnuConfQosReplace(ponid, ulOnuid-1, ethIdx, i,&oldprio,&newprio)==VOS_OK)
                        {
                          vty_out(vty, "old priority is %d  new priority is %d\r\n", oldprio,newprio);
                        }
                   }
                }
                else
                    vty_out(vty, "FE port %d replacing 802.1p priority get fail!!\r\n", ethIdx);
            }
            else
                vty_out(vty, "FE port %d replacing 802.1p priority get fail!!\r\n", ethIdx);
            END_PARSE_PORT_LIST_TO_PORT()      
        }
        else
        {
            char *pbuf = NULL;
            USHORT len;
            if (OnuMgt_CliCall(ponid, ulOnuid - 1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            vty_big_out(vty, len, "%s", pbuf);
                   /* vty_out(vty, "\r\n%s\r\n", pbuf);*/
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }
    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
		{
		RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos user_pri_reg  !\r\n");	

	return CMD_SUCCESS;
#endif
}


DEFUN  (
    onu_qos_user_pri_tc_show,
    onu_qos_user_pri_tc_show_cmd,
    "qos user_pri_tc",    
    "Show or config onu QoS information\n"
    "Show or config onu 802.1p priority associated with traffic-class queue\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "qos user_pri_tc",NULL );
#elif defined __RPC_ONU_COMMAND
    ULONG devIdx;
    ULONG brdIdx;

    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG pon;
    ULONG onu;
    int ponid = 0;
    ULONG suffix;
	ULONG ulIfIndex =(ULONG) vty->index;
    if(IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int i = 0;
        int flag = 0;
        ONUConfigData_t *pd = ( ONUConfigData_t *)vty->onuconfptr;
        ONU_CONF_SEM_TAKE
        {
            if(!onuConfCheckByPtr(suffix, pd))
            {
                vty_out(vty, "\r\n802.1p priority associated with traffic-class queue\r\n");               
                for(i=0;i<8;i++)
                {
                    if(pd->qosMap.queue[i]!=0)
                    {
                        flag = 1;
                        vty_out(vty, "802.1p priority is %d  traffic-class queue is %d\r\n", i,pd->qosMap.queue[i]-1);
                    }
                }
            }
        }
        ONU_CONF_SEM_GIVE;
        if(!flag)
            vty_out(vty, "\r\n802.1p priority associated with traffic-class queue get error!\r\n");               
    }
    else
    {
        if (parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;
        ulSlot = brdIdx;
        ulPort = pon;
        ulOnuid = onu;
        ponid = GetPonPortIdxBySlot(brdIdx, pon);
        if(ponid == -1)
        {
            vty_out(vty, "wrong onu parameters!\r\n");
            return CMD_WARNING;
        }
        ONU_CONF_WRITEABLE_CHECK
        if (isCtcOnu(brdIdx, pon, onu))
        {
            int i = 0;
            int prio;
            int queue;
            int flag = 0;
            for(i=0;i<8;i++)
            {
                if( showOnuConfQosPrioToQueue(ponid, ulOnuid-1,i,&prio,&queue)==VOS_OK)
                {
                    flag = 1;
                    vty_out(vty, "802.1p priority is %d  traffic-class queue is %d\r\n", prio,queue);
                }
            }
            if(!flag)
                vty_out(vty, "\r\n802.1p priority associated with traffic-class queue get error!\r\n");               
        }
        else
        {
            char *pbuf = NULL;
            USHORT len;
            if (OnuMgt_CliCall(ponid, ulOnuid - 1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            vty_big_out(vty, len, "%s", pbuf);
                   /* vty_out(vty, "\r\n%s\r\n", pbuf);*/
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }
    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos user_pri_tc !\r\n");	


	return CMD_SUCCESS;
#endif
}

/* 问题单:6200
	GT810和GT816的qos设置命令队列只有0~3
	GT811/812只有4个队列,其他类型ONU,有8个队列
*/

 DEFUN  (
    onu_qos_user_pri_tc,
    onu_qos_user_pri_tc_cmd,
    "qos user_pri_tc <0-7> {<0-7>}*1",
    "Show or config onu QoS information\n"
    "Show or config onu 802.1p priority associated with traffic-class queue\n"
    "user_pri,range:0-7\n"
    "traffic_class,range:0-7\n"
    /*"traffic_class,range:0-7;but for "DEVICE_TYPE_NAME_GT811_STR"/"DEVICE_TYPE_NAME_GT812_STR",the range is 0-3\n"*/
    )

{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "qos user_pri_tc",NULL );
#elif defined __RPC_ONU_COMMAND
    ULONG devIdx;
    ULONG brdIdx;

    int prio;
    int queue;
    ULONG   ulSlot, ulPort, ulOnuid;

    ULONG pon;
    ULONG onu;
    int ponid = 0;
    ULONG suffix;
	ULONG ulIfIndex =(ULONG) vty->index;

    if(IsProfileNodeVty(ulIfIndex, &suffix))
    {
        if(argc == 2)
        {
            prio= VOS_AtoI(argv[0]);
            queue= VOS_AtoI(argv[1]);
            if(setOnuConfQosPrioToQueueByPtr(suffix, vty->onuconfptr, prio,queue) != VOS_OK)
                vty_out(vty, "802.1p priority associated with traffic-class queue set fail!!\r\n");
 
        }
        else
        {
            int flag = 0;
            prio= VOS_AtoI(argv[0]);
            if(getOnuConfQosPrioToQueueByPtr(suffix, vty->onuconfptr, prio,&queue) == VOS_OK)
            {
                flag = 1;
                vty_out(vty, "802.1p priority is %d  traffic-class queue is %d\r\n", prio,queue);
            }
            else
                vty_out(vty, "802.1p priority associated with traffic-class queue get fail!!\r\n");
        }
            
    }

    
    else
    {
        if (parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;

        ulSlot = brdIdx;
        ulPort = pon;
        ulOnuid = onu;

        ponid = GetPonPortIdxBySlot(brdIdx, pon);
        if(ponid == -1)
        {
            vty_out(vty, "wrong onu parameters!\r\n");
            return CMD_WARNING;
        }

        if(argc == 2)
        {
            ONU_CONF_WRITEABLE_CHECK

            prio= VOS_AtoI(argv[0]);
            queue= VOS_AtoI(argv[1]);
            if(OnuMgt_SetQosPrioToQueue(ponid, ulOnuid-1, prio, queue) != VOS_OK)
                vty_out(vty, "802.1p priority associated with traffic-class queue set fail!!\r\n");
        }
        else
        {
            if (isCtcOnu(brdIdx, pon, onu))
            {


                    prio= VOS_AtoI(argv[0]);
                    if(getOnuConfQosPrioToQueue(ponid, onu-1, prio,&queue) == VOS_OK)
                    {
                        vty_out(vty, "802.1p priority is %d  traffic-class queue is %d\r\n", prio,queue);
                    }
                    else
                        vty_out(vty, "802.1p priority associated with traffic-class queue get fail!!\r\n");

            }
            else
            {
                char *pbuf = NULL;
                USHORT len;

                if (OnuMgt_CliCall(ponid, onu - 1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            vty_big_out(vty, len, "%s", pbuf);
                   /* vty_out(vty, "\r\n%s\r\n", pbuf);*/
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos user_pri_tc !\r\n");	


	return CMD_SUCCESS;
#endif
}

LDEFUN  (
    onu_qos_def_pri,
    onu_qos_def_pri_cmd,
    "qos def_pri <port_list> {<0-7>}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu FE port default 802.1p priority\n"
    "Please input the port number\n"
    "Please input def_pri(0-7)\n",
    ONU_NODE
    )
{	
	return CMD_SUCCESS;
}

/*已注释*/
#if 0
DEFUN  (
    onu_qos_def_port_tc,
    onu_qos_def_port_tc_cmd,
    "qos def_port_tc <port_list> {<0-3>}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu FE port default traffic_calss priority\n"
    "Please input the port number\n"
    "Please input traffic_class(0-3)\n"
    )
{
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
		{
		return CMD_WARNING;
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos def_port_tc !\r\n");	


	return CMD_SUCCESS;
}
#endif

LDEFUN  (
    onu_qos_user_pri_reg,
    onu_qos_user_pri_reg_cmd,
    "qos user_pri_reg <port_list> <0-7> {<0-7>}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu FE port replacing 802.1p priority\n"
    "Please input the port number\n"
    "Please input old_pri(0-7)\n"
    "Please input new_pri(0-7)\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


LDEFUN  (
    onu_qos_user_pri_reg_show,
    onu_qos_user_pri_reg_show_cmd,
    "qos user_pri_reg <port_list>",    
    "Show onu QoS information\n"
    "Show or config onu FE port replacing 802.1p priority\n"
    "Please input the port number\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


LDEFUN  (
    onu_qos_user_pri_tc_show,
    onu_qos_user_pri_tc_show_cmd,
    "qos user_pri_tc",    
    "Show or config onu QoS information\n"
    "Show or config onu 802.1p priority associated with traffic-class queue\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

/* 问题单:6200
	GT810和GT816的qos设置命令队列只有0~3
	GT811/812只有4个队列,其他类型ONU,有8个队列
*/

 LDEFUN  (
    onu_qos_user_pri_tc,
    onu_qos_user_pri_tc_cmd,
    "qos user_pri_tc <0-7> {<0-7>}*1",
    "Show or config onu QoS information\n"
    "Show or config onu 802.1p priority associated with traffic-class queue\n"
    "user_pri,range:0-7\n"
    "traffic_class,range:0-7\n",
    ONU_NODE
    /*"traffic_class,range:0-7;but for "DEVICE_TYPE_NAME_GT811_STR"/"DEVICE_TYPE_NAME_GT812_STR",the range is 0-3\n"*/
    )

{
	return CMD_SUCCESS;
}


/* 问题单:6200
	GT810和GT816的qos设置命令队列只有0~3
	GT811/812只有4个队列,其他类型ONU,有8个队列
*/
/*问题单7743，添加了提示，GT831B也只有4个队列0-3*/
#ifdef _OEM_TYPE_CLI_
char  qos_dscp_tc[500]={0};
 DEFUN  (
    onu_qos_tos_tc,
    onu_qos_tos_tc_cmd,
    "qos dscp_tc <0-63> {<0-7>}*1",  
    qos_dscp_tc
    )
#else 
DEFUN  (
    onu_qos_tos_tc,
    onu_qos_tos_tc_cmd,
    "qos dscp_tc <0-63> {<0-7>}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu IP-DSCP associated with traffi-class queue\n"
    "tos, range:0-63\n"
    /*"traffic_class,range:0-7;but for "DEVICE_TYPE_NAME_GT811_STR"/"DEVICE_TYPE_NAME_GT812_STR",range is 0-3\n" */
    "traffic_class,range:0-7\n"
    )
#endif
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "qos dscp_tc",NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 2)
        {

            int dscpnum = VOS_StrToUL(argv[0], NULL, 10);
            int queue = VOS_StrToUL(argv[1], NULL, 10);

            ONU_CONF_WRITEABLE_CHECK
            if(VOS_OK != OnuMgt_SetQosDscpToQueue(ponID, ulOnuid-1, dscpnum,queue) )
            {
#if 0                
                vty_out(vty, "IP-DSCP associated with traffi-class queue set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);        
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                int dscpnum = VOS_StrToUL(argv[0], NULL, 10);
                int queue;

                    if(getOnuConfQosDscpToQueue(ponID, ulOnuid-1,dscpnum, &queue) == VOS_OK)
                    {
                        vty_out(vty, "IP-DSCP associated with traffi-class queue\r\n");
                        vty_out(vty, "tos is %d traffi-class queue is %d\r\n",dscpnum,queue);
                    }
                    else
                        vty_out(vty, "IP-DSCP associated with traffi-class queue get fail!\r\n\r\n");

            }
            else
            {
                char *pRecv;
                USHORT len;
                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                 }
            }
        }
    }
    
    else
    {
        int dscpnum = VOS_StrToUL(argv[0], NULL, 10);
        int queue;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(argc == 2)
        { 
            queue = VOS_StrToUL(argv[1], NULL, 10);
            if(setOnuConfQosDscpToQueueByPtr(suffix, pd,dscpnum,queue))
                vty_out(vty, "IP-DSCP associated with traffi-class queue set fail!\r\n");
        }
        else
        {
            if(getOnuConfQosDscpToQueueByPtr(suffix, pd , dscpnum, &queue)==VOS_OK)
            {
                vty_out(vty, "IP-DSCP associated with traffi-class queue\r\n");
                vty_out(vty, "tos is %d traffi-class queue is %d\r\n",dscpnum,queue);
            }
            else
                vty_out(vty, "IP-DSCP associated with traffi-class queue get fail!\r\n\r\n");
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos dscp_tc  !\r\n");	


	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_qos_tos_tc_show,
    onu_qos_tos_tc_show_cmd,
    "qos dscp_tc",    
    "Show or config onu QoS information\n"
    "Show or config onu IP-DSCP associated with traffi-class queue\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "qos dscp_tc",NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    int i = 0,j = 0;
    int flag = 0;
    ulIfIndex =(ULONG) vty->index;
    
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }
        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {
            int dscpnum;
            int queue;
            for(i=0;i<8;i++)
            {
                for(j=0;j<8;j++)
                {
                    if( showOnuConfQosDscpToQueue(ponID,ulOnuid-1,i,j,&dscpnum,&queue)==VOS_OK)
                    {
                        flag = 1;
                        vty_out(vty, "tos is %d traffi-class queue is %d\r\n",dscpnum,queue);
                    }
                }
            }
            if(!flag)
                vty_out(vty, "IP-DSCP associated with traffi-class queue get fail!\r\n\r\n");
        }
        else
        { 
            char *pRecv;
            USHORT len;
            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
		            vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }
    
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        int k = 0;
        ONU_CONF_SEM_TAKE
        {
            if(!onuConfCheckByPtr(suffix, pd))
            {
                vty_out(vty, "IP-DSCP associated with traffi-class queue\r\n");
                for(i=0;i<8;i++)
                {
                    for(j=0;j<8;j++)
                    {
                        if(pd->qosMap.qosDscpQueue[i][j])
                        {
                            for(k=0;k<8;k++)
                            {
                                if(pd->qosMap.qosDscpQueue[i][j]&(1<<k))
                                flag = 1;
                                vty_out(vty, "tos is %d traffi-class queue is %d\r\n",j*8+k,i);
                            }
                        }
                    }
                }
            }
        }
        ONU_CONF_SEM_GIVE;           
        if(!flag)
            vty_out(vty, "IP-DSCP associated with traffi-class queue get fail!\r\n\r\n");
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos dscp_tc  !\r\n");	


	return CMD_SUCCESS;
#endif
}



DEFUN  (
    onu_qos_algorithm,
    onu_qos_algorithm_cmd,
    "qos algorithm {[wrr|spq]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu QoS algorithm\n"
    "Weight Round Robin\n"
    "Strict Priority Queuing\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "qos algorithm",NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {

            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK != OnuMgt_SetQosAlgorithm(ponID, ulOnuid-1,VOS_MemCmp(argv[0], "wrr", 3)?1:0))
            {
#if  0               
                vty_out(vty, "qos algorithm set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                    ULONG uv = 0;
                    if(getOnuConfSimpleVar(ponID, ulOnuid-1, sv_enum_qosAlgorithm, &uv) == VOS_OK)
                        vty_out(vty, "QoS algorithm is %s\r\n\r\n", uv?"Strict Priority Queuing":"Weight Round Robin");
                    else
                        vty_out(vty, "QoS algorithm get fail\r\n\r\n");
            }
            else
            {
               char *pRecv;
               USHORT len;

               if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
               {
		            vty_big_out(vty, len, "%s", pRecv);
               }
               else
               {
                   vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
               }
            }
        }
    }
    
    else
    {
        ULONG uv = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(argc == 1)
        {
            uv = VOS_MemCmp(argv[0], "wrr", 3)?1:0;

            if(setOnuConfSimpleVarByPtr(suffix, pd, sv_enum_qosAlgorithm, uv)!=VOS_OK)
                vty_out(vty, "qos algorithm set fail!\r\n");
        }
        else
        { 
            if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_qosAlgorithm, &uv)== VOS_OK)
               vty_out(vty, "QoS algorithm is %s\r\n\r\n", uv?"Strict Priority Queuing":"Weight Round Robin");
            else
               vty_out(vty, "QoS algorithm get fail\r\n\r\n");
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos algorithm  !\r\n");	


	return CMD_SUCCESS;
#endif
}



DEFUN  (
    onu_qos_rule,
    onu_qos_rule_cmd,
    "qos rule <port_list> {[user|ip]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu current running Qos rule(802.1p or IP-DSCP)\n"
    "Please input the port number\n"
    "IEEE tag priority\n"
    "IP priority\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "qos rule" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port, suffix;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
    ULONG mode = 0;
    
    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {
            mode = VOS_MemCmp(argv[1],"user", 4)?1:0;
            ONU_CONF_WRITEABLE_CHECK
            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                if(OnuMgt_SetPortQosRuleType(ponID, ulOnuid-1, port, mode) )
                {
#if 0                    
                    vty_out(vty, "port %d current running Qos rule set fail!\r\n", port);
#else
                    error_flag = 1;
#endif
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                     BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                        {
                            if(VOS_OK == getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_qoset_rule, &mode))
                                vty_out(vty, "\r\nport %d current running Qos rule is %s\r\n", port, mode?"IP priority":"IEEE tag priority");
                            else
                                vty_out(vty, "\r\nport current running Qos rule get error!\r\n");
                        }
                     END_PARSE_PORT_LIST_TO_PORT()


            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
		
        if(argc == 2)
        {
            mode = VOS_MemCmp(argv[1],"user", 4)?1:0;
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_qoset_rule, mode))
                    vty_out(vty, "port %d current running Qos rule set fail!\r\n", port);
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(getOnuConfPortSimpleVarByPtr(suffix, pd , port, sv_enum_port_qoset_rule, &mode))
                    vty_out(vty, "port %d current running Qos rule get fail!\r\n", port);
                else
    		        vty_out(vty, "\r\nport %d current running Qos rule is %s\r\n", port, mode?"IP priority":"IEEE tag priority");
            }
            END_PARSE_PORT_LIST_TO_PORT()
            
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }	
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
     if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
		{
		RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();
	
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos algorithm  !\r\n");	

	return CMD_SUCCESS;
#endif
}



DEFUN  (
    onu_qos_ip_pri_en,
    onu_qos_ip_pri_en_cmd,
    "qos ip_pri_en <port_list> {[0|1]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu IP-DSCP priority enable status\n"
    "Please input the port number\n"
    "Disable\n"
    "Enable\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "qos ip_pri_en" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port, suffix;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
    ULONG mode = 0;
    
    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {
            mode = VOS_StrToUL(argv[1], NULL, 10);
            ONU_CONF_WRITEABLE_CHECK
            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                if(VOS_OK != OnuMgt_SetPortIpPriorityEnable(ponID, ulOnuid-1, port, mode) )
                {
#if 0                    
                    vty_out(vty, "port %d IP-DSCP priority enable set fail!\r\n", port);
#else
                    error_flag = 0;
#endif
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {

                     BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                        {
                            if(VOS_OK == getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_qoset_ip_enable, &mode))
                                vty_out(vty, "\r\nport %d IP-DSCP priority enable is %s\r\n", port, mode?"enable":"disable");
                            else
                                vty_out(vty, "\r\nport IP-DSCP priority enable get error!\r\n");
                        }
                     END_PARSE_PORT_LIST_TO_PORT()


            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
		
        if(argc == 2)
        {
            mode = VOS_StrToUL(argv[1], NULL, 10);
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_qoset_ip_enable, mode))
                    vty_out(vty, "port %d IP-DSCP priority enable set fail!\r\n", port);
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(getOnuConfPortSimpleVarByPtr(suffix, pd , port, sv_enum_port_qoset_ip_enable, &mode))
                    vty_out(vty, "port %d IP-DSCP priority enable get fail!\r\n", port);
                else
    		        vty_out(vty, "\r\nport %d IP-DSCP priority enable is %s\r\n", port, mode?"enable":"disable");
            }
            END_PARSE_PORT_LIST_TO_PORT()
            
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }	
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
		{
		RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();
	
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos algorithm  !\r\n");	

	return CMD_SUCCESS;
#endif
}


DEFUN  (
    onu_qos_user_pri_en,
    onu_qos_user_pri_en_cmd,
    "qos user_pri_en <port_list> {[0|1]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu 802.1p priority enable status\n"
    "Please input the port number\n"
    "Disable\n"
    "Enable\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "qos user_pri_en" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port, suffix;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
    ULONG mode = 0;
    
    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {
            mode = VOS_StrToUL(argv[1], NULL, 10);
            ONU_CONF_WRITEABLE_CHECK
            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                if(OnuMgt_SetPortUserPriorityEnable(ponID, ulOnuid-1, port, mode) )
                {
#if 0                    
                    vty_out(vty, "port %d 802.1p priority enable set fail!\r\n", port);
#else
                    error_flag = 1;
#endif
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                {
                    if(VOS_OK == getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_qoset_user_enable, &mode))
                        vty_out(vty, "\r\nport %d 802.1p priority is %s\r\n", port, mode?"enable":"disable");
                    else
                        vty_out(vty, "\r\nport 802.1p priority enable get error!\r\n");
                }
                END_PARSE_PORT_LIST_TO_PORT()
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
		
        if(argc == 2)
        {
            mode = VOS_StrToUL(argv[1], NULL, 10);
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_qoset_user_enable, mode))
                    vty_out(vty, "port %d 802.1p priority enable set fail!\r\n", port);
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(getOnuConfPortSimpleVarByPtr(suffix, pd , port, sv_enum_port_qoset_user_enable, &mode))
                    vty_out(vty, "port %d 802.1p priority enable get fail!\r\n", port);
                else
    		        vty_out(vty, "\r\nport %d 802.1p priority is %s\r\n", port, mode?"enable":"disable");
            }
            END_PARSE_PORT_LIST_TO_PORT()
            
        }
    }

    return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }	
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
		{
		RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();
	
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%qos algorithm  !\r\n");	

	return CMD_SUCCESS;
#endif
}

#ifdef _OEM_TYPE_CLI_
char  qos_dscp_tc[500]={0};
 LDEFUN  (
    onu_qos_tos_tc,
    onu_qos_tos_tc_cmd,
    "qos dscp_tc <0-63> {<0-7>}*1",  
    qos_dscp_tc,
    ONU_NODE
    )
#else 
LDEFUN  (
    onu_qos_tos_tc,
    onu_qos_tos_tc_cmd,
    "qos dscp_tc <0-63> {<0-7>}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu IP-DSCP associated with traffi-class queue\n"
    "tos, range:0-63\n"
    /*"traffic_class,range:0-7;but for "DEVICE_TYPE_NAME_GT811_STR"/"DEVICE_TYPE_NAME_GT812_STR",range is 0-3\n" */
    "traffic_class,range:0-7\n",
    ONU_NODE
    )
#endif
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_qos_tos_tc_show,
    onu_qos_tos_tc_show_cmd,
    "qos dscp_tc",    
    "Show or config onu QoS information\n"
    "Show or config onu IP-DSCP associated with traffi-class queue\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}



LDEFUN  (
    onu_qos_algorithm,
    onu_qos_algorithm_cmd,
    "qos algorithm {[wrr|spq]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu QoS algorithm\n"
    "Weight Round Robin\n"
    "Strict Priority Queuing\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}



LDEFUN  (
    onu_qos_rule,
    onu_qos_rule_cmd,
    "qos rule <port_list> {[user|ip]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu current running Qos rule(802.1p or IP-DSCP)\n"
    "Please input the port number\n"
    "IEEE tag priority\n"
    "IP priority\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}



LDEFUN  (
    onu_qos_ip_pri_en,
    onu_qos_ip_pri_en_cmd,
    "qos ip_pri_en <port_list> {[0|1]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu IP-DSCP priority enable status\n"
    "Please input the port number\n"
    "Disable\n"
    "Enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


LDEFUN  (
    onu_qos_user_pri_en,
    onu_qos_user_pri_en_cmd,
    "qos user_pri_en <port_list> {[0|1]}*1",    
    "Show or config onu QoS information\n"
    "Show or config onu 802.1p priority enable status\n"
    "Please input the port number\n"
    "Disable\n"
    "Enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

#if loopback
#endif

/*Loopback*/
DEFUN  (
    onu_loopback_uni_test,
    onu_loopback_uni_test_cmd,
    "loopback uni_test",    
    "Show or config onu loopback information\n"
    "Loopback uni_test\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "loopback uni_test",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1	 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%loopback uni_test !\r\n");	


	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_loopback_port_en,
    onu_loopback_port_en_cmd,
    "eth-loop port_en <port_list> {[0|1]}*1",
    /*"loopback port_en <1-24> {[0|1]}*1",*/    
    "Show or config onu loopback information\n"
    "Show or config onu loopback port_en information\n"
    "Please input the port number\n"
    "Disable\n"
    "Enable\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	VOS_MemCpy(&vty->buf[0], "loopback", 8);
	return onu_fe_cli_process( vty, argv[0], "eth-loop port_en" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {
            ULONG portlist = 0;

            ONU_CONF_WRITEABLE_CHECK

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                portlist |= 1<<(port-1);
            }
            END_PARSE_PORT_LIST_TO_PORT();

            if(VOS_OK == OnuMgt_SetPortLoopDetect(ponID, ulOnuid-1, portlist, VOS_StrToUL(argv[1], NULL, 10)) )
            {
            }
            else
            {
#if 0                
                vty_out(vty, "port %d loop_detect_enable set error!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);    
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                    ULONG enable= 0;
                    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                    {
                        if(VOS_OK == getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_loop_detect_enable, &enable))
                            vty_out(vty, "\r\nport %d loop_detect_enable is %s\r\n", port, enable?"disable":"enable");
                        else
                            vty_out(vty, "\r\nport loop_detect_enable get error!\r\n");
                    }
                    END_PARSE_PORT_LIST_TO_PORT()
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG enable= 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
		
        if(argc == 2)
        {
            enable= VOS_StrToUL(argv[1], NULL, 10);

            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_loop_detect_enable, enable))
                vty_out(vty, "port %d loop_detect_enable set fail!\r\n", port);

            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(getOnuConfPortSimpleVarByPtr(suffix, pd , port, sv_enum_port_loop_detect_enable, &enable))
                vty_out(vty, "port %d loop_detect_enable get fail!\r\n", port);
            else
                vty_out(vty, "port %d loop_detect_enable %d\r\n", port ,enable);

            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length = 0;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
    {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
    }
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
		{
		RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();

	if (argc == 1)
		VOS_Sprintf(pBuff,"loopback port_en %s",argv[0]);
	else if (argc == 2)
	{
		VOS_Sprintf(pBuff,"loopback port_en %s %s",argv[0],argv[1]);
	}
	else
		vty_out( vty, "  %% Parameter error\r\n");
	
	length = VOS_StrLen(pBuff);
 
	/*
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	*/
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;
	
	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%eth-loop port_en !\r\n");	


	return CMD_SUCCESS;
#endif
}

LDEFUN  (
    onu_loopback_uni_test,
    onu_loopback_uni_test_cmd,
    "loopback uni_test",    
    "Show or config onu loopback information\n"
    "Loopback uni_test\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_loopback_port_en,
    onu_loopback_port_en_cmd,
    "eth-loop port_en <port_list> {[0|1]}*1",
    /*"loopback port_en <1-24> {[0|1]}*1",*/    
    "Show or config onu loopback information\n"
    "Show or config onu loopback port_en information\n"
    "Please input the port number\n"
    "Disable\n"
    "Enable\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}


#if statistics
#endif
/*Statistic*/
DEFUN  (
    onu_statistic_port_histogram,
    onu_statistic_port_histogram_cmd,
    "stat port_histogram {[0|1|2]}*1",    
    "Show or config onu Statistic information\n"
    "Show or config onu Statistic port_histogram information\n"
    "RX counter only\n"
    "TX counter only\n"
    "Both RX and TX counter\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "stat port_histogram",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%stat port_histogram !\r\n");	


	return CMD_SUCCESS;
#endif
}



DEFUN  (
    onu_statistic_port_flush,
    onu_statistic_port_flush_cmd,
    "stat port_flush {<port_list>}*1",    
    "Show or config onu Statistic information\n"
    "Clear onu Statistic information\n"
    "Please input port Number\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	char *parg = NULL;
	if( argc != 0 )
		parg = argv[0];
	return onu_fe_cli_process( vty, parg, "stat port_flush" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }
        
        if(argc == 1)
        {
            ULONG portlist = 0;

			VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

            ONU_CONF_WRITEABLE_CHECK

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                portlist |= 1<<(port-1);
            }
            END_PARSE_PORT_LIST_TO_PORT();

            if(VOS_OK == OnuMgt_SetPortStatFlush(ponID, ulOnuid-1, portlist, 1) )
            {
            }
            else
            {
#if 0                
                vty_out(vty, "port statistaics flush fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);    
#endif
            }
        }
        else
        {
            char *pRecv;
            USHORT len;

            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
            {
		            vty_big_out(vty, len, "%s", pRecv);
            }
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }
    else
    {
        
    }
    return CMD_SUCCESS;

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }

	if(argc != 0)
	{	/*
		ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    		if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
			return CMD_WARNING;
		*/
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
			RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%stat port_flush !\r\n");	


	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_statistic_port_show,
    onu_statistic_port_show_cmd,
    /*"stat port_show <1-24> {<0-31>}*1",  */
	"stat port_show <port_list>", 
    "Show or config onu Statistic information\n"
    "Show onu Statistic information\n"
    "Please input port Number\n"
    /*"Please input the counter number\n"*/
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "stat port_show" );
#elif defined __RPC_ONU_COMMAND
	return rpc_onu_fe_cli_process( vty, argv[0], "stat port_show" );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/

	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }

	/*added by wutw at 16 October*/
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
     if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
			RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%stat port_show  !\r\n");	


	return CMD_SUCCESS;
#endif
}



DEFUN  (
    onu_statistic_pas_flush,
    onu_statistic_pas_flush_cmd,
    "stat pas_flush",    
    "Show or config onu Statistic information\n"
    "Clear all the pon port, MPCP and OAM Statistic information\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "stat pas_flush",NULL );
#elif defined __RPC_ONU_COMMAND
    LONG lRet;
    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;
    ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
    if( ulRet !=VOS_OK )
        return CMD_WARNING;


    ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);

    lRet = OnuMgt_SetPasFlush(ponID, ulOnuid-1, 0);

    vty_out(vty, "\r\npas flush %s\r\n", lRet?"fail":"ok");

	return CMD_SUCCESS;
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%stat pas_flush !\r\n");	


	return CMD_SUCCESS;
#endif
}


LDEFUN  (
    onu_statistic_port_histogram,
    onu_statistic_port_histogram_cmd,
    "stat port_histogram {[0|1|2]}*1",    
    "Show or config onu Statistic information\n"
    "Show or config onu Statistic port_histogram information\n"
    "RX counter only\n"
    "TX counter only\n"
    "Both RX and TX counter\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}



LDEFUN  (
    onu_statistic_port_flush,
    onu_statistic_port_flush_cmd,
    "stat port_flush {<port_list>}*1",    
    "Show or config onu Statistic information\n"
    "Clear onu Statistic information\n"
    "Please input port Number\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

LDEFUN  (
    onu_statistic_port_show,
    onu_statistic_port_show_cmd,
    /*"stat port_show <1-24> {<0-31>}*1",  */
	"stat port_show <port_list>", 
    "Show or config onu Statistic information\n"
    "Show onu Statistic information\n"
    "Please input port Number\n",
    ONU_NODE
    /*"Please input the counter number\n"*/
    )
{
	return CMD_SUCCESS;
}



LDEFUN  (
    onu_statistic_pas_flush,
    onu_statistic_pas_flush_cmd,
    "stat pas_flush",    
    "Show or config onu Statistic information\n"
    "Clear all the pon port, MPCP and OAM Statistic information\n",
    ONU_NODE
    )
{
	return CMD_SUCCESS;
}

#if 0
/*added by wutongwu for stat show all*/
DEFUN  (
    onu_statistic_port_all_show,
    onu_statistic_port_all_show_cmd,
    "stat port_show <port_list>",    
    "Show or config onu Statistic information\n"
    "Show onu Statistic information\n"
    "Please input port Number\n"
    )
{
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid, ulOnuFeid;
	USHORT length;
	ULONG ulRet;
	short int fePort = 0;
	int onuType = -1;
	short int counter = 0;
	cliPayload tPayload;
	/*cliPayload *stPayload=NULL;*/
	cliPayload *stPayload=NULL;
	

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/

	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }


	lRet = GetOnuType( ponID, (ulOnuid-1), &onuType );
	if(ROK  != lRet)
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}
	/*
	ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid,ulOnuFeid,vty))
		return CMD_WARNING;
	*/
	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			{
			return CMD_WARNING;
			}
		}
	END_PARSE_PORT_LIST_TO_PORT();

	for(counter= 0;counter < 32; counter++)
	{
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;
	sprintf(pBuff, "stat port_show %d %d",fePort,counter);
	length = strlen(pBuff);
	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, &tPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%stat port_show  !\r\n");	
	taskDelay(5);
	}

	return CMD_SUCCESS;
}
#endif

#if mgt
#endif
/*ONU device manager*/
DEFUN  (
    onu_mgt_temperature,
    onu_mgt_temperature_cmd,
    "mgt temperature {<30-50>}*1",    
    "Show or config onu mgt information\n"
    "Show or config onu mgt temperature information\n"
    "Please input the max value\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "mgt temperature",NULL );
#elif defined __RPC_ONU_COMMAND

#if 1
	return rpc_onu_dev_cli_process( vty, "mgt temperature",NULL );
#else
    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;
    ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
    if( ulRet !=VOS_OK )
        return CMD_WARNING;

    /*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/

    ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
    if(ponID == (VOS_ERROR))
    {
        vty_out( vty, "  %% Parameter error\r\n");
        return CMD_WARNING;
    }

    if(argc == 1)
    {
        ulRet = VOS_StrToUL(argv[0], NULL, 10);
        if(OnuMgt_SetTemperature(ponID, ulOnuid, ulRet) == VOS_OK)
            vty_out(vty, "\nset temperature OK\n");
        else
            vty_out(vty, "\nset temperature ERROR\n");
    }
    else
    {
        char *pbuf = NULL;
        int len = 0;

        if(OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pbuf, &len)== VOS_OK && len)
		            vty_big_out(vty, len, "%s", pbuf);
    }

    return CMD_SUCCESS;
#endif

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}

     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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
	
	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%mgt temperature  !\r\n");	


	return CMD_SUCCESS;
#endif
}

/* B--added by liwei056@2009-6-16 for BroadCast-Cli-Commit-SaveBug */
int Mgt_Cli_save_broadcastCB( struct vty * vty )
{
	int iRet = 0;
	int  doFlag;
   	int  iVal;
    short int ponID, onuID;

    VOS_ASSERT(vty);

    doFlag = VTY_CONFIRM_CB_GETPARAM1(vty);
    iVal   = VTY_CONFIRM_CB_GETPARAM2(vty);
    ponID = (short int)((iVal >> 16) & 0xFFFF);
    onuID = (short int)(iVal & 0xFFFF);

	/* added by wutw 2006/11/10 */
	if ( doFlag == 1)
	{/*save*/
#if 0
		iRet = cl_save_runconfig_to_startupconfig( _MN_WAY_CLI_, cl_vty_freeze, \
	 											cl_vty_unfreeze_and_put, ( void * ) vty, \
	 											vty->fd, vty->user_name, vty->address, 1 , \
	 											vty->v_timeout );
		/*if( lRet == 1 )
			cfgDataSaveSuccess_EventReport(ulSlot *10000 + ulPort*1000 + ulOnuid);
		else
			cfgDataSaveFail_EventReport(ulSlot *10000 + ulPort*1000 + ulOnuid);*/
			
	    if ( iRet < 0 )
	    {
	        vty->frozen = 0;
	    }
	    if ( iRet == -2 )
	    {
	        vty_direct_out( vty, "  %% Flash busy, please try later.\r\n" );
	        return CMD_WARNING;
	    }
	    if ( iRet == -1 )
	    {
	        vty_direct_out( vty, "  %% System does not have enough resource to save config to flash.\r\n" );
	        return CMD_WARNING;
	    }
#endif
	        vty_direct_out( vty, "  %% ONUs config data save finished.\r\n" );
	}
	else if ( doFlag == 2)
	{/*clear*/
#if 0
		{/*software auto-update flag*/
		int defSoftUpdata = 0;
		defSoftUpdata = GetOnuSWUpdateCtrlDefault();
		if ( (defSoftUpdata == 1) || (defSoftUpdata == 2))
			SetOnuSWUpdateCtrl(ponID, onuID, defSoftUpdata);
		}
		
		{/*encrypt-keytime*/
			unsigned int timeLen = 0;
			timeLen = GetOnuEncryptKeyExchagetimeDefault();
			/*GetPonPortEncryptKeyTimeDefault( &timeLen );*/
			/* modified by chenfj 2007-10-18
			问题单:5573.OLT的V1R05B096版本，4槽主控，show run中GT812的密钥更新时间为0
			数据库中的时间参数单位为毫秒，设置时应乘以SENOND*1
			*/
			SetOnuEncryptKeyExchagetime(ponID, onuID, timeLen);
		}
		
		{/**/
			unsigned int encrypt = 0;
			/*GetPonPortEncryptDefault( &encrypt );*/
			encrypt = GetOnuEncryptDefault();
			/*GetOnuEncrypt( (short int) ponID, (short int)(ulOnuid-1), &encrypt );*/
			OnuEncryptionOperation(ponID, onuID, encrypt );	
			
		}
		
		{/*history-statistics*/
			unsigned int status15m = 0;
			unsigned int status24h = 0;
			unsigned int bucketNum15M = 0;
			unsigned int bucketNum24h = 0;
			HisStats15MinMaxRecordGet(&bucketNum15M);
			HisStats24HoursMaxRecordGet( &bucketNum24h);			
			iRet = CliHisStatsONUStatusGet(ponID, onuID,&status15m, &status24h);
			if (iRet == VOS_OK)
			{
				if (status15m == 1)
					HisStatsOnu15MModified(ponID, onuID, bucketNum15M, FALSE);
				taskDelay(10);
				if (status24h == 1)
					HisStatsOnu24HModified(ponID, onuID, bucketNum24h, FALSE);
				taskDelay(10);
			}
		}
		{
			int defSoftUpdata = 0;
			defSoftUpdata = GetOnuSWUpdateCtrlDefault();
			SetOnuSWUpdateCtrl(ponID, onuID, defSoftUpdata);
		}
		
		{/*Clear p2p*/
			int unicastFlag = 2;/*2 - disable, 1 - enable*/
			int brdFlag = 2;/*2 - disable, 1 - enable*/
			SetOnuPeerToPeerForward(ponID, onuID, unicastFlag, brdFlag );		
		}
		/*save */
		iRet = cl_save_runconfig_to_startupconfig( _MN_WAY_CLI_, cl_vty_freeze, \
	 											cl_vty_unfreeze_and_put, ( void * ) vty, \
	 											vty->fd, vty->user_name, vty->address, 1 , \
	 											vty->v_timeout );
		/*if( lRet == 1 )
			flashClearSuccess_EventReport(ulSlot *10000 + ulPort*1000 + ulOnuid);
		else
			flashClearFail_EventReport(ulSlot *10000 + ulPort*1000 + ulOnuid);*/

	    if ( iRet < 0 )
	    {
	        vty->frozen = 0;
	    }
	    if ( iRet == -2 )
	    {
	        vty_direct_out( vty, "  %% Flash busy, please try later.\r\n" );
	        return CMD_WARNING;
	    }
	    if ( iRet == -1 )
	    {
	        vty_direct_out( vty, "  %% System does not have enough resource to save config to flash.\r\n" );
	        return CMD_WARNING;
	    }
#endif
	        vty_direct_out( vty, "  %% ONUs config data clear finished.\r\n" );
	}

    
    return iRet;
}
/* E--added by liwei056@2009-6-16 for BroadCast-Cli-Commit-SaveBug */


DEFUN  (
    onu_mgt_conf,
    onu_mgt_conf_cmd,
    /* modified by chenfj 2007-10-22
     问题单#5552:命令mgt config disable/enable执行失败
     处理:因为mgt config disable/enable和mgt config clear/save的功能是一样的。去掉mgt config disable/enable
    */
    /*"mgt config [enable|disable|save|clear]",    */
    "mgt config [save|clear]",
    "Show or config onu mgt information\n"
    "Show or config onu mgt config information\n"
    /*"enable save or clear the configuration\n"*/
    /*"disable save or clear the configuration\n"*/
    "Save configuration\n"
    "Clear all configuration\n"
    )
{
#ifdef __RPC_ONU_COMMAND

    /*add by wangxiaoyu 2011-10-10
     * 非透传模式下，GW－ONU的本地配置保存要被屏蔽 problem-13592*/
    if(!OnuTransmission_flag && (!VOS_StrCmp(argv[0], "save")))
    {
        vty_out(vty, "forbidden action because the onu has been associated with a config file\r\n");
        return CMD_WARNING;
    }

    return rpc_onu_dev_cli_process( vty, vty->buf ,NULL );
#else
	LONG lRet;
	CHAR pBuff[256];   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length = 0;
	cliPayload *stPayload=NULL;
	ULONG doFlag = 0;

	if( check_onu_cli_is_deny(vty) )	/* modified by xieshl 20100521, 问题单10260 */
		return CMD_SUCCESS;

	ulIfIndex =(ULONG) vty->index;
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}

     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }
	
	length += VOS_Sprintf( pBuff+length, "mgt config ");
	if( VOS_StrCmp( argv[0], "save") ==0 )
	{
		length += VOS_Sprintf( pBuff+length,"save");
		doFlag = 1;/*save*/
	}
	else if( VOS_StrCmp( argv[0], "clear") ==0 )
	{
		length += VOS_Sprintf( pBuff+length, "clear");
		doFlag = 2;/*clear*/
		vty_out( vty, "\r\n\r\nReset current onu to default configure...Done. \r\n");	/* 问题单9302 */
	}
	/* 问题单#5552:
	else if( VOS_StrCmp( argv[0], "enable" ) == 0 )
		length += VOS_Sprintf( pBuff+length, "enable" );
	else if( VOS_StrCmp( argv[0], "disable" ) == 0 )
		length += VOS_Sprintf( pBuff+length, "disable" );
	*/
	
	/*VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	*/
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%mgt config  !\r\n");	

    if (vty->node == CONFIRM_ACTION_NODE)
    {
        VTY_CONFIRM_CB_SETPARAM1(vty, doFlag);
        VTY_CONFIRM_CB_SETPARAM2(vty, (ponID << 16) | ((ulOnuid-1) & 0xFFFF));
        VTY_CONFIRM_CB_REGISTER(vty, Mgt_Cli_save_broadcastCB);
        
    	return CMD_SUCCESS;
    }
    vty_out( vty, "  %% Config data %s finished.\r\n", argv[0] );	/* 问题单9302 */

	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_mgt_reset,
    onu_mgt_reset_cmd,
    "mgt reset {def}*1",    
    "Show or config onu mgt information\n"
    "Show or config onu mgt reset information\n"
    "Reset onu and load default configuration\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "mgt reset",NULL );
#elif defined __RPC_ONU_COMMAND

#if 0
	return rpc_onu_dev_cli_process( vty, "mgt reset",NULL );
#else
    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        /*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {

            if(OnuMgt_SetMgtReset(ponID, ulOnuid-1, 1) != VOS_OK)
            {
#if 0                
                vty_out(vty, "\r\nmgt reset ERROR\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);    
#endif
            }
            else
                vty_out(vty, "\r\nmgt reset OK!\r\n");

        }
        else
        {
            char *pbuf = NULL;
            USHORT len;

            if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pbuf, &len) && len)
		            vty_big_out(vty, len, "%s", pbuf);
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }

        /*指定load default时，去关联ONU的配置文件，清除配置文件内容，但关联不变 */
        if(VOS_StrStr(vty->buf, "def") && vty->node != ONU_VIEW_NODE)
        {
            ONUConfigData_t *pdef = OnuConfigProfile_init();
            ONUConfigData_t *pdev = getOnuConfFromHashBucket(getOnuConfNamePtrByPonId(ponID, ulOnuid-1));
    		short int to_slot = device_standby_master_slotno_get();
            if(pdef && pdev)
            {
                onuconfCopy(pdev, pdef);
                if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
                {
                    OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, ulSlot, 0, 0, getOnuConfNamePtrByPonId(ponID, ulOnuid-1), NULL, NULL);                    
                    if(to_slot)
                        OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, to_slot, 0, 0, getOnuConfNamePtrByPonId(ponID, ulOnuid-1), NULL, NULL);                    
                }
            }

            if(pdef)
                onuconf_free(pdef, ONU_CONF_MEM_DATA_ID);
        }
    }
    else
    {
        
    }

    return CMD_SUCCESS;

#endif

#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%mgt reset  !\r\n");	


	return CMD_SUCCESS;
#endif
}



DEFUN  (
    onu_mgt_laser,
    onu_mgt_laser_cmd,
    "mgt laser {[0|1]}*1",    
    "Show or config onu mgt information\n"
    "Show or config onu mgt laser information\n"
    "Laser is turned on according to received grants\n"
    "Laser is turned on permanently\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "mgt laser",NULL );
#elif defined __RPC_ONU_COMMAND
	return rpc_onu_dev_cli_process( vty, "mgt laser",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam(  ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%mgt laser  !\r\n");	


	return CMD_SUCCESS;
#endif
}

DEFUN  (
    onu_mgt_self_check_result,
    onu_mgt_self_check_result_cmd,
    "mgt self_check_result [0|1|2|3|4|5|6|7|8|9|10|11]",    
    "Show or config onu mgt information\n"
    "Show or config onu mgt self check result information\n"
    "0:Means\n"
    "1:Means\n"
    "2:Means\n"
    "3:Means\n"
    "4:Means\n"
    "5:Means\n"
    "6:Means\n" 
    "7:Means\n"
    "8:Means\n"
    "9:Means\n"
    "10:Means\n"
    "11:Means\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "mgt self_check_result",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam( ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%mgt self_check_result  !\r\n");	


	return CMD_SUCCESS;
#endif
}



/*added by wutw at 19 October*/
DEFUN  (
    onu_mgt_self_check_all_result,
    onu_mgt_self_check_result_all_cmd,
    "mgt self_check_result",    
    "Show or config onu mgt information\n"
    "Show or config onu mgt self check result information\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "mgt self_check_result",NULL );
#elif defined __RPC_ONU_COMMAND
	return rpc_onu_dev_cli_process( vty, "mgt self_check_result",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam( ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%mgt self_check_result  !\r\n");	


	return CMD_SUCCESS;
#endif
}

/*added by wangxy 2007-11-06*/
DEFUN  (
    onu_mgt_read_reg,
    onu_mgt_read_reg_cmd,
    "mgt read_reg",    
    "Show or config onu mgt information\n"
    "Read all registers for the switch chip\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "mgt read_reg",NULL );
#elif defined __RPC_ONU_COMMAND
    return rpc_onu_dev_cli_process( vty, "mgt read_reg",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
	
     	/*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    	lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    	if ( CLI_EPON_ONUUP != lRet)
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

	lRet=lCli_SendbyOam( ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %%mgt read register fail  !\r\n");	


	return CMD_SUCCESS;
#endif
}

/*added by wutw at 19 October*/
DEFUN  (
    onu_uptime_show,
    onu_uptime_show_cmd,
    "mgt uptime",    
    "Show or config onu mgt information\n"
    "show onu uptime\n"
    )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "mgt uptime",NULL );
#elif defined __RPC_ONU_COMMAND
	return rpc_onu_dev_cli_process( vty, "mgt uptime",NULL );
#else
	LONG lRet;
	CHAR pBuff[256]={0};   
	int ponID=0;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	cliPayload *stPayload=NULL;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return CMD_WARNING;

	/*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/
	
	ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	if(ponID == (VOS_ERROR))
	{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
	}
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( ponID, ulOnuid-1);
    if ( CLI_EPON_ONUUP != lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",ponID, ulOnuid-1) ;
       #endif
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid) ;
		return CMD_WARNING;
       }

	/*modified by wutw at 20 October*/
	/*VOS_Sprintf(pBuff,"mgt uptime");
	length = strlen(pBuff);*/
	
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;
	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( ponID, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		vty_out(vty, "  %% mgt uptime !\r\n");	


	return CMD_SUCCESS;
#endif
}


/*主题: GT810/816增加的命令行 */ 

DEFUN( onu_IgmpsnoopAuthEnable_Func,
        onu_Igmpsnoop_Authenable_Cmd,
        "igmpsnooping auth enable {[1|0]}*1",
        "IGMP snooping authentication config\n"
        "IGMP Snooping auth function\n"
        "Enable or disable IGMP Snooping auth\n"
        "Enable IGMP Snooping auth\n"
        "Disable IGMP Snooping auth\n")

{
#if 0 /* __CTC_TEST_NMS_COMMAND*/
	int onu_mask[] = {V2R1_ONU_GT810, V2R1_ONU_GT816, V2R1_ONU_GT861, V2R1_ONU_GT811, V2R1_ONU_GT812,
					V2R1_ONU_GT811_A, V2R1_ONU_GT812_A, 0 };
	return onu_dev_cli_process( vty, "igmpsnooping auth enable", onu_mask );
#elif defined __RPC_ONU_COMMAND


    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet, suffix;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        /*vty_out( vty, " ulSlot =%d, ulPort = %d, ulOnuid = %d \r\n", ulSlot, ulPort, ulOnuid-1 );*/

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {

            ULONG en = VOS_StrToUL(argv[0], NULL, 10);

            ONU_CONF_WRITEABLE_CHECK

            if(OnuMgt_SetIgmpAuth(ponID, ulOnuid-1, en) != VOS_OK)
            {
#if 0                
                vty_out(vty, "igmpsnooping auth %s FAIL!\r\n", en?"enable":"disable");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {

                    int en = 0;
                    /*if(getOnuConfSimpleVar(ponID, ulOnuid-1, sv_enum_igmp_auth_enable, &en) == VOS_OK)*/
                    if(VOS_OK == OnuMgt_GetMulticastSwitch(ponID, ulOnuid-1, &en))
                    {
                        vty_out(vty,"igmpsnooping auth is %s\r\n", en?"enable":"disable");
                    }
                    else
                    {
                        vty_out(vty, ONU_CMD_ERROR_STR);                        
                    }
            }
            else
            {
/*
                int onu_mask[] = {V2R1_ONU_GT810, V2R1_ONU_GT816, V2R1_ONU_GT861, V2R1_ONU_GT811, V2R1_ONU_GT812,
                                V2R1_ONU_GT811_A, V2R1_ONU_GT812_A, 0 };
                return rpc_onu_dev_cli_process( vty, "igmpsnooping auth enable", onu_mask );
                */
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
                    vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG en = 0;

        if(argc == 1)
        {
            en = VOS_StrToUL(argv[0], NULL, 10);
            if(setOnuConfSimpleVarByPtr(suffix, vty->onuconfptr, sv_enum_igmp_auth_enable, en) != VOS_OK)
                vty_out(vty, "config igmp auth fail!\r\n");
        }
        else
        {
            if(getOnuConfSimpleVarByPtr(suffix, vty->onuconfptr, sv_enum_igmp_auth_enable, &en) != VOS_OK)
                vty_out(vty, "get igmp auth fail!\r\n");
            else
                vty_out(vty, "igmp auth %s\r\n", en?"enable":"disable");
        }
    }

    return CMD_SUCCESS;
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

	if( check_onu_cli_is_deny(vty) )	/* modified by xieshl 20100521, 问题单10260 */
		return CMD_SUCCESS;

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

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;

	if( OnuType == V2R1_ONU_GT811 || OnuType == V2R1_ONU_GT812 )
	{
		char *p = VOS_StrStr( pBuff, " enable" );
		if( p != NULL )
			*p = '_';			
	}
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
 
/*使能或关闭UNI接口的MAC地址学习。*/
/* modified by chenfj 2007-8-30
	按照大亚方式统一一下命令。 张新辉需按照新的定义修改GT816的软件
	*/
DEFUN(onu_atu_learn,
         onu_atu_learn_cmd,
         "atu learning <port_list> {[1|0]}*1",
         "Mac table config\n"
         "Uni mac learn configuration\n" 
         "input the onu fe port number\n"
         "1-learning enable\n"
         "0-learning disable\n")

{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "atu learning" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
        return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {
            ULONG portlist = 0;

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                portlist |= 1<<(port-1);
            }
            END_PARSE_PORT_LIST_TO_PORT();

            if(VOS_OK == OnuMgt_SetPortAtuLearn(ponID, ulOnuid-1, portlist, VOS_StrToUL(argv[1], NULL, 10)) )
            {
            }
            else
            {
#if 0                
                vty_out(vty, "port atu learning enable set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                ULONG enable = 0;
                    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                    {
                        if(getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_atu_learn_enable, &enable))
                            vty_out(vty, "port %d atu learning enable set fail!\r\n", port);
                        else
                            vty_out(vty, "port %d atu learning %s\r\n", port, enable?"enable":"disable");
                    }END_PARSE_PORT_LIST_TO_PORT();
            }

            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG enable = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
		
        if(argc == 2)
        {
            enable= VOS_StrToUL(argv[1], NULL, 10);

            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_atu_learn_enable, enable))
                vty_out(vty, "port %d atu learning enable set fail!\r\n", port);

            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_atu_learn_enable, &enable))
                vty_out(vty, "port %d atu learning enable set fail!\r\n", port);
            else
                vty_out(vty, "port %d atu learning %s\r\n", port, enable?"enable":"disable");

            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;
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
	ULONG ulOnuFeid;

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

	/*ONU_TYPE_AND_COMMAND_CHECK_GT816_810 */

	if( argc == 2 )
		{
		/*
		ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
		 if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			return CMD_WARNING;
		 */
		 BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		 	{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
				{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
				}
			}
		END_PARSE_PORT_LIST_TO_PORT();
		}

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %% atu learning failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

/*主题: GT810/816增加的命令行。end */ 

/* added by chenfj 2007-9-17
	在GT811/GT812下增加如下两个命令*/
DEFUN(onu_atu_flood,
         onu_atu_flood_cmd,
         "atu flood <port_list> {[1|0]}*1",
         "Mac table config\n"
         "Mac table config\n"
         "input the onu fe port number\n"
         "1-enable flood mode\n"
         "0-disable flood mode\n")

{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "atu flood" );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {
            ULONG portlist = 0;

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                portlist |= 1<<(port-1);
            }
            END_PARSE_PORT_LIST_TO_PORT();

            if(VOS_OK == OnuMgt_SetPortAtuFlood(ponID, ulOnuid-1, portlist, VOS_StrToUL(argv[1], NULL, 10)) )
            {
            }
            else
            {
#if 0                
                vty_out(vty, "port atu flood mode set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {

                    ULONG en = 0;
                    BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
                    {
                        if(getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_atu_flood_enable, &en) == VOS_OK)
                            vty_out(vty, "port %d atu flood %s\r\n", port, en?"enable":"disable");
                    }
                    END_PARSE_PORT_LIST_TO_PORT()

            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG enable = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
		
        if(argc == 2)
        {
            enable = VOS_StrToUL(argv[1], NULL, 10);

            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_atu_flood_enable, enable))
                vty_out(vty, "port %d atu flood mode set fail!\r\n", port);

            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            ULONG en = 0;
            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                if(getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_atu_flood_enable, &en) == VOS_OK)
                    vty_out(vty, "port %d atu flood %s\r\n", port, en?"enable":"disable");
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;

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
	ULONG ulOnuFeid;

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

	/*ONU_TYPE_AND_COMMAND_CHECK_GT816_810 */

	if( argc == 2 )
		{
		/*
		ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
		 if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			return CMD_WARNING;
		 */
		 BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulOnuFeid )
		 	{
			if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
				{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
				}
			}
		END_PARSE_PORT_LIST_TO_PORT();
		}

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %% atu flood failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

DEFUN(onu_atu_limit,
         onu_atu_limit_cmd,
         "atu limit {[0|<1-64>]}*1",
         "Mac table config\n"
         "pon address learning limit\n"
         "0-no limit\n"
         "the max learning MAC address\n")

{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_dev_cli_process( vty, "atu limit", NULL );
#elif defined __RPC_ONU_COMMAND

    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc == 1)
        {
    
            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK == OnuMgt_SetAtuLimit(ponID, ulOnuid-1, VOS_StrToUL(argv[0], NULL, 10)) )
            {
            }
            else
            {
#if 0                
                vty_out(vty, "atu limit set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                ULONG limit;
                if(getOnuConfSimpleVar(ponID, ulOnuid-1, sv_enum_atu_limit, &limit) == VOS_OK)
                    vty_out(vty, "onu atu limit is %d\r\n\r\n", limit);
                else
                    vty_out(vty, "onu atu limit get fail!\r\n\r\n");
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG limit= 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(argc == 1)
        {
            limit = VOS_StrToUL(argv[0], NULL, 10);

            if(setOnuConfSimpleVarByPtr(suffix, pd,sv_enum_atu_limit, limit))
                vty_out(vty, "atu limit set fail!\r\n");
        }
        else
        {
            if(getOnuConfSimpleVarByPtr(suffix, pd , sv_enum_atu_limit, &limit))
                vty_out(vty, "atu limit get fail!\r\n" );
            else
                vty_out(vty, "atu limit %d\r\n", limit);
        }
    }

    return CMD_SUCCESS;

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
	/*ULONG ulOnuFeid;*/

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

	/*ONU_TYPE_AND_COMMAND_CHECK_GT816_810 

	if( argc == 2 )
		{
		ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
		 if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			return CMD_WARNING;
		}
	*/
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %% atu limit failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}

/* added by chenfj 2007-9-17
	DHCP relay功能，dscp_relay_agent : Enable/Disable DHCP Relay Agent ,
	涉及到的ONU：811/812/821/831/810/816
*/
DEFUN(onu_dscp_relay_agent,
         onu_dscp_relay_agent_cmd,
         "dhcp_relay_agent {[0|1]}*1",
         "Enable/Disable DHCP Relay Agent\n"
         "0-disable\n"
         "1-enable\n")

{
#ifdef __CTC_TEST_NMS_COMMAND
	int onu_mask[] = {V2R1_ONU_GT811, V2R1_ONU_GT812, 0 };
	return onu_dev_cli_process( vty, "dhcp_relay_agent", onu_mask );
#elif defined __RPC_ONU_COMMAND
	int onu_mask[] = {V2R1_ONU_GT811, V2R1_ONU_GT812, 0 };
	return rpc_onu_dev_cli_process( vty, "dhcp_relay_agent", onu_mask );
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
	/*ULONG ulOnuFeid;*/

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

	/*ONU_TYPE_AND_COMMAND_CHECK_GT816_810 

	if( argc == 2 )
		{
		ulOnuFeid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
		 if(VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulOnuFeid,vty))
			return CMD_WARNING;
		}
	*/
	/* modified by chenfj 2008-5-21
		问题单:(6691)dhcp-relay-agent命令无法识别
		在大亚811和812 ONU节点下,此命令无法识别;
		在其它ONU(如:GT812A/Gt811A, gt810/816, gt821/831) 可以正确识别
		张井军测试发现, 大亚811和812 ONU 实际支持的命令格式为:
		dhcp relay-agent {[0|1]}*1
		在此加一个判断,若是大亚811和812 ONU, 则变换输入的命令字符串
	*/
	if((OnuType == V2R1_ONU_GT811) || (OnuType == V2R1_ONU_GT812))
		{
		unsigned char *Ptr = NULL;
		unsigned int len1 = 0;
		length = VOS_StrLen("dhcp relay_agent" );
		VOS_MemCpy(&pBuff[0],"dhcp relay_agent", length);
		Ptr = VOS_StrStr(vty->buf, " ");
		if(Ptr != NULL )
			{
			len1 = VOS_StrLen(Ptr);			
			VOS_MemCpy(&pBuff[length], Ptr, len1);
			length += len1;
			}
		}
	else {
		VOS_MemCpy( pBuff, vty->buf, vty->length);
		length = vty->length;	
		}
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;


	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %% dhcp_relay_agent failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}
/*
发件人: KD刘冬 
发送时间: 2007年10月22日 14:58
收件人: KD陈福军
主题: 请在GT810/816/811a/812a节点下增加Download命令,支持FTP方式下载程序
*/
DEFUN(ftpc_download_ftp_phenixos_master_func,
 ftpc_download_ftp_phenixos_master_cmd,
 "download ftp [os] <A.B.C.D> <user> <pass> <filename>",
 "Download OS\n"
 "Download file using ftp protocol\n"
 "Download new GROS image\n"
 "Please input ftp server's IP address\n"
 "Please input user name\n" 
 "Please input the password\n" 
 "Please input the file name\n"
 )
{
#ifdef __CTC_TEST_NMS_COMMAND
	int onu_mask[] = {V2R1_ONU_GT811, V2R1_ONU_GT812, 0 };
	return onu_dev_cli_process( vty, "ftp download to onu", onu_mask );
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

	if(( OnuType != V2R1_ONU_GT810 ) && ( OnuType != V2R1_ONU_GT816 ) && ( OnuType != V2R1_ONU_GT811_A) &&( OnuType != V2R1_ONU_GT811_B) && ( OnuType != V2R1_ONU_GT812_A))
		{
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}

	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%ftp download to onu failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
}
/*
发件人: KD刘冬 
发送时间: 2007年10月29日 14:48
收件人: KD陈福军
抄送: KD盖鹏飞
主题: 请在GT811/GT812节点下增加命令
*/
DEFUN(onu_cable_test_gt811,
		onu_cable_test_gt811_cmd,
		"cable test port <port_list>", 
		"Ethernet cable test\n" 
		"Test ethernet cable characters\n"
		"port number\n"
		DescStringPortList
 		) 
{
#ifdef __CTC_TEST_NMS_COMMAND
	return onu_fe_cli_process( vty, argv[0], "cable test port" );
#elif defined __RPC_ONU_COMMAND
	return rpc_onu_fe_cli_process(vty, argv[0], "cable test port");
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
	ULONG ulFePort = 0;

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
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot, ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}
	
	if(( OnuType == V2R1_ONU_GT811 ) || ( OnuType == V2R1_ONU_GT812 ))
		{
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}

	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ulFePort )
	{
		if (VOS_OK != CliOnuTypeVty(ulSlot , ulPort, ulOnuid, ulFePort,vty))
		{
			RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
		}
	}
	END_PARSE_PORT_LIST_TO_PORT();
	
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%cable test port failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif
    return CMD_SUCCESS;
}

DEFUN(onu_mgt_config_ip_addr_gt811,
		onu_mgt_config_ip_addr_gt811_cmd,
		"set ip <A.B.C.D/M>",
		"Set system operation\n" 
		"Set ip address in eeprom\n"
		"IP address information(eg 192.168.30.1/24)\n"
 		)

{
#ifdef __CTC_TEST_NMS_COMMAND
	int onu_mask[] = {V2R1_ONU_GT811, V2R1_ONU_GT812, 0 };
	return onu_dev_cli_process( vty, "set ip", onu_mask );
#elif defined __RPC_ONU_COMMAND
	int onu_mask[] = {V2R1_ONU_GT811, V2R1_ONU_GT812, 0 };
	    return rpc_onu_dev_cli_process( vty, "set ip", onu_mask );
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
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot, ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}
	
	if(( OnuType == V2R1_ONU_GT811 ) || ( OnuType == V2R1_ONU_GT812 ))
		{
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%set ip  failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif	
}


DEFUN(onu_mgt_running_config_gt811,
		onu_mgt_running_config_gt811_cmd,
		"show running-config",
		"Show system operation\n" 
		"show system running config\n"
 		)

{
#ifdef __CTC_TEST_NMS_COMMAND
	int onu_mask[] = {V2R1_ONU_GT811, V2R1_ONU_GT812, 0 };
	return onu_dev_cli_process( vty, "show running-config", onu_mask );
#elif defined __RPC_ONU_COMMAND
    short int PonPortIdx =0;
    short int OnuIdx = 0;
    int ret = CMD_SUCCESS;

    ULONG ulIfIndex = 0;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet;

    ulIfIndex = (ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &ulRet))
    {
        
        ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
        if (ulRet != VOS_OK)
            return CMD_WARNING;
        /*modified by luh 2012-9-12 备用主控不支持onu的show-run*/
        if(isCtcOnu(ulSlot, ulPort, ulOnuid)&& (!SYS_LOCAL_MODULE_ISMASTERSTANDBY))
        {
            PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
            OnuIdx = ulOnuid-1;
            CHECK_ONU_RANGE

             /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/
            if ( CLI_EPON_ONUUP != GetOnuOperStatus( PonPortIdx, OnuIdx ))
            {
                vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, ulOnuid ) ;
                return (CMD_WARNING );
            }
            
#ifdef ONUID_MAP
            ONU_CONF_SEM_TAKE
            {
#else
            ONU_MGMT_SEM_TAKE
#endif
            vty_out(vty, "\r\n");

            {
                int result = 0;
                char *file = NULL/*(char*)cl_config_mem_file_init()*/;
                ONUConfigData_t *p = OnuConfigProfile_init();
                char szname[ONU_CONFIG_NAME_LEN+1] = "";
                VOS_Sprintf(szname, "*auto%08p", p);
    			if(p)
    			{
				    VOS_StrCpy(p->confname, szname);   
                    result = ctc_show_running_config_all(PonPortIdx, ulOnuid,p);
                    if(result != VOS_OK)
                    {
                        vty_out(vty, ONU_CMD_ERROR_STR);
                    }
                    OnuProfile_Action_ByCode(OnuProfile_Add, 0, 0, 0, szname, NULL, p);                    
    			}

                /*if (file)*/
                if(result == VOS_OK)
                {
                    if (!generateOnuConfClMemFile(p->confname, -1, vty, file,0,0,0))
                    {
                        vty_out(vty, "generate config file fail!\r\n");
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
#if 0
                else
                {
                    vty_out(vty, "alloc file memory fail!\r\n");
                    ret = CMD_WARNING;
                }
#endif
                if(p)
                    OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, p->confname, NULL, NULL);                                    
                    
                vty_out(vty, "\r\n");
            }
            }
#ifdef ONUID_MAP   
    ONU_CONF_SEM_GIVE
#else
    ONU_MGMT_SEM_GIVE
#endif
        }
        else
        {
        	int onu_mask[] = {V2R1_ONU_GT811, V2R1_ONU_GT812, 0 };
        	    return rpc_onu_dev_cli_process( vty, "show running-config", onu_mask );
        }
    }
#if 0
	int onu_mask[] = {V2R1_ONU_GT811, V2R1_ONU_GT812, 0 };
	    return rpc_onu_dev_cli_process( vty, "show running-config", onu_mask );
#endif
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
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot, ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}
	
	if(( OnuType == V2R1_ONU_GT811 ) || ( OnuType == V2R1_ONU_GT812 ))
		{
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show running-config failed!\r\n");	
		return CMD_WARNING;
		}
#endif	
	return CMD_SUCCESS;
}	

DEFUN(onu_mgt_startup_config_gt811,
		onu_mgt_startup_config_gt811_cmd,
		"show startup-config",
		"Show system operation\n" 
		"show system startup config\n"
 		)

{
#ifdef __CTC_TEST_NMS_COMMAND
	int onu_mask[] = {V2R1_ONU_GT811, V2R1_ONU_GT812, 0 };
	return onu_dev_cli_process( vty, "show startup-config", onu_mask);
#elif defined __RPC_ONU_COMMAND
	int onu_mask[] = {V2R1_ONU_GT811, V2R1_ONU_GT812, 0 };
	    return rpc_onu_dev_cli_process( vty, "show startup-config", onu_mask);
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
		vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot, ulPort, ulOnuid ) ;
		return (CMD_WARNING );
	   }

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
		}
	
	if(( OnuType == V2R1_ONU_GT811 ) || ( OnuType == V2R1_ONU_GT812 ))
		{
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	
	VOS_MemCpy( pBuff, vty->buf, vty->length);
	length = vty->length;	
	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %%show startup-config failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
#endif	
}	

#define BEGIN_QOS_CLIS

DEFUN(
        show_onu_conf_qos_policy_map,
        show_onu_conf_qos_policy_map_cmd,
        "show policy-map {port <port_list>}*1",
        SHOW_STR
        "show QoS map information\n"
        "set port<1-24>\n")
{

    ULONG brdIdx;


    ULONG pon;
    ULONG onu;
    ULONG port, ethidx, idx;
    int ponid;

    ULONG suffix;

    if(IsProfileNodeVty(vty->index, &suffix))
    {
        if(argc == 0)
        {
            for(ethidx=1; ethidx <= ONU_MAX_PORT; ethidx++)
            {
                if(getOnuConfPortSimpleVarByPtr(suffix, vty->onuconfptr, ethidx, sv_enum_port_qoset_idx, &idx) == VOS_OK)
                {
                    if(idx)
                        show_onu_conf_qosset_vty_by_ptr(vty, suffix, vty->onuconfptr, idx);
                }
            }
        }
        else
        {

			VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
		
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(getOnuConfPortSimpleVarByPtr(suffix, vty->onuconfptr, port, sv_enum_port_qoset_idx, &idx) == VOS_OK)
                {
                    if(idx)
                        show_onu_conf_qosset_vty_by_ptr(vty, suffix, vty->onuconfptr, idx);
                }
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
    }
    else
    {

        if(PON_GetSlotPortOnu( (ULONG)vty->index, &brdIdx, &pon, &onu ) != VOS_OK)
        {
            return CMD_WARNING;
        }


        ponid = GetPonPortIdxBySlot((short int)brdIdx, (short int)pon);
        if(ponid == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if (argc == 0)
        {
            for(ethidx = 1; ethidx <= ONU_MAX_PORT; ethidx++)
            {
                if(getOnuConfPortSimpleVar(ponid, onu-1, ethidx, sv_enum_port_qoset_idx, &idx) == VOS_OK)
                {
                    if(idx)
                        show_onu_conf_qosset_vty(vty, ponid, onu-1, idx);
                }
            }
        }
        else
        {

			VTY_ONU_FE_PORT_IN_RANGE(brdIdx, pon, onu, argv[0], port)
		
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(getOnuConfPortSimpleVar(ponid, onu-1, port, sv_enum_port_qoset_idx, &idx) == VOS_OK)
                {
                    if(idx)
                        show_onu_conf_qosset_vty(vty, ponid, onu-1, idx);
                }
            }
            END_PARSE_PORT_LIST_TO_PORT()
        }
    }
    return CMD_SUCCESS;
}

/*set service-policy*/
DEFUN(onu_conf_qos_service_policy,
        onu_conf_qos_service_policy_cmd,
        "service-policy port [<port_list>] policy <policy_index>",
        "set QoS policy\n"
        "set QoS policy port\n"
        "set  port\n"
        "policy key word\n"
        "<policy-index>\n" )
{
    ULONG devIdx = 0;
    ULONG brdIdx = 0;
    ULONG ethIdx = 0;
    ULONG idx = 0;

    ULONG   ulSlot = 0, ulPort = 0, ulOnuid = 0;
    ULONG gMaxPort = 0;
    ULONG pon = 0;
    ULONG onu = 0;

    int ponid = 0;

    ULONG suffix = 0;

    idx = VOS_AtoI(argv[1]);

    /*ulIfIndex = ( ULONG ) ( vty->index ) ;    */

    if(IsProfileNodeVty(vty->index, &suffix))
    {

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], ethIdx)
	
        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], ethIdx)
        if(setOnuConfPortSimpleVarByPtr(suffix, vty->onuconfptr, ethIdx, sv_enum_port_qoset_idx, idx) != VOS_OK)
            vty_out(vty, "set port%d  QoS policy-index fail!!\r\n", ethIdx);

        END_PARSE_PORT_LIST_TO_PORT()

    }
    else
    {
        if (parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;

        ulSlot = brdIdx;
        ulPort = pon;
        ulOnuid = onu;

        ponid = GetPonPortIdxBySlot(brdIdx, pon);
        if(ponid == -1)
        {
            vty_out(vty, "wrong onu parameters!\r\n");
            return CMD_WARNING;
        }

		/*VTY_ONU_FE_PORT_IN_RANGE(brdIdx, pon, onu, argv[0], ethIdx)*/
        if(getDeviceCapEthPortNum(MAKEDEVID(brdIdx, pon, onu), &gMaxPort) != VOS_OK)
        {
            vty_out(vty, "unknown onu port num!\r\n");
            return CMD_WARNING;
        }

        ONU_CONF_WRITEABLE_CHECK

        if (isCtcOnu(brdIdx, pon, onu))
        {
            int error_flag = 0;
            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
            {
                if(ethIdx > gMaxPort)
                    continue;
                    
                if (OnuMgt_SetPortQosRule(ponid, onu-1, ethIdx, idx) != VOS_OK)
                {
#if 0                    
                    vty_out(vty, "set port%d  QoS policy-index fail!!\r\n", ethIdx);
#else
                    error_flag = 1;
#endif
                }
            }END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
        }
        else
        {
            char *pbuf = NULL;
            USHORT len;

            if (OnuMgt_CliCall(ponid, onu - 1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            vty_big_out(vty, len+4, "\r\n%s\r\n", pbuf);
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }

    return CMD_SUCCESS;
}


/*undo service-policy*/
DEFUN(undo_onu_conf_qos_service_policy,
        undo_onu_conf_qos_service_policy_cmd,
        "undo service-policy port [<port_list>]",
        "undo operation\n"
        "undo QoS policy\n"
        "set  port\n"
        "port list \n"
        )
{
    ULONG devIdx;
    ULONG brdIdx;
    ULONG ethIdx;

    ULONG   ulSlot, ulPort, ulOnuid;

    ULONG pon;
    ULONG onu;

    int ponid = 0;
    ULONG suffix;

    /*ulIfIndex = ( ULONG ) ( vty->index ) ;    */

    if(IsProfileNodeVty(vty->index, &suffix))
    {

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], ethIdx)
	
        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], ethIdx)
        if(setOnuConfPortSimpleVarByPtr(suffix, vty->onuconfptr, ethIdx, sv_enum_port_qoset_idx, 0) != VOS_OK)
            vty_out(vty, "undo QoS policy-index is fail!\r\n", ethIdx);
            END_PARSE_PORT_LIST_TO_PORT()
    }
    else
    {
        if (parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;

        ulSlot = brdIdx;
        ulPort = pon;
        ulOnuid = onu;

        ponid = GetPonPortIdxBySlot(brdIdx, pon);
        if(ponid == -1)
        {
            vty_out(vty, "wrong onu parameters!\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(brdIdx, pon, onu, argv[0], ethIdx)

        ONU_CONF_WRITEABLE_CHECK


        if(isCtcOnu(brdIdx, pon, onu))
        {
            int error_flag = 0;

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
            {
                if(OnuMgt_ClrPortQosRule(ponid, onu-1, ethIdx, 0) != VOS_OK)
                {
#if 0                    
                    vty_out(vty, "undo QoS policy-index is fail!\r\n", ethIdx);
#else
                    error_flag = 1;
#endif
                }
            }END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
        }
        else
        {
            char *pbuf = NULL;
            USHORT len;

            if(OnuMgt_CliCall(ponid, onu-1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            vty_big_out(vty, len+4, "\r\n%s\r\n", pbuf);
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }

    return CMD_SUCCESS;
}


/*set policy-map*/
DEFUN(onu_conf_qos_policy_map,
      onu_conf_qos_policy_map_cmd,
      "policy-map <policy_index> <class_map_index> queue-map <0-7> priority-mark <0-7>",
      "set policy map\n"
      "set policy index\n"
      "set class map index\n"
      "<0-7>\n"
      "priority-mark\n"
      "<0-7>\n" )
{
    ULONG devIdx;
    ULONG brdIdx;
    /*ULONG   ethIdx;*/
    ULONG qosset;
    ULONG qosrule;
    ULONG QueueMap;
    ULONG PriorityMark;

    ULONG   ulSlot, ulPort, ulOnuid;

    ULONG pon;
    ULONG onu;
    int ponid;

    ULONG suffix;

    qosset = VOS_AtoI(argv[0]);
    qosrule = VOS_AtoI(argv[1]);
    QueueMap = VOS_AtoI(argv[2]);
    PriorityMark = VOS_AtoI(argv[3]);

    /*ulIfIndex = ( ULONG ) ( vty->index ) ;    */

    if(IsProfileNodeVty(vty->index, &suffix))
    {
        if(setOnuConfQosRuleByPtr(suffix, vty->onuconfptr, qosset, qosrule, QueueMap, PriorityMark) != VOS_OK)
            vty_out(vty, "\r\nset qos rule ERROR\r\n");
    }
    else
    {
        if (parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;

        ulSlot = brdIdx;
        ulPort = pon;
        ulOnuid = onu;

        ponid = GetPonPortIdxBySlot(brdIdx, pon);
        if(ponid == -1)
        {
            return CMD_WARNING;
        }

        ONU_CONF_WRITEABLE_CHECK

        if(isCtcOnu(brdIdx, pon, onu))
        {

            if(OnuMgt_SetQosRule(ponid, onu-1, qosset, qosrule, QueueMap, PriorityMark) == VOS_OK)
            {
            }
            else
            {
#if 0                
                vty_out(vty, "\r\nset qos rule ERROR\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            char *pbuf = NULL;
            USHORT len;

            if(OnuMgt_CliCall(ponid, onu-1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            vty_big_out(vty, len+4, "\r\n%s\r\n", pbuf);
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }

    return CMD_SUCCESS;
}


/*add by shixh@2007/08/17*/
/*undo policy-map*/
DEFUN(undo_onu_conf_qos_policy_map,
      undo_onu_conf_qos_policy_map_cmd,
         "undo policy-map <policy_index> <class_map_index> ",
      "undo operation\n"
      "undo QoS map\n"
      "policy-index\n"
      "class-map-index\n")
{
    ULONG devIdx;
    ULONG brdIdx;
    /*ULONG   ethIdx;*/
    ULONG qosset;
    ULONG qosrule;



    ULONG   ulSlot, ulPort, ulOnuid;

    ULONG pon;
    ULONG onu;
    int ponid;

    ULONG suffix;

    qosset = VOS_AtoI(argv[0]);
    qosrule = VOS_AtoI(argv[1]);


    if(IsProfileNodeVty(vty->index, &suffix))
    {
        if(clrOnuConfQosRuleByPtr(suffix, vty->onuconfptr, qosset, qosrule) != VOS_OK)
            vty_out(vty, "\r\nclear qos rule ERROR\r\n");
    }
    else
    {
        if (parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;

        ulSlot = brdIdx;
        ulPort = pon;
        ulOnuid = onu;

        ponid = GetPonPortIdxBySlot(brdIdx, pon);
        if(ponid == -1)
        {
            return CMD_WARNING;
        }

        ONU_CONF_WRITEABLE_CHECK

        if(isCtcOnu(brdIdx, pon, onu))
        {

            if(OnuMgt_ClrQosRule(ponid, onu-1, qosset, qosrule) == VOS_OK)
            {
            }
            else
            {
#if 0                
                vty_out(vty, "\r\nclear qos rule ERROR\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            char *pbuf = NULL;
            USHORT len;

            if(OnuMgt_CliCall(ponid, onu-1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            vty_big_out(vty, len+4, "\r\n%s\r\n", pbuf);
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }

    return CMD_SUCCESS;
}

DEFUN(onu_conf_qos_class_match,
        onu_conf_qos_class_match_cmd,
         "class-match <policy_index> <class_map_index> <match_index> select [d-mac|s-mac|cos|vlan|eth-type|d-ip|s-ip|ip-type|ip4-tos-dscp|ip6-precedence|s-l4-port|d-l4-port] <match_value> {operator [never|equal|not-equal|less-equal|greater-equal|exist|not-exist|always]}*1 ",
       "set class match\n"
      "policy-index\n"
      "class-map-index\n"
      "match-index\n"
      "select field\n"
      "d-mac addr\n"
      "s-mac addr\n"
      "cos\n"
      "vlan id\n"
      "eth-type\n"
      "d-ip addr\n"
      "s-ip addr\n"
      "ip-type\n"
      "ip4-tos-dscp\n"
      "ip6-precedence\n"
      "s-l4-port"
      "d-l4-port\n"
      "match-value\n"
      "operator\n"
      "never\nequal\nnot-equal\nless-equal\ngreater-equaln\existn\not-existn\always\n" )
{
    ULONG devIdx;
    ULONG brdIdx;
    /*ULONG   ethIdx;*/
    ULONG qosfieldset;
    ULONG qosfieldrule;
    ULONG qosfield;
    ULONG fieldSelect;
    uchar *matchval;
    ULONG fieldOperator;

    ULONG   ulSlot, ulPort, ulOnuid;

    ULONG pon;
    ULONG onu;

    ULONG suffix;

    int ponid;

    qos_value_t val;

    VOS_MemZero(&val, sizeof(qos_value_t));

    qosfieldset = VOS_AtoI(argv[0]);
    qosfieldrule = VOS_AtoI(argv[1]);
    qosfield = VOS_AtoI(argv[2]);
    fieldSelect = select_field_to_int(argv[3])-1;
    matchval = argv[4];
    fieldOperator = operator_field_to_int(argv[5])-1;

    onuconf_qosRuleFieldValue_StrToVal_parase(fieldSelect, matchval, &val);

    /*ulIfIndex = ( ULONG ) ( vty->index ) ;    */

    if( IsProfileNodeVty(vty->index, &suffix))
    {
        if(setOnuConfQosClassByPtr(suffix, vty->onuconfptr, qosfieldset, qosfieldrule, qosfield, fieldSelect, fieldOperator, &val, sizeof(qos_value_t)) != VOS_OK)
            vty_out(vty, "set qos class ERROR!\r\n");
    }
    else
    {

        if (parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;

        ulSlot = brdIdx;
        ulPort = pon;
        ulOnuid = onu;

        ponid = GetPonPortIdxBySlot(brdIdx, pon);
        if (ponid == -1)
        {
            return CMD_WARNING;
        }

        if(isCtcOnu(brdIdx, pon, onu))
        {

            ONU_CONF_WRITEABLE_CHECK

    #if 0
            if(setQosRuleFieldEntry(devIdx,qosfieldset,qosfieldrule,qosfield,fieldSelect,matchval,fieldOperator)==CTC_STACK_EXIT_ERROR)
            vty_out( vty, " set class map fail!\r\n");
    #else


            if(OnuMgt_SetQosClass(ponid, onu-1, qosfieldset, qosfieldrule, qosfield, fieldSelect, fieldOperator, (char*)&val, sizeof(qos_value_t)) != VOS_OK)
                vty_out(vty, ONU_CMD_ERROR_STR);
    #endif

        }
        else
        {
            char *pbuf = NULL;
            USHORT len;

            if(OnuMgt_CliCall(ponid, onu-1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            vty_big_out(vty, len+4, "\r\n%s\r\n", pbuf);
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }
    return CMD_SUCCESS;
}

DEFUN(undo_onu_conf_qos_class_match,
      undo_onu_conf_qos_class_match_cmd,
         "undo class-match <policy_index> <class_map_index> <match_index> ",
      "undo operation\n"
      "undo class match \n"
      "policy-index\n"
      "class-map-index\n"
       "match-index\n")
{
    ULONG devIdx;
    ULONG brdIdx;
    /*ULONG   ethIdx;*/
    ULONG qosset;
    ULONG qosrule;
    ULONG qosfield;

    ULONG   ulSlot, ulPort, ulOnuid;

    ULONG pon;
    ULONG onu;
    int ponid;
    ULONG suffix;

    qosset = VOS_AtoI(argv[0]);
    qosrule = VOS_AtoI(argv[1]);
    qosfield = VOS_AtoI(argv[2]);

    /*ulIfIndex = ( ULONG ) ( vty->index ) ;    */

    if(IsProfileNodeVty(vty->index, &suffix))
    {
        if(clrOnuConfQosClassByPtr(suffix, vty->onuconfptr, qosset, qosrule, qosfield ) != VOS_OK)
            vty_out(vty, "\r\nclear qos class ERROR\r\n");
    }
    else
    {

        if (parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK)
            return CMD_WARNING;

        ulSlot = brdIdx;
        ulPort = pon;
        ulOnuid = onu;

        ponid = GetPonPortIdxBySlot(brdIdx, pon);
        if (ponid == -1)
        {
            return CMD_WARNING;
        }

        ONU_CONF_WRITEABLE_CHECK

        if(isCtcOnu(brdIdx, pon, onu))
        {

    #if 0
            if(delQosRuleFieldEntry(devIdx,qosset,qosrule,qosfield)==CTC_STACK_EXIT_ERROR)
            vty_out( vty, " undo class match fail!\r\n");
    #else
            if(OnuMgt_ClrQosClass(ponid, onu-1, qosset, qosrule, qosfield) != VOS_OK)
            {
#if 0                
                vty_out(vty, "\r\nclear qos class ERROR\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
    #endif
        }
        else
        {
            char *pbuf = NULL;
            USHORT len;

            if(OnuMgt_CliCall(ponid, onu-1, vty->buf, vty->length, &pbuf, &len) == VOS_OK && len)
		            vty_big_out(vty, len+4, "\r\n%s\r\n", pbuf);
            else
            {
                vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
            }
        }
    }

    return CMD_SUCCESS;
}

#define END_QOS_CLIS

DEFUN(onu_port_multicast_vlan_set,
        onu_port_multicast_vlan_set_cmd,
        "multicast vlan <port_num> {[0|<vlan_list>]}*1",
        "multicast operation\n"
        "vlan operation\n"
        "port number\n"
        "empty port multicast vids\n"
        "vlan list(2-4094),e.g. 100,200,1000 maximum 50 multicast vlan\n")
{

    int ponID = 0;
    ULONG ulIfIndex = 0, port, suffix;
    ULONG ulSlot, ulPort, ulOnuid;
    ULONG ulRet;
    ULONG vlanNum = 0;
    ulIfIndex = (ULONG) vty->index;

    if (IsProfileNodeVty(ulIfIndex, &suffix))
    {

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
	
        if(argc == 2)
        {
            if(VOS_StriCmp(argv[1],"0")==0)
            {
                if(setOnuConfPortMulticastVlanByPtr(suffix, vty->onuconfptr, port, 0) != VOS_OK)
                    vty_out(vty, "delete port %d multicast vlan ERROR!\r\n", port);
        }
        else
        {
                BEGIN_PARSE_VLAN_LIST_TO_VLAN_NUM(argv[1], vlanNum)
                if(setOnuConfPortMulticastVlanByPtr(suffix, vty->onuconfptr, port, vlanNum) != VOS_OK)
                    vty_out(vty, "port %d set multicast vlan %d ERROR!\r\n", port, vlanNum);
                END_PARSE_VLAN_LIST_TO_VLAN_NUM()
            }                
        }
        else
                            {
                                int num = 0, vids[ONU_MAX_IGMP_VLAN];

                                if (getOnuConfPortMulticastVlanByPtr(suffix, vty->onuconfptr, port, &num, vids) == VOS_OK)
                                {
                                    int i = 0;
                                    vty_out(vty, "port %d multicast vlan counter %d\r\n", port, num);
                                    for (i = 0; i < num; i++)
                                    {

                                        if (!(i % 8))
                                            vty_out(vty, "\r\n");
                                        vty_out(vty, "%-08d", vids[i]);
                                    }
                                    vty_out(vty, "\r\n");
                                }
                                else
                                    vty_out(vty, "\r\n\tport multicast vlan get fail!\r\n");
        }
    }
    else
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
        if (ulRet != VOS_OK)
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
        if (ponID == (VOS_ERROR))
        {
            vty_out(vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if (argc == 2)
        {

            ONU_CONF_WRITEABLE_CHECK

            if(VOS_StriCmp(argv[1],"0")==0)
            {
                if (OnuMgt_SetPortMulticastVlan(ponID, ulOnuid - 1, port, 0)!= VOS_OK)
                {
#if 0                    
                    vty_out(vty, "port multicast vlan set fail!\r\n");       
#else
                    vty_out(vty, ONU_CMD_ERROR_STR);
#endif
                }
            }
            else
            {
                BEGIN_PARSE_VLAN_LIST_TO_VLAN_NUM(argv[1], vlanNum)
                if (OnuMgt_SetPortMulticastVlan(ponID, ulOnuid - 1, port, vlanNum)!= VOS_OK)
                {
#if 0                    
                    vty_out(vty, "port multicast vlan set fail!\r\n");
#else
                    error_flag = 0;
#endif
                }
                END_PARSE_VLAN_LIST_TO_VLAN_NUM();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);                    
            }  
        }
        else
        {
            if (isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                int num = 0;
                CTC_STACK_multicast_vlan_t mv;

                if(OnuMgt_GetEthPortMulticastVlan(ponID, ulOnuid-1, port, &mv) == VOS_OK)
                {
                    int i;
                    num = mv.num_of_vlan_id;
                    vty_out(vty, "port %d multicast vlan counter %d\r\n", port, num);
                    for (i = 0; i < num; i++)
                    {

                    if (!(i % 8))
                    vty_out(vty, "\r\n");
                    vty_out(vty, "%-08d", mv.vlan_id[i]);
                    }
                    vty_out(vty, "\r\n");
                }
                else
                {
#if 0                    
                    vty_out(vty, "\r\n\tport multicast vlan get fail!\r\n");
#else
                    vty_out(vty, ONU_CMD_ERROR_STR);
#endif
                }
            }
            else
            {
                char *pRecv;
                USHORT len;

                if (VOS_OK == OnuMgt_CliCall(ponID, ulOnuid - 1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }

    return CMD_SUCCESS;
}

DEFUN(onu_port_igmpsnooping_tagstrip_set,
        onu_port_igmpsnooping_tagstrip_set_cmd,
        "igmpsnooping tag strip <port_list> {[1|0]}*1",
        "igmp command\n"
        "tag operation\n"
        "strip vlan tag\n"
        "port list\n"
        "enable strip\n"
        "disable strip\n")
{

    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {
            ULONG en = VOS_StrToUL(argv[1], NULL, 10);

            ONU_CONF_WRITEABLE_CHECK

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                if( OnuMgt_SetEthPortMulticastTagStrip(ponID, ulOnuid-1, port, en) != VOS_OK)
                {
#if 0                    
                    vty_out(vty, "port %d igmp tag strip set fail!\r\n",port);
#else
                    error_flag = 1;
#endif
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                int en = 0;
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                {

                    /*if(getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_igmp_tag_strip, &en) == VOS_OK)*/
                    if(OnuMgt_GetEthPortMulticastTagStrip(ponID, ulOnuid-1, port, &en) == VOS_OK)
                    {
                        vty_out(vty, "port %d tag strip is %s\r\n", port, en?"enable":"disable");
                    }
                    else
                    {
#if 0                            
                        vty_out(vty, "\r\n\tport igmp tag strip get fail!\r\n");
#else
                        error_flag = 0;
#endif
                    }
                }
                END_PARSE_PORT_LIST_TO_PORT();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);
                        
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG en = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
		
        if(argc == 2)
        {
            en = VOS_StrToUL(argv[1], NULL, 10);

            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_igmp_tag_strip, en))
                vty_out(vty, "port %d igmp tag strip set fail!\r\n", port);

            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(getOnuConfPortSimpleVarByPtr(suffix, pd , port, sv_enum_port_igmp_tag_strip, &en))
                vty_out(vty, "port %d igmp tag strip get fail!\r\n", port);
            else
                vty_out(vty, "port %d igmp tag strip %d\r\n", port ,en);

            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;
}

DEFUN(onu_port_igmpsnooping_tagoper_set,
        onu_port_igmpsnooping_tagoper_set_cmd,
        "igmpsnooping tag oper <port_list> {iptv-vid <1-4094> mc-vid <1-4094>}*8",
        "igmp command\n"
        "tag operation\n"
        "vlan translation operation\n"
        "port list\n"
        "iptv user vlan id\n"
        "vlan id value\n"
        "multicast vlan id\n"
        "vlan id value\n")
{

    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc > 1)
        {
        	int i, j=0;
        	CTC_STACK_multicast_vlan_switching_t sw;
            
        	VOS_MemZero(&sw, sizeof(sw));
        	for(i=1; i<argc; i+=2,j++)
        	{
        		sw.entries[j].iptv_user_vlan = VOS_AtoI(argv[i]);
        		sw.entries[j].multicast_vlan = VOS_AtoI(argv[i+1]);
        	}
        	sw.number_of_entries = j;

            ONU_CONF_WRITEABLE_CHECK

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                if(OnuMgt_SetEthPortMulticastTagOper(ponID, ulOnuid-1, port, CTC_TRANSLATE_VLAN_TAG, &sw) != VOS_OK)
#if 0                    
                	vty_out(vty, "set port igmp tag oper fail!\r\n");
#else
                    error_flag = 1;
#endif
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                    int en = 0;
                    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                    {

                    	CTC_STACK_tag_oper_t oper;
                    	CTC_STACK_multicast_vlan_switching_t sw;
                        if(OnuMgt_GetEthPortMulticastTagOper(ponID, ulOnuid-1, port, &oper, &sw) == VOS_OK)
                        {
                        	switch(oper)
                        	{
                        		case CTC_NO_STRIP_VLAN_TAG:
                        			vty_out(vty, "port %d tag oper is %s\r\n", port, "no-strip");
                        			break;
                        		case CTC_STRIP_VLAN_TAG:
                        			vty_out(vty, "port %d tag oper is %s\r\n", port,  "strip");
                        			break;
                        		case CTC_TRANSLATE_VLAN_TAG:
                        			vty_out(vty, "port %d tag oper is %s\r\n",  port, "translation");
                        			break;
                        		default:
                        			vty_out(vty, "port %d tag oper is %s\r\n",  port, "unknow");
                        			break;
                        	}

                        	if(oper == CTC_TRANSLATE_VLAN_TAG && sw.number_of_entries > 0)
                        	{
                        		int i = 0;

                        		vty_out(vty,"\r\n");
                        		vty_out(vty, "number        iptv-vid        mc-vid\r\n");
                        		for(i=0; i<sw.number_of_entries; i++)
                        		{
                        			vty_out(vty, "%-6d        %-8d        %-6d\r\n", i+1, sw.entries[i].iptv_user_vlan, sw.entries[i].multicast_vlan);
                        		}

                        		vty_out(vty,"\r\n");
                        	}
                        }
                        else
                        {
#if 0                            
                            vty_out(vty, "\r\n\tport igmp tag oper get fail!\r\n");
#else
                            error_flag = 0;
#endif
                        }
                    }
                    END_PARSE_PORT_LIST_TO_PORT();
                    if(error_flag)
                        vty_out(vty, ONU_CMD_ERROR_STR);
                        
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG en = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        if(argc == 2)
        {
            en = VOS_StrToUL(argv[1], NULL, 10);

            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;
}


DEFUN(onu_port_igmpsnooping_groupnum_set,
        onu_port_igmpsnooping_groupnum_set_cmd,
        "igmpsnooping group-num <port_list> {<1-255>}*1",
        "igmp command\n"
        "group number of port\n"
        "port list\n"
        "number\n")
{

    int ponID=0;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet, suffix;

    ulIfIndex =(ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

		VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc == 2)
        {
            ULONG num = VOS_StrToUL(argv[1], NULL, 10);

            ONU_CONF_WRITEABLE_CHECK

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                if( OnuMgt_SetEthPortMulticastGroupMaxNumber(ponID, ulOnuid-1, port, num) != VOS_OK)
                {
#if 0                    
                    vty_out(vty, "port igmp group number set fail!\r\n");
#else
                    error_flag = 1;
#endif
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                    {
                        int num = 0;

                        /*if(getOnuConfPortSimpleVar(ponID, ulOnuid-1, port, sv_enum_port_igmp_max_group, &num) == VOS_OK)*/
                        if(OnuMgt_GetEthPortMulticastGroupMaxNumber(ponID, ulOnuid-1, port, &num) == VOS_OK)
                        {
                            vty_out(vty, "port %d group number is %d\r\n", port,  num);
                        }
                        else
                        {
#if 0                            
                            vty_out(vty, "\r\n\tport igmp group number get fail!\r\n");
#else
                            error_flag = 1;
#endif
                        }
                    }
                    END_PARSE_PORT_LIST_TO_PORT();
                    if(error_flag)
                        vty_out(vty, ONU_CMD_ERROR_STR);
                        
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ULONG num = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
			
        if(argc == 2)
        {
            num = VOS_StrToUL(argv[1], NULL, 10);

            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_port_igmp_max_group, num))
                vty_out(vty, "port %d igmp group number set fail!\r\n", port);

            END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)

            if(getOnuConfPortSimpleVarByPtr(suffix, pd , port, sv_enum_port_igmp_max_group, &num))
                vty_out(vty, "port %d igmp group number get fail!\r\n", port);
            else
                vty_out(vty, "port %d igmp group number %d\r\n", port ,num);

            END_PARSE_PORT_LIST_TO_PORT()
        }
    }

    return CMD_SUCCESS;
}


LDEFUN  (
    onu_mgt_temperature,
    onu_mgt_temperature_cmd,
    "mgt temperature {<30-50>}*1",    
    "Show or config onu mgt information\n"
    "Show or config onu mgt temperature information\n"
    "Please input the max value\n",
    ONU_NODE
    )
{
    return CMD_SUCCESS;
}

LDEFUN  (
    onu_mgt_conf,
    onu_mgt_conf_cmd,
    /* modified by chenfj 2007-10-22
     问题单#5552:命令mgt config disable/enable执行失败
     处理:因为mgt config disable/enable和mgt config clear/save的功能是一样的。去掉mgt config disable/enable
    */
    /*"mgt config [enable|disable|save|clear]",    */
    "mgt config [save|clear]",
    "Show or config onu mgt information\n"
    "Show or config onu mgt config information\n"
    /*"enable save or clear the configuration\n"*/
    /*"disable save or clear the configuration\n"*/
    "Save configuration\n"
    "Clear all configuration\n",
    ONU_NODE
    )
{
    return CMD_SUCCESS;
}

LDEFUN  (
    onu_mgt_reset,
    onu_mgt_reset_cmd,
    "mgt reset {def}*1",    
    "Show or config onu mgt information\n"
    "Show or config onu mgt reset information\n"
    "Reset onu and load default configuration\n",
    ONU_NODE
    )
{
    return CMD_SUCCESS;
}



LDEFUN  (
    onu_mgt_laser,
    onu_mgt_laser_cmd,
    "mgt laser {[0|1]}*1",    
    "Show or config onu mgt information\n"
    "Show or config onu mgt laser information\n"
    "Laser is turned on according to received grants\n"
    "Laser is turned on permanently\n",
    ONU_NODE
    )
{
    return CMD_SUCCESS;
}

LDEFUN  (
    onu_mgt_self_check_result,
    onu_mgt_self_check_result_cmd,
    "mgt self_check_result [0|1|2|3|4|5|6|7|8|9|10|11]",    
    "Show or config onu mgt information\n"
    "Show or config onu mgt self check result information\n"
    "0:Means\n"
    "1:Means\n"
    "2:Means\n"
    "3:Means\n"
    "4:Means\n"
    "5:Means\n"
    "6:Means\n" 
    "7:Means\n"
    "8:Means\n"
    "9:Means\n"
    "10:Means\n"
    "11:Means\n",
    ONU_NODE
    )
{
    return CMD_SUCCESS;
}



/*added by wutw at 19 October*/
LDEFUN  (
    onu_mgt_self_check_all_result,
    onu_mgt_self_check_result_all_cmd,
    "mgt self_check_result",    
    "Show or config onu mgt information\n"
    "Show or config onu mgt self check result information\n",
    ONU_NODE
    )
{
    return CMD_SUCCESS;
}

/*added by wangxy 2007-11-06*/
LDEFUN  (
    onu_mgt_read_reg,
    onu_mgt_read_reg_cmd,
    "mgt read_reg",    
    "Show or config onu mgt information\n"
    "Read all registers for the switch chip\n",
    ONU_NODE
    )
{
    return CMD_SUCCESS;
}

/*added by wutw at 19 October*/
LDEFUN  (
    onu_uptime_show,
    onu_uptime_show_cmd,
    "mgt uptime",    
    "Show or config onu mgt information\n"
    "show onu uptime\n",
    ONU_NODE
    )
{
    return CMD_SUCCESS;
}


/*主题: GT810/816增加的命令行 */ 

LDEFUN( onu_IgmpsnoopAuthEnable_Func,
        onu_Igmpsnoop_Authenable_Cmd,
        "igmpsnooping auth enable {[1|0]}*1",
        "IGMP snooping authentication config\n"
        "IGMP Snooping auth function\n"
        "Enable or disable IGMP Snooping auth\n"
        "Enable IGMP Snooping auth\n"
        "Disable IGMP Snooping auth\n",
    	 ONU_NODE)

{
    return CMD_SUCCESS;
}
 
/*使能或关闭UNI接口的MAC地址学习。*/
/* modified by chenfj 2007-8-30
	按照大亚方式统一一下命令。 张新辉需按照新的定义修改GT816的软件
	*/
LDEFUN(onu_atu_learn,
         onu_atu_learn_cmd,
         "atu learning <port_list> {[1|0]}*1",
         "Mac table config\n"
         "Uni mac learn configuration\n" 
         "input the onu fe port number\n"
         "1-learning enable\n"
         "0-learning disable\n",
    	  ONU_NODE)

{
    return CMD_SUCCESS;
}

/*主题: GT810/816增加的命令行。end */ 

/* added by chenfj 2007-9-17
	在GT811/GT812下增加如下两个命令*/
LDEFUN(onu_atu_flood,
         onu_atu_flood_cmd,
         "atu flood <port_list> {[1|0]}*1",
         "Mac table config\n"
         "Mac table config\n"
         "input the onu fe port number\n"
         "1-enable flood mode\n"
         "0-disable flood mode\n",
   	  ONU_NODE)

{
    return CMD_SUCCESS;
}

LDEFUN(onu_atu_limit,
         onu_atu_limit_cmd,
         "atu limit {[0|<1-64>]}*1",
         "Mac table config\n"
         "pon address learning limit\n"
         "0-no limit\n"
         "the max learning MAC address\n",
         ONU_NODE)

{
    return CMD_SUCCESS;
}

/* added by chenfj 2007-9-17
	DHCP relay功能，dscp_relay_agent : Enable/Disable DHCP Relay Agent ,
	涉及到的ONU：811/812/821/831/810/816
*/
LDEFUN(onu_dscp_relay_agent,
         onu_dscp_relay_agent_cmd,
         "dhcp_relay_agent {[0|1]}*1",
         "Enable/Disable DHCP Relay Agent\n"
         "0-disable\n"
         "1-enable\n",
         ONU_NODE)

{
    return CMD_SUCCESS;
}
/*
发件人: KD刘冬 
发送时间: 2007年10月22日 14:58
收件人: KD陈福军
主题: 请在GT810/816/811a/812a节点下增加Download命令,支持FTP方式下载程序
*/
LDEFUN(ftpc_download_ftp_phenixos_master_func,
 ftpc_download_ftp_phenixos_master_cmd,
 "download ftp [os] <A.B.C.D> <user> <pass> <filename>",
 "Download OS\n"
 "Download file using ftp protocol\n"
 "Download new GROS image\n"
 "Please input ftp server's IP address\n"
 "Please input user name\n" 
 "Please input the password\n" 
 "Please input the file name\n",
    ONU_NODE
 )
{
    return CMD_SUCCESS;
}
/*
发件人: KD刘冬 
发送时间: 2007年10月29日 14:48
收件人: KD陈福军
抄送: KD盖鹏飞
主题: 请在GT811/GT812节点下增加命令
*/
LDEFUN(onu_cable_test_gt811,
		onu_cable_test_gt811_cmd,
		"cable test port <port_list>", 
		"Ethernet cable test\n" 
		"Test ethernet cable characters\n"
		"port number\n"
		DescStringPortList,
   		 ONU_NODE
 		) 
{
    return CMD_SUCCESS;
}

LDEFUN(onu_mgt_config_ip_addr_gt811,
		onu_mgt_config_ip_addr_gt811_cmd,
		"set ip <A.B.C.D/M>",
		"Set system operation\n" 
		"Set ip address in eeprom\n"
		"IP address information(eg 192.168.30.1/24)\n",
    		ONU_NODE
 		)

{
    return CMD_SUCCESS;
}


LDEFUN(onu_mgt_running_config_gt811,
		onu_mgt_running_config_gt811_cmd,
		"show running-config",
		"Show system operation\n" 
		"show system running config\n",
   		 ONU_NODE
 		)

{
    return CMD_SUCCESS;
}	

LDEFUN(onu_mgt_startup_config_gt811,
		onu_mgt_startup_config_gt811_cmd,
		"show startup-config",
		"Show system operation\n" 
		"show system startup config\n",
    		ONU_NODE
 		)

{
    return CMD_SUCCESS;
}	


int check_onu_exception(int onu_type, int *onu_mask)
{
	int i;
	if( (onu_type == 0) || (onu_mask == NULL) )
		return 0;
	for( i=0; i<V2R1_ONU_MAX; i++ )
	{
		if( onu_mask[i] == 0 )
			break;
		if( onu_mask[i] == onu_type )
			return 1;
	}
	return 0;
}

static int rpc_onu_dev_cli_process( struct vty *vty, char *hint_str, int *onu_mask )
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
    int OnuType;

    if( check_onu_cli_is_deny(vty) )    /* modified by xieshl 20100521, 问题单10260 */
        return CMD_SUCCESS;

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

    if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
    {
        vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
        return (CMD_WARNING );
    }
    /* added by chenfj
       问题单6985.[FDB]不支持atu filter
       不再判断ONU 类型, 是否支持cli 命令由ONU 自行处理
       */
    /*if(( OnuType != V2R1_ONU_GT815 ) && ( OnuType != V2R1_ONU_GT816 ) && ( OnuType != V2R1_ONU_GT810 ) && ( OnuType != V2R1_ONU_GT811_A ) && ( OnuType != V2R1_ONU_GT812_A ) && ( OnuType != V2R1_ONU_GT813 )
        && ( OnuIsGT831( PonPortIdx, OnuIdx) != ROK ) )
        {
        vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );
        return( CMD_WARNING );
        }*/
    if( check_onu_exception(OnuType, onu_mask) != 0 )
    {
        vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );
        return( CMD_WARNING );
    }

    VOS_MemCpy( pBuff, vty->buf, vty->length);
    length = vty->length;

    vty_out(vty, "\r\n");

    if(OnuMgt_CliCall(PonPortIdx, OnuIdx, pBuff, length, &stPayload, &length) == VOS_OK && length)
    {
        vty_big_out(vty, length, "%s", stPayload);
        /*vty_out( vty, "  %% %s OK!\r\n", hint_str );*/   /* 问题单9302 */
    }
    else
    {
        vty_out(vty, "  %% %s failed!\r\n", hint_str );
        return CMD_WARNING;
    }

    vty_out(vty, "\r\n");

    return CMD_SUCCESS;
}


/* added by xieshl 2007-9-30 , 用于网通测试 */
static int onu_dev_cli_process( struct vty *vty, char *hint_str, int *onu_mask )
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
	int OnuType;

	if( check_onu_cli_is_deny(vty) )	/* modified by xieshl 20100521, 问题单10260 */
		return CMD_SUCCESS;

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

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
	{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (CMD_WARNING );
	}
	/* added by chenfj 
	   问题单6985.[FDB]不支持atu filter
	   不再判断ONU 类型, 是否支持cli 命令由ONU 自行处理
	   */
	/*if(( OnuType != V2R1_ONU_GT815 ) && ( OnuType != V2R1_ONU_GT816 ) && ( OnuType != V2R1_ONU_GT810 ) && ( OnuType != V2R1_ONU_GT811_A ) && ( OnuType != V2R1_ONU_GT812_A ) && ( OnuType != V2R1_ONU_GT813 ) 
		&& ( OnuIsGT831( PonPortIdx, OnuIdx) != ROK ) )
		{
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}*/
	if( check_onu_exception(OnuType, onu_mask) != 0 )
	{
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
	}

	VOS_MemCpy( pBuff, vty->buf, vty->length);
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
/*	队列信息显示功能, 涉及到的ONU：810/813
*/

DEFUN(onu_pas_pq_set,
	onu_pas_pq_set_cmd,
	"pq limit [up|down|p2c|c2p|u2c] [all|<0-7>] {<0-2288>}*1", 
	"Priority queue config\n"
	"Show/config Priority queue limit\n"
	"Direction: Upstream\n"
	"Direction: Downstream\n"
	"Direction: Pon to Cpu\n"
	"Direction: Cpu to Pon\n"
	"Direction: Uni to Cpu\n"
	"All priority class\n"
	"Priority class\n"
	"Rate limit in 128Bytes blocks\n")
{
	return rpc_onu_dev_cli_process( vty, "onu_pas_pq_set",NULL );
}

/*
** QOS filter rules
*/
DEFUN(onu_qos_filter_l3_rules,
	onu_qos_filter_l3_rules_cmd,
	"qos filter [ip-address |udp-port |tcp-port]  [source|destination] [<A.B.C.D>|<1-65535>|<1-65535>]  [up|down] [drop|cpu|dp|both] [0|1]",
	"QoS config\n"
	"Qos filter config\n"
	"Based on ip address\n"
	"Based on udp port\n" 
	"Based on tcp port\n"
	"Source\n"
	"Destination\n"
	"IP address\n"
	"Udp port\n"
	"Tcp port\n"
	"Upstream\n"
	"Downstream\n" 
	"Drop the stream\n"
	"Forward the stream to cpu only\n"
	"Forward the stream to data path only\n"
	"Forward the stream to cpu and data path\n"
	"Disable\n"
	"Enable\n"
	)
{
	return rpc_onu_dev_cli_process( vty, "onu_qos_filter_l3_rules",NULL );
}

DEFUN(onu_qos_filter_l2_rules,
	onu_qos_filter_l2_rules_cmd,
	"qos filter {[ether-type|ip-protocol|vlan-id] [<ether>|<protocol>|<vid>] [up|down] [drop|cpu|dp|both] [0|1]}*1",
	"QoS config\n"
	"Qos filter config\n"
	"Based on ether type\n"
	"Based on ip protocol\n"
	"Based on vlan id\n"
	"Ether type in HEX (eg. 0x0800)\n"
	"IP protocol value\n"
	"Vlan id\n" 
	"Upstream\n"
	"Downstream\n" 
	"Drop the stream\n"
	"Forward the stream to cpu only\n"
	"Forward the stream to data path only\n"
	"Forward the stream to cpu and data path\n"
	"Disable\n"
	"Enable\n"
	)
{
	return rpc_onu_dev_cli_process( vty, "onu_qos_filter_l2_rules",NULL );
}

/*
** QOS Classfier rules
*/
DEFUN(onu_qos_classifier_l3_rules,
	onu_qos_classifier_l3_rules_cmd,
	"qos classifier [ip-address|udp-port |tcp-port]  [source|destination] [<A.B.C.D> |<1-65535>|<1-65535>] [up|down] <0-7> [0|1]",
	"QoS config\n"
	"Qos classifier config\n"
	"Based on ip address\n"
	"Based on udp port\n" 
	"Based on tcp port\n"
	"Based on source information\n"
	"Based on destination information\n"
	"IP address\n"
	"Udp port\n"
	"Tcp port\n"
	"Upstream\n"
	"Downstream\n" 
	"Priority\n" 
	"Disable\n"
	"Enable\n"
	)
{
	return rpc_onu_dev_cli_process( vty, "onu_qos_classfier_l3_rules",NULL );
}

DEFUN(onu_qos_classifier_l2_rules,
	onu_qos_classifier_l2_rules_cmd,
	"qos classifier {[ether-type|ip-protocol |vlan-id]  [<ether> |<protocol>|<vid>] [up|down] <0-7> <0-7> [0|1]}*1",
	"QoS config\n"
	"Qos classifier config\n"
	"Based on ether type\n"
	"Based on ip protocol\n"
	"Based on vlan id\n"
	"Ether type in HEX (eg. 0x0800)\n"
	"IP protocol value\n"
	"Vlan id\n" 
	"Upstream\n"
	"Downstream\n" 
	"Priority\n" 
	"Queue\n"
	"Disable\n"
	"Enable\n"
	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return rpc_onu_dev_cli_process( vty, "onu_qos_classfier_l2_rules",NULL );
#elif defined __RPC_ONU_COMMAND
    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc != 0)
        {
            int dir = VOS_StrCmp(argv[2], "up")?QOS_RULE_DOWN_DIRECTION:QOS_RULE_UP_DIRECTION;
            gw_rule_t rule;
            int code = VOS_StrToUL(argv[5], NULL, 10)==1?SET_QOS_RULE:CLR_QOS_RULE;
            VOS_MemZero(&rule, sizeof(gw_rule_t));
            
            if(VOS_StrCmp(argv[0], "ether-type") == 0)
            {
                rule.qos_rule.mode = BASE_ON_ETHER_TYPE;
                rule.qos_rule.value = VOS_StrToUL(argv[1], NULL, 16);
            }
            else if(VOS_StrCmp(argv[0], "ip-protocol") == 0)
            {
                rule.qos_rule.mode = BASE_ON_IP_PROTOCAL;
                rule.qos_rule.value = VOS_StrToUL(argv[1], NULL, 10);                
            }
            else
            {
                rule.qos_rule.mode = BASE_ON_VLAN_ID;
                rule.qos_rule.value = VOS_StrToUL(argv[1], NULL, 10);                
            }
            rule.qos_rule.priority_mark = (unsigned char)VOS_StrToUL(argv[3], NULL, 10);
            rule.qos_rule.queue_mapped = (unsigned char)VOS_StrToUL(argv[4], NULL, 10);
            
            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK == OnuMgt_SetRule(ponID, ulOnuid-1, dir, code, rule))
            {
            }
            else
            {
#if 0                
                vty_out(vty, "Qos rule set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(argc != 0)
        {
            int dir = VOS_StrCmp(argv[2], "up")?QOS_RULE_DOWN_DIRECTION:QOS_RULE_UP_DIRECTION;
            gw_qos_classification_rule_t qos_rule;
            int code = VOS_StrToUL(argv[5], NULL, 10)==1?SET_QOS_RULE:CLR_QOS_RULE;
            VOS_MemZero(&qos_rule, sizeof(gw_qos_classification_rule_t));
            
            if(VOS_StrCmp(argv[0], "ether-type") == 0)
            {
                qos_rule.mode = BASE_ON_ETHER_TYPE;
                qos_rule.value = VOS_StrToUL(argv[1], NULL, 16);
            }
            else if(VOS_StrCmp(argv[0], "ip-protocol") == 0)
            {
                qos_rule.mode = BASE_ON_IP_PROTOCAL;
                qos_rule.value = VOS_StrToUL(argv[1], NULL, 10);                
            }
            else
            {
                qos_rule.mode = BASE_ON_VLAN_ID;
                qos_rule.value = VOS_StrToUL(argv[1], NULL, 10);                
            }
            qos_rule.priority_mark = (unsigned char)VOS_StrToUL(argv[3], NULL, 10);
            qos_rule.queue_mapped = (unsigned char)VOS_StrToUL(argv[4], NULL, 10);
            if(code == SET_QOS_RULE)
            {
                if(SetOnuConf_Qos_RuleByPtr(suffix, pd, dir, qos_rule))
                    vty_out(vty, "Qos rule mode set fail!\r\n");
            }
            else
            {
                if(ClrOnuConf_Qos_RuleByPtr(suffix, pd, dir, qos_rule))
                    vty_out(vty, "Qos rule mode del fail!\r\n");                
            }
        }
        else
        {
            int index = 0;
            gw_qos_classification_rule_t qos_rule;
            VOS_MemZero(&qos_rule, sizeof(gw_qos_classification_rule_t));
            
            vty_out(vty, "  count    qualifier          value          direction      priority    queue\r\n");            
            vty_out(vty, "------------------------------------------------------------------------------\r\n");
            for(index=0;index<MAX_CLASS_RULES_FOR_SINGLE_PATH;index++)
            {
                if(getOnuConf_Qos_RuleByPtr(suffix, pd , QOS_RULE_UP_DIRECTION, index, &qos_rule) == VOS_OK)
                {
                    if(qos_rule.mode == BASE_ON_ETHER_TYPE)
                        vty_out(vty, "%6d     %-9s          0x%-4x         %9s      %8d    %5d\r\n", index+1, "ETHER", qos_rule.value, "UP", qos_rule.priority_mark, qos_rule.queue_mapped);
                    else
                        vty_out(vty, "%6d     %-9s          %-6d         %9s      %8d    %5d\r\n", index+1, qos_rule.mode == BASE_ON_VLAN_ID?"VLAN":"IP-PROTO", qos_rule.value, "UP", qos_rule.priority_mark, qos_rule.queue_mapped);
                }
            }
            for(index=0;index<MAX_CLASS_RULES_FOR_SINGLE_PATH;index++)
            {
                if(getOnuConf_Qos_RuleByPtr(suffix, pd , QOS_RULE_DOWN_DIRECTION, index, &qos_rule) == VOS_OK)
                {
                    if(qos_rule.mode == BASE_ON_ETHER_TYPE)
                        vty_out(vty, "%6d     %-9s          0x%-4x         %9s      %8d    %5d\r\n", index+1, "ETHER", qos_rule.value, "DOWN", qos_rule.priority_mark, qos_rule.queue_mapped);
                    else
                        vty_out(vty, "%6d     %-9s          %-6d         %9s      %8d    %5d\r\n", index+1, qos_rule.mode == BASE_ON_VLAN_ID?"VLAN":"IP-PROTO", qos_rule.value, "DOWN", qos_rule.priority_mark, qos_rule.queue_mapped);
                }
            }   
            vty_out(vty, "------------------------------------------------------------------------------\r\n");            
        }
    }
    return CMD_SUCCESS;
#endif
}

/*
** MAC地址过滤
*/
DEFUN(onu_atu_filter,
	onu_atu_filter_cmd,
	"atu filter {[source|dest] <mac> [drop|cpu|dp|both] [0|1]}*1",
	"Mac table config\n"
	"Filter the packet with specified mac address\n" 
	"According to source mac address\n"
	"Destination mac address\n"
	"Specify the mac address (HH-HH-HH-HH-HH-HH)\n"
	"Drop the packet\n"
	"Forward the packet to CPU\n"
	"Forward the packet to data-path\n"
	"Forward the packet to CPU and data-path\n"
	"Disable the filter\n"
	"Enable the filter\n"
	)
{
	return rpc_onu_dev_cli_process( vty, "onu_atu_filter" ,NULL);
}
/* end 2007-9-30 */
/* added by xieshl 2007-9-30 , 用于网通测试 */
DEFUN(onu_qos_vlan_priority_mode,
	onu_qos_vlan_priority_mode_cmd,
	"qos vlan priority_mode {[up|down] [priority_translation|priority_vid]}*1",
	"QoS config\n"
	"Qos vlan rule config\n"
	"Vlan priority mode\n"
	"Upstream direction\n"
	"Downstream direction\n"
	"Prioritization translation (default)\n"
	"Prioritization by Vlan ID (When changing to prioritize by VID mode all VLAN rules are removed)\n"
	)
{
#ifdef __CTC_TEST_NMS_COMMAND
	return rpc_onu_dev_cli_process( vty, "onu_qos_vlan_priority_mode" ,NULL);
#elif defined __RPC_ONU_COMMAND
    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc != 0)
        {
            int dir = VOS_StrCmp(argv[0], "up")?QOS_RULE_DOWN_DIRECTION:QOS_RULE_UP_DIRECTION;
            unsigned char mode = VOS_StrCmp(argv[1], "priority_vid")?QOS_MODE_PRIO_TRANS:QOS_MODE_PRIO_VID;
            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK == OnuMgt_SetQosRuleMode(ponID, ulOnuid-1, dir, mode) )
            {
            }
            else
            {
#if 0                
                vty_out(vty, "Qos rule mode set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);    
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        unsigned char mode = 0;
        if(argc != 0)
        {
            int dir = VOS_StrCmp(argv[0], "up")?QOS_RULE_DOWN_DIRECTION:QOS_RULE_UP_DIRECTION;
            mode = VOS_StrCmp(argv[1], "priority_vid")?QOS_MODE_PRIO_TRANS:QOS_MODE_PRIO_VID;
            
            if(SetOnuConf_Qos_ModeByPtr(suffix, pd, dir, mode))
                vty_out(vty, "Qos rule mode set fail!\r\n");
        }
        else
        {
            if(getOnuConf_Qos_ModeByPtr(suffix, pd , QOS_RULE_UP_DIRECTION, &mode) == VOS_OK)
                vty_out(vty, "  Upstream      :    %s\r\n", mode == QOS_MODE_PRIO_VID?"Priority by vlan ID":"Vlan priority translation");
            if(getOnuConf_Qos_ModeByPtr(suffix, pd , QOS_RULE_DOWN_DIRECTION, &mode) == VOS_OK)
                vty_out(vty, "  Downstream    :    %s\r\n", mode == QOS_MODE_PRIO_VID?"Priority by vlan ID":"Vlan priority translation");            
        }
    }
    return CMD_SUCCESS;
#endif
}

DEFUN(onu_qos_classfier_pas_show,
	onu_qos_classfier_pas_show_cmd,
	"qos pas_classfier",
	"Show QoS config\n"
	"Qos classifier in pas chipset\n"
	)
{
	return rpc_onu_dev_cli_process( vty, "onu_qos_classfier_pas_show",NULL );
}

DEFUN(onu_qos_precedence,
	onu_qos_precedence_cmd,
	"qos precedence {[up|down] [l2|l3]}*1",
	"QoS config\n" 
	"Qos precedence configuration\n"
	"Up stream\n"
	"Down stream\n"
	"Enable layer 2 qos (Based on VLAN priority, IP-TOS/DSCP, VLAN-ID)\n" 
	"Enable layer 3 qos (Based on IP address and UDP/TCP port)\n"
	)
{
	return rpc_onu_dev_cli_process( vty, "onu_qos_precedence",NULL );
}
DEFUN(onu_qos_classifier_rules_precedence,
	onu_qos_classifier_rules_precedence_cmd,
	"qos precedence {[up|down] [234|243|342|432|34|43|2--]}*1",
	"QoS config\n"
	"Qos precedence config\n"
	"Upstream\n"
	"Downstream\n" 
	"precedence: [vlan] L2, L3, L4\n" 
	"precedence: [vlan] L2, L4, L3\n"
	"precedence: L3, L4, [vlan] L2\n"
	"precedence: L4, L3, [vlan] L2\n"
	"precedence: L3, L4 [vlan]\n"
	"precedence: L4, L3 [vlan]\n"
	"precedence: [L2 ether type/IP protocol] [vlan] L2\n"
	)
{
	return rpc_onu_dev_cli_process( vty, "onu_qos_classifier_rules_precedence",NULL );
}
DEFUN(onu_vlan_pas_rule,
	onu_vlan_pas_rule_cmd,
	"vlan pas-rule {[ether-type|ip-protocol|vlan-id] [<ether> |<protocol>|<vid>] [up|down] [attach|exchange|none] <2-4094> [8100|9100|88A8|<UserDefined>][original|classifier] [0|1]}*1",
	"vlan config\n"
	"vlan rule config\n"
	"Based on ether type\n"
	"Based on ip protocol\n"
	"Based on vlan id\n"
	"Ether type in HEX (eg. 0x0800)\n"
	"IP protocol value NOT in HEX (eg. 800)\n"
	"Vlan id (eg. 100,200 (NOT in HEX))\n" 
	"Upstream\n"
	"Downstream\n" 
	"For vlan stacking\n" 
	"For vlan translation and force vlan priority\n" 
	"Do nothing\n" 
	"New Vlan id\n" 
	"Vlan type field in tag = 0x8100\n" 
	"Vlan type field in tag = 0x9100\n" 
	"Vlan type field in tag = 0x88A8\n" 
	"Vlan type field in tag = User-defined in HEX (eg. 0x8100)\n" 
	"Priority in new vlan tag copied from original tag\n" 
	"Priority in new vlan tag copied from classifier rule\n" 
	"Disable\n" 
	"Enable\n" )
{
#ifdef __CTC_TEST_NMS_COMMAND
	return rpc_onu_dev_cli_process( vty, "onu_vlan_pas_rule",NULL );
#elif defined __RPC_ONU_COMMAND
    int ponID=0;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;

    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        if(argc != 0)
        {
            int dir = VOS_StrCmp(argv[2], "up")?QOS_RULE_DOWN_DIRECTION:QOS_RULE_UP_DIRECTION;
            gw_rule_t rule;
            int code = VOS_StrToUL(argv[7], NULL, 10)==1?SET_PAS_RULE:CLR_PAS_RULE;
            VOS_MemZero(&rule, sizeof(gw_rule_t));
            
            if(VOS_StrCmp(argv[0], "ether-type") == 0)
            {
                rule.pas_rule.mode = BASE_ON_ETHER_TYPE;
                rule.pas_rule.value = VOS_StrToUL(argv[1], NULL, 16);
            }
            else if(VOS_StrCmp(argv[0], "ip-protocol") == 0)
            {
                rule.pas_rule.mode = BASE_ON_IP_PROTOCAL;
                rule.pas_rule.value = VOS_StrToUL(argv[1], NULL, 10);                
            }
            else
            {
                rule.pas_rule.mode = BASE_ON_VLAN_ID;
                rule.pas_rule.value = VOS_StrToUL(argv[1], NULL, 10);                
            }
            if(VOS_StrCmp(argv[3], "attach") == 0)
                rule.pas_rule.action = PAS_RULE_ACTION_ATTACH;
            else if(VOS_StrCmp(argv[3], "exchange") == 0)
                rule.pas_rule.action = PAS_RULE_ACTION_EXCHANGE;
            else
                rule.pas_rule.action = PAS_RULE_ACTION_NONE;

            rule.pas_rule.new_vid = (unsigned short)VOS_StrToUL(argv[4], NULL, 10);
            rule.pas_rule.vlan_type = (unsigned short)VOS_StrToUL(argv[5], NULL, 16);

            rule.pas_rule.prio_source = VOS_StrCmp(argv[6], "original")?PAS_RULE_PRIO_SOURCE_CLASSIFIER:PAS_RULE_PRIO_SOURCE_ORIGINAL;
            ONU_CONF_WRITEABLE_CHECK

            if(VOS_OK == OnuMgt_SetRule(ponID, ulOnuid-1, dir, code, rule))
            {
            }
            else
            {
#if 0                
                vty_out(vty, "Vlan Pas-rule set fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
        }
        else
        {
            if(isCtcOnu(ulSlot, ulPort, ulOnuid))
            {
                
            }
            else
            {
                char *pRecv;
                USHORT len;

                if(VOS_OK == OnuMgt_CliCall(ponID, ulOnuid-1, vty->buf, vty->length, &pRecv, &len) && pRecv && len)
                {
		            vty_big_out(vty, len, "%s", pRecv);
                }
                else
                {
                    vty_out(vty, "%s\r\n", ONU_CONF_OAM_ERR_STR);
                }
            }
        }
    }
    else
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(argc != 0)
        {
            int dir = VOS_StrCmp(argv[2], "up")?QOS_RULE_DOWN_DIRECTION:QOS_RULE_UP_DIRECTION;
            gw_pas_rule_t pas_rule;
            int code = VOS_StrToUL(argv[7], NULL, 10)==1?SET_PAS_RULE:CLR_PAS_RULE;
            VOS_MemZero(&pas_rule, sizeof(gw_pas_rule_t));
            
            if(VOS_StrCmp(argv[0], "ether-type") == 0)
            {
                pas_rule.mode = BASE_ON_ETHER_TYPE;
                pas_rule.value = VOS_StrToUL(argv[1], NULL, 16);
            }
            else if(VOS_StrCmp(argv[0], "ip-protocol") == 0)
            {
                pas_rule.mode = BASE_ON_IP_PROTOCAL;
                pas_rule.value = VOS_StrToUL(argv[1], NULL, 10);                
            }
            else
            {
                pas_rule.mode = BASE_ON_VLAN_ID;
                pas_rule.value = VOS_StrToUL(argv[1], NULL, 10);                
            }
            if(VOS_StrCmp(argv[3], "attach") == 0)
                pas_rule.action = PAS_RULE_ACTION_ATTACH;
            else if(VOS_StrCmp(argv[3], "exchange") == 0)
                pas_rule.action = PAS_RULE_ACTION_EXCHANGE;
            else
                pas_rule.action = PAS_RULE_ACTION_NONE;

            pas_rule.new_vid = (unsigned short)VOS_StrToUL(argv[4], NULL, 10);
            pas_rule.vlan_type = (unsigned short)VOS_StrToUL(argv[5], NULL, 16);

            pas_rule.prio_source = VOS_StrCmp(argv[6], "original")?PAS_RULE_PRIO_SOURCE_CLASSIFIER:PAS_RULE_PRIO_SOURCE_ORIGINAL;
            
            if(code == SET_PAS_RULE)
            {
                if(SetOnuConf_Pas_RuleByPtr(suffix, pd, dir, pas_rule))
                    vty_out(vty, "Qos rule mode set fail!\r\n");
            }
            else
            {
                if(ClrOnuConf_Pas_RuleByPtr(suffix, pd, dir, pas_rule))
                    vty_out(vty, "Qos rule mode del fail!\r\n");
            }
        }
        else
        {
            int index = 0;
            gw_pas_rule_t pas_rule;
            int i = 1;
            VOS_MemZero(&pas_rule, sizeof(gw_pas_rule_t));
            
            vty_out(vty, "  count  qualifier  value           direction  prio-source  new-vid  vlan-type  action\r\n");            
            vty_out(vty, "----------------------------------------------------------------------------------------\r\n");
            for(index=0;index<MAX_CLASS_RULES_FOR_SINGLE_PATH;index++)
            {
                if(getOnuConf_Pas_RuleByPtr(suffix, pd , QOS_RULE_UP_DIRECTION, index, &pas_rule) == VOS_OK)
                {
                    if(pas_rule.mode == BASE_ON_ETHER_TYPE)
                        vty_out(vty, "%7d  %-9s  0x%-4x          %-9s  %-11s  %7d  0x%-7x  %-5s\r\n", i, "ETHER", pas_rule.value, "UP", 
                        pas_rule.prio_source==PAS_RULE_PRIO_SOURCE_CLASSIFIER?"CLASSIFIER":"ORIGINAL", pas_rule.new_vid, 
                        pas_rule.vlan_type, pas_rule.action == PAS_RULE_ACTION_NONE?"NONE":pas_rule.action==PAS_RULE_ACTION_ATTACH?"ATTACH":"EXCHANGE");
                    else
                        vty_out(vty, "%7d  %-9s  %-6d          %-9s  %-11s  %7d  0x%-7x  %-5s\r\n", i, pas_rule.mode == BASE_ON_VLAN_ID?"VLAN":"IP-PROTO", pas_rule.value, "UP", 
                        pas_rule.prio_source==PAS_RULE_PRIO_SOURCE_CLASSIFIER?"CLASSIFIER":"ORIGINAL", pas_rule.new_vid, 
                        pas_rule.vlan_type, pas_rule.action == PAS_RULE_ACTION_NONE?"NONE":pas_rule.action==PAS_RULE_ACTION_ATTACH?"ATTACH":"EXCHANGE");
                    i++;
                }
            }
            for(index=0;index<MAX_CLASS_RULES_FOR_SINGLE_PATH;index++)
            {
                if(getOnuConf_Pas_RuleByPtr(suffix, pd , QOS_RULE_DOWN_DIRECTION, index, &pas_rule) == VOS_OK)
                {
                    if(pas_rule.mode == BASE_ON_ETHER_TYPE)
                        vty_out(vty, "%7d  %-9s  0x%-4x          %-9s  %-11s  %7d  0x%-7x  %-5s\r\n", i, "ETHER", pas_rule.value, "DOWN", 
                        pas_rule.prio_source==PAS_RULE_PRIO_SOURCE_CLASSIFIER?"CLASSIFIER":"ORIGINAL", pas_rule.new_vid, 
                        pas_rule.vlan_type, pas_rule.action == PAS_RULE_ACTION_NONE?"NONE":pas_rule.action==PAS_RULE_ACTION_ATTACH?"ATTACH":"EXCHANGE");
                    else
                        vty_out(vty, "%7d  %-9s  %-6d          %-9s  %-11s  %7d  0x%-7x  %-5s\r\n", i, pas_rule.mode == BASE_ON_VLAN_ID?"VLAN":"IP-PROTO", pas_rule.value, "DOWN", 
                        pas_rule.prio_source==PAS_RULE_PRIO_SOURCE_CLASSIFIER?"CLASSIFIER":"ORIGINAL", pas_rule.new_vid, 
                        pas_rule.vlan_type, pas_rule.action == PAS_RULE_ACTION_NONE?"NONE":pas_rule.action==PAS_RULE_ACTION_ATTACH?"ATTACH":"EXCHANGE");
                    i++;                    
                }
            }   
            vty_out(vty, "----------------------------------------------------------------------------------------\r\n");
        }
    }
    return CMD_SUCCESS;
#endif
}

DEFUN(onu_atu_port_learn_limit,
 onu_atu_port_learn_limit_cmd,
 "atu port-limit {<port_list> <1-8192>}*1",
 "Mac table config\n"
 "Port mac learn limit\n" 
 "Specify interface's port (1-24) list(e.g.: 1; 1,2; 1-24)\n"
 "MAC learn limits <1-8192>\n")
{
	return rpc_onu_dev_cli_process( vty, "onu_atu_port_learn_limit" ,NULL);
}

#if 0
DEFUN  (
    onu_profile_associate,
    onu_profile_associate_cmd,
    "onu profile associate <name>",
    "onu configuration\n"
    "profile create\n"
    "associate operation\n"
    "Specify onu profile name\n"
    )
{

    onu_profile_associate_by_index(vty, vty->index, argv[0]);

    return CMD_SUCCESS;

}

DEFUN  (
    onu_profile_private,
    onu_profile_private_cmd,
    "onu profile private <name>",
    "onu configuration\n"
    "profile create\n"
    "private the profile\n"
    "Specify onu profile name\n"
    )
{

    LONG    lRet;
    ULONG   ulSlot, ulPort, ulOnuid, onuEntry;
    short int ponPortIdx;

    lRet = PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnuid );
    if( lRet != VOS_OK )
    {
        vty_out(vty, "invalid index!\r\n");
        return CMD_WARNING;
    }

    ponPortIdx = GetGlobalPonPortIdxBySlot(ulSlot, ulPort);
    onuEntry = ponPortIdx*MAXONUPERPON+ulOnuid-1;

    if(onuDumpConfData(ponPortIdx, ulOnuid-1, argv[0]) == VOS_OK)
    {
#ifdef ONUID_MAP
        ONU_CONF_SEM_TAKE
        {
#if 1
        if(VOS_OK != OnuProfile_Action_ByCode(OnuMap_Update, 0, ponPortIdx, ulOnuid-1, argv[0], NULL, NULL))
#else
        if(setOnuConfOnuIdMapByPonId(ponPortIdx, ulOnuid-1, argv[0]) != VOS_OK ||
                setOnuConfNameMapByPonId(argv[0], ponPortIdx, ulOnuid-1) != VOS_OK)
#endif                
            vty_out(vty, "associate file %s fail!\r\n", argv[0]);
        }
        ONU_CONF_SEM_GIVE
#else
        ONU_MGMT_SEM_TAKE

        VOS_StrCpy(OnuMgmtTable[onuEntry].configFileName, argv[0]);

        ONU_MGMT_SEM_GIVE
#endif
    }
    else
    {
        vty_out(vty, "private config file create fail!\r\n");
        return CMD_WARNING;
    }

    return CMD_SUCCESS;

}
#endif
LDEFUN(onu_pas_pq_set,
	onu_pas_pq_set_cmd,
	"pq limit [up|down|p2c|c2p|u2c] [all|<0-7>] {<0-2288>}*1", 
	"Priority queue config\n"
	"Show/config Priority queue limit\n"
	"Direction: Upstream\n"
	"Direction: Downstream\n"
	"Direction: Pon to Cpu\n"
	"Direction: Cpu to Pon\n"
	"Direction: Uni to Cpu\n"
	"All priority class\n"
	"Priority class\n"
	"Rate limit in 128Bytes blocks\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}

/*
** QOS filter rules
*/
LDEFUN(onu_qos_filter_l3_rules,
	onu_qos_filter_l3_rules_cmd,
	"qos filter [ip-address |udp-port |tcp-port]  [source|destination] [<A.B.C.D>|<1-65535>|<1-65535>]  [up|down] [drop|cpu|dp|both] [0|1]",
	"QoS config\n"
	"Qos filter config\n"
	"Based on ip address\n"
	"Based on udp port\n" 
	"Based on tcp port\n"
	"Source\n"
	"Destination\n"
	"IP address\n"
	"Udp port\n"
	"Tcp port\n"
	"Upstream\n"
	"Downstream\n" 
	"Drop the stream\n"
	"Forward the stream to cpu only\n"
	"Forward the stream to data path only\n"
	"Forward the stream to cpu and data path\n"
	"Disable\n"
	"Enable\n",
	ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(onu_qos_filter_l2_rules,
	onu_qos_filter_l2_rules_cmd,
	"qos filter {[ether-type|ip-protocol|vlan-id] [<ether>|<protocol>|<vid>] [up|down] [drop|cpu|dp|both] [0|1]}*1",
	"QoS config\n"
	"Qos filter config\n"
	"Based on ether type\n"
	"Based on ip protocol\n"
	"Based on vlan id\n"
	"Ether type in HEX (eg. 0x0800)\n"
	"IP protocol value\n"
	"Vlan id\n" 
	"Upstream\n"
	"Downstream\n" 
	"Drop the stream\n"
	"Forward the stream to cpu only\n"
	"Forward the stream to data path only\n"
	"Forward the stream to cpu and data path\n"
	"Disable\n"
	"Enable\n",
	ONU_NODE
	)
{
	return CMD_SUCCESS;
}

/*
** QOS Classfier rules
*/
LDEFUN(onu_qos_classifier_l3_rules,
	onu_qos_classifier_l3_rules_cmd,
	"qos classifier [ip-address|udp-port |tcp-port]  [source|destination] [<A.B.C.D> |<1-65535>|<1-65535>] [up|down] <0-7> [0|1]",
	"QoS config\n"
	"Qos classifier config\n"
	"Based on ip address\n"
	"Based on udp port\n" 
	"Based on tcp port\n"
	"Based on source information\n"
	"Based on destination information\n"
	"IP address\n"
	"Udp port\n"
	"Tcp port\n"
	"Upstream\n"
	"Downstream\n" 
	"Priority\n" 
	"Disable\n"
	"Enable\n",
	ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(onu_qos_classifier_l2_rules,
	onu_qos_classifier_l2_rules_cmd,
	"qos classifier {[ether-type|ip-protocol |vlan-id]  [<ether> |<protocol>|<vid>] [up|down] <0-7> <0-7> [0|1]}*1",
	"QoS config\n"
	"Qos classifier config\n"
	"Based on ether type\n"
	"Based on ip protocol\n"
	"Based on vlan id\n"
	"Ether type in HEX (eg. 0x0800)\n"
	"IP protocol value\n"
	"Vlan id\n" 
	"Upstream\n"
	"Downstream\n" 
	"Priority\n" 
	"Queue\n"
	"Disable\n"
	"Enable\n",
	ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(onu_atu_filter,
	onu_atu_filter_cmd,
	"atu filter {[source|dest] <mac> [drop|cpu|dp|both] [0|1]}*1",
	"Mac table config\n"
	"Filter the packet with specified mac address\n" 
	"According to source mac address\n"
	"Destination mac address\n"
	"Specify the mac address (HH-HH-HH-HH-HH-HH)\n"
	"Drop the packet\n"
	"Forward the packet to CPU\n"
	"Forward the packet to data-path\n"
	"Forward the packet to CPU and data-path\n"
	"Disable the filter\n"
	"Enable the filter\n",
	ONU_NODE
	)
{
	return CMD_SUCCESS;
}
/* end 2007-9-30 */
/* added by xieshl 2007-9-30 , 用于网通测试 */
LDEFUN(onu_qos_vlan_priority_mode,
	onu_qos_vlan_priority_mode_cmd,
	"qos vlan priority_mode {[up|down] [priority_translation|priority_vid]}*1",
	"QoS config\n"
	"Qos vlan rule config\n"
	"Vlan priority mode\n"
	"Upstream direction\n"
	"Downstream direction\n"
	"Prioritization translation (default)\n"
	"Prioritization by Vlan ID (When changing to prioritize by VID mode all VLAN rules are removed)\n",
	ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(onu_qos_classfier_pas_show,
	onu_qos_classfier_pas_show_cmd,
	"qos pas_classfier",
	"Show QoS config\n"
	"Qos classifier in pas chipset\n",
	ONU_NODE
	)
{
	return CMD_SUCCESS;
}

LDEFUN(onu_qos_precedence,
	onu_qos_precedence_cmd,
	"qos precedence {[up|down] [l2|l3]}*1",
	"QoS config\n" 
	"Qos precedence configuration\n"
	"Up stream\n"
	"Down stream\n"
	"Enable layer 2 qos (Based on VLAN priority, IP-TOS/DSCP, VLAN-ID)\n" 
	"Enable layer 3 qos (Based on IP address and UDP/TCP port)\n",
	ONU_NODE
	)
{
	return CMD_SUCCESS;
}
LDEFUN(onu_qos_classifier_rules_precedence,
	onu_qos_classifier_rules_precedence_cmd,
	"qos precedence {[up|down] [234|243|342|432|34|43|2--]}*1",
	"QoS config\n"
	"Qos precedence config\n"
	"Upstream\n"
	"Downstream\n" 
	"precedence: [vlan] L2, L3, L4\n" 
	"precedence: [vlan] L2, L4, L3\n"
	"precedence: L3, L4, [vlan] L2\n"
	"precedence: L4, L3, [vlan] L2\n"
	"precedence: L3, L4 [vlan]\n"
	"precedence: L4, L3 [vlan]\n"
	"precedence: [L2 ether type/IP protocol] [vlan] L2\n",
	ONU_NODE
	)
{
	return CMD_SUCCESS;
}
LDEFUN(onu_vlan_pas_rule,
	onu_vlan_pas_rule_cmd,
	"vlan pas-rule {[ether-type|ip-protocol|vlan-id] [<ether> |<protocol>|<vid>] [up|down] [attach|exchange|none] <2-4094> [8100|9100|88A8|<UserDefined>][original|classifier] [0|1]}*1",
	"vlan config\n"
	"vlan rule config\n"
	"Based on ether type\n"
	"Based on ip protocol\n"
	"Based on vlan id\n"
	"Ether type in HEX (eg. 0x0800)\n"
	"IP protocol value NOT in HEX (eg. 800)\n"
	"Vlan id (eg. 100,200 (NOT in HEX))\n" 
	"Upstream\n"
	"Downstream\n" 
	"For vlan stacking\n" 
	"For vlan translation and force vlan priority\n" 
	"Do nothing\n" 
	"New Vlan id\n" 
	"Vlan type field in tag = 0x8100\n" 
	"Vlan type field in tag = 0x9100\n" 
	"Vlan type field in tag = 0x88A8\n" 
	"Vlan type field in tag = User-defined in HEX (eg. 0x8100)\n" 
	"Priority in new vlan tag copied from original tag\n" 
	"Priority in new vlan tag copied from classifier rule\n" 
	"Disable\n" 
	"Enable\n",
	ONU_NODE )
{
	return CMD_SUCCESS;
}

LDEFUN(onu_atu_port_learn_limit,
 onu_atu_port_learn_limit_cmd,
 "atu port-limit {<port_list> <1-8192>}*1",
 "Mac table config\n"
 "Port mac learn limit\n" 
 "Specify interface's port (1-24) list(e.g.: 1; 1,2; 1-24)\n"
 "MAC learn limits <1-8192>\n",
  ONU_NODE)
{
	return CMD_SUCCESS;
}
/* added by xieshl 2007-10-10, only for CNC test */
#ifdef  CNC_2007_10_TEST

#define MAX_CNC_TEST_VLAN_NUM	4094
typedef struct {
	ulong_t tagPortlist;
	ulong_t untagPortlist;
	uchar_t isValid;
} cnc_test_onu_vlan_t;
static cnc_test_onu_vlan_t onu_vlan_table[MAXONUPERPONNOLIMIT][MAX_CNC_TEST_VLAN_NUM+1];

void init_cnc_test_onu_vlan()
{
	int i;
	VOS_MemZero( (VOID*)&onu_vlan_table[0][0], sizeof( onu_vlan_table) );

	for( i=0; i<MAXONUPERPON; i++ )
	{
		onu_vlan_table[i][1].isValid = 1;
		onu_vlan_table[i][1].tagPortlist = 0;
		onu_vlan_table[i][1].untagPortlist = 0xffffff;
	}
}
int check_cnc_test_vlan_onu_type( struct vty *vty, ULONG *pSlot, ULONG *pPon, ULONG *pOnu )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
	int OnuType;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);

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
		return (VOS_ERROR );
	}

	if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
	{
		vty_out(vty, "  %% Get onu %d/%d/%d type err\r\n", ulSlot, ulPort, ulOnuid );
		return (VOS_ERROR );
	}

	if(( OnuType != V2R1_ONU_GT816 ) && ( OnuType != V2R1_ONU_GT810 ) && ( OnuType != V2R1_ONU_GT813 ) && ( onuType != V2R1_ONU_GT866) )
	{
		vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );
		return( VOS_ERROR );
	}

	if( (ulOnuid > MAXONUPERPON) || (ulOnuid == 0) )
		ulOnuid = 0;
	else
		ulOnuid--;
	*pSlot = ulSlot;
	*pPon = ulPort;
	*pOnu = ulOnuid;

	return OnuType;
}

/*DEFUN ( onu_cnc_vlan_auto_create,
         onu_cnc_vlan_auto_create_cmd,
         "interface qvlan auto ",
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Auto create vlan\n" )
{
	ULONG slot, pon, onu;
	int onu_type;
	int vid;

	onu_type = check_cnc_test_vlan_onu_type( vty, &slot, &pon, &onu );
	if( onu_type != VOS_ERROR )
	{
		for( vid = 2; vid<MAX_CNC_TEST_VLAN_NUM; vid++ )
		{
			onu_vlan_table[onu][vid].isValid = 1;
			if( onu_type == V2R1_ONU_GT813 )
			{
				if( vid & 1 )
					onu_vlan_table[onu][vid].tagPortlist = 0;
				onu_vlan_table[onu][vid].untagPortlist = 0;
			}
			else
			{
				onu_vlan_table[onu][vid].tagPortlist = 0;
				onu_vlan_table[onu][vid].untagPortlist = 0;
			}
		}
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}*/
DEFUN ( onu_cnc_vlan_create,
         onu_cnc_vlan_create_cmd,
         "interface qvlan <2-4094>",
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Specify vlan's ID\n" )
{
	ULONG slot, pon, onu;
	int onu_type;
	int vid;

	onu_type = check_cnc_test_vlan_onu_type( vty, &slot, &pon, &onu );
	if( onu_type != VOS_ERROR )
	{
		vid = VOS_AtoI( argv[0] );
		if( onu_vlan_table[onu][vid].isValid == 1 )
		{
			vty_out( vty, "Vlan %d already exist.\r\n", vid );
			return CMD_WARNING;
		}
		onu_vlan_table[onu][vid].isValid = 1;
		onu_vlan_table[onu][vid].tagPortlist = 0;
		onu_vlan_table[onu][vid].untagPortlist = 0;
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN ( onu_cnc_vlan_delete,
         onu_cnc_vlan_delete_cmd,
         "undo interface qvlan <2-4094>",
         NO_STR
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Specify vlan's ID\n" )
{
	ULONG slot, pon, onu;
	int onu_type;
	int vid;

	onu_type = check_cnc_test_vlan_onu_type( vty, &slot, &pon, &onu );
	if( onu_type != VOS_ERROR )
	{
		vid = VOS_AtoI( argv[0] );
		if( onu_vlan_table[onu][vid].isValid == 0 )
		{
			vty_out( vty, "Vlan %d does not exist.\r\n", vid );
			return CMD_WARNING;
		}
		onu_vlan_table[onu][vid].isValid = 0;
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN ( onu_cnc_vlan_port_add,
         onu_cnc_vlan_port_add_cmd,
         "interface qvlan_port <2-4094> <1-24> [1|2]",
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Specify vlan's ID\n"
	  "Please input the port number\n"
	  "Tagged\n"
	  "Untagged\n" )
{
	ULONG slot, pon, onu;
	int onu_type;
	int vid, port, type;

	onu_type = check_cnc_test_vlan_onu_type( vty, &slot, &pon, &onu );
	if( onu_type == VOS_ERROR )
		return CMD_WARNING;

	vid = VOS_AtoI( argv[0] );
	if( onu_vlan_table[onu][vid].isValid != 1 )
	{
		vty_out( vty, "Vlan %d does not exist.\r\n", vid );
		return CMD_WARNING;
	}
	port = VOS_AtoI( argv[1] );
	type = VOS_AtoI( argv[2] );

	switch( onu_type )
	{
		case V2R1_ONU_GT816:
		case V2R1_ONU_GT810:
			if( port > 1 )
			{
				vty_out( vty, "  Port number error.\r\n", vid );
				return CMD_WARNING;
			}
		case V2R1_ONU_GT813:
		case V2R1_ONU_GT866:
		case V2R1_ONU_GT865:
		case V2R1_ONU_GT813_B:
		case V2R1_ONU_GT862:
		case V2R1_ONU_GT863:
			port --;
			if( type == 1 )
			{
				onu_vlan_table[onu][vid].tagPortlist |= (1 << port); 
				onu_vlan_table[onu][vid].untagPortlist &= (~(1 << port));
			}
			else
			{
				onu_vlan_table[onu][vid].tagPortlist &= (~(1 << port));
				onu_vlan_table[onu][vid].untagPortlist |= (1 << port);
			}
			return CMD_SUCCESS;
		default:
			break;
	}
	return CMD_WARNING;
}

DEFUN ( onu_cnc_vlan_port_del,
         onu_cnc_vlan_port_del_cmd,
         "undo interface qvlan_port <2-4094> <1-24>",
         NO_STR
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Specify vlan's ID\n" 
	  "Port number\n" )
{
	ULONG slot, pon, onu;
	int onu_type;
	int vid, port;

	onu_type = check_cnc_test_vlan_onu_type( vty, &slot, &pon, &onu );
	if( onu_type == VOS_ERROR )
		return CMD_WARNING;

	vid = VOS_AtoI( argv[0] );
	if( onu_vlan_table[onu][vid].isValid != 1 )
	{
		vty_out( vty, "Vlan %d does not exist.\r\n", vid );
		return CMD_WARNING;
	}
	port = VOS_AtoI( argv[1] );

	switch( onu_type )
	{
		case V2R1_ONU_GT816:
		case V2R1_ONU_GT810:
			if( port > 1 )
			{
				vty_out( vty, "  Port number error.\r\n", vid );
				return CMD_WARNING;
			}
		case V2R1_ONU_GT813:
		case V2R1_ONU_GT866:
		case V2R1_ONU_GT865:
		case V2R1_ONU_GT813_B:
		case V2R1_ONU_GT862:
		case V2R1_ONU_GT863:
			port --;
			onu_vlan_table[onu][vid].tagPortlist &= (~(1 << port)); 
			onu_vlan_table[onu][vid].untagPortlist &= (~(1 << port));
			return CMD_SUCCESS;
		default:
			break;
	}
	return CMD_WARNING;
}

LDEFUN ( onu_cnc_vlan_create,
         onu_cnc_vlan_create_cmd,
         "interface qvlan <2-4094>",
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Specify vlan's ID\n" ,
         ONU_NODE
         )
{
	return CMD_SUCCESS;
}

LDEFUN ( onu_cnc_vlan_delete,
         onu_cnc_vlan_delete_cmd,
         "undo interface qvlan <2-4094>",
         NO_STR
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Specify vlan's ID\n",
         ONU_NODE )
{
	return CMD_SUCCESS;
}

LDEFUN ( onu_cnc_vlan_port_add,
         onu_cnc_vlan_port_add_cmd,
         "interface qvlan_port <2-4094> <1-24> [1|2]",
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Specify vlan's ID\n"
	  "Please input the port number\n"
	  "Tagged\n"
	  "Untagged\n" ,
         ONU_NODE)
{
	return CMD_SUCCESS;
}

LDEFUN ( onu_cnc_vlan_port_del,
         onu_cnc_vlan_port_del_cmd,
         "undo interface qvlan_port <2-4094> <1-24>",
         NO_STR
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Specify vlan's ID\n" 
	  "Port number\n" ,
         ONU_NODE)
{
	return CMD_SUCCESS;
}

#if 0
DEFUN ( onu_cnc_vlan_show,
         onu_cnc_vlan_show_cmd,
         "show interface qvlan {<1-4094>}*1",
         SHOW_STR
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Specify vlan's ID\n" )
{
	ULONG slot, pon, onu;
	int onu_type;
	ULONG port, line;
	int vid = 1, vlan_num = MAX_CNC_TEST_VLAN_NUM;

	if( argc == 1 )
	{
		vid = VOS_AtoI( argv[0] );
		vlan_num = vid;
	}

	onu_type = check_cnc_test_vlan_onu_type( vty, &slot, &pon, &onu );
	if( onu_type == VOS_ERROR )
	{
		vty_out( vty, "\r\n  check_cnc_test_vlan_onu_type fail." );
		return CMD_WARNING;
	}

	for( ; vid <= vlan_num; vid++ )
	{
		if( onu_vlan_table[onu][vid].isValid == 1 )
		{
			vty_out( vty, "\r\nInterface Vlan v%d is down.", vid );
			vty_out( vty, "\r\n  Physical status is down, administrator status is up." );
			vty_out( vty, "\r\n  MTU 1500 bytes." );
			vty_out( vty, "\r\n  IP binding disabled." );
			vty_out( vty, "\r\n  MulitiCast Flood Mode is 2." );
			vty_out( vty, "\r\n  Vlan id is %d.", vid );
			vty_out( vty, "\r\n  Port member list:\r\n" );
			line = 0;

			if( (onu_type == V2R1_ONU_GT816) || (onu_type == V2R1_ONU_GT810) )
			{
				if( onu_vlan_table[onu][vid].untagPortlist & 1 )
				{
					vty_out( vty, "    eth1/1(u)" );
				}
				else if( onu_vlan_table[onu][vid].tagPortlist & 1 )
				{
					vty_out( vty, "    eth1/1(t)" );
				}
			}
			else if( onu_type == V2R1_ONU_GT813 )
			{
				for( port = 0; port < 24; port++ )
				{
					if( onu_vlan_table[onu][vid].untagPortlist & (1<<port) )
					{
						if( port < 9 )
							vty_out( vty, "    eth1/%d(u)  ", port+1 );
						else
							vty_out( vty, "    eth1/%d(u) ", port+1 );
						line++;
					}
					else if( onu_vlan_table[onu][vid].tagPortlist & (1<<port) )
					{
						if( port < 9 )
							vty_out( vty, "    eth1/%d(t)  ", port+1 );
						else
							vty_out( vty, "    eth1/%d(t) ", port+1 );
						line++;
					}
					if( line >= 4 )
					{
						line = 0;
						vty_out( vty, "\r\n" );
					}
				}
			}
			if( line < 4 )
				vty_out( vty, "\r\n" );
		}
	}
	vty_out( vty, "\r\n" );
	return CMD_SUCCESS;
}
#endif

static void display_vlan( struct vty *vty, ULONG onu, int onu_type, int from_vid, int to_vid )
{
	ULONG port, count;
	int vid;
	for( vid = from_vid; vid <= to_vid; vid++ )
	{
		if( onu_vlan_table[onu][vid].isValid == 1 )
		{
			vty_out( vty, "\r\nInterface Vlan v%d is down.", vid );
			vty_out( vty, "\r\n  Physical status is down, administrator status is up." );
			vty_out( vty, "\r\n  MTU 1500 bytes." );
			vty_out( vty, "\r\n  IP binding disabled." );
			vty_out( vty, "\r\n  MulitiCast Flood Mode is 2." );
			vty_out( vty, "\r\n  Vlan id is %d.", vid );
			vty_out( vty, "\r\n  Port member list:\r\n" );
			count = 0;

			if( (onu_type == V2R1_ONU_GT816) || (onu_type == V2R1_ONU_GT810) )
			{
				if( onu_vlan_table[onu][vid].untagPortlist & 1 )
				{
					vty_out( vty, "    eth1/1(u)" );
				}
				else if( onu_vlan_table[onu][vid].tagPortlist & 1 )
				{
					vty_out( vty, "    eth1/1(t)" );
				}
			}
			else if( (onu_type == V2R1_ONU_GT813) || (onu_type == V2R1_ONU_GT866)  || (onu_type == V2R1_ONU_GT863))
			{
				for( port = 0; port < 24; port++ )
				{
					if( onu_vlan_table[onu][vid].untagPortlist & (1<<port) )
					{
						if( port < 9 )
							vty_out( vty, "    eth1/%d(u)  ", port+1 );
						else
							vty_out( vty, "    eth1/%d(u) ", port+1 );
						count++;
					}
					else if( onu_vlan_table[onu][vid].tagPortlist & (1<<port) )
					{
						if( port < 9 )
							vty_out( vty, "    eth1/%d(t)  ", port+1 );
						else
							vty_out( vty, "    eth1/%d(t) ", port+1 );
						count++;
					}
					if( count >= 4 )
					{
						count = 0;
						vty_out( vty, "\r\n" );
					}
				}
			}
			if( count < 4 )
				vty_out( vty, "\r\n" );
		}
	}
}
DEFUN ( onu_cnc_vlan_count_show,
         onu_cnc_vlan_count_show_cmd,
         "show interface qvlan {[count]}*1 {[vid] <1-4094>}*1",
         SHOW_STR
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Show interfaces vlan's count\n"
         "Show interfaces specified vlan\n"
         "Specify vlan's ID\n" )
{
	ULONG slot, pon, onu;
	int onu_type;
	int vid;
	ULONG count = 0;

	onu_type = check_cnc_test_vlan_onu_type( vty, &slot, &pon, &onu );
	if( onu_type == VOS_ERROR )
	{
		vty_out( vty, "  check_cnc_test_vlan_onu_type fail.\r\n" );
		return CMD_WARNING;
	}

	taskDelay( 80 );

	if( argc == 0  )	/* display all vlan config */
	{
		taskDelay( 80 );
		display_vlan( vty, onu, onu_type, 1, MAX_CNC_TEST_VLAN_NUM );
		vty_out( vty, "\r\n" );
	}
	else if( argc == 1 )	/* display all vlan count */
	{
		for( vid = 1; vid <= MAX_CNC_TEST_VLAN_NUM; vid++ )
		{
			if( onu_vlan_table[onu][vid].isValid == 1 )
				count++;
		}
		vty_out( vty, "\r\nTotal vlan num is:%d\r\n\r\n", count );
	}
	else if( argc == 2 )	/* display specified vlan config */
	{
		vid = VOS_AtoI( argv[1] );
		display_vlan( vty, onu, onu_type, vid, vid );
	}
	else if( argc == 3 )	/* display specified vlan count */
	{
		vid = VOS_AtoI( argv[2] );
		if( onu_vlan_table[onu][vid].isValid == 1 )
			count++;
		vty_out( vty, "\r\nTotal vlan num is:%d\r\n\r\n", count );
	}

	return CMD_SUCCESS;
}
LDEFUN ( onu_cnc_vlan_count_show,
         onu_cnc_vlan_count_show_cmd,
         "show interface qvlan {[count]}*1 {[vid] <1-4094>}*1",
         SHOW_STR
         "Select an interface to config\n"
         "Config vlan interface\n"
         "Show interfaces vlan's count\n"
         "Show interfaces specified vlan\n"
         "Specify vlan's ID\n",
         ONU_NODE)
{
	return CMD_SUCCESS;
}
#endif
/* end 2007-10-10 */

/*  2008-3-28
    按张新辉要求,在GT831节点下,增加如下命令*/
/*
ONU新支持的命令列表：
1. 显示ONU侧保存在内存、flash中的日志信息*/
DEFUN(onu_syslog_show,
	onu_syslog_show_cmd,
	"show syslog [flash|memory] {<0-3>}*1", 
	SHOW_STR 
	"Syslog\n"
	"Syslog saved in flash\n"
	"Syslog saved in memory\n"
	"Syslog block number (total syslog size is 128K, splitted into 4 block, 32K in each block)\n"
	)
{
	return onu_dev_cli_process( vty, "show syslog",NULL );
}

/*2、显示语音侧保存在内存、flash中的日志信息*/
DEFUN(config_cmd2lic_voice_syslog_show,
	config_cmd2lic_voice_syslog_show_cmd,
	"show voice syslog [flash|memory] {<0-3>}*1", 
	SHOW_STR 
	"Voice module information\n" 
	"Syslog\n"
	"Syslog saved in flash\n"
	"Syslog saved in memory\n"
	"Syslog block number (total syslog size is 128K, splitted into 4 block, 32K in each block)\n"
        )
{
    	return onu_dev_cli_process( vty, "show voice syslog",NULL );
}

/*3、显示语音侧的异常日志*/
DEFUN ( show_voice_exception_syslog_func,
        show_voice_exception_syslog_cmd,
        "show voice execpt-log",
        SHOW_STR
        "Voice module information\n"
        "Exception log file\n")
{
	return onu_dev_cli_process( vty, "show voice execpt-log" ,NULL);
}

/*4、显示ONU侧的异常日志*/
DEFUN ( show_exception_syslog_func,
        show_exception_syslog_cmd,
        "show execpt-log",
        SHOW_STR
        "Exception log file\n")
{
	return onu_dev_cli_process( vty, "show execpt-log",NULL );
}

/*5、使能SIP的PRACK功能*/
DEFUN(config_cmd2lic_sip_provisional_reliable,
	config_cmd2lic_sip_provisional_reliable_cmd,
	"sip prack {[enable|disable]}*1",
	"SIP configuration.\n"
	"PRACK support.\n"
	"Enable.\n"
	"Disable.\n"
    	)
{
	return onu_dev_cli_process( vty, "sip prack",NULL );
}

/*6、指定SIP的补充业务是在本地还是软交换上实现*/
DEFUN(config_cmd2lic_sip_supplementary_service,
	config_cmd2lic_sip_supplementary_service_cmd,
	"sip supplement_service {[local|soft-switch]}*1",
	"SIP configuration.\n"
	"Supplement service support.\n"
	"Enable at local.\n"
	"Enable on softswitch.\n"
    	)
{
	return onu_dev_cli_process( vty, "sip supplement_service",NULL );
}

/*7、配置SIP的每次REGISTER是否改变CID*/
DEFUN(config_cmd2lic_sip_registar_cid_change,
	config_cmd2lic_sip_registar_cid_change_cmd,
	"sip registar_cid {[const|change]}*1",
	"SIP configuration.\n"
	"Registart CID policy.\n"
	"Remain the same as long as the iad running.\n"
	"Change in each re-registion.\n"
    	)
{
	return onu_dev_cli_process( vty, "sip registar_cid",NULL );
}

/*8、显示语音呼叫统计*/
DEFUN(config_cmd2lic_voice_stat_show,
	config_cmd2lic_voice_stat_show_cmd,
	"show voice call-statistics", 
	SHOW_STR 
	"Voice module information\n" 
	"Call statistics\n"
     )
{
	return onu_dev_cli_process( vty, "show voice call-statistics",NULL );
}
/* 9. 清除事件*/
DEFUN(onu_event_clear,
	onu_event_clear_cmd,
	"event clear", 
	"ONU event config\n" 
	"Clear onu events\n")
{
	return onu_dev_cli_process( vty, "event clear" ,NULL);
}

#if 1
/*add by luh 2011-12*/
DEFUN  (
    onu_bat_port_vlan_dot1q_add,
    onu_bat_port_vlan_dot1q_add_cmd,
    "vlan dot1q_add onu_range <slot/port/onuid> <slot/port/onuid> <2-4094> [1|2] {<port_list>}*1",
    "config onu vlan information\n"
    "add 802.1Q Vlan\n"
    "config for onu range\n"
    "Please input the begin onu id\n"
    "Please input end onu id\n"
    "Please input the Vlan Vid(2-4094)\n"
    "Tagged\n"
    "Untagged\n"
    "Please input the port number\n"    
    )
{
    ULONG   begin_slot, begin_port, begin_onuid;
    ULONG   end_slot, end_port, end_onuid;
    ULONG   vid, begin_onuIdx, end_onuIdx;
    ULONG   suffix, portlist = 0, port = 0;
    int     onu_Num;
    UCHAR   mode;
    LONG    lRet = 0;
    if(IsProfileNodeVty(vty->index, &suffix))
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
    	lRet = PON_ParseSlotPortOnu( argv[0], &begin_slot, &begin_port, &begin_onuid );
    	if( lRet != VOS_OK )
    		return CMD_WARNING;
    	if( PonCardSlotPortCheckWhenRunningByVty(begin_slot, begin_port, vty) != ROK )
    		return(CMD_WARNING );
    	if ((begin_onuid<1) || (begin_onuid>MAXONUPERPON))
    	{
    		vty_out( vty, "  %% onu %d/%d/%d error\r\n",begin_slot, begin_port, begin_onuid );
    		return CMD_WARNING;	
    	}
        
    	lRet = PON_ParseSlotPortOnu( argv[1], &end_slot, &end_port, &end_onuid );
    	if( lRet != VOS_OK )
    		return CMD_WARNING;
    	if( PonCardSlotPortCheckWhenRunningByVty(end_slot, end_port, vty) != ROK )
    		return(CMD_WARNING );
    	if ((begin_onuid<1) || (begin_onuid>MAXONUPERPON))
    	{
    		vty_out( vty, "  %% onu %d/%d/%d error\r\n",end_slot, end_port, end_onuid );
    		return CMD_WARNING;	
    	}
        if(argc == 5)
        {
    		VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[4], port)        
            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[4], port )
            {
                portlist |= 1<<(port-1);
            }
            END_PARSE_PORT_LIST_TO_PORT();
        }
        
        vid = VOS_StrToUL(argv[2], NULL, 10);
        mode = VOS_StrToUL(argv[3], NULL, 10);
        onu_Num = Get_OnuNumFromOnuRangeByVty(vty, begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid);
        begin_onuIdx = MAKEDEVID(begin_slot, begin_port,begin_onuid); 
        end_onuIdx = MAKEDEVID(end_slot, end_port, end_onuid); 
        if(onu_Num == VOS_ERROR)
            return CMD_WARNING;
        
        Set_OnuProfile_VlanBat_Mode_Vid(vty, suffix, pd, begin_onuIdx, end_onuIdx, vid, portlist, mode);
    }
    return CMD_SUCCESS;
}
DEFUN  (
    onu_bat_vlan_dot1q_step_add,
    onu_bat_vlan_dot1q_step_add_cmd,
    "vlan dot1q_add onu_range <slot/port/onuid> <slot/port/onuid> vlan-range <2-4094> <2-4094> <step> [1|2] {port <step>}*1",
    "config onu vlan information\n"
    "add 802.1Q Vlan\n"
    "config for onu range\n"
    "Please input the begin onu id\n"
    "Please input end onu id\n"
    "config vlan range\n"
    "Please input the Begin Vlan Vid(2-4094)\n"
    "Please input the End Vlan Vid(2-4094)\n"
    "config vlan step between onu\n"
    "Tagged\n"
    "Untagged\n"
    "base on onu port\n"
    "config vlan step between onu port\n"
    )
{
    ULONG   begin_vid, end_vid, onu_step = 0, port_step = 0, mode;
    ULONG   begin_slot, begin_port, begin_onuid;
    ULONG   end_slot, end_port, end_onuid;
    ULONG   begin_onuIdx, end_onuIdx;
    int     onu_Num = 0;
    ULONG   suffix;
    LONG    lRet = 0;
    if(IsProfileNodeVty(vty->index, &suffix))
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

    	lRet = PON_ParseSlotPortOnu( argv[0], &begin_slot, &begin_port, &begin_onuid );
    	if( lRet != VOS_OK )
    		return CMD_WARNING;
    	if( PonCardSlotPortCheckWhenRunningByVty(begin_slot, begin_port, vty) != ROK )
    		return(CMD_WARNING );
    	if ((begin_onuid<1) || (begin_onuid>MAXONUPERPON))
    	{
    		vty_out( vty, "  %% onu %d/%d/%d error\r\n",begin_slot, begin_port, begin_onuid );
    		return CMD_WARNING;	
    	}
        
    	lRet = PON_ParseSlotPortOnu( argv[1], &end_slot, &end_port, &end_onuid );
    	if( lRet != VOS_OK )
    		return CMD_WARNING;
    	if( PonCardSlotPortCheckWhenRunningByVty(end_slot, end_port, vty) != ROK )
    		return(CMD_WARNING );
    	if ((begin_onuid<1) || (begin_onuid>MAXONUPERPON))
    	{
    		vty_out( vty, "  %% onu %d/%d/%d error\r\n",end_slot, end_port, end_onuid );
    		return CMD_WARNING;	
    	}
        
        begin_vid = VOS_StrToUL(argv[2], NULL, 10);
        end_vid = VOS_StrToUL(argv[3], NULL, 10);
        onu_step = VOS_StrToUL(argv[4], NULL, 10);
        mode = VOS_StrToUL(argv[5], NULL, 10);
        onu_Num = Get_OnuNumFromOnuRangeByVty(vty, begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid);
        if(onu_Num == VOS_ERROR)
            return CMD_WARNING;
        
        if(argc == 7)
        {
            if(onu_step < 1)
            {
                vty_out(vty,"step must be more then 1 or equal 1.\r\n");
                return CMD_WARNING;
            }
            port_step = VOS_StrToUL(argv[6], NULL, 10);
            if(onu_Num*onu_step > (end_vid-begin_vid+1))
            {
                vty_out(vty,"vlan id is not enough for the selected onu.\r\n");
                return CMD_WARNING;
            }
        }
        else
        {
            if(onu_step < 1)
            {
                vty_out(vty,"step must be more then 1 or equal 1.\r\n");
                return CMD_WARNING;
            }
            if((onu_Num-1)*onu_step > (end_vid-begin_vid))
            {
                vty_out(vty,"vlan id is not enough for the selected onu.\r\n");
                return CMD_WARNING;
            }
        }
        begin_onuIdx = MAKEDEVID(begin_slot, begin_port,begin_onuid); 
        end_onuIdx = MAKEDEVID(end_slot, end_port, end_onuid); 

        Set_OnuProfile_VlanBat_Mode_Range(vty, suffix, pd, begin_onuIdx, end_onuIdx, begin_vid, end_vid, onu_step, mode, port_step);
    }
    return CMD_SUCCESS;
}
DEFUN  (
    onu_bat_vlan_dot1q_step_portid_add,
    onu_bat_vlan_dot1q_step_portid_add_cmd,
    "vlan dot1q_add onu_range <slot/port/onuid> <slot/port/onuid> vlan-range <2-4094> <2-4094> <step> [1|2] {port_id <port>}*1",
    "config onu vlan information\n"
    "add 802.1Q Vlan\n"
    "config for onu range\n"
    "Please input the begin onu id\n"
    "Please input end onu id\n"
    "config vlan range\n"
    "Please input the Begin Vlan Vid(2-4094)\n"
    "Please input the End Vlan Vid(2-4094)\n"
    "config vlan step between onu\n"
    "Tagged\n"
    "Untagged\n"
    "base on the onu port\n"
    "Please input the onu port\n"
    )
{
    ULONG   begin_vid, end_vid, onu_step = 0, port_id = 0, mode;
    ULONG   begin_slot, begin_port, begin_onuid;
    ULONG   end_slot, end_port, end_onuid;
    ULONG   begin_onuIdx, end_onuIdx;
    int     onu_Num = 0;
    ULONG   suffix;
    LONG    lRet = 0;
    if(IsProfileNodeVty(vty->index, &suffix))
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;

    	lRet = PON_ParseSlotPortOnu( argv[0], &begin_slot, &begin_port, &begin_onuid );
    	if( lRet != VOS_OK )
    		return CMD_WARNING;
    	if( PonCardSlotPortCheckWhenRunningByVty(begin_slot, begin_port, vty) != ROK )
    		return(CMD_WARNING );
    	if ((begin_onuid<1) || (begin_onuid>MAXONUPERPON))
    	{
    		vty_out( vty, "  %% onu %d/%d/%d error\r\n",begin_slot, begin_port, begin_onuid );
    		return CMD_WARNING;	
    	}
        
    	lRet = PON_ParseSlotPortOnu( argv[1], &end_slot, &end_port, &end_onuid );
    	if( lRet != VOS_OK )
    		return CMD_WARNING;
    	if( PonCardSlotPortCheckWhenRunningByVty(end_slot, end_port, vty) != ROK )
    		return(CMD_WARNING );
    	if ((end_onuid<1) || (end_onuid>MAXONUPERPON))
    	{
    		vty_out( vty, "  %% onu %d/%d/%d error\r\n",end_slot, end_port, end_onuid );
    		return CMD_WARNING;	
    	}
        
        begin_vid = VOS_StrToUL(argv[2], NULL, 10);
        end_vid = VOS_StrToUL(argv[3], NULL, 10);
        onu_step = VOS_StrToUL(argv[4], NULL, 10);
        mode = VOS_StrToUL(argv[5], NULL, 10);
        onu_Num = Get_OnuNumFromOnuRangeByVty(vty, begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid);
        if(onu_Num == VOS_ERROR)
            return CMD_WARNING;
        
        if(argc == 7)
        {
            if(onu_step < 1)
            {
                vty_out(vty,"step must be more then 1 or equal 1.\r\n");
                return CMD_WARNING;
            }
            port_id = VOS_StrToUL(argv[6], NULL, 10);
            if(onu_Num*onu_step > (end_vid-begin_vid+1))
            {
                vty_out(vty,"vlan id is not enough for the selected onu.\r\n");
                return CMD_WARNING;
            }
        }
        else
        {
            if(onu_step < 1)
            {
                vty_out(vty,"step must be more then 1 or equal 1.\r\n");
                return CMD_WARNING;
            }
            if((onu_Num-1)*onu_step > (end_vid-begin_vid))
            {
                vty_out(vty,"vlan id is not enough for the selected onu.\r\n");
                return CMD_WARNING;
            }
        }
		
        begin_onuIdx = MAKEDEVID(begin_slot, begin_port,begin_onuid); 
        end_onuIdx = MAKEDEVID(end_slot, end_port, end_onuid); 

        Set_OnuProfile_VlanBat_Mode_RangeByPortId(vty, suffix, pd, begin_onuIdx, end_onuIdx, begin_vid, end_vid, onu_step, mode, port_id);
    }
    return CMD_SUCCESS;
}
DEFUN  (
    onu_bat_vlan_dot1q_rules_show,
    onu_bat_vlan_dot1q_rules_show_cmd,
    "vlan dot1q_show rules",
    "config onu vlan information\n"
    "show 802.1Q Vlan\n"
    "show vlan dot1q bat rules\n"
    )
{
    ULONG   suffix;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;

	ulIfIndex =(ULONG) vty->index;
    if(IsProfileNodeVty(ulIfIndex, &suffix))
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        if(Show_OnuProfile_VlanBat_RuleListByPtr(vty, suffix, pd) != VOS_OK)
            vty_out(vty, "  %%executed command error!\r\n");
    }
    else
    {
    	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
    	if( ulRet !=VOS_OK )
    		return CMD_WARNING;
        
        if(Show_OnuProfile_VlanBat_RuleList(vty, ulSlot, ulPort, ulOnuid) != VOS_OK)
            vty_out(vty, "  %%executed command error!\r\n");
    }
    return CMD_SUCCESS;
}

DEFUN  (
    onu_bat_vlan_dot1q_rules_delete,
    onu_bat_vlan_dot1q_rules_delete_cmd,
    "vlan dot1q_del rules {<1-16>}*1",
    "config onu vlan information\n"
    "delete 802.1Q Vlan rules\n"
    "delete 802.1Q Vlan rules\n"    
    "Please input rule number\n"
    )
{
    ULONG   suffix;
    ULONG   num = 0;
    if(argc == 1)
        num = VOS_StrToUL(argv[0], NULL, 10);
    
    if(IsProfileNodeVty(vty->index, &suffix))
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        Delete_onuProfile_VlanBat_Rule(vty, suffix, pd, num);
    }
    return CMD_SUCCESS;
}
DEFUN  (
    onu_vlan_transparent_port_enable,
    onu_vlan_transparent_port_enable_cmd,
    "vlan-transparent <port_list> {[1|0]}*1",
    "Show or config onu vlan-transparent information\n"
    "Please input the port number\n"
    "Enable\n"
    "Disable\n"
    )
{
	int ponID=0, mode = ONU_SEPCAL_FUNCTION;
    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet,suffix;
    ULONG i = 0, entryNum=0;
    ulIfIndex =(ULONG) vty->index;

    if(!IsProfileNodeVty(ulIfIndex, &suffix))
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }
        ONU_CONF_WRITEABLE_CHECK
            
        if(isCtcOnu(ulSlot, ulPort, ulOnuid))
        {
            VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)            
            if(argc == 2)
            {    
                mode |= VOS_StrToUL(argv[1], NULL, 10)?ONU_CONF_VLAN_MODE_TRANSPARENT:ONU_CONF_VLAN_MODE_TRUNK;
                
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                {

                    if( OnuMgt_SetVlanMode(ponID, ulOnuid-1, port, mode) != VOS_OK)
                    {
#if 0                        
                        vty_out(vty, "set port %d vlan-transparent fail!\r\n", port);
#else
                        error_flag = 1;
#endif
                    }
                }
                END_PARSE_PORT_LIST_TO_PORT();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);
                
            }
            else
            {
                CTC_STACK_port_vlan_configuration_t Vconfig;
                BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                if(OnuMgt_GetEthPortVlanConfig(ponID, ulOnuid - 1, port, &Vconfig) == VOS_OK)
                {
                    vty_out(vty, "port %d vlan-transparent is %s\r\n", port, Vconfig.mode == ONU_CONF_VLAN_MODE_TRANSPARENT?"enable":"disable");
                }
                else
                    error_flag = 1;
                END_PARSE_PORT_LIST_TO_PORT();
                if(error_flag)
                    vty_out(vty, ONU_CMD_ERROR_STR);
                
            }
        }
    }
    else
    {
        ULONG enable = 0;
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)
        if(argc == 2)
        {
            enable = VOS_StrToUL(argv[1], NULL, 10);            
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(setOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_onu_transparent_enable, enable) != VOS_OK)
                    vty_out(vty, "set port %d vlan-transparent fail!\r\n", port); 
                else if(enable)
                    OnuConfDelVlanPortByPtr(suffix, pd, port);
            }
            END_PARSE_PORT_LIST_TO_PORT(); 
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
                if(getOnuConfPortSimpleVarByPtr(suffix, pd, port, sv_enum_onu_transparent_enable, &enable) != VOS_OK)
                    vty_out(vty, "get port %d vlan-transparent fail!\r\n", port); 
                else
                    vty_out(vty, "port %d vlan-transparent is %s\r\n", port, enable?"enable":"disable");
            }
            END_PARSE_PORT_LIST_TO_PORT(); 
        }
    }
    return CMD_SUCCESS;
}
/* add for GPON by yangzl@2016-4-29:begin*/
DEFUN  (
    gpononu_igmp_set_template,
    gpononu_igmp_set_template_cmd,
    "igmp-snooping add [static-profile|dynamic-profile] <1-128> <1-128> <2337-4094> <1-4096> <A.B.C.D> <A.B.C.D> ",
    "igmpsnooping add  profile\n"
    "igmpsnooping add  profile\n"
    "igmpsnooping add  static profile\n"
    "igmpsnooping add  dynamic profile\n"
    "Please enter the primary index \n"
    "Please enter the secondary indexes\n"
    "multicast Port number\n"
    "multicast VLAN ID\n"
   /* "Source IPAddress\n"*/
    "multicast Group Address Start\n"
    "multicast Group Address Stop\n"
   /* "imputed Group Bandwidth\n"*/
    )
{
       int prof1 = 0,prof2 = 0;
	unsigned int sourceIPAddress;
	unsigned int multicastGroupAddressStart;
	unsigned int multicastGroupAddressStop;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	unsigned char stateflag = 0;
	CTC_GPONADP_ONU_Multicast_Prof_t cfg;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;

    if ( 0 == VOS_StrnCmp( "static", argv[ 0 ],6) )
        stateflag  = 0;
    else 
	{
		stateflag  = 1;
    	}
	
	prof1 = VOS_StrToUL(argv[1], NULL, 10);
	prof2 = VOS_StrToUL(argv[2], NULL, 10);
	cfg.multicastGemPortNo = VOS_StrToUL(argv[3], NULL, 10);
	cfg.multicastVLANID = VOS_StrToUL(argv[4], NULL, 10);
	multicastGroupAddressStart = get_long_from_ipdotstring( argv[5] );
	cfg.multicastGroupAddressStart = multicastGroupAddressStart;
	multicastGroupAddressStop = get_long_from_ipdotstring( argv[6] );
	cfg.multicastGroupAddressStop = multicastGroupAddressStop;
	cfg.sourceIPAddress = 0;  
	cfg.imputedGroupBandwidth= 0;

     if(VOS_OK != OnuMgt_SetMulticastTemplate(PonPortIdx,OnuIdx,prof1, prof2, &cfg,stateflag))
	      vty_out(vty, "%%executed command error!\r\n");         
    return CMD_SUCCESS;
}
DEFUN  (
    gpononu_igmp_get_template,
    gpononu_igmp_get_template_cmd,
    "show igmp-snooping [static-profile|dynamic-profile] <1-128> <1-128> ",
    "show igmpsnooping profile\n"
    "show igmpsnooping profile\n"
    "show igmpsnooping static profile\n"
    "show igmpsnooping dynamic profile\n"
    "Please enter the primary index \n"
    "Please enter the secondary indexes\n"
    )
{
       int prof1 = 0,prof2 = 0;
	unsigned int multicastGroupAddressStart;
	unsigned int multicastGroupAddressStop;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	unsigned char stateflag = 0;
	unsigned char data[4] = {0};
	unsigned char uconut = 0;
	CTC_GPONADP_ONU_Multicast_Prof_t cfg;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;

    if ( 0 == VOS_StrnCmp( "static", argv[ 0 ],6) )
        stateflag  = 0;
    else 
	{
		stateflag  = 1;
    	}
	
	prof1 = VOS_StrToUL(argv[1], NULL, 10);
	prof2 = VOS_StrToUL(argv[2], NULL, 10);
       if(VOS_OK != OnuMgt_GetMulticastTemplate(PonPortIdx,OnuIdx,prof1, prof2, &cfg,stateflag))
		      vty_out(vty, "%%executed command error!\r\n");   
	  else
	  {
	  	multicastGroupAddressStart = cfg.multicastGroupAddressStart;
		multicastGroupAddressStop = cfg.multicastGroupAddressStop;
	  	if(0 == stateflag)
			vty_out(vty, "\r  Static Profile %03d-%03d \r\n", prof1,prof2);   
		else
			vty_out(vty, "\r   Dynamic Profile %03d-%03d \r\n", prof1,prof2);   

		vty_out(vty, "=================================\r\n"); 
		vty_out(vty, "multicast Port number : %d\r\n",cfg.multicastGemPortNo ); 
		vty_out(vty, "multicast VLAN ID : %d\r\n",cfg.multicastVLANID ); 
			for(uconut = 0;uconut < 4;uconut ++)
			{
				data[uconut] = multicastGroupAddressStart & 0xff;
				multicastGroupAddressStart >>= 8;
			}
		vty_out(vty, "multicast Group Address Start : %d.%d.%d.%d\r\n",data[3],data[2],data[1],data[0] ); 
		for(uconut = 0;uconut < 4;uconut ++)
			{
				data[uconut] = multicastGroupAddressStop & 0xff;
				multicastGroupAddressStop >>= 8;
			}
		vty_out(vty, "multicast Group Address Stop : %d.%d.%d.%d\r\n",data[3],data[2],data[1],data[0] ); 
          }
	   	
    return CMD_SUCCESS;
}

DEFUN  (
    gpononu_igmp_set_operating_profile_first,
    gpononu_igmp_set_operating_profile_first_cmd,
    "igmp-snooping add operating-profile <1-128> version <1-3> controlMode <0-2> fastLeaveMode [0|1]",
    "igmpsnooping add operating-profile\n"
    "igmpsnooping add operating-profile\n"
    "igmpsnooping add operating-profile\n"
    "please enter ID of porfile\n"
    "igmp version\n"
    "igmp version\n"
    "igmp control mode\n"
    "0:igmp transparent 1:snooping with proxy reporting 2:IGMP proxy\n"
    "igmp fastLeaveMode\n"
    "Disable fastLeaveMode(The default configuration)\n"
    "Enable fastLeaveMode\n"
    )
{
       int prof = 0;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;	
	unsigned int igmpVersion;
	unsigned int multicastControlMode;
	unsigned int fastLeaveMode;
	CTC_GPONADP_ONU_McastOper_Prof_t cfg;
	VOS_MemSet( &cfg,0,sizeof(CTC_GPONADP_ONU_McastOper_Prof_t));

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;

	prof = VOS_StrToUL(argv[0], NULL, 10);
	igmpVersion = VOS_StrToUL(argv[1], NULL, 10);
	cfg.bitMask |= GPONADP_ONU_SERV_PROTO_PROF_MASK_IGMP_VER; 
	cfg.igmpVersion = igmpVersion;
	multicastControlMode = VOS_StrToUL(argv[2], NULL, 10);
	cfg.bitMask |= GPONADP_ONU_SERV_PROTO_PROF_MASK_MULT_CONTROL_MODE; 
	cfg.multicastControlMode = multicastControlMode;
	fastLeaveMode = VOS_StrToUL(argv[3], NULL, 10);
	cfg.bitMask |= GPONADP_ONU_SERV_PROTO_PROF_MASK_FAST_LEAVE_MODE;
	cfg.fastLeaveMode = fastLeaveMode;
	
       if(VOS_OK != OnuMgt_SetMcastOperProfile(PonPortIdx,OnuIdx,prof, &cfg))
		      vty_out(vty, "%%executed command error!\r\n");    

    return CMD_SUCCESS;
}
DEFUN  (
    gpononu_igmp_set_operating_profile_second,
    gpononu_igmp_set_operating_profile_second_cmd,
    "igmp-snooping add operating-profile <1-128> upstreamIGMPTCI <number> upstreamIGMPTagControl <0-3>",
    "igmpsnooping add operating-profile\n"
    "igmpsnooping add operating-profile\n"
    "igmpsnooping add operating-profile\n"
    "please enter ID of porfile\n"
    "set upstreamIGMPTCI mode\n"
    "set upstreamIGMPTCI mode\n"
    "set upstreamIGMPTagControl\n"
   "0:transparent. 1:add tag, tag specified by upstreamIGMPTCI. 2:replace outer tag, include vid and pbit. 3:replace outer tag, only vid. \n"
    )
{
       int prof = 0;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;	
	unsigned int upstreamIGMPTCI;
	unsigned int upstreamIGMPTagControl;
	CTC_GPONADP_ONU_McastOper_Prof_t cfg;
	VOS_MemSet( &cfg,0,sizeof(CTC_GPONADP_ONU_McastOper_Prof_t));

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;

	prof = VOS_StrToUL(argv[0], NULL, 10);
	
	upstreamIGMPTCI = VOS_StrToUL(argv[1], NULL, 10);
	cfg.bitMask |= GPONADP_ONU_SERV_PROTO_PROF_MASK_UPSTREAM_IGMP_TCI; 
	cfg.upstreamIGMPTCI = upstreamIGMPTCI;
	upstreamIGMPTagControl = VOS_StrToUL(argv[2], NULL, 10);
	cfg.bitMask |= GPONADP_ONU_SERV_PROTO_PROF_MASK_UPSTREAM_IGMP_TAG_CONTROL; 
	cfg.upstreamIGMPTagControl = upstreamIGMPTagControl;
	
       if(VOS_OK != OnuMgt_SetMcastOperProfile(PonPortIdx,OnuIdx,prof, &cfg))
		      vty_out(vty, "%%executed command error!\r\n");    

    return CMD_SUCCESS;
}

DEFUN  (
    gpononu_igmp_set_operating_profile_third,
    gpononu_igmp_set_operating_profile_third_cmd,
    "igmp-snooping add operating-profile <1-128> oltTagStrip [0|1] downstreamIGMPTCI <number> downstreamIGMPTagControl <0-4>",
    "igmpsnooping add operating-profile\n"
    "igmpsnooping add operating-profile\n"
    "igmpsnooping add operating-profile\n"
    "please enter ID of porfile\n"
    "set oltTagStrip\n"
    "0: transparent in OLT\n"
    "1: olt strip outer tag\n"
    "set downstreamIGMPTCI mode\n"
    "set downstreamIGMPTCI mode\n"
    "set downstreamIGMPTagControl\n"
   "0:transparent. 1: strip outer tag. 2:add tag, tag specified by downstreamIgmpMcastTCI. 3:replace outer tag, include vid and pbit. 4:replace outer tag, only vid. \n"
    )
{
       int prof = 0;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;	
	unsigned char oltTagStrip;
	unsigned short downstreamIgmpMcastTCI;
	unsigned char downstreamIgmpMcastTagControl;

	CTC_GPONADP_ONU_McastOper_Prof_t cfg;
	VOS_MemSet( &cfg,0,sizeof(CTC_GPONADP_ONU_McastOper_Prof_t));

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;

	prof = VOS_StrToUL(argv[0], NULL, 10);
	
	oltTagStrip = VOS_StrToUL(argv[1], NULL, 10);
	cfg.bitMask |= GPONADP_ONU_SERV_PROTO_PROF_MASK_OLT_TAG_STRIP; 
	cfg.oltTagStrip = oltTagStrip;
	downstreamIgmpMcastTCI = VOS_StrToUL(argv[2], NULL, 10);
	cfg.bitMask |= GPONADP_ONU_SERV_PROTO_PROF_MASK_DS_IGMP_MCAST_TCI; 
	cfg.downstreamIgmpMcastTCI= downstreamIgmpMcastTCI;
	downstreamIgmpMcastTagControl = VOS_StrToUL(argv[3], NULL, 10);
	cfg.bitMask |= GPONADP_ONU_SERV_PROTO_PROF_MASK_DS_IGMP_MCAST_TAG_CONTROL;
	cfg.downstreamIgmpMcastTagControl = downstreamIgmpMcastTagControl;
	
       if(VOS_OK != OnuMgt_SetMcastOperProfile(PonPortIdx,OnuIdx,prof, &cfg))
		      vty_out(vty, "%%executed command error!\r\n");    

    return CMD_SUCCESS;
}

DEFUN  (
    gpononu_igmp_get_operating_profile,
    gpononu_igmp_get_operating_profile_cmd,
    "show igmp-snooping operating-profile <1-128>",
    "show igmpsnooping operating-profile\n"
    "show igmpsnooping operating-profile\n"
    "show igmpsnooping operating-profile\n"
    "Please enter the primary index \n"
    )
{
       int prof = 0;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	CTC_GPONADP_ONU_McastOper_Prof_t cfg;
	VOS_MemSet( &cfg,0,sizeof(CTC_GPONADP_ONU_McastOper_Prof_t));

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;

	prof = VOS_StrToUL(argv[0], NULL, 10);
	
       if(VOS_OK != OnuMgt_GetMcastOperProfile(PonPortIdx,OnuIdx,prof, &cfg))
		      vty_out(vty, "%%executed command error!\r\n");  
	else
		{
			vty_out(vty, "\r  Operating Profile %03d\r\n", prof);   
			vty_out(vty, "==============================\r\n"); 
			vty_out(vty, "Igmp version: v%d\r\n", cfg.igmpVersion); 
			switch(cfg.multicastControlMode)
				{
					case 0:vty_out(vty, "Multicast Control Mode:IGMP transparent\r\n"); break;
					case 1:vty_out(vty, "Multicast Control Mode:snooping with proxy reporting\r\n"); break;
					case 2:vty_out(vty, "Multicast Control Mode:IGMP proxy\r\n"); break;
					default:vty_out(vty, "Multicast Control Mode:unknown\r\n"); break;
				}
			if (cfg.fastLeaveMode == 0)
				vty_out(vty, "Fast Leave Mode:disable\r\n"); 
			else
				vty_out(vty, "Fast Leave Mode:enable\r\n"); 
			
			vty_out(vty, "UpstreamIGMPTCI: %d\r\n", cfg.upstreamIGMPTCI); 
			switch(cfg.upstreamIGMPTagControl)
				{
					case 0:vty_out(vty, "UpstreamIGMPTagControl:transparent(default)\r\n"); break;
					case 1:vty_out(vty, "UpstreamIGMPTagControl:add tag, tag specified by upstreamIGMPTCI\r\n"); break;
					case 2:vty_out(vty, "UpstreamIGMPTagControl:replace outer tag, include vid and pbit\r\n"); break;
					case 3:vty_out(vty, "UpstreamIGMPTagControl:replace outer tag, only vid\r\n"); break;
					default:vty_out(vty, "UpstreamIGMPTagControl:unknown\r\n"); break;
				}
			if (cfg.oltTagStrip == 0)
				vty_out(vty, "OltTagStrip:transparent in OLT\r\n"); 
			else
				vty_out(vty, "OltTagStrip:olt strip outer tag\r\n" ); 
			vty_out(vty, "DownstreamIgmpMcastTCI: %d\r\n", cfg.downstreamIgmpMcastTCI); 
			switch(cfg.downstreamIgmpMcastTagControl)
				{
					case 0:vty_out(vty, "DownstreamIGMPTagControl:transparent(default)\r\n"); break;
					case 1:vty_out(vty, "DownstreamIGMPTagControl:strip outer tag\r\n"); break;
					case 2:vty_out(vty, "DownstreamIGMPTagControl:add tag, tag specified by downstreamIGMPTCI\r\n"); break;
					case 3:vty_out(vty, "DownstreamIGMPTagControl:replace outer tag, include vid and pbit\r\n"); break;
					case 4:vty_out(vty, "DownstreamIGMPTagControl:replace outer tag, only vid\r\n"); break;
					default:vty_out(vty, "DownstreamIGMPTagControl:unknown\r\n"); break;
				}
		}

    return CMD_SUCCESS;
}

DEFUN  (
    gpononu_igmp_set_uniportassociate,
    gpononu_igmp_set_uniportassociate_cmd,
    "igmpsnooping set port <1-24> associate [static-profile|dynamic-profile|operating-profile] <1-128> ",
    "igmpsnooping set  uniport associate with profile\n"
    "igmpsnooping set  uniport associate with profile\n"
    "igmpsnooping set  uniport associate with profile\n"
    "Please enter the uniport number \n"
    "igmpsnooping set  uniport associate with profile\n"
    "igmpsnooping set  uniport associate with static-profile\n"
    "igmpsnooping set  uniport associate with dynamic-profile\n"
    "igmpsnooping set  uniport associate with operating-profile\n"
    "Please enter the profile index \n"
    )
{
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	unsigned char stateflag = 0;
	short int portid = 0;
	int  profIdx = 0;
	CTC_GPONADP_ONU_McastOper_Prof_t cfg;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;

	portid = VOS_StrToUL(argv[0], NULL, 10);
	if ( 0 == VOS_StrnCmp( "static", argv[ 1 ],6) )
             stateflag  = 0;
       else if(0 == VOS_StrnCmp( "dynamic", argv[ 1 ],7))
	{
		stateflag  = 1;
    	}
	else if(0 == VOS_StrnCmp( "operating", argv[ 1 ],9))
	{
		stateflag  = 2;
	}
	else
		stateflag  = 3;
	profIdx = VOS_StrToUL(argv[2], NULL, 10);

       if(VOS_OK != OnuMgt_SetUniPortAssociateMcastProf(PonPortIdx,OnuIdx,portid ,stateflag, profIdx))
		      vty_out(vty, "%%executed command error!\r\n");   

    return CMD_SUCCESS;
}
DEFUN  (
    gpononu_igmp_get_uniportassociate,
    gpononu_igmp_get_uniportassociate_cmd,
    "show igmpsnooping associate-profile port <1-24>",
    "show igmpsnooping associate-profile\n"
    "show igmpsnooping associate-profile\n"
    "show igmpsnooping associate-profile\n"
    "show igmpsnooping associate-profile \n"
    "please enter port number\n"
    )
{
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
	short int portid = 0;
	CTC_GPONADP_ONU_Profile_t ProfIdx;
	
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (VOS_ERROR);

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;

	portid = VOS_StrToUL(argv[0], NULL, 10);

       if(VOS_OK != OnuMgt_GetUniPortAssociateMcastProf(PonPortIdx,OnuIdx,portid ,&ProfIdx))
		      vty_out(vty, "%%executed command error!\r\n");  
	 else
	 	{
	 		vty_out(vty, "Port %d associate with :\r\n",portid);  
			vty_out(vty, "StaticProfile:%d\r\n",ProfIdx.staticProfIdx);  
			vty_out(vty, "DynamicProfile:%d\r\n",ProfIdx.dynamicProfIdx);  
			vty_out(vty, "OperatingProfile:%d\r\n",ProfIdx.operProfIdx);  
	 	}

    return CMD_SUCCESS;
}
/* add for GPON by yangzl@2016-4-29:end*/

#endif

DEFUN(
	config_ctc_onu_tx_power,
	config_ctc_onu_tx_power_cmd,
	"ctc onu-tx-power-control <0-65535>",
    "ctc stack operations\n"
    "shut onu optical tx-power suply\n"
	"0:re-enable TX power supply;1-65534:duration shutdown time,unit:second;65535:permanently shutdown\n")
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;	
	ULONG onuIdx = 0; 
	INT16 PonPortIdx = 0;
	unsigned char ONUID[6];
	int len;
    CTC_STACK_onu_tx_power_supply_control_t parameter;
    if( PON_GetSlotPortOnu( (ULONG)(vty->index), &brdIdx, &portIdx, &onuIdx) == VOS_ERROR )
        return CMD_WARNING;
	PonPortIdx = GetPonPortIdxBySlot( (short int)brdIdx, (short int)portIdx );
	if (PonPortIdx == VOS_ERROR)
	{
		return CMD_WARNING;
	}

	VOS_MemZero( ONUID, 6 );
	if( GetOnuMacAddr( PonPortIdx, onuIdx-1, ONUID, &len) == VOS_ERROR )
        return CMD_WARNING;
    parameter.optical_id = CTC_OPTICAL_TRANSMITER_BOTH;
    VOS_MemCpy(parameter.onu_sn, ONUID, 6);
    parameter.action =  VOS_StrToUL(argv[0], NULL, 10); 
    if(VOS_OK != OnuMgt_SetTxPowerSupplyControl(PonPortIdx, onuIdx-1, &parameter))
        vty_out(vty, "  %%executed command error!\r\n");
	
	return CMD_SUCCESS;
}

/* end 2007-10-10 */

/*  2008-3-28
    按张新辉要求,在GT831节点下,增加如下命令*/
/*
ONU新支持的命令列表：
1. 显示ONU侧保存在内存、flash中的日志信息*/
LDEFUN(onu_syslog_show,
	onu_syslog_show_cmd,
	"show syslog [flash|memory] {<0-3>}*1", 
	SHOW_STR 
	"Syslog\n"
	"Syslog saved in flash\n"
	"Syslog saved in memory\n"
	"Syslog block number (total syslog size is 128K, splitted into 4 block, 32K in each block)\n",
	ONU_NODE
	)
{
	return CMD_SUCCESS;
}

/*2、显示语音侧保存在内存、flash中的日志信息*/
LDEFUN(config_cmd2lic_voice_syslog_show,
	config_cmd2lic_voice_syslog_show_cmd,
	"show voice syslog [flash|memory] {<0-3>}*1", 
	SHOW_STR 
	"Voice module information\n" 
	"Syslog\n"
	"Syslog saved in flash\n"
	"Syslog saved in memory\n"
	"Syslog block number (total syslog size is 128K, splitted into 4 block, 32K in each block)\n",
	ONU_NODE
        )
{
	return CMD_SUCCESS;
}

/*3、显示语音侧的异常日志*/
LDEFUN ( show_voice_exception_syslog_func,
        show_voice_exception_syslog_cmd,
        "show voice execpt-log",
        SHOW_STR
        "Voice module information\n"
        "Exception log file\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}

/*4、显示ONU侧的异常日志*/
LDEFUN ( show_exception_syslog_func,
        show_exception_syslog_cmd,
        "show execpt-log",
        SHOW_STR
        "Exception log file\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}

/*5、使能SIP的PRACK功能*/
LDEFUN(config_cmd2lic_sip_provisional_reliable,
	config_cmd2lic_sip_provisional_reliable_cmd,
	"sip prack {[enable|disable]}*1",
	"SIP configuration.\n"
	"PRACK support.\n"
	"Enable.\n"
	"Disable.\n",
	ONU_NODE
    	)
{
	return CMD_SUCCESS;
}

/*6、指定SIP的补充业务是在本地还是软交换上实现*/
LDEFUN(config_cmd2lic_sip_supplementary_service,
	config_cmd2lic_sip_supplementary_service_cmd,
	"sip supplement_service {[local|soft-switch]}*1",
	"SIP configuration.\n"
	"Supplement service support.\n"
	"Enable at local.\n"
	"Enable on softswitch.\n",
	ONU_NODE
    	)
{
	return CMD_SUCCESS;
}

/*7、配置SIP的每次REGISTER是否改变CID*/
LDEFUN(config_cmd2lic_sip_registar_cid_change,
	config_cmd2lic_sip_registar_cid_change_cmd,
	"sip registar_cid {[const|change]}*1",
	"SIP configuration.\n"
	"Registart CID policy.\n"
	"Remain the same as long as the iad running.\n"
	"Change in each re-registion.\n",
	ONU_NODE
    	)
{
	return CMD_SUCCESS;
}

/*8、显示语音呼叫统计*/
LDEFUN(config_cmd2lic_voice_stat_show,
	config_cmd2lic_voice_stat_show_cmd,
	"show voice call-statistics", 
	SHOW_STR 
	"Voice module information\n" 
	"Call statistics\n",
	ONU_NODE
     )
{
	return CMD_SUCCESS;
}
/* 9. 清除事件*/
LDEFUN(onu_event_clear,
	onu_event_clear_cmd,
	"event clear", 
	"ONU event config\n" 
	"Clear onu events\n",
	ONU_NODE)
{
	return CMD_SUCCESS;
}


enum{
    statistic_code_get_state = 1,
    statistic_code_set_state,
    statistic_code_get_data,
    statistic_code_set_data,
    statistic_code_get_his_data
};
DEFUN(ctc_onu_statistic_test,
        ctc_onu_statistic_test_cmd,
        "statistic [get|set] [state|data|hisdata] <port> {[enable|disable] <circle>}*1",
        "onu statistic cmds\n"
        "get operation\n"
        "set operation\n"
        "state of statistic function for a port\n"
        "data operation of a port\n"
        "port num\n"
        "enable port statistic state\n"
        "disble port statistic state\n"
        "statistic period set\n")
{
    int ponID = 0, ret = 0;

    ULONG ulIfIndex = 0, port;
    ULONG ulSlot, ulPort, ulOnuid, suffix;
    ULONG ulRet;

    int circle = 0, vcode = 0, en = 0;

    ulIfIndex = (ULONG) vty->index;

    ulRet = PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
    if (ulRet != VOS_OK)
        return CMD_WARNING;

    ponID = GetPonPortIdxBySlot((short int) ulSlot, (short int) ulPort);
    if (ponID == (VOS_ERROR))
    {
        vty_out(vty, "  %% Parameter error\r\n");
        return CMD_WARNING;
    }

    VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

    if(argc == 5)
    {
        en = VOS_StrCmp(argv[3], "enable")?1:2;
        circle = VOS_AtoI(argv[4]);
    }

    port = VOS_AtoI(argv[2]);

    if(!VOS_StriCmp(argv[0], "get"))
    {
        if(!VOS_StrCmp(argv[1], "state"))
            vcode = statistic_code_get_state;
        else
            vcode = VOS_StrCmp(argv[1], "data")?statistic_code_get_his_data:statistic_code_get_data;
    }
    else
    {
        vcode = VOS_StrCmp(argv[1], "data")?statistic_code_set_state:statistic_code_set_data;
    }

    if(argc == 5)
        ret = ctctest_statistic(ulSlot, ulPort, ulOnuid, port, vcode, en, circle);
    else
        ret = ctctest_statistic(ulSlot, ulPort, ulOnuid, port, vcode);


    return (ret)?CMD_SUCCESS:CMD_WARNING;
}

DEFUN(
        ctc_onu_port_statistic_state,
        ctc_onu_port_statistic_state_cmd,
        "statistic state <port_list> {[enable | disable]}*1 {<circle>}*1",
        "statistic operation\n"
        "statistic state show or set\n"
        "port list\n"
        "enable\n"
        "disable\n"
        "interval of port statistic, unit: second\n"
        )
{
    int ponID=0;

    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid, suffix;
    ULONG ulRet;

    ulIfIndex =(ULONG) vty->index;

    if(IsProfileNodeVty(ulIfIndex, &suffix))
    {

        ONUConfigData_t *pd = (ONUConfigData_t*) vty->onuconfptr;

        VTY_ONU_PROFILE_FE_PORT_IN_RANGE(argv[0], port)

        if (argc > 1)
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
            }END_PARSE_PORT_LIST_TO_PORT()
        }
        else
        {
            BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
            {
            }END_PARSE_PORT_LIST_TO_PORT()
        }

    }
    else
    {
        int error_flag = 0;
        ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( ulRet !=VOS_OK )
            return CMD_WARNING;

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        if(argc > 1)
        {

            CTC_STACK_statistic_state_t st;

            ONU_CONF_WRITEABLE_CHECK

            if(argc == 3)
            {
                st.circle = VOS_AtoI(argv[2]);
                st.status = VOS_StrCmp(argv[1], "enable")?CTC_STACK_STATISTIC_STATE_DISABLE:CTC_STACK_STATISTIC_STATE_ENABLE;
            }
            else
            {
                st.status = VOS_StrCmp(argv[1], "enable")?CTC_STACK_STATISTIC_STATE_DISABLE:CTC_STACK_STATISTIC_STATE_ENABLE;
                st.circle = (st.status == CTC_STACK_STATISTIC_STATE_ENABLE)?(15*60):0;
            }

            BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], port )
            {
                if( OnuMgt_SetOnuPortStatisticState(ponID, ulOnuid-1, (int)port, &st) != VOS_OK)
                {
#if 0                    
                    vty_out(vty, "port statistic state set fail\r\n");
#else
                    error_flag = 1;
#endif
                }
            }
            END_PARSE_PORT_LIST_TO_PORT();
            if(error_flag)
                vty_out(vty, ONU_CMD_ERROR_STR);
                
        }
        else
        {
                    CTC_STACK_statistic_state_t st;
                    BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
                    {
                        if(OnuMgt_GetOnuPortStatisticState(ponID, ulOnuid-1, (int)port, &st) == VOS_OK)
                            vty_out(vty, "\r\nport %d statistic stae %s, circle is %d\r\n", port, st.status == CTC_STACK_STATISTIC_STATE_ENABLE?"enable":"disable", st.circle);
                        else
                        {
#if 0                            
                            vty_out(vty, "\r\nport statistic state get fail!\r\n");
#else
                            error_flag = 1;
#endif
                        }
                    }
                    END_PARSE_PORT_LIST_TO_PORT();
                    if(error_flag)
                        vty_out(vty, ONU_CMD_ERROR_STR);
                        
        }
    }

    return CMD_SUCCESS;

}

DEFUN(
        ctc_onu_statistic_data_show,
        ctc_onu_statistic_data_show_cmd,
        "show statistic data <port_list>",
        SHOW_STR
        "statistic operation\n"
        "statistic data\n"
        "port list\n")
{
    int ponID=0;

    ULONG   ulIfIndex = 0, port;
    ULONG   ulSlot, ulPort, ulOnuid, suffix;
    ULONG ulRet;

    CTC_STACK_statistic_data_t data;

    ulIfIndex =(ULONG) vty->index;

    ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
    if( ulRet !=VOS_OK )
        return CMD_WARNING;

    ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
    if(ponID == (VOS_ERROR))
    {
        vty_out( vty, "  %% Parameter error\r\n");
        return CMD_WARNING;
    }
    
#define STATISTIC_OUT_FORMAT "  %-30s: 0x%08x%08x    %-30s: 0x%08x%08x\r\n"

    if(VOS_OK != VOS_MemCmp(argv[0], "pon", 3))
    {
        VTY_ONU_FE_PORT_IN_RANGE(ulSlot, ulPort, ulOnuid, argv[0], port)

        BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], port)
        {
            if(OnuMgt_GetOnuPortStatisticData(ponID, ulOnuid-1, (int)port, &data) == VOS_OK)
            {
                vty_out(vty, "\r\nport %d statistic data:\r\n\r\n", port);

                vty_out(vty, STATISTIC_OUT_FORMAT, "outDropEvents", data.downStreamDropEvents.msb, data.downStreamDropEvents.lsb,
                        "inDropEvents", data.upStreamDropEvents.msb, data.upStreamDropEvents.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outOctets", data.downStreamOctets.msb, data.downStreamOctets.lsb,
                                    "inOctets", data.upStreamOctets.msb, data.upStreamOctets.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames", data.downStreamFrames.msb, data.downStreamFrames.lsb,
                        "inFrames", data.upStreamFrames.msb, data.upStreamFrames.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outBroadcastFrames", data.downStreamBroadcastFrames.msb, data.downStreamBroadcastFrames.lsb,
                                    "inBroadcastFrames", data.upStreamBroadcastFrames.msb, data.upStreamBroadcastFrames.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outMulticastFrames", data.downStreamMulticastFrames.msb, data.downStreamMulticastFrames.lsb,
                        "inMulticastFrames", data.upStreamMulticastFrames.msb, data.upStreamMulticastFrames.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outCrcErroredFrames", data.downStreamCrcErroredFrames.msb, data.downStreamCrcErroredFrames.lsb,
                                    "inCrcErroredFrames", data.upStreamCrcErroredFrames.msb, data.upStreamCrcErroredFrames.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outUndersizeFrames", data.downStreamUndersizeFrames.msb, data.downStreamUndersizeFrames.lsb,
                        "inUndersizeFrames", data.upStreamUndersizeFrames.msb, data.upStreamUndersizeFrames.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outOversizeFrames", data.downStreamOversizeFrames.msb, data.downStreamOversizeFrames.lsb,
                                    "inOversizeFrames", data.upStreamOversizeFrames.msb, data.upStreamOversizeFrames.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outFragments", data.downStreamFragments.msb, data.downStreamFragments.lsb,
                        "inFragments", data.upStreamFragments.msb, data.upStreamFragments.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outJabbers", data.downStreamJabbers.msb, data.downStreamJabbers.lsb,
                                    "inJabbers", data.upStreamJabbers.msb, data.upStreamJabbers.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames64Octets", data.downStreamFrames64Octets.msb, data.downStreamFrames64Octets.lsb,
                        "inFrames64Octets", data.upStreamFrames64Octets.msb, data.upStreamFrames64Octets.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames65to127Octets", data.downStreamFrames65to127Octets.msb, data.downStreamFrames65to127Octets.lsb,
                                    "inFrames65to127Octets", data.upStreamFrames65to127Octets.msb, data.upStreamFrames65to127Octets.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames128to255Octets", data.downStreamFrames128to255Octets.msb, data.downStreamFrames128to255Octets.lsb,
                        "inFrames128to255Octets", data.upStreamFrames128to255Octets.msb, data.upStreamFrames128to255Octets.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames256to511Octets", data.downStreamFrames256to511Octets.msb, data.downStreamFrames256to511Octets.lsb,
                                    "inFrames256to511Octets", data.upStreamFrames256to511Octets.msb, data.upStreamFrames256to511Octets.lsb);

                vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames512to1023Octets", data.downStreamFrames512to1023Octets.msb, data.downStreamFrames512to1023Octets.lsb,
                                    "inFrames512to1023Octets", data.upStreamFrames512to1023Octets.msb, data.upStreamFrames512to1023Octets.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames1024to1518Octets", data.downStreamFrames1024to1518Octets.msb, data.downStreamFrames1024to1518Octets.lsb,
                                    "inFrames1024to1518Octets", data.upStreamFrames1024to1518Octets.msb, data.upStreamFrames1024to1518Octets.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outDiscards", data.downStreamDiscards.msb, data.downStreamDiscards.lsb,
                                    "inDiscards", data.upStreamDiscards.msb, data.upStreamDiscards.lsb);
                vty_out(vty, STATISTIC_OUT_FORMAT, "outErrors", data.downStreamErrors.msb, data.downStreamErrors.lsb,
                                    "inErrors", data.upStreamErrors.msb, data.upStreamErrors.lsb);

                vty_out(vty, "  %-30s: 0x%08x%08x\r\n", "portLinkChangTimes", data.portstatuschangetimes.msb, data.portstatuschangetimes.lsb);
            }
            else
                vty_out(vty, "\r\nport statistic state get fail!\r\n");
        }
        END_PARSE_PORT_LIST_TO_PORT()
    }
    else
    {
        if(OnuMgt_GetOnuPortStatisticData(ponID, ulOnuid-1, 0, &data) == VOS_OK)
        {    
            vty_out(vty, "\r\npon statistic data:\r\n\r\n");

            vty_out(vty, STATISTIC_OUT_FORMAT, "outDropEvents", data.downStreamDropEvents.msb, data.downStreamDropEvents.lsb,
                    "inDropEvents", data.upStreamDropEvents.msb, data.upStreamDropEvents.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outOctets", data.downStreamOctets.msb, data.downStreamOctets.lsb,
                                "inOctets", data.upStreamOctets.msb, data.upStreamOctets.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames", data.downStreamFrames.msb, data.downStreamFrames.lsb,
                    "inFrames", data.upStreamFrames.msb, data.upStreamFrames.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outBroadcastFrames", data.downStreamBroadcastFrames.msb, data.downStreamBroadcastFrames.lsb,
                                "inBroadcastFrames", data.upStreamBroadcastFrames.msb, data.upStreamBroadcastFrames.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outMulticastFrames", data.downStreamMulticastFrames.msb, data.downStreamMulticastFrames.lsb,
                    "inMulticastFrames", data.upStreamMulticastFrames.msb, data.upStreamMulticastFrames.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outCrcErroredFrames", data.downStreamCrcErroredFrames.msb, data.downStreamCrcErroredFrames.lsb,
                                "inCrcErroredFrames", data.upStreamCrcErroredFrames.msb, data.upStreamCrcErroredFrames.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outUndersizeFrames", data.downStreamUndersizeFrames.msb, data.downStreamUndersizeFrames.lsb,
                    "inUndersizeFrames", data.upStreamUndersizeFrames.msb, data.upStreamUndersizeFrames.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outOversizeFrames", data.downStreamOversizeFrames.msb, data.downStreamOversizeFrames.lsb,
                                "inOversizeFrames", data.upStreamOversizeFrames.msb, data.upStreamOversizeFrames.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outFragments", data.downStreamFragments.msb, data.downStreamFragments.lsb,
                    "inFragments", data.upStreamFragments.msb, data.upStreamFragments.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outJabbers", data.downStreamJabbers.msb, data.downStreamJabbers.lsb,
                                "inJabbers", data.upStreamJabbers.msb, data.upStreamJabbers.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames64Octets", data.downStreamFrames64Octets.msb, data.downStreamFrames64Octets.lsb,
                    "inFrames64Octets", data.upStreamFrames64Octets.msb, data.upStreamFrames64Octets.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames65to127Octets", data.downStreamFrames65to127Octets.msb, data.downStreamFrames65to127Octets.lsb,
                                "inFrames65to127Octets", data.upStreamFrames65to127Octets.msb, data.upStreamFrames65to127Octets.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames128to255Octets", data.downStreamFrames128to255Octets.msb, data.downStreamFrames128to255Octets.lsb,
                    "inFrames128to255Octets", data.upStreamFrames128to255Octets.msb, data.upStreamFrames128to255Octets.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames256to511Octets", data.downStreamFrames256to511Octets.msb, data.downStreamFrames256to511Octets.lsb,
                                "inFrames256to511Octets", data.upStreamFrames256to511Octets.msb, data.upStreamFrames256to511Octets.lsb);

            vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames512to1023Octets", data.downStreamFrames512to1023Octets.msb, data.downStreamFrames512to1023Octets.lsb,
                                "inFrames512to1023Octets", data.upStreamFrames512to1023Octets.msb, data.upStreamFrames512to1023Octets.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outFrames1024to1518Octets", data.downStreamFrames1024to1518Octets.msb, data.downStreamFrames1024to1518Octets.lsb,
                                "inFrames1024to1518Octets", data.upStreamFrames1024to1518Octets.msb, data.upStreamFrames1024to1518Octets.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outDiscards", data.downStreamDiscards.msb, data.downStreamDiscards.lsb,
                                "inDiscards", data.upStreamDiscards.msb, data.upStreamDiscards.lsb);
            vty_out(vty, STATISTIC_OUT_FORMAT, "outErrors", data.downStreamErrors.msb, data.downStreamErrors.lsb,
                                "inErrors", data.upStreamErrors.msb, data.upStreamErrors.lsb);

            vty_out(vty, "  %-30s: 0x%08x%08x\r\n", "portLinkChangTimes", data.portstatuschangetimes.msb, data.portstatuschangetimes.lsb);
        }
        else
            vty_out(vty, "\r\npon port statistic state get fail!\r\n");        
    }

    return CMD_SUCCESS;
}

#ifdef _OEM_TYPE_CLI_
void  init_onu_daya_information(void)
{
	/*port mode <slotport_list> {[0|8|9|10|11|12]}*1*/
	VOS_StrCpy(fe_port_mode,"Show or config onu FE port information\nShow or config onu FE port mode information\nPlease input the slot/port list\nAuto negotiation\n");
	VOS_StrCat(fe_port_mode,"100M/FD\n100M/HD\n10M/FD\n10M/HD\n1000M/FD,only for");
	VOS_StrCat(fe_port_mode,GetDeviceTypeString(V2R1_ONU_GT816));
	VOS_StrCat(fe_port_mode,"\n");

	/*port ingress_rate <slotport_list> {[0|1|2|3] [<0>|<62-100000>]}*1*/
	VOS_StrCpy(ingress_rate,"Show or config onu FE port information\nShow or config onu FE port rate information\nPlease input the slot/port list\n0-Limit all frames");
	VOS_StrCat(ingress_rate,"0-Limit all frames\n1-Limit Broadcast, Multicast and flooded unicast frames\n2-Limit Broadcast and Multicast frames only\n3-Limit Broadcast frames only\n0-not limit\nport ingress rate,unit:kbps,for");
	VOS_StrCat(ingress_rate,GetDeviceTypeString(V2R1_ONU_GT810));
	VOS_StrCat(ingress_rate,"/");
	VOS_StrCat(ingress_rate,GetDeviceTypeString(V2R1_ONU_GT816));
	VOS_StrCat(ingress_rate,",the rate is 100-100000;for");
	VOS_StrCat(ingress_rate,GetDeviceTypeString(V2R1_ONU_GT811_A));
	VOS_StrCat(ingress_rate,"/");
	VOS_StrCat(ingress_rate,GetDeviceTypeString(V2R1_ONU_GT812_A));
	VOS_StrCat(ingress_rate,"/");
	VOS_StrCat(ingress_rate,GetDeviceTypeString(V2R1_ONU_GT816));
	VOS_StrCat(ingress_rate,",Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64M are supported\n");

	/*port ingress_rate <slotport_list> [0|1|2|3] [<0>|<62-100000>] {[drop|pause] [12k|24k|48k|96k]}*1*/
	VOS_StrCpy(ingress_rate1,"Show or config onu FE port information\nShow or config onu FE port rate information\nPlease input the slot/port list\n0-Limit all frames\n1-Limit Broadcast, Multicast and flooded unicast frames\n2-Limit Broadcast and Multicast frames only\n3-Limit Broadcast frames only\n0-not limit\nport ingress rate,unit:kbps,for");
	VOS_StrCat(ingress_rate1,GetDeviceTypeString(V2R1_ONU_GT810));
	VOS_StrCat(ingress_rate1,"/");
	VOS_StrCat(ingress_rate1,GetDeviceTypeString(V2R1_ONU_GT816));
	VOS_StrCat(ingress_rate1,"the rate is 100-100000;for");
	VOS_StrCat(ingress_rate1,GetDeviceTypeString(V2R1_ONU_GT811_A));
	VOS_StrCat(ingress_rate1,"/");
	VOS_StrCat(ingress_rate1,GetDeviceTypeString(V2R1_ONU_GT812_A));
	VOS_StrCat(ingress_rate1,"/");
	VOS_StrCat(ingress_rate1,GetDeviceTypeString(V2R1_ONU_GT816));
	VOS_StrCat(ingress_rate1,",Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64M are supported\n");
	VOS_StrCat(ingress_rate1,"Frames will be dropped when exceed limit.\nPort will transmit pause frame when exceed limit.\nBurst mode : Burst Size is 12k bytes.\nBurst mode : Burst Size is 24k bytes.\nBurst mode : Burst Size is 48k bytes.\nBurst mode : Burst Size is 96k bytes.\n");

	/*port egress_rate <slotport_list> {[0|<62-100000>]}*1*/
	VOS_StrCpy(egress_rate,"Show or config onu FE port information\nShow or config onu FE port rate information\nPlease input the slot/port list\n0-not limit\nPlease input the rate( 62-100000) kbps,for");
	VOS_StrCat(egress_rate,GetDeviceTypeString(V2R1_ONU_GT816));
	VOS_StrCat(egress_rate,",Only 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64Mbps are supported\n");

	/*port ingress_rate <port_list> {[0|1|2|3] [<0>|<62-100000>]}*1*/
	VOS_StrCpy(onu_ingress_rate,"Show or config onu FE port information\nShow or config onu FE port rate information\nPlease input the port number\n");
	VOS_StrCat(onu_ingress_rate,"0-Limit all frames\n1-Limit Broadcast, Multicast and flooded unicast frames\n2-Limit Broadcast and Multicast frames only\n3-Limit Broadcast frames only\n0-not limit\nport ingress rate,unit:kbps,for");
	VOS_StrCat(onu_ingress_rate,GetDeviceTypeString(V2R1_ONU_GT810));
	VOS_StrCat(onu_ingress_rate,"/");
	VOS_StrCat(onu_ingress_rate,GetDeviceTypeString(V2R1_ONU_GT816));
	VOS_StrCat(onu_ingress_rate,",the rate is 100-100000;for");
	VOS_StrCat(onu_ingress_rate,GetDeviceTypeString(V2R1_ONU_GT811_A));
	VOS_StrCat(onu_ingress_rate,"/");
	VOS_StrCat(onu_ingress_rate,GetDeviceTypeString(V2R1_ONU_GT812_A));
	VOS_StrCat(onu_ingress_rate,",Only 64, 128, 256, 512, 1000, 2000, 4000, 8000, 16000, 32000, 64000 are supported\n");

	/*port ingress_rate <port_list> [0|1|2|3] [<0>|<62-100000>] {[drop|pause] [12k|24k|48k|96k]}*1*/
	VOS_StrCpy(onu_ingress_rate1,"Show or config onu FE port information\nShow or config onu FE port rate information\nPlease input the port number\n");
	VOS_StrCat(onu_ingress_rate1,"0-Limit all frames\n1-Limit Broadcast, Multicast and flooded unicast frames\n2-Limit Broadcast and Multicast frames only\n3-Limit Broadcast frames only\n0-not limit\nport ingress rate,unit:kbps,for");
	VOS_StrCat(onu_ingress_rate1,GetDeviceTypeString(V2R1_ONU_GT810));
	VOS_StrCat(onu_ingress_rate1,"/");
	VOS_StrCat(onu_ingress_rate1,GetDeviceTypeString(V2R1_ONU_GT816));
	VOS_StrCat(onu_ingress_rate1,",the rate is 100-100000;for");
	VOS_StrCat(onu_ingress_rate1,GetDeviceTypeString(V2R1_ONU_GT811_A));
	VOS_StrCat(onu_ingress_rate1,"/");
	VOS_StrCat(onu_ingress_rate1,GetDeviceTypeString(V2R1_ONU_GT812_A));
	VOS_StrCat(onu_ingress_rate1,",Only 64, 128, 256, 512, 1000, 2000, 4000, 8000, 16000, 32000, 64000 are supported\n");

	/*port egress_rate <port_list> {[0|<62-100000>]}*1*/
	VOS_StrCpy(onu_egress_rate, "Show or config onu FE port information\nShow or config onu FE port rate information\nPlease input the port number\n0-not limit\nPlease input the rate( 62-100000) kbps,for");
	VOS_StrCat(onu_egress_rate,GetDeviceTypeString(V2R1_ONU_GT810));
	VOS_StrCat(onu_egress_rate,"/");
	VOS_StrCat(onu_egress_rate,GetDeviceTypeString(V2R1_ONU_GT816));
	VOS_StrCat(onu_egress_rate,",the rate is (100-100000)kbps\n");

	/*qos user_pri_tc <0-7> {<0-7>}*1*/
	VOS_StrCpy(onu_qos_pri,"Show or config onu QoS information\nShow or config onu 802.1p priority associated with traffic-class queue\nuser_pri,range:0-7\ntraffic_class,range:0-7;but for ");
	VOS_StrCat(onu_qos_pri,GetDeviceTypeString(V2R1_ONU_GT811));
	VOS_StrCat(onu_qos_pri,"/");
	VOS_StrCat(onu_qos_pri,GetDeviceTypeString(V2R1_ONU_GT812));
	VOS_StrCat(onu_qos_pri,",the range is 0-3\n");

	/*qos dscp_tc <0-63> {<0-7>}*1*/
	VOS_StrCpy(qos_dscp_tc,"Show or config onu QoS information\nShow or config onu IP-DSCP associated with traffi-class queue\ntos,range:0-63\ntraffic_class,range:0-7;but for");
	VOS_StrCat(qos_dscp_tc,GetDeviceTypeString(V2R1_ONU_GT811));
	VOS_StrCat(qos_dscp_tc,"/");
	VOS_StrCat(qos_dscp_tc,GetDeviceTypeString(V2R1_ONU_GT812));
	VOS_StrCat(qos_dscp_tc,",range is 0-3\n");

}
#endif

LONG onu_Qos_CommandInstall(enum node_type  node )
{
	install_element ( node, &onu_pas_pq_set_cmd );
	install_element ( node, &onu_qos_filter_l3_rules_cmd );
	install_element ( node, &onu_qos_filter_l2_rules_cmd );
	
	install_element ( node, &onu_qos_classifier_l3_rules_cmd );
	install_element ( node, &onu_qos_classifier_l2_rules_cmd );
	install_element ( node, &onu_atu_filter_cmd );
	install_element ( node, &onu_qos_vlan_priority_mode_cmd );

	
	install_element ( node, &onu_qos_classfier_pas_show_cmd );
/*	发件人: KD刘冬 
发送时间: 2008年1月3日 11:32
收件人: KD解世立; KD盖鹏飞
抄送: KD许永刚; KD姚力; KD张新辉; KD张西昌; KD陈福军; KD黄杨广
主题: 答复: ONU节点下增加的命令没有说明文档

新增命令说明见附件ONU节点。
注意：请OLT侧将qos precedence {[up|down] [l2|l3]}*1 删掉
将qos precedence {[up|down] [234|243|342|432|34|43|2]}*1中的"2"改为"2--"（两个"-"）
（即：qos precedence {[up|down] [234|243|342|432|34|43|2--]}*1）
*/
	/*install_element ( node, &onu_qos_precedence_cmd );*/
	install_element ( node, &onu_qos_classifier_rules_precedence_cmd );
	install_element ( node, &onu_vlan_pas_rule_cmd );

	/* modified by chenfj 2007-11-26
	按新辉要求，GT831 不支持atu port-limit ，将此删除
	if( node != ONU_GT821_GT831_NODE )
		install_element ( node, &onu_atu_port_learn_limit_cmd );	 added by xieshl 20071008 */
	/*install_element ( node, &onu_qos_ip_pri_en_cmd );*/

/* added by xieshl 20071010, only for CNC test */
#ifdef CNC_2007_10_TEST
	init_cnc_test_onu_vlan();
	install_element ( node, &onu_cnc_vlan_create_cmd );
	install_element ( node, &onu_cnc_vlan_delete_cmd );
	install_element ( node, &onu_cnc_vlan_port_add_cmd );
	install_element ( node, &onu_cnc_vlan_port_del_cmd );
	/*install_element ( node, &onu_cnc_vlan_show_cmd );*/
	install_element ( node, &onu_cnc_vlan_count_show_cmd );
/* end 20071010 */
#endif

	return VOS_OK;
}
/* end 2007-10-07 */

LONG onu_Qos_CommandInstall_Ldefun(enum node_type  node )
{	
	install_element ( node, &ldef_onu_pas_pq_set_cmd );
	install_element ( node, &ldef_onu_qos_filter_l3_rules_cmd );
	install_element ( node, &ldef_onu_qos_filter_l2_rules_cmd );
	
	install_element ( node, &ldef_onu_qos_classifier_l3_rules_cmd );
	install_element ( node, &ldef_onu_qos_classifier_l2_rules_cmd );
	install_element ( node, &ldef_onu_atu_filter_cmd );
	install_element ( node, &ldef_onu_qos_vlan_priority_mode_cmd );

	
	install_element ( node, &ldef_onu_qos_classfier_pas_show_cmd );
/*	发件人: KD刘冬 
发送时间: 2008年1月3日 11:32
收件人: KD解世立; KD盖鹏飞
抄送: KD许永刚; KD姚力; KD张新辉; KD张西昌; KD陈福军; KD黄杨广
主题: 答复: ONU节点下增加的命令没有说明文档

新增命令说明见附件ONU节点。
注意：请OLT侧将qos precedence {[up|down] [l2|l3]}*1 删掉
将qos precedence {[up|down] [234|243|342|432|34|43|2]}*1中的"2"改为"2--"（两个"-"）
（即：qos precedence {[up|down] [234|243|342|432|34|43|2--]}*1）
*/
	/*install_element ( node, &onu_qos_precedence_cmd );*/
	install_element ( node, &ldef_onu_qos_classifier_rules_precedence_cmd );
	install_element ( node, &ldef_onu_vlan_pas_rule_cmd );

	/* modified by chenfj 2007-11-26
	按新辉要求，GT831 不支持atu port-limit ，将此删除
	if( node != ONU_GT821_GT831_NODE )
		install_element ( node, &onu_atu_port_learn_limit_cmd );	 added by xieshl 20071008 */
	/*install_element ( node, &onu_qos_ip_pri_en_cmd );*/

/* added by xieshl 20071010, only for CNC test */
#ifdef CNC_2007_10_TEST
	init_cnc_test_onu_vlan();
	install_element ( node, &ldef_onu_cnc_vlan_create_cmd );
	install_element ( node, &ldef_onu_cnc_vlan_delete_cmd );
	install_element ( node, &ldef_onu_cnc_vlan_port_add_cmd );
	install_element ( node, &ldef_onu_cnc_vlan_port_del_cmd );
	/*install_element ( node, &onu_cnc_vlan_show_cmd );*/
	install_element ( node, &ldef_onu_cnc_vlan_count_show_cmd );
/* end 20071010 */
#endif

	return VOS_OK;
}

/* added by xieshl 20080603, 问题单6349 */
VOID onu_share_cmd_install(enum node_type  node)
{
	install_element ( node, &onu_fe_port_show_cmd);
}
extern LONG ONU_ExtBrd_IAD_CommandInstall( enum node_type  node);


LONG ONUDAYA_CommandInstall_Ldefun( enum node_type  node)
{
	if(( node == ONU_NODE ) ||(node == ONU_GT821_GT831_NODE) || (node == ONU_GT861_NODE) || (node == ONU_GT831B_NODE))
    		onu_Qos_CommandInstall_Ldefun( node );

	if((node == ONU_GT821_GT831_NODE) || (node == ONU_GT861_NODE) /*|| (node == ONU_GT831B_NODE)*/)/*问题单7737*/
		ONU_ExtBrd_IAD_CommandInstall( node);


	/* added by xieshl 20090226, 根据GT815的流控问题新增2个命令 */
	install_element ( node, &ldef_onu_cfg_uni_pause_cmd);
	install_element ( node, &ldef_onu_poe_mode_cmd);


    /*********************大亚命令****************************/
    install_element ( node, &ldef_onu_show_igmpsnoop_version_cmd);
    install_element ( node, &ldef_onu_port_igmpsnoop_enable_cmd);
	/*modified by wutw at 4 October*/
    /*install_element ( node, &onu_igmpsnoop_channel_cmd);*/
	/*install_element ( node, &onu_igmpsnoop_aging_cmd);*/
	if(node == ONU_GT861_NODE)
		install_element ( node, &ldef_onu_igmpsnoop_gda_add_cmd_for861);
	else
		install_element ( node, &ldef_onu_igmpsnoop_gda_add_cmd);
    install_element ( node, &ldef_onu_igmpsnoop_gda_del_cmd);
    install_element ( node, &ldef_onu_igmpsnoop_gda_show_cmd);
	/*modified by wutw at 4 October*/
    /*install_element ( node, &onu_igmpsnoop_last_member_cmd);*/

    /*install_element ( node, &onu_port_igmpsnoop_param_cmd);*/

	/*add by wangxy 2007-03-12 */
	install_element( node, &ldef_onu_igmpsnooping_group_aging_time_cmd );
	install_element( node, &ldef_onu_igmpsnooping_host_aging_time_cmd );
	if(node == ONU_GT861_NODE)
		install_element( node, &ldef_onu_port_igmpsnoop_fast_leave_cmd_for_861 );
	else
		install_element( node, &ldef_onu_port_igmpsnoop_fast_leave_cmd);
	install_element( node, &ldef_onu_igmpsnooping_max_response_time_cmd);
	install_element( node, &ldef_onu_igmpsnooping_max_group_cmd);

    install_element ( node, &ldef_onu_vlan_enable_cmd);
	/* modified by chenfj 2007-11-12
	问题单#5778: 无法将端口的PVID设置为默认1
	在GT831/821/810/816中，由于端口的PVID是直接跟端口的默认VLAN相关的，PVID命令没有用处并经常造成理解问题，建议在OLT的这四个ONU节点下去除该命令
     处理：在831节点下删除该命令。
                     在810/816节点下该命令保留，因为当前810/816与811/812共用一个命令节点，而这个命令在811/812上是有用的；但在处理时检查ONU类型；
                     若是810/816，则给出出错信息提示：GT810(或GT816)don't support this cli command
	*/	
	if(node != ONU_GT821_GT831_NODE)
		{
		if(node == ONU_GT861_NODE)
			install_element ( node, &ldef_onu_vlan_pvid_cmd_for_861);
		else 
   		 	install_element ( node, &ldef_onu_vlan_pvid_cmd);
		}
	if(node == ONU_GT861_NODE)
		{
		install_element ( node, &ldef_onu_vlan_forward_type_cmd_for_861);
		install_element ( node, &ldef_onu_vlan_ingress_filter_cmd_for_861);
		install_element ( node, &ldef_onu_vlan_dot1q_port_add_cmd_for_861 );
		install_element ( node, &ldef_onu_vlan_dot1q_port_del_cmd_for_861);
		}
	else{
		install_element ( node, &ldef_onu_vlan_forward_type_cmd);
		install_element ( node, &ldef_onu_vlan_ingress_filter_cmd);
		install_element ( node, &ldef_onu_vlan_dot1q_port_add_cmd);
		install_element ( node, &ldef_onu_vlan_dot1q_port_del_cmd);
		}
    install_element ( node, &ldef_onu_vlan_dot1q_add_cmd);
    install_element ( node, &ldef_onu_vlan_dot1q_del_cmd);
    install_element ( node, &ldef_onu_vlan_dot1q_show_cmd);
    install_element ( node, &ldef_onu_vlan_port_isolate_cmd);
    
    /*install_element ( node, &onu_atu_size_cmd);*/
    install_element ( node, &ldef_onu_atu_aging_cmd);
	if(node == ONU_GT861_NODE)
		{
		install_element ( node, &ldef_onu_atu_static_add_cmd_for_861);
		install_element ( node, &ldef_onu_atu_static_add__dup_cmd_for_861);
		}
	else{
		install_element ( node, &ldef_onu_atu_static_add_cmd);
		install_element ( node, &ldef_onu_atu_static_add__dup_cmd );
		}
    
    install_element ( node, &ldef_onu_atu_static_del_cmd);
    install_element ( node, &ldef_onu_atu_static_show_cmd);
    /*install_element ( node, &onu_event_oam_sym_alarm_cmd);
    install_element ( node, &onu_event_oam_frm_alarm_cmd); 
    install_element ( node, &onu_event_oam_frp_alarm_cmd); 
    install_element ( node, &onu_event_oam_sec_alarm_cmd);  */
    install_element ( node, &ldef_onu_event_show_cmd);

    install_element ( node, &ldef_onu_band_set_cmd);     
    /*install_element ( node, &onu_band_broadcast_set_cmd);*/
	if(node == ONU_GT861_NODE)
		{
		install_element ( node, &ldef_onu_fe_port_en_set_cmd_for_861);
		install_element ( node, &ldef_onu_fe_port_desc_set_cmd_for_861);
		install_element ( node, &ldef_onu_fe_port_mode_set_cmd_for_861 );
		install_element ( node, &ldef_onu_fe_port_fc_set_cmd_for_861);
		}
	else{
		install_element ( node, &ldef_onu_fe_port_en_set_cmd);
		install_element ( node, &ldef_onu_fe_port_desc_set_cmd);
		install_element ( node, &ldef_onu_fe_port_mode_set_cmd);
		install_element ( node, &ldef_onu_fe_port_fc_set_cmd);
		}

/*added by wangxy 2007-11-06*/
	/* modified by chenfj  问题单7324
	   1  port link_mon 和port mode_mon 命令在831 和861上不支持, 故不挂在这些节点下
	   2  在ONU_NODE  节点下, GT810/GT816/GT812_A/GT811_A/gt815不支持, daya GT811/GT812支持
	   */
	if(!((node == ONU_GT821_GT831_NODE) || (node == ONU_GT861_NODE) || (node == ONU_GT831B_NODE)))
		{
		install_element( node, &ldef_onu_fe_port_link_mon_cmd );
		install_element( node, &ldef_onu_fe_port_mode_mon_cmd );
		}
	
    /*install_element ( node, &onu_fe_port_rate_set_cmd);*/
	
	/*modified by wutw 2006/11/12*/
    /*install_element ( node, &onu_fe_port_maclimit_set_cmd);*/

	if(node == ONU_GT861_NODE)
		{
		install_element ( node, &ldef_onu_qos_def_pri_cmd_for_861);
		install_element ( node, &ldef_onu_qos_user_pri_reg_cmd_for_861);
		
		install_element ( node, &ldef_onu_loopback_port_en_cmd_for_861);
		install_element ( node, &ldef_onu_statistic_port_flush_cmd_for_861);
		install_element ( node, &ldef_onu_statistic_port_show_cmd_for_861);
		}
	else{
		install_element ( node, &ldef_onu_qos_def_pri_cmd);
		install_element ( node, &ldef_onu_qos_user_pri_reg_cmd);

		install_element ( node, &ldef_onu_loopback_port_en_cmd);
		install_element ( node, &ldef_onu_statistic_port_flush_cmd);
		install_element ( node, &ldef_onu_statistic_port_show_cmd);
		}

    /*install_element ( node, &onu_qos_def_port_tc_cmd);*/
    install_element ( node, &ldef_onu_qos_user_pri_tc_cmd);
    install_element ( node, &ldef_onu_qos_tos_tc_cmd);
    install_element ( node, &ldef_onu_qos_algorithm_cmd);

    /*install_element ( node, &onu_loopback_uni_test_cmd);*/
    
    /*install_element ( node, &onu_statistic_port_histogram_cmd);
    install_element ( node, &onu_statistic_port_flush_cmd);
    install_element ( node, &onu_statistic_port_show_cmd);*/
    install_element ( node, &ldef_onu_statistic_pas_flush_cmd);

 	/* modified by chenfj 2007-11-12
 	问题单#5794: 不支持温度命令，建议将该命令去掉
 	*/
 	if( node != ONU_GT821_GT831_NODE )
   		install_element ( node, &ldef_onu_mgt_temperature_cmd);
    install_element ( node, &ldef_onu_mgt_conf_cmd);
    install_element ( node, &ldef_onu_mgt_reset_cmd);
    install_element ( node, &ldef_onu_mgt_laser_cmd);

	install_element( node, &ldef_onu_mgt_read_reg_cmd );
	
    /*install_element ( node, &onu_mgt_self_check_result_cmd); */   

	/*added by wutw at 23 October*/
	if(node == ONU_GT861_NODE)
		install_element ( node, &ldef_onu_qos_user_pri_reg_show_cmd_for_861);
	else
		install_element ( node, &ldef_onu_qos_user_pri_reg_show_cmd);
    install_element ( node, &ldef_onu_qos_user_pri_tc_show_cmd);
    install_element ( node, &ldef_onu_qos_tos_tc_show_cmd); 	


	/*added by wutongwu at 18 October*/
	/*install_element ( node, &onu_statistic_port_all_show_cmd); */
	/* modified by chenfj 2007-11-12
	     问题单#5793: mgt self_check_result命令OLT不支持
	     其实是ONU侧不支持该命令
	     修改: 对831 ONU，删除该命令
	     */
	     if( node != ONU_GT821_GT831_NODE )
			install_element ( node, &ldef_onu_mgt_self_check_result_all_cmd);
	/*added by wutongwu at 26 October*/		
	
	if(node == ONU_GT861_NODE)
		{
		install_element ( node, &ldef_onu_fe_ingress_rate_set_cmd_for_861);
		install_element ( node, &ldef_onu_fe_ingress_rate_set_cmd1_for_861);
		install_element ( node, &ldef_onu_fe_egress_rate_set_cmd_for_861);
		}
	else{
		install_element ( node, &ldef_onu_fe_ingress_rate_set_cmd);
		install_element ( node, &ldef_onu_fe_ingress_rate_set_cmd1);
		install_element ( node, &ldef_onu_fe_egress_rate_set_cmd);
		}


	/*added by wangxy 2007-05-15*/
	/* modified by chenfj 2007-11-12
	     问题单#5782: port ingress_rate_limit_base命令OLT不支持
	     其实是ONU侧不支持该命令
	     修改: 对831 ONU，删除该命令
	     */
	if( node  != ONU_GT821_GT831_NODE )
		install_element ( node, &ldef_onu_fe_port_ingress_rate_limit_base_cmd );

	/*11.6 added by wutw*/
	install_element ( node, &ldef_onu_uptime_show_cmd);
	
	if(node == ONU_GT861_NODE)
		{
		install_element ( node, &ldef_onu_qos_rule_cmd_for_861);
		install_element ( node, &ldef_onu_qos_ip_pri_en_cmd_for_861);
		install_element ( node, &ldef_onu_qos_user_pri_en_cmd_for_861);
		
		install_element ( node, &ldef_onu_fe_port_negoration_show_cmd_for_861 );
		install_element ( node, &ldef_onu_fe_port_mirror_from_set_cmd_for_861);
		install_element ( node, &ldef_onu_fe_port_mirror_to_set_cmd_for_861);
		}
	else{
		install_element ( node, &ldef_onu_qos_rule_cmd);
		install_element ( node, &ldef_onu_qos_ip_pri_en_cmd);
		install_element ( node, &ldef_onu_qos_user_pri_en_cmd);
		
		install_element ( node, &ldef_onu_fe_port_negoration_show_cmd);
		install_element ( node, &ldef_onu_fe_port_mirror_from_set_cmd);
		install_element ( node, &ldef_onu_fe_port_mirror_to_set_cmd);
		}

	install_element ( node, &ldef_onu_fe_port_show_cmd);
	install_element ( node, &ldef_onu_fe_port_mirror_show_cmd);	
 	/*********************end of daya  ********************************/   

	/* only GT816/810 支持的命令*/
	/* modified by chenfj 2007-8-30
		按照大亚方式统一一下命令。 张新辉需按照新的定义修改GT816的软件
	*/
	if(( node == ONU_NODE ) || (node == ONU_GT861_NODE) || (node == ONU_GT831B_NODE))
		{
		install_element ( node, &ldef_onu_Igmpsnoop_Authenable_Cmd );
		if(node == ONU_GT861_NODE)
			{
			install_element ( node, &ldef_onu_atu_flood_cmd_for_861);
			install_element ( node, &ldef_onu_cable_test_gt811_cmd_for_861);

			install_element ( node, &ldef_iad_vlan_set_cmd);
			install_element ( node, &ldef_onu_slot_mirror_from_to_cmd);
			}
		else{
			install_element ( node, &ldef_onu_atu_flood_cmd );
			install_element ( node, &ldef_onu_cable_test_gt811_cmd );
			}

		/* ftp download file to onu */
		install_element ( node, &ldef_ftpc_download_ftp_phenixos_master_cmd );
		/* added by chenfj 2007-10-30 */
		if(node!=ONU_GT831B_NODE)                 /*问题单7738*/
		install_element ( node, &ldef_onu_mgt_config_ip_addr_gt811_cmd );
		}

	if(( node == ONU_NODE ) ||(node == ONU_GT821_GT831_NODE) || (node == ONU_GT861_NODE) || (node == ONU_GT831B_NODE))
		{
		if(node == ONU_GT861_NODE)
			install_element ( node, &ldef_onu_atu_learn_cmd_for_861);
		else
			install_element ( node, &ldef_onu_atu_learn_cmd );
		
		install_element ( node, &ldef_onu_dscp_relay_agent_cmd );

		install_element ( node, &ldef_onu_mgt_running_config_gt811_cmd );
		install_element ( node, &ldef_onu_mgt_startup_config_gt811_cmd );

		install_element ( node, &ldef_onu_atu_limit_cmd );
		}


    return VOS_OK;
}

#if 1
LONG ONUDAYA_CommandInstall( enum node_type  node)
{

    /*
     * add by wangxy 2011-06-09
     */
    if((node == ONU_CTC_NODE) || (node == ONU_CMC_NODE))
    {
        CTC_VoipCmd_Init(node);
		
        install_element(node, &DrPeng_onu_set_port_isolation_cmd);
	 install_element(node, &DrPeng_onu_get_port_id_cmd);
	  install_element(node, &DrPeng_onu_get_mac_number_cmd);
	  install_element(node, &DrPeng_onu_set_storm_control_cmd);
	  install_element(node, &DrPeng_onu_get_storm_control_cmd);
	  install_element(node, &DrPeng_onu_get_port_isolation_cmd);
	  install_element( node , &onu_mac_entry_show_cmd);
	  install_element( node , &DrPeng_onu_set_port_mode_cmd);
	  install_element( node , &DrPeng_onu_set_loop_detec_cmd);
	  install_element( node , &DrPeng_onu_get_loop_detec_cmd);
	  install_element( node , &DrPeng_onu_set_port_save_cmd);
	  install_element( node , &DrPeng_onu_device_name_config_cmd);
	  install_element( node , &DrPeng_onu_location_name_config_cmd);
	  install_element( node , &DrPeng_onu_description_name_config_cmd);
        /*new added by lu 2012-2-20*/
        install_element(node, &config_ctc_onu_tx_power_cmd);
        install_element(node, &onu_bat_vlan_dot1q_rules_show_cmd);
        /*comment by wangxiaoyu 2011-07-27 onu节点下添加了自动私有化配置文件操作，为防止在临时私有文件关联时执行关联、去关联操作*/
#if 0
        install_element(node, &onu_profile_associate_cmd);
        install_element(node, &onu_profile_undo_associate_cmd);
        install_element(node, &onu_profile_private_cmd);
#endif
        install_element(node, &onu_vlan_translation_set_cmd);
        install_element(node, &onu_vlan_translation_del_cmd);
        install_element(node, &onu_vlan_aggregation_add_cmd);
        install_element(node, &onu_vlan_aggregation_del_cmd);
		install_element ( node, &onu_mgt_running_config_gt811_cmd );/*add by luh 2011-11-23*/
        install_element(node, &onu_qinq_vlan_tag_enable_cmd);
        install_element(node, &onu_qinq_vlan_tag_add_cmd);
        install_element(node, &onu_qinq_vlan_tag_del_cmd);
        install_element(node, &onu_qinq_vlan_tag_config_show_cmd);
        
        install_element(node, &onu_vlan_transparent_port_enable_cmd);
        install_element(node, &onu_vlan_enable_cmd);
        install_element(node, &onu_vlan_pvid_cmd);
        install_element(node, &onu_vlan_dot1q_add_cmd);
        install_element(node, &onu_vlan_dot1q_del_cmd);
        install_element(node, &onu_vlan_dot1q_port_add_cmd);
        install_element(node, &onu_vlan_dot1q_port_del_cmd);
        install_element(node, &onu_vlan_dot1q_show_cmd);

        install_element(node, &onu_port_igmpsnoop_enable_cmd);
        install_element(node, &onu_port_multicast_vlan_set_cmd);
        install_element(node, &onu_port_igmpsnooping_groupnum_set_cmd);
        install_element(node, &onu_port_igmpsnooping_tagstrip_set_cmd);
        install_element(node, &onu_port_igmpsnooping_tagoper_set_cmd);
        install_element(node, &onu_Igmpsnoop_Authenable_Cmd);

        install_element(node, &onu_fe_port_en_set_cmd);
        /*install_element(node, &onu_fe_port_fc_set_cmd);*/
        install_element(node, &onu_fe_ingress_rate_set_cmd);
		install_element(node, &onu_fe_ingress_rate_set_cmd1);
        install_element(node, &onu_fe_egress_rate_set_cmd);
        install_element(node, &onu_fe_port_mode_set_cmd);
        install_element(node, &onu_fe_port_an_rst_cmd);
        install_element(node, &onu_fe_port_pause_set_cmd);

        install_element ( node, &onu_igmpsnoop_gda_show_cmd);   /*added by luh at 2011.11.17*/
        install_element(node, &onu_vlan_mode_cmd);

        install_element(node, &onu_mgt_reset_cmd);

        install_element(node, &onu_conf_qos_class_match_cmd);
        install_element(node, &onu_conf_qos_policy_map_cmd);
        install_element(node, &onu_conf_qos_service_policy_cmd);

        install_element(node, &undo_onu_conf_qos_class_match_cmd);
        install_element(node, &undo_onu_conf_qos_policy_map_cmd);
        install_element(node, &undo_onu_conf_qos_service_policy_cmd);
        install_element(node, &show_onu_conf_qos_policy_map_cmd);

        install_element(node, &onu_vlan_translation_show_cmd);
        install_element(node, &show_onu_profile_cmd);
        install_element(node, &show_onu_conf_name_by_id_cmd);

        /*install_element(node, &show_onu_version_cmd);*/
        install_element ( node, &onu_fe_port_show_cmd);

        /*install_element( node, &ctc_onu_statistic_test_cmd);*/
        install_element( node, &ctc_onu_port_statistic_state_cmd);
        install_element( node , &ctc_onu_statistic_data_show_cmd);

        return VOS_OK;
    }
    else if(node == ONU_PROFILE_NODE)
    {

#if 0
        install_element(node, &onu_profile_undo_associate_cmd);
#endif
        /*install_element(node, &onu_bat_vlan_dot1q_valid_config_cmd);*/
        /*install_element(node, &onu_bat_vlan_dot1q_add_cmd);*/
        install_element(node, &onu_bat_port_vlan_dot1q_add_cmd);
        install_element(node, &onu_bat_vlan_dot1q_step_add_cmd);
        install_element(node, &onu_bat_vlan_dot1q_step_portid_add_cmd);/*added by luh 2015-1-8*/
        
        install_element(node, &onu_bat_vlan_dot1q_rules_show_cmd);
        install_element(node, &onu_bat_vlan_dot1q_rules_delete_cmd);

        install_element(node, &onu_vlan_mode_cmd);

        install_element(node, &onu_atu_aging_cmd);
        install_element(node, &onu_atu_limit_cmd);

        install_element(node, &onu_vlan_port_isolate_cmd);
        install_element(node, &onu_fe_port_link_mon_cmd);
        install_element(node, &onu_fe_port_mode_mon_cmd);
        install_element(node, &onu_vlan_translation_set_cmd);
        install_element(node, &onu_vlan_translation_del_cmd);
        install_element(node, &onu_vlan_aggregation_add_cmd);
        install_element(node, &onu_vlan_aggregation_del_cmd);

        install_element(node, &onu_vlan_transparent_port_enable_cmd);
        install_element(node, &onu_vlan_enable_cmd);
        install_element(node, &onu_vlan_pvid_cmd);
        install_element(node, &onu_vlan_ingress_filter_cmd);/*New add by luh*/
        install_element(node, &onu_vlan_dot1q_add_cmd);
        install_element(node, &onu_vlan_dot1q_del_cmd);
        install_element(node, &onu_vlan_dot1q_port_add_cmd);
        install_element(node, &onu_vlan_dot1q_port_del_cmd);
        install_element(node, &onu_vlan_dot1q_show_cmd);

        /*qinq for ctc commands*/
        install_element(node, &onu_qinq_vlan_tag_enable_cmd);
        install_element(node, &onu_qinq_vlan_tag_add_cmd);
        install_element(node, &onu_qinq_vlan_tag_del_cmd);
		install_element(node, &onu_qinq_vlan_tag_config_show_cmd);

        install_element(node, &onu_port_igmpsnoop_enable_cmd);
        install_element(node, &onu_port_multicast_vlan_set_cmd);
        install_element(node, &onu_port_igmpsnooping_groupnum_set_cmd);
        install_element(node, &onu_port_igmpsnooping_tagstrip_set_cmd);
        install_element(node, &onu_port_igmpsnoop_fast_leave_cmd);/*New add by luh*/

        install_element(node, &onu_fe_port_en_set_cmd);
        install_element(node, &onu_fe_port_fc_set_cmd);
        install_element(node, &onu_fe_port_mode_set_cmd);
        install_element(node, &onu_fe_ingress_rate_set_cmd);
		install_element(node, &onu_fe_ingress_rate_set_cmd1);
        install_element(node, &onu_fe_egress_rate_set_cmd);
        /*install_element(node, &onu_fe_port_an_rst_cmd);*/
        install_element(node, &onu_fe_port_pause_set_cmd);

        install_element(node, &onu_statistic_port_flush_cmd);
        install_element(node, &onu_atu_learn_cmd);
        install_element(node, &onu_atu_flood_cmd);

        install_element(node, &onu_loopback_port_en_cmd);

        install_element(node, &onu_igmpsnooping_group_aging_time_cmd);
        install_element(node, &onu_igmpsnooping_host_aging_time_cmd);
        install_element(node, &onu_igmpsnooping_max_response_time_cmd);
        install_element(node, &onu_igmpsnooping_max_group_cmd);

        install_element(node, &onu_Igmpsnoop_Authenable_Cmd);


        install_element(node, &onu_conf_qos_class_match_cmd);
        install_element(node, &onu_conf_qos_policy_map_cmd);
        install_element(node, &onu_conf_qos_service_policy_cmd);

        install_element(node, &undo_onu_conf_qos_class_match_cmd);
        install_element(node, &undo_onu_conf_qos_policy_map_cmd);
        install_element(node, &undo_onu_conf_qos_service_policy_cmd);

        install_element(node, &show_onu_conf_qos_policy_map_cmd);
    	install_element (node, &onu_qos_vlan_priority_mode_cmd );   /*new added by luh 2013-05-16*/
    	install_element ( node, &onu_qos_classifier_l2_rules_cmd ); /*new added by luh 2013-05-16*/
    	install_element ( node, &onu_vlan_pas_rule_cmd );           /*new added by luh 2013-05-16*/
        
        install_element(node, &onu_vlan_translation_show_cmd);
        install_element(node, &show_onu_profile_cmd);
        install_element(node, &show_onu_conf_name_by_id_cmd);

	    install_element ( node, &onu_fe_port_mirror_from_set_cmd);  /*new added by luh at 2011.7.12*/
		install_element ( node, &onu_fe_port_mirror_to_set_cmd);    /*new added by luh at 2011.7.12*/
	    install_element ( node, &onu_fe_port_mirror_show_cmd);	    /*new added by luh at 2011.7.12*/

        install_element ( node, &onu_igmpsnoop_gda_add_cmd);    /*new added by luh at 2011.7.12*/
        install_element ( node, &onu_igmpsnoop_gda_del_cmd);    /*new added by luh at 2011.7.12*/
        install_element ( node, &onu_igmpsnoop_gda_show_cmd);   /*new added by luh at 2011.7.12*/
		install_element ( node, &onu_vlan_forward_type_cmd);/*new added at July 26*/
		install_element ( node, &onu_qos_def_pri_cmd);/*new added at July 26*/
		install_element ( node, &onu_qos_user_pri_reg_cmd);/*new added at July 27*/
        install_element ( node, &onu_qos_user_pri_tc_cmd);/*new added at July 27*/
        install_element ( node, &onu_qos_tos_tc_show_cmd);	/*new added at July 27*/
		install_element ( node, &onu_qos_rule_cmd);/*new added at July 27*/
        install_element ( node, &onu_qos_tos_tc_cmd);/*new added at July 27*/
        install_element ( node, &onu_qos_algorithm_cmd);/*new added at July 27*/
        install_element ( node, &onu_qos_ip_pri_en_cmd);/*new added at July 27*/
		install_element ( node, &onu_qos_user_pri_en_cmd);/*new added at July 27*/
		install_element ( node, &onu_fe_port_ingress_rate_limit_base_cmd );/*new added at July 27*/
        return VOS_OK;
    }
    else if(node != ONU_GT861_NODE)
    {
        install_element(node, &onu_bat_vlan_dot1q_rules_show_cmd);
        
        install_element(node, &show_onu_profile_cmd);
        install_element(node, &show_onu_conf_name_by_id_cmd);
        /*comment by wangxiaoyu 2011-07-27 onu节点下添加了自动私有化配置文件操作，为防止在临时私有文件关联时执行关联、去关联操作*/
#if 0
        install_element(node, &onu_profile_associate_cmd);
        install_element(node, &onu_profile_private_cmd);
        install_element(node, &onu_profile_undo_associate_cmd);
#endif
    }


    /*
     * end 2011-0609
     */

	if(( node == ONU_NODE ) ||(node == ONU_GT821_GT831_NODE) || (node == ONU_GT861_NODE) || (node == ONU_GT831B_NODE))
    		onu_Qos_CommandInstall( node );

	if((node == ONU_GT821_GT831_NODE) || (node == ONU_GT861_NODE) /*|| (node == ONU_GT831B_NODE)*/)/*问题单7737*/
		ONU_ExtBrd_IAD_CommandInstall( node);


	/* added by xieshl 20090226, 根据GT815的流控问题新增2个命令 */
	install_element ( node, &onu_cfg_uni_pause_cmd);
	install_element ( node, &onu_poe_mode_cmd);


    /*********************大亚命令****************************/
    install_element ( node, &onu_show_igmpsnoop_version_cmd);
    install_element ( node, &onu_port_igmpsnoop_enable_cmd);
	/*modified by wutw at 4 October*/
    /*install_element ( node, &onu_igmpsnoop_channel_cmd);*/
	/*install_element ( node, &onu_igmpsnoop_aging_cmd);*/
	if(node == ONU_GT861_NODE)
		install_element ( node, &onu_igmpsnoop_gda_add_cmd_for861);
	else
		install_element ( node, &onu_igmpsnoop_gda_add_cmd);
    install_element ( node, &onu_igmpsnoop_gda_del_cmd);
    install_element ( node, &onu_igmpsnoop_gda_show_cmd);
	/*modified by wutw at 4 October*/
    /*install_element ( node, &onu_igmpsnoop_last_member_cmd);*/

    /*install_element ( node, &onu_port_igmpsnoop_param_cmd);*/

	/*add by wangxy 2007-03-12 */
	install_element( node, &onu_igmpsnooping_group_aging_time_cmd );
	install_element( node, &onu_igmpsnooping_host_aging_time_cmd );
	if(node == ONU_GT861_NODE)
		install_element( node, &onu_port_igmpsnoop_fast_leave_cmd_for_861 );
	else
		install_element( node, &onu_port_igmpsnoop_fast_leave_cmd);
	install_element( node, &onu_igmpsnooping_max_response_time_cmd);
	install_element( node, &onu_igmpsnooping_max_group_cmd);

	
    install_element ( node, &onu_vlan_enable_cmd);
	/* modified by chenfj 2007-11-12
	问题单#5778: 无法将端口的PVID设置为默认1
	在GT831/821/810/816中，由于端口的PVID是直接跟端口的默认VLAN相关的，PVID命令没有用处并经常造成理解问题，建议在OLT的这四个ONU节点下去除该命令
     处理：在831节点下删除该命令。
                     在810/816节点下该命令保留，因为当前810/816与811/812共用一个命令节点，而这个命令在811/812上是有用的；但在处理时检查ONU类型；
                     若是810/816，则给出出错信息提示：GT810(或GT816)don't support this cli command
	*/	
	if(node != ONU_GT821_GT831_NODE)
		{
		if(node == ONU_GT861_NODE)
			install_element ( node, &onu_vlan_pvid_cmd_for_861);
		else 
   		 	install_element ( node, &onu_vlan_pvid_cmd);
		}
	if(node == ONU_GT861_NODE)
		{
		install_element ( node, &onu_vlan_forward_type_cmd_for_861);
		install_element ( node, &onu_vlan_ingress_filter_cmd_for_861);
		install_element ( node, &onu_vlan_dot1q_port_add_cmd_for_861 );
		install_element ( node, &onu_vlan_dot1q_port_del_cmd_for_861);
		}
	else{
		install_element ( node, &onu_vlan_forward_type_cmd);
		install_element ( node, &onu_vlan_ingress_filter_cmd);
		install_element ( node, &onu_vlan_dot1q_port_add_cmd);
		install_element ( node, &onu_vlan_dot1q_port_del_cmd);
		}
    install_element ( node, &onu_vlan_dot1q_add_cmd);
    install_element ( node, &onu_vlan_dot1q_del_cmd);
    install_element ( node, &onu_vlan_dot1q_show_cmd);
    install_element ( node, &onu_vlan_port_isolate_cmd);
    
    /*install_element ( node, &onu_atu_size_cmd);*/
    install_element ( node, &onu_atu_aging_cmd);
	if(node == ONU_GT861_NODE)
		{
		install_element ( node, &onu_atu_static_add_cmd_for_861);
		install_element ( node, &onu_atu_static_add__dup_cmd_for_861);
		}
	else{
		install_element ( node, &onu_atu_static_add_cmd);
		install_element ( node, &onu_atu_static_add__dup_cmd );
		}
    
    install_element ( node, &onu_atu_static_del_cmd);
    install_element ( node, &onu_atu_static_show_cmd);
    /*install_element ( node, &onu_event_oam_sym_alarm_cmd);
    install_element ( node, &onu_event_oam_frm_alarm_cmd); 
    install_element ( node, &onu_event_oam_frp_alarm_cmd); 
    install_element ( node, &onu_event_oam_sec_alarm_cmd);  */
    install_element ( node, &onu_event_show_cmd);
	
    install_element ( node, &onu_band_set_cmd);     
    /*install_element ( node, &onu_band_broadcast_set_cmd);*/
	if(node == ONU_GT861_NODE)
		{
		install_element ( node, &onu_fe_port_en_set_cmd_for_861);
		install_element ( node, &onu_fe_port_desc_set_cmd_for_861);
		install_element ( node, &onu_fe_port_mode_set_cmd_for_861 );
		install_element ( node, &onu_fe_port_fc_set_cmd_for_861);
		}
	else{
		install_element ( node, &onu_fe_port_en_set_cmd);
		install_element ( node, &onu_fe_port_desc_set_cmd);
		install_element ( node, &onu_fe_port_mode_set_cmd);
		install_element ( node, &onu_fe_port_fc_set_cmd);
		}

/*added by wangxy 2007-11-06*/
	/* modified by chenfj  问题单7324
	   1  port link_mon 和port mode_mon 命令在831 和861上不支持, 故不挂在这些节点下
	   2  在ONU_NODE  节点下, GT810/GT816/GT812_A/GT811_A/gt815不支持, daya GT811/GT812支持
	   */
	if(!((node == ONU_GT821_GT831_NODE) || (node == ONU_GT861_NODE) || (node == ONU_GT831B_NODE)))
		{
		install_element( node, &onu_fe_port_link_mon_cmd );
		install_element( node, &onu_fe_port_mode_mon_cmd );
		}
	
    /*install_element ( node, &onu_fe_port_rate_set_cmd);*/
	
	/*modified by wutw 2006/11/12*/
    /*install_element ( node, &onu_fe_port_maclimit_set_cmd);*/

	if(node == ONU_GT861_NODE)
		{
		install_element ( node, &onu_qos_def_pri_cmd_for_861);
		install_element ( node, &onu_qos_user_pri_reg_cmd_for_861);
		
		install_element ( node, &onu_loopback_port_en_cmd_for_861);
		install_element ( node, &onu_statistic_port_flush_cmd_for_861);
		install_element ( node, &onu_statistic_port_show_cmd_for_861);
		}
	else{
		install_element ( node, &onu_qos_def_pri_cmd);
		install_element ( node, &onu_qos_user_pri_reg_cmd);

		install_element ( node, &onu_loopback_port_en_cmd);
		install_element ( node, &onu_statistic_port_flush_cmd);
		install_element ( node, &onu_statistic_port_show_cmd);
		}

    /*install_element ( node, &onu_qos_def_port_tc_cmd);*/
    install_element ( node, &onu_qos_user_pri_tc_cmd);
    install_element ( node, &onu_qos_tos_tc_cmd);
    install_element ( node, &onu_qos_algorithm_cmd);

    /*install_element ( node, &onu_loopback_uni_test_cmd);*/
    
    /*install_element ( node, &onu_statistic_port_histogram_cmd);
    install_element ( node, &onu_statistic_port_flush_cmd);
    install_element ( node, &onu_statistic_port_show_cmd);*/
    install_element ( node, &onu_statistic_pas_flush_cmd);

 	/* modified by chenfj 2007-11-12
 	问题单#5794: 不支持温度命令，建议将该命令去掉
 	*/
 	if( node != ONU_GT821_GT831_NODE )
   		install_element ( node, &onu_mgt_temperature_cmd);
    install_element ( node, &onu_mgt_conf_cmd);
    install_element ( node, &onu_mgt_reset_cmd);
    install_element ( node, &onu_mgt_laser_cmd);

	install_element( node, &onu_mgt_read_reg_cmd );
	
    /*install_element ( node, &onu_mgt_self_check_result_cmd); */   

	/*added by wutw at 23 October*/
	if(node == ONU_GT861_NODE)
		install_element ( node, &onu_qos_user_pri_reg_show_cmd_for_861);
	else
		install_element ( node, &onu_qos_user_pri_reg_show_cmd);
    install_element ( node, &onu_qos_user_pri_tc_show_cmd);
    install_element ( node, &onu_qos_tos_tc_show_cmd); 	


	/*added by wutongwu at 18 October*/
	/*install_element ( node, &onu_statistic_port_all_show_cmd); */
	/* modified by chenfj 2007-11-12
	     问题单#5793: mgt self_check_result命令OLT不支持
	     其实是ONU侧不支持该命令
	     修改: 对831 ONU，删除该命令
	     */
	     if( node != ONU_GT821_GT831_NODE )
			install_element ( node, &onu_mgt_self_check_result_all_cmd);
	/*added by wutongwu at 26 October*/		
	
	if(node == ONU_GT861_NODE)
		{
		install_element ( node, &onu_fe_ingress_rate_set_cmd_for_861);
		install_element ( node, &onu_fe_ingress_rate_set_cmd1_for_861);
		install_element ( node, &onu_fe_egress_rate_set_cmd_for_861);
		}
	else{
		install_element ( node, &onu_fe_ingress_rate_set_cmd);
		install_element ( node, &onu_fe_ingress_rate_set_cmd1);
		install_element ( node, &onu_fe_egress_rate_set_cmd);
		}


	/*added by wangxy 2007-05-15*/
	/* modified by chenfj 2007-11-12
	     问题单#5782: port ingress_rate_limit_base命令OLT不支持
	     其实是ONU侧不支持该命令
	     修改: 对831 ONU，删除该命令
	     */
	if( node  != ONU_GT821_GT831_NODE )
		install_element ( node, &onu_fe_port_ingress_rate_limit_base_cmd );

	/*11.6 added by wutw*/
	install_element ( node, &onu_uptime_show_cmd);
	
	if(node == ONU_GT861_NODE)
		{
		install_element ( node, &onu_qos_rule_cmd_for_861);
		install_element ( node, &onu_qos_ip_pri_en_cmd_for_861);
		install_element ( node, &onu_qos_user_pri_en_cmd_for_861);
		
		install_element ( node, &onu_fe_port_negoration_show_cmd_for_861 );
		install_element ( node, &onu_fe_port_mirror_from_set_cmd_for_861);
		install_element ( node, &onu_fe_port_mirror_to_set_cmd_for_861);
		}
	else{
		install_element ( node, &onu_qos_rule_cmd);
		install_element ( node, &onu_qos_ip_pri_en_cmd);
		install_element ( node, &onu_qos_user_pri_en_cmd);
		
		install_element ( node, &onu_fe_port_negoration_show_cmd);
		install_element ( node, &onu_fe_port_mirror_from_set_cmd);
		install_element ( node, &onu_fe_port_mirror_to_set_cmd);
		}

	install_element ( node, &onu_fe_port_show_cmd);
	install_element ( node, &onu_fe_port_mirror_show_cmd);	
 	/*********************end of daya  ********************************/   

	/* only GT816/810 支持的命令*/
	/* modified by chenfj 2007-8-30
		按照大亚方式统一一下命令。 张新辉需按照新的定义修改GT816的软件
	*/
	if(( node == ONU_NODE ) || (node == ONU_GT861_NODE) || (node == ONU_GT831B_NODE))
		{
		install_element ( node, &onu_Igmpsnoop_Authenable_Cmd );
		if(node == ONU_GT861_NODE)
			{
			install_element ( node, &onu_atu_flood_cmd_for_861);
			install_element ( node, &onu_cable_test_gt811_cmd_for_861);

			install_element ( node, &iad_vlan_set_cmd);
			install_element ( node, &onu_slot_mirror_from_to_cmd);
			}
		else{
			install_element ( node, &onu_atu_flood_cmd );
			install_element ( node, &onu_cable_test_gt811_cmd );
			}

		/* ftp download file to onu */
		install_element ( node, &ftpc_download_ftp_phenixos_master_cmd );
		/* added by chenfj 2007-10-30 */
		if(node!=ONU_GT831B_NODE)                 /*问题单7738*/
		install_element ( node, &onu_mgt_config_ip_addr_gt811_cmd );
		}

	if(( node == ONU_NODE ) ||(node == ONU_GT821_GT831_NODE) || (node == ONU_GT861_NODE) || (node == ONU_GT831B_NODE))
		{
		if(node == ONU_GT861_NODE)
			install_element ( node, &onu_atu_learn_cmd_for_861);
		else
			install_element ( node, &onu_atu_learn_cmd );
		
		install_element ( node, &onu_dscp_relay_agent_cmd );

		install_element ( node, &onu_mgt_running_config_gt811_cmd );
		install_element ( node, &onu_mgt_startup_config_gt811_cmd );

		install_element ( node, &onu_atu_limit_cmd );
		}

    return VOS_OK;
}
#else
LONG ONUDAYA_CommandInstall( enum node_type  node)
{

    install_element(node, &onu_profile_associate_cmd);
    install_element(node, &onu_profile_private_cmd);
    install_element(node, &onu_vlan_enable_cmd);
    install_element(node, &onu_vlan_pvid_cmd);
    install_element(node, &onu_vlan_dot1q_add_cmd);
    install_element(node, &onu_vlan_dot1q_del_cmd);
    install_element(node, &onu_vlan_dot1q_port_add_cmd);
    install_element(node, &onu_vlan_dot1q_port_del_cmd);
    install_element(node, &onu_vlan_dot1q_show_cmd);

    install_element(node, &onu_port_igmpsnoop_enable_cmd);

    install_element(node, &onu_fe_port_en_set_cmd);
    install_element(node, &onu_fe_port_fc_set_cmd);

    install_element(node, &onu_config_file_name_show_cmd);

    if(node == ONU_CTC_NODE)
    {
        install_element(node, &onu_vlan_mode_cmd);
    }

    if(node == NEW_ONU_TYPE_CLI_NODE)
    {
        install_element(node, &onu_atu_aging_cmd);
        install_element(node, &onu_atu_limit_cmd);

        install_element(node, &onu_vlan_port_isolate_cmd);
        install_element(node, &onu_fe_port_link_mon_cmd);
        install_element(node, &onu_fe_port_mode_mon_cmd);


        install_element(node, &onu_fe_port_mode_set_cmd);
        install_element(node, &onu_statistic_port_flush_cmd);

        install_element(node, &onu_atu_learn_cmd);
        install_element(node, &onu_atu_flood_cmd);

        install_element(node, &onu_loopback_port_en_cmd);

        install_element(node, &onu_igmpsnooping_group_aging_time_cmd);
        install_element(node, &onu_igmpsnooping_host_aging_time_cmd);
        install_element(node, &onu_igmpsnooping_max_response_time_cmd);
        install_element(node, &onu_igmpsnooping_max_group_cmd);
    }

    if(node == ONU_PROFILE_NODE)
    {

        install_element(node, &onu_vlan_mode_cmd);

        install_element(node, &onu_atu_aging_cmd);
        install_element(node, &onu_atu_limit_cmd);

        install_element(node, &onu_vlan_port_isolate_cmd);
        install_element(node, &onu_fe_port_link_mon_cmd);
        install_element(node, &onu_fe_port_mode_mon_cmd);

        install_element(node, &onu_fe_port_en_set_cmd);
        install_element(node, &onu_fe_port_fc_set_cmd);
        install_element(node, &onu_fe_port_mode_set_cmd);
        install_element(node, &onu_statistic_port_flush_cmd);
        install_element(node, &onu_atu_learn_cmd);
        install_element(node, &onu_atu_flood_cmd);

        install_element(node, &onu_loopback_port_en_cmd);

        install_element(node, &onu_igmpsnooping_group_aging_time_cmd);
        install_element(node, &onu_igmpsnooping_host_aging_time_cmd);
        install_element(node, &onu_igmpsnooping_max_response_time_cmd);
        install_element(node, &onu_igmpsnooping_max_group_cmd);
    }

    return VOS_OK;
}
#endif


int onu_gt861_init_func()
{
    return VOS_OK;
}

int onu_gt861_showrun( struct vty * vty )
{    
    return VOS_OK;
}


int onu_gt861_config_write ( struct vty * vty )
{
    return VOS_OK;
}

struct cmd_node onu_gt861_node =
{
    ONU_GT861_NODE,
    NULL,
    1
};

LONG onu_gt861_node_install()
{
    install_node( &onu_gt861_node, onu_gt861_config_write);
    onu_gt861_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_ONU);
    if ( !onu_gt861_node.prompt )
    {
        ASSERT( 0 );
        return (-IFM_E_NOMEM);
    }
    install_default( ONU_GT861_NODE );
    return VOS_OK;
}


LONG onu_gt861_module_init()
{
    struct cl_cmd_module * onu_gt861_module = NULL;

    onu_gt861_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_ONU);
    if ( !onu_gt861_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) onu_gt861_module, sizeof( struct cl_cmd_module ) );

    onu_gt861_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_ONU);
    if ( !onu_gt861_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( onu_gt861_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( onu_gt861_module->module_name, "onu_gt861" );

    onu_gt861_module->init_func = onu_gt861_init_func;
    onu_gt861_module->showrun_func = /*onu_gt813_showrun*/NULL;
    onu_gt861_module->next = NULL;
    onu_gt861_module->prev = NULL;

	/* if(cmd_rugular_register(&stCMD_onu_slotport_List_Check)==no_match)
 	{
		ASSERT( 0 );
 	}*/	/* removed by xieshl 20090624 重复定义 */

    cl_install_module( onu_gt861_module );
  
    return VOS_OK;
}


LONG  OnuGT861CommandInstall()
{
	
	onu_gt861_node_install();
    	return onu_gt861_module_init();
}


int onu_CTC_init_func()
{
    return VOS_OK;
}

int onu_CTC_showrun( struct vty * vty )
{    
    return VOS_OK;
}


int onu_CTC_config_write ( struct vty * vty )
{
    return VOS_OK;
}

struct cmd_node onu_CTC_node =
{
    ONU_CTC_NODE,
    NULL,
    1
};

LONG onu_CTC_node_install()
{
    install_node( &onu_CTC_node, onu_CTC_config_write);
    onu_CTC_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_ONU);
    if ( !onu_CTC_node.prompt )
    {
        ASSERT( 0 );
        return (-IFM_E_NOMEM);
    }
    install_default( ONU_CTC_NODE );
    return VOS_OK;
}


LONG onu_CTC_module_init()
{
    struct cl_cmd_module * onu_CTC_module = NULL;

    onu_CTC_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_ONU);
    if ( !onu_CTC_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) onu_CTC_module, sizeof( struct cl_cmd_module ) );

    onu_CTC_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_ONU);
    if ( !onu_CTC_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( onu_CTC_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( onu_CTC_module->module_name, "onu_CTC" );

    onu_CTC_module->init_func = onu_CTC_init_func;
    onu_CTC_module->showrun_func = /*onu_gt813_showrun*/onu_CTC_showrun;
    onu_CTC_module->next = NULL;
    onu_CTC_module->prev = NULL;

    cl_install_module( onu_CTC_module );
  
    return VOS_OK;
}



LONG  OnuCTCCommandInstall()
{
	if( V2R1_CTC_STACK )
	{
        CTC_AlarmCmd_Init();        
        onu_CTC_node_install();
        onu_CTC_module_init();
#if 0
		CT_RMan_CLI_Init ();
#else
		CT_RMan_ONU_Init(ONU_CTC_NODE);
#endif
	}
	return( 0 );
}

#if 1
int onu_gpon_init_func()
{
    return VOS_OK;
}

int onu_gpon_showrun( struct vty * vty )
{    
    return VOS_OK;
}


int onu_gpon_config_write ( struct vty * vty )
{
    return VOS_OK;
}

struct cmd_node onu_gpon_node =
{
    ONU_GPON_NODE,
    NULL,
    1
};

LONG onu_gpon_node_install()
{
    install_node( &onu_gpon_node, onu_gpon_config_write);
    onu_gpon_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_ONU);
    if ( !onu_gpon_node.prompt )
    {
        ASSERT( 0 );
        return (-IFM_E_NOMEM);
    }
    install_default( ONU_GPON_NODE );
    return VOS_OK;
}

LONG onu_gpon_module_init()
{
    struct cl_cmd_module * onu_gpon_module = NULL;

    onu_gpon_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_ONU);
    if ( !onu_gpon_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) onu_gpon_module, sizeof( struct cl_cmd_module ) );

    onu_gpon_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_ONU);
    if ( !onu_gpon_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( onu_gpon_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( onu_gpon_module->module_name, "onu_GPON" );

    onu_gpon_module->init_func = onu_gpon_init_func;
    onu_gpon_module->showrun_func = /*onu_gt813_showrun*/NULL;
    onu_gpon_module->next = NULL;
    onu_gpon_module->prev = NULL;

    cl_install_module( onu_gpon_module );
  
    return VOS_OK;
}
LONG ONUGPONDAYA_CommandInstall(enum node_type  node)
{
    install_element(node, &onu_bat_vlan_dot1q_rules_show_cmd);
    install_element(node, &onu_mgt_running_config_gt811_cmd );/*add by luh 2011-11-23*/    
    install_element(node, &onu_vlan_enable_cmd);
    install_element(node, &onu_vlan_pvid_cmd);
    install_element(node, &onu_vlan_dot1q_add_cmd);
    install_element(node, &onu_vlan_dot1q_del_cmd);
    install_element(node, &onu_vlan_dot1q_port_add_cmd);
    install_element(node, &onu_vlan_dot1q_port_del_cmd);
    install_element(node, &onu_vlan_dot1q_show_cmd);
    install_element(node, &onu_vlan_transparent_port_enable_cmd); /*add by yangzl@2016-4-18*/
    /*install_element(node, &onu_port_igmpsnoop_enable_cmd);*/
    install_element(node, &onu_fe_port_en_set_cmd);
    /*install_element(node, &onu_fe_ingress_rate_set_cmd);
    install_element(node, &onu_fe_ingress_rate_set_cmd1);
    install_element(node, &onu_fe_egress_rate_set_cmd);*/
    install_element(node, &onu_fe_port_mode_set_cmd);
	/*
    install_element(node, &onu_fe_port_an_rst_cmd);
    install_element(node, &onu_fe_port_pause_set_cmd);*/
    install_element(node, &onu_vlan_mode_cmd);
    install_element(node, &onu_mgt_reset_cmd);
    install_element(node, &show_onu_profile_cmd);
    install_element(node, &show_onu_conf_name_by_id_cmd);
    install_element(node, &onu_fe_port_show_cmd);

    install_element(node, &onu_qinq_vlan_tag_add_cmd);
    install_element(node, &onu_qinq_vlan_tag_del_cmd);
    install_element(node, &onu_qinq_vlan_tag_config_show_cmd);
   install_element(node, &gpononu_igmp_set_template_cmd);/*add by yangzl for GPON igmpsnooping*/
   install_element(node, &gpononu_igmp_get_template_cmd);
   install_element(node, &gpononu_igmp_set_operating_profile_first_cmd);
   install_element(node, &gpononu_igmp_set_operating_profile_second_cmd);
   install_element(node, &gpononu_igmp_set_operating_profile_third_cmd);
   install_element(node, &gpononu_igmp_get_operating_profile_cmd);
   install_element(node, &gpononu_igmp_set_uniportassociate_cmd);
   install_element(node, &gpononu_igmp_get_uniportassociate_cmd);
   install_element(node, &ctc_onu_statistic_data_show_cmd);
   install_element( node, &ctc_onu_port_statistic_state_cmd);

    return VOS_OK;
}
extern  void GPON_onu_init();
extern LONG ONUCommandInstall( enum node_type  node);

LONG  OnuGPONCommandInstall()
{
    onu_gpon_node_install();
    onu_gpon_module_init();
    GPON_onu_init();    
    ONUGPONDAYA_CommandInstall(ONU_GPON_NODE);
    ONUCommandInstall(ONU_GPON_NODE);
	return (0);
}

#endif
int onu_newtype_init_func()
{
    return VOS_OK;
}

int onu_newtype_showrun( struct vty * vty )
{    
    return VOS_OK;
}


int onu_newtype_config_write ( struct vty * vty )
{
    return VOS_OK;
}

struct cmd_node onu_newtype_node =
{
    NEW_ONU_TYPE_CLI_NODE,
    NULL,
    1
};

LONG onu_newtype_node_install()
{
    install_node( &onu_newtype_node, onu_gt861_config_write);
    onu_newtype_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_ONU);
    if ( !onu_newtype_node.prompt )
    {
        ASSERT( 0 );
        return (-IFM_E_NOMEM);
    }
    install_default( NEW_ONU_TYPE_CLI_NODE );
    return VOS_OK;
}


LONG onu_newtype_module_init()
{
    struct cl_cmd_module * onu_newtype_module = NULL;

    onu_newtype_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_ONU);
    if ( !onu_newtype_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) onu_newtype_module, sizeof( struct cl_cmd_module ) );

    onu_newtype_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_ONU);
    if ( !onu_newtype_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( onu_newtype_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( onu_newtype_module->module_name, "onu_newtype" );

    onu_newtype_module->init_func = onu_newtype_init_func;
    onu_newtype_module->showrun_func = /*onu_gt813_showrun*/NULL;
    onu_newtype_module->next = NULL;
    onu_newtype_module->prev = NULL;

	 if(cmd_rugular_register(&stCMD_onu_slotport_List_Check)==no_match)
 	{
		ASSERT( 0 );
 	}

    cl_install_module( onu_newtype_module );
  
    return VOS_OK;
}

LONG  OnuNewTypeCommandInstall_Ldefun()
{
	
	onu_newtype_node_install();
    onu_newtype_module_init();

	install_element ( NEW_ONU_TYPE_CLI_NODE, &ldef_onu_mgt_conf_cmd);
	return VOS_OK;
}

LONG  OnuNewTypeCommandInstall()
{
	
	onu_newtype_node_install();
    onu_newtype_module_init();

	install_element ( NEW_ONU_TYPE_CLI_NODE, &onu_mgt_conf_cmd);

	return VOS_OK;
}

int CliOnuTypeVty(ULONG slotId , ULONG port, ULONG onuId,ULONG onuFePort,struct vty *vty)
{
	ulong_t devIdx = 0;
	ulong_t devType = 0;
	LONG lRet = VOS_OK;
	/*devIdx = (ulong_t)( slotId*10000 + port*1000 + onuId);*/
        devIdx=(ulong_t)MAKEDEVID(slotId,port,onuId);
	lRet = getDeviceType( devIdx, &devType );
	if(lRet != VOS_OK)
	{	
		vty_out( vty,"  %% get device type error\r\n");
		return VOS_ERROR;
	}
	/* added by chenfj 2009-5-19
	     增加设备类型检查*/
	if((devType >= V2R1_ONU_MAX) || (devType < V2R1_ONU_GT811))
	{
		vty_out( vty, "  %% Unknow onu device`s type\r\n");
		return VOS_ERROR;
	}
    /*modified by luh 2013-05-24, 由onu sysfile 中定义的fe口数目决定端口是否越界*/
#if 0	
	switch(devType)
	{/*INTEGER  { other ( 1 ) , gfa6700 ( 2 ) , gfa6100 ( 3 ) , gt811 ( 4 ) , gt821 
	( 5 ) , gt831 ( 6 ) , gt812 ( 7 ) , gt813 ( 8 ) , gt881 ( 9 ) , gt861 ( 10 ) 
	, gt891 ( 11 ) } */
		case V2R1_OTHER:
			vty_out( vty, "  %% Unknow device`s type\r\n");
			return VOS_ERROR;
		case V2R1_OLT_GFA6700:
			/*gfa6700*/
			vty_out( vty, "  %% olt device %s\r\n",GetDeviceDescString(V2R1_OLT_GFA6700));
			return VOS_ERROR;
		case V2R1_OLT_GFA6100:
			/*6100*/
			vty_out( vty, "  %% olt device %s\r\n",GetDeviceDescString(V2R1_OLT_GFA6100));
			return VOS_ERROR;
		case V2R1_ONU_GT811:
		case V2R1_ONU_GT811_A:
		case V2R1_ONU_GT811_B:
		case V2R1_ONU_GT851:
		case V2R1_ONU_GT871:
			/*gt811*/
			if( (onuFePort<1) || (onuFePort>4))
			{
				vty_out( vty, "  %% Port error. Port must be 1-4\r\n");
				return VOS_ERROR;
			}
			break;
		case V2R1_ONU_GT831:
		case V2R1_ONU_GT831_A:
			/*gt821*/
			/*if( (onuFePort<1) || (onuFePort>5))
			{
				vty_out( vty, "  %% Port error. Port must be 1-5\r\n");
				return VOS_ERROR;
			}				
			break;*/
		case  V2R1_ONU_GT831_CATV:
		case  V2R1_ONU_GT831_A_CATV:
		case V2R1_ONU_GT831_B:
		case  V2R1_ONU_GT831_B_CATV:
			/*gt831*/
			if( (onuFePort<1) || (onuFePort>5))
			{
				vty_out( vty, "  %% Port error. Port must be 1-5\r\n");
				return VOS_ERROR;
			}				
			break;
		case V2R1_ONU_GT812:
		case V2R1_ONU_GT812_A:
		case V2R1_ONU_GT812_B:
		case V2R1_ONU_GT862:
			/*gt812*/
			if( (onuFePort<1) || (onuFePort>8))
			{
				vty_out( vty, "  %% Port error. Port must be 1-8\r\n");
				return VOS_ERROR;
			}				
			break;
		case V2R1_ONU_GT813:
		case V2R1_ONU_GT863:
			/*gt813*/
			if( (onuFePort<1) || (onuFePort>24))
			{
				vty_out( vty, "  %% Port error. Port must be 1-24\r\n");
				return VOS_ERROR;
			}
			break;
		case V2R1_ONU_GT881:
			/*gt881*/
			if( (onuFePort<1) || (onuFePort>4))
			{
				vty_out( vty, "  %% Port error. Port must be 1-4\r\n");
				return VOS_ERROR;
			}				
			break;
		case V2R1_ONU_GT861:
			/*gt861*/
			if( (onuFePort<1) || (onuFePort>34))
			{
				vty_out( vty, "  %% Port error. Port must be 1-34\r\n");
				return VOS_ERROR;
			}	
			break;
		case V2R1_ONU_GT891:
			/*gt891*/
			if( (onuFePort<1) || (onuFePort>4))
			{
				vty_out( vty, "  %% Port error. Port must be 1-4\r\n");
				return VOS_ERROR;
			}				
			break;
		case V2R1_ONU_GT810:
			/*gt810*/
			if( (onuFePort  != 1) /*|| (onuFePort>4)*/)
			{
				vty_out( vty, "  %% Port error. Port must be 1\r\n");
				return VOS_ERROR;
			}				
			break;
		case V2R1_ONU_GT816 :
			if( (onuFePort  != 1) /*|| (onuFePort>4)*/)
			{
				vty_out( vty, "  %% Port error. Port must be 1\r\n");
				return VOS_ERROR;
			}				
			break;			
		/*case V2R1_ONU_GT863:
			if( (onuFePort<1) || (onuFePort>4))
			{
				vty_out( vty, "  %% Port error. Port must be 1-4\r\n");
				return VOS_ERROR;
			}				
			break;*/
		case V2R1_ONU_CTC:
			/*gt_CTC*/
			if( (onuFePort < 1) || (onuFePort > MAX_ETH_PORT_NUM))
			{
				vty_out( vty, "  %% Port error. Port must be 1-%d\r\n", MAX_ETH_PORT_NUM);
				return VOS_ERROR;
			}				
			break;

		case V2R1_ONU_GT865:
		case V2R1_ONU_GT866:
			/*gt865*/
			return ROK;
			break;
		case V2R1_ONU_GT815:
		case V2R1_ONU_GT815_B:
			if( (onuFePort<1) || (onuFePort >16))
			{
				vty_out( vty, "  %% Port error. Port must be 1-16\r\n");
				return VOS_ERROR;
			}				
			break;
		
		default:
			/* modified by chenfj 2009-5-19
			   新增类型ONU 支持的ETH 端口数通过初始系统配置文件获取*/
			{
			int EthNum;
			
			EthNum = GetOnuFePortNum(devType);
			if((onuFePort < 1) || ( onuFePort > EthNum ))
				{
				vty_out( vty, "  %% Port error. Port must be 1-%d\r\n", EthNum);
				return VOS_ERROR;
				}
			}
			break;					
	}
#else
    {
		int EthNum;
		
		getDeviceCapEthPortNum(devIdx, &EthNum);
		if((onuFePort < 1) || ( onuFePort > EthNum ))
		{
			vty_out( vty, "  %% Port error. Port must be 1-%d\r\n", EthNum);
			return VOS_ERROR;
		}
    }
#endif
	return VOS_OK;

}


#include "onu_gt831_cli.c"


/* added by xieshl 20111102, 需求9283, 在新增的ONU-VIEW节点挂接部分命令 */
VOID onu_view_daya_cmd_install()
{
	install_element( ONU_VIEW_NODE, &onu_fe_port_show_cmd );
	install_element( ONU_VIEW_NODE, &onu_statistic_port_show_cmd );
	install_element( ONU_VIEW_NODE, &onu_vlan_dot1q_show_cmd );
	install_element( ONU_VIEW_NODE, &onu_fe_port_negoration_show_cmd );
	install_element( ONU_VIEW_NODE, &onu_event_show_cmd );
	install_element( ONU_VIEW_NODE, &onu_atu_static_show_cmd );
	install_element( ONU_VIEW_NODE, &onu_cable_test_gt811_cmd );
    install_element( ONU_VIEW_NODE, &onu_mgt_reset_cmd);         /*added by luh @2015-1-15*/
}


#ifdef	__cplusplus
}
#endif/* __cplusplus */
